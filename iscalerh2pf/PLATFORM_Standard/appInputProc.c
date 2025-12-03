#include "appInputProc.h"
#include "appInputProcCommon.h"  //H2PF_Simon_0087
#include "appMailBox.h"
#include "appPoll.h"
#include "appDataMgr.h"
#include "appSystem.h"
#include "appLedProcAPI.h"
//#include "appIapProcAPI.h"
//#include "appGui.h"
#include "appIllumination.h"
#include "appEnvironment.h" //A70LV_Larry_0082
#include "appDataPath.h" //A70LV_Larry_0111
#include "halGui.h" //A70LV_Larry_0001
#include "halMotorCtrlAPI.h"
#include "halKeyCodeList.h"
#include "halInputCtrlAPI.h"
#include "halFormatter.h"
#include "halFrontEndCtrlAPI.h"
//#include "halLDProc.h" //A70LV_Larry_0322
#include "halMCUCtrlAPI.h"
#include "halBoardCtrlAPI.h"

#include "utilDbgMsg.h"
#include "utilCounterAPI.h"
#include "utilCommon.h"
#include "utilQueueAPI.h"
#include "utilWarpDemo.h"		//G100_Doulas_0027
#include "utilBIST.h"

#include "opdCtrlAPI.h"

#include "palCoreVar.h"
#include "palGui.h"
#include "palMotorMgr.h"
#include "palLDMgr.h"   //HICC2_Doulas_0032
#include "palInputKeyMgr.h" //HICC2_Doulas_0054
#include "palSysCtrlMgr.h"
#include "palGeoAPI.h"

#define BIST_KeyCheck_OFF 0   //HICC2_Steven_0002 start
#define BIST_KeyCheck_STEP1 1
#define BIST_KeyCheck_STEP2 2
#define BIST_KeyCheck_STEP3 3
#define BIST_KeyCheck_STEP4 4
#define BIST_KeyCheck_STEP5 5
#define BIST_KeyCheck_OPEN  6
static UINT8 m_ucBIST_KeyStatus = BIST_KeyCheck_OFF; //HICC2_Steven_0002 end

extern sGUI_CALLBACK GuiCb;

#define KEY_BUFFER_SIZE (8)
static sQUEUE m_sInputRingBuffer;
static sKEY_DATA m_asInputBuffer[KEY_BUFFER_SIZE] = {0};
static UINT8 m_cLensMotorLock = 0; //A70LV_Larry_0129
static UINT8 m_cDisableKey = 0; //for upgrade
//static UINT8 m_cTestPattern = eCM_TEST_PATTERN_OFF; //A70LV_Larry_0196
static UINT8 m_cUSTTestPattern = 0;//ZU860_Energy_0009 //A35G2_BRC_Casper_0088


//static BOOL uHor_Flip = TRUE;
//static BOOL uVer_Flip = TRUE;

static BOOL m_bMotorLensEnale = FALSE; //A70LV_Larry_0219
static BOOL m_bMotorFocusEnale = FALSE; //A70LV_Larry_0219
static BOOL m_bMotorZoomEnale = FALSE; //A70LV_Larry_0219
static BOOL m_bMotorLensAPPEnable = FALSE; //HICC2_Casper_0030
static sLENSMEMORY        m_sLensMem = {0}; //R70PD_AC_0105 //R70KPD_VSC_Wesley_0003 //HICC2_Casper_0030

#ifdef OE_JIG
static UINT8 uPMode = 0;  //T100_Casper_0010
UINT16 uPWIndex;
UINT16 uFWIndex;
static UINT8 uTPattern = 0;
static BOOL bLD_OnOff = TRUE;
#endif

#ifdef OSD_LOCK_ENABLE
static UINT8 m_ucOsdUnlock_Procedure_Status = OSD_UNLOCK_PROCEDURE_STEP1; //A65_OPTOMA_Jerry_0005
#endif //OSD_LOCK_ENABLE

const char *m_sInputKeyString_0[] =  //A35G2_Simon_0075
{
	STRINGER(kePOWERON),
    STRINGER(keSTANDBY),
    STRINGER(keMUTE),

    STRINGER(keMENU),
    STRINGER(keUP),
    STRINGER(keDOWN),
    STRINGER(keLEFT),
    STRINGER(keRIGHT),
    STRINGER(keENTER),

    STRINGER(keFOCUSUP),
    STRINGER(keFOCUSDOWN),
    STRINGER(keZOOMIN),
    STRINGER(keZOOMOUT),
    STRINGER(keLENSHRIGHT),
    STRINGER(keLENSHLEFT),
    STRINGER(keLENSVUP),
    STRINGER(keLENSVDOWN),

    STRINGER(keHOTKEY),
    STRINGER(ke0),
    STRINGER(ke1),
    STRINGER(ke2),
    STRINGER(ke3),
    STRINGER(ke4),
    STRINGER(ke5),
    STRINGER(ke6),
    STRINGER(ke7),
    STRINGER(ke8),
    STRINGER(ke9),

    STRINGER(keLENPWR),
    STRINGER(keINFO),
    STRINGER(keISP_PWR),
    STRINGER(keGAMMA),
    STRINGER(keBRIGHT),
    STRINGER(keCONTRAST),
    STRINGER(kePIP),
    STRINGER(kePIP_SIZE),
    STRINGER(kePIP_LAYOUT),
    STRINGER(keTEST),
    STRINGER(keHKEYSTONE_LEFT),
    STRINGER(keHKEYSTONE_RIGHT),
    STRINGER(keVKEYSTONE_UP),
    STRINGER(keVKEYSTONE_DOWN),

    STRINGER(keISP),
    STRINGER(keOSD),
    STRINGER(keFOCUS),
    STRINGER(keINPUT),
    STRINGER(keAUTO),
    STRINGER(keSHUTTER),
    STRINGER(keLENS),
    STRINGER(keBACK),
    STRINGER(keHELP),
    STRINGER(keZOOM),
    STRINGER(kePRESETS),
    STRINGER(keID),
    STRINGER(keMODE),
    STRINGER(kePATTERN),
    STRINGER(keHOTKEY2),
    STRINGER(keSWAP),

    STRINGER(keAUDIO_MUTE),
    STRINGER(keVOLUME_UP),
    STRINGER(keVOLUME_DOWN),
    STRINGER(keGEOMETRIC),
    STRINGER(keBLEND),
    STRINGER(keSCALE),
    STRINGER(keALL),
};

#define KEY_STRING0_MAX  sizeof(m_sInputKeyString_0)/sizeof(m_sInputKeyString_0[0])

const char *m_sInputKeyString_1[] =
{
    STRINGER(keHOLD_HOTKEY),
    STRINGER(keHOLD_POWERON),
    STRINGER(keHOLD_STANDBY),
    STRINGER(keHOLD_MUTE),

    STRINGER(keHOLD_MENU),
    STRINGER(keHOLD_UP),
    STRINGER(keHOLD_DOWN),
    STRINGER(keHOLD_LEFT),
    STRINGER(keHOLD_RIGHT),
    STRINGER(keHOLD_ENTER),

    STRINGER(keHOLD_HKEYSTONE_LEFT),
    STRINGER(keHOLD_HKEYSTONE_RIGHT),
    STRINGER(keHOLD_VKEYSTONE_UP),
    STRINGER(keHOLD_VKEYSTONE_DOWN),

    STRINGER(keHOLD_FOCUSUP),
    STRINGER(keHOLD_FOCUSDOWN),
    STRINGER(keHOLD_ZOOMIN),
    STRINGER(keHOLD_ZOOMOUT),
    STRINGER(keHOLD_LENSHRIGHT),
    STRINGER(keHOLD_LENSHLEFT),
    STRINGER(keHOLD_LENSVUP),
    STRINGER(keHOLD_LENSVDOWN),
    STRINGER(keHOLD_SWAP),
    STRINGER(keHOLD_LAN_UPGRADE),
    STRINGER(keHOLD_FAN_CAL),
    STRINGER(keHOLD_SCALAR_NMI),
    STRINGER(keHOLD_UART_TO_NXP),
    STRINGER(keHOLD_RESET_POWER_ON),

    STRINGER(keHOLD_0),
    STRINGER(keHOLD_1),
    STRINGER(keHOLD_2),
    STRINGER(keHOLD_3),
    STRINGER(keHOLD_4),
    STRINGER(keHOLD_5),
    STRINGER(keHOLD_6),
    STRINGER(keHOLD_7),
    STRINGER(keHOLD_8),
    STRINGER(keHOLD_9),

    STRINGER(keHOLD_TEST),
    STRINGER(keHOLD_OSD),
    STRINGER(keHOLD_FOCUS),
    STRINGER(keHOLD_INPUT),
    STRINGER(keHOLD_AUTO),
    STRINGER(keHOLD_SHUTTER),
    STRINGER(keHOLD_LENS),
    STRINGER(keHOLD_BACK),
    STRINGER(keHOLD_HELP),
    STRINGER(keHOLD_ZOOM),

    STRINGER(keHOLD_PRESETS),
    STRINGER(keHOLD_ID),
    STRINGER(keHOLD_MODE),
    STRINGER(keHOLD_PATTERN),
    STRINGER(keHOLD_HOTKEY2),

    STRINGER(keHOLD_VOLUME_UP),
    STRINGER(keHOLD_VOLUME_DOWN),
    STRINGER(keHOLD_GAMMA),
    STRINGER(keHOLD_BRIGHT),
    STRINGER(keHOLD_CONTRAST),
    STRINGER(keHOLD_PIP),
    STRINGER(keHOLD_PIP_SIZE),
    STRINGER(keHOLD_PIP_LAYOUT),
    STRINGER(keHOLD_INFO),

};

#define KEY_STRING1_MAX  sizeof(m_sInputKeyString_1)/sizeof(m_sInputKeyString_1[0])

const char *m_sInputKeyString_2[] =
{
    STRINGER(keRELEASE_HOTKEY),
    STRINGER(keRELEASE_POWERON),
    STRINGER(keRELEASE_STANDBY),
    STRINGER(keRELEASE_MUTE),

    STRINGER(keRELEASE_MENU),
    STRINGER(keRELEASE_UP),
    STRINGER(keRELEASE_DOWN),
    STRINGER(keRELEASE_LEFT),
    STRINGER(keRELEASE_RIGHT),
    STRINGER(keRELEASE_ENTER),

    STRINGER(keRELEASE_HKEYSTONE_LEFT),
    STRINGER(keRELEASE_HKEYSTONE_RIGHT),
    STRINGER(keRELEASE_VKEYSTONE_UP),
    STRINGER(keRELEASE_VKEYSTONE_DOWN),

    STRINGER(keRELEASE_FOCUSUP),
    STRINGER(keRELEASE_FOCUSDOWN),
    STRINGER(keRELEASE_ZOOMIN),
    STRINGER(keRELEASE_ZOOMOUT),
    STRINGER(keRELEASE_LENSHRIGHT),
    STRINGER(keRELEASE_LENSHLEFT),
    STRINGER(keRELEASE_LENSVUP),
    STRINGER(keRELEASE_LENSVDOWN),
    STRINGER(keRELEASE_LAN_UPGRADE),
    STRINGER(keRELEASE_SCALAR_NMI),
    STRINGER(keRELEASE_UART_TO_NXP),
    STRINGER(keRELEASE_SWAP),

    STRINGER(keRELEASE_0),
    STRINGER(keRELEASE_1),
    STRINGER(keRELEASE_2),
    STRINGER(keRELEASE_3),
    STRINGER(keRELEASE_4),
    STRINGER(keRELEASE_5),
    STRINGER(keRELEASE_6),
    STRINGER(keRELEASE_7),
    STRINGER(keRELEASE_8),
    STRINGER(keRELEASE_9),

    STRINGER(keRELEASE_TEST),
    STRINGER(keRELEASE_OSD),
    STRINGER(keRELEASE_FOCUS),
    STRINGER(keRELEASE_INPUT),
    STRINGER(keRELEASE_AUTO),
    STRINGER(keRELEASE_SHUTTER),
    STRINGER(keRELEASE_LENS),
    STRINGER(keRELEASE_BACK),
    STRINGER(keRELEASE_HELP),
    STRINGER(keRELEASE_ZOOM),

    STRINGER(keRELEASE_PRESETS),
    STRINGER(keRELEASE_ID),
    STRINGER(keRELEASE_MODE),
    STRINGER(keRELEASE_PATTERN),
    STRINGER(keRELEASE_HOTKEY2),

    STRINGER(keRELEASE_VOLUME_UP),
    STRINGER(keRELEASE_VOLUME_DOWN),
    STRINGER(keRELEASE_GAMMA),
    STRINGER(keRELEASE_BRIGHT),
    STRINGER(keRELEASE_CONTRAST),
    STRINGER(keRELEASE_PIP),
    STRINGER(keRELEASE_PIP_SIZE),
    STRINGER(keRELEASE_PIP_LAYOUT),
    STRINGER(keRELEASE_INFO),
};

#define KEY_STRING2_MAX  sizeof(m_sInputKeyString_2)/sizeof(m_sInputKeyString_2[0])

const char *m_sInputKeyString_3[] =
{
    STRINGER(keLAN_UPGRADE),
    STRINGER(keSCALAR_NMI),
    STRINGER(keUART_TO_NXP),
    STRINGER(keRESET_POWER_ON),
    STRINGER(keDDP_UPGRADE),
};

#define KEY_STRING3_MAX  sizeof(m_sInputKeyString_3)/sizeof(m_sInputKeyString_3[0])

static void palInput_Motor_Process(eKEY_LIST ekey) //A70LV_Larry_0219
{
    switch(ekey)
    {

        case keFOCUSUP:
        case keFOCUSDOWN:
        case keHOLD_FOCUSUP:
        case keHOLD_FOCUSDOWN:
            utilCounterSet(eCOUNTER_TYPE_MSG_FOCUS, 5000);
            m_bMotorFocusEnale = TRUE;
			m_bMotorZoomEnale = FALSE;
			m_bMotorLensEnale = FALSE;
            break;

        case keZOOMIN:
        case keZOOMOUT:
        case keHOLD_ZOOMIN:
        case keHOLD_ZOOMOUT:
            utilCounterSet(eCOUNTER_TYPE_MSG_ZOOM, 5000);
            m_bMotorFocusEnale = FALSE;
			m_bMotorZoomEnale = TRUE;
			m_bMotorLensEnale = FALSE;
            m_bMotorLensAPPEnable = TRUE; //HICC2_Casper_0030
            break;

        case keLENSHRIGHT:
        case keLENSHLEFT:
        case keLENSVUP:
        case keLENSVDOWN:
        case keHOLD_LENSHRIGHT:
        case keHOLD_LENSHLEFT:
        case keHOLD_LENSVUP:
        case keHOLD_LENSVDOWN:
            utilCounterSet(eCOUNTER_TYPE_MSG_LENS, 5000);
			m_bMotorFocusEnale = FALSE;
			m_bMotorZoomEnale = FALSE;
            m_bMotorLensEnale = TRUE;
            m_bMotorLensAPPEnable = TRUE; //HICC2_Casper_0030
            break;

        default:
            m_bMotorFocusEnale = FALSE; //A70LV_Larry_0298
			m_bMotorZoomEnale = FALSE; //A70LV_Larry_0298
            m_bMotorLensEnale = FALSE; //A70LV_Larry_0298
            break;
    }

}

void palKeypadBacklighthandle(sKEY_DATA *psInputKey)
{
#if (0)
    UINT32 dwLED = 0;

    if(psInputKey->eKeyType == eKEYINPUT_KEYPAD)
    {
        switch(psInputKey->wKeyCode)
        {
            case keMENU:
                dwLED = eLED_LIST_MENU;
                break;

            case keLEFT:
                dwLED = eLED_LIST_LEFT;
                break;

            case keAUTO:
                dwLED = eLED_LIST_AUTO;
                break;

            case keUP:
                dwLED = eLED_LIST_UP;
                break;

            case keENTER:
                dwLED = eLED_LIST_ENTER;
                break;

            case keDOWN:
                dwLED = eLED_LIST_DOWN;
                break;

            case keINPUT:
                dwLED = eLED_LIST_INPUT;
                break;

            case keRIGHT:
                dwLED = eLED_LIST_RIGHT;
                break;

            case keBACK:
                dwLED = eLED_LIST_BACK;
                break;

            case keLENS:
                dwLED = eLED_LIST_LENS;
                break;

            case keMUTE:
                dwLED = eLED_LIST_MUTE;
                break;

            case keZOOM:
                dwLED = eLED_LIST_ZOOM;
                break;

            case keFOCUS:
                dwLED = eLED_LIST_FOCUS;
                break;

            case keOSD:
                dwLED = eLED_LIST_OSD;
                break;

            case keHELP:
                dwLED = eLED_LIST_HELP;
                break;

            default:
                dwLED = 0xFFFF;
                break;
         }
         palLedProc_SingleLed_Set(dwLED);
    }
#else
#ifdef CUSTOM_CHRISTIE
    if(psInputKey->eKeyEvent == eKEY_EVENT_PRESSED)//press
	{
		palLedProc_EventSet(eLED_EVENT_BACKLIGHT_ACTIVE);
	}
	else//hold on key event, release key
	{
		palLedProc_EventSet(eLED_EVENT_BACKLIGHT_TIMER_UPDATE);
	}
#else
    if(psInputKey->eKeyType == eKEYINPUT_KEYPAD)    //G100_Owen_0015
    {
        if(psInputKey->eKeyEvent == eKEY_EVENT_PRESSED)//press
        {
            palLedProc_EventSet(eLED_EVENT_BACKLIGHT_ACTIVE);
        }
        else//hold on key event, release key
        {
            palLedProc_EventSet(eLED_EVENT_BACKLIGHT_TIMER_UPDATE);
        }
    }
    else if(psInputKey->eKeyType == eKEYINPUT_IR)
    {
        if(psInputKey->eKeyEvent == eKEY_EVENT_PRESSED)
        {
            UINT8 ucShutterEnable = 0;
            palDataMgr_Data_Access(edcPICTURE_MUTE, edaREAD, &ucShutterEnable);

            if(ucShutterEnable == 0)
            {
                palLedProc_LED_Behavior_Set(eLED_STATUS_IR_BROADCAST);
            }
        }
    }
#endif /* CUSTOM_CHRISTIE */
#endif /* SMT_KEYPAD_TEST */
}

// ==============================================================================
// FUNCTION NAME: palInput_Uart_Switch_Process
// DESCRIPTION:
//
//
// Params:
// eKEY_LIST ekey:
//
// Returns:
//
//
// Modification History
// --------------------
// 2014/12/23, Leo Create
// --------------------
// ==============================================================================
static void palInput_Uart_Switch_Process(eKEY_LIST ekey)
{
	UINT8 ucIndex = 0;
    switch(ekey)
    {
        case keUART_TO_NXP:
            // Start Timer
            utilCounterSet(eCOUNTER_TYPE_HOTKEY, 5000);
            break;

        case keHOLD_UART_TO_NXP:

            // Timer out
            if(0 == utilCounterGet(eCOUNTER_TYPE_HOTKEY))
            {
                //LOG_MSG(eDB_MSK_LIST_ASSERT, "Uart to Nxp\r\n");
                //halBoard_UartSwitch_Set(eUART_SW_RS232_TO_NXP);                       /////wait review/////

                palDataMgr_Data_Access(edcUART_SWITCH, edaWRITE_THROUGH_WITH_ACTION, &ucIndex); //GuiCb.fpGui_DataCode_Value_SetCb(edcUART_SWITCH, edaWRITE_THROUGH_WITH_ACTION, 0);
                utilDbgMsg_Set(db_KEEP_UART_SW, 0);

                utilCounterSet(eCOUNTER_TYPE_HOTKEY, 500000);

                MS_SLEEP(10);
                LOG_MSG(db_ALWAYS, "Uart to Nxp\r\n");
            }

            break;

        case keRELEASE_UART_TO_NXP:
            // Clear Timer
            utilCounterSet(eCOUNTER_TYPE_HOTKEY, 0);
            break;

        default:
            break;
    } // End of Switch
}

// ==============================================================================
// FUNCTION NAME: APPINPUTPROCINIT
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
// 22/04/2013, Leohong Create
// --------------------
// ==============================================================================
eEXEC_CODE palInputProc_Init(void)
{
    //UINT16 uiCustomId = 0; //A70LV_Larry_0001
    UINT8  ucValue = 0;

    utilQueueInitial(&m_sInputRingBuffer, KEY_BUFFER_SIZE, sizeof(sKEY_DATA),(UINT8*)m_asInputBuffer);

    //palInputProc_Param_Get(eINPUT_PARAM_REMOTE_ID, &wCustomId);

    //halInputCtrl_IR_CustomId_Set(wCustomId); //A70LV_Larry_0001

    palInputKeyMgr_Init();
    palInputKeyMgr_Register_Callback(palInputProc_BufferInsert);
    // utilEvent_Send(eEVENT_LIST_WARM_UP, 0 ,NULL);
#if 0
    palDataMgr_Data_Access(edcTOP_IR, edaREAD, (void*)&ucValue); //A70LV_Larry_0023
    palDataMgr_Data_Access(edcTOP_IR, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&ucValue);

    palDataMgr_Data_Access(edcFRONT_IR, edaREAD, (void*)&ucValue); //A70LV_Larry_0023
    palDataMgr_Data_Access(edcFRONT_IR, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&ucValue);

    palDataMgr_Data_Access(edcHDBASET_IR, edaREAD, (void*)&ucValue); //A70LV_Larry_0023
    palDataMgr_Data_Access(edcHDBASET_IR, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&ucValue);

    palDataMgr_Data_Access(edcSERIAL_PORT_BAUD_RATE, edaREAD, (void*)&ucValue); //A70LV_Larry_0023
    palDataMgr_Data_Access(edcSERIAL_PORT_BAUD_RATE, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&ucValue);

    palDataMgr_Data_Access(edcSERIAL_PORT_ECHO, edaREAD, (void*)&ucValue);
    palDataMgr_Data_Access(edcSERIAL_PORT_ECHO, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&ucValue);

    palDataMgr_Data_Access(edcPROJECTOR_ADDRESS, edaREAD, (void*)&ucValue); //A70LV_Larry_0060
    palDataMgr_Data_Access(edcPROJECTOR_ADDRESS, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&ucValue);
#endif
    palDataMgr_Data_Access(edcLOCK_ALL_LENS_MOTORS, edaREAD, &m_cLensMotorLock); //A70LV_Larry_0129

    palDataMgr_Data_Access(edcSERIAL_PORT_BAUD_RATE, edaREAD, (void*)&ucValue); //G100_Owen_0012
    palSysCtrlMgr_Baudrate_Set(ucValue);                                             //G100_Owen_0012

    //SERIAL PORT PATH
    //palDataMgr_Data_Access(edcSERIAL_PORT_PATH, edaREAD, (void*)&ucValue);                       //A70LV_Larry_0237 mask
    //palDataMgr_Data_Access(edcSERIAL_PORT_PATH, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&ucValue); //A70LV_Larry_0237 mask

    //LOG_MSG(db_ALWAYS, "\r\n eIR_ENABLE_HD = %d", ucIREnable);

    //LOG_MSG(db_ALWAYS, "\r\n BurnIn_Enable = %d\r\n", palIllumination_BurnIn_Enable()); //A70LV_Larry_0023


    GuiCb = Gui_fpCallbackGet();

#ifdef NEW_PWR_ON_WAY
    m_cPowerPressed = FALSE;
#endif // NEW_PWR_ON_WAY
    return eEXEC_CODE_PASS;
}

eEXEC_CODE palInputProc_PowerNormal(void)       //G100_Owen_0007
{
    UINT8  ucValue = 0;

    palDataMgr_Data_Access(edcTOP_IR, edaREAD, (void*)&ucValue); //A70LV_Larry_0023
    palDataMgr_Data_Access(edcTOP_IR, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&ucValue);

    palDataMgr_Data_Access(edcFRONT_IR, edaREAD, (void*)&ucValue); //A70LV_Larry_0023
    palDataMgr_Data_Access(edcFRONT_IR, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&ucValue);

    palDataMgr_Data_Access(edcHDBASET_IR, edaREAD, (void*)&ucValue); //A70LV_Larry_0023
    palDataMgr_Data_Access(edcHDBASET_IR, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&ucValue);

    palDataMgr_Data_Access(edcSERIAL_PORT_ECHO, edaREAD, (void*)&ucValue);
    palDataMgr_Data_Access(edcSERIAL_PORT_ECHO, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&ucValue);

    palDataMgr_Data_Access(edcPROJECTOR_ADDRESS, edaREAD, (void*)&ucValue); //A70LV_Larry_0060
    palDataMgr_Data_Access(edcPROJECTOR_ADDRESS, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&ucValue);

    palDataMgr_Data_Access(edcPROJECTOR_ID, edaREAD, (void*)&ucValue);
    palDataMgr_Data_Access(edcPROJECTOR_ID, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&ucValue);

    //m_cLensMotorLock = 0;

	palDataMgr_Data_Access(edcLOCK_ALL_LENS_MOTORS, edaREAD, &m_cLensMotorLock); //A70Gen2_Steven_0001


    m_cDisableKey = 0;
    //palDataMgr_CurTestPatternGet() = eTID_OFF;
    m_cUSTTestPattern = 0;

    return eEXEC_CODE_PASS;
}
// ==============================================================================
// FUNCTION NAME: APPINPUTPROC_BUFFERINSERT
// DESCRIPTION:
//
//
// Params:
// void *pvKey:
//
// Returns:
//
//
// modification history
// --------------------
// 24/12/2013, Leo Create
// --------------------
// ==============================================================================
eRESULT palInputProc_BufferInsert(void *pvKey)
{
    palInputProc_OPDKeyEvent(*(sKEY_DATA *)pvKey);  //A35G2_Simon_0075

    return (eRESULT)utilQueueWrite(&m_sInputRingBuffer, (UINT8*)pvKey);
}

