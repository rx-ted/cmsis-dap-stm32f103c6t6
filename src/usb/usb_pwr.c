/**
  ******************************************************************************
  * @file    usb_pwr.c
  * @brief   Connection/disconnection & power management
  ******************************************************************************
  */

#include "hw_config.h"
#include "usb_lib.h"
#include "usb_conf.h"
#include "usb_pwr.h"

__IO uint32_t bDeviceState = UNCONNECTED;
__IO bool fSuspendEnabled = TRUE;

struct
{
  __IO RESUME_STATE eState;
  __IO uint8_t bESOFcnt;
} ResumeS;

RESULT PowerOn(void)
{
#ifndef STM32F10X_CL
  uint16_t wRegVal;

  /* cable plugged-in ? */
  USB_Cable_Config(ENABLE);

  /* CNTR_PWDN = 0 */
  wRegVal = CNTR_FRES;
  _SetCNTR(wRegVal);

  /* CNTR_FRES = 0 */
  wInterrupt_Mask = 0;
  _SetCNTR(wInterrupt_Mask);

  _SetISTR(0);

  wInterrupt_Mask = CNTR_RESETM | CNTR_SUSPM | CNTR_WKUPM;
  _SetCNTR(wInterrupt_Mask);
#endif

  return USB_SUCCESS;
}

RESULT PowerOff(void)
{
#ifndef STM32F10X_CL
  _SetCNTR(CNTR_FRES);
  _SetISTR(0);
  USB_Cable_Config(DISABLE);
  _SetCNTR(CNTR_FRES + CNTR_PDWN);
#endif

  return USB_SUCCESS;
}

void Suspend(void)
{
#ifndef STM32F10X_CL
  uint16_t wCNTR;

  wCNTR = _GetCNTR();
  wCNTR |= CNTR_FSUSP;
  _SetCNTR(wCNTR);
#endif

  Enter_LowPowerMode();
}

void Resume_Init(void)
{
  Leave_LowPowerMode();

#ifndef STM32F10X_CL
  _SetCNTR(IMR_MSK);
#endif
}

void Resume(RESUME_STATE eResumeSetVal)
{
#ifndef STM32F10X_CL
  uint16_t wCNTR;
#endif

  if (eResumeSetVal != RESUME_ESOF)
  {
    ResumeS.eState = eResumeSetVal;
  }

  switch (ResumeS.eState)
  {
    case RESUME_EXTERNAL:
      Resume_Init();
      ResumeS.eState = RESUME_OFF;
      break;
    case RESUME_INTERNAL:
      Resume_Init();
      ResumeS.eState = RESUME_START;
      break;
    case RESUME_LATER:
      ResumeS.bESOFcnt = 2;
      ResumeS.eState = RESUME_WAIT;
      break;
    case RESUME_WAIT:
      ResumeS.bESOFcnt--;
      if (ResumeS.bESOFcnt == 0)
      {
        ResumeS.eState = RESUME_START;
      }
      break;
    case RESUME_START:
     #ifndef STM32F10X_CL
      wCNTR = _GetCNTR();
      wCNTR |= CNTR_RESUME;
      _SetCNTR(wCNTR);
     #endif
      ResumeS.eState = RESUME_ON;
      ResumeS.bESOFcnt = 10;
      break;
    case RESUME_ON:
     #ifndef STM32F10X_CL
      ResumeS.bESOFcnt--;
      if (ResumeS.bESOFcnt == 0)
      {
        wCNTR = _GetCNTR();
        wCNTR &= (~CNTR_RESUME);
        _SetCNTR(wCNTR);
        ResumeS.eState = RESUME_OFF;
      }
     #endif
      break;
    case RESUME_OFF:
    case RESUME_ESOF:
    default:
      ResumeS.eState = RESUME_OFF;
      break;
  }
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/