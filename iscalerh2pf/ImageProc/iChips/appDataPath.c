#include "appDataPath.h"
#include "appDataMgr.h"
#include "appSystem.h"
//#include "appGui.h"
#include "appLANProcAPI.h"
#include "halScaler.h"
#include "utilDbgMsg.h"
#include "halFormatter.h"       //A70LV_Doulas_0013
#ifdef SCALER_C821_C789
#include "halC789CtrlAPI.h"    //A70LV_Doulas_0072
#include "dvC821.h"        	//G100_Tim_0057, add //A35G2_BRC_Casper_0060
#endif

#ifdef SCALER_C341
#include "dvC341.h"
#endif

#include "halFrontEndCtrlAPI.h"     //A70LV_Doulas_0076
#include "halMCUCtrlAPI.h"
#include "halFrontEndCtrlAPI.h"
#include "appEnvironment.h"         //A70LV_Doulas_0179
#include "appIllumination.h"        //A70LV_Doulas_0212
#include "halBoardCtrlAPI.h"        //A70LV_Doulas_0216
#include "utilCounterAPI.h"         //A70LV_Doulas_0308
#include "utilWarpDemo.h"		//G100_Doulas_0027
#include "utilCLICmdAPI.h"
#include "palGui.h"
#include "palGeoAPI.h"
#include "palImgMgr.h"
#include "palSysCtrlMgr.h"
#include "palFormatterMgr.h"
#include "palFrontEndMgr.h"
#include "utilOPD_TEST.h" //H30K_Doulas_0073

#define  XPR_PANEL_CHECK_COUNT (10)
#define  XPR_RESET_TIMEOUT 5

sGUI_CALLBACK GuiCb;

static sPAL_DATA_PATH_INFORMATION   m_sPalDataPathInfo;
static PsSYSTEM_CONFIGURATION       m_psSysConfiguration;        //Default system settings, like TI gpConfiguration
static sDATA_PATH_SOURCE_DESC       m_sSourceDesc;
static sVIDEO_FORMAT                m_FrontEndVideoFormat = {eVIDEO_COLORDEPTH_8BIT,eVIDEO_COLORSPACE_RGB,eVIDEO_COLORRANGE_FULL,eVIDEO_SCANNING_PROGRESSIVE,0,0,eVGA_SYNC_TYPE_UNKNOW,eVIDEO_YUV_UNKNOW};  //A70LV_Doulas_0109 modify    //A70LV_Doulas_0076
static UINT8 uc3D_Panel_Timer = 0;  //A70LV_Doulas_0154
static sVIDEO_TIMING                m_FrontEndVideoTiming;           //A70LV_Doulas_0176
static UINT8 ucTestPatternSourceLostCount = 0; //ZU860_Doulas_0124
static EOTF_STREAM m_Video_EOTF_STREAM = EOTF_SDR;
static sHDR_LUMINANCE_INFO m_HDRLuminanceInfo;  //H2PF_Simon_0193

#ifdef CUSTOM_CHRISTIE
static eDATA_PATH_STATE m_DataPathLastState;
#endif
static UINT8 m_ucPanelChangeing = ets_OFF;                      //A70LV_Doulas_0367
static UINT8 m_ucPanelIDForTwist = ePANEL_ID_XGA_60HZ;          //A70LV_Doulas_0367
static UINT8 m_uc3D_FormatConfigForTwist = eINPUT_3D_TYPE_OFF;  //A70LV_Doulas_0367
static UINT8 m_ucC789_Output_V_FreqForTwist = 0;                //A70LV_Doulas_0367
static UINT8 m_ucAlreadyShowDisconnectTwistMsg = ets_OFF;       //A70LV_Doulas_0367
static UINT8 m_ucLast3DEnable_Setting = ets_OFF;                //A70LV_Doulas_0367
//G100_Steven_0028
static sVIDEO_TIMING m_BKInput_FirstTiming, m_BKInput_SecondTiming;
static UINT8 m_BKInput_FirstFormat[eFE_MSG_VIDEO_FORMAT_MAIN_SZ] = {0};
static UINT8 m_BKInput_SecondFormat[eFE_MSG_VIDEO_FORMAT_SUB_SZ] = {0};
static UINT8 m_BKInput_Status = 0, m_BKInput_Active = 0; //G100_Steven_0055
//G100_Steven_0028
static UINT8 m_C789_C821_InitReady = 0;		//G100_Doulas_0076

static UINT8 m_ucQuickSwitch_SrcReady = FALSE ;
static UINT8 m_BKInput_Change = 0;		//A65_OPTOMA_Doulas_0115
static UINT8 ucXPR_Reset_Timeout = XPR_RESET_TIMEOUT;
UINT16 uiResetXPR_Cnt = 0;

///////////////////////////////////////////////////////// test only  start
INT32 TestTimeStart = 0; //only for testing
INT32 TestTimeEnd = 0; //only for testing
//UINT8 ucTestSplashOff = 0; //only for testing
///////////////////////////////////////////////////////// test only  end

static UINT16 m_uiInputDetect = 0;  //A35G2_Simon_0075
static UINT16 m_uiLastInputDetect = 0;  //A35G2_Simon_0075
UINT8 ucMenuTransparencyEnable = FALSE;     //A70LV_Doulas_0122

static UINT16 m_uiFirstLanguageCount = 0;//HICC2_Julie_0037
static UINT8 m_ucSmoothOnReadyCheck = 0;//HICC2_Julie_0037
static UINT8 ucInfoFrame_Change_flag = 0;

void palDataPath_HDR_Process(void);  //H2PF_Simon_0193

static void palDataPath_ResetTimeInState(void)
{
    m_sPalDataPathInfo.ulStateStartTicks = xTaskGetTickCount();
}

/****************************************************************************/
/*                 */
/****************************************************************************/
///////////////////////////////////////////////////////////////////////////////////
///@brief palDataPath_TimeElapsedInState : Returns time elapsed in the current state in milliseconds
///@return UINT32 - Number tick past
///@author Roger Chen
///////////////////////////////////////////////////////////////////////////////////
UINT32 palDataPath_TimeElapsedInState(void)
{
    TickType_t ulTicks;

    ulTicks = xTaskGetTickCount();

    if(m_sPalDataPathInfo.ulStateStartTicks > ulTicks)
    {
        return (0xFFFFFFFF - m_sPalDataPathInfo.ulStateStartTicks + 1 + ulTicks) / portTICK_RATE_MS;
    }
    else
    {
        return (ulTicks - m_sPalDataPathInfo.ulStateStartTicks) / portTICK_RATE_MS;
    }
}


static void palDataPath_StopCurrentOperation(void)
{
#if 0
    /* flag that the external source is not active */
    sourceDesc.sourceActive = FALSE;

    if(blankScreenEnabled)
    {
        blankScreenEnabled = FALSE;          /* reset state of blank screen */
    }

    if(datapathState == TPG_DISPLAYED)
    {
        source_SetTestPattern(DPTPG_NONE, NULL);             /* disable TPG */
    }
    else
    {
        //----- Roger Added 20061013  Start
        switch(sourceDesc.source)
        {
            case SRC_DVI_D:
            case SRC_HDMI:
                dvicontrol_StopCurrentOperation();
                break;

            case SRC_VGA1:
            case SRC_VGA2:
            case SRC_DVI_A:
            case SRC_RCA_ADC_COMPONENT:
                adccontrol_StopCurrentOperation();
                break;

            case SRC_COMPOSITE:
            case SRC_SVIDEO:
            case SRC_VGA1_DEC_COMPONENT:
            case SRC_VGA2_DEC_COMPONENT:
            case SRC_DVI_DEC_COMPONENT:
            case SRC_RCA_DEC_COMPONENT:
            case SRC_VGA1_SCART:
            case SRC_VGA2_SCART:
            case SRC_DVI_SCART:
                deccontrol_StopCurrentOperation();
                break;
        }

        //----- Roger Added 20061013  End
    }

#endif
}

static eMAIL_BOX_EXEC_CODE palDataPath_MailBox_Send(xQueueHandle xQueueHandle, EventGroupHandle_t xEventGroupHandle,
                                                    UINT16 uiMsgID, INT32 lBlockTime,
                                                    UINT32 ulParam1, UINT32 *pulParam2, BOOL bDispose)    //A70LV_Doulas_0278
{
    eMAIL_BOX_EXEC_CODE eMailExecCode = eMAIL_BOX_EXEC_CODE_PASS;

    LOG_MSG(db_APP_DATAPATH, "palDataPath_MailBox_Send Event[%d]\r\n", uiMsgID);

#if (SYSTEM_OS_TYPE == POSIX_COMPLIANT)
    eMailExecCode = palMailBox_Send_MsgQueue(m_sPalDataPathInfo.xMsgQueue, QUEUE_DATAPATH_NAME, uiMsgID, lBlockTime, ulParam1, pulParam2);
#else
	eMailExecCode = palMailBox_Send(xQueueHandle, xEventGroupHandle, uiMsgID, lBlockTime, ulParam1, pulParam2, bDispose);
#endif

    if(eMailExecCode != eMAIL_BOX_EXEC_CODE_PASS)
    {
        LOG_MSG(db_ALWAYS, "(func:%s, line:%d) : palDataPath_MailBox_Send Fail [%d] [%d]!\r\n", __FUNCTION__, __LINE__, uiMsgID, eMailExecCode);
    }

    return eMailExecCode;
}

eEXEC_CODE palDataPath_GetPathReady(void)
{
    UINT32 bIsReady = 0;

    /* Wait up to 10 seconds for the datapath to finish processing init */
    /* message and then this message
    */
    if(palDataPath_MailBox_Send(m_sPalDataPathInfo.xMsgQueue, m_sPalDataPathInfo.xEventGroupHandle, eDATA_PATH_MSG_GETREADY,
                       10000, 0, &bIsReady, FALSE) != eMAIL_BOX_EXEC_CODE_PASS)
    {
        return eEXEC_CODE_FAIL;
    }

    if(bIsReady)
    {
        return eEXEC_CODE_PASS;
    }
    else
    {
        return eEXEC_CODE_FAIL;
    }
}

static void palDataPath_DisplaySplash(BOOL Force) //A70LV_Larry_0164
{
    halScaler_DisplaySplash();
    //m_sSourceDesc.eActiveDisplay = eDATA_PATH_ACTIVE_DISPLAY_SPLASH;

    tDATA_CODE IsOSDOpen = 0;
    palDataMgr_Data_Access(edcUI_STATUS_IS_OSD_OPEN, edaREAD, &IsOSDOpen);

    if((!IsOSDOpen) || Force)
    {
        palDataMgr_UI_EventSend(edcUI_EVENT_SPLASH, 1, NULL);
    }
}

static void palDataPath_TurnOffSplash(void)
{
    if(palDataMgr_PIN_Protect_Checking() == ePASSWORD_PROTECT_UNLOCK)
    {
        m_sSourceDesc.eActiveDisplay = eDATA_PATH_ACTIVE_DISPLAY_EXTERNAL;  //A70LV_Doulas_0032 modify
        palDataMgr_UI_EventSend(edcUI_EVENT_SPLASH, 0, NULL);
    }
}

#if (ENABLE_COLOR_UNIFORMITY == TRUE)   //G100_Tim_0012, mod, start
// ==============================================================================
// DESCRIPTION: Set Color Uniformity data from the relative file if the file is valid.
// Params: None
// Returns: None
//
// Modification History
// --------------------
// 2021/06/21, Tim Chen modify
// --------------------
// ==============================================================================
static void palDataPath_Color_Uniformity_PictureMode_Check(void)
{
    UINT32 dwOffset = 0;

    // according to the G100 ACU rules :
    // 1. No Twist Link => remove it.
    // 2. Don't care the Source and the Display Mode => remove it
    // 3. if the ACU data file is valid, apply it.

    if(palDataMgr_Access_Color_Uniformity_Table_Offset_Get(&dwOffset) == eEXEC_CODE_PASS)
    {
        eHAL_WARPING_EXEC_CODE eExecCode = eHAL_WARPING_EXEC_CODE_FAIL;

        switch(dwOffset)
        {
            case eACU_FILE_0_VALID:
                eExecCode = palGeo_Color_Uniformity_File_Apply(COLOR_UNIFORMITY_DATA0);
                break;

            case eACU_FILE_1_VALID:
                eExecCode = palGeo_Color_Uniformity_File_Apply(COLOR_UNIFORMITY_DATA1);
                break;

            case eACU_FILE_2_VALID:
                eExecCode = palGeo_Color_Uniformity_File_Apply(COLOR_UNIFORMITY_DATA2);
                break;

            case eACU_FILE_3_VALID:
                eExecCode = palGeo_Color_Uniformity_File_Apply(COLOR_UNIFORMITY_DATA3);
                break;

            default:
                palGeo_Color_Uniformity_Enable_Set(0);      //G100_Tim_0024, add
                break;
        }

        LOG_MSG(db_APP_DATAPATH, "palDataPath_Color_Uniformity_PictureMode_Check(),  Set OK\r\n");
    }
    else
    {
        LOG_MSG(db_APP_DATAPATH, "palDataPath_Color_Uniformity_PictureMode_Check(), End, invalid data\r\n"); //G100_Tim_0018
        palGeo_Color_Uniformity_Enable_Set(0);      //G100_Tim_0024, mod
    }

#ifdef CUSTOM_CHRISTIE
	if(palDataMgr_CU_Data_Enable_Get() && (palDataMgr_CU_Data_Status_Get()))	//A70Gen2_Doulas_0051 ACU table//A35G2_Alan_0007
		palGeo_Color_Uniformity_Enable_Set(1);
	else
		palGeo_Color_Uniformity_Enable_Set(0);
#endif
}

//G100_Tim_0020, add, start
void palDataPath_Color_Uniformity_Recheck(void)
{
    palDataPath_Color_Uniformity_PictureMode_Check();
}
//G100_Tim_0020, add, end

#else //ENABLE_COLOR_UNIFORMITY         //G100_Tim_0012, mod, end

static void palDataPath_Color_Uniformity_PictureMode_Check(void)
{
#if (ENABLE_COLOR_UNIFORMITY == FALSE)  //G100_Simon_0090
    return;
#endif

    UINT32 dwOffset = 0;

    if(palGeo_ApLinkFlag_Get() == FALSE)
    {
        if(palDataMgr_Access_Color_Uniformity_Table_Offset_Get(&dwOffset) == eEXEC_CODE_PASS)
        {
            eHAL_WARPING_EXEC_CODE eExecCode = eHAL_WARPING_EXEC_CODE_FAIL;
            UINT8 ucDispalyMode = eCM_PICTURE_SETTINGS_BRIGHT;
            palDataMgr_Data_Access(edcPICTURE_SETTINGS, edaREAD, (void*)&ucDispalyMode);
            if(ucDispalyMode == eCM_PICTURE_SETTINGS_BLENDING)
            {
                switch(dwOffset)
                {
                    case WARPING_COLOR_UNIFORMITY_Offset_0:
                        eExecCode = palGeo_Color_Uniformity_File_Apply(COLOR_UNIFORMITY_DATA0);
                        break;

                    case WARPING_COLOR_UNIFORMITY_Offset_1:
                        eExecCode = palGeo_Color_Uniformity_File_Apply(COLOR_UNIFORMITY_DATA1);
                        break;

                    case WARPING_COLOR_UNIFORMITY_Offset_2:
                        eExecCode = palGeo_Color_Uniformity_File_Apply(COLOR_UNIFORMITY_DATA2);
                        break;

                    case WARPING_COLOR_UNIFORMITY_Offset_3:
                        eExecCode = palGeo_Color_Uniformity_File_Apply(COLOR_UNIFORMITY_DATA3);
                        break;

                    default:
                        break;
                }

                if(eExecCode == eHAL_WARPING_EXEC_CODE_PASS)
                {
                    palGeo_Color_Uniformity_Enable_Set(1);
                }
            }
            else
            {
                palGeo_Color_Uniformity_Enable_Set(0);
            }
        }
        else
        {
            palGeo_Color_Uniformity_Enable_Set(0);
        }
    }

}

#endif //ENABLE_COLOR_UNIFORMITY        //G100_Tim_0012, mod, end

static void palDataPath_GotoState(UINT32 ulNewState)
{
    eSOURCE_STATE ucSourceState;    //A70LV_Doulas_0241
    static eSOURCE_STATE ucLastSourceState = eDATA_PATH_STATE_INVALID;    //G100_Simon_0035
    eCM_TEST_PATTERN_ID ucHSG_Pattern = eCM_TEST_PATTERN_OFF;

    switch(ulNewState)
    {
        case eDATA_PATH_STATE_SPLASH_AT_STARTUP:
            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : eDATA_PATH_STATE_SPLASH_AT_STARTUP\r\n", __FUNCTION__, __LINE__);
            palDataPath_DisplaySplash(TRUE);
            palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL);         //A70LV_Doulas_0049
#if 0
            /* load splash */
            source_DisplaySplash(&sourceDesc);
            dispfmt_ConfigureForSource(TRUE);
            pictcont_ConfigureForInternalSource();
#endif
            /* poll every 0.5 seconds if time has elasped */
            m_sPalDataPathInfo.lPollPeriod = 500;//500ms
            break;

        case eDATA_PATH_STATE_SCALER_PORT_CONFIG:
            {
                /* poll every 0.5 seconds if time has elasped */

                //TestTimeStart = TMO_GetSysRunTime();
        #if 0 //###
                #ifdef CUSTOM_CHRISTIE
                if((GuiCb.fpGui_IsSplash_On_GetCb() == TRUE) && (halScaler_PIP_PBP_Enable_Get()))
                {
                    palDataMgr_UI_EventSend(edcUI_EVENT_SPLASH, 0, NULL);
                }
                #endif
        #endif
                palImgMgr_SetInputSource(eSOURCE_WINDOW_MAIN, m_sSourceDesc.eConnector);   //A70LV_Doulas_0007
                halFrontEndCtrl_MainInput_Set((UINT8*)&m_sSourceDesc.eConnector);    //A70LV_Doulas_0084 set input source
                halScaler_FrontEndScanMode_Clear(eSOURCE_WINDOW_MAIN);       //A70LV_Doulas_0175
                if(palDataPath_AlreadyShowDisconnectTwistMsg_Flag_Get() == ets_ON)// HICC2_Bruce_0008     //HICC2_Doulas_0053 remove//A70LV_Doulas_0367
                {
                    palDataPath_AlreadyShowDisconnectTwistMsg_Flag_Set(ets_OFF); //HICC2_Doulas_0053 remove
                    palDataMgr_UI_EventSend(edcUI_EVENT_DISCONNECT_TWIST_MSG, FALSE, NULL); //HICC2_Doulas_0053//disable Twist OSD Message
                }

                palLANProcSendToLAN(edcADV_WARP_CONTROL); //A35G2_Coda_0128

                //A35G2_CDS_Simon_0002
                if(palDataPath_CurrentSrcIsQuickSwitchSrc() == TRUE ||
                   m_sSourceDesc.eConnector == eCM_SOURCE_3GSDI)
                {
                    m_sPalDataPathInfo.lPollPeriod = 1;
                }
                else
                {
                    m_sPalDataPathInfo.lPollPeriod = 500;
                }

    			LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : eDATA_PATH_STATE_SCALER_PORT_CONFIG %d <%d>\r\n", __FUNCTION__, __LINE__,m_sSourceDesc.eConnector, TestTimeStart);


                m_sSourceDesc.ucSourcelostCount = 0;  //A70LV_Doulas_0329
            }
            break;

        case eDATA_PATH_STATE_SUSPENDED:
        default:
            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : eDATA_PATH_STATE_SUSPENDED\r\n", __FUNCTION__, __LINE__);
            /* suspend the task indefinitely */
            m_sPalDataPathInfo.lPollPeriod = 0;
            m_sSourceDesc.ucSourcelostCount = 0;  //A70LV_Doulas_0329
            break;

        case eDATA_PATH_STATE_TPG_DISPLAYED:
            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : eDATA_PATH_STATE_TPG_DISPLAYED\r\n", __FUNCTION__, __LINE__);
            m_sPalDataPathInfo.lPollPeriod = 500;    //A70LV_Doulas_0266 Add
            palDataPath_AutoDisableMenuTransparency(); //A70LV_Larry_0422

            if(m_sPalDataPathInfo.eDataPathState != eDATA_PATH_STATE_TPG_DISPLAYED)    //A70LV_Doulas_0308
            {
//                halFormatter_DMD_BGDisplayModeSet(BG_BLK);      //G100_Owen_0008 remove //DDP background on
            //    palEnvironment_LightSourceBlankingOn_Set(TRUE,1);      //A70Gen2_Doulas_0026 remove //LD off
                //MS_SLEEP(200);
            }
			//if(palDataPath_Last3DEnable_Get() == ets_ON)	//A70Gen2_Doulas_0021 remove//A70Gen2_Doulas_0017 Modify image error on 3D mode
			//{
			//	halFormatter_3D_FormatSet(eCM_3D_LR_REFERENCE_1ST_FRAME);
			//}
            //halScaler_ForcedSyncResetDisable_Set();       //A70Gen2_Doulas_0021 remove//A70LV_Doulas_0308 frame lock disable
            m_sSourceDesc.ucSourcelostCount = 0;  //A70LV_Doulas_0329
            //ucTestPatternSourceLostCount = 0;   //ZU860_Doulas_0124
            #if 0
            /* suspend the task indefinitely, alert GUI */
            datapathTaskPollPeriod = 0;
            guiSourceStatusMessage(GUI_SOURCE_TPG);
            #endif

            #ifdef CUSTOM_CHRISTIE
            palDataPath_DisplaySplash(TRUE);
            palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL);
            #endif
        break;

        case eDATA_PATH_STATE_BEGIN_SCAN:
            {
                LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : eDATA_PATH_STATE_BEGIN_SCAN <%d>\r\n", __FUNCTION__, __LINE__, TMO_GetSysRunTime());

                UINT8 ucInputKeyFunction = 0;
                UINT8 ucVal = 0;        //A70LV_Doulas_0326
                eRESULT eResult = rcINVALID;       //A70LV_Doulas_0367
                ucSourceState = eSOURCE_STATE_CHECKING_FOR_SIGNAL;    //A70LV_Doulas_0241
                /* manually flush mailbox to clean out old AutoLock status messages */
                /* without losing user messages                                     */
                if(m_sSourceDesc.ucSourcelostCount < 200)  //A70LV_Doulas_0329
                    m_sSourceDesc.ucSourcelostCount++;

                if(m_ucLast3DEnable_Setting == ets_ON)      //A70LV_Doulas_0367 Add
                {
                    eResult = halFormatter_3D_FormatGet(&ucVal);
                    if((ucVal == eCM_3D_LR_REFERENCE_FIELD_GPIO) &&
                       (eResult == rcSUCCESS))
                    {
                    //    palEnvironment_LightSourceBlankingOn_Set(TRUE,2); //H30K_Doulas_0028
                        halFormatter_3D_FormatSet(eCM_3D_LR_REFERENCE_1ST_FRAME);  //A70Gen2_Doulas_0024 //A70Gen2_Doulas_0023 remove//change to first frame mode
                        halFormatter_ChannelSourceSet(DISP_EXTERNAL_SOURCE);	   //A70Gen2_Doulas_0024 //A70Gen2_Doulas_0023 rmove
                        //palDataPath_Set3D_FirstFrameMode();		//A70Gen2_Doulas_0024 remove//A70Gen2_Doulas_0023
                    }
                }
				if(halFormatter_Get_Current_DLPLink_Pulse() == TRUE)
					halFormatter_3D_DLPLinkPulseSet(FALSE);

                palDataPath_FreezeChecking(eSOURCE_WINDOW_MAIN);   //A70LV_Doulas_0223 Freeze check
                halScaler_ForcedSyncResetDisable_Set();     //A70LV_Doulas_0281
                //palDataMgr_Data_Access(edcSHOW_MAINSOURCE_MESSAGES, edaREAD, &ucSourceState) ;//A70LV_Doulas_0075 modify source status
                //if(ucSourceState != eSOURCE_STATE_CHECKING_FOR_SIGNAL)
                //{
                    //ucSourceState = eSOURCE_STATE_CHECKING_FOR_SIGNAL;
                    //palDataMgr_Data_Access(edcSHOW_MAINSOURCE_MESSAGES, edaWRITE_RAM_ONLY_NO_ACTION, &ucSourceState) ;
                //}
#ifdef FIRST_CHKVIDRDY
                ucVal = 0;
                // Bit 0 retrn Main Source Status
                // Bit 2 ~ 4 retrn Crrent Main Source ID
                // 00:HDMI1, 01:HDMI2, 02:DisplayPort, 03:HDBASET, 04:3GSDI/12GSDI,
                halFrontEndCtrl_videoReady_Get(&ucVal);
                if(ucVal & 0x01)
#endif
                {
                    //uPALIMGMGR_INFO uInfo = {.sMeasureInput.ucCH = eSOURCE_WINDOW_MAIN};
                    //palImgMgr_MeasureInput(0);      //A70LV_Doulas_0142 Add  //A35G2_CDS_Simon_0054 remove
                }

                if(palDataMgr_DataCode_Control(edcINPUT_KEY) == eFUNC_CONTROL_ENABLE) //A70LV_Larry_0298
                {
                    palDataMgr_Data_Access(edcINPUT_KEY, edaREAD, &ucInputKeyFunction) ;
                }
                else
                {
                    ucInputKeyFunction = eSOURCE_KEY_CHANGE_SOURCE;
                }
                palDataMgr_Data_Access(edcSHOW_MAINSOURCE_MESSAGES, edaREAD, &ucSourceState) ;      //A70LV_Doulas_0142 Add
                if((m_sSourceDesc.ucSourcelostCount >= SOURCE_LOST_TIME)&&(m_sSourceDesc.eConnector == eCM_SOURCE_VGA))  //ZU860_Doulas_0104 modify
                {
                    if(m_sSourceDesc.ucSourcelostCount == SOURCE_LOST_TIME)
                    {
                        ucSourceState = eSOURCE_STATE_SIGNAL_OUT_OF_RANGE;
                        palDataMgr_Data_Access(edcSHOW_MAINSOURCE_MESSAGES, edaWRITE_RAM_ONLY_NO_ACTION, &ucSourceState);
                    }
                }
                else if(ucInputKeyFunction == eSOURCE_KEY_CHANGE_SOURCE_AUTO && ucSourceState != eSOURCE_STATE_SEARCHING)
                {
                    ucSourceState = eSOURCE_STATE_SEARCHING;
                    palDataMgr_Data_Access(edcSHOW_MAINSOURCE_MESSAGES, edaWRITE_RAM_ONLY_NO_ACTION, &ucSourceState);
                }
                else if(ucInputKeyFunction != eSOURCE_KEY_CHANGE_SOURCE_AUTO && ucSourceState != eSOURCE_STATE_CHECKING_FOR_SIGNAL)
                {
                    ucSourceState = eSOURCE_STATE_CHECKING_FOR_SIGNAL;
                    palDataMgr_Data_Access(edcSHOW_MAINSOURCE_MESSAGES, edaWRITE_RAM_ONLY_NO_ACTION, &ucSourceState);
                }
                //palDataPath_DisplaySplash();
                //appGui_SendSourceMsgEvent();

                if(!palDataMgr_IsOsdOpen()) //HICC2_Larry_0033
                {
                    palDataMgr_UI_EventSend(edcUI_EVENT_SOURCE_MENU_OPEN_SET, TRUE, NULL);
                    palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL);// HICC2_Bruce_0008
                }

                //halScaler_ForcedSyncResetDisable_Set();     //A70LV_Doulas_0281 remove//A70LV_Doulas_0079
                palDataPath_ShowLogo();     //A70LV_Doulas_0049
                if(palDataPath_IsSourceMonitorSub() == FALSE)    //A70LV_Doulas_0310
                {
                    palDataPath_AutoDisableMenuTransparency();   //A70LV_Doulas_0122 auto dsiable Menu Transparency
                }
                //halADCCalibrationCheckFlagSet(FALSE);            //A70LV_Doulas_0124
        		#ifdef SCALER_C821_C789
                halC789Ctrl_V_Start_Checking();         //A70LV_Doulas_0157
                #endif
                palDataPath_ModeAdjusmenttDisableSetting();       //A70LV_Doulas_0209 modify//A70LV_Doulas_0195
                ucVal = 0;
                palDataMgr_Data_Access(edcLOGO_CHANGE, edaREAD, &ucVal);
                if(ucVal != eCM_CHANGE_LOGO_CAPTURED)
                {
                    halScaler_SourceOutputOff(eSOURCE_WINDOW_MAIN);     //ZU860_Doulas_0124
                }

             //   m_sPalDataPathInfo.ucFlushIndex++;                        //A70LV_Doulas_0076 remove
             //   appMailBox_Send(m_sPalDataPathInfo.xMsgQueue,
             //                   m_sPalDataPathInfo.xEventGroupHandle,
             //                   eDATA_PATH_MSG_FLUSHED, -1, m_sPalDataPathInfo.ucFlushIndex, 0, FALSE);

                if(palImgMgr_WB_OutputEnableGet() == FALSE)   //A70LV_Doulas_0079 avoid C789 always output disable
                {
                    if(palDataPath_GetPanelChangeState() == ets_OFF)  //A35G2_CDS_Simon_0061
                    {
                        palImgMgr_WB_OutputEnableSet(1);
                    }
                }

                /* poll every 0.5 seconds if time has elasped */
                m_sPalDataPathInfo.lPollPeriod = 100;//100ms    //A70LV_Doulas_0120 modify//A70LV_Doulas_0075modify
                #ifdef CUSTOM_CHRISTIE
                m_DataPathLastState = (eDATA_PATH_STATE)ulNewState;
                #endif

                LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : eDATA_PATH_STATE_BEGIN_SCAN End <%d>\r\n", __FUNCTION__, __LINE__, TMO_GetSysRunTime());//A70LV_Doulas_0231 debug
            }

            if(palDataMgr_PIN_Protect_Checking() == ePASSWORD_PROTECT_LOCKED)   //G100_Coda_0081
            {
                UINT8 ucBurnInEnable = 0;   //A70LV_Doulas_0106
                palDataMgr_Data_Access(edcBURNIN_ENABLE, edaREAD, &ucBurnInEnable);

                if(ucBurnInEnable == ets_OFF)
                {
                    palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL); //G100_Coda_0072
                }
                break;
            }

            break;

        case eDATA_PATH_STATE_LOOK_FOR_SYNCS:
            {
                LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : eDATA_PATH_STATE_LOOK_FOR_SYNCS <%d>\r\n", __FUNCTION__, __LINE__, TMO_GetSysRunTime());

                if(palGeo_ApLinkFlag_Get() == TRUE &&
                   palDataPath_IsErrorInputGetForTwistOn() == TRUE )  //change panel when twist on
                {
                    m_sPalDataPathInfo.lPollPeriod = 1000;
                }
                else if(palDataPath_CurrentSrcIsQuickSwitchSrc() == TRUE)  //A35G2_CDS_Simon_0002
                {
                    m_sPalDataPathInfo.lPollPeriod = 10;
                }
                else if(m_sSourceDesc.eConnector == eCM_SOURCE_3GSDI)
                {
                    UINT8 ucVal = 0;
                    // Bit 0 retrn Main Source Status
                    // Bit 2 ~ 4 retrn Crrent Main Source ID
                    // 00:HDMI1, 01:HDMI2, 02:DisplayPort, 03:HDBASET, 04:3GSDI/12GSDI,
                    halFrontEndCtrl_videoReady_Get(&ucVal);
                    if(ucVal & 0x01)  //3GSDI source ready
                    {
                        if(((ucVal>>2)&0x07) != (UINT8)CM2GUI(edcMAIN_INPUT, m_sSourceDesc.eConnector))  //A35G2_CDS_Simon_0002
                        {
                            m_sPalDataPathInfo.lPollPeriod = 10;
                        }
                        else
                        {
                            m_sPalDataPathInfo.lPollPeriod = 150;
                        }
                    }
                    else
                    {
                        m_sPalDataPathInfo.lPollPeriod = 150;  //A35G2_CDS_Simon_0002
                    }
                }
                else
                {
                    m_sPalDataPathInfo.lPollPeriod = 150;
                }

            }
 #ifdef Low_Latency_All //A70Gen2_Doulas_0053//A35G2_Alan_0035
 			{
				UINT8 ucVlaue = FALSE;
				palDataMgr_Data_Access(edcLOW_LATENCY_MODE, edaREAD, &ucVlaue);
				if(((halScaler_IS_3D_Enable() == ets_ON) || FrontEnd_Down_Scaling_Get()) && halScaler_Is_LowLatencyMode_On())    //G100_Clare_0053   //H2PF_Simon_0187
				{
					if(ucVlaue)
					{
						ucVlaue = FALSE;
						palDataMgr_Data_Access(edcLOW_LATENCY_MODE, edaWRITE_THROUGH_NO_ACTION, &ucVlaue);
						palImgMgr_WB_OutputEnableSet(FALSE);
						halFormatter_FRCByPassModeSet(FALSE);  //A35G2_CDS_Simon_0009
						palDataPath_Low_Latency_Set(FALSE);
						#if defined(CUSTOM_BARCO) //A35G2_BRC_Casper_0098
				        utilCounterSet(eCOUNTER_TYPE_OSD_GRAY_OUT_MONITOR, 2000); //A35G2_BRC_Casper_0101
						#endif

						if(palImgMgr_WB_OutputEnableGet() == FALSE)
						{
							palImgMgr_WB_OutputEnableSet(TRUE);
						}
				        halFormatter_ChannelSourceSet(DISP_EXTERNAL_SOURCE);    //A70LV_Doulas_0299 DDP normal

					}
				}
 			}
