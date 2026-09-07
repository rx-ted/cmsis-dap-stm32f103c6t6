/**
  ******************************************************************************
  * @file    hw_config.c
  * @brief   Hardware Configuration & Setup for the C6 CMSIS-DAP.
  *          Register-level only (no StdPeriph driver library).
  ******************************************************************************
  */

#include "hw_config.h"
#include "usb_lib.h"
#include "usb_desc.h"
#include "usb_prop.h"
#include "usb_pwr.h"
#include "usb_endp.h"
#include "../config/DAP_config.h"

/* CDC line coding, used by usb_prop.c */
LINE_CODING linecoding = {115200, 0, 8, 0, 0};

/* Chip unique ID (96-bit, base 0x1FFFF7E8) formatted as "C6" + 8 hex digits. */
const char *GetUID_String(void)
{
  static const char hex[] = "0123456789ABCDEF";
  static char uid[ DAP_SER_NUM_LEN ];
  const uint32_t *pid = (const uint32_t *)UID_BASE;
  uint32_t v = pid[0] ^ ((pid[1] << 16) | (pid[2] & 0xFFFF));
  char *p = uid;
  uint32_t n;

  *p++ = 'C';
  *p++ = '6';
  for (n = 0; n < 8U; n++)
  {
    *p++ = hex[(v >> 28) & 0x0F];
    v <<= 4;
  }
  *p = '\0';
  return uid;
}

/* Fill the USB serial number string descriptor (UTF-16LE) from the chip UID. */
void Get_SerialNum(void)
{
  uint8_t *p = (uint8_t *)USBD_StringSerial + 2U;
  const char *s = GetUID_String();

  while (*s)
  {
    *p++ = (uint8_t)*s;
    *p++ = 0U;
    s++;
  }
}

void Clock_Configuration(void);

void Set_System(void)
{
  Clock_Configuration();
  GPIO_Configuration();
  USART_Configuration();
}

/* 72 MHz system clock from HSE (8 MHz) x 9, USB clock 48 MHz (SYSCLK / 1.5). */
void Clock_Configuration(void)
{
  uint32_t timeout;

  /* Enable HSE */
  RCC->CR |= RCC_CR_HSEON;
  timeout = 0x100000;
  while (!(RCC->CR & RCC_CR_HSERDY))
  {
    if (--timeout == 0)
    {
      break;   /* no crystal: continue on HSI rather than hang */
    }
  }

  /* FLASH: prefetch + 2 wait states for 72 MHz */
  FLASH->ACR = FLASH_ACR_PRFTBE | FLASH_ACR_LATENCY_2;

  /* HCLK = SYSCLK, PCLK2 = HCLK, PCLK1 = HCLK/2, ADCCLK = PCLK2/2 */
  RCC->CFGR = RCC_CFGR_PPRE1_DIV2;

  /* PLL: HSE x 9 = 72 MHz, USB clock = PLL / 1.5 = 48 MHz (USBPRE = 0) */
  RCC->CFGR |= RCC_CFGR_PLLSRC | RCC_CFGR_PLLMULL9;
  RCC->CFGR &= ~RCC_CFGR_USBPRE;

  /* Enable PLL and wait until ready */
  RCC->CR |= RCC_CR_PLLON;
  timeout = 0x100000;
  while (!(RCC->CR & RCC_CR_PLLRDY))
  {
    if (--timeout == 0)
    {
      break;
    }
  }

  /* Switch system clock to PLL */
  RCC->CFGR |= RCC_CFGR_SW_PLL;
  timeout = 0x100000;
  while ((RCC->CFGR & RCC_CFGR_SW_Msk) != RCC_CFGR_SW_PLL)
  {
    if (--timeout == 0)
    {
      break;
    }
  }
}

static void Set_Pin_Nibbles(GPIO_TypeDef *port, uint16_t pinmask, uint32_t mode)
{
  uint32_t crl, crh;
  uint32_t pin;

  crl = port->CRL;
  crh = port->CRH;

  for (pin = 0; pin < 16; pin++)
  {
    if (pinmask & (1UL << pin))
    {
      if (pin < 8)
      {
        crl = (crl & ~((uint32_t)0xF << (pin << 2))) | (mode << (pin << 2));
      }
      else
      {
        crh = (crh & ~((uint32_t)0xF << ((pin - 8) << 2))) | (mode << ((pin - 8) << 2));
      }
    }
  }

  port->CRL = crl;
  port->CRH = crh;
}

