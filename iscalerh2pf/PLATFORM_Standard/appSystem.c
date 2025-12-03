#include "appSystem.h"
#include "appDataPath.h"
#include "appPoll.h"
//#include "appGui.h"
#include "appCLICmd.h"
#include "appDataMgr.h"
#include "appIllumination.h"
#include "appEnvironment.h"
//#include "appEthernet.h"
#include "appInputProc.h"
//#include "appChipCtrl.h"
//#include "appLedProcAPI.h"
#include "appIapProcAPI.h" //A70LV_Larry_0079
#include "appLANProcAPI.h" //A70LV_Larry_0172
#include "appCLICmd.h"
#include "appHostPath.h"

#include "utilCLICmdAPI.h"
#include "utilDbgMsg.h"
#include "utilDatabaseAPI.h"
#include "opdCtrlAPI.h"
#include "utilDataMgrAPI.h"
#include "utilIPCAPI.h"
#include "utilMisc.h"
#include "utilWarpDemo.h"

#include "palCoreVar.h"
#include "palGui.h"
#include "palFormatterMgr.h"
#include "palMotorMgr.h"
#include "palGeoAPI.h"
#include "palImgMgr.h"
#include "palSysCtrlMgr.h"

#include "halFormatter.h"
#include "halLDProc.h"
#include "halFrontEndCtrlAPI.h"
#include "halMotorCtrlAPI.h"
#include "halMCUCtrlAPI.h"
#include "halBoardCtrlAPI.h"
#include "halCFUCtrlAPI.h"
#include "halFrontEndCtrlAPI.h"
#include "halWarping.h"

//#include "dvLPCIOExp.h"
#ifdef Low_Latency_All
#include "halScaler.h"
#endif	/*Low_Latency_All*/
#include "halGui.h"
#include "halC789CtrlAPI.h"    //A70LV_Doulas_0216
//#include "MemMap.h"

#include "GEC_EventTable.h"
#include "GEC_CoreFunction.h"

#include "utilCommonMSSCAPI.h"		//A65_OPTOMA_Doulas_0069
#include "utilDataMapping.h"
#include "utilHPBU_Tester.h"

#ifdef CUSTOM_OPTOMA
#include "utilOptomaMSSCAPI.h"
#endif

extern sGUI_CALLBACK GuiCb;
static sPAL_SYSTEM_INFORMATION  m_sPalSysInfo;                 //Use for system operation

//Default system settings, like TI gpConfiguration
//It is read only variable, thus do not need semphore protect
static sSYSTEM_CONFIGURATION       m_sDefSysConfiguration;

static BOOL m_bNoFanControlFlag = FALSE; //A70LV_Larry_0079
static BOOL m_bIsAsicInitialed = FALSE;
static UINT8 m_bAsicReadyCheckCount = 20; // Retry to check ASIC ready
static BOOL m_bIs12VPowerLost = FALSE;  //G100_Owen_0064
static BOOL m_bIsLogoCapture = FALSE;	//A65_OPTOMA_Doulas_0125
static UINT8 m_cLensModel = 0; //A35G2_Larry_0063
static BOOL m_bSystemCheck = 0;

//G100_Julie_0018, start.
const char *m_sSystem_Event_String[] =
{
	STRINGER(eSYSTEM_EVENT_IDLE),
	STRINGER(eSYSTEM_EVENT_START),
	STRINGER(eSYSTEM_EVENT_POWERDOWN),
	STRINGER(eSYSTEM_EVENT_POWERKEY),
	STRINGER(eSYSTEM_EVENT_FAULT),
	STRINGER(eSYSTEM_EVENT_COOL),
	STRINGER(eSYSTEM_EVENT_PROGMODE),
	STRINGER(eSYSTEM_EVENT_CFU_SELECT),
	STRINGER(eSYSTEM_EVENT_BIN2FLASH),
	STRINGER(eSYSTEM_EVENT_CUSTOMER_SPLAH),
	STRINGER(eSYSTEM_EVENT_LOGO_CAPTURE),
};

#define SYSTEM_EVENT_STRING_MAX  sizeof(m_sSystem_Event_String)/sizeof(m_sSystem_Event_String[0])


//A35G2_Simon_0075
sMONITOR_TASK_INFO sMonTaskInfo[eTID_NUMBER] = {
                                                   {"appCLICmdProcess_Task",10, 10, eCOUNTER_TYPE_CLI_CMD_PROC_TASK_MONITOR,    10, 10, TASK_STATUS_NORMAL, FALSE,  "0"},
                                                   {"appCLICmdDecode_Task", 10, 10, eCOUNTER_TYPE_CLI_CMD_DECODE_TASK_MONITOR,  10, 10, TASK_STATUS_NORMAL, FALSE,  "0"},
                                                   {"palDataPath_Task",     15, 15, eCOUNTER_TYPE_DATAPATH_TASK_MONITOR,        10, 10, TASK_STATUS_NORMAL, FALSE,  "0"},
                                                   {"palDataPath_TaskSub",  15, 15, eCOUNTER_TYPE_SUB_DATAPATH_TASK_MONITOR,    10, 10, TASK_STATUS_NORMAL, FALSE,  "0"},
                                                   {"appGui_Task",          10, 10, eCOUNTER_TYPE_GUI_TASK_MONITOR,             10, 10, TASK_STATUS_NORMAL, TRUE,   "0"}, //A35G2_BRC_Casper_0100  //A35G2_Simon_0078
                                                   {"palHostPath_Task",     10, 10, eCOUNTER_TYPE_HOST_PATH_TASK_MONITOR,       10, 10, TASK_STATUS_NORMAL, FALSE,  "0"},
                                                   {"appPoll_Task",         10, 10, eCOUNTER_TYPE_POLLING_TASK_MONITOR,         10, 10, TASK_STATUS_NORMAL, FALSE,  "0"},
                                                   {"Scaler_recvData",      10, 10, eCOUNTER_TYPE_IPC_RECV_DATA_TASK_MONITOR,   10, 10, TASK_STATUS_NORMAL, FALSE,  "0"},
                                                   {"palSystem_SysMonTask", 10, 10, eCOUNTER_TYPE_SYSTEM_TASK_MONITOR,          10, 10, TASK_STATUS_NORMAL, TRUE,   "0"},
                                               };


static void palSystem_PowerOPDEvent(UINT16 uiMsgID)
{
    uOPD_DATA uOPDData = {0};

    if(uiMsgID >= SYSTEM_EVENT_STRING_MAX)
    {
        ASSERT_ALWAYS();
        return;
    }

    sprintf(uOPDData.cString, "%s", m_sSystem_Event_String[uiMsgID]);
    utilOPD_EventSet(eOPD_SYSTEM_STATUS_LOG, &uOPDData);
}
//G100_Julie_0018, end.

static void palSystem_Fault(void)
{
    //while(1) //A70LV_Larry_0118 modify
    //{
        LOG_MSG(db_ALWAYS, "(func:%s, line:%d): Fault!!\r\n", __FUNCTION__, __LINE__);
    //}
}

static void palSystem_SetSysEvent(eSYSTEM_EVENT eSysEvent)
{
    UINT32 temp;
#if (SYSTEM_OS_TYPE == POSIX_COMPLIANT)
    if(palMailBox_Send_MsgQueue(m_sPalSysInfo.xMsgQueue, QUEUE_SYSTEM_NAME, eSysEvent, -1, 0, &temp) != eMAIL_BOX_EXEC_CODE_PASS)
#else
    if(palMailBox_Send(m_sPalSysInfo.xMsgQueue, m_sPalSysInfo.xEventGroupHandle, eSysEvent, -1, 0, &temp, FALSE) != eMAIL_BOX_EXEC_CODE_PASS)
#endif
    {
        LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d) : Set SysEvent[%d] Fail!\r\n", __FUNCTION__, __LINE__, eSysEvent);
    }

}

static eEXEC_CODE palSystem_Standby(void)
{
    BOOL bPass = TRUE;

    palSystem_SystemReadySet(0);

    if(palDataPath_PowerStandby() != eEXEC_CODE_PASS)            /* inform datapath */
    {
        bPass = FALSE;
        LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d) System Task : datapath won't transition to standby\n", __FUNCTION__, __LINE__);
    }

    if(palDataPath_PowerStandbySub() != eEXEC_CODE_PASS)  //A70LV_Doulas_0003   /* inform datapath sub*/
    {
        bPass = FALSE;
        LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d) System Task : datapath sub won't transition to standby\n", __FUNCTION__, __LINE__);
    }


    if(palEnvironment_PowerStandby() != eEXEC_CODE_PASS)
    {
        bPass = FALSE;
        LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d) System Task : environment won't transition to standby\n", __FUNCTION__, __LINE__);
    }

    if(palIllumination_PowerStandby() != eEXEC_CODE_PASS)
    {
        bPass = FALSE;
        LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d) System Task : illumination won't transition to standby\n", __FUNCTION__, __LINE__);
    }

    if(palSystem_PowerStandby_OSDExit() != eEXEC_CODE_PASS) //HICC2_Casper_0011
    {
        bPass = FALSE;
        LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d) System Task : gui won't transition to standby\n", __FUNCTION__, __LINE__);
    }

    return bPass ? eEXEC_CODE_PASS : eEXEC_CODE_FAIL;
}

static eEXEC_CODE palSystem_UpgradeEnable(void) //A70LV_Larry_0112
{
    BOOL bPass = TRUE;

    if(palSystem_PowerStandby_OSDExit() != eEXEC_CODE_PASS)
    {
        bPass = FALSE;
        LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d) System Task : gui won't transition to standby\n", __FUNCTION__, __LINE__);
    }

    if(palEnvironment_PowerStandby() != eEXEC_CODE_PASS)
    {
        bPass = FALSE;
        LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d) System Task : environment won't transition to standby\n", __FUNCTION__, __LINE__);
    }

    if(palIllumination_UpgradeCooling() != eEXEC_CODE_PASS)
    {
        bPass = FALSE;
        LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d) System Task : illumination won't transition to standby\n", __FUNCTION__, __LINE__);
    }


    if(palDataPath_PowerStandby() != eEXEC_CODE_PASS)            /* inform datapath */
    {
        bPass = FALSE;
        LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d) System Task : datapath won't transition to standby\n", __FUNCTION__, __LINE__);
    }

    if(palDataPath_PowerStandbySub() != eEXEC_CODE_PASS)    /* inform datapath sub*/
    {
        bPass = FALSE;
        LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d) System Task : datapath sub won't transition to standby\n", __FUNCTION__, __LINE__);
    }

    return bPass ? eEXEC_CODE_PASS : eEXEC_CODE_FAIL;
}

void palSystem_CoolDown(void)
{
    palSystem_SetSysEvent(eSYSTEM_EVENT_COOL);
}

void palSystem_PowerDown(void)                /* inform system of power down */
{
	if(utilWarp_GetOsdPatternType() != PAT_TYPE__OFF)	//G100_Doulas_0041
	{
		palGeo_AdvWarpShowOsdPattern(PAT_TYPE__OFF);
	}
    palSystem_SetSysEvent(eSYSTEM_EVENT_POWERDOWN);
}

void palSystem_PowerKey(void)              /* power key pressed (system not in active mode) */
{
    palSystem_SetSysEvent(eSYSTEM_EVENT_POWERKEY);
}

void palSystem_Idle(void)
{
    switch(m_sPalSysInfo.ePowerState)
    {
        case ePOWER_STATE_WARMUP:
        case ePOWER_STATE_ACTIVE:
        case ePOWER_STATE_COOLING:
        case ePOWER_STATE_UPGRADE:
            palSystem_SetSysEvent(eSYSTEM_EVENT_IDLE);
            break;

        default:
            LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d) : ePowerState[%d] Fail!\r\n", __FUNCTION__, __LINE__, m_sPalSysInfo.ePowerState);
            break;
    }

}

void palSystem_WarmUp(void)
{
    switch(m_sPalSysInfo.ePowerState)
    {
        case ePOWER_STATE_RESET:
        case ePOWER_STATE_STANDBY:
            m_sPalSysInfo.ePowerState = ePOWER_STATE_RESET;
            palSystem_SetSysEvent(eSYSTEM_EVENT_START);
            break;

        default:
            LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d) : ePowerState[%d] Fail!\r\n", __FUNCTION__, __LINE__, m_sPalSysInfo.ePowerState);
            break;
    }
}

void palSystem_CustomerSplah(UINT8 Customer) //A70LV_Larry_0235
{
    UINT32 temp;
#if (SYSTEM_OS_TYPE == POSIX_COMPLIANT)
    if(palMailBox_Send_MsgQueue(m_sPalSysInfo.xMsgQueue, QUEUE_SYSTEM_NAME, eSYSTEM_EVENT_CUSTOMER_SPLAH, -1, Customer, &temp) != eMAIL_BOX_EXEC_CODE_PASS)
#else
    if(palMailBox_Send(m_sPalSysInfo.xMsgQueue, m_sPalSysInfo.xEventGroupHandle, eSYSTEM_EVENT_CUSTOMER_SPLAH, -1, Customer, &temp, FALSE) != eMAIL_BOX_EXEC_CODE_PASS)
#endif
    {
        LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d) : Set CustomerSplah[%d] Fail!\r\n", __FUNCTION__, __LINE__);
    }
}

void palSystem_FwUpgradeMode(UINT8 ucSelect) //A70LV_Larry_0112
{
    UINT32 temp;

#if (SYSTEM_OS_TYPE == POSIX_COMPLIANT)
    if(palMailBox_Send_MsgQueue(m_sPalSysInfo.xMsgQueue, QUEUE_SYSTEM_NAME, eSYSTEM_EVENT_CFU_SELECT, -1, (UINT32)ucSelect, &temp) != eMAIL_BOX_EXEC_CODE_PASS)
#else
    if(palMailBox_Send(m_sPalSysInfo.xMsgQueue, m_sPalSysInfo.xEventGroupHandle, eSYSTEM_EVENT_CFU_SELECT, -1, (UINT32)ucSelect, &temp, FALSE) != eMAIL_BOX_EXEC_CODE_PASS)
#endif
    {
        LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d) : Set Fw upgrade select[%d] Fail!\r\n", __FUNCTION__, __LINE__, ucSelect);
    }
}

void palSystem_LogoCapture(void)
{
    UINT32 temp;

#if (SYSTEM_OS_TYPE == POSIX_COMPLIANT)
    if(palMailBox_Send_MsgQueue(m_sPalSysInfo.xMsgQueue, QUEUE_SYSTEM_NAME, eSYSTEM_EVENT_LOGO_CAPTURE, -1, 0, &temp) != eMAIL_BOX_EXEC_CODE_PASS)
#else
    if(palMailBox_Send(m_sPalSysInfo.xMsgQueue, m_sPalSysInfo.xEventGroupHandle, eSYSTEM_EVENT_LOGO_CAPTURE, -1, 0, &temp, FALSE) != eMAIL_BOX_EXEC_CODE_PASS)
#endif
    {
        LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d) : Fail!\r\n", __FUNCTION__, __LINE__);
    }
}

void palSystem_ScreenSaveToImage(void)// R70G2_Bruce#0022
{
    UINT32 temp;

#if (SYSTEM_OS_TYPE == POSIX_COMPLIANT)
    if(palMailBox_Send_MsgQueue(m_sPalSysInfo.xMsgQueue, QUEUE_SYSTEM_NAME, eSYSTEM_EVENT_SCREEN_SAVE_TO_IMAGE, -1, 0, &temp) != eMAIL_BOX_EXEC_CODE_PASS)
#else
    if(palMailBox_Send(m_sPalSysInfo.xMsgQueue, m_sPalSysInfo.xEventGroupHandle, eSYSTEM_EVENT_SCREEN_SAVE_TO_IMAGE, -1, 0, &temp, FALSE) != eMAIL_BOX_EXEC_CODE_PASS)
#endif
    {
        LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d) : Fail!\r\n", __FUNCTION__, __LINE__);
    }
}

void palSystem_LogoReplacement(void)
{
    UINT32 temp;

#if (SYSTEM_OS_TYPE == POSIX_COMPLIANT)
    if(palMailBox_Send_MsgQueue(m_sPalSysInfo.xMsgQueue, QUEUE_SYSTEM_NAME, eSYSTEM_EVENT_LOGO_REPLACEMENT, -1, 0, &temp) != eMAIL_BOX_EXEC_CODE_PASS)
#else
    if(palMailBox_Send(m_sPalSysInfo.xMsgQueue, m_sPalSysInfo.xEventGroupHandle, eSYSTEM_EVENT_LOGO_REPLACEMENT, -1, 0, &temp, FALSE) != eMAIL_BOX_EXEC_CODE_PASS)
#endif
    {
        LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d) : Fail!\r\n", __FUNCTION__, __LINE__);
    }
}

void palSystem_Enter2WMode(void)
{
    UINT32 temp;

#if (SYSTEM_OS_TYPE == POSIX_COMPLIANT)
    if(palMailBox_Send_MsgQueue(m_sPalSysInfo.xMsgQueue, QUEUE_SYSTEM_NAME, eSYSTEM_EVENT_ENTER_2W_MODE, -1, 0, &temp) != eMAIL_BOX_EXEC_CODE_PASS)
#else
    if(palMailBox_Send(m_sPalSysInfo.xMsgQueue, m_sPalSysInfo.xEventGroupHandle, eSYSTEM_EVENT_ENTER_2W_MODE, -1, 0, &temp, FALSE) != eMAIL_BOX_EXEC_CODE_PASS)
#endif
    {
        LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d) : Fail!\r\n", __FUNCTION__, __LINE__);
    }
}

//A35G2_Simon_0091
void palSystem_WriteCustomizeEDID_Event(UINT8 ucSourceID)
{
    UINT32 temp;

#if (SYSTEM_OS_TYPE == POSIX_COMPLIANT)
    if(palMailBox_Send_MsgQueue(m_sPalSysInfo.xMsgQueue, QUEUE_SYSTEM_NAME, eSYSTEM_EVENT_CUSTOMIZE_EDID, -1, ucSourceID, &temp) != eMAIL_BOX_EXEC_CODE_PASS)
#else
    if(palMailBox_Send(m_sPalSysInfo.xMsgQueue, m_sPalSysInfo.xEventGroupHandle, eSYSTEM_EVENT_CUSTOMIZE_EDID, -1, ucSourceID, &temp, FALSE) != eMAIL_BOX_EXEC_CODE_PASS)
#endif
    {
        LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d) : Fail!\r\n", __FUNCTION__, __LINE__);
    }
}

void palSystem_ApplyAPModeGeoFunc_Event(void)  //A35G2_Simon_0100
{
    UINT32 temp;

#if (SYSTEM_OS_TYPE == POSIX_COMPLIANT)
    if(palMailBox_Send_MsgQueue(m_sPalSysInfo.xMsgQueue, QUEUE_SYSTEM_NAME, eSYSTEM_EVENT_APPLY_AP_MODE_GEO_FUNC, -1, 0, &temp) != eMAIL_BOX_EXEC_CODE_PASS)
#else
    if(palMailBox_Send(m_sPalSysInfo.xMsgQueue, m_sPalSysInfo.xEventGroupHandle, eSYSTEM_EVENT_APPLY_AP_MODE_GEO_FUNC, -1, 0, &temp, FALSE) != eMAIL_BOX_EXEC_CODE_PASS)
#endif
    {
        LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d) : Fail!\r\n", __FUNCTION__, __LINE__);
    }
}

void palSystem_ActuatorPatternSet(UINT8 ucPatten)
{
    (void)ucPatten;
}


static eEXEC_CODE palSystem_NormalRun(void)
{
    BOOL bPass = TRUE;
    UINT8 ucData = FALSE; //HICC2_Doulas_0060

    LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d) Transition to normal operating mode\r\n", __FUNCTION__, __LINE__);

    m_bIsAsicInitialed = FALSE;
    m_bAsicReadyCheckCount = 20;
    palSystem_PanelID_Set((ePANEL_ID)m_sDefSysConfiguration.System.ucDefaultPanelId);

    if(palDataMgr_PowerNormal() != eEXEC_CODE_PASS)
    {
        bPass = FALSE;
        LOG_MSG(db_ALWAYS, "(func:%s, line:%d)Fault in Data transition to operating mode\n", __FUNCTION__, __LINE__);
    }

    //clear OPFU Check
    if(m_bIsAsicInitialed) //HICC2_Doulas_0060
    {
        palDataMgr_Data_Access(edcOPFU_CHECK, edaWRITE_THROUGH_WITH_ACTION, &ucData);
    }

    if(palEnvironment_PowerNormal() != eEXEC_CODE_PASS)
    {
        bPass = FALSE;
        LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d)Fault in environment transition to operating mode\n", __FUNCTION__, __LINE__);
    }

    if(palDataPath_PowerNormal(m_sPalSysInfo.ePanelTimingId) != eEXEC_CODE_PASS)
    {
        bPass = FALSE;
        LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d)Fault in datapath transition to operating mode\n", __FUNCTION__, __LINE__);
    }

    if(palDataPath_PowerNormalSub(m_sPalSysInfo.ePanelTimingId) != eEXEC_CODE_PASS)    //A70LV_Doulas_0003 Add
    {
        bPass = FALSE;
        LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d)Fault in datapath sub transition to operating mode\n", __FUNCTION__, __LINE__);
    }

    if(palIllumination_PowerNormal() != eEXEC_CODE_PASS)
    {
        bPass = FALSE;
        LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d) System Task : Fault in illumination transition to operating mode\n", __FUNCTION__, __LINE__);
    }

    if(palInputProc_PowerNormal() != eEXEC_CODE_PASS)
    {
        bPass = FALSE;
        LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d) System Task : Fault in InputProc transition to operating mode\n", __FUNCTION__, __LINE__);
    }

    if(!bPass)
    {
        LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d) System Task : Can't transition to operating mode\r\n", __FUNCTION__, __LINE__);
        palSystem_Standby();                  /* ensure everything back to standby */
    }
    else
    {
        LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d) System Task : Transition to normal complete\r\n", __FUNCTION__, __LINE__);
    }

    return bPass ? eEXEC_CODE_PASS : eEXEC_CODE_FAIL;
}

