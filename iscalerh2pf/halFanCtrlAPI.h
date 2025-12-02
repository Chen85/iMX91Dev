#ifndef _HALFANCTRLAPI_H_
#define _HALFANCTRLAPI_H_
// ==============================================================================
// FILE NAME: HALFANCTRLAPI.H
// DESCRIPTION:
//
//
// modification history
// --------------------
// 05/09/2013, Larry written
// --------------------
// ==============================================================================

#include "dvLDDriver.h"

//#define FAN_NUMBER                  (16)   //get by System Config (eFan_INFO_Num_defined_in_MCU)
//#define THERMAL_SENSOR_NUMBER       (3)

#define FILTER_FAIL_SHUT_DOWN         (60)
#define FILTER_NTC_SHUT_DOWN          (30)
#define FAN_ROTATION_FAIL_SHUT_DOWN   (30)


typedef enum
{
    efsWARMUP,              //0
    efsLAMPIGNITION,        //1
    efsALTITUDECAL,         //2
    efsINITIALMODE,         //3
    efsNORMALMODE,          //4
    efsWHISPER,             //6
    efsCOOLINGMODE,         //5
    efsSTANDBYPOWERSAVE,    //8
    efsQUCIKCOOLING,        //9
    efsNOFANCONTROL,        //7

    efsINVALID,
} eFANSTATE;

typedef union
{
    UINT16 wFlag;

    struct
    {
        unsigned int cCLIModeEnable         : 1;
        unsigned int cHighAltitude          : 1;
        unsigned int cFanCalibration        : 1;
        unsigned int cOverTempdoubleChek    : 1;
        unsigned int cDisableTempCheck      : 1;
        unsigned int cDisableFTypeCheck     : 1;
        unsigned int cDisableRTypeCheck     : 1;
        unsigned int cDisableOsdItem        : 1;
        unsigned int cBurnInMode            : 1;
        unsigned int cKeepRTypeStatus       : 1;
        unsigned int cKeepFTypeStatus       : 1;
        unsigned int cLampCoolingMode       : 1;
    } sBITS;
} uFANFLAGS;


typedef union
{
    UINT16 wFlag;

    struct
    {
        unsigned int cUpdateRtypeVoltage    : 1;
        unsigned int cUpdateFtypeVoltage    : 1;
        unsigned int cUpdateFtypeRpm        : 1;
    } sBITS;
} uFANUPDATEFLAG;

typedef union
{
    UINT16 wFlag;

    struct
    {
        unsigned int cZetaFail          :1;
        unsigned int cOverTemperature   :1;
        unsigned int cFanLock           :1;
        unsigned int cFanStall          :1;
        unsigned int cLampFail          :1;
        unsigned int cCWFail            :1;
    } sBIT;

} uERRORFLAGS;


typedef struct
{
    // Fan Control States
    UINT16          wLampsMode;
    UINT16          wFanState;
    UINT16          wSubState;
    UINT16          wPowerMode;
    UINT16          wRotationMode;
    UINT16          wRunningTimer;
    UINT16          wFanLockTimer;
    UINT16          wFanStallTimer;

    // Fan Control Flags
    uERRORFLAGS     uErrorFlags;
    uFANFLAGS       uFanFlags;
    uFANUPDATEFLAG  uUpdateFlags;
    UINT32         dwLockedRFans;
    UINT32         dwLockedFFans;
    UINT32         dwWorkingRFans;
    UINT32         dwWorkingFFans;
    UINT32         dwStallFans;

    UINT16          wMaxFanStep;
    UINT16          wFanStep;
    INT16         wCurrentTemperature;
    UINT16          *pwZeta;
    UINT16          *pwFactoryFanRpm;
    UINT16          *pwTargetFanRpm;
    UINT16          *pwCurrentFanRpm;
    UINT16          *pwCurrentVoltages;
} sFAN_PROC_STATUS;

// R-Type Fan Number
typedef enum
{
	eR_TYPE_FAN1,

	eR_TYPE_FAN_NUMBERS,
} eR_TYPE_FAN; //A70LH_Larry_0014

