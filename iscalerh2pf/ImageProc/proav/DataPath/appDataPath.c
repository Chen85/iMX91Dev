#include "appDataPath.h"
#include "appDataMgr.h"
#include "appSystem.h"
#include "appEnvironment.h"
#include "appIllumination.h"
#include "appLANProcAPI.h"

#include "utilDbgMsg.h"
#include "utilDataMgrAPI.h"
#include "halFormatter.h"
#include "halFrontEndCtrlAPI.h"
#include "halBoardCtrlAPI.h"
#include "halMCUCtrlAPI.h"
#include "halFormatter.h"
#include "halScaler.h"
#include "halMotorCtrlAPI.h"

#include "GEC_EventTable.h"

#include "palGui.h"
#include "palGeoAPI.h"
#include "palImgMgr.h"
#include "palMotorMgr.h"
#include "palFormatterMgr.h"
#include "palFrontEndMgr.h"
#include "ProcessMutexData.h"

#define  PANEL_CHECK_COUNT (10)

sGUI_CALLBACK GuiCb;

static sPAL_DATA_PATH_INFORMATION   m_sPalDataPathInfo;
static PsSYSTEM_CONFIGURATION       m_psSysConfiguration;        //Default system settings, like TI gpConfiguration
static sDATA_PATH_SOURCE_DESC       m_sSourceDesc;
//static sVIDEO_FORMAT                m_FrontEndVideoFormat = {eVIDEO_COLORDEPTH_8BIT,eVIDEO_COLORSPACE_RGB,eVIDEO_COLORRANGE_FULL,eVIDEO_SCANNING_PROGRESSIVE,0,0,eVGA_SYNC_TYPE_UNKNOW,eVIDEO_YUV_UNKNOW};  //A70LV_Doulas_0109 modify    //A70LV_Doulas_0076
static UINT8 uc3D_Panel_Timer = 0;  //A70LV_Doulas_0154
static UINT8 ucDDPStatus = 0;
//static UINT8 ucCW_Status = 0;
static UINT8 m_ucAlreadyShowDisconnectTwistMsg = ets_OFF;       //A70LV_Doulas_0367 //A70LK_Simon_0018

#ifdef CUSTOM_CHRISTIE
static eDATA_PATH_STATE m_DataPathLastState;
#endif

static UINT16 m_uiInputDetect = 0;  //A35G2_Simon_0075
static UINT16 m_uiLastInputDetect = 0;  //A35G2_Simon_0075
UINT8 ucMenuTransparencyEnable = FALSE;     //A70LV_Doulas_0122

static UINT8 m_ucSmoothOnReadyCheck = 0;//HICC2_Julie_0037

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

    if((!palDataMgr_IsOsdOpen()) || Force)
    {
        //GuiCb.fpGui_SendSplashEventCb(1);
        palDataMgr_UI_EventSend(edcUI_EVENT_SPLASH, 1, NULL);
    }
}

static void palDataPath_TurnOffSplash(void)
{
    m_sSourceDesc.eActiveDisplay = eDATA_PATH_ACTIVE_DISPLAY_EXTERNAL;  //A70LV_Doulas_0032 modify
    //GuiCb.fpGui_SendSplashEventCb(0);
    palDataMgr_UI_EventSend(edcUI_EVENT_SPLASH, 0, NULL);
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

  #ifdef ENABLE_ACU_TWIST_LINK_CHECK
    if(palGeo_ApLinkFlag_Get() == TRUE)
    {
        LOG_MSG(db_APP_DATAPATH, "palDataPath_Color_Uniformity_PictureMode_Check(), End, Twist Linked, abort!!\r\n");  //G100_Tim_0018, mod
        return;
    }
  #endif //ENABLE_ACU_TWIST_LINK_CHECK

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
	if(palDataMgr_CU_Data_Enable_Get() &&
	   palDataMgr_CU_Data_Status_Get() &&   //A70Gen2_Doulas_0051 ACU table//A35G2_Alan_0007
	   m_sPalDataPathInfo.ePanelTimingId == PANEL_2D_OUTPUT     //A70LK_Simon_0009
	)
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

static void palDataPath_LowLatencySetting(void)
{
    if((m_sPalDataPathInfo.ePanelTimingId == PANEL_3D_OUTPUT) &&
       (halScaler_LowLatencyEnable() == TRUE))
    {
	    //3D frame sequential, 2D high speed
		UINT8 uc3DMode = halScaler_3DEnable_Get();
        UINT8 ucLowLatency = eCM_LOW_LATENCY_MODE_OFF;
        UINT16 uiFrameDelay = 0;

        palDataMgr_Data_Access(edcLOW_LATENCY_MODE, edaREAD, &ucLowLatency);

		if((ucLowLatency == eCM_LOW_LATENCY_MODE_OFF) &&
           (uc3DMode != eCM_3D_FORMAT_DUALPIPE_3D) &&
           (uc3DMode != eCM_3D_FORMAT_4K3D_DUALPIPE))
		{
			ucLowLatency = eCM_LOW_LATENCY_MODE_TYPICAL;
		}
        else if((ucLowLatency != eCM_LOW_LATENCY_MODE_OFF) && ((uc3DMode == eCM_3D_FORMAT_DUALPIPE_3D) || (uc3DMode == eCM_3D_FORMAT_4K3D_DUALPIPE)))
        {
            ucLowLatency = eCM_LOW_LATENCY_MODE_OFF;
        }

        palDataMgr_Data_Access(edcFRAME_DELAY, edaREAD, &uiFrameDelay);
        palDataMgr_Data_Access(edcLOW_LATENCY_MODE, edaWRITE_THROUGH_WITH_ACTION, &ucLowLatency);
        //MS_SLEEP(10);
        palDataMgr_Data_Access(edcFRAME_DELAY, edaWRITE_RAM_ONLY_WITH_ACTION, &uiFrameDelay);
    }
    else if(m_sPalDataPathInfo.ePanelTimingId == PANEL_3D_OUTPUT)
    {
    	//frame packing 24hz, top an bottom 50hz...
        UINT8 ucLowLatency = eCM_LOW_LATENCY_MODE_OFF;
        UINT16 uiFrameDelay = 0;

        palDataMgr_Data_Access(edcFRAME_DELAY, edaREAD, &uiFrameDelay);
        palDataMgr_Data_Access(edcLOW_LATENCY_MODE, edaREAD, &ucLowLatency);

        if(ucLowLatency != eCM_LOW_LATENCY_MODE_OFF)
        {
            ucLowLatency = eCM_LOW_LATENCY_MODE_OFF;

            palDataMgr_Data_Access(edcLOW_LATENCY_MODE, edaWRITE_RAM_ONLY_WITH_ACTION, &ucLowLatency);
            //MS_SLEEP(10);
            palDataMgr_Data_Access(edcFRAME_DELAY, edaWRITE_RAM_ONLY_WITH_ACTION, &uiFrameDelay);
        }
    }
	else //2D
    {
		UINT8 ucLowLatency = eCM_LOW_LATENCY_MODE_OFF;
		UINT8 uc3DMode = halScaler_3DEnable_Get();

        if(halScaler_PIP_PBP_Enable_Get() == ets_OFF)
        {
#ifdef AUTO_LOW_LATENCY_ENABLE
            if(palDataMgr_DataCode_Control(edcLOW_LATENCY_MODE) == eFUNC_CONTROL_ENABLE)
            {
                palDataMgr_Data_Access(edcLOW_LATENCY_MODE, edaREAD, &ucLowLatency);

            	if((halScaler_LowLatencyEnable() == TRUE) &&
                    (uc3DMode != eCM_3D_FORMAT_DUALPIPE) &&
                    (uc3DMode != eCM_3D_FORMAT_DUALPIPE_3D) &&
                    (uc3DMode != eCM_3D_FORMAT_4K3D) &&
                    (uc3DMode != eCM_3D_FORMAT_4K3D_DUALPIPE))
            	{
            	    if(ucLowLatency == eCM_LOW_LATENCY_MODE_OFF)
                    {
                        ucLowLatency = eCM_LOW_LATENCY_MODE_TYPICAL;
                    }
            	}
				else
				{
					ucLowLatency = eCM_LOW_LATENCY_MODE_OFF;
				}

                palDataMgr_Data_Access(edcLOW_LATENCY_MODE, edaWRITE_THROUGH_WITH_ACTION, &ucLowLatency);
            }
            else
#endif /* AUTO_LOW_LATENCY_ENABLE */
            {
                if((palDataMgr_DataCode_Control(edcLOW_LATENCY_MODE) == eFUNC_CONTROL_ENABLE) &&
                    (uc3DMode != eCM_3D_FORMAT_DUALPIPE) &&
                    (uc3DMode != eCM_3D_FORMAT_DUALPIPE_3D) &&
                    (uc3DMode != eCM_3D_FORMAT_4K3D) &&
                    (uc3DMode != eCM_3D_FORMAT_4K3D_DUALPIPE))
                {
                    palDataMgr_Data_Access(edcLOW_LATENCY_MODE, edaREAD, &ucLowLatency);
                }
                else if(palDataMgr_DataCode_Control(edcLOW_LATENCY_MODE) == eFUNC_CONTROL_ENABLE &&
                        (uc3DMode == eCM_3D_FORMAT_4K3D))
                {
                    if(ucLowLatency == eCM_LOW_LATENCY_MODE_OFF)
                    {
                        ucLowLatency = eCM_LOW_LATENCY_MODE_TYPICAL;
                    }
                }
                else
                {
                    ucLowLatency = eCM_LOW_LATENCY_MODE_OFF;
                }
                palDataMgr_Data_Access(edcLOW_LATENCY_MODE, edaWRITE_THROUGH_WITH_ACTION, &ucLowLatency);
            }
        }
        else //PIP on
        {
            ucLowLatency = eCM_LOW_LATENCY_MODE_OFF;
            palDataMgr_Data_Access(edcLOW_LATENCY_MODE, edaWRITE_THROUGH_WITH_ACTION, &ucLowLatency);
        }
    }
}

static void palDataPath_SourcePathSetting(UINT8 ucChangeSourceReset)
{
    if(m_sPalDataPathInfo.ucSourceEnhanceCheck)
    {  //A70LK_Larry_0169
        ucChangeSourceReset = 0;
    }

    if((halScaler_PIP_PBP_Enable_Get() == ets_OFF) && (halScaler_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_OFF))
	{
		if((m_sSourceDesc.eConnector == m_sSourceDesc.ucPrimaryInput) || (m_sSourceDesc.eConnector == m_sSourceDesc.ucSecondaryInput))
		{
            if(palImgMgr_BackupSwitchGet() != ets_ON)
            {
                palImgMgr_SclDatapathSet(eSOURCE_MODE_SEAMLESS);
				if(((m_sSourceDesc.eConnector == m_sSourceDesc.ucSecondaryInput) && (m_sSourceDesc.eConnector == palImgMgr_BackupPrimaryInput_Get())) ||
				  ((m_sSourceDesc.eConnector == m_sSourceDesc.ucPrimaryInput) && (m_sSourceDesc.eConnector != palImgMgr_BackupPrimaryInput_Get()) && (m_sSourceDesc.eConnector == palImgMgr_BackupSecondaryInput_Get())))
				{
                    if(m_sSourceDesc.ucSecondaryInput != palImgMgr_BackupPrimaryInput_Get())
                    {
						palFrontEndMgr_MainInput_Set((UINT8*)&m_sSourceDesc.ucSecondaryInput, TRUE, ucChangeSourceReset);
                        halScaler_BackupPrimaryInput_Set(m_sSourceDesc.ucSecondaryInput);
                        //halScaler_CurrentSourceRxPortSet(eSOURCE_WINDOW_MAIN, RXPORT_CHANNEL_1);
                        //halScaler_RxPortCheck(RXPORT_CHANNEL_1);
                    }

                    if(m_sSourceDesc.ucPrimaryInput != palImgMgr_BackupSecondaryInput_Get())
                    {
						palFrontEndMgr_SubInput_Set((UINT8 *)&m_sSourceDesc.ucPrimaryInput, TRUE, ucChangeSourceReset);
						halScaler_BackupSecondaryInput_Set(m_sSourceDesc.ucPrimaryInput);
                        //halScaler_CurrentSourceRxPortSet(eSOURCE_WINDOW_SUB, RXPORT_CHANNEL_0);
                        //halScaler_RxPortCheck(RXPORT_CHANNEL_0);
					}
				}
				else
				{
                    if(m_sSourceDesc.ucPrimaryInput != palImgMgr_BackupPrimaryInput_Get())
                    {
						palFrontEndMgr_MainInput_Set((UINT8*)&m_sSourceDesc.ucPrimaryInput, TRUE, ucChangeSourceReset);
						halScaler_BackupPrimaryInput_Set(m_sSourceDesc.ucPrimaryInput);
                        //halScaler_CurrentSourceRxPortSet(eSOURCE_WINDOW_MAIN, RXPORT_CHANNEL_0);
                        //halScaler_RxPortCheck(RXPORT_CHANNEL_0);
					}

                    if(m_sSourceDesc.ucSecondaryInput != palImgMgr_BackupSecondaryInput_Get())
                    {
						palFrontEndMgr_SubInput_Set((UINT8 *)&m_sSourceDesc.ucSecondaryInput, TRUE, ucChangeSourceReset);
						halScaler_BackupSecondaryInput_Set(m_sSourceDesc.ucSecondaryInput);
                        //halScaler_CurrentSourceRxPortSet(eSOURCE_WINDOW_SUB, RXPORT_CHANNEL_1);
                        //halScaler_RxPortCheck(RXPORT_CHANNEL_1);
					}
				}

				if(m_sSourceDesc.eConnector == palImgMgr_BackupSecondaryInput_Get())
				{
					//m_sSourceDesc.ucConnectorChannel = eSOURCE_WINDOW_SUB;
                    halScaler_CurrentSourceRxPortSet(eSOURCE_WINDOW_MAIN, RXPORT_CHANNEL_1);
                    halScaler_RxPortCheck(RXPORT_CHANNEL_1, ucChangeSourceReset);
				}
				else
				{
					//m_sSourceDesc.ucConnectorChannel = eSOURCE_WINDOW_MAIN;
                    halScaler_CurrentSourceRxPortSet(eSOURCE_WINDOW_MAIN, RXPORT_CHANNEL_0);
                    halScaler_RxPortCheck(RXPORT_CHANNEL_0, ucChangeSourceReset);
				}
			}
			else
			{

                palFrontEndMgr_MainInput_Set((UINT8*)&m_sSourceDesc.eConnector, TRUE, ucChangeSourceReset);
                halScaler_CurrentSourceRxPortSet(eSOURCE_WINDOW_MAIN, RXPORT_CHANNEL_0);
                palImgMgr_SclDatapathSet(eSOURCE_MODE_SIGNLE);
                //m_sSourceDesc.ucConnectorChannel = eSOURCE_WINDOW_MAIN;

                halScaler_RxPortCheck(RXPORT_CHANNEL_0, ucChangeSourceReset);
                halScaler_BackupPrimaryInput_Set(m_sSourceDesc.eConnector);
                halScaler_BackupSecondaryInput_Set(m_sSourceDesc.eConnector);

                if(m_sSourceDesc.eConnector == m_sSourceDesc.ucPrimaryInput)
                {
                    halScaler_BackupSecondaryInput_Set(m_sSourceDesc.ucSecondaryInput);
                    palFrontEndMgr_SubInput_Set((UINT8 *)&m_sSourceDesc.ucSecondaryInput, TRUE, ucChangeSourceReset);
                }
                else if(m_sSourceDesc.eConnector == m_sSourceDesc.ucSecondaryInput)
                {
                    halScaler_BackupSecondaryInput_Set(m_sSourceDesc.ucPrimaryInput);
                    palFrontEndMgr_SubInput_Set((UINT8 *)&m_sSourceDesc.ucPrimaryInput, TRUE, ucChangeSourceReset);
                }
			}
		}
		else
		{
			palFrontEndMgr_MainInput_Set((UINT8*)&m_sSourceDesc.eConnector, TRUE, ucChangeSourceReset);
            halScaler_CurrentSourceRxPortSet(eSOURCE_WINDOW_MAIN, RXPORT_CHANNEL_0);
            palImgMgr_SclDatapathSet(eSOURCE_MODE_SIGNLE);
            //m_sSourceDesc.ucConnectorChannel = eSOURCE_WINDOW_MAIN;
            halScaler_RxPortCheck(RXPORT_CHANNEL_0, ucChangeSourceReset);
			halScaler_BackupPrimaryInput_Set(m_sSourceDesc.eConnector);
			halScaler_BackupSecondaryInput_Set(m_sSourceDesc.eConnector);
			//palFrontEndMgr_SubInput_Set((UINT8 *)&m_sSourceDesc.eConnector, TRUE);
		}
    }
    else
    {
		palFrontEndMgr_MainInput_Set((UINT8*)&m_sSourceDesc.eConnector, TRUE, ucChangeSourceReset);
		halScaler_BackupPrimaryInput_Set(m_sSourceDesc.eConnector);
		halScaler_BackupSecondaryInput_Set(m_sSourceDesc.eConnector);
        halScaler_CurrentSourceRxPortSet(eSOURCE_WINDOW_MAIN, RXPORT_CHANNEL_0);

		if(halScaler_PIP_PBP_Enable_Get() == ets_OFF)
		{
            if((halScaler_3DEnable_Get() == eCM_3D_FORMAT_DUALPIPE_3D) || (halScaler_3DEnable_Get() == eCM_3D_FORMAT_4K3D_DUALPIPE))
            {
                eCM_SOURCE_ID eInput = m_sSourceDesc.eConnector;

                if(m_sSourceDesc.eConnector == eCM_SOURCE_HDMI1)
                {
                    eInput = eCM_SOURCE_HDMI2;
                    halScaler_BackupSecondaryInput_Set(eInput);
                }
                else if(m_sSourceDesc.eConnector == eCM_SOURCE_HDMI2)
                {
                    eInput = eCM_SOURCE_HDMI1;
                    halScaler_BackupSecondaryInput_Set(eInput);
                }
                palFrontEndMgr_SubInput_Set((UINT8*)&eInput, TRUE, ucChangeSourceReset);

                halScaler_CurrentSourceRxPortSet(eSOURCE_WINDOW_SUB, RXPORT_CHANNEL_1);
                halScaler_RxPortCheck(RXPORT_CHANNEL_1, ucChangeSourceReset);
            }
            else
            {
                if((m_sSourceDesc.eConnector == m_sSourceDesc.ucPrimaryInput) || (m_sSourceDesc.eConnector == m_sSourceDesc.ucSecondaryInput))
                {
					palImgMgr_SclDatapathSet(eSOURCE_MODE_SEAMLESS);
                }
                else
                {
                    palImgMgr_SclDatapathSet(eSOURCE_MODE_SIGNLE);
                }
            }
		}
		else
		{
            //halScaler_CurrentSourceRxPortSet(eSOURCE_WINDOW_SUB, RXPORT_CHANNEL_1);
            //halScaler_RxPortCheck(RXPORT_CHANNEL_1);
            palImgMgr_SclDatapathSet(eSOURCE_MODE_PIPPOP);
		}

        halScaler_RxPortCheck(RXPORT_CHANNEL_0, ucChangeSourceReset);
    }
}


static void palDataPath_GotoState(UINT32 ulNewState)
{
    eSOURCE_STATE ucSourceState;    //A70LV_Doulas_0241
    static eSOURCE_STATE ucLastSourceState = eDATA_PATH_STATE_INVALID;    //G100_Simon_0035

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
            palDataPath_SourcePathSetting(TRUE);
            halScaler_RxPortReset();

            /* poll every 0.5 seconds if time has elasped */
            m_sPalDataPathInfo.lPollPeriod = 500;//500ms
            break;

        case eDATA_PATH_STATE_SCALER_PORT_CONFIG:
        {
            /* poll every 0.5 seconds if time has elasped */
            UINT8 ucInputKeyFunction = 0;
            ucSourceState = eSOURCE_STATE_CHECKING_FOR_SIGNAL;    //A70LV_Doulas_0241
            //UINT8  uc3D_Enable = eCM_3D_FORMAT_OFF;

            palDataMgr_Data_Access(edcINPUT_KEY, edaREAD, &ucInputKeyFunction) ;
            palDataMgr_Data_Access(edcSHOW_MAINSOURCE_MESSAGES, edaREAD, &ucSourceState);      //A70LV_Doulas_0142 Add
            //palDataMgr_Data_Access(edc3D_ENABLE, edaREAD, &uc3D_Enable);
            if(ucInputKeyFunction == eSOURCE_KEY_CHANGE_SOURCE_AUTO && ucSourceState != eSOURCE_STATE_SEARCHING)
            {
                ucSourceState = eSOURCE_STATE_SEARCHING;
                palDataMgr_Data_Access(edcSHOW_MAINSOURCE_MESSAGES, edaWRITE_RAM_ONLY_NO_ACTION, &ucSourceState);
                if(!palDataMgr_IsOsdOpen()) //A70LK_Jacky_0013
                {
                    //appGui_SendUpdateOSDEvent();
					palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL);
                }
            }
            else if((ucInputKeyFunction != eSOURCE_KEY_CHANGE_SOURCE_AUTO) && (ucSourceState != eSOURCE_STATE_CHECKING_FOR_SIGNAL))
            {
                ucSourceState = eSOURCE_STATE_CHECKING_FOR_SIGNAL;
                palDataMgr_Data_Access(edcSHOW_MAINSOURCE_MESSAGES, edaWRITE_RAM_ONLY_NO_ACTION, &ucSourceState);

                if(!palDataMgr_IsOsdOpen()) //A70LK_Jacky_0013
                {
                    //appGui_SendUpdateOSDEvent();
					palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL);
                }
            }

            palDataPath_SourcePathSetting(TRUE);

            //halScaler_SetInputSource(eSOURCE_WINDOW_MAIN, m_sSourceDesc.eConnector);   //A70LV_Doulas_0007
			halScaler_Resync_Init(eSOURCE_WINDOW_MAIN);
            //halScaler_FrontEndScanMode_Clear(m_sSourceDesc.ucConnectorChannel);
            //MS_SLEEP(40);
            //m_sSourceDesc.ucAutoBackupInput = 0;
            //m_sSourceDesc.ucCurrentBackupInput = 0;
            halScaler_RxPortCheckPolling();

            if(palDataPath_AlreadyShowDisconnectTwistMsg_Flag_Get() == ets_ON)     //A70LV_Doulas_0367  //A70LK_Simon_0018
            {
                palDataPath_AlreadyShowDisconnectTwistMsg_Flag_Set(ets_OFF);
				palDataMgr_UI_EventSend(edcUI_EVENT_DISCONNECT_TWIST_MSG, FALSE, NULL);          //disable Twist OSD Message
            }

            m_sPalDataPathInfo.lPollPeriod = 10;//500ms
            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : eDATA_PATH_STATE_SCALER_PORT_CONFIG %d\r\n", __FUNCTION__, __LINE__,m_sSourceDesc.eConnector);
        }
        break;

        case eDATA_PATH_STATE_SUSPENDED:
        default:
            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : eDATA_PATH_STATE_SUSPENDED\r\n", __FUNCTION__, __LINE__);
            /* suspend the task indefinitely */
            m_sPalDataPathInfo.lPollPeriod = 0;
            break;

        case eDATA_PATH_STATE_TPG_DISPLAYED:
            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : eDATA_PATH_STATE_TPG_DISPLAYED\r\n", __FUNCTION__, __LINE__);
            palDataPath_AutoDisableMenuTransparency();

            if(m_sPalDataPathInfo.eDataPathState == eDATA_PATH_STATE_MONITOR_SOURCE || m_sPalDataPathInfo.bTestPatternTrigger)
            {
                m_sPalDataPathInfo.bTestPatternTrigger = TRUE;
            }
            else
            {
                m_sPalDataPathInfo.bTestPatternTrigger = FALSE;
            }

            m_sPalDataPathInfo.lPollPeriod = 0;    //A70LV_Doulas_0266 Add