static void palSystem_CallBack(UINT16 uiMsgID, UINT32 ulParam1, UINT32 *pulParam2)
{
    switch(uiMsgID)
    {
        case eSYSTEM_EVENT_IDLE:
            {
                LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d) : eSYSTEM_EVENT_IDLE\r\n", __FUNCTION__, __LINE__);
                m_bIsAsicInitialed = FALSE;
                m_sPalSysInfo.ePowerState = ePOWER_STATE_STANDBY;
                palSystem_HostReadySet(0);
                palSystem_SystemReadySet(0);

                eRESULT eRet = halMCUCtrl_SystemState_Set((UINT8)m_sPalSysInfo.ePowerState);

                if(eRet != rcSUCCESS) //G100_Larry_0022
                {
                    LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d) halMCUCtrl_SystemState_Set Fail (%d)\r\n", __FUNCTION__, __LINE__, eRet);
                }

                UINT32 Data = 0;
                palLANProcPowerStateSend(ePOWER_STATE_STANDBY, 4, (UINT8*)&Data);
                m_bIs12VPowerLost = FALSE;  //G100_Owen_0064
            }
            break;

        case eSYSTEM_EVENT_START:
            LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d) : eSYSTEM_EVENT_START\r\n", __FUNCTION__, __LINE__);

            if(m_sPalSysInfo.ePowerState != ePOWER_STATE_RESET)          /* validate state */
            {
                LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d) : Invalid state for eSYSTEM_EVENT_START\r\n", __FUNCTION__, __LINE__);
            }
            else if(m_sDefSysConfiguration.System.ucStartupState == eSTARTUP_STATE_NORMAL)
            {

                UINT8 ucHostReady = 0;

                halFormatter_AsicReadySet(TRUE); //HICC2_Doulas_0041 Modify//HICC2_Doulas_0040 remove
			    #ifdef CUSTOM_OPTOMA
				halGui_UpgradeAccess_Set(FALSE);	//A65_OPTOMA_Doulas_0126
				#endif

                if(rcSUCCESS == halMCU_SystemReadyGet(&ucHostReady))//HICC2_Julie_0046
                {
                    m_bSystemCheck = FALSE;

                    if(ucHostReady == SINGLE_BOARD_NUMBER)
                    {
                        Board_SingleBoard_Set(TRUE);

                        LOG_MSG(db_ASSERT, "Single Board debug\n");
                        printf("Single Board debug\n");
                    }
                    else if(ucHostReady == SYSTEM_CHECK_NUMBER)
                    {
                        m_bSystemCheck = TRUE;
                        LOG_MSG(db_ASSERT, "System check debug\n");
                        printf("System check debug\n");
                    }
                }

                if(palSystem_NormalRun() == eEXEC_CODE_PASS)
                {
                    UINT8 ucOPFUEnable = 0;

                    palCoreVar_SetSystemPowerOnVar(TRUE);
                    m_sPalSysInfo.ePowerState = ePOWER_STATE_ACTIVE;

                    halMCU_OPFU_Get(&ucOPFUEnable);
                    if(ucOPFUEnable == TRUE)  //G100_Simon_0064  //OPFU is not complete
                    {
    					LOG_MSG(db_UPGRADE, "\r\n(func:%s, line:%d) OPFU is not complete, Entering OPFU Mode\r\n", __FUNCTION__, __LINE__);
                        palSystem_FwUpgradeMode(eSYSTEM_UPGRADE_MODE_SET);
                    }

					palSystem_PowerOPDEvent(uiMsgID); //G100_Julie_00020
					LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
                }
                else
                {
                    m_sPalSysInfo.ePowerState = ePOWER_STATE_COOLING;
                    LOG_MSG(db_ALWAYS, "(func:%s, line:%d) : Can not start system!!!!!\r\n", __FUNCTION__, __LINE__);
                }

            }
            else if(m_sDefSysConfiguration.System.ucStartupState == eSTARTUP_STATE_STANDBY)
            {
                palSystem_Standby();
                m_sPalSysInfo.ePowerState = ePOWER_STATE_STANDBY;
                palSystem_HostReadySet(0);
            }
            LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d) : eSYSTEM_EVENT_START ps=%d\r\n", __FUNCTION__, __LINE__, m_sPalSysInfo.ePowerState);

            break;

        case eSYSTEM_EVENT_POWERDOWN:
            LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d) : eSYSTEM_EVENT_POWERDOWN %d\r\n", __FUNCTION__, __LINE__,m_sPalSysInfo.ePowerState);	//G100_Doulas_0019
			#ifdef CUSTOM_OPTOMA		//A65_OPTOMA_Doulas_0069
			utilOptoma_OutputInfoMsg(sOptoma_System_Auto_Send_Lut[eOPT_COOLING_DOWN].cINFO_String);
			#endif
            #if 1
            {
                UINT8 ucVal = 1;
                palDataMgr_Data_Access(edcTWIST_LINK, edaREAD, &ucVal);

                if(ucVal != 0)
                {
                    ucVal = 0 ;
                    palDataMgr_Data_Access(edcTWIST_LINK, edaWRITE_THROUGH_WITH_ACTION, &ucVal);
                }
            }
            #endif

            if(m_sPalSysInfo.ePowerState == ePOWER_STATE_ACTIVE)
            {
                //G100_Tim_0007, mod, start
                UINT8 ucData = 0;   //G100_Clare_0055

                if(palDataMgr_CurTestPatternGet() != eTID_OFF) //HICC2_Doulas_0128
                {
                    palDataMgr_TestPatternHandle(eTID_OFF);
                }
                palDataMgr_Data_Access(edcFAST_POWER_ON, edaREAD, &ucData);
                if(ucData && palEnvironment_Fake_Power_Down_Get() == FALSE) //A35G2_BRC_Casper_0053
                {
                    palEnvironment_Fake_Power_Down_Set(TRUE);
                }
                else
                {
					ucData = 0xFF;
                    m_sPalSysInfo.ePowerState = ePOWER_STATE_COOLING;   //G100_Owen_0064
                    palEnvironment_Fake_Power_Down_Set(FALSE); //A35G2_BRC_Casper_0054

                    palDataMgr_OPDSnapshot(eOPD_POWER_OFF_LOG); //G100_Steven_0081

                    //halMCU_SystemCooling();
                    palCoreVar_SetSystemPowerOnVar(FALSE);
                    palCoreVar_SetIsCoolingVar(TRUE);
                    palSystem_Standby();
                    if(!m_bIs12VPowerLost)  //G100_Owen_0064
                    {
                        halFormatter_Standby(); //A70LV_Larry_0212
                    }
                    palDataMgr_Data_Access(edcLENS_CALIBRATION, edaWRITE_THROUGH_WITH_ACTION, &ucData);
                    //palMotorLensSet(eLENS_CMDS_CANCEL); //A70LV_Larry_0388
                    //palLedProc_LED_Behavior_Set(eLED_STATUS_COOLING); //A70LV_Larry_0020
				}
				halFormatter_AsicReadySet(FALSE); //HICC2_Doulas_0041
				#if (ENABLE_COLOR_UNIFORMITY == TRUE)		//G100_Tim_0025, add, start
				palGeo_Color_Uniformity_Init_Status_Set( WARPING_ACU_INIT_NOT_READY );
				#endif //ENABLE_COLOR_UNIFORMITY			//G100_Tim_0025, add, end
            }
            else if((m_sPalSysInfo.ePowerState == ePOWER_STATE_UPGRADE) && (m_sPalSysInfo.ucFW_upgrade == 0)) //A70LV_Larry_0121
            {
                UINT8 ucValue = 0xFF;

                palSystem_Standby();
                halMCU_SystemCooling();
                //palLedProc_LED_Behavior_Set(eLED_STATUS_COOLING);
                palDataMgr_Data_Access(edcLENS_CALIBRATION, edaWRITE_THROUGH_WITH_ACTION, &ucValue);
                //palMotorLensSet(eLENS_CMDS_CANCEL); //A70LV_Larry_0388
                m_sPalSysInfo.ePowerState = ePOWER_STATE_COOLING;
                halFormatter_AsicReadySet(FALSE); //HICC2_Doulas_0041
            }
            else if(m_sPalSysInfo.ePowerState == ePOWER_STATE_STANDBY)
            {
                palEnvironment_StartCoolTimer();
                m_sPalSysInfo.ePowerState = ePOWER_STATE_COOLING;
                halFormatter_AsicReadySet(FALSE); //HICC2_Doulas_0041
            }
            else
            {
                LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d) : Power-down unexpected: Not in ePOWER_STATE_ACTIVE\r\n", __FUNCTION__, __LINE__);
            }

            utilOPD_Write2File();  //A35G2_Simon_0075
            break;

        case eSYSTEM_EVENT_POWERKEY:
            //G100_Tim_0007, mod, start
            if(palEnvironment_Fake_Power_Down_Get())
            {
                palEnvironment_Fake_Power_Down_Set(FALSE);
            }
            else
            {
                switch(m_sPalSysInfo.ePowerState)
                {
                    case ePOWER_STATE_RESET:
                    case ePOWER_STATE_STANDBY:
                        halBoard_PowerOn();
                        break;

                    default:
                        break;
                }
            }
            //G100_Tim_0007, mod, end
            break;

        case eSYSTEM_EVENT_COOL:
            LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d) : eSYSTEM_EVENT_COOL\r\n", __FUNCTION__, __LINE__);

            if(m_sPalSysInfo.ePowerState != ePOWER_STATE_COOLING)
            {
                LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d) : Cooldown unexpected: Not in ePOWER_STATE_COOLING\r\n", __FUNCTION__, __LINE__);
            }
            else if(m_sDefSysConfiguration.System.ucStartupState == eSTARTUP_STATE_NORMAL)
            {
                //TODO
                LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d) : After Cooling, go to start up again\r\n", __FUNCTION__, __LINE__);
            }
            else if(m_sDefSysConfiguration.System.ucStartupState == eSTARTUP_STATE_STANDBY)
            {
                //TODO
                LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d) : After Cooling, go to standby\r\n", __FUNCTION__, __LINE__);
            }
            else if(m_sDefSysConfiguration.System.ucStartupState == eSTARTUP_STATE_LOWPOWER)
            {
                //TODO
                LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d) : After Cooling, go to low power\r\n", __FUNCTION__, __LINE__);
            }

            UINT8 ucVal = 1; //HICC2_Doulas_0078
			palDataMgr_Data_Access(edcSTANDBY_MODE, edaREAD, &ucVal); //HICC2_Doulas_0078
            if(ucVal != eCM_STANDBY_MODE_COMMUNICATION) //HICC2_Doulas_0078
            {
                halBoard_CameraEnableSet(FALSE); //G100_Larry_0029
            }

            //utilEventHandler_Send(eEVENT_LIST_GOTO_STAND_BY, 0, NULL); //A70LV_Larry_0009
            palSystem_PowerOff_Config(); //A70LV_Larry_0112
			palSystem_PowerOPDEvent(uiMsgID); //G100_Julie_0018
            MS_SLEEP(2000); //for MCU power off sequence to avoid i2c fail

			#ifdef CUSTOM_OPTOMA		//A35G2_Coda_0132
			utilOptoma_CLI_StringOutput(eccTelnet, sOptoma_System_Auto_Send_Lut[eOPT_STANDBY_MODE].cINFO_String); //send info0 for telnet only. rs232 send by 54605 mcu
			#endif
            break;

        case eSYSTEM_EVENT_CFU_SELECT: //A70LV_Larry_0112
            {
                LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d) : fw upgrade select [%d] PowerState [%d]\r\n", __FUNCTION__, __LINE__, ulParam1, m_sPalSysInfo.ePowerState);

                switch((eSYSTEM_UPGRADE_SELECT)ulParam1)
                {
                    case eSYSTEM_UPGRADE_MODE_SET:
                        if(m_sPalSysInfo.ePowerState == ePOWER_STATE_ACTIVE)
                        {
                            UINT8 ucData = TRUE;
                            UINT8 ucValue = 0xFF;
                            UINT8 ucEnable = eCM_STANDBY_MODE_COMMUNICATION;
                            eDATA_CODE eDataCode = edcOPFU_CHECK; //A70Gen2_Julie_0097

                            palCoreVar_SetSystemPowerOnVar(FALSE);
                            palCoreVar_SetIsCoolingVar(TRUE);
                            palSystem_UpgradeEnable();

                            palDataMgr_Data_Access(edcSTANDBY_MODE, edaWRITE_THROUGH_WITH_ACTION, (void*)&ucEnable);

                            //palLedProc_LED_Behavior_Set(eLED_STATUS_FW_UPGRAGE);
                            //palLedProc_Poll(1);
                            palInputProc_DisableKey(1); //A70LV_Larry_0191
                            palLANProcStartSet(0); //A70LV_Larry_0191

                            m_sPalSysInfo.ePowerState = ePOWER_STATE_UPGRADE; //A70LV_Larry_0121

                            halFormatter_Upgrade_Set(TRUE);  //A70LV_Larry_0138
                            //halMCU_DDP_UpgradeSet(TRUE);
                            //palMotorLensSet(eLENS_CMDS_CANCEL); //A70LV_Larry_0388
                            palDataMgr_Data_Access(edcLENS_CALIBRATION, edaWRITE_THROUGH_WITH_ACTION, &ucValue);

                            palDataMgr_Data_Access(edcOPFU_CHECK, edaWRITE_THROUGH_WITH_ACTION, &ucData);   //G100_Simon_0064
                            halMCU_OPFU_Set(TRUE);  //G100_Simon_0063

                            ucEnable = 0;

                            MS_SLEEP(10);

                            halMCU_OPFU_Get(&ucEnable);

                            palDataMgr_ImportDataToDatabase(eDataCode); //A70Gen2_Julie_0097
                            utilIpc_SendData(eIPC_SEND_DATA_UPDATE_DATACODE_ITEM, 0, &eDataCode, eEXEC_CODE_PASS, sizeof(UINT16)*1); //A70Gen2_Julie_0097//HICC2_Julie_0009

                            LOG_MSG(db_UPGRADE, "(func:%s, line:%d) : opfu enable %d\r\n", __FUNCTION__, __LINE__, ucEnable);
                            LOG_MSG(db_ALWAYS, "\nOPFU Mode\n");

                            //utilStartupWrite(ePARA_STARTUP_OPFU_ENABLE, (void*)&ucEnable);
                            //utilStartupUpdate();

                            LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d) : fw upgrade mode enter\r\n", __FUNCTION__, __LINE__);
                        }
                        else if(m_sPalSysInfo.ePowerState == ePOWER_STATE_UPGRADE) //for retry
                        {
                            BYTE ucEnable = 0;

                            halMCU_OPFU_Set(TRUE);

                            ucEnable = 0;

                            MS_SLEEP(10);

                            halMCU_OPFU_Get(&ucEnable);

                            LOG_MSG(db_UPGRADE, "(func:%s, line:%d) : opfu enable %d\r\n", __FUNCTION__, __LINE__, ucEnable);
                            LOG_MSG(db_ALWAYS,  "\nOPFU Mode\n");
                        }
                        //halMCU_DDP_UpgradeSet(TRUE);    //Test
                        break;

                    case eSYSTEM_UPGRADE_SELECT_LPCMCU:
                        {
                            //vTaskPrioritySet(m_sPalSysInfo.xTaskHandle, (APP_SYSMON_PRIORITY+2));
                            m_sPalSysInfo.ucFW_upgrade = 1;

                            //palSystem_UpgradeLPCMCU(eLPCMCU_INDEX_FRONT_2K, TRUE);
                            //palSystem_UpgradeLPCMCU(eLPCMCU_INDEX_MOTORBD, TRUE);

                            if(rcSUCCESS == palSystem_UpgradeLPCMCU(eLPCMCU_INDEX_SYSTEM, TRUE))
                            {
                                LOG_MSG(db_UPGRADE, "System upgrade pass\r\n");
                            }
                            else
                            {
                                LOG_MSG(db_UPGRADE, "System upgrade fail\r\n");
                            }
                            if(rcSUCCESS == palSystem_UpgradeLPCMCU(eLPCMCU_INDEX_FMT_2K, TRUE))
                            {
                                LOG_MSG(db_UPGRADE, "FMT upgrade pass\r\n");
                            }
                            else
                            {
                                LOG_MSG(db_UPGRADE, "FMT upgrade fail\r\n");
                            }
                            if(rcSUCCESS == palSystem_UpgradeLPCMCU(eLPCMCU_INDEX_FRONT_2K, TRUE))
                            {
                                LOG_MSG(db_UPGRADE, "Frontend upgrade pass\r\n");
                            }
                            else
                            {
                                LOG_MSG(db_UPGRADE, "Frontend upgrade fail\r\n");
                            }
                            if(rcSUCCESS == palSystem_UpgradeLPCMCU(eLPCMCU_INDEX_MOTORBD, TRUE))
                            {
                                LOG_MSG(db_UPGRADE, "Motor board upgrade pass\r\n");
                            }
                            else
                            {
                                LOG_MSG(db_UPGRADE, "Motor board upgrade fail\r\n");
                            }
                            //vTaskPrioritySet(m_sPalSysInfo.xTaskHandle, (APP_SYSMON_PRIORITY));
                            m_sPalSysInfo.ucFW_upgrade = 0;
                        }
                        break;

                    case eSYSTEM_UPGRADE_SELECT_SCALER_FOCUS:
                        {
                            vTaskPrioritySet(m_sPalSysInfo.xTaskHandle, (APP_SYSMON_PRIORITY+2));
                            m_sPalSysInfo.ucFW_upgrade = 1;

                            palSystem_Scaler_Upgrade(1);

                            vTaskPrioritySet(m_sPalSysInfo.xTaskHandle, (APP_SYSMON_PRIORITY));
                            m_sPalSysInfo.ucFW_upgrade = 0;
                            halMCU_OPFU_Check_Set(FALSE);
                            halMCU_OPFU_Set(FALSE);
                        }
                        break;

                    case eSYSTEM_UPGRADE_SELECT_SCALER:
                        {
                            vTaskPrioritySet(m_sPalSysInfo.xTaskHandle, (APP_SYSMON_PRIORITY+2));
                            m_sPalSysInfo.ucFW_upgrade = 1;

                            palSystem_Scaler_Upgrade(0);

                            vTaskPrioritySet(m_sPalSysInfo.xTaskHandle, (APP_SYSMON_PRIORITY));
                            m_sPalSysInfo.ucFW_upgrade = 0;
                            halMCU_OPFU_Check_Set(FALSE);
                            halMCU_OPFU_Set(FALSE);
                        }
                        break;

                    case eSYSTEM_UPGRADE_SELECT_DDP:
                        if(m_sPalSysInfo.ePowerState == ePOWER_STATE_UPGRADE)
                        {
                            halMCU_DDP_UpgradeSet(1);

                            LOG_MSG(db_APP_SYSTEM, "DDP upgrade mode\r\n");
                        }
                        break;

                    case eSYSTEM_UPGRADE_DISABLE_UPGRADE_MODE:
                        {
                            halMCU_OPFU_Check_Set(FALSE);
                            halMCU_OPFU_Set(FALSE);
                        }
                        break;
