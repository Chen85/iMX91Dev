// ==============================================================================
// FILE NAME: APPLEDPROC.C
// DESCRIPTION:
//
//
// modification history
// --------------------
// 27/05/2013, Leohong written
// 02/10/2013 Larry fix  //A70_Larry_0038
// --------------------
// ==============================================================================


#include "appLedProcAPI.h"
#include "appPoll.h"
#include "appDataMgr.h"

#include "halLEDCtrlAPI.h"

#include "utilDbgMsg.h"
#include "utilQueueAPI.h"
#include "utilMathAPI.h"


typedef struct
{
    eLED_BEHAVIOR       eLED_Behavior;
    eLED_BEHAVIOR       eLED_LastBehavior;

    eLED_STATUS         eLED_Status; //ALWAYS ON, ALWAYS OFF, WARNING/ERROR

    eKEYPAD_BACKLIGHT   eKeyPadLight;
    eLED_CUSTOMID_TYPE  eCustomType;

    eLED_SHUTTER        eShutter;
    UINT8               ucPowerKey; //A70LV_Larry_0314

    //eLED_EVENT          eLED_Event;

    UINT32              ulSingleKeyPadLed;
    UINT16              uiKeypadLedLightTime;

}sLED_CFG;

static sLED_CFG m_sLEDCfg;

#define MAX_LED_PKT 4

static sQUEUE       m_sLEDRingBuffer;
static UINT8        m_asInputBuffer[MAX_LED_PKT];

// ==============================================================================
// FUNCTION NAME: palLedProc_WarningEvent
// DESCRIPTION:
//
//
// Params:
// eLED_STATUS eStatus:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/04/07, Larry Create
// --------------------
// ==============================================================================
static BOOL palLedProc_WarningEventGet(eLED_BEHAVIOR eBehavior)
{
    if(eBehavior == eLED_STATUS_POWERON ||
       eBehavior == eLED_STATUS_WARMUP ||
       eBehavior == eLED_STATUS_COOLING ||
       eBehavior == eLED_STATUS_CWFAIL_COOLING ||
       eBehavior == eLED_STATUS_FANLOCK_COOLING ||
       eBehavior == eLED_STATUS_OVER_TEMP_COOLING ||
       eBehavior == eLED_STATUS_WARNNING ||
       eBehavior == eLED_STATUS_NTC_DETECTED ||
       eBehavior == eLED_STATUS_DA_WARRING ||
       eBehavior == eLED_STATUS_ROTATION_FAIL ||
       eBehavior == eLED_STATUS_POWER_LOST ||
       eBehavior == eLED_STATUS_LD_OVERTEMP ||
       eBehavior == eLED_STATUS_LD_SHUNTCIRCUIT ||
       eBehavior == eLED_STATUS_PUMP_FAIL ||
       eBehavior == eLED_STATUS_PUMP_WARNNING ||
       eBehavior == eLED_STATUS_FAN_STALL ||
       eBehavior == eLED_STATUS_LD_OCP)
    {
        return TRUE;
    }
    return FALSE;
}

// ==============================================================================
// FUNCTION NAME: palLedProc_KeepWarningEventGet
// DESCRIPTION:
//
//
// Params:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/04/11, Larry Create
// --------------------
// ==============================================================================
static BOOL palLedProc_KeepWarningEventGet(eLED_BEHAVIOR eBehavior)
{
    if(eBehavior == eLED_STATUS_LAMP_OUT_OF_LIFE ||
       eBehavior == eLED_STATUS_COOLING ||
       eBehavior == eLED_STATUS_OVER_TEMP_COOLING ||
       eBehavior == eLED_STATUS_FANLOCK_COOLING ||
       eBehavior == eLED_STATUS_LMPFAIL_COOLING ||
       eBehavior == eLED_STATUS_CWFAIL_COOLING ||
       eBehavior == eLED_STATUS_WARNNING ||
       eBehavior == eLED_STATUS_NTC_DETECTED ||
       eBehavior == eLED_STATUS_DA_WARRING ||
       eBehavior == eLED_STATUS_ROTATION_FAIL ||
       eBehavior == eLED_STATUS_POWER_LOST ||
       eBehavior == eLED_STATUS_LD_OVERTEMP ||
       eBehavior == eLED_STATUS_LD_SHUNTCIRCUIT ||
       eBehavior == eLED_STATUS_PUMP_FAIL ||
       eBehavior == eLED_STATUS_PUMP_WARNNING ||
       eBehavior == eLED_STATUS_FAN_STALL ||
       eBehavior == eLED_STATUS_LD_OCP)
    {
        return TRUE;
    }
    return FALSE;
}

