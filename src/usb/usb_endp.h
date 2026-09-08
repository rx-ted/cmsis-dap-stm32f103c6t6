/**
  ******************************************************************************
  * @file    usb_endp.h
  * @brief   Endpoint routines interface for the composite HID + CDC device.
  ******************************************************************************
  */

#ifndef __USB_ENDP_H
#define __USB_ENDP_H

#include "usb_lib.h"

void HID_Init(void);
uint32_t HID_Process(void);
void HID_SendPending(void);

void CDC_Init(void);
void CDC_TxPush(uint8_t data);
int32_t CDC_TxPop(void);
uint32_t CDC_TxPending(void);
void CDC_RxPush(uint8_t data);
uint32_t CDC_RxPending(void);
void CDC_RxSend(void);
void CDC_EP2OutFlow(void);

#endif  /* __USB_ENDP_H */