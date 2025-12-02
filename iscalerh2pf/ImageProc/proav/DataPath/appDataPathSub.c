#include "appDataPath.h"
#include "appDataMgr.h"
#include "appSystem.h"
#include "appEnvironment.h"
#include "appLANProcAPI.h"
#include "appIllumination.h"

#include "halScaler.h"
#include "halWarping.h"
#include "utilDbgMsg.h"

#include "palGui.h"
#include "palImgMgr.h"
#include "palFrontEndMgr.h"

sGUI_CALLBACK GuiCb;

static sPAL_DATA_PATH_INFORMATION   m_sPalDataPathInfoSub;
static PsSYSTEM_CONFIGURATION       m_psSysConfigurationSub;        //Default system settings, like TI gpConfiguration
static sDATA_PATH_SOURCE_DESC       m_sSourceDescSub;
static sVIDEO_FORMAT                m_FrontEndVideoFormatSub = {eVIDEO_COLORDEPTH_8BIT,eVIDEO_COLORSPACE_RGB,eVIDEO_COLORRANGE_FULL,eVIDEO_SCANNING_PROGRESSIVE,0,0,eVGA_SYNC_TYPE_UNKNOW,eVIDEO_YUV_UNKNOW};   //A70LV_Doulas_0109 modify    //A70LV_Doulas_0076
static sVIDEO_TIMING                m_FrontEndVideoTimingSub;           //A70LV_Doulas_0176


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
    if(m_sPalDataPathInfoSub.ePanelTimingId == PANEL_3D_OUTPUT)    //A70LV_Doulas_0154 Modify
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
    static eSOURCE_STATE ucLastSourceState = eDATA_PATH_STATE_INVALID;

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
        {
#if 0
            /* poll every 0.5 seconds if time has elasped */
            UINT8 ucInputKeyFunction = 0;
            palDataMgr_Data_Access(edcINPUT_KEY, edaREAD, &ucInputKeyFunction) ;
            palDataMgr_Data_Access(edcSHOW_SUBSOURCE_MESSAGES, edaREAD, &ucSourceState) ;   //A70LV_Doulas_0075 Add sub source status
            if(ucInputKeyFunction == eSOURCE_KEY_CHANGE_SOURCE_AUTO && ucSourceState != eSOURCE_STATE_SEARCHING)
            {
                ucSourceState = eSOURCE_STATE_SEARCHING;
                palDataMgr_Data_Access(edcSHOW_SUBSOURCE_MESSAGES, edaWRITE_RAM_ONLY_NO_ACTION, &ucSourceState);
            }
            else if(ucSourceState != eSOURCE_STATE_CHECKING_FOR_SIGNAL)
            {
                ucSourceState = eSOURCE_STATE_CHECKING_FOR_SIGNAL;
                palDataMgr_Data_Access(edcSHOW_SUBSOURCE_MESSAGES, edaWRITE_RAM_ONLY_NO_ACTION, &ucSourceState);
            }
#endif /* 0 */

            halScaler_SetInputSource(eSOURCE_WINDOW_SUB, m_sSourceDescSub.eConnector);   //A70LV_Doulas_0007
            if(halScaler_PIP_PBP_Enable_Get() != eCM_SCREEN_MODE_OFF)      //A70LV_Doulas_0120
            {
                uPALIMGMGR_INFO uInfo = {.sConfig_NoSignalOutput.eWindow = eSOURCE_WINDOW_SUB,
                                         .sConfig_NoSignalOutput.ucDisplayOutput = 2};//keep vop out
                palImgMgr_Config_NoSignalOutput(&uInfo);
                //halScaler_SclVopWindowOffSet(eSOURCE_WINDOW_SUB, TRUE);
                halScaler_RxPortClear(RXPORT_CHANNEL_1);
                palFrontEndMgr_SubInput_Set((UINT8*)&m_sSourceDescSub.eConnector, TRUE, TRUE);    //set input source
                halScaler_CurrentSourceRxPortSet(eSOURCE_WINDOW_SUB, RXPORT_CHANNEL_1);
                halScaler_RxPortCheck(RXPORT_CHANNEL_1, 0);
            }
            m_sPalDataPathInfoSub.lPollPeriod = 100;//200ms     //A70LV_Doulas_0007
            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : eDATA_PATH_STATE_SCALER_PORT_CONFIG\r\n", __FUNCTION__, __LINE__);
        }
        break;


        case eDATA_PATH_STATE_SUSPENDED:
        default:
            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : eDATA_PATH_STATE_SUSPENDED\r\n", __FUNCTION__, __LINE__);
            /* suspend the task indefinitely */
            m_sPalDataPathInfoSub.lPollPeriod = 0;
            break;

        case eDATA_PATH_STATE_TPG_DISPLAYED:
            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : eDATA_PATH_STATE_TPG_DISPLAYED\r\n", __FUNCTION__, __LINE__);
#if 0
            /* suspend the task indefinitely, alert GUI */
            datapathTaskPollPeriod = 0;
            guiSourceStatusMessage(GUI_SOURCE_TPG);