// ==============================================================================
// FUNCTION NAME: palLedProc_StatusLedColorGet
// DESCRIPTION:
//
//
// Params:
// void:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/04/11, Larry Create
// --------------------
// ==============================================================================
UINT8 palLedProc_StatusLedColorGet(void)
{
    UINT8 cLED = 0;

    UINT8 cBehavior = 0;//halLedCtrl_Status_Behavior_Get();

    switch(m_sLEDCfg.eLED_Behavior)
    {
        case eLED_STATUS_POWERON:
        case eLED_STATUS_WARMUP:
        case eLED_STATUS_LAMPIGNITE:
        case eLED_STATUS_NORMAL:
        case eLED_STATUS_LAMP_OUT_OF_LIFE:
        case eLED_STATUS_OVER_TEMP_COOLING:
        case eLED_STATUS_FANLOCK_COOLING:
        case eLED_STATUS_LMPFAIL_COOLING:
        case eLED_STATUS_BALLASTFAIL_COOLING:
        case eLED_STATUS_CWFAIL_COOLING:
        case eLED_STATUS_SHUTTER_ON:
        case eLED_STATUS_SHUTTER_OFF:
        case eLED_STATUS_NORMAL_BURNIN_OFF:
        case eLED_STATUS_IR_BROADCAST:
        case eLED_STATUS_IR_DIRECT:
        case eLED_STATUS_LED_ALWAYS_OFF:
        case eLED_STATUS_WARNNING:
        case eLED_STATUS_NTC_DETECTED:
        case eLED_STATUS_DA_WARRING:
        case eLED_STATUS_ROTATION_FAIL:
        case eLED_STATUS_POWER_LOST:
        case eLED_STATUS_FACTORY_RESET_ON:
        case eLED_STATUS_LD_OVERTEMP:
        case eLED_STATUS_LD_SHUNTCIRCUIT:
        case eLED_STATUS_PUMP_FAIL:
        case eLED_STATUS_PUMP_WARNNING:
        case eLED_STATUS_FAN_STALL:
        case eLED_STATUS_LD_OCP:
            default:
            {
                switch(cBehavior)
                {
                    case 0: //eLED_BEHAVIOR_GOOD:
                        cLED = eSTATUS_LED_COLOR_GREEN;
                        break;

                    case 2: //eLED_BEHAVIOR_WARNING:
                        cLED = eSTATUS_LED_COLOR_YELLOW;
                        break;

                    case 3: //eLED_BEHAVIOR_ERROR:
                        cLED = eSTATUS_LED_COLOR_RED;
                        break;

                    default:
                        cLED = eSTATUS_LED_COLOR_NULL;
                        break;
                }
            }
            break;


        case eLED_STATUS_POWEROFF:
        case eLED_STATUS_COOLING:
            {
                switch(cBehavior)
                {
                    case 0: //eLED_BEHAVIOR_GOOD:
                        cLED = eSTATUS_LED_COLOR_BLUE;
                        break;

                    case 2: //eLED_BEHAVIOR_WARNING:
                        cLED = eSTATUS_LED_COLOR_YELLOW;
                        break;

                    case 3: //eLED_BEHAVIOR_ERROR:
                        cLED = eSTATUS_LED_COLOR_RED;
                        break;

                    default:
                        cLED = eSTATUS_LED_COLOR_NULL;
                        break;
                }
            }
            break;

        case eLED_STATUS_FW_UPGRAGE:
            cLED = eSTATUS_LED_COLOR_WHITE;
            break;
    }

    return cLED;
}

// ==============================================================================
// FUNCTION NAME: APPKEYPADLED_SET
// DESCRIPTION:
//
//
// Params:
// DWORD dwLedMask:
//
// Returns:
//
//
// modification history
// --------------------
// 2014/04/02, Leo Create
// 21/08/2013 Larry fix //A70_Larry_0027
// --------------------
// ==============================================================================
static eRESULT palLedProc_Keypad_LED_Active(UINT32 dwLedMask)
{
    UINT8  cCount = 0;
    UINT32 dwMask = 0x01;
    UINT8 acLedState[eLED_LIST_RED_OSD + 1];
    eLED_LIST aeLedList[eLED_LIST_RED_OSD + 1];

    for(cCount = 0; cCount < eLED_LIST_RED_OSD; cCount++)
    {
        dwMask = 1 << cCount;
        aeLedList[cCount] = (eLED_LIST)cCount;
        acLedState[cCount] = 1;
        acLedState[cCount] = !GETBIT(dwLedMask, dwMask);

    } // End of For

    aeLedList[cCount] = eLED_LIST_NUMBERS;

    return rcSUCCESS;   //halLedCtrl_MultiLed_Set(aeLedList, acLedState);
}

