/**
  ******************************************************************************
  * @file    usb_desc.c
  * @brief   Descriptors for the composite HID + CDC (ACM) device.
  *          Interface 0: HID (CMSIS-DAP, EP3 IN/OUT)
  *          Interface 1: CDC Communication (EP1 IN, notify)
  *          Interface 2: CDC Data (EP2 IN/OUT, bulk)
  ******************************************************************************
  */

#include "usb_lib.h"
#include "usb_desc.h"
#include "../config/DAP_config.h"

/* USB Standard Device Descriptor */
const uint8_t USBD_DeviceDescriptor[USBD_SIZ_DEVICE_DESC] =
  {
    0x12,                       /* bLength */
    USB_DEVICE_DESCRIPTOR_TYPE, /* bDescriptorType */
    0x00,                       /* bcdUSB = 2.00 */
    0x02,
    0x00,                       /* bDeviceClass: composite */
    0x00,                       /* bDeviceSubClass */
    0x00,                       /* bDeviceProtocol */
    0x40,                       /* bMaxPacketSize0 */
    0x28,                       /* idVendor (0x0D28 = ARM) */
    0x0D,
    0x04,                       /* idProduct (0x0204 = CMSIS-DAP) */
    0x02,
    0x00,                       /* bcdDevice 0.00 */
    0x00,
    1,                          /* iManufacturer */
    2,                          /* iProduct */
    3,                          /* iSerialNumber */
    0x01                        /* bNumConfigurations */
  };

/* USB Configuration Descriptor */
const uint8_t USBD_ConfigDescriptor[USBD_SIZ_CONFIG_DESC] =
  {
    /* Configuration Descriptor */
    0x09,                                   /* bLength */
    USB_CONFIGURATION_DESCRIPTOR_TYPE,      /* bDescriptorType */
    USBD_SIZ_CONFIG_DESC,                   /* wTotalLength */
    0x00,
    0x03,                                   /* bNumInterfaces */
    0x01,                                   /* bConfigurationValue */
    0x00,                                   /* iConfiguration */
    0x80,                                   /* bmAttributes: bus powered */
    0x32,                                   /* bMaxPower = 100 mA */

    /* ---------- Interface 0: HID ---------- */
    0x09,                                   /* bLength */
    USB_INTERFACE_DESCRIPTOR_TYPE,          /* bDescriptorType */
    0x00,                                   /* bInterfaceNumber = 0 */
    0x00,                                   /* bAlternateSetting */
    0x02,                                   /* bNumEndpoints */
    0x03,                                   /* bInterfaceClass: HID */
    0x00,                                   /* bInterfaceSubClass */
    0x00,                                   /* bInterfaceProtocol */
    0x00,                                   /* iInterface */

    /* HID descriptor */
    0x09,                                   /* bLength */
    HID_DESCRIPTOR_TYPE,                    /* bDescriptorType */
    0x01,                                   /* bcdHID 1.01 */
    0x01,
    0x00,                                   /* bCountryCode */
    0x01,                                   /* bNumDescriptors */
    REPORT_DESCRIPTOR,                      /* bDescriptorType */
    USBD_SIZ_REPORT_DESC,                   /* wItemLength */
    0x00,

    /* HID OUT endpoint (EP3 OUT) */
    0x07,                                   /* bLength */
    USB_ENDPOINT_DESCRIPTOR_TYPE,           /* bDescriptorType */
    0x03,                                   /* bEndpointAddress: OUT3 */
    0x03,                                   /* bmAttributes: Interrupt */
    HID_MAX_PACKET_SIZE,                    /* wMaxPacketSize */
    0x00,
    0x01,                                   /* bInterval */

    /* HID IN endpoint (EP3 IN) */
    0x07,                                   /* bLength */
    USB_ENDPOINT_DESCRIPTOR_TYPE,           /* bDescriptorType */
    0x83,                                   /* bEndpointAddress: IN3 */
    0x03,                                   /* bmAttributes: Interrupt */
    HID_MAX_PACKET_SIZE,                    /* wMaxPacketSize */
    0x00,
    0x01,                                   /* bInterval */

    /* ---------- Interface 1: CDC Communication ---------- */
    0x09,                                   /* bLength */
    USB_INTERFACE_DESCRIPTOR_TYPE,          /* bDescriptorType */
    0x01,                                   /* bInterfaceNumber = 1 */
    0x00,                                   /* bAlternateSetting */
    0x01,                                   /* bNumEndpoints */
    0x02,                                   /* bInterfaceClass: CDC */
    0x02,                                   /* bInterfaceSubClass: ACM */
    0x01,                                   /* bInterfaceProtocol: AT commands */
    0x00,                                   /* iInterface */

    /* Header Functional Descriptor */
    0x05,
    0x24,
    0x00,
    0x10,
    0x01,

    /* Call Management Functional Descriptor */
    0x05,
    0x24,
    0x01,
    0x00,
    0x02,

    /* ACM Functional Descriptor */
    0x04,
    0x24,
    0x02,
    0x02,

    /* Union Functional Descriptor */
    0x05,
    0x24,
    0x06,
    0x01,
    0x02,

    /* CDC notify endpoint (EP1 IN) */
    0x07,                                   /* bLength */
    USB_ENDPOINT_DESCRIPTOR_TYPE,           /* bDescriptorType */
    0x81,                                   /* bEndpointAddress: IN1 */
    0x03,                                   /* bmAttributes: Interrupt */
    CDC_CMD_MAX_PACKET_SIZE,                /* wMaxPacketSize */
    0x00,
    0xFF,                                   /* bInterval */

    /* ---------- Interface 2: CDC Data ---------- */
    0x09,                                   /* bLength */
    USB_INTERFACE_DESCRIPTOR_TYPE,          /* bDescriptorType */
    0x02,                                   /* bInterfaceNumber = 2 */
    0x00,                                   /* bAlternateSetting */
    0x02,                                   /* bNumEndpoints */
    0x0A,                                   /* bInterfaceClass: CDC Data */
    0x00,                                   /* bInterfaceSubClass */
    0x00,                                   /* bInterfaceProtocol */
    0x00,                                   /* iInterface */

    /* CDC data OUT endpoint (EP2 OUT) */
    0x07,                                   /* bLength */
    USB_ENDPOINT_DESCRIPTOR_TYPE,           /* bDescriptorType */
    0x02,                                   /* bEndpointAddress: OUT2 */
    0x02,                                   /* bmAttributes: Bulk */
    CDC_DATA_MAX_PACKET_SIZE,               /* wMaxPacketSize */
    0x00,
    0x00,                                   /* bInterval */

    /* CDC data IN endpoint (EP2 IN) */
    0x07,                                   /* bLength */
    USB_ENDPOINT_DESCRIPTOR_TYPE,           /* bDescriptorType */
    0x82,                                   /* bEndpointAddress: IN2 */
    0x02,                                   /* bmAttributes: Bulk */
    CDC_DATA_MAX_PACKET_SIZE,               /* wMaxPacketSize */
    0x00,
    0x00                                    /* bInterval */
  };