#endif
            break;

        case eDATA_PATH_STATE_BEGIN_SCAN:
        {
            /* manually flush mailbox to clean out old AutoLock status messages */
            /* without losing user messages                                     */
            palDataPath_FreezeChecking(eSOURCE_WINDOW_SUB);   //A70LV_Doulas_0223 Freeze check

            uPALIMGMGR_INFO uInfo = {.sConfig_NoSignalOutput.eWindow = eSOURCE_WINDOW_SUB,
                                     .sConfig_NoSignalOutput.ucDisplayOutput = 2}; //keep vop out
            palImgMgr_Config_NoSignalOutput(&uInfo);      //A70LV_Doulas_0142 //A70LV_Doulas_0120 show black sub
            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : eDATA_PATH_STATE_BEGIN_SCAN\r\n", __FUNCTION__, __LINE__);
            #if 0
            eResult = halScaler_Freeze_Get(0,&bFreezeEn);     //A70LV_Doulas_0101
            if((bFreezeEn == TRUE) && (eResult == eHAL_SCALER_EXEC_CODE_PASS))
            {
                if(palDataPath_Main_Or_Sub_SourceMonitor() == FALSE)
                {
                    GuiCb.fpGui_DataCode_Value_SetCb(edcIMAGE_FREEZE, 0);
                    appGui_CloseMenu();
                }
            }
            #endif

#if 1
            UINT8 ucInputKeyFunction = 0;
            palDataMgr_Data_Access(edcINPUT_KEY, edaREAD, &ucInputKeyFunction) ;

            palDataMgr_Data_Access(edcSHOW_SUBSOURCE_MESSAGES, edaREAD, &ucSourceState) ;   //A70LV_Doulas_0075 Add sub source status
            if(ucInputKeyFunction == eSOURCE_KEY_CHANGE_SOURCE_AUTO && ucSourceState != eSOURCE_STATE_SEARCHING)
            {
                ucSourceState = eSOURCE_STATE_SEARCHING;
                palDataMgr_Data_Access(edcSHOW_SUBSOURCE_MESSAGES, edaWRITE_RAM_ONLY_NO_ACTION, &ucSourceState);
            }
            else if(ucSourceState != eSOURCE_STATE_CHECKING_FOR_SIGNAL)
            {
                ucSourceState = eSOURCE_STATE_CHECKING_FOR_SIGNAL;
                palDataMgr_Data_Access(edcSHOW_SUBSOURCE_MESSAGES, edaWRITE_RAM_ONLY_NO_ACTION, &ucSourceState);
            }
#endif /* 0 */
            MS_SLEEP(20);
            halScaler_InputSelectSet(eSOURCE_WINDOW_SUB, (UINT8)m_sSourceDescSub.eConnector, FALSE);

            /* poll every 0.5 seconds if time has elasped */
            palDataPath_ShowLogoSub();              //A70LV_Doulas_0276 //A70LV_Doulas_0075 remove  //A70LV_Doulas_0032

            if(halScaler_RxPortIsReady(1, (UINT8)m_sSourceDescSub.eConnector) == eHAL_SCALER_EXEC_CODE_PASS)
            {
                m_sPalDataPathInfoSub.lPollPeriod = 100;//100ms     //A70LV_Doulas_0120 modify//A70LV_Doulas_0007
            }
            else
            {
                m_sPalDataPathInfoSub.lPollPeriod = 500;//100ms     //A70LV_Doulas_0120 modify//A70LV_Doulas_0007
            }

        }
#if 0


            guiSourceStatusMessage((GUI_SOURCEENUM) sourceDesc.connector);

            if(datapath_GetCycleCount() >= NTH_CYCLE_SHOW_NOSIGNAL)
            {
                guiSourceStatusMessage(GUI_SOURCE_NOSIGNAL);
            }
            else
            {
                guiSourceStatusMessage(GUI_SOURCE_SEARCHING);
            }

            //----- Roger Added 20061013  Start
            {
                uint08 mute = 0;
                coresystem_Get2230Mute(&mute);

                //If system is in the avmute then we can not let audio out
                if(!mute)
                {
                    audio_SetMute(TRUE);
                }
            }
            //----- Roger Added 20061013  End
            /* poll every 0.5 seconds if flush msg was lost */
            datapathTaskPollPeriod = 500;
#endif
            break;

        case eDATA_PATH_STATE_LOOK_FOR_SYNCS:

            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : eDATA_PATH_STATE_LOOK_FOR_SYNCS\r\n", __FUNCTION__, __LINE__);
            m_sPalDataPathInfoSub.lPollPeriod = 100;//100ms

#if 0

            /* connector scan started -- initialize device and APIs to detect syncs */

            //----- Roger Added 20061013  Start
            switch(sourceDesc.source)
            {
                case SRC_DVI_D:
                case SRC_HDMI:
                    if(datapath_IsSignalPresented(sourceDesc.source))
                    {
#ifdef __ENABLE_SIGNAL_PRETEST
                        corevariable_SetIsSignalPretestOkVal(TRUE);
#endif//__ENABLE_SIGNAL_PRETEST
                        dvicontrol_ConfigureForSearch(sourceDesc.source);
                    }

#ifdef __ENABLE_SIGNAL_PRETEST
                    else
                    {
                        corevariable_SetIsSignalPretestOkVal(FALSE);
                    }

#endif//__ENABLE_SIGNAL_PRETEST
                    break;

                case SRC_VGA1:
                case SRC_VGA2:
                case SRC_DVI_A:
                case SRC_RCA_ADC_COMPONENT:
                    if(datapath_IsSignalPresented(sourceDesc.source))
                    {
#ifdef __ENABLE_SIGNAL_PRETEST
                        corevariable_SetIsSignalPretestOkVal(TRUE);
#endif//__ENABLE_SIGNAL_PRETEST
                        adccontrol_ConfigureForSearch(sourceDesc.source);
                    }

#ifdef __ENABLE_SIGNAL_PRETEST
                    else
                    {
                        corevariable_SetIsSignalPretestOkVal(FALSE);
                    }

#endif//__ENABLE_SIGNAL_PRETEST
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
                    if(datapath_IsSignalPresented(sourceDesc.source))
                    {
#ifdef __ENABLE_SIGNAL_PRETEST
                        corevariable_SetIsSignalPretestOkVal(TRUE);
#endif//__ENABLE_SIGNAL_PRETEST
                        deccontrol_ConfigureForSearch(sourceDesc.source);
                    }

#ifdef __ENABLE_SIGNAL_PRETEST
                    else
                    {
                        corevariable_SetIsSignalPretestOkVal(FALSE);
                    }

#endif//__ENABLE_SIGNAL_PRETEST
                    break;
            }

            //----- Roger Added 20061013  End
            /* poll every 100 milliseconds if syncs are detected */
            datapathTaskPollPeriod = 100;