// ==============================================================================
// FUNCTION NAME: palInputProc_KeyTimerHander
// DESCRIPTION:
//
//
// Params:
// sKEY_DATA *psInputKey:
//
// Returns:
//
//
// Modification History
// --------------------
// 2018/01/11, Larry Create
// --------------------
// ==============================================================================
static BOOL palInputProc_KeyTimerHander(sKEY_DATA *psInputKey)
{
    BOOL bEnable = FALSE;

    switch(psInputKey->eKeyEvent)
    {
        case eKEY_EVENT_PRESSED:
            if(utilCounterGet(eCOUNTER_TYPE_GUI_KEY) == 0)
            {
                utilCounterSet(eCOUNTER_TYPE_GUI_KEY, 100);
                bEnable = TRUE;
            }
            break;

        case eKEY_EVENT_HOLD:
            if((utilCounterGet(eCOUNTER_TYPE_GUI_KEY) == 0) && (utilCounterGet(eCOUNTER_TYPE_LENS_SHIFT_CONTROL) == 0))
            {
                utilCounterSet(eCOUNTER_TYPE_GUI_KEY, 250);
                bEnable = TRUE;
            }
            else if(utilCounterGet(eCOUNTER_TYPE_LENS_SHIFT_CONTROL) != 0)
			{
			    if(utilCounterGet(eCOUNTER_TYPE_GUI_KEY) == 0)
                {
                    utilCounterSet(eCOUNTER_TYPE_GUI_KEY, 100);
                    bEnable = TRUE;
                }
            }
            break;

        case eKEY_EVENT_RELEASE:
            bEnable = TRUE;
            break;
    }

    return bEnable;
}

// ==============================================================================
// FUNCTION NAME: palInputProc_KeyFilter
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
// 2018/01/15, Larry Create
// --------------------
// ==============================================================================
static BOOL palInputProc_KeyFilter(sKEY_DATA *psInputKey)
{
    BOOL bEnable = FALSE;
    UINT8 ucUST = ets_OFF;
    UINT8 ucLens_Detect = 0;
    UINT8 ucLens_Model = 0;
    UINT8 ucFreeze= 0;
#if defined(CUSTOM_BARCO)  //A35G2_BRC_Casper_0104
    UINT8 ucFastPowerDown = 0;
#endif

    palDataMgr_Data_Access(edcUST_SET, edaREAD, &ucUST); //A35G2_BRC_Casper_0088
#if defined(CUSTOM_BARCO) //A35G2_BRC_Casper_0104
    palDataMgr_Data_Access(edcFAST_POWER_ON, edaREAD, &ucFastPowerDown);
#endif
    palDataMgr_Data_Access(edcIMAGE_FREEZE, edaREAD, &ucFreeze);

    if(palCoreVar_GetLightSourceOnVar() == FALSE) //LD Off
    {
        switch(psInputKey->wKeyCode)
        {
            case kePOWERON:
            case keHOLD_POWERON:
            case keRELEASE_POWERON:
            case keMUTE:
            case keHOLD_MUTE:
            case keRELEASE_MUTE:
            case keSTANDBY:
            case keHOLD_STANDBY:
                bEnable= TRUE;
                break;
        }
    }
#if defined(CUSTOM_BARCO) //A35G2_BRC_Casper_0104
    else if(ucFastPowerDown && palEnvironment_Fake_Power_Down_Get())
    {
        switch(psInputKey->wKeyCode)
        {
            case kePOWERON:
            case keHOLD_POWERON:
            case keRELEASE_POWERON:
                bEnable= TRUE;
                break;

            default:
                bEnable = FALSE;
                break;
        }
    }
    else if(palEnvironment_SecurityPowerDownTimerGet()) //A35G2_BRC_Casper_0107
    {
        switch(psInputKey->wKeyCode)
        {
            case keMUTE:
                bEnable= FALSE;
                break;

            default:
                bEnable = TRUE;
                break;
        }
    }
#endif
    else if( palDataMgr_Camera_OSD_Lock_Get() == eOSD_LOCKED ) // not allow to do anything, except Power Off, and AF/AC set cmds.
    {
        switch(psInputKey->wKeyCode)
        {
            //Power Off
            case kePOWERON:
            case keHOLD_POWERON:
            case keRELEASE_POWERON:
            case keSTANDBY:
            case keHOLD_STANDBY:
            case keRELEASE_STANDBY:
                bEnable = TRUE;
                break;

            // AF/AC functions

            case keFOCUSDOWN:
            case keFOCUSUP:
            case keOSD:
            case keBACK:
                if( psInputKey->eKeyType== eKEYINPUT_LAN )  //G100_Clare_0056//will not from LAN
                {
    				bEnable = TRUE;
                }
				break;

            }
    }
    else if((palGeo_ApLinkFlag_Get() == TRUE) && (TRUE == palGeo_IsDraw_OSD(eWIL_BEFORE_WARP)))  //H2PF_Simon_0070
    {
        switch(psInputKey->wKeyCode)
        {
            case kePIP:
            case kePIP_SIZE:
            case kePIP_LAYOUT:
            case keSWAP:
            case keAUTO:
            case keINPUT:
            case ke0:
            case ke1:
            case ke2:
            case ke3:
            case ke4:
            case ke5:
            case ke6:
            case ke7:
            case ke8:
            case ke9:
            case keOSD:
            case keTEST:
            case keMENU:
            case keUP:
            case keDOWN:
            case keLEFT:
            case keRIGHT:
            case keENTER:
                bEnable = FALSE;
                break;

            default:
                bEnable = TRUE;
                break;
        }
    }
#ifdef OSD_LOCK_ENABLE     //A65_OPTOMA_Jerry_0005
    else if(palInputProc_OSD_Lock_Check_Key(psInputKey->eKeyType, psInputKey->wKeyCode) == eCMD_KEY_DATACODE_NOT_ALLOWED_TO_EXECUTE )
    {
        // OSD is locked, this key is not allow to execute.
        bEnable = FALSE;
    }
#endif
    //G100_Clare_0027, add, end
    else if(palIllumination_ABP_StatusGet() == eABP_STATUS_FACTORY_CAL) //A70LV_Larry_0201 //A35G2_BRC_Casper_0066
    {
        bEnable = FALSE;
    }
    else if(palDataMgr_ACU_Target_Status_Get() == ets_ON)  //ACU Select Target
    {
        bEnable = palInputProc_ACU_Target_Select_KeyCheck(psInputKey->wKeyCode);
    }
    else if((ucFreeze == 0) &&
            (palDataMgr_CurTestPatternGet() != eTID_OFF) &&
            (palDataMgr_CurTestPatternGet() < eTID_HSG_RED)) //HICC2_Doulas_0051 Modify//A70LV_Doulas_0230 Test pattern on //A35G2_BRC_Casper_0115 //HICC2_Casper_0019
    {
            BOOL bServiceMode = palDataMgr_ServiceModeGet();
            UINT8 ucTP;
            switch(psInputKey->wKeyCode)
            {
                case keMUTE:
                case ke2:
                case ke4:
                case ke6:
                case ke8:
                case keHOLD_2:
                case keHOLD_4:
                case keHOLD_6:
                case keHOLD_8:
                case keRELEASE_2:
                case keRELEASE_4:
                case keRELEASE_6:
                case keRELEASE_8:

                case keZOOMIN:
                case keZOOMOUT:
                case keHOLD_ZOOMIN:
                case keHOLD_ZOOMOUT:
                case keFOCUSUP:
                case keFOCUSDOWN:
                case keHOLD_FOCUSUP:
                case keHOLD_FOCUSDOWN:

                case keFOCUS:
                case keZOOM:

                case keHKEYSTONE_LEFT:
                case keHKEYSTONE_RIGHT:
                case keHOLD_HKEYSTONE_LEFT:
                case keHOLD_HKEYSTONE_RIGHT:
                case keRELEASE_HKEYSTONE_LEFT:
                case keRELEASE_HKEYSTONE_RIGHT:

                case keVKEYSTONE_UP:
                case keVKEYSTONE_DOWN:
                case keHOLD_VKEYSTONE_UP:
                case keHOLD_VKEYSTONE_DOWN:
                case keRELEASE_VKEYSTONE_UP:
                case keRELEASE_VKEYSTONE_DOWN:

                case keID: //G100_Steven_0019
                    bEnable = TRUE;
                    break;


                case keLENS:
                case keLENSHLEFT:
                case keLENSHRIGHT:
                case keLENSVUP:
                case keLENSVDOWN:
                case keHOLD_LENSHLEFT:
                case keHOLD_LENSHRIGHT:
                case keHOLD_LENSVUP:
                case keHOLD_LENSVDOWN:
                    if(ucUST == ets_OFF) //A35G2_BRC_Casper_0115
                    {
                        bEnable = TRUE;
                    }
                    else
                    {
                        bEnable = FALSE;
                    }
                    break;

                case keSTANDBY:
                    if(palInputProc_KeyTimerHander(psInputKey) == TRUE)
                    {
                        LOG_MSG(db_APP_INPUTKEY, "\r\nDisable Test Pattern");

                        ucTP = (UINT8)eTID_OFF;
                        palDataMgr_Data_Access(edcSERVICE_TEST_PATTERN, edaWRITE_THROUGH_WITH_ACTION, &ucTP) ;
                        bEnable = TRUE;
                    }
                    else
                    {
                        bEnable = FALSE;
                    }
                    break;

                case kePOWERON: //HICC2_Casper_0028
                    if(psInputKey->eKeyType == eKEYINPUT_KEYPAD)
                    {
                        LOG_MSG(db_APP_INPUTKEY, "\r\nDisable Test Pattern");

                        ucTP = eTID_OFF;
                        palDataMgr_Data_Access(edcSERVICE_TEST_PATTERN, edaWRITE_THROUGH_WITH_ACTION, &ucTP) ;
                    }
                    bEnable = TRUE;
                    break;

                //case keMENU: //A70LV_Larry_0353
                case keENTER:
                case keHELP:
                case keINPUT:
                case keINFO:
                case keAUTO:
                case keGAMMA:
                case keBRIGHT:
                case keCONTRAST:
                case keMODE:
                case keHOTKEY:
                case keHOTKEY2: //G100_Casper_0001
                case keSWAP:
                case kePIP:
                case kePIP_SIZE:
                case kePIP_LAYOUT:
                    if(palInputProc_KeyTimerHander(psInputKey) == TRUE)
                    {
                        LOG_MSG(db_APP_INPUTKEY, "\r\nDisable Test Pattern");

                        ucTP = (UINT8)eTID_OFF;
                        palDataMgr_Data_Access(edcSERVICE_TEST_PATTERN, edaWRITE_THROUGH_WITH_ACTION, &ucTP) ;
                    }
                    bEnable = FALSE;
                    break;

                case keMENU: //A70LV_Larry_0353
               case keBACK: //HICC2_Casper_0019
                    bEnable = TRUE;
                    break;

                case keLEFT:
                    if(palInputProc_KeyTimerHander(psInputKey) == TRUE)
                    {
                        LOG_MSG(db_APP_INPUTKEY, "\r\nLeft Key");
                        if(bServiceMode)
                        {
                            palDataMgr_Data_Access(edcSERVICE_TEST_PATTERN, edaREAD, &ucTP);
                            ucTP = (UINT8)DataMapping_GetPreCMvalueFromGui(edcSERVICE_TEST_PATTERN, ucTP);
                            palDataMgr_Data_Access(edcSERVICE_TEST_PATTERN, edaWRITE_THROUGH_WITH_ACTION, &ucTP);
                        }
                        else
                        {
                            palDataMgr_Data_Access(edcOSDTEST_PATTERN, edaREAD, &ucTP);
                            ucTP = (UINT8)DataMapping_GetPreCMvalueFromGui(edcOSDTEST_PATTERN, ucTP);
                            palDataMgr_Data_Access(edcOSDTEST_PATTERN, edaWRITE_THROUGH_WITH_ACTION, &ucTP);
                			//GuiCb.fpGui_TestPatternFocusIndexEventCb(psInputKey->eKeyType, ekLEFT); //G100_Casper_0006
                        }
                    }
                    bEnable = FALSE;
                    break;

                case keRIGHT:
                case keTEST:
                    if(palInputProc_KeyTimerHander(psInputKey) == TRUE)
                    {
                        LOG_MSG(db_APP_INPUTKEY, "\r\nRight or Test Key");

                        if(bServiceMode)
                        {
                            palDataMgr_Data_Access(edcSERVICE_TEST_PATTERN, edaREAD, &ucTP);
                            ucTP = (UINT8)DataMapping_GetNextCMvalueFromGui(edcSERVICE_TEST_PATTERN, ucTP);
                            palDataMgr_Data_Access(edcSERVICE_TEST_PATTERN, edaWRITE_THROUGH_WITH_ACTION, &ucTP);
                        }
                        else
                        {
                            palDataMgr_Data_Access(edcOSDTEST_PATTERN, edaREAD, &ucTP);
                            ucTP = (UINT8)DataMapping_GetNextCMvalueFromGui(edcOSDTEST_PATTERN, ucTP);
                            palDataMgr_Data_Access(edcOSDTEST_PATTERN, edaWRITE_THROUGH_WITH_ACTION, &ucTP);
                        }
                    }
                    bEnable = FALSE;
                    break;

                default:
                    bEnable = FALSE;
                    break;
            }
    }
    else if((ucFreeze == 0) && (ucUST == ets_ON) && (m_cUSTTestPattern == DDP_SPLASH_MIN_VALUE) && (palDataMgr_CurTestPatternGet() == eTID_OFF)) //A35G2_BRC_Casper_0088 //A35G2_BRC_Casper_0115
    {
        switch(psInputKey->wKeyCode)
        {
            case keLENSHLEFT:
            case keLENSHRIGHT:
            case keLENSVUP:
            case keLENSVDOWN:
            case keHOLD_LENSHLEFT:
            case keHOLD_LENSHRIGHT:
            case keHOLD_LENSVUP:
            case keHOLD_LENSVDOWN:
            case keLENS:
                bEnable = FALSE;
            break;

            default:
                bEnable = TRUE;
                break;
        }
    }
	else if((ucFreeze == 0) && (ucUST == ets_ON) && (m_cUSTTestPattern != DDP_SPLASH_MIN_VALUE) && (palDataMgr_CurTestPatternGet() == eTID_OFF))       //ZU860_Energy_0011 ust pattern behavior //A35G2_BRC_Casper_0088 //A35G2_BRC_Casper_0115
    {
        //BOOL bServiceMode = FALSE;//palDataMgr_ServiceModeGet();    //ZU860_Doulas_0031 modify
        UINT8 ucTP;
        switch(psInputKey->wKeyCode)
        {
			case keTEST://ZU860_Energy_0011
			case kePATTERN:
                ucTP = m_cUSTTestPattern;
                if(ucTP == (DDP_SPLASH_MAX_VALUE - 1)) //A35G2_BRC_Casper_0097
                {
                    ucTP = DDP_SPLASH_MIN_VALUE + 1;
                }
                else
                {
                    ucTP++;
                }
                palDataMgr_Data_Access((eDATA_CODE)edcUST_Pattern, edaWRITE_RAM_ONLY_WITH_ACTION, &ucTP);
                break;

            case keBACK:
                ucTP = DDP_SPLASH_MIN_VALUE;
                palDataMgr_Data_Access((eDATA_CODE)edcUST_Pattern, edaWRITE_RAM_ONLY_WITH_ACTION, &ucTP);
                palDataMgr_UI_EventSend(edcUI_EVENT_OSD_EXIT, TRUE, NULL);
                #if defined(CUSTOM_BARCO) || defined(CUSTOM_CHRISTIE) || defined(CUSTOM_OPTOMA) ////A35G2_Coda_0117
                //appGui_OpenMenu(USTINSTALL_MESSAGE2_MENU_ICOUNT);
                //GuiCb.fpGui_Send_MenuOpen_UST_Message2Cb(); //###
                #endif
                palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL);
                break;
            case keZOOMIN:
            case keZOOMOUT:
            case keHOLD_ZOOMIN:
            case keHOLD_ZOOMOUT:
            case keFOCUSUP:
            case keFOCUSDOWN:
            case keHOLD_FOCUSUP:
            case keHOLD_FOCUSDOWN:
            case keLENSHLEFT:
            case keLENSHRIGHT:
            case keLENSVUP:
            case keLENSVDOWN:
            case keHOLD_LENSHLEFT:
            case keHOLD_LENSHRIGHT:
            case keHOLD_LENSVUP:
            case keHOLD_LENSVDOWN:
            case keFOCUS:
            case keLENS:
            case keZOOM:
            case keUP:
            case keDOWN:
            case keLEFT:
            case keRIGHT:
            case keHOLD_UP:
            case keHOLD_DOWN:
            case keHOLD_LEFT:
            case keHOLD_RIGHT:
                bEnable = TRUE;
            break;

            default:
                bEnable = FALSE;
            break;
        }
    }
#if defined(CUSTOM_OPTOMA)
	else if(utilWarp_GetOsdPatternType() != PAT_TYPE__OFF) 	//A35G2_Owen_0002 //A65_OPTOMA_Doulas_0037 warp/blend/black level setting filter
    {
    	switch(psInputKey->wKeyCode)
		{
			case keSTANDBY:
			case kePOWERON:
			case keMENU:
			case keBACK:
			case keENTER:
			case keUP:
			case keDOWN:
			case keRIGHT:
			case keLEFT:
			case keSHUTTER:
			case keMUTE:
			case keHOLD_UP:
            case keHOLD_DOWN:
            case keHOLD_LEFT:
            case keHOLD_RIGHT:
			case keRELEASE_LEFT:	//A65_OPTOMA_Doulas_0166
			case keRELEASE_RIGHT:	//A65_OPTOMA_Doulas_0166
			case keRELEASE_UP:		//A65_OPTOMA_Doulas_0166
    		case keRELEASE_DOWN:	//A65_OPTOMA_Doulas_0166
  			case keAUTO:			//A65_OPTOMA_Doulas_0211
			case keINFO:			//A65_OPTOMA_Doulas_0211
			case keINPUT:			//A65_OPTOMA_Doulas_0211

			case keFOCUSUP:				//A65_OPTOMA_Doulas_0231
			case keFOCUSDOWN:			//A65_OPTOMA_Doulas_0231
			case keZOOMIN:				//A65_OPTOMA_Doulas_0231
			case keZOOMOUT: 			//A65_OPTOMA_Doulas_0231
			case keLENSHRIGHT:			//A65_OPTOMA_Doulas_0231
			case keLENSHLEFT:			//A65_OPTOMA_Doulas_0231
			case keLENSVUP: 			//A65_OPTOMA_Doulas_0231
			case keLENSVDOWN:	    	//A65_OPTOMA_Doulas_0231
			case keHOLD_FOCUSUP:		//A65_OPTOMA_Doulas_0231
            case keHOLD_FOCUSDOWN:		//A65_OPTOMA_Doulas_0231
            case keHOLD_ZOOMIN:			//A65_OPTOMA_Doulas_0231
            case keHOLD_ZOOMOUT:		//A65_OPTOMA_Doulas_0231
            case keHOLD_LENSHRIGHT:		//A65_OPTOMA_Doulas_0231
            case keHOLD_LENSHLEFT:		//A65_OPTOMA_Doulas_0231
            case keHOLD_LENSVUP:		//A65_OPTOMA_Doulas_0231
            case keHOLD_LENSVDOWN:		//A65_OPTOMA_Doulas_0231
            case keRELEASE_FOCUSUP:		//A65_OPTOMA_Doulas_0231
            case keRELEASE_FOCUSDOWN:	//A65_OPTOMA_Doulas_0231
            case keRELEASE_ZOOMIN:		//A65_OPTOMA_Doulas_0231
            case keRELEASE_ZOOMOUT:		//A65_OPTOMA_Doulas_0231
            case keRELEASE_LENSHRIGHT:	//A65_OPTOMA_Doulas_0231
            case keRELEASE_LENSHLEFT:	//A65_OPTOMA_Doulas_0231
            case keRELEASE_LENSVUP:		//A65_OPTOMA_Doulas_0231
            case keRELEASE_LENSVDOWN:	//A65_OPTOMA_Doulas_0231
				bEnable = TRUE;
				break;

			default:
				bEnable = FALSE;
				break;
		}
    }
#endif
    else if(ucFreeze)
    {
        if((psInputKey->wKeyCode == keMENU) ||
           (psInputKey->wKeyCode == keBACK) ||
           (psInputKey->wKeyCode == keENTER) ||
           (psInputKey->wKeyCode == keSTANDBY) ||
        #if defined(CUSTOM_CHRISTIE)
           (psInputKey->wKeyCode == keDOWN) ||
           (psInputKey->wKeyCode == keUP) ||
        #else// HICC2_Bruce_0003, for unfreeze
           (psInputKey->wKeyCode == keRIGHT) ||
           (psInputKey->wKeyCode == keLEFT) ||
        #endif
		   (psInputKey->wKeyCode == keMUTE)|| // HICC2_Steven_0034, ISS-0022742: Freeze狀態下按Shutter 不會執行Shutter功能.
		   (psInputKey->wKeyCode == kePOWERON && psInputKey->eKeyType == eKEYINPUT_KEYPAD)) //HICC2_Steven_0038, ISS-0023273 Freeze狀態下透過keypad關機會詢問是否解除freeze，與remote不同
        {
            bEnable = TRUE;
        }
        else
        {
            sKEY_DATA sKey = {psInputKey->wKeyCode, psInputKey->eKeyEvent, psInputKey->eKeyType};

            LOG_MSG(db_APP_INPUTKEY, "Block Line %d\n", __LINE__);

            sKey.wKeyCode = eKEY_LIST_NUMBER;

            palInputProc_KeyEventSend(sKey);

            bEnable = FALSE;
        }
    }
    else
    {
        bEnable = TRUE;
    }

    return bEnable;
}

// ==============================================================================
// FUNCTION NAME: palInputProc_Remap
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
// 2019/04/02, Larry Create
// --------------------
// ==============================================================================
static UINT32 palInputProc_Remap(eKEY_LIST eKey) //ZU860_Doulas_0080
{
    UINT32 wKeyEvent = (UINT32)eKey;

    if(palMotorEvent_CeilingGet() == TRUE)
    {
        switch(eKey)
        {
            case keLEFT:
                wKeyEvent = keRIGHT;
                break;

            case keRIGHT:
                wKeyEvent = keLEFT;
                break;

            case keUP:
                wKeyEvent = keDOWN;
                break;

            case keDOWN:
                wKeyEvent = keUP;
                break;

            case keHOLD_LEFT:
                wKeyEvent = keHOLD_RIGHT;
                break;

            case keHOLD_RIGHT:
                wKeyEvent = keHOLD_LEFT;
                break;

            case keHOLD_UP:
                wKeyEvent = keHOLD_DOWN;
                break;

            case keHOLD_DOWN:
                wKeyEvent = keHOLD_UP;
                break;

            default:
                break;
        }
    }

    if(palMotorEvent_RearGet() == TRUE)
    {
       switch(wKeyEvent)
       {
           case keLEFT:
               wKeyEvent = keRIGHT;
               break;

           case keRIGHT:
               wKeyEvent = keLEFT;
               break;

           case keHOLD_LEFT:
               wKeyEvent = keHOLD_RIGHT;
               break;

           case keHOLD_RIGHT:
               wKeyEvent = keHOLD_LEFT;
               break;

           default:
               break;
       }
    }

    return wKeyEvent;
}

