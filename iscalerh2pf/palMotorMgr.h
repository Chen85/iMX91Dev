#ifndef _PALMOTORMGR_H_
#define _PALMOTORMGR_H_

#include "Common.h"
#include "utilDbgMsg.h"


#if 0
typedef enum
{
    //A70 Lens Type
    ePAL_A70_LENS_ID_A26,           // ID #1 , 10000 ohm
    ePAL_A70_LENS_ID_A25,           // ID #2 ,  8200 ohm
    ePAL_A70_LENS_ID_A23,           // ID #3 ,  6800 ohm
    ePAL_A70_LENS_ID_A22,           // ID #4 ,  5600 ohm
    ePAL_A70_LENS_ID_A21,           // ID #5 ,  3900 ohm
    ePAL_A70_LENS_ID_A20,           // ID #6 ,  3000 ohm
    ePAL_A70_LENS_ID_AUST,          // ID #7 ,  2200 ohm
    ePAL_A70_LENS_ID_AXX_8,         // ID #8 ,  1000 ohm
    ePAL_A70_LENS_ID_A11,           // ID #9 , 12000 ohm
    ePAL_A70_LENS_ID_A27,           // ID #10, 14700 ohm
    ePAL_A70_LENS_ID_A18,           // ID #11, 17800 ohm
    ePAL_A70_LENS_ID_A19,           // ID #12, 22000 ohm
    ePAL_A70_LENS_ID_AXX_13,        // ID #13, 27400 ohm
    ePAL_A70_LENS_ID_AXX_14,        // ID #14, 35700 ohm
    ePAL_A70_LENS_ID_AXX_15,        // ID #15, 49900 ohm
    ePAL_A70_LENS_ID_AXX_16,        // ID #16, 82000 ohm
    ePAL_A70_LENS_ID_AUST_BARCO,    // ID #??,       GND

    ePAL_A70_LENS_ID_NUMBERS,

    //A35 Lens Type
    ePAL_A35_LENS_ID2_REV0,  //93  //reserve
    ePAL_A35_LENS_ID2_REV1,  //185 //reserve
    ePAL_A35_LENS_ID2_A01,   //236
    ePAL_A35_LENS_ID2_A02,   //287
    ePAL_A35_LENS_ID2_REV2,  //368
    ePAL_A35_LENS_ID2_A03,   //414
    ePAL_A35_LENS_ID2_A06,   //461
    ePAL_A35_LENS_ID2_A13,   //512
    ePAL_A35_LENS_ID2_A15,   //559
    ePAL_A35_LENS_ID2_A15F,  //614
    ePAL_A35_LENS_ID2_A16,   //658
    ePAL_A35_LENS_ID2_A17,   //704
    ePAL_A35_LENS_ID2_KMUST, //748
    ePAL_A35_LENS_ID2_Y06,   //801
    ePAL_A35_LENS_ID2_REV3,  //853
    ePAL_A35_LENS_ID2_REV4,  //913
    ePAL_A35_LENS_ID2_REV5,

    ePAL_A35_LENS_ID2_NUMBERS,       // used for enumerated type range checking (DO NOT REMOVE)

    //H60 Lens Type
    ePAL_H60_LENS_ID_NVRAM,         // ID #8 ,  1000 ohm
    ePAL_H60_LENS_ID_AUST,          // ID #7 ,  2200 ohm
    ePAL_H60_LENS_ID_A20,           // ID #6 ,  3000 ohm
    ePAL_H60_LENS_ID_A21,           // ID #5 ,  3900 ohm
    ePAL_H60_LENS_ID_A22,           // ID #4 ,  5600 ohm	//40
    ePAL_H60_LENS_ID_A23,           // ID #3 ,  6800 ohm
    ePAL_H60_LENS_ID_A25,           // ID #2 ,  8200 ohm
    ePAL_H60_LENS_ID_A26,           // ID #1 , 10000 ohm
    ePAL_H60_LENS_ID_A11,           // ID #9 , 12000 ohm
    ePAL_H60_LENS_ID_A27,           // ID #10, 14700 ohm
    ePAL_H60_LENS_ID_A18,           // ID #11, 17800 ohm
    ePAL_H60_LENS_ID_A19,           // ID #12, 22000 ohm
    ePAL_H60_LENS_ID_EN68,          // ID #13, 27400 ohm
    ePAL_H60_LENS_ID_T20C1,         // ID #14, 36000 ohm //HICC2_Doulas_0030
    ePAL_H60_LENS_ID_T20C2,         // ID #15, 49900 ohm
    ePAL_H60_LENS_ID_FUJI_UST,//51      // ID #16, 82000 ohm
    ePAL_H60_LENS_ID_AUST_BARCO,    // ID #17,       GND

    ePAL_H60_LENS_ID_NUMBERS,

    ePAL_LENS_ID_NUMBER,       // used for enumerated type range checking (DO NOT REMOVE)
} ePAL_LENS_ID;

