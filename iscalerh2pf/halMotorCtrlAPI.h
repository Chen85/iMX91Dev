#ifndef _HALMOTORCTRLAPI_H_
#define _HALMOTORCTRLAPI_H_
// ==============================================================================
// FILE NAME: HALMOTORCTRLAPI.H
// DESCRIPTION:
//
//
// modification history
// --------------------
// 06/09/2013, Larry
// 15/11/2013, Larry Fixed
// 06/01/2014, Larry Fixed
// 26/03/2014, Larry Fixed
// --------------------
// ==============================================================================

#include "Common.h"

//Lens Memory default value //A70_Larry_0257
#define MAX_LENMEMORY_RECORDER      5
#define DEF_LENS_H_POSITION         5870
#define DEF_LENS_V_POSITION         8805
#define DEF_LENS_ZOOM_POSITION      512
#define DEF_LENS_FOCUS_POSITION     512

#pragma pack(push)  /* push current alignment to stack */
#pragma pack(1)     /* set alignment to 1 byte boundary */


#pragma pack(pop)   /* restore original alignment from stack */

typedef enum
{
    eSHUTTER_CMDS_IDEL,
    eSHUTTER_CMDS_GO_TO_ON,
    eSHUTTER_CMDS_GO_TO_OFF,
    eSHUTTER_CMDS_CALIBRATION,
    eSHUTTER_CMDS_FORCE_GO_OFF,
    eSHUTTER_CMDS_FORCE_GO_ON,
    eSHUTTER_CMDS_BROKEN,

    eSHUTTER_CMDS_NUMBERS,
} eSHUTTER_CMDS;

//A70_Larry_0152 start  //A70_Larry_0308
typedef enum
{
    eSHUTTER_CURRENT_IDEL,
    eSHUTTER_CURRENT_ON,
    eSHUTTER_CURRENT_OFF,
    eSHUTTER_CURRENT_HOME,
    eSHUTTER_CURRENT_ERROR,

    eSHUTTER_CURRENT_NUMBERS,
} eSHUTTER_CURRENT_POS;

//A70_Larry_0152 end

typedef enum
{
    /*  0 */ eMOTOR_EVENT_LENS,
    /*  1 */ eMOTOR_EVENT_FOCUS,
    /*  2 */ eMOTOR_EVENT_ZOOM,
    /*  3 */ eMOTOR_EVENT_LENSCALIBRATION,
    /*  4 */ eMOTOR_EVENT_LENSMOVING,
    /*  5 */ eMOTOR_EVENT_LENS_MEMORY,
    /*  6 */ eMOTOR_EVENT_LENS_IDLE,
    /*  7 */ eMOTOR_EVENT_FOCUS_LIMIT,
    /*  8 */ eMOTOR_EVENT_ZOOM_LIMIT,
    /*  9 */ eMOTOR_EVENT_LENS_BACKLASH,
    /* 10 */ eMOTOR_EVENT_ENCODE,           //A70_Larry_0116
    /* 11 */ eMOTOR_EVENT_LENS_MEMORY_SAVE,
    /* 12 */ eMOTOR_EVENT_LENS_MEMORY_APPLY,

    /* 20 */ eMOTOR_EVENT_AF_MODULE_TEST = 20,  //For SMT test GPIO

    eMOTOR_EVENT_NUMBERS,
} eMOTOR_CONTROL_CMD;



typedef enum
{
    eMOTOR_SHUTTER,

    eMOTOR_SHUTTER_NUMBERS,
} eMOTOR_SHUTTER_CMD;

typedef enum
{
    eLENS_CAL_IDLE,         // Idle無動作
    eLENS_CAL_CHECK_SENSOR, // 檢查是否已經在Home(sensor), 在Home點則先離開
    eLENS_CAL_FINE_HOME,    // 返回Home(Sensor)
    eLENS_CAL_GO_CENTER,    // 回到Center點
    eLENS_CAL_ERROR,        // 發生錯誤,可能是馬達或Sensor有問題

    eLENS_CAL_NUMBERS,
} eLENS_CAL;

typedef enum
{
    eOLD_LENS,
    eA16_LENS,
} LENS_TYPE;    //A70LH_Doulas_0241

typedef enum
{
    eLENS_MODULE_T100,
    eLENS_MODULE_A70LV,
    eLENS_MODULE_A65,
    eLENS_MODULE_X35_S600,
    eLENS_MODULE_X35_TYPA,
    eLENS_MODULE_H60,

    eLENS_MODULE_A16 = 99,

    eLENS_MODULE_NUMBERS,
} eLENS_MODULE;