#endif	/*Low_Latency_All*/
            break;

        case eDATA_PATH_STATE_ATTEMPT_LOCK:
            {
                if((m_sSourceDesc.ucSourcelostCount < SOURCE_LOST_TIME)||(m_sSourceDesc.eConnector != eCM_SOURCE_VGA))  //ZU860_Doulas_0104 modify
                {
                    ucSourceState = eSOURCE_STATE_SETTING_UP_IMAGE;        //A70LV_Doulas_0241 modify //A70LV_Larry_0350
                    palDataMgr_Data_Access(edcSHOW_MAINSOURCE_MESSAGES, edaWRITE_RAM_ONLY_NO_ACTION, &ucSourceState) ;      //A70LV_Doulas_0075 Add source status
                    palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL);        //A70LV_Doulas_0049 modify
                }

                LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : eDATA_PATH_STATE_ATTEMPT_LOCK <%d>\r\n", __FUNCTION__, __LINE__, TMO_GetSysRunTime());
            /* syncs found -- wait to lock to source */
            /* poll every 100 milliseconds if the source is detected */
			    //palDataPath_TurnOffSplash();        //A70LV_Doulas_0254 remove//A70LV_Doulas_0032
                m_sPalDataPathInfo.lPollPeriod = 100;//100ms    //A70LV_Doulas_0007
            }
            break;

        case eDATA_PATH_STATE_AUTO_PHASE:     //A70LV_Doulas_0007
            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : eDATA_PATH_STATE_AUTO_PHASE <%d>\r\n", __FUNCTION__, __LINE__, TMO_GetSysRunTime());
            /* poll every 50 milliseconds if the source is detected */
            palDataMgr_ResetPhaseRam();     //A70LV_Doulas_0117
            m_sPalDataPathInfo.lPollPeriod = 50;
            break;

        case eDATA_PATH_STATE_AUTO_POSITION:     //A70LV_Doulas_0007
            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : eDATA_PATH_STATE_AUTO_POSITION <%d>\r\n", __FUNCTION__, __LINE__, TMO_GetSysRunTime());
            /* poll every 50 milliseconds if the source is detected */
            halScaler_AutoPsitionStart(eSOURCE_WINDOW_MAIN,TRUE);       //A70LV_Doulas_0118
            m_sPalDataPathInfo.lPollPeriod = 50;
            break;

        case eDATA_PATH_STATE_MONITOR_SOURCE:
        {
            UINT8 IsOSDOpen = 0;

            ucXPR_Reset_Timeout = XPR_RESET_TIMEOUT;
		    ucSourceState = eSOURCE_STATE_SHOW_SOURCE_RESOLUTION;     //A70LV_Doulas_0241 modify

            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : eDATA_PATH_STATE_MONITOR_SOURCE <%d>\r\n", __FUNCTION__, __LINE__ , TMO_GetSysRunTime());
            palDataMgr_Data_Access(edcSHOW_MAINSOURCE_MESSAGES, edaWRITE_RAM_ONLY_NO_ACTION, &ucSourceState) ; //ZU860_Doulas_0054//A70LV_Larry_0350
            //A35G2_BRC_Coda_0011: Re-get HDR flag
            UINT8 ucVideoFormat[eFE_MSG_VIDEO_FORMAT_MAIN_SZ] = {0};
            halFrontEndCtrl_videoFormat_Get(ucVideoFormat);
            LOG_MSG(db_APP_DATAPATH, " m_FrontEndVideoFormat.u8VideoHDRType : %d\r\n", m_FrontEndVideoFormat.u8VideoHDRType);
            LOG_MSG(db_APP_DATAPATH, " sucVideoFormat[10] : %d\r\n", ucVideoFormat[10]);
            if(m_FrontEndVideoFormat.u8VideoHDRType != ucVideoFormat[10])
            {
                m_FrontEndVideoFormat.u8VideoHDRType = ucVideoFormat[10];
				palLANProcSendToLAN(edcDATAPATH_HDR_INFO);
                palDataMgr_DisplayModeChecking();

                palDataMgr_Data_Access(edcUI_STATUS_IS_OSD_OPEN, edaREAD, &IsOSDOpen);
                if(IsOSDOpen == 1)
                {
                //    palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL); //H30K_Doulas_0078
                }

                //HDR Handle
                LOG_MSG(db_APP_DATAPATH, "HDR DRM Changed\r\n");
            }
            ucTestPatternSourceLostCount = 0;

            #if (SCALER_HDR_ENABLE == TRUE)   //H2PF_Simon_0193
            palDataPath_HDR_Process();
            #endif

            palDataMgr_Data_Access(edcHSG_TEST_PATTERN_CTRL, edaREAD, &ucHSG_Pattern) ;
            if(ucHSG_Pattern == eCM_TEST_PATTERN_OFF) //(appGui_HSG_TestPatternEnableGet() == FALSE)   //A70LV_Doulas_0296 Modify HSG Test pattern bug
            {
                if(palDataMgr_CurTestPatternGet() == eTID_OFF)
                {
                    palDataPath_TurnOffSplash() ; //A70LV_Larry_0350
                    #ifdef CUSTOM_OPTOMA_ZU860  //EK816U_626U_Owen_0003
                    appGui_SendSourceInfoUpdate(); //ZU860_Larry_0014
                    #else
                    palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL);        //H30K_Doulas_0079//H30K_Doulas_0078//A70LV_Doulas_0208 Add
                    #endif
                }
            }
            palDataPath_Test_GetTimeElasped(1);

            #ifdef FREEZE_C789_ON_SYNC_MODE_CHANGED
            if(halScaler_PIP_PBP_Enable_Get() == FALSE)     //A35G2_CDS_Simon_0003
            {
                palImgMgr_WB_Freeze(TRUE);
            }
            #endif

            m_sPalDataPathInfo.ucSyncLockCount = 0;      //A70LV_Doulas_0009 Add    //A35G2_CDS_Simon_0003
            palGeo_FrameMemoryCheck();  //H2PF_Simon_0131 //H2PF_Simon_0135
            palDataPath_Test_GetTimeElasped(2);
            palImgMgr_ConfigureForDisplay(0, 1);    //A70LV_Doulas_0007
            palDataPath_Test_GetTimeElasped(3);

            if(palDataPath_GetPanelChangeState() == ets_ON)
            {
                LOG_MSG(db_HAL_WARPING, "re-config warping/blending due to panel change\n");

                #ifdef CUSTOM_CHRISTIE  //A35G2_Simon_0093
                UINT8 ucTwistLink = palGeo_ApLinkFlag_Get();  //A35G2_Simon_0086
                UINT8 ucWarpMemoryType = palDataMgr_CurrentWarpMemoryType();
                UINT8 ucBlendMemoryType = palDataMgr_CurrentBlendMemoryType();

                if(ucTwistLink == TRUE)
                {
                    LOG_MSG(db_HAL_WARPING,  "Error : panel change at twist link on\n");
                    LOG_MSG(db_APP_DATAPATH, "Error : panel change at twist link on\n");
                }
                else //twist off : reconfig
                {
                    if( ucWarpMemoryType == WARPING_TYPE_AP_MYSTIQUE )
                    {
                        UINT8 ucApplyIndex = 0;
                        palDataMgr_Data_Access(edcWARP_MEMORY_APPLY, edaREAD, &ucApplyIndex);
                        palDataMgr_Data_Access(edcWARP_MEMORY_APPLY, edaWRITE_RAM_ONLY_WITH_ACTION, &ucApplyIndex);
                    }
                    else
                    {
                        palGeo_ADV_WarpingParameterInit();
                        palGeo_Warp_GeometrySet();
                    }

                    if( ucBlendMemoryType == WARPING_TYPE_AP_MYSTIQUE )
                    {
                        UINT8 ucApplyIndex = 0;
                        palDataMgr_Data_Access(edcBLEND_MEMORY_APPLY, edaREAD, &ucApplyIndex);
                        palDataMgr_Data_Access(edcBLEND_MEMORY_APPLY, edaWRITE_RAM_ONLY_WITH_ACTION, &ucApplyIndex);
                    }
                    else
                    {
                        palGeo_ADV_WarpingParameterInit();
                        palGeo_Warp_BlendingSet();
                        palDataMgr_MenuOffsetUpdateForEdgeBlending();
                    }
                }
                #endif
            }

            #ifdef C789_2D_NO_FRAME_LOCK       //A70LV_Doulas_0311 modify
            if((halScaler_IS_3D_Enable() == ets_ON) ||
               (palImgMgr_WB_OutputChenged_Get())    ||
                (palSystem_PanelID_Get() == ePANEL_ID_WUXGA_120HZ) ||    //A70LV_Doulas_0377 Add
               (palSystem_PanelID_Get() == ePANEL_ID_1080P_120HZ))  //ZU860_Doulas_0083
            {
                palEnvironment_LightSourceBlankingOn_Set(TRUE,4);
            }
            else
            {
            //    palEnvironment_LightSourceBlankingOn_Set(TRUE,1);	//A70Gen2_Doulas_0026
            }
            #else
            if((halScaler_IS_3D_Enable() == ets_ON) ||
               (palImgMgr_WB_OutputChenged_Get())    ||
                (palSystem_PanelID_Get() == ePANEL_ID_WUXGA_120HZ) ||
               (palSystem_PanelID_Get() == ePANEL_ID_1080P_120HZ) ||
               (palDataPath_GetPanelChangeState() == ets_ON))
                palEnvironment_LightSourceBlankingOn_Set(TRUE,7);   //H30K_Doulas_0039//A70LV_Doulas_0294
            #endif

            if(palSysCtrlMgr_XillinxFPGA_NewVersionGet() == TRUE)        //A70LV_Doulas_0218
            {
                palSysCtrlMgr_XillinxFPGA_Manual_Mode_Set(m_sPalDataPathInfo.ePanelTimingId);    //A70LV_Doulas_0216
            }

            #ifdef CHECKING_OUTPUT_3D_FIELD //H30K_Doulas_0035//A35G2_Simon_0121 Start
            UINT16 uiV_Freq = 0; //H30K_Doulas_0038
            halScaler_InputVertRefresh2_Get(eSOURCE_WINDOW_MAIN,&uiV_Freq); //H30K_Doulas_0038
            if((halScaler_IS_3D_Enable() == ets_ON) &&
                (((uiV_Freq > 5960) && (uiV_Freq < 6030)) || ((uiV_Freq > 4960) && (uiV_Freq < 5030))) &&  // 50/60hz
                ((halScaler_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_TOPANDBOTTOM) ||
                  (halScaler_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_FRAMEPACKING) ||
                  (halScaler_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_SIDEBYSIDE))) //H30K_Doulas_0038
            {
                MS_SLEEP(700);  //wait output stable //H30K_Doulas_0039
                appDataPath_CheckScalerOutput3DField();
            }
			#endif//A35G2_Simon_0121 End

			palDataMgr_Format_Normal_WithSemaphoreSet();     //H30K_Doulas_0075//A70LV_Doulas_0046

		    #ifdef SCALER_C821_C789
            halC789Ctrl_V_Start_Checking();     //A70LV_Doulas_0081 auto update C789 input V-start
            #endif

            palDataPath_MenuTransparencyEnableSet(TRUE); //HICC2_AC_0022

            #ifdef FREEZE_C789_ON_SYNC_MODE_CHANGED
            //if(halScaler_PIP_PBP_Enable_Get() == FALSE)      //A35G2_CDS_Simon_0003
            {
                palImgMgr_WB_Freeze(FALSE);
            }
            #endif

            m_sPalDataPathInfo.lPollPeriod = 1000;//1000ms    //A70LV_Doulas_0164 modify//A70LV_Doulas_0122 Modify
            if(palSysCtrlMgr_XillinxFPGA_NewVersionGet() == FALSE)        //A70LV_Doulas_0218
            {
                palEnvironment_XillinxFPGA_Reset();     //A70LV_Doulas_0216 remove//A70LV_Doulas_0179 Add
            }

            if(m_sPalDataPathInfo.uiXPRPanelChange)
            {
                UINT8 ucXPRErrorStatus = 0;

                m_sPalDataPathInfo.uiXPRPanelChange = 0;

                //if(m_sPalDataPathInfo.uiXPRPanelChange == (XPR_PANEL_CHECK_COUNT - 1))
                {
                    halFormatter_XPR_ErrorStatus_Get(&ucXPRErrorStatus);

                    if(ucXPRErrorStatus)
                    {
                        halFormatter_XPR_ErrorStatus_Set();
                    }
                }
            }


			#ifdef Low_Latency_All
			if(((halScaler_IS_3D_Enable() == ets_ON) || FrontEnd_Down_Scaling_Get()) && halScaler_Is_LowLatencyMode_On())	//G100_Clare_0053   //H2PF_Simon_0187
			{
				#if 0	//A70Gen2_Doulas_0053 remove
				UINT8 ucVlaue = FALSE;
				palDataMgr_Data_Access(edcLOW_LATENCY_MODE, edaWRITE_THROUGH_NO_ACTION, &ucVlaue);
				palDataPath_Low_Latency_Set(FALSE);
        		halFormatter_FRCByPassModeSet(FALSE);   //A35G2_CDS_Simon_0004
				/*halFormatter_FRCByPassModeSet(&ucVlaue);
				halScaler_Low_Latency_Set(ucVlaue);
				halC789Ctrl_Low_Latency_Set(ucVlaue);*/
				halC789Ctrl_Change_Panel(palSystem_PanelID_Get());
				#endif
			}
            else //G100_Larry_0028
            {
                //LOG_MSG(1, "halScaler_Input_3D_Format_Get() = %d\n", halScaler_Input_3D_Format_Get());
                //LOG_MSG(1, "halScaler_IS_3D_Enable() = %d\n", halScaler_IS_3D_Enable());
                //LOG_MSG(1, "halScaler_3D_Type() = %d\n", halScaler_3D_Type());

                if(halScaler_3D_Type() == eINPUT_3D_TYPE2_OFF)
                {
                    halFormatter_FRCByPassModeSet(TRUE);
                }
                else
                {
                    //halFormatter_FRCByPassModeSet(FALSE);
                }
            }

            #endif	/*Low_Latency_All*/

            if(palDataPath_AlreadyShowDisconnectTwistMsg_Flag_Get() == ets_ON) // HICC2_Bruce_0008    //HICC2_Doulas_0053 remove//ZU860_Doulas_0138
            {
                palDataPath_AlreadyShowDisconnectTwistMsg_Flag_Set(ets_OFF);// HICC2_Bruce_0008 //HICC2_Doulas_0053 remove
                palDataMgr_UI_EventSend(edcUI_EVENT_DISCONNECT_TWIST_MSG, FALSE, NULL);       //disable Twist OSD Message
            }

            m_sSourceDesc.ucSourcelostCount = 0;  //ZU860_Doulas_0104
            palLANProcSendToLAN(edcCOLOR_WHEEL_SPEED);//A35G2_Coda_0110
            palEnvironment_FEOPD_Cnt_Set(10); //G100_Steven_0059
			palDataMgr_OPDSourceInfo(0);

            if(palDataPath_GetPanelChangeState() == ets_ON)
            {
                if((palImgMgr_IS_3D_Enable() != ets_ON) &&
                   (palSystem_PanelID_Get() != PANEL_3D_OUTPUT) &&
                   (palSystem_PanelID_Get() != PANEL_2D_HIGHSPEED))
                    halFormatter_ChannelSourceSet(DISP_EXTERNAL_SOURCE); //A35G2_CDS_Simon_0061
                palSystem_WaitWheelStable(); //A35G2_CDS_Simon_0061
                palDataMgr_Formatter_HSG_CE_Set(); //A35G2_BRC_Casper_0096
                //set Gamma //Must be after "palDataMgr_Formatter_HSG_CE_Set()"
                palDataPath_ConfigGamma(); //A35G2_CDS_Simon_0061

                palDataPath_SetPanelChangeState(ets_OFF);
				palEnvironment_LightSourceBlankingOn_Set(FALSE,0);   //A70Gen2_Doulas_0026
            }

            palImgMgr_WB_OutputEnableSet(1);         //A70LV_Doulas_0193 Add //A35G2_CDS_Simon_0061
            TestTimeEnd = TMO_GetSysRunTime();
            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : eDATA_PATH_STATE_MONITOR_SOURCE  End <%d>\r\n", __FUNCTION__, __LINE__, TestTimeEnd);
            palDataPath_Test_GetTimeElasped(4);
            palLANProcSendToLAN(edcAUTO_IMAGE); //H30K_Doulas_0019
            palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL); //H30K_Doulas_0075
        }
        break;
    }

    palDataPath_ResetTimeInState();
    m_sPalDataPathInfo.eDataPathState = (eDATA_PATH_STATE)ulNewState;

    if(ulNewState == eDATA_PATH_STATE_BEGIN_SCAN)   //G100_Simon_0035
    {
        if(ucLastSourceState != eDATA_PATH_STATE_BEGIN_SCAN && ucLastSourceState != eDATA_PATH_STATE_LOOK_FOR_SYNCS)
        {
            palLANProcForceCheckUpdateData();
        }
    }

    if(ulNewState == eDATA_PATH_STATE_MONITOR_SOURCE)  //HICC2_Doulas_0053
    {
        if(ucLastSourceState != eDATA_PATH_STATE_MONITOR_SOURCE)
        {
            palLANProcForceCheckUpdateData();
        }
    }

    if(ucLastSourceState != m_sPalDataPathInfo.eDataPathState)
    {
        if((ulNewState >= eDATA_PATH_STATE_SCALER_PORT_CONFIG) &&
           (ulNewState <= eDATA_PATH_STATE_ATTEMPT_LOCK))
        {
            m_sPalDataPathInfo.ucSourceConnectChange = 0;
        }
        palLANProcSendToLAN((UINT16)edcIS_SOURCE_LOCK);
    }

    ucLastSourceState = m_sPalDataPathInfo.eDataPathState;
}

eDATA_PATH_STATE palDataPath_GetDataPathState(void) //A70LV_Larry_0108
{
    return m_sPalDataPathInfo.eDataPathState;
}


static void palDataPath_StartDisplay(void)
{
    UINT8 ucData = 0; //A70LV_Larry_0051
    UINT8 ucFirstLanguage = 0;
    UINT8 ucPictureMode = 0;
    UINT8 ucPreUserMode = 0;
    UINT16  uiData = 0;
	UINT32  ulData = 0;
    UINT8   ucSN[32] = {0};
    UINT16  uiInput = 0; //H30K_Doulas_0073
    //UINT32 Data = 0 ;
    //palDataMgr_Data_Access(edcAFN_BACKUP_RESTORE_RESTORE, edaWRITE_RAM_ONLY_WITH_ACTION, &Data); //HICC2_Doulas_0039

#if 0
    adccontrol_powerNormal();
    dvicontrol_powerNormal();
    deccontrol_powerNormal();
#endif
    palDataPath_InitInputSource();  //A70LV_Doulas_0007
    palDataPath_CheckFrontendReady();

    ucData = 0xff; //A70LV_Larry_0051
    halFrontEndCtrl_HDMI_OUT_Set(&ucData);

    palDataMgr_Data_Access(edcEQ_MODE_HDMI1, edaREAD, &ucData);
    palDataMgr_Data_Access(edcEQ_MODE_HDMI1, edaWRITE_RAM_ONLY_WITH_ACTION, &ucData);
    palDataMgr_Data_Access(edcEQ_MODE_HDMI2, edaREAD, &ucData);
    palDataMgr_Data_Access(edcEQ_MODE_HDMI2, edaWRITE_RAM_ONLY_WITH_ACTION, &ucData);
    palDataMgr_Data_Access(edcEQ_MODE_DVI, edaREAD, &ucData);
    palDataMgr_Data_Access(edcEQ_MODE_DVI, edaWRITE_RAM_ONLY_WITH_ACTION, &ucData);

    palDataPath_FreezeChecking(0);  //G100_Simon_0044
    palDataMgr_Data_Access(edcPICTURE_SETTINGS, edaREAD, &ucData);      //ZU860_Doulas_0076 Add
    if((ucData == eCM_PICTURE_SETTINGS_3D)||(ucData == eCM_PICTURE_SETTINGS_2DHIGHSPEED))               //ZU860_Doulas_0076 Add
    {
        ucData = eCM_PICTURE_SETTINGS_BRIGHT;
        palDataMgr_Data_Access(edcPICTURE_SETTINGS, edaWRITE_THROUGH_NO_ACTION, &ucData) ;
    }
    else if(ucData == eCM_PICTURE_SETTINGS_USER) //A35G2_CDS_Larry_0027
    {
        palDataMgr_Data_Access(edcUSER_COLOR_MODE, edaREAD, &ucData);

        if((ucData == eCM_PICTURE_SETTINGS_3D) || (ucData == eCM_PICTURE_SETTINGS_2DHIGHSPEED))
        {
            ucData = eCM_PICTURE_SETTINGS_BRIGHT;
            palDataMgr_Data_Access(edcUSER_COLOR_MODE, edaWRITE_THROUGH_NO_ACTION, &ucData) ;
        }
    }
    palDataMgr_Scaler_Mode_Adjustment_EEPROM_Iint();  //A70LV_Doulas_0195 Add

	//set actuator	//HICC2_Zonic_0008
    palDataMgr_Data_Access(edcCH0_DACGAIN, edaREAD, &ucData);
    palDataMgr_Data_Access(edcCH0_DACGAIN, edaWRITE_RAM_ONLY_WITH_ACTION, &ucData);
    palDataMgr_Data_Access(edcCH0_SFDELAY, edaREAD, &ulData);
    palDataMgr_Data_Access(edcCH0_SFDELAY, edaWRITE_RAM_ONLY_WITH_ACTION, &ulData);
    palDataMgr_Data_Access(edcCH0_SEGMENTLEN, edaREAD, &uiData);
    palDataMgr_Data_Access(edcCH0_SEGMENTLEN, edaWRITE_RAM_ONLY_WITH_ACTION, &uiData);

    palDataMgr_Data_Access(edcCH1_DACGAIN, edaREAD, &ucData);
    palDataMgr_Data_Access(edcCH1_DACGAIN, edaWRITE_RAM_ONLY_WITH_ACTION, &ucData);
    palDataMgr_Data_Access(edcCH1_SFDELAY, edaREAD, &ulData);
    palDataMgr_Data_Access(edcCH1_SFDELAY, edaWRITE_RAM_ONLY_WITH_ACTION, &ulData);
    palDataMgr_Data_Access(edcCH1_SEGMENTLEN, edaREAD, &uiData);
    palDataMgr_Data_Access(edcCH1_SEGMENTLEN, edaWRITE_RAM_ONLY_WITH_ACTION, &uiData);

    palDataMgr_Data_Access(edcFIRSTSTARTUPFLAG, edaREAD, &ucFirstLanguage);

    if(ucFirstLanguage)
    {
        palDataMgr_Data_Access(edcPICTURE_SETTINGS, edaREAD, &ucPictureMode);
        palDataMgr_Data_Access(edcUSER_COLOR_MODE, edaREAD, &ucPreUserMode);

        if((ucPictureMode == eCM_PICTURE_SETTINGS_USER &&
            (ucPreUserMode == eCM_PICTURE_SETTINGS_2DHIGHSPEED ||
            ucPreUserMode == eCM_PICTURE_SETTINGS_3D_PASSIVE ||
            ucPreUserMode == eCM_PICTURE_SETTINGS_3D)) ||
            ucPictureMode == eCM_PICTURE_SETTINGS_2DHIGHSPEED ||
            ucPictureMode == eCM_PICTURE_SETTINGS_3D_PASSIVE ||
            ucPictureMode == eCM_PICTURE_SETTINGS_3D)
        {
            ucPictureMode = eCM_PICTURE_SETTINGS_BRIGHT;  //default picture mode
            palDataMgr_Data_Access(edcPICTURE_SETTINGS, edaWRITE_RAM_ONLY_NO_ACTION, &ucPictureMode);
        }
    }

    #ifdef QUICKLY_POWER_ON	//G100_Doulas_0024 Modify
	#else
    //palDataMgr_Format_Normal_Set();   //A70LV_Doulas_0046 Add,init DDP442x
    palDataMgr_Format_Init();   //X35G2_Simon_0003
    #endif

    palDataMgr_Data_Access(edcPICTURE_SETTINGS, edaREAD, &ucPictureMode);
    palDataMgr_Data_Access(edcUSER_COLOR_MODE, edaREAD, &ucPreUserMode);

    if((ucPictureMode == eCM_PICTURE_SETTINGS_USER &&
        (ucPreUserMode == eCM_PICTURE_SETTINGS_2DHIGHSPEED ||
        ucPreUserMode == eCM_PICTURE_SETTINGS_3D_PASSIVE ||
        ucPreUserMode == eCM_PICTURE_SETTINGS_3D)) ||
        ucPictureMode == eCM_PICTURE_SETTINGS_2DHIGHSPEED ||
        ucPictureMode == eCM_PICTURE_SETTINGS_3D_PASSIVE ||
        ucPictureMode == eCM_PICTURE_SETTINGS_3D)
    {
        palDataMgr_Formatter_ForcePictureMode_Set(eCM_PICTURE_SETTINGS_BRIGHT); //default picture mode
    }

#if 1//def PLATFORM_H60_2K //HICC2_Doulas_0018 read RX24T version
    {
        UINT16  wRX24TVer = 0;
        UINT8  aucVerString[32] = {0};

        halFormatter_RX24TVersion_Get(&wRX24TVer);

        LOG_MSG(db_APP_SYSTEM, "RX24T      P%02d.%02d\n",
            (UINT8)(wRX24TVer),
            (UINT8)(wRX24TVer>>8));

        sprintf((char*)aucVerString, "P%02d.%02d" ,
            (UINT8)(wRX24TVer),
            (UINT8)(wRX24TVer>>8));

        palDataMgr_Data_Access(edcPMCU_VERSION, edaWRITE_RAM_ONLY_NO_ACTION, aucVerString);
    }
#endif /* ON_BOARD */

    palDataPath_QuickSwitch_SrcReady_Set(FALSE);
    palDataPath_InitVGA_SyncThreshold();    //A70LV_Doulas_0114
    palDataPath_Init_FrontEnd_PIP_Setting();    //A70LV_Doulas_0120

    palFormatterMgr_XPR_SN_Get(ucSN);
    palDataMgr_Data_Access(edcCLOSE_LOOP_SN, edaWRITE_RAM_ONLY_WITH_ACTION, ucSN);

    if(palDataMgr_PIN_Protect_Checking() == ePASSWORD_PROTECT_UNLOCK)
    {
 	    palDataMgr_Data_Access(edcHDMI_OUT,edaREAD, &ucData);
 	    palDataMgr_Data_Access(edcHDMI_OUT,edaWRITE_RAM_ONLY_WITH_ACTION, &ucData);
    }
#ifdef CUSTOM_BARCO
  	palDataMgr_Data_Access(edcAUTO_HDMI_SWITCH, edaREAD, &ucData);
 	if(ucData == TRUE)  //A35G2_BRC_Simon_0006
 	{
 	    palDataMgr_Data_Access(edcAUTO_HDMI_SWITCH, edaWRITE_RAM_ONLY_WITH_ACTION, &ucData);
 	}
 	else
 	{
     	palDataMgr_Data_Access(edcBACKUPINPUT_AUTOSWITCH, edaREAD, &ucData);
 	    palDataMgr_Data_Access(edcBACKUPINPUT_AUTOSWITCH, edaWRITE_RAM_ONLY_WITH_ACTION, &ucData);
 	}
#else
 	palDataMgr_Data_Access(edcBACKUPINPUT_AUTOSWITCH, edaREAD, &ucData);
	palDataMgr_Data_Access(edcBACKUPINPUT_AUTOSWITCH, edaWRITE_RAM_ONLY_WITH_ACTION, &ucData);
#endif
    //palDataMgr_Init_FrontEnd_BackUpInput();  //G100_Steven_0036
    //halFormatter_3D_DLPLinkPulseSet(FALSE);          //ZU860_Doulas_0038 remove//A70LV_Doulas_0161 init DLP Link off

    ////Set BK input //G100_Steven_0028
	//palDataMgr_Data_Access(edcBACKUPINPUT_AUTOSWITCH,edaREAD, &ucData);
	//palDataMgr_Data_Access(edcBACKUPINPUT_AUTOSWITCH,edaWRITE_RAM_ONLY_WITH_ACTION, &ucData);

    //Set CeilingMount && RearProjection
	palDataMgr_Data_Access(edcPROJECTION_MDOE,edaREAD, &ucData);                        //ZU860_Doulas_0089
	palDataMgr_Data_Access(edcPROJECTION_MDOE,edaWRITE_RAM_ONLY_WITH_ACTION, &ucData);  //ZU860_Doulas_0089

	//Init Auto Source Reync
	palDataMgr_Data_Access(edcAUTO_SOURCE_RESYNC, edaREAD, &ucData);						//G100_Doulas_0006
    palDataMgr_Data_Access(edcAUTO_SOURCE_RESYNC, edaWRITE_THROUGH_WITH_ACTION, &ucData);	//G100_Doulas_0006

	#ifdef SCALER_C821_C789
	    #ifdef Low_Latency_All
        halC789Ctrl_Init(m_sPalDataPathInfo.ePanelTimingId, halScaler_Is_LowLatencyMode_On()); //A70LV_Larry_0124   //H2PF_Simon_0187
	    #else
	    halC789Ctrl_Init(m_sPalDataPathInfo.ePanelTimingId); //A70LV_Larry_0124
	    #endif	/*Low_Latency_All*/
	#endif

//#if(CURRENT_PLATFORM == PLATFORM_4K_C790 || CURRENT_PLATFORM == PLATFORM_2K_C787) //A70LV_Larry_0051 mask

//#endif

    //init warp
    palGeo_PowerNormal(m_sPalDataPathInfo.ePanelTimingId);

	palLANProcSendToLAN(edcCOLOR_WHEEL_SPEED);
    //palDataMgr_Data_Access(edcSPLASH_STARTUP, edaREAD, (PUINT8)&ucIsLogoOn);    //A70LV_Doulas_0148//A70LV_Doulas_0023


    if(rcSUCCESS == halFrontEndCtrl_INPUT_SOURCE_DETECT_Get(&uiInput)) //H30K_Doulas_0073
    {
        palDataMgr_Data_Access(edcSOURCE_INFO, edaWRITE_RAM_ONLY_NO_ACTION, &uiInput);
        palDataPath_InputDetectSet(uiInput);
        palDataMgr_OPDInputPlugInState(eOPD_INPUT_DETECT_LOG);
        palDataPath_LastInputDetectSet(uiInput);
    }


    #ifdef CURSOR_FIXTURE
    if(PANEL_2D_OUTPUT == ePANEL_ID_WUXGA_240HZ)
    {
        LOG_MSG(db_ALWAYS, "Panel 2K240\r\n");
        palSystem_ActuatorEnable_Set(FALSE);
        palFormatterMgr_XPR_Set(eXPR_OFF_4WAY_240);
        halFormatter_XPR_DataSwap_Set(0);
        palFormatterMgr_AutoLockResync();
        MS_SLEEP(2000);
    }
    #endif

    halMCU_SystemReadySet();

    #if 1
    palDataPath_GotoState(eDATA_PATH_STATE_SPLASH_AT_STARTUP);
    #else
    palDataPath_GotoState(eDATA_PATH_STATE_SCALER_PORT_CONFIG);
    #endif
}

