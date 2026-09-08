/**
  ******************************************************************************
  * @file    usb_prop.h
  * @brief   All processing related to the composite HID + CDC device.
  ******************************************************************************
  */

#ifndef __USB_PROP_H
#define __USB_PROP_H

#include "usb_conf.h"

#define GET_PROTOCOL                0x03
#define SET_PROTOCOL                0x0B

#define SET_LINE_CODING             0x20
#define GET_LINE_CODING             0x21
#define SET_CONTROL_LINE_STATE      0x22

#define USB_CDC_ACM                  1
#define USB_CDC_DATA                 2

typedef struct
{
  uint32_t dwDTERate;
  uint8_t  bCharFormat;
  uint8_t  bDataBits;
  uint8_t  bParityType;
  uint8_t  padding;
} LINE_CODING;

void USBD_init(void);
void USBD_Reset(void);
void USBD_SetConfiguration(void);
void USBD_SetDeviceAddress(void);
void USBD_Status_In(void);
void USBD_Status_Out(void);
RESULT USBD_Data_Setup(uint8_t RequestNo);
RESULT USBD_NoData_Setup(uint8_t RequestNo);
RESULT USBD_Get_Interface_Setting(uint8_t Interface, uint8_t AlternateSetting);
uint8_t *USBD_GetDeviceDescriptor(uint16_t Length);
uint8_t *USBD_GetConfigDescriptor(uint16_t Length);
uint8_t *USBD_GetStringDescriptor(uint16_t Length);
uint8_t *USBD_GetHIDReportDescriptor(uint16_t Length);
uint8_t *USBD_GetHIDDescriptor(uint16_t Length);
uint8_t *USBD_GetProtocolValue(uint16_t Length);
RESULT USBD_SetProtocol(void);
uint8_t *USBD_GetLineCoding(uint16_t Length);
uint8_t *USBD_SetLineCoding(uint16_t Length);

#define USBD_GetConfiguration          NOP_Process
/*#define USBD_SetConfiguration          NOP_Process*/
#define USBD_GetInterface              NOP_Process
#define USBD_SetInterface              NOP_Process
#define USBD_GetStatus                 NOP_Process
#define USBD_ClearFeature              NOP_Process
#define USBD_SetEndPointFeature        NOP_Process
#define USBD_SetDeviceFeature          NOP_Process
/*#define USBD_SetDeviceAddress          NOP_Process*/

#endif /* __USB_PROP_H */