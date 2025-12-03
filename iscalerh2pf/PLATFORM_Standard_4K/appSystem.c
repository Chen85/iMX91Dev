#include "appSystem.h"
#include "appDataPath.h"
#include "appPoll.h"
//#include "appGui.h"
#include "appCLICmd.h"
#include "appDataMgr.h"
#include "appIllumination.h"
#include "appEnvironment.h"
#include "appHostPath.h"
#include "appInputProc.h"
#include "appIapProcAPI.h" //A70LV_Larry_0079
#include "appLANProcAPI.h" //A70LV_Larry_0172
#include "appCLICmd.h"
#include "appHostPath.h"
#include "appLedProcAPI.h"

#include "palCoreVar.h"
#include "palGui.h"
#include "palFormatterMgr.h"
#include "palMotorMgr.h"
#include "palGeoAPI.h"
#include "palSysCtrlMgr.h"
#include "palImgMgr.h"

#include "halFormatter.h"
#include "halLDProc.h"
#include "halFrontEndCtrlAPI.h"
#include "halMotorCtrlAPI.h"
#include "halMCUCtrlAPI.h"
#include "halBoardCtrlAPI.h"
#include "halGui.h"
#include "halCFUCtrlAPI.h"
#include "halFanCtrlAPI.h"
#include "halScaler.h"

#ifdef SCALER_FPGA_F34
#include "halRsu_ProAV.h"
#include "dvProAV_Base.h"
#endif

#include "utilCLICmdAPI.h"
#include "utilDbgMsg.h"
#include "utilIPCAPI.h"
#include "utilDatabaseAPI.h"
#include "utilDataMgrAPI.h"
#include "utilDataMapping.h"
#include "utilMisc.h"

#include "timeout.h"

#include "GEC_EventTable.h"
#include "GEC_CoreFunction.h"
#include "utilHPBU_Tester.h"

//#include "MemMap.h"
#if(BIST_ENABLE)
#include "utilBIST.h"
#endif

#ifdef CUSTOM_OPTOMA
#include "utilOptomaMSSCAPI.h"
#endif

#include "conf.h"

sGUI_CALLBACK GuiCb;
static sPAL_SYSTEM_INFORMATION  m_sPalSysInfo;                 //Use for system operation

//Default system settings, like TI gpConfiguration
//It is read only variable, thus do not need semphore protect
static sSYSTEM_CONFIGURATION       m_sDefSysConfiguration;

static BOOL     m_bNoFanControlFlag = FALSE; //A70LV_Larry_0079
static BOOL     m_bIsAsicInitialed = FALSE;
static UINT8    m_bAsicReadyCheckCount = 20; // Retry to check ASIC ready
static BOOL     m_bIs12VPowerLost = FALSE;  //G100_Owen_0064
static BOOL     m_bMenuUpgradeCheck = FALSE;
static BOOL     m_bIsLogoCapture = FALSE;	//A65_OPTOMA_Doulas_0125
static UINT8    m_cLensModel = 0; //A35G2_Larry_0063
static BOOL     m_bSystemCheck = 0;
static BOOL     m_bActuatorEnable = 0;  //A70LK_Doulas_0018

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

#ifdef OPFU_TAG_ENABLE
static void palSystem_OPFU_Tag(void);
#endif /* OPFU_TAG_ENABLE */

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

static void palSystem_PowerOnRecoverEvent(BOOL bVal)
{
    uOPD_DATA uOPDData = {0};

    uOPDData.sSnapshotLog.ulDebug = (UINT32)bVal;
    utilOPD_EventSet(eOPD_DEBUG_LOG, &uOPDData);
}


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
            palSystem_HostReadySet(1);
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
    if(palMailBox_Send_MsgQueue(m_sPalSysInfo.xMsgQueue, QUEUE_SYSTEM_NAME, eSYSTEM_EVENT_CUSTOMIZE_EDID, -1, (UINT32)ucSourceID, &temp) != eMAIL_BOX_EXEC_CODE_PASS)
#else
    if(palMailBox_Send(m_sPalSysInfo.xMsgQueue, m_sPalSysInfo.xEventGroupHandle, eSYSTEM_EVENT_CUSTOMIZE_EDID, -1, (UINT32)ucSourceID, &temp, FALSE) != eMAIL_BOX_EXEC_CODE_PASS)
#endif
    {
        LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d) : Fail!\r\n", __FUNCTION__, __LINE__);
    }
}

void palSystem_ActuatorPatternSet(UINT8 ucPatten)
{
    UINT32 temp;

#if (SYSTEM_OS_TYPE == POSIX_COMPLIANT)
    if(palMailBox_Send_MsgQueue(m_sPalSysInfo.xMsgQueue, QUEUE_SYSTEM_NAME, eSYSTEM_EVENT_ACTUATOR_PATTERN, -1, (UINT32)ucPatten, &temp) != eMAIL_BOX_EXEC_CODE_PASS)
#else
    if(palMailBox_Send(m_sPalSysInfo.xMsgQueue, m_sPalSysInfo.xEventGroupHandle, eSYSTEM_EVENT_ACTUATOR_PATTERN, -1, (UINT32)ucPatten, &temp, FALSE) != eMAIL_BOX_EXEC_CODE_PASS)
#endif
    {
        LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d) : Fail!\r\n", __FUNCTION__, __LINE__);
    }
}


void palSystem_WarpPattern_Event(ePAL_GEO_FUNCTION eFunction, uPALGEOAPI_INFO uData)
{
    sDRAW_WARP_QUEUE_DATA Data = {eFunction, uData};

#if (SYSTEM_OS_TYPE == POSIX_COMPLIANT)
    if(palMailBox_Send_MsgQueue(m_sPalSysInfo.xMsgQueue, QUEUE_SYSTEM_NAME, eSYSTEM_EVENT_WARP_PATTERN, 100, (UINT32)eFunction, (UINT32 *)&Data) != eMAIL_BOX_EXEC_CODE_PASS)
#else
    if(palMailBox_Send(m_sPalSysInfo.xMsgQueue, m_sPalSysInfo.xEventGroupHandle, eSYSTEM_EVENT_WARP_PATTERN, -1, (UINT32)eFunction, &Data, FALSE) != eMAIL_BOX_EXEC_CODE_PASS)
#endif
    {
        LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d) : Fail!\r\n", __FUNCTION__, __LINE__);
    }
}

void palSystem_WarpPattern_DrawRect_Event(eWARPOSD_INSERT_LOCATION eWarpOsdPosition,
                                                       UINT16 PosX,
                                                       UINT16 PosY,
                                                       UINT16 Width,
                                                       UINT16 Height,
                                                       UINT16 ColorIdx
                                                       )
{
    #ifdef SAME_RAM_FOR_BEFORE_WARP_OSD_AND_SCALER_OSD
    if(eWarpOsdPosition == eWIL_BEFORE_WARP)
    {
        if(palGeo_Is4kWarp())
        {
            PosX = PosX / 2;
            PosY = PosY / 2;
            Width = (Width+1) / 2;
            Height = (Height+1) / 2;
        }
        else
        {
            //not change
        }

        Width  = (Width  == 0) ? 1 : Width;
        Height = (Height == 0) ? 1 : Height;
    }
    #endif

    uPALGEOAPI_INFO uData = {.sGFN_DRAW_RECT_INFO.eInsertLocation = eWarpOsdPosition,
                             .sGFN_DRAW_RECT_INFO.PosX = PosX,
                             .sGFN_DRAW_RECT_INFO.PosY = PosY,
                             .sGFN_DRAW_RECT_INFO.Width = Width,
                             .sGFN_DRAW_RECT_INFO.Height = Height,
                             .sGFN_DRAW_RECT_INFO.ColorIdx = ColorIdx
                             };

#if (SYSTEM_OS_TYPE == POSIX_COMPLIANT)
    if(palMailBox_Send_MsgQueue(m_sPalSysInfo.xMsgQueue, QUEUE_SYSTEM_NAME, eSYSTEM_EVENT_WARP_PATTERN, 1000, (UINT32)eGFN_DRAW_RECT, (UINT32 *)&uData) != eMAIL_BOX_EXEC_CODE_PASS)
#else
    if(palMailBox_Send(m_sPalSysInfo.xMsgQueue, m_sPalSysInfo.xEventGroupHandle, eSYSTEM_EVENT_WARP_PATTERN, 1000, (UINT32)eGFN_DRAW_RECT, (UINT32 *)&uData, FALSE) != eMAIL_BOX_EXEC_CODE_PASS)
#endif
    {
        LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d) : Fail!\r\n", __FUNCTION__, __LINE__);
    }
}


void palSystem_WarpPattern_OsdOn_Event(eWARPOSD_INSERT_LOCATION eWarpOsdPosition)
{
    uPALGEOAPI_INFO uData = {.sGFN_DRAW_OSD_INFO.eLocation = eWarpOsdPosition };

#if (SYSTEM_OS_TYPE == POSIX_COMPLIANT)
    if(palMailBox_Send_MsgQueue(m_sPalSysInfo.xMsgQueue, QUEUE_SYSTEM_NAME, eSYSTEM_EVENT_WARP_PATTERN, 1000, (UINT32)eGFN_DRAW_OSD, (UINT32 *)&uData) != eMAIL_BOX_EXEC_CODE_PASS)
#else
    if(palMailBox_Send(m_sPalSysInfo.xMsgQueue, m_sPalSysInfo.xEventGroupHandle, eSYSTEM_EVENT_WARP_PATTERN, 1000, (UINT32)eGFN_DRAW_OSD, (UINT32 *)&uData, FALSE) != eMAIL_BOX_EXEC_CODE_PASS)
#endif
    {
        LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d) : Fail!\r\n", __FUNCTION__, __LINE__);
    }
}


void palSystem_WarpPattern_ClearOsd_Event(eWARPOSD_INSERT_LOCATION eWarpOsdPosition)
{
    uPALGEOAPI_INFO uData = {.sGFN_DRAW_OSD_INFO.eLocation = eWarpOsdPosition };

#if (SYSTEM_OS_TYPE == POSIX_COMPLIANT)
    if(palMailBox_Send_MsgQueue(m_sPalSysInfo.xMsgQueue, QUEUE_SYSTEM_NAME, eSYSTEM_EVENT_WARP_PATTERN, 1000, (UINT32)eGFN_CLEAR_OSD, (UINT32 *)&uData) != eMAIL_BOX_EXEC_CODE_PASS)
#else
    if(palMailBox_Send(m_sPalSysInfo.xMsgQueue, m_sPalSysInfo.xEventGroupHandle, eSYSTEM_EVENT_WARP_PATTERN, 1000, (UINT32)eGFN_DRAW_OSD, (UINT32 *)&uData, FALSE) != eMAIL_BOX_EXEC_CODE_PASS)
#endif
    {
        LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d) : Fail!\r\n", __FUNCTION__, __LINE__);
    }
}

