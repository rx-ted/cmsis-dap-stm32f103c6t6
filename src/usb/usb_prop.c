/**
  ******************************************************************************
  * @file    usb_prop.c
  * @brief   Class specific requests and device property table for the
  *          composite HID + CDC device.
  ******************************************************************************
  */

#include "hw_config.h"
#include "usb_lib.h"
#include "usb_conf.h"
#include "usb_prop.h"
#include "usb_desc.h"
#include "usb_pwr.h"

extern LINE_CODING linecoding;

DEVICE Device_Table =
  {
    EP_NUM,
    1
  };

DEVICE_PROP Device_Property =
  {
    USBD_init,
    USBD_Reset,
    USBD_Status_In,
    USBD_Status_Out,
    USBD_Data_Setup,
    USBD_NoData_Setup,
    USBD_Get_Interface_Setting,
    USBD_GetDeviceDescriptor,
    USBD_GetConfigDescriptor,
    USBD_GetStringDescriptor,
    0,
    0x40                                          /* MaxPacketSize */
  };

USER_STANDARD_REQUESTS User_Standard_Requests =
  {
    USBD_GetConfiguration,
    USBD_SetConfiguration,
    USBD_GetInterface,
    USBD_SetInterface,
    USBD_GetStatus,
    USBD_ClearFeature,
    USBD_SetEndPointFeature,
    USBD_SetDeviceFeature,
    USBD_SetDeviceAddress
  };

ONE_DESCRIPTOR Device_Descriptor =
  {
    (uint8_t *)USBD_DeviceDescriptor,
    USBD_SIZ_DEVICE_DESC
  };

ONE_DESCRIPTOR Config_Descriptor =
  {
    (uint8_t *)USBD_ConfigDescriptor,
    USBD_SIZ_CONFIG_DESC
  };

ONE_DESCRIPTOR USBD_Report_Descriptor =
  {
    (uint8_t *)USBD_ReportDescriptor,
    USBD_SIZ_REPORT_DESC
  };

ONE_DESCRIPTOR USBD_Hid_Descriptor =
  {
    (uint8_t *)USBD_ConfigDescriptor + USBD_HID_DESC_OFF_SET,
    USBD_SIZ_HID_DESC
  };

ONE_DESCRIPTOR String_Descriptor[4] =
  {
    {(uint8_t *)USBD_StringLangID, USBD_SIZ_STRING_LANGID},
    {(uint8_t *)USBD_StringVendor, USBD_SIZ_STRING_VENDOR},
    {(uint8_t *)USBD_StringProduct, USBD_SIZ_STRING_PRODUCT},
    {(uint8_t *)USBD_StringSerial, USBD_SIZ_STRING_SERIAL}
  };

void USBD_init(void)
{
  pInformation->Current_Configuration = 0;

  PowerOn();

  USB_SIL_Init();

  bDeviceState = UNCONNECTED;
}

void USBD_Reset(void)
{
  pInformation->Current_Configuration = 0;
  pInformation->Current_Interface = 0;
  pInformation->Current_Feature = USBD_ConfigDescriptor[7];

#ifndef STM32F10X_CL
  SetBTABLE(BTABLE_ADDRESS);

  /* Initialize Endpoint 0 */
  SetEPType(ENDP0, EP_CONTROL);
  SetEPTxStatus(ENDP0, EP_TX_STALL);
  SetEPRxAddr(ENDP0, ENDP0_RXADDR);
  SetEPTxAddr(ENDP0, ENDP0_TXADDR);
  Clear_Status_Out(ENDP0);
  SetEPRxCount(ENDP0, Device_Property.MaxPacketSize);
  SetEPRxValid(ENDP0);

  /* Initialize Endpoint 1 - CDC notification (IN, interrupt) */
  SetEPType(ENDP1, EP_INTERRUPT);
  SetEPTxAddr(ENDP1, ENDP1_TXADDR);
  SetEPTxCount(ENDP1, CDC_CMD_MAX_PACKET_SIZE);
  SetEPTxStatus(ENDP1, EP_TX_NAK);
  SetEPRxStatus(ENDP1, EP_RX_DIS);

  /* Initialize Endpoint 2 - CDC data (bulk IN/OUT) */
  SetEPType(ENDP2, EP_BULK);
  SetEPTxAddr(ENDP2, ENDP2_TXADDR);
  SetEPTxStatus(ENDP2, EP_TX_NAK);
  SetEPRxAddr(ENDP2, ENDP2_RXADDR);
  SetEPRxCount(ENDP2, CDC_DATA_MAX_PACKET_SIZE);
  SetEPRxStatus(ENDP2, EP_RX_VALID);

  /* Initialize Endpoint 3 - HID (interrupt IN/OUT) */
  SetEPType(ENDP3, EP_INTERRUPT);
  SetEPTxAddr(ENDP3, ENDP3_TXADDR);
  SetEPTxStatus(ENDP3, EP_TX_NAK);
  SetEPRxAddr(ENDP3, ENDP3_RXADDR);
  SetEPRxCount(ENDP3, HID_MAX_PACKET_SIZE);
  SetEPRxStatus(ENDP3, EP_RX_VALID);

  /* Set this device to response on default address */
  SetDeviceAddress(0);
#endif

  bDeviceState = ATTACHED;
}

void USBD_SetConfiguration(void)
{
  if (pInformation->Current_Configuration != 0)
  {
    bDeviceState = CONFIGURED;
  }
}

