/**
  ******************************************************************************
  * @file    usb_istr.c
  * @brief   ISTR events interrupt service routines
  ******************************************************************************
  */

#include "usb_lib.h"
#include "usb_prop.h"
#include "usb_pwr.h"
#include "usb_istr.h"

__IO uint16_t wIstr;
__IO uint8_t bIntPackSOF = 0;

void (*pEpInt_IN[7])(void) =
  {
    EP1_IN_Callback,
    EP2_IN_Callback,
    EP3_IN_Callback,
    EP4_IN_Callback,
    EP5_IN_Callback,
    EP6_IN_Callback,
    EP7_IN_Callback,
  };

void (*pEpInt_OUT[7])(void) =
  {
    EP1_OUT_Callback,
    EP2_OUT_Callback,
    EP3_OUT_Callback,
    EP4_OUT_Callback,
    EP5_OUT_Callback,
    EP6_OUT_Callback,
    EP7_OUT_Callback,
  };

#ifndef STM32F10X_CL

void USB_Istr(void)
{
  wIstr = _GetISTR();

#if (IMR_MSK & ISTR_CTR)
  if (wIstr & ISTR_CTR & wInterrupt_Mask)
  {
    CTR_LP();
#ifdef CTR_CALLBACK
    CTR_Callback();
#endif
  }
#endif

#if (IMR_MSK & ISTR_RESET)
  if (wIstr & ISTR_RESET & wInterrupt_Mask)
  {
    _SetISTR((uint16_t)CLR_RESET);
    Device_Property.Reset();
#ifdef RESET_CALLBACK
    RESET_Callback();
#endif
  }
#endif

#if (IMR_MSK & ISTR_DOVR)
  if (wIstr & ISTR_DOVR & wInterrupt_Mask)
  {
    _SetISTR((uint16_t)CLR_DOVR);
#ifdef DOVR_CALLBACK
    DOVR_Callback();
#endif
  }
#endif

#if (IMR_MSK & ISTR_ERR)
  if (wIstr & ISTR_ERR & wInterrupt_Mask)
  {
    _SetISTR((uint16_t)CLR_ERR);
#ifdef ERR_CALLBACK
    ERR_Callback();
#endif
  }
#endif

#if (IMR_MSK & ISTR_WKUP)
  if (wIstr & ISTR_WKUP & wInterrupt_Mask)
  {
    _SetISTR((uint16_t)CLR_WKUP);
    Resume(RESUME_EXTERNAL);
#ifdef WKUP_CALLBACK
    WKUP_Callback();
#endif
  }
#endif

#if (IMR_MSK & ISTR_SUSP)
  if (wIstr & ISTR_SUSP & wInterrupt_Mask)
  {
    if (fSuspendEnabled)
    {
      Suspend();
    }
    else
    {
      Resume(RESUME_LATER);
    }
    _SetISTR((uint16_t)CLR_SUSP);
#ifdef SUSP_CALLBACK
    SUSP_Callback();
#endif
  }
#endif

#if (IMR_MSK & ISTR_SOF)
  if (wIstr & ISTR_SOF & wInterrupt_Mask)
  {
    _SetISTR((uint16_t)CLR_SOF);
    bIntPackSOF++;

#ifdef SOF_CALLBACK
    SOF_Callback();
#endif
  }
#endif

#if (IMR_MSK & ISTR_ESOF)
  if (wIstr & ISTR_ESOF & wInterrupt_Mask)
  {
    _SetISTR((uint16_t)CLR_ESOF);
    Resume(RESUME_ESOF);

#ifdef ESOF_CALLBACK
    ESOF_Callback();
#endif
  }
#endif
}

#endif /* STM32F10X_CL */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/