// ==============================================================================
// FUNCTION NAME: APPKEYPADLED_PWR_SET
// DESCRIPTION:
//
//
// Params:
// BYTE cOn:
//
// Returns:
//
//
// modification history
// --------------------
// 2014/04/17, Leo Create
// --------------------
// ==============================================================================
static eRESULT palLedProc_PWR_LED_Set(UINT8 cOn)
{
#ifdef LED_CONTROL
    if(cOn == 1)//ON
    {
        halLedCtrl_Led_Set(eLED_LIST_BLUE_PWR, FALSE);
        halLedCtrl_Led_Set(eLED_LIST_RED_PWR, FALSE);
    }
    else if(cOn == 2) //Init
    {
        halLedCtrl_Led_Set(eLED_LIST_BLUE_PWR, TRUE);
        halLedCtrl_Led_Set(eLED_LIST_RED_PWR, FALSE);
    }
    else // OFF
    {
        halLedCtrl_Led_Set(eLED_LIST_BLUE_PWR, TRUE);
        halLedCtrl_Led_Set(eLED_LIST_RED_PWR, TRUE);
    }
#endif /* LED_CONTROL */

    return rcSUCCESS;
}



// ==============================================================================
// FUNCTION NAME: palLedProc_BacklightKeypad_Set
// DESCRIPTION:
//
//
// Params:
// eKEYPAD_BACKLIGHT eBackLightLED:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/10/28, Larry Create
// --------------------
// ==============================================================================
void palLedProc_BacklightKeypad_Set(eKEYPAD_BACKLIGHT eBackLightLED)
{
    m_sLEDCfg.eKeyPadLight = eBackLightLED;

    switch(m_sLEDCfg.eKeyPadLight)
    {
        case eKEYPAD_LED_BACKLIGHT_05:
            m_sLEDCfg.uiKeypadLedLightTime = 50;
            break;

        case eKEYPAD_LED_BACKLIGHT_10:
            m_sLEDCfg.uiKeypadLedLightTime = 100;
            break;

        case eKEYPAD_LED_BACKLIGHT_20:
            m_sLEDCfg.uiKeypadLedLightTime = 200;
            break;

        case eKEYPAD_LED_BACKLIGHT_30:
            m_sLEDCfg.uiKeypadLedLightTime = 300;
            break;

        case eKEYPAD_LED_ALWAYSON:
            m_sLEDCfg.uiKeypadLedLightTime = 0xFFFF;
            break;

        case eKEYPAD_LED_ALWAYSOFF:
            break;

        default:
            break;
    }

    palLedProc_EventSet(eLED_EVENT_BACKLIGHT_TIMER_UPDATE); //G100_Owen_0059
}

