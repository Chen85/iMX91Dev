#ifndef HALLDCTRLAPI_H
#define HALLDCTRLAPI_H

// ===============================================================================
// FILE NAME: halLDCtrlAPI.h
// DESCRIPTION:
//
//
// Modification History
// --------------------
// 2017/05/26, Casper Create
// --------------------
// ===============================================================================


//#include "CommonDef.h"
#include "dvLDDriver.h"

typedef enum                                                //LPC54113_JS_0007 add start
{
    /*01*/eFAILURE_FAN_LOCK = 0,
    /*02*/eFAILURE_PUMP_LOCK,
    /*03*/eFAILURE_TEC_NOT_WORK,
    /*04*/eFAILURE_DW_NOT_WORK,
    /*05*/eFAILURE_PW_NOT_WORK,
    /*06*/eFAILURE_I2C_NOT_WORK,
    /*07*/eFAILURE_LD_OVER_TEMP,
    /*08*/eFAILURE_OVERTEMPERATURE,
    /*09*/eFAILURE_SYS_OVER_TEMP,                                 //A70LV_John_0025 sync LDDRV error items (no function now)//LDDRV_JS_0079 modify
    /*10*/eFAILURE_FAN_STALL,
    /*11*/eFAILURE_TEC_OVER_TEMP,                                 //A70LV_John_0025 sync LDDRV error items (no function now)//LDDRV_JS_0079 add
    /*12*/eFAILURE_50V_NG,
    /*13*/eFAILURE_RLD_OCP,                                       //LDDRV_JS_0128 add
    /*14*/eFAILURE_BLD2_OCP,                                      //LDDRV_JS_0128 add
    /*15*/eFAILURE_BLD1_OCP,                                      //LDDRV_JS_0128 add
    /*16*/eFAILURE_PUMP_STALL,

    eFAILURE_NUMBERS,
} eFAILURE_LIST; //A70LV_Larry_0023

#pragma pack(push)  /* push current alignment to stack */
#pragma pack(1)     /* set alignment to 1 byte boundary */

typedef struct
{
    UINT8   ucGroup;
    UINT16  uiFailure;
    UINT8   ucLD_OverTemperature[3];
    UINT8   ucLD_OverTempCount;
}sFAILURE; //A70LV_Larry_0023

#pragma pack(pop)   /* restore original alignment from stack */

//#define LD_ENABLEBLOCK1MASK         0x01
//#define LD_ENABLEBLOCK2MASK         0x02
//#define LD_ENABLEBLOCK3MASK         0x04
//#define LD_ENABLEBLOCK4MASK         0x08
//#define LD_ENABLEBLOCK5MASK         0x10

#if 1
typedef enum
{
    eBLD_1,
    eBLD_2,
    eBLD_3,
    eBLD_4,
    eBLD_5,
    eBLD_6,
    eBLD_7,
    eBLD_8,
    eBLD_9,
    eBLD_10,
    eRLD_1,
    eRLD_2,
    eRLD_3,
    eRLD_4,

    eLD_BANK_NUMBERS,
} eLD_BANK_ITEM;
#else
typedef enum
{
    eLD_BANK_ENALE_01 = 0,
    eLD_BANK_ENALE_02,
    eLD_BANK_ENALE_03,
    eLD_BANK_ENALE_04,
    eLD_BANK_ENALE_05,
    eLD_BANK_ENALE_06,
    eLD_BANK_ENALE_07,
    eLD_BANK_ENALE_08,
    eLD_BANK_ENALE_09,
    eLD_BANK_ENALE_10,
    eLD_BANK_ENALE_11,
    eLD_BANK_ENALE_12,

    eLD_BANK_ENALE_NUMBERS,
} eLD_BANK_ENALE_ITEM;
#endif



typedef enum
{
    eTEC_P1,
    eTEC_P2,

    eTEC_ALL,
} eTEC_LIST;

typedef enum
{
    eLC_1,
    eLC_2,

    eLC_NUMBERS,
} eLC_ITEM;

typedef enum   //ZU860_John_0026 fix temperature reading
{
    eTEMPERATURE_DMD,
    eTEMPERATURE_1,
    eTEMPERATURE_2,
    eTEMPERATURE_LVPS,
    eTEMPERATURE_SYSTEM,
    eTEMPERATURE_5,

    eTEMPERATURE_NUMBERS,
} eTEMPERATURE_ITEM;

typedef enum
{
    eABP_STATE_IDLE,

    eABP_STATE_START,
    eABP_STATE_LDRELOAD,
    eABP_STATE_AUTOTUNNING,

    eABP_STATE_NUMBER,

}eABP_STATE; //A70LV_Larry_0179

typedef enum
{
    eFAN_PWRMODE_ECO,
    eFAN_PWRMODE_FULL,
    eFAN_PWRMODE_SUPER,

    eFAN_PWRMODE_NUMBERS,
} eFAN_PWRMODE;	//G100_Clare_0011, add

