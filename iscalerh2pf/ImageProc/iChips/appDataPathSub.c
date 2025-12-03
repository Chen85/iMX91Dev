#include "appDataPath.h"
#include "appDataMgr.h"
#include "appSystem.h" //A70LV_Larry_0112
//#include "appGui.h"
#include "appLANProcAPI.h"
#include "halScaler.h"
#include "halWarping.h"
#include "utilDbgMsg.h"
#ifdef SCALER_C821_C789
#include "halC789CtrlAPI.h"    //A70LV_Doulas_0072
#endif
#include "halFrontEndCtrlAPI.h"     //A70LV_Doulas_0076
#include "appIllumination.h"        //A70LV_Doulas_0212
#include "appEnvironment.h"         //A70LV_Doulas_0294
#include "utilCounterAPI.h"         //A70LV_Doulas_0311
#include "utilCLICmdAPI.h"
#include "halFormatter.h"
#include "palGui.h"
#include "palImgMgr.h"
#include "palGeoAPI.h"

extern sGUI_CALLBACK GuiCb;

static sPAL_DATA_PATH_INFORMATION   m_sPalDataPathInfoSub;
static PsSYSTEM_CONFIGURATION       m_psSysConfigurationSub;        //Default system settings, like TI gpConfiguration
static sDATA_PATH_SOURCE_DESC       m_sSourceDescSub;
static sVIDEO_FORMAT                m_FrontEndVideoFormatSub = {eVIDEO_COLORDEPTH_8BIT,eVIDEO_COLORSPACE_RGB,eVIDEO_COLORRANGE_FULL,eVIDEO_SCANNING_PROGRESSIVE,0,0,eVGA_SYNC_TYPE_UNKNOW,eVIDEO_YUV_UNKNOW};   //A70LV_Doulas_0109 modify    //A70LV_Doulas_0076
static sVIDEO_TIMING                m_FrontEndVideoTimingSub;           //A70LV_Doulas_0176
static UINT8                        m_ucPanelChangeingSub = ets_OFF;

static void palDataPath_ResetTimeInStateSub(void)
{
    m_sPalDataPathInfoSub.ulStateStartTicks = xTaskGetTickCount();
}

/****************************************************************************/
/*                 */
/****************************************************************************/
///////////////////////////////////////////////////////////////////////////////////
///@brief palDataPath_TimeElapsedInState : Returns time elapsed in the current state in milliseconds
///@return UINT32 - Number tick past
///@author Roger Chen
///////////////////////////////////////////////////////////////////////////////////
UINT32 palDataPath_TimeElapsedInStateSub(void)
{
    TickType_t ulTicks;

    ulTicks = xTaskGetTickCount();

    if(m_sPalDataPathInfoSub.ulStateStartTicks > ulTicks)
    {
        return (0xFFFFFFFF - m_sPalDataPathInfoSub.ulStateStartTicks + 1 + ulTicks) / portTICK_RATE_MS;
    }
    else
    {
        return (ulTicks - m_sPalDataPathInfoSub.ulStateStartTicks) / portTICK_RATE_MS;
    }
}


static void palDataPath_StopCurrentOperationSub(void)
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

static eMAIL_BOX_EXEC_CODE palDataPathSub_MailBox_Send(xQueueHandle xQueueHandle, EventGroupHandle_t xEventGroupHandle,
                                                    UINT16 uiMsgID, INT32 lBlockTime,
                                                    UINT32 ulParam1, UINT32 *pulParam2, BOOL bDispose)    //A70LV_Doulas_0278
{
    eMAIL_BOX_EXEC_CODE eMailExecCode = eMAIL_BOX_EXEC_CODE_PASS;

    LOG_MSG(db_APP_DATAPATH, "palDataPathSub_MailBox_Send Event[%d]\r\n", uiMsgID);

#if (SYSTEM_OS_TYPE == POSIX_COMPLIANT)
    eMailExecCode = palMailBox_Send_MsgQueue(m_sPalDataPathInfoSub.xMsgQueue, QUEUE_DATAPATHSUB_NAME, uiMsgID, lBlockTime, ulParam1, pulParam2);
#else
	eMailExecCode = palMailBox_Send(xQueueHandle, xEventGroupHandle, uiMsgID, lBlockTime, ulParam1, pulParam2, bDispose);
#endif

    if(eMailExecCode != eMAIL_BOX_EXEC_CODE_PASS)
    {
        LOG_MSG(db_ALWAYS, "(func:%s, line:%d) : palDataPathSub_MailBox_Send Fail [%d] [%d]!\r\n", __FUNCTION__, __LINE__, uiMsgID, eMailExecCode);
    }

    return eMailExecCode;
}

eEXEC_CODE palDataPath_GetPathReadySub(void)
{
    UINT32 temp;
    BOOL bIsReady = FALSE;

    /* Wait up to 10 seconds for the datapath to finish processing init */
    /* message and then this message
    */
    if(palDataPathSub_MailBox_Send(m_sPalDataPathInfoSub.xMsgQueue, m_sPalDataPathInfoSub.xEventGroupHandle, eDATA_PATH_MSG_GETREADY,
                       10000, 0, (UINT32*)&bIsReady, FALSE) != eMAIL_BOX_EXEC_CODE_PASS)
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



static void palDataPath_DisplaySFGSub(UINT8 ucScalerCH, UINT8 ucFillColor, BOOL bEnable)
{
    halScaler_DisplaySFG(ucScalerCH, ucFillColor, bEnable);
    m_sSourceDescSub.eActiveDisplay = eDATA_PATH_ACTIVE_DISPLAY_SFG;
}

#if 0
static void palDataPath_DisplaySplashSub(void)
{
    halScaler_DisplaySplash();
    m_sSourceDescSub.eActiveDisplay = eDATA_PATH_ACTIVE_DISPLAY_SPLASH;

}
#endif /* 0 */

static void palDataPath_TurnOffSplashSub(void)      //A70LV_Doulas_0032
{
    m_sSourceDescSub.eActiveDisplay = eDATA_PATH_ACTIVE_DISPLAY_EXTERNAL;
    //GuiCb.fpGui_SendSplashEventCb(0);
    if((m_sPalDataPathInfoSub.ePanelTimingId == ePANEL_ID_1080P_120HZ) ||
       (m_sPalDataPathInfoSub.ePanelTimingId == ePANEL_ID_WUXGA_120HZ))    //A70LV_Doulas_0377 Modify//A70LV_Doulas_0154 Modify
    {
        palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL);
    }
    else
    {
        palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL);
    }
}


static void palDataPath_GotoStateSub(UINT32 ulNewState)
{
    eSOURCE_STATE ucSourceState = eSOURCE_STATE_CHECKING_FOR_SIGNAL;    //A70LV_Doulas_0075
    static eSOURCE_STATE ucLastSourceState = eDATA_PATH_STATE_INVALID;    //G100_Simon_0035

    switch(ulNewState)
    {
        case eDATA_PATH_STATE_SPLASH_AT_STARTUP:
            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : eDATA_PATH_STATE_SPLASH_AT_STARTUP\r\n", __FUNCTION__, __LINE__);
        //    palDataPath_DisplaySplashSub();   //A70LV_Doulas_0032 remove
#if 0
            /* load splash */
            source_DisplaySplash(&sourceDesc);
            dispfmt_ConfigureForSource(TRUE);
            pictcont_ConfigureForInternalSource();
#endif
            /* poll every 0.5 seconds if time has elasped */
            m_sPalDataPathInfoSub.lPollPeriod = 500;//500ms
            break;

        case eDATA_PATH_STATE_SCALER_PORT_CONFIG:
            /* poll every 0.5 seconds if time has elasped */
            palImgMgr_SetInputSource(eSOURCE_WINDOW_SUB, m_sSourceDescSub.eConnector);   //A70LV_Doulas_0007
            if(halScaler_PIP_PBP_Enable_Get() || palDataMgr_GetBackupRestoreExecution())      //G100_Owen_0012 //A70LV_Doulas_0120 //H30K_Wesley_0004
            {
               halFrontEndCtrl_SubInput_Set((UINT8*)&m_sSourceDescSub.eConnector);    //set input source
            }
            m_sPalDataPathInfoSub.lPollPeriod = 200;//200ms     //A70LV_Doulas_0007
            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : eDATA_PATH_STATE_SCALER_PORT_CONFIG %d\r\n", __FUNCTION__, __LINE__,m_sSourceDescSub.eConnector);
            m_sSourceDescSub.ucSourcelostCount = 0;  //ZU860_Doulas_0104
            break;

        case eDATA_PATH_STATE_SUSPENDED:
        default:
            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : eDATA_PATH_STATE_SUSPENDED\r\n", __FUNCTION__, __LINE__);
            /* suspend the task indefinitely */
            m_sPalDataPathInfoSub.lPollPeriod = 0;
            m_sSourceDescSub.ucSourcelostCount = 0;  //ZU860_Doulas_0104
            break;

        case eDATA_PATH_STATE_TPG_DISPLAYED:
            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : eDATA_PATH_STATE_TPG_DISPLAYED\r\n", __FUNCTION__, __LINE__);
            m_sSourceDescSub.ucSourcelostCount = 0;  //ZU860_Doulas_0104
#if 0
            /* suspend the task indefinitely, alert GUI */
            datapathTaskPollPeriod = 0;
            guiSourceStatusMessage(GUI_SOURCE_TPG);
#endif
            break;

        case eDATA_PATH_STATE_BEGIN_SCAN:
        {
            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : eDATA_PATH_STATE_BEGIN_SCAN\r\n", __FUNCTION__, __LINE__);

            UINT8 ucInputKeyFunction = 0;
            UINT8 ucLogoChg = 0;    //G100_Owen_0062
            /* manually flush mailbox to clean out old AutoLock status messages */
            /* without losing user messages                                     */
            if(m_sSourceDescSub.ucSourcelostCount < 200)  //ZU860_Doulas_0104
                m_sSourceDescSub.ucSourcelostCount++;
            palDataMgr_Data_Access(edcSHOW_SUBSOURCE_MESSAGES, edaREAD, &ucSourceState) ;   //A70LV_Doulas_0075 Add sub source status
            palDataPath_FreezeChecking(eSOURCE_WINDOW_SUB);   //A70LV_Doulas_0223 Freeze check

            //palDataMgr_Data_Access(edcINPUT_KEY, edaREAD, &ucInputKeyFunction) ;
            if(palDataMgr_DataCode_Control(edcINPUT_KEY) == eFUNC_CONTROL_ENABLE) //A70LV_Larry_0298
            {
                palDataMgr_Data_Access(edcINPUT_KEY, edaREAD, &ucInputKeyFunction) ;
            }
            else
            {
                ucInputKeyFunction = eSOURCE_KEY_CHANGE_SOURCE;
            }

            if((m_sSourceDescSub.ucSourcelostCount >= SOURCE_LOST_TIME)&&
               (m_sSourceDescSub.eConnector == eCM_SOURCE_VGA)&&
               (halScaler_PIP_PBP_Enable_Get()))    //ZU860_Doulas_0104 modify
            {
                if((m_sSourceDescSub.ucSourcelostCount == SOURCE_LOST_TIME))
                {
                    ucSourceState = eSOURCE_STATE_SIGNAL_OUT_OF_RANGE;
                    palDataMgr_Data_Access(edcSHOW_SUBSOURCE_MESSAGES, edaWRITE_RAM_ONLY_NO_ACTION, &ucSourceState);
                }
            }
            else if(ucInputKeyFunction == eSOURCE_KEY_CHANGE_SOURCE_AUTO && ucSourceState != eSOURCE_STATE_SEARCHING)
            {
                ucSourceState = eSOURCE_STATE_SEARCHING;
                palDataMgr_Data_Access(edcSHOW_SUBSOURCE_MESSAGES, edaWRITE_RAM_ONLY_NO_ACTION, &ucSourceState);
            }
            else if(ucSourceState != eSOURCE_STATE_CHECKING_FOR_SIGNAL)
            {
                ucSourceState = eSOURCE_STATE_CHECKING_FOR_SIGNAL;
                palDataMgr_Data_Access(edcSHOW_SUBSOURCE_MESSAGES, edaWRITE_RAM_ONLY_NO_ACTION, &ucSourceState);
            }

            uPALIMGMGR_INFO uInfo = {.sConfig_NoSignalOutput.eWindow = eSOURCE_WINDOW_SUB,
                                     .sConfig_NoSignalOutput.ucDisplayOutput = TRUE};
            palImgMgr_Config_NoSignalOutput(&uInfo);      //A70LV_Doulas_0142 //A70LV_Doulas_0120 show black sub  //A35G2_CDS_Simon_0017

            #if 0
            eResult = halScaler_Freeze_Get(0,&bFreezeEn);     //A70LV_Doulas_0101
            if((bFreezeEn == TRUE) && (eResult == eHAL_SCALER_EXEC_CODE_PASS))
            {
                if(palDataPath_Main_Or_Sub_SourceMonitor() == FALSE)
                {
                    GuiCb.fpGui_DataCode_Value_SetCb(edcIMAGE_FREEZE, 0);
                    GuiCb.fpGui_Send_OSD_ExitCb();
                }
            }
            #endif
            if(palDataPath_IsSourceMonitor() == FALSE)    //ZU860_Doulas_0079
            {
                palDataPath_AutoDisableMenuTransparency();   //A70LV_Doulas_0122 auto dsiable Menu Transparency
            }

            /* poll every 0.5 seconds if time has elasped */
            m_sPalDataPathInfoSub.lPollPeriod = 100;//100ms     //A70LV_Doulas_0120 modify//A70LV_Doulas_0007
            palDataPath_ShowLogoSub();              //A70LV_Doulas_0276 //A70LV_Doulas_0075 remove  //A70LV_Doulas_0032
        }
        break;

        case eDATA_PATH_STATE_LOOK_FOR_SYNCS:

            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : eDATA_PATH_STATE_LOOK_FOR_SYNCS\r\n", __FUNCTION__, __LINE__);
            m_sPalDataPathInfoSub.lPollPeriod = 100;//100ms

            break;

        case eDATA_PATH_STATE_ATTEMPT_LOCK:
            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : eDATA_PATH_STATE_ATTEMPT_LOCK\r\n", __FUNCTION__, __LINE__);
            if((m_sSourceDescSub.ucSourcelostCount < SOURCE_LOST_TIME) ||
               (m_sSourceDescSub.eConnector != eCM_SOURCE_VGA) ||
               (halScaler_PIP_PBP_Enable_Get() == FALSE) )          //A70LV_Doulas_0329 modify
            {
                ucSourceState = eSOURCE_STATE_SETTING_UP_IMAGE; //A70LV_Larry_0350
                palDataMgr_Data_Access(edcSHOW_SUBSOURCE_MESSAGES, edaWRITE_RAM_ONLY_NO_ACTION, &ucSourceState) ;      //A70LV_Doulas_0075 Add sub source status
            }

            /* syncs found -- wait to lock to source */
            /* poll every 100 milliseconds if the source is detected */
            m_sPalDataPathInfoSub.lPollPeriod = 100;//100ms  //A70LV_Doulas_0007
            break;

        case eDATA_PATH_STATE_AUTO_PHASE:     //A70LV_Doulas_0007
            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : eDATA_PATH_STATE_AUTO_PHASE\r\n", __FUNCTION__, __LINE__);
            /* poll every 50 milliseconds if the source is detected */
            m_sPalDataPathInfoSub.lPollPeriod = 50;
            break;

        case eDATA_PATH_STATE_AUTO_POSITION:     //A70LV_Doulas_0007
            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : eDATA_PATH_STATE_AUTO_POSITION\r\n", __FUNCTION__, __LINE__);
            /* poll every 50 milliseconds if the source is detected */
            halScaler_AutoPsitionStart(eSOURCE_WINDOW_SUB,TRUE);       //A70LV_Doulas_0118
            m_sPalDataPathInfoSub.lPollPeriod = 50;
            break;

        case eDATA_PATH_STATE_MONITOR_SOURCE:
            ucSourceState = eSOURCE_STATE_SHOW_SOURCE_RESOLUTION;
#ifdef C789_2D_NO_FRAME_LOCK       //A70LV_Doulas_0311 modify
            if((halScaler_IS_3D_Enable() == ets_ON) ||
               (palImgMgr_WB_OutputChenged_Get())    ||
                (palSystem_PanelID_Get() == ePANEL_ID_WUXGA_120HZ) ||    //A70LV_Doulas_0377 Add
               (palSystem_PanelID_Get() == ePANEL_ID_1080P_120HZ))      //ZU860_Doulas_0083
            {
            //    palEnvironment_LightSourceBlankingOn_Set(TRUE,2);	//A70Gen2_Doulas_0026 remove
            }
            else
            {
            //    palEnvironment_LightSourceBlankingOn_Set(TRUE,1);	//A70Gen2_Doulas_0026 remove
            }
#else
            //palEnvironment_LightSourceBlankingOn_Set(TRUE,5);   //H30K_Doulas_0028//A70LV_Doulas_0294
#endif
            palDataMgr_Data_Access(edcSHOW_SUBSOURCE_MESSAGES, edaWRITE_RAM_ONLY_NO_ACTION, &ucSourceState) ;      //A70LV_Doulas_0075 Add sub source status
            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : eDATA_PATH_STATE_MONITOR_SOURCE\r\n", __FUNCTION__, __LINE__);
            m_sPalDataPathInfoSub.ucSyncLockCount = 0;      //A70LV_Doulas_0009 Add
            halScaler_ConfigureForDisplay(1, 1);       //A70LV_Doulas_0007
            //palDataPath_DisplaySFGSub(1, 5, FALSE);    //A70LV_Doulas_0007
            m_sPalDataPathInfoSub.lPollPeriod = 1000;//1000ms    //A70LV_Doulas_0164 modify //A70LV_Doulas_0009
            palDataPath_TurnOffSplashSub();     //A70LV_Doulas_0032
            #ifdef SCALER_C821_C789
            halC789Ctrl_V_Start_Checking();  //A70LV_Doulas_0042
            #endif
            if(halScaler_PIP_PBP_Enable_Get())    //A70LV_Doulas_0182 modify, Sub CH DDP not setting on the C821 3D 1080P120hz output
            {
                palDataMgr_Format_Normal_WithSemaphoreSet();     //A70LV_Doulas_0046
                palDataPath_MenuTransparencyEnableSet(TRUE); //GuiCb.fpGui_OSD_MenuTransparencyEnableSetCb(TRUE);     //A70LV_Doulas_0122 enable Menu Transparency
                palDataMgr_UI_EventSend(edcUI_EVENT_SUB_SOURCE_INFOR_MSG, TRUE, NULL); //GuiCb.fpGui_SendSubSourceInfoMessageCb();
            }
            m_sSourceDescSub.ucSourcelostCount = 0;         //A70LV_Doulas_0329

            palEnvironment_FEOPD_Cnt_Set(10); //G100_Steven_0059
			palDataMgr_OPDSourceInfo(1);
            //appGui_SendUpdateOSDEvent();        //A70LV_Doulas_0208 Add

            if(m_ucPanelChangeingSub == ets_ON)
            {
                halFormatter_ChannelSourceSet(DISP_EXTERNAL_SOURCE); //A35G2_CDS_Simon_0061
                palSystem_WaitWheelStable(); //A35G2_CDS_Simon_0061
                m_ucPanelChangeingSub = ets_OFF;
		#if 0
                eEXEC_CODE eResult = eEXEC_CODE_PASS;
                UINT8 ucRetry = 100;

                do
                {
                    eResult = palDataMgr_Data_Access(edcPICTURE_MUTE, edaWRITE_RAM_ONLY_WITH_ACTION, &m_ucPanelChangeingSub);

                    if(eResult == eEXEC_CODE_MUTEX_LOCKED)  //A35G2_Simon_0101
                    {
                        ucRetry--;
                        MS_SLEEP(100);
                    }
                    else
                    {
                        break;
                    }

                }while(ucRetry > 0);
		#endif
            }
            palLANProcSendToLAN(edcAUTO_IMAGE); //H30K_Doulas_0019
            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : eDATA_PATH_STATE_MONITOR_SOURCE  End\r\n", __FUNCTION__, __LINE__);      //A70LV_Doulas_0216
            break;
    }

    palDataPath_ResetTimeInStateSub();
    m_sPalDataPathInfoSub.eDataPathState = (eDATA_PATH_STATE)ulNewState;

    if(ulNewState == eDATA_PATH_STATE_BEGIN_SCAN)   //G100_Simo_0035
    {
        if(ucLastSourceState != eDATA_PATH_STATE_BEGIN_SCAN && ucLastSourceState != eDATA_PATH_STATE_LOOK_FOR_SYNCS)
        {
            palLANProcForceCheckUpdateDataSub(); //HICC2_Doulas_0113
        }
    }

    if(ulNewState == eDATA_PATH_STATE_MONITOR_SOURCE)  //HICC2_Doulas_0053
    {
        if(ucLastSourceState != eDATA_PATH_STATE_MONITOR_SOURCE)
        {
            palLANProcForceCheckUpdateDataSub(); //HICC2_Doulas_0113
        }
    }

    if(ucLastSourceState != m_sPalDataPathInfoSub.eDataPathState)
    {
        if((ulNewState >= eDATA_PATH_STATE_SCALER_PORT_CONFIG) &&
           (ulNewState <= eDATA_PATH_STATE_ATTEMPT_LOCK))
        {
            m_sPalDataPathInfoSub.ucSourceConnectChange = 0;
        }
        palLANProcSendToLAN((UINT16)edcIS_SUB_SOURCE_LOCK);
        //UINT8 Data = 0;
        //LOG_MSG(db_ALWAYS, "edcIS_SUB_SOURCE_LOCK set %d\n", Data);
        //utilIpc_NotifyDatacodeChanged(edcIS_SUB_SOURCE_LOCK);
        //palDataMgr_Data_Access(edcIS_SOURCE_LOCK, edaWRITE_RAM_ONLY_WITH_ACTION, &Data);
    }

    ucLastSourceState = m_sPalDataPathInfoSub.eDataPathState;
}