static void palDataPath_TestPatternProc(void)
{
    if(palImgMgr_SYNC_Lock_Get(eSOURCE_WINDOW_MAIN) != eHAL_SCALER_EXEC_CODE_PASS)   //ZU860_Doulas_0124 check lost/change input
    {
        if(ucTestPatternSourceLostCount < 4)
        {
            UINT8 ucVal = 0;
            palDataMgr_Data_Access(edcLOGO_CHANGE, edaREAD, &ucVal);
            if(ucVal != eCM_CHANGE_LOGO_CAPTURED)
            {
                halScaler_SourceOutputOff(eSOURCE_WINDOW_MAIN);
            }
            ucTestPatternSourceLostCount++;

    		//palEnvironment_LightSourceBlankingOn_Set(TRUE,2);	//A70Gen2_Doulas_0036 remove//A70Gen2_Doulas_0021
    		halScaler_ForcedSyncResetDisable_Set();             //A70Gen2_Doulas_0021 //A70LV_Doulas_0308 frame lock disable
    		if(palDataPath_Last3DEnable_Get() == ets_ON)		//A70Gen2_Doulas_0021 Modify image error on 3D mode
    		{
    			//halFormatter_3D_FormatSet(eCM_3D_LR_REFERENCE_1ST_FRAME); 	//A70Gen2_Doulas_0023 remove
    			//halFormatter_ChannelSourceSet(DISP_EXTERNAL_SOURCE);		//A70Gen2_Doulas_0023 remove
    			palDataPath_Set3D_FirstFrameMode();  //A70Gen2_Doulas_0023
    		}
        }
    }
}
static void palDataPath_StateMachineProc(UINT16 uiMsgID)
{
    //BOOL bSyncLock; //A70LV_Doulas_0007
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_FAIL; //A70LV_Doulas_0009 Add
    UINT8 ucVal = 0;

    if((palSystem_PowerStateGet() == ePOWER_STATE_STANDBY) || (palSystem_PowerStateGet() == ePOWER_STATE_UPGRADE)
        || (m_sPalDataPathInfo.bTaskPause == TRUE))
    {
        return;
    }

    if(palDataMgr_CurTestPatternGet() != eTID_OFF) //test pattern on
    {
        if(m_sPalDataPathInfo.eDataPathState == eDATA_PATH_STATE_MONITOR_SOURCE)
        {
            palDataPath_TestPatternProc();

            if(palImgMgr_SYNC_Lock_Get(eSOURCE_WINDOW_MAIN) != eHAL_SCALER_EXEC_CODE_PASS)  //HICC2_Doulas_0145
            {
                LOG_MSG(db_APP_DATAPATH, "HSG Pattern main LOSTLOCK\r\n");
                palDataPath_GotoState(eDATA_PATH_STATE_LOOK_FOR_SYNCS);
            }
        }
        return;
    }

    //LOG_MSG(db_APP_DATAPATH ,"(%s)<%d %d>\n", __FUNCTION__, m_sPalDataPathInfo.eDataPathState, m_sPalDataPathInfo.lPollPeriod);

    switch(m_sPalDataPathInfo.eDataPathState)
    {
        case eDATA_PATH_STATE_SPLASH_AT_STARTUP:
        {
            UINT8 ucValue = 0;
			UINT8 ucPIPEable= 0;

            if((m_ucSmoothOnReadyCheck == 1) && (m_uiFirstLanguageCount < (DATAPATH_STATE_FIRST_LANGAGE_MEOUT+1)))//HICC2_Julie_0037
            {
                m_uiFirstLanguageCount++;
            }
            #if 0 //defined(CUSTOM_BARCO) //###
            if((palDataMgr_PIN_Protect_Checking() == ePASSWORD_PROTECT_UNLOCK) && (GuiCb.fpGui_IsFirstStartupMenuCb() == FALSE))
            #else
            palDataMgr_Data_Access(edcFIRSTSTARTUPFLAG, edaREAD, &ucValue);
            if(((palDataMgr_PIN_Protect_Checking() == ePASSWORD_PROTECT_UNLOCK) && (ucValue == ets_OFF))
            #endif
                || (m_uiFirstLanguageCount >= DATAPATH_STATE_FIRST_LANGAGE_MEOUT))
            {

			#ifdef CUSTOM_OPTOMA ////H30K_David_0050
                if(m_ucSmoothOnReadyCheck == 1)
                {
					if(ucValue == TRUE && palDataMgr_PIN_Protect_Checking() == ePASSWORD_PROTECT_UNLOCK) // (only first langage menu) and (first langage menu  / security menu after security menu < 60sec)
                	{
                    palDataMgr_UI_EventSend(edcUI_EVENT_OSD_EXIT, TRUE, NULL);//H30K_David_0047
                    	palDataPath_GotoState(eDATA_PATH_STATE_SCALER_PORT_CONFIG);
                    	//LOG_MSG(db_ALWAYS , "ok1(1:%d, 2:%d 3:%d)\r\n", palDataMgr_PIN_Protect_Checking(),ucValue ,m_uiFirstLanguageCount);
                	}
					else if(ucValue == TRUE && palDataMgr_PIN_Protect_Checking() == ePASSWORD_PROTECT_LOCKED) // (first langage menu / security menu > 60sec)
                	{
                    	m_uiFirstLanguageCount=0;
                    	//LOG_MSG(db_ALWAYS , "ok2(1:%d, 2:%d 3:%d)\r\n", palDataMgr_PIN_Protect_Checking(),ucValue ,m_uiFirstLanguageCount);
                	}
					else // only security menu
					{
					  palDataMgr_Data_Access(edcPIP_PBP_ENABLE, edaREAD, &ucPIPEable);
					  //LOG_MSG(db_ALWAYS , "ok3(:%d, :%d :%d :%d)\r\n", palDataMgr_PIN_Protect_Checking(),ucPIPEable,ucValue ,m_uiFirstLanguageCount);
                    LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d)eDATA_PATH_STATE_SPLASH_AT_STARTUP GoTo  eDATA_PATH_STATE_SCALER_PORT_CONFIG \r\n", __FUNCTION__, __LINE__);
					  if(ucPIPEable!=eCM_SCREEN_MODE_OFF&& palDataMgr_PIN_Protect_Checking() == ePASSWORD_PROTECT_LOCKED)//H30K_David_0052
					  {
                        m_uiFirstLanguageCount=0;
					  }
					  else
					  {
                    palDataPath_GotoState(eDATA_PATH_STATE_SCALER_PORT_CONFIG);
					  }
					}
                }
			#else
 				if(m_ucSmoothOnReadyCheck == 1)
                {
                    ucValue = ets_OFF;
                    palDataMgr_Data_Access(edcFIRSTSTARTUPFLAG, edaWRITE_THROUGH_NO_ACTION, &ucValue);
                    LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d)eDATA_PATH_STATE_SPLASH_AT_STARTUP GoTo  eDATA_PATH_STATE_SCALER_PORT_CONFIG \r\n", __FUNCTION__, __LINE__);
                    palDataPath_GotoState(eDATA_PATH_STATE_SCALER_PORT_CONFIG);
                }
			#endif
            }
            else
            {
                palDataMgr_Data_Access(edcUST_FIRSTSTARTUP, edaREAD, &ucValue);
                if((palDataMgr_PIN_Protect_Checking() == ePASSWORD_PROTECT_LOCKED) || (ucValue == ets_ON))
                {
                    palDataPath_ResetTimeInState();
                }
            }

            switch(uiMsgID)
            {
                case eDATA_PATH_MSG_RESYNC:     //A70LV_Doulas_0004
                    LOG_MSG(db_APP_DATAPATH, " eDATA_PATH_MSG_RESYNC --Main\r\n");
                    palDataPath_ResetTimeInState();
                break;
            }
        }
        break;

        case eDATA_PATH_STATE_SCALER_PORT_CONFIG:
            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d): eDATA_PATH_STATE_SCALER_PORT_CONFIG, uiMsgID: %d\r\n", __FUNCTION__, __LINE__,uiMsgID);

            switch(uiMsgID) //A70LV_Doulas_0004
            {
                default:
                if(halScaler_PIP_PBP_Enable_Get())  //G100_Owen_0012  //A70LV_Doulas_0142 Add
                {
                    halScaler_Resync_Init(eSOURCE_WINDOW_MAIN);
                    uPALIMGMGR_INFO uInfo = {.sConfig_NoSignalOutput.eWindow = eSOURCE_WINDOW_MAIN,
                                             .sConfig_NoSignalOutput.ucDisplayOutput = FALSE};
                    palImgMgr_Config_NoSignalOutput(&uInfo);  //A35G2_CDS_Simon_0017
                }

                if((m_sPalDataPathInfo.ePanelTimingId == ePANEL_ID_1080P_120HZ) ||
                   (m_sPalDataPathInfo.ePanelTimingId == ePANEL_ID_WUXGA_120HZ))    //A70LV_Doulas_0377 Modify//A70LV_Doulas_0154
                {
                    palDataPath_ResyncSub();
                }

                palDataPath_GotoState(eDATA_PATH_STATE_BEGIN_SCAN);     //A70LV_Doulas_0004
                break;

                case eDATA_PATH_MSG_PANEL_CHANGE:   //A70LV_Doulas_0154
                    LOG_MSG(db_APP_DATAPATH, " eDATA_PATH_MSG_PANEL_CHANGE\r\n");
                    halScaler_DisplaySFG(0, 5, TRUE);
                    if(halScaler_Panel_Set(0,m_sPalDataPathInfo.ePanelTimingId) == eHAL_SCALER_EXEC_CODE_PASS)
                    {
                        palDataPath_GotoState(eDATA_PATH_STATE_BEGIN_SCAN);
                    }
                    else
                    {
                        halScaler_DisplaySFG(0, 5, FALSE);
                    }
                    break;

                case eDATA_PATH_MSG_RESYNC:     //A70LV_Doulas_0004
                    LOG_MSG(db_APP_DATAPATH, " eDATA_PATH_MSG_RESYNC\r\n");
                    palDataPath_GotoState(eDATA_PATH_STATE_SCALER_PORT_CONFIG);
                    break;
            }

            #if (ENABLE_COLOR_UNIFORMITY == TRUE)   //G100_Simon_0090
            //palDataPath_Color_Uniformity_PictureMode_Check(); //A70LV_John_0126 Color Uniformity is only enabled in blending mode // A35G2_CDS_Coda_0028
            #endif

            break;

        case eDATA_PATH_STATE_SUSPENDED:
            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d)%d : eDATA_PATH_STATE_SUSPENDED\r\n", __FUNCTION__, __LINE__,uiMsgID);
            switch(uiMsgID) //A70LV_Larry_0111
            {
                case eDATA_PATH_MSG_RESYNC:
                    LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) eDATA_PATH_MSG_RESYNC\r\n", __FUNCTION__, __LINE__);
                    palDataPath_GotoState(eDATA_PATH_STATE_SCALER_PORT_CONFIG);
                break;
            }

            break;

        case eDATA_PATH_STATE_TPG_DISPLAYED:
        //    LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d)%d : eDATA_PATH_STATE_TPG_DISPLAYED\r\n", __FUNCTION__, __LINE__,uiMsgID);    //A70LV_Doulas_0035

            /* an external state change request is the only way to exit suspended state */
            switch(uiMsgID)     //A70LV_Doulas_0266 Add
            {
                #ifdef CUSTOM_CHRISTIE
                case eDATA_PATH_MSG_RESYNC:     //A70LV_Doulas_0004
                    LOG_MSG(db_APP_DATAPATH, " eDATA_PATH_MSG_RESYNC\r\n");
                    palDataPath_GotoState(eDATA_PATH_STATE_SCALER_PORT_CONFIG);
                break;
                #endif

                default:
                {
                    #if 0
                    INT32 iSetting = 0;
                    if(GuiCb.fpGui_HSG_TestPatternEnableGetCb())
                    {

                    }
                    else
                    {
                        palDataMgr_Data_Access(edcOSDTEST_PATTERN, edaREAD, &iSetting);
                        if(iSetting == eCM_TEST_PATTERN_OFF)
                        {
                            palDataPath_GotoState(eDATA_PATH_STATE_LOOK_FOR_SYNCS);         //A70LV_Doulas_0293 modify
                        }
                    }
                    #endif

                    eResult = palImgMgr_SYNC_Lock_Get(eSOURCE_WINDOW_MAIN);       //ZU860_Doulas_0124
                    if(eResult != eHAL_SCALER_EXEC_CODE_PASS)   //ZU860_Doulas_0124 check lost/change input
                    {
                        if(ucTestPatternSourceLostCount < 4)
                        {
                            UINT8 ucVal = 0;
                            palDataMgr_Data_Access(edcLOGO_CHANGE, edaREAD, &ucVal);
                            if(ucVal != eCM_CHANGE_LOGO_CAPTURED)
                            {
                                halScaler_SourceOutputOff(eSOURCE_WINDOW_MAIN);
                            }
                            ucTestPatternSourceLostCount++;

							//palEnvironment_LightSourceBlankingOn_Set(TRUE,2);	//A70Gen2_Doulas_0036 remove//A70Gen2_Doulas_0021
            				halScaler_ForcedSyncResetDisable_Set();             //A70Gen2_Doulas_0021 //A70LV_Doulas_0308 frame lock disable
            				if(palDataPath_Last3DEnable_Get() == ets_ON)		//A70Gen2_Doulas_0021 Modify image error on 3D mode
							{
								//halFormatter_3D_FormatSet(eCM_3D_LR_REFERENCE_1ST_FRAME); 	//A70Gen2_Doulas_0023 remove
								//halFormatter_ChannelSourceSet(DISP_EXTERNAL_SOURCE);		//A70Gen2_Doulas_0023 remove
								palDataPath_Set3D_FirstFrameMode();  //A70Gen2_Doulas_0023
							}
                        }
                    }
                }
                break;

            }


            break;

        case eDATA_PATH_STATE_BEGIN_SCAN:
         //   LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d)%d : eDATA_PATH_STATE_BEGIN_SCAN --\r\n", __FUNCTION__, __LINE__,uiMsgID);

            switch(uiMsgID)
            {
                case eDATA_PATH_MSG_FLUSHED:
                    LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) eDATA_PATH_MSG_FLUSHED\r\n", __FUNCTION__, __LINE__);
                    palDataPath_GotoState(eDATA_PATH_STATE_LOOK_FOR_SYNCS);
                    break;

                case eDATA_PATH_MSG_RESYNC:     //A70LV_Doulas_0004
                    LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) eDATA_PATH_MSG_RESYNC\r\n", __FUNCTION__, __LINE__);
					palDataPath_GotoState(eDATA_PATH_STATE_BEGIN_SCAN);
				    break;

                case eDATA_PATH_MSG_PANEL_CHANGE:   //A70LV_Doulas_0005
                    LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) eDATA_PATH_MSG_PANEL_CHANGE\r\n", __FUNCTION__, __LINE__);
                    halScaler_DisplaySFG(0, 5, TRUE);
                    if(halScaler_Panel_Set(0,m_sPalDataPathInfo.ePanelTimingId) == eHAL_SCALER_EXEC_CODE_PASS)
                    {
                        palDataPath_GotoState(eDATA_PATH_STATE_BEGIN_SCAN);
                    }
                    else
                    {
                        halScaler_DisplaySFG(0, 5, FALSE);
                    }
                    break;

                case eDATA_PATH_MSG_NONE:   //A70LV_Doulas_0007
                    {
                        //A70LV_Doulas_0076 modify
                        #if 1
                        UINT8 ucVal;
                        eRESULT eResult2 = rcINVALID;
                        eRESULT eResult3 = rcINVALID;
                        eSOURCE_STATE ucSourceState = eSOURCE_STATE_SHOW_SOURCE_RESOLUTION;     //A70LV_Doulas_0331 Add
                        UINT8 ucInputKeyFunction = 0;                                           //A70LV_Doulas_0331 Add

                        UINT8 ucBurnInEnable = 0;   //A70LV_Doulas_0106
                        UINT8 ucAutoFocus = 0;
						UINT8 ucEOTF = 0;

                        palDataMgr_Data_Access(edcBURNIN_ENABLE, edaREAD, &ucBurnInEnable);
                        palDataMgr_Data_Access(edcAUTO_FOCUS, edaREAD, &ucAutoFocus);
                        if((palDataMgr_PIN_Protect_Checking() == ePASSWORD_PROTECT_LOCKED) && (ucBurnInEnable == ets_OFF))   //A70LV_Doulas_0107 Modify
                        {
                            break;
                        }

                        if(palIllumination_FirstLightSourceOnGet() == FALSE)    //A70LV_Doulas_0212 wait Light on
                        {
                            break;
                        }

                    #if 0 //###
                        if(GuiCb.fpGui_IsFirstStartupMenuCb() == TRUE)
                        {
                            break;
                        }
                    #endif

                        uc3D_Panel_Timer++;  //A70LV_Doulas_0154
                        if(uc3D_Panel_Timer >= 45)//30)  //4.5 sec         //A70LV_Doulas_0235 modify
                        {
                            UINT8  ucLogoChangeItem = 0;  //G100_Coda_00101
                            palDataMgr_Data_Access(edcLOGO_CHANGE, edaREAD, &ucLogoChangeItem);

                            if(ucLogoChangeItem == eCM_CHANGE_LOGO_CAPTURED)
                            {
                                palDataMgr_3D_Check();       																					//G100_Doulas_0064
                                if((palSystem_PanelID_Get() == ePANEL_ID_1080P_120HZ || palSystem_PanelID_Get() == ePANEL_ID_WUXGA_120HZ))    				//G100_Doulas_0064
                                {
                                    palSystem_ChangePanelID(PANEL_2D_OUTPUT, NULL);

                                    halScaler_Input_3D_Format_Set(eINPUT_3D_TYPE_OFF);      //A70LV_Doulas_0375 Add

                                    palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL);
                                }

                            }
                            #if 0   //ZU860_Doulas_0138 remove
                            BOOL  bDDP442x_3D_Get = 0;
                            UINT8 ucData;
                            uc3D_Panel_Timer = 0;
                            if(m_sPalDataPathInfo.ePanelTimingId == ePANEL_ID_1080P_120HZ)      //A70LV_Doulas_0216
                            {
                                halScaler_Input_3D_Format_Set(eINPUT_3D_TYPE_OFF);
                                #if defined (DMD_DHD)
                                palSystem_ChangePanelID(ePANEL_ID_1080P_60HZ);
                                #elif defined (DMD_WUXGA)
                                palSystem_ChangePanelID(ePANEL_ID_WUXGA_60HZ);
                                #else
                                palSystem_ChangePanelID(ePANEL_ID_1080P_60HZ);
                                #endif
                                halFormatter_AspectRatio_Get(&ucData);      //ZU860_Doulas_0119
                                if(ucData != DDP_ASPECT_NATIVE)             //ZU860_Doulas_0119
                                {
                                    ucData = DDP_ASPECT_NATIVE;
                                    halFormatter_AspectRatio_Set(&ucData);
                                }

                                palDataMgr_Data_Access(edcPICTURE_SETTINGS, edaREAD, &ucData);      //ZU860_Doulas_0076 Add
                                if((ucData == eCM_PICTURE_SETTINGS_3D)||(ucData == eCM_PICTURE_SETTINGS_2DHIGHSPEED))               //ZU860_Doulas_0076
                                {
                                    palDataPath_PictureModeChange(eCM_PICTURE_SETTINGS_BRIGHT);
                                }
                                halFormatter_3D_3DModeSet(FALSE);                   //ZU860_Doulas_0121
                                halScaler_Input_3D_Format_Set(eINPUT_3D_TYPE_OFF);  //ZU860_Doulas_0121
                            }
                            if(palDataMgr_Is_Formatter_Setting() == FALSE)  //A70LV_Doulas_0271 Add
                            {
                                halFormatter_3D_3DModeGet(&bDDP442x_3D_Get);
                                if(bDDP442x_3D_Get == TRUE)
                                {
                                    palDataMgr_Data_Access(edcPICTURE_SETTINGS, edaREAD, &ucData);      //ZU860_Doulas_0076 Add
                                    if((ucData == eCM_PICTURE_SETTINGS_3D)||(ucData == eCM_PICTURE_SETTINGS_2DHIGHSPEED))               //ZU860_Doulas_0076 Modify
                                    {
                                        halScaler_Input_3D_Format_Set(eINPUT_3D_TYPE_OFF);
                                        palDataPath_PictureModeChange(eCM_PICTURE_SETTINGS_BRIGHT);
                                    }
                                    else
                                    {
                                        halC789Ctrl_OutputEnableSet(0);     //A70LV_Doulas_0216
                                        halFormatter_3D_3DModeSet(FALSE);
                                        halScaler_Input_3D_Format_Set(eINPUT_3D_TYPE_OFF);      //A70LV_Doulas_0187 Add
                                        halScaler_FrameDelay_Set(FRAME_DELAY_DEFAULT_VALUE);
                                        //palDataMgr_Formatter_Parameter_Set();       //ZU860_Doulas_0054 remove//A70LV_Doulas_0160 update system mode and PWM ...
                                        halC789Ctrl_OutputEnableSet(1);     //A70LV_Doulas_0216
                                    }
                                }
                            }
                            //LOG_MSG(db_ALWAYS, "+++Timer+++\r\n");
                            #endif
                        }

                        if(m_sPalDataPathInfo.uiPanelChange)
                        {
                            m_sPalDataPathInfo.uiPanelChange--;//避免change panel當下，太早抓到source，觸發後續semaphore timeout

                            if(m_sPalDataPathInfo.uiPanelChange == 0)
                            {
                                UINT8 ucXPRErrorStatus = 0;
                                halFormatter_XPR_ErrorStatus_Get(&ucXPRErrorStatus);

                                LOG_MSG(db_APP_DATAPATH, "(%s, %d) XPRErrorStatus = %d\n", __FUNCTION__, __LINE__, ucXPRErrorStatus);

                                if(ucXPRErrorStatus)
                                {
                                    halFormatter_XPR_ErrorStatus_Set();
                                }
                            }
                            break;
                        }

                        // Bit 0 retrn Main Source Status
                        // Bit 2 ~ 4 retrn Crrent Main Source ID
                        // 00:HDMI1, 01:HDMI2, 02:DisplayPort, 03:HDBASET, 04:3GSDI/12GSDI,
                        eResult2 = halFrontEndCtrl_videoReady_Get(&ucVal);
                        LOG_MSG(db_HAL_RESERVED19, "(func:%s, line:%d)  videoReady :0x%02X (%d)\r\n", __FUNCTION__, __LINE__, ucVal, TMO_GetSysRunTime());

                        #ifdef H30_DEVELOP_TEMP_DEFINE
						#else
                        if(eResult2 == rcSUCCESS)        //A70LV_Doulas_0235
                        {
                            if(((ucVal>>2)&0x07) != (UINT8)CM2GUI(edcMAIN_INPUT, m_sSourceDesc.eConnector))
                            {
                                eResult2 = rcERROR;
                            }
                        }
						#endif

                        if((eResult2 == rcSUCCESS) && (ucVal & 0x01))   //A70LV_Doulas_0112 modify
                        {
                            UINT8 ucVideoFormat[eFE_MSG_VIDEO_FORMAT_MAIN_SZ] = {0};
                            UINT8 cFRCByPass = 0; //G100_Larry_0028

                            LOG_MSG(db_HAL_RESERVED19, "Main) videoFormat_Get\r\n");
                            eResult2 = halFrontEndCtrl_videoFormat_Get(ucVideoFormat);
                            if(eResult2 == rcSUCCESS)
                            {
                                m_FrontEndVideoFormat.u8VideoColorDepth = ucVideoFormat[0];
                                m_FrontEndVideoFormat.u8VideoColorSpace = ucVideoFormat[1];
                                m_FrontEndVideoFormat.u8VideoDynamicRange = ucVideoFormat[2];
                                m_FrontEndVideoFormat.u8VideoScanning =  ucVideoFormat[3];
                                m_FrontEndVideoFormat.u8VideoAVIInfoDet =  ucVideoFormat[4];
                                m_FrontEndVideoFormat.u8Video3DFormat =  ucVideoFormat[5];
                                m_FrontEndVideoFormat.u8VideoVGASyncType = ucVideoFormat[6];    //A70LV_Doulas_0109
                                m_FrontEndVideoFormat.u8VideoColorYUV    = ucVideoFormat[7];    //A70LV_Doulas_0109
                                m_FrontEndVideoFormat.u8VideoDownScaling = ucVideoFormat[8];    //A70LV_Doulas_0154
                                m_FrontEndVideoFormat.u8VideoDualPixelMode = ucVideoFormat[9];  //A70LV_Doulas_0154
                                m_FrontEndVideoFormat.u8VideoHDRType       = ucVideoFormat[10];         //ZU860_Doulas_0100
                                m_FrontEndVideoFormat.u8VideoExtentedColorimetry = ucVideoFormat[11];   //ZU860_Doulas_0100
                                LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d)  eDATA_PATH_STATE_BEGIN_SCAN (%d,%d,%d)(%d,%d,%d)(%d,%d)(%d,%d)(%d,%d)(%d)\r\n", __FUNCTION__, __LINE__,ucVideoFormat[0],ucVideoFormat[1],ucVideoFormat[2],ucVideoFormat[3],ucVideoFormat[4],ucVideoFormat[5]
                                                        ,ucVideoFormat[6],ucVideoFormat[7],ucVideoFormat[8],ucVideoFormat[9],ucVideoFormat[10],ucVideoFormat[11],m_sSourceDesc.eConnector);    //A70LV_Doulas_0154 modify //A70LV_Doulas_0109

								palLANProcSendToLAN(edcDATAPATH_HDR_INFO);
								palDataPath_FrontEndVideoInfo_Set(); //H30K_Doulas_0055
								palDataPath_Input3D_PanelSet();  //H30K_Doulas_0060
                                if(halScaler_PIP_PBP_Enable_Get())    //ZU860_Doulas_0104
                                {
                                    if(m_sSourceDesc.eConnector == eCM_SOURCE_VGA)
                                    {
                                        if(halScaler_IsVsync120or100Hz_Get(eSOURCE_WINDOW_MAIN))
                                        {
                                            palDataMgr_Data_Access(edcSHOW_MAINSOURCE_MESSAGES, edaREAD, &ucSourceState);
                                            if(ucSourceState != eSOURCE_STATE_SIGNAL_OUT_OF_RANGE)
                                            {
                                                halScaler_Input_3D_Format_Set(eINPUT_3D_TYPE_OFF);  //EK816U_626U_Doulas_0005
                                                ucSourceState = eSOURCE_STATE_SIGNAL_OUT_OF_RANGE;
                                                uPALIMGMGR_INFO uInfo = {.sConfig_NoSignalOutput.eWindow = eSOURCE_WINDOW_MAIN,
                                                                         .sConfig_NoSignalOutput.ucDisplayOutput = TRUE};
                                                palImgMgr_Config_NoSignalOutput(&uInfo);  //A35G2_CDS_Simon_0017
                                                palDataMgr_Data_Access(edcSHOW_MAINSOURCE_MESSAGES, edaWRITE_RAM_ONLY_NO_ACTION, &ucSourceState);
                                                palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL);
                                                m_sPalDataPathInfo.lPollPeriod = 1000;
                                                palDataPath_ResetTimeInState();
                                                halScaler_SourceOutputOff(eSOURCE_WINDOW_MAIN);     //ZU860_Doulas_0128
                                            }
                                            break;
                                        }
                                    }
                                    else
                                    {
                                        if(m_FrontEndVideoFormat.u8Video3DFormat || m_FrontEndVideoFormat.u8VideoDualPixelMode)
                                        {
                                            palDataMgr_Data_Access(edcSHOW_MAINSOURCE_MESSAGES, edaREAD, &ucSourceState);
                                            if(ucSourceState != eSOURCE_STATE_SIGNAL_OUT_OF_RANGE)
                                            {
                                                halScaler_Input_3D_Format_Set(eINPUT_3D_TYPE_OFF);  //EK816U_626U_Doulas_0005
                                                ucSourceState = eSOURCE_STATE_SIGNAL_OUT_OF_RANGE;
                                                palDataMgr_Data_Access(edcSHOW_MAINSOURCE_MESSAGES, edaWRITE_RAM_ONLY_NO_ACTION, &ucSourceState);
                                                palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL);
                                                m_sPalDataPathInfo.lPollPeriod = 1000;
                                                palDataPath_ResetTimeInState();
                                                //halScaler_SourceOutputOff(eSOURCE_WINDOW_MAIN);     //ZU860_Doulas_0128
                                                uPALIMGMGR_INFO uInfo = {.sConfig_NoSignalOutput.eWindow = eSOURCE_WINDOW_MAIN,
                                                                         .sConfig_NoSignalOutput.ucDisplayOutput = TRUE};
                                                palImgMgr_Config_NoSignalOutput(&uInfo);  //A35G2_CDS_Simon_0017
                                            }
                                            break;
                                        }
                                    }
                                }
                                else
                                {
                                    /*if(appDataPath_Is_4K_TP_FP_SBS()) //H30K_Doulas_0078 remove//H30K_Doulas_0055 4K FP/TP/SBS not support
                                    {
                                        palDataMgr_Data_Access(edcSHOW_MAINSOURCE_MESSAGES, edaREAD, &ucSourceState);
                                        if(ucSourceState != eSOURCE_STATE_SIGNAL_OUT_OF_RANGE)
                                        {
                                            //halScaler_Input_3D_Format_Set(eINPUT_3D_TYPE_OFF);
                                            ucSourceState = eSOURCE_STATE_SIGNAL_OUT_OF_RANGE;
                                            palDataMgr_Data_Access(edcSHOW_MAINSOURCE_MESSAGES, edaWRITE_RAM_ONLY_NO_ACTION, &ucSourceState);
                                            palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL);
                                            m_sPalDataPathInfo.lPollPeriod = 1000;
                                            palDataPath_ResetTimeInState();
                                            //halScaler_SourceOutputOff(eSOURCE_WINDOW_MAIN);
                                            uPALIMGMGR_INFO uInfo = {.sConfig_NoSignalOutput.eWindow = eSOURCE_WINDOW_MAIN,
                                                                     .sConfig_NoSignalOutput.ucDisplayOutput = TRUE};
                                            palImgMgr_Config_NoSignalOutput(&uInfo);
                                        }
                                        break;
                                    }*/
                                	#if 0	//A70Gen2_Doulas_0005 remove (need fornt end modify,但也只改一半56hz/70hz/72hz/75hz/85hz一樣破圖)
                                    UINT8 ucData;
                                    palDataMgr_Data_Access(edc3D_MODE, edaREAD, &ucData);
                                    if(ets_ON == ucData)
                                    {
                                        UINT8 ucValue;
                                        palDataMgr_Data_Access(edc3D_ENABLE, edaREAD, &ucValue);
                                        if(eCM_3D_FORMAT_FRAME_SEQUENTIAL == ucValue)
                                        {
                                            UINT8 ucVal[eFE_MSG_VIDEO_TIMING_MAIN_SZ];
                                            halFrontEndCtrl_Main_Timing_Get(ucVal);
                                            if(31 >= (ucVal[12]+(ucVal[13]<<8)))
                                            {
                                                palDataMgr_Data_Access(edcSHOW_MAINSOURCE_MESSAGES, edaREAD, &ucSourceState);
                                                if(ucSourceState != eSOURCE_STATE_SIGNAL_OUT_OF_RANGE)
                                                {
                                                    halScaler_Input_3D_Format_Set(eINPUT_3D_TYPE_OFF);  //EK816U_626U_Doulas_0005
                                                    ucSourceState = eSOURCE_STATE_SIGNAL_OUT_OF_RANGE;
                                                    palDataMgr_Data_Access(edcSHOW_MAINSOURCE_MESSAGES, edaWRITE_RAM_ONLY_NO_ACTION, &ucSourceState);
                                                    palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL);
                                                    m_sPalDataPathInfo.lPollPeriod = 1000;
                                                    palDataPath_ResetTimeInState();
                                                    //halScaler_SourceOutputOff(eSOURCE_WINDOW_MAIN);     //ZU860_Doulas_0128
                                                    palImageMgr_Config_NoSignalOutput(eSOURCE_WINDOW_MAIN, TRUE);  //A35G2_CDS_Simon_0017
                                                }
                                                break;
                                            }
                                        }
                                    }
									#endif
                                }
                                #if 0
								if(m_sSourceDesc.eConnector == eCM_SOURCE_DVI)	//G100_Doulas_0045 Add
								{
									if(halScaler_IsVsync120or100Hz_Get(eSOURCE_WINDOW_MAIN))  //not support 120/100hz
                                    {
                                        palDataMgr_Data_Access(edcSHOW_MAINSOURCE_MESSAGES, edaREAD, &ucSourceState);
                                        if(ucSourceState != eSOURCE_STATE_SIGNAL_OUT_OF_RANGE)
                                        {
                                            halScaler_Input_3D_Format_Set(eINPUT_3D_TYPE_OFF);
                                            ucSourceState = eSOURCE_STATE_SIGNAL_OUT_OF_RANGE;
                                            palImageMgr_Config_NoSignalOutput(eSOURCE_WINDOW_MAIN, TRUE);  //A35G2_CDS_Simon_0017
                                            palDataMgr_Data_Access(edcSHOW_MAINSOURCE_MESSAGES, edaWRITE_RAM_ONLY_NO_ACTION, &ucSourceState);
                                            palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL);
                                            m_sPalDataPathInfo.lPollPeriod = 1000;
                                            palDataPath_ResetTimeInState();
                                            UINT8 ucVal = 0;
                                            palDataMgr_Data_Access(edcLOGO_CHANGE, edaREAD, &ucVal);
                                            if(ucVal != eCM_CHANGE_LOGO_CAPTURED)
                                            {
                                                halScaler_SourceOutputOff(eSOURCE_WINDOW_MAIN);
                                            }
                                        }
                                        break;
                                    }
								}
                                #endif
                                //palDataPath_FrontEndVideoInfo_Set(); //H30K_Doulas_0055
#ifdef NOT_USING_SHUTTER_COVER_TRANSIENT
#else
                                if(halScaler_PIP_PBP_Enable_Get() && palDataPath_GetPanelChangeState())
                                {
                                    palDataMgr_Data_Access(edcPICTURE_MUTE, edaWRITE_RAM_ONLY_WITH_ACTION, &m_ucPanelChangeing);
                                }
#endif /* NOT_USING_SHUTTER_COVER_TRANSIENT */
                                palDataPath_GotoState(eDATA_PATH_STATE_LOOK_FOR_SYNCS);

                                uc3D_Panel_Timer = 0;  //A70LV_Doulas_0154
                                //palDataPath_Input3D_PanelSet();         //H30K_Doulas_0060//A70LV_Doulas_0159
                            }
							eResult3 = halFrontEndCtrl_HDR_EOTF_STREAM_Get(&ucEOTF);
							if(eResult3 == rcSUCCESS)
							{
								m_Video_EOTF_STREAM = ucEOTF;
								palLANProcSendToLAN(edcDATAPATH_HLG_INFO);
								LOG_MSG(db_APP_DATAPATH, "@@@@@@ m_Video_EOTF_STREAM = %d\r\n", m_Video_EOTF_STREAM);
							}
                        }
                        else        //ZU860_Doulas_0128 Add
                        {
                            palDataMgr_Data_Access(edcSHOW_MAINSOURCE_MESSAGES, edaREAD, &ucSourceState);
                            if(ucSourceState == eSOURCE_STATE_SIGNAL_OUT_OF_RANGE)
                            {
                                m_sSourceDesc.ucSourcelostCount = 0;  //A70LV_Doulas_0332
                                if(palDataMgr_DataCode_Control(edcINPUT_KEY) == eFUNC_CONTROL_ENABLE)
                                {
                                    palDataMgr_Data_Access(edcINPUT_KEY, edaREAD, &ucInputKeyFunction) ;
                                }
                                else
                                {
                                    ucInputKeyFunction = eSOURCE_KEY_CHANGE_SOURCE;
                                }

                                if(ucInputKeyFunction == eSOURCE_KEY_CHANGE_SOURCE_AUTO && ucSourceState != eSOURCE_STATE_SEARCHING)
                                {
                                    ucSourceState = eSOURCE_STATE_SEARCHING;
                                    palDataMgr_Data_Access(edcSHOW_MAINSOURCE_MESSAGES, edaWRITE_RAM_ONLY_NO_ACTION, &ucSourceState);
                                    palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL);
                                }
                                else if(ucSourceState != eSOURCE_STATE_CHECKING_FOR_SIGNAL)
                                {
                                    ucSourceState = eSOURCE_STATE_CHECKING_FOR_SIGNAL;
                                    palDataMgr_Data_Access(edcSHOW_MAINSOURCE_MESSAGES, edaWRITE_RAM_ONLY_NO_ACTION, &ucSourceState);
                                    palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL);
                                }
                            }
                        }
                        #else
                            palDataPath_GotoState(eDATA_PATH_STATE_LOOK_FOR_SYNCS);
                        #endif
                    }
                    break;

                default:        //A70LV_Doulas_0004

                    break;
            }

            break;

        case eDATA_PATH_STATE_LOOK_FOR_SYNCS:
            //LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d)%d : eDATA_PATH_STATE_LOOK_FOR_SYNCS --\r\n", __FUNCTION__, __LINE__,uiMsgID);

            //TODO
            //uiMsgID = (UINT16)eDATA_PATH_MSG_SYNCSDETECTED; //A70LV_Doulas_0004

            switch(uiMsgID)
            {
                case eDATA_PATH_MSG_SYNCSDETECTED:
                    LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) eDATA_PATH_MSG_SYNCSDETECTED\r\n", __FUNCTION__, __LINE__);
                    halScaler_DisplaySFG(0, 5, FALSE);
                    palDataPath_GotoState(eDATA_PATH_STATE_ATTEMPT_LOCK);
                    break;

                case eDATA_PATH_MSG_RESYNC:     //A70LV_Doulas_0004
                    LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) eDATA_PATH_MSG_RESYNC\r\n", __FUNCTION__, __LINE__);
                    palDataPath_GotoState(eDATA_PATH_STATE_BEGIN_SCAN);
                    break;

                case eDATA_PATH_MSG_PANEL_CHANGE:   //A70LV_Doulas_0005
                    LOG_MSG(db_APP_DATAPATH, " eDATA_PATH_MSG_PANEL_CHANGE\r\n");
                    halScaler_DisplaySFG(0, 5, TRUE);
                    if(halScaler_Panel_Set(0,m_sPalDataPathInfo.ePanelTimingId) == eHAL_SCALER_EXEC_CODE_PASS)
                    {
                        palDataPath_GotoState(eDATA_PATH_STATE_BEGIN_SCAN);
                    }
                    else
                    {
                        halScaler_DisplaySFG(0, 5, FALSE);
                    }
                    break;

                case eDATA_PATH_MSG_NONE:   //A70LV_Doulas_0007
                    {
                        //LOG_MSG(db_APP_DATAPATH, "\r\n");
#ifdef FIRST_CHKVIDRDY
                        // Bit 0 retrn Main Source Status
                        // Bit 2 ~ 4 retrn Crrent Main Source ID
                        // 00:HDMI1, 01:HDMI2, 02:DisplayPort, 03:HDBASET, 04:3GSDI/12GSDI,
                        halFrontEndCtrl_videoReady_Get(&ucVal);
                        if(!ucVal)
                        {
                            palDataPath_GotoState(eDATA_PATH_STATE_BEGIN_SCAN);
                            break;
                        }
#endif
                        uPALIMGMGR_INFO uInfo = {.sMeasureInput.ucCH = eSOURCE_WINDOW_MAIN};
                        if(palImgMgr_MeasureInput(&uInfo) == eHAL_SCALER_EXEC_CODE_PASS)     //A70LV_Doulas_0031 modify
                        {
                            if(palGeo_ApLinkFlag_Get() == TRUE)                                       //ZU860_Doulas_0138
                            {
                                if(palDataPath_IsErrorInputGetForTwistOn())
                                {
                                    palDataPath_GotoState(eDATA_PATH_STATE_BEGIN_SCAN);
                                    if(palDataPath_AlreadyShowDisconnectTwistMsg_Flag_Get() == ets_OFF)// HICC2_Bruce_0008 //HICC2_Doulas_0053 remove
                                    {
                                        palDataPath_AlreadyShowDisconnectTwistMsg_Flag_Set(ets_ON);// HICC2_Bruce_0008 //HICC2_Doulas_0053 remove
                                        palDataMgr_UI_EventSend(edcUI_EVENT_DISCONNECT_TWIST_MSG, TRUE, NULL);
                                    }
                                    break;
                                }
                            }
                            else
                            {
                                if(halScaler_PIP_PBP_Enable_Get() == ets_OFF)
                                {
                                    palDataPath_Input3D_PanelSet2();
                                }
                            }

                            palDataMgr_DisplayModeChecking();         //ZU860_Doulas_0045
                            halScaler_DisplaySFG(0, 5, TRUE);
                            #if 0 //H30K_Doulas_0001 remove
                            if((halScaler_Input_3D_Format_Config_Get() != eINPUT_3D_TYPE_OFF) &&
                               (palImgMgr_SYNC_Lock_Get(eSOURCE_WINDOW_SUB) != eHAL_SCALER_EXEC_CODE_PASS))
                            {
                                palDataPath_GotoState(eDATA_PATH_STATE_BEGIN_SCAN);
                            }
                            else
                            #endif
                            {
                                palDataPath_GotoState(eDATA_PATH_STATE_ATTEMPT_LOCK);
                            }
                            if(m_sSourceDesc.eConnector == eCM_SOURCE_VGA)    //A70LV_Doulas_0112
                            {
                                UINT16 uiVal;
                                UINT32 udVal;
                                UINT16 uiADCGain[3];
                                UINT16 uiADCOffset[3];
                                UINT16 uiH_Active;  //A70LV_Doulas_0200
                                UINT16 uiV_Active;  //A70LV_Doulas_0200
                                UINT16 uiV_Freq;    //A70LV_Doulas_0200

                                halScaler_VGA_H_Total_Get(eSOURCE_WINDOW_MAIN,&uiVal);
                                halFrontEndCtrl_VGA_H_Total_Set(&uiVal);
                                udVal = (UINT32)uiVal * halScaler_uiHFreq_Get(eSOURCE_WINDOW_MAIN);     //A70LV_Doulas_0115
                                halFrontEndCtrl_VGA_Pixel_Clock_Set(udVal);     //A70LV_Doulas_0115

                                if(m_FrontEndVideoFormat.u8VideoVGASyncType == eVGA_SYNC_TYPE_SOG)    //A70LV_Doulas_0124 Set ADC gain/offset
                                {
                                    palDataMgr_Data_Access(edcADC_YUV_GAIN, edaREAD, uiADCGain);
                                    palDataMgr_Data_Access(edcADC_YUV_OFFSET, edaREAD, uiADCOffset);
                                }
                                else
                                {
                                    palDataMgr_Data_Access(edcADC_RGB_GAIN, edaREAD, uiADCGain);
                                    palDataMgr_Data_Access(edcADC_RGB_OFFSET, edaREAD, uiADCOffset);
                                }
                                LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) ADC G(%d,%d,%d) O(%d,%d,%d)\r\n",__FUNCTION__, __LINE__, uiADCGain[0],uiADCGain[1],uiADCGain[2],
                                                            uiADCOffset[0],uiADCOffset[1],uiADCOffset[2]);
                                halFrontEndCtrl_VGA_Gain_Set(uiADCGain[0],uiADCGain[1],uiADCGain[2]);
                                halFrontEndCtrl_VGA_Offset_Set(uiADCOffset[0],uiADCOffset[1],uiADCOffset[2]);

                                uiH_Active = halScaler_Input_H_Active_Get(eSOURCE_WINDOW_MAIN);      //A70LV_Doulas_0200
                                uiV_Active = halScaler_Input_V_Active_Get(eSOURCE_WINDOW_MAIN);
                                halScaler_InputVertRefresh2_Get(eSOURCE_WINDOW_MAIN,&uiV_Freq);
                                halFrontEndCtrl_VGA_TIMING_Set(uiH_Active,uiV_Active,uiV_Freq);
                            }
                        }
                        else if(palDataPath_TimeElapsedInState() >= (m_psSysConfiguration->Datapath.uiSplashAtStartupTimeout * 1000)) //2s
                        {
                            LOG_MSG(db_APP_DATAPATH, "Main Window LOOK_FOR_SYNCS TimeOut ! (%d)\r\n",palDataPath_TimeElapsedInState());
                            palDataPath_GotoState(eDATA_PATH_STATE_BEGIN_SCAN);
                        }
                    }
                    break;

                default:


                   /* if(palDataPath_TimeElapsedInState() >= 3000)  //A70LV_Doulas_0004
                    {
                        palDataPath_GotoState(eDATA_PATH_STATE_BEGIN_SCAN);
                    }*/

                    break;
            }

            break;

        case eDATA_PATH_STATE_ATTEMPT_LOCK:
            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d)%d : eDATA_PATH_STATE_ATTEMPT_LOCK --\r\n", __FUNCTION__, __LINE__,uiMsgID);

            switch(uiMsgID)
            {
                case eDATA_PATH_MSG_LOCKED:
                    LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) eDATA_PATH_MSG_LOCKED\r\n", __FUNCTION__, __LINE__);
                    break;

                case eDATA_PATH_MSG_LOSTLOCK:
                	//ZU860_Clare_0052,add, >>>
                    LOG_MSG(db_APP_DATAPATH, " eDATA_PATH_MSG_LOSTLOCK11\r\n");
                    if(palDataMgr_PIN_Protect_Checking() == ePASSWORD_PROTECT_UNLOCK)
                    {
                        palDataPath_GotoState(eDATA_PATH_STATE_BEGIN_SCAN);
                    }
                    else
                    {
                        palDataPath_GotoState(eDATA_PATH_STATE_SPLASH_AT_STARTUP);
                    }
				break;
				//ZU860_Clare_0052, add, <<<
                case eDATA_PATH_MSG_REACQUIRE:
                    LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) eDATA_PATH_MSG_LOSTLOCK11\r\n", __FUNCTION__, __LINE__);
                    palDataPath_GotoState(eDATA_PATH_STATE_BEGIN_SCAN);
#if 0
                    //----- Roger Added 20061013  Start
                    /* cannot find source on the current connector, so cycle to next connector */
                    datapath_StopCurrentOperation();
                    datapath_CycleSource(&sourceDesc);
                    //----- Roger Added 20061013  End
