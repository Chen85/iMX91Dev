#ifndef HPBU_TESTERH
#define HPBU_TESTERH

#include "Common.h"
//#include "dvA70LV_LDDriver.h"   // A70LV_Eric.C_0004
//#include "halFanCtrlAPI.h"   // A70LV_Eric.C_0004
//#include "halMotorCtrlAPI.h"    // A70LV_Eric.C_0024



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// file name : HPBU_Tester.h
// //Common_James_0258 // Add Tester Module code
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//#define _BCB_MEM  // use stand along memory.
//#define _FIRMWARE_SYSTME
#define COMMAND_LENGTH      (4)
#define COMMAND_DATA_START  (8)

typedef enum
{
	eOFF,
    eON,
} eSWITCH;

typedef struct
{
    char chCommand[COMMAND_LENGTH];
    unsigned char cType;
    unsigned char cDataSize;
    unsigned char *pcData;

} HPBU_TEST_COM_DATA;

#define _TesterMaxItems     200
#define _TesterMaxData      100      // A70LH_Jonas_0048, 40 --> 80 //A70LH_Larry_0062 100

typedef struct _Rs232TesterCommLutTable
{
    char cFuncCode[COMMAND_LENGTH];
    unsigned char(*fpTester_Request)(HPBU_TEST_COM_DATA *spComm);
    unsigned int uiSpecialFlag;
} Rs232TesterCommLutTable;

#ifdef CUSTOM_BARCO
//G100_Steven_0080
typedef enum //LDDRV_John_0040 add command for TEC JIG data transfer
{
	eTEC_Data_DMD_ENVIR_T, //G100_Steven_0091
    eTEC_Data_DMD_V1,
    eTEC_Data_DMD_V60,
    eTEC_Data_DMD_T0,
    eTEC_Data_DMD_dT,
    eTEC_Data_DMD_I,

    eTEC_Data_MaxList
} eTEC_Data_List;
#endif

////////  callback function start  ////////
typedef void (*fpDataMgr_ServiceModeSet)(UINT8 ucVal);
#ifdef PALDATAMGR_ACCESS_WITHLOG
typedef eEXEC_CODE (*fpDataMgr_Data_Access)(eDATA_CODE DataCode, eDATA_ACCESS_MODE eAccessMode, void *pValue, const char *cFuncName, UINT32 ulLineNum);
#else
typedef eEXEC_CODE (*fpDataMgr_Data_Access)(eDATA_CODE DataCode, eDATA_ACCESS_MODE eAccessMode, void *pValue);
#endif
typedef eEXEC_CODE (*fpDataMgr_Data_StructDataAccess)(eDATA_CODE DataCode, eDATA_ACCESS_MODE eAccessMode, void *pValue);
typedef UINT8 (*fpDataMgr_Access_Get_FAN_Duty)(UINT8 FAN_Index);
typedef UINT16 (*fpDataMgr_Access_Get_FAN_RPM)(UINT8 FAN_Index);
typedef UINT16 (*fpIllumination_TEC_Gating_ResultGet)( UINT8 cIndex);
typedef UINT16 (*fpDataMgr_Access_Get_LD_Voltage)(UINT8 ucIndex);
typedef UINT16 (*fpDataMgr_Access_Get_LD_Current)(UINT8 ucIndex);
typedef INT16 (*fpDataMgr_Access_Get_LD_Temperature)(UINT8 ucIndex);
typedef UINT16 (*fpDataMgr_Access_Get_TEC_Current)(UINT8 ucIndex);
typedef INT16 (*fpDataMgr_Access_Get_Thermal_Sensor)(UINT8 ucIndex);
typedef UINT32 (*fpIllumination_CurrentRunTimeGet)(void);
typedef UINT16 (*fpDataMgr_Access_Get_ALTIMETRY)(void);
typedef void (*fpDataMgr_DateTime_TimeZoneStr_Set)(BOOL bSigned, char *pcTimeZoneStr);
typedef void (*fpDataMgr_DateTime_TimeZoneStr_Get)(char *pcTimeZoneStr);
typedef void (*fpDataMgr_OPDRegulatoryInfo)(void);
typedef UINT8 (*fpDataMgr_LD_Info_Num_Get)(void);
typedef UINT8 (*fpDataMgr_BLD_Num_Get)(void);
typedef UINT8 (*fpDataMgr_RLD_Num_Get)(void);

typedef struct
{
    fpDataMgr_ServiceModeSet fpDataMgr_ServiceModeSetCb;
    fpDataMgr_Data_Access fpDataMgr_Data_AccessCb;
    fpDataMgr_Data_StructDataAccess fpDataMgr_Data_StructDataAccessCb;
    fpDataMgr_Access_Get_FAN_Duty fpDataMgr_Access_Get_FAN_DutyCb;
    fpDataMgr_Access_Get_FAN_RPM fpDataMgr_Access_Get_FAN_RPMCb;
    fpIllumination_TEC_Gating_ResultGet fpIllumination_TEC_Gating_ResultGetCb;
    fpDataMgr_Access_Get_LD_Voltage fpDataMgr_Access_Get_LD_VoltageCb;
    fpDataMgr_Access_Get_LD_Current fpDataMgr_Access_Get_LD_CurrentCb;
    fpDataMgr_Access_Get_LD_Temperature fpDataMgr_Access_Get_LD_TemperatureCb;
    fpDataMgr_Access_Get_TEC_Current fpDataMgr_Access_Get_TEC_CurrentCb;
    fpDataMgr_Access_Get_Thermal_Sensor fpDataMgr_Access_Get_Thermal_SensorCb;
    fpIllumination_CurrentRunTimeGet fpIllumination_CurrentRunTimeGetCb;
    fpDataMgr_Access_Get_ALTIMETRY fpDataMgr_Access_Get_ALTIMETRYCb;
    fpDataMgr_DateTime_TimeZoneStr_Set fpDataMgr_DateTime_TimeZoneStr_SetCb;
    fpDataMgr_DateTime_TimeZoneStr_Get fpDataMgr_DateTime_TimeZoneStr_GetCb;
    fpDataMgr_OPDRegulatoryInfo fpDataMgr_OPDRegulatoryInfoCb;
    fpDataMgr_LD_Info_Num_Get fpDataMgr_LD_Info_Num_GetCb;
    fpDataMgr_BLD_Num_Get fpDataMgr_BLD_Num_GetCb;
    fpDataMgr_RLD_Num_Get fpDataMgr_RLD_Num_GetCb;
}sUtilHPBUTester_CALLBACK;

void utilHPBUTest_CLI_RegCallback(sUtilHPBUTester_CALLBACK fpCallback);
/////////  callback function end  /////////


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Public functions
//unsigned char cIsTesterMode(void);
void SetTesterMode(void);
void ClearTesterMode(void);
//unsigned char cTesterDecode(char *cpComm);

UINT8 utilHPBUTest_CLI_Handle(UINT8 ucData);
void utilHPBUTest_LS_TargetValue_Get(UINT16 *Data);   //G100_Owen_0115

#ifdef UNITY_CLI
UINT8 utilHPBUTest_UNITYCmd_Decode(INT8 *cFuncIn, INT8 *cStringIn, INT8 *BufferOut);
UINT8 utilHPBUTest_UNITYCmd_GetFlag(INT8 *cHPBUCmd, UINT16 *uiSpecialFlag);
#endif // UNITY_CLI





//H2 wait review
//for compiler
//#define utilHPBUTest_CLI_Handle(a) 0
//#define SetTesterMode()
//#define utilHPBUTest_LS_TargetValue_Get(a)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