static void palDataPath_StartDisplaySub(void)
{
//    UINT8 ucIsLogoOn = 0; //A70LV_Doulas_0025
#if 0
    adccontrol_powerNormal();
    dvicontrol_powerNormal();
    deccontrol_powerNormal();
#endif

#if 1
    INT16 uiRetry = 120;
    while(halScaler_PowerNormalReadyGet((UINT8)eSOURCE_WINDOW_SUB) == FALSE)  //not ready   //A35G2_CDS_Simon_0034
    {
        MS_SLEEP(100);
        //LOG_MSG(db_APP_DATAPATH, "sub wait hal ready (%d)\n", uiRetry);

        if(uiRetry-- <= 0)
        {
            LOG_MSG(db_APP_DATAPATH, "sub hal power normal not ready\n");
            break;
        }
    }
#else
    MS_SLEEP(4000);
#endif

    palDataPath_FreezeChecking(eSOURCE_WINDOW_SUB);  //G100_Simon_0044
    palDataPath_InitInputSourceSub();  //A70LV_Doulas_0007
    palImgMgr_SetInputSource(eSOURCE_WINDOW_SUB, m_sSourceDescSub.eConnector);   //A70LV_Doulas_0007
    //halScaler_PowerNormal(1, m_sPalDataPathInfoSub.ePanelTimingId);    //init CH2 //A35G2_BRC_Casper_0031

//#if(CURRENT_PLATFORM == PLATFORM_4K_C790 || CURRENT_PLATFORM == PLATFORM_2K_C787)
    //halWarping_PowerNormal(m_sPalDataPathInfoSub.ePanelTimingId);
//#endif //A70LV_Larry_0051 mask

//    palDataMgr_Data_Access(edcSPLASH_STARTUP, edaREAD, (PUINT8)&ucIsLogoOn);  //A70LV_Doulas_0032 remove //A70LV_Doulas_0025

    //Add C734 OSD hardware init function after halScaler_PowerNormal(in PC platform demo board will connect after this function), OSD should init complete before Splash display
    //appGui_HWInit();

    #if 0       //A70LV_Doulas_0032 Modify
    if(ucIsLogoOn == eLOGO_PATTERN_FACTORY_LOGO)        //A70LV_Doulas_0025 modify
    {
        palDataPath_GotoStateSub(eDATA_PATH_STATE_SPLASH_AT_STARTUP);
    }
    else
    {   if(ucIsLogoOn == eLOGO_PATTERN_BLUE)
            palDataPath_DisplaySFGSub(0, eFILL_COLOR_BLUE2 , TRUE);
        else if(ucIsLogoOn == eLOGO_PATTERN_BLACK)
            palDataPath_DisplaySFGSub(0, eFILL_COLOR_BLACK2 , TRUE);
        else
            palDataPath_DisplaySFGSub(0, eFILL_COLOR_WHITE2 , TRUE);
        palDataPath_GotoStateSub(eDATA_PATH_STATE_SCALER_PORT_CONFIG);
    }
    #else
    palDataPath_DisplaySFGSub(1, eFILL_COLOR_BLACK2 , TRUE);
    #if 1
    palDataPath_GotoStateSub(eDATA_PATH_STATE_SPLASH_AT_STARTUP);
    #else
    palDataPath_GotoStateSub(eDATA_PATH_STATE_SCALER_PORT_CONFIG);      //A70LV_Doulas_0043
    #endif
    #endif

}