#endif

            break;

        case eDATA_PATH_STATE_ATTEMPT_LOCK:
            ucSourceState = eSOURCE_STATE_SETTING_UP_IMAGE; //A70LV_Larry_0350
            palDataMgr_Data_Access(edcSHOW_SUBSOURCE_MESSAGES, edaWRITE_RAM_ONLY_NO_ACTION, &ucSourceState) ;      //A70LV_Doulas_0075 Add sub source status
            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : eDATA_PATH_STATE_ATTEMPT_LOCK\r\n", __FUNCTION__, __LINE__);
            /* syncs found -- wait to lock to source */
            /* poll every 100 milliseconds if the source is detected */

            palLANProcSendToLAN(edcSUB_INPUT);
            palLANProcSendToLAN(edcSUB_ASPECT_RATIO);
            palLANProcSendToLAN(edcSUB_SIGNAL_FORMAT);
            palLANProcSendToLAN(edcSUB_RESOLUTION);
            palLANProcSendToLAN(edcSUB_PIXEL_CLOCK);
            palLANProcSendToLAN(edcSUB_SYNC_TYPE);
            palLANProcSendToLAN(edcSUB_HORZ_REFRESH);
            palLANProcSendToLAN(edcSUB_VERT_REFRESH);
            palLANProcSendToLAN(edcSUB_COLOR_SPACE);

            if(!palDataMgr_IsOsdOpen()) //A70LK_Jacky_0013
            {
                //appGui_SendUpdateOSDEvent();
			    palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL);
            }
            palDataMgr_UI_EventSend(edcUI_EVENT_DRAW_PIP_LAYOUT, TRUE, NULL); //appGui_SendDrawPIPLayoutEvent(1);
            palDataPath_FreezeChecking(eSOURCE_WINDOW_SUB);
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

            palEnvironment_LightSourceBlankingOn_Set(TRUE,5);   //A70LV_Doulas_0294
            palDataMgr_Data_Access(edcSHOW_SUBSOURCE_MESSAGES, edaWRITE_RAM_ONLY_NO_ACTION, &ucSourceState) ;      //A70LV_Doulas_0075 Add sub source status
            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : eDATA_PATH_STATE_MONITOR_SOURCE\r\n", __FUNCTION__, __LINE__);
            m_sPalDataPathInfoSub.ucSyncLockCount = 0;      //A70LV_Doulas_0009 Add

            halScaler_ConfigureForDisplay(1, 1);       //A70LV_Doulas_0007
            halScaler_ConfigureColor(1);
            //palDataPath_DisplaySFGSub(1, 5, FALSE);    //A70LV_Doulas_0007
            m_sPalDataPathInfoSub.lPollPeriod = 500;//1000ms    //A70LV_Doulas_0164 modify //A70LV_Doulas_0009
            palDataPath_TurnOffSplashSub();     //A70LV_Doulas_0032

            if((halScaler_PIP_PBP_Enable_Get() != eCM_SCREEN_MODE_OFF) && (palDataPath_IsSourceLock() == FALSE))    //A70LV_Doulas_0182 modify, Sub CH DDP not setting on the C821 3D 1080P120hz output
            {
                palDataMgr_Format_Normal_WithSemaphoreSet();     //A70LV_Doulas_0046
            }

            //palDataPath_MenuTransparencyEnableSet(TRUE); //appGui_OSD_MenuTransparencyEnableSet(TRUE);     //A70LV_Doulas_0122 enable Menu Transparency
            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : eDATA_PATH_STATE_MONITOR_SOURCE  End\r\n", __FUNCTION__, __LINE__);      //A70LV_Doulas_0216
            palDataMgr_UI_EventSend(edcUI_EVENT_SUB_SOURCE_INFOR_MSG, TRUE, NULL); //appGui_SendSubSourceInfoMessage();
            //appGui_SendUpdateOSDEvent();        //A70LV_Doulas_0208 Add
			palDataMgr_OPDSourceInfo(1);
#if 0

            /* source locked -- configure datapath for source */

            //----- Roger Added 20061214  Start
            if(datapath_ConfigureForExternalSource() != PASS)
            {
                /* if FillSourceConfig failed, redisplay SFG and */
                /*   output OOR msg to GUI                       */
                sourceDesc.sourceActive = FALSE;
                source_DisplaySFG(&sourceDesc);
                dispfmt_ConfigureForSource(FALSE);
                pictcont_ConfigureForInternalSource();

                if(sourceDesc.isOor == TRUE)
                {
                    guiSourceStatusMessage(GUI_SOURCE_OOR);
                    guiSourceStatusMessage((GUI_SOURCEENUM) sourceDesc.connector);
                }
            }
            else
            {
                guiSourceStatusMessage(GUI_SOURCE_FOUND);
                corevariable_SetIsSourceActiveVal(TRUE);
            }

            //----- Roger Added 20061214  End

            /* poll every 0.5 seconds to monitor source */
            datapathTaskPollPeriod = 500;
#endif
            break;

        case eDATA_PATH_STATE_SOURCE_LOST:
            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : eDATA_PATH_STATE_SOURCE_LOST\r\n", __FUNCTION__, __LINE__);
            halScaler_MeasureInput(eSOURCE_WINDOW_SUB, (UINT8)m_sSourceDescSub.eConnector);

            palLANProcSendToLAN(edcSUB_INPUT);
            palLANProcSendToLAN(edcSUB_ASPECT_RATIO);
            palLANProcSendToLAN(edcSUB_SIGNAL_FORMAT);
            palLANProcSendToLAN(edcSUB_RESOLUTION);
            palLANProcSendToLAN(edcSUB_PIXEL_CLOCK);
            palLANProcSendToLAN(edcSUB_SYNC_TYPE);
            palLANProcSendToLAN(edcSUB_HORZ_REFRESH);
            palLANProcSendToLAN(edcSUB_VERT_REFRESH);
            palLANProcSendToLAN(edcSUB_COLOR_SPACE);

            palDataMgr_UI_EventSend(edcUI_EVENT_DRAW_PIP_LAYOUT, TRUE, NULL); //appGui_SendDrawPIPLayoutEvent(1);
            palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL);
            break;
    }

    palDataPath_ResetTimeInStateSub();
    m_sPalDataPathInfoSub.eDataPathState = (eDATA_PATH_STATE)ulNewState;


    if(ucLastSourceState != m_sPalDataPathInfoSub.eDataPathState)
    {
        if((ulNewState >= eDATA_PATH_STATE_SCALER_PORT_CONFIG) &&
           (ulNewState <= eDATA_PATH_STATE_ATTEMPT_LOCK))
        {
            m_sPalDataPathInfoSub.ucSourceConnectChange = 0;
        }
        palLANProcSendToLAN((UINT16)edcIS_SUB_SOURCE_LOCK);
    }

    ucLastSourceState = m_sPalDataPathInfoSub.eDataPathState;
}

