#ifndef _PALILLUMINATION_H_
#define _PALILLUMINATION_H_

#ifdef __cplusplus
extern "C" {
#endif


#include "utilStorageCfg.h"
#include "Common.h"

//#define RLD_TYPE //A35G2_CDS_Casper_0001
#define ILLUMINATION_PERIOD         950//1000           /* 1000 milliseconds polling period */   //A70LV_Doulas_0152 modify

#define LAMP_PERIOD_UPDATE          60             /* 1 minutes, in milliseconds */

//====================== Enum Start ======================//

#define ABC_LIGHT_SENSOR_MAXIMUM (1023L)//(55000L) //550K	//G100_Doulas_0010 Modify
#define ABC_LIGHT_SENSOR_MINIMUM (80L)//(1000L)  //10K	//G100_Doulas_0010 Modify

#define ABC_LIGHT_SENSOR_SAMPLE_TIME_T_MINIMUM		1		//G100_Doulas_0106  //G100_Owen_0138

#define ABC_LIGHT_SENSOR_CAL_BLD_MAXIMUM			980		//G100_Doulas_0106
#define ABC_LIGHT_SENSOR_CAL_BLD_MINIMUM			30//100		//HICC2_Doulas_0007//G100_Doulas_0106  //G100_Owen_0138

#define ABC_LIGHT_SENSOR_CAL_RLD_MAXIMUM			980		//G100_Doulas_0106  //G100_Owen_0138
#define ABC_LIGHT_SENSOR_CAL_RLD_MINIMUM			30//100		//HICC2_Doulas_0007//G100_Doulas_0106


typedef enum
{
    eILLUM_MODE_ECO,
    eILLUM_MODE_BRIGHT,
}eILLUM_MODE;

//====================== Enum End ======================//
typedef enum
{
    eLIGHT_SENSOR_CAL_IDEL,
    eLIGHT_SENSOR_CAL_FULL_START,
    eLIGHT_SENSOR_CAL_BLD1_FULL,
    eLIGHT_SENSOR_CAL_BLD2_FULL,
    eLIGHT_SENSOR_CAL_RLD1_FULL,
    eLIGHT_SENSOR_CAL_ECO_START,
    eLIGHT_SENSOR_CAL_BLD1_ECO,
    eLIGHT_SENSOR_CAL_BLD2_ECO,
    eLIGHT_SENSOR_CAL_RLD1_ECO,
    eLIGHT_SENSOR_CAL_DONE,
    eLIGHT_SENSOR_CAL_SUCCESS,
    eLIGHT_SENSOR_CAL_ERROR,
    eLIGHT_SENSOR_CAL_SAMPLE_TIME_START,		//A65_OPTOMA_Doulas_0106 12
    eLIGHT_SENSOR_CAL_SAMPLE_TIME_WAIT,			//A65_OPTOMA_Doulas_0106
    eLIGHT_SENSOR_CAL_SAMPLE_TIME_T_VALUE,		//A65_OPTOMA_Doulas_0106 14
    eLIGHT_SENSOR_CAL_SAMPLE_TIME_BLD_VALUE,	//A65_OPTOMA_Doulas_0106
	eLIGHT_SENSOR_CAL_SAMPLE_TIME_RLD_VALUE,	//A65_OPTOMA_Doulas_0106
	eLIGHT_SENSOR_CAL_SAMPLE_TIME_DONE,			//A65_OPTOMA_Doulas_0106
	eLIGHT_SENSOR_CAL_SAMPLE_TIME_SUCCESS,		//A65_OPTOMA_Doulas_0106
    eLIGHT_SENSOR_CAL_SAMPLE_TIME_ERROR,		//A65_OPTOMA_Doulas_0106
    eLIGHT_SENSOR_CAL_SAMPLE_TIME_COLLECT_INFO, //HICC2_Jacky_0003
    eLIGHT_SENSOR_CAL_SAMPLE_TIME_WAIT_CHANGE_POS,   //HICC2_Jacky_0003

}eLIGHT_SENSOR_CAL; //T100_Larry_0015

typedef enum
{
    eLIGHT_SENSOR_RLD_CAL_IDEL,
    eLIGHT_SENSOR_RLD_CAL_START,
    eLIGHT_SENSOR_RLD_CAL_FULL,
    eLIGHT_SENSOR_RLD_CAL_ECO_START,	//G100_Doulas_0023 Add
    eLIGHT_SENSOR_RLD_CAL_ECO,
    eLIGHT_SENSOR_RLD_CAL_DONE,

}eLIGHT_SENSOR_RLD_CAL; //T100_Larry_0019

typedef enum
{
    eLIGHT_SENSOR_INTESSITY_IDEL,
    eLIGHT_SENSOR_INTESSITY_START,
    eLIGHT_SENSOR_INTESSITY_BLD,
    eLIGHT_SENSOR_INTESSITY_RLD,
    eLIGHT_SENSOR_INTESSITY_DONE,
	eLIGHT_SENSOR_INTESSITY_ERROR,

}eLIGHT_SENSOR_INTESSITY; //T100_Larry_0019

typedef enum
{
    eABP_STATUS_IDEL,
    eABP_STATUS_FACTORY_CAL,
    eABP_STATUS_DYNAMIC_RLD_CAL,
    eABP_STATUS_CONSTANT_INTENSITY,
    eABP_STATUS_DYNAMIC_RLD_CAL_SHUTDOWN,

}eABP_STATUS; //T100_Larry_0018


typedef enum
{
    eCONSTANT_INTENSITY_IDEL,
    eCONSTANT_INTENSITY_CALCULATE_BLD,
    eCONSTANT_INTENSITY_CALCULATE_RLD,
    eCONSTANT_INTENSITY_SUCCESS,
    eCONSTANT_INTENSITY_ERROR,

}eCONSTANT_INTENSITY; //T100_Larry_0019

typedef enum
{
    eABP_AUTOTUNING_STATE_PASS,
	eABP_AUTOTUNING_STATE_PENDING,
	eABP_AUTOTUNING_STATE_OVER_RANGE,
	eABP_AUTOTUNING_STATE_SATURATED,
	eABP_AUTOTUNING_STATE_STOP,         //H30K_Tim_0008, add
	eABP_AUTOTUNING_STATE_NUMBER,

} eABP_AUTOTUNING_STATE; //T100_Larry_0022

#if defined(CUSTOM_BARCO) || defined(CUSTOM_OPTOMA)//A35G2_OTM_Coda_0002
typedef enum //G100_Steven_0084
{
	TEC_Gating_STEP0,
	TEC_Gating_STEP1,
	TEC_Gating_STEP2,
	TEC_Gating_STEP3,
	TEC_Gating_STEP4,
	TEC_Gating_STEP5,

	TEC_Gating_NUMBER,
} eTEC_GATING_STEP;
#endif


//===================== Struct Start ===================//

typedef struct
{
    UINT32      ulLampPeriod;//Use lamp hour count by Minute
    UINT32      ulStartUpLampPeriod;//Record lamp hour by Minute on every startup
    UINT16      uiLampUpdateCount;
    BOOL        bIllumEnabled;
    BOOL        bFirstLightSourceOn; //A70LV_Larry_0020
    UINT8       ucSmoothOn; //A70LV_Larry_0255
    UINT32      ulTotalProjectorHours;              //A70LV_Doulas_0061 , by Minute
    UINT32      ulTotalProjectorHoursUpdateCount;   //A70LV_Doulas_0061
    UINT8       ucLightSensorPolling; //T100_Larry_0011
    UINT8       ucLightSensorCalStatus; //T100_Larry_0018
    UINT8       ucLightSensorRLDCalStatus; //T100_Larry_0019
    UINT8       ucLightSensorConstantIntensity; //T100_Larry_0019
    UINT8       ucABPStatus; //T100_Larry_0015
    UINT8       ucABPTimer; //T100_Larry_0015
    UINT8       ucLightSourceMode; //T100_Larry_0018
    UINT8       ucRentalModeEnable;
    UINT8       ucABPDelayStart;
    UINT8       ucABPAutoTuningCount;
    UINT8       cLensPlug; //A70LV_Larry_0340
    UINT16      uiABPTargetBLDLight[eLD_SEQ_NUMBER];
    UINT16      uiABPTargetRLDLight[eLD_SEQ_NUMBER];
    UINT8       ucFanPollCount; //A70LV_Larry_0251
    UINT8       ucLensDetection;
	UINT8       ucConstantBrightness;	//G100_Doulas_0008
	UINT32      ulCurrentRunTime;
    sBURNIN_INFORMATION sBurnIn_Info;       //A70LV_Doulas_0015
    UINT8       ucSnapshotUpdatePeriod; //A35G2_CDS_Larry_0020
	UINT8       ucLVPS_110VDet;
	UINT8       ucFanFilter;
    UINT8       ucSnapshotPowerOn;

}sPAL_ILLUMINATION_INFORMATION, *PsPAL_ILLUMINATION_INFORMATION;

typedef struct
{
    UINT16 uiW;
    UINT16 uiY;
    UINT16 uiR;
    UINT16 uiB;
    UINT16 uiG;
}sLIGHT_SENSOR; //A70LV_Larry_0142

//===================== Struct End ===================//


INT16 palIllumination_Poll(UINT16 uiTick);
eEXEC_CODE palIllumination_PowerNormal(void);
eEXEC_CODE palIllumination_UpgradeCooling(void); //A70LV_Larry_0112
eEXEC_CODE palIllumination_PowerStandby(void);
eEXEC_CODE palIllumination_Init(void);
eEXEC_CODE palIllumination_BurnIn_Count_Start(UINT8 ucBurnin_enable, UINT8 ucLampOnTime, UINT16 uiBurnin_Cycle);
eEXEC_CODE palIllumination_BurnIn_Count_Stop(void);
UINT8 palIllumination_BurnIn_Enable(void); //A70LV_Larry_0016
UINT8 palIllumination_Shutter(UINT8 ucEnable); //A70LV_Larry_0081
UINT8 palIllumination_DirectShutter(UINT8 ucEnable);
BOOL palIllumination_FirstLightSourceOnGet(void); //A70LV_Larry_0098
eEXEC_CODE palIllumination_Init_LDHoursSet(UINT32 ulHours);     //A70LV_Doulas_0128 //A70LV_Larry_0268 modify
eEXEC_CODE palIllumination_Init_TotalProjectHoursSet(UINT32 ulHours);   //A70LV_Doulas_0128 //A70LV_Larry_0268 modify
UINT8 palIllumination_ABP_StatusGet(void); //A70LV_Larry_0179
void palIllumination_ABP_StatusSet(UINT8 ucIndex);
UINT32 palIllumination_TotalProjectSec_Get(void); //A70LV_Larry_0276
void palIllumination_LensDetectionSet(UINT8 ucEnable);
UINT8 palIllumination_LensDetectionGet(void);
void palIllumination_InstantOff(void);  //G100_Owen_0064
eEXEC_CODE util_RLD_ValueCheck(UINT32 dwY_Hi, UINT32 dwY_Lo, UINT32 dwX_In, UINT32 dwX_Hi, UINT32 dwX_Lo); 	//G100_Doulas_0023
UINT32 palIllumination_CurrentRunTimeGet(void);
void palIllumination_OPDSnapshotPowerOnDelay(UINT8 cDelay);
void palIllumination_OPDSnapshot_Recode(UINT8 cIndex); //A35G2_CDS_Larry_0020

#ifdef CUSTOM_BARCO
void palIllumination_TEC_Gating(UINT32 dwLD_On); //G100_Steven_0084 start
void palIllumination_TEC_Result(void);
void palIllumination_TEC_Gating_InfoGet(void);
void palIllumination_TEC_Gating_InfoSet( UINT16 cCycleData);
void palIllumination_TEC_Gating_InfoClean(void); //G100_Steven_0084 end
UINT16 palIllumination_TEC_Gating_ResultGet( UINT8 cIndex);  //G100_Steven_0085
#endif

#if defined(CUSTOM_CHRISTIE) || defined(CUSTOM_OPTOMA)
void palIllumination_LightSourceMode_Init(void);
#endif

UINT8 palIllumination_LVPS_110VDetGet(void);


#ifdef __cplusplus
}
#endif

#endif  //_PALILLUMINATION_H_
