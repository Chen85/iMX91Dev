#ifndef _PALSYSTEM_H_
#define _PALSYSTEM_H_

#ifdef __cplusplus
extern "C" {
#endif


#include "Common.h"
#include "appMailBox.h"
#include "utilCounterAPI.h"

//====================== Enum Start ======================//

typedef enum
{
    eSYSTEM_EVENT_IDLE,         /* wait power key event */
    eSYSTEM_EVENT_START,        /* start power state machine coming out of reset */
    eSYSTEM_EVENT_POWERDOWN,    /* perform normal power-down */
    eSYSTEM_EVENT_POWERKEY,     /* power key pressed (system not in active mode) */
    eSYSTEM_EVENT_FAULT,        /* fault detected, perform immediate power-down */
    eSYSTEM_EVENT_COOL,         /* cooldown is complete */
    eSYSTEM_EVENT_PROGMODE,     /* transition to flash programming mode */
    eSYSTEM_EVENT_CFU_SELECT,   /* for fw upgrade select*/ 	//A70LV_Larry_0112
    eSYSTEM_EVENT_BIN2FLASH,
    eSYSTEM_EVENT_CUSTOMER_SPLAH,
    eSYSTEM_EVENT_LOGO_CAPTURE,
    eSYSTEM_EVENT_LOGO_REPLACEMENT,
    eSYSTEM_EVENT_SCREEN_SAVE_TO_IMAGE,// R70G2_Bruce#0022
    eSYSTEM_EVENT_ENTER_2W_MODE, //G100_Larry_0022
    eSYSTEM_EVENT_CUSTOMIZE_EDID,   //A35G2_Simon_0091
    eSYSTEM_EVENT_APPLY_AP_MODE_GEO_FUNC,   //A35G2_Simon_0100
    eSYSTEM_EVENT_ACTUATOR_PATTERN,

}eSYSTEM_EVENT;


typedef enum
{
    eSTARTUP_STATE_NORMAL,        /* Normal startup (operating) */
    eSTARTUP_STATE_STANDBY,       /* Standby (no color wheel or lamp) */
    eSTARTUP_STATE_LOWPOWER       /* Low power (all modules at minimum power */
}eSTARTUP_STATE;

typedef enum
{
    eSYSTEM_UPGRADE_MODE_SET,
    eSYSTEM_UPGRADE_SELECT_LPCMCU,
    eSYSTEM_UPGRADE_SELECT_SCALER_FOCUS,
    eSYSTEM_UPGRADE_SELECT_SCALER,
    eSYSTEM_UPGRADE_SELECT_DDP,
    eSYSTEM_UPGRADE_SELECT_STARTUP_BASED,
    eSYSTEM_UPGRADE_SELECT_FPGA,
    eSYSTEM_UPGRADE_SELECT_All,
    eSYSTEM_UPGRADE_DISABLE_UPGRADE_MODE = 9,

	eSYSTEM_UPGRADE_SECOND_LOGO_REPLACE,
	eSYSTEM_UPGRADE_SERVICE_SECOND_LOGO_REPLACE, //A65_OPTOMA_Julie_0076
	eSYSTEM_UPGRADE_CUSTOMIZE_EDID,  //A35G2_Simon_0091

    eSYSTEM_UPGRADE_SELECT_NUMBER,
}eSYSTEM_UPGRADE_SELECT; //A70LV_Larry_0112

typedef enum
{
    eSTB_POWER_STATE_STANDBY,           // Standby 0.5W
    eSTB_POWER_STATE_2WMODE,            // Communication 2W  //STB_Energy_0001
    eSTB_POWER_STATE_COMMUNICATION,     // Communication 8W
    eSTB_POWER_STATE_WARM_UP,           // Runnig Gpio cfg, power sequence.
    eSTB_POWER_STATE_PROJ_ON,           // Board Power On ok
    eSTB_POWER_STATE_PROJ_OFF,          // Go to Power Off

    eSTB_POWER_STATE_NUMBERS,
} eSTB_POWER_STATE;

typedef enum
{
    eSTB_PRPHRL_ADC = 1,
    eSTB_PRPHRL_KEYPAD,
} eSTB_PRPHRL_RECOVER;

//====================== Enum End ======================//

//===================== Struct Start ===================//

#if (SYSTEM_OS_TYPE == POSIX_COMPLIANT)

    typedef struct
    {
        pthread_t                    xTaskHandle;       // Task
        pthread_attr_t               xTaskHandle_attr;  // Task attribute
        mqd_t                        xMsgQueue;         // queue descriptors
        struct mq_attr               xMsgQueue_attr;    // queue attribute
        pthread_mutex_t              xTaskMutex;
        BOOL                         bTaskPause;

        EventGroupHandle_t           xEventGroupHandle; //unused

        ePANEL_ID                    ePanelTimingId;
        ePOWER_STATE                 ePowerState;

        UINT8                        ucHostReady;
        UINT8                        ucFW_upgrade; //A70LV_Larry_0112
        //UINT8                        ucModelID; //rename from ucCustomID  //A70LV_Larry_0132
        UINT8                        ucCustomer_Splash;
        UINT32                       ulSystemCount;
        UINT8                        ucSystemMcuState;
        UINT8                        ucSingle_iScaler_Mode;  //No UI

    }sPAL_SYSTEM_INFORMATION, *PsPAL_SYSTEM_INFORMATION;

#else  //freertos

    typedef struct
    {
        TaskHandle_t                 xTaskHandle;
        xQueueHandle                 xMsgQueue;
        EventGroupHandle_t           xEventGroupHandle;

    #if(CURRENT_RTOS_TYPE == RTOS_STATIC)
        StaticQueue_t                xStaticQueue;
        UINT8                        ucQueueStorageArea[MAIL_BOX_QUEUE_SIZE * sizeof(sMAIL_BOX_MESSAGE)];
        StaticEventGroup_t           xCreatedEventGroup;
    #endif

        ePANEL_ID                    ePanelTimingId;
        ePOWER_STATE                 ePowerState;

        UINT8                        ucHostReady;
        UINT8                        ucFW_upgrade; //A70LV_Larry_0112
        //UINT8                        ucModelID; //rename from ucCustomID   //A70LV_Larry_0132
        UINT32                       ulSystemCount;
        UINT8                        ucSystemMcuState;

    }sPAL_SYSTEM_INFORMATION, *PsPAL_SYSTEM_INFORMATION;

#endif


//// Monitor Task Start ////      //A35G2_Simon_0075
typedef struct
{
    char acTaskName[32] ;
    UINT16 uiWaitResponseTime;
    UINT16 uiCountdownTimer;  //if it counts down to 0, it means there is no response. (need to record log)
    eCOUNTER_TYPE eCounterType;
    UINT8 ucMaxLogOutNum;  //max record log number
    UINT8 ucCountdownMaxLogOut;  //if it counts down to 0, it will stop recording log or OPD.
    UINT8 ucTaskWorkStatus;
    UINT8 ucStopMonitor; //if TRUE, no monitor this task
    char acLogInfoStr[64] ;
}sMONITOR_TASK_INFO;

typedef enum
{
    eTID_CLI_CMD_PROC,
    eTID_CLI_CMD_DECODE,
    eTID_DATAPATH,
    eTID_SUB_DATAPATH,
    eTID_GUI,
    eTID_HOST_PATH,
    eTID_POLLING,
    eTID_IPC_RECV_DATA,
    eTID_SYSTEM,

    eTID_NUMBER,
}eTASK_ID;

#define TASK_STATUS_NORMAL     (0)
#define TASK_STATUS_NO_RESPONE (1)
#define TASK_STATUS_SUSPEND    (2)

//// Monitor Task End ////


typedef struct
{
    BOOL bActuatorEnable;
    BOOL bBlicking;
}sSYSTEM_CHANGEPANELID_INFO;

//===================== Struct End ===================//

#if (SYSTEM_OS_TYPE == POSIX_COMPLIANT)
pthread_t* palSystem_GetTaskHandle(void);
pthread_attr_t* palSystem_GetTaskAttr(void);
#else  //freertos
TaskHandle_t* palSystem_GetTaskHandle(void);
#endif

void* palSystem_SysMonTask(void* pParameters);
void palSystem_CoolDown(void);
void palSystem_PowerDown(void);

void palSystem_PowerKey(void);
void palSystem_Idle(void);
void palSystem_WarmUp(void);
void palSystem_CustomerSplah(UINT8 Customer); //A70LV_Larry_0235
void palSystem_FwUpgradeMode(UINT8 ucSelect);  //A70LV_Larry_0112
void palSystem_LogoCapture(void);           //A65_Owen_0006
void palSystem_ScreenSaveToImage(void);
void palSystem_LogoReplacement(void);
void palSystem_Enter2WMode(void); //G100_Larry_0022
void palSystem_WriteCustomizeEDID_Event(UINT8 ucSourceID);  //A35G2_Simon_0091
void palSystem_ApplyAPModeGeoFunc_Event(void);  //A35G2_Simon_0100
void palSystem_ActuatorPatternSet(UINT8 ucPatten);
void palSystem_ChangePanelID(ePANEL_ID ePanelId, sSYSTEM_CHANGEPANELID_INFO *psInfo);   //A70LV_Doulas_0005
BOOL appSysInitAfterAsicReady(void);    // A70LV_Eric.C_0021 //A70LV_Larry_0097
BOOL palSystem_NoFanControlFlagGet(void); //A70LV_Larry_0079
eRESULT palSystem_VersionCheck(BOOL bAutoUpgrade);  //A70LV_Larry_0112
void palSystem_Scaler_Upgrade(UINT8 ucFocus);
#if (LOGO_REPLACE == 1)	//ZU860_Clare_0124
void palSystem_SecondLogoPartialUpgrade(UINT8 ucFocus);
void palSystem_ServiceCustomLogoPartialUpgrade(UINT8 ucFocus); //A65_OPTOMA_Julie_0076
#endif
void palSystem_PowerOff_Config(void);  //A70LV_Larry_0112
UINT8 palSystem_FW_UpgradeCheckGet(void);  //A70LV_Larry_0112
void palSystem_FW_UpgradeCheckSet(void);
UINT8 palSystem_ModelIDGet(void);
void palSystem_ModelIDSet(UINT8 ucData);
ePOWER_STATE palSystem_PowerStateGet(void); //A70LV_Larry_0135
UINT8 palSystem_HostReadyGet(void);
void palSystem_HostReadySet(UINT8 ucValue);
UINT8 palSystem_SystemReadyGet(void);
void palSystem_SystemReadySet(UINT8 ucValue);
ePANEL_ID palSystem_PanelID_Get(void);      //A70LV_Doulas_0155
void palSystem_PanelID_Set(ePANEL_ID PanelID);
void palSystem_PanelResolution_Get(ePANEL_ID ePanelId, UINT16 *HRes, UINT16 *VRes);
eRESULT palSystem_UpgradeLPCMCU(UINT8 cIndex, BOOL bEnforce); //A70LV_Larry_0189
UINT8 palSystem_CustomerSplashGet(void); //A70LV_Larry_0235
void palSystem_CustomerSplashSet(UINT8 ucCustomer); //A70LV_Larry_0235
eRESULT palSystem_Suspend(const char *cFuncName);
eRESULT palSystem_Resume(const char *cFuncName);

BOOL palSystem_TaskPauseGet(void);
void palSystem_12VPwrLostSet(BOOL bFlag);   //G100_Owen_0064
UINT8 palSystem_OPFUCheck(void);
eRESULT palSystem_AC_Voltage_Check(void);                   //G100_Tim_0009, add
BOOL palSystem_IS_LOGO_Capture_Enble(void);		//A65_OPTOMA_Doulas_0125
eRESULT palSystem_Model_ID_Get(void); //A35G2_BRC_Casper_0023
UINT8 palSystem_WaitWheelStable(void); //A35G2_CDS_Simon_0061
UINT8 palSystem_LensModelGet(void);  //A35G2_Larry_0063
void palSystem_LensModelSet(UINT8 ucType); //A35G2_Larry_0066
void palSystem_TaskMonitor(void);  //A35G2_Simon_0075
void palSystem_TaskMonitorTimerReset(eTASK_ID eTaskID, char *InfoStr);  //A35G2_Simon_0075
eRESULT palSystem_WriteCustomizedEDID(eCM_SOURCE_ID eSource);  //A35G2_Simon_0091
void palSystem_Apply_AP_Mode_GeoFunc(void);  //A35G2_Simon_0100

void Init_GEC_Callback(void);
void Init_Formatter_Callback(void);
void Init_Database_Callback(void);
void Init_HPBUTester_Callback(void);
void Init_IPC_Callback(void);
void Init_WarpDemo_Callback(void);
void Init_CLICmd_Callback(void);

void palSystem_NULL_Function(void);

UINT8 palSystem_SingleiScalerModeFlagGet(void);
void palSystem_SingleiScalerModeFlagSet(UINT8 Data);
eEXEC_CODE palSystem_PowerStandby_OSDExit(void);
BOOL palSystem_SystemCheckGet(void);


#ifdef __cplusplus
}
#endif


#endif  //_PALSYSTEM_H_