#if 0



                    case eSYSTEM_UPGRADE_SELECT_STARTUP_BASED:
                        {
                            //vTaskPrioritySet(m_sPalSysInfo.xTaskHandle, (APP_SYSMON_PRIORITY+2));
                            m_sPalSysInfo.ucFW_upgrade = 1;
                            //appCLICmd_TaskPriorityHighSet(FALSE);

                            palIapProc_StartupBased_upgrade();

                            //vTaskPrioritySet(m_sPalSysInfo.xTaskHandle, (APP_SYSMON_PRIORITY));
                            m_sPalSysInfo.ucFW_upgrade = 0;
                        }
                        break;

                    case eSYSTEM_UPGRADE_SELECT_FPGA:
                        {
                            UINT8 acUpgrade[2] = {0};
                            UINT8 ucEnable = 1;

                            m_sPalSysInfo.ucFW_upgrade = 1;

                            utilStartupWrite(ePARA_STARTUP_UPGRADE_FPGA_1, (void*)&ucEnable);
                            utilStartupWrite(ePARA_STARTUP_UPGRADE_XFPGA, (void*)&ucEnable);
                            utilStartupUpdate();

                            acUpgrade[0] = 1;
                            acUpgrade[1] = 1;

                            palIapProc_FPGA_Upgrade(acUpgrade);

                            //palIapProc_FPGA_upgrade(eFPGA_INDEX_VIDEO);
                            //palIapProc_FPGA_upgrade(eFPGA_INDEX_LD);

                            m_sPalSysInfo.ucFW_upgrade = 0;
                        }
                        break;

                    case eSYSTEM_UPGRADE_SELECT_All:
                        {
                            UINT32  *pulExtFlashTag = (UINT32*)EXTERNAL_FLASH_TAG_ADDR;
                            UINT8   ucEnable = 1;
                            UINT8   acUpgrade[eFPGA_INDEX_NUMBERS] = {0};
                            UINT8   aucVersion[4] = {0};
                            eRESULT eResut = rcSUCCESS;
                            UINT32* pulExtVer = NULL;
                            UINT8   aucExtVersion[2] = {0};
                            UINT8   aucFrontEndVersion[4] = {0};
                            UINT16  uiFPGA_Ver = 0;
                            UINT16  uiMCU_Ver = 0;

                            //vTaskPrioritySet(m_sPalSysInfo.xTaskHandle, (APP_SYSMON_PRIORITY+2));
                            m_sPalSysInfo.ucFW_upgrade = 1;
                            m_sPalSysInfo.ePowerState = ePOWER_STATE_UPGRADE; //A70LV_Larry_0356

                            palLANProc_UpgradePercentage(0);

                            utilStartupWrite(ePARA_STARTUP_UPGRADE_ICHIP,       (void*)&ucEnable);
                            utilStartupWrite(ePARA_STARTUP_UPGRADE_LPC54113,    (void*)&ucEnable);
                            utilStartupUpdate();

                            if(*pulExtFlashTag == EXTERNAL_FLASH_TAG)
                            {
                                eResut = palSystem_UpgradeLPCMCU(eLPC54113_INDEX_FRONT, FALSE);

                                if(eResut != rcSUCCESS)
                                {
                                    m_sPalSysInfo.ucFW_upgrade = 0;
                                    palLANProc_UpgradePercentage(255);
                                    return;
                                }
                                palLANProc_UpgradePercentage(15);

                                eResut = palSystem_UpgradeLPCMCU(eLPC54113_INDEX_MOTORBD, FALSE);
                                if(eResut != rcSUCCESS)
                                {
                                    m_sPalSysInfo.ucFW_upgrade = 0;
                                    palLANProc_UpgradePercentage(255);
                                    return;
                                }
                                palLANProc_UpgradePercentage(30);

                                eResut = palSystem_UpgradeLPCMCU(eLPC54113_INDEX_LDBD, FALSE);
                                if(eResut != rcSUCCESS)
                                {
                                    m_sPalSysInfo.ucFW_upgrade = 0;
                                    palLANProc_UpgradePercentage(255);
                                    return;
                                }
                                palLANProc_UpgradePercentage(45);

                                //palSystem_VersionCheck(FALSE);
                                palSystem_Scaler_Upgrade();
                                palLANProc_UpgradePercentage(70);

                                //FPGA 0 video
                                LOG_MSG(db_UPGRADE, "FPGA 0 Upgrade Start\r\n");

                                pulExtVer = (UINT32 *)(FW_INFO_ADDR + FW_NAME_STRING_MAXIMUM + sMEM_TAG_PARAM_SIZE*eFW_FPGA_0);

                                aucExtVersion[0] = *pulExtVer & 0xFF;
                                aucExtVersion[1] = (*pulExtVer>>8) & 0xFF;
                                eResut = halFrontEndCtrl_FPGA_Version_Get(aucVersion);
                                eResut &= halFrontEndCtrl_Version_Get(aucFrontEndVersion);

                                if(eResut == rcSUCCESS)
                                {
                                    uiFPGA_Ver = (UINT16)((aucVersion[1] << 8) & 0xFF00) + aucVersion[2];
                                    uiMCU_Ver = (UINT16)((aucFrontEndVersion[1] << 8) & 0xFF00) + aucFrontEndVersion[0];

                                    LOG_MSG(db_UPGRADE, "uiFPGA_Ver 0x%04x\r\n", uiFPGA_Ver);
                                    LOG_MSG(db_UPGRADE, "uiMCU_Ver  0x%04x\r\n", uiMCU_Ver);

                                    if((uiFPGA_Ver >= 15) && (uiMCU_Ver >= 95)) //ver fpga 00.15 mcu 00.95
                                    {
                                        if((aucVersion[1] != aucExtVersion[1]) || (aucVersion[2] != aucExtVersion[0]))
                                        {
                                            acUpgrade[eFPGA_INDEX_VIDEO] = 1;
                                        }
                                        else
                                        {
                                            acUpgrade[eFPGA_INDEX_VIDEO] = 0;
                                        }
                                    }
                                    else
                                    {
                                        acUpgrade[eFPGA_INDEX_VIDEO] = 0;
                                    }
                                }
                                else
                                {
                                    m_sPalSysInfo.ucFW_upgrade = 0;
                                    palLANProc_UpgradePercentage(255);
                                    return;
                                }
                                LOG_MSG(db_UPGRADE, "FPGA 0 Upgrade %d\r\n", acUpgrade[eFPGA_INDEX_VIDEO]);

                                //FPGA 1 system

                                LOG_MSG(db_UPGRADE, "FPGA 1 Upgrade Start\r\n");

                                pulExtVer = (UINT32 *)(FW_INFO_ADDR + FW_NAME_STRING_MAXIMUM + sMEM_TAG_PARAM_SIZE*eFW_FPGA_1);

                                aucExtVersion[0] = *pulExtVer & 0xFF;
                                aucExtVersion[1] = (*pulExtVer>>8) & 0xFF;

                                eResut = halBoard_FPGA2_Version_Get(aucVersion);

                                if(eResut == rcSUCCESS)
                                {
                                    ucEnable = 0;
                                    uiFPGA_Ver = (UINT16)((aucVersion[1] << 8) & 0xFF00) + aucVersion[2];

                                    LOG_MSG(db_UPGRADE, "uiFPGA_Ver 0x%04x\r\n", uiFPGA_Ver);

                                    if(uiFPGA_Ver >= 20)
                                    {
                                        if((aucVersion[1] != aucExtVersion[1]) || (aucVersion[2] != aucExtVersion[0]))
                                        {
                                            ucEnable = 1;
                                        }
                                    }
                                    else
                                    {
                                        ucEnable = 0;
                                    }

                                    LOG_MSG(db_UPGRADE, "FPGA 1 Upgrade %d\r\n", ucEnable);

                                    utilStartupWrite(ePARA_STARTUP_UPGRADE_FPGA_1, (void*)&ucEnable);
                                }
                                else
                                {
                                    m_sPalSysInfo.ucFW_upgrade = 0;
                                    palLANProc_UpgradePercentage(255);
                                    return;
                                }

                                //FPGA 2 LD

                                LOG_MSG(db_UPGRADE, "FPGA 2 Upgrade Start\r\n");

                                pulExtVer = (UINT32 *)(FW_INFO_ADDR + FW_NAME_STRING_MAXIMUM + sMEM_TAG_PARAM_SIZE*eFW_FPGA_2);

                                aucExtVersion[0] = *pulExtVer & 0xFF;
                                aucExtVersion[1] = (*pulExtVer>>8) & 0xFF;
                                eResut = halLDCtrl_Version_Get(aucVersion);

                                if(eResut == rcSUCCESS)
                                {
                                    uiFPGA_Ver = (UINT16)((aucVersion[3] << 8) & 0xFF00) + aucVersion[2];
                                    uiMCU_Ver = (UINT16)((aucVersion[1]  << 8) & 0xFF00) + aucVersion[0];

                                    LOG_MSG(db_UPGRADE, "uiFPGA_Ver 0x%04x\r\n", uiFPGA_Ver);
                                    LOG_MSG(db_UPGRADE, "uiMCU_Ver  0x%04x\r\n", uiMCU_Ver);

                                    if((uiFPGA_Ver >= 9) && (uiMCU_Ver >= 83))  //ver fpga 00.09 mcu 00.83
                                    {
                                        if((aucVersion[3] != aucExtVersion[1]) || (aucVersion[2] != aucExtVersion[0]))
                                        {
                                            acUpgrade[eFPGA_INDEX_LD] = 1;
                                        }
                                        else
                                        {
                                            acUpgrade[eFPGA_INDEX_LD] = 0;
                                        }
                                    }
                                    else
                                    {
                                        acUpgrade[eFPGA_INDEX_LD] = 0;
                                    }
                                }
                                else
                                {
                                    m_sPalSysInfo.ucFW_upgrade = 0;
                                    palLANProc_UpgradePercentage(255);
                                    return;
                                }
                                LOG_MSG(db_UPGRADE, "FPGA 2 Upgrade %d\r\n", acUpgrade[eFPGA_INDEX_LD]);

                                //XFPGA
                                eResut = halBoard_XillinxFPGA_Read(eXFPGA_CMD_BL_VER, &aucVersion[0]);
                                eResut &= halBoard_XillinxFPGA_Read(eXFPGA_CMD_BH_VER, &aucVersion[1]);

                                if(eResut == rcSUCCESS)
                                {
								    ucEnable = 0;
                                    pulExtVer = (UINT32 *)(XLINX_INFO_ADDR + FW_NAME_STRING_MAXIMUM);

                                    aucExtVersion[0] = *pulExtVer & 0xFF;
                                    aucExtVersion[1] = (*pulExtVer>>8) & 0xFF;

                                    LOG_MSG(db_UPGRADE, "XFPGA %02d.%02d(%02d.%02d)\r\n", aucVersion[1], aucVersion[0], aucExtVersion[1], aucExtVersion[0]);

                                    if((aucVersion[1] != aucExtVersion[1]) || (aucVersion[0] != aucExtVersion[0]))
                                    {
                                        ucEnable = 1;
                                    }

                                    if(*pulExtVer == 0xFFFFFFFF)
                                    {
                                        ucEnable = 0;
                                    }

                                    utilStartupWrite(ePARA_STARTUP_UPGRADE_XFPGA, (void*)&ucEnable);

                                    LOG_MSG(db_UPGRADE, "XFPGA Upgrade %d\r\n", ucEnable);
                                }
                                else
                                {
                                    m_sPalSysInfo.ucFW_upgrade = 0;
                                    palLANProc_UpgradePercentage(255);
                                    return;
                                }

                                utilStartupUpdate();

                                palIapProc_FPGA_Upgrade(acUpgrade);

                                palLANProc_UpgradePercentage(90);


                                palIapProc_StartupBased_upgrade();
                                palLANProc_UpgradePercentage(100);
                            }
                            else
                            {
                                palLANProc_UpgradePercentage(255);
                            }

                            ucEnable = 0;
                            utilStartupWrite(ePARA_STARTUP_UPGRADE_ICHIP,       (void*)&ucEnable);
                            utilStartupWrite(ePARA_STARTUP_UPGRADE_LPC54113,    (void*)&ucEnable);
                            utilStartupUpdate();

                            //vTaskPrioritySet(m_sPalSysInfo.xTaskHandle, (APP_SYSMON_PRIORITY));
                            m_sPalSysInfo.ucFW_upgrade = 0;

                            LOG_MSG(db_ALWAYS, "Upgrade All Done\r\n");
                        }

                        break;
#endif /* 0 */

                #if (LOGO_REPLACE == 1) // R70G2_Bruce#0019 //A35G2_Coda_0067
                    case eSYSTEM_UPGRADE_SECOND_LOGO_REPLACE:
                    {// R70G2_Bruce#0023
                        //vTaskPrioritySet(m_sPalSysInfo.xTaskHandle, (APP_SYSMON_PRIORITY+2));
                        m_sPalSysInfo.ucFW_upgrade = 1;
        				appGui_Send_MenuOpen_Logo_Replace_Message(); //A65_OPTOMA_Julie_0082 //A65_OPTOMA_Julie_0079

                        palSystem_SecondLogoPartialUpgrade(1);   //ZU860_Clare_0125
                        UINT8 ucValue = ets_ON;
                        LOG_MSG(db_UPGRADE, "%s() palDataMgr_Access_2nd_Logo_Replaced(%d, %d)\r\n", __FUNCTION__, edaWRITE_THROUGH_WITH_ACTION, ucValue);
        				palDataMgr_Data_Access(edcSECOND_LOGO_REPLACED, edaWRITE_THROUGH_WITH_ACTION, &ucValue);

        				ucValue = palDataMgr_LogoReplace_DisplayCheck(); //A65_OPTOMA_Julie_0079

                        palLANProcSendToLAN(edcSECOND_LOGO_REPLACED);
                        palLANProcSendToLAN(edcSPLASH_STARTUP);

                        //vTaskPrioritySet(m_sPalSysInfo.xTaskHandle, (APP_SYSMON_PRIORITY));
                        m_sPalSysInfo.ucFW_upgrade = 0;
                    }
                        break;

        			case eSYSTEM_UPGRADE_SERVICE_SECOND_LOGO_REPLACE: //A65_OPTOMA_Julie_0076
        			{
                        m_sPalSysInfo.ucFW_upgrade = 1;
        				appGui_Send_MenuOpen_Logo_Replace_Message(); //A65_OPTOMA_Julie_0082 //A65_OPTOMA_Julie_0079

                        palSystem_ServiceCustomLogoPartialUpgrade(1);
                        UINT8 ucValue = ets_ON;
                        LOG_MSG(db_UPGRADE, "(%s,%d) (datacode:%d, value:%d)\r\n", __FUNCTION__, __LINE__, edcSERVICE_SECOND_LOGO_REPLACED, ucValue);
        				palDataMgr_Data_Access(edcSERVICE_SECOND_LOGO_REPLACED, edaWRITE_THROUGH_WITH_ACTION, &ucValue);

        				ucValue = palDataMgr_LogoReplace_DisplayCheck(); //A65_OPTOMA_Julie_0079

                        palLANProcSendToLAN(edcSERVICE_SECOND_LOGO_REPLACED);
                        palLANProcSendToLAN(edcSPLASH_STARTUP);

                        m_sPalSysInfo.ucFW_upgrade = 0;
        			}
        				break;
		        #endif	//ZU860_Clare_0124, add, <<<

                    default:
                        LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d) : fw upgrade select Not, item\r\n", __FUNCTION__, __LINE__);
                        break;
                }

            }
            break;

            //case eSYSTEM_EVENT_BIN2FLASH:
                //palIapProc_BIN2ExtFlash_Set();
                //break;

            case eSYSTEM_EVENT_CUSTOMER_SPLAH:
                //palIapProc_CustomerSplashSet((UINT8)ulParam1);
                break;

            case eSYSTEM_EVENT_LOGO_CAPTURE:
            { //A35G2_BRC_Casper_0037
                BOOL ucCaptureFlag = FALSE;
                if(rcSUCCESS == palSystem_Suspend(__FUNCTION__))
                {
                    if(palDataPath_LogoCapture() == eEXEC_CODE_PASS)
                    {
                        ucCaptureFlag = TRUE;
                    }
                    palSystem_Resume(__FUNCTION__);
                }
                if(ucCaptureFlag)
                {
                    UINT8 ucVal = eCM_CHANGE_LOGO_CAPTURED;

                    palDataMgr_Data_Access(edcLOGO_CHANGE, edaWRITE_THROUGH_WITH_ACTION, &ucVal);   //G100_Owen_0049
                }
                palDataMgr_UI_EventSend(edcUI_EVENT_WAIT_PROCESS_MESSAGE_EXIT, 2, NULL);
            }
                break;

            case eSYSTEM_EVENT_SCREEN_SAVE_TO_IMAGE://R70G2_Bruce#0022
                if(rcSUCCESS == palSystem_Suspend(__FUNCTION__))
                {
                    if(palDataPath_ScreenSaveToImage() == eEXEC_CODE_PASS)
                    {

                    }
                    palSystem_Resume(__FUNCTION__);
                }
                break;

            case eSYSTEM_EVENT_ENTER_2W_MODE: //G100_Larry_0022
                LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d) Enter 2W mode\r\n", __FUNCTION__, __LINE__);
                SYSTEM_CALL("echo mem > /sys/power/state");
                LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d) Exit 2W mode\r\n", __FUNCTION__, __LINE__);
                break;


            case eSYSTEM_EVENT_CUSTOMIZE_EDID:  //A35G2_Simon_0091
                {
                    //if(rcSUCCESS == palSystem_Suspend(__FUNCTION__))
                    {
                        if(palSystem_WriteCustomizedEDID((eCM_SOURCE_ID)ulParam1) != rcSUCCESS)
                        {
                            LOG_MSG(db_APP_DATAPATH, "Write Customize EDID Fail\n");
                        }
                        else
                        {
                            LOG_MSG(db_APP_DATAPATH, "Write Customize EDID Done\n");
                        }

                        //palSystem_Resume(__FUNCTION__);
                    }

                    //GuiCb.fpGui_Send_WaitProcessMessageExitCb(2);
                }
                break;

            case eSYSTEM_EVENT_APPLY_AP_MODE_GEO_FUNC:   //A35G2_Simon_0100
                palSystem_Apply_AP_Mode_GeoFunc();
                break;

           case eSYSTEM_EVENT_ACTUATOR_PATTERN:
                break;

            default:
                break;

    }
}

static void palSystem_DefaultDataConfig(void)//gpConfiguration
{
    m_sDefSysConfiguration.System.ucStartupState = eSTARTUP_STATE_NORMAL;

    m_sDefSysConfiguration.System.ucDefaultPanelId = PANEL_2D_OUTPUT;

    m_sDefSysConfiguration.Environment.ucEnableEnvironmentPoll = TRUE;
    m_sDefSysConfiguration.Environment.ucCheckFanLock = TRUE;
    m_sDefSysConfiguration.Environment.ucCheckOvertemp = TRUE;
    m_sDefSysConfiguration.Environment.ucInstalledFans = 3;

    m_sDefSysConfiguration.Datapath.uiSplashAtStartupTimeout = 2;//Second

    m_sDefSysConfiguration.Illum.uiCooldownDelay = Syscfg_Value_Get_Typeint(eCooling_Time);//Second

}


static void palSystem_Reset(void)
{
    LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);

    //utilUI_CallBack_FuncInit();
    GuiCb = Gui_fpCallbackGet();


    Init_GEC_Callback();
    Init_Formatter_Callback();
    Init_Database_Callback();
    Init_HPBUTester_Callback();
    Init_IPC_Callback();
    Init_WarpDemo_Callback();
    Init_CLICmd_Callback();
    //Init_CommonUserCLI_Callback();


    //TODO
    /*
    Init System Releated, including I2C, SPI and etc.
    */

    //Try to use one chip concept to save parameters
    palSystem_PanelID_Set((ePANEL_ID)m_sDefSysConfiguration.System.ucDefaultPanelId);

    if(palCoreVar_Init() == eEXEC_CODE_FAIL)
    {
        LOG_MSG(db_ALWAYS, "(func:%s, line:%d): Core var Init Fail!\r\n", __FUNCTION__, __LINE__);
        palSystem_Fault();
    }

    if(palDataMgr_Init() == eEXEC_CODE_FAIL)
    {
        LOG_MSG(db_ALWAYS, "(func:%s, line:%d): DataMgr Init Fail!\r\n", __FUNCTION__, __LINE__);
        palSystem_Fault();
    }
    LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d):\r\n", __FUNCTION__, __LINE__);

    if(palEnvironment_Init(&m_sDefSysConfiguration) == eEXEC_CODE_FAIL)
    {
        LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d): Environment Init Fail!\r\n", __FUNCTION__, __LINE__);
        palSystem_Fault();
    }
    LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d):\r\n", __FUNCTION__, __LINE__);

#if 0//A70LV_Doulas_2000 remove
    if(appChip_init() == eEXEC_CODE_FAIL)
    {
        LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d): Chip Init Fail!\r\n", __FUNCTION__, __LINE__);
        palSystem_Fault();
    }
#endif

    if(palDataPath_Init(m_sPalSysInfo.ePanelTimingId, &m_sDefSysConfiguration) == eEXEC_CODE_FAIL)
    {
        LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d): Datapath Init Fail!\r\n", __FUNCTION__, __LINE__);
        palSystem_Fault();
    }

    if(palDataPath_InitSub(m_sPalSysInfo.ePanelTimingId, &m_sDefSysConfiguration) == eEXEC_CODE_FAIL)   //A70LV_Doulas_0003
    {
        LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d): Datapath sub Init Fail!\r\n", __FUNCTION__, __LINE__);
        palSystem_Fault();
    }

    //import data to shared memory
    palDataMgr_ImportAllDataToDatabase();
    LOG_MSG(db_UTL_DATABASE, "Database created !\r\n");
    #if 0
    if(utilDatabase_Init() == -1)
    {
        LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d): Database Init Fail!\r\n", __FUNCTION__, __LINE__);
    }
    else
    {
        palDataMgr_ImportAllDataToDatabase();
        LOG_MSG(db_UTL_DATABASE, "Database created !\r\n");
    }
    #endif


    //init IPC
    if(utilIpc_Init() == eEXEC_CODE_FAIL)
    {
        LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d): IPC Init Fail!\r\n", __FUNCTION__, __LINE__);
        palSystem_Fault();
    }

    if(palInputProc_Init() == eEXEC_CODE_FAIL)
    {
        LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d): Input Init Fail!\r\n", __FUNCTION__, __LINE__);
        palSystem_Fault();
    }

    if(palIllumination_Init() == eEXEC_CODE_FAIL)
    {
        LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d): Illumination Init Fail!\r\n", __FUNCTION__, __LINE__);
        palSystem_Fault();
    }

    if(palLANProc_Init() == eEXEC_CODE_FAIL)
    {
        LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d): LAN Init Fail!\r\n", __FUNCTION__, __LINE__);
        palSystem_Fault();
    }

    if(appPoll_Init() == eEXEC_CODE_FAIL)
    {
        LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d): Poll Init Fail!\r\n", __FUNCTION__, __LINE__);
        palSystem_Fault();
    }

    if(appCLICmd_Init() == eEXEC_CODE_FAIL)
    {
        LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d): CLI Cmd Init Fail!\r\n", __FUNCTION__, __LINE__);
        palSystem_Fault();
    }

    if(palHostPath_Init() == eEXEC_CODE_FAIL)
    {
        LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d): Host Path Init Fail!\r\n", __FUNCTION__, __LINE__);
        palSystem_Fault();
    }

    LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d):\r\n", __FUNCTION__, __LINE__);
}


