// ===============================================================================
// FILE NAME: utilCommonMSSCAPI.h
// DESCRIPTION:
//
//
// Modification History
// --------------------
// 2021/03/24, Larry Create
// --------------------
// ===============================================================================

#ifndef UTILCOMMONMSSCAPI_H
#define UTILCOMMONMSSCAPI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "Common.h"


#if defined(CUSTOM_CHRISTIE)
#include "utilChristieMSSCAPI.h"
#elif defined(CUSTOM_OPTOMA)
#include "utilOptomaMSSCAPI.h"
#ifdef GENERAL_MSSC_CLI
#include "utilGeneralMSSCAPI.h"
#endif /* GENERAL_MSSC_CLI */
#else
#include "utilGeneralMSSCAPI.h"
#endif

//#include "utilGeneralMSSCAPI.h"

#define MSSC_CMD_LENGTH_MAX     (230)

#define CLI_DATA_SIZE           (255)

#define DATATYPE_NO_DATA        0
#define DATATYPE_DIGIT          1
#define DATATYPE_DIGIT_UP_DOWN  2
#define DATATYPE_STRING         3
#define DATATYPE_ALWAYS_ON      4

#define WRITE_COMMAND   (0)
#define READ_COMMAND    (1)

#define DECIAML_2   2  //(%02d)
#define DECIAML_3   3  //(%03d)
#define DECIAML_4   4  //(%04d)
#define DECIAML_5   5
#define STRING_CHAR 8

#define CLI_COM_READ_ONLY               (0x0001 << 1)
#define CLI_COM_WRITE_ONLY              (0x0001 << 2)
#define CLI_COM_NEED_SUB_CODE           (0x0001 << 3)
#define CLI_COM_NEED_SERVICE_CODE_WRITE (0x0001 << 4)
#define CLI_COM_NEED_SERVICE_CODE_READ  (0x0001 << 5)
#define CLI_COM_TYPE_PN_ONLY            (0x0001 << 6)
#define CLI_COM_NO_REPLY                (0x0001 << 7) //Special REPLY
#define CLI_COM_IGNORE_CTRL_CHECK       (0x0001 << 8)
#define CLI_COM_TYPE_SUPPORT_PN         (0x0001 << 9)
#define CLI_COM_CHECK_BUSY              (0x0001 << 10)
#define CLI_COM_ENABLE                  (0x0001 << 11)
#define CLI_COM_DEC2STRING              (0x0001 << 12)

#define SWGEC_CMD_HEADER              '<'
#define SWGEC_CMD_END                 '>'

#define HPBUTEST_START_HEADER       '*'
#define HPBUTEST_END_HEADER_1       '\n'
#define HPBUTEST_END_HEADER_2       '\r'

#define CLI_EXECTUE_READ_CMD        (0x0001 << 1)
#define CLI_EXECTUE_WRITE_CMD       (0x0001 << 2)
#define CLI_EXECTUE_WRITE_BUFFER    (0x0001 << 3)
#define CLI_EXECTUE_READ_BUFFER     (0x0001 << 4)
#define CLI_EXECTUE_NORMAL          (CLI_EXECTUE_READ_CMD | CLI_EXECTUE_WRITE_CMD)
/*
typedef enum
{
    eCLI_STYLE_CHRISTIE,
    eCLI_STYLE_GENERAL,
    eCLI_STYLE_OPTOMA,

    eCLI_STYLE_NUMBER,
}eCLI_STYLE;//A35G2_Coda_0045
*/
typedef enum
{
    ecmWrite,
    ecmRead,

    ecmNumber,
}eCLI_MODE;

typedef enum
{
    eccPC,
    eccTelnet,
    eccHDBaseT,

    eccNumber,
}eCLI_CHANNEL;

typedef enum
{
    eCLI_DECODE_AMX_STEP_A = 0,
    eCLI_DECODE_AMX_STEP_M,
    eCLI_DECODE_AMX_STEP_X,
    eCLI_DECODE_AMX_STEP_END,

}eCLI_DECODE_AMX_STEP;