// ==============================================================================
// FUNCTION NAME: palLedProc_BacklightKeypad_ACtive
// DESCRIPTION:
//
//
// Params:
// void:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/10/28, Larry Create
// --------------------
// ==============================================================================
static void palLedProc_BacklightKeypad_Active(eKEYPAD_BACKLIGHT eBackLightLED)
{
#ifdef LED_CONTROL
    UINT32 dwLED = 0;
    UINT16 uiKeypadLedLightTime = 0;

    m_sLEDCfg.eKeyPadLight = eBackLightLED;

    switch(m_sLEDCfg.eKeyPadLight)
    {
        case eKEYPAD_LED_BACKLIGHT_05:
            dwLED = ~dwLED;
            uiKeypadLedLightTime = 50;
            break;

        case eKEYPAD_LED_BACKLIGHT_10:
            dwLED = ~dwLED;
            uiKeypadLedLightTime = 100;
            break;

        case eKEYPAD_LED_BACKLIGHT_20:
            dwLED = ~dwLED;
            uiKeypadLedLightTime = 200;
            break;

        case eKEYPAD_LED_BACKLIGHT_30:
            dwLED = ~dwLED;
            uiKeypadLedLightTime = 300;
            break;

        case eKEYPAD_LED_ALWAYSON:
            uiKeypadLedLightTime = 0xFFFF;
            dwLED = ~dwLED;
            break;

        case eKEYPAD_LED_ALWAYSOFF:
            break;

        default:
            break;
    }

    if(m_sLEDCfg.eKeyPadLight == eKEYPAD_LED_ALWAYSOFF) //A70LV_Larry_0314
    {
        if(m_sLEDCfg.ucPowerKey != 0)
        {
            m_sLEDCfg.ucPowerKey = 0;
            palLedProc_PWR_LED_Set(m_sLEDCfg.ucPowerKey);
        }
    }
    else
    {
        if(m_sLEDCfg.ucPowerKey != 1)
        {
            m_sLEDCfg.ucPowerKey = 1;
            palLedProc_PWR_LED_Set(m_sLEDCfg.ucPowerKey);
        }
    }

    if(uiKeypadLedLightTime == 0)
    {
        dwLED = 0;
    }

    m_sLEDCfg.uiKeypadLedLightTime = uiKeypadLedLightTime;

    palLedProc_Keypad_LED_Active(dwLED);
#else
    (void)eBackLightLED;
#endif /* PROJECTOR_2K */
}

// ==============================================================================
// FUNCTION NAME: palLedProc_CustomType_Set
// DESCRIPTION:
//
//
// Params:
// eLED_CUSTOMID_TYPE  eCustomType:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/10/28, Larry Create
// --------------------
// ==============================================================================
void palLedProc_CustomType_Set(eLED_CUSTOMID_TYPE  eCustomType)
{
    m_sLEDCfg.eCustomType = eCustomType;
}


// ==============================================================================
// FUNCTION NAME: palLedProc_Shutter_Led_Set
// DESCRIPTION:
//
//
// Params:
// BYTE cColor:
//
// Returns:
//
//
// Modification History
// --------------------
// 2014/09/03, Larry Create
// --------------------
// ==============================================================================
void palLedProc_Shutter_LED_Set(eLED_SHUTTER eLedShutter)
{
    eLED_EVENT  eLED_Event = eLED_EVENT_SHUTTER_LED;

    m_sLEDCfg.eShutter = eLedShutter;

//    utilQueueWrite(&m_sLEDRingBuffer,(UINT8 *)&eLED_Event);   //G100_Owen_0019
    palLedProc_EventSet(eLED_Event);  //G100_Owen_0019
}

// ==============================================================================
// FUNCTION NAME: palLedProc_Shutter_Led_Get
// DESCRIPTION:
//
//
// Params:
// void:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/05/05, Larry Create
// --------------------
// ==============================================================================
UINT8 palLedProc_Shutter_LED_Get(void)
{
    UINT8 cShutterLED = (UINT8)eLED_SHUTTER_OFF;

    switch(m_sLEDCfg.eLED_Status)
    {
        case eSTATUSLED_ALWAYSON:
            cShutterLED = (UINT8)m_sLEDCfg.eShutter;
            break;

        case eSTATUSLED_ALWAYSOFF:
            break;

        case eSTATUSLED_WARNINGERROR:
            if(m_sLEDCfg.eShutter == eLED_SHUTTER_FAIL)
            {
                cShutterLED = m_sLEDCfg.eShutter;
            }
            break;

        default:
            break;
    }

    return cShutterLED;
}