static void palSystem_Process(void)
{
    switch(m_sPalSysInfo.ePowerState)
    {
        case ePOWER_STATE_RESET:
            break;

        case ePOWER_STATE_STANDBY:
            if((m_sPalSysInfo.ulSystemCount % 5000) == 0)
            {
                UINT8 ucState = 0;

#ifndef SIMULATOR_ISCALER
                if(rcSUCCESS == halMCUCtrl_SystemState_Get(&ucState))
                {
                    if(m_sPalSysInfo.ucSystemMcuState != ucState)
                    {
                        switch(ucState)
                        {
                            case eSYSTEM_STATE_COMMUNICATION: //HICC2_Doulas_0078
                                halBoard_CameraEnableSet(TRUE);
                                break;
                            case eSYSTEM_STATE_WARMUP:
                            case eSYSTEM_STATE_POWERON:
                            case eSYSTEM_STATE_UPGRADE:
                                {
                                    UINT8 ucResetMode = 0;

                                    if(rcSUCCESS == halMCU_Factory_Reset_Get(&ucResetMode))
                                    {
                                        if(ucResetMode == FACTORY_RESET_NUMBER)
                                        {
                                            palDataMgr_ResetAllToDefault();
					                        utilOPD_ResetAllToDefault();
                                        }
                                    }

                                    palSystem_WarmUp();
                                    palSystem_HostReadySet(1);
                                    LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d):\r\n", __FUNCTION__, __LINE__);
                                }
                                break;

                            case eSYSTEM_STATE_COOLING:
                                //palSystem_PowerDown(); //避免與system 54605不同步 Larry
                                LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d):\r\n", __FUNCTION__, __LINE__);
                                break;

                            default:
                                break;
                        }

                        m_sPalSysInfo.ucSystemMcuState = ucState;
                    }

                    LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d): Mcu State %d\r\n", __FUNCTION__, __LINE__, ucState);
                }
                else
                {
                    LOG_MSG(db_APP_SYSTEM, "halMCUCtrl_SystemState_Get Fail\r\n");
                }
#else
                palSystem_WarmUp();
#endif /* SIMULATOR_ISCALER */
            }
            break;

        case ePOWER_STATE_WARMUP:
            break;

        case ePOWER_STATE_ACTIVE:
            break;

        case ePOWER_STATE_COOLING:
            if((m_sPalSysInfo.ulSystemCount % 5000) == 0)
            {
                UINT8 ucState = 0;

                if(rcSUCCESS == halMCUCtrl_SystemState_Get(&ucState))
                {
                    m_sPalSysInfo.ucSystemMcuState = ucState;
                }
                LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d): Mcu State %d\r\n", __FUNCTION__, __LINE__, ucState);
            }
            break;

        case ePOWER_STATE_UPGRADE:
            break;
    }
}


#if (SYSTEM_OS_TYPE == POSIX_COMPLIANT)
pthread_t *palSystem_GetTaskHandle(void)
{
    return &m_sPalSysInfo.xTaskHandle;
}
pthread_attr_t *palSystem_GetTaskAttr(void)
{
    return &m_sPalSysInfo.xTaskHandle_attr;
}
#else  //freertos
TaskHandle_t *palSystem_GetTaskHandle(void)
{
    return &m_sPalSysInfo.xTaskHandle;
}
#endif

static void palSystem_SetCfgAddr(void)
{
    //Roger Commented 20160708
    //Must put in step 1
    palSystem_DefaultDataConfig();
    palDataMgr_DefaultSettingConfig();//TODO
    palSystem_DefaultValueInit();
}

void* palSystem_SysMonTask(void *pParameters)
{
    eMAIL_BOX_EXEC_CODE eExecCode;
    psEEPROM_SETTINGS pDefEEP = NULL;
    UINT32 *pulExtFlashTag = NULL; //A70LV_Larry_0118
    UINT32 *pulCustomer_Splash  = NULL; //A70LV_Larry_0235
    UINT32 ulData = 0;

    m_sPalSysInfo.ucFW_upgrade = 0;

    m_bNoFanControlFlag = FALSE;

    m_sPalSysInfo.ucCustomer_Splash = CUSTOM_LOGO_CHRISTIE;

    pDefEEP = palDataMgr_GetDefaultSettingConfig();

	#if 0 //Depends on X35Gen2 design
	palDataMgr_Check_ModelID();		//G100_Doulas_0080
	#endif
	#if 0
	palDataMgr_SKUTypeSet();		//G100_Doulas_0080
	#endif
    palSystem_SetCfgAddr();

    //if(utilDbgMsg_Init(pDefEEP->sSystemDefault.ulDbMask) == eEXEC_CODE_FAIL)    //A70LV_Doulas_0015 modify
    //{
        //LOG_MSG(db_ALWAYS, "(func:%s, line:%d): DbgMsg Init Fail!\r\n", __FUNCTION__, __LINE__);
        //palSystem_Fault();
    //}

    LOG_MSG(db_APP_SYSTEM, "\r\nFW Ver:(V%02d.%02d)\r\n", VER_MAJOR, VER_MINOR);  //show FW version

    LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);

    LOG_MSG(db_APP_SYSTEM, "m_Customer_logo :%d \r\n", m_sPalSysInfo.ucCustomer_Splash); //A70LV_Larry_0235

    //Create Thread Pasue
    if(pthread_mutex_init(&m_sPalSysInfo.xTaskMutex, NULL) != 0)
    {
        LOG_MSG(db_ALWAYS, "(func:%s, line:%d): Create xTaskMutex Fail!\r\n", __FUNCTION__, __LINE__);
    }
    m_sPalSysInfo.bTaskPause = FALSE;

#if (SYSTEM_OS_TYPE == POSIX_COMPLIANT)

    LOG_MSG(db_APP_SYSTEM, "System queue create start\n");

    if(palMailBox_Create_MsgQueue(&m_sPalSysInfo.xMsgQueue, QUEUE_SYSTEM_NAME, &m_sPalSysInfo.xMsgQueue_attr) != eMAIL_BOX_EXEC_CODE_PASS)
    {
        ASSERT_ALWAYS();
        LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d): Create Msg Queue Fail!\r\n", __FUNCTION__, __LINE__);
        return eEXEC_CODE_FAIL;
    }

    LOG_MSG(db_APP_SYSTEM, "System queue create end\n");

#else

    if(palMailBox_Create(&m_sPalSysInfo.xMsgQueue, &m_sPalSysInfo.xEventGroupHandle) != eMAIL_BOX_EXEC_CODE_PASS)
    {
        /* Queue was not created and must not be used. */
        LOG_MSG(db_ALWAYS, "(func:%s, line:%d): Create Msg Queue Fail!\r\n", __FUNCTION__, __LINE__);
        palSystem_Fault();
    }

#endif

    LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);

    palSystem_Reset();

    LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);

    m_sPalSysInfo.ePowerState = ePOWER_STATE_RESET;
    palSystem_HostReadySet(0);

    palLANProcPowerStateSend(ePOWER_STATE_RESET, 4, (UINT8*)&ulData);
    palLANProc_Reset();

    //MS_SLEEP( 2000 );
    //vTaskPrioritySet()
    palSystem_SetSysEvent(eSYSTEM_EVENT_IDLE);
    //palSystem_SetSysEvent(eSYSTEM_EVENT_START);

    Board_ModelID_Set(MODEL_ID_0);
    m_sPalSysInfo.ucSystemMcuState = 0;
    m_sPalSysInfo.ulSystemCount = 0;

    for(;;)
    {
#if (SYSTEM_OS_TYPE == POSIX_COMPLIANT)
        if((eExecCode = palMailBox_Receive_MsgQueue(m_sPalSysInfo.xMsgQueue,
                                                    QUEUE_SYSTEM_NAME,
                                                    1,
                                                    palSystem_CallBack)) != eMAIL_BOX_EXEC_CODE_PASS)
#else
        if((eExecCode = palMailBox_Receive(m_sPalSysInfo.xMsgQueue, 1, palSystem_CallBack)) != eMAIL_BOX_EXEC_CODE_PASS)
#endif
        {
            if(eExecCode == eMAIL_BOX_EXEC_CODE_NOMSG)
            {
                m_sPalSysInfo.ulSystemCount++;
                palSystem_Process();
            }
            else
            {
                LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d): Receive System Event Error ID [%d]!\r\n", __FUNCTION__, __LINE__, eExecCode);
            }
        }

        palSystem_TaskMonitor();  //A35G2_Simon_0075
    }
}

void palSystem_ChangePanelID(ePANEL_ID ePanelId, sSYSTEM_CHANGEPANELID_INFO *psInfo)   //A70LV_Doulas_0005
{
    UINT8 ucPIP_EN = ets_OFF;
	UINT8  ucData;		//G100_Doulas_0027
    LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d) Panel %d\r\n", __FUNCTION__, __LINE__, ePanelId);

    if(halC789Ctrl_TwistLinkFlag_Get() == ets_ON)                                       //ZU860_Doulas_0138
    {
        return;
    }

    //halC789Ctrl_OutputEnableSet(0);

    //Try to use one chip concept to save parameters
    palSystem_PanelID_Set(ePanelId);
    palEnvironment_LightSourceBlankingOn_Set(TRUE,10);   //A70LV_Doulas_0294
    palDataMgr_Data_Access(edcPIP_PBP_ENABLE, edaREAD, &ucPIP_EN);      //A70LV_Doulas_0156
    if((ucPIP_EN) &&                                                                    //G100_Owen_0012
       (ePanelId == ePANEL_ID_1080P_120HZ || ePanelId == ePANEL_ID_WUXGA_120HZ))         //A70LV_Doulas_0377 Modify//A70LV_Doulas_0156
    {
        ucPIP_EN = ets_OFF;
        palDataMgr_Data_Access(edcPIP_PBP_ENABLE, edaWRITE_THROUGH_WITH_ACTION, &ucPIP_EN);
    }
	#ifdef Low_Latency_All
	if((ePanelId == ePANEL_ID_1080P_120HZ || ePanelId == ePANEL_ID_WUXGA_120HZ || FrontEnd_Down_Scaling_Get()) && halScaler_Low_Latency_Get())	//G100_Clare_0053
    {
    	UINT8 ucVlaue = FALSE;
		halFormatter_FRCByPassModeSet(FALSE);    //A35G2_CDS_Simon_0004
		palDataMgr_Data_Access(edcLOW_LATENCY_MODE, edaWRITE_THROUGH_NO_ACTION, &ucVlaue);
		palDataPath_Low_Latency_Set(FALSE);
		halC789Ctrl_Change_Panel(ePanelId);
	}
	#endif	/*Low_Latency_All*/

#if 0
    if(ePanelId == PANEL_3D_OUTPUT)
    {
        halFormatter_XPR_Set(0);
    }
    else
    {
        halFormatter_XPR_Set(1);
    }
#endif
    if(palEnvironment_LightSourceBlankingEn_Get()) //A70LV_Doulas_0300
    {
        palImgMgr_WB_OutputEnableSet(0);     //A70LV_Doulas_0216
    }
    if(palDataPath_Panel_Change(m_sPalSysInfo.ePanelTimingId) == eEXEC_CODE_FAIL)
    {
        LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d): palDataPath_Panel_Change Fail!\r\n", __FUNCTION__, __LINE__);
       // palSystem_Fault();
    }
    MS_SLEEP(200);          //A70LV_Doulas_0154 Add
    if(palDataPath_Panel_ChangeSub(m_sPalSysInfo.ePanelTimingId) == eEXEC_CODE_FAIL)
    {
        LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d): palDataPath_Panel_ChangeSub  Fail!\r\n", __FUNCTION__, __LINE__);
       // palSystem_Fault();
    }
    palLANProcSendToLAN(edcCOLOR_WHEEL_SPEED);		//A65_OPTOMA_Doulas_0131//A35G2_Coda_0110
    //GuiCb.fpGui_PanelChangeCb(ePanelId); //A70LV_Larry_0173
    palLANProcSendToLAN(edc3D_CONFIG_TYPE);
    palDataMgr_UI_EventSend(edcUI_EVENT_PANEL_CHANGE, ePanelId, NULL);
    halAdvWarpVstartOffsetSet(ePanelId);
    palGeo_AdvWarpingInit(m_sPalSysInfo.ePanelTimingId);									//G100_Doulas_0027 Add
    palDataMgr_Data_Access(edcADV_WARP_CONTROL, edaREAD, &ucData);							//G100_Doulas_0027 Add
    if(ucData == WARP_CTRL__ADVANCED)											//G100_Doulas_0061 Modify//G100_Doulas_0060 Modify ,change panel too long
	{
		//palGeo_AdvWarpingInit(palSystem_PanelID_Get());
    	palGeo_AdvWarpControl(ucData);

    	//apply black level
    	UINT8 ucApplyIndex = 0; //m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sWarpSetting.ucWarpingApplySetting;
		palDataMgr_Data_Access(edcWARP_MEMORY_APPLY, edaREAD, &ucApplyIndex);

		#if (CMD_ONLY_BLACK_LEVEL == TRUE)
    	palGeo_Func_Set(eGFN_CLI_BKLV_RESET, NULL);

        uPALGEOAPI_INFO uInfo = {.sGFN_CLI_BKLV_LOAD_DATA_INFO.Index = BLACKLEVEL_CURRENT_INDEX};
        palGeo_Func_Set(eGFN_CLI_BKLV_LOAD_DATA, &uInfo);
    	#endif
	}
	else
	{
		palDataMgr_Data_Access(edcADV_WARP_CONTROL, edaWRITE_RAM_ONLY_WITH_ACTION, &ucData);	//G100_Doulas_0027 Add
	}
    if(palEnvironment_LightSourceBlankingEn_Get()) //A70LV_Doulas_0300
    {
        palImgMgr_WB_OutputEnableSet(1);     //A70LV_Doulas_0216
    }

}

// A70LV_Eric.C_0021 Start //A70LV_Larry_0097 modify
BOOL appSysInitAfterAsicReady(void)
{
    if (Board_Stage_Get() > EVT_STAGE)
    {
        if(m_bIsAsicInitialed == FALSE)
        {
            if(!halDDP4422_IsAsicReady())
            {
                if(m_bAsicReadyCheckCount > 0)  //G100_Simon_0064
                {
                    if(m_bAsicReadyCheckCount % 10 == 0)
                    {
                        halMCU_DDP_PowerSet(1);
                    }
                    m_bAsicReadyCheckCount--;

                    if(0 == m_bAsicReadyCheckCount)
                    {
    					UINT32 ulErrorIndex = 0 ;
    					ulErrorIndex = DDPFail;

    					utilDataMgr_WriteGecLog_CM(ulErrorIndex);	//HICC2_Doulas_0003
                        LOG_MSG(db_HAL_FORMATTER, "ASIC Not Ready\r\n");

                        if(palSystem_OPFUCheck() == TRUE)   //G100_Simon_0064
                        {
                            LOG_MSG(db_UPGRADE, "\n\n---> ASIC Not Ready go to upgrade mode\n\n");
        					LOG_MSG(db_UPGRADE, "\r\n(func:%s, line:%d) Entering OPFU Mode\r\n", __FUNCTION__, __LINE__);
                            palSystem_FwUpgradeMode(eSYSTEM_UPGRADE_MODE_SET);
                        }
                        else
                        {
                            halMCU_DDP_PowerSet(1);
                        }
                    }
                }
                LOG_MSG(db_HAL_FORMATTER, "Waiting for ASIC Ready (%d)\r\n", m_bAsicReadyCheckCount);
                return FALSE; //A70LV_Larry_0097
            }
            //else if(m_bAsicReadyCheckCount > 0) //HICC2_Doulas_0060 remove//HICC2_Doulas_0059 avoid palDataMgr_Data_Access() Mutex lock error
            //{
            //    LOG_MSG(db_UPGRADE, "===> ASIC Ready count %d\n",m_bAsicReadyCheckCount);
            //    m_bAsicReadyCheckCount--;
            //}
            else
            {
                LOG_MSG(db_UPGRADE, "\n\n===> ASIC Ready \n\n"); //HICC2_Doulas_0060

                //clear OPFU Check
                UINT8 ucData = FALSE;
                //palDataMgr_Data_Access(edcOPFU_CHECK, edaWRITE_THROUGH_WITH_ACTION, &ucData);   //HICC2_Doulas_0060 remove//G100_Simon_0064

                //A70LV_Doulas_0077 read DDP version
                UINT16 wVerDDPApp = 0;
                UINT16 wVerDDPApp_compare = 0; //A70LV_John_0019 add ddp version checking mechanism
                UINT32 dwVerDDPSq = 0;
                UINT32 wVerDDPSq_compare = 0; //A70LV_Larry_0379
                UINT8  aucVerString[32] = {0};
                UINT8 ucRetry = 2; //A70LV_John_0019 add ddp version checking mechanism

				#ifdef CUSTOM_OPTOMA		//A65_OPTOMA_Doulas_0069 //A35G2_Coda_0062
				utilOptoma_OutputInfoMsg(sOptoma_System_Auto_Send_Lut[eOPT_WARMING_UP].cINFO_String);
				#endif
                //A70LV_John_0019 start add ddp version checking mechanism
                while (ucRetry > 0)
                {
                    MS_SLEEP(50);
                    halFormatter_SYS_AppVersionGet(&wVerDDPApp);
                    if(wVerDDPApp == wVerDDPApp_compare)
                    {
                        ucRetry --;
                    }
                    wVerDDPApp_compare = wVerDDPApp;
                }
                //A70LV_John_0019 end

                ucRetry = 2;

                while (ucRetry > 0) //A70LV_Larry_0379
                {
                    MS_SLEEP(50);
                    halFormatter_SEQ_RevisionGet(&dwVerDDPSq);
                    if(dwVerDDPSq == wVerDDPSq_compare)
                    {
                        ucRetry --;
                    }
                    wVerDDPSq_compare = dwVerDDPSq;
                }


                LOG_MSG(db_APP_SYSTEM, "DDP      F%02d.%02d(%c%c%c%d)\n",
                    (UINT8)(wVerDDPApp>>8),      //DDP442x       Fxx.xx(HDRxx)
                    (UINT8)(wVerDDPApp),
                    (UINT8)(dwVerDDPSq>>24),
                    (UINT8)(dwVerDDPSq>>16),
                    (UINT8)(dwVerDDPSq>>8),
                    (UINT8)(dwVerDDPSq));

                sprintf((char*)aucVerString, "F%02d.%02d(%c%c%c%d)" , //HICC2_Doulas_0022
                    (UINT8)(wVerDDPApp>>8),      //DDP442x       Fxx.xx(HDRxx)
                    (UINT8)(wVerDDPApp),
                    (UINT8)(dwVerDDPSq>>24),
                    (UINT8)(dwVerDDPSq>>16),
                    (UINT8)(dwVerDDPSq>>8),
                    (UINT8)(dwVerDDPSq));
                palDataMgr_Data_Access(edcFORMATER_VERSION, edaWRITE_RAM_ONLY_NO_ACTION, aucVerString);


                LOG_MSG(db_HAL_FORMATTER, "ASIC Ready\r\n");        //A70LV_Doulas_0046 Add

                //clear OPFU Check
                palDataMgr_Data_Access(edcOPFU_CHECK, edaWRITE_THROUGH_WITH_ACTION, &ucData); //HICC2_Doulas_0076

                halBoard_CameraEnableSet(TRUE); //G100_Larry_0029
                m_bIsAsicInitialed = TRUE;
            }
        }
    }
    else
    {
        if(m_bIsAsicInitialed == FALSE)
        {
            UINT8  uciScalerOpfuCheck = FALSE;

            //clear OPFU Check //for single board debug
            palDataMgr_Data_Access(edcOPFU_CHECK, edaWRITE_THROUGH_WITH_ACTION, &uciScalerOpfuCheck);

            m_bIsAsicInitialed = TRUE;
        }
    }

    return m_bIsAsicInitialed; //A70LV_Larry_0097
}
// A70LV_Eric.C_0021 End


// ==============================================================================
// FUNCTION NAME: palSystem_NoFanControlFlagGet
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
BOOL palSystem_NoFanControlFlagGet(void)
{
    return m_bNoFanControlFlag;
}

// ==============================================================================
// FUNCTION NAME: palSystem_VersionCheck
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
// 2018/02/23, Larry Create
// --------------------
// ==============================================================================