void GPIO_Configuration(void)
{
  uint32_t mode;

  /* Clocks for GPIOA/GPIOB/USART1 (APB2) and USB (APB1) */
  RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN |
                  RCC_APB2ENR_USART1EN;
  RCC->APB1ENR |= RCC_APB1ENR_USBEN;

  /* PA2 SWDIO, PA4 SWCLK: output push-pull 50 MHz (0b0011) */
  mode = 0x3;
  Set_Pin_Nibbles(GPIOA, (1UL << 2) | (1UL << 4), mode);

  /* PA6 nRESET: output push-pull 50 MHz, drive high (idle) */
  Set_Pin_Nibbles(GPIOA, (1UL << 6), 0x3);
  GPIOA->BSRR = (1UL << 6);

  /* PA9 USART1 TX: AF push-pull 50 MHz (0xC), PA10 RX: input floating (0x4) */
  Set_Pin_Nibbles(GPIOA, (1UL << 9), 0x0B);
  Set_Pin_Nibbles(GPIOA, (1UL << 10), 0x04);

  /* PA11/PA12 USB D-/D+: AF push-pull 10 MHz (0x9) */
  Set_Pin_Nibbles(GPIOA, (1UL << 11) | (1UL << 12), 0x09);

  /* PB8 LED_CONNECTED, PB12 LED_RUNNING: output push-pull 2 MHz (0x1), off */
  Set_Pin_Nibbles(GPIOB, (1UL << 8) | (1UL << 12), 0x01);
  GPIOB->BSRR = (1UL << 8) | (1UL << 12);
}

void USART_Configuration(void)
{
  /* USART1: 115200 8-N-1, RX interrupt enabled. PCLK2 = 72 MHz. */
  USART1->BRR = 72000000UL / 115200UL;
  USART1->CR1 = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE | USART_CR1_RXNEIE;
}

/* Kick the USART transmitter when USB has queued data.
   Only enables the TXE interrupt - all bytes are written to DR from the
   ISR, so the main loop can never race the interrupt on DR. */
void USART_TX_Kick(void)
{
  if (CDC_TxPending() != 0)
  {
    USART1->CR1 |= USART_CR1_TXEIE;
  }
}

/* Called from USART1_IRQHandler. */
void USART_IRQHandler_Ext(void)
{
  uint32_t sr = USART1->SR;

  if (sr & USART_SR_RXNE)
  {
    uint8_t data = (uint8_t)(USART1->DR);
    CDC_RxPush(data);
  }
  /* Clear sticky overrun/error flags (reading DR clears them). */
  if (sr & (USART_SR_ORE | USART_SR_NE | USART_SR_FE | USART_SR_PE))
  {
    (void)USART1->DR;
  }
  if (USART1->SR & USART_SR_TXE)
  {
    int32_t data = CDC_TxPop();
    if (data >= 0)
    {
      USART1->DR = (uint8_t)data;
      CDC_EP2OutFlow();
    }
    else
    {
      USART1->CR1 &= ~USART_CR1_TXEIE;
    }
  }
}

void Set_USBClock(void)
{
  /* USB clock is PLL / 1.5 = 48 MHz, already selected in Clock_Configuration. */
}

/* C6 has no external USB D+ pull-up control: USB is always connected. */
void USB_Cable_Config(FunctionalState NewState)
{
  (void)NewState;
}

void USB_Interrupts_Config(void)
{
  NVIC_SetPriority(USB_LP_CAN1_RX0_IRQn, 0);
  NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);
  NVIC_SetPriority(USART1_IRQn, 0);
  NVIC_EnableIRQ(USART1_IRQn);
}

void Enter_LowPowerMode(void)
{
  /* Keep it simple: stay fully running. */
}

void Leave_LowPowerMode(void)
{
}

/* ---- CMSIS-DAP runtime pin control (declared in DAP_config.h) ---- */

void PORT_SWD_SETUP(void)
{
  PIN_SWDIO_TMS_OUT_ENABLE();
  PIN_SWCLK_TCK_SET();
  PIN_nRESET_HIGH();
}

void PORT_OFF(void)
{
  PIN_SWDIO_TMS_OUT_DISABLE();
  PIN_nRESET_HIGH();
}

void vResetTarget(uint8_t bit)
{
  if (bit)
  {
    PIN_nRESET_LOW();
    PIN_nRESET_HIGH();
  }
  else
  {
    PIN_nRESET_HIGH();
  }
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/