static void palDataPath_StartDisplaySub(void)
{
//    UINT8 ucIsLogoOn = 0; //A70LV_Doulas_0025
#ifndef __ICHIP_CONTROL__
    UINT8 ucData = 0;
#endif /*__ICHIP_CONTROL__*/

#if 0
    adccontrol_powerNormal();
    dvicontrol_powerNormal();
    deccontrol_powerNormal();
#endif
    MS_SLEEP(6000);

    ucData = eSOURCE_STATE_CHECKING_FOR_SIGNAL;
    palDataMgr_Data_Access(edcSHOW_SUBSOURCE_MESSAGES, edaWRITE_RAM_ONLY_NO_ACTION, &ucData);

    palDataPath_FreezeChecking(eSOURCE_WINDOW_SUB);  //G100_Simon_0044
    palDataPath_InitInputSourceSub();  //A70LV_Doulas_0007
    halScaler_SetInputSource(eSOURCE_WINDOW_SUB,m_sSourceDescSub.eConnector);   //A70LV_Doulas_0007
    halScaler_PowerNormal(eSOURCE_WINDOW_SUB, m_sPalDataPathInfoSub.ePanelTimingId);    //init CH2
    palFrontEndMgr_SubInput_Set((UINT8*)&m_sSourceDescSub.eConnector, TRUE, TRUE);

//#if(CURRENT_PLATFORM == PLATFORM_4K_C790 || CURRENT_PLATFORM == PLATFORM_2K_C787)
    //halWarping_PowerNormal(m_sPalDataPathInfoSub.ePanelTimingId);
//#endif //A70LV_Larry_0051 mask

//    palDataMgr_Data_Access(edcSPLASH_STARTUP, edaREAD, (PUINT8)&ucIsLogoOn);  //A70LV_Doulas_0032 remove //A70LV_Doulas_0025

    //Add C734 OSD hardware init function after halScaler_PowerNormal(in PC platform demo board will connect after this function), OSD should init complete before Splash display
    //appGui_HWInit();

	//if(halScaler_PIP_PBP_Enable_Get() != eCM_SCREEN_MODE_OFF)	//A70LK_Larry_0132 mask //A70LK_Sammy_0003
    	//halScaler_ConfigureForScaler(eSOURCE_WINDOW_SUB);


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
    palDataPath_GotoStateSub(eDATA_PATH_STATE_SPLASH_AT_STARTUP);      //A70LV_Doulas_0043
    #endif

#ifndef __ICHIP_CONTROL__
    palDataMgr_Data_Access(edcSUB_INPUT, edaREAD, &ucData); //A70LV_Larry_0139
	m_sSourceDescSub.eConnector = ucData;
	halScaler_InputSelectSet(1, ucData, TRUE);
#endif /*__ICHIP_CONTROL__*/


}


