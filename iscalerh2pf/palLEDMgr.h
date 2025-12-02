#ifndef _PALLEDMGR_H_
#define _PALLEDMGR_H_

#include "Common.h"

void palLEDMgr_BacklightKeypadStatusSet(eKEYPAD_BACKLIGHT eBacklightKeypad, UINT8 cCustomer_ID);
void palLEDMgr_StatueLEDStateSet(eLED_STATUS eStatus);
void palLEDMgr_StatusLedBehaviorSet(eLED_BEHAVIOR eLED_Behavior);
void palLEDMgr_SingleKeyLedSet(UINT32 dwData);
void palLEDMgr_ShutterLedSet(BOOL bEnable);
eRESULT palLEDMgr_VersionGet(UINT8 *pucData);
void palLEDMgr_PwrLedSet(UINT8 ucEnable);

#endif /*_PALLEDMGR_H_*/

