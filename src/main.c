/**
  ******************************************************************************
  * @file    main.c
  * @brief   Main program body for the C6 mini CMSIS-DAP.
  ******************************************************************************
  */

#include "stm32f1xx.h"
#include "system_stm32f1xx.h"

#include "../dap/DAP.h"
#include "hw_config.h"
#include "usb_lib.h"
#include "usb_pwr.h"
#include "usb_endp.h"
#include "../config/DAP_config.h"

/* SysTick 1 ms counter used for LED timing. */
static volatile uint32_t SysTick_ms;

void SysTick_Handler(void)
{
  SysTick_ms++;
}

uint32_t Get_SysTick_ms(void)
{
  return SysTick_ms;
}

/* CMSIS-DAP core descriptor: LED callbacks invoked by the host status command. */
static void SetLED_Connected(uint16_t b)
{
  if (b)
  {
    LED_CONNECTED_PORT->ODR |= LED_CONNECTED_MASK;
  }
  else
  {
    LED_CONNECTED_PORT->ODR &= ~LED_CONNECTED_MASK;
  }
}

static void SetLED_Running(uint16_t b)
{
  if (b)
  {
    LED_RUNNING_PORT->ODR |= LED_RUNNING_MASK;
  }
  else
  {
    LED_RUNNING_PORT->ODR &= ~LED_RUNNING_MASK;
  }
}

static CoreDescriptor_t coreDescriptor = { SetLED_Connected, SetLED_Running };
const CoreDescriptor_t *pCoreDescriptor = &coreDescriptor;

int main(void)
{
  Set_System();
  SystemCoreClockUpdate();

  /* SysTick for LED blink timing (1 ms). */
  SysTick_Config(SystemCoreClock / 1000UL);
  SysTick_ms = 0;

  /* Enable the DWT cycle counter for the DAP_TIMESTAMP feature. */
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CYCCNT = 0;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

  DAP_Setup();
  Get_SerialNum();   /* fill USB serial number descriptor from chip UID */

  USB_Interrupts_Config();
  Set_USBClock();
  USB_Init();

  HID_Init();
  CDC_Init();

  while (1)
  {
    /* CMSIS-DAP commands received on the HID endpoint. */
    HID_Process();
    HID_SendPending();

    /* CDC bridge: USART RX -> USB IN, USB OUT -> USART TX. */
    CDC_RxSend();
    USART_TX_Kick();

#if (CDC_JTAG_SWITCH != 0)
    /* JTAG idle timeout: revert shared PA9/PA10 back to CDC if the host
       has been quiet for CDC_JTAG_TIMEOUT_MS. */
    JTAG_Port_Tick();
#endif

    /* Target Running LED blinks once per second. */
    if ((SysTick_ms % 1000UL) == 0UL)
    {
      LED_RUNNING_PORT->ODR ^= LED_RUNNING_MASK;
      SysTick_ms = 1UL;
    }

    /* Connected LED reflects the USB configured state. */
    if (bDeviceState == CONFIGURED)
    {
      LED_CONNECTED_PORT->ODR |= LED_CONNECTED_MASK;
    }
    else
    {
      LED_CONNECTED_PORT->ODR &= ~LED_CONNECTED_MASK;
    }
  }
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/