static void palDataPath_StateMachineProcSub(UINT16 uiMsgID)
{
    //BOOL bSyncLock; //A70LV_Doulas_0007
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_FAIL; //A70LV_Doulas_0009 Add
    static UINT8 ucCheckTimer = 0;  //A70LV_Doulas_0240
    UINT8 ucVal = 0;

    if((palSystem_PowerStateGet() == ePOWER_STATE_STANDBY) || (palSystem_PowerStateGet() == ePOWER_STATE_UPGRADE)) //A70LV_Larry_0112
    {
        return;
    }

    if(palDataMgr_CurTestPatternGet() != eTID_OFF) //test pattern on
    {
        if(m_sPalDataPathInfoSub.eDataPathState == eDATA_PATH_STATE_MONITOR_SOURCE) //HICC2_Doulas_0145
        {
            if(palImgMgr_SYNC_Lock_Get(eSOURCE_WINDOW_SUB) != eHAL_SCALER_EXEC_CODE_PASS)
            {
                LOG_MSG(db_APP_DATAPATH, "HSG Pattern sub LOSTLOCK\r\n");
                palDataPath_GotoStateSub(eDATA_PATH_STATE_LOOK_FOR_SYNCS);
            }
        }
        return;
    }

    switch(m_sPalDataPathInfoSub.eDataPathState)
    {
        case eDATA_PATH_STATE_SPLASH_AT_STARTUP:
        {
            UINT8 ucValue = 0 ;                                    //T100_Simon_0100 Start

            palDataMgr_Data_Access(edcFIRSTSTARTUPFLAG, edaREAD, &ucValue);
            if((palDataMgr_PIN_Protect_Checking() == ePASSWORD_PROTECT_UNLOCK) && (ucValue == ets_OFF))
            {
                LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d)eDATA_PATH_STATE_SPLASH_AT_STARTUP GoTo eDATA_PATH_STATE_SCALER_PORT_CONFIG \r\n", __FUNCTION__, __LINE__);
                palDataPath_GotoStateSub(eDATA_PATH_STATE_SCALER_PORT_CONFIG);
            }

            switch(uiMsgID)
            {
                case eDATA_PATH_MSG_RESYNC:     //A70LV_Doulas_0004
                    LOG_MSG(db_APP_DATAPATH, " eDATA_PATH_MSG_RESYNC --Sub\r\n");
                    palDataPath_ResetTimeInStateSub();
                break;
            }
        }
        break;

        case eDATA_PATH_STATE_SCALER_PORT_CONFIG:
            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d)%d : eDATA_PATH_STATE_SCALER_PORT_CONFIG\r\n", __FUNCTION__, __LINE__,uiMsgID);

            switch(uiMsgID) //A70LV_Doulas_0004
            {
                default:
                #if 0   //A70LV_Doulas_0004
                if(halScaler_PortConfig(1) == eHAL_SCALER_EXEC_CODE_PASS)
                {
                    halScaler_ConfigureForDisplay(1, 1);//RGB Color Space
                    palDataPath_DisplaySFGSub(1, 5, FALSE);        //A70LV_Doulas_0003 modify
                    palDataPath_GotoStateSub(eDATA_PATH_STATE_BEGIN_SCAN);
                }
                #endif
                if(halScaler_PIP_PBP_Enable_Get())  //G100_Owen_0033
                {
                    halScaler_Resync_Init(eSOURCE_WINDOW_SUB);
                }
                palDataPath_GotoStateSub(eDATA_PATH_STATE_BEGIN_SCAN);  //A70LV_Doulas_0004
                break;

                case eDATA_PATH_MSG_PANEL_CHANGE:   //A70LV_Doulas_0154
                    LOG_MSG(db_APP_DATAPATH, " eDATA_PATH_MSG_PANEL_CHANGE Sub\r\n");
                    halScaler_DisplaySFG(1, 5, TRUE);
                    //MS_SLEEP(4000);     //A70LV_Doulas_0098
                    if(halScaler_Panel_Set(1,m_sPalDataPathInfoSub.ePanelTimingId) == eHAL_SCALER_EXEC_CODE_PASS)
                    {
                        palDataPath_GotoStateSub(eDATA_PATH_STATE_BEGIN_SCAN);
                    }
                    else
                    {
                        halScaler_DisplaySFG(1, 5, FALSE);
                    }
                    break;

                case eDATA_PATH_MSG_RESYNC:     //A70LV_Doulas_0004
                    LOG_MSG(db_APP_DATAPATH, " eDATA_PATH_MSG_RESYNC\r\n");
                    //palDataPath_GotoStateSub(eDATA_PATH_STATE_SCALER_PORT_CONFIG);
                    break;
            }
            break;

        case eDATA_PATH_STATE_SUSPENDED:
            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d)%d : eDATA_PATH_STATE_SUSPENDED\r\n", __FUNCTION__, __LINE__,uiMsgID);
            break;

        case eDATA_PATH_STATE_TPG_DISPLAYED:
        //    LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d)%d : eDATA_PATH_STATE_TPG_DISPLAYED\r\n", __FUNCTION__, __LINE__,uiMsgID);
            break;

        case eDATA_PATH_STATE_BEGIN_SCAN:
        //    LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d)%d : eDATA_PATH_STATE_BEGIN_SCAN --\r\n", __FUNCTION__, __LINE__,uiMsgID);

            switch(uiMsgID)
            {
                case eDATA_PATH_MSG_FLUSHED:
                    LOG_MSG(db_APP_DATAPATH, " eDATA_PATH_MSG_FLUSHED\r\n");
                    palDataPath_GotoStateSub(eDATA_PATH_STATE_LOOK_FOR_SYNCS);
                    break;

                case eDATA_PATH_MSG_RESYNC:     //A70LV_Doulas_0004
                    LOG_MSG(db_APP_DATAPATH, " eDATA_PATH_MSG_RESYNC\r\n");
                    palDataPath_GotoStateSub(eDATA_PATH_STATE_BEGIN_SCAN);
                    break;

                case eDATA_PATH_MSG_PANEL_CHANGE:   //A70LV_Doulas_0005
                    LOG_MSG(db_APP_DATAPATH, " eDATA_PATH_MSG_PANEL_CHANGE Sub\r\n");
                    halScaler_DisplaySFG(1, 5, TRUE);
                    //MS_SLEEP(4000);     //A70LV_Doulas_0098
                    if(halScaler_Panel_Set(1,m_sPalDataPathInfoSub.ePanelTimingId) == eHAL_SCALER_EXEC_CODE_PASS)
                    {
                        palDataPath_GotoStateSub(eDATA_PATH_STATE_BEGIN_SCAN);
                    }
                    else
                    {
                        halScaler_DisplaySFG(1, 5, FALSE);
                    }
                    break;

                case eDATA_PATH_MSG_NONE:   //A70LV_Doulas_0007
                    {
                        //A70LV_Doulas_0076 modify
                        #if 1   //A70LV_Doulas_0120
                        UINT8 ucVal;
                        eRESULT eResult2 = rcINVALID;
                        eSOURCE_STATE ucSourceState = eSOURCE_STATE_SHOW_SOURCE_RESOLUTION;     //ZU860_Doulas_0128 Add
                        UINT8 ucInputKeyFunction = 0;                                           //ZU860_Doulas_0128 Add

                        UINT8 ucBurnInEnable = 0; //A70LV_Doulas_0106
                        palDataMgr_Data_Access(edcBURNIN_ENABLE, edaREAD, &ucBurnInEnable);
                        if((palDataMgr_PIN_Protect_Checking() == ePASSWORD_PROTECT_LOCKED) && (ucBurnInEnable == ets_OFF))
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

                    #ifdef SCALER_C821_C789  //H30K_Doulas_0032
                        if((halScaler_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_OFF) &&
                           (halScaler_PIP_PBP_Enable_Get() == ets_OFF))                     //A70LV_Doulas_0240 check sub
                    #else
                        if(halScaler_PIP_PBP_Enable_Get() == ets_OFF)
                    #endif
                        {
                            break;
                        }

                        if(m_sPalDataPathInfoSub.uiPanelChange)
                        {
                            m_sPalDataPathInfoSub.uiPanelChange--;//避免change panel當下，太早抓到source，觸發後續semaphore timeout
                            break;
                        }

                        // Bit 1 retrn Sub Source Status
                        // Bit 5 ~ 7 retrn Crrent Sub Source ID
                        // 00:HDMI1, 01:HDMI2, 02:DisplayPort, 03:HDBASET, 04:3GSDI/12GSDI,
                        eResult2 = halFrontEndCtrl_videoReady_Get(&ucVal);
                        LOG_MSG(db_HAL_RESERVED19, "Sub (func:%s, line:%d)  videoReady :0x%02X \r\n", __FUNCTION__, __LINE__, ucVal);
                        if((eResult2 == rcSUCCESS) && (halScaler_PIP_PBP_Enable_Get() != ets_OFF))
                        {
                            //LOG_MSG(db_ALWAYS, "(func:%s, line:%d)  videoReady :%d (%d,%d)\r\n", __FUNCTION__, __LINE__, ucVal, (ucVal>>5)&0x07, (UINT8)CM2GUI(edcSUB_INPUT, m_sSourceDescSub.eConnector));
                            if(((ucVal>>5)&0x07) != (UINT8)CM2GUI(edcSUB_INPUT, m_sSourceDescSub.eConnector))
                            {
                                eResult2 = rcERROR;
                            }
                        }
                        #ifdef SCALER_C821_C789  //H30K_Doulas_0005
                        if((((halScaler_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_FRAMEPACKING) ||
                             (halScaler_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_SIDEBYSIDE) ||
                             (halScaler_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_TOPANDBOTTOM)) &&
                            (eResult2 == rcSUCCESS) && (ucVal & 0x01) && (((ucVal>>2)&0x07) == (UINT8)CM2GUI(edcMAIN_INPUT, palDataPath_Connector_Get())) ) ||     //A35G2_CDS_Simon_0051
                            (((halScaler_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_OFF) ||
                             (halScaler_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_FRAME_SEQUENTIAL)) &&
                            (eResult2 == rcSUCCESS) && (ucVal & 0x02) )
                            )       //A70LV_Doulas_0159 modify //A70LV_Doulas_0154 modify
                        #else
                        if((eResult2 == rcSUCCESS) && (ucVal & 0x02))
                        #endif
                        {
                            UINT8 ucVideoFormat[eFE_MSG_VIDEO_FORMAT_SUB_SZ]={0};
                            #ifdef SCALER_C821_C789  //H30K_Doulas_0005
                            if((halScaler_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_FRAMEPACKING) ||
                               (halScaler_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_SIDEBYSIDE) ||
                               (halScaler_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_TOPANDBOTTOM))     //A70LV_Doulas_0159 modify //A70LV_Doulas_0154
                            {
                                LOG_MSG(db_HAL_RESERVED19, "Sub) videoFormat_Get\r\n");
                                eResult2 = halFrontEndCtrl_videoFormat_Get(ucVideoFormat);
                            }
                            else
                            #endif
                            {
                                LOG_MSG(db_HAL_RESERVED19, "Sub) videoFormat Sub Get\r\n");
                                eResult2 = halFrontEndCtrl_videoFormatSub_Get(ucVideoFormat);    //A70LV_Doulas_0112
                            }

                            if(eResult2 == rcSUCCESS)
                            {
                                if(halScaler_PIP_PBP_Enable_Get() == ets_OFF)    //A70LV_Doulas_0255 modify main and sub different setting
                                {
                                    palImgMgr_IintChannelSetting(eSOURCE_WINDOW_SUB);
                                    if((ucVal & 0x03) == 0x03)
                                    {
                                        #ifdef SCALER_C821_C789  //H30K_Doulas_0009
                                        palDataPath_Input3D_PanelSet2();
                                        #endif
                                    }
                                }
                                m_FrontEndVideoFormatSub.u8VideoColorDepth = ucVideoFormat[0];
                                m_FrontEndVideoFormatSub.u8VideoColorSpace = ucVideoFormat[1];
                                m_FrontEndVideoFormatSub.u8VideoDynamicRange = ucVideoFormat[2];
                                m_FrontEndVideoFormatSub.u8VideoScanning =  ucVideoFormat[3];
                                m_FrontEndVideoFormatSub.u8VideoAVIInfoDet =  ucVideoFormat[4];
                                m_FrontEndVideoFormatSub.u8Video3DFormat =  ucVideoFormat[5];
                                m_FrontEndVideoFormatSub.u8VideoVGASyncType = ucVideoFormat[6];    //A70LV_Doulas_0109
                                m_FrontEndVideoFormatSub.u8VideoColorYUV    = ucVideoFormat[7];    //A70LV_Doulas_0109
                                m_FrontEndVideoFormatSub.u8VideoDownScaling = ucVideoFormat[8];    //A70LV_Doulas_0154
                                m_FrontEndVideoFormatSub.u8VideoDualPixelMode = ucVideoFormat[9];  //A70LV_Doulas_0154
                                m_FrontEndVideoFormatSub.u8VideoHDRType       = ucVideoFormat[10];         //ZU860_Doulas_0100
                                m_FrontEndVideoFormatSub.u8VideoExtentedColorimetry = ucVideoFormat[11];   //ZU860_Doulas_0100
                                LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d)  eDATA_PATH_STATE_BEGIN_SCAN (%d,%d,%d)(%d,%d,%d)(%d,%d)(%d,%d)(%d,%d)(%d)\r\n", __FUNCTION__, __LINE__,ucVideoFormat[0],ucVideoFormat[1],ucVideoFormat[2],ucVideoFormat[3],ucVideoFormat[4],ucVideoFormat[5]
                                                        ,ucVideoFormat[6],ucVideoFormat[7],ucVideoFormat[8],ucVideoFormat[9],ucVal,ucVideoFormat[10],ucVideoFormat[11],ucVal);    //A70LV_Doulas_0154 modify//A70LV_Doulas_0109

                                if(halScaler_PIP_PBP_Enable_Get())    //ZU860_Doulas_0104
                                {
                                    if(m_sSourceDescSub.eConnector == eCM_SOURCE_VGA)
                                    {
                                        if(halScaler_IsVsync120or100Hz_Get(eSOURCE_WINDOW_SUB))
                                        {
                                            eSOURCE_STATE ucSourceState = eSOURCE_STATE_SHOW_SOURCE_RESOLUTION;

                                            palDataMgr_Data_Access(edcSHOW_SUBSOURCE_MESSAGES, edaREAD, &ucSourceState);
                                            if(ucSourceState != eSOURCE_STATE_SIGNAL_OUT_OF_RANGE)
                                            {
                                                ucSourceState = eSOURCE_STATE_SIGNAL_OUT_OF_RANGE;
                                                palDataMgr_Data_Access(edcSHOW_SUBSOURCE_MESSAGES, edaWRITE_RAM_ONLY_NO_ACTION, &ucSourceState);
                                                palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL);
                                                m_sPalDataPathInfoSub.lPollPeriod = 1000;
                                                palDataPath_ResetTimeInStateSub();
                                                //halScaler_SourceOutputOff(eSOURCE_WINDOW_SUB);     //ZU860_Doulas_0128
                                            }
                                            break;
                                        }
                                    }
                                    #if 0
									else if(m_sSourceDescSub.eConnector == eCM_SOURCE_DVI)	//G100_Doulas_0045 Add
                                    {
                                        if(halScaler_IsVsync120or100Hz_Get(eSOURCE_WINDOW_SUB))	//not support 120/100hz
                                        {
                                            eSOURCE_STATE ucSourceState = eSOURCE_STATE_SHOW_SOURCE_RESOLUTION;

                                            palDataMgr_Data_Access(edcSHOW_SUBSOURCE_MESSAGES, edaREAD, &ucSourceState);
                                            if(ucSourceState != eSOURCE_STATE_SIGNAL_OUT_OF_RANGE)
                                            {
                                                ucSourceState = eSOURCE_STATE_SIGNAL_OUT_OF_RANGE;
                                                palDataMgr_Data_Access(edcSHOW_SUBSOURCE_MESSAGES, edaWRITE_RAM_ONLY_NO_ACTION, &ucSourceState);
                                                palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL);
                                                m_sPalDataPathInfoSub.lPollPeriod = 1000;
                                                palDataPath_ResetTimeInStateSub();
                                                //halScaler_SourceOutputOff(eSOURCE_WINDOW_SUB);
                                            }
                                            break;
                                        }
                                    }
                                    #endif
                                    else
                                    {
                                        if(m_FrontEndVideoFormatSub.u8Video3DFormat || m_FrontEndVideoFormatSub.u8VideoDualPixelMode)
                                        {
                                            eSOURCE_STATE ucSourceState = eSOURCE_STATE_SHOW_SOURCE_RESOLUTION;

                                            palDataMgr_Data_Access(edcSHOW_SUBSOURCE_MESSAGES, edaREAD, &ucSourceState);
                                            if(ucSourceState != eSOURCE_STATE_SIGNAL_OUT_OF_RANGE)
                                            {
                                                ucSourceState = eSOURCE_STATE_SIGNAL_OUT_OF_RANGE;
                                                palDataMgr_Data_Access(edcSHOW_SUBSOURCE_MESSAGES, edaWRITE_RAM_ONLY_NO_ACTION, &ucSourceState);
                                                palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL);
                                                m_sPalDataPathInfoSub.lPollPeriod = 1000;
                                                palDataPath_ResetTimeInStateSub();
                                                //halScaler_SourceOutputOff(eSOURCE_WINDOW_SUB);     //ZU860_Doulas_0128
                                                uPALIMGMGR_INFO uInfo = {.sConfig_NoSignalOutput.eWindow = eSOURCE_WINDOW_SUB,
                                                                         .sConfig_NoSignalOutput.ucDisplayOutput = TRUE};
                                                palImgMgr_Config_NoSignalOutput(&uInfo);  //A35G2_CDS_Simon_0017
                                            }
                                            break;
                                        }
                                    }
                                }
                                palDataPath_FrontEndVideoInfoSub_Set();
#ifdef NOT_USING_SHUTTER_COVER_TRANSIENT
#else
                                if(halScaler_PIP_PBP_Enable_Get() && (m_ucPanelChangeingSub == ets_ON))
                                {
                                    palDataMgr_Data_Access(edcPICTURE_MUTE, edaWRITE_RAM_ONLY_WITH_ACTION, &m_ucPanelChangeingSub);
                                }