#if 0
            /* suspend the task indefinitely, alert GUI */
            datapathTaskPollPeriod = 0;
            guiSourceStatusMessage(GUI_SOURCE_TPG);
#endif
            break;

        case eDATA_PATH_STATE_BEGIN_SCAN:
        {
            UINT8 ucLowLatency = 0;
            UINT8 ucFrameDelay = 0;
            UINT8 ucInputKeyFunction = 0;
            //UINT8 uc3D_Enable = eCM_3D_FORMAT_OFF;
            ucSourceState = eSOURCE_STATE_CHECKING_FOR_SIGNAL;    //A70LV_Doulas_0241
            /* manually flush mailbox to clean out old AutoLock status messages */
            /* without losing user messages                                     */
            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : eDATA_PATH_STATE_BEGIN_SCAN\r\n", __FUNCTION__, __LINE__);

            m_sPalDataPathInfo.ucSourceEnhanceCheck = 0;

            if(m_sPalDataPathInfo.bTestPatternTrigger)
            {
                m_sPalDataPathInfo.bTestPatternTrigger = FALSE;
                //appGui_SendUpdateOSDEvent();
				palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL);
            }
            palDataMgr_Data_Access(edcINPUT_KEY, edaREAD, &ucInputKeyFunction) ;
            palDataMgr_Data_Access(edcSHOW_MAINSOURCE_MESSAGES, edaREAD, &ucSourceState);
           // palDataMgr_Data_Access(edcSHOW_MAINSOURCE_MESSAGES, edaREAD, &ucSourceState);
            //palDataMgr_Data_Access(edc3D_ENABLE, edaREAD, &uc3D_Enable);
            if(ucInputKeyFunction == eSOURCE_KEY_CHANGE_SOURCE_AUTO && ucSourceState != eSOURCE_STATE_SEARCHING)
            {
                ucSourceState = eSOURCE_STATE_SEARCHING;
                palDataMgr_Data_Access(edcSHOW_MAINSOURCE_MESSAGES, edaWRITE_RAM_ONLY_NO_ACTION, &ucSourceState);
            }
            else if(ucInputKeyFunction == eSOURCE_KEY_CHANGE_SOURCE_AUTO && ucSourceState == eSOURCE_STATE_SEARCHING)
            {
            }
            else if((ucInputKeyFunction != eSOURCE_KEY_CHANGE_SOURCE_AUTO) && (ucSourceState != eSOURCE_STATE_CHECKING_FOR_SIGNAL))
            {
                ucSourceState = eSOURCE_STATE_CHECKING_FOR_SIGNAL;
                palDataMgr_Data_Access(edcSHOW_MAINSOURCE_MESSAGES, edaWRITE_RAM_ONLY_NO_ACTION, &ucSourceState);
            }

            if(!palDataMgr_IsOsdOpen()) //A70LK_Jacky_0013
            {
                //appGui_SendUpdateOSDEvent();
                palDataMgr_UI_EventSend(edcUI_EVENT_SOURCE_MENU_OPEN_SET, TRUE, NULL);
                palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL);
            }

            palDataPath_FreezeChecking(eSOURCE_WINDOW_MAIN);   //A70LV_Doulas_0223 Freeze check
            halScaler_ForcedSyncResetDisable_Set();     //A70LV_Doulas_0281

            palDataMgr_Data_Access(edcLOW_LATENCY_MODE, edaREAD, &ucLowLatency);
            if(ucLowLatency != eCM_LOW_LATENCY_MODE_OFF)
            {
                if(Board_SingleBoard_Get() == FALSE)
                {
                    UINT8 cMode = 0;
                    halScaler_FrameSyncModeGet(&cMode);
                    if(cMode != 0)
                    {
                        //off
                        halScaler_FrameSyncModeSet(eSOURCE_WINDOW_MAIN, 0); // for frame sync test
                        //MS_SLEEP(50);
                        palGeo_WarpDownScalingIssueWorkAround();
                    }
                }
            }

            m_sSourceDesc.ucAutoBackupInput = 0;

            //if(m_sSourceDesc.ucCurrentBackupInput)
            //{
                m_sSourceDesc.ucCurrentBackupInput = 0;
                palDataMgr_Data_Access(edcBACKUP_INPUT_CHANGE, edaWRITE_THROUGH_WITH_ACTION, &m_sSourceDesc.ucCurrentBackupInput);
            //}

            /* poll every 0.5 seconds if time has elasped */
            //MS_SLEEP(20);
            halScaler_InputSelectSet(eSOURCE_WINDOW_MAIN, (UINT8)m_sSourceDesc.eConnector, TRUE);

            if(eHAL_SCALER_EXEC_CODE_PASS != halScaler_RxPortIsReady(eSOURCE_WINDOW_MAIN, (UINT8)m_sSourceDesc.eConnector))
            {
                LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);//A70LV_Doulas_0231 debug
                palDataPath_ShowLogo();     //A70LV_Doulas_0049
            }

            palDataPath_ModeAdjusmenttDisableSetting();       //A70LV_Doulas_0209 modify//A70LV_Doulas_0195
            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : eDATA_PATH_STATE_BEGIN_SCAN End\r\n", __FUNCTION__, __LINE__);//A70LV_Doulas_0231 debug

			if(palFormatterMgr_FRC_Bypass_Flag_Get()) //A70LK_Doulas_0008
			{
				palFormatterMgr_FRC_BypassSet(0);
				halScaler_4K3D_DualPipeOrientationSet(palFormatterMgr_FRC_Bypass_Flag_Get(),palMotorEvent_RearGet(),palMotorEvent_CeilingGet()); 	//A70LK_Doulas_0017
			}

            if(m_sPalDataPathInfo.ePanelTimingId == PANEL_3D_OUTPUT)
            {
                halFormatter_3D_3DModeSet(FALSE);
            }

            if(palGeo_ApLinkFlag_Get() == TRUE &&
               palDataPath_IsErrorInputGetForTwistOn())   //A70LK_Simon_0018
            {
                m_sPalDataPathInfo.lPollPeriod = 1000;
            }
            else if(halScaler_RxPortIsReady(eSOURCE_WINDOW_MAIN, (UINT8)m_sSourceDesc.eConnector) == eHAL_SCALER_EXEC_CODE_PASS)
            {
                m_sPalDataPathInfo.lPollPeriod = 5;//100ms    //A70LV_Doulas_0120 modify//A70LV_Doulas_0075modify
            }
            else
            {
                m_sPalDataPathInfo.lPollPeriod = 100;//    //A70LV_Doulas_0120 modify/
            }

            if(m_sPalDataPathInfo.uiPanelChange == PANEL_CHECK_COUNT)
            {
                //避免再BEGIN_SCAN timeout
                //timeout 10s
                m_sPalDataPathInfo.uiPanelChange = PANEL_CHECK_COUNT*(1000/m_sPalDataPathInfo.lPollPeriod);
            }
            else if(m_sPalDataPathInfo.uiPanelChange != 0) //避免RGB enable沒打開
            {
                m_sPalDataPathInfo.uiPanelChange = 2;
            }

			#ifdef CUSTOM_CHRISTIE
			m_DataPathLastState = (eDATA_PATH_STATE)ulNewState;
			#endif
        }
        break;

        case eDATA_PATH_STATE_LOOK_FOR_SYNCS:
            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : eDATA_PATH_STATE_LOOK_FOR_SYNCS\r\n", __FUNCTION__, __LINE__);

            if(m_sPalDataPathInfo.uiPanelChange)
            {
               m_sPalDataPathInfo.uiPanelChange = PANEL_CHECK_COUNT;
            }
            m_sPalDataPathInfo.lPollPeriod = 100;//100ms
            break;

        case eDATA_PATH_STATE_ATTEMPT_LOCK:
        {
            ucSourceState = eSOURCE_STATE_SETTING_UP_IMAGE;        //A70LV_Doulas_0241 modify //A70LV_Larry_0350
            palDataMgr_Data_Access(edcSHOW_MAINSOURCE_MESSAGES, edaWRITE_RAM_ONLY_NO_ACTION, &ucSourceState) ;      //A70LV_Doulas_0075 Add source status
            m_sPalDataPathInfo.eDataPathState = (eDATA_PATH_STATE)ulNewState;
            m_sPalDataPathInfo.bTestPatternTrigger = FALSE;

#if 0 //HICC2_Larry_0033
            if(!palDataMgr_IsOsdOpen()) //A70LK_Jacky_0013
            {
                //appGui_SendUpdateOSDEvent();
                palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL);
            }
#endif /* 0 */

            //if(m_sPalDataPathInfo.uiPanelChange == 0)
            //{
                palDataPath_LowLatencySetting();
            //}

            palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL); //appGui_SendUpdateOSDEvent();
            //MS_SLEEP(100); //HICC2_Larry_0033 mask //不加delay的話，有可能會來不及更新OSD,會閃一下logo

            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : eDATA_PATH_STATE_ATTEMPT_LOCK\r\n", __FUNCTION__, __LINE__);
            /* syncs found -- wait to lock to source */
            /* poll every 100 milliseconds if the source is detected */
		    //palDataPath_TurnOffSplash();        //A70LV_Doulas_0254 remove//A70LV_Doulas_0032
            m_sPalDataPathInfo.bXPRErrorCheck = TRUE;
            m_sPalDataPathInfo.lPollPeriod = 300;//100ms    //HICC2_Larry_0033 100 -> 300 //A70LV_Doulas_0007
        }
            break;

        case eDATA_PATH_STATE_AUTO_PHASE:     //A70LV_Doulas_0007
            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : eDATA_PATH_STATE_AUTO_PHASE\r\n", __FUNCTION__, __LINE__);
            /* poll every 50 milliseconds if the source is detected */
            palDataMgr_ResetPhaseRam();     //A70LV_Doulas_0117
            m_sPalDataPathInfo.lPollPeriod = 50;
            break;

        case eDATA_PATH_STATE_AUTO_POSITION:     //A70LV_Doulas_0007
            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : eDATA_PATH_STATE_AUTO_POSITION\r\n", __FUNCTION__, __LINE__);
            /* poll every 50 milliseconds if the source is detected */
            halScaler_AutoPsitionStart(eSOURCE_WINDOW_MAIN,TRUE);       //A70LV_Doulas_0118
            m_sPalDataPathInfo.lPollPeriod = 50;
            break;

        case eDATA_PATH_STATE_MONITOR_SOURCE:
            {
                UINT8 ucLowLatency = eCM_LOW_LATENCY_MODE_OFF;
                UINT8 ucPictureMode = 0;
                UINT8 IsOSDOpen = 0;

                palDataMgr_DisplayModeChecking();

                palDataMgr_Data_Access(edcUI_STATUS_IS_OSD_OPEN, edaREAD, &IsOSDOpen);
                if(IsOSDOpen == 1)
                {
                    palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL);
                }

			    ucSourceState = eSOURCE_STATE_SHOW_SOURCE_RESOLUTION;     //A70LV_Doulas_0241 modify
                LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : eDATA_PATH_STATE_MONITOR_SOURCE\r\n", __FUNCTION__, __LINE__);
                m_sPalDataPathInfo.ucSyncLockCount = 0;      //A70LV_Doulas_0009 Add
                m_sSourceDesc.ucCurrentBackupInput = 0; //A70LK_Larry_0133
                palDataMgr_Data_Access(edcBACKUP_INPUT_CHANGE, edaWRITE_THROUGH_WITH_ACTION, &m_sSourceDesc.ucCurrentBackupInput);
                palDataMgr_HV_Start_Position_Reset();       //A70LV_Doulas_0269
#ifdef __ICHIP_CONTROL__
                palImgMgr_WB_OutputEnableSet(m_sSourceDesc.ucConnectorChannel);     //A70LV_Doulas_0079
#endif /* __ICHIP_CONTROL__ */

                if((palImgMgr_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_DAUL_PIPE) ||
                   (palImgMgr_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_4K3D_DAUL_PIPE))
                {
                    palImgMgr_ConfigureColorSpace(eSOURCE_WINDOW_MAIN, TRUE);
                }
                else
                {
                    palImgMgr_ConfigureColorSpace(eSOURCE_WINDOW_MAIN, FALSE);
                }

                halScaler_ConfigureColor(eSOURCE_WINDOW_MAIN);

				palDataMgr_Formatter_3D_Set();
                if((halScaler_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_DAUL_PIPE) || (halScaler_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_4K3D_DAUL_PIPE)) //A70LK_Larry_0133
                {
                    if(eHAL_SCALER_EXEC_CODE_PASS == halScaler_RxPortIsReady(eSOURCE_WINDOW_SUB, palImgMgr_BackupSecondaryInput_Get()))
                    {
                        if(palImgMgr_ColorSpace_Get(eSOURCE_WINDOW_MAIN) == eCM_COLOR_SPACE_AUTO)
                        {
                            palImgMgr_AVIInfoFrame(eSOURCE_WINDOW_SUB);
                        }
                        halScaler_ScreenOff(eSOURCE_WINDOW_SUB, FALSE);
                        halScaler_InputFrameRateSetting(eSOURCE_WINDOW_SUB);
                    }
               }
#ifdef __ICHIP_CONTROL__
                halC789Ctrl_V_Start_Checking();     //A70LV_Doulas_0081 auto update C789 input V-start
#endif /* __ICHIP_CONTROL__ */
                //halC789Ctrl_OutputEnableSet(1);     //A70LV_Doulas_0193 remove//A70LV_Doulas_0079
                palDataPath_MenuTransparencyEnableSet(TRUE); //appGui_OSD_MenuTransparencyEnableSet(TRUE);     //A70LV_Doulas_0122 enable Menu Transparency

                if(m_sPalDataPathInfo.uiPanelChange)
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

                //palDataMgr_Data_Access(edcBACKUPINPUT_PRIMARY_INPUT, edaREAD, &m_sSourceDesc.ucPrimaryInput);
                palDataMgr_Data_Access(edcSHOW_MAINSOURCE_MESSAGES, edaWRITE_RAM_ONLY_NO_ACTION, &ucSourceState) ; //A70LV_Larry_0350
                palDataPath_TurnOffSplash() ; //A70LV_Larry_0350
                palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL); //appGui_SendUpdateOSDEvent();        //A70LV_Doulas_0208 Add
                palDataMgr_OPDSourceInfo(eSOURCE_WINDOW_MAIN);

                if(palDataPath_AlreadyShowDisconnectTwistMsg_Flag_Get() == ets_ON)     //A70LV_Doulas_0367  //A70LK_Simon_0018
                {
                    palDataPath_AlreadyShowDisconnectTwistMsg_Flag_Set(ets_OFF);
                    palDataMgr_UI_EventSend(edcUI_EVENT_DISCONNECT_TWIST_MSG, FALSE, NULL); //HICC2_Doulas_0053 //appGui_SendDisconnectTwistMessage(FALSE);    //disable Twist OSD Message
                }

                LOG_MSG(db_APP_DATAPATH, "(%s, %d) m_sPalDataPathInfo.bPanelChange = %d \n", __FUNCTION__, __LINE__, m_sPalDataPathInfo.uiPanelChange);

#if (FRC_BYPASS_ENABLE) //A70LK_Casper_0009
                palDataMgr_Data_Access(edcLOW_LATENCY_MODE, edaREAD, &ucLowLatency); //A70LK_Casper_0009

                if((ucLowLatency != eCM_LOW_LATENCY_MODE_OFF) && (halScaler_Input_3D_Format_Get() == eINPUT_3D_TYPE_OFF))
                {
                    palImgMgr_DisableRGB(eHAL_SCALER_RGB_ITEM_FRC);
                }
#endif
                palDataMgr_Format_Normal_WithSemaphoreSet();     //A70LV_Doulas_0046

#if (ENABLE_COLOR_UNIFORMITY == TRUE)   //G100_Simon_0090
                palDataPath_Color_Uniformity_PictureMode_Check();//A70LV_John_0126 Color Uniformity is only enabled in blending mode
#endif
                palDataMgr_Data_Access(edcPICTURE_SETTINGS, edaREAD, &ucPictureMode);

				//LOG_MSG(db_APP_DATAPATH,"Panel = %d \r\n",palSystem_PanelID_Get());	//A70LK_Doulas_0008
#if (FRC_BYPASS_ENABLE) //A70LK_Doulas_0017//A70LK_Doulas_0011 remove FRC bypass
                if(ucLowLatency != eCM_LOW_LATENCY_MODE_OFF) //A70LK_Casper_0009
                {
                    if(halScaler_Input_3D_Format_Get() == eINPUT_3D_TYPE_OFF)//A70LK_Doulas_0008 Modify,4K60hz panel set FRC bypass
                    {
                        palFormatterMgr_FRC_BypassSet(1);
                    }
    				else
    				{
    					if(palFormatterMgr_FRC_Bypass_Flag_Get())
    					{
    						palFormatterMgr_FRC_BypassSet(0);
    					}
    				}
                    palImgMgr_RGBEnableItemSet(eHAL_SCALER_RGB_ITEM_FRC); //A70LK_Casper_0009
                    utilCounter_Reg_TimerEvent(eTIMER_EVENT_BLACKING_DISABLE_COUNT, 3000, 0, 0, palImgMgr_EnableRGB, NULL); //A70LK_Casper_0009
                }
    		    halScaler_4K3D_DualPipeOrientationSet(palFormatterMgr_FRC_Bypass_Flag_Get(),palMotorEvent_RearGet(),palMotorEvent_CeilingGet());  //A70LK_Casper_0012
#endif
                LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : eDATA_PATH_STATE_MONITOR_SOURCE  End\r\n", __FUNCTION__, __LINE__);


#ifdef CUSTOM_CHRISTIE
				m_DataPathLastState = (eDATA_PATH_STATE)ulNewState;
#endif
                m_sPalDataPathInfo.lPollPeriod = 100;

            }
            break;

        case eDATA_PATH_STATE_SOURCE_LOST:
            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : eDATA_PATH_STATE_SOURCE_LOST\r\n", __FUNCTION__, __LINE__);
            palDataMgr_BeforeDisplayModeToDefault();
            halScaler_MeasureInput(eSOURCE_WINDOW_MAIN, (UINT8)m_sSourceDesc.eConnector);
            m_sPalDataPathInfo.lPollPeriod = 10;
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
        //utilIpc_NotifyDatacodeChanged(edcIS_SOURCE_LOCK);
    }

    ucLastSourceState = m_sPalDataPathInfo.eDataPathState;
}

eDATA_PATH_STATE palDataPath_GetDataPathState(void) //A70LV_Larry_0108
{
    return m_sPalDataPathInfo.eDataPathState;
}


