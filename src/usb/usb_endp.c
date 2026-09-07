/**
  ******************************************************************************
  * @file    usb_endp.c
  * @brief   Endpoint routines for the composite HID (CMSIS-DAP) + CDC bridge.
  ******************************************************************************
  */

#include "hw_config.h"
#include "usb_lib.h"
#include "usb_istr.h"
#include "usb_pwr.h"
#include "usb_desc.h"
#include "usb_endp.h"
#include "../config/DAP_config.h"
#include "../dap/DAP.h"

extern volatile uint8_t DAP_TransferAbort;

void HID_SendPending(void);

/* ---------------- HID / CMSIS-DAP buffers ---------------- */

static volatile uint8_t  HID_RequestFlag;
static volatile uint32_t HID_RequestIn;
static volatile uint32_t HID_RequestOut;
static volatile uint8_t  HID_ResponseIdle;
static volatile uint8_t  HID_ResponsePending;
static volatile uint32_t HID_ResponseIn;
static volatile uint32_t HID_ResponseOut;

static uint8_t HID_Request [DAP_PACKET_COUNT][DAP_PACKET_SIZE];
static uint8_t HID_Response[DAP_PACKET_COUNT][DAP_PACKET_SIZE];

void HID_Init(void)
{
  uint32_t n;

  HID_RequestFlag = 0;
  HID_RequestIn = 0;
  HID_RequestOut = 0;
  HID_ResponseIdle = 1;
  HID_ResponsePending = 0;
  HID_ResponseIn = 0;
  HID_ResponseOut = 0;

  for (n = 0; n < DAP_PACKET_COUNT; n++)
  {
    HID_Request[n][0] = 0;
    HID_Response[n][0] = 0;
  }
}

/* USB HID Callback: data received from the host on EP3 OUT */
void EP3_OUT_Callback(void)
{
  uint8_t Buffer[DAP_PACKET_SIZE];
  uint32_t n;

  USB_SIL_Read(EP3_OUT, Buffer);

  if (Buffer[0] == ID_DAP_TransferAbort)
  {
    DAP_TransferAbort = 1;
  }
  else
  {
    n = HID_RequestIn + 1;
    if (n == DAP_PACKET_COUNT)
    {
      n = 0;
    }
    if (HID_RequestFlag || (n == HID_RequestOut))
    {
      /* Request ring full: discard this packet. */
    }
    else
    {
      for (n = 0; n < DAP_PACKET_SIZE; n++)
      {
        HID_Request[HID_RequestIn][n] = Buffer[n];
      }
      HID_RequestIn++;
      if (HID_RequestIn == DAP_PACKET_COUNT)
      {
        HID_RequestIn = 0;
      }
      if (HID_RequestIn == HID_RequestOut)
      {
        HID_RequestFlag = 1;
      }
    }
  }

  SetEPRxStatus(ENDP3, EP_RX_VALID);
}

/* USB HID Callback: data sent to the host on EP3 IN */
void EP3_IN_Callback(void)
{
  HID_ResponseIdle = 1;
  HID_SendPending();
}

/* Process one pending CMSIS-DAP request. Call periodically from main loop. */
uint32_t HID_Process(void)
{
  if (HID_RequestFlag || (HID_RequestOut != HID_RequestIn))
  {
    uint32_t n;

    DAP_ProcessCommand(HID_Request[HID_RequestOut], HID_Response[HID_ResponseIn]);

    n = HID_RequestOut + 1;
    if (n == DAP_PACKET_COUNT)
    {
      n = 0;
    }
    HID_RequestOut = n;
    if (HID_RequestOut == HID_RequestIn)
    {
      HID_RequestFlag = 0;
    }

    HID_ResponseIn++;
    if (HID_ResponseIn == DAP_PACKET_COUNT)
    {
      HID_ResponseIn = 0;
    }
    HID_ResponsePending++;

    HID_SendPending();
    return 1;
  }
  return 0;
}