eEXEC_CODE palInputProc_ServiceCodeCheck(sKEY_DATA *psInputKey)//HICC2_Julie_0020
{

    BYTE cMenuOpen = 0;
    palDataMgr_Data_Access(edcIS_SERVICE_CODE_MENU, edaREAD, &cMenuOpen);

    if(psInputKey->eKeyType == eKEYINPUT_KEYPAD)
    {
        if(cMenuOpen == TRUE)
        {
            sKEY_DATA sInputKey2;
            sInputKey2.wKeyCode = psInputKey->wKeyCode;
            sInputKey2.eKeyEvent = psInputKey->eKeyEvent;
            sInputKey2.eKeyType = psInputKey->eKeyType;

            switch(sInputKey2.wKeyCode)
            {
                case keLEFT:
                    LOG_MSG(db_APP_INPUTKEY, "\r\nService Left Key");
                    if(palInputProc_KeyTimerHander(&sInputKey2) == TRUE)
                    {
                        sInputKey2.wKeyCode = ke4;
                        palInputProc_KeyEventSend(sInputKey2);
                    }
                    return eEXEC_CODE_PASS;

                case keDOWN:
                    LOG_MSG(db_APP_INPUTKEY, "\r\nService Down Key");
                    if(palInputProc_KeyTimerHander(&sInputKey2) == TRUE)
                    {
                        sInputKey2.wKeyCode = ke7;
                        palInputProc_KeyEventSend(sInputKey2);
                    }
                    return eEXEC_CODE_PASS;

                case keRIGHT:
                    LOG_MSG(db_APP_INPUTKEY, "\r\nService Right Key");
                    if(palInputProc_KeyTimerHander(&sInputKey2) == TRUE)
                    {
                        sInputKey2.wKeyCode = ke5;
                        palInputProc_KeyEventSend(sInputKey2);
                    }
                    return eEXEC_CODE_PASS;

                case keINPUT:
                    LOG_MSG(db_APP_INPUTKEY, "\r\nService Input Key");
                    if(palInputProc_KeyTimerHander(&sInputKey2) == TRUE)
                    {
                        sInputKey2.wKeyCode = ke3;
                        palInputProc_KeyEventSend(sInputKey2);
                    }
                    return eEXEC_CODE_PASS;
            }
        }
    }

    return eEXEC_CODE_FAIL;
}

#ifdef OE_JIG
static eBG_DISPLAY_MODE eMode = BG_DISABLE;

#if 0 //x35
BOOL palInputProc_OEJig_HotKey(sKEY_DATA *psInputKey)
{
    sKEY_DATA sInputKey = {0};

    sInputKey.wKeyCode = psInputKey->wKeyCode;
    sInputKey.eKeyEvent = psInputKey->eKeyEvent;
    sInputKey.eKeyType = psInputKey->eKeyType;

    switch(sInputKey.wKeyCode)
    {
         case kePOWERON: //A70LV_Larry_0256 modify
    	 	{
                LOG_MSG(db_APP_INPUTKEY, "\r\n Key1 PowerMode = Low"); //A70LV_John_0038 fix OE JIG bug

    			UINT8 ucData;

    			ucData = eCM_POWER_MODE_CONSTANT_POWER;
    			palDataMgr_Data_Access(edcPOWER_MODE, edaWRITE_THROUGH_NO_ACTION, (void*)&ucData);

    			ucData = 1;
    			palDataMgr_Data_Access(edcCONSTANT_POWER_NUMBER, edaWRITE_THROUGH_NO_ACTION, (void*)&ucData);

    			//ucData = eCM_COLOR_WHEEL_SPEED_2X;
    			//palDataMgr_Data_Access(edcCOLOR_WHEEL_SPEED, edaWRITE_THROUGH_NO_ACTION, (void*)&ucData);

    			LOG_MSG(db_APP_INPUTKEY, "\r\nPower On Key"); //A70LV_Larry_0083

                if(sInputKey.eKeyType == eKEYINPUT_KEYPAD)
                {
                    palSystem_PowerDown();
                }

         	}
            break;

        case keSTANDBY: //A70LV_Larry_0083 //A70LV_Larry_0256
            {
    			//UINT8 ucData;

    			//ucData = eCM_POWER_MODE_CONSTANT_POWER;
    			//palDataMgr_Data_Access(edcPOWER_MODE, edaWRITE_THROUGH_NO_ACTION, (void*)&ucData);

    			//ucData = 1;
    			//palDataMgr_Data_Access(edcCONSTANT_POWER_NUMBER, edaWRITE_THROUGH_NO_ACTION, (void*)&ucData);

    			//ucData = eCOLOR_SPEED_2X;
    			//palDataMgr_Data_Access(edcCOLOR_WHEEL_SPEED, edaWRITE_THROUGH_NO_ACTION, (void*)&ucData);

                //LOG_MSG(db_APP_INPUTKEY, "\r\nStandby Key");
                    palSystem_PowerDown();
            }
            break;
        // A70LV_Eric.C_0012 Start
        /* Hotkey - start */
        case ke1:
                if(GuiCb.fpGui_Get_MenuState_IsOSD_OpenCb())
            {
                if(palInputProc_KeyTimerHander(&sInputKey) == TRUE)
                {
                        GuiCb.fpGui_SendKeyEventCb(ucKeyType, ek1);
                }
            }
            else
            {
                static eBG_DISPLAY_MODE eMode = BG_DISABLE;

                if(eMode == BG_DISABLE)
                {
                    eMode = BG_BLK;
                }
                else
                {
                    eMode = BG_DISABLE;
                }
                LOG_MSG(db_APP_INPUTKEY, "\r\n ke1 : BGDisplayMode = %d", eMode);
                halFormatter_DMD_BGDisplayModeSet(eMode);
        	}
            break;

        case ke2:
                if(GuiCb.fpGui_Get_MenuState_IsOSD_OpenCb())
            {
                if(palInputProc_KeyTimerHander(&sInputKey) == TRUE)
                {
                        GuiCb.fpGui_SendKeyEventCb(ucKeyType, ek2);
                }
            }
            else
            {
                LOG_MSG(db_APP_INPUTKEY, "\r\n ke2 : Lens Walk around "); //A70LV_John_0038 fix OE JIG bug
                palMotorLensSet(eLENS_CMDS_MOVING);
            }
            break;

        case ke3:
                if(GuiCb.fpGui_Get_MenuState_IsOSD_OpenCb())
            {
                if(palInputProc_KeyTimerHander(&sInputKey) == TRUE)
                {
                        GuiCb.fpGui_SendKeyEventCb(ucKeyType, ek3);
                }
            }
            else
            {
                LOG_MSG(db_APP_INPUTKEY, "\r\n Key3 : HDMI1");

                if(palDataMgr_CurTestPatternGet() == eTID_OFF)
                {
                    UINT8 ucSource = eCM_SOURCE_HDMI1;

                    if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(edcMAIN_INPUT))
                        break;
                    palDataMgr_Data_Access(edcMAIN_INPUT, edaWRITE_THROUGH_WITH_ACTION, &ucSource) ;
                    palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL);
                }
                else
                {
                    GuiCb.fpGui_SendKeyEventCb(ucKeyType, ek3);
                }
            }
            break;

        case ke4: //A70LV_Larry_0256
                if(GuiCb.fpGui_Get_MenuState_IsOSD_OpenCb())
            {
                if(palInputProc_KeyTimerHander(&sInputKey) == TRUE)
                {
                        GuiCb.fpGui_SendKeyEventCb(ucKeyType, ek4);
                }
            }
            else
            {
                LOG_MSG(db_APP_INPUTKEY, "\r\n Key4 : HDMI2");

                if(palDataMgr_CurTestPatternGet() == eTID_OFF)
                {
                    UINT8 ucSource = eCM_SOURCE_HDMI2;

                    if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(edcMAIN_INPUT))
                        break;
                    palDataMgr_Data_Access(edcMAIN_INPUT, edaWRITE_THROUGH_WITH_ACTION, &ucSource) ;
                        palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL);
                }
                else
                {
                    GuiCb.fpGui_SendKeyEventCb(ucKeyType, ek4);
                }
            }
            break;

        case ke5:
            if(GuiCb.fpGui_Get_MenuState_IsOSD_OpenCb())
            {
                if(palInputProc_KeyTimerHander(&sInputKey) == TRUE)
                {
                    GuiCb.fpGui_SendKeyEventCb(ucKeyType, ek5);
                }
            }
            else
            {
                LOG_MSG(db_APP_INPUTKEY, "\r\n Key5 : DVI");

                if(palDataMgr_CurTestPatternGet() == eTID_OFF)
                {
                    UINT8 ucSource = eCM_SOURCE_DVI;

                    if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(edcMAIN_INPUT))
                        break;
                    palDataMgr_Data_Access(edcMAIN_INPUT, edaWRITE_THROUGH_WITH_ACTION, &ucSource) ;
                    palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL);
                }
                else
                {
                    GuiCb.fpGui_SendKeyEventCb(ucKeyType, ek5);
                }
            }
            break;

        case ke6: //A70LV_Larry_0256
            if(GuiCb.fpGui_Get_MenuState_IsOSD_OpenCb())
            {
                if(palInputProc_KeyTimerHander(&sInputKey) == TRUE)
                {
                    GuiCb.fpGui_SendKeyEventCb(ucKeyType, ek6);
                }
            }
            else
            {
                LOG_MSG(db_APP_INPUTKEY, "\r\n Key6 : 3GSDI");

                if(palDataMgr_CurTestPatternGet() == eTID_OFF)
                {
                    UINT8 ucSource = eCM_SOURCE_3GSDI;

                    if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(edcMAIN_INPUT))
                        break;
                    palDataMgr_Data_Access(edcMAIN_INPUT, edaWRITE_THROUGH_WITH_ACTION, &ucSource) ;
                    palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL);
                }
                else
                {
                    GuiCb.fpGui_SendKeyEventCb(ucKeyType, ek6);
                }
            }
            break;

        case ke7:
            if(GuiCb.fpGui_Get_MenuState_IsOSD_OpenCb())
            {
                if(palInputProc_KeyTimerHander(&sInputKey) == TRUE)
                {
                    GuiCb.fpGui_SendKeyEventCb(ucKeyType, ek7);
                }
            }
            else
            {
                LOG_MSG(db_APP_INPUTKEY, "\r\n Key7 : HDBaseT");

                if(palDataMgr_CurTestPatternGet() == eTID_OFF)
                {
                    UINT8 ucSource = eCM_SOURCE_HDBASET;

                    if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(edcMAIN_INPUT))
                        break;
                    palDataMgr_Data_Access(edcMAIN_INPUT, edaWRITE_THROUGH_WITH_ACTION, &ucSource) ;
                    palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL);
                }
                else
                {
                    GuiCb.fpGui_SendKeyEventCb(ucKeyType, ek7);
                }
            }
            break;

        case ke8:
            LOG_MSG(db_APP_INPUTKEY, "\r\n Key8 : PWM : 8k");
            if(GuiCb.fpGui_Get_MenuState_IsOSD_OpenCb()) //T100_Sander_0016
            {
                if(palInputProc_KeyTimerHander(&sInputKey) == TRUE) //A70LV_Larry_0074
                {
                    GuiCb.fpGui_SendKeyEventCb(ucKeyType, ek8);
                }
            }
            else //LH OEJIG
            {
            	halFormatter_PowerMode_Set(2);
    			halFormatter_SystemModeSet(1);
            }
            break;

        case ke9:
            LOG_MSG(db_APP_INPUTKEY, "\r\n Key9 : PWM : 9.5K");
            if(GuiCb.fpGui_Get_MenuState_IsOSD_OpenCb()) //T100_Sander_0016
            {
                if(palInputProc_KeyTimerHander(&sInputKey) == TRUE) //A70LV_Larry_0074
                {
                    GuiCb.fpGui_SendKeyEventCb(ucKeyType, ek9);
                }
            }
            else //LH OEJIG
            {
            	halFormatter_PowerMode_Set(3);
    			halFormatter_SystemModeSet(12); //A35G2_BRC_Casper_0090
            }
            break;

        case ke0:
            LOG_MSG(db_APP_INPUTKEY, "\r\n Key0 : PWM : 11.5K");
            if(GuiCb.fpGui_Get_MenuState_IsOSD_OpenCb()) //T100_Sander_0016
            {
                if(palInputProc_KeyTimerHander(&sInputKey) == TRUE) //A70LV_Larry_0074
                {
                    GuiCb.fpGui_SendKeyEventCb(ucKeyType, ek0);
                }
            }
            else //LH OEJIG
            {
            	halFormatter_PowerMode_Set(4);
    			halFormatter_SystemModeSet(12); //A35G2_BRC_Casper_0090
            }
            break;

        /* Hotkey - end */
        // A70LV_Eric.C_0012 End

        /* Menu - start */ //to be confirmed
        case keMENU:
            LOG_MSG(db_APP_INPUTKEY, "\r\nMenu Key");

            if(palInputProc_KeyTimerHander(&sInputKey) == TRUE) //A70LV_Larry_0074
            {
            	//if(palInputProc_ADV_WarpBackCheck() == FALSE)		//G100_Doulas_0027 Modify
    			{
                    GuiCb.fpGui_SendKeyEventCb(ucKeyType, ekMENU);
    			}

                if(palDataMgr_CurTestPatternGet() != eTID_OFF) //A70LV_Larry_0296 //A70LV_Larry_0353
                {
                    UINT8 uTP = (UINT8)eCM_TEST_PATTERN_OFF;
                    palDataMgr_Data_Access(edcSERVICE_TEST_PATTERN, edaWRITE_RAM_ONLY_WITH_ACTION, &uTP);
                }
            }
            break;
        case keUP:
            LOG_MSG(db_APP_INPUTKEY, "\r\nUp Key");
            if(palInputProc_KeyTimerHander(&sInputKey) == TRUE) //A70LV_Larry_0074
            {
                //G100_Wilsonj_0067 Start
                //UINT8 ucServiceCode = 0;
                //INT32 ucMAX_Number = 0;

                //palDataMgr_Data_Access(edcSERVICE_CODE_INFO2, edaREAD, &ucServiceCode);
                //palDataMgr_Data_Range_Get(edcSERVICE_CODE_INFO2, edrMAX, &ucMAX_Number);

                #ifdef CUSTOM_CHRISTIE
                #else
                if((appGui_CurrentMenuIndexGet() == SERVICE_CODE_INPUT_MENU_ICOUNT) && (sInputKey.eKeyType == eKEYINPUT_KEYPAD) /*&& (ucServiceCode < (UINT8)ucMAX_Number)*/)
                {
                    GuiCb.fpGui_SendKeyEventCb(ucKeyType, ek0);
                }
                else
                #endif
                {
    				//if(palInputProc_ADV_WarpDirectionCheck(DIR__UP) == FALSE)	//G100_Doulas_0027 Modify
    				{
                        	GuiCb.fpGui_SendKeyEventCb(ucKeyType, ekUP);
    				}
                }
                //G100_Wilsonj_0067 End
            }
            break;
        case keDOWN:
        {
            LOG_MSG(db_APP_INPUTKEY, "\r\nDown Key");
            if(palInputProc_KeyTimerHander(&sInputKey) == TRUE) //A70LV_Larry_0074
            {
                //G100_Wilsonj_0067 Start
                //UINT8 ucServiceCode = 0;
                //INT32 ucMAX_Number = 0;

                //palDataMgr_Data_Access(edcSERVICE_CODE_INFO2, edaREAD, &ucServiceCode);
                //palDataMgr_Data_Range_Get(edcSERVICE_CODE_INFO2, edrMAX, &ucMAX_Number);
                {
                	//if(palInputProc_ADV_WarpDirectionCheck(DIR__DOWN) == FALSE)	//G100_Doulas_0027 Modify
                	{
                        	GuiCb.fpGui_SendKeyEventCb(ucKeyType, ekDOWN);
                	}
                }
                //G100_Wilsonj_0067 End
            }
        } //A70LV_John_0038 end
            break;
        case keLEFT:
            LOG_MSG(db_APP_INPUTKEY, "\r\nLeft Key");
            if(palInputProc_KeyTimerHander(&sInputKey) == TRUE) //A70LV_Larry_0074
            {
                //G100_Wilsonj_0067 Start
                //UINT8 ucServiceCode = 0;
                //INT32 ucMAX_Number = 0;

                //palDataMgr_Data_Access(edcSERVICE_CODE_INFO2, edaREAD, &ucServiceCode);
                //palDataMgr_Data_Range_Get(edcSERVICE_CODE_INFO2, edrMAX, &ucMAX_Number);
                {
                	//if(palInputProc_ADV_WarpDirectionCheck(DIR__LEFT) == FALSE)	//G100_Doulas_0027 Modify
                	{
                        	GuiCb.fpGui_SendKeyEventCb(ucKeyType, ekLEFT);
                	}
                }
                //G100_Wilsonj_0067 End
            }
            break;
        case keRIGHT:
            LOG_MSG(db_APP_INPUTKEY, "\r\nRight Key");
            if(palInputProc_KeyTimerHander(&sInputKey) == TRUE) //A70LV_Larry_0074
            {
                //G100_Wilsonj_0067 Start
                //UINT8 ucServiceCode = 0;
                //INT32 ucMAX_Number = 0;

                //palDataMgr_Data_Access(edcSERVICE_CODE_INFO2, edaREAD, &ucServiceCode);
                //palDataMgr_Data_Range_Get(edcSERVICE_CODE_INFO2, edrMAX, &ucMAX_Number);
                {
                	//if(palInputProc_ADV_WarpDirectionCheck(DIR__RIGHT) == FALSE)	//G100_Doulas_0027 Modify
                	{
                        	GuiCb.fpGui_SendKeyEventCb(ucKeyType, ekRIGHT);
                	}
                }
                //G100_Wilsonj_0067 End
            }
            break;
        case keENTER:
            LOG_MSG(db_APP_INPUTKEY, "\r\nEnter Key");
            if(palInputProc_KeyTimerHander(&sInputKey) == TRUE) //A70LV_Larry_0074
            {
            	//if(palInputProc_ADV_WarpEnterCheck() == FALSE)		//G100_Doulas_0027 Modify
    			{
                    	GuiCb.fpGui_SendKeyEventCb(ucKeyType, ekENTER);
    			}
            }
            break;
        case keBACK:
            LOG_MSG(db_APP_INPUTKEY, "\r\nBack Key");
            if(palInputProc_KeyTimerHander(&sInputKey) == TRUE) //A70LV_Larry_0074
            {
            	//if(palInputProc_ADV_WarpBackCheck() == FALSE)		//G100_Doulas_0027 Modify
    			{
                    	GuiCb.fpGui_SendKeyEventCb(ucKeyType, ekEXIT);
    			}

                if(palDataMgr_CurTestPatternGet() != eTID_OFF) //A70LV_Larry_0296
                {
                    UINT8 uTP = (UINT8)eCM_TEST_PATTERN_OFF;
                    palDataMgr_Data_Access(edcSERVICE_TEST_PATTERN, edaWRITE_RAM_ONLY_NO_ACTION, &uTP);
                }
            }
            break;
        /* Menu - end */

        /* Lens - start */  //Eric.C OK
        case keZOOMIN:
            LOG_MSG(db_APP_INPUTKEY, "\r\n keZOOMIN"); //A70LV_John_0038 fix OE JIG bug
            palMotorZoomSet(eZOOMLENS_CMD_ZOOM_IN_STEP);
            break;
        case keZOOMOUT:
            LOG_MSG(db_APP_INPUTKEY, "\r\n keZOOMOUT"); //A70LV_John_0038 fix OE JIG bug
            palMotorZoomSet(eZOOMLENS_CMD_ZOOM_OUT_STEP);
            break;
        case keFOCUSUP:
            LOG_MSG(db_APP_INPUTKEY, "\r\n keFOCUSUP"); //A70LV_John_0038 fix OE JIG bug
            palMotorFocusSet(eZOOMLENS_CMD_FOCUS_NEAR_STEP);
            break;
        case keFOCUSDOWN:
            LOG_MSG(db_APP_INPUTKEY, "\r\n keFOCUSDOWN"); //A70LV_John_0038 fix OE JIG bug
            palMotorFocusSet(eZOOMLENS_CMD_FOCUS_FAR_STEP);
            break;
        case keLENSHLEFT:
            LOG_MSG(db_APP_INPUTKEY, "\r\n keLENSHLEFT"); //A70LV_John_0038 fix OE JIG bug
            palMotorLensSet(eLENS_CMDS_STEP_LEFT);
            break;
        case keLENSHRIGHT:
            LOG_MSG(db_APP_INPUTKEY, "\r\n keLENSHRIGHT"); //A70LV_John_0038 fix OE JIG bug
            palMotorLensSet(eLENS_CMDS_STEP_RIGHT);
            break;
        case keLENSVUP:
            LOG_MSG(db_APP_INPUTKEY, "\r\n keLENSVUP"); //A70LV_John_0038 fix OE JIG bug
            palMotorLensSet(eLENS_CMDS_STEP_UP);
            break;
        case keLENSVDOWN:
            LOG_MSG(db_APP_INPUTKEY, "\r\n keLENSVDOWN"); //A70LV_John_0038 fix OE JIG bug
            palMotorLensSet(eLENS_CMDS_STEP_DOWN);
            break;

        case keHOLD_ZOOMIN:
            LOG_MSG(db_APP_INPUTKEY, "\r\n keHOLD_ZOOMIN"); //A70LV_John_0038 fix OE JIG bug
            palMotorZoomSet(eZOOMLENS_CMD_ZOOM_IN_RUN);
            break;
        case keHOLD_ZOOMOUT:
            LOG_MSG(db_APP_INPUTKEY, "\r\n keHOLD_ZOOMOUT"); //A70LV_John_0038 fix OE JIG bug
            palMotorZoomSet(eZOOMLENS_CMD_ZOOM_OUT_RUN);
            break;
        case keHOLD_FOCUSUP:
            LOG_MSG(db_APP_INPUTKEY, "\r\n keHOLD_FOCUSUP"); //A70LV_John_0038 fix OE JIG bug
            palMotorFocusSet(eZOOMLENS_CMD_FOCUS_NEAR_RUN);
            break;
        case keHOLD_FOCUSDOWN:
            LOG_MSG(db_APP_INPUTKEY, "\r\n keHOLD_FOCUSDOWN"); //A70LV_John_0038 fix OE JIG bug
            palMotorFocusSet(eZOOMLENS_CMD_FOCUS_FAR_RUN);
            break;
        case keHOLD_LENSHLEFT:
            LOG_MSG(db_APP_INPUTKEY, "\r\n keHOLD_LENSHLEFT"); //A70LV_John_0038 fix OE JIG bug
            palMotorLensSet(eLENS_CMDS_RUN_LEFT);
            break;
        case keHOLD_LENSHRIGHT:
            LOG_MSG(db_APP_INPUTKEY, "\r\n keHOLD_LENSHRIGHT"); //A70LV_John_0038 fix OE JIG bug
            palMotorLensSet(eLENS_CMDS_RUN_RIGHT);
            break;
        case keHOLD_LENSVUP:
            LOG_MSG(db_APP_INPUTKEY, "\r\n keHOLD_LENSVUP"); //A70LV_John_0038 fix OE JIG bug
            palMotorLensSet(eLENS_CMDS_RUN_UP);
            break;
        case keHOLD_LENSVDOWN:
            LOG_MSG(db_APP_INPUTKEY, "\r\n keHOLD_LENSVDOWN"); //A70LV_John_0038 fix OE JIG bug
            palMotorLensSet(eLENS_CMDS_RUN_DOWN);
            break;

        case keINFO:
            LOG_MSG(db_APP_INPUTKEY, "\r\n keINFO : Lens Calibration "); //A70LV_John_0038 fix OE JIG bug
            palMotorLensSet(eLENS_CMDS_CALIBRAION);
            break;
        /* Lens - end */

        /* Others - start */
        case keAUTO:
            LOG_MSG(db_APP_INPUTKEY, "\r\nAuto Key");
            if(palInputProc_KeyTimerHander(&sInputKey) == TRUE) //A70LV_Larry_0074
            {
                    GuiCb.fpGui_SendKeyEventCb(ucKeyType, ekAUTO);
            }
            break;

        case keINPUT:
            {
                UINT8 InputKey = eSOURCE_KEY_CHANGE_SOURCE;
                eEXEC_CODE eResult = eEXEC_CODE_PASS;
    			//uOPD_DATA uOPDData = {0};

                LOG_MSG(db_APP_INPUTKEY, "\r\nInput Key");

             	if(palDataMgr_PIN_Protect_Checking() == ePASSWORD_PROTECT_LOCKED)   //A70LV_Doulas_0222 Add
                {
                    break;
                }

                //eResult = palDataMgr_Data_Access(edcINPUT_KEY, edaREAD, &InputKey);
    			if(utilWarp_GetOsdPatternType() != PAT_TYPE__OFF)		//A35G2_Owen_0002 //A65_OPTOMA_Doulas_0211
    			{
    				if(palInputProc_KeyTimerHander(&sInputKey) == TRUE)
            		{
                    		GuiCb.fpGui_SendKeyEventCb(ucKeyType, ekINPUT);
            		}
    				//eResult = eEXEC_CODE_FAIL;
    			}
                else if(palDataMgr_DataCode_Control(edcINPUT_KEY) == eFUNC_CONTROL_ENABLE) //A70LV_Larry_0298
                {
                    eResult = palDataMgr_Data_Access(edcINPUT_KEY, edaREAD, &InputKey) ;
                }
                else
                {
                    InputKey = eSOURCE_KEY_CHANGE_SOURCE;
                }

                if(eResult != eEXEC_CODE_PASS)
                    break;

                #ifdef CUSTOM_CHRISTIE
                {
                    if(InputKey == eSOURCE_KEY_LIST_ALL_SOURCE) //A70LV_Larry_0109
                    {
                        UINT16 uiInput = 0;

                        halFrontEndCtrl_INPUT_SOURCE_DETECT_Get(&uiInput);
                        palDataMgr_Data_Access(edcSOURCE_INFO, edaWRITE_RAM_ONLY_NO_ACTION, &uiInput);

                        appGui_SendListAllSourceEvent();
                    }
                    else
                    {
                        palDataMgr_MainInputSourceChangeToNext();
                    	palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL);        //A70LV_Doulas_0352 Add
                    }
                }
                #else
                palDataMgr_MainInputSourceChangeToNext();
                #endif

    			//sprintf(uOPDData.cString, "%s", "keINPUT");
    			//utilOPD_EventSet(eOPD_KEY_KEYPAD_LOG + ucKeyType, &uOPDData);  //G100_Julie_0040
            }
            break;
        /* Others - end */

        case keHOLD_LEFT:
            if(palInputProc_KeyTimerHander(&sInputKey) == TRUE) //A70LV_Larry_0083
            {
                    GuiCb.fpGui_SendKeyEventCb(ucKeyType, ekLEFT_HOLD);
            }
            break;

        case keHOLD_RIGHT:
            if(palInputProc_KeyTimerHander(&sInputKey) == TRUE) //A70LV_Larry_0083
            {
                    GuiCb.fpGui_SendKeyEventCb(ucKeyType, ekRIGHT_HOLD);
            }
            break;
    	//G100_Clare_0006, add, >>>
    	case kePATTERN:
    		LOG_MSG(db_APP_INPUTKEY, "\r\nPATTERN Key");
				GuiCb.fpGui_SendKeyEventCb(ucKeyType, ekPATTERN);
    		break;
        case keMUTE:
            {
                UINT8 ucData = 0 ;
                palDataMgr_Data_Access(edcPICTURE_MUTE, edaREAD, &ucData);
                ucData = !ucData ;
                palDataMgr_Data_Access(edcPICTURE_MUTE, edaWRITE_RAM_ONLY_WITH_ACTION, &ucData);

                LOG_MSG(db_APP_INPUTKEY, "\r\nMUTE Key");
            }
            break;

        case keBRIGHT:
    		{
                halFormatter_PW_Index_Get(&uPWIndex);
                uPWIndex++;
                LOG_MSG(db_APP_INPUTKEY, "\r\n BRIGHT Key : uPWIndex++ = %x",uPWIndex);
                halFormatter_PW_Index_Set(uPWIndex);
    		}
            break;

        case kePIP_SIZE:
    		{
                halFormatter_PW_Index_Get(&uPWIndex);
                uPWIndex--;
                LOG_MSG(db_APP_INPUTKEY, "\r\n kePIP_SIZE Key : uPWIndex-- = %x",uPWIndex);
                halFormatter_PW_Index_Set(uPWIndex);
    		}
            break;

        case keCONTRAST:
    		{
    			halFormatter_FW_Index_Get(&uFWIndex);
    			uFWIndex++;
                LOG_MSG(db_APP_INPUTKEY, "\r\n CONTRAST Key : uFWIndex++ = %d",uFWIndex);
                halFormatter_FW_Index_Set(uFWIndex);
    		}
            break;

        case kePIP_LAYOUT:
    		{
    			halFormatter_FW_Index_Get(&uFWIndex);
    			uFWIndex--;
                LOG_MSG(db_APP_INPUTKEY, "\r\n PIP_LAYOUT Key : uFWIndex-- = %d",uFWIndex);
                halFormatter_FW_Index_Set(uFWIndex);
    		}
            break;

        case keGAMMA:
    		{
    			uPMode ++;
    			uPMode = uPMode%3; //A35G2_BRC_Casper_0122
    			LOG_MSG(db_APP_INPUTKEY, "\r\n Gamma Key : PowerMode++ = %d",uPMode);
                halFormatter_PowerMode_Set(uPMode);
    		}
            break;

        case kePIP:
    		{
    		    if(uPMode == 0)
                {
    				uPMode=2; //A35G2_BRC_Casper_0122
                    halFormatter_PowerMode_Set(uPMode);
                }
                else
                {
    				uPMode --;
    				uPMode = uPMode%3; //A35G2_BRC_Casper_0122
                    halFormatter_PowerMode_Set(uPMode);
                }
    		    LOG_MSG(db_APP_INPUTKEY, "\r\n PIP Key : PowerMode-- = %d",uPMode);
    		}
            break;

        case keTEST:
    		{
    			uTPattern++;
    			uTPattern = uTPattern%14;
                LOG_MSG(db_APP_INPUTKEY, "\r\n keTest : uTPattern = %d",uTPattern);
                palFormatterMgr_TestPattern(uTPattern); //T100_Casper_0043
    		}
            break;

        case keOSD:
    		{
    		    if(uTPattern == 0)
                {
    				uTPattern=13;
                    palFormatterMgr_TestPattern(uTPattern); //T100_Casper_0043
                }
                else
                {
    				uTPattern --;
    				uTPattern = uTPattern%14;
                    palFormatterMgr_TestPattern(uTPattern); //T100_Casper_0043
                }
    		    LOG_MSG(db_APP_INPUTKEY, "\r\n keOSD Key : uTPattern-- = %d",uTPattern);
    		}
            break;

        case keHOTKEY:
            LOG_MSG(db_APP_INPUTKEY, "\r\n keHOTKEY : LD Information");
    		appGui_Send_MenuOpen(LD_MENU_ICOUNT);
            break;

        case keSWAP:
    		LOG_MSG(db_APP_INPUTKEY, "\r\n keSWAP : Fan Information");
    		appGui_Send_MenuOpen(FAN_CONTROL_MENU_ICOUNT);
            break;

        case keHELP:
    		LOG_MSG(db_APP_INPUTKEY, "\r\n keHELP : Temperature Information");
    		appGui_Send_MenuOpen(TEMPERATURE_MENU_ICOUNT);
            break;

        case keHOLD_BRIGHT:
    		{
                halFormatter_PW_Index_Get(&uPWIndex);
                uPWIndex+=10;
                LOG_MSG(db_APP_INPUTKEY, "\r\n BRIGHT Key : uPWIndex++ = %d",uPWIndex);
                halFormatter_PW_Index_Set(uPWIndex);
    		}
            break;

        case keHOLD_PIP_SIZE:
    		{
                halFormatter_PW_Index_Get(&uPWIndex);
                uPWIndex-=10;
                LOG_MSG(db_APP_INPUTKEY, "\r\n kePIP_SIZE Key : uPWIndex-- = %d",uPWIndex);
                halFormatter_PW_Index_Set(uPWIndex);
    		}
            break;

        case keHOLD_CONTRAST:
    		{
    			halFormatter_FW_Index_Get(&uFWIndex);
    			uFWIndex+=10;
                LOG_MSG(db_APP_INPUTKEY, "\r\n CONTRAST Key : uFWIndex++ = %d",uFWIndex);
                halFormatter_FW_Index_Set(uFWIndex);
    		}
            break;

        case keHOLD_PIP_LAYOUT:
    		{
    			halFormatter_FW_Index_Get(&uFWIndex);
    			uFWIndex-=10;
                LOG_MSG(db_APP_INPUTKEY, "\r\n CONTRAST Key : uFWIndex-- = %d",uFWIndex);
                halFormatter_FW_Index_Set(uFWIndex);
    		}
            break;

        case keRELEASE_BRIGHT:
    		{
                LOG_MSG(db_APP_INPUTKEY, "\r\n keRELEASE_BRIGHT");
                if(palInputProc_KeyTimerHander(&sInputKey) == TRUE) //A70LV_Larry_0074
                {
                        GuiCb.fpGui_SendKeyEventCb(ucKeyType, keRELEASE_BRIGHT); //A70LV_Larry_0073
                }
    		}
            break;

        case keRELEASE_PIP_SIZE:
    		{
                LOG_MSG(db_APP_INPUTKEY, "\r\n keRELEASE_PIP_SIZE");
                if(palInputProc_KeyTimerHander(&sInputKey) == TRUE) //A70LV_Larry_0074
                {
                        GuiCb.fpGui_SendKeyEventCb(ucKeyType, keRELEASE_PIP_SIZE); //A70LV_Larry_0073
                }
    		}
            break;

        case keRELEASE_CONTRAST:
    		{
                LOG_MSG(db_APP_INPUTKEY, "\r\n keRELEASE_CONTRAST");
                if(palInputProc_KeyTimerHander(&sInputKey) == TRUE) //A70LV_Larry_0074
                {
                        GuiCb.fpGui_SendKeyEventCb(ucKeyType, keRELEASE_CONTRAST); //A70LV_Larry_0073
                }
    		}
            break;

        case keRELEASE_PIP_LAYOUT:
    		{
                LOG_MSG(db_APP_INPUTKEY, "\r\n keRELEASE_PIP_LAYOUT");
                if(palInputProc_KeyTimerHander(&sInputKey) == TRUE) //A70LV_Larry_0074
                {
                        GuiCb.fpGui_SendKeyEventCb(ucKeyType, keRELEASE_PIP_LAYOUT); //A70LV_Larry_0073
                }
    		}
            break;

        default:
            break;
    	//G100_Clare_0006, add, <<<
    	}
}
#endif /* 0 */

