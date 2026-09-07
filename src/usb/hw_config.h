/**
  ******************************************************************************
  * @file    hw_config.h
  * @brief   Hardware Configuration & Setup Header
  ******************************************************************************
  */

#ifndef __HW_CONFIG_H
#define __HW_CONFIG_H

#include "platform_config.h"

void Set_System(void);
void Set_USBClock(void);
void GPIO_Configuration(void);
void USB_Interrupts_Config(void);
void USB_Cable_Config(FunctionalState NewState);
void Get_SerialNum(void);
void Enter_LowPowerMode(void);
void Leave_LowPowerMode(void);

void USART_Configuration(void);
void USART_TX_Kick(void);
void USART_IRQHandler_Ext(void);

#endif  /*__HW_CONFIG_H*/