#if defined(PLATFORM_A70G2) || defined(PLATFORM_H60_2K)
eRESULT palSystem_VersionCheck(BOOL bAutoUpgrade)
{

    UINT8 aucVersion[8] = {0};
    UINT8 aucVerString[32] = {0};  //G100_Wilsonj_0069
    eRESULT eResult = rcSUCCESS;
    eRESULT eResultAll = rcSUCCESS;
    UINT8 cFPGA_Model = 0;
	INT32 ulWith_MCU = 0;

    //Version check

    //FMT LD
    Syscfg_Value_Get(eWith_LDDRV_MCU,&ulWith_MCU); //HICC2_Julie_0055
    if(ulWith_MCU)
    {
        memset(aucVersion, 0, sizeof(aucVersion));
        #if 0 //Depends on X35Gen2 design
        eResult = halFrontEndCtrl_Version_Get(aucVersion);
        #else
        eResult = halLDCtrl_Version_Get(aucVersion);
        #endif

        if(eResult == rcSUCCESS)
        {
            LOG_MSG(db_APP_SYSTEM, "FMT   L%02d.%02d\n", aucVersion[1], aucVersion[0]);

            sprintf((char*)aucVerString, "L%02d.%02d", aucVersion[1], aucVersion[0]);
            palDataMgr_Data_Access(edcLD_DRIVER_VERSION, edaWRITE_RAM_ONLY_NO_ACTION, aucVerString);

            if(strcmp(aucVerString, "L00.00") == 0 ||   //G100_Simon_0077
            strcmp(aucVerString, "LFF.FF") == 0 )
            {
                LOG_MSG(db_APP_SYSTEM, "FMT version error %s\n", aucVerString);
                eResult = rcERROR;
            }
        }
        eResultAll &= eResult;
    }

	palDataMgr_Model_ID_Set(); //A70Gen2_Julie_0004

    //FMT LD Convert
    #if 0
    memset(aucVersion, 0, sizeof(aucVersion));
    eResult = halLDCtrl_ConvertVersion_Get(aucVersion);

    if(eResult == rcSUCCESS)
    {
        LOG_MSG(db_APP_SYSTEM, "FMT convert D%02d.%02d\n", aucVersion[1], aucVersion[0]);

        sprintf((char*)aucVerString, "D%02d.%02d", aucVersion[1], aucVersion[0]); //A70Gen2_Julie_0024
        palDataMgr_Data_Access(edcSMCU_VERSION, edaWRITE_RAM_ONLY_NO_ACTION, aucVerString);

        if(strcmp(aucVerString, "D00.00") == 0 ||   //G100_Simon_0077
           strcmp(aucVerString, "DFF.FF") == 0 )
        {
            LOG_MSG(db_APP_SYSTEM, "FMT convert version error %s\n", aucVerString);
            eResult = rcERROR;
        }
    }
    eResultAll &= eResult;
    #endif

    //System
    Syscfg_Value_Get(eWith_System_MCU,&ulWith_MCU); //HICC2_Julie_0055
    if(ulWith_MCU)
    {
        memset(aucVersion, 0, sizeof(aucVersion));
        eResult = halMCUCtrl_Version_Get(aucVersion);

        if(eResult == rcSUCCESS)
        {
            LOG_MSG(db_APP_SYSTEM, "MCU A%02d.%02d\n", aucVersion[1], aucVersion[0] );

            sprintf((char*)aucVerString, "A%02d.%02d", aucVersion[1], aucVersion[0]);
            palDataMgr_Data_Access(edcFMCU_VERSION, edaWRITE_RAM_ONLY_NO_ACTION, aucVerString);

            if(strcmp(aucVerString, "A00.00") == 0 ||   //G100_Simon_0077
            strcmp(aucVerString, "AFF.FF") == 0 )
            {
                LOG_MSG(db_APP_SYSTEM, "MCU version error %s\n", aucVerString);
                eResult = rcERROR;
            }
        }
        eResultAll &= eResult;
    }

	//Front end
    Syscfg_Value_Get(eWith_Frontend_MCU,&ulWith_MCU); //HICC2_Doulas_0010 //HICC2_Julie_0055
	if(ulWith_MCU) //HICC2_Doulas_0010
	{
		memset(aucVersion, 0, sizeof(aucVersion));
		eResult = halFrontEndCtrl_Version_Get(aucVersion);

		if(eResult == rcSUCCESS)
		{
			LOG_MSG(db_APP_SYSTEM, "FrontEnd E%02d.%02d\n", aucVersion[1], aucVersion[0] );

			sprintf((char*)aucVerString, "E%02d.%02d", aucVersion[1], aucVersion[0]);
			palDataMgr_Data_Access(edcFRONTEND_VERSION, edaWRITE_RAM_ONLY_NO_ACTION, aucVerString);

	        if(strcmp(aucVerString, "E00.00") == 0 ||
	            strcmp(aucVerString, "EFF.FF") == 0 )
	        {
	            LOG_MSG(db_APP_SYSTEM, "FrontEnd version error %s\n", aucVerString);
	            eResult = rcERROR;
	        }
		}
	    eResultAll &= eResult;
	}

	//Keypad
    memset(aucVersion, 0, sizeof(aucVersion));
    eResult = halMCUCtrl_KeypadVersion_Get(aucVersion);
    if(eResult == rcSUCCESS)
    {
        LOG_MSG(db_APP_SYSTEM, "Keypad   K%02d.%02d\n", aucVersion[1], aucVersion[0] );

        sprintf((char*)aucVerString, "K%02d.%02d", aucVersion[1], aucVersion[0]);
        palDataMgr_Data_Access(edcKEYPAD_VERSION, edaWRITE_RAM_ONLY_NO_ACTION, aucVerString);

        if(strcmp(aucVerString, "K00.00") == 0 ||   //G100_Simon_0077
           strcmp(aucVerString, "KFF.FF") == 0 )
        {
            LOG_MSG(db_APP_SYSTEM, "Keypad version error %s\n", aucVerString);
            eResult = rcERROR;
        }
    }
    eResultAll &= eResult;

    //SYS FPGA
    Syscfg_Value_Get(eWith_FPGA0_MCU,&ulWith_MCU); //HICC2_Julie_0055
    if(ulWith_MCU)
    {
        memset(aucVersion, 0, sizeof(aucVersion));
        eResult = halMCUCtrl_FPGA1_Version_Get(aucVersion);

        if(eResult == rcSUCCESS)
        {
            LOG_MSG(db_APP_SYSTEM, "FPGA1   X%02d.%02d\n", aucVersion[1], aucVersion[0]);

            sprintf((char*)aucVerString, "X%02d.%02d", aucVersion[1], aucVersion[0]);

            if(aucVersion[2] == 'A')
            {
                cFPGA_Model = 1; //FPGA is 10M08
            }
            else
            {
                cFPGA_Model = 0; //FPGA is 10M04
            }

            palDataMgr_Data_Access(edcFPGA1_VERSION, edaWRITE_RAM_ONLY_NO_ACTION, aucVerString);

            if(strcmp(aucVerString, "X00.00") == 0 ||   //G100_Simon_0077
            strcmp(aucVerString, "XFF.FF") == 0 )
            {
                LOG_MSG(db_APP_SYSTEM, "FPGA1 version error %s\n", aucVerString);
                eResult = rcERROR;
            }
        }
        eResultAll &= eResult;
    }

	//FMT FPGA
    Syscfg_Value_Get(eWith_FPGA2_MCU,&ulWith_MCU); //HICC2_Julie_0055
    if(ulWith_MCU)
    {
        memset(aucVersion, 0, sizeof(aucVersion));
        eResult = halMCUCtrl_FPGA3_Version_Get(aucVersion);

        if(eResult == rcSUCCESS)
        {
            LOG_MSG(db_APP_SYSTEM, "FPGA3   Z%02d.%02d\n", aucVersion[1], aucVersion[0]);

            sprintf((char*)aucVerString, "Z%02d.%02d", aucVersion[1], aucVersion[0]);
            palDataMgr_Data_Access(edcFPGA3_VERSION, edaWRITE_RAM_ONLY_NO_ACTION, aucVerString);

            if(strcmp(aucVerString, "Z00.00") == 0 ||   //G100_Simon_0077
                strcmp(aucVerString, "ZFF.FF") == 0 )
            {
                LOG_MSG(db_APP_SYSTEM, "FPGA3 version error %s\n", aucVerString);
                eResult = rcERROR;
            }
        }
        eResultAll &= eResult;
    }

	//HW version
    memset(aucVersion, 0, sizeof(aucVersion));
    eResult = halMCUCtrl_HWVersion_Get(aucVersion);

    if(eResult == rcSUCCESS)
    {
        LOG_MSG(db_APP_SYSTEM, "HW version %02d\n", aucVersion[0]);

        //if(cFPGA_Model)
        //{
        //    aucVersion[0] = aucVersion[0] + eHWVER_M08_VERA;
        //}

        palDataMgr_Data_Access(edcHW_PROJECTOR_ID, edaWRITE_RAM_ONLY_NO_ACTION, aucVersion);
    }

    eResultAll &= eResult;

    //XFPGA Version
    Syscfg_Value_Get(eWith_Xilinx_FPGA,&ulWith_MCU); //HICC2_Julie_0055
    if(ulWith_MCU)
    {
        eResult  = halBoard_XillinxFPGA_Version_BL_Get(&aucVersion[0]); //G100_Simon_0059   //A35G2_Simon_0065
        eResult &= halBoard_XillinxFPGA_Version_BH_Get(&aucVersion[1]);  //A35G2_Simon_0065

        if(eResult == rcSUCCESS)
        {
            LOG_MSG(db_APP_SYSTEM, "XFPGA  Q%02d.%02d\n", aucVersion[1], aucVersion[0]);
            sprintf((char*)aucVerString, "Q%02d.%02d", aucVersion[1], aucVersion[0]);
            palDataMgr_Data_Access(edcXFPGA_VERSION, edaWRITE_RAM_ONLY_NO_ACTION, aucVerString);

            if(strcmp(aucVerString, "Q00.00") == 0 ||   //G100_Simon_0077
               strcmp(aucVerString, "QFF.FF") == 0 )
            {
                LOG_MSG(db_APP_SYSTEM, "XFPGA version error %s\n", aucVerString);
                eResult = rcERROR;
            }
        }
        eResultAll &= eResult;
    }

	//HDBaseT
    //G100_Wilsonj_0069 Start
    memset(aucVersion, 0, sizeof(aucVersion));
    eResult = halMCUCtrl_HDBaseT_Version_Get(aucVersion);

    if(eResult == rcSUCCESS)
    {
        LOG_MSG(db_APP_SYSTEM, "HDBaseT   H%02X.%02X.%02X.%02X\n", aucVersion[0], aucVersion[1] ,aucVersion[2], aucVersion[3]);  //G100_Wilsonj_0070

        sprintf((char*)aucVerString, "H%02X.%02X.%02X.%02X", aucVersion[0], aucVersion[1], aucVersion[2], aucVersion[3]);  //G100_Wilsonj_0070
        palDataMgr_Data_Access(edcHDBASET_VERSION, edaWRITE_RAM_ONLY_NO_ACTION, aucVerString);

        if(strcmp(aucVerString, "H00.00.00.00") == 0 ||   //G100_Simon_0077
           strcmp(aucVerString, "HFF.FF.FF.FF") == 0 )
        {
            LOG_MSG(db_APP_SYSTEM, "HDBaseT version error %s\n", aucVerString);
            eResult = rcERROR;
        }
    }
    eResultAll &= eResult;
    //G100_Wilsonj_0069 End

	//Motor Board
    Syscfg_Value_Get(eWith_Motor_MCU,&ulWith_MCU); //HICC2_Julie_0055
    if(ulWith_MCU)
    {
        memset(aucVersion, 0, sizeof(aucVersion));
        eResult = palMotor_Version_Get(aucVersion);

        if(eResult == rcSUCCESS)
        {
            UINT8 ucModule = 0;

            palMotor_LensModule_Get(&ucModule);
#if 1 //A70Gen2_Doulas_0002 Modify//不做判斷 //A35G2_Larry_0064
            if((eLENS_MODULE)ucModule != (eLENS_MODULE)Syscfg_Value_Get_Typeint(eLensModuleType))
            {
                ucModule = (UINT8)Syscfg_Value_Get_Typeint(eLensModuleType);
                palMotor_LensModule_Set(&ucModule);
            }
#endif /* 0 */

#ifdef OE_JIG
            //halMotor_OEJig_Set(); //HICC2_Doulas_0032 remove
#endif /* OE_JIG */

            m_cLensModel = ucModule;

            //palEnvironment_LensModelCheck_Set(FALSE);	//G100_Clare_0059, add

            LOG_MSG(db_APP_SYSTEM, "Motor Board M%02d.%02d\n", aucVersion[1], aucVersion[0]);
            sprintf((char*)aucVerString, "M%02d.%02d", aucVersion[1], aucVersion[0]);
            palDataMgr_Data_Access(edcMOTOR_VERSION, edaWRITE_RAM_ONLY_NO_ACTION, aucVerString);

            if(strcmp(aucVerString, "M00.00") == 0 ||   //G100_Simon_0077
            strcmp(aucVerString, "MFF.FF") == 0 )
            {
                LOG_MSG(db_APP_SYSTEM, "Motor version error %s\n", aucVerString);
                eResult = rcERROR;
            }
        }
        eResultAll &= eResult;
    }

    //DDP Version check         //G100_Simon_0070
    Syscfg_Value_Get(eWith_DDP,&ulWith_MCU); //HICC2_Julie_0055
    if(ulWith_MCU)
    {
        memset(aucVerString, 0, sizeof(aucVerString));
        palDataMgr_Data_Access(edcFORMATER_VERSION, edaREAD, aucVerString);
        if(strcmp(aucVerString , "") == 0 ||
            strcmp(aucVerString , "F00.00") == 0 )
        {
            LOG_MSG(db_APP_SYSTEM, "DDP version check fail (%s)\n", aucVerString);

            if(halDDP4422_IsAsicReady())
            {
                //A70LV_Doulas_0077 read DDP version
                UINT16 wVerDDPApp = 0;
                UINT16 wVerDDPApp_compare = 0; //A70LV_John_0019 add ddp version checking mechanism
                UINT32 dwVerDDPSq = 0;
                UINT32 wVerDDPSq_compare = 0; //A70LV_Larry_0379
                UINT8  aucVerString[32] = {0};
                UINT8 ucRetry = 2; //A70LV_John_0019 add ddp version checking mechanism

                //A70LV_John_0019 start add ddp version checking mechanism
                while (ucRetry > 0)
                {
                    MS_SLEEP(50);
                    halFormatter_SYS_AppVersionGet(&wVerDDPApp);
                    if(wVerDDPApp == wVerDDPApp_compare)
                    {
                        ucRetry --;
                    }
                    wVerDDPApp_compare = wVerDDPApp;
                }
                //A70LV_John_0019 end

                ucRetry = 2;

                while (ucRetry > 0) //A70LV_Larry_0379
                {
                    MS_SLEEP(50);
                    halFormatter_SEQ_RevisionGet(&dwVerDDPSq);
                    if(dwVerDDPSq == wVerDDPSq_compare)
                    {
                        ucRetry --;
                    }
                    wVerDDPSq_compare = dwVerDDPSq;
                }

                LOG_MSG(db_APP_SYSTEM, "DDP      F%02d.%02d(%c%c%c%d)\n",
                    (UINT8)(wVerDDPApp>>8),      //DDP442x       Fxx.xx(HDRxx)
                    (UINT8)(wVerDDPApp),
                    (UINT8)(dwVerDDPSq>>24),
                    (UINT8)(dwVerDDPSq>>16),
                    (UINT8)(dwVerDDPSq>>8),
                    (UINT8)(dwVerDDPSq));

                sprintf((char*)aucVerString, "F%02d.%02d(%c%c%c%d)" ,
                    (UINT8)(wVerDDPApp>>8),      //DDP442x       Fxx.xx(HDRxx)
                    (UINT8)(wVerDDPApp),
                    (UINT8)(dwVerDDPSq>>24),
                    (UINT8)(dwVerDDPSq>>16),
                    (UINT8)(dwVerDDPSq>>8),
                    (UINT8)(dwVerDDPSq));

                if(eEXEC_CODE_PASS != palDataMgr_Data_Access(edcFORMATER_VERSION, edaWRITE_RAM_ONLY_NO_ACTION, aucVerString))
                {
                    eResult = rcERROR;
                }

                palDataMgr_Data_Access(edcFORMATER_VERSION, edaREAD, aucVerString);

                if(strcmp(aucVerString , "") == 0 ||        //G100_Simon_0077
                strcmp(aucVerString , "F00.00") == 0 )
                {
                    LOG_MSG(db_APP_SYSTEM, "DDP version error %s\n", aucVerString);
                    eResult = rcERROR;
                }
            }
            else
            {
                eResult = rcERROR;
            }
        }
        eResultAll &= eResult;
        LOG_MSG(db_APP_SYSTEM, "DDP  %s\n", aucVerString);
    }

    //LAN Version         //G100_Simon_0070
    memset(aucVerString, 0, sizeof(aucVerString));
    palDataMgr_Data_Access(edcLAN_VERSION, edaREAD, aucVerString);
    if(strcmp(aucVerString , "") == 0 )
    {
        LOG_MSG(db_APP_SYSTEM, "LAN version check fail\n");

        FILE *fp = fopen(LAN_VERSION_FILE_PATH, "r");
        if(fp == NULL)
        {
            LOG_MSG(db_APP_SYSTEM, "LAN version check fail (no file %s)\n", LAN_VERSION_FILE_PATH);
            eResult = rcERROR;
        }
        else
        {
            fscanf(fp, "Rootfs=%12s", aucVerString);
            fclose(fp);
            if(eEXEC_CODE_PASS != palDataMgr_Data_Access(edcLAN_VERSION, edaWRITE_RAM_ONLY_NO_ACTION, aucVerString))
            {
                eResult = rcERROR;
            }

            palDataMgr_Data_Access(edcLAN_VERSION, edaREAD, aucVerString);

            if(strcmp(aucVerString, "") == 0)   //G100_Simon_0077
            {
                LOG_MSG(db_APP_SYSTEM, "LAN version error %s\n", aucVerString);
                eResult = rcERROR;
            }
        }
    }
    eResultAll &= eResult;
    LOG_MSG(db_APP_SYSTEM, "LAN  %s\n", aucVerString);

	palDataMgr_OPDFWversion(eOPD_FW_VERSION_LOG); //G100_Julie_0044

	palLANProcSendToLAN(edcFOTA_MODEL_NAME);  //A35G2_Coda_0118
    return eResultAll;
}

#else