//H2 wait review
BOOL palInputProc_OEJig_HotKey(sKEY_DATA *psInputKey)
{
#if 1 //HICC2_Doulas_0032
    sKEY_DATA sInputKey = {0};
    UINT8 ucKeyType = psInputKey->eKeyType;
    tDATA_CODE IsOSDOpen = 0;

    sInputKey.wKeyCode = psInputKey->wKeyCode;
    sInputKey.eKeyEvent = psInputKey->eKeyEvent;
    sInputKey.eKeyType = psInputKey->eKeyType;

    palDataMgr_Data_Access(edcUI_STATUS_IS_OSD_OPEN, edaREAD, &IsOSDOpen);

    switch(sInputKey.wKeyCode)
    {
         case kePOWERON: //A70LV_Larry_0256 modify
		 	{
                LOG_MSG(db_APP_INPUTKEY, "\r\n Key1 PowerMode = Low"); //A70LV_John_0038 fix OE JIG bug

				UINT8 ucData;

				ucData = eCM_POWER_MODE_CONSTANT_POWER;
				palDataMgr_Data_Access(edcPOWER_MODE, edaWRITE_THROUGH_NO_ACTION, (void*)&ucData);

				ucData = 1;
				palDataMgr_Data_Access(edcCONSTANT_POWER_NUMBER, edaWRITE_THROUGH_NO_ACTION, (void*)&ucData);

				//ucData = eCM_COLOR_WHEEL_SPEED_2X;
				//palDataMgr_Data_Access(edcCOLOR_WHEEL_SPEED, edaWRITE_THROUGH_NO_ACTION, (void*)&ucData);

				LOG_MSG(db_APP_INPUTKEY, "\r\nPower On Key"); //A70LV_Larry_0083

                if(sInputKey.eKeyType == eKEYINPUT_KEYPAD)
                {
                    palSystem_PowerDown();
                }

         	}
            break;

        case keSTANDBY: //A70LV_Larry_0083 //A70LV_Larry_0256
            {
                palSystem_PowerDown();
            }
            break;

        case ke1:
            LOG_MSG(db_APP_INPUTKEY, "\r\n Key1 dim power");
            if(IsOSDOpen)
            {
                if(palInputProc_KeyTimerHander(&sInputKey) == TRUE) //A70LV_Larry_0074
                {
                    palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010 //HICC2_Doulas_0032
                }
            }
            else
            {
                palDataMgr_LightSource_Set(12, 100); //12 is eCM_POWER_MODE_30
            	//halFormatter_PowerMode_Set(4);
				//halFormatter_SystemModeSet(12);
            }
            break;

        case ke2:
            LOG_MSG(db_APP_INPUTKEY, "\r\n Key2 power 30");
            if(IsOSDOpen) //T100_Sander_0016
            {
                if(palInputProc_KeyTimerHander(&sInputKey) == TRUE) //A70LV_Larry_0074
                {
                    palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010 //HICC2_Doulas_0032
                }
            }
            else
            {
                palDataMgr_LightSource_Set(11, 100);  //11 is eCM_POWER_MODE_50
                //ucData = 1;
                //palDataMgr_Data_Access(edcCONSTANT_POWER_NUMBER, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&ucData);
            	//halFormatter_PowerMode_Set(3);
				//halFormatter_SystemModeSet(12); //A35G2_BRC_Casper_0090
            }
            break;

        case ke3:
            LOG_MSG(db_APP_INPUTKEY, "\r\n Key3 power 100");
            if(IsOSDOpen)
            {
                if(palInputProc_KeyTimerHander(&sInputKey) == TRUE)
                {
                    palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010 //HICC2_Doulas_0032
                }
            }
            else
            {
                palDataMgr_LightSource_Set(10, 100); //13 is eCM_POWER_MODE_10
            	//halFormatter_PowerMode_Set(4);
				//halFormatter_SystemModeSet(12);
            }
            break;

        case ke4: //A70LV_Larry_0256
            LOG_MSG(db_APP_INPUTKEY, "\r\n Key4");
            if(IsOSDOpen)
            {
                if(palInputProc_KeyTimerHander(&sInputKey) == TRUE)
                {
                    palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010 //HICC2_Doulas_0032
                }
            }
            else
            {
                UINT8 ucTP;
                LOG_MSG(db_APP_INPUTKEY, "\r\n Test Pattern White");

                ucTP = eCM_FACTORY_TP_WHITE; //HICC2_Casper_0022
                palDataMgr_Data_Access(edcFACTORY_TEST_PATTERN, edaWRITE_THROUGH_WITH_ACTION, &ucTP);
        	}
            break;

        case ke5:
            LOG_MSG(db_APP_INPUTKEY, "\r\n Key5");
            if(IsOSDOpen)
            {
                if(palInputProc_KeyTimerHander(&sInputKey) == TRUE)
                {
                    palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010 //HICC2_Doulas_0032
                }
            }
            else
            {
                UINT8 ucTP;
                LOG_MSG(db_APP_INPUTKEY, "\r\n Test Pattern Full Screen");

                ucTP = eCM_FACTORY_TP_FULL_SREEN_W;//eCM_FACTORY_TP_WRGB64;//eFACTORY_TP_FULL_SREEN_W; //HICC2_Doulas_0125//HICC2_Casper_0022
                palDataMgr_Data_Access(edcFACTORY_TEST_PATTERN, edaWRITE_THROUGH_WITH_ACTION, &ucTP);
        	}
            break;

        case ke6: //A70LV_Larry_0256
            LOG_MSG(db_APP_INPUTKEY, "\r\n Key6");
            if(IsOSDOpen)
            {
                if(palInputProc_KeyTimerHander(&sInputKey) == TRUE)
                {
                    palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010 //HICC2_Doulas_0032
                }
            }
            else
            {
                UINT8 ucTP;
                LOG_MSG(db_APP_INPUTKEY, "\r\n Test Pattern Red");

                ucTP = eCM_FACTORY_TP_RED; //HICC2_Casper_0022
                palDataMgr_Data_Access(edcFACTORY_TEST_PATTERN, edaWRITE_THROUGH_WITH_ACTION, &ucTP);
        	}
            break;

        case ke7:
            LOG_MSG(db_APP_INPUTKEY, "\r\n Key7");
            #if 1//HICC2_Doulas_0032
            palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010
            #else
            if(appGui_Get_MenuState_IsOSD_Open())
            {
                if(palInputProc_KeyTimerHander(&sInputKey) == TRUE)
                {
                    palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010 //HICC2_Doulas_0032
                }
            }
            else
            {
                LOG_MSG(db_APP_INPUTKEY, "\r\n LD Information");
                appGui_Send_MenuOpen(LD_MENU_ICOUNT);
            }
            #endif
            break;

        case ke8:
            LOG_MSG(db_APP_INPUTKEY, "\r\n Key8");
            #if 1//HICC2_Doulas_0032
            palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010
            #else
            if(appGui_Get_MenuState_IsOSD_Open()) //T100_Sander_0016
            {
                if(palInputProc_KeyTimerHander(&sInputKey) == TRUE) //A70LV_Larry_0074
                {
                    palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010 //HICC2_Doulas_0032
                }
            }
            else
            {
                LOG_MSG(db_APP_INPUTKEY, "\r\n Light Sensor Info");
                appGui_Send_MenuOpen(ABC_FUNCTION_MENU_ICOUNT);
            }
            #endif
            break;

        case ke9:
            LOG_MSG(db_APP_INPUTKEY, "\r\n Key9");
            if(IsOSDOpen) //T100_Sander_0016
            {
                if(palInputProc_KeyTimerHander(&sInputKey) == TRUE) //A70LV_Larry_0074
                {
                    palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010 //HICC2_Doulas_0032
                }
            }
            else
            {
                UINT8 ucTP;
                LOG_MSG(db_APP_INPUTKEY, "\r\n Test Pattern Red");

                //HICC2_Doulas_0032 remove //no RLD
                //ucTP = eCM_FACTORY_TP_RED; //HICC2_Casper_0022
                //palDataMgr_Data_Access(edcFACTORY_TEST_PATTERN, edaWRITE_THROUGH_WITH_ACTION, &ucTP);
                //halLDCtrl_LD_GroupEnable(0, FALSE);
                //halLDCtrl_LD_GroupEnable(1, FALSE);
                //halLDCtrl_LD_GroupEnable(2, TRUE);
            }
            break;

        case ke0:
            if(IsOSDOpen)
            {
                if(palInputProc_KeyTimerHander(&sInputKey) == TRUE)
                {
                    palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010 //HICC2_Doulas_0032
                }
            }
            else
            {
                UINT8 ucTP;
                LOG_MSG(db_APP_INPUTKEY, "\r\n Test Pattern Black");

                ucTP = eCM_FACTORY_TP_BLACK; //HICC2_Casper_0022
                palDataMgr_Data_Access(edcFACTORY_TEST_PATTERN, edaWRITE_THROUGH_WITH_ACTION, &ucTP);
        	}
            break;

        /* Hotkey - end */
        // A70LV_Eric.C_0012 End

        /* Menu - start */ //to be confirmed
        case keMENU:
            LOG_MSG(db_APP_INPUTKEY, "\r\nMenu Key");

            if(palInputProc_KeyTimerHander(&sInputKey) == TRUE) //A70LV_Larry_0074
            {
            	//if(palInputProc_ADV_WarpBackCheck() == FALSE)		//G100_Doulas_0027 Modify
            	if(!palInputProc_DDP_PatternCheck()) //HICC2_Doulas_0108
				{
                	palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010 //HICC2_Doulas_0032
				}

                if(palDataMgr_CurTestPatternGet() != eTID_OFF) //A70LV_Larry_0296 //A70LV_Larry_0353
                {
                    UINT8 uTP = (UINT8)eCM_TEST_PATTERN_OFF;
                    palDataMgr_Data_Access(edcSERVICE_TEST_PATTERN, edaWRITE_RAM_ONLY_WITH_ACTION, &uTP);
                }
            }
            break;
        case keUP:
            LOG_MSG(db_APP_INPUTKEY, "\r\nUp Key");
            if(palInputProc_KeyTimerHander(&sInputKey) == TRUE) //A70LV_Larry_0074
            {
                //G100_Wilsonj_0067 Start
                //UINT8 ucServiceCode = 0;
                //INT32 ucMAX_Number = 0;

                //palDataMgr_Data_Access(edcSERVICE_CODE_INFO2, edaREAD, &ucServiceCode);
                //palDataMgr_Data_Range_Get(edcSERVICE_CODE_INFO2, edrMAX, &ucMAX_Number);

                #if 1//def CUSTOM_CHRISTIE //###
                #else
                if((appGui_CurrentMenuIndexGet() == SERVICE_CODE_INPUT_MENU_ICOUNT) && (sInputKey.eKeyType == eKEYINPUT_KEYPAD) /*&& (ucServiceCode < (UINT8)ucMAX_Number)*/)
                {
                    sInputKey.wKeyCode = ek0; //HICC2_Doulas_0032
                    palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010 //HICC2_Doulas_0032
                }
                else
                #endif
                {
					//if(palInputProc_ADV_WarpDirectionCheck(DIR__UP) == FALSE)	//G100_Doulas_0027 Modify
					{
                    	palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010 //HICC2_Doulas_0032
					}
                }
                //G100_Wilsonj_0067 End
            }
            break;
        case keDOWN:
        {
            LOG_MSG(db_APP_INPUTKEY, "\r\nDown Key");
            if(palInputProc_KeyTimerHander(&sInputKey) == TRUE) //A70LV_Larry_0074
            {
                //G100_Wilsonj_0067 Start
                //UINT8 ucServiceCode = 0;
                //INT32 ucMAX_Number = 0;

                //palDataMgr_Data_Access(edcSERVICE_CODE_INFO2, edaREAD, &ucServiceCode);
                //palDataMgr_Data_Range_Get(edcSERVICE_CODE_INFO2, edrMAX, &ucMAX_Number);
                {
                	//if(palInputProc_ADV_WarpDirectionCheck(DIR__DOWN) == FALSE)	//G100_Doulas_0027 Modify
                	{
                    	palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010 //HICC2_Doulas_0032
                	}
                }
                //G100_Wilsonj_0067 End
            }
        } //A70LV_John_0038 end
            break;
        case keLEFT:
            LOG_MSG(db_APP_INPUTKEY, "\r\nLeft Key");
            if(palInputProc_KeyTimerHander(&sInputKey) == TRUE) //A70LV_Larry_0074
            {
                //G100_Wilsonj_0067 Start
                //UINT8 ucServiceCode = 0;
                //INT32 ucMAX_Number = 0;

                //palDataMgr_Data_Access(edcSERVICE_CODE_INFO2, edaREAD, &ucServiceCode);
                //palDataMgr_Data_Range_Get(edcSERVICE_CODE_INFO2, edrMAX, &ucMAX_Number);
                {
                	//if(palInputProc_ADV_WarpDirectionCheck(DIR__LEFT) == FALSE)	//G100_Doulas_0027 Modify
                	{
                    	palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010 //HICC2_Doulas_0032
                	}
                }
                //G100_Wilsonj_0067 End
            }
            break;
        case keRIGHT:
            LOG_MSG(db_APP_INPUTKEY, "\r\nRight Key");
            if(palInputProc_KeyTimerHander(&sInputKey) == TRUE) //A70LV_Larry_0074
            {
                //G100_Wilsonj_0067 Start
                //UINT8 ucServiceCode = 0;
                //INT32 ucMAX_Number = 0;

                //palDataMgr_Data_Access(edcSERVICE_CODE_INFO2, edaREAD, &ucServiceCode);
                //palDataMgr_Data_Range_Get(edcSERVICE_CODE_INFO2, edrMAX, &ucMAX_Number);
                {
                	//if(palInputProc_ADV_WarpDirectionCheck(DIR__RIGHT) == FALSE)	//G100_Doulas_0027 Modify
                	{
                    	palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010 //HICC2_Doulas_0032
                	}
                }
                //G100_Wilsonj_0067 End
            }
            break;
        case keENTER:
            LOG_MSG(db_APP_INPUTKEY, "\r\nEnter Key");
            if(palInputProc_KeyTimerHander(&sInputKey) == TRUE) //A70LV_Larry_0074
            {
            	//if(palInputProc_ADV_WarpEnterCheck() == FALSE)		//G100_Doulas_0027 Modify
				{
                	palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010 //HICC2_Doulas_0032
				}
            }
            break;
        case keBACK:
            LOG_MSG(db_APP_INPUTKEY, "\r\nBack Key");
            if(palInputProc_KeyTimerHander(&sInputKey) == TRUE) //A70LV_Larry_0074
            {
            	//if(palInputProc_ADV_WarpBackCheck() == FALSE)		//G100_Doulas_0027 Modify
            	if(!palInputProc_DDP_PatternCheck()) //HICC2_Doulas_0108
				{
                	palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010 //HICC2_Doulas_0032
				}

                if(palDataMgr_CurTestPatternGet() != eTID_OFF) //A70LV_Larry_0296
                {
                    UINT8 uTP = (UINT8)eCM_TEST_PATTERN_OFF;
                    palDataMgr_Data_Access(edcSERVICE_TEST_PATTERN, edaWRITE_RAM_ONLY_NO_ACTION, &uTP);
                }
            }
            break;
        /* Menu - end */

        /* Lens - start */  //Eric.C OK
        case keZOOMIN:
            LOG_MSG(db_APP_INPUTKEY, "\r\n keZOOMIN"); //A70LV_John_0038 fix OE JIG bug
            palMotorZoomSet(eZOOMLENS_CMD_ZOOM_IN_STEP);
            break;
        case keZOOMOUT:
            LOG_MSG(db_APP_INPUTKEY, "\r\n keZOOMOUT"); //A70LV_John_0038 fix OE JIG bug
            palMotorZoomSet(eZOOMLENS_CMD_ZOOM_OUT_STEP);
            break;
        case keFOCUSUP:
            LOG_MSG(db_APP_INPUTKEY, "\r\n keFOCUSUP"); //A70LV_John_0038 fix OE JIG bug
            palMotorFocusSet(eZOOMLENS_CMD_FOCUS_NEAR_STEP);
            break;
        case keFOCUSDOWN:
            LOG_MSG(db_APP_INPUTKEY, "\r\n keFOCUSDOWN"); //A70LV_John_0038 fix OE JIG bug
            palMotorFocusSet(eZOOMLENS_CMD_FOCUS_FAR_STEP);
            break;
        case keLENSHLEFT:
            LOG_MSG(db_APP_INPUTKEY, "\r\n keLENSHLEFT"); //A70LV_John_0038 fix OE JIG bug
            palMotorLensSet(eLENS_CMDS_STEP_LEFT);
            break;
        case keLENSHRIGHT:
            LOG_MSG(db_APP_INPUTKEY, "\r\n keLENSHRIGHT"); //A70LV_John_0038 fix OE JIG bug
            palMotorLensSet(eLENS_CMDS_STEP_RIGHT);
            break;
        case keLENSVUP:
            LOG_MSG(db_APP_INPUTKEY, "\r\n keLENSVUP"); //A70LV_John_0038 fix OE JIG bug
            palMotorLensSet(eLENS_CMDS_STEP_UP);
            break;
        case keLENSVDOWN:
            LOG_MSG(db_APP_INPUTKEY, "\r\n keLENSVDOWN"); //A70LV_John_0038 fix OE JIG bug
            palMotorLensSet(eLENS_CMDS_STEP_DOWN);
            break;

        case keHOLD_ZOOMIN:
            LOG_MSG(db_APP_INPUTKEY, "\r\n keHOLD_ZOOMIN"); //A70LV_John_0038 fix OE JIG bug
            palMotorZoomSet(eZOOMLENS_CMD_ZOOM_IN_RUN);
            break;
        case keHOLD_ZOOMOUT:
            LOG_MSG(db_APP_INPUTKEY, "\r\n keHOLD_ZOOMOUT"); //A70LV_John_0038 fix OE JIG bug
            palMotorZoomSet(eZOOMLENS_CMD_ZOOM_OUT_RUN);
            break;
        case keHOLD_FOCUSUP:
            LOG_MSG(db_APP_INPUTKEY, "\r\n keHOLD_FOCUSUP"); //A70LV_John_0038 fix OE JIG bug
            palMotorFocusSet(eZOOMLENS_CMD_FOCUS_NEAR_RUN);
            break;
        case keHOLD_FOCUSDOWN:
            LOG_MSG(db_APP_INPUTKEY, "\r\n keHOLD_FOCUSDOWN"); //A70LV_John_0038 fix OE JIG bug
            palMotorFocusSet(eZOOMLENS_CMD_FOCUS_FAR_RUN);
            break;
        case keHOLD_LENSHLEFT:
            LOG_MSG(db_APP_INPUTKEY, "\r\n keHOLD_LENSHLEFT"); //A70LV_John_0038 fix OE JIG bug
            palMotorLensSet(eLENS_CMDS_RUN_LEFT);
            break;
        case keHOLD_LENSHRIGHT:
            LOG_MSG(db_APP_INPUTKEY, "\r\n keHOLD_LENSHRIGHT"); //A70LV_John_0038 fix OE JIG bug
            palMotorLensSet(eLENS_CMDS_RUN_RIGHT);
            break;
        case keHOLD_LENSVUP:
            LOG_MSG(db_APP_INPUTKEY, "\r\n keHOLD_LENSVUP"); //A70LV_John_0038 fix OE JIG bug
            palMotorLensSet(eLENS_CMDS_RUN_UP);
            break;
        case keHOLD_LENSVDOWN:
            LOG_MSG(db_APP_INPUTKEY, "\r\n keHOLD_LENSVDOWN"); //A70LV_John_0038 fix OE JIG bug
            palMotorLensSet(eLENS_CMDS_RUN_DOWN);
            break;

        case keINFO:
            LOG_MSG(db_APP_INPUTKEY, "\r\n keINFO : Lens Calibration "); //A70LV_John_0038 fix OE JIG bug
            palMotorLensSet(eLENS_CMDS_CALIBRAION);
            break;
        /* Lens - end */

        /* Others - start */
        case keAUTO:
            LOG_MSG(db_APP_INPUTKEY, "\r\nAuto Key");
            if(palInputProc_KeyTimerHander(&sInputKey) == TRUE) //A70LV_Larry_0074
            {
                palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010 //HICC2_Doulas_0032
            }
            break;

        case keINPUT:
            {
                UINT8 InputKey = eSOURCE_KEY_CHANGE_SOURCE;
                eEXEC_CODE eResult = eEXEC_CODE_PASS;
				//uOPD_DATA uOPDData = {0};

                LOG_MSG(db_APP_INPUTKEY, "\r\nInput Key");

                if(palDataMgr_PIN_Protect_Checking() == ePASSWORD_PROTECT_LOCKED)   //A70LV_Doulas_0222 Add
                {
                    break;
                }

                //eResult = palDataMgr_Data_Access(edcINPUT_KEY, edaREAD, &InputKey);

                if(palDataMgr_DataCode_Control(edcINPUT_KEY) == eFUNC_CONTROL_ENABLE) //A70LV_Larry_0298
                {
                    eResult = palDataMgr_Data_Access(edcINPUT_KEY, edaREAD, &InputKey) ;
                }
                #if defined(CUSTOM_BARCO)
                else if( palDataMgr_Auto_HDMI_Switch_RAM_Get() == ets_ON ) //A35G2_BRC_Casper_0106
                {
                    palDataMgr_UI_EventSend(edcUI_EVENT_SOURCE_MESSAGE, eSOURCE_MESSAGE_NOT_SUPPORTED, NULL); //GuiCb.fpGui_SendSourceMessageCb(eSOURCE_MESSAGE_NOT_SUPPORTED);
                }
                #endif
                else
                {
                    InputKey = eSOURCE_KEY_CHANGE_SOURCE;
                }

                if(eResult != eEXEC_CODE_PASS)
                    break;

                #ifdef CUSTOM_CHRISTIE
                {
                    if(InputKey == eSOURCE_KEY_LIST_ALL_SOURCE) //A70LV_Larry_0109
                    {
                        UINT16 uiInput = 0;

                        halFrontEndCtrl_INPUT_SOURCE_DETECT_Get(&uiInput);
                        palDataMgr_Data_Access(edcSOURCE_INFO, edaWRITE_RAM_ONLY_NO_ACTION, &uiInput);

                        //appGui_SendListAllSourceEvent();
                        palDataMgr_UI_EventSend(edcUI_EVENT_LIST_ALL_SOURCE, TRUE, NULL);
                    }
                    else
                    {
                        palDataMgr_MainInputSourceChangeToNext();
                        palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL);        //A70LV_Doulas_0352 Add
                    }
                }
                #else
                palDataMgr_MainInputSourceChangeToNext();
                palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD_IF_NO_MENU, TRUE, NULL);
                #endif

                if(palInputProc_KeyTimerHander(&sInputKey) == TRUE)     //ZU860_Julie_0002 //A35G2_BRC_Casper_0088
                {
                    palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010
                }

				//sprintf(uOPDData.cString, "%s", "keINPUT");
				//utilOPD_EventSet(eOPD_KEY_KEYPAD_LOG + ucKeyType, &uOPDData);  //G100_Julie_0040
            }
            break;
        /* Others - end */

        case keHOLD_LEFT:
            if(palInputProc_KeyTimerHander(&sInputKey) == TRUE) //A70LV_Larry_0083
            {
                palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010 //HICC2_Doulas_0032
            }
            break;

        case keHOLD_RIGHT:
            if(palInputProc_KeyTimerHander(&sInputKey) == TRUE) //A70LV_Larry_0083
            {
                palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010 //HICC2_Doulas_0032
            }
            break;
		//G100_Clare_0006, add, >>>
		case kePATTERN:
			break;

        case keMUTE:
            {
                UINT8 ucData = 0 ;
                palDataMgr_Data_Access(edcPICTURE_MUTE, edaREAD, &ucData);
                ucData = !ucData ;
                palDataMgr_Data_Access(edcPICTURE_MUTE, edaWRITE_RAM_ONLY_WITH_ACTION, &ucData);

                LOG_MSG(db_APP_INPUTKEY, "\r\nMUTE Key");
            }
            break;

        case keBRIGHT: //HICC2_Doulas_0032 Modify
            halLDCtrl_BLD_Bank_Enable(FALSE);
            palLDMgr_Bank_Enable_Set(ePAL_LD_BANK_ENALE_07, TRUE);
            palLDMgr_Bank_Enable_Set(ePAL_LD_BANK_ENALE_08, TRUE);
            palLDMgr_Bank_Enable_Set(ePAL_LD_BANK_ENALE_09, TRUE);
            palLDMgr_Bank_Enable_Set(ePAL_LD_BANK_ENALE_10, TRUE);
            palLDMgr_Bank_Enable_Set(ePAL_LD_BANK_ENALE_11, TRUE);
            palLDMgr_Bank_Enable_Set(ePAL_LD_BANK_ENALE_12, TRUE);
            break;

        case kePIP_SIZE:
            break;

        case keCONTRAST: //HICC2_Doulas_0032 Modify
            halLDCtrl_BLD_Bank_Enable(TRUE);
            break;

        case kePIP_LAYOUT:
            break;

        case keGAMMA: //HICC2_Doulas_0032 Modify
            halLDCtrl_BLD_Bank_Enable(FALSE);
            palLDMgr_Bank_Enable_Set(ePAL_LD_BANK_ENALE_01, TRUE);
            palLDMgr_Bank_Enable_Set(ePAL_LD_BANK_ENALE_02, TRUE);
            palLDMgr_Bank_Enable_Set(ePAL_LD_BANK_ENALE_03, TRUE);
            palLDMgr_Bank_Enable_Set(ePAL_LD_BANK_ENALE_04, TRUE);
            palLDMgr_Bank_Enable_Set(ePAL_LD_BANK_ENALE_05, TRUE);
            palLDMgr_Bank_Enable_Set(ePAL_LD_BANK_ENALE_06, TRUE);
            break;

        case kePIP:
			{
			    if(uPMode == 0)
                {
    				uPMode=2; //A35G2_BRC_Casper_0122
                    palFormatterMgr_PowerMode_Set(uPMode);
                }
                else
                {
    				uPMode --;
    				uPMode = uPMode%3; //A35G2_BRC_Casper_0122
                    palFormatterMgr_PowerMode_Set(uPMode);
                }
    		    LOG_MSG(db_APP_INPUTKEY, "\r\n PIP Key : PowerMode-- = %d",uPMode);
			}
            break;

        case keTEST:
            if(palInputProc_KeyTimerHander(&sInputKey) == TRUE)
            {
                UINT8 ucTP;
                LOG_MSG(db_APP_INPUTKEY, "\r\n keTEST");


                if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(edcOSDTEST_PATTERN))  //available check
                    break;

                if(palDataMgr_Data_Access(edcOSDTEST_PATTERN, edaREAD, &ucTP) == eEXEC_CODE_PASS)
                {
                    ucTP = (UINT8)DataMapping_GetNextCMvalueFromGui(edcOSDTEST_PATTERN, ucTP);

                    if(ucTP >= eCM_TEST_PATTERN_NUMBER)
                    {
                        ucTP = eCM_TEST_PATTERN_OFF; //HICC2_Casper_0019
                    }
                    palDataMgr_Data_Access(edcOSDTEST_PATTERN, edaWRITE_THROUGH_WITH_ACTION, &ucTP);
                }
            }
            break;

        case keOSD:  //HICC2_Doulas_0096
            {
                UINT8 ucMenuShow;
                palDataMgr_Data_Access(edcOSD_SHOW, edaREAD, &ucMenuShow);

                if(ucMenuShow == ets_ON)
                {
                    ucMenuShow = ets_OFF;
                }
                else
                {
                    ucMenuShow = ets_ON;
                }

                palDataMgr_Data_Access(edcOSD_SHOW, edaWRITE_THROUGH_WITH_ACTION, &ucMenuShow) ;
                LOG_MSG(db_APP_INPUTKEY, "\r\nOSD Key");
            }
            break;

        case keHOTKEY:
            LOG_MSG(db_APP_INPUTKEY, "\r\n keHOTKEY : Fan Information");
			//appGui_Send_MenuOpen(FAN_CONTROL_MENU_ICOUNT); //HICC2_Doulas_0032
			palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010 //HICC2_Doulas_0032
            break;

        case keSWAP:
            LOG_MSG(db_APP_INPUTKEY, "\r\n keSWAP : Lens Walk around "); //A70LV_John_0038 fix OE JIG bug
            palMotorLensSet(eLENS_CMDS_MOVING);
            break;

        case keHELP:
			LOG_MSG(db_APP_INPUTKEY, "\r\n keHELP : Temperature Information");
			//appGui_Send_MenuOpen(TEMPERATURE_MENU_ICOUNT); //HICC2_Doulas_0032
			palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010 //HICC2_Doulas_0032
            break;

        case keHOLD_BRIGHT:
            break;

        case keHOLD_PIP_SIZE:
            break;

        case keHOLD_CONTRAST:
            break;

        case keHOLD_PIP_LAYOUT:
            break;

        case keRELEASE_BRIGHT:
            break;

        case keRELEASE_PIP_SIZE:
            break;

        case keRELEASE_CONTRAST:
            break;

        case keRELEASE_PIP_LAYOUT:
            break;

        default:
            break;
		}