typedef enum
{
    eNVRAM_ID_B20U100      = 20,     //20
    eNVRAM_ID_T20C1,                 //21
    eNVRAM_ID_T20C2,                 //22

    eNVRAM_ID_NUMBERS,
} eNVRAM_ID;
#endif
typedef struct
{
	UINT32 Row;
	UINT32 Column;
	UINT32 Length;
	UINT32 GetSeek;
	char *GetBuffer;
} sFILE_PARSE;

typedef struct
{
    INT32 lID;
    INT32 lDefined;
    INT32 lSupported;
    char cLensName[32];
    char cThrowRatio[32];
    char cID[32];
} sLENS_TYPE;

#pragma pack(push)  /* push current alignment to stack */
#pragma pack(1)     /* set alignment to 1 byte boundary */


#pragma pack(pop)

void palMotorLens_Init(void); //HICC2_Casper_0034
void palMotorEvent_CeilingSet(BOOL bCeiling);
BOOL palMotorEvent_CeilingGet(void);
void palMotorEvent_RearSet(BOOL bIsRear);
BOOL palMotorEvent_RearGet(void);
static eLENS_CMDS palMotorEvent_Remap(eLENS_CMDS eLensEvent);
void palMotorLensSet(eLENS_CMDS eLensMoving);
void palMotorLensRunPixelSet(eLENS_CMDS eLensCmd, UINT32 ulPixel);
void palMotorFocusSet(eZOOMLENS_CMD eFocusMoving);
void palMotorFocusSetWithDuration(eZOOMLENS_CMD eFocusMoving, UINT16 uiDuration);
UINT16 palMotorFocusDurationGet(void);
UINT8 palMotorFocusGet(void);
void palMotorZoomSet(eZOOMLENS_CMD eZoomMoving);

WORD palMotorLensCalibrationStatusGet(void);

void palMotorLensGoCenter(void);
UINT8 palMotorLensMovingStateGet(void);
//unused
UINT8 palMotorLensMovingStartGet(void);

void palMotorLensPositionGet(sLENSMEMORY *psLensmemory);

