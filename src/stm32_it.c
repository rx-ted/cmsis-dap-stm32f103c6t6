/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32_it.c
  * @brief   Interrupt Service Routines for the C6 CMSIS-DAP firmware.
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "usb_lib.h"
#include "hw_config.h"
#include "usb_pwr.h"
#include "usb_istr.h"

/* External variables --------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Interruption and Exception Handlers         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  */
void HardFault_Handler(void)
{
  while (1)
  {
  }
}

void MemManage_Handler(void)
{
  while (1)
  {
  }
}

void BusFault_Handler(void)
{
  while (1)
  {
  }
}

void UsageFault_Handler(void)
{
  while (1)
  {
  }
}

void SVC_Handler(void)
{
}

void DebugMon_Handler(void)
{
}

void PendSV_Handler(void)
{
}

/******************************************************************************/
/*                 STM32 Peripherals Interrupt Handlers                       */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f103xb.s).                                            */
/******************************************************************************/

/**
  * @brief  This function handles USB Low Priority or CAN1 RX0 interrupts.
  */
void USB_LP_CAN1_RX0_IRQHandler(void)
{
  USB_Istr();
}

/**
  * @brief  This function handles USART1 global interrupt.
  */
void USART1_IRQHandler(void)
{
  USART_IRQHandler_Ext();
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/