#endif
}
#endif /* OE_JIG */


// ==============================================================================
// FUNCTION NAME: palInputProc_Process
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
// 12/11/2013, Leo Create
// --------------------
// ==============================================================================
INT16 palInputProc_Poll(UINT16 uiTick)
{
    sKEY_DATA sInputKey = {0};
    UINT8  ucQuickKeyValue = 0;		//A65_OPTOMA_Doulas_0025//A35G2_Coda_0049

    if(m_cDisableKey || (palSystem_PowerStateGet() != ePOWER_STATE_ACTIVE)) //A65_Owen_0001
    {
        //LOG_MSG(db_APP_INPUTKEY, "%d Key Blocked case 1\n", sInputKey.wKeyCode);
        return INPUT_POLLING_PERIOD / POLL_PERIOD ;
    }

    //halInputCtrl_IR_Decode();

    if(rcSUCCESS == (eRESULT)utilQueueRead(&m_sInputRingBuffer, (UINT8*)&sInputKey))
    {
		UINT8 ucKeyType = sInputKey.eKeyType;

        palInput_Uart_Switch_Process((eKEY_LIST)sInputKey.wKeyCode);

        if(palEnvironment_Fake_Power_Down_Get())
        {
            if(sInputKey.wKeyCode != kePOWERON)
            {
                LOG_MSG(db_APP_INPUTKEY, "%d Key Blocked case 2\n", sInputKey.wKeyCode);
                return INPUT_POLLING_PERIOD / POLL_PERIOD;
            }
        }
        else if(palIllumination_BurnIn_Enable()) //A70LV_Larry_0016 //A70LV_Larry_0111
        {
            if((sInputKey.wKeyCode != keMENU) ||
               (sInputKey.eKeyType != eKEYINPUT_KEYPAD)) //Only "Menu" key from "keypad" work to "burn in" disabled
            {
                LOG_MSG(db_APP_INPUTKEY, "%d Key Blocked case 3\n", sInputKey.wKeyCode);
                return INPUT_POLLING_PERIOD / POLL_PERIOD;
            }
        }
        else //A70LV_Larry_0110
        {
            if((palDataPath_GetDataPathState() == eDATA_PATH_STATE_SUSPENDED) ||
                (palDataPath_GetDataPathState() >= eDATA_PATH_STATE_INVALID))
            {
                LOG_MSG(db_APP_INPUTKEY, "%d Key Blocked case 4 (%d)\n", sInputKey.wKeyCode, palDataPath_GetDataPathState());
                return INPUT_POLLING_PERIOD / POLL_PERIOD ;
            }
            else if(palInputProc_ServiceCodeCheck(&sInputKey) == eEXEC_CODE_PASS)
            {
                return INPUT_POLLING_PERIOD / POLL_PERIOD;
            }
        }

        palKeypadBacklighthandle(&sInputKey); //HICC2_Casper_0039

        if(palInputProc_KeyFilter(&sInputKey) == FALSE) //A70LV_Larry_0081
        {
            LOG_MSG(db_APP_INPUTKEY, "%d Key Blocked case 7 (%d %d %d %d)\n", sInputKey.wKeyCode
                                                                            , palCoreVar_GetLightSourceOnVar()
                                                                            , palDataMgr_Camera_OSD_Lock_Get()
                                                                            , palIllumination_ABP_StatusGet()
                                                                            , palDataMgr_CurTestPatternGet()
                                                                            );
            return INPUT_POLLING_PERIOD / POLL_PERIOD ;
        }

        if(CFG_CUSTOMER_ID == CUSTOMER_ID_OPTOMA)
        {
            if(palEnvironment_SleepTimerGet() <= 60)
            {
                palEnvironment_SleepTimerClear();
            }
        }
        else
        {
            if(palEnvironment_SleepTimerGet() <= 60 && sInputKey.wKeyCode == keBACK)
            {
                palEnvironment_SleepTimerClear();
            }
        }

        //palEnvironment_AutoShutDownClear(); //A70LV_Larry_0082
        UINT8 ucReset = 1;
        palDataMgr_Data_Access(edcAUTO_SHUTDOWN_TIMER_RESET, edaWRITE_THROUGH_WITH_ACTION, &ucReset);

        if(sInputKey.eKeyType == eKEYINPUT_KEYPAD)      //ZU860_Doulas_0080
        {
            sInputKey.wKeyCode = palInputProc_Remap((eKEY_LIST)sInputKey.wKeyCode);
        }

        //A35G2_CDS_Simon_0049
        #if 0   //H2 wait review
        if(sInputKey.wKeyCode != kePOWERON &&
           sInputKey.wKeyCode != keSTANDBY
        )
        {
            if(appGui_MsgQueueRemainSizeIsLow())
            {
                return INPUT_POLLING_PERIOD / POLL_PERIOD ;
            }
        }
        #endif

#ifdef OE_JIG  // A70LV_Eric.C_0001 OEJIG -> OE_JIG
        // A70LV_Eric.C_0001 Start
        LOG_MSG(db_APP_INPUTKEY, "\r\n KeyCode = %d",sInputKey.wKeyCode); //A70LV_John_0038 fix OE JIG bug

        palInputProc_OEJig_HotKey(&sInputKey);

 		// A70LV_Eric.C_0001 End
#else

    	if( TRUE == palInputProc_BIST_KeyCheckFun(sInputKey.wKeyCode)) //HICC2_Steven_0002
    	{
    		return INPUT_POLLING_PERIOD / POLL_PERIOD ;
    	}


        switch(sInputKey.wKeyCode)
        {
                // ==============================================================================
                // Pressed key
                // ==============================================================================
            case kePOWERON:
                {
                    UINT8 ucFactory_TestPattern = eCM_FACTORY_TP_NONE; //HICC2_Casper_0028

					//G100_Clare_0055, add, >>>
                    LOG_MSG(db_APP_INPUTKEY, "\r\nPower On Key");
                    if(palEnvironment_Fake_Power_Down_Get())
                    {
                        LOG_MSG(db_APP_INPUTKEY, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
                        palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010
                    }
					//G100_Clare_0055, add, <<<
                    else if(sInputKey.eKeyType == eKEYINPUT_KEYPAD) //A70LV_Larry_0023
                    {
                        #ifdef NO_POWER_OFF_DURING_CAMERA_WORKING                   //G100_Tim_0051, add, start //G100_Tim_0046
                        if( palDataMgr_Get_Camera_Working_Status() == eCAMERA_STATUS_WORKING )
                        {
                            // not allowed to power off !!!
                            LOG_MSG(db_APP_INPUTKEY, "palInputProc_Poll(), No Power off during camera working, kePOWERON\r\n");
                            break;
                        }
                        #endif //NO_POWER_OFF_DURING_CAMERA_WORKING                 //G100_Tim_0051, add, end //G100_Tim_0046

                    	if(utilWarp_GetOsdPatternType() != PAT_TYPE__OFF)	//G100_Doulas_0041
    					{
    						palGeo_AdvWarpShowOsdPattern(PAT_TYPE__OFF);
    					}

                    	palDataMgr_Data_Access(edcFACTORY_TEST_PATTERN, edaREAD, &ucFactory_TestPattern);
    					if(ucFactory_TestPattern != eCM_FACTORY_TP_NONE) //HICC2_Casper_0028
    					{
    						ucFactory_TestPattern = eCM_FACTORY_TP_NONE;
    						palDataMgr_Data_Access(edcFACTORY_TEST_PATTERN, edaWRITE_RAM_ONLY_WITH_ACTION, &ucFactory_TestPattern);
    					}

                    	sInputKey.wKeyCode = keSTANDBY;
                    	palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010
                    }
                }
                break;

            case keSTANDBY: //A70LV_Larry_0001
                {
                	UINT8 ucFactory_TestPattern = eCM_FACTORY_TP_NONE;  //HICC2_Casper_0019 //HICC2_Casper_0022
                    LOG_MSG(db_APP_INPUTKEY, "\r\nStandby Key");

                    #ifdef NO_POWER_OFF_DURING_CAMERA_WORKING				  //G100_Tim_0046, add, start
					if( palDataMgr_Get_Camera_Working_Status() == eCAMERA_STATUS_WORKING )
					{
					// not allowed to power off !!!
						return INPUT_POLLING_PERIOD / POLL_PERIOD;
					}
					#endif //NO_POWER_OFF_DURING_CAMERA_WORKING 			  //G100_Tim_0046, add, end

					if(utilWarp_GetOsdPatternType() != PAT_TYPE__OFF)	//G100_Doulas_0041
					{
						palGeo_AdvWarpShowOsdPattern(PAT_TYPE__OFF);
					}

                	palDataMgr_Data_Access(edcFACTORY_TEST_PATTERN, edaREAD, &ucFactory_TestPattern);	 //HICC2_Casper_0019
					if(ucFactory_TestPattern != eCM_FACTORY_TP_NONE) //HICC2_Casper_0019 //HICC2_Casper_0022
					{
						ucFactory_TestPattern = eCM_FACTORY_TP_NONE; //HICC2_Casper_0022
						palDataMgr_Data_Access(edcFACTORY_TEST_PATTERN, edaWRITE_RAM_ONLY_WITH_ACTION, &ucFactory_TestPattern);
					}
                    palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010
                }
                break;

            case keMENU:
                LOG_MSG(db_APP_INPUTKEY, "\r\nMenu Key");

                if(palInputProc_KeyTimerHander(&sInputKey) == TRUE) //A70LV_Larry_0074
                {
                    if(palInputProc_ADV_WarpBackCheck())
                    {
                        break;
                    }
                    else if(!palInputProc_DDP_PatternCheck()) //HICC2_Casper_0019
                    {
                    	//if(palInputProc_ADV_WarpBackCheck() == FALSE)
    					{
                            palInputProc_KeyEventSend(sInputKey);
    					}
                    }
                }
                break;

            case keUP:
                LOG_MSG(db_APP_INPUTKEY, "\r\nUp Key");
                if(palInputProc_KeyTimerHander(&sInputKey) == TRUE) //A70LV_Larry_0074
                {
                    if(palInputProc_ADV_WarpDirectionCheck(DIR__UP) == TRUE)	//A35G2_Owen_0002 //A65_OPTOMA_Doulas_0211 Modify//A65_OPTOMA_Doulas_0152
        			{
        				break;
        			}

                    //G100_Wilsonj_0067 Start
                    //UINT8 ucServiceCode = 0;
                    //INT32 ucMAX_Number = 0;

                    //palDataMgr_Data_Access(edcSERVICE_CODE_INFO2, edaREAD, &ucServiceCode);
                    //palDataMgr_Data_Range_Get(edcSERVICE_CODE_INFO2, edrMAX, &ucMAX_Number);

                    {
						//if(palInputProc_ADV_WarpDirectionCheck(DIR__UP) == FALSE)	//G100_Doulas_0027 Modify
						{
                            palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010
						}
                    }
                    //G100_Wilsonj_0067 End
                }
                break;

            case keDOWN: //A70LV_Larry_0248
                LOG_MSG(db_APP_INPUTKEY, "\r\nDown Key");
                if(palInputProc_KeyTimerHander(&sInputKey) == TRUE) //A70LV_Larry_0074
                {
                    if(palInputProc_ADV_WarpDirectionCheck(DIR__DOWN) == TRUE)	//A35G2_Owen_0002 //A65_OPTOMA_Doulas_0211 Modify//A65_OPTOMA_Doulas_0152
        			{
        				break;
        			}
                    //G100_Wilsonj_0067 Start
                    //UINT8 ucServiceCode = 0;
                    //INT32 ucMAX_Number = 0;

                    //palDataMgr_Data_Access(edcSERVICE_CODE_INFO2, edaREAD, &ucServiceCode);
                    //palDataMgr_Data_Range_Get(edcSERVICE_CODE_INFO2, edrMAX, &ucMAX_Number);
                    {
                    	//if(palInputProc_ADV_WarpDirectionCheck(DIR__DOWN) == FALSE)	//G100_Doulas_0027 Modify
                    	{
                            palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010
                    	}
                    }
                    //G100_Wilsonj_0067 End
                }
                break;

            case keLEFT: //A70LV_Larry_0248
                LOG_MSG(db_APP_INPUTKEY, "\r\nLeft Key");
                if(palInputProc_KeyTimerHander(&sInputKey) == TRUE) //A70LV_Larry_0074
                {
                    if(palInputProc_ADV_WarpDirectionCheck(DIR__LEFT) == TRUE)	//A35G2_Owen_0002 //A65_OPTOMA_Doulas_0211 Modify//A65_OPTOMA_Doulas_0152
        			{
        				break;
        			}
                    //G100_Wilsonj_0067 Start
                    //UINT8 ucServiceCode = 0;
                    //INT32 ucMAX_Number = 0;

                    //palDataMgr_Data_Access(edcSERVICE_CODE_INFO2, edaREAD, &ucServiceCode);
                    //palDataMgr_Data_Range_Get(edcSERVICE_CODE_INFO2, edrMAX, &ucMAX_Number);
                    {
                    	//if(palInputProc_ADV_WarpDirectionCheck(DIR__LEFT) == FALSE)	//G100_Doulas_0027 Modify
                    	{
                            palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010
                    	}
                    }
                    //G100_Wilsonj_0067 End
                }
                break;

            case keRIGHT: //A70LV_Larry_0248
                LOG_MSG(db_APP_INPUTKEY, "\r\nRight Key");
                if(palInputProc_KeyTimerHander(&sInputKey) == TRUE) //A70LV_Larry_0074
                {
                    if(palInputProc_ADV_WarpDirectionCheck(DIR__RIGHT) == TRUE)	//A35G2_Owen_0002 //A65_OPTOMA_Doulas_0211 Modify//A65_OPTOMA_Doulas_0152
        			{
        				break;
        			}
                    //G100_Wilsonj_0067 Start
                    //UINT8 ucServiceCode = 0;
                    //INT32 ucMAX_Number = 0;

                    //palDataMgr_Data_Access(edcSERVICE_CODE_INFO2, edaREAD, &ucServiceCode);
                    //palDataMgr_Data_Range_Get(edcSERVICE_CODE_INFO2, edrMAX, &ucMAX_Number);
                    {
                    	//if(palInputProc_ADV_WarpDirectionCheck(DIR__RIGHT) == FALSE)	//G100_Doulas_0027 Modify
                    	{
                            palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010
                    	}
                    }
                    //G100_Wilsonj_0067 End
                }
                break;

            case keENTER:
                LOG_MSG(db_APP_INPUTKEY, "\r\nEnter Key");
                if(palInputProc_KeyTimerHander(&sInputKey) == TRUE) //A70LV_Larry_0074
                {
                	//if(palInputProc_ADV_WarpEnterCheck() == FALSE)		//G100_Doulas_0027 Modify
					{
                        palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010
					}
                }
                break;

            case keHOTKEY:
                LOG_MSG(db_APP_INPUTKEY, "\r\nHOTKEY1 Key");
                palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010
                break;

            case ke0:
                if(palInputProc_KeyTimerHander(&sInputKey) == TRUE)
                {
                    LOG_MSG(db_APP_INPUTKEY, "\r\n Key0");
                    palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010
                }
                break;

            case ke1:
                if(palInputProc_KeyTimerHander(&sInputKey) == TRUE)
                {
                    LOG_MSG(db_APP_INPUTKEY, "\r\n Key1");
                    palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010
                }
                break;

            case ke2: //G100_Steven_0008 start
                if(palInputProc_KeyTimerHander(&sInputKey) == TRUE)
                {
                    LOG_MSG(db_APP_INPUTKEY, "\r\n Key2");
                    palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010
                }
                break;

            case ke3:
                if(palInputProc_KeyTimerHander(&sInputKey) == TRUE)
                {
                    LOG_MSG(db_APP_INPUTKEY, "\r\n Key3");
                    palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010
                }
                break;

            case ke4:
                if(palInputProc_KeyTimerHander(&sInputKey) == TRUE)
                {
                    LOG_MSG(db_APP_INPUTKEY, "\r\n Key4");
                    palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010
                }
                break;

            case ke5:
                if(palInputProc_KeyTimerHander(&sInputKey) == TRUE)
                {
                    LOG_MSG(db_APP_INPUTKEY, "\r\n Key5");
                    palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010
                }
                break;

            case ke6:
#ifndef DEMO_SNDS   //HICC2_Doulas_0021
                if(palInputProc_KeyTimerHander(&sInputKey) == TRUE)
                {
                    LOG_MSG(db_APP_INPUTKEY, "\r\n Key6");
                    palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010
                }
#endif
                break;

            case ke7:
#ifndef DEMO_SNDS   //HICC2_Doulas_0021
                if(palInputProc_KeyTimerHander(&sInputKey) == TRUE)
                {
                    LOG_MSG(db_APP_INPUTKEY, "\r\n Key7");
                    palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010
                }
#endif
                break;

            case ke8:
#ifndef DEMO_SNDS   //HICC2_Doulas_0021
                if(palInputProc_KeyTimerHander(&sInputKey) == TRUE)
                {
                    LOG_MSG(db_APP_INPUTKEY, "\r\n Key8");
                    palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010
                }
#endif
                break;

            case ke9:
                if(palInputProc_KeyTimerHander(&sInputKey) == TRUE)
                {
                    LOG_MSG(db_APP_INPUTKEY, "\r\n Key9");
                    palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010
                }
                break;

            case keMUTE:
                {
                    sDATABASE_ITEM_DATA_FORMAT sInputData;
                    UINT8 ucData = 0 ;
					//uOPD_DATA uOPDData = {0};
                    //if(palDataMgr_PIN_Protect_Checking() != ePASSWORD_PROTECT_UNLOCK)   //A70LV_Doulas_0222 Add //G100_Coda_0093 remove
                    //{
                       // break;
                    //}
                    palDataMgr_Data_Access(edcPICTURE_MUTE, edaREAD, &ucData);
                    ucData = !ucData ;
                    sInputData.uiItemIndex = edcPICTURE_MUTE;
                    sInputData.uValue.lValue = ucData;
                    palDataMgr_NotifyGroupingEvent(sInputData);
                    palDataMgr_Data_Access(edcPICTURE_MUTE, edaWRITE_RAM_ONLY_WITH_ACTION, &ucData);

	                LOG_MSG(db_APP_INPUTKEY, "\r\nMUTE Key");

					//sprintf(uOPDData.cString, "%s", "keMUTE");
					//utilOPD_EventSet(eOPD_KEY_KEYPAD_LOG + ucKeyType, &uOPDData);  //G100_Julie_0040
                }
                break;

            case keZOOMIN:
                {
                    LOG_MSG(db_APP_INPUTKEY, "\r\n keZOOMIN");
                    if(m_cLensMotorLock == eLENS_LOCK_ALL_ALLOW)
                    {
                    	palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010

                        if(m_bMotorZoomEnale) //A70LV_Larry_0219
                            palMotorZoomSet(eZOOMLENS_CMD_ZOOM_IN_STEP);
                    }
                }
                break;

            case keZOOMOUT:
                {
                    LOG_MSG(db_APP_INPUTKEY, "\r\n keZOOMOUT");
                    if(m_cLensMotorLock == eLENS_LOCK_ALL_ALLOW)
                    {
                    	palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010

                        if(m_bMotorZoomEnale) //A70LV_Larry_0219
                            palMotorZoomSet(eZOOMLENS_CMD_ZOOM_OUT_STEP);
                    }
                }
                break;

            case keFOCUSUP:
            {
                LOG_MSG(db_APP_INPUTKEY, "\r\n keFOCUSUP");
                if(m_cLensMotorLock == eLENS_LOCK_ALL_ALLOW)
                {
                	palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010

                    if(m_bMotorFocusEnale) //A70LV_Larry_0219
                    {
					    #ifdef CORRECT_FOCUS_DIRECTION  //A35G2_Simon_0109
                        palMotorFocusSet(eZOOMLENS_CMD_FOCUS_FAR_STEP);
                        #else
                        palMotorFocusSet(eZOOMLENS_CMD_FOCUS_NEAR_STEP);
                        #endif
                    }
                }
            }
            break;

            case keFOCUSDOWN:
            {
                LOG_MSG(db_APP_INPUTKEY, "\r\n keFOCUSDOWN");
                if(m_cLensMotorLock == eLENS_LOCK_ALL_ALLOW)
                {
                	palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010

                    if(m_bMotorFocusEnale) //A70LV_Larry_0219
                    {
					    #ifdef CORRECT_FOCUS_DIRECTION  //A35G2_Simon_0109
                        palMotorFocusSet(eZOOMLENS_CMD_FOCUS_NEAR_STEP);
                        #else
                        palMotorFocusSet(eZOOMLENS_CMD_FOCUS_FAR_STEP);
                        #endif
                    }
                }
            }
            break;

            case keLENSHLEFT:
            {
                LOG_MSG(db_APP_INPUTKEY, "\r\n keLENSHLEFT");
                if(m_cLensMotorLock == eLENS_LOCK_ALL_ALLOW)
                {
                		palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010
                        #if 0 // No Lens ID for X35Gen2
                        if(palEnvironment_LensModelCheck_Get() && palEnvironment_LensCalFlag_Get())
                        #else
                        if(palEnvironment_LensCalFlag_Get())
                        #endif
                        {
                            if(m_bMotorLensEnale) //A70LV_Larry_0219
                                palMotorLensSet(eLENS_CMDS_STEP_LEFT);
                        }

                }
            }
            break;

            case keLENSHRIGHT:
            {
                LOG_MSG(db_APP_INPUTKEY, "\r\n keLENSHRIGHT");
                if(m_cLensMotorLock == eLENS_LOCK_ALL_ALLOW)
                {
                	palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010
                #if 0 // No Lens ID for X35Gen2
                    if(palEnvironment_LensModelCheck_Get() && palEnvironment_LensCalFlag_Get())
                #else
                    if(palEnvironment_LensCalFlag_Get())
                #endif
                    {
                        if(m_bMotorLensEnale) //A70LV_Larry_0219
                            palMotorLensSet(eLENS_CMDS_STEP_RIGHT);
                    }
                }
            }
            break;

            case keLENSVUP:
                {
                    LOG_MSG(db_APP_INPUTKEY, "\r\n keLENSVUP");
                    if(m_cLensMotorLock == eLENS_LOCK_ALL_ALLOW)
                    {
                    	palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010
                    #if 0 // No Lens ID for X35Gen2
                        if(palEnvironment_LensModelCheck_Get() && palEnvironment_LensCalFlag_Get())
                    #else
                        if(palEnvironment_LensCalFlag_Get())
                    #endif
                        {
                            if(m_bMotorLensEnale) //A70LV_Larry_0219
                                palMotorLensSet(eLENS_CMDS_STEP_UP);
                        }
                    }
                }
                break;

            case keLENSVDOWN:
                {
                    LOG_MSG(db_APP_INPUTKEY, "\r\n keLENSVDOWN");
                    if(m_cLensMotorLock == eLENS_LOCK_ALL_ALLOW)
                    {
                    	palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010
                    #if 0 // No Lens ID for X35Gen2
                        if(palEnvironment_LensModelCheck_Get() && palEnvironment_LensCalFlag_Get())
                    #else
                        if(palEnvironment_LensCalFlag_Get())
                    #endif
                        {
                            if(m_bMotorLensEnale) //A70LV_Larry_0219
                            palMotorLensSet(eLENS_CMDS_STEP_DOWN);
                        }
                    }
                }
                break;

            case keINFO:
                LOG_MSG(db_APP_INPUTKEY, "\r\n keINFO");
                palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010
                break;

            case keSCALAR_NMI:
                break;

            case keISP_PWR:
                LOG_MSG(db_APP_INPUTKEY, "\r\nISP PWR Key");
                break;

            case keISP:
                LOG_MSG(db_APP_INPUTKEY, "\r\nISP Key");
                break;

            case keOSD: //A70LV_Larry_0111 //A70LV_Larry_0248
                {
                    LOG_MSG(db_APP_INPUTKEY, "\r\nOSD Key");

                    if(palDataMgr_DataCode_Control(edcOSD_SHOW) == eFUNC_CONTROL_ENABLE) //A70Gen2_Julie_0031
                    {
                        palInputProc_KeyEventSend(sInputKey);
                    }
                }
                break;

            case keFOCUS:
                {
                    LOG_MSG(db_APP_INPUTKEY, "\r\nFocus Key");

                    if(m_cLensMotorLock == eLENS_LOCK_ALL_ALLOW)
                    {
                        palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010
                    }
                }
                break;

            case keINPUT:   //A70LV_Doulas_0082 Modify //A70LV_Larry_0248
                {
                    UINT8 InputKey = eSOURCE_KEY_CHANGE_SOURCE;
                    eEXEC_CODE eResult = eEXEC_CODE_PASS;
					//uOPD_DATA uOPDData = {0};

                    LOG_MSG(db_APP_INPUTKEY, "\r\nInput Key");

                    if(palDataMgr_PIN_Protect_Checking() == ePASSWORD_PROTECT_LOCKED)   //A70LV_Doulas_0222 Add
                    {
                        break;
                    }

                    //eResult = palDataMgr_Data_Access(edcINPUT_KEY, edaREAD, &InputKey);
#ifndef DEMO_SNDS   //HICC2_Doulas_0021
                    if(palDataMgr_DataCode_Control(edcINPUT_KEY) == eFUNC_CONTROL_ENABLE) //A70LV_Larry_0298
                    {
                        eResult = palDataMgr_Data_Access(edcINPUT_KEY, edaREAD, &InputKey) ;
                    }
                    #if defined(CUSTOM_BARCO)
                    else if( palDataMgr_Auto_HDMI_Switch_RAM_Get() == ets_ON ) //A35G2_BRC_Casper_0106
                    {
                        palDataMgr_UI_EventSend(edcUI_EVENT_SOURCE_MESSAGE, eSOURCE_MESSAGE_NOT_SUPPORTED, NULL); //GuiCb.fpGui_SendSourceMessageCb(eSOURCE_MESSAGE_NOT_SUPPORTED);
                    }
                    #endif
                    else
                    {
                        InputKey = eSOURCE_KEY_CHANGE_SOURCE;
                    }

                    if(eResult != eEXEC_CODE_PASS)
                        break;

                    #ifdef CUSTOM_CHRISTIE
                    {
                        if(InputKey == eSOURCE_KEY_LIST_ALL_SOURCE) //A70LV_Larry_0109
                        {
                            UINT16 uiInput = 0;

                            halFrontEndCtrl_INPUT_SOURCE_DETECT_Get(&uiInput);
                            palDataMgr_Data_Access(edcSOURCE_INFO, edaWRITE_RAM_ONLY_NO_ACTION, &uiInput);

                            //appGui_SendListAllSourceEvent();
                            palDataMgr_UI_EventSend(edcUI_EVENT_LIST_ALL_SOURCE, TRUE, NULL);
                        }
                        else
                        {
                            palDataMgr_MainInputSourceChangeToNext();
                            palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL);        //A70LV_Doulas_0352 Add
                        }
                    }
                    #else
                    palDataMgr_MainInputSourceChangeToNext();
                    palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD_IF_NO_MENU, TRUE, NULL);
                    #endif

                    if(palInputProc_KeyTimerHander(&sInputKey) == TRUE)     //ZU860_Julie_0002 //A35G2_BRC_Casper_0088
                    {
                        palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010
                    }

					//sprintf(uOPDData.cString, "%s", "keINPUT");
					//utilOPD_EventSet(eOPD_KEY_KEYPAD_LOG + ucKeyType, &uOPDData);  //G100_Julie_0040
#endif
                }
                break;

            case keAUTO:
                LOG_MSG(db_APP_INPUTKEY, "\r\nAuto Key");
                if(palInputProc_KeyTimerHander(&sInputKey) == TRUE) //A70LV_Larry_0074
                {
                    palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010
                }
                break;

            case keSHUTTER:
                LOG_MSG(db_APP_INPUTKEY, "\r\nShutter Key");
                break;

            case keLENS:
                {
                    LOG_MSG(db_APP_INPUTKEY, "\r\nLens Key");

                    if(m_cLensMotorLock == eLENS_LOCK_ALL_ALLOW)
                    {
                    	palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010
                    }
                }
                break;

            case keBACK:
                LOG_MSG(db_APP_INPUTKEY, "\r\nBack Key");
                if(palInputProc_KeyTimerHander(&sInputKey) == TRUE) //A70LV_Larry_0074
                {
                    if(palInputProc_ADV_WarpBackCheck())
                    {
                        break;
                    }
                    else if(!palInputProc_DDP_PatternCheck()) //HICC2_Casper_0019
					{
	                	//if(palInputProc_ADV_WarpBackCheck() == FALSE)		//G100_Doulas_0027 Modify
						{
                            palInputProc_KeyEventSend(sInputKey);
						}
	                }
                }
                break;

            case keHELP:
                LOG_MSG(db_APP_INPUTKEY, "\r\nHelp Key");
                palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010
                break;

            case keZOOM:
                {
                    LOG_MSG(db_APP_INPUTKEY, "\r\nZoom Key");

                    if(m_cLensMotorLock == eLENS_LOCK_ALL_ALLOW)
                    {
                        palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010
                    }
                }
                break;

            case keGAMMA: //A70LV_Larry_0108
                LOG_MSG(db_APP_INPUTKEY, "\r\nGamma Key");
                #ifdef HOTKET_FOR_TEST_AF_AC_ACU_MESSAGE
                palDataMgr_UI_EventSend(edcUI_EVENT_AF_CAL_FAIL, TRUE, NULL);
                #else
                palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010
                #endif
                break;

            case keBRIGHT: //A70LV_Larry_0108
                LOG_MSG(db_APP_INPUTKEY, "\r\nBright Key");
                #ifdef HOTKET_FOR_TEST_AF_AC_ACU_MESSAGE
                palDataMgr_UI_EventSend(edcUI_EVENT_AC_CAL_FAIL, TRUE, NULL);
                #else
                palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010
                #endif
                break;

            case keID: //A70LV_Larry_0108
                LOG_MSG(db_APP_INPUTKEY, "\r\nIDt Key");
                palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010
                break;

            case keCONTRAST: //A70LV_Larry_0108
                LOG_MSG(db_APP_INPUTKEY, "\r\nContrast Key");
                #ifdef HOTKET_FOR_TEST_AF_AC_ACU_MESSAGE
                palDataMgr_UI_EventSend(edcUI_EVENT_ACU_CAL_FAIL, TRUE, NULL);
                #else
                palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010
                #endif
                break;

            case keHKEYSTONE_LEFT: //A70LV_Larry_0148
            {
            	LOG_MSG(db_APP_INPUTKEY, "\r\nkeHKEYSTONE_LEFT");

            	if( (palGeo_ApLinkFlag_Get() == TRUE) || 				//A70Gen2_Steven_0005 fixed ISS-0008137 // R70CDS_Bruce#0087
            		(palDataMgr_CurrentWarpMemoryType() == WARPING_TYPE_AP || palDataMgr_CurrentWarpMemoryType() == WARPING_TYPE_AP_MYSTIQUE) )// R70CDS_Bruce#0084
            	{
                    UINT8 cMsg = eSOURCE_MESSAGE_NOT_SUPPORTED;
                    palDataMgr_Data_Access(edcSUPPORT_MESSAGE, edaWRITE_THROUGH_WITH_ACTION, &cMsg);
                    palDataMgr_UI_EventSend(edcUI_EVENT_SOURCE_MESSAGE, cMsg, NULL);
            	}
            	else
            	{
                    palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010
            	}
            }
                break;

            case keHKEYSTONE_RIGHT: //A70LV_Larry_0148
            {
                LOG_MSG(db_APP_INPUTKEY, "\r\nkeHKEYSTONE_RIGHT");
                if( (palGeo_ApLinkFlag_Get() == TRUE) || 				 //A70Gen2_Steven_0005  fixed ISS-0008137 // R70CDS_Bruce#0087
                	(palDataMgr_CurrentWarpMemoryType() == WARPING_TYPE_AP || palDataMgr_CurrentWarpMemoryType() == WARPING_TYPE_AP_MYSTIQUE) )// R70CDS_Bruce#0084
                {
                    UINT8 cMsg = eSOURCE_MESSAGE_NOT_SUPPORTED;
                    palDataMgr_Data_Access(edcSUPPORT_MESSAGE, edaWRITE_THROUGH_WITH_ACTION, &cMsg);
                    palDataMgr_UI_EventSend(edcUI_EVENT_SOURCE_MESSAGE, cMsg, NULL);
                }
                else
                {
                    palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010
                }
            }
                break;

            case keVKEYSTONE_UP: //A70LV_Larry_0148
            {
                LOG_MSG(db_APP_INPUTKEY, "\r\nkeVKEYSTONE_UP");
                if( (palGeo_ApLinkFlag_Get() == TRUE ) ||				//A70Gen2_Steven_0005 fixed ISS-0008137 // R70CDS_Bruce#0087
                	(palDataMgr_CurrentWarpMemoryType() == WARPING_TYPE_AP || palDataMgr_CurrentWarpMemoryType() == WARPING_TYPE_AP_MYSTIQUE))// R70CDS_Bruce#0084
                {
                    UINT8 cMsg = eSOURCE_MESSAGE_NOT_SUPPORTED;
                    palDataMgr_Data_Access(edcSUPPORT_MESSAGE, edaWRITE_THROUGH_WITH_ACTION, &cMsg);
                    palDataMgr_UI_EventSend(edcUI_EVENT_SOURCE_MESSAGE, cMsg, NULL);
                }
                else
                {
                    palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010
                }
            }
                break;

            case keVKEYSTONE_DOWN: //A70LV_Larry_0148
            {
                LOG_MSG(db_APP_INPUTKEY, "\r\nkeVKEYSTONE_DOWN");
                if( (palGeo_ApLinkFlag_Get() == TRUE ) || 				//A70Gen2_Steven_0005 fixed ISS-0008137// R70CDS_Bruce#0087
                	(palDataMgr_CurrentWarpMemoryType() == WARPING_TYPE_AP || palDataMgr_CurrentWarpMemoryType() == WARPING_TYPE_AP_MYSTIQUE))// R70CDS_Bruce#0084
                {
                    UINT8 cMsg = eSOURCE_MESSAGE_NOT_SUPPORTED;
                    palDataMgr_Data_Access(edcSUPPORT_MESSAGE, edaWRITE_THROUGH_WITH_ACTION, &cMsg);
                    palDataMgr_UI_EventSend(edcUI_EVENT_SOURCE_MESSAGE, cMsg, NULL);
                }
                else
                {
                    palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010
                }
            }
                break;


            case keRESET_POWER_ON: //A70_Larry_0127
                break;

                // ==============================================================================
                // Hold key
                // ==============================================================================
            case keHOLD_HOTKEY:
                break;

            case keHOLD_POWERON:
                if(sInputKey.eKeyType == eKEYINPUT_KEYPAD)
                {
                    sInputKey.wKeyCode = keSTANDBY;
                	palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010
                }
                break;

            case keHOLD_MENU:
                break;

            case keHOLD_UP:
                if(palInputProc_KeyTimerHander(&sInputKey) == TRUE) //A70LV_Larry_0074
                {
                	//if(palInputProc_ADV_WarpDirectionCheck(DIR__UP) == FALSE)	//G100_Doulas_0027 Modify
                	{
                        palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010
                	}
                }
                break;

            case keHOLD_DOWN:
                if(palInputProc_KeyTimerHander(&sInputKey) == TRUE) //A70LV_Larry_0074
                {
                	//if(palInputProc_ADV_WarpDirectionCheck(DIR__DOWN) == FALSE)	//G100_Doulas_0027 Modify
                	{
                        palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010
                	}
                }
                break;

            case keHOLD_LEFT:
                if(palInputProc_KeyTimerHander(&sInputKey) == TRUE) //A70LV_Larry_0074
                {
                	//if(palInputProc_ADV_WarpDirectionCheck(DIR__LEFT) == FALSE)	//G100_Doulas_0027 Modify
                	{
                        palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010
                	}
                }
                break;

            case keHOLD_RIGHT:
                if(palInputProc_KeyTimerHander(&sInputKey) == TRUE) //A70LV_Larry_0074
                {
                	//if(palInputProc_ADV_WarpDirectionCheck(DIR__RIGHT) == FALSE)	//G100_Doulas_0027 Modify
                	{
                        palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010
                	}
                }
                break;

            case keHOLD_ENTER:
                break;

            case keHOLD_ZOOMIN:
                {
                    if(m_cLensMotorLock == eLENS_LOCK_ALL_ALLOW)
                    {
                    	palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010
                        palMotorZoomSet(eZOOMLENS_CMD_ZOOM_IN_RUN);
                    }
                }
                LOG_MSG(db_APP_INPUTKEY, "\r\n keHOLD_ZOOMIN");
                break;

            case keHOLD_ZOOMOUT:
                {
                    if(m_cLensMotorLock == eLENS_LOCK_ALL_ALLOW)
                    {
                    	palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010
                        palMotorZoomSet(eZOOMLENS_CMD_ZOOM_OUT_RUN);
                    }
                }
                LOG_MSG(db_APP_INPUTKEY, "\r\n keHOLD_ZOOMOUT");
                break;

            case keHOLD_FOCUSUP:
                {
                    if(m_cLensMotorLock == eLENS_LOCK_ALL_ALLOW)
                    {
                    	palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010
					    #ifdef CORRECT_FOCUS_DIRECTION  //A35G2_Simon_0109
                        palMotorFocusSet(eZOOMLENS_CMD_FOCUS_FAR_RUN);
                        #else
                        palMotorFocusSet(eZOOMLENS_CMD_FOCUS_NEAR_RUN);
                        #endif
                    }
                }
                LOG_MSG(db_APP_INPUTKEY, "\r\n keHOLD_FOCUSUP");
                break;

            case keHOLD_FOCUSDOWN:
                {
                    if(m_cLensMotorLock == eLENS_LOCK_ALL_ALLOW)
                    {
                    	palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010
					    #ifdef CORRECT_FOCUS_DIRECTION  //A35G2_Simon_0109
                        palMotorFocusSet(eZOOMLENS_CMD_FOCUS_NEAR_RUN);
                        #else
                        palMotorFocusSet(eZOOMLENS_CMD_FOCUS_FAR_RUN);
                        #endif
                    }
                }
                LOG_MSG(db_APP_INPUTKEY, "\r\n keHOLD_FOCUSDOWN");
                break;

            case keHOLD_LENSHLEFT:
                {
                    if(m_cLensMotorLock == eLENS_LOCK_ALL_ALLOW)
                    {
                        #if 0 // No Lens ID for X35Gen2
                        if(palEnvironment_LensModelCheck_Get() && palEnvironment_LensCalFlag_Get())
                        #else
                        if(palEnvironment_LensCalFlag_Get())
                        #endif
                        {
                        	palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010

                            if(sInputKey.eKeyType == eKEYINPUT_LAN) //G100_Larry_0040
                            {
                                #ifdef LAN_MOTOR_KEY_DOUBLE_RUN   //HICC2_Simon_0004
                                palMotorLensSet(eLENS_CMDS_RUN_LEFT_DOUBLE);
                                #else
                                palMotorLensSet(eLENS_CMDS_RUN_LEFT);
                                #endif
                            }
                            else
                            {
                                palMotorLensSet(eLENS_CMDS_RUN_LEFT);
                            }
                        }
                    }
                }
                LOG_MSG(db_APP_INPUTKEY, "\r\n keHOLD_LENSHLEFT");
                break;

            case keHOLD_LENSHRIGHT:
                {
                    if(m_cLensMotorLock == eLENS_LOCK_ALL_ALLOW)
                    {
                        #if 0 // No Lens ID for X35Gen2
                        if(palEnvironment_LensModelCheck_Get() && palEnvironment_LensCalFlag_Get())
                        #else
                        if(palEnvironment_LensCalFlag_Get())
                        #endif
                        {
                        	 palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010

                            if(sInputKey.eKeyType == eKEYINPUT_LAN) //G100_Larry_0040
                            {
                                #ifdef LAN_MOTOR_KEY_DOUBLE_RUN   //HICC2_Simon_0004
                                palMotorLensSet(eLENS_CMDS_RUN_RIGHT_DOUBLE);
                                #else
                                palMotorLensSet(eLENS_CMDS_RUN_RIGHT);
                                #endif
                            }
                            else
                            {
                                palMotorLensSet(eLENS_CMDS_RUN_RIGHT);
                            }
                        }
                    }
                }
                LOG_MSG(db_APP_INPUTKEY, "\r\n keHOLD_LENSHRIGHT");
                break;

            case keHOLD_LENSVUP:
                {
                    if(m_cLensMotorLock == eLENS_LOCK_ALL_ALLOW)
                    {
                        #if 0 // No Lens ID for X35Gen2
                        if(palEnvironment_LensModelCheck_Get() && palEnvironment_LensCalFlag_Get())
                        #else
                        if(palEnvironment_LensCalFlag_Get())
                        #endif
                        {
                        	palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010

                            if(sInputKey.eKeyType == eKEYINPUT_LAN) //G100_Larry_0040
                            {
                                #ifdef LAN_MOTOR_KEY_DOUBLE_RUN   //HICC2_Simon_0004
                                palMotorLensSet(eLENS_CMDS_RUN_UP_DOUBLE);
                                #else
                                palMotorLensSet(eLENS_CMDS_RUN_UP);
                                #endif
                            }
                            else
                            {
                                palMotorLensSet(eLENS_CMDS_RUN_UP);
                            }
                        }
                    }
                }
                LOG_MSG(db_APP_INPUTKEY, "\r\n keHOLD_LENSVUP");
                break;

            case keHOLD_LENSVDOWN:
                {
                    if(m_cLensMotorLock == eLENS_LOCK_ALL_ALLOW)
                    {
                        #if 0 // No Lens ID for X35Gen2
                        if(palEnvironment_LensModelCheck_Get() && palEnvironment_LensCalFlag_Get())
                        #else
                        if(palEnvironment_LensCalFlag_Get())
                        #endif
                        {
                        	palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010

                            if(sInputKey.eKeyType == eKEYINPUT_LAN) //G100_Larry_0040
                            {
                                #ifdef LAN_MOTOR_KEY_DOUBLE_RUN   //HICC2_Simon_0004
                                palMotorLensSet(eLENS_CMDS_RUN_DOWN_DOUBLE);
                                #else
                                palMotorLensSet(eLENS_CMDS_RUN_DOWN);
                                #endif
                            }
                            else
                            {
                                palMotorLensSet(eLENS_CMDS_RUN_DOWN);
                            }
                        }
                    }
                }
                LOG_MSG(db_APP_INPUTKEY, "\r\n keHOLD_LENSVDOWN");
                break;

            case keHOLD_STANDBY:
                //palInputProc_KeyEventSend(ucKeyType, ekSTANDBY); //A70LV_Larry_0333 mask
                break;

            case keHOLD_FAN_CAL:
                break;

            case keHOLD_RESET_POWER_ON:
                break;

            case keHOLD_HKEYSTONE_LEFT: //A70LV_Larry_0148
                LOG_MSG(db_APP_INPUTKEY, "\r\nkeHOLD_HKEYSTONE_LEFT");
                palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010
                break;

            case keHOLD_HKEYSTONE_RIGHT: //A70LV_Larry_0148
                LOG_MSG(db_APP_INPUTKEY, "\r\nkeHOLD_HKEYSTONE_RIGHT");
                palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010
                break;

            case keHOLD_VKEYSTONE_UP: //A70LV_Larry_0148
                LOG_MSG(db_APP_INPUTKEY, "\r\nkeHOLD_VKEYSTONE_UP");
                palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010
                break;

            case keHOLD_VKEYSTONE_DOWN: //A70LV_Larry_0148
                LOG_MSG(db_APP_INPUTKEY, "\r\nkeHOLD_VKEYSTONE_DOWN");
                palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010
                break;

            case keHOLD_2:
                palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010
                LOG_MSG(db_APP_INPUTKEY, "\r\nkeHOLD_2");
                break;

            case keHOLD_4:
                palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010
                LOG_MSG(db_APP_INPUTKEY, "\r\nkeHOLD_4");
                break;

            case keHOLD_6:
                palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010
                LOG_MSG(db_APP_INPUTKEY, "\r\nkeHOLD_6");
                break;

            case keHOLD_8:
                palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010
                LOG_MSG(db_APP_INPUTKEY, "\r\nkeHOLD_8");
                break;

                //==============================================================================
                // Release Key
                //==============================================================================
            case keRELEASE_HOTKEY:
                break;

            case keRELEASE_POWERON:
                break;

            case keRELEASE_MENU:
                break;

            case keRELEASE_UP:
                //G100_Wilsonj_0047 Start
                if(palInputProc_KeyTimerHander(&sInputKey) == TRUE) //A70LV_Larry_0074
                {
                    palInputProc_KeyEventSend(sInputKey);//HICC2_Steven_0010
                }
                //G100_Wilsonj_0047 End
                break;

            case keRELEASE_DOWN:
                //G100_Wilsonj_0047 Start
                if(palInputProc_KeyTimerHander(&sInputKey) == TRUE) //A70LV_Larry_0074
                {
                    palInputProc_KeyEventSend(sInputKey);//HICC2_Steven_0010
                }
                //G100_Wilsonj_0047 End
                break;

            case keRELEASE_LEFT:
                if(palInputProc_KeyTimerHander(&sInputKey) == TRUE) //A70LV_Larry_0074
                {
                    palInputProc_KeyEventSend(sInputKey);//HICC2_Steven_0010
                }
                break;

            case keRELEASE_RIGHT:
                if(palInputProc_KeyTimerHander(&sInputKey) == TRUE) //A70LV_Larry_0074
                {
                    palInputProc_KeyEventSend(sInputKey);//HICC2_Steven_0010
                }
                break;

            case keRELEASE_ENTER:
                break;

            case keRELEASE_FOCUSUP:
                break;

            case keRELEASE_FOCUSDOWN:
                break;

            case keRELEASE_ZOOMIN:
                break;

            case keRELEASE_ZOOMOUT:
                break;

            case keRELEASE_LENSHLEFT:
                break;

            case keRELEASE_LENSHRIGHT:
                break;

            case keRELEASE_LENSVUP:
                break;

            case keRELEASE_LENSVDOWN:
                break;

            case keRELEASE_MUTE:
                break;

            case keRELEASE_STANDBY:
                break;

            case keRELEASE_HKEYSTONE_LEFT: //A70LV_Larry_0148
                LOG_MSG(db_APP_INPUTKEY, "\r\nkeRELEASE_HKEYSTONE_LEFT");
                palInputProc_KeyEventSend(sInputKey);//HICC2_Steven_0010
                break;

            case keRELEASE_HKEYSTONE_RIGHT: //A70LV_Larry_0148
                LOG_MSG(db_APP_INPUTKEY, "\r\nkeRELEASE_HKEYSTONE_RIGHT");
                palInputProc_KeyEventSend(sInputKey);//HICC2_Steven_0010
                break;

            case keRELEASE_VKEYSTONE_UP: //A70LV_Larry_0148
                LOG_MSG(db_APP_INPUTKEY, "\r\nkeRELEASE_VKEYSTONE_UP");
                palInputProc_KeyEventSend(sInputKey);//HICC2_Steven_0010
                break;

            case keRELEASE_VKEYSTONE_DOWN: //A70LV_Larry_0148
                LOG_MSG(db_APP_INPUTKEY, "\r\nkeRELEASE_VKEYSTONE_DOWN");
                palInputProc_KeyEventSend(sInputKey);//HICC2_Steven_0010
                break;

            case keRELEASE_2:
                LOG_MSG(db_APP_INPUTKEY, "\r\nkeRELEASE_2");
                palInputProc_KeyEventSend(sInputKey);//HICC2_Steven_0010
                break;

            case keRELEASE_4:
                LOG_MSG(db_APP_INPUTKEY, "\r\nkeRELEASE_4");
                palInputProc_KeyEventSend(sInputKey);//HICC2_Steven_0010
                break;

            case keRELEASE_6:
                LOG_MSG(db_APP_INPUTKEY, "\r\nkeRELEASE_6");
                palInputProc_KeyEventSend(sInputKey);//HICC2_Steven_0010
                break;

            case keRELEASE_8:
                LOG_MSG(db_APP_INPUTKEY, "\r\nkeRELEASE_8");
                palInputProc_KeyEventSend(sInputKey);//HICC2_Steven_0010
                break;
                //==============================================================================
                // Upgrade LAN hot key // A70_Larry_0011 add
                //==============================================================================

            case keLAN_UPGRADE:
                break;

            case keHOLD_LAN_UPGRADE:
                break;

                //A70_Larry_0089 start
            case keRELEASE_LAN_UPGRADE:
                break;

            case keDDP_UPGRADE: //A70LH_Larry_0157
                break;

            case keSWAP:
				#ifdef CUSTOM_DEMO
					ucSystemMode = 4;
					halFormatter_SystemModeSet(ucSystemMode);
				#endif
                LOG_MSG(db_APP_INPUTKEY, "\r\n keSWAP");

                if(palDataMgr_PIN_Protect_Checking() == ePASSWORD_PROTECT_LOCKED)   //A70LV_Doulas_0222 Add
                {
                    break;
                }
                //if(appGui_Get_MenuState_IsOSD_Open())   //A70LV_Doulas_0095 modify
                //{
                //}
                //else
                {
                    UINT8 ucSWAP = 0;
                    if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(edcSWAP))  //available check
					{
                        UINT8 cMsg = eSOURCE_MESSAGE_NOT_SUPPORTED;
                        palDataMgr_Data_Access(edcSUPPORT_MESSAGE, edaWRITE_THROUGH_WITH_ACTION, &cMsg);
                        palDataMgr_UI_EventSend(edcUI_EVENT_SOURCE_MESSAGE, cMsg, NULL);
						break;
					}

                    palDataMgr_Data_Access(edcSWAP, edaWRITE_THROUGH_WITH_ACTION, &ucSWAP);
                }
                break;

            case keHOLD_SWAP:
                break;

            case keRELEASE_SWAP:
                break;
                //A70_Larry_0089 end

            case keTEST:    //A70LV_Doulas_0230 modify//A70LV_Doulas_0095
                if(palInputProc_KeyTimerHander(&sInputKey) == TRUE)
                {
 #ifdef CURSOR_FIXTURE	//HICC2_Doulas_0020
 					UINT8 ucTP;
                    LOG_MSG(db_APP_INPUTKEY, "\r\n keTEST");

                    if(palDataMgr_PIN_Protect_Checking() == ePASSWORD_PROTECT_LOCKED)   //A70LV_Doulas_0222 Add
                    {
                        break;
                    }

					if(palDataMgr_Data_Access(edcFACTORY_TEST_PATTERN, edaREAD, &ucTP) == eEXEC_CODE_PASS)
                    {
                        ucTP++;

                        if(ucTP >= 12)
                        {
                            ucTP = eCM_FACTORY_TP_NONE; //HICC2_Casper_0022
                        }
                        palDataMgr_Data_Access(edcFACTORY_TEST_PATTERN, edaWRITE_THROUGH_WITH_ACTION, &ucTP) ;
                    }
 #else
                    UINT8 ucTP;
                    LOG_MSG(db_APP_INPUTKEY, "\r\n keTEST");

                    if(palDataMgr_PIN_Protect_Checking() == ePASSWORD_PROTECT_LOCKED)   //A70LV_Doulas_0222 Add
                    {
                        break;
                    }

                    #ifdef CUSTOM_CHRISTIE
                    if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(edcOSDTEST_PATTERN))  //available check
                        break;

                    if(palDataMgr_Data_Access(edcOSDTEST_PATTERN, edaREAD, &ucTP) == eEXEC_CODE_PASS)
                    {
                        ucTP = (UINT8)DataMapping_GetNextCMvalueFromGui(edcOSDTEST_PATTERN, ucTP);

                        if(ucTP >= eCM_TEST_PATTERN_NUMBER)
                        {
                            ucTP = eCM_TEST_PATTERN_OFF; //HICC2_Casper_0019
                        }
                        palDataMgr_Data_Access(edcOSDTEST_PATTERN, edaWRITE_THROUGH_WITH_ACTION, &ucTP);
                    }
                    #else
                    if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(edcSERVICE_TEST_PATTERN))  //available check
                        break;

                    if(palDataMgr_Data_Access(edcOSDTEST_PATTERN, edaREAD, &ucTP) == eEXEC_CODE_PASS)
                    {
                        ucTP = (UINT8)DataMapping_GetNextCMvalueFromGui(edcOSDTEST_PATTERN, ucTP);

                        if(ucTP < eCM_TEST_PATTERN_NUMBER)
                        {
                            palDataMgr_Data_Access(edcSERVICE_TEST_PATTERN, edaWRITE_THROUGH_WITH_ACTION, &ucTP) ;
                        }
                        else
                        {
                            ucTP = eCM_TEST_PATTERN_OFF; //HICC2_Casper_0019
                            palDataMgr_Data_Access(edcSERVICE_TEST_PATTERN, edaWRITE_THROUGH_WITH_ACTION, &ucTP) ;
                        }
                    }
                    #endif