#endif /* NOT_USING_SHUTTER_COVER_TRANSIENT */
                                palDataPath_GotoStateSub(eDATA_PATH_STATE_LOOK_FOR_SYNCS);
                            }
                        }
                        else        //ZU860_Doulas_0128 Add
                        {
                            palDataMgr_Data_Access(edcSHOW_SUBSOURCE_MESSAGES, edaREAD, &ucSourceState);
                            if(ucSourceState == eSOURCE_STATE_SIGNAL_OUT_OF_RANGE)
                            {
                                m_sSourceDescSub.ucSourcelostCount = 0;  //A70LV_Doulas_0332
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
                                    palDataMgr_Data_Access(edcSHOW_SUBSOURCE_MESSAGES, edaWRITE_RAM_ONLY_NO_ACTION, &ucSourceState);
                                    palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL);
                                }
                                else if(ucSourceState != eSOURCE_STATE_CHECKING_FOR_SIGNAL)
                                {
                                    ucSourceState = eSOURCE_STATE_CHECKING_FOR_SIGNAL;
                                    palDataMgr_Data_Access(edcSHOW_SUBSOURCE_MESSAGES, edaWRITE_RAM_ONLY_NO_ACTION, &ucSourceState);
                                    palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL);
                                }
                            }
                        }
                        #else
                        UINT8 ucBurnInEnable = 0; //A70LV_Doulas_0106
                        palDataMgr_Data_Access(edcBURNIN_ENABLE, edaREAD, &ucBurnInEnable);
                        if((palDataMgr_PIN_Protect_Checking() == ePASSWORD_PROTECT_LOCKED) && (ucBurnInEnable == ets_OFF))
                        {
                            break;
                        }

                        palDataPath_GotoStateSub(eDATA_PATH_STATE_LOOK_FOR_SYNCS);
                        #endif
                    }
                    break;

                default:        //A70LV_Doulas_0004

                    break;
            }

            break;

        case eDATA_PATH_STATE_LOOK_FOR_SYNCS:
        //    LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d)%d : eDATA_PATH_STATE_LOOK_FOR_SYNCS --", __FUNCTION__, __LINE__,uiMsgID);

            //TODO
            //uiMsgID = (UINT16)eDATA_PATH_MSG_SYNCSDETECTED; //A70LV_Doulas_0004

            switch(uiMsgID)
            {
                case eDATA_PATH_MSG_SYNCSDETECTED:
                    LOG_MSG(db_APP_DATAPATH, " eDATA_PATH_MSG_SYNCSDETECTED\r\n");
#if (CURRENT_MAIN_BOARD == ICHIP_BOARD)
                    palDataPath_DisplaySFGSub(1, 5, FALSE);        //A70LV_Doulas_0003 modify
#endif
                    palDataPath_GotoStateSub(eDATA_PATH_STATE_ATTEMPT_LOCK);
                    break;

                case eDATA_PATH_MSG_RESYNC:     //A70LV_Doulas_0004
                    LOG_MSG(db_APP_DATAPATH, " eDATA_PATH_MSG_RESYNC\r\n");
                    palDataPath_GotoStateSub(eDATA_PATH_STATE_BEGIN_SCAN);
                    break;
#if 0

                case DPMSG_STATE_TOGGLEBLANK:
                    datapath_StopCurrentOperation();
                    datapath_GotoState(SUSPENDED);

                    blankScreenEnabled = TRUE;
                    break;

                case DPMSG_STATE_SUSPENDSCAN:
                    datapath_StopCurrentOperation();
                    datapath_GotoState(SUSPENDED);
                    break;
#endif
                case eDATA_PATH_MSG_PANEL_CHANGE:   //A70LV_Doulas_0005
                    LOG_MSG(db_APP_DATAPATH, " eDATA_PATH_MSG_PANEL_CHANGE Sub\r\n");
                    halScaler_DisplaySFG(1, 5, TRUE);
                    //MS_SLEEP(4000);     //A70LV_Doulas_0098
                    if(halScaler_Panel_Set(1,m_sPalDataPathInfoSub.ePanelTimingId) == eHAL_SCALER_EXEC_CODE_PASS)
                    {
                        palDataPath_GotoStateSub(eDATA_PATH_STATE_BEGIN_SCAN);
                    }
                    else
                    {
                        halScaler_DisplaySFG(1, 5, FALSE);
                    }
                    break;

                case eDATA_PATH_MSG_NONE:   //A70LV_Doulas_0007
                {
               //     LOG_MSG(db_APP_DATAPATH, "\r\n");
#ifdef FIRST_CHKVIDRDY
                    // Bit 1 retrn Sub Source Status
                    // Bit 5 ~ 7 retrn Crrent Sub Source ID
                    // 00:HDMI1, 01:HDMI2, 02:DisplayPort, 03:HDBASET, 04:3GSDI/12GSDI,
                    halFrontEndCtrl_videoReady_Get(&ucVal);
                    if(!ucVal)
                    {
                        ucCheckTimer++;  //A70LV_Doulas_0240
                        if(ucCheckTimer > 3)    //A70LV_Doulas_0240 modify
                        {
                            ucCheckTimer = 0;
                            palDataPath_GotoStateSub(eDATA_PATH_STATE_BEGIN_SCAN);  //A70LV_Doulas_0009 remove
                        }
                        break;
                    }
#endif
                    uPALIMGMGR_INFO uInfo = {.sMeasureInput.ucCH = eSOURCE_WINDOW_SUB};
                    if(palImgMgr_MeasureInput(&uInfo) == eHAL_SCALER_EXEC_CODE_PASS)     //A70LV_Doulas_0031 modify
                    {
                        if(palGeo_ApLinkFlag_Get() == TRUE)                                       //ZU860_Doulas_0138
                        {
                            if(halScaler_PIP_PBP_Enable_Get() == ets_OFF)
                            {
                                if(palDataPath_IsErrorInputGetForTwistOn())
                                {
                                    palDataPath_GotoStateSub(eDATA_PATH_STATE_BEGIN_SCAN);
                                    //if(palDataPath_AlreadyShowDisconnectTwistMsg_Flag_Get() == ets_OFF) //HICC2_Doulas_0053 remove
                                    {
                                        //palDataPath_AlreadyShowDisconnectTwistMsg_Flag_Set(ets_ON); //HICC2_Doulas_0053 remove
                                        palDataMgr_UI_EventSend(edcUI_EVENT_DISCONNECT_TWIST_MSG, TRUE, NULL);
                                    }
                                    break;
                                }
                            }
                        }

                        if((palDataPath_IsSourceLock() == FALSE) && (halScaler_PIP_PBP_Enable_Get() == ets_OFF)) //HICC2_Doulas_0140
                        {
                            break;
                        }

						if((halScaler_PIP_PBP_Enable_Get() == ets_OFF) &&
						    ((m_sPalDataPathInfoSub.ePanelTimingId == ePANEL_ID_1080P_120HZ) ||
                              (m_sPalDataPathInfoSub.ePanelTimingId == ePANEL_ID_WUXGA_120HZ))) //HICC2_Doulas_0136 //A70Gen2_Doulas_0034
                        {
							palDataMgr_DisplayModeChecking();
                    	}
                        halScaler_DisplaySFG(1, 5, TRUE);
                        palDataPath_GotoStateSub(eDATA_PATH_STATE_ATTEMPT_LOCK);
                        if((m_sSourceDescSub.eConnector == eCM_SOURCE_VGA) &&
                           (halScaler_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_OFF))      //A70LV_Doulas_0159 modify//A70LV_Doulas_0154 modify //A70LV_Doulas_0112
                        {
                            UINT16 uiVal;
                            UINT32 udVal;
                            UINT16 uiADCGain[3];
                            UINT16 uiADCOffset[3];
                            UINT16 uiH_Active;  //A70LV_Doulas_0200
                            UINT16 uiV_Active;  //A70LV_Doulas_0200
                            UINT16 uiV_Freq;    //A70LV_Doulas_0200

                            halScaler_VGA_H_Total_Get(eSOURCE_WINDOW_SUB,&uiVal);
                            halFrontEndCtrl_VGA_H_Total_Set(&uiVal);
                            udVal = (UINT32)uiVal * halScaler_uiHFreq_Get(eSOURCE_WINDOW_SUB);     //A70LV_Doulas_0115
                            halFrontEndCtrl_VGA_Pixel_Clock_Set(udVal);     //A70LV_Doulas_0115

                            if(m_FrontEndVideoFormatSub.u8VideoVGASyncType == eVGA_SYNC_TYPE_SOG)    //A70LV_Doulas_0124 Set ADC gain/offset
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

                            uiH_Active = halScaler_Input_H_Active_Get(eSOURCE_WINDOW_SUB);      //A70LV_Doulas_0200
                            uiV_Active = halScaler_Input_V_Active_Get(eSOURCE_WINDOW_SUB);
                            halScaler_InputVertRefresh2_Get(eSOURCE_WINDOW_SUB,&uiV_Freq);
                            halFrontEndCtrl_VGA_TIMING_Set(uiH_Active,uiV_Active,uiV_Freq);
                        }
                        ucCheckTimer = 0;   //A70LV_Doulas_0240
                    }
                    else if(palDataPath_TimeElapsedInStateSub() >= (m_psSysConfigurationSub->Datapath.uiSplashAtStartupTimeout * 1000)) //2s
                    {
                        ucCheckTimer++;  //A70LV_Doulas_0240
                        if(ucCheckTimer > 3)    //A70LV_Doulas_0240 modify
                        {
                            ucCheckTimer = 0;
                            LOG_MSG(db_APP_DATAPATH, "Sub Window LOOK_FOR_SYNCS TimeOut ! (%d)\r\n",palDataPath_TimeElapsedInStateSub());
                            palDataPath_GotoStateSub(eDATA_PATH_STATE_BEGIN_SCAN);  //A70LV_Doulas_0009 remove
                        }
                    }
                }
                break;

                default:


                    /*if(palDataPath_TimeElapsedInStateSub() >= 3000)  //A70LV_Doulas_0004
                    {
                        palDataPath_GotoStateSub(eDATA_PATH_STATE_BEGIN_SCAN);
                    }*/

                    break;
#if 0
                    //----- Roger Added 20061013  Start
#ifdef __ENABLE_SIGNAL_PRETEST

                    if(datapath_TimeElapsedInState() >= 3000  ||
                        !corevariable_GetIsSignalPretestOkVal())
                    {
                        /* if we are searching all connectors, give up after 1 second */
                        /* cannot find syncs on the current connector, so cycle to next connector */
                        datapath_StopCurrentOperation();
                        datapath_CycleSource(&sourceDesc);
                        datapath_GotoState(BEGIN_SCAN);
                    }

                    break;
#else//__ENABLE_SIGNAL_PRETEST

                    if(datapath_TimeElapsedInState() >= 3000)
                    {
                        /* if we are searching all connectors, give up after 1 second */
                        /* cannot find syncs on the current connector, so cycle to next connector */
                        datapath_StopCurrentOperation();
                        datapath_CycleSource(&sourceDesc);
                        datapath_GotoState(BEGIN_SCAN);
                    }

                    break;
#endif//__ENABLE_SIGNAL_PRETEST
                    //----- Roger Added 20061013  End
#endif
            }

            break;

        case eDATA_PATH_STATE_ATTEMPT_LOCK:
            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d)%d : eDATA_PATH_STATE_ATTEMPT_LOCK --", __FUNCTION__, __LINE__,uiMsgID);

            switch(uiMsgID)
            {
                case eDATA_PATH_MSG_LOCKED:
#if 0
                    //----- Roger Added 20061013  Start
                    {
                        ALC_StatusStruct alc_stat;
                        ALC_GetStatus(&alc_stat);

                        /**************************Reject some timing*****************************/
                        dbmsg_ftrace(DBM_ALWAYS, " H %d , V %d \r\n", alc_stat.ASMHRes,  alc_stat.ASMVRes);

#if defined( __CORE_EP752_SYSTEM) ||defined( __CORE_EP761_SYSTEM) ||defined( __CORE_EP763_SYSTEM)

                        if(sourceDesc.source == SRC_VGA1  || sourceDesc.source == SRC_VGA2 || sourceDesc.source == SRC_DVI_A ||
                            sourceDesc.source == SRC_RCA_ADC_COMPONENT)
                        {
                            if((alc_stat.ASMHRes == 720 && alc_stat.ASMVRes == 480 && alc_stat.PortIsInterlaced == 1) ||//480i
                                (alc_stat.ASMHRes == 720 && alc_stat.ASMVRes == 576 && alc_stat.PortIsInterlaced == 1)/* ||//576i
                (alc_stat.ASMHRes == 1920 && alc_stat.ASMVRes == 1080 && !alc_stat.PortIsInterlaced)*/)//1080P
                            {
                                datapath_StopCurrentOperation();
                                datapath_CycleSource(&sourceDesc);
                                datapath_GotoState(BEGIN_SCAN);
                            }
                            else
                            {
                                datapath_GotoState(MONITOR_SOURCE);
                            }
                        }
                        else
                        {
                            datapath_GotoState(MONITOR_SOURCE);
                        }

#endif

                        /**************************Reject some timing*****************************/
                    }

                    //----- Roger Added 20061013  End
#else
                    LOG_MSG(db_APP_DATAPATH, " eDATA_PATH_MSG_LOCKED\r\n");
                    //halScaler_ConfigureForDisplay(1, 1);//RGB Color Space   //A70LV_Doulas_0007
                    //palDataPath_DisplaySFGSub(1, 5, FALSE);     //A70LV_Doulas_0007
                    if(palDataPath_AutoCheckColorSpaceChangeSub() == TRUE)
                    {
                        LOG_MSG(db_APP_DATAPATH, " Sub palDataPath_AutoCheckColorSpaceChangeSub 2 \r\n");     //ZU860_Doulas_0136
                        palDataPath_GotoStateSub(eDATA_PATH_STATE_BEGIN_SCAN);
                    }
                    else
                    {
                        palDataPath_GotoStateSub(eDATA_PATH_STATE_MONITOR_SOURCE);
                    }

#endif
                    break;

                case eDATA_PATH_MSG_LOSTLOCK:
                case eDATA_PATH_MSG_REACQUIRE:
                    LOG_MSG(db_APP_DATAPATH, " eDATA_PATH_MSG_LOSTLOCK21\r\n");
#if 0
                    //----- Roger Added 20061013  Start
                    /* cannot find source on the current connector, so cycle to next connector */
                    datapath_StopCurrentOperation();
                    datapath_CycleSource(&sourceDesc);
                    //----- Roger Added 20061013  End
#endif
                    palDataPath_GotoStateSub(eDATA_PATH_STATE_BEGIN_SCAN);
                    break;
#if 0

                case DPMSG_ALC_PHASESTART:
                    /* display intermediate result */
                    datapath_ConfigureForExternalSource();
                    DISP_SetFreeze(TRUE);
                    break;

                case DPMSG_ALC_PHASEEND:
                    DISP_SetFreeze(FALSE);
                    break;

                case DPMSG_STATE_TOGGLEBLANK:
                    datapath_StopCurrentOperation();
                    datapath_GotoState(SUSPENDED);

                    blankScreenEnabled = TRUE;
                    break;

                case DPMSG_STATE_SUSPENDSCAN:
                    datapath_StopCurrentOperation();
                    datapath_GotoState(SUSPENDED);
                    break;
#endif

                case eDATA_PATH_MSG_RESYNC:     //A70LV_Doulas_0004
                    LOG_MSG(db_APP_DATAPATH, " eDATA_PATH_MSG_RESYNC\r\n");
                    palDataPath_GotoStateSub(eDATA_PATH_STATE_BEGIN_SCAN);
                    break;

                case eDATA_PATH_MSG_PANEL_CHANGE:   //A70LV_Doulas_0005
                    LOG_MSG(db_APP_DATAPATH, " eDATA_PATH_MSG_PANEL_CHANGE Sub\r\n");
                    halScaler_DisplaySFG(1, 5, TRUE);
                    //MS_SLEEP(4000);       //A70LV_Doulas_0098
                    if(halScaler_Panel_Set(1,m_sPalDataPathInfoSub.ePanelTimingId) == eHAL_SCALER_EXEC_CODE_PASS)
                    {
                        palDataPath_GotoStateSub(eDATA_PATH_STATE_BEGIN_SCAN);
                    }
                    else
                    {
                        halScaler_DisplaySFG(1, 5, FALSE);
                    }
                    break;

                case eDATA_PATH_MSG_NONE:   //A70LV_Doulas_0007
                    LOG_MSG(db_APP_DATAPATH, "\r\n");
                    if(palImgMgr_SYNC_Lock_Get(eSOURCE_WINDOW_SUB) == eHAL_SCALER_EXEC_CODE_PASS)     //A70LV_Doulas_0009
                    {
                        if(halScaler_PIP_PBP_Enable_Get())    //A70LV_Doulas_0329
                        {
                            if(m_sSourceDescSub.eConnector == eCM_SOURCE_VGA)
                            {
                                if(halScaler_IsVsync120or100Hz_Get(eSOURCE_WINDOW_SUB))
                                {
                                    palDataPath_GotoStateSub(eDATA_PATH_STATE_BEGIN_SCAN);
                                    break;
                                }
                            }
                        }

                        if(halScaler_AutoPhaseStart(1) == eHAL_SCALER_EXEC_CODE_PASS)
                        {
                            palEnvironment_LightSourceBlankingOn_Set(TRUE,2);   //A70LV_Doulas_0307
                            MS_SLEEP(120);   //A70LV_Doulas_0311
                            palDataPath_GotoStateSub(eDATA_PATH_STATE_AUTO_PHASE);
                        }
                        else
                        {
                            if(palDataPath_AutoCheckColorSpaceChangeSub() == TRUE)
                            {
                                LOG_MSG(db_APP_DATAPATH, " Sub palDataPath_AutoCheckColorSpaceChangeSub 1 \r\n");     //ZU860_Doulas_0136
                                palDataPath_GotoStateSub(eDATA_PATH_STATE_BEGIN_SCAN);
                            }
                            else
                            {
                                palDataPath_GotoStateSub(eDATA_PATH_STATE_MONITOR_SOURCE);
                            }
                        }
                    }
                    else
                    {
                        palDataPath_GotoStateSub(eDATA_PATH_STATE_BEGIN_SCAN);
                    }
                    break;

                case eDATA_PATH_MSG_RECONFIG_IMAGE:     //A70LV_Doulas_0050
                    LOG_MSG(db_APP_DATAPATH, " eDATA_PATH_MSG_RECONFIG_IMAGE Sub\r\n");
                    halScaler_DisplaySFG(1, 5, TRUE);
                    palDataPath_GotoStateSub(eDATA_PATH_STATE_ATTEMPT_LOCK);
                    break;

                default:
#if 0

                    //----- Roger Added 20061013  Start
                    if(datapath_TimeElapsedInState() >= 15000)
                    {
                        /* give up after 15 seconds */
                        /* cannot find source on the current connector, so cycle to next connector */
                        datapath_StopCurrentOperation();
                        datapath_CycleSource(&sourceDesc);
                        datapath_GotoState(BEGIN_SCAN);
                    }

                    //----- Roger Added 20061013  End
#endif

                    break;
            }

            break;

        case eDATA_PATH_STATE_AUTO_PHASE:   //A70LV_Doulas_0007 Add
            //LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d)%d : eDATA_PATH_STATE_AUTO_PHASE --", __FUNCTION__, __LINE__,uiMsgID);

            switch(uiMsgID)
            {
                case eDATA_PATH_MSG_SYNCSDETECTED:
                    LOG_MSG(db_APP_DATAPATH, " eDATA_PATH_MSG_SYNCSDETECTED\r\n");
#if (CURRENT_MAIN_BOARD == ICHIP_BOARD)
                    palDataPath_DisplaySFGSub(1, 5, FALSE);
#endif
                    palDataPath_GotoStateSub(eDATA_PATH_STATE_ATTEMPT_LOCK);
                    break;

                case eDATA_PATH_MSG_RESYNC:
                    LOG_MSG(db_APP_DATAPATH, " eDATA_PATH_MSG_RESYNC\r\n");
                    palDataPath_GotoStateSub(eDATA_PATH_STATE_BEGIN_SCAN);
                    break;

                case eDATA_PATH_MSG_PANEL_CHANGE:
                    LOG_MSG(db_APP_DATAPATH, " eDATA_PATH_MSG_PANEL_CHANGE Sub\r\n");
                    halScaler_DisplaySFG(1, 5, TRUE);
                    //MS_SLEEP(4000);       //A70LV_Doulas_0098
                    if(halScaler_Panel_Set(1,m_sPalDataPathInfoSub.ePanelTimingId) == eHAL_SCALER_EXEC_CODE_PASS)
                    {
                        palDataPath_GotoStateSub(eDATA_PATH_STATE_BEGIN_SCAN);
                    }
                    else
                    {
                        halScaler_DisplaySFG(1, 5, FALSE);
                    }
                    break;

                case eDATA_PATH_MSG_NONE:
                    if(halScaler_AutoPhase(1) == eHAL_SCALER_EXEC_CODE_PASS)
                    {
                        palDataPath_GotoStateSub(eDATA_PATH_STATE_AUTO_POSITION);
                    }
                    break;

                case eDATA_PATH_MSG_LOSTLOCK:       //A70LV_Doulas_0032
                case eDATA_PATH_MSG_REACQUIRE:
                    LOG_MSG(db_APP_DATAPATH, " eDATA_PATH_MSG_LOSTLOCK22\r\n");
                    palDataPath_GotoStateSub(eDATA_PATH_STATE_BEGIN_SCAN);
                    break;

                case eDATA_PATH_MSG_RECONFIG_IMAGE:     //A70LV_Doulas_0050
                    LOG_MSG(db_APP_DATAPATH, " eDATA_PATH_MSG_RECONFIG_IMAGE Sub\r\n");
                    halScaler_DisplaySFG(1, 5, TRUE);
                    palDataPath_GotoStateSub(eDATA_PATH_STATE_ATTEMPT_LOCK);
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
#if (CURRENT_MAIN_BOARD == ICHIP_BOARD)
                    palDataPath_DisplaySFGSub(1, 5, FALSE);
#endif
                    palDataPath_GotoStateSub(eDATA_PATH_STATE_ATTEMPT_LOCK);
                    break;

                case eDATA_PATH_MSG_RESYNC:
                    LOG_MSG(db_APP_DATAPATH, " eDATA_PATH_MSG_RESYNC\r\n");
                    palDataPath_GotoStateSub(eDATA_PATH_STATE_BEGIN_SCAN);
                    break;

                case eDATA_PATH_MSG_PANEL_CHANGE:
                    LOG_MSG(db_APP_DATAPATH, " eDATA_PATH_MSG_PANEL_CHANGE Sub\r\n");
                    halScaler_DisplaySFG(1, 5, TRUE);
                    //MS_SLEEP(4000);   //A70LV_Doulas_0098
                    if(halScaler_Panel_Set(1,m_sPalDataPathInfoSub.ePanelTimingId) == eHAL_SCALER_EXEC_CODE_PASS)
                    {
                        palDataPath_GotoStateSub(eDATA_PATH_STATE_BEGIN_SCAN);
                    }
                    else
                    {
                        halScaler_DisplaySFG(1, 5, FALSE);
                    }
                    break;

                case eDATA_PATH_MSG_NONE:
                {
                    LOG_MSG(db_APP_DATAPATH, "\r\n");
                    uPALIMGMGR_INFO uInfo = {.sMeasureInput.ucCH = eSOURCE_WINDOW_SUB};
                    if(palImgMgr_MeasureInput(&uInfo) == eHAL_SCALER_EXEC_CODE_PASS)     //A70LV_Doulas_0031 modify
                    {
                        palDataPath_GotoStateSub(eDATA_PATH_STATE_MONITOR_SOURCE);
                    }
                    else
                    {
                        palDataPath_GotoStateSub(eDATA_PATH_STATE_BEGIN_SCAN);
                    }
                }
                break;

                case eDATA_PATH_MSG_LOSTLOCK:       //A70LV_Doulas_0032
                case eDATA_PATH_MSG_REACQUIRE:
                    LOG_MSG(db_APP_DATAPATH, " eDATA_PATH_MSG_LOSTLOCK23\r\n");
                    palDataPath_GotoStateSub(eDATA_PATH_STATE_BEGIN_SCAN);
					break;

                case eDATA_PATH_MSG_RECONFIG_IMAGE:     //A70LV_Doulas_0050
                    LOG_MSG(db_APP_DATAPATH, " eDATA_PATH_MSG_RECONFIG_IMAGE Sub\r\n");
                    halScaler_DisplaySFG(1, 5, TRUE);
                    palDataPath_GotoStateSub(eDATA_PATH_STATE_ATTEMPT_LOCK);
                    break;

                default:

                    break;
            }

            break;

        case eDATA_PATH_STATE_MONITOR_SOURCE:

            //        LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : eDATA_PATH_STATE_MONITOR_SOURCE --", __FUNCTION__, __LINE__);

            switch(uiMsgID)
            {
                case eDATA_PATH_MSG_NONE:        //A70LV_Doulas_0007

                    m_sPalDataPathInfoSub.ucSyncLockCount++;      //A70LV_Doulas_0009 Modify monitor functions
                    #ifdef H30_DEVELOP_TEMP_DEFINE
                    #else
                    if(m_sPalDataPathInfoSub.ucSyncLockCount > 5)      //A70LV_Doulas_0164 Modify
                    {
                        BOOL bFreezeEn = FALSE;
                        m_sPalDataPathInfoSub.ucSyncLockCount = 0;
                        uPALIMGMGR_INFO uInfo = {.sMeasureInput.ucCH = eSOURCE_WINDOW_SUB};
                        eResult = palImgMgr_MeasureInput(&uInfo);      //measure all     //A70LV_Doulas_0031 modify

                        if(eResult != eHAL_SCALER_EXEC_CODE_PASS)   //A35G2_CDS_Simon_0059
                        {
                            LOG_MSG(db_APP_DATAPATH, " eDATA_PATH_MSG_LOSTLOCK27\r\n");
                        }

                        halScaler_Freeze_Get(eSOURCE_WINDOW_SUB,&bFreezeEn);     //A70LV_Doulas_0223 Add
                        if(bFreezeEn == TRUE)
                        {
                            eResult = eHAL_SCALER_EXEC_CODE_PASS;   //keep monitor
                        }
                    }
                    else
                    #endif
                    {
                        eResult = palImgMgr_SYNC_Lock_Get(eSOURCE_WINDOW_SUB);
                        if(eResult == eHAL_SCALER_EXEC_CODE_PASS)   //A70LV_Doulas_0283
                        {
                            if(palDataPath_AutoCheckColorSpaceChangeSub() == TRUE)
                            {
                                LOG_MSG(db_APP_DATAPATH, " eDATA_PATH_MSG_LOSTLOCK25\r\n");     //ZU860_Doulas_0136
                                eResult = eHAL_SCALER_EXEC_CODE_FAIL;
                            }
                        }
                        else
                        {
                            LOG_MSG(db_APP_DATAPATH, " eDATA_PATH_MSG_LOSTLOCK26\r\n");     //ZU860_Doulas_0136
                        }
                    }

                    if(eResult == eHAL_SCALER_EXEC_CODE_PASS)
                    {
                    }
                    else
                    {
                        //BOOL bFreezeEn = FALSE;
                        //halScaler_Freeze_Get(1,&bFreezeEn);
                        //if(bFreezeEn == FALSE)
                        palEnvironment_LightSourceBlankingOn_Set(TRUE,1);   //H30K_Doulas_0028//ZU860_Doulas_0083
                        palDataPath_FreezeChecking(eSOURCE_WINDOW_SUB);   //A70LV_Doulas_0223 Freeze check
                        {
                            palLANProcSendToLAN(edcSUB_INPUT);
                            palLANProcSendToLAN(edcSUB_ASPECT_RATIO);
                            palLANProcSendToLAN(edcSUB_SIGNAL_FORMAT);
                            palLANProcSendToLAN(edcSUB_RESOLUTION);
                            palLANProcSendToLAN(edcSUB_PIXEL_CLOCK);
                            palLANProcSendToLAN(edcSUB_SYNC_TYPE);
                            palLANProcSendToLAN(edcSUB_HORZ_REFRESH);
                            palLANProcSendToLAN(edcSUB_VERT_REFRESH);
                            palLANProcSendToLAN(edcSUB_COLOR_SPACE);

                            palDataMgr_UI_EventSend(edcUI_EVENT_DRAW_PIP_LAYOUT, TRUE, NULL);
                            palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL);

                            LOG_MSG(db_APP_DATAPATH, " eDATA_PATH_MSG_LOSTLOCK24\r\n");
                            halScaler_DisplaySFG(eSOURCE_WINDOW_SUB, 5, TRUE);
                            palDataPath_GotoStateSub(eDATA_PATH_STATE_BEGIN_SCAN);
                        }
                    }
                    break;

                case eDATA_PATH_MSG_LOSTLOCK:
                    LOG_MSG(db_APP_DATAPATH, " eDATA_PATH_MSG_LOSTLOCK25_1\r\n");
                    palEnvironment_LightSourceBlankingOn_Set(TRUE,1);   //H30K_Doulas_0028//ZU860_Doulas_0083
                    halScaler_DisplaySFG(1, 5, TRUE);       //A70LV_Doulas_0003 modify
                    palDataPath_GotoStateSub(eDATA_PATH_STATE_BEGIN_SCAN);   //A70LV_Doulas_0004