/* Send one queued response on EP3 IN if the endpoint is idle. */
void HID_SendPending(void)
{
  uint32_t n;

  if (HID_ResponsePending && HID_ResponseIdle)
  {
    HID_ResponseIdle = 0;
    USB_SIL_Write(EP3_IN, HID_Response[HID_ResponseOut], DAP_PACKET_SIZE);
    SetEPTxStatus(ENDP3, EP_TX_VALID);
    HID_ResponseOut++;
    if (HID_ResponseOut == DAP_PACKET_COUNT)
    {
      HID_ResponseOut = 0;
    }
    HID_ResponsePending--;
  }
}

/* ---------------- CDC bridge buffers ---------------- */

#define CDC_BUFFER_SIZE  USART_CDC_BUFFER_SIZE
#define CDC_MASK         (CDC_BUFFER_SIZE - 1)

/* Flow control: halt the EP2 OUT receiver when less than a full packet of
   space remains, resume once a packet's worth has been drained. */
#define CDC_FLOW_THRESHOLD (CDC_BUFFER_SIZE - CDC_DATA_MAX_PACKET_SIZE)

/* Staging buffer for EP2 IN (UART RX -> host). */
static uint8_t USB_Rx_Buffer[CDC_DATA_MAX_PACKET_SIZE];
/* Staging buffer for EP2 OUT (host -> USART TX). Kept separate from the IN
   buffer so full-duplex traffic cannot corrupt an in-flight IN packet. */
static uint8_t CDC_OutBuf[CDC_DATA_MAX_PACKET_SIZE];
static volatile uint8_t CDC_TxFlowNak;

/* Both rings are single-producer/single-consumer using head/tail indices
   only (no shared byte counter), so ISR preemption cannot race the count.
   The producer writes only In, the consumer only Out. */

/* Ring: host -> USART (produced in EP2_OUT_Callback, consumed by the
   USART TX interrupt). */
static uint8_t  CDC_TxBuf[CDC_BUFFER_SIZE];
static volatile uint32_t CDC_TxIn;
static volatile uint32_t CDC_TxOut;

/* Ring: USART -> host (produced by the USART RX interrupt, consumed by
   CDC_RxSend()). */
static uint8_t  CDC_RxBuf[CDC_BUFFER_SIZE];
static volatile uint32_t CDC_RxIn;
static volatile uint32_t CDC_RxOut;
static volatile uint8_t  CDC_RxBusy;

static uint32_t CDC_TxAvailable(void) { return (CDC_TxIn - CDC_TxOut) & CDC_MASK; }
static uint32_t CDC_RxAvailable(void) { return (CDC_RxIn - CDC_RxOut) & CDC_MASK; }

void CDC_Init(void)
{
  CDC_TxIn = 0;
  CDC_TxOut = 0;
  CDC_RxIn = 0;
  CDC_RxOut = 0;
  CDC_RxBusy = 0;
  CDC_TxFlowNak = 0;
}

/* ---- host -> UART ---- */
void CDC_TxPush(uint8_t data)
{
  if (((CDC_TxIn + 1) & CDC_MASK) == CDC_TxOut)
  {
    return;   /* ring full, drop */
  }
  CDC_TxBuf[CDC_TxIn] = data;
  CDC_TxIn = (CDC_TxIn + 1) & CDC_MASK;
}

uint32_t CDC_TxPending(void)
{
  return CDC_TxAvailable();
}

/* Pop one byte for the USART transmitter. Returns -1 when empty. */
int32_t CDC_TxPop(void)
{
  uint32_t data;

  if (CDC_TxIn == CDC_TxOut)
  {
    return -1;
  }
  data = CDC_TxBuf[CDC_TxOut];
  CDC_TxOut = (CDC_TxOut + 1) & CDC_MASK;
  return (int32_t)data;
}