#endif
                }
                break;

            case kePIP:    //A70LV_Doulas_0095
                if(palInputProc_KeyTimerHander(&sInputKey) == TRUE)   //A35G2_CDS_Simon_0049
                {
                    LOG_MSG(db_APP_INPUTKEY, "\r\n kePIP");
                    palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010 //A70LK_Larry_0084 modify
                }
                break;

            case kePIP_SIZE:    //A70LV_Doulas_0095
                LOG_MSG(db_APP_INPUTKEY, "\r\n kePIP_SIZE");
                palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010 //A70LK_Larry_0084 modify
                break;

            case kePIP_LAYOUT:    //A70LV_Doulas_0095
                LOG_MSG(db_APP_INPUTKEY, "\r\n kePIP_LAYOUT");
                palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010 //A70LK_Larry_0084 modify
                break;

            //G100_Wilsonj_0043 Start
            case keMODE:
                LOG_MSG(db_APP_INPUTKEY, "\r\nMODE Key");
                palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010
                break;

            case kePATTERN:
                LOG_MSG(db_APP_INPUTKEY, "\r\nPATTERN Key");
                palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010
                break;

            case keHOTKEY2:
                LOG_MSG(db_APP_INPUTKEY, "\r\nHOTKEY2 Key");
                palInputProc_KeyEventSend(sInputKey); //HICC2_Steven_0010
                break;
            //G100_Wilsonj_0043 End

            default:
                //ASSERT_ALWAYS();
                break;
        }

        palInput_Motor_Process((eKEY_LIST)sInputKey.wKeyCode); //A70LV_Larry_0219