static void palDataPath_StartDisplay(void)
{
    UINT8   ucData = 0; //A70LV_Larry_0051
    UINT16  uiPW_IndexValue = 0;
    UINT16  uiFW_IndexValue = 0;
	UINT8   ucDDPStatusRetry = 0; //T100_Casper_0087
	UINT8   ucSRCStatus[17]={0}; //T100_Casper_0087
	UINT8   ucVer[32]={'\0'};
    UINT32  ulVersion = 0;
    UINT16  uiCount = 0;
    UINT8   uc3DMode = 0;
    UINT32  ulData = 0;
    UINT16  uiData = 0;
    UINT8   ucSN[32] = {0};
    UINT32  ulZData0 = 0;
    UINT32  ulZData1 = 0;
    UINT8   ucFirstLanguage = 0;
    UINT8   ucPictureMode = 0;
    UINT8   ucPreUserMode = 0;

    LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);

    //palGeo_Palette_Init();  //H2PF_Simon_0052

    #ifdef CUSTOM_CHRISTIE
    UINT8 ucString[32] = {0}, ucIndex = 0;
    palDataMgr_ServiceModeSet(ets_ON);
	palDataMgr_Data_Access(edcSERIAL_NUMBER, edaREAD, ucString);
	palDataMgr_Data_Access(edcSERIAL_NUMBER, edaWRITE_RAM_ONLY_WITH_ACTION, ucString);
    palDataMgr_ServiceModeSet(ets_OFF);
    #endif

	//Init Auto Source Reync
	palDataMgr_Data_Access(edcAUTO_SOURCE_RESYNC, edaREAD, &ucData);						//G100_Doulas_0006
    palDataMgr_Data_Access(edcAUTO_SOURCE_RESYNC, edaWRITE_THROUGH_WITH_ACTION, &ucData);	//G100_Doulas_0006

    palDataMgr_Data_Access(edcEQ_MODE_HDMI1, edaREAD, &ucData);
    palDataMgr_Data_Access(edcEQ_MODE_HDMI1, edaWRITE_RAM_ONLY_WITH_ACTION, &ucData);
    palDataMgr_Data_Access(edcEQ_MODE_HDMI2, edaREAD, &ucData);
    palDataMgr_Data_Access(edcEQ_MODE_HDMI2, edaWRITE_RAM_ONLY_WITH_ACTION, &ucData);

 	palDataMgr_Data_Access(edcHDMI_OUT,edaREAD, &ucData);
 	palDataMgr_Data_Access(edcHDMI_OUT,edaWRITE_RAM_ONLY_WITH_ACTION, &ucData);

    palDataPath_FreezeChecking(eSOURCE_WINDOW_MAIN);
    ucData = ets_OFF;
    palDataMgr_Data_Access(edcIMAGE_FREEZE, edaWRITE_THROUGH_WITH_ACTION, &ucData);

    palDataPath_InitInputSource();  //A70LV_Doulas_0007
    palDataMgr_Scaler_EEPROM_Iint();    //A70LV_Doulas_0229 Add    //move to palDataMgr_Init()

    ucData = eSOURCE_STATE_CHECKING_FOR_SIGNAL;
    palDataMgr_Data_Access(edcSHOW_MAINSOURCE_MESSAGES, edaWRITE_RAM_ONLY_NO_ACTION, &ucData);

    palDataMgr_Data_Access(edcMAIN_INPUT, edaREAD, &ucData);
    m_sSourceDesc.eConnector = ucData;

    m_sSourceDesc.ucAutoBackupInput = 0;
    m_sSourceDesc.ucCurrentBackupInput = 0;
	//m_sSourceDesc.ucConnectorChannel = eSOURCE_WINDOW_MAIN;
    halScaler_SetInputSource(eSOURCE_WINDOW_MAIN, m_sSourceDesc.eConnector);   //A70LV_Doulas_0007
    palFrontEndMgr_MainInput_Set((UINT8*)&m_sSourceDesc.eConnector, TRUE, TRUE);

    //halScaler_PowerNormal(eSOURCE_WINDOW_MAIN, m_sPalDataPathInfo.ePanelTimingId);
    //palGeo_PowerNormal(m_sPalDataPathInfo.ePanelTimingId);
    //palDataMgr_Data_Access(edcBACKUPINPUT_AUTOSWITCH, edaREAD, &ucData);
    //palDataMgr_Data_Access(edcBACKUPINPUT_AUTOSWITCH, edaWRITE_RAM_ONLY_WITH_ACTION, &ucData);
    palDataMgr_Scaler_Mode_Adjustment_EEPROM_Iint();  //A70LV_Doulas_0195 Add

    //appGui_HWInit();

    ulVersion = halScaler_VersionGet();
    sprintf((char*)ucVer, "Q%02d.%02d.%02d\0", (ulVersion >> 8)&0xFF, ulVersion&0xFF,  (ulVersion >> 16)&0xFF);
    palDataMgr_Data_Access(edcXFPGA_VERSION, edaWRITE_RAM_ONLY_NO_ACTION, &ucVer[0]);

    if(Board_SingleBoard_Get() == FALSE)
    {
        for(uiCount = 0; uiCount < 600; uiCount++) //A70LK_Larry_0096
        {
            if(TRUE != halDDP4422_IsAsicReady())
            {
                MS_SLEEP(100);

                if((uiCount%10) == 0)
                {
                    LOG_MSG(db_HAL_FORMATTER, "datapath wait ready = %d\r\n", uiCount/10);
                }
            }
            else
            {
                break;
            }
        }
    }

    if(m_sPalDataPathInfo.ePanelTimingId == PANEL_2D_HIGHSPEED)
    {
        palSystem_ActuatorEnable_Set(FALSE);
        palFormatterMgr_XPR_Set(eXPR_OFF_4WAY_240);
        halFormatter_XPR_DataSwap_Set(0);
    }
    else
    {
        //XPR init true
        palSystem_ActuatorEnable_Set(TRUE);
        palFormatterMgr_XPR_Set(eXPR_ON);
        MS_SLEEP(50);
    	halFormatter_XPR_3DLR_Set(TRUE);
        halFormatter_XPR_DataSwap_Set(1);
    }

    palFormatterMgr_AutoLockResync(); //A70LK_Larry_0096

    //palDataMgr_Data_Access(edcLOW_LATENCY_MODE, edaREAD, &ucData);
    //palDataMgr_Data_Access(edcLOW_LATENCY_MODE, edaWRITE_RAM_ONLY_WITH_ACTION, &ucData);

    //halFormatter_3D_DLPLinkPulseSet(FALSE);          //A70LV_Doulas_0161 init DLP Link off
    //palDataMgr_Data_Access(edc3D_ENABLE, edaREAD, &ucData);
    //palDataMgr_Data_Access(edc3D_ENABLE, edaWRITE_RAM_ONLY_WITH_ACTION, &ucData);
    {
        UINT16  wRX24TVer = 0;
        UINT8   aucVerString[32] = {0};
        UINT32  ulI2CDiag[2] = {0};

        if(Board_SingleBoard_Get() == FALSE)
        {

        #if defined(PLATFORM_R70K) //HICC2_AC_0003 //for DDP7541
            do
            {
                halFormatter_SRCDescriptionGet(ucSRCStatus);
                ucDDPStatusRetry++;
                MS_SLEEP(500);
            }while(ucSRCStatus[16] != DP_STATE_MONITORING && ucDDPStatusRetry <= 35);
        #else
            do
            {
                halFormatter_SRCDescriptionGet(ucSRCStatus);
                ucDDPStatusRetry++;
                MS_SLEEP(500);
            }while(ucSRCStatus[16] != DPP_MONITOR_SOURCE && ucDDPStatusRetry <= 35); //T100_Casper_0087
        #endif

            if(ucDDPStatusRetry <=35) //T100_Casper_0087
            {
                LOG_MSG(db_APP_SYSTEM, "(@%s,#%d) : DDP Datapath Ready\r\n", __FUNCTION__, __LINE__);
            }
            else
            {
                LOG_MSG(db_ALWAYS, "(@%s,#%d) : DDP Datapath not Ready\r\n", __FUNCTION__, __LINE__);
            }
        }

        halFormatter_RX24TVersion_Get(&wRX24TVer);

        LOG_MSG(db_APP_SYSTEM, "RX24T      P%02d.%02d\n",
            (UINT8)(wRX24TVer),
            (UINT8)(wRX24TVer>>8));

        sprintf((char*)aucVerString, "P%02d.%02d\0" ,
            (UINT8)(wRX24TVer),
            (UINT8)(wRX24TVer>>8));

        palDataMgr_Data_Access(edcPMCU_VERSION, edaWRITE_RAM_ONLY_NO_ACTION, aucVerString);

        halFormatter_GetI2C_Diag((UINT8*)&ulI2CDiag);

        if(ulI2CDiag[1]) //error
        {
            for(uiCount = 0; uiCount < DDP_I2C_DIAG_NUMBER; uiCount++)
            {
                if(((ulI2CDiag[0] >> uiCount) & 0x01) && ((ulI2CDiag[1] >> uiCount) & 0x01))
                {
                    UINT32 ulErrorIndex = I2CError26;
	                ulErrorIndex += uiCount;
                    utilDataMgr_WriteGecLog(ulErrorIndex);
                }
            }

            LOG_MSG(db_ALWAYS, "(%s,%d) : DDP i2c diag waork %08x error %08x\r\n", __FUNCTION__, __LINE__, ulI2CDiag[0], ulI2CDiag[1]);
        }
    }

    //set wheel index
    palDataMgr_Data_Access(edcPHOSPHOR_WHEEL_INDEX, edaREAD, &uiPW_IndexValue);
    palDataMgr_Data_Access(edcPHOSPHOR_WHEEL_INDEX, edaWRITE_RAM_ONLY_WITH_ACTION, &uiPW_IndexValue);
    palDataMgr_Data_Access(edcFILTER_WHEEL_INDEX, edaREAD, &uiPW_IndexValue);
    palDataMgr_Data_Access(edcFILTER_WHEEL_INDEX, edaWRITE_RAM_ONLY_WITH_ACTION, &uiPW_IndexValue);

    //set actuator
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
            ucPictureMode = eCM_PICTURE_SETTINGS_BRIGHT; //default picture mode
            palDataMgr_Data_Access(edcPICTURE_SETTINGS, edaWRITE_RAM_ONLY_NO_ACTION, &ucPictureMode);
        }
    }

    palDataMgr_Format_Normal_WithSemaphoreSet();   //A70LV_Doulas_0046 Add,init DDP442x

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

    palDataPath_Init_FrontEnd_PIP_Setting();    //A70LV_Doulas_0120

    palFormatterMgr_XPR_SN_Get(ucSN);
    palDataMgr_Data_Access(edcCLOSE_LOOP_SN, edaWRITE_RAM_ONLY_WITH_ACTION, ucSN);

    palFormatterMgr_XPR_Eeprom_ZData0_Get(&ulZData0);
    palDataMgr_Data_Access(edcXPR_CLOSELOOP_Z_DATA0, edaWRITE_RAM_ONLY_WITH_ACTION, &ulZData0);
    palFormatterMgr_XPR_Eeprom_ZData1_Get(&ulZData1);
    palDataMgr_Data_Access(edcXPR_CLOSELOOP_Z_DATA1, edaWRITE_RAM_ONLY_WITH_ACTION, &ulZData1);

#ifdef __ICHIP_CONTROL__
    halC789Ctrl_Init(m_sPalDataPathInfo.ePanelTimingId); //A70LV_Larry_0124
#endif /* __ICHIP_CONTROL__ */

    palGeo_DBD_Blending_Init();
    palGeo_BlackLevel_Init();

    //palDataMgr_Access_Color_Uniformity_Table_Init();

	palDataMgr_Data_Access(edcMENU_LOCATION, edaREAD, &ucData);
	palDataMgr_Data_Access(edcMENU_LOCATION, edaWRITE_THROUGH_WITH_ACTION, &ucData);
    palDataMgr_MenuOffsetUpdateForEdgeBlending();

#ifndef __ICHIP_CONTROL__
    palDataMgr_Data_Access(edcMAIN_INPUT, edaREAD, &ucData); //A70LV_Larry_0139
	m_sSourceDesc.eConnector = ucData;
    halScaler_InputSelectSet(eSOURCE_WINDOW_MAIN, ucData, TRUE);
#endif /* __ICHIP_CONTROL__ */

	#if 0
    palDataMgr_Data_Access(edcBACKUPINPUT_AUTOSWITCH, edaREAD, &ucData);

    if(ucData == eSOURCE_MODE_BACKUP)
    {
        palDataMgr_Data_Access(edcBACKUPINPUT_PRIMARY_INPUT, edaREAD, &ucData);
        m_sSourceDesc.ucPrimaryInput = ucData;
        palDataMgr_Data_Access(edcBACKUPINPUT_SECONDARY_INPUT, edaREAD, &ucData);
        m_sSourceDesc.ucSecondaryInput = ucData;
    }
    else
    {
        m_sSourceDesc.ucPrimaryInput = eCM_SOURCE_NUMBER;
        m_sSourceDesc.ucSecondaryInput = eCM_SOURCE_NUMBER;
    }
    #else
    halScaler_BackupPrimaryInput_Set(eCM_SOURCE_NUMBER);
    halScaler_BackupSecondaryInput_Set(eCM_SOURCE_NUMBER);
	palDataMgr_Data_Access(edcBACKUPINPUT_PRIMARY_INPUT, edaREAD, &ucData);
	m_sSourceDesc.ucPrimaryInput = ucData;
	palDataMgr_Data_Access(edcBACKUPINPUT_SECONDARY_INPUT, edaREAD, &ucData);
	m_sSourceDesc.ucSecondaryInput = ucData;
    #endif

    uc3D_Panel_Timer = 0;

#ifdef ENABLE_LOGO_REPLACEMENT
    appGui_LoadLogoReplacement();
#endif /* ENABLE_LOGO_REPLACEMENT */

	halMCU_SystemReadySet();

    palDataMgr_3DModeConditionUpdate(); //A70LK_Jacky_0014


#if 0 //A70LK_Larry_0132
    uc3DMode = halScaler_3DEnable_Get(); //A70LK_Larry_0121

    halScaler_3DEnable_Set(eCM_3D_FORMAT_OFF); //A70LK_Larry_0121
    halScaler_ConfigureForScaler(eSOURCE_WINDOW_MAIN);  //A70LK_Sammy_0003

    halScaler_3DEnable_Set(uc3DMode); //A70LK_Larry_0121
#endif /* 0 */

    palDataPath_GotoState(eDATA_PATH_STATE_SPLASH_AT_STARTUP);
 //   palEnvironment_XillinxFPGA_Reset();     //A70LV_Doulas_0216 remove//A70LV_Doulas_0197 fixed Xillinx FPGA fail on the projecter power on
    //halBoard_XillinxFPGA_Init_Set(m_sPalDataPathInfo.ePanelTimingId);    //A70LV_Doulas_0216

}


static void palDataPath_StateMachineProc(UINT16 uiMsgID)
{
    //BOOL bSyncLock; //A70LV_Doulas_0007
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_FAIL; //A70LV_Doulas_0009 Add

    if((palSystem_PowerStateGet() == ePOWER_STATE_STANDBY) || (palSystem_PowerStateGet() == ePOWER_STATE_UPGRADE)) //A70LV_Larry_0112
    {
        return;
    }

    if(palDataMgr_CurTestPatternGet() != eTID_OFF) //test pattern on
    {
        return;
    }

    switch(m_sPalDataPathInfo.eDataPathState)
    {
        case eDATA_PATH_STATE_SPLASH_AT_STARTUP:
        {
            UINT8 ucValue = 0;

            #if 0 //defined(CUSTOM_BARCO) //A35G2_Coda_0142
            if(((palDataMgr_PIN_Protect_Checking() == ePASSWORD_PROTECT_UNLOCK) && (appGui_IsFirstStartupMenu() == FALSE))
            #else
            palDataMgr_Data_Access(edcFIRSTSTARTUPFLAG, edaREAD, &ucValue);
            if(((palDataMgr_PIN_Protect_Checking() == ePASSWORD_PROTECT_UNLOCK) && (ucValue == ets_OFF))
            #endif
                || (palDataPath_TimeElapsedInState() >= DATAPATH_STATE_FIRST_LANGAGE_MEOUT))
            {
                if(ucValue == ets_ON)
                {
                    //ucValue = appGui_Get_Cursor_Item_Number() - 1;
                    //if(ucValue >= eGUI_LANGUAGE_NUMBER) //R70K_AC_0070
                    {
                        ucValue = eCM_LANGUAGE_ENGLISH;
                    }
                    palDataMgr_Data_Access(edcLANGUAGE, edaWRITE_THROUGH_WITH_ACTION, &ucValue);
                }

                if(m_ucSmoothOnReadyCheck == 1)
                {
                    palDataMgr_UI_EventSend(edcFIRSTSTARTUPFLAG, FALSE, NULL);

                    LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d)eDATA_PATH_STATE_SPLASH_AT_STARTUP GoTo  eDATA_PATH_STATE_SCALER_PORT_CONFIG \r\n", __FUNCTION__, __LINE__);
                    palDataPath_GotoState(eDATA_PATH_STATE_SCALER_PORT_CONFIG);
                }
            }
            else
            {
                if(palDataMgr_PIN_Protect_Checking() != ePASSWORD_PROTECT_UNLOCK)
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
                    if(halScaler_PIP_PBP_Enable_Get() != eCM_SCREEN_MODE_OFF)    //A70LV_Doulas_0142 Add
                    {
                        halScaler_Resync_Init(eSOURCE_WINDOW_MAIN);
                        uPALIMGMGR_INFO uInfo = {.sConfig_NoSignalOutput.eWindow = eSOURCE_WINDOW_MAIN,
                                                 .sConfig_NoSignalOutput.ucDisplayOutput = 2}; //keep vop out
                        palImgMgr_Config_NoSignalOutput(&uInfo);
                    }

                    if(m_sPalDataPathInfo.ePanelTimingId == PANEL_3D_OUTPUT)      //A70LV_Doulas_0154
                    {
                        palDataPath_ResyncSub();
                    }

                    //MS_SLEEP(2000);
                    LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);

                    if(halScaler_RxPortIsReady(eSOURCE_WINDOW_MAIN, (UINT8)m_sSourceDesc.eConnector) == eHAL_SCALER_EXEC_CODE_PASS)
                    {
                        if(palSystem_PanelID_Get() != palDataPath_PanelOutputGet())
                        {
                            palDataPath_GotoState(eDATA_PATH_STATE_BEGIN_SCAN);
                            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
                        }
                        else if(palGeo_ApLinkFlag_Get() == TRUE)
                        {
                            palDataPath_GotoState(eDATA_PATH_STATE_BEGIN_SCAN);
                        }
                        else if(halScaler_InputCompare(eSOURCE_WINDOW_MAIN, (UINT8)m_sSourceDesc.eConnector))
                        {
                            UINT8 ucLowLatency = 0;

                            //如果要做切source不遮黑，要設FALSE
                            halScaler_InputSelectSet(eSOURCE_WINDOW_MAIN, (UINT8)m_sSourceDesc.eConnector, TRUE);

                            palDataPath_FreezeChecking(eSOURCE_WINDOW_MAIN);   //A70LV_Doulas_0223 Freeze check
                            halScaler_ForcedSyncResetDisable_Set();     //A70LV_Doulas_0281

                            palDataMgr_Data_Access(edcLOW_LATENCY_MODE, edaREAD, &ucLowLatency);
                            if(ucLowLatency != eCM_LOW_LATENCY_MODE_OFF)
                            {
                                if(Board_SingleBoard_Get() == FALSE)
                                {
                                    UINT8 cMode = 0;
                                    halScaler_FrameSyncModeGet(&cMode);
                                    if(cMode != 0)
                                    {
                                        //off
                                        halScaler_FrameSyncModeSet(eSOURCE_WINDOW_MAIN, 0); // for frame sync test
                                        //MS_SLEEP(50);
                                        palGeo_WarpDownScalingIssueWorkAround();
                                    }
                                }
                            }

                            palDataPath_GotoState(eDATA_PATH_STATE_LOOK_FOR_SYNCS);
                            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);

                            //MS_SLEEP(2000);

                            uc3D_Panel_Timer = 0;  //A70LV_Doulas_0154
                            if(palDataPath_Input3D_FormatGet() == eVIDEO_3D_FORMAT_2D)
                            {
                                halScaler_Input_3D_Format_Set(eINPUT_3D_TYPE_OFF);
                            }
                            palDataPath_Input3D_PanelSet();         //A70LV_Doulas_0159
                        }
                        else
                        {
                            palDataPath_GotoState(eDATA_PATH_STATE_BEGIN_SCAN);
                            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
                        }
                    }
                    else
                    {
                        palDataPath_GotoState(eDATA_PATH_STATE_BEGIN_SCAN);     //A70LV_Doulas_0004
                        LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
                    }
                    break;

                case eDATA_PATH_MSG_PANEL_CHANGE:   //A70LV_Doulas_0154
                    LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) eDATA_PATH_MSG_PANEL_CHANGE\r\n", __FUNCTION__, __LINE__);
                    palDataPath_GotoState(eDATA_PATH_STATE_BEGIN_SCAN);
                    break;

                case eDATA_PATH_MSG_RESYNC:     //A70LV_Doulas_0004
                    LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d)eDATA_PATH_MSG_RESYNC\r\n", __FUNCTION__, __LINE__);
					palDataPath_GotoState(eDATA_PATH_STATE_SCALER_PORT_CONFIG);
				    break;
            }
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

#if 0

            switch(uiMsgID)
            {
                case DPMSG_STATE_TOGGLEBLANK:
                    if(!blankScreenEnabled)
                    {
                        blankScreenEnabled = TRUE;
                    }
                    else
                    {
                        //----- Roger Added 20061220  Start
                        blankScreenEnabled = FALSE;

                        //----- Alex Modified 20070530 Start
                        //corevariable_SetIsConnectorScanStopVal(FALSE);
                        /* leave SFG/splash displayed for search -- no action */
                        if(!corevariable_GetIsConnectorScanStopVal())
                        {
                            datapath_GotoState(BEGIN_SCAN);    /* restart source search */
                        }

                        //----- Alex Modified 20070530 End
                        //----- Roger Added 20061220  End
                    }

                    break;

                case DPMSG_STATE_RESUMESCAN:
                    if(!blankScreenEnabled)
                    {
                        datapath_GotoState(BEGIN_SCAN);
                    }

                    break;
            }