#if 0
typedef enum
{
    eLENS_ID_A26,           // ID #1 , 10000 ohm
    eLENS_ID_A25,           // ID #2 ,  8200 ohm
    eLENS_ID_A23,           // ID #3 ,  6800 ohm
    eLENS_ID_A22,           // ID #4 ,  5600 ohm
    eLENS_ID_A21,           // ID #5 ,  3900 ohm
    eLENS_ID_A20,           // ID #6 ,  3000 ohm
    eLENS_ID_AUST,          // ID #7 ,  2200 ohm
    eLENS_ID_AXX_8,         // ID #8 ,  1000 ohm
    eLENS_ID_A11,           // ID #9 , 12000 ohm
    eLENS_ID_A27,           // ID #10, 14700 ohm
    eLENS_ID_A18,           // ID #11, 17800 ohm
    eLENS_ID_A19,           // ID #12, 22000 ohm
    eLENS_ID_AXX_13,        // ID #13, 27400 ohm
    eLENS_ID_AXX_14,        // ID #14, 35700 ohm
    eLENS_ID_AXX_15,        // ID #15, 49900 ohm
    eLENS_ID_AXX_16,        // ID #16, 82000 ohm
    eLENS_ID_AUST_BARCO,    // ID #??,       GND    //A65_Owen_0004

    eLENS_ID_NUMBERS,       // used for enumerated type range checking (DO NOT REMOVE)
} eLENS_ID_A70TYPE; //A70LV_Larry_0188

typedef enum
{
    eLENS_ID2_REV0,  //93  //reserve
    eLENS_ID2_REV1,  //185 //reserve
    eLENS_ID2_A01,   //236
    eLENS_ID2_A02,   //287
    eLENS_ID2_REV2,  //368
    eLENS_ID2_A03,   //414
    eLENS_ID2_A06,   //461
    eLENS_ID2_A13,   //512
    eLENS_ID2_A15,   //559
    eLENS_ID2_A15F,  //614
    eLENS_ID2_A16,   //658
    eLENS_ID2_A17,   //704
    eLENS_ID2_KMUST, //748
    eLENS_ID2_Y06,   //801
    eLENS_ID2_REV3,  //853
    eLENS_ID2_REV4,  //913

    eLENS_ID2_NUMBERS,       // used for enumerated type range checking (DO NOT REMOVE)
} eLENS_ID2_A35TYPE;
#endif /* 0 */

typedef enum
{
    eHAL_MOTOR_EXEC_CODE_PASS,    /* pass */
    eHAL_MOTOR_EXEC_CODE_FAIL,    /* general fail indication */
    eHAL_MOTOR_EXEC_CODE_INIT_FAIL,
    eHAL_MOTOR_EXEC_CODE_NOT_INIT,
    eHAL_MOTOR_EXEC_CODE_MALLOC_FAIL,
    eHAL_MOTOR_EXEC_CODE_MUTEX_FAIL,
}eHAL_MOTOR_EXEC_CODE;

typedef enum
{
    eLENS_HEADER_CORE_SN,
    eLENS_HEADER_VENDOR_SN,
    eLENS_HEADER_CUSTOMER_SN,
    eLENS_HEADER_NVRAM_ID,

    eLENS_HEADER_MAX_BRIGHTNESS,
    eLENS_HEADER_THROW_RATIO_MAX,
    eLENS_HEADER_THROW_RATIO_MIN,
    eLENS_HEADER_CONTROLLER,
    eLENS_HEADER_ENCODER,
    eLENS_HEADER_LIMIT_ZOOM_OUT,
    eLENS_HEADER_LIMIT_ZOOM_IN,
    eLENS_HEADER_LIMIT_FOCUS_FAR,
    eLENS_HEADER_LIMIT_FOCUS_NEAR,
    eLENS_HEADER_LIMIT_FOCUS2_FAR,
    eLENS_HEADER_LIMIT_FOCUS2_NEAR,

    eLENS_SYS_PARAM_NUMBERS,
} eLENS_SYS_PARAM;