//unused
void palMotorLensMemorySet(sLENSMEMORY *psLensmemory);
//unused
BOOL palMotorLensIdleGet(void);
//unused
void palMotor_Focue_Limit_Get(sMOTOR_LIMIT *psMotor_limit);
//unused
void palMotor_Zoom_Limit_Get(sMOTOR_LIMIT *psMotor_limit);
void palMotor_Lens_Backlash_Get(sMOTOR_LIMIT *psMotor_limit);
//unused
UINT8 palMotor_Lens_Encode_Get(void);
BOOL palMoter_LensEncodeSaveGet(void);
void palMotorLensMemorySaveSet(UINT8 cLensMemoryID);
//unused
UINT8 palMotorLensMemoryApplyGet(void);
void palMotorLensMemoryApplySet(UINT8 cLensMemoryID);
//unused
void palMotorLensA16MotorGoCenter(void);
//unused
void palMotorLensSelectSet(UINT8 cLens);
eRESULT palMotor_Version_Get(UINT8* pcData);
eRESULT palMotor_LensModule_Get(UINT8* pcData);
eRESULT palMotor_LensModule_Set(UINT8* pcData);
eRESULT palMotor_OEJig_Set(UINT8 Enable);
eRESULT palMotor_LensMemoryRecord_Get(UINT8* pcData);
eRESULT palMotor_LensMemoryReset(void);
UINT8 palMotor_LensMemoryStatus_Get(void);
UINT8 palMotor_LensId_Get(void);
eRESULT palMotor_ZoomLimit_Get(sMOTOR_LIMIT *psLimit);
eRESULT palMotor_FocusLimit_Get(sMOTOR_LIMIT *psLimit);
UINT8 palMotor_LensPlug_Get(void);
UINT8 palMotor_LensFujiDet_Get(void);
eRESULT palMotor_LensIdADC_Get(UINT32 *psIDAdc);
UINT8 palMotor_WR_Test(void);
eRESULT palMotor_LensCenterEnable_Get(UINT8 *pucEnable);
eRESULT palMotor_LensCenterEnable_Set(UINT8 ucEnable);
eRESULT palMotor_LensCenterApply_Set(void);
eRESULT palMotor_LensCalDone_Get(UINT8 *pucLensCalFlg);
eRESULT palMotor_LensCenterSettingSet(UINT8 *pcData);
eRESULT palMotor_LensCenterSettingGet(UINT8 *pcData);
eRESULT palMotor_ZoomDutySet(BYTE cMax, BYTE cMin);
eRESULT palMotor_FocusDutySet(BYTE cMax, BYTE cMin);
eRESULT palMotor_LensCenterGet(UINT8 *pcData);
eRESULT palMotor_LensPositionFullGet(UINT8 *pcData);
eRESULT palMotor_LensPositionActiveGet(UINT8 *pcData);
eRESULT palMotor_Lens_Speed_Set(UINT8* pcData);
eRESULT palMotor_Lens_Speed_Control_Set(UINT8* pcData);
eRESULT palMotor_Lens_Speed_Control_Get(UINT16 *pcData);
eRESULT palMotor_LENS_EEPROM_Version_Get(UINT8* pcData);  //HICC2_Steven_0027
UINT8 palMotor_Lens_Shift_Status_Get(void); //HICC2_Casper_0041
UINT8 palMotor_Lsm_Sensor_Mute_H_Get(void);
eRESULT palMotor_Lsm_Sensor_Mute_H_Set(UINT8 ucMute);
UINT8 palMotor_Lsm_Sensor_Mute_V_Get(void);
eRESULT palMotor_Lsm_Sensor_Mute_V_Set(UINT8 ucMute);
UINT8 palMotor_Lsm_Boundary_Mute_H_Get(void);
eRESULT palMotor_Lsm_Boundary_Mute_H_Set(UINT8 ucMute);
UINT8 palMotor_Lsm_Boundary_Mute_V_Get(void);
eRESULT palMotor_Lsm_Boundary_Mute_V_Set(UINT8 ucMute);
eRESULT palMotor_Lsm_Boundary_Calibration_Set(UINT8 ucMute);
eRESULT palMotor_Lsm_V_Up_Boundary_Set(UINT8 ucMute);
eRESULT palMotor_Lsm_V_Down_Boundary_Set(UINT8 ucMute);
eRESULT palMotor_Lsm_H_Left_Boundary_Set(UINT8 ucMute);
eRESULT palMotor_Lsm_H_Right_Boundary_Set(UINT8 ucMute);
void palMotor_LensDetection_State_Set(BYTE ucVal);
UINT8 palMotor_LensDetection_State_Get(void);
eRESULT palMotor_Lens_FujiLens_BackFocus_Set(UINT8* pcData);
void palMotor_LensLock_State_Set(BYTE ucVal);
UINT8 palMotor_LensLock_State_Get(void);

#if defined(PLATFORM_R70K) || defined(PLATFORM_H30_4K) //HICC2_AC_0002
eRESULT palMotor_LSM_Version_Get(void);
eRESULT palMotor_LSM_Center_Get(void);
eRESULT palMotor_LSM_Center_Set(void);
eRESULT palMotor_LSM_Center_Clear(void);
eRESULT palMotor_LSM_Status_Ask(void);
eRESULT palMotor_LSM_CalibrationDone_Ask(void);
eRESULT palMotor_LSM_CalibrationStatus_Set(UINT8 wData);
void palMotor_LSM_CalibrationDone_Set(UINT8 wData);
UINT8 palMotor_LSM_CalibrationDone_Get(void);
eRESULT palMotor_LSM_Lens_ID_Get(void);
eRESULT palMotor_LSM_Factory_Menu(void);
eRESULT palMotor_LSM_Memory_Set(UINT8 uData);
eRESULT palMotor_LSM_Memory_Get(void);
eRESULT palMotor_LSM_Memory_Query(void);
eRESULT palMotor_LSM_Memory_Clear(void);
void palMotor_LSM_Lens_Plug_Set(UINT8 wData);
UINT8 palMotor_LSM_Lens_Plug_Get(void);
eRESULT palMotor_LSM_Plug_Query(void);
eRESULT palMotor_LSM_Lock_Query(void);
void palMotor_LSM_Lens_Detection_Bypass(BOOL bBypass);
void palMotor_LSM_Lens_Detection_Bypass_Query(void);

#endif
int palMotor_LensTable_GetRow(sFILE_PARSE *psFileParse, const char *pFile);
int palMotor_LensTable_ParseRow(sFILE_PARSE *psFileParse, sLENS_TYPE *psLensType);

#endif //_PALMOTORMGR_H_