#endif
                    break;

                case eDATA_PATH_MSG_RESYNC:     //A70LV_Doulas_0004
                    LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) eDATA_PATH_MSG_RESYNC\r\n", __FUNCTION__, __LINE__);
                    palDataPath_GotoState(eDATA_PATH_STATE_LOOK_FOR_SYNCS);
                    break;

                case eDATA_PATH_MSG_PANEL_CHANGE:   //A70LV_Doulas_0005
                    LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) eDATA_PATH_MSG_PANEL_CHANGE\r\n", __FUNCTION__, __LINE__);
                    halScaler_DisplaySFG(0, 5, TRUE);
                    if(halScaler_Panel_Set(0,m_sPalDataPathInfo.ePanelTimingId) == eHAL_SCALER_EXEC_CODE_PASS)
                    {
                        palDataPath_GotoState(eDATA_PATH_STATE_BEGIN_SCAN);
                    }
                    else
                    {
                        halScaler_DisplaySFG(0, 5, FALSE);
                    }
                    break;

                case eDATA_PATH_MSG_NONE:   //A70LV_Doulas_0007
                    LOG_MSG(db_APP_DATAPATH, "\r\n");
                    if(palImgMgr_SYNC_Lock_Get(eSOURCE_WINDOW_MAIN) == eHAL_SCALER_EXEC_CODE_PASS) //A70LV_Doulas_0009 modify
                    {
                        if(halScaler_PIP_PBP_Enable_Get())    //ZU860_Doulas_0104
                        {
                            if(m_sSourceDesc.eConnector == eCM_SOURCE_VGA)
                            {
                                if(halScaler_IsVsync120or100Hz_Get(eSOURCE_WINDOW_MAIN))
                                {
                                    palDataPath_GotoState(eDATA_PATH_STATE_BEGIN_SCAN);
                                    break;
                                }
                            }
                            else    //ZU860_Doulas_0130 Add
                            {
                                //LOG_MSG(db_APP_DATAPATH, "*** (%d)(%d,%d)\r\n",m_sSourceDesc.eConnector,m_FrontEndVideoFormat.u8Video3DFormat,m_FrontEndVideoFormat.u8VideoDualPixelMode );
                                if((m_FrontEndVideoFormat.u8Video3DFormat || m_FrontEndVideoFormat.u8VideoDualPixelMode) &&
                                   (m_sSourceDesc.eConnector != eCM_SOURCE_3GSDI))          //PIP on ,not show 3D
                                {
                                    palDataPath_GotoState(eDATA_PATH_STATE_BEGIN_SCAN);
                                    break;
                                }
                            }
                        }
                        /*else if(appDataPath_Is_4K_TP_FP_SBS()) //H30K_Doulas_0078 remove//H30K_Doulas_0055 4K FP/TP/SBS
                        {
                            palDataPath_GotoState(eDATA_PATH_STATE_BEGIN_SCAN);
                            break;
                        }*/

                        if(halScaler_AutoPhaseStart(0) == eHAL_SCALER_EXEC_CODE_PASS)
                        {
                            palEnvironment_LightSourceBlankingOn_Set(TRUE,2);   //ZU860_Doulas_0083
                            MS_SLEEP(120);   //A70LV_Doulas_0311
                            palDataPath_GotoState(eDATA_PATH_STATE_AUTO_PHASE);
                        }
                        else
                        {
                            #if 0    //A70LV_Doulas_0109
                            if(m_sPalDataPathInfo.ePanelTimingId == ePANEL_ID_1080P_120HZ)  //A70LV_Doulas_0098 modify CH1 jitter bug
                            {
                                if(palDataPath_IsSourceMonitorSub() ==  TRUE)
                                {
                                    palDataPath_GotoState(eDATA_PATH_STATE_MONITOR_SOURCE);
                                }
                            }
                            else
                            #endif
                            {
                                if(palDataPath_AutoCheckColorSpaceChange() == TRUE)
                                {
                                    LOG_MSG(db_APP_DATAPATH, " palDataPath_AutoCheckColorSpaceChange..\r\n");
                                    palDataPath_GotoState(eDATA_PATH_STATE_BEGIN_SCAN);  //H30K_Doulas_0065 Modify
                                }
                                else
                                {
                                    palDataPath_GotoState(eDATA_PATH_STATE_MONITOR_SOURCE);
                                }
                            }
                        }
                    }
                    else
                    {
                        palDataPath_GotoState(eDATA_PATH_STATE_BEGIN_SCAN);
                    }
                    break;

                case eDATA_PATH_MSG_RECONFIG_IMAGE:     //A70LV_Doulas_0050
                    LOG_MSG(db_APP_DATAPATH, " eDATA_PATH_MSG_RECONFIG_IMAGE\r\n");
                    halScaler_DisplaySFG(0, 5, TRUE);
                    palDataPath_GotoState(eDATA_PATH_STATE_ATTEMPT_LOCK);
                    break;

                default:

                    break;
            }

            #if (ENABLE_COLOR_UNIFORMITY == TRUE)   //G100_Simon_0090
            palDataPath_Color_Uniformity_PictureMode_Check();//A70LV_John_0126 Color Uniformity is only enabled in blending mode
            #endif

            break;

        case eDATA_PATH_STATE_AUTO_PHASE:   //A70LV_Doulas_0007 Add
            //LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d)%d : eDATA_PATH_STATE_AUTO_PHASE --", __FUNCTION__, __LINE__,uiMsgID);

            switch(uiMsgID)
            {
                case eDATA_PATH_MSG_SYNCSDETECTED:
                    LOG_MSG(db_APP_DATAPATH, " eDATA_PATH_MSG_SYNCSDETECTED\r\n");
                    halScaler_DisplaySFG(0, 5, FALSE);
                    palDataPath_GotoState(eDATA_PATH_STATE_ATTEMPT_LOCK);
                    break;

                case eDATA_PATH_MSG_RESYNC:
                    LOG_MSG(db_APP_DATAPATH, " eDATA_PATH_MSG_RESYNC\r\n");
                    palDataPath_GotoState(eDATA_PATH_STATE_BEGIN_SCAN);
                    break;

                case eDATA_PATH_MSG_PANEL_CHANGE:
                    LOG_MSG(db_APP_DATAPATH, " eDATA_PATH_MSG_PANEL_CHANGE\r\n");
                    halScaler_DisplaySFG(0, 5, TRUE);
                    if(halScaler_Panel_Set(0,m_sPalDataPathInfo.ePanelTimingId) == eHAL_SCALER_EXEC_CODE_PASS)
                    {
                        palDataPath_GotoState(eDATA_PATH_STATE_BEGIN_SCAN);
                    }
                    else
                    {
                        halScaler_DisplaySFG(0, 5, FALSE);
                    }
                    break;

                case eDATA_PATH_MSG_NONE:
                    if(halScaler_AutoPhase(0) == eHAL_SCALER_EXEC_CODE_PASS)
                    {
                        palDataPath_GotoState(eDATA_PATH_STATE_AUTO_POSITION);
                    }
                    break;

                case eDATA_PATH_MSG_LOSTLOCK:       //A70LV_Doulas_0032
                case eDATA_PATH_MSG_REACQUIRE:
                    LOG_MSG(db_APP_DATAPATH, " eDATA_PATH_MSG_LOSTLOCK12\r\n");
                    palDataPath_GotoState(eDATA_PATH_STATE_BEGIN_SCAN);
					break;

                case eDATA_PATH_MSG_RECONFIG_IMAGE:     //A70LV_Doulas_0050
                    LOG_MSG(db_APP_DATAPATH, " eDATA_PATH_MSG_RECONFIG_IMAGE\r\n");
                    halScaler_DisplaySFG(0, 5, TRUE);
                    palDataPath_GotoState(eDATA_PATH_STATE_ATTEMPT_LOCK);
                    break;

                default:

                    break;
            }

            break;

        case eDATA_PATH_STATE_AUTO_POSITION:   //A70LV_Doulas_0007 Add
            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d)%d : eDATA_PATH_STATE_AUTO_POSITION --", __FUNCTION__, __LINE__,uiMsgID);

            switch(uiMsgID)
            {
                case eDATA_PATH_MSG_SYNCSDETECTED:
                    LOG_MSG(db_APP_DATAPATH, " eDATA_PATH_MSG_SYNCSDETECTED\r\n");
                    halScaler_DisplaySFG(0, 5, FALSE);
                    palDataPath_GotoState(eDATA_PATH_STATE_ATTEMPT_LOCK);
                    break;

                case eDATA_PATH_MSG_RESYNC:
                    LOG_MSG(db_APP_DATAPATH, " eDATA_PATH_MSG_RESYNC\r\n");
                    palDataPath_GotoState(eDATA_PATH_STATE_BEGIN_SCAN);
                    break;

                case eDATA_PATH_MSG_PANEL_CHANGE:
                    LOG_MSG(db_APP_DATAPATH, " eDATA_PATH_MSG_PANEL_CHANGE\r\n");
                    halScaler_DisplaySFG(0, 5, TRUE);
                    if(halScaler_Panel_Set(0,m_sPalDataPathInfo.ePanelTimingId) == eHAL_SCALER_EXEC_CODE_PASS)
                    {
                        palDataPath_GotoState(eDATA_PATH_STATE_BEGIN_SCAN);
                    }
                    else
                    {
                        halScaler_DisplaySFG(0, 5, FALSE);
                    }
                    break;

                case eDATA_PATH_MSG_NONE:
                    {
                        LOG_MSG(db_APP_DATAPATH, "\r\n");
                        uPALIMGMGR_INFO uInfo = {.sMeasureInput.ucCH = eSOURCE_WINDOW_MAIN};
                        if(palImgMgr_MeasureInput(&uInfo) == eHAL_SCALER_EXEC_CODE_PASS)     //A70LV_Doulas_0031 modify
                        {
                            palDataPath_GotoState(eDATA_PATH_STATE_MONITOR_SOURCE);
                        }
                        else
                        {
                            palDataPath_GotoState(eDATA_PATH_STATE_BEGIN_SCAN);
                        }
                    }
                    break;

                case eDATA_PATH_MSG_LOSTLOCK:       //A70LV_Doulas_0032
                case eDATA_PATH_MSG_REACQUIRE:
                    LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) eDATA_PATH_MSG_LOSTLOCK13\r\n", __FUNCTION__, __LINE__);
                    palDataPath_GotoState(eDATA_PATH_STATE_BEGIN_SCAN);
					break;

                case eDATA_PATH_MSG_RECONFIG_IMAGE:     //A70LV_Doulas_0050
                    LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) eDATA_PATH_MSG_RECONFIG_IMAGE\r\n", __FUNCTION__, __LINE__);
                    halScaler_DisplaySFG(0, 5, TRUE);
                    palDataPath_GotoState(eDATA_PATH_STATE_ATTEMPT_LOCK);
                    break;

                default:

                    break;
            }

            break;

        case eDATA_PATH_STATE_MONITOR_SOURCE:

            //LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : eDATA_PATH_STATE_MONITOR_SOURCE --\r\n", __FUNCTION__, __LINE__);

            switch(uiMsgID)
            {
                case eDATA_PATH_MSG_NONE:   //A70LV_Doulas_0007
                    if(ucXPR_Reset_Timeout)
                    {
                        UINT8 ucMasterProjectMode, ucSlaveProjectMode;

                        ucXPR_Reset_Timeout--;
                        palFormatterMgr_Projection_Mode_Get(&ucMasterProjectMode);
			            palFormatterMgr_SlaveProjection_Mode_Get(&ucSlaveProjectMode);
                        //LOG_MSG(db_APP_DATAPATH, "@@DDP Master %d, Slave %d\r\n",ucMasterProjectMode, ucSlaveProjectMode);
                        if(ucMasterProjectMode == eCMD_Formatter_SolidField || ucSlaveProjectMode == eCMD_Formatter_SolidField)
                        {
                            LOG_MSG(db_APP_DATAPATH, "@@Reset XPR!\r\n");
                            uiResetXPR_Cnt++;
                            halFormatter_XPR_ErrorStatus_Set();
                            ucXPR_Reset_Timeout = 0;
                        }
                    }
                    m_sPalDataPathInfo.ucSyncLockCount++;      //A70LV_Doulas_0009 Modify monitor functions
                    #ifdef H30_DEVELOP_TEMP_DEFINE
					#else
                    if(m_sPalDataPathInfo.ucSyncLockCount > 5)     //A70LV_Doulas_0164 Modify
                    {
                        BOOL bFreezeEn = FALSE;
                        m_sPalDataPathInfo.ucSyncLockCount = 0;
                        uPALIMGMGR_INFO uInfo = {.sMeasureInput.ucCH = eSOURCE_WINDOW_MAIN};
                        eResult = palImgMgr_MeasureInput(&uInfo);      //measure all     //A70LV_Doulas_0031 modify

                        if(eResult != eHAL_SCALER_EXEC_CODE_PASS)   //A35G2_CDS_Simon_0059
                        {
                            if(palDataPath_BKInput_Status() == TRUE)  //backup input active
                            {
                                m_sPalDataPathInfo.ucBK_SyncLoseRetry++ ;
                                LOG_MSG(db_APP_DATAPATH, " eDATA_PATH_MSG_LOSTLOCK17 (%d)\r\n", m_sPalDataPathInfo.ucBK_SyncLoseRetry);
                            }
                            #if 0//def CUSTOM_BARCO    //A35G2_Simon_0096 //###
                            else
                            {
                                UINT8 ucData = 0;
                                palDataMgr_Data_Access(edcBACKUPINPUT_AUTOSWITCH, edaREAD, &ucData);
                                if(ucData == TRUE &&
                                   palDataPath_BKInput_Status() == FALSE &&
                                   halScaler_PIP_PBP_Enable_Get() == FALSE &&
                                   palDataPath_CurrentSrcIsQuickSwitchSrc() == TRUE)  //A35G2_Simon_0108
                                {
                                    GuiCb.fpGui_BackupInput_Inactive_MessageCb();
                                }
                            }
                            #endif

                            LOG_MSG(db_APP_DATAPATH, " eDATA_PATH_MSG_LOSTLOCK17\r\n");
                        }

                        halScaler_Freeze_Get(eSOURCE_WINDOW_MAIN,&bFreezeEn);     //A70LV_Doulas_0223
                        if(bFreezeEn == TRUE)
                        {
                            eResult = eHAL_SCALER_EXEC_CODE_PASS;   //keep monitor
                        }
                    }
                    else
                    #endif
                    {
                        eResult = palImgMgr_SYNC_Lock_Get(eSOURCE_WINDOW_MAIN);   //measure H/V Sync
                        if(eResult == eHAL_SCALER_EXEC_CODE_PASS)   //A70LV_Doulas_0283
                        {
                            if(palDataPath_AutoCheckColorSpaceChange() == TRUE)
                            {
                                uOPD_DATA uOPDData = {0};

                                uOPDData.sSnapshotLog.ulINFO_Frame_Change = ucInfoFrame_Change_flag;
                                utilOPD_EventSet(eOPD_INFOFRAME_CHANGE_LOG, &uOPDData);
                                LOG_MSG(db_APP_DATAPATH, " eDATA_PATH_MSG_LOSTLOCK15..\r\n");     //ZU860_Doulas_0136
                                eResult = eHAL_SCALER_EXEC_CODE_FAIL;
                            }
                        }
                        else
                        {
                            #if 0 //def CUSTOM_BARCO  //A35G2_Simon_0096 //###
                            else
                            {
                                UINT8 ucData = 0;
                                palDataMgr_Data_Access(edcBACKUPINPUT_AUTOSWITCH, edaREAD, &ucData);
                                if(ucData == TRUE &&
                                   palDataPath_BKInput_Status() == FALSE &&
                                   halScaler_PIP_PBP_Enable_Get() == FALSE &&
                                   palDataPath_CurrentSrcIsQuickSwitchSrc() == TRUE)  //A35G2_Simon_0108
                                {
                                    GuiCb.fpGui_BackupInput_Inactive_MessageCb();
                                }
                            }
                            #endif
                            LOG_MSG(db_APP_DATAPATH, " eDATA_PATH_MSG_LOSTLOCK16\r\n");     //ZU860_Doulas_0136
                        }
                    }

                    if(eResult == eHAL_SCALER_EXEC_CODE_PASS)
                    {
                        m_sPalDataPathInfo.ucBK_SyncLoseRetry = 0 ;   //A35G2_CDS_Simon_0059
                    }
                    else
                    {
                        //BOOL bFreezeEn = FALSE;
                        //halScaler_Freeze_Get(0,&bFreezeEn);
                        //if(bFreezeEn == FALSE)
                        if(palDataPath_BKInput_Status() == TRUE &&
                           m_sPalDataPathInfo.ucBK_SyncLoseRetry < BKINPUT_SYNC_LOSE_RETRY)     //A35G2_CDS_Simon_0059
                        {
                            m_sPalDataPathInfo.ucBK_SyncLoseRetry++ ;
                            LOG_MSG(db_APP_DATAPATH, " eDATA_PATH_MSG_LOSTLOCK skip\r\n");
                        }
                        else
                        {
                            palEnvironment_LightSourceBlankingOn_Set(TRUE,1);       //H30K_Doulas_0028//ZU860_Doulas_0083
                            palDataPath_FreezeChecking(eSOURCE_WINDOW_MAIN);   //A70LV_Doulas_0223 Freeze check
                            {
                                uiMsgID = eDATA_PATH_MSG_LOSTLOCK;
                                LOG_MSG(db_APP_DATAPATH, " eDATA_PATH_MSG_LOSTLOCK14\r\n");
                                halScaler_DisplaySFG(eSOURCE_WINDOW_MAIN, 5, TRUE);
                                #if defined(CUSTOM_CHRISTIE) || defined(CUSTOM_OPTOMA)
                                m_sPalDataPathInfo.eDataPathState = (eDATA_PATH_STATE)eDATA_PATH_STATE_BEGIN_SCAN;
                                #endif
                                palDataPath_GotoState(eDATA_PATH_STATE_BEGIN_SCAN);
                                palDataPath_RecordSourceLost(1);
                            }
                            m_sPalDataPathInfo.ucBK_SyncLoseRetry = 0 ;   //A35G2_CDS_Simon_0059
                        }
                    }
                    break;

                case eDATA_PATH_MSG_LOSTLOCK:
                    LOG_MSG(db_APP_DATAPATH, " eDATA_PATH_MSG_LOSTLOCK15_1\r\n");
                    palEnvironment_LightSourceBlankingOn_Set(TRUE,1);   //H30K_Doulas_0028//ZU860_Doulas_0083
                    halScaler_DisplaySFG(0, 5, TRUE);       //A70LV_Doulas_0003 modify
                    palDataPath_DisplaySplash(TRUE);
                    halScaler_FrontEndScanMode_Clear(eSOURCE_WINDOW_MAIN);       //A70LV_Doulas_0175
					palDataPath_GotoState(eDATA_PATH_STATE_BEGIN_SCAN);   //A70LV_Doulas_0004
					palDataPath_RecordSourceLost(2);
                    break;

                case eDATA_PATH_MSG_REACQUIRE:
                    LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) eDATA_PATH_MSG_REACQUIRE\r\n", __FUNCTION__, __LINE__);
                    palDataPath_DisplaySplash(FALSE);
					palDataPath_RecordSourceLost(3);
                    break;

                case eDATA_PATH_MSG_RESYNC:
                    LOG_MSG(db_APP_DATAPATH, " eDATA_PATH_MSG_RESYNC\r\n");
                    palDataPath_GotoState(eDATA_PATH_STATE_BEGIN_SCAN); //A70LV_Doulas_0004
					palDataPath_RecordSourceLost(4);
                    break;

                //----- Roger Added 20061221  Start
                case eDATA_PATH_MSG_SUSPENDSCAN:
                    LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) eDATA_PATH_MSG_SUSPENDSCAN\r\n", __FUNCTION__, __LINE__);
                    break;

                case eDATA_PATH_MSG_PANEL_CHANGE:   //A70LV_Doulas_0005
                    LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) eDATA_PATH_MSG_PANEL_CHANGE\r\n", __FUNCTION__, __LINE__);
                    halScaler_DisplaySFG(0, 5, TRUE);
                    if(halScaler_Panel_Set(0,m_sPalDataPathInfo.ePanelTimingId) == eHAL_SCALER_EXEC_CODE_PASS)
                    {
                        palDataPath_GotoState(eDATA_PATH_STATE_BEGIN_SCAN);
                    }
                    else
                    {
                        halScaler_DisplaySFG(0, 5, FALSE);
                    }
					palDataPath_RecordSourceLost(5);
                    break;

                case eDATA_PATH_MSG_RECONFIG_IMAGE:     //A70LV_Doulas_0009
                    LOG_MSG(db_APP_DATAPATH, " eDATA_PATH_MSG_RECONFIG_IMAGE\r\n");
                    halScaler_DisplaySFG(0, 5, TRUE);
                    palDataPath_GotoState(eDATA_PATH_STATE_ATTEMPT_LOCK);
					palDataPath_RecordSourceLost(6);
                    break;

            }

        default:
            {
                //appGui_Open_Freeze_Menu(); //A70LV_Doulas_0164 remove //A70LV_Larry_0111
                UINT8 ucValue = 0;
                palDataMgr_Data_Access(edcIMAGE_FREEZE, edaREAD, &ucValue);     //A70LV_Doulas_0167
                //if(ucValue && appGui_CurrentMenuIndexGet() != POWER_OFF_MENU_ICOUNT)// BruceLin#20201218, POWER_OFF_STANDALONEMENU_ICOUNT //ZU860_Energy_0016
                if(ucValue)     //Move to appgui.c
				{
                    palDataMgr_UI_EventSend(edcUI_EVENT_OPEN_FREEZE_MENU_MSG, TRUE, NULL);
                }
            }
            break;
    }
}


static void palDataPath_Callback(UINT16 uiMsgID, UINT32 ulParam1, UINT32 *pulParam2)
{
    if((palSystem_PowerStateGet() == ePOWER_STATE_STANDBY) || (palSystem_PowerStateGet() == ePOWER_STATE_UPGRADE)) //A70LV_Larry_0112
    {
        return;
    }

    switch(uiMsgID)
    {
        case eDATA_PATH_MSG_INIT:
            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : eDATA_PATH_MSG_INIT\r\n", __FUNCTION__, __LINE__);
            palDataPath_StartDisplay();
            //halC789Ctrl_BypassMode();   //A70LV_Doulas_0031
            //halC789Ctrl_Init(m_sPalDataPathInfo.ePanelTimingId);    //A70LV_Doulas_0036 //A70LV_Larry_0124 mask
            m_sPalDataPathInfo.bDataPathReady = TRUE;
            palSystem_SystemReadySet(1);
            break;

        case eDATA_PATH_MSG_GETREADY:
            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : eDATA_PATH_MSG_GETREADY\r\n", __FUNCTION__, __LINE__);
            *pulParam2 = (UINT32)m_sPalDataPathInfo.bDataPathReady;
            break;

        case eDATA_PATH_MSG_GOTOSTATE:
            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : eDATA_PATH_MSG_GOTOSTATE (%d) \r\n", __FUNCTION__, __LINE__, m_sPalDataPathInfo.eDataPathState);
            palDataPath_StopCurrentOperation();
            palDataPath_GotoState(ulParam1);
            break;

        case eDATA_PATH_MSG_SETCONNECTOR:
            {
                UINT8 ucOldMainSource = m_sSourceDesc.eConnector;
                LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : eDATA_PATH_MSG_SETCONNECTOR %d, %d\r\n", __FUNCTION__, __LINE__, ulParam1, *pulParam2);//A70LV_Doulas_0112
                m_sSourceDesc.eConnector = (eCM_SOURCE_ID)ulParam1;
                palImgMgr_SetInputSource(eSOURCE_WINDOW_MAIN, m_sSourceDesc.eConnector);
                if(halScaler_PIP_PBP_Enable_Get())
                {
                    if(palDataPath_Last3DEnable_Get()  && (palDataPath_IsSourceLock() == FALSE))
                    {
                        palDataPath_SetPanelChangeState(ets_ON);
                    }
                }
                else
                {
                    palDataPath_SetPanelChangeState(ets_OFF);
                }
                if(halScaler_PIP_PBP_Enable_Get() == TRUE && palDataPath_IsSourceLockSub())
                {
                    palDataPath_Reconfig_ImageSub();
                }

                if(CFG_CUSTOMER_ID != CUSTOMER_ID_OPTOMA)
                {
                    if(m_sSourceDesc.eConnector != ucOldMainSource)
                    {
                        UINT8 ucAspectRatio = 0;

                        palDataMgr_Data_Access(edcSIZE_PRESETS, edaREAD, (void*)&ucAspectRatio);
                        if(ucAspectRatio != eCM_SCALING_MODE_CUSTOM)
                        {
                            palDataMgr_ResetDigitalZoomShift();
                        }
                    }
                }

                palDataPath_GotoState(eDATA_PATH_STATE_SCALER_PORT_CONFIG);
            }
            break;


        case eDATA_PATH_MSG_FLUSHED:
            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : eDATA_PATH_MSG_FLUSHED\r\n", __FUNCTION__, __LINE__);

            if(ulParam1 != m_sPalDataPathInfo.ucFlushIndex)
            {
                return;
            }

            palDataPath_StateMachineProc(uiMsgID);
            break;

        case eDATA_PATH_MSG_SYNCSDETECTED:
            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : eDATA_PATH_MSG_SYNCSDETECTED\r\n", __FUNCTION__, __LINE__);
            palDataPath_StateMachineProc(uiMsgID);
            break;

        case eDATA_PATH_MSG_LOCKED:
            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : eDATA_PATH_MSG_LOCKED\r\n", __FUNCTION__, __LINE__);
            palDataPath_StateMachineProc(uiMsgID);
            break;

        case eDATA_PATH_MSG_REACQUIRE:
            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : eDATA_PATH_MSG_REACQUIRE\r\n", __FUNCTION__, __LINE__);
            palDataPath_StateMachineProc(uiMsgID);
            break;

        case eDATA_PATH_MSG_LOSTLOCK:
            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : eDATA_PATH_MSG_LOSTLOCK\r\n", __FUNCTION__, __LINE__);
            palDataPath_StateMachineProc(uiMsgID);
            break;

        case eDATA_PATH_MSG_SUSPENDSCAN:
            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : eDATA_PATH_MSG_SUSPENDSCAN\r\n", __FUNCTION__, __LINE__);
            palDataPath_StateMachineProc(uiMsgID);
            break;

        case eDATA_PATH_MSG_RESUMESCAN:
            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : eDATA_PATH_MSG_RESUMESCAN\r\n", __FUNCTION__, __LINE__);
            palDataPath_StateMachineProc(uiMsgID);
            break;

        case eDATA_PATH_MSG_RESYNC:
            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : eDATA_PATH_MSG_RESYNC (%d)\r\n", __FUNCTION__, __LINE__, m_sPalDataPathInfo.eDataPathState);
            halScaler_Resync_Init(0);    //A70LV_Doulas_0004
            halScaler_FrontEndScanMode_Clear(eSOURCE_WINDOW_MAIN);       //A70LV_Doulas_0175
            palDataPath_StateMachineProc(uiMsgID);

            UINT8  ucLogoChangeItem = 0;	 //G100_Coda_00101
            palDataMgr_Data_Access(edcLOGO_CHANGE, edaREAD, &ucLogoChangeItem);

            if(halScaler_PIP_PBP_Enable_Get())    //ZU860_Doulas_0138 Add
            {
                if(palSystem_PanelID_Get() == ePANEL_ID_1080P_120HZ || palSystem_PanelID_Get() == ePANEL_ID_WUXGA_120HZ)
                {
                    UINT8 ucSize = 0;
                    UINT8 ucLayout = 0;
                    UINT8 ucDisplayMode = 0;
                    //UINT8 ucData = 0;  //G100_Doulas_0064

                    palSystem_ChangePanelID(PANEL_2D_OUTPUT, NULL);

                    halScaler_FrameDelay_Set(FRAME_DELAY_DEFAULT_VALUE);
                    palDataMgr_3D_Check();
                    palDataMgr_Data_Access(edcMAIN_LAYOUT, edaREAD, &ucLayout) ;
                    palDataMgr_Data_Access(edcSIZE, edaREAD, &ucSize) ;
                    palDataMgr_UI_EventSend(edcUI_EVENT_DRAW_PIP_LAYOUT_POSITION, TRUE, NULL);
                    palDataMgr_UI_EventSend(edcUI_EVENT_DRAW_PIP_LAYOUT, TRUE, NULL);
                    if(palDataMgr_Data_Access(edcPICTURE_SETTINGS, edaREAD, &ucDisplayMode) == eEXEC_CODE_PASS)
                    {
                        if((ucDisplayMode == eCM_PICTURE_SETTINGS_2DHIGHSPEED) || (ucDisplayMode == eCM_PICTURE_SETTINGS_3D))
                        {
#if defined(CUSTOM_BARCO) || defined(CUSTOM_OPTOMA)
                            ucDisplayMode = eCM_PICTURE_SETTINGS_PRESENTATION;
#else
                            ucDisplayMode = eCM_PICTURE_SETTINGS_VIDEO;
#endif
                        }
                        palDataMgr_Data_Access(edcPICTURE_SETTINGS, edaWRITE_THROUGH_WITH_ACTION, &ucDisplayMode);
                        halScaler_Input_3D_Format_Set(eINPUT_3D_TYPE_OFF);
                    }
                    //halFormatter_AspectRatio_Get(&ucData);      //G100_Doulas_0064 remove//ZU860_Doulas_0119
                    //if(ucData != DDP_ASPECT_NATIVE)             //ZU860_Doulas_0119
                    //{
                    //    ucData = DDP_ASPECT_NATIVE;
                    //    halFormatter_AspectRatio_Set(&ucData);
                    //}
                }
                else
                {
                    if(palDataPath_Last3DEnable_Get() && (palDataPath_IsSourceLock() == FALSE))
                    {
                        UINT8  ucDisplayMode = 0;
                        palDataMgr_3D_Check();

                        if(palDataMgr_Data_Access(edcPICTURE_SETTINGS, edaREAD, &ucDisplayMode) == eEXEC_CODE_PASS)
                        {
                            if((ucDisplayMode == eCM_PICTURE_SETTINGS_2DHIGHSPEED) || (ucDisplayMode == eCM_PICTURE_SETTINGS_3D))
                            {
#if defined(CUSTOM_BARCO) || defined(CUSTOM_OPTOMA)
                                ucDisplayMode = eCM_PICTURE_SETTINGS_PRESENTATION;
#else
                                ucDisplayMode = eCM_PICTURE_SETTINGS_VIDEO;
#endif
                            }
                            palDataMgr_Data_Access(edcPICTURE_SETTINGS, edaWRITE_THROUGH_WITH_ACTION, &ucDisplayMode);
                            halScaler_Input_3D_Format_Set(eINPUT_3D_TYPE_OFF);
                        }
                    }
                }
            }
            else if(ucLogoChangeItem == eCM_CHANGE_LOGO_CAPTURED)	 //G100_Coda_00101
            {
                if(palSystem_PanelID_Get() == ePANEL_ID_1080P_120HZ || palSystem_PanelID_Get() == ePANEL_ID_WUXGA_120HZ)
                {
                //to do
                }
            }
            m_sSourceDesc.ucSourcelostCount = 0;  //ZU860_Doulas_0104
            break;

        case eDATA_PATH_MSG_RECONFIG_IMAGE:     //A70LV_Doulas_0009
            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : eDATA_PATH_MSG_RECONFIG_IMAGE\r\n", __FUNCTION__, __LINE__);
      //      halC789Ctrl_OutputEnableSet(0);     //A70LV_Doulas_0079 disable C789 output
            palDataPath_StateMachineProc(uiMsgID);
            m_sSourceDesc.ucSourcelostCount = 0;  //ZU860_Doulas_0104
            break;

        case eDATA_PATH_MSG_TEST_PATTERN:        //A70LV_Doulas_0030
            {
                UINT8 uc3D_Enable = 0;

                LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : eDATA_PATH_MSG_TEST_PATTERN\r\n", __FUNCTION__, __LINE__);
                palDataMgr_Data_Access(edc3D_ENABLE, edaREAD, &uc3D_Enable);

                if(uc3D_Enable == eCM_3D_FORMAT_FRAME_SEQUENTIAL)
                {
                    halMCU_3D_Sync_In_Select_Set(e3D_SYNC_INPUT_SELECTION_C821_OUTPUT_3D_SYNC);
                    halFormatter_3D_SYNC_SelectionSet(e3D_SYNC_INPUT_SELECTION_C821_OUTPUT_3D_SYNC);
                    palDataMgr_FRAME_SEQUENTIAL_3D_SYNC_Select_Set(e3D_SYNC_INPUT_SELECTION_C821_OUTPUT_3D_SYNC);
                }
                palDataPath_GotoState(eDATA_PATH_STATE_TPG_DISPLAYED);
            }
            break;

        case eDATA_PATH_MSG_NONE:                   /* timeout handling used for polling */
            /* call the polling function for each device */
            //----- Roger Added 20061012  Start
#if 0
            switch(sourceDesc.source)
            {
                case SRC_VGA1:
                case SRC_VGA2:
                case SRC_DVI_A:
                case SRC_RCA_ADC_COMPONENT:
                    if(sourceDesc.sourceActive)
                    {
                        msgID = adccontrol_Poll(datapathState);
                    }
                    else
                    {
                        msgID = DPMSG_NONE;
                    }

                    break;

                case SRC_DVI_D:
                case SRC_HDMI:
                    if(sourceDesc.sourceActive)
                    {
                        msgID = dvicontrol_Poll(datapathState);
                    }
                    else
                    {
                        msgID = DPMSG_NONE;
                    }

                    break;

                case SRC_COMPOSITE:
                case SRC_SVIDEO:
                case SRC_VGA1_DEC_COMPONENT:
                case SRC_VGA1_SCART:
                case SRC_VGA2_DEC_COMPONENT:
                case SRC_VGA2_SCART:
                case SRC_DVI_DEC_COMPONENT:
                case SRC_DVI_SCART:
                case SRC_RCA_DEC_COMPONENT:
                    msgID = deccontrol_Poll(datapathState);
                    break;

                default:
                    msgID = DPMSG_NONE;
                    break;
            }

#endif

            /* process messages returned from the polling functions */
            /* note: DPMSG_NONE still checks each state for timeout */
            palDataPath_StateMachineProc(uiMsgID);

            break;

        default:
            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : DEFAULT\r\n", __FUNCTION__, __LINE__);
            palDataPath_StateMachineProc(uiMsgID);
            break;
    }
}



eEXEC_CODE palDataPath_PowerStandby(void)
{
    UINT32 temp;
    UINT8 ucData = 0;

    LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);

    palDataPath_MailBox_Send(m_sPalDataPathInfo.xMsgQueue, m_sPalDataPathInfo.xEventGroupHandle,
                    eDATA_PATH_MSG_GOTOSTATE, -1, eDATA_PATH_STATE_SUSPENDED, &temp, FALSE);

    //TODO
    /*
        1.Front End go to standby
           adccontrol_powerStandby();
           dvicontrol_powerStandby();
           deccontrol_powerStandby();
    */
    palDataMgr_Data_Access(edcOSDTEST_PATTERN, edaREAD, &ucData);   //G100_Owen_0034
    if(ucData != eCM_TEST_PATTERN_OFF)      //G100_Owen_0034
    {
        ucData = eCM_TEST_PATTERN_OFF;
        palDataMgr_Data_Access(edcOSDTEST_PATTERN, edaWRITE_RAM_ONLY_WITH_ACTION, &ucData);
    }

    halScaler_Input_3D_Format_Set(eINPUT_3D_TYPE_OFF); //A35G2_CDS_Larry_0027
    palImgMgr_PowerStandby(NULL);

    ucData = HDMI_OUT_DISABLE;
    palFrontEndMgr_HDMI_OUT_Set(&ucData);
    palFrontEndCtrl_FocusResync();

    return eEXEC_CODE_PASS;
}



eEXEC_CODE palDataPath_PowerNormal(ePANEL_ID ePanelId)
{
    UINT32 temp;
    UINT8 ucData = 0;

    LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) pID=%d\r\n", __FUNCTION__, __LINE__, m_sPalDataPathInfo.ePanelTimingId);

    m_sPalDataPathInfo.bDataPathReady = FALSE;

    m_sPalDataPathInfo.ePanelTimingId  = ePanelId;
    m_sPalDataPathInfo.uiPanelChange = 0;
    LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) pID=%d set=%d\r\n", __FUNCTION__, __LINE__, m_sPalDataPathInfo.ePanelTimingId, ePanelId);
#if (CUSTOMIZED_EDID_ENABLE == TRUE)
    palDataPath_CustomizedEDID_Init();    //A35G2_Simon_0091
#endif
    palDataMgr_Scaler_EEPROM_Iint();    //A70LV_Doulas_0229 Add
    palDataPath_InitInputSource(); //H30K_Wesley_0002 //H30K_Wesley_0003
    palImgMgr_SetInputSource(eSOURCE_WINDOW_MAIN, m_sSourceDesc.eConnector);   //A70LV_Doulas_0007
    halScaler_PowerNormal(0, m_sPalDataPathInfo.ePanelTimingId);
    halScaler_PowerNormal(1, m_sPalDataPathInfo.ePanelTimingId);    //init CH2 //A35G2_BRC_Casper_0031

    m_uiFirstLanguageCount = 0;
    m_ucSmoothOnReadyCheck = 0;
    palSysCtrlMgr_XillinxFPGA_Init_Set(m_sPalDataPathInfo.ePanelTimingId);    //A70LV_Doulas_0216
	palDataPath_C789_C821_InitReadySet(1);		//G100_Doulas_0076
    halFormatter_IMG_AlgorithmSet(IMG_ALG_CCA,TRUE);        //A70LV_Doulas_0207 Add
    halFormatter_IMG_AlgorithmSet(IMG_ALG_GAMMA,TRUE);      //A70LV_Doulas_0207 Add
    //Add C734 OSD hardware init function after halScaler_PowerNormal(in PC platform demo board will connect after this function), OSD should init complete before Splash display
#if(defined(CUSTOM_OPTOMA))
    palDataMgr_Data_Access(edcBACKGROUND_COLOR, edaREAD, &ucData);
    palDataMgr_Data_Access(edcBACKGROUND_COLOR, edaWRITE_RAM_ONLY_WITH_ACTION, &ucData);
#else
    palDataMgr_Data_Access(edcSPLASH_STARTUP, edaREAD, &ucData);
    palDataMgr_Data_Access(edcSPLASH_STARTUP, edaWRITE_RAM_ONLY_WITH_ACTION, &ucData);
#endif

    palDataMgr_UI_EventSend(edcUI_EVENT_PANEL_CHANGE, ePanelId, NULL);
    palDataMgr_UI_EventSend(edcUI_EVENT_INITIAL, 1, NULL);

    palDataPath_Test_SetTimeStart();
    palDataPath_MailBox_Send(m_sPalDataPathInfo.xMsgQueue,
                    m_sPalDataPathInfo.xEventGroupHandle,
                    eDATA_PATH_MSG_INIT, -1, 0, &temp, FALSE);

    return eEXEC_CODE_PASS;
}

void palDataPath_Task_Suspend(void)
{
#if (SYSTEM_OS_TYPE == POSIX_COMPLIANT)
	if(m_sPalDataPathInfo.bTaskPause == FALSE)
	{
        struct timespec s_timeout;
        clock_gettime(CLOCK_REALTIME, &s_timeout);
        util_TimespecAddms(&s_timeout, APP_TASK_SEMPHORE_WAIT);

        if(pthread_mutex_timedlock(&m_sPalDataPathInfo.xTaskMutex, &s_timeout) == 0)
        {
    		m_sPalDataPathInfo.bTaskPause = TRUE;
    		pthread_mutex_unlock(&m_sPalDataPathInfo.xTaskMutex);
        }
        else
        {
            printf("(func:%s, line:%d) fail\r\n", __FUNCTION__, __LINE__);
        }
    }
	else
	{
        printf("(func:%s, line:%d) transport suspend already\r\n", __FUNCTION__, __LINE__);
	}
#else
    UINT32 temp;
    palDataPath_MailBox_Send(m_sPalDataPathInfo.xMsgQueue, m_sPalDataPathInfo.xEventGroupHandle,
                    eDATA_PATH_MSG_GOTOSTATE, -1, eDATA_PATH_STATE_SUSPENDED, &temp, FALSE);

#endif
}

void palDataPath_Task_Resume(void)
{
#if (SYSTEM_OS_TYPE == POSIX_COMPLIANT)
	if(m_sPalDataPathInfo.bTaskPause == TRUE)
	{
        struct timespec s_timeout;
        clock_gettime(CLOCK_REALTIME, &s_timeout);
        util_TimespecAddms(&s_timeout, APP_TASK_SEMPHORE_WAIT);

        if(pthread_mutex_timedlock(&m_sPalDataPathInfo.xTaskMutex, &s_timeout) == 0)
        {
    		m_sPalDataPathInfo.bTaskPause = FALSE;
    		pthread_cond_broadcast(&m_sPalDataPathInfo.xTaskCond);

    		pthread_mutex_unlock(&m_sPalDataPathInfo.xTaskMutex);
        }
        else
        {
            printf("(func:%s, line:%d) fail\r\n", __FUNCTION__, __LINE__);
        }
    }
	else
	{
        printf("(func:%s, line:%d) transport resume already\r\n", __FUNCTION__, __LINE__);
	}
#else
    UINT32 temp;
    palDataPath_MailBox_Send(m_sPalDataPathInfo.xMsgQueue, m_sPalDataPathInfo.xEventGroupHandle,
                    eDATA_PATH_MSG_GOTOSTATE, -1, eDATA_PATH_STATE_BEGIN_SCAN, &temp, FALSE);

#endif
}

static void* palDataPath_Task(void *pParameters)
{
    eMAIL_BOX_EXEC_CODE eExecCode;

    LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);

    while(1)
    {
        if(!m_sPalDataPathInfo.bTaskPause)
        {
#if (SYSTEM_OS_TYPE == POSIX_COMPLIANT)
            if((eExecCode = palMailBox_Receive_MsgQueue(m_sPalDataPathInfo.xMsgQueue,
                                                        QUEUE_DATAPATH_NAME,
                                                        (INT32)m_sPalDataPathInfo.lPollPeriod,
                                                        palDataPath_Callback)) != eMAIL_BOX_EXEC_CODE_PASS)
#else

            if((eExecCode = palMailBox_Receive(m_sPalDataPathInfo.xMsgQueue,
                                               m_sPalDataPathInfo.lPollPeriod,
                                               palDataPath_Callback)) != eMAIL_BOX_EXEC_CODE_PASS)
#endif
            {
#if (SYSTEM_OS_TYPE == POSIX_COMPLIANT)
                pthread_mutex_lock(&m_sPalDataPathInfo.xTaskMutex);
#endif
                if(eExecCode == eMAIL_BOX_EXEC_CODE_NOMSG)      /* timeout -- used to poll state machine */
                {
                    //LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d): NO MSG EVENT!\r\n", __FUNCTION__, __LINE__);
                    palDataPath_Callback(eDATA_PATH_MSG_NONE, 0, 0);
                }
                else
                {
                    LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d): Receive Event Error ID [%d]!\r\n", __FUNCTION__, __LINE__, eExecCode);
                }

#if (SYSTEM_OS_TYPE == POSIX_COMPLIANT)
                pthread_mutex_unlock(&m_sPalDataPathInfo.xTaskMutex);
#endif
            }

            palSystem_TaskMonitorTimerReset(eTID_DATAPATH, NULL);  //A35G2_Simon_0075
        }
        else
        {
#if (SYSTEM_OS_TYPE == POSIX_COMPLIANT)
            pthread_cond_wait(&m_sPalDataPathInfo.xTaskCond, &m_sPalDataPathInfo.xTaskMutex);
            pthread_mutex_unlock(&m_sPalDataPathInfo.xTaskMutex);
#endif
        }
    }

}



eEXEC_CODE palDataPath_Init(ePANEL_ID ePanelId, const PsSYSTEM_CONFIGURATION psSysConfiguration)
{
#if(CURRENT_RTOS_TYPE != RTOS_STATIC)
    BaseType_t xReturned;
#endif

    UINT8 ucWarpingMode  = 0; //A70LV_Larry_0137
    UINT8 ucWarpingPitch = 0; //A70LV_Larry_0137
    UINT32 temp; //H30K_Doulas_0071

    LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);

    m_psSysConfiguration = psSysConfiguration;

    m_sPalDataPathInfo.ePanelTimingId   = ePanelId;
    m_sPalDataPathInfo.lPollPeriod      = 0;//Because lPollPeriod to 0 means max delay

    GuiCb = Gui_fpCallbackGet();
#if (SYSTEM_OS_TYPE == POSIX_COMPLIANT)

    //Create Mailbox
    LOG_MSG(db_APP_DATAPATH, "Datapath queue create start\n");

    if(palMailBox_Create_MsgQueue(&m_sPalDataPathInfo.xMsgQueue, QUEUE_DATAPATH_NAME, &m_sPalDataPathInfo.xMsgQueue_attr) != eMAIL_BOX_EXEC_CODE_PASS)
    {
        LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d): Create Msg Queue Fail!\r\n", __FUNCTION__, __LINE__);
        return eEXEC_CODE_FAIL;
    }

    LOG_MSG(db_APP_DATAPATH, "Datapath queue create end\n");


    //init
    halScaler_Init(0);//A70LV_Doulas_0003 //Must put before palDataPath_PowerStandby()
    palGeo_Initial();     //H2PF_Simon_0107
    //palDataPath_PowerStandby(); //H30K_Doulas_0071
    palDataPath_MailBox_Send(m_sPalDataPathInfo.xMsgQueue, m_sPalDataPathInfo.xEventGroupHandle,
                    eDATA_PATH_MSG_GOTOSTATE, -1, eDATA_PATH_STATE_SUSPENDED, &temp, FALSE); //H30K_Doulas_0071
    palImgMgr_PowerStandby(NULL); //H30K_Doulas_0071
    palFrontEndCtrl_FocusResync(); //H30K_Doulas_0071


    //Create Thread Pasue
    if(pthread_mutex_init(&m_sPalDataPathInfo.xTaskMutex, NULL) != 0)
    {
        LOG_MSG(db_ALWAYS, "(func:%s, line:%d): Create xTaskMutex Fail!\r\n", __FUNCTION__, __LINE__);
    }

    m_sPalDataPathInfo.bTaskPause = FALSE;

    //Create Thread
    pthread_attr_init(&m_sPalDataPathInfo.xTaskHandle_attr);
    pthread_attr_setstacksize(&m_sPalDataPathInfo.xTaskHandle_attr, APP_DATAPATH_STACK_SIZE);