// ==============================================================================
// FUNCTION NAME: palLedProc_Shutter_Led_Active
// DESCRIPTION:
//
//
// Params:
// eLED_SHUTTER eLedShutter:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/10/28, Larry Create
// --------------------
// ==============================================================================
static void palLedProc_Shutter_LED_Active(eLED_SHUTTER eLedShutter)
{
#ifdef LED_CONTROL
    switch(m_sLEDCfg.eLED_Status)
    {
        case eSTATUSLED_ALWAYSON:
            switch(eLedShutter)
            {
                case eLED_SHUTTER_ON:
                    halLedCtrl_Led_Set(eLED_LIST_STATUS_SHUTTER_RED,  TRUE);
                    halLedCtrl_Led_Set(eLED_LIST_STATUS_SHUTTER_BLUE, TRUE);
                    break;

                case eLED_SHUTTER_OFF:
                    halLedCtrl_Led_Set(eLED_LIST_STATUS_SHUTTER_RED,  FALSE);
                    halLedCtrl_Led_Set(eLED_LIST_STATUS_SHUTTER_BLUE, FALSE);
                    break;

                case eLED_SHUTTER_FAIL:
                    halLedCtrl_Led_Set(eLED_LIST_STATUS_SHUTTER_RED,  TRUE);
                    halLedCtrl_Led_Set(eLED_LIST_STATUS_SHUTTER_BLUE, FALSE);
                    break;

                default:
                    break;
            }

            break;

        case eSTATUSLED_ALWAYSOFF:
            halLedCtrl_Led_Set(eLED_LIST_STATUS_SHUTTER_RED,  FALSE);
            halLedCtrl_Led_Set(eLED_LIST_STATUS_SHUTTER_BLUE, FALSE);
            break;

        case eSTATUSLED_WARNINGERROR:
            if(eLedShutter == eLED_SHUTTER_FAIL)
            {
                halLedCtrl_Led_Set(eLED_LIST_STATUS_SHUTTER_RED,  TRUE);
                halLedCtrl_Led_Set(eLED_LIST_STATUS_SHUTTER_BLUE, FALSE);
            }
            else
            {
                halLedCtrl_Led_Set(eLED_LIST_STATUS_SHUTTER_RED,  FALSE);
                halLedCtrl_Led_Set(eLED_LIST_STATUS_SHUTTER_BLUE, FALSE);
            }

            break;

        default:
            break;
    }
#else
    (void)eLedShutter;
#endif /* PROJECTOR_2K */
}

// ==============================================================================
// FUNCTION NAME: palLedProc_LED_Behavior_Get
// DESCRIPTION:
//
//
// Params:
// void:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/10/28, Larry Create
// --------------------
// ==============================================================================
 UINT8 palLedProc_LED_Behavior_Get(void)
{
    //if(halLedCtrl_Status_Behavior_Get() != 0)
    //{
    //    return  (UINT8)m_sLEDCfg.eLED_LastBehavior;
    //}

    return  (UINT8)m_sLEDCfg.eLED_Behavior;
}

// ==============================================================================
// FUNCTION NAME: palLedProc_LED_Behavior_Active
// DESCRIPTION:
//
//
// Params:
// eLED_BEHAVIOR eBehavior:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/10/28, Larry Create
// --------------------
// ==============================================================================
static void palLedProc_LED_Behavior_Active(eLED_BEHAVIOR eBehavior)
{
#ifdef LED_CONTROL
    switch(eBehavior)
    {
        case eLED_STATUS_WARMUP:
            if(m_sLEDCfg.eKeyPadLight == eKEYPAD_LED_ALWAYSOFF)
            {
                m_sLEDCfg.ucPowerKey = 0;
                palLedProc_PWR_LED_Set(m_sLEDCfg.ucPowerKey);
            }
            else
            {
                m_sLEDCfg.ucPowerKey = 1;
                palLedProc_PWR_LED_Set(m_sLEDCfg.ucPowerKey); //A70LH_Larry_0039
                palLedProc_BacklightKeypad_Active(m_sLEDCfg.eKeyPadLight);
            }
            break;

        case eLED_STATUS_POWEROFF:
            if(m_sLEDCfg.eKeyPadLight == eKEYPAD_LED_ALWAYSOFF)
            {
                m_sLEDCfg.ucPowerKey = 0;
                palLedProc_PWR_LED_Set(m_sLEDCfg.ucPowerKey);
            }
            else
            {
                m_sLEDCfg.ucPowerKey = 2;
                palLedProc_PWR_LED_Set(m_sLEDCfg.ucPowerKey);
            }

            palLedProc_Shutter_LED_Active(eLED_SHUTTER_OFF);
            break;

        case eLED_STATUS_COOLING:
            palLedProc_BacklightKeypad_Active(m_sLEDCfg.eKeyPadLight);
            break;

        case eLED_STATUS_NORMAL:
            palLedProc_BacklightKeypad_Active(m_sLEDCfg.eKeyPadLight);
            break;

        default:
            break;
    }

    switch(m_sLEDCfg.eLED_Status)
    {
        case eSTATUSLED_ALWAYSON:
            halLedCtrl_Status_Led_Set(eBehavior);
            break;

        case eSTATUSLED_ALWAYSOFF:
            if(eBehavior == eLED_STATUS_POWERON ||
               eBehavior == eLED_STATUS_WARMUP ||
               eBehavior == eLED_STATUS_COOLING)
            {
                halLedCtrl_Status_Led_Set(eBehavior);
            }
            else
            {
                halLedCtrl_Status_Led_Set(eLED_STATUS_LED_ALWAYS_OFF);
            }

            break;

        case eSTATUSLED_WARNINGERROR:
            if(palLedProc_WarningEventGet(eBehavior))
            {
                halLedCtrl_Status_Led_Set(eBehavior);
            }
            else
            {
                if((m_sLEDCfg.eLED_Behavior == eLED_STATUS_POWEROFF) &&
                   (halLedCtrl_Status_Behavior_Get() != 0)) //0 = eLED_BEHAVIOR_GOOD
                {
                    halLedCtrl_Status_Led_Set(eLED_STATUS_POWEROFF);
                }
                else
                {
                    halLedCtrl_Status_Led_Set(eLED_STATUS_LED_ALWAYS_OFF);
                }
            }

            break;

        default:
            break;
    }

    if(TRUE == palLedProc_KeepWarningEventGet(eBehavior)) //A35LS_Larry_0085
    {
        m_sLEDCfg.eLED_LastBehavior = eBehavior;
    }
#else
    (void)eBehavior;
    halLED_StatueLEDStateSet(m_sLEDCfg.eLED_Status);
#endif /* PROJECTOR_2K */
}