eRESULT palSystem_VersionCheck(BOOL bAutoUpgrade)   //A35G2
{

    UINT8 aucVersion[8] = {0};
    UINT8 aucVerString[32] = {0};  //G100_Wilsonj_0069
    eRESULT eResult = rcSUCCESS;
    eRESULT eResultAll = rcSUCCESS;
    UINT8 cFPGA_Model = 0;

    //Version check

    memset(aucVersion, 0, sizeof(aucVersion));
	#if 0 //Depends on X35Gen2 design
	eResult = halFrontEndCtrl_Version_Get(aucVersion);
	#else
    eResult = halLDCtrl_Version_Get(aucVersion);
    #endif

    if(eResult == rcSUCCESS)
    {
        palDataMgr_Model_ID_Set();

        if(strcmp(aucVerString, "E00.00") == 0 ||   //G100_Simon_0077
           strcmp(aucVerString, "EFF.FF") == 0 )
        {
            LOG_MSG(db_APP_SYSTEM, "MCU version error %s\n", aucVerString);
            eResult = rcERROR;
        }

        LOG_MSG(db_APP_SYSTEM, "MCU E%02d.%02d , FMT mcu Platform = 0x%02X, FMT mcu Customer = %d\n", aucVersion[1], aucVersion[0], aucVersion[2], aucVersion[3]);

        sprintf((char*)aucVerString, "E%02d.%02d", aucVersion[1], aucVersion[0]);
        palDataMgr_Data_Access(edcFRONTEND_VERSION, edaWRITE_RAM_ONLY_NO_ACTION, aucVerString);

        //palDataMgr_Data_Access(edcCUSTOMER_CODE, edaWRITE_THROUGH_WITH_ACTION, &aucVersion[2]); //set custom code
        //palDataMgr_Data_Access(edcMODEL_NAME, edaWRITE_THROUGH_WITH_ACTION, &aucVersion[2]); //set model name

        #if 0
        switch(aucVersion[2])
        {
            case MODULE_TYPE_ID0_PLATFORM:
                m_sPalSysInfo.ucModelID = MODEL_ID_0;   //ex : eMODEL_TYPE_DWU880GS
                break;

            case MODULE_TYPE_ID1_PLATFORM:
                m_sPalSysInfo.ucModelID = MODEL_ID_1;   //ex : eMODEL_TYPE_DWU1100GS
                break;

            case MODULE_TYPE_ID2_PLATFORM:
                m_sPalSysInfo.ucModelID = MODEL_ID_2;   //ex : eMODEL_TYPE_DWU1300GS
                break;

            default:
                break;
        }
        #endif

//Optoma和Barco外殼一樣，所以thermal預設先用Barco的 //A35G2_Larry_0061
        UINT8 CustomerID = Syscfg_Value_Get_Typeint(eCustomerID);

        if(CustomerID == CUSTOMER_ID_CHRISTIE)
        {
            halLDCtrl_CustomerID_Set(CUSTOMER_ID_CHRISTIE);
        }
        else if(CustomerID == CUSTOMER_ID_OPTOMA)
        {
            halLDCtrl_CustomerID_Set(CUSTOMER_ID_OPTOMA);
        }
        else //(Syscfg_Value_Get_Typeint(eCustomerID) != CUSTOMER_CODE_BASIL)
        {
            halLDCtrl_CustomerID_Set(CUSTOMER_ID_BARCO);
        }
    }
    eResultAll &= eResult;

    memset(aucVersion, 0, sizeof(aucVersion));
    eResult = halMCUCtrl_Version_Get(aucVersion);

    if(eResult == rcSUCCESS)
    {
        LOG_MSG(db_APP_SYSTEM, "MCU A%02d.%02d\n", aucVersion[1], aucVersion[0] );

        sprintf((char*)aucVerString, "A%02d.%02d", aucVersion[1], aucVersion[0]);
        palDataMgr_Data_Access(edcFMCU_VERSION, edaWRITE_RAM_ONLY_NO_ACTION, aucVerString);

        if(strcmp(aucVerString, "A00.00") == 0 ||   //G100_Simon_0077
           strcmp(aucVerString, "AFF.FF") == 0 )
        {
            LOG_MSG(db_APP_SYSTEM, "MCU version error %s\n", aucVerString);
            eResult = rcERROR;
        }
    }
    eResultAll &= eResult;

    memset(aucVersion, 0, sizeof(aucVersion));
    eResult = halMCUCtrl_KeypadVersion_Get(aucVersion);
    if(eResult == rcSUCCESS)
    {
        LOG_MSG(db_APP_SYSTEM, "Keypad   K%02d.%02d\n", aucVersion[1], aucVersion[0] );

        sprintf((char*)aucVerString, "K%02d.%02d", aucVersion[1], aucVersion[0]);
        palDataMgr_Data_Access(edcKEYPAD_VERSION, edaWRITE_RAM_ONLY_NO_ACTION, aucVerString);

        if(strcmp(aucVerString, "K00.00") == 0 ||   //G100_Simon_0077
           strcmp(aucVerString, "KFF.FF") == 0 )
        {
            LOG_MSG(db_APP_SYSTEM, "Keypad version error %s\n", aucVerString);
            eResult = rcERROR;
        }
    }
    eResultAll &= eResult;

    //G100_Wilsonj_0025 Start
    memset(aucVersion, 0, sizeof(aucVersion));
    eResult = halMCUCtrl_FPGA1_Version_Get(aucVersion);

    if(eResult == rcSUCCESS)
    {
        LOG_MSG(db_APP_SYSTEM, "FPGA1   X%02d.%02d\n", aucVersion[1], aucVersion[0]);

        sprintf((char*)aucVerString, "X%02d.%02d", aucVersion[1], aucVersion[0]);

        if(aucVersion[2] == 'A')
        {
            cFPGA_Model = 1; //FPGA is 10M08
        }
        else
        {
            cFPGA_Model = 0; //FPGA is 10M04
        }

        palDataMgr_Data_Access(edcFPGA1_VERSION, edaWRITE_RAM_ONLY_NO_ACTION, aucVerString);

        if(strcmp(aucVerString, "X00.00") == 0 ||   //G100_Simon_0077
           strcmp(aucVerString, "XFF.FF") == 0 )
        {
            LOG_MSG(db_APP_SYSTEM, "FPGA1 version error %s\n", aucVerString);
            eResult = rcERROR;
        }
    }
    eResultAll &= eResult;

    memset(aucVersion, 0, sizeof(aucVersion));
    eResult = halMCUCtrl_FPGA2_Version_Get(aucVersion);

    if(eResult == rcSUCCESS)
    {
        LOG_MSG(db_APP_SYSTEM, "FPGA2   Y%02d.%02d\n", aucVersion[1], aucVersion[0]);

        sprintf((char*)aucVerString, "Y%02d.%02d", aucVersion[1], aucVersion[0]);
        palDataMgr_Data_Access(edcFPGA2_VERSION, edaWRITE_RAM_ONLY_NO_ACTION, aucVerString);

        if(strcmp(aucVerString, "Y00.00") == 0 ||   //G100_Simon_0077
           strcmp(aucVerString, "YFF.FF") == 0 )
        {
            LOG_MSG(db_APP_SYSTEM, "FPGA2 version error %s\n", aucVerString);
            eResult = rcERROR;
        }
    }
    eResultAll &= eResult;

    memset(aucVersion, 0, sizeof(aucVersion));
    eResult = halMCUCtrl_HWVersion_Get(aucVersion);

    if(eResult == rcSUCCESS)
    {
        LOG_MSG(db_APP_SYSTEM, "HW version %02d\n", aucVersion[0]);

        if(cFPGA_Model)
        {
            //aucVersion[0] = aucVersion[0] + eHWVER_M08_VERA;
        }

        palDataMgr_Data_Access(edcHW_PROJECTOR_ID, edaWRITE_RAM_ONLY_NO_ACTION, aucVersion);
    }

    eResultAll &= eResult;

    //XFPGA Version
    eResult  = halBoard_XillinxFPGA_Version_BL_Get(&aucVersion[0]); //G100_Simon_0059   //A35G2_Simon_0065
    eResult &= halBoard_XillinxFPGA_Version_BH_Get(&aucVersion[1]);  //A35G2_Simon_0065

    if(eResult == rcSUCCESS)
    {
        LOG_MSG(db_APP_SYSTEM, "XFPGA  Q%02d.%02d\n", aucVersion[1], aucVersion[0]);
        sprintf((char*)aucVerString, "Q%02d.%02d", aucVersion[1], aucVersion[0]);
        palDataMgr_Data_Access(edcXFPGA_VERSION, edaWRITE_RAM_ONLY_NO_ACTION, aucVerString);

        if(strcmp(aucVerString, "Q00.00") == 0 ||   //G100_Simon_0077
           strcmp(aucVerString, "QFF.FF") == 0 )
        {
            LOG_MSG(db_APP_SYSTEM, "XFPGA version error %s\n", aucVerString);
            eResult = rcERROR;
        }
    }
    eResultAll &= eResult;

    //G100_Wilsonj_0069 Start
    memset(aucVersion, 0, sizeof(aucVersion));
    eResult = halMCUCtrl_HDBaseT_Version_Get(aucVersion);

    if(eResult == rcSUCCESS)
    {
        LOG_MSG(db_APP_SYSTEM, "HDBaseT   H%02X.%02X.%02X.%02X\n", aucVersion[0], aucVersion[1] ,aucVersion[2], aucVersion[3]);  //G100_Wilsonj_0070

        sprintf((char*)aucVerString, "H%02X.%02X.%02X.%02X", aucVersion[0], aucVersion[1], aucVersion[2], aucVersion[3]);  //G100_Wilsonj_0070
        palDataMgr_Data_Access(edcHDBASET_VERSION, edaWRITE_RAM_ONLY_NO_ACTION, aucVerString);

        if(strcmp(aucVerString, "H00.00.00.00") == 0 ||   //G100_Simon_0077
           strcmp(aucVerString, "HFF.FF.FF.FF") == 0 )
        {
            LOG_MSG(db_APP_SYSTEM, "HDBaseT version error %s\n", aucVerString);
            eResult = rcERROR;
        }
    }
    eResultAll &= eResult;
    //G100_Wilsonj_0069 End


    memset(aucVersion, 0, sizeof(aucVersion));
    eResult = halMotor_Version_Get(aucVersion);

    if(eResult == rcSUCCESS)
    {
        UINT8 ucModule = 0;

        halMotor_LensModule_Get(&ucModule);
#if 0 //不做判斷 //A35G2_Larry_0064
        if((eLENS_MODULE)ucModule != eLENS_MODULE_X35_S600)
        {
            ucModule = (UINT8)eLENS_MODULE_X35_S600;
            halMotor_LensModule_Set(&ucModule);
        }
#endif /* 0 */

        m_cLensModel = ucModule;

        //palEnvironment_LensModelCheck_Set(FALSE);	//G100_Clare_0059, add

        LOG_MSG(db_APP_SYSTEM, "Motor Board M%02d.%02d\n", aucVersion[1], aucVersion[0]);
        sprintf((char*)aucVerString, "M%02d.%02d", aucVersion[1], aucVersion[0]);
        palDataMgr_Data_Access(edcMOTOR_VERSION, edaWRITE_RAM_ONLY_NO_ACTION, aucVerString);

        if(strcmp(aucVerString, "M00.00") == 0 ||   //G100_Simon_0077
           strcmp(aucVerString, "MFF.FF") == 0 )
        {
            LOG_MSG(db_APP_SYSTEM, "Motor version error %s\n", aucVerString);
            eResult = rcERROR;
        }
    }
    eResultAll &= eResult;


    //DDP Version check         //G100_Simon_0070
    memset(aucVerString, 0, sizeof(aucVerString));
    palDataMgr_Data_Access(edcFORMATER_VERSION, edaREAD, aucVerString);
    if(strcmp(aucVerString , "") == 0 ||
       strcmp(aucVerString , "F00.00") == 0 )
    {
        LOG_MSG(db_APP_SYSTEM, "DDP version check fail (%s)\n", aucVerString);

        if(halDDP4422_IsAsicReady())
        {
            //A70LV_Doulas_0077 read DDP version
            UINT16 wVerDDPApp = 0;
            UINT16 wVerDDPApp_compare = 0; //A70LV_John_0019 add ddp version checking mechanism
            UINT32 dwVerDDPSq = 0;
            UINT32 wVerDDPSq_compare = 0; //A70LV_Larry_0379
            UINT8  aucVerString[32] = {0};
            UINT8 ucRetry = 2; //A70LV_John_0019 add ddp version checking mechanism

            //A70LV_John_0019 start add ddp version checking mechanism
            while (ucRetry > 0)
            {
                MS_SLEEP(50);
                halFormatter_SYS_AppVersionGet(&wVerDDPApp);
                if(wVerDDPApp == wVerDDPApp_compare)
                {
                    ucRetry --;
                }
                wVerDDPApp_compare = wVerDDPApp;
            }
            //A70LV_John_0019 end

            ucRetry = 2;

            while (ucRetry > 0) //A70LV_Larry_0379
            {
                MS_SLEEP(50);
                halFormatter_SEQ_RevisionGet(&dwVerDDPSq);
                if(dwVerDDPSq == wVerDDPSq_compare)
                {
                    ucRetry --;
                }
                wVerDDPSq_compare = dwVerDDPSq;
            }

            LOG_MSG(db_APP_SYSTEM, "DDP      F%02d.%02d(%c%c%c%d)\n",
                (UINT8)(wVerDDPApp>>8),      //DDP442x       Fxx.xx(HDRxx)
                (UINT8)(wVerDDPApp),
                (UINT8)(dwVerDDPSq>>24),
                (UINT8)(dwVerDDPSq>>16),
                (UINT8)(dwVerDDPSq>>8),
                (UINT8)(dwVerDDPSq));

            sprintf((char*)aucVerString, "F%02d.%02d(%c%c%c%d)" ,   //HICC2_Doulas_0022
                (UINT8)(wVerDDPApp>>8),      //DDP442x       Fxx.xx(HDRxx)
                (UINT8)(wVerDDPApp),
                (UINT8)(dwVerDDPSq>>24),
                (UINT8)(dwVerDDPSq>>16),
                (UINT8)(dwVerDDPSq>>8),
                (UINT8)(dwVerDDPSq));

            if(eEXEC_CODE_PASS != palDataMgr_Data_Access(edcFORMATER_VERSION, edaWRITE_RAM_ONLY_NO_ACTION, aucVerString))
            {
                eResult = rcERROR;
            }

            palDataMgr_Data_Access(edcFORMATER_VERSION, edaREAD, aucVerString);

            if(strcmp(aucVerString , "") == 0 ||        //G100_Simon_0077
               strcmp(aucVerString , "F00.00") == 0 )
            {
                LOG_MSG(db_APP_SYSTEM, "DDP version error %s\n", aucVerString);
                eResult = rcERROR;
            }
        }
        else
        {
            eResult = rcERROR;
        }
    }
    eResultAll &= eResult;
    LOG_MSG(db_APP_SYSTEM, "DDP  %s\n", aucVerString);


    //LAN Version         //G100_Simon_0070
    memset(aucVerString, 0, sizeof(aucVerString));
    palDataMgr_Data_Access(edcLAN_VERSION, edaREAD, aucVerString);
    if(strcmp(aucVerString , "") == 0 )
    {
        LOG_MSG(db_APP_SYSTEM, "LAN version check fail\n");

        FILE *fp = fopen(LAN_VERSION_FILE_PATH, "r");
        if(fp == NULL)
        {
            LOG_MSG(db_APP_SYSTEM, "LAN version check fail (no file %s)\n", LAN_VERSION_FILE_PATH);
            eResult = rcERROR;
        }
        else
        {
            fscanf(fp, "Rootfs=%12s", aucVerString);
            fclose(fp);
            if(eEXEC_CODE_PASS != palDataMgr_Data_Access(edcLAN_VERSION, edaWRITE_RAM_ONLY_NO_ACTION, aucVerString))
            {
                eResult = rcERROR;
            }

            palDataMgr_Data_Access(edcLAN_VERSION, edaREAD, aucVerString);

            if(strcmp(aucVerString, "") == 0)   //G100_Simon_0077
            {
                LOG_MSG(db_APP_SYSTEM, "LAN version error %s\n", aucVerString);
                eResult = rcERROR;
            }
        }
    }
    eResultAll &= eResult;
    LOG_MSG(db_APP_SYSTEM, "LAN  %s\n", aucVerString);

	palDataMgr_OPDFWversion(eOPD_FW_VERSION_LOG); //G100_Julie_0044

	palLANProcSendToLAN(edcFOTA_MODEL_NAME);  //A35G2_Coda_0118
    return eResultAll;
}

#endif

// ==============================================================================
// FUNCTION NAME: palSystem_Scaler_Upgrade
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
// 2018/03/12, Larry Create
// --------------------
// ==============================================================================
void palSystem_Scaler_Upgrade(UINT8 ucFocus)
{
    if(palSystem_Suspend(__FUNCTION__) == rcSUCCESS)
    {
        halGui_Upgrade(ucFocus); //A35G2_CDS_Larry_0050
        halGui_OSDReload();

        palSystem_Resume(__FUNCTION__);
    }

    LOG_MSG(db_UPGRADE, "Ichip Upgrade Done\n");
}

#if (LOGO_REPLACE == 1)	//A35G2_Coda_0067
//--------------LOGO--------------
void palSystem_SecondLogoPartialUpgrade(UINT8 ucFocus) //T100_Coda_0001 //A65_OPTOMA_Julie_0067 //A65_OPTOMA_Julie_0102
{
    if(palSystem_Suspend(__FUNCTION__) == rcSUCCESS)
    {
        //lGui_SecondLogoPaletteStore(TMP_2ND_LOGO_PALETTE_FILE, eCM_USER_LOGO_2D);
		//lGui_GuiData_PartialWrite(TMP_2ND_LOGO_RAWDATA_FILE, (UINT32)DEF_SFL_OSD_2ND_LOGO, (UINT32)OSD_2ND_LOGO_BITMAP_SIZE, ucFocus);

		halGui_Upgrade_Second_Logo_Replace(ucFocus);

		halGui_2ND_LOGO_OSDReload(eCM_USER_LOGO_2D);
        palSystem_Resume(__FUNCTION__);
        LOG_MSG(db_UPGRADE, "%s() pass.\r\n", __FUNCTION__);
    }
    else
        LOG_MSG(db_UPGRADE, "%s() fail!\r\n", __FUNCTION__);
}

void palSystem_ServiceCustomLogoPartialUpgrade(UINT8 ucFocus) //A65_OPTOMA_Julie_0076 //A65_OPTOMA_Julie_0102//A35G2_Coda_0067
{
    if(palSystem_Suspend(__FUNCTION__) == rcSUCCESS)
    {
        //halGui_SecondLogoPaletteStore(TMP_2ND_LOGO_PALETTE_FILE, eCM_USER_LOGO_SERVICE_2D);
		//lGui_GuiData_PartialWrite(TMP_2ND_LOGO_RAWDATA_FILE, (UINT32)DEF_SFL_OSD_SERVICE_2ND_LOGO, (UINT32)OSD_2ND_LOGO_BITMAP_SIZE, ucFocus);

		halGui_Upgrade_Service_Second_Logo_Replace(ucFocus);

		halGui_2ND_LOGO_OSDReload(eCM_USER_LOGO_SERVICE_2D);
        palSystem_Resume(__FUNCTION__);
        LOG_MSG(db_UPGRADE, "%s() pass.\r\n", __FUNCTION__);
    }
    else
        LOG_MSG(db_UPGRADE, "%s() fail!\r\n", __FUNCTION__);
}
#endif	//ZU860_Clare_0124, add, <<<


// ==============================================================================
// FUNCTION NAME: palSystem_PowerOff_Config
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
// 2018/02/27, Larry Create
// --------------------
// ==============================================================================
void palSystem_PowerOff_Config(void)
{
    halMCU_SystemPowerOff();

    //palLedProc_LED_Behavior_Set(eLED_STATUS_POWEROFF); //A70LV_Larry_0020
    //palLedProc_Poll(1); //A70LV_Larry_0020
}

// ==============================================================================
// FUNCTION NAME: palSystem_FW_UpgradeCheckGet
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
// 2018/02/27, Larry Create
// --------------------
// ==============================================================================
UINT8 palSystem_FW_UpgradeCheckGet(void)
{
    //0 is Idel, 1 is busy
    return m_sPalSysInfo.ucFW_upgrade;
}

// ==============================================================================
// FUNCTION NAME: palSystem_FW_UpgradeCheckSet
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
// 2018/03/12, Larry Create
// --------------------
// ==============================================================================
void palSystem_FW_UpgradeCheckSet(void)
{
    m_sPalSysInfo.ucFW_upgrade = 1;
}

// ==============================================================================
// FUNCTION NAME: palSystem_ModelIDGet
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
// 2018/03/19, Larry Create
// --------------------
// ==============================================================================
UINT8 palSystem_ModelIDGet(void)
{
    //MODEL_ID_0
    //MODEL_ID_1
    //MODEL_ID_2...

    return Board_ModelID_Get();
}

// ==============================================================================
// FUNCTION NAME: palSystem_ModelIDSet
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
// 2022/07/21, Larry Create
// --------------------
// ==============================================================================
void palSystem_ModelIDSet(UINT8 ucData)
{
	switch(ucData)
	{
        case MODULE_TYPE_ID0_PLATFORM:
			Board_ModelID_Set(MODEL_ID_0);
			break;

        case MODULE_TYPE_ID1_PLATFORM:
			Board_ModelID_Set(MODEL_ID_1);
			break;

        case MODULE_TYPE_ID2_PLATFORM:
			Board_ModelID_Set(MODEL_ID_2);
			break;

		default:
			Board_ModelID_Set(MODEL_ID_INVALID);
			break;
	}
}
// ==============================================================================
// FUNCTION NAME: palSystem_PowerStateGet
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
// 2018/06/21, Larry Create
// --------------------
// ==============================================================================
ePOWER_STATE palSystem_PowerStateGet(void) //A70LV_Larry_0135
{
    return m_sPalSysInfo.ePowerState;
}

// ==============================================================================
// FUNCTION NAME: palSystem_HostReady
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
// 2020/05/14, Larry Create
// --------------------
// ==============================================================================
UINT8 palSystem_HostReadyGet(void)
{
    return m_sPalSysInfo.ucHostReady;
}

// ==============================================================================
// FUNCTION NAME: palSystem_HostReady
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
// 2020/05/14, Larry Create
// --------------------
// ==============================================================================
UINT8 palSystem_SystemReadyGet(void)
{
    //datapath init ready
    return m_sPalSysInfo.ucSystemReady;
}

// ==============================================================================
// FUNCTION NAME: palSystem_HostReadySet
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
// 2020/05/14, Larry Create
// --------------------
// ==============================================================================
void palSystem_SystemReadySet(UINT8 ucValue)
{
    m_sPalSysInfo.ucSystemReady = ucValue;
}

// ==============================================================================
// FUNCTION NAME: palSystem_HostReadySet
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
// 2020/05/14, Larry Create
// --------------------
// ==============================================================================
void palSystem_HostReadySet(UINT8 ucValue)
{
    m_sPalSysInfo.ucHostReady = ucValue;
}

// ==============================================================================
// FUNCTION NAME: palSystem_PanelID_Get
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
// 2018/06/21, Larry Create
// --------------------
// ==============================================================================
ePANEL_ID palSystem_PanelID_Get(void)   //A70LV_Doulas_0155
{
    return m_sPalSysInfo.ePanelTimingId;
}

void palSystem_PanelID_Set(ePANEL_ID PanelID)
{
    m_sPalSysInfo.ePanelTimingId = PanelID;

    palLANProcSendToLAN((UINT16)edcPANEL_H_RESOLUTION);
    palLANProcSendToLAN((UINT16)edcPANEL_V_RESOLUTION);
    palLANProcSendToLAN((UINT16)edcPANEL_IS_3D_OUTPUT);
    palLANProcSendToLAN((UINT16)edcSYSTEM_PANEL_ID);

}