//    pthread_attr_setschedpolicy( &m_sPalDataPathInfo.xTaskHandle_attr, SCHED_FIFO );
    pthread_attr_setdetachstate( &m_sPalDataPathInfo.xTaskHandle_attr, PTHREAD_CREATE_DETACHED );
    pthread_cond_init(&m_sPalDataPathInfo.xTaskCond, NULL); //A35G2_BRC_Casper_0051
    INT16 iCreateThreadError = 0 ;
    iCreateThreadError = pthread_create(&m_sPalDataPathInfo.xTaskHandle, &m_sPalDataPathInfo.xTaskHandle_attr, palDataPath_Task, (void*) NULL);

    if(iCreateThreadError != 0)
    {
        LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d): Create Thread Fail!\r\n", __FUNCTION__, __LINE__);
        return eEXEC_CODE_FAIL;
    }

    LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d): Create Thread Pass! (ID:%d)\r\n", __FUNCTION__, __LINE__ , m_sPalDataPathInfo.xTaskHandle);


#else //freertos

#if(CURRENT_RTOS_TYPE == RTOS_STATIC)

    if(palMailBox_Create_Static(&m_sPalDataPathInfo.xMsgQueue, &m_sPalDataPathInfo.xEventGroupHandle,
                                &m_sPalDataPathInfo.xStaticQueue, m_sPalDataPathInfo.ucQueueStorageArea,
                                &m_sPalDataPathInfo.xCreatedEventGroup) != eMAIL_BOX_EXEC_CODE_PASS)
    {
        LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d): Create Msg Queue Fail!\r\n", __FUNCTION__, __LINE__);
        return eEXEC_CODE_FAIL;
    }

    halScaler_Init(eSOURCE_WINDOW_MAIN);//A70LV_Doulas_0003 //Must put before palDataPath_PowerStandby()

//#if(CURRENT_PLATFORM == PLATFORM_4K_C790 || CURRENT_PLATFORM == PLATFORM_2K_C787) //A70LV_Larry_0051 mask

    palDataMgr_Data_Access(edcWARP_MODE, edaREAD, &ucWarpingMode); //A70LV_Larry_0137
    palDataMgr_Data_Access(edcWARP_MOVE_PITCH_MODE, edaREAD, &ucWarpingPitch); //A70LV_Larry_0137

    m_sPalDataPathInfo.eMode = (eHAL_WARPING_WARP_MODE)ucWarpingMode; //A70LV_Larry_0137
    m_sPalDataPathInfo.bGridShow = FALSE;
    m_sPalDataPathInfo.eMoveIdx = (eHAL_WARPING_MOVE_PITCH)ucWarpingPitch; //A70LV_Larry_0137

    m_sPalDataPathInfo.sWarpPjParam.fAxis = 50;
    m_sPalDataPathInfo.sWarpPjParam.fDist = 3000;
    m_sPalDataPathInfo.sWarpPjParam.fVw = 1000;


    palGeo_Initial();

    //halWarping_Init(m_sPalDataPathInfo.eMode,
    //                m_sPalDataPathInfo.eMoveIdx);
//#endif

    //palDataPath_PowerStandby(); //H30K_Doulas_0071
    palDataPath_MailBox_Send(m_sPalDataPathInfo.xMsgQueue, m_sPalDataPathInfo.xEventGroupHandle,
                    eDATA_PATH_MSG_GOTOSTATE, -1, eDATA_PATH_STATE_SUSPENDED, &temp, FALSE); //H30K_Doulas_0071
    palImgMgr_PowerStandby(NULL); //H30K_Doulas_0071
    palFrontEndCtrl_FocusResync(); //H30K_Doulas_0071

    m_sPalDataPathInfo.xTaskHandle = xTaskCreateStatic(
                                         palDataPath_Task,       /* Function that implements the task. */
                                         "DPTSK",          /* Text name for the task. */
                                         APP_DATAPATH_STACK_SIZE,      /* Number of indexes in the xStack array. */
                                         NULL,    /* Parameter passed into the task. */
                                         APP_DATAPATH_PRIORITY,/* Priority at which the task is created. */
                                         m_sPalDataPathInfo.xStack,          /* Array to use as the task's stack. */
                                         &m_sPalDataPathInfo.xTaskBuffer);  /* Variable to hold the task's data structure. */

    if(m_sPalDataPathInfo.xTaskHandle == NULL)
    {
        LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d): Create Task Fail!\r\n", __FUNCTION__, __LINE__);
        return eEXEC_CODE_FAIL;
    }

#else   //(CURRENT_RTOS_TYPE == RTOS_DYNAMIC)

    if(palMailBox_Create(&m_sPalDataPathInfo.xMsgQueue, &m_sPalDataPathInfo.xEventGroupHandle) != eMAIL_BOX_EXEC_CODE_PASS)
    {
        LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d): Create Msg Queue Fail!\r\n", __FUNCTION__, __LINE__);
        return eEXEC_CODE_FAIL;
    }

    halScaler_Init(eSOURCE_WINDOW_MAIN);//A70LV_Doulas_0003 //Must put before palDataPath_PowerStandby()

    //palDataPath_PowerStandby(); //H30K_Doulas_0071
    palDataPath_MailBox_Send(m_sPalDataPathInfo.xMsgQueue, m_sPalDataPathInfo.xEventGroupHandle,
                    eDATA_PATH_MSG_GOTOSTATE, -1, eDATA_PATH_STATE_SUSPENDED, &temp, FALSE); //H30K_Doulas_0071
    palImgMgr_PowerStandby(NULL); //H30K_Doulas_0071
    palFrontEndCtrl_FocusResync(); //H30K_Doulas_0071

    xReturned = xTaskCreate(palDataPath_Task, (const char *) "DPTSK",
                            APP_DATAPATH_STACK_SIZE,
                            NULL,
                            APP_DATAPATH_PRIORITY,
                            &m_sPalDataPathInfo.xTaskHandle);

    if(xReturned != pdPASS)
    {
        LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d): Create Task Fail!\r\n", __FUNCTION__, __LINE__);
        return eEXEC_CODE_FAIL;
    }

#endif

#endif //(SYSTEM_OS_TYPE == XX)  endif

    return eEXEC_CODE_PASS;

}


void palDataPath_Resync(void)
{
    UINT32 temp;
  //  palDataPath_GotoState(eDATA_PATH_STATE_SCALER_PORT_CONFIG);
    palEnvironment_AutoSourceSearchTimerClear();        //A70LV_Doulas_0179
    palDataPath_MailBox_Send(m_sPalDataPathInfo.xMsgQueue,
                    m_sPalDataPathInfo.xEventGroupHandle,
                    eDATA_PATH_MSG_RESYNC, -1, 0, &temp, FALSE);    //A70LV_Doulas_0003
}

eEXEC_CODE palDataPath_Panel_Change(ePANEL_ID ePanelId)   //A70LV_Doulas_0005
{
    UINT32 temp;
    m_sPalDataPathInfo.ePanelTimingId = ePanelId;
//    palDataPath_MailBox_Send(m_sPalDataPathInfo.xMsgQueue,
//                    m_sPalDataPathInfo.xEventGroupHandle,
//                    eDATA_PATH_MSG_PANEL_CHANGE, -1, 0, &temp, FALSE);
    if(halScaler_Panel_Set(0,m_sPalDataPathInfo.ePanelTimingId) == eHAL_SCALER_EXEC_CODE_PASS)  //ZU860_Doulas_0076 modify
    {
        UINT8 ucPIP_EN = 0;     //HICC2_Doulas_0133

        LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d): palDataPath_Panel_Change %d\r\n", __FUNCTION__, __LINE__, ePanelId);

        palDataMgr_Data_Access(edcPIP_PBP_ENABLE, edaREAD, &ucPIP_EN); //HICC2_Doulas_0133
        if((ePanelId == PANEL_2D_OUTPUT) && (ucPIP_EN != eCM_SCREEN_MODE_OFF)) //HICC2_Doulas_0133
        {
            UINT8  ucDisplayMode = 0;
            UINT8  ucPreUserMode = eCM_PICTURE_SETTINGS_BRIGHT;
            if(palDataMgr_Data_Access(edcPICTURE_SETTINGS, edaREAD, &ucDisplayMode) == eEXEC_CODE_PASS)
            {
                if((ucDisplayMode == eCM_PICTURE_SETTINGS_3D) ||
                   (ucDisplayMode == eCM_PICTURE_SETTINGS_2DHIGHSPEED))
                {
                    ucPreUserMode = eCM_PICTURE_SETTINGS_BRIGHT;
                    palDataMgr_Access_Apply_User_Mode(edaWRITE_THROUGH_NO_ACTION, &ucPreUserMode);

                    ucDisplayMode = eCM_PICTURE_SETTINGS_BRIGHT;
                    palDataMgr_Access_PictureSettings(edaWRITE_THROUGH_NO_ACTION, &ucDisplayMode);
                    palLANProcSendToLAN((UINT16)edcPICTURE_SETTINGS);
                    palLANProcSendToLAN((UINT16)edcUSER_COLOR_MODE);
                }
			    else if(ucDisplayMode == eCM_PICTURE_SETTINGS_USER)
			    {
			        palDataMgr_Access_Apply_User_Mode(edaREAD,&ucPreUserMode);
                    if((ucPreUserMode == eCM_PICTURE_SETTINGS_3D) ||
                       (ucPreUserMode == eCM_PICTURE_SETTINGS_2DHIGHSPEED))
                    {
                        ucPreUserMode = eCM_PICTURE_SETTINGS_BRIGHT;
                        palDataMgr_Access_Apply_User_Mode(edaWRITE_THROUGH_NO_ACTION, &ucPreUserMode);
                        palLANProcSendToLAN((UINT16)edcUSER_COLOR_MODE);
                    }
			    }
			    palDataMgr_Formatter_Parameter_Set(TRUE);
            }

        }
        else
        {
            palDataMgr_Formatter_Parameter_Set(TRUE); //H30K_Doulas_0081
        }

        m_sPalDataPathInfo.uiPanelChange = 20; //2s

        palDataPath_MailBox_Send(m_sPalDataPathInfo.xMsgQueue,
                    m_sPalDataPathInfo.xEventGroupHandle,
                    eDATA_PATH_MSG_RESYNC, -1, 0, &temp, FALSE);
    }
    return eEXEC_CODE_PASS;
}

eEXEC_CODE palDataPath_InitInputSource(void)   //A70LV_Doulas_0029 modify //A70LV_Doulas_0007
{
    INT32 iVAl = 0;     //A70LV_Doulas_0332 modify

    palDataMgr_Data_Access(edcMAIN_INPUT, edaREAD, &iVAl);
    m_sSourceDesc.eConnector = (eCM_SOURCE_ID)iVAl;

    return eEXEC_CODE_PASS;
}

eEXEC_CODE palDataPath_CheckFrontendReady(void)
{
    INT32 iVAl = 0;     //A70LV_Doulas_0332 modify
    INT16 uiRetry = 10; //H30K_Wesley_0002

    halFrontEndCtrl_MainInput_Set((UINT8*)&m_sSourceDesc.eConnector);    //set main input source
    do
    {
        MS_SLEEP(100);
        halFrontEndCtrl_MainInput_Get((UINT8 *)&iVAl);
        if(iVAl != m_sSourceDesc.eConnector)
            halFrontEndCtrl_MainInput_Set((UINT8*)&m_sSourceDesc.eConnector);    //set main input source
        LOG_MSG(db_APP_DATAPATH, "palDataPath_InitInputSource wait FrontEnd ready ! (%d, %d)\r\n", m_sSourceDesc.eConnector, iVAl);
    }while((iVAl != m_sSourceDesc.eConnector) && uiRetry--);
    return eEXEC_CODE_PASS;
}




eEXEC_CODE palDataPath_Reconfig_Image(void)     //A70LV_Doulas_0009
{
    UINT32 temp;
    palDataPath_MailBox_Send(m_sPalDataPathInfo.xMsgQueue,
                    m_sPalDataPathInfo.xEventGroupHandle,
                    eDATA_PATH_MSG_RECONFIG_IMAGE, -1, 0, &temp, FALSE);

    return eEXEC_CODE_PASS;
}

eEXEC_CODE palDataPath_OverscanSet(UINT8 ucOverScan)   //A70LV_Doulas_0009
{
    eHAL_SCALER_EXEC_CODE eScalerResult = eHAL_SCALER_EXEC_CODE_FAIL;
    eEXEC_CODE eExecResult = eEXEC_CODE_PASS;

    eScalerResult = halScaler_Overscan_Set(ucOverScan);     //A70LV_Doulas_0020
    if(eScalerResult == eHAL_SCALER_EXEC_CODE_PASS)
    {
        palDataPath_Reconfig_Image();
    }
    else
        eExecResult = eEXEC_CODE_FAIL;

    return eExecResult;
}

eEXEC_CODE palDataPath_Digital_Horz_Zoom_Set(UINT16 uwDigital_Horz_Zoom)   //A70LV_Doulas_0009
{
    eHAL_SCALER_EXEC_CODE eScalerResult = eHAL_SCALER_EXEC_CODE_FAIL;
    eEXEC_CODE eExecResult = eEXEC_CODE_PASS;

    eScalerResult = palImgMgr_DigitalHorzZoom_Set(uwDigital_Horz_Zoom);     //A70LV_Doulas_0020
    if(eScalerResult == eHAL_SCALER_EXEC_CODE_PASS)
    {
        //palDataPath_Reconfig_Image();
    //    palEnvironment_LightSourceBlankingOn_Set(TRUE,3);	//A70Gen2_Doulas_0026
        halScaler_ConfigureForScaler(eSOURCE_WINDOW_MAIN);  //A70LV_Doulas_0284
    }
    else
        eExecResult = eEXEC_CODE_FAIL;

    return eExecResult;
}

eEXEC_CODE palDataPath_Digital_Vert_Zoom_Set(UINT16 uwDigital_Vert_Zoom)   //A70LV_Doulas_0009
{
    eHAL_SCALER_EXEC_CODE eScalerResult = eHAL_SCALER_EXEC_CODE_FAIL;
    eEXEC_CODE eExecResult = eEXEC_CODE_PASS;

    eScalerResult = palImgMgr_DigitalVertZoom_Set(uwDigital_Vert_Zoom);     //A70LV_Doulas_0020
    if(eScalerResult == eHAL_SCALER_EXEC_CODE_PASS)
    {
        //palDataPath_Reconfig_Image();
    //    palEnvironment_LightSourceBlankingOn_Set(TRUE,3);	//A70Gen2_Doulas_0026
        halScaler_ConfigureForScaler(eSOURCE_WINDOW_MAIN);  //A70LV_Doulas_0284
    }
    else
        eExecResult = eEXEC_CODE_FAIL;

    return eExecResult;
}

eEXEC_CODE palDataPath_Digital_Horz_Shift_Set(UINT16 uwDigital_Horz_Shift)   //A70LV_Doulas_0009
{
    eHAL_SCALER_EXEC_CODE eScalerResult = eHAL_SCALER_EXEC_CODE_FAIL;
    eEXEC_CODE eExecResult = eEXEC_CODE_PASS;

    eScalerResult = halScaler_DigitalHorzShift_Set(uwDigital_Horz_Shift);   //A70LV_Doulas_0020
    if(eScalerResult == eHAL_SCALER_EXEC_CODE_PASS)
    {
        //palDataPath_Reconfig_Image();
    //    palEnvironment_LightSourceBlankingOn_Set(TRUE,3);	//A70Gen2_Doulas_0026
        halScaler_ConfigureForScaler(eSOURCE_WINDOW_MAIN);  //A70LV_Doulas_0284
    }
    else
        eExecResult = eEXEC_CODE_FAIL;

    return eExecResult;
}

eEXEC_CODE palDataPath_Digital_Vert_Shift_Set(UINT16 uwDigital_Vert_Shift)   //A70LV_Doulas_0009
{
    eHAL_SCALER_EXEC_CODE eScalerResult = eHAL_SCALER_EXEC_CODE_FAIL;
    eEXEC_CODE eExecResult = eEXEC_CODE_PASS;

    eScalerResult = halScaler_DigitalVertShift_Set(uwDigital_Vert_Shift);   //A70LV_Doulas_0020
    if(eScalerResult == eHAL_SCALER_EXEC_CODE_PASS)
    {
        //palDataPath_Reconfig_Image();
    //    palEnvironment_LightSourceBlankingOn_Set(TRUE,3);	//A70Gen2_Doulas_0026
        halScaler_ConfigureForScaler(eSOURCE_WINDOW_MAIN);  //A70LV_Doulas_0284
    }
    else
        eExecResult = eEXEC_CODE_FAIL;

    return eExecResult;
}

eEXEC_CODE palDataPath_Vert_Start_Position_Set(UINT8 uwVertPosition)   //A70LV_Doulas_0010
{
    eHAL_SCALER_EXEC_CODE eScalerResult = eHAL_SCALER_EXEC_CODE_FAIL;
    eEXEC_CODE eExecResult = eEXEC_CODE_PASS;

    eScalerResult = halScaler_VertPosition_Set(uwVertPosition);     //A70LV_Doulas_0020
    if(eScalerResult == eHAL_SCALER_EXEC_CODE_PASS)
    {
        //palDataPath_Reconfig_Image();
        halScaler_ConfigureForPosition(eSOURCE_WINDOW_MAIN);     //A70LV_Doulas_0238
        //halScaler_ConfigureForDisplay(eSOURCE_WINDOW_MAIN,1);        //A70LV_Doulas_0238 remove//A70LV_Doulas_0154 Add
        eScalerResult = halScaler_Position_Run(eSOURCE_WINDOW_MAIN);   //A70LV_Doulas_0113 modify
        if(eScalerResult != eHAL_SCALER_EXEC_CODE_PASS)
            eExecResult = eEXEC_CODE_FAIL;
    }
    else
        eExecResult = eEXEC_CODE_FAIL;

    return eExecResult;
}

eEXEC_CODE palDataPath_Horz_Start_Position_Set(UINT8 uwHorzPosition)   //A70LV_Doulas_0010
{
    eHAL_SCALER_EXEC_CODE eScalerResult = eHAL_SCALER_EXEC_CODE_FAIL;
    eEXEC_CODE eExecResult = eEXEC_CODE_PASS;

    eScalerResult = halScaler_HorzPosition_Set(uwHorzPosition);     //A70LV_Doulas_0020
    if(eScalerResult == eHAL_SCALER_EXEC_CODE_PASS)
    {
        //palDataPath_Reconfig_Image();
        halScaler_ConfigureForPosition(eSOURCE_WINDOW_MAIN);     //A70LV_Doulas_0238
        //halScaler_ConfigureForDisplay(eSOURCE_WINDOW_MAIN,1);        //A70LV_Doulas_0238 remove//A70LV_Doulas_0154 Add
        eScalerResult = halScaler_Position_Run(eSOURCE_WINDOW_MAIN);   //A70LV_Doulas_0113 modify
        if(eScalerResult != eHAL_SCALER_EXEC_CODE_PASS)
            eExecResult = eEXEC_CODE_FAIL;
    }
    else
        eExecResult = eEXEC_CODE_FAIL;

    return eExecResult;
}

eEXEC_CODE palDataPath_Format_ContrastEnhancementSet(UINT8 ucContrastEnhancement, UINT8 ucRealBlackLevel)   //A70LV_Doulas_0013
{
    //eHAL_SCALER_EXEC_CODE eScalerResult = eHAL_SCALER_EXEC_CODE_FAIL;
    eEXEC_CODE eExecResult = eEXEC_CODE_PASS;

#ifdef OUTPUT_PANEL_4K //H30K_Doulas_0031
    halFormatter_DB_EnableSet(FALSE); //Enable to Enable --> PWM doesn't updated.
    switch(ucContrastEnhancement)
    {
        default:
        case eCONTRAST_ENHANCE_OFF:
            {
                //halFormatter_DB_EnableSet(FALSE); //A70LV_Doulas_0190 remove
                halFormatter_DISP_BlackStatusSet(FALSE);
                halFormatter_DISP_BlackLevelSet(eDDP_DISP_BLACK_LEVEL_OFF);
                halFormatter_DB_EnableSet(FALSE);   //A70LV_Doulas_0190 Add
            }
            break;

        case eCONTRAST_ENHANCE_DYNAMIC_BLACK:
            {
                //halFormatter_DISP_BlackStatusSet(FALSE);
                halFormatter_DISP_BlackStatusSet(eCONTRAST_ENHANCE_DYNAMIC_BLACK - eCONTRAST_ENHANCE_DYNAMIC_BLACK);
                #ifdef DMD_WUXGA
                halFormatter_DB_SetBorderCfg(0,0,960,1200);
                #else
    			halFormatter_DB_SetBorderCfg(0,0,960,1080);	//A70LK_Doulas_0006
    			#endif
    			halFormatter_DB_SetClipPixels(5120);		//A70LK_Doulas_0006
                halFormatter_DB_EnableSet(TRUE);
                halFormatter_DISP_BlackLevelSet(eDDP_DISP_BLACK_LEVEL_OFF);
            }
            break;

        case eCONTRAST_ENHANCE_REAL_BLACK: //A70LV_Larry_0236 modify
            {
                //UINT8 cBlackLevel = eDDP_DISP_BLACK_LEVEL_DEFAULT;

                //cBlackLevel = eDDP_DISP_BLACK_LEVEL_DEFAULT + (ucRealBlackLevel << LIGHTSOUTSIGNALLEVELSTRENGHT);
                halFormatter_DISP_BlackStatusSet(eCONTRAST_ENHANCE_REAL_BLACK - eCONTRAST_ENHANCE_DYNAMIC_BLACK);
                halFormatter_DB_SetBorderCfg(0,0,960,1080);	//A70LK_Doulas_0006
                halFormatter_DB_SetClipPixels(5120);		//A70LK_Doulas_0006
                halFormatter_DB_EnableSet(TRUE);
                halFormatter_DISP_BlackLevelSet(ucRealBlackLevel);
            }
            break;

        case eCONTRAST_ENHANCE_DYNAMIC_BLACK_REAL_BLACK:
            {
                halFormatter_DISP_BlackStatusSet(eCONTRAST_ENHANCE_DYNAMIC_BLACK_REAL_BLACK - eCONTRAST_ENHANCE_DYNAMIC_BLACK);
                halFormatter_DB_SetBorderCfg(0,0,960,1080); //A70LK_Doulas_0006
                halFormatter_DB_SetClipPixels(5120);        //A70LK_Doulas_0006
                halFormatter_DB_EnableSet(TRUE);
                halFormatter_DISP_BlackLevelSet(ucRealBlackLevel);   //T100_Clare_0002
            }
            break;
    }
#else
    switch(ucContrastEnhancement)
    {
        default:
        case eCONTRAST_ENHANCE_OFF:
            //halFormatter_DB_EnableSet(FALSE); //A70LV_Doulas_0190 remove
            halFormatter_DISP_BlackLevelSet(eDDP_DISP_BLACK_LEVEL_OFF);
            halFormatter_DB_EnableSet(FALSE);   //A70LV_Doulas_0190 Add
            break;

        case eCONTRAST_ENHANCE_DYNAMIC_BLACK:
            if(palSystem_PanelID_Get() == ePANEL_ID_WUXGA_60HZ || palSystem_PanelID_Get() == ePANEL_ID_WUXGA_120HZ)   //ZU860_Doulas_0047 Add
            {
                halFormatter_DB_SetBorderCfg(0,0,1920,1200);
            }
            else
            {
                halFormatter_DB_SetBorderCfg(0,0,1920,1080);
            }

            halFormatter_DB_EnableSet(TRUE);
            halFormatter_DISP_BlackLevelSet(eDDP_DISP_BLACK_LEVEL_OFF);
            break;

        case eCONTRAST_ENHANCE_REAL_BLACK: //A70LV_Larry_0236 modify
            {
                UINT8 cBlackLevel = eDDP_DISP_BLACK_LEVEL_DEFAULT;

                cBlackLevel = ucRealBlackLevel;//eDDP_DISP_BLACK_LEVEL_DEFAULT + (ucRealBlackLevel << LIGHTSOUTSIGNALLEVELSTRENGHT);	//A70Gen2_Doulas_0044 Modify
                halFormatter_DB_EnableSet(FALSE);
                halFormatter_DISP_BlackLevelSet(cBlackLevel);
            }
            break;
    }
#endif
    return eExecResult;
}

void palDataPath_BurnInStart(void)  //A70LV_Doulas_0023
{
    palDataPath_GotoState(eDATA_PATH_STATE_SUSPENDED);
}

eEXEC_CODE palDataPath_UserSetSourceInput(eCM_SOURCE_ID eInputSource)    //A70LV_Doulas_0029
{
    UINT32 temp;
    eEXEC_CODE eExecResult = eEXEC_CODE_PASS;

    halScaler_Resync_Init(eSOURCE_WINDOW_MAIN);   //A70LV_Doulas_0120
    LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) eInputSource = %d\r\n", __FUNCTION__, __LINE__, eInputSource);//A70LV_Doulas_0231 debug
    if(palDataPath_MailBox_Send(m_sPalDataPathInfo.xMsgQueue,
                       m_sPalDataPathInfo.xEventGroupHandle,
                       eDATA_PATH_MSG_SETCONNECTOR, -1, eInputSource, &temp, FALSE) != eMAIL_BOX_EXEC_CODE_PASS )     //A70LV_Doulas_0295 modify BlockTime  //ProAV_Simon_0002
    {
        eExecResult = eEXEC_CODE_FAIL;
        LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) fail\r\n", __FUNCTION__, __LINE__);//A70LV_Doulas_0231 debug
    }
    else
    {
        m_sPalDataPathInfo.ucSourceConnectChange = 1;
    }

    return eExecResult ;
}

eEXEC_CODE palDataPath_AspectRatioSet(UINT8 ucAspectRatio)
{
    eHAL_SCALER_EXEC_CODE eScalerResult = eHAL_SCALER_EXEC_CODE_FAIL;
    eEXEC_CODE eExecResult = eEXEC_CODE_PASS;

    eScalerResult = halScaler_SizePresets_Set(ucAspectRatio);
    halScaler_DigitalHorzZoom_Set(halScaler_DigitalHorzZoom_Get());     //A70LV_Doulas_0146
    halScaler_DigitalVertZoom_Set(halScaler_DigitalVertZoom_Get());     //A70LV_Doulas_0146
    halScaler_DigitalHorzShift_Set(halScaler_DigitalHorzShift_Get());   //A70LV_Doulas_0146
    halScaler_DigitalVertShift_Set(halScaler_DigitalVertShift_Get());   //A70LV_Doulas_0146
    if(eScalerResult == eHAL_SCALER_EXEC_CODE_PASS)
    {
        //palDataPath_Reconfig_Image();
    //    palEnvironment_LightSourceBlankingOn_Set(TRUE,3);	//A70Gen2_Doulas_0026
        halScaler_ConfigureForScaler(eSOURCE_WINDOW_MAIN);  //A70LV_Doulas_0284
    }
    else
        eExecResult = eEXEC_CODE_FAIL;

    return eExecResult;
}

BOOL palDataPath_IsSourceLock(void)       //A70LV_Doulas_0032
{
    if(m_sPalDataPathInfo.ucSourceConnectChange)
    {
        return FALSE;
    }

    if((m_sPalDataPathInfo.eDataPathState == eDATA_PATH_STATE_MONITOR_SOURCE) ||
       (m_sPalDataPathInfo.eDataPathState == eDATA_PATH_STATE_ATTEMPT_LOCK) ||
       (m_sPalDataPathInfo.eDataPathState == eDATA_PATH_STATE_AUTO_PHASE) ||
       (m_sPalDataPathInfo.eDataPathState == eDATA_PATH_STATE_AUTO_POSITION))    //A70LV_Doulas_0050
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

BOOL palDataPath_IsSourceMonitor(void)       //A70LV_Doulas_0101
{
    if(m_sPalDataPathInfo.eDataPathState == eDATA_PATH_STATE_MONITOR_SOURCE)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

void palDataPath_SourceLostSet(void)
{
    UINT32 temp;
    if(m_sPalDataPathInfo.eDataPathState == eDATA_PATH_STATE_MONITOR_SOURCE || m_sPalDataPathInfo.eDataPathState == eDATA_PATH_STATE_ATTEMPT_LOCK ||
       m_sPalDataPathInfo.eDataPathState == eDATA_PATH_STATE_AUTO_PHASE     || m_sPalDataPathInfo.eDataPathState == eDATA_PATH_STATE_AUTO_POSITION)
    {
        palDataPath_MailBox_Send(m_sPalDataPathInfo.xMsgQueue,
                        m_sPalDataPathInfo.xEventGroupHandle,
                        eDATA_PATH_MSG_LOSTLOCK, -1,
                        0, &temp, FALSE);
    }
}

BOOL palDataPath_ShowLogo(void)     //A70LV_Doulas_0032
{
    //UINT8 ucIsLogoOn = 0;
    UINT8 ucValue = 0;
    eEXEC_CODE eExecResult = eEXEC_CODE_PASS;
    //UINT8 ucOSD_TP = eOSD_TP_OFF;
    UINT8 ucService_TP = eCM_TEST_PATTERN_OFF;

    //A70LV_Doulas_0053 check test pattern enable
    #if 1       //A70LV_Doulas_0230 Modify
    if(palDataMgr_CurTestPatternGet() != eTID_OFF)
    {
        ucService_TP = eCM_TEST_PATTERN_OFF;
        palDataMgr_Data_Access(edcSERVICE_TEST_PATTERN, edaWRITE_THROUGH_NO_ACTION, (PUINT8)&ucService_TP);
    }
    #else
    eExecResult = palDataMgr_Data_Access(edcOSDTEST_PATTERN, edaREAD, (PUINT8)&ucOSD_TP);
    if(eExecResult == eEXEC_CODE_PASS)      //A70LV_Doulas_0157 modify
    {
        if(ucOSD_TP != eOSD_TP_OFF)
        {
            ucOSD_TP = eOSD_TP_OFF;
            palDataMgr_Data_Access(edcOSDTEST_PATTERN, edaWRITE_THROUGH_NO_ACTION, (PUINT8)&ucOSD_TP);
        }
    }

    eExecResult = palDataMgr_Data_Access(edcSERVICE_TEST_PATTERN, edaREAD, (PUINT8)&ucService_TP);
    if(eExecResult == eEXEC_CODE_PASS)      //A70LV_Doulas_0157 modify
    {
        if(ucService_TP != eOSD_TP_OFF)
        {
            ucService_TP = eOSD_TP_OFF;
            palDataMgr_Data_Access(edcSERVICE_TEST_PATTERN, edaWRITE_THROUGH_NO_ACTION, (PUINT8)&ucService_TP);
        }
    }
    #endif
    //A70LV_Doulas_0053 end

    eExecResult = palDataMgr_Data_Access(edcPIP_PBP_ENABLE, edaREAD, (PUINT8)&ucValue);

	if(eExecResult != eEXEC_CODE_PASS)  //A70LV_Doulas_0049
        return FALSE;

    if((ucValue == eCM_SCREEN_MODE_PIP) || (ucValue == eCM_SCREEN_MODE_PBP))   //G100_Owen_0012
    {
        //if(GuiCb.fpGui_IsSplash_On_GetCb() == TRUE)
        {
            palDataMgr_UI_EventSend(edcUI_EVENT_SPLASH, 0, NULL);
        }
#if 1 //A35G2_CDS_Larry_0045
        palDataMgr_UI_EventSend(edcUI_EVENT_SUB_SOURCE_INFOR_MSG, TRUE, NULL); //GuiCb.fpGui_SendSubSourceInfoMessageCb();
#else
        appGui_SendSourceMsgEvent();        //ZU860_Doulas_0128
#endif
        return TRUE;
    }
    else    //ZU860_Owen_0020 //EK816U_626U_Doulas_0002 modify//ZU860_Doulas_0128 modify//A70LV_Doulas_0276
    {
		if(halScaler_PIP_PBP_Enable_Get() != eCM_SCREEN_MODE_OFF) //G100_Steven_0004 start
		{
			palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL);
		}
		else                                             //G100_Steven_0004 end
		{
			palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD_IF_NO_SPLASH, TRUE, NULL);

			#if 0
			if(GuiCb.fpGui_IsSplash_On_GetCb() == FALSE )
			{
				palDataPath_DisplaySplash(TRUE);
				palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL);
			}
			#endif
		}
    }

    return TRUE;
}
#if 0
eEXEC_CODE palDataPath_OSD_TestPattern_Set(UINT8 uwTestPattern)   //A70LV_Doulas_0035
{
    UINT32 temp;
    //eHAL_SCALER_EXEC_CODE eScalerResult = eHAL_SCALER_EXEC_CODE_FAIL;
    eEXEC_CODE eExecResult = eEXEC_CODE_PASS;

    #if 0//#ifdef CUSTOM_CHRISTIE
    if((uwTestPattern == eCM_TEST_PATTERN_OFF) && (GuiCb.fpGui_HSG_TestPatternEnableGetCb()))
    {
        palDataMgr_Data_Access(edcHSG_TEST_PATTERN_CTRL, edaREAD, (void*)&uwTestPattern);
    }
    #endif

    #if 1   //A70LV_Doulas_0157 modify
    GuiCb.fpGuiStyle_CurrentTestPatternSetCb(uwTestPattern);
    if(uwTestPattern == eCM_TEST_PATTERN_OFF)
    {
        //appGuiStyle_CurrentTestPatternSet(uwTestPattern);
        if(palDataPath_IsTestPatternEnable() == TRUE)
        {
            #ifdef CUSTOM_CHRISTIE
            if((m_DataPathLastState != eDATA_PATH_STATE_MONITOR_SOURCE) || (ucTestPatternSourceLostCount))
            {
                //palDataPath_ShowLogo();
				palDataPath_DisplaySplash(TRUE);
                palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL);
                if(palDataPath_MailBox_Send(m_sPalDataPathInfo.xMsgQueue,
                       m_sPalDataPathInfo.xEventGroupHandle,
                       eDATA_PATH_MSG_GOTOSTATE, -1, eDATA_PATH_STATE_SCALER_PORT_CONFIG, &temp, FALSE) != eMAIL_BOX_EXEC_CODE_PASS )
                {
                    eExecResult = eEXEC_CODE_FAIL;
                }
            }
            else
            #endif
            {
                //palDataPath_TurnOffSplash();      //ZU860_Doulas_0058 remove
                palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL);        //G100_Owen_0030
                if(m_sSourceDesc.eConnector == eCM_SOURCE_VGA)    //ZU860_Doulas_0083
                {
                    palEnvironment_LightSourceBlankingOn_Set(TRUE,1);       //LD off
                }

                if(palDataPath_MailBox_Send(m_sPalDataPathInfo.xMsgQueue,
                       m_sPalDataPathInfo.xEventGroupHandle,
                       eDATA_PATH_MSG_GOTOSTATE, -1, eDATA_PATH_STATE_LOOK_FOR_SYNCS, &temp, FALSE) != eMAIL_BOX_EXEC_CODE_PASS )     //A70LV_Doulas_0293 modify
                {
                    eExecResult = eEXEC_CODE_FAIL;
                }
            }
        }
    }
    else if(palDataPath_MailBox_Send(m_sPalDataPathInfo.xMsgQueue,
                       m_sPalDataPathInfo.xEventGroupHandle,
                       eDATA_PATH_MSG_TEST_PATTERN, -1, (UINT32)uwTestPattern, &temp, FALSE) == eMAIL_BOX_EXEC_CODE_PASS )      //A70LV_Doulas_0296 Modify Test pattern bug
    {
        #ifdef CUSTOM_BARCO //A35G2_BRC_Casper_0051
        if(utilWarp_GetOsdPatternType() != PAT_TYPE__OFF) //G100_Owen_0112
        {
            halAdvWarpShowOsdPattern(PAT_TYPE__OFF);
        }
        #endif
        palDataPath_DisplaySplash(TRUE);
        palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL);
        m_sSourceDesc.eActiveDisplay = eDATA_PATH_ACTIVE_DISPLAY_TESTPATTERN;
//        halFormatter_DMD_BGDisplayModeSet(BG_DISABLE);  //G100_Owen_0008 remove //ZU860_Doulas_0077 //DDP background off
    }
    else
    {
        eExecResult = eEXEC_CODE_FAIL;
    }
    #else
    if(uwTestPattern == eCM_TEST_PATTERN_OFF)
    {
        if(palDataPath_IsTestPatternEnable() == TRUE)
        {
            if(palMailBox_Send(m_sPalDataPathInfo.xMsgQueue,
                       m_sPalDataPathInfo.xEventGroupHandle,
                       eDATA_PATH_MSG_GOTOSTATE, -1, eDATA_PATH_STATE_MONITOR_SOURCE, &temp, FALSE) == eMAIL_BOX_EXEC_CODE_PASS )
            {
                halScaler_OSD_TestPattern(uwTestPattern);
            }
            else
            {
                eExecResult = eEXEC_CODE_FAIL;
            }
        }

    }
    else if(palMailBox_Send(m_sPalDataPathInfo.xMsgQueue,
                       m_sPalDataPathInfo.xEventGroupHandle,
                       eDATA_PATH_MSG_TEST_PATTERN, -1, uwTestPattern, &temp, FALSE) == eMAIL_BOX_EXEC_CODE_PASS )
    {
        halScaler_OSD_TestPattern(uwTestPattern);
        if(m_sSourceDesc.eActiveDisplay == eDATA_PATH_ACTIVE_DISPLAY_SPLASH)
        {
            palDataPath_TurnOffSplash();
            palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL);
        }
        m_sSourceDesc.eActiveDisplay = eDATA_PATH_ACTIVE_DISPLAY_TESTPATTERN;
    }
    else
    {
        eExecResult = eEXEC_CODE_FAIL;
    }
    #endif

    return eExecResult;
}
#endif
#if 0
eEXEC_CODE palDataPath_Service_TestPattern_Set(UINT8 uwTestPattern)
{
    UINT32 temp;
    //eHAL_SCALER_EXEC_CODE eScalerResult = eHAL_SCALER_EXEC_CODE_FAIL;
    eEXEC_CODE eExecResult = eEXEC_CODE_PASS;

    if(uwTestPattern == eCM_TEST_PATTERN_OFF)
    {
        if(palDataPath_IsTestPatternEnable() == TRUE)
        {
            if(palDataPath_MailBox_Send(m_sPalDataPathInfo.xMsgQueue,
                       m_sPalDataPathInfo.xEventGroupHandle,
                       eDATA_PATH_MSG_GOTOSTATE, -1, eDATA_PATH_STATE_MONITOR_SOURCE, &temp, FALSE) == eMAIL_BOX_EXEC_CODE_PASS )
            {
                halScaler_Service_TestPattern(uwTestPattern);
            }
            else
            {
                eExecResult = eEXEC_CODE_FAIL;
            }
        }

    }
    else if(palDataPath_MailBox_Send(m_sPalDataPathInfo.xMsgQueue,
                       m_sPalDataPathInfo.xEventGroupHandle,
                       eDATA_PATH_MSG_TEST_PATTERN, -1, uwTestPattern, &temp, FALSE) == eMAIL_BOX_EXEC_CODE_PASS )
    {
        #ifdef CUSTOM_BARCO //A35G2_BRC_Casper_0051
        if(utilWarp_GetOsdPatternType() != PAT_TYPE__OFF) //G100_Owen_0112
        {
            halAdvWarpShowOsdPattern(PAT_TYPE__OFF);
        }
        #endif
        halScaler_Service_TestPattern(uwTestPattern);
        if(m_sSourceDesc.eActiveDisplay == eDATA_PATH_ACTIVE_DISPLAY_SPLASH)
        {
            palDataPath_TurnOffSplash();
            palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL);
        }
        m_sSourceDesc.eActiveDisplay = eDATA_PATH_ACTIVE_DISPLAY_TESTPATTERN;
    }
    else
    {
        eExecResult = eEXEC_CODE_FAIL;
    }

    return eExecResult;
}
#endif