// ==============================================================================
// FUNCTION NAME: palLedProc_SingleLed_Set
// DESCRIPTION:
//
//
// Params:
// DWORD dwData:
//
// Returns:
//
//
// Modification History
// --------------------
// 2015/04/07, Larry Create
// --------------------
// ==============================================================================
void palLedProc_SingleLed_Set(UINT32 ulSingleKeyPadLed)
{
    eLED_EVENT  eLED_Event = eLED_EVENT_KEYPAD_LED;

    m_sLEDCfg.ulSingleKeyPadLed = ulSingleKeyPadLed;
    m_sLEDCfg.uiKeypadLedLightTime = 10;

//    utilQueueWrite(&m_sLEDRingBuffer,(UINT8 *)&eLED_Event);   //G100_Owen_0019
}

// ==============================================================================
// FUNCTION NAME: palLedProc_LED_BehaviorSet
// DESCRIPTION:
//
//
// Params:
// eLED_STATUS eStatus:
//
// Returns:
//
//
// Modification History
// --------------------
// 2014/09/30, Larry Create
// --------------------
// ==============================================================================
void palLedProc_LED_Behavior_Set(eLED_BEHAVIOR eBehavior)
{
    eLED_EVENT  eLED_Event = eLED_EVENT_STATUS_LED;

    m_sLEDCfg.eLED_Behavior = eBehavior;
    palLedProc_LED_Behavior_Active(m_sLEDCfg.eLED_Behavior);

//    utilQueueWrite(&m_sLEDRingBuffer,(UINT8 *)&eLED_Event);   //G100_Owen_0019
    palLedProc_EventSet(eLED_Event);    //G100_Owen_0019
}

// ==============================================================================
// FUNCTION NAME: palLedProc_Status_Led_State_Set
// DESCRIPTION:
//
//
// Params:
// eLED_STATUS eStatus:
//
// Returns:
//
//
// Modification History
// --------------------
// 2014/09/30, Larry Create
// --------------------
// ==============================================================================
void palLedProc_Status_LED_State_Set(eLED_STATUS eStatus)
{
    eLED_EVENT  eLED_Event = eLED_EVENT_STATUS_LED;

    m_sLEDCfg.eLED_Status = eStatus;

    palLedProc_LED_Behavior_Active(m_sLEDCfg.eLED_Behavior); //A70LV_Larry_0402

//    utilQueueWrite(&m_sLEDRingBuffer,(UINT8 *)&eLED_Event);   //G100_Owen_0019
}

#if 0
// ==============================================================================
// FUNCTION NAME: palLedProc_SingleLed_Active
// DESCRIPTION:
//
//
// Params:
// UINT32 ulSingleKeyPadLed:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/10/28, Larry Create
// --------------------
// ==============================================================================
static void palLedProc_SingleLed_Active(UINT32 ulSingleKeyPadLed)
{
    palLedProc_Keypad_LED_Active(ulSingleKeyPadLed);
}
#endif /* 0 */

