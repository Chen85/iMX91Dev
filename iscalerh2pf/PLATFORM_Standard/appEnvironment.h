#ifndef _PALENVIRONMENT_H_
#define _PALENVIRONMENT_H_

#ifdef __cplusplus
extern "C" {
#endif


#include "utilStorageCfg.h"
#include "Common.h"
#include "utilCommon.h"

#define ENVIRONMENT_PERIOD       980//1000           /* 2000 milliseconds polling period */  //A70LV_Doulas_0152 modify

#define ENVIRONMENT_ADC_PERIOD     150          //A70LV_Doulas_0124
#define ENVIRONMENT_LIGHTSOURCE_BLANKING_MONITOR_PERIOD 150

#define AUTO_SOURCE_DET_SRC_GO_NEXT_TIME_NORMAL     15 //sec //HICC2_Steven_0011
#define AUTO_SOURCE_DET_SRC_GO_NEXT_TIME_ENHANCE    20 //sec //HICC2_Steven_0011
#define AUTO_SOURCE_NO_SRC_GO_NEXT_TIME     1      //A35G2_Simon_0039
#define AUTO_SOURCE_KEEP_SOURCE_TIME        2      //A35G2_Simon_0039

#define ENVIRONMENT_MOTOR_PERIOD    200         //G100_Owen_0043

#define FAST_PWR_ON_STANDBY             1                   //G100_Tim_0007, add
#define FAST_PWR_ON_ACTIVE              1                   //G100_Tim_0007, add
//G100_Tim_0008, add, start
#define FAST_PWR_ON_COOLING_START       19
#define FAST_PWR_ON_COOLING_END         2
#define FAST_PWR_ON_WARMING_START       2
#define FAST_PWR_ON_WARMING_END         30
//G100_Tim_0008, add, end

//#define BK_MAIN_CHANNEL_READY   (0x01) //bit0
//#define BK_SUB_CHANNEL_READY    (0x02) //bit1
//#define BK_ALL_CHANNEL_READY    (0x03) //bit0 | bit1

#define BK_FIRST_INPUT_READY   (0x01) //bit0
#define BK_SECOND_INPUT_READY  (0x02) //bit1
#define BK_ALL_INPUT_READY     (0x03) //bit0 | bit1

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
    UINT8                        ucAutoSourceCounter;   //A70LV_Doulas_0082
    UINT8                        ucAutoSourceWaitTimeToChange;   //A35G2_Simon_0039
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
    UINT8                        cLensA16CalibrationState;	//ZU860_Julie_0007
    UINT8                        ucBackupRestoreTimer;		//G100_Doulas_0002

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

    PsSYSTEM_CONFIGURATION       psSysConfiguration;

}sPAL_ENVIRONMENT_INFORMATION, *PsPAL_ENVIRONMENT_INFORMATION;


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
void palEnvironment_ADC_CalibrationEnableSet(void);        //A70LV_Doulas_0124
void palEnvironment_ADC_CalibrationStop(void);
BOOL palEnvironment_ADC_CalibrationEnableGet(void);
void palEnvironment_ADC_StateProcess(void);
INT16 palEnvironment_ADC_Poll(UINT16 uiTick);
void palEnvironment_ADCCalStatusSet(eNBRESULT state);
eNBRESULT palEnvironment_ADCCalStatusget(void);
eEXEC_CODE palEnvironment_XillinxFPGA_Reset(void);        //A70LV_Doulas_0179
void palEnvironment_3D_SYNC_Monitor(void);             //A70LV_Doulas_0191
void palEnvironment_Xillinx_FPGA_Lock_Monitor(void);   //A70LV_Doulas_0216
void palEnvironment_NotificationSystemUpdate(void); 	//ZU860_Julie_0003 //A35G2_Coda_0061
void palEnvironment_LightSourceBlankingOn_Set(BOOL bBlnakingOn, UINT8 ucTimer);   //A70LV_Doulas_0294
void palEnvironment_LightSourceBlankingEn_Set(BOOL bBlnakingEn);                //A70LV_Doulas_0294
void palEnvironment_LightSourceBlanking_Monitor(void);                          //A70LV_Doulas_0294
BOOL palEnvironment_LightSourceBlankingEn_Get(void);     //A70LV_Doulas_0300
void palEnvironment_BackupRestoreTimerSet(UINT8 ucTimer); 	//G100_Doulas_0002
UINT8 palEnvironment_BackupRestoreTimerGet(void);			//G100_Doulas_0002

eEXEC_CODE palEnvironment_InstantCooling(void);