// ==============================================================================
// FUNCTION NAME: palSystem_UpgradeLPCMCU
// DESCRIPTION:
//
//
// Params:
// eLPC54113_INDEX eIndex:
// BOOL bEnforce:
//
// Returns:
//
//
// Modification History
// --------------------
// 2018/05/31, Larry Create
// --------------------
// ==============================================================================
eRESULT palSystem_UpgradeLPCMCU(UINT8 cIndex, BOOL bEnforce)
{
    eRESULT eResut = rcSUCCESS;

    FILE *pFile = NULL;
    UINT8 ucRetry = 0;
    UINT8 aucVersion[8] = {0};
    UINT8 aucExtVersion[2] = {0};
    UINT8* pcBuffer = NULL;
    UINT32 ulSize = 0;

    switch(cIndex)
    {
        case eLPCMCU_INDEX_FRONT_2K:
        {
            pFile = fopen("/mnt/configs/scaler/FwPackage/lpc5411x_frondend.bin", "rb");

            if(pFile == NULL)
                return rcERROR;

            fseek(pFile, 0, SEEK_END);
            ulSize = ftell(pFile);

            rewind(pFile);

            pcBuffer = (UINT8*)malloc(ulSize+1);

            memset(pcBuffer, 0, ulSize);

            fread(pcBuffer, 1, ulSize, pFile);

            fclose(pFile);

            memcpy(aucExtVersion, &pcBuffer[LPCMCU_APP_FIRMWARE_VERSION], 2);

			#if 0 //Depends on X35Gen2 design
            eResut = halFrontEndCtrl_Version_Get(aucVersion);
			#else
			eResut = halLDCtrl_Version_Get(aucVersion);
            #endif
        }
            break;

        case eLPCMCU_INDEX_MOTORBD:
        {
            pFile = fopen("/mnt/configs/scaler/FwPackage/lpc5411x_motor_bd.bin", "rb");

            if(pFile == NULL)
                return rcERROR;

            fseek(pFile, 0, SEEK_END);
            ulSize = ftell(pFile);

            rewind(pFile);

            pcBuffer = (UINT8*)malloc(ulSize+1);

            memset(pcBuffer, 0, ulSize);

            fread(pcBuffer, 1, ulSize, pFile);

            fclose(pFile);

            memcpy(aucExtVersion, &pcBuffer[LPCMCU_APP_FIRMWARE_VERSION], 2);

            eResut = halMotor_Version_Get(aucVersion);
        }
            break;

        case eLPCMCU_INDEX_SYSTEM:
            {
                pFile = fopen("/mnt/configs/scaler/FwPackage/ExtFlash.bin", "rb");

                if(pFile == NULL)
                {
                    ASSERT_ALWAYS();
                    return rcERROR;
                }
                fseek(pFile, 0, SEEK_END);
                ulSize = ftell(pFile);

                rewind(pFile);

                pcBuffer = (UINT8*)malloc(ulSize+1);

                memset(pcBuffer, 0, ulSize);

                fread(pcBuffer, 1, ulSize, pFile);

                fclose(pFile);

                memcpy(aucExtVersion, &pcBuffer[LPCMCU_APP_FIRMWARE_VERSION], 2);

                eResut = halMCUCtrl_Version_Get(aucVersion);
            }
            break;

        case eLPCMCU_INDEX_FMT_2K:
            {
                pFile = fopen("/mnt/configs/scaler/FwPackage/lpc5460x_FMT_8K.bin", "rb");

                if(pFile == NULL)
                    return rcERROR;

                fseek(pFile, 0, SEEK_END);
                ulSize = ftell(pFile);

                rewind(pFile);

                pcBuffer = (UINT8*)malloc(ulSize+1);

                memset(pcBuffer, 0, ulSize);

                fread(pcBuffer, 1, ulSize, pFile);

                fclose(pFile);

                memcpy(aucExtVersion, &pcBuffer[LPCMCU_APP_FIRMWARE_VERSION], 2);

                eResut = halMotor_Version_Get(aucVersion);
            }
            break;

        default:
            return rcERROR;
    }

#if 1
    if(eResut == rcSUCCESS)
    {
        LOG_MSG(db_UPGRADE, "MCU Version %02d.%02d, ExtVersion %02d.%02d\r\n", aucVersion[1], aucVersion[0], aucExtVersion[1], aucExtVersion[0]);

        if(((aucVersion[1] != aucExtVersion[1]) || (aucVersion[0] != aucExtVersion[0])) || bEnforce)
        {
            eResut = rcBUSY;
            for(ucRetry = 0; ((ucRetry < 10) && (eResut != rcSUCCESS)); ucRetry++)
            {
                LOG_MSG(db_UPGRADE, "Lpc54113 %d Upgrade retry %d\r\n", cIndex, ucRetry);

                if(eLPCMCU_INDEX_SYSTEM == cIndex)
                {
                    eResut = palIapProc_UpgradeExtFlash(cIndex, pcBuffer, ulSize);
                }
                else
                {
                    eResut = palIapProc_LPCMCU_Upgrade(cIndex, pcBuffer, ulSize);
                }

                MS_SLEEP(1000);
            }

            if(eResut == rcSUCCESS)
            {
                memset(aucVersion, 0, sizeof(aucVersion));

                eRESULT eVerResut = rcBUSY;
                for(ucRetry = 0; ((ucRetry < 10) && (eVerResut != rcSUCCESS)); ucRetry++)
                {
                    LOG_MSG(db_UPGRADE, "LpcMCU %d Ver get retry %d\r\n", cIndex, ucRetry);

                    switch(cIndex)
                    {
                        case eLPCMCU_INDEX_FRONT_2K:
							#if 0 //Depends on X35Gen2 design
                            eVerResut = halFrontEndCtrl_Version_Get(aucVersion);
							#else
							eVerResut = halLDCtrl_Version_Get(aucVersion);
							#endif
                            LOG_MSG(db_UPGRADE, "MCU frontend Ver E%02d.%02d\r\n", aucVersion[1], aucVersion[0]);
                            break;

                        case eLPCMCU_INDEX_MOTORBD:
                            eVerResut = halMotor_Version_Get(aucVersion);
                            LOG_MSG(db_UPGRADE, "MCU Motor Ver M%02d.%02d\r\n", aucVersion[1], aucVersion[0]);
                            break;

                        case eLPCMCU_INDEX_SYSTEM:
                            eVerResut = halMCUCtrl_Version_Get(aucVersion);
                            LOG_MSG(db_UPGRADE, "MCU system Ver A%02d.%02d\r\n", aucVersion[1], aucVersion[0]);
                            break;

                        case eLPCMCU_INDEX_FMT_2K:
                            eVerResut = halMotor_Version_Get(aucVersion);
                            LOG_MSG(db_UPGRADE, "MCU system Ver M%02d.%02d\r\n", aucVersion[1], aucVersion[0]);
                            break;

                        default:
                            break;
                    }

                    if(eVerResut == rcSUCCESS)
                    {
                        break;
                    }
                    else
                    {
                        MS_SLEEP(1000);
                    }
                }
            }
        }
    }
    else
    {
        ASSERT_ALWAYS();
    }
#endif /* 0 */

    free(pcBuffer);

    return eResut;
}

// ==============================================================================
// FUNCTION NAME: palSystem_CustomerSplashGet
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
// 2018/06/21, Larry Create
// --------------------
// ==============================================================================
UINT8 palSystem_CustomerSplashGet(void)
{
    return m_sPalSysInfo.ucCustomer_Splash;
}

// ==============================================================================
// FUNCTION NAME: palSystem_CustomerSplashSet
// DESCRIPTION:
//
//
// Params:
// UINT8 ucCustomer:
//
// Returns:
//
//
// Modification History
// --------------------
// 2018/06/22, Larry Create
// --------------------
// ==============================================================================
void palSystem_CustomerSplashSet(UINT8 ucCustomer)
{
    m_sPalSysInfo.ucCustomer_Splash = ucCustomer;
}

// ==============================================================================
// FUNCTION NAME: palSystem_Pause
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
// 2020/03/11, Larry Create
// --------------------
// ==============================================================================
eRESULT palSystem_Suspend(const char *cFuncName)
{
    //LOG_MSG(db_ALWAYS, "palSystem_Suspend %s\n", cFuncName);
    if(m_sPalSysInfo.bTaskPause == TRUE)	//G100_Doulas_0042
    {
		return rcERROR;
    }

    struct timespec s_timeout;
    clock_gettime(CLOCK_REALTIME, &s_timeout);
    util_TimespecAddms(&s_timeout, APP_TASK_SEMPHORE_WAIT);


    if(pthread_mutex_timedlock(&m_sPalSysInfo.xTaskMutex, &s_timeout) == 0)
    {
        m_sPalSysInfo.bTaskPause = TRUE;   //G100_Simon_0006 move here

        appPoll_Task_Suspend();
        palDataPath_Task_Suspend();
        palDataPath_TaskSub_Suspend();
        //GuiCb.fpGui_Task_SuspendCb();

        MS_SLEEP(200); //

        pthread_mutex_unlock(&m_sPalSysInfo.xTaskMutex);

        return rcSUCCESS;
    }
    else
    {
        LOG_MSG(db_APP_SYSTEM, "%s fail\n", __FUNCTION__);
    }

    return rcERROR;
}

// ==============================================================================
// FUNCTION NAME: palSystem_Resume
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
// 2020/03/11, Larry Create
// --------------------
// ==============================================================================
eRESULT palSystem_Resume(const char *cFuncName)
{
    //LOG_MSG(db_ALWAYS, "palSystem_Resume %s\n", cFuncName);
    if(m_sPalSysInfo.bTaskPause == FALSE)	//G100_Doulas_0042
    {
		return rcERROR;
    }

    struct timespec s_timeout;
    clock_gettime(CLOCK_REALTIME, &s_timeout);
    util_TimespecAddms(&s_timeout, APP_TASK_SEMPHORE_WAIT);

    if(pthread_mutex_timedlock(&m_sPalSysInfo.xTaskMutex, &s_timeout) == 0)
    {
        appPoll_Task_Resume();
        palDataPath_Task_Resume();
        palDataPath_TaskSub_Resume();
        //GuiCb.fpGui_Task_ResumeCb();

        m_sPalSysInfo.bTaskPause = FALSE;

        pthread_mutex_unlock(&m_sPalSysInfo.xTaskMutex);

        return rcSUCCESS;
    }

    return rcERROR;
}

// ==============================================================================
// FUNCTION NAME: palSystem_TaskPauseGet
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
// 2020/03/11, Larry Create
// --------------------
// ==============================================================================
BOOL palSystem_TaskPauseGet(void)
{
    return  m_sPalSysInfo.bTaskPause;
}

void palSystem_12VPwrLostSet(BOOL bFlag)    //G100_Owen_0064
{
    m_bIs12VPowerLost = bFlag;
}