// ==============================================================================
// FUNCTION NAME: palLedProc_SatusLED_IR_Active
// DESCRIPTION:
//
//
// Params:
// void:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/10/28, Larry Create
// --------------------
// ==============================================================================
static void palLedProc_SatusLED_IR_Active(void)
{
    if(m_sLEDCfg.eLED_Status == eSTATUSLED_ALWAYSOFF)
    {
        return;
    }

    if(m_sLEDCfg.eLED_Behavior == eLED_STATUS_WARMUP)
    {
        return;
    }

    switch(m_sLEDCfg.eCustomType)
    {
        case eLED_CUSTOMID_TYPE_BROADCAST:
            //halLedCtrl_Status_Led_Set(eLED_STATUS_IR_BROADCAST);
            break;

        case eLED_CUSTOMID_TYPE_DIRECT:
            //halLedCtrl_Status_Led_Set(eLED_STATUS_IR_DIRECT);
            break;

        case eLED_CUSTOMID_TYPE_NONE:
        case eLED_CUSTOMID_TYPE_NUMBERS:
            break;
    }
}
// ==============================================================================
// FUNCTION NAME: palLedProc_INIT
// DESCRIPTION:
//
//
// Params:
// void:
//
// Returns:
//
//
// modification history
// --------------------
// 2014/03/21, Leo Create
// 2014/09/03, A70_Larry_0029 fix
// --------------------
// ==============================================================================
void palLedProc_Init(void)
{
    //UINT8 cBurnInEnable = 0;

    //halLedCtrl_Init();

    LOG_MSG(db_APP_POLL, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);

//    utilQueueInitial(&m_sLEDRingBuffer, sizeof(UINT8), MAX_LED_PKT,(UINT8*)m_asInputBuffer);  //G100_Owen_0019


    m_sLEDCfg.eLED_Behavior = eLED_STATUS_WARMUP;
    m_sLEDCfg.eLED_LastBehavior = eLED_STATUS_WARMUP;

    m_sLEDCfg.eLED_Status = eSTATUSLED_ALWAYSON; //ALWAYS ON, ALWAYS OFF, WARNING/ERROR
    m_sLEDCfg.eKeyPadLight = eKEYPAD_LED_BACKLIGHT_05;

    //Backlight Preferences //A70LV_Larry_0314
    palDataMgr_Data_Access(edcKEYPAD_BACKLIGHT, edaREAD, &m_sLEDCfg.eKeyPadLight);
    palDataMgr_Data_Access(edcSTATUS_LED, edaREAD, &m_sLEDCfg.eLED_Status);

    m_sLEDCfg.eCustomType = eLED_CUSTOMID_TYPE_BROADCAST;
    m_sLEDCfg.eShutter = eLED_SHUTTER_OFF;
    //m_sLEDCfg.eLED_Event = eLED_EVENT_IDEL;
    m_sLEDCfg.ulSingleKeyPadLed = 0;
    m_sLEDCfg.uiKeypadLedLightTime = 0;
    m_sLEDCfg.ucPowerKey = 0; //A70LV_Larry_0314

    LOG_MSG(db_APP_POLL, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);

    //palLedProc_Shutter_LED_Active(m_sLEDCfg.eShutter);
    //palLedProc_LED_Behavior_Active(m_sLEDCfg.eLED_Behavior);
}

