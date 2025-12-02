#ifndef _PALLDMGR_H
#define _PALLDMGR_H

// ===============================================================================
// FILE NAME: palLDMgrAPI.h
// DESCRIPTION:
//
//
// Modification History
// --------------------
// 2017/05/26, Casper Create
// --------------------
// ===============================================================================


#include "Common.h"
#include "halLDProc.h"

typedef enum                                                //LPC54113_JS_0007 add start
{
    /*01*/ePAL_FAILURE_FAN_LOCK = 0,
    /*02*/ePAL_FAILURE_PUMP_LOCK,
    /*03*/ePAL_FAILURE_TEC_NOT_WORK,
    /*04*/ePAL_FAILURE_DW_NOT_WORK,
    /*05*/ePAL_FAILURE_PW_NOT_WORK,
    /*06*/ePAL_FAILURE_I2C_NOT_WORK,
    /*07*/ePAL_FAILURE_LD_OVER_TEMP,
    /*08*/ePAL_FAILURE_OVERTEMPERATURE,
    /*09*/ePAL_FAILURE_SYS_OVER_TEMP,                                 //A70LV_John_0025 sync LDDRV error items (no function now)//LDDRV_JS_0079 modify
    /*10*/ePAL_FAILURE_FAN_STALL,
    /*11*/ePAL_FAILURE_TEC_OVER_TEMP,                                 //A70LV_John_0025 sync LDDRV error items (no function now)//LDDRV_JS_0079 add
    /*12*/ePAL_FAILURE_50V_NG,
    /*13*/ePAL_FAILURE_RLD_OCP,                                       //LDDRV_JS_0128 add
    /*14*/ePAL_FAILURE_BLD2_OCP,                                      //LDDRV_JS_0128 add
    /*15*/ePAL_FAILURE_BLD1_OCP,                                      //LDDRV_JS_0128 add
    /*16*/ePAL_FAILURE_PUMP_STALL,

    ePAL_FAILURE_NUMBERS,
} ePAL_FAILURE_LIST; //A70LV_Larry_0023

#pragma pack(push)  /* push current alignment to stack */
#pragma pack(1)     /* set alignment to 1 byte boundary */

typedef struct
{
    UINT8   ucGroup;
    UINT16  uiFailure;
    UINT8   ucLD_OverTemperature[3];
    UINT8   ucLD_OverTempCount;
}sPAL_FAILURE; //A70LV_Larry_0023

#pragma pack(pop)   /* restore original alignment from stack */

#define PAL_LD_ENABLEBLOCK1MASK         0x01
#define PAL_LD_ENABLEBLOCK2MASK         0x02
#define PAL_LD_ENABLEBLOCK3MASK         0x04
#define PAL_LD_ENABLEBLOCK4MASK         0x08
//#define LD_ENABLEBLOCK5MASK         0x10


typedef enum
{
    ePAL_LD_BANK_ENALE_01 = 0,
    ePAL_LD_BANK_ENALE_02,
    ePAL_LD_BANK_ENALE_03,
    ePAL_LD_BANK_ENALE_04,
    ePAL_LD_BANK_ENALE_05,
    ePAL_LD_BANK_ENALE_06,
    ePAL_LD_BANK_ENALE_07,
    ePAL_LD_BANK_ENALE_08,
    ePAL_LD_BANK_ENALE_09,
    ePAL_LD_BANK_ENALE_10,
    ePAL_LD_BANK_ENALE_11,
    ePAL_LD_BANK_ENALE_12,
    ePAL_LD_BANK_ENALE_13,
    ePAL_LD_BANK_ENALE_14,

    ePAL_LD_BANK_ENALE_NUMBERS,
} ePAL_LD_BANK_ENALE_ITEM;


typedef enum
{
    ePAL_TEC_P1,
    ePAL_TEC_P2,

    ePAL_TEC_ALL,
} ePAL_TEC_LIST;

typedef enum
{
    ePAL_LC_1,
    ePAL_LC_2,

    ePAL_LC_NUMBERS,
} ePAL_LC_ITEM;

typedef enum   //ZU860_John_0026 fix temperature reading
{
    ePAL_TEMPERATURE_DMD,
    ePAL_TEMPERATURE_1,
    ePAL_TEMPERATURE_2,
    ePAL_TEMPERATURE_LVPS,
    ePAL_TEMPERATURE_SYSTEM,
    ePAL_TEMPERATURE_5,

    ePAL_TEMPERATURE_NUMBERS,
} ePAL_TEMPERATURE_ITEM;

typedef enum
{
    ePAL_LD_SEG_BLD_R,
    ePAL_LD_SEG_BLD_G,
    ePAL_LD_SEG_BLD_B,
    ePAL_LD_SEG_BLD_Y,
    ePAL_LD_SEG_RLD_R,
    ePAL_LD_SEG_RLD_Y,

    ePAL_LD_SEG_NUMBERS,
} ePAL_LD_SEG; //A70LV_Larry_0084

