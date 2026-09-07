/**
  ******************************************************************************
  * @file    usb_desc.h
  * @brief   Descriptor Header for the composite HID + CDC device on C6.
  ******************************************************************************
  */

#ifndef __USB_DESC_H
#define __USB_DESC_H

#define USB_DEVICE_DESCRIPTOR_TYPE              0x01
#define USB_CONFIGURATION_DESCRIPTOR_TYPE       0x02
#define USB_STRING_DESCRIPTOR_TYPE              0x03
#define USB_INTERFACE_DESCRIPTOR_TYPE           0x04
#define USB_ENDPOINT_DESCRIPTOR_TYPE            0x05

#define HID_DESCRIPTOR_TYPE                     0x21
#define REPORT_DESCRIPTOR                       0x22
#define USBD_SIZ_HID_DESC                       0x09

#define USBD_SIZ_DEVICE_DESC                  18
#define USBD_SIZ_CONFIG_DESC                  99
#define USBD_SIZ_REPORT_DESC                  34
#define USBD_SIZ_STRING_LANGID                 4
#define USBD_SIZ_STRING_VENDOR                 8
#define USBD_SIZ_STRING_PRODUCT               28
#define USBD_SIZ_STRING_SERIAL               22

#define USBD_HID_DESC_OFF_SET                  18      /* offset of HID descriptor in config */

#define CDC_DATA_MAX_PACKET_SIZE              64
#define CDC_CMD_MAX_PACKET_SIZE                8
#define HID_MAX_PACKET_SIZE                   64

extern const uint8_t USBD_DeviceDescriptor[USBD_SIZ_DEVICE_DESC];
extern const uint8_t USBD_ConfigDescriptor[USBD_SIZ_CONFIG_DESC];
extern const uint8_t USBD_ReportDescriptor[USBD_SIZ_REPORT_DESC];
extern const uint8_t USBD_StringLangID[USBD_SIZ_STRING_LANGID];
extern const uint8_t USBD_StringVendor[USBD_SIZ_STRING_VENDOR];
extern const uint8_t USBD_StringProduct[USBD_SIZ_STRING_PRODUCT];
extern uint8_t USBD_StringSerial[USBD_SIZ_STRING_SERIAL];

#endif /* __USB_DESC_H */