static void palDataPath_StateMachineProcSub(UINT16 uiMsgID)
{
    //BOOL bSyncLock; //A70LV_Doulas_0007
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_FAIL; //A70LV_Doulas_0009 Add
#ifdef __ICHIP_CONTROL__
    static UINT8 ucCheckTimer = 0;  //A70LV_Doulas_0240
#endif /* __ICHIP_CONTROL__ */

    if((palSystem_PowerStateGet() == ePOWER_STATE_STANDBY) || (palSystem_PowerStateGet() == ePOWER_STATE_UPGRADE)) //A70LV_Larry_0112
    {
        return;
    }

    if(palDataMgr_CurTestPatternGet() != eTID_OFF) //test pattern on
        return;

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

                if(halScaler_RxPortIsReady(1, (UINT8)m_sSourceDescSub.eConnector) == eHAL_SCALER_EXEC_CODE_PASS)
                {
                    if(halScaler_InputCompare(1, (UINT8)m_sSourceDescSub.eConnector))
                    {
                        palDataPath_GotoStateSub(eDATA_PATH_STATE_LOOK_FOR_SYNCS);
                    }
                    else
                    {
                        if(halScaler_PIP_PBP_Enable_Get())  //A70LK_Larry_0132 //G100_Owen_0033
                        {
                            halScaler_Resync_Init(eSOURCE_WINDOW_SUB);
                        }
                        palDataPath_GotoStateSub(eDATA_PATH_STATE_BEGIN_SCAN);
                    }

                }
                else
                {
                    if(halScaler_PIP_PBP_Enable_Get())  //A70LK_Larry_0132 //G100_Owen_0033
                    {
                        halScaler_Resync_Init(eSOURCE_WINDOW_SUB);
                    }
                    palDataPath_GotoStateSub(eDATA_PATH_STATE_BEGIN_SCAN);     //A70LV_Doulas_0004
                }
                break;

                case eDATA_PATH_MSG_PANEL_CHANGE:   //A70LV_Doulas_0154
                    LOG_MSG(db_APP_DATAPATH, " eDATA_PATH_MSG_PANEL_CHANGE Sub\r\n");
                    palDataPath_GotoStateSub(eDATA_PATH_STATE_BEGIN_SCAN);
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
                    palDataPath_GotoStateSub(eDATA_PATH_STATE_BEGIN_SCAN);
                    break;

                case eDATA_PATH_MSG_NONE:   //A70LV_Doulas_0007
                    {
                        //A70LV_Doulas_0076 modify
#ifdef __ICHIP_CONTROL__
                        UINT8 ucVal;
                        eRESULT eResult2 = rcINVALID;
#endif /* __ICHIP_CONTROL__ */

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

                        if((halScaler_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_OFF) &&
                           (halScaler_PIP_PBP_Enable_Get() == ets_OFF))                     //A70LV_Doulas_0240 check sub
                        {
                            break;
                        }

#ifdef __ICHIP_CONTROL__
                        eResult2 = halFrontEndCtrl_videoReady_Get(&ucVal);
                        if(eResult2 != rcSUCCESS)        //A70LV_Doulas_0235
                        {
                            LOG_MSG(db_APP_DATAPATH, " FrontEnd2 read Fail\r\n");
                        }
                        if((((halScaler_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_FRAMEPACKING) ||
                             (halScaler_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_SIDEBYSIDE) ||
                             (halScaler_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_TOPANDBOTTOM)) &&
                            (eResult2 == rcSUCCESS) && (ucVal & 0x01) ) ||
                            (((halScaler_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_OFF) ||
                             (halScaler_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_FRAME_SEQUENTIAL)) &&
                            (eResult2 == rcSUCCESS) && (ucVal & 0x02) )
                            )       //A70LV_Doulas_0159 modify //A70LV_Doulas_0154 modify
                        {
                            UINT8 ucVideoFormat[10]={0};
                            if((halScaler_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_FRAMEPACKING) ||
                               (halScaler_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_SIDEBYSIDE) ||
                               (halScaler_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_TOPANDBOTTOM))     //A70LV_Doulas_0159 modify //A70LV_Doulas_0154
                                eResult2 = halFrontEndCtrl_videoFormat_Get(ucVideoFormat);
                            else
                                eResult2 = halFrontEndCtrl_videoFormatSub_Get(ucVideoFormat);    //A70LV_Doulas_0112

                            if(eResult2 == rcSUCCESS)
                            {
                                if(halScaler_PIP_PBP_Enable_Get() == ets_OFF)    //A70LV_Doulas_0255 modify main and sub different setting
                                {
                                    halScaler_IintChannelSetting(eSOURCE_WINDOW_SUB);
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
                                LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d)  eDATA_PATH_STATE_BEGIN_SCAN (%d,%d,%d)(%d,%d,%d)(%d,%d)(%d,%d)(%d)\r\n", __FUNCTION__, __LINE__,ucVideoFormat[0],ucVideoFormat[1],ucVideoFormat[2],ucVideoFormat[3],ucVideoFormat[4],ucVideoFormat[5]
                                                        ,ucVideoFormat[6],ucVideoFormat[7],ucVideoFormat[8],ucVideoFormat[9],ucVal);    //A70LV_Doulas_0154 modify//A70LV_Doulas_0109
                                palDataPath_FrontEndVideoInfoSub_Set();
                                palDataPath_GotoStateSub(eDATA_PATH_STATE_LOOK_FOR_SYNCS);
                            }
                        }
#else
						//eResult = halScaler_Monitor(1);
                        //eResult = eHAL_SCALER_EXEC_CODE_PASS; //ProAV_Larry_0025 remove
                        eResult = halScaler_RxPortIsReady(1, (UINT8)m_sSourceDescSub.eConnector);

                        if((eResult == eHAL_SCALER_EXEC_CODE_PASS) && (halScaler_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_OFF))
                        {
                            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d)  eDATA_PATH_STATE_BEGIN_SCAN Sub 2 eDATA_PATH_STATE_LOOK_FOR_SYNCS\r\n", __FUNCTION__, __LINE__);

                            if((halScaler_ColorSpace_Get(eSOURCE_WINDOW_SUB) == eCM_COLOR_SPACE_RGB_FULL) || (halScaler_ColorSpace_Get(eSOURCE_WINDOW_SUB) == eCM_COLOR_SPACE_RGB_LIMITED)) //A70LK_Nina_0047
                            {
                                if(halScaler_PixModIsYUV420(eSOURCE_WINDOW_SUB, (UINT8)m_sSourceDescSub.eConnector))
                                {
                                    UINT8 ucValue = eCM_COLOR_SPACE_YUV_REC709;
                                    palDataMgr_Data_Access(edcSUB_IMAGE_COLOR_SPACE, edaWRITE_THROUGH_WITH_ACTION, &ucValue);
                                }
                            }

                            palDataPath_GotoStateSub(eDATA_PATH_STATE_LOOK_FOR_SYNCS);
                        }
                        else
                        {
                            m_sPalDataPathInfoSub.lPollPeriod = 500;
                        }
#endif /* __ICHIP_CONTROL__ */
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
                    palDataPath_GotoStateSub(eDATA_PATH_STATE_BEGIN_SCAN);
                    break;

                case eDATA_PATH_MSG_NONE:   //A70LV_Doulas_0007