#endif
    }

    if((utilCounterGet(eCOUNTER_TYPE_MSG_FOCUS) == 0) && m_bMotorFocusEnale)
    {
        m_bMotorFocusEnale = FALSE;
    }

    if((utilCounterGet(eCOUNTER_TYPE_MSG_ZOOM) == 0) && m_bMotorZoomEnale)
    {
        m_bMotorZoomEnale = FALSE;
    }

    if((utilCounterGet(eCOUNTER_TYPE_MSG_LENS) == 0) && m_bMotorLensEnale)
    {
        m_bMotorLensEnale = FALSE;
    }

    if(m_bMotorLensAPPEnable && halMotorLensCalibrationStatusGet()==0) //HICC2_Casper_0030 //HICC2_Casper_0033
    {
        sLENSMEMORY        sLensMem = {0};

        palMotorLensPositionGet(&sLensMem);
        palDataMgr_Data_Access(edcCUR_H_POSITION, edaWRITE_RAM_ONLY_NO_ACTION, (void*)&sLensMem.wLensHPosition);
        palDataMgr_Data_Access(edcCUR_V_POSITION, edaWRITE_RAM_ONLY_NO_ACTION, (void*)&sLensMem.wLensVPosition);
        palDataMgr_Data_Access(edcCUR_H_DIR, edaWRITE_RAM_ONLY_NO_ACTION, (void*)&sLensMem.cDirH);
        palDataMgr_Data_Access(edcCUR_V_DIR, edaWRITE_RAM_ONLY_NO_ACTION, (void*)&sLensMem.cDirV);
        m_bMotorLensAPPEnable = FALSE;

        LOG_MSG(db_APP_INPUTKEY, "Lens posiotion (%d,%d) Dir(%d,%d)\r\n",sLensMem.wLensHPosition,sLensMem.wLensVPosition,sLensMem.cDirH,sLensMem.cDirV);
    }

    return INPUT_POLLING_PERIOD / POLL_PERIOD ;
}

// ==============================================================================
// FUNCTION NAME: APPINPUTPROC_CLIHANDLER
// DESCRIPTION:
//
//
// Params:
//
// Returns:
//
//
// modification history
// --------------------
// 12/11/2013, Leo Create
// --------------------
// ==============================================================================
eEXEC_CODE palInputProc_CLIHandler(UINT8 cKey)
{
    eRESULT eResult = rcERROR;

    ASSERT(eKEY_LIST_NUMBER > cKey);

    if(eKEY_LIST_NUMBER > cKey)
    {
        sKEY_DATA sKeyData = {0, eKEY_EVENT_PRESSED, eKEYINPUT_KEYPAD};
        sKeyData.wKeyCode = cKey;

        palInputProc_BufferInsert(&sKeyData);
        eResult = rcSUCCESS;
    }

    return (eEXEC_CODE)eResult;
}

//模擬 Key 輸入 (用在 RS232 CLI command , LAN 控制 ..... 等) //A70LV_Larry_0135
eRESULT palInputProc_InputKeycode(UINT32 ucKeyCode)
{
    eRESULT eResult = rcSUCCESS;
    sKEY_DATA sKeyStatus;

    sKeyStatus.eKeyType = eKEYINPUT_IR;
    sKeyStatus.eKeyEvent = eKEY_EVENT_PRESSED;
    sKeyStatus.wKeyCode = ucKeyCode;

#if defined(CUSTOM_BARCO) //A35G2_BRC_Casper_0049
    palInputKeyMgr_CmdProc(&sKeyStatus);
#else
    palInputKeyMgr_Proc(&sKeyStatus);
#endif

    return eResult;
}