#if 0

                    //----- Alex Added 20070516 Start
                    if(!blankScreenEnabled)
                    {
                        sourceDesc.sourceActive = FALSE;
                        guiSourceStatusMessage(GUI_SOURCE_LOST);

                        source_DisplaySFG(&sourceDesc);
                        dispfmt_ConfigureForSource(FALSE);
                        pictcont_ConfigureForInternalSource();
                        datapath_StopCurrentOperation();
                        corevariable_SetIsSourceActiveVal(FALSE);
                        datapath_GotoState(BEGIN_SCAN);
                    }
                    else
                    {
                        //In the last two line datapath_StopCurrentOperation function, sets blankScreenEnabled to false.
                        //Because our approch is different with TI's.
                        //When our spoke osd closing, it will TOGGLEBLANK again to disable blank screen.
                        datapath_StopCurrentOperation();
                        //----- Alex Added 20070516 Start
                        corevariable_SetIsSourceActiveVal(FALSE);
                        //----- Alex Added 20070516 End
                        datapath_GotoState(SUSPENDED);
                        blankScreenEnabled = TRUE;
                    }

                    //----- Alex Added 20070516 End
#endif
                    break;

                case eDATA_PATH_MSG_REACQUIRE:
                    LOG_MSG(db_APP_DATAPATH, " eDATA_PATH_MSG_REACQUIRE\r\n");
#if 0
                    sourceDesc.sourceActive = FALSE;
                    corevariable_SetIsSourceActiveVal(FALSE);

                    if(!blankScreenEnabled)
                    {
                        //----- Roger Added 20061013  Start
                        source_DisplaySFG(&sourceDesc);
                        dispfmt_ConfigureForSource(FALSE);
                        pictcont_ConfigureForInternalSource();
                        guiSourceStatusMessage(GUI_SOURCE_LOST);
                        guiSourceStatusMessage((GUI_SOURCEENUM) sourceDesc.connector);
                        datapath_GotoState(ATTEMPT_LOCK);
                        //----- Roger Added 20061013  End
                    }
                    else
                    {
                        datapath_StopCurrentOperation();
                        datapath_GotoState(SUSPENDED);
                    }

#endif
                    break;
#if 0

                case DPMSG_STATE_TOGGLEBLANK:
                    if(!blankScreenEnabled)
                    {
                        sourceDesc.sourceActive = FALSE;
                        blankScreenEnabled = TRUE;

                        DISP_SetFreeze(TRUE);
                        datapath_StoreSourceDesc();
                        source_DisplaySFG(&sourceDesc);
                        dispfmt_ConfigureForSource(FALSE);
                        pictcont_ConfigureForInternalSource();
                        DISP_SetUnfreezeDelay(125);
                        DISP_SetFreeze(FALSE);
                    }
                    else
                    {
                        /* remove SFG/splash, display external source */
                        DISP_SetFreeze(TRUE);
                        DISP_SetChannelSource(SRC_PRIMARY, DISP_EXTERNAL_SOURCE);

                        sourceDesc.sourceActive = TRUE;
                        blankScreenEnabled = FALSE;
                        /* don't change state, just restore settings for source */
                        sourceDesc.activeDisplay = DP_EXTERNAL;
                        datapath_RetrieveSourceDesc();
                        dispfmt_ConfigureForSource(FALSE);
                        pictcont_ConfigureForExternalSource();

                        DISP_SetUnfreezeDelay(125);
                        DISP_SetFreeze(FALSE);
                    }

                    break;
#endif

                case eDATA_PATH_MSG_RESYNC:
                    LOG_MSG(db_APP_DATAPATH, " eDATA_PATH_MSG_RESYNC\r\n");
                    palDataPath_GotoStateSub(eDATA_PATH_STATE_BEGIN_SCAN);  //A70LV_Doulas_0004
#if 0
                    sourceDesc.sourceActive = FALSE;
                    datapath_StopCurrentOperation();
                    DISP_SetFreeze(TRUE);
                    source_InitializePort1();
                    source_DisplaySFG(&sourceDesc);
                    DISP_SetFreeze(FALSE);
                    dispfmt_ConfigureForSource(FALSE);
                    pictcont_ConfigureForInternalSource();

                    guiSourceStatusMessage(GUI_SOURCE_LOST);
                    corevariable_SetIsSourceActiveVal(FALSE);
                    datapath_GotoState(BEGIN_SCAN);
#endif
                    break;

                //----- Roger Added 20061221  Start
                case eDATA_PATH_MSG_SUSPENDSCAN:
                    LOG_MSG(db_APP_DATAPATH, " eDATA_PATH_MSG_SUSPENDSCAN\r\n");
#if 0
                    //In the monitor state , you can not stop monitoring source.
                    //But in the monitor state, you start the image menu, the image menu
                    //will call datapath_UserStopConnectorScan.
                    //It means scan will be stopped, but it does not.
                    //So we have to change back the variable here
                    corevariable_SetIsConnectorScanStopVal(FALSE);
                    //----- Roger Added 20061221  End
#endif
                    break;

                case eDATA_PATH_MSG_PANEL_CHANGE:   //A70LV_Doulas_0005
                    LOG_MSG(db_APP_DATAPATH, " eDATA_PATH_MSG_PANEL_CHANGE Sub\r\n");
                    halScaler_DisplaySFG(1, 5, TRUE);
                    //MS_SLEEP(4000);   //A70LV_Doulas_0098
                    if(halScaler_Panel_Set(1,m_sPalDataPathInfoSub.ePanelTimingId) == eHAL_SCALER_EXEC_CODE_PASS)
                    {
                        palDataPath_GotoStateSub(eDATA_PATH_STATE_BEGIN_SCAN);
                    }
                    else
                    {
                        halScaler_DisplaySFG(1, 5, FALSE);
                    }
                    break;

                case eDATA_PATH_MSG_RECONFIG_IMAGE:     //A70LV_Doulas_0009
                    LOG_MSG(db_APP_DATAPATH, " eDATA_PATH_MSG_RECONFIG_IMAGE Sub\r\n");
                    halScaler_DisplaySFG(1, 5, TRUE);
                    palDataPath_GotoStateSub(eDATA_PATH_STATE_ATTEMPT_LOCK);
                    break;

            }

        default:
            {       //A70LV_Doulas_0101 Add
                UINT8 ucValue = 0;
                palDataMgr_Data_Access(edcIMAGE_FREEZE, edaREAD, &ucValue);     //A70LV_Doulas_0167
                if(ucValue)     //A70LV_Doulas_0167
                {
                    palDataMgr_UI_EventSend(edcUI_EVENT_OPEN_FREEZE_MENU_MSG, TRUE, NULL);
                }
            }
            break;
    }
}