#ifdef __ICHIP_CONTROL__
               //     LOG_MSG(db_APP_DATAPATH, "\r\n");
                    if(halScaler_MeasureInput(1) == eHAL_SCALER_EXEC_CODE_PASS)     //A70LV_Doulas_0031 modify
                    {
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
                    else
                    {
                        ucCheckTimer++;  //A70LV_Doulas_0240
                        if(ucCheckTimer > 3)    //A70LV_Doulas_0240 modify
                        {
                            ucCheckTimer = 0;
                            palDataPath_GotoStateSub(eDATA_PATH_STATE_BEGIN_SCAN);  //A70LV_Doulas_0009 remove
                        }
                    }
#else
					if((halScaler_PIP_PBP_Enable_Get() != eCM_SCREEN_MODE_OFF) && (halScaler_MeasureInput(1, (UINT8)m_sSourceDescSub.eConnector) == eHAL_SCALER_EXEC_CODE_PASS))
                    {
                        halScaler_AVIInfoFrame(1);
                        halScaler_InputFrameRateSetting(1); // ProAV_Rex_0041
                        palDataPath_GotoStateSub(eDATA_PATH_STATE_ATTEMPT_LOCK);
                    }
                    else
                    {
                        palDataPath_GotoStateSub(eDATA_PATH_STATE_BEGIN_SCAN);
                    }
#endif /* __ICHIP_CONTROL__ */
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



                    palDataPath_GotoStateSub(eDATA_PATH_STATE_MONITOR_SOURCE);

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
                    palDataPath_GotoStateSub(eDATA_PATH_STATE_BEGIN_SCAN);
                    break;

                case eDATA_PATH_MSG_NONE:   //A70LV_Doulas_0007
#ifdef __ICHIP_CONTROL__
                    LOG_MSG(db_APP_DATAPATH, "\r\n");
                    if(halScaler_SYNC_Lock_Get(1) == eHAL_SCALER_EXEC_CODE_PASS)     //A70LV_Doulas_0009
                    {
                        if(halScaler_AutoPhaseStart(1) == eHAL_SCALER_EXEC_CODE_PASS)
                        {
                            palDataPath_GotoStateSub(eDATA_PATH_STATE_AUTO_PHASE);
                        }
                        else
                        {
                            palDataPath_GotoStateSub(eDATA_PATH_STATE_MONITOR_SOURCE);
                        }
                    }
                    else
                    {
                        palDataPath_GotoStateSub(eDATA_PATH_STATE_BEGIN_SCAN);
                    }
#else
					if(halScaler_SYNC_Lock_Get(eSOURCE_WINDOW_SUB) == eHAL_SCALER_EXEC_CODE_PASS) //A70LV_Doulas_0009 modify
                    {
                        if((halScaler_SourceLock(eSOURCE_WINDOW_SUB) == eHAL_SCALER_EXEC_CODE_PASS) &&	//A70LK_Sammy_0003
                           (halScaler_MeasureCheck(eSOURCE_WINDOW_SUB) == eHAL_SCALER_EXEC_CODE_PASS))
                        {
                            palDataPath_GotoStateSub(eDATA_PATH_STATE_MONITOR_SOURCE);
                        }
                    }
                    else
                    {
                        palDataPath_GotoStateSub(eDATA_PATH_STATE_BEGIN_SCAN);
                    }

#endif /* __ICHIP_CONTROL__ */
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
                    palDataPath_GotoStateSub(eDATA_PATH_STATE_BEGIN_SCAN);
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
                    palDataPath_GotoStateSub(eDATA_PATH_STATE_BEGIN_SCAN);
                    break;

                case eDATA_PATH_MSG_NONE:
                    LOG_MSG(db_APP_DATAPATH, "\r\n");
                    if(halScaler_MeasureInput(1, (UINT8)m_sSourceDescSub.eConnector) == eHAL_SCALER_EXEC_CODE_PASS)     //A70LV_Doulas_0031 modify
                    {
                        palDataPath_GotoStateSub(eDATA_PATH_STATE_MONITOR_SOURCE);
                    }
                    else
                    {
                        palDataPath_GotoStateSub(eDATA_PATH_STATE_BEGIN_SCAN);
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
                    if(m_sPalDataPathInfoSub.ucSyncLockCount > 5)      //A70LV_Doulas_0164 Modify
                    {
                        BOOL bFreezeEn = FALSE;
                        m_sPalDataPathInfoSub.ucSyncLockCount = 0;
                        eResult = halScaler_MeasureCheck(1);        //A70LV_Doulas_0031 modify

                        halScaler_Freeze_Get(eSOURCE_WINDOW_SUB,&bFreezeEn);     //A70LV_Doulas_0223 Add
                        if(bFreezeEn == TRUE)
                        {
                            eResult = eHAL_SCALER_EXEC_CODE_PASS;   //keep monitor
                        }
                    }
                    else
                    {
                        //eResult = halScaler_SYNC_Lock_Get(1);

                        eResult =halScaler_RxPortIsReady(1, (UINT8)m_sSourceDescSub.eConnector);
                        if(eResult == eHAL_SCALER_EXEC_CODE_PASS)   //A70LV_Doulas_0283
                        {
#ifdef __ICHIP_CONTROL__
                            if(palDataPath_AutoCheckColorSpaceChangeSub() == TRUE)
                            {
                                eResult = eHAL_SCALER_EXEC_CODE_FAIL;
                            }
#endif /* __ICHIP_CONTROL__ */
                        }
                    }

                    if(eResult == eHAL_SCALER_EXEC_CODE_PASS)
                    {
                    }
                    else if (palDataPath_IsTestPatternEnable() == FALSE)
                    {
                        //BOOL bFreezeEn = FALSE;
                        //halScaler_Freeze_Get(1,&bFreezeEn);
                        //if(bFreezeEn == FALSE)
                        palEnvironment_LightSourceBlankingOn_Set(TRUE,5);   //A70LV_Doulas_0294
                        palDataPath_FreezeChecking(eSOURCE_WINDOW_SUB);   //A70LV_Doulas_0223 Freeze check
                        {
                            LOG_MSG(db_APP_DATAPATH, " eDATA_PATH_MSG_LOSTLOCK24\r\n");
                            halScaler_DisplaySFG(1, 5, TRUE);

                            palDataPath_GotoStateSub(eDATA_PATH_STATE_SOURCE_LOST);
                        }
                    }
                    break;

                case eDATA_PATH_MSG_LOSTLOCK:
                    LOG_MSG(db_APP_DATAPATH, " eDATA_PATH_MSG_LOSTLOCK25\r\n");
                    palEnvironment_LightSourceBlankingOn_Set(TRUE,5);   //A70LV_Doulas_0294
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
                    palDataPath_GotoStateSub(eDATA_PATH_STATE_BEGIN_SCAN);
                    break;

                case eDATA_PATH_MSG_RECONFIG_IMAGE:     //A70LV_Doulas_0009
                    LOG_MSG(db_APP_DATAPATH, " eDATA_PATH_MSG_RECONFIG_IMAGE Sub\r\n");
                    halScaler_DisplaySFG(1, 5, TRUE);
                    palDataPath_GotoStateSub(eDATA_PATH_STATE_ATTEMPT_LOCK);
                    break;

            }

            {   //A70LV_Doulas_0101 Add
                UINT8 ucValue = 0;
                palDataMgr_Data_Access(edcIMAGE_FREEZE, edaREAD, &ucValue);     //A70LV_Doulas_0167
                if(ucValue)     //A70LV_Doulas_0167 //A70LK_Jacky_0040
                {
                    palDataMgr_UI_EventSend(edcUI_EVENT_OPEN_FREEZE_MENU_MSG, TRUE, NULL);
                }
            }
            break;

        case eDATA_PATH_STATE_SOURCE_LOST:
            palDataPath_GotoStateSub(eDATA_PATH_STATE_BEGIN_SCAN);
        	break;

        default:
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
            m_sSourceDescSub.eConnector = (eCM_SOURCE_ID)ulParam1;         //A70LV_Doulas_0029 modify
            halScaler_SetInputSource(eSOURCE_WINDOW_SUB, m_sSourceDescSub.eConnector);
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
            break;

        case eDATA_PATH_MSG_RECONFIG_IMAGE:     //A70LV_Doulas_0009
            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : eDATA_PATH_MSG_RECONFIG_IMAGE\r\n", __FUNCTION__, __LINE__);
            palDataPath_StateMachineProcSub(uiMsgID);
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

    uPALIMGMGR_INFO uInfo = {.sPowerStandby.ucCH = 1};
    palImgMgr_PowerStandby(&uInfo);

    return eEXEC_CODE_PASS;
}