/* ---- UART -> host ---- */
void CDC_RxPush(uint8_t data)
{
  if (((CDC_RxIn + 1) & CDC_MASK) == CDC_RxOut)
  {
    return;   /* ring full, drop */
  }
  CDC_RxBuf[CDC_RxIn] = data;
  CDC_RxIn = (CDC_RxIn + 1) & CDC_MASK;
}

uint32_t CDC_RxPending(void)
{
  return CDC_RxAvailable();
}

/* Send UART RX data up to the host on EP2 IN if the endpoint is idle. */
void CDC_RxSend(void)
{
  uint8_t out;
  uint32_t n;
  uint32_t avail;

  if (CDC_RxBusy)
  {
    return;
  }

  avail = CDC_RxAvailable();
  if (avail == 0)
  {
    return;
  }

  n = avail;
  if (n > CDC_DATA_MAX_PACKET_SIZE)
  {
    n = CDC_DATA_MAX_PACKET_SIZE;
  }

  for (out = 0; out < n; out++)
  {
    USB_Rx_Buffer[out] = CDC_RxBuf[CDC_RxOut];
    CDC_RxOut = (CDC_RxOut + 1) & CDC_MASK;
  }

  /* USB_SIL_Write copies into the PMA synchronously, so the ring can be
     advanced here. The busy flag only gates overlapping IN transfers. */
  USB_SIL_Write(EP2_IN, USB_Rx_Buffer, n);
  CDC_RxBusy = 1;
  SetEPTxStatus(ENDP2, EP_TX_VALID);
#ifdef CDC_USB_LOOPBACK_TEST
  CDC_EP2OutFlow();
#endif
}

/* EP2 OUT callback: USB CDC data received from host.
   Normal mode: data -> USART TX ring (see CDC_TxPush).
   Test mode (CDC_USB_LOOPBACK_TEST): echo directly back to EP2 IN via the
   RX ring so the USB/flow-control path can be exercised without UART.
   The F1 USB peripheral auto-NAKs a single-buffered RX endpoint after every
   received packet, so flow control is natural: simply stop re-arming it
   while less than a full packet of ring space remains. CDC_EP2OutFlow()
   re-arms it once that space has been drained. */
void EP2_OUT_Callback(void)
{
  uint32_t n;
  uint32_t i;

  n = USB_SIL_Read(EP2_OUT, CDC_OutBuf);
  for (i = 0; i < n; i++)
  {
#ifdef CDC_USB_LOOPBACK_TEST
    CDC_RxPush(CDC_OutBuf[i]);
#else
    CDC_TxPush(CDC_OutBuf[i]);
#endif
  }

#ifdef CDC_USB_LOOPBACK_TEST
  if (CDC_RxAvailable() >= CDC_FLOW_THRESHOLD)
#else
  if (CDC_TxAvailable() >= CDC_FLOW_THRESHOLD)
#endif
  {
    CDC_TxFlowNak = 1;   /* leave the endpoint halted */
  }
  else
  {
    CDC_TxFlowNak = 0;
    SetEPRxStatus(ENDP2, EP_RX_VALID);
  }
}

/* Re-arm EP2 OUT once the ring in use has drained below the flow threshold.
   Normal mode: called from the USART interrupt on each byte popped.
   Test mode: called from CDC_RxSend() when the RX ring is drained. */
void CDC_EP2OutFlow(void)
{
#ifdef CDC_USB_LOOPBACK_TEST
  uint32_t avail = CDC_RxAvailable();
#else
  uint32_t avail = CDC_TxAvailable();
#endif
  if (CDC_TxFlowNak && (avail < CDC_FLOW_THRESHOLD))
  {
    CDC_TxFlowNak = 0;
    SetEPRxStatus(ENDP2, EP_RX_VALID);
  }
}

/* EP2 IN callback: USB CDC data sent to host, endpoint idle again */
void EP2_IN_Callback(void)
{
  CDC_RxBusy = 0;
}

/* EP1 IN callback: CDC notification endpoint (not used) */
void EP1_IN_Callback(void)
{
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/