void palSystem_ApplyAPModeGeoFunc_Event(void)
{
    UINT32 temp;

#if (SYSTEM_OS_TYPE == POSIX_COMPLIANT)
    if(palMailBox_Send_MsgQueue(m_sPalSysInfo.xMsgQueue, QUEUE_SYSTEM_NAME, eSYSTEM_EVENT_APPLY_AP_MODE_GEO_FUNC, -1, 0, &temp) != eMAIL_BOX_EXEC_CODE_PASS)
#else
    if(appMailBox_Send(m_sAppSysInfo.xMsgQueue, m_sAppSysInfo.xEventGroupHandle, eSYSTEM_EVENT_APPLY_AP_MODE_GEO_FUNC, -1, 0, &temp, FALSE) != eMAIL_BOX_EXEC_CODE_PASS)
#endif
    {
        LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d) : Fail!\r\n", __FUNCTION__, __LINE__);
    }
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

    if(bPass == TRUE) //for crestron warmup
    {
        m_sPalSysInfo.ePowerState = ePOWER_STATE_WARMUP;
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
#if 0//(BIST_ENABLE)
    if(utilBIST_Status_Get() == 1)
    {
        utilBIST_Handle(1);
        utilBIST_Status_Set(0);
    }
#endif
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

static void palSystem_UpgradeSelect(eSYSTEM_UPGRADE_SELECT eSelect)
{
    switch(eSelect)
    {
        case eSYSTEM_UPGRADE_MODE_SET:
            if(m_sPalSysInfo.ePowerState == ePOWER_STATE_ACTIVE)
            {
                UINT8  ucEnable = eCM_STANDBY_MODE_COMMUNICATION;
                UINT32 ulTemp = 0;
                UINT8  ucValue = 0xFF;
                eDATA_CODE eDataCode = edcOPFU_CHECK; //A70Gen2_Julie_0097

#ifdef OPFU_TAG_ENABLE
                palSystem_OPFU_Tag();
#endif /* OPFU_TAG_ENABLE */
                palCoreVar_SetSystemPowerOnVar(FALSE);
                palCoreVar_SetIsCoolingVar(TRUE);
                palSystem_UpgradeEnable();

                palDataMgr_Data_Access(edcSTANDBY_MODE, edaWRITE_THROUGH_WITH_ACTION, (void*)&ucEnable);

                //palLedProc_LED_Behavior_Set(eLED_STATUS_FW_UPGRAGE);
                //palLedProc_Poll(1);
                palInputProc_DisableKey(1); //A70LV_Larry_0191
                palLANProcStartSet(0); //A70LV_Larry_0191

                m_sPalSysInfo.ePowerState = ePOWER_STATE_UPGRADE; //A70LV_Larry_0121

                halFormatter_Standby();
                halFormatter_Upgrade_Set(TRUE);  //A70LV_Larry_0138
                //halMCU_DDP_UpgradeSet(TRUE);
                //halMotorLensSet(eLENS_CMDS_CANCEL); //A70LV_Larry_0388
                palDataMgr_Data_Access(edcLENS_CALIBRATION, edaWRITE_THROUGH_WITH_ACTION, &ucValue);

                ucEnable = 1;

                palDataMgr_Data_Access(edcOPFU_CHECK, edaWRITE_THROUGH_WITH_ACTION, (void*)&ucEnable);   //G100_Simon_0064
                halMCU_OPFU_Set(TRUE);

                ucEnable = 0;

                MS_SLEEP(10);

                halMCU_OPFU_Get(&ucEnable);

                LOG_MSG(db_ALWAYS, "(func:%s, line:%d) : mcu opfu enable %d\r\n", __FUNCTION__, __LINE__, ucEnable);

                palLANProcPowerStateSend(ePOWER_STATE_UPGRADE, 4, (UINT8*)&ulTemp);

                palDataMgr_ImportDataToDatabase(eDataCode); //A70Gen2_Julie_0097
                utilIpc_SendData(eIPC_SEND_DATA_UPDATE_DATACODE_ITEM, 0, &eDataCode, eEXEC_CODE_PASS, sizeof(UINT16)*1); //A70Gen2_Julie_0097//HICC2_Julie_0009

                LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d) : fw upgrade mode enter\r\n", __FUNCTION__, __LINE__);

                #if (ENABLE_COLOR_UNIFORMITY == TRUE)       //G100_Tim_0025, add, start
                palGeo_Color_Uniformity_Init_Status_Set( WARPING_ACU_INIT_NOT_READY );
                #endif //ENABLE_COLOR_UNIFORMITY            //G100_Tim_0025, add, end
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
            break;

        case eSYSTEM_UPGRADE_SELECT_LPCMCU:
            {
                m_sPalSysInfo.ucFW_upgrade = 1;

                if(rcSUCCESS == palSystem_UpgradeLPCMCU(eLPCMCU_INDEX_SYSTEM, TRUE))
                {
                    LOG_MSG(db_UPGRADE, "System upgrade pass\r\n");
                }
                else
                {
                    LOG_MSG(db_UPGRADE, "System upgrade fail\r\n");
                }

                if(palSystem_UpgradeLPCMCU(eLPCMCU_INDEX_FRONT_4K, TRUE))
                {
                    LOG_MSG(db_UPGRADE, "Frontend upgrade pass\r\n");
                }
                else
                {
                    LOG_MSG(db_UPGRADE, "Frontend upgrade fail\r\n");
                }

                if(palSystem_UpgradeLPCMCU(eLPCMCU_INDEX_MOTORBD, TRUE))
                {
                    LOG_MSG(db_UPGRADE, "Motor board upgrade pass\r\n");
                }
                else
                {
                    LOG_MSG(db_UPGRADE, "Motor board upgrade fail\r\n");
                }

                m_sPalSysInfo.ucFW_upgrade = 0;
            }
            break;

        case eSYSTEM_UPGRADE_SELECT_SCALER:
            {
                m_sPalSysInfo.ucFW_upgrade = 1;

                palSystem_Scaler_Upgrade(FALSE);

                m_sPalSysInfo.ucFW_upgrade = 0;
            }
            break;

        case eSYSTEM_UPGRADE_SELECT_FPGA:
            if(rcSUCCESS == palSystem_Suspend(__FUNCTION__))
            {
                m_sPalSysInfo.ucFW_upgrade = 1;

                #ifdef SCALER_FPGA_F34
                halRsu_Upgrade_Partial(FW_FPGA480, APP_CODE_BASED_ADDRESS, FALSE);
                halRsu_FPGAVersionGet();
                #endif

                palSystem_Resume(__FUNCTION__);
                m_sPalSysInfo.ucFW_upgrade = 0;
            }
            break;

        case eSYSTEM_UPGRADE_SELECT_All:
            if(rcSUCCESS == palSystem_Suspend(__FUNCTION__))
            {
                UINT16 uiCount = 0;
                UINT16 uiState = 0;
                UINT8  cResult = 1;
                UINT32 Ver = 0;
                UINT8  subVer, minorVer;

                m_sPalSysInfo.ucFW_upgrade = 1;

                uiState = rcBUSY;
                utilIpc_SendData(eIPC_SEND_DATA_UPDATE_INFO_ITEM, eDI_UPGRADE_STATUS, (UINT8*)&uiState, eEXEC_CODE_PASS, 2);

                uiCount = 0;
                utilIpc_SendData(eIPC_SEND_DATA_UPDATE_INFO_ITEM, eDI_UPGRADE_PERCENTAGE, (UINT8*)&uiCount, eEXEC_CODE_PASS, 2);

#ifdef NO_INTERFACE

                MS_SLEEP(5000);
#else
                cResult &= palSystem_Scaler_Upgrade(FALSE);
#endif /* NO_INTERFACE */

                uiCount = 20;
                utilIpc_SendData(eIPC_SEND_DATA_UPDATE_INFO_ITEM, eDI_UPGRADE_PERCENTAGE, (UINT8*)&uiCount, eEXEC_CODE_PASS, 2);

#ifdef NO_INTERFACE
                for(uiCount = 21; uiCount < 100; uiCount++)
                {
                    MS_SLEEP(1000);
                    utilIpc_SendData(eIPC_SEND_DATA_UPDATE_INFO_ITEM, eDI_UPGRADE_PERCENTAGE, (UINT8*)&uiCount, eEXEC_CODE_PASS, 2);
                }
#else

                #ifdef SCALER_FPGA_F34
                Ver = halRsu_FPGAVersionGet();

                subVer = Ver&0xFF;
                minorVer = Ver&0xFF;

                if((subVer >= 8) && (minorVer >= 0))
                {
                    cResult &= halRsu_Upgrade_Partial(FW_FPGA480, APP_CODE_BASED_ADDRESS, FALSE);
                }
                #endif

#endif /* NO_INTERFACE */

                uiCount = 100;
                utilIpc_SendData(eIPC_SEND_DATA_UPDATE_INFO_ITEM, eDI_UPGRADE_PERCENTAGE, (UINT8*)&uiCount, eEXEC_CODE_PASS, 2);

                if(cResult == 1)
                {
                    uiState = rcSUCCESS;
                }
                else
                {
                    uiState = rcERROR;
                }
                utilIpc_SendData(eIPC_SEND_DATA_UPDATE_INFO_ITEM, eDI_UPGRADE_STATUS, (UINT8*)&uiState, eEXEC_CODE_PASS, 2);

                palSystem_Resume(__FUNCTION__);
                m_sPalSysInfo.ucFW_upgrade = 0;
            }
            break;

        case eSYSTEM_UPGRADE_TEST_1:
            palSystem_Scaler_Upgrade(TRUE);
            break;

        case eSYSTEM_UPGRADE_TEST_2:
            if(rcSUCCESS == palSystem_Suspend(__FUNCTION__))
            {
                m_sPalSysInfo.ucFW_upgrade = 1;

                #ifdef SCALER_FPGA_F34
                halRsu_Upgrade_Partial(FW_FPGA480, APP_CODE_BASED_ADDRESS, TRUE);
                halRsu_FPGAVersionGet();
                #endif

                palSystem_Resume(__FUNCTION__);
                m_sPalSysInfo.ucFW_upgrade = 0;
            }
            break;

        default:
            LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d) : fw upgrade select Not, item\r\n", __FUNCTION__, __LINE__);
            break;
   }
}