#if 0 //A70LV_Larry_0062
// F-Type Fan Number
typedef enum
{
	eF_TYPE_FAN1,
	eF_TYPE_FAN2,
	eF_TYPE_FAN3,
	eF_TYPE_FAN4,
	eF_TYPE_FAN5,
	eF_TYPE_FAN6,
    eF_TYPE_FAN7,
    eF_TYPE_FAN8,
    eF_TYPE_FAN9,
    eF_TYPE_FAN10,

	eF_TYPE_FAN_NUMBERS,
} eFTYPEFAN; //A70LH_Larry_0014
#endif

typedef enum
{
    eFAN_1,
    eFAN_2,
    eFAN_3,
    eFAN_4,
    eFAN_5,
    eFAN_6,
    eFAN_7,
    eFAN_8,
    eFAN_9,
    eFAN_10,

    eFAN_NUMBERS,
} eFAN_ITEM; //A70LV_Larry_0062

typedef enum
{
	ePUMP_ID_1,
	ePUMP_ID_2,

	ePUMP_ID_NUMBERS,
} ePUMP_ID; //A70LH_Larry_0057

typedef enum
{
    eFAN_MODE_STATE_PASS,
    eFAN_MODE_FILTER_FAIL,
    eFAN_MODE_OVERTEMPERTURE,
    eFAN_MODE_SHUTDOWN,
    eFAN_MODE_ROTATION_FAIL, //A70_Larry_0424 add

    eFAN_MODE_NUMBERS,
} eFAN_MODE_STATE;

typedef enum                                                //LDDRV_JS_0118 mod
{
    eFILTER_TYPE_FOG        = 0x00,
    eFILTER_TYPE_NOFILTER_R = 0x01,
    eFILTER_TYPE_NOFILTER_L = 0x02,
    eFILTER_TYPE_NOFILTER   = 0x03,

    eFILTER_TYPE_NUMBERS,
} eFILTER_TYPE;


eRESULT halFanCtrl_HighAltModeSet(UINT8 cData); //A70LV_Larry_0034
eRESULT halFanCtrl_PowerModeSet(UINT8 ucPowerMode);
eRESULT halFanCtrl_Fan_Speed_Get(UINT8* pcData);   // A70LV_Eric.C_0003
eRESULT halFanCtrl_Fan_Lock_Indicate_Get(UINT8* pcData);   // A70LV_Eric.C_0003
eRESULT halFanCtrl_Fan_Duty_Get(UINT8* pcData);    // A70LV_Eric.C_0003
eRESULT halFanCtrl_Thermal_Sensor_Get(UINT8* pcData);   // A70LV_Eric.C_0007
eRESULT halFanCtrl_Liquid_Speed_Get(UINT8* pcData);
eRESULT halFanCtrl_NoFanControlSet(void); //A70LV_Larry_0078
eRESULT halFanCtrl_OperationModeSet(void);
eRESULT halFanCtrl_Fan_Speed_All_Get(UINT8* pcData); //A70LV_Larry_0144
eRESULT halFanCtrl_Fan_Duty_All_Get(UINT8* pcData); //A70LV_Larry_0144
eRESULT halFanCtrl_Thermal_Sensor_All_Get(UINT8* pcData); //A70LV_Larry_0144
eRESULT halFanCtrl_LD_OverTemperatureNumber_Get(UINT8* pcData);
eRESULT halFanCtrl_Filter_Get(UINT8* pcData); //A70LV_Larry_0293
eRESULT halFanCtrl_Wheel_Speed_Get(UINT8* pcData);   //G100_Julie_0014
eRESULT halFanCtrl_Platform_Get(UINT8* pcData);
eRESULT halFanCtrl_Platform_Set(UINT8 cData);
eRESULT halFanCtrl_Customer_Set(UINT8 cData);
eRESULT halFanCtrl_SkuReload_Set(UINT8 cData);


#endif /*_HALFANCTRLAPI_H_*/
