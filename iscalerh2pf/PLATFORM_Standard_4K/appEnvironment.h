#ifndef _PALENVIRONMENT_H_
#define _PALENVIRONMENT_H_

#ifdef __cplusplus
extern "C" {
#endif


#include "utilStorageCfg.h"
#include "Common.h"
#include "utilCommon.h"
#include "halFrontEndCtrlAPI.h"

#define ENVIRONMENT_PERIOD       10//1000           /* 2000 milliseconds polling period */  //A70LV_Doulas_0152 modify

#define ENVIRONMENT_ADC_PERIOD     150
#define ENVIRONMENT_LIGHTSOURCE_BLANKING_MONITOR_PERIOD 150

#define AUTO_SOURCE_DET_SRC_GO_NEXT_TIME_NORMAL     15 //sec
#define AUTO_SOURCE_DET_SRC_GO_NEXT_TIME_ENHANCE    20 //sec
#define AUTO_SOURCE_NO_SRC_GO_NEXT_TIME     1

#define FAST_PWR_ON_STANDBY             1
#define FAST_PWR_ON_ACTIVE              1
#define FAST_PWR_ON_COOLING_START       19
#define FAST_PWR_ON_COOLING_END         2
#define FAST_PWR_ON_WARMING_START       2
#define FAST_PWR_ON_WARMING_END         30

#ifdef DATAPATH_ICHIPS
#define BK_FIRST_INPUT_READY   (0x01) //bit0
#define BK_SECOND_INPUT_READY  (0x02) //bit1
#define BK_ALL_INPUT_READY     (0x03) //bit0 | bit1
#endif

typedef enum
{
    eSYSTEM_MCU_STATE_IDLE,
    eSYSTEM_MCU_STATE_STANDBY,
    eSYSTEM_MCU_STATE_COMMUNICATION,
    eSYSTEM_MCU_STATE_WARMUP,
    eSYSTEM_MCU_STATE_POWERON,
    eSYSTEM_MCU_STATE_COOLING,
    eSYSTEM_MCU_STATE_POWERDOWN,
    eSYSTEM_MCU_STATE_UPGRADE,
    eSYSTEM_MCU_STATE_UPGRADE_ERROR,
    eSYSTEM_MCU_STATE_UPGRADE_DONE,
    eSYSTEM_MCU_STATE_UPGRADE_DDP,

    eSYSTEM_STATE_NUMBERS,
}eSYSTEM_MCU_STATE; 


typedef struct
{

#if (SYSTEM_OS_TYPE == POSIX_COMPLIANT)
    pthread_mutex_t              xMutex ;
#else  //freetros
    SemaphoreHandle_t            xSemaphore;
#if(CURRENT_RTOS_TYPE == RTOS_STATIC)
    StaticSemaphore_t            xMutexBuffer;
#endif
#endif

    BOOL                         bEnableEnvironmentPoll;
    BOOL                         bCheckFanLock;
    BOOL                         bCheckOverTemp;
    BOOL                         bInstantPowerOff;

    UINT8                        ucCurrentCeilingMount;
    UINT8                        ucCeilingMountCheck;
    INT16                        iWarmupTimer; //A70LV_Larry_0135
    INT16                        iWaitWarmupTimer;
    INT16                        iCoolTimer;
    INT8                         iSecurityPowerDownTimer;	//ZU860_Clare_0022,
    INT32                        ilRunTimer; //A70LV_Larry_0023
    UINT32                       uiSleepTimer;   //A70LV_Larry_0082
    UINT32                       uiAutoShutDown; //A70LV_Larry_0082
    UINT32                       uiAutoShutDownDefault;
    UINT8                        ucSystemCount;  //A70LV_Larry_0082
    UINT32                       ulSystemTimer;  //A70LV_Larry_0082
    UINT8                        ucAutoSourceSearchTimer;   //A70LV_Doulas_0082
    UINT8                        ucAutoSourceWaitTimeToChange;
    UINT8                        ucLensID; //A70LV_Larry_0188
    UINT8                        cNetworkWaitMessage; //A70LV_Larry_0278
    UINT8                        cNetworkReadyCount; //A70LV_Larry_0411
    UINT8                        cNetworkStatusMessageCount;

    UINT8                        ucMainSource;
    UINT8                        ucSubSource;
    UINT8                        ucLost_12V;
    UINT16                       uiFailureList;  //A70LV_Larry_0357
    UINT8                        ucFailureCount; //A70LV_Larry_0357
    UINT8                        ucFotaSystemUpdateTimer;	//ZU860_Julie_0003
    UINT8                        ucMonitorFlag;
    BOOL                         bSourceSetting;

    UINT16                       uiOSD_Unlock_Timer;	//G100_Clare_0022, add
    UINT32                       ulLensMemorySavingCondition;   //G100_Simon_0048
    UINT16                       uiFast_Power_On_Timer;	//G100_Clare_0055
    BOOL                         bFake_Power_Down;	//G100_Clare_0055
    UINT8                        ucVersionCheck;    //G100_Simon_0056
    UINT8                        ucFastPowerOnProcessTimer; //G100_Tim_0008, add

    UINT32                       iProServicePowerDownTimer; //A65_OPTOMA_Jerry_0004
    UINT32                       iProServicePowerDownCount; //A65_OPTOMA_Jerry_0004
    UINT8                        iReplaceModelPowerDownTimer; //A65_OPTOMA_Julie_0061
    UINT8                        ucOPDRunTimeFlag;
    UINT16                       cLensCalibrationTimer; //A70LV_Larry_0129
    UINT8                        ucLensChanged; //HICC2_Julie_0070//H30K_Julie_0005

    PsSYSTEM_CONFIGURATION       psSysConfiguration;

}sPAL_ENVIRONMENT_INFORMATION, *PsPAL_ENVIRONMENT_INFORMATION;

eEXEC_CODE palEnvironment_InstantCooling(void);
void palEnvironment_StartCoolTimer(void);
INT16 palEnvironment_Poll(UINT16 uiTick);
eEXEC_CODE palEnvironment_PowerNormal(void);
eEXEC_CODE palEnvironment_PowerStandby(void);
eEXEC_CODE palEnvironment_Init(const PsSYSTEM_CONFIGURATION psSysConfiguration);
void palEnvironment_NetworkState(UINT8 ucData); //A70LV_Larry_0135
UINT8 palEnvironment_NetworkReady(void); //A70LV_Larry_0294
void palEnvironment_NetworkIsReceivePowerOnCmd_Set(UINT8 ucData); //A70LV_Larry_0135
UINT8 palEnvironment_NetworkIsReceivePowerOnCmd_Get(void); //A70LV_Larry_0135
void palEnvironment_SecurityPowerDownTimerSet(UINT32 ulValue, BOOL UpdateToDatabase);	//ZU860_Clare_0022,
UINT32 palEnvironment_SecurityPowerDownTimerGet(void);	//ZU860_Clare_0022,
UINT32 palEnvironment_AutoShutDownTimerGet(void);	//ZU860_Clare_0144
void palEnvironment_AutoShutDownTimerSet(UINT32 Value, BOOL UpdateToDatabase);
void palEnvironment_AutoShutDownDefaultSet(UINT32 Value);
void palEnvironment_SleepTimerSet(UINT8 ucSleepTimerStep, BOOL UpdateToDatabase); //A70LV_Larry_0130
void palEnvironment_SleepTimerClear(void);   //A70LV_Larry_0082
void palEnvironment_AutoShutDownClear(void); //A70LV_Larry_0082
void palEnvironment_AutoSourceSearchTimerClear(void);       //A70LV_Doulas_0082
UINT32 palEnvironment_SleepTimerGet(void); //A70LV_Larry_0130
void palEnvironment_NetworkWaitMessage(UINT8 ucTimer); //A70LV_Larry_0278
void palEnvironment_NetworkStatusMessage(UINT8 ucTimer); //HICC2_Julie_0064
void palEnvironment_LenCalibrationTimeOutSet(void);
void palEnvironment_ADC_CalibrationEnableSet(void);        //A70LV_Doulas_0124
void palEnvironment_ADC_CalibrationStop(void);
BOOL palEnvironment_ADC_CalibrationEnableGet(void);
void palEnvironment_ADC_StateProcess(void);
INT16 palEnvironment_ADC_Poll(UINT16 uiTick);
void palEnvironment_ADCCalStatusSet(eNBRESULT state);
eNBRESULT palEnvironment_ADCCalStatusget(void);
eEXEC_CODE palEnvironment_XillinxFPGA_Reset(void);        //A70LV_Doulas_0179
void palEnvironment_3D_SYNC_Monitor(void);             //A70LV_Doulas_0191
void palEnvironment_ReCheckOrientationState(void);
void palEnvironment_LensMemorySavingCondition_Set(UINT16 uiItemCondition);
UINT16 palEnvironment_LensMemorySavingCondition_Get(void);
void palEnvironment_Fast_Power_On_TimerSet(UINT8 ucTimer);
void palEnvironment_Fast_Power_On_TimerClear(void);	//G100_Clare_0055
void palEnvironment_Fake_Power_Down_Set(BOOL bFakePD);
BOOL palEnvironment_Fake_Power_Down_Get(void);
void palEnvironment_Xillinx_FPGA_Lock_Monitor(void);   //A70LV_Doulas_0216
void palEnvironment_LightSourceBlankingOn_Set(BOOL bBlnakingOn, UINT8 ucTimer);   //A70LV_Doulas_0294
void palEnvironment_LightSourceBlankingEn_Set(BOOL bBlnakingEn);                //A70LV_Doulas_0294
void palEnvironment_LightSourceBlanking_Monitor(void);                          //A70LV_Doulas_0294
BOOL palEnvironment_LightSourceBlankingEn_Get(void);     //A70LV_Doulas_0300
BYTE palEnvironment_FEOPD_Cnt_Get(void);
void palEnvironment_FEOPD_Cnt_Set(BYTE cCnt);
void palEnvironment_LensCalFlag_Update(void);
UINT8 palEnvironment_LensCalFlag_Get(void);
void palEnvironment_LensCalFlag_Set(BYTE cValue);
UINT8 palEnvironment_LensCalFlag_Get(void);
void palEnvironment_LensCalFlag_Set(BYTE cValue);
UINT8 palEnvironment_LensSpecialFlag_Get(void);
void palEnvironment_LensSpecialFlag_Set(BYTE cValue);
UINT8 palEnvironment_Auto_HDMI_Switch_Source_Changing_Get(void);
void palEnvironment_Auto_HDMI_Switch_Source_Changing_Set(UINT8 ucValue);
void palEnvironment_Auto_HDMI_Switch_Source_Monitor(void);
UINT32 palEnvironment_ProServicePowerDownMessageCountGet(void); //A65_OPTOMA_Jerry_0004
void palEnvironment_ProServicePowerDownMessageCountSet(UINT32 ucValue);//A65_OPTOMA_Jerry_0004
UINT32 palEnvironment_ProServicePowerDownTimerGet(void); //A65_OPTOMA_Jerry_0004
void palEnvironment_ProServicePowerDownTimerSet(UINT32 ucValue); //A65_OPTOMA_Jerry_0004

void palEnvironment_ReplaceModel_TimerSet(void); //A65_OPTOMA_Julie_0061
UINT32 palEnvironment_SystemTimer_Get(void); //A65_OPTOMA_Julie_0101
void palEnvironment_OSD_Lock_Timer_Set(UINT16 ucOSD_Unlock_Time);	//G100_Clare_0022, add
UINT16 palEnvironment_OSD_Lock_Timer_Get(void); //G100_Clare_0022, add
void palEnvironment_LensModelCheck_Set(BOOL bModelCheck);	//G100_Clare_0059, add
BOOL palEnvironment_LensModelCheck_Get(void);	//G100_Clare_0059, add

#ifdef NO_POWER_OFF_DURING_CAMERA_WORKING                   //G100_Tim_0046, add, start
void palEnvironment_Camera_Force_Update_Power_Status_Lunch(void);
void palEnvironment_Camera_Force_Update_Power_Status_Check(void);
void palEnvironment_Camera_Force_Update_Power_Status_Set(void);
#endif //NO_POWER_OFF_DURING_CAMERA_WORKING                 //G100_Tim_0046, add, end

INT16 pal_ScalerBlankingLD_Poll(UINT16 uiTick);
void palEnvironment_LENS_EEPROM_Version_Get(UINT8* pcData);  //HICC2_Steven_0027
void palEnvironment_DMD_Airtight_Get(void); //HICC2_Steven_0032
void palEnvironment_LenCalibrationTimerReset(void);
void appEnvironment_NotificationSystemUpdate(void);
BOOL palEnvironment_Send_PIN_StatusGet(void);
void palEnvironment_Send_PIN_StatusSet(BOOL bValue);

#ifdef __cplusplus
}
#endif






#endif  //_PALENVIRONMENT_H_