#endif
            break;

        case eDATA_PATH_STATE_TPG_DISPLAYED:
        //    LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d)%d : eDATA_PATH_STATE_TPG_DISPLAYED\r\n", __FUNCTION__, __LINE__,uiMsgID);    //A70LV_Doulas_0035

            /* an external state change request is the only way to exit suspended state */
            switch(uiMsgID)     //A70LV_Doulas_0266 Add
            {
                default:
                {
                    INT32 iSetting = 0;
                    eCM_TEST_PATTERN_ID ucHSG_Pattern = eCM_TEST_PATTERN_OFF;

                    palDataMgr_Data_Access(edcHSG_TEST_PATTERN_CTRL, edaREAD, &ucHSG_Pattern) ;
                    if(ucHSG_Pattern == eCM_TEST_PATTERN_OFF) //(appGui_HSG_TestPatternEnableGet()) //###
                    {

                    }
                    else
                    {
                        if(palDataMgr_CurTestPatternGet() == eTID_OFF)
                        {
                            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
                            palDataPath_GotoState(eDATA_PATH_STATE_LOOK_FOR_SYNCS);       //A70LV_Doulas_0293 modify
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
                    palDataPath_GotoState(eDATA_PATH_STATE_BEGIN_SCAN);
                    break;

                case eDATA_PATH_MSG_NONE:   //A70LV_Doulas_0007
                    {
#ifndef	__ICHIP_CONTROL__
                        static eHAL_SCALER_EXEC_CODE eResult3 = eHAL_SCALER_EXEC_CODE_DV_RX_INIT_FAIL;
                        static eHAL_SCALER_EXEC_CODE eResult4 = eHAL_SCALER_EXEC_CODE_DV_RX_INIT_FAIL;
                        static UINT8 ucHDRStatus = 0;
#endif /* __ICHIP_CONTROL__ */
                        UINT8 ucBurnInEnable = 0;   //A70LV_Doulas_0106
                        palDataMgr_Data_Access(edcBURNIN_ENABLE, edaREAD, &ucBurnInEnable);
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

#if 0 //3D source掉訊號後，保持解析度
                        uc3D_Panel_Timer++;  //A70LV_Doulas_0154
                        if(uc3D_Panel_Timer >= 45)//30)  //4.5 sec         //A70LV_Doulas_0235 modify
                        {
                            BOOL  bDDP442x_3D_Get = 0;
                            uc3D_Panel_Timer = 0;
                            if(m_sPalDataPathInfo.ePanelTimingId == PANEL_3D_OUTPUT)      //A70LV_Doulas_0216
                            {
                                halScaler_Input_3D_Format_Set(eINPUT_3D_TYPE_OFF);
                                palSystem_ChangePanelID(PANEL_2D_OUTPUT);
                                MS_SLEEP(200);
                                appGui_SendUpdateOSDEvent();
                                palDataMgr_BeforeDisplayModeToDefault();
                            }
                            if(palDataMgr_Is_Formatter_Setting() == FALSE)  //A70LV_Doulas_0271 Add
                            {
                                halFormatter_3D_3DModeGet(&bDDP442x_3D_Get);
                                if(bDDP442x_3D_Get == TRUE)
                                {
                                halFormatter_3D_3DModeSet(FALSE);
                                halScaler_Input_3D_Format_Set(eINPUT_3D_TYPE_OFF);      //A70LV_Doulas_0187 Add
                                halScaler_FrameDelay_Set(FRAME_DELAY_DEFAULT_VALUE);
                                palDataMgr_Formatter_Parameter_Set();       //A70LV_Doulas_0160 update system mode and PWM ...
                                }
                            }
                            //LOG_MSG(db_ALWAYS, "+++Timer+++\r\n");
                        }
#endif /* 0 */
                        //eResult2 = halFrontEndCtrl_videoReady_Get(&ucVal);
                        halScaler_RxPortCheckPolling();
                        //eResult3 = halScaler_Monitor(0);
                        eResult3 = halScaler_RxPortIsReady(eSOURCE_WINDOW_MAIN, (UINT8)m_sSourceDesc.eConnector);

                        if(eResult3 == eHAL_SCALER_EXEC_CODE_PASS)// && (ucVal & 0x01))   //A70LV_Doulas_0112 modify
                        {
                            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) cPanelChange = %d\r\n", __FUNCTION__, __LINE__, m_sPalDataPathInfo.uiPanelChange);

                            if(palGeo_ApLinkFlag_Get() == TRUE)    //A70LK_Simon_0018                                    //ZU860_Doulas_0138
                            {
                                LOG_MSG(db_APP_DATAPATH, "(TWIST Link on)\r\n");

                                if(palDataPath_IsErrorInputGetForTwistOn())
                                {
                                    LOG_MSG(db_APP_DATAPATH, "(BLOCK Source)\r\n");

                                    palDataPath_GotoState(eDATA_PATH_STATE_BEGIN_SCAN);
                                    if(palDataPath_AlreadyShowDisconnectTwistMsg_Flag_Get() == ets_OFF)
                                    {
                                        palDataPath_AlreadyShowDisconnectTwistMsg_Flag_Set(ets_ON);
                                        palDataMgr_UI_EventSend(edcUI_EVENT_DISCONNECT_TWIST_MSG, TRUE, NULL);
                                    }
                                    break;
                                }
                            }

                            if((halScaler_ColorSpace_Get(eSOURCE_WINDOW_MAIN) == eCM_COLOR_SPACE_RGB_FULL) || (halScaler_ColorSpace_Get(eSOURCE_WINDOW_MAIN) == eCM_COLOR_SPACE_RGB_LIMITED)) //A70LK_Nina_0047
                            {
                                if(halScaler_PixModIsYUV420(eSOURCE_WINDOW_MAIN, (UINT8)m_sSourceDesc.eConnector))
                                {
                                    UINT8 ucValue = eCM_COLOR_SPACE_YUV_REC709;
                                    palDataMgr_Data_Access(edcCOLOR_SPACE, edaWRITE_THROUGH_WITH_ACTION, &ucValue);
                                }
                            }

                            palDataPath_GotoState(eDATA_PATH_STATE_LOOK_FOR_SYNCS);

                            uc3D_Panel_Timer = 0;  //A70LV_Doulas_0154

                            if(m_sPalDataPathInfo.uiPanelChange) //A70LK_Larry_0096 modify
                            {
                                m_sPalDataPathInfo.uiPanelChange = PANEL_CHECK_COUNT;
                            }
                            palDataPath_Input3D_PanelSet();         //A70LV_Doulas_0159
                        }
                        else
                        {
                            if((halScaler_PIP_PBP_Enable_Get() == ets_OFF) && (halScaler_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_OFF) &&
                                /*(halScaler_InputPortIsReady(eSOURCE_WINDOW_SUB, halScaler_BackupSecondaryInput_Get())) &&*/
                                (palImgMgr_BackupSwitchGet() == ets_ON))
                            {
                                if(eResult4 != halScaler_RxPortIsReady(eSOURCE_WINDOW_SUB, palImgMgr_BackupSecondaryInput_Get()))
                                {
                                    eResult4 = halScaler_RxPortIsReady(eSOURCE_WINDOW_SUB, palImgMgr_BackupSecondaryInput_Get());
                                    palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL); //appGui_SendUpdateOSDEvent();
                                }
                                if(eResult4 == eHAL_SCALER_EXEC_CODE_PASS)
                                {
                                    halScaler_MeasureCheck(eSOURCE_WINDOW_SUB);
                                    if(ucHDRStatus != halScaler_HDR_Get(eSOURCE_WINDOW_SUB))
                                    {
                                        ucHDRStatus = halScaler_HDR_Get(eSOURCE_WINDOW_SUB);
                                        palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL); //appGui_SendUpdateOSDEvent();
                                    }
                                }
                            }
                            m_sPalDataPathInfo.lPollPeriod = 100;

                            if(m_sPalDataPathInfo.uiPanelChange > PANEL_CHECK_COUNT*(1000/m_sPalDataPathInfo.lPollPeriod))
                            {
                                //大於10s timeout
                                //m_sPalDataPathInfo.lPollPeriod = 5的case下，數值修正
                                m_sPalDataPathInfo.uiPanelChange = PANEL_CHECK_COUNT*(1000/m_sPalDataPathInfo.lPollPeriod);
                            }

                            if(m_sPalDataPathInfo.uiPanelChange)
                            {
                                m_sPalDataPathInfo.uiPanelChange--;
                                if(m_sPalDataPathInfo.uiPanelChange == 0)
                                {
                                    LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) cPanelChange timout\r\n", __FUNCTION__, __LINE__);
                                    palImgMgr_RGBEnableItemSet(eHAL_SCALER_RGB_ITEM_PANELCHANGE); //A70LK_Casper_0008
                                    palImgMgr_EnableRGB();
                                }
                            }
                        }
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
                    LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) eDATA_PATH_MSG_SYNCSDETECTED\r\n", __FUNCTION__, __LINE__);
#if (CURRENT_MAIN_BOARD == ICHIP_BOARD)
                    palDataPath_DisplaySFG(eSOURCE_WINDOW_MAIN, 5, FALSE);        //A70LV_Doulas_0003 modify
#endif
                    palDataPath_GotoState(eDATA_PATH_STATE_ATTEMPT_LOCK);
                    break;

                case eDATA_PATH_MSG_RESYNC:     //A70LV_Doulas_0004
                    LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) eDATA_PATH_MSG_RESYNC\r\n", __FUNCTION__, __LINE__);
					palDataPath_GotoState(eDATA_PATH_STATE_SCALER_PORT_CONFIG);
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
                    LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) eDATA_PATH_MSG_PANEL_CHANGE\r\n", __FUNCTION__, __LINE__);
                    palDataPath_GotoState(eDATA_PATH_STATE_BEGIN_SCAN);
                    break;

                case eDATA_PATH_MSG_NONE:   //A70LV_Doulas_0007
             //       LOG_MSG(db_APP_DATAPATH, "\r\n");
#ifdef __ICHIP_CONTROL__
                    if(halScaler_MeasureInput(eSOURCE_WINDOW_MAIN) == eHAL_SCALER_EXEC_CODE_PASS)     //A70LV_Doulas_0031 modify
                    {
                        halScaler_DisplaySFG(eSOURCE_WINDOW_MAIN, 5, TRUE);
                        palDataPath_GotoState(eDATA_PATH_STATE_ATTEMPT_LOCK);
                        if(m_sSourceDesc.eConnector == eINPUT_SOURCE_VGA)    //A70LV_Doulas_0112
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

                            uiH_Active = halScaler_Input_H_Active_Get(eSOURCE_WINDOW_MAIN);
                            uiV_Active = halScaler_Input_V_Active_Get(eSOURCE_WINDOW_MAIN);
                            halScaler_InputVertRefresh2_Get(eSOURCE_WINDOW_MAIN,&uiV_Freq);
                            halFrontEndCtrl_VGA_TIMING_Set(uiH_Active,uiV_Active,uiV_Freq);
                        }
                    }
                    else
                    {
                        LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
                        palDataPath_GotoState(eDATA_PATH_STATE_BEGIN_SCAN);
                    }
#else
                    if(halScaler_MeasureInput(eSOURCE_WINDOW_MAIN, (UINT8)m_sSourceDesc.eConnector) == eHAL_SCALER_EXEC_CODE_PASS)
                    {
                        halScaler_AVIInfoFrame(eSOURCE_WINDOW_MAIN);
                        halScaler_InputFrameRateSetting(eSOURCE_WINDOW_MAIN);

                        palDataPath_GotoState(eDATA_PATH_STATE_ATTEMPT_LOCK);
                        palDataMgr_DisplayModeChecking();

                    }
                    else
                    {
                        LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
                        palDataPath_GotoState(eDATA_PATH_STATE_BEGIN_SCAN);
                    }
#endif /* __ICHIP_CONTROL__ */
                    break;

                default:


                   /* if(palDataPath_TimeElapsedInState() >= 3000)  //A70LV_Doulas_0004
                    {
                        palDataPath_GotoState(eDATA_PATH_STATE_BEGIN_SCAN);
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
            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d)%d : eDATA_PATH_STATE_ATTEMPT_LOCK --\r\n", __FUNCTION__, __LINE__,uiMsgID);

            switch(uiMsgID)
            {
                case eDATA_PATH_MSG_LOCKED:
                    LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) eDATA_PATH_MSG_LOCKED\r\n", __FUNCTION__, __LINE__);
                    //halScaler_ConfigureForDisplay(0, 1);//RGB Color Space   //A70LV_Doulas_0007
                    //palDataPath_DisplaySFG(0, 5, FALSE);    //A70LV_Doulas_0007
                    palDataPath_GotoState(eDATA_PATH_STATE_MONITOR_SOURCE);
                    break;

                case eDATA_PATH_MSG_LOSTLOCK:
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
                    LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) eDATA_PATH_MSG_RESYNC\r\n", __FUNCTION__, __LINE__);
					palDataPath_GotoState(eDATA_PATH_STATE_SCALER_PORT_CONFIG);
				break;

                case eDATA_PATH_MSG_PANEL_CHANGE:   //A70LV_Doulas_0005
                    LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) eDATA_PATH_MSG_PANEL_CHANGE\r\n", __FUNCTION__, __LINE__);
                    palDataPath_GotoState(eDATA_PATH_STATE_BEGIN_SCAN);
                    break;

                case eDATA_PATH_MSG_NONE:   //A70LV_Doulas_0007
                    LOG_MSG(db_APP_DATAPATH, "\r\n");
#ifdef __ICHIP_CONTROL__
                    if(halScaler_SYNC_Lock_Get(eSOURCE_WINDOW_MAIN) == eHAL_SCALER_EXEC_CODE_PASS) //A70LV_Doulas_0009 modify
                    {
                        if(halScaler_AutoPhaseStart(eSOURCE_WINDOW_MAIN) == eHAL_SCALER_EXEC_CODE_PASS)
                        {
                            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
                            palDataPath_GotoState(eDATA_PATH_STATE_AUTO_PHASE);
                        }
                        else
                        {

                            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
                            palDataPath_GotoState(eDATA_PATH_STATE_MONITOR_SOURCE);
                        }
                    }
                    else
                    {
                        palDataPath_GotoState(eDATA_PATH_STATE_BEGIN_SCAN);
                    }
#else
                    if(halScaler_SYNC_Lock_Get(eSOURCE_WINDOW_MAIN) == eHAL_SCALER_EXEC_CODE_PASS) //A70LV_Doulas_0009 modify
                    {
                        if((halScaler_Input_3D_Format_Config_Get() != eINPUT_3D_TYPE_OFF) &&
                           (halScaler_PIP_PBP_Enable_Get() == ets_OFF))
                        {
                            palImgMgr_SclDatapathSet(eSOURCE_MODE_SIGNLE);
                        }
                        else if((halScaler_PIP_PBP_Enable_Get() == ets_OFF) && (palImgMgr_BackupSwitchGet() != ets_ON) && ((m_sSourceDesc.eConnector == m_sSourceDesc.ucPrimaryInput) || (m_sSourceDesc.eConnector == m_sSourceDesc.ucSecondaryInput)))
                        {
                            if(halScaler_SclDatapathCompare(eSOURCE_MODE_SEAMLESS))
                            {
                                palImgMgr_SclDatapathSet(eSOURCE_MODE_SEAMLESS);
                            }
                        }

                        if((halScaler_SourceLock(eSOURCE_WINDOW_MAIN) == eHAL_SCALER_EXEC_CODE_PASS) &&
                           (halScaler_MeasureCheck(eSOURCE_WINDOW_MAIN) == eHAL_SCALER_EXEC_CODE_PASS))
                        {
                            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
                            palDataPath_GotoState(eDATA_PATH_STATE_MONITOR_SOURCE);
                        }
                    }
                    else
                    {
                        LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
                        palDataPath_GotoState(eDATA_PATH_STATE_BEGIN_SCAN);
                    }
#endif /* __ICHIP_CONTROL__ */
                    break;

                case eDATA_PATH_MSG_RECONFIG_IMAGE:     //A70LV_Doulas_0050
					LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) eDATA_PATH_MSG_RECONFIG_IMAGE\r\n", __FUNCTION__, __LINE__);
                    halScaler_DisplaySFG(eSOURCE_WINDOW_MAIN, 5, TRUE);
                    palDataPath_GotoState(eDATA_PATH_STATE_ATTEMPT_LOCK);
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
                    LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) eDATA_PATH_MSG_SYNCSDETECTED\r\n", __FUNCTION__, __LINE__);
#if (CURRENT_MAIN_BOARD == ICHIP_BOARD)
                    palDataPath_DisplaySFG(eSOURCE_WINDOW_MAIN, 5, FALSE);
#endif
                    palDataPath_GotoState(eDATA_PATH_STATE_ATTEMPT_LOCK);
                    break;

                case eDATA_PATH_MSG_RESYNC:
                    LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) eDATA_PATH_MSG_RESYNC\r\n", __FUNCTION__, __LINE__);
					palDataPath_GotoState(eDATA_PATH_STATE_SCALER_PORT_CONFIG);
				    break;

                case eDATA_PATH_MSG_PANEL_CHANGE:
                    LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) eDATA_PATH_MSG_PANEL_CHANGE\r\n", __FUNCTION__, __LINE__);
                    palDataPath_GotoState(eDATA_PATH_STATE_BEGIN_SCAN);
                    break;

                case eDATA_PATH_MSG_NONE:
                    if(halScaler_AutoPhase(eSOURCE_WINDOW_MAIN) == eHAL_SCALER_EXEC_CODE_PASS)
                    {
                        palDataPath_GotoState(eDATA_PATH_STATE_AUTO_POSITION);
                    }
                    break;

                case eDATA_PATH_MSG_LOSTLOCK:       //A70LV_Doulas_0032
                case eDATA_PATH_MSG_REACQUIRE:
                    LOG_MSG(db_APP_DATAPATH, "eDATA_PATH_MSG_LOSTLOCK12\r\n");
                    palDataPath_GotoState(eDATA_PATH_STATE_BEGIN_SCAN);
					break;

                case eDATA_PATH_MSG_RECONFIG_IMAGE:     //A70LV_Doulas_0050
					LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) eDATA_PATH_MSG_RECONFIG_IMAGE\r\n", __FUNCTION__, __LINE__);
                    halScaler_DisplaySFG(eSOURCE_WINDOW_MAIN, 5, TRUE);
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
                    LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) eDATA_PATH_MSG_SYNCSDETECTED\r\n", __FUNCTION__, __LINE__);
#if (CURRENT_MAIN_BOARD == ICHIP_BOARD)
                    palDataPath_DisplaySFG(eSOURCE_WINDOW_MAIN, 5, FALSE);
#endif
                    palDataPath_GotoState(eDATA_PATH_STATE_ATTEMPT_LOCK);
                    break;

                case eDATA_PATH_MSG_RESYNC:
                    LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) eDATA_PATH_MSG_RESYNC\r\n", __FUNCTION__, __LINE__);
                    palDataPath_GotoState(eDATA_PATH_STATE_SCALER_PORT_CONFIG);
               	    break;

                case eDATA_PATH_MSG_PANEL_CHANGE:
                    LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) eDATA_PATH_MSG_PANEL_CHANGE\r\n", __FUNCTION__, __LINE__);
                    palDataPath_GotoState(eDATA_PATH_STATE_BEGIN_SCAN);
                    break;

                case eDATA_PATH_MSG_NONE:
                    LOG_MSG(db_APP_DATAPATH, "\r\n");
                    if(halScaler_MeasureInput(eSOURCE_WINDOW_MAIN, (UINT8)m_sSourceDesc.eConnector) == eHAL_SCALER_EXEC_CODE_PASS)     //A70LV_Doulas_0031 modify
                    {
                        palDataPath_GotoState(eDATA_PATH_STATE_MONITOR_SOURCE);
                    }
                    else
                    {
                        palDataPath_GotoState(eDATA_PATH_STATE_BEGIN_SCAN);
                    }
                    break;

                case eDATA_PATH_MSG_LOSTLOCK:       //A70LV_Doulas_0032
                case eDATA_PATH_MSG_REACQUIRE:
                    LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) eDATA_PATH_MSG_LOSTLOCK13\r\n", __FUNCTION__, __LINE__);
                    palDataPath_GotoState(eDATA_PATH_STATE_BEGIN_SCAN);
					break;

                case eDATA_PATH_MSG_RECONFIG_IMAGE:     //A70LV_Doulas_0050
                    LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) eDATA_PATH_MSG_RECONFIG_IMAGE\r\n", __FUNCTION__, __LINE__);
                    halScaler_DisplaySFG(eSOURCE_WINDOW_MAIN, 5, TRUE);
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
                {
	    			static eHAL_SCALER_EXEC_CODE eMainResult = eHAL_SCALER_EXEC_CODE_NUMBER;
	    			static eHAL_SCALER_EXEC_CODE eSubResult = eHAL_SCALER_EXEC_CODE_NUMBER;
	    			DDP_SYSTEM_STATUS sDDPSystemStatus;
	    			eRESULT eMonitorResult = rcINVALID;
	    			eRESULT eTempResult = rcINVALID;
					BOOL bCW_Detect = FALSE; //T100_Casper_0089
	    			static UINT8 ucHDRStatus[2] = {0, 0};

                    halScaler_RxPortCheckPolling();

                    m_sPalDataPathInfo.ucSyncLockCount++;      //A70LV_Doulas_0009 Modify monitor functions
                    if(m_sPalDataPathInfo.ucSyncLockCount > 5)     //A70LV_Doulas_0164 Modify
                    {
                        BOOL bFreezeEn = FALSE;
                        m_sPalDataPathInfo.ucSyncLockCount = 0;
                        if((m_sSourceDesc.ucCurrentBackupInput == ets_ON) && (palImgMgr_BackupSwitchGet() == ets_ON))
                        {
                            eMonitorResult = halScaler_MeasureCheck(eSOURCE_WINDOW_SUB);
                            eSubResult = eMonitorResult;

                            eTempResult = halScaler_MeasureCheck(eSOURCE_WINDOW_MAIN);

                            if(eMainResult != eTempResult)
                            {
                                halScaler_AVIInfoFrame(eSOURCE_WINDOW_MAIN);
                                eMainResult = eTempResult;

                                if(eMainResult == eHAL_SCALER_EXEC_CODE_PASS)
                                {
                                    halScaler_ConfigureForDisplay(eSOURCE_WINDOW_MAIN, 1);
                                }
                                palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL); //appGui_SendUpdateOSDEvent();
                            }

                            if(eMonitorResult != eHAL_SCALER_EXEC_CODE_PASS)
                            {
                                //觸發backup，從sub source切回main source
                                if(m_sSourceDesc.ucAutoBackupInput == ets_ON)
                                {
                                    eMonitorResult = eMainResult;
                                }
#if 0
                                else if(eMonitorResult != eSubResult)
                                {
                                    //eResult = eSubResult;
                                }
#endif /* 0 */
                            }


                            halScaler_Freeze_Get(eSOURCE_WINDOW_SUB, &bFreezeEn);
                            if(bFreezeEn == TRUE)
                            {
                                eMonitorResult = eHAL_SCALER_EXEC_CODE_PASS;   //keep monitor
                            }
                        }
                        else
                        {

                            eMonitorResult = halScaler_MeasureCheck(eSOURCE_WINDOW_MAIN);
                            eMainResult = eMonitorResult;

                            if((palImgMgr_BackupSwitchGet() == ets_ON)  && ((m_sSourceDesc.eConnector == m_sSourceDesc.ucPrimaryInput) || (m_sSourceDesc.eConnector == m_sSourceDesc.ucSecondaryInput)))
                            {

                                eTempResult = halScaler_MeasureCheck(eSOURCE_WINDOW_SUB);
                                if(eSubResult != eTempResult)
                                {
                                    halScaler_AVIInfoFrame(eSOURCE_WINDOW_SUB);
                                    eSubResult = eTempResult;
                                    if(eSubResult == eHAL_SCALER_EXEC_CODE_PASS)
                                    {
                                        halScaler_ConfigureForDisplay(eSOURCE_WINDOW_SUB, 1);
                                    }
                                    palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL); //appGui_SendUpdateOSDEvent();
                                }

                                if(eMonitorResult != eHAL_SCALER_EXEC_CODE_PASS)
                                {
                                    if((m_sSourceDesc.ucAutoBackupInput == ets_ON) && (eMonitorResult != eHAL_SCALER_EXEC_CODE_HDR_CHANGE))
                                    {
                                        eMonitorResult = eSubResult;
                                    }
#if 0
                                    else if(eMonitorResult != eMainResult)
                                    {
                                        //eResult = eMainResult;
                                        //eMainResult = halScaler_MeasureCheck(eSOURCE_WINDOW_MAIN);
                                    }
#endif /* 0 */
                                }


                            }
                            halScaler_Freeze_Get(eSOURCE_WINDOW_MAIN, &bFreezeEn);     //A70LV_Doulas_0223
                            if(bFreezeEn == TRUE)
                            {
                                eMonitorResult = eHAL_SCALER_EXEC_CODE_PASS;   //keep monitor
                            }
                        }

                        if((palImgMgr_BackupSwitchGet() == ets_ON)  && ((m_sSourceDesc.eConnector == m_sSourceDesc.ucPrimaryInput) || (m_sSourceDesc.eConnector == m_sSourceDesc.ucSecondaryInput)))
                        {
                            if(eMainResult == eHAL_SCALER_EXEC_CODE_HDR_CHANGE)
                            {
                                if(ucHDRStatus[0] != halScaler_HDR_Get(eSOURCE_WINDOW_MAIN))
                                {
                                    ucHDRStatus[0] = halScaler_HDR_Get(eSOURCE_WINDOW_MAIN);
                                    palLANProcSendToLAN((UINT16)edcFIRST_INPUT_HDR_INFO);
                                    palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL); //appGui_SendUpdateOSDEvent();
                                }
                            }

                            if(eSubResult == eHAL_SCALER_EXEC_CODE_HDR_CHANGE)
                            {
                                if(ucHDRStatus[1] != halScaler_HDR_Get(eSOURCE_WINDOW_SUB))
                                {
                                    ucHDRStatus[1] = halScaler_HDR_Get(eSOURCE_WINDOW_SUB);
                                    palLANProcSendToLAN((UINT16)edcSECOND_INPUT_HDR_INFO);
                                    palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL); //appGui_SendUpdateOSDEvent();
                                }
                            }
                        }

                        if(m_sPalDataPathInfo.uiPanelChange)
                        {
                            UINT8 ucMasterProjectMode = 0;
                            UINT8 ucSlaveProjectMode = 0;
                            UINT8 ucXPRErrorStatus = 0;
                            UINT32 ulCWSpeed = 0, ulCWFrequence = 0;

                            m_sPalDataPathInfo.uiPanelChange--;

                            palFormatterMgr_CWIndexClockGet(&ulCWSpeed, &ulCWFrequence);

                            if(m_sPalDataPathInfo.uiPanelChange < 1)
                            {
                                if(!palDataMgr_IsOsdOpen())
                                {
                                    palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL); //appGui_SendUpdateOSDEvent();
                                }
                                palImgMgr_RGBEnableItemSet(eHAL_SCALER_RGB_ITEM_PANELCHANGE); //A70LK_Casper_0008
                                palImgMgr_EnableRGB();
                            }

                            if(m_sPalDataPathInfo.uiPanelChange == (PANEL_CHECK_COUNT - 1))
                            {
                                m_sPalDataPathInfo.bXPRErrorCheck = FALSE;
#if 0 //HICC2_Casper_0029
                                halFormatter_XPR_ErrorStatus_Get(&ucXPRErrorStatus);

                                LOG_MSG(db_APP_DATAPATH, "(%s, %d) XPRErrorStatus = %d\n", __FUNCTION__, __LINE__, ucXPRErrorStatus);

                                if(ucXPRErrorStatus)
                                {
                                    halFormatter_XPR_ErrorStatus_Set();
                                }
#endif
                            }

                            if(m_sPalDataPathInfo.uiPanelChange <= (PANEL_CHECK_COUNT - 1) && m_sPalDataPathInfo.uiPanelChange >= (PANEL_CHECK_COUNT/2))
                            {
                                m_sPalDataPathInfo.bXPRErrorCheck = FALSE;

                                if(palSystem_PanelID_Get() == PANEL_3D_OUTPUT)
                                {
                                    if((ulCWSpeed <= 121) && (ulCWSpeed >= 119))
                                    {
                                        m_sPalDataPathInfo.uiPanelChange = (PANEL_CHECK_COUNT/2) - 1;
                                    }
                                }
                                else
                                {
                                    if((ulCWSpeed <= 241) && (ulCWSpeed >= 239))
                                    {
                                        m_sPalDataPathInfo.uiPanelChange = (PANEL_CHECK_COUNT/2) - 1;
                                    }
                                }
                            }


                            if(m_sPalDataPathInfo.uiPanelChange < (PANEL_CHECK_COUNT/2))
                            {
                                palFormatterMgr_Projection_Mode_Get(&ucMasterProjectMode);
                                palFormatterMgr_SlaveProjection_Mode_Get(&ucSlaveProjectMode);

                                if((ucMasterProjectMode != eCMD_Formatter_External) || (ucSlaveProjectMode != eCMD_Formatter_External))
                                {
                                    halFormatter_Projection_Mode_Set(eCMD_Formatter_External);
                                }
                                else
                                {
                                    if(m_sPalDataPathInfo.uiPanelChange > 2)
                                    {
                                        m_sPalDataPathInfo.uiPanelChange = 2;
                                    }
                                }

                                if(!palDataMgr_IsOsdOpen())
                                {
                                    palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL); //appGui_SendUpdateOSDEvent();
                                }
                            }

