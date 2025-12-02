// ==============================================================================
// FILE NAME: HALLEDCTRL.C
// DESCRIPTION:
//
//
// modification history
// --------------------
// 05/09/2013, Larry written
// A70_Larry_0306 fixd
// --------------------
// ==============================================================================

#include "halLEDCtrlAPI.h"
#include "dvLpcIOExp.h"
#include "utilDbgMsg.h"
#include "utilMathAPI.h"
#include "utilCLICmdAPI.h"
#include "utilHostAPI.h"


// ==============================================================================
// FUNCTION NAME: halLED_BacklightKeypadStatusSet
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
void halLED_BacklightKeypadStatusSet(eKEYPAD_BACKLIGHT eBacklightKeypad, UINT8 cCustomer_ID)
{
    sLEDPAYLOAD sLedPayload;

    switch(eBacklightKeypad)
    {
        case eKEYPAD_LED_BACKLIGHT_05:
            sLedPayload.cBacklight = eBACKLIGHT_EVENT_NORMAL;
            sLedPayload.cTimeOut = 5;
            break;

        case eKEYPAD_LED_BACKLIGHT_10:
            sLedPayload.cBacklight = eBACKLIGHT_EVENT_NORMAL;
            sLedPayload.cTimeOut = 10;
            break;

        case eKEYPAD_LED_BACKLIGHT_20:
            sLedPayload.cBacklight = eBACKLIGHT_EVENT_NORMAL;
            sLedPayload.cTimeOut = 20;
            break;

        case eKEYPAD_LED_BACKLIGHT_30:
            sLedPayload.cBacklight = eBACKLIGHT_EVENT_NORMAL;
            sLedPayload.cTimeOut = 30;
            break;

        case eKEYPAD_LED_ALWAYSON:
            sLedPayload.cBacklight = eBACKLIGHT_EVENT_ALWAYS_ON;
            sLedPayload.cTimeOut = 0xFF;
            break;

        case eKEYPAD_LED_ALWAYSOFF:
            sLedPayload.cBacklight = eBACKLIGHT_EVENT_ALWAYS_OFF;
            sLedPayload.cTimeOut = 0;
            break;

        default:
            break;
    }

    sLedPayload.cCustomID = cCustomer_ID;

    utilHost_EventWrite(eCMD_MODULE_LED, eMSG_TYPE_UART_1, eLED_MSG_BACKLIGHT_KEYPAD, BACKLIGHT_LED_SIZE, (UINT8*)&sLedPayload, FALSE);
    //utilHost_SystemSet(eCMD_MODULE_LED, eLED_MSG_BACKLIGHT_KEYPAD, BACKLIGHT_LED_SIZE, (UINT8*)&sLedPayload);


//    dvMCUEventWrite(eCMD_MODULE_LED, eLED_MSG_BACKLIGHT_KEYPAD, BACKLIGHT_LED_SIZE,(UINT8*)&sLedPayload, FALSE);
}

// ==============================================================================
// FUNCTION NAME: halLED_StatueLEDStateSet
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
void halLED_StatueLEDStateSet(eLED_STATUS eStatus)
{
#ifdef LED_CONTROL
    (void)eStatus;
#else
    UINT8 cData = (UINT8)eStatus;
    utilHost_SystemSet(eCMD_MODULE_LED, eLED_MSG_LEDSTATE, sizeof(UINT8),&cData);
#endif /* PROJECTOR_2K */
}

// ==============================================================================
// FUNCTION NAME: halLED_StatusLedBehaviorSet
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
void halLED_StatusLedBehaviorSet(eLED_BEHAVIOR eLED_Behavior)
{
    utilHost_EventWrite(eCMD_MODULE_LED, eMSG_TYPE_UART_1, eLED_MSG_STATUS_LED_BEHAVIOR, sizeof(UINT8), (UINT8*)&eLED_Behavior, FALSE);

    //utilHost_SystemSet(eCMD_MODULE_LED, eLED_MSG_STATUS_LED_BEHAVIOR, sizeof(UINT8), (UINT8*)&eLED_Behavior);
}

// ==============================================================================
// FUNCTION NAME: halLED_SingleKeyLedSet
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
void halLED_SingleKeyLedSet(UINT32 dwData)
{
#ifdef LED_CONTROL
    (void)dwData;
#else
    utilHost_SystemSet(eCMD_MODULE_LED, eLED_MSG_SINGLE_KEYPAD, sizeof(UINT32), (UINT8*)&dwData);
#endif /* PROJECTOR_2K */
}

// ==============================================================================
// FUNCTION NAME: halLED_ShutterLedSet
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
void halLED_ShutterLedSet(BOOL bEnable) //A70LH_Larry_0043
{
    utilHost_EventWrite(eCMD_MODULE_LED, eMSG_TYPE_UART_1, eLED_MSG_SHUTTER_LED, sizeof(UINT8), (UINT8*)&bEnable, FALSE);
    //utilHost_SystemSet(eCMD_MODULE_LED, eLED_MSG_SHUTTER_LED, sizeof(UINT8), (UINT8*)&bEnable);  //G100_Owen_0019

}

// ==============================================================================
// FUNCTION NAME: halLED_VersionGet
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
eRESULT halLED_VersionGet(UINT8 *pucData)
{
    return dvLPCIOExp_Version_Get(pucData);
}

// ==============================================================================
// FUNCTION NAME: halLED_PwrLedSet
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
void halLED_PwrLedSet(UINT8 ucEnable)
{
    utilHost_EventWrite(eCMD_MODULE_LED, eMSG_TYPE_UART_1, eLED_MSG_BACKLIGHT_PWRKEY, sizeof(UINT8), (UINT8*)&ucEnable, FALSE);
    //utilHost_SystemSet(eCMD_MODULE_LED, eLED_MSG_BACKLIGHT_PWRKEY, sizeof(UINT8), (UINT8*)&ucEnable);
}