//模擬 Key 輸入
//input 參數 eKeyListID 為 eKEY_LIST  列舉
eRESULT palInputProc_InputKeyListID(eKEY_LIST eKeyListID)       //T100_Simon_0022
{
    eRESULT eResult = rcSUCCESS;
    sKEY_DATA sOutPutKey;
    eKEY_EVENT eKeyEvent = eKEY_EVENT_PRESSED;

    if(eKeyListID >= keHOLD_HOTKEY && eKeyListID < keRELEASE_HOTKEY)
    {
        eKeyEvent = eKEY_EVENT_HOLD;
    }

    if(eKEY_LIST_NUMBER > eKeyListID)
    {
        sOutPutKey.eKeyType  = eKEYINPUT_LAN;	//G100_Clare_0056
        sOutPutKey.eKeyEvent = eKeyEvent;
        sOutPutKey.wKeyCode  = eKeyListID;     //此 Keycode 為  eKEY_LIST   列舉 , 非遙控器 keycode
        eResult = palInputProc_BufferInsert(&sOutPutKey);
    }
    else
    {
        eResult = rcERROR ;
    }

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: palInputProc_Remote_CustomId_Set
// DESCRIPTION:
//
//
// Params:
// UINT8 cCustomId:
//
// Returns:
//
//
// Modification History
// --------------------
// 2015/04/17, Leo Create
// --------------------
// ==============================================================================
void palInputProc_Remote_CustomId_Set(UINT16 wCustomId)
{
    if(CUSTOM_ID_COUNT > wCustomId)
    {
        palInputKeyMgr_IR_CustomId_Set(wCustomId);
        halMCU_RemoteIDSet((UINT8)wCustomId);
    }
}

// ==============================================================================
// FUNCTION NAME: palInputProc_LensMotorLockSet
// DESCRIPTION:
//
//
// Params:
// UINT8 ucEnable:
//
// Returns:
//
//
// Modification History
// --------------------
// 2018/03/16, Larry Create
// --------------------
// ==============================================================================
void palInputProc_LensMotorLockSet(UINT8 ucEnable)
{
    m_cLensMotorLock = ucEnable;
}

// ==============================================================================
// FUNCTION NAME: palInputProc_LensMotorLockGet
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
// 2018/03/16, Larry Create
// --------------------
// ==============================================================================
UINT8 palInputProc_LensMotorLockGet(void)
{
    return m_cLensMotorLock;
}

// ==============================================================================
// FUNCTION NAME: palInputProc_DisableKey
// DESCRIPTION:
//
//
// Params:
// UINT8 ucEnable:
//
// Returns:
//
//
// Modification History
// --------------------
// 2018/06/01, Larry Create
// --------------------
// ==============================================================================
void palInputProc_DisableKey(UINT8 ucEnable)
{
    m_cDisableKey = ucEnable;
}

BOOL palInputProc_DDP_PatternCheck(void) //HICC2_Casper_0019
{
	UINT8 ucFactory_TestPattern = eCM_FACTORY_TP_NONE; //HICC2_Casper_0022
	UINT8 ucFactory_SPOKE = 0;
	UINT8 ucFactory_SPLASH = 0;
	UINT8 ucFactory_BIT = 0;

	palDataMgr_Data_Access(edcFACTORY_TEST_PATTERN, edaREAD, &ucFactory_TestPattern);	//A70LK_Doulas_0018
	palDataMgr_Data_Access(edcFACTORY_DDP_SPOKE, edaREAD, &ucFactory_SPOKE);
	palDataMgr_Data_Access(edcFACTORY_DDP_SPLASH, edaREAD, &ucFactory_SPLASH);

	ucFactory_BIT = ucFactory_TestPattern | ucFactory_SPOKE | ucFactory_SPLASH;

	LOG_MSG(db_APP_INPUTKEY, "palInputProc_DDP_PatternCheck %d(%d,%d,%d)\n", ucFactory_BIT,ucFactory_TestPattern,ucFactory_SPOKE,ucFactory_SPLASH);

	if(ucFactory_BIT != eCM_FACTORY_TP_NONE) //HICC2_Casper_0022
	{
		ucFactory_TestPattern = eCM_FACTORY_TP_NONE; //HICC2_Casper_0022
		palDataMgr_Data_Access(edcFACTORY_TEST_PATTERN, edaWRITE_RAM_ONLY_WITH_ACTION, &ucFactory_TestPattern);
		palDataMgr_Data_Access(edcFACTORY_DDP_SPOKE, edaWRITE_RAM_ONLY_WITH_ACTION, &ucFactory_TestPattern);
		palDataMgr_Data_Access(edcFACTORY_DDP_SPLASH, edaWRITE_RAM_ONLY_WITH_ACTION, &ucFactory_TestPattern);

        return TRUE;
	}
	return FALSE;
}

// 2018/06/04, Energy Create
// --------------------
// ==============================================================================
void palInputProc_USTTestPattern(UINT8 ucTP) //ZU860_Energy_0009 //A35G2_BRC_Casper_0088
{
    m_cUSTTestPattern = ucTP;
}

BOOL palInputProc_ADV_WarpBackCheck(void)		//G100_Doulas_0027 //A35G2_Coda_0131
{
    BOOL bCheck = FALSE;
    //if(utilWarp_GetWarpCtrl() == WARP_CTRL__ADVANCED)	//G100_Doulas_0089 remove
    {
        if(utilWarp_GetOsdPatternType() == PAT_TYPE__WARP_SEL_CTRL_POINT)
        {
            palGeoAdvWarpShowOsdPattern_CheckReApplyBlend(PAT_TYPE__OFF);
            bCheck = TRUE;
        }
        else if(utilWarp_GetOsdPatternType() == PAT_TYPE__WARP_MOV_CTRL_POINT)
        {
            palGeo_AdvWarpShowOsdPattern(PAT_TYPE__WARP_SEL_CTRL_POINT);
            bCheck = TRUE;
        }
        else if(utilWarp_GetOsdPatternType() == PAT_TYPE__SEL_BLEND_WIDTH)
        {
            palGeo_AdvWarpShowOsdPattern(PAT_TYPE__OFF);
            bCheck = TRUE;
        }
        else if(utilWarp_GetOsdPatternType() == PAT_TYPE__ADJ_BLEND_WIDTH)
        {
            palGeo_AdvWarpShowOsdPattern(PAT_TYPE__SEL_BLEND_WIDTH);
            bCheck = TRUE;
        }
    #if (ADVANCED_BLACK_LEVEL == TRUE)
        else if(utilWarp_GetOsdPatternType() == PAT_TYPE__BLACKLEVEL_SEL_POINT)	//A65_OPTOMA_Doulas_0020
        {
            palGeo_AdvWarpShowOsdPattern(PAT_TYPE__BLACKLEVEL_PREVIEW);
            bCheck = TRUE;
        }
        else if(utilWarp_GetOsdPatternType() == PAT_TYPE__BLACKLEVEL_MOV_POINT)	//A65_OPTOMA_Doulas_0020
        {
            palGeo_AdvWarpShowOsdPattern(PAT_TYPE__BLACKLEVEL_SEL_POINT);
            bCheck = TRUE;
        }
        else if(utilWarp_GetOsdPatternType() == PAT_TYPE__BLACKLEVEL_ADD_POINT) //A35G2_Owen_0007 //A65_OPTOMA_Doulas_0020
        {
            //palGeo_AdvWarpShowOsdPattern(PAT_TYPE__BLACKLEVEL_SEL_POINT);
            //palGeo_AdvWarpShowOsdPattern(PAT_TYPE__OFF);  //A65_OPTOMA_Doulas_0023
            bCheck = TRUE;
        }
        else if(utilWarp_GetOsdPatternType() == PAT_TYPE__BLACKLEVEL_DEL_POINT) //A35G2_Owen_0007 //A65_OPTOMA_Doulas_0020
        {
            //palGeo_AdvWarpShowOsdPattern(PAT_TYPE__BLACKLEVEL_SEL_POINT);
            //palGeo_AdvWarpShowOsdPattern(PAT_TYPE__OFF);  //A65_OPTOMA_Doulas_0023
            bCheck = TRUE;
        }
        else if(utilWarp_GetOsdPatternType() == PAT_TYPE__BLACKLEVEL_PREVIEW)	//A65_OPTOMA_Doulas_0020
        {
            if(appGui_BlackLevelBrightnessMenuChecking() == FALSE)		//A65_OPTOMA_Doulas_0095 Modify
            {
                palGeo_AdvWarpShowOsdPattern(PAT_TYPE__OFF);
                //appGui_BackMenu(1);		//A65_OPTOMA_Doulas_0023 //###
                bCheck = TRUE;
            }
            else
            {
                palGeo_AdvWarpShowOsdPattern(PAT_TYPE__OFF);
            }
        }
    #endif
    }

	return bCheck;
}

BOOL palInputProc_ADV_WarpEnterCheck(void)		//G100_Doulas_0027
{
	BOOL bCheck = FALSE;
    if(utilWarp_GetWarpCtrl() == WARP_CTRL__ADVANCED)
	{
		if(utilWarp_GetOsdPatternType() == PAT_TYPE__WARP_SEL_CTRL_POINT)
		{
			palGeo_AdvWarpShowOsdPattern(PAT_TYPE__WARP_MOV_CTRL_POINT);
			bCheck = TRUE;
		}
		else if(utilWarp_GetOsdPatternType() == PAT_TYPE__WARP_MOV_CTRL_POINT)
		{
			palGeo_AdvWarpShowOsdPattern(PAT_TYPE__WARP_SEL_CTRL_POINT);
			bCheck = TRUE;
		}
		else if(utilWarp_GetOsdPatternType() == PAT_TYPE__SEL_BLEND_WIDTH)
		{
			palGeo_AdvWarpShowOsdPattern(PAT_TYPE__ADJ_BLEND_WIDTH);
			bCheck = TRUE;
		}
		else if(utilWarp_GetOsdPatternType() == PAT_TYPE__ADJ_BLEND_WIDTH)
		{
			palGeo_AdvWarpShowOsdPattern(PAT_TYPE__SEL_BLEND_WIDTH);
			bCheck = TRUE;
		}
#if (ADVANCED_BLACK_LEVEL == TRUE)
		else if(utilWarp_GetOsdPatternType() == PAT_TYPE__BLACKLEVEL_SEL_POINT)	//A65_OPTOMA_Doulas_0020
		{
			palGeo_AdvWarpShowOsdPattern(PAT_TYPE__BLACKLEVEL_MOV_POINT);
			bCheck = TRUE;
		}
		else if(utilWarp_GetOsdPatternType() == PAT_TYPE__BLACKLEVEL_MOV_POINT)	//A65_OPTOMA_Doulas_0020
		{
			palGeo_AdvWarpShowOsdPattern(PAT_TYPE__BLACKLEVEL_SEL_POINT);
			bCheck = TRUE;
		}
		else if(utilWarp_GetOsdPatternType() == PAT_TYPE__BLACKLEVEL_ADD_POINT)	//A65_OPTOMA_Doulas_0020
		{
			palGeo_AdvBlackLevel_AddPoint();
			bCheck = TRUE;
		}
		else if(utilWarp_GetOsdPatternType() == PAT_TYPE__BLACKLEVEL_DEL_POINT)	//A65_OPTOMA_Doulas_0020
		{
			palGeo_AdvBlackLevel_DeletePoint();
			bCheck = TRUE;
		}
		else if(utilWarp_GetOsdPatternType() == PAT_TYPE__BLACKLEVEL_PREVIEW)	//A65_OPTOMA_Doulas_0020
		{
			if(appGui_BlackLevelBrightnessMenuChecking() == FALSE)		//A65_OPTOMA_Doulas_0095 Modify
			{
				palGeo_AdvWarpShowOsdPattern(PAT_TYPE__BLACKLEVEL_SEL_POINT);	//A65_OPTOMA_Doulas_0023 Modify edit area
				bCheck = TRUE;
			}
		}
#endif
    }

	return bCheck;
}

BOOL palInputProc_ADV_WarpDirectionCheck(UINT8 ucVal)		//G100_Doulas_0027
{
	BOOL bCheck = FALSE;
    if(utilWarp_GetWarpCtrl() == WARP_CTRL__ADVANCED)
	{
		if(utilWarp_GetOsdPatternType() == PAT_TYPE__WARP_SEL_CTRL_POINT)
		{
			palGeo_AdvWarpGP_SelectControlPoint((eADV_WARP_MOVE_DIR)ucVal);
			bCheck = TRUE;
		}
		else if(utilWarp_GetOsdPatternType() == PAT_TYPE__WARP_MOV_CTRL_POINT)
		{
			palGeo_AdvWarpGP_MoveControlPoint((eADV_WARP_MOVE_DIR)ucVal);
			bCheck = TRUE;
		}
		else if(utilWarp_GetOsdPatternType() == PAT_TYPE__SEL_BLEND_WIDTH)
		{
			palGeo_AdvBlend_SelectOsdBlendWidth(ucVal);
			bCheck = TRUE;
		}
		else if(utilWarp_GetOsdPatternType() == PAT_TYPE__ADJ_BLEND_WIDTH)
		{
			palGeo_AdvBlend_SelectOsdBlendWidth(ucVal);
			palDataMgr_ADV_Blend_LfRtTpBm_Update();
			bCheck = TRUE;
		}
#if (ADVANCED_BLACK_LEVEL == TRUE)
		else if(utilWarp_GetOsdPatternType() == PAT_TYPE__BLACKLEVEL_SEL_POINT)	//A65_OPTOMA_Doulas_0020
		{
			halAdvBlackLevelSelectControlPoint((eDIR)ucVal);
			bCheck = TRUE;
		}
		else if(utilWarp_GetOsdPatternType() == PAT_TYPE__BLACKLEVEL_MOV_POINT)	//A65_OPTOMA_Doulas_0020
		{
			halAdvBlackLevelMoveControlPoint((eDIR)ucVal);
			bCheck = TRUE;
		}
		else if(utilWarp_GetOsdPatternType() == PAT_TYPE__BLACKLEVEL_ADD_POINT)	//A65_OPTOMA_Doulas_0020
		{
			halAdvBlackLevelSelectControlPoint((eDIR)ucVal);
			bCheck = TRUE;
		}
		else if(utilWarp_GetOsdPatternType() == PAT_TYPE__BLACKLEVEL_DEL_POINT)	//A65_OPTOMA_Doulas_0020
		{
			halAdvBlackLevelSelectControlPoint((eDIR)ucVal);
			bCheck = TRUE;
		}
		else if(utilWarp_GetOsdPatternType() == PAT_TYPE__BLACKLEVEL_PREVIEW)	//A65_OPTOMA_Doulas_0020
		{
			//palGeo_AdvWarpShowOsdPattern(PAT_TYPE__OFF);   //A65_OPTOMA_Doulas_0023 remove
			if(appGui_BlackLevelBrightnessMenuChecking() == FALSE)		//A65_OPTOMA_Doulas_0095 Modify
			{
				bCheck = TRUE;
			}
		}
#endif
    }

	return bCheck;
}

#ifdef OSD_LOCK_ENABLE //A65_OPTOMA_Jerry_0005 start
void palInputProc_OSD_Unlock_Step_Set(UINT8 ucOSD_Unlock_Step)
{
    m_ucOsdUnlock_Procedure_Status = ucOSD_Unlock_Step;
}

UINT8 palInputProc_OSD_Unlock_Step_Get(void)
{
    return m_ucOsdUnlock_Procedure_Status;
}

static void palInputProc_OSD_Unlock_Procedure(eKEY_INPUT_TYPE eKeyType, UINT32 dwKey)
{
    switch (m_ucOsdUnlock_Procedure_Status)
    {
        case OSD_UNLOCK_PROCEDURE_STEP1:
            if ( ( ( eKeyType == eKEYINPUT_KEYPAD ) && ( dwKey == kePOWERON ) ) // Keypad power off
                 || ( ( eKeyType == eKEYINPUT_IR ) && ( dwKey == keSTANDBY) ))    // IR power off
            {
                palInputProc_OSD_Unlock_Step_Set(OSD_UNLOCK_PROCEDURE_STEP2);
            }
            break;

        case OSD_UNLOCK_PROCEDURE_STEP2:
            if (dwKey== keLEFT) {
                palInputProc_OSD_Unlock_Step_Set(OSD_UNLOCK_PROCEDURE_STEP3);
            }
            else {
                palInputProc_OSD_Unlock_Step_Set(OSD_UNLOCK_PROCEDURE_STEP1);
            }
            break;

        case OSD_UNLOCK_PROCEDURE_STEP3:
            if (dwKey== keLEFT) {
                palInputProc_OSD_Unlock_Step_Set(OSD_UNLOCK_PROCEDURE_STEP4);
            }
            else {
                palInputProc_OSD_Unlock_Step_Set(OSD_UNLOCK_PROCEDURE_STEP1);
            }
            break;

        case OSD_UNLOCK_PROCEDURE_STEP4:
            if (dwKey == keLEFT) {
                UINT8 ucOsdLockStatus = eOSD_NOT_LOCKED;//ets_OFF;

                // OSD unlock currently, it will be locked again if the system is restart.
                palDataMgr_Data_Access(edcOSD_LOCK, edaWRITE_RAM_ONLY_WITH_ACTION, &ucOsdLockStatus);

                palInputProc_OSD_Unlock_Step_Set(OSD_UNLOCK_PROCEDURE_DONE);

                appGui_Open_OSD_Lock_Remind_Msg_Menu();
            }
            else {
                palInputProc_OSD_Unlock_Step_Set(OSD_UNLOCK_PROCEDURE_STEP1);
            }
            break;

        default:
            break;
    }
}

eCMD_KEY_DATACODE_EXECUTE_STATUS palInputProc_OSD_Lock_Key_Filter(eKEY_INPUT_TYPE eKeyType, UINT32 dwKey)
{
    eCMD_KEY_DATACODE_EXECUTE_STATUS eRetStatus = eCMD_KEY_DATACODE_NOT_ALLOWED_TO_EXECUTE;

    switch(eKeyType)
    {
        case eKEYINPUT_KEYPAD:
            switch(dwKey)
            {
                //Power Off
                case kePOWERON:
                case keHOLD_POWERON:
                case keRELEASE_POWERON:
                //Input
                case keINPUT:           //A35G2_Owen_0011
                case keHOLD_INPUT:      //A35G2_Owen_0011
                case keRELEASE_INPUT:   //A35G2_Owen_0011
                    eRetStatus = eCMD_KEY_DATACODE_ALLOWED_TO_EXECUTE;
                    break;

                default:
                    break;
            }
            break;

        case eKEYINPUT_IR:
            switch(dwKey)
            {
                //Power Off
                case keSTANDBY:
                case keHOLD_STANDBY:
                case keRELEASE_STANDBY:
                //Mute
                case keMUTE:
                //Input
                case keINPUT:           //A35G2_Owen_0011
                case keHOLD_INPUT:      //A35G2_Owen_0011
                case keRELEASE_INPUT:   //A35G2_Owen_0011
                    eRetStatus = eCMD_KEY_DATACODE_ALLOWED_TO_EXECUTE;
                    break;

                default:
                    break;
            }
            break;
    }

    return eRetStatus;
}

eCMD_KEY_DATACODE_EXECUTE_STATUS palInputProc_OSD_Lock_Check_Key( eKEY_INPUT_TYPE eKeyType, UINT32 dwKey)
{
    eCMD_KEY_DATACODE_EXECUTE_STATUS eRetStatus = eCMD_KEY_DATACODE_ALLOWED_TO_EXECUTE;                // allow key to execute
    UINT8 ucOSD_Lock_Status = eOSD_NOT_LOCKED;
    eEXEC_CODE eRet = palDataMgr_Data_Access(edcOSD_LOCK, edaREAD, &ucOSD_Lock_Status);

    if ( ( eRet == eEXEC_CODE_PASS ) && ( ucOSD_Lock_Status == eOSD_LOCKED ) )       // OSD is locking
    {
        // check OSD unlock by key
        palInputProc_OSD_Unlock_Procedure( eKeyType, dwKey);

        // check the key is allowed to execute or not
        eRetStatus = palInputProc_OSD_Lock_Key_Filter( eKeyType, dwKey);

        // remind user that the OSD locked
        if( ( eRetStatus == eCMD_KEY_DATACODE_NOT_ALLOWED_TO_EXECUTE )
            && ( palInputProc_OSD_Unlock_Step_Get() != OSD_UNLOCK_PROCEDURE_DONE )
        )
        {
            appGui_Open_OSD_Lock_Icon_Msg_Menu();
        }
    }

    return eRetStatus;
}
#endif //OSD_LOCK_ENABLE
//A65_OPTOMA_Jerry_0005 end

void palInputProc_OPDKeyEvent(sKEY_DATA sKeyData)  //A35G2_Simon_0075
{
    uOPD_DATA uOPDData ;

    UINT8 aucKeyEventStr[32] = {0};
    switch((eKEY_EVENT)sKeyData.eKeyEvent)
    {
        case eKEY_EVENT_PRESSED:    sprintf(aucKeyEventStr, "%s",   STRINGER(eKEY_EVENT_PRESSED));    break;
        case eKEY_EVENT_HOLD:       sprintf(aucKeyEventStr, "%s",   STRINGER(eKEY_EVENT_HOLD));       break;
        case eKEY_EVENT_RELEASE:    sprintf(aucKeyEventStr, "%s",   STRINGER(eKEY_EVENT_RELEASE));    break;
        default:                    sprintf(aucKeyEventStr, "%s%d", STRINGER(eKEY_EVENT_NUMBER), sKeyData.eKeyEvent);    break;
    }

    if(sKeyData.wKeyCode < 200)
    {
        if(sKeyData.wKeyCode < KEY_STRING0_MAX)
        {
            snprintf(uOPDData.cString, 128,  "%s,%s", m_sInputKeyString_0[sKeyData.wKeyCode], aucKeyEventStr);
        }
        else
        {
            LOG_MSG(db_APP_INPUTKEY, "unknown0 %d\n", sKeyData.wKeyCode);
        }
    }
    else if(sKeyData.wKeyCode < 400)
    {
        if(sKeyData.wKeyCode - 200 < KEY_STRING1_MAX)
        {
            snprintf(uOPDData.cString, 128,  "%s,%s", m_sInputKeyString_1[sKeyData.wKeyCode - 200], aucKeyEventStr);
        }
        else
        {
            LOG_MSG(db_APP_INPUTKEY, "unknown1 %d\n", sKeyData.wKeyCode);
        }
    }
    else if(sKeyData.wKeyCode < 600)
    {
        if(sKeyData.wKeyCode - 400 < KEY_STRING2_MAX)
        {
            snprintf(uOPDData.cString, 128,  "%s,%s", m_sInputKeyString_2[sKeyData.wKeyCode - 400], aucKeyEventStr);
        }
        else
        {
            LOG_MSG(db_APP_INPUTKEY, "unknown2 %d\n", sKeyData.wKeyCode);
        }
    }
    else
    {
        if(sKeyData.wKeyCode - 600 < KEY_STRING3_MAX)
        {
            snprintf(uOPDData.cString, 128,  "%s,%s", m_sInputKeyString_3[sKeyData.wKeyCode - 600], aucKeyEventStr);
        }
        else
        {
            LOG_MSG(db_APP_INPUTKEY, "unknown3 %d\n", sKeyData.wKeyCode);
        }
    }

    switch(sKeyData.eKeyType)
    {
        case eKEYINPUT_KEYPAD:
            utilOPD_EventSet(eOPD_KEY_KEYPAD_LOG, &uOPDData);
            break;

        case eKEYINPUT_IR:
            utilOPD_EventSet(eOPD_KEY_IR_LOG, &uOPDData);
            break;

        case eKEYINPUT_LAN:
            utilOPD_EventSet(eOPD_KEY_LAN_LOG, &uOPDData);
            break;

        case eKEYINPUT_HDBaseT:
            utilOPD_EventSet(eOPD_KEY_HDBaseT_LOG, &uOPDData);
            break;

        case eKEYINPUT_CLI:
            utilOPD_EventSet(eOPD_KEY_CLI_LOG, &uOPDData);
            break;

        case eKEYINPUT_WIRED: //A70LK_Larry_0098
            utilOPD_EventSet(eOPD_KEY_WIDE_LOG, &uOPDData);
            break;

        case eKEYINPUT_BACK:
            utilOPD_EventSet(eOPD_KEY_BACK_LOG, &uOPDData);
            break;

        default:
            break;
    }

}


eEXEC_CODE palInputProc_KeyEventSend(sKEY_DATA Key) //HICC2_Steven_0010
{
    eEXEC_CODE eRet = eEXEC_CODE_FAIL;

    LOG_MSG(db_APP_INPUTKEY, ">> KeyEventSend %d\n", Key); //HICC2_Doulas_0083
    eRet = palDataMgr_Data_Access(edcACTION_KEY_CODE, edaWRITE_RAM_ONLY_WITH_ACTION, &Key);

    return eRet;
}

//HICC2_Steven_0002
BOOL palInputProc_BIST_KeyCheckFun(UINT8 eKey) // "ekEXIT + ekUP + ekDOWN + ekLEFT + ekRIGHT + ekENTER"
{
	//LOG_MSG(db_ALWAYS, ">> BIST_KeyCheckFun %d, %d\n", eKey, m_ucBIST_KeyStatus);
	BOOL bValue = FALSE;

    switch (m_ucBIST_KeyStatus)
    {
    	case BIST_KeyCheck_OFF:
        if (eKey == keBACK)
        {
        	m_ucBIST_KeyStatus = BIST_KeyCheck_STEP1;
        }
        break;

        case BIST_KeyCheck_STEP1:
        if (eKey== keUP) //A70LK_Steven_0013
        {
        	m_ucBIST_KeyStatus = BIST_KeyCheck_STEP2;
        }
        else
        {
        	m_ucBIST_KeyStatus = BIST_KeyCheck_OFF;
        }
        break;

        case BIST_KeyCheck_STEP2:
        if (eKey== keDOWN)
        {
        	m_ucBIST_KeyStatus = BIST_KeyCheck_STEP3;
        }
        else
        {
        	m_ucBIST_KeyStatus = BIST_KeyCheck_OFF;
        }
        break;

        case BIST_KeyCheck_STEP3:
        if (eKey== keLEFT) //A70LK_Steven_0014
        {
        	m_ucBIST_KeyStatus = BIST_KeyCheck_STEP4;
        }
        else
        {
        	m_ucBIST_KeyStatus = BIST_KeyCheck_OFF;
        }
        break;

        case BIST_KeyCheck_STEP4:
        if (eKey== keRIGHT)
        {
        	m_ucBIST_KeyStatus = BIST_KeyCheck_STEP5;
        }
        else
        {
        	m_ucBIST_KeyStatus = BIST_KeyCheck_OFF;
        }
        break;

        case BIST_KeyCheck_STEP5:
        if (eKey== keENTER) //A70LK_Steven_0013
        {
        	//UINT8 ucBIST = 1; //HICC2_Steven_0008
        	//palDataMgr_Data_Access(edcBIST_ENABLE, edaWRITE_THROUGH_WITH_ACTION,(void*)&ucBIST);
        	utilBIST_Handle(1);  //HICC2_Steven_0012
        	m_ucBIST_KeyStatus = BIST_KeyCheck_OFF;
        	bValue = TRUE;
        }
        else
        {
        	m_ucBIST_KeyStatus = BIST_KeyCheck_OFF;
        }
        break;

        default:
        	break;
    }
    return bValue;
}

UINT32 palInput_Get_H_Current_Position(void) //R70PD_AC_0105 //R70KPD_VSC_Wesley_0003 //HICC2_Casper_0030 //HICC2_Casper_0040
{
	return m_sLensMem.wLensHPosition;
}

void palInput_Set_H_Current_Position(UINT32 wPosition) //R70KPD_VSC_Wesley_0003 //HICC2_Casper_0030 //HICC2_Casper_0040
{
	m_sLensMem.wLensHPosition = wPosition;
}

UINT32 palInput_Get_V_Current_Position(void) //R70KPD_VSC_Wesley_0003 //HICC2_Casper_0030 //HICC2_Casper_0040
{
	return m_sLensMem.wLensVPosition;
}

void palInput_Set_V_Current_Position(UINT32 wPosition) //R70KPD_VSC_Wesley_0003 //HICC2_Casper_0030 //HICC2_Casper_0040
{
	m_sLensMem.wLensVPosition = wPosition;
}

UINT8 palInput_Get_H_Current_Direction(void) //R70KPD_VSC_Wesley_0003 //HICC2_Casper_0030
{
	return m_sLensMem.cDirH;
}

void palInput_Set_H_Current_Direction(UINT8 bDirection) //R70KPD_VSC_Wesley_0003 //HICC2_Casper_0030
{
	m_sLensMem.cDirH = bDirection;
}

UINT8 palInput_Get_V_Current_Direction(void) //R70KPD_VSC_Wesley_0003 //HICC2_Casper_0030
{
	return m_sLensMem.cDirV;
}

void palInput_Set_V_Current_Direction(UINT8 bDirection) //R70KPD_VSC_Wesley_0003 //HICC2_Casper_0030
{
	m_sLensMem.cDirV = bDirection;
}

BOOL palInput_Web_LensShift_Check(UINT32 wData)//HICC2_Julie_0048
{
    if(palEnvironment_LensCalFlag_Get() == FALSE)
    {
        if((wData >= keLENSHRIGHT && wData <= keLENSVDOWN) || (wData >= keHOLD_LENSHRIGHT && wData <= keHOLD_LENSVDOWN))
        {
            return FALSE;
        }
    }
    return TRUE;
}