eEXEC_CODE palDataPath_HSG_TestPattern_Set(UINT8 ucTestPattern)     //A70LV_Doulas_0063
{
#if 0
    UINT32 temp;
    eEXEC_CODE eExecResult = eEXEC_CODE_PASS;

    #if 0//#ifdef CUSTOM_CHRISTIE
    if((ucTestPattern == eCM_TEST_PATTERN_OFF) && (GuiCb.fpGui_HSG_TestPatternEnableGetCb()))
    {
        palDataMgr_Data_Access(edcHSG_TEST_PATTERN_CTRL, edaREAD, (void*)&ucTestPattern);
    }
    #endif

    #if 1   //A70LV_Doulas_0157 modify
    GuiCb.fpGuiStyle_CurrentTestPatternSetCb(ucTestPattern);
    if(ucTestPattern == eCM_TEST_PATTERN_OFF)
    {
        //GuiCb.fpGuiStyle_CurrentTestPatternSetCb(uwTestPattern);
        if(palDataPath_IsTestPatternEnable() == TRUE)
        {
            #ifdef CUSTOM_CHRISTIE
            if((m_DataPathLastState != eDATA_PATH_STATE_MONITOR_SOURCE) || (ucTestPatternSourceLostCount))
            {
                //palDataPath_ShowLogo();
                if(palDataPath_MailBox_Send(m_sPalDataPathInfo.xMsgQueue,
                       m_sPalDataPathInfo.xEventGroupHandle,
                       eDATA_PATH_MSG_GOTOSTATE, -1, eDATA_PATH_STATE_SCALER_PORT_CONFIG, &temp, FALSE) != eMAIL_BOX_EXEC_CODE_PASS )
                {
                    eExecResult = eEXEC_CODE_FAIL;
                }
            }
            else
            #endif
            {
                //palDataPath_TurnOffSplash();      //ZU860_Doulas_0058 remove
                //palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL);      //ZU860_Doulas_0058 remove
                if(palDataPath_MailBox_Send(m_sPalDataPathInfo.xMsgQueue,
                       m_sPalDataPathInfo.xEventGroupHandle,
                       eDATA_PATH_MSG_GOTOSTATE, -1, eDATA_PATH_STATE_LOOK_FOR_SYNCS, &temp, FALSE) != eMAIL_BOX_EXEC_CODE_PASS )       //A70LV_Doulas_0293 modify
                {
                    eExecResult = eEXEC_CODE_FAIL;
                }
            }
        }
    }
    else if(palDataPath_MailBox_Send(m_sPalDataPathInfo.xMsgQueue,
                       m_sPalDataPathInfo.xEventGroupHandle,
                       eDATA_PATH_MSG_TEST_PATTERN, -1, ucTestPattern, &temp, FALSE) == eMAIL_BOX_EXEC_CODE_PASS )
    {
        #if defined(CUSTOM_BARCO) //A35G2_BRC_Casper_0051 //A35G2_Coda_0103 //A35G2_Coda_0116
        if(utilWarp_GetOsdPatternType() != PAT_TYPE__OFF) //G100_Owen_0112
        {
            halAdvWarpShowOsdPattern(PAT_TYPE__OFF);
        }
        #endif
        palDataPath_DisplaySplash(TRUE);
        palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL);
        m_sSourceDesc.eActiveDisplay = eDATA_PATH_ACTIVE_DISPLAY_TESTPATTERN;
        halFormatter_DMD_BGDisplayModeSet(BG_DISABLE);  //ZU860_Doulas_0094
    }
    else
    {
        eExecResult = eEXEC_CODE_FAIL;
    }
    #else
    if(ucTestPattern == (UINT8)eCM_TEST_PATTERN_OFF)
    {
        if(palDataPath_IsTestPatternEnable() == TRUE)
        {
            if(palMailBox_Send(m_sPalDataPathInfo.xMsgQueue,
                       m_sPalDataPathInfo.xEventGroupHandle,
                       eDATA_PATH_MSG_GOTOSTATE, -1, eDATA_PATH_STATE_MONITOR_SOURCE, 0, FALSE) == eMAIL_BOX_EXEC_CODE_PASS )
            {
                halScaler_Service_TestPattern(ucTestPattern);
            }
            else
            {
                eExecResult = eEXEC_CODE_FAIL;
            }
        }

    }
    else if(palMailBox_Send(m_sPalDataPathInfo.xMsgQueue,
                       m_sPalDataPathInfo.xEventGroupHandle,
                       eDATA_PATH_MSG_TEST_PATTERN, -1, ucTestPattern, 0, FALSE) == eMAIL_BOX_EXEC_CODE_PASS )
    {
        halScaler_Service_TestPattern(ucTestPattern);
        if(m_sSourceDesc.eActiveDisplay == eDATA_PATH_ACTIVE_DISPLAY_SPLASH)
        {
            palDataPath_TurnOffSplash();
            palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL);
        }
        m_sSourceDesc.eActiveDisplay = eDATA_PATH_ACTIVE_DISPLAY_TESTPATTERN;
    }
    else
    {
        eExecResult = eEXEC_CODE_FAIL;
    }
    #endif

    return eExecResult;
#endif

    return eEXEC_CODE_PASS;
}

BOOL palDataPath_IsTestPatternEnable(void)
{
#if 0
    if(m_sPalDataPathInfo.eDataPathState == eDATA_PATH_STATE_TPG_DISPLAYED)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
#endif

    return FALSE;
}

eEXEC_CODE palDataPath_Formatter_Gamma_Set(UINT8 ucGamma)   //A70LV_Doulas_0038
{
    eEXEC_CODE eExecResult = eEXEC_CODE_PASS;
    UINT8 ucPictureMode;

    ucPictureMode = halScaler_PictureSettings_Get_NotReturnUser((UINT8)eSOURCE_WINDOW_MAIN);    //A70LV_Doulas_0073 Add

    if(ucPictureMode == eCM_PICTURE_SETTINGS_BLENDING)     //ZU860_Doulas_0101
    {
        ucGamma = palDataMgr_sDDP_Gamma_Transfer(eCM_GAMMA_2_2);//eCM_GAMMA_GRAPHICS;     //2.2
    }

    ucGamma = palDataMgr_sDDP_Gamma_Transfer(ucGamma);
    halFormatter_IMG_GammaLutSet(ucGamma);
#if 0
    switch(ucGamma)		//A70Gen2_Doulas_0003 Modify
    {
        case eCM_GAMMA_FILM:
            halFormatter_IMG_GammaLutSet(DDP442X_GAMMA_FILM);
            break;

        case eCM_GAMMA_VIDEO:
			#if 0//#ifdef CUSTOM_CHRISTIE	//HICC2_Doulas_0073 remove//A70Gen2_Doulas_0045 Add
			if(ucPictureMode == eCM_PICTURE_SETTINGS_ENHANCED)
			{
				halFormatter_IMG_GammaLutSet(DDP442X_GAMMA_1P8);  //Gamma 1.8
			}
			else
			#endif
			{
            	halFormatter_IMG_GammaLutSet(DDP442X_GAMMA_VIDEO);
			}
            break;

        case eCM_GAMMA_GRAPHICS:
            #ifdef PLATFORM_H60_2K  //HICC2_Doulas_0033
            halFormatter_IMG_GammaLutSet(DDP442X_GAMMA_GRAPHIC);
            #elif defined(PLATFORM_H30_4K)
            halFormatter_IMG_GammaLutSet(DDP442X_GAMMA_GRAPHIC);
            #else
            halFormatter_IMG_GammaLutSet(DDP442X_GAMMA_2P2_2);
            #endif
            break;

        case eCM_GAMMA_STANDARD:
            halFormatter_IMG_GammaLutSet(DDP442X_GAMMA_2P2);
            break;

        case eCM_GAMMA_3D:
            halFormatter_IMG_GammaLutSet(DDP442X_GAMMA_2P2);
            break;

        case eCM_GAMMA_BLACKBOARD:
            halFormatter_IMG_GammaLutSet(DDP442X_GAMMA_BLACKBOARD);
            break;

        case eCM_GAMMA_DICOM:
            halFormatter_IMG_GammaLutSet(DDP442X_GAMMA_DICOM);
            break;

        case eCM_GAMMA_1_8:
            halFormatter_IMG_GammaLutSet(DDP442X_GAMMA_1P8);
            break;

        case eCM_GAMMA_1_9:
            //halFormatter_IMG_GammaLutSet(DDP442X_GAMMA_1P9);
            break;

        case eCM_GAMMA_2_0:
            halFormatter_IMG_GammaLutSet(DDP442X_GAMMA_2P0);
            break;

        case eCM_GAMMA_2_1:
            //halFormatter_IMG_GammaLutSet(DDP442X_GAMMA_2P1);
            break;

        case eCM_GAMMA_2_2:
            halFormatter_IMG_GammaLutSet(DDP442X_GAMMA_2P2);
            break;

        case eCM_GAMMA_2_3:
            //halFormatter_IMG_GammaLutSet(DDP442X_GAMMA_2P3);
            break;

        case eCM_GAMMA_2_4:
            halFormatter_IMG_GammaLutSet(DDP442X_GAMMA_2P4);
            break;

        case eCM_GAMMA_2_5:
            //halFormatter_IMG_GammaLutSet(DDP442X_GAMMA_2P5);
            break;

        case eCM_GAMMA_2_6:
            halFormatter_IMG_GammaLutSet(DDP442X_GAMMA_2P6);
            break;

		case eCM_GAMMA_CRT:
            #ifdef PLATFORM_H30_4K
            halFormatter_IMG_GammaLutSet(DDP442X_GAMMA_CRT);
            #else
			halFormatter_IMG_GammaLutSet(DDP442X_GAMMA_CRT);
			#endif
			break;

		case eCM_GAMMA_ENHANCED:
			//halFormatter_IMG_GammaLutSet(DDP442X_GAMMA_ENHANCED);
			halFormatter_IMG_GammaLutSet(DDP442X_GAMMA_1P8);
			break;

		case eCM_GAMMA_HDR_STANDARD:
			halFormatter_IMG_GammaLutSet(DDP442X_GAMMA_HDR_STANDARD);
			break;

		case eCM_GAMMA_HDR_Film_HC:
            #ifdef PLATFORM_H30_4K
            halFormatter_IMG_GammaLutSet(DDP442X_GAMMA_2P2);
            #else
			halFormatter_IMG_GammaLutSet(DDP442X_GAMMA_HDR_FILM);
			#endif
			break;

		case eCM_GAMMA_HDR_Bright_HC:
            #ifdef PLATFORM_H30_4K
            halFormatter_IMG_GammaLutSet(DDP442X_GAMMA_2P2);
            #else
			halFormatter_IMG_GammaLutSet(DDP442X_GAMMA_HDR_BRIGHT);
			#endif
			break;

		case eCM_GAMMA_HDR_Detail_HC:
			halFormatter_IMG_GammaLutSet(DDP442X_GAMMA_HDR_DETAIL);
			break;

		case eCM_GAMMA_HDR_Standard_Optoma:
			halFormatter_IMG_GammaLutSet(DDP442X_GAMMA_HDR_STANDARD);
			break;

		case eCM_GAMMA_HDR_Film_Optoma:
			halFormatter_IMG_GammaLutSet(DDP442X_GAMMA_HDR_FILM);
			break;

		case eCM_GAMMA_HDR_Bright_Optoma:
			halFormatter_IMG_GammaLutSet(DDP442X_GAMMA_HDR_BRIGHT);
			break;

		case eCM_GAMMA_HDR_Detail_Optoma:
			halFormatter_IMG_GammaLutSet(DDP442X_GAMMA_HDR_DETAIL);
			break;

        default:
            halFormatter_IMG_GammaLutSet(DDP442X_GAMMA_2P2);
            break;
	//G100_Clare_0014, modify, <<<
    }
#endif /* 0 */

    return eExecResult;
}

BOOL palDataPath_InputSourceIs3DFormat_Get(void)    //A70LV_Doulas_0154 modify
{
    BOOL bValue = FALSE;
    switch(halScaler_Input_3D_Format_Get())
    {
        case eINPUT_3D_TYPE_FRAMEPACKING:
        case eINPUT_3D_TYPE_TOPANDBOTTOM:
        case eINPUT_3D_TYPE_SIDEBYSIDE:
        //case eINPUT_3D_TYPE_FRAME_SEQUENTIAL:       //ZU860_Doulas_0049 remove
            bValue = TRUE;
            break;

        default:
            break;
    }
    return bValue;
}

eEXEC_CODE palDataPath_GOTO_MONITOR_SOURCE_Set(void)   //A70LV_Doulas_0044
{
    UINT32 temp;
    eEXEC_CODE eExecResult = eEXEC_CODE_PASS;

    if(palDataPath_MailBox_Send(m_sPalDataPathInfo.xMsgQueue,
                       m_sPalDataPathInfo.xEventGroupHandle,
                       eDATA_PATH_MSG_GOTOSTATE, -1, eDATA_PATH_STATE_MONITOR_SOURCE, &temp, FALSE) == eMAIL_BOX_EXEC_CODE_PASS )
    {
    }
    else
    {
        eExecResult = eEXEC_CODE_FAIL;
    }


    return eExecResult;
}

eEXEC_CODE palDataPath_InputPixelClock_Get(UINT8 *ucValue)   //A70LV_Doulas_0056
{
    eEXEC_CODE eExecResult = eEXEC_CODE_PASS;

    if(palDataPath_IsSourceLock())
    {
        if(m_FrontEndVideoFormat.u8VideoDownScaling)        //A70LV_Doulas_0187 modify
        {
            #if 0
            char aucString[VERSION_STRING_MAX_LENGTH];
            DOUBLE dbVal;
            dbVal = (DOUBLE)m_FrontEndVideoTiming.u32VideoPCLK;
            sprintf(aucString, "%d.%03dMHz",(UINT32)dbVal/1000,(UINT32)dbVal%1000);
            memcpy(ucValue, aucString, strlen(aucString)+1);
            #endif

            char aucString[VERSION_STRING_MAX_LENGTH];
            DOUBLE dbPixelClock;
            UINT16 uiVFreq;
            halScaler_InputVertRefresh2_Get(eSOURCE_WINDOW_MAIN, &uiVFreq);

            //H/V Total from IT6805 , V sync freq from C821
            dbPixelClock = (DOUBLE)m_FrontEndVideoTiming.u16VideoHTotal *      //A35G2_CDS_Simon_0052
                    (DOUBLE)m_FrontEndVideoTiming.u16VideoVTotal *
                    (DOUBLE)uiVFreq / 100;
            sprintf(aucString, "%d.%03dMHz",(UINT32)dbPixelClock/1000000,(UINT32)dbPixelClock/1000%1000);
            memcpy(ucValue, aucString, strlen(aucString)+1);
        }
        else
        {
            halScaler_InputPixelClock_Get(eSOURCE_WINDOW_MAIN,ucValue);
        }
    }
    else
    {
        *ucValue = '-';
        *(ucValue+1) = '\0';
    }

    return eExecResult;
}

eEXEC_CODE palDataPath_InputSignalFormat_Get(UINT8 *ucValue)
{
    eEXEC_CODE eExecResult = eEXEC_CODE_PASS;
    char aucString[VERSION_STRING_MAX_LENGTH];

    if(palDataPath_IsSourceLock())
    {
        if(m_FrontEndVideoFormat.u8VideoDownScaling)        //A70LV_Doulas_0187 modify
        {
            if(m_sSourceDesc.eConnector == eCM_SOURCE_VGA)
                sprintf(aucString, "Analog");
            else
                sprintf(aucString, "Digital");
            memcpy(ucValue, aucString, strlen(aucString)+1);
        }
        else
        {
            //halScaler_InputSignalFormat_Get(eSOURCE_WINDOW_MAIN,ucValue);     //A70LV_Doulas_0318 Modify
            UINT8 ucColorSpace = 0;
            halScaler_FrontEndColorSpace_Get(eSOURCE_WINDOW_MAIN,&ucColorSpace);
            if((ucColorSpace == eCOLOR_FORMAT_RGB) ||
               (ucColorSpace == eCOLOR_FORMAT_RGB_LIMIT))
            {
                if(m_sSourceDesc.eConnector == eCM_SOURCE_VGA)
                    sprintf(aucString, "Analog");
                else
                    sprintf(aucString, "Digital");
                memcpy(ucValue, aucString, strlen(aucString)+1);
            }
            else
            {
                halScaler_InputSignalFormat_Get(eSOURCE_WINDOW_MAIN,ucValue);
            }
        }
    }
    else
    {
        *ucValue = '-';
        *(ucValue+1) = '\0';
    }

    return eExecResult;
}

eEXEC_CODE palDataPath_InputResoultion_Get(UINT8 *ucValue)
{
    eEXEC_CODE eExecResult = eEXEC_CODE_PASS;

    if(palDataPath_IsSourceLock())
    {
        if(m_FrontEndVideoFormat.u8VideoDownScaling)        //A70LV_Doulas_0187 modify
        {
            char aucString[VERSION_STRING_MAX_LENGTH];
            sprintf(aucString, "%d x %d",m_FrontEndVideoTiming.u16VideoHActive ,m_FrontEndVideoTiming.u16VideoVActive);
            memcpy(ucValue, aucString, strlen(aucString)+1);
        }
        else
        {
            halScaler_InputResolution_Get(eSOURCE_WINDOW_MAIN,ucValue);
        }
    }
    else
    {
        *ucValue = '-';
        *(ucValue+1) = '\0';
    }

    return eExecResult;
}

eEXEC_CODE palDataPath_InputHorzRefresh_Get(UINT8 *ucValue)
{
    eEXEC_CODE eExecResult = eEXEC_CODE_PASS;

    if(palDataPath_IsSourceLock())
    {
        if(m_FrontEndVideoFormat.u8VideoDownScaling)        //A70LV_Doulas_0187 modify
        {
            #if 0
            char aucString[VERSION_STRING_MAX_LENGTH];
            DOUBLE dbVal;
            dbVal = (DOUBLE)m_FrontEndVideoTiming.u32VideoPCLK *1000 / (DOUBLE)m_FrontEndVideoTiming.u16VideoHTotal;
            sprintf(aucString, "%d.%03dkHz",(UINT32)dbVal/1000,(UINT32)dbVal%1000);
            memcpy(ucValue, aucString, strlen(aucString)+1);
            #endif

            char aucString[VERSION_STRING_MAX_LENGTH];
            DOUBLE dbHRefresh;
            UINT16 uiVFreq;
            halScaler_InputVertRefresh2_Get(eSOURCE_WINDOW_MAIN, &uiVFreq);

            dbHRefresh = (DOUBLE)m_FrontEndVideoTiming.u16VideoVTotal *       //A35G2_CDS_Simon_0052
                         (DOUBLE)uiVFreq / 100;

            sprintf(aucString, "%d.%03dkHz",(UINT32)dbHRefresh/1000,(UINT32)dbHRefresh%1000);
            memcpy(ucValue, aucString, strlen(aucString)+1);
        }
        else
        {
            halScaler_InputHorzRefresh_Get(eSOURCE_WINDOW_MAIN,ucValue);
        }
    }
    else
    {
        *ucValue = '-';
        *(ucValue+1) = '\0';
    }

    return eExecResult;
}

eEXEC_CODE palDataPath_InputVertRefresh_Get(UINT8 *ucValue)
{
    eEXEC_CODE eExecResult = eEXEC_CODE_PASS;

    if(palDataPath_IsSourceLock())
    {
        #if 0     //A35G2_CDS_Simon_0052 remove
        if(m_FrontEndVideoFormat.u8VideoDownScaling)        //A70LV_Doulas_0187 modify
        {
            char aucString[VERSION_STRING_MAX_LENGTH];
            DOUBLE dbVal;
            dbVal = (DOUBLE)m_FrontEndVideoTiming.u32VideoPCLK *1000 / (DOUBLE)m_FrontEndVideoTiming.u16VideoHTotal / (DOUBLE)m_FrontEndVideoTiming.u16VideoVTotal * 100;
            sprintf(aucString, "%d.%02dHz",(UINT32)dbVal/100,(UINT32)dbVal%100);
            memcpy(ucValue, aucString, strlen(aucString)+1);
        }
        else
        #endif
        {
            halScaler_InputVertRefresh_Get(eSOURCE_WINDOW_MAIN,ucValue);
        }
    }
    else
    {
        *ucValue = '-';
        *(ucValue+1) = '\0';
    }

    return eExecResult;
}

eEXEC_CODE palDataPath_InputSyncType_Get(UINT8 *ucValue)
{
    eEXEC_CODE eExecResult = eEXEC_CODE_PASS;
    char aucString[VERSION_STRING_MAX_LENGTH];

    if(palDataPath_IsSourceLock())
    {
        if(m_sSourceDesc.eConnector == eCM_SOURCE_VGA)
        {
            //check sync "Sync on Green" or "Separate"
            if(m_FrontEndVideoFormat.u8VideoVGASyncType == eVGA_SYNC_TYPE_SOG)  //A70LV_Doulas_0109 modify
            {
                sprintf(aucString, "Sync on Green");
                memcpy(ucValue, aucString, strlen(aucString)+1);
            }
            else
            {
                sprintf(aucString, "Separate");
                memcpy(ucValue, aucString, strlen(aucString)+1);
            }
        }
        else
        {
            sprintf(aucString, "Separate");
            memcpy(ucValue, aucString, strlen(aucString)+1);
        }
    }
    else
    {
        *ucValue = '-';
        *(ucValue+1) = '\0';
    }

    return eExecResult;
}

eEXEC_CODE palDataPath_InputAspectRatio_Get(UINT8 *ucValue)
{
    eEXEC_CODE eExecResult = eEXEC_CODE_PASS;

    if(palDataPath_IsSourceLock())
    {
        halScaler_InputAspectRatio_Get(eSOURCE_WINDOW_MAIN,ucValue);
    }
    else
    {
        *ucValue = '-';
        *(ucValue+1) = '\0';
    }

    return eExecResult;
}

eEXEC_CODE palDataPath_InputColorSpace_Get(UINT8 *ucValue)
{
    eEXEC_CODE eExecResult = eEXEC_CODE_PASS;

    if(palDataPath_IsSourceLock())
    {
        halScaler_FrontEndColorSpaceInfo_Get(eSOURCE_WINDOW_MAIN,ucValue);     //A70LV_Doulas_0109 Modify
    }
    else
    {
        *ucValue = '-';
        *(ucValue+1) = '\0';
    }

    return eExecResult;
}

eEXEC_CODE palDataPath_FrontEndVideoInfo_Set(void)      //A70LV_Doulas_0112 //A70LV_Doulas_0109 modify//A70LV_Doulas_0076 Add
{
    eEXEC_CODE eExecResult = eEXEC_CODE_PASS;
    UINT8 ucColorSpace;
    UINT8 ucVideoYUV;
    UINT8 ucCS; //G100_Larry_0020

    //Color space
    if(m_sSourceDesc.eConnector == eCM_SOURCE_VGA)
    {
        halScaler_FrontEndVideoYUV_Set(eSOURCE_WINDOW_MAIN,eVIDEO_YUV_UNKNOW);
        if(m_FrontEndVideoFormat.u8VideoVGASyncType == eVGA_SYNC_TYPE_SOG)
            ucColorSpace = eCOLOR_FORMAT_444;
        else
            ucColorSpace = eCOLOR_FORMAT_RGB;
        halScaler_VGA_SYNC_TYPE_Set(eSOURCE_WINDOW_MAIN,m_FrontEndVideoFormat.u8VideoVGASyncType);

        palDataMgr_Data_Access(edcCOLOR_SPACE, edaREAD, (void*)&ucCS); //G100_Larry_0020
        halScaler_ColorSpace_Set(eSOURCE_WINDOW_MAIN, ucCS);           //G100_Larry_0020

        halScaler_FrontEndColorSpace_Set(eSOURCE_WINDOW_MAIN,ucColorSpace);
    }
    else if(m_sSourceDesc.eConnector == eCM_SOURCE_3GSDI)
    {
        halScaler_FrontEndVideoYUV_Set(eSOURCE_WINDOW_MAIN,eVIDEO_YUV_UNKNOW);
        ucColorSpace = eCOLOR_FORMAT_422;//eCOLOR_FORMAT_RGB;       A70LV_Doulas_0091 modify

        palDataMgr_Data_Access(edcCOLOR_SPACE, edaREAD, (void*)&ucCS); //G100_Larry_0020
        halScaler_ColorSpace_Set(eSOURCE_WINDOW_MAIN, ucCS);           //G100_Larry_0020

        halScaler_FrontEndColorSpace_Set(eSOURCE_WINDOW_MAIN,ucColorSpace);
    }
    else
    {
        if(m_FrontEndVideoFormat.u8VideoAVIInfoDet == TRUE)
        {
            switch(m_FrontEndVideoFormat.u8VideoColorYUV)
            {
                case eVIDEO_YUV_REC601:
                    ucVideoYUV = eVIDEO_YUV_REC601;
                    break;

                case eVIDEO_YUV_REC709:
                    ucVideoYUV = eVIDEO_YUV_REC709;
                    break;

                case eVIDEO_YUV_EXTENDED_COLORIMETRY:   //ZU860_Doulas_0097
                    ucVideoYUV = eVIDEO_YUV_REC709;
                    break;

                default:
                    ucVideoYUV = eVIDEO_YUV_UNKNOW;
                    m_FrontEndVideoFormat.u8VideoColorYUV = eVIDEO_YUV_UNKNOW;
                    break;
            }
            halScaler_FrontEndVideoYUV_Set(eSOURCE_WINDOW_MAIN,ucVideoYUV);

            switch(m_FrontEndVideoFormat.u8VideoColorSpace)
            {
                case eVIDEO_COLORSPACE_RGB:
                    if(m_FrontEndVideoFormat.u8VideoDynamicRange == eVIDEO_COLORRANGE_FULL)
                        ucColorSpace = eCOLOR_FORMAT_RGB;
                    else
                        ucColorSpace = eCOLOR_FORMAT_RGB_LIMIT;
                    break;

                case eVIDEO_COLORSPACE_Y422:
                    #if 0
					if(m_sSourceDesc.eConnector == eCM_SOURCE_DVI)	//G100_Doulas_0003 modify
					{
						ucColorSpace = eCOLOR_FORMAT_444;
					}
					else
					#endif
					{
                    	ucColorSpace = eCOLOR_FORMAT_422;
					}
                    break;

                 case eVIDEO_COLORSPACE_Y444:
                    ucColorSpace = eCOLOR_FORMAT_444;
                    break;

                case eVIDEO_COLORSPACE_Y420:
                    ucColorSpace = eCOLOR_FORMAT_422;
                    break;

                default:
                    ucColorSpace = eCOLOR_FORMAT_RGB;
                    break;
            }

            palDataMgr_Data_Access(edcCOLOR_SPACE, edaREAD, (void*)&ucCS); //G100_Larry_0020
            halScaler_ColorSpace_Set(eSOURCE_WINDOW_MAIN, ucCS);           //G100_Larry_0020

            halScaler_FrontEndColorSpace_Set(eSOURCE_WINDOW_MAIN,ucColorSpace);
        }
        else
        {
            halScaler_FrontEndVideoYUV_Set(eSOURCE_WINDOW_MAIN,eVIDEO_YUV_UNKNOW);
            ucColorSpace = eCOLOR_FORMAT_RGB;

            palDataMgr_Data_Access(edcCOLOR_SPACE, edaREAD, (void*)&ucCS); //G100_Larry_0020
            halScaler_ColorSpace_Set(eSOURCE_WINDOW_MAIN, ucCS);           //G100_Larry_0020

            halScaler_FrontEndColorSpace_Set(eSOURCE_WINDOW_MAIN,ucColorSpace);
        }
    }

    //Interlaced
    halScaler_FrontEndScanMode_Set(eSOURCE_WINDOW_MAIN,m_FrontEndVideoFormat.u8VideoScanning);

    #ifndef SCALER_C341  //H30K_Doulas_0011
    if(m_FrontEndVideoFormat.u8VideoDownScaling)    //A70LV_Doulas_0176 Add
    #endif
    {
        UINT8 ucVal[eFE_MSG_VIDEO_TIMING_MAIN_SZ];
        halFrontEndCtrl_Main_Timing_Get(ucVal);
        m_FrontEndVideoTiming.u32VideoPCLK = ucVal[0]+ (ucVal[1]<<8) + (ucVal[2]<<16) +(ucVal[3]<<24);
        m_FrontEndVideoTiming.u16VideoHTotal = ucVal[4]+ (ucVal[5]<<8);
        m_FrontEndVideoTiming.u16VideoHActive = ucVal[6]+ (ucVal[7]<<8);
        m_FrontEndVideoTiming.u16VideoVTotal = ucVal[8]+ (ucVal[9]<<8);
        m_FrontEndVideoTiming.u16VideoVActive = ucVal[10]+ (ucVal[11]<<8);
        m_FrontEndVideoTiming.u16VideoVRate =  ucVal[12]+ (ucVal[13]<<8);
        palDataPath_FrontEnd_Timing_Set(eSOURCE_WINDOW_MAIN,m_FrontEndVideoTiming); //H30K_Doulas_0011
        LOG_MSG(db_APP_DATAPATH, "PCLK    =0x%x \r\n",m_FrontEndVideoTiming.u32VideoPCLK);
        LOG_MSG(db_APP_DATAPATH, "H-total =%d \r\n",m_FrontEndVideoTiming.u16VideoHTotal);
        LOG_MSG(db_APP_DATAPATH, "V-total =%d \r\n",m_FrontEndVideoTiming.u16VideoVTotal);
        LOG_MSG(db_APP_DATAPATH, "H-Activ =%d \r\n",m_FrontEndVideoTiming.u16VideoHActive);
        LOG_MSG(db_APP_DATAPATH, "V-Activ =%d \r\n",m_FrontEndVideoTiming.u16VideoVActive);
        LOG_MSG(db_APP_DATAPATH, "V-Freq  =%d \r\n",m_FrontEndVideoTiming.u16VideoVRate);
    }
    return eExecResult;
}

BOOL palDataPath_Main_Or_Sub_SourceMonitor(void)       //A70LV_Doulas_0101
{
    if(palDataPath_IsSourceMonitor() == TRUE)
    {
        return TRUE;
    }
    else
    {
        if(halScaler_PIP_PBP_Enable_Get())    //PIP on  //G100_Owen_0012
        {
            if(palDataPath_IsSourceMonitorSub() == TRUE)
            {
                return TRUE;
            }
        }
    }
    return FALSE;
}

eEXEC_CODE palDataPath_InitVGA_SyncThreshold(void)   //A70LV_Doulas_0115
{
    UINT8 ucValue;
    ucValue = halScaler_SyncThreshold_Get();
    halFrontEndCtrl_VGA_Sync_Threshold_Set(&ucValue);
    //palDataMgr_Data_Access(edcSYNC_THRESHOLD, edaWRITE_RAM_ONLY_WITH_ACTION, &ucValue);
    return eEXEC_CODE_PASS;
}

eEXEC_CODE palDataPath_Init_FrontEnd_PIP_Setting(void)   //A70LV_Doulas_0120
{
    UINT8 ucValue;
    ucValue = halScaler_PIP_PBP_Enable_Get();
    halFrontEndCtrl_PIP_Enable_Set(&ucValue);

    if(ucValue)
    {
        UINT8 ucSubSource;
        palDataMgr_Data_Access(edcSUB_INPUT, edaREAD, &ucSubSource);
        halFrontEndCtrl_SubInput_Set(&ucSubSource);
    }

    palDataMgr_UI_EventSend(edcUI_EVENT_PANEL_CHANGE, palSystem_PanelID_Get(), NULL);
    halAdvWarpVstartOffsetSet(palSystem_PanelID_Get());

    return eEXEC_CODE_PASS;
}

eEXEC_CODE palDataPath_AutoDisableMenuTransparency(void)   //A70LV_Doulas_0122 auto dsiable Menu Transparency //A70LV_Larry_0422 modify
{
    if(palDataPath_OSD_MenuTransparencyEnableGet() == TRUE) //(GuiCb.fpGui_OSD_MenuTransparencyEnableGetCb() == TRUE)
    {
        //LOG_MSG(db_DV_SCALER, "Transparency_3 %d\r\n");
        palDataPath_MenuTransparencyEnableSet(FALSE); //GuiCb.fpGui_OSD_MenuTransparencyEnableSetCb(FALSE);
    }
    return eEXEC_CODE_PASS;
}

UINT8 palDataPath_Input3D_FormatGet(void)   //A70LV_Doulas_0154
{
    return m_FrontEndVideoFormat.u8Video3DFormat;
}

void palDataPath_Input3D_PanelSet(void)       //A70LV_Doulas_0159
{
    UINT8 ucInput3D_ConfigOld;
    ucInput3D_ConfigOld = halScaler_Input_3D_Format_Config_Get();

    LOG_MSG(db_HAL_RESERVED19, "palDataPath_Input3D_PanelSet (%d)(%d)\n", ucInput3D_ConfigOld, m_FrontEndVideoFormat.u8Video3DFormat);

    switch(m_FrontEndVideoFormat.u8Video3DFormat)
    {
        case eVIDEO_3D_FORMAT_FP:
            halScaler_Input_3D_Format_Set(eINPUT_3D_TYPE_FRAMEPACKING);
            break;

        case eVIDEO_3D_FORMAT_SS:
            halScaler_Input_3D_Format_Set(eINPUT_3D_TYPE_SIDEBYSIDE);
            break;

        case eVIDEO_3D_FORMAT_TB:
            halScaler_Input_3D_Format_Set(eINPUT_3D_TYPE_TOPANDBOTTOM);
            break;

        default:
            if(m_FrontEndVideoFormat.u8VideoDualPixelMode == TRUE)
            {
                halScaler_Input_3D_Format_Set(eINPUT_3D_TYPE_FRAME_SEQUENTIAL);
            }
            else
            {
                halScaler_Input_3D_Format_Set(eINPUT_3D_TYPE_OFF);
            }
            break;
    }

    if((m_sPalDataPathInfo.ePanelTimingId == ePANEL_ID_1080P_120HZ) ||
       (m_sPalDataPathInfo.ePanelTimingId == ePANEL_ID_WUXGA_120HZ))    //A70LV_Doulas_0377 Modify
    {
        if((halScaler_Input_3D_Format_Config_Get() != eINPUT_3D_TYPE_OFF) &&
           (halScaler_Input_3D_Format_Config_Get() != ucInput3D_ConfigOld))
        {
            palDataPath_ResyncSub();
            halScaler_3D_InputPort_Set();
            palDataMgr_UI_EventSend(edcUI_EVENT_PANEL_CHANGE, m_sPalDataPathInfo.ePanelTimingId, NULL);
            halAdvWarpVstartOffsetSet(m_sPalDataPathInfo.ePanelTimingId);
        }
    }
    #if 0    //A70LV_Doulas_0367 remove
    if((halScaler_Input_3D_Format_Config_Get() != eINPUT_3D_TYPE_OFF) &&
       (m_sPalDataPathInfo.ePanelTimingId != ePANEL_ID_1080P_120HZ))
    {
        palSystem_ChangePanelID(ePANEL_ID_1080P_120HZ);
        palDataMgr_DisplayModeChecking();         //ZU860_Doulas_0045
    }
    else if((halScaler_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_OFF) &&
            (m_sPalDataPathInfo.ePanelTimingId == ePANEL_ID_1080P_120HZ))
    {
        #if defined (DMD_DHD)
        palSystem_ChangePanelID(ePANEL_ID_1080P_60HZ);
        #elif defined (DMD_WUXGA)
        palSystem_ChangePanelID(ePANEL_ID_WUXGA_60HZ);
        #else
        palSystem_ChangePanelID(ePANEL_ID_1080P_60HZ);
        #endif
        palDataMgr_DisplayModeChecking();         //ZU860_Doulas_0045
        halFormatter_3D_3DModeSet(FALSE);                       //ZU860_Doulas_0104
        halScaler_FrameDelay_Set(FRAME_DELAY_DEFAULT_VALUE);    //ZU860_Doulas_0104
        palDataMgr_Formatter_Parameter_Set(FALSE);                   //ZU860_Doulas_0104
    }
    #endif
}