#if 0
                            if(m_sPalDataPathInfo.uiPanelChange == 0)
                            {
                                palDataPath_LowLatencySetting();
                            }
#endif /* 0 */

                            LOG_MSG(db_APP_DATAPATH, "(%s, %d) ucMasterProjectMode = %d,  ucSlaveProjectMode = %d, Count = %d CW Speed (%d, %d)\n", __FUNCTION__, __LINE__, ucMasterProjectMode,  ucSlaveProjectMode, m_sPalDataPathInfo.uiPanelChange, ulCWSpeed, ulCWFrequence);
                        }
                        else if(m_sPalDataPathInfo.bXPRErrorCheck == TRUE)
                        {
                            UINT8 ucXPRErrorStatus = 0;
                            #if 0 //HICC2_Casper_0029
                            halFormatter_XPR_ErrorStatus_Get(&ucXPRErrorStatus);

                            if(ucXPRErrorStatus)
                            {
                                palImgMgr_DisableRGB(eHAL_SCALER_RGB_ITEM_XPR); //A70LK_Casper_0008
                                MS_SLEEP(50);
                                halFormatter_XPR_ErrorStatus_Set();
                                MS_SLEEP(50);
                                palImgMgr_RGBEnableItemSet(eHAL_SCALER_RGB_ITEM_XPR); //A70LK_Casper_0008
                                palImgMgr_EnableRGB();
                            }
                            #endif
                            m_sPalDataPathInfo.bXPRErrorCheck = FALSE;

                        }
                    }
                    else
                    {
                        //eResult = halScaler_SYNC_Lock_Get(0);   //measure H/V Sync
                        eMonitorResult = halScaler_RxPortIsReady(eSOURCE_WINDOW_MAIN, (UINT8)m_sSourceDesc.eConnector);
                        //LOG_MSG(db_ALWAYS, "Test %d %d %d %d\r\n", m_sSourceDesc.eConnector, m_sSourceDesc.ucAutoBackupInput, halScaler_BackupPrimaryInput_Get(), halScaler_BackupSecondaryInput_Get());

                        if((halScaler_PIP_PBP_Enable_Get() == ets_OFF) && (halScaler_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_OFF) &&
                            (palImgMgr_BackupSwitchGet() == ets_ON) && (palImgMgr_BackupSecondaryInput_Get() != m_sSourceDesc.eConnector))
                        {
                            eRESULT eResult2 = rcINVALID;
                            eResult2 = halScaler_RxPortIsReady(eSOURCE_WINDOW_SUB, palImgMgr_BackupSecondaryInput_Get());

                            if(eHAL_SCALER_EXEC_CODE_PASS == eResult2)
                            {
                                if(m_sSourceDesc.ucAutoBackupInput == ets_OFF)
                                {
                                    if(ePAL_IMG_EXEC_CODE_PASS == palImgMgr_BackupInputCheck(m_sSourceDesc.eConnector, palImgMgr_BackupSecondaryInput_Get()))
                                    {
                                        halScaler_BackupSizePresetSetting();
                                        palImgMgr_SclDatapathSet(eSOURCE_MODE_BACKUP);

                                        if(m_sSourceDesc.ucCurrentBackupInput == ets_OFF)
                                        {
                                            halScaler_AVIInfoFrame(eSOURCE_WINDOW_SUB);
                                            halScaler_InputSelectSet(eSOURCE_WINDOW_SUB, palImgMgr_BackupSecondaryInput_Get(), FALSE);
                                            halScaler_Monitor(eSOURCE_WINDOW_SUB);
                                        }
                                        else
                                        {
                                            halScaler_AVIInfoFrame(eSOURCE_WINDOW_MAIN);
                                            halScaler_InputSelectSet(eSOURCE_WINDOW_MAIN, m_sSourceDesc.eConnector, FALSE);
                                            halScaler_Monitor(eSOURCE_WINDOW_MAIN);
                                        }
                                        halScaler_BackupRxPortSet(eSOURCE_WINDOW_MAIN);
                                        m_sSourceDesc.ucAutoBackupInput = ets_ON;
                                        eMainResult = halScaler_MeasureInput(eSOURCE_WINDOW_MAIN, (UINT8)m_sSourceDesc.eConnector);
                                        eSubResult = halScaler_MeasureInput(eSOURCE_WINDOW_SUB, (UINT8)palImgMgr_BackupSecondaryInput_Get());
                                        //palDataMgr_Data_Access(edcBACKUPINPUT_STATUS, edaWRITE_THROUGH_WITH_ACTION, &m_sSourceDesc.ucAutoBackupInput);
                                        palDataMgr_Data_Access(edcBACKUP_INPUT_CHANGE, edaWRITE_THROUGH_WITH_ACTION, &m_sSourceDesc.ucCurrentBackupInput);
                                        palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL); //appGui_SendUpdateOSDEvent();
                                        LOG_MSG(db_HAL_BACKUPSOURCE, "eBackupConnector Enable\r\n");
                                    }
                                }
                                else
                                {
                                    UINT8 ucBackupSourceGet = halScaler_CurrentBackupSourceGet();

                                    if((m_sSourceDesc.ucCurrentBackupInput != ucBackupSourceGet) || (ePAL_IMG_EXEC_CODE_PASS != palImgMgr_BackupInputCheck(m_sSourceDesc.eConnector, palImgMgr_BackupSecondaryInput_Get())))
                                    {
                                        if(ucBackupSourceGet) //return 1 is sub source active
                                        {
                                            MS_SLEEP(500); //A70LK_Larry_0192
                                            halScaler_AVIInfoFrame(eSOURCE_WINDOW_SUB);
                                            halScaler_InputFrameRateSetting(eSOURCE_WINDOW_SUB); // ProAV_Rex_0041
                                            halScaler_BackupRxPortSet(eSOURCE_WINDOW_SUB);
                                            //halScaler_ConfigureForDisplay(eSOURCE_WINDOW_MAIN, 1);
                                        }
                                        else
                                        {
                                            palImgMgr_SclDatapathSet(eSOURCE_MODE_SIGNLE);
                                        }
                                        m_sSourceDesc.ucAutoBackupInput = ets_OFF;
                                        m_sSourceDesc.ucCurrentBackupInput = ucBackupSourceGet;
                                        //palDataMgr_Data_Access(edcBACKUPINPUT_STATUS, edaWRITE_THROUGH_WITH_ACTION, &m_sSourceDesc.ucAutoBackupInput);
                                        palDataMgr_Data_Access(edcBACKUP_INPUT_CHANGE, edaWRITE_THROUGH_WITH_ACTION, &m_sSourceDesc.ucCurrentBackupInput);
                                        LOG_MSG(db_HAL_BACKUPSOURCE, "Current BackupSource %d\r\n", m_sSourceDesc.ucCurrentBackupInput);
                                        m_sPalDataPathInfo.ucSyncLockCount = 0;
                                    }
                                }
                            }
                            else
                            {
                                if(m_sSourceDesc.ucAutoBackupInput == ets_ON)
                                {
                                    UINT8 ucBackupSourceGet = halScaler_CurrentBackupSourceGet();

                                    m_sSourceDesc.ucAutoBackupInput = ets_OFF;

                                    if(ets_OFF == ucBackupSourceGet) //return 0 is main source active
                                    {
                                        halScaler_AVIInfoFrame(eSOURCE_WINDOW_MAIN);
                                        halScaler_InputFrameRateSetting(eSOURCE_WINDOW_MAIN);
                                        halScaler_ConfigureForDisplay(eSOURCE_WINDOW_MAIN, 1);
                                    }
                                    m_sSourceDesc.ucCurrentBackupInput = ucBackupSourceGet;
                                    //palDataMgr_Data_Access(edcBACKUPINPUT_STATUS, edaWRITE_THROUGH_WITH_ACTION, &m_sSourceDesc.ucAutoBackupInput);
                                    palDataMgr_Data_Access(edcBACKUP_INPUT_CHANGE, edaWRITE_THROUGH_WITH_ACTION, &m_sSourceDesc.ucCurrentBackupInput);
                                    palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL);
                                    palImgMgr_SclDatapathSet(eSOURCE_MODE_SIGNLE);
                                    LOG_MSG(db_HAL_BACKUPSOURCE, "eBackupConnector Disable 2\r\n");
                                    m_sPalDataPathInfo.ucSyncLockCount = 0;
                                }
                            }

                            if(m_sSourceDesc.ucCurrentBackupInput == ets_ON)
                            {
                                if(eMainResult != eMonitorResult)
                                {
                                    eMainResult = eMonitorResult;
                                    if(eMonitorResult == eHAL_SCALER_EXEC_CODE_PASS)
                                    {
                                        halScaler_ConfigureForDisplay(eSOURCE_WINDOW_MAIN, 1);
                                    }
                                    palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL); //appGui_SendUpdateOSDEvent();
                                }
                                eMonitorResult = eResult2;
                            }
                            else
                            {
                                if(eSubResult != eResult2)
                                {
                                    eSubResult = eResult2;
                                    if(eResult2 == eHAL_SCALER_EXEC_CODE_PASS)
                                    {
                                        halScaler_ConfigureForDisplay(eSOURCE_WINDOW_SUB, 1);
                                    }
                                    palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL); //appGui_SendUpdateOSDEvent();
                                }
                            }
                            //palDataMgr_Data_Access(edcBACKUPINPUT_STATUS, edaWRITE_THROUGH_NO_ACTION, &m_sSourceDesc.ucAutoBackupInput); //trigger send LAN data, write function is null, so don't care write data
                         }

                        if(eMonitorResult == eHAL_SCALER_EXEC_CODE_PASS)   //A70LV_Doulas_0283
                        {
                            if((halScaler_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_DAUL_PIPE) || (halScaler_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_4K3D_DAUL_PIPE))
                            {
                                if(eHAL_SCALER_EXEC_CODE_PASS == halScaler_RxPortIsReady(eSOURCE_WINDOW_SUB, palImgMgr_BackupSecondaryInput_Get()))
                                {
                                    if((m_sPalDataPathInfo.ucSyncLockCount % 5) == 0) //A70LK_Larry_0133
                                    {
                                    halScaler_Monitor(eSOURCE_WINDOW_SUB);
                                    }
                                    if(m_sSourceDesc.ucCurrentBackupInput == ets_OFF)
                                    {
                                        halScaler_3DEnable_Set(halScaler_3DEnable_Get());
                                        if(palImgMgr_ColorSpace_Get(eSOURCE_WINDOW_MAIN) == eCM_COLOR_SPACE_AUTO)
                                        {
                                            palImgMgr_AVIInfoFrame(eSOURCE_WINDOW_SUB);
                                        }
                                        else
                                        {
                                            palImgMgr_ConfigureColorSpace(eSOURCE_WINDOW_MAIN, TRUE);
                                        }
                                        halScaler_ScreenOff(eSOURCE_WINDOW_SUB, FALSE);
                                        halScaler_MeasureInput(eSOURCE_WINDOW_SUB, (UINT8)palImgMgr_BackupSecondaryInput_Get());
                                        halScaler_InputFrameRateSetting(eSOURCE_WINDOW_SUB);
                                    }
                                    m_sSourceDesc.ucCurrentBackupInput = ets_ON;
                                }
                                else
                                {
                                    if(m_sSourceDesc.ucCurrentBackupInput == ets_ON) //A70LK_Larry_0133
                                    {
                                        halScaler_ScreenOff(eSOURCE_WINDOW_SUB, TRUE);
                                    }
                                    m_sSourceDesc.ucCurrentBackupInput = ets_OFF;
                                }
                            }
                            //else if(halScaler_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_FRAME_SEQUENTIAL)
                            {
							    //external 3d sync det.
                                BOOL b3DSyncIn = halScaler_AutoExt_3D_SyncSetting();


                                if(b3DSyncIn != m_sPalDataPathInfo.b3DSyncIn)
                                {
                                    m_sPalDataPathInfo.b3DSyncIn = b3DSyncIn;
                                    palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL); //appGui_SendUpdateOSDEvent();
                                }

                            }
                        }
                    }

                    if(eMonitorResult == eHAL_SCALER_EXEC_CODE_PASS)
                    {
                        if(m_sPalDataPathInfo.ucSyncLockCount == 0)
                        {
                            UINT8 ucValue = 0;
                            palDataMgr_Data_Access(edcIMAGE_FREEZE, edaREAD, &ucValue);     //A70LV_Doulas_0167
                            //if(ucValue && appGui_CurrentMenuIndexGet() != POWER_OFF_MENU_ICOUNT)// BruceLin#20201218, POWER_OFF_STANDALONEMENU_ICOUNT //ZU860_Energy_0016
                            if(ucValue)     //Move to appgui.c
            				{
                                palDataMgr_UI_EventSend(edcUI_EVENT_OPEN_FREEZE_MENU_MSG, TRUE, NULL);
                            }
                        }
                    }
                    else
                    {
                        //BOOL bFreezeEn = FALSE;
                        //halScaler_Freeze_Get(0,&bFreezeEn);
                        //if(bFreezeEn == FALSE)
                        palEnvironment_LightSourceBlankingOn_Set(TRUE,5);   //A70LV_Doulas_0294
                        palDataPath_FreezeChecking(eSOURCE_WINDOW_MAIN);   //A70LV_Doulas_0223 Freeze check
                        {
                            uiMsgID = eDATA_PATH_MSG_LOSTLOCK;
                            halScaler_DisplaySFG(eSOURCE_WINDOW_MAIN, 5, TRUE);
                            palDataPath_GotoState(eDATA_PATH_STATE_SOURCE_LOST);
                        }
                    }
                }
                break;

                case eDATA_PATH_MSG_LOSTLOCK:
				{
    				BOOL bCW_Detect = TRUE; //T100_Casper_0089

					//halMCU_CW_Detect_Event_Setting((UINT8*)&bCW_Detect, 1); //T100_Casper_0089
					//ucCW_Status = 1;
                    palEnvironment_LightSourceBlankingOn_Set(TRUE,5);   //A70LV_Doulas_0294
                    halScaler_DisplaySFG(eSOURCE_WINDOW_MAIN, 5, TRUE);       //A70LV_Doulas_0003 modify
                    palDataPath_DisplaySplash(TRUE);
                    //halScaler_FrontEndScanMode_Clear(eSOURCE_WINDOW_MAIN);       //A70LV_Doulas_0175

                    LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
					palDataPath_GotoState(eDATA_PATH_STATE_BEGIN_SCAN);   //A70LV_Doulas_0004
					halWarping_WarpingCheckFlagSet(TRUE);  //need to re-apply warping
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
              	}
                    break;

                case eDATA_PATH_MSG_REACQUIRE:
                    LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) eDATA_PATH_MSG_REACQUIRE\r\n", __FUNCTION__, __LINE__);
                    palDataPath_DisplaySplash(FALSE);
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
                    LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) eDATA_PATH_MSG_RESYNC\r\n", __FUNCTION__, __LINE__);
                    halScaler_InputSelectSet(eSOURCE_WINDOW_MAIN, m_sSourceDesc.eConnector, TRUE);
                    palDataPath_GotoState(eDATA_PATH_STATE_SCALER_PORT_CONFIG);
                    halWarping_WarpingCheckFlagSet(TRUE);  //need to re-apply warping

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
                    LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) eDATA_PATH_MSG_SUSPENDSCAN\r\n", __FUNCTION__, __LINE__);
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
                    LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) eDATA_PATH_MSG_PANEL_CHANGE\r\n", __FUNCTION__, __LINE__);
                    palDataPath_GotoState(eDATA_PATH_STATE_BEGIN_SCAN);
                    break;

                case eDATA_PATH_MSG_RECONFIG_IMAGE:     //A70LV_Doulas_0009
                    LOG_MSG(db_APP_DATAPATH, " eDATA_PATH_MSG_RECONFIG_IMAGE\r\n");
                    halScaler_DisplaySFG(eSOURCE_WINDOW_MAIN, 5, TRUE);
                    palDataPath_GotoState(eDATA_PATH_STATE_ATTEMPT_LOCK);
                    break;

            }
			break;

        case eDATA_PATH_STATE_SOURCE_LOST:
            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
            palDataPath_GotoState(eDATA_PATH_STATE_BEGIN_SCAN);
            break;

        default:
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
                LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : eDATA_PATH_MSG_SETCONNECTOR %d\r\n", __FUNCTION__, __LINE__, ulParam1);//A70LV_Doulas_0112
                m_sSourceDesc.eConnector = (eCM_SOURCE_ID)ulParam1;         //A70LV_Doulas_0029 modify
                palImgMgr_ScreenOff(eSOURCE_WINDOW_MAIN, TRUE);
                palImgMgr_SclVopWindowOffSet(eSOURCE_WINDOW_MAIN, TRUE);
                palLANProcSendToLAN(edcBACKUPINPUT_CURRENT_SOURCE);   //A70LK_Nina_0022
                palDataMgr_BeforeDisplayModeToDefault();
                halScaler_SetInputSource(eSOURCE_WINDOW_MAIN, m_sSourceDesc.eConnector);
                if(halScaler_PIP_PBP_Enable_Get() == ets_OFF)
                {
                    halScaler_3DEnable_Set(halScaler_3DEnable_Get());
                }

                if(m_sSourceDesc.eConnector != ucOldMainSource)
                {
                    UINT8 ucAspectRatio = 0;

                    palDataMgr_Data_Access(edcSIZE_PRESETS, edaREAD, (void*)&ucAspectRatio);
                    if(ucAspectRatio != eCM_SCALING_MODE_CUSTOM)
                        palDataMgr_ResetDigitalZoomShift();
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
            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : eDATA_PATH_MSG_RESYNC\r\n", __FUNCTION__, __LINE__);
            halScaler_Resync_Init(eSOURCE_WINDOW_MAIN);    //A70LV_Doulas_0004
            //halScaler_FrontEndScanMode_Clear(eSOURCE_WINDOW_MAIN);       //A70LV_Doulas_0175
            palDataPath_StateMachineProc(uiMsgID);
            break;

        case eDATA_PATH_MSG_RECONFIG_IMAGE:     //A70LV_Doulas_0009
            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : eDATA_PATH_MSG_RECONFIG_IMAGE\r\n", __FUNCTION__, __LINE__);
      //      halC789Ctrl_OutputEnableSet(0);     //A70LV_Doulas_0079 disable C789 output
            palDataPath_StateMachineProc(uiMsgID);
            break;

        case eDATA_PATH_MSG_TEST_PATTERN:        //A70LV_Doulas_0030
            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : eDATA_PATH_MSG_TEST_PATTERN\r\n", __FUNCTION__, __LINE__);
            palDataPath_GotoState(eDATA_PATH_STATE_TPG_DISPLAYED);
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

        case eDATA_PATH_MSG_BACKUP_PRIMARY_CONNECTOR:
            m_sSourceDesc.ucPrimaryInput = (UINT8)ulParam1;
            m_sSourceDesc.ucAutoBackupInput = 0;
            m_sSourceDesc.ucCurrentBackupInput = 0;
            if((m_sSourceDesc.eConnector != m_sSourceDesc.ucPrimaryInput) && (m_sSourceDesc.eConnector != m_sSourceDesc.ucSecondaryInput))
            {
                halScaler_BackupPrimaryInput_Set(m_sSourceDesc.eConnector);
                halScaler_BackupSecondaryInput_Set(m_sSourceDesc.eConnector);
            }
		    break;

        case eDATA_PATH_MSG_BACKUP_SECONDARY_CONNECTOR:
            m_sSourceDesc.ucSecondaryInput = (UINT8)ulParam1;
            m_sSourceDesc.ucAutoBackupInput = 0;
            m_sSourceDesc.ucCurrentBackupInput = 0;
            if((m_sSourceDesc.eConnector != m_sSourceDesc.ucPrimaryInput) && (m_sSourceDesc.eConnector != m_sSourceDesc.ucSecondaryInput))
            {
                halScaler_BackupPrimaryInput_Set(m_sSourceDesc.eConnector);
                halScaler_BackupSecondaryInput_Set(m_sSourceDesc.eConnector);
            }
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

    uPALIMGMGR_INFO uInfo = {.sPowerStandby.ucCH = eSOURCE_WINDOW_MAIN};
    palImgMgr_PowerStandby(&uInfo);

    ucData = HDMI_OUT_DISABLE;
    palFrontEndMgr_HDMI_OUT_Set(&ucData);
    palFrontEndCtrl_FocusResync();
    palDataPath_ProcMutexDataScalerInfoInit();  //H2PF_Simon_0033

    return eEXEC_CODE_PASS;
}