static void palDataPath_CallbackSub(UINT16 uiMsgID, UINT32 ulParam1, UINT32 *pulParam2)
{
    if((palSystem_PowerStateGet() == ePOWER_STATE_STANDBY) || (palSystem_PowerStateGet() == ePOWER_STATE_UPGRADE)) //A70LV_Larry_0112
    {
        return;
    }

    switch(uiMsgID)
    {
        case eDATA_PATH_MSG_INIT:
            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : eDATA_PATH_MSG_INIT\r\n", __FUNCTION__, __LINE__);
            palDataPath_StartDisplaySub();
            m_sPalDataPathInfoSub.bDataPathReady = TRUE;
            break;

        case eDATA_PATH_MSG_GETREADY:
            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : eDATA_PATH_MSG_GETREADY\r\n", __FUNCTION__, __LINE__);
            *pulParam2 = (UINT32)m_sPalDataPathInfoSub.bDataPathReady;
            break;

        case eDATA_PATH_MSG_GOTOSTATE:
            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : eDATA_PATH_MSG_GOTOSTATE\r\n", __FUNCTION__, __LINE__);
            palDataPath_StopCurrentOperationSub();
            palDataPath_GotoStateSub(ulParam1);
            break;

        case eDATA_PATH_MSG_SETCONNECTOR:
            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : eDATA_PATH_MSG_SETCONNECTOR %d\r\n", __FUNCTION__, __LINE__, ulParam1);   //A70LV_Doulas_0120
            m_sSourceDescSub.eConnector = (eCM_SOURCE_ID)ulParam1;
            palImgMgr_SetInputSource(eSOURCE_WINDOW_SUB, m_sSourceDescSub.eConnector);
            if(halScaler_PIP_PBP_Enable_Get())
            {
                if(palDataPath_Last3DEnable_Get()  && (palDataPath_IsSourceLock() == FALSE))
                {
                    m_ucPanelChangeingSub = ets_ON;
                }
            }
            else
            {
                m_ucPanelChangeingSub = ets_OFF;
            }
            palDataPath_GotoStateSub(eDATA_PATH_STATE_SCALER_PORT_CONFIG);
            break;


        case eDATA_PATH_MSG_FLUSHED:
            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : eDATA_PATH_MSG_FLUSHED\r\n", __FUNCTION__, __LINE__);

            if(ulParam1 != m_sPalDataPathInfoSub.ucFlushIndex)
            {
                return;
            }

            palDataPath_StateMachineProcSub(uiMsgID);
            break;

        case eDATA_PATH_MSG_SYNCSDETECTED:
            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : eDATA_PATH_MSG_SYNCSDETECTED\r\n", __FUNCTION__, __LINE__);
            palDataPath_StateMachineProcSub(uiMsgID);
            break;

        case eDATA_PATH_MSG_LOCKED:
            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : eDATA_PATH_MSG_LOCKED\r\n", __FUNCTION__, __LINE__);
            palDataPath_StateMachineProcSub(uiMsgID);
            break;

        case eDATA_PATH_MSG_REACQUIRE:
            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : eDATA_PATH_MSG_REACQUIRE\r\n", __FUNCTION__, __LINE__);
            palDataPath_StateMachineProcSub(uiMsgID);
            break;

        case eDATA_PATH_MSG_LOSTLOCK:
            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : eDATA_PATH_MSG_LOSTLOCK\r\n", __FUNCTION__, __LINE__);
            palDataPath_StateMachineProcSub(uiMsgID);
            break;

        case eDATA_PATH_MSG_SUSPENDSCAN:
            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : eDATA_PATH_MSG_SUSPENDSCAN\r\n", __FUNCTION__, __LINE__);
            palDataPath_StateMachineProcSub(uiMsgID);
            break;

        case eDATA_PATH_MSG_RESUMESCAN:
            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : eDATA_PATH_MSG_RESUMESCAN\r\n", __FUNCTION__, __LINE__);
            palDataPath_StateMachineProcSub(uiMsgID);
            break;

        case eDATA_PATH_MSG_RESYNC:
            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : eDATA_PATH_MSG_RESYNC\r\n", __FUNCTION__, __LINE__);
            halScaler_Resync_Init(1);    //A70LV_Doulas_0004
            palDataPath_StateMachineProcSub(uiMsgID);
            m_sSourceDescSub.ucSourcelostCount = 0;  //ZU860_Doulas_0104
            break;

        case eDATA_PATH_MSG_RECONFIG_IMAGE:     //A70LV_Doulas_0009
            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : eDATA_PATH_MSG_RECONFIG_IMAGE\r\n", __FUNCTION__, __LINE__);
            palDataPath_StateMachineProcSub(uiMsgID);
            m_sSourceDescSub.ucSourcelostCount = 0;  //ZU860_Doulas_0104
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
            palDataPath_StateMachineProcSub(uiMsgID);

            break;

        default:
            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : DEFAULT\r\n", __FUNCTION__, __LINE__);
            palDataPath_StateMachineProcSub(uiMsgID);
            break;
    }
}



eEXEC_CODE palDataPath_PowerStandbySub(void)
{
    UINT32 temp;

    LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);

    palDataPathSub_MailBox_Send(m_sPalDataPathInfoSub.xMsgQueue, m_sPalDataPathInfoSub.xEventGroupHandle,
                    eDATA_PATH_MSG_GOTOSTATE, -1, eDATA_PATH_STATE_SUSPENDED, &temp, FALSE);

    //TODO
    /*
        1.Front End go to standby
           adccontrol_powerStandby();
           dvicontrol_powerStandby();
           deccontrol_powerStandby();
    */

    palImgMgr_PowerStandby(NULL);

    return eEXEC_CODE_PASS;
}



eEXEC_CODE palDataPath_PowerNormalSub(ePANEL_ID ePanelId)
{
    UINT32 temp;
    LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);

    m_sPalDataPathInfoSub.bDataPathReady = FALSE;

    m_sPalDataPathInfoSub.ePanelTimingId  = ePanelId;
    m_sPalDataPathInfoSub.uiPanelChange = 0;

    palDataPathSub_MailBox_Send(m_sPalDataPathInfoSub.xMsgQueue,
                    m_sPalDataPathInfoSub.xEventGroupHandle,
                    eDATA_PATH_MSG_INIT, -1, 0, &temp, FALSE);

    return eEXEC_CODE_PASS;
}

void palDataPath_TaskSub_Suspend(void)
{
#if 0//(SYSTEM_OS_TYPE == POSIX_COMPLIANT)
	if(m_sPalDataPathInfoSub.bTaskPause == FALSE)
	{
        struct timespec s_timeout;
        clock_gettime(CLOCK_REALTIME, &s_timeout);
        util_TimespecAddms(&s_timeout, APP_TASK_SEMPHORE_WAIT);

        if(pthread_mutex_timedlock(&m_sPalDataPathInfoSub.xTaskMutex, &s_timeout) == 0)
        {
    		m_sPalDataPathInfoSub.bTaskPause = TRUE;
    		pthread_mutex_unlock(&m_sPalDataPathInfoSub.xTaskMutex);
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
    palDataPathSub_MailBox_Send(m_sPalDataPathInfoSub.xMsgQueue, m_sPalDataPathInfoSub.xEventGroupHandle,
                    eDATA_PATH_MSG_GOTOSTATE, -1, eDATA_PATH_STATE_SUSPENDED, &temp, FALSE);

#endif
}

void palDataPath_TaskSub_Resume(void)
{
#if 0//(SYSTEM_OS_TYPE == POSIX_COMPLIANT)
	if(m_sPalDataPathInfoSub.bTaskPause == TRUE)
	{
        struct timespec s_timeout;
        clock_gettime(CLOCK_REALTIME, &s_timeout);
        util_TimespecAddms(&s_timeout, APP_TASK_SEMPHORE_WAIT);

        if(pthread_mutex_timedlock(&m_sPalDataPathInfoSub.xTaskMutex, &s_timeout) == 0)
        {
    		m_sPalDataPathInfoSub.bTaskPause = FALSE;
    		pthread_cond_broadcast(&m_sPalDataPathInfoSub.xTaskCond);

    		pthread_mutex_unlock(&m_sPalDataPathInfoSub.xTaskMutex);
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
    palDataPathSub_MailBox_Send(m_sPalDataPathInfoSub.xMsgQueue, m_sPalDataPathInfoSub.xEventGroupHandle,
                    eDATA_PATH_MSG_GOTOSTATE, -1, eDATA_PATH_STATE_BEGIN_SCAN, &temp, FALSE);

#endif
}


static void* palDataPath_TaskSub(void *pParameters)
{
    eMAIL_BOX_EXEC_CODE eExecCode;

    LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);

    while(1)
    {
        if(!m_sPalDataPathInfoSub.bTaskPause)
        {
#if (SYSTEM_OS_TYPE == POSIX_COMPLIANT)
            if((eExecCode = palMailBox_Receive_MsgQueue(m_sPalDataPathInfoSub.xMsgQueue,
                                                        QUEUE_DATAPATHSUB_NAME,
                                                        (INT32)m_sPalDataPathInfoSub.lPollPeriod,
                                                        palDataPath_CallbackSub)) != eMAIL_BOX_EXEC_CODE_PASS)
#else
            if((eExecCode = palMailBox_Receive(m_sPalDataPathInfoSub.xMsgQueue,
                                               m_sPalDataPathInfoSub.lPollPeriod,
                                               palDataPath_CallbackSub)) != eMAIL_BOX_EXEC_CODE_PASS)
#endif
            {
#if (SYSTEM_OS_TYPE == POSIX_COMPLIANT)
                pthread_mutex_lock(&m_sPalDataPathInfoSub.xTaskMutex);
#endif
                if(eExecCode == eMAIL_BOX_EXEC_CODE_NOMSG)      /* timeout -- used to poll state machine */
                {
    //                LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d): NO MSG EVENT!\r\n", __FUNCTION__, __LINE__);
                    palDataPath_CallbackSub(eDATA_PATH_MSG_NONE, 0, NULL);
                }
                else
                {
                    LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d): Receive Event Error ID [%d]!\r\n", __FUNCTION__, __LINE__, eExecCode);
                }

#if (SYSTEM_OS_TYPE == POSIX_COMPLIANT)
                pthread_mutex_unlock(&m_sPalDataPathInfoSub.xTaskMutex);
#endif
            }

            palSystem_TaskMonitorTimerReset(eTID_SUB_DATAPATH, NULL);  //A35G2_Simon_0075
        }
        else
        {
#if (SYSTEM_OS_TYPE == POSIX_COMPLIANT)
            pthread_cond_wait(&m_sPalDataPathInfoSub.xTaskCond, &m_sPalDataPathInfoSub.xTaskMutex);
            pthread_mutex_unlock(&m_sPalDataPathInfoSub.xTaskMutex);
#endif
        }
    }

}



eEXEC_CODE palDataPath_InitSub(ePANEL_ID ePanelId, const PsSYSTEM_CONFIGURATION psSysConfiguration)
{
#if(CURRENT_RTOS_TYPE != RTOS_STATIC)
    BaseType_t xReturned;
#endif

    LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);

    m_psSysConfigurationSub = psSysConfiguration;

    m_sPalDataPathInfoSub.ePanelTimingId    = ePanelId;
    m_sPalDataPathInfoSub.lPollPeriod       = 0;//Because lPollPeriod to 0 means max delay

    GuiCb = Gui_fpCallbackGet();

#if (SYSTEM_OS_TYPE == POSIX_COMPLIANT)

    //Create Mailbox
    LOG_MSG(db_APP_DATAPATH, "DatapathSub queue create start\n");

    if(palMailBox_Create_MsgQueue(&m_sPalDataPathInfoSub.xMsgQueue, QUEUE_DATAPATHSUB_NAME, &m_sPalDataPathInfoSub.xMsgQueue_attr) != eMAIL_BOX_EXEC_CODE_PASS)
    {
        LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d): Create Msg Queue Fail!\r\n", __FUNCTION__, __LINE__);
        return eEXEC_CODE_FAIL;
    }

    LOG_MSG(db_APP_DATAPATH, "DatapathSub queue create end\n");


    halScaler_Init(1);//A70LV_Doulas_0003 //Must put before palDataPath_PowerStandbySub()

    palDataPath_PowerStandbySub();

    //Create Thread Pasue
    if(pthread_mutex_init(&m_sPalDataPathInfoSub.xTaskMutex, NULL) != 0)
    {
        LOG_MSG(db_ALWAYS, "(func:%s, line:%d): Create xTaskMutex Fail!\r\n", __FUNCTION__, __LINE__);
    }
    m_sPalDataPathInfoSub.bTaskPause = FALSE;

    //Create Thread
    pthread_attr_init(&m_sPalDataPathInfoSub.xTaskHandle_attr);
    pthread_attr_setstacksize(&m_sPalDataPathInfoSub.xTaskHandle_attr, APP_DATAPATH_STACK_SIZE);
//    pthread_attr_setschedpolicy( &m_sPalDataPathInfoSub.xTaskHandle_attr, SCHED_FIFO );
    pthread_attr_setdetachstate( &m_sPalDataPathInfoSub.xTaskHandle_attr, PTHREAD_CREATE_DETACHED );
    pthread_cond_init(&m_sPalDataPathInfoSub.xTaskCond, NULL); //A35G2_BRC_Casper_0051
    INT16 iCreateThreadError = 0 ;
    iCreateThreadError = pthread_create(&m_sPalDataPathInfoSub.xTaskHandle, &m_sPalDataPathInfoSub.xTaskHandle_attr, palDataPath_TaskSub, (void*) NULL);

    if(iCreateThreadError != 0)
    {
        LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d): Create Thread Fail!\r\n", __FUNCTION__, __LINE__);
        return eEXEC_CODE_FAIL;
    }

    LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d): Create Thread Pass! (ID:%d)\r\n", __FUNCTION__, __LINE__ , m_sPalDataPathInfoSub.xTaskHandle);

#else    //freertos

#if(CURRENT_RTOS_TYPE == RTOS_STATIC)

    if(palMailBox_Create_Static(&m_sPalDataPathInfoSub.xMsgQueue, &m_sPalDataPathInfoSub.xEventGroupHandle,
                                &m_sPalDataPathInfoSub.xStaticQueue, m_sPalDataPathInfoSub.ucQueueStorageArea,
                                &m_sPalDataPathInfoSub.xCreatedEventGroup) != eMAIL_BOX_EXEC_CODE_PASS)
    {
        LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d): Create Msg Queue Fail!\r\n", __FUNCTION__, __LINE__);
        return eEXEC_CODE_FAIL;
    }

    halScaler_Init(1);//A70LV_Doulas_0003 //Must put before palDataPath_PowerStandbySub()

#if 0 //A70LV_Larry_0051 mask
#if(CURRENT_PLATFORM == PLATFORM_4K_C790 || CURRENT_PLATFORM == PLATFORM_2K_C787)
    m_sPalDataPathInfoSub.eMode = HAL_WARPING_WARP_MODE_2x2;
    m_sPalDataPathInfoSub.bGridShow = FALSE;
    m_sPalDataPathInfoSub.eMoveIdx = eHAL_WARPING_MOVE_PITCH_100;

    m_sPalDataPathInfoSub.sWarpPjParam.fAxis = 50;
    m_sPalDataPathInfoSub.sWarpPjParam.fDist = 3000;
    m_sPalDataPathInfoSub.sWarpPjParam.fVw = 1000;


    halWarping_Init(m_sPalDataPathInfoSub.eMode,
                    m_sPalDataPathInfoSub.eMoveIdx);
#endif
#endif /* 0 */

    palDataPath_PowerStandbySub();

    m_sPalDataPathInfoSub.xTaskHandle = xTaskCreateStatic(
                                         palDataPath_TaskSub,       /* Function that implements the task. */
                                         "DPTSKSUB",          /* Text name for the task. */
                                         APP_DATAPATH_STACK_SIZE,      /* Number of indexes in the xStack array. */
                                         NULL,    /* Parameter passed into the task. */
                                         APP_DATAPATH_PRIORITY,/* Priority at which the task is created. */
                                         m_sPalDataPathInfoSub.xStack,          /* Array to use as the task's stack. */
                                         &m_sPalDataPathInfoSub.xTaskBuffer);  /* Variable to hold the task's data structure. */

    if(m_sPalDataPathInfoSub.xTaskHandle == NULL)
    {
        LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d): Create Task Fail!\r\n", __FUNCTION__, __LINE__);
        return eEXEC_CODE_FAIL;
    }