typedef enum
{
    ePAL_ABP_STATE_IDLE,

    ePAL_ABP_STATE_START,
    ePAL_ABP_STATE_LDRELOAD,
    ePAL_ABP_STATE_AUTOTUNNING,

    ePAL_ABP_STATE_NUMBER,

}ePAL_ABP_STATE; //A70LV_Larry_0179

typedef enum
{
    ePAL_FAN_PWRMODE_ECO,
    ePAL_FAN_PWRMODE_FULL,
    ePAL_FAN_PWRMODE_SUPER,

    ePAL_FAN_PWRMODE_NUMBERS,
} ePAL_FAN_PWRMODE;	//G100_Clare_0011, add


typedef enum
{
    ePAL_LDBANK_A70LV,

    // ------------------------------------------------------------------------
    ePAL_LD_NUMBERS,    // used for enumerated type range
                        // checking (DO NOT REMOVE)
} ePAL_LD_ID;


typedef enum
{
    ePAL_LD_BANK_R1,
    ePAL_LD_BANK_B1,
    ePAL_LD_BANK_B2,

    ePAL_LD_BANK_NUMBERS,
} ePAL_LD_BANK_ITEM;

eRESULT palLDMgr_Version_Get(UINT8* pcVersion);
eRESULT palLDMgr_LD_Voltage_Get(ePAL_LD_ID eDriverID, UINT8* pcData);
eRESULT palLDMgr_LD_Current_Get(ePAL_LD_ID eDriverID, UINT8* pcData);
eRESULT palLDMgr_LD_Temperature_Get(ePAL_LD_ID eDriverID, UINT8* pcData);
eRESULT palLDMgr_LD_SourceEnable_Get(ePAL_LD_ID eDriverID, UINT8* pcData);
eRESULT palLDMgr_LD_SourceEnable_Set(ePAL_LD_ID eDriverID, UINT8 ucData);
eRESULT palLDMgr_LD_PWM_Get(ePAL_LD_ID eDriverID, UINT8* pcData);
eRESULT palLDMgr_LD_PWM_Set(ePAL_LD_ID eDriverID, UINT8 ucChannel, UINT16 uiPWM);
eRESULT palLDMgr_Fan_Mode_Get(ePAL_LD_ID eDriverID, UINT8* pcData);
eRESULT palLDMgr_Fan_RPM_Get(ePAL_LD_ID eDriverID, UINT8* pcData);
eRESULT palLDMgr_Tec_Enable_Get(ePAL_LD_ID eDriverID, UINT8* pcData);
eRESULT palLDMgr_Tec_Enable_Set(ePAL_LD_ID eDriverID, UINT8 ucTecID, UINT8 ucEnable);
BOOL palLDMgr_BLD_Bank_Enable(BOOL bEnable);
BOOL palLDMgr_RLD_Bank_Enable(BOOL bEnable);
eRESULT palLDMgr_Tec_Duty_Get(ePAL_LD_ID eDriverID, UINT8* pcData);
eRESULT palLDMgr_Tec_Current_Get(ePAL_LD_ID eDriverID, UINT8* pcData);
eRESULT palLDMgr_Tec_Temperature_Get(ePAL_LD_ID eDriverID, UINT8* pcData);
eRESULT palLDMgr_Tec_Mode_Get(ePAL_LD_ID eDriverID, UINT8* pcData);
eRESULT palLDMgr_Power_Status_Get(ePAL_LD_ID eDriverID, UINT8* pcData);
eRESULT palLDMgr_Power_Status_Set(ePAL_LD_ID eDriverID, UINT8 ucPowerMode);
eRESULT palLDMgr_Failure_Status_Get(ePAL_LD_ID eDriverID, UINT8* pcData);
eRESULT palLDMgr_High_Altitude_Get(ePAL_LD_ID eDriverID, UINT8* pcData);
eRESULT palLDMgr_Wheer_Speed_Get(ePAL_LD_ID eDriverID, UINT8* pcData);
eRESULT palLDMgr_Orientation_Get(ePAL_LD_ID eDriverID, UINT8* pcData);
eRESULT palLDMgr_LD_Power_Get(ePAL_LD_ID eDriverID, UINT8* pcData);
eRESULT palLDMgr_LD_Power_Set(ePAL_LD_ID eDriverID, UINT8* pucLDPower);
eRESULT palLDMgr_LD_PowerAll_Set(ePAL_LD_ID eDriverID, UINT8* pucLDPower);
//eRESULT palLDMgr_LD_Power_Get(eLD_ID eDriverID, UINT8* pcData);	//G100_Clare_0033, marked
eRESULT palLDMgr_Light_Module_Status_Get(ePAL_LD_ID eDriverID, UINT8* pcData);
eRESULT palLDMgr_Light_Module_Status_Set(ePAL_LD_ID eDriverID, UINT8 ucLightEnable);
UINT32 palLDMgr_Duration_of_Lighting_Get(void);
void palLDMgr_Duration_of_Lighting_Set(UINT32 uiSec);
eRESULT palLDMgr_Light_Sensor_Get(ePAL_LD_ID eDriverID, UINT8* pcData);
eRESULT palLDMgr_CeilingMount_Get(ePAL_LD_ID eDriverID, UINT8* pcData);    //A70LV_Doulas_0062
eRESULT palLDMgr_Bank_Enable_Set(ePAL_LD_BANK_ENALE_ITEM eBank, BOOL bEnable);
BOOL palLDMgr_Bank_Enable_Get(ePAL_LD_BANK_ENALE_ITEM eBank);
eRESULT palLDMgr_LD_GroupEnable(UINT8 ucGroup, UINT8 ucEnable);
eRESULT palLDMgr_LD_Voltage_All_Get(ePAL_LD_ID eDriverID, UINT8* pcData); //A70LV_Larry_0144
eRESULT palLDMgr_LD_Current_All_Get(ePAL_LD_ID eDriverID, UINT8* pcData); //A70LV_Larry_0144
eRESULT palLDMgr_LD_Temperature_All_Get(UINT8* pcData);
eRESULT palLDMgr_Tec_Voltage_All_Get(ePAL_LD_ID eDriverID, UINT8* pcData); //A70LV_Larry_0163
eRESULT palLDMgr_Tec_Current_All_Get(ePAL_LD_ID eDriverID, UINT8* pcData); //A70LV_Larry_0144
eRESULT palLDMgr_Tec_Temperature_All_Get(ePAL_LD_ID eDriverID, UINT8* pcData); //A70LV_Larry_0144
eRESULT palLDMgr_ABP_ModeSet(UINT8 cData); //A70LV_Larry_0179
eRESULT palLDMgr_TargetBLDLightSet(UINT8 *pcData); //A70LV_Larry_0179
eRESULT palLDMgr_TargetBLDPWMSet(UINT8 *pcData); //A70LV_Larry_0179
eRESULT palLDMgr_TargetRLDLightSet(UINT8 *pcData); //A70LV_Larry_0179
eRESULT palLDMgr_TargetRLDPWMSet(UINT8 *pcData); //A70LV_Larry_0179
eRESULT palLDMgr_ABP_AutoTuning_Get(UINT8* pcData); //A70LV_Larry_0181
eRESULT palLDMgr_DisableShutdown(void); //A70LV_Larry_0199
eRESULT palLDMgr_LD_Count_Get(UINT8* pcData);
eRESULT palLDMgr_LD_SourceBlanking_Get(ePAL_LD_ID eDriverID, UINT8* pcData);       //A70LV_Doulas_0294
eRESULT palLDMgr_LD_SourceBlanking_Set(ePAL_LD_ID eDriverID, UINT8 ucData);        //A70LV_Doulas_0294
eRESULT palLDMgr_LD_WheelBlanking_Get(ePAL_LD_ID eDriverID, UINT8* pcData);        //A70LV_Doulas_0306
eRESULT palLDMgr_LD_WheelBlanking_Set(ePAL_LD_ID eDriverID, UINT8 ucData);   //A70LV_Doulas_0306
eRESULT palLDMgr_First_LD_On_Set(ePAL_LD_ID eDriverID, UINT8 ucData);
eRESULT palLDMgr_LS_RLD_Enable_Set(UINT8 cData);		//G100_Doulas_0010
eRESULT palLDMgr_LightSensorT1_Set(UINT8* pcData);									//G100_Doulas_0012
eRESULT palLDMgr_LightSensorT0_Set(UINT8* pcData);									//G100_Doulas_0012
eRESULT palLDMgr_LightSensorTrigger_Set(UINT8 cData);							//G100_Doulas_0012
eRESULT palLDMgr_T1LightSensorValue_Get(ePAL_LD_ID eDriverID, UINT8* pcData);		//G100_Doulas_0012
eRESULT palLDMgr_T0LightSensorValue_Get(ePAL_LD_ID eDriverID, UINT8* pcData);		//G100_Doulas_0012
eRESULT palLDMgr_Fan_Mode_Set(eCM_POWER_MODE_ID eLightMode, eCM_PICTURE_SETTINGS_ID ePictureSetting, eCM_PICTURE_SETTINGS_ID eUserModePicSetting);	//G100_Clare_0011
eRESULT palLDMgr_OPD_Register_Set(UINT8 reg, UINT8 size, UINT8 *pcData);
eRESULT palLDMgr_OPD_Register_Get(UINT8 reg, UINT8 size, UINT8* pcData);
eRESULT palLDMgr_ProjectorID_Set(UINT8 cData);
eRESULT palLDMgr_CustomerID_Set(UINT8 cData);
eRESULT palLDMgr_PlatformID_Set(UINT8 cData);
eRESULT palLDMgr_LightSensor_Position_Set(UINT8 cData); //HICC2_Jacky_0001


#endif /* _PALLDMGR_H */


