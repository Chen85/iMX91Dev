#if defined(CUSTOM_CHRISTIE)
// ===============================================================================
// FILE NAME: utilChristieMSSC.c
// DESCRIPTION:
//
//
// Modification History
// --------------------
// 2017/10/08, Larry Create
// --------------------
// ===============================================================================

#include "Board_Uart.h"
#include "appIllumination.h"
//#include "appGui.h"
#include "appSystem.h"
#include "appDataMgr.h"
#include "appInputProc.h"
#include "appDataPath.h"
#include "appLANProcAPI.h"
#include "appEnvironment.h"
#include "halMotorCtrlAPI.h"
#include "halInputCtrlAPI.h"
#include "halBoardCtrlAPI.h"
#include "halLDProc.h"
#include "halFormatter.h"
#ifdef SCALER_C821_C789
#include "halC789CtrlAPI.h"
#endif
#include "halMCUCtrlAPI.h"
#include "halScaler.h"
#include "halFrontEndCtrlAPI.h"
#include "halFanCtrlAPI.h"

#include "palImgMgr.h"
#include "palGeoAPI.h"
#include "palFormatterMgr.h"
#include "palMotorMgr.h"

#include "utilCommon.h"
#include "utilDbgMsg.h"
#include "utilOPD_TEST.h"
#include "utilCommonMSSCAPI.h"
#include "utilCLICmdAPI.h"
#include "utilDataMgrAPI.h"
#include "utilDatabaseAPI.h"
#include "utilDataMapping.h"
#include "CommonAPI.h"

#include "dvFPGA10M04.h"
#include "opdCtrlAPI.h"
#include "CustomDef.h"
#include "CustomDefCLI.h"
#include "ItemConfigAccess.h"

#if(BIST_ENABLE)
#include "utilBIST.h" //HICC2_Steven_0001
#endif

#define CHRISTIE_CMD_REQUEST        '?'
#define CHRISTIE_CMD_REPLY          '!'
#define CHRISTIE_CMD_SPACE          ' '

#define CHRISTIE_CMD_DIS_CHAR_1     '\\'

#define CHRISTIE_CMD_RPR_CHAR_SIMPLE_ACK    '$'
#define CHRISTIE_CMD_RPR_CHAR_FULL_ACK      '#'
#define CHRISTIE_CMD_RPR_CHAR_CHECK_SUM     '&'

#define CHRISTIE_CMD_DATA_P  'P'
#define CHRISTIE_CMD_DATA_N  'N'
#define CHRISTIE_CMD_DATA_p  'p'
#define CHRISTIE_CMD_DATA_n  'n'

#define CHRISTIE_CMD_DATA_U  'U' 		//HICC2_Steven_0056 start
#define CHRISTIE_CMD_DATA_D  'D'
#define CHRISTIE_CMD_DATA_u  'u'
#define CHRISTIE_CMD_DATA_d  'd'

#define CHRISTIE_CMD_DATA_R  'R'
#define CHRISTIE_CMD_DATA_L  'L'
#define CHRISTIE_CMD_DATA_r  'r'
#define CHRISTIE_CMD_DATA_l  'l'        //HICC2_Steven_0056 end
#define CHRISTIE_CMD_DATA_STRING    '"'
#define CHRISTIE_CMD_SUB_CONNECT    '+'
#define CHRISTIE_CMD_LENGTH_MAX     (230)
#define CHRISTIE_CMD_LENGTH_MIN     (6) //(SYS?)\0

#define CLI_COM_NORMAL              (0)
#define CLI_COM_NORMAL_AND_PN       CLI_COM_NORMAL | CLI_COM_TYPE_SUPPORT_PN
#define CLI_COM_LENS                CLI_COM_NORMAL | CLI_COM_TYPE_PN_ONLY | CLI_COM_WRITE_ONLY | CLI_COM_TYPE_SUPPORT_PN | CLI_COM_NO_REPLY

#define CHRISTIE_MSSC_VER           1

#define DATA_CODE_NA (0xFFFF)

#define DDY_CMD "DDY"
#define DDM_CMD "DDM"
#define DDD_CMD "DDD"
#define DTH_CMD "DTH"
#define DTM_CMD "DTM"
#define DST_CMD "DST"
#define NTP_CMD "NTP"
#define UTC_CMD "UTC"
#define DUI_CMD "DUI"

typedef enum
{
    eCLI_DECODE_STEP_HEADER = 0,
    eCLI_DECODE_STEP_PRE_CHAR_CHECK, //PREFIX_CHAR check
    eCLI_DECODE_STEP_ADDRESS_CHECK,  //Projector Host check
    eCLI_DECODE_STEP_MAIN,
    eCLI_DECODE_STEP_SUB,
    eCLI_DECODE_STEP_DATA_FORMAT,
    eCLI_DECODE_STEP_END,

}eCLI_DECODE_STEP;

typedef struct _Uart_ErrMsg
{
    UINT8 ucErrCode;
    char* pcErrorMsg;
}sCLI_ERROR_MSG;


typedef struct
{
    UINT16 uiMainCmdID;
    char* cSubCmd;

    UINT8 ucCmdIsRead;
    UINT8 ucCmdBytesOnRange;

    UINT16 uiFunCodeID;
    eCLI_ERROR_CODE (*iCLI_Replay)(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat);

    UINT16 uiSpecialFlag; //read only, write only,...

}sCLI_SUB_LUT;

typedef struct
{
    UINT16 uiMainCmdID;
    char* cMainCmd;

}sCLI_MAIN_LUT;

static UINT8  m_ucWriteBusy = 0;

// ==============================================================================
// FUNCTION NAME: m_sCli_Config
// DESCRIPTION:
//
//
// Params:
//
// Returns:
//
//
// Modification History
// --------------------
// 2021/03/24, Larry Create
// --------------------
// ==============================================================================
const static sCLI_CONFIG m_sCli_Config[eccNumber] =
{
    {eccPC,         CHRISTIE_CMD_HEADER_1, CHRISTIE_CMD_END_1},
    {eccTelnet,     CHRISTIE_CMD_HEADER_1, CHRISTIE_CMD_END_1},  //A70LV_Larry_0258
    {eccHDBaseT,    CHRISTIE_CMD_HEADER_1, CHRISTIE_CMD_END_1},  //A70LV_Larry_0258
};

const static sCLI_ERROR_MSG m_sErrorLut[eCLI_ERROR_NUMBER] =
{

    /*00*/{119,   "Syntax Error: Unknow Error"},  // CUart_NoError
    /*01*/{10,    "Syntax Error: Invalid control"}, // CUart_LengthError
    /*02*/{10,    "Syntax Error: dest address out of range:"}, // CUart_DestAddError
    /*03*/{10,    "Syntax Error: src address out of range:"}, //CUart_SrcAddError
    /*04*/{101,   "Control Not Found"}, //CUart_FuncCodeNotFound
    /*05*/{10,    "Syntax Error: invalid dest address:"}, //CUart_FormatErr
    /*06*/{10,    "Syntax Error: invalid code"}, //CUart_negativeErr
    /*07*/{104,   "Invalid Value"}, //CUart_decimalErr
    /*08*/{108,   "Exceeded Text Size"}, //CUart_StringErr
    /*09*/{108,   "Exceeded Text Size"}, //CUart_StringTooBig
    /*10*/{5,     "Too Few Parameters"}, //CUart_DataError
    /*11*/{8,     "Checksum error"}, //CUart_CKSumNoSpace
    /*12*/{8,     "Checksum error"}, //CUart_CKSumError
    /*13*/{119,   "[fw]Message Type Error"}, //CUart_MessageTypeError
    /*14*/{104,   "Invalid Value"}, //CUart_DataOverRange
    /*15*/{119,   "[fw]Lookup Index Error"}, //CUart_LookupIndexErr
    /*16*/{3,     "Invalid Parameter"}, //CUart_DataMustToBeDecimal
    /*17*/{3,     "Invalid Parameter"}, //CUart_DataMustToBeString
    /*18*/{105,   "Disabled Control"}, //CUart_MustDependOnSrc
    /*19*/{119,   "[fw]Function no define"}, //CUart_FunctionNoDefine
    /*20*/{119,   "[fw]Data Special check Fail"}, //CUart_DataSpecialCheckFail
    /*21*/{119,   "[fw]Data Special src check Fail"}, //CUart_SpecialSourceDependCheckFail
    /*22*/{105,   "Disabled Control"}, //CUart_SetFail
    /*23*/{119,   "[fw]Request Fail"}, //CUart_RequestFail
    /*24*/{119,   "[fw]ReplyFail Fail"}, //CUart_ReplyFail
    /*25*/{102,   "Subcontrol Not Found"}, //CUart_SubCodeErr
    /*26*/{7,     "Command not executed"}, //CUart_ComReadOnly
    /*27*/{7,     "Command not executed"}, //CUart_ComWriteOnly
    /*28*/{102,   "Subcontrol Not Found"}, //CUart_SubCodeNeed
    /*29*/{110,   "Function protected"}, //CUart_ServiceCodeNeedWhenWrite
    /*30*/{110,   "Function protected"}, //CUart_ServiceCodeNeedWhenRead
    /*31*/{10,    "Syntax Error: N"}, //CUart_N_NoSpace
    /*32*/{10,    "Syntax Error: P"}, //CUart_P_NoSpace

    /*33*/{10,    "Syntax Error: U"}, 	                        /*eCLI_ERROR_CODE_CUart_U_NOSPACE */ //HICC2_Steven_0056start
    /*34*/{10,    "Syntax Error: D"}, 	                        /*eCLI_ERROR_CODE_CUart_D_NOSPACE */
    /*35*/{10,    "Syntax Error: R"}, 	                        /*eCLI_ERROR_CODE_CUart_R_NOSPACE */
    /*36*/{10,    "Syntax Error: L"}, 	                        /*eCLI_ERROR_CODE_CUart_L_NOSPACE */ //HICC2_Steven_0056 end

};

static UINT8 ucAMX_Step = eCLI_DECODE_AMX_STEP_A;

typedef enum
{
    eCLI_MAIN_SYS,
    eCLI_MAIN_SZP,
    eCLI_MAIN_OVS,
    eCLI_MAIN_PXT,
    eCLI_MAIN_PXP,
    eCLI_MAIN_HOR,
    eCLI_MAIN_VRT,
    eCLI_MAIN_DZH,
    eCLI_MAIN_DZV,
    eCLI_MAIN_DSH,
    eCLI_MAIN_DSV,
    eCLI_MAIN_PCM,
    eCLI_MAIN_GME,
    eCLI_MAIN_WRP,
    eCLI_MAIN_AWF,
    eCLI_MAIN_MWF,
    eCLI_MAIN_HPC,
    eCLI_MAIN_VPC,
    eCLI_MAIN_CNR,
    eCLI_MAIN_WRE,
    eCLI_MAIN_AIG,
    eCLI_MAIN_BRT,
    eCLI_MAIN_CON,
    eCLI_MAIN_CSP,
    eCLI_MAIN_DTL,
    eCLI_MAIN_TDM,
    eCLI_MAIN_TDE,
    eCLI_MAIN_TDI,
    eCLI_MAIN_SIS,
    eCLI_MAIN_SOS,
    eCLI_MAIN_FDY,
    eCLI_MAIN_SDY,
    eCLI_MAIN_CLR,
    eCLI_MAIN_TNT,
    eCLI_MAIN_NDR,
    eCLI_MAIN_FTC,
    eCLI_MAIN_VBL,
    eCLI_MAIN_FMD,
    eCLI_MAIN_CLC,
    eCLI_MAIN_ROG,
    eCLI_MAIN_GOG,
    eCLI_MAIN_BOG,
    eCLI_MAIN_ROO,
    eCLI_MAIN_GOO,
    eCLI_MAIN_BOO,
    eCLI_MAIN_SYT,
    eCLI_MAIN_GOR,
    eCLI_MAIN_PST,
    eCLI_MAIN_PSU,
    eCLI_MAIN_FPS,
    eCLI_MAIN_DIM,
    eCLI_MAIN_FRZ,
    eCLI_MAIN_BGC,
    eCLI_MAIN_WPK,
    eCLI_MAIN_CCI,
    eCLI_MAIN_EDG,
    eCLI_MAIN_CWS,
    eCLI_MAIN_HSG,
    eCLI_MAIN_VBD,
    eCLI_MAIN_LOC,
    eCLI_MAIN_FCS,
    eCLI_MAIN_ZOM,
    eCLI_MAIN_LVO,
    eCLI_MAIN_LHO,
    eCLI_MAIN_LVU,
    eCLI_MAIN_LVD,
    eCLI_MAIN_LHL,
    eCLI_MAIN_LHR,
    eCLI_MAIN_LMR,
    eCLI_MAIN_LMA,
    eCLI_MAIN_LMS,
    eCLI_MAIN_LCB,
    eCLI_MAIN_CEL,
    eCLI_MAIN_SOR,
    eCLI_MAIN_LED,
    eCLI_MAIN_MSH,
    eCLI_MAIN_MSV,
    eCLI_MAIN_MBE,
    eCLI_MAIN_OST,
    eCLI_MAIN_SPS,
    eCLI_MAIN_PIV,
    eCLI_MAIN_PCG,
    eCLI_MAIN_PWR,
    eCLI_MAIN_APW,
    eCLI_MAIN_ASH,
    eCLI_MAIN_SLP,
    eCLI_MAIN_HAT,
    eCLI_MAIN_IRC,
    eCLI_MAIN_VTT,
    eCLI_MAIN_NET,
    eCLI_MAIN_NTW,
    eCLI_MAIN_BDR,
    eCLI_MAIN_SEC,
    eCLI_MAIN_SPP,
    eCLI_MAIN_ADR,
    eCLI_MAIN_KBL,
    eCLI_MAIN_SBL,
    eCLI_MAIN_CCA,
    eCLI_MAIN_MDR,
    eCLI_MAIN_TDR,
    eCLI_MAIN_MAR,
    eCLI_MAIN_HAR,
    eCLI_MAIN_HKS,
    eCLI_MAIN_PIF,
    eCLI_MAIN_TPG,
    eCLI_MAIN_DEF,
    eCLI_MAIN_ITP,
    eCLI_MAIN_FTP,
    eCLI_MAIN_CWI,
    eCLI_MAIN_ERR,
    eCLI_MAIN_MDT,
    eCLI_MAIN_LDI,
    eCLI_MAIN_FAN,
    eCLI_MAIN_UST,
    eCLI_MAIN_LPM,
    eCLI_MAIN_LPP,
    eCLI_MAIN_LPI,
    eCLI_MAIN_LIF,
    eCLI_MAIN_RLC,
    eCLI_MAIN_SST,
    eCLI_MAIN_SIN,
    eCLI_MAIN_PIP,
    eCLI_MAIN_PPS,
    eCLI_MAIN_PHS,
    eCLI_MAIN_PPP,
    eCLI_MAIN_TMG,
    eCLI_MAIN_MIF,
    eCLI_MAIN_SIF,
    eCLI_MAIN_SKS,
    eCLI_MAIN_BSS,
    eCLI_MAIN_UID,
    eCLI_MAIN_SIV,
    eCLI_MAIN_LCE,
    eCLI_MAIN_LSE,
    eCLI_MAIN_SNS,
    eCLI_MAIN_KEY,
    eCLI_MAIN_SHU,
    eCLI_MAIN_FCT,
    eCLI_MAIN_OSD,
    eCLI_MAIN_NRD,
    eCLI_MAIN_TTT,
    eCLI_MAIN_CSS,
    eCLI_MAIN_CFU,
    eCLI_MAIN_EXF,
    eCLI_MAIN_BUR,
    eCLI_MAIN_WAP,
    eCLI_MAIN_WAS,
    eCLI_MAIN_BLD,
    eCLI_MAIN_BLS,
    eCLI_MAIN_EBL,
    eCLI_MAIN_SCR,
    eCLI_MAIN_TNR,
    eCLI_MAIN_MNR,
    eCLI_MAIN_MTO,
    eCLI_MAIN_ICI,
    eCLI_MAIN_ILI,

    eCLI_MAIN_ICP,
    eCLI_MAIN_ICD,
    eCLI_MAIN_FGA,
    eCLI_MAIN_ADC,
    eCLI_MAIN_LET,
    eCLI_MAIN_LRR,
    eCLI_MAIN_FVI,
    eCLI_MAIN_STY,
    eCLI_MAIN_LOT,
    eCLI_MAIN_LOL,
    eCLI_MAIN_FKC,
    eCLI_MAIN_CCT,
    eCLI_MAIN_BAC,

    eCLI_MAIN_SPD,
    eCLI_MAIN_STH,
    eCLI_MAIN_DLL,
    eCLI_MAIN_DRB,

    eCLI_MAIN_TCM,
    eCLI_MAIN_DDY,
    eCLI_MAIN_DDM,
    eCLI_MAIN_DDD,
    eCLI_MAIN_DTH,
    eCLI_MAIN_DTM,
    eCLI_MAIN_DST,
    eCLI_MAIN_NTP,
    eCLI_MAIN_UTC,
    eCLI_MAIN_DUI,
    eCLI_MAIN_DTS,

    eCLI_MAIN_SDT,
    eCLI_MAIN_SD0,
    eCLI_MAIN_SD1,
    eCLI_MAIN_SD2,
    eCLI_MAIN_SD3,
    eCLI_MAIN_SD4,
    eCLI_MAIN_SD5,
    eCLI_MAIN_SD6,

    eCLI_MAIN_SCH,
    eCLI_MAIN_HDR,
    eCLI_MAIN_EDI,

    eCLI_MAIN_DBS,
    eCLI_MAIN_RBS,
    eCLI_MAIN_CER,
    eCLI_MAIN_EQM,

    eCLI_MAIN_AIR,
    eCLI_MAIN_PBR,
    eCLI_MAIN_PCO,
    eCLI_MAIN_PCS,
    eCLI_MAIN_LLM,

    eCLI_MAIN_BKI,
    eCLI_MAIN_IST,

    eCLI_MAIN_HMO,
    eCLI_MAIN_OPD,
    eCLI_MAIN_TAR,

	eCLI_MAIN_CST,	//A70Gen2_Doulas_0043

    eCLI_MAIN_ATF,    //A35G2_CDS_Coda_0027
    eCLI_MAIN_ATW,

    #if (ENABLE_COLOR_UNIFORMITY == TRUE) //G100_Tim_0012, add, start
	eCLI_MAIN_ACE,					  // ACU Enable
	eCLI_MAIN_ACC,					  // Execute ACU
	eCLI_MAIN_ACS,					  // Access ACU status
	eCLI_MAIN_ACT,					  // Select ACU target for calibration
	eCLI_MAIN_ACR,					  // Reset ACU data and status
	//eCLI_MAIN_ACUW, 					// save ACU data
    #endif //ENABLE_COLOR_UNIFORMITY	  //G100_Tim_0012, add, end

    eCLI_MAIN_BOD,
    eCLI_MAIN_PLT,
    eCLI_MAIN_LTL,		//A70Gen2_Doulas_0044
    eCLI_MAIN_WAE,
    eCLI_MAIN_DBG,
    eCLI_MAIN_STP,      //HICC2_Doulas_0071
    eCLI_MAIN_FDI,      //HICC2_Doulas_0071
    eCLI_MAIN_FDO,      //HICC2_Doulas_0071

#if(BIST_ENABLE) //HICC2_Steven_0001
	eCLI_MAIN_BST,
#endif
	eCLI_MAIN_OPP,  //HICC2_Steven_0009

    eCLI_MAIN_PRO,
    eCLI_MAIN_CSL,

    eCLI_MAIN_RSP,
    eCLI_MAIN_RIS,
    eCLI_MAIN_RCF,
    eCLI_MAIN_LSF,
	eCLI_MAIN_ATD,
	eCLI_MAIN_GUP,


    eCLI_MAIN_END,
}eCLI_MAIN_LIST;

// ==============================================================================
// FUNCTION NAME: m_sMainLut
// DESCRIPTION:
//
//
// Params:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/11/08, Larry Create
// --------------------
// ==============================================================================
const static sCLI_MAIN_LUT m_sMainLut[] =
{
    {eCLI_MAIN_SYS,  "SYS"},
    {eCLI_MAIN_SZP,  "SZP"},
    {eCLI_MAIN_OVS,  "OVS"},
    {eCLI_MAIN_PXT,  "PXT"},
    {eCLI_MAIN_PXP,  "PXP"},
    {eCLI_MAIN_HOR,  "HOR"},
    {eCLI_MAIN_VRT,  "VRT"},
    {eCLI_MAIN_DZH,  "DZH"},
    {eCLI_MAIN_DZV,  "DZV"},
    {eCLI_MAIN_DSH,  "DSH"},
    {eCLI_MAIN_DSV,  "DSV"},
    {eCLI_MAIN_PCM,  "PCM"},
    {eCLI_MAIN_GME,  "GME"},
    {eCLI_MAIN_WRP,  "WRP"},
    {eCLI_MAIN_AWF,  "AWF"},
    {eCLI_MAIN_MWF,  "MWF"},
    {eCLI_MAIN_HPC,  "HPC"},
    {eCLI_MAIN_VPC,  "VPC"},
    {eCLI_MAIN_CNR,  "CNR"},
    {eCLI_MAIN_WRE,  "WRE"},
    {eCLI_MAIN_AIG,  "AIG"},
    {eCLI_MAIN_BRT,  "BRT"},
    {eCLI_MAIN_CON,  "CON"},
    {eCLI_MAIN_CSP,  "CSP"},
    {eCLI_MAIN_DTL,  "DTL"},
    {eCLI_MAIN_TDM,  "TDM"},
    {eCLI_MAIN_TDE,  "TDE"},
    {eCLI_MAIN_TDI,  "TDI"},
    {eCLI_MAIN_SIS,  "SIS"},
    {eCLI_MAIN_SOS,  "SOS"},
    {eCLI_MAIN_FDY,  "FDY"},
    {eCLI_MAIN_SDY,  "SDY"},
    {eCLI_MAIN_CLR,  "CLR"},
    {eCLI_MAIN_TNT,  "TNT"},
    {eCLI_MAIN_NDR,  "NDR"},
    {eCLI_MAIN_FTC,  "FTC"},
    {eCLI_MAIN_VBL,  "VBL"},
    {eCLI_MAIN_FMD,  "FMD"},
    {eCLI_MAIN_CLC,  "CLC"},
    {eCLI_MAIN_ROG,  "ROG"},
    {eCLI_MAIN_GOG,  "GOG"},
    {eCLI_MAIN_BOG,  "BOG"},
    {eCLI_MAIN_ROO,  "ROO"},
    {eCLI_MAIN_GOO,  "GOO"},
    {eCLI_MAIN_BOO,  "BOO"},
    {eCLI_MAIN_SYT,  "SYT"},
    {eCLI_MAIN_GOR,  "GOR"},
    {eCLI_MAIN_PST,  "PST"},
    {eCLI_MAIN_PSU,  "PSU"},
    {eCLI_MAIN_FPS,  "FPS"},
    {eCLI_MAIN_DIM,  "DIM"},
    {eCLI_MAIN_FRZ,  "FRZ"},
    {eCLI_MAIN_BGC,  "BGC"},
    {eCLI_MAIN_WPK,  "WPK"},
    {eCLI_MAIN_CCI,  "CCI"},
    {eCLI_MAIN_EDG,  "EDG"},
    {eCLI_MAIN_CWS,  "CWS"},
    {eCLI_MAIN_HSG,  "HSG"},
    {eCLI_MAIN_VBD,  "VBD"},
    {eCLI_MAIN_LOC,  "LOC"},
    {eCLI_MAIN_FCS,  "FCS"},
    {eCLI_MAIN_ZOM,  "ZOM"},
    {eCLI_MAIN_LVO,  "LVO"},
    {eCLI_MAIN_LHO,  "LHO"},
    {eCLI_MAIN_LVU,  "LVU"},
    {eCLI_MAIN_LVD,  "LVD"},
    {eCLI_MAIN_LHL,  "LHL"},
    {eCLI_MAIN_LHR,  "LHR"},
    {eCLI_MAIN_LMR,  "LMR"},
    {eCLI_MAIN_LMA,  "LMA"},
    {eCLI_MAIN_LMS,  "LMS"},
    {eCLI_MAIN_LCB,  "LCB"},
    {eCLI_MAIN_CEL,  "CEL"},
    {eCLI_MAIN_SOR,  "SOR"},
    {eCLI_MAIN_LED,  "LED"},
    {eCLI_MAIN_MSH,  "MSH"},
    {eCLI_MAIN_MSV,  "MSV"},
    {eCLI_MAIN_MBE,  "MBE"},
    {eCLI_MAIN_OST,  "OST"},
    {eCLI_MAIN_SPS,  "SPS"},
    {eCLI_MAIN_PIV,  "PIV"},
    {eCLI_MAIN_PCG,  "PCG"},
    {eCLI_MAIN_PWR,  "PWR"},
    {eCLI_MAIN_APW,  "APW"},
    {eCLI_MAIN_ASH,  "ASH"},
    {eCLI_MAIN_SLP,  "SLP"},
    {eCLI_MAIN_HAT,  "HAT"},
    {eCLI_MAIN_IRC,  "IRC"},
    {eCLI_MAIN_VTT,  "VTT"},
    {eCLI_MAIN_NET,  "NET"},
    {eCLI_MAIN_NTW,  "NTW"},
    {eCLI_MAIN_BDR,  "BDR"},
    {eCLI_MAIN_SEC,  "SEC"},
    {eCLI_MAIN_SPP,  "SPP"},
    {eCLI_MAIN_ADR,  "ADR"},
    {eCLI_MAIN_KBL,  "KBL"},
    {eCLI_MAIN_SBL,  "SBL"},
    {eCLI_MAIN_CCA,  "CCA"},
    {eCLI_MAIN_MDR,  "MDR"},
    {eCLI_MAIN_TDR,  "TDR"},
    {eCLI_MAIN_MAR,  "MAR"},
    {eCLI_MAIN_HAR,  "HAR"},
    {eCLI_MAIN_HKS,  "HKS"},
    {eCLI_MAIN_PIF,  "PIF"},
    {eCLI_MAIN_TPG,  "TPG"},
    {eCLI_MAIN_DEF,  "DEF"},
    {eCLI_MAIN_ITP,  "ITP"},
    {eCLI_MAIN_FTP,  "FTP"},
    {eCLI_MAIN_CWI,  "CWI"},
    {eCLI_MAIN_ERR,  "ERR"},
    {eCLI_MAIN_MDT,  "MDT"},
    {eCLI_MAIN_LDI,  "LDI"},
    {eCLI_MAIN_FAN,  "FAN"},
    {eCLI_MAIN_UST,  "UST"},
    {eCLI_MAIN_LPM,  "LPM"},
    {eCLI_MAIN_LPP,  "LPP"},
    {eCLI_MAIN_LPI,  "LPI"},
    {eCLI_MAIN_LIF,  "LIF"},
    {eCLI_MAIN_RLC,  "RLC"},
    {eCLI_MAIN_SST,  "SST"},
    {eCLI_MAIN_SIN,  "SIN"},
    {eCLI_MAIN_PIP,  "PIP"},
    {eCLI_MAIN_PPS,  "PPS"},
    {eCLI_MAIN_PHS,  "PHS"},
    {eCLI_MAIN_PPP,  "PPP"},
    {eCLI_MAIN_TMG,  "TMG"},
    {eCLI_MAIN_MIF,  "MIF"},
    {eCLI_MAIN_SIF,  "SIF"},
    {eCLI_MAIN_SKS,  "SKS"},
    {eCLI_MAIN_BSS,  "BSS"},
    {eCLI_MAIN_UID,  "UID"},
    {eCLI_MAIN_SIV,  "SIV"},
    {eCLI_MAIN_LCE,  "LCE"},
    {eCLI_MAIN_LSE,  "LSE"},
    {eCLI_MAIN_SNS,  "SNS"},
    {eCLI_MAIN_KEY,  "KEY"},
    {eCLI_MAIN_SHU,  "SHU"},
    {eCLI_MAIN_FCT,  "FCT"},
    {eCLI_MAIN_OSD,  "OSD"},
    {eCLI_MAIN_NRD,  "NRD"},
    {eCLI_MAIN_TTT,  "TTT"},
    {eCLI_MAIN_CSS,  "CSS"}, //chip switch set
    {eCLI_MAIN_CFU,  "CFU"}, //chip FW upgrade
    {eCLI_MAIN_EXF,  "EXF"},
    {eCLI_MAIN_BUR,  "BUR"},
    {eCLI_MAIN_WAP,  "WAP"},
    {eCLI_MAIN_WAS,  "WAS"},
    {eCLI_MAIN_BLD,  "BLD"},
    {eCLI_MAIN_BLS,  "BLS"},
    {eCLI_MAIN_EBL,  "EBL"},
    {eCLI_MAIN_SCR,  "SCR"},
    {eCLI_MAIN_TNR,  "TNR"},
    {eCLI_MAIN_MNR,  "MNR"},
    {eCLI_MAIN_MTO,  "MTO"},
    {eCLI_MAIN_ICI,  "ICI"},
    {eCLI_MAIN_ILI,  "ILI"},

    {eCLI_MAIN_ICP,  "ICP"}, //A70LV_Larry_0086
    {eCLI_MAIN_ICD,  "ICD"},
    {eCLI_MAIN_FGA,  "FGA"}, //A70LV_Larry_0086
    {eCLI_MAIN_ADC,  "ADC"},    //A70LV_Doulas_0124
    {eCLI_MAIN_LET,  "LET"},
    {eCLI_MAIN_LRR,  "LRR"},
    {eCLI_MAIN_FVI,  "FVI"}, //A70LV_Larry_0232
    {eCLI_MAIN_STY,  "STY"}, //A70LV_Larry_0235
    {eCLI_MAIN_LOT,  "LOT"},    //A70LV_Doulas_0228
    {eCLI_MAIN_LOL,  "LOL"},    //A70LV_Doulas_0228
    {eCLI_MAIN_FKC,  "FKC"}, //A70LV_Larry_0283
    {eCLI_MAIN_CCT,  "CCT"},
    {eCLI_MAIN_BAC,  "BAC"}, //A70LV_Larry_0396

    {eCLI_MAIN_SPD,  "SPD"},    //A70LV_Doulas_0327
    {eCLI_MAIN_STH,  "STH"},    //A70LV_Doulas_0327
    {eCLI_MAIN_DLL,  "DLL"},    //A70LV_Doulas_0327
    {eCLI_MAIN_DRB,  "DRB"},    //A70LV_Doulas_0327

    {eCLI_MAIN_TCM, "TCM"},
    {eCLI_MAIN_DDY, "DDY"},
    {eCLI_MAIN_DDM, "DDM"},
    {eCLI_MAIN_DDD, "DDD"},
    {eCLI_MAIN_DTH, "DTH"},
    {eCLI_MAIN_DTM, "DTM"},
    {eCLI_MAIN_DST, "DST"},
    {eCLI_MAIN_NTP, "NTP"},
    {eCLI_MAIN_UTC, "UTC"},
    {eCLI_MAIN_DUI, "DUI"},
    {eCLI_MAIN_DTS, "DTS"},

    {eCLI_MAIN_SDT,  "SDT"},
    {eCLI_MAIN_SD0,  "SD0"},
    {eCLI_MAIN_SD1,  "SD1"},
    {eCLI_MAIN_SD2,  "SD2"},
    {eCLI_MAIN_SD3,  "SD3"},
    {eCLI_MAIN_SD4,  "SD4"},
    {eCLI_MAIN_SD5,  "SD5"},
    {eCLI_MAIN_SD6,  "SD6"},
    {eCLI_MAIN_SCH,  "SCH"},

    {eCLI_MAIN_HDR,  "HDR"},
    {eCLI_MAIN_EDI,  "EDI"},

    {eCLI_MAIN_DBS,  "DBS"},
    {eCLI_MAIN_RBS,  "RBS"},
    {eCLI_MAIN_CER,  "CER"},

    {eCLI_MAIN_EQM,  "EQM"},

    {eCLI_MAIN_AIR,  "AIR"},
    {eCLI_MAIN_PBR,  "PBR"},
    {eCLI_MAIN_PCO,  "PCO"},
    {eCLI_MAIN_PCS,  "PCS"},
    {eCLI_MAIN_LLM,  "LLM"},

    {eCLI_MAIN_BKI,  "BKI"},
    {eCLI_MAIN_IST,  "IST"},

    {eCLI_MAIN_HMO,  "HMO"},

    {eCLI_MAIN_OPD,  "OPD"},
    {eCLI_MAIN_TAR,  "TAR"},

	{eCLI_MAIN_CST,  "CST"},	//A70Gen2_Doulas_0043

    {eCLI_MAIN_ATF,  "ATF"},	//G100_Clare_0028
    {eCLI_MAIN_ATW,  "ATW"},

    #if (ENABLE_COLOR_UNIFORMITY == TRUE) //G100_Tim_0012, add, start
	  {eCLI_MAIN_ACE, "ACE"},
	  {eCLI_MAIN_ACC, "ACC"},
	  {eCLI_MAIN_ACS, "ACS"},
	  {eCLI_MAIN_ACT, "ACT"},
	  {eCLI_MAIN_ACR, "ACR"},
	  //{eCLI_MAIN_ACUW, "ACUW"},
    #endif //ENABLE_COLOR_UNIFORMITY	  //G100_Tim_0012, add, end

    {eCLI_MAIN_BOD, "BOD"},
    {eCLI_MAIN_PLT, "PLT"},
    {eCLI_MAIN_LTL, "LTL"},		//A70Gen2_Doulas_0044
    {eCLI_MAIN_WAE, "WAE"},
    {eCLI_MAIN_DBG, "DBG"},
    {eCLI_MAIN_STP, "STP"},     //HICC2_Doulas_0071
    {eCLI_MAIN_FDI, "FDI"},     //HICC2_Doulas_0071
    {eCLI_MAIN_FDO, "FDO"},     //HICC2_Doulas_0071

#if(BIST_ENABLE)
	{eCLI_MAIN_BST, "BST"},		//HICC2_Steven_0001
#endif
	{eCLI_MAIN_OPP,  "OPP"},    //HICC2_Steven_0009

    {eCLI_MAIN_PRO, "PRO"},
    {eCLI_MAIN_CSL, "CSL"},

    {eCLI_MAIN_RSP,  "RSP"},
    {eCLI_MAIN_RIS,  "RIS"},
    {eCLI_MAIN_RCF,  "RCF"},
    {eCLI_MAIN_LSF,  "LSF"},

	{eCLI_MAIN_ATD,  "ATD"}, //airtight dmd
	{eCLI_MAIN_GUP,  "GUP"},

    {eCLI_MAIN_END,  "FFF"} //
};

#define CLI_MAINLUT_NUMBER sizeof(m_sMainLut)/sizeof(m_sMainLut[0])

static sCLI_DATA_INFO m_sCLI_Info[eccNumber];
static UINT32 m_ulAddressID = 0;  //G100_Wilsonj_0050

////////////////////////////////////////////////////////////////

//Special case function reply

// ==============================================================================
// FUNCTION NAME: utilChristie_CLI_SYS
// DESCRIPTION:
//
//
// Params:
// eCLI_MODE eAccessMode:
// sCLI_CHRISTIE_FORMAT* sCmdFormat:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/11/08, Larry Create
// --------------------
// ==============================================================================
eCLI_ERROR_CODE utilChristie_CLI_SYS(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)
{
    typedef enum
    {
        sSate_NoPower,
        sSate_OFF,
        sSate_Startup,
        sSate_Pin,//pin function is on.
        sSate_Search,
        sSate_Display = 7,
        sSate_BlankScreen = 9,
        sSate_CooldDown = 12,
        sSate_ScreenSave = 13,
        sSate_FlashUpdate = 17,
        sSate_Error = 18,

        //for
    } dvShastaStae;

    if(eAccessMode == ecmRead)
    {
        switch(palSystem_PowerStateGet())
        {
            case ePOWER_STATE_WARMUP:
            case ePOWER_STATE_ACTIVE:
                if(palDataMgr_PIN_Protect_Checking() != ePASSWORD_PROTECT_UNLOCK) //(appGui_PIN_Protect_Checking() == eEXEC_CODE_FAIL)
                {
                    sCmdFormat->lData = sSate_Pin;
                }
                else if(utilCommonCLI_DataControl(edcAUTO_IMAGE) == eFUNC_CONTROL_ENABLE)
                {
                    sCmdFormat->lData = sSate_Display;
                }
                else
                {
                    sCmdFormat->lData = sSate_Search;
                }

                break;

            case ePOWER_STATE_COOLING:
                sCmdFormat->lData = sSate_CooldDown;
                break;

			case ePOWER_STATE_UPGRADE:
				sCmdFormat->lData = sSate_FlashUpdate;
				break;

            case ePOWER_STATE_STANDBY:
				sCmdFormat->lData = sSate_OFF;
                break;

            default:
                sCmdFormat->lData = sSate_Error;
                break;
        }
    }
	/*else	//A70Gen2_Doulas_0006 debug
	{
		if(sCmdFormat->lData == 0)
		{
			halFormatter_3D_3DModeSet(FALSE);
		}
		else if(sCmdFormat->lData == 1)
		{
			halFormatter_3D_3DModeSet(TRUE);
		}
	}*/
    return eCLI_ERROR_CODE_NO;

}

// ==============================================================================
// FUNCTION NAME: utilChristie_CLI_TTT
// DESCRIPTION:
//
//
// Params:
// eCLI_MODE eAccessMode:
// sCLI_CHRISTIE_FORMAT* sCmdFormat:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/11/08, Larry Create
// --------------------
// ==============================================================================
eCLI_ERROR_CODE utilChristie_CLI_TTT(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)
{
    eCLI_ERROR_CODE eErrorCode;
    sCLI_COMMON_FORMAT sCommonFormat;

    sCommonFormat.eAccessMode = eAccessMode;
    sCommonFormat.ucCmdFrom = sCmdFormat->ucCmdFrom;
    sCommonFormat.ucDataType = sCmdFormat->ucDataType;
    sCommonFormat.lData = sCmdFormat->lData;
    sCommonFormat.fData = sCmdFormat->fData;

    eErrorCode = utilCommonCLI_TTT(&sCommonFormat);

    if(eAccessMode == ecmRead)
    {
        sCmdFormat->lData = sCommonFormat.lData;
    }

    return eErrorCode;
}

// ==============================================================================
// FUNCTION NAME: utilChristie_CLI_KEY
// DESCRIPTION:
//
//
// Params:
// eCLI_MODE eAccessMode:
// sCLI_CHRISTIE_FORMAT* sCmdFormat:
//
// Returns:
//
//
// Modification History
// --------------------
// 2018/02/09, Simon Create
// --------------------
// ==============================================================================
eCLI_ERROR_CODE utilChristie_CLI_KEY(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)
{
    eCLI_ERROR_CODE eCLI_ErrorCode = eCLI_ERROR_CODE_SETFAIL ;

    if(palInputProc_InputKeycode(sCmdFormat->lData) == rcSUCCESS)
    {
        eCLI_ErrorCode = eCLI_ERROR_CODE_NO ;
    }

    return eCLI_ErrorCode;
}
eCLI_ERROR_CODE utilChristie_CLI_SYS_SMDC(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)
{
    if(eAccessMode == ecmRead)
    {
        utilDatabase_ItemPrint();
    }

    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilChristie_CLI_SYS_SMIF(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)
{
    if(eAccessMode == ecmRead)
    {
        utilDatabase_InformationPrint();
    }

    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilChristie_CLI_SYS_EmergencyTest(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)
{
    if(eAccessMode == ecmRead)
    {
    }
    else if(eAccessMode == ecmWrite)
    {
        switch(sCmdFormat->lData)
        {
            case 0: //clear
            {
                UINT8 Data = 1;
                palDataMgr_Data_Access(edcEMERGENCY_MESSAGE_CLEAN, edaWRITE_THROUGH_WITH_ACTION, &Data);
            }
            break;

            case 1:
            {
                char String[] = "ABCDEFGH 123456789 ABCDEFGH 123456789 ABCDEFGH 123456789 ABCDEFGH 123456789 ABCDEFGH 123456789 ABCDEFGH 123456789 ABCDEFGH 123456789 ABCDEFGH 123456789 ABCDEFGH 123456789 ABCDEFGH 123456789 ABCDEFGH 123456789 ABCDEFGH 123456789 KKKKK" ;
                palDataMgr_Data_Access(edcEMERGENCY_MESSAGE_SHOWS, edaWRITE_THROUGH_WITH_ACTION, String);
            }
            break;

            case 2:
            {
                char String[] = "ABCDEFGH 123456789 " ;
                palDataMgr_Data_Access(edcEMERGENCY_MESSAGE_SHOWS, edaWRITE_THROUGH_WITH_ACTION, String);
            }
            break;
        }

    }

    return eCLI_ERROR_CODE_NO;
}


// ==============================================================================
// FUNCTION NAME: utilChristie_CLI_SYSDBMK
// DESCRIPTION:
//
//
// Params:
// eCLI_MODE eAccessMode:
// sCLI_CHRISTIE_FORMAT* sCmdFormat:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/11/08, Larry Create
// --------------------
// ==============================================================================
eCLI_ERROR_CODE utilChristie_CLI_SYSDBMK(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)
{
    eCLI_ERROR_CODE eErrorCode;
    sCLI_COMMON_FORMAT sCommonFormat;

    sCommonFormat.eAccessMode = eAccessMode;
    sCommonFormat.ucCmdFrom = sCmdFormat->ucCmdFrom;
    sCommonFormat.ucDataType = sCmdFormat->ucDataType;
    sCommonFormat.lData = sCmdFormat->lData;
    sCommonFormat.fData = sCmdFormat->fData;

    eErrorCode = utilCommonCLI_SYSDBMK(&sCommonFormat);

    return eErrorCode;
}


eCLI_ERROR_CODE utilChristie_CLI_SYSELOG(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)  //A70LV_John_0008 add basic function and flow of GEC
{
    eCLI_ERROR_CODE eErrorCode = eCLI_ERROR_CODE_NO;

    char  ucOutputString[64] = {0};
    UINT16 uiNumber = 0;

    //uiNumber = GEC_GetTotalErrorCount();
    if(eAccessMode == ecmRead) //A70LV_John_0036 complete SYSELOG
    {
        if(uiNumber != 0)
        {
            //GEC_ReadAll_to_Name(); //A70LV_John_0027 Fixed error code bug
            sprintf(ucOutputString, "\r\nTotal Error = %3d\r\n", uiNumber);
            __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        }
        else
        {
            sprintf(ucOutputString, "\r\nNo Error\r\n");
            __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        }
    }
    else if(eAccessMode == ecmWrite)
    {
        eErrorCode = eCLI_ERROR_CODE_COMMAND_READ_ONLY;
    }

    return eErrorCode;
}

eCLI_ERROR_CODE utilChristie_CLI_ERR(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)  //A70LV_John_0008 add basic function and flow of GEC
{
    eCLI_ERROR_CODE eErrorCode = eCLI_ERROR_CODE_NO;

    char cLogTemp[20][128] = {'\0'};
    char  ucOutputString[128] = {0};
    UINT16 uiNumber = 0;

    uiNumber = utilDataMgr_LoadLastLog_CM(20, cLogTemp); //HICC2_Doulas_0031

    if(eAccessMode == ecmRead)
    {
        for(UINT16 uiCount = 0; uiCount < uiNumber; uiCount++)
        {
            sprintf(ucOutputString, "%s\r\n", cLogTemp[uiCount]);
            __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        }

        sprintf(ucOutputString, "(ERR!%02d)\r\n", uiNumber);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
    }
    else if(eAccessMode == ecmWrite)
    {
        eErrorCode = eCLI_ERROR_CODE_COMMAND_READ_ONLY;
    }

    return eErrorCode;
}

// ==============================================================================
// FUNCTION NAME: utilChristie_CFU
// DESCRIPTION:
//
//
// Params:
// eCLI_MODE eAccessMode:
// sCLI_CHRISTIE_FORMAT* sCmdFormat:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/11/08, Larry Create //A70LV_Larry_0112 modify
// --------------------
// ==============================================================================
eCLI_ERROR_CODE utilChristie_CFU(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)
{
    /*00*///eSYSTEM_UPGRADE_MODE_SET,
    /*01*///eSYSTEM_UPGRADE_SELECT_LPCMCU,
    /*02*///eSYSTEM_UPGRADE_SELECT_SCALER_FOCUS,
    /*03*///eSYSTEM_UPGRADE_SELECT_ICHIP,
    /*04*///eSYSTEM_UPGRADE_SELECT_DDP,
    /*05*///eSYSTEM_UPGRADE_SELECT_STARTUP_BASED,
    /*06*///eSYSTEM_UPGRADE_SELECT_FPGA,
    /*07*///eSYSTEM_UPGRADE_SELECT_All,
    char  ucOutputString[128] = {""};

    if(eAccessMode == ecmRead)
    {
        sprintf(ucOutputString, "0 Enter Upgrade Mode\r\n");
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        sprintf(ucOutputString, "1 Upgrade Lpc MCU\r\n");
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        sprintf(ucOutputString, "2 Upgrade OSD Flash Focus\r\n");
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        sprintf(ucOutputString, "3 Upgrade OSD Flash\r\n");
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        sprintf(ucOutputString, "4 DDP upgrade mode\r\n");
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        sprintf(ucOutputString, "5 Upgrade Startup Code Based\r\n");
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        sprintf(ucOutputString, "6 Upgrade FPGA\r\n");
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        sprintf(ucOutputString, "7 Upgrade All\r\n");
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
    }
    else if(eAccessMode == ecmWrite)
    {
        UINT8 ucSelect = (UINT8)sCmdFormat->lData;
        palDataMgr_Data_Access(edcUPGRADE_SELECT, edaWRITE_THROUGH_WITH_ACTION, (void*)&ucSelect);

        switch(sCmdFormat->lData)
        {
            case 0:
                sprintf(ucOutputString, "Enter Upgrade Mode\r\n");
                __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
                break;

            case 1:
                sprintf(ucOutputString, "Upgrade Lpc54113\r\n");
                __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
                break;

            case 2:
                sprintf(ucOutputString, "Upgrade OSD Flash Focus\r\n");
                __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
                break;

            case 3:
                sprintf(ucOutputString, "Upgrade OSD Flash\r\n");
                __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
                break;

            case 4:
                sprintf(ucOutputString, "DDP upgrade mode\r\n");
                __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
                break;

            case 5:
                sprintf(ucOutputString, "Upgrade Startup Code Based\r\n");
                __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
                break;

            case 6:
                sprintf(ucOutputString, "Upgrade FPGA\r\n");
                __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
                break;

            case 7:
                sprintf(ucOutputString, "Upgrade All\r\n");
                __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
                break;
        }
    }

    return eCLI_ERROR_CODE_NO;
}

// ==============================================================================
// FUNCTION NAME: utilChristie_EXFADDR
// DESCRIPTION:
//
//
// Params:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/11/13, Larry Create
// --------------------
// ==============================================================================
eCLI_ERROR_CODE utilChristie_EXFADDR(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)
{
    char  ucOutputString[128] = {""};

    if(eAccessMode == ecmRead)
    {
        sprintf(ucOutputString, "Upgrade select\r\n");
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        sprintf(ucOutputString, "01 C821 Erase all\r\n");
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        sprintf(ucOutputString, "02 C821 Bitmap Raw date\r\n");
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        sprintf(ucOutputString, "03 C821 Text Raw date\r\n");
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
    }
    else if(eAccessMode == ecmWrite)
    {

    }

    return eCLI_ERROR_CODE_NO;
}

// ==============================================================================
// FUNCTION NAME: utilChristie_EXFCHUM
// DESCRIPTION:
//
//
// Params:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/11/13, Larry Create
// --------------------
// ==============================================================================
eCLI_ERROR_CODE utilChristie_EXFCHUM(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)
{
    if(eAccessMode == ecmRead)
    {
        //utilEventHandler_Send(eEVENT_LIST_SPIF_BLOCK_CHECKSUM, 0, NULL);

    }
    else if(eAccessMode == ecmWrite)
    {
        //UINT32 ulBlock = (UINT32)sCmdFormat->lData;

        //utilEventHandler_Send(eEVENT_LIST_SPIF_BLOCK_CHECKSUM, 4, (UINT8*)&ulBlock);
    }

    return eCLI_ERROR_CODE_NO;
}

// ==============================================================================
// FUNCTION NAME: utilChristie_EXFERBL
// DESCRIPTION:
//
//
// Params:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/11/13, Larry Create
// --------------------
// ==============================================================================
eCLI_ERROR_CODE utilChristie_EXFERBL(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)
{
    if(eAccessMode == ecmRead)
    {
        //utilEventHandler_Send(eEVENT_LIST_SPIF_BLOCK_CHECKSUM, 0, NULL);

    }
    else if(eAccessMode == ecmWrite)
    {
        //UINT32 ulBlock = (UINT32)sCmdFormat->lData;

        //utilEventHandler_Send(eEVENT_LIST_SPIF_ERASE_BLOCK, 4, (UINT8*)&ulBlock);
    }

    return eCLI_ERROR_CODE_NO;
}

// ==============================================================================
// FUNCTION NAME: utilChristie_EXFWREL
// DESCRIPTION:
//
//
// Params:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/11/13, Larry Create
// --------------------
// ==============================================================================
eCLI_ERROR_CODE utilChristie_EXFWREL(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)
{
    if(eAccessMode == ecmRead)
    {
        //utilEventHandler_Send(eEVENT_LIST_SPIF_BLOCK_CHECKSUM, 0, NULL);

    }
    else if(eAccessMode == ecmWrite)
    {
        //UINT32 ulBlock = (UINT32)sCmdFormat->lData;

        //utilEventHandler_Send(eEVENT_LIST_SPIF_BLOCK_WRITE, 4, (UINT8*)&ulBlock);
    }

    return eCLI_ERROR_CODE_NO;
}

// ==============================================================================
// FUNCTION NAME: utilChristie_PWR
// DESCRIPTION:
//
//
// Params:
// eCLI_MODE eAccessMode:
// sCLI_CHRISTIE_FORMAT* sCmdFormat:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/11/08, Larry Create
// --------------------
// ==============================================================================
eCLI_ERROR_CODE utilChristie_PWR(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)
{
	eCLI_ERROR_CODE eErrorCode = eCLI_ERROR_CODE_NO;

	UINT8 ucPwrStatus = palSystem_PowerStateGet(); //A70Gen2_Julie_0011

#if 1
    if(eAccessMode == ecmRead)
    {
        //sCmdFormat->lData = palSystem_PowerStateGet();
        //LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d): read pwr = %d\r\n", __FUNCTION__, __LINE__, sCmdFormat->lData);

    	switch(ucPwrStatus) //HICC2_Doulas_0069 Modify
        {
            case ePOWER_STATE_STANDBY:
                sCmdFormat->lData = 0;
                break;

            case ePOWER_STATE_ACTIVE:
                if(palEnvironment_NetworkIsReceivePowerOnCmd_Get())
                {
                    sCmdFormat->lData = 1; // Power On
                }
                else
                {
                    sCmdFormat->lData = 11; // Warm Up
                }
                break;

            case ePOWER_STATE_COOLING:
                sCmdFormat->lData = 10;
                break;

            default:
                sCmdFormat->lData = palSystem_PowerStateGet();
                break;
        }
    }
    else if(eAccessMode == ecmWrite)
    {
    	//LOG_MSG(db_ALWAYS, "PWR type %d\r\n",sCmdFormat->ucDataType);
    	if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)	//A70Gen2_Doulas_0050
		{
			return eCLI_ERROR_CODE_DATAERROR;
		}

        if(sCmdFormat->lData == 1)
        {
			if(ucPwrStatus != ePOWER_STATE_COOLING && ucPwrStatus != ePOWER_STATE_ACTIVE)
            //palSystem_WarmUp();
            	palSystem_PowerKey();
        }
        else if(sCmdFormat->lData == 0)
        {
			if(ucPwrStatus == ePOWER_STATE_ACTIVE)
            	palSystem_PowerDown();
        }
		else
		{
        	eErrorCode = eCLI_ERROR_CODE_SETFAIL;
		}
        LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d): write pwr = %d\r\n", __FUNCTION__, __LINE__, sCmdFormat->lData);
    }
#else
    if(eAccessMode == ecmRead)
    {
        sCmdFormat->lData = 1;
    }
    else if(eAccessMode == ecmWrite)
    {
        if(sCmdFormat->lData == 0)
        {
            UINT8 ucLightSourceMode = 0;
            UINT8 ucCalibration = 0;
            UINT8 ucNO_POWER_OFF = 0;   //A70LV_Doulas_0374

            palDataMgr_Data_Access(edcNO_POWER_OFF, edaREAD, &ucNO_POWER_OFF);  //A70LV_Doulas_0374 Add
            if(ucNO_POWER_OFF)                                                  //A70LV_Doulas_0374 Add
            {
                return eCLI_ERROR_CODE_SETFAIL;
            }

            palDataMgr_Data_Access(edcPOWER_MODE, edaREAD, &ucLightSourceMode);
            palDataMgr_Data_Access(edcLIGHT_SENSOR_CALIBRATION, edaREAD, &ucCalibration);

            if(((ePM_CONSTANT_INTENSITY == ucLightSourceMode) ||
                (ePM_RENTAL_MODE == ucLightSourceMode)) &&
                (ucCalibration == eLIGHT_SENSOR_CALIBRATION_MODE_DEFAULT))
            {
                palIllumination_ABP_StatusSet(eABP_STATUS_DYNAMIC_RLD_CAL_SHUTDOWN);
            }
            else
            {
                palSystem_PowerDown();
            }
        }
		else
		{
        	eErrorCode = eCLI_ERROR_CODE_SETFAIL;
		}
    }
#endif

	return eErrorCode;
}

// ==============================================================================
// FUNCTION NAME: utilChristie_PWRSLEP
// DESCRIPTION:
//
//
// Params:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/11/14, Larry Create
// --------------------
// ==============================================================================
eCLI_ERROR_CODE utilChristie_PWRSLEP(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)
{
    if(eAccessMode == ecmRead)
    {
        UINT8 ucEnable = 0;
        //utilStartupRead(ePARA_STARTUP_SLEEP_MODE, &ucEnable);

        sCmdFormat->lData = (INT32)ucEnable;
    }
    else if(eAccessMode == ecmWrite)
    {
        UINT8 ucEnable = 0;

        if(sCmdFormat->lData == 0)
        {
            //utilStartupWrite(ePARA_STARTUP_SLEEP_MODE, &ucEnable);
        }
        else
        {
            ucEnable = 1;
            //utilStartupWrite(ePARA_STARTUP_SLEEP_MODE, &ucEnable);
        }
    }
    return eCLI_ERROR_CODE_NO;
}


// ==============================================================================
// FUNCTION NAME: utilChristie_FCS
// DESCRIPTION:
//
//
// Params:
// eCLI_MODE eAccessMode:
// sCLI_CHRISTIE_FORMAT* sCmdFormat:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/11/08, Larry Create
// --------------------
// ==============================================================================
eCLI_ERROR_CODE utilChristie_FCS(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)
{
    if(eAccessMode == ecmRead)
    {
        return eCLI_ERROR_CODE_REQUESTFAIL;

    }
    else if(eAccessMode == ecmWrite)
    {
        sKEY_DATA sInputKey = {0};

        if(palInputProc_LensMotorLockGet() == eLENS_LOCK_ALL_ALLOW) //A70LV_Larry_0129
        {
            if(sCmdFormat->ucCmdFrom == eccTelnet)
            {
                sInputKey.eKeyType = eKEYINPUT_LAN;
            }
            else if(sCmdFormat->ucCmdFrom == eccHDBaseT)
            {
                sInputKey.eKeyType = eKEYINPUT_HDBaseT;
            }
            else
            {
                sInputKey.eKeyType = eKEYINPUT_CLI;
            }

            switch(sCmdFormat->ucDataType)
            {
                case eCLI_DATA_TYPE_N:
                case eCLI_DATA_TYPE_N_STEP:
                    {
                        sInputKey.wKeyCode = keFOCUSUP;
                        sInputKey.eKeyEvent = eKEY_EVENT_PRESSED;

                        palInputProc_BufferInsert(&sInputKey);
                    }
                    break;

                case eCLI_DATA_TYPE_P:
                case eCLI_DATA_TYPE_P_STEP:
                    {
                        sInputKey.wKeyCode = keFOCUSDOWN;
                        sInputKey.eKeyEvent = eKEY_EVENT_PRESSED;

                        palInputProc_BufferInsert(&sInputKey);
                    }
                    break;

                case eCLI_DATA_TYPE_N_STOP:
                    break;

                case eCLI_DATA_TYPE_N_RUN:
                    {
                        sInputKey.wKeyCode = keFOCUSUP;
                        sInputKey.eKeyEvent = eKEY_EVENT_HOLD;

                        palInputProc_BufferInsert(&sInputKey);
                    }
                    break;

                case eCLI_DATA_TYPE_P_STOP:
                    break;

                case eCLI_DATA_TYPE_P_RUN:
                    {
                        sInputKey.wKeyCode = keFOCUSDOWN;
                        sInputKey.eKeyEvent = eKEY_EVENT_HOLD;

                        palInputProc_BufferInsert(&sInputKey);
                    }
                    break;

                default:
                    break;
            }
        }
        else
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

// ==============================================================================
// FUNCTION NAME: utilChristie_ZOM
// DESCRIPTION:
//
//
// Params:
// eCLI_MODE eAccessMode:
// sCLI_CHRISTIE_FORMAT* sCmdFormat:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/11/08, Larry Create
// --------------------
// ==============================================================================
eCLI_ERROR_CODE utilChristie_ZOM(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)
{
    if(eAccessMode == ecmRead)
    {
        return eCLI_ERROR_CODE_REQUESTFAIL;

    }
    else if(eAccessMode == ecmWrite)
    {
        sKEY_DATA sInputKey = {0};

        if(palInputProc_LensMotorLockGet() == eLENS_LOCK_ALL_ALLOW) //A70LV_Larry_0129
        {
            if(sCmdFormat->ucCmdFrom == eccTelnet)
            {
                sInputKey.eKeyType = eKEYINPUT_LAN;
            }
            else if(sCmdFormat->ucCmdFrom == eccHDBaseT)
            {
                sInputKey.eKeyType = eKEYINPUT_HDBaseT;
            }
            else
            {
                sInputKey.eKeyType = eKEYINPUT_CLI;
            }

            switch(sCmdFormat->ucDataType) //A35G2_CDS_Larry_0008 fixed
            {
                case eCLI_DATA_TYPE_N_STOP:
                    break;

                case eCLI_DATA_TYPE_N:
                case eCLI_DATA_TYPE_N_STEP:
                    {
                        sInputKey.wKeyCode = keZOOMOUT;
                        sInputKey.eKeyEvent = eKEY_EVENT_PRESSED;

                        palInputProc_BufferInsert(&sInputKey);
                    }
                    break;

                case eCLI_DATA_TYPE_N_RUN:
                    {
                        sInputKey.wKeyCode = keHOLD_ZOOMOUT;
                        sInputKey.eKeyEvent = eKEY_EVENT_PRESSED;

                        palInputProc_BufferInsert(&sInputKey);
                    }
                    break;

                case eCLI_DATA_TYPE_P_STOP:
                    break;

                case eCLI_DATA_TYPE_P:
                case eCLI_DATA_TYPE_P_STEP:
                    {
                        sInputKey.wKeyCode = keZOOMIN;
                        sInputKey.eKeyEvent = eKEY_EVENT_PRESSED;

                        palInputProc_BufferInsert(&sInputKey);
                    }
                    break;

                case eCLI_DATA_TYPE_P_RUN:
                    {
                        sInputKey.wKeyCode = keHOLD_ZOOMIN;
                        sInputKey.eKeyEvent = eKEY_EVENT_PRESSED;

                        palInputProc_BufferInsert(&sInputKey);
                    }
                    break;

                default:
                    break;
            }
        }
        else
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

// ==============================================================================
// FUNCTION NAME: utilChristie_LVO
// DESCRIPTION:
//
//
// Params:
// eCLI_MODE eAccessMode:
// sCLI_CHRISTIE_FORMAT* sCmdFormat:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/11/08, Larry Create
// --------------------
// ==============================================================================
eCLI_ERROR_CODE utilChristie_LVO(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)
{
    if(eAccessMode == ecmRead)
    {
        return eCLI_ERROR_CODE_REQUESTFAIL;
    }
    else if(eAccessMode == ecmWrite)
    {
        sKEY_DATA sInputKey = {0};

        if(palInputProc_LensMotorLockGet() == eLENS_LOCK_ALL_ALLOW) //A70LV_Larry_0129
        {
            if(palEnvironment_LensCalFlag_Get() == FALSE)//HICC2_Julie_0048
            {
                return eCLI_ERROR_CODE_SETFAIL;
            }

            if(sCmdFormat->ucCmdFrom == eccTelnet)
            {
                sInputKey.eKeyType = eKEYINPUT_LAN;
            }
            else if(sCmdFormat->ucCmdFrom == eccHDBaseT)
            {
                sInputKey.eKeyType = eKEYINPUT_HDBaseT;
            }
            else
            {
                sInputKey.eKeyType = eKEYINPUT_CLI;
            }

            switch(sCmdFormat->ucDataType)
            {
                case eCLI_DATA_TYPE_N:
                case eCLI_DATA_TYPE_N_STEP:
                case eCLI_DATA_TYPE_D:       //HICC2_Steven_0056
                case eCLI_DATA_TYPE_D_STEP:
                    {
                        sInputKey.wKeyCode = keLENSVDOWN;
                        sInputKey.eKeyEvent = eKEY_EVENT_PRESSED;

                        palInputProc_BufferInsert(&sInputKey);
                    }
                    break;

                case eCLI_DATA_TYPE_P:
                case eCLI_DATA_TYPE_P_STEP:
                case eCLI_DATA_TYPE_U:      //HICC2_Steven_0056
                case eCLI_DATA_TYPE_U_STEP:
                    {
                        sInputKey.wKeyCode = keLENSVUP;
                        sInputKey.eKeyEvent = eKEY_EVENT_PRESSED;

                        palInputProc_BufferInsert(&sInputKey);
                    }
                    break;

                case eCLI_DATA_TYPE_N_STOP:
                case eCLI_DATA_TYPE_D_STOP:  //HICC2_Steven_0056
                    break;

                case eCLI_DATA_TYPE_N_RUN:
                case eCLI_DATA_TYPE_D_RUN:  //HICC2_Steven_0056
                    {
                        sInputKey.wKeyCode = keHOLD_LENSVDOWN;
                        sInputKey.eKeyEvent = eKEY_EVENT_PRESSED;

                        palInputProc_BufferInsert(&sInputKey);
                    }
                    break;

                case eCLI_DATA_TYPE_P_STOP:
                case eCLI_DATA_TYPE_U_STOP:  //HICC2_Steven_0056
                    break;

                case eCLI_DATA_TYPE_P_RUN:
                case eCLI_DATA_TYPE_U_RUN:  //HICC2_Steven_0056
                    {
                        sInputKey.wKeyCode = keHOLD_LENSVUP;
                        sInputKey.eKeyEvent = eKEY_EVENT_PRESSED;

                        palInputProc_BufferInsert(&sInputKey);
                    }
                    break;

                default:
                    break;
            }
        }
        else
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

// ==============================================================================
// FUNCTION NAME: utilChristie_LHO
// DESCRIPTION:
//
//
// Params:
// eCLI_MODE eAccessMode:
// sCLI_CHRISTIE_FORMAT* sCmdFormat:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/11/08, Larry Create
// --------------------
// ==============================================================================
eCLI_ERROR_CODE utilChristie_LHO(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)
{
    if(eAccessMode == ecmRead)
    {
        return eCLI_ERROR_CODE_REQUESTFAIL;

    }
    else if(eAccessMode == ecmWrite)
    {
        sKEY_DATA sInputKey = {0};

        if(sCmdFormat->ucCmdFrom == eccTelnet)
        {
            sInputKey.eKeyType = eKEYINPUT_LAN;
        }
        else if(sCmdFormat->ucCmdFrom == eccHDBaseT)
        {
            sInputKey.eKeyType = eKEYINPUT_HDBaseT;
        }
        else
        {
            sInputKey.eKeyType = eKEYINPUT_CLI;
        }

        if(palInputProc_LensMotorLockGet() == eLENS_LOCK_ALL_ALLOW) //A70LV_Larry_0129
        {
            if(palEnvironment_LensCalFlag_Get() == FALSE)//HICC2_Julie_0048
            {
                return eCLI_ERROR_CODE_SETFAIL;
            }

            switch(sCmdFormat->ucDataType)
            {
                case eCLI_DATA_TYPE_N:
                case eCLI_DATA_TYPE_N_STEP:
                case eCLI_DATA_TYPE_R:        //HICC2_Steven_0056
                case eCLI_DATA_TYPE_R_STEP:
                    {
                        sInputKey.wKeyCode = keLENSHRIGHT;
                        sInputKey.eKeyEvent = eKEY_EVENT_PRESSED;

                        palInputProc_BufferInsert(&sInputKey);
                    }
                    break;

                case eCLI_DATA_TYPE_P:
                case eCLI_DATA_TYPE_P_STEP:
                case eCLI_DATA_TYPE_L:        //HICC2_Steven_0056
                case eCLI_DATA_TYPE_L_STEP:
                    {
                        sInputKey.wKeyCode = keLENSHLEFT;
                        sInputKey.eKeyEvent = eKEY_EVENT_PRESSED;

                        palInputProc_BufferInsert(&sInputKey);
                    }
                    break;

                case eCLI_DATA_TYPE_N_STOP:
                case eCLI_DATA_TYPE_R_STOP:   //HICC2_Steven_0056
                    break;

                case eCLI_DATA_TYPE_N_RUN:
                case eCLI_DATA_TYPE_R_RUN:    //HICC2_Steven_0056
                    {
                        sInputKey.wKeyCode = keHOLD_LENSHRIGHT;
                        sInputKey.eKeyEvent = eKEY_EVENT_PRESSED;

                        palInputProc_BufferInsert(&sInputKey);
                    }
                    break;

                case eCLI_DATA_TYPE_P_STOP:
                case eCLI_DATA_TYPE_L_STOP:  //HICC2_Steven_0056
                    break;

                case eCLI_DATA_TYPE_P_RUN:
                case eCLI_DATA_TYPE_L_RUN:   //HICC2_Steven_0056
                    {
                        sInputKey.wKeyCode = keHOLD_LENSHLEFT;
                        sInputKey.eKeyEvent = eKEY_EVENT_PRESSED;

                        palInputProc_BufferInsert(&sInputKey);
                    }
                    break;

                default:
                    break;
            }
        }
        else
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

// ==============================================================================
// FUNCTION NAME: utilChristie_LVU
// DESCRIPTION:
//
//
// Params:
// eCLI_MODE eAccessMode:
// sCLI_CHRISTIE_FORMAT* sCmdFormat:
//
// Returns:
//
//
// Modification History
// --------------------
// 2019/07/09, Larry Create
// --------------------
// ==============================================================================
eCLI_ERROR_CODE utilChristie_LVU(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)
{
    if(eAccessMode == ecmRead)
    {
        return eCLI_ERROR_CODE_REQUESTFAIL;
    }
    else if(eAccessMode == ecmWrite)
    {
        if(palInputProc_LensMotorLockGet() == eLENS_LOCK_ALL_ALLOW)
        {
            if(palEnvironment_LensCalFlag_Get() == FALSE)//HICC2_Julie_0048
            {
                return eCLI_ERROR_CODE_SETFAIL;
            }
            else
            {
                sKEY_DATA sInputKey = {0};
                sInputKey.wKeyCode = keLENS;

                if(sCmdFormat->ucCmdFrom == eccTelnet)
                {
                    sInputKey.eKeyType = eKEYINPUT_LAN;
                }
                else if(sCmdFormat->ucCmdFrom == eccHDBaseT)
                {
                    sInputKey.eKeyType = eKEYINPUT_HDBaseT;
                }
                else
                {
                    sInputKey.eKeyType = eKEYINPUT_CLI;
                }

                if(sCmdFormat->lData == 0)
                {
                    if(sCmdFormat->ucCmdFrom == eccTelnet)
                    {
                        palInputProc_KeyEventSend(sInputKey); //appGui_SendKeyEvent(eKEYINPUT_LAN, ekLENSSHIFT);
                    }
                    else if(sCmdFormat->ucCmdFrom == eccHDBaseT)
                    {
                        palInputProc_KeyEventSend(sInputKey); //appGui_SendKeyEvent(eKEYINPUT_HDBaseT, ekLENSSHIFT);
                    }
                    else
                    {
                        palInputProc_KeyEventSend(sInputKey); //appGui_SendKeyEvent(eKEYINPUT_CLI, ekLENSSHIFT);
                    }
                    palMotorLensSet(eLENS_CMDS_STEP_UP); //HICC2_Doulas_0105
                }
                else if((sCmdFormat->lData >= 1)  && (sCmdFormat->lData <= 100))
                {
                    if(sCmdFormat->ucCmdFrom == eccTelnet)
                    {
                        palInputProc_KeyEventSend(sInputKey); //appGui_SendKeyEvent(eKEYINPUT_LAN, ekLENSSHIFT);
                    }
                    else if(sCmdFormat->ucCmdFrom == eccHDBaseT)
                    {
                        palInputProc_KeyEventSend(sInputKey); //appGui_SendKeyEvent(eKEYINPUT_HDBaseT, ekLENSSHIFT);
                    }
                    else
                    {
                        palInputProc_KeyEventSend(sInputKey); //appGui_SendKeyEvent(eKEYINPUT_CLI, ekLENSSHIFT);
                    }
                    palMotorLensRunPixelSet(eLENS_CMDS_PIXEL_UP, (UINT32)sCmdFormat->lData);
                }
                else
                {
                    return eCLI_ERROR_CODE_DATAOVERRANGE;//HICC2_Julie_0039
                }
            }
        }
        else
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

// ==============================================================================
// FUNCTION NAME: utilChristie_LVD
// DESCRIPTION:
//
//
// Params:
// eCLI_MODE eAccessMode:
// sCLI_CHRISTIE_FORMAT* sCmdFormat:
//
// Returns:
//
//
// Modification History
// --------------------
// 2019/07/09, Larry Create
// --------------------
// ==============================================================================
eCLI_ERROR_CODE utilChristie_LVD(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)
{
    if(eAccessMode == ecmRead)
    {
        return eCLI_ERROR_CODE_REQUESTFAIL;

    }
    else if(eAccessMode == ecmWrite)
    {
        if(palInputProc_LensMotorLockGet() == eLENS_LOCK_ALL_ALLOW) //A70LV_Larry_0129
        {
            if(palEnvironment_LensCalFlag_Get() == FALSE)//HICC2_Julie_0048
            {
                return eCLI_ERROR_CODE_SETFAIL;
            }
            else
            {
                sKEY_DATA sInputKey = {0};
                sInputKey.wKeyCode = keLENS;

                if(sCmdFormat->ucCmdFrom == eccTelnet)
                {
                    sInputKey.eKeyType = eKEYINPUT_LAN;
                }
                else if(sCmdFormat->ucCmdFrom == eccHDBaseT)
                {
                    sInputKey.eKeyType = eKEYINPUT_HDBaseT;
                }
                else
                {
                    sInputKey.eKeyType = eKEYINPUT_CLI;
                }

                if(sCmdFormat->lData == 0)
                {
                    if(sCmdFormat->ucCmdFrom == eccTelnet)
                    {
                        palInputProc_KeyEventSend(sInputKey); //appGui_SendKeyEvent(eKEYINPUT_LAN, ekLENSSHIFT);
                    }
                    else if(sCmdFormat->ucCmdFrom == eccHDBaseT)
                    {
                        palInputProc_KeyEventSend(sInputKey); //appGui_SendKeyEvent(eKEYINPUT_HDBaseT, ekLENSSHIFT);
                    }
                    else
                    {
                        palInputProc_KeyEventSend(sInputKey); //appGui_SendKeyEvent(eKEYINPUT_CLI, ekLENSSHIFT);
                    }
                    palMotorLensSet(eLENS_CMDS_STEP_DOWN); //HICC2_Doulas_0105
                }
                else if((sCmdFormat->lData >= 1)  && (sCmdFormat->lData <= 100))
                {
                    if(sCmdFormat->ucCmdFrom == eccTelnet)
                    {
                        palInputProc_KeyEventSend(sInputKey); //appGui_SendKeyEvent(eKEYINPUT_LAN, ekLENSSHIFT);
                    }
                    else if(sCmdFormat->ucCmdFrom == eccHDBaseT)
                    {
                        palInputProc_KeyEventSend(sInputKey); //appGui_SendKeyEvent(eKEYINPUT_HDBaseT, ekLENSSHIFT);
                    }
                    else
                    {
                        palInputProc_KeyEventSend(sInputKey); //appGui_SendKeyEvent(eKEYINPUT_CLI, ekLENSSHIFT);
                    }
                    palMotorLensRunPixelSet(eLENS_CMDS_PIXEL_DOWN, (UINT32)sCmdFormat->lData);
                }
                else
                {
                    return eCLI_ERROR_CODE_DATAOVERRANGE;//HICC2_Julie_0039
                }
            }
        }
        else
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

// ==============================================================================
// FUNCTION NAME: utilChristie_LHL
// DESCRIPTION:
//
//
// Params:
// eCLI_MODE eAccessMode:
// sCLI_CHRISTIE_FORMAT* sCmdFormat:
//
// Returns:
//
//
// Modification History
// --------------------
// 2019/07/09, Larry Create
// --------------------
// ==============================================================================
eCLI_ERROR_CODE utilChristie_LHL(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)
{
    if(eAccessMode == ecmRead)
    {
        return eCLI_ERROR_CODE_REQUESTFAIL;

    }
    else if(eAccessMode == ecmWrite)
    {
        if(palInputProc_LensMotorLockGet() == eLENS_LOCK_ALL_ALLOW) //A70LV_Larry_0129
        {
            if(palEnvironment_LensCalFlag_Get() == FALSE)//HICC2_Julie_0048
            {
                return eCLI_ERROR_CODE_SETFAIL;
            }
            else
            {
                sKEY_DATA sInputKey = {0};
                sInputKey.wKeyCode = keLENS;

                if(sCmdFormat->ucCmdFrom == eccTelnet)
                {
                    sInputKey.eKeyType = eKEYINPUT_LAN;
                }
                else if(sCmdFormat->ucCmdFrom == eccHDBaseT)
                {
                    sInputKey.eKeyType = eKEYINPUT_HDBaseT;
                }
                else
                {
                    sInputKey.eKeyType = eKEYINPUT_CLI;
                }

                if(sCmdFormat->lData == 0)
                {
                    if(sCmdFormat->ucCmdFrom == eccTelnet)
                    {
                        palInputProc_KeyEventSend(sInputKey); //appGui_SendKeyEvent(eKEYINPUT_LAN, ekLENSSHIFT);
                    }
                    else if(sCmdFormat->ucCmdFrom == eccHDBaseT)
                    {
                        palInputProc_KeyEventSend(sInputKey); //appGui_SendKeyEvent(eKEYINPUT_HDBaseT, ekLENSSHIFT);
                    }
                    else
                    {
                        palInputProc_KeyEventSend(sInputKey); //appGui_SendKeyEvent(eKEYINPUT_CLI, ekLENSSHIFT);
                    }
                    palMotorLensSet(eLENS_CMDS_STEP_LEFT); //HICC2_Doulas_0105
                }
                else if((sCmdFormat->lData >= 1)  && (sCmdFormat->lData <= 100))
                {
                    if(sCmdFormat->ucCmdFrom == eccTelnet)
                    {
                        palInputProc_KeyEventSend(sInputKey); //appGui_SendKeyEvent(eKEYINPUT_LAN, ekLENSSHIFT);
                    }
                    else if(sCmdFormat->ucCmdFrom == eccHDBaseT)
                    {
                        palInputProc_KeyEventSend(sInputKey); //appGui_SendKeyEvent(eKEYINPUT_HDBaseT, ekLENSSHIFT);
                    }
                    else
                    {
                        palInputProc_KeyEventSend(sInputKey); //appGui_SendKeyEvent(eKEYINPUT_CLI, ekLENSSHIFT);
                    }
                    palMotorLensRunPixelSet(eLENS_CMDS_PIXEL_LEFT, (UINT32)sCmdFormat->lData);
                }
                else
                {
                    return eCLI_ERROR_CODE_DATAOVERRANGE;//HICC2_Julie_0039
                }
            }
        }
        else
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

// ==============================================================================
// FUNCTION NAME: utilChristie_LHR
// DESCRIPTION:
//
//
// Params:
// eCLI_MODE eAccessMode:
// sCLI_CHRISTIE_FORMAT* sCmdFormat:
//
// Returns:
//
//
// Modification History
// --------------------
// 2019/07/09, Larry Create
// --------------------
// ==============================================================================
eCLI_ERROR_CODE utilChristie_LHR(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)
{
    if(eAccessMode == ecmRead)
    {
        return eCLI_ERROR_CODE_REQUESTFAIL;
    }
    else if(eAccessMode == ecmWrite)
    {
        if(palInputProc_LensMotorLockGet() == eLENS_LOCK_ALL_ALLOW) //A70LV_Larry_0129
        {
            if(palEnvironment_LensCalFlag_Get() == FALSE)//HICC2_Julie_0048
            {
                return eCLI_ERROR_CODE_SETFAIL;
            }
            else
            {
                sKEY_DATA sInputKey = {0};
                sInputKey.wKeyCode = keLENS;

                if(sCmdFormat->ucCmdFrom == eccTelnet)
                {
                    sInputKey.eKeyType = eKEYINPUT_LAN;
                }
                else if(sCmdFormat->ucCmdFrom == eccHDBaseT)
                {
                    sInputKey.eKeyType = eKEYINPUT_HDBaseT;
                }
                else
                {
                    sInputKey.eKeyType = eKEYINPUT_CLI;
                }

                if(sCmdFormat->lData == 0)
                {
                    if(sCmdFormat->ucCmdFrom == eccTelnet)
                    {
                        palInputProc_KeyEventSend(sInputKey); //appGui_SendKeyEvent(eKEYINPUT_LAN, ekLENSSHIFT);
                    }
                    else if(sCmdFormat->ucCmdFrom == eccHDBaseT)
                    {
                        palInputProc_KeyEventSend(sInputKey); //appGui_SendKeyEvent(eKEYINPUT_HDBaseT, ekLENSSHIFT);
                    }
                    else
                    {
                        palInputProc_KeyEventSend(sInputKey); //appGui_SendKeyEvent(eKEYINPUT_CLI, ekLENSSHIFT);
                    }
                    palMotorLensSet(eLENS_CMDS_STEP_RIGHT); //HICC2_Doulas_0105
                }
                else if((sCmdFormat->lData >= 1)  && (sCmdFormat->lData <= 100))
                {
                    if(sCmdFormat->ucCmdFrom == eccTelnet)
                    {
                        palInputProc_KeyEventSend(sInputKey); //appGui_SendKeyEvent(eKEYINPUT_LAN, ekLENSSHIFT);
                    }
                    else if(sCmdFormat->ucCmdFrom == eccHDBaseT)
                    {
                        palInputProc_KeyEventSend(sInputKey); //appGui_SendKeyEvent(eKEYINPUT_HDBaseT, ekLENSSHIFT);
                    }
                    else
                    {
                        palInputProc_KeyEventSend(sInputKey); //appGui_SendKeyEvent(eKEYINPUT_CLI, ekLENSSHIFT);
                    }
                    palMotorLensRunPixelSet(eLENS_CMDS_PIXEL_RIGHT, (UINT32)sCmdFormat->lData);
                }
                else
                {
                    return eCLI_ERROR_CODE_DATAOVERRANGE;//HICC2_Julie_0039
                }
            }
        }
        else
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilChristie_IST(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)
{
    if(eAccessMode == ecmRead)
    {
        return eCLI_ERROR_CODE_NO;
    }
    else if(eAccessMode == ecmWrite)
    {
    	UINT8 ucCount = 0xFF;
        UINT16 uiData = sCmdFormat->lData, uiValue = 0;;
        eRESULT eResult_sys = rcERROR, eResult_FE = rcERROR, eResult_DDP = rcERROR;
        float fRate = 0;

        //clear error count
		utilOPD_MasterDevice_Count_Reset();
		halMCUCtrl_OPD_Register_Set(eOPD_MSG_EVENT_RESET, eOPD_MSG_EVENT_RESET_SZ, &ucCount);
		MS_SLEEP(1000);

        //i2c+uart+spi stress test.
        for(UINT16 uiNum = 0; uiNum < uiData; uiNum++)
        {
            if((uiNum % 10 == 0) && (uiNum >= 10))
            {
                fRate = (float)uiNum * 100 / uiData;
                LOG_MSG(db_ALWAYS,"Stress Test Rate = %2.2f%\r\n", fRate);
            }

            //System 54605
 			ucCount = 1;
            eResult_sys = halMCUCtrl_OPD_Register_Set(eOPD_MSG_I2C_STRESS_TEST, 1, &ucCount);
            if(eResult_sys != rcSUCCESS)
            {
                utilOPD_MasterDevice_Count_Set(eIF_I2C, eI2C_STANDBY);
            }

            //FrontEnd 54605
            eResult_sys = halFrontEndCtrl_OPD_Register_Set(eOPD_MSG_I2C_STRESS_TEST, 1, &ucCount);
            if(eResult_sys != rcSUCCESS)
            {
                utilOPD_MasterDevice_Count_Set(eIF_I2C, eI2C_FRONDEND);
            }

            //DDP4422
			eResult_DDP = halFormatter_SYS_AppVersionGet(&uiValue); //G100_Julie_0012
            if(eResult_DDP != rcSUCCESS)
            {
                utilOPD_MasterDevice_Count_Set(eIF_I2C, eI2C_DDP4422);
            }
			//LOG_MSG(db_ALWAYS,"(%s,%d) [%d]\r\n", __FUNCTION__, __LINE__, eResult_DDP);

			utilOPD_UART_SPI_TEST(); //G100_Julie_0004
			MS_SLEEP(1 * eCM_IF_NUMBER);   //G100_Julie_0014
        }
        utilOPD_Stress_Test_Enable_Set(eIF_I2C, TRUE);
		utilOPD_Stress_Test_Enable_Set(eIF_UART, TRUE);
        LOG_MSG(db_ALWAYS,"Stress Test Finish.\r\n");

        //uart & spi test
        utilOPD_UartTestCount_Set(uiData);

    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilChristie_ICP(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat) //A70LV_Larry_0086
{
    eCLI_ERROR_CODE eErrorCode;
    sCLI_COMMON_FORMAT sCommonFormat;

    sCommonFormat.eAccessMode = eAccessMode;
    sCommonFormat.ucCmdFrom = sCmdFormat->ucCmdFrom;
    sCommonFormat.ucDataType = sCmdFormat->ucDataType;
    sCommonFormat.lData = sCmdFormat->lData;
    sCommonFormat.fData = sCmdFormat->fData;

    snprintf(sCommonFormat.cTextString, CLI_DATA_SIZE, "%s", sCmdFormat->cTextString);

    eErrorCode = utilCommonCLI_ICP(&sCommonFormat);

    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilChristie_ICPC821_SET(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat) //A35G2_Larry_0088
{
    eCLI_ERROR_CODE eErrorCode;
    sCLI_COMMON_FORMAT sCommonFormat;

    sCommonFormat.eAccessMode = eAccessMode;
    sCommonFormat.ucCmdFrom = sCmdFormat->ucCmdFrom;
    sCommonFormat.ucDataType = sCmdFormat->ucDataType;
    sCommonFormat.lData = sCmdFormat->lData;
    sCommonFormat.fData = sCmdFormat->fData;

    snprintf(sCommonFormat.cTextString, CLI_DATA_SIZE, "%s", sCmdFormat->cTextString);

    eErrorCode = utilCommonCLI_ICPC821_SET(&sCommonFormat);

    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilChristie_ICPC821_GET(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat) //A35G2_Larry_0088
{
    eCLI_ERROR_CODE eErrorCode;
    sCLI_COMMON_FORMAT sCommonFormat;

    sCommonFormat.eAccessMode = eAccessMode;
    sCommonFormat.ucCmdFrom = sCmdFormat->ucCmdFrom;
    sCommonFormat.ucDataType = sCmdFormat->ucDataType;
    sCommonFormat.lData = sCmdFormat->lData;
    sCommonFormat.fData = sCmdFormat->fData;

    snprintf(sCommonFormat.cTextString, CLI_DATA_SIZE, "%s", sCmdFormat->cTextString);

    eErrorCode = utilCommonCLI_ICPC821_GET(&sCommonFormat);

    sprintf(sCmdFormat->cTextString, "(%s+%s!%s)\n", sCmdFormat->cMainCode, sCmdFormat->cSubCode, sCommonFormat.cTextString);

    __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, sCmdFormat->cTextString);

    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilChristie_ICPC789_SET(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat) //A35G2_Larry_0088
{
    eCLI_ERROR_CODE eErrorCode;
    sCLI_COMMON_FORMAT sCommonFormat;

    sCommonFormat.eAccessMode = eAccessMode;
    sCommonFormat.ucCmdFrom = sCmdFormat->ucCmdFrom;
    sCommonFormat.ucDataType = sCmdFormat->ucDataType;
    sCommonFormat.lData = sCmdFormat->lData;
    sCommonFormat.fData = sCmdFormat->fData;

    snprintf(sCommonFormat.cTextString, CLI_DATA_SIZE, "%s", sCmdFormat->cTextString);

    eErrorCode = utilCommonCLI_ICPC789_SET(&sCommonFormat);

    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilChristie_ICPC789_GET(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat) //A35G2_Larry_0088
{
    eCLI_ERROR_CODE eErrorCode;
    sCLI_COMMON_FORMAT sCommonFormat;

    sCommonFormat.eAccessMode = eAccessMode;
    sCommonFormat.ucCmdFrom = sCmdFormat->ucCmdFrom;
    sCommonFormat.ucDataType = sCmdFormat->ucDataType;
    sCommonFormat.lData = sCmdFormat->lData;
    sCommonFormat.fData = sCmdFormat->fData;

    snprintf(sCommonFormat.cTextString, CLI_DATA_SIZE, "%s", sCmdFormat->cTextString);

    eErrorCode = utilCommonCLI_ICPC789_GET(&sCommonFormat);

    sprintf(sCmdFormat->cTextString, "(%s+%s!%s)\n", sCmdFormat->cMainCode, sCmdFormat->cSubCode, sCommonFormat.cTextString);

    __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, sCmdFormat->cTextString);

    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilChristie_ICPC341_SET(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)
{
    eCLI_ERROR_CODE eErrorCode;
    sCLI_COMMON_FORMAT sCommonFormat;

    sCommonFormat.eAccessMode = eAccessMode;
    sCommonFormat.ucCmdFrom = sCmdFormat->ucCmdFrom;
    sCommonFormat.ucDataType = sCmdFormat->ucDataType;
    sCommonFormat.lData = sCmdFormat->lData;
    sCommonFormat.fData = sCmdFormat->fData;

    snprintf(sCommonFormat.cTextString, CLI_DATA_SIZE, "%s", sCmdFormat->cTextString);

    eErrorCode = utilCommonCLI_ICPC341_SET(&sCommonFormat);

    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilChristie_ICPC341_GET(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)
{
    eCLI_ERROR_CODE eErrorCode;
    sCLI_COMMON_FORMAT sCommonFormat;

    sCommonFormat.eAccessMode = eAccessMode;
    sCommonFormat.ucCmdFrom = sCmdFormat->ucCmdFrom;
    sCommonFormat.ucDataType = sCmdFormat->ucDataType;
    sCommonFormat.lData = sCmdFormat->lData;
    sCommonFormat.fData = sCmdFormat->fData;

    snprintf(sCommonFormat.cTextString, CLI_DATA_SIZE, "%s", sCmdFormat->cTextString);

    eErrorCode = utilCommonCLI_ICPC341_GET(&sCommonFormat);

    sprintf(sCmdFormat->cTextString, "(%s+%s!%s)\n", sCmdFormat->cMainCode, sCmdFormat->cSubCode, sCommonFormat.cTextString);

    __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, sCmdFormat->cTextString);

    return eCLI_ERROR_CODE_NO;
}

#if 0
eCLI_ERROR_CODE util_ICD(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)
{
    char  ucOutputString[128] = {""};

    if(sCmdFormat->ucDataType == eCLI_DATA_TYPE_STRING)
    {
        UINT8 ucBank = 0;
        UINT8 ucReg = 0;
        UINT8 ucSize = 0;
        UINT8 ucData = 0;
        UINT8 aucData[3] = {0};
        UINT32 ulAddr = 0;
        UINT32 ulData = 0;

        //(ICD"W0,010101,010101")
        //(ICD"R0,010101")

        //ucBank
        aucData[0] = sCmdFormat->cTextString[3];
        aucData[1] = sCmdFormat->cTextString[4];
        AscIIToHex(aucData[0], aucData[1], &ucBank);

        //ucReg
        aucData[0] = sCmdFormat->cTextString[5];
        aucData[1] = sCmdFormat->cTextString[6];
        AscIIToHex(aucData[0], aucData[1], &ucReg);

        //ucSize
        aucData[0] = sCmdFormat->cTextString[7];
        aucData[1] = sCmdFormat->cTextString[8];
        AscIIToHex(aucData[0], aucData[1], &ucSize);

        ulAddr = ucBank<<16 | ucReg << 8 | ucSize;

        if((ucSize > 4) || (ucSize == 0))
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }

        if(sCmdFormat->cTextString[0] == 'R' && sCmdFormat->cTextString[1] == '0')//C821 Read
        {
            if(ulAddr == 0xFFFF01)
            {
                sICHIP_DUMP* m_pc821Reg;

                m_pc821Reg = (sICHIP_DUMP*)ICHIPC821_DUMP_ADDR;

                if(m_pc821Reg->ulAddress == 0x12345678)
                {
                    sprintf(ucOutputString, "C821 Addr 0x000000 Get 0x00000000\r\n");
                    __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);

                    m_pc821Reg++;

                    while(1)
                    {
                        if(m_pc821Reg->ulAddress == 0xFFFFFFFF)
                        {
                            sprintf(ucOutputString, "C821 Addr 0xFFFFFF Get 0xFFFFFFFF\r\n");
                            __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
                            return eCLI_ERROR_CODE_NO;
                        }
                        else
                        {

                             ulData = dvC821_Read(m_pc821Reg->ulAddress, 0);
                             sprintf(ucOutputString, "C821 Addr 0x%06x Get 0x%08x\r\n", m_pc821Reg->ulAddress, ulData);
                             __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);

                             utilDelayMs(10);
                        }
                        m_pc821Reg++;
                    }
                }
            }
            else
            {
                ulData = dvC821_Read(ulAddr, 0);
                sprintf(ucOutputString, "C821 Addr 0x%06x Get 0x%08x\r\n", ulAddr, ulData);
                __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
            }
        }
        else if(sCmdFormat->cTextString[0] == 'W' && sCmdFormat->cTextString[1] == '0') //C821 Write
        {
            ulData = 0;

            switch(ucSize)
            {
                case 4:
                    aucData[0] = sCmdFormat->cTextString[10];
                    aucData[1] = sCmdFormat->cTextString[11];
                    AscIIToHex(aucData[0], aucData[1], &ucData);
                    ulData |= (ucData << 24) & 0xFF000000;
                case 3:
                    aucData[0] = sCmdFormat->cTextString[12];
                    aucData[1] = sCmdFormat->cTextString[13];
                    AscIIToHex(aucData[0], aucData[1], &ucData);
                    ulData |= (ucData << 16) & 0x00FF0000;
                case 2:
                    aucData[0] = sCmdFormat->cTextString[14];
                    aucData[1] = sCmdFormat->cTextString[15];
                    AscIIToHex(aucData[0], aucData[1], &ucData);
                    ulData |= (ucData << 8) & 0x0000FF00;
                case 1:
                    aucData[0] = sCmdFormat->cTextString[16];
                    aucData[1] = sCmdFormat->cTextString[17];
                    AscIIToHex(aucData[0], aucData[1], &ucData);
                    ulData |= ucData & 0x000000FF;
                    break;

                default:
                    break;
            }

            dvC821_Write(ulAddr, ulData, 0);

            sprintf(ucOutputString, "C821 Addr 0x%06x Set 0x%08x\r\n", ulAddr, ulData);
            __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        }
        else if(sCmdFormat->cTextString[0] == 'R' && sCmdFormat->cTextString[1] == '1')//C789 Read
        {
            if(ulAddr == 0xFFFF01)
            {
                sICHIP_DUMP* m_pc789Reg;

                m_pc789Reg = (sICHIP_DUMP*)ICHIPC789_DUMP_ADDR;

                if(m_pc789Reg->ulAddress == 0x12345678)
                {
                    sprintf(ucOutputString, "C789 Addr 0x000000 Get 0x00000000\r\n");
                    __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);

                    m_pc789Reg++;

                    while(1)
                    {
                        if(m_pc789Reg->ulAddress == 0xFFFFFFFF)
                        {
                            sprintf(ucOutputString, "C789 Addr 0xFFFFFF Get 0xFFFFFFFF\r\n");
                            __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
                            return eCLI_ERROR_CODE_NO;
                        }
                        else
                        {

                             ulData = dvC789_Read(m_pc789Reg->ulAddress);
                             sprintf(ucOutputString, "C789 Addr 0x%06x Get 0x%08x\r\n", m_pc789Reg->ulAddress, ulData);
                             __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);

                             utilDelayMs(10);
                        }
                        m_pc789Reg++;
                    }
                }
            }
            else
            {
                ulData = dvC789_Read(ulAddr);

                sprintf(ucOutputString, "C789 Addr 0x%06x Get 0x%08x\r\n", ulAddr, ulData);
                __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
            }
        }
        else if(sCmdFormat->cTextString[0] == 'W' && sCmdFormat->cTextString[1] == '1') //C789 Write
        {
            ulData = 0;

            switch(ucSize)
            {
                case 4:
                    aucData[0] = sCmdFormat->cTextString[10];
                    aucData[1] = sCmdFormat->cTextString[11];
                    AscIIToHex(aucData[0], aucData[1], &ucData);
                    ulData |= (ucData << 24) & 0xFF000000;
                case 3:
                    aucData[0] = sCmdFormat->cTextString[12];
                    aucData[1] = sCmdFormat->cTextString[13];
                    AscIIToHex(aucData[0], aucData[1], &ucData);
                    ulData |= (ucData << 16) & 0x00FF0000;
                case 2:
                    aucData[0] = sCmdFormat->cTextString[14];
                    aucData[1] = sCmdFormat->cTextString[15];
                    AscIIToHex(aucData[0], aucData[1], &ucData);
                    ulData |= (ucData << 8) & 0x0000FF00;
                case 1:
                    aucData[0] = sCmdFormat->cTextString[16];
                    aucData[1] = sCmdFormat->cTextString[17];
                    AscIIToHex(aucData[0], aucData[1], &ucData);
                    ulData |= ucData & 0x000000FF;
                    break;

                default:
                    break;
            }


            dvC789_Write(ulAddr, ulData);

            sprintf(ucOutputString, "C789 Addr 0x%06x Set 0x%08x\r\n", ulAddr, ulData);
            __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        }
        else
        {
            sprintf(ucOutputString, "ichip comamnd fail!!\r\n");
            __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        }
    }
    else
    {

        sprintf(ucOutputString, "comamnd fail!!\r\n");
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
    }

    return eCLI_ERROR_CODE_NO;
}
#endif /* 0 */



eCLI_ERROR_CODE util_FPGA_System(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat) //A70LV_Larry_0086
{
    char  ucOutputString[128] = {""};

    if(sCmdFormat->ucDataType == eCLI_DATA_TYPE_STRING)
    {
        UINT8 ucReg = 0;
        UINT8 ucData = 0;
        UINT8 aucData[3] = {0};

        //ucReg
        aucData[0] = sCmdFormat->cTextString[3];
        aucData[1] = sCmdFormat->cTextString[4];
        AscIIToHex(aucData[0], aucData[1], &ucReg);

        if(sCmdFormat->cTextString[0] == 'R' && sCmdFormat->cTextString[1] == '1')
        {
            dvFPGA_System_Read((UINT16)ucReg,&ucData);

            sprintf(ucOutputString, "FPGA System R REG 0x%02x, 0x%02x\r\n", ucReg, ucData);
            __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        }
        else if(sCmdFormat->cTextString[0] == 'W' && sCmdFormat->cTextString[1] == '1')
        {
            //ucReg
            aucData[0] = sCmdFormat->cTextString[6];
            aucData[1] = sCmdFormat->cTextString[7];
            AscIIToHex(aucData[0], aucData[1], &ucData);

            dvFPGA_System_Write((UINT16)ucReg,&ucData);

            sprintf(ucOutputString, "FPGA System W REG 0x%02x, 0x%02x\r\n", ucReg, ucData);
            __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        }
        else
        {
            sprintf(ucOutputString, "FPGA_System comamnd fail!!\r\n");
            __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        }
    }
    else
    {
        sprintf(ucOutputString, "FPGA_System comamnd fail!!\r\n");
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
    }


    return eCLI_ERROR_CODE_NO;
}

#if 0
eCLI_ERROR_CODE utilChristie_MIF_APRT(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat) //A70LV_Doulas_0137
{
    UINT8   ucAspectRatio = CLI_SCALING_MODE_AUTO ;
    char    ucOutputString[128] = {""};

    if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(edcMAIN_ASPECT_RATIO))  //available check
    {
        return eCLI_ERROR_CODE_MUSTDEPENDONSRC;
    }

    if(eAccessMode == ecmRead)
    {
        palDataMgr_Data_Access(edcMAIN_ASPECT_RATIO, edaREAD, &ucAspectRatio) ;
        ucAspectRatio = (UINT8)CM2CLI(edcMAIN_ASPECT_RATIO, ucAspectRatio); //HICC2_Doulas_0065
        switch(ucAspectRatio)
        {
            case CLI_SCALING_MODE_AUTO:
                sprintf(ucOutputString, "\r(mif+aprt!\"%s\" )\r\n","Auto");
                __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
                break;
            case CLI_SCALING_MODE_NATIVE:
                sprintf(ucOutputString, "\r(mif+aprt!\"%s\" )\r\n","Native");
                __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
                break;
            case CLI_SCALING_MODE_4_3:
                sprintf(ucOutputString, "\r(mif+aprt!\"%s\" )\r\n","4:3");
                __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
                break;
            case CLI_SCALING_MODE_LETTER_BOX:
                sprintf(ucOutputString, "\r(mif+aprt!\"%s\" )\r\n","Letter Box");
                __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
                break;
            case CLI_SCALING_MODE_FULL_SIZE:
                sprintf(ucOutputString, "\r(mif+aprt!\"%s\" )\r\n","Full Size");
                __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
                break;
            case CLI_SCALING_MODE_FULL_WIDTH:
                sprintf(ucOutputString, "\r(mif+aprt!\"%s\" )\r\n","Full Width");
                __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
                break;
            case CLI_SCALING_MODE_FULL_HEIGHT:
                sprintf(ucOutputString, "\r(mif+aprt!\"%s\" )\r\n","Full Height");
                __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
                break;
            case CLI_SCALING_MODE_CUSTOM:
                sprintf(ucOutputString, "\r(mif+aprt!\"%s\" )\r\n","Custom");
                __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
                break;
            case CLI_SCALING_MODE_3D:
                sprintf(ucOutputString, "\r(mif+aprt!\"%s\" )\r\n","3D");
                __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
                break;

            case CLI_SCALING_MODE_21_9:  //HICC2_Doulas_0065
                sprintf(ucOutputString, "\r(mif+aprt!\"%s\" )\r\n","21:9");
                __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
                break;
        }
    }
    else if(eAccessMode == ecmWrite)
    {
        return eCLI_ERROR_CODE_COMMAND_READ_ONLY;
    }
    return eCLI_ERROR_CODE_NO;
}


eCLI_ERROR_CODE utilChristie_SIF_APRT(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat) //A70LV_Doulas_0137
{
    UINT8 ucAspectRatio = CLI_SCALING_MODE_AUTO;
    char    ucOutputString[128] = {""};

    if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(edcSUB_ASPECT_RATIO))  //available check
    {
        return eCLI_ERROR_CODE_MUSTDEPENDONSRC;
    }

    if(eAccessMode == ecmRead)
    {
        palDataMgr_Data_Access(edcSUB_ASPECT_RATIO, edaREAD, &ucAspectRatio) ;
        ucAspectRatio = (UINT8)CM2CLI(edcSUB_ASPECT_RATIO, ucAspectRatio); //HICC2_Doulas_0065
        switch(ucAspectRatio)
        {
            case CLI_SCALING_MODE_AUTO:
                sprintf(ucOutputString, "\r(sif+aprt!\"%s\" )\r\n","Auto");
                __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
                break;
            case CLI_SCALING_MODE_NATIVE:
                sprintf(ucOutputString, "\r(sif+aprt!\"%s\" )\r\n","Native");
                __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
                break;
            case CLI_SCALING_MODE_4_3:
                sprintf(ucOutputString, "\r(sif+aprt!\"%s\" )\r\n","4:3");
                __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
                break;
            case CLI_SCALING_MODE_LETTER_BOX:
                sprintf(ucOutputString, "\r(sif+aprt!\"%s\" )\r\n","Letter Box");
                __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
                break;
            case CLI_SCALING_MODE_FULL_SIZE:
                sprintf(ucOutputString, "\r(sif+aprt!\"%s\" )\r\n","Full Size");
                __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
                break;
            case CLI_SCALING_MODE_FULL_WIDTH:
                sprintf(ucOutputString, "\r(sif+aprt!\"%s\" )\r\n","Full Width");
                __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
                break;
            case CLI_SCALING_MODE_FULL_HEIGHT:
                sprintf(ucOutputString, "\r(sif+aprt!\"%s\" )\r\n","Full Height");
                __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
                break;
            case CLI_SCALING_MODE_CUSTOM:
                sprintf(ucOutputString, "\r(sif+aprt!\"%s\" )\r\n","Custom");
                __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
                break;
            case CLI_SCALING_MODE_3D:
                sprintf(ucOutputString, "\r(sif+aprt!\"%s\" )\r\n","3D");
                __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
                break;

            case CLI_SCALING_MODE_21_9: //HICC2_Doulas_0065
                sprintf(ucOutputString, "\r(sif+aprt!\"%s\" )\r\n","21:9");
                __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
                break;
        }
    }
    else if(eAccessMode == ecmWrite)
    {
        return eCLI_ERROR_CODE_COMMAND_READ_ONLY;
    }
    return eCLI_ERROR_CODE_NO;
}
#endif /* 0 */

eCLI_ERROR_CODE utilChristie_DEF(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)   //A70LV_Doulas_0138
{
    UINT8 ucValue = 1;
    eEXEC_CODE eResult = eEXEC_CODE_PASS;

    if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(edcFACTORY_RESET))  //available check
    {
        return eCLI_ERROR_CODE_DataSpecialCheckFail;
    }

    if(eAccessMode == ecmRead)
    {
        return eCLI_ERROR_CODE_COMMAND_WRITE_ONLY;
    }
    else if(eAccessMode == ecmWrite)
    {
        if(sCmdFormat->lData == 111)
        {
            eResult = palDataMgr_Data_Access(edcFACTORY_RESET, edaWRITE_THROUGH_WITH_ACTION, &ucValue) ;
            if(eResult != eEXEC_CODE_PASS)
            {
                return eCLI_ERROR_CODE_SETFAIL;
            }
        }
        else
            return eCLI_ERROR_CODE_DATAERROR;
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilChristie_CWI_PF2X(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)   //A70LV_Doulas_0138
{
    UINT16 iWheelIndex[2] = {0};
    eEXEC_CODE eResult = eEXEC_CODE_PASS;

    if(palDataMgr_Data_Access(edcWHEEL_INDEX_2X, edaREAD, &iWheelIndex) != eEXEC_CODE_PASS)
    {
        if(eAccessMode == ecmRead)
            return eCLI_ERROR_CODE_REQUESTFAIL;
        else
            return eCLI_ERROR_CODE_SETFAIL;
    }

    if(eAccessMode == ecmRead)
    {
        sCmdFormat->lData = (INT32) iWheelIndex[0];
    }
    else if(eAccessMode == ecmWrite)
    {
        if((sCmdFormat->lData <= PW_INDEX_MAX_VALUE) && (sCmdFormat->lData >= PW_INDEX_MIN_VALUE))
        {
            iWheelIndex[0] = (UINT16)sCmdFormat->lData;
            eResult = palDataMgr_Data_Access(edcWHEEL_INDEX_2X, edaWRITE_THROUGH_WITH_ACTION, &iWheelIndex) ;
            if(eResult != eEXEC_CODE_PASS)
            {
                return eCLI_ERROR_CODE_SETFAIL;
            }
        }
        else
        {
            return eCLI_ERROR_CODE_DATAERROR;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilChristie_CWI_FT2X(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)   //A70LV_Doulas_0138
{
    UINT16 iWheelIndex[2] = {0};
    eEXEC_CODE eResult = eEXEC_CODE_PASS;

    if(palDataMgr_Data_Access(edcWHEEL_INDEX_2X, edaREAD, &iWheelIndex) != eEXEC_CODE_PASS)
    {
        if(eAccessMode == ecmRead)
            return eCLI_ERROR_CODE_REQUESTFAIL;
        else
            return eCLI_ERROR_CODE_SETFAIL;
    }

    if(eAccessMode == ecmRead)
    {
        sCmdFormat->lData = (INT32) iWheelIndex[1];
    }
    else if(eAccessMode == ecmWrite)
    {
        if((sCmdFormat->lData <= PW_INDEX_MAX_VALUE) && (sCmdFormat->lData >= PW_INDEX_MIN_VALUE))
        {
            iWheelIndex[1] = (UINT16)sCmdFormat->lData;
            eResult = palDataMgr_Data_Access(edcWHEEL_INDEX_2X, edaWRITE_THROUGH_WITH_ACTION, &iWheelIndex) ;
            if(eResult != eEXEC_CODE_PASS)
            {
                return eCLI_ERROR_CODE_SETFAIL;
            }
        }
        else
        {
            return eCLI_ERROR_CODE_DATAERROR;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilChristie_CWI_PF3X(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)   //A70LV_Doulas_0138
{
    //UINT16 iWheelIndex[2] = {0};
    UINT16 iWheelIndex = 0;
    eEXEC_CODE eResult = eEXEC_CODE_PASS;

    if(palDataMgr_Data_Access(edcPHOSPHOR_WHEEL_INDEX, edaREAD, &iWheelIndex) != eEXEC_CODE_PASS)
    {
        if(eAccessMode == ecmRead)
            return eCLI_ERROR_CODE_REQUESTFAIL;
        else
            return eCLI_ERROR_CODE_SETFAIL;
    }

    if(eAccessMode == ecmRead)
    {
        //sCmdFormat->lData = (INT32) iWheelIndex[0];
        sCmdFormat->lData = (INT32) iWheelIndex;
    }
    else if(eAccessMode == ecmWrite)
    {
        if((sCmdFormat->lData <= PW_INDEX_MAX_VALUE) && (sCmdFormat->lData >= PW_INDEX_MIN_VALUE))
        {
            //iWheelIndex[0] = (UINT16)sCmdFormat->lData;
            iWheelIndex = (UINT16)sCmdFormat->lData;
            eResult = palDataMgr_Data_Access(edcPHOSPHOR_WHEEL_INDEX, edaWRITE_THROUGH_WITH_ACTION, &iWheelIndex) ;
            if(eResult != eEXEC_CODE_PASS)
            {
                return eCLI_ERROR_CODE_SETFAIL;
            }
        }
        else
        {
            return eCLI_ERROR_CODE_DATAERROR;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilChristie_CWI_FT3X(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)   //A70LV_Doulas_0138
{
    //UINT16 iWheelIndex[2] = {0};
    UINT16 iWheelIndex = 0;
    eEXEC_CODE eResult = eEXEC_CODE_PASS;

    if(palDataMgr_Data_Access(edcFILTER_WHEEL_INDEX, edaREAD, &iWheelIndex) != eEXEC_CODE_PASS)
    {
        if(eAccessMode == ecmRead)
            return eCLI_ERROR_CODE_REQUESTFAIL;
        else
            return eCLI_ERROR_CODE_SETFAIL;
    }

    if(eAccessMode == ecmRead)
    {
        //sCmdFormat->lData = (INT32) iWheelIndex[1];
        sCmdFormat->lData = (INT32) iWheelIndex;
    }
    else if(eAccessMode == ecmWrite)
    {
        if((sCmdFormat->lData <= PW_INDEX_MAX_VALUE) && (sCmdFormat->lData >= PW_INDEX_MIN_VALUE))
        {
            //iWheelIndex[1] = (UINT16)sCmdFormat->lData;
            iWheelIndex = (UINT16)sCmdFormat->lData;
            eResult = palDataMgr_Data_Access(edcFILTER_WHEEL_INDEX, edaWRITE_THROUGH_WITH_ACTION, &iWheelIndex) ;
            if(eResult != eEXEC_CODE_PASS)
            {
                return eCLI_ERROR_CODE_SETFAIL;
            }
        }
        else
        {
            return eCLI_ERROR_CODE_DATAERROR;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilChristie_PIV(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)       //A70LV_Doulas_0139
{
    eEXEC_CODE eResult = eEXEC_CODE_PASS;
    UINT8 ucPIN_PROTECT_EN;
    //char  ucOutputString[128] = {""};

    if(palDataMgr_Data_Access(edcPIN_PROTECT, edaREAD, &ucPIN_PROTECT_EN) != eEXEC_CODE_PASS)
    {
        if(eAccessMode == ecmRead)
            return eCLI_ERROR_CODE_REQUESTFAIL;
        else
            return eCLI_ERROR_CODE_SETFAIL;
    }

    if(eAccessMode == ecmRead)
    {
        sCmdFormat->lData = ucPIN_PROTECT_EN;
    }
    else if(eAccessMode == ecmWrite)
    {
        if(sCmdFormat->ucDataType == eCLI_DATA_TYPE_STRING)
        {
            UINT8 ucPWord[5] = {0};
            UINT8 ucPWord_Input[5] = {0};
            UINT8 ucVAl;

            //sprintf(ucOutputString, "utilChristie_PIV (%s) \r\n", sCmdFormat->cTextString);
            //__CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);

            if(strlen((char *)sCmdFormat->cTextString) != 5)
            {
                return eCLI_ERROR_CODE_DATAOVERRANGE;
            }

            if(palDataMgr_Data_Access(edcCHANGE_PIN, edaREAD, ucPWord) != eEXEC_CODE_PASS)
            {
                return eCLI_ERROR_CODE_SETFAIL;
            }

            ucVAl = 0;
            for(; ucVAl < 5 ; ucVAl++)
            {
                if((sCmdFormat->cTextString[ucVAl] >= '0') &&
                   (sCmdFormat->cTextString[ucVAl] <= '9'))
                {
                    ucPWord_Input[ucVAl] = sCmdFormat->cTextString[ucVAl] ;                       //T100_Simon_0048
                    if(ucPWord_Input[ucVAl] != ucPWord[ucVAl])
                    {
                        return eCLI_ERROR_CODE_DATAERROR;
                    }
                }
                else
                {
                    return eCLI_ERROR_CODE_DATAERROR;
                }
            }

            ucPIN_PROTECT_EN = !ucPIN_PROTECT_EN;

            eResult = palDataMgr_Data_Access(edcPIN_PROTECT, edaWRITE_THROUGH_WITH_ACTION, &ucPIN_PROTECT_EN) ;
            if(ucPIN_PROTECT_EN == 0)	//A35G2_CDS_Coda_0033
            {
                palDataMgr_UI_EventSend(edcUI_EVENT_PIN_PROTECT_CLEAR, TRUE, NULL); //appGui_SendPinProtectClearEvent(); //HICC2_Julie_0021
            }

            if(eResult != eEXEC_CODE_PASS)
            {
                return eCLI_ERROR_CODE_SETFAIL;
            }
        }
        else
        {
            return eCLI_ERROR_CODE_DATAERROR;
        }

    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilChristie_PCG(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)       //A70LV_Doulas_0139
{
    eEXEC_CODE eResult = eEXEC_CODE_PASS;
    char  ucOutputString[128] = {""};

    if(eAccessMode == ecmRead)
    {
        return eCLI_ERROR_CODE_COMMAND_WRITE_ONLY;
    }
    else if(eAccessMode == ecmWrite)
    {
        if(sCmdFormat->ucDataType == eCLI_DATA_TYPE_STRING)
        {
            UINT8 ucPWord[5] = {0};
            UINT8 ucPWord_Input[5] = {0};
            UINT8 ucNewPWord[5] = {0};
            UINT8 ucVAl;

            //sprintf(ucOutputString, "utilChristie_PCG (%s) \r\n", sCmdFormat->cTextString);
            //__CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);

            if(strlen((char *)sCmdFormat->cTextString) != 11)
            {
                return eCLI_ERROR_CODE_DATAOVERRANGE;
            }

            if(palDataMgr_Data_Access(edcCHANGE_PIN, edaREAD, ucPWord) != eEXEC_CODE_PASS)
            {
                return eCLI_ERROR_CODE_SETFAIL;
            }

            ucVAl = 0;
            for(; ucVAl < 5 ; ucVAl++)      //check password
            {
                if((sCmdFormat->cTextString[ucVAl] >= '0') &&
                   (sCmdFormat->cTextString[ucVAl] <= '9'))
                {
                    ucPWord_Input[ucVAl] = sCmdFormat->cTextString[ucVAl];  //(sCmdFormat->cTextString[ucVAl] - '0' ) + ek0;       //A70LV_Doulas_0264 modify
                    if(ucPWord_Input[ucVAl] != ucPWord[ucVAl])
                    {
                        //LOG_MSG(db_ALWAYS, "111 %d,%d,%d,%d,%d \r\n",ucPWord[0],ucPWord[1],ucPWord[2],ucPWord[3],ucPWord[4]); //A70LV_Doulas_0264
                        return eCLI_ERROR_CODE_DATAERROR;
                    }
                }
                else
                {
                    return eCLI_ERROR_CODE_DATAERROR;
                }
            }

            if(sCmdFormat->cTextString[5] != ',')
            {
                return eCLI_ERROR_CODE_DATAOVERRANGE;
            }

            ucVAl = 6;
            for(; ucVAl < 11 ; ucVAl++)     //check new password
            {
                if((sCmdFormat->cTextString[ucVAl] >= '0') &&
                   (sCmdFormat->cTextString[ucVAl] <= '9'))
                {
                    ucNewPWord[ucVAl - 6] = sCmdFormat->cTextString[ucVAl];//(sCmdFormat->cTextString[ucVAl] - '0' ) + ek0;      ////A70LV_Doulas_0264 modify
                }
                else
                {
                    return eCLI_ERROR_CODE_DATAERROR;
                }
            }

            eResult = palDataMgr_Data_Access(edcCHANGE_PIN, edaWRITE_THROUGH_NO_ACTION, ucNewPWord) ;
            if(eResult != eEXEC_CODE_PASS)
            {
                return eCLI_ERROR_CODE_SETFAIL;
            }
        }
        else
        {
            return eCLI_ERROR_CODE_DATAERROR;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

#if 1
eCLI_ERROR_CODE utilChristie_SHU_LOCK(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat) //A70LV_Larry_0384
{
    if(eAccessMode == ecmWrite)
    {
        UINT8 ucIndex = 0;
        sDATABASE_ITEM_DATA_FORMAT sInputData;

        if(sCmdFormat->lData == 0)
        {
            INT16 uiShutter = 0;

            palDataMgr_Data_Access(edcLENS_DETECTION, edaWRITE_THROUGH_WITH_ACTION, &uiShutter) ;//appGui_DataCode_Value_Set(edcLENS_DETECTION, edaWRITE_THROUGH_WITH_ACTION, 0);
            palDataMgr_Data_Access(edcPICTURE_MUTE, edaREAD, &uiShutter); //appGui_DataCode_Value_Get(edcPICTURE_MUTE, (tDATA_CODE*)&uiShutter);

            if(uiShutter)
            {
                ucIndex = 0;
                sInputData.uiItemIndex = edcPICTURE_MUTE;
                sInputData.uValue.lValue = (UINT32)ucIndex;
                palDataMgr_NotifyGroupingEvent(sInputData);
                palDataMgr_Data_Access(edcPICTURE_MUTE, edaWRITE_THROUGH_WITH_ACTION, &ucIndex) ;//appGui_DataCode_Value_Set(edcPICTURE_MUTE, edaWRITE_THROUGH_WITH_ACTION, 0);
            }
        }
        else
        {
            ucIndex = 1;
            sInputData.uiItemIndex = edcPICTURE_MUTE;
            sInputData.uValue.lValue = (UINT32)ucIndex;
            palDataMgr_NotifyGroupingEvent(sInputData);
            palDataMgr_Data_Access(edcLENS_DETECTION, edaWRITE_THROUGH_WITH_ACTION, &ucIndex) ;//appGui_DataCode_Value_Set(edcLENS_DETECTION, edaWRITE_THROUGH_WITH_ACTION, 1);
        }
    }
    return eCLI_ERROR_CODE_NO;
}
#endif /* 0 */

eCLI_ERROR_CODE utilChristie_SIV(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)       //A70LV_Doulas_0139
{
    if(eAccessMode == ecmRead)
    {
        sCmdFormat->lData = CHRISTIE_MSSC_VER;
    }
    else if(eAccessMode == ecmWrite)
    {
        return eCLI_ERROR_CODE_COMMAND_READ_ONLY;
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilChristie_SST(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)       //A70LV_Doulas_0140
{
    UINT8 ucString[64] = {0};
    char  ucOutputString[128] = {""};
    UINT8 ucIndex = 0;
    UINT8 ucVal;
    UINT8 ucPIP_EN;
    UINT16 uiDelay = 0;

    if(eccHDBaseT == sCmdFormat->ucCmdFrom)
    {
        uiDelay = 10; //for baudrate 1200
    }

    if(eAccessMode == ecmRead)
    {
        palDataMgr_Data_Access(edcMODEL_NAME, edaREAD, ucString);
        sprintf(ucOutputString, "\r(SST!%03d  \"%s\" \"Model Name\")\r\n",ucIndex, ucString);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        ucIndex++;

        if(uiDelay)
        {
            MS_SLEEP(uiDelay);
        }

        palDataMgr_Data_Access(edcSERIAL_NUMBER, edaREAD, ucString);
        sprintf(ucOutputString, "\r(SST!%03d  \"%s\" \"Serial Number\")\r\n",ucIndex, ucString);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        ucIndex++;

        if(uiDelay)
        {
            MS_SLEEP(uiDelay);
        }

        palDataMgr_Data_Access(edcNATIVE_RESOLUTION, edaREAD, ucString);
        sprintf(ucOutputString, "\r(SST!%03d  \"%s\" \"Native Resolution\")\r\n",ucIndex, ucString);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        ucIndex++;

        if(uiDelay)
        {
            MS_SLEEP(uiDelay);
        }

        //palDataMgr_Data_Access(edcFIRMWARE, edaREAD, ucString);                               //A70LV_Doulas_0227 remove
        //sprintf(ucOutputString, "\r(SST!%03d  \"%s\" \"Firmware\")\r\n",ucIndex, ucString);
        //__CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        //ucIndex++;

        palDataMgr_Data_Access(edcMAIN_INPUT, edaREAD, &ucVal);
        switch(ucVal)
        {
            case CLI_SOURCE_HDMI1:
                sprintf(ucOutputString, "\r(SST!%03d  \"%s\" \"Main Input\")\r\n",ucIndex, "HDMI 1");
                __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
                break;

             case CLI_SOURCE_HDMI2:
                sprintf(ucOutputString, "\r(SST!%03d  \"%s\" \"Main Input\")\r\n",ucIndex, "HDMI 2");
                __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
                break;

            case CLI_SOURCE_DP:	//A70Gen2_Doulas_0004
                sprintf(ucOutputString, "\r(SST!%03d  \"%s\" \"Main Input\")\r\n",ucIndex, "DISPLAYPORT");
                __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
                break;

            case CLI_SOURCE_3GSDI:
                sprintf(ucOutputString, "\r(SST!%03d  \"%s\" \"Main Input\")\r\n",ucIndex, "3GSDI");
                __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
                break;

            case CLI_SOURCE_HDBASET:
                sprintf(ucOutputString, "\r(SST!%03d  \"%s\" \"Main Input\")\r\n",ucIndex, "HDBaseT");
                __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
                break;
        }
        ucIndex++;

        if(uiDelay)
        {
            MS_SLEEP(uiDelay);
        }

        palDataMgr_Data_Access(edcMAIN_SIGNAL_FORMAT, edaREAD, ucString);
        sprintf(ucOutputString, "\r(SST!%03d  \"%s\" \"Main Signal Format\")\r\n",ucIndex, ucString);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        ucIndex++;

        if(uiDelay)
        {
            MS_SLEEP(uiDelay);
        }

        palDataMgr_Data_Access(edcMAIN_PIXEL_CLOCK, edaREAD, ucString);
        sprintf(ucOutputString, "\r(SST!%03d  \"%s\" \"Main Pixel Clock\")\r\n",ucIndex, ucString);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        ucIndex++;

        if(uiDelay)
        {
            MS_SLEEP(uiDelay);
        }

        palDataMgr_Data_Access(edcMAIN_SYNC_TYPE, edaREAD, ucString);
        sprintf(ucOutputString, "\r(SST!%03d  \"%s\" \"Main Sync Type\")\r\n",ucIndex, ucString);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        ucIndex++;

        if(uiDelay)
        {
            MS_SLEEP(uiDelay);
        }

        palDataMgr_Data_Access(edcMAIN_HORZ_REFRESH, edaREAD, ucString);
        sprintf(ucOutputString, "\r(SST!%03d  \"%s\" \"Main Horz Refresh\")\r\n",ucIndex, ucString);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        ucIndex++;

        if(uiDelay)
        {
            MS_SLEEP(uiDelay);
        }

        palDataMgr_Data_Access(edcMAIN_VERT_REFRESH, edaREAD, ucString);
        sprintf(ucOutputString, "\r(SST!%03d  \"%s\" \"Main Vert Refresh\")\r\n",ucIndex, ucString);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        ucIndex++;

        if(uiDelay)
        {
            MS_SLEEP(uiDelay);
        }

    //----------------------
        palDataMgr_Data_Access(edcPIP_PBP_ENABLE, edaREAD, &ucPIP_EN);
        if(ucPIP_EN == ets_ON)
        {
            palDataMgr_Data_Access(edcSUB_INPUT, edaREAD, &ucVal);
            switch(ucVal)
            {
            case CLI_SOURCE_HDMI1:
                sprintf(ucOutputString, "\r(SST!%03d  \"%s\" \"PIP / PBP Input\")\r\n",ucIndex, "HDMI 1");
                __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
                break;

             case CLI_SOURCE_HDMI2:
                sprintf(ucOutputString, "\r(SST!%03d  \"%s\" \"PIP / PBP Input\")\r\n",ucIndex, "HDMI 2");
                __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
                break;

            case CLI_SOURCE_DP:	//A70Gen2_Doulas_0004
                sprintf(ucOutputString, "\r(SST!%03d  \"%s\" \"PIP / PBP Input\")\r\n",ucIndex, "DISPLAYPORT");
                __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
                break;

            case CLI_SOURCE_3GSDI:
                sprintf(ucOutputString, "\r(SST!%03d  \"%s\" \"PIP / PBP Input\")\r\n",ucIndex, "3GSDI");
                __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
                break;

            case CLI_SOURCE_HDBASET:
                sprintf(ucOutputString, "\r(SST!%03d  \"%s\" \"PIP / PBP Input\")\r\n",ucIndex, "HDBaseT");
                __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
                break;
            }
        }
        else
        {
            sprintf(ucOutputString, "\r(SST!%03d  \"%s\" \"PIP / PBP Input\")\r\n",ucIndex, "-");
            __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        }
        ucIndex++;

        if(uiDelay)
        {
            MS_SLEEP(uiDelay);
        }

        palDataMgr_Data_Access(edcSUB_SIGNAL_FORMAT, edaREAD, ucString);
        sprintf(ucOutputString, "\r(SST!%03d  \"%s\" \"PIP / PBP Signal Format\")\r\n",ucIndex, ucString);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        ucIndex++;

        if(uiDelay)
        {
            MS_SLEEP(uiDelay);
        }

        palDataMgr_Data_Access(edcSUB_PIXEL_CLOCK, edaREAD, ucString);
        sprintf(ucOutputString, "\r(SST!%03d  \"%s\" \"PIP / PBP Pixel Clock\")\r\n",ucIndex, ucString);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        ucIndex++;

        if(uiDelay)
        {
            MS_SLEEP(uiDelay);
        }

        palDataMgr_Data_Access(edcSUB_SYNC_TYPE, edaREAD, ucString);
        sprintf(ucOutputString, "\r(SST!%03d  \"%s\" \"PIP / PBP Sync Type\")\r\n",ucIndex, ucString);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        ucIndex++;

        if(uiDelay)
        {
            MS_SLEEP(uiDelay);
        }

        palDataMgr_Data_Access(edcSUB_HORZ_REFRESH, edaREAD, ucString);
        sprintf(ucOutputString, "\r(SST!%03d  \"%s\" \"PIP / PBP Horz Refresh\")\r\n",ucIndex, ucString);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        ucIndex++;

        if(uiDelay)
        {
            MS_SLEEP(uiDelay);
        }

        palDataMgr_Data_Access(edcSUB_VERT_REFRESH, edaREAD, ucString);
        sprintf(ucOutputString, "\r(SST!%03d  \"%s\" \"PIP / PBP Vert Refresh\")\r\n",ucIndex, ucString);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        ucIndex++;

        if(uiDelay)
        {
            MS_SLEEP(uiDelay);
        }
    //----------------------
        palDataMgr_Data_Access(edcINFO_LIGHT_SOURCE_POWER, edaREAD, ucString);
        sprintf(ucOutputString, "\r(SST!%03d  \"%s\" \"Light Source Power\")\r\n",ucIndex, ucString);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        ucIndex++;

        palDataMgr_Data_Access(edcSTANDBY_MODE, edaREAD, &ucVal);
        if(ucVal == eSTANDBY_MODE_0d5W)
        {
            sprintf(ucOutputString, "\r(SST!%03d  \"%s\" \"Standby Mode\")\r\n",ucIndex, "ECO Mode");//HICC2_Julie_0031
            __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        }
        else
        {
            sprintf(ucOutputString, "\r(SST!%03d  \"%s\" \"Standby Mode\")\r\n",ucIndex, "Communication Mode");
            __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        }
        ucIndex++;

        //if(palSystem_CustomerSplashGet() != CUSTOM_LOGO_CHRISTIE_CHINA) //A70LV_Julie_0043
        {
            ucVal = palGeo_ApLinkFlag_Get();  //A35G2_Simon_0086
            if(ucVal == 0)
            {
                sprintf(ucOutputString, "\r(SST!%03d  \"%s\" \"Twist LINK\")\r\n",ucIndex, "Off");	//A70LV_Julie_0016
            }
            else
            {
                sprintf(ucOutputString, "\r(SST!%03d  \"%s\" \"Twist LINK\")\r\n",ucIndex, "On");	//A70LV_Julie_0016
            }
            __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
            ucIndex++;

            if(uiDelay)
            {
                MS_SLEEP(uiDelay);
            }
        }

        palDataMgr_Data_Access(edcCOLOR_SPACE, edaREAD, &ucVal);
        if(ucVal == 0)
        {
           sprintf(ucOutputString, "\r(SST!%03d  \"%s\" \"Color Space Settings\")\r\n",ucIndex, "Auto");
        }
        else if(ucVal == 1)
        {
           sprintf(ucOutputString, "\r(SST!%03d  \"%s\" \"Color Space Settings\")\r\n",ucIndex, "RGB Full");
        }
        else if(ucVal == 2)
        {
           sprintf(ucOutputString, "\r(SST!%03d  \"%s\" \"Color Space Settings\")\r\n",ucIndex, "RGB Limited");
        }
        else if(ucVal == 3)
        {
           sprintf(ucOutputString, "\r(SST!%03d  \"%s\" \"Color Space Settings\")\r\n",ucIndex, "REC709");
        }
        else
        {
           sprintf(ucOutputString, "\r(SST!%03d  \"%s\" \"Color Space Settings\")\r\n",ucIndex, "REC601");
        }
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        ucIndex++;

        if(uiDelay)
        {
            MS_SLEEP(uiDelay);
        }


        palDataMgr_Data_Access(edcLOCK_ALL_LENS_MOTORS, edaREAD, &ucVal);
        if(ucVal == eLENS_LOCK_ALL_LOCKED)
        {
            sprintf(ucOutputString, "\r(SST!%03d  \"%s\" \"Lock all Lens Motors\")\r\n",ucIndex, "Locked"); //A70Gen2_Julie_0035
            __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        }
        else
        {
            sprintf(ucOutputString, "\r(SST!%03d  \"%s\" \"Lock all Lens Motors\")\r\n",ucIndex, "Allow"); //A70Gen2_Julie_0035
            __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        }
        ucIndex++;

        if(uiDelay)
        {
            MS_SLEEP(uiDelay);
        }

        palDataMgr_Data_Access(edcSYSTEM_TEMPERATURE, edaREAD, ucString);
        sprintf(ucOutputString, "\r(SST!%03d  \"%s\" \"System Temperature\")\r\n",ucIndex, ucString);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        ucIndex++;

        if(uiDelay)
        {
            MS_SLEEP(uiDelay);
        }

        palDataMgr_Data_Access(edcAMBIENT_TEMPERATURE, edaREAD, ucString);
        sprintf(ucOutputString, "\r(SST!%03d  \"%s\" \"Ambient Temperature\")\r\n",ucIndex, ucString);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        ucIndex++;

        if(uiDelay)
        {
            MS_SLEEP(uiDelay);
        }
    }
    else if(eAccessMode == ecmWrite)
    {
        return eCLI_ERROR_CODE_COMMAND_READ_ONLY;
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilChristie_ILI(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)       //A70LV_Doulas_0140
{
    UINT8 ucString[64] = {0};
    char  ucOutputString[128] = {""};
    UINT8 ucIndex = 0;

    if(eAccessMode == ecmRead)
    {
        palDataMgr_Data_Access(edcTOTAL_PROJECTOR_HOURS_INFO, edaREAD, ucString);
        sprintf(ucOutputString, "\r(ILI!%03d  \"%s\" \"Total Projector Hours\")\r\n",ucIndex, ucString);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        ucIndex++;

        palDataMgr_Data_Access(edcLD_HOURS_INFO, edaREAD, ucString);
        sprintf(ucOutputString, "\r(ILI!%03d  \"%s\" \"LD Hours\")\r\n",ucIndex, ucString);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        ucIndex++;
    }
    else if(eAccessMode == ecmWrite)
    {
        return eCLI_ERROR_CODE_COMMAND_READ_ONLY;
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilChristie_ICI(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)       //A70LV_Doulas_0231 modify//A70LV_Doulas_0151
{
    UINT8 ucString[64] = {0};
    char  ucOutputString[128] = {""};
    UINT8 ucIndex = 0;
    UINT8 ucVal;

    if(eAccessMode == ecmRead)
    {
        ucString[0] = '\0';
        //sprintf(ucOutputString, "\r(ICI!%03d  \"%s\" \"LAN\")\r\n",ucIndex, ucString);
        //__CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        //ucIndex++;

        //palDataMgr_Data_Access(edcCOMMUNICATIONS_INFO_CONNECTION, edaREAD, ucString);
        //sprintf(ucOutputString, "\r(ICI!%03d  \"%s\" \"Connection\")\r\n",ucIndex, ucString);
        //__CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        //ucIndex++;

        palDataMgr_Data_Access(edcLAN_STATUS, edaREAD, &ucVal);
        if(ucVal == 1)
        {
            sprintf(ucOutputString, "\r(ICI!%03d  \"%s\" \"Network Status\")\r\n",ucIndex, "Connected");
        }
        else
        {
            sprintf(ucOutputString, "\r(ICI!%03d  \"%s\" \"Network Status\")\r\n",ucIndex, "Disconnected");
        }
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        ucIndex++;

        palDataMgr_Data_Access(edcLAN_DHCP, edaREAD, &ucVal);      //T100_Simon_0037      //T100_Simon_0008 Start
        if(ucVal == 1)                                             //T100_Simon_0037
        {
            sprintf(ucOutputString, "\r(ICI!%03d  \"%s\" \"DHCP\")\r\n",ucIndex, "On");
        }
        else
        {
            sprintf(ucOutputString, "\r(ICI!%03d  \"%s\" \"DHCP\")\r\n",ucIndex, "Off"); //T100_Simon_0008 End
        }
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        ucIndex++;

        palDataMgr_Data_Access(edcLAN_IP_ADDRESS, edaREAD, ucString);
        sprintf(ucOutputString, "\r(ICI!%03d  \"%s\" \"IP Address\")\r\n",ucIndex, ucString);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        ucIndex++;

        palDataMgr_Data_Access(edcLAN_SUBNET_MASK, edaREAD, ucString);
        sprintf(ucOutputString, "\r(ICI!%03d  \"%s\" \"Subnet Mask\")\r\n",ucIndex, ucString);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        ucIndex++;

        palDataMgr_Data_Access(edcLAN_DEFAULT_GATEWAY, edaREAD, ucString);
        sprintf(ucOutputString, "\r(ICI!%03d  \"%s\" \"Gateway\")\r\n",ucIndex, ucString);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        ucIndex++;

        palDataMgr_Data_Access(edcLAN_PRIMARY_DNS, edaREAD, ucString);
        sprintf(ucOutputString, "\r(ICI!%03d  \"%s\" \"Primary DNS\")\r\n",ucIndex, ucString);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        ucIndex++;

        palDataMgr_Data_Access(edcLAN_SECOND_DNS, edaREAD, ucString);
        sprintf(ucOutputString, "\r(ICI!%03d  \"%s\" \"Secondary DNS\")\r\n",ucIndex, ucString);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        ucIndex++;

        palDataMgr_Data_Access(edcLAN_MAC_ADDRESS, edaREAD, ucString);
        sprintf(ucOutputString, "\r(ICI!%03d  \"%s\" \"MAC Address\")\r\n",ucIndex, ucString);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        ucIndex++;

        //ucString[0] = '\0';
        //sprintf(ucOutputString, "\r(ICI!%03d  \"%s\" \"WLAN\")\r\n",ucIndex, ucString);
        //__CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        //ucIndex++;

        //palDataMgr_Data_Access(edcWLAN_SSID, edaREAD, ucString);
        //sprintf(ucOutputString, "\r(ICI!%03d  \"%s\" \"SSID\")\r\n",ucIndex, ucString);
        //__CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        //ucIndex++;

        //palDataMgr_Data_Access(edcWLAN_DEFAULT_GATEWAY, edaREAD, ucString);
        //sprintf(ucOutputString, "\r(ICI!%03d  \"%s\" \"Default Gateway\")\r\n",ucIndex, ucString);
        //__CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        //ucIndex++;
    }
    else if(eAccessMode == ecmWrite)
    {
        return eCLI_ERROR_CODE_COMMAND_READ_ONLY;
    }

    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilChristie_LSE(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)
{
    if(eAccessMode == ecmRead)
    {
        UINT16 uiLastErrorCode = 0 ;
        palDataMgr_Data_Access(edcLAST_ERRORCODE, edaREAD, &uiLastErrorCode);

        sCmdFormat->lData = uiLastErrorCode;
    }
    else if(eAccessMode == ecmWrite)
    {
        return eCLI_ERROR_CODE_COMMAND_READ_ONLY;
    }

    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilChristie_OSD(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)       //A70LV_Doulas_0163
{
    if(eAccessMode == ecmRead)
    {
        UINT8 ucOSD_Show = 0 ;
        palDataMgr_Data_Access(edcOSD_SHOW, edaREAD, &ucOSD_Show);

        sCmdFormat->lData = ucOSD_Show;
    }
    else if(eAccessMode == ecmWrite)
    {
        UINT8 ucMenuShow;
		if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)	//A70Gen2_Doulas_0050
		{
			return eCLI_ERROR_CODE_DATAERROR;
		}

        if((sCmdFormat->lData <= ets_ON) && (sCmdFormat->lData >= ets_OFF))
        {
            ucMenuShow = (UINT8)sCmdFormat->lData;
            palDataMgr_UI_EventSend(edcOSD_SHOW, ucMenuShow, NULL); //appGui_OSD_ON_Set(ucMenuShow);
            palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL); //appGui_SendUpdateOSDEvent();
        }
        else
        {
            return eCLI_ERROR_CODE_DATAERROR;
        }

    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilChristie_CLI_SYS_C821(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)   //A70LV_Doulas_0165
{
    if(eAccessMode == ecmRead)
    {
        palImgMgr_Dump_Register(NULL);  //all bank
    }
    else if(eAccessMode == ecmWrite)
    {
        return eCLI_ERROR_CODE_COMMAND_READ_ONLY;
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilChristie_CLI_SYS_C789(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)      //A70LV_Doulas_0170 debug
{
    if(eAccessMode == ecmRead)
    {
		if(palSystem_Suspend(__FUNCTION__) == rcSUCCESS)
    	{
    	    #ifdef SCALER_C821_C789
        	halC789Ctrl_RegDump();
        	#endif

        	palSystem_Resume(__FUNCTION__);
    	}
    }
    else if(eAccessMode == ecmWrite)
    {
        return eCLI_ERROR_CODE_COMMAND_READ_ONLY;
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilChristie_CLI_SYS_FPGA(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)
{
#ifdef SCALER_FPGA_F34
    if(eAccessMode == ecmRead)
    {
        char  ucOutputString[128] = {""};
        sprintf(ucOutputString, "sys+fpgaX, X = bank, X = 99 is printf all");
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
    }
    else if(eAccessMode == ecmWrite)
    {
        halScaler_Dump_Register((UINT8)sCmdFormat->lData);
    }
#endif /* SCALER_FPGA_F34 */
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilChristie_CLI_SYS_RSXF(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)      //A70LV_Doulas_0184 debug
{
    if(eAccessMode == ecmRead)
    {
        sCmdFormat->lData = 4;
        return eCLI_ERROR_CODE_COMMAND_WRITE_ONLY;
    }
    else if(eAccessMode == ecmWrite)
    {
        dvFPGA_XillinxFPGA_Reset();
        /*
        if(halBoard_XillinxFPGA_NewVersionGet() == TRUE)        //A70LV_Doulas_0218
        {
            UINT8 ePanelId = 0;
            ePanelId = (UINT8)palSystem_PanelID_Get();
            halBoard_XillinxFPGA_Manual_Mode_Set(ePanelId);    //A70LV_Doulas_0216
        }*/
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilChristie_CLI_SYS_RVXF(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)      //A70LV_Doulas_0218 read X-FPGA version
{
    if(eAccessMode == ecmRead)      //A70LV_Doulas_0243 modify
    {
        UINT8 a = 0xff;
        UINT8 b = 0xff;
        eRESULT eResult = rcERROR;
        eRESULT eResult2 = rcERROR;
        UINT8 ucData;
        sCmdFormat->lData = 4;
        eResult  = halBoard_XillinxFPGA_Version_BL_Get(&a);  //A35G2_Simon_0065
        eResult &= halBoard_XillinxFPGA_Version_BH_Get(&b);  //A35G2_Simon_0065
        eResult2 = halBoard_XillinxFPGA_DRP_Status_Get(&ucData);  //A35G2_Simon_0065
        LOG_MSG(db_ALWAYS, "X-FPAG version %d.%d(R%d),Lock Status = %d(R%d) \r\n",b,a,eResult,ucData,eResult2);
    }
    else if(eAccessMode == ecmWrite)
    {
        return eCLI_ERROR_CODE_COMMAND_READ_ONLY;
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilChristie_LET(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat) //A70Gen2_Julie_0021
{
    char  ucOutputString[128] = {""};
    UINT8  ucVal;

    if(eAccessMode == ecmRead)
    {
        palDataMgr_Data_Access(edcLENS_TYPE, edaREAD, &ucVal);

        sprintf(ucOutputString, "%s\r\n",palDataMgr_LensType_String(ucVal));
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
    }
    else if(eAccessMode == ecmWrite)
    {
        return eCLI_ERROR_CODE_COMMAND_READ_ONLY;
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilChristie_FVI(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat) //A70LV_Larry_0232
{
    UINT8 ucString[64] = {0};
    char  ucOutputString[128] = {""};
    UINT8 ucIndex = 0;

    if(eAccessMode == ecmRead)
    {
        UINT16 cIndex = 0, cTotal = 0;
        UINT32 ulDataCode;
        char cVerString[64] = {""};

        cTotal = CmdCfg_VersionInfo_Get(0xFF, &ulDataCode, cVerString);
#if 1
        for(cIndex = 0; cIndex < cTotal; cIndex++)
        {
            CmdCfg_VersionInfo_Get(cIndex, &ulDataCode, cVerString);

            palDataMgr_Data_Access(ulDataCode, edaREAD, ucString);
            sprintf(ucOutputString, "\r(FVI!%03d  \"%s\" \"%s\")\r\n",ucIndex, ucString, cVerString);
            __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
            ucIndex++;
        }

#else
        palDataMgr_Data_Access(edcRELEASE_VERSION, edaREAD, ucString);
        sprintf(ucOutputString, "\r(FVI!%03d  \"%s\" \"FW Version\")\r\n",ucIndex, ucString);	//A70Gen2_Doulas_0013 Modify
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        ucIndex++;

#if 0	//A70Gen2_Doulas_0013 remove
        palDataMgr_Data_Access(edcFRONTEND_VERSION, edaREAD, ucString);
        sprintf(ucOutputString, "\r(FVI!%03d  \"  %s\" \"F-MCU Version\")\r\n",ucIndex, ucString);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        ucIndex++;
#endif
        sprintf(ucOutputString, "\r(FVI!%03d  \"  V%02d.%02d\" \"I-SCALER Version\")\r\n",ucIndex, VER_MAJOR, VER_MINOR);	//A70Gen2_Doulas_0013 Modify
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        ucIndex++;

#if 1	//A70Gen2_Doulas_0013 Modify
        palDataMgr_Data_Access(edcMOTOR_VERSION, edaREAD, ucString);
        sprintf(ucOutputString, "\r(FVI!%03d  \"  %s\" \"M-MCU Version\")\r\n",ucIndex, ucString);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        ucIndex++;

        palDataMgr_Data_Access(edcLD_DRIVER_VERSION, edaREAD, ucString);
        sprintf(ucOutputString, "\r(FVI!%03d  \"  %s\" \"L-MCU Version\")\r\n",ucIndex, ucString);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        ucIndex++;
#endif

        palDataMgr_Data_Access(edcKEYPAD_VERSION, edaREAD, ucString);
        sprintf(ucOutputString, "\r(FVI!%03d  \"  %s\" \"K-MCU Version\")\r\n",ucIndex, ucString);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        ucIndex++;

        palDataMgr_Data_Access(edcLAN_VERSION, edaREAD, ucString);
        sprintf(ucOutputString, "\r(FVI!%03d  \"  %s\" \"LAN  Version\")\r\n",ucIndex, ucString);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        ucIndex++;

        palDataMgr_Data_Access(edcFORMATER_VERSION, edaREAD, ucString);
        sprintf(ucOutputString, "\r(FVI!%03d  \"  %s\" \"Formatter Version\")\r\n",ucIndex, ucString);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        ucIndex++;

        palDataMgr_Data_Access(edcHDBASET_VERSION, edaREAD, ucString);
        sprintf(ucOutputString, "\r(FVI!%03d  \"  %s\" \"HDBaseT Version\")\r\n",ucIndex, ucString);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        ucIndex++;

        palDataMgr_Data_Access(edcFMCU_VERSION, edaREAD, ucString);
        sprintf(ucOutputString, "\r(FVI!%03d  \"  %s\" \"A-MCU Version\")\r\n",ucIndex, ucString);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        ucIndex++;

        palDataMgr_Data_Access(edcCAMERA_FW_VERSION, edaREAD, ucString);
        sprintf(ucOutputString, "\r(FVI!%03d  \"  %s\" \"Camera Version\")\r\n",ucIndex, ucString);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        ucIndex++;

        if(sCmdFormat->cSubCode[0] != '\0') //A35Gen2_Coda
        {
            palDataMgr_Data_Access(edcFPGA1_VERSION, edaREAD, ucString);
            sprintf(ucOutputString, "\r(FVI!%03d  \"  %s\" \"FPGA1 Version\")\r\n",ucIndex, ucString);
            __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
            ucIndex++;

            palDataMgr_Data_Access(edcFPGA2_VERSION, edaREAD, ucString);
            sprintf(ucOutputString, "\r(FVI!%03d  \"  %s\" \"FPGA2 Version\")\r\n",ucIndex, ucString);
            __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
            ucIndex++;

            palDataMgr_Data_Access(edcXFPGA_VERSION, edaREAD, ucString);
            sprintf(ucOutputString, "\r(FVI!%03d  \"  %s\" \"XFPGA Version\")\r\n",ucIndex, ucString);
            __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
            ucIndex++;

            palDataMgr_Data_Access(edcHW_PROJECTOR_ID, edaREAD, ucString);
            sprintf(ucOutputString, "\r(FVI!%03d  \"  %s\" \"HW Version\")\r\n",ucIndex, ucString);
            __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
            ucIndex++;

        }
#endif
    }
    else if(eAccessMode == ecmWrite)
    {
        return eCLI_ERROR_CODE_COMMAND_READ_ONLY;
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilChristie_MDT(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)   //A70LV_Doulas_0209
{
    char ucString[64] = {0};
    char ucString2[20] = {0};
    char  ucOutputString[128] = {""};
    UINT16 uiVal;
    UINT8  ucVal;
    sMODE_ADJUSTMENT_TABLE sModeAdjustmentTable;
    //eEXEC_CODE eResult = eEXEC_CODE_PASS;

    if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(edcMODE_ADJ_TABLE_NUMBER))
    {
        return eCLI_ERROR_CODE_MUSTDEPENDONSRC;
    }

    if(eAccessMode == ecmRead)
    {
        palDataMgr_Data_Access(edcMODE_ADJ_TABLE_NUMBER, edaWRITE_THROUGH_WITH_ACTION, &ucString);

        palDataMgr_Data_Access(edcMODE_ADJ_TABLE_NUMBER, edaREAD, &ucString);
        sprintf(ucOutputString, "\r(MDT!000  \"%s\" \"Model Table\")\r\n",ucString);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);

        palDataMgr_Data_Access(edcMODE_ADJ_BEST_ID, edaREAD, &ucString);
        sprintf(ucOutputString, "\r(MDT!001  \"%s\" \"Best Mode ID\")\r\n",ucString);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);

        palDataMgr_Data_Access(edcMODE_ADJ_H_PERIOD, edaREAD, &ucString);
        sprintf(ucOutputString, "\r(MDT!002  \"%s\" \"H-Period\")\r\n",ucString);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);

        palDataMgr_Data_Access(edcMODE_ADJ_H_TOTAL, edaREAD, &ucString);
        sprintf(ucOutputString, "\r(MDT!003  \"%s\" \"H-Total\")\r\n",ucString);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);

        palDataMgr_Data_Access(edcMODE_ADJ_V_TOTAL, edaREAD, &ucString);
        sprintf(ucOutputString, "\r(MDT!004  \"%s\" \"V-Total\")\r\n",ucString);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);

        palDataMgr_Data_Access(edcMODE_ADJ_H_ACTIVE, edaREAD, &ucString);
        sprintf(ucOutputString, "\r(MDT!005  \"%s\" \"H-Active\")\r\n",ucString);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);

        palDataMgr_Data_Access(edcMODE_ADJ_V_ACTIVE, edaREAD, &ucString);
        sprintf(ucOutputString, "\r(MDT!006  \"%s\" \"V-Active\")\r\n",ucString);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);

        palDataMgr_Data_Access(edcMODE_ADJ_H_START, edaREAD, &uiVal);
        sprintf(ucOutputString, "\r(MDT!007  \"%d\" \"Horz Pos\")\r\n",uiVal);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);

        palDataMgr_Data_Access(edcMODE_ADJ_V_START, edaREAD, &uiVal);
        sprintf(ucOutputString, "\r(MDT!008  \"%d\" \"Vert Pos\")\r\n",uiVal);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);

        palDataMgr_Data_Access(edcMODE_ADJ_INFO,   edaREAD, (void*)&sModeAdjustmentTable);
        if(sModeAdjustmentTable.ucNewMA_Mode_Flag == 0xA5)
        {
            sprintf(ucString2, " %d - %d ", sModeAdjustmentTable.ucNewMA_Table_ID
                                            , sModeAdjustmentTable.uiNewMA_Mode_ID);

            sprintf(ucString, "%d0 - %d - %d - %d - %d - %d - %d ",   sModeAdjustmentTable.uiNewMA_HPeriod
                                                                  , sModeAdjustmentTable.uiNewMA_HTotal
                                                                  , sModeAdjustmentTable.uiNewMA_VTotal
                                                                  , sModeAdjustmentTable.uiNewMA_HActive
                                                                  , sModeAdjustmentTable.uiNewMA_VActive
                                                                  , sModeAdjustmentTable.uiNewMA_HStart
                                                                  , sModeAdjustmentTable.uiNewMA_VStart);

            palDataMgr_Data_Access(edcMODE_ADJ_RECORD_INDEX, edaREAD, &ucVal);
            sprintf(ucOutputString, "\r(MDT!009  \"%d\" \"%s\" \"%s\" \"Record index\")\r\n",ucVal,ucString2,ucString);
            __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        }
        else
        {
            palDataMgr_Data_Access(edcMODE_ADJ_RECORD_INDEX, edaREAD, &ucVal);
            sprintf(ucOutputString, "\r(MDT!009  \"%d\" \"Empty\" \"Record index\")\r\n",ucVal);
            __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        }

    }
    else if(eAccessMode == ecmWrite)
    {
        return eCLI_ERROR_CODE_COMMAND_READ_ONLY;
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilChristie_MDT_HPOS(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)   //A70LV_Doulas_0209
{
    UINT16 uiVal;
    char ucString[10] = {0};
    eEXEC_CODE eResult = eEXEC_CODE_PASS;

    if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(edcMODE_ADJ_H_START))
    {
        return eCLI_ERROR_CODE_MUSTDEPENDONSRC;
    }

    palDataMgr_Data_Access(edcMODE_ADJ_TABLE_NUMBER, edaWRITE_THROUGH_WITH_ACTION, &ucString);
    if(eAccessMode == ecmRead)
    {
        palDataMgr_Data_Access(edcMODE_ADJ_H_START, edaREAD, &uiVal);
        sCmdFormat->lData = (INT32) uiVal;
    }
    else if(eAccessMode == ecmWrite)
    {
        if((sCmdFormat->lData <= MODE_ADJ_H_START_MAX_VALUE) && (sCmdFormat->lData >= MODE_ADJ_H_START_MIN_VALUE))
        {
            uiVal = (UINT16)sCmdFormat->lData;

            eResult = palDataMgr_Data_Access(edcMODE_ADJ_H_START, edaWRITE_THROUGH_WITH_ACTION, &uiVal) ;
            if(eResult != eEXEC_CODE_PASS)
            {
                return eCLI_ERROR_CODE_SETFAIL;
            }
        }
        else
        {
            return eCLI_ERROR_CODE_DATAERROR;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilChristie_MDT_VPOS(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)   //A70LV_Doulas_0209
{
    UINT16 uiVal;
    char ucString[10] = {0};
    eEXEC_CODE eResult = eEXEC_CODE_PASS;

    if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(edcMODE_ADJ_V_START))
    {
        return eCLI_ERROR_CODE_MUSTDEPENDONSRC;
    }

    palDataMgr_Data_Access(edcMODE_ADJ_TABLE_NUMBER, edaWRITE_THROUGH_WITH_ACTION, &ucString);
    if(eAccessMode == ecmRead)
    {
        palDataMgr_Data_Access(edcMODE_ADJ_V_START, edaREAD, &uiVal);
        sCmdFormat->lData = (INT32) uiVal;
    }
    else if(eAccessMode == ecmWrite)
    {
        if((sCmdFormat->lData <= MODE_ADJ_V_START_MAX_VALUE) && (sCmdFormat->lData >= MODE_ADJ_V_START_MIN_VALUE))
        {
            uiVal = (UINT16)sCmdFormat->lData;

            eResult = palDataMgr_Data_Access(edcMODE_ADJ_V_START, edaWRITE_THROUGH_WITH_ACTION, &uiVal) ;
            if(eResult != eEXEC_CODE_PASS)
            {
                return eCLI_ERROR_CODE_SETFAIL;
            }
        }
        else
        {
            return eCLI_ERROR_CODE_DATAERROR;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilChristie_MDT_SAVE(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)   //A70LV_Doulas_0264 modify//A70LV_Doulas_0209
{
    UINT8 ucVal;
    UINT8 ucVal2;
    char ucString[10] = {0};
    eEXEC_CODE eResult = eEXEC_CODE_PASS;

    if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(edcMODE_ADJ_SAVE))
    {
        return eCLI_ERROR_CODE_MUSTDEPENDONSRC;
    }

    if(eAccessMode == ecmRead)
    {
        sCmdFormat->lData = 4;
        return eCLI_ERROR_CODE_COMMAND_WRITE_ONLY;
    }
    else if(eAccessMode == ecmWrite)
    {
        palDataMgr_Data_Access(edcMODE_ADJ_TABLE_NUMBER, edaWRITE_THROUGH_WITH_ACTION, &ucString);
        //if((sCmdFormat->lData <= ets_ON) && (sCmdFormat->lData >= ets_OFF))
        if((sCmdFormat->lData <= MODE_ADJ_RECORD_INDEX_MAX_VALUE) && (sCmdFormat->lData >= MODE_ADJ_RECORD_INDEX_MIN_VALUE))
        {
            ucVal = (UINT8)sCmdFormat->lData;

            palDataMgr_Data_Access(edcMODE_ADJ_RECORD_INDEX, edaREAD, &ucVal2);
            palDataMgr_Data_Access(edcMODE_ADJ_RECORD_INDEX, edaWRITE_THROUGH_WITH_ACTION, &ucVal);

            eResult = palDataMgr_Data_Access(edcMODE_ADJ_SAVE, edaWRITE_THROUGH_WITH_ACTION, &ucVal) ;

            palDataMgr_Data_Access(edcMODE_ADJ_RECORD_INDEX, edaWRITE_THROUGH_WITH_ACTION, &ucVal2);
            if(eResult != eEXEC_CODE_PASS)
            {
                return eCLI_ERROR_CODE_SETFAIL;
            }
        }
        else
        {
            return eCLI_ERROR_CODE_DATAERROR;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilChristie_MDT_CLER(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)   //A70LV_Doulas_0264 modify//A70LV_Doulas_0209
{
    UINT8 ucVal;
    UINT8 ucVal2;
    eEXEC_CODE eResult = eEXEC_CODE_PASS;

    if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(edcMODE_ADJ_CLEAR))
    {
        return eCLI_ERROR_CODE_MUSTDEPENDONSRC;
    }

    if(eAccessMode == ecmRead)
    {
        sCmdFormat->lData = 4;
        return eCLI_ERROR_CODE_COMMAND_WRITE_ONLY;
    }
    else if(eAccessMode == ecmWrite)
    {
        //if((sCmdFormat->lData <= ets_ON) && (sCmdFormat->lData >= ets_OFF))
        if((sCmdFormat->lData <= MODE_ADJ_RECORD_INDEX_MAX_VALUE) && (sCmdFormat->lData >= MODE_ADJ_RECORD_INDEX_MIN_VALUE))
        {
            ucVal = (UINT8)sCmdFormat->lData;

            palDataMgr_Data_Access(edcMODE_ADJ_RECORD_INDEX, edaREAD, &ucVal2);
            palDataMgr_Data_Access(edcMODE_ADJ_RECORD_INDEX, edaWRITE_THROUGH_WITH_ACTION, &ucVal);

            eResult = palDataMgr_Data_Access(edcMODE_ADJ_CLEAR, edaWRITE_THROUGH_WITH_ACTION, &ucVal) ;

            palDataMgr_Data_Access(edcMODE_ADJ_RECORD_INDEX, edaWRITE_THROUGH_WITH_ACTION, &ucVal2);
            if(eResult != eEXEC_CODE_PASS)
            {
                return eCLI_ERROR_CODE_SETFAIL;
            }
        }
        else
        {
            return eCLI_ERROR_CODE_DATAERROR;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilChristie_MDT_RDIX(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)   //A70LV_Doulas_0209
{
    UINT8 ucVal;
    eEXEC_CODE eResult = eEXEC_CODE_PASS;

    if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(edcMODE_ADJ_RECORD_INDEX))
    {
        return eCLI_ERROR_CODE_MUSTDEPENDONSRC;
    }

    if(eAccessMode == ecmRead)
    {
        palDataMgr_Data_Access(edcMODE_ADJ_RECORD_INDEX, edaREAD, &ucVal);
        sCmdFormat->lData = (INT32) ucVal;
    }
    else if(eAccessMode == ecmWrite)
    {
        if((sCmdFormat->lData <= MODE_ADJ_RECORD_INDEX_MAX_VALUE) && (sCmdFormat->lData >= MODE_ADJ_RECORD_INDEX_MIN_VALUE))
        {
            ucVal = (UINT8)sCmdFormat->lData;

            eResult = palDataMgr_Data_Access(edcMODE_ADJ_RECORD_INDEX, edaWRITE_THROUGH_WITH_ACTION, &ucVal) ;
            if(eResult != eEXEC_CODE_PASS)
            {
                return eCLI_ERROR_CODE_SETFAIL;
            }
        }
        else
        {
            return eCLI_ERROR_CODE_DATAERROR;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilChristie_PIF_SXRJ(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)   //A70LV_Larry_0235
{
    char  ucOutputString[32] = {""};

    if(palSystem_CustomerSplashGet() == CUSTOM_LOGO_SCREENX)
    {
        if(eAccessMode == ecmRead)
        {
            sprintf(ucOutputString, "SX-C13K-10\r\n");
            __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
            return eCLI_ERROR_CODE_NO;
        }
        else if(eAccessMode == ecmWrite)
        {
            return eCLI_ERROR_CODE_COMMAND_READ_ONLY;
        }
    }
    else
    {
        sprintf(ucOutputString, "Control Not Found\0\r\n");
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        return eCLI_ERROR_CODE_NO;
    }

    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilChristie_LIF_TPHS(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat) //A70LV_Larry_0268
{
	if(eAccessMode == ecmRead)
	{
		UINT32 ulVlaue = 0;

		palDataMgr_Data_Access(edcTOTAL_PROJECTOR_HOURS, edaREAD, &ulVlaue) ;

		sCmdFormat->lData= (INT32)(ulVlaue/60);
		return eCLI_ERROR_CODE_NO;
	}
	else if(eAccessMode == ecmWrite)
	{
		UINT32 ulVlaue = (UINT32)sCmdFormat->lData;
		INT32  iMax = 0;
		INT32  iMin = 0;

		ulVlaue = ulVlaue * 60;

		if((eEXEC_CODE_PASS == palDataMgr_Data_Range_Get(edcTOTAL_PROJECTOR_HOURS, edrMAX, &iMax)) &&
		   (eEXEC_CODE_PASS == palDataMgr_Data_Range_Get(edcTOTAL_PROJECTOR_HOURS, edrMIN, &iMin)))
		{

			if((iMax >= (INT32)ulVlaue) && ((INT32)ulVlaue >= iMin))
			{
                palIllumination_Init_TotalProjectHoursSet(ulVlaue);
                palDataMgr_Data_Access(edcTOTAL_PROJECTOR_HOURS, edaWRITE_THROUGH_NO_ACTION, &ulVlaue);

				return eCLI_ERROR_CODE_NO;
			}
		}
		return eCLI_ERROR_CODE_DECIMALERR;
	}
	return eCLI_ERROR_CODE_NO;
}


eCLI_ERROR_CODE utilChristie_LIF_LSHS(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat) //A70LV_Larry_0268
{
	if(eAccessMode == ecmRead)
	{
		UINT32 ulVlaue = 0;

		palDataMgr_Data_Access(edcLD_HOURS, edaREAD, &ulVlaue) ;

        sCmdFormat->lData= (INT32)(ulVlaue/60);

		return eCLI_ERROR_CODE_NO;
	}
	else if(eAccessMode == ecmWrite)
	{
		UINT32 ulVlaue = (UINT32)sCmdFormat->lData;
		INT32  iMax = 0;
		INT32  iMin = 0;

		ulVlaue = ulVlaue * 60;

		if((eEXEC_CODE_PASS == palDataMgr_Data_Range_Get(edcLD_HOURS, edrMAX, &iMax)) &&
		   (eEXEC_CODE_PASS == palDataMgr_Data_Range_Get(edcLD_HOURS, edrMIN, &iMin)))
		{
			if((iMax >= (INT32)ulVlaue) && ((INT32)ulVlaue >= iMin))
			{
                palIllumination_Init_LDHoursSet(ulVlaue);
                palDataMgr_Data_Access(edcLD_HOURS, edaWRITE_THROUGH_NO_ACTION, &ulVlaue);

				return eCLI_ERROR_CODE_NO;
			}
		}
		return eCLI_ERROR_CODE_DECIMALERR;
	}
	return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilChristie_CLI_SYS_DDPR(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)      //A70LV_Doulas_0236
{
    if(eAccessMode == ecmRead)
    {
        sCmdFormat->lData = 4;
        return eCLI_ERROR_CODE_COMMAND_WRITE_ONLY;
    }
    else if(eAccessMode == ecmWrite)
    {
        halFormatter_ChannelSourceSet(DISP_EXTERNAL_SOURCE);        //DISP_EXTERNAL_SOURCE
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilChristie_CLI_SYS_DAPH(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)       //A70LV_Doulas_0236
{
    char  ucOutputString[128] = {""};

    if(eAccessMode == ecmRead)
    {
        sprintf(ucOutputString, "DataPath state (%d,%d) \r\n",palDataPath_GetDataPathState(),palDataPath_GetDataPathSubState());
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilChristie_CLI_SYS_LVDS(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)      //A70LV_Doulas_0262
{
    if(eAccessMode == ecmRead)
    {
        UINT8 ucData = 3;
        eRESULT eResult = rcERROR;
        eResult = halBoard_XillinxFPGA_LDVS_Cover_Get(&ucData);  //A35G2_Simon_0065

        LOG_MSG(db_ALWAYS, "Xillinx_FPGA_LVDS_COVER get %d(R%d)\r\n",ucData,eResult);
    }
    else
    {
        UINT8 ucData = 2;
        if(sCmdFormat->lData == 1)   //LVDS cover
        {
            ucData = ets_ON;
            palDataMgr_XFPGA_LVDS_COVER_Set(ucData);
            LOG_MSG(db_ALWAYS, "Xillinx_FPGA_LVDS_COVER Enable \r\n");
        }
        else       //no LVDS cover
        {
            ucData = ets_OFF;
            palDataMgr_XFPGA_LVDS_COVER_Set(ucData);
            LOG_MSG(db_ALWAYS, "Xillinx_FPGA_LVDS_COVER Disable \r\n");
        }

    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilChristie_CLI_SYS_LDON(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)      //A70LV_Doulas_0306 Add
{
    char  ucOutputString[128] = {""};
    UINT8 ucData1 = 2;
    UINT8 ucData2 = 2;
    UINT8 ucData3 = 2;

    if(eAccessMode == ecmRead)
    {
        halLDCtrl_LD_SourceEnable_Get(eLDBANK_A70LV, &ucData1);
        halLDCtrl_LD_SourceBlanking_Get(eLDBANK_A70LV, &ucData2);
        halLDCtrl_LD_WheelBlanking_Get(eLDBANK_A70LV, &ucData3);
        sprintf(ucOutputString, "LD Shutter(%d),Source(%d),Wheel(%d) \r\n",ucData1,ucData2,ucData3);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilChristie_CLI_CCT_REST(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)
{
    if(eAccessMode == ecmRead)
    {
        return eCLI_ERROR_CODE_COMMAND_WRITE_ONLY;
    }
    else
    {
        if(sCmdFormat->lData == 1)
        {
            //appIapProc_CCT_StorageErase();
        }
        else
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

#if 0
eCLI_ERROR_CODE utilChristie_CLI_PST(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)  //G100_Coda_0042
{
    if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(edcPICTURE_SETTINGS))
    {
        return eCLI_ERROR_CODE_MUSTDEPENDONSRC;
    }

    if(eAccessMode == ecmRead)
    {
        INT32 iData = 0;
        utilCommonCLI_DataConversionGet(edcPICTURE_SETTINGS, &iData);
        sCmdFormat->lData = iData;
    }
    else if(eAccessMode == ecmWrite)
    {
        INT32 iData = 0;
        INT32 iGetData = 0;

		if(sCmdFormat->ucDataType == eCLI_DATA_TYPE_N)	//A70Gen2_Doulas_0047 Add
		{
        	//if(m_sSubLut_Christie[sCmdFormat->uiFuncID].uiSpecialFlag & CLI_COM_TYPE_SUPPORT_PN)
			{
                INT8 cResult = eEXEC_CODE_FAIL;
				INT32 iSetting = 0;
				UINT16 uiDataCode = edcPICTURE_SETTINGS;
				INT32 iUserData = 0;

    			cResult = utilCommonCLI_DataConversionGet(uiDataCode, &iSetting);
				switch(iSetting)
				{
					case CLI_PICTURE_SETTINGS_USER:
           				utilCommonCLI_DataConversionGet(edcUSER_COLOR_MODE, &iUserData);

						if(iUserData == CLI_PRE_USER_HDR)
            			{
            				iSetting = CLI_PICTURE_SETTINGS_HDR;
							cResult = utilCommonCLI_DataConversionSet(uiDataCode, iSetting);
							if(cResult == eEXEC_CODE_PASS)
							{
								return eCLI_ERROR_CODE_NO;
							}
            			}
						else if(iUserData == CLI_PRE_USER_3D)
						{
							iSetting = CLI_PICTURE_SETTINGS_3D;
							cResult = utilCommonCLI_DataConversionSet(uiDataCode, iSetting);
							if(cResult == eEXEC_CODE_PASS)
							{
								return eCLI_ERROR_CODE_NO;
							}
						}
						else if(iUserData == CLI_PRE_USER_2D_HIGH_SPEED)
						{
							iSetting = CLI_PICTURE_SETTINGS_2D_HIGH_SPEED;
							cResult = utilCommonCLI_DataConversionSet(uiDataCode, iSetting);
							if(cResult == eEXEC_CODE_PASS)
							{
								return eCLI_ERROR_CODE_NO;
							}
						}
						else
						{
							iSetting = CLI_PICTURE_SETTINGS_BLENDING;
							cResult = utilCommonCLI_DataConversionSet(uiDataCode, iSetting);
							if(cResult == eEXEC_CODE_PASS)
							{
								return eCLI_ERROR_CODE_NO;
							}
						}
						break;

					case CLI_PICTURE_SETTINGS_HDR:
					case CLI_PICTURE_SETTINGS_3D:
					case CLI_PICTURE_SETTINGS_2D_HIGH_SPEED:
						cResult = utilCommonCLI_DataConversionSet(uiDataCode, iSetting);
						if(cResult == eEXEC_CODE_PASS)
						{
							return eCLI_ERROR_CODE_NO;
						}
						break;

					default:
						if(eEXEC_CODE_PASS == utilCommonCLI_Decrease_Data(uiDataCode))
						{
							 return eCLI_ERROR_CODE_NO;
						}
						break;
				}

				return eCLI_ERROR_CODE_SETFAIL;
			}
		}
		else if(sCmdFormat->ucDataType == eCLI_DATA_TYPE_P)		//A70Gen2_Doulas_0047 Add
		{
			//if(m_sSubLut_Christie[sCmdFormat->uiFuncID].uiSpecialFlag & CLI_COM_TYPE_SUPPORT_PN)
			{
                INT8 cResult = eEXEC_CODE_FAIL;
				INT32 iSetting = 0;
				UINT16 uiDataCode = edcPICTURE_SETTINGS;

    			cResult = utilCommonCLI_DataConversionGet(uiDataCode, &iSetting);
				switch(iSetting)
				{
					case CLI_PICTURE_SETTINGS_USER:
						cResult = utilCommonCLI_DataConversionSet(uiDataCode, iSetting);
						if(cResult == eEXEC_CODE_PASS)
						{
							return eCLI_ERROR_CODE_NO;
						}
						break;

					case CLI_PICTURE_SETTINGS_HDR:
					case CLI_PICTURE_SETTINGS_3D:
					case CLI_PICTURE_SETTINGS_2D_HIGH_SPEED:
					case CLI_PICTURE_SETTINGS_BLENDING:
						iSetting = CLI_PICTURE_SETTINGS_USER;
						cResult = utilCommonCLI_DataConversionSet(uiDataCode, iSetting);
						if(cResult == eEXEC_CODE_PASS)
						{
							return eCLI_ERROR_CODE_NO;
						}
						break;

					default:
						if(eEXEC_CODE_PASS == utilCommonCLI_Increase_Data(uiDataCode))
						{
							 return eCLI_ERROR_CODE_NO;
						}
						break;
				}

				return eCLI_ERROR_CODE_SETFAIL;
			}
		}

        iData = sCmdFormat->lData;
        utilCommonCLI_DataConversionGet(edcPICTURE_SETTINGS, &iGetData);

        //User Mode
        if(iGetData == CLI_PICTURE_SETTINGS_USER)
        {
            INT32 iUserData = 0;
            utilCommonCLI_DataConversionGet(edcUSER_COLOR_MODE, &iUserData);
            if(((iUserData >= CLI_PRE_USER_HDR) && (iData != iUserData) && (iData != CLI_PICTURE_SETTINGS_USER))
              || ((iUserData < CLI_PRE_USER_HDR) && (iData >= CLI_PICTURE_SETTINGS_HDR) && (iData < CLI_PICTURE_SETTINGS_USER)))
            {
                return eCLI_ERROR_CODE_MUSTDEPENDONSRC;//eCLI_ERROR_CODE_DATAOVERRANGE;
            }
        }

        //When HDR,3D,2D Hight Speed, can't set other picture mode
        else if(iGetData == CLI_PICTURE_SETTINGS_HDR || iGetData == CLI_PICTURE_SETTINGS_3D || iGetData == CLI_PICTURE_SETTINGS_2D_HIGH_SPEED)
        {
            if((iData != CLI_PICTURE_SETTINGS_USER) && ((iData != iGetData)))
            {
                return eCLI_ERROR_CODE_MUSTDEPENDONSRC;//eCLI_ERROR_CODE_DATAOVERRANGE;
            }

        }
        //When normal picture mode, can't set HDR,3D,2D Hight Speed
        else
        {
            if((iData >= CLI_PICTURE_SETTINGS_HDR) && (iData < CLI_PICTURE_SETTINGS_USER))
            {
                return eCLI_ERROR_CODE_MUSTDEPENDONSRC;//eCLI_ERROR_CODE_DATAOVERRANGE;
            }
        }

        if(eEXEC_CODE_PASS == utilCommonCLI_RangeCheck(edcPICTURE_SETTINGS, iData))
        {
            if(eEXEC_CODE_PASS == utilCommonCLI_DataConversionSet(edcPICTURE_SETTINGS, iData))
            {
                return eCLI_ERROR_CODE_NO;
            }
        }
        else
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }
    }
    return eCLI_ERROR_CODE_NO;

}

eCLI_ERROR_CODE utilChristie_CLI_PSU(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)
{
    if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(edcUSER_COLOR_MODE))
    {
        return eCLI_ERROR_CODE_MUSTDEPENDONSRC;
    }

    if(eAccessMode == ecmRead)
    {
        INT32 iData = 0;
        utilCommonCLI_DataConversionGet(edcUSER_COLOR_MODE, &iData);
        sCmdFormat->lData = iData;
    }
    else if(eAccessMode == ecmWrite)
    {
        INT32 iData = 0;
        INT32 iGetData = 0;

		if(sCmdFormat->ucDataType == eCLI_DATA_TYPE_N)	//A70Gen2_Doulas_0047
		{
        	//if(m_sSubLut_Christie[sCmdFormat->uiFuncID].uiSpecialFlag & CLI_COM_TYPE_SUPPORT_PN)
			{
                INT8 cResult = eEXEC_CODE_FAIL;
				INT32 iSetting = 0;
				UINT16 uiDataCode = edcUSER_COLOR_MODE;

    			cResult = utilCommonCLI_DataConversionGet(uiDataCode, &iSetting);
				switch(iSetting)
				{
					case CLI_PRE_USER_HDR:
					case CLI_PRE_USER_3D:
					case CLI_PRE_USER_2D_HIGH_SPEED:
						cResult = utilCommonCLI_DataConversionSet(uiDataCode, iSetting);
						if(cResult == eEXEC_CODE_PASS)
						{
							return eCLI_ERROR_CODE_NO;
						}
						break;

					default:
						if(eEXEC_CODE_PASS == utilCommonCLI_Decrease_Data(uiDataCode))
						{
							 return eCLI_ERROR_CODE_NO;
						}
						break;
				}

				return eCLI_ERROR_CODE_SETFAIL;
			}
		}
		else if(sCmdFormat->ucDataType == eCLI_DATA_TYPE_P)		//A70Gen2_Doulas_0047
		{
			//if(m_sSubLut_Christie[sCmdFormat->uiFuncID].uiSpecialFlag & CLI_COM_TYPE_SUPPORT_PN)
			{
                INT8 cResult = eEXEC_CODE_FAIL;
				INT32 iSetting = 0;
				UINT16 uiDataCode = edcUSER_COLOR_MODE;

    			cResult = utilCommonCLI_DataConversionGet(uiDataCode, &iSetting);
				switch(iSetting)
				{
					case CLI_PRE_USER_HDR:
					case CLI_PRE_USER_3D:
					case CLI_PRE_USER_2D_HIGH_SPEED:
					case CLI_PRE_USER_BLENDING:
						cResult = utilCommonCLI_DataConversionSet(uiDataCode, iSetting);
						if(cResult == eEXEC_CODE_PASS)
						{
							return eCLI_ERROR_CODE_NO;
						}
						break;

					default:
						if(eEXEC_CODE_PASS == utilCommonCLI_Increase_Data(uiDataCode))
						{
							 return eCLI_ERROR_CODE_NO;
						}
						break;
				}

				return eCLI_ERROR_CODE_SETFAIL;
			}
		}

        iData = sCmdFormat->lData;
        utilCommonCLI_DataConversionGet(edcUSER_COLOR_MODE, &iGetData);

        //When HDR,3D,2D Hight Speed, can't set other picture mode
        if(iGetData == CLI_PRE_USER_HDR || iGetData == CLI_PRE_USER_3D || iGetData == CLI_PRE_USER_2D_HIGH_SPEED)
        {
            if((iData < CLI_PRE_USER_HDR) || (iData != iGetData))
            {
                return eCLI_ERROR_CODE_MUSTDEPENDONSRC;//eCLI_ERROR_CODE_DATAOVERRANGE;
            }

        }

        //When normal picture mode, can't set HDR,3D,2D Hight Speed
        else
        {
            if(iData >= CLI_PRE_USER_HDR)
				if(iData <= CLI_PRE_USER_2D_HIGH_SPEED) //A70Gen2_Julie_0031
				{
	                return eCLI_ERROR_CODE_MUSTDEPENDONSRC;
				}
				else
				{
					return eCLI_ERROR_CODE_DATAOVERRANGE;
				}
        }

        if(eEXEC_CODE_PASS == utilCommonCLI_RangeCheck(edcUSER_COLOR_MODE, iData))
        {
            if(eEXEC_CODE_PASS == utilCommonCLI_DataConversionSet(edcUSER_COLOR_MODE, iData))
            {
                return eCLI_ERROR_CODE_NO;
            }
        }
        else
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }
    }
    return eCLI_ERROR_CODE_NO;

}
#endif

#if defined(PLATFORM_H60_4K)
eCLI_ERROR_CODE utilChristie_CLI_TDE(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)
{
    if(eAccessMode == ecmRead)
    {
        INT32 iData = 0;
        utilCommonCLI_DataConversionGet(edc3D_ENABLE, &iData);
        sCmdFormat->lData = iData;
    }
    else if(eAccessMode == ecmWrite)
    {
        INT32 iData = 0;
        INT32 iSource = 0;

        iData = sCmdFormat->lData;
        utilCommonCLI_DataConversionGet(edcMAIN_INPUT, &iSource);

        if((iSource != CLI_SOURCE_HDMI1) && (iSource != CLI_SOURCE_HDMI2) &&
           ((iData == CLI_3D_FORMAT_DUALPIPE) || (iData == CLI_3D_FORMAT_4K3D_DUALPIPE)))
        {
            return eCLI_ERROR_CODE_MUSTDEPENDONSRC;
        }

        //A70LK_Jacky_0045 start
        INT8 uc3DMode = 0;
        palDataMgr_Data_Access(edc3D_MODE, edaREAD, &uc3DMode);

        if((uc3DMode == eCM_3D_MODE_ON) && ((iData == CLI_3D_FORMAT_4K3D) || (iData == CLI_3D_FORMAT_4K3D_DUALPIPE)))
        {
            return eCLI_ERROR_CODE_MUSTDEPENDONSRC;
        }
        //A70LK_Jacky_0045 end

        if(eEXEC_CODE_PASS != utilCommonCLI_DataConversionSet(edc3D_ENABLE, iData))
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }

    return eCLI_ERROR_CODE_NO;
}
#endif

eCLI_ERROR_CODE utilChristie_GAMM(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)    //A70Gen2_Steven_0002 start
{
    if(eAccessMode == ecmRead) //A70Gen2_Julie_0031
    {
        INT32 iData = 0;
        utilCommonCLI_DataConversionGet(edcGAMMA, &iData);
        sCmdFormat->lData = iData;

		if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(edcGAMMA) && (iData != CLI_GAMMA_HDR))
		{
			  return eCLI_ERROR_CODE_MUSTDEPENDONSRC;
		}
	}
    else if(eAccessMode == ecmWrite)
    {
        INT32 iHDR_Auto = ets_OFF;

        utilCommonCLI_DataConversionGet(edcHDR_AUTOENABLE, &iHDR_Auto);
        if(sCmdFormat->lData > CLI_GAMMA_HDR)
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }
        else if(sCmdFormat->lData == CLI_GAMMA_HDR)//A70Gen2_Julie_0032
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
        //G100_Tim_0005, mod, start
        else if( (iHDR_Auto == ets_OFF) || (palImgMgr_HDR_Get(eSOURCE_WINDOW_MAIN) == eHDR_SETTING_SDR) )
        {
			if(sCmdFormat->ucDataType == eCLI_DATA_TYPE_N)	//A70Gen2_Doulas_0047 Add
			{
	        	//if(m_sSubLut_Christie[sCmdFormat->uiFuncID].uiSpecialFlag & CLI_COM_TYPE_SUPPORT_PN)
				{
	                INT8 cResult = eEXEC_CODE_FAIL;
					INT32 iSetting = 0;
					UINT16 uiDataCode = edcGAMMA;

	    			cResult = utilCommonCLI_DataConversionGet(uiDataCode, &iSetting);
					switch(iSetting)
					{
						case CLI_GAMMA_VIDEO:
							cResult = utilCommonCLI_DataConversionSet(uiDataCode, iSetting);
							if(cResult == eEXEC_CODE_PASS)
							{
								return eCLI_ERROR_CODE_NO;
							}
							break;

						case CLI_GAMMA_HDR:
							break;

						default:
							if(eEXEC_CODE_PASS == utilCommonCLI_Decrease_Data(uiDataCode))
							{
								 return eCLI_ERROR_CODE_NO;
							}
							break;
					}

					return eCLI_ERROR_CODE_SETFAIL;
				}
			}
			else if(sCmdFormat->ucDataType == eCLI_DATA_TYPE_P)		//A70Gen2_Doulas_0047 Add
			{
				//if(m_sSubLut_Christie[sCmdFormat->uiFuncID].uiSpecialFlag & CLI_COM_TYPE_SUPPORT_PN)
				{
	                INT8 cResult = eEXEC_CODE_FAIL;
					INT32 iSetting = 0;
					UINT16 uiDataCode = edcGAMMA;

	    			cResult = utilCommonCLI_DataConversionGet(uiDataCode, &iSetting);
					switch(iSetting)
					{
						case CLI_GAMMA_DICOM:
							cResult = utilCommonCLI_DataConversionSet(uiDataCode, iSetting);
							if(cResult == eEXEC_CODE_PASS)
							{
								return eCLI_ERROR_CODE_NO;
							}
							break;

						case CLI_GAMMA_HDR:
							break;

						default:
							if(eEXEC_CODE_PASS == utilCommonCLI_Increase_Data(uiDataCode))
							{
								 return eCLI_ERROR_CODE_NO;
							}
							break;
					}

					return eCLI_ERROR_CODE_SETFAIL;
				}
			}

            if( eEXEC_CODE_PASS == utilCommonCLI_DataConversionSet(edcGAMMA, sCmdFormat->lData) )
            {
                return eCLI_ERROR_CODE_NO;
            }
            else
            {
                return eCLI_ERROR_CODE_SETFAIL;
            }
        }
        else
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
        //G100_Tim_0005, mod, end
    }

    return eCLI_ERROR_CODE_NO;
} //A70Gen2_Steven_0002 end
#if 0
eCLI_ERROR_CODE utilChristie_SZP(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)
{
	if(eAccessMode == ecmRead)
	{
		return eCLI_ERROR_CODE_NO;
	}
	else if(eAccessMode == ecmWrite)
	{
	    if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(edcSIZE_PRESETS))    //A70LV_Doulas_0378 Add
        {
            return eCLI_ERROR_CODE_MUSTDEPENDONSRC;
        }

        switch(sCmdFormat->ucDataType)
        {
            case eCLI_DATA_TYPE_DEC:
                {
                    UINT32 ulVlaue = (UINT32)sCmdFormat->lData;
            		INT32  iMax = 0;
            		INT32  iMin = 0;
                    UINT8  ucSizePresets = 0;
                    //UINT8  cSHIFT_H = 0, cSHIFT_V = 0; //A70LV_Doulas_0342 remove//A70LV_Julie_0017
                    //UINT16 uiZOOM_H = 0, uiZOOM_V = 0; //A70LV_Doulas_0342 remove//A70LV_Julie_0017

                    palDataMgr_Data_Access(edcSIZE_PRESETS, edaREAD, &ucSizePresets);

            		if((eEXEC_CODE_PASS == palDataMgr_Data_Range_Get(edcSIZE_PRESETS, edrMAX, &iMax)) &&
            		   (eEXEC_CODE_PASS == palDataMgr_Data_Range_Get(edcSIZE_PRESETS, edrMIN, &iMin)))
            		{
                        eEXEC_CODE eResult = eEXEC_CODE_FAIL;	//A70LV_Julie_0021

            			if((iMax >= (INT32)ulVlaue) && ((INT32)ulVlaue >= iMin))
            			{
                            if(ucSizePresets == eSCALING_MODE_3D)
                            {
                                if(ulVlaue == eSCALING_MODE_3D) //A70LV_Julie_0028
                                {
                                    return eCLI_ERROR_CODE_NO;
                                }
                                else
                                {
                                    return eCLI_ERROR_CODE_SETFAIL;
                                }
                            }

                            ucSizePresets = (UINT8)ulVlaue;

                            eResult =  palDataMgr_Data_Access(edcSIZE_PRESETS, edaWRITE_THROUGH_WITH_ACTION, &ucSizePresets);	//A70LV_Julie_0021
                            /*      //A70LV_Doulas_0342 remove
                            palDataMgr_Data_Access(edcDIGITAL_HORZ_ZOOM, edaREAD, &uiZOOM_H);
                            palDataMgr_Data_Access(edcDIGITAL_VERT_ZOOM, edaREAD, &uiZOOM_V);
                            palDataMgr_Data_Access(edcDIGITAL_HORZ_SHIFT, edaREAD, &cSHIFT_H);
                            palDataMgr_Data_Access(edcDIGITAL_VERT_SHIFT, edaREAD, &cSHIFT_V);

                            palDataMgr_Data_Access(edcDIGITAL_HORZ_ZOOM, edaWRITE_THROUGH_WITH_ACTION, &uiZOOM_H);
                            palDataMgr_Data_Access(edcDIGITAL_VERT_ZOOM, edaWRITE_THROUGH_WITH_ACTION, &uiZOOM_V);
                            palDataMgr_Data_Access(edcDIGITAL_HORZ_SHIFT, edaWRITE_THROUGH_WITH_ACTION, &cSHIFT_H);
                            palDataMgr_Data_Access(edcDIGITAL_VERT_SHIFT, edaWRITE_THROUGH_WITH_ACTION, &cSHIFT_V);
                            */

                           if(eResult == eEXEC_CODE_PASS)	//A70LV_Julie_0021
                           {
                               return eCLI_ERROR_CODE_NO;
                           }
                           else
                           {
                               return eCLI_ERROR_CODE_SETFAIL;
                           }
            			}
            		}
                }
                return eCLI_ERROR_CODE_DATAOVERRANGE;

            case eCLI_DATA_TYPE_N:
                if(eEXEC_CODE_PASS == utilCommonCLI_Decrease_Data(edcSIZE_PRESETS))
                {
                    return eCLI_ERROR_CODE_NO;
                }
                return eCLI_ERROR_CODE_SETFAIL;

            case eCLI_DATA_TYPE_P:
                if(eEXEC_CODE_PASS == utilCommonCLI_Increase_Data(edcSIZE_PRESETS))
                {
                    return eCLI_ERROR_CODE_NO;
                }
                return eCLI_ERROR_CODE_SETFAIL;

            default:
                return eCLI_ERROR_CODE_DATAERROR;
        }
	}
	return eCLI_ERROR_CODE_NO;
}
#endif /* 0 */

eCLI_ERROR_CODE utilChristie_CLI_SYS_CMOS(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)      //A70LV_Doulas_0361
{
    if(eAccessMode == ecmRead)
    {
        sCmdFormat->lData = 4;
        return eCLI_ERROR_CODE_COMMAND_WRITE_ONLY;
    }
    else if(eAccessMode == ecmWrite)
    {
        //dvC821_COMS_Output_Set((UINT8)sCmdFormat->lData);
        #ifdef SCALER_C821_C789
        halC789Ctrl_COMS_Output_Set((UINT8)sCmdFormat->lData);
        #endif
    }
    return eCLI_ERROR_CODE_NO;
}


eCLI_ERROR_CODE utilChristie_SCW_EVRT(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)  //G100_Coda_0043
{
    if(eAccessMode == ecmWrite)
    {
        UINT32 ucCurrentWeekdayValue = 0;
        UINT8 ucCurrentEventListNum = 0;
        UINT8 ucUserEventListNum = 0;
        UINT32 ucUserChoiceWeekday = 0;
        UINT8 ucIndex = 1;
        sDST_SCHEDULE_EVENT_INFO sCurrentEvent = {0};

        ucUserChoiceWeekday = sCmdFormat->cMainCode[2] - 0x30;

        if(sCmdFormat->lData == 0 || sCmdFormat->lData > 16 || ucUserChoiceWeekday > 6)
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE ;
        }

        //backup system value
        palDataMgr_Data_Access(edcUI_SCHEDULE_EVENT_INFO, edaREAD, &sCurrentEvent);

        ucCurrentEventListNum = sCurrentEvent.ucCurrentEventListNum;

        palDataMgr_Data_Access(edcSCHEDULE_VIEW_WEEKDAY, edaREAD, &ucCurrentWeekdayValue); //appGui_DataCode_Value_Get(edcSCHEDULE_VIEW_WEEKDAY, &ucCurrentWeekdayValue);

        //set event reset
        ucUserEventListNum = sCmdFormat->lData - 1 ;
        sCurrentEvent.ucCurrentEventListNum = ucUserEventListNum;

        LOG_MSG(db_APP_SCHEDULE, "user event reset %d (%s,%d)\r\n", ucUserEventListNum, sCmdFormat->cMainCode, ucUserChoiceWeekday);
        palDataMgr_Data_Access(edcUI_SCHEDULE_EVENT_INFO, edaWRITE_RAM_ONLY_WITH_ACTION, &sCurrentEvent);
        palDataMgr_Data_Access(edcSCHEDULE_VIEW_WEEKDAY, edaWRITE_THROUGH_WITH_ACTION, &ucUserChoiceWeekday); //ppGui_DataCode_Value_Set(edcSCHEDULE_VIEW_WEEKDAY, edaWRITE_THROUGH_WITH_ACTION, ucUserChoiceWeekday);
        palDataMgr_Data_Access(edcSCHEDULE_EVENT_RESET_FUCNTION, edaWRITE_THROUGH_WITH_ACTION, &ucIndex); //appGui_DataCode_Value_Set(edcSCHEDULE_EVENT_RESET_FUCNTION, edaWRITE_THROUGH_WITH_ACTION, 1);

        //restore system value
        sCurrentEvent.ucCurrentEventListNum = ucCurrentEventListNum;
        palDataMgr_Data_Access(edcUI_SCHEDULE_EVENT_INFO, edaWRITE_RAM_ONLY_WITH_ACTION, &sCurrentEvent);
        palDataMgr_Data_Access(edcSCHEDULE_VIEW_WEEKDAY, edaWRITE_THROUGH_WITH_ACTION, &ucCurrentWeekdayValue); //appGui_DataCode_Value_Set(edcSCHEDULE_VIEW_WEEKDAY, edaWRITE_THROUGH_WITH_ACTION, ucCurrentWeekdayValue);
    }
    else if(eAccessMode == ecmRead)
    {
        //INT32 iValue = 0 ;
        //utilCommonCLI_DataConversionGet(edcWARP_MEMORY_APPLY, &iValue);
        //sCmdFormat->lData = iValue + 1 ;
    }

    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilChristie_SCW_REST(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)  //G100_Coda_0042
{
    if(eAccessMode == ecmWrite)
    {
        UINT8 ucResetWeekdayValue = 0 ;
    	UINT32 ucCurrentWeekdayValue = 0;
        UINT8 ucIndex = 1;

        ucResetWeekdayValue = sCmdFormat->cMainCode[2] - 0x30;
        LOG_MSG(db_APP_SCHEDULE, "user wday reset (%s,%d)\r\n", sCmdFormat->cMainCode, ucResetWeekdayValue);

        if(sCmdFormat->lData == 0 || sCmdFormat->lData > 1 || ucResetWeekdayValue > 6)
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE ;
        }

        palDataMgr_Data_Access(edcSCHEDULE_VIEW_WEEKDAY, edaREAD, &ucCurrentWeekdayValue); //appGui_DataCode_Value_Get(edcSCHEDULE_VIEW_WEEKDAY, &ucCurrentWeekdayValue);
        palDataMgr_Data_Access(edcSCHEDULE_VIEW_WEEKDAY, edaWRITE_THROUGH_WITH_ACTION, &ucResetWeekdayValue); //appGui_DataCode_Value_Set(edcSCHEDULE_VIEW_WEEKDAY, ucResetWeekdayValue);
        palDataMgr_Data_Access(edcSCHEDULE_EVENT_RESET_DAY, edaWRITE_THROUGH_WITH_ACTION, &ucIndex); //appGui_DataCode_Value_Set(edcSCHEDULE_EVENT_RESET_DAY, 1);
        palDataMgr_Data_Access(edcSCHEDULE_VIEW_WEEKDAY, edaWRITE_THROUGH_WITH_ACTION, &ucCurrentWeekdayValue); //appGui_DataCode_Value_Set(edcSCHEDULE_VIEW_WEEKDAY, ucCurrentWeekdayValue);

    }
    else if(eAccessMode == ecmRead)
    {
        //INT32 iValue = 0 ;
        //utilCommonCLI_DataConversionGet(edcWARP_MEMORY_APPLY, &iValue);
        //sCmdFormat->lData = iValue + 1 ;
    }

    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilChristie_SCW_CPWD(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)  //G100_Coda_0042
{
    if(eAccessMode == ecmWrite)
    {
        UINT32 ucCurrentWeekday = 0;
        UINT8 ucCopyWeekday = 0;
        UINT32 ucUserChoiceWeekday = 0;

        ucUserChoiceWeekday = sCmdFormat->cMainCode[2] - 0x30;
        ucCopyWeekday = sCmdFormat->lData;

        if(ucCopyWeekday > 6|| ucUserChoiceWeekday > 6 || (ucCopyWeekday == ucUserChoiceWeekday))
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE ;
        }

        //backup system value
        palDataMgr_Data_Access(edcSCHEDULE_VIEW_WEEKDAY, edaREAD, &ucCurrentWeekday); //appGui_DataCode_Value_Get(edcSCHEDULE_VIEW_WEEKDAY, &ucCurrentWeekday);

        //set copy event

        LOG_MSG(db_APP_SCHEDULE, "user Copy wday from %d (%s) to %d\r\n", ucUserChoiceWeekday, sCmdFormat->cMainCode, ucCopyWeekday);

        palDataMgr_Data_Access(edcSCHEDULE_VIEW_WEEKDAY, edaWRITE_THROUGH_WITH_ACTION, &ucUserChoiceWeekday); //appGui_DataCode_Value_Set(edcSCHEDULE_VIEW_WEEKDAY, ucUserChoiceWeekday);

        //appDataMgr_Schedule_User_Current_Copy_Weekday_Num_Set(ucCopyWeekday);
        palDataMgr_Data_Access(edcSCHEDULE_COPY_EVENT_INDEX, edaWRITE_THROUGH_WITH_ACTION, &ucCopyWeekday); //utilCommonCLI_DataConversionSet(edcSCHEDULE_COPY_EVENT_INDEX, ucCopyWeekday);
        //appGui_DataCode_Value_Set(edcSCHEDULE_COPY_EVENT_INDEX, edaWRITE_THROUGH_WITH_ACTION, ucCopyWeekday);

        //restore system value
        palDataMgr_Data_Access(edcSCHEDULE_VIEW_WEEKDAY, edaWRITE_THROUGH_WITH_ACTION, &ucCurrentWeekday); //appGui_DataCode_Value_Set(edcSCHEDULE_VIEW_WEEKDAY, ucCurrentWeekday);
    }
    else if(eAccessMode == ecmRead)
    {

    }

    return eCLI_ERROR_CODE_NO;
}
//G100_Coda_0047
/*
eCLI_ERROR_CODE utilSchedule_Parse_Event_to_Rs232(UINT8 * ucToParseStr, UINT32 * uiEventInfo)
{
    INT32 iValue = 0;
    UINT8 aucDataString[32] = {0};  //下面使用 strtok , 會把 . 取代為 NULL , 所以先複製一份來處理

    memcpy(aucDataString, ucToParseStr, strlen((char *)ucToParseStr));
    LOG_MSG(db_APP_SCHEDULE, "\r\nparse str = %s\r\n", aucDataString);
    UINT8 ucIndex = 0 ;
    char *pch ;

    pch = strtok((char *)aucDataString, ",") ;   // 以 , 這個字元來分割字串

    if(pch == NULL)
    {
        LOG_MSG(db_APP_SCHEDULE, " strtok overflow \r\n");
        return eCLI_ERROR_CODE_DATAOVERRANGE ;
    }

    while(pch != NULL && ucIndex < 3)
    {
        uiEventInfo[ucIndex] = atoi(pch) ;

        LOG_MSG(db_APP_SCHEDULE, "parse %d = %02d\r\n", ucIndex, uiEventInfo[ucIndex]);

        if((uiEventInfo[ucIndex] < 0) || (uiEventInfo[ucIndex] > 2359))  //G100_Owen_0047
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }


        ucIndex++ ;
        pch = strtok(NULL, ",") ;
    }


    return eCLI_ERROR_CODE_NO;
}*/
//G100_Coda_0047
eCLI_ERROR_CODE utilChristie_SCW_EVWR(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)  //G100_Coda_0043
{
    if(eAccessMode == ecmWrite)
    {
        UINT32 ucBackupCurrentWeekdayValue = 0;
        UINT8 ucBackupCurrentEventListNum = 0;

        UINT8 ucUserEventListNum = 0;
        UINT32 ucUserChoiceWeekday = 0;
        UINT8 ucUserFunctionEvent = 0;
        UINT8 ucUserEventStr[12] = {0};
        UINT8 ucUserFunction = 0;
        UINT8 ucUserEvent = 0;
        UINT16 ucUserTimeMinute = 0;
        UINT8 ucUserTimeInfo[5] = {0};
        UINT8 ucValue = 0;

        UINT8 ucIndex = 0 ;
        UINT32 uiEventInfo[4] = {0};
        UINT16 len = strlen((char *)sCmdFormat->cTextString);	//A35G2_CDS_Coda_0014
        sDST_SCHEDULE_EVENT_INFO sCurrentEvent = {0};

        ucUserChoiceWeekday = sCmdFormat->cMainCode[2] - 0x30;

        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_STRING)
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE ;
        }

        if((len < 6) || (len > 10))	//A35G2_CDS_Coda_0014
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }
        memcpy(ucUserEventStr, sCmdFormat->cTextString, len);

        for(ucIndex = 0; ucIndex < len; ucIndex++)	//A35G2_CDS_Coda_0014
        {// R70CDS_Bruce#0016
            if ((__CheckNum(ucUserEventStr[ucIndex]) == 0) && (ucUserEventStr[ucIndex] != ','))
                return eCLI_ERROR_CODE_DATAOVERRANGE;
        }

        if(utilCommonCLI_ScheduleParseEvent_to_Rs232(sCmdFormat->cTextString, uiEventInfo) != eCLI_ERROR_CODE_NO)
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }

        for(ucIndex = 0; ucIndex < 4; ucIndex++)
        {
            LOG_MSG(db_APP_SCHEDULE, "final %d = %d\r\n", ucIndex, uiEventInfo[ucIndex]);
        }

        if((strlen((char *)sCmdFormat->cTextString) < 6) || (strlen((char *)sCmdFormat->cTextString) > 10))
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }
   //--------------
        //--parse Event list num
        ucUserEventListNum = uiEventInfo[0];

        if(ucUserEventListNum > 16 || ucUserEventListNum == 0)
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }

        ucUserEventListNum = ucUserEventListNum - 1;  //user  1 ~ 16 transfer to system 0 ~ 15 event

        //--parse Event time
        sprintf(ucUserTimeInfo, "%04d", uiEventInfo[1]);
        LOG_MSG(db_APP_SCHEDULE, "get time  %s = %d\r\n", ucUserTimeInfo, uiEventInfo[1]);

        if(__CheckNum(ucUserTimeInfo[0]))
        {
            ucValue = ucUserTimeInfo[0] - 0x30;
            if(ucValue > 2)
            {
                return eCLI_ERROR_CODE_DATAOVERRANGE;
            }
            ucUserTimeMinute += ucValue * 10 * 60; //x0:00 transfer minute
        }

        if(__CheckNum(ucUserTimeInfo[1]))
        {
            ucValue = ucUserTimeInfo[1] - 0x30;
            ucUserTimeMinute += ucValue * 60; //0x:00 transfer minute
        }

        if(__CheckNum(ucUserTimeInfo[2]))
        {
            ucValue = ucUserTimeInfo[2] - 0x30;
            if(ucValue > 5)
            {
                return eCLI_ERROR_CODE_DATAOVERRANGE;
            }
            ucUserTimeMinute += ucValue * 10; //00:x0 transfer minute
        }

        if(__CheckNum(ucUserTimeInfo[3]))
        {
            ucValue = ucUserTimeInfo[3] - 0x30;
            ucUserTimeMinute += ucValue; //00:x0 transfer minute
        }

        LOG_MSG(db_APP_SCHEDULE, "get total minute %d\r\n", ucUserTimeMinute);

        if(ucUserTimeMinute >= 1440)
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }

        //parse event function

        ucUserFunction = uiEventInfo[2] / 10 ;

        if(ucUserFunction >= eSCHEDULE_EVENT_FUNCTION_NUMBER)
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }

        ucUserEvent = uiEventInfo[2] % 10;
        LOG_MSG(db_APP_SCHEDULE, "f e (%d,%d)\r\n", ucUserFunction, ucUserEvent);
        extern const sEVENT_LIST_TRANSLATION_TEXT_ID_LUT sSchedueleEventIndexTranslateOsdTextIdLUT[eSCHEDULE_EVENT_FUNCTION_NUMBER][10];
        for(ucIndex = 0; ucIndex < 10; ucIndex++)
        {
            if(sSchedueleEventIndexTranslateOsdTextIdLUT[ucUserFunction][ucIndex].ucEventIndex == 99)
            {   //data not found
                return eCLI_ERROR_CODE_DATAOVERRANGE;
            }
            else if(sSchedueleEventIndexTranslateOsdTextIdLUT[ucUserFunction][ucIndex].uwCliValue == uiEventInfo[2])
            {   //data exist
                ucUserEvent = sSchedueleEventIndexTranslateOsdTextIdLUT[ucUserFunction][ucIndex].ucEventIndex;
                break;
            }
        }

    //-------------------
        palDataMgr_Data_Access(edcUI_SCHEDULE_EVENT_INFO, edaREAD, &sCurrentEvent);

        //backup system value
        ucBackupCurrentEventListNum = sCurrentEvent.ucCurrentEventListNum;
        palDataMgr_Data_Access(edcSCHEDULE_VIEW_WEEKDAY, edaREAD, &ucBackupCurrentWeekdayValue); //appGui_DataCode_Value_Get(edcSCHEDULE_VIEW_WEEKDAY, &ucBackupCurrentWeekdayValue);

        //set event reset

        LOG_MSG(db_APP_SCHEDULE, "-->user wday %d event %d set %d/%d (%s,%d)\r\n", ucUserChoiceWeekday, ucUserEventListNum, ucUserFunction, ucUserEvent, sCmdFormat->cTextString, strlen((char *)sCmdFormat->cTextString));

        palDataMgr_Data_Access(edcSCHEDULE_VIEW_WEEKDAY, edaWRITE_THROUGH_WITH_ACTION, &ucUserChoiceWeekday); //appGui_DataCode_Value_Set(edcSCHEDULE_VIEW_WEEKDAY, ucUserChoiceWeekday); //set weekday

        sCurrentEvent.ucCurrentEventListNum = ucUserEventListNum;
        palDataMgr_Data_Access(edcUI_SCHEDULE_EVENT_INFO, edaWRITE_RAM_ONLY_WITH_ACTION, &sCurrentEvent);//set event list
        palDataMgr_Data_Access(edcSCHEDULE_EVENT_TIME, edaWRITE_THROUGH_WITH_ACTION, &ucUserTimeMinute); //appGui_DataCode_Value_Set(edcSCHEDULE_EVENT_TIME, ucUserTimeMinute); //set time
        palDataMgr_Data_Access(edcSCHEDULE_EVENT_TYPE, edaWRITE_THROUGH_WITH_ACTION, &ucUserFunction); //appGui_DataCode_Value_Set(edcSCHEDULE_EVENT_TYPE, ucUserFunction);
        palDataMgr_Data_Access(edcSCHEDULE_EVENT_TYPE_INDEX, edaWRITE_THROUGH_WITH_ACTION, &ucUserEvent); //appGui_DataCode_Value_Set(edcSCHEDULE_EVENT_TYPE_INDEX, ucUserEvent);
        MS_SLEEP(1);
        palDataMgr_Schedule_Sort_Event_List_By_ExecuteTime();
        palLANProcScheduleSend();

        //restore system value

        sCurrentEvent.ucCurrentEventListNum = ucBackupCurrentEventListNum;
        palDataMgr_Data_Access(edcUI_SCHEDULE_EVENT_INFO, edaWRITE_RAM_ONLY_WITH_ACTION, &sCurrentEvent);
        palDataMgr_Data_Access(edcSCHEDULE_VIEW_WEEKDAY, edaWRITE_THROUGH_WITH_ACTION, &ucBackupCurrentWeekdayValue); //appGui_DataCode_Value_Set(edcSCHEDULE_VIEW_WEEKDAY, ucBackupCurrentWeekdayValue);
    }
    else if(eAccessMode == ecmRead)
    {
        //INT32 iValue = 0 ;
        //utilCommonCLI_DataConversionGet(edcWARP_MEMORY_APPLY, &iValue);
        //sCmdFormat->lData = iValue + 1 ;
    }

    return eCLI_ERROR_CODE_NO;
}
//G100_Coda_0047 //G100_Coda_0049
eCLI_ERROR_CODE utilChristie_SCW_EVRD(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)  //G100_Coda_0043
{
    if(eAccessMode == ecmWrite)
    {
       return eCLI_ERROR_CODE_COMMAND_READ_ONLY;
    }
    else if(eAccessMode == ecmRead)
    {
        UINT8 ucUserChoiceWeekday = 0;
        sSCHEDULE sUserSchedule;
        UINT8 ucIndex = 0;
        UINT8 WeekdayStr[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
        UINT8 ucOutputString[128] = {0}; //G100_Coda_0049

        memset((UINT8*)&sUserSchedule, 0, sizeof(sSCHEDULE));
        memset((UINT8*)ucOutputString, 0, sizeof(ucOutputString));

        ucUserChoiceWeekday = sCmdFormat->cMainCode[2] - 0x30;

        if(ucUserChoiceWeekday > 6)
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE ;
        }

        extern const sEVENT_LIST_TRANSLATION_TEXT_ID_LUT sSchedueleEventIndexTranslateOsdTextIdLUT[eSCHEDULE_EVENT_FUNCTION_NUMBER][10];

        palDataMgr_Schedule_All_Data_Get(&sUserSchedule);
        sprintf(ucOutputString, "\r\n%s\r\n", WeekdayStr[ucUserChoiceWeekday]);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        memset((UINT8*)ucOutputString, 0, sizeof(ucOutputString));

        sprintf(ucOutputString, "Event / Time  / Function\r\n");
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        memset((UINT8*)ucOutputString, 0, sizeof(ucOutputString));

        for(ucIndex = 0; ucIndex < 16; ucIndex++)
        {
            if(sUserSchedule.sWeekday_Info[ucUserChoiceWeekday].sWeekDayEvent[ucIndex].ucFunctionType != ets_OFF)
            {
                sprintf(ucOutputString, "%02d      %02d:%02d   %s\r\n",
                                    ucIndex + 1,
                                    sUserSchedule.sWeekday_Info[ucUserChoiceWeekday].sWeekDayEvent[ucIndex].uwTimeMin / 60,
                                    sUserSchedule.sWeekday_Info[ucUserChoiceWeekday].sWeekDayEvent[ucIndex].uwTimeMin % 60,
                                    sSchedueleEventIndexTranslateOsdTextIdLUT[sUserSchedule.sWeekday_Info[ucUserChoiceWeekday].sWeekDayEvent[ucIndex].ucFunctionType][sUserSchedule.sWeekday_Info[ucUserChoiceWeekday].sWeekDayEvent[ucIndex].ucEventIndex].ucEventIndexStr
                                    );
                __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
                memset((UINT8*)ucOutputString, 0, sizeof(ucOutputString));

            }
            else
                break;


        }

    }

    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilChristie_SCHE_ShowToday(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)  //G100_Coda_0043 //G100_Coda_0049
{
    if(eAccessMode == ecmWrite)
    {
       return eCLI_ERROR_CODE_COMMAND_READ_ONLY;
    }
    else if(eAccessMode == ecmRead)
    {
        UINT32 ucCurrentWeekdayValue = 0;
        UINT8 WeekdayStr[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
        UINT8 ucOutputString[128] = {0};
        memset((UINT8*)ucOutputString, 0, sizeof(ucOutputString));

        palDataMgr_Data_Access(edcSCHEDULE_VIEW_TODAY, edaREAD, &ucCurrentWeekdayValue); //appGui_DataCode_Value_Get(edcSCHEDULE_VIEW_TODAY, &ucCurrentWeekdayValue);

        if(ucCurrentWeekdayValue > 6)
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }

        sprintf(ucOutputString, "%s" , WeekdayStr[ucCurrentWeekdayValue]);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
    }

    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilChristie_BKI_FSRC(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)
{
    UINT8 ucVal;

    if(eAccessMode == ecmRead)
    {
        palDataMgr_Data_Access(edcBACKUPINPUT_PRIMARY_INPUT, edaREAD, &ucVal);
        switch(ucVal)
        {
            case 0:
                sCmdFormat->lData= (INT32)CLI_SOURCE_HDMI1;
            break;

             case 1:
                sCmdFormat->lData= (INT32)CLI_SOURCE_HDMI2;
            break;

            case 2:
                sCmdFormat->lData= (INT32)CLI_SOURCE_DP;	//A70Gen2_Doulas_0004
            break;

            case 3:
                sCmdFormat->lData= (INT32)CLI_SOURCE_HDBASET;
            break;
        }
    }
    else if(eAccessMode == ecmWrite)
    {
        switch(sCmdFormat->lData)
        {
            case CLI_SOURCE_HDMI1:
                ucVal = 0;
            break;

            case CLI_SOURCE_HDMI2:
                ucVal = 1;
            break;

            case CLI_SOURCE_DP:	//A70Gen2_Doulas_0004
                ucVal = 2;
            break;

            case CLI_SOURCE_HDBASET:
                ucVal = 3;
            break;

            default:
                return eCLI_ERROR_CODE_DATAOVERRANGE;
            break;
        }
        palDataMgr_Data_Access(edcBACKUPINPUT_PRIMARY_INPUT, edaWRITE_THROUGH_WITH_ACTION, &ucVal);
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilChristie_BKI_SSRC(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)
{
    UINT8 ucVal;

    if(eAccessMode == ecmRead)
    {
        palDataMgr_Data_Access(edcBACKUPINPUT_SECONDARY_INPUT, edaREAD, &ucVal);
        switch(ucVal)
        {
            case 0:
                sCmdFormat->lData= (INT32)CLI_SOURCE_HDMI1;
            break;

            case 1:
                sCmdFormat->lData= (INT32)CLI_SOURCE_HDMI2;
            break;

            case 2:
                sCmdFormat->lData= (INT32)CLI_SOURCE_DP;	//A70Gen2_Doulas_0004
            break;

            case 3:
                sCmdFormat->lData= (INT32)CLI_SOURCE_HDBASET;
            break;
        }
    }
    else if(eAccessMode == ecmWrite)
    {
        switch(sCmdFormat->lData)
        {
            case CLI_SOURCE_HDMI1:
                ucVal = 0;
            break;

            case CLI_SOURCE_HDMI2:
                ucVal = 1;
            break;

            case CLI_SOURCE_DP:	//A70Gen2_Doulas_0004
                ucVal = 2;
            break;

            case CLI_SOURCE_HDBASET:
                ucVal = 3;
            break;

            default:
                return eCLI_ERROR_CODE_DATAOVERRANGE;
            break;
        }
        palDataMgr_Data_Access(edcBACKUPINPUT_SECONDARY_INPUT, edaWRITE_THROUGH_WITH_ACTION, &ucVal);
    }
    return eCLI_ERROR_CODE_NO;
}

#if 0
// ==============================================================================
// FUNCTION NAME: utilChristie_CLI_NET_ETH0
// DESCRIPTION:
//
//
// Params:
//
// Returns:
//
//
// Modification History
// --------------------
// 2021/05/28, Larry Create
// --------------------
// ==============================================================================
eCLI_ERROR_CODE utilChristie_CLI_NET_ETH0(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)  //G100_Coda_0043 //G100_Coda_0049
{
    if(eAccessMode == ecmWrite)
    {

    }
    else if(eAccessMode == ecmRead)
    {
        UINT8 ucString[CHRISTIE_CMD_LENGTH_MAX] = {'\0'};
        UINT8 ucStrlen = 0;

        if(eEXEC_CODE_PASS == utilCommonCLI_String_Get(edcLAN_IP_ADDRESS, ucString))
        {
            ucStrlen = strlen((char*)ucString);

            if(ucStrlen > CHRISTIE_CMD_TEXT_SIZE)
            {
                ucStrlen = CHRISTIE_CMD_TEXT_SIZE;
            }
            memcpy(sCmdFormat->cTextString, ucString, ucStrlen);
            return eCLI_ERROR_CODE_NO;
        }
    }

    return eCLI_ERROR_CODE_NO;
}

// ==============================================================================
// FUNCTION NAME: utilChristie_CLI_NET_SUB0
// DESCRIPTION:
//
//
// Params:
//
// Returns:
//
//
// Modification History
// --------------------
// 2021/05/28, Larry Create
// --------------------
// ==============================================================================
eCLI_ERROR_CODE utilChristie_CLI_NET_SUB0(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)  //G100_Coda_0043 //G100_Coda_0049
{
    if(eAccessMode == ecmWrite)
    {

    }
    else if(eAccessMode == ecmRead)
    {
        UINT8 ucString[CHRISTIE_CMD_LENGTH_MAX] = {'\0'};
        UINT8 ucStrlen = 0;

        if(eEXEC_CODE_PASS == utilCommonCLI_String_Get(edcLAN_SUBNET_MASK, ucString))
        {
            ucStrlen = strlen((char*)ucString);

            if(ucStrlen > CHRISTIE_CMD_TEXT_SIZE)
            {
                ucStrlen = CHRISTIE_CMD_TEXT_SIZE;
            }
            memcpy(sCmdFormat->cTextString, ucString, ucStrlen);
            return eCLI_ERROR_CODE_NO;
        }
    }

    return eCLI_ERROR_CODE_NO;
}

// ==============================================================================
// FUNCTION NAME: utilChristie_CLI_NET_GATE
// DESCRIPTION:
//
//
// Params:
//
// Returns:
//
//
// Modification History
// --------------------
// 2021/05/28, Larry Create
// --------------------
// ==============================================================================
eCLI_ERROR_CODE utilChristie_CLI_NET_GATE(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)  //G100_Coda_0043 //G100_Coda_0049
{
    if(eAccessMode == ecmWrite)
    {

    }
    else if(eAccessMode == ecmRead)
    {
        UINT8 ucString[CHRISTIE_CMD_LENGTH_MAX] = {'\0'};
        UINT8 ucStrlen = 0;

        if(eEXEC_CODE_PASS == utilCommonCLI_String_Get(edcLAN_DEFAULT_GATEWAY, ucString))
        {
            ucStrlen = strlen((char*)ucString);

            if(ucStrlen > CHRISTIE_CMD_TEXT_SIZE)
            {
                ucStrlen = CHRISTIE_CMD_TEXT_SIZE;
            }
            memcpy(sCmdFormat->cTextString, ucString, ucStrlen);
            return eCLI_ERROR_CODE_NO;
        }
    }

    return eCLI_ERROR_CODE_NO;
}


// ==============================================================================
// FUNCTION NAME: utilChristie_CLI_NET_PDNS
// DESCRIPTION:
//
//
// Params:
//
// Returns:
//
//
// Modification History
// --------------------
// 2021/05/28, Larry Create
// --------------------
// ==============================================================================
eCLI_ERROR_CODE utilChristie_CLI_NET_PDNS(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)  //G100_Coda_0043 //G100_Coda_0049
{
    if(eAccessMode == ecmWrite)
    {

    }
    else if(eAccessMode == ecmRead)
    {
        UINT8 ucString[CHRISTIE_CMD_LENGTH_MAX] = {'\0'};
        UINT8 ucStrlen = 0;

        if(eEXEC_CODE_PASS == utilCommonCLI_String_Get(edcLAN_PRIMARY_DNS, ucString))
        {
            ucStrlen = strlen((char*)ucString);

            if(ucStrlen > CHRISTIE_CMD_TEXT_SIZE)
            {
                ucStrlen = CHRISTIE_CMD_TEXT_SIZE;
            }
            memcpy(sCmdFormat->cTextString, ucString, ucStrlen);
            return eCLI_ERROR_CODE_NO;
        }
    }

    return eCLI_ERROR_CODE_NO;
}


// ==============================================================================
// FUNCTION NAME: utilChristie_CLI_NET_SDNS
// DESCRIPTION:
//
//
// Params:
//
// Returns:
//
//
// Modification History
// --------------------
// 2021/05/28, Larry Create
// --------------------
// ==============================================================================
eCLI_ERROR_CODE utilChristie_CLI_NET_SDNS(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)  //G100_Coda_0043 //G100_Coda_0049
{
    if(eAccessMode == ecmWrite)
    {

    }
    else if(eAccessMode == ecmRead)
    {
        UINT8 ucString[CHRISTIE_CMD_LENGTH_MAX] = {'\0'};
        UINT8 ucStrlen = 0;

        if(eEXEC_CODE_PASS == utilCommonCLI_String_Get(edcLAN_SECOND_DNS, ucString))
        {
            ucStrlen = strlen((char*)ucString);

            if(ucStrlen > CHRISTIE_CMD_TEXT_SIZE)
            {
                ucStrlen = CHRISTIE_CMD_TEXT_SIZE;
            }
            memcpy(sCmdFormat->cTextString, ucString, ucStrlen);
            return eCLI_ERROR_CODE_NO;
        }
    }

    return eCLI_ERROR_CODE_NO;
}
#endif /* 0 */

eCLI_ERROR_CODE utilChristie_CLI_WRP_DBGG(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)   //A70LV_Doulas_0165
{
    if(eAccessMode == ecmRead)
    {
        palGeo_InfoPrint();  //H2PF_Simon_0038

        palDataMgr_ApplyMemoryAvailablePrint();

        utilWarp_PrintWarpConfig();
    }
    else if(eAccessMode == ecmWrite)
    {
        //turn off splash
        //palDataMgr_UI_EventSend(edcUI_EVENT_SPLASH, 0, NULL);
        //MS_SLEEP(300);  //wait OSD event done

        switch(sCmdFormat->lData)
        {
            #if 0
            case 0:
                palGeo_Draw_PNG_Pattern(0, eDRAW_565HIGH_COLOR, "/usr/bin/Pattern1.png");
                break;

            case 1:
                palGeo_Draw_PNG_Pattern(1, eDRAW_565HIGH_COLOR, "/usr/bin/Pattern2.png");
                break;

            case 2:
                palGeo_Draw_PNG_Pattern(0, eDRAW_TRUE_COLOR, "/usr/bin/Pattern1.png");
                break;

            case 3:
                palGeo_Draw_PNG_Pattern(1, eDRAW_TRUE_COLOR, "/usr/bin/Pattern2.png");
                break;

            case 900:
                {
                    UINT32 Data = 1;
                    palDataMgr_Data_Access(edcGEO_TESTPATTERN_OFF, edaWRITE_THROUGH_WITH_ACTION, &Data);
                }
                break;

            case 901:
                {
                    UINT32 Data = eCM_TEST_PATTERN_WHITE;
                    palDataMgr_Data_Access(edcSERVICE_TEST_PATTERN, edaWRITE_THROUGH_WITH_ACTION, &Data);
                }
                break;

            case 0:
                {
                    sWARPOSD_CIRCLE_GRID_DRAW_INFO sCircleNewInfo;
                    sCircleNewInfo.ucLayerMode = 1; //0:Before warp, 1:After Warp
                    sCircleNewInfo.color_red = 255;
                    sCircleNewInfo.color_green = 255;
                    sCircleNewInfo.color_blue = 255;

                    sCircleNewInfo.x_start = 0;
                    sCircleNewInfo.x_end = 40;
                    sCircleNewInfo.x_offset = 74;
                    sCircleNewInfo.x_step = 1;

                    sCircleNewInfo.y_start = 0;
                    sCircleNewInfo.y_end = 22;
                    sCircleNewInfo.y_offset = 53;
                    sCircleNewInfo.y_step = 1;

                    sCircleNewInfo.radius = 26;
                    sCircleNewInfo.space = 91;
                    sCircleNewInfo.bShowLast = 1;
                    sCircleNewInfo.bClearOSD = 0;

                    palGeo_DrawMultiCircle(sCircleNewInfo);
                }
                break;

            case 1:
                palGeo_OSD_On(eWIL_AFTER_WARP);
                break;


            case 2:
                DrawPNG_Test();
                break;

            case 3:
                #ifdef SCALER_FPGA_F34
                halWarpOSD_ClearOSD(0);
                halWarpOSD_ClearOSD(1);
                #endif
                break;

            #endif

            case 0:
                break;

            case 3:
                {
                    UINT8 Data = 1;
                    palDataMgr_Data_Access(edcADV_WARP_GRID_POINTS, edaWRITE_RAM_ONLY_WITH_ACTION, &Data);

                    utilWarp_GridPoint_SetColIndex(1);
                    utilWarp_GridPoint_SetRowIndex(0);

                    sDST_AW_WARP_POINT_POSITION Pos = {1800, 400};
                    palDataMgr_Data_Access(edcAW_WARP_POINT_POSITION, edaWRITE_RAM_ONLY_WITH_ACTION, &Pos);
                }
                break;

            case 4:
                //halWarping_FreezeImage(0);
                break;

            case 5:
                palGeo_AdvWarpShowOsdPattern(PAT_TYPE__WARP_SEL_CTRL_POINT);
                break;

            case 10:
                palImgMgr_WB_Freeze(0);
                break;
            case 11:
                palImgMgr_WB_Freeze(1);
                break;

            case 20:
                halWarping_DrawCheckboard(1, 20);
                break;

            case 21:
                halWarping_DrawCheckboard(1, 80);
                break;

            case 22:
                halWarping_DrawCheckboard(0, 80);
                break;

            case 30:  //C789 OSD HST++
                {
                    UINT16 uiData = 0;
                    halWarping_WarpOSD_HST_Get(&uiData);
                    uiData++;
                    halWarping_WarpOSD_HST_Set(uiData);

                    MS_SLEEP(100);
                    halWarping_WarpOSD_HST_Get(&uiData);

                    LOG_MSG(db_HAL_WARPING, "OSD_HST = %d\n", uiData);
                }
                break;
            case 31:  //C789 OSD HST--
                {
                    UINT16 uiData = 0;
                    halWarping_WarpOSD_HST_Get(&uiData);
                    if(uiData > 0)
                        uiData--;
                    halWarping_WarpOSD_HST_Set(uiData);

                    MS_SLEEP(100);
                    halWarping_WarpOSD_HST_Get(&uiData);

                    LOG_MSG(db_HAL_WARPING, "OSD_HST = %d\n", uiData);
                }
                break;
            case 32:  //C789 OSD VST++
                {
                    UINT16 uiData = 0;
                    halWarping_WarpOSD_VST_Get(&uiData);
                    uiData++;
                    halWarping_WarpOSD_VST_Set(uiData);

                    MS_SLEEP(100);
                    halWarping_WarpOSD_VST_Get(&uiData);

                    LOG_MSG(db_HAL_WARPING, "OSD_VST = %d\n", uiData);
                }
                break;
            case 33:  //C789 OSD VST--
                {
                    UINT16 uiData = 0;
                    halWarping_WarpOSD_VST_Get(&uiData);
                    if(uiData > 0)
                        uiData--;
                    halWarping_WarpOSD_VST_Set(uiData);

                    MS_SLEEP(100);
                    halWarping_WarpOSD_VST_Get(&uiData);

                    LOG_MSG(db_HAL_WARPING, "OSD_VST = %d\n", uiData);
                }
                break;

             case 40:
                 {
                     UINT8 ucVal[15];
                     UINT16 VSyncFreqCH1;
                     UINT16 VSyncFreqCH2;
                     halFrontEndCtrl_Main_Timing_Get(ucVal);
                     palImgMgr_InputVertRefresh2_Get_FromSyncCount(eSOURCE_WINDOW_MAIN, &VSyncFreqCH1);   //A35G2_CDS_Simon_0052
                     LOG_MSG(db_ALWAYS, "FrontEndCtrl_Main_Timing\n");
                     LOG_MSG(db_ALWAYS, "HTotal %d\n", ucVal[4]+ (ucVal[5]<<8));
                     LOG_MSG(db_ALWAYS, "HTotal %d\n", ucVal[4]+ (ucVal[5]<<8));
                     LOG_MSG(db_ALWAYS, "HActive %d\n", ucVal[6]+ (ucVal[7]<<8));
                     LOG_MSG(db_ALWAYS, "VTotal %d\n", ucVal[8]+ (ucVal[9]<<8));
                     LOG_MSG(db_ALWAYS, "VActive %d\n", ucVal[10]+ (ucVal[11]<<8));
                     LOG_MSG(db_ALWAYS, "VSyncFreqCH1 %d\n", VSyncFreqCH1);
                     MS_SLEEP(10);
                     halFrontEndCtrl_Sub_Timing_Get(ucVal);
                     palImgMgr_InputVertRefresh2_Get_FromSyncCount(eSOURCE_WINDOW_SUB, &VSyncFreqCH2);   //A35G2_CDS_Simon_0052
                     LOG_MSG(db_ALWAYS, "FrontEndCtrl_Sub_Timing\n");
                     LOG_MSG(db_ALWAYS, "HTotal %d\n", ucVal[4]+ (ucVal[5]<<8));
                     LOG_MSG(db_ALWAYS, "HTotal %d\n", ucVal[4]+ (ucVal[5]<<8));
                     LOG_MSG(db_ALWAYS, "HActive %d\n", ucVal[6]+ (ucVal[7]<<8));
                     LOG_MSG(db_ALWAYS, "VTotal %d\n", ucVal[8]+ (ucVal[9]<<8));
                     LOG_MSG(db_ALWAYS, "VActive %d\n", ucVal[10]+ (ucVal[11]<<8));
                     LOG_MSG(db_ALWAYS, "VSyncFreqCH2 %d\n", VSyncFreqCH2);
                 }
                 break;

             case 51:
                {
                    UINT8 Value = 1;
                    palDataMgr_Data_Access(edcHSG_RGBCMY_RESET, edaWRITE_RAM_ONLY_WITH_ACTION, (UINT8*)&Value);
                }
                break;

             case 52:
                {
                    UINT8 Value = 1;
                    palDataMgr_Data_Access(edcHSG_WHITE_GAIN_RESET_DEFAULT, edaWRITE_RAM_ONLY_WITH_ACTION, (UINT8*)&Value);
                }
                break;

             case 101:  utilWarp_MoveControlPoint(DIR__UP);      break;
             case 102:  utilWarp_MoveControlPoint(DIR__DOWN);      break;
             case 103:  utilWarp_MoveControlPoint(DIR__LEFT);      break;
             case 104:  utilWarp_MoveControlPoint(DIR__RIGHT);      break;

             case 105:  utilWarp_SelectControlPoint(DIR__UP);      break;
             case 106:  utilWarp_SelectControlPoint(DIR__DOWN);      break;
             case 107:  utilWarp_SelectControlPoint(DIR__LEFT);      break;
             case 108:  utilWarp_SelectControlPoint(DIR__RIGHT);      break;

             case 133:
                {
                    UINT32 Data = WARP_POINT__3x3;
                    palDataMgr_Data_Access(edcADV_WARP_GRID_POINTS, edaWRITE_RAM_ONLY_WITH_ACTION, &Data);
                }
                break;

             case 155:
                {
                    UINT32 Data = WARP_POINT__5x5;
                    palDataMgr_Data_Access(edcADV_WARP_GRID_POINTS, edaWRITE_RAM_ONLY_WITH_ACTION, &Data);
                }
                break;

             case 13333:
                {
                    UINT32 Data = WARP_POINT__33x33;
                    palDataMgr_Data_Access(edcADV_WARP_GRID_POINTS, edaWRITE_RAM_ONLY_WITH_ACTION, &Data);
                }
                break;


             case 999: utilWarp_PrintWarpConfig(); break;

             case 111:
                {
                    UINT8 Data = WARP_CTRL__ADVANCED;
                    palDataMgr_Data_Access(edcADV_WARP_CONTROL, edaWRITE_RAM_ONLY_WITH_ACTION, &Data);
                }
                break;


        }

    }

    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilChristie_OPD(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)
{
    if(eAccessMode == ecmWrite)
    {
        return eCLI_ERROR_CODE_COMMAND_READ_ONLY;
    }
    else //Read
    {
        utilOPD_CopyToUsb();
    }

    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilChristie_OPDRSET(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)
{
    if(eAccessMode == ecmWrite)
    {
        if(sCmdFormat->lData == 1)
        {
            utilOPD_ResetAllToDefault();
        }
        else
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }
    }
    else //Read
    {
        return eCLI_ERROR_CODE_COMMAND_WRITE_ONLY;
    }

    return eCLI_ERROR_CODE_NO;
}


eCLI_ERROR_CODE utilChristie_TAR(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)
{
    if(eAccessMode == ecmWrite)
    {
    }
    else //Read
    {
        palDataMgr_CopyTar2Usb();
    }
    return eCLI_ERROR_CODE_NO;
}

// ==============================================================================
// FUNCTION NAME: utilChristie_BODCUST
// DESCRIPTION:
//
//
// Params:
//
// Returns:
//
//
// Modification History
// --------------------
// 2021/11/04, Larry Create
// --------------------
// ==============================================================================
eCLI_ERROR_CODE utilChristie_BODCUST(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)
{
    eCLI_ERROR_CODE eErrorCode;
    sCLI_COMMON_FORMAT sCommonFormat;

    sCommonFormat.eAccessMode = eAccessMode;
    sCommonFormat.ucCmdFrom = sCmdFormat->ucCmdFrom;
    sCommonFormat.ucDataType = sCmdFormat->ucDataType;
    sCommonFormat.lData = sCmdFormat->lData;
    snprintf(sCommonFormat.cTextString, CLI_DATA_SIZE, "%s", sCmdFormat->cTextString);

    eErrorCode = utilCommonCLI_BODCUST(&sCommonFormat);

    sCmdFormat->lData = sCommonFormat.lData;

    return eErrorCode;
}

// ==============================================================================
// FUNCTION NAME: utilChristie_BODCUST
// DESCRIPTION:
//
//
// Params:
//
// Returns:
//
//
// Modification History
// --------------------
// 2021/11/04, Larry Create
// --------------------
// ==============================================================================
eCLI_ERROR_CODE utilChristie_BODPLAT(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)
{
    eCLI_ERROR_CODE eErrorCode;
    sCLI_COMMON_FORMAT sCommonFormat;

    sCommonFormat.eAccessMode = eAccessMode;
    sCommonFormat.ucCmdFrom = sCmdFormat->ucCmdFrom;
    sCommonFormat.ucDataType = sCmdFormat->ucDataType;
    sCommonFormat.lData = sCmdFormat->lData;
    snprintf(sCommonFormat.cTextString, CLI_DATA_SIZE, "%s", sCmdFormat->cTextString);

    eErrorCode = utilCommonCLI_BODPLAT(&sCommonFormat);

    sCmdFormat->lData = sCommonFormat.lData;

    return eErrorCode;
}

eCLI_ERROR_CODE utilChristie_CLI_TPG(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)
{
    eEXEC_CODE eResult = eEXEC_CODE_PASS;
    UINT8 cValue = 1;

    if(eAccessMode == ecmRead)
    {
        if(palDataMgr_Data_Access(edcSERVICE_TEST_PATTERN, edaREAD, &cValue) != eEXEC_CODE_PASS)
        {
            sCmdFormat->lData = (INT32) cValue;
        }
    }
    else if(eAccessMode == ecmWrite)
    {
        UINT8 cData = (UINT8)GUI2CM(edcSERVICE_TEST_PATTERN, sCmdFormat->lData);

        palDataMgr_Data_Access(edcSERVICE_MODE, edaWRITE_THROUGH_WITH_ACTION, &cValue);//test only.

        palDataMgr_Access_TestPattern_List(edcSERVICE_TEST_PATTERN, edaWRITE_THROUGH_WITH_ACTION, &cData);
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilChristie_CLI_ITP(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)
{
    eEXEC_CODE eResult = eEXEC_CODE_PASS;
    UINT8 cValue = 1;

    if(eAccessMode == ecmRead)
    {
        if(palDataMgr_Data_Access(edcOSDTEST_PATTERN, edaREAD, &cValue) != eEXEC_CODE_PASS)
        {
            sCmdFormat->lData = (INT32) cValue;
        }
    }
    else if(eAccessMode == ecmWrite)
    {
        UINT8 cData = (UINT8)GUI2CM(edcOSDTEST_PATTERN, sCmdFormat->lData);

        palDataMgr_Access_TestPattern_List(edcOSDTEST_PATTERN, edaWRITE_THROUGH_WITH_ACTION, &cData);
    }
    return eCLI_ERROR_CODE_NO;
}

//AF calibration
eCLI_ERROR_CODE utilChristie_ATF(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)//HICC2_Julie_0031
{
    UINT8 ucValue = 0;
    eDATA_CODE eDataCode = edcINVALID;

    if(eAccessMode == ecmWrite)
    {
        if(sCmdFormat->lData == 1)
        {
            eDataCode = edcAUTO_FOCUS;

            if(palDataMgr_DataCode_Control(eDataCode) != eFUNC_CONTROL_ENABLE)
            {
    			return eCLI_ERROR_CODE_SETFAIL;
            }
            else
            {
                ucValue = eAF_AC_EXECUTE;
                palDataMgr_Data_Access(eDataCode, edaWRITE_THROUGH_WITH_ACTION, &ucValue);
            }

        }
        else
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }
    }
    else //Read
    {
        eDataCode = edcAUTO_FOCUS_STATUS;

        if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(eDataCode))  //available check
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
        else
        {
            palDataMgr_Data_Access(eDataCode, edaREAD, &ucValue);
            sCmdFormat->lData = ucValue;
        }
    }

    return eCLI_ERROR_CODE_NO;
}

//AC calibration
eCLI_ERROR_CODE utilChristie_ATW(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)//HICC2_Julie_0031
{
    UINT8 ucValue = 0;
    eDATA_CODE eDataCode = edcINVALID;

    if(eAccessMode == ecmWrite)
    {
        if(sCmdFormat->lData == 1)
        {
            eDataCode = edcAUTO_COLOR;

            if(palDataMgr_DataCode_Control(eDataCode) != eFUNC_CONTROL_ENABLE)
            {
    			return eCLI_ERROR_CODE_SETFAIL;
            }
            else
            {
                ucValue = (UINT8)eAF_AC_EXECUTE_NO_CHECK;
                palDataMgr_Data_Access(eDataCode, edaWRITE_THROUGH_WITH_ACTION, &ucValue);
            }
        }
        else
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }
    }
    else //Read
    {
        eDataCode = edcAUTO_WALL_COLOR_STATUS;

        if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(eDataCode))  //available check
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
        else
        {
            palDataMgr_Data_Access(eDataCode, edaREAD, &ucValue);
            sCmdFormat->lData = ucValue;
        }
    }

    return eCLI_ERROR_CODE_NO;
}

//ACU calibration
eCLI_ERROR_CODE utilChristie_ACC(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)
{
    UINT8 ucValue = 0;
    eDATA_CODE eDataCode = edcINVALID;

    if(eAccessMode == ecmWrite)
    {
        if(sCmdFormat->lData == 1)
        {
            eDataCode = edcACU_EXECUTE;

            if(palDataMgr_DataCode_Control(eDataCode) != eFUNC_CONTROL_ENABLE)    //H2PF_Simon_0038
            {
    			return eCLI_ERROR_CODE_SETFAIL;
            }
            else
            {
                ucValue = (UINT8)eACU_EXECUTING_NO_CHECK;
                palDataMgr_Data_Access(eDataCode, edaWRITE_THROUGH_WITH_ACTION, &ucValue);
            }
        }
        else
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }
    }
    else //Read
    {
        eDataCode = edcACU_STATUS;

        if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(eDataCode))  //available check
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
        else
        {
            palDataMgr_Data_Access(eDataCode, edaREAD, &ucValue);
            sCmdFormat->lData = ucValue;
        }
    }

    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilChristie_CLI_SYS_LANS(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)      //HICC2_Doulas_0057
{
    if(eAccessMode == ecmRead)
    {
        sCmdFormat->lData = 4;
        return eCLI_ERROR_CODE_COMMAND_WRITE_ONLY;
    }
    else if(eAccessMode == ecmWrite)
    {
        UINT8 cData = 1;
        UINT8 cData1 = 0;
        UINT8 cData2 = 0;
        UINT8 cData3 = 0;
        UINT8 cData4 = 0;
        if(sCmdFormat->lData == 1)
        {
            palDataMgr_Data_Access(edcExtron, edaWRITE_THROUGH_WITH_ACTION, &cData);
            palDataMgr_Data_Access(edcPJ_Link, edaWRITE_THROUGH_WITH_ACTION, &cData);
            palDataMgr_Data_Access(edcAMX, edaWRITE_THROUGH_WITH_ACTION, &cData);
            palDataMgr_Data_Access(edcTelnet, edaWRITE_THROUGH_WITH_ACTION, &cData);
            palDataMgr_Data_Access(edcHTTP, edaWRITE_THROUGH_WITH_ACTION, &cData);
        }
        else
        {
            palDataMgr_Data_Access(edcExtron, edaREAD, &cData);
            palDataMgr_Data_Access(edcPJ_Link, edaREAD, &cData1);
            palDataMgr_Data_Access(edcAMX, edaREAD, &cData2);
            palDataMgr_Data_Access(edcTelnet, edaREAD, &cData3);
            palDataMgr_Data_Access(edcHTTP, edaREAD, &cData4);
            LOG_MSG(db_ALWAYS, "Read (%d,%d,%d,%d,%d) \r\n",cData,cData1,cData2,cData3,cData4);
        }
    }
    return eCLI_ERROR_CODE_NO;
}
//G100_Coda_0013
eCLI_ERROR_CODE utilChristie_Debug_Schedule(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)  //G100_Coda
{
    if(eAccessMode == ecmWrite)
    {
    	switch(sCmdFormat->lData)
    	{
    	    case 0:
                //GuiCb.fpGui_Send_MenuOpen_Schedule_NextMenu_L2Cb(); //G100 move to appgui.c
    	    break;

    	    case 1:
    	    {   UINT32 iValue = 0;
    	        utilCommonCLI_DataConversionGet(edcCOLOR_ENHANCEMENT, &iValue);
    	        LOG_MSG(db_ALWAYS, "edcCOLOR_ENHANCEMENT = %d\r\n", iValue);
    	    }
    	    break;

    	    case 2:
    	    {   UINT32 iValue = 0;
    	        utilCommonCLI_DataConversionSet(edcLENS_DETECTION, 0);
    	        LOG_MSG(db_ALWAYS, "edcLENS_DETECTION  = %d\r\n", iValue);
    	    }
    	    break;

            #ifdef CUSTOM_OPTOMA  //A35G2_Jacky_0008
            case 3:
    	    {
    	        _sStructPayload sStructPayload;
    	        _sCcv2 sLanCcv2 = {
    	          .Address = "192.168.0.3",
    	          .IpId = 6,
    	          .Port = 41795,
                };
    	        memcpy((UINT8*)&sStructPayload.Data, (UINT8*)&sLanCcv2, sizeof(_sCcv2));

                UINT16 uwStructID = 0 ;//0 : time

                palLANProcSturctDataSend(uwStructID, sizeof(_sStructPayload), (void *)&sStructPayload);
                LOG_MSG(db_ALWAYS, "utilIpc_SendData Time Struct\n");
            }
            break;
            #endif

    	    case 10:  //test date and time setting //G100_Coda_0016
    	        {
    	            _sStructPayload sStructPayload;
    	            _sDateTime sLanDateTime = {
    	            .Year = "2020",
    	            .Month = "2",
    	            .Day = "14",
    	            .Hour = "13",
    	            .Minute="12",
    	            .Mode = "Manual",
    	            .Second = "10",
    	            .Timezone = "Etc/GMT-8",
    	            .NTP = "time.google.com.tw",
    	            .Interval = "Hourly",
    	            .DST = "0",
    	            .Dstzone = "Etc/GMT-8",
                    };
    	            memcpy((UINT8*)&sStructPayload.Data, (UINT8*)&sLanDateTime, sizeof(_sDateTime));

                    UINT16 uwStructID = 0 ;//0 : time

                    palLANProcSturctDataSend(uwStructID, sizeof(_sStructPayload), (void *)&sStructPayload);
                    LOG_MSG(db_ALWAYS, "utilIpc_SendData Time Struct\n");
                }
                break;

            case 9:	 //G100_Coda_00110
                palDataMgr_DateTime_Build_LanPacket_ResetDefault();
                break;

            case 11:  //test Schedule struct setting //G100_Coda_0016
    	        {
    	            sLAN_SCHEDULE_StructPayload sStructPayload;
    	            sLAN_SCHEDULE_PACKET sSendSchedule = {
    	            .ScheduleModeEn                = "1",
    	            .Weekday[0].Enable             = "1",
    	            .Weekday[0].EventList[0].Time     = "12:34",
    	            .Weekday[0].EventList[0].Func     = 1,
    	            .Weekday[0].EventList[0].Event    = 2,
                    };
    	            memcpy((UINT8*)&sStructPayload.Data, (UINT8*)&sSendSchedule, sizeof(sLAN_SCHEDULE_PACKET));

                    UINT16 uwStructID = 1 ;//0 : time

                    palDataMgr_Data_Access(edcSTRUCT_SCHEDULE, edaWRITE_THROUGH_WITH_ACTION, &sStructPayload);

                    //palDataMgr_Schedule_Unpack_LanPacket(&sSendSchedule);

                    //palLANProcSturctDataSend(uwStructID, sizeof(sLAN_SCHEDULE_StructPayload), (void *)&sStructPayload);

                    LOG_MSG(db_ALWAYS, "palLANProcSturctDataSend Schedule Struct %d\n", sizeof(sLAN_SCHEDULE_StructPayload));
                }
    	    break;

    	    case 12:  //test Schedule struct setting  //G100_Coda_0021
    	        {
    	            sLAN_SCHEDULE_StructPayload sStructPayload;
    	            sLAN_SCHEDULE_PACKET sSendSchedule;
    	            memset((UINT8*)&sSendSchedule, 0, sizeof(sLAN_SCHEDULE_PACKET));
    	            palDataMgr_Schedule_Build_LanPacket(&sSendSchedule);
    	            memcpy((UINT8*)&sStructPayload.Data, (UINT8*)&sSendSchedule, sizeof(sLAN_SCHEDULE_PACKET));

                    UINT16 uwStructID = 1 ;//0 : time

                    //palDataMgr_Schedule_Unpack_LanPacket(&sSendSchedule);

                    palLANProcSturctDataSend(uwStructID, sizeof(sLAN_SCHEDULE_StructPayload), (void *)&sStructPayload);

                    LOG_MSG(db_ALWAYS, "Build Schedule Struct %d\n", sizeof(sLAN_SCHEDULE_StructPayload));
                }
    	    break;

            case 30:
                palDataMgr_Schedule_Sort_Event_List_By_ExecuteTime();
                break;

			default:
				break;
    	}
    }

	if(eAccessMode == ecmRead)
	{
		LOG_MSG(db_ALWAYS, "\r\n[debug Mode - Schuedule]\r\n");
		LOG_MSG(db_ALWAYS, "0     : Opeon Schedule Menu\r\n");

        sLAN_SCHEDULE_StructPayload sStructPayload;
        palDataMgr_Data_Access(edcSTRUCT_SCHEDULE, edaREAD, &sStructPayload);
        ASSERT_ALWAYS();

        sSCHEDULE Schedule_Info;
        palDataMgr_Data_Access(edcSCHEDULE_ALL_DATA, edaREAD, &sStructPayload);

        ASSERT_ALWAYS();
	}

    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilChristie_Debug_AF_Msg(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)  //HICC2_Simon_0018
{
    if(eAccessMode == ecmWrite)
    {
        palDataMgr_Data_Access(edcAUTO_FOCUS_STATUS, edaWRITE_THROUGH_WITH_ACTION, &sCmdFormat->lData);
        palDataMgr_UI_EventSend(edcUI_EVENT_AF_CAL_FAIL, TRUE, NULL);
        return eCLI_ERROR_CODE_NO;
    }
}

eCLI_ERROR_CODE utilChristie_Debug_AC_Msg(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)  //HICC2_Simon_0018
{
    if(eAccessMode == ecmWrite)
    {
        palDataMgr_Data_Access(edcAUTO_WALL_COLOR_STATUS, edaWRITE_THROUGH_WITH_ACTION, &sCmdFormat->lData);
        palDataMgr_UI_EventSend(edcUI_EVENT_AC_CAL_FAIL, sCmdFormat->lData, NULL);
        return eCLI_ERROR_CODE_NO;
    }
}

eCLI_ERROR_CODE utilChristie_Debug_ACU_Msg(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)  //HICC2_Simon_0018
{
    if(eAccessMode == ecmWrite)
    {
        palDataMgr_Data_Access(edcACU_STATUS, edaWRITE_THROUGH_WITH_ACTION, &sCmdFormat->lData);
        palDataMgr_UI_EventSend(edcUI_EVENT_ACU_CAL_FAIL, sCmdFormat->lData, NULL);
        return eCLI_ERROR_CODE_NO;
    }
}

#if(BIST_ENABLE) //HICC2_Steven_0001
// ==============================================================================
eCLI_ERROR_CODE utilChristie_BIST_SET(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)
{
    eEXEC_CODE eResult = eEXEC_CODE_PASS;
    UINT8 cValue = 1;

    if(eAccessMode == ecmRead)
    {
    	return eCLI_ERROR_CODE_COMMAND_WRITE_ONLY;
    }
    else if(eAccessMode == ecmWrite)
    {
    	if(sCmdFormat->lData == 1)
    	{
        	//UINT8 ucBIST = 1;  //HICC2_Steven_0008
        	//palDataMgr_Data_Access(edcBIST_ENABLE, edaWRITE_THROUGH_WITH_ACTION,(void*)&ucBIST);
    		utilBIST_Handle(1);   //HICC2_Steven_0012
    	}
    }
    return eCLI_ERROR_CODE_NO;
}
#endif

#ifdef SCALER_FPGA_F34
extern int dvProAV_AccessRaw_Read(UINT8 bank, UINT8 offset, UINT16 len, UINT8 *data);
extern int dvProAV_AccessRaw_Write(UINT8 bank, UINT8 offset, UINT16 len, UINT8 *data);
extern int dvProAV_Access_Write(UINT32 regEnum, UINT32 data);
extern int dvProAV_Access_Read(UINT32 regEnum, UINT32 *data);
#endif

eCLI_ERROR_CODE utilChristie_PRO(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)
{
    eCLI_ERROR_CODE eErrorCode;
    sCLI_COMMON_FORMAT sCommonFormat;

    sCommonFormat.eAccessMode = eAccessMode;
    sCommonFormat.ucCmdFrom = sCmdFormat->ucCmdFrom;
    sCommonFormat.ucDataType = sCmdFormat->ucDataType;
    sCommonFormat.lData = sCmdFormat->lData;
    sCommonFormat.fData = sCmdFormat->fData;

    eErrorCode = utilCommonCLI_PROSETT(&sCommonFormat);

    return eErrorCode;
}

eCLI_ERROR_CODE utilChristie_PROSETT(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)
{
    eCLI_ERROR_CODE eErrorCode;
    sCLI_COMMON_FORMAT sCommonFormat;

    sCommonFormat.eAccessMode = eAccessMode;
    sCommonFormat.ucCmdFrom = sCmdFormat->ucCmdFrom;
    sCommonFormat.ucDataType = sCmdFormat->ucDataType;
    sCommonFormat.lData = sCmdFormat->lData;
    sCommonFormat.fData = sCmdFormat->fData;
    snprintf(sCommonFormat.cTextString, CLI_DATA_SIZE, "%s", sCmdFormat->cTextString);

    eErrorCode = utilCommonCLI_PROSETT(&sCommonFormat);

    return eErrorCode;
}

eCLI_ERROR_CODE utilChristie_PROGETT(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)
{
    eCLI_ERROR_CODE eErrorCode;
    sCLI_COMMON_FORMAT sCommonFormat;

    sCommonFormat.eAccessMode = eAccessMode;
    sCommonFormat.ucCmdFrom = sCmdFormat->ucCmdFrom;
    sCommonFormat.ucDataType = sCmdFormat->ucDataType;
    sCommonFormat.lData = sCmdFormat->lData;
    sCommonFormat.fData = sCmdFormat->fData;
    snprintf(sCommonFormat.cTextString, CLI_DATA_SIZE, "%s", sCmdFormat->cTextString);

    eErrorCode = utilCommonCLI_PROGETT(&sCommonFormat);

    sprintf(sCmdFormat->cTextString, "(%s+%s!%s)\n", sCmdFormat->cMainCode, sCmdFormat->cSubCode, sCommonFormat.cTextString);

    __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, sCmdFormat->cTextString);

    return eErrorCode;
}

eCLI_ERROR_CODE utilChristie_CLI_XPR_ENAX(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)
{
    UINT8 data = 0;

    if(eAccessMode == ecmRead)
    {
        palFormatterMgr_XPR_Close_Loop_Enable_Axis0_Get(&data);
        sCmdFormat->lData = (INT32)data;
    }
    else if(eAccessMode == ecmWrite)
    {
        data = sCmdFormat->lData;
        if(data < 2)
        {
            palFormatterMgr_XPR_Close_Loop_Enable_Axis0_Set(data);
        }
        else
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilChristie_CLI_XPR_ENAY(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)
{
    UINT8 data = 0;

    if(eAccessMode == ecmRead)
    {
        palFormatterMgr_XPR_Close_Loop_Enable_Axis1_Get(&data);
        sCmdFormat->lData = (INT32)data;
    }
    else if(eAccessMode == ecmWrite)
    {
        data = sCmdFormat->lData;
        if(data < 2)
        {
            palFormatterMgr_XPR_Close_Loop_Enable_Axis1_Set(data);
        }
        else
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilChristie_CLI_XPR_GANX(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)
{
    UINT32 data = 0;

    if(eAccessMode == ecmRead)
    {
        palFormatterMgr_XPR_Gain_of_Axis0_Get(&data);
        sCmdFormat->lData = (INT32)data;
    }
    else if(eAccessMode == ecmWrite)
    {

    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilChristie_CLI_XPR_GANY(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)
{
    UINT32 data = 0;

    if(eAccessMode == ecmRead)
    {
        palFormatterMgr_XPR_Gain_of_Axis1_Get(&data);
        sCmdFormat->lData = (INT32)data;
    }
    else if(eAccessMode == ecmWrite)
    {

    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilChristie_CLI_XPR_SGLX(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)
{
    UINT32 data = 0;

    if(eAccessMode == ecmRead)
    {
        palFormatterMgr_XPR_Segment_Length_of_Axis0_Get(&data);
        sCmdFormat->lData = (INT32)data;
    }
    else if(eAccessMode == ecmWrite)
    {

    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilChristie_CLI_XPR_SGLY(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)
{
    UINT32 data = 0;

    if(eAccessMode == ecmRead)
    {
        palFormatterMgr_XPR_Segment_Length_of_Axis1_Get(&data);
        sCmdFormat->lData = (INT32)data;
    }
    else if(eAccessMode == ecmWrite)
    {

    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilChristie_CLI_XPR_SNGT(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)
{
    UINT8 data[32] = {0};

    if(eAccessMode == ecmRead)
    {
        palFormatterMgr_XPR_SN_Get(data);

        snprintf(sCmdFormat->cTextString, CLI_DATA_SIZE, "%s", data);
    }
    else if(eAccessMode == ecmWrite)
    {

    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilChristie_CLI_XPR_ERROR_CODE_0(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)
{
    UINT32 data = 0;

    if(eAccessMode == ecmRead)
    {
        palFormatterMgr_XPR_CloseLoop_ErrorCode0(&data);

        sCmdFormat->lData = (INT32)data;
    }
    else if(eAccessMode == ecmWrite)
    {

    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilChristie_CLI_XPR_ERROR_CODE_1(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)
{
    UINT32 data = 0;

    if(eAccessMode == ecmRead)
    {
        palFormatterMgr_XPR_CloseLoop_ErrorCode1(&data);

        sCmdFormat->lData = (INT32)data;
    }
    else if(eAccessMode == ecmWrite)
    {

    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilChristie_CLI_XPR_ERROR_CODE_RESET(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)
{
    UINT8 data = 0;

    if(eAccessMode == ecmRead)
    {
    }
    else if(eAccessMode == ecmWrite)
    {
        data = sCmdFormat->lData;
        if(data < 2)
        {
            palFormatterMgr_XPR_Close_Loop_Error_Reset_Set(data);
        }
        else
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilChristie_CLI_GAIN_WRITE_EEPROM(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)
{
    UINT8 data = 0;

    if(eAccessMode == ecmRead)
    {
    }
    else if(eAccessMode == ecmWrite)
    {
        data = sCmdFormat->lData;
        if(data < 2)
        {
            palFormatterMgr_XPR_Gain_Write_EEPROM();
        }
        else
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilChristie_CLI_XPR_TARGET_ADC0(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)
{
    UINT32 data = 0;

    if(eAccessMode == ecmRead)
    {
        palFormatterMgr_XPR_EepromTargetAdcAxis0_Get(&data);

        sCmdFormat->lData = (INT32)data;
    }
    else if(eAccessMode == ecmWrite)
    {

    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilChristie_CLI_XPR_TARGET_ADC1(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)
{
    UINT32 data = 0;

    if(eAccessMode == ecmRead)
    {
        palFormatterMgr_XPR_EepromTargetAdcAxis1_Get(&data);

        sCmdFormat->lData = (INT32)data;
    }
    else if(eAccessMode == ecmWrite)
    {

    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilChristie_CLI_XPR_ZDATA0(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)
{
    UINT32 data = 0;

    if(eAccessMode == ecmRead)
    {
        palFormatterMgr_XPR_Eeprom_ZData0_Get(&data);

        sCmdFormat->lData = (INT32)data;
    }
    else if(eAccessMode == ecmWrite)
    {

    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilChristie_CLI_XPR_ZDATA1(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)
{
    UINT32 data = 0;

    if(eAccessMode == ecmRead)
    {
        palFormatterMgr_XPR_Eeprom_ZData1_Get(&data);

        sCmdFormat->lData = (INT32)data;
    }
    else if(eAccessMode == ecmWrite)
    {

    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilChristie_CLI_XPR_CLOSE_LOOP_VALID(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)
{
    UINT8 data = 0;

    if(eAccessMode == ecmRead)
    {
        palFormatterMgr_XPR_CloseLoop_Valid(&data);
        sCmdFormat->lData = (INT32)data;
    }
    else if(eAccessMode == ecmWrite)
    {
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilChristie_CLI_FAN_LD_INFO(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)//HICC2_Julie_0022//HICC2_Julie_0027
{
    if(eAccessMode == ecmRead)
    {
        char cStrTemp[8] = {""}, cStrFunc[8] = {""};
        char  ucString[64] = {0}, ucOutputString[64] = {0};
        UINT8 cData = 0;
        UINT32 uiCode = 0, uiDataCode[8] = {0};
    	sUI_ITEM_CFG sUI_Info;

        memcpy(cStrTemp, sCmdFormat->cSubCode, strlen((char *)sCmdFormat->cSubCode));
        sscanf(cStrTemp, "%[A-z]%d", cStrFunc, &cData);

        if(strncmp(cStrFunc, "FA", 2) == 0)
        {
            if(cData > 0 && cData < 17)
            {
                uiCode = edcFAN_RPM_01 + cData - 1;
            }
            else if(cData >= 17 && cData <= 20)
            {
                uiCode = edcFAN_RPM_17 + cData - 17;
            }
            else
            {
                return eCLI_ERROR_CODE_DATAOVERRANGE;
            }
        }
        else if(strncmp(cStrFunc, "LD", 2) == 0)
        {
            if(cData > 0 && cData < 17)
            {
                uiCode = edcLD_INFO_01 + cData - 1;
            }
            else
            {
                return eCLI_ERROR_CODE_DATAOVERRANGE;
            }
        }
        else
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        uiDataCode[0] = uiCode;
        if(GetItemInfo_ByDatacode(1, uiDataCode, &sUI_Info) == FALSE)
        {
            return eCLI_ERROR_CODE_DataSpecialCheckFail;
        }
        else
        {
            if(sUI_Info.Hide != 0)
            {
                return eCLI_ERROR_CODE_DATAOVERRANGE;
            }
        }

        if(palDataMgr_Data_Access(uiCode, edaREAD, &ucString) != eEXEC_CODE_PASS)
        {
            return eCLI_ERROR_CODE_ReplyFail;
        }
        else
        {
            sprintf(ucOutputString, "\r(%s+%s!\"%s\")\r\n",sCmdFormat->cMainCode, sCmdFormat->cSubCode, ucString);
            __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        }
    }
    else if(eAccessMode == ecmWrite)
    {
        return eCLI_ERROR_CODE_COMMAND_READ_ONLY;
    }

    return eCLI_ERROR_CODE_NO;
}

// ==============================================================================
// FUNCTION NAME: utilChristie_CLI_GUP
// DESCRIPTION:
//
//
// Params:
//
// Returns:
//
//
// Modification History
// --------------------
// 2024/08/02, Larry Create
// --------------------
// ==============================================================================
eCLI_ERROR_CODE utilChristie_CLI_GUP(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)
{
    UINT32 iValue = 0;

    if(eAccessMode == ecmRead)
    {
    	utilCommonCLI_DataConversionGet(edcPROJECTOR_GROUP_STATUS, &iValue);

        if(iValue)
        {
            sCmdFormat->lData = 1;
        }
        else
        {
            sCmdFormat->lData = 0;
        }
    }
    else if(eAccessMode == ecmWrite)
    {
    	utilCommonCLI_DataConversionGet(edcPROJECTOR_GROUP_STATUS, &iValue);

        if(iValue == eCM_GROUP_STATUS_SLAVE)
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        iValue = sCmdFormat->lData;

        if(iValue == 1)
        {
            iValue = 2;
        }
        else if(iValue == 0)
        {
            iValue = 0;
        }
        else
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }

        utilCommonCLI_DataConversionSet((eDATA_CODE)edcPROJECTOR_GROUP_ENABLE, iValue);

    }
    return eCLI_ERROR_CODE_NO;
}

// ==============================================================================
// FUNCTION NAME: utilChristie_CLI_GUP_FRZE
// DESCRIPTION:
//
//
// Params:
//
// Returns:
//
//
// Modification History
// --------------------
// 2024/08/02, Larry Create
// --------------------
// ==============================================================================
eCLI_ERROR_CODE utilChristie_CLI_GUP_FRZE(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)
{
    UINT32 iValue = 0;

    if(eAccessMode == ecmRead)
    {
        if(utilCommonCLI_ControlFunction(eacGROUPING_FUNCTION_ENABLE) != eFUNC_CONTROL_ENABLE)
        {
            return eCLI_ERROR_CODE_REQUESTFAIL;
        }

        utilCommonCLI_DataConversionGet(edcGROUP_EVENT_FREEZE, &iValue);

        if(iValue)
        {
            sCmdFormat->lData = 1;
        }
        else
        {
            sCmdFormat->lData = 0;
        }
    }
    else if(eAccessMode == ecmWrite)
    {
        if(iValue > 1)
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }

    	utilCommonCLI_DataConversionGet(edcPROJECTOR_GROUP_STATUS, &iValue);

        if(iValue != eCM_GROUP_STATUS_MASTER)
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        iValue = sCmdFormat->lData;

        utilCommonCLI_DataConversionSet((eDATA_CODE)edcGROUP_EVENT_FREEZE, iValue);
    }
    return eCLI_ERROR_CODE_NO;
}


// ==============================================================================
// FUNCTION NAME: utilChristie_CLI_GUP_SHUT
// DESCRIPTION:
//
//
// Params:
//
// Returns:
//
//
// Modification History
// --------------------
// 2024/08/05, Larry Create
// --------------------
// ==============================================================================
eCLI_ERROR_CODE utilChristie_CLI_GUP_SHUT(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)
{
    UINT32 iValue = 0;

    if(eAccessMode == ecmRead)
    {
        if(utilCommonCLI_ControlFunction(eacGROUPING_FUNCTION_ENABLE) != eFUNC_CONTROL_ENABLE)
        {
            return eCLI_ERROR_CODE_REQUESTFAIL;
        }

        utilCommonCLI_DataConversionGet(edcGROUP_EVENT_SHUTTER, &iValue);

        if(iValue)
        {
            sCmdFormat->lData = 1;
        }
        else
        {
            sCmdFormat->lData = 0;
        }
    }
    else if(eAccessMode == ecmWrite)
    {
        if(iValue > 1)
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }

    	utilCommonCLI_DataConversionGet(edcPROJECTOR_GROUP_STATUS, &iValue);

        if(iValue != eCM_GROUP_STATUS_MASTER)
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        iValue = sCmdFormat->lData;

        utilCommonCLI_DataConversionSet((eDATA_CODE)edcGROUP_EVENT_SHUTTER, iValue);
    }
    return eCLI_ERROR_CODE_NO;
}



// ==============================================================================
// FUNCTION NAME: utilChristie_CLI_GUP_SCHE
// DESCRIPTION:
//
//
// Params:
//
// Returns:
//
//
// Modification History
// --------------------
// 2024/08/05, Larry Create
// --------------------
// ==============================================================================
eCLI_ERROR_CODE utilChristie_CLI_GUP_SCHE(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)
{
    UINT32 iValue = 0;

    if(eAccessMode == ecmRead)
    {
        if(utilCommonCLI_ControlFunction(eacGROUPING_FUNCTION_ENABLE) != eFUNC_CONTROL_ENABLE)
        {
            return eCLI_ERROR_CODE_REQUESTFAIL;
        }

        utilCommonCLI_DataConversionGet(edcGROUP_EVENT_SCHEDULE, &iValue);

        if(iValue)
        {
            sCmdFormat->lData = 1;
        }
        else
        {
            sCmdFormat->lData = 0;
        }
    }
    else if(eAccessMode == ecmWrite)
    {
        if(iValue > 1)
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }

    	utilCommonCLI_DataConversionGet(edcPROJECTOR_GROUP_STATUS, &iValue);

        if(iValue != eCM_GROUP_STATUS_MASTER)
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        iValue = sCmdFormat->lData;

        utilCommonCLI_DataConversionSet((eDATA_CODE)edcGROUP_EVENT_SCHEDULE, iValue);
    }
    return eCLI_ERROR_CODE_NO;
}


// ==============================================================================
// FUNCTION NAME: utilChristie_CLI_GUP_ATFE
// DESCRIPTION:
//
//
// Params:
//
// Returns:
//
//
// Modification History
// --------------------
// 2024/08/05, Larry Create
// --------------------
// ==============================================================================
eCLI_ERROR_CODE utilChristie_CLI_GUP_ATFE(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)
{
    UINT32 iValue = 0;

    if(eAccessMode == ecmRead)
    {
        if(utilCommonCLI_ControlFunction(eacGROUPING_FUNCTION_ENABLE) != eFUNC_CONTROL_ENABLE)
        {
            return eCLI_ERROR_CODE_REQUESTFAIL;
        }

        utilCommonCLI_DataConversionGet(edcGROUP_EVENT_AUTO_FOCUS, &iValue);

        if(iValue)
        {
            sCmdFormat->lData = 1;
        }
        else
        {
            sCmdFormat->lData = 0;
        }
    }
    else if(eAccessMode == ecmWrite)
    {
        if(iValue > 1)
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }

    	utilCommonCLI_DataConversionGet(edcPROJECTOR_GROUP_STATUS, &iValue);

        if(iValue != eCM_GROUP_STATUS_MASTER)
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        iValue = sCmdFormat->lData;

        utilCommonCLI_DataConversionSet((eDATA_CODE)edcGROUP_EVENT_AUTO_FOCUS, iValue);
    }
    return eCLI_ERROR_CODE_NO;
}

// ==============================================================================
// FUNCTION NAME: utilChristie_CLI_GUP_ACME
// DESCRIPTION:
//
//
// Params:
//
// Returns:
//
//
// Modification History
// --------------------
// 2024/08/05, Larry Create
// --------------------
// ==============================================================================
eCLI_ERROR_CODE utilChristie_CLI_GUP_ACME(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)
{
    UINT32 iValue = 0;

    if(eAccessMode == ecmRead)
    {
        if(utilCommonCLI_ControlFunction(eacGROUPING_FUNCTION_ENABLE) != eFUNC_CONTROL_ENABLE)
        {
            return eCLI_ERROR_CODE_REQUESTFAIL;
        }

        utilCommonCLI_DataConversionGet(edcGROUP_EVENT_MUTLI_COLOR_MATCH, &iValue);

        if(iValue)
        {
            sCmdFormat->lData = 1;
        }
        else
        {
            sCmdFormat->lData = 0;
        }
    }
    else if(eAccessMode == ecmWrite)
    {
        if(iValue > 1)
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }

    	utilCommonCLI_DataConversionGet(edcPROJECTOR_GROUP_STATUS, &iValue);

        if(iValue != eCM_GROUP_STATUS_MASTER)
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        iValue = sCmdFormat->lData;

        utilCommonCLI_DataConversionSet((eDATA_CODE)edcGROUP_EVENT_MUTLI_COLOR_MATCH, iValue);
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilChristie_Manual_Date_Time(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)//HICC2_Julie_0039
{
    UINT16 ucValue = 0;
    eDATA_CODE eDataCode = edcINVALID;
    eDATA_CODE eDataCode_Control = edcDATE_TIME;

    //find datacode
    if(__CheckCmd(sCmdFormat->cMainCode, DDY_CMD, CHRISTIE_MAIN_CMD_LEN) == 1)      eDataCode = edcDATE_YEAR;
    else if(__CheckCmd(sCmdFormat->cMainCode, DDM_CMD, CHRISTIE_MAIN_CMD_LEN) == 1) eDataCode = edcDATE_MONTH;
    else if(__CheckCmd(sCmdFormat->cMainCode, DDD_CMD, CHRISTIE_MAIN_CMD_LEN) == 1) eDataCode = edcDATE_DAY;
    else if(__CheckCmd(sCmdFormat->cMainCode, DTH_CMD, CHRISTIE_MAIN_CMD_LEN) == 1) eDataCode = edcDATE_HOUR;
    else if(__CheckCmd(sCmdFormat->cMainCode, DTM_CMD, CHRISTIE_MAIN_CMD_LEN) == 1) eDataCode = edcDATE_MINUTE;

    //available check
	if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(eDataCode_Control))
	{
        if(eAccessMode == ecmWrite)
            return eCLI_ERROR_CODE_SETFAIL;
        else
            return eCLI_ERROR_CODE_REQUESTFAIL;
	}

    if(eAccessMode == ecmWrite)
    {
        ucValue = (UINT16)sCmdFormat->lData;

        if(eEXEC_CODE_PASS == utilCommonCLI_RangeCheck(eDataCode, (INT32)ucValue))
        {
            if(eEXEC_CODE_PASS != utilCommonCLI_DataConversionSet(eDataCode, ucValue))
            {
                return eCLI_ERROR_CODE_SETFAIL;
            }
        }
        else
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }
    }
    else //Read
    {
        if(eEXEC_CODE_PASS != utilCommonCLI_DataConversionGet(eDataCode, (INT32*)&ucValue))
        {
            return eCLI_ERROR_CODE_REQUESTFAIL;
        }
		sCmdFormat->lData = (INT32) ucValue;
    }

    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilChristie_UseNTPServer_CMD(eCLI_MODE eAccessMode, sCLI_CHRISTIE_FORMAT* sCmdFormat)//HICC2_Julie_0039
{
    UINT16 ucValue = 0;
    eDATA_CODE eDataCode = edcINVALID;

    //find datacode
    if(__CheckCmd(sCmdFormat->cMainCode, DST_CMD, CHRISTIE_MAIN_CMD_LEN) == 1)      eDataCode = edcDATE_DAYNIGHT_SAVING_TIME;
    else if(__CheckCmd(sCmdFormat->cMainCode, NTP_CMD, CHRISTIE_MAIN_CMD_LEN) == 1) eDataCode = edcDATE_NTP_SERVER;
    else if(__CheckCmd(sCmdFormat->cMainCode, UTC_CMD, CHRISTIE_MAIN_CMD_LEN) == 1) eDataCode = edcDATE_ZONE;
    else if(__CheckCmd(sCmdFormat->cMainCode, DUI_CMD, CHRISTIE_MAIN_CMD_LEN) == 1) eDataCode = edcDATE_UPDATE_INTERVAL;

    //available check
	if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(eDataCode))
	{
        if(eAccessMode == ecmWrite)
            return eCLI_ERROR_CODE_SETFAIL;
        else
            return eCLI_ERROR_CODE_REQUESTFAIL;
	}

    if(eAccessMode == ecmWrite)
    {
        ucValue = (UINT16)sCmdFormat->lData;

        if(eEXEC_CODE_PASS == utilCommonCLI_RangeCheck(eDataCode, (INT32)ucValue))
        {
            if(eEXEC_CODE_PASS != utilCommonCLI_DataConversionSet(eDataCode, ucValue))
            {
                return eCLI_ERROR_CODE_SETFAIL;
            }
        }
        else
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }
    }
    else //Read
    {
        if(eEXEC_CODE_PASS != utilCommonCLI_DataConversionGet(eDataCode, (INT32*)&ucValue))
        {
            return eCLI_ERROR_CODE_REQUESTFAIL;
        }
		sCmdFormat->lData = (INT32) ucValue;
    }

    return eCLI_ERROR_CODE_NO;
}

// ==============================================================================
// FUNCTION NAME: m_sSubLut_Christie
// DESCRIPTION:
//
//
// Params:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/11/08, Larry Create
// --------------------
// ==============================================================================
sCLI_SUB_LUT m_sSubLut_Christie[] =
{
    //uiMainCmdID       ucSubCmd    ucCmdIsRead       ucCmdBytesOnRange   iFunCodeID;             uiSpecialFlag
    //SYS============================================================================================================//SYS
    {eCLI_MAIN_SYS,     NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,   utilChristie_CLI_SYS,       CLI_COM_READ_ONLY},
    {eCLI_MAIN_SYS,     NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,                  NULL,        CLI_COM_READ_ONLY},
    {eCLI_MAIN_SYS,     "DBMK",     READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,   utilChristie_CLI_SYSDBMK,   CLI_COM_NO_REPLY},
    {eCLI_MAIN_SYS,     "DBMK",     WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,   utilChristie_CLI_SYSDBMK,   CLI_COM_NO_REPLY},
    {eCLI_MAIN_SYS,     "ELOG",     READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,   utilChristie_CLI_SYSELOG,   CLI_COM_READ_ONLY | CLI_COM_NO_REPLY},    //A70LV_John_0008 add basic function and flow of GEC
    {eCLI_MAIN_SYS,     "C821",     READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,   utilChristie_CLI_SYS_C821,  CLI_COM_READ_ONLY | CLI_COM_NO_REPLY},     //A70LV_Doulas_0165 Add
    {eCLI_MAIN_SYS,     "C789",     READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,   utilChristie_CLI_SYS_C789,  CLI_COM_READ_ONLY | CLI_COM_NO_REPLY},     //A70LV_Doulas_0170 debug
    {eCLI_MAIN_SYS,     "C789",     WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,   utilChristie_CLI_SYS_C789,  CLI_COM_NO_REPLY},     //A70LV_Doulas_0170 debug
    {eCLI_MAIN_SYS,     "RSXF",     WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,   utilChristie_CLI_SYS_RSXF,  CLI_COM_WRITE_ONLY},     //A70LV_Doulas_0184 debug
    {eCLI_MAIN_SYS,     "TWIS",     WRITE_COMMAND,    DECIAML_2,          edcTWIST_LINK,                     NULL,  CLI_COM_WRITE_ONLY}, //A70LV_Larry_0218
    {eCLI_MAIN_SYS,     "TWIT",     WRITE_COMMAND,    DECIAML_2,          edcTWIST_PATTERN,                  NULL,  CLI_COM_WRITE_ONLY}, //A70LV_Larry_0218
    {eCLI_MAIN_SYS,     "SWIT",     WRITE_COMMAND,    DECIAML_2,          edcUART_SWITCH,                    NULL,  CLI_COM_WRITE_ONLY},
    {eCLI_MAIN_SYS,     "SMDC",     READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,   utilChristie_CLI_SYS_SMDC,  CLI_COM_READ_ONLY | CLI_COM_NO_REPLY},
    {eCLI_MAIN_SYS,     "SMIF",     READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,   utilChristie_CLI_SYS_SMIF,  CLI_COM_READ_ONLY | CLI_COM_NO_REPLY},
 	{eCLI_MAIN_SYS,     "EMER",     WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,   utilChristie_CLI_SYS_EmergencyTest,    CLI_COM_NO_REPLY},

    {eCLI_MAIN_SYS,     "RVXF",     READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,   utilChristie_CLI_SYS_RVXF,  CLI_COM_READ_ONLY},  //A70LV_Doulas_0218 debug
    {eCLI_MAIN_SYS,     "DDPR",     WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,   utilChristie_CLI_SYS_DDPR,  CLI_COM_WRITE_ONLY},    //A70LV_Doulas_0236 debug
    {eCLI_MAIN_SYS,     "DAPH",     READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,   utilChristie_CLI_SYS_DAPH,  CLI_COM_READ_ONLY},     //A70LV_Doulas_0236 debug

    {eCLI_MAIN_SYS,     "LVDS",     READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,   utilChristie_CLI_SYS_LVDS,  CLI_COM_NORMAL},        //A70LV_Doulas_0262
    {eCLI_MAIN_SYS,     "LVDS",     WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,   utilChristie_CLI_SYS_LVDS,  CLI_COM_NORMAL},        //A70LV_Doulas_0262

    {eCLI_MAIN_SYS,     "LDON",     READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,   utilChristie_CLI_SYS_LDON,  CLI_COM_READ_ONLY},        //A70LV_Doulas_0306 Add

    {eCLI_MAIN_SYS,     "CMOS",     WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,   utilChristie_CLI_SYS_CMOS,  CLI_COM_WRITE_ONLY},    //A70LV_Doulas_0361

    {eCLI_MAIN_SYS,     "LANS",     WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,   utilChristie_CLI_SYS_LANS,  CLI_COM_WRITE_ONLY},    //HICC2_Doulas_0057

    {eCLI_MAIN_SYS,     "SSRH",     READ_COMMAND,     DECIAML_2,          edcBAROMETRY_VALUE,                  NULL,  CLI_COM_READ_ONLY},     //HICC2_Doulas_0064 Humidity

    {eCLI_MAIN_SYS,     "FPGA",     READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,  utilChristie_CLI_SYS_FPGA,  CLI_COM_NO_REPLY},
    {eCLI_MAIN_SYS,     "FPGA",     WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,  utilChristie_CLI_SYS_FPGA,  CLI_COM_NO_REPLY},

    //{eCLI_MAIN_SYS,     "NPWR",     READ_COMMAND,     DECIAML_2,          edcNO_POWER_OFF,                   NULL,  CLI_COM_NORMAL},        //A70LV_Doulas_0374
    //{eCLI_MAIN_SYS,     "NPWR",     WRITE_COMMAND,    DECIAML_2,          edcNO_POWER_OFF,                   NULL,  CLI_COM_NORMAL},        //A70LV_Doulas_0374

    //SZP============================================================================================================//SZP
    {eCLI_MAIN_SZP,     NULL,       READ_COMMAND,     DECIAML_2,          edcSIZE_PRESETS,            NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_SZP,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcSIZE_PRESETS,            NULL,           CLI_COM_NORMAL},

    //HOR============================================================================================================//HOR
    {eCLI_MAIN_HOR,     NULL,       READ_COMMAND,     DECIAML_3,          edcHORZ_POSITION,            NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_HOR,     NULL,       WRITE_COMMAND,    DECIAML_3,          edcHORZ_POSITION,            NULL,           CLI_COM_NORMAL},

    //VRT============================================================================================================//VRT
    {eCLI_MAIN_VRT,     NULL,       READ_COMMAND,     DECIAML_3,          edcVERT_POSITION,            NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_VRT,     NULL,       WRITE_COMMAND,    DECIAML_3,          edcVERT_POSITION,            NULL,           CLI_COM_NORMAL},

    //DZH============================================================================================================//DZH
    {eCLI_MAIN_DZH,     NULL,       READ_COMMAND,     DECIAML_3,          edcDIGITAL_HORZ_ZOOM,        NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_DZH,     NULL,       WRITE_COMMAND,    DECIAML_3,          edcDIGITAL_HORZ_ZOOM,        NULL,           CLI_COM_NORMAL},

    //DZV============================================================================================================//DZV
    {eCLI_MAIN_DZV,     NULL,       READ_COMMAND,     DECIAML_3,          edcDIGITAL_VERT_ZOOM,        NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_DZV,     NULL,       WRITE_COMMAND,    DECIAML_3,          edcDIGITAL_VERT_ZOOM,        NULL,           CLI_COM_NORMAL},

    //DSH============================================================================================================//DSH
    {eCLI_MAIN_DSH,     NULL,       READ_COMMAND,     DECIAML_3,          edcDIGITAL_HORZ_SHIFT,       NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_DSH,     NULL,       WRITE_COMMAND,    DECIAML_3,          edcDIGITAL_HORZ_SHIFT,       NULL,           CLI_COM_NORMAL},

    //DSV============================================================================================================//DSV
    {eCLI_MAIN_DSV,     NULL,       READ_COMMAND,     DECIAML_3,          edcDIGITAL_VERT_SHIFT,       NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_DSV,     NULL,       WRITE_COMMAND,    DECIAML_3,          edcDIGITAL_VERT_SHIFT,       NULL,           CLI_COM_NORMAL},

    //GME============================================================================================================//GME
    {eCLI_MAIN_GME,     NULL,       READ_COMMAND,     DECIAML_3,          edcWARP_TOGGLE,              NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_GME,     NULL,       WRITE_COMMAND,    DECIAML_3,          edcWARP_TOGGLE,              NULL,           CLI_COM_NORMAL},

    //WRP============================================================================================================//WRP
    {eCLI_MAIN_WRP,     "HKST",     READ_COMMAND,     DECIAML_2,          edcHORIZONTAL_KEYSTONE,      NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_WRP,     "HKST",     WRITE_COMMAND,    DECIAML_2,          edcHORIZONTAL_KEYSTONE,      NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_WRP,     "VKST",     READ_COMMAND,     DECIAML_2,          edcVERTICAL_KEYSTONE,        NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_WRP,     "VKST",     WRITE_COMMAND,    DECIAML_2,          edcVERTICAL_KEYSTONE,        NULL,           CLI_COM_NORMAL},
    //{eCLI_MAIN_WRP,     "MWWM",     READ_COMMAND,     DECIAML_2,          edcWARP_MODE,                NULL,           CLI_COM_NORMAL}, //A70LV_Larry_0257 mask
    //{eCLI_MAIN_WRP,     "MWWM",     WRITE_COMMAND,    DECIAML_2,          edcWARP_MODE,                NULL,           CLI_COM_NORMAL}, //A70LV_Larry_0257 mask
    //{eCLI_MAIN_WRP,     "MWWP",     READ_COMMAND,     DECIAML_2,          edcWARP_MOVE_PITCH_MODE,     NULL,           CLI_COM_NORMAL}, //A70LV_Larry_0257 mask
    //{eCLI_MAIN_WRP,     "MWWP",     WRITE_COMMAND,    DECIAML_2,          edcWARP_MOVE_PITCH_MODE,     NULL,           CLI_COM_NORMAL}, //A70LV_Larry_0257 mask
    //{eCLI_MAIN_WRP,     "MWCA",     READ_COMMAND,     DECIAML_2,          edcWARP_ADJUSTMENT,          NULL,           CLI_COM_NORMAL}, //A70LV_Larry_0257 mask
    //{eCLI_MAIN_WRP,     "MWCA",     WRITE_COMMAND,    DECIAML_2,          edcWARP_ADJUSTMENT,          NULL,           CLI_COM_NORMAL}, //A70LV_Larry_0257 mask
    {eCLI_MAIN_WRP,     "DBGG",     READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,                utilChristie_CLI_WRP_DBGG,  CLI_COM_NORMAL| CLI_COM_NO_REPLY},
    {eCLI_MAIN_WRP,     "DBGG",     WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,                utilChristie_CLI_WRP_DBGG,  CLI_COM_NORMAL| CLI_COM_NO_REPLY},

    {eCLI_MAIN_WRP,     "CTRL",     READ_COMMAND,     DECIAML_2,          edcADV_WARP_CONTROL,         NULL,           CLI_COM_NORMAL},		//G100_Doulas_0027
    {eCLI_MAIN_WRP,     "CTRL",     WRITE_COMMAND,    DECIAML_2,          edcADV_WARP_CONTROL,         NULL,           CLI_COM_NORMAL},

    {eCLI_MAIN_WRP,     "GPTS",     READ_COMMAND,     DECIAML_2,          edcADV_WARP_GRID_POINTS,     NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_WRP,     "GPTS",     WRITE_COMMAND,    DECIAML_2,          edcADV_WARP_GRID_POINTS,     NULL,           CLI_COM_NORMAL},

    {eCLI_MAIN_WRP,     "INNR",     READ_COMMAND,     DECIAML_2,          edcADV_WARP_INNER,           NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_WRP,     "INNR",     WRITE_COMMAND,    DECIAML_2,          edcADV_WARP_INNER,           NULL,           CLI_COM_NORMAL},

    {eCLI_MAIN_WRP,     "SHRP",     READ_COMMAND,     DECIAML_2,          edcADV_WARP_SHARPNESS,       NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_WRP,     "SHRP",     WRITE_COMMAND,    DECIAML_2,          edcADV_WARP_SHARPNESS,       NULL,           CLI_COM_NORMAL},

    {eCLI_MAIN_WRP,     "GCLR",     READ_COMMAND,     DECIAML_2,          edcADV_WARP_GRID_COLOR,      NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_WRP,     "GCLR",     WRITE_COMMAND,    DECIAML_2,          edcADV_WARP_GRID_COLOR,      NULL,           CLI_COM_NORMAL},

    {eCLI_MAIN_WRP,     "BCLR",     READ_COMMAND,     DECIAML_2,          edcADV_WARP_GRID_BACKGROUND, NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_WRP,     "BCLR",     WRITE_COMMAND,    DECIAML_2,          edcADV_WARP_GRID_BACKGROUND, NULL,           CLI_COM_NORMAL},

    //HPC============================================================================================================//HPC
    {eCLI_MAIN_HPC,     NULL,       READ_COMMAND,     DECIAML_3,          edcHORZ_PINCUSHION,          NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_HPC,     NULL,       WRITE_COMMAND,    DECIAML_3,          edcHORZ_PINCUSHION,          NULL,           CLI_COM_NORMAL},

    //VPC============================================================================================================//VPC
    {eCLI_MAIN_VPC,     NULL,       READ_COMMAND,     DECIAML_3,          edcVERT_PINCUSHION,          NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_VPC,     NULL,       WRITE_COMMAND,    DECIAML_3,          edcVERT_PINCUSHION,          NULL,           CLI_COM_NORMAL},

    //CNR============================================================================================================//CNR
    {eCLI_MAIN_CNR,     "TLCX",     READ_COMMAND,     DECIAML_3,          edc4CORNER_TOP_LEFT_HORZ,     NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_CNR,     "TLCX",     WRITE_COMMAND,    DECIAML_3,          edc4CORNER_TOP_LEFT_HORZ,     NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_CNR,     "TLCY",     READ_COMMAND,     DECIAML_3,          edc4CORNER_TOP_LEFT_VERT,     NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_CNR,     "TLCY",     WRITE_COMMAND,    DECIAML_3,          edc4CORNER_TOP_LEFT_VERT,     NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_CNR,     "TRCX",     READ_COMMAND,     DECIAML_3,          edc4CORNER_TOP_RIGHT_HORZ,    NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_CNR,     "TRCX",     WRITE_COMMAND,    DECIAML_3,          edc4CORNER_TOP_RIGHT_HORZ,    NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_CNR,     "TRCY",     READ_COMMAND,     DECIAML_3,          edc4CORNER_TOP_RIGHT_VERT,    NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_CNR,     "TRCY",     WRITE_COMMAND,    DECIAML_3,          edc4CORNER_TOP_RIGHT_VERT,    NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_CNR,     "BLCX",     READ_COMMAND,     DECIAML_3,          edc4CORNER_BOTTOM_LEFT_HORZ,  NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_CNR,     "BLCX",     WRITE_COMMAND,    DECIAML_3,          edc4CORNER_BOTTOM_LEFT_HORZ,  NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_CNR,     "BLCY",     READ_COMMAND,     DECIAML_3,          edc4CORNER_BOTTOM_LEFT_VERT,  NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_CNR,     "BLCY",     WRITE_COMMAND,    DECIAML_3,          edc4CORNER_BOTTOM_LEFT_VERT,  NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_CNR,     "BRCX",     READ_COMMAND,     DECIAML_3,          edc4CORNER_BOTTOM_RIGHT_HORZ, NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_CNR,     "BRCX",     WRITE_COMMAND,    DECIAML_3,          edc4CORNER_BOTTOM_RIGHT_HORZ, NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_CNR,     "BRCY",     READ_COMMAND,     DECIAML_3,          edc4CORNER_BOTTOM_RIGHT_VERT, NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_CNR,     "BRCY",     WRITE_COMMAND,    DECIAML_3,          edc4CORNER_BOTTOM_RIGHT_VERT, NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_CNR,     "RSET",     READ_COMMAND,     DECIAML_2,          edc4CORNER_RESET,             NULL,          CLI_COM_WRITE_ONLY},
    {eCLI_MAIN_CNR,     "RSET",     WRITE_COMMAND,    DECIAML_2,          edc4CORNER_RESET,             NULL,          CLI_COM_WRITE_ONLY},

    //AWF============================================================================================================//AWF
    {eCLI_MAIN_AWF,     NULL,       READ_COMMAND,     DECIAML_2,          edcAUTO_WARP_FILTER,         NULL,           CLI_COM_NORMAL_AND_PN},
    {eCLI_MAIN_AWF,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcAUTO_WARP_FILTER,         NULL,           CLI_COM_NORMAL_AND_PN},

    //MWF============================================================================================================//MWF
    {eCLI_MAIN_MWF,     "HORZ",     READ_COMMAND,     DECIAML_2,          edcMANUAL_WARP_HORZ_FILTER,  NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_MWF,     "HORZ",     WRITE_COMMAND,    DECIAML_2,          edcMANUAL_WARP_HORZ_FILTER,  NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_MWF,     "VERT",     READ_COMMAND,     DECIAML_2,          edcMANUAL_WARP_VERT_FILTER,  NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_MWF,     "VERT",     WRITE_COMMAND,    DECIAML_2,          edcMANUAL_WARP_VERT_FILTER,  NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_MWF,     NULL,       READ_COMMAND,     DECIAML_2,          edcMANUAL_WARP_HORZ_FILTER,  NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_MWF,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcMANUAL_WARP_HORZ_FILTER,  NULL,           CLI_COM_NORMAL},

    //WRE============================================================================================================//WRE
    {eCLI_MAIN_WRE,     NULL,       READ_COMMAND,     DECIAML_2,          edcWARP_RESET,               NULL,           CLI_COM_WRITE_ONLY},
    {eCLI_MAIN_WRE,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcWARP_RESET,               NULL,           CLI_COM_WRITE_ONLY},

    //AIG============================================================================================================//AIG
    {eCLI_MAIN_AIG,     NULL,       READ_COMMAND,     DECIAML_2,          edcAUTO_IMAGE,               NULL,           CLI_COM_NORMAL_AND_PN},
    {eCLI_MAIN_AIG,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcAUTO_IMAGE,               NULL,           CLI_COM_NORMAL_AND_PN},

    //BRT============================================================================================================//BRT
    {eCLI_MAIN_BRT,     NULL,       READ_COMMAND,     DECIAML_3,          edcBRIGHTNESS,               NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_BRT,     NULL,       WRITE_COMMAND,    DECIAML_3,          edcBRIGHTNESS,               NULL,           CLI_COM_NORMAL},

    //CON============================================================================================================//CON
    {eCLI_MAIN_CON,     NULL,       READ_COMMAND,     DECIAML_3,          edcCONTRAST,                 NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_CON,     NULL,       WRITE_COMMAND,    DECIAML_3,          edcCONTRAST,                 NULL,           CLI_COM_NORMAL},

    //CSP============================================================================================================//CSP
    {eCLI_MAIN_CSP,     NULL,       READ_COMMAND,     DECIAML_2,          edcCOLOR_SPACE,              NULL,           CLI_COM_NORMAL_AND_PN},
    {eCLI_MAIN_CSP,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcCOLOR_SPACE,              NULL,           CLI_COM_NORMAL_AND_PN},

    //DTL============================================================================================================//DTL
    {eCLI_MAIN_DTL,     NULL,       READ_COMMAND,     DECIAML_2,          edcDETAIL,                   NULL,           CLI_COM_NORMAL_AND_PN},
    {eCLI_MAIN_DTL,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcDETAIL,                   NULL,           CLI_COM_NORMAL_AND_PN},

	//TMD============================================================================================================//TMD
    {eCLI_MAIN_TDE,     NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,        utilChristie_CLI_TDE,           CLI_COM_NORMAL_AND_PN}, //A70LV_Doulas_0228 modify
    {eCLI_MAIN_TDE,     NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,        utilChristie_CLI_TDE,           CLI_COM_NORMAL_AND_PN}, //A70LV_Doulas_0228 modify

#if defined(PLATFORM_H60_4K)
    //TDE============================================================================================================//TDE
    {eCLI_MAIN_TDE,     NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,        utilChristie_CLI_TDE,           CLI_COM_NORMAL_AND_PN}, //A70LV_Doulas_0228 modify
    {eCLI_MAIN_TDE,     NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,        utilChristie_CLI_TDE,           CLI_COM_NORMAL_AND_PN}, //A70LV_Doulas_0228 modify
#else
    //TDE============================================================================================================//TDE
    {eCLI_MAIN_TDE,     NULL,       READ_COMMAND,     DECIAML_2,          edc3D_ENABLE,                NULL,           CLI_COM_NORMAL_AND_PN}, //A70LV_Doulas_0228 modify
    {eCLI_MAIN_TDE,     NULL,       WRITE_COMMAND,    DECIAML_2,          edc3D_ENABLE,                NULL,           CLI_COM_NORMAL_AND_PN}, //A70LV_Doulas_0228 modify
#endif

    //TDI============================================================================================================//TDI
    {eCLI_MAIN_TDI,     NULL,       READ_COMMAND,     DECIAML_2,          edc3D_INVERT,                NULL,           CLI_COM_NORMAL_AND_PN},
    {eCLI_MAIN_TDI,     NULL,       WRITE_COMMAND,    DECIAML_2,          edc3D_INVERT,                NULL,           CLI_COM_NORMAL_AND_PN},

    //SIS============================================================================================================//SOS
    {eCLI_MAIN_SIS,     NULL,       READ_COMMAND,     DECIAML_2,          edc3D_SYNC_TYPE,              NULL,           CLI_COM_NORMAL_AND_PN},
    {eCLI_MAIN_SIS,     NULL,       WRITE_COMMAND,    DECIAML_2,          edc3D_SYNC_TYPE,              NULL,           CLI_COM_NORMAL_AND_PN},

    //SOS============================================================================================================//SOS
    {eCLI_MAIN_SOS,     NULL,       READ_COMMAND,     DECIAML_2,          edc3D_SYNC_OUT,              NULL,           CLI_COM_NORMAL_AND_PN},
    {eCLI_MAIN_SOS,     NULL,       WRITE_COMMAND,    DECIAML_2,          edc3D_SYNC_OUT,              NULL,           CLI_COM_NORMAL_AND_PN},

    //FDY============================================================================================================//FDY
    {eCLI_MAIN_FDY,     NULL,       READ_COMMAND,     DECIAML_2,          edcFRAME_DELAY,              NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_FDY,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcFRAME_DELAY,              NULL,           CLI_COM_NORMAL},

    //SDY============================================================================================================//FDY
    {eCLI_MAIN_SDY,     NULL,       READ_COMMAND,     DECIAML_2,          edc3D_SYNC_DELAY,              NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_SDY,     NULL,       WRITE_COMMAND,    DECIAML_2,          edc3D_SYNC_DELAY,              NULL,           CLI_COM_NORMAL},

    //ROG============================================================================================================//ROG
    {eCLI_MAIN_ROG,     NULL,       READ_COMMAND,     DECIAML_3,          edcRED_GAIN,                 NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_ROG,     NULL,       WRITE_COMMAND,    DECIAML_3,          edcRED_GAIN,                 NULL,           CLI_COM_NORMAL},

    //GOG============================================================================================================//GOG
    {eCLI_MAIN_GOG,     NULL,       READ_COMMAND,     DECIAML_3,          edcGREEN_GAIN,               NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_GOG,     NULL,       WRITE_COMMAND,    DECIAML_3,          edcGREEN_GAIN,               NULL,           CLI_COM_NORMAL},

    //BOG============================================================================================================//BOG
    {eCLI_MAIN_BOG,     NULL,       READ_COMMAND,     DECIAML_3,          edcBLUE_GAIN,                NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_BOG,     NULL,       WRITE_COMMAND,    DECIAML_3,          edcBLUE_GAIN,                NULL,           CLI_COM_NORMAL},

    //ROO============================================================================================================//ROO
    {eCLI_MAIN_ROO,     NULL,       READ_COMMAND,     DECIAML_3,          edcRED_OFFSET,               NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_ROO,     NULL,       WRITE_COMMAND,    DECIAML_3,          edcRED_OFFSET,               NULL,           CLI_COM_NORMAL},

    //GOO============================================================================================================//GOO
    {eCLI_MAIN_GOO,     NULL,       READ_COMMAND,     DECIAML_3,          edcGREEN_OFFSET,             NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_GOO,     NULL,       WRITE_COMMAND,    DECIAML_3,          edcGREEN_OFFSET,             NULL,           CLI_COM_NORMAL},

    //BOO============================================================================================================//BOO
    {eCLI_MAIN_BOO,     NULL,       READ_COMMAND,     DECIAML_3,          edcBLUE_OFFSET,              NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_BOO,     NULL,       WRITE_COMMAND,    DECIAML_3,          edcBLUE_OFFSET,              NULL,           CLI_COM_NORMAL},

    //GOR============================================================================================================//GOR
    {eCLI_MAIN_GOR,     NULL,       READ_COMMAND,     DECIAML_2,          edcRESET_RGB_GAIN_OFFSET,    NULL,           CLI_COM_WRITE_ONLY},     //A70LV_Doulas_0227 Modify
    {eCLI_MAIN_GOR,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcRESET_RGB_GAIN_OFFSET,    NULL,           CLI_COM_WRITE_ONLY},

    //SYT============================================================================================================//SYT
    {eCLI_MAIN_SYT,     NULL,       READ_COMMAND,     DECIAML_3,          edcSYNC_THRESHOLD,           NULL,           CLI_COM_NORMAL},     //A70LV_Doulas_0132 modify
    {eCLI_MAIN_SYT,     NULL,       WRITE_COMMAND,    DECIAML_3,          edcSYNC_THRESHOLD,           NULL,           CLI_COM_NORMAL},     //A70LV_Doulas_0132 modify

    //PST============================================================================================================//PST
    {eCLI_MAIN_PST,     NULL,       READ_COMMAND,     DECIAML_2,          edcPICTURE_SETTINGS,         NULL,  CLI_COM_NORMAL_AND_PN},
    {eCLI_MAIN_PST,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcPICTURE_SETTINGS,         NULL,  CLI_COM_NORMAL_AND_PN},
    {eCLI_MAIN_PST,     "USER",     READ_COMMAND,     DECIAML_2,          edcSAVE_TO_USER,             NULL,           CLI_COM_WRITE_ONLY},
    {eCLI_MAIN_PST,     "USER",     WRITE_COMMAND,    DECIAML_2,          edcSAVE_TO_USER,             NULL,           CLI_COM_WRITE_ONLY},

    //PSU============================================================================================================//PST
    {eCLI_MAIN_PSU,     NULL,       READ_COMMAND,     DECIAML_2,          edcUSER_COLOR_MODE,         NULL,           CLI_COM_NORMAL_AND_PN},
    {eCLI_MAIN_PSU,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcUSER_COLOR_MODE,         NULL,           CLI_COM_NORMAL_AND_PN},

    //FPS============================================================================================================//FPS //A70LV_Larry_0450
    {eCLI_MAIN_FPS,     NULL,       READ_COMMAND,     DECIAML_2,          edcFORCE_PICTURE_SETTINGS,   NULL,           CLI_COM_NORMAL_AND_PN},
    {eCLI_MAIN_FPS,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcFORCE_PICTURE_SETTINGS,   NULL,           CLI_COM_NORMAL_AND_PN},

    //DIM============================================================================================================//DIM
    {eCLI_MAIN_DIM,     NULL,       READ_COMMAND,     DECIAML_2,          edcCONTRAST_ENHANCEMENT,     NULL,           CLI_COM_NORMAL_AND_PN},
    {eCLI_MAIN_DIM,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcCONTRAST_ENHANCEMENT,     NULL,           CLI_COM_NORMAL_AND_PN},

    //FRZ============================================================================================================//FRZ
    {eCLI_MAIN_FRZ,     NULL,       READ_COMMAND,     DECIAML_2,          edcIMAGE_FREEZE,             NULL,           CLI_COM_NORMAL_AND_PN},
    {eCLI_MAIN_FRZ,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcIMAGE_FREEZE,             NULL,           CLI_COM_NORMAL_AND_PN},

    //BGC============================================================================================================//BGC
    {eCLI_MAIN_BGC,     NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,                utilChristie_GAMM,	CLI_COM_NORMAL_AND_PN}, //A70Gen2_Julie_0031 //HICC2_Julie_0039
    {eCLI_MAIN_BGC,     NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,                utilChristie_GAMM,   CLI_COM_NORMAL_AND_PN}, //A70Gen2_Steven_0002//HICC2_Julie_0039

    //WPK============================================================================================================//WPK
    {eCLI_MAIN_WPK,     NULL,       READ_COMMAND,     DECIAML_3,          edcWHITE_PEAKING,            NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_WPK,     NULL,       WRITE_COMMAND,    DECIAML_3,          edcWHITE_PEAKING,            NULL,           CLI_COM_NORMAL},

    //CCI============================================================================================================//CCI
    {eCLI_MAIN_CCI,     NULL,       READ_COMMAND,     DECIAML_2,          edcCOLOR_TEMPERATURE,        NULL,           CLI_COM_NORMAL_AND_PN},
    {eCLI_MAIN_CCI,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcCOLOR_TEMPERATURE,        NULL,           CLI_COM_NORMAL_AND_PN},

    //CWS============================================================================================================//CWS
    {eCLI_MAIN_CWS,     NULL,       READ_COMMAND,     DECIAML_2,          edcCOLOR_WHEEL_SPEED,        NULL,           CLI_COM_NORMAL_AND_PN},
    {eCLI_MAIN_CWS,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcCOLOR_WHEEL_SPEED,        NULL,           CLI_COM_NORMAL_AND_PN},

    //LOC============================================================================================================//LOC
    {eCLI_MAIN_LOC,     "LANG",     READ_COMMAND,     DECIAML_2,          edcLANGUAGE,                 NULL,           CLI_COM_NORMAL_AND_PN},
    {eCLI_MAIN_LOC,     "LANG",     WRITE_COMMAND,    DECIAML_2,          edcLANGUAGE,                 NULL,           CLI_COM_NORMAL_AND_PN},

    //FCS============================================================================================================//FCS
    {eCLI_MAIN_FCS,     NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,        utilChristie_FCS,          CLI_COM_LENS},
    {eCLI_MAIN_FCS,     NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,        utilChristie_FCS,          CLI_COM_LENS},

     //ZOM============================================================================================================//ZOM
    {eCLI_MAIN_ZOM,     NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,        utilChristie_ZOM,          CLI_COM_LENS},
    {eCLI_MAIN_ZOM,     NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,        utilChristie_ZOM,          CLI_COM_LENS},

     //LVO============================================================================================================//LVO
    {eCLI_MAIN_LVO,     NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,        utilChristie_LVO,          CLI_COM_LENS},
    {eCLI_MAIN_LVO,     NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,        utilChristie_LVO,          CLI_COM_LENS},

     //LHO============================================================================================================//LHO
    {eCLI_MAIN_LHO,     NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,        utilChristie_LHO,          CLI_COM_LENS},
    {eCLI_MAIN_LHO,     NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,        utilChristie_LHO,          CLI_COM_LENS},

     //LVU============================================================================================================//LVU
    {eCLI_MAIN_LVU,     NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,        utilChristie_LVU,          CLI_COM_NORMAL | CLI_COM_WRITE_ONLY},
    {eCLI_MAIN_LVU,     NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,        utilChristie_LVU,          CLI_COM_NORMAL | CLI_COM_WRITE_ONLY},

     //LVD============================================================================================================//LVD
    {eCLI_MAIN_LVD,     NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,        utilChristie_LVD,          CLI_COM_NORMAL | CLI_COM_WRITE_ONLY},
    {eCLI_MAIN_LVD,     NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,        utilChristie_LVD,          CLI_COM_NORMAL | CLI_COM_WRITE_ONLY},

     //LHL============================================================================================================//LHL
    {eCLI_MAIN_LHL,     NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,        utilChristie_LHL,          CLI_COM_NORMAL | CLI_COM_WRITE_ONLY},
    {eCLI_MAIN_LHL,     NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,        utilChristie_LHL,          CLI_COM_NORMAL | CLI_COM_WRITE_ONLY},

     //LHR============================================================================================================//LHR
    {eCLI_MAIN_LHR,     NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,        utilChristie_LHR,          CLI_COM_NORMAL | CLI_COM_WRITE_ONLY},
    {eCLI_MAIN_LHR,     NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,        utilChristie_LHR,          CLI_COM_NORMAL | CLI_COM_WRITE_ONLY},

     //LMA============================================================================================================//LMA
    {eCLI_MAIN_LMA,     NULL,       READ_COMMAND,     DECIAML_2,          edcLENS_APPLY_POSITION,        NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_LMA,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcLENS_APPLY_POSITION,        NULL,          CLI_COM_NORMAL},

     //LMS============================================================================================================//LMS
    {eCLI_MAIN_LMS,     NULL,       READ_COMMAND,     DECIAML_2,          edcLENS_SAVE_CURRENT_POSITION, NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_LMS,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcLENS_SAVE_CURRENT_POSITION, NULL,          CLI_COM_NORMAL},

     //LCB============================================================================================================//LCB
    {eCLI_MAIN_LCB,     "LOCK",     READ_COMMAND,     DECIAML_2,          edcLOCK_ALL_LENS_MOTORS,      NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_LCB,     "LOCK",     WRITE_COMMAND,    DECIAML_2,          edcLOCK_ALL_LENS_MOTORS,      NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_LCB,     "HOME",     READ_COMMAND,     DECIAML_2,          edcLENS_CALIBRATION,          NULL,           CLI_COM_WRITE_ONLY},
    {eCLI_MAIN_LCB,     "HOME",     WRITE_COMMAND,    DECIAML_2,          edcLENS_CALIBRATION,          NULL,           CLI_COM_WRITE_ONLY},

     //CEL============================================================================================================//CEL
    {eCLI_MAIN_CEL,     NULL,       READ_COMMAND,     DECIAML_2,          edcCEILING_MOUNT,             NULL,           CLI_COM_NORMAL_AND_PN},
    {eCLI_MAIN_CEL,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcCEILING_MOUNT,             NULL,           CLI_COM_NORMAL_AND_PN},

     //SOR============================================================================================================//SOR
    {eCLI_MAIN_SOR,     NULL,       READ_COMMAND,     DECIAML_2,          edcREAR_PROJECTION,           NULL,           CLI_COM_NORMAL_AND_PN},
    {eCLI_MAIN_SOR,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcREAR_PROJECTION,           NULL,           CLI_COM_NORMAL_AND_PN},

     //MSH============================================================================================================//MSH
    {eCLI_MAIN_MSH,     NULL,       READ_COMMAND,     DECIAML_3,          edcMEMU_HORZ_OFFSET,          NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_MSH,     NULL,       WRITE_COMMAND,    DECIAML_3,          edcMEMU_HORZ_OFFSET,          NULL,           CLI_COM_NORMAL},

    //MSV============================================================================================================//MSV
    {eCLI_MAIN_MSV,     NULL,       READ_COMMAND,     DECIAML_3,          edcMEMU_VERT_OFFSET,          NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_MSV,     NULL,       WRITE_COMMAND,    DECIAML_3,          edcMEMU_VERT_OFFSET,          NULL,          CLI_COM_NORMAL},

    //MBE============================================================================================================//MBE
    {eCLI_MAIN_MBE,     "USER",     READ_COMMAND,     DECIAML_2,          edcSHOW_MESSAGES,             NULL,          CLI_COM_NORMAL_AND_PN },
    {eCLI_MAIN_MBE,     "USER",     WRITE_COMMAND,    DECIAML_2,          edcSHOW_MESSAGES,             NULL,          CLI_COM_NORMAL_AND_PN },

    //OST============================================================================================================//OST
    {eCLI_MAIN_OST,     NULL,       READ_COMMAND,     DECIAML_3,          edcMENU_TRANSPARENCY,         NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_OST,     NULL,       WRITE_COMMAND,    DECIAML_3,          edcMENU_TRANSPARENCY,         NULL,          CLI_COM_NORMAL},

    //SPS============================================================================================================//SPS
    {eCLI_MAIN_SPS,     "SLCT",     READ_COMMAND,     DECIAML_2,          edcSPLASH_STARTUP,            NULL,          CLI_COM_NORMAL_AND_PN},
    {eCLI_MAIN_SPS,     "SLCT",     WRITE_COMMAND,    DECIAML_2,          edcSPLASH_STARTUP,            NULL,          CLI_COM_NORMAL_AND_PN},

    //PIV============================================================================================================//PIV          //A70LV_Doulas_0139
    {eCLI_MAIN_PIV,     NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,           utilChristie_PIV,       CLI_COM_NORMAL },
    {eCLI_MAIN_PIV,     NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,           utilChristie_PIV,       CLI_COM_NORMAL },

    //PCG============================================================================================================//PCG          //A70LV_Doulas_0139
    {eCLI_MAIN_PCG,     NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,           utilChristie_PCG,       CLI_COM_WRITE_ONLY },
    {eCLI_MAIN_PCG,     NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,           utilChristie_PCG,       CLI_COM_WRITE_ONLY },

    //PWR============================================================================================================//PWR
    {eCLI_MAIN_PWR,     NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,       utilChristie_PWR,          CLI_COM_NORMAL},
    {eCLI_MAIN_PWR,     NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,       utilChristie_PWR,          CLI_COM_NORMAL},
    {eCLI_MAIN_PWR,     "STBM",     READ_COMMAND,     DECIAML_2,          edcSTANDBY_MODE,              NULL,            CLI_COM_NORMAL_AND_PN},
    {eCLI_MAIN_PWR,     "STBM",     WRITE_COMMAND,    DECIAML_2,          edcSTANDBY_MODE,              NULL,            CLI_COM_NORMAL_AND_PN},
    {eCLI_MAIN_PWR,     "SLEP",     READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,   utilChristie_PWRSLEP,          CLI_COM_NORMAL},
    {eCLI_MAIN_PWR,     "SLEP",     WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,   utilChristie_PWRSLEP,          CLI_COM_NORMAL},

    //APW============================================================================================================//APW
    {eCLI_MAIN_APW,     NULL,       READ_COMMAND,     DECIAML_2,          edcAC_POWER_ON,               NULL,          CLI_COM_NORMAL_AND_PN},
    {eCLI_MAIN_APW,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcAC_POWER_ON,               NULL,          CLI_COM_NORMAL_AND_PN},

    //ASH============================================================================================================//ASH
    {eCLI_MAIN_ASH,     NULL,       READ_COMMAND,     DECIAML_2,          edcAUTO_SHUTDOWN,             NULL,          CLI_COM_NORMAL_AND_PN},
    {eCLI_MAIN_ASH,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcAUTO_SHUTDOWN,             NULL,          CLI_COM_NORMAL_AND_PN},

    //SLP============================================================================================================//SLP
    {eCLI_MAIN_SLP,     NULL,       READ_COMMAND,     DECIAML_2,          edcSLEEP_TIMER,               NULL,          CLI_COM_NORMAL_AND_PN},
    {eCLI_MAIN_SLP,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcSLEEP_TIMER,               NULL,          CLI_COM_NORMAL_AND_PN},

    //HAT============================================================================================================//HAT
    {eCLI_MAIN_HAT,     NULL,       READ_COMMAND,     DECIAML_2,          edcHIGH_ALTITUDE,             NULL,          CLI_COM_NORMAL_AND_PN},
    {eCLI_MAIN_HAT,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcHIGH_ALTITUDE,             NULL,          CLI_COM_NORMAL_AND_PN},

    //IRC============================================================================================================//IRC
    {eCLI_MAIN_IRC,     "TOPP",     READ_COMMAND,     DECIAML_2,          edcTOP_IR,                    NULL,          CLI_COM_NORMAL_AND_PN},
    {eCLI_MAIN_IRC,     "TOPP",     WRITE_COMMAND,    DECIAML_2,          edcTOP_IR,                    NULL,          CLI_COM_NORMAL_AND_PN},
    {eCLI_MAIN_IRC,     "FRNT",     READ_COMMAND,     DECIAML_2,          edcFRONT_IR,                  NULL,          CLI_COM_NORMAL_AND_PN},
    {eCLI_MAIN_IRC,     "FRNT",     WRITE_COMMAND,    DECIAML_2,          edcFRONT_IR,                  NULL,          CLI_COM_NORMAL_AND_PN},
    {eCLI_MAIN_IRC,     "HDBT",     READ_COMMAND,     DECIAML_2,          edcHDBASET_IR,                NULL,          CLI_COM_NORMAL_AND_PN},
    {eCLI_MAIN_IRC,     "HDBT",     WRITE_COMMAND,    DECIAML_2,          edcHDBASET_IR,                NULL,          CLI_COM_NORMAL_AND_PN},

    //VTT============================================================================================================//VTT
    {eCLI_MAIN_VTT,     NULL,       READ_COMMAND,     DECIAML_2,          edc12V_TRIGGER,               NULL,          CLI_COM_NORMAL_AND_PN},
    {eCLI_MAIN_VTT,     NULL,       WRITE_COMMAND,    DECIAML_2,          edc12V_TRIGGER,               NULL,          CLI_COM_NORMAL_AND_PN},

    //NET============================================================================================================//NET
    {eCLI_MAIN_NET,     "DHCP",     READ_COMMAND,     DECIAML_2,          edcLAN_DHCP,                  NULL,          CLI_COM_NORMAL_AND_PN},
    {eCLI_MAIN_NET,     "DHCP",     WRITE_COMMAND,    DECIAML_2,          edcLAN_DHCP,                  NULL,          CLI_COM_NORMAL_AND_PN},
    {eCLI_MAIN_NET,     "ETH0",     READ_COMMAND,     STRING_CHAR,        edcLAN_IP_ADDRESS,            NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_NET,     "ETH0",     WRITE_COMMAND,    DECIAML_2,          edcLAN_IP_ADDRESS,            NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_NET,     "SUB0",     READ_COMMAND,     STRING_CHAR,        edcLAN_SUBNET_MASK,           NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_NET,     "SUB0",     WRITE_COMMAND,    DECIAML_2,          edcLAN_SUBNET_MASK,           NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_NET,     "GATE",     READ_COMMAND,     STRING_CHAR,        edcLAN_DEFAULT_GATEWAY,       NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_NET,     "GATE",     WRITE_COMMAND,    DECIAML_2,          edcLAN_DEFAULT_GATEWAY,       NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_NET,     "MAC0",     READ_COMMAND,     DECIAML_2,          edcLAN_MAC_ADDRESS,           NULL,          CLI_COM_READ_ONLY},
    {eCLI_MAIN_NET,     "MAC0",     WRITE_COMMAND,    DECIAML_2,          edcLAN_MAC_ADDRESS,           NULL,          CLI_COM_READ_ONLY},
    {eCLI_MAIN_NET,     "HOST",     READ_COMMAND,     DECIAML_2,          edcPROJECTOR_NAME,            NULL,          CLI_COM_READ_ONLY},
    {eCLI_MAIN_NET,     "HOST",     WRITE_COMMAND,    DECIAML_2,          edcPROJECTOR_NAME,            NULL,          CLI_COM_READ_ONLY},
    {eCLI_MAIN_NET,     "SHOW",     READ_COMMAND,     DECIAML_2,          edcSHOW_NETWORK_MESSAGE,      NULL,          CLI_COM_NORMAL_AND_PN},
    {eCLI_MAIN_NET,     "SHOW",     WRITE_COMMAND,    DECIAML_2,          edcSHOW_NETWORK_MESSAGE,      NULL,          CLI_COM_NORMAL_AND_PN},
    {eCLI_MAIN_NET,     "RSET",     READ_COMMAND,     DECIAML_2,          edcNETWORK_FACTORY_RESET,     NULL,          CLI_COM_WRITE_ONLY},
    {eCLI_MAIN_NET,     "RSET",     WRITE_COMMAND,    DECIAML_2,          edcNETWORK_FACTORY_RESET,     NULL,          CLI_COM_WRITE_ONLY},
    {eCLI_MAIN_NET,     "PDNS",     READ_COMMAND,     STRING_CHAR,        edcLAN_PRIMARY_DNS,           NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_NET,     "PDNS",     WRITE_COMMAND,    DECIAML_2,          edcLAN_PRIMARY_DNS,           NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_NET,     "SDNS",     READ_COMMAND,     STRING_CHAR,        edcLAN_SECOND_DNS,            NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_NET,     "SDNS",     WRITE_COMMAND,    DECIAML_2,          edcLAN_SECOND_DNS,            NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_NET,     "SETT",     READ_COMMAND,     DECIAML_2,          edcLAN_APPLY,                 NULL,          CLI_COM_WRITE_ONLY},
    {eCLI_MAIN_NET,     "SETT",     WRITE_COMMAND,    DECIAML_2,          edcLAN_APPLY,                 NULL,          CLI_COM_WRITE_ONLY},
    {eCLI_MAIN_NET,     "STAT",     READ_COMMAND,     DECIAML_2,          edcLAN_STATUS,                NULL,          CLI_COM_READ_ONLY},
    {eCLI_MAIN_NET,     "STAT",     WRITE_COMMAND,    DECIAML_2,          edcLAN_STATUS,                NULL,          CLI_COM_READ_ONLY},
    {eCLI_MAIN_NET,     "PATH",     READ_COMMAND,     DECIAML_2,          edcLAN_PATH_SWITCH,           NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_NET,     "PATH",     WRITE_COMMAND,    DECIAML_2,          edcLAN_PATH_SWITCH,           NULL,          CLI_COM_NORMAL},

    //NTW============================================================================================================//NTW
    {eCLI_MAIN_NTW,     "SLCT",     READ_COMMAND,     DECIAML_2,          edcWLAN_ENABLE,               NULL,          CLI_COM_NORMAL_AND_PN},
    {eCLI_MAIN_NTW,     "SLCT",     WRITE_COMMAND,    DECIAML_2,          edcWLAN_ENABLE,               NULL,          CLI_COM_NORMAL_AND_PN},
    {eCLI_MAIN_NTW,     "ETH0",     READ_COMMAND,     DECIAML_2,          edcWLAN_START_IP,             NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_NTW,     "ETH0",     WRITE_COMMAND,    DECIAML_2,          edcWLAN_START_IP,             NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_NTW,     "ENIP",     READ_COMMAND,     DECIAML_2,          edcWLAN_END_IP,               NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_NTW,     "ENIP",     WRITE_COMMAND,    DECIAML_2,          edcWLAN_END_IP,               NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_NTW,     "SUB0",     READ_COMMAND,     DECIAML_2,          edcWLAN_SUBNET_MASK,          NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_NTW,     "SUB0",     WRITE_COMMAND,    DECIAML_2,          edcWLAN_SUBNET_MASK,          NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_NTW,     "GATE",     READ_COMMAND,     DECIAML_2,          edcWLAN_DEFAULT_GATEWAY,      NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_NTW,     "GATE",     WRITE_COMMAND,    DECIAML_2,          edcWLAN_DEFAULT_GATEWAY,      NULL,          CLI_COM_NORMAL},
    //{eCLI_MAIN_NTW,     "MAC0",     READ_COMMAND,     DECIAML_2,          edcWLAN_MAC_ADDRESS,          NULL,          CLI_COM_NORMAL},
    //{eCLI_MAIN_NTW,     "MAC0",     WRITE_COMMAND,    DECIAML_2,          edcWLAN_MAC_ADDRESS,          NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_NTW,     "SSID",     READ_COMMAND,     DECIAML_2,          edcWLAN_SSID,                 NULL,          CLI_COM_READ_ONLY},
    {eCLI_MAIN_NTW,     "SSID",     WRITE_COMMAND,    DECIAML_2,          edcWLAN_SSID,                 NULL,          CLI_COM_READ_ONLY},
    {eCLI_MAIN_NTW,     "SETT",     READ_COMMAND,     DECIAML_2,          edcWLAN_APPLY,                NULL,          CLI_COM_WRITE_ONLY},
    {eCLI_MAIN_NTW,     "SETT",     WRITE_COMMAND,    DECIAML_2,          edcWLAN_APPLY,                NULL,          CLI_COM_WRITE_ONLY},

    //BDR============================================================================================================//BDR
    {eCLI_MAIN_BDR,     NULL,       READ_COMMAND,     DECIAML_2,          edcSERIAL_PORT_BAUD_RATE,     NULL,          CLI_COM_NORMAL_AND_PN},
    {eCLI_MAIN_BDR,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcSERIAL_PORT_BAUD_RATE,     NULL,          CLI_COM_NORMAL_AND_PN},

    //SEC============================================================================================================//SEC
    {eCLI_MAIN_SEC,     NULL,       READ_COMMAND,     DECIAML_2,          edcSERIAL_PORT_ECHO,          NULL,          CLI_COM_NORMAL_AND_PN},
    {eCLI_MAIN_SEC,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcSERIAL_PORT_ECHO,          NULL,          CLI_COM_NORMAL_AND_PN},

    //SPP============================================================================================================//SPP
    {eCLI_MAIN_SPP,     NULL,       READ_COMMAND,     DECIAML_2,          edcSERIAL_PORT_PATH,          NULL,          CLI_COM_NORMAL_AND_PN},
    {eCLI_MAIN_SPP,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcSERIAL_PORT_PATH,          NULL,          CLI_COM_NORMAL_AND_PN},

    //ADR============================================================================================================//ADR
    {eCLI_MAIN_ADR,     NULL,       READ_COMMAND,     DECIAML_2,          edcPROJECTOR_ADDRESS,         NULL,          CLI_COM_NORMAL_AND_PN},
    {eCLI_MAIN_ADR,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcPROJECTOR_ADDRESS,         NULL,          CLI_COM_NORMAL_AND_PN},

    //KBL============================================================================================================//KBL
    {eCLI_MAIN_KBL,     NULL,       READ_COMMAND,     DECIAML_2,          edcKEYPAD_BACKLIGHT,          NULL,          CLI_COM_NORMAL_AND_PN},
    {eCLI_MAIN_KBL,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcKEYPAD_BACKLIGHT,          NULL,          CLI_COM_NORMAL_AND_PN},

    //SBL============================================================================================================//SBL
    {eCLI_MAIN_SBL,     NULL,       READ_COMMAND,     DECIAML_2,          edcSTATUS_LED,                NULL,          CLI_COM_NORMAL_AND_PN},
    {eCLI_MAIN_SBL,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcSTATUS_LED,                NULL,          CLI_COM_NORMAL_AND_PN},

    //CCA============================================================================================================//CCA
    {eCLI_MAIN_CCA,     "HSGE",     READ_COMMAND,     DECIAML_2,          edcHSG_ENABLE,                NULL,          CLI_COM_NORMAL_AND_PN},
    {eCLI_MAIN_CCA,     "HSGE",     WRITE_COMMAND,    DECIAML_2,          edcHSG_ENABLE,                NULL,          CLI_COM_NORMAL_AND_PN},
    {eCLI_MAIN_CCA,     "MHTP",     READ_COMMAND,     DECIAML_3,          edcHSG_AUTO_TEST_PATTERN,     NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_CCA,     "MHTP",     WRITE_COMMAND,    DECIAML_3,          edcHSG_AUTO_TEST_PATTERN,     NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_CCA,     "REDH",     READ_COMMAND,     DECIAML_3,          edcHSG_RED_HUE,               NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_CCA,     "REDH",     WRITE_COMMAND,    DECIAML_3,          edcHSG_RED_HUE,               NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_CCA,     "REDS",     READ_COMMAND,     DECIAML_3,          edcHSG_RED_SATURATION,        NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_CCA,     "REDS",     WRITE_COMMAND,    DECIAML_3,          edcHSG_RED_SATURATION,        NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_CCA,     "REDG",     READ_COMMAND,     DECIAML_3,          edcHSG_RED_GAIN,              NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_CCA,     "REDG",     WRITE_COMMAND,    DECIAML_3,          edcHSG_RED_GAIN,              NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_CCA,     "GREH",     READ_COMMAND,     DECIAML_3,          edcHSG_GREEN_HUE,             NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_CCA,     "GREH",     WRITE_COMMAND,    DECIAML_3,          edcHSG_GREEN_HUE,             NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_CCA,     "GRES",     READ_COMMAND,     DECIAML_3,          edcHSG_GREEN_SATURATION,      NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_CCA,     "GRES",     WRITE_COMMAND,    DECIAML_3,          edcHSG_GREEN_SATURATION,      NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_CCA,     "GREG",     READ_COMMAND,     DECIAML_3,          edcHSG_GREEN_GAIN,            NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_CCA,     "GREG",     WRITE_COMMAND,    DECIAML_3,          edcHSG_GREEN_GAIN,            NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_CCA,     "BLUH",     READ_COMMAND,     DECIAML_3,          edcHSG_BLUE_HUE,              NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_CCA,     "BLUH",     WRITE_COMMAND,    DECIAML_3,          edcHSG_BLUE_HUE,              NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_CCA,     "BLUS",     READ_COMMAND,     DECIAML_3,          edcHSG_BLUE_SATURATION,       NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_CCA,     "BLUS",     WRITE_COMMAND,    DECIAML_3,          edcHSG_BLUE_SATURATION,       NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_CCA,     "BLUG",     READ_COMMAND,     DECIAML_3,          edcHSG_BLUE_GAIN,             NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_CCA,     "BLUG",     WRITE_COMMAND,    DECIAML_3,          edcHSG_BLUE_GAIN,             NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_CCA,     "CYAH",     READ_COMMAND,     DECIAML_3,          edcHSG_CYAN_HUE,              NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_CCA,     "CYAH",     WRITE_COMMAND,    DECIAML_3,          edcHSG_CYAN_HUE,              NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_CCA,     "CYAS",     READ_COMMAND,     DECIAML_3,          edcHSG_CYAN_SATURATION,       NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_CCA,     "CYAS",     WRITE_COMMAND,    DECIAML_3,          edcHSG_CYAN_SATURATION,       NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_CCA,     "CYAG",     READ_COMMAND,     DECIAML_3,          edcHSG_CYAN_GAIN,             NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_CCA,     "CYAG",     WRITE_COMMAND,    DECIAML_3,          edcHSG_CYAN_GAIN,             NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_CCA,     "MAGH",     READ_COMMAND,     DECIAML_3,          edcHSG_MAGENTA_HUE,           NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_CCA,     "MAGH",     WRITE_COMMAND,    DECIAML_3,          edcHSG_MAGENTA_HUE,           NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_CCA,     "MAGS",     READ_COMMAND,     DECIAML_3,          edcHSG_MAGENTA_SATURATION,    NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_CCA,     "MAGS",     WRITE_COMMAND,    DECIAML_3,          edcHSG_MAGENTA_SATURATION,    NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_CCA,     "MAGG",     READ_COMMAND,     DECIAML_3,          edcHSG_MAGENTA_GAIN,          NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_CCA,     "MAGG",     WRITE_COMMAND,    DECIAML_3,          edcHSG_MAGENTA_GAIN,          NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_CCA,     "YELH",     READ_COMMAND,     DECIAML_3,          edcHSG_YELLOW_HUE,            NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_CCA,     "YELH",     WRITE_COMMAND,    DECIAML_3,          edcHSG_YELLOW_HUE,            NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_CCA,     "YELS",     READ_COMMAND,     DECIAML_3,          edcHSG_YELLOW_SATURATION,     NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_CCA,     "YELS",     WRITE_COMMAND,    DECIAML_3,          edcHSG_YELLOW_SATURATION,     NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_CCA,     "YELG",     READ_COMMAND,     DECIAML_3,          edcHSG_YELLOW_GAIN,           NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_CCA,     "YELG",     WRITE_COMMAND,    DECIAML_3,          edcHSG_YELLOW_GAIN,           NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_CCA,     "WHRG",     READ_COMMAND,     DECIAML_3,          edcHSG_WHITE_GAIN_RED,        NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_CCA,     "WHRG",     WRITE_COMMAND,    DECIAML_3,          edcHSG_WHITE_GAIN_RED,        NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_CCA,     "WHGG",     READ_COMMAND,     DECIAML_3,          edcHSG_WHITE_GAIN_GREEN,      NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_CCA,     "WHGG",     WRITE_COMMAND,    DECIAML_3,          edcHSG_WHITE_GAIN_GREEN,      NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_CCA,     "WHBG",     READ_COMMAND,     DECIAML_3,          edcHSG_WHITE_GAIN_BLUE,       NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_CCA,     "WHBG",     WRITE_COMMAND,    DECIAML_3,          edcHSG_WHITE_GAIN_BLUE,       NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_CCA,     "WALL",     READ_COMMAND,     DECIAML_2,          edcWALL_COLOR,                NULL,          CLI_COM_NORMAL_AND_PN},
    {eCLI_MAIN_CCA,     "WALL",     WRITE_COMMAND,    DECIAML_2,          edcWALL_COLOR,                NULL,          CLI_COM_NORMAL_AND_PN},
    {eCLI_MAIN_CCA,     "WRTD",     READ_COMMAND,     DECIAML_2,      edcHSG_WHITE_GAIN_RESET_DEFAULT,  NULL,          CLI_COM_WRITE_ONLY},  //A70LV_Doulas_0126 modify
    {eCLI_MAIN_CCA,     "WRTD",     WRITE_COMMAND,    DECIAML_2,      edcHSG_WHITE_GAIN_RESET_DEFAULT,  NULL,          CLI_COM_WRITE_ONLY},  //A70LV_Doulas_0126 modify
    {eCLI_MAIN_CCA,     "RRTD",     READ_COMMAND,     DECIAML_2,          edcHSG_RED_RESET_DEFAULT,     NULL,          CLI_COM_WRITE_ONLY},  //A70LV_Doulas_0133
    {eCLI_MAIN_CCA,     "RRTD",     WRITE_COMMAND,    DECIAML_2,          edcHSG_RED_RESET_DEFAULT,     NULL,          CLI_COM_WRITE_ONLY},  //A70LV_Doulas_0133
    {eCLI_MAIN_CCA,     "GRTD",     READ_COMMAND,     DECIAML_2,          edcHSG_GREEN_RESET_DEFAULT,   NULL,          CLI_COM_WRITE_ONLY},  //A70LV_Doulas_0133
    {eCLI_MAIN_CCA,     "GRTD",     WRITE_COMMAND,    DECIAML_2,          edcHSG_GREEN_RESET_DEFAULT,   NULL,          CLI_COM_WRITE_ONLY},  //A70LV_Doulas_0133
    {eCLI_MAIN_CCA,     "BRTD",     READ_COMMAND,     DECIAML_2,          edcHSG_BLUE_RESET_DEFAULT,    NULL,          CLI_COM_WRITE_ONLY},  //A70LV_Doulas_0133
    {eCLI_MAIN_CCA,     "BRTD",     WRITE_COMMAND,    DECIAML_2,          edcHSG_BLUE_RESET_DEFAULT,    NULL,          CLI_COM_WRITE_ONLY},  //A70LV_Doulas_0133
    {eCLI_MAIN_CCA,     "CRTD",     READ_COMMAND,     DECIAML_2,          edcHSG_CYAN_RESET_DEFAULT,    NULL,          CLI_COM_WRITE_ONLY},  //A70LV_Doulas_0133
    {eCLI_MAIN_CCA,     "CRTD",     WRITE_COMMAND,    DECIAML_2,          edcHSG_CYAN_RESET_DEFAULT,    NULL,          CLI_COM_WRITE_ONLY},  //A70LV_Doulas_0133
    {eCLI_MAIN_CCA,     "MRTD",     READ_COMMAND,     DECIAML_2,          edcHSG_MAGENTA_RESET_DEFAULT, NULL,          CLI_COM_WRITE_ONLY},  //A70LV_Doulas_0133
    {eCLI_MAIN_CCA,     "MRTD",     WRITE_COMMAND,    DECIAML_2,          edcHSG_MAGENTA_RESET_DEFAULT, NULL,          CLI_COM_WRITE_ONLY},  //A70LV_Doulas_0133
    {eCLI_MAIN_CCA,     "YRTD",     READ_COMMAND,     DECIAML_2,          edcHSG_YELLOW_RESET_DEFAULT,  NULL,          CLI_COM_WRITE_ONLY},  //A70LV_Doulas_0133
    {eCLI_MAIN_CCA,     "YRTD",     WRITE_COMMAND,    DECIAML_2,          edcHSG_YELLOW_RESET_DEFAULT,  NULL,          CLI_COM_WRITE_ONLY},  //A70LV_Doulas_0133

    //HAR============================================================================================================//HAR
    {eCLI_MAIN_HAR,     NULL,       READ_COMMAND,     DECIAML_2,          edcHSG_RESET_DEFAULT,         NULL,          CLI_COM_WRITE_ONLY},
    {eCLI_MAIN_HAR,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcHSG_RESET_DEFAULT,         NULL,          CLI_COM_WRITE_ONLY},

    //HSG============================================================================================================//HSG
    {eCLI_MAIN_HSG,     NULL,       READ_COMMAND,     DECIAML_2,          edcCOLOR_ENHANCEMENT,         NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_HSG,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcCOLOR_ENHANCEMENT,         NULL,          CLI_COM_NORMAL},

    //HKS============================================================================================================//HKS
    {eCLI_MAIN_HKS,     NULL,       READ_COMMAND,     DECIAML_2,          edcHOT_KEY_SETTINGS,          NULL,          CLI_COM_NORMAL_AND_PN},
    {eCLI_MAIN_HKS,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcHOT_KEY_SETTINGS,          NULL,          CLI_COM_NORMAL_AND_PN},

    //PIF============================================================================================================//PIF
    {eCLI_MAIN_PIF,     "MDLN",     READ_COMMAND,     DECIAML_2,          edcMODEL_NAME,                NULL,          CLI_COM_READ_ONLY | CLI_COM_NEED_SERVICE_CODE_READ},
    //{eCLI_MAIN_PIF,     "MDLN",     WRITE_COMMAND,    DECIAML_2,          edcMODEL_NAME,                NULL,          CLI_COM_READ_ONLY},
    {eCLI_MAIN_PIF,     "SNUM",     READ_COMMAND,     DECIAML_2,          edcSERIAL_NUMBER,             NULL,          CLI_COM_READ_ONLY | CLI_COM_NEED_SERVICE_CODE_READ},
    //{eCLI_MAIN_PIF,     "SNUM",     WRITE_COMMAND,    DECIAML_2,          edcSERIAL_NUMBER,             NULL,          CLI_COM_NORMAL},   //A70LV_Doulas_0125 remove
    {eCLI_MAIN_PIF,     "NERS",     READ_COMMAND,     DECIAML_2,          edcNATIVE_RESOLUTION,         NULL,          CLI_COM_READ_ONLY | CLI_COM_NEED_SERVICE_CODE_READ},
    //{eCLI_MAIN_PIF,     "NERS",     WRITE_COMMAND,    DECIAML_2,          edcNATIVE_RESOLUTION,         NULL,          CLI_COM_READ_ONLY},

    {eCLI_MAIN_PIF,     "FWV1",     READ_COMMAND,     DECIAML_2,          edcFRONTEND_VERSION,          NULL,          CLI_COM_READ_ONLY | CLI_COM_NEED_SERVICE_CODE_READ}, //HICC2_Doulas_0027 front end version
    //{eCLI_MAIN_PIF,     "FWV1",     WRITE_COMMAND,    DECIAML_2,          edcFRONTEND_VERSION,          NULL,          CLI_COM_READ_ONLY},

	{eCLI_MAIN_PIF,     "FWV2",     READ_COMMAND,     DECIAML_2,          edcFIRMWARE,              	NULL,          CLI_COM_READ_ONLY | CLI_COM_NEED_SERVICE_CODE_READ},
    //{eCLI_MAIN_PIF,     "FWV2",     WRITE_COMMAND,    DECIAML_2,          edcSMCU_VERSION,              NULL,          CLI_COM_READ_ONLY},
    {eCLI_MAIN_PIF,     "FWV3",     READ_COMMAND,     DECIAML_2,          edcMOTOR_VERSION,             NULL,          CLI_COM_READ_ONLY | CLI_COM_NEED_SERVICE_CODE_READ},		//A70Gen2_Doulas_00012 Modify
    //{eCLI_MAIN_PIF,     "FWV3",     WRITE_COMMAND,    DECIAML_2,          edcMOTOR_VERSION,             NULL,          CLI_COM_READ_ONLY},
    {eCLI_MAIN_PIF,     "FWV4",     READ_COMMAND,     DECIAML_2,          edcLD_DRIVER_VERSION,         NULL,          CLI_COM_READ_ONLY | CLI_COM_NEED_SERVICE_CODE_READ},		//A70Gen2_Doulas_00012 Modify
    //{eCLI_MAIN_PIF,     "FWV4",     WRITE_COMMAND,    DECIAML_2,          edcLD_DRIVER_VERSION,         NULL,          CLI_COM_READ_ONLY},
    {eCLI_MAIN_PIF,     "FWV5",     READ_COMMAND,     DECIAML_2,          edcKEYPAD_VERSION,            NULL,          CLI_COM_READ_ONLY | CLI_COM_NEED_SERVICE_CODE_READ},
    //{eCLI_MAIN_PIF,     "FWV5",     WRITE_COMMAND,    DECIAML_2,          edcKEYPAD_VERSION,            NULL,          CLI_COM_READ_ONLY},
    {eCLI_MAIN_PIF,     "FWV6",     READ_COMMAND,     DECIAML_2,          edcLAN_VERSION,               NULL,          CLI_COM_READ_ONLY | CLI_COM_NEED_SERVICE_CODE_READ},
    //{eCLI_MAIN_PIF,     "FWV6",     WRITE_COMMAND,    DECIAML_2,          edcLAN_VERSION,               NULL,          CLI_COM_READ_ONLY},
    {eCLI_MAIN_PIF,     "FWV7",     READ_COMMAND,     DECIAML_2,          edcFORMATER_VERSION,          NULL,          CLI_COM_READ_ONLY | CLI_COM_NEED_SERVICE_CODE_READ},
    //{eCLI_MAIN_PIF,     "FWV7",     WRITE_COMMAND,    DECIAML_2,          edcFORMATER_VERSION,          NULL,          CLI_COM_READ_ONLY},
    {eCLI_MAIN_PIF,     "FWV8",     READ_COMMAND,     DECIAML_2,          edcHDBASET_VERSION,           NULL,          CLI_COM_READ_ONLY | CLI_COM_NEED_SERVICE_CODE_READ},
    //{eCLI_MAIN_PIF,     "FWV8",     WRITE_COMMAND,    DECIAML_2,          edcHDBASET_VERSION,           NULL,          CLI_COM_READ_ONLY},
    {eCLI_MAIN_PIF,     "FWV9",     READ_COMMAND,     DECIAML_2,          edcFMCU_VERSION,              NULL,          CLI_COM_READ_ONLY | CLI_COM_NEED_SERVICE_CODE_READ},
    //{eCLI_MAIN_PIF,     "FWV9",     WRITE_COMMAND,    DECIAML_2,          edcFMCU_VERSION,              NULL,          CLI_COM_READ_ONLY},
    {eCLI_MAIN_PIF,     "FWVA",     READ_COMMAND,     DECIAML_2,          edcFPGA1_VERSION,             NULL,          CLI_COM_READ_ONLY | CLI_COM_NEED_SERVICE_CODE_READ},
    //{eCLI_MAIN_PIF,     "FWVA",     WRITE_COMMAND,    DECIAML_2,          edcFPGA1_VERSION,             NULL,          CLI_COM_READ_ONLY},
    {eCLI_MAIN_PIF,     "FWVB",     READ_COMMAND,     DECIAML_2,          edcFPGA3_VERSION,             NULL,          CLI_COM_READ_ONLY | CLI_COM_NEED_SERVICE_CODE_READ},
    //{eCLI_MAIN_PIF,     "FWVB",     WRITE_COMMAND,    DECIAML_2,          edcFPGA2_VERSION,             NULL,          CLI_COM_READ_ONLY},
    {eCLI_MAIN_PIF,     "FWVC",     READ_COMMAND,     DECIAML_2,          edcCAMERA_FW_VERSION,             NULL,      CLI_COM_READ_ONLY | CLI_COM_NEED_SERVICE_CODE_READ},		//A70Gen2_Doulas_00012 Modify
    //{eCLI_MAIN_PIF,     "FWVC",     WRITE_COMMAND,    DECIAML_2,          edcFPGA3_VERSION,             NULL,          CLI_COM_READ_ONLY},
	{eCLI_MAIN_PIF,     "FWVD",     READ_COMMAND,     DECIAML_2,          edcSMCU_VERSION,              NULL,          CLI_COM_READ_ONLY | CLI_COM_NEED_SERVICE_CODE_READ},
    {eCLI_MAIN_PIF,     "FWVR",     READ_COMMAND,     DECIAML_2,          edcRELEASE_VERSION,           NULL,          CLI_COM_READ_ONLY | CLI_COM_NEED_SERVICE_CODE_READ},
    //{eCLI_MAIN_PIF,     "FWVR",     WRITE_COMMAND,    DECIAML_2,          edcRELEASE_VERSION,           NULL,          CLI_COM_READ_ONLY},
    {eCLI_MAIN_PIF,     "FWVX",     READ_COMMAND,     DECIAML_2,          edcXFPGA_VERSION,             NULL,          CLI_COM_READ_ONLY | CLI_COM_NEED_SERVICE_CODE_READ}, //A70LK_Jacky_0007//HICC2_Julie_0005

    {eCLI_MAIN_PIF,     "WHEE",     READ_COMMAND,     DECIAML_3,          edcWHEEL_INDEX_SHOW,          NULL,          CLI_COM_READ_ONLY | CLI_COM_NEED_SERVICE_CODE_READ},
    //{eCLI_MAIN_PIF,     "WHEE",     WRITE_COMMAND,    DECIAML_3,          edcWHEEL_INDEX_SHOW,          NULL,          CLI_COM_READ_ONLY},
    //{eCLI_MAIN_PIF,     "SXPJ",     READ_COMMAND,     DECIAML_3,          DATA_CODE_NA,  utilChristie_PIF_SXRJ,          CLI_COM_READ_ONLY | CLI_COM_NO_REPLY}, //A70LV_Larry_0235
    //{eCLI_MAIN_PIF,     "SXPJ",     WRITE_COMMAND,    DECIAML_3,          DATA_CODE_NA,  utilChristie_PIF_SXRJ,          CLI_COM_READ_ONLY | CLI_COM_NO_REPLY}, //A70LV_Larry_0235

    {eCLI_MAIN_PIF,     "STBM",     READ_COMMAND,     DECIAML_2,          edcSTANDBY_MODE,              NULL,          CLI_COM_NORMAL_AND_PN | CLI_COM_NEED_SERVICE_CODE_READ},
    {eCLI_MAIN_PIF,     "LOCK",     READ_COMMAND,     DECIAML_2,          edcLOCK_ALL_LENS_MOTORS,      NULL,          CLI_COM_NORMAL | CLI_COM_NEED_SERVICE_CODE_READ},

    //DEF============================================================================================================//DEF
    //{eCLI_MAIN_DEF,     NULL,       READ_COMMAND,     DECIAML_3,          DATA_CODE_NA,       utilChristie_DEF,          CLI_COM_WRITE_ONLY | CLI_COM_NEED_SERVICE_CODE_WRITE},     //A70LV_Doulas_0138
    {eCLI_MAIN_DEF,     NULL,       WRITE_COMMAND,    DECIAML_3,          DATA_CODE_NA,       utilChristie_DEF,          CLI_COM_WRITE_ONLY | CLI_COM_NEED_SERVICE_CODE_WRITE | CLI_COM_CHECK_BUSY},     //A70LV_Doulas_0138

    //TPG+ITP============================================================================================================//ITP
    {eCLI_MAIN_TPG,     "ITPG",      READ_COMMAND,     DECIAML_2,          edcSERVICE_TEST_PATTERN,      NULL,          CLI_COM_NORMAL_AND_PN | CLI_COM_NEED_SERVICE_CODE_READ}, //HICC2_Doulas_0029
    {eCLI_MAIN_TPG,     "ITPG",      WRITE_COMMAND,    DECIAML_2,          edcSERVICE_TEST_PATTERN,      NULL,          CLI_COM_NORMAL_AND_PN | CLI_COM_NEED_SERVICE_CODE_WRITE}, //HICC2_Doulas_0029
    //{eCLI_MAIN_TPG,     "ITPG",      READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,      utilChristie_CLI_TPG,          CLI_COM_NORMAL_AND_PN},
    //{eCLI_MAIN_TPG,     "ITPG",      WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,      utilChristie_CLI_TPG,          CLI_COM_NORMAL_AND_PN},

    //ITP============================================================================================================//ITP
    {eCLI_MAIN_ITP,     NULL,       READ_COMMAND,     DECIAML_2,           edcOSDTEST_PATTERN,           NULL,          CLI_COM_NORMAL_AND_PN}, //HICC2_Doulas_0029
    {eCLI_MAIN_ITP,     NULL,       WRITE_COMMAND,    DECIAML_2,           edcOSDTEST_PATTERN,           NULL,          CLI_COM_NORMAL_AND_PN}, //HICC2_Doulas_0029

    //FTP============================================================================================================//ITP
    {eCLI_MAIN_FTP,     NULL,       READ_COMMAND,     DECIAML_2,           edcFACTORY_TEST_PATTERN,           NULL,          CLI_COM_NORMAL_AND_PN},
    {eCLI_MAIN_FTP,     NULL,       WRITE_COMMAND,    DECIAML_2,           edcFACTORY_TEST_PATTERN,           NULL,          CLI_COM_NORMAL_AND_PN},

    //CWI============================================================================================================//CWI
    //{eCLI_MAIN_CWI,     "PF2X",     READ_COMMAND,     DECIAML_3,          DATA_CODE_NA,  utilChristie_CWI_PF2X,          CLI_COM_NORMAL | CLI_COM_NEED_SERVICE_CODE_WRITE},     //A70LV_Doulas_0138 modify
    //{eCLI_MAIN_CWI,     "PF2X",     WRITE_COMMAND,    DECIAML_3,          DATA_CODE_NA,  utilChristie_CWI_PF2X,          CLI_COM_NORMAL | CLI_COM_NEED_SERVICE_CODE_WRITE},
    //{eCLI_MAIN_CWI,     "FT2X",     READ_COMMAND,     DECIAML_3,          DATA_CODE_NA,  utilChristie_CWI_FT2X,          CLI_COM_NORMAL | CLI_COM_NEED_SERVICE_CODE_WRITE},
    //{eCLI_MAIN_CWI,     "FT2X",     WRITE_COMMAND,    DECIAML_3,          DATA_CODE_NA,  utilChristie_CWI_FT2X,          CLI_COM_NORMAL | CLI_COM_NEED_SERVICE_CODE_WRITE},
    {eCLI_MAIN_CWI,     "PF3X",     READ_COMMAND,     DECIAML_3,          DATA_CODE_NA,  utilChristie_CWI_PF3X,          CLI_COM_NORMAL | CLI_COM_NEED_SERVICE_CODE_READ},
    {eCLI_MAIN_CWI,     "PF3X",     WRITE_COMMAND,    DECIAML_3,          DATA_CODE_NA,  utilChristie_CWI_PF3X,          CLI_COM_NORMAL | CLI_COM_NEED_SERVICE_CODE_WRITE},
    {eCLI_MAIN_CWI,     "FT3X",     READ_COMMAND,     DECIAML_3,          DATA_CODE_NA,  utilChristie_CWI_FT3X,          CLI_COM_NORMAL | CLI_COM_NEED_SERVICE_CODE_READ},
    {eCLI_MAIN_CWI,     "FT3X",     WRITE_COMMAND,    DECIAML_3,          DATA_CODE_NA,  utilChristie_CWI_FT3X,          CLI_COM_NORMAL | CLI_COM_NEED_SERVICE_CODE_WRITE},
    {eCLI_MAIN_CWI,     "PF4X",     READ_COMMAND,     DECIAML_3,          DATA_CODE_NA,  utilChristie_CWI_PF3X,          CLI_COM_NORMAL | CLI_COM_NEED_SERVICE_CODE_READ},
    {eCLI_MAIN_CWI,     "PF4X",     WRITE_COMMAND,    DECIAML_3,          DATA_CODE_NA,  utilChristie_CWI_PF3X,          CLI_COM_NORMAL | CLI_COM_NEED_SERVICE_CODE_WRITE},
    {eCLI_MAIN_CWI,     "FT4X",     READ_COMMAND,     DECIAML_3,          DATA_CODE_NA,  utilChristie_CWI_FT3X,          CLI_COM_NORMAL | CLI_COM_NEED_SERVICE_CODE_READ},
    {eCLI_MAIN_CWI,     "FT4X",     WRITE_COMMAND,    DECIAML_3,          DATA_CODE_NA,  utilChristie_CWI_FT3X,          CLI_COM_NORMAL | CLI_COM_NEED_SERVICE_CODE_WRITE},


    //ERR============================================================================================================//ERR
    {eCLI_MAIN_ERR,     NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,            utilChristie_CLI_ERR,          CLI_COM_READ_ONLY | CLI_COM_NO_REPLY | CLI_COM_NEED_SERVICE_CODE_READ},
    //{eCLI_MAIN_ERR,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcSHOW_ERROR_LOG,            NULL,          CLI_COM_READ_ONLY},
    //{eCLI_MAIN_ERR,     "CLER",     READ_COMMAND,     DECIAML_2,          edcCLEAR_ERROR_LOG,           NULL,          CLI_COM_WRITE_ONLY},
    {eCLI_MAIN_ERR,     "CLER",     WRITE_COMMAND,    DECIAML_2,          edcCLEAR_ERROR_LOG,           NULL,          CLI_COM_WRITE_ONLY | CLI_COM_NEED_SERVICE_CODE_WRITE},

    //MDT============================================================================================================//MDT
    {eCLI_MAIN_MDT,     NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,       utilChristie_MDT,          CLI_COM_READ_ONLY | CLI_COM_NEED_SERVICE_CODE_WRITE},  //A70LV_Doulas_0209 modify
    {eCLI_MAIN_MDT,     NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,       utilChristie_MDT,          CLI_COM_READ_ONLY | CLI_COM_NEED_SERVICE_CODE_WRITE},  //A70LV_Doulas_0209 modify
    {eCLI_MAIN_MDT,     "HPOS",     READ_COMMAND,     DECIAML_3,          DATA_CODE_NA,  utilChristie_MDT_HPOS,          CLI_COM_NORMAL | CLI_COM_NEED_SERVICE_CODE_WRITE},     //A70LV_Doulas_0209 modify
    {eCLI_MAIN_MDT,     "HPOS",     WRITE_COMMAND,    DECIAML_3,          DATA_CODE_NA,  utilChristie_MDT_HPOS,          CLI_COM_NORMAL | CLI_COM_NEED_SERVICE_CODE_WRITE},     //A70LV_Doulas_0209 modify
    {eCLI_MAIN_MDT,     "VPOS",     READ_COMMAND,     DECIAML_3,          DATA_CODE_NA,  utilChristie_MDT_VPOS,          CLI_COM_NORMAL | CLI_COM_NEED_SERVICE_CODE_WRITE},     //A70LV_Doulas_0209 modify
    {eCLI_MAIN_MDT,     "VPOS",     WRITE_COMMAND,    DECIAML_3,          DATA_CODE_NA,  utilChristie_MDT_VPOS,          CLI_COM_NORMAL | CLI_COM_NEED_SERVICE_CODE_WRITE},     //A70LV_Doulas_0209 modify
    {eCLI_MAIN_MDT,     "SAVE",     READ_COMMAND,     DECIAML_3,          DATA_CODE_NA,  utilChristie_MDT_SAVE,          CLI_COM_WRITE_ONLY | CLI_COM_NEED_SERVICE_CODE_WRITE}, //A70LV_Doulas_0209 modify
    {eCLI_MAIN_MDT,     "SAVE",     WRITE_COMMAND,    DECIAML_3,          DATA_CODE_NA,  utilChristie_MDT_SAVE,          CLI_COM_WRITE_ONLY | CLI_COM_NEED_SERVICE_CODE_WRITE}, //A70LV_Doulas_0209 modify
    {eCLI_MAIN_MDT,     "CLER",     READ_COMMAND,     DECIAML_3,          DATA_CODE_NA,  utilChristie_MDT_CLER,          CLI_COM_WRITE_ONLY | CLI_COM_NEED_SERVICE_CODE_WRITE}, //A70LV_Doulas_0209 modify
    {eCLI_MAIN_MDT,     "CLER",     WRITE_COMMAND,    DECIAML_3,          DATA_CODE_NA,  utilChristie_MDT_CLER,          CLI_COM_WRITE_ONLY | CLI_COM_NEED_SERVICE_CODE_WRITE}, //A70LV_Doulas_0209 modify
    {eCLI_MAIN_MDT,     "RDIX",     READ_COMMAND,     DECIAML_3,          DATA_CODE_NA,  utilChristie_MDT_RDIX,          CLI_COM_NORMAL | CLI_COM_NEED_SERVICE_CODE_WRITE},     //A70LV_Doulas_0209
    {eCLI_MAIN_MDT,     "RDIX",     WRITE_COMMAND,    DECIAML_3,          DATA_CODE_NA,  utilChristie_MDT_RDIX,          CLI_COM_NORMAL | CLI_COM_NEED_SERVICE_CODE_WRITE},     //A70LV_Doulas_0209

    //LD Info============================================================================================================//LD Info
    //{eCLI_MAIN_LDI,     "LD01",     WRITE_COMMAND,    DECIAML_2,          edcLD_INFO_01,                NULL,         CLI_COM_READ_ONLY},
    {eCLI_MAIN_LDI,     "LD01",     READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,     utilChristie_CLI_FAN_LD_INFO,         CLI_COM_NO_REPLY | CLI_COM_READ_ONLY | CLI_COM_NEED_SERVICE_CODE_READ},//HICC2_Julie_0022
    //{eCLI_MAIN_LDI,     "LD02",     WRITE_COMMAND,    DECIAML_2,          edcLD_INFO_02,                NULL,         CLI_COM_READ_ONLY},
    {eCLI_MAIN_LDI,     "LD02",     READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,     utilChristie_CLI_FAN_LD_INFO,         CLI_COM_NO_REPLY | CLI_COM_READ_ONLY | CLI_COM_NEED_SERVICE_CODE_READ},//HICC2_Julie_0022
    //{eCLI_MAIN_LDI,     "LD03",     WRITE_COMMAND,    DECIAML_2,          edcLD_INFO_03,                NULL,         CLI_COM_READ_ONLY},
    {eCLI_MAIN_LDI,     "LD03",     READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,     utilChristie_CLI_FAN_LD_INFO,         CLI_COM_NO_REPLY | CLI_COM_READ_ONLY | CLI_COM_NEED_SERVICE_CODE_READ},//HICC2_Julie_0022
    //{eCLI_MAIN_LDI,     "LD04",     WRITE_COMMAND,    DECIAML_2,          edcLD_INFO_04,                NULL,         CLI_COM_READ_ONLY},
    {eCLI_MAIN_LDI,     "LD04",     READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,     utilChristie_CLI_FAN_LD_INFO,         CLI_COM_NO_REPLY | CLI_COM_READ_ONLY | CLI_COM_NEED_SERVICE_CODE_READ},//HICC2_Julie_0022
    //{eCLI_MAIN_LDI,     "LD05",     WRITE_COMMAND,    DECIAML_2,          edcLD_INFO_05,                NULL,         CLI_COM_READ_ONLY},
    {eCLI_MAIN_LDI,     "LD05",     READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,     utilChristie_CLI_FAN_LD_INFO,         CLI_COM_NO_REPLY | CLI_COM_READ_ONLY | CLI_COM_NEED_SERVICE_CODE_READ},//HICC2_Julie_0022
    //{eCLI_MAIN_LDI,     "LD06",     WRITE_COMMAND,    DECIAML_2,          edcLD_INFO_06,                NULL,         CLI_COM_READ_ONLY},
    {eCLI_MAIN_LDI,     "LD06",     READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,     utilChristie_CLI_FAN_LD_INFO,         CLI_COM_NO_REPLY | CLI_COM_READ_ONLY | CLI_COM_NEED_SERVICE_CODE_READ},//HICC2_Julie_0022
    //{eCLI_MAIN_LDI,     "LD07",     WRITE_COMMAND,    DECIAML_2,          edcLD_INFO_07,                NULL,         CLI_COM_READ_ONLY},
    {eCLI_MAIN_LDI,     "LD07",     READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,     utilChristie_CLI_FAN_LD_INFO,         CLI_COM_NO_REPLY | CLI_COM_READ_ONLY | CLI_COM_NEED_SERVICE_CODE_READ},//HICC2_Julie_0022
    //{eCLI_MAIN_LDI,     "LD08",     WRITE_COMMAND,    DECIAML_2,          edcLD_INFO_08,                NULL,         CLI_COM_READ_ONLY},
    {eCLI_MAIN_LDI,     "LD08",     READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,     utilChristie_CLI_FAN_LD_INFO,         CLI_COM_NO_REPLY | CLI_COM_READ_ONLY | CLI_COM_NEED_SERVICE_CODE_READ},//HICC2_Julie_0022
    //{eCLI_MAIN_LDI,     "LD09",     WRITE_COMMAND,    DECIAML_2,          edcLD_INFO_09,                NULL,         CLI_COM_READ_ONLY},
    {eCLI_MAIN_LDI,     "LD09",     READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,     utilChristie_CLI_FAN_LD_INFO,         CLI_COM_NO_REPLY | CLI_COM_READ_ONLY | CLI_COM_NEED_SERVICE_CODE_READ},//HICC2_Julie_0022
    //{eCLI_MAIN_LDI,     "LD10",     WRITE_COMMAND,    DECIAML_2,          edcLD_INFO_10,                NULL,         CLI_COM_READ_ONLY},
    {eCLI_MAIN_LDI,     "LD10",     READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,     utilChristie_CLI_FAN_LD_INFO,         CLI_COM_NO_REPLY | CLI_COM_READ_ONLY | CLI_COM_NEED_SERVICE_CODE_READ},//HICC2_Julie_0022
    //{eCLI_MAIN_LDI,     "LD11",     WRITE_COMMAND,    DECIAML_2,          edcLD_INFO_11,                NULL,         CLI_COM_READ_ONLY},
    {eCLI_MAIN_LDI,     "LD11",     READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,     utilChristie_CLI_FAN_LD_INFO,         CLI_COM_NO_REPLY | CLI_COM_READ_ONLY | CLI_COM_NEED_SERVICE_CODE_READ},//HICC2_Julie_0022
    //{eCLI_MAIN_LDI,     "LD12",     WRITE_COMMAND,    DECIAML_2,          edcLD_INFO_12,                NULL,         CLI_COM_READ_ONLY},
    {eCLI_MAIN_LDI,     "LD12",     READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,     utilChristie_CLI_FAN_LD_INFO,         CLI_COM_NO_REPLY | CLI_COM_READ_ONLY | CLI_COM_NEED_SERVICE_CODE_READ},//HICC2_Julie_0022
    //{eCLI_MAIN_LDI,     "LD13",     WRITE_COMMAND,    DECIAML_2,          edcLD_INFO_13,                NULL,         CLI_COM_READ_ONLY},
    {eCLI_MAIN_LDI,     "LD13",     READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,     utilChristie_CLI_FAN_LD_INFO,         CLI_COM_NO_REPLY | CLI_COM_READ_ONLY | CLI_COM_NEED_SERVICE_CODE_READ},//HICC2_Julie_0022
    //{eCLI_MAIN_LDI,     "LD14",     WRITE_COMMAND,    DECIAML_2,          edcLD_INFO_14,                NULL,         CLI_COM_READ_ONLY},
    {eCLI_MAIN_LDI,     "LD14",     READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,     utilChristie_CLI_FAN_LD_INFO,         CLI_COM_NO_REPLY | CLI_COM_READ_ONLY | CLI_COM_NEED_SERVICE_CODE_READ},//HICC2_Julie_0022

    //Fan Info============================================================================================================//Fan Info
    //{eCLI_MAIN_FAN,     "FA01",     WRITE_COMMAND,    DECIAML_4,          edcFAN_RPM_01,                NULL,         CLI_COM_READ_ONLY},
    {eCLI_MAIN_FAN,     "FA01",     READ_COMMAND,     DECIAML_4,          DATA_CODE_NA,     utilChristie_CLI_FAN_LD_INFO,         CLI_COM_NO_REPLY | CLI_COM_READ_ONLY | CLI_COM_NEED_SERVICE_CODE_READ},//HICC2_Julie_0022
    //{eCLI_MAIN_FAN,     "FA02",     WRITE_COMMAND,    DECIAML_4,          edcFAN_RPM_02,                NULL,         CLI_COM_READ_ONLY},
    {eCLI_MAIN_FAN,     "FA02",     READ_COMMAND,     DECIAML_4,          DATA_CODE_NA,     utilChristie_CLI_FAN_LD_INFO,         CLI_COM_NO_REPLY | CLI_COM_READ_ONLY | CLI_COM_NEED_SERVICE_CODE_READ},//HICC2_Julie_0022
    //{eCLI_MAIN_FAN,     "FA03",     WRITE_COMMAND,    DECIAML_4,          edcFAN_RPM_03,                NULL,         CLI_COM_READ_ONLY},
    {eCLI_MAIN_FAN,     "FA03",     READ_COMMAND,     DECIAML_4,          DATA_CODE_NA,     utilChristie_CLI_FAN_LD_INFO,         CLI_COM_NO_REPLY | CLI_COM_READ_ONLY | CLI_COM_NEED_SERVICE_CODE_READ},//HICC2_Julie_0022
    //{eCLI_MAIN_FAN,     "FA04",     WRITE_COMMAND,    DECIAML_4,          edcFAN_RPM_04,                NULL,         CLI_COM_READ_ONLY},
    {eCLI_MAIN_FAN,     "FA04",     READ_COMMAND,     DECIAML_4,          DATA_CODE_NA,     utilChristie_CLI_FAN_LD_INFO,         CLI_COM_NO_REPLY | CLI_COM_READ_ONLY | CLI_COM_NEED_SERVICE_CODE_READ},//HICC2_Julie_0022
    //{eCLI_MAIN_FAN,     "FA05",     WRITE_COMMAND,    DECIAML_4,          edcFAN_RPM_05,                NULL,         CLI_COM_READ_ONLY},
    {eCLI_MAIN_FAN,     "FA05",     READ_COMMAND,     DECIAML_4,          DATA_CODE_NA,     utilChristie_CLI_FAN_LD_INFO,         CLI_COM_NO_REPLY | CLI_COM_READ_ONLY | CLI_COM_NEED_SERVICE_CODE_READ},//HICC2_Julie_0022
    //{eCLI_MAIN_FAN,     "FA06",     WRITE_COMMAND,    DECIAML_4,          edcFAN_RPM_06,                NULL,         CLI_COM_READ_ONLY},
    {eCLI_MAIN_FAN,     "FA06",     READ_COMMAND,     DECIAML_4,          DATA_CODE_NA,     utilChristie_CLI_FAN_LD_INFO,         CLI_COM_NO_REPLY | CLI_COM_READ_ONLY | CLI_COM_NEED_SERVICE_CODE_READ},//HICC2_Julie_0022
    //{eCLI_MAIN_FAN,     "FA07",     WRITE_COMMAND,    DECIAML_4,          edcFAN_RPM_07,                NULL,         CLI_COM_READ_ONLY},
    {eCLI_MAIN_FAN,     "FA07",     READ_COMMAND,     DECIAML_4,          DATA_CODE_NA,     utilChristie_CLI_FAN_LD_INFO,         CLI_COM_NO_REPLY | CLI_COM_READ_ONLY | CLI_COM_NEED_SERVICE_CODE_READ},//HICC2_Julie_0022
    //{eCLI_MAIN_FAN,     "FA08",     WRITE_COMMAND,    DECIAML_4,          edcFAN_RPM_08,                NULL,         CLI_COM_READ_ONLY},
    {eCLI_MAIN_FAN,     "FA08",     READ_COMMAND,     DECIAML_4,          DATA_CODE_NA,     utilChristie_CLI_FAN_LD_INFO,         CLI_COM_NO_REPLY | CLI_COM_READ_ONLY | CLI_COM_NEED_SERVICE_CODE_READ},//HICC2_Julie_0022
    //{eCLI_MAIN_FAN,     "FA09",     WRITE_COMMAND,    DECIAML_4,          edcFAN_RPM_09,                NULL,         CLI_COM_READ_ONLY},
    {eCLI_MAIN_FAN,     "FA09",     READ_COMMAND,     DECIAML_4,          DATA_CODE_NA,     utilChristie_CLI_FAN_LD_INFO,         CLI_COM_NO_REPLY | CLI_COM_READ_ONLY | CLI_COM_NEED_SERVICE_CODE_READ},//HICC2_Julie_0022
    //{eCLI_MAIN_FAN,     "FA10",     WRITE_COMMAND,    DECIAML_4,          edcFAN_RPM_10,                NULL,         CLI_COM_READ_ONLY},
    {eCLI_MAIN_FAN,     "FA10",     READ_COMMAND,     DECIAML_4,          DATA_CODE_NA,     utilChristie_CLI_FAN_LD_INFO,         CLI_COM_NO_REPLY | CLI_COM_READ_ONLY | CLI_COM_NEED_SERVICE_CODE_READ},//HICC2_Julie_0022
    //{eCLI_MAIN_FAN,     "FA11",     WRITE_COMMAND,    DECIAML_4,          edcFAN_RPM_11,                NULL,         CLI_COM_READ_ONLY},
    {eCLI_MAIN_FAN,     "FA11",     READ_COMMAND,     DECIAML_4,          DATA_CODE_NA,     utilChristie_CLI_FAN_LD_INFO,         CLI_COM_NO_REPLY | CLI_COM_READ_ONLY | CLI_COM_NEED_SERVICE_CODE_READ},//HICC2_Julie_0022
    //{eCLI_MAIN_FAN,     "FA12",     WRITE_COMMAND,    DECIAML_4,          edcFAN_RPM_12,                NULL,         CLI_COM_READ_ONLY},
    {eCLI_MAIN_FAN,     "FA12",     READ_COMMAND,     DECIAML_4,          DATA_CODE_NA,     utilChristie_CLI_FAN_LD_INFO,         CLI_COM_NO_REPLY | CLI_COM_READ_ONLY | CLI_COM_NEED_SERVICE_CODE_READ},//HICC2_Julie_0022

    {eCLI_MAIN_FAN,     "FA13",     READ_COMMAND,     DECIAML_4,          DATA_CODE_NA,     utilChristie_CLI_FAN_LD_INFO,         CLI_COM_NO_REPLY | CLI_COM_READ_ONLY | CLI_COM_NEED_SERVICE_CODE_READ}, //HICC2_Doulas_0029//HICC2_Julie_0022
    {eCLI_MAIN_FAN,     "FA14",     READ_COMMAND,     DECIAML_4,          DATA_CODE_NA,     utilChristie_CLI_FAN_LD_INFO,         CLI_COM_NO_REPLY | CLI_COM_READ_ONLY | CLI_COM_NEED_SERVICE_CODE_READ}, //HICC2_Doulas_0029//HICC2_Julie_0022
    {eCLI_MAIN_FAN,     "FA15",     READ_COMMAND,     DECIAML_4,          DATA_CODE_NA,     utilChristie_CLI_FAN_LD_INFO,         CLI_COM_NO_REPLY | CLI_COM_READ_ONLY | CLI_COM_NEED_SERVICE_CODE_READ}, //HICC2_Doulas_0029//HICC2_Julie_0022
    {eCLI_MAIN_FAN,     "FA16",     READ_COMMAND,     DECIAML_4,          DATA_CODE_NA,     utilChristie_CLI_FAN_LD_INFO,         CLI_COM_NO_REPLY | CLI_COM_READ_ONLY | CLI_COM_NEED_SERVICE_CODE_READ}, //HICC2_Doulas_0029//HICC2_Julie_0022
    {eCLI_MAIN_FAN,     "FA17",     READ_COMMAND,     DECIAML_4,          DATA_CODE_NA,     utilChristie_CLI_FAN_LD_INFO,         CLI_COM_NO_REPLY | CLI_COM_READ_ONLY | CLI_COM_NEED_SERVICE_CODE_READ}, //HICC2_Doulas_0029//HICC2_Julie_0022

	{eCLI_MAIN_FAN,     "TEC1",     READ_COMMAND,     DECIAML_4,          edcTECCURRENT_1,              NULL,         CLI_COM_READ_ONLY | CLI_COM_NEED_SERVICE_CODE_READ}, //A70Gen2_Julie_0034
	{eCLI_MAIN_FAN,     "TEC2",     READ_COMMAND,     DECIAML_4,          edcTECCURRENT_2,              NULL,         CLI_COM_READ_ONLY | CLI_COM_NEED_SERVICE_CODE_READ}, //A70Gen2_Julie_0034
	{eCLI_MAIN_FAN,     "PUMP",     READ_COMMAND,     DECIAML_4,          edcPUMP_RPM,                  NULL,         CLI_COM_READ_ONLY | CLI_COM_NEED_SERVICE_CODE_READ}, //A70Gen2_Julie_0034
	{eCLI_MAIN_FAN,     "FILT",     READ_COMMAND,     DECIAML_4,          edcFAN_FILTER,                NULL,         CLI_COM_READ_ONLY | CLI_COM_NEED_SERVICE_CODE_READ}, //A70Gen2_Julie_0034

    //LPM============================================================================================================//LPM
    {eCLI_MAIN_LPM,     NULL,       READ_COMMAND,     DECIAML_2,          edcPOWER_MODE,                NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_LPM,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcPOWER_MODE,                NULL,          CLI_COM_NORMAL},

    //LPP============================================================================================================//LPP
    {eCLI_MAIN_LPP,     NULL,       READ_COMMAND,     DECIAML_2,          edcCONSTANT_POWER_NUMBER,     NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_LPP,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcCONSTANT_POWER_NUMBER,     NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_LPP,     "OFFT",     READ_COMMAND,     DECIAML_2,          edcCONSTANT_POWER_OFFSET,     NULL,          CLI_COM_NORMAL}, //A70LV_Larry_0409
    {eCLI_MAIN_LPP,     "OFFT",     WRITE_COMMAND,    DECIAML_2,          edcCONSTANT_POWER_OFFSET,     NULL,          CLI_COM_NORMAL}, //A70LV_Larry_0409

    //LIF============================================================================================================//LIF
    {eCLI_MAIN_LIF,     "TPHS",     READ_COMMAND,     DECIAML_5,          DATA_CODE_NA,   utilChristie_LIF_TPHS,          CLI_COM_NORMAL| CLI_COM_NEED_SERVICE_CODE_WRITE}, //A70LV_Larry_0268
    {eCLI_MAIN_LIF,     "TPHS",     WRITE_COMMAND,    DECIAML_5,          DATA_CODE_NA,   utilChristie_LIF_TPHS,          CLI_COM_NORMAL| CLI_COM_NEED_SERVICE_CODE_WRITE}, //A70LV_Larry_0268
    {eCLI_MAIN_LIF,     "LSHS",     READ_COMMAND,     DECIAML_5,          DATA_CODE_NA,   utilChristie_LIF_LSHS,          CLI_COM_NORMAL| CLI_COM_NEED_SERVICE_CODE_WRITE}, //A70LV_Larry_0268
    {eCLI_MAIN_LIF,     "LSHS",     WRITE_COMMAND,    DECIAML_5,          DATA_CODE_NA,   utilChristie_LIF_LSHS,          CLI_COM_NORMAL| CLI_COM_NEED_SERVICE_CODE_WRITE}, //A70LV_Larry_0268

    //RLC============================================================================================================//RLC
    {eCLI_MAIN_RLC,     NULL,       READ_COMMAND,     DECIAML_2,          edcLIGHT_SENSOR_CALIBRATION,  NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_RLC,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcLIGHT_SENSOR_CALIBRATION,  NULL,          CLI_COM_NORMAL},

    //SIN============================================================================================================//SIN
    {eCLI_MAIN_SIN,     NULL,       READ_COMMAND,     DECIAML_2,          edcMAIN_INPUT,       NULL,          CLI_COM_NORMAL},     //A70LV_Doulas_0135 modify
    {eCLI_MAIN_SIN,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcMAIN_INPUT,       NULL,          CLI_COM_NORMAL},     //A70LV_Doulas_0135 modify
    {eCLI_MAIN_SIN,     "MAIN",     READ_COMMAND,     DECIAML_2,          edcMAIN_INPUT,       NULL,          CLI_COM_NORMAL},     //A70LV_Doulas_0135 modify
    {eCLI_MAIN_SIN,     "MAIN",     WRITE_COMMAND,    DECIAML_2,          edcMAIN_INPUT,       NULL,          CLI_COM_NORMAL},     //A70LV_Doulas_0135 modify
    {eCLI_MAIN_SIN,     "PIIP",     READ_COMMAND,     DECIAML_2,          edcSUB_INPUT,        NULL,          CLI_COM_NORMAL},     //A70LV_Doulas_0135 modify
    {eCLI_MAIN_SIN,     "PIIP",     WRITE_COMMAND,    DECIAML_2,          edcSUB_INPUT,        NULL,          CLI_COM_NORMAL},     //A70LV_Doulas_0135 modify

    //PIP============================================================================================================//PIP
    {eCLI_MAIN_PIP,     NULL,       READ_COMMAND,     DECIAML_2,          edcPIP_PBP_ENABLE,            NULL,          CLI_COM_NORMAL_AND_PN},
    {eCLI_MAIN_PIP,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcPIP_PBP_ENABLE,            NULL,          CLI_COM_NORMAL_AND_PN},

    //PPS============================================================================================================//PPS
    {eCLI_MAIN_PPS,     NULL,       READ_COMMAND,     DECIAML_2,          edcSWAP,                      NULL,          CLI_COM_WRITE_ONLY},
    {eCLI_MAIN_PPS,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcSWAP,                      NULL,          CLI_COM_WRITE_ONLY},

    //PHS============================================================================================================//PHS
    {eCLI_MAIN_PHS,     NULL,       READ_COMMAND,     DECIAML_2,          edcSIZE,                      NULL,          CLI_COM_NORMAL_AND_PN},
    {eCLI_MAIN_PHS,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcSIZE,                      NULL,          CLI_COM_NORMAL_AND_PN},

    //PPP============================================================================================================//PPP
    {eCLI_MAIN_PPP,     NULL,       READ_COMMAND,     DECIAML_2,          edcMAIN_LAYOUT,               NULL,          CLI_COM_NORMAL_AND_PN},
    {eCLI_MAIN_PPP,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcMAIN_LAYOUT,               NULL,          CLI_COM_NORMAL_AND_PN},

    //TMG============================================================================================================//TMG
    {eCLI_MAIN_TMG,     NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,                   NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_TMG,     NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,                   NULL,          CLI_COM_NORMAL},

    //SKS============================================================================================================//SKS
    {eCLI_MAIN_SKS,     NULL,       READ_COMMAND,     DECIAML_2,          edcINPUT_KEY,                 NULL,          CLI_COM_NORMAL_AND_PN},
    {eCLI_MAIN_SKS,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcINPUT_KEY,                 NULL,          CLI_COM_NORMAL_AND_PN},

    //KEY============================================================================================================//KEY
    {eCLI_MAIN_KEY,     NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,      utilChristie_CLI_KEY,       CLI_COM_WRITE_ONLY},

    //PXP============================================================================================================//PXP
    {eCLI_MAIN_PXP,     NULL,       READ_COMMAND,     DECIAML_3,          edcPIXEL_PHASE,               NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_PXP,     NULL,       WRITE_COMMAND,    DECIAML_3,          edcPIXEL_PHASE,               NULL,          CLI_COM_NORMAL},

    //WAP============================================================================================================//WAP
    {eCLI_MAIN_WAP,     NULL,       READ_COMMAND,     DECIAML_2,          edcWARP_MEMORY_APPLY,         NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_WAP,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcWARP_MEMORY_APPLY,         NULL,          CLI_COM_NORMAL},

    //WAS============================================================================================================//WAS
    {eCLI_MAIN_WAS,     NULL,       READ_COMMAND,     DECIAML_2,          edcWARP_MEMORY_SAVE,          NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_WAS,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcWARP_MEMORY_SAVE,          NULL,          CLI_COM_NORMAL},

    {eCLI_MAIN_WAS,     "NAME",     WRITE_COMMAND,    DECIAML_2,          edcWARP_MEMORY_NAME_4,        NULL,          CLI_COM_NORMAL}, //for debug

    //BLD============================================================================================================//WAP
    {eCLI_MAIN_BLD,     NULL,       READ_COMMAND,     DECIAML_2,          edcBLEND_MEMORY_APPLY,        NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_BLD,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcBLEND_MEMORY_APPLY,        NULL,          CLI_COM_NORMAL},

    //BLS============================================================================================================
    {eCLI_MAIN_BLS,     NULL,       READ_COMMAND,     DECIAML_2,          edcBLEND_MEMORY_SAVE,         NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_BLS,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcBLEND_MEMORY_SAVE,         NULL,          CLI_COM_NORMAL},

    {eCLI_MAIN_BLS,     "NAME",     WRITE_COMMAND,    DECIAML_2,          edcBLEND_MEMORY_NAME_4,       NULL,          CLI_COM_NORMAL}, //for debug

    //EBL============================================================================================================//EBL
    {eCLI_MAIN_EBL,     "TOPE",     READ_COMMAND,     DECIAML_2,          edcBLENDING_TOP_ENABLE,           NULL,           CLI_COM_NORMAL_AND_PN},
    {eCLI_MAIN_EBL,     "TOPE",     WRITE_COMMAND,    DECIAML_2,          edcBLENDING_TOP_ENABLE,           NULL,           CLI_COM_NORMAL_AND_PN},
    {eCLI_MAIN_EBL,     "TOPS",     READ_COMMAND,     DECIAML_3,          edcBLENDING_TOP_START_PIXEL,      NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_EBL,     "TOPS",     WRITE_COMMAND,    DECIAML_3,          edcBLENDING_TOP_START_PIXEL,      NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_EBL,     "TOPW",     READ_COMMAND,     DECIAML_3,          edcBLENDING_TOP_PIXEL_WIDTH,      NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_EBL,     "TOPW",     WRITE_COMMAND,    DECIAML_3,          edcBLENDING_TOP_PIXEL_WIDTH,      NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_EBL,     "BTME",     READ_COMMAND,     DECIAML_2,          edcBLENDING_BOTTOM_ENABLE,        NULL,           CLI_COM_NORMAL_AND_PN},
    {eCLI_MAIN_EBL,     "BTME",     WRITE_COMMAND,    DECIAML_2,          edcBLENDING_BOTTOM_ENABLE,        NULL,           CLI_COM_NORMAL_AND_PN},
    {eCLI_MAIN_EBL,     "BTMS",     READ_COMMAND,     DECIAML_3,          edcBLENDING_BOTTOM_START_PIXEL,   NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_EBL,     "BTMS",     WRITE_COMMAND,    DECIAML_3,          edcBLENDING_BOTTOM_START_PIXEL,   NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_EBL,     "BTMW",     READ_COMMAND,     DECIAML_3,          edcBLENDING_BOTTOM_PIXEL_WIDTH,   NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_EBL,     "BTMW",     WRITE_COMMAND,    DECIAML_3,          edcBLENDING_BOTTOM_PIXEL_WIDTH,   NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_EBL,     "LFTE",     READ_COMMAND,     DECIAML_2,          edcBLENDING_LEFT_ENABLE,          NULL,           CLI_COM_NORMAL_AND_PN},
    {eCLI_MAIN_EBL,     "LFTE",     WRITE_COMMAND,    DECIAML_2,          edcBLENDING_LEFT_ENABLE,          NULL,           CLI_COM_NORMAL_AND_PN},
    {eCLI_MAIN_EBL,     "LFTS",     READ_COMMAND,     DECIAML_3,          edcBLENDING_LEFT_START_PIXEL,     NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_EBL,     "LFTS",     WRITE_COMMAND,    DECIAML_3,          edcBLENDING_LEFT_START_PIXEL,     NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_EBL,     "LFTW",     READ_COMMAND,     DECIAML_3,          edcBLENDING_LEFT_PIXEL_WIDTH,     NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_EBL,     "LFTW",     WRITE_COMMAND,    DECIAML_3,          edcBLENDING_LEFT_PIXEL_WIDTH,     NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_EBL,     "RHTE",     READ_COMMAND,     DECIAML_2,          edcBLENDING_RIGHT_ENABLE,         NULL,           CLI_COM_NORMAL_AND_PN},
    {eCLI_MAIN_EBL,     "RHTE",     WRITE_COMMAND,    DECIAML_2,          edcBLENDING_RIGHT_ENABLE,         NULL,           CLI_COM_NORMAL_AND_PN},
    {eCLI_MAIN_EBL,     "RHTS",     READ_COMMAND,     DECIAML_3,          edcBLENDING_RIGHT_START_PIXEL,    NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_EBL,     "RHTS",     WRITE_COMMAND,    DECIAML_3,          edcBLENDING_RIGHT_START_PIXEL,    NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_EBL,     "RHTW",     READ_COMMAND,     DECIAML_3,          edcBLENDING_RIGHT_PIXEL_WIDTH,    NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_EBL,     "RHTW",     WRITE_COMMAND,    DECIAML_3,          edcBLENDING_RIGHT_PIXEL_WIDTH,    NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_EBL,     "GAMA",     READ_COMMAND,     DECIAML_2,          edcBLENDING_GAMMA,                NULL,           CLI_COM_NORMAL_AND_PN},
    {eCLI_MAIN_EBL,     "GAMA",     WRITE_COMMAND,    DECIAML_2,          edcBLENDING_GAMMA,                NULL,           CLI_COM_NORMAL_AND_PN},

    //CLR============================================================================================================//CLR
    {eCLI_MAIN_CLR,     NULL,       READ_COMMAND,     DECIAML_2,          edcCOLOR,                         NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_CLR,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcCOLOR,                         NULL,          CLI_COM_NORMAL},

    //TNT============================================================================================================//TNT
    {eCLI_MAIN_TNT,     NULL,       READ_COMMAND,     DECIAML_2,          edcTINT,                         NULL,          CLI_COM_NORMAL},  //A70LV_Doulas_0134 modify
    {eCLI_MAIN_TNT,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcTINT,                         NULL,          CLI_COM_NORMAL},  //A70LV_Doulas_0134 modify

    //SCR============================================================================================================//SCR
    {eCLI_MAIN_SCR,     NULL,       READ_COMMAND,     DECIAML_2,          edcSKIN_COLOR,                   NULL,          CLI_COM_NORMAL_AND_PN},
    {eCLI_MAIN_SCR,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcSKIN_COLOR,                   NULL,          CLI_COM_NORMAL_AND_PN},

    //TNR============================================================================================================//TNR
    {eCLI_MAIN_TNR,     NULL,       READ_COMMAND,     DECIAML_2,          edcTEMPORAL_NOISE_REDUCTION,     NULL,          CLI_COM_NORMAL_AND_PN},
    {eCLI_MAIN_TNR,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcTEMPORAL_NOISE_REDUCTION,     NULL,          CLI_COM_NORMAL_AND_PN},

    //MNR============================================================================================================//MNR
    {eCLI_MAIN_MNR,     NULL,       READ_COMMAND,     DECIAML_2,          edcMPEG_NOISE_REDUCTION,         NULL,          CLI_COM_NORMAL_AND_PN},
    {eCLI_MAIN_MNR,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcMPEG_NOISE_REDUCTION,         NULL,          CLI_COM_NORMAL_AND_PN},

    //MTO============================================================================================================//MTO
    {eCLI_MAIN_MTO,     NULL,       READ_COMMAND,     DECIAML_2,          edcMENU_TIME_OUT,                NULL,          CLI_COM_NORMAL_AND_PN},
    {eCLI_MAIN_MTO,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcMENU_TIME_OUT,                NULL,          CLI_COM_NORMAL_AND_PN},

    //SST============================================================================================================//SST
    {eCLI_MAIN_SST,     NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,         utilChristie_SST,           CLI_COM_NO_REPLY|CLI_COM_READ_ONLY},    //A70LV_Doulas_0140 modify
    {eCLI_MAIN_SST,     NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,         utilChristie_SST,           CLI_COM_NO_REPLY|CLI_COM_READ_ONLY},    //A70LV_Doulas_0140 modify

    //MIF============================================================================================================//MIF
    {eCLI_MAIN_MIF,     "ACTS",     READ_COMMAND,     DECIAML_2,          edcMAIN_INPUT,                   NULL,          CLI_COM_READ_ONLY},   //A70LV_Doulas_0136 modify
    {eCLI_MAIN_MIF,     "ACTS",     WRITE_COMMAND,    DECIAML_2,          edcMAIN_INPUT,                   NULL,          CLI_COM_READ_ONLY},   //A70LV_Doulas_0136 modify
    {eCLI_MAIN_MIF,     "SGFT",     READ_COMMAND,     DECIAML_2,          edcMAIN_SIGNAL_FORMAT,           NULL,          CLI_COM_READ_ONLY},
    {eCLI_MAIN_MIF,     "SGFT",     WRITE_COMMAND,    DECIAML_2,          edcMAIN_SIGNAL_FORMAT,           NULL,          CLI_COM_READ_ONLY},
    {eCLI_MAIN_MIF,     "APRT",     READ_COMMAND,     STRING_CHAR,        edcMAIN_ASPECT_RATIO,            NULL,          CLI_COM_DEC2STRING|CLI_COM_READ_ONLY},   //A70LV_Doulas_0137 modify
    {eCLI_MAIN_MIF,     "APRT",     WRITE_COMMAND,    STRING_CHAR,        edcMAIN_ASPECT_RATIO,            NULL,          CLI_COM_DEC2STRING|CLI_COM_READ_ONLY},
    {eCLI_MAIN_MIF,     "RESL",     READ_COMMAND,     DECIAML_2,          edcMAIN_RESOLUTION,              NULL,          CLI_COM_READ_ONLY},
    {eCLI_MAIN_MIF,     "RESL",     WRITE_COMMAND,    DECIAML_2,          edcMAIN_RESOLUTION,              NULL,          CLI_COM_READ_ONLY},
    {eCLI_MAIN_MIF,     "VREF",     READ_COMMAND,     DECIAML_2,          edcMAIN_VERT_REFRESH,            NULL,          CLI_COM_READ_ONLY},
    {eCLI_MAIN_MIF,     "VREF",     WRITE_COMMAND,    DECIAML_2,          edcMAIN_VERT_REFRESH,            NULL,          CLI_COM_READ_ONLY},
    {eCLI_MAIN_MIF,     "HREF",     READ_COMMAND,     DECIAML_2,          edcMAIN_HORZ_REFRESH,            NULL,          CLI_COM_READ_ONLY},
    {eCLI_MAIN_MIF,     "HREF",     WRITE_COMMAND,    DECIAML_2,          edcMAIN_HORZ_REFRESH,            NULL,          CLI_COM_READ_ONLY},
    {eCLI_MAIN_MIF,     "PIXC",     READ_COMMAND,     DECIAML_2,          edcMAIN_PIXEL_CLOCK,             NULL,          CLI_COM_READ_ONLY},
    {eCLI_MAIN_MIF,     "PIXC",     WRITE_COMMAND,    DECIAML_2,          edcMAIN_PIXEL_CLOCK,             NULL,          CLI_COM_READ_ONLY},
    {eCLI_MAIN_MIF,     "SYNC",     READ_COMMAND,     DECIAML_2,          edcMAIN_SYNC_TYPE,               NULL,          CLI_COM_READ_ONLY},
    {eCLI_MAIN_MIF,     "SYNC",     WRITE_COMMAND,    DECIAML_2,          edcMAIN_SYNC_TYPE,               NULL,          CLI_COM_READ_ONLY},
    {eCLI_MAIN_MIF,     "CLSP",     READ_COMMAND,     DECIAML_2,          edcMAIN_COLOR_SPACE,             NULL,          CLI_COM_READ_ONLY},
    {eCLI_MAIN_MIF,     "CLSP",     WRITE_COMMAND,    DECIAML_2,          edcMAIN_COLOR_SPACE,             NULL,          CLI_COM_READ_ONLY},

    //SIF============================================================================================================//SIF
    {eCLI_MAIN_SIF,     "ACTS",     READ_COMMAND,     DECIAML_2,          edcSUB_INPUT,                    NULL,          CLI_COM_READ_ONLY},   //A70LV_Doulas_0136 modify
    {eCLI_MAIN_SIF,     "ACTS",     WRITE_COMMAND,    DECIAML_2,          edcSUB_INPUT,                    NULL,          CLI_COM_READ_ONLY},   //A70LV_Doulas_0136 modify
    {eCLI_MAIN_SIF,     "SGFT",     READ_COMMAND,     DECIAML_2,          edcSUB_SIGNAL_FORMAT,            NULL,          CLI_COM_READ_ONLY},
    {eCLI_MAIN_SIF,     "SGFT",     WRITE_COMMAND,    DECIAML_2,          edcSUB_SIGNAL_FORMAT,            NULL,          CLI_COM_READ_ONLY},
    {eCLI_MAIN_SIF,     "APRT",     READ_COMMAND,     STRING_CHAR,        edcSUB_ASPECT_RATIO,             NULL,          CLI_COM_DEC2STRING|CLI_COM_READ_ONLY},   //A70LV_Doulas_0137 modify
    {eCLI_MAIN_SIF,     "APRT",     WRITE_COMMAND,    STRING_CHAR,        edcSUB_ASPECT_RATIO,             NULL,          CLI_COM_DEC2STRING|CLI_COM_READ_ONLY},   //A70LV_Doulas_0137 modify
    {eCLI_MAIN_SIF,     "RESL",     READ_COMMAND,     DECIAML_2,          edcSUB_RESOLUTION,               NULL,          CLI_COM_READ_ONLY},
    {eCLI_MAIN_SIF,     "RESL",     WRITE_COMMAND,    DECIAML_2,          edcSUB_RESOLUTION,               NULL,          CLI_COM_READ_ONLY},
    {eCLI_MAIN_SIF,     "VREF",     READ_COMMAND,     DECIAML_2,          edcSUB_VERT_REFRESH,             NULL,          CLI_COM_READ_ONLY},
    {eCLI_MAIN_SIF,     "VREF",     WRITE_COMMAND,    DECIAML_2,          edcSUB_VERT_REFRESH,             NULL,          CLI_COM_READ_ONLY},
    {eCLI_MAIN_SIF,     "HREF",     READ_COMMAND,     DECIAML_2,          edcSUB_HORZ_REFRESH,             NULL,          CLI_COM_READ_ONLY},
    {eCLI_MAIN_SIF,     "HREF",     WRITE_COMMAND,    DECIAML_2,          edcSUB_HORZ_REFRESH,             NULL,          CLI_COM_READ_ONLY},
    {eCLI_MAIN_SIF,     "PIXC",     READ_COMMAND,     DECIAML_2,          edcSUB_PIXEL_CLOCK,              NULL,          CLI_COM_READ_ONLY},
    {eCLI_MAIN_SIF,     "PIXC",     WRITE_COMMAND,    DECIAML_2,          edcSUB_PIXEL_CLOCK,              NULL,          CLI_COM_READ_ONLY},
    {eCLI_MAIN_SIF,     "SYNC",     READ_COMMAND,     DECIAML_2,          edcSUB_SYNC_TYPE,                NULL,          CLI_COM_READ_ONLY},
    {eCLI_MAIN_SIF,     "SYNC",     WRITE_COMMAND,    DECIAML_2,          edcSUB_SYNC_TYPE,                NULL,          CLI_COM_READ_ONLY},
    {eCLI_MAIN_SIF,     "CLSP",     READ_COMMAND,     DECIAML_2,          edcSUB_COLOR_SPACE,              NULL,          CLI_COM_READ_ONLY},
    {eCLI_MAIN_SIF,     "CLSP",     WRITE_COMMAND,    DECIAML_2,          edcSUB_COLOR_SPACE,              NULL,          CLI_COM_READ_ONLY},

    //ICI============================================================================================================//ICI
    {eCLI_MAIN_ICI,     NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,          utilChristie_ICI,         CLI_COM_NO_REPLY},     //A70LV_Doulas_0151 modify
    {eCLI_MAIN_ICI,     NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,          utilChristie_ICI,         CLI_COM_NO_REPLY},     //A70LV_Doulas_0151 modify

    //ILI============================================================================================================//ILI
    {eCLI_MAIN_ILI,     NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,           utilChristie_ILI,         CLI_COM_NO_REPLY|CLI_COM_READ_ONLY},  //A70LV_Doulas_0140 modify
    {eCLI_MAIN_ILI,     NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,           utilChristie_ILI,         CLI_COM_NO_REPLY|CLI_COM_READ_ONLY},  //A70LV_Doulas_0140 modify

    //TTT HBPU test
    {eCLI_MAIN_TTT,     NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,       utilChristie_CLI_TTT,         CLI_COM_READ_ONLY},

    //CFU chip FW upgrade
    {eCLI_MAIN_CFU,     NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,      utilChristie_CFU,           CLI_COM_NORMAL | CLI_COM_NO_REPLY},
    {eCLI_MAIN_CFU,     NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,      utilChristie_CFU,           CLI_COM_NORMAL | CLI_COM_NO_REPLY},

    //External Flash burn
    {eCLI_MAIN_EXF,     "ADDR",     WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,      utilChristie_EXFADDR,       CLI_COM_NORMAL | CLI_COM_NO_REPLY},
    {eCLI_MAIN_EXF,     "ADDR",     READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,      utilChristie_EXFADDR,       CLI_COM_NORMAL | CLI_COM_NO_REPLY},
    {eCLI_MAIN_EXF,     "CHUM",     WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,      utilChristie_EXFCHUM,       CLI_COM_NORMAL | CLI_COM_NO_REPLY},
    {eCLI_MAIN_EXF,     "CHUM",     READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,      utilChristie_EXFCHUM,       CLI_COM_NORMAL | CLI_COM_NO_REPLY},
    {eCLI_MAIN_EXF,     "ERBL",     WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,      utilChristie_EXFERBL,       CLI_COM_NORMAL | CLI_COM_NO_REPLY},
    {eCLI_MAIN_EXF,     "ERBL",     READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,      utilChristie_EXFERBL,       CLI_COM_NORMAL | CLI_COM_NO_REPLY},
    {eCLI_MAIN_EXF,     "WREL",     WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,      utilChristie_EXFWREL,       CLI_COM_NORMAL | CLI_COM_NO_REPLY},
    {eCLI_MAIN_EXF,     "WREL",     READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,      utilChristie_EXFWREL,       CLI_COM_NORMAL | CLI_COM_NO_REPLY},

    //Burnin
    {eCLI_MAIN_BUR,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcBURN_IN,                   NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_BUR,     NULL,       READ_COMMAND,     DECIAML_2,          edcBURN_IN,                   NULL,           CLI_COM_NORMAL},

    //C821 debug
    {eCLI_MAIN_ICP,     NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,      utilChristie_ICP,          CLI_COM_NORMAL | CLI_COM_NO_REPLY},
    {eCLI_MAIN_ICP,     NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,      utilChristie_ICP,          CLI_COM_NORMAL | CLI_COM_NO_REPLY},
    {eCLI_MAIN_ICP,     "SET1",     WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,      utilChristie_ICPC821_SET,  CLI_COM_NORMAL | CLI_COM_NO_REPLY},
    {eCLI_MAIN_ICP,     "GET1",     WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,      utilChristie_ICPC821_GET,  CLI_COM_NORMAL | CLI_COM_NO_REPLY},
    {eCLI_MAIN_ICP,     "SET2",     WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,      utilChristie_ICPC789_SET,  CLI_COM_NORMAL | CLI_COM_NO_REPLY},
    {eCLI_MAIN_ICP,     "GET2",     WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,      utilChristie_ICPC789_GET,  CLI_COM_NORMAL | CLI_COM_NO_REPLY},
    {eCLI_MAIN_ICP,     "SET3",     WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,      utilChristie_ICPC341_SET,  CLI_COM_NORMAL | CLI_COM_NO_REPLY},
    {eCLI_MAIN_ICP,     "GET3",     WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,      utilChristie_ICPC341_GET,  CLI_COM_NORMAL | CLI_COM_NO_REPLY},

    //C821 debug
    //{eCLI_MAIN_ICD,     NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,      util_ICD,                    CLI_COM_NORMAL | CLI_COM_NO_REPLY},
    //{eCLI_MAIN_ICD,     NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,      util_ICD,                    CLI_COM_NORMAL | CLI_COM_NO_REPLY},

    {eCLI_MAIN_FGA,     NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,      util_FPGA_System,            CLI_COM_NORMAL | CLI_COM_NO_REPLY}, //A70LV_Larry_0086
    {eCLI_MAIN_FGA,     NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,      util_FPGA_System,            CLI_COM_NORMAL | CLI_COM_NO_REPLY}, //A70LV_Larry_0086

    //ADC Calibration
    //ADC============================================================================================================//ADC
    {eCLI_MAIN_ADC,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcADC_CALIBRAION,               NULL,        CLI_COM_NORMAL }, //A70LV_Doulas_0124
    {eCLI_MAIN_ADC,     NULL,       READ_COMMAND,     DECIAML_2,          edcADC_CALIBRAION,               NULL,        CLI_COM_NORMAL }, //A70LV_Doulas_0124

    //FCT============================================================================================================//FCT  //A70LV_Doulas_0125
    {eCLI_MAIN_FCT,     "SERN",     READ_COMMAND,     DECIAML_2,          edcSERIAL_NUMBER,                NULL,        CLI_COM_NORMAL }, //SN
    {eCLI_MAIN_FCT,     "SERN",     WRITE_COMMAND,    DECIAML_2,          edcSERIAL_NUMBER,                NULL,        CLI_COM_NORMAL | CLI_COM_NEED_SERVICE_CODE_WRITE }, //SN //A70LV_Larry_0409

    //SHU============================================================================================================//SHU  //A70LV_Doulas_0127
    {eCLI_MAIN_SHU,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcPICTURE_MUTE,                 NULL,        CLI_COM_NORMAL_AND_PN },
    {eCLI_MAIN_SHU,     NULL,       READ_COMMAND,     DECIAML_2,          edcPICTURE_MUTE,                 NULL,        CLI_COM_NORMAL_AND_PN },
    {eCLI_MAIN_SHU,     "LOCK",     WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,     utilChristie_SHU_LOCK,        CLI_COM_NORMAL }, //A70LV_Larry_0384
    {eCLI_MAIN_SHU,     "LOCK",     READ_COMMAND,     DECIAML_2,          edcLENS_DETECTION,               NULL,        CLI_COM_NORMAL }, //A70LV_Larry_0384

    //UID============================================================================================================//UID  //A70LV_Doulas_0138
    {eCLI_MAIN_UID,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcSERVICE_MODE_FLAG,            NULL,       CLI_COM_NORMAL },
    {eCLI_MAIN_UID,     NULL,       READ_COMMAND,     DECIAML_2,          edcSERVICE_MODE_FLAG,            NULL,       CLI_COM_NORMAL },

    //SIV============================================================================================================//SIV          //A70LV_Doulas_0139
    {eCLI_MAIN_SIV,     NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,           utilChristie_SIV,       CLI_COM_READ_ONLY },
    {eCLI_MAIN_SIV,     NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,           utilChristie_SIV,       CLI_COM_READ_ONLY },

    //LCE============================================================================================================//LCE          //A70LV_John_0034 add LCE command
    {eCLI_MAIN_LCE,     NULL,       READ_COMMAND,     DECIAML_3,          edcLAST_SERIAL_CMD_ERRORCODE,    NULL,       CLI_COM_READ_ONLY },

    //LSE============================================================================================================//LSE          //A70LV_John_0035 add LSE command
    //{eCLI_MAIN_LSE,     NULL,       READ_COMMAND,     DECIAML_3,          edcLAST_ERRORCODE,               NULL,       CLI_COM_READ_ONLY },

    //OSD============================================================================================================//OSD          //A70LV_Doulas_0163
    {eCLI_MAIN_OSD,     NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,           utilChristie_OSD,       CLI_COM_NORMAL },
    {eCLI_MAIN_OSD,     NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,           utilChristie_OSD,       CLI_COM_NORMAL },

    //SNS
    {eCLI_MAIN_SNS,     "SRC0",     WRITE_COMMAND,    DECIAML_2,          edcSOURCE0_NAME,                 NULL,        CLI_COM_READ_ONLY },        //A70LV_Doulas_0231 modify//T100_Simon_0006
    {eCLI_MAIN_SNS,     "SRC0",     READ_COMMAND,     DECIAML_2,          edcSOURCE0_NAME,                 NULL,        CLI_COM_READ_ONLY },        //A70LV_Doulas_0231 modify//T100_Simon_0006
    {eCLI_MAIN_SNS,     "SRC1",     WRITE_COMMAND,    DECIAML_2,          edcSOURCE1_NAME,                 NULL,        CLI_COM_READ_ONLY },        //T100_Simon_0006
    {eCLI_MAIN_SNS,     "SRC1",     READ_COMMAND,     DECIAML_2,          edcSOURCE1_NAME,                 NULL,        CLI_COM_READ_ONLY },        //T100_Simon_0006
    {eCLI_MAIN_SNS,     "SRC2",     WRITE_COMMAND,    DECIAML_2,          edcSOURCE2_NAME,                 NULL,        CLI_COM_READ_ONLY },        //A70LV_Doulas_0231 modify//T100_Simon_0006
    {eCLI_MAIN_SNS,     "SRC2",     READ_COMMAND,     DECIAML_2,          edcSOURCE2_NAME,                 NULL,        CLI_COM_READ_ONLY },        //A70LV_Doulas_0231 modify//T100_Simon_0006
    {eCLI_MAIN_SNS,     "SRC3",     WRITE_COMMAND,    DECIAML_2,          edcSOURCE3_NAME,                 NULL,        CLI_COM_NORMAL },        //T100_Simon_0006
    {eCLI_MAIN_SNS,     "SRC3",     READ_COMMAND,     DECIAML_2,          edcSOURCE3_NAME,                 NULL,        CLI_COM_NORMAL },        //T100_Simon_0006
    {eCLI_MAIN_SNS,     "SRC4",     WRITE_COMMAND,    DECIAML_2,          edcSOURCE4_NAME,                 NULL,        CLI_COM_NORMAL },        //T100_Simon_0006
    {eCLI_MAIN_SNS,     "SRC4",     READ_COMMAND,     DECIAML_2,          edcSOURCE4_NAME,                 NULL,        CLI_COM_NORMAL },        //T100_Simon_0006
    {eCLI_MAIN_SNS,     "SRC5",     WRITE_COMMAND,    DECIAML_2,          edcSOURCE5_NAME,                 NULL,        CLI_COM_READ_ONLY },        //T100_Simon_0006
    {eCLI_MAIN_SNS,     "SRC5",     READ_COMMAND,     DECIAML_2,          edcSOURCE5_NAME,                 NULL,        CLI_COM_READ_ONLY },        //T100_Simon_0006
    {eCLI_MAIN_SNS,     "SRC6",     WRITE_COMMAND,    DECIAML_2,          edcSOURCE6_NAME,                 NULL,        CLI_COM_NORMAL },        //A70LV_Doulas_0231 modify//T100_Simon_0006
    {eCLI_MAIN_SNS,     "SRC6",     READ_COMMAND,     DECIAML_2,          edcSOURCE6_NAME,                 NULL,        CLI_COM_NORMAL },        //A70LV_Doulas_0231 modify//T100_Simon_0006
    {eCLI_MAIN_SNS,     "SRC7",     WRITE_COMMAND,    DECIAML_2,          edcSOURCE7_NAME,                 NULL,        CLI_COM_NORMAL },        //T100_Simon_0006
    {eCLI_MAIN_SNS,     "SRC7",     READ_COMMAND,     DECIAML_2,          edcSOURCE7_NAME,                 NULL,        CLI_COM_NORMAL },        //T100_Simon_0006
    {eCLI_MAIN_SNS,     "SRC8",     WRITE_COMMAND,    DECIAML_2,          edcSOURCE8_NAME,                 NULL,        CLI_COM_NORMAL },        //T100_Simon_0006
    {eCLI_MAIN_SNS,     "SRC8",     READ_COMMAND,     DECIAML_2,          edcSOURCE8_NAME,                 NULL,        CLI_COM_NORMAL },        //T100_Simon_0006
    {eCLI_MAIN_SNS,     "SRC9",     WRITE_COMMAND,    DECIAML_2,          edcSOURCE9_NAME,                 NULL,        CLI_COM_READ_ONLY },        //A70LV_Doulas_0231 modify//T100_Simon_0006
    {eCLI_MAIN_SNS,     "SRC9",     READ_COMMAND,     DECIAML_2,          edcSOURCE9_NAME,                 NULL,        CLI_COM_READ_ONLY },        //A70LV_Doulas_0231 modify//T100_Simon_0006
    {eCLI_MAIN_SNS,     "SRCA",     WRITE_COMMAND,    DECIAML_2,          edcSOURCE10_NAME,                NULL,        CLI_COM_READ_ONLY },        //A70LV_Doulas_0231 modify//T100_Simon_0006
    {eCLI_MAIN_SNS,     "SRCA",     READ_COMMAND,     DECIAML_2,          edcSOURCE10_NAME,                NULL,        CLI_COM_READ_ONLY },        //A70LV_Doulas_0231 modify//T100_Simon_0006
    {eCLI_MAIN_SNS,     "SRCB",     WRITE_COMMAND,    DECIAML_2,          edcSOURCE11_NAME,                NULL,        CLI_COM_READ_ONLY },        //A70LV_Doulas_0231 modify//T100_Simon_0006
    {eCLI_MAIN_SNS,     "SRCB",     READ_COMMAND,     DECIAML_2,          edcSOURCE11_NAME,                NULL,        CLI_COM_READ_ONLY },        //A70LV_Doulas_0231 modify//T100_Simon_0006
    //{eCLI_MAIN_SNS,     "SRCC",     WRITE_COMMAND,    DECIAML_2,          edcSOURCE12_NAME,                NULL,        CLI_COM_NORMAL },        //A70LV_Doulas_0231 removremovee//T100_Simon_0006
    //{eCLI_MAIN_SNS,     "SRCC",     READ_COMMAND,     DECIAML_2,          edcSOURCE12_NAME,                NULL,        CLI_COM_NORMAL },        //A70LV_Doulas_0231 remove//T100_Simon_0006
    //{eCLI_MAIN_SNS,     "SRCD",     WRITE_COMMAND,    DECIAML_2,          edcSOURCE13_NAME,                NULL,        CLI_COM_NORMAL },        //A70LV_Doulas_0231 remove//T100_Simon_0006
    //{eCLI_MAIN_SNS,     "SRCD",     READ_COMMAND,     DECIAML_2,          edcSOURCE13_NAME,                NULL,        CLI_COM_NORMAL },        //A70LV_Doulas_0231 remove//T100_Simon_0006
    //{eCLI_MAIN_SNS,     "SRCE",     WRITE_COMMAND,    DECIAML_2,          edcSOURCE14_NAME,                NULL,        CLI_COM_NORMAL },        //A70LV_Doulas_0231 remove//T100_Simon_0006
    //{eCLI_MAIN_SNS,     "SRCE",     READ_COMMAND,     DECIAML_2,          edcSOURCE14_NAME,                NULL,        CLI_COM_NORMAL },        //A70LV_Doulas_0231 remove//T100_Simon_0006

    //LET============================================================================================================//LET
    {eCLI_MAIN_LET,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcLENS_TYPE,                    NULL,        CLI_COM_READ_ONLY },
    {eCLI_MAIN_LET,     NULL,       READ_COMMAND,     DECIAML_2,          edcLENS_TYPE,                    NULL,        CLI_COM_READ_ONLY },
    //{eCLI_MAIN_LET,     NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,         utilChristie_LET,        CLI_COM_NO_REPLY|CLI_COM_READ_ONLY},

    //LRR============================================================================================================//LRR
    {eCLI_MAIN_LRR,     NULL,       WRITE_COMMAND,    DECIAML_2,          edc3D_LR_REFERENCE,              NULL,        CLI_COM_NORMAL_AND_PN },
    {eCLI_MAIN_LRR,     NULL,       READ_COMMAND,     DECIAML_2,          edc3D_LR_REFERENCE,              NULL,        CLI_COM_NORMAL_AND_PN },

    //FVI============================================================================================================//FVI
    {eCLI_MAIN_FVI,     NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,         utilChristie_FVI,         CLI_COM_NO_REPLY|CLI_COM_READ_ONLY},    //A70LV_Doulas_0140 modify
    {eCLI_MAIN_FVI,     NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,         utilChristie_FVI,         CLI_COM_NO_REPLY|CLI_COM_READ_ONLY},    //A70LV_Doulas_0140 modify

    //FVI============================================================================================================//FVI
    {eCLI_MAIN_FVI,     "ALLV",       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,         utilChristie_FVI,         CLI_COM_NO_REPLY|CLI_COM_READ_ONLY},    //A35Gen2_Coda modify
    {eCLI_MAIN_FVI,     "ALLV",       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,         utilChristie_FVI,         CLI_COM_NO_REPLY|CLI_COM_READ_ONLY},    //A35Gen2_Coda modify

    //STY============================================================================================================//FVI
    {eCLI_MAIN_STY,     NULL,       READ_COMMAND,     DECIAML_2,          edcCUSTOMER_SPLASH,              NULL,        CLI_COM_NORMAL | CLI_COM_NEED_SERVICE_CODE_WRITE}, //A70LV_Larry_0235
    {eCLI_MAIN_STY,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcCUSTOMER_SPLASH,              NULL,        CLI_COM_NORMAL | CLI_COM_NEED_SERVICE_CODE_WRITE}, //A70LV_Larry_0235

    //{eCLI_MAIN_STY,     "STAN",     READ_COMMAND,     DECIAML_2,          edcSTANDARD,                     NULL,        CLI_COM_NORMAL | CLI_COM_NEED_SERVICE_CODE_WRITE}, //A70LV_Larry_0439
    //{eCLI_MAIN_STY,     "STAN",     WRITE_COMMAND,    DECIAML_2,          edcSTANDARD,                     NULL,        CLI_COM_NORMAL | CLI_COM_NEED_SERVICE_CODE_WRITE}, //A70LV_Larry_0439

    //LOT============================================================================================================//LOT
    {eCLI_MAIN_LOT,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcLIGHTS_OUT_TIMER_X05,         NULL,        CLI_COM_NORMAL },   //A70LV_Doulas_0228
    {eCLI_MAIN_LOT,     NULL,       READ_COMMAND,     DECIAML_2,          edcLIGHTS_OUT_TIMER_X05,         NULL,        CLI_COM_NORMAL },   //A70LV_Doulas_0228

    //LOL============================================================================================================//LOL
    {eCLI_MAIN_LOL,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcLIGHTS_OUT_SIGNAL_LEVEL,      NULL,        CLI_COM_NORMAL },   //A70LV_Doulas_0228
    {eCLI_MAIN_LOL,     NULL,       READ_COMMAND,     DECIAML_2,          edcLIGHTS_OUT_SIGNAL_LEVEL,      NULL,        CLI_COM_NORMAL },   //A70LV_Doulas_0228

	//LTL============================================================================================================//LTL
    {eCLI_MAIN_LTL,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcLIGHTS_ON_THRESHOLD,          NULL,        CLI_COM_NORMAL },   //A70Gen2_Doulas_0044
    {eCLI_MAIN_LTL,     NULL,       READ_COMMAND,     DECIAML_2,          edcLIGHTS_ON_THRESHOLD,      	   NULL,        CLI_COM_NORMAL }, 	//A70Gen2_Doulas_0044

    //FKC============================================================================================================//FKC
    {eCLI_MAIN_FKC,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcCOMPATIBLE_4K,                NULL,        CLI_COM_NORMAL_AND_PN },   //A70LV_Larry_0283
    {eCLI_MAIN_FKC,     NULL,       READ_COMMAND,     DECIAML_2,          edcCOMPATIBLE_4K,                NULL,        CLI_COM_NORMAL_AND_PN },   //A70LV_Larry_0283
#if 0
    //CCT============================================================================================================//CCT
    {eCLI_MAIN_CCT,     NULL,       WRITE_COMMAND,    DECIAML_3,          edcCOLOR_OFFSET,                 NULL,        CLI_COM_NORMAL | CLI_COM_NEED_SERVICE_CODE_WRITE},
    {eCLI_MAIN_CCT,     NULL,       READ_COMMAND,     DECIAML_3,          edcCOLOR_OFFSET,                 NULL,        CLI_COM_NORMAL | CLI_COM_NEED_SERVICE_CODE_READ},
    {eCLI_MAIN_CCT,     "REST",     WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,       utilChristie_CLI_CCT_REST,        CLI_COM_WRITE_ONLY | CLI_COM_NEED_SERVICE_CODE_WRITE},
    {eCLI_MAIN_CCT,     "REST",     READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,       utilChristie_CLI_CCT_REST,        CLI_COM_WRITE_ONLY | CLI_COM_NEED_SERVICE_CODE_READ},
#endif
    //BAC============================================================================================================//BAC
    {eCLI_MAIN_BAC,     "SAVE",     WRITE_COMMAND,    DECIAML_2,          edcBACKUP_RESTORE_SAVE,          NULL,  CLI_COM_NORMAL_AND_PN | CLI_COM_CHECK_BUSY},
    {eCLI_MAIN_BAC,     "SAVE",     READ_COMMAND,     DECIAML_2,          edcBACKUP_RESTORE_SAVE,          NULL,  CLI_COM_NORMAL_AND_PN},
    {eCLI_MAIN_BAC,     "REST",     WRITE_COMMAND,    DECIAML_2,          edcBACKUP_RESTORE_RESTORE,       NULL,  CLI_COM_NORMAL_AND_PN | CLI_COM_CHECK_BUSY},
    {eCLI_MAIN_BAC,     "REST",     READ_COMMAND,     DECIAML_2,          edcBACKUP_RESTORE_RESTORE,       NULL,  CLI_COM_NORMAL_AND_PN},

    //BSS============================================================================================================//BSS
    {eCLI_MAIN_BSS,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcBLANK_SIGNAL_SWITCH,          NULL,        CLI_COM_NORMAL_AND_PN },   //A70LV_Doulas_0294
    {eCLI_MAIN_BSS,     NULL,       READ_COMMAND,     DECIAML_2,          edcBLANK_SIGNAL_SWITCH,          NULL,        CLI_COM_NORMAL_AND_PN },   //A70LV_Doulas_0294

    //SPD============================================================================================================//SPD
    {eCLI_MAIN_SPD,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcDB_SPEED,                     NULL,        CLI_COM_NORMAL },   //A70LV_Doulas_0327
    {eCLI_MAIN_SPD,     NULL,       READ_COMMAND,     DECIAML_2,          edcDB_SPEED,                     NULL,        CLI_COM_NORMAL },   //A70LV_Doulas_0327

    //STH============================================================================================================//STH
    {eCLI_MAIN_STH,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcDB_STRENGTH,                  NULL,        CLI_COM_NORMAL },   //A70LV_Doulas_0327
    {eCLI_MAIN_STH,     NULL,       READ_COMMAND,     DECIAML_2,          edcDB_STRENGTH,                  NULL,        CLI_COM_NORMAL },   //A70LV_Doulas_0327

    //DLL============================================================================================================//DLL
    {eCLI_MAIN_DLL,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcDB_LIGHT_LEVEL,               NULL,        CLI_COM_NORMAL },   //A70LV_Doulas_0327
    {eCLI_MAIN_DLL,     NULL,       READ_COMMAND,     DECIAML_2,          edcDB_LIGHT_LEVEL,               NULL,        CLI_COM_NORMAL },   //A70LV_Doulas_0327

    //DRB============================================================================================================//DRB
    {eCLI_MAIN_DRB,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcDB_REAL_BLACK_ENABLE,         NULL,        CLI_COM_NORMAL },   //A70LV_Doulas_0327
    {eCLI_MAIN_DRB,     NULL,       READ_COMMAND,     DECIAML_2,          edcDB_REAL_BLACK_ENABLE,         NULL,        CLI_COM_NORMAL },   //A70LV_Doulas_0327
//--------------
    //TCM============================================================================================================//DTCM
    {eCLI_MAIN_TCM,     NULL,       READ_COMMAND,     DECIAML_2,          edcDATE_MODE,    	NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_TCM,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcDATE_MODE,     NULL,           CLI_COM_NORMAL},

    //DDY============================================================================================================//DTSY
    {eCLI_MAIN_DDY,     NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,    	utilChristie_Manual_Date_Time,  CLI_COM_NORMAL},//HICC2_Julie_0039
    {eCLI_MAIN_DDY,     NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,    	utilChristie_Manual_Date_Time,  CLI_COM_NORMAL},//HICC2_Julie_0039

    //DDM============================================================================================================//DTSM
    {eCLI_MAIN_DDM,     NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,    	utilChristie_Manual_Date_Time,  CLI_COM_NORMAL},//HICC2_Julie_0039
    {eCLI_MAIN_DDM,     NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,    	utilChristie_Manual_Date_Time,  CLI_COM_NORMAL},//HICC2_Julie_0039

    //DDD============================================================================================================//DTSD
    {eCLI_MAIN_DDD,     NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,    	utilChristie_Manual_Date_Time,  CLI_COM_NORMAL},//HICC2_Julie_0039
    {eCLI_MAIN_DDD,     NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,    	utilChristie_Manual_Date_Time,  CLI_COM_NORMAL},//HICC2_Julie_0039

    //DTH============================================================================================================//DTTH
    {eCLI_MAIN_DTH,     NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,    	utilChristie_Manual_Date_Time,  CLI_COM_NORMAL},//HICC2_Julie_0039
    {eCLI_MAIN_DTH,     NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,    	utilChristie_Manual_Date_Time,  CLI_COM_NORMAL},//HICC2_Julie_0039

    //DTM============================================================================================================//DTTM
    {eCLI_MAIN_DTM,     NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,    	utilChristie_Manual_Date_Time,  CLI_COM_NORMAL},//HICC2_Julie_0039
    {eCLI_MAIN_DTM,     NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,    	utilChristie_Manual_Date_Time,  CLI_COM_NORMAL},//HICC2_Julie_0039

    //DST============================================================================================================//DTDS
    {eCLI_MAIN_DST,     NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,    	utilChristie_UseNTPServer_CMD,  CLI_COM_NORMAL},//HICC2_Julie_0039
    {eCLI_MAIN_DST,     NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,     utilChristie_UseNTPServer_CMD,  CLI_COM_NORMAL},//HICC2_Julie_0039

    //NTP============================================================================================================//DTNS
    {eCLI_MAIN_NTP,     NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,    	utilChristie_UseNTPServer_CMD,  CLI_COM_NORMAL},//HICC2_Julie_0039
    {eCLI_MAIN_NTP,     NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,     utilChristie_UseNTPServer_CMD,  CLI_COM_NORMAL},//HICC2_Julie_0039

    //UTC============================================================================================================//DTTZ
    {eCLI_MAIN_UTC,     NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,    	utilChristie_UseNTPServer_CMD,  CLI_COM_NORMAL},//HICC2_Julie_0039
    {eCLI_MAIN_UTC,     NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,     utilChristie_UseNTPServer_CMD,  CLI_COM_NORMAL},//HICC2_Julie_0039

    //DUI============================================================================================================//DTUI
    {eCLI_MAIN_DUI,     NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,    	utilChristie_UseNTPServer_CMD,  CLI_COM_NORMAL},//HICC2_Julie_0039
    {eCLI_MAIN_DUI,     NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,     utilChristie_UseNTPServer_CMD,  CLI_COM_NORMAL},//HICC2_Julie_0039

    //DTS============================================================================================================//DTAP
    {eCLI_MAIN_DTS,     NULL,       READ_COMMAND,     DECIAML_2,          edcDATE_APPLY,     NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_DTS,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcDATE_APPLY,     NULL,           CLI_COM_NORMAL},

    //SDT============================================================================================================//DTIF
    {eCLI_MAIN_SDT,     NULL,      READ_COMMAND,     DECIAML_2,    		edcREAL_DATE_TIME,   		NULL,         	CLI_COM_READ_ONLY}, //G100_Coda_0047

    //SCME============================================================================================================//SCME
    {eCLI_MAIN_SCH,     "MODE",       READ_COMMAND,     DECIAML_2,          edcSCHEDULE_MODE,    	NULL,           CLI_COM_NORMAL}, //G100_Coda_0049
    {eCLI_MAIN_SCH,     "MODE",       WRITE_COMMAND,    DECIAML_2,          edcSCHEDULE_MODE,     NULL,           CLI_COM_NORMAL},

    //SCTD============================================================================================================//SCTD
    {eCLI_MAIN_SCH,     "WDAY",        READ_COMMAND,     DECIAML_2,    		DATA_CODE_NA,   		utilChristie_SCHE_ShowToday,      CLI_COM_NO_REPLY|CLI_COM_READ_ONLY}, //G100_Coda_0049


    //SCW1+MODE============================================================================================================//SCW1+MODE
    {eCLI_MAIN_SD1,    "MODE",         READ_COMMAND,     DECIAML_2,      edcSCHEDULE_MONDAY_ENABLE,  NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_SD1,    "MODE",         WRITE_COMMAND,    DECIAML_2,      edcSCHEDULE_MONDAY_ENABLE,  NULL,          CLI_COM_NORMAL},

    //SCW2+MODE============================================================================================================//SCW2+MODE
    {eCLI_MAIN_SD2,    "MODE",         READ_COMMAND,     DECIAML_2,      edcSCHEDULE_TUESDAY_ENABLE,  NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_SD2,    "MODE",         WRITE_COMMAND,    DECIAML_2,      edcSCHEDULE_TUESDAY_ENABLE,  NULL,          CLI_COM_NORMAL},

    //SCW3+MODE============================================================================================================//SCW3+MODE
    {eCLI_MAIN_SD3,    "MODE",         READ_COMMAND,     DECIAML_2,      edcSCHEDULE_WEDNESDAY_ENABLE,  NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_SD3,    "MODE",         WRITE_COMMAND,    DECIAML_2,      edcSCHEDULE_WEDNESDAY_ENABLE,  NULL,          CLI_COM_NORMAL},

    //SCW4+MODE============================================================================================================//SCW4+MODE
    {eCLI_MAIN_SD4,    "MODE",         READ_COMMAND,     DECIAML_2,      edcSCHEDULE_THURSDAY_ENABLE,  NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_SD4,    "MODE",         WRITE_COMMAND,    DECIAML_2,      edcSCHEDULE_THURSDAY_ENABLE,  NULL,          CLI_COM_NORMAL},

    //SCW5+MODE============================================================================================================//SCW5+MODE
    {eCLI_MAIN_SD5,    "MODE",         READ_COMMAND,     DECIAML_2,      edcSCHEDULE_FRIDAY_ENABLE,  NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_SD5,    "MODE",         WRITE_COMMAND,    DECIAML_2,      edcSCHEDULE_FRIDAY_ENABLE,  NULL,          CLI_COM_NORMAL},

    //SCW6+MODE============================================================================================================//SCW6+MODE
    {eCLI_MAIN_SD6,    "MODE",         READ_COMMAND,     DECIAML_2,      edcSCHEDULE_SATURDAY_ENABLE,  NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_SD6,    "MODE",         WRITE_COMMAND,    DECIAML_2,      edcSCHEDULE_SATURDAY_ENABLE,  NULL,          CLI_COM_NORMAL},

    //SCW0+MODE============================================================================================================//SCW0+MODE
    {eCLI_MAIN_SD0,    "MODE",         READ_COMMAND,     DECIAML_2,      edcSCHEDULE_SUNDAY_ENABLE,  NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_SD0,    "MODE",         WRITE_COMMAND,    DECIAML_2,      edcSCHEDULE_SUNDAY_ENABLE,  NULL,          CLI_COM_NORMAL},


    //SCW1+EVRT============================================================================================================//SCW1+EVRT
    {eCLI_MAIN_SD1,    "EVRT",         READ_COMMAND,     DECIAML_2,      DATA_CODE_NA,  utilChristie_SCW_EVRT,          CLI_COM_WRITE_ONLY},
    {eCLI_MAIN_SD1,    "EVRT",         WRITE_COMMAND,    DECIAML_2,      DATA_CODE_NA,  utilChristie_SCW_EVRT,          CLI_COM_NORMAL},

    //SCW2+EVRT============================================================================================================//SCW2+EVRT
    {eCLI_MAIN_SD2,    "EVRT",         READ_COMMAND,     DECIAML_2,      DATA_CODE_NA,  utilChristie_SCW_EVRT,          CLI_COM_WRITE_ONLY},
    {eCLI_MAIN_SD2,    "EVRT",         WRITE_COMMAND,    DECIAML_2,      DATA_CODE_NA,  utilChristie_SCW_EVRT,          CLI_COM_NORMAL},

    //SCW3+EVRT============================================================================================================//SCW3+EVRT
    {eCLI_MAIN_SD3,    "EVRT",         READ_COMMAND,     DECIAML_2,      DATA_CODE_NA,  utilChristie_SCW_EVRT,          CLI_COM_WRITE_ONLY},
    {eCLI_MAIN_SD3,    "EVRT",         WRITE_COMMAND,    DECIAML_2,      DATA_CODE_NA,  utilChristie_SCW_EVRT,          CLI_COM_NORMAL},

    //SCW4+EVRT============================================================================================================//SCW4+EVRT
    {eCLI_MAIN_SD4,    "EVRT",         READ_COMMAND,     DECIAML_2,      DATA_CODE_NA,  utilChristie_SCW_EVRT,          CLI_COM_WRITE_ONLY},
    {eCLI_MAIN_SD4,    "EVRT",         WRITE_COMMAND,    DECIAML_2,      DATA_CODE_NA,  utilChristie_SCW_EVRT,          CLI_COM_NORMAL},

    //SCW5+EVRT============================================================================================================//SCW5+EVRT
    {eCLI_MAIN_SD5,    "EVRT",         READ_COMMAND,     DECIAML_2,      DATA_CODE_NA,  utilChristie_SCW_EVRT,          CLI_COM_WRITE_ONLY},
    {eCLI_MAIN_SD5,    "EVRT",         WRITE_COMMAND,    DECIAML_2,      DATA_CODE_NA,  utilChristie_SCW_EVRT,          CLI_COM_NORMAL},

    //SCW6+EVRT============================================================================================================//SCW6+EVRT
    {eCLI_MAIN_SD6,    "EVRT",         READ_COMMAND,     DECIAML_2,      DATA_CODE_NA,  utilChristie_SCW_EVRT,          CLI_COM_WRITE_ONLY},
    {eCLI_MAIN_SD6,    "EVRT",         WRITE_COMMAND,    DECIAML_2,      DATA_CODE_NA,  utilChristie_SCW_EVRT,          CLI_COM_NORMAL},

    //SCW0+EVRT============================================================================================================//SCW0+EVRT
    {eCLI_MAIN_SD0,    "EVRT",         READ_COMMAND,     DECIAML_2,      DATA_CODE_NA,  utilChristie_SCW_EVRT,          CLI_COM_WRITE_ONLY},
    {eCLI_MAIN_SD0,    "EVRT",         WRITE_COMMAND,    DECIAML_2,      DATA_CODE_NA,  utilChristie_SCW_EVRT,          CLI_COM_NORMAL},


    //SCW1+REST============================================================================================================//SCW1+REST
    {eCLI_MAIN_SD1,    "REST",         READ_COMMAND,     DECIAML_2,      DATA_CODE_NA,  utilChristie_SCW_REST,          CLI_COM_WRITE_ONLY},
    {eCLI_MAIN_SD1,    "REST",         WRITE_COMMAND,    DECIAML_2,      DATA_CODE_NA,  utilChristie_SCW_REST,          CLI_COM_NORMAL},

    //SCW2+REST============================================================================================================//SCW2+REST
    {eCLI_MAIN_SD2,    "REST",         READ_COMMAND,     DECIAML_2,      DATA_CODE_NA,  utilChristie_SCW_REST,          CLI_COM_WRITE_ONLY},
    {eCLI_MAIN_SD2,    "REST",         WRITE_COMMAND,    DECIAML_2,      DATA_CODE_NA,  utilChristie_SCW_REST,          CLI_COM_NORMAL},

    //SCW3+REST============================================================================================================//SCW3+REST
    {eCLI_MAIN_SD3,    "REST",         READ_COMMAND,     DECIAML_2,      DATA_CODE_NA,  utilChristie_SCW_REST,          CLI_COM_WRITE_ONLY},
    {eCLI_MAIN_SD3,    "REST",         WRITE_COMMAND,    DECIAML_2,      DATA_CODE_NA,  utilChristie_SCW_REST,          CLI_COM_NORMAL},

    //SCW4+REST============================================================================================================//SCW4+REST
    {eCLI_MAIN_SD4,    "REST",         READ_COMMAND,     DECIAML_2,      DATA_CODE_NA,  utilChristie_SCW_REST,          CLI_COM_WRITE_ONLY},
    {eCLI_MAIN_SD4,    "REST",         WRITE_COMMAND,    DECIAML_2,      DATA_CODE_NA,  utilChristie_SCW_REST,          CLI_COM_NORMAL},

    //SCW5+REST============================================================================================================//SCW5+REST
    {eCLI_MAIN_SD5,    "REST",         READ_COMMAND,     DECIAML_2,      DATA_CODE_NA,  utilChristie_SCW_REST,          CLI_COM_WRITE_ONLY},
    {eCLI_MAIN_SD5,    "REST",         WRITE_COMMAND,    DECIAML_2,      DATA_CODE_NA,  utilChristie_SCW_REST,          CLI_COM_NORMAL},

    //SCW6+REST============================================================================================================//SCW6+REST
    {eCLI_MAIN_SD6,    "REST",         READ_COMMAND,     DECIAML_2,      DATA_CODE_NA,  utilChristie_SCW_REST,          CLI_COM_WRITE_ONLY},
    {eCLI_MAIN_SD6,    "REST",         WRITE_COMMAND,    DECIAML_2,      DATA_CODE_NA,  utilChristie_SCW_REST,          CLI_COM_NORMAL},

    //SCW0+REST============================================================================================================//SCW0+REST
    {eCLI_MAIN_SD0,    "REST",         READ_COMMAND,     DECIAML_2,      DATA_CODE_NA,  utilChristie_SCW_REST,          CLI_COM_WRITE_ONLY},
    {eCLI_MAIN_SD0,    "REST",         WRITE_COMMAND,    DECIAML_2,      DATA_CODE_NA,  utilChristie_SCW_REST,          CLI_COM_NORMAL},


    //SCW1+CPWD============================================================================================================//SCW1+CPWD
    {eCLI_MAIN_SD1,    "CPWD",         READ_COMMAND,     DECIAML_2,      DATA_CODE_NA,  utilChristie_SCW_CPWD,          CLI_COM_NORMAL},
    {eCLI_MAIN_SD1,    "CPWD",         WRITE_COMMAND,    DECIAML_2,      DATA_CODE_NA,  utilChristie_SCW_CPWD,          CLI_COM_NORMAL},

    //SCW2+CPWD============================================================================================================//SCW2+CPWD
    {eCLI_MAIN_SD2,    "CPWD",         READ_COMMAND,     DECIAML_2,      DATA_CODE_NA,  utilChristie_SCW_CPWD,          CLI_COM_NORMAL},
    {eCLI_MAIN_SD2,    "CPWD",         WRITE_COMMAND,    DECIAML_2,      DATA_CODE_NA,  utilChristie_SCW_CPWD,          CLI_COM_NORMAL},

    //SCW3+REST============================================================================================================//SCW3+CPWD
    {eCLI_MAIN_SD3,    "CPWD",         READ_COMMAND,     DECIAML_2,      DATA_CODE_NA,  utilChristie_SCW_CPWD,          CLI_COM_NORMAL},
    {eCLI_MAIN_SD3,    "CPWD",         WRITE_COMMAND,    DECIAML_2,      DATA_CODE_NA,  utilChristie_SCW_CPWD,          CLI_COM_NORMAL},

    //SCW4+REST============================================================================================================//SCW4+CPWD
    {eCLI_MAIN_SD4,    "CPWD",         READ_COMMAND,     DECIAML_2,      DATA_CODE_NA,  utilChristie_SCW_CPWD,          CLI_COM_NORMAL},
    {eCLI_MAIN_SD4,    "CPWD",         WRITE_COMMAND,    DECIAML_2,      DATA_CODE_NA,  utilChristie_SCW_CPWD,          CLI_COM_NORMAL},

    //SCW5+REST============================================================================================================//SCW5+CPWD
    {eCLI_MAIN_SD5,    "CPWD",         READ_COMMAND,     DECIAML_2,      DATA_CODE_NA,  utilChristie_SCW_CPWD,          CLI_COM_NORMAL},
    {eCLI_MAIN_SD5,    "CPWD",         WRITE_COMMAND,    DECIAML_2,      DATA_CODE_NA,  utilChristie_SCW_CPWD,          CLI_COM_NORMAL},

    //SCW6+REST============================================================================================================//SCW6+CPWD
    {eCLI_MAIN_SD6,    "CPWD",         READ_COMMAND,     DECIAML_2,      DATA_CODE_NA,  utilChristie_SCW_CPWD,          CLI_COM_NORMAL},
    {eCLI_MAIN_SD6,    "CPWD",         WRITE_COMMAND,    DECIAML_2,      DATA_CODE_NA,  utilChristie_SCW_CPWD,          CLI_COM_NORMAL},

    //SCW0+REST============================================================================================================//SCW0+CPWD
    {eCLI_MAIN_SD0,    "CPWD",         READ_COMMAND,     DECIAML_2,      DATA_CODE_NA,  utilChristie_SCW_CPWD,          CLI_COM_NORMAL},
    {eCLI_MAIN_SD0,    "CPWD",         WRITE_COMMAND,    DECIAML_2,      DATA_CODE_NA,  utilChristie_SCW_CPWD,          CLI_COM_NORMAL},

//G100_Coda_0047
    //SCW1+EVWR============================================================================================================//SCW1+EVWR
    {eCLI_MAIN_SD1,    "EVRD",         READ_COMMAND,     DECIAML_2,      DATA_CODE_NA,  utilChristie_SCW_EVRD,          CLI_COM_NO_REPLY|CLI_COM_READ_ONLY},
    {eCLI_MAIN_SD1,    "EVWR",         WRITE_COMMAND,    DECIAML_2,      DATA_CODE_NA,  utilChristie_SCW_EVWR,          CLI_COM_WRITE_ONLY},

    //SCW2+EVWR============================================================================================================//SCW2+EVWR
    {eCLI_MAIN_SD2,    "EVRD",         READ_COMMAND,     DECIAML_2,      DATA_CODE_NA,  utilChristie_SCW_EVRD,          CLI_COM_NO_REPLY|CLI_COM_READ_ONLY},
    {eCLI_MAIN_SD2,    "EVWR",         WRITE_COMMAND,    DECIAML_2,      DATA_CODE_NA,  utilChristie_SCW_EVWR,          CLI_COM_WRITE_ONLY},

    //SCW3+EVWR============================================================================================================//SCW3+EVWR
    {eCLI_MAIN_SD3,    "EVRD",         READ_COMMAND,     DECIAML_2,      DATA_CODE_NA,  utilChristie_SCW_EVRD,          CLI_COM_NO_REPLY|CLI_COM_READ_ONLY},
    {eCLI_MAIN_SD3,    "EVWR",         WRITE_COMMAND,    DECIAML_2,      DATA_CODE_NA,  utilChristie_SCW_EVWR,          CLI_COM_WRITE_ONLY},

    //SCW4+EVWR============================================================================================================//SCW4+EVWR
    {eCLI_MAIN_SD4,    "EVRD",         READ_COMMAND,     DECIAML_2,      DATA_CODE_NA,  utilChristie_SCW_EVRD,          CLI_COM_NO_REPLY|CLI_COM_READ_ONLY},
    {eCLI_MAIN_SD4,    "EVWR",         WRITE_COMMAND,    DECIAML_2,      DATA_CODE_NA,  utilChristie_SCW_EVWR,          CLI_COM_WRITE_ONLY},

    //SCW5+EVWR============================================================================================================//SCW5+EVWR
    {eCLI_MAIN_SD5,    "EVRD",         READ_COMMAND,     DECIAML_2,      DATA_CODE_NA,  utilChristie_SCW_EVRD,          CLI_COM_NO_REPLY|CLI_COM_READ_ONLY},
    {eCLI_MAIN_SD5,    "EVWR",         WRITE_COMMAND,    DECIAML_2,      DATA_CODE_NA,  utilChristie_SCW_EVWR,          CLI_COM_WRITE_ONLY},

    //SCW6+EVWR============================================================================================================//SCW6+EVWR
    {eCLI_MAIN_SD6,    "EVRD",         READ_COMMAND,     DECIAML_2,      DATA_CODE_NA,  utilChristie_SCW_EVRD,          CLI_COM_NO_REPLY|CLI_COM_READ_ONLY},
    {eCLI_MAIN_SD6,    "EVWR",         WRITE_COMMAND,    DECIAML_2,      DATA_CODE_NA,  utilChristie_SCW_EVWR,          CLI_COM_WRITE_ONLY},

    //SCW0+EVWR============================================================================================================//SCW0+EVWR
    {eCLI_MAIN_SD0,    "EVRD",         READ_COMMAND,     DECIAML_2,      DATA_CODE_NA,  utilChristie_SCW_EVRD,          CLI_COM_NO_REPLY|CLI_COM_READ_ONLY},
    {eCLI_MAIN_SD0,    "EVWR",         WRITE_COMMAND,    DECIAML_2,      DATA_CODE_NA,  utilChristie_SCW_EVWR,          CLI_COM_WRITE_ONLY},


    //SCHE+REST============================================================================================================//SCHE+REST
	{eCLI_MAIN_SCH, 	"REST", 	    READ_COMMAND,	  DECIAML_2,	  edcSCHEDULE_EVENT_RESET_ALL,	NULL,		   CLI_COM_WRITE_ONLY},
	{eCLI_MAIN_SCH, 	"REST", 	    WRITE_COMMAND,	  DECIAML_2,	  edcSCHEDULE_EVENT_RESET_ALL,	NULL,		   CLI_COM_WRITE_ONLY},

	//HDR+HDRE============================================================================================================
	{eCLI_MAIN_HDR, 	"HDRE", 	    READ_COMMAND,	  DECIAML_2,	  edcHDR_AUTOENABLE,	        NULL,		   CLI_COM_NORMAL},
	{eCLI_MAIN_HDR, 	"HDRE", 	    WRITE_COMMAND,	  DECIAML_2,	  edcHDR_AUTOENABLE,	        NULL,		   CLI_COM_NORMAL},

	//HDR+HDRL============================================================================================================
	{eCLI_MAIN_HDR, 	"HDRL", 	    READ_COMMAND,	  DECIAML_2,	  edcHDR_LEVEL,	        NULL,		   CLI_COM_NORMAL},
	{eCLI_MAIN_HDR, 	"HDRL", 	    WRITE_COMMAND,	  DECIAML_2,	  edcHDR_LEVEL,	        NULL,		   CLI_COM_NORMAL},

	//EDI+HD1S============================================================================================================
	{eCLI_MAIN_EDI, 	"HD1S", 	    READ_COMMAND,	  DECIAML_2,	  edcHDMI_EDID_1,	        NULL,		   CLI_COM_NORMAL},
	{eCLI_MAIN_EDI, 	"HD1S", 	    WRITE_COMMAND,	  DECIAML_2,	  edcHDMI_EDID_1,	        NULL,		   CLI_COM_NORMAL},

	//EDI+HD2S============================================================================================================
	{eCLI_MAIN_EDI, 	"HD2S", 	    READ_COMMAND,	  DECIAML_2,	  edcHDMI_EDID_2,	        NULL,		   CLI_COM_NORMAL},
	{eCLI_MAIN_EDI, 	"HD2S", 	    WRITE_COMMAND,	  DECIAML_2,	  edcHDMI_EDID_2,	        NULL,		   CLI_COM_NORMAL},

	//EDI+HDBT============================================================================================================
	{eCLI_MAIN_EDI, 	"HDBT", 	READ_COMMAND,	  DECIAML_2,	 edcHDBASET_EDID,	        NULL,		   CLI_COM_NORMAL},
	{eCLI_MAIN_EDI, 	"HDBT", 	WRITE_COMMAND,	  DECIAML_2,	 edcHDBASET_EDID,	        NULL,		   CLI_COM_NORMAL},

	//DBS============================================================================================================
	{eCLI_MAIN_DBS, 	NULL, 	    READ_COMMAND,	  DECIAML_2,	 edcDYNAMIC_BLACK,	        NULL,		   CLI_COM_NORMAL},
	{eCLI_MAIN_DBS, 	NULL, 	        WRITE_COMMAND,	  DECIAML_2,	  edcDYNAMIC_BLACK,	        NULL,		   CLI_COM_NORMAL},

	//RBS============================================================================================================
	{eCLI_MAIN_RBS, 	NULL, 	        READ_COMMAND,	  DECIAML_2,	  edcREAL_BLACK,	        NULL,		   CLI_COM_NORMAL},
	{eCLI_MAIN_RBS, 	NULL, 	        WRITE_COMMAND,	  DECIAML_2,	  edcREAL_BLACK,	        NULL,		   CLI_COM_NORMAL},

	//CRE============================================================================================================
	{eCLI_MAIN_CER, 	NULL, 	        READ_COMMAND,	  DECIAML_2,	  edcDYNAMIC_CONTRAST_RESET,	        NULL,		   CLI_COM_WRITE_ONLY}, //A70Gen2_Julie_0014
	{eCLI_MAIN_CER, 	NULL, 	        WRITE_COMMAND,	  DECIAML_2,	  edcDYNAMIC_CONTRAST_RESET,	        NULL,		   CLI_COM_NORMAL},

	//EQM + HDMA============================================================================================================
	{eCLI_MAIN_EQM, 	"HDMA", 	    READ_COMMAND,	  DECIAML_2,	  edcEQ_MODE_HDMI1,	        NULL,		   CLI_COM_NORMAL | CLI_COM_NEED_SERVICE_CODE_READ},
	{eCLI_MAIN_EQM, 	"HDMA", 	    WRITE_COMMAND,	  DECIAML_2,	  edcEQ_MODE_HDMI1,	        NULL,		   CLI_COM_NORMAL | CLI_COM_NEED_SERVICE_CODE_WRITE},

	//EQM + HDMB============================================================================================================
	{eCLI_MAIN_EQM, 	"HDMB", 	    READ_COMMAND,	  DECIAML_2,	  edcEQ_MODE_HDMI2,	        NULL,		   CLI_COM_NORMAL | CLI_COM_NEED_SERVICE_CODE_READ},
	{eCLI_MAIN_EQM, 	"HDMB", 	    WRITE_COMMAND,	  DECIAML_2,	  edcEQ_MODE_HDMI2,	        NULL,		   CLI_COM_NORMAL | CLI_COM_NEED_SERVICE_CODE_WRITE},

	//EQM + DVIC============================================================================================================
	{eCLI_MAIN_EQM, 	"DVIC", 	    READ_COMMAND,	  DECIAML_2,	  edcEQ_MODE_DVI,	        NULL,		   CLI_COM_NORMAL | CLI_COM_NEED_SERVICE_CODE_READ},
	{eCLI_MAIN_EQM, 	"DVIC", 	    WRITE_COMMAND,	  DECIAML_2,	  edcEQ_MODE_DVI,	        NULL,		   CLI_COM_NORMAL | CLI_COM_NEED_SERVICE_CODE_WRITE},

	//AIR============================================================================================================
	{eCLI_MAIN_AIR, 	NULL, 	        READ_COMMAND,	  DECIAML_2,	  edcAUTO_SOURCE_RESYNC,	        NULL,		   CLI_COM_NORMAL},
	{eCLI_MAIN_AIR, 	NULL, 	        WRITE_COMMAND,	  DECIAML_2,	  edcAUTO_SOURCE_RESYNC,	        NULL,		   CLI_COM_NORMAL},

	//PBR============================================================================================================
	{eCLI_MAIN_PBR, 	NULL, 	        READ_COMMAND,	  DECIAML_2,	  edcSUB_IMAGE_BRIGHTNESS,	        NULL,		   CLI_COM_NORMAL},
	{eCLI_MAIN_PBR, 	NULL, 	        WRITE_COMMAND,	  DECIAML_2,	  edcSUB_IMAGE_BRIGHTNESS,	        NULL,		   CLI_COM_NORMAL},

	//PCO============================================================================================================
	{eCLI_MAIN_PCO, 	NULL, 	        READ_COMMAND,	  DECIAML_2,	  edcSUB_IMAGE_CONTRAST,	        NULL,		   CLI_COM_NORMAL},
	{eCLI_MAIN_PCO, 	NULL, 	        WRITE_COMMAND,	  DECIAML_2,	  edcSUB_IMAGE_CONTRAST,	        NULL,		   CLI_COM_NORMAL},

	//PCS============================================================================================================
	{eCLI_MAIN_PCS, 	NULL, 	        READ_COMMAND,	  DECIAML_2,	  edcSUB_IMAGE_COLOR_SPACE,	        NULL,		   CLI_COM_NORMAL},
	{eCLI_MAIN_PCS, 	NULL, 	        WRITE_COMMAND,	  DECIAML_2,	  edcSUB_IMAGE_COLOR_SPACE,	        NULL,		   CLI_COM_NORMAL},

	{eCLI_MAIN_LLM,     NULL,           WRITE_COMMAND,    DECIAML_2,          edcLOW_LATENCY_MODE,            NULL,         CLI_COM_NORMAL },
    {eCLI_MAIN_LLM,     NULL,           READ_COMMAND,     DECIAML_2,          edcLOW_LATENCY_MODE,            NULL,         CLI_COM_NORMAL },

    //BKI============================================================================================================//
    {eCLI_MAIN_BKI,    NULL,       READ_COMMAND,     DECIAML_2,          edcBACKUPINPUT_AUTOSWITCH,           NULL,        CLI_COM_NORMAL},
    {eCLI_MAIN_BKI,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcBACKUPINPUT_AUTOSWITCH,           NULL,        CLI_COM_NORMAL},

    //BKI+"CSRC"============================================================================================================//
    {eCLI_MAIN_BKI,    "CSRC",       READ_COMMAND,     DECIAML_2,          edcBACKUPINPUT_CURRENT_SOURCE,           NULL,        CLI_COM_READ_ONLY},
    {eCLI_MAIN_BKI,    "CSRC",       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,           				   NULL,        CLI_COM_READ_ONLY},

    //BKI+"FSRC"============================================================================================================//
    {eCLI_MAIN_BKI,    "FSRC",       READ_COMMAND,     DECIAML_2,          edcBACKUPINPUT_PRIMARY_INPUT,    NULL, CLI_COM_NORMAL}, //HICC2_Doulas_0082//utilChristie_BKI_FSRC
    {eCLI_MAIN_BKI,    "FSRC",       WRITE_COMMAND,    DECIAML_2,          edcBACKUPINPUT_PRIMARY_INPUT, 	NULL, CLI_COM_NORMAL},	//A70Gen2_Doulas_00012

    ///BKI+"FRES"============================================================================================================//
    {eCLI_MAIN_BKI,    "FRES",       READ_COMMAND,     DECIAML_2,          edcBACKUPINPUT_PRIMARY_RESOLUTION,           NULL,        CLI_COM_READ_ONLY},
    {eCLI_MAIN_BKI,    "FRES",       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,           NULL,        CLI_COM_READ_ONLY},

    ///BKI+""FHOR""============================================================================================================//
    {eCLI_MAIN_BKI,    "FHOR",       READ_COMMAND,     DECIAML_2,          edcBACKUPINPUT_PRIMARY_HORZRATE,           NULL,        CLI_COM_READ_ONLY},
    {eCLI_MAIN_BKI,    "FHOR",       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,           NULL,        CLI_COM_READ_ONLY},

    ///BKI+"FCSP"============================================================================================================//
    {eCLI_MAIN_BKI,    "FCSP",       READ_COMMAND,     DECIAML_2,          edcBACKUPINPUT_PRIMARY_COLORSPACE,           NULL,        CLI_COM_READ_ONLY},
    {eCLI_MAIN_BKI,    "FCSP",       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,           NULL,        CLI_COM_READ_ONLY},

    ///BKI+"SSRC"============================================================================================================//
    {eCLI_MAIN_BKI,    "SSRC",       READ_COMMAND,     DECIAML_2,          edcBACKUPINPUT_SECONDARY_INPUT,  NULL, CLI_COM_NORMAL}, //HICC2_Doulas_0082//utilChristie_BKI_SSRC
    {eCLI_MAIN_BKI,    "SSRC",       WRITE_COMMAND,    DECIAML_2,          edcBACKUPINPUT_SECONDARY_INPUT, 	NULL, CLI_COM_NORMAL},	//A70Gen2_Doulas_00012

    ///BKI+"SRES"============================================================================================================//
    {eCLI_MAIN_BKI,    "SRES",       READ_COMMAND,     DECIAML_2,          edcBACKUPINPUT_SECONDARY_RESOLUTION,           NULL,        CLI_COM_READ_ONLY},
    {eCLI_MAIN_BKI,    "SRES",       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,           NULL,        CLI_COM_READ_ONLY},

    ///BKI+"SHOR"============================================================================================================//BKSH
    {eCLI_MAIN_BKI,    "SHOR",       READ_COMMAND,     DECIAML_2,          edcBACKUPINPUT_SECONDARY_HORZRATE,           NULL,        CLI_COM_READ_ONLY},
    {eCLI_MAIN_BKI,    "SHOR",       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,           NULL,        CLI_COM_READ_ONLY},

    ///BKI+"SCSP"============================================================================================================//BKSC
    {eCLI_MAIN_BKI,    "SCSP",       READ_COMMAND,     DECIAML_2,          edcBACKUPINPUT_SECONDARY_COLORSPACE,           NULL,        CLI_COM_READ_ONLY},
    {eCLI_MAIN_BKI,    "SCSP",       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,           NULL,        CLI_COM_READ_ONLY},

    ///BKI+"STAT"============================================================================================================//BKIS
    {eCLI_MAIN_BKI,    "STAT",       READ_COMMAND,     DECIAML_2,          edcBACKUPINPUT_STATUS,            NULL,        CLI_COM_READ_ONLY},
    {eCLI_MAIN_BKI,    "STAT",       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,                     NULL,        CLI_COM_READ_ONLY},

    ///BKI+"CHAN"============================================================================================================//CHAN
    {eCLI_MAIN_BKI,    "CHAN",       READ_COMMAND,     DECIAML_2,          edcBACKUP_INPUT_CHANGE,           NULL,        CLI_COM_READ_ONLY},   //A35G2_CDS_Simon_0042
    {eCLI_MAIN_BKI,    "CHAN",       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,                     NULL,        CLI_COM_READ_ONLY},

    ///BKI+"FHDR"============================================================================================================//FHDR
    {eCLI_MAIN_BKI,    "FHDR",       READ_COMMAND,     DECIAML_2,          edcFIRST_INPUT_HDR_INFO,          NULL,        CLI_COM_READ_ONLY},   //A35G2_CDS_Simon_0042
    {eCLI_MAIN_BKI,    "FHDR",       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,                     NULL,        CLI_COM_READ_ONLY},

    ///BKI+"SHDR"============================================================================================================//SHDR
    {eCLI_MAIN_BKI,    "SHDR",       READ_COMMAND,     DECIAML_2,          edcSECOND_INPUT_HDR_INFO,         NULL,        CLI_COM_READ_ONLY},   //A35G2_CDS_Simon_0042
    {eCLI_MAIN_BKI,    "SHDR",       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,                     NULL,        CLI_COM_READ_ONLY},

	//IST============================================================================================================
	{eCLI_MAIN_IST, 	NULL, 	    READ_COMMAND,	  DECIAML_2,	  DATA_CODE_NA,	        utilChristie_IST,		   CLI_COM_NORMAL},
	{eCLI_MAIN_IST, 	NULL, 	    WRITE_COMMAND,	  DECIAML_2,	  DATA_CODE_NA,	        utilChristie_IST,		   CLI_COM_NORMAL},

    //HMO
    {eCLI_MAIN_HMO,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcHDMI_OUT,            NULL,         CLI_COM_NORMAL },
    {eCLI_MAIN_HMO,     NULL,       READ_COMMAND,     DECIAML_2,          edcHDMI_OUT,            NULL,         CLI_COM_NORMAL },

    //OPD============================================================================================================//OPD
    {eCLI_MAIN_OPD,     NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,            utilChristie_OPD,      CLI_COM_NORMAL| CLI_COM_NO_REPLY},
    {eCLI_MAIN_OPD,     NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,            utilChristie_OPD,      CLI_COM_NORMAL},
    {eCLI_MAIN_OPD,     "REST",     READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,            utilChristie_OPDRSET,  CLI_COM_WRITE_ONLY},
    {eCLI_MAIN_OPD,     "REST",     WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,            utilChristie_OPDRSET,  CLI_COM_NORMAL},

    //TARG============================================================================================================//TARG  //G100_Julie_00020
    {eCLI_MAIN_TAR,     NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,            utilChristie_TAR,    CLI_COM_NORMAL| CLI_COM_NO_REPLY},
    {eCLI_MAIN_TAR,     NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,            utilChristie_TAR,    CLI_COM_NORMAL| CLI_COM_NO_REPLY},

	//CST============================================================================================================//CST	//A70Gen2_Doulas_0043
    {eCLI_MAIN_CST,     NULL,          READ_COMMAND,     DECIAML_2,      edcCAMERA_MODULE_STATUS,  NULL,           CLI_COM_NORMAL},

    //ATFC============================================================================================================//ATFC	//G100_Clare_0028
    {eCLI_MAIN_ATF,     NULL,          READ_COMMAND,     DECIAML_2,      DATA_CODE_NA,            utilChristie_ATF,     CLI_COM_NORMAL},  //A70Gen2_Doulas_0043 Modify//HICC2_Julie_0031
    {eCLI_MAIN_ATF,     NULL,          WRITE_COMMAND,    DECIAML_2,      DATA_CODE_NA,            utilChristie_ATF,     CLI_COM_NORMAL},

    //ATWC============================================================================================================//ATWC	//G100_Clare_0028
    {eCLI_MAIN_ATW,     NULL,          READ_COMMAND,     DECIAML_2,      DATA_CODE_NA,            utilChristie_ATW,     CLI_COM_NORMAL},	//A70Gen2_Doulas_0043 Modify//HICC2_Julie_0031
    {eCLI_MAIN_ATW,     NULL,          WRITE_COMMAND,    DECIAML_2,      DATA_CODE_NA,            utilChristie_ATW,     CLI_COM_NORMAL},

#if (ENABLE_COLOR_UNIFORMITY == TRUE) //G100_Tim_0012, add, start
	//ACUC============================================================================================================//ACUC
	{eCLI_MAIN_ACE,	  NULL, 		  READ_COMMAND, 	DECIAML_2,		edcCU_DATA_ENABLE, 		  NULL, 		  CLI_COM_NORMAL},
	{eCLI_MAIN_ACE,	  NULL, 		  WRITE_COMMAND,	DECIAML_2,		edcCU_DATA_ENABLE, 		  NULL, 		  CLI_COM_NORMAL},

	//ACUC============================================================================================================//ACUC
	{eCLI_MAIN_ACC,	  NULL, 		  READ_COMMAND, 	DECIAML_2,		DATA_CODE_NA, 		      utilChristie_ACC, 		  CLI_COM_NORMAL},	//A70Gen2_Doulas_0043 Modify//HICC2_Julie_0031
	{eCLI_MAIN_ACC,	  NULL, 		  WRITE_COMMAND,	DECIAML_2,		DATA_CODE_NA, 		      utilChristie_ACC, 	CLI_COM_NORMAL},

	//ACUS============================================================================================================//ACUS
	{eCLI_MAIN_ACS,	  NULL, 		  READ_COMMAND, 	DECIAML_2,		edcACU_STATUS,			  NULL, 		  CLI_COM_NORMAL},
	//{eCLI_MAIN_ACUS,	NULL,			WRITE_COMMAND,	  DECIAML_2,	  edcACU_STATUS,		    NULL,			CLI_COM_NORMAL}, //not allow to set ACU status via RS232 Cmd

	//ACUT============================================================================================================//ACUT
	{eCLI_MAIN_ACT,	  NULL, 		  READ_COMMAND, 	 DECIAML_2,   edcACU_TARGET_SEL,		  NULL, 		  CLI_COM_NORMAL},
	{eCLI_MAIN_ACT,	  NULL, 		  WRITE_COMMAND,	 DECIAML_2,   edcACU_TARGET_SEL,		  NULL, 		  CLI_COM_NORMAL},

	//ACUR============================================================================================================//ACUR
	{eCLI_MAIN_ACR,	  NULL, 		  READ_COMMAND, 	DECIAML_2,		edcACU_RESET,			  NULL, 		  CLI_COM_NORMAL}, //not allow
	{eCLI_MAIN_ACR,	  NULL, 		  WRITE_COMMAND,	DECIAML_2,		edcACU_RESET,			  NULL, 		  CLI_COM_NORMAL},

#endif //ENABLE_COLOR_UNIFORMITY	  //G100_Tim_0012, add, end

    //BOD=================================================================================================//BOD
    {eCLI_MAIN_BOD,     "CUST",     WRITE_COMMAND,    DECIAML_2,        DATA_CODE_NA,       utilChristie_BODCUST,       CLI_COM_NORMAL | CLI_COM_NEED_SERVICE_CODE_WRITE},
    {eCLI_MAIN_BOD,     "CUST",     READ_COMMAND,     DECIAML_2,        DATA_CODE_NA,       utilChristie_BODCUST,       CLI_COM_NORMAL},
    {eCLI_MAIN_BOD,     "PLAT",     WRITE_COMMAND,    DECIAML_2,        DATA_CODE_NA,       utilChristie_BODPLAT,       CLI_COM_NORMAL | CLI_COM_NEED_SERVICE_CODE_WRITE},
    {eCLI_MAIN_BOD,     "PLAT",     READ_COMMAND,     DECIAML_2,        DATA_CODE_NA,       utilChristie_BODPLAT,       CLI_COM_NORMAL},

    //UST=================================================================================================//BOD
	{eCLI_MAIN_UST,     NULL,       WRITE_COMMAND,    DECIAML_2,        edcUST_SET, 	    NULL,/*utilGeneral_CLI_UST*/CLI_COM_NORMAL | CLI_COM_NEED_SERVICE_CODE_WRITE},
    {eCLI_MAIN_UST,     NULL,       READ_COMMAND,     DECIAML_2,        edcUST_SET,	    	NULL,/*utilGeneral_CLI_UST*/CLI_COM_NORMAL | CLI_COM_NEED_SERVICE_CODE_WRITE},

	//WAE============================================================================================================//WAE
    {eCLI_MAIN_WAE,    NULL,         READ_COMMAND,    DECIAML_2,      edcWAP_ENABLE,             NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_WAE,    NULL,         WRITE_COMMAND,   DECIAML_2,      edcWAP_ENABLE,             NULL,           CLI_COM_NORMAL},  //G50_Casper_0006

	{eCLI_MAIN_DBG, 	"SCHE",		WRITE_COMMAND,	  DECIAML_2,		  DATA_CODE_NA,		   utilChristie_Debug_Schedule,		CLI_COM_NORMAL},         //T100_Coda_0014	//G100_Coda_0013
    {eCLI_MAIN_DBG,     "SCHE",     READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,        utilChristie_Debug_Schedule,     CLI_COM_NORMAL | CLI_COM_NO_REPLY},         //T100_Coda_0014

	{eCLI_MAIN_DBG, 	"AFMS",		WRITE_COMMAND,	  DECIAML_2,		  DATA_CODE_NA,		   utilChristie_Debug_AF_Msg,		CLI_COM_NORMAL},     //HICC2_Simon_0018
    {eCLI_MAIN_DBG,     "ACMS",     WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,        utilChristie_Debug_AC_Msg,       CLI_COM_NORMAL},     //HICC2_Simon_0018
    {eCLI_MAIN_DBG,     "ACUM",     WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,        utilChristie_Debug_ACU_Msg,      CLI_COM_NORMAL},     //HICC2_Simon_0018

    //STP============================================================================================================//STP
    {eCLI_MAIN_STP,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcSTARTUP_SHUTTER,              NULL,        CLI_COM_NORMAL_AND_PN }, //HICC2_Doulas_0071
    {eCLI_MAIN_STP,     NULL,       READ_COMMAND,     DECIAML_2,          edcSTARTUP_SHUTTER,              NULL,        CLI_COM_NORMAL_AND_PN }, //HICC2_Doulas_0071

    //FDI============================================================================================================//FDI
    {eCLI_MAIN_FDI,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcFADE_IN,                      NULL,        CLI_COM_NORMAL_AND_PN }, //HICC2_Doulas_0071
    {eCLI_MAIN_FDI,     NULL,       READ_COMMAND,     DECIAML_2,          edcFADE_IN,                      NULL,        CLI_COM_NORMAL_AND_PN }, //HICC2_Doulas_0071

    //FDO============================================================================================================//FDO
    {eCLI_MAIN_FDO,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcFADE_OUT,                     NULL,        CLI_COM_NORMAL_AND_PN }, //HICC2_Doulas_0071
    {eCLI_MAIN_FDO,     NULL,       READ_COMMAND,     DECIAML_2,          edcFADE_OUT,                     NULL,        CLI_COM_NORMAL_AND_PN }, //HICC2_Doulas_0071

#if(BIST_ENABLE) //HICC2_Steven_0001
    //BST=================================================================================================//
    {eCLI_MAIN_BST,     NULL,       WRITE_COMMAND,    DECIAML_2,        DATA_CODE_NA,       utilChristie_BIST_SET,       CLI_COM_NORMAL},
	{eCLI_MAIN_BST,     NULL,       READ_COMMAND,    DECIAML_2,        edcBIST_CHECK,       NULL,       CLI_COM_NORMAL},  //HICC2_Steven_0009
#endif

	//OPP============================================================================================================//OPP  //HICC2_Steven_0009
	{eCLI_MAIN_OPP,     NULL,       WRITE_COMMAND,    DECIAML_3,          edcOPD_PERIOD,            NULL,           CLI_COM_NORMAL},
	{eCLI_MAIN_OPP,     NULL,       READ_COMMAND,     DECIAML_3,          edcOPD_PERIOD,            NULL,           CLI_COM_NORMAL},

    //PRO============================================================================================================//PRO
    {eCLI_MAIN_PRO,     NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,            utilChristie_PRO,      CLI_COM_NORMAL| CLI_COM_NO_REPLY},
    {eCLI_MAIN_PRO,     NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,            utilChristie_PRO,      CLI_COM_NORMAL| CLI_COM_NO_REPLY},
    {eCLI_MAIN_PRO,     "SETT",     WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,            utilChristie_PROSETT,  CLI_COM_NORMAL| CLI_COM_NO_REPLY},
    {eCLI_MAIN_PRO,     "GETT",     WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,            utilChristie_PROGETT,  CLI_COM_NORMAL| CLI_COM_NO_REPLY},
    //CSL============================================================================================================//CSL
    {eCLI_MAIN_CSL,     "ENAX",     READ_COMMAND,     DECIAML_2,      DATA_CODE_NA,      utilChristie_CLI_XPR_ENAX,          CLI_COM_NORMAL},
    {eCLI_MAIN_CSL,     "ENAX",     WRITE_COMMAND,    DECIAML_2,      DATA_CODE_NA,      utilChristie_CLI_XPR_ENAX,          CLI_COM_NORMAL},
    {eCLI_MAIN_CSL,     "ENAY",     READ_COMMAND,     DECIAML_2,      DATA_CODE_NA,      utilChristie_CLI_XPR_ENAY,          CLI_COM_NORMAL},
    {eCLI_MAIN_CSL,     "ENAY",     WRITE_COMMAND,    DECIAML_2,      DATA_CODE_NA,      utilChristie_CLI_XPR_ENAY,          CLI_COM_NORMAL},

    {eCLI_MAIN_CSL,     "GANX",     READ_COMMAND,     DECIAML_2,      DATA_CODE_NA,      utilChristie_CLI_XPR_GANX,          CLI_COM_READ_ONLY},
    {eCLI_MAIN_CSL,     "GANY",     READ_COMMAND,     DECIAML_2,      DATA_CODE_NA,      utilChristie_CLI_XPR_GANY,          CLI_COM_READ_ONLY},

    {eCLI_MAIN_CSL,     "SGLX",     READ_COMMAND,     DECIAML_2,      DATA_CODE_NA,      utilChristie_CLI_XPR_SGLX,          CLI_COM_READ_ONLY},
    {eCLI_MAIN_CSL,     "SGLY",     READ_COMMAND,     DECIAML_2,      DATA_CODE_NA,      utilChristie_CLI_XPR_SGLY,          CLI_COM_READ_ONLY},
    {eCLI_MAIN_CSL,     "SNGT",     READ_COMMAND,     STRING_CHAR,      DATA_CODE_NA,      utilChristie_CLI_XPR_SNGT,          CLI_COM_READ_ONLY},
    {eCLI_MAIN_CSL,     "ALOP",     WRITE_COMMAND,     DECIAML_2,     edcCLOSE_LOOP_APPLY_OPEN_LOOP,       NULL,          CLI_COM_NORMAL},

    {eCLI_MAIN_CSL,     "ERR0",     READ_COMMAND,     DECIAML_2,      DATA_CODE_NA,      utilChristie_CLI_XPR_ERROR_CODE_0,          CLI_COM_NORMAL},
    {eCLI_MAIN_CSL,     "ERR1",     READ_COMMAND,     DECIAML_2,      DATA_CODE_NA,      utilChristie_CLI_XPR_ERROR_CODE_1,          CLI_COM_NORMAL},
    {eCLI_MAIN_CSL,     "VALD",     READ_COMMAND,     DECIAML_2,      DATA_CODE_NA,      utilChristie_CLI_XPR_CLOSE_LOOP_VALID,          CLI_COM_NORMAL},
    {eCLI_MAIN_CSL,     "ERRS",     WRITE_COMMAND,    DECIAML_2,      DATA_CODE_NA,      utilChristie_CLI_XPR_ERROR_CODE_RESET,          CLI_COM_NORMAL},
    {eCLI_MAIN_CSL,     "APEE",     WRITE_COMMAND,    DECIAML_2,      DATA_CODE_NA,      utilChristie_CLI_GAIN_WRITE_EEPROM,          CLI_COM_NORMAL},
    {eCLI_MAIN_CSL,     "ADC0",     READ_COMMAND,     DECIAML_2,      DATA_CODE_NA,      utilChristie_CLI_XPR_TARGET_ADC0,          CLI_COM_NORMAL},
    {eCLI_MAIN_CSL,     "ADC1",     READ_COMMAND,     DECIAML_2,      DATA_CODE_NA,      utilChristie_CLI_XPR_TARGET_ADC1,          CLI_COM_NORMAL},
    {eCLI_MAIN_CSL,     "ZDT0",     READ_COMMAND,     DECIAML_2,      DATA_CODE_NA,      utilChristie_CLI_XPR_ZDATA0,          CLI_COM_NORMAL},
    {eCLI_MAIN_CSL,     "ZDT1",     READ_COMMAND,     DECIAML_2,      DATA_CODE_NA,      utilChristie_CLI_XPR_ZDATA1,          CLI_COM_NORMAL},

    //RSP============================================================================================================//RSP  // R70K_Bruce_0067//HICC2_Julie_0011
    {eCLI_MAIN_RSP,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcDISPLAY_RESET,                NULL,        CLI_COM_WRITE_ONLY},
    {eCLI_MAIN_RSP,     NULL,       READ_COMMAND,     DECIAML_2,          edcDISPLAY_RESET,                NULL,        CLI_COM_WRITE_ONLY},

    //RIS============================================================================================================//RIS  // R70K_Bruce_0067//HICC2_Julie_0011
    {eCLI_MAIN_RIS,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcImageReset,                   NULL,        CLI_COM_WRITE_ONLY},
    {eCLI_MAIN_RIS,     NULL,       READ_COMMAND,     DECIAML_2,          edcImageReset,                   NULL,        CLI_COM_WRITE_ONLY},

    //RCF============================================================================================================//RCF  // R70K_Bruce_0067//HICC2_Julie_0011
    {eCLI_MAIN_RCF,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcSETUP_RESET,                  NULL,        CLI_COM_WRITE_ONLY},
    {eCLI_MAIN_RCF,     NULL,       READ_COMMAND,     DECIAML_2,          edcSETUP_RESET,                  NULL,        CLI_COM_WRITE_ONLY},

    //LSF============================================================================================================//LSF //A35G2_Wesley_0153 //A70Gen2_Julie_0109//HICC2_Julie_0014
    {eCLI_MAIN_LSF,     NULL,       READ_COMMAND,     DECIAML_2,          edcLENS_TOP_BOTTOM_CENTER,    NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_LSF,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcLENS_TOP_BOTTOM_CENTER,    NULL,          CLI_COM_NORMAL},

	//ATD============================================================================================================//ATD
	{eCLI_MAIN_ATD,     NULL,       READ_COMMAND,     DECIAML_2,          edcDMD_AIRTIGHT_STATUS,    NULL,          CLI_COM_NORMAL},
	{eCLI_MAIN_ATD,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcDMD_AIRTIGHT_STATUS,    NULL,          CLI_COM_NORMAL},

	//GUP============================================================================================================//GUP
	{eCLI_MAIN_GUP,     NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,             utilChristie_CLI_GUP,       CLI_COM_NORMAL},
	{eCLI_MAIN_GUP,     NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,             utilChristie_CLI_GUP,       CLI_COM_NORMAL},
	{eCLI_MAIN_GUP,     "FRZE",     READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,             utilChristie_CLI_GUP_FRZE,  CLI_COM_NORMAL},
	{eCLI_MAIN_GUP,     "FRZE",     WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,             utilChristie_CLI_GUP_FRZE,  CLI_COM_NORMAL},
	{eCLI_MAIN_GUP,     "SHUT",     READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,             utilChristie_CLI_GUP_SHUT,  CLI_COM_NORMAL},
	{eCLI_MAIN_GUP,     "SHUT",     WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,             utilChristie_CLI_GUP_SHUT,  CLI_COM_NORMAL},
	{eCLI_MAIN_GUP,     "SCHE",     READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,             utilChristie_CLI_GUP_SCHE,  CLI_COM_NORMAL},
	{eCLI_MAIN_GUP,     "SCHE",     WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,             utilChristie_CLI_GUP_SCHE,  CLI_COM_NORMAL},
	{eCLI_MAIN_GUP,     "ATFE",     READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,             utilChristie_CLI_GUP_ATFE,  CLI_COM_NORMAL},
	{eCLI_MAIN_GUP,     "ATFE",     WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,             utilChristie_CLI_GUP_ATFE,  CLI_COM_NORMAL},
	{eCLI_MAIN_GUP,     "ACME",     READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,             utilChristie_CLI_GUP_ACME,  CLI_COM_NORMAL},
	{eCLI_MAIN_GUP,     "ACME",     WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,             utilChristie_CLI_GUP_ACME,  CLI_COM_NORMAL},
	{eCLI_MAIN_GUP,     "RSET",     READ_COMMAND,     DECIAML_2,          edcPROJECTOR_GROUP_RESET, NULL,                       CLI_COM_WRITE_ONLY},
	{eCLI_MAIN_GUP,     "RSET",     WRITE_COMMAND,    DECIAML_2,          edcPROJECTOR_GROUP_RESET, NULL,                       CLI_COM_WRITE_ONLY},
};

#define CHRISTIE_CMD_LUT_NUMBER sizeof(m_sSubLut_Christie)/sizeof(sCLI_SUB_LUT)

BOOL utilChristie_PulldownListCheck(UINT16 uiDatacode, sCLI_CHRISTIE_FORMAT* sCmdFormat)
{
     switch(uiDatacode)
     {
        case edcPICTURE_SETTINGS:
        case edcUSER_COLOR_MODE:
        case edcGAMMA:
        case edcCOLOR_SPACE:
        case edcSUB_IMAGE_COLOR_SPACE:
            {
                UINT32 iValue = CLI2CM(uiDatacode, sCmdFormat->lData);

                if(CommonAPI_CM_FuncIsAvailable(uiDatacode, iValue))
                {
                    return FALSE;
                }
                else
                {
                    return TRUE;
                }
            }
            break;

        default:
            return FALSE;
     }
}

// ==============================================================================
// FUNCTION NAME: utilChristie_CLI_AddressID
// DESCRIPTION:
//
//
// Params:
// UINT16 uiAddress:
//
// Returns:
//
//
// Modification History
// --------------------
// 2018/11/21, Larry Create
// --------------------
// ==============================================================================
void utilChristie_CLI_AddressID(UINT32 ulAddress)
{
    m_ulAddressID = ulAddress;
}

// ==============================================================================
// FUNCTION NAME: utilChristie_CMD_Decode
// DESCRIPTION:
//
//
// Params:
// UINT8* pcString:
// sCLI_CHRISTIE_FORMAT* sCmdFormat:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/11/08, Larry Create
// --------------------
// ==============================================================================
UINT8 utilChristie_CMD_Decode(UINT8* pcString, sCLI_CHRISTIE_FORMAT* sCmdFormat)
{
    UINT8   ucStringLen = 0;
    UINT8   ucErrorCode = eCLI_ERROR_CODE_NO;
    UINT8   ucCmdIndex = 0;
    UINT8   ucDecdoeStep = eCLI_DECODE_STEP_HEADER;
    UINT8   ucCount = 0;
    UINT8   ucFoundSpece = 0;
    UINT8   ucDataSpece = 0; //A70LV_Larry_0444
    UINT16  uiMainCode = 0;
    UINT16  uiSubCode = 0;

    ucStringLen = strlen((char*)pcString);

    if(ucStringLen > CHRISTIE_CMD_LENGTH_MAX || ucStringLen < CHRISTIE_CMD_LENGTH_MIN) //R70K_AC_0031
    {
        return eCLI_ERROR_CODE_LENGTH;
    }

    do
    {
        switch(ucDecdoeStep)
        {
            case eCLI_DECODE_STEP_HEADER:
                if(pcString[ucCmdIndex] == CHRISTIE_CMD_HEADER_1 || pcString[ucCmdIndex] == CHRISTIE_CMD_HEADER_2)
                {
                    ucCmdIndex++;
                    ucDecdoeStep = eCLI_DECODE_STEP_PRE_CHAR_CHECK;
                }
                else
                {
                    ucCmdIndex++;
                }
                break;

            case eCLI_DECODE_STEP_PRE_CHAR_CHECK:
                {
                    switch(pcString[ucCmdIndex])
                    {
                        case CHRISTIE_CMD_RPR_CHAR_SIMPLE_ACK:
                            sCmdFormat->ucPrefixCharType = eCLI_PREFIX_CHAR_SIMPLE_ACK;
                            ucCmdIndex++;
                            break;
                        case CHRISTIE_CMD_RPR_CHAR_FULL_ACK:
                            sCmdFormat->ucPrefixCharType = eCLI_PREFIX_CHAR_FULL_SUM;
                            ucCmdIndex++;
                            break;
                        case CHRISTIE_CMD_RPR_CHAR_CHECK_SUM:
                            sCmdFormat->ucPrefixCharType = eCLI_PREFIX_CHAR_CHECKSUM;
                            ucCmdIndex++;
                            break;

                        default:
                            sCmdFormat->ucPrefixCharType = eCLI_PREFIX_CHAR_NO;
                            break;
                    }

                    ucDecdoeStep = eCLI_DECODE_STEP_ADDRESS_CHECK;
                }
                break;

            case eCLI_DECODE_STEP_ADDRESS_CHECK:
                if(__CheckNum(pcString[ucCmdIndex]))
                {
                    sCmdFormat->uiProjectorAddress[0] = '\0';
                    //Projector Add
                    for(ucCount = 0; ucCount < ucStringLen; ucCount++)
                    {
                        sCmdFormat->uiProjectorAddress[ucCount] = '\0';

                        if(__CheckNum(pcString[ucCmdIndex]))
                        {
                            sCmdFormat->uiProjectorAddress[ucCount] = pcString[ucCmdIndex];
                            ucCmdIndex++;

                            if(ucCount >= CHRISTIE_ADDRESS_LEN_MAX)
                            {
                                return eCLI_ERROR_CODE_DESTADDERR;
                            }
                        }
                        else if(pcString[ucCmdIndex] == CHRISTIE_CMD_SPACE)
                        {
                            ucCmdIndex++;
							sCmdFormat->ulProjectorAddressID = (UINT32)atoi(sCmdFormat->uiProjectorAddress); //A70LV_Larry_0385
                            break;
                        }
                        else //other //A70LV_Larry_0385
                        {
                            sCmdFormat->ulProjectorAddressID = (UINT32)atoi(sCmdFormat->uiProjectorAddress);
                            break;
                        }
                    }

                    sCmdFormat->uiProjectorHost[0] = '\0';

                    if(__CheckNum(pcString[ucCmdIndex])) //has Host Add
                    {
                        //Host Add
                        for(ucCount = 0; ucCount < ucStringLen; ucCount++)
                        {
                            sCmdFormat->uiProjectorHost[ucCount] = '\0';

                            if(__CheckNum(pcString[ucCmdIndex]))
                            {
                                sCmdFormat->uiProjectorHost[ucCount] = pcString[ucCmdIndex];
                                ucCmdIndex++;

                                if(ucCount >= CHRISTIE_ADDRESS_LEN_MAX)
                                {
                                    return eCLI_ERROR_CODE_SRCADDERR;
                                }
                            }
                            else
                            {
                                break;
                            }
                        }
                        sCmdFormat->ucHasHost = 1; //A70LV_Larry_0385
                        sCmdFormat->ulProjectorHostID = (UINT32)atoi(sCmdFormat->uiProjectorHost); //A70LV_Larry_0385
                    }
                }
                ucDecdoeStep = eCLI_DECODE_STEP_MAIN;
                break;

            case eCLI_DECODE_STEP_MAIN:
                if((ucCmdIndex + CHRISTIE_MAIN_CMD_LEN) >= ucStringLen) //(XXX > cmd length
                {
                    return eCLI_ERROR_CODE_LENGTH;
                }
                else
                {
                    strncpy((char*)&sCmdFormat->cMainCode[0], (char*)&pcString[ucCmdIndex], CHRISTIE_MAIN_CMD_LEN);

                    ucCmdIndex += CHRISTIE_MAIN_CMD_LEN;

                    //Lookup main code
                    for(uiMainCode = 0; uiMainCode <= CLI_MAINLUT_NUMBER; uiMainCode++)
                    {
                        if((m_sMainLut[uiMainCode].uiMainCmdID == eCLI_MAIN_END) || (uiMainCode == CLI_MAINLUT_NUMBER))
                        {
                            return eCLI_ERROR_CODE_FUNCCODENOTFOUND;
                        }
                        else
                        {
                            if(__CheckCmd(sCmdFormat->cMainCode, m_sMainLut[uiMainCode].cMainCmd, CHRISTIE_MAIN_CMD_LEN))
                            {
                                sCmdFormat->cMainCode[CHRISTIE_MAIN_CMD_LEN] = '\0';
                                ucDecdoeStep = eCLI_DECODE_STEP_SUB;
                                break;
                            }
                        }
                    }
                }
                break;

                case eCLI_DECODE_STEP_SUB:
                    {
                        //UINT8 ucFoundSpece = 0;

                        if(pcString[ucCmdIndex] == CHRISTIE_CMD_SUB_CONNECT) //has sub code "+"
                        {
                            ucCmdIndex++;
                            if((ucCmdIndex + CHRISTIE_SUB_CMD_LEN) >= ucStringLen) //(XXX > cmd length
                            {
                                return eCLI_ERROR_CODE_LENGTH;
                            }
                            strncpy((char*)&sCmdFormat->cSubCode[0], (char*)&pcString[ucCmdIndex], CHRISTIE_SUB_CMD_LEN);

                            sCmdFormat->cSubCode[CHRISTIE_SUB_CMD_LEN] = '\0';
                            sCmdFormat->ucHasSubCode = 1;
                            ucCmdIndex += CHRISTIE_SUB_CMD_LEN;
                        }
                        else if(pcString[ucCmdIndex] == CHRISTIE_CMD_SPACE && ucFoundSpece)
                        {
                            return eCLI_ERROR_CODE_DATAERROR;
                        }
                        else if(pcString[ucCmdIndex] == CHRISTIE_CMD_SPACE)
                        {
                            ucFoundSpece = 1;
                            ucCmdIndex++;
                            break; //A70LV_Larry_0444
                        }
                        else
                        {
                            sCmdFormat->cSubCode[0] = '\0';
                        }

                        ucDataSpece = 0; //A70LV_Larry_0444

                        for(ucCount = ucCmdIndex; ucCount < ucStringLen; ucCount++) //Cmd Is Read?
                        {
                            if(pcString[ucCount] == CHRISTIE_CMD_SPACE)
                            {
                                ucDataSpece++; //A70LV_Larry_0444
                                continue;
                            }
                            else if(pcString[ucCount] == CHRISTIE_CMD_REQUEST)
                            {
                                if((ucFoundSpece) && (sCmdFormat->ucHasSubCode == 1)) //A70LV_Larry_0444
                                {
                                    return eCLI_ERROR_CODE_DATAERROR;
                                }
                                if(ucDataSpece > 1)
                                {
                                    return eCLI_ERROR_CODE_DATAERROR;
                                }
                                ucCmdIndex = ucCount; //A70LV_Larry_0444
                                sCmdFormat->ucIsRead = READ_COMMAND;
                                break;
                            }
                            else
                            {
                                if(ucDataSpece > 1) //A70LV_Larry_0444
                                {
                                    return eCLI_ERROR_CODE_DATAERROR;
                                }
                                ucCmdIndex = ucCount; //A70LV_Larry_0444
                                sCmdFormat->ucIsRead = WRITE_COMMAND;
                                break;
                            }
                        }

                        //Lookup sub code
                        for(uiSubCode = 0; uiSubCode < CHRISTIE_CMD_LUT_NUMBER; uiSubCode++)
                        {
                            if((m_sSubLut_Christie[uiSubCode].uiMainCmdID == uiMainCode) && (m_sSubLut_Christie[uiSubCode].ucCmdIsRead == sCmdFormat->ucIsRead))
                            {
                                if((sCmdFormat->ucHasSubCode == 0) && (m_sSubLut_Christie[uiSubCode].cSubCmd == NULL))
                                {
                                    sCmdFormat->uiFuncID = uiSubCode;
									ucDecdoeStep = eCLI_DECODE_STEP_DATA_FORMAT;
                                    break;
                                }
                                else
                                {
                                    if(m_sSubLut_Christie[uiSubCode].cSubCmd != NULL)
                                    {
                                        if(__CheckCmd(sCmdFormat->cSubCode, m_sSubLut_Christie[uiSubCode].cSubCmd, CHRISTIE_SUB_CMD_LEN))
                                        {
                                            sCmdFormat->uiFuncID = uiSubCode;
                                            ucFoundSpece = 0; //clear spece flag
                                            ucDecdoeStep = eCLI_DECODE_STEP_DATA_FORMAT;
                                            break;
                                        }
                                    }
                                }
                            }
                        }

                        if(uiSubCode == CHRISTIE_CMD_LUT_NUMBER)
                        {
                            if(sCmdFormat->cSubCode[0] == '\0') //has main code define, but not found any command
                            {
                                return eCLI_ERROR_CODE_FUNCCODENOTFOUND;
                            }
                            else
                            {
                                return eCLI_ERROR_CODE_SUBCODEERR;
                            }
                        }

                        if(!(m_sSubLut_Christie[uiSubCode].uiSpecialFlag & CLI_COM_ENABLE)) //Rs232Cmd.cfg set command not support.
                        {
                            return eCLI_ERROR_CODE_FUNCCODENOTFOUND;
                        }
                    }
                    break;

                    case eCLI_DECODE_STEP_DATA_FORMAT:
                            if(pcString[ucCmdIndex] == CHRISTIE_CMD_END_1 || pcString[ucCmdIndex] == CHRISTIE_CMD_END_2)
                            {
                                return eCLI_ERROR_CODE_DATAERROR;
                            }
                            else
                            {
                                //UINT8 ucFoundSpece = 0;

                                if(sCmdFormat->ucIsRead == READ_COMMAND)
                                {
                                    sCmdFormat->ucDataType = eCLI_DATA_TYPE_DONTCARE;
                                }
                                else //Write
                                {
                                    sCmdFormat->ucDataType = eCLI_DATA_TYPE_UNKNOW;

                                    for(ucCount = ucCmdIndex; ucCount < ucStringLen; ucCount++)
                                    {
                                        if(pcString[ucCount] == CHRISTIE_CMD_SPACE)
                                        {
                                            if(ucFoundSpece)
                                            {
                                                return eCLI_ERROR_CODE_DATAERROR;
                                            }
                                            ucFoundSpece++;
                                        }
                                        else if(pcString[ucCount] == CHRISTIE_CMD_DATA_STRING)
                                        {
                                            ucCmdIndex = ucCount;
                                            ucCmdIndex++;
                                            sCmdFormat->ucDataType = eCLI_DATA_TYPE_STRING;
                                            break;
                                        }
                                        else if((pcString[ucCount] == CHRISTIE_CMD_DATA_P) || (pcString[ucCount] == CHRISTIE_CMD_DATA_p))
                                        {
                                            ucCmdIndex = ucCount;
                                            ucCmdIndex++;
                                            sCmdFormat->ucDataType = eCLI_DATA_TYPE_P;
                                            break;
                                        }
                                        else if((pcString[ucCount] == CHRISTIE_CMD_DATA_N) || (pcString[ucCount] == CHRISTIE_CMD_DATA_n))
                                        {
                                            ucCmdIndex = ucCount;
                                            ucCmdIndex++;
                                            sCmdFormat->ucDataType = eCLI_DATA_TYPE_N;
                                            break;
                                        }
                                        else if((pcString[ucCount] == CHRISTIE_CMD_DATA_U) || (pcString[ucCount] == CHRISTIE_CMD_DATA_u)) //HICC2_Steven_0056 start
                                        {
                                            ucCmdIndex = ucCount;
                                            ucCmdIndex++;
                                            sCmdFormat->ucDataType = eCLI_DATA_TYPE_U;
                                            break;
                                        }
                                        else if((pcString[ucCount] == CHRISTIE_CMD_DATA_D) || (pcString[ucCount] == CHRISTIE_CMD_DATA_d))
                                        {
                                            ucCmdIndex = ucCount;
                                            ucCmdIndex++;
                                            sCmdFormat->ucDataType = eCLI_DATA_TYPE_D;
                                            break;
                                        }
                                        else if((pcString[ucCount] == CHRISTIE_CMD_DATA_R) || (pcString[ucCount] == CHRISTIE_CMD_DATA_r))
                                         {
                                             ucCmdIndex = ucCount;
                                             ucCmdIndex++;
                                             sCmdFormat->ucDataType = eCLI_DATA_TYPE_R;
                                             break;
                                         }
                                         else if((pcString[ucCount] == CHRISTIE_CMD_DATA_L) || (pcString[ucCount] == CHRISTIE_CMD_DATA_l))
                                         {
                                             ucCmdIndex = ucCount;
                                             ucCmdIndex++;
                                             sCmdFormat->ucDataType = eCLI_DATA_TYPE_L;
                                             break;
                                         } //HICC2_Steven_0056 end
                                        else if((pcString[ucCount] >= '0' && pcString[ucCount] <= '9') || (pcString[ucCount] == '-'))
                                        {
                                            ucCmdIndex = ucCount;
                                            sCmdFormat->ucDataType = eCLI_DATA_TYPE_DEC;
                                            break;
                                        }
                                        else
                                        {
                                            break;
                                        }
                                    }
                                }

                                switch(sCmdFormat->ucDataType)
                                {
                                    case eCLI_DATA_TYPE_UNKNOW:
                                        return eCLI_ERROR_CODE_DATAERROR;

                                    case eCLI_DATA_TYPE_DEC:
                                        {
                                            UINT8 ucIsPoint = 0;
                                            UINT8 ucNegative = 0;
                                            //UINT8 ucFoundSpece = 0;
                                            UINT8 aucDataTemp[7] = {'\0'};
                                            UINT8 ucDataIndex = 0;

                                            if(pcString[ucCount] == CHRISTIE_CMD_SPACE)
                                            {
                                                ucCmdIndex++;
                                            }

                                            for(ucCount = ucCmdIndex; ucCount < ucStringLen; ucCount++)
                                            {
                                                if((pcString[ucCount] == '-') && (ucNegative == 1))
                                                {
                                                    return eCLI_ERROR_CODE_NEGATIVEERR;
                                                }
                                                else if(pcString[ucCount] == '-')
                                                {
                                                    //ucDataIndex++;
                                                    ucNegative = 1;
                                                }
                                                else if((pcString[ucCount] == '.') && (ucIsPoint == 1))
                                                {
                                                    return eCLI_ERROR_CODE_DECIMALERR;
                                                }
                                                else if(pcString[ucCount] == '.')
                                                {
                                                    aucDataTemp[ucDataIndex++] = pcString[ucCount];
                                                    ucIsPoint = 1;
                                                }
                                                else if(__CheckNum(pcString[ucCount]))
                                                {
                                                    aucDataTemp[ucDataIndex++] = pcString[ucCount];
                                                }
                                                else if((pcString[ucCount] == CHRISTIE_CMD_END_1) ||
                                                        (pcString[ucCount] == CHRISTIE_CMD_END_2) ||
                                                        (pcString[ucCount] == CHRISTIE_CMD_SPACE)) //end
                                                {

                                                    aucDataTemp[ucDataIndex++] = '\0';
                                                    ucCmdIndex = ucCount;
                                                    break;
                                                }
                                                else
                                                {
                                                    return eCLI_ERROR_CODE_DECIMALERR;
                                                }

                                                if(ucDataIndex >= 6)//overflow
                                                    return eCLI_ERROR_CODE_DECIMALERR;
                                            }

                                            if(ucIsPoint)
                                            {
                                                sCmdFormat->fData = atof((char*)aucDataTemp);

                                                if(ucNegative)
                                                {
                                                    sCmdFormat->fData = -sCmdFormat->fData;
                                                }
                                                sCmdFormat->ucDataType = eCLI_DATA_TYPE_POINT;
                                                //LOG_MSG(db_DV_FAN, "fData %f\n", sCmdFormat->fData);
                                            }
                                            else
                                            {
                                                sCmdFormat->lData = atoi((char*)aucDataTemp);

                                                if(ucNegative)
                                                {
                                                    sCmdFormat->lData = -sCmdFormat->lData;
                                                }

                                                //LOG_MSG(db_DV_FAN, "iData %d\n", sCmdFormat->lData);
                                            }
                                        }
                                        break;

                                    case eCLI_DATA_TYPE_STRING:
                                        if((ucCmdIndex + CHRISTIE_CMD_TEXT_SIZE + 2) < ucStringLen) //+2 for end ' " 'and ')'
                                        {
                                            //LOG_MSG(db_DV_FAN, "STRING %d %d %d\n", ucCmdIndex, CHRISTIE_CMD_TEXT_SIZE, ucStringLen);

                                            return eCLI_ERROR_CODE_STRINGTOOBIG;
                                        }
                                        else
                                        {
                                            UINT8 ucStringIndex = 0;

                                            for(ucCount = ucCmdIndex; ucCount < ucStringLen; ucCount++)
                                            {
                                                if(pcString[ucCount] == CHRISTIE_CMD_DATA_STRING) //end
                                                {
                                                    sCmdFormat->cTextString[ucStringIndex] = '\0';
                                                    ucCmdIndex = ucCount;
                                                    ucCmdIndex++;

                                                    //LOG_MSG(db_DV_FAN, "STRING %s\n", sCmdFormat->cTestString);
                                                    break;
                                                }
												else if((pcString[ucCount] != CHRISTIE_CMD_HEADER_1) &&
                                                        (pcString[ucCount] != CHRISTIE_CMD_HEADER_2) &&
                                                        (pcString[ucCount] != CHRISTIE_CMD_END_1) &&
                                                        (pcString[ucCount] != CHRISTIE_CMD_END_2) &&
                                                        (pcString[ucCount] != CHRISTIE_CMD_DIS_CHAR_1))
                                                {
                                                    sCmdFormat->cTextString[ucStringIndex++] = pcString[ucCount];
                                                }
                                                else
                                                {
                                                    return eCLI_ERROR_CODE_STRINGERR;
                                                }
                                            }

                                        }
                                        break;

                                    case eCLI_DATA_TYPE_DONTCARE:
                                        ucCmdIndex++;
                                        break;

                                    case eCLI_DATA_TYPE_N:
                                        if(!ucFoundSpece)
                                        {
                                            return eCLI_ERROR_CODE_CUart_N_NOSPACE;
                                        }
                                        else
                                        {
                                            switch(pcString[ucCmdIndex])
                                            {
                                                case '0':
                                                    ucCmdIndex++;
                                                    sCmdFormat->ucDataType = eCLI_DATA_TYPE_N_STOP;
                                                    break;

                                                case '1':
                                                    ucCmdIndex++;
                                                    sCmdFormat->ucDataType = eCLI_DATA_TYPE_N_STEP;
                                                    break;

                                                case '2':
                                                    ucCmdIndex++;
                                                    sCmdFormat->ucDataType = eCLI_DATA_TYPE_N_RUN;
                                                    break;

                                                default:
                                                    break;
                                            }
                                        }
                                        break;

                                    case eCLI_DATA_TYPE_P:
                                        if(!ucFoundSpece)
                                        {
                                            return eCLI_ERROR_CODE_CUart_P_NOSPACE;
                                        }
                                        else
                                        {
                                            switch(pcString[ucCmdIndex])
                                            {
                                                case '0':
                                                    ucCmdIndex++;
                                                    sCmdFormat->ucDataType = eCLI_DATA_TYPE_P_STOP;
                                                    break;

                                                case '1':
                                                    ucCmdIndex++;
                                                    sCmdFormat->ucDataType = eCLI_DATA_TYPE_P_STEP;
                                                    break;

                                                case '2':
                                                    ucCmdIndex++;
                                                    sCmdFormat->ucDataType = eCLI_DATA_TYPE_P_RUN;
                                                    break;

                                                default:
                                                    break;
                                            }
                                        }
                                        break;

                                    case eCLI_DATA_TYPE_U: //HICC2_Steven_0056 start
                                        if(!ucFoundSpece)
                                        {
                                            return eCLI_ERROR_CODE_CUart_U_NOSPACE;
                                        }
                                        else
                                        {
                                            switch(pcString[ucCmdIndex])
                                            {
                                                case '0':
                                                    ucCmdIndex++;
                                                    sCmdFormat->ucDataType = eCLI_DATA_TYPE_U_STOP;
                                                    break;

                                                case '1':
                                                    ucCmdIndex++;
                                                    sCmdFormat->ucDataType = eCLI_DATA_TYPE_U_STEP;
                                                    break;

                                                case '2':
                                                    ucCmdIndex++;
                                                    sCmdFormat->ucDataType = eCLI_DATA_TYPE_U_RUN;
                                                    break;

                                                default:
                                                    break;
                                            }
                                        }
                                        break;

                                    case eCLI_DATA_TYPE_D:
                                        if(!ucFoundSpece)
                                        {
                                            return eCLI_ERROR_CODE_CUart_D_NOSPACE;
                                        }
                                        else
                                        {
                                            switch(pcString[ucCmdIndex])
                                            {
                                                case '0':
                                                    ucCmdIndex++;
                                                    sCmdFormat->ucDataType = eCLI_DATA_TYPE_D_STOP;
                                                    break;

                                                case '1':
                                                    ucCmdIndex++;
                                                    sCmdFormat->ucDataType = eCLI_DATA_TYPE_D_STEP;
                                                    break;

                                                case '2':
                                                    ucCmdIndex++;
                                                    sCmdFormat->ucDataType = eCLI_DATA_TYPE_D_RUN;
                                                    break;

                                                default:
                                                    break;
                                            }
                                        }
                                        break;

                                    case eCLI_DATA_TYPE_R:
                                        if(!ucFoundSpece)
                                        {
                                            return eCLI_ERROR_CODE_CUart_R_NOSPACE;
                                        }
                                        else
                                        {
                                            switch(pcString[ucCmdIndex])
                                            {
                                                case '0':
                                                    ucCmdIndex++;
                                                    sCmdFormat->ucDataType = eCLI_DATA_TYPE_R_STOP;
                                                    break;

                                                case '1':
                                                    ucCmdIndex++;
                                                    sCmdFormat->ucDataType = eCLI_DATA_TYPE_R_STEP;
                                                    break;

                                                case '2':
                                                    ucCmdIndex++;
                                                    sCmdFormat->ucDataType = eCLI_DATA_TYPE_R_RUN;
                                                    break;

                                                default:
                                                    break;
                                            }
                                        }
                                        break;

                                    case eCLI_DATA_TYPE_L:
                                        if(!ucFoundSpece)
                                        {
                                            return eCLI_ERROR_CODE_CUart_L_NOSPACE;
                                        }
                                        else
                                        {
                                            switch(pcString[ucCmdIndex])
                                            {
                                                case '0':
                                                    ucCmdIndex++;
                                                    sCmdFormat->ucDataType = eCLI_DATA_TYPE_L_STOP;
                                                    break;

                                                case '1':
                                                    ucCmdIndex++;
                                                    sCmdFormat->ucDataType = eCLI_DATA_TYPE_L_STEP;
                                                    break;

                                                case '2':
                                                    ucCmdIndex++;
                                                    sCmdFormat->ucDataType = eCLI_DATA_TYPE_L_RUN;
                                                    break;

                                                default:
                                                    break;
                                            }
                                        }
                                        break; //HICC2_Steven_0056 end
                                }
                                ucDecdoeStep = eCLI_DECODE_STEP_END;
                            }
                        break;

                    case eCLI_DECODE_STEP_END:
                        if(sCmdFormat->ucPrefixCharType == eCLI_PREFIX_CHAR_CHECKSUM)
                        {
                            if(pcString[ucCmdIndex] != CHRISTIE_CMD_SPACE)
                            {
                                return eCLI_ERROR_CODE_CKSUMNPSPACE;
                            }
                            else
                            {
                                UINT8 ucDataCheckSum = 0;

                                UINT8 aucCheckSum[CHRISTIE_CHECKSUM_LEN]={'\0'};

                                for(ucCount = 1; ucCount <= ucCmdIndex; ucCount++) //&SYS ~ ' '
                                {
                                    ucDataCheckSum += pcString[ucCount];
                                }

                                ucCmdIndex++;

                                for(ucCount = 0; ucCount < CHRISTIE_CHECKSUM_LEN; ucCount++)
                                {
                                    if(__CheckNum(pcString[ucCmdIndex + ucCount]))
                                    {
                                        aucCheckSum[ucCount] = pcString[ucCmdIndex + ucCount];
                                    }
                                    else
                                    {
                                        break;
                                    }
                                }

                                ucCmdIndex += ucCount;

                                if((pcString[ucCmdIndex] == CHRISTIE_CMD_END_1) || (pcString[ucCmdIndex] == CHRISTIE_CMD_END_2))
                                {
                                    if(ucDataCheckSum != atoi((char*)aucCheckSum))
                                    {
                                        return eCLI_ERROR_CODE_CKSUMERR;
                                    }
        							ucCmdIndex++;
                                    ucErrorCode = eCLI_ERROR_CODE_NO;
                                }
                                else
                                {
                                    return eCLI_ERROR_CODE_DATAOVERRANGE;
                                }
                            }
                        }
                        else if((pcString[ucCmdIndex] == CHRISTIE_CMD_END_1) || (pcString[ucCmdIndex] == CHRISTIE_CMD_END_2))
                        {
							ucCmdIndex++;
                            ucErrorCode = eCLI_ERROR_CODE_NO;
                        }
                        else
                        {
                            return eCLI_ERROR_CODE_DATAOVERRANGE;
                        }
                        break;

                default:
                    ucCmdIndex = ucStringLen;
                    break;
                    //turn eCLI_ERROR_CODE_FORMATERR;
        }
    }while(ucCmdIndex < ucStringLen);

    //LOG_MSG(db_DV_FAN, "Add code %s, Host code %s\n", sCmdFormat->uiProjectorAddress, sCmdFormat->uiProjectorHost);
    //LOG_MSG(db_DV_FAN, "Main code %s, main code id %d\n", sCmdFormat->cMainCode, uiMainCode);
    //LOG_MSG(db_DV_FAN, "Sub code %s, sub code id %d\n", sCmdFormat->cSubCode, uiSubCode);
    //LOG_MSG(db_DV_FAN, "CmdType %d\n", sCmdFormat->ucCmdType);

    return ucErrorCode;
}

// ==============================================================================
// FUNCTION NAME: utilChristie_CMD_Reply
// DESCRIPTION:
//
//
// Params:
// UINT8 ucChannel:
// UINT8 ucErrorCode:
// sCLI_CHRISTIE_FORMAT* sCmdFormat:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/11/08, Larry Create
// --------------------
// ==============================================================================
void utilChristie_CMD_Reply(UINT8 ucChannel, UINT8 ucErrorCode, sCLI_CHRISTIE_FORMAT* sCmdFormat, char *ucReturnString)
{
    UINT8 ucDataType = utilCommonCLI_DataTypeGet(m_sSubLut_Christie[sCmdFormat->uiFuncID].uiFunCodeID); //A70LV_Larry_0065

    if(m_sSubLut_Christie[sCmdFormat->uiFuncID].uiFunCodeID == DATA_CODE_NA)
    {
        if(m_sSubLut_Christie[sCmdFormat->uiFuncID].ucCmdBytesOnRange == STRING_CHAR)
        {
            ucDataType = DATATYPE_STRING;
        }
    }

    if((m_sSubLut_Christie[sCmdFormat->uiFuncID].uiSpecialFlag & CLI_COM_NO_REPLY) && (ucErrorCode == eCLI_ERROR_CODE_NO))
    {
        return;
    }

    if((ucErrorCode != eCLI_ERROR_CODE_NO) && (ucErrorCode < eCLI_ERROR_NUMBER))
    {
        switch(ucErrorCode)
        {
            case eCLI_ERROR_CODE_SUBCODEERR:
            case eCLI_ERROR_CODE_SUBCODENEED:
                sprintf(ucReturnString, "%c65535 00000 ERR%05d \"%s%c%s: %s\"%c", m_sCli_Config[ucChannel].cCMD_Hander, m_sErrorLut[ucErrorCode].ucErrCode,  sCmdFormat->cMainCode, CHRISTIE_CMD_SUB_CONNECT, sCmdFormat->cSubCode, m_sErrorLut[ucErrorCode].pcErrorMsg, m_sCli_Config[ucChannel].cCMD_End);
                break;

            default :
                sprintf(ucReturnString, "%c65535 00000 ERR%05d \"%s: %s\"%c", m_sCli_Config[ucChannel].cCMD_Hander, m_sErrorLut[ucErrorCode].ucErrCode, sCmdFormat->cMainCode, m_sErrorLut[ucErrorCode].pcErrorMsg, m_sCli_Config[ucChannel].cCMD_End);
                break;
        }
        //__CMD_Respond(ucChannel, ucReturnString);
        //LOG_MSG(db_ALWAYS, "%s", );
    }
    else
    {
        if(sCmdFormat->ucIsRead == WRITE_COMMAND) //Write
        {
            switch(sCmdFormat->ucPrefixCharType)
            {
                case eCLI_PREFIX_CHAR_SIMPLE_ACK:
                    sprintf(ucReturnString, "%c", CHRISTIE_CMD_RPR_CHAR_SIMPLE_ACK);
                    break;

                case eCLI_PREFIX_CHAR_FULL_SUM:
                    switch(sCmdFormat->ucDataType)
                    {
                        case eCLI_DATA_TYPE_DEC:
                        case eCLI_DATA_TYPE_POINT:
                        case eCLI_DATA_TYPE_N:
                        case eCLI_DATA_TYPE_P:
                            {
                                char ucaCLITemp[11] = {'\0'};

                                if(sCmdFormat->ucHasSubCode)
                                {
                                    sprintf(ucaCLITemp, "%s%c%s%c", sCmdFormat->cMainCode, CHRISTIE_CMD_SUB_CONNECT, sCmdFormat->cSubCode, CHRISTIE_CMD_REPLY);
                                }
                                else
                                {
                                    sprintf(ucaCLITemp, "%s%c", sCmdFormat->cMainCode, CHRISTIE_CMD_REPLY);
                                }

                                if((sCmdFormat->ucDataType == eCLI_DATA_TYPE_DEC) || (sCmdFormat->ucDataType == eCLI_DATA_TYPE_N) || (sCmdFormat->ucDataType == eCLI_DATA_TYPE_P))
                                {
                                    switch(m_sSubLut_Christie[sCmdFormat->uiFuncID].ucCmdBytesOnRange)
                                    {
                                        case DECIAML_2:
                                            sprintf(ucReturnString, "%c%s%02d%c", m_sCli_Config[ucChannel].cCMD_Hander, ucaCLITemp, sCmdFormat->lData, m_sCli_Config[ucChannel].cCMD_End);
                                            break;

                                        case DECIAML_3:
                                            sprintf(ucReturnString, "%c%s%03d%c", m_sCli_Config[ucChannel].cCMD_Hander, ucaCLITemp, sCmdFormat->lData, m_sCli_Config[ucChannel].cCMD_End);
                                            break;

                                        case DECIAML_4:
                                            sprintf(ucReturnString, "%c%s%04d%c", m_sCli_Config[ucChannel].cCMD_Hander, ucaCLITemp, sCmdFormat->lData, m_sCli_Config[ucChannel].cCMD_End);
                                            break;

                                        case DECIAML_5:
                                            sprintf(ucReturnString, "%c%s%05d%c", m_sCli_Config[ucChannel].cCMD_Hander, ucaCLITemp, sCmdFormat->lData, m_sCli_Config[ucChannel].cCMD_End);
                                            break;
                                    }
                                }
                                else if(sCmdFormat->ucDataType == eCLI_DATA_TYPE_POINT)
                                {
                                    switch(m_sSubLut_Christie[sCmdFormat->uiFuncID].ucCmdBytesOnRange)
                                    {
                                        case DECIAML_2:
                                            sprintf(ucReturnString, "%c%s%02f%c", m_sCli_Config[ucChannel].cCMD_Hander, ucaCLITemp, sCmdFormat->fData, m_sCli_Config[ucChannel].cCMD_End);
                                            break;

                                        case DECIAML_3:
                                            sprintf(ucReturnString, "%c%s%03f%c", m_sCli_Config[ucChannel].cCMD_Hander, ucaCLITemp, sCmdFormat->fData, m_sCli_Config[ucChannel].cCMD_End);
                                            break;

                                        case DECIAML_4:
                                            sprintf(ucReturnString, "%c%s%04f%c", m_sCli_Config[ucChannel].cCMD_Hander, ucaCLITemp, sCmdFormat->fData, m_sCli_Config[ucChannel].cCMD_End);
                                            break;

                                        case DECIAML_5:
                                            sprintf(ucReturnString, "%c%s%05f%c", m_sCli_Config[ucChannel].cCMD_Hander, ucaCLITemp, sCmdFormat->fData, m_sCli_Config[ucChannel].cCMD_End);
                                            break;
                                    }
                                }
                                else
                                {
                                    sprintf(ucReturnString, "");
                                }
                            }
                            break;

                        case eCLI_DATA_TYPE_STRING:
                            if(sCmdFormat->ucHasSubCode)
                            {
                                sprintf(ucReturnString, "%c%s%c%s%c%s%c", m_sCli_Config[ucChannel].cCMD_Hander, sCmdFormat->cMainCode, CHRISTIE_CMD_SUB_CONNECT, sCmdFormat->cSubCode, CHRISTIE_CMD_REPLY, sCmdFormat->cTextString, m_sCli_Config[ucChannel].cCMD_End);
                            }
                            else
                            {
                                sprintf(ucReturnString, "%c%s%c%s%c", m_sCli_Config[ucChannel].cCMD_Hander, sCmdFormat->cMainCode, CHRISTIE_CMD_REPLY, sCmdFormat->cTextString, m_sCli_Config[ucChannel].cCMD_End);
                            }
                            break;

                        default:
                            if(sCmdFormat->ucHasSubCode)
                            {
                                sprintf(ucReturnString, "%c%s%c%s%c%c", m_sCli_Config[ucChannel].cCMD_Hander, sCmdFormat->cMainCode, CHRISTIE_CMD_SUB_CONNECT, sCmdFormat->cSubCode, CHRISTIE_CMD_REPLY, m_sCli_Config[ucChannel].cCMD_End);
                            }
                            else
                            {
                                sprintf(ucReturnString, "%c%s%c%c", m_sCli_Config[ucChannel].cCMD_Hander, sCmdFormat->cMainCode, CHRISTIE_CMD_REPLY, m_sCli_Config[ucChannel].cCMD_End);
                            }
                            break;

                    }
                    break;

                default:
                    break;
                }
            }
            else //Read
            {
                char ucCLITemp[32] = {'\0'};
                char ucCLITempAddress[16] = {'\0'}; //A70LV_Larry_0385

                if(sCmdFormat->ucHasHost) //A70LV_Larry_0385
                {
                    sprintf(ucCLITempAddress, "%05d %05d", sCmdFormat->ulProjectorHostID, sCmdFormat->ulProjectorAddressID);
                }

                if(sCmdFormat->ucHasSubCode)
                {
                    sprintf(ucCLITemp, "%s%s%c%s%c", ucCLITempAddress, sCmdFormat->cMainCode, CHRISTIE_CMD_SUB_CONNECT, sCmdFormat->cSubCode, CHRISTIE_CMD_REPLY);
                }
                else
                {
                    sprintf(ucCLITemp, "%s%s%c", ucCLITempAddress, sCmdFormat->cMainCode, CHRISTIE_CMD_REPLY);
                }

                switch(ucDataType)
                {
                    case DATATYPE_DIGIT:
                        switch(m_sSubLut_Christie[sCmdFormat->uiFuncID].ucCmdBytesOnRange)
                        {
                            case DECIAML_2:
                                sprintf(ucReturnString, "%c%s%02d%c", m_sCli_Config[ucChannel].cCMD_Hander, ucCLITemp, sCmdFormat->lData, m_sCli_Config[ucChannel].cCMD_End);
                                break;

                            case DECIAML_3:
                                sprintf(ucReturnString, "%c%s%03d%c", m_sCli_Config[ucChannel].cCMD_Hander, ucCLITemp, sCmdFormat->lData, m_sCli_Config[ucChannel].cCMD_End);
                                break;

                            case DECIAML_4:
                                sprintf(ucReturnString, "%c%s%04d%c", m_sCli_Config[ucChannel].cCMD_Hander, ucCLITemp, sCmdFormat->lData, m_sCli_Config[ucChannel].cCMD_End);
                                break;

                            case DECIAML_5:
                                sprintf(ucReturnString, "%c%s%05d%c", m_sCli_Config[ucChannel].cCMD_Hander, ucCLITemp, sCmdFormat->lData, m_sCli_Config[ucChannel].cCMD_End);
                                break;

                            case STRING_CHAR:
                                if(m_sSubLut_Christie[sCmdFormat->uiFuncID].uiSpecialFlag & CLI_COM_DEC2STRING)
                                {
                                    sprintf(ucReturnString, "%c%s%c%s%c%c", m_sCli_Config[ucChannel].cCMD_Hander, ucCLITemp, CHRISTIE_CMD_DATA_STRING, sCmdFormat->cTextString, CHRISTIE_CMD_DATA_STRING,  m_sCli_Config[ucChannel].cCMD_End);
                                }
                                break;

							default:
								break;
                        }
                        break;

                    case DATATYPE_STRING:
                        {
                            sprintf(ucReturnString, "%c%s%c%s%c%c", m_sCli_Config[ucChannel].cCMD_Hander, ucCLITemp, CHRISTIE_CMD_DATA_STRING, sCmdFormat->cTextString, CHRISTIE_CMD_DATA_STRING,  m_sCli_Config[ucChannel].cCMD_End);
                        }
                        break;

                    default:
                        break;
                }
            }
    }
    return;
}

// ==============================================================================
// FUNCTION NAME: utilChristie_CMD_Handle
// DESCRIPTION:
//
//
// Params:
// sCLI_CHRISTIE_FORMAT* sCmdFormat:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/11/08, Larry Create
// --------------------
// ==============================================================================
UINT8 utilChristie_CMD_Handle(sCLI_CHRISTIE_FORMAT* sCmdFormat)
{
    UINT8 ucErrorCode = eCLI_ERROR_CODE_NO; //A35G2_Wesley_0155
    UINT8 ucDataType = 0;
    UINT8 ucFreeze = 0; //A35G2_Wesley_0155

    ucDataType = utilCommonCLI_DataTypeGet(m_sSubLut_Christie[sCmdFormat->uiFuncID].uiFunCodeID);
    palDataMgr_Data_Access(edcIMAGE_FREEZE, edaREAD, &ucFreeze); //A35G2_Wesley_0155//HICC2_Julie_0038

    if(sCmdFormat->uiFuncID < CHRISTIE_CMD_LUT_NUMBER)
    {
        if(sCmdFormat->ucIsRead == WRITE_COMMAND) //Write command
        {
#ifdef NO_POWER_OFF_DURING_CAMERA_WORKING			//A35G2_Alan_0004 start
			if( palDataMgr_Get_Camera_Working_Status() == eCAMERA_STATUS_WORKING )
			{
				// not allowed to change setting !!!
				return eCLI_ERROR_CODE_SETFAIL;
			}
#endif //NO_POWER_OFF_DURING_CAMERA_WORKING 	//A35G2_Alan_0004 end

			//A35G2_Wesley_0155//HICC2_Julie_0038
    		if(ucFreeze)
            {
                switch(m_sSubLut_Christie[sCmdFormat->uiFuncID].uiMainCmdID)
    			{
                    case eCLI_MAIN_FRZ:
                    case eCLI_MAIN_SYS:
                    case eCLI_MAIN_SHU:
                    case eCLI_MAIN_PWR:
                    case eCLI_MAIN_RIS:
                        break;

                    default:
                        ucErrorCode = eCLI_ERROR_CODE_SETFAIL;
                        break;
    			}
    			if(ucErrorCode != eCLI_ERROR_CODE_NO)
    			{
                    return eCLI_ERROR_CODE_SETFAIL;
    			}
            }

        #if 0//###
        	//A35G2_Alan_0009 --->
        	if(m_sSubLut_Christie[sCmdFormat->uiFuncID].uiMainCmdID == eCLI_MAIN_ACC || m_sSubLut_Christie[sCmdFormat->uiFuncID].uiMainCmdID == eCLI_MAIN_ATW || m_sSubLut_Christie[sCmdFormat->uiFuncID].uiMainCmdID == eCLI_MAIN_ATF)//A35G2_Alan_0028
        	{
				appGui_UC_Data_Cmd_Flag_Set(TRUE);
        	}
			//A35G2_Alan_0009 <---
        #endif

            if(m_sSubLut_Christie[sCmdFormat->uiFuncID].uiSpecialFlag & CLI_COM_NEED_SERVICE_CODE_WRITE) //A70LV_Larry_0235
            {
                if(palDataMgr_ServiceModeGet() == ets_OFF)
                {
                    return eCLI_ERROR_CODE_SERVICE_CODE_NEED_WHEN_WRITE;
                }
            }

            if(palDataMgr_PIN_Protect_Checking() != ePASSWORD_PROTECT_UNLOCK && m_sSubLut_Christie[sCmdFormat->uiFuncID].uiMainCmdID != eCLI_MAIN_PIV && m_sSubLut_Christie[sCmdFormat->uiFuncID].uiMainCmdID != eCLI_MAIN_KEY)
            {
                return eCLI_ERROR_CODE_SERVICE_CODE_NEED_WHEN_WRITE;
            }

            if((m_sSubLut_Christie[sCmdFormat->uiFuncID].uiSpecialFlag & CLI_COM_READ_ONLY))
            {
                return eCLI_ERROR_CODE_COMMAND_READ_ONLY;
            }

            if(m_sSubLut_Christie[sCmdFormat->uiFuncID].uiFunCodeID != DATA_CODE_NA)
            {
                switch(utilCommonCLI_DataControl(m_sSubLut_Christie[sCmdFormat->uiFuncID].uiFunCodeID))
                {
                    case eFUNC_CONTROL_ENABLE:
                        break;

                    case eFUNC_CONTROL_SOURCE_DEPEND:
                        return eCLI_ERROR_CODE_MUSTDEPENDONSRC;

                    case eFUNC_CONTROL_SERCIVE_PROTECT:
                        return eCLI_ERROR_CODE_SERVICE_CODE_NEED_WHEN_WRITE;

                    default:
                        return eCLI_ERROR_CODE_SETFAIL;
                }

                if(utilChristie_PulldownListCheck(m_sSubLut_Christie[sCmdFormat->uiFuncID].uiFunCodeID, sCmdFormat))
                {
                    return eCLI_ERROR_CODE_SETFAIL;
                }
            }
            else //special case, ex, hpbu test, SST
            {
                if(m_sSubLut_Christie[sCmdFormat->uiFuncID].iCLI_Replay != NULL)
                {
                    return  m_sSubLut_Christie[sCmdFormat->uiFuncID].iCLI_Replay(ecmWrite, sCmdFormat);
                }
                else
                {
                    return eCLI_ERROR_CODE_SETFAIL;
                }
            }

            if(m_sSubLut_Christie[sCmdFormat->uiFuncID].uiSpecialFlag & CLI_COM_TYPE_PN_ONLY) //lens shift
            {
                switch(sCmdFormat->ucDataType)
                {
                    case eCLI_DATA_TYPE_N:
                    case eCLI_DATA_TYPE_N_STEP:
                    case eCLI_DATA_TYPE_N_STOP:
                    case eCLI_DATA_TYPE_N_RUN:
                    case eCLI_DATA_TYPE_P:
                    case eCLI_DATA_TYPE_P_STEP:
                    case eCLI_DATA_TYPE_P_STOP:
                    case eCLI_DATA_TYPE_P_RUN:
                        break;

                    default:
                        return eCLI_ERROR_CODE_SETFAIL;
                }

                return eCLI_ERROR_CODE_NO;
            }

            switch(sCmdFormat->ucDataType) //Write command
            {
                case eCLI_DATA_TYPE_DEC:
                    {
                        INT32 iData = 0;
                        sDATABASE_ITEM_DATA_FORMAT sInputData;
                        iData = sCmdFormat->lData;

                        sInputData.uiItemIndex = m_sSubLut_Christie[sCmdFormat->uiFuncID].uiFunCodeID;
                        sInputData.uValue.lValue = iData;
                        palDataMgr_NotifyGroupingEvent(sInputData);
                        if(eEXEC_CODE_PASS == utilCommonCLI_RangeCheck(m_sSubLut_Christie[sCmdFormat->uiFuncID].uiFunCodeID, iData))
                        {
                            if(eEXEC_CODE_PASS == utilCommonCLI_DataConversionSet(m_sSubLut_Christie[sCmdFormat->uiFuncID].uiFunCodeID, iData))
                            {
                                return eCLI_ERROR_CODE_NO;
                            }
                        }
                        else
                        {
                            return eCLI_ERROR_CODE_DATAOVERRANGE;
                        }
                    }
					return eCLI_ERROR_CODE_DATAOVERRANGE;

                case eCLI_DATA_TYPE_STRING:
                    {
                        if(ucDataType != DATATYPE_STRING)
                        {
                            return eCLI_ERROR_CODE_SETFAIL;
                        }

                        if(eEXEC_CODE_PASS == utilCommonCLI_String_Set(m_sSubLut_Christie[sCmdFormat->uiFuncID].uiFunCodeID, (UINT8*)sCmdFormat->cTextString))
                        {
                            return eCLI_ERROR_CODE_NO;
                        }
                    }
					return eCLI_ERROR_CODE_SETFAIL;

                case eCLI_DATA_TYPE_POINT:
                    break;

                case eCLI_DATA_TYPE_N:
                    if(m_sSubLut_Christie[sCmdFormat->uiFuncID].uiSpecialFlag & CLI_COM_TYPE_SUPPORT_PN)
                    {
                        if(eEXEC_CODE_PASS == utilCommonCLI_Decrease_Data(m_sSubLut_Christie[sCmdFormat->uiFuncID].uiFunCodeID))
                        {
                            INT32 iSetting = 0;

                            utilCommonCLI_DataConversionGet(m_sSubLut_Christie[sCmdFormat->uiFuncID].uiFunCodeID, &iSetting);

                            sCmdFormat->lData = iSetting;

                            return eCLI_ERROR_CODE_NO;
                        }
                    }
					return eCLI_ERROR_CODE_DATAERROR;

                case eCLI_DATA_TYPE_N_STEP:
                case eCLI_DATA_TYPE_N_STOP:
                case eCLI_DATA_TYPE_N_RUN:
					return eCLI_ERROR_CODE_DATAERROR;

                case eCLI_DATA_TYPE_P:
                    if(m_sSubLut_Christie[sCmdFormat->uiFuncID].uiSpecialFlag & CLI_COM_TYPE_SUPPORT_PN)
                    {
                        if(eEXEC_CODE_PASS == utilCommonCLI_Increase_Data(m_sSubLut_Christie[sCmdFormat->uiFuncID].uiFunCodeID))
                        {
                            INT32 iSetting = 0;

                            utilCommonCLI_DataConversionGet(m_sSubLut_Christie[sCmdFormat->uiFuncID].uiFunCodeID, &iSetting);

                            sCmdFormat->lData = iSetting;
                            return eCLI_ERROR_CODE_NO;
                        }
                    }
					return eCLI_ERROR_CODE_DATAERROR;

                case eCLI_DATA_TYPE_P_STEP:
                case eCLI_DATA_TYPE_P_STOP:
                case eCLI_DATA_TYPE_P_RUN:
					return eCLI_ERROR_CODE_DATAERROR;

                default:
                    break;
            }
        }
        else //Read
        {
            if((m_sSubLut_Christie[sCmdFormat->uiFuncID].uiSpecialFlag & CLI_COM_WRITE_ONLY))
            {
                return eCLI_ERROR_CODE_COMMAND_WRITE_ONLY;
            }

#if 1
            if(m_sSubLut_Christie[sCmdFormat->uiFuncID].uiSpecialFlag & CLI_COM_NEED_SERVICE_CODE_READ)
            {
                if(palDataMgr_ServiceModeGet() == ets_OFF)
                {
                    return eCLI_ERROR_CODE_SERVICE_CODE_NEED_WHEN_READ;
                }
            }
#endif /* 0 */

            if(m_sSubLut_Christie[sCmdFormat->uiFuncID].uiFunCodeID != DATA_CODE_NA)
            {
                switch(utilCommonCLI_DataControl(m_sSubLut_Christie[sCmdFormat->uiFuncID].uiFunCodeID))
                {
                    case eFUNC_CONTROL_ENABLE:
                        break;

                    case eFUNC_CONTROL_SOURCE_DEPEND:
                        return eCLI_ERROR_CODE_MUSTDEPENDONSRC;

                    case eFUNC_CONTROL_SERCIVE_PROTECT:
                        return eCLI_ERROR_CODE_SERVICE_CODE_NEED_WHEN_READ;

                    default:
                        return eCLI_ERROR_CODE_REQUESTFAIL;
                }
            }
            else //special case, ex, hpbu test, SST
            {
                if(m_sSubLut_Christie[sCmdFormat->uiFuncID].iCLI_Replay != NULL)
                {
                    return m_sSubLut_Christie[sCmdFormat->uiFuncID].iCLI_Replay(ecmRead, sCmdFormat);

                }
                else
                {
                    return eCLI_ERROR_CODE_REQUESTFAIL;
                }
            }


            switch(ucDataType) //Read command
            {
                case DATATYPE_DIGIT:
                    {
                        INT32 iData = 0;

                        if(eEXEC_CODE_PASS == utilCommonCLI_DataConversionGet(m_sSubLut_Christie[sCmdFormat->uiFuncID].uiFunCodeID, &iData))
                        {
                            sCmdFormat->lData = iData;

                            if(m_sSubLut_Christie[sCmdFormat->uiFuncID].uiSpecialFlag & CLI_COM_DEC2STRING)
                            {
                                UINT8 ucString[CHRISTIE_CMD_LENGTH_MAX] = {'\0'};
                                UINT8 ucStrlen = 0;
                                INT32 iValue2 = CLI2CM(m_sSubLut_Christie[sCmdFormat->uiFuncID].uiFunCodeID, iData);

                                if(eCLI_ERROR_CODE_NO == utilCommonCLI_DecToString(m_sSubLut_Christie[sCmdFormat->uiFuncID].uiFunCodeID, iValue2, ucString))
                                {
                                    ucStrlen = strlen(ucString);

                                    memcpy(sCmdFormat->cTextString, ucString, ucStrlen);
                                }
                                else
                                {
                                    return eCLI_ERROR_CODE_MessageTypeError;
                                }
                            }

                            return eCLI_ERROR_CODE_NO;
                        }
                    }
                    return eCLI_ERROR_CODE_REQUESTFAIL;

                case DATATYPE_STRING:
                    {
                        UINT8 ucString[CHRISTIE_CMD_LENGTH_MAX] = {'\0'};
                        UINT8 ucStrlen = 0;

                        if(eEXEC_CODE_PASS == utilCommonCLI_String_Get(m_sSubLut_Christie[sCmdFormat->uiFuncID].uiFunCodeID, ucString))
                        {
                            ucStrlen = strlen((char*)ucString);

                            if(ucStrlen > CHRISTIE_CMD_TEXT_SIZE)
                            {
                                ucStrlen = CHRISTIE_CMD_TEXT_SIZE;
                            }
                            memcpy(sCmdFormat->cTextString, ucString, ucStrlen);
                            return eCLI_ERROR_CODE_NO;
                        }
                    }
					return eCLI_ERROR_CODE_REQUESTFAIL;

                default:
                    break;
            }

        }

    }
    return eCLI_ERROR_CODE_NO;
}

// ==============================================================================
// FUNCTION NAME: utilChristie_CLI_Handle
// DESCRIPTION:
//
//
// Params:
// eCLI_CHANNEL eCh:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/11/08, Larry Create
// --------------------
// ==============================================================================
UINT8 utilChristie_CLI_Handle(UINT8 eCh, UINT8 *pcData)
{
    UINT8 ucData = 0;
    UINT16 uiEmpty = 1;
    UINT8 ucDataOut = 0; //A70LV_John_0008 add basic function and flow of GEC

    if(eCh >= eccNumber)
        return 0;

    ucData = *pcData;
    utilChristie_CLI_AMX(ucData);

    if(utilDbgMsg_Get(db_APP_CLI))
    {
        LOG_MSG(db_ALWAYS, "\n%c\0\n", ucData);
    }
//A70LV_John_0008 end

    if(m_sCLI_Info[eCh].ucSerialPortEcho && uiEmpty) //A70LV_Larry_0060
    {
        LOG_MSG(db_ALWAYS, "%c", ucData);
    }

    if((ucData == m_sCli_Config[eCh].cCMD_Hander || m_sCLI_Info[eCh].uiCLI_Poll_Position != 0) && uiEmpty)
    {
        m_sCLI_Info[eCh].ucCLI_DATA[m_sCLI_Info[eCh].uiCLI_Poll_Position++] = ucData;

        if(ucData == m_sCli_Config[eCh].cCMD_Hander) //(sys(sys?)
        {
            m_sCLI_Info[eCh].uiCLI_Poll_Position = 0;
            m_sCLI_Info[eCh].ucCLI_DATA[m_sCLI_Info[eCh].uiCLI_Poll_Position++] = ucData;
        }

        if(m_sCLI_Info[eCh].uiCLI_Poll_Position > CHRISTIE_CMD_LENGTH_MAX)
        {
            m_sCLI_Info[eCh].uiCLI_Poll_Position = 0;
            return 0;
        }

        if(ucData ==  m_sCli_Config[eCh].cCMD_End)
        {
            sCLI_CHRISTIE_FORMAT sCmdFormat = {0};
            sCmdFormat.ucCmdFrom = eCh; //T100_Simon_0020

            char ucReturnString[CHRISTIE_CMD_LENGTH_MAX] = {'\0'};

            m_sCLI_Info[eCh].ucCLI_DATA[m_sCLI_Info[eCh].uiCLI_Poll_Position] = '\0';

            utilChristie_CLI_Process(eCh, m_sCLI_Info[eCh].ucCLI_DATA, ucReturnString, &sCmdFormat, CLI_EXECTUE_READ_CMD | CLI_EXECTUE_WRITE_BUFFER); //A70LV_Larry_0388

            m_sCLI_Info[eCh].uiCLI_Poll_Position = 0;

            return 1;
        }
    }
    return 0;
}

// ==============================================================================
// FUNCTION NAME: utilChristie_CLI_Init
// DESCRIPTION:
//
//
// Params:
//
// Returns:
//
//
// Modification History
// --------------------
// 2018/01/04, Larry Create
// --------------------
// ==============================================================================
void utilChristie_CLI_Init(void)
{
    UINT16 uiCount = 0;

    for(uiCount = 0; uiCount<eccNumber; uiCount++)
    {
        memset(&m_sCLI_Info[uiCount], 0, sizeof(sCLI_DATA_INFO));  //G100_Simon_0060
        m_sCLI_Info[uiCount].ucSerialPortEcho = 0;
    }

    for(uiCount = 0; uiCount < CHRISTIE_CMD_LUT_NUMBER; uiCount++)
    {
        UINT16 uiMainCount = 0;

        for(uiMainCount = 0; uiMainCount < CLI_MAINLUT_NUMBER; uiMainCount++)
        {
            if(m_sSubLut_Christie[uiCount].uiMainCmdID == m_sMainLut[uiMainCount].uiMainCmdID)
            {
                if(CmdCfg_CommandSupport_Get(m_sMainLut[uiMainCount].cMainCmd, m_sSubLut_Christie[uiCount].cSubCmd, m_sSubLut_Christie[uiCount].ucCmdIsRead))
                {
                    m_sSubLut_Christie[uiCount].uiSpecialFlag |= CLI_COM_ENABLE;
                    break;
                }
            }
        }

    }
}

// ==============================================================================
// FUNCTION NAME: utilChristie_CLI_SerialPortEcho
// DESCRIPTION:
//
//
// Params:
// eCLI_CHANNEL eCh:
// UINT8 ucEcho:
//
// Returns:
//
//
// Modification History
// --------------------
// 2018/01/04, Larry Create
// --------------------
// ==============================================================================
void utilChristie_CLI_SerialPortEcho(UINT8 eCh, UINT8 ucEcho)
{
    m_sCLI_Info[eCh].ucSerialPortEcho = ucEcho;
}

// ==============================================================================
// FUNCTION NAME: utilChristie_CLI_Execute
// DESCRIPTION:
//
//
// Params:
//
// Returns:
//
//
// Modification History
// --------------------
// 2021/09/02, Larry Create
// --------------------
// ==============================================================================
UINT8 utilChristie_CLI_Execute(UINT8 cCh, UINT8* pcString, char* pcReturnString, sCLI_CHRISTIE_FORMAT* sCmdFormat)
{
    UINT8 ucErrorCode = eCLI_ERROR_CODE_NO;

    utilOPD_CLI_EventSet((eOPD_CLI_RS232_LOG + cCh), (char*)pcString);

    //G100_Wilsonj_0050 Start
    if((sCmdFormat->ulProjectorAddressID == 0) ||
       (sCmdFormat->ulProjectorAddressID == 65535) || //A35G2_Larry_0078
       (sCmdFormat->ulProjectorAddressID == m_ulAddressID))
    {
        if(ucErrorCode == eCLI_ERROR_CODE_NO)
        {
            ucErrorCode = utilChristie_CMD_Handle(sCmdFormat);
        }

        LOG_MSG(db_APP_CLI, "(%s, %d) (%d)\r\n", __FUNCTION__, __LINE__, ucErrorCode);

        utilChristie_CMD_Reply(cCh, ucErrorCode, sCmdFormat, pcReturnString);
        __CMD_Respond(cCh, pcReturnString);

    	if(ucErrorCode == eCLI_ERROR_CODE_NO) //A70LV_Larry_0376
        {
            if((edcLENS_CALIBRATION == m_sSubLut_Christie[sCmdFormat->uiFuncID].uiFunCodeID) && (sCmdFormat->ucIsRead == WRITE_COMMAND)) //A70LV_Larry_0315
            {
                palDataMgr_UI_EventSend(edcUI_EVENT_MENUOPEN_LENS_CAL_MSG, TRUE, NULL); //appGui_Send_LensCalibrationMenuOpen();
            }
            else if((edcLENS_APPLY_POSITION == m_sSubLut_Christie[sCmdFormat->uiFuncID].uiFunCodeID) && (sCmdFormat->ucIsRead == WRITE_COMMAND))
            {
                palDataMgr_UI_EventSend(edcUI_EVENT_MENUOPEN_LENS_MOVING_MSG, TRUE, NULL); //appGui_Send_LensMovingMenuOpen();
            }
            else if((edcLENS_TOP_BOTTOM_CENTER == m_sSubLut_Christie[sCmdFormat->uiFuncID].uiFunCodeID) && (sCmdFormat->ucIsRead == WRITE_COMMAND)//A35G2_Wesley_0153
            && (eFUNC_CONTROL_ENABLE == palDataMgr_DataCode_Control(edcLENS_TOP_BOTTOM_CENTER)))// A35G2_Bruce_0008 //A70Gen2_Julie_0109//HICC2_Julie_0014
            {
                palDataMgr_UI_EventSend(edcUI_EVENT_MENUOPEN_LENS_MOVING_MSG, TRUE, NULL); //appGui_Send_LensMovingMenuOpen();
            }
            else if(ucErrorCode == eCLI_ERROR_CODE_NO && sCmdFormat->ucIsRead == WRITE_COMMAND) //A70LV_Larry_0213
            {
                palEnvironment_AutoShutDownClear();	//A35G2_CDS_Coda_0034
                if(m_sSubLut_Christie[sCmdFormat->uiFuncID].uiMainCmdID != eCLI_MAIN_KEY &&
                   m_sSubLut_Christie[sCmdFormat->uiFuncID].uiMainCmdID != eCLI_MAIN_PRO) //A70LV_Larry_0427
                {
                    palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD_CLI, TRUE, NULL); //appGui_SendUpdateOSD_CLI_Event();
                }
            }
		}
    }

    return ucErrorCode;
}

// ==============================================================================
// FUNCTION NAME: utilChristie_CLI_Process
// DESCRIPTION:
//
//
// Params:
// eCLI_CHANNEL eCh:
// UINT8* pcString:
// sCLI_CHRISTIE_FORMAT* sCmdFormat:
//
// Returns:
//
//
// Modification History
// --------------------
// 2018/11/23, Larry Create
// --------------------
// ==============================================================================
UINT8 utilChristie_CLI_Process(UINT8 cCh, UINT8* pcString, char* pcReturnString, sCLI_CHRISTIE_FORMAT* sCmdFormat, UINT16 uiType)
{
    UINT8 ucErrorCode = eCLI_ERROR_CODE_NO;

	utilOPD_CLI_EventSet((eOPD_CLI_RS232_LOG + cCh), (char*)pcString); //G100_Julie_0041

    ucErrorCode = utilChristie_CMD_Decode(pcString, sCmdFormat);

    if(ucErrorCode == eCLI_ERROR_CODE_NO)
    {
        if(sCmdFormat->ucIsRead == WRITE_COMMAND)
        {
            if(uiType & CLI_EXECTUE_WRITE_CMD)
            {
                sCmdFormat->ucCmdFrom = cCh;
                ucErrorCode = utilChristie_CLI_Execute(cCh, pcString, pcReturnString, sCmdFormat);

                if(m_sSubLut_Christie[sCmdFormat->uiFuncID].uiSpecialFlag & CLI_COM_CHECK_BUSY)
                {
                    m_ucWriteBusy = 0;
                }
            }
            else if(uiType & CLI_EXECTUE_WRITE_BUFFER)
            {
                if(m_ucWriteBusy)
                {
                    ucErrorCode = eCLI_ERROR_CODE_SETFAIL;
                    utilChristie_CMD_Reply(cCh, ucErrorCode, sCmdFormat, pcReturnString);
                    __CMD_Respond(cCh, pcReturnString);

                    return ucErrorCode;
                }

                if(m_sSubLut_Christie[sCmdFormat->uiFuncID].uiSpecialFlag & CLI_COM_CHECK_BUSY)
                {
                    m_ucWriteBusy = 1;
                }

                utilCommon_CLI_Callback(cCh, eCLI_STYLE_CHRISTIE, pcString, strlen((char*)pcString)); //A35G2_Coda_0045
            }
        }
        else //read
        {
            if(uiType & CLI_EXECTUE_READ_CMD)
            {
                ucErrorCode = utilChristie_CLI_Execute(cCh, pcString,pcReturnString, sCmdFormat);
            }
            else if(uiType & CLI_EXECTUE_READ_BUFFER)
            {
                utilCommon_CLI_Callback(cCh, eCLI_STYLE_CHRISTIE, pcString, strlen((char*)pcString));	//A35G2_Coda_0045
            }
        }
    }
    else if(ucErrorCode != eCLI_ERROR_CODE_NO)
    {
        utilChristie_CMD_Reply(cCh, ucErrorCode, sCmdFormat, pcReturnString);
        __CMD_Respond(cCh, pcReturnString);
    }

    palDataMgr_Data_Access(edcLAST_SERIAL_CMD_ERRORCODE, edaWRITE_RAM_ONLY_NO_ACTION, (void *)&ucErrorCode); //save last serial cmd error code   //T100_Simon_0002

    return ucErrorCode;
}

void utilChristie_CLI_AMX(UINT8 ucData)
{
	switch(ucData)
	{
		case 'A':
			ucAMX_Step = eCLI_DECODE_AMX_STEP_M;
			break;

		case 'M':
			if(ucAMX_Step == eCLI_DECODE_AMX_STEP_M)
			{
				ucAMX_Step = eCLI_DECODE_AMX_STEP_X;
			}
			else
			{
				ucAMX_Step = eCLI_DECODE_AMX_STEP_A;
			}
			break;

		case 'X':
			if(ucAMX_Step == eCLI_DECODE_AMX_STEP_X)
			{
				ucAMX_Step = eCLI_DECODE_AMX_STEP_END;
			}
			else
			{
				ucAMX_Step = eCLI_DECODE_AMX_STEP_A;
			}
			break;

		case '\r':
			if(ucAMX_Step == eCLI_DECODE_AMX_STEP_END)
			{
			    //UINT8 ucCustomID = palSystem_CustomIDGet();
			    UINT8 ucString[32] ={0};
			    palDataMgr_Data_Access(edcMODEL_NAME, edaREAD, ucString); //A35G2_CDS_Coda_0031
#if 1 //HICC2_Steven_0024
			    LOG_MSG(db_ALWAYS, AMXB_STR_CHRISTIE, ucString);
#else
			    if(ucCustomID < eMODEL_TYPE_NUMBER)
			    {
                    LOG_MSG(db_ALWAYS, AMXB_STR_CHRISTIE, ucString);
                }
                else
                {
                    LOG_MSG(db_ALWAYS, AMXB_STR_CHRISTIE, "UNKNOWN");
                }
#endif
			}
            ucAMX_Step = eCLI_DECODE_AMX_STEP_A;
			break;

		default:
			ucAMX_Step = eCLI_DECODE_AMX_STEP_A;
			break;
	}
}

#endif //defined(CUSTOM_CHRISTIE)

