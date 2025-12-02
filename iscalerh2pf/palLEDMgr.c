#include "Common.h"
#include "utilDbgMsg.h"
#include "utilMathAPI.h"
#include "utilCLICmdAPI.h"
#include "utilHostAPI.h"
#include "halLEDCtrlAPI.h"
#include "palLEDMgr.h"

// ==============================================================================
// FUNCTION NAME: palLEDMgr_BacklightKeypadStatusSet
// DESCRIPTION:
//
//
// Params:
// eBACKLIGHT_KEYPAD eBacklightKeypad:
// UINT8 cTimerOut:
// UINT8 cCustomer_ID:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/10/28, Larry Create
// --------------------
// ==============================================================================
void palLEDMgr_BacklightKeypadStatusSet(eKEYPAD_BACKLIGHT eBacklightKeypad, UINT8 cCustomer_ID)
{
    halLED_BacklightKeypadStatusSet(eBacklightKeypad, cCustomer_ID);
}

// ==============================================================================
// FUNCTION NAME: palLEDMgr_StatueLEDStateSet
// DESCRIPTION:
//
//
// Params:
// eSTATUS_LED eSatusLed:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/10/28, Larry Create
// --------------------
// ==============================================================================
void palLEDMgr_StatueLEDStateSet(eLED_STATUS eStatus)
{
#ifdef LED_CONTROL
    (void)eStatus;
#else
    UINT8 cData = (UINT8)eStatus;
    halLED_StatueLEDStateSet(cData);
#endif /* PROJECTOR_2K */
}

// ==============================================================================
// FUNCTION NAME: palLEDMgr_StatusLedBehaviorSet
// DESCRIPTION:
//
//
// Params:
// eLED_BEHAVIOR eLED_Behavior:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/10/28, Larry Create
// --------------------
// ==============================================================================
void palLEDMgr_StatusLedBehaviorSet(eLED_BEHAVIOR eLED_Behavior)
{
    halLED_StatusLedBehaviorSet(eLED_Behavior);
}

// ==============================================================================
// FUNCTION NAME: palLEDMgr_SingleKeyLedSet
// DESCRIPTION:
//
//
// Params:
// UINT32 dwData:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/10/28, Larry Create
// --------------------
// ==============================================================================
void palLEDMgr_SingleKeyLedSet(UINT32 dwData)
{
#ifdef LED_CONTROL
    (void)dwData;
#else
    halLED_SingleKeyLedSet(dwData);
#endif /* PROJECTOR_2K */
}

// ==============================================================================
// FUNCTION NAME: palLEDMgr_ShutterLedSet
// DESCRIPTION:
//
//
// Params:
// BOOL bEnable:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/10/28, Larry Create
// --------------------
// ==============================================================================
void palLEDMgr_ShutterLedSet(BOOL bEnable) //A70LH_Larry_0043
{
    halLED_ShutterLedSet(bEnable);
}


// ==============================================================================
// FUNCTION NAME: palLEDMgr_VersionGet
// DESCRIPTION:
//
//
// Params:
// UINT8 *pucData:
//
// Returns:
//
//
// Modification History
// --------------------
// 2018/05/21, Larry Create
// --------------------
// ==============================================================================
eRESULT palLEDMgr_VersionGet(UINT8 *pucData)
{
    return halLED_VersionGet(pucData);
}

// ==============================================================================
// FUNCTION NAME: palLEDMgr_PwrLedSet
// DESCRIPTION:
//
//
// Params:
// BOOL bEnable:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/10/28, Larry Create
// --------------------
// ==============================================================================
void palLEDMgr_PwrLedSet(UINT8 ucEnable)
{
    halLED_PwrLedSet(ucEnable);
}