static void palSystem_CallBack(UINT16 uiMsgID, UINT32 ulParam1, UINT32 *pulParam2)
{
    switch(uiMsgID)
    {
        case eSYSTEM_EVENT_IDLE:
		{
			UINT32 Data = 0;

            LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d) : eSYSTEM_EVENT_IDLE\r\n", __FUNCTION__, __LINE__);
            m_bIsAsicInitialed = FALSE;
            m_sPalSysInfo.ePowerState = ePOWER_STATE_STANDBY;
            palSystem_HostReadySet(0);
            palSystem_SystemReadySet(0);
            halMCUCtrl_SystemState_Set((UINT8)m_sPalSysInfo.ePowerState);
			palLANProcPowerStateSend(ePOWER_STATE_STANDBY, 4, (UINT8*)&Data);
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

                if(rcSUCCESS == halMCU_SystemReadyGet(&ucHostReady))
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
                    UINT8 ucPowerOn_Recover = FALSE;
                    //utilStartupRead(ePARA_STARTUP_UPGRADE_LPC54113,    (void*)&ucUpgradeCheck);

                    palCoreVar_SetSystemPowerOnVar(TRUE);
                    m_sPalSysInfo.ePowerState = ePOWER_STATE_ACTIVE;

                    halMCU_OPFU_Get(&ucOPFUEnable);
                    if(ucOPFUEnable == TRUE)  //G100_Simon_0064  //OPFU is not complete
                    {
    					LOG_MSG(db_UPGRADE, "\r\n(func:%s, line:%d) OPFU is not complete, Entering OPFU Mode\r\n", __FUNCTION__, __LINE__);
                        palSystem_FwUpgradeMode(eSYSTEM_UPGRADE_MODE_SET);
                    }
                    palDataMgr_Power_On_Counter();// HICC2_Bruce_0007
                    palSystem_PowerOPDEvent(uiMsgID); //G100_Julie_00020
                    halMCUCtrl_PowerOn_Recover_Get(&ucPowerOn_Recover);
                    if(ucPowerOn_Recover == TRUE)
                    {
                        palSystem_PowerOnRecoverEvent(ucPowerOn_Recover);
                    }


                }
                else
                {
                    m_sPalSysInfo.ePowerState = ePOWER_STATE_COOLING;
                    LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d) : Can not start system!!!!!\r\n", __FUNCTION__, __LINE__);
                }

            }
            else if(m_sDefSysConfiguration.System.ucStartupState == eSTARTUP_STATE_STANDBY)
            {
                palSystem_Standby();
                m_sPalSysInfo.ePowerState = ePOWER_STATE_STANDBY;
                palSystem_HostReadySet(0);
            }
            break;

        case eSYSTEM_EVENT_POWERDOWN:
            LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d) : eSYSTEM_EVENT_POWERDOWN\r\n", __FUNCTION__, __LINE__);

            //disconnect Twist Link   //A70LK_Simon_0010
            UINT8 ucTwistLink = 1;
            palDataMgr_Data_Access(edcTWIST_LINK, edaREAD, &ucTwistLink);
            if(ucTwistLink != 0)
            {
                ucTwistLink = 0 ;
                palDataMgr_Data_Access(edcTWIST_LINK, edaWRITE_THROUGH_WITH_ACTION, &ucTwistLink);
            }

        #ifdef CUSTOM_OPTOMA		//A65_OPTOMA_Doulas_0069
			utilOptoma_OutputInfoMsg(sOptoma_System_Auto_Send_Lut[eOPT_COOLING_DOWN].cINFO_String);
        #endif

            if(palDataMgr_CurTestPatternGet() != eTID_OFF) //HICC2_Doulas_0128
            {
                palDataMgr_TestPatternHandle(eTID_OFF); //HICC2_Doulas_0128
            }
            if(m_sPalSysInfo.ePowerState == ePOWER_STATE_ACTIVE)
            {
                //G100_Tim_0007, mod, start
                UINT8 ucData = 0;   //G100_Clare_0055

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
                    palDataMgr_Power_Off_Counter();// HICC2_Bruce_0007
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
                    //halMotorLensSet(eLENS_CMDS_CANCEL); //A70LV_Larry_0388
                    //palLedProc_LED_Behavior_Set(eLED_STATUS_COOLING); //A70LV_Larry_0020
				}
				halFormatter_AsicReadySet(FALSE); //HICC2_Doulas_0041
            }
            else if((m_sPalSysInfo.ePowerState == ePOWER_STATE_UPGRADE) && (m_sPalSysInfo.ucFW_upgrade == 0)) //A70LV_Larry_0121
            {
                UINT8 ucValue = 0xFF;

                palSystem_Standby();
                halMCU_SystemCooling();
                //palLedProc_LED_Behavior_Set(eLED_STATUS_COOLING);
                palDataMgr_Data_Access(edcLENS_CALIBRATION, edaWRITE_THROUGH_WITH_ACTION, &ucValue);
                //halMotorLensSet(eLENS_CMDS_CANCEL); //A70LV_Larry_0388
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

            break;

        case eSYSTEM_EVENT_POWERKEY:
            halBoard_PowerOn();
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

            UINT8 ucVal = 1; //A70LK_Jacky_0002 start
			palDataMgr_Data_Access(edcSTANDBY_MODE, edaREAD, &ucVal);
			if(ucVal != eCM_STANDBY_MODE_COMMUNICATION) //A70Gen2_Julie_0035
			{
	            halBoard_CameraEnableSet(FALSE); //G100_Larry_0029
			}
        #if(defined(CUSTOM_OPTOMA))// HICC2_Bruce_0004
            else
            {
				UINT8 ucOn = FALSE;
				palDataMgr_Data_Access(edcUSB_POWER, edaREAD, &ucOn);
				if(ucOn == TRUE)
				{
					halBoard_CameraEnableSet(TRUE);
				}
				else
				{
					halBoard_CameraEnableSet(FALSE);
				}
            }
            //A70LK_Jacky_0002 end
            utilOptoma_OutputInfoMsg(sOptoma_System_Auto_Send_Lut[eOPT_STANDBY_MODE].cINFO_String);
        #endif
            //utilEventHandler_Send(eEVENT_LIST_GOTO_STAND_BY, 0, NULL); //A70LV_Larry_0009
            palSystem_PowerOff_Config(); //A70LV_Larry_0112
			palSystem_PowerOPDEvent(uiMsgID); //G100_Julie_0018

            MS_SLEEP(2000); //for MCU power off sequence to avoid i2c fail
            break;

        case eSYSTEM_EVENT_CFU_SELECT: //A70LV_Larry_0112
            {
                LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d) : fw upgrade select [%d]\r\n", __FUNCTION__, __LINE__, ulParam1);

                switch((eSYSTEM_UPGRADE_SELECT)ulParam1)
                {
                    case eSYSTEM_UPGRADE_MODE_SET:
                    case eSYSTEM_UPGRADE_SELECT_LPCMCU:
                    case eSYSTEM_UPGRADE_SELECT_SCALER:
                    case eSYSTEM_UPGRADE_SELECT_FPGA:
                    case eSYSTEM_UPGRADE_SELECT_All:
                    case eSYSTEM_UPGRADE_TEST_1:
                    case eSYSTEM_UPGRADE_TEST_2:
                        palSystem_UpgradeSelect((eSYSTEM_UPGRADE_SELECT)ulParam1);
                        break;

                    case eSYSTEM_UPGRADE_MENU_OSD:
                    case eSYSTEM_UPGRADE_MENU_DDP:
                    case eSYSTEM_UPGRADE_MENU_MCU:
                    case eSYSTEM_UPGRADE_MENU_FPGA:
                    case eSYSTEM_UPGRADE_MENU_ISO:
                        {
                            MS_SLEEP(3000);

                            switch((eSYSTEM_UPGRADE_SELECT)ulParam1)
                            {
                                case eSYSTEM_UPGRADE_MENU_OSD:
                                    palSystem_UpgradeSelect(eSYSTEM_UPGRADE_SELECT_SCALER);
                                    break;

                                case eSYSTEM_UPGRADE_MENU_FPGA:
                                    palSystem_UpgradeSelect(eSYSTEM_UPGRADE_SELECT_FPGA);
                                    break;

                                default:
                                    break;
                            }
                        }
                        break;

				#if (LOGO_REPLACE == 1) // R70G2_Bruce#0019 //A35G2_Coda_0067
					case eSYSTEM_UPGRADE_SECOND_LOGO_REPLACE:
					{// HICC2_Bruce_0014 // R70G2_Bruce#0023
						LOG_MSG(db_UPGRADE, "%s(eSYSTEM_UPGRADE_SECOND_LOGO_REPLACE)[start 4K]\n", __func__);
						//vTaskPrioritySet(m_sPalSysInfo.xTaskHandle, (APP_SYSMON_PRIORITY+2));
						m_sPalSysInfo.ucFW_upgrade = 1;
						eDATA_CODE eDataCode = edcUI_EVENT_LOGO_REPLACE;
						UINT32 ulValue = eCM_LOGO_REPLACE_PROCESS;
						palDataMgr_Access_UI_Event_LogoReplace(edaWRITE_RAM_ONLY_WITH_ACTION, &ulValue);
						palDataMgr_ImportDataToDatabase(eDataCode);
						utilIpc_SendData(eIPC_SEND_DATA_UPDATE_DATACODE_ITEM, 0, &eDataCode, eEXEC_CODE_PASS, sizeof(UINT16)*1);

						palSystem_SecondLogoPartialUpgrade(1);	 //ZU860_Clare_0125
						ulValue = eCM_USER_LOGO_PASS;
						palDataMgr_Data_Access(edcSPLASH_STARTUP, edaWRITE_THROUGH_WITH_ACTION, &ulValue);
						palLANProcSendToLAN(edcSPLASH_STARTUP);

						ulValue = eCM_LOGO_REPLACE_READY;
						palDataMgr_Data_Access(edcSECOND_LOGO_REPLACED, edaWRITE_THROUGH_WITH_ACTION, &ulValue);
						palDataMgr_Access_UI_Event_LogoReplace(edaWRITE_RAM_ONLY_WITH_ACTION, &ulValue);
						palDataMgr_ImportDataToDatabase(eDataCode);
						utilIpc_SendData(eIPC_SEND_DATA_UPDATE_DATACODE_ITEM, 0, &eDataCode, eEXEC_CODE_PASS, sizeof(UINT16)*1);
						palDataMgr_LogoReplace_DisplayCheck();// A65_OPTOMA_Julie_0079

						//palLANProcSendToLAN(edcSECOND_LOGO_REPLACED);
						//palLANProcSendToLAN(edcSPLASH_STARTUP);

						//vTaskPrioritySet(m_sPalSysInfo.xTaskHandle, (APP_SYSMON_PRIORITY));
						m_sPalSysInfo.ucFW_upgrade = 0;
						LOG_MSG(db_UPGRADE, "%s(eSYSTEM_UPGRADE_SECOND_LOGO_REPLACE)[done 4K]\n", __func__);
					}
						break;

					case eSYSTEM_UPGRADE_SERVICE_SECOND_LOGO_REPLACE: //A65_OPTOMA_Julie_0076
					{
						m_sPalSysInfo.ucFW_upgrade = 1;
						LOG_MSG(db_UPGRADE, "%s:%s(eSYSTEM_UPGRADE_SERVICE_SECOND_LOGO_REPLACE)[]\n", __FILE__, __func__);
						//appGui_Send_MenuOpen_Logo_Replace_Message(); //A65_OPTOMA_Julie_0082 //A65_OPTOMA_Julie_0079

						palSystem_ServiceCustomLogoPartialUpgrade(1);
						UINT8 ucValue = ets_ON;
						LOG_MSG(db_UPGRADE, "(%s,%d) (datacode:%d, value:%d)\r\n", __FUNCTION__, __LINE__, edcSERVICE_SECOND_LOGO_REPLACED, ucValue);
						palDataMgr_Data_Access(edcSERVICE_SECOND_LOGO_REPLACED, edaWRITE_THROUGH_WITH_ACTION, &ucValue);

						//ucValue = palDataMgr_LogoReplace_DisplayCheck(); //A65_OPTOMA_Julie_0079

						palLANProcSendToLAN(edcSERVICE_SECOND_LOGO_REPLACED);
						palLANProcSendToLAN(edcSPLASH_STARTUP);

						m_sPalSysInfo.ucFW_upgrade = 0;
					}
						break;
				#endif

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
                MS_SLEEP(200);
                if(rcSUCCESS == palSystem_Suspend(__FUNCTION__))
                {
                    palDataPath_LogoCapture();

                    palSystem_Resume(__FUNCTION__);
                }
                //appGui_Send_WaitProcessMessageExit(1);
                palDataMgr_UI_EventSend(edcUI_EVENT_WAIT_PROCESS_MESSAGE_EXIT, 1, NULL);
                break;

            case eSYSTEM_EVENT_LOGO_REPLACEMENT:
                MS_SLEEP(200);
                if(rcSUCCESS == palSystem_Suspend(__FUNCTION__))
                {
                    #if 0   //H2 wait review
                    appGui_LogoReplacement();
                    #endif

                    palSystem_Resume(__FUNCTION__);
                }
                //appGui_Send_WaitProcessMessageExit(1);
                palDataMgr_UI_EventSend(edcUI_EVENT_WAIT_PROCESS_MESSAGE_EXIT, 1, NULL);
                break;

            case eSYSTEM_EVENT_ENTER_2W_MODE:
                LOG_MSG(db_ALWAYS, "(func:%s, line:%d) Enter 2W mode\r\n", __FUNCTION__, __LINE__);
                SYSTEM_CALL("echo mem > /sys/power/state");
                LOG_MSG(db_ALWAYS, "(func:%s, line:%d) Exit 2W mode\r\n", __FUNCTION__, __LINE__);
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

                    //appGui_Send_WaitProcessMessageExit(2);
                }
                break;

            case eSYSTEM_EVENT_ACTUATOR_PATTERN:
                {
                    char TP_FileName[128] = {0};

                    //palDataMgr_ResetCountSet(__FUNCTION__, 30);

                    MS_SLEEP(500);  //wait OSD event done

                    if(palSystem_Suspend(__FUNCTION__) == rcSUCCESS)
                    {
                        UINT8 ucPattern = (UINT8)ulParam1;

                        if(ucPattern == 0)
                        {
                            snprintf(TP_FileName, sizeof(TP_FileName), "%s/%s", CONF_SCALER_TP_PATH, TP_BUILTIN_ACTUATOR_TP_FILE_NAME_ID0);
                            palGeo_Draw_PNG_Pattern(0, eDRAW_565HIGH_COLOR, TP_FileName);
                        }
                        else if(ucPattern == 1)
                        {
                            snprintf(TP_FileName, sizeof(TP_FileName), "%s/%s", CONF_SCALER_TP_PATH, TP_BUILTIN_ACTUATOR_TP_FILE_NAME_ID1);
                            palGeo_Draw_PNG_Pattern(1, eDRAW_565HIGH_COLOR, TP_FileName);
                        }

                        palSystem_Resume(__FUNCTION__);
                    }

                    //palDataMgr_ResetCountSet(__FUNCTION__, 0);
                }
				break;

            case eSYSTEM_EVENT_WARP_PATTERN:
                {
                    ePAL_GEO_FUNCTION eFunction = (ePAL_GEO_FUNCTION)ulParam1;
                    uPALGEOAPI_INFO uData = *(uPALGEOAPI_INFO *)pulParam2;

                    LOG_MSG(db_HAL_WARPING, "eSYSTEM_EVENT_WARP_PATTERN Event: %d\r\n", (ePAL_GEO_FUNCTION)ulParam1);

                    palGeo_DrawOSDQueueSet(eFunction, uData);

                    if(eFunction == eGFN_DRAW_OSD)
                    {
            			palLANProcSendToLAN(edcGEO_OSD_ON);
                    }
                    else if(eFunction == eGFN_CLEAR_OSD)
                    {
            			palLANProcSendToLAN(edcGEO_CLEAR_WARP_OSD);
                    }
                }
                break;


            case eSYSTEM_EVENT_APPLY_AP_MODE_GEO_FUNC:   //A35G2_Simon_0100
                palSystem_Apply_AP_Mode_GeoFunc();
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

    m_sDefSysConfiguration.Illum.uiCooldownDelay = Syscfg_Value_Get_Typeint(eCooling_Time);//Secon //HICC2_Steven_0037

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
    LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d):\r\n", __FUNCTION__, __LINE__);

    if(palDataPath_InitSub(m_sPalSysInfo.ePanelTimingId, &m_sDefSysConfiguration) == eEXEC_CODE_FAIL)   //A70LV_Doulas_0003
    {
        LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d): Datapath sub Init Fail!\r\n", __FUNCTION__, __LINE__);
        palSystem_Fault();
    }
    LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d):\r\n", __FUNCTION__, __LINE__);

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

    LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d):\r\n", __FUNCTION__, __LINE__);

    //init IPC
    if(utilIpc_Init() == eEXEC_CODE_FAIL)
    {
        LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d): IPC Init Fail!\r\n", __FUNCTION__, __LINE__);
        palSystem_Fault();
    }
    LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d):\r\n", __FUNCTION__, __LINE__);

    if(palInputProc_Init() == eEXEC_CODE_FAIL)
    {
        LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d): Input Init Fail!\r\n", __FUNCTION__, __LINE__);
        palSystem_Fault();
    }
    LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d):\r\n", __FUNCTION__, __LINE__);

    if(palIllumination_Init() == eEXEC_CODE_FAIL)
    {
        LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d): Illumination Init Fail!\r\n", __FUNCTION__, __LINE__);
        palSystem_Fault();
    }
    LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d):\r\n", __FUNCTION__, __LINE__);

    if(palLANProc_Init() == eEXEC_CODE_FAIL)
    {
        LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d): LAN Init Fail!\r\n", __FUNCTION__, __LINE__);
        palSystem_Fault();
    }
    LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d):\r\n", __FUNCTION__, __LINE__);

    if(appPoll_Init() == eEXEC_CODE_FAIL)
    {
        LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d): Poll Init Fail!\r\n", __FUNCTION__, __LINE__);
        palSystem_Fault();
    }
    LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d):\r\n", __FUNCTION__, __LINE__);

    if(appCLICmd_Init() == eEXEC_CODE_FAIL)
    {
        LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d): CLI Cmd Init Fail!\r\n", __FUNCTION__, __LINE__);
        palSystem_Fault();
    }
    LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d):\r\n", __FUNCTION__, __LINE__);

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
                            case eSYSTEM_STATE_COMMUNICATION: //A70Gen2_Julie_0035 //A70LK_Jacky_0002
                            {
                            	if(CFG_CUSTOMER_ID == CUSTOMER_ID_OPTOMA)
								{
									UINT8 ucOn = FALSE;
    								palDataMgr_Data_Access(edcUSB_POWER, edaREAD, &ucOn);
									if(ucOn == TRUE)
									{
										halBoard_CameraEnableSet(TRUE);
									}
									else
									{
										halBoard_CameraEnableSet(FALSE);
									}
								}
								else
								{
									halBoard_CameraEnableSet(TRUE);
								}
                        	}
							break;
                            //case eSYSTEM_STATE_WARMUP:
                            case eSYSTEM_STATE_POWERON:
                            case eSYSTEM_STATE_UPGRADE:
                                {
                                    UINT8 ucResetMode = 0;
                                    UINT8 ucHostReady = 0;

                                    if(rcSUCCESS == halMCU_Factory_Reset_Get(&ucResetMode))
                                    {
                                        if(ucResetMode == FACTORY_RESET_NUMBER)
                                        {
                                            palDataMgr_ResetAllToDefault();
					                        utilOPD_ResetAllToDefault();
                                        }
                                    }

                                    palSystem_WarmUp();
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

    palLANProc_Reset();
    palLANProcPowerStateSend(ePOWER_STATE_RESET, 4, (UINT8*)&ulData);

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
    }
}