eEXEC_CODE palDataPath_PowerNormalSub(ePANEL_ID ePanelId)
{
    UINT32 temp;
    LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);

    m_sPalDataPathInfoSub.bDataPathReady = FALSE;

    m_sPalDataPathInfoSub.ePanelTimingId  = ePanelId;

    palDataPathSub_MailBox_Send(m_sPalDataPathInfoSub.xMsgQueue,
                    m_sPalDataPathInfoSub.xEventGroupHandle,
                    eDATA_PATH_MSG_INIT, -1, 0, &temp, FALSE);

    return eEXEC_CODE_PASS;
}

void palDataPath_TaskSub_Suspend(void)
{
#if (SYSTEM_OS_TYPE == POSIX_COMPLIANT)
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

#endif
}

void palDataPath_TaskSub_Resume(void)
{
#if (SYSTEM_OS_TYPE == POSIX_COMPLIANT)
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
    pthread_attr_setdetachstate( &m_sPalDataPathInfoSub.xTaskHandle_attr, PTHREAD_CREATE_DETACHED );
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

#else  //(CURRENT_RTOS_TYPE == RTOS_DYNAMIC)

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
    palDataPathSub_MailBox_Send(m_sPalDataPathInfoSub.xMsgQueue,
                    m_sPalDataPathInfoSub.xEventGroupHandle,
                    eDATA_PATH_MSG_PANEL_CHANGE, -1, 0, &temp, FALSE);
    return eEXEC_CODE_PASS;
}

eEXEC_CODE palDataPath_InitInputSourceSub(void)   //A70LV_Doulas_0029 modify //A70LV_Doulas_0007
{
    INT32 iVAl;
    palDataMgr_Data_Access(edcSUB_INPUT, edaREAD, &iVAl);
    m_sSourceDescSub.eConnector = (eCM_SOURCE_ID)iVAl;

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

    if(palDataPath_IsSourceMonitor() == FALSE)    //A70LV_Doulas_0310
    {
        palDataPath_AutoDisableMenuTransparency();   //A70LV_Doulas_0122 auto dsiable Menu Transparency
    }

    ucValue = halScaler_PIP_PBP_Enable_Get();   //A70LV_Doulas_0120 modify
    if(ucValue != eCM_SCREEN_MODE_OFF)
    {
        palDataMgr_UI_EventSend(edcUI_EVENT_SUB_SOURCE_INFOR_MSG, TRUE, NULL); //GuiCb.fpGui_SendSubSourceInfoMessageCb();
    }

    return TRUE;
}