#else

    if(palMailBox_Create(&m_sPalDataPathInfoSub.xMsgQueue, &m_sPalDataPathInfoSub.xEventGroupHandle) != eMAIL_BOX_EXEC_CODE_PASS)
    {
        LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d): Create Msg Queue Fail!\r\n", __FUNCTION__, __LINE__);
        return eEXEC_CODE_FAIL;
    }

    halScaler_Init(1);//A70LV_Doulas_0003 //Must put before palDataPath_PowerStandbySub()

    palDataPath_PowerStandbySub();


    xReturned = xTaskCreate(palDataPath_TaskSub, (const char *) "DPTSKSUB",
                            APP_DATAPATH_STACK_SIZE,
                            NULL,
                            APP_DATAPATH_PRIORITY,
                            &m_sPalDataPathInfoSub.xTaskHandle);

    if(xReturned != pdPASS)
    {
        LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d): Create Task Fail!\r\n", __FUNCTION__, __LINE__);
        return eEXEC_CODE_FAIL;
    }
#endif

#endif

    return eEXEC_CODE_PASS;

}


void palDataPath_ResyncSub(void)
{
    UINT32 temp;
    //palDataPath_GotoStateSub(eDATA_PATH_STATE_SCALER_PORT_CONFIG);
    palDataPathSub_MailBox_Send(m_sPalDataPathInfoSub.xMsgQueue,
                    m_sPalDataPathInfoSub.xEventGroupHandle,
                    eDATA_PATH_MSG_RESYNC, -1, 0, &temp, FALSE);    //A70LV_Doulas_0003
}

eEXEC_CODE palDataPath_Panel_ChangeSub(ePANEL_ID ePanelId)   //A70LV_Doulas_0005
{
    UINT32 temp;
    m_sPalDataPathInfoSub.ePanelTimingId = ePanelId;
    if(halScaler_Panel_Set(1, m_sPalDataPathInfoSub.ePanelTimingId) == eHAL_SCALER_EXEC_CODE_PASS)   //ZU860_Doulas_0076 modify
    {
        LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d): palDataPath_Panel_ChangeSub %d\r\n", __FUNCTION__, __LINE__, ePanelId);

        palDataPathSub_MailBox_Send(m_sPalDataPathInfoSub.xMsgQueue,
                        m_sPalDataPathInfoSub.xEventGroupHandle,
                        eDATA_PATH_MSG_PANEL_CHANGE, -1, 0, &temp, FALSE);

        m_sPalDataPathInfoSub.uiPanelChange = 50; //5s
    }
    return eEXEC_CODE_PASS;
}

eEXEC_CODE palDataPath_InitInputSourceSub(void)   //A70LV_Doulas_0029 modify //A70LV_Doulas_0007
{
    INT16 iVAl = 0;		//G100_Doulas_0050 Modify

    palDataMgr_SubSourceChecking(); //H30K_Wesley_0002
    palDataMgr_Data_Access(edcSUB_INPUT, edaREAD, &iVAl);
    m_sSourceDescSub.eConnector = (eCM_SOURCE_ID)iVAl;
    halFrontEndCtrl_SubInput_Set((UINT8*)&m_sSourceDescSub.eConnector);    //set input source

    return eEXEC_CODE_PASS;
}

eEXEC_CODE palDataPath_Reconfig_ImageSub(void)   //A70LV_Doulas_0009
{
    UINT32 temp;
    palDataPathSub_MailBox_Send(m_sPalDataPathInfoSub.xMsgQueue,
                    m_sPalDataPathInfoSub.xEventGroupHandle,
                    eDATA_PATH_MSG_RECONFIG_IMAGE, -1, 0, &temp, FALSE);

    return eEXEC_CODE_PASS;
}

eEXEC_CODE palDataPath_OverscanSetSub(UINT8 ucOverScan)   //A70LV_Doulas_0009
{
    eHAL_SCALER_EXEC_CODE eScalerResult = eHAL_SCALER_EXEC_CODE_FAIL;
    eEXEC_CODE eExecResult = eEXEC_CODE_PASS;

    eScalerResult = halScaler_Overscan_Set(ucOverScan);     //A70LV_Doulas_0020
    if(eScalerResult == eHAL_SCALER_EXEC_CODE_PASS)
    {
        palDataPath_Reconfig_ImageSub();
    }
    else
        eExecResult = eEXEC_CODE_FAIL;

    return eExecResult;
}

eEXEC_CODE palDataPath_UserSetSourceInputSub(eCM_SOURCE_ID eInputSource)    //A70LV_Doulas_0029
{
    eEXEC_CODE eExecResult = eEXEC_CODE_PASS;
    UINT32 temp;

    halScaler_Resync_Init(eSOURCE_WINDOW_SUB);   //A70LV_Doulas_0120
    if(palDataPathSub_MailBox_Send(m_sPalDataPathInfoSub.xMsgQueue,
                       m_sPalDataPathInfoSub.xEventGroupHandle,
                       eDATA_PATH_MSG_SETCONNECTOR, -1, eInputSource, (UINT32*)&temp, FALSE) != eMAIL_BOX_EXEC_CODE_PASS ) //A70LV_Doulas_0295 modify BlockTime
    {
        eExecResult = eEXEC_CODE_FAIL;
    }
    else
    {
        m_sPalDataPathInfoSub.ucSourceConnectChange = 1;
    }

    return eExecResult ;
}