// ==============================================================================
// FUNCTION NAME: palLedProc_Poll
// DESCRIPTION:
//
//
// Params:
// void:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/10/28, Larry Create
// --------------------
// ==============================================================================
INT16 palLedProc_Poll(UINT16 uiTick)
{
    eLED_EVENT  eLED_Event = eLED_EVENT_IDEL;
#ifdef PROJECTOR_2K
    if(utilQueueRead(&m_sLEDRingBuffer, (UINT8*)&eLED_Event))
    {
        LOG_MSG(db_APP_INPUTKEY, "\r\n eLED_Event = %d", eLED_Event);

        switch(eLED_Event)
        {
            case eLED_EVENT_STATUS_LED:
                break;

            case eLED_EVENT_SHUTTER_LED:
                palLedProc_Shutter_LED_Active(m_sLEDCfg.eShutter);
                break;

            case eLED_EVENT_KEYPAD_LED:
                palLedProc_Keypad_LED_Active(m_sLEDCfg.ulSingleKeyPadLed);
                break;

            case eLED_EVENT_UPDATE:
                palLedProc_LED_Behavior_Active(m_sLEDCfg.eLED_Behavior);
                palLedProc_Shutter_LED_Active(m_sLEDCfg.eShutter);
                palLedProc_Keypad_LED_Active(m_sLEDCfg.ulSingleKeyPadLed);
                break;

            case eLED_EVENT_BACKLIGHT_ACTIVE:
                palLedProc_SatusLED_IR_Active();
                palLedProc_BacklightKeypad_Active(m_sLEDCfg.eKeyPadLight);
                break;

            case eLED_EVENT_BACKLIGHT_TIMER_UPDATE:
                palLedProc_BacklightKeypad_Set(m_sLEDCfg.eKeyPadLight);
                break;

            default:
                break;
        }

        //m_sLEDCfg.eLED_Event = eLED_EVENT_IDEL;
    }

    if((m_sLEDCfg.eKeyPadLight != eKEYPAD_LED_ALWAYSOFF) && (m_sLEDCfg.uiKeypadLedLightTime != 0))
    {
       if(!((m_sLEDCfg.uiKeypadLedLightTime == 0xFFFF) || (m_sLEDCfg.eLED_Behavior == eLED_STATUS_COOLING || m_sLEDCfg.eLED_Behavior == eLED_STATUS_WARMUP)))
       {
            m_sLEDCfg.uiKeypadLedLightTime--;

            //if(m_sLEDCfg.uiKeypadLedLightTime%10 == 0)
            //{
                //LOG_MSG(db_APP_INPUTKEY, "\r\n KeypadLedLightTime = %d" ,m_sLEDCfg.uiKeypadLedLightTime/10);
            //}
            if(m_sLEDCfg.uiKeypadLedLightTime == 0)
            {
                palLedProc_Keypad_LED_Active(0);
            }
       }
    }
#else
    if(utilQueueRead(&m_sLEDRingBuffer, (UINT8 *)&eLED_Event))
    {
        //LOG_MSG(db_APP_INPUTKEY, "\r\n eLED_Event = %d" ,eLED_Event);

        switch(eLED_Event)
        {
            case eLED_EVENT_STATUS_LED:
                halLED_StatusLedBehaviorSet(m_sLEDCfg.eLED_Behavior);
                break;

            case eLED_EVENT_SHUTTER_LED:
                halLED_ShutterLedSet(m_sLEDCfg.eShutter);
                break;

            case eLED_EVENT_KEYPAD_LED:
                halLED_SingleKeyLedSet(m_sLEDCfg.ulSingleKeyPadLed);
                break;

            case eLED_EVENT_UPDATE:
                break;

            case eLED_EVENT_BACKLIGHT_ACTIVE:
                halLED_BacklightKeypadStatusSet(m_sLEDCfg.eKeyPadLight, m_sLEDCfg.eCustomType);
                break;

            case eLED_EVENT_BACKLIGHT_TIMER_UPDATE:
                halLED_BacklightKeypadStatusSet(m_sLEDCfg.eKeyPadLight, eLED_CUSTOMID_TYPE_NONE);
                break;

            default:
                break;
        }

    }
#endif /* PROJECTOR_2K */

    return LED_POLLING_PERIOD / POLL_PERIOD ;
}

// ==============================================================================
// FUNCTION NAME: palLedProc_EventSet
// DESCRIPTION:
//
//
// Params:
// eLED_EVENT eEvent:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/10/28, Larry Create
// --------------------
// ==============================================================================
void palLedProc_EventSet(eLED_EVENT eEvent)
{
//    utilQueueWrite(&m_sLEDRingBuffer, (UINT8 *)&eEvent);
    switch(eEvent)
    {
        case eLED_EVENT_STATUS_LED:
            halLED_StatusLedBehaviorSet(m_sLEDCfg.eLED_Behavior);
            break;

        case eLED_EVENT_SHUTTER_LED:
            halLED_ShutterLedSet(m_sLEDCfg.eShutter);
            break;

        case eLED_EVENT_KEYPAD_LED:
            halLED_SingleKeyLedSet(m_sLEDCfg.ulSingleKeyPadLed);
            break;

        case eLED_EVENT_UPDATE:
            break;

        case eLED_EVENT_BACKLIGHT_ACTIVE:
            halLED_BacklightKeypadStatusSet(m_sLEDCfg.eKeyPadLight, m_sLEDCfg.eCustomType);
            break;

        case eLED_EVENT_BACKLIGHT_TIMER_UPDATE:
            halLED_BacklightKeypadStatusSet(m_sLEDCfg.eKeyPadLight, eLED_CUSTOMID_TYPE_NONE);
            break;

        default:
            break;
    }

}