void palSystem_SetMcuState(UINT8 ucState)
{
	m_sPalSysInfo.ucSystemMcuState = ucState;
    LOG_MSG(db_APP_SYSTEM, "palSystem_SetMcuState %d\r\n", ucState);
}

UINT8 palSystem_GetMcuState(void)
{
	return m_sPalSysInfo.ucSystemMcuState;
}


#ifdef SCALER_FPGA_F34
void palSystem_ChangePanelID(ePANEL_ID ePanelId, sSYSTEM_CHANGEPANELID_INFO *psInfo)   //A70LV_Doulas_0005
{
    //UINT8 ucPIP_EN = ets_OFF;
	UINT8 ucValue = 0;
    UINT8 uc3DMode = palImgMgr_3DEnable_Get();

    //3D mode   /Scaler /Warping
    //Frame seq 2K120   2K120
    //4K3D      4K60    4K60
    //4K3D dual 4K60    2K240

    LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d) ePanelId = %d\r\n", __FUNCTION__, __LINE__, ePanelId);

    if(psInfo == NULL)
    {
        LOG_MSG(db_APP_SYSTEM, "Error: (func:%s, line:%d) psInfo == NULL\r\n", __FUNCTION__, __LINE__);
        return;
    }

    palEnvironment_LightSourceBlankingOn_Set(TRUE,5);   //A70LV_Doulas_0294

    //Try to use one chip concept to save parameters
    //m_sPalSysInfo.ePanelTimingId = ePanelId;
	m_bActuatorEnable = psInfo->bActuatorEnable;  //A70LK_Doulas_0018

    //palDataPath_Panel_ChangeCheck(psInfo->bBlicking);

    palSystem_PanelID_Set(ePanelId);  //H2PF_Simon_0038

    if((uc3DMode == eCM_3D_FORMAT_4K3D_DUALPIPE) && (ePanelId == PANEL_2D_OUTPUT) && (psInfo->bActuatorEnable == FALSE))
    {
        //4K3D dualpipe
        palDataMgr_UI_EventSend(edcUI_EVENT_PANEL_CHANGE, ePANEL_ID_1920x2400_60HZ, NULL); //appGui_PanelChange(PANEL_2D_HIGHSPEED);
    }
    else
    {
        palDataMgr_UI_EventSend(edcUI_EVENT_PANEL_CHANGE, ePanelId, NULL); //appGui_PanelChange(ePanelId);
    }
    halAdvWarpVstartOffsetSet(ePanelId); //###
    //palGeo_AdvWarpingInit(m_sPalSysInfo.ePanelTimingId); //###


    if(halScaler_Panel_Set(0, m_sPalSysInfo.ePanelTimingId) == eHAL_SCALER_EXEC_CODE_PASS)
    {
        halScaler_Default4K3DSEnable_Set(uc3DMode);

        if((uc3DMode == eCM_3D_FORMAT_4K3D_DUALPIPE) &&
           (m_sPalSysInfo.ePanelTimingId == PANEL_2D_OUTPUT) &&
           (psInfo->bActuatorEnable == FALSE)) //A70LK_Larry_0114
        {
            palGeo_ConfigPanel(ePANEL_ID_1920x2400_60HZ, TRUE);
        }
        else
        {
            palGeo_ConfigPanel(m_sPalSysInfo.ePanelTimingId, FALSE);
        }

        MS_SLEEP(200);

        if(ePanelId == PANEL_3D_OUTPUT || ePanelId == PANEL_2D_HIGHSPEED)
        {
#ifndef VPD_XPR_ENABLE
            if(ePanelId == PANEL_3D_OUTPUT)
            {
                palFormatterMgr_XPR_Set(eXPR_OFF_4WAY_120);
            }
            else
            {
                palFormatterMgr_XPR_Set(eXPR_OFF_4WAY_240);
            }
#else
            if(ePanelId == PANEL_3D_OUTPUT)
            {
                halFormatter_XPR_Set(eXPR_OFF_4WAY_120);
            }
            else
            {
                halFormatter_XPR_Set(eXPR_OFF_4WAY_240);
            }
#endif /* VPD_XPR_ENABLE */
        }
        else
        {
            if(psInfo->bActuatorEnable) //2D
            {
#ifdef VPD_XPR_ENABLE
                halScaler_PixelShiftEnable_Set(TRUE);
#endif /* VPD_XPR_ENABLE */
                palFormatterMgr_XPR_Set(eXPR_ON);
            }
            else //4K3D
            {
#ifndef VPD_XPR_ENABLE
                palFormatterMgr_XPR_Set(eXPR_OFF_4WAY_240);
#else
                halFormatter_XPR_Set(eXPR_OFF_4WAY_240);
#endif /* VPD_XPR_ENABLE */
            }
        }

        if(palDataPath_Panel_Change(m_sPalSysInfo.ePanelTimingId) == eEXEC_CODE_FAIL)
        {
            LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d): palDataPath_Panel_Change Fail!\r\n", __FUNCTION__, __LINE__);
           // palSystem_Fault();
        }

        if(palDataPath_Panel_ChangeSub(m_sPalSysInfo.ePanelTimingId) == eEXEC_CODE_FAIL)
        {
            LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d): palDataPath_Panel_ChangeSub  Fail!\r\n", __FUNCTION__, __LINE__);
           // palSystem_Fault();
        }
    }
}

#else

void palSystem_ChangePanelID(ePANEL_ID ePanelId, sSYSTEM_CHANGEPANELID_INFO *psInfo)   //A70LV_Doulas_0005
{
    UINT8 ucPIP_EN = ets_OFF;
	UINT8  ucData;		//G100_Doulas_0027
    LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d) Panel %d\r\n", __FUNCTION__, __LINE__, ePanelId);

#ifdef SCALER_C821_C789
    if(palGeo_ApLinkFlag_Get() == ets_ON)                                       //ZU860_Doulas_0138
    {
        return;
    }
#endif
    //halC789Ctrl_OutputEnableSet(0);

    //Try to use one chip concept to save parameters
    palSystem_PanelID_Set(ePanelId);
    palEnvironment_LightSourceBlankingOn_Set(TRUE,10);   //H30K_Doulas_0025//A70LV_Doulas_0294
    palDataMgr_Data_Access(edcPIP_PBP_ENABLE, edaREAD, &ucPIP_EN);      //A70LV_Doulas_0156
    if((ucPIP_EN) &&                                                                    //G100_Owen_0012
       (ePanelId == ePANEL_ID_1080P_120HZ || ePanelId == ePANEL_ID_WUXGA_120HZ))         //A70LV_Doulas_0377 Modify//A70LV_Doulas_0156
    {
        ucPIP_EN = ets_OFF;
        palDataMgr_Data_Access(edcPIP_PBP_ENABLE, edaWRITE_THROUGH_WITH_ACTION, &ucPIP_EN);
    }
#ifdef Low_Latency_All
	if((ePanelId == ePANEL_ID_1080P_120HZ || ePanelId == ePANEL_ID_WUXGA_120HZ || FrontEnd_Down_Scaling_Get()) && halScaler_Is_LowLatencyMode_On())	//G100_Clare_0053    //H2PF_Simon_0187
    {
    	UINT8 ucVlaue = FALSE;
		halFormatter_FRCByPassModeSet(FALSE);    //A35G2_CDS_Simon_0004
		palDataMgr_Data_Access(edcLOW_LATENCY_MODE, edaWRITE_THROUGH_NO_ACTION, &ucVlaue);
		palDataPath_Low_Latency_Set(FALSE);
#ifdef SCALER_C821_C789
		halC789Ctrl_Change_Panel(ePanelId);
#endif
	}
	#endif	/*Low_Latency_All*/

    if(palEnvironment_LightSourceBlankingEn_Get()) //A70LV_Doulas_0300
    {
        palImgMgr_WB_OutputEnableSet(0);     //A70LV_Doulas_0216
    }

    #ifndef SCALER_C341
    MS_SLEEP(200);          //A70LV_Doulas_0154 Add
    if(palDataPath_Panel_ChangeSub(m_sPalSysInfo.ePanelTimingId) == eEXEC_CODE_FAIL)
    {
        LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d): palDataPath_Panel_ChangeSub  Fail!\r\n", __FUNCTION__, __LINE__);
       // palSystem_Fault();
    }
    #endif
    palLANProcSendToLAN(edcCOLOR_WHEEL_SPEED);		//A65_OPTOMA_Doulas_0131//A35G2_Coda_0110
    //GuiCb.fpGui_PanelChangeCb(ePanelId); //A70LV_Larry_0173
    palLANProcSendToLAN(edc3D_CONFIG_TYPE);
    palDataMgr_UI_EventSend(edcUI_EVENT_PANEL_CHANGE, ePanelId, NULL);

    if(ePanelId == PANEL_3D_OUTPUT || ePanelId == PANEL_2D_HIGHSPEED) //H30K_Doulas_0001
    {
        if(ePanelId == PANEL_3D_OUTPUT)
        {
            palFormatterMgr_XPR_Set(eXPR_OFF_4WAY_120);
        }
        else
        {
            palFormatterMgr_XPR_Set(eXPR_OFF_4WAY_240);
        }
    }
    else
    {
        palFormatterMgr_XPR_Set(eXPR_ON);
    }

    if(palDataPath_Panel_Change(m_sPalSysInfo.ePanelTimingId) == eEXEC_CODE_FAIL)
    {
        LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d): palDataPath_Panel_Change Fail!\r\n", __FUNCTION__, __LINE__);
       // palSystem_Fault();
    }
#if 1   //H30K_Doulas_0001 to do(warping not ready)
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
    	halWarping_Blacklevel_Reset();
    	halWarping_Blacklevel_LoadData(BLACKLEVEL_CURRENT_INDEX);  //A35G2_Simon_0093
    	#endif
	}
	else
	{
	    palDataMgr_AccessADV_WarpControlSet(ucData);
		//palDataMgr_Data_Access(edcADV_WARP_CONTROL, edaWRITE_RAM_ONLY_WITH_ACTION, &ucData);	//G100_Doulas_0027 Add
	}
    if(palEnvironment_LightSourceBlankingEn_Get()) //A70LV_Doulas_0300
    {
        palImgMgr_WB_OutputEnableSet(1);     //A70LV_Doulas_0216
    }
#endif
}
#endif

// A70LV_Eric.C_0021 Start //A70LV_Larry_0097 modify
BOOL appSysInitAfterAsicReady(void)
{
    if(Board_SingleBoard_Get() == FALSE)
    {
        if(m_bIsAsicInitialed == FALSE)
        {
            if(!halDDP4422_IsAsicReady())
            {
                if(m_bAsicReadyCheckCount > 0)  //G100_Simon_0064
                {
                    m_bAsicReadyCheckCount--;

                    if(0 == m_bAsicReadyCheckCount)
                    {
                        UINT32 ulErrorIndex = 0;
                        ulErrorIndex = DDPFail;

    					utilDataMgr_WriteGecLog_CM(ulErrorIndex);	//HICC2_Doulas_0003
                        LOG_MSG(db_HAL_FORMATTER, "ASIC Not Ready\r\n");

                        if(palSystem_OPFUCheck() == TRUE)   //G100_Simon_0064
                        {
                            LOG_MSG(db_UPGRADE, "\n\n---> ASIC Not Ready go to upgrade mode\n\n");
                            LOG_MSG(db_UPGRADE, "\r\n(func:%s, line:%d) Entering OPFU Mode\r\n", __FUNCTION__, __LINE__);
                            palSystem_FwUpgradeMode(eSYSTEM_UPGRADE_MODE_SET);
                        }

                        palDataMgr_OPDFWversion(eOPD_FW_VERSION_LOG);
                        palLedProc_LED_Behavior_Set(eLED_STATUS_CWFAIL_COOLING);
                        palSystem_PowerDown();
                    }

                    if(m_bAsicReadyCheckCount == 16 || m_bAsicReadyCheckCount == 12)
                    {
                        halMCU_DDP_PowerSet(1);
                        LOG_MSG(db_HAL_FORMATTER, "ASIC Not Ready and Reset retry\r\n");
                        MS_SLEEP(3000);
                    }
                }

                LOG_MSG(db_HAL_FORMATTER, "Waiting for ASIC Ready (%d)\r\n", m_bAsicReadyCheckCount);

                return FALSE; //A70LV_Larry_0097
            }
            else
            {
                //A70LV_Doulas_0077 read DDP version
                UINT16 wVerDDPApp = 0;
                UINT16 wVerDDPApp_compare = 0; //A70LV_John_0019 add ddp version checking mechanism
                UINT32 dwVerDDPSq = 0;
                UINT32 wVerDDPSq_compare = 0; //A70LV_Larry_0379
                UINT8  aucVerString[32] = {0};
                UINT8  ucRetry = 2; //A70LV_John_0019 add ddp version checking mechanism
                UINT8  ucData = FALSE;
                UINT8  uciScalerOpfuCheck = 0;

                LOG_MSG(db_UPGRADE, "\n\n===> ASIC Ready , clear OPFU check\n\n");
				#ifdef CUSTOM_OPTOMA
				utilOptoma_OutputInfoMsg(sOptoma_System_Auto_Send_Lut[eOPT_WARMING_UP].cINFO_String);
				#endif
                //clear OPFU Check
                palDataMgr_Data_Access(edcOPFU_CHECK, edaWRITE_THROUGH_WITH_ACTION, &uciScalerOpfuCheck);   //G100_Simon_0064

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

                LOG_MSG(db_APP_SYSTEM, "DDP      F%02d.%02d(%c%c%c%02d)\n",
                    (UINT8)(wVerDDPApp>>8),      //DDP442x       Fxx.xx(DHDxx)
                    (UINT8)(wVerDDPApp),
                    (UINT8)(dwVerDDPSq>>24),
                    (UINT8)(dwVerDDPSq>>16),
                    (UINT8)(dwVerDDPSq>>8),
                    (UINT8)(dwVerDDPSq));

                sprintf((char*)aucVerString, "F%02d.%02d(%c%c%c%02d)" ,
                    (UINT8)(wVerDDPApp>>8),      //DDP442x       Fxx.xx(DHDxx)
                    (UINT8)(wVerDDPApp),
                    (UINT8)(dwVerDDPSq>>24),
                    (UINT8)(dwVerDDPSq>>16),
                    (UINT8)(dwVerDDPSq>>8),
                    (UINT8)(dwVerDDPSq));

                palDataMgr_Data_Access(edcFORMATER_VERSION, edaWRITE_RAM_ONLY_NO_ACTION, aucVerString);
            #if(defined(CUSTOM_OPTOMA))// HICC2_Bruce_0004
                palDataMgr_Data_ReadCurrentWrite(edcUSB_POWER);
            #else
                halBoard_CameraEnableSet(TRUE); //G100_Larry_0029
            #endif
                LOG_MSG(db_HAL_FORMATTER, "ASIC Ready\r\n");        //A70LV_Doulas_0046 Add
                m_bIsAsicInitialed = TRUE;

                palDataMgr_OPDFWversion(eOPD_FW_VERSION_LOG); //G100_Julie_0044
                halFormatter_XPR_ErrorStatus_Set();
            }
        }

        return m_bIsAsicInitialed; //A70LV_Larry_0097
    }
    else
    {
        if(m_bIsAsicInitialed == FALSE)
        {
            UINT8  uciScalerOpfuCheck = FALSE;

            //clear OPFU Check
            palDataMgr_Data_Access(edcOPFU_CHECK, edaWRITE_THROUGH_WITH_ACTION, &uciScalerOpfuCheck);   //G100_Simon_0064

            m_bIsAsicInitialed = TRUE;
        }
        return TRUE;
    }
}
// A70LV_Eric.C_0021 End

BOOL appSysIsAsicReadyGet(void)
{
    return m_bIsAsicInitialed;
}

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
eRESULT palSystem_VersionCheck(BOOL bAutoUpgrade)
{
	UINT32 ulVersion = 0;
    UINT8 aucVersion[8] = {0};
    UINT8 aucVerString[32] = {0};
    eRESULT eResult = rcSUCCESS;
    eRESULT eResultAll = rcSUCCESS;
    UINT8 Count = 0;
    UINT8 ucXPRVer[8] = {0}; //HICC2_Steven_0039

    //Version check

	//Cassper_ProAV
    //pulExtVer = (UINT32 *)(FW_INFO_ADDR + FW_NAME_STRING_MAXIMUM + sMEM_TAG_PARAM_SIZE*eFW_FPGA_1);

	///LOG_MSG(db_APP_SYSTEM, "Global V%d.%d.%d\n", VERSTAGE, VERMAJOR, VERMINOR);

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

    //System
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
    ///////////////////////////////////////////////////////////////////////////


	LOG_MSG(db_APP_SYSTEM, "IScaler V%02d.%02d\n", VER_MAJOR, VER_MINOR);

    //Motor Board
    memset(aucVersion, 0, sizeof(aucVersion));
    eResult = halMotor_Version_Get(aucVersion);

    if(eResult == rcSUCCESS)
    {
        UINT8 ucModule = 0;

        palMotor_LensModule_Get(&ucModule);

#if 0 //A70Gen2_Doulas_0002 Modify//不做判斷 //A35G2_Larry_0064
        if((eLENS_MODULE)ucModule != eLENS_MODULE_H60)
        {
            ucModule = (UINT8)eLENS_MODULE_H60;
            palMotor_LensModule_Set(&ucModule);
        }
#endif /* 0 */

#ifdef OE_JIG
        //halMotor_OEJig_Set();  //HICC2_Doulas_0096 remove
#endif /* OE_JIG */

        m_cLensModel = ucModule;

        //palEnvironment_LensModelCheck_Set(FALSE); //G100_Clare_0059, add

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

#if defined(PLATFORM_R70K) || defined(PLATFORM_H30_4K) //HICC2_AC_0002
    palMotor_LSM_Version_Get();
    palMotor_LSM_Factory_Menu();
    palMotor_LSM_CalibrationDone_Ask();
    palMotor_LSM_Lens_ID_Get();
    palMotor_LSM_Memory_Query(); //HICC2_AC_0019
    palMotor_LSM_Plug_Query();	//HICC2_Zonic_0009
    palMotor_LSM_Lock_Query(); //H30K_Julie_0005

    #ifdef CURSOR_FIXTURE
    {
   		UINT8 ucVal;
		palDataMgr_Data_Access(edcLENS_DURATIONTIME_ENABLE, edaREAD, &ucVal);
		if(ucVal == ets_ON)
		{
			halMotor_LSM_Zoom_Duration_Set();
			halMotor_LSM_Focus_Duration_Set();
			halMotor_LSM_BackFocus_Duration_Set();
		}
	}
	#endif
#endif

    //HDBaseT
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
///////////////////////////////////////////////////////////////////////////

    //FMT LD
    memset(aucVersion, 0, sizeof(aucVersion));
    eResult = halLDCtrl_Version_Get(aucVersion);

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

	palDataMgr_Model_ID_Set(); //A70Gen2_Julie_0004
///////////////////////////////////////////////////////////////////////////

    //FMT LD Convert
    #if 0 //HICC2_Doulas_0098 remove
    memset(aucVersion, 0, sizeof(aucVersion));
    eResult = halLDCtrl_ConvertVersion_Get(aucVersion);

    if(eResult == rcSUCCESS)
    {
        LOG_MSG(db_APP_SYSTEM, "FMT convert D%02d.%02d\n", aucVersion[1], aucVersion[0]);

        sprintf((char*)aucVerString, "D%02d.%02d", aucVersion[1], aucVersion[0]);
        palDataMgr_Data_Access(edcM481_VERSION, edaWRITE_RAM_ONLY_NO_ACTION, aucVerString);

        if(strcmp(aucVerString, "D00.00") == 0 ||   //G100_Simon_0077
           strcmp(aucVerString, "DFF.FF") == 0 )
        {
            LOG_MSG(db_APP_SYSTEM, "FMT convert version error %s\n", aucVerString);
            eResult = rcERROR;
        }
    }
    eResultAll &= eResult;
    #endif
///////////////////////////////////////////////////////////////////////////
    //Actuator FPGA

    memset(aucVersion, 0, sizeof(aucVersion));
    eResult = halMCUCtrl_FPGA2_Version_Get(aucVersion);

    if(eResult == rcSUCCESS)
    {
        LOG_MSG(db_APP_SYSTEM, "FPGA1   Y%02d.%02d\n", aucVersion[1], aucVersion[0]);

        sprintf((char*)aucVerString, "Y%02d.%02d", aucVersion[1], aucVersion[0]);
        palDataMgr_Data_Access(edcFPGA1_VERSION, edaWRITE_RAM_ONLY_NO_ACTION, aucVerString);

        if(strcmp(aucVerString, "Y00.00") == 0 ||
           strcmp(aucVerString, "YFF.FF") == 0 )
        {
            LOG_MSG(db_APP_SYSTEM, "FPGA2 version error %s\n", aucVerString);
            eResult = rcERROR;
        }
    }
    eResultAll &= eResult;

///////////////////////////////////////////////////////////////////////////
    //FMT FPGA

    memset(aucVersion, 0, sizeof(aucVersion));
    eResult = halMCUCtrl_FPGA3_Version_Get(aucVersion);

    if(eResult == rcSUCCESS)
    {
        LOG_MSG(db_APP_SYSTEM, "FPGA3   Z%02d.%02d\n", aucVersion[1], aucVersion[0]); //HICC2_Doulas_0094

        sprintf((char*)aucVerString, "Z%02d.%02d", aucVersion[1], aucVersion[0]);
        palDataMgr_Data_Access(edcFPGA3_VERSION, edaWRITE_RAM_ONLY_NO_ACTION, aucVerString); //HICC2_Doulas_0093

        if(strcmp(aucVerString, "Z00.00") == 0 ||   //G100_Simon_0077
           strcmp(aucVerString, "ZFF.FF") == 0 )
        {
            LOG_MSG(db_APP_SYSTEM, "FPGA3 version error %s\n", aucVerString);
            eResult = rcERROR;
        }
    }
    eResultAll &= eResult;
///////////////////////////////////////////////////////////////////////////

    //HW version
    memset(aucVersion, 0, sizeof(aucVersion));
    eResult = halMCUCtrl_HWVersion_Get(aucVersion);

    if(eResult == rcSUCCESS)
    {
        ulVersion = (UINT32)(aucVersion[1] << 8) | (UINT32)aucVersion[0];
        LOG_MSG(db_APP_SYSTEM, "HW version Main %02d, Formatter %02d, %08x\n", aucVersion[0], aucVersion[1], ulVersion);

        palDataMgr_Data_Access(edcHW_PROJECTOR_ID, edaWRITE_RAM_ONLY_NO_ACTION, (void*)&ulVersion);
    }

    eResultAll &= eResult;
///////////////////////////////////////////////////////////////////////////

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
//////////////////////////////////////////////////////////////////////////////////

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
            UINT8  ucRetry = 2; //A70LV_John_0019 add ddp version checking mechanism

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

            LOG_MSG(db_APP_SYSTEM, "DDP      F%02d.%02d(%c%c%c%02d)\n",
                (UINT8)(wVerDDPApp>>8),      //DDP442x       Fxx.xx(DHDxx)
                (UINT8)(wVerDDPApp),
                (UINT8)(dwVerDDPSq>>24),
                (UINT8)(dwVerDDPSq>>16),
                (UINT8)(dwVerDDPSq>>8),
                (UINT8)(dwVerDDPSq));

            sprintf((char*)aucVerString, "F%02d.%02d(%c%c%c%02d)" ,
                (UINT8)(wVerDDPApp>>8),      //DDP442x       Fxx.xx(DHDxx)
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

    ///////////////////////////////////////////////////////////////////////////
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

	palLANProcSendToLAN(edcFOTA_MODEL_NAME);  //A35G2_Coda_0118

	// XPR Version //HICC2_Steven_0039
	memset(aucVerString, 0, sizeof(aucVerString));
    halFormatter_XPR_Version_Get(ucXPRVer);
    sprintf((char*)aucVerString, "%d.%d.%d.%d", ucXPRVer[0], ucXPRVer[1], ucXPRVer[2], ucXPRVer[3]);
    palDataMgr_Data_Access(edcXPR_VERSION, edaWRITE_RAM_ONLY_NO_ACTION, aucVerString);

    //memset(aucVerString, 0, sizeof(aucVerString));
    //palDataMgr_Data_Access(edcXPR_VERSION, edaREAD, aucVerString);
    //LOG_MSG(db_ALWAYS, ">> XPR Version Get %s\r\n", aucVerString);
    return eResultAll;
}

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
UINT8 palSystem_Scaler_Upgrade(UINT8 ucFocus)
{
    UINT8 cResult = 1;

    if(palSystem_Suspend(__FUNCTION__) == rcSUCCESS)
    {
        //SYSTEM_CALL("killall iScalerH2CT");

        cResult &= (UINT8)palGui_Upgrade(ucFocus);

        palSystem_Resume(__FUNCTION__);
    }

    cResult &= halGui_OSDReload();

    LOG_MSG(db_UPGRADE, "Scaler Upgrade Done\n");

    return cResult;
}

#if (LOGO_REPLACE == 1)	//A35G2_Coda_0067
//--------------LOGO--------------
eRESULT palSystem_Suspend_LogoReplace(const char *cFuncName)// HICC2_Bruce_0014
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

        //appPoll_Task_Suspend();
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
eRESULT palSystem_Resume_LogoReplace(const char *cFuncName)// HICC2_Bruce_0014
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
        //appPoll_Task_Resume();
        palDataPath_Task_Resume();
        palDataPath_TaskSub_Resume();
        //GuiCb.fpGui_Task_ResumeCb();

        m_sPalSysInfo.bTaskPause = FALSE;

        pthread_mutex_unlock(&m_sPalSysInfo.xTaskMutex);

        return rcSUCCESS;
    }

    return rcERROR;
}

void palSystem_SecondLogoPartialUpgrade(UINT8 ucFocus) //T100_Coda_0001 //A65_OPTOMA_Julie_0067 //A65_OPTOMA_Julie_0102
{
    if(palSystem_Suspend_LogoReplace(__FUNCTION__) == rcSUCCESS)// HICC2_Bruce_0014
    {
		halGui_Upgrade_Second_Logo_Replace(ucFocus);
		halGui_2ND_LOGO_OSDReload(eCM_USER_LOGO_2D);

        palSystem_Resume_LogoReplace(__FUNCTION__);
        LOG_MSG(db_UPGRADE, "%s(4K) pass.\r\n", __FUNCTION__);
    }
    else
	{
        LOG_MSG(db_UPGRADE, "%s(4K) fail!\r\n", __FUNCTION__);
	}
}

void palSystem_ServiceCustomLogoPartialUpgrade(UINT8 ucFocus) //A65_OPTOMA_Julie_0076 //A65_OPTOMA_Julie_0102//A35G2_Coda_0067
{
    if(palSystem_Suspend_LogoReplace(__FUNCTION__) == rcSUCCESS)// HICC2_Bruce_0014
    {
		halGui_Upgrade_Service_Second_Logo_Replace(ucFocus);
		//halGui_2ND_LOGO_OSDReload(eCM_USER_LOGO_SERVICE_2D);
        palSystem_Resume_LogoReplace(__FUNCTION__);
        LOG_MSG(db_UPGRADE, "%s(4K) pass.\r\n", __FUNCTION__);
    }
    else
	{
        LOG_MSG(db_UPGRADE, "%s(4K) fail!\r\n", __FUNCTION__);
	}
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
    //eMODEL_TYPE_4K13HS,    //MODEL_ID_0
    //eMODEL_TYPE_4K16HS,    //MODEL_ID_1
    //eMODEL_TYPE_4K22HS,    //MODEL_ID_2

    return Board_ModelID_Get(); //m_sPalSysInfo.ucModelID;
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
// FUNCTION NAME: palSystem_ActuatorEnable_Get
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
// 2023/02/10, Doulas Create
// --------------------
// ==============================================================================
BOOL palSystem_ActuatorEnable_Get(void)   //A70LK_Doulas_0018
{
    return m_bActuatorEnable;
}

// ==============================================================================
// FUNCTION NAME: palSystem_ActuatorEnable_Set
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
// 2023/02/15, Larry Create
// --------------------
// ==============================================================================
void palSystem_ActuatorEnable_Set(BOOL bEnable)
{
    m_bActuatorEnable = bEnable;
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
    UINT8 aucVersion[4] = {0};
    UINT8 aucExtVersion[2] = {0};
    UINT8* pcBuffer = NULL;
    UINT32 ulSize = 0;

    switch(cIndex)
    {
        case eLPCMCU_INDEX_FRONT:
            {
                pFile = fopen(FW_MCU_FRONTEND, "rb");

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

                eResut = halFrontEndCtrl_Version_Get(aucVersion);
            }
            break;

        case eLPCMCU_INDEX_MOTORBD:
            {
                pFile = fopen(FW_MCU_MOTOR, "rb");

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
                pFile = fopen(FW_MCU_EXT, "rb");

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

                eResut = halMCUCtrl_Version_Get(aucVersion);
            }
            break;

        default:
            return rcERROR;
    }

#if 1
    if(eResut == rcSUCCESS)
    {
        LOG_MSG(db_UPGRADE, "MCU Version %02d.%02d, ExtVersion %02d.%02d\r\n", aucVersion[0], aucVersion[1], aucExtVersion[0], aucExtVersion[1]);

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

                switch(cIndex)
                {
                    case eLPCMCU_INDEX_FRONT:
                        eResut = halFrontEndCtrl_Version_Get(aucVersion);
                        LOG_MSG(db_UPGRADE, "MCU frontend Ver E%02d.%02d\r\n", aucVersion[1], aucVersion[0]);
                        break;

                    case eLPCMCU_INDEX_MOTORBD:
                        eResut = halMotor_Version_Get(aucVersion);
                        LOG_MSG(db_UPGRADE, "MCU Motor Ver M%02d.%02d\r\n", aucVersion[1], aucVersion[0]);
                        break;

                    case eLPCMCU_INDEX_SYSTEM:
                        eResut = halMCUCtrl_Version_Get(aucVersion);
                        LOG_MSG(db_UPGRADE, "MCU system Ver A%02d.%02d\r\n", aucVersion[1], aucVersion[0]);
                        break;

                    default:
                        break;
                }

            }
        }
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
#if 1	//HICC2_Zonic_0013
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
#endif /* 0 */
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

    do
    {
        DDP_SYSTEM_STATUS sDDPSystemStatus;
        eRESULT eRet = halFormatter_SYS_SystemStatusGet((UINT8 *)&sDDPSystemStatus);

        if(eRet == rcSUCCESS &&
           sDDPSystemStatus.DDP_ST_CW_PHASELOCK  == TRUE &&
           sDDPSystemStatus.DDP_ST_CW_FREQLOCK   == TRUE &&
           sDDPSystemStatus.DDP_ST_SEQ_PHASELOCK == TRUE &&
           sDDPSystemStatus.DDP_ST_SEQ_FREQLOCK  == TRUE
        )
        {
            ucRecheck++ ;

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

    halAdvWarpControl(WARP_CTRL__AP);

    if( access("/mnt/configs/Geometry/HICC2/Warp0", R_OK) == 0 ||
        access("/mnt/configs/Geometry/HICC2/Blend0", R_OK) == 0 ||
        access("/mnt/configs/Geometry/HICC2/BlackLevel0", R_OK) == 0  )
    {
        LOG_MSG(db_HAL_WARPING, "Apply Current AP setting\n");

        uPALGEOAPI_INFO uInfo;
        uInfo.sGFN_AP_WARP_FUNCTION_APPLY_INFO.eType = eWAT_ALL;
        palGeo_Func_Set(eGFN_AP_WARP_FUNCTION_APPLY, &uInfo);

        uInfo.sGFN_AP_DBDBLEND_FUNCTION_APPLY_INFO.eType = eBAT_ALL;
        palGeo_Func_Set(eGFN_AP_DBDBLEND_FUNCTION_APPLY, &uInfo);

        uInfo.sGFN_AP_BKLV_FUNCTION_APPLY_INFO.eType = eLAT_ALL;
        palGeo_Func_Set(eGFN_AP_BKLV_FUNCTION_APPLY, &uInfo);

    }
    else if(palGeo_ApLinkFlag_Get() == TRUE)  //A35G2_Simon_0086
    {
        LOG_MSG(db_HAL_WARPING, "Blending AP on , no action\n");
    }
    #if 0   //no need
    else if(palDataMgr_CurrentWarpMemoryType() == WARPING_TYPE_AP)
    {
        LOG_MSG(db_HAL_WARPING, "Apply memory AP %d\n", ucWarpingApplySetting);
        palDataMgr_Access_Warping_Apply(edaWRITE_THROUGH_WITH_ACTION, &ucWarpingApplySetting);
    }
    #endif
    else
    {
        LOG_MSG(db_HAL_WARPING, "reset All\n");
        palGeo_Func_Set(eGFN_AP_CLEAR_ALL, NULL);
    }

    LOG_MSG(db_HAL_WARPING, "WarpControl End AP\n");

}

BOOL palSystem_MenuUpgradeCheck(eSYSTEM_UPGRADE_SELECT eSelect)
{
    m_bMenuUpgradeCheck = FALSE;

    switch(eSelect)
    {
        case eSYSTEM_UPGRADE_MENU_OSD:
            if(access(USB_DEV_MENU_UPGRADE_PATH, 0) != -1)
            {
                char temp[128] = {'\0'};

                sprintf(temp, "%s/OSD/BitmapRawData.dat", USB_DEV_MENU_UPGRADE_PATH);

                if(access(temp, 0) != -1)
                {
                    SYSTEM_CALL("cp %s %s", temp, OSD_BITMAP_RAWDATA);
                }
                else
                {
                    return FALSE;
                }

                sprintf(temp, "%s/OSD/TextRawData.dat", USB_DEV_MENU_UPGRADE_PATH);

                if(access(temp, 0) != -1)
                {
                    SYSTEM_CALL("cp %s %s", temp, OSD_TEXT_RAWDATA);
                }
                else
                {
                    return FALSE;
                }

                sprintf(temp, "%s/OSD/BitmapStruct.json", USB_DEV_MENU_UPGRADE_PATH);

                if(access(temp, 0) != -1)
                {
                    SYSTEM_CALL("cp %s %s", temp, BITMAP_STRUCT_JASON);
                }
                else
                {
                    return FALSE;
                }

                sprintf(temp, "%s/OSD/OSDStruct.json", USB_DEV_MENU_UPGRADE_PATH);

                if(access(temp, 0) != -1)
                {
                    SYSTEM_CALL("cp %s %s", temp, OSD_STRUCT_JASON);
                }
                else
                {
                    return FALSE;
                }

                sprintf(temp, "%s/OSD/TextStruct.json", USB_DEV_MENU_UPGRADE_PATH);

                if(access(temp, 0) != -1)
                {
                    SYSTEM_CALL("cp %s %s", temp, TEXT_STRUCT_JASON);
                }
                else
                {
                    return FALSE;
                }

                m_bMenuUpgradeCheck = TRUE;
            }
            break;

        case eSYSTEM_UPGRADE_MENU_DDP:
            break;

        case eSYSTEM_UPGRADE_MENU_MCU:
            break;

        case eSYSTEM_UPGRADE_MENU_FPGA:
            if(access(USB_DEV_MENU_UPGRADE_PATH, 0) != -1)
            {
                char temp[128] = {'\0'};

                sprintf(temp, "%s/FPGA_480.rpd", USB_DEV_MENU_UPGRADE_PATH);

                if(access(FW_UPGRADE_PATH, 0) == -1)
                {
                    mkdir(FW_UPGRADE_PATH, 0777);
                }

                if(access(temp, 0) != -1)
                {
                    SYSTEM_CALL("cp %s %s", temp, FW_FPGA480);
                    m_bMenuUpgradeCheck = TRUE;
                }
            }
            break;

        case eSYSTEM_UPGRADE_MENU_ISO:
            break;

        default:
            break;
    }

    return m_bMenuUpgradeCheck;
}

BOOL palSystem_MenuUpgradeGet(void)
{
    return m_bMenuUpgradeCheck;
}

BOOL palSystem_SystemCheckGet(void)
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
//H2 wait review
#if 1 //HICC2_Doulas_0098
    sUtilHPBUTester_CALLBACK sHPBUTester_Cb ;
    sHPBUTester_Cb.fpDataMgr_ServiceModeSetCb =                  palDataMgr_ServiceModeSet;
#ifdef PALDATAMGR_ACCESS_WITHLOG
    sHPBUTester_Cb.fpDataMgr_Data_AccessCb =                     palDataMgr_Data_Access_WithLog;
#else
    sHPBUTester_Cb.fpDataMgr_Data_AccessCb =                     palDataMgr_Data_Access;
#endif
    sHPBUTester_Cb.fpDataMgr_Access_Get_FAN_DutyCb =             palDataMgr_Access_Get_FAN_Duty;
    sHPBUTester_Cb.fpDataMgr_Access_Get_FAN_RPMCb =              palDataMgr_Access_Get_FAN_RPM;
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
    sHPBUTester_Cb.fpDataMgr_Data_StructDataAccessCb =           NULL;
    sHPBUTester_Cb.fpIllumination_TEC_Gating_ResultGetCb =       NULL;

    utilHPBUTest_CLI_RegCallback(sHPBUTester_Cb);
#endif
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
#if defined(SCALER_FPGA_F34)
    sWarpDemo_Cb.fpGeo_DrawOsdRectCb = palSystem_WarpPattern_DrawRect_Event;
    sWarpDemo_Cb.fpGeo_EnableTestPatternCb = palSystem_WarpPattern_OsdOn_Event;
    sWarpDemo_Cb.fpGeo_ClearTestPatternCb = palSystem_WarpPattern_ClearOsd_Event;
#endif

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
#if (ENABLE_COLOR_UNIFORMITY == TRUE)
    sCLICmd_Cb.fpDataMgr_ACU_Target_Select_SetCb =                  palDataMgr_ACU_Target_Select_Set;
#else
    sCLICmd_Cb.fpDataMgr_ACU_Target_Select_SetCb =                  NULL;
#endif /* ENABLE_COLOR_UNIFORMITY */
    sCLICmd_Cb.fpDataPath_BackupConfigFlagCb =                      NULL;//palDataPath_BackupConfigFlag;
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

    MS_SLEEP(500);

    return eResult;
}

UINT8 appSystem_FW_UpgradeCheckGet(void)
{
    //0 is Idel, 1 is busy
    return m_sPalSysInfo.ucFW_upgrade;
}

#ifdef OPFU_TAG_ENABLE
static void palSystem_OPFU_Tag(void)
{
    if(access("/tmp/OpfuIdTmp.conf", R_OK) == 0)
    {
        FILE *pFile = NULL;
        UINT32 ulSize = 0;
        UINT32 Model = 0;

        pFile = fopen("/tmp/OpfuIdTmp.conf", "r");

        fseek(pFile, 0, SEEK_END);
        ulSize = ftell(pFile);
        fclose(pFile);

        char *conf_argv[1][2];
        char *pcBuffer;
        pcBuffer = (char*)malloc(ulSize+1);

        readConf("/tmp/OpfuIdTmp.conf", conf_argv, 1, pcBuffer, ulSize);

        sscanf(conf_argv[0][1], "%x", &Model);

#ifdef OPFU_TAG_ENABLE_PLATFORM_ID
        Model = (Model>>8) & 0xFF;

        switch(Model)
        {
            case MODEL_ID_0:
                if(MODULE_TYPE_ID0_PLATFORM != MODULE_NA_PLATFORM)
                {
                    LOG_MSG(db_APP_SYSTEM, "PLATFORM ID Set 0x%02x\n", MODULE_TYPE_ID0_PLATFORM);
                    palSystem_ModelIDSet(MODULE_TYPE_ID0_PLATFORM);
                    halMCU_PlatformID_Set(MODULE_TYPE_ID0_PLATFORM);
#ifdef PLATFORM_A35G2
                    halLDCtrl_PlatformID_Set(MODULE_TYPE_ID0_PLATFORM);
#else
                    halFanCtrl_Platform_Set(Board_ModelID_Get());
#endif
                }
                break;

            case MODEL_ID_1:
                if(MODULE_TYPE_ID1_PLATFORM != MODULE_NA_PLATFORM)
                {
                    LOG_MSG(db_APP_SYSTEM, "PLATFORM ID Set 0x%02x\n", MODULE_TYPE_ID1_PLATFORM);
                    palSystem_ModelIDSet(MODULE_TYPE_ID1_PLATFORM);
                    halMCU_PlatformID_Set(MODULE_TYPE_ID1_PLATFORM);
#ifdef PLATFORM_A35G2
                    halLDCtrl_PlatformID_Set(MODULE_TYPE_ID1_PLATFORM);
#else
                    halFanCtrl_Platform_Set(Board_ModelID_Get());
#endif
                }
                break;

            case MODEL_ID_2:
                if(MODULE_TYPE_ID2_PLATFORM != MODULE_NA_PLATFORM)
                {
                    LOG_MSG(db_APP_SYSTEM, "PLATFORM ID Set 0x%02x\n", MODULE_TYPE_ID2_PLATFORM);
                    palSystem_ModelIDSet(MODULE_TYPE_ID2_PLATFORM);
                    halMCU_PlatformID_Set(MODULE_TYPE_ID2_PLATFORM);
#ifdef PLATFORM_A35G2
                    halLDCtrl_PlatformID_Set(MODULE_TYPE_ID2_PLATFORM);
#else
                    halFanCtrl_Platform_Set(Board_ModelID_Get());
#endif
                }
                break;
        }
#else
    halMCU_ProjectorID_Set((UINT8*)&Model);
    LOG_MSG(db_APP_SYSTEM, "Projector ID Set 0x%08x\n", Model);
#endif /* OPFU_TAG_ENABLE_PLATFORM_ID */
    }
}
#endif /* OPFU_TAG_ENABLE */