#if 0
void halMotorEvent_CeilingSet(BOOL bCeiling);
BOOL halMotorEvent_CeilingGet(void);        //A70LV_Doulas_0062
void halMotorEvent_RearSet(BOOL bIsRear);
BOOL halMotorEvent_RearGet(void);
#endif
eHAL_MOTOR_EXEC_CODE halMotor_Init(void);
void halMotorLensSet(eLENS_CMDS eLensMoving);
void halMotorLensRunPixelSet(eLENS_CMDS eLensCmd, UINT32 ulPixel);  //G100_Wilsonj_0018
void halMotorFocusSet(eZOOMLENS_CMD eFocusMoving);
UINT8 halMotorFocusGet(void);  //A65_OPTOMA_CL_0008
void halMotorFocusSetWithDuration(eZOOMLENS_CMD eFocusMoving, UINT16 uiDuration);  //A65_OPTOMA_CL_0011
void halMotorFocusDurationGet(UINT8 *pcData);  //A65_OPTOMA_CL_0011
void halMotorZoomSet(eZOOMLENS_CMD eZoomMoving);
WORD halMotorLensCalibrationStatusGet(void);
void halLensMotorGoCenter(void);
BOOL halLensLensIdleGet(void);
UINT8 halLensMovingStateGet(void);    //G100_Owen_0054
UINT8 halLensMovingStartGet(void);
void halLensMovingRestartSet(BOOL bEnable);
BOOL halLensMovingRestartGet(void);
void halLensLensPositionGet(sLENSMEMORY *psLensmemory); //A70LV_Larry_0287
void halLensLensMemorySet(sLENSMEMORY *psLensmemory);
void halMotor_Focue_Limit_Get(sMOTOR_LIMIT *psMotor_limit);
void halMotor_Zoom_Limit_Get(sMOTOR_LIMIT *psMotor_limit);
void halMotor_Lens_Backlash_Get(sMOTOR_LIMIT *psMotor_limit);
UINT8 halMotor_Lens_Encode_Get(void);
BOOL halMoter_LensEncodeSaveGet(void);
void halLensLensMemorySaveSet(UINT8 cLensMemoryID);
UINT8 halLensLensMemoryApplyGet(void);
void halLensLensMemoryApplySet(UINT8 cLensMemoryID);
void halLensA16MotorGoCenter(void);
void halLensSelectSet(UINT8 cLens);
eRESULT halMotor_Version_Get(UINT8* pcData); // A70LV_Eric.C_0024 //A70LV_Larry_0112
eRESULT halMotor_LensModule_Get(UINT8* pcData);
eRESULT halMotor_LensModule_Set(UINT8* pcData);
eRESULT halMotor_OEJig_Set(UINT8 ucEnable);
eRESULT halMotor_LensMemoryRecord_Get(UINT8* pcData); //A70LV_Larry_0151
eRESULT halMotor_LensMemoryReset(void); //A70LV_Larry_0151
UINT8 halMotor_LensMemoryStatus_Get(void); //A70LV_Larry_0151
UINT8 halMotor_LensId_Get(void); //A70LV_Larry_0188
eRESULT halMotor_ZoomLimit_Get(sMOTOR_LIMIT *psLimit);
eRESULT halMotor_FocusLimit_Get(sMOTOR_LIMIT *psLimit);
UINT8 halMotor_LensPlug_Get(void); //A70LV_Larry_0284
UINT8 halMotor_LensFujiDet_Get(void);
eRESULT halMotor_LensIdADC_Get(UINT32 *psIDAdc); //G100_Owen_0055
UINT8 halMotor_WR_Test(void);
eRESULT halMotor_LensCenterEnable_Get(UINT8 *pucEnable);
eRESULT halMotor_LensCenterEnable_Set(UINT8 ucEnable);
eRESULT halMotor_LensCenterApply_Set(void);
eRESULT halMotor_LensCalDone_Get(UINT8 *pucLensCalFlg);
eRESULT halMotor_LensCenterSettingSet(UINT8 *pcData);
eRESULT halMotor_LensCenterSettingGet(UINT8 *pcData);
eRESULT halMotor_ZoomDutySet(BYTE cMax, BYTE cMin);
eRESULT halMotor_FocusDutySet(BYTE cMax, BYTE cMin);
eRESULT halMotor_LensCenterGet(UINT8 *pcData);
eRESULT halMotor_LensPositionFullGet(UINT8 *pcData);
eRESULT halMotor_LensPositionActiveGet(UINT8 *pcData);

