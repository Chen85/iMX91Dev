#ifndef _HALLEDCTRLAPI_H_
#define _HALLEDCTRLAPI_H_
// ==============================================================================
// FILE NAME: HALLEDCTRLAPI.C
// DESCRIPTION:
//
//
// modification history
// --------------------
// 05/09/2013, Larry written
// 15/11/2013, Larry Fixed
// 06/01/2014, Larry Fixed
// --------------------
// ==============================================================================

#include "Common.h"

#define BACKLIGHT_LED_SIZE 3

void halLED_BacklightKeypadStatusSet(eKEYPAD_BACKLIGHT eBacklightKeypad, UINT8 cCustomer_ID);
void halLED_StatueLEDStateSet(eLED_STATUS eStatus);
void halLedCtrl_Init(void);
UINT8 halLedCtrl_Status_Behavior_Get(void);
eRESULT halLedCtrl_Status_Led_Set(eLED_BEHAVIOR eStatus);
void halLED_StatusLedBehaviorSet(eLED_BEHAVIOR eLED_Behavior);
void halLED_SingleKeyLedSet(UINT32 dwData);
void halLED_ShutterLedSet(BOOL bEnable);
eRESULT halLED_VersionGet(UINT8 *pucData); //A70LV_Larry_0166
void halLED_PwrLedSet(UINT8 ucEnable);

#endif /*_HALLEDCTRLAPI_H_*/