/* Vendor-defined HID report descriptor (CMSIS-DAP): one 64-byte
   Input report (DAP response) and one 64-byte Output report (DAP
   request). No Report ID is used. */
const uint8_t USBD_ReportDescriptor[USBD_SIZ_REPORT_DESC] =
  {
    0x06, 0x00, 0xFF,      /* USAGE_PAGE (Vendor Defined: 0xFF00) */
    0x09, 0x01,            /* USAGE (CMSIS-DAP) */
    0xA1, 0x01,            /* COLLECTION (Application) */
    0x09, 0x02,            /*   USAGE (DAP Input) */
    0x15, 0x00,            /*   LOGICAL_MINIMUM (0) */
    0x26, 0xFF, 0x00,      /*   LOGICAL_MAXIMUM (255) */
    0x75, 0x08,            /*   REPORT_SIZE (8) */
    0x95, 0x40,            /*   REPORT_COUNT (64) */
    0x81, 0x02,            /*   INPUT (Data,Var,Abs) */
    0x09, 0x02,            /*   USAGE (DAP Output) */
    0x15, 0x00,            /*   LOGICAL_MINIMUM (0) */
    0x26, 0xFF, 0x00,      /*   LOGICAL_MAXIMUM (255) */
    0x75, 0x08,            /*   REPORT_SIZE (8) */
    0x95, 0x40,            /*   REPORT_COUNT (64) */
    0x91, 0x02,            /*   OUTPUT (Data,Var,Abs) */
    0xC0                   /* END_COLLECTION */
  };

/* USB String Descriptors */
const uint8_t USBD_StringLangID[USBD_SIZ_STRING_LANGID] =
  {
    USBD_SIZ_STRING_LANGID,
    USB_STRING_DESCRIPTOR_TYPE,
    0x09,
    0x04
  };

const uint8_t USBD_StringVendor[USBD_SIZ_STRING_VENDOR] =
  {
    USBD_SIZ_STRING_VENDOR,
    USB_STRING_DESCRIPTOR_TYPE,
    'A', 0, 'R', 0, 'M', 0
  };

const uint8_t USBD_StringProduct[USBD_SIZ_STRING_PRODUCT] =
  {
    USBD_SIZ_STRING_PRODUCT,
    USB_STRING_DESCRIPTOR_TYPE,
    'C', 0, 'M', 0, 'S', 0, 'I', 0, 'S', 0, '-', 0,
    'D', 0, 'A', 0, 'P', 0, '-', 0, 'C', 0, '6', 0
  };

uint8_t USBD_StringSerial[USBD_SIZ_STRING_SERIAL] =
  {
    USBD_SIZ_STRING_SERIAL,
    USB_STRING_DESCRIPTOR_TYPE,
    '0', 0, '0', 0, '0', 0, '0', 0
  };

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/