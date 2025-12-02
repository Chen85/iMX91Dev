#ifndef _PALDATAPATH_H_
#define _PALDATAPATH_H_

#ifdef __cplusplus
extern "C" {
#endif


#include "utilStorageCfg.h"
#include "Common.h"
#include "appMailBox.h"
#include "halWarping.h"
#include "utilCommon.h"

#define DATAPATH_STATE_PASS 1
#define DATAPATH_STATE_SET_SOURCE_FAIL -1

#define DATAPATH_STATE_FIRST_LANGAGE_MEOUT 60000 //60 secs

//====================== Enum Start ======================//
typedef enum
{
    eDATA_PATH_MSG_NONE,
    eDATA_PATH_MSG_INIT,
    eDATA_PATH_MSG_GETREADY,
    eDATA_PATH_MSG_FLUSHED,
    eDATA_PATH_MSG_SYNCSDETECTED,
    eDATA_PATH_MSG_LOCKED,
    eDATA_PATH_MSG_LOSTLOCK,
    eDATA_PATH_MSG_REACQUIRE,
    eDATA_PATH_MSG_SUSPENDSCAN,
    eDATA_PATH_MSG_RESUMESCAN,
    eDATA_PATH_MSG_RESYNC,
    eDATA_PATH_MSG_PANEL_CHANGE,    //A70LV_Doulas_0005
    eDATA_PATH_MSG_RECONFIG_IMAGE,          //A70LV_Doulas_0009
    eDATA_PATH_MSG_TEST_PATTERN,        //A70LV_Doulas_0030

    eDATA_PATH_MSG_GOTOSTATE,
    eDATA_PATH_MSG_SETCONNECTOR,
    eDATA_PATH_MSG_BACKUP_PRIMARY_CONNECTOR,
    eDATA_PATH_MSG_BACKUP_SECONDARY_CONNECTOR,

}eDATA_PATH_MSG;

typedef enum
{
    eDATA_PATH_STATE_SPLASH_AT_STARTUP,
    eDATA_PATH_STATE_TPG_DISPLAYED,
    eDATA_PATH_STATE_SUSPENDED,
    eDATA_PATH_STATE_SCALER_PORT_CONFIG, //Roger Added 20160707
    eDATA_PATH_STATE_BEGIN_SCAN,  //no signal //4
    eDATA_PATH_STATE_LOOK_FOR_SYNCS, // 5
    eDATA_PATH_STATE_ATTEMPT_LOCK, // 6
    eDATA_PATH_STATE_AUTO_PHASE,        //A70LV_Doulas_0007
    eDATA_PATH_STATE_AUTO_POSITION,     //A70LV_Doulas_0007
    eDATA_PATH_STATE_MONITOR_SOURCE, // 9
    eDATA_PATH_STATE_SOURCE_LOST, // 10
    eDATA_PATH_STATE_INVALID,  //A70LV_Larry_0108
}eDATA_PATH_STATE;

typedef enum
{
	eDATA_PATH_CONNECTOR_VGA,
    eDATA_PATH_CONNECTOR_DVI,
	eDATA_PATH_CONNECTOR_HDMI1,     //A70LV_Doulas_0007
    eDATA_PATH_CONNECTOR_HDMI2,     //A70LV_Doulas_0007
	eDATA_PATH_CONNECTOR_HDBASET,
	eDATA_PATH_CONNECTOR_3G_SDI,    //A70LV_Doulas_0007
	eDATA_PATH_CONNECTOR_DAUGHTER1,
	eDATA_PATH_CONNECTOR_DAUGHTER2,
	eDATA_PATH_CONNECTOR_LAST_CONNECTOR,
	eDATA_PATH_CONNECTOR_INVALID_CONNECTOR,
	eDATA_PATH_CONNECTOR_NEXT_CONNECTOR,
	eDATA_PATH_CONNECTOR_NOSOURCE,
	eDATA_PATH_CONNECTOR_BLANKSCREEN,
	eDATA_PATH_CONNECTOR_SPLASH,
	eDATA_PATH_CONNECTOR_TESTPATTERN
}eDATA_PATH_CONNECTOR;

typedef enum
{
    eDATA_PATH_ACTIVE_DISPLAY_EXTERNAL,
    eDATA_PATH_ACTIVE_DISPLAY_SFG,
    eDATA_PATH_ACTIVE_DISPLAY_SPLASH,
    eDATA_PATH_ACTIVE_DISPLAY_TESTPATTERN
}eDATA_PATH_ACTIVE_DISPLAY;

//====================== Enum End ======================//

//===================== Struct Start ==================//


typedef struct
{
	eCM_SOURCE_ID                   eConnector;     //A70LV_Doulas_0029 modify
    UINT8                           ucConnectorChannel;
    UINT8                           ucPrimaryInput;
	UINT8                           ucSecondaryInput;
	UINT8                           ucAutoBackupInput;
	UINT8                           ucCurrentBackupInput;
    eDATA_PATH_ACTIVE_DISPLAY       eActiveDisplay;
}sDATA_PATH_SOURCE_DESC, *PsDATA_PATH_SOURCE_DESC;


#if (SYSTEM_OS_TYPE == POSIX_COMPLIANT)

    typedef struct
    {
        pthread_t                       xTaskHandle;       // Task
        pthread_attr_t                  xTaskHandle_attr;  // Task attribute
        mqd_t                           xMsgQueue;         // queue descriptors
        struct mq_attr                  xMsgQueue_attr;    // queue attribute
        pthread_mutex_t                 xTaskMutex;
        pthread_cond_t                  xTaskCond;
        BOOL                            bTaskPause;

        EventGroupHandle_t              xEventGroupHandle; //unused

        ePANEL_ID                       ePanelTimingId;
        BOOL                            bDataPathReady;
        eDATA_PATH_STATE                eDataPathState;
        //Synchronizes flush messages
        UINT8                           ucFlushIndex;


        INT32                           lPollPeriod;
        UINT32                          ulStateStartTicks;

        eHAL_WARPING_WARP_MODE          eMode;
        BOOL                            bGridShow;
        eHAL_WARPING_MOVE_PITCH         eMoveIdx;

        sHALWARPING_PJPARAM             sWarpPjParam;
        UINT8                           ucSyncLockCount;    //A70LV_Doulas_0007
        BOOL                            b3DSyncIn;
        UINT16                          uiPanelChange;
        BOOL                            bXPRErrorCheck;
        BOOL                            bTestPatternTrigger;
        UINT8                           ucWarpingAPConfig;
        UINT8                           ucSourceEnhanceCheck; //for enhance mode
        UINT8                           ucSourceConnectChange;

    }sPAL_DATA_PATH_INFORMATION, *PsPAL_DATA_PATH_INFORMATION;

#else //freertos

    typedef struct
    {
        TaskHandle_t					xTaskHandle;
        xQueueHandle                    xMsgQueue;
        EventGroupHandle_t              xEventGroupHandle;

    #if(CURRENT_RTOS_TYPE == RTOS_STATIC)
        StaticQueue_t                   xStaticQueue;
        UINT8                           ucQueueStorageArea[MAIL_BOX_QUEUE_SIZE * sizeof(sMAIL_BOX_MESSAGE)];
        StaticEventGroup_t              xCreatedEventGroup;
        StaticTask_t                    xTaskBuffer;
        StackType_t                     xStack[APP_DATAPATH_STACK_SIZE];
    #endif


        ePANEL_ID						ePanelTimingId;
        BOOL							bDataPathReady;
        eDATA_PATH_STATE                eDataPathState;
        //Synchronizes flush messages
        UINT8							ucFlushIndex;


        INT32							lPollPeriod;
        TickType_t						ulStateStartTicks;

        eHAL_WARPING_WARP_MODE          eMode;
        BOOL                            bGridShow;
        eHAL_WARPING_MOVE_PITCH         eMoveIdx;

        sHALWARPING_PJPARAM             sWarpPjParam;
        UINT8                           ucSyncLockCount;    //A70LV_Doulas_0007

    }sPAL_DATA_PATH_INFORMATION, *PsPAL_DATA_PATH_INFORMATION;

#endif
//====================== Struct End ====================//

void palDataPath_Task_Suspend(void) ;
void palDataPath_Task_Resume(void);

eEXEC_CODE palDataPath_Init(ePANEL_ID ePanelId, const PsSYSTEM_CONFIGURATION psSysConfiguration);
eEXEC_CODE palDataPath_PowerNormal(ePANEL_ID ePanelId);
eEXEC_CODE palDataPath_PowerStandby(void);
eEXEC_CODE palDataPath_GetPathReady(void);
void palDataPath_Resync(void);
void palDataPath_ResyncCheckEnhance(void); //A70LK_Larry_0169
eEXEC_CODE palDataPath_Panel_Change(ePANEL_ID ePanelId);   //A70LV_Doulas_0005
eEXEC_CODE palDataPath_InitInputSource(void);   //A70LV_Doulas_0007
eEXEC_CODE palDataPath_Reconfig_Image(void);      //A70LV_Doulas_0009
eEXEC_CODE palDataPath_OverscanSet(UINT8 ucOverScan);   //A70LV_Doulas_0009
eEXEC_CODE palDataPath_Digital_Horz_Zoom_Set(UINT16 uwDigital_Horz_Zoom);   //A70LV_Doulas_0009
eEXEC_CODE palDataPath_Digital_Vert_Zoom_Set(UINT16 uwDigital_Vert_Zoom);   //A70LV_Doulas_0009
eEXEC_CODE palDataPath_Digital_Horz_Shift_Set(UINT16 uwDigital_Horz_Shift);   //A70LV_Doulas_0009
eEXEC_CODE palDataPath_Digital_Vert_Shift_Set(UINT16 uwDigital_Vert_Shift);   //A70LV_Doulas_0009
eEXEC_CODE palDataPath_Vert_Start_Position_Set(UINT8 uwVertPosition);   //A70LV_Doulas_0010
eEXEC_CODE palDataPath_Horz_Start_Position_Set(UINT8 uwHorzPosition);   //A70LV_Doulas_0010
eEXEC_CODE palDataPath_Format_ContrastEnhancementSet(UINT8 ucContrastEnhancement, UINT8 ucRealBlackLevel);   //A70LV_Doulas_0013
void palDataPath_BurnInStart(void);  //A70LV_Doulas_0023
eEXEC_CODE palDataPath_UserSetSourceInput(eCM_SOURCE_ID eInputSource);    //A70LV_Doulas_0029
eEXEC_CODE palDataPath_BackupPrimaryInput(eCM_SOURCE_ID eInputSource);
eEXEC_CODE palDataPath_BackupSecondaryInput(eCM_SOURCE_ID eInputSource);
eEXEC_CODE palDataPath_AspectRatioSet(UINT8 ucAspectRatio);
BOOL palDataPath_IsSourceLock(void);       //A70LV_Doulas_0032
BOOL palDataPath_IsSourceMonitor(void);       //A70LV_Doulas_0101
void palDataPath_SourceLostSet(void);
BOOL palDataPath_ShowLogo(void);
eEXEC_CODE palDataPath_OSD_TestPattern_Set(UINT8 uwTestPattern);   //A70LV_Doulas_0035
eEXEC_CODE palDataPath_Service_TestPattern_Set(UINT8 uwTestPattern);
eEXEC_CODE palDataPath_HSG_TestPattern_Set(UINT8 ucTestPattern);     //A70LV_Doulas_0063
BOOL palDataPath_IsTestPatternEnable(void);
//A70LV_Doulas_0038
eEXEC_CODE palDataPath_Formatter_Gamma_Set(UINT8 ucGamma);
BOOL palDataPath_InputSourceIs3DFormat_Get(void);
eEXEC_CODE palDataPath_InputPixelClock_Get(UINT8 *ucValue);   //A70LV_Doulas_0056
eEXEC_CODE palDataPath_InputSignalFormat_Get(UINT8 *ucValue);
eEXEC_CODE palDataPath_InputResoultion_Get(UINT8 *ucValue);
eEXEC_CODE palDataPath_InputHorzRefresh_Get(UINT8 *ucValue);
eEXEC_CODE palDataPath_InputVertRefresh_Get(UINT8 *ucValue);
eEXEC_CODE palDataPath_InputSyncType_Get(UINT8 *ucValue);
eEXEC_CODE palDataPath_InputAspectRatio_Get(UINT8 *ucValue);
eEXEC_CODE palDataPath_InputColorSpace_Get(UINT8 *ucValue);
BOOL palDataPath_Main_Or_Sub_SourceMonitor(void);       //A70LV_Doulas_0101
eEXEC_CODE palDataPath_InitVGA_SyncThreshold(void);   //A70LV_Doulas_0115
eEXEC_CODE palDataPath_Init_FrontEnd_PIP_Setting(void);   //A70LV_Doulas_0120
eEXEC_CODE palDataPath_AutoDisableMenuTransparency(void);   //A70LV_Doulas_0122
UINT8 palDataPath_Input3D_FormatGet(void);                  //A70LV_Doulas_0154
UINT8 palDataPath_PanelOutputGet(void);
void palDataPath_Input3D_PanelSet(void);       //A70LV_Doulas_0159
UINT8 palDataPath_Input3D_Timing_Get(void);               //A70LV_Doulas_0196
eEXEC_CODE palDataPath_ModeAdjusmenttEnableSetting(void);   //A70LV_Doulas_0209
eEXEC_CODE palDataPath_ModeAdjusmenttDisableSetting(void);   //A70LV_Doulas_0209
eEXEC_CODE palDataPath_FreezeChecking(UINT8 ucCH);   //A70LV_Doulas_0223 Freeze check


void palDataPath_TaskSub_Suspend(void);
void palDataPath_TaskSub_Resume(void);
eEXEC_CODE palDataPath_InitSub(ePANEL_ID ePanelId, const PsSYSTEM_CONFIGURATION psSysConfiguration);
eEXEC_CODE palDataPath_PowerNormalSub(ePANEL_ID ePanelId);
eEXEC_CODE palDataPath_PowerStandbySub(void);
eEXEC_CODE palDataPath_GetPathReadySub(void);
void palDataPath_ResyncSub(void);
eEXEC_CODE palDataPath_Panel_ChangeSub(ePANEL_ID ePanelId);   //A70LV_Doulas_0005
eEXEC_CODE palDataPath_InitInputSourceSub(void);   //A70LV_Doulas_0007
eEXEC_CODE palDataPath_Reconfig_ImageSub(void);     //A70LV_Doulas_0009
eEXEC_CODE palDataPath_OverscanSetSub(UINT8 ucOverScan);   //A70LV_Doulas_0009
eEXEC_CODE palDataPath_UserSetSourceInputSub(eCM_SOURCE_ID eInputSource);    //A70LV_Doulas_0029
BOOL palDataPath_IsSourceLockSub(void);       //A70LV_Doulas_0032
BOOL palDataPath_IsSourceMonitorSub(void);       //A70LV_Doulas_0098
void palDataPath_SourceLostSetSub(void);
BOOL palDataPath_ShowLogoSub(void);
eEXEC_CODE palDataPath_InputPixelClockSub_Get(UINT8 *ucValue);   //A70LV_Doulas_0056
eEXEC_CODE palDataPath_InputSignalFormatSub_Get(UINT8 *ucValue);
eEXEC_CODE palDataPath_InputResoultionSub_Get(UINT8 *ucValue);
eEXEC_CODE palDataPath_InputHorzRefreshSub_Get(UINT8 *ucValue);
eEXEC_CODE palDataPath_InputVertRefreshSub_Get(UINT8 *ucValue);
eEXEC_CODE palDataPath_InputSyncTypeSub_Get(UINT8 *ucValue);
eEXEC_CODE palDataPath_InputAspectRatioSub_Get(UINT8 *ucValue);
eEXEC_CODE palDataPath_InputColorSpaceSub_Get(UINT8 *ucValue);
eEXEC_CODE palDataPath_FrontEndVideoInfoSub_Set(void);      //A70LV_Doulas_0076 Add
eDATA_PATH_STATE palDataPath_GetDataPathState(void);  //A70LV_Larry_0108
eDATA_PATH_STATE palDataPath_GetDataPathSubState(void); //A70LV_Doulas_0236
BOOL palDataPath_AutoCheckColorSpaceChangeSub(void);        //A70LV_Doulas_0283
eCM_SOURCE_ID palDataPath_CurrentBackupInputGet(void);
eEXEC_CODE palDataPath_LogoCapture(void);
void palDataPath_AlreadyShowDisconnectTwistMsg_Flag_Set(UINT8 ucValue);
UINT8 palDataPath_HDR_Info_Sub_Get(void);	//A65_OPTOMA_Doulas_0149
UINT8 palDataPath_BKInput_First_Input_Ready(void);
UINT8 palDataPath_BKInput_Second_Input_Ready(void);
void palDataPath_Color_Uniformity_Recheck(void);
BOOL palDataPath_3DSyncInDet(void);
eEXEC_CODE palDataPath_Panel_ChangeCheck(BOOL bBlicking);
UINT8 palDataPath_AlreadyShowDisconnectTwistMsg_Flag_Get(void);  //A70LK_Simon_0018
void palDataPath_AlreadyShowDisconnectTwistMsg_Flag_Set(UINT8 ucValue);  //A70LK_Simon_0018
BOOL palDataPath_IsErrorInputGetForTwistOn(void);  //A70LK_Simon_0018
void palDataPath_InputDetectSet(UINT16 uiData);  //A35G2_Simon_0075
UINT16 palDataPath_InputDetectGet(void);  //A35G2_Simon_0075
void palDataPath_LastInputDetectSet(UINT16 uiData);  //A35G2_Simon_0075
UINT16 palDataPath_LastInputDetectGet(void);  //A35G2_Simon_0075
eEXEC_CODE palDataPath_TestPatternEntry(void);
void palDataPath_CustomizedEDID_Init(void);  //A35G2_Simon_0091
UINT8 palDataPath_PanelConfigGet(void); //A70LK_Larry_0180
INT8  palDataPath_MenuTransparencyEnableSet(UINT8 ucEnalbe);
UINT8 palDataPath_OSD_MenuTransparencyEnableGet(void);
void palDataPath_SmoothOnReadyCheck_Set(UINT8 ucData);
void palDataPath_ProcMutexDataScalerInfoInit(void);      //H2PF_Simon_0033
UINT8 palDataPath_GetPanelChangeState(void);
UINT8 palDataPath_BKInput_Change(void);
UINT8 palDataPath_BKInput_Status(void);
eEXEC_CODE palDataPath_EdgeMaskSet(UINT8 ucEdgeMask);
void palDataPath_Low_Latency_Set(UINT8 ucVal);
UINT8 palDataPath_Last3DEnable_Get(void);
UINT8 FrontEnd_Down_Scaling_Get(void);
eEXEC_CODE palDataPath_BKInput_SecondHRefresh_Get(UINT8 *ucValue);
eEXEC_CODE palDataPath_BKInput_FirstColorSpace_Get(UINT8 *ucValue);
eEXEC_CODE palDataPath_BKInput_SecondVideoInfo_Get(void);
eEXEC_CODE palDataPath_BKInput_SecondResoultion_Get(UINT8 *ucValue);
eEXEC_CODE palDataPath_BKInput_FirstVideoFormat_Get(void);
eEXEC_CODE palDataPath_BKInput_FirstHRefresh_Get(UINT8 *ucValue);
eEXEC_CODE palDataPath_BKInput_FirstVideoInfo_Get(void);
eEXEC_CODE palDataPath_BKInput_FirstResoultion_Get(UINT8 *ucValue);
eEXEC_CODE palDataPath_BKInput_SecondVideoFormat_Get(void);
eEXEC_CODE palDataPath_BKInput_SecondColorSpace_Get(UINT8 *ucValue);
UINT8 palDataPath_First_Input_HDR_Get(void);
UINT8 palDataPath_Second_Input_HDR_Get(void);
eEXEC_CODE palDataPath_Formatter_HDR_Picture_Mode_Set(UINT8 Mode, UINT8 ucHDRSelect);
void palDataPath_Test_SetTimeStart(void);
void palDataPath_Auto_HDMI_Switch_Source_Config( eCM_SOURCE_ID eSource_ID);
void palDataPath_BackupConfigFlag(void);

#ifdef __cplusplus
}
#endif
#endif  //_PALDATAPATH_H_