typedef enum
{
    /*00*/ eCLI_ERROR_CODE_NO = 0,
    /*01*/ eCLI_ERROR_CODE_LENGTH,
    /*02*/ eCLI_ERROR_CODE_DESTADDERR,
    /*03*/ eCLI_ERROR_CODE_SRCADDERR,
    /*04*/ eCLI_ERROR_CODE_FUNCCODENOTFOUND,
    /*05*/ eCLI_ERROR_CODE_FORMATERR,
    /*06*/ eCLI_ERROR_CODE_NEGATIVEERR,
    /*07*/ eCLI_ERROR_CODE_DECIMALERR,
    /*08*/ eCLI_ERROR_CODE_STRINGERR,
    /*09*/ eCLI_ERROR_CODE_STRINGTOOBIG,
    /*10*/ eCLI_ERROR_CODE_DATAERROR,
    /*11*/ eCLI_ERROR_CODE_CKSUMNPSPACE,
    /*12*/ eCLI_ERROR_CODE_CKSUMERR,
    /*13*/ eCLI_ERROR_CODE_MessageTypeError,
    //data check code.
    /*14*/ eCLI_ERROR_CODE_DATAOVERRANGE, //ok
    /*15*/ eCLI_ERROR_CODE_LookupIndexErr, //ok
    /*16*/ eCLI_ERROR_CODE_DataMustToBeDecimal, //ok
    /*17*/ eCLI_ERROR_CODE_DataMustToBeString, //ok
    /*18*/ eCLI_ERROR_CODE_MUSTDEPENDONSRC,
    /*19*/ eCLI_ERROR_CODE_FunctionNoDefine, // ok
    /*20*/ eCLI_ERROR_CODE_DataSpecialCheckFail, //ok
    /*21*/ eCLI_ERROR_CODE_SpecialSourceDependCheckFail,// ok
    //set,request,reply
    /*22*/ eCLI_ERROR_CODE_SETFAIL,   //ok
    /*23*/ eCLI_ERROR_CODE_REQUESTFAIL,  //ok
    /*24*/ eCLI_ERROR_CODE_ReplyFail,
    /*25*/ eCLI_ERROR_CODE_SUBCODEERR,
    /*26*/ eCLI_ERROR_CODE_COMMAND_READ_ONLY,
    /*27*/ eCLI_ERROR_CODE_COMMAND_WRITE_ONLY,
    /*28*/ eCLI_ERROR_CODE_SUBCODENEED,
    /*29*/ eCLI_ERROR_CODE_SERVICE_CODE_NEED_WHEN_WRITE,
    /*30*/ eCLI_ERROR_CODE_SERVICE_CODE_NEED_WHEN_READ,

    /*31*/ eCLI_ERROR_CODE_CUart_N_NOSPACE,
    /*32*/ eCLI_ERROR_CODE_CUart_P_NOSPACE,

	/*33*/eCLI_ERROR_CODE_CUart_U_NOSPACE,  //HICC2_Steven_0056  start
	/*34*/eCLI_ERROR_CODE_CUart_D_NOSPACE,
	/*35*/eCLI_ERROR_CODE_CUart_R_NOSPACE,
	/*36*/eCLI_ERROR_CODE_CUart_L_NOSPACE,  //HICC2_Steven_0056  end

    eCLI_ERROR_NUMBER,
}eCLI_ERROR_CODE;


typedef enum
{
    eCLI_DATA_TYPE_UNKNOW   = 0,
    eCLI_DATA_TYPE_DEC      = 1,
    eCLI_DATA_TYPE_STRING   = 2,
    eCLI_DATA_TYPE_DONTCARE = 3,
    eCLI_DATA_TYPE_N        = 4, //Next
    eCLI_DATA_TYPE_P        = 5, //Pre.
    eCLI_DATA_TYPE_POINT    = 6,

    eCLI_DATA_TYPE_D        = 20, //HICC2_Steven_0056 start
    eCLI_DATA_TYPE_D_STOP   = 21,
    eCLI_DATA_TYPE_D_STEP   = 22,
    eCLI_DATA_TYPE_D_RUN    = 23,
    eCLI_DATA_TYPE_U        = 24, //Pre.
    eCLI_DATA_TYPE_U_STOP   = 25,
    eCLI_DATA_TYPE_U_STEP   = 26,
    eCLI_DATA_TYPE_U_RUN    = 27,

    eCLI_DATA_TYPE_R        = 28, //Next
    eCLI_DATA_TYPE_R_STOP   = 29,
    eCLI_DATA_TYPE_R_STEP   = 30,
    eCLI_DATA_TYPE_R_RUN    = 31,
    eCLI_DATA_TYPE_L        = 32, //Pre.
    eCLI_DATA_TYPE_L_STOP   = 33,
    eCLI_DATA_TYPE_L_STEP   = 34,
    eCLI_DATA_TYPE_L_RUN    = 35,  //HICC2_Steven_0056 end

    eCLI_DATA_TYPE_N_STOP   = 40,
    eCLI_DATA_TYPE_N_STEP   = 41,
    eCLI_DATA_TYPE_N_RUN    = 42,
    eCLI_DATA_TYPE_P_STOP   = 50,
    eCLI_DATA_TYPE_P_STEP   = 51,
    eCLI_DATA_TYPE_P_RUN    = 52,
    eCLI_DATA_TYPE_OUTPUT_STRING   = 60,//ZU860_Doulas_0113
}eCLI_DATA_TYPE;

typedef enum
{
    eCLI_PREFIX_CHAR_NO = 0,
    eCLI_PREFIX_CHAR_SIMPLE_ACK,
    eCLI_PREFIX_CHAR_FULL_SUM,
    eCLI_PREFIX_CHAR_CHECKSUM,

}eCLI_PREFIX_CHAR;

typedef struct
{
    eCLI_MODE eAccessMode;
    UINT8 ucDataType;
    UINT8 ucCmdFrom;
    char  cTextString[CLI_DATA_SIZE + 1];
    INT32 lData;
    float fData;
}sCLI_COMMON_FORMAT;

typedef struct
{
    UINT8 cCh;
    UINT8 cStyle;
    char  cString[CLI_DATA_SIZE + 1];
}sCLI_BUFFER_FORMAT;

typedef struct
{
    UINT8 ucUartCh;
    char  cCMD_Hander;
    char  cCMD_End;

}sCLI_CONFIG;

typedef struct
{
    UINT8 ucInterFace;//PC,LAN
    UINT8 ucCLI_DATA[CLI_DATA_SIZE + 1];
    UINT16 uiCLI_Front;
    UINT16 uiCLI_End;
    UINT16 uiCLI_Poll_Position;
    UINT16 uiCLI_Length;
    UINT8  ucSerialPortEcho;
}sCLI_DATA_INFO;

typedef void (*fpCLISTRINGCALLBACK)(UINT8 cCh, UINT8 cStyle, UINT8 * pcBuffer, UINT16 uiSize);	//A35G2_Coda_0045
void utilCommon_Init(void);
UINT8 __CheckCmd(char *cSrcA, char *cDest, UINT8 cuLens);
UINT8 __CheckNum(char cCharNum);
UINT8 __CheckText(char cCharNum);
UINT8 __CheckAsciiCharacter(char cCharNum);
void __CMD_Respond(UINT8 ucCh, char *pstring);
void Low2UpCase(UINT8 *pcData);
eRESULT Hex2Bin(UINT8 *pcData);
eRESULT AscIIToHex(UINT8 ucHighByte, UINT8 ucLowByte, UINT8 *pucData);
INT8 utilCommonCLI_DataControl(UINT16 uiDataCode);
INT8 utilCommonCLI_ControlFunction(UINT16 uiControl);
UINT8 utilCommonCLI_DataTypeGet(UINT16 uiDataCode);
INT8 utilCommonCLI_String_Get(UINT16 uiDataCode, UINT8 *pucString);
INT8 utilCommonCLI_String_Set(UINT16 uiDataCode, UINT8 *pucString);
INT8 utilCommonCLI_DataConversionGet(UINT16 uiDataCode, INT32 *piValue);
eEXEC_CODE utilCommonCLI_DataConversionSet(UINT16 uiDataCode, INT32 iValue);
INT8 utilCommonCLI_RangeCheck(UINT16 uiDataCode, INT32 iValue);
INT8 utilCommonCLI_Increase_Data(UINT16 uiDataCode);
INT8 utilCommonCLI_Decrease_Data(UINT16 uiDataCode);
void utilCommon_Callback_fun(fpCLISTRINGCALLBACK fpCallBack);
void utilCommon_CLI_Callback(UINT8 cCh, UINT8 cStyle, UINT8 * pcBuffer, UINT16 uiSize);	//A35G2_Coda_0045


//Common CLI
eCLI_ERROR_CODE utilCommonCLI_SYSDBMK(sCLI_COMMON_FORMAT* sCmdFormat);
eCLI_ERROR_CODE utilCommonCLI_ICP(sCLI_COMMON_FORMAT* sCmdFormat);
eCLI_ERROR_CODE utilCommonCLI_ICPC821_SET(sCLI_COMMON_FORMAT* sCmdFormat);
eCLI_ERROR_CODE utilCommonCLI_ICPC821_GET(sCLI_COMMON_FORMAT* sCmdFormat);
eCLI_ERROR_CODE utilCommonCLI_ICPC789_SET(sCLI_COMMON_FORMAT* sCmdFormat);
eCLI_ERROR_CODE utilCommonCLI_ICPC789_GET(sCLI_COMMON_FORMAT* sCmdFormat);
eCLI_ERROR_CODE utilCommonCLI_ICPC341_SET(sCLI_COMMON_FORMAT* sCmdFormat);
eCLI_ERROR_CODE utilCommonCLI_ICPC341_GET(sCLI_COMMON_FORMAT* sCmdFormat);
eCLI_ERROR_CODE utilCommonCLI_PRO(sCLI_COMMON_FORMAT* sCmdFormat);
eCLI_ERROR_CODE utilCommonCLI_PROSETT(sCLI_COMMON_FORMAT* sCmdFormat);
eCLI_ERROR_CODE utilCommonCLI_PROGETT(sCLI_COMMON_FORMAT* sCmdFormat);

eCLI_ERROR_CODE utilCommonCLI_BODCUST(sCLI_COMMON_FORMAT* sCmdFormat);
eCLI_ERROR_CODE utilCommonCLI_BODPLAT(sCLI_COMMON_FORMAT* sCmdFormat);
eCLI_ERROR_CODE utilCommonCLI_TTT(sCLI_COMMON_FORMAT* sCmdFormat);
eCLI_ERROR_CODE utilCommonCLI_ShowOSDBitmapRawData(sCLI_COMMON_FORMAT* sCmdFormat);  //A35G2_Simon_0116
eCLI_ERROR_CODE utilCommonCLI_DecToString(UINT16 uiDataCode, INT32 iValue, char* cString);
eCLI_ERROR_CODE utilCommonCLI_ScheduleParseEvent_to_Rs232(UINT8 * ucToParseStr, UINT32 * uiEventInfo);
UINT8 utilCommonCLI_SWGEC_Handle(UINT8 eCh, UINT8 *pcData);
UINT8 utilCommonCLI_SWGEC_Telnet_Handle(UINT8 *pcData);


#endif /* UTILCOMMONMSSCAPI_H */



