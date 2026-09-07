/**
  ******************************************************************************
  * @file    usb_conf.h
  * @brief   Configuration for the composite HID + CDC USB device on C6.
  ******************************************************************************
  */

#ifndef __USB_CONF_H
#define __USB_CONF_H

#include "platform_config.h"

/*-------------------------------------------------------------*/
/* EP_NUM - number of physical (non-control) endpoint pairs used.
   EP0..EP3 are used: EP0 control, EP1 CDC notify IN,
   EP2 CDC data IN/OUT, EP3 HID IN/OUT.                                */
/*-------------------------------------------------------------*/
#define EP_NUM     (4)

#ifndef STM32F10X_CL
/*-------------------------------------------------------------*/
/* --------------   Buffer Description Table  -----------------*/
/*-------------------------------------------------------------*/
#define BTABLE_ADDRESS      (0x00)

/* EP0  rx/tx buffer base address (64 bytes each) */
#define ENDP0_RXADDR        (0x40)
#define ENDP0_TXADDR        (0x80)

/* EP1  CDC notification endpoint (IN, 8 bytes) */
#define ENDP1_TXADDR        (0xC0)

/* EP2  CDC data endpoints (bulk 64 bytes each) */
#define ENDP2_TXADDR        (0x100)
#define ENDP2_RXADDR        (0x140)

/* EP3  HID endpoints (interrupt 64 bytes each) */
#define ENDP3_TXADDR        (0x180)
#define ENDP3_RXADDR        (0x1C0)

/*-------------------------------------------------------------*/
/* -------------------   ISTR events  -------------------------*/
/*-------------------------------------------------------------*/
#define IMR_MSK (CNTR_CTRM  | CNTR_WKUPM | CNTR_SUSPM | CNTR_ERRM  | CNTR_SOFM \
                 | CNTR_ESOFM | CNTR_RESETM )
#endif /* STM32F10X_CL */

/* CTR service routines */
/* associated to defined endpoints */
/*#define  EP1_IN_Callback   NOP_Process */
/*#define  EP2_IN_Callback   NOP_Process */
/*#define  EP3_IN_Callback   NOP_Process */
#define  EP4_IN_Callback   NOP_Process
#define  EP5_IN_Callback   NOP_Process
#define  EP6_IN_Callback   NOP_Process
#define  EP7_IN_Callback   NOP_Process

#define  EP1_OUT_Callback  NOP_Process
/*#define  EP2_OUT_Callback   NOP_Process */
/*#define  EP3_OUT_Callback   NOP_Process */
#define  EP4_OUT_Callback  NOP_Process
#define  EP5_OUT_Callback  NOP_Process
#define  EP6_OUT_Callback  NOP_Process
#define  EP7_OUT_Callback  NOP_Process

#endif /* __USB_CONF_H */