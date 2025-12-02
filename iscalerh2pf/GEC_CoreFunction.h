/*********************************************************************************
 * FILE NAME: GEC_COREFUNCTION.H
 * DESCRIPTION:
 * Modification History
 *
 *
 * Date: 26 June, 2017
 * Author: John.Chung
*********************************************************************************/

#ifndef GEC_COREFUNCTION_H  //A70LV_John_0008 start add basic function and flow of GEC
#define GEC_COREFUNCTION_H
/*
 * File structure:
 * 1. include
 * 2. define
 * 3. enum for function usage
 * 4. GEC functions
 */

#include "GEC_UserOption.h"

#if GEC_FULL_MODE

#elif GEC_TIMELESS_MODE

#elif GEC_BRIEF_MODE
typedef enum
{
    eGEC_Print_ErrorLog2AP_INITIAL,
    eGEC_Print_ErrorLog2AP_ucIndexM,
    eGEC_Print_ErrorLog2AP_uiTime,
    eGEC_Print_ErrorLog2AP_uiErrorIndex,
    eGEC_Print_ErrorLog2AP_ucCheckSum,
    eGEC_Print_ErrorLog2AP_VALID,

    eGEC_Print_ErrorLog2AP_MAXSTEP,
}eGEC_Print_ErrorLog2AP_STATE;

typedef enum
{
    eGEC_Print_ErrorLog2Name_INITIAL,
    eGEC_Print_ErrorLog2Name_uiTime,
    eGEC_Print_ErrorLog2Name_uiErrorIndex,
    eGEC_Print_ErrorLog2Name_ucCheckSum,
    eGEC_Print_ErrorLog2Name_VALID,

    eGEC_Print_ErrorLog2Name_MAXSTEP,
}eGEC_Print_ErrorLog2Name_STATE;

#elif GEC_COMPACT_MODE

#endif

typedef enum
{
    eLittle_Endian,
    eBig_Endian,

    eInvalid_Endian
}eENDIAN;

#if 0
typedef enum
{
    rcERROR,
    rcSUCCESS,
    rcBUSY,

    rcINVALID,
} eRESULT;
#endif

#if GEC_FULL_MODE

typedef union
{
    UINT8 cAuxCfg; //GEC body M part
    struct
    {
        UINT8 ucAuxLength : 7;
        UINT8 fgAuxForm  : 1;
    } sAUX;
} uAUX_M_CFG;

typedef struct gec_errorname  //A70LV_John_0167 move GEC table to external flash
{
    UINT32 uiErrorIndex;
    UINT16 eErrorList;
    CHAR ERROREVENT[64];               //Error Type in Generic error code
    CHAR CustomizeEvent[64];           //Error Type in Customize error code  //A70LV_John_0046 add customized error code name
    UINT8 fgAuxForm;        //Error Body Part M
    UINT8 ucAuxLength;              //Error Body Part M, it does not include Time(4) and Info(5) Size
    UINT8 AUXFormat[64];               //Error Body Part X
    UINT8 ucWriteCheck;
}sGEC_ERRORTABLE;

#elif GEC_TIMELESS_MODE
#elif GEC_BRIEF_MODE

typedef union
{
    UINT8 cAuxCfg; //GEC body M part
    struct
    {
        UINT8 ucAuxLength : 7;
        UINT8 fgAuxForm  : 1;
    } sAUX;
} uAUX_M_CFG;

typedef enum
{
    eGEC_WRITETYPE_COUNTER,
    eGEC_WRITETYPE_ERRORLOG,
    eGEC_WRITETYPE_WARNINGLOG, //A70LV_John_0087 pre add warning message tag
	eGEC_WRITETYPE_REMINDER  //A70LV_John_0167 move GEC table to external flash

}eGEC_WRITETYPE;

typedef struct gec_errorname //A70LV_John_0167 start move GEC table to external flash
{
    UINT32 uiErrorIndex;
    UINT16 eErrorList;
    CHAR ERROREVENT[64];               //Error Type in Generic error code
    CHAR CustomizeEvent[64];           //Error Type in Customize error code  //A70LV_John_0046 add customized error code name
    UINT8 fgAuxForm;        //Error Body Part M
    UINT8 ucWriteCheck;
}sGEC_ERRORTABLE;  //A70LV_John_0167 end

#elif GEC_COMPACT_MODE
#endif

typedef enum
{
    eGEC_CLI_STATE_Initial,
    eGEC_CLI_STATE_Input,
    eGEC_CLI_STATE_End,

    eGEC_CLI_STATE_Invalid
}eGEC_CLI_STATE;

typedef enum
{
    eGEC_COMMAND_STATE_Initial,
    eGEC_COMMAND_STATE_Comma1,
    eGEC_COMMAND_STATE_Comma2,
    eGEC_COMMAND_STATE_Comma3,
    eGEC_COMMAND_STATE_End,

    eGEC_COMMAND_State_Invalid

}eGEC_COMMAND_STATE;

typedef enum
{
    eGEC_RS232Read_FromStart,
    eGEC_RS232Read_FromEnd,
    eGEC_RS232Read_FromSpecific,

    eGEC_OSDRead_FromStart,
    eGEC_OSDRead_FromEnd,
    eGEC_OSDRead_FromSpecific,

    eGEC_Read_Invalid,
}eGEC_ReadType;

typedef enum //G100_Steven_0014
{
    eGEC_TP_NONE = 0,
	eGEC_TP_WHITE,
	eGEC_TP_RED,
	eGEC_TP_GREEN,

    eGEC_TP_Invalid

}eGEC_TP;

//H2 wait review
//A70LK_Simon_0001
//need sync with DDP enum (refCmdI2C_hpbu.h : eCM_PICTURE_SETTINGS_ID) for mapping
typedef enum
{
#if defined (PLATFORM_H30_4K)	//for Optoma H30K
	/* 00 */ eDDP_PICTURE_SETTINGS_PRESENTATION,
    /* 01 */ eDDP_PICTURE_SETTINGS_VIDEO,
    /* 02 */ eDDP_PICTURE_SETTINGS_BRIGHT,
    /* 03 */ eDDP_PICTURE_SETTINGS_ENHANCED,
    /* 04 */ eDDP_PICTURE_SETTINGS_REC709,
    /* 05 */ eDDP_PICTURE_SETTINGS_REAL,
    /* 06 */ eDDP_PICTURE_SETTINGS_DICOMSIM,
    /* 07 */ eDDP_PICTURE_SETTINGS_2DHIGHSPEED,
    /* 08 */ eDDP_PICTURE_SETTINGS_3D,
    /* 09 */ eDDP_PICTURE_SETTINGS_BLENDING,
    /* 10 */ eDDP_PICTURE_SETTINGS_USER,
    /* 11 */ eDDP_PICTURE_SETTINGS_HDR,
    /* 12 */ eDDP_PICTURE_SETTINGS_SRGB,
    /* 13 */ eDDP_PICTURE_SETTINGS_HLG,
    /* 14 */ eDDP_PICTURE_SETTINGS_PASSIVE_3D,
#else

    /* 00 */ eDDP_PICTURE_SETTINGS_PRESENTATION,
    /* 01 */ eDDP_PICTURE_SETTINGS_VIDEO,
    /* 02 */ eDDP_PICTURE_SETTINGS_BRIGHT,
    /* 03 */ eDDP_PICTURE_SETTINGS_ENHANCED,
    /* 04 */ eDDP_PICTURE_SETTINGS_REC709,
    /* 05 */ eDDP_PICTURE_SETTINGS_REAL,
    /* 06 */ eDDP_PICTURE_SETTINGS_DICOMSIM,
    /* 07 */ eDDP_PICTURE_SETTINGS_2DHIGHSPEED,
    /* 08 */ eDDP_PICTURE_SETTINGS_3D,
    /* 09 */ eDDP_PICTURE_SETTINGS_BLENDING,
    /* 10 */ eDDP_PICTURE_SETTINGS_USER,
    /* 11 */ eDDP_PICTURE_SETTINGS_HDR,
    /* 12 */ eDDP_PICTURE_SETTINGS_PASSIVE_3D,
    ///* 13 */ eDDP_PICTURE_SETTINGS_SUPER_BRIGHT,
    ///* 14 */ eDDP_PICTURE_SETTINGS_SUPER_RED,
#endif
    eDDP_PICTURE_SETTINGS_NUMBER,
}eDDP_PICTURE_SETTINGS_ID;


typedef enum
{
#if defined (PLATFORM_H30_4K)	//for Optoma H30K
	/* 00 */  eWAP_PICTURE_SETTINGS_VIVID,
    /* 01 */  eWAP_PICTURE_SETTINGS_HDR,
    /* 02 */  eWAP_PICTURE_SETTINGS_HLG,
    /* 03 */  eWAP_PICTURE_SETTINGS_VIDEO,
    /* 04 */  eWAP_PICTURE_SETTINGS_REFERENCE,
    /* 05 */  eWAP_PICTURE_SETTINGS_BRIGHT,
    /* 06 */  eWAP_PICTURE_SETTINGS_DICOMSIM,
    /* 07 */  eWAP_PICTURE_SETTINGS_BLENDING,
    /* 08 */  eWAP_PICTURE_SETTINGS_3D,
    /* 09 */  eWAP_PICTURE_SETTINGS_2DHIGHSPEED,
    /* 10 */  eWAP_PICTURE_SETTINGS_USER,
#else
    /* 00 */  eWAP_PICTURE_SETTINGS_VIDEO,
    /* 01 */  eWAP_PICTURE_SETTINGS_BRIGHT,
    /* 02 */  eWAP_PICTURE_SETTINGS_ENHANCED,
    /* 03 */  eWAP_PICTURE_SETTINGS_REC709,
    /* 04 */  eWAP_PICTURE_SETTINGS_DICOMSIM,
    /* 05 */  eWAP_PICTURE_SETTINGS_BLENDING,
    /* 06 */  eWAP_PICTURE_SETTINGS_HDR,
    /* 07 */  eWAP_PICTURE_SETTINGS_3D,
    /* 08 */  eWAP_PICTURE_SETTINGS_2DHIGHSPEED,
    /* 09 */  eWAP_PICTURE_SETTINGS_USER,
    /* 09 */  eWAP_PICTURE_SETTINGS_3D_PASSIVE,
#endif
    eWAP_PICTURE_SETTINGS_NUMBER,
}eWAP_PICTURE_SETTINGS_ID;

////////  callback function start  ////////
typedef void (*fpSetTestPatternCallBack)(eCM_TEST_PATTERN_ID ePatternID);
typedef void (*fpCCT_StorageSet)(uCCT_TABLE *psEEP);
typedef void (*fpCCT_StorageGet)(uCCT_TABLE *psEEP);

typedef struct
{
    fpSetTestPatternCallBack fpSetTpCallback;
    fpCCT_StorageSet fpCCT_StorageSetCallback;
    fpCCT_StorageGet fpCCT_StorageGetCallback;
}sGEC_CALLBACK;

void GEC_RegCallback(sGEC_CALLBACK fpCallback);
/////////  callback function end  /////////


void GEC_DebugMode_Set(UINT8 ucvalue);
UINT8 GEC_DebugMode_Get(void);
void GEC_EEPROM_Modify(UINT32 uiPosition, UINT8 uiData);
void GEC_EEPROM_Print(UINT32 uiData, UINT32 uiLength);
void GEC_EEPROM_InitialCheck(void);
eRESULT GEC_ErrorCode_EraseAll(void);
//UINT8 GEC_ErrorCode_WriteCheck(eERROR_LIST eErrorList);
eENDIAN GEC_Endian_Check(void);
eRESULT GEC_Decode_ErrorLog_APString(UINT8 ucSubID, UINT8 ucLength, UINT8 *pucAuxIn, char* cString);
eRESULT GEC_Decode_ErrorLog_NameString(UINT8 ucSubID, UINT8 ucLength, UINT8 *pucAuxIn, char *cString);
//eRESULT GEC_Show_ErrorLog_Name(eGEC_ReadType eReadType, UINT16 uiReadNumber, UINT8 *pucAuxIn,CHAR *cString); //TBD
eRESULT GEC_ReadAll_to_AP(void);
static void utilGEC_Low2UpCase(UINT8 *pcData);
//void GetErrorCode(eERRORCODE_READ_TYPE ReadType, sCLI_INDEX *sCLI);
void GEC_GetGecTable(void);
eRESULT GEC_GetErrorCount(void);
UINT16 GEC_GetTotalErrorCount(void);  //A70LV_John_0064 fix err reply message
void GEC_GetCommonInfo(UINT16 subID, UINT16 CommonInfo);
void GEC_CLI_Command_Execute(sCLI_INDEX *sCLI);
void GEC_CLI_SubIndex_Convert(UINT8 *pucCOMMAND, UINT8 ucLength);
void GEC_CommandProcess_Set(UINT8 ucEnable);
UINT8 GEC_CommandProcess_Get(void);
void utilDataMgr_WriteGecLog(UINT32 ulIndex);
UINT32 GEC_WAP_Flag_Get(void); //G50_Casper_0006
void GEC_WAP_Flag_Set(UINT32 uFlag); //G50_Casper_0006
eCM_PICTURE_SETTINGS_ID GEC_MapToCM_PicMode(eWAP_PICTURE_SETTINGS_ID ePicMode);
UINT16 GEC_Limit_PWM(UINT16 uiPWM);

#endif  //A70LV_John_0008 end