eEXEC_CODE palDataPath_PowerNormal(ePANEL_ID ePanelId)
{
    UINT32 temp;
    UINT8 ucData = 0;

    LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);

    m_sPalDataPathInfo.bDataPathReady = FALSE;

    m_sPalDataPathInfo.ePanelTimingId  = ePanelId;
#if (CUSTOMIZED_EDID_ENABLE == TRUE)
    palDataPath_CustomizedEDID_Init();    //A35G2_Simon_0091
#endif
    uc3D_Panel_Timer = 0;
    m_sPalDataPathInfo.b3DSyncIn = FALSE;
    m_sPalDataPathInfo.uiPanelChange = 0;
    if(ePanelId == PANEL_3D_OUTPUT)
    {
        m_sPalDataPathInfo.ucWarpingAPConfig = eWAPRING_AP_GROUP_3D;
    }
    else
    {
        m_sPalDataPathInfo.ucWarpingAPConfig = eWAPRING_AP_GROUP_2D;
    }
    m_sPalDataPathInfo.ucSourceEnhanceCheck = 0;

    halScaler_PowerNormal(eSOURCE_WINDOW_MAIN, m_sPalDataPathInfo.ePanelTimingId);
    m_ucSmoothOnReadyCheck = 0;
    palGeo_Palette_Init();
    palGeo_PowerNormal(m_sPalDataPathInfo.ePanelTimingId);
#if(defined(CUSTOM_OPTOMA))
    palDataMgr_Data_Access(edcBACKGROUND_COLOR, edaREAD, &ucData);
    palDataMgr_Data_Access(edcBACKGROUND_COLOR, edaWRITE_RAM_ONLY_WITH_ACTION, &ucData);
#else
    palDataMgr_Data_Access(edcSPLASH_STARTUP, edaREAD, &ucData);
    palDataMgr_Data_Access(edcSPLASH_STARTUP, edaWRITE_RAM_ONLY_WITH_ACTION, &ucData);
#endif
    palDataMgr_UI_EventSend(edcUI_EVENT_PANEL_CHANGE, ePanelId, NULL);

    palDataMgr_UI_EventSend(edcUI_EVENT_INITIAL, 1, NULL);

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
    halScaler_Init(eSOURCE_WINDOW_MAIN);//A70LV_Doulas_0003 //Must put before palDataPath_PowerStandby()
    //palDataPath_PowerStandby(); //H30K_Doulas_0071
    palDataPath_MailBox_Send(m_sPalDataPathInfo.xMsgQueue, m_sPalDataPathInfo.xEventGroupHandle,
                    eDATA_PATH_MSG_GOTOSTATE, -1, eDATA_PATH_STATE_SUSPENDED, &temp, FALSE); //H30K_Doulas_0071
    uPALIMGMGR_INFO uInfo = {.sPowerStandby.ucCH = eSOURCE_WINDOW_MAIN}; //H30K_Doulas_0071
    palImgMgr_PowerStandby(&uInfo); //H30K_Doulas_0071
    palFrontEndCtrl_FocusResync(); //H30K_Doulas_0071
    palDataPath_ProcMutexDataScalerInfoInit(); //H30K_Doulas_0071


    //Create Thread Pasue
    if(pthread_mutex_init(&m_sPalDataPathInfo.xTaskMutex, NULL) != 0)
    {
        LOG_MSG(db_ALWAYS, "(func:%s, line:%d): Create xTaskMutex Fail!\r\n", __FUNCTION__, __LINE__);
    }

    m_sPalDataPathInfo.bTaskPause = FALSE;

    //Create Thread
    pthread_attr_init(&m_sPalDataPathInfo.xTaskHandle_attr);
    pthread_attr_setstacksize(&m_sPalDataPathInfo.xTaskHandle_attr, APP_DATAPATH_STACK_SIZE);
    INT16 iCreateThreadError = 0 ;
    iCreateThreadError = pthread_create(&m_sPalDataPathInfo.xTaskHandle, &m_sPalDataPathInfo.xTaskHandle_attr, palDataPath_Task, (void*) NULL);

    if(iCreateThreadError != 0)
    {
        LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d): Create Thread Fail!\r\n", __FUNCTION__, __LINE__);
        return eEXEC_CODE_FAIL;
    }

    palGeo_Initial();  //H2PF_Simon_0038
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
    m_sPalDataPathInfo.b3DSyncIn = FALSE;

    palGeo_Initial();
//#endif

    //palDataPath_PowerStandby(); //H30K_Doulas_0071
    palDataPath_MailBox_Send(m_sPalDataPathInfo.xMsgQueue, m_sPalDataPathInfo.xEventGroupHandle,
                    eDATA_PATH_MSG_GOTOSTATE, -1, eDATA_PATH_STATE_SUSPENDED, &temp, FALSE); //H30K_Doulas_0071
    uPALIMGMGR_INFO uInfo = {.sPowerStandby.ucCH = eSOURCE_WINDOW_MAIN}; //H30K_Doulas_0071
    palImgMgr_PowerStandby(&uInfo); //H30K_Doulas_0071
    palFrontEndCtrl_FocusResync(); //H30K_Doulas_0071
    palDataPath_ProcMutexDataScalerInfoInit(); //H30K_Doulas_0071


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
    uPALIMGMGR_INFO uInfo = {.sPowerStandby.ucCH = eSOURCE_WINDOW_MAIN}; //H30K_Doulas_0071
    palImgMgr_PowerStandby(&uInfo); //H30K_Doulas_0071
    palFrontEndCtrl_FocusResync(); //H30K_Doulas_0071
    palDataPath_ProcMutexDataScalerInfoInit(); //H30K_Doulas_0071


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

void palDataPath_ResyncCheckEnhance(void) //A70LK_Larry_0169
{
    UINT32 temp;

    palEnvironment_AutoSourceSearchTimerClear();

    m_sPalDataPathInfo.ucSourceEnhanceCheck = 1;

    palDataPath_MailBox_Send(m_sPalDataPathInfo.xMsgQueue,
                    m_sPalDataPathInfo.xEventGroupHandle,
                    eDATA_PATH_MSG_RESYNC, -1, 0, &temp, FALSE);
}


eEXEC_CODE palDataPath_Panel_Change(ePANEL_ID ePanelId)   //A70LV_Doulas_0005
{
    UINT32 temp;
    m_sPalDataPathInfo.ePanelTimingId = ePanelId;
    palEnvironment_AutoSourceSearchTimerClear();
    palDataPath_MailBox_Send(m_sPalDataPathInfo.xMsgQueue,
                    m_sPalDataPathInfo.xEventGroupHandle,
                    eDATA_PATH_MSG_PANEL_CHANGE, -1, 0, &temp, FALSE);
    return eEXEC_CODE_PASS;
}