typedef enum
{
    eLSP_STATE_IDLE,
    eLSP_STATE_CALIB,
    eLSP_STATE_SAMPLE,

    eLSP_STATE_NUMBER,
} eLSP_STATE;		//A65_OPTOMA_Doulas_0106


void halLDCtrl_LD_Enable_Default(void);
eRESULT halLDCtrl_Version_Get(UINT8* pcVersion);
eRESULT halLDCtrl_ConvertVersion_Get(UINT8* pcVersion);
eRESULT halLDCtrl_LD_Voltage_Get(eLD_ID eDriverID, UINT8* pcData);
eRESULT halLDCtrl_LD_Current_Get(eLD_ID eDriverID, UINT8* pcData);
eRESULT halLDCtrl_LD_Temperature_Get(eLD_ID eDriverID, UINT8* pcData);
eRESULT halLDCtrl_LD_SourceEnable_Get(eLD_ID eDriverID, UINT8* pcData);
eRESULT halLDCtrl_LD_SourceEnable_Set(eLD_ID eDriverID, UINT8 ucData);
eRESULT halLDCtrl_LD_PWM_Get(eLD_ID eDriverID, UINT8* pcData);
eRESULT halLDCtrl_LD_PWM_Set(eLD_ID eDriverID, UINT8 ucChannel, UINT16 uiPWM);
eRESULT halLDCtrl_Fan_Mode_Get(eLD_ID eDriverID, UINT8* pcData);
eRESULT halLDCtrl_Fan_RPM_Get(eLD_ID eDriverID, UINT8* pcData);
eRESULT halLDCtrl_Tec_Enable_Get(eLD_ID eDriverID, UINT8* pcData);
eRESULT halLDCtrl_Tec_Enable_Set(eLD_ID eDriverID, UINT8 ucTecID, UINT8 ucEnable);
eRESULT halLDCtrl_Tec_Duty_Get(eLD_ID eDriverID, UINT8* pcData);
eRESULT halLDCtrl_Tec_Current_Get(eLD_ID eDriverID, UINT8* pcData);
eRESULT halLDCtrl_Tec_Temperature_Get(eLD_ID eDriverID, UINT8* pcData);
eRESULT halLDCtrl_Tec_Mode_Get(eLD_ID eDriverID, UINT8* pcData);
eRESULT halLDCtrl_Power_Status_Get(eLD_ID eDriverID, UINT8* pcData);
eRESULT halLDCtrl_Power_Status_Set(eLD_ID eDriverID, UINT8 ucPowerMode);
eRESULT halLDCtrl_Failure_Status_Get(eLD_ID eDriverID, UINT8* pcData);
eRESULT halLDCtrl_High_Altitude_Get(eLD_ID eDriverID, UINT8* pcData);
eRESULT halLDCtrl_Wheer_Speed_Get(eLD_ID eDriverID, UINT8* pcData);
eRESULT halLDCtrl_Orientation_Get(eLD_ID eDriverID, UINT8* pcData);
eRESULT halLDCtrl_Light_Sensor_Get(eLD_ID eDriverID, UINT8* pcData);
eRESULT halLDCtrl_LD_Power_Get(eLD_ID eDriverID, UINT8* pcData);
eRESULT halLDCtrl_LD_Power_Set(eLD_ID eDriverID, UINT8* pucLDPower);
eRESULT halLDCtrl_LD_PowerAll_Set(eLD_ID eDriverID, UINT8* pucLDPower);
//eRESULT halLDCtrl_LD_Power_Get(eLD_ID eDriverID, UINT8* pcData);	//G100_Clare_0033, marked
eRESULT halLDCtrl_Light_Module_Status_Get(eLD_ID eDriverID, UINT8* pcData);
eRESULT halLDCtrl_Light_Module_Status_Set(eLD_ID eDriverID, UINT8 ucLightEnable);
UINT32 halLDCtrl_Duration_of_Lighting_Get(void);
void halLDCtrl_Duration_of_Lighting_Set(UINT32 uiSec);
eRESULT halLDCtrl_Light_Sensor_Get(eLD_ID eDriverID, UINT8* pcData);
eRESULT halLDCtrl_CeilingMount_Get(eLD_ID eDriverID, UINT8* pcData);    //A70LV_Doulas_0062
eRESULT halLDCtrl_Bank_Enable_Set(eLD_BANK_ITEM eBank, BOOL bEnable);
BOOL halLDCtrl_Bank_Enable_Get(eLD_BANK_ITEM eBank);
eRESULT halLDCtrl_BLD_Bank_Enable(BOOL bBEn);
eRESULT halLDCtrl_RLD_Bank_Enable(BOOL bREn);
eRESULT halLDCtrl_LD_GroupEnable(UINT8 ucGroup, BOOL bEnable);
eRESULT halLDCtrl_LD_Voltage_All_Get(eLD_ID eDriverID, UINT8* pcData); //A70LV_Larry_0144
eRESULT halLDCtrl_LD_Current_All_Get(eLD_ID eDriverID, UINT8* pcData); //A70LV_Larry_0144
eRESULT halLDCtrl_LD_Temperature_All_Get(UINT8* pcData);
eRESULT halLDCtrl_Tec_Voltage_All_Get(eLD_ID eDriverID, UINT8* pcData); //A70LV_Larry_0163
eRESULT halLDCtrl_Tec_Current_All_Get(eLD_ID eDriverID, UINT8* pcData); //A70LV_Larry_0144
eRESULT halLDCtrl_Tec_Temperature_All_Get(eLD_ID eDriverID, UINT8* pcData); //A70LV_Larry_0144
eRESULT halLDCtrl_ABP_ModeSet(UINT8 cData); //A70LV_Larry_0179
eRESULT halLDCtrl_TargetBLDLightSet(UINT8 *pcData); //A70LV_Larry_0179
eRESULT halLDCtrl_TargetBLDPWMSet(UINT8 *pcData); //A70LV_Larry_0179
eRESULT halLDCtrl_TargetRLDLightSet(UINT8 *pcData); //A70LV_Larry_0179
eRESULT halLDCtrl_TargetRLDPWMSet(UINT8 *pcData); //A70LV_Larry_0179
eRESULT halLDCtrl_ABP_AutoTuning_Get(UINT8* pcData); //A70LV_Larry_0181
eRESULT halLDCtrl_DisableShutdown(void); //A70LV_Larry_0199
eRESULT halLDCtrl_LD_Count_Get(UINT8* pcData);
eRESULT halLDCtrl_G_Sensor_Data_Get(eLD_ID eDriverID, UINT8* pcData);
eRESULT halLDCtrl_LD_SourceBlanking_Get(eLD_ID eDriverID, UINT8* pcData);       //A70LV_Doulas_0294
eRESULT halLDCtrl_LD_SourceBlanking_Set(eLD_ID eDriverID, UINT8 ucData);        //A70LV_Doulas_0294
eRESULT halLDCtrl_LD_WheelBlanking_Get(eLD_ID eDriverID, UINT8* pcData);        //A70LV_Doulas_0306
eRESULT halLDCtrl_LD_WheelBlanking_Set(eLD_ID eDriverID, UINT8 ucData);   //A70LV_Doulas_0306
eRESULT halLDCtrl_First_LD_On_Set(eLD_ID eDriverID, UINT8 ucData);
eRESULT halLDCtrl_LS_RLD_Enable_Set(UINT8 cData);		//G100_Doulas_0010
eRESULT halLDCtrl_LS_RLD_Enable_Get(UINT8 *pcData);    //G100_Owen_0094
eRESULT halLDCtrl_LightSensorT1_Set(UINT8* pcData);									//G100_Doulas_0012
eRESULT halLDCtrl_LightSensorT0_Set(UINT8* pcData);									//G100_Doulas_0012
eRESULT halLDCtrl_LightSensorT0_Get(UINT8* pcData);									//G100_Doulas_0012
eRESULT halLDCtrl_LightSensorTrigger_Set(UINT8 cData);							//G100_Doulas_0012
eRESULT halLDCtrl_LightSensorTrigger_Get(UINT8 *pcData);		//A65_OPTOMA_Doulas_0106 get LS ADC statue
eRESULT halLDCtrl_T1LightSensorValue_Get(eLD_ID eDriverID, UINT8* pcData);		//G100_Doulas_0012
eRESULT halLDCtrl_T0LightSensorValue_Get(eLD_ID eDriverID, UINT8* pcData);		//G100_Doulas_0012
eRESULT halLDCtrl_LS_TargetValue_Set(UINT8 *pcData);    //G100_Owen_0087
eRESULT halLDCtrl_LightSensorOffset_Get(UINT8 *pcData);		//G100_Doulas_0012  //G100_Owen_0138
eRESULT halLDCtrl_LightSensorOffset_Set(UINT8 *pcData);  //G100_Owen_0138
eRESULT halLDCtrl_LightSensorGain_Get(UINT8* pcData);    //G100_Owen_0128
eRESULT halLDCtrl_Fan_Mode_Set();	//G100_Clare_0011
eRESULT halLDCtrl_OPD_Register_Set(UINT8 reg, UINT8 size, UINT8 *pcData);
eRESULT halLDCtrl_OPD_Register_Get(UINT8 reg, UINT8 size, UINT8* pcData);
eRESULT halLDCtrl_ProjectorID_Set(UINT8 cData);
eRESULT halLDCtrl_CustomerID_Set(UINT8 cData);
eRESULT halLDCtrl_PlatformID_Set(UINT8 cData);
eRESULT halLDCtrl_Tec_JIG_Data_Get(eLD_ID eDriverID, UINT8* pcData); //A70Gen2_Julie_0036//A70LV_John_0158 add TEC JIG communication commands
eRESULT halLDCtrl_EnvironmentDimPower_Get(UINT8* pcData);
eRESULT halLDCtrl_LightSensorPosition_Set(UINT8 pcData); //HICC2_Jacky_0001


#endif /* HALLDCTRLAPI_H */