BOOL palDataPath_IsSourceLockSub(void)       //A70LV_Doulas_0032
{
    if(m_sPalDataPathInfoSub.ucSourceConnectChange)
    {
        return FALSE;
    }

    if((m_sPalDataPathInfoSub.eDataPathState == eDATA_PATH_STATE_MONITOR_SOURCE) ||
       (m_sPalDataPathInfoSub.eDataPathState == eDATA_PATH_STATE_ATTEMPT_LOCK) ||
       (m_sPalDataPathInfoSub.eDataPathState == eDATA_PATH_STATE_AUTO_PHASE) ||
       (m_sPalDataPathInfoSub.eDataPathState == eDATA_PATH_STATE_AUTO_POSITION) )   //A70LV_Doulas_0050
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

BOOL palDataPath_IsSourceMonitorSub(void)       //A70LV_Doulas_0098
{
    if(m_sPalDataPathInfoSub.eDataPathState == eDATA_PATH_STATE_MONITOR_SOURCE)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

void palDataPath_SourceLostSetSub(void)
{
    UINT32 temp;
    if(m_sPalDataPathInfoSub.eDataPathState == eDATA_PATH_STATE_MONITOR_SOURCE || m_sPalDataPathInfoSub.eDataPathState == eDATA_PATH_STATE_ATTEMPT_LOCK ||
       m_sPalDataPathInfoSub.eDataPathState == eDATA_PATH_STATE_AUTO_PHASE     || m_sPalDataPathInfoSub.eDataPathState == eDATA_PATH_STATE_AUTO_POSITION)
    {
        palDataPathSub_MailBox_Send(m_sPalDataPathInfoSub.xMsgQueue,
                        m_sPalDataPathInfoSub.xEventGroupHandle,
                        eDATA_PATH_MSG_LOSTLOCK, -1,
                        0, &temp, FALSE);
    }
}

BOOL palDataPath_ShowLogoSub(void)     //A70LV_Doulas_0276 modify//A70LV_Doulas_0032
{
    UINT8 ucValue = 0;

    ucValue = halScaler_PIP_PBP_Enable_Get();   //A70LV_Doulas_0120 modify
    if((ucValue == eCM_SCREEN_MODE_PIP) || (ucValue == eCM_SCREEN_MODE_PBP))    //G100_Owen_0012
    {
        palDataMgr_UI_EventSend(edcUI_EVENT_SUB_SOURCE_INFOR_MSG, TRUE, NULL); //GuiCb.fpGui_SendSubSourceInfoMessageCb();
    }

    return TRUE;
}

eEXEC_CODE palDataPath_InputPixelClockSub_Get(UINT8 *ucValue)   //A70LV_Doulas_0056
{
    eEXEC_CODE eExecResult = eEXEC_CODE_PASS;

    if((halScaler_PIP_PBP_Enable_Get() != ets_OFF) && (palDataPath_IsSourceLockSub()))
    {
        if((m_sPalDataPathInfoSub.ePanelTimingId == ePANEL_ID_1080P_120HZ) ||
           (m_sPalDataPathInfoSub.ePanelTimingId == ePANEL_ID_WUXGA_120HZ))    //A70LV_Doulas_0377 Modify//A70LV_Doulas_0287
        {
            *ucValue = '-';
            *(ucValue+1) = '\0';
        }
        else if(m_FrontEndVideoFormatSub.u8VideoDownScaling)        //A70LV_Doulas_0187 modify
        {
            #if 0
            char aucString[VERSION_STRING_MAX_LENGTH];
            DOUBLE dbVal;
            dbVal = (DOUBLE)m_FrontEndVideoTimingSub.u32VideoPCLK;
            sprintf(aucString, "%d.%03dMHz",(UINT32)dbVal/1000,(UINT32)dbVal%1000);
            memcpy(ucValue, aucString, strlen(aucString)+1);
            #endif

            char aucString[VERSION_STRING_MAX_LENGTH];
            DOUBLE dbPixelClock;
            UINT16 uiVFreq;
            halScaler_InputVertRefresh2_Get(eSOURCE_WINDOW_SUB, &uiVFreq);

            //H/V Total from IT6805 , V sync freq from C821
            dbPixelClock = (DOUBLE)m_FrontEndVideoTimingSub.u16VideoHTotal *   //A35G2_CDS_Simon_0052
                           (DOUBLE)m_FrontEndVideoTimingSub.u16VideoVTotal *
                           (DOUBLE)uiVFreq / 100;
            sprintf(aucString, "%d.%03dMHz",(UINT32)dbPixelClock/1000000,(UINT32)dbPixelClock/1000%1000);
            memcpy(ucValue, aucString, strlen(aucString)+1);
        }
        else
        {
            halScaler_InputPixelClock_Get(eSOURCE_WINDOW_SUB,ucValue);
        }
    }
    else
    {
        *ucValue = '-';
        *(ucValue+1) = '\0';
    }

    return eExecResult;
}

eEXEC_CODE palDataPath_InputSignalFormatSub_Get(UINT8 *ucValue)
{
    eEXEC_CODE eExecResult = eEXEC_CODE_PASS;
    char aucString[VERSION_STRING_MAX_LENGTH];

    if((halScaler_PIP_PBP_Enable_Get() != ets_OFF) && (palDataPath_IsSourceLockSub()))
    {
        if((m_sPalDataPathInfoSub.ePanelTimingId == ePANEL_ID_1080P_120HZ) ||
           (m_sPalDataPathInfoSub.ePanelTimingId == ePANEL_ID_WUXGA_120HZ))    //A70LV_Doulas_0377 Modify//A70LV_Doulas_0287
        {
            *ucValue = '-';
            *(ucValue+1) = '\0';
        }
        else if(m_FrontEndVideoFormatSub.u8VideoDownScaling)        //A70LV_Doulas_0187 modify
        {
            if(m_sSourceDescSub.eConnector == eCM_SOURCE_VGA)
                sprintf(aucString, "Analog");
            else
                sprintf(aucString, "Digital");
            memcpy(ucValue, aucString, strlen(aucString)+1);
        }
        else
        {
            //halScaler_InputSignalFormat_Get(eSOURCE_WINDOW_SUB,ucValue);        //ZU860_Doulas_0108 Modify
            UINT8 ucColorSpace = 0;
            halScaler_FrontEndColorSpace_Get(eSOURCE_WINDOW_SUB,&ucColorSpace);
            if((ucColorSpace == eCOLOR_FORMAT_RGB) ||
               (ucColorSpace == eCOLOR_FORMAT_RGB_LIMIT))
            {
                if(m_sSourceDescSub.eConnector == eCM_SOURCE_VGA)
                    sprintf(aucString, "Analog");
                else
                    sprintf(aucString, "Digital");
                memcpy(ucValue, aucString, strlen(aucString)+1);
            }
            else
            {
                halScaler_InputSignalFormat_Get(eSOURCE_WINDOW_SUB,ucValue);
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

eEXEC_CODE palDataPath_InputResoultionSub_Get(UINT8 *ucValue)
{
    eEXEC_CODE eExecResult = eEXEC_CODE_PASS;

    if((halScaler_PIP_PBP_Enable_Get() != ets_OFF) && (palDataPath_IsSourceLockSub()))
    {
        if((m_sPalDataPathInfoSub.ePanelTimingId == ePANEL_ID_1080P_120HZ) ||
           (m_sPalDataPathInfoSub.ePanelTimingId == ePANEL_ID_WUXGA_120HZ))    //A70LV_Doulas_0377 Modify//A70LV_Doulas_0287
        {
            *ucValue = '-';
            *(ucValue+1) = '\0';
        }
        else if(m_FrontEndVideoFormatSub.u8VideoDownScaling)        //A70LV_Doulas_0187 modify
        {
            char aucString[VERSION_STRING_MAX_LENGTH];
            sprintf(aucString, "%d x %d",m_FrontEndVideoTimingSub.u16VideoHActive ,m_FrontEndVideoTimingSub.u16VideoVActive);
            memcpy(ucValue, aucString, strlen(aucString)+1);
        }
        else
        {
            halScaler_InputResolution_Get(eSOURCE_WINDOW_SUB,ucValue);
        }
    }
    else
    {
        *ucValue = '-';
        *(ucValue+1) = '\0';
    }

    return eExecResult;
}

eEXEC_CODE palDataPath_InputHorzRefreshSub_Get(UINT8 *ucValue)
{
    eEXEC_CODE eExecResult = eEXEC_CODE_PASS;

    if((halScaler_PIP_PBP_Enable_Get() != ets_OFF) && (palDataPath_IsSourceLockSub()))
    {
        if((m_sPalDataPathInfoSub.ePanelTimingId == ePANEL_ID_1080P_120HZ) ||
           (m_sPalDataPathInfoSub.ePanelTimingId == ePANEL_ID_WUXGA_120HZ))    //A70LV_Doulas_0377 Modify//A70LV_Doulas_0287
        {
            *ucValue = '-';
            *(ucValue+1) = '\0';
        }
        else if(m_FrontEndVideoFormatSub.u8VideoDownScaling)        //A70LV_Doulas_0187 modify
        {
            #if 0
            char aucString[VERSION_STRING_MAX_LENGTH];
            DOUBLE dbVal;
            dbVal = (DOUBLE)m_FrontEndVideoTimingSub.u32VideoPCLK *1000 / (DOUBLE)m_FrontEndVideoTimingSub.u16VideoHTotal;
            sprintf(aucString, "%d.%03dkHz",(UINT32)dbVal/1000,(UINT32)dbVal%1000);
            memcpy(ucValue, aucString, strlen(aucString)+1);
            #endif

            char aucString[VERSION_STRING_MAX_LENGTH];
            DOUBLE dbHRefresh;
            UINT16 uiVFreq;
            halScaler_InputVertRefresh2_Get(eSOURCE_WINDOW_SUB, &uiVFreq);

            dbHRefresh = (DOUBLE)m_FrontEndVideoTimingSub.u16VideoVTotal *      //A35G2_CDS_Simon_0052
                         (DOUBLE)uiVFreq / 100;

            sprintf(aucString, "%d.%03dkHz",(UINT32)dbHRefresh/1000,(UINT32)dbHRefresh%1000);
            memcpy(ucValue, aucString, strlen(aucString)+1);

        }
        else
        {
            halScaler_InputHorzRefresh_Get(eSOURCE_WINDOW_SUB,ucValue);
        }
    }
    else
    {
        *ucValue = '-';
        *(ucValue+1) = '\0';
    }

    return eExecResult;
}

eEXEC_CODE palDataPath_InputVertRefreshSub_Get(UINT8 *ucValue)
{
    eEXEC_CODE eExecResult = eEXEC_CODE_PASS;

    if((halScaler_PIP_PBP_Enable_Get() != ets_OFF) && (palDataPath_IsSourceLockSub()))
    {
        if((m_sPalDataPathInfoSub.ePanelTimingId == ePANEL_ID_1080P_120HZ) ||
           (m_sPalDataPathInfoSub.ePanelTimingId == ePANEL_ID_WUXGA_120HZ))    //A70LV_Doulas_0377 Modify//A70LV_Doulas_0287
        {
            *ucValue = '-';
            *(ucValue+1) = '\0';
        }
        #if 0   //A35G2_CDS_Simon_0052 remove
        else if(m_FrontEndVideoFormatSub.u8VideoDownScaling)        //A70LV_Doulas_0187 modify
        {
            char aucString[VERSION_STRING_MAX_LENGTH];
            DOUBLE dbVal;
            dbVal = (DOUBLE)m_FrontEndVideoTimingSub.u32VideoPCLK *1000 / (DOUBLE)m_FrontEndVideoTimingSub.u16VideoHTotal / (DOUBLE)m_FrontEndVideoTimingSub.u16VideoVTotal * 100;
            sprintf(aucString, "%d.%02dHz",(UINT32)dbVal/100,(UINT32)dbVal%100);
            memcpy(ucValue, aucString, strlen(aucString)+1);
        }
        else
        #endif
        {
            halScaler_InputVertRefresh_Get(eSOURCE_WINDOW_SUB,ucValue);
        }
    }
    else
    {
        *ucValue = '-';
        *(ucValue+1) = '\0';
    }

    return eExecResult;
}

eEXEC_CODE palDataPath_InputSyncTypeSub_Get(UINT8 *ucValue)
{
    eEXEC_CODE eExecResult = eEXEC_CODE_PASS;
    char aucString[VERSION_STRING_MAX_LENGTH];

    if((halScaler_PIP_PBP_Enable_Get() != ets_OFF) && (palDataPath_IsSourceLockSub()))
    {
        if((m_sPalDataPathInfoSub.ePanelTimingId == ePANEL_ID_1080P_120HZ) ||
           (m_sPalDataPathInfoSub.ePanelTimingId == ePANEL_ID_WUXGA_120HZ))    //A70LV_Doulas_0377 Modify//A70LV_Doulas_0287
        {
            *ucValue = '-';
            *(ucValue+1) = '\0';
        }
        else if(m_sSourceDescSub.eConnector == eCM_SOURCE_VGA)
        {
            //check sync "Sync on Green" or "Separate"
            if(m_FrontEndVideoFormatSub.u8VideoVGASyncType == eVGA_SYNC_TYPE_SOG)  //A70LV_Doulas_0109 modify
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

eEXEC_CODE palDataPath_InputAspectRatioSub_Get(UINT8 *ucValue)
{
    eEXEC_CODE eExecResult = eEXEC_CODE_PASS;

    if((halScaler_PIP_PBP_Enable_Get() != ets_OFF) && (palDataPath_IsSourceLockSub()))
    {
        halScaler_InputAspectRatio_Get(eSOURCE_WINDOW_SUB,ucValue);
    }
    else
    {
        *ucValue = '-';
        *(ucValue+1) = '\0';
    }

    return eExecResult;
}

eEXEC_CODE palDataPath_InputColorSpaceSub_Get(UINT8 *ucValue)
{
    eEXEC_CODE eExecResult = eEXEC_CODE_PASS;

    if((m_sPalDataPathInfoSub.ePanelTimingId == ePANEL_ID_1080P_120HZ) ||
       (m_sPalDataPathInfoSub.ePanelTimingId == ePANEL_ID_WUXGA_120HZ))    //A70LV_Doulas_0377 Modify//A70LV_Doulas_0287
    {
        *ucValue = '-';
        *(ucValue+1) = '\0';
    }
    else if((halScaler_PIP_PBP_Enable_Get() != ets_OFF) && (palDataPath_IsSourceLockSub()))
    {
        halScaler_FrontEndColorSpaceInfo_Get(eSOURCE_WINDOW_SUB,ucValue);     //A70LV_Doulas_0109 Modify
    }
    else
    {
        *ucValue = '-';
        *(ucValue+1) = '\0';
    }

    return eExecResult;
}

eEXEC_CODE palDataPath_FrontEndVideoInfoSub_Set(void)      //A70LV_Doulas_0112 //A70LV_Doulas_0109 modify //A70LV_Doulas_0076 Add
{
    eEXEC_CODE eExecResult = eEXEC_CODE_PASS;
    UINT8 ucColorSpace;
    UINT8 ucVideoYUV;

    //Color space
    if((m_sSourceDescSub.eConnector == eCM_SOURCE_VGA) &&
       (halScaler_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_OFF))      //A70LV_Doulas_0180 modify 3D color error
    {
        halScaler_FrontEndVideoYUV_Set(eSOURCE_WINDOW_SUB,eVIDEO_YUV_UNKNOW);
        if(m_FrontEndVideoFormatSub.u8VideoVGASyncType == eVGA_SYNC_TYPE_SOG)
            ucColorSpace = eCOLOR_FORMAT_444;
        else
            ucColorSpace = eCOLOR_FORMAT_RGB;
        halScaler_VGA_SYNC_TYPE_Set(eSOURCE_WINDOW_SUB,m_FrontEndVideoFormatSub.u8VideoVGASyncType);
        halScaler_FrontEndColorSpace_Set(eSOURCE_WINDOW_SUB,ucColorSpace);
    }
    else if((m_sSourceDescSub.eConnector == eCM_SOURCE_3GSDI) &&
            (halScaler_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_OFF))     //A70LV_Doulas_0180 modify 3D color error
    {
        halScaler_FrontEndVideoYUV_Set(eSOURCE_WINDOW_SUB,eVIDEO_YUV_UNKNOW);
        ucColorSpace = eCOLOR_FORMAT_422;//eCOLOR_FORMAT_RGB;
        halScaler_FrontEndColorSpace_Set(eSOURCE_WINDOW_SUB,ucColorSpace);
    }
    else
    {
        if(m_FrontEndVideoFormatSub.u8VideoAVIInfoDet == TRUE)
        {
            switch(m_FrontEndVideoFormatSub.u8VideoColorYUV)
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
                    m_FrontEndVideoFormatSub.u8VideoColorYUV = eVIDEO_YUV_UNKNOW;
                    break;
            }
            halScaler_FrontEndVideoYUV_Set(eSOURCE_WINDOW_SUB,ucVideoYUV);

            switch(m_FrontEndVideoFormatSub.u8VideoColorSpace)
            {
                case eVIDEO_COLORSPACE_RGB:
                    if(m_FrontEndVideoFormatSub.u8VideoDynamicRange == eVIDEO_COLORRANGE_FULL)
                        ucColorSpace = eCOLOR_FORMAT_RGB;
                    else
                        ucColorSpace = eCOLOR_FORMAT_RGB_LIMIT;
                    break;

                case eVIDEO_COLORSPACE_Y422:
                    ucColorSpace = eCOLOR_FORMAT_422;  //A352_Simon_0082
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
            halScaler_FrontEndColorSpace_Set(eSOURCE_WINDOW_SUB,ucColorSpace);
        }
        else
        {
            halScaler_FrontEndVideoYUV_Set(eSOURCE_WINDOW_SUB,eVIDEO_YUV_UNKNOW);
            ucColorSpace = eCOLOR_FORMAT_RGB;
            halScaler_FrontEndColorSpace_Set(eSOURCE_WINDOW_SUB,ucColorSpace);
        }
    }

    //Interlaced
    halScaler_FrontEndScanMode_Set(eSOURCE_WINDOW_SUB,m_FrontEndVideoFormatSub.u8VideoScanning);

    #ifndef SCALER_C341  //H30K_Doulas_0011
    if(m_FrontEndVideoFormatSub.u8VideoDownScaling)    //A70LV_Doulas_0176 Add
    #endif
    {
        UINT8 ucVal[15];
        halFrontEndCtrl_Sub_Timing_Get(ucVal);
        m_FrontEndVideoTimingSub.u32VideoPCLK = ucVal[0]+ (ucVal[1]<<8) + (ucVal[2]<<16) +(ucVal[3]<<24);
        m_FrontEndVideoTimingSub.u16VideoHTotal = ucVal[4]+ (ucVal[5]<<8);
        m_FrontEndVideoTimingSub.u16VideoHActive = ucVal[6]+ (ucVal[7]<<8);
        m_FrontEndVideoTimingSub.u16VideoVTotal = ucVal[8]+ (ucVal[9]<<8);
        m_FrontEndVideoTimingSub.u16VideoVActive = ucVal[10]+ (ucVal[11]<<8);
        m_FrontEndVideoTimingSub.u16VideoVRate =  ucVal[12]+ (ucVal[13]<<8);
        palDataPath_FrontEnd_Timing_Set(eSOURCE_WINDOW_SUB,m_FrontEndVideoTimingSub); //H30K_Doulas_0011
        LOG_MSG(db_APP_DATAPATH, "PCLK    =0x%x \r\n",m_FrontEndVideoTimingSub.u32VideoPCLK);
        LOG_MSG(db_APP_DATAPATH, "H-total =%d \r\n",m_FrontEndVideoTimingSub.u16VideoHTotal);
        LOG_MSG(db_APP_DATAPATH, "V-total =%d \r\n",m_FrontEndVideoTimingSub.u16VideoVTotal);
        LOG_MSG(db_APP_DATAPATH, "H-Activ =%d \r\n",m_FrontEndVideoTimingSub.u16VideoHActive);
        LOG_MSG(db_APP_DATAPATH, "V-Activ =%d \r\n",m_FrontEndVideoTimingSub.u16VideoVActive);
        LOG_MSG(db_APP_DATAPATH, "V-Freq  =%d \r\n",m_FrontEndVideoTimingSub.u16VideoVRate);
    }
    return eExecResult;
}

eDATA_PATH_STATE palDataPath_GetDataPathSubState(void)      //A70LV_Doulas_0236
{
    return m_sPalDataPathInfoSub.eDataPathState;
}

BOOL palDataPath_AutoCheckColorSpaceChangeSub(void)        //A70LV_Doulas_0283
{
    eRESULT eResult = rcINVALID;
    UINT8 ucVideoFormat[eFE_MSG_VIDEO_FORMAT_SUB_SZ] = {0};

    if(halScaler_PIP_PBP_Enable_Get() == ets_OFF)
    {
        //if((m_sSourceDescSub.eConnector == eCM_SOURCE_VGA) ||
        //   (m_sSourceDescSub.eConnector == eCM_SOURCE_3GSDI))
        //{
        //    return FALSE;
        //}

        if( halScaler_ColorSpace_Get(eSOURCE_WINDOW_MAIN) !=  eCM_COLOR_SPACE_AUTO)
        {
            return FALSE;
        }

        if((halScaler_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_FRAMEPACKING) ||
            (halScaler_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_SIDEBYSIDE) ||
            (halScaler_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_TOPANDBOTTOM))
            eResult = halFrontEndCtrl_videoFormat_Get(ucVideoFormat);
        else
            eResult = halFrontEndCtrl_videoFormatSub_Get(ucVideoFormat);

        if(eResult == rcSUCCESS)    //A70LV_Doulas_0288 modify
        {
            if(m_FrontEndVideoFormatSub.u8VideoAVIInfoDet != ucVideoFormat[4])
            {
                return TRUE;
            }
            else if(m_FrontEndVideoFormatSub.u8VideoAVIInfoDet == TRUE)
            {
                if(m_FrontEndVideoFormatSub.u8VideoColorSpace != ucVideoFormat[1])
                {
                    return TRUE;
                }
                else if(m_FrontEndVideoFormatSub.u8VideoColorSpace == eVIDEO_COLORSPACE_RGB) // RGB
                {
                    if(m_FrontEndVideoFormatSub.u8VideoDynamicRange != ucVideoFormat[2])
                    {
                        return TRUE;
                    }
                }
                else //YUV
                {
                    if(m_FrontEndVideoFormatSub.u8VideoColorYUV != ucVideoFormat[7])
                    {
                        return TRUE;
                    }
                }
            }
        }
    }
    else
    {
        if(m_sSourceDescSub.eConnector == eCM_SOURCE_3GSDI)       //ZU860_Doulas_0126 modify
        {
            return FALSE;
        }

        //if( halScaler_ColorSpace_Get(eSOURCE_WINDOW_SUB) !=  eCOLOR_SPACE_AUTO)
        //{
        //    return FALSE;
        //}

        eResult = halFrontEndCtrl_videoFormatSub_Get(ucVideoFormat);
        if(eResult == rcSUCCESS)    //A70LV_Doulas_0288 modify
        {
            if(m_sSourceDescSub.eConnector == eCM_SOURCE_VGA)    //ZU860_Doulas_0126 modify
            {
                if(m_FrontEndVideoFormatSub.u8VideoVGASyncType != ucVideoFormat[6])
                {
                    return TRUE;
                }
                else
                {
                    return FALSE;
                }
            }

            if(m_FrontEndVideoFormatSub.u8VideoAVIInfoDet != ucVideoFormat[4])
            {
                return TRUE;
            }
            else if(m_FrontEndVideoFormatSub.u8VideoAVIInfoDet == TRUE)
            {
                if(m_FrontEndVideoFormatSub.u8VideoColorSpace != ucVideoFormat[1])
                {
                    return TRUE;
                }
                else if(m_FrontEndVideoFormatSub.u8VideoColorSpace == eVIDEO_COLORSPACE_RGB) // RGB
                {
                    if(m_FrontEndVideoFormatSub.u8VideoDynamicRange != ucVideoFormat[2])
                    {
                        return TRUE;
                    }
                }
                else //YUV
                {
                    if(m_FrontEndVideoFormatSub.u8VideoColorYUV != ucVideoFormat[7])
                    {
                        return TRUE;
                    }
                }
            }
        }
    }
    return FALSE;
}


UINT8 palDataPath_HDR_Info_Sub_Get(void)    //A65_OPTOMA_Doulas_0149
{
    return m_FrontEndVideoFormatSub.u8VideoHDRType;
}

eEXEC_CODE palDataPath_ColorDepth_Sub_Get(UINT8 *ucValue)
{
    eEXEC_CODE eExecResult = eEXEC_CODE_PASS;
    char aucString[24];
    char acColorDepthString[10] = {0};
    char acColorSpaceString[10] = {0};
    UINT8 ucColorSpace = eVIDEO_COLORSPACE_RGB;
    UINT8 ucColorDepth = eCOLOR_DEPTH_8BIT;

    if(palDataPath_IsSourceLockSub())
    {
        switch(m_sSourceDescSub.eConnector)
        {
            case eCM_SOURCE_VGA:
            case eCM_SOURCE_3GSDI:
                *ucValue = '-';
                *(ucValue+1) = '\0';
                return eExecResult;

            case eCM_SOURCE_HDMI1:
            case eCM_SOURCE_HDMI2:
            case eCM_SOURCE_DVI:
            case eCM_SOURCE_HDBASET:
            case eCM_SOURCE_DISPLAYPORT:
                if(m_FrontEndVideoFormatSub.u8VideoAVIInfoDet == TRUE)
                {
                    switch(m_FrontEndVideoFormatSub.u8VideoColorSpace)
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
                ucColorDepth = m_FrontEndVideoFormatSub.u8VideoColorDepth;
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
                sprintf((char *)acColorSpaceString, "-");
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
                sprintf((char *)acColorDepthString, "-");
                break;
        }
        sprintf(aucString,"%s",  acColorDepthString);
        memcpy(ucValue, aucString, strlen(aucString)+1);
    }
    else
    {
        *ucValue = '-';
        *(ucValue+1) = '\0';
    }

    return eExecResult;
}