void palDataPath_Input3D_PanelSet2(void)       //A70LV_Doulas_0367
{
    UINT16 uiV_Freq;
    UINT8  ucDisplayMode = 0; //H30K_Doulas_0081
    palDataMgr_Data_Access(edcPICTURE_SETTINGS, edaREAD, &ucDisplayMode); //H30K_Doulas_0081
    halScaler_InputVertRefresh2_Get(eSOURCE_WINDOW_MAIN,&uiV_Freq); //H30K_Doulas_0009

    LOG_MSG(db_APP_DATAPATH, "palDataPath_Input3D_PanelSet2 %d \r\n",uiV_Freq); //H30K_Doulas_0009
#ifdef CURSOR_FIXTURE
    if((halScaler_Input_3D_Format_Config_Get() != eINPUT_3D_TYPE_OFF) &&
        (m_sPalDataPathInfo.ePanelTimingId != ePANEL_ID_WUXGA_120HZ) &&    //A70LV_Doulas_0377 Add
       (m_sPalDataPathInfo.ePanelTimingId != ePANEL_ID_1080P_120HZ))
#else
#ifdef CONFIG_4K_DISPLAY //H30K_Doulas_0009
    if(//(halScaler_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_OFF) &&
        (halScaler_IS_3D_Enable() == ets_OFF) &&
        (m_sPalDataPathInfo.ePanelTimingId != ePANEL_ID_WUXGA_240HZ) &&
        (m_sPalDataPathInfo.ePanelTimingId != ePANEL_ID_1080P_240HZ) &&
        ((uiV_Freq > 9500) && (uiV_Freq < 24500)))  // > 95hz
    {
        ucDisplayMode = eCM_PICTURE_SETTINGS_2DHIGHSPEED; //H30K_Doulas_0081
        palDataMgr_Data_Access(edcPICTURE_SETTINGS, edaWRITE_THROUGH_WITH_ACTION, &ucDisplayMode); //H30K_Doulas_0081
        #if defined (DMD_DHD)
        palSystem_ChangePanelID(ePANEL_ID_1080P_240HZ, NULL);
        #elif defined (DMD_WUXGA)
        palSystem_ChangePanelID(ePANEL_ID_WUXGA_240HZ, NULL);
        #else
        palSystem_ChangePanelID(ePANEL_ID_WUXGA_240HZ, NULL);
        #endif
        //palDataMgr_DisplayModeChecking();
        palDataPath_Panel_ChangeCheck();
    }
    else
#endif
#ifdef CONFIG_4K_DISPLAY //H30K_Doulas_0002 Modify
    if(//((halScaler_Input_3D_Format_Config_Get() != eINPUT_3D_TYPE_OFF) || (halScaler_IS_3D_Enable() == ets_ON)) && //H30K_Doulas_0009
        (halScaler_IS_3D_Enable() == ets_ON) &&                           //H30K_Doulas_0009
        (m_sPalDataPathInfo.ePanelTimingId != ePANEL_ID_WUXGA_120HZ) &&
       (m_sPalDataPathInfo.ePanelTimingId != ePANEL_ID_1080P_120HZ)) //3D input timing and FS timing(50/60/120hz...) change 3D panel
#else
    if((halScaler_Input_3D_Format_Config_Get() != eINPUT_3D_TYPE_OFF) &&
        (m_sPalDataPathInfo.ePanelTimingId != ePANEL_ID_WUXGA_120HZ) &&    //A70LV_Doulas_0377 Add
       (m_sPalDataPathInfo.ePanelTimingId != ePANEL_ID_1080P_120HZ))
#endif
#endif
    {
        if(palGeo_ApLinkFlag_Get() == FALSE)
        {
            palDataPath_SetPanelChangeState(ets_ON);
        }
#ifdef NOT_USING_SHUTTER_COVER_TRANSIENT	//A70Gen2_Doulas_0039
#else
        palDataMgr_Data_Access(edcPICTURE_MUTE, edaWRITE_RAM_ONLY_WITH_ACTION, &m_ucPanelChangeing);
#endif
        ucDisplayMode = eCM_PICTURE_SETTINGS_3D; //H30K_Doulas_0081
        palDataMgr_Data_Access(edcPICTURE_SETTINGS, edaWRITE_THROUGH_WITH_ACTION, &ucDisplayMode); //H30K_Doulas_0081
    #ifdef SUPPORT_WUXGA_120HZ_3D_PANEL    //A70LV_Doulas_0377 Modify
        #if defined (DMD_DHD)
        palSystem_ChangePanelID(ePANEL_ID_1080P_120HZ, NULL);
        #elif defined (DMD_WUXGA)
        palSystem_ChangePanelID(ePANEL_ID_WUXGA_120HZ, NULL);
        #else
        palSystem_ChangePanelID(ePANEL_ID_1080P_120HZ, NULL);
        #endif
    #else
        #if defined (DMD_DHD)   //H30K_Doulas_0001 Modify
        palSystem_ChangePanelID(ePANEL_ID_1080P_120HZ, NULL);
        #elif defined (DMD_WUXGA)
        palSystem_ChangePanelID(ePANEL_ID_WUXGA_120HZ, NULL);
        #else
        palSystem_ChangePanelID(ePANEL_ID_1080P_120HZ, NULL);
        #endif
    #endif
        //palDataMgr_DisplayModeChecking();         //ZU860_Doulas_0045
        palDataPath_Panel_ChangeCheck();
    }
#ifdef CURSOR_FIXTURE
    else if((halScaler_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_OFF) &&
                (m_sPalDataPathInfo.ePanelTimingId == ePANEL_ID_1080P_120HZ|| m_sPalDataPathInfo.ePanelTimingId == ePANEL_ID_WUXGA_120HZ))
#else
#ifdef CONFIG_4K_DISPLAY //H30K_Doulas_0002 Modify
    //else if(((halScaler_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_OFF) && (halScaler_IS_3D_Enable() != ets_ON)) &&
    //        (m_sPalDataPathInfo.ePanelTimingId == ePANEL_ID_1080P_120HZ || m_sPalDataPathInfo.ePanelTimingId == ePANEL_ID_WUXGA_120HZ || ((m_sPalDataPathInfo.ePanelTimingId == ePANEL_ID_WUXGA_240HZ) && (uiV_Freq < 9500)) )) //H30K_Doulas_0009
    else if(((m_sPalDataPathInfo.ePanelTimingId == ePANEL_ID_WUXGA_240HZ) && (uiV_Freq < 9500)) ||
             ((halScaler_IS_3D_Enable() != ets_ON) && (m_sPalDataPathInfo.ePanelTimingId == PANEL_3D_OUTPUT)))   //H30K_Doulas_0024
#else
    else if((halScaler_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_OFF) &&
            (m_sPalDataPathInfo.ePanelTimingId == ePANEL_ID_1080P_120HZ || m_sPalDataPathInfo.ePanelTimingId == ePANEL_ID_WUXGA_120HZ))
#endif
#endif
    {
        if(palGeo_ApLinkFlag_Get() == FALSE)
        {
            palDataPath_SetPanelChangeState(ets_ON);
        }
#ifdef NOT_USING_SHUTTER_COVER_TRANSIENT	//A70Gen2_Doulas_0039
#else
        palDataMgr_Data_Access(edcPICTURE_MUTE, edaWRITE_RAM_ONLY_WITH_ACTION, &m_ucPanelChangeing);
#endif
        //UINT8 ucData = 0;	//G100_Doulas_0064
        if((ucDisplayMode == eCM_PICTURE_SETTINGS_2DHIGHSPEED) ||
           (ucDisplayMode == eCM_PICTURE_SETTINGS_3D) ||
           (ucDisplayMode == eCM_PICTURE_SETTINGS_3D_PASSIVE ) ||
           (ucDisplayMode == eCM_PICTURE_SETTINGS_HLG ) ||
           (ucDisplayMode == eCM_PICTURE_SETTINGS_HDR)) //H30K_Doulas_0081
        {
        #ifdef ENABLE_GO_BACK_TO_LAST_MODE
            ucDisplayMode = palDataMgr_Last_Display_Mode_Get();
        #else //ENABLE_GO_BACK_TO_LAST_MODE
		#ifdef OE_JIG
            ucDisplayMode = eCM_PICTURE_SETTINGS_BRIGHT;
		#else
            ucDisplayMode = palDataMgr_DefaultDisplayModeGet();
        #endif
        #endif //ENABLE_GO_BACK_TO_LAST_MODE
            palDataMgr_Data_Access(edcPICTURE_SETTINGS, edaWRITE_THROUGH_WITH_ACTION, &ucDisplayMode);
        }
        palSystem_ChangePanelID(PANEL_2D_OUTPUT, NULL);

        //palDataMgr_DisplayModeChecking();         //ZU860_Doulas_0045
        halFormatter_3D_3DModeSet(FALSE);                       //ZU860_Doulas_0104
        halScaler_FrameDelay_Set(FRAME_DELAY_DEFAULT_VALUE);    //ZU860_Doulas_0104
        //palDataMgr_Formatter_Parameter_Set(TRUE);//(FALSE);                   //HICC2_Doulas_0085//ZU860_Doulas_0104

        //halFormatter_AspectRatio_Get(&ucData);	//G100_Doulas_0064 remove
        //if(ucData != DDP_ASPECT_NATIVE)
        //{
        //    ucData = DDP_ASPECT_NATIVE;
        //    halFormatter_AspectRatio_Set(&ucData);
        //}
        palDataPath_Panel_ChangeCheck();
    }
}

UINT8 palDataPath_Input3D_Timing_Get(void)               //A70LV_Doulas_0196
{
    UINT8  uc3DTiming = e3D_Customer_Timing;

    halScaler_Input3D_Timing_Get(eSOURCE_WINDOW_MAIN, &uc3DTiming);

    return uc3DTiming;
}

eEXEC_CODE palDataPath_ModeAdjusmenttEnableSetting(void)   //A70LV_Doulas_0209
{
    eEXEC_CODE eExecResult = eEXEC_CODE_PASS;

    halScaler_ModeAdjusmenttEnableSetting();

    return eExecResult;
}

eEXEC_CODE palDataPath_ModeAdjusmenttDisableSetting(void)   //A70LV_Doulas_0209
{
    eEXEC_CODE eExecResult = eEXEC_CODE_PASS;

    halScaler_ModeAdjusmenttDisableSetting();

    return eExecResult;
}

eEXEC_CODE palDataPath_FreezeChecking(UINT8 ucCH)   //A70LV_Doulas_0223 Freeze check
{
    BOOL bFreezeEn = FALSE;
    eEXEC_CODE eExecResult = eEXEC_CODE_PASS;
    halScaler_Freeze_Get(ucCH,&bFreezeEn);
    if(bFreezeEn == TRUE)
    {
        UINT8 ucValue = ets_OFF;
        eExecResult = palDataMgr_Data_Access(edcIMAGE_FREEZE, edaWRITE_THROUGH_WITH_ACTION, &ucValue);
    }
    return eExecResult;
}

eEXEC_CODE palDataPath_LogoCapture(void)
{
    eEXEC_CODE eExecResult = eEXEC_CODE_FAIL;

    if(halScaler_CaptureImage() == eHAL_SCALER_EXEC_CODE_PASS)
    {
        eExecResult = eEXEC_CODE_PASS;
    }

    return eExecResult;

}

eEXEC_CODE palDataPath_ScreenSaveToImage(void)
{
    eEXEC_CODE eExecResult = eEXEC_CODE_FAIL;

    if(halScaler_ScreenSaveToImage() == eHAL_SCALER_EXEC_CODE_PASS)
    {
        eExecResult = eEXEC_CODE_PASS;
    }

    return eExecResult;
}


BOOL palDataPath_AutoCheckColorSpaceChange(void)        //A70LV_Doulas_0283
{
    eRESULT eResult = rcINVALID;
    UINT8 ucVideoFormat[eFE_MSG_VIDEO_FORMAT_MAIN_SZ] = {0};  //ZU860_Doulas_0127 modify
    UINT8 ucEOTF;
	eRESULT eResult2 = rcINVALID;

#if 0  //A35G2_CDS_Simon_0026 remove
    if(palDataPath_BKInput_Status()) //G100_Steven_0028
    {
    	return FALSE;
    }
#endif

    if(m_sSourceDesc.eConnector == eCM_SOURCE_3GSDI || m_sSourceDesc.eConnector == eCM_SOURCE_12GSDI )      //ZU860_Doulas_0126 modify
    {
        return FALSE;
    }

    //if( halScaler_ColorSpace_Get(eSOURCE_WINDOW_MAIN) !=  eCM_COLOR_SPACE_AUTO) //H30K_Doulas_0069 remove
    //{
    //    return FALSE;
    //}

    eResult = halFrontEndCtrl_videoFormat_Get(ucVideoFormat);

    #if 0
    UINT8 ucBackupSecond = 0;
    palDataMgr_Data_Access(edcBACKUPINPUT_SECONDARY_INPUT, edaREAD, &ucBackupSecond);
    if(m_sSourceDesc.eConnector == ucBackupSecond)
    {
        eResult = halFrontEndCtrl_videoFormatSub_Get(ucVideoFormat);
    }
    else
    {
        eResult = halFrontEndCtrl_videoFormat_Get(ucVideoFormat);
    }
    #endif

    if(eResult == rcSUCCESS)    //A70LV_Doulas_0288 modify
    {
        if(m_sSourceDesc.eConnector == eCM_SOURCE_VGA)       //ZU860_Doulas_0126
        {
            if(m_FrontEndVideoFormat.u8VideoVGASyncType != ucVideoFormat[6])
            {
                ucInfoFrame_Change_flag = 1;
                return TRUE;
            }
            else
            {
                return FALSE;
            }
        }

        if(((m_sSourceDesc.eConnector == eCM_SOURCE_HDMI1)||
           (m_sSourceDesc.eConnector == eCM_SOURCE_HDMI2) ||
           (m_sSourceDesc.eConnector == eCM_SOURCE_HDBASET)) &&
           (m_FrontEndVideoFormat.u8VideoHDRType != ucVideoFormat[10]))  //H30K_Doulas_0065 Modify//ZU860_Doulas_0127 HDR checking
        {
            //if(m_FrontEndVideoFormat.u8VideoHDRType != ucVideoFormat[10]) //H30K_Doulas_0065
            {
                LOG_MSG(db_APP_DATAPATH, "HDR change\r\n"); //H30K_Doulas_0007
                ucInfoFrame_Change_flag = 2;
                eResult2 = rcSUCCESS;
            }
        }
        else if(m_FrontEndVideoFormat.u8VideoAVIInfoDet != ucVideoFormat[4])
        {
            LOG_MSG(db_APP_DATAPATH, "AVIInfoDet change\r\n"); //H30K_Doulas_0007
            ucInfoFrame_Change_flag = 3;
            eResult2 = rcSUCCESS;
        }
        else if(m_FrontEndVideoFormat.u8VideoAVIInfoDet == TRUE)
        {
            if(m_FrontEndVideoFormat.u8VideoColorSpace != ucVideoFormat[1])
            {
                LOG_MSG(db_APP_DATAPATH, "VideoColorSpace change\r\n"); //H30K_Doulas_0007
                ucInfoFrame_Change_flag = 4;
                eResult2 = rcSUCCESS;
            }
            else if(m_FrontEndVideoFormat.u8VideoColorSpace == eVIDEO_COLORSPACE_RGB) // RGB
            {
                if(m_FrontEndVideoFormat.u8VideoDynamicRange != ucVideoFormat[2])
                {
                    LOG_MSG(db_APP_DATAPATH, "VideoDynamicRange change\r\n"); //H30K_Doulas_0007
                    ucInfoFrame_Change_flag = 5;
                    eResult2 = rcSUCCESS;
                }
            }
            else //YUV
            {
                if(m_FrontEndVideoFormat.u8VideoColorYUV != ucVideoFormat[7])
                {
                    LOG_MSG(db_APP_DATAPATH, "VideoColorYUV change\r\n"); //H30K_Doulas_0007
                    ucInfoFrame_Change_flag = 6;
                    eResult2 = rcSUCCESS;
                }
            }
        }

        if(m_FrontEndVideoFormat.u8Video3DFormat != ucVideoFormat[5]) //H30K_Doulas_0043 3D format check
        {
            LOG_MSG(db_APP_DATAPATH, "## 3D Format change\r\n");
            ucInfoFrame_Change_flag = 7;
            eResult2 = rcSUCCESS;
        }
    }

    if(eResult2 == rcSUCCESS)
    {
        m_FrontEndVideoFormat.u8VideoColorDepth = ucVideoFormat[0];
        m_FrontEndVideoFormat.u8VideoColorSpace = ucVideoFormat[1];
        m_FrontEndVideoFormat.u8VideoDynamicRange = ucVideoFormat[2];
        m_FrontEndVideoFormat.u8VideoScanning =  ucVideoFormat[3];
        m_FrontEndVideoFormat.u8VideoAVIInfoDet =  ucVideoFormat[4];
        m_FrontEndVideoFormat.u8Video3DFormat =  ucVideoFormat[5];
        m_FrontEndVideoFormat.u8VideoVGASyncType = ucVideoFormat[6];
        m_FrontEndVideoFormat.u8VideoColorYUV    = ucVideoFormat[7];
        m_FrontEndVideoFormat.u8VideoDownScaling = ucVideoFormat[8];
        m_FrontEndVideoFormat.u8VideoDualPixelMode = ucVideoFormat[9];
        m_FrontEndVideoFormat.u8VideoHDRType       = ucVideoFormat[10];
        m_FrontEndVideoFormat.u8VideoExtentedColorimetry = ucVideoFormat[11];
        LOG_MSG(db_APP_DATAPATH, "palDataPath_AutoCheckColorSpaceChange check is ture !\r\n"); //H30K_Doulas_0007
        return TRUE;
    }

	eResult2 = halFrontEndCtrl_HDR_EOTF_STREAM_Get(&ucEOTF);
	if(eResult2 == rcSUCCESS)
	{
		//LOG_MSG(db_APP_DATAPATH, "@@@@@@@@@@@@ ucEOTF = %d,  m_FrontEndVideoFormat.u8VideoHDRType = %d\r\n", ucEOTF, m_FrontEndVideoFormat.u8VideoHDRType); //H30K_Doulas_0007

		if(ucEOTF != m_Video_EOTF_STREAM)
		{
		    LOG_MSG(db_APP_DATAPATH, "Video_EOTF_STREAM change = (%d,%d)\r\n", ucEOTF, m_Video_EOTF_STREAM); //H30K_Doulas_0007
			m_Video_EOTF_STREAM = (EOTF_STREAM)ucEOTF;
			palLANProcSendToLAN(edcDATAPATH_HLG_INFO);
            ucInfoFrame_Change_flag = 8;
			return TRUE;
		}

        #if (SCALER_HDR_ENABLE == TRUE)   //H2PF_Simon_0193
        sHDR_LUMINANCE_INFO sNewHDRLumianceInfo;
        eRESULT eRet = halFrontEndCtrl_HDR_LuminanceInfo_Get((UINT8 *)&sNewHDRLumianceInfo);
        if(memcmp(&sNewHDRLumianceInfo, &m_HDRLuminanceInfo, sizeof(sHDR_LUMINANCE_INFO)) != 0)
        {
            LOG_MSG(db_APP_DATAPATH, "--- New HDR Luminance detected\r\n");
            memcpy(&m_HDRLuminanceInfo, &sNewHDRLumianceInfo, sizeof(sHDR_LUMINANCE_INFO));
            ucInfoFrame_Change_flag = 9;
			return TRUE;
        }
        #endif
	}
    return FALSE;
}

eEXEC_CODE palDataPath_EdgeMaskSet(UINT8 ucEdgeMask)   //ZU860_Doulas_0004
{
    eHAL_SCALER_EXEC_CODE eScalerResult = eHAL_SCALER_EXEC_CODE_FAIL;
    eEXEC_CODE eExecResult = eEXEC_CODE_PASS;

    eScalerResult = halScaler_EdgeMask_Set(ucEdgeMask);
    if(eScalerResult == eHAL_SCALER_EXEC_CODE_PASS)
    {
        //palDataPath_Reconfig_Image();
        #ifndef SCALER_C341 //H30K_Doulas_0028
        palEnvironment_LightSourceBlankingOn_Set(TRUE,3);
        #endif
        halScaler_ConfigureForScaler(eSOURCE_WINDOW_MAIN);      //ZU860_Doulas_0025 modify
    }
    else
        eExecResult = eEXEC_CODE_FAIL;

    return eExecResult;
}

eEXEC_CODE palDataPath_Format_DB_RealBlackSet(void)   //ZU860_Doulas_0006
{
    eEXEC_CODE eExecResult = eEXEC_CODE_PASS;
    UINT8 ucRealBlack = ets_OFF;
    UINT8 ucDynamicBlack = ets_OFF;
    UINT8 ucLightsOutSignalLevel = LIGHTS_OUT_SIGNAL_LEVEL_DEFAULT_VALUE;
    UINT8 ucLightsOutTimer = LIGHTS_OUT_TIMER_X05_DEFAULT_VALUE;
    UINT8 ucContrastEnhancement = ets_OFF;

    palDataMgr_Data_Access(edcREAL_BLACK, edaREAD, (void*)&ucRealBlack);
    palDataMgr_Data_Access(edcDYNAMIC_BLACK, edaREAD, (void*)&ucDynamicBlack);
    palDataMgr_Data_Access(edcLIGHTS_OUT_SIGNAL_LEVEL, edaREAD, (void*)&ucLightsOutSignalLevel);

    if((ucRealBlack == ets_OFF) && (ucDynamicBlack == ets_OFF))
    {
        ucContrastEnhancement = eCONTRAST_ENHANCE_OFF;
    }
    else if(ucRealBlack == ets_ON)
    {
        ucContrastEnhancement = eCONTRAST_ENHANCE_REAL_BLACK;
        palDataMgr_Data_Access(edcLIGHTS_OUT_TIMER_X05, edaREAD, (void*)&ucLightsOutTimer);
        halFormatter_LightsOutTimerSet(ucLightsOutTimer);
    }
    else
    {
        ucContrastEnhancement = eCONTRAST_ENHANCE_DYNAMIC_BLACK;
    }

    eExecResult = palDataPath_Format_ContrastEnhancementSet(ucContrastEnhancement,ucLightsOutSignalLevel);
    return eExecResult;
}

eEXEC_CODE palDataPath_PictureModeChange(UINT8 ucData)    //ZU860_Doulas_0076
{
    eEXEC_CODE eExecResult = eEXEC_CODE_PASS;

    halFormatter_DMD_BGDisplayModeSet(BG_BLK);      //DDP background on
    palImgMgr_WB_OutputEnableSet(0);
    palDataMgr_Data_Access(edcPICTURE_SETTINGS, edaWRITE_THROUGH_WITH_ACTION, &ucData) ;
    //palDataMgr_Format_Normal_Set();                                   //ZU860_Doulas_0123 remove
    palImgMgr_WB_OutputEnableSet(1);
    halFormatter_DMD_BGDisplayModeSet(BG_DISABLE);  //DDP background off
    return eExecResult;
}

eEXEC_CODE palDataPath_ColorDepth_Get(UINT8 *ucValue)    //ZU860_Doulas_0097
{
    eEXEC_CODE eExecResult = eEXEC_CODE_PASS;
    char aucString[24];
    char acColorDepthString[10] = {0};
    char acColorSpaceString[10] = {0};
    UINT8 ucColorSpace = eVIDEO_COLORSPACE_RGB;
    UINT8 ucColorDepth = eCOLOR_DEPTH_8BIT;

    if(palDataPath_IsSourceLock())
    {
        switch(m_sSourceDesc.eConnector)
        {
            case eCM_SOURCE_VGA:     //ZU860_Doulas_0125 modify
            case eCM_SOURCE_3GSDI:
                *ucValue = '-';
                *(ucValue+1) = '\0';
                return eExecResult;

            case eCM_SOURCE_HDMI1:
            case eCM_SOURCE_HDMI2:
            case eCM_SOURCE_DVI:
            case eCM_SOURCE_HDBASET:
            case eCM_SOURCE_DISPLAYPORT: //G100_Steven_0001
                if(m_FrontEndVideoFormat.u8VideoAVIInfoDet == TRUE)
                {
                    switch(m_FrontEndVideoFormat.u8VideoColorSpace)
                    {
                        case eVIDEO_COLORSPACE_RGB:
                            ucColorSpace = eVIDEO_COLORSPACE_RGB;
                            break;

                        case eVIDEO_COLORSPACE_Y422:
                            ucColorSpace = eVIDEO_COLORSPACE_Y422;
                            break;

                        case eVIDEO_COLORSPACE_Y444:
                           ucColorSpace = eVIDEO_COLORSPACE_Y444;
                           break;

                        case eVIDEO_COLORSPACE_Y420:
                           ucColorSpace = eVIDEO_COLORSPACE_Y420;
                           break;

                        default:
                           ucColorSpace = eVIDEO_COLORSPACE_RGB;
                           break;
                    }
                }
                else
                {
                      ucColorSpace = eVIDEO_COLORSPACE_RGB;
                }
                ucColorDepth = m_FrontEndVideoFormat.u8VideoColorDepth;
                break;

            default:
                ucColorSpace = eVIDEO_COLORSPACE_RGB;
                ucColorDepth = eCOLOR_DEPTH_8BIT;
                break;
        }

        switch (ucColorSpace)
        {
            case eVIDEO_COLORSPACE_RGB: //RGB
                sprintf((char *)acColorSpaceString, "RGB");
                break;

            case eVIDEO_COLORSPACE_Y422: //422
                sprintf((char *)acColorSpaceString, "4.2.2");
                break;

            case eVIDEO_COLORSPACE_Y444: //444
                sprintf((char *)acColorSpaceString, "4.4.4");
                break;

            case eVIDEO_COLORSPACE_Y420:
                sprintf((char *)acColorSpaceString, "4.2.0");
                break;

             default:
                sprintf((char *)acColorSpaceString, "-");     //ZU860_Doulas_0125 modify
                break;
        }

        switch (ucColorDepth)
        {
            case eCOLOR_DEPTH_8BIT: //8
                sprintf((char *)acColorDepthString, "8bit");
                break;

            case eCOLOR_DEPTH_10BIT: //10
                sprintf((char *)acColorDepthString, "10bit");
                break;

            case eCOLOR_DEPTH_12BIT: //12
                sprintf((char *)acColorDepthString, "12bit");
                break;

            case eCOLOR_DEPTH_16BIT: //16
                sprintf((char *)acColorDepthString, "16bit");
                break;

             default:
                sprintf((char *)acColorDepthString, "-");    //ZU860_Doulas_0125 modify
                break;
        }
        sprintf(aucString,"%s",  acColorDepthString);		//HICC2_Zonic_0014
        memcpy(ucValue, aucString, strlen(aucString)+1);
    }
    else
    {
        *ucValue = '-';
        *(ucValue+1) = '\0';
    }

    return eExecResult;
}

eEXEC_CODE palDataPath_ColorFormat_Get(UINT8 *ucValue)    //ZU860_Doulas_0097
{
    eEXEC_CODE eExecResult = eEXEC_CODE_PASS;
    char aucString[24];
    char acSourceFormatString[10] = {0};
    char acColorimetryString[10] = {0};
    UINT8 ucColorType = eVIDEO_YUV_REC601;
    UINT8 ucSourceFormat = eHDR_SETTING_SDR;

    if(palDataPath_IsSourceLock())
    {
        switch(m_sSourceDesc.eConnector)
        {
            case eCM_SOURCE_VGA:         //ZU860_Doulas_0125 modify
            case eCM_SOURCE_3GSDI:
                *ucValue = '-';
                *(ucValue+1) = '\0';
                return eExecResult;

            case eCM_SOURCE_HDMI1:
            case eCM_SOURCE_HDMI2:
            case eCM_SOURCE_DVI:
            case eCM_SOURCE_HDBASET:
            case eCM_SOURCE_DISPLAYPORT: //G100_Steven_0001
                if(m_FrontEndVideoFormat.u8VideoAVIInfoDet == TRUE)
                {
                    switch(m_FrontEndVideoFormat.u8VideoColorYUV)
                    {
                        case eVIDEO_YUV_REC601:
                        case eVIDEO_YUV_REC709:
                            ucColorType = m_FrontEndVideoFormat.u8VideoColorYUV;
                            break;

                        case eVIDEO_YUV_EXTENDED_COLORIMETRY:   //ZU860_Doulas_0100 modify
                            if((m_FrontEndVideoFormat.u8VideoHDRType == eHDR_SETTING_HDR) ||
                               (m_FrontEndVideoFormat.u8VideoExtentedColorimetry == eEXTENDED_COLORIMETRY_BT2020_YcCbcCrc) ||
                               (m_FrontEndVideoFormat.u8VideoExtentedColorimetry == eEXTENDED_COLORIMETRY_BT2020RGB_YCbCr) )
                            {
                                ucColorType = eVIDEO_YUV_BT2020;
                            }
                            else if(m_FrontEndVideoFormat.u8VideoExtentedColorimetry == eEXTENDED_COLORIMETRY_xvYCC709)
                            {
                                ucColorType = eVIDEO_YUV_REC709;
                            }
                            else
                            {
                                ucColorType = eVIDEO_YUV_REC601;
                            }
                            break;

                        default:
                            ucColorType = eVIDEO_YUV_REC601;
                            break;
                    }
                }
                else
                {
                      ucColorType = eVIDEO_YUV_REC601;
                }

                ucSourceFormat = m_FrontEndVideoFormat.u8VideoHDRType;      //ZU860_Doulas_0100 modify
                break;

            default:
                ucSourceFormat = eHDR_SETTING_SDR;
                ucColorType = eVIDEO_YUV_REC601;
                break;
        }

        switch (ucSourceFormat)
        {
            case eHDR_SETTING_SDR:
                sprintf((char *)acSourceFormatString, "SDR");
                break;

            case eHDR_SETTING_HDR:
                sprintf((char *)acSourceFormatString, "HDR");
                break;

             default:
                sprintf((char *)acSourceFormatString, "-");   //ZU860_Doulas_0125 modify
                break;
        }

        switch (ucColorType)
        {
            case eVIDEO_YUV_REC601:
                sprintf((char *)acColorimetryString, "BT.601");
                break;

            case eVIDEO_YUV_REC709:
                sprintf((char *)acColorimetryString, "BT.709");
                break;

            case eVIDEO_YUV_BT2020:
                sprintf((char *)acColorimetryString, "BT.2020");
                break;

             default:
                sprintf((char *)acColorimetryString, "-");     //ZU860_Doulas_0125 modify
                break;
        }
        sprintf(aucString,"%s  %s",  acSourceFormatString, acColorimetryString);
        memcpy(ucValue, aucString, strlen(aucString)+1);
    }
    else
    {
        *ucValue = '-';
        *(ucValue+1) = '\0';
    }

    return eExecResult;
}


//H2 Simon : m_FrontEndVideoFormat need to move to palImgMgr or palFrontendMgr
UINT8 palDataPath_HDR_Info_Get(void)    //ZU860_Doulas_0100
{
    return m_FrontEndVideoFormat.u8VideoHDRType;
}

eEXEC_CODE palDataPath_Formatter_HDR_Picture_Mode_Set(UINT8 Mode)   //ZU860_Doulas_0100
{
    eEXEC_CODE eExecResult = eEXEC_CODE_PASS;

    #if (SCALER_HDR_ENABLE == TRUE)   //H2PF_Simon_0193
    {
        halFormatter_IMG_GammaLutSet(DDP442X_GAMMA_2P2);
    }
    #else
    {
        switch(Mode)
        {
            case eCM_HDR_LEVEL1:
                halFormatter_IMG_GammaLutSet(DDP442X_GAMMA_HDR_BRIGHT);	//G100_Clare_0040
                break;

            case eCM_HDR_LEVEL2:
                halFormatter_IMG_GammaLutSet(DDP442X_GAMMA_HDR_STANDARD);	//G100_Clare_0040
                break;

            case eCM_HDR_LEVEL3:
                halFormatter_IMG_GammaLutSet(DDP442X_GAMMA_HDR_FILM);	//G100_Clare_0040
                break;

            case eCM_HDR_LEVEL4:
                halFormatter_IMG_GammaLutSet(DDP442X_GAMMA_HDR_DETAIL);	//G100_Clare_0040
                break;

            default:
                halFormatter_IMG_GammaLutSet(DDP442X_GAMMA_HDR_BRIGHT);	//G100_Clare_0040
                break;
        }
    }
    #endif

    return eExecResult;
}

void palDataPath_DDP_AspectRatioSet(void)   //ZU860_Doulas_0119
{
    UINT8 ucValue = DDP_ASPECT_NATIVE;
    halFormatter_AspectRatio_Get(&ucValue);

    if((halScaler_Input_3D_Format_Get() == eINPUT_3D_TYPE_FRAME_SEQUENTIAL) &&
	   (palDataPath_IsSourceLock() == TRUE))
    {
        if(halScaler_IsAspectRation16_9_Get())
        {
            if(ucValue != DDP_ASPECT_NATIVE)
            {
                ucValue = DDP_ASPECT_NATIVE;
                halFormatter_AspectRatio_Set(&ucValue);
            }
        }
        else if(halScaler_IsAspectRation16_10_Get())     //ZU860_Doulas_0120 16:10
        {
            if(ucValue != DDP_ASPECT_FILL)
            {
                ucValue = DDP_ASPECT_FILL;
                halFormatter_AspectRatio_Set(&ucValue);
            }
        }
        else
        {
            if(ucValue != DDP_ASPECT_4_3)
            {
                ucValue = DDP_ASPECT_4_3;
                halFormatter_AspectRatio_Set(&ucValue);
            }
        }
    }
    else
    {
        if(ucValue != DDP_ASPECT_NATIVE)
        {
            ucValue = DDP_ASPECT_NATIVE;
            halFormatter_AspectRatio_Set(&ucValue);
        }
    }
}

UINT8 palDataPath_GetPanelChangeState(void)     //ZU860_Doulas_0138
{
    return m_ucPanelChangeing;
}

void palDataPath_SetPanelChangeState(UINT8 ucPanelChangeing)    //ZU860_Doulas_0138
{
    m_ucPanelChangeing = ucPanelChangeing;
}

UINT8 palDataPath_AlreadyShowDisconnectTwistMsg_Flag_Get(void) //ZU860_Doulas_0138
{
    return m_ucAlreadyShowDisconnectTwistMsg;
}

void palDataPath_AlreadyShowDisconnectTwistMsg_Flag_Set(UINT8 ucValue) //ZU860_Doulas_0138
{
    m_ucAlreadyShowDisconnectTwistMsg = ucValue;
}

void palDataPath_BackupConfigFlag(void)    //ZU860_Doulas_0138
{
    //m_ucPanelIDForTwist = m_ucPanelIDForTwist;
    //m_uc3D_FormatConfigForTwist = m_uc3D_FormatConfigForTwist;
    m_ucPanelIDForTwist = m_sPalDataPathInfo.ePanelTimingId;
    m_uc3D_FormatConfigForTwist = halScaler_Input_3D_Format_Config_Get();
#ifdef SCALER_C821_C789
    m_ucC789_Output_V_FreqForTwist = halC789Ctrl_OutputV_FreqGet();
#endif
    //m_ucAlreadyShowDisconnectTwistMsg = ets_OFF; //HICC2_Doulas_0053 remove
    palDataMgr_UI_EventSend(edcUI_EVENT_DISCONNECT_TWIST_MSG, FALSE, NULL);
}

BOOL palDataPath_IsErrorInputGetForTwistOn(void)    //ZU860_Doulas_0138
{
    #ifdef SCALER_C341   //wait review
    return FALSE;
    #endif

    LOG_MSG(db_HAL_WARPING, "\n=> Check Is Error Input For Twist On\n");
    LOG_MSG(db_HAL_WARPING, "halScaler_Input_3D_Format_Config_Get() %d\n" , halScaler_Input_3D_Format_Config_Get());
    LOG_MSG(db_HAL_WARPING, "m_sPalDataPathInfo.ePanelTimingId %d\n" , m_sPalDataPathInfo.ePanelTimingId);
    LOG_MSG(db_HAL_WARPING, "halScaler_OutputVsyncFreqGetForTwistOn() %d\n" , halScaler_OutputVsyncFreqGetForTwistOn());
    LOG_MSG(db_HAL_WARPING, "m_ucC789_Output_V_FreqForTwist %d\n\n" , m_ucC789_Output_V_FreqForTwist);

    if((halScaler_Input_3D_Format_Config_Get() != eINPUT_3D_TYPE_OFF) &&
        (m_sPalDataPathInfo.ePanelTimingId != ePANEL_ID_WUXGA_120HZ) &&    //A70LV_Doulas_0377 Add
       (m_sPalDataPathInfo.ePanelTimingId != ePANEL_ID_1080P_120HZ))
    {
        LOG_MSG(db_HAL_WARPING, "block source change case 1\n");
        return TRUE;
    }
    else if((halScaler_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_OFF) &&
        (m_sPalDataPathInfo.ePanelTimingId == ePANEL_ID_1080P_120HZ || m_sPalDataPathInfo.ePanelTimingId == ePANEL_ID_WUXGA_120HZ))  //A70LV_Doulas_0377 Modify
    {
        LOG_MSG(db_HAL_WARPING, "block source change case 2\n");
        return TRUE;
    }

    if((m_sPalDataPathInfo.ePanelTimingId == ePANEL_ID_1080P_120HZ) ||
       (m_sPalDataPathInfo.ePanelTimingId == ePANEL_ID_WUXGA_120HZ))    //A70LV_Doulas_0377 Modify
    {
        if(halScaler_OutputVsyncFreqGetForTwistOn() != m_ucC789_Output_V_FreqForTwist)
        {
            LOG_MSG(db_HAL_WARPING, "block source change case 3\n");
            return TRUE;
        }
    }
    else
    {
        if(halScaler_OutputVsyncFreqGetForTwistOn() != m_ucC789_Output_V_FreqForTwist)
        {
            LOG_MSG(db_HAL_WARPING, "block source change case 4\n");
            return TRUE;
        }
    }

    return FALSE;
}

void palDataPath_Last3DEnable_Set(UINT8 ucVal)      //A70LV_Doulas_0367
{
    m_ucLast3DEnable_Setting = ucVal;
}

UINT8 palDataPath_Last3DEnable_Get(void)      //A70LV_Doulas_0368
{
    return m_ucLast3DEnable_Setting;
}
#ifdef Low_Latency_All
void palDataPath_Low_Latency_Set(eLOW_LATENCY_MODE eLLMode)     //H2PF_Simon_0150
{
	if(eLLMode > eLLM_OFF)  //low latency on
	{
		//halFormatter_FRCByPassModeSet(TRUE); //G100_Larry_0028
		#ifdef SCALER_C821_C789
		halC789Ctrl_Low_Latency_Set(TRUE);
		#endif

        palImgMgr_Low_Latency_Set(eLLMode);
	}
	else
	{
		#ifdef SCALER_C821_C789
		if(halScaler_3D_Type() == eINPUT_3D_TYPE2_1P4)
		{
			//halFormatter_FRCByPassModeSet(FALSE); //G100_Larry_0028
			halC789Ctrl_Low_Latency_Set(FALSE); //G100_Simon_0047
		}
		else if(halScaler_3D_Type() == eINPUT_3D_TYPE2_FRAME_SEQUENTIAL)
		{
			//halFormatter_FRCByPassModeSet(FALSE); //G100_Larry_0028
			halC789Ctrl_Low_Latency_Set(FALSE);
		}
		else
		{
			halC789Ctrl_Low_Latency_Set(FALSE);
			//halFormatter_FRCByPassModeSet(TRUE); //G100_Larry_0028
		}
		#endif

		palImgMgr_Low_Latency_Set(eLLMode);
	}
}
#endif	/*Low_Latency_All*/


UINT8 palDataPath_BKInput_First_Input_Ready(void)
{
#ifdef SCALER_C821_C789

    if(palEnvironment_BKInput_Ready_Get() & BK_FIRST_INPUT_READY)  //A35G2_CDS_Simon_0036
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }

#else

    return 0;

#endif