eEXEC_CODE palDataPath_InputPixelClockSub_Get(UINT8 *ucValue)   //A70LV_Doulas_0056
{
    eEXEC_CODE eExecResult = eEXEC_CODE_PASS;

    if(palDataPath_IsSourceLockSub())
    {
        if(m_sPalDataPathInfoSub.ePanelTimingId == PANEL_3D_OUTPUT)   //A70LV_Doulas_0287
        {
            *ucValue = '-';
            *(ucValue+1) = '\0';
        }
        else if(m_FrontEndVideoFormatSub.u8VideoDownScaling)        //A70LV_Doulas_0187 modify
        {
            char aucString[VERSION_STRING_MAX_LENGTH];
            DOUBLE dbVal;
            dbVal = (DOUBLE)m_FrontEndVideoTimingSub.u32VideoPCLK;
            sprintf(aucString, "%d.%03dMHz\0",(UINT32)dbVal/1000,(UINT32)dbVal%1000);
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

    if(palDataPath_IsSourceLockSub())
    {
        if(m_sPalDataPathInfoSub.ePanelTimingId == PANEL_3D_OUTPUT)   //A70LV_Doulas_0287
        {
            *ucValue = '-';
            *(ucValue+1) = '\0';
        }
        else if(m_FrontEndVideoFormatSub.u8VideoDownScaling)        //A70LV_Doulas_0187 modify
        {
            char aucString[VERSION_STRING_MAX_LENGTH];
            //if(m_sSourceDescSub.eConnector == eINPUT_SOURCE_VGA)
                //sprintf(aucString, "Analog\0");
            //else
            sprintf(aucString, "Digital\0");
            memcpy(ucValue, aucString, strlen(aucString)+1);
        }
        else
        {
            halScaler_InputSignalFormat_Get(eSOURCE_WINDOW_SUB,ucValue);
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

    if(palDataPath_IsSourceLockSub())
    {
        if(m_sPalDataPathInfoSub.ePanelTimingId == PANEL_3D_OUTPUT)   //A70LV_Doulas_0287
        {
            *ucValue = '-';
            *(ucValue+1) = '\0';
        }
        else if(m_FrontEndVideoFormatSub.u8VideoDownScaling)        //A70LV_Doulas_0187 modify
        {
            char aucString[VERSION_STRING_MAX_LENGTH];
            sprintf(aucString, "%d x %d\0",m_FrontEndVideoTimingSub.u16VideoHActive ,m_FrontEndVideoTimingSub.u16VideoVActive);
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

    if(palDataPath_IsSourceLockSub())
    {
        if(m_sPalDataPathInfoSub.ePanelTimingId == PANEL_3D_OUTPUT)   //A70LV_Doulas_0287
        {
            *ucValue = '-';
            *(ucValue+1) = '\0';
        }
        else if(m_FrontEndVideoFormatSub.u8VideoDownScaling)        //A70LV_Doulas_0187 modify
        {
            char aucString[VERSION_STRING_MAX_LENGTH];
            DOUBLE dbVal;
            dbVal = (DOUBLE)m_FrontEndVideoTimingSub.u32VideoPCLK *1000 / (DOUBLE)m_FrontEndVideoTimingSub.u16VideoHTotal;
            sprintf(aucString, "%d.%03dkHz\0",(UINT32)dbVal/1000,(UINT32)dbVal%1000);
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

    if(palDataPath_IsSourceLockSub())
    {
        if(m_sPalDataPathInfoSub.ePanelTimingId == PANEL_3D_OUTPUT)   //A70LV_Doulas_0287
        {
            *ucValue = '-';
            *(ucValue+1) = '\0';
        }
        else if(m_FrontEndVideoFormatSub.u8VideoDownScaling)        //A70LV_Doulas_0187 modify
        {
            char aucString[VERSION_STRING_MAX_LENGTH];
            DOUBLE dbVal;
            dbVal = (DOUBLE)m_FrontEndVideoTimingSub.u32VideoPCLK *1000 / (DOUBLE)m_FrontEndVideoTimingSub.u16VideoHTotal / (DOUBLE)m_FrontEndVideoTimingSub.u16VideoVTotal * 100;
            sprintf(aucString, "%d.%02dHz\0",(UINT32)dbVal/100,(UINT32)dbVal%100);
            memcpy(ucValue, aucString, strlen(aucString)+1);
        }
        else
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

    if(palDataPath_IsSourceLockSub())
    {
        if(m_sPalDataPathInfoSub.ePanelTimingId == PANEL_3D_OUTPUT)   //A70LV_Doulas_0287
        {
            *ucValue = '-';
            *(ucValue+1) = '\0';
        }
#if 0
        else if(m_sSourceDescSub.eConnector == eINPUT_SOURCE_VGA)
        {
            //check sync "Sync on Green" or "Separate"
            if(m_FrontEndVideoFormatSub.u8VideoVGASyncType == eVGA_SYNC_TYPE_SOG)  //A70LV_Doulas_0109 modify
            {
                sprintf(aucString, "Sync on Green\0");
                memcpy(ucValue, aucString, strlen(aucString)+1);
            }
            else
            {
                sprintf(aucString, "Separate\0");
                memcpy(ucValue, aucString, strlen(aucString)+1);
            }
        }
#endif /* 0 */
        else
        {
            sprintf(aucString, "Separate\0");
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

    if(palDataPath_IsSourceLockSub())
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

    if(m_sPalDataPathInfoSub.ePanelTimingId == PANEL_3D_OUTPUT)   //A70LV_Doulas_0287
    {
        *ucValue = '-';
        *(ucValue+1) = '\0';
    }
    else if(palDataPath_IsSourceLockSub())
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
    //UINT8 ucColorSpace;
    //UINT8 ucVideoYUV;

    //Color space
#if 0
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
#endif /* 0 */
#if 0
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
                    ucColorSpace = eCOLOR_FORMAT_422;
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

    if(m_FrontEndVideoFormatSub.u8VideoDownScaling)    //A70LV_Doulas_0176 Add
    {
        UINT8 ucVal[15];
        halFrontEndCtrl_Sub_Timing_Get(ucVal);
        m_FrontEndVideoTimingSub.u32VideoPCLK = ucVal[0]+ (ucVal[1]<<8) + (ucVal[2]<<16) +(ucVal[3]<<24);
        m_FrontEndVideoTimingSub.u16VideoHTotal = ucVal[4]+ (ucVal[5]<<8);
        m_FrontEndVideoTimingSub.u16VideoHActive = ucVal[6]+ (ucVal[7]<<8);
        m_FrontEndVideoTimingSub.u16VideoVTotal = ucVal[8]+ (ucVal[9]<<8);
        m_FrontEndVideoTimingSub.u16VideoVActive = ucVal[10]+ (ucVal[11]<<8);
        m_FrontEndVideoTimingSub.u16VideoVRate =  ucVal[12]+ (ucVal[13]<<8);
        LOG_MSG(db_APP_DATAPATH, "PCLK    =0x%x \r\n",m_FrontEndVideoTimingSub.u32VideoPCLK);
        LOG_MSG(db_APP_DATAPATH, "H-total =%d \r\n",m_FrontEndVideoTimingSub.u16VideoHTotal);
        LOG_MSG(db_APP_DATAPATH, "V-total =%d \r\n",m_FrontEndVideoTimingSub.u16VideoVTotal);
        LOG_MSG(db_APP_DATAPATH, "H-Activ =%d \r\n",m_FrontEndVideoTimingSub.u16VideoHActive);
        LOG_MSG(db_APP_DATAPATH, "V-Activ =%d \r\n",m_FrontEndVideoTimingSub.u16VideoVActive);
        LOG_MSG(db_APP_DATAPATH, "V-Freq  =%d \r\n",m_FrontEndVideoTimingSub.u16VideoVRate);
    }
#endif /* 0 */
    return eExecResult;
}

eDATA_PATH_STATE palDataPath_GetDataPathSubState(void)      //A70LV_Doulas_0236
{
    return m_sPalDataPathInfoSub.eDataPathState;
}

BOOL palDataPath_AutoCheckColorSpaceChangeSub(void)        //A70LV_Doulas_0283
{
    eRESULT eResult = rcINVALID;
    UINT8 ucVideoFormat[10] = {0};

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
#if 0
        if((m_sSourceDescSub.eConnector == eINPUT_SOURCE_VGA) ||
           (m_sSourceDescSub.eConnector == eINPUT_SOURCE_3G_SDI))
        {
            return FALSE;
        }
#endif /* 0 */

        //if( halScaler_ColorSpace_Get(eSOURCE_WINDOW_SUB) !=  eCM_COLOR_SPACE_AUTO)
        //{
        //    return FALSE;
        //}

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
    return FALSE;
}


UINT8 palDataPath_HDR_Info_Sub_Get(void)    //A65_OPTOMA_Doulas_0149
{
    return m_FrontEndVideoFormatSub.u8VideoHDRType;
}