UINT8 palSystem_OPFUCheck(void) //G100_Simon_0064
{
    UINT8 ucSystemMcuOpfuCheck = 0;
    halMCU_OPFU_Check_Get(&ucSystemMcuOpfuCheck);

    UINT8 uciScalerOpfuCheck = 0;
    palDataMgr_Data_Access(edcOPFU_CHECK, edaREAD, &uciScalerOpfuCheck);

    if(ucSystemMcuOpfuCheck || uciScalerOpfuCheck)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

//G100_Tim_0009, add, start
eRESULT palSystem_AC_Voltage_Check(void) //AC voltage detect from LVPS. //A65_OPTOMA_Julie_0018
{
	UINT8 ucAC_Data = 0xFF;
	char  aucVerString[AC_VOLTAGE_MAX_LEN] = { 0, 0, 0, 0, 0, 0, 0, 0};

	if(halMCUCtrl_AC_Voltage_Info_Get(&ucAC_Data) == rcSUCCESS)
	{
		if(ucAC_Data == 1)
		{
			sprintf(aucVerString, "110V");
		}
		else
		{
			sprintf(aucVerString, "220V");
		}
	}
	else
	{
		LOG_MSG(db_APP_SYSTEM,"palSystem_AC_Voltage_Check(), Get Fail !!\r\n");
		return rcERROR;
	}

	if(palDataMgr_Data_Access(edcAC_VOLTAGE_INFO, edaWRITE_RAM_ONLY_NO_ACTION, aucVerString) == rcSUCCESS)
	{
		return rcSUCCESS;
	}
	else
	{
		LOG_MSG(db_APP_SYSTEM,"palSystem_AC_Voltage_Check(), Save Fail, AC_Data[%d] VerString[%s]\r\n", ucAC_Data, aucVerString);
		return rcERROR;
	}
}
//G100_Tim_0009, add, end


//return eMODEL_TYPE_DWU880GS / eMODEL_TYPE_DWU1100GS / eMODEL_TYPE_DWU1300GS
eRESULT palSystem_Model_ID_Get(void) //A35G2_BRC_Casper_0023
{
    eRESULT eResult = rcSUCCESS;

    UINT8 ModelID;
    eResult = palSysCtrl_ModelID_Get(&ModelID);

    LOG_MSG(db_APP_SYSTEM, "palSystem_Model_ID_Get 0x%x\r\n", ModelID);

    if(eResult == rcSUCCESS)
    {
        switch(ModelID)
        {
            case MODULE_TYPE_ID0_PLATFORM:
                Board_ModelID_Set(MODEL_ID_0);  //ex: eMODEL_TYPE_DWU880GS
                break;

            case MODULE_TYPE_ID1_PLATFORM:
                Board_ModelID_Set(MODEL_ID_1);  //ex: eMODEL_TYPE_DWU1100GS
                break;

            case MODULE_TYPE_ID2_PLATFORM:
                Board_ModelID_Set(MODEL_ID_2);  //ex: eMODEL_TYPE_DWU1300GS
                break;

            default:
                break;
        }
    }

    return eResult;
}

BOOL palSystem_IS_LOGO_Capture_Enble(void)		//A65_OPTOMA_Doulas_0125
{
	return m_bIsLogoCapture;
}

UINT8 palSystem_WaitWheelStable(void) //A35G2_CDS_Simon_0061
{
    UINT8 ucCheckResult = FALSE;
    UINT8 ucRetry = 30;
    UINT8 ucRecheck = 0;
    UINT32 ulCWSpeed = 0, ulCWFrequence = 0;

    do
    {
        DDP_SYSTEM_STATUS sDDPSystemStatus;
        eRESULT eRet = halFormatter_SYS_SystemStatusGet((UINT8 *)&sDDPSystemStatus);

        if(eRet == rcSUCCESS &&
           //sDDPSystemStatus.DDP_ST_CW_PHASELOCK  == TRUE &&
           sDDPSystemStatus.DDP_ST_CW_FREQLOCK   == TRUE &&
           sDDPSystemStatus.DDP_ST_SEQ_PHASELOCK == TRUE &&
           sDDPSystemStatus.DDP_ST_SEQ_FREQLOCK  == TRUE
        )
        {
            if(palSystem_PanelID_Get() == PANEL_3D_OUTPUT)
            {
                if((ulCWFrequence <= 121) && (ulCWFrequence >= 119))
                {
                    ucRecheck++;
                }
            }
            else
            {
                if((ulCWFrequence <= 181) && (ulCWFrequence >= 179))
                {
                    ucRecheck++;
                }
            }

            // check 3 time OK
            if(ucRecheck >= 3)
            {
                ucCheckResult = TRUE;
                break;
            }
        }
        else
        {
            ucRecheck = 0 ;  //need continuous check 3 times OK
        }

        ucRetry-- ;

        // check Fail
        if(ucRetry == 0)
        {
            ucCheckResult = FALSE;
            break;
        }

        MS_SLEEP(200);

    }while(1);

    return ucCheckResult;
}

// ==============================================================================
// FUNCTION NAME: palSystem_LensModelGet
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
// 2021/11/24, Larry Create
// --------------------
// ==============================================================================
UINT8 palSystem_LensModelGet(void)
{
    return m_cLensModel;
}

// ==============================================================================
// FUNCTION NAME: palSystem_LensModelSet
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
// 2021/11/29, Larry Create
// --------------------
// ==============================================================================
void palSystem_LensModelSet(UINT8 ucType)
{
    m_cLensModel = ucType;
}


void palSystem_TaskMonitor(void)  //A35G2_Simon_0075
{
    for(eTASK_ID eTaskID = 0 ; eTaskID < eTID_NUMBER ; eTaskID++)
    {
        if(sMonTaskInfo[eTaskID].ucStopMonitor == TRUE)
        {
            continue;
        }

        if(palSystem_PowerStateGet() == ePOWER_STATE_UPGRADE)
        {
            if(eTaskID == eTID_DATAPATH ||
               eTaskID == eTID_SUB_DATAPATH ||
               eTaskID == eTID_GUI
            )
            {
                continue;
            }
        }

        if(palSystem_PowerStateGet() == ePOWER_STATE_COOLING ||  //A35G2_Simon_0094
           palSystem_PowerStateGet() == ePOWER_STATE_STANDBY)    //A35G2_Simon_0097
        {
            if(eTaskID == eTID_DATAPATH ||
               eTaskID == eTID_SUB_DATAPATH
            )
            {
                continue;
            }
        }

        if(utilCounterGet(sMonTaskInfo[eTaskID].eCounterType) == 0)
        {
            if(sMonTaskInfo[eTaskID].uiCountdownTimer > 0)
            {
                sMonTaskInfo[eTaskID].uiCountdownTimer--;
            }

            if(sMonTaskInfo[eTaskID].uiCountdownTimer == 0)
            {
                if(sMonTaskInfo[eTaskID].ucCountdownMaxLogOut > 0)
                {
                    sMonTaskInfo[eTaskID].ucCountdownMaxLogOut--;

                    //record to OPD
                    uOPD_DATA uOPDData ;
                    snprintf(uOPDData.cString, 256,  "%s,%s", sMonTaskInfo[eTaskID].acTaskName, sMonTaskInfo[eTaskID].acLogInfoStr);
                    utilOPD_EventSet(eOPD_TASK_NO_RESPONSE_LOG, &uOPDData);

                    LOG_MSG(db_APP_SYSTEM, "%s No Respone (%s)\n", sMonTaskInfo[eTaskID].acTaskName, sMonTaskInfo[eTaskID].acLogInfoStr);
                }
            }

            utilCounterSet(sMonTaskInfo[eTaskID].eCounterType, 1000);
        }
    }
}

void palSystem_TaskMonitorTimerReset(eTASK_ID eTaskID, char *InfoStr)  //A35G2_Simon_0075
{
    if(InfoStr != NULL)
    {
        snprintf(sMonTaskInfo[eTaskID].acLogInfoStr, 64, "%s", InfoStr);
    }

    //reset wait response count down timer
    sMonTaskInfo[eTaskID].uiCountdownTimer = sMonTaskInfo[eTaskID].uiWaitResponseTime;

    //reset max log out times
    sMonTaskInfo[eTaskID].ucCountdownMaxLogOut = sMonTaskInfo[eTaskID].ucMaxLogOutNum;
}

eRESULT palSystem_WriteCustomizedEDID(eCM_SOURCE_ID eSource)  //A35G2_Simon_0091
{
    eRESULT eResult = rcERROR;

    UINT8 ucStatus = (UINT8)eCES_PROCESSING;

    eDATA_CODE eStatusDatacode = edcCUSTOMIZE_EDID_HDMI1;
    switch(eSource)
    {
        case eCM_SOURCE_HDMI1:  eStatusDatacode = edcCUSTOMIZE_EDID_HDMI1; break;
        case eCM_SOURCE_HDMI2:  eStatusDatacode = edcCUSTOMIZE_EDID_HDMI2; break;

        default:
            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : Source ID error (%d)\n", __FUNCTION__, __LINE__, eSource);
            return rcERROR;
    }

    //set processing status
    ucStatus = (UINT8)eCES_PROCESSING;
    palDataMgr_Data_Access(eStatusDatacode, edaWRITE_THROUGH_NO_ACTION, &ucStatus);

    //Get file name
    char cFileName[64];
    switch(eSource)
    {
        case eCM_SOURCE_HDMI1:
            snprintf(cFileName, sizeof(cFileName), "%s", CUSTOMIZE_EDID_HDMI1_FILE);
            break;

        case eCM_SOURCE_HDMI2:
            snprintf(cFileName, sizeof(cFileName), "%s", CUSTOMIZE_EDID_HDMI2_FILE);
            break;

        default:
            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : Source ID error (%d)\n", __FUNCTION__, __LINE__, eSource);
            return rcERROR;
    }

    //Get EDID data and send to System54605 to write
    UINT8 aucEDID[256];
    memset(aucEDID, 0, sizeof(aucEDID));
    if(utilMisc_GetFileData((INT8 *)cFileName, aucEDID, sizeof(aucEDID)) == UTILMISC_EXEC_PASS)
    {
        uCUSTOMIZE_EDID_DATA uSendData;
        uSendData.sEDID_DATA.ucSourceID = (UINT8)eSource;
        memcpy(uSendData.sEDID_DATA.ucEDID, aucEDID, sizeof(aucEDID));

        //check header  //00h FFh FFh FFh FFh FFh FFh 00h
        UINT8 aucEDID_Header[8] = {0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00};
        if(memcmp(uSendData.sEDID_DATA.ucEDID, aucEDID_Header, sizeof(aucEDID_Header)) != 0)
        {
            //set header error status
            ucStatus = (UINT8)eCES_HEADER_ERROR;
            palDataMgr_Data_Access(eStatusDatacode, edaWRITE_THROUGH_NO_ACTION, &ucStatus);
            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : error\n", __FUNCTION__, __LINE__);
            return rcERROR;
        }

        //check checksum
        UINT8 ucBlock0_Sum = 0;
        UINT8 ucBlock1_Sum = 0;
        for(int i=0 ; i<128 ; i++)
        {
            ucBlock0_Sum += uSendData.sEDID_DATA.ucEDID[i] ;
        }
        for(int i=128 ; i<256 ; i++)
        {
            ucBlock1_Sum += uSendData.sEDID_DATA.ucEDID[i] ;
        }
        if(ucBlock0_Sum != 0 || ucBlock1_Sum != 0)
        {
            //set header error status
            ucStatus = (UINT8)eCES_CHECKSUM_ERROR;
            palDataMgr_Data_Access(eStatusDatacode, edaWRITE_THROUGH_NO_ACTION, &ucStatus);
            LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : error\n", __FUNCTION__, __LINE__);
            return rcERROR;
        }


        //send to System54605
        LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : Time Start %d\n", __FUNCTION__, __LINE__, TMO_GetSysRunTime());
        eResult = halFrontEndCtrl_CustomizedEDID_Set((UINT8 *)&uSendData);
        LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : Time End %d\n", __FUNCTION__, __LINE__, TMO_GetSysRunTime());
    }
    else
    {
        LOG_MSG(db_APP_DATAPATH, "(func:%s, line:%d) : File error\n", __FUNCTION__, __LINE__);
        return rcERROR;
    }

    ucStatus = (UINT8)eCES_ACTIVE;
    palDataMgr_Data_Access(eStatusDatacode, edaWRITE_THROUGH_NO_ACTION, &ucStatus);

    return eResult;
}


void palSystem_Apply_AP_Mode_GeoFunc(void)  //A35G2_Simon_0100
{
    LOG_MSG(db_HAL_WARPING, "palSystem_ApplyGeoFunc Start\n");

    UINT8 ucWarpingApplySetting = 0;
    palDataMgr_Data_Access(edcWARP_MEMORY_APPLY, edaREAD, &ucWarpingApplySetting);

    palGeo_AdvWarpControl(WARP_CTRL__AP);

    if( access(BLENDING_AP_WARP_TPX_CURRENT_FILENAME, R_OK) == 0 ||
        access(BLENDING_AP_WARP_TPY_CURRENT_FILENAME, R_OK) == 0 ||
        access(BLENDING_AP_DBD_TABLE_CURRENT_FILENAME, R_OK) == 0 ||
        access(BLENDING_AP_BKLEVEL_TABLE_CURRENT_FILENAME, R_OK) == 0 ||
        access(BLENDING_AP_BKLEVEL_PALETTE_CURRENT_FILENAME, R_OK) == 0 )
    {
        LOG_MSG(db_HAL_WARPING, "Apply Current AP setting\n");
        //palGeo_Func_Set(eGFN_AP_WARP_FUNCTION_APPLY, NULL);
        //halWarping_MemoryApplyAP_CurrentSetting();
    }
    else if(palGeo_ApLinkFlag_Get() == TRUE)  //A35G2_Simon_0086
    {
        LOG_MSG(db_HAL_WARPING, "Blending AP on , no action\n");
    }
    else if(palDataMgr_CurrentWarpMemoryType() == WARPING_TYPE_AP)
    {
        LOG_MSG(db_HAL_WARPING, "Apply memory AP %d\n", ucWarpingApplySetting);
        palDataMgr_Access_Warping_Apply(edaWRITE_THROUGH_WITH_ACTION, &ucWarpingApplySetting);
    }
    else
    {
        LOG_MSG(db_HAL_WARPING, "reset All\n");
        palGeo_Func_Set(eGFN_AP_CLEAR_ALL, NULL);
    }

    LOG_MSG(db_HAL_WARPING, "WarpControl End AP\n");

}

BOOL palSystem_SystemCheckGet(void)//HICC2_Julie_0046
{
    return m_bSystemCheck;
}

void Init_GEC_Callback(void)
{
    sGEC_CALLBACK sGEC_Cb;
    sGEC_Cb.fpSetTpCallback = palDataMgr_TestPatternSet;
    sGEC_Cb.fpCCT_StorageSetCallback = palDataMgr_CCT_Set;	//HICC2_Doulas_0003 //HICC2_Casper_0014 //HICC2_Casper_0025
    sGEC_Cb.fpCCT_StorageGetCallback = palDataMgr_CCT_Get;	//HICC2_Doulas_0003 //HICC2_Casper_0014 //HICC2_Casper_0025

    GEC_RegCallback(sGEC_Cb);
}

void Init_Formatter_Callback(void)
{
    sFORMATTER_MGR_CALLBACK sFormatter_Cb;
    sFormatter_Cb.fpGetPictureModeCb = palDataMgr_PictureModeGet;

    palFormatterMgr_RegCallback(sFormatter_Cb);
}

void Init_Database_Callback(void)
{
    sUTILDATABASE_CALLBACK sDatabase_Cb;
    sDatabase_Cb.fpDataMgr_DataCodeStringGetCb = palDataMgr_DataCodeStringGet;

    utilDatabase_RegCallback(sDatabase_Cb);
}

void Init_HPBUTester_Callback(void)
{
    sUtilHPBUTester_CALLBACK sHPBUTester_Cb ;
    sHPBUTester_Cb.fpDataMgr_ServiceModeSetCb =                  palDataMgr_ServiceModeSet;
#ifdef PALDATAMGR_ACCESS_WITHLOG
    sHPBUTester_Cb.fpDataMgr_Data_AccessCb =                     palDataMgr_Data_Access_WithLog;
#else
    sHPBUTester_Cb.fpDataMgr_Data_AccessCb =                     palDataMgr_Data_Access;
#endif
    //sHPBUTester_Cb.fpDataMgr_Data_StructDataAccessCb =           palDataMgr_Data_StructDataAccess;
    sHPBUTester_Cb.fpDataMgr_Access_Get_FAN_DutyCb =             palDataMgr_Access_Get_FAN_Duty;
    sHPBUTester_Cb.fpDataMgr_Access_Get_FAN_RPMCb =              palDataMgr_Access_Get_FAN_RPM;
    //sHPBUTester_Cb.fpIllumination_TEC_Gating_ResultGetCb =       palIllumination_TEC_Gating_ResultGet;
    sHPBUTester_Cb.fpDataMgr_Access_Get_LD_VoltageCb =           palDataMgr_Access_Get_LD_Voltage;
    sHPBUTester_Cb.fpDataMgr_Access_Get_LD_CurrentCb =           palDataMgr_Access_Get_LD_Current;
    sHPBUTester_Cb.fpDataMgr_Access_Get_LD_TemperatureCb =       palDataMgr_Access_Get_LD_Temperature;
    sHPBUTester_Cb.fpDataMgr_Access_Get_TEC_CurrentCb =          palDataMgr_Access_Get_TEC_Current;
    sHPBUTester_Cb.fpDataMgr_Access_Get_Thermal_SensorCb =       palDataMgr_Access_Get_Thermal_Sensor;
    sHPBUTester_Cb.fpIllumination_CurrentRunTimeGetCb =          palIllumination_CurrentRunTimeGet;
    sHPBUTester_Cb.fpDataMgr_Access_Get_ALTIMETRYCb =            palDataMgr_Access_Get_ALTIMETRY;
    sHPBUTester_Cb.fpDataMgr_DateTime_TimeZoneStr_SetCb =        palDataMgr_DateTime_TimeZoneStr_Set;
    sHPBUTester_Cb.fpDataMgr_DateTime_TimeZoneStr_GetCb =        palDataMgr_DateTime_TimeZoneStr_Get;
    sHPBUTester_Cb.fpDataMgr_OPDRegulatoryInfoCb =               palDataMgr_OPDRegulatoryInfo;
    sHPBUTester_Cb.fpDataMgr_LD_Info_Num_GetCb =                 palDataMgr_LD_Info_Num_Get;
    sHPBUTester_Cb.fpDataMgr_BLD_Num_GetCb =                     palDataMgr_BLD_Num_Get;
    sHPBUTester_Cb.fpDataMgr_RLD_Num_GetCb =                     palDataMgr_RLD_Num_Get;

    utilHPBUTest_CLI_RegCallback(sHPBUTester_Cb);
}

void Init_IPC_Callback(void)
{
    sUTILIPC_CALLBACK sIPC_Cb;
    sIPC_Cb.fpDataMgr_Get_Camera_Working_StatusCb =                       palDataMgr_Get_Camera_Working_Status;
    sIPC_Cb.fpEnvironment_Camera_Force_Update_Power_Status_SetCb =        palEnvironment_Camera_Force_Update_Power_Status_Set;
    sIPC_Cb.fpSystem_PowerDownCb =                                        palSystem_PowerDown;
    sIPC_Cb.fpSystem_PowerKeyCb =                                         palSystem_PowerKey;
    sIPC_Cb.fpSystem_FwUpgradeModeCb =                                    palSystem_FwUpgradeMode;
    sIPC_Cb.fpInputProc_InputKeyListIDCb =                                palInputProc_InputKeyListID;
    sIPC_Cb.fpDataMgr_Data_TypeCb =                                       palDataMgr_Data_Type;
    sIPC_Cb.fpDataMgr_DataCode_ControlCb =                                palDataMgr_DataCode_Control;
    sIPC_Cb.fpDataMgr_DataSizeGetCb =                                     palDataMgr_DataSizeGet;

    utilIPC_RegCallback(sIPC_Cb);
}

void Init_WarpDemo_Callback(void)
{
    sUTILWARPDEMO_CALLBACK sWarpDemo_Cb;
    sWarpDemo_Cb.fpDataMgr_ACU_Target_Select_Tmp_GetCb = palDataMgr_ACU_Target_Select_Tmp_Get;
    //sWarpDemo_Cb.fpGui_OSD_ON_SetCb = appGui_OSD_ON_Set;
    //sWarpDemo_Cb.fpGui_SendUpdateOSDEventCb = appGui_SendUpdateOSDEvent;

    utilWarpDemo_RegCallback(sWarpDemo_Cb);
}

void Init_CLICmd_Callback(void)
{
    sUTILCLICMD_CALLBACK sCLICmd_Cb;
#ifdef PALDATAMGR_ACCESS_WITHLOG
    sCLICmd_Cb.fpDataMgr_Data_AccessCb =                            palDataMgr_Data_Access_WithLog;
#else
    sCLICmd_Cb.fpDataMgr_Data_AccessCb =                            palDataMgr_Data_Access;
#endif
    sCLICmd_Cb.fpInputProc_BufferInsertCb =                         palInputProc_BufferInsert;
    sCLICmd_Cb.fpSystem_PowerStateGetCb =                           palSystem_PowerStateGet;
    sCLICmd_Cb.fpSystem_HostReadyGetCb =                            palSystem_HostReadyGet;
    sCLICmd_Cb.fpSystem_HostReadySetCb =                            palSystem_HostReadySet;
    sCLICmd_Cb.fpSystem_WarmUpCb =                                  palSystem_WarmUp;
    sCLICmd_Cb.fpSystem_PowerDownCb =                               palSystem_PowerDown;
    sCLICmd_Cb.fpSystem_Enter2WModeCb =                             palSystem_Enter2WMode;
    sCLICmd_Cb.fpSystem_ModelIDGetCb =                              Board_ModelID_Get;

    sCLICmd_Cb.fpDataMgr_ResetAllToDefaultCb =                      palDataMgr_ResetAllToDefault;

    sCLICmd_Cb.fpEnvironment_InstantCoolingCb =                     palEnvironment_InstantCooling;
    sCLICmd_Cb.fpEnvironment_Fake_Power_Down_SetCb =                palEnvironment_Fake_Power_Down_Set;
    sCLICmd_Cb.fpEnvironment_Fake_Power_Down_GetCb =                palEnvironment_Fake_Power_Down_Get;

    //sCLICmd_Cb.fpGui_SendKeyEventCb =                               palGui_SendKeyEvent;
    sCLICmd_Cb.fpDataMgr_Data_TypeCb =                              palDataMgr_Data_Type;
    sCLICmd_Cb.fpDataMgr_DataCode_ControlCb =                       palDataMgr_DataCode_Control;
    sCLICmd_Cb.fpGui_PIN_Protect_CheckingCb =                       palDataMgr_PIN_Protect_Checking; //HICC2_Doulas_0134
    sCLICmd_Cb.fpEnvironment_NetworkIsReceivePowerOnCmd_GetCb =     palEnvironment_NetworkIsReceivePowerOnCmd_Get;
    sCLICmd_Cb.fpDataMgr_DataCodeStringGetCb =                      palDataMgr_DataCodeStringGet;
    sCLICmd_Cb.fpDataMgr_Data_Range_GetCb =                         palDataMgr_Data_Range_Get;
    sCLICmd_Cb.fpDataMgr_IsNetworkDataCodeCb =                      palDataMgr_IsNetworkDataCode;
    //sCLICmd_Cb.fpGui_DataCode_Value_SetCb =                         palGui_DataCode_Value_Set;
    //sCLICmd_Cb.fpGui_DataCode_Value_GetCb =                         palGui_DataCode_Value_Get;
    sCLICmd_Cb.fpDataMgr_OPDEventCb =                               palDataMgr_OPDEvent;
    //sCLICmd_Cb.fpGui_DataCode_String_SetCb =                        palGui_DataCode_String_Set;
    sCLICmd_Cb.fpDataMgr_LanArgCopy2Cb =                            palDataMgr_LanArgCopy2;
    sCLICmd_Cb.fpDataMgr_LanArg_Send_McuCb =                        palDataMgr_LanArg_Send_Mcu;
    sCLICmd_Cb.fpDataMgr_WLanArgCopy2Cb =                           palDataMgr_WLanArgCopy2;
    sCLICmd_Cb.fpDataMgr_CrestronArgCopy2Cb =                       palDataMgr_CrestronArgCopy2;
    sCLICmd_Cb.fpDataMgr_PJLinkArgCopy2Cb =                         palDataMgr_PJLinkArgCopy2;
    sCLICmd_Cb.fpGui_SendUpdateOSDEventCb =                         palSystem_NULL_Function; //palGui_SendUpdateOSDEvent;
    sCLICmd_Cb.fpGui_Send_LensCalibrationMenuOpenCb =               palSystem_NULL_Function; //palGui_Send_LensCalibrationMenuOpen;
    sCLICmd_Cb.fpGui_Send_LensMovingMenuOpenCb =                    palSystem_NULL_Function; //palGui_Send_LensMovingMenuOpen;
    sCLICmd_Cb.fpGui_Get_MenuState_IsOSD_OpenCb =                   palDataMgr_IsOsdOpen; //palGui_Get_MenuState_IsOSD_Open;
    sCLICmd_Cb.fpGui_Send_OSD_ExitCb =                              palSystem_NULL_Function; //palGui_Send_OSD_Exit;
    sCLICmd_Cb.fpLANProcUpdateOSD_SetCb =                           palLANProcUpdateOSD_Set;
    sCLICmd_Cb.fpLANProcSendToLANCb =                               palLANProcSendToLAN;
    sCLICmd_Cb.fpEnvironment_AutoShutDownClearCb =                  palEnvironment_AutoShutDownClear;
    sCLICmd_Cb.fpLANProcSendAckDoneCb =                             palLANProcSendAckDone;
    sCLICmd_Cb.fpDataMgr_LAN_IP_Copy2_AllCb =                       palDataMgr_LAN_IP_Copy2_All;
    sCLICmd_Cb.fpDataMgr_DateTime_Unpack_LanPacketCb =              palDataMgr_DateTime_Unpack_LanPacket;
    sCLICmd_Cb.fpDataMgr_Schedule_Unpack_LanPacketCb =              palDataMgr_Schedule_Unpack_LanPacket;
    sCLICmd_Cb.fpDataMgr_ACU_Target_Select_SetCb =                  palDataMgr_ACU_Target_Select_Set;
    sCLICmd_Cb.fpDataPath_BackupConfigFlagCb =                      palDataPath_BackupConfigFlag;
    //sCLICmd_Cb.fpDataMgr_XFPGA_LVDS_COVER_SetCb =                   palDataMgr_XpalGA_LVDS_COVER_Set;
    sCLICmd_Cb.fpDataPath_InputDetectSetCb =                        palDataPath_InputDetectSet;
    sCLICmd_Cb.fpDataMgr_OPDInputPlugInStateCb =                    palDataMgr_OPDInputPlugInState;
    sCLICmd_Cb.fpDataPath_LastInputDetectSetCb =                    palDataPath_LastInputDetectSet;
    sCLICmd_Cb.fpDataMapping_CMValueRangeCheckCb =                  utilDataMapping_CMValueRangeCheck;
    sCLICmd_Cb.fpGui_SendPinProtectClearEventCb =                   palDataMgr_UI_EVENT_Pin_Protect_Clear;//appGui_SendPinProtectClearEvent;
    sCLICmd_Cb.fpGui_SendDataCodeAndUpdateEventCb =                 palDataMgr_UI_DataCodeAndUpdateOSDEvent;
    sCLICmd_Cb.fpGui_Send_ABP_Cal_Msg_OpenCb =                      palDataMgr_UI_EVENT_ABP_Cal_Msg_Open;
    sCLICmd_Cb.fpDataMgr_IPV6LanArgCopy2Cb =                        palDataMgr_IPV6_LanArgCopy2; //HICC2_AC_0055 // HICC2_Bruce_0020
    sCLICmd_Cb.fpDataMgr_IPV6LanArg_Send_McuCb =                    palDataMgr_IPV6LanArg_Send_Mcu;

    utilCLICmd_RegCallback(sCLICmd_Cb);
}

#if 0
void Init_CommonUserCLI_Callback(void)
{
    sUTILCOMMONCLI_CALLBACK sCommonUserCLI_Cb;
    sCommonUserCLI_Cb.fpDataMapping_CMValueRangeCheckCb =           utilDataMapping_CMValueRangeCheck;
    //sCommonUserCLI_Cb.fpGui_DataCode_Value_SetCb =                  NULL;
    sCommonUserCLI_Cb.fpLANProcTelnetReplyCb =                      palLANProcTelnetReply;
    sCommonUserCLI_Cb.fpDataMgr_DataCode_ControlCb =                palDataMgr_DataCode_Control;
    sCommonUserCLI_Cb.fpDataMgr_Data_TypeCb =                       palDataMgr_Data_Type;
    sCommonUserCLI_Cb.fpDataMgr_Data_AccessCb =                     palDataMgr_Data_Access;
    //sCommonUserCLI_Cb.fpGui_DataCode_Value_GetCb =                  NULL;
    //sCommonUserCLI_Cb.fpGui_DataCode_PointerVar_SetCb =             NULL;
    sCommonUserCLI_Cb.fpSystem_CustomIDGetCb =                      palSystem_ModelIDGet;
    sCommonUserCLI_Cb.fpDbgMsg_ftraceCb =                           utilDbgMsg_ftrace;
    sCommonUserCLI_Cb.fpHost_SystemSetCb =                          utilHost_SystemSet;

    utilCommonCLI_RegCallback(sCommonUserCLI_Cb);
}
#endif

void palSystem_NULL_Function(void)
{
    //H2 wait review
}

UINT8 palSystem_SingleiScalerModeFlagGet(void)
{
    return m_sPalSysInfo.ucSingle_iScaler_Mode;
}
void palSystem_SingleiScalerModeFlagSet(UINT8 Data)
{
    m_sPalSysInfo.ucSingle_iScaler_Mode = Data;
}


void palSystem_PanelResolution_Get(ePANEL_ID ePanelId, UINT16 *HRes, UINT16 *VRes)
{
    switch(ePanelId)
    {
        case ePANEL_ID_XGA_60HZ:
        case ePANEL_ID_XGA_120HZ:
            *HRes = 1024;
            *VRes = 768;
            break;

        case ePANEL_ID_WXGA_60HZ:
        case ePANEL_ID_WXGA_120HZ:
            *HRes = 1280;
            *VRes = 800;
            break;

        case ePANEL_ID_1600x1200_60HZ:
            *HRes = 1600;
            *VRes = 1200;
            break;

        case ePANEL_ID_WUXGA_60HZ:
        case ePANEL_ID_WUXGA_120HZ:
        case ePANEL_ID_WUXGA_240HZ:
            *HRes = 1920;
            *VRes = 1200;
            break;
        case ePANEL_ID_WQXGA_60HZ:
            *HRes = 2560;
            *VRes = 1600;
            break;

        case ePANEL_ID_720P_50HZ:
        case ePANEL_ID_720P_60HZ:
        case ePANEL_ID_720P_120HZ:
            *HRes = 1280;
            *VRes = 720;
            break;

        case ePANEL_ID_1080P_50HZ:
        case ePANEL_ID_1080P_60HZ:
        case ePANEL_ID_1080P_120HZ:
        case ePANEL_ID_1080P_240HZ:
            *HRes = 1920;
            *VRes = 1080;
            break;

        case ePANEL_ID_3840x2160_50HZ:
        case ePANEL_ID_3840x2160_60HZ:
            *HRes = 3840;
            *VRes = 2160;
            break;

        case ePANEL_ID_3840x2400_60HZ:
            *HRes = 3840;
            *VRes = 2400;
            break;

        case ePANEL_ID_4096x2160_50HZ:
        case ePANEL_ID_4096x2160_60HZ:
            *HRes = 4096;
            *VRes = 2160;
            break;

        case ePANEL_ID_960x2160_50HZ:
        case ePANEL_ID_960x2160_60HZ:
            *HRes = 960;
            *VRes = 2160;
            break;

        case ePANEL_ID_1280x2160_50HZ:
        case ePANEL_ID_1280x2160_60HZ:
            *HRes = 1280;
            *VRes = 2160;
            break;

        case ePANEL_ID_2688x1472_120HZ:
            *HRes = 2688;
            *VRes = 1472;
            break;

        case ePANEL_ID_2716x1528_120HZ:
            *HRes = 2716;
            *VRes = 1528;
            break;

        default:
        case ePANEL_ID_LAST:
            *HRes = 1920;
            *VRes = 1080;
            LOG_MSG(db_ALWAYS, "(func:%s,line:%d) unknown Panel ID %d\n", ePanelId);
            break;
    }

}

eEXEC_CODE palSystem_PowerStandby_OSDExit(void)
{
    eEXEC_CODE eResult = eEXEC_CODE_PASS;

    eResult = palDataMgr_UI_EventSend(edcUI_EVENT_GO_STANDBY, TRUE, NULL);

	MS_SLEEP(200);

    return eResult;
}