#if 0
    UINT8 ucSource = 0;
    UINT8 ucBackupFirstInput  = 0;
    UINT8 ucBackupSecondInput = 0;
    palDataMgr_Data_Access(edcMAIN_INPUT, edaREAD, &ucSource);
    palDataMgr_Data_Access(edcBACKUPINPUT_PRIMARY_INPUT,   edaREAD, &ucBackupFirstInput);
    palDataMgr_Data_Access(edcBACKUPINPUT_SECONDARY_INPUT, edaREAD, &ucBackupSecondInput);

    if(ucSource == palDataMgr_BKInput_Trans2_CMInput(ucBackupFirstInput))
    {
        //Backup First Src  : Main Channel
        //Backup Second Src : Sub Channel
        if(palEnvironment_BKInput_Ready_Get() & BK_MAIN_CHANNEL_READY)
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    else if(ucSource == palDataMgr_BKInput_Trans2_CMInput(ucBackupSecondInput))
    {
        //Backup First Src  : Sub Channel
        //Backup Second Src : Main Channel
        if(palEnvironment_BKInput_Ready_Get() & BK_SUB_CHANNEL_READY)
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
#endif

    return FALSE;
}

UINT8 palDataPath_BKInput_Second_Input_Ready(void)
{
#ifdef SCALER_C821_C789

    if(palEnvironment_BKInput_Ready_Get() & BK_SECOND_INPUT_READY)  //A35G2_CDS_Simon_0036
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
#else

    return FALSE;

#endif


#if 0
    UINT8 ucSource = 0;
    UINT8 ucBackupFirstInput  = 0;
    UINT8 ucBackupSecondInput = 0;
    palDataMgr_Data_Access(edcMAIN_INPUT, edaREAD, &ucSource);
    palDataMgr_Data_Access(edcBACKUPINPUT_PRIMARY_INPUT,   edaREAD, &ucBackupFirstInput);
    palDataMgr_Data_Access(edcBACKUPINPUT_SECONDARY_INPUT, edaREAD, &ucBackupSecondInput);

    if(ucSource == palDataMgr_BKInput_Trans2_CMInput(ucBackupFirstInput))
    {
        //Backup First Src  : Main Channel
        //Backup Second Src : Sub Channel
        if(palEnvironment_BKInput_Ready_Get() & BK_SUB_CHANNEL_READY)
        {
        	return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    else if(ucSource == palDataMgr_BKInput_Trans2_CMInput(ucBackupSecondInput))
    {
        //Backup First Src  : Sub Channel
        //Backup Second Src : Main Channel
        if(palEnvironment_BKInput_Ready_Get() & BK_MAIN_CHANNEL_READY)
        {
        	return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
#endif

    return FALSE;
}



//G100_Steven_0028
eEXEC_CODE palDataPath_BKInput_FirstVideoInfo_Get(void)
{
    eEXEC_CODE eExecResult = eEXEC_CODE_PASS;
    char aucString[VERSION_STRING_MAX_LENGTH];
    UINT8 ucVal[15];
    UINT16 VSyncFreq;

    UINT8 ucSource = 0;
    UINT8 ucBackupFirstInput  = 0;
    UINT8 ucBackupSecondInput = 0;
    palDataMgr_Data_Access(edcMAIN_INPUT, edaREAD, &ucSource);
    palDataMgr_Data_Access(edcBACKUPINPUT_PRIMARY_INPUT,   edaREAD, &ucBackupFirstInput);
    palDataMgr_Data_Access(edcBACKUPINPUT_SECONDARY_INPUT, edaREAD, &ucBackupSecondInput);

    if(palDataPath_BKInput_First_Input_Ready() == TRUE)
    {
        if(ucSource == palDataMgr_BKInput_Trans2_CMInput(ucBackupFirstInput))
        {
            halFrontEndCtrl_Main_Timing_Get(ucVal);

            if(m_BKInput_Change == FALSE)  //A35G2_CDS_Simon_0055
            {
                palImgMgr_InputVertRefresh2_Get_FromSyncCount(eSOURCE_WINDOW_MAIN, &VSyncFreq);   //A35G2_CDS_Simon_0052
            }
            else
            {
                palImgMgr_InputVertRefresh2_Get_FromSyncCount(eSOURCE_WINDOW_SUB, &VSyncFreq);   //A35G2_CDS_Simon_0052
            }
        }
        else
        {
            halFrontEndCtrl_Sub_Timing_Get(ucVal);

            if(m_BKInput_Change == FALSE)  //A35G2_CDS_Simon_0055
            {
                palImgMgr_InputVertRefresh2_Get_FromSyncCount(eSOURCE_WINDOW_SUB, &VSyncFreq);   //A35G2_CDS_Simon_0052
            }
            else
            {
                palImgMgr_InputVertRefresh2_Get_FromSyncCount(eSOURCE_WINDOW_MAIN, &VSyncFreq);   //A35G2_CDS_Simon_0052
            }
        }

        //m_BKInput_FirstTiming.u32VideoPCLK    = ucVal[0]+ (ucVal[1]<<8) + (ucVal[2]<<16) +(ucVal[3]<<24);
        m_BKInput_FirstTiming.u16VideoHTotal  = ucVal[4]+ (ucVal[5]<<8);
        m_BKInput_FirstTiming.u16VideoHActive = ucVal[6]+ (ucVal[7]<<8);
        m_BKInput_FirstTiming.u16VideoVTotal  = ucVal[8]+ (ucVal[9]<<8);
        m_BKInput_FirstTiming.u16VideoVActive = ucVal[10]+ (ucVal[11]<<8);
        //m_BKInput_FirstTiming.u16VideoVRate   = ucVal[12]+ (ucVal[13]<<8);
        m_BKInput_FirstTiming.u16VideoVRate   = VSyncFreq;  //from C821     //A35G2_CDS_Simon_0052
        m_BKInput_FirstTiming.u32VideoPCLK    = m_BKInput_FirstTiming.u16VideoHTotal * m_BKInput_FirstTiming.u16VideoVTotal * m_BKInput_FirstTiming.u16VideoVRate / 100 ;   //A35G2_CDS_Simon_0052
    }
    else
    {
        memset(&m_BKInput_FirstTiming, 0, sizeof(m_BKInput_FirstTiming));
    }

    return eExecResult;
}

eEXEC_CODE palDataPath_BKInput_SecondVideoInfo_Get(void)
{
    eEXEC_CODE eExecResult = eEXEC_CODE_PASS;
    char aucString[VERSION_STRING_MAX_LENGTH];
    UINT8 ucVal[15];
    UINT16 VSyncFreq;

    UINT8 ucSource = 0;
    UINT8 ucBackupFirstInput  = 0;
    UINT8 ucBackupSecondInput = 0;
    palDataMgr_Data_Access(edcMAIN_INPUT, edaREAD, &ucSource);
    palDataMgr_Data_Access(edcBACKUPINPUT_PRIMARY_INPUT,   edaREAD, &ucBackupFirstInput);
    palDataMgr_Data_Access(edcBACKUPINPUT_SECONDARY_INPUT, edaREAD, &ucBackupSecondInput);

    if(palDataPath_BKInput_Second_Input_Ready() == TRUE)
    {
        if(ucSource == palDataMgr_BKInput_Trans2_CMInput(ucBackupSecondInput))
        {
            halFrontEndCtrl_Main_Timing_Get(ucVal);

            if(m_BKInput_Change == FALSE)  //A35G2_CDS_Simon_0055
            {
                palImgMgr_InputVertRefresh2_Get_FromSyncCount(eSOURCE_WINDOW_MAIN, &VSyncFreq);   //A35G2_CDS_Simon_0052
            }
            else
            {
                palImgMgr_InputVertRefresh2_Get_FromSyncCount(eSOURCE_WINDOW_SUB, &VSyncFreq);   //A35G2_CDS_Simon_0052
            }
        }
        else
        {
            halFrontEndCtrl_Sub_Timing_Get(ucVal);

            if(m_BKInput_Change == FALSE)  //A35G2_CDS_Simon_0055
            {
                palImgMgr_InputVertRefresh2_Get_FromSyncCount(eSOURCE_WINDOW_SUB, &VSyncFreq);   //A35G2_CDS_Simon_0052
            }
            else
            {
                palImgMgr_InputVertRefresh2_Get_FromSyncCount(eSOURCE_WINDOW_MAIN, &VSyncFreq);   //A35G2_CDS_Simon_0052
            }
        }

        //m_BKInput_SecondTiming.u32VideoPCLK    = ucVal[0]+ (ucVal[1]<<8) + (ucVal[2]<<16) +(ucVal[3]<<24);
        m_BKInput_SecondTiming.u16VideoHTotal  = ucVal[4]+ (ucVal[5]<<8);
        m_BKInput_SecondTiming.u16VideoHActive = ucVal[6]+ (ucVal[7]<<8);
        m_BKInput_SecondTiming.u16VideoVTotal  = ucVal[8]+ (ucVal[9]<<8);
        m_BKInput_SecondTiming.u16VideoVActive = ucVal[10]+ (ucVal[11]<<8);
        //m_BKInput_SecondTiming.u16VideoVRate   = ucVal[12]+ (ucVal[13]<<8);

        m_BKInput_SecondTiming.u16VideoVRate   = VSyncFreq;  //from C821   //A35G2_CDS_Simon_0052
        m_BKInput_SecondTiming.u32VideoPCLK    = m_BKInput_SecondTiming.u16VideoHTotal * m_BKInput_SecondTiming.u16VideoVTotal * m_BKInput_SecondTiming.u16VideoVRate / 100 ;   //A35G2_CDS_Simon_0052
    }
    else
    {
        memset(&m_BKInput_SecondTiming, 0, sizeof(m_BKInput_SecondTiming));
    }

    return eExecResult;
}

eEXEC_CODE palDataPath_BKInput_FirstResoultion_Get(UINT8 *ucValue)
{
    eEXEC_CODE eExecResult = eEXEC_CODE_PASS;
    char aucString[VERSION_STRING_MAX_LENGTH];

    if(palDataPath_BKInput_First_Input_Ready() == TRUE)
    {
        sprintf(aucString, "%d x %d",m_BKInput_FirstTiming.u16VideoHActive ,m_BKInput_FirstTiming.u16VideoVActive);
        memcpy(ucValue, aucString, strlen(aucString)+1);
    }
    else
    {
        *ucValue = '-';
        *(ucValue+1) = '\0';
    }

    return eExecResult;
}

eEXEC_CODE palDataPath_BKInput_SecondResoultion_Get(UINT8 *ucValue)
{
    eEXEC_CODE eExecResult = eEXEC_CODE_PASS;
    char aucString[VERSION_STRING_MAX_LENGTH];

    if(palDataPath_BKInput_Second_Input_Ready() == TRUE)
    {
        sprintf(aucString, "%d x %d",m_BKInput_SecondTiming.u16VideoHActive ,m_BKInput_SecondTiming.u16VideoVActive);
        memcpy(ucValue, aucString, strlen(aucString)+1);
    }
    else
    {
        *ucValue = '-';
        *(ucValue+1) = '\0';
    }

    return eExecResult;
}

eEXEC_CODE palDataPath_BKInput_FirstHRefresh_Get(UINT8 *ucValue)
{
    eEXEC_CODE eExecResult = eEXEC_CODE_PASS;

    if(palDataPath_BKInput_First_Input_Ready() == TRUE)
    {
        #if 0
        char aucString[VERSION_STRING_MAX_LENGTH];
        DOUBLE dbVal;
        dbVal = (DOUBLE)m_BKInput_FirstTiming.u32VideoPCLK *1000 / (DOUBLE)m_BKInput_FirstTiming.u16VideoHTotal;
        sprintf(aucString, "%d.%03dkHz",(UINT32)dbVal/1000,(UINT32)dbVal%1000);
        memcpy(ucValue, aucString, strlen(aucString)+1);
        #endif

        char aucString[VERSION_STRING_MAX_LENGTH];
        DOUBLE dbVal;
        dbVal = (DOUBLE)m_BKInput_FirstTiming.u16VideoVTotal * (DOUBLE)m_BKInput_FirstTiming.u16VideoVRate / 100 ;   //A35G2_CDS_Simon_0052
        sprintf(aucString, "%d.%03dkHz",(UINT32)dbVal/1000,(UINT32)dbVal%1000);
        memcpy(ucValue, aucString, strlen(aucString)+1);
    }
    else
    {
        *ucValue = '-';
        *(ucValue+1) = '\0';
    }

    return eExecResult;
}

eEXEC_CODE palDataPath_BKInput_SecondHRefresh_Get(UINT8 *ucValue)
{
    eEXEC_CODE eExecResult = eEXEC_CODE_PASS;

    if(palDataPath_BKInput_Second_Input_Ready() == TRUE)
    {
        #if 0
        char aucString[VERSION_STRING_MAX_LENGTH];
        DOUBLE dbVal;
        dbVal = (DOUBLE)m_BKInput_SecondTiming.u32VideoPCLK *1000 / (DOUBLE)m_BKInput_SecondTiming.u16VideoHTotal;
        sprintf(aucString, "%d.%03dkHz",(UINT32)dbVal/1000,(UINT32)dbVal%1000);
        memcpy(ucValue, aucString, strlen(aucString)+1);
        #endif

        char aucString[VERSION_STRING_MAX_LENGTH];
        DOUBLE dbVal;
        dbVal = (DOUBLE)m_BKInput_SecondTiming.u16VideoVTotal * (DOUBLE)m_BKInput_SecondTiming.u16VideoVRate / 100 ;   //A35G2_CDS_Simon_0052
        sprintf(aucString, "%d.%03dkHz",(UINT32)dbVal/1000,(UINT32)dbVal%1000);
        memcpy(ucValue, aucString, strlen(aucString)+1);
    }
    else
    {
        *ucValue = '-';
        *(ucValue+1) = '\0';
    }

    return eExecResult;
}

eEXEC_CODE palDataPath_BKInput_FirstColorSpace_Get(UINT8 *ucValue)
{
	eEXEC_CODE eExecResult = eEXEC_CODE_PASS;

	if(palDataPath_BKInput_First_Input_Ready() == TRUE)
	{
		char aucString[VERSION_STRING_MAX_LENGTH];

	    switch(m_BKInput_FirstFormat[1])
	    {
	        case 0:
	            sprintf(aucString, "RGB");
	            memcpy(ucValue, aucString, strlen(aucString)+1);
	            break;

	        case 1:
	            sprintf(aucString, "Y422");
	            memcpy(ucValue, aucString, strlen(aucString)+1);
	            break;

	        case 2:
	            sprintf(aucString, "Y444");
	            memcpy(ucValue, aucString, strlen(aucString)+1);
	            break;

	        case 3:
	            sprintf(aucString, "Y420");
	            memcpy(ucValue, aucString, strlen(aucString)+1);
	            break;

	        default:
	    		*ucValue = '-';
	    		*(ucValue+1) = '\0';
	            break;
	    }
	}
	else
	{
		*ucValue = '-';
		*(ucValue+1) = '\0';
	}
	return eExecResult;
}

eEXEC_CODE palDataPath_BKInput_SecondColorSpace_Get(UINT8 *ucValue)
{
	eEXEC_CODE eExecResult = eEXEC_CODE_PASS;

	if(palDataPath_BKInput_Second_Input_Ready() == TRUE)
	{
		char aucString[VERSION_STRING_MAX_LENGTH];

	    switch(m_BKInput_SecondFormat[1])
	    {
	        case 0:
	            sprintf(aucString, "RGB");
	            memcpy(ucValue, aucString, strlen(aucString)+1);
	            break;

	        case 1:
	            sprintf(aucString, "Y422");
	            memcpy(ucValue, aucString, strlen(aucString)+1);
	            break;

	        case 2:
	            sprintf(aucString, "Y444");
	            memcpy(ucValue, aucString, strlen(aucString)+1);
	            break;

	        case 3:
	            sprintf(aucString, "Y420");
	            memcpy(ucValue, aucString, strlen(aucString)+1);
	            break;

	        default:
	    		*ucValue = '-';
	    		*(ucValue+1) = '\0';
	            break;
	    }
	}
	else
	{
		*ucValue = '-';
		*(ucValue+1) = '\0';
	}
	return eExecResult;
}
//G100_Steven_0028

eEXEC_CODE palDataPath_BKInput_FirstVideoFormat_Get(void)
{
	eEXEC_CODE eExecResult = eEXEC_CODE_PASS;

    UINT8 ucSource = 0;
    UINT8 ucBackupFirstInput  = 0;
    UINT8 ucBackupSecondInput = 0;
    palDataMgr_Data_Access(edcMAIN_INPUT, edaREAD, &ucSource);
    palDataMgr_Data_Access(edcBACKUPINPUT_PRIMARY_INPUT,   edaREAD, &ucBackupFirstInput);
    palDataMgr_Data_Access(edcBACKUPINPUT_SECONDARY_INPUT, edaREAD, &ucBackupSecondInput);

    if(palDataPath_BKInput_First_Input_Ready() == TRUE)
    {
        if(ucSource == palDataMgr_BKInput_Trans2_CMInput(ucBackupFirstInput))
        {
            halFrontEndCtrl_videoFormat_Get(m_BKInput_FirstFormat);
        }
        else
        {
            halFrontEndCtrl_videoFormatSub_Get(m_BKInput_FirstFormat);
        }
	}
	else
	{
	    memset(m_BKInput_FirstFormat, 0, sizeof(m_BKInput_FirstFormat));
	}

	return eExecResult;

}

eEXEC_CODE palDataPath_BKInput_SecondVideoFormat_Get(void)
{
	eEXEC_CODE eExecResult = eEXEC_CODE_PASS;

    UINT8 ucSource = 0;
    UINT8 ucBackupFirstInput  = 0;
    UINT8 ucBackupSecondInput = 0;
    palDataMgr_Data_Access(edcMAIN_INPUT, edaREAD, &ucSource);
    palDataMgr_Data_Access(edcBACKUPINPUT_PRIMARY_INPUT,   edaREAD, &ucBackupFirstInput);
    palDataMgr_Data_Access(edcBACKUPINPUT_SECONDARY_INPUT, edaREAD, &ucBackupSecondInput);

    if(palDataPath_BKInput_Second_Input_Ready() == TRUE)
    {
        UINT8 ucSource = 0;
        UINT8 ucBackupSecondInput = 0;
        palDataMgr_Data_Access(edcMAIN_INPUT, edaREAD, &ucSource);
        palDataMgr_Data_Access(edcBACKUPINPUT_SECONDARY_INPUT, edaREAD, &ucBackupSecondInput);
        if(ucSource == palDataMgr_BKInput_Trans2_CMInput(ucBackupSecondInput))
        {
            halFrontEndCtrl_videoFormat_Get(m_BKInput_SecondFormat);
        }
        else
        {
            halFrontEndCtrl_videoFormatSub_Get(m_BKInput_SecondFormat);
        }
	}
	else
	{
	    memset(m_BKInput_SecondFormat, 0, sizeof(m_BKInput_SecondFormat));
	}

	return eExecResult;

}

UINT8 palDataPath_BKInput_IsSyncCountSimilar(void)  //A35G2_CDS_Simon_0041
{
    UINT8 m_BKInput_ScalerSyncCountIsSimilar = FALSE;  //A35G2_CDS_Simon_0041
    halScaler_CheckChannelSyncCount(&m_BKInput_ScalerSyncCountIsSimilar);
    //LOG_MSG(db_APP_DATAPATH, "m_BKInput_ScalerSyncCountIsSimilar = %d\n", m_BKInput_ScalerSyncCountIsSimilar);

    return m_BKInput_ScalerSyncCountIsSimilar;
}

UINT8 palDataPath_BKInput_FE_Status_Get(void) //G100_Steven_0050
{
    //m_BKInput_Status:
    //Bit0 : BK_Active
    //Bit1 : BK_Swap
    //Bit2 : BK_QuickSwitchActive

	halFrontEndCtrl_Backup_Status_Get(&m_BKInput_Status); //bit0:Active ; bit1:SWAP
	m_BKInput_Active = (m_BKInput_Status & 0x01); //G100_Steven_0055
	m_BKInput_Change = ((m_BKInput_Status & 0x02) == 0x02) ? TRUE : FALSE;		//A65_OPTOMA_Doulas_0115

	//LOG_MSG(db_ALWAYS, "BKInput_Status_Get %d\r\n", m_BKInput_Status);

	return m_BKInput_Status;
}

UINT8 palDataPath_BKInput_Status(void)
{
    UINT8 ucData = 0/*, ucReady = 0*/; //G100_Steven_0050
    palDataMgr_Data_Access(edcBACKUPINPUT_AUTOSWITCH,edaREAD, &ucData);

    //ucReady = palEnvironment_BKInput_Ready_Get();

    if(ucData == TRUE && m_BKInput_Active == TRUE /*&& palDataPath_BKInput_IsSyncCountSimilar()*/)
    {
        return TRUE;
    }

    return FALSE; //G100_Steven_0050
}

UINT8 palDataPath_BKInput_Change(void)	//A65_OPTOMA_Doulas_0115
{
	UINT8 ucData = 0;
	palDataMgr_Data_Access(edcBACKUPINPUT_AUTOSWITCH,edaREAD, &ucData);

	if(ucData==TRUE && m_BKInput_Change == TRUE)
		return eBACKUP_INPUT_CHANGED;
	else
		return eBACKUP_INPUT_NO_CHANGE;
}

UINT8 FrontEnd_Down_Scaling_Get(void)	//G100_Clare_0053
{
    return m_FrontEndVideoFormat.u8VideoDownScaling;
}

UINT8 palDataPath_C789_C821_InitReadyGet(void)	//G100_Doulas_0076
{
	return m_C789_C821_InitReady;
}

void palDataPath_C789_C821_InitReadySet(UINT8 ucVlue)	//G100_Doulas_0076
{
	m_C789_C821_InitReady = ucVlue;
}



void palDataPath_QuickSwitch_SrcReady_Set(UINT8 ucReady)
{
    m_ucQuickSwitch_SrcReady = ucReady;
}

UINT8 palDataPath_QuickSwitch_SrcReady_Get(void)
{
    return m_ucQuickSwitch_SrcReady ;
}

UINT8 palDataPath_CurrentSrcIsQuickSwitchSrc(void)
{
    UINT8 ucMainSrc = eCM_SOURCE_NUMBER;
    UINT8 ucBackupSrcMain = eCM_SOURCE_NUMBER;
    UINT8 ucBackupSrcSub  = eCM_SOURCE_NUMBER;
    palDataMgr_Data_Access(edcMAIN_INPUT,   edaREAD, &ucMainSrc);
    palDataMgr_Data_Access(edcBACKUPINPUT_PRIMARY_INPUT,   edaREAD, &ucBackupSrcMain);
    ucBackupSrcMain = palDataMgr_BKInput_Trans2_CMInput(ucBackupSrcMain);
    palDataMgr_Data_Access(edcBACKUPINPUT_SECONDARY_INPUT, edaREAD, &ucBackupSrcSub);
    ucBackupSrcSub  = palDataMgr_BKInput_Trans2_CMInput(ucBackupSrcSub);

    if((ucMainSrc == ucBackupSrcMain) || (ucMainSrc == ucBackupSrcSub))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

//test only
void palDataPath_Test_SetTimeStart(void)
{
    TestTimeStart = TMO_GetSysRunTime();
}

//test only
void palDataPath_Test_GetTimeElasped(UINT16 uiTestID)
{
    LOG_MSG(db_APP_DATAPATH, "Time%d : %d\n" , uiTestID, TMO_GetSysRunTime() - (UINT32)TestTimeStart);
}

UINT8 palDataPath_First_Input_HDR_Get(void)	//A65_OPTOMA_Doulas_0116
{
	return m_BKInput_FirstFormat[10];
}

UINT8 palDataPath_Second_Input_HDR_Get(void)	//A65_OPTOMA_Doulas_0116
{
	return m_BKInput_SecondFormat[10];
}

//G100_Tim_0057, add, start //A35G2_BRC_Casper_0060
// ==============================================================================
// DESCRIPTION  : Config the sub source of the Front End only
// Params       : None
// Returns:     : None
//
// Modification History
// --------------------
// 2021/09/03, Tim Chen create
// --------------------
// ==============================================================================
void palDataPath_Auto_HDMI_Switch_Config_Sub( eCM_SOURCE_ID eSub_Src_ID )
{
    UINT8 ucSrc_Sub = eSub_Src_ID;

    LOG_MSG(db_APP_DATA_MGR, "AHSW_Config_Sub(), Src: %d\r\n", eSub_Src_ID); //tim100, add

    palEnvironment_Auto_HDMI_Switch_Source_Changing_Set(ets_ON);
    palImgMgr_SetInputSource(eSOURCE_WINDOW_SUB, ucSrc_Sub);
    halFrontEndCtrl_SubInput_Set( &ucSrc_Sub );
    MS_SLEEP(10);
    palEnvironment_Auto_HDMI_Switch_Source_Changing_Set(ets_OFF);
}

// ==============================================================================
// DESCRIPTION  : Change the main Source and sub source
// Params       : None
// Returns:     : None
//
// Modification History
// --------------------
// 2021/09/03, Tim Chen create
// --------------------
// ==============================================================================
void palDataPath_Auto_HDMI_Switch_Source_Config( eCM_SOURCE_ID eSource_ID )
{
    UINT8 ucValue = eCM_SOURCE_RESERVED;
    eEXEC_CODE eResult = palDataMgr_Data_Access(edcMAIN_INPUT, edaREAD, &ucValue);  // get the current Main source value

    if( eResult != eEXEC_CODE_PASS )
    {
        LOG_MSG(db_APP_DATA_MGR, "AHSW_Config(), get current MAIN fail\r\n"); //tim100, add
        ucValue = eCM_SOURCE_RESERVED;
    }

    switch( ucValue )
    {
        case eCM_SOURCE_HDMI1:
            //set sub source to HDMI 2 for auto HDMI switch monitor
            eResult = palDataMgr_Data_Access(edcSUB_INPUT, edaREAD, &ucValue);  // get the current Sub source value
            if( ( eResult != eEXEC_CODE_PASS )              // get fail
                || ( ucValue != eCM_SOURCE_HDMI2 )          // not HDMI 2
            )
            {
                LOG_MSG(db_APP_DATA_MGR, "AHSW_Config(), Current MAIN is HDMI 1, Sub is not HDMI 2, force sub to HDMI 2\r\n"); //tim100, add
                palDataPath_Auto_HDMI_Switch_Config_Sub(eCM_SOURCE_HDMI2);
            }
            break;

        case eCM_SOURCE_HDMI2:
            // Don't care the sub source. Do nothing.
            eResult = palDataMgr_Data_Access(edcSUB_INPUT, edaREAD, &ucValue);  // get the current Sub source value
            if( ( eResult != eEXEC_CODE_PASS )              // get fail
                || ( ucValue != eCM_SOURCE_HDMI1 )          // not HDMI 2
            )
            {
                LOG_MSG(db_APP_DATA_MGR, "AHSW_Config(), Current MAIN is HDMI 1, Sub is not HDMI 2, force sub to HDMI 2\r\n"); //tim100, add
                palDataPath_Auto_HDMI_Switch_Config_Sub(eCM_SOURCE_HDMI1);  //A35G2_BRC_Simon_0006
            }
            LOG_MSG(db_APP_DATA_MGR, "AHSW_Config(), Current MAIN is HDMI 2, do nothing\r\n"); //tim100, add
            break;

        default:
            // neither HDMI 2 nor HDMI 1, force the main source to HDMI 2.
            //if set fail, it will be retried during auto HDMI switch monitor.
            LOG_MSG(db_APP_DATA_MGR, "AHSW_Config(), Current MAIN is %d, force to HDMI2\r\n", ucValue); //tim100, add
            ucValue = eCM_SOURCE_HDMI2;
            eResult = palDataMgr_Data_Access(edcMAIN_INPUT, edaWRITE_THROUGH_WITH_ACTION, &ucValue);
            ucValue = eCM_SOURCE_HDMI1;
            eResult = palDataMgr_Data_Access(edcSUB_INPUT,  edaWRITE_THROUGH_WITH_ACTION, &ucValue);  //A35G2_BRC_Simon_0006
    }
}
//G100_Tim_0057, add, end

eCM_SOURCE_ID palDataPath_Connector_Get(void)
{
    return m_sSourceDesc.eConnector;
}

void palDataPath_ConfigGamma(void) //A35G2_CDS_Simon_0061
{
    UINT8 ucHDRAutoEnable = FALSE ;
    UINT8 ucValue = 0;
    palDataMgr_Data_Access(edcHDR_AUTOENABLE, edaREAD, &ucHDRAutoEnable);
    if((ucHDRAutoEnable == ets_ON) && (palImgMgr_HDR_Get(eSOURCE_WINDOW_MAIN) == eHDR_SETTING_HDR) && (palDataMgr_DataCode_Control(edcHDR_LEVEL) == eFUNC_CONTROL_ENABLE))
    {
        palDataMgr_Data_Access(edcHDR_LEVEL, edaREAD, &ucValue);
        palDataMgr_Data_Access(edcHDR_LEVEL, edaWRITE_RAM_ONLY_WITH_ACTION, &ucValue);
    }
    else
    {
        palDataMgr_Data_Access(edcGAMMA, edaREAD, &ucValue);
        palDataMgr_Data_Access(edcGAMMA, edaWRITE_RAM_ONLY_WITH_ACTION, &ucValue);
    }
}

void palDataPath_InputDetectSet(UINT16 uiData)  //A35G2_Simon_0075
{
    m_uiInputDetect = uiData;
}

UINT16 palDataPath_InputDetectGet(void)  //A35G2_Simon_0075
{
    return m_uiInputDetect;
}

void palDataPath_LastInputDetectSet(UINT16 uiData)  //A35G2_Simon_0075
{
    m_uiLastInputDetect = uiData;
}

UINT16 palDataPath_LastInputDetectGet(void)  //A35G2_Simon_0075
{
    return m_uiLastInputDetect;
}

void palDataPath_CustomizedEDID_Init(void)  //A35G2_Simon_0091
{
    UINT8 ucCustomizedEDID_Status = 0 ;
    palDataMgr_Data_Access(edcCUSTOMIZE_EDID_HDMI1, edaREAD, &ucCustomizedEDID_Status);
    if(ucCustomizedEDID_Status == eCES_ACTIVE)
    {
        //resend to frontend
        palSystem_WriteCustomizeEDID_Event(eCM_SOURCE_HDMI1);
    }

    palDataMgr_Data_Access(edcCUSTOMIZE_EDID_HDMI2, edaREAD, &ucCustomizedEDID_Status);
    if(ucCustomizedEDID_Status == eCES_ACTIVE)
    {
        //resend to frontend
        palSystem_WriteCustomizeEDID_Event(eCM_SOURCE_HDMI2);
    }
}

BOOL palDataPath_IS_3D_FrameSequentialSupport(void)	//A70Gen2_Doulas_0014 Add
{
	UINT16 uiV_Freq;
	BOOL b3D_FrameSequentialSupport = FALSE;

	halScaler_InputVertRefresh2_Get(eSOURCE_WINDOW_MAIN,&uiV_Freq);
	if((m_sPalDataPathInfo.ePanelTimingId == ePANEL_ID_1080P_120HZ) ||
       (m_sPalDataPathInfo.ePanelTimingId == ePANEL_ID_WUXGA_120HZ))
	{
		b3D_FrameSequentialSupport = TRUE;
	}
	else
	{
		//LOG_MSG(db_ALWAYS, "V-FREQ = %d\r\n",uiV_Freq);
		if(uiV_Freq > 4950 && uiV_Freq < 5030)
		{
			b3D_FrameSequentialSupport = TRUE;
		}
		else if(uiV_Freq > 5950 && uiV_Freq < 6030)
		{
			b3D_FrameSequentialSupport = TRUE;
		}
		else
		{
			b3D_FrameSequentialSupport = FALSE;
		}
	}

	return b3D_FrameSequentialSupport;
}

void palDataPath_Set3D_FirstFrameMode(void)  //A70Gen2_Doulas_0023
{
    UINT8 uc3DSource[13] = {0};
	halFormatter_3DSourceConfigGet(uc3DSource);
    uc3DSource[1] = LR_1ST_FRAME;
	halFormatter_3DSourceConfigSet(uc3DSource);
}

INT8  palDataPath_MenuTransparencyEnableSet(UINT8 ucEnalbe)
{
	palGui_OSD_MenuTransparencyEnableSet(ucEnalbe);
	ucMenuTransparencyEnable = ucEnalbe;
}

UINT8 palDataPath_OSD_MenuTransparencyEnableGet(void)
{
	return ucMenuTransparencyEnable;
}

void palDataPath_SmoothOnReadyCheck_Set(UINT8 ucData)//HICC2_Julie_0037
{
    m_ucSmoothOnReadyCheck = ucData;
    palDataPath_ResetTimeInState();
}

EOTF_STREAM palDataPath_GetEOTF_STREAM(void)
{
	return m_Video_EOTF_STREAM;
}

void palDataPath_FrontEnd_Timing_Set(UINT8 ucCH ,sVIDEO_TIMING sFrontEnd_Timing) //H30K_Doulas_0011
{
#ifdef SCALER_C341

    halScaler_FrontEnd_Timing_Set(ucCH,sFrontEnd_Timing);

#else

#endif
}

void appDataPath_CheckScalerOutput3DField(void) //H30K_Doulas_0035 //A35G2_Simon_0121
{
#ifdef CHECKING_OUTPUT_3D_FIELD
    UINT8 ucIsReconfig = 0;
    LOG_MSG(db_APP_DATAPATH, "Check Scaler 3D sync Field start\r\n");
    halScaler_3DFLD_Check(&ucIsReconfig);

    LOG_MSG(db_APP_DATAPATH, "Check Scaler 3D sync Field => %d (1 is recovered)\r\n", ucIsReconfig);

    if(ucIsReconfig == TRUE)  //if C821 reconfig POFLD for 3D, need to set DDP 3D off->on
    {
        halFormatter_3D_3DModeSet(FALSE);
        MS_SLEEP(2000);
        //halFormatter_3D_3DModeSet(TRUE);
    }
#endif
}

BOOL appDataPath_Is_4K_TP_FP_SBS(void) //H30K_Doulas_0055
{
    UINT16 uiH_Active = 0;
    UINT16 uiV_Active = 0;

    uiH_Active = m_FrontEndVideoTiming.u16VideoHActive;//halScaler_Input_H_Active_Get(eSOURCE_WINDOW_MAIN);
    uiV_Active = m_FrontEndVideoTiming.u16VideoVActive;//halScaler_Input_V_Active_Get(eSOURCE_WINDOW_MAIN);
    if(((uiH_Active == 3840) && (uiV_Active == 2160)) ||
       ((uiH_Active == 4096) && (uiV_Active == 2160)) ||
       ((uiH_Active == 3840) && (uiV_Active == 2400)) )
    {
        if(halScaler_3D_Type() == eINPUT_3D_TYPE2_1P4)
            return TRUE;
        else
            return FALSE;
    }
    else
        return FALSE;
}

void palDataPath_FirstLanguageCountSet(UINT16 uiData)  //H30K_David_0047
{
    m_uiFirstLanguageCount = uiData;
}

UINT16 palDataPath_FirstLanguageCountGet(void)  //H30K_David_0047
{
    return m_uiFirstLanguageCount;
}

eEXEC_CODE palDataPath_Panel_ChangeCheck(void)
{
    UINT8 ucMasterProjectMode = 0;
    UINT8 ucSlaveProjectMode = 0;

    m_sPalDataPathInfo.uiXPRPanelChange = XPR_PANEL_CHECK_COUNT;

    //if(m_sPalDataPathInfo.uiPanelChange)
    //{
        palFormatterMgr_Projection_Mode_Get(&ucMasterProjectMode);
        palFormatterMgr_SlaveProjection_Mode_Get(&ucSlaveProjectMode);

        if((ucMasterProjectMode != eCMD_Formatter_External) &&
            (ucSlaveProjectMode != eCMD_Formatter_External)) //如果不是External,就不再做判斷
        {
            m_sPalDataPathInfo.uiXPRPanelChange = 0;
        }
        else
        {

        }

        LOG_MSG(db_APP_DATAPATH, "(%s, %d) ucMasterProjectMode = %d, ucSlaveProjectMode = %d, cPanelChange = %d\n", __FUNCTION__, __LINE__, ucMasterProjectMode, ucSlaveProjectMode, m_sPalDataPathInfo.uiXPRPanelChange);
    //}

    return eEXEC_CODE_PASS;
}

UINT8 palDataPath_DataPathIsReady(void)
{
    return m_sPalDataPathInfo.bDataPathReady;
}

void palDataPath_RecordSourceLost(UINT8 ucVal)
{
    uOPD_DATA uOPDData = {0};

    uOPDData.sSnapshotLog.ulSource_Lost = ucVal;
    utilOPD_EventSet(eOPD_SOURCE_LOST_LOG, &uOPDData);
    if(m_sSourceDesc.eConnector == eCM_SOURCE_HDBASET)
    {
        UINT8 ucCur_HDBT_Status[eSYSTEM_MSG_SYSTEM_HDBT_STATUS_SZ];

        if(halMCUCtrl_HDBT_Status_Get(ucCur_HDBT_Status) == rcSUCCESS)
        {
            palDataMgr_Data_Access(edcHDBaseT_Status, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&ucCur_HDBT_Status[0]);
            uOPD_DATA uOPDData = {0};

            for(UINT8 ucIndex = 0 ; ucIndex < eSYSTEM_MSG_SYSTEM_HDBT_STATUS_SZ ; ucIndex++)
            {
                uOPDData.sSnapshotLog.ulHDBT_Status[ucIndex] = ucCur_HDBT_Status[ucIndex];
            }
            utilOPD_EventSet(eOPD_HDBT_STATUS_LOG, &uOPDData);
        }

    }
}


void palDataPath_HDR_Process(void)   //H2PF_Simon_0193
{
    UINT8 HDREnable = 0;
    palDataMgr_Data_Access(edcHDR_AUTOENABLE, edaREAD, &HDREnable);

    if(HDREnable && m_FrontEndVideoFormat.u8VideoHDRType == eHDR_SETTING_HDR)
    {
        //HDR Handle
        LOG_MSG(db_APP_DATAPATH, "HDR :\r\n");
        LOG_MSG(db_APP_DATAPATH, "  - u8VideoHDRType : %d\r\n", m_FrontEndVideoFormat.u8VideoHDRType);
        LOG_MSG(db_APP_DATAPATH, "  - EOTF : %d\r\n", palDataPath_GetEOTF_STREAM());
        LOG_MSG(db_APP_DATAPATH, "  - MaxCLL : %d\r\n", m_HDRLuminanceInfo.MaxCLL);
        LOG_MSG(db_APP_DATAPATH, "  - MaxFALL : %d\r\n", m_HDRLuminanceInfo.MaxFALL);
        LOG_MSG(db_APP_DATAPATH, "  - MaxMasteringNits : %d\r\n", m_HDRLuminanceInfo.MaxMasteringNits);
        LOG_MSG(db_APP_DATAPATH, "  - MinMasteringNits : %f\r\n", m_HDRLuminanceInfo.MinMasteringNits);

        UINT8 HDRMode = 0;
        palDataMgr_Data_Access(edcHDR_MODE, edaREAD, &HDRMode);

        UINT16 HDRManualLevelNit = 0;
        UINT16 HDRManualContentNit = 0;
        palDataMgr_Data_Access(edcHDR_MANUAL_LEVEL_NIT, edaREAD, &HDRManualLevelNit);
        switch(HDRManualLevelNit)
        {
            case eCM_HDR_500_NIT:   HDRManualContentNit = 500;    break;
            case eCM_HDR_1000_NIT:  HDRManualContentNit = 1000;   break;
            case eCM_HDR_2000_NIT:  HDRManualContentNit = 2000;   break;
            case eCM_HDR_4000_NIT:  HDRManualContentNit = 4000;   break;

            case eCM_HDR_CUSTOM_NIT:
                {
                    palDataMgr_Data_Access(edcHDR_MANUAL_CUSTOM_NIT, edaREAD, &HDRManualContentNit);
                }
                break;

            default:
                LOG_MSG(db_APP_DATAPATH, "!!!unknown HDR Manual Level Nit %d\r\n", HDRManualLevelNit);
                HDRManualContentNit = 1000;
                break;
        }

        if(palDataPath_GetEOTF_STREAM() == EOTF_SMPTE)
        {
            if(HDRMode == eCM_HDR_MANUAL) //manual select content nit
            {
                LOG_MSG(db_APP_DATAPATH, "HDR10 Manual Content Nit : %d\r\n", HDRManualContentNit);
                palImgMgr_HDR10_Process((UINT16)HDRManualContentNit);
            }
            else if(m_HDRLuminanceInfo.MaxMasteringNits != 0)
            {
                LOG_MSG(db_APP_DATAPATH, "HDR10 MaxMasteringNits : %d\r\n", m_HDRLuminanceInfo.MaxMasteringNits);
                palImgMgr_HDR10_Process((UINT16)m_HDRLuminanceInfo.MaxMasteringNits);
            }
            else if(m_HDRLuminanceInfo.MaxCLL != 0)
            {
                LOG_MSG(db_APP_DATAPATH, "HDR10 MaxCLL : %d\r\n", m_HDRLuminanceInfo.MaxCLL);
                palImgMgr_HDR10_Process((UINT16)m_HDRLuminanceInfo.MaxCLL);
            }
            else
            {
                LOG_MSG(db_APP_DATAPATH, "HDR10 1000\r\n");
                palImgMgr_HDR10_Process(1000);   //unknown HDR Content max nit information
            }
        }
        else if(palDataPath_GetEOTF_STREAM() == EOTF_HLG)
        {
            LOG_MSG(db_APP_DATAPATH, "HLG\r\n");
            palImgMgr_HLG_Process(1000);
        }
        else
        {
            LOG_MSG(db_APP_DATAPATH, "undefined EOTF %d\r\n", palDataPath_GetEOTF_STREAM());
        }

    }
    else  //SDR
    {
        //SDR Enhance
        UINT8 SDREnhance = 0;
        palDataMgr_Data_Access(edcHDR_DYNAMIC_CONTRAST, edaREAD, &SDREnhance);

        if(SDREnhance)
        {
            LOG_MSG(db_APP_DATAPATH, "SDR Enhance\r\n");
            palImgMgr_HDR_Disable();
            palImgMgr_SDREnhance_Process();
        }
        else
        {
            LOG_MSG(db_APP_DATAPATH, "Disable HDR\r\n");
            palImgMgr_HDR_Disable();
        }
    }
}