eEXEC_CODE palDataPath_InitInputSource(void)   //A70LV_Doulas_0029 modify //A70LV_Doulas_0007
{
    INT32 iVAl;
    palDataMgr_Data_Access(edcMAIN_INPUT, edaREAD, &iVAl);
    m_sSourceDesc.eConnector = (eCM_SOURCE_ID)iVAl;

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

    eScalerResult = halScaler_DigitalHorzZoom_Set(uwDigital_Horz_Zoom);     //A70LV_Doulas_0020
    if(eScalerResult == eHAL_SCALER_EXEC_CODE_PASS)
    {
        //palDataPath_Reconfig_Image();
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

    eScalerResult = halScaler_DigitalVertZoom_Set(uwDigital_Vert_Zoom);     //A70LV_Doulas_0020
    if(eScalerResult == eHAL_SCALER_EXEC_CODE_PASS)
    {
        //palDataPath_Reconfig_Image();
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
    			halFormatter_DB_SetBorderCfg(0,0,960,1080);	//A70LK_Doulas_0006
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

    //halScaler_Resync_Init(eSOURCE_WINDOW_MAIN);   //A70LV_Doulas_0120
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

eEXEC_CODE palDataPath_TestPatternEntry(void)
{
    UINT32 temp;
    eEXEC_CODE eExecResult = eEXEC_CODE_PASS;

    if(palDataPath_MailBox_Send(m_sPalDataPathInfo.xMsgQueue,
                       m_sPalDataPathInfo.xEventGroupHandle,
                       eDATA_PATH_MSG_TEST_PATTERN, -1, 0, &temp, FALSE) != eMAIL_BOX_EXEC_CODE_PASS )
    {
        eExecResult = eEXEC_CODE_FAIL;
        LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) fail\r\n", __FUNCTION__, __LINE__);
    }

    return eExecResult ;
}

eEXEC_CODE palDataPath_BackupPrimaryInput(eCM_SOURCE_ID eInputSource)
{
    UINT32 temp;
    eEXEC_CODE eExecResult = eEXEC_CODE_PASS;

    LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : eInputSource = %d\r\n", __FUNCTION__, __LINE__, eInputSource);
    if(palDataPath_MailBox_Send(m_sPalDataPathInfo.xMsgQueue,
                       m_sPalDataPathInfo.xEventGroupHandle,
                       eDATA_PATH_MSG_BACKUP_PRIMARY_CONNECTOR, -1, eInputSource, &temp, FALSE) != eMAIL_BOX_EXEC_CODE_PASS )
    {
        eExecResult = eEXEC_CODE_FAIL;
        LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) fail\r\n", __FUNCTION__, __LINE__);
    }

    return eExecResult ;
}

eEXEC_CODE palDataPath_BackupSecondaryInput(eCM_SOURCE_ID eInputSource)
{
    UINT32 temp;
    eEXEC_CODE eExecResult = eEXEC_CODE_PASS;

    LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) eInputSource = %d\r\n", __FUNCTION__, __LINE__, eInputSource);
    if(palDataPath_MailBox_Send(m_sPalDataPathInfo.xMsgQueue,
                       m_sPalDataPathInfo.xEventGroupHandle,
                       eDATA_PATH_MSG_BACKUP_SECONDARY_CONNECTOR, -1, eInputSource, &temp, FALSE) != eMAIL_BOX_EXEC_CODE_PASS )
    {
        eExecResult = eEXEC_CODE_FAIL;
        LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) fail\r\n", __FUNCTION__, __LINE__);
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
       //(m_sPalDataPathInfo.eDataPathState == eDATA_PATH_STATE_LOOK_FOR_SYNCS) ||
       (m_sPalDataPathInfo.eDataPathState == eDATA_PATH_STATE_ATTEMPT_LOCK) ||
       (m_sPalDataPathInfo.eDataPathState == eDATA_PATH_STATE_AUTO_PHASE) ||
       (m_sPalDataPathInfo.eDataPathState == eDATA_PATH_STATE_AUTO_POSITION) ||
       (m_sPalDataPathInfo.bTestPatternTrigger == TRUE)
       )    //A70LV_Doulas_0050
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
    if((m_sPalDataPathInfo.eDataPathState == eDATA_PATH_STATE_MONITOR_SOURCE) && (!m_sPalDataPathInfo.uiPanelChange))
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
    if(palDataPath_IsSourceMonitorSub() == FALSE)    //A70LV_Doulas_0310
    {
        halScaler_SclVopWindowOffSet(eSOURCE_WINDOW_MAIN, TRUE);
        palDataPath_AutoDisableMenuTransparency();   //A70LV_Doulas_0122 auto dsiable Menu Transparency
    }

    #if 1       //A70LV_Doulas_0230 Modify
    if(palDataMgr_CurTestPatternGet() != eTID_OFF)
    {
        UINT8 ucService_TP = eCM_TEST_PATTERN_OFF;
        ucService_TP = eCM_TEST_PATTERN_OFF;
        palDataMgr_Data_Access(edcSERVICE_TEST_PATTERN, edaWRITE_THROUGH_NO_ACTION, (PUINT8)&ucService_TP);
    }
    #endif

    if(halScaler_PIP_PBP_Enable_Get() != eCM_SCREEN_MODE_OFF)
    {
        palDataMgr_UI_EventSend(edcUI_EVENT_SPLASH, 0, NULL);
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
            palImgMgr_SclDatapathSet(eSOURCE_MODE_SIGNLE);
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
    UINT32 temp = 0;
    UINT8 cCount = 0;
    //eHAL_SCALER_EXEC_CODE eScalerResult = eHAL_SCALER_EXEC_CODE_FAIL;
    eEXEC_CODE eExecResult = eEXEC_CODE_PASS;

    #ifdef CUSTOM_CHRISTIE
    if((uwTestPattern == eCM_TEST_PATTERN_OFF) && (appGui_HSG_TestPatternEnableGet()))
    {
        palDataMgr_Data_Access(edcHSG_TEST_PATTERN_CTRL, edaREAD, (void*)&uwTestPattern);
    }
    #endif

    #if 1   //A70LV_Doulas_0157 modify
    appGuiStyle_CurrentTestPatternSet(uwTestPattern);
    if(uwTestPattern == eCM_TEST_PATTERN_OFF)
    {
        halScaler_RxPortCheckPolling();
        appGui_TestPattern(uwTestPattern);
        if(palDataPath_IsTestPatternEnable() == TRUE)
        {
            #ifdef CUSTOM_CHRISTIE
            if((m_DataPathLastState == eDATA_PATH_STATE_MONITOR_SOURCE) &&
                (halScaler_RxPortIsReady(eSOURCE_WINDOW_MAIN, (UINT8)m_sSourceDesc.eConnector) == eHAL_SCALER_EXEC_CODE_PASS))
            {
                //palDataPath_TurnOffSplash();      //ZU860_Doulas_0058 remove

                if((eCM_TEST_PATTERN_OFF == uwTestPattern) && appGui_OSD_ON_Get()==ets_OFF )
                {
                    //appGui_CloseMenu();
                    //appGui_OpenMenuwithUpdateOSDEvent(MAIN_MENU_ICOUNT);
                    appGui_SendSplashEvent(0);
                    appGui_SendForceCloseMenu();
                }
                else
                {
                    appGui_SendUpdateOSDEvent();        //G100_Owen_0030
                }
#if 0
                if(m_sSourceDesc.eConnector == eCM_SOURCE_VGA)    //ZU860_Doulas_0083
                {
                    palEnvironment_LightSourceBlankingOn_Set(TRUE,1);       //LD off
                }
#endif /* 0 */

                if(palDataPath_MailBox_Send(m_sPalDataPathInfo.xMsgQueue,
                   m_sPalDataPathInfo.xEventGroupHandle,
                   eDATA_PATH_MSG_GOTOSTATE, -1, eDATA_PATH_STATE_LOOK_FOR_SYNCS, &temp, FALSE) != eMAIL_BOX_EXEC_CODE_PASS )     //A70LV_Doulas_0293 modify
                {
                    eExecResult = eEXEC_CODE_FAIL;
                }
            }
            else
            {
                //palDataPath_ShowLogo();
                if(!palDataMgr_IsOsdOpen() && appGui_OSD_ON_Get()==ets_ON)
                {
                    appGui_SendSubSourceInfoMessage();
                }
                else if(appGui_OSD_ON_Get()==ets_OFF && eCM_TEST_PATTERN_OFF==uwTestPattern)
                {
                    appGui_SendSplashEvent(1);
                    appGui_SendForceCloseMenu();
                }
                else
                {
                    appGui_SendUpdateOSDEvent(); //A70LK_Jacky_0021
                }

                if(palDataPath_MailBox_Send(m_sPalDataPathInfo.xMsgQueue,
                       m_sPalDataPathInfo.xEventGroupHandle,
                       eDATA_PATH_MSG_GOTOSTATE, -1, eDATA_PATH_STATE_SCALER_PORT_CONFIG, &temp, FALSE) != eMAIL_BOX_EXEC_CODE_PASS )
                {
                    eExecResult = eEXEC_CODE_FAIL;
                }
            }
            #else
            {
                //palDataPath_TurnOffSplash();      //ZU860_Doulas_0058 remove
                appGui_SendUpdateOSDEvent();        //G100_Owen_0030
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
            #endif
        }
    }
    else if(palDataPath_MailBox_Send(m_sPalDataPathInfo.xMsgQueue,
                       m_sPalDataPathInfo.xEventGroupHandle,
                       eDATA_PATH_MSG_TEST_PATTERN, -1, (UINT32)uwTestPattern, &temp, FALSE) == eMAIL_BOX_EXEC_CODE_PASS )      //A70LV_Doulas_0296 Modify Test pattern bug
    {
        #ifdef __ICHIP_CONTROL__
        #ifdef CUSTOM_BARCO //A35G2_BRC_Casper_0051
        if(utilWarp_GetOsdPatternType() != PAT_TYPE__OFF) //G100_Owen_0112
        {
            halAdvWarpShowOsdPattern(PAT_TYPE__OFF);
        }
        #endif
        #endif
#if 1

        UINT8 ucLowLatency = 0;

        palDataMgr_Data_Access(edcLOW_LATENCY_MODE, edaREAD, &ucLowLatency);
        if(ucLowLatency != eCM_LOW_LATENCY_MODE_OFF)
        {
#ifdef ON_BOARD
            if(Board_SingleBoard_Get() == FALSE)
            {
                UINT8 cMode = 0;
                halScaler_FrameSyncModeGet(&cMode);
                palImgMgr_DisableRGB(eHAL_SCALER_RGB_ITEM_LATENCY); //A70LK_Casper_0005 //A70LK_Casper_0008
                if(cMode != 0)
                {
                    MS_SLEEP(50); //A70LK_Casper_0005
                    //off
                    halScaler_FrameSyncModeSet(eSOURCE_WINDOW_MAIN, 0); // for frame sync test
                    //MS_SLEEP(50);
                    halWarping_WarpDownScalingIssueWorkAround();
                }
                halScaler_RGBEnableItemSet(eHAL_SCALER_RGB_ITEM_LATENCY); //A70LK_Casper_0008
                utilCounter_Reg_TimerEvent(eTIMER_EVENT_BLACKING_DISABLE_COUNT, 350, 0, 0, halScaler_EnableRGB, NULL); //A70LK_Casper_0005
                if(m_sPalDataPathInfo.uiPanelChange) //A70LK_Casper_0008
                {
                    halScaler_RGBEnableItemSet(eHAL_SCALER_RGB_ITEM_PANELCHANGE);
                    halScaler_EnableRGB();
                }
            }
#endif /* ON_BOARD */
        }
#endif /* 0 */


        palDataPath_DisplaySplash(TRUE);
        appGui_SendUpdateOSDEvent();
        m_sSourceDesc.eActiveDisplay = eDATA_PATH_ACTIVE_DISPLAY_TESTPATTERN;
//        halFormatter_DMD_BGDisplayModeSet(BG_DISABLE);  //G100_Owen_0008 remove //ZU860_Doulas_0077 //DDP background off

        for(cCount = 0; cCount < 200; cCount++) //R70K_Larry_0023//wait monitor source configure
        {
            if(palDataPath_IsTestPatternEnable() == FALSE)
            {
                MS_SLEEP(10);
            }
            else
            {
                break;
            }
        }
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
            appGui_SendUpdateOSDEvent();
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
            appGui_SendUpdateOSDEvent();
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
    UINT8 cCount = 0;
    UINT32 temp;
    eEXEC_CODE eExecResult = eEXEC_CODE_PASS;

    #ifdef CUSTOM_CHRISTIE
    if((ucTestPattern == eCM_TEST_PATTERN_OFF) && (appGui_HSG_TestPatternEnableGet()))
    {
        palDataMgr_Data_Access(edcHSG_TEST_PATTERN_CTRL, edaREAD, (void*)&ucTestPattern);
    }
    #endif

    #if 1   //A70LV_Doulas_0157 modify
    appGuiStyle_CurrentTestPatternSet(ucTestPattern);
    if(ucTestPattern == eCM_TEST_PATTERN_OFF)
    {
        halScaler_RxPortCheckPolling();
        appGui_TestPattern(ucTestPattern);
        if(palDataPath_IsTestPatternEnable() == TRUE)
        {
            #ifdef CUSTOM_CHRISTIE
            if((m_DataPathLastState != eDATA_PATH_STATE_MONITOR_SOURCE))
            {
                if((!palDataMgr_IsOsdOpen()) && (!utilChristie_UC_Data_Cmd_Flag_Get()) && (!utilChristie_UC_Data_OSD_Cmd_Flag_Get()))
                {
                    appGui_SendSubSourceInfoMessage();
                }
                else
                {
                    appGui_SendUpdateOSDEvent();        //G100_Owen_0030
                }

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
#ifdef __ICHIP_CONTROL__
        #if defined(CUSTOM_BARCO) //A35G2_BRC_Casper_0051 //A35G2_Coda_0103 //A35G2_Coda_0116
        if(utilWarp_GetOsdPatternType() != PAT_TYPE__OFF) //G100_Owen_0112
        {
            halAdvWarpShowOsdPattern(PAT_TYPE__OFF);
        }
        #endif
#endif
        palDataPath_DisplaySplash(TRUE);
        appGui_SendUpdateOSDEvent();
        m_sSourceDesc.eActiveDisplay = eDATA_PATH_ACTIVE_DISPLAY_TESTPATTERN;
        halFormatter_DMD_BGDisplayModeSet(BG_DISABLE);  //ZU860_Doulas_0094

        for(cCount = 0; cCount < 200; cCount++) //R70K_Larry_0023//wait monitor source configure
        {
            if(palDataPath_IsTestPatternEnable() == FALSE)
            {
                MS_SLEEP(10);
            }
            else
            {
                break;
            }
        }
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
            appGui_SendUpdateOSDEvent();
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
    if(m_sPalDataPathInfo.eDataPathState == eDATA_PATH_STATE_TPG_DISPLAYED)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

eEXEC_CODE palDataPath_Formatter_Gamma_Set(UINT8 ucGamma)   //A70LV_Doulas_0038
{
    eEXEC_CODE eExecResult = eEXEC_CODE_PASS;
    UINT8 ucPictureMode;

    ucPictureMode = halScaler_PictureSettings_Get_NotReturnUser((UINT8)eSOURCE_WINDOW_MAIN);    //A70LV_Doulas_0073 Add

    if(ucPictureMode == eCM_PICTURE_SETTINGS_BLENDING)
    {
        //halFormatter_IMG_GammaLutSet(DDP442X_GAMMA_2P2);
        ucGamma = palDataMgr_sDDP_Gamma_Transfer(eCM_GAMMA_2_2);
    }

    ucGamma = palDataMgr_sDDP_Gamma_Transfer(ucGamma);
    halFormatter_IMG_GammaLutSet(ucGamma);
#if 0
    switch(ucGamma)
    {
    	case eCM_GAMMA_HDR_STANDARD:	//A70LK_Doulas_0004
    		halFormatter_IMG_GammaLutSet(DDP442X_GAMMA_2P2);
    		break;

        case eCM_GAMMA_VIDEO:	//A70LK_Doulas_0004 Modify
#if 0//#ifdef CUSTOM_CHRISTIE	//HICC2_Doulas_0100//A70Gen2_Doulas_0045 Add
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

        case eCM_GAMMA_FILM:	//A70LK_Doulas_0004 Modify
            halFormatter_IMG_GammaLutSet(DDP442X_GAMMA_FILM);
            break;

        //case eCM_GAMMA_2_0:
            ///halFormatter_IMG_GammaLutSet(DDP442X_GAMMA_BRIGHT_2P2);
            //break;

        case eCM_GAMMA_CRT:		//A70LK_Doulas_0004 Modify
         	halFormatter_IMG_GammaLutSet(DDP442X_GAMMA_CRT); //HICC2_Doulas_0100
            break;

        case eCM_GAMMA_2_2:
            halFormatter_IMG_GammaLutSet(DDP442X_GAMMA_2P2);
            break;

        case eCM_GAMMA_DICOM:	//A70LK_Doulas_0004 Modify
            halFormatter_IMG_GammaLutSet(DDP442X_GAMMA_DICOM); //HICC2_Doulas_0100
            break;

        case eCM_GAMMA_2_4:
            halFormatter_IMG_GammaLutSet(DDP442X_GAMMA_2P4);
            break;

        default:
            halFormatter_IMG_GammaLutSet(DDP442X_GAMMA_2P2); //HICC2_Doulas_0100
            break;
    }
#endif /* 0 */

    return eExecResult;
}

BOOL palDataPath_InputSourceIs3DFormat_Get(void)    //A70LV_Doulas_0154 modify
{
    BOOL bValue = FALSE;
    switch(halScaler_Input_3D_Format_Get())
    {
        case eINPUT_3D_TYPE_OFF:
            bValue = FALSE;
            break;

        default:
            bValue = TRUE;
            break;
    }
    return bValue;
}

eEXEC_CODE palDataPath_InputPixelClock_Get(UINT8 *ucValue)   //A70LV_Doulas_0056
{
    eEXEC_CODE eExecResult = eEXEC_CODE_PASS;

    if(palDataPath_IsSourceLock())
    {
        //if(m_FrontEndVideoFormat.u8VideoDownScaling)        //A70LV_Doulas_0187 modify
        //{
            //char aucString[VERSION_STRING_MAX_LENGTH];
            //DOUBLE dbVal;
            //dbVal = (DOUBLE)m_FrontEndVideoTiming.u32VideoPCLK;
            //sprintf(aucString, "%d.%03dMHz\0",(UINT32)dbVal/1000,(UINT32)dbVal%1000);
            //memcpy(ucValue, aucString, strlen(aucString)+1);
        //}
        //else
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

    if(palDataPath_IsSourceLock())
    {
        halScaler_InputSignalFormat_Get(eSOURCE_WINDOW_MAIN,ucValue);
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
        //if(m_FrontEndVideoFormat.u8VideoDownScaling)        //A70LV_Doulas_0187 modify
        //{
            //char aucString[VERSION_STRING_MAX_LENGTH];
            //sprintf(aucString, "%d x %d\0",m_FrontEndVideoTiming.u16VideoHActive ,m_FrontEndVideoTiming.u16VideoVActive);
            //memcpy(ucValue, aucString, strlen(aucString)+1);
        //}
        //else
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
        //if(m_FrontEndVideoFormat.u8VideoDownScaling)        //A70LV_Doulas_0187 modify
        //{
            //char aucString[VERSION_STRING_MAX_LENGTH];
            //DOUBLE dbVal;
            //dbVal = (DOUBLE)m_FrontEndVideoTiming.u32VideoPCLK *1000 / (DOUBLE)m_FrontEndVideoTiming.u16VideoHTotal;
            //sprintf(aucString, "%d.%03dkHz\0",(UINT32)dbVal/1000,(UINT32)dbVal%1000);
            //memcpy(ucValue, aucString, strlen(aucString)+1);
        //}
        //else
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
        //if(m_FrontEndVideoFormat.u8VideoDownScaling)        //A70LV_Doulas_0187 modify
        //{
            //char aucString[VERSION_STRING_MAX_LENGTH];
            //DOUBLE dbVal;
            //dbVal = (DOUBLE)m_FrontEndVideoTiming.u32VideoPCLK *1000 / (DOUBLE)m_FrontEndVideoTiming.u16VideoHTotal / (DOUBLE)m_FrontEndVideoTiming.u16VideoVTotal * 100;
            //sprintf(aucString, "%d.%02dHz\0",(UINT32)dbVal/100,(UINT32)dbVal%100);
            //memcpy(ucValue, aucString, strlen(aucString)+1);
        //}
        //else
        //{
            halScaler_InputVertRefresh_Get(eSOURCE_WINDOW_MAIN, ucValue);
        //}
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
#if 0
        if(m_sSourceDesc.eConnector == eINPUT_SOURCE_VGA)
        {
            //check sync "Sync on Green" or "Separate"
            if(m_FrontEndVideoFormat.u8VideoVGASyncType == eVGA_SYNC_TYPE_SOG)  //A70LV_Doulas_0109 modify
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
        else
        {
            sprintf(aucString, "Separate\0");
            memcpy(ucValue, aucString, strlen(aucString)+1);
        }
#else
        sprintf(aucString, "Separate\0");
        memcpy(ucValue, aucString, strlen(aucString)+1);
#endif /* 0 */


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

BOOL palDataPath_Main_Or_Sub_SourceMonitor(void)       //A70LV_Doulas_0101
{
    if(palDataPath_IsSourceLock() == TRUE)
    {
        return TRUE;
    }
    else
    {
        if(halScaler_PIP_PBP_Enable_Get() != eCM_SCREEN_MODE_OFF)    //PIP on
        {
            if(palDataPath_IsSourceLockSub() == TRUE)
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
    //appGui_PanelChange(palSystem_PanelID_Get());        //A70LV_Doulas_0226 change OSD V-start

    return eEXEC_CODE_PASS;
}

eEXEC_CODE palDataPath_AutoDisableMenuTransparency(void)   //A70LV_Doulas_0122 auto dsiable Menu Transparency
{
    if(palDataPath_OSD_MenuTransparencyEnableGet() == TRUE) //((GuiCb.fpGui_OSD_MenuTransparencyEnableGetCb() == TRUE)
    {
        palDataPath_MenuTransparencyEnableSet(FALSE); //GuiCb.fpGui_OSD_MenuTransparencyEnableSetCb(FALSE);
    }
    return eEXEC_CODE_PASS;
}

UINT8 palDataPath_Input3D_FormatGet(void)   //A70LV_Doulas_0154
{
    UINT8 ucRx = halScaler_CurrentSourceRxPortGet(eSOURCE_WINDOW_MAIN, m_sSourceDesc.eConnector);

    if(ucRx > PROA_RX_4)
    {
        return 0;
    }

    return halScaler_3DModeGet(ucRx);
}

UINT8 palDataPath_PanelOutputGet(void)
{
    UINT8 ucRx = halScaler_CurrentSourceRxPortGet(eSOURCE_WINDOW_MAIN, m_sSourceDesc.eConnector);
    UINT8 uc3DMode = halScaler_3DEnable_Get();
    UINT8 ucPictureMode = 0;
    UINT8 ucPanelOutput = PANEL_2D_OUTPUT;
    BOOL bActuatorEnable = TRUE;
    BOOL bHighFrameEnable = FALSE;

    ucPictureMode = halScaler_PictureSettings_Get_NotReturnUser(eSOURCE_WINDOW_MAIN);
    LOG_MSG(db_APP_DATAPATH, "(%s, %d)ucPictureMode = %d\n", __FUNCTION__, __LINE__, ucPictureMode);
    switch(uc3DMode)
    {
        case eCM_3D_FORMAT_AUTO:
            switch(halScaler_3DModeGet(ucRx))
            {
                case eVIDEO_3D_FORMAT_FP:
                    ucPanelOutput = PANEL_3D_OUTPUT;
                    break;

                case eVIDEO_3D_FORMAT_SS:
                    ucPanelOutput = PANEL_3D_OUTPUT;
                    break;

                case eVIDEO_3D_FORMAT_TB:
                    ucPanelOutput = PANEL_3D_OUTPUT;
                    break;

                default:
                    bHighFrameEnable = TRUE;
                    ucPanelOutput = PANEL_2D_OUTPUT;
                    break;
            }
            break;

        case eCM_3D_FORMAT_FRAME_PACKING:
            ucPanelOutput = PANEL_3D_OUTPUT;
            break;

        case eCM_3D_FORMAT_SIDE_BY_SIDE:
            ucPanelOutput = PANEL_3D_OUTPUT;
            break;

        case eCM_3D_FORMAT_TOP_AND_BOTTOM:
            ucPanelOutput = PANEL_3D_OUTPUT;
            break;

        case eCM_3D_FORMAT_DUALPIPE:
        case eCM_3D_FORMAT_DUALPIPE_3D:
            ucPanelOutput = PANEL_3D_OUTPUT;
            break;

        case eCM_3D_FORMAT_FRAME_SEQUENTIAL:
            ucPanelOutput = PANEL_3D_OUTPUT;
            break;

        case eCM_3D_FORMAT_4K3D:
        case eCM_3D_FORMAT_4K3D_DUALPIPE:
            ucPanelOutput = PANEL_2D_OUTPUT;
            break;

        default:
            bHighFrameEnable = TRUE;
            ucPanelOutput = PANEL_2D_OUTPUT;
            break;
    }

    if((bHighFrameEnable == TRUE) && (halScaler_InputPortVertRateGet(0) > 95)  && (halScaler_PIP_PBP_Enable_Get() == eCM_SCREEN_MODE_OFF)) //HICC2_Jacky_0005
    {
        ucPanelOutput = PANEL_2D_HIGHSPEED;
    }

    return ucPanelOutput;
}

void palDataPath_Input3D_PanelSet(void)       //A70LV_Doulas_0159
{
    UINT8 ucRx = halScaler_CurrentSourceRxPortGet(eSOURCE_WINDOW_MAIN, m_sSourceDesc.eConnector);
    UINT8 ucInput3D_ConfigOld = halScaler_Input_3D_Format_Config_Get();
	BOOL  bCW_Detect = TRUE; //T100_Casper_0089
    UINT8 uc3DMode = halScaler_3DEnable_Get();
    UINT8 uc3DEnable = TRUE;
    UINT8 ucPictureMode = 0;
    UINT8 ucPanelOutput = PANEL_2D_OUTPUT;
    UINT8 ucWarpingPanelOutput = ePANEL_ID_LAST;
    BOOL bActuatorEnable = TRUE;

    if(ucRx > PROA_RX_4)
    {
        return;
    }

    ucPictureMode = halScaler_PictureSettings_Get_NotReturnUser(eSOURCE_WINDOW_MAIN);
    if(halScaler_PIP_PBP_Enable_Get() == eCM_SCREEN_MODE_OFF)
    {
        ucPanelOutput = palDataPath_PanelOutputGet();
        ucWarpingPanelOutput = ucPanelOutput;
        switch(uc3DMode)
        {
            case eCM_3D_FORMAT_AUTO:
                switch(halScaler_3DModeGet(ucRx))
                {
                    case eVIDEO_3D_FORMAT_FP:
                        m_sPalDataPathInfo.ucWarpingAPConfig = eWAPRING_AP_GROUP_3D;
                        bActuatorEnable = FALSE;
                        halScaler_Input_3D_Format_Set(eINPUT_3D_TYPE_FRAMEPACKING);
                        LOG_MSG(db_HAL_SOURCE_INFO, "3D_FORMAT_FP\r\n");
                        break;

                    case eVIDEO_3D_FORMAT_SS:
                        m_sPalDataPathInfo.ucWarpingAPConfig = eWAPRING_AP_GROUP_3D;
                        bActuatorEnable = FALSE;
                        halScaler_Input_3D_Format_Set(eINPUT_3D_TYPE_SIDEBYSIDE);
                        LOG_MSG(db_HAL_SOURCE_INFO, "3D_FORMAT_SS\r\n");
                        break;

                    case eVIDEO_3D_FORMAT_TB:
                        m_sPalDataPathInfo.ucWarpingAPConfig = eWAPRING_AP_GROUP_3D;
                        bActuatorEnable = FALSE;
                        halScaler_Input_3D_Format_Set(eINPUT_3D_TYPE_TOPANDBOTTOM);
                        LOG_MSG(db_HAL_SOURCE_INFO, "3D_FORMAT_TB\r\n");
                        break;

                    default:// eVIDEO_3D_FORMAT_2D
                        m_sPalDataPathInfo.ucWarpingAPConfig = eWAPRING_AP_GROUP_2D;
                        uc3DEnable = FALSE;
                        halScaler_Input_3D_Format_Set(eINPUT_3D_TYPE_OFF);
                        LOG_MSG(db_HAL_SOURCE_INFO, "3D_OFF\r\n");
                        break;
                }
                break;

            case eCM_3D_FORMAT_FRAME_PACKING:
                m_sPalDataPathInfo.ucWarpingAPConfig = eWAPRING_AP_GROUP_3D;
                bActuatorEnable = FALSE;
                halScaler_Input_3D_Format_Set(eINPUT_3D_TYPE_FRAMEPACKING);
                LOG_MSG(db_HAL_SOURCE_INFO, "3D_FORMAT_FP\r\n");
                break;

            case eCM_3D_FORMAT_SIDE_BY_SIDE:
                m_sPalDataPathInfo.ucWarpingAPConfig = eWAPRING_AP_GROUP_3D;
                bActuatorEnable = FALSE;
                halScaler_Input_3D_Format_Set(eINPUT_3D_TYPE_SIDEBYSIDE);
                LOG_MSG(db_HAL_SOURCE_INFO, "3D_FORMAT_SS\r\n");
                break;

            case eCM_3D_FORMAT_TOP_AND_BOTTOM:
                m_sPalDataPathInfo.ucWarpingAPConfig = eWAPRING_AP_GROUP_3D;
                bActuatorEnable = FALSE;
                halScaler_Input_3D_Format_Set(eINPUT_3D_TYPE_TOPANDBOTTOM);
                LOG_MSG(db_HAL_SOURCE_INFO, "3D_FORMAT_TB\r\n");
                break;

            case eCM_3D_FORMAT_DUALPIPE:
            case eCM_3D_FORMAT_DUALPIPE_3D:
                m_sPalDataPathInfo.ucWarpingAPConfig = eWAPRING_AP_GROUP_3D;
                bActuatorEnable = FALSE;
                halScaler_Input_3D_Format_Set(eINPUT_3D_TYPE_DAUL_PIPE);
                LOG_MSG(db_HAL_SOURCE_INFO, "3D_FORMAT_DAUL_PIPE\r\n");
                break;

            case eCM_3D_FORMAT_FRAME_SEQUENTIAL:
                m_sPalDataPathInfo.ucWarpingAPConfig = eWAPRING_AP_GROUP_3D;
                bActuatorEnable = FALSE;
                halScaler_Input_3D_Format_Set(eINPUT_3D_TYPE_FRAME_SEQUENTIAL);
                LOG_MSG(db_HAL_SOURCE_INFO, "3D_FORMAT_FRAME_SEQUENTIAL\r\n");
                break;

            case eCM_3D_FORMAT_4K3D:
                m_sPalDataPathInfo.ucWarpingAPConfig = eWAPRING_AP_GROUP_4K3D;
                bActuatorEnable = FALSE;
                halScaler_Input_3D_Format_Set(eINPUT_3D_TYPE_4K3D_SEQUENTIAL);
                LOG_MSG(db_HAL_SOURCE_INFO, "3D_FORMAT_4K3D\r\n");
                break;

            case eCM_3D_FORMAT_4K3D_DUALPIPE:
                m_sPalDataPathInfo.ucWarpingAPConfig = eWAPRING_AP_GROUP_4K3D_DUALPIPE;
                bActuatorEnable = FALSE;
                ucWarpingPanelOutput = ePANEL_ID_1920x2400_60HZ;
                halScaler_Input_3D_Format_Set(eINPUT_3D_TYPE_4K3D_DAUL_PIPE);
                 LOG_MSG(db_HAL_SOURCE_INFO, "3D_FORMAT_4K3D_DUALPIPE\r\n");
                break;

            default:// eCM_3D_FORMAT_OFF
                m_sPalDataPathInfo.ucWarpingAPConfig = eWAPRING_AP_GROUP_2D;
                uc3DEnable = FALSE;
                bActuatorEnable = TRUE;
                halScaler_Input_3D_Format_Set(eINPUT_3D_TYPE_OFF);
                break;
        }
    }
    else
    {
        m_sPalDataPathInfo.ucWarpingAPConfig = eWAPRING_AP_GROUP_2D;
        ucPanelOutput = PANEL_2D_OUTPUT;
        ucWarpingPanelOutput = ucPanelOutput;
        uc3DEnable = FALSE;
        bActuatorEnable = TRUE;
        halScaler_Input_3D_Format_Set(eINPUT_3D_TYPE_OFF);
    }

    if(uc3DMode == eCM_3D_FORMAT_4K3D || uc3DMode == eCM_3D_FORMAT_4K3D_DUALPIPE)
    {
        UINT8 ucGeometryEnable = 0;

        palDataMgr_Data_Access(edcWARP_TOGGLE, edaREAD, (void*)&ucGeometryEnable);

        if(ucGeometryEnable == 0)
        {
            ucGeometryEnable = 1;
            palDataMgr_Data_Access(edcWARP_TOGGLE, edaWRITE_THROUGH_WITH_ACTION, (void*)&ucGeometryEnable);
        }
    }

    if(ucPanelOutput == PANEL_2D_HIGHSPEED)
    {
        bActuatorEnable = FALSE;
        m_sPalDataPathInfo.ucWarpingAPConfig = eWAPRING_AP_GROUP_HIGH_SPEED;
    }

    if(m_sPalDataPathInfo.ePanelTimingId == PANEL_3D_OUTPUT)
    {
        if((ucPanelOutput != PANEL_3D_OUTPUT) &&
           (halScaler_Input_3D_Format_Config_Get() != ucInput3D_ConfigOld))
        {
            palDataPath_ResyncSub();
            halScaler_3D_InputPort_Set();
        }
    }

    LOG_MSG(db_APP_DATAPATH, "(%s, %d)Cuurent Panel %d, ucPanelOutput %d, WarpPanel %d, WarpSet %d, APConfig %d\n", __FUNCTION__, __LINE__, m_sPalDataPathInfo.ePanelTimingId, ucPanelOutput, halWarping_PanelGet(), ucWarpingPanelOutput, m_sPalDataPathInfo.ucWarpingAPConfig);

    if((ucPanelOutput == PANEL_3D_OUTPUT) &&
       (halScaler_PIP_PBP_Enable_Get() == eCM_SCREEN_MODE_OFF) &&
       (m_sPalDataPathInfo.ePanelTimingId != PANEL_3D_OUTPUT))
    {
        if(Board_SingleBoard_Get() == FALSE)
        {
            UINT8 cMode = 0;
            halScaler_FrameSyncModeGet(&cMode);
            if(cMode != 0)
            {
                //off
                halScaler_FrameSyncModeSet(eSOURCE_WINDOW_MAIN, 0); // for frame sync test
                MS_SLEEP(50);
            }
        }

        #if 0 //H2 wait review
        if((palDataMgr_IsOsdOpen()) && (appGui_Is_AC_Completed_Menu() || appGui_Is_ACU_Completed_Menu()))
        {
            appGui_Send_OSD_Exit();
        }
        #endif

        sSYSTEM_CHANGEPANELID_INFO sInfo = {bActuatorEnable, TRUE};

        palDataPath_Panel_ChangeCheck(sInfo.bBlicking);
        palDataMgr_Formatter_Parameter_Set(TRUE);

        palSystem_ChangePanelID(PANEL_3D_OUTPUT, &sInfo);
        palDataPath_SourcePathSetting(FALSE);
        //MS_SLEEP(200);
        //appGui_SendUpdateOSDEvent();
        //halScaler_Input_2D_SyncOut_Enable(0);
        LOG_MSG(db_APP_DATAPATH, "(%s, %d)\n", __FUNCTION__, __LINE__);
    }
    else if((uc3DEnable == FALSE) &&
            (halScaler_InputPortVertRateGet(0) > 95) &&
            (halScaler_PIP_PBP_Enable_Get() == eCM_SCREEN_MODE_OFF) &&
            (m_sPalDataPathInfo.ePanelTimingId != PANEL_2D_HIGHSPEED))
    {
        //if(ucPanelOutput == PANEL_3D_OUTPUT)
        {
            if(Board_SingleBoard_Get() == FALSE)
            {
                UINT8 cMode = 0;
                halScaler_FrameSyncModeGet(&cMode);
                if(cMode != 0)
                {
                    //off
                    halScaler_FrameSyncModeSet(eSOURCE_WINDOW_MAIN, 0); // for frame sync test
                    MS_SLEEP(50);
                }
            }

            #if 0 //H2 wait review
            if((palDataMgr_IsOsdOpen()) && (appGui_Is_AC_Completed_Menu() || appGui_Is_ACU_Completed_Menu()))
            {
                appGui_Send_OSD_Exit();
            }
            #endif

            sSYSTEM_CHANGEPANELID_INFO sInfo = {FALSE, TRUE};

            palDataPath_Panel_ChangeCheck(sInfo.bBlicking);

            palDataMgr_Formatter_Parameter_Set(TRUE);
            //m_sPalDataPathInfo.ucWarpingAPConfig = eWAPRING_AP_GROUP_HIGH_SPEED;

            palSystem_ChangePanelID(PANEL_2D_HIGHSPEED, &sInfo);
            palDataPath_SourcePathSetting(FALSE);
            LOG_MSG(db_APP_DATAPATH, "(%s, %d)\n", __FUNCTION__, __LINE__);
        }
    }
    else if((ucPanelOutput == PANEL_2D_OUTPUT) &&
            ((m_sPalDataPathInfo.ePanelTimingId != PANEL_2D_OUTPUT) ||
             (palSystem_ActuatorEnable_Get() != bActuatorEnable) ||
             (halWarping_PanelGet() != ucWarpingPanelOutput)))
    {
        UINT8 ucLowLatency = eCM_LOW_LATENCY_MODE_OFF;

        if(palDataMgr_BeforeDisplayModeGet() != eCM_PICTURE_SETTINGS_2DHIGHSPEED)
        {
            if(Board_SingleBoard_Get() == FALSE)
            {
                UINT8 cMode = 0;
                halScaler_FrameSyncModeGet(&cMode);
                if(cMode != 0)
                {
                    //off
                    halScaler_FrameSyncModeSet(eSOURCE_WINDOW_MAIN, 0); // for frame sync test
                    MS_SLEEP(50);
                }
            }
            sSYSTEM_CHANGEPANELID_INFO sInfo = {bActuatorEnable, TRUE};

            palDataPath_Panel_ChangeCheck(sInfo.bBlicking);

            palDataMgr_Formatter_Parameter_Set(TRUE);

            palSystem_ChangePanelID(PANEL_2D_OUTPUT, &sInfo);
            palDataPath_SourcePathSetting(FALSE);
        }

        palDataMgr_Data_Access(edcLOW_LATENCY_MODE, edaREAD, &ucLowLatency);

        if(ucLowLatency != eCM_LOW_LATENCY_MODE_OFF)
        {
            ucLowLatency = eCM_LOW_LATENCY_MODE_OFF;
            palDataMgr_Data_Access(edcLOW_LATENCY_MODE, edaWRITE_THROUGH_NO_ACTION, &ucLowLatency);
        }

        LOG_MSG(db_APP_DATAPATH, "(%s, %d)\n", __FUNCTION__, __LINE__);
    }

    if((palDataMgr_DataCode_Control(edcBACKUPINPUT_AUTOSWITCH) != eFUNC_CONTROL_ENABLE) &&
       ((uc3DEnable == TRUE) || (m_sPalDataPathInfo.ePanelTimingId == PANEL_2D_HIGHSPEED)))
    {
        UINT8 ucValue = ets_OFF;
        palDataMgr_Data_Access(edcBACKUPINPUT_AUTOSWITCH, edaREAD, &ucValue);

        if(ucValue == ets_ON)
        {
            ucValue = ets_OFF;
            palImgMgr_BackupSwitchSet(ucValue);
            //palDataMgr_Data_Access(edcBACKUPINPUT_AUTOSWITCH, edaWRITE_THROUGH_WITH_ACTION, &ucValue);
        }
    }
    else
    {
        if(halScaler_PIP_PBP_Enable_Get() == eCM_SCREEN_MODE_OFF)
        {
            UINT8 ucValue = ets_OFF;

            palDataMgr_Data_Access(edcBACKUPINPUT_AUTOSWITCH, edaREAD, &ucValue);

            if(ucValue == ets_ON)
            {
                palImgMgr_BackupSwitchSet(ucValue);
            }
        }
    }

    if(!palDataMgr_IsOsdOpen())
    {
        palDataMgr_UI_EventSend(edcUI_EVENT_SUB_SOURCE_INFOR_MSG, TRUE, NULL); //appGui_SendSubSourceInfoMessage();
    }
    else
    {
        palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL); //appGui_SendUpdateOSDEvent();
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

eCM_SOURCE_ID palDataPath_CurrentBackupInputGet(void)
{
    eCM_SOURCE_ID eInput = eCM_SOURCE_HDMI1;

    if((palDataPath_IsSourceMonitor()) && (palImgMgr_BackupSwitchGet() == ets_ON))
    {
        if(m_sSourceDesc.ucCurrentBackupInput == ets_OFF)
        {
            eInput = m_sSourceDesc.eConnector;
        }
        else
        {
            eInput = palImgMgr_BackupSecondaryInput_Get();
        }
    }
    else
    {
	    //Cassper_ProAV
        eInput = m_sSourceDesc.eConnector; //eCM_SOURCE_NUMBER;
	    //Cassper_ProAV
    }
    return eInput ;
}

eEXEC_CODE palDataPath_LogoCapture(void)
{
    UINT8 ucFreezeEn = 0;
    eEXEC_CODE eExecResult = eEXEC_CODE_PASS;

    palDataMgr_Data_Access(edcIMAGE_FREEZE, edaREAD, &ucFreezeEn);

    //halGui_OSD_Off();

#if 1
    halScaler_ConfigurePanelSize(PANEL_2D_OUTPUT);

    MS_SLEEP(50);

    if(!ucFreezeEn)
    {
        halScaler_ImageFreeze_Set(1);
    }

    halScaler_LogoDisplayEn(FALSE);
    halScaler_LogoCaptureSet();

    if(!ucFreezeEn)
    {
        halScaler_ImageFreeze_Set(0);
    }

    eExecResult = (eEXEC_CODE)halScaler_LogoCapturePanel(PANEL_2D_OUTPUT);
#endif /* 0 */

#if 1
    halScaler_ConfigurePanelSize(PANEL_3D_OUTPUT);

    MS_SLEEP(50);

    if(!ucFreezeEn)
    {
        halScaler_ImageFreeze_Set(1);
    }

    halScaler_LogoDisplayEn(FALSE);
    halScaler_LogoCaptureSet();

    if(!ucFreezeEn)
    {
        halScaler_ImageFreeze_Set(0);
    }

    eExecResult = (eEXEC_CODE)halScaler_LogoCapturePanel(PANEL_3D_OUTPUT);
#endif /* 0 */

    halScaler_ConfigurePanelSize(PANEL_2D_OUTPUT);


    return eExecResult;
}

UINT8 palDataPath_AlreadyShowDisconnectTwistMsg_Flag_Get(void) //ZU860_Doulas_0138  //A70LK_Simon_0018
{
    return m_ucAlreadyShowDisconnectTwistMsg;
}

void palDataPath_AlreadyShowDisconnectTwistMsg_Flag_Set(UINT8 ucValue) //ZU860_Doulas_0138  //A70LK_Simon_0018
{
    m_ucAlreadyShowDisconnectTwistMsg = ucValue;
}

BOOL palDataPath_IsErrorInputGetForTwistOn(void)    //ZU860_Doulas_0138  //A70LK_Simon_0018
{
    LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) %d %d %d (%d %d) (%d)\r\n", __FUNCTION__, __LINE__
                                                                       , halScaler_Input_3D_Format_Config_Get()
                                                                       , m_sPalDataPathInfo.ePanelTimingId
                                                                       , halWarping_PanelGet()
                                                                       , palSystem_PanelID_Get()
                                                                       , palDataPath_PanelOutputGet()
                                                                       ,m_sPalDataPathInfo.ucWarpingAPConfig);

    ePANEL_ID eCurPanelID = palSystem_PanelID_Get();
    ePANEL_ID eNewPanelID = palDataPath_PanelOutputGet();

    if(((eCurPanelID == PANEL_2D_HIGHSPEED) && (eNewPanelID != PANEL_2D_HIGHSPEED)) ||
       ((eCurPanelID == PANEL_3D_OUTPUT) && (eNewPanelID != PANEL_3D_OUTPUT)) ||
       ((eCurPanelID == PANEL_2D_OUTPUT) && (eNewPanelID != PANEL_2D_OUTPUT)))
    {
        return TRUE;
    }
    else
    {
        if(eCurPanelID == PANEL_2D_OUTPUT)
        {
            if(halScaler_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_4K3D_SEQUENTIAL && (m_sPalDataPathInfo.ucWarpingAPConfig != eWAPRING_AP_GROUP_4K3D))
            {
                return TRUE;
            }
            else if(halScaler_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_4K3D_DAUL_PIPE && (m_sPalDataPathInfo.ucWarpingAPConfig != eWAPRING_AP_GROUP_4K3D_DUALPIPE))
            {
                return TRUE;
            }
        }
    }

    return FALSE;
}


UINT8 palDataPath_BKInput_First_Input_Ready(void)
{
    if(m_sSourceDesc.ucPrimaryInput == palImgMgr_BackupPrimaryInput_Get())
    {
        if(halScaler_RxPortIsReady(eSOURCE_WINDOW_MAIN, (UINT8)palImgMgr_BackupPrimaryInput_Get()) == eHAL_SCALER_EXEC_CODE_PASS)
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    else if(m_sSourceDesc.ucPrimaryInput == halScaler_BackupSecondaryInput_Get())
    {
        if(halScaler_RxPortIsReady(eSOURCE_WINDOW_SUB, (UINT8)palImgMgr_BackupSecondaryInput_Get()) == eHAL_SCALER_EXEC_CODE_PASS)
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
}

UINT8 palDataPath_BKInput_Second_Input_Ready(void)
{
    if(m_sSourceDesc.ucSecondaryInput == palImgMgr_BackupPrimaryInput_Get())
    {
        if(halScaler_RxPortIsReady(eSOURCE_WINDOW_MAIN, (UINT8)palImgMgr_BackupPrimaryInput_Get()) == eHAL_SCALER_EXEC_CODE_PASS)
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    else if(m_sSourceDesc.ucSecondaryInput == halScaler_BackupSecondaryInput_Get())
    {
        if(halScaler_RxPortIsReady(eSOURCE_WINDOW_SUB, (UINT8)palImgMgr_BackupSecondaryInput_Get()) == eHAL_SCALER_EXEC_CODE_PASS)
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
}

BOOL palDataPath_3DSyncInDet(void)
{
    return m_sPalDataPathInfo.b3DSyncIn;
}

eEXEC_CODE palDataPath_Panel_ChangeCheck(BOOL bBlicking)
{
    UINT8 ucMasterProjectMode = 0;
    UINT8 ucSlaveProjectMode = 0;

    m_sPalDataPathInfo.uiPanelChange = PANEL_CHECK_COUNT;

    //if(m_sPalDataPathInfo.uiPanelChange)
    //{
        palFormatterMgr_Projection_Mode_Get(&ucMasterProjectMode);
        palFormatterMgr_SlaveProjection_Mode_Get(&ucSlaveProjectMode);

        if((ucMasterProjectMode != eCMD_Formatter_External) &&
            (ucSlaveProjectMode != eCMD_Formatter_External)) //如果不是External,就不再做判斷
        {
            palImgMgr_RGBEnableItemSet(eHAL_SCALER_RGB_ITEM_PANELCHANGE); //A70LK_Casper_0008
            palImgMgr_EnableRGB();
            m_sPalDataPathInfo.uiPanelChange = 0;
        }
        else
        {
            if(bBlicking == TRUE)
            {
                palImgMgr_DisableRGB(eHAL_SCALER_RGB_ITEM_PANELCHANGE); //A70LK_Casper_0008
            }
        }

        LOG_MSG(db_APP_DATAPATH, "(%s, %d) ucMasterProjectMode = %d, ucSlaveProjectMode = %d, cPanelChange = %d\n", __FUNCTION__, __LINE__, ucMasterProjectMode, ucSlaveProjectMode, m_sPalDataPathInfo.uiPanelChange);
    //}

    return eEXEC_CODE_PASS;
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


UINT8 palDataPath_PanelConfigGet(void) //A70LK_Larry_0180
{
    //eWAPRING_AP_GROUP_2D
    //eWAPRING_AP_GROUP_3D
    //eWAPRING_AP_GROUP_4K3D
    //eWAPRING_AP_GROUP_4K3D_DUALPIPE
    //eWAPRING_AP_GROUP_HIGH_SPEED

    return m_sPalDataPathInfo.ucWarpingAPConfig;
}



////////////////////////////////////  H2 wait review
////// For Compiler Error
////////////////////////////////////  H2 wait review
UINT8 palDataPath_GetPanelChangeState(void)
{
    return FALSE;
}

UINT8 palDataPath_BKInput_Change(void)
{
    //* - 0 : use main path
    //* - 1 : use backup path

    return m_sSourceDesc.ucCurrentBackupInput;
}

UINT8 palDataPath_BKInput_Status(void)
{
    return eBACKUP_SOURCE_STATUS_AVTIVE;
}

eEXEC_CODE palDataPath_EdgeMaskSet(UINT8 ucEdgeMask)
{
    return eEXEC_CODE_PASS;
}

void palDataPath_Low_Latency_Set(UINT8 ucVal)
{
    return;
}

UINT8 palDataPath_Last3DEnable_Get(void)
{
    return 0;
}

UINT8 FrontEnd_Down_Scaling_Get(void)
{
    return 0;
}

eEXEC_CODE palDataPath_BKInput_SecondHRefresh_Get(UINT8 *ucValue)
{
    //UINT8 ucBackupPrimaryInput = 0;
    UINT8 ucBackupSecondaryInput = 0;

    //palDataMgr_Data_Access(edcBACKUPINPUT_PRIMARY_INPUT, edaREAD, &ucBackupPrimaryInput);
    palDataMgr_Data_Access(edcBACKUPINPUT_SECONDARY_INPUT, edaREAD, &ucBackupSecondaryInput);

    if(palImgMgr_BackupPrimaryInput_Get() == ucBackupSecondaryInput)
    {
        halScaler_InputPortHorzRate(eSOURCE_WINDOW_MAIN, palImgMgr_BackupPrimaryInput_Get(), ucValue);
    }
    else if(palImgMgr_BackupSecondaryInput_Get() == ucBackupSecondaryInput)
    {
        halScaler_InputPortHorzRate(eSOURCE_WINDOW_SUB, palImgMgr_BackupSecondaryInput_Get(), ucValue);
    }
    else
    {
        sprintf((char *)ucValue, "-");
    }

    return eEXEC_CODE_PASS;
}

eEXEC_CODE palDataPath_BKInput_FirstColorSpace_Get(UINT8 *ucValue)
{
    UINT8 ucBackupPrimaryInput = 0;
    //UINT8 ucBackupSecondaryInput = 0;

    palDataMgr_Data_Access(edcBACKUPINPUT_PRIMARY_INPUT, edaREAD, &ucBackupPrimaryInput);
    //palDataMgr_Data_Access(edcBACKUPINPUT_SECONDARY_INPUT, edaREAD, &ucBackupSecondaryInput);

    if(palImgMgr_BackupPrimaryInput_Get() == ucBackupPrimaryInput)
    {
        halScaler_InputPortColorSpace(eSOURCE_WINDOW_MAIN, palImgMgr_BackupPrimaryInput_Get(), ucValue);
    }
    else if(palImgMgr_BackupSecondaryInput_Get() == ucBackupPrimaryInput)
    {
        halScaler_InputPortColorSpace(eSOURCE_WINDOW_SUB, palImgMgr_BackupSecondaryInput_Get(), ucValue);
    }
    else
    {
        sprintf((char *)ucValue, "-");
    }

    return eEXEC_CODE_PASS;
}

eEXEC_CODE palDataPath_BKInput_SecondVideoInfo_Get(void)
{
    return eEXEC_CODE_PASS;
}

eEXEC_CODE palDataPath_BKInput_SecondResoultion_Get(UINT8 *ucValue)
{
    //UINT8 ucBackupPrimaryInput = 0;
    UINT8 ucBackupSecondaryInput = 0;

    //palDataMgr_Data_Access(edcBACKUPINPUT_PRIMARY_INPUT, edaREAD, &ucBackupPrimaryInput);
    palDataMgr_Data_Access(edcBACKUPINPUT_SECONDARY_INPUT, edaREAD, &ucBackupSecondaryInput);

    if(palImgMgr_BackupPrimaryInput_Get() == ucBackupSecondaryInput)
    {
        halScaler_InputPortResolution(eSOURCE_WINDOW_MAIN, palImgMgr_BackupPrimaryInput_Get(), ucValue);
    }
    else if(palImgMgr_BackupSecondaryInput_Get() == ucBackupSecondaryInput)
    {
        halScaler_InputPortResolution(eSOURCE_WINDOW_SUB, palImgMgr_BackupSecondaryInput_Get(), ucValue);
    }
    else
    {
        sprintf((char *)ucValue, "-");
    }

    return eEXEC_CODE_PASS;
}

eEXEC_CODE palDataPath_BKInput_FirstVideoFormat_Get(void)
{
    return eEXEC_CODE_PASS;
}

eEXEC_CODE palDataPath_BKInput_FirstHRefresh_Get(UINT8 *ucValue)
{
    UINT8 ucBackupPrimaryInput = 0;
    //UINT8 ucBackupSecondaryInput = 0;

    palDataMgr_Data_Access(edcBACKUPINPUT_PRIMARY_INPUT, edaREAD, &ucBackupPrimaryInput);
    //palDataMgr_Data_Access(edcBACKUPINPUT_SECONDARY_INPUT, edaREAD, &ucBackupSecondaryInput);

    if(palImgMgr_BackupPrimaryInput_Get() == ucBackupPrimaryInput)
    {
        halScaler_InputPortHorzRate(eSOURCE_WINDOW_MAIN, palImgMgr_BackupPrimaryInput_Get(), ucValue);
    }
    else if(palImgMgr_BackupSecondaryInput_Get() == ucBackupPrimaryInput)
    {
        halScaler_InputPortHorzRate(eSOURCE_WINDOW_SUB, palImgMgr_BackupSecondaryInput_Get(), ucValue);
    }
    else
    {
        sprintf((char *)ucValue, "-");
    }

    return eEXEC_CODE_PASS;
}

eEXEC_CODE palDataPath_BKInput_FirstVideoInfo_Get(void)
{
    return eEXEC_CODE_PASS;
}

eEXEC_CODE palDataPath_BKInput_FirstResoultion_Get(UINT8 *ucValue)
{
    UINT8 ucBackupPrimaryInput = 0;
    //UINT8 ucBackupSecondaryInput = 0;

    palDataMgr_Data_Access(edcBACKUPINPUT_PRIMARY_INPUT, edaREAD, &ucBackupPrimaryInput);
    //palDataMgr_Data_Access(edcBACKUPINPUT_SECONDARY_INPUT, edaREAD, &ucBackupSecondaryInput);

    if(palImgMgr_BackupPrimaryInput_Get() == ucBackupPrimaryInput)
    {
        halScaler_InputPortResolution(eSOURCE_WINDOW_MAIN, palImgMgr_BackupPrimaryInput_Get(), ucValue);
    }
    else if(palImgMgr_BackupSecondaryInput_Get() == ucBackupPrimaryInput)
    {
        halScaler_InputPortResolution(eSOURCE_WINDOW_SUB, palImgMgr_BackupSecondaryInput_Get(), ucValue);
    }
    else
    {
        sprintf((char *)ucValue, "-");
    }


    return eEXEC_CODE_PASS;
}

eEXEC_CODE palDataPath_BKInput_SecondVideoFormat_Get(void)
{
    return eEXEC_CODE_PASS;
}

eEXEC_CODE palDataPath_BKInput_SecondColorSpace_Get(UINT8 *ucValue)
{
    //UINT8 ucBackupPrimaryInput = 0;
    UINT8 ucBackupSecondaryInput = 0;

    //palDataMgr_Data_Access(edcBACKUPINPUT_PRIMARY_INPUT, edaREAD, &ucBackupPrimaryInput);
    palDataMgr_Data_Access(edcBACKUPINPUT_SECONDARY_INPUT, edaREAD, &ucBackupSecondaryInput);

    if(palImgMgr_BackupPrimaryInput_Get() == ucBackupSecondaryInput)
    {
        halScaler_InputPortColorSpace(eSOURCE_WINDOW_MAIN, palImgMgr_BackupPrimaryInput_Get(), ucValue);
    }
    else if(palImgMgr_BackupSecondaryInput_Get() == ucBackupSecondaryInput)
    {
        halScaler_InputPortColorSpace(eSOURCE_WINDOW_SUB, palImgMgr_BackupSecondaryInput_Get(), ucValue);
    }
    else
    {
        sprintf((char *)ucValue, "-");
    }

    return eEXEC_CODE_PASS;
}

UINT8 palDataPath_First_Input_HDR_Get(void)
{
    UINT8 ucBackupPrimaryInput = 0;
    //UINT8 ucBackupSecondaryInput = 0;

    palDataMgr_Data_Access(edcBACKUPINPUT_PRIMARY_INPUT, edaREAD, &ucBackupPrimaryInput);
    //palDataMgr_Data_Access(edcBACKUPINPUT_SECONDARY_INPUT, edaREAD, &ucBackupSecondaryInput);

    if(palImgMgr_BackupPrimaryInput_Get() == ucBackupPrimaryInput)
    {
        if(eHDR_SETTING_HDR == halScaler_HDR_Get(eSOURCE_WINDOW_MAIN))
        {
			return eBACKUP_INPUT_HDR_YES;
        }
        else
        {
            return eBACKUP_INPUT_HDR_NO;
        }
    }
    else if(palImgMgr_BackupSecondaryInput_Get() == ucBackupPrimaryInput)
    {
        if(eHDR_SETTING_HDR == halScaler_HDR_Get(eSOURCE_WINDOW_SUB))
        {
			return eBACKUP_INPUT_HDR_YES;
        }
        else
        {
            return eBACKUP_INPUT_HDR_NO;
        }
    }
    return eBACKUP_INPUT_HDR_NO_SIGNAL;
}

UINT8 palDataPath_Second_Input_HDR_Get(void)
{
    //UINT8 ucBackupPrimaryInput = 0;
    UINT8 ucBackupSecondaryInput = 0;

    //palDataMgr_Data_Access(edcBACKUPINPUT_PRIMARY_INPUT, edaREAD, &ucBackupPrimaryInput);
    palDataMgr_Data_Access(edcBACKUPINPUT_SECONDARY_INPUT, edaREAD, &ucBackupSecondaryInput);

    if(palImgMgr_BackupPrimaryInput_Get() == ucBackupSecondaryInput)
    {
        if(eHDR_SETTING_HDR == halScaler_HDR_Get(eSOURCE_WINDOW_MAIN))
        {
			return eBACKUP_INPUT_HDR_YES;
        }
        else
        {
            return eBACKUP_INPUT_HDR_NO;
        }
    }
    else if(palImgMgr_BackupSecondaryInput_Get() == ucBackupSecondaryInput)
    {
        if(eHDR_SETTING_HDR == halScaler_HDR_Get(eSOURCE_WINDOW_SUB))
        {
			return eBACKUP_INPUT_HDR_YES;
        }
        else
        {
            return eBACKUP_INPUT_HDR_NO;
        }
    }
    return eBACKUP_INPUT_HDR_NO_SIGNAL;
}

eEXEC_CODE palDataPath_Formatter_HDR_Picture_Mode_Set(UINT8 Mode, UINT8 ucHDRSelect)
{
    if((palDataPath_IsSourceLock() && (halScaler_HDR_Get(eSOURCE_WINDOW_MAIN) == eHDR_SETTING_HDR)) &&
        (Mode >= eCM_HDR_LEVEL1 && Mode <= eCM_HDR_LEVEL4))
    {
        halScaler_HDRLevelSet(Mode, ucHDRSelect);
    }
    else if((palDataPath_IsSourceLock() && (halScaler_HDR_Get(eSOURCE_WINDOW_MAIN) == eHDR_SETTING_SDR)) &&
        (Mode >= eCM_HDR_LEVEL1 && Mode <= eCM_HDR_LEVEL4))
    {
        halScaler_HDRLevelSet(Mode, ucHDRSelect);
    }
    else
    {
        halScaler_HDRLevelSet(eCM_HDR_LEVEL_SDR, ucHDRSelect);
    }

    return eEXEC_CODE_PASS;
}

void palDataPath_Test_SetTimeStart(void)
{
    return;
}

void palDataPath_Auto_HDMI_Switch_Source_Config( eCM_SOURCE_ID eSource_ID )
{
    return;
}

void palDataPath_BackupConfigFlag(void)
{
    return;
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

void palDataPath_ProcMutexDataScalerInfoInit(void)      //H2PF_Simon_0033
{
#ifdef SCALER_FPGA_F34
    utilProcMutexData_InhibitColorEnable_RegSet(eOsdInhiColorEn);
    utilProcMutexData_InhibitColorIdx_RegSet(eOsdInhiColor);

    utilProcMutexData_InhibitColorEnable_DataSet(FALSE);
    utilProcMutexData_InhibitColorIdx_DataSet(0xFFFF);

#endif
}