UINT8 halMotorLensMovingStateGet(void);
void halMotorLensPositionGet(sLENSMEMORY *psLensmemory);
BOOL halMotorLensIdleGet(void);
void halMotorLensMemorySaveSet(UINT8 cLensMemoryID);
UINT8 halMotorLensMemoryApplyGet(void);
void halMotorLensMemoryApplySet(UINT8 cLensMemoryID);
void halMotorLensA16MotorGoCenter(void);
void halMotorLensSelectSet(UINT8 cLens);
eRESULT halMotor_Lens_Speed_Set(UINT8* pcData);
eRESULT halMotor_Lens_Speed_Control_Set(UINT8* pcData);
eRESULT halMotor_Lens_Speed_Control_Get(UINT16 *pcData);
eRESULT halMotor_LENS_EEPROM_Version_Get(UINT8* pcData);  //HICC2_Steven_0027
UINT8 halMotor_LensShiftStatus_Get(void); //HICC2_Casper_0040
UINT8 halMotor_Lsm_Sensor_Mute_H_Get(void);
eRESULT halMotor_Lsm_Sensor_Mute_H_Set(UINT8 pcData);
UINT8 halMotor_Lsm_Sensor_Mute_V_Get(void);
eRESULT halMotor_Lsm_Sensor_Mute_V_Set(UINT8 pcData);
UINT8 halMotor_Lsm_Boundary_Mute_H_Get(void);
eRESULT halMotor_Lsm_Boundary_Mute_H_Set(UINT8 pcData);
UINT8 halMotor_Lsm_Boundary_Mute_V_Get(void);
eRESULT halMotor_Lsm_Boundary_Mute_V_Set(UINT8 pcData);
eRESULT halMotor_Lsm_Boundary_Calibration_Set(UINT8 pcData);
eRESULT halMotor_Lsm_V_Up_Boundary_Set(UINT8 pcData);
eRESULT halMotor_Lsm_V_Down_Boundary_Set(UINT8 pcData);
eRESULT halMotor_Lsm_H_Left_Boundary_Set(UINT8 pcData);
eRESULT halMotor_Lsm_H_Right_Boundary_Set(UINT8 pcData);

#if defined(PLATFORM_R70K) || defined(PLATFORM_H30_4K) //HICC2_AC_0002
void halMotor_LSM_Version_Get(void);
void halMotor_LSM_Center_Get(void);
void halMotor_LSM_Center_Set(void);
void halMotor_LSM_Center_Clear(void);
void halMotor_LSM_Status_Ask(void);
void halMotor_LSM_CalibrationDone_Ask(void);
void halMotor_LSM_CalibrationStatus_Set(UINT16 wData);
UINT8 halMotor_LSM_CalibrationStatus_Get(void);
void halMotor_LSM_CalibrationDone_Set(UINT8 wData);
UINT8 halMotor_LSM_CalibrationDone_Get(void);
void halMotor_LSM_Lens_ID_Ask(void);
void halMotor_LSM_Lens_Module_Set(UINT8 uData);
UINT8 halMotor_LSM_Lens_Module_Get(void);
void halMotor_LSM_Lens_ID_Set(UINT8 uData);
UINT8 halMotor_LSM_Lens_ID_Get(void);
void halMotor_LSM_Factory_Menu(void);
void halMotor_LSM_Factory_Menu_Lens(void);
void halMotor_LSM_Factory_Menu_ZoomFocus(void);
void halMotor_LSM_Center_Config_Set(UINT8 uData);
UINT8 halMotor_LSM_Center_Config_Get(void);
void halMotor_LSM_Memory_Set(UINT8 uData); //HICC2_AC_0019
UINT8 halMotor_LSM_Memory_Get(void);
void halMotor_LSM_Memory_Query(void);
void halMotor_LSM_Memory_Clear(void);
void halMotor_LSM_Lens_Plug_Set(UINT8 uData);
UINT8 halMotor_LSM_Lens_Plug_Get(void);
void halMotor_LSM_Lens_Plug_Query(void);
void halMotor_LSM_NVRAM_Status_Set(UINT8 uVal);
UINT8 halMotor_LSM_NVRAM_Status_Get(void);
void halMotor_LSM_Lens_Lock_Query(void);//DET2

#endif

eRESULT halMotor_Lens_Header_Get(UINT8 cSize, UINT8 *pcData);
eRESULT halMotor_Lens_Header_Index_Set(UINT8 pcData);
BYTE halMotor_Lens_Header_Param_Size(UINT8 cIndex);
void halMotor_LSM_Zoom_Duration_Set(void);
void halMotor_LSM_Focus_Duration_Set(void);
void halMotor_LSM_BackFocus_Duration_Set(void);
eRESULT halMotor_FujiLens_BackFocus_Set(UINT8* pcData);
eRESULT halMotor_FujiLens_BackFocus_Get(UINT16 *pcData);
void halMotor_LSM_Lens_Detection_Bypass(BOOL bBypass);
void halMotor_LSM_Lens_Detection_Bypass_Query(void);



#endif //_HALMOTORCTRLAPI_H_