void USBD_SetDeviceAddress(void)
{
  bDeviceState = ADDRESSED;
}

void USBD_Status_In(void)
{
}

void USBD_Status_Out(void)
{
}

RESULT USBD_Data_Setup(uint8_t RequestNo)
{
  uint8_t *(*CopyRoutine)(uint16_t);
  uint8_t Interface;
  uint8_t wValue1;

  CopyRoutine = NULL;
  Interface = pInformation->USBwIndex0;

  wValue1 = pInformation->USBwValue1;

  switch (Interface)
  {
    case 0:
      /* HID interface requests */
      if ((RequestNo == GET_DESCRIPTOR)
          && (Type_Recipient == (STANDARD_REQUEST | INTERFACE_RECIPIENT)))
      {
        if (wValue1 == REPORT_DESCRIPTOR)
        {
          CopyRoutine = USBD_GetHIDReportDescriptor;
        }
        else if (wValue1 == HID_DESCRIPTOR_TYPE)
        {
          CopyRoutine = USBD_GetHIDDescriptor;
        }
      }
      if ((Type_Recipient == (CLASS_REQUEST | INTERFACE_RECIPIENT))
          && (RequestNo == GET_PROTOCOL))
      {
        CopyRoutine = USBD_GetProtocolValue;
      }
      break;

    case USB_CDC_ACM:
      /* CDC Communication interface requests */
      if ((Type_Recipient == (CLASS_REQUEST | INTERFACE_RECIPIENT))
          && (RequestNo == GET_LINE_CODING))
      {
        CopyRoutine = USBD_GetLineCoding;
      }
      if ((Type_Recipient == (CLASS_REQUEST | INTERFACE_RECIPIENT))
          && (RequestNo == SET_LINE_CODING))
      {
        CopyRoutine = USBD_SetLineCoding;
      }
      break;

    default:
      break;
  }

  if (CopyRoutine == NULL)
  {
    return USB_UNSUPPORT;
  }

  pInformation->Ctrl_Info.CopyData = CopyRoutine;
  pInformation->Ctrl_Info.Usb_wOffset = 0;
  (*CopyRoutine)(0);
  return USB_SUCCESS;
}

RESULT USBD_NoData_Setup(uint8_t RequestNo)
{
  uint8_t Interface = pInformation->USBwIndex0;

  switch (Interface)
  {
    case 0:
      /* HID interface requests */
      if ((Type_Recipient == (CLASS_REQUEST | INTERFACE_RECIPIENT))
          && (RequestNo == SET_PROTOCOL))
      {
        return USBD_SetProtocol();
      }
      break;

    case USB_CDC_ACM:
      /* CDC Communication interface requests */
      if ((Type_Recipient == (CLASS_REQUEST | INTERFACE_RECIPIENT))
          && (RequestNo == SET_CONTROL_LINE_STATE))
      {
        return USB_SUCCESS;
      }
      break;

    default:
      break;
  }

  return USB_UNSUPPORT;
}

uint8_t *USBD_GetDeviceDescriptor(uint16_t Length)
{
  return Standard_GetDescriptorData(Length, &Device_Descriptor);
}

uint8_t *USBD_GetConfigDescriptor(uint16_t Length)
{
  return Standard_GetDescriptorData(Length, &Config_Descriptor);
}

uint8_t *USBD_GetStringDescriptor(uint16_t Length)
{
  uint8_t wValue0 = pInformation->USBwValue0;

  if (wValue0 > 4)
  {
    return NULL;
  }
  return Standard_GetDescriptorData(Length, &String_Descriptor[wValue0]);
}

uint8_t *USBD_GetHIDReportDescriptor(uint16_t Length)
{
  return Standard_GetDescriptorData(Length, &USBD_Report_Descriptor);
}

uint8_t *USBD_GetHIDDescriptor(uint16_t Length)
{
  return Standard_GetDescriptorData(Length, &USBD_Hid_Descriptor);
}

RESULT USBD_Get_Interface_Setting(uint8_t Interface, uint8_t AlternateSetting)
{
  if (AlternateSetting > 0)
  {
    return USB_UNSUPPORT;
  }
  if (Interface > USB_CDC_DATA)
  {
    return USB_UNSUPPORT;
  }
  return USB_SUCCESS;
}

uint8_t *USBD_GetProtocolValue(uint16_t Length)
{
  static uint8_t ProtocolValue = 0;

  if (Length == 0)
  {
    pInformation->Ctrl_Info.Usb_wLength = 1;
    return NULL;
  }
  return (uint8_t *)(&ProtocolValue);
}

RESULT USBD_SetProtocol(void)
{
  pInformation->USBwValue0;
  return USB_SUCCESS;
}

/* CDC Line Coding */
uint8_t *USBD_GetLineCoding(uint16_t Length)
{
  extern LINE_CODING linecoding;

  if (Length == 0)
  {
    pInformation->Ctrl_Info.Usb_wLength = sizeof(linecoding);
    return NULL;
  }
  return (uint8_t *)&linecoding;
}

uint8_t *USBD_SetLineCoding(uint16_t Length)
{
  extern LINE_CODING linecoding;

  if (Length == 0)
  {
    pInformation->Ctrl_Info.Usb_wLength = sizeof(linecoding);
    return NULL;
  }
  return (uint8_t *)&linecoding;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/