void palEnvironment_OSD_Lock_Timer_Set(UINT16 ucOSD_Unlock_Time);	//G100_Clare_0022, add
UINT16 palEnvironment_OSD_Lock_Timer_Get(void);	//G100_Clare_0022, add
INT16 palEnvironment_Motor_Poll(UINT16 uiTick);
void palEnvironment_BKInput_Check(void); //G100_Steven_0028
UINT8 palEnvironment_BKInput_Ready_Get(void); //G100_Steven_0028
void palEnvironment_ReCheckOrientationState(void);  //G100_Simon_0019
void palEnvironment_LensMemorySavingCondition_Set(UINT16 uiItemCondition);
UINT16 palEnvironment_LensMemorySavingCondition_Get(void);
void palEnvironment_Fake_Power_Down_Set(BOOL bFakePD);	//G100_Clare_0055
BOOL palEnvironment_Fake_Power_Down_Get(void);	//G100_Clare_0055
void palEnvironment_Fast_Power_On_TimerSet(UINT8 ucTimer);	//G100_Clare_0055
void palEnvironment_Fast_Power_On_TimerClear(void); //A35G2_OTM_Owen_0001
BYTE palEnvironment_BKInput_FPGA_SW_Get(void); //G100_Steven_0055
#ifdef CUSTOM_OPTOMA
void palEnvironment_LensModelCheck_Set(BOOL bModelCheck);	//G100_Clare_0059, add
BOOL palEnvironment_LensModelCheck_Get(void);	//G100_Clare_0059, add
#endif
BYTE palEnvironment_FEOPD_Cnt_Get(void); //G100_Steven_0059
void palEnvironment_FEOPD_Cnt_Set(BYTE cCnt); //G100_Steven_0059
void palEnvironment_LensCalFlag_Update(void);   //G100_Owen_0076
UINT8 palEnvironment_LensCalFlag_Get(void); //G100_Owen_0076
void palEnvironment_LensCalFlag_Set(BYTE cValue);
UINT8 palEnvironment_LensCalFlag_Get(void);
void palEnvironment_LensCalFlag_Set(BYTE cValue);
UINT8 palEnvironment_LensSpecialFlag_Get(void);
void palEnvironment_LensSpecialFlag_Set(BYTE cValue);
void palEnvironment_Fast_Power_On_to_LAN_Standby_Start(void);                   //G100_Tim_0008, mod //G100_Tim_0007, add
void palEnvironment_Fast_Power_On_to_LAN_Active_Start(void);                    //G100_Tim_0008, mod //G100_Tim_0007, add
void palEnvironment_Fast_Power_On_to_LAN_Standby_Processing(void);              //G100_Tim_0008, add
void palEnvironment_Fast_Power_On_to_LAN_Active_Processing(void);               //G100_Tim_0008, add
//G100_Tim_0057, add, start //A35G2_BRC_Casper_0060
UINT8 palEnvironment_Auto_HDMI_Switch_Source_Changing_Get(void);
void  palEnvironment_Auto_HDMI_Switch_Source_Changing_Set( UINT8 ucValue );
void  palEnvironment_Auto_HDMI_Switch_Source_Monitor(void);
//G100_Tim_0057, add, end
UINT32 palEnvironment_ProServicePowerDownMessageCountGet(void); //A65_OPTOMA_Jerry_0004
void palEnvironment_ProServicePowerDownMessageCountSet(UINT32 ucValue);//A65_OPTOMA_Jerry_0004
UINT32 palEnvironment_ProServicePowerDownTimerGet(void); //A65_OPTOMA_Jerry_0004
void palEnvironment_ProServicePowerDownTimerSet(UINT32 ucValue); //A65_OPTOMA_Jerry_0004
void palEnvironment_ReplaceModel_TimerSet(void); //A65_OPTOMA_Julie_0061

#ifdef NO_POWER_OFF_DURING_CAMERA_WORKING                   //G100_Tim_0046, add, start
void palEnvironment_Camera_Force_Update_Power_Status_Lunch(void);
void palEnvironment_Camera_Force_Update_Power_Status_Check(void);
void palEnvironment_Camera_Force_Update_Power_Status_Set(void);
#endif //NO_POWER_OFF_DURING_CAMERA_WORKING                 //G100_Tim_0046, add, end

void palEnvironment_A16State_LenCalibrationSet(void); //A35G2_BRC_Casper_0088
UINT8 palEnvironment_A16State_LenCalibrationGet(void); //A35G2_BRC_Casper_0088
void palEnvironment_LenCalibrationTimeOutSet(void);

void palEnvironment_SetLensID(UINT8 ucLensID);  //A65_OPTOMA_CL_0012
INT16 pal_ScalerBlankingLD_Poll(UINT16 uiTick); //HICC2_Doulas_0023
void palEnvironment_LENS_EEPROM_Version_Get(UINT8* pcData);  //HICC2_Steven_0055

#ifdef __cplusplus
}
#endif






#endif  //_PALENVIRONMENT_H_
