// ===============================================================================
// FILE NAME: utilGeneralMSSC.c
// DESCRIPTION:
//
//
// Modification History
// --------------------
// 2017/10/08, Larry Create
// --------------------
// ===============================================================================

#include "utilCommonMSSCAPI.h"

#ifdef GENERAL_MSSC_CLI

#include "halScaler.h"
#include "utilCommon.h"
//#include "appGui.h"

#include "appSystem.h"
#include "appDataMgr.h"
#include "appInputProc.h"
#include "appLANProcAPI.h"
#include "appDataPath.h"
#include "appPoll.h"
#include "appIllumination.h"

#include "halFrontEndCtrlAPI.h"
#include "halMotorCtrlAPI.h"
#include "halFormatter.h" //T100_Casper_0008
#include "halInputCtrlAPI.h"
#include "halMCUCtrlAPI.h"
#include "halWarping.h"
#ifdef SCALER_C821_C789
#include "halC789CtrlAPI.h"
#endif
#include "halGui.h"
#include "dvVCXO949.h"
#include "dvC341.h"   //H2PF_Simon_0193

#include "utilDbgMsg.h"
#include "utilHPBU_Tester.h"
#include "utilHostAPI.h"
#include "opdCtrlAPI.h"
#include "utilOPD_TEST.h"
#include "utilDataMgrAPI.h"
#include "utilDatabaseAPI.h"
#include "utilGeneralMSSCAPI.h"

#include "Board_Uart.h" //G100_Steven_0002
#include "appEnvironment.h"	//G100_Clare_0055

#include "GEC_CoreFunction.h" //G100_Steven_0007 //T100IR_Casper_0021

#ifdef SCALER_FPGA_F34
#include "utilWarpDemoProAV.h"
#else
#include "utilWarpDemo.h"
#endif

#include "CustomDefCLI.h"

#include "palCoreVar.h"
#include "palGui.h"
#include "palGeoAPI.h"
#include "palImgMgr.h"
#include "palMotorMgr.h"

#include "ProjectSettings.h"

sGUI_CALLBACK GuiCb;

#define GENERAL_CMD_REQUEST        '?'
#define GENERAL_CMD_REPLY          '!'
#define GENERAL_CMD_SPACE          ' '

#define GENERAL_CMD_DIS_CHAR_1     '\\'

#define GENERAL_CMD_RPR_CHAR_SIMPLE_ACK    '$'
#define GENERAL_CMD_RPR_CHAR_FULL_ACK      '#'
#define GENERAL_CMD_RPR_CHAR_CHECK_SUM     '&'

#define GENERAL_CMD_DATA_P  'P'
#define GENERAL_CMD_DATA_N  'N'
#define GENERAL_CMD_DATA_p  'p'
#define GENERAL_CMD_DATA_n  'n'

#define GENERAL_CMD_DATA_STRING    '"'
#define GENERAL_CMD_SUB_CONNECT    '+'
#define GENERAL_CMD_LENGTH_MAX     (230)
#define GENERAL_CMD_LENGTH_MIN     (6) //(SYS?)\0


#define CLI_COM_NORMAL                  (0)
#define CLI_COM_LENS                    CLI_COM_NORMAL | CLI_COM_TYPE_PN_ONLY | CLI_COM_WRITE_ONLY

#define GENERAL_MSSC_VER                (1)

#define DATA_CODE_NA                    (0xFFFF)

#define STR_COMPARE_MATCH               (0)                   //B35LC_Tim_0043, add
#define POWR_CMD                        "POWR"                //B35LC_Tim_0043, add

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
    eCLI_ERROR_CODE (*iCLI_Replay)(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat);

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
// 2017/11/08, Larry Create
// --------------------
// ==============================================================================
const static sCLI_CONFIG m_sCli_General_Config[eccNumber] =
{
    {eccPC, 	 GENERAL_CMD_HEADER_1, GENERAL_CMD_END_1},
    {eccTelnet,  GENERAL_CMD_HEADER_1, GENERAL_CMD_END_1},
    {eccHDBaseT, GENERAL_CMD_HEADER_1, GENERAL_CMD_END_1},  //G100_Owen_0016
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
};

static UINT8 ucAMX_Step = eCLI_DECODE_AMX_STEP_A;

typedef enum
{
    eCLI_MAIN_SYS,
    eCLI_MAIN_SZP,
    //eCLI_MAIN_OVS,
    //eCLI_MAIN_PXT,
    //eCLI_MAIN_PXP,
    eCLI_MAIN_HOR,
    eCLI_MAIN_VRT,
    eCLI_MAIN_DZH,
    eCLI_MAIN_DZV,
    eCLI_MAIN_DSH,
    eCLI_MAIN_DSV,
    eCLI_MAIN_PCM,
    eCLI_MAIN_WRP,
    eCLI_MAIN_AWF,
    eCLI_MAIN_MWF,
    eCLI_MAIN_HPC,
    eCLI_MAIN_VPC,
    eCLI_MAIN_TLCX, //G100_Wilsonj_0032
    eCLI_MAIN_TLCY, //G100_Wilsonj_0032
    eCLI_MAIN_TRCX, //G100_Wilsonj_0032
    eCLI_MAIN_TRCY, //G100_Wilsonj_0032
    eCLI_MAIN_BLCX, //G100_Wilsonj_0032
    eCLI_MAIN_BLCY, //G100_Wilsonj_0032
    eCLI_MAIN_BRCX, //G100_Wilsonj_0032
    eCLI_MAIN_BRCY, //G100_Wilsonj_0032
    eCLI_MAIN_CNR,
    eCLI_MAIN_WRE,
    eCLI_MAIN_AIM,
    eCLI_MAIN_BRT,
    eCLI_MAIN_CON,
    eCLI_MAIN_CSP,
    eCLI_MAIN_DTL,
    eCLI_MAIN_TIN, //T100IR_Casper_0032
    eCLI_MAIN_TDN,
    eCLI_MAIN_TDE,
    eCLI_MAIN_TDD,
    eCLI_MAIN_TDI,
    eCLI_MAIN_SIS, //A35G2_BRC_Casper_0016
    eCLI_MAIN_SOS,
    eCLI_MAIN_FDY,
    eCLI_MAIN_LRR, //T100_Casper_0062
    eCLI_MAIN_TDDL, //G100_Wilsonj_0032
    eCLI_MAIN_TDR,
    //eCLI_MAIN_CLR,
    //eCLI_MAIN_TNT,
    //eCLI_MAIN_NDR,
    //eCLI_MAIN_FTC,
    //eCLI_MAIN_VBL,
    //eCLI_MAIN_FMD,
    //eCLI_MAIN_CLC,
    eCLI_MAIN_ROG,
    eCLI_MAIN_GOG,
    eCLI_MAIN_BOG,
    eCLI_MAIN_ROO,
    eCLI_MAIN_GOO,
    eCLI_MAIN_BOO,
    //eCLI_MAIN_SYT,
    eCLI_MAIN_GOR,
    eCLI_MAIN_DPMO,
    eCLI_MAIN_DPSU, //G100_Wilsonj_0032
    eCLI_MAIN_FPS,
    eCLI_MAIN_DIM,
    eCLI_MAIN_FRZ,
    eCLI_MAIN_BGC,
    eCLI_MAIN_WHPK,
    eCLI_MAIN_CCI,
	eCLI_MAIN_LOT, //T100_Casper_0062
	eCLI_MAIN_LOL, //T100_Casper_0062
    //eCLI_MAIN_EDG,
    //eCLI_MAIN_CWS,
    eCLI_MAIN_HSG,
    //eCLI_MAIN_VBD,
    eCLI_MAIN_LOC,
    eCLI_MAIN_LET, //T100_Casper_0063
    //eCLI_MAIN_FCS,
    eCLI_MAIN_FCSI, //G100_Wilsonj_0035
    eCLI_MAIN_FCSO, //G100_Wilsonj_0035
    //eCLI_MAIN_ZOM,
    eCLI_MAIN_ZOMI, //G100_Wilsonj_0035
    eCLI_MAIN_ZOMO, //G100_Wilsonj_0035
    eCLI_MAIN_LVO,
    eCLI_MAIN_LHO,
    eCLI_MAIN_LVU, //T100_Casper_0099
    eCLI_MAIN_LVD, //T100_Casper_0099
    eCLI_MAIN_LHL, //T100_Casper_0099
    eCLI_MAIN_LHR, //T100_Casper_0099
    //eCLI_MAIN_LMR,
    eCLI_MAIN_LMA,
    eCLI_MAIN_LMS,
    eCLI_MAIN_LCB,
    eCLI_MAIN_LLK, //G100_Wilsonj_0032
    eCLI_MAIN_CEL,
    eCLI_MAIN_SOR,
    //eCLI_MAIN_LED,
    eCLI_MAIN_MSH,
    eCLI_MAIN_MSV,
    eCLI_MAIN_MBE,
    eCLI_MAIN_OST,
    eCLI_MAIN_SPSG,
    eCLI_MAIN_BGCL,
    eCLI_MAIN_BGRT,
    eCLI_MAIN_PWR,
    eCLI_MAIN_SBPM, //G100_Wilsonj_0041
    eCLI_MAIN_APW,
    eCLI_MAIN_ASH,
    eCLI_MAIN_SLP,
    eCLI_MAIN_HIAL,
    //eCLI_MAIN_IRC,
    eCLI_MAIN_FRNT, //G100_Wilsonj_0035
    eCLI_MAIN_TOPP, //G100_Wilsonj_0035
    eCLI_MAIN_HDBT, //G100_Wilsonj_0035
    eCLI_MAIN_VTT,
    eCLI_MAIN_NET,
    eCLI_MAIN_LMAC, //G100_Wilsonj_0035
    eCLI_MAIN_LDHC, //G100_Wilsonj_0035
    eCLI_MAIN_LIPA, //G100_Wilsonj_0035
    eCLI_MAIN_LSUB, //G100_Wilsonj_0035
    eCLI_MAIN_LGAT, //G100_Wilsonj_0035
    eCLI_MAIN_DNSG, //G100_Wilsonj_0035
    eCLI_MAIN_LAPY, //G100_Wilsonj_0035
    //eCLI_MAIN_NTW,
    eCLI_MAIN_BDR,
    eCLI_MAIN_SEC,
    eCLI_MAIN_SPP,
    eCLI_MAIN_SLA,
    eCLI_MAIN_PJP,
    eCLI_MAIN_ADR,
    eCLI_MAIN_KBL,
    eCLI_MAIN_SBL,
    eCLI_MAIN_HGWP,
    eCLI_MAIN_HGRH, //G100_Wilsonj_0032
    eCLI_MAIN_HGRS, //G100_Wilsonj_0032
    eCLI_MAIN_HGRG, //G100_Wilsonj_0032
    eCLI_MAIN_HGGH, //G100_Wilsonj_0032
    eCLI_MAIN_HGGS, //G100_Wilsonj_0032
    eCLI_MAIN_HGGG, //G100_Wilsonj_0032
    eCLI_MAIN_HGBH, //G100_Wilsonj_0032
    eCLI_MAIN_HGBS, //G100_Wilsonj_0032
    eCLI_MAIN_HGBG, //G100_Wilsonj_0032
    eCLI_MAIN_HGCH, //G100_Wilsonj_0032
    eCLI_MAIN_HGCS, //G100_Wilsonj_0032
    eCLI_MAIN_HGCG, //G100_Wilsonj_0032
    eCLI_MAIN_HGYH, //G100_Wilsonj_0032
    eCLI_MAIN_HGYS, //G100_Wilsonj_0032
    eCLI_MAIN_HGYG, //G100_Wilsonj_0032
    eCLI_MAIN_HGMH, //G100_Wilsonj_0032
    eCLI_MAIN_HGMS, //G100_Wilsonj_0032
    eCLI_MAIN_HGMG, //G100_Wilsonj_0032
    eCLI_MAIN_HGWR, //G100_Wilsonj_0032
    eCLI_MAIN_HGWG, //G100_Wilsonj_0032
    eCLI_MAIN_HGWB, //G100_Wilsonj_0032
	eCLI_MAIN_CCAG,
    //eCLI_MAIN_MDR,
    //eCLI_MAIN_MAR,
    eCLI_MAIN_HAR,
    eCLI_MAIN_HSR, //A35G2_BRC_Casper_0114
    eCLI_MAIN_HKS,
    eCLI_MAIN_LGC, //T100IR_Casper_0032
    eCLI_MAIN_PIF,
    eCLI_MAIN_MDNA, //G100_Wilsonj_0042
    eCLI_MAIN_SERI, //G100_Wilsonj_0042
    eCLI_MAIN_FWIS, //G100_Wilsonj_0042
    eCLI_MAIN_FWMF, //G100_Wilsonj_0042
    //eCLI_MAIN_FWMM, //G100_Wilsonj_0042
    //eCLI_MAIN_FWML, //G100_Wilsonj_0042
    eCLI_MAIN_FWMA, //G100_Wilsonj_0042
    eCLI_MAIN_FWMK, //G100_Wilsonj_0042
    eCLI_MAIN_FWLA, //G100_Wilsonj_0042
    eCLI_MAIN_FWFM, //G100_Wilsonj_0042
    eCLI_MAIN_FWFA, //G100_Wilsonj_0042
    eCLI_MAIN_FWFB, //G100_Wilsonj_0042
    //eCLI_MAIN_FWFC, //G100_Wilsonj_0042
    eCLI_MAIN_FWFX, //G100_Wilsonj_0042
    eCLI_MAIN_FWHD, //G100_Wilsonj_0042
    eCLI_MAIN_DEF,
    eCLI_MAIN_ITP,
    eCLI_MAIN_PSID,
    eCLI_MAIN_FTID,
    eCLI_MAIN_ERR,
    //eCLI_MAIN_MDT,
    eCLI_MAIN_LDI,
    eCLI_MAIN_FAN,
    eCLI_MAIN_UST, //A35G2_BRC_Casper_0088
    eCLI_MAIN_LPM,
    eCLI_MAIN_LPP,
    eCLI_MAIN_LPI,
    eCLI_MAIN_LLP, //T100IR_Casper_0029
    eCLI_MAIN_LLI, //T100IR_Casper_0029
    eCLI_MAIN_BLE, //T100IR_Casper_0032
    eCLI_MAIN_IRE, //T100IR_Casper_0032
    eCLI_MAIN_LIF,
    eCLI_MAIN_RLC,
    eCLI_MAIN_SST,
    eCLI_MAIN_SIN,
    eCLI_MAIN_PIP,
    eCLI_MAIN_PPS,
    eCLI_MAIN_PHS,
    eCLI_MAIN_PPP,
    //eCLI_MAIN_TMG,
    eCLI_MAIN_MIF,
    eCLI_MAIN_MSRS, //G100_Wilsonj_0042
    eCLI_MAIN_MSSF, //G100_Wilsonj_0042
    eCLI_MAIN_MSPC, //G100_Wilsonj_0042
    eCLI_MAIN_MSHR, //G100_Wilsonj_0042
    eCLI_MAIN_MSVR, //G100_Wilsonj_0042
    eCLI_MAIN_MSCS, //G100_Wilsonj_0042
    eCLI_MAIN_SIF,
    eCLI_MAIN_SSRS, //G100_Wilsonj_0042
    eCLI_MAIN_SSSF, //G100_Wilsonj_0042
    eCLI_MAIN_SSPC, //G100_Wilsonj_0042
    eCLI_MAIN_SSHR, //G100_Wilsonj_0042
    eCLI_MAIN_SSVR, //G100_Wilsonj_0042
    eCLI_MAIN_SSCS, //G100_Wilsonj_0042
    eCLI_MAIN_SKS,
    //eCLI_MAIN_BSS,
    eCLI_MAIN_UID,
    eCLI_MAIN_SIV,
    eCLI_MAIN_LCE,
    eCLI_MAIN_LSE,
    eCLI_MAIN_SNS,
    eCLI_MAIN_KEY,
    eCLI_MAIN_SHU,
    eCLI_MAIN_FCT,
    eCLI_MAIN_OSD,
    //eCLI_MAIN_NRD,
    eCLI_MAIN_TTT,
    //eCLI_MAIN_CSS,
    eCLI_MAIN_CFU,
    eCLI_MAIN_EXF,
    eCLI_MAIN_BUR,
    //eCLI_MAIN_GUI,
    //eCLI_MAIN_BLD,
    //eCLI_MAIN_BLS,
    //eCLI_MAIN_MMC,
    //eCLI_MAIN_HDR,
    eCLI_MAIN_MTO,
    eCLI_MAIN_ILI,
    eCLI_MAIN_ICI,  //T100_Doulas_0035
    eCLI_MAIN_DRD,  //T100_Casper_0008
    eCLI_MAIN_DBG,  //T100_Simon_0024
    eCLI_MAIN_WAP,  //T100_Simon_0031
    eCLI_MAIN_WAS,  //T100_Simon_0031
    eCLI_MAIN_KSH, //G100_Wilsonj_0032
    eCLI_MAIN_KSV, //G100_Wilsonj_0032
    //eCLI_MAIN_CAM,  //T100_Simon_0063
    //eCLI_MAIN_ICP,
    //eCLI_MAIN_ICD,
    eCLI_MAIN_LLM,
	eCLI_MAIN_EBL,
    eCLI_MAIN_FKC, //T100_Coda
    eCLI_MAIN_FVI,
    eCLI_MAIN_CCT,
    eCLI_MAIN_COL, //T100IR_Casper_0032
    eCLI_MAIN_MLT, //T100IR_Casper_0032
    eCLI_MAIN_HOE, //T100IR_Casper_0032
    eCLI_MAIN_HSE, //T100IR_Casper_0032
    //eCLI_MAIN_HBE, //T100IR_Casper_0032
    eCLI_MAIN_DYBK, //G100_Wilsonj_0032
    eCLI_MAIN_EXBK, //G100_Wilsonj_0032
    eCLI_MAIN_DCSP, //G100_Wilsonj_0032
    eCLI_MAIN_DCST, //G100_Wilsonj_0032
    eCLI_MAIN_DCLE, //G100_Wilsonj_0032
    eCLI_MAIN_DCLT, //G100_Wilsonj_0032
    eCLI_MAIN_DCLS, //G100_Wilsonj_0032
    eCLI_MAIN_MSRC, //G100_Wilsonj_0032
    eCLI_MAIN_SSRC, //G100_Wilsonj_0032
    eCLI_MAIN_EDMA, //G100_Wilsonj_0033
    eCLI_MAIN_IST,  //G100_Julie_0001
    eCLI_MAIN_SPON, //G100_Wilsonj_0040
    eCLI_MAIN_PINM, //G100_Wilsonj_0041
	eCLI_MAIN_PIND, //G100_Wilsonj_0041
	eCLI_MAIN_PINH, //G100_Wilsonj_0041
	eCLI_MAIN_LPTH, //G100_Wilsonj_0042
	eCLI_MAIN_LEPT, //G100_Wilsonj_0042 //A35G2_BRC_Casper_0147
	eCLI_MAIN_SSTP, //G100_Wilsonj_0042
	//eCLI_MAIN_SSAC, //G100_Wilsonj_0046
    eCLI_MAIN_LSNT, //G100_Larry_0006
    eCLI_MAIN_LSET, //G100_Larry_0006
    eCLI_MAIN_LSQT, //G100_Larry_0006
    eCLI_MAIN_LSCT, //G100_Larry_0006
    eCLI_MAIN_PPZM, //G100_Wilsonj_0060
    eCLI_MAIN_HKST, //G100_Wilsonj_0060
    eCLI_MAIN_HKSB, //G100_Wilsonj_0060
	eCLI_MAIN_OPD,
	eCLI_MAIN_CREN, //G100_Wilsonj_0061
	eCLI_MAIN_CRIP, //G100_Wilsonj_0061
	eCLI_MAIN_CRID, //G100_Wilsonj_0061
	eCLI_MAIN_CRPO, //G100_Wilsonj_0061
	eCLI_MAIN_CRAP, //G100_Wilsonj_0061
	eCLI_MAIN_EXEN, //G100_Wilsonj_0061
	eCLI_MAIN_PJEN, //G100_Wilsonj_0061
	eCLI_MAIN_PJIP, //G100_Wilsonj_0061
	eCLI_MAIN_PJAP, //G100_Wilsonj_0061
	eCLI_MAIN_AXEN, //G100_Wilsonj_0061
	eCLI_MAIN_TEEN, //G100_Wilsonj_0061
	eCLI_MAIN_HTEN, //G100_Wilsonj_0061
	eCLI_MAIN_CNRT, //G100_Wilsonj_0061
    eCLI_MAIN_LSHS, //G100_Larry_0007
    eCLI_MAIN_PMUT, //G100_Larry_0007
    eCLI_MAIN_SHFI, //G100_Larry_0007
    eCLI_MAIN_SHFO, //G100_Larry_0007
    eCLI_MAIN_SCRT,
    eCLI_MAIN_GCRT,
    eCLI_MAIN_LMNS,
//    eCLI_MAIN_LCCR,
//    eCLI_MAIN_LCEX,
//    eCLI_MAIN_LCPJ,
//    eCLI_MAIN_LCAX,
//    eCLI_MAIN_LCTE,
//    eCLI_MAIN_LCHT,
    eCLI_MAIN_LANI,
    eCLI_MAIN_SYRT,
    eCLI_MAIN_FRST,
    eCLI_MAIN_DPRT,
    eCLI_MAIN_IMRT,
    eCLI_MAIN_CMRT,
    eCLI_MAIN_PSRT,
    eCLI_MAIN_BKLV,

    eCLI_MAIN_LMRT,	//G100_Coda_0005
    eCLI_MAIN_LERT,
    eCLI_MAIN_BKSW,
    eCLI_MAIN_BKCS,
    eCLI_MAIN_BKFI,
    eCLI_MAIN_BKFR,
    eCLI_MAIN_BKFH,
    eCLI_MAIN_BKFC,
    eCLI_MAIN_BKFD, //A35G2_BRC_Simon_0004
    eCLI_MAIN_BKSI,
    eCLI_MAIN_BKSR,
    eCLI_MAIN_BKSH,
    eCLI_MAIN_BKSC,
    eCLI_MAIN_BKSD, //A35G2_BRC_Simon_0004
    eCLI_MAIN_BKIS,
    eCLI_MAIN_BKIC, //A35G2_BRC_Simon_0004
    //eCLI_MAIN_HSWP,
    eCLI_MAIN_DCRT,
    eCLI_MAIN_TDRT,
    eCLI_MAIN_NFRS,
    eCLI_MAIN_BALP,
    eCLI_MAIN_PINP,
    eCLI_MAIN_PINC,

    eCLI_MAIN_PHAS,//---G100_Coda_0007
    eCLI_MAIN_HPOS,
    eCLI_MAIN_VPOS,
    eCLI_MAIN_DVRS,
    eCLI_MAIN_DHOP,
    eCLI_MAIN_DHEA,
    eCLI_MAIN_DHEB,
    eCLI_MAIN_DHSL,
    //eCLI_MAIN_DHBT,
    eCLI_MAIN_DHBL,
    eCLI_MAIN_ASRS,
    eCLI_MAIN_HDRG,
    eCLI_MAIN_HDRP,
    eCLI_MAIN_LLTC,
    eCLI_MAIN_MNTP,
    eCLI_MAIN_WWCG,			//G100_Doulas_0027 Add
    eCLI_MAIN_WGPG,
    eCLI_MAIN_WWIG,
    eCLI_MAIN_WWSG,
    eCLI_MAIN_WGCG,
    eCLI_MAIN_WBCG,
    eCLI_MAIN_GMSG,
	eCLI_MAIN_GMAG,
	eCLI_MAIN_GMCG,
    eCLI_MAIN_ICPG,
	eCLI_MAIN_TARG,
	eCLI_MAIN_DTCM,

    eCLI_MAIN_DTSY,
    eCLI_MAIN_DTSM,
    eCLI_MAIN_DTSD,
    eCLI_MAIN_DTTH,
    eCLI_MAIN_DTTM,

    eCLI_MAIN_DTDS,
    eCLI_MAIN_DTNS,
    eCLI_MAIN_DTTZ,
    eCLI_MAIN_DTUI,
    eCLI_MAIN_DTAP,
    eCLI_MAIN_DTIF,
    eCLI_MAIN_SCW0,
    eCLI_MAIN_SCW1,
    eCLI_MAIN_SCW2,
    eCLI_MAIN_SCW3,
    eCLI_MAIN_SCW4,
    eCLI_MAIN_SCW5,
    eCLI_MAIN_SCW6,

    eCLI_MAIN_SCHE,
    eCLI_MAIN_ATFC,	//G100_Clare_0028
    eCLI_MAIN_ATWC,	//G100_Clare_0028

    eCLI_MAIN_SHSP, //G100_Coda_0054

    eCLI_MAIN_LOGC, //G100_Owen_0049
    eCLI_MAIN_LOGS, //G100_Owen_0049
    eCLI_MAIN_LOGD, //G100_Owen_0049

#if defined (CUSTOM_OPTOMA) || defined(CUSTOM_BARCO) //A35G2_BRC_Casper_0039 //A35G2_BRC_Casper_0152
    eCLI_MAIN_CAST,    //G100_Clare_0036
    eCLI_MAIN_AFST,    //G100_Clare_0036
    eCLI_MAIN_WCST,    //G100_Clare_0036
#endif

	eCLI_MAIN_DPAU,    //G100_Steven_0030
    eCLI_MAIN_FWVR,		//G100_Clare_0039
    //eCLI_MAIN_CAVR,		//G100_Clare_0039
    eCLI_MAIN_UDSA,		//G100_Doulas_0062
    eCLI_MAIN_UDLD,		//G100_Doulas_0062
	eCLI_MAIN_WCOL,		//G100_Doulas_0067
    eCLI_MAIN_WROW,		//G100_Doulas_0067
	eCLI_MAIN_SEQM,     //G100_Steven_0048 start
	eCLI_MAIN_BRPM,
	eCLI_MAIN_BGPM,
	eCLI_MAIN_BBPM,
	eCLI_MAIN_BYPM,
	eCLI_MAIN_RRPM,
	eCLI_MAIN_RYPM,
	eCLI_MAIN_LEDT,
	eCLI_MAIN_SSPA,
	eCLI_MAIN_SSRH,
	eCLI_MAIN_DDTP,
	//eCLI_MAIN_FANF,     //G100_Steven_0048 end
	eCLI_MAIN_FPON,	//G100_Clare_0055
	eCLI_MAIN_PBRI, //G100_Steven_0079
	eCLI_MAIN_PCON, //G100_Steven_0079
	eCLI_MAIN_PCSP, //G100_Steven_0079

    eCLI_MAIN_WPTP, //G100_Owen_0089 PToolset start //A35G2_BRC_Casper_0046
    eCLI_MAIN_WPPU,
    eCLI_MAIN_WPPD,
    eCLI_MAIN_WPPL,
    eCLI_MAIN_WPPR,
    eCLI_MAIN_WPXY,
    eCLI_MAIN_WRST,

	eCLI_MAIN_LMCP,  //G100_Steven_0098 //A35G2_BRC_Casper_0047
	eCLI_MAIN_GMCC,
	eCLI_MAIN_UDCC,

	eCLI_MAIN_HUTP,  //G100_Steven_0118 //A35G2_BRC_Casper_0051
	eCLI_MAIN_DDTC,  //G100_Steven_0118 //A35G2_BRC_Casper_0051

    eCLI_MAIN_AHSW,  	//G100_Tim_0057, add //A35G2_BRC_Casper_0060
    eCLI_MAIN_PJNT,   	//G100_Tim_0058, add //A35G2_BRC_Casper_0060
    eCLI_MAIN_BODG,

    eCLI_MAIN_USTS, //SNPLU9000_Energy_0011 //A35G2_BRC_Casper_0088
    eCLI_MAIN_OPRA,//A35G2_Coda_0110
    eCLI_MAIN_CBMP, //A35G2_BRC_Casper_0145
	eCLI_MAIN_TDAR,  //G100_Steven_0163 //A35G2_BRC_Casper_0146

#if (ENABLE_COLOR_UNIFORMITY == TRUE) //G100_Tim_0012, add, start //A35G2_BRC_Casper_0152
    eCLI_MAIN_ACUE,                     // Execute ACU
    eCLI_MAIN_ACUS,                     // Access ACU status
    eCLI_MAIN_ACUT,                     // Select ACU target for calibration
    eCLI_MAIN_ACUR,                     // Reset ACU data and status
    //eCLI_MAIN_ACUW,                     // save ACU data
#endif //ENABLE_COLOR_UNIFORMITY      //G100_Tim_0012, add, end

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
    {eCLI_MAIN_SYS,  "SYSG"},
    {eCLI_MAIN_SZP,  "ASPR"},    //Aspect Ratio
    //{eCLI_MAIN_OVS,  "OVSG"},
    //{eCLI_MAIN_PXT,  "PXTG"},
    //{eCLI_MAIN_PXP,  "PXPG"},
    {eCLI_MAIN_HOR,  "HORG"},
    {eCLI_MAIN_VRT,  "VRTG"},
    {eCLI_MAIN_DZH,  "HDZM"},    //Digital Zoom Horizontal
    {eCLI_MAIN_DZV,  "VDZM"},    //Digital Zoom Vertical
    {eCLI_MAIN_DSH,  "HDSH"},    //Digital Shift Horizontal
    {eCLI_MAIN_DSV,  "VDSH"},    //Digital Shift Vertical
    {eCLI_MAIN_PCM,  "PCMO"},//"PCMG"},
    {eCLI_MAIN_WRP,  "WRPG"},
    {eCLI_MAIN_AWF,  "AWFG"},
    {eCLI_MAIN_MWF,  "MWFG"},
    {eCLI_MAIN_HPC,  "HPIC"},    //Pincushion Horizontal
    {eCLI_MAIN_VPC,  "VPIC"},    //Pincushion Vertical
    {eCLI_MAIN_TLCX, "TLCX"},    //4-Corner Top Left Horz        //G100_Wilsonj_0032
    {eCLI_MAIN_TLCY, "TLCY"},    //4-Corner Top Left Vert        //G100_Wilsonj_0032
    {eCLI_MAIN_TRCX, "TRCX"},    //4-Corner Top Right Horz       //G100_Wilsonj_0032
    {eCLI_MAIN_TRCY, "TRCY"},    //4-Corner Top Right Vert       //G100_Wilsonj_0032
    {eCLI_MAIN_BLCX, "BLCX"},    //4-Corner Bottom Left Horz     //G100_Wilsonj_0032
    {eCLI_MAIN_BLCY, "BLCY"},    //4-Corner Bottom Left Vert     //G100_Wilsonj_0032
    {eCLI_MAIN_BRCX, "BRCX"},    //4-Corner Bottom Right Horz    //G100_Wilsonj_0032
    {eCLI_MAIN_BRCY, "BRCY"},    //4-Corner Bottom Right Vert    //G100_Wilsonj_0032
    {eCLI_MAIN_CNR,  "CNRG"},
    {eCLI_MAIN_WRE,  "WREG"},
    {eCLI_MAIN_AIM,  "AIMG"},
    {eCLI_MAIN_BRT,  "BRIG"},    //Brightness
    {eCLI_MAIN_CON,  "CONT"},    //Contrast
    {eCLI_MAIN_CSP,  "CSPA"},    //Color Space
    {eCLI_MAIN_DTL,  "SHRP"},    //Sharpness
	{eCLI_MAIN_TIN,  "TINT"},    //Tint
    {eCLI_MAIN_TDN,  "TDNG"},    //3D Mode
    {eCLI_MAIN_TDE,  "TDEN"},    //3D Format
    {eCLI_MAIN_TDD,  "TDLR"},    //3D-2D
    {eCLI_MAIN_TDI,  "TDIV"},    //3D Invert
    {eCLI_MAIN_SIS,  "SISG"},    //3D Sync In Select //A35G2_BRC_Casper_0016
    {eCLI_MAIN_SOS,  "SOSG"},    //3D Sync Out
    {eCLI_MAIN_FDY,  "FDYG"},    //Frame Delay
    {eCLI_MAIN_LRR,  "LRRG"},    //L/R Reference
    {eCLI_MAIN_TDDL, "TDDL"},    //3D Tech (DLP-link)
    //{eCLI_MAIN_TDR,  "TDRT"},                             //G100_Simon_0011 remove
    //{eCLI_MAIN_CLR,  "CLRG"},
    //{eCLI_MAIN_TNT,  "TNTG"},
    //{eCLI_MAIN_NDR,  "NDRG"},
    //{eCLI_MAIN_FTC,  "FTCG"},
    //{eCLI_MAIN_VBL,  "VBLG"},
    //{eCLI_MAIN_FMD,  "FMDG"},
    //{eCLI_MAIN_CLC,  "CLCG"},
    {eCLI_MAIN_ROG,  "RGAN"},    //Red Gain
    {eCLI_MAIN_GOG,  "GGAN"},    //Green Gain
    {eCLI_MAIN_BOG,  "BGAN"},    //Blue Gain
    {eCLI_MAIN_ROO,  "ROFS"},    //Red Offset
    {eCLI_MAIN_GOO,  "GOFS"},    //Green Offset
    {eCLI_MAIN_BOO,  "BOFS"},    //Blue Offset
    //{eCLI_MAIN_SYT,  "SYTG"},
    {eCLI_MAIN_GOR,  "RSGO"}, //"GORG"},
    {eCLI_MAIN_DPMO, "DPMO"},    //Color Mode (Picture Settings)
    {eCLI_MAIN_DPSU, "DPSU"},    //Save to User
    {eCLI_MAIN_FPS,  "FPSG"},
    {eCLI_MAIN_DIM,  "DIMG"},
    {eCLI_MAIN_FRZ,  "FRZE"},    //Freeze Screen
    {eCLI_MAIN_BGC,  "GAMM"},    //Gamma
    {eCLI_MAIN_WHPK, "WHPK"},    //White peaking
    {eCLI_MAIN_CCI,  "CTMP"},    //Color Temperature
    {eCLI_MAIN_LOT,  "LOTG"}, //T100_Casper_0062
    {eCLI_MAIN_LOL,  "LOLG"}, //T100_Casper_0062
    //{eCLI_MAIN_EDG,  "EDGG"},
    //{eCLI_MAIN_CWS,  "CWSG"},
    {eCLI_MAIN_HSG,  "HSGG"},
    //{eCLI_MAIN_VBD,  "VBDG"},
    {eCLI_MAIN_LOC,  "LANG"}, //"LOCG"},
    {eCLI_MAIN_LET,  "LETG"}, //T100_Casper_0063
    //{eCLI_MAIN_FCS,  "FCSO"}, //"FCSG"},
    {eCLI_MAIN_FCSI, "FCSI"},    //Focus In
    {eCLI_MAIN_FCSO, "FCSO"},    //Focus Out
    //{eCLI_MAIN_ZOM,  "ZOMO"}, //"ZOMG"},
    {eCLI_MAIN_ZOMI, "ZOMI"},    //Zoom In
    {eCLI_MAIN_ZOMO, "ZOMO"},    //Zoom Out
    //{eCLI_MAIN_LVO,  "LVOG"},
    //{eCLI_MAIN_LHO,  "LHOG"},
    {eCLI_MAIN_LVU,  "LSVU"},    //Lens Shift Up     //T100_Casper_0099
    {eCLI_MAIN_LVD,  "LSVD"},    //Lens Shift Down   //T100_Casper_0099
    {eCLI_MAIN_LHL,  "LSHL"},    //Lens Shift Left   //T100_Casper_0099
    {eCLI_MAIN_LHR,  "LSHR"},    //Lens Shift Right  //T100_Casper_0099
    //{eCLI_MAIN_LMR,  "LMRG"},
    {eCLI_MAIN_LMA,  "LMAP"},    //Lens Memory Apply
    {eCLI_MAIN_LMS,  "LMSP"},    //Lens Memory Save
    {eCLI_MAIN_LCB,  "LECA"},    //Lens Calibration
    {eCLI_MAIN_LLK,  "LELO"},    //Lens Lock    //G100_Wilsonj_0032
    {eCLI_MAIN_CEL,  "CEMO"},    //Ceiling Mount
    {eCLI_MAIN_SOR,  "REPJ"},    //Direction (Rear Projection)
    //{eCLI_MAIN_LED,  "LEDG"},
    {eCLI_MAIN_MSH,  "MSHG"},
    {eCLI_MAIN_MSV,  "MSVG"},
    {eCLI_MAIN_MBE,  "SMSG"}, //"MBEG"},
    {eCLI_MAIN_OST,  "METR"}, //"OSTG"},
    {eCLI_MAIN_SPSG, "SPSG"},
    {eCLI_MAIN_BGCL, "BGCL"},
    {eCLI_MAIN_BGRT, "BGRT"},
    {eCLI_MAIN_PWR,  "POWR"},       //G100_Owen_0039
    {eCLI_MAIN_SBPM, "SBPM"},    //Standby Mode     //G100_Wilsonj_0041
    {eCLI_MAIN_FPON,  "FPON"},    //Fast Power On    //G100_Clare_0074//G100_Wilsonj_0041
    {eCLI_MAIN_ASH,  "APOF"},    //Auto Power Off   //G100_Wilsonj_0041
    {eCLI_MAIN_SLP,  "SLTM"},    //Sleep Timer      //G100_Wilsonj_0041
    {eCLI_MAIN_HIAL, "HIAL"}, //"HATG"},
    //{eCLI_MAIN_IRC,  "IRCG"},
    {eCLI_MAIN_FRNT, "FRNT"},    //Front IR    //G100_Wilsonj_0035
    {eCLI_MAIN_TOPP, "TOPP"},    //Top IR      //G100_Wilsonj_0035
    {eCLI_MAIN_HDBT, "HDBT"},    //HDBaseT IR  //G100_Wilsonj_0035
    {eCLI_MAIN_VTT,  "TRIG"},    //12V Trigger //G100_Wilsonj_0041
    {eCLI_MAIN_NET,  "NETG"},
    {eCLI_MAIN_LMAC, "LMAC"},    //LAN MAC Address  //G100_Wilsonj_0035
    {eCLI_MAIN_LDHC, "LDHC"},    //LAN DHCP         //G100_Wilsonj_0035
    {eCLI_MAIN_LIPA, "LIPA"},    //LAN IP Address   //G100_Wilsonj_0035
    {eCLI_MAIN_LSUB, "LSUB"},    //LAN Subnet Mask  //G100_Wilsonj_0035
    {eCLI_MAIN_LGAT, "LGAT"},    //LAN Gateway      //G100_Wilsonj_0035
    {eCLI_MAIN_DNSG, "DNSG"},    //LAN DNS          //G100_Wilsonj_0035
    {eCLI_MAIN_LAPY, "LAPY"},    //LAN Apply        //G100_Wilsonj_0035
    //{eCLI_MAIN_NTW,  "NTWG"},
    {eCLI_MAIN_BDR,  "SPBI"},     //Baud Rate Serial Port In
    {eCLI_MAIN_SEC,  "SECG"},
    {eCLI_MAIN_SPP,  "SPPG"},
    {eCLI_MAIN_SLA,  "SLAG"},
    {eCLI_MAIN_PJP,  "PJPD"},
    {eCLI_MAIN_ADR,  "PJAD"},    //Remote Code    //G100_Wilsonj_0036
    {eCLI_MAIN_KBL,  "BALI"},    //Backlight - Keypad    //G100_Wilsonj_0041
    {eCLI_MAIN_SBL,  "SBLG"},
    {eCLI_MAIN_HGWP, "HGWP"},
    {eCLI_MAIN_HGRH, "HGRH"},    //HSG Red Hue               //G100_Wilsonj_0032
    {eCLI_MAIN_HGRS, "HGRS"},    //HSG Red Saturation        //G100_Wilsonj_0032
    {eCLI_MAIN_HGRG, "HGRG"},    //HSG Red Gain              //G100_Wilsonj_0032
    {eCLI_MAIN_HGGH, "HGGH"},    //HSG Green Hue             //G100_Wilsonj_0032
    {eCLI_MAIN_HGGS, "HGGS"},    //HSG Green Saturation      //G100_Wilsonj_0032
    {eCLI_MAIN_HGGG, "HGGG"},    //HSG Green Gain            //G100_Wilsonj_0032
    {eCLI_MAIN_HGBH, "HGBH"},    //HSG Blue Hue              //G100_Wilsonj_0032
    {eCLI_MAIN_HGBS, "HGBS"},    //HSG Blue Saturation       //G100_Wilsonj_0032
    {eCLI_MAIN_HGBG, "HGBG"},    //HSG Blue Gain             //G100_Wilsonj_0032
    {eCLI_MAIN_HGCH, "HGCH"},    //HSG Cyan Hue              //G100_Wilsonj_0032
    {eCLI_MAIN_HGCS, "HGCS"},    //HSG Cyan Saturation       //G100_Wilsonj_0032
    {eCLI_MAIN_HGCG, "HGCG"},    //HSG Cyan Gain             //G100_Wilsonj_0032
    {eCLI_MAIN_HGYH, "HGYH"},    //HSG Yellow Hue            //G100_Wilsonj_0032
    {eCLI_MAIN_HGYS, "HGYS"},    //HSG Yellow Saturation     //G100_Wilsonj_0032
    {eCLI_MAIN_HGYG, "HGYG"},    //HSG Yellow Gain           //G100_Wilsonj_0032
    {eCLI_MAIN_HGMH, "HGMH"},    //HSG Magenta Hue           //G100_Wilsonj_0032
    {eCLI_MAIN_HGMS, "HGMS"},    //HSG Magenta Saturation    //G100_Wilsonj_0032
    {eCLI_MAIN_HGMG, "HGMG"},    //HSG Magenta Gain          //G100_Wilsonj_0032
    {eCLI_MAIN_HGWR, "HGWR"},    //HSG White Red             //G100_Wilsonj_0032
    {eCLI_MAIN_HGWG, "HGWG"},    //HSG White Green           //G100_Wilsonj_0032
    {eCLI_MAIN_HGWB, "HGWB"},    //HSG White Blue            //G100_Wilsonj_0032
    {eCLI_MAIN_CCAG, "CCAG"},
    //{eCLI_MAIN_MAR,  "MARG"},
    {eCLI_MAIN_HAR,  "HGRE"},    //HSG Reset tp Default
    {eCLI_MAIN_HSR,  "HSRT"}, //A35G2_BRC_Casper_0114
    {eCLI_MAIN_HKS,  "HKSG"},
    {eCLI_MAIN_LGC,  "LGCG"}, //T100IR_Casper_0032
    {eCLI_MAIN_PIF,  "PIFG"},
    {eCLI_MAIN_MDNA, "MDNA"},    //Model Name                      //G100_Wilsonj_0042
    {eCLI_MAIN_SERI, "SERI"},    //Serial Number                   //G100_Wilsonj_0042
    {eCLI_MAIN_FWIS, "FWIS"},    //Firmware Version - I-SCALER     //G100_Wilsonj_0042
    {eCLI_MAIN_FWMF, "FWMF"},    //Firmware Version - F-MCU        //G100_Wilsonj_0042
    //{eCLI_MAIN_FWMM, "FWMM"},    //Firmware Version - M-MCU        //G100_Wilsonj_0042
    //{eCLI_MAIN_FWML, "FWML"},    //Firmware Version - L-MCU        //G100_Wilsonj_0042
    {eCLI_MAIN_FWMA, "FWMA"},    //Firmware Version - A-MCU        //G100_Wilsonj_0042
    {eCLI_MAIN_FWMK, "FWMK"},    //Firmware Version - K-MCU        //G100_Wilsonj_0042
    {eCLI_MAIN_FWLA, "FWLA"},    //Firmware Version - LAN          //G100_Wilsonj_0042
    {eCLI_MAIN_FWFM, "FWFM"},    //Firmware Version - Formatter    //G100_Wilsonj_0042
    {eCLI_MAIN_FWFA, "FWFA"},    //Firmware Version - FPGA0        //G100_Wilsonj_0042
    {eCLI_MAIN_FWFB, "FWFB"},    //Firmware Version - FPGA1        //G100_Wilsonj_0042
    //{eCLI_MAIN_FWFC, "FWFC"},    //Firmware Version - FPGA2        //G100_Wilsonj_0042
    {eCLI_MAIN_FWFX, "FWFX"},    //Firmware Version - XFPGA        //G100_Wilsonj_0042
    {eCLI_MAIN_FWHD, "FWHD"},    //Firmware Version - HDBaseT      //G100_Wilsonj_0042
    {eCLI_MAIN_DEF,  "DEFG"},
    {eCLI_MAIN_ITP,  "TPRN"},    //Test Pattern
    {eCLI_MAIN_PSID, "PSID"},
    {eCLI_MAIN_FTID, "FTID"},
    {eCLI_MAIN_ERR,  "ERRG"},
    //{eCLI_MAIN_MDT,  "MDTG"},
    {eCLI_MAIN_LDI,  "LDIG"},
    {eCLI_MAIN_FAN,  "FANG"},
    {eCLI_MAIN_UST,  "USTG"}, //A35G2_BRC_Casper_0088
    {eCLI_MAIN_LPM,  "LPMO"},    //Light Source Mode    //G100_Wilsonj_0042
    {eCLI_MAIN_LPP,  "LSBL"}, //"LPPG"},
    {eCLI_MAIN_LPI,  "LPIG"},
    {eCLI_MAIN_LLP,  "LLPG"}, //T100IR_Casper_0029
    {eCLI_MAIN_LLI,  "LLIG"}, //T100IR_Casper_0029
    {eCLI_MAIN_BLE,  "BLEG"},  //T100IR_Casper_0032
    {eCLI_MAIN_IRE,  "IREG"},  //T100IR_Casper_0032
    {eCLI_MAIN_LIF,  "LIFG"},
    {eCLI_MAIN_RLC,  "RLCG"},
    //{eCLI_MAIN_SST,  "SSTG"}, //G100_Larry_0029
    //{eCLI_MAIN_SIN,  "SRCS"},    //Input Signal (Main Input)
    {eCLI_MAIN_PIP,  "PIBP"},//"PIPG"},
    {eCLI_MAIN_PPS,  "PISW"},//"PPSG"},
    {eCLI_MAIN_PHS,  "PHSG"},
    {eCLI_MAIN_PPP,  "PILO"}, //"PPPG"},
    //{eCLI_MAIN_TMG,  "TMGG"},
    //{eCLI_MAIN_MIF,  "MIFG"},
    {eCLI_MAIN_MSRS, "MSRS"},    //Main Source - Resolution       //G100_Wilsonj_0042
    {eCLI_MAIN_MSSF, "MSSF"},    //Main Source - Signal Format    //G100_Wilsonj_0042
    {eCLI_MAIN_MSPC, "MSPC"},    //Main Source - Pixel Clock      //G100_Wilsonj_0042
    {eCLI_MAIN_MSHR, "MSHR"},    //Main Source - Horz Refresh     //G100_Wilsonj_0042
    {eCLI_MAIN_MSVR, "MSVR"},    //Main Source - Vert Refresh     //G100_Wilsonj_0042
    {eCLI_MAIN_MSCS, "MSCS"},    //Main Source - Color Space      //G100_Wilsonj_0042
    //{eCLI_MAIN_SIF,  "SIFG"},
    {eCLI_MAIN_SSRS, "SSRS"},    //Sub Source - Resolution       //G100_Wilsonj_0042
    {eCLI_MAIN_SSSF, "SSSF"},    //Sub Source - Signal Format    //G100_Wilsonj_0042
    {eCLI_MAIN_SSPC, "SSPC"},    //Sub Source - Pixel Clock      //G100_Wilsonj_0042
    {eCLI_MAIN_SSHR, "SSHR"},    //Sub Source - Horz Refresh     //G100_Wilsonj_0042
    {eCLI_MAIN_SSVR, "SSVR"},    //Sub Source - Vert Refresh     //G100_Wilsonj_0042
    {eCLI_MAIN_SSCS, "SSCS"},    //Sub Source - Color Space      //G100_Wilsonj_0042
    {eCLI_MAIN_SKS,  "ASRC"},    //Auto Signal (Auto Source)
    //{eCLI_MAIN_BSS,  "BSSG"},
    {eCLI_MAIN_UID,  "UIDG"},
    {eCLI_MAIN_SIV,  "SIVG"},
    {eCLI_MAIN_LCE,  "LCEG"},
    {eCLI_MAIN_LSE,  "LSEG"},
    {eCLI_MAIN_SNS,  "SNSG"},
    {eCLI_MAIN_KEY,  "KEYG"},
    {eCLI_MAIN_SHU,  "SHUG"},
    {eCLI_MAIN_FCT,  "FCTG"},
    {eCLI_MAIN_OSD,  "OSDG"},
    //{eCLI_MAIN_NRD,  "NRDG"},
    {eCLI_MAIN_TTT,  "TTTG"},
    //{eCLI_MAIN_CSS,  "CSSG"}, //chip switch set
    {eCLI_MAIN_CFU,  "CFUG"}, //chip FW upgrade
    {eCLI_MAIN_EXF,  "EXFG"},
    {eCLI_MAIN_BUR,  "BURG"},
    //{eCLI_MAIN_GUI,  "GUIG"},
    //{eCLI_MAIN_BLD,  "BLDG"},
    //{eCLI_MAIN_BLS,  "BLSG"},
    //{eCLI_MAIN_MMC,  "MMCG"},
    //{eCLI_MAIN_HDR,  "HDRG"},
    {eCLI_MAIN_MTO,  "METI"},    //Menu Timer
    {eCLI_MAIN_ILI,  "ILIG"},
    {eCLI_MAIN_ICI,  "ICIG"},        //T100_Doulas_0035
	{eCLI_MAIN_DRD,  "DRDG"},		//T100_Casper_0008
	{eCLI_MAIN_DBG,  "DBGG"},		//T100_Simon_0024
	{eCLI_MAIN_WAP,  "GCAM"},//"WAPG"},		//T100_Simon_0031
	{eCLI_MAIN_WAS,  "GCSM"}, //"WASG"},		//T100_Simon_0031
	{eCLI_MAIN_KSH,  "HKES"},    //Keystone Horizontal    //G100_Wilsonj_0032
	{eCLI_MAIN_KSV,  "VKES"},    //Keystone Vertical      //G100_Wilsonj_0032
	//{eCLI_MAIN_CAM,  "CAMG"},		//T100_Simon_0063
    //{eCLI_MAIN_ICP,  "ICPG"},
    //{eCLI_MAIN_ICD,  "ICDG"},
	{eCLI_MAIN_LLM,  "LLMG"},
    {eCLI_MAIN_EBL,  "EBLG"},    //Edge Blend(Blend Area)
	{eCLI_MAIN_FKC,  "FKCG"}, 	//T100_Coda
	{eCLI_MAIN_FVI,  "FVIG"},
	{eCLI_MAIN_CCT,  "CCTG"},
	{eCLI_MAIN_COL,  "SATU"},    //Saturation
	{eCLI_MAIN_MLT,  "MELG"},    //Menu Location  //T100IR_Casper_0032
	{eCLI_MAIN_HOE,  "HOEG"}, //T100IR_Casper_0032
	{eCLI_MAIN_HSE,  "HSEG"}, //T100IR_Casper_0032
	//{eCLI_MAIN_HBE,  "HBEG"}, //T100IR_Casper_0032
    {eCLI_MAIN_DYBK, "DYBK"},    //Dynamic Black    //G100_Wilsonj_0032
    {eCLI_MAIN_EXBK, "EXBK"},    //Extreme Black    //G100_Wilsonj_0032
    {eCLI_MAIN_DCSP, "DCSP"},    //Dynamic Black Speed       //G100_Wilsonj_0032
    {eCLI_MAIN_DCST, "DCST"},    //Dynamic Black Strength    //G100_Wilsonj_0032
    {eCLI_MAIN_DCLE, "DCLE"},    //Dynamic Black Level       //G100_Wilsonj_0032
    {eCLI_MAIN_DCLT, "DCLT"},    //Extreme Black Light Out Timer           //G100_Wilsonj_0032
    {eCLI_MAIN_DCLS, "DCLS"},    //Extreme Black Light Out Signal Level    //G100_Wilsonj_0032
    {eCLI_MAIN_MSRC, "MSRC"},    //Main Source     //G100_Wilsonj_0032
    {eCLI_MAIN_SSRC, "SSRC"},    //Sub Source      //G100_Wilsonj_0032
    {eCLI_MAIN_EDMA, "EDMA"},    //Edge Mask       //G100_Wilsonj_0033
	{eCLI_MAIN_IST,  "ISTG"},    //G100_Julie_0001
	{eCLI_MAIN_SPON, "SPON"},    //Signal Power On  //G100_Wilsonj_0040
	{eCLI_MAIN_PINM, "PINM"},    //Security Timer Month  //G100_Wilsonj_0041
	{eCLI_MAIN_PIND, "PIND"},    //Security Timer Day    //G100_Wilsonj_0041
	{eCLI_MAIN_PINH, "PINH"},    //Security Timer Hour   //G100_Wilsonj_0041
    {eCLI_MAIN_LPTH, "LPTH"},    //Total Projector Hour  //G100_Wilsonj_0042
    {eCLI_MAIN_LEPT, "LEPT"},    //Lens Type             //G100_Wilsonj_0042 //A35G2_BRC_Casper_0147
    {eCLI_MAIN_SSTP, "SSTP"},    //Temperature           //G100_Wilsonj_0042
    //{eCLI_MAIN_SSAC, "SSAC"},    //AC Voltage            //G100_Wilsonj_0046
    {eCLI_MAIN_LSNT, "LSNT"},
    {eCLI_MAIN_LSET, "LSET"},
    {eCLI_MAIN_LSQT, "LSQT"},
    {eCLI_MAIN_LSCT, "LSCT"},
    {eCLI_MAIN_PPZM, "PPZM"},    //Proportional   //G100_Wilsonj_0060
    {eCLI_MAIN_HKST, "HKST"},    //User Button 1  //G100_Owen_0074
    {eCLI_MAIN_HKSB, "HKSB"},    //User Button 2  //G100_Wilsonj_0060
	{eCLI_MAIN_OPD,  "OPDG"},
    {eCLI_MAIN_CREN, "CREN"},    //Crestron              //G100_Wilsonj_0061
	{eCLI_MAIN_CRIP, "CRIP"},    //IP Address            //G100_Wilsonj_0061
	{eCLI_MAIN_CRID, "CRID"},    //IPID                  //G100_Wilsonj_0061
	{eCLI_MAIN_CRPO, "CRPO"},    //Port                  //G100_Wilsonj_0061
	{eCLI_MAIN_CRAP, "CRAP"},    //Crestron Setup Apply  //G100_Wilsonj_0061
	{eCLI_MAIN_EXEN, "EXEN"},    //Extron                //G100_Wilsonj_0061
	{eCLI_MAIN_PJEN, "PJEN"},    //PJ Link               //G100_Wilsonj_0061
	{eCLI_MAIN_PJIP, "PJIP"},    //Service               //G100_Wilsonj_0061
	{eCLI_MAIN_PJAP, "PJAP"},    //PJ Link Setup Apply   //G100_Wilsonj_0061
	{eCLI_MAIN_AXEN, "AXEN"},    //AMX                   //G100_Wilsonj_0061
	{eCLI_MAIN_TEEN, "TEEN"},    //Telnet                //G100_Wilsonj_0061
	{eCLI_MAIN_HTEN, "HTEN"},    //HTTP                  //G100_Wilsonj_0061
	{eCLI_MAIN_CNRT, "CNRT"},    //Reset                 //G100_Wilsonj_0061
	{eCLI_MAIN_LSHS, "LSHS"},  //G100_Larry_0007
	{eCLI_MAIN_PMUT, "PMUT"},  //G100_Larry_0007
	{eCLI_MAIN_SHFI, "SHFI"},  //G100_Larry_0007
	{eCLI_MAIN_SHFO, "SHFO"},  //G100_Larry_0007
    {eCLI_MAIN_SCRT, "SCRT"},
    {eCLI_MAIN_GCRT, "GCRT"},
    {eCLI_MAIN_LMNS, "LMNS"},
//    {eCLI_MAIN_LCCR, "LCCR"},
//    {eCLI_MAIN_LCEX, "LCEX"},
//    {eCLI_MAIN_LCPJ, "LCPJ"},
//    {eCLI_MAIN_LCAX, "LCAX"},
//    {eCLI_MAIN_LCTE, "LCTE"},
//    {eCLI_MAIN_LCHT, "LCHT"},
    {eCLI_MAIN_LANI, "LANI"},
    {eCLI_MAIN_SYRT, "SYRT"},
    {eCLI_MAIN_FRST, "FRST"},
    {eCLI_MAIN_DPRT, "DPRT"},
    {eCLI_MAIN_IMRT, "IMRT"},
    {eCLI_MAIN_CMRT, "CMRT"},
    {eCLI_MAIN_PSRT, "PSRT"},
    {eCLI_MAIN_BKLV, "BKLV"},
    {eCLI_MAIN_LMRT, "LMRT"},//G100_Coda_0005
    {eCLI_MAIN_LERT, "LERT"},
    {eCLI_MAIN_BKSW, "BKSW"},
    {eCLI_MAIN_BKCS, "BKCS"},
    {eCLI_MAIN_BKFI, "BKFI"},
    {eCLI_MAIN_BKFR, "BKFR"},
    {eCLI_MAIN_BKFH, "BKFH"},
    {eCLI_MAIN_BKFC, "BKFC"},
    {eCLI_MAIN_BKFD, "BKFD"},  //A35G2_BRC_Simon_0004
    {eCLI_MAIN_BKSI, "BKSI"},
    {eCLI_MAIN_BKSR, "BKSR"},
    {eCLI_MAIN_BKSH, "BKSH"},
    {eCLI_MAIN_BKSC, "BKSC"},
    {eCLI_MAIN_BKSD, "BKSD"},  //A35G2_BRC_Simon_0004
    {eCLI_MAIN_BKIS, "BKIS"},
    {eCLI_MAIN_BKIC, "BKIC"},  //A35G2_BRC_Simon_0004
    //{eCLI_MAIN_HSWP, "HSWP"},
    {eCLI_MAIN_DCRT, "DCRT"},
    {eCLI_MAIN_TDRT, "TDRT"},
    {eCLI_MAIN_NFRS, "NFRS"},
    {eCLI_MAIN_BALP, "BALP"},
    {eCLI_MAIN_PINP, "PINP"},   //G100_Owen_0074
    {eCLI_MAIN_PINC, "PINC"},
    {eCLI_MAIN_PHAS, "PHAS"},	//G100_Clare_0064
    {eCLI_MAIN_HPOS, "HPOS"},
    {eCLI_MAIN_VPOS, "VPOS"},
    {eCLI_MAIN_DVRS, "DVRS"},
    {eCLI_MAIN_DHOP, "DHOP"},
    {eCLI_MAIN_DHEA, "DHEA"},
    {eCLI_MAIN_DHEB, "DHEB"},
    {eCLI_MAIN_DHSL, "DHSL"},
    //{eCLI_MAIN_DHBT, "DHBT"},
    {eCLI_MAIN_DHBL, "DHBL"},
    {eCLI_MAIN_ASRS, "ASRS"},
    {eCLI_MAIN_HDRG, "HDRG"},
    {eCLI_MAIN_HDRP, "HDRP"},
    {eCLI_MAIN_LLTC, "LLTC"},
    {eCLI_MAIN_MNTP, "MNTP"},
    {eCLI_MAIN_WWCG, "WWCG"},		//G100_Doulas_0027
    {eCLI_MAIN_WGPG, "WGPG"},
    {eCLI_MAIN_WWIG, "WWIG"},
    {eCLI_MAIN_WWSG, "WWSG"},
    {eCLI_MAIN_WGCG, "WGCG"},
	{eCLI_MAIN_WBCG, "WBCG"},
	{eCLI_MAIN_GMSG, "GMSG"},
	{eCLI_MAIN_GMAG, "GMAG"},
	{eCLI_MAIN_GMCG, "GMCG"},
	{eCLI_MAIN_ICPG, "ICPG"},
	{eCLI_MAIN_TARG, "TARG"},
	{eCLI_MAIN_DTCM, "DTCM"},
	{eCLI_MAIN_DTSY, "DTSY"},
	{eCLI_MAIN_DTSM, "DTSM"},
	{eCLI_MAIN_DTSD, "DTSD"},
	{eCLI_MAIN_DTTH, "DTTH"},
	{eCLI_MAIN_DTTM, "DTTM"},
	{eCLI_MAIN_DTDS, "DTDS"},
	{eCLI_MAIN_DTNS, "DTNS"},
	{eCLI_MAIN_DTTZ, "DTTZ"},
	{eCLI_MAIN_DTUI, "DTUI"},
	{eCLI_MAIN_DTAP, "DTAP"},

	{eCLI_MAIN_DTIF, "DTIF"},

	{eCLI_MAIN_SCW1, "SCW1"},
	{eCLI_MAIN_SCW2, "SCW2"},
	{eCLI_MAIN_SCW3, "SCW3"},
	{eCLI_MAIN_SCW4, "SCW4"},
	{eCLI_MAIN_SCW5, "SCW5"},
	{eCLI_MAIN_SCW6, "SCW6"},
	{eCLI_MAIN_SCW0, "SCW0"},
    {eCLI_MAIN_SCHE, "SCHE"},
    {eCLI_MAIN_ATFC, "ATFC"},	//G100_Clare_0028
    {eCLI_MAIN_ATWC, "ATWC"},	//G100_Clare_0028
    {eCLI_MAIN_SHSP, "SHSP"},
    {eCLI_MAIN_LOGC, "LOGC"},
    {eCLI_MAIN_LOGS, "LOGS"},
    {eCLI_MAIN_LOGD, "LOGD"},
#if defined (CUSTOM_OPTOMA) || defined(CUSTOM_BARCO) //A35G2_BRC_Casper_0039 //A35G2_BRC_Casper_0152
    {eCLI_MAIN_CAST, "CAST"},    //G100_Clare_0036
    {eCLI_MAIN_AFST, "AFST"},    //G100_Clare_0036
    {eCLI_MAIN_WCST, "WCST"},    //G100_Clare_0036
#endif

	{eCLI_MAIN_DPAU, "DPAU"},    //apply user mode //G100_Steven_0030
    {eCLI_MAIN_FWVR, "FWVR"},    //G100_Clare_0039
    //{eCLI_MAIN_CAVR, "CAVR"},    //G100_Clare_0039

	{eCLI_MAIN_UDSA, "UDSA"}, 	 //G100_Doulas_0062
	{eCLI_MAIN_UDLD, "UDLD"}, 	 //G100_Doulas_0062

	{eCLI_MAIN_WCOL, "WCOL"}, 	 //G100_Doulas_0067
	{eCLI_MAIN_WROW, "WROW"}, 	 //G100_Doulas_0067

	{eCLI_MAIN_SEQM, "SEQM"},    //G100_Steven_0048 start
	{eCLI_MAIN_BRPM, "BRPM"},
	{eCLI_MAIN_BGPM, "BGPM"},
	{eCLI_MAIN_BBPM, "BBPM"},
	{eCLI_MAIN_BYPM, "BYPM"},
	{eCLI_MAIN_RRPM, "RRPM"},
	{eCLI_MAIN_RYPM, "RYPM"},
	{eCLI_MAIN_LEDT, "LEDT"},
	{eCLI_MAIN_SSPA, "SSPA"},
	{eCLI_MAIN_SSRH, "SSRH"},
	{eCLI_MAIN_DDTP, "DDTP"},
	//{eCLI_MAIN_FANF, "FANF"},    //G100_Steven_0048 end
	{eCLI_MAIN_PBRI, "PBRI"}, //G100_Steven_0079
	{eCLI_MAIN_PCON, "PCON"}, //G100_Steven_0079
	{eCLI_MAIN_PCSP, "PCSP"}, //G100_Steven_0079

    {eCLI_MAIN_WPTP, "WPTP"},   //G100_Owen_0089 PToolset start //A35G2_BRC_Casper_0046
    {eCLI_MAIN_WPPU, "WPPU"},
    {eCLI_MAIN_WPPD, "WPPD"},
    {eCLI_MAIN_WPPL, "WPPL"},
    {eCLI_MAIN_WPPR, "WPPR"},
    {eCLI_MAIN_WPXY, "WPXY"},
    {eCLI_MAIN_WRST, "WRST"},

	{eCLI_MAIN_LMCP, "LMCP"}, //G100_Steven_0098 //A35G2_BRC_Casper_0047
	{eCLI_MAIN_GMCC, "GMCC"}, //G100_Steven_0098
	{eCLI_MAIN_UDCC, "UDCC"}, //G100_Steven_0098

	{eCLI_MAIN_HUTP, "HUTP"}, //G100_Steven_0118 //A35G2_BRC_Casper_0051
	{eCLI_MAIN_DDTC, "DDTC"}, //G100_Steven_0118 //A35G2_BRC_Casper_0051

    {eCLI_MAIN_AHSW, "AHSW"}, //G100_Tim_0057, add //A35G2_BRC_Casper_0060
    {eCLI_MAIN_PJNT, "PJNT"}, //G100_Tim_0058, add //A35G2_BRC_Casper_0060

    {eCLI_MAIN_BODG, "BODG"},
    {eCLI_MAIN_USTS, "USTS"}, //A35G2_BRC_Casper_0088
    {eCLI_MAIN_OPRA, "OPRA"}, //A65_OPTOMA_Julie_0022//A35G2_Coda_0110
    {eCLI_MAIN_CBMP, "CBMP"}, //A35G2_BRC_Casper_0145
	{eCLI_MAIN_TDAR, "TDAR"},  //G100_Steven_0163 //A35G2_BRC_Casper_0146

#if (ENABLE_COLOR_UNIFORMITY == TRUE) //G100_Tim_0012, add, start //A35G2_BRC_Casper_0152
    {eCLI_MAIN_ACUE, "ACUE"},
    {eCLI_MAIN_ACUS, "CUST"},
    {eCLI_MAIN_ACUT, "ACUT"},
    {eCLI_MAIN_ACUR, "ACUR"},
    //{eCLI_MAIN_ACUW, "ACUW"},
#endif //ENABLE_COLOR_UNIFORMITY      //G100_Tim_0012, add, end

    {eCLI_MAIN_END,  "FFFF"} //
};

#define CLI_MAINLUT_NUMBER sizeof(m_sMainLut)/sizeof(m_sMainLut[0])

static sCLI_DATA_INFO m_sCLI_General_Info[eccNumber];
static UINT32 m_ulAddressID = 0;  //G100_Wilsonj_0050

////////////////////////////////////////////////////////////////



//Special case function reply

// ==============================================================================
// FUNCTION NAME: utilGeneral_CLI_SYS
// DESCRIPTION:
//
//
// Params:
// eCLI_MODE eAccessMode:
// sCLI_GENERAL_FORMAT* sCmdFormat:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/11/08, Larry Create //T100_Larry_0067 fixed
// --------------------
// ==============================================================================
eCLI_ERROR_CODE utilGeneral_CLI_SYS(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
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
                if(palDataMgr_PIN_Protect_Checking() == ePASSWORD_PROTECT_LOCKED)
                {
                    sCmdFormat->lData = sSate_Pin;
                }
                else if(utilCommonCLI_DataControl(edcMAIN_SIGNAL_FORMAT) == eFUNC_CONTROL_ENABLE)
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

            default:
                sCmdFormat->lData = sSate_Startup;
                break;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

// ==============================================================================
// FUNCTION NAME: utilGeneral_CLI_TTT
// DESCRIPTION:
//
//
// Params:
// eCLI_MODE eAccessMode:
// sCLI_GENERAL_FORMAT* sCmdFormat:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/11/08, Larry Create
// --------------------
// ==============================================================================
eCLI_ERROR_CODE utilGeneral_CLI_TTT(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
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
// FUNCTION NAME: utilGeneral_CLI_KEY
// DESCRIPTION:
//
//
// Params:
// eCLI_MODE eAccessMode:
// sCLI_GENERAL_FORMAT* sCmdFormat:
//
// Returns:
//
//
// Modification History
// --------------------
// 2018/02/09, Simon Create
// --------------------
// ==============================================================================
eCLI_ERROR_CODE utilGeneral_CLI_KEY(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
{
    eCLI_ERROR_CODE eCLI_ErrorCode = eCLI_ERROR_CODE_SETFAIL ;

    if(palInputProc_InputKeycode(sCmdFormat->lData) == rcSUCCESS)
    {
        eCLI_ErrorCode = eCLI_ERROR_CODE_NO ;
    }

    return eCLI_ErrorCode;
}

// ==============================================================================
// FUNCTION NAME: utilGeneral_CLI_SYSELOG
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
// 2020/05/22, Larry Create
// --------------------
// ==============================================================================
eCLI_ERROR_CODE utilGeneral_CLI_SYSELOG(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
{
    eCLI_ERROR_CODE eErrorCode = eCLI_ERROR_CODE_NO;
    char cLogTemp[100][128] = {'\0'};
    char  ucOutputString[128] = {0};
    UINT16 uiNumber = 0;
    UINT16 uiReadCount = 0;
    UINT16 uiCount = 0;

    //uiReadCount = utilDataMgr_LoadLastLog(cLogTemp, cLogTemp);    //G100_Wilsonj_0018 mask
	uiReadCount = utilDataMgr_LoadLastLog_CM(100, cLogTemp);	//HICC2_Doulas_0003

    if(eAccessMode == ecmRead)
    {
        if(!uiReadCount)
        {
            sprintf(ucOutputString, "\r\nNo Error\r\n");
            __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        }
        else
        {
            for(uiCount = 0; (uiCount < 100) && (uiReadCount) ; uiCount++)
            {
                uiReadCount--;
                sprintf(ucOutputString, "%s\r\n", cLogTemp[uiReadCount]);
                __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
            }
        }
    }
    else if(eAccessMode == ecmWrite)
    {
        eErrorCode = eCLI_ERROR_CODE_COMMAND_READ_ONLY;
    }

    return eErrorCode;
}



// ==============================================================================
// FUNCTION NAME: utilGeneral_CLI_SYSDBMK
// DESCRIPTION:
//
//
// Params:
// eCLI_MODE eAccessMode:
// sCLI_GENERAL_FORMAT* sCmdFormat:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/11/08, Larry Create
// --------------------
// ==============================================================================
eCLI_ERROR_CODE utilGeneral_CLI_SYSDBMK(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
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

eCLI_ERROR_CODE utilGeneral_CLI_SYS_FPGA(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
{
    if(eAccessMode == ecmRead)
    {
        if(rcSUCCESS == palSystem_Suspend(__FUNCTION__))
        {
            //dvPro_RegDump();    //G100_Wilsonj_0018 mask

            palSystem_Resume(__FUNCTION__);
        }
    }
    else if(eAccessMode == ecmWrite)
    {
        return eCLI_ERROR_CODE_COMMAND_READ_ONLY;
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilGeneral_CLI_SYS_C821(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
{
    if(eAccessMode == ecmRead)
    {
        palImgMgr_Dump_Register(NULL);  //all bank
    }
    else if(eAccessMode == ecmWrite)
    {
        switch(sCmdFormat->lData)
        {
            case 0:
                {
                    UINT16 uiTotalCnt = 0, uiErrorCnt = 0;
                    palImgMgr_Test(&uiTotalCnt, &uiErrorCnt);
                }
                break;

            default:
                break;
        }
//        return eCLI_ERROR_CODE_COMMAND_READ_ONLY;
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilGeneral_CLI_SYS_VCXO949(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
{
    if(eAccessMode == ecmRead)
    {
        dvVCXO949_DumpRegTable(eVCXO_SEL_C821_WU);
    }
    else if(eAccessMode == ecmWrite)
    {
        dvVCXO949_DumpRegTable(eVCXO_SEL_LAST);
    }
    return eCLI_ERROR_CODE_NO;
}


#ifdef SCALER_C821_C789
#define ARGV_COUNT_MAX 10		//G100_Doulas_0027
eCLI_ERROR_CODE utilGeneral_CLI_SYS_C789(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)		//G100_Doulas_0027
{
    if(eAccessMode == ecmRead)
    {
    	LOG_MSG(db_ALWAYS, "C789 ecmRead\r\n");
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
		if(sCmdFormat->ucDataType==eCLI_DATA_TYPE_STRING)
		{
			UINT16 ipArgv[ARGV_COUNT_MAX] = {0};
			UINT16 iArgvCount = 0;
			char cpToken[] = " ";
			char *ptr = strtok(sCmdFormat->cTextString, cpToken);
			while (ptr != NULL && iArgvCount < ARGV_COUNT_MAX)
			{
				ipArgv[iArgvCount] = atoi(ptr);
				iArgvCount++;
				ptr = strtok(NULL, cpToken);
			}

			utilWarp_Test(ipArgv[0], ipArgv[1], ipArgv[2], ipArgv[3], ipArgv[4], ipArgv[5]);
		}
        else
        {
            switch(sCmdFormat->lData)
            {
                case 0:
                    {
                        UINT16 uiTotalCnt = 0, uiErrorCnt = 0;
                        halC789Ctrl_Cmd_Test(&uiTotalCnt, &uiErrorCnt);
                    }
                    break;

                case 1:
                    LOG_MSG(db_ALWAYS, "C789 V start test\r\n");
                    halC789Ctrl_V_Start_Test();
                    LOG_MSG(db_ALWAYS, "C789 V start test end!\r\n");
                    break;

                default:
                    break;
            }
        }
    }
    return eCLI_ERROR_CODE_NO;
}
#endif

eCLI_ERROR_CODE utilGeneral_CLI_SYS_SMDC(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
{
    if(eAccessMode == ecmRead)
    {
        utilDatabase_ItemPrint();
    }

    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilGeneral_CLI_SYS_SMIF(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
{
    if(eAccessMode == ecmRead)
    {
        utilDatabase_InformationPrint();
    }

    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilGeneral_CLI_SYS_EmergencyTest(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
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
                char String[256] = "ABCDEFGH 123456789 ABCDEFGH 123456789 ABCDEFGH 123456789 ABCDEFGH 123456789 ABCDEFGH 123456789 ABCDEFGH 123456789 ABCDEFGH 123456789 ABCDEFGH 123456789 ABCDEFGH 123456789 ABCDEFGH 123456789 ABCDEFGH 123456789 ABCDEFGH 123456789 KKKKK" ;
                palDataMgr_Data_Access(edcEMERGENCY_MESSAGE_SHOWS, edaWRITE_THROUGH_WITH_ACTION, String);
            }
            break;
        }

    }

    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilGeneral_CLI_SYS_DMDB(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
{
    if(eAccessMode == ecmRead)
    {
    }
    else if(eAccessMode == ecmWrite)
    {
        switch(sCmdFormat->lData)
        {
            case 0:
            case 1:
            case 2:
            case 3:
                DataMapping_DebugModeSet(sCmdFormat->lData);
                break;

            case 99:
                {
                    INT32 source_list[15];
                    INT32 source_count = 0 ;
                    source_count = DataMapping_GuiItemList_Get(edcMAIN_INPUT, source_list);
                    printf("\nsource_count = %d\n ", source_count);
                }
                break;
        }

    }

    return eCLI_ERROR_CODE_NO;
}


eCLI_ERROR_CODE utilGeneral_CLI_SYS_VersionCheck(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
{
    if(eAccessMode == ecmRead)
    {
    }
    else if(eAccessMode == ecmWrite)
    {
        switch(sCmdFormat->lData)
        {
            default:
                palSystem_VersionCheck(FALSE);
                break;
        }

    }

    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilGeneral_CLI_SYS_HDBT_Status(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
{
    if(eAccessMode == ecmRead)
    {
        UINT8 ucCur_HDBT_Status;
        uOPD_DATA uOPDData = {0};

        palDataMgr_Data_Access(edcHDBaseT_Status, edaREAD, (void*)&ucCur_HDBT_Status);

        LOG_MSG(db_ALWAYS, "HDBT Status = %d\r\n", ucCur_HDBT_Status);

        uOPDData.sSnapshotLog.ulDebug = ucCur_HDBT_Status;
        utilOPD_EventSet(eOPD_DEBUG_LOG, &uOPDData);
    }
    else if(eAccessMode == ecmWrite)
    {

    }

    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilGeneral_CLI_RXPR(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
{
    if(eAccessMode == ecmRead)
    {

    }
    else if(eAccessMode == ecmWrite)
    {
        LOG_MSG(db_ALWAYS, "CLI Reset XPR\r\n");
        halFormatter_XPR_ErrorStatus_Set();
    }

    return eCLI_ERROR_CODE_NO;
}

//A35G2_Simon_0116
eCLI_ERROR_CODE utilGeneral_CLI_CBMP(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
{
    eCLI_ERROR_CODE eErrorCode;
    sCLI_COMMON_FORMAT sCommonFormat;

    sCommonFormat.eAccessMode = eAccessMode;
    sCommonFormat.ucCmdFrom = sCmdFormat->ucCmdFrom;
    sCommonFormat.ucDataType = sCmdFormat->ucDataType;
    sCommonFormat.lData = sCmdFormat->lData;
    sCommonFormat.fData = sCmdFormat->fData;

    eErrorCode = utilCommonCLI_ShowOSDBitmapRawData(&sCommonFormat);

    return eErrorCode;
}



#if 0
extern void dvC734_RegDump(void);
extern void dvC790_RegDump(void);

eCLI_ERROR_CODE utilGeneral_CLI_SYS_C734(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)   //A70LV_Doulas_0165
{
    if(eAccessMode == ecmRead)
    {
        dvC734_RegDump();
    }
    else if(eAccessMode == ecmWrite)
    {
        return eCLI_ERROR_CODE_COMMAND_READ_ONLY;
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilGeneral_CLI_SYS_C790(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)      //A70LV_Doulas_0170 debug
{
    if(eAccessMode == ecmRead)
    {
        dvC790_RegDump();
    }
    else if(eAccessMode == ecmWrite)
    {
        return eCLI_ERROR_CODE_COMMAND_READ_ONLY;
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilGeneral_CLI_SYS_6805(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
{
    if(eAccessMode == ecmRead)
    {
        dvIT6805_debugMessage();
    }
    else if(eAccessMode == ecmWrite)
    {
        UINT8 ucTextStringSize = strlen(sCmdFormat->cTextString);
        printf("ucTextStringSize = %d" , ucTextStringSize);

        switch(ucTextStringSize)
        {
            case 5:
                if(sCmdFormat->cTextString[4] == '?')
                {
                    UINT8 cAddr = 0;

                    if( ((sCmdFormat->cTextString[0] == '0') || (sCmdFormat->cTextString[0] == '1'))
                        && (rcSUCCESS == AscIIToHex(sCmdFormat->cTextString[1], sCmdFormat->cTextString[2], &cAddr))
                        && ((sCmdFormat->cTextString[3] >= '0') && (sCmdFormat->cTextString[3] <= '7')) )
                    {
                        UINT8 cDevice = sCmdFormat->cTextString[0] - '0';
                        UINT8 cBank = sCmdFormat->cTextString[3] - '0';

                        dvIT6805_debugMessageMap(cDevice, cAddr, cBank);
                    }
                    else
                    {
                        LOG_MSG(db_ALWAYS, "\r\n ---> CMD Fail!!\r\n");
                    }
                }
                break;

            case 8:
            {
                UINT8 cDevice = 0;
                UINT8 cAddr = 0;
                UINT8 cBank = 0;
                UINT8 cReg = 0;
                UINT8 cData = 0;

                if( (sCmdFormat->cTextString[0] == '0') || (sCmdFormat->cTextString[0] == '1') )
                {
                    if( (sCmdFormat->cTextString[3] >= '0') && (sCmdFormat->cTextString[3] <= '7') )
                    {
                        cDevice = sCmdFormat->cTextString[0] - '0';
                        cBank = sCmdFormat->cTextString[3] - '0';

                        if(rcSUCCESS == AscIIToHex(sCmdFormat->cTextString[1], sCmdFormat->cTextString[2], &cAddr))
                        {
                            if(rcSUCCESS == AscIIToHex(sCmdFormat->cTextString[4], sCmdFormat->cTextString[5], &cReg))
                            {
                                if(rcSUCCESS == AscIIToHex(sCmdFormat->cTextString[6], sCmdFormat->cTextString[7], &cData))
                                {
                                    LOG_MSG(db_ALWAYS, "\r\n ---> IT6805 Set: Addr 0x%X, Bank %d, Reg 0x%X, Data 0x%X\r\n",cAddr,cBank,cReg,cData);

                                    dvIT6805_regWrite(cDevice, cAddr, cBank, cReg, cData);
                                    break;
                                }
                            }
                        }
                    }
                }

                LOG_MSG(db_ALWAYS, "\r\n ---> CMD Fail!!\r\n");
            }
            break;
        }


    }

    return eCLI_ERROR_CODE_NO;
}
#endif

// ==============================================================================
// FUNCTION NAME: utilGeneral_CFU
// DESCRIPTION:
//
//
// Params:
// eCLI_MODE eAccessMode:
// sCLI_GENERAL_FORMAT* sCmdFormat:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/11/08, Larry Create
// --------------------
// ==============================================================================
eCLI_ERROR_CODE utilGeneral_CFU(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
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
        palDataMgr_Data_Access(edcUPGRADE_SELECT, edaWRITE_THROUGH_WITH_ACTION, &ucSelect); //GuiCb.fpGui_DataCode_Value_SetCb(edcUPGRADE_SELECT, edaWRITE_THROUGH_WITH_ACTION, (INT32)ucSelect);

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
// FUNCTION NAME: utilGeneral_EXFADDR
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
eCLI_ERROR_CODE utilGeneral_EXFADDR(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
{
    if(eAccessMode == ecmRead)
    {
        LOG_MSG(db_ALWAYS, "Upgrade select\r\n");
        LOG_MSG(db_ALWAYS, "01 C734 Erase all\r\n");
        LOG_MSG(db_ALWAYS, "02 C734 Bitmap Raw date\r\n");
        LOG_MSG(db_ALWAYS, "03 C734 Text Raw date\r\n");
    }
    else if(eAccessMode == ecmWrite)
    {

    }

    return eCLI_ERROR_CODE_NO;
}

// ==============================================================================
// FUNCTION NAME: utilGeneral_EXFCHUM
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
eCLI_ERROR_CODE utilGeneral_EXFCHUM(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
{
    return eCLI_ERROR_CODE_NO;
}

// ==============================================================================
// FUNCTION NAME: utilGeneral_EXFERBL
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
eCLI_ERROR_CODE utilGeneral_EXFERBL(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
{

    return eCLI_ERROR_CODE_NO;
}

// ==============================================================================
// FUNCTION NAME: utilGeneral_EXFWREL
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
eCLI_ERROR_CODE utilGeneral_EXFWREL(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
{
    return eCLI_ERROR_CODE_NO;
}

// ==============================================================================
// FUNCTION NAME: utilGeneral_PWR
// DESCRIPTION:
//
//
// Params:
// eCLI_MODE eAccessMode:
// sCLI_GENERAL_FORMAT* sCmdFormat:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/11/08, Larry Create
// --------------------
// ==============================================================================
eCLI_ERROR_CODE utilGeneral_PWR(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
{
	eCLI_ERROR_CODE eErrorCode = eCLI_ERROR_CODE_NO;

    #if (DEBUG_CHECK_WPOFV == TRUE)
    if(halScaler_Check_WPOFV() == TRUE)   //H2FP_Simon_0187
    {
        LOG_MSG(db_ALWAYS, "WPOFV Error\r\n");
        return eCLI_ERROR_CODE_NO;
    }
	#endif

    if(eAccessMode == ecmRead)
    {
        if((palSystem_PowerStateGet() == ePOWER_STATE_STANDBY) || palEnvironment_Fake_Power_Down_Get())
        {
            sCmdFormat->lData = 0;
        }
        else
        {
            sCmdFormat->lData = 1;
        }
    }
    else if(eAccessMode == ecmWrite)
    {
#if 1   //G100_Owen_0039
        if(sCmdFormat->lData == 1)
        {
            //LOG_MSG(db_ALWAYS, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
            //G100_Clare_0055, add, >>>
            if(palEnvironment_Fake_Power_Down_Get())
            {
                palEnvironment_Fake_Power_Down_Set(FALSE);
            }
            else
            //G100_Clare_0055, add, <<<
            {
                palSystem_PowerKey();
            }
        }
        else if(sCmdFormat->lData == 0 && palSystem_PowerStateGet() != ePOWER_STATE_STANDBY && palSystem_SystemReadyGet())//G100_Steven_0120 //A35G2_BRC_Casper_0051
        {
        	//G100_Clare_0055, add, >>>
            UINT8 ucData = 0 ;
            palDataMgr_Data_Access(edcFAST_POWER_ON, edaREAD, &ucData);
            if (ucData)
            {
                palEnvironment_Fake_Power_Down_Set(TRUE);
            }
            else
            //G100_Clare_0055, add, <<<
            {
                palSystem_PowerDown();
            }
        }
        else
        {
            eErrorCode = eCLI_ERROR_CODE_SETFAIL;
        }
#else
        if(sCmdFormat->lData == 1)
        {
            palSystem_WarmUp();
        }
        else if(sCmdFormat->lData == FACTORY_RESET_NUMBER)
        {
            palDataMgr_ResetAllToDefault();
            palSystem_WarmUp();
        }
        else
        {
            palSystem_PowerDown();
        }
#endif
    }

	return eErrorCode;
}

// ==============================================================================
// FUNCTION NAME: utilGeneral_PWRSLEP
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
eCLI_ERROR_CODE utilGeneral_PWRSLEP(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
{
    #if (MERGE_2K == TRUE)
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
    #endif

    return eCLI_ERROR_CODE_NO;
}
//G100_Steven_0072
eCLI_ERROR_CODE utilGeneral_StandbyMode(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
{

	char ucOutputString[128] = {""} ;

    if(eAccessMode == ecmRead)
    {

    }
    else if(eAccessMode == ecmWrite)
    {
    	if(sCmdFormat->lData > 2 || sCmdFormat->lData < 0)
    		return eCLI_ERROR_CODE_DATAOVERRANGE;

    	if(sCmdFormat->lData == 2)
    	{
    		sprintf(ucOutputString, "Power consumption will increase\r\n");
    		__CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
    	}
    	palDataMgr_ExecuteStandbyMode(sCmdFormat->lData);
    }

    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilGeneral_AutoShutDown(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat) //A35G2_BRC_Casper_0111
{
    if(eAccessMode == ecmWrite)
    {
        if(sCmdFormat->lData < 0 || sCmdFormat->lData > 180 || sCmdFormat->lData == 1)
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE ;
        }

    	utilCommonCLI_DataConversionSet(edcAUTO_SHUTDOWN, (INT32)sCmdFormat->lData);
    }
    else if(eAccessMode == ecmRead)
    {

    }

    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilGeneral_NetWorkRst(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
{

	char ucOutputString[128] = {""} ;

    if(eAccessMode == ecmRead)
    {

    }
    else if(eAccessMode == ecmWrite)
    {

       	if(sCmdFormat->lData != 1)
       		return eCLI_ERROR_CODE_DATAOVERRANGE;

    	sprintf(ucOutputString, "Please wait while Network Settings are updated\r\n");
    	__CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
    	palDataMgr_ExecuteNetworkFactoryReset(sCmdFormat->lData);
    }

    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilGeneral_NetWorkApply(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
{

	char ucOutputString[128] = {""} ;

    if(eAccessMode == ecmRead)
    {

    }
    else if(eAccessMode == ecmWrite)
    {
    	if(sCmdFormat->lData != 1)
           	return eCLI_ERROR_CODE_DATAOVERRANGE;

    	sprintf(ucOutputString, "Please wait while Network Settings are updated\r\n");
    	__CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
    	palDataMgr_ExecuteLanIPApply(sCmdFormat->lData);
    }

    return eCLI_ERROR_CODE_NO;
}



#if 0 //A35G2_CDS_Larry_0035 mask
// ==============================================================================
// FUNCTION NAME: utilGeneral_FCS
// DESCRIPTION:
//
//
// Params:
// eCLI_MODE eAccessMode:
// sCLI_GENERAL_FORMAT* sCmdFormat:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/11/08, Larry Create
// --------------------
// ==============================================================================
eCLI_ERROR_CODE utilGeneral_FCS(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
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
			switch(sCmdFormat->ucDataType)
			{
				case eCLI_DATA_TYPE_N:
				case eCLI_DATA_TYPE_N_STEP:
					{
						sInputKey.wKeyCode = keFOCUSUP;
						sInputKey.eKeyEvent = eKEY_EVENT_PRESSED;
						sInputKey.eKeyType = eKEYINPUT_CLI;//eKEYINPUT_IR; //G100_Julie_0040

						palInputProc_BufferInsert(&sInputKey);
					}
					break;

				case eCLI_DATA_TYPE_P:
				case eCLI_DATA_TYPE_P_STEP:
					{
						sInputKey.wKeyCode = keFOCUSDOWN;
						sInputKey.eKeyEvent = eKEY_EVENT_PRESSED;
						sInputKey.eKeyType = eKEYINPUT_CLI;//eKEYINPUT_IR; //G100_Julie_0040

						palInputProc_BufferInsert(&sInputKey);
					}
					break;

				case eCLI_DATA_TYPE_N_STOP:
					break;

				case eCLI_DATA_TYPE_N_RUN:
					{
						if((GuiCb.fpGui_Get_MenuState_IsOSD_OpenCb()) && (GuiCb.fpGui_IsFocus_MenuCb() == TRUE))
						{
							sInputKey.wKeyCode = keHOLD_FOCUSUP;
							sInputKey.eKeyEvent = eKEY_EVENT_HOLD;
							sInputKey.eKeyType = eKEYINPUT_CLI;//eKEYINPUT_IR; //G100_Julie_0040

							palInputProc_BufferInsert(&sInputKey);
						}
						else
						{
							sInputKey.wKeyCode = keFOCUSUP;
							sInputKey.eKeyEvent = eKEY_EVENT_PRESSED;
							sInputKey.eKeyType = eKEYINPUT_CLI;//eKEYINPUT_IR; //G100_Julie_0040

							palInputProc_BufferInsert(&sInputKey);
						}
					}
					break;

				case eCLI_DATA_TYPE_P_STOP:
					break;

				case eCLI_DATA_TYPE_P_RUN:
					{
						if((GuiCb.fpGui_Get_MenuState_IsOSD_OpenCb()) && (GuiCb.fpGui_IsFocus_MenuCb() == TRUE))
						{
							sInputKey.wKeyCode = keHOLD_FOCUSDOWN;
							sInputKey.eKeyEvent = eKEY_EVENT_HOLD;
							sInputKey.eKeyType = eKEYINPUT_CLI;//eKEYINPUT_IR; //G100_Julie_0040

							palInputProc_BufferInsert(&sInputKey);
						}
						else
						{
							sInputKey.wKeyCode = keFOCUSDOWN;
							sInputKey.eKeyEvent = eKEY_EVENT_PRESSED;
							sInputKey.eKeyType = eKEYINPUT_CLI;//eKEYINPUT_IR; //G100_Julie_0040

							palInputProc_BufferInsert(&sInputKey);
						}
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
#endif /* 0 */

//G100_Wilsonj_0035 Start
eCLI_ERROR_CODE utilGeneral_FCSI(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
{
    if(eAccessMode == ecmRead)
    {
        return eCLI_ERROR_CODE_REQUESTFAIL;

    }
    else if(eAccessMode == ecmWrite)
    {
        UINT8 ucKeyType = eKEYINPUT_CLI;
        UINT8 ucUST = ets_OFF;
        UINT8 ucUSTPattern = ets_OFF;
        UINT8 ucLensCalDoing = 0;

        palDataMgr_Data_Access(edcUST_SET, edaREAD, &ucUST);
        palDataMgr_Data_Access(edcUST_Pattern, edaREAD, &ucUSTPattern);
        if((palSystem_PowerStateGet() == ePOWER_STATE_STANDBY) || palEnvironment_Fake_Power_Down_Get()) //A35G2_BRC_Casper_0144
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        if(palInputProc_LensMotorLockGet() == eLENS_LOCK_ALL_ALLOW)
        {
            palDataMgr_Data_Access(edcLENS_CALIBRATION, edaREAD, &ucLensCalDoing);
            if(ucLensCalDoing != 0xFF) //if(GuiCb.fpGui_LenCalibrationDoingCb()) //###
            {
                return eCLI_ERROR_CODE_SETFAIL;
            }

            if(sCmdFormat->lData == 0)
            {
				//GuiCb.fpGui_SendKeyEventCb(ucKeyType, ekFOCUS); //G100_Coda_00123
				#ifdef CORRECT_FOCUS_DIRECTION  //A35G2_Simon_0109
                palMotorFocusSet(eZOOMLENS_CMD_FOCUS_FAR_STEP);
                #else
                palMotorFocusSet(eZOOMLENS_CMD_FOCUS_NEAR_STEP);
                #endif
            }
            else if((sCmdFormat->lData >= 1)  && (sCmdFormat->lData <= 100))
            {
                //GuiCb.fpGui_SendKeyEventCb(ucKeyType, ekFOCUS);	//G100_Coda_00123
				#ifdef CORRECT_FOCUS_DIRECTION  //A35G2_Simon_0109
                palMotorFocusSet(eZOOMLENS_CMD_FOCUS_FAR_STEP);
                #else
                palMotorFocusSet(eZOOMLENS_CMD_FOCUS_NEAR_STEP);
                #endif
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
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilGeneral_FCSO(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
{
    if(eAccessMode == ecmRead)
    {
        return eCLI_ERROR_CODE_REQUESTFAIL;

    }
    else if(eAccessMode == ecmWrite)
    {
        UINT8 ucKeyType = eKEYINPUT_CLI;
        UINT8 ucUST = ets_OFF;
        UINT8 ucUSTPattern = ets_OFF;
        UINT8 ucLensCalDoing = 0;

        palDataMgr_Data_Access(edcUST_SET, edaREAD, &ucUST);
        palDataMgr_Data_Access(edcUST_Pattern, edaREAD, &ucUSTPattern);
        if((palSystem_PowerStateGet() == ePOWER_STATE_STANDBY) || palEnvironment_Fake_Power_Down_Get()) //A35G2_BRC_Casper_0144
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        if(palInputProc_LensMotorLockGet() == eLENS_LOCK_ALL_ALLOW)
        {
            palDataMgr_Data_Access(edcLENS_CALIBRATION, edaREAD, &ucLensCalDoing);
            if(ucLensCalDoing != 0xFF) //if(GuiCb.fpGui_LenCalibrationDoingCb()) //###
            {
                return eCLI_ERROR_CODE_SETFAIL;
            }

            if(sCmdFormat->lData == 0)
            {
				//GuiCb.fpGui_SendKeyEventCb(ucKeyType, ekFOCUS); //G100_Coda_00123
                palMotorFocusSet(eZOOMLENS_CMD_FOCUS_NEAR_STEP);
            }
            else if((sCmdFormat->lData >= 1)  && (sCmdFormat->lData <= 100))
            {
                //GuiCb.fpGui_SendKeyEventCb(ucKeyType, ekFOCUS); //G100_Coda_00123
                palMotorFocusSet(eZOOMLENS_CMD_FOCUS_NEAR_STEP);
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
    }
    return eCLI_ERROR_CODE_NO;
}
//G100_Wilsonj_0035 End

#if 0 //A35G2_CDS_Larry_0035 mask
// ==============================================================================
// FUNCTION NAME: utilGeneral_ZOM
// DESCRIPTION:
//
//
// Params:
// eCLI_MODE eAccessMode:
// sCLI_GENERAL_FORMAT* sCmdFormat:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/11/08, Larry Create
// --------------------
// ==============================================================================
eCLI_ERROR_CODE utilGeneral_ZOM(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
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
			switch(sCmdFormat->ucDataType)
			{
				case eCLI_DATA_TYPE_N:
				case eCLI_DATA_TYPE_N_STEP:
					{
						sInputKey.wKeyCode = keZOOMOUT;
						sInputKey.eKeyEvent = eKEY_EVENT_PRESSED;
						sInputKey.eKeyType = eKEYINPUT_CLI;//eKEYINPUT_IR; //G100_Julie_0040

						palInputProc_BufferInsert(&sInputKey);
					}
					break;

				case eCLI_DATA_TYPE_P:
				case eCLI_DATA_TYPE_P_STEP:
					{
						sInputKey.wKeyCode = keZOOMIN;
						sInputKey.eKeyEvent = eKEY_EVENT_PRESSED;
						sInputKey.eKeyType = eKEYINPUT_CLI;//eKEYINPUT_IR; //G100_Julie_0040

						palInputProc_BufferInsert(&sInputKey);
					}
					break;

				case eCLI_DATA_TYPE_N_STOP:
					break;

				case eCLI_DATA_TYPE_N_RUN:
					{
						if((GuiCb.fpGui_Get_MenuState_IsOSD_OpenCb()) && (GuiCb.fpGui_IsZoom_MenuCb() == TRUE))
						{
							sInputKey.wKeyCode = keHOLD_ZOOMOUT;
							sInputKey.eKeyEvent = eKEY_EVENT_HOLD;
							sInputKey.eKeyType = eKEYINPUT_CLI;//eKEYINPUT_IR; //G100_Julie_0040

							palInputProc_BufferInsert(&sInputKey);
						}
						else
						{
							sInputKey.wKeyCode = keZOOMOUT;
							sInputKey.eKeyEvent = eKEY_EVENT_PRESSED;
							sInputKey.eKeyType = eKEYINPUT_CLI;//eKEYINPUT_IR; //G100_Julie_0040

							palInputProc_BufferInsert(&sInputKey);
						}
					}

					break;

				case eCLI_DATA_TYPE_P_STOP:
					break;

				case eCLI_DATA_TYPE_P_RUN:
					{
						if((GuiCb.fpGui_Get_MenuState_IsOSD_OpenCb()) && (GuiCb.fpGui_IsZoom_MenuCb() == TRUE))
						{
							sInputKey.wKeyCode = keHOLD_ZOOMIN;
							sInputKey.eKeyEvent = eKEY_EVENT_HOLD;
							sInputKey.eKeyType = eKEYINPUT_CLI;//eKEYINPUT_IR; //G100_Julie_0040

							palInputProc_BufferInsert(&sInputKey);
						}
						else
						{
							sInputKey.wKeyCode = keZOOMIN;
							sInputKey.eKeyEvent = eKEY_EVENT_PRESSED;
							sInputKey.eKeyType = eKEYINPUT_CLI;//eKEYINPUT_IR; //G100_Julie_0040

							palInputProc_BufferInsert(&sInputKey);
						}
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
#endif /* 0 */

//G100_Wilsonj_0035 Start
eCLI_ERROR_CODE utilGeneral_ZOMI(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
{
    if(eAccessMode == ecmRead)
    {
        return eCLI_ERROR_CODE_REQUESTFAIL;

    }
    else if(eAccessMode == ecmWrite)
    {
        UINT8 ucKeyType = eKEYINPUT_CLI;
        UINT8 ucLensType = 0; //G100_Steven_0152
        UINT8 ucUST = ets_OFF;
        UINT8 ucUSTPattern = ets_OFF;
        UINT8 ucLensCalDoing = 0;

        palDataMgr_Data_Access(edcLENS_TYPE, edaREAD, &ucLensType); //G100_Steven_0152
        palDataMgr_Data_Access(edcUST_SET, edaREAD, &ucUST);
        palDataMgr_Data_Access(edcUST_Pattern, edaREAD, &ucUSTPattern);
        if((palSystem_PowerStateGet() == ePOWER_STATE_STANDBY) || palEnvironment_Fake_Power_Down_Get()) //A35G2_BRC_Casper_0144
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        if(palInputProc_LensMotorLockGet() == eLENS_LOCK_ALL_ALLOW)// && (ucLensType != eLENS_TYPE_KMUST) ) //A35G2_BRC_Casper_0149
        {
            palDataMgr_Data_Access(edcLENS_CALIBRATION, edaREAD, &ucLensCalDoing);
            if(ucLensCalDoing != 0xFF) //if(GuiCb.fpGui_LenCalibrationDoingCb()) //###
            {
                return eCLI_ERROR_CODE_SETFAIL;
            }

            if(sCmdFormat->lData == 0)
            {
				//GuiCb.fpGui_SendKeyEventCb(ucKeyType, ekZOOM);	//G100_Coda_00123
                palMotorZoomSet(eZOOMLENS_CMD_ZOOM_IN_STEP);
            }
            else if((sCmdFormat->lData >= 1)  && (sCmdFormat->lData <= 100))
            {
                //GuiCb.fpGui_SendKeyEventCb(ucKeyType, ekZOOM);	//G100_Coda_00123
                palMotorZoomSet(eZOOMLENS_CMD_ZOOM_IN_STEP);
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
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilGeneral_ZOMO(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
{
    if(eAccessMode == ecmRead)
    {
        return eCLI_ERROR_CODE_REQUESTFAIL;

    }
    else if(eAccessMode == ecmWrite)
    {
        UINT8 ucKeyType = eKEYINPUT_CLI;
        UINT8 ucLensType = 0; //G100_Steven_0152
        UINT8 ucUST = ets_OFF;
        UINT8 ucUSTPattern = ets_OFF;
        UINT8 ucLensCalDoing = 0;

        palDataMgr_Data_Access(edcLENS_TYPE, edaREAD, &ucLensType); //G100_Steven_0152
        palDataMgr_Data_Access(edcUST_SET, edaREAD, &ucUST);
        palDataMgr_Data_Access(edcUST_Pattern, edaREAD, &ucUSTPattern);
        if((palSystem_PowerStateGet() == ePOWER_STATE_STANDBY) || palEnvironment_Fake_Power_Down_Get()) //A35G2_BRC_Casper_0144
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        if(palInputProc_LensMotorLockGet() == eLENS_LOCK_ALL_ALLOW)// && (ucLensType != eLENS_TYPE_KMUST)) //A35G2_BRC_Casper_0149
        {
            palDataMgr_Data_Access(edcLENS_CALIBRATION, edaREAD, &ucLensCalDoing);
            if(ucLensCalDoing != 0xFF) //if(GuiCb.fpGui_LenCalibrationDoingCb()) //###
            {
                return eCLI_ERROR_CODE_SETFAIL;
            }

            if(sCmdFormat->lData == 0)
            {
				//GuiCb.fpGui_SendKeyEventCb(ucKeyType, ekZOOM);	//G100_Coda_00123
                palMotorZoomSet(eZOOMLENS_CMD_ZOOM_OUT_STEP);
            }
            else if((sCmdFormat->lData >= 1)  && (sCmdFormat->lData <= 100))
            {
                //GuiCb.fpGui_SendKeyEventCb(ucKeyType, ekZOOM);	//G100_Coda_00123
                palMotorZoomSet(eZOOMLENS_CMD_ZOOM_OUT_STEP);
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
    }
    return eCLI_ERROR_CODE_NO;
}
//G100_Wilsonj_0035 End

#if 0
// ==============================================================================
// FUNCTION NAME: utilGeneral_LVO
// DESCRIPTION:
//
//
// Params:
// eCLI_MODE eAccessMode:
// sCLI_GENERAL_FORMAT* sCmdFormat:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/11/08, Larry Create
// --------------------
// ==============================================================================
eCLI_ERROR_CODE utilGeneral_LVO(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
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
            switch(sCmdFormat->ucDataType)
            {
                case eCLI_DATA_TYPE_N:
                case eCLI_DATA_TYPE_N_STEP:
                    {
                        sInputKey.wKeyCode = keLENSVDOWN;
                        sInputKey.eKeyEvent = eKEY_EVENT_PRESSED;
                        sInputKey.eKeyType = eKEYINPUT_IR;

                        palInputProc_BufferInsert(&sInputKey);
                    }
                    break;

                case eCLI_DATA_TYPE_P:
                case eCLI_DATA_TYPE_P_STEP:
                    {
                        sInputKey.wKeyCode = keLENSVUP;
                        sInputKey.eKeyEvent = eKEY_EVENT_PRESSED;
                        sInputKey.eKeyType = eKEYINPUT_IR;

                        palInputProc_BufferInsert(&sInputKey);
                    }
                    break;

                case eCLI_DATA_TYPE_N_STOP:
                    break;

                case eCLI_DATA_TYPE_N_RUN:
                    {
						if((GuiCb.fpGui_Get_MenuState_IsOSD_OpenCb()) && (appGui_Get_MenuState_MenuIndex() == LENS_SHIFT_STANDALONEMENU_ICOUNT))
						{
							sInputKey.wKeyCode = keHOLD_LENSVDOWN;
	                        sInputKey.eKeyEvent = eKEY_EVENT_HOLD;
	                        sInputKey.eKeyType = eKEYINPUT_IR;

	                        palInputProc_BufferInsert(&sInputKey);

						}
						else
						{
	                        sInputKey.wKeyCode = keLENSVDOWN;
	                        sInputKey.eKeyEvent = eKEY_EVENT_PRESSED;
	                        sInputKey.eKeyType = eKEYINPUT_IR;

	                        palInputProc_BufferInsert(&sInputKey);
						}
                    }
                    break;

                case eCLI_DATA_TYPE_P_STOP:
                    break;

                case eCLI_DATA_TYPE_P_RUN:
                    {
						if((GuiCb.fpGui_Get_MenuState_IsOSD_OpenCb()) && (appGui_Get_MenuState_MenuIndex() == LENS_SHIFT_STANDALONEMENU_ICOUNT))
						{
	                        sInputKey.wKeyCode = keHOLD_LENSVUP;
	                        sInputKey.eKeyEvent = eKEY_EVENT_HOLD;
	                        sInputKey.eKeyType = eKEYINPUT_IR;

	                        palInputProc_BufferInsert(&sInputKey);
						}
						else
						{
							sInputKey.wKeyCode = keLENSVUP;
	                        sInputKey.eKeyEvent = eKEY_EVENT_PRESSED;
	                        sInputKey.eKeyType = eKEYINPUT_IR;

	                        palInputProc_BufferInsert(&sInputKey);
						}
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
// FUNCTION NAME: utilGeneral_LHO
// DESCRIPTION:
//
//
// Params:
// eCLI_MODE eAccessMode:
// sCLI_GENERAL_FORMAT* sCmdFormat:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/11/08, Larry Create
// --------------------
// ==============================================================================
eCLI_ERROR_CODE utilGeneral_LHO(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
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
			switch(sCmdFormat->ucDataType)
			{
				case eCLI_DATA_TYPE_N:
				case eCLI_DATA_TYPE_N_STEP:
					{
						sInputKey.wKeyCode = keLENSHRIGHT;
						sInputKey.eKeyEvent = eKEY_EVENT_PRESSED;
						sInputKey.eKeyType = eKEYINPUT_IR;

						palInputProc_BufferInsert(&sInputKey);
					}
					break;

				case eCLI_DATA_TYPE_P:
				case eCLI_DATA_TYPE_P_STEP:
					{
						sInputKey.wKeyCode = keLENSHLEFT;
						sInputKey.eKeyEvent = eKEY_EVENT_PRESSED;
						sInputKey.eKeyType = eKEYINPUT_IR;

						palInputProc_BufferInsert(&sInputKey);
					}
					break;

				case eCLI_DATA_TYPE_N_STOP:
					break;

				case eCLI_DATA_TYPE_N_RUN:
					{
						if((GuiCb.fpGui_Get_MenuState_IsOSD_OpenCb()) && (appGui_Get_MenuState_MenuIndex() == LENS_SHIFT_STANDALONEMENU_ICOUNT))
						{
							sInputKey.wKeyCode = keHOLD_LENSHRIGHT;
							sInputKey.eKeyEvent = eKEY_EVENT_HOLD;
							sInputKey.eKeyType = eKEYINPUT_IR;

							palInputProc_BufferInsert(&sInputKey);
						}
						else
						{
							sInputKey.wKeyCode = keLENSHRIGHT;
							sInputKey.eKeyEvent = eKEY_EVENT_PRESSED;
							sInputKey.eKeyType = eKEYINPUT_IR;

							palInputProc_BufferInsert(&sInputKey);
						}
					}
					break;

				case eCLI_DATA_TYPE_P_STOP:
					break;

				case eCLI_DATA_TYPE_P_RUN:
					{
						if((GuiCb.fpGui_Get_MenuState_IsOSD_OpenCb()) && (appGui_Get_MenuState_MenuIndex() == LENS_SHIFT_STANDALONEMENU_ICOUNT))
						{
							sInputKey.wKeyCode = keHOLD_LENSHLEFT;
							sInputKey.eKeyEvent = eKEY_EVENT_HOLD;
							sInputKey.eKeyType = eKEYINPUT_IR;

							palInputProc_BufferInsert(&sInputKey);
						}
						else
						{
							sInputKey.wKeyCode = keLENSHLEFT;
							sInputKey.eKeyEvent = eKEY_EVENT_PRESSED;
							sInputKey.eKeyType = eKEYINPUT_IR;

							palInputProc_BufferInsert(&sInputKey);
						}
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

#endif /* 0 */

// ==============================================================================
// FUNCTION NAME: utilGeneral_LVU
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
// 2019/07/12,  //T100_Casper_0099 Create
// --------------------
// ==============================================================================
eCLI_ERROR_CODE utilGeneral_LVU(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
{
    if(eAccessMode == ecmRead)
    {
        return eCLI_ERROR_CODE_REQUESTFAIL;

    }
    else if(eAccessMode == ecmWrite)
    {
        UINT8 ucKeyType = eKEYINPUT_CLI;
        UINT8 ucUST = ets_OFF;
        UINT8 ucUSTPattern = ets_OFF;

        palDataMgr_Data_Access(edcUST_SET, edaREAD, &ucUST);
        palDataMgr_Data_Access(edcUST_Pattern, edaREAD, &ucUSTPattern);
        if((ucUST == ets_ON && ucUSTPattern == ets_OFF) || (palSystem_PowerStateGet() == ePOWER_STATE_STANDBY) || palEnvironment_Fake_Power_Down_Get())
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        if(palInputProc_LensMotorLockGet() == eLENS_LOCK_ALL_ALLOW)
        {
            #if 0 // No Lens ID for X35Gen2
            if(!palEnvironment_LensModelCheck_Get())
            {
				GuiCb.fpGui_DataCode_Value_SetCb(edcSUPPORT_MESSAGE, edaWRITE_THROUGH_WITH_ACTION, eSOURCE_MESSAGE_SOURCE_SIGNAL_REQUIRED);
                appGui_Send_MenuOpen_Not_Supported_StandAloneMenu(); //G100 move to appgui.c
            }
            else
            #endif
            if(palEnvironment_LensCalFlag_Get() == FALSE)  //G100_Owen_0076
            {
                palDataMgr_UI_EventSend(edcUI_EVENT_MENUOPEN_LENS_CAL_MSG, TRUE, NULL); //GuiCb.fpGui_Send_MenuOpen_Lens_Calibration_ConfirmDialogCb(); //G100 code move to appgui
                return eCLI_ERROR_CODE_SETFAIL;
            }
            else
            {
                if(sCmdFormat->lData == 0)
                {
    				//GuiCb.fpGui_SendKeyEventCb(ucKeyType, ekLENSSHIFT);	//G100_Coda_00123
                    palMotorLensSet(eLENS_CMDS_STEP_UP);
                }
                else if((sCmdFormat->lData >= 1)  && (sCmdFormat->lData <= 100))
                {
                    //GuiCb.fpGui_SendKeyEventCb(ucKeyType, ekLENSSHIFT);	//G100_Coda_00123
                    palMotorLensRunPixelSet(eLENS_CMDS_PIXEL_UP, (UINT32)sCmdFormat->lData);
                }
                else
                {
                    return eCLI_ERROR_CODE_DATAOVERRANGE;
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
// FUNCTION NAME: utilGeneral_LVD
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
// 2019/07/12,  //T100_Casper_0099 Create
// --------------------
// ==============================================================================
eCLI_ERROR_CODE utilGeneral_LVD(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
{
    if(eAccessMode == ecmRead)
    {
        return eCLI_ERROR_CODE_REQUESTFAIL;

    }
    else if(eAccessMode == ecmWrite)
    {
        UINT8 ucKeyType = eKEYINPUT_CLI;
        UINT8 ucUST = ets_OFF;
        UINT8 ucUSTPattern = ets_OFF;

        palDataMgr_Data_Access(edcUST_SET, edaREAD, &ucUST);
        palDataMgr_Data_Access(edcUST_Pattern, edaREAD, &ucUSTPattern);
        if((ucUST == ets_ON && ucUSTPattern == ets_OFF) || (palSystem_PowerStateGet() == ePOWER_STATE_STANDBY) || palEnvironment_Fake_Power_Down_Get())
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        if(palInputProc_LensMotorLockGet() == eLENS_LOCK_ALL_ALLOW) //A70LV_Larry_0129
        {
            #if 0 // No Lens ID for X35Gen2
            if(!palEnvironment_LensModelCheck_Get())
            {
				GuiCb.fpGui_DataCode_Value_SetCb(edcSUPPORT_MESSAGE, edaWRITE_THROUGH_WITH_ACTION, eSOURCE_MESSAGE_SOURCE_SIGNAL_REQUIRED);
                appGui_Send_MenuOpen_Not_Supported_StandAloneMenu(); //G100 move to appgui.c
            }
            else
            #endif
            if(palEnvironment_LensCalFlag_Get() == FALSE)  //G100_Owen_0076
            {
                palDataMgr_UI_EventSend(edcUI_EVENT_MENUOPEN_LENS_CAL_MSG, TRUE, NULL); //GuiCb.fpGui_Send_MenuOpen_Lens_Calibration_ConfirmDialogCb(); //G100 code move to appgui
                return eCLI_ERROR_CODE_SETFAIL;
            }
            else
            {
                if(sCmdFormat->lData == 0)
                {
    				//GuiCb.fpGui_SendKeyEventCb(ucKeyType, ekLENSSHIFT);	//G100_Coda_00123
                    palMotorLensSet(eLENS_CMDS_STEP_DOWN);
                }
                else if((sCmdFormat->lData >= 1)  && (sCmdFormat->lData <= 100))
                {
                    //GuiCb.fpGui_SendKeyEventCb(ucKeyType, ekLENSSHIFT);	//G100_Coda_00123
                    palMotorLensRunPixelSet(eLENS_CMDS_PIXEL_DOWN, (UINT32)sCmdFormat->lData);
                }
                else
                {
                    return eCLI_ERROR_CODE_DATAOVERRANGE;
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
// FUNCTION NAME: utilGeneral_LHL
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
// 2019/07/12,  //T100_Casper_0099 Create
// --------------------
// ==============================================================================
eCLI_ERROR_CODE utilGeneral_LHL(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
{
    if(eAccessMode == ecmRead)
    {
        return eCLI_ERROR_CODE_REQUESTFAIL;

    }
    else if(eAccessMode == ecmWrite)
    {
        UINT8 ucKeyType = eKEYINPUT_CLI;
        UINT8 ucUST = ets_OFF;
        UINT8 ucUSTPattern = ets_OFF;

        palDataMgr_Data_Access(edcUST_SET, edaREAD, &ucUST);
        palDataMgr_Data_Access(edcUST_Pattern, edaREAD, &ucUSTPattern);
        if((ucUST == ets_ON && ucUSTPattern == ets_OFF) || (palSystem_PowerStateGet() == ePOWER_STATE_STANDBY) || palEnvironment_Fake_Power_Down_Get())
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        if(palInputProc_LensMotorLockGet() == eLENS_LOCK_ALL_ALLOW) //A70LV_Larry_0129
        {
            #if 0 // No Lens ID for X35Gen2
            if(!palEnvironment_LensModelCheck_Get())
            {
				GuiCb.fpGui_DataCode_Value_SetCb(edcSUPPORT_MESSAGE, edaWRITE_THROUGH_WITH_ACTION, eSOURCE_MESSAGE_SOURCE_SIGNAL_REQUIRED);
                appGui_Send_MenuOpen_Not_Supported_StandAloneMenu(); //G100 move to appgui.c
            }
            else
            #endif
            if(palEnvironment_LensCalFlag_Get() == FALSE)  //G100_Owen_0076
            {
                palDataMgr_UI_EventSend(edcUI_EVENT_MENUOPEN_LENS_CAL_MSG, TRUE, NULL); //GuiCb.fpGui_Send_MenuOpen_Lens_Calibration_ConfirmDialogCb(); //G100 code move to appgui
                return eCLI_ERROR_CODE_SETFAIL;
            }
            else
            {
                if(sCmdFormat->lData == 0)
                {
    				//GuiCb.fpGui_SendKeyEventCb(ucKeyType, ekLENSSHIFT);	//G100_Coda_00123
                    palMotorLensSet(eLENS_CMDS_STEP_LEFT);
                }
                else if((sCmdFormat->lData >= 1)  && (sCmdFormat->lData <= 100))
                {
                    //GuiCb.fpGui_SendKeyEventCb(ucKeyType, ekLENSSHIFT);	//G100_Coda_00123
                    palMotorLensRunPixelSet(eLENS_CMDS_PIXEL_LEFT, (UINT32)sCmdFormat->lData);
                }
                else
                {
                    return eCLI_ERROR_CODE_DATAOVERRANGE;
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
// FUNCTION NAME: utilGeneral_LHR
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
// 2019/07/12,  //T100_Casper_0099 Create
// --------------------
// ==============================================================================
eCLI_ERROR_CODE utilGeneral_LHR(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
{
    if(eAccessMode == ecmRead)
    {
        return eCLI_ERROR_CODE_REQUESTFAIL;
    }
    else if(eAccessMode == ecmWrite)
    {
        UINT8 ucKeyType = eKEYINPUT_CLI;
        UINT8 ucUST = ets_OFF;
        UINT8 ucUSTPattern = ets_OFF;

        palDataMgr_Data_Access(edcUST_SET, edaREAD, &ucUST);
        palDataMgr_Data_Access(edcUST_Pattern, edaREAD, &ucUSTPattern);
        if((ucUST == ets_ON && ucUSTPattern == ets_OFF) || (palSystem_PowerStateGet() == ePOWER_STATE_STANDBY) || palEnvironment_Fake_Power_Down_Get())
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        if(palInputProc_LensMotorLockGet() == eLENS_LOCK_ALL_ALLOW) //A70LV_Larry_0129
        {
            #if 0 // No Lens ID for X35Gen2
            if(!palEnvironment_LensModelCheck_Get())
            {
				GuiCb.fpGui_DataCode_Value_SetCb(edcSUPPORT_MESSAGE, edaWRITE_THROUGH_WITH_ACTION, eSOURCE_MESSAGE_SOURCE_SIGNAL_REQUIRED);
                appGui_Send_MenuOpen_Not_Supported_StandAloneMenu(); //G100 move to appgui.c
            }
            else
            #endif
            if(palEnvironment_LensCalFlag_Get() == FALSE)  //G100_Owen_0076
            {
                palDataMgr_UI_EventSend(edcUI_EVENT_MENUOPEN_LENS_CAL_MSG, TRUE, NULL); //GuiCb.fpGui_Send_MenuOpen_Lens_Calibration_ConfirmDialogCb(); //G100 code move to appgui
                return eCLI_ERROR_CODE_SETFAIL;
            }
            else
            {
                if(sCmdFormat->lData == 0)
                {
    				//GuiCb.fpGui_SendKeyEventCb(ucKeyType, ekLENSSHIFT);	//G100_Coda_00123
                    palMotorLensSet(eLENS_CMDS_STEP_RIGHT);
                }
                else if((sCmdFormat->lData >= 1)  && (sCmdFormat->lData <= 100))
                {
                    //GuiCb.fpGui_SendKeyEventCb(ucKeyType, ekLENSSHIFT);	//G100_Coda_00123
                    palMotorLensRunPixelSet(eLENS_CMDS_PIXEL_RIGHT, (UINT32)sCmdFormat->lData);
                }
                else
                {
                    return eCLI_ERROR_CODE_DATAOVERRANGE;
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

eCLI_ERROR_CODE utilGeneral_LELO(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat) //A35G2_CDS_Larry_0035
{
    UINT8 ucLensCalDoing = 0;

    if(eAccessMode == ecmRead)
    {
        return eCLI_ERROR_CODE_NO;
    }
    else if(eAccessMode == ecmWrite)
    {
        palDataMgr_Data_Access(edcLENS_CALIBRATION, edaREAD, &ucLensCalDoing);
        if(ucLensCalDoing == 0xFF) //if(GuiCb.fpGui_LenCalibrationDoingCb() == FALSE)  //###
        {
            UINT8 ucLock = 0;

            ucLock = (UINT8)sCmdFormat->lData;

            palDataMgr_Data_Access(edcLOCK_ALL_LENS_MOTORS, edaWRITE_THROUGH_WITH_ACTION, &ucLock);
        }
        else
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilGeneral_PSID(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)   //A70LV_Doulas_0138
{
    INT32 iWheelIndex = 0;
    eEXEC_CODE eResult = eEXEC_CODE_PASS;

    //if(palDataMgr_ServiceModeGet() == ets_OFF)
    //{
    //    return eCLI_ERROR_CODE_DECIMALERR;
    //}

    if(utilCommonCLI_DataConversionGet(edcPHOSPHOR_WHEEL_INDEX, (INT32*)&iWheelIndex) != eEXEC_CODE_PASS)
    {
        if(eAccessMode == ecmRead)
            return eCLI_ERROR_CODE_REQUESTFAIL;
        else
            return eCLI_ERROR_CODE_SETFAIL;
    }

    if(eAccessMode == ecmRead)
    {
        sCmdFormat->lData = (INT32) iWheelIndex;
    }
    else if(eAccessMode == ecmWrite)
    {
        if((sCmdFormat->lData <= PW_INDEX_MAX_VALUE) && (sCmdFormat->lData >= PW_INDEX_MIN_VALUE))
        {
            iWheelIndex = (UINT16)sCmdFormat->lData;
            eResult = utilCommonCLI_DataConversionSet(edcPHOSPHOR_WHEEL_INDEX, (INT32)iWheelIndex);
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

eCLI_ERROR_CODE utilGeneral_FSID(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)   //A70LV_Doulas_0138
{
    INT32 iWheelIndex = 0;
    eEXEC_CODE eResult = eEXEC_CODE_PASS;

   //if(palDataMgr_ServiceModeGet() == ets_OFF)
   //{
   //    return eCLI_ERROR_CODE_DECIMALERR;
   //}

    if(utilCommonCLI_DataConversionGet(edcFILTER_WHEEL_INDEX, (INT32*)&iWheelIndex) != eEXEC_CODE_PASS)
    {
        if(eAccessMode == ecmRead)
            return eCLI_ERROR_CODE_REQUESTFAIL;
        else
            return eCLI_ERROR_CODE_SETFAIL;
    }

    if(eAccessMode == ecmRead)
    {
        sCmdFormat->lData = (INT32) iWheelIndex;
    }
    else if(eAccessMode == ecmWrite)
    {
        if((sCmdFormat->lData <= FW_INDEX_MAX_VALUE) && (sCmdFormat->lData >= FW_INDEX_MIN_VALUE))
        {
            iWheelIndex = (UINT16)sCmdFormat->lData;
            eResult = utilCommonCLI_DataConversionSet(edcFILTER_WHEEL_INDEX, (INT32)iWheelIndex);
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
eCLI_ERROR_CODE utilGeneral_PIV(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)       //A70LV_Doulas_0139
{
    eEXEC_CODE eResult = eEXEC_CODE_PASS;
    INT32 iPIN_PROTECT_EN;

    if(utilCommonCLI_DataConversionGet(edcPIN_PROTECT, (INT32*)&iPIN_PROTECT_EN) != eEXEC_CODE_PASS)
    {
        if(eAccessMode == ecmRead)
            return eCLI_ERROR_CODE_REQUESTFAIL;
        else
            return eCLI_ERROR_CODE_SETFAIL;
    }

    if(eAccessMode == ecmRead)
    {
        sCmdFormat->lData = iPIN_PROTECT_EN;
    }
    else if(eAccessMode == ecmWrite)
    {
        if(sCmdFormat->ucDataType == eCLI_DATA_TYPE_STRING)
        {
            UINT8 ucPWord[5] = {0};
            UINT8 ucPWord_Input[5] = {0};
            UINT8 ucVAl;

            //LOG_MSG(db_ALWAYS, "utilGeneral_PIV (%s) \r\n", sCmdFormat->cTextString);
            if(strlen((char *)sCmdFormat->cTextString) != 4)  //G100_Wilsonj_0037
            {
                return eCLI_ERROR_CODE_DATAOVERRANGE;
            }

            if(utilCommonCLI_String_Get(edcCHANGE_PIN, ucPWord) != eEXEC_CODE_PASS)
            {
                return eCLI_ERROR_CODE_FunctionNoDefine;
            }

            ucVAl = 0;
            for(; ucVAl < 4 ; ucVAl++)  //G100_Wilsonj_0037
            {
                if((sCmdFormat->cTextString[ucVAl] >= '0') &&
                   (sCmdFormat->cTextString[ucVAl] <= '9'))
                {
                    ucPWord_Input[ucVAl] = sCmdFormat->cTextString[ucVAl] ;                       //T100_Simon_0048
                    if(ucPWord_Input[ucVAl] != ucPWord[ucVAl])
                    {
                        return eCLI_ERROR_CODE_DECIMALERR;
                    }
                }
                else
                {
                    return eCLI_ERROR_CODE_DATAERROR;
                }
            }

            if(iPIN_PROTECT_EN)
            {
                iPIN_PROTECT_EN = 0;
            }
            else
            {
                iPIN_PROTECT_EN = 1;
            }

            eResult = utilCommonCLI_DataConversionSet(edcPIN_PROTECT, iPIN_PROTECT_EN) ;
            if(eResult != eEXEC_CODE_PASS)
            {
                return eCLI_ERROR_CODE_FunctionNoDefine;
            }
        }
        else
        {
            return eCLI_ERROR_CODE_FUNCCODENOTFOUND;
        }

    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilGeneral_PCG(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)       //A70LV_Doulas_0139
{
    eEXEC_CODE eResult = eEXEC_CODE_PASS;

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

            //LOG_MSG(db_ALWAYS, "utilGeneral_PCG (%s) \r\n", sCmdFormat->cTextString);

            if(strlen((char *)sCmdFormat->cTextString) != 9)  //G100_Wilsonj_0037
            {
                return eCLI_ERROR_CODE_DATAOVERRANGE;
            }

            if(utilCommonCLI_String_Get(edcCHANGE_PIN, ucPWord) != eEXEC_CODE_PASS)
            {
                return eCLI_ERROR_CODE_FunctionNoDefine;
            }

            ucVAl = 0;
            for(; ucVAl < 4 ; ucVAl++)      //check password  //G100_Wilsonj_0037
            {
                if((sCmdFormat->cTextString[ucVAl] >= '0') &&
                   (sCmdFormat->cTextString[ucVAl] <= '9'))
                {
                    ucPWord_Input[ucVAl] = sCmdFormat->cTextString[ucVAl] ;                       //T100_Simon_0048
                    if(ucPWord_Input[ucVAl] != ucPWord[ucVAl])
                    {
                        return eCLI_ERROR_CODE_DECIMALERR;
                    }
                }
                else
                {
                    return eCLI_ERROR_CODE_DATAERROR;
                }
            }

            if(sCmdFormat->cTextString[4] != ',')
            {
                return eCLI_ERROR_CODE_DATAOVERRANGE;
            }

            ucVAl = 5;  //G100_Wilsonj_0037
            for(; ucVAl < 9 ; ucVAl++)     //check new password  //G100_Wilsonj_0037
            {
                if((sCmdFormat->cTextString[ucVAl] >= '0') &&
                   (sCmdFormat->cTextString[ucVAl] <= '9'))
                {
                    ucNewPWord[ucVAl - 5] = sCmdFormat->cTextString[ucVAl] ;
                }
                else
                {
                    return eCLI_ERROR_CODE_DECIMALERR;
                }
            }

            eResult = utilCommonCLI_String_Set(edcCHANGE_PIN, ucNewPWord) ;
            if(eResult != eEXEC_CODE_PASS)
            {
                return eCLI_ERROR_CODE_FunctionNoDefine;
            }
        }
        else
        {
            return eCLI_ERROR_CODE_DATAERROR;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilGeneral_SHU_LOCK(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat) //A70LV_Larry_0384 //T100_Casper_0093
{
    if(eAccessMode == ecmWrite)
    {
        if(sCmdFormat->lData == 0)
        {
            sDATABASE_ITEM_DATA_FORMAT sInputData;
            INT32 uiShutter = 0;

            utilCommonCLI_DataConversionSet(edcLENS_DETECTION, 0);
            utilCommonCLI_DataConversionGet(edcPICTURE_MUTE, &uiShutter);

            sInputData.uiItemIndex = edcPICTURE_MUTE;
            sInputData.uValue.lValue = uiShutter;
            palDataMgr_NotifyGroupingEvent(sInputData);
            if(uiShutter)
            {
                utilCommonCLI_DataConversionSet(edcPICTURE_MUTE, 0);
            }
        }
        else
        {
            utilCommonCLI_DataConversionSet(edcLENS_DETECTION, 1);
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilGeneral_SIV(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)       //A70LV_Doulas_0139
{
    if(eAccessMode == ecmRead)
    {
        sCmdFormat->lData = GENERAL_MSSC_VER;
    }
    else if(eAccessMode == ecmWrite)
    {
        return eCLI_ERROR_CODE_COMMAND_READ_ONLY;
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilGeneral_SST(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)       //A70LV_Doulas_0140  //T100_Simon_0050 //T100IR_Casper_0032
{
    UINT8 ucString[64] = {0};
    char  ucOutputString[128] = {""};      //T100_Simon_0020 Start
    UINT8 ucIndex = 0;
    UINT8 ucVal = 0;
    INT32 iPIP_EN = 0;

    char cSourceName[eCM_SOURCE_NUMBER][16] =
    {
        "Reserved",
        "VGA",
        "BNC",
        "HDMI 1",
        "HDMI 2",
        "DVI",
        "Display Port",
        "3GSDI",
        "HDBaseT",
        "Composite",
        "Presenter",
        "Card Reader",
        "MinUSB",
        "Slot 1",
        "Slot 2",
    };

    if(eAccessMode == ecmRead)
    {
        char cMainCode[GENERAL_MAIN_CMD_LEN + 1]; //+1 for '\0'
        ucString[0] = '\0';

        for(ucIndex = 0; ucIndex < GENERAL_MAIN_CMD_LEN; ucIndex++)
        {
            cMainCode[ucIndex] = sCmdFormat->cMainCode[ucIndex];
            Low2UpCase(&cMainCode[ucIndex]);
        }

        cMainCode[GENERAL_MAIN_CMD_LEN] = '\0';

        ucIndex = 0;

        utilCommonCLI_String_Get(edcMODEL_NAME, ucString);
        sprintf(ucOutputString, "\r%c%s!%03d  \"%s\" \"Model Name\"%c\r\n", m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_Hander, cMainCode, ucIndex, ucString, m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_End);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        ucIndex++;

        utilCommonCLI_String_Get(edcSERIAL_NUMBER, ucString);
        sprintf(ucOutputString, "%c%s!%03d  \"%s\" \"Serial Number\"%c\r\n", m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_Hander, cMainCode, ucIndex, ucString, m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_End);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        ucIndex++;

        utilCommonCLI_String_Get(edcNATIVE_RESOLUTION, ucString);
        sprintf(ucOutputString, "%c%s!%03d  \"%s\" \"Native Resolution\"%c\r\n", m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_Hander, cMainCode, ucIndex, ucString, m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_End);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        ucIndex++;

        utilCommonCLI_String_Get(edcRELEASE_VERSION, ucString);
        sprintf(ucOutputString, "%c%s!%03d  \"%s\" \"FW Version\"%c\r\n", m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_Hander, cMainCode, ucIndex, ucString, m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_End);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        ucIndex++;

        utilCommonCLI_String_Get(edcFMCU_VERSION, ucString);
        sprintf(ucOutputString, "%c%s!%03d  \"  %s\" \"F-MCU Version\"%c\r\n", m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_Hander, cMainCode, ucIndex, ucString, m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_End);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        ucIndex++;

        utilCommonCLI_String_Get(edcSMCU_VERSION, ucString);
        sprintf(ucOutputString, "%c%s!%03d  \"  %s\" \"S-MCU Version\"%c\r\n", m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_Hander, cMainCode, ucIndex, ucString, m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_End);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        ucIndex++;

        utilCommonCLI_String_Get(edcMOTOR_VERSION, ucString);
        sprintf(ucOutputString, "%c%s!%03d  \"  %s\" \"M-MCU Version\"%c\r\n", m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_Hander, cMainCode, ucIndex, ucString, m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_End);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        ucIndex++;

        utilCommonCLI_String_Get(edcLD_DRIVER_VERSION, ucString);
        sprintf(ucOutputString, "%c%s!%03d  \"  %s\" \"L-MCU Version\"%c\r\n", m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_Hander, cMainCode, ucIndex, ucString, m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_End);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        ucIndex++;

        utilCommonCLI_String_Get(edcKEYPAD_VERSION, ucString);
        sprintf(ucOutputString, "%c%s!%03d  \"  %s\" \"K-MCU Version\"%c\r\n", m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_Hander, cMainCode, ucIndex, ucString, m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_End);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        ucIndex++;

        utilCommonCLI_String_Get(edcPMCU_VERSION, ucString);
        sprintf(ucOutputString, "%c%s!%03d  \"  %s\" \"P-MCU Version\"%c\r\n", m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_Hander, cMainCode, ucIndex, ucString, m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_End);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        ucIndex++;

        utilCommonCLI_String_Get(edcLAN_VERSION, ucString);
        sprintf(ucOutputString, "%c%s!%03d  \"  %s\" \"LAN Version\"%c\r\n", m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_Hander, cMainCode, ucIndex, ucString, m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_End);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        ucIndex++;

        utilCommonCLI_String_Get(edcFORMATER_VERSION, ucString);
        sprintf(ucOutputString, "%c%s!%03d  \"  %s\" \"Formatter Version\"%c\r\n", m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_Hander, cMainCode, ucIndex, ucString, m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_End);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        ucIndex++;

        utilCommonCLI_String_Get(edcHDBASET_VERSION, ucString);
        sprintf(ucOutputString, "%c%s!%03d  \"  %s\" \"HDBaseT Version\"%c\r\n", m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_Hander, cMainCode, ucIndex, ucString, m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_End);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        ucIndex++;

        MS_SLEEP(100);  //T100_Simon_0041

        palDataMgr_Data_Access(edcMAIN_INPUT, edaREAD, &ucVal);

        if(ucVal < eCM_SOURCE_NUMBER)
        {
            sprintf(ucOutputString, "%c%s!%03d  \"%s\" \"Main Input\"%c\r\n", m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_Hander, cMainCode, ucIndex, cSourceName[ucVal], m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_End);
        }
        else
        {
            sprintf(ucOutputString, "%c%s!%03d  \"%s\" \"Main Input\"%c\r\n", m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_Hander, cMainCode, ucIndex, "-",m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_End);
        }

        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        ucIndex++;

		utilCommonCLI_String_Get(edcMAIN_RESOLUTION, ucString) ;
        sprintf(ucOutputString, "%c%s!%03d  \"%s\" \"Main Resolution\"%c\r\n", m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_Hander, cMainCode, ucIndex, ucString, m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_End);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        ucIndex++;

        utilCommonCLI_String_Get(edcMAIN_SIGNAL_FORMAT, ucString);
        sprintf(ucOutputString, "%c%s!%03d  \"%s\" \"Main Signal Format\"%c\r\n", m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_Hander, cMainCode, ucIndex, ucString, m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_End);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        ucIndex++;

        utilCommonCLI_String_Get(edcMAIN_PIXEL_CLOCK, ucString);
        sprintf(ucOutputString, "%c%s!%03d  \"%s\" \"Main Pixel Clock\"%c\r\n", m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_Hander, cMainCode, ucIndex, ucString, m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_End);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        ucIndex++;

        /*utilCommonCLI_String_Get(edcMAIN_SYNC_TYPE, edaREAD, ucString);
        sprintf(ucOutputString, "%c%s!%03d  \"%s\" \"Main Sync Type\"%c\n", m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_Hander, cMainCode, ucIndex, ucString, m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_End);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        ucIndex++;*/

        utilCommonCLI_String_Get(edcMAIN_HORZ_REFRESH, ucString);
        sprintf(ucOutputString, "%c%s!%03d  \"%s\" \"Main Horz Refresh\"%c\r\n", m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_Hander, cMainCode, ucIndex, ucString, m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_End);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        ucIndex++;

        utilCommonCLI_String_Get(edcMAIN_VERT_REFRESH, ucString);
        sprintf(ucOutputString, "%c%s!%03d  \"%s\" \"Main Vert Refresh\"%c\r\n", m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_Hander, cMainCode, ucIndex, ucString, m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_End);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        ucIndex++;

    //----------------------
        utilCommonCLI_DataConversionGet(edcPIP_PBP_ENABLE, (INT32*)&iPIP_EN);
        if(iPIP_EN != ets_OFF)
        {
            palDataMgr_Data_Access(edcSUB_INPUT, edaREAD, &ucVal);
            sprintf(ucOutputString, "%c%s!%03d  \"%s\" \"PIP / PBP Input\"%c\r\n", m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_Hander, cMainCode, ucIndex, cSourceName[ucVal], m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_End);
        }
        else
        {
            sprintf(ucOutputString, "%c%s!%03d  \"%s\" \"PIP / PBP Input\"%c\r\n", m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_Hander, cMainCode, ucIndex, "-", m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_End);
        }

        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        ucIndex++;

		utilCommonCLI_String_Get(edcSUB_RESOLUTION, ucString) ;
        sprintf(ucOutputString, "%c%s!%03d  \"%s\" \"PIP / PBP Resolution\"%c\r\n", m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_Hander, cMainCode, ucIndex, ucString, m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_End);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        ucIndex++;

        utilCommonCLI_String_Get(edcSUB_SIGNAL_FORMAT, ucString);
        sprintf(ucOutputString, "%c%s!%03d  \"%s\" \"PIP / PBP Signal Format\"%c\r\n", m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_Hander, cMainCode, ucIndex, ucString, m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_End);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        ucIndex++;

        utilCommonCLI_String_Get(edcSUB_PIXEL_CLOCK, ucString);
        sprintf(ucOutputString, "%c%s!%03d  \"%s\" \"PIP / PBP Pixel Clock\"%c\r\n", m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_Hander, cMainCode, ucIndex, ucString, m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_End);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        ucIndex++;

        utilCommonCLI_String_Get(edcSUB_HORZ_REFRESH, ucString);
        sprintf(ucOutputString, "%c%s!%03d  \"%s\" \"PIP / PBP Horz Refresh\"%c\r\n", m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_Hander, cMainCode, ucIndex, ucString, m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_End);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        ucIndex++;

        utilCommonCLI_String_Get(edcSUB_VERT_REFRESH, ucString);
        sprintf(ucOutputString, "%c%s!%03d  \"%s\" \"PIP / PBP Vert Refresh\"%c\r\n", m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_Hander, cMainCode, ucIndex, ucString, m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_End);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        ucIndex++;

        MS_SLEEP(100);  //T100_Simon_0041

    //----------------------
		utilCommonCLI_String_Get(edcPICTURE_SETTINGS, ucString);
        sprintf(ucOutputString, "%c%s!%03d  \"%s\" \"Display Mode\"%c\r\n", m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_Hander, cMainCode, ucIndex, ucString, m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_End);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        ucIndex++;

        utilCommonCLI_String_Get(edcCOLOR_SPACE, &ucVal);
        if(ucVal == 0)
        {
           sprintf(ucOutputString, "%c%s!%03d  \"%s\" \"Color Space Setting\"%c\r\n", m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_Hander, cMainCode, ucIndex, "Auto", m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_End);
        }
        else if(ucVal == 1)
        {
           sprintf(ucOutputString, "%c%s!%03d  \"%s\" \"Color Space Setting\"%c\r\n", m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_Hander, cMainCode, ucIndex, "RGB Full", m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_End);
        }
        else if(ucVal == 2)
        {
           sprintf(ucOutputString, "%c%s!%03d  \"%s\" \"Color Space Setting\"%c\r\n", m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_Hander, cMainCode, ucIndex, "RGB Limited", m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_End);
        }
        else if(ucVal == 3)
        {
           sprintf(ucOutputString, "%c%s!%03d  \"%s\" \"Color Space Setting\"%c\r\n", m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_Hander, cMainCode, ucIndex, "REC709", m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_End);
        }
        else if(ucVal == 4)
        {
           sprintf(ucOutputString, "%c%s!%03d  \"%s\" \"Color Space Setting\"%c\r\n", m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_Hander, cMainCode, ucIndex, "REC601", m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_End);
        }
        else
        {
           sprintf(ucOutputString, "%c%s!%03d  \"%s\" \"Color Space Setting\"%c\r\n", m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_Hander, cMainCode, ucIndex, "-", m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_End);
        }
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        ucIndex++;

        utilCommonCLI_String_Get(edcINFO_LIGHT_SOURCE_POWER, ucString);
        sprintf(ucOutputString, "%c%s!%03d  \"%s\" \"Light Source Power\"%c\r\n", m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_Hander, cMainCode, ucIndex, ucString, m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_End);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        ucIndex++;

        utilCommonCLI_String_Get(edcTOTAL_PROJECTOR_HOURS, ucString);
        sprintf(ucOutputString, "%c%s!%03d  \"%s\" \"Total Projector Hours\"%c\r\n", m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_Hander, cMainCode, ucIndex, ucString, m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_End);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        ucIndex++;

        utilCommonCLI_String_Get(edcSTANDBY_MODE, &ucVal);
        if(ucVal == eSTANDBY_MODE_0d5W)
        {
            sprintf(ucOutputString, "%c%s!%03d  \"%s\" \"Standby Mode\"%c\r\n", m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_Hander, cMainCode, ucIndex, "0.5W Mode", m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_End);
        }
        else
        {
            sprintf(ucOutputString, "%c%s!%03d  \"%s\" \"Standby Mode\"%c\r\n", m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_Hander, cMainCode, ucIndex, "Communication Mode", m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_End);
        }
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        ucIndex++;

        ucVal = palGeo_ApLinkFlag_Get();  //A35G2_Simon_0086
        if(ucVal == 0)
        {
            sprintf(ucOutputString, "%c%s!%03d  \"%s\" \"Twisk Link\"%c\r\n", m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_Hander, cMainCode, ucIndex, "Off", m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_End);
        }
        else
        {
            sprintf(ucOutputString, "%c%s!%03d  \"%s\" \"Twisk Link\"%c\r\n", m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_Hander, cMainCode, ucIndex, "On", m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_End);
        }
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        ucIndex++;

        utilCommonCLI_String_Get(edcLOCK_ALL_LENS_MOTORS, &ucVal);
        if(ucVal == eLENS_LOCK_ALL_LOCKED)
        {
            sprintf(ucOutputString, "%c%s!%03d  \"%s\" \"Lock all Lens Motors\"%c\r\n", m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_Hander, cMainCode, ucIndex, "Locked", m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_End);
        }
        else
        {
            sprintf(ucOutputString, "%c%s!%03d  \"%s\" \"Lock all Lens Motors\"%c\r\n", m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_Hander, cMainCode, ucIndex, "Allow", m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_End);
        }
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        ucIndex++;

        utilCommonCLI_String_Get(edcSYSTEM_TEMPERATURE, ucString);
        sprintf(ucOutputString, "%c%s!%03d  \"%s\" \"System Temperature\"%c\r\n", m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_Hander, cMainCode, ucIndex, ucString, m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_End);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);                 //T100_Simon_0020 End
        ucIndex++;
    }
    else if(eAccessMode == ecmWrite)
    {
        return eCLI_ERROR_CODE_COMMAND_READ_ONLY;
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilGeneral_ILI(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)       //A70LV_Doulas_0140
{
    UINT8 ucString[64];
    char  ucOutputString[128] = {""};     //T100_Simon_0020 Start
    UINT8 ucIndex = 0;

    if(eAccessMode == ecmRead)
    {
        char cMainCode[GENERAL_MAIN_CMD_LEN + 1]; //+1 for '\0'
        ucString[0] = '\0';

        for(ucIndex = 0; ucIndex < GENERAL_MAIN_CMD_LEN; ucIndex++)
        {
            cMainCode[ucIndex] = sCmdFormat->cMainCode[ucIndex];
            Low2UpCase(&cMainCode[ucIndex]);
        }

        cMainCode[GENERAL_MAIN_CMD_LEN] = '\0';

        ucIndex = 0;
        utilCommonCLI_String_Get(edcTOTAL_PROJECTOR_HOURS_INFO, ucString);
        sprintf(ucOutputString, "\r%c%s!%03d  \"%s\" \"Total Projector Hours\"%c\n", m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_Hander, cMainCode, ucIndex, ucString, m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_End);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        ucIndex++;

        utilCommonCLI_String_Get(edcLD_HOURS_INFO, ucString);
        sprintf(ucOutputString, "%c%s!%03d  \"%s\" \"LD Hours\"%c\n", m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_Hander, cMainCode, ucIndex, ucString, m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_End);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);     //T100_Simon_0020 End
        ucIndex++;
    }
    else if(eAccessMode == ecmWrite)
    {
        return eCLI_ERROR_CODE_COMMAND_READ_ONLY;
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilGeneral_DEF(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)   //T100_Doulas_0029
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
            if(palDataMgr_ServiceModeGet() == ets_OFF)
            {
                return eCLI_ERROR_CODE_DECIMALERR;
            }
            else
            {
                //MenuOpen_WaitMessageMenu();
                palDataMgr_ServiceModeSet(ets_ON);
                eResult = utilCommonCLI_DataConversionSet(edcFACTORY_RESET, (INT32)ucValue) ;
                if(eResult != eEXEC_CODE_PASS)
                {
                    palDataMgr_UI_EventSend(edcUI_EVENT_OSD_EXIT, TRUE, NULL); //GuiCb.fpGui_Send_OSD_ExitCb();
                    return eCLI_ERROR_CODE_SETFAIL;
                }
                palDataMgr_UI_EventSend(edcUI_EVENT_OSD_EXIT, TRUE, NULL); //GuiCb.fpGui_Send_OSD_ExitCb();
            }
        }
        else
            return eCLI_ERROR_CODE_DATAERROR;
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilGeneral_ICI(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)       //A70LV_Doulas_0231 modify//A70LV_Doulas_0151
{
    UINT8 ucString[64] = {0};
    char  ucOutputString[128] = {""};
    UINT8 ucIndex = 0;
    INT32 iVal;

    if(eAccessMode == ecmRead)
    {
        char cMainCode[GENERAL_MAIN_CMD_LEN + 1]; //+1 for '\0'
        ucString[0] = '\0';

        for(ucIndex = 0; ucIndex < GENERAL_MAIN_CMD_LEN; ucIndex++)
        {
            cMainCode[ucIndex] = sCmdFormat->cMainCode[ucIndex];
            Low2UpCase(&cMainCode[ucIndex]);
        }

        cMainCode[GENERAL_MAIN_CMD_LEN] = '\0';

        ucIndex = 0;

        sprintf(ucOutputString, "\r%c%s!%03d  \"LAN\"%c\n", m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_Hander, cMainCode, ucIndex, m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_End);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        ucIndex++;

        //palDataMgr_Data_Access(edcCOMMUNICATIONS_INFO_CONNECTION, edaREAD, ucString);
        //sprintf(ucOutputString, "%c%s!%03d  \"%s\" \"Connection\"%c\n",ucIndex, ucString);
        //__CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        //ucIndex++;

        utilCommonCLI_DataConversionGet(edcLAN_DHCP, (INT32*)&iVal);
        if(iVal == 1)
        {
            sprintf(ucOutputString, "%c%s!%03d  \"%s\" \"DHCP\"%c\n", m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_Hander, cMainCode, ucIndex, "On", m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_End);
        }
        else
        {
            sprintf(ucOutputString, "%c%s!%03d  \"%s\" \"DHCP\"%c\n", m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_Hander, cMainCode, ucIndex, "Off", m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_End); //T100_Simon_0008 End
        }
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        ucIndex++;

        utilCommonCLI_String_Get(edcLAN_IP_ADDRESS, ucString);
        sprintf(ucOutputString, "%c%s!%03d  \"%s\" \"IP Address\"%c\n", m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_Hander, cMainCode, ucIndex, ucString, m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_End);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        ucIndex++;

        utilCommonCLI_String_Get(edcLAN_SUBNET_MASK, ucString);
        sprintf(ucOutputString, "%c%s!%03d  \"%s\" \"Subnet Mask\"%c\n", m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_Hander, cMainCode, ucIndex, ucString, m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_End);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        ucIndex++;

        utilCommonCLI_String_Get(edcLAN_DEFAULT_GATEWAY, ucString);
        sprintf(ucOutputString, "%c%s!%03d  \"%s\" \"Gateway\"%c\n",  m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_Hander, cMainCode, ucIndex, ucString, m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_End);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        ucIndex++;

        utilCommonCLI_String_Get(edcLAN_PRIMARY_DNS, ucString);
        sprintf(ucOutputString, "%c%s!%03d  \"%s\" \"Primary DNS\"%c\n", m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_Hander, cMainCode, ucIndex, ucString, m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_End);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        ucIndex++;

        utilCommonCLI_String_Get(edcLAN_SECOND_DNS, ucString);
        sprintf(ucOutputString, "%c%s!%03d  \"%s\" \"Secondary DNS\"%c\n", m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_Hander, cMainCode, ucIndex, ucString, m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_End);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        ucIndex++;

        utilCommonCLI_String_Get(edcLAN_MAC_ADDRESS, ucString);
        sprintf(ucOutputString, "%c%s!%03d  \"%s\" \"MAC Address\"%c\n", m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_Hander, cMainCode, ucIndex, ucString, m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_End);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        ucIndex++;

        ucString[0] = '\0';
        sprintf(ucOutputString, "%c%s!%03d  \"WLAN\"%c\n", m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_Hander, cMainCode, ucIndex, m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_End);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        ucIndex++;

        utilCommonCLI_String_Get(edcWLAN_SSID, ucString);
        sprintf(ucOutputString, "%c%s!%03d  \"%s\" \"SSID\"%c\n", m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_Hander, cMainCode, ucIndex, ucString, m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_End);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        ucIndex++;

        utilCommonCLI_String_Get(edcWLAN_DEFAULT_GATEWAY, ucString);
        sprintf(ucOutputString, "%c%s!%03d  \"%s\" \"Default Gateway\"%c\n", m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_Hander, cMainCode, ucIndex, ucString, m_sCli_General_Config[sCmdFormat->ucCmdFrom].cCMD_End);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        ucIndex++;

    }
    else if(eAccessMode == ecmWrite)
    {
        return eCLI_ERROR_CODE_COMMAND_READ_ONLY;
    }

    return eCLI_ERROR_CODE_NO;
}


eCLI_ERROR_CODE utilGeneral_LCE(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
{
    if(eAccessMode == ecmRead)
    {
        INT32 iLastErrorCode = 0 ;
        utilCommonCLI_DataConversionGet(edcLAST_ERRORCODE, (INT32*)&iLastErrorCode);

        sCmdFormat->lData = iLastErrorCode;
    }
    else if(eAccessMode == ecmWrite)
    {
        return eCLI_ERROR_CODE_COMMAND_READ_ONLY;
    }

    return eCLI_ERROR_CODE_NO;
}


eCLI_ERROR_CODE utilGeneral_DRD(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)  //T100_Casper_0008
{

	//halFormatter_EEPROM_Reset();

    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilGeneral_Debug(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)  //T100_Simon_0024
{
    if(eAccessMode == ecmWrite)
    {
        UINT8 ucIndex = 0;
    	switch(sCmdFormat->lData)
    	{
    	    case 0:
    	        //palDataMgr_ShowGeometryFlag();    //G100_Wilsonj_0018 mask
    	        break;

    	    case 1: //G100_Coda_0052
    	        palSystem_WarmUp();
    	        break;

            #ifdef CHECKING_OUTPUT_3D_FIELD  //H30K_Doulas_0035
            case 9: //A35G2_Simon_0121 only for debug
                halScaler_STATECH1();
                break;
            #endif

		   case 298:	//A35G2_Coda_0101
		        ucIndex = ePROSERVICE_STATUS_ERROR_REGISTER_LOST;
                palDataMgr_Data_Access(edcPROSERVICE_BINDING_STATUS, edaWRITE_RAM_ONLY_WITH_ACTION, &ucIndex); //GuiCb.fpGui_DataCode_Value_SetCb(edcPROSERVICE_BINDING_STATUS, edaWRITE_RAM_ONLY_WITH_ACTION, ePROSERVICE_STATUS_ERROR_REGISTER_LOST);
                break;

           case 299:
		        ucIndex = ePROSERVICE_STATUS_IDLE;
                palDataMgr_Data_Access(edcPROSERVICE_BINDING_STATUS, edaWRITE_RAM_ONLY_WITH_ACTION, &ucIndex); //GuiCb.fpGui_DataCode_Value_SetCb(edcPROSERVICE_BINDING_STATUS, edaWRITE_RAM_ONLY_WITH_ACTION, ePROSERVICE_STATUS_IDLE);
                break;

            case 300:
		        ucIndex = ePROSERVICE_STATUS_NO_BINDING;
                palDataMgr_Data_Access(edcPROSERVICE_BINDING_STATUS, edaWRITE_RAM_ONLY_WITH_ACTION, &ucIndex); //GuiCb.fpGui_DataCode_Value_SetCb(edcPROSERVICE_BINDING_STATUS, edaWRITE_RAM_ONLY_WITH_ACTION, ePROSERVICE_STATUS_NO_BINDING);
                break;

            case 301:
		        ucIndex = ePROSERVICE_STATUS_BINDING;
                palDataMgr_Data_Access(edcPROSERVICE_BINDING_STATUS, edaWRITE_RAM_ONLY_WITH_ACTION, &ucIndex); //GuiCb.fpGui_DataCode_Value_SetCb(edcPROSERVICE_BINDING_STATUS, edaWRITE_RAM_ONLY_WITH_ACTION, ePROSERVICE_STATUS_BINDING);
                break;

            case 302:
		        ucIndex = ePROSERVICE_PAIR_STATUS_PAIR_CODE_READY_FROM_SERVER;
                palDataMgr_Data_Access(edcPROSERVICE_PAIR_STATUS, edaWRITE_RAM_ONLY_WITH_ACTION, &ucIndex); //GuiCb.fpGui_DataCode_Value_SetCb(edcPROSERVICE_PAIR_STATUS, edaWRITE_RAM_ONLY_WITH_ACTION, ePROSERVICE_PAIR_STATUS_PAIR_CODE_READY_FROM_SERVER);
                break;

            case 303:
            {
                char PairCode[8] = {"7788"};
                palDataMgr_Data_Access(edcPROSERVICE_PAIR_CODE, edaWRITE_THROUGH_WITH_ACTION, PairCode); //GuiCb.fpGui_DataCode_String_SetCb(edcPROSERVICE_PAIR_CODE, PairCode);
            }
            break;

            case 304:
            {
                char Account[20] = {"Coda HPBU TW 538"};
                palDataMgr_Data_Access(edcPROSERVICE_ACCOUNT, edaWRITE_THROUGH_WITH_ACTION, Account); //GuiCb.fpGui_DataCode_String_SetCb(edcPROSERVICE_ACCOUNT, Account);
            }
                break;

            case 305:
            {
		        ucIndex = ePROSERVICE_PAIR_STATUS_PAIR_CODE_SUCCESS_FROM_SERVER;
                palDataMgr_Data_Access(edcPROSERVICE_PAIR_STATUS, edaWRITE_RAM_ONLY_WITH_ACTION, &ucIndex); //GuiCb.fpGui_DataCode_Value_SetCb(edcPROSERVICE_PAIR_STATUS, edaWRITE_RAM_ONLY_WITH_ACTION, ePROSERVICE_PAIR_STATUS_PAIR_CODE_SUCCESS_FROM_SERVER);
            }
                break;

            case 306:
            {
                UINT32 ProService_ErrorCode = 12345;
                palDataMgr_Data_Access(edcPROSERVICE_ERROR_CODE, edaWRITE_RAM_ONLY_WITH_ACTION, &ProService_ErrorCode); //GuiCb.fpGui_DataCode_Value_SetCb(edcPROSERVICE_ERROR_CODE, edaWRITE_RAM_ONLY_WITH_ACTION, ProService_ErrorCode);

		        ucIndex = ePROSERVICE_PAIR_STATUS_PAIR_CODE_ERROR;
                palDataMgr_Data_Access(edcPROSERVICE_PAIR_STATUS, edaWRITE_RAM_ONLY_WITH_ACTION, &ucIndex); //GuiCb.fpGui_DataCode_Value_SetCb(edcPROSERVICE_PAIR_STATUS, edaWRITE_RAM_ONLY_WITH_ACTION, ePROSERVICE_PAIR_STATUS_PAIR_CODE_ERROR);
            }
                break;

    	    case 400:
		        ucIndex = eFOTA_SYSTEM_UPDATE_STATUS_NEW_FW_EXIST;
    	        palDataMgr_Data_Access(edcSystemUpdateStatus, edaWRITE_THROUGH_WITH_ACTION, &ucIndex); //GuiCb.fpGui_DataCode_Value_SetCb(edcSystemUpdateStatus, edaWRITE_THROUGH_WITH_ACTION, eFOTA_SYSTEM_UPDATE_STATUS_NEW_FW_EXIST);
    	        break;

    	    case 401:
		        ucIndex = eFOTA_SYSTEM_UPDATE_STATUS_NO_NEW_FW;
    	        palDataMgr_Data_Access(edcSystemUpdateStatus, edaWRITE_THROUGH_WITH_ACTION, &ucIndex); //GuiCb.fpGui_DataCode_Value_SetCb(edcSystemUpdateStatus, edaWRITE_THROUGH_WITH_ACTION, eFOTA_SYSTEM_UPDATE_STATUS_NO_NEW_FW);
    	        break;

    	    case 500:
    	    {
    	        tDATA_CODE value;
    	        palDataMgr_Data_Access(edcCOLOR_WHEEL_SPEED, edaREAD, &value); //GuiCb.fpGui_DataCode_Value_GetCb(edcCOLOR_WHEEL_SPEED, &value);
    	        LOG_MSG(db_ALWAYS, "Get edcCOLOR_WHEEL_SPEED %d\n", value);

    	    }
    	        break;

    	}
    }

	if(eAccessMode == ecmRead)
	{
		LOG_MSG(db_ALWAYS, "\n[--- Debug Mode ---]\n");
		LOG_MSG(db_ALWAYS, "0:ShowGeometryFlag\n");
	}

    return eCLI_ERROR_CODE_NO;
}

//G100_Coda_0013
eCLI_ERROR_CODE utilGeneral_Debug_Schedule(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)  //G100_Coda
{
    if(eAccessMode == ecmWrite)
    {
    	switch(sCmdFormat->lData)
    	{
    	    case 0:
                //GuiCb.fpGui_Send_MenuOpen_Schedule_NextMenu_L2Cb(); //G100 move to appgui.c //###
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

                    palDataMgr_Schedule_Unpack_LanPacket(&sSendSchedule);

                    palLANProcSturctDataSend(uwStructID, sizeof(sLAN_SCHEDULE_StructPayload), (void *)&sStructPayload);

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
	}

    return eCLI_ERROR_CODE_NO;
}


#if 0


eCLI_ERROR_CODE utilGeneral_Debug_MEMC(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)  //T100_Coda_
{
    if(eAccessMode == ecmWrite)
    {
    	switch(sCmdFormat->lData)
    	{
    		case 0:
                halScaler_Dv7410_Reset();
                break;

    	    case 10:
                MEMCBoard_SetFreeze(0);
                break;

            case 11:
                MEMCBoard_SetFreeze(1);
                break;
			case 20:
				MEMCBoard_SetVideoMute(0);
				break;
			case 21:
				MEMCBoard_SetVideoMute(1);
				break;
			case 30:
				MEMCBoard_SetPQByPass(0);
				break;
            case 31:
				MEMCBoard_SetPQByPass(1);
				break;

            case 90: // close
                MEMCBoard_SetRGBByPass(0);
                break;

            case 91: // enable
                MEMCBoard_SetRGBByPass(1);
                break;

			default:
				break;

    	}
    }

	if(eAccessMode == ecmRead)
	{
		printf("\r\n[debug Mode - MEMC ]\r\n");
		printf("0     : Reset MEMC\r\n");
		printf("10    : UnFreeze MEMC\r\n");
		printf("11    : Freeze MEMC\r\n");
	}

    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilGeneral_Debug_FrontEndBoard(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)  //T100_Coda_
{
	//#include "halMCUCtrlAPI.h"
	extern void halMCU_FrontendChip_Reset(UINT8 cFrontendDebug);     //T100_Coda

    if(eAccessMode == ecmWrite)
    {
    	switch(sCmdFormat->lData)
    	{
    		case eFE_IC_RESET_CMD_SCALER_4337: //T100IR_Casper_0021
				printf("reset Scaler NXP4337\r\n");
                halMCU_FrontendChip_Reset(0);
                break;

			case eFE_IC_RESET_CMD_MST9U13: //T100IR_Casper_0021
				printf("reset FrontEnd Mst9u13\r\n");
                halMCU_FrontendChip_Reset(3);
                break;

			case eFE_IC_RESET_CMD_IT6634: //T100IR_Casper_0021
				printf("reset FrontEnd IT6634\r\n");
                halMCU_FrontendChip_Reset(6);
                break;

			default:
				break;

    	}
    }

	if(eAccessMode == ecmRead)
	{
		printf("\r\n[debug Mode - Frontend Board IC ]\r\n");
		printf("0     : Reset Scaler Nxp4337 by frontend\r\n");
		printf("3     : Reset Mst9u13\r\n");
		printf("6     : Reset It6634\r\n");
	}

    return eCLI_ERROR_CODE_NO;
}

#endif



eCLI_ERROR_CODE utilGeneral_WAS(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)  //T100_Simon_0031
{
    if(eAccessMode == ecmWrite)
    {
        if(sCmdFormat->lData == 0 || sCmdFormat->lData > 5 )
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE ;
        }

    	UINT8 ucValue = sCmdFormat->lData - 1 ;
    	utilCommonCLI_DataConversionSet(edcWARP_MEMORY_SAVE, (INT32)ucValue);
    }
    else if(eAccessMode == ecmRead)
    {
        INT32 iValue = 0 ;
        utilCommonCLI_DataConversionGet(edcWARP_MEMORY_SAVE, &iValue);
        sCmdFormat->lData = iValue + 1 ;
    }

    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilGeneral_LIF_TPHS(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat) //A70LV_Larry_0268
{
	if(eAccessMode == ecmRead)
	{
		UINT32 ulVlaue1 = 0;
		UINT32 ulVlaue2 = 0;

		utilCommonCLI_DataConversionGet(edcTOTAL_PROJECTOR_HOURS, (INT32*)&ulVlaue1);
		utilCommonCLI_DataConversionGet(edcLD_HOURS, (INT32*)&ulVlaue2);

        if(ulVlaue1 > ulVlaue2)
        {
		    sCmdFormat->lData= (INT32)(ulVlaue1/60);
        }
        else
        {
		    sCmdFormat->lData= (INT32)(ulVlaue2/60);
        }

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


eCLI_ERROR_CODE utilGeneral_LIF_LSHS(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat) //A70LV_Larry_0268
{
	if(eAccessMode == ecmRead)
	{
		UINT32 ulVlaue1 = 0;
		UINT32 ulVlaue2 = 0;

		utilCommonCLI_DataConversionGet(edcLD_HOURS, (INT32*)&ulVlaue1) ;
		//palDataMgr_Data_Access(edcLD_HOURS_2, edaREAD, &ulVlaue2) ;

        if(ulVlaue1 > ulVlaue2)
        {
            sCmdFormat->lData= (INT32)(ulVlaue1/60);
        }
        else
        {
            sCmdFormat->lData= (INT32)(ulVlaue2/60);
        }

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

eCLI_ERROR_CODE utilGeneral_FVI(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
{
    UINT8 ucString[64] = {0};
    char  ucOutputString[128] = {""};
    UINT8 ucIndex = 0;

    if(eAccessMode == ecmRead)
    {
        UINT16 cIndex = 0, cTotal = 0;
        UINT32 ulDataCode;
        char cVerString[64] = {""};
        char cMainCode[GENERAL_MAIN_CMD_LEN + 1]; //+1 for '\0'
        ucString[0] = '\0';

        for(ucIndex = 0; ucIndex < GENERAL_MAIN_CMD_LEN; ucIndex++)
        {
            cMainCode[ucIndex] = sCmdFormat->cMainCode[ucIndex];
            Low2UpCase(&cMainCode[ucIndex]);
        }

        cMainCode[GENERAL_MAIN_CMD_LEN] = '\0';
#if 1
        cTotal = CmdCfg_VersionInfo_Get(0, &ulDataCode, cVerString);

        for(cIndex = 0; cIndex < cTotal; cIndex++)
        {
            CmdCfg_VersionInfo_Get(cIndex, &ulDataCode, cVerString);

            palDataMgr_Data_Access(ulDataCode, edaREAD, ucString);
            sprintf(ucOutputString, "\r\"%s\"         %s\r\n", cVerString, ucString);
            __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);

       }

#else

        palDataMgr_Data_Access(edcRELEASE_VERSION, edaREAD, ucString); //A35G2_Coda_0108
        sprintf(ucOutputString, "\r\"Main Version\"         %s\r\n", ucString);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);

         palDataMgr_Data_Access(edcSMCU_VERSION, edaREAD, ucString);
        sprintf(ucOutputString, "\r\"I-SCALER Version\"     %s\r\n", ucString);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);

        palDataMgr_Data_Access(edcFRONTEND_VERSION, edaREAD, ucString);
        sprintf(ucOutputString, "\r\"F-MCU Version\"        %s\r\n", ucString);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);

        palDataMgr_Data_Access(edcFMCU_VERSION, edaREAD, ucString);
        sprintf(ucOutputString, "\r\"A-MCU Version\"        %s\r\n", ucString);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);

        palDataMgr_Data_Access(edcKEYPAD_VERSION, edaREAD, ucString);
        sprintf(ucOutputString, "\r\"K-MCU Version\"        %s\r\n", ucString);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);

        palDataMgr_Data_Access(edcLAN_VERSION, edaREAD, ucString);
        sprintf(ucOutputString, "\r\"LAN Version\"          %s\r\n", ucString);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);

        palDataMgr_Data_Access(edcFORMATER_VERSION, edaREAD, ucString);
        sprintf(ucOutputString, "\r\"Formatter Version\"    %s\r\n", ucString);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);

        palDataMgr_Data_Access(edcFPGA1_VERSION, edaREAD, ucString);
        sprintf(ucOutputString, "\r\"FPGA0 Version\"        %s\r\n", ucString);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);

        palDataMgr_Data_Access(edcFPGA2_VERSION, edaREAD, ucString);
        sprintf(ucOutputString, "\r\"FPGA1 Version\"        %s\r\n", ucString);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);

        palDataMgr_Data_Access(edcXFPGA_VERSION, edaREAD, ucString);
        sprintf(ucOutputString, "\r\"XFPGA Version\"        %s\r\n", ucString);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);

        palDataMgr_Data_Access(edcHDBASET_VERSION, edaREAD, ucString);
        sprintf(ucOutputString, "\r\"HDBaseT Version\"      %s\r\n", ucString);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);

        palDataMgr_Data_Access(edcCAMERA_FW_VERSION, edaREAD, ucString); //A35G2_Coda_0088
        sprintf(ucOutputString, "\r\"Camera Version\"       %s\r\n", ucString);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);

        palDataMgr_Data_Access(edcHW_PROJECTOR_ID, edaREAD, ucString);
        sprintf(ucOutputString, "\r\"HW Version\"       %s\r\n", ucString); //A35G2_Coda_0126
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        ucIndex++;
#endif
    }
    else if(eAccessMode == ecmWrite)
    {
        return eCLI_ERROR_CODE_COMMAND_READ_ONLY;
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilGeneral_IST(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
{
    if(eAccessMode == ecmRead)
    {
        return eCLI_ERROR_CODE_NO;
    }
    else if(eAccessMode == ecmWrite)
    {
    	UINT8 ucCount = 0;
        UINT16 uiData = sCmdFormat->lData, uiValue = 0;;
        eRESULT eResult_sys = rcERROR, eResult_FE = rcERROR, eResult_DDP = rcERROR;
        float fRate = 0;

        //clear error count
		utilOPD_MasterDevice_Count_Reset();

        //i2c+uart+spi stress test.
        for(UINT16 uiNum = 0; uiNum < uiData; uiNum++)
        {
            if((uiNum % 10 == 0) && (uiNum >= 10))
            {
                fRate = (float)uiNum * 100 / uiData;
                LOG_MSG(db_ALWAYS,"Stress Test Rate = %2.2f%\r\n", fRate);
            }

			ucCount = 1;
            //eResult_sys = halMCUCtrl_System_IST_Set(1); //i2c system
            eResult_sys = halMCUCtrl_OPD_Register_Set(eOPD_MSG_I2C_STRESS_TEST, 1, &ucCount);
            if(eResult_sys != rcSUCCESS)
            {
                utilOPD_MasterDevice_Count_Set(eIF_I2C, eI2C_STANDBY);
            }

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

eCLI_ERROR_CODE utilGeneral_LSAT(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat) //G100_Larry_0007
{
    if(eAccessMode == ecmRead)
    {
        UINT32 ulNormal = 0;
        UINT32 ulEco = 0;
        UINT32 ulQuiet = 0;
        UINT32 ulCustom = 0;

        utilCommonCLI_DataConversionGet(edcLIGHT_SOURCE_HOURS_NORAML, (INT32*)&ulNormal);
        utilCommonCLI_DataConversionGet(edcLIGHT_SOURCE_HOURS_ECO, (INT32*)&ulEco);
        utilCommonCLI_DataConversionGet(edcLIGHT_SOURCE_HOURS_QUIET, (INT32*)&ulQuiet);
        utilCommonCLI_DataConversionGet(edcLIGHT_SOURCE_HOURS_CUSTOM, (INT32*)&ulCustom);

        ulNormal = ulNormal + ulEco + ulQuiet + ulCustom;

        sCmdFormat->lData = (INT32)(ulNormal/60);

        return eCLI_ERROR_CODE_NO;
    }
    else if(eAccessMode == ecmWrite)
    {
		return eCLI_ERROR_CODE_COMMAND_READ_ONLY;
	}
    return eCLI_ERROR_CODE_NO;
}


#if 1 //A35G2_BRC_Casper_0147
//G100_Tim_002, add, start
// Read only, reply the throw ratio according to the Lens Type, TR x.x ~ x.x ( same as the OSD displayed )
eCLI_ERROR_CODE utilGeneral_LEPT(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
{
    if(eAccessMode == ecmRead)
    {
        char cLensName[32] = cUNKNOWN;

        utilCommonCLI_String_Get(edcLENS_TYPE_THROW_RATIO, cLensName);
        sprintf(sCmdFormat->cTextString, "%s", cLensName);
    }
    else if(eAccessMode == ecmWrite)
    {
        // read only
		return eCLI_ERROR_CODE_DECIMALERR;
	}
    return eCLI_ERROR_CODE_NO;
}
//G100_Tim_002, add, end
#endif /* 0 */


eCLI_ERROR_CODE utilGeneral_LSNT(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
{
    if(eAccessMode == ecmRead)
    {
        UINT32 ulVlaue = 0;

        utilCommonCLI_DataConversionGet(edcLIGHT_SOURCE_HOURS_NORAML, (INT32*)&ulVlaue);

        sCmdFormat->lData = (INT32)(ulVlaue/60);

        return eCLI_ERROR_CODE_NO;
    }
    else if(eAccessMode == ecmWrite)
    {
		UINT32 ulVlaue = (UINT32)sCmdFormat->lData;

		ulVlaue = ulVlaue * 60;

		if(eEXEC_CODE_PASS == utilCommonCLI_RangeCheck(edcLIGHT_SOURCE_HOURS_NORAML, (INT32)ulVlaue))
		{
            //palIllumination_Init_LDHoursSet(ulVlaue, TRUE);
            utilCommonCLI_DataConversionSet(edcLIGHT_SOURCE_HOURS_NORAML, (INT32)ulVlaue);

			return eCLI_ERROR_CODE_NO;
	    }

		return eCLI_ERROR_CODE_DECIMALERR;
	}
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilGeneral_LSET(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
{
    if(eAccessMode == ecmRead)
    {
        UINT32 ulVlaue = 0;

        utilCommonCLI_DataConversionGet(edcLIGHT_SOURCE_HOURS_ECO, (INT32*)&ulVlaue);

        sCmdFormat->lData = (INT32)(ulVlaue/60);

        return eCLI_ERROR_CODE_NO;
    }
    else if(eAccessMode == ecmWrite)
    {
		UINT32 ulVlaue = (UINT32)sCmdFormat->lData;

		ulVlaue = ulVlaue * 60;

		if(eEXEC_CODE_PASS == utilCommonCLI_RangeCheck(edcLIGHT_SOURCE_HOURS_ECO, (INT32)ulVlaue))
		{
            //palIllumination_Init_LDHoursSet(ulVlaue, TRUE);
            utilCommonCLI_DataConversionSet(edcLIGHT_SOURCE_HOURS_ECO, (INT32)ulVlaue);

			return eCLI_ERROR_CODE_NO;
	    }
		return eCLI_ERROR_CODE_DECIMALERR;
	}
    return eCLI_ERROR_CODE_NO;
}


eCLI_ERROR_CODE utilGeneral_LSQT(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
{
    if(eAccessMode == ecmRead)
    {
        UINT32 ulVlaue = 0;

        utilCommonCLI_DataConversionGet(edcLIGHT_SOURCE_HOURS_QUIET, (INT32*)&ulVlaue);

        sCmdFormat->lData = (INT32)(ulVlaue/60);

        return eCLI_ERROR_CODE_NO;
    }
    else if(eAccessMode == ecmWrite)
    {
		UINT32 ulVlaue = (UINT32)sCmdFormat->lData;

		ulVlaue = ulVlaue * 60;

		if(eEXEC_CODE_PASS == utilCommonCLI_RangeCheck(edcLIGHT_SOURCE_HOURS_QUIET, (INT32)ulVlaue))
		{
            //palIllumination_Init_LDHoursSet(ulVlaue, TRUE);
            utilCommonCLI_DataConversionSet(edcLIGHT_SOURCE_HOURS_QUIET, (INT32)ulVlaue);

			return eCLI_ERROR_CODE_NO;
	    }

		return eCLI_ERROR_CODE_DECIMALERR;
	}
    return eCLI_ERROR_CODE_NO;
}


eCLI_ERROR_CODE utilGeneral_LSCT(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
{
    if(eAccessMode == ecmRead)
    {
        UINT32 ulVlaue = 0;

        utilCommonCLI_DataConversionGet(edcLIGHT_SOURCE_HOURS_CUSTOM, (INT32*)&ulVlaue);

        sCmdFormat->lData = (INT32)(ulVlaue/60);

        return eCLI_ERROR_CODE_NO;
    }
    else if(eAccessMode == ecmWrite)
    {
		UINT32 ulVlaue = (UINT32)sCmdFormat->lData;

		ulVlaue = ulVlaue * 60;

		if(eEXEC_CODE_PASS == utilCommonCLI_RangeCheck(edcLIGHT_SOURCE_HOURS_CUSTOM, (INT32)ulVlaue))
		{
            //palIllumination_Init_LDHoursSet(ulVlaue, TRUE);
            utilCommonCLI_DataConversionSet(edcLIGHT_SOURCE_HOURS_CUSTOM, (INT32)ulVlaue);

			return eCLI_ERROR_CODE_NO;
	    }

		return eCLI_ERROR_CODE_DECIMALERR;
	}
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilGeneral_PMUT(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat) //G100_Larry_0007
{
    if(eAccessMode == ecmRead)
    {
        INT32 iVlaue = 0;

        utilCommonCLI_DataConversionGet(edcPICTURE_MUTE, (INT32*)&iVlaue);

        sCmdFormat->lData = iVlaue;
    }
    else if(eAccessMode == ecmWrite)
    {
		UINT8 cVlaue = (sCmdFormat->lData != 0) ? 1 : 0;

        switch((UINT32)sCmdFormat->lData)
        {
            case 0:
            case 1:
                if(eEXEC_CODE_PASS != utilCommonCLI_DataConversionSet(edcPICTURE_MUTE, (INT32)cVlaue))
                {
                    return eCLI_ERROR_CODE_SETFAIL;
                }
                break;

            default:
                 return eCLI_ERROR_CODE_DATAOVERRANGE;
        }
	}

    return eCLI_ERROR_CODE_NO;
}

#if 0 //A35G2_CDS_Larry_0035 mask
//G100_Steven_0090
eCLI_ERROR_CODE utilGeneral_FRZE(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat) //G100_Larry_0007
{
    if(eAccessMode == ecmRead)
    {
        INT32 iVlaue = 0;

        utilCommonCLI_DataConversionGet(edcIMAGE_FREEZE, (INT32*)&iVlaue);

        sCmdFormat->lData = iVlaue;
    }
    else if(eAccessMode == ecmWrite)
    {
		//UINT8 cVlaue = (sCmdFormat->lData != 0) ? 1 : 0;

    	UINT8 cVlaue = sCmdFormat->lData;

        switch((UINT32)sCmdFormat->lData)
        {
            case 0:
            case 1:
                if(eEXEC_CODE_PASS != utilCommonCLI_DataConversionSet(edcIMAGE_FREEZE, (INT32)cVlaue))
                {
                    return eCLI_ERROR_CODE_SETFAIL;
                }
                break;

            default:
                 return eCLI_ERROR_CODE_DATAOVERRANGE;
        }

	}

    return eCLI_ERROR_CODE_NO;
} //G100_Steven_0090
#endif /* 0 */

eCLI_ERROR_CODE utilGeneral_OPD(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
{
    if(eAccessMode == ecmWrite)
    {

    }
    else //Read
    {
        utilOPD_CopyToUsb();
    }

    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilGeneral_OPDRSET(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
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

eCLI_ERROR_CODE utilGeneral_TARG(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat) //G100_Julie_00020
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

eCLI_ERROR_CODE utilGeneral_DPMO(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)		//G100_Doulas_0053 Modify
{
    if(eAccessMode == ecmWrite)
    {
        char uacVoltage[AC_VOLTAGE_MAX_LEN] = {0};          //G100_Tim_0009, mod, 5 //G100_Clare_0046

    	if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(edcPICTURE_SETTINGS))		//G100_Doulas_0068 Add
		{
			return eCLI_ERROR_CODE_SETFAIL;
		}

        if((UINT32)sCmdFormat->lData > CLI_PICTURE_SETTINGS_USER)
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }

		//Check 2D hight speed/ 3D
		if(palDataMgr_IS_3D_Enable())
		{
			if((sCmdFormat->lData != CLI_PICTURE_SETTINGS_3D) &&
			   (sCmdFormat->lData != CLI_PICTURE_SETTINGS_USER))
				return eCLI_ERROR_CODE_SETFAIL;
		}
		else if(halScaler_Input_3D_Format_Get() == eINPUT_3D_TYPE_FRAME_SEQUENTIAL)
		{
			if((sCmdFormat->lData != CLI_PICTURE_SETTINGS_2D_HIGH_SPEED) &&
			   (sCmdFormat->lData != CLI_PICTURE_SETTINGS_USER))
			   return eCLI_ERROR_CODE_SETFAIL;
		}
		else
		{
			if((sCmdFormat->lData == CLI_PICTURE_SETTINGS_2D_HIGH_SPEED) ||
			   (sCmdFormat->lData == CLI_PICTURE_SETTINGS_3D))
			   return eCLI_ERROR_CODE_SETFAIL;
		}

        INT32 iHDR_Auto = ets_OFF;

        utilCommonCLI_DataConversionGet(edcHDR_AUTOENABLE, &iHDR_Auto);
		//Check HDR
        if((iHDR_Auto == ets_ON) && (palImgMgr_HDR_Get(eSOURCE_WINDOW_MAIN) == eHDR_SETTING_HDR))
        {
        	if((sCmdFormat->lData != CLI_PICTURE_SETTINGS_HDR) &&
			   (sCmdFormat->lData != CLI_PICTURE_SETTINGS_USER))
        	{
            	return eCLI_ERROR_CODE_SETFAIL;
        	}
        }
        else
        {
        	if(sCmdFormat->lData == CLI_PICTURE_SETTINGS_HDR)
			{
            	return eCLI_ERROR_CODE_SETFAIL;
        	}
        }
 		//G100_Clare_0046, add, >>>
        palSystem_AC_Voltage_Check();   //G100_Tim_0009, add
        palDataMgr_Access_AC_Voltage_Info_Get(edaREAD, (void*)&uacVoltage);
        #if 0
        if((UINT32)sCmdFormat->lData == CLI_PICTURE_SETTINGS_SUPER_BRIGHT
        && ((palDataMgr_Model_ID_Get() == MODULE_TYPE_ID0_PLATFORM)
        || (palDataMgr_Model_ID_Get() == MODULE_TYPE_ID1_PLATFORM)
        || (palDataMgr_Model_ID_Get() == MODULE_TYPE_ID2_PLATFORM)
        || (uacVoltage[1] == '1')))
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
        #endif

		//G100_Clare_0046, add, <<<
		if(eEXEC_CODE_PASS == utilCommonCLI_DataConversionSet(edcPICTURE_SETTINGS, sCmdFormat->lData))
        {

        }
        else
        {
        	return eCLI_ERROR_CODE_SETFAIL;
        }

		#if 0
        switch((UINT32)sCmdFormat->lData)
        {
            case CLI_PICTURE_SETTINGS_3D:
            case CLI_PICTURE_SETTINGS_2D_HIGH_SPEED:
                return eCLI_ERROR_CODE_SETFAIL;
            default:		//G100_Doulas_0046 Modify
                {
                    INT32 iHDR_Auto = ets_OFF;

                    utilCommonCLI_DataConversionGet(edcHDR_AUTOENABLE, &iHDR_Auto);

                    if((iHDR_Auto == ets_ON) && (palDataPath_HDR_Info_Get() == eHDR_SETTING_HDR))
                    {
                    	if((sCmdFormat->lData != CLI_PICTURE_SETTINGS_HDR) &&
						   (sCmdFormat->lData != CLI_PICTURE_SETTINGS_USER))
                    	{
                        	return eCLI_ERROR_CODE_SETFAIL;
                    	}
                    }
                    else
                    {
                    	if(sCmdFormat->lData == CLI_PICTURE_SETTINGS_HDR)
						{
                        	return eCLI_ERROR_CODE_SETFAIL;
                    	}
                    }

					if(eEXEC_CODE_PASS == utilCommonCLI_DataConversionSet(edcPICTURE_SETTINGS, sCmdFormat->lData))
                    {

                    }
                    else
                    {
                    	return eCLI_ERROR_CODE_SETFAIL;
                    }
                }
                break;

        }
		#endif
    }
    else //Read
    {

    }

    return eCLI_ERROR_CODE_NO;
}


eCLI_ERROR_CODE utilGeneral_BKLV_ASEL(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
{
    if(utilWarp_GetWarpCtrl() != WARP_CTRL__ADVANCED)  //G100_Simon_0005
    {
        if(eAccessMode == ecmRead)
            return eCLI_ERROR_CODE_REQUESTFAIL;
        else
            return eCLI_ERROR_CODE_SETFAIL;
    }

    if(eAccessMode == ecmRead)
    {
        sCmdFormat->lData = (INT32)halWarp_BlacklevelParameterGet(eBKLV_EVENT_AREA_SEL);
    }
    else if(eAccessMode == ecmWrite)
    {
        INT32 cValue = sCmdFormat->lData;

        if(cValue < MAX_BLACKLEVEL_AREA_NUMBER)
        {
            halWarp_BlacklevelParameterSet(eBKLV_EVENT_AREA_SEL, cValue);
        }
        else
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }
    }

    return eCLI_ERROR_CODE_NO;
}


eCLI_ERROR_CODE utilGeneral_BKLV_ENAB(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
{
    if(utilWarp_GetWarpCtrl() != WARP_CTRL__ADVANCED)  //G100_Simon_0005
    {
        if(eAccessMode == ecmRead)
            return eCLI_ERROR_CODE_REQUESTFAIL;
        else
            return eCLI_ERROR_CODE_SETFAIL;
    }

    if(eAccessMode == ecmRead)
    {
        sCmdFormat->lData = (INT32)halWarp_BlacklevelParameterGet(eBKLV_EVENT_AREA_ENABLE);
    }
    else if(eAccessMode == ecmWrite)
    {
        INT32 cValue = sCmdFormat->lData;

        halWarp_BlacklevelParameterSet(eBKLV_EVENT_AREA_ENABLE, !!cValue);
    }

    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilGeneral_BKLV_TLCX(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
{
    if(utilWarp_GetWarpCtrl() != WARP_CTRL__ADVANCED)  //G100_Simon_0005
    {
        if(eAccessMode == ecmRead)
            return eCLI_ERROR_CODE_REQUESTFAIL;
        else
            return eCLI_ERROR_CODE_SETFAIL;
    }

    if(eAccessMode == ecmRead)
    {
        sCmdFormat->lData = (INT32)halWarp_BlacklevelParameterGet(eBKLV_EVENT_CUR_TL_X);
    }
    else if(eAccessMode == ecmWrite)
    {
        INT32 cValue = sCmdFormat->lData;

        if(cValue >= 0 && cValue < halWarping_HResGet())
        {
            halWarp_BlacklevelParameterSet(eBKLV_EVENT_CUR_TL_X, cValue);
        }
        else
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }

    }

    return eCLI_ERROR_CODE_NO;
}


eCLI_ERROR_CODE utilGeneral_BKLV_TLCY(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
{
    if(utilWarp_GetWarpCtrl() != WARP_CTRL__ADVANCED)  //G100_Simon_0005
    {
        if(eAccessMode == ecmRead)
            return eCLI_ERROR_CODE_REQUESTFAIL;
        else
            return eCLI_ERROR_CODE_SETFAIL;
    }

    if(eAccessMode == ecmRead)
    {
        sCmdFormat->lData = (INT32)halWarp_BlacklevelParameterGet(eBKLV_EVENT_CUR_TL_Y);
    }
    else if(eAccessMode == ecmWrite)
    {
        INT32 cValue = sCmdFormat->lData;

        if(cValue >= 0 && cValue < halWarping_VResGet())
        {
            halWarp_BlacklevelParameterSet(eBKLV_EVENT_CUR_TL_Y, cValue);
        }
        else
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }
    }

    return eCLI_ERROR_CODE_NO;
}


eCLI_ERROR_CODE utilGeneral_BKLV_TRCX(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
{
    if(utilWarp_GetWarpCtrl() != WARP_CTRL__ADVANCED)  //G100_Simon_0005
    {
        if(eAccessMode == ecmRead)
            return eCLI_ERROR_CODE_REQUESTFAIL;
        else
            return eCLI_ERROR_CODE_SETFAIL;
    }

    if(eAccessMode == ecmRead)
    {
        sCmdFormat->lData = (INT32)halWarp_BlacklevelParameterGet(eBKLV_EVENT_CUR_TR_X);
    }
    else if(eAccessMode == ecmWrite)
    {
        INT32 cValue = sCmdFormat->lData;

        if(cValue >= 0 && cValue < halWarping_HResGet())
        {
            halWarp_BlacklevelParameterSet(eBKLV_EVENT_CUR_TR_X, cValue);
        }
        else
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }
    }

    return eCLI_ERROR_CODE_NO;
}


eCLI_ERROR_CODE utilGeneral_BKLV_TRCY(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
{
    if(utilWarp_GetWarpCtrl() != WARP_CTRL__ADVANCED)  //G100_Simon_0005
    {
        if(eAccessMode == ecmRead)
            return eCLI_ERROR_CODE_REQUESTFAIL;
        else
            return eCLI_ERROR_CODE_SETFAIL;
    }

    if(eAccessMode == ecmRead)
    {
        sCmdFormat->lData = (INT32)halWarp_BlacklevelParameterGet(eBKLV_EVENT_CUR_TR_Y);
    }
    else if(eAccessMode == ecmWrite)
    {
        INT32 cValue = sCmdFormat->lData;

        if(cValue >= 0 && cValue < halWarping_VResGet())
        {
            halWarp_BlacklevelParameterSet(eBKLV_EVENT_CUR_TR_Y, cValue);
        }
        else
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }
    }

    return eCLI_ERROR_CODE_NO;
}


eCLI_ERROR_CODE utilGeneral_BKLV_BLCX(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
{
    if(utilWarp_GetWarpCtrl() != WARP_CTRL__ADVANCED)  //G100_Simon_0005
    {
        if(eAccessMode == ecmRead)
            return eCLI_ERROR_CODE_REQUESTFAIL;
        else
            return eCLI_ERROR_CODE_SETFAIL;
    }

    if(eAccessMode == ecmRead)
    {
        sCmdFormat->lData = (INT32)halWarp_BlacklevelParameterGet(eBKLV_EVENT_CUR_BL_X);
    }
    else if(eAccessMode == ecmWrite)
    {
        INT32 cValue = sCmdFormat->lData;

        if(cValue >= 0 && cValue < halWarping_HResGet())
        {
            halWarp_BlacklevelParameterSet(eBKLV_EVENT_CUR_BL_X, cValue);
        }
        else
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }

    }

    return eCLI_ERROR_CODE_NO;
}


eCLI_ERROR_CODE utilGeneral_BKLV_BLCY(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
{
    if(utilWarp_GetWarpCtrl() != WARP_CTRL__ADVANCED)  //G100_Simon_0005
    {
        if(eAccessMode == ecmRead)
            return eCLI_ERROR_CODE_REQUESTFAIL;
        else
            return eCLI_ERROR_CODE_SETFAIL;
    }

    if(eAccessMode == ecmRead)
    {
        sCmdFormat->lData = (INT32)halWarp_BlacklevelParameterGet(eBKLV_EVENT_CUR_BL_Y);
    }
    else if(eAccessMode == ecmWrite)
    {
        INT32 cValue = sCmdFormat->lData;

        if(cValue >= 0 && cValue < halWarping_VResGet())
        {
            halWarp_BlacklevelParameterSet(eBKLV_EVENT_CUR_BL_Y, cValue);
        }
        else
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }
    }

    return eCLI_ERROR_CODE_NO;
}


eCLI_ERROR_CODE utilGeneral_BKLV_BRCX(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
{
    if(utilWarp_GetWarpCtrl() != WARP_CTRL__ADVANCED)  //G100_Simon_0005
    {
        if(eAccessMode == ecmRead)
            return eCLI_ERROR_CODE_REQUESTFAIL;
        else
            return eCLI_ERROR_CODE_SETFAIL;
    }

    if(eAccessMode == ecmRead)
    {
        sCmdFormat->lData = (INT32)halWarp_BlacklevelParameterGet(eBKLV_EVENT_CUR_BR_X);
    }
    else if(eAccessMode == ecmWrite)
    {
        INT32 cValue = sCmdFormat->lData;

        if(cValue >= 0 && cValue < halWarping_HResGet())
        {
            halWarp_BlacklevelParameterSet(eBKLV_EVENT_CUR_BR_X, cValue);
        }
        else
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }
    }

    return eCLI_ERROR_CODE_NO;
}


eCLI_ERROR_CODE utilGeneral_BKLV_BRCY(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
{
    if(utilWarp_GetWarpCtrl() != WARP_CTRL__ADVANCED)  //G100_Simon_0005
    {
        if(eAccessMode == ecmRead)
            return eCLI_ERROR_CODE_REQUESTFAIL;
        else
            return eCLI_ERROR_CODE_SETFAIL;
    }

    if(eAccessMode == ecmRead)
    {
        sCmdFormat->lData = (INT32)halWarp_BlacklevelParameterGet(eBKLV_EVENT_CUR_BR_Y);
    }
    else if(eAccessMode == ecmWrite)
    {
        INT32 cValue = sCmdFormat->lData;

        if(cValue >= 0 && cValue < halWarping_VResGet())
        {
            halWarp_BlacklevelParameterSet(eBKLV_EVENT_CUR_BR_Y, cValue);
        }
        else
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }
    }

    return eCLI_ERROR_CODE_NO;
}


eCLI_ERROR_CODE utilGeneral_BKLV_RLVL(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
{
    if(utilWarp_GetWarpCtrl() != WARP_CTRL__ADVANCED)  //G100_Simon_0005
    {
        if(eAccessMode == ecmRead)
            return eCLI_ERROR_CODE_REQUESTFAIL;
        else
            return eCLI_ERROR_CODE_SETFAIL;
    }

    if(eAccessMode == ecmRead)
    {
        sCmdFormat->lData = (INT32)halWarp_BlacklevelParameterGet(eBKLV_EVENT_PLT_RED);
    }
    else if(eAccessMode == ecmWrite)
    {
        INT32 cValue = sCmdFormat->lData;

        if(cValue >= 0 && cValue <= 255)
        {
            halWarp_BlacklevelParameterSet(eBKLV_EVENT_PLT_RED, cValue);
        }
        else
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }
    }

    return eCLI_ERROR_CODE_NO;
}


eCLI_ERROR_CODE utilGeneral_BKLV_GLVL(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
{
    if(utilWarp_GetWarpCtrl() != WARP_CTRL__ADVANCED)  //G100_Simon_0005
    {
        if(eAccessMode == ecmRead)
            return eCLI_ERROR_CODE_REQUESTFAIL;
        else
            return eCLI_ERROR_CODE_SETFAIL;
    }

    if(eAccessMode == ecmRead)
    {
        sCmdFormat->lData = (INT32)halWarp_BlacklevelParameterGet(eBKLV_EVENT_PLT_GREEN);
    }
    else if(eAccessMode == ecmWrite)
    {
        INT32 cValue = sCmdFormat->lData;

        if(cValue >= 0 && cValue <= 255)
        {
            halWarp_BlacklevelParameterSet(eBKLV_EVENT_PLT_GREEN, cValue);
        }
        else
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }
    }

    return eCLI_ERROR_CODE_NO;
}


eCLI_ERROR_CODE utilGeneral_BKLV_BLVL(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
{
    if(utilWarp_GetWarpCtrl() != WARP_CTRL__ADVANCED)  //G100_Simon_0005
    {
        if(eAccessMode == ecmRead)
            return eCLI_ERROR_CODE_REQUESTFAIL;
        else
            return eCLI_ERROR_CODE_SETFAIL;
    }

    if(eAccessMode == ecmRead)
    {
        sCmdFormat->lData = (INT32)halWarp_BlacklevelParameterGet(eBKLV_EVENT_PLT_BLUE);
    }
    else if(eAccessMode == ecmWrite)
    {
        INT32 cValue = sCmdFormat->lData;

        if(cValue >= 0 && cValue <= 255)
        {
            halWarp_BlacklevelParameterSet(eBKLV_EVENT_PLT_BLUE, cValue);
        }
        else
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }
    }

    return eCLI_ERROR_CODE_NO;
}


eCLI_ERROR_CODE utilGeneral_BKLV_APLY(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
{
    if(utilWarp_GetWarpCtrl() != WARP_CTRL__ADVANCED)  //G100_Simon_0005
    {
        if(eAccessMode == ecmRead)
            return eCLI_ERROR_CODE_REQUESTFAIL;
        else
            return eCLI_ERROR_CODE_SETFAIL;
    }

    if(eAccessMode == ecmRead)
    {
        sCmdFormat->lData = (INT32)halWarp_BlacklevelParameterGet(eBKLV_EVENT_APPLY_ALL_AREA);
    }
    else if(eAccessMode == ecmWrite)
    {
        INT32 cValue = sCmdFormat->lData;

        halWarp_BlacklevelParameterSet(eBKLV_EVENT_APPLY_ALL_AREA, cValue);
    }

    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilGeneral_BKLV_RSET(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
{
    if(eAccessMode == ecmRead)
    {
    }
    else if(eAccessMode == ecmWrite)
    {
        halWarp_BlacklevelParameterSet(eBKLV_EVENT_RESET, TRUE);
    }

    return eCLI_ERROR_CODE_NO;
}


#if 0
eCLI_ERROR_CODE utilGeneral_BKLV_APSI(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
{
    if(eAccessMode == ecmRead)
    {
        sCmdFormat->lData = (INT32)halWarp_BlacklevelParameterGet(eBKLV_EVENT_APPLY);
    }
    else if(eAccessMode == ecmWrite)
    {
        INT32 cValue = sCmdFormat->lData;

        halWarp_BlacklevelParameterSet(eBKLV_EVENT_APPLY, cValue);
    }

    return eCLI_ERROR_CODE_NO;

}
#endif

eCLI_ERROR_CODE utilGeneral_BKLV_DBGG(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
{
    if(eAccessMode == ecmRead)
    {
        halWarping_Blacklevel_Debug();
    }
    else if(eAccessMode == ecmWrite)
    {
    }

    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilGeneral_BKLV_SAVE(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
{
    if(eAccessMode == ecmRead)
    {
    }
    else if(eAccessMode == ecmWrite)
    {
        if(sCmdFormat->lData < 5)
        {
            halWarping_Blacklevel_SaveData(sCmdFormat->lData);
        }
        else
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }
    }

    return eCLI_ERROR_CODE_NO;
}


eCLI_ERROR_CODE utilGeneral_BKLV_LOAD(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
{
    if(eAccessMode == ecmRead)
    {
    }
    else if(eAccessMode == ecmWrite)
    {
        //dvC789_LoadEgbBiasArea(BLACKLEVEL_TMP_FILE /*"ebarea0.bak"*/);

        if(sCmdFormat->lData <= WARPING_APPLY_MAX_VALUE)  //A35G2_Simon_0093
        {
            uPALGEOAPI_INFO uInfo = {.sGFN_CLI_BKLV_LOAD_DATA_INFO.Index = sCmdFormat->lData};
            palGeo_Func_Set(eGFN_CLI_BKLV_LOAD_DATA, &uInfo);
        }
        else
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }
    }

    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilGeneral_GMSG(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)  //G100_Doulas_0027
{
    if(eAccessMode == ecmWrite)
    {
        if(sCmdFormat->lData == 0 || sCmdFormat->lData > 5 )
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE ;
        }

    	UINT8 ucValue = sCmdFormat->lData - 1 ;
		if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(edcWARP_MEMORY_SAVE))		//G100_Doulas_0040 Add
		{
			return eCLI_ERROR_CODE_SETFAIL;
		}
    	utilCommonCLI_DataConversionSet(edcWARP_MEMORY_SAVE, (INT32)ucValue);
    }
    else if(eAccessMode == ecmRead)
    {
        INT32 iValue = 0 ;
		if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(edcWARP_MEMORY_SAVE))		//G100_Doulas_0040 Add
		{
			return eCLI_ERROR_CODE_REQUESTFAIL;
		}
        utilCommonCLI_DataConversionGet(edcWARP_MEMORY_SAVE, &iValue);
        sCmdFormat->lData = iValue + 1 ;
    }

    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilGeneral_GMAG(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)  //G100_Doulas_0027
{
    if(eAccessMode == ecmWrite)
    {
        if(sCmdFormat->lData == 0 || sCmdFormat->lData > 5 )
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE ;
        }

    	UINT8 ucValue = sCmdFormat->lData - 1 ;
		if((palDataMgr_ApplyWarpMemoryItemAvailable(ucValue) == 0) ||
		   (eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(edcWARP_MEMORY_APPLY)))		//G100_Doulas_0040 Add
		{
			return eCLI_ERROR_CODE_SETFAIL;
		}
    	utilCommonCLI_DataConversionSet(edcWARP_MEMORY_APPLY, (INT32)ucValue);
    }
    else if(eAccessMode == ecmRead)
    {
        INT32 iValue = 0 ;
		if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(edcWARP_MEMORY_APPLY))		//G100_Doulas_0040 Add
		{
			return eCLI_ERROR_CODE_REQUESTFAIL;
		}
        utilCommonCLI_DataConversionGet(edcWARP_MEMORY_APPLY, &iValue);
        sCmdFormat->lData = iValue + 1 ;
    }

    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilGeneral_SCW_EVRT(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)  //G100_Coda_0043
{
    if(eAccessMode == ecmWrite)
    {
        UINT32 ucCurrentWeekdayValue = 0;
        UINT8 ucCurrentEventListNum = 0;
        UINT8 ucUserEventListNum = 0;
        UINT32 ucUserChoiceWeekday = 0;
        UINT8 ucIndex = 1;
        sDST_SCHEDULE_EVENT_INFO sCurrentEvent = {0};

        ucUserChoiceWeekday = sCmdFormat->cMainCode[3] - 0x30;

        if(sCmdFormat->lData == 0 || sCmdFormat->lData > 16 || ucUserChoiceWeekday > 6)
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE ;
        }

        //backup system value

        palDataMgr_Data_Access(edcUI_SCHEDULE_EVENT_INFO, edaREAD, &sCurrentEvent);
        ucCurrentEventListNum = sCurrentEvent.ucCurrentEventListNum;

        palDataMgr_Data_Access(edcSCHEDULE_VIEW_WEEKDAY, edaREAD, &ucCurrentWeekdayValue); //GuiCb.fpGui_DataCode_Value_GetCb(edcSCHEDULE_VIEW_WEEKDAY, &ucCurrentWeekdayValue);

        //set event reset
        ucUserEventListNum = sCmdFormat->lData - 1 ;
        sCurrentEvent.ucCurrentEventListNum = ucUserEventListNum;
        LOG_MSG(db_APP_SCHEDULE, "user event reset %d (%s,%d)\r\n", ucUserEventListNum, sCmdFormat->cMainCode, ucUserChoiceWeekday);
        palDataMgr_Data_Access(edcUI_SCHEDULE_EVENT_INFO, edaWRITE_RAM_ONLY_WITH_ACTION, &sCurrentEvent);
        palDataMgr_Data_Access(edcSCHEDULE_VIEW_WEEKDAY, edaWRITE_THROUGH_WITH_ACTION, &ucUserChoiceWeekday); //GuiCb.fpGui_DataCode_Value_SetCb(edcSCHEDULE_VIEW_WEEKDAY, edaWRITE_THROUGH_WITH_ACTION, ucUserChoiceWeekday);
        palDataMgr_Data_Access(edcSCHEDULE_EVENT_RESET_FUCNTION, edaWRITE_THROUGH_WITH_ACTION, &ucIndex); //GuiCb.fpGui_DataCode_Value_SetCb(edcSCHEDULE_EVENT_RESET_FUCNTION, edaWRITE_THROUGH_WITH_ACTION, 1);

        //restore system value

        sCurrentEvent.ucCurrentEventListNum = ucCurrentEventListNum;
        palDataMgr_Data_Access(edcUI_SCHEDULE_EVENT_INFO, edaWRITE_RAM_ONLY_WITH_ACTION, &sCurrentEvent);

        palDataMgr_Data_Access(edcSCHEDULE_VIEW_WEEKDAY, edaWRITE_THROUGH_WITH_ACTION, &ucCurrentWeekdayValue); //GuiCb.fpGui_DataCode_Value_SetCb(edcSCHEDULE_VIEW_WEEKDAY, edaWRITE_THROUGH_WITH_ACTION, ucCurrentWeekdayValue);
    }
    else if(eAccessMode == ecmRead)
    {
        //INT32 iValue = 0 ;
        //utilCommonCLI_DataConversionGet(edcWARP_MEMORY_APPLY, &iValue);
        //sCmdFormat->lData = iValue + 1 ;
    }

    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilGeneral_SCW_REST(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)  //G100_Coda_0042
{
    if(eAccessMode == ecmWrite)
    {
        UINT8 ucResetWeekdayValue = 0 ;
    	UINT32 ucCurrentWeekdayValue = 0;
        UINT8 ucIndex = 1;

        ucResetWeekdayValue = sCmdFormat->cMainCode[3] - 0x30;
        LOG_MSG(db_APP_SCHEDULE, "user wday reset (%s,%d)\r\n", sCmdFormat->cMainCode, ucResetWeekdayValue);

        if(sCmdFormat->lData == 0 || sCmdFormat->lData > 1 || ucResetWeekdayValue > 6)
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE ;
        }

        palDataMgr_Data_Access(edcSCHEDULE_VIEW_WEEKDAY, edaREAD, &ucCurrentWeekdayValue); //GuiCb.fpGui_DataCode_Value_GetCb(edcSCHEDULE_VIEW_WEEKDAY, &ucCurrentWeekdayValue);
        palDataMgr_Data_Access(edcSCHEDULE_VIEW_WEEKDAY, edaWRITE_THROUGH_WITH_ACTION, &ucResetWeekdayValue); //GuiCb.fpGui_DataCode_Value_SetCb(edcSCHEDULE_VIEW_WEEKDAY, edaWRITE_THROUGH_WITH_ACTION, ucResetWeekdayValue);
        palDataMgr_Data_Access(edcSCHEDULE_EVENT_RESET_DAY, edaWRITE_THROUGH_WITH_ACTION, &ucIndex); //GuiCb.fpGui_DataCode_Value_SetCb(edcSCHEDULE_EVENT_RESET_DAY, edaWRITE_THROUGH_WITH_ACTION, 1);
        palDataMgr_Data_Access(edcSCHEDULE_VIEW_WEEKDAY, edaWRITE_THROUGH_WITH_ACTION, &ucCurrentWeekdayValue); //GuiCb.fpGui_DataCode_Value_SetCb(edcSCHEDULE_VIEW_WEEKDAY, edaWRITE_THROUGH_WITH_ACTION, ucCurrentWeekdayValue);

    }
    else if(eAccessMode == ecmRead)
    {
        //INT32 iValue = 0 ;
        //utilCommonCLI_DataConversionGet(edcWARP_MEMORY_APPLY, &iValue);
        //sCmdFormat->lData = iValue + 1 ;
    }

    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilGeneral_SCW_CPWD(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)  //G100_Coda_0042
{
    if(eAccessMode == ecmWrite)
    {
        UINT32 ucCurrentWeekday = 0;
        UINT8 ucCopyWeekday = 0;
        UINT32 ucUserChoiceWeekday = 0;
        sDST_SCHEDULE_EVENT_INFO sCurrentEvent = {0};

        ucUserChoiceWeekday = sCmdFormat->cMainCode[3] - 0x30;
        ucCopyWeekday = sCmdFormat->lData;

        if(ucCopyWeekday > 6|| ucUserChoiceWeekday > 6 || (ucCopyWeekday == ucUserChoiceWeekday))
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE ;
        }

        //backup system value
        palDataMgr_Data_Access(edcSCHEDULE_VIEW_WEEKDAY, edaREAD, &ucCurrentWeekday); //GuiCb.fpGui_DataCode_Value_GetCb(edcSCHEDULE_VIEW_WEEKDAY, &ucCurrentWeekday);
        palDataMgr_Data_Access(edcUI_SCHEDULE_EVENT_INFO, edaREAD, &sCurrentEvent);

        //set copy event

        LOG_MSG(db_APP_SCHEDULE, "user Copy wday from %d (%s) to %d\r\n", ucUserChoiceWeekday, sCmdFormat->cMainCode, ucCopyWeekday);

        palDataMgr_Data_Access(edcSCHEDULE_VIEW_WEEKDAY, edaWRITE_THROUGH_WITH_ACTION, &ucUserChoiceWeekday); //GuiCb.fpGui_DataCode_Value_SetCb(edcSCHEDULE_VIEW_WEEKDAY, edaWRITE_THROUGH_WITH_ACTION, ucUserChoiceWeekday);

        sCurrentEvent.ucCurrentCopyWeekdayNum = ucCopyWeekday;

        palDataMgr_Data_Access(edcUI_SCHEDULE_EVENT_INFO, edaWRITE_RAM_ONLY_WITH_ACTION, &sCurrentEvent);

        palDataMgr_Data_Access(edcSCHEDULE_COPY_EVENT_INDEX, edaWRITE_THROUGH_WITH_ACTION, &ucCopyWeekday); //GuiCb.fpGui_DataCode_Value_SetCb(edcSCHEDULE_COPY_EVENT_INDEX, edaWRITE_THROUGH_WITH_ACTION, ucCopyWeekday); //A35G2_BRC_Casper_0017

        //restore system value
        palDataMgr_Data_Access(edcSCHEDULE_VIEW_WEEKDAY, edaWRITE_THROUGH_WITH_ACTION, &ucCurrentWeekday); //GuiCb.fpGui_DataCode_Value_SetCb(edcSCHEDULE_VIEW_WEEKDAY, edaWRITE_THROUGH_WITH_ACTION, ucCurrentWeekday);
    }
    else if(eAccessMode == ecmRead)
    {

    }

    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilGeneral_SCW_EVWR(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)  //G100_Coda_0043
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

        ucUserChoiceWeekday = sCmdFormat->cMainCode[3] - 0x30;

        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_STRING)
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE ;
        }

        if((len < 6) || (len > 10))	//A35G2_CDS_Coda_0014
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }

        memcpy(ucUserEventStr, sCmdFormat->cTextString, len);	//A35G2_CDS_Coda_0014

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
            if(sSchedueleEventIndexTranslateOsdTextIdLUT[ucUserFunction][ucIndex].ucEventIndex == 99) //G100_Coda_0087 //A35G2_BRC_Casper_0013
            {   //data not found
                return eCLI_ERROR_CODE_DATAOVERRANGE;
            }
            else if(sSchedueleEventIndexTranslateOsdTextIdLUT[ucUserFunction][ucIndex].ucEventIndex == ucUserEvent)
            {   //data exist
                break;
            }
        }

    //-------------------
        //backup system value
        palDataMgr_Data_Access(edcUI_SCHEDULE_EVENT_INFO, edaREAD, &ucBackupCurrentEventListNum);
        ucBackupCurrentEventListNum = sCurrentEvent.ucCurrentEventListNum;

        palDataMgr_Data_Access(edcSCHEDULE_VIEW_WEEKDAY, edaREAD, &ucBackupCurrentWeekdayValue); //GuiCb.fpGui_DataCode_Value_GetCb(edcSCHEDULE_VIEW_WEEKDAY, &ucBackupCurrentWeekdayValue);

        //set event reset

        LOG_MSG(db_APP_SCHEDULE, "-->user wday %d event %d set %d/%d (%s,%d)\r\n", ucUserChoiceWeekday, ucUserEventListNum, ucUserFunction, ucUserEvent, sCmdFormat->cTextString, strlen((char *)sCmdFormat->cTextString));

        palDataMgr_Data_Access(edcSCHEDULE_VIEW_WEEKDAY, edaWRITE_THROUGH_WITH_ACTION, &ucUserChoiceWeekday); //GuiCb.fpGui_DataCode_Value_SetCb(edcSCHEDULE_VIEW_WEEKDAY, edaWRITE_THROUGH_WITH_ACTION, ucUserChoiceWeekday); //set weekday

        sCurrentEvent.ucCurrentEventListNum = ucUserEventListNum;
        palDataMgr_Data_Access(edcUI_SCHEDULE_EVENT_INFO, edaWRITE_RAM_ONLY_WITH_ACTION, &sCurrentEvent);//set event list
        palDataMgr_Data_Access(edcSCHEDULE_EVENT_TIME, edaWRITE_THROUGH_WITH_ACTION, &ucUserTimeMinute); //GuiCb.fpGui_DataCode_Value_SetCb(edcSCHEDULE_EVENT_TIME, edaWRITE_THROUGH_WITH_ACTION, ucUserTimeMinute); //set time
        palDataMgr_Data_Access(edcSCHEDULE_EVENT_TYPE, edaWRITE_THROUGH_WITH_ACTION, &ucUserFunction); //GuiCb.fpGui_DataCode_Value_SetCb(edcSCHEDULE_EVENT_TYPE, edaWRITE_THROUGH_WITH_ACTION, ucUserFunction);
        palDataMgr_Data_Access(edcSCHEDULE_EVENT_TYPE_INDEX, edaWRITE_THROUGH_WITH_ACTION, &ucUserEvent); //GuiCb.fpGui_DataCode_Value_SetCb(edcSCHEDULE_EVENT_TYPE_INDEX, edaWRITE_THROUGH_WITH_ACTION, ucUserEvent);
        MS_SLEEP(1);
        palDataMgr_Schedule_Sort_Event_List_By_ExecuteTime();
        palLANProcScheduleSend();

        //restore system value
        sCurrentEvent.ucCurrentEventListNum = ucBackupCurrentEventListNum;
        palDataMgr_Data_Access(edcUI_SCHEDULE_EVENT_INFO, edaWRITE_RAM_ONLY_WITH_ACTION, &sCurrentEvent);
        palDataMgr_Data_Access(edcSCHEDULE_VIEW_WEEKDAY, edaWRITE_THROUGH_WITH_ACTION, &ucBackupCurrentWeekdayValue); //GuiCb.fpGui_DataCode_Value_SetCb(edcSCHEDULE_VIEW_WEEKDAY, edaWRITE_THROUGH_WITH_ACTION, ucBackupCurrentWeekdayValue);
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
eCLI_ERROR_CODE utilGeneral_SCW_EVRD(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)  //G100_Coda_0043
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

        ucUserChoiceWeekday = sCmdFormat->cMainCode[3] - 0x30;

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

eCLI_ERROR_CODE utilGeneral_SCHE_ShowToday(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)  //G100_Coda_0043 //G100_Coda_0049
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

        palDataMgr_Data_Access(edcSCHEDULE_VIEW_TODAY, edaREAD, &ucCurrentWeekdayValue); //GuiCb.fpGui_DataCode_Value_GetCb(edcSCHEDULE_VIEW_TODAY, &ucCurrentWeekdayValue);

        if(ucCurrentWeekdayValue > 6)
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }

        sprintf(ucOutputString, "%s" , WeekdayStr[ucCurrentWeekdayValue]);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
    }

    return eCLI_ERROR_CODE_NO;
}


eCLI_ERROR_CODE utilGeneral_PIP_Layout(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat) //G100_Larry_0007
{
    if(eAccessMode == ecmRead)
    {
        INT32 iVlaue = 0;

        utilCommonCLI_DataConversionGet(edcMAIN_LAYOUT, (INT32*)&iVlaue);

        sCmdFormat->lData = iVlaue;
    }
    else if(eAccessMode == ecmWrite)
    {
		INT32 uiVlaue = sCmdFormat->lData;
        INT32 uiPipEnable = 0;


		utilCommonCLI_DataConversionGet(edcPIP_PBP_ENABLE, (INT32*)&uiPipEnable);

        uiPipEnable = (UINT8)CM2CLI(edcPIP_PBP_ENABLE, uiPipEnable);	//G100_Clare_0074

        if(uiPipEnable == eCM_SCREEN_MODE_PBP)
        {
            if(uiVlaue > eCM_MAIN_LAYOUT_PBP_MAIN_BOTTOM)
            {
                return eCLI_ERROR_CODE_DATAOVERRANGE;
            }
        }
        else if(uiPipEnable == eCM_SCREEN_MODE_PIP)
        {
            if(uiVlaue < eCM_MAIN_LAYOUT_PIP_BOTTOM_RIGHT)
            {
                return eCLI_ERROR_CODE_DATAOVERRANGE;
            }
        }

        utilCommonCLI_DataConversionSet(edcMAIN_LAYOUT, uiVlaue);

	}

    return eCLI_ERROR_CODE_NO;
}


void TestBlacklevel0(void)
{
    LOG_MSG(db_HAL_WARPING, "-- Start --\r\n");

    sCLI_GENERAL_FORMAT sData;

    sData.lData = 0;
    utilGeneral_BKLV_ASEL(ecmWrite, &sData);

    sData.lData = 1;
    utilGeneral_BKLV_ENAB(ecmWrite, &sData);

    // TL
    sData.lData = 400;
    utilGeneral_BKLV_TLCX(ecmWrite, &sData);

    sData.lData = 400;
    utilGeneral_BKLV_TLCY(ecmWrite, &sData);

    // TR
    sData.lData = 1350;
    utilGeneral_BKLV_TRCX(ecmWrite, &sData);

    sData.lData = 200;
    utilGeneral_BKLV_TRCY(ecmWrite, &sData);

    // BL
    sData.lData = 300;
    utilGeneral_BKLV_BLCX(ecmWrite, &sData);

    sData.lData = 1000;
    utilGeneral_BKLV_BLCY(ecmWrite, &sData);

    // BR
    sData.lData = 1919;
    utilGeneral_BKLV_BRCX(ecmWrite, &sData);

    sData.lData = 1199;
    utilGeneral_BKLV_BRCY(ecmWrite, &sData);

    // R level
    sData.lData = 220;
    utilGeneral_BKLV_RLVL(ecmWrite, &sData);
    // G level
    sData.lData = 50;
    utilGeneral_BKLV_GLVL(ecmWrite, &sData);
    // B level
    sData.lData = 60;
    utilGeneral_BKLV_BLVL(ecmWrite, &sData);



    utilGeneral_BKLV_DBGG(ecmRead, NULL);

    sData.lData = 1;
    //utilGeneral_BKLV_APLY(ecmWrite, &sData);

    LOG_MSG(db_HAL_WARPING, "-- End --\r\n");

}


void TestBlacklevel1(void)
{
    LOG_MSG(db_HAL_WARPING, "-- Start --\r\n");

    sCLI_GENERAL_FORMAT sData;

    sData.lData = 1;
    utilGeneral_BKLV_ASEL(ecmWrite, &sData);

    sData.lData = 1;
    utilGeneral_BKLV_ENAB(ecmWrite, &sData);

    // TL
    sData.lData = 600;
    utilGeneral_BKLV_TLCX(ecmWrite, &sData);

    sData.lData = 600;
    utilGeneral_BKLV_TLCY(ecmWrite, &sData);

    // TR
    sData.lData = 1200;
    utilGeneral_BKLV_TRCX(ecmWrite, &sData);

    sData.lData = 600;
    utilGeneral_BKLV_TRCY(ecmWrite, &sData);

    // BL
    sData.lData = 600;
    utilGeneral_BKLV_BLCX(ecmWrite, &sData);

    sData.lData = 1000;
    utilGeneral_BKLV_BLCY(ecmWrite, &sData);

    // BR
    sData.lData = 1200;
    utilGeneral_BKLV_BRCX(ecmWrite, &sData);

    sData.lData = 1000;
    utilGeneral_BKLV_BRCY(ecmWrite, &sData);

    // R level
    sData.lData = 255;
    utilGeneral_BKLV_RLVL(ecmWrite, &sData);
    // G level
    sData.lData = 255;
    utilGeneral_BKLV_GLVL(ecmWrite, &sData);
    // B level
    sData.lData = 255;
    utilGeneral_BKLV_BLVL(ecmWrite, &sData);



    utilGeneral_BKLV_DBGG(ecmRead, NULL);

    sData.lData = 1;
    //utilGeneral_BKLV_APLY(ecmWrite, &sData);

    LOG_MSG(db_HAL_WARPING, "-- End --\r\n");

}


void TestBlacklevel3(void)
{
    LOG_MSG(db_HAL_WARPING, "-- Start --\r\n");

    sCLI_GENERAL_FORMAT sData;

    sData.lData = 3;
    utilGeneral_BKLV_ASEL(ecmWrite, &sData);

    sData.lData = 1;
    utilGeneral_BKLV_ENAB(ecmWrite, &sData);

    // TL
    sData.lData = 100;
    utilGeneral_BKLV_TLCX(ecmWrite, &sData);

    sData.lData = 200;
    utilGeneral_BKLV_TLCY(ecmWrite, &sData);

    // TR
    sData.lData = 1450;
    utilGeneral_BKLV_TRCX(ecmWrite, &sData);

    sData.lData = 200;
    utilGeneral_BKLV_TRCY(ecmWrite, &sData);

    // BL
    sData.lData = 300;
    utilGeneral_BKLV_BLCX(ecmWrite, &sData);

    sData.lData = 800;
    utilGeneral_BKLV_BLCY(ecmWrite, &sData);

    // BR
    sData.lData = 1919;
    utilGeneral_BKLV_BRCX(ecmWrite, &sData);

    sData.lData = 1199;
    utilGeneral_BKLV_BRCY(ecmWrite, &sData);

    // R level
    sData.lData = 0;
    utilGeneral_BKLV_RLVL(ecmWrite, &sData);
    // G level
    sData.lData = 200;
    utilGeneral_BKLV_GLVL(ecmWrite, &sData);
    // B level
    sData.lData = 200;
    utilGeneral_BKLV_BLVL(ecmWrite, &sData);



    utilGeneral_BKLV_DBGG(ecmRead, NULL);

    sData.lData = 1;
    //utilGeneral_BKLV_APLY(ecmWrite, &sData);

    LOG_MSG(db_HAL_WARPING, "-- End --\r\n");

}


void TestBlacklevel5(void)
{
    LOG_MSG(db_HAL_WARPING, "-- Start --\r\n");

    sCLI_GENERAL_FORMAT sData;

    sData.lData = 5;
    utilGeneral_BKLV_ASEL(ecmWrite, &sData);

    sData.lData = 1;
    utilGeneral_BKLV_ENAB(ecmWrite, &sData);

    // TL
    sData.lData = 500;
    utilGeneral_BKLV_TLCX(ecmWrite, &sData);

    sData.lData = 100;
    utilGeneral_BKLV_TLCY(ecmWrite, &sData);

    // TR
    sData.lData = 1919;
    utilGeneral_BKLV_TRCX(ecmWrite, &sData);

    sData.lData = 50;
    utilGeneral_BKLV_TRCY(ecmWrite, &sData);

    // BL
    sData.lData = 600;
    utilGeneral_BKLV_BLCX(ecmWrite, &sData);

    sData.lData = 800;
    utilGeneral_BKLV_BLCY(ecmWrite, &sData);

    // BR
    sData.lData = 1919;
    utilGeneral_BKLV_BRCX(ecmWrite, &sData);

    sData.lData = 1079;
    utilGeneral_BKLV_BRCY(ecmWrite, &sData);

    // R level
    sData.lData = 250;
    utilGeneral_BKLV_RLVL(ecmWrite, &sData);
    // G level
    sData.lData = 100;
    utilGeneral_BKLV_GLVL(ecmWrite, &sData);
    // B level
    sData.lData = 175;
    utilGeneral_BKLV_BLVL(ecmWrite, &sData);



    utilGeneral_BKLV_DBGG(ecmRead, NULL);

    sData.lData = 1;
    //utilGeneral_BKLV_APLY(ecmWrite, &sData);

    LOG_MSG(db_HAL_WARPING, "-- End --\r\n");

}


void TestBlacklevel7(void)
{
    LOG_MSG(db_HAL_WARPING, "-- Start --\r\n");

    sCLI_GENERAL_FORMAT sData;

    sData.lData = 7;
    utilGeneral_BKLV_ASEL(ecmWrite, &sData);

    sData.lData = 1;
    utilGeneral_BKLV_ENAB(ecmWrite, &sData);

    // TL
    sData.lData = 700;
    utilGeneral_BKLV_TLCX(ecmWrite, &sData);

    sData.lData = 500;
    utilGeneral_BKLV_TLCY(ecmWrite, &sData);

    // TR
    sData.lData = 1200;
    utilGeneral_BKLV_TRCX(ecmWrite, &sData);

    sData.lData = 400;
    utilGeneral_BKLV_TRCY(ecmWrite, &sData);

    // BL
    sData.lData = 800;
    utilGeneral_BKLV_BLCX(ecmWrite, &sData);

    sData.lData = 700;
    utilGeneral_BKLV_BLCY(ecmWrite, &sData);

    // BR
    sData.lData = 1600;
    utilGeneral_BKLV_BRCX(ecmWrite, &sData);

    sData.lData = 1000;
    utilGeneral_BKLV_BRCY(ecmWrite, &sData);

    // R level
    sData.lData = 50;
    utilGeneral_BKLV_RLVL(ecmWrite, &sData);
    // G level
    sData.lData = 200;
    utilGeneral_BKLV_GLVL(ecmWrite, &sData);
    // B level
    sData.lData = 50;
    utilGeneral_BKLV_BLVL(ecmWrite, &sData);



    utilGeneral_BKLV_DBGG(ecmRead, NULL);

    sData.lData = 1;
    //utilGeneral_BKLV_APLY(ecmWrite, &sData);

    LOG_MSG(db_HAL_WARPING, "-- End --\r\n");

}



void TestBlacklevel14(void)
{
    LOG_MSG(db_HAL_WARPING, "-- Start --\r\n");

    sCLI_GENERAL_FORMAT sData;

    sData.lData = 14;
    utilGeneral_BKLV_ASEL(ecmWrite, &sData);

    sData.lData = 1;
    utilGeneral_BKLV_ENAB(ecmWrite, &sData);

    // TL
    sData.lData = 0;
    utilGeneral_BKLV_TLCX(ecmWrite, &sData);

    sData.lData = 0;
    utilGeneral_BKLV_TLCY(ecmWrite, &sData);

    // TR
    sData.lData = 200;
    utilGeneral_BKLV_TRCX(ecmWrite, &sData);

    sData.lData = 0;
    utilGeneral_BKLV_TRCY(ecmWrite, &sData);

    // BL
    sData.lData = 0;
    utilGeneral_BKLV_BLCX(ecmWrite, &sData);

    sData.lData = 1199;
    utilGeneral_BKLV_BLCY(ecmWrite, &sData);

    // BR
    sData.lData = 200;
    utilGeneral_BKLV_BRCX(ecmWrite, &sData);

    sData.lData = 1199;
    utilGeneral_BKLV_BRCY(ecmWrite, &sData);

    // R level
    sData.lData = 200;
    utilGeneral_BKLV_RLVL(ecmWrite, &sData);
    // G level
    sData.lData = 200;
    utilGeneral_BKLV_GLVL(ecmWrite, &sData);
    // B level
    sData.lData = 0;
    utilGeneral_BKLV_BLVL(ecmWrite, &sData);



    utilGeneral_BKLV_DBGG(ecmRead, NULL);

    sData.lData = 1;
    //utilGeneral_BKLV_APLY(ecmWrite, &sData);

    LOG_MSG(db_HAL_WARPING, "-- End --\r\n");

}


eCLI_ERROR_CODE utilGeneral_BKLV_TEST(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
{
    if(eAccessMode == ecmRead)
    {
        //dvC789_LoadEgbBiasArea("ebarea0.bak");
    }
    else if(eAccessMode == ecmWrite)
    {
        switch(sCmdFormat->lData)
        {
            case 0:
                TestBlacklevel0();
                break;

            case 1:
                TestBlacklevel1();
                break;

            case 3:
                TestBlacklevel3();
                break;

            case 5:
                TestBlacklevel5();
                break;

            case 7:
                TestBlacklevel7();
                break;

            case 14:
                TestBlacklevel14();
                break;

            case 20:
                halWarping_DrawCheckboard(1, 20);
                break;

            case 21:
                halWarping_DrawCheckboard(1, 80);
                break;

            case 30:
                halWarping_BlackLevel_Enable(0);
                break;
            case 31:
                halWarping_BlackLevel_Enable(1);
                break;

            case 80:
                DrawPNG_Test();
                break;

            case 99:
                //halWarpOSD_ClearOSD();
                break;
        }
    }

    return eCLI_ERROR_CODE_NO;
}


eCLI_ERROR_CODE utilGeneral_BKLV_OSDM(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
{
//H2 wait review
#ifdef SCALER_C821_C789
    #include "dvC789.h"
    if(eAccessMode == ecmRead)
    {
        //dvC789_LoadEgbBiasArea("ebarea0.bak");
    }
    else if(eAccessMode == ecmWrite)
    {

        dvC789_Write( B1_OSDMODE, sCmdFormat->lData );

        switch(sCmdFormat->lData)
        {
        }
    }
#endif

    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilGeneral_WRPG_DBGG(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
{
    if(eAccessMode == ecmRead)
    {
        halWarping_OSD_PalettePrint();
    }
    else if(eAccessMode == ecmWrite)
    {
        switch(sCmdFormat->lData)
        {
            case 0:
                palGeo_DrawFrameMemoryTest(0,0,3840,1,0,255,255);
                break;

            case 1:
                palGeo_OSD_On(eWIL_AFTER_WARP);
                break;

            case 2:
                DrawPNG_Test();
                break;

            case 3:
                halWarping_InitTestPalette();

            case 6:
                palGeo_DrawFrameMemoryTest(1,0,1,2400,0,255,255);
                break;

            case 4:
                palGeo_FreezeImage(0);
                break;
            case 5:
                palGeo_FreezeImage(1);
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

            case 100:
                HDR_Demo_SplitScreenMode_Set(0);
                break;

            case 101:
                HDR_Demo_SplitScreenMode_Set(1);
                break;

        }

    }

    return eCLI_ERROR_CODE_NO;
}


eCLI_ERROR_CODE utilGeneral_WCOL(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)  //G100_Doulas_0067
{ //A35G2_BRC_Casper_0046
    UINT8 ucData = 0;

    palDataMgr_Data_Access(edcADV_WARP_CONTROL, edaREAD, &ucData);
    if(ucData != WARP_CTRL__ADVANCED)
    {
        return eCLI_ERROR_CODE_REQUESTFAIL;
    }

    if(eAccessMode == ecmWrite)
    {
        if(halAdvWarpGP_SetColIndex((UINT8)sCmdFormat->lData) != eHAL_WARPING_EXEC_CODE_PASS)
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    else if(eAccessMode == ecmRead)
    {
        sCmdFormat->lData = (INT32)utilWarp_GridPoint_GetColIndex();
    }

    return eCLI_ERROR_CODE_NO;
}


eCLI_ERROR_CODE utilGeneral_WROW(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)  //G100_Doulas_0067
{ //A35G2_BRC_Casper_0046
    UINT8 ucData = 0;

    palDataMgr_Data_Access(edcADV_WARP_CONTROL, edaREAD, &ucData);
    if(ucData != WARP_CTRL__ADVANCED)
    {
        return eCLI_ERROR_CODE_REQUESTFAIL;
    }

    if(eAccessMode == ecmWrite)
    {
        if(halAdvWarpGP_SetRowIndex((UINT8)sCmdFormat->lData) != eHAL_WARPING_EXEC_CODE_PASS)
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    else if(eAccessMode == ecmRead)
    {
        sCmdFormat->lData = (INT32)utilWarp_GridPoint_GetRowIndex();
    }

    return eCLI_ERROR_CODE_NO;
}


 //G100_Steven_0048 start
eCLI_ERROR_CODE utilGeneral_SEQM(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)  //G100_Doulas_0067
{
    if(eAccessMode == ecmWrite)
    {
        return eCLI_ERROR_CODE_COMMAND_READ_ONLY;
    }
    else if(eAccessMode == ecmRead)
    {
        UINT8 ucData = 0;
        UINT8 ucSystemMode = 0;

        halFormatter_SystemModeGet(&ucSystemMode);
        palDataMgr_Data_Access(edcFACTORY_DDP_SYSTEMMODE, edaWRITE_RAM_ONLY_NO_ACTION, &ucSystemMode);
		palDataMgr_Data_Access(edcFACTORY_DDP_SYSTEMMODE, edaREAD, &ucData);

		if(ucData < DDP_SYSTEM_MODE_MIN_VALUE || ucData > DDP_SYSTEM_MODE_MAX_VALUE)
		{
			return eCLI_ERROR_CODE_REQUESTFAIL;
		}
		else
		{
			sCmdFormat->lData = (INT32)ucData;
		}
    }

    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilGeneral_BRPM(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)  //G100_Doulas_0067
{
    if(eAccessMode == ecmWrite)
    {
        return eCLI_ERROR_CODE_COMMAND_READ_ONLY;
    }
    else if(eAccessMode == ecmRead)
    {
    	UINT16 uiData = 0;

        halFormatter_PWM_Update();
        uiData = palFormatterMgr_PWM_Get(eLD_SEG_BLD_R);

		if(uiData < DDP_PWM_MIN_VALUE || uiData > DDP_PWM_MAX_VALUE)
		{
			return eCLI_ERROR_CODE_REQUESTFAIL;
		}
		else
		{
			sCmdFormat->lData = (INT32)uiData;
		}
    }

    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilGeneral_BGPM(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)  //G100_Doulas_0067
{
    if(eAccessMode == ecmWrite)
    {
        return eCLI_ERROR_CODE_COMMAND_READ_ONLY;
    }
    else if(eAccessMode == ecmRead)
    {
    	UINT16 uiData = 0;

        halFormatter_PWM_Update();
        uiData = palFormatterMgr_PWM_Get(eLD_SEG_BLD_G);

		if(uiData < DDP_PWM_MIN_VALUE || uiData > DDP_PWM_MAX_VALUE)
		{
			return eCLI_ERROR_CODE_REQUESTFAIL;
		}
		else
		{
			sCmdFormat->lData = (INT32)uiData;
		}
    }

    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilGeneral_BBPM(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)  //G100_Doulas_0067
{
    if(eAccessMode == ecmWrite)
    {
        return eCLI_ERROR_CODE_COMMAND_READ_ONLY;
    }
    else if(eAccessMode == ecmRead)
    {
    	UINT16 uiData = 0;

        halFormatter_PWM_Update();
        uiData = palFormatterMgr_PWM_Get(eLD_SEG_BLD_B);

		if(uiData < DDP_PWM_MIN_VALUE || uiData > DDP_PWM_MAX_VALUE)
		{
			return eCLI_ERROR_CODE_REQUESTFAIL;
		}
		else
		{
			sCmdFormat->lData = (INT32)uiData;
		}
    }

    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilGeneral_BYPM(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)  //G100_Doulas_0067
{
    if(eAccessMode == ecmWrite)
    {
        return eCLI_ERROR_CODE_COMMAND_READ_ONLY;
    }
    else if(eAccessMode == ecmRead)
    {
    	UINT16 uiData = 0;

        halFormatter_PWM_Update();
        uiData = palFormatterMgr_PWM_Get(eLD_SEG_BLD_Y);

		if(uiData < DDP_PWM_MIN_VALUE || uiData > DDP_PWM_MAX_VALUE)
		{
			return eCLI_ERROR_CODE_REQUESTFAIL;
		}
		else
		{
			sCmdFormat->lData = (INT32)uiData;
		}
    }

    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilGeneral_RRPM(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)  //G100_Doulas_0067
{
    if(eAccessMode == ecmWrite)
    {
        return eCLI_ERROR_CODE_COMMAND_READ_ONLY;
    }
    else if(eAccessMode == ecmRead)
    {
    	UINT16 uiData = 0;

        halFormatter_PWM_Update();
        uiData = palFormatterMgr_PWM_Get(eLD_SEG_RLD_R);

		if(uiData < DDP_PWM_MIN_VALUE || uiData > DDP_PWM_MAX_VALUE)
		{
			return eCLI_ERROR_CODE_REQUESTFAIL;
		}
		else
		{
			sCmdFormat->lData = (INT32)uiData;
		}
    }

    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilGeneral_RYPM(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)  //G100_Doulas_0067
{
    if(eAccessMode == ecmWrite)
    {
        return eCLI_ERROR_CODE_COMMAND_READ_ONLY;
    }
    else if(eAccessMode == ecmRead)
    {
    	UINT16 uiData = 0;

        halFormatter_PWM_Update();
        uiData = palFormatterMgr_PWM_Get(eLD_SEG_RLD_Y);

		if(uiData < DDP_PWM_MIN_VALUE || uiData > DDP_PWM_MAX_VALUE)
		{
			return eCLI_ERROR_CODE_REQUESTFAIL;
		}
		else
		{
			sCmdFormat->lData = (INT32)uiData;
		}
    }

    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilGeneral_LEDT(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)  //G100_Doulas_0067
{
    if(eAccessMode == ecmWrite)
    {
        return eCLI_ERROR_CODE_COMMAND_READ_ONLY;
    }
    else if(eAccessMode == ecmRead)
    {
    	UINT8 ucData = 0;

    	palDataMgr_Data_Access(edcLENS_DETECTION, edaREAD, &ucData);

		if(ucData < 0 || ucData > 1)
		{
			return eCLI_ERROR_CODE_REQUESTFAIL;
		}
		else
		{
			sCmdFormat->lData = (INT32)ucData;
		}
    }

    return eCLI_ERROR_CODE_NO;
}
 //G100_Steven_0048 end
eCLI_ERROR_CODE utilGeneral_DPAU(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)		//G100_Doulas_0068
{
    if(eAccessMode == ecmWrite)
    {
		if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(edcUSER_COLOR_MODE))
		{
			return eCLI_ERROR_CODE_SETFAIL;
		}

        if((UINT32)sCmdFormat->lData > CLI_PRE_USER_2D_HIGH_SPEED)
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }

		//Check 2D hight speed/ 3D
		if(palDataMgr_IS_3D_Enable())
		{
			if(sCmdFormat->lData != CLI_PRE_USER_3D)
				return eCLI_ERROR_CODE_SETFAIL;
		}
		else if(halScaler_Input_3D_Format_Get() == eINPUT_3D_TYPE_FRAME_SEQUENTIAL)  //120hz
		{
			if(sCmdFormat->lData != CLI_PRE_USER_2D_HIGH_SPEED)
			   return eCLI_ERROR_CODE_SETFAIL;
		}
		else
		{
			if((sCmdFormat->lData == CLI_PRE_USER_2D_HIGH_SPEED) ||
			   (sCmdFormat->lData == CLI_PRE_USER_3D))
			   return eCLI_ERROR_CODE_SETFAIL;
		}

        INT32 iHDR_Auto = ets_OFF;

        utilCommonCLI_DataConversionGet(edcHDR_AUTOENABLE, &iHDR_Auto);
		//Check HDR
        if((iHDR_Auto == ets_ON) && (palImgMgr_HDR_Get(eSOURCE_WINDOW_MAIN) == eHDR_SETTING_HDR))
        {
        	if(sCmdFormat->lData != CLI_PRE_USER_HDR)
        	{
            	return eCLI_ERROR_CODE_SETFAIL;
        	}
        }
        else
        {
        	if(sCmdFormat->lData == CLI_PRE_USER_HDR)
			{
            	return eCLI_ERROR_CODE_SETFAIL;
        	}
        }

        #if 0
 		//G100_Clare_0076, add, >>>
        if((UINT32)sCmdFormat->lData == CLI_PICTURE_SETTINGS_SUPER_BRIGHT
        && ((palDataMgr_Model_ID_Get() == MODULE_TYPE_ID0_PLATFORM)
        || (palDataMgr_Model_ID_Get() == MODULE_TYPE_ID1_PLATFORM)
        || (palDataMgr_Model_ID_Get() == MODULE_TYPE_ID2_PLATFORM)))
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
        #endif

		//G100_Clare_0076, add, <<<

		if(eEXEC_CODE_PASS == utilCommonCLI_DataConversionSet(edcUSER_COLOR_MODE, sCmdFormat->lData))
        {

        }
        else
        {
        	return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    else //Read
    {

    }

    return eCLI_ERROR_CODE_NO;
}


eCLI_ERROR_CODE utilGeneral_GAMM(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)    //G100_Simon_0066
{
    if(eAccessMode == ecmRead)
    {
        UINT32 uwData = 0;
        INT32 iHDR_Auto = ets_OFF;

        utilCommonCLI_DataConversionGet(edcHDR_AUTOENABLE, &iHDR_Auto);

        if(palImgMgr_HDR_Get(eSOURCE_WINDOW_MAIN) == eHDR_SETTING_HDR && iHDR_Auto == ets_ON)
        {
            uwData = CLI_GAMMA_HDR;
        }
        else
        {
    	    utilCommonCLI_DataConversionGet(edcGAMMA, &uwData);
        }

		if(uwData < 0 || uwData > CLI_GAMMA_HDR)
		{
			return eCLI_ERROR_CODE_REQUESTFAIL;
		}
		else
		{
			sCmdFormat->lData = (INT32)uwData;
		}

    }
    else if(eAccessMode == ecmWrite)
    {
        INT32 iHDR_Auto = ets_OFF;	//G100_Clare_0075

        utilCommonCLI_DataConversionGet(edcHDR_AUTOENABLE, &iHDR_Auto);	//G100_Clare_0075
        if(sCmdFormat->lData >= CLI_GAMMA_HDR)
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }
        //G100_Tim_0005, mod, start
        else if( (iHDR_Auto == ets_OFF) || (palImgMgr_HDR_Get(eSOURCE_WINDOW_MAIN) == eHDR_SETTING_SDR) )
        {
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
}

eCLI_ERROR_CODE utilGeneral_PJIP(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
{
    if(eAccessMode == ecmWrite)
    {
        INT32 iValue = 0;
        UINT8 aucLanTmp[4] = {0};
        UINT8 aucDataString[32] = {0};  //下面使用 strtok , 會把 . 取代為 NULL , 所以先複製一份來處理
        UINT8 ucIndex = 0;
        char *pch;

        memcpy(aucDataString, sCmdFormat->cTextString, strlen(sCmdFormat->cTextString));
        pch = strtok((char *)aucDataString, ".") ;   // 以 . 這個字元來分割字串

        while(pch != NULL && ucIndex < 4)
        {
            iValue = atoi(pch);

            if(eEXEC_CODE_PASS != utilCommonCLI_RangeCheck((UINT16)(edcPJLINK_ADDRESS1 + ucIndex), iValue))
            {
                return eCLI_ERROR_CODE_SETFAIL;
            }

            ucIndex++;
            pch = strtok(NULL, ".");
        }

        if(ucIndex != 4)
        {
            return eCLI_ERROR_CODE_FORMATERR;
        }

        utilCommonCLI_String_Set(edcPJLINK_ADDRESS, sCmdFormat->cTextString);
    }

    return eCLI_ERROR_CODE_NO;
}


eCLI_ERROR_CODE utilGeneral_CRIP(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
{
    if(eAccessMode == ecmWrite)
    {
        INT32 iValue = 0;
        UINT8 aucLanTmp[4] = {0};
        UINT8 aucDataString[32] = {0};  //下面使用 strtok , 會把 . 取代為 NULL , 所以先複製一份來處理
        UINT8 ucIndex = 0;
        char *pch;

        memcpy(aucDataString, sCmdFormat->cTextString, strlen(sCmdFormat->cTextString));
        pch = strtok((char *)aucDataString, ".") ;   // 以 . 這個字元來分割字串

        while(pch != NULL && ucIndex < 4)
        {
            iValue = atoi(pch);

            if(eEXEC_CODE_PASS != utilCommonCLI_RangeCheck((UINT16)(edcCRESTRON_ADDRESS1 + ucIndex), iValue))
            {
                return eCLI_ERROR_CODE_SETFAIL;
            }

            ucIndex++;
            pch = strtok(NULL, ".");
        }

        if(ucIndex != 4)
        {
            return eCLI_ERROR_CODE_FORMATERR;
        }

        utilCommonCLI_String_Set(edcCRESTRON_ADDRESS, sCmdFormat->cTextString);
    }

    return eCLI_ERROR_CODE_NO;
}


eCLI_ERROR_CODE utilGeneral_FANFA09(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
{
    if(eAccessMode == ecmWrite)
    {
        if(palSystem_ModelIDGet() == eMODEL_TYPE_DWU880GS)
        {
            return eCLI_ERROR_CODE_FUNCCODENOTFOUND;
        }
        else
        {
            return eCLI_ERROR_CODE_COMMAND_READ_ONLY;
        }
    }
    else if(eAccessMode == ecmRead)
    {
        if(palSystem_ModelIDGet() == eMODEL_TYPE_DWU880GS)
        {
            return eCLI_ERROR_CODE_FUNCCODENOTFOUND;
        }
        else
        {
            char cString[128] = {'\0'};

            utilCommonCLI_String_Get(edcFAN_RPM_09, cString);
            snprintf(sCmdFormat->cTextString, GENERAL_CMD_TEXT_SIZE, "%s", cString);
        }
    }

    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilGeneral_ICP(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
{
    eCLI_ERROR_CODE eErrorCode;
    sCLI_COMMON_FORMAT sCommonFormat;

    sCommonFormat.eAccessMode = eAccessMode;
    sCommonFormat.ucCmdFrom = sCmdFormat->ucCmdFrom;
    sCommonFormat.ucDataType = sCmdFormat->ucDataType;
    sCommonFormat.lData = sCmdFormat->lData;
    sCommonFormat.fData = sCmdFormat->fData;
    snprintf(sCommonFormat.cTextString, CLI_DATA_SIZE, "%s\n", sCmdFormat->cTextString);

    eErrorCode = utilCommonCLI_ICP(&sCommonFormat);

    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilGeneral_ICPC341_SET(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
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

eCLI_ERROR_CODE utilGeneral_ICPC341_GET(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
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

     sprintf(sCmdFormat->cTextString, "[%s+%s!%s]\n", sCmdFormat->cMainCode, sCmdFormat->cSubCode, sCommonFormat.cTextString);

    __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, sCmdFormat->cTextString);

    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilGeneral_DDTP(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
{
    if(eAccessMode == ecmWrite)
    {
        if(palSystem_ModelIDGet() == eMODEL_TYPE_DWU880GS)
        {
            return eCLI_ERROR_CODE_FUNCCODENOTFOUND;
        }
        else
        {
            return eCLI_ERROR_CODE_COMMAND_READ_ONLY;
        }
    }
    else if(eAccessMode == ecmRead)
    {
        if(palSystem_ModelIDGet() == eMODEL_TYPE_DWU880GS)
        {
            return eCLI_ERROR_CODE_FUNCCODENOTFOUND;
        }
        else
        {
            char cString[128] = {'\0'};

            utilCommonCLI_String_Get(edcTHERMAL_SENSOR_2, cString);
            snprintf(sCmdFormat->cTextString, GENERAL_CMD_TEXT_SIZE, "%s", cString);
        }
    }

    return eCLI_ERROR_CODE_NO;
}

 //A35G2_BRC_Casper_0046
eCLI_ERROR_CODE utilGeneral_WPTP(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
{
    UINT8 ucData = 0;
    palDataMgr_Data_Access(edcADV_WARP_CONTROL, edaREAD, &ucData);
    if(ucData != WARP_CTRL__ADVANCED)
    {
        return eCLI_ERROR_CODE_REQUESTFAIL;
    }

    if(eAccessMode == ecmWrite)
    {
        if(sCmdFormat->lData == 0)
        {
            if(palGeo_AdvWarpShowOsdPattern(PAT_TYPE__OFF) != eHAL_WARPING_EXEC_CODE_PASS)
            {
                return eCLI_ERROR_CODE_SETFAIL;
            }
        }
        else if(sCmdFormat->lData == 1)
        {
            if(palGeo_AdvWarpShowOsdPattern(PAT_TYPE__WARP_SEL_CTRL_POINT) != eHAL_WARPING_EXEC_CODE_PASS)
            {
                return eCLI_ERROR_CODE_SETFAIL;
            }
        }
        else
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    else if(eAccessMode == ecmRead)
    {
        sCmdFormat->lData = (INT32)utilWarp_GetWarpPatternState();
    }

    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilGeneral_WPPU(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
{
    UINT8 ucData = 0;

    palDataMgr_Data_Access(edcADV_WARP_CONTROL, edaREAD, &ucData);
    if(ucData != WARP_CTRL__ADVANCED)
    {
        return eCLI_ERROR_CODE_REQUESTFAIL;
    }

    if(eAccessMode == ecmWrite)
    {
        if(halAdvWarpGP_Move(DIR__UP, (UINT16)sCmdFormat->lData) != eHAL_WARPING_EXEC_CODE_PASS)
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    else if(eAccessMode == ecmRead)
    {
        return eCLI_ERROR_CODE_REQUESTFAIL;
    }

    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilGeneral_WPPD(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
{
    UINT8 ucData = 0;

    palDataMgr_Data_Access(edcADV_WARP_CONTROL, edaREAD, &ucData);
    if(ucData != WARP_CTRL__ADVANCED)
    {
        return eCLI_ERROR_CODE_REQUESTFAIL;
    }

    if(eAccessMode == ecmWrite)
    {
        if(halAdvWarpGP_Move(DIR__DOWN, (UINT16)sCmdFormat->lData) != eHAL_WARPING_EXEC_CODE_PASS)
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    else if(eAccessMode == ecmRead)
    {
        return eCLI_ERROR_CODE_REQUESTFAIL;
    }

    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilGeneral_WPPL(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
{
    UINT8 ucData = 0;

    palDataMgr_Data_Access(edcADV_WARP_CONTROL, edaREAD, &ucData);
    if(ucData != WARP_CTRL__ADVANCED)
    {
        return eCLI_ERROR_CODE_REQUESTFAIL;
    }

    if(eAccessMode == ecmWrite)
    {
        if(halAdvWarpGP_Move(DIR__LEFT, (UINT16)sCmdFormat->lData) != eHAL_WARPING_EXEC_CODE_PASS)
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    else if(eAccessMode == ecmRead)
    {
        return eCLI_ERROR_CODE_REQUESTFAIL;
    }

    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilGeneral_WPPR(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
{
    UINT8 ucData = 0;

    palDataMgr_Data_Access(edcADV_WARP_CONTROL, edaREAD, &ucData);
    if(ucData != WARP_CTRL__ADVANCED)
    {
        return eCLI_ERROR_CODE_REQUESTFAIL;
    }

    if(eAccessMode == ecmWrite)
    {
        if(halAdvWarpGP_Move(DIR__RIGHT,  (UINT16)sCmdFormat->lData) != eHAL_WARPING_EXEC_CODE_PASS)
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    else if(eAccessMode == ecmRead)
    {
        return eCLI_ERROR_CODE_REQUESTFAIL;
    }

    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilGeneral_WPXY(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
{
    UINT8 ucData = 0;

    palDataMgr_Data_Access(edcADV_WARP_CONTROL, edaREAD, &ucData);
    if(ucData != WARP_CTRL__ADVANCED)
    {
        return eCLI_ERROR_CODE_REQUESTFAIL;
    }

    if(eAccessMode == ecmRead)
    {
        sprintf(sCmdFormat->cTextString, "%d,%d", utilWarp_GridPoint_GetColPosition(), utilWarp_GridPoint_GetRowPosition());
    }
    else if(eAccessMode == ecmWrite)
    {
        INT32 alVal[2] = {0};
        UINT8 ucIndex = 0 ;
        char *pch ;
        pch = strtok(sCmdFormat->cTextString, ",");

        while(pch != NULL && ucIndex < 2)
        {
            alVal[ucIndex] = atol(pch);
            ucIndex++ ;
            pch = strtok(NULL, ",") ;
        }

        if(ucIndex < 2)
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        if(halAdvWarpGP_SetPositionMove(alVal[0], alVal[1]) != eHAL_WARPING_EXEC_CODE_PASS)
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }

    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilGeneral_WRST(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
{
    UINT8 ucData = 0;
    palDataMgr_Data_Access(edcADV_WARP_CONTROL, edaREAD, &ucData);
    if(ucData != WARP_CTRL__ADVANCED)
    {
        return eCLI_ERROR_CODE_REQUESTFAIL;
    }

    if(eAccessMode == ecmWrite)
    {
        if(sCmdFormat->lData == 1)
        {
            if(halAdvWarpSettingReset() == eHAL_WARPING_EXEC_CODE_PASS)
            {
                palDataMgr_AdvReset_Execution(0);   //G100_Owen_0091
                return eCLI_ERROR_CODE_NO;
            }
        }
    }
//    else if(eAccessMode == ecmRead)
//    {
//        return eCLI_ERROR_CODE_REQUESTFAIL;
//    }

    return eCLI_ERROR_CODE_SETFAIL;
}

eCLI_ERROR_CODE utilGeneral_BLRS(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat) //A35G2_BRC_Casper_0051
{
    UINT8 ucData = 0;   //G100_Owen_0103

    palDataMgr_Data_Access(edcADV_WARP_CONTROL, edaREAD, &ucData);  //G100_Owen_0103
    if(ucData != WARP_CTRL__ADVANCED)   //G100_Owen_0103
    {
        return eCLI_ERROR_CODE_SETFAIL;
    }

    if(eAccessMode == ecmWrite)
    {
        if(halAdvBlendingEnable_Set((BOOL)sCmdFormat->lData) != eHAL_WARPING_EXEC_CODE_PASS)
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    else if(eAccessMode == ecmRead) //G100_Owen_0091
    {
        sCmdFormat->lData = (INT32)utilWarp_GetBlendEnable();
    }

    return eCLI_ERROR_CODE_NO;
}


eCLI_ERROR_CODE utilGeneral_OLTP(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat) //A35G2_BRC_Casper_0051
{
    UINT8 ucData = 0;   //G100_Owen_0103

    palDataMgr_Data_Access(edcADV_WARP_CONTROL, edaREAD, &ucData);  //G100_Owen_0103
    if(ucData != WARP_CTRL__ADVANCED)   //G100_Owen_0103
    {
        return eCLI_ERROR_CODE_SETFAIL;
    }

    if(eAccessMode == ecmWrite)
    {
        if(sCmdFormat->lData == 0)
        {
            if(palGeo_AdvWarpShowOsdPattern(PAT_TYPE__OFF) != eHAL_WARPING_EXEC_CODE_PASS)
            {
                return eCLI_ERROR_CODE_SETFAIL;
            }
        }
        else if(sCmdFormat->lData == 1)
        {
            if(palGeo_AdvWarpShowOsdPattern(PAT_TYPE__BLEND_NO_CTRL_POINT) != eHAL_WARPING_EXEC_CODE_PASS)
            {
                return eCLI_ERROR_CODE_SETFAIL;
            }
        }
        else
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    else if(eAccessMode == ecmRead) //G100_Owen_0091
    {
        sCmdFormat->lData = (INT32)utilWarp_GetBlendPatternState();
    }

    return eCLI_ERROR_CODE_NO;
}


eCLI_ERROR_CODE utilGeneral_OLPU(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat) //A35G2_BRC_Casper_0051
{
    UINT8 ucData = 0;   //G100_Owen_0103

    palDataMgr_Data_Access(edcADV_WARP_CONTROL, edaREAD, &ucData);  //G100_Owen_0103
    if(ucData != WARP_CTRL__ADVANCED)   //G100_Owen_0103
    {
        return eCLI_ERROR_CODE_SETFAIL;
    }

    if(eAccessMode == ecmWrite)
    {
        if(halAdvBlending_Width(DIR__UP,  (UINT16)sCmdFormat->lData) != eHAL_WARPING_EXEC_CODE_PASS)
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
        if( utilWarp_GetOsdPatternType() == PAT_TYPE__SEL_BLEND_WIDTH )
        {
            palGeo_AdvWarpShowOsdPattern(PAT_TYPE__SEL_BLEND_WIDTH);
        }
    }
    else if(eAccessMode == ecmRead) //G100_Owen_0091
    {
        sCmdFormat->lData = (INT32)utilWarp_GetOsdBlendWidthTop();
    }

    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilGeneral_OLPD(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat) //A35G2_BRC_Casper_0051
{
    UINT8 ucData = 0;   //G100_Owen_0103

    palDataMgr_Data_Access(edcADV_WARP_CONTROL, edaREAD, &ucData);  //G100_Owen_0103
    if(ucData != WARP_CTRL__ADVANCED)   //G100_Owen_0103
    {
        return eCLI_ERROR_CODE_SETFAIL;
    }

    if(eAccessMode == ecmWrite)
    {
        if(halAdvBlending_Width(DIR__DOWN,  (UINT16)sCmdFormat->lData) != eHAL_WARPING_EXEC_CODE_PASS)
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
        if( utilWarp_GetOsdPatternType() == PAT_TYPE__SEL_BLEND_WIDTH )
        {
            palGeo_AdvWarpShowOsdPattern(PAT_TYPE__SEL_BLEND_WIDTH);
        }
    }
    else if(eAccessMode == ecmRead) //G100_Owen_0091
    {
        sCmdFormat->lData = (INT32)utilWarp_GetOsdBlendWidthDown();
    }

    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilGeneral_OLPL(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat) //A35G2_BRC_Casper_0051
{
    UINT8 ucData = 0;   //G100_Owen_0103

    palDataMgr_Data_Access(edcADV_WARP_CONTROL, edaREAD, &ucData);  //G100_Owen_0103
    if(ucData != WARP_CTRL__ADVANCED)   //G100_Owen_0103
    {
        return eCLI_ERROR_CODE_SETFAIL;
    }

    if(eAccessMode == ecmWrite)
    {
        if(halAdvBlending_Width(DIR__LEFT,  (UINT16)sCmdFormat->lData) != eHAL_WARPING_EXEC_CODE_PASS)
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
        if( utilWarp_GetOsdPatternType() == PAT_TYPE__SEL_BLEND_WIDTH )
        {
            palGeo_AdvWarpShowOsdPattern(PAT_TYPE__SEL_BLEND_WIDTH);
        }
    }
    else if(eAccessMode == ecmRead) //G100_Owen_0091
    {
        sCmdFormat->lData = (INT32)utilWarp_GetOsdBlendWidthLeft();
    }

    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilGeneral_OLPR(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat) //A35G2_BRC_Casper_0051
{
    UINT8 ucData = 0;   //G100_Owen_0103

    palDataMgr_Data_Access(edcADV_WARP_CONTROL, edaREAD, &ucData);  //G100_Owen_0103
    if(ucData != WARP_CTRL__ADVANCED)   //G100_Owen_0103
    {
        return eCLI_ERROR_CODE_SETFAIL;
    }

    if(eAccessMode == ecmWrite)
    {
        if(halAdvBlending_Width(DIR__RIGHT,  (UINT16)sCmdFormat->lData) != eHAL_WARPING_EXEC_CODE_PASS)
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
        if( utilWarp_GetOsdPatternType() == PAT_TYPE__SEL_BLEND_WIDTH )
        {
            palGeo_AdvWarpShowOsdPattern(PAT_TYPE__SEL_BLEND_WIDTH);
        }
    }
    else if(eAccessMode == ecmRead) //G100_Owen_0091
    {
        sCmdFormat->lData = (INT32)utilWarp_GetOsdBlendWidthRight();
    }

    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilGeneral_OSPU(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat) //A35G2_BRC_Casper_0051
{
    UINT8 ucData = 0;   //G100_Owen_0103

    palDataMgr_Data_Access(edcADV_WARP_CONTROL, edaREAD, &ucData);  //G100_Owen_0103
    if(ucData != WARP_CTRL__ADVANCED)   //G100_Owen_0103
    {
        return eCLI_ERROR_CODE_SETFAIL;
    }

    if(eAccessMode == ecmWrite)
    {
        if(halAdvBlending_Offset(DIR__UP,  (UINT16)sCmdFormat->lData) != eHAL_WARPING_EXEC_CODE_PASS)
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
        if( utilWarp_GetOsdPatternType() == PAT_TYPE__SEL_BLEND_WIDTH )
        {
            palGeo_AdvWarpShowOsdPattern(PAT_TYPE__SEL_BLEND_WIDTH);
        }
    }
    else if(eAccessMode == ecmRead) //G100_Owen_0091
    {
        sCmdFormat->lData = (INT32)utilWarp_GetOsdBlendOffsetTop();
    }

    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilGeneral_OSPD(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat) //A35G2_BRC_Casper_0051
{
    UINT8 ucData = 0;   //G100_Owen_0103

    palDataMgr_Data_Access(edcADV_WARP_CONTROL, edaREAD, &ucData);  //G100_Owen_0103
    if(ucData != WARP_CTRL__ADVANCED)   //G100_Owen_0103
    {
        return eCLI_ERROR_CODE_SETFAIL;
    }

    if(eAccessMode == ecmWrite)
    {
        if(halAdvBlending_Offset(DIR__DOWN,  (UINT16)sCmdFormat->lData) != eHAL_WARPING_EXEC_CODE_PASS)
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
        if( utilWarp_GetOsdPatternType() == PAT_TYPE__SEL_BLEND_WIDTH )
        {
            palGeo_AdvWarpShowOsdPattern(PAT_TYPE__SEL_BLEND_WIDTH);
        }
    }
    else if(eAccessMode == ecmRead) //G100_Owen_0091
    {
        sCmdFormat->lData = (INT32)utilWarp_GetOsdBlendOffsetDown();
    }

    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilGeneral_OSPL(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat) //A35G2_BRC_Casper_0051
{
    UINT8 ucData = 0;   //G100_Owen_0103

    palDataMgr_Data_Access(edcADV_WARP_CONTROL, edaREAD, &ucData);  //G100_Owen_0103
    if(ucData != WARP_CTRL__ADVANCED)   //G100_Owen_0103
    {
        return eCLI_ERROR_CODE_SETFAIL;
    }

    if(eAccessMode == ecmWrite)
    {
        if(halAdvBlending_Offset(DIR__LEFT,  (UINT16)sCmdFormat->lData) != eHAL_WARPING_EXEC_CODE_PASS)
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
        if( utilWarp_GetOsdPatternType() == PAT_TYPE__SEL_BLEND_WIDTH )
        {
            palGeo_AdvWarpShowOsdPattern(PAT_TYPE__SEL_BLEND_WIDTH);
        }
    }
    else if(eAccessMode == ecmRead) //G100_Owen_0091
    {
        sCmdFormat->lData = (INT32)utilWarp_GetOsdBlendOffsetLeft();
    }

    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilGeneral_OSPR(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat) //A35G2_BRC_Casper_0051
{
    UINT8 ucData = 0;   //G100_Owen_0103

    palDataMgr_Data_Access(edcADV_WARP_CONTROL, edaREAD, &ucData);  //G100_Owen_0103
    if(ucData != WARP_CTRL__ADVANCED)   //G100_Owen_0103
    {
        return eCLI_ERROR_CODE_SETFAIL;
    }

    if(eAccessMode == ecmWrite)
    {
        if(halAdvBlending_Offset(DIR__RIGHT,  (UINT16)sCmdFormat->lData) != eHAL_WARPING_EXEC_CODE_PASS)
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
        if( utilWarp_GetOsdPatternType() == PAT_TYPE__SEL_BLEND_WIDTH )
        {
            palGeo_AdvWarpShowOsdPattern(PAT_TYPE__SEL_BLEND_WIDTH);
        }
    }
    else if(eAccessMode == ecmRead) //G100_Owen_0091
    {
        sCmdFormat->lData = (INT32)utilWarp_GetOsdBlendOffsetRight();
    }

    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilGeneral_REST(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat) //A35G2_BRC_Casper_0051
{
    UINT8 ucData = 0;   //G100_Owen_0103

    palDataMgr_Data_Access(edcADV_WARP_CONTROL, edaREAD, &ucData);  //G100_Owen_0103
    if(ucData != WARP_CTRL__ADVANCED)   //G100_Owen_0103
    {
        return eCLI_ERROR_CODE_SETFAIL;
    }

    if(eAccessMode == ecmWrite)
    {
        if(sCmdFormat->lData == 1)
        {
            if(halAdvBlendConfigReset() == eHAL_WARPING_EXEC_CODE_PASS)
            {
                palDataMgr_AdvReset_Execution(1);   //G100_Owen_0091
                return eCLI_ERROR_CODE_NO;
            }
        }
    }
//    else if(eAccessMode == ecmRead)
//    {
//        return eCLI_ERROR_CODE_REQUESTFAIL;
//    }

    return eCLI_ERROR_CODE_SETFAIL;
}


//G100_Steven_0098 //A35G2_BRC_Casper_0047
eCLI_ERROR_CODE utilGeneral_UDCC(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
{
    if(eAccessMode == ecmRead)
    {
    	BYTE cIndex = 0;
    	//UINT8 ucUserMemData = 0;
        UINT8 ucOutputString[128] = {0};
        memset((UINT8*)ucOutputString, 0, sizeof(ucOutputString));

        sprintf(ucOutputString, "[UDCC!");//G100_Steven_0103 start
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);

    	for(cIndex=0; cIndex < eUSER_DATA_LOAD_MAX_NUMBER; cIndex++)
    	{
    		if(utilDataMgr_UserDataLoadItemAvailable_CM(cIndex)==1)	//HICC2_Doulas_0003
    		{
    			//ucUserMemData = ucUserMemData | (1 << cIndex);
    			sprintf(ucOutputString, "1");
    			__CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
    		}
    		else
    		{
    			sprintf(ucOutputString, "0");
    			__CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
    		}
    	} //G100_Steven_0103 end

        sprintf(ucOutputString, "]\r\n"); //G100_Steven_0101
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
    }

    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilGeneral_LMSP(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)  //G100_Doulas_0027 //A35G2_BRC_Casper_0093
{
    if(eAccessMode == ecmWrite)
    {
        if(sCmdFormat->lData == 0 || sCmdFormat->lData > 5 )
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE ;
        }

    	UINT8 ucValue = sCmdFormat->lData;
		if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(edcLENS_SAVE_CURRENT_POSITION))		//G100_Doulas_0040 Add
		{
			return eCLI_ERROR_CODE_SETFAIL;
		}
    	utilCommonCLI_DataConversionSet(edcLENS_SAVE_CURRENT_POSITION, (INT32)ucValue);
    }
    else if(eAccessMode == ecmRead)
    {
        INT32 iValue = 0 ;
		if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(edcLENS_SAVE_CURRENT_POSITION))		//G100_Doulas_0040 Add
		{
			return eCLI_ERROR_CODE_REQUESTFAIL;
		}
        utilCommonCLI_DataConversionGet(edcLENS_SAVE_CURRENT_POSITION, &iValue);
        sCmdFormat->lData = iValue;
    }

    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilGeneral_LMAP(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)  //G100_Doulas_0027 //A35G2_BRC_Casper_0093
{
    if(eAccessMode == ecmWrite)
    {
    	UINT8 ucValue = sCmdFormat->lData;
        UINT16 ucLMAP_Index = 0x01 << (ucValue-1); //A35G2_BRC_Casper_0094

        if(sCmdFormat->lData == 0 || sCmdFormat->lData > 5 )
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE ;
        }

		if(!(ucLMAP_Index & palEnvironment_LensMemorySavingCondition_Get()) ||
		   (eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(edcLENS_APPLY_POSITION)))		//G100_Doulas_0040 Add //A35G2_BRC_Casper_0094
		{
			return eCLI_ERROR_CODE_SETFAIL;
		}
    	utilCommonCLI_DataConversionSet(edcLENS_APPLY_POSITION, (INT32)ucValue);
    }
    else if(eAccessMode == ecmRead)
    {
        INT32 iValue = 0 ;
		if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(edcLENS_APPLY_POSITION))		//G100_Doulas_0040 Add
		{
			return eCLI_ERROR_CODE_REQUESTFAIL;
		}
        utilCommonCLI_DataConversionGet(edcLENS_APPLY_POSITION, &iValue);
        sCmdFormat->lData = iValue;
    }

    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilGeneral_LMCP(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat) //A35G2_BRC_Casper_0047
{
    if(eAccessMode == ecmRead)
    {
    	BYTE cIndex = 0;
       	UINT8 ucLesMemData = 0;
        UINT8 ucOutputString[128] = {0};
        memset((UINT8*)ucOutputString, 0, sizeof(ucOutputString));
        halMotor_LensMemoryRecord_Get(&ucLesMemData);

        sprintf(ucOutputString, "[LMCP!"); //G100_Steven_0101
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);

        for(cIndex=0; cIndex < 5; cIndex++) //G100_Steven_0103
        {
        	if(utilGeneral_CheckBit(ucLesMemData, cIndex))
        	{
        		sprintf(ucOutputString, "1");
        		__CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        	}
        	else
        	{
        		sprintf(ucOutputString, "0");
        		__CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        	}
        }

        sprintf(ucOutputString, "]\r\n"); //G100_Steven_0101
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        //sCmdFormat->lData = ucLesMemData;

    }
    return eCLI_ERROR_CODE_NO;
}



eCLI_ERROR_CODE utilGeneral_GMCC(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat) //A35G2_BRC_Casper_0047
{

    if(eAccessMode == ecmRead)
    {
    	BYTE cIndex = 0;
       	//UINT8 ucWarpMemData = 0;
        UINT8 ucOutputString[128] = {0};
        memset((UINT8*)ucOutputString, 0, sizeof(ucOutputString));

        sprintf(ucOutputString, "[GMCC!"); //G100_Steven_0101
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);

       	for(cIndex=0; cIndex < 5; cIndex++) //G100_Steven_0103
       	{
       		if(palDataMgr_ApplyWarpMemoryItemAvailable(cIndex) == 0) //A35G2_BRC_Casper_0084
       		{
       			//ucWarpMemData = ucWarpMemData | (1 << cIndex);
       			sprintf(ucOutputString, "0");
        		__CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
       		}
       		else //A35G2_BRC_Casper_0084
       		{
      			sprintf(ucOutputString, "1");
        		__CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
       		}
        }

        sprintf(ucOutputString, "]"); //G100_Steven_0101
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);

    }
    return eCLI_ERROR_CODE_NO;
}


//G100_Steven_0098


//G100_Steven_0118 //A35G2_BRC_Casper_0051
eCLI_ERROR_CODE utilGeneral_DDTC(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
{
    if(eAccessMode == ecmRead)
    {

        UINT8 ucOutputString[128] = {0};
        UINT16 wDMDTEC_Current = palDataMgr_Access_Get_TEC_Current(0);

        memset((UINT8*)ucOutputString, 0, sizeof(ucOutputString));

        sprintf(ucOutputString, "[DDTC!%d]\r\n",wDMDTEC_Current); //G100_Steven_0101
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);

    }
    return eCLI_ERROR_CODE_NO;
}
//G100_Steven_0118

eCLI_ERROR_CODE utilGeneral_UDLD(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)  //A35G2_BRC_Casper_0057
{
    if(eAccessMode == ecmWrite)
    {
    	UINT8 ucValue = (UINT8)sCmdFormat->lData;

        if(utilDataMgr_UserDataLoadItemAvailable_CM(ucValue) == FALSE)		//HICC2_Doulas_0003
        {
            return eCLI_ERROR_CODE_REQUESTFAIL;
        }
    	utilCommonCLI_DataConversionSet(edcBACKUP_RESTORE_RESTORE, (INT32)ucValue);
    }
    else if(eAccessMode == ecmRead)
    {
        INT32 iValue = 0 ;
        utilCommonCLI_DataConversionGet(edcBACKUP_RESTORE_RESTORE, &iValue);
        sCmdFormat->lData = iValue;
    }

    return eCLI_ERROR_CODE_NO;
}

// ==============================================================================
// FUNCTION NAME: utilGeneral_BODCUST
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
eCLI_ERROR_CODE utilGeneral_BODCUST(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
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
// FUNCTION NAME: utilGeneral_BODCUST
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
eCLI_ERROR_CODE utilGeneral_BODPLAT(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)
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

eCLI_ERROR_CODE utilGeneral_CLI_UST(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)		//ZU860_Julie_0002 //A35G2_BRC_Casper_0088
{
    if(eAccessMode == ecmRead)
    {
        sCmdFormat->lData = 4;
        return eCLI_ERROR_CODE_COMMAND_WRITE_ONLY;

    }
    else if(eAccessMode == ecmWrite)
    {
        if(sCmdFormat->lData == 1)
        {
            char ucOutputString[512];

            utilCommonCLI_DataConversionSet(edcUST_SET, (INT32)sCmdFormat->lData); //GuiCb.fpGui_USTInstall_StartCb(); //###
            sprintf(ucOutputString, "\r\n1. To position the ultra short throw lens, perform a lens shift calibration.\r\n2. Once calibration is complete, press Exit.\r\n3. Power off the projector.\r\n4. Install the ultra short throw lens.\r\n5. Power on the projector.\r\n   The constant power is lower during the adjustment process.\r\n");

            __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        }
        else
        {
            utilCommonCLI_DataConversionSet(edcUST_SET, ets_OFF); //GuiCb.fpGui_DataCode_Value_SetCb(edcUST_SET, edaWRITE_THROUGH_WITH_ACTION, ets_OFF); //###
        }
    }

    return eCLI_ERROR_CODE_NO;
}
//SNPLU9000_Energy_0011
eCLI_ERROR_CODE utilGeneral_CLI_USTS(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat)		//ZU860_Julie_0002 //A35G2_BRC_Casper_0088
{
    if(eAccessMode == ecmRead)
    {
        sCmdFormat->lData = 4;
        return eCLI_ERROR_CODE_COMMAND_WRITE_ONLY;

    }
    else if(eAccessMode == ecmWrite)
    {
        if(sCmdFormat->lData == 0)
        {
            utilCommonCLI_DataConversionSet(edcUST_SET, ets_ON); //GuiCb.fpGui_USTInstall_StartCb(); //###
        }
        else
        {
            return eCLI_ERROR_CODE_DATAERROR;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilGeneral_HSRT(eCLI_MODE eAccessMode, sCLI_GENERAL_FORMAT* sCmdFormat) //A35G2_BRC_Casper_0114
{
    if(sCmdFormat->lData < 0 || sCmdFormat->lData > 6 )
    {
        return eCLI_ERROR_CODE_DATAOVERRANGE ;
    }

    if(eAccessMode == ecmWrite)
    {
        eDATA_CODE uwDatacode = 0;
        UINT8 ucIndex = ets_ON;
		uwDatacode = edcHSG_RED_RESET_DEFAULT + (UINT16)sCmdFormat->lData;
		palDataMgr_Data_Access(uwDatacode, edaWRITE_THROUGH_WITH_ACTION, &ucIndex); //GuiCb.fpGui_DataCode_Value_SetCb(uwDatacode, edaWRITE_THROUGH_WITH_ACTION, ets_ON);
        return eCLI_ERROR_CODE_NO;
    }

    return eCLI_ERROR_CODE_SETFAIL;
}

///////////////////////////////////////////////////////////////

// ==============================================================================
// FUNCTION NAME: m_sSubLut
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
const sCLI_SUB_LUT m_sSubLut[] =
{
    //uiMainCmdID       ucSubCmd    ucCmdIsRead       ucCmdBytesOnRange   iFunCodeID;             uiSpecialFlag
    //SYS============================================================================================================//SYS
    {eCLI_MAIN_SYS,     NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,   utilGeneral_CLI_SYS,       CLI_COM_READ_ONLY},
    {eCLI_MAIN_SYS,     NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,                  NULL,       CLI_COM_READ_ONLY},
    {eCLI_MAIN_SYS,     "DBMK",     READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,   utilGeneral_CLI_SYSDBMK,   CLI_COM_NO_REPLY},
    {eCLI_MAIN_SYS,     "DBMK",     WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,   utilGeneral_CLI_SYSDBMK,   CLI_COM_NO_REPLY},
    {eCLI_MAIN_SYS,     "ELOG",     READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,   utilGeneral_CLI_SYSELOG,   CLI_COM_READ_ONLY},
    {eCLI_MAIN_SYS,     "ELOG",     WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,   utilGeneral_CLI_SYSELOG,   CLI_COM_READ_ONLY},
    {eCLI_MAIN_SYS,     "TWIS",     WRITE_COMMAND,    DECIAML_2,          edcTWIST_LINK,               NULL,        CLI_COM_WRITE_ONLY}, //A70LV_Larry_0218
    //{eCLI_MAIN_SYS,     "FPGA",     READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,   utilGeneral_CLI_SYS_FPGA,  CLI_COM_READ_ONLY | CLI_COM_NO_REPLY},
    //{eCLI_MAIN_SYS,     "FPGA",     READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,   utilGeneral_CLI_SYS_FPGA,  CLI_COM_READ_ONLY | CLI_COM_NO_REPLY},

    {eCLI_MAIN_SYS,     "SMDC",     READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,   utilGeneral_CLI_SYS_SMDC,  CLI_COM_READ_ONLY | CLI_COM_NO_REPLY},
    {eCLI_MAIN_SYS,     "SMIF",     READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,   utilGeneral_CLI_SYS_SMIF,  CLI_COM_READ_ONLY | CLI_COM_NO_REPLY},

    {eCLI_MAIN_SYS,     "C949",     READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,   utilGeneral_CLI_SYS_VCXO949,  CLI_COM_READ_ONLY | CLI_COM_NO_REPLY},
    {eCLI_MAIN_SYS,     "C949",     WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,   utilGeneral_CLI_SYS_VCXO949,  CLI_COM_NO_REPLY},
 //   {eCLI_MAIN_SYS,     "6805",     READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,   utilGeneral_CLI_SYS_6805,  CLI_COM_NO_REPLY},
 //   {eCLI_MAIN_SYS,     "6805",     WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,   utilGeneral_CLI_SYS_6805,  CLI_COM_NO_REPLY},
#ifdef SCALER_C821_C789
    {eCLI_MAIN_SYS,     "C821",     READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,   utilGeneral_CLI_SYS_C821,  CLI_COM_READ_ONLY | CLI_COM_NO_REPLY},
    {eCLI_MAIN_SYS,     "C821",     WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,   utilGeneral_CLI_SYS_C821,  CLI_COM_NO_REPLY},
 	{eCLI_MAIN_SYS,     "C789",     READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,   utilGeneral_CLI_SYS_C789,  CLI_COM_READ_ONLY | CLI_COM_NO_REPLY},	//G100_Doulas_0027
 	{eCLI_MAIN_SYS,     "C789",     WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,   utilGeneral_CLI_SYS_C789,  CLI_COM_NO_REPLY},						//G100_Doulas_0027
#elif defined(SCALER_C341)
    {eCLI_MAIN_SYS,     "C341",     READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,   utilGeneral_CLI_SYS_C821,  CLI_COM_READ_ONLY | CLI_COM_NO_REPLY},
    {eCLI_MAIN_SYS,     "C341",     WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,   utilGeneral_CLI_SYS_C821,  CLI_COM_NO_REPLY},
#endif
 	{eCLI_MAIN_SYS,     "EMER",     WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,   utilGeneral_CLI_SYS_EmergencyTest,    CLI_COM_NO_REPLY},
 	{eCLI_MAIN_SYS,     "DMDB",     WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,   utilGeneral_CLI_SYS_DMDB,             CLI_COM_NO_REPLY},

 	{eCLI_MAIN_SYS,     "VERC",     WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,   utilGeneral_CLI_SYS_VersionCheck,     CLI_COM_NO_REPLY},
    {eCLI_MAIN_SYS,     "HDBT",     READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,   utilGeneral_CLI_SYS_HDBT_Status,      CLI_COM_NO_REPLY},

    {eCLI_MAIN_SYS,     "RXPR",     WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,   utilGeneral_CLI_RXPR,      CLI_COM_NO_REPLY},

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

	//PCM============================================================================================================//DSV
	{eCLI_MAIN_PCM, 	NULL,		READ_COMMAND,	  DECIAML_3,		  edcTWIST_LINK,	   		   NULL,		   CLI_COM_NORMAL},

    //WAP
  //  {eCLI_MAIN_WAP,     NULL,       READ_COMMAND,     DECIAML_2,          edcWARP_MEMORY_APPLY,        NULL,           CLI_COM_NORMAL},  //G100_Doulas_0027 remove//T100_Simon_0031
  //  {eCLI_MAIN_WAP,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcWARP_MEMORY_APPLY,        NULL,           CLI_COM_NORMAL},  //G100_Doulas_0027 remove//T100_Simon_0031

    //WAS
  //  {eCLI_MAIN_WAS,     NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,                utilGeneral_WAS,  CLI_COM_NORMAL},  //G100_Doulas_0027 remove//T100_Simon_0031
  //  {eCLI_MAIN_WAS,     NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,                utilGeneral_WAS,  CLI_COM_NORMAL},  //G100_Doulas_0027 remove//T100_Simon_0031

    //HKS============================================================================================================//HKS
    {eCLI_MAIN_KSH,     NULL,       READ_COMMAND,     DECIAML_3,          edcHORIZONTAL_KEYSTONE,      NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_KSH,     NULL,       WRITE_COMMAND,    DECIAML_3,          edcHORIZONTAL_KEYSTONE,      NULL,           CLI_COM_NORMAL},

    //VKS============================================================================================================//VKS
    {eCLI_MAIN_KSV,     NULL,       READ_COMMAND,     DECIAML_3,          edcVERTICAL_KEYSTONE,        NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_KSV,     NULL,       WRITE_COMMAND,    DECIAML_3,          edcVERTICAL_KEYSTONE,        NULL,           CLI_COM_NORMAL},

    //WRP============================================================================================================//WRP
    {eCLI_MAIN_WRP,     "MWWM",     READ_COMMAND,     DECIAML_2,          edcWARP_MODE,                NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_WRP,     "MWWM",     WRITE_COMMAND,    DECIAML_2,          edcWARP_MODE,                NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_WRP,     "MWWP",     READ_COMMAND,     DECIAML_2,          edcWARP_MOVE_PITCH_MODE,     NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_WRP,     "MWWP",     WRITE_COMMAND,    DECIAML_2,          edcWARP_MOVE_PITCH_MODE,     NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_WRP,     "MWCA",     READ_COMMAND,     DECIAML_2,          edcWARP_ADJUSTMENT,          NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_WRP,     "MWCA",     WRITE_COMMAND,    DECIAML_2,          edcWARP_ADJUSTMENT,          NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_WRP,     "DBGG",     READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,                utilGeneral_WRPG_DBGG,   CLI_COM_NORMAL},
    {eCLI_MAIN_WRP,     "DBGG",     WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,                utilGeneral_WRPG_DBGG,   CLI_COM_NORMAL},

    //HPC============================================================================================================//HPC
    {eCLI_MAIN_HPC,     NULL,       READ_COMMAND,     DECIAML_3,          edcHORZ_PINCUSHION,          NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_HPC,     NULL,       WRITE_COMMAND,    DECIAML_3,          edcHORZ_PINCUSHION,          NULL,           CLI_COM_NORMAL},

    //VPC============================================================================================================//VPC
    {eCLI_MAIN_VPC,     NULL,       READ_COMMAND,     DECIAML_3,          edcVERT_PINCUSHION,          NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_VPC,     NULL,       WRITE_COMMAND,    DECIAML_3,          edcVERT_PINCUSHION,          NULL,           CLI_COM_NORMAL},

    //TLCX============================================================================================================//TLCX
    {eCLI_MAIN_TLCX,    NULL,       READ_COMMAND,     DECIAML_3,          edc4CORNER_TOP_LEFT_HORZ,    NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_TLCX,    NULL,       WRITE_COMMAND,    DECIAML_3,          edc4CORNER_TOP_LEFT_HORZ,    NULL,           CLI_COM_NORMAL},

    //TLCY============================================================================================================//TLCY
    {eCLI_MAIN_TLCY,    NULL,       READ_COMMAND,     DECIAML_3,          edc4CORNER_TOP_LEFT_VERT,    NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_TLCY,    NULL,       WRITE_COMMAND,    DECIAML_3,          edc4CORNER_TOP_LEFT_VERT,    NULL,           CLI_COM_NORMAL},

    //TRCX============================================================================================================//TRCX
    {eCLI_MAIN_TRCX,    NULL,       READ_COMMAND,     DECIAML_3,          edc4CORNER_TOP_RIGHT_HORZ,   NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_TRCX,    NULL,       WRITE_COMMAND,    DECIAML_3,          edc4CORNER_TOP_RIGHT_HORZ,   NULL,           CLI_COM_NORMAL},

    //TRCY============================================================================================================//TRCY
    {eCLI_MAIN_TRCY,    NULL,       READ_COMMAND,     DECIAML_3,          edc4CORNER_TOP_RIGHT_VERT,   NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_TRCY,    NULL,       WRITE_COMMAND,    DECIAML_3,          edc4CORNER_TOP_RIGHT_VERT,   NULL,           CLI_COM_NORMAL},

    //BLCX============================================================================================================//BLCX
    {eCLI_MAIN_BLCX,    NULL,       READ_COMMAND,     DECIAML_3,          edc4CORNER_BOTTOM_LEFT_HORZ, NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_BLCX,    NULL,       WRITE_COMMAND,    DECIAML_3,          edc4CORNER_BOTTOM_LEFT_HORZ, NULL,           CLI_COM_NORMAL},

    //BLCY============================================================================================================//BLCY
    {eCLI_MAIN_BLCY,    NULL,       READ_COMMAND,     DECIAML_3,          edc4CORNER_BOTTOM_LEFT_VERT, NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_BLCY,    NULL,       WRITE_COMMAND,    DECIAML_3,          edc4CORNER_BOTTOM_LEFT_VERT, NULL,           CLI_COM_NORMAL},

    //BRCX============================================================================================================//BRCX
    {eCLI_MAIN_BRCX,    NULL,       READ_COMMAND,     DECIAML_3,          edc4CORNER_BOTTOM_RIGHT_HORZ, NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_BRCX,    NULL,       WRITE_COMMAND,    DECIAML_3,          edc4CORNER_BOTTOM_RIGHT_HORZ, NULL,          CLI_COM_NORMAL},

    //BRCY============================================================================================================//BRCY
    {eCLI_MAIN_BRCY,    NULL,       READ_COMMAND,     DECIAML_3,          edc4CORNER_BOTTOM_RIGHT_VERT, NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_BRCY,    NULL,       WRITE_COMMAND,    DECIAML_3,          edc4CORNER_BOTTOM_RIGHT_VERT, NULL,          CLI_COM_NORMAL},

    //CNR============================================================================================================//CNR
    {eCLI_MAIN_CNR,     "RSET",     READ_COMMAND,     DECIAML_2,          edc4CORNER_RESET,             NULL,          CLI_COM_WRITE_ONLY}, //T100_Simon_0030
    {eCLI_MAIN_CNR,     "RSET",     WRITE_COMMAND,    DECIAML_2,          edc4CORNER_RESET,             NULL,          CLI_COM_WRITE_ONLY}, //T100_Simon_0030

    //AWF============================================================================================================//AWF
    {eCLI_MAIN_AWF,     NULL,       READ_COMMAND,     DECIAML_2,          edcAUTO_WARP_FILTER,         NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_AWF,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcAUTO_WARP_FILTER,         NULL,           CLI_COM_NORMAL},

    //MWF============================================================================================================//MWF
    {eCLI_MAIN_MWF,     "HORZ",     READ_COMMAND,     DECIAML_2,          edcMANUAL_WARP_HORZ_FILTER,  NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_MWF,     "HORZ",     WRITE_COMMAND,    DECIAML_2,          edcMANUAL_WARP_HORZ_FILTER,  NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_MWF,     "VERT",     READ_COMMAND,     DECIAML_2,          edcMANUAL_WARP_VERT_FILTER,  NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_MWF,     "VERT",     WRITE_COMMAND,    DECIAML_2,          edcMANUAL_WARP_VERT_FILTER,  NULL,           CLI_COM_NORMAL},

    //WRE============================================================================================================//WRE
    {eCLI_MAIN_WRE,     NULL,       READ_COMMAND,     DECIAML_2,          edcWARP_RESET,               NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_WRE,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcWARP_RESET,               NULL,           CLI_COM_NORMAL | CLI_COM_CHECK_BUSY},

    //AIM============================================================================================================//AIM
    {eCLI_MAIN_AIM,     NULL,       READ_COMMAND,     DECIAML_2,          edcAUTO_IMAGE,               NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_AIM,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcAUTO_IMAGE,               NULL,           CLI_COM_NORMAL},

    //BRT============================================================================================================//BRT
    {eCLI_MAIN_BRT,     NULL,       READ_COMMAND,     DECIAML_3,          edcBRIGHTNESS,               NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_BRT,     NULL,       WRITE_COMMAND,    DECIAML_3,          edcBRIGHTNESS,               NULL,           CLI_COM_NORMAL},

    //CON============================================================================================================//CON
    {eCLI_MAIN_CON,     NULL,       READ_COMMAND,     DECIAML_3,          edcCONTRAST,                 NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_CON,     NULL,       WRITE_COMMAND,    DECIAML_3,          edcCONTRAST,                 NULL,           CLI_COM_NORMAL},

//G100_Steven_0079 start
    //PBRI============================================================================================================//PBRI
    {eCLI_MAIN_PBRI,     NULL,       READ_COMMAND,     DECIAML_3,          edcSUB_IMAGE_BRIGHTNESS,    NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_PBRI,     NULL,       WRITE_COMMAND,    DECIAML_3,          edcSUB_IMAGE_BRIGHTNESS,    NULL,           CLI_COM_NORMAL},

    //PCON============================================================================================================//PCON
    {eCLI_MAIN_PCON,     NULL,       READ_COMMAND,     DECIAML_3,          edcSUB_IMAGE_CONTRAST,      NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_PCON,     NULL,       WRITE_COMMAND,    DECIAML_3,          edcSUB_IMAGE_CONTRAST,      NULL,           CLI_COM_NORMAL},

    //PCSP============================================================================================================//PCSP
    {eCLI_MAIN_PCSP,     NULL,       READ_COMMAND,     DECIAML_2,          edcSUB_IMAGE_COLOR_SPACE,   NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_PCSP,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcSUB_IMAGE_COLOR_SPACE,   NULL,           CLI_COM_NORMAL},
//G100_Steven_0079 end

    //CSP============================================================================================================//CSP
    {eCLI_MAIN_CSP,     NULL,       READ_COMMAND,     DECIAML_2,          edcCOLOR_SPACE,              NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_CSP,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcCOLOR_SPACE,              NULL,           CLI_COM_NORMAL},

    //DTL============================================================================================================//DTL
    {eCLI_MAIN_DTL,     NULL,       READ_COMMAND,     DECIAML_2,          edcDETAIL,                   NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_DTL,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcDETAIL,                   NULL,           CLI_COM_NORMAL},

	//COL============================================================================================================//COL //T100IR_Casper_0032
	{eCLI_MAIN_COL, 	NULL,		READ_COMMAND,	  DECIAML_2,		  edcCOLOR, 				   NULL,		   CLI_COM_NORMAL},
	{eCLI_MAIN_COL, 	NULL,		WRITE_COMMAND,	  DECIAML_2,		  edcCOLOR, 				   NULL,		   CLI_COM_NORMAL},

	//TIN============================================================================================================//TIN //T100IR_Casper_0032
	{eCLI_MAIN_TIN, 	NULL,		READ_COMMAND,	  DECIAML_2,		  edcTINT,					   NULL,		   CLI_COM_NORMAL},
	{eCLI_MAIN_TIN, 	NULL,		WRITE_COMMAND,	  DECIAML_2,		  edcTINT,					   NULL,		   CLI_COM_NORMAL},

	//TDN============================================================================================================//TDN
	{eCLI_MAIN_TDN, 	NULL,		READ_COMMAND,	  DECIAML_2,		  edc3D_MODE, 			   	   NULL,		   CLI_COM_NORMAL},
	{eCLI_MAIN_TDN, 	NULL,		WRITE_COMMAND,	  DECIAML_2,		  edc3D_MODE, 			   	   NULL,		   CLI_COM_NORMAL},

    //TDE============================================================================================================//TDE
    {eCLI_MAIN_TDE,     NULL,       READ_COMMAND,     DECIAML_2,          edc3D_ENABLE,                NULL,           CLI_COM_NORMAL},  //T100_Simon_0044
    {eCLI_MAIN_TDE,     NULL,       WRITE_COMMAND,    DECIAML_2,          edc3D_ENABLE,                NULL,           CLI_COM_NORMAL},  //T100_Simon_0044

	//TDD============================================================================================================//TDD
	{eCLI_MAIN_TDD, 	NULL,		READ_COMMAND,	  DECIAML_2,		  edc3D_2D, 			   	   NULL,		   CLI_COM_NORMAL},
	{eCLI_MAIN_TDD, 	NULL,		WRITE_COMMAND,	  DECIAML_2,		  edc3D_2D, 			   	   NULL,		   CLI_COM_NORMAL},

    //TDI============================================================================================================//TDI
    {eCLI_MAIN_TDI,     NULL,       READ_COMMAND,     DECIAML_2,          edc3D_INVERT,                NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_TDI,     NULL,       WRITE_COMMAND,    DECIAML_2,          edc3D_INVERT,                NULL,           CLI_COM_NORMAL},

    //SIS============================================================================================================//SIS //A35G2_BRC_Casper_0016
    {eCLI_MAIN_SIS,     NULL,       READ_COMMAND,     DECIAML_2,          edc3D_SYNC_TYPE,             NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_SIS,     NULL,       WRITE_COMMAND,    DECIAML_2,          edc3D_SYNC_TYPE,             NULL,           CLI_COM_NORMAL},

    //SOS============================================================================================================//SOS
    {eCLI_MAIN_SOS,     NULL,       READ_COMMAND,     DECIAML_2,          edc3D_SYNC_OUT,              NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_SOS,     NULL,       WRITE_COMMAND,    DECIAML_2,          edc3D_SYNC_OUT,              NULL,           CLI_COM_NORMAL},

    //FDY============================================================================================================//FDY
    {eCLI_MAIN_FDY,     NULL,       READ_COMMAND,     DECIAML_2,          edcFRAME_DELAY,              NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_FDY,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcFRAME_DELAY,              NULL,           CLI_COM_NORMAL},

    //LRR============================================================================================================//LRR //T100_Casper_0062
    {eCLI_MAIN_LRR,     NULL,       WRITE_COMMAND,    DECIAML_2,          edc3D_LR_REFERENCE,          NULL,        CLI_COM_NORMAL },
    {eCLI_MAIN_LRR,     NULL,       READ_COMMAND,     DECIAML_2,          edc3D_LR_REFERENCE,          NULL,        CLI_COM_NORMAL },

    //TDDL============================================================================================================//TDDL //G00_Wilsonj_0032
    {eCLI_MAIN_TDDL,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcDLPLink_ON,               NULL,        CLI_COM_NORMAL },
    {eCLI_MAIN_TDDL,    NULL,       READ_COMMAND,     DECIAML_2,          edcDLPLink_ON,               NULL,        CLI_COM_NORMAL },

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

    //SYT============================================================================================================//SYT
  //  {eCLI_MAIN_SYT,     NULL,       READ_COMMAND,     DECIAML_3,          edcINPUT_LEVEL,              NULL,           CLI_COM_NORMAL},
  //  {eCLI_MAIN_SYT,     NULL,       WRITE_COMMAND,    DECIAML_3,          edcINPUT_LEVEL,              NULL,           CLI_COM_NORMAL},

    //GOR============================================================================================================//GOR
    {eCLI_MAIN_GOR,     NULL,       READ_COMMAND,     DECIAML_3,          edcWHITE_BALANCE_RESET,      NULL,           CLI_COM_WRITE_ONLY},
    {eCLI_MAIN_GOR,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcWHITE_BALANCE_RESET,      NULL,           CLI_COM_WRITE_ONLY},

    //DPMO============================================================================================================//DPMO
    {eCLI_MAIN_DPMO,    NULL,       READ_COMMAND,     DECIAML_2,          edcPICTURE_SETTINGS,         NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_DPMO,    NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,    utilGeneral_DPMO,           CLI_COM_NORMAL},

    //DPSU============================================================================================================//DPSU  //G00_Wilsonj_0032
    {eCLI_MAIN_DPSU,    NULL,       READ_COMMAND,     DECIAML_2,          edcSAVE_TO_USER,             NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_DPSU,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcSAVE_TO_USER,             NULL,           CLI_COM_NORMAL},

    //DPAU============================================================================================================//DPAU //G100_Steven_0030
    {eCLI_MAIN_DPAU,    NULL,       READ_COMMAND,     DECIAML_2,          edcUSER_COLOR_MODE,          NULL,            CLI_COM_NORMAL},
    {eCLI_MAIN_DPAU,    NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,    utilGeneral_DPAU,            CLI_COM_NORMAL},	//G100_Doulas_0068 Modify

    {eCLI_MAIN_FPS,     NULL,       READ_COMMAND,     DECIAML_2,          edcFORCE_PICTURE_SETTINGS,   NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_FPS,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcFORCE_PICTURE_SETTINGS,   NULL,           CLI_COM_NORMAL},

    //DIM============================================================================================================//DIM
    {eCLI_MAIN_DIM,     NULL,       READ_COMMAND,     DECIAML_2,          edcCONTRAST_ENHANCEMENT,     NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_DIM,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcCONTRAST_ENHANCEMENT,     NULL,           CLI_COM_NORMAL},

    //FRZ============================================================================================================//FRZ
    {eCLI_MAIN_FRZ,     NULL,       READ_COMMAND,     DECIAML_2,          edcIMAGE_FREEZE,             NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_FRZ,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcIMAGE_FREEZE,             NULL,           CLI_COM_NORMAL}, //A35G2_CDS_Larry_0035

    //BGC============================================================================================================//BGC
    {eCLI_MAIN_BGC,     NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,                utilGeneral_GAMM,           CLI_COM_NORMAL},
    {eCLI_MAIN_BGC,     NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,                utilGeneral_GAMM,CLI_COM_NORMAL},    //G100_Simon_0066

    //WPK============================================================================================================//WPK
    {eCLI_MAIN_WHPK,    NULL,       READ_COMMAND,     DECIAML_2,          edcWHITE_PEAKING,            NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_WHPK,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcWHITE_PEAKING,            NULL,           CLI_COM_NORMAL},

    //CCI============================================================================================================//CCI
    {eCLI_MAIN_CCI,     NULL,       READ_COMMAND,     DECIAML_2,          edcCOLOR_TEMPERATURE,        NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_CCI,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcCOLOR_TEMPERATURE,        NULL,           CLI_COM_NORMAL},

    //LOT============================================================================================================//LOT //T100_Casper_0062
    {eCLI_MAIN_LOT,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcLIGHTS_OUT_TIMER_X05,         NULL,        CLI_COM_NORMAL },   //A70LV_Doulas_0228
    {eCLI_MAIN_LOT,     NULL,       READ_COMMAND,     DECIAML_2,          edcLIGHTS_OUT_TIMER_X05,         NULL,        CLI_COM_NORMAL },   //A70LV_Doulas_0228

    //LOL============================================================================================================//LOL //T100_Casper_0062
    {eCLI_MAIN_LOL,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcLIGHTS_OUT_SIGNAL_LEVEL,      NULL,        CLI_COM_NORMAL },   //A70LV_Doulas_0228
    {eCLI_MAIN_LOL,     NULL,       READ_COMMAND,     DECIAML_2,          edcLIGHTS_OUT_SIGNAL_LEVEL,      NULL,        CLI_COM_NORMAL },   //A70LV_Doulas_0228

    //CWS============================================================================================================//CWS
    //{eCLI_MAIN_CWS,     NULL,       READ_COMMAND,     DECIAML_2,          edcCOLOR_WHEEL_SPEED,        NULL,           CLI_COM_NORMAL},
    //{eCLI_MAIN_CWS,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcCOLOR_WHEEL_SPEED,        NULL,           CLI_COM_NORMAL},

    //LOC============================================================================================================//LOC
    {eCLI_MAIN_LOC,     NULL,     READ_COMMAND,     DECIAML_2,        	  edcLANGUAGE,                 NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_LOC,     NULL,     WRITE_COMMAND,    DECIAML_2,            edcLANGUAGE,                 NULL,           CLI_COM_NORMAL},
	/*
	//LOC============================================================================================================//LOC
	{eCLI_MAIN_LOC, 	"LANG", 	READ_COMMAND,	  DECIAML_2,		  edcLANGUAGE,				   NULL,		   CLI_COM_NORMAL},
	{eCLI_MAIN_LOC, 	"LANG", 	WRITE_COMMAND,	  DECIAML_2,		  edcLANGUAGE,				   NULL,		   CLI_COM_NORMAL},
	*/
	//LET============================================================================================================//LET //T100_Casper_0063
	{eCLI_MAIN_LET, 	NULL,		WRITE_COMMAND,	  DECIAML_2,		  edcLENS_TYPE, 				   NULL,		CLI_COM_READ_ONLY },
	{eCLI_MAIN_LET, 	NULL,		READ_COMMAND,	  DECIAML_2,		  edcLENS_TYPE, 				   NULL,		CLI_COM_READ_ONLY },

    //FCS============================================================================================================//FCS
    //{eCLI_MAIN_FCS,     NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,        utilGeneral_FCS,          CLI_COM_LENS},
    //{eCLI_MAIN_FCS,     NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,        utilGeneral_FCS,          CLI_COM_LENS},

    //FCSI============================================================================================================//FCSI //G100_Wilsonj_0035
    {eCLI_MAIN_FCSI,    NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,        utilGeneral_FCSI,          CLI_COM_LENS},
    {eCLI_MAIN_FCSI,    NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,        utilGeneral_FCSI,          CLI_COM_LENS},

    //FCSO============================================================================================================//FCSO //G100_Wilsonj_0035
    {eCLI_MAIN_FCSO,    NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,        utilGeneral_FCSO,          CLI_COM_LENS},
    {eCLI_MAIN_FCSO,    NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,        utilGeneral_FCSO,          CLI_COM_LENS},

    //ZOM============================================================================================================//ZOM
    //{eCLI_MAIN_ZOM,     NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,        utilGeneral_ZOM,          CLI_COM_LENS},
    //{eCLI_MAIN_ZOM,     NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,        utilGeneral_ZOM,          CLI_COM_LENS},

    //ZOMI============================================================================================================//ZOMI //G100_Wilsonj_0035
    {eCLI_MAIN_ZOMI,    NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,        utilGeneral_ZOMI,          CLI_COM_LENS},
    {eCLI_MAIN_ZOMI,    NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,        utilGeneral_ZOMI,          CLI_COM_LENS},

    //ZOMO============================================================================================================//ZOMO //G100_Wilsonj_0035
    {eCLI_MAIN_ZOMO,    NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,        utilGeneral_ZOMO,          CLI_COM_LENS},
    {eCLI_MAIN_ZOMO,    NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,        utilGeneral_ZOMO,          CLI_COM_LENS},

     //LVO============================================================================================================//LVO
    //{eCLI_MAIN_LVO,     NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,        utilGeneral_LVO,          CLI_COM_LENS},
    //{eCLI_MAIN_LVO,     NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,        utilGeneral_LVO,          CLI_COM_LENS},

     //LHO============================================================================================================//LHO
    //{eCLI_MAIN_LHO,     NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,        utilGeneral_LHO,          CLI_COM_LENS},
    //{eCLI_MAIN_LHO,     NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,        utilGeneral_LHO,          CLI_COM_LENS},

	//LVU============================================================================================================//LVU
	{eCLI_MAIN_LVU, 	NULL,		READ_COMMAND,	  DECIAML_2,		  DATA_CODE_NA,		 utilGeneral_LVU,			CLI_COM_NORMAL | CLI_COM_WRITE_ONLY},
	{eCLI_MAIN_LVU, 	NULL,		WRITE_COMMAND,	  DECIAML_2,		  DATA_CODE_NA,		 utilGeneral_LVU,			CLI_COM_NORMAL | CLI_COM_WRITE_ONLY},

	 //LVD============================================================================================================//LVD //T100_Casper_0099
	{eCLI_MAIN_LVD, 	NULL,		READ_COMMAND,	  DECIAML_2,		  DATA_CODE_NA,		 utilGeneral_LVD,			CLI_COM_NORMAL | CLI_COM_WRITE_ONLY},
	{eCLI_MAIN_LVD, 	NULL,		WRITE_COMMAND,	  DECIAML_2,		  DATA_CODE_NA,		 utilGeneral_LVD,			CLI_COM_NORMAL | CLI_COM_WRITE_ONLY},

	 //LHL============================================================================================================//LHL //T100_Casper_0099
	{eCLI_MAIN_LHL, 	NULL,		READ_COMMAND,	  DECIAML_2,		  DATA_CODE_NA,		 utilGeneral_LHL,			CLI_COM_NORMAL | CLI_COM_WRITE_ONLY},
	{eCLI_MAIN_LHL, 	NULL,		WRITE_COMMAND,	  DECIAML_2,		  DATA_CODE_NA,		 utilGeneral_LHL,			CLI_COM_NORMAL | CLI_COM_WRITE_ONLY},

	 //LHR============================================================================================================//LHR //T100_Casper_0099
	{eCLI_MAIN_LHR, 	NULL,		READ_COMMAND,	  DECIAML_2,		  DATA_CODE_NA,		 utilGeneral_LHR,			CLI_COM_NORMAL | CLI_COM_WRITE_ONLY},
	{eCLI_MAIN_LHR, 	NULL,		WRITE_COMMAND,	  DECIAML_2,		  DATA_CODE_NA,		 utilGeneral_LHR,			CLI_COM_NORMAL | CLI_COM_WRITE_ONLY},

     //LMA============================================================================================================//LMA
    {eCLI_MAIN_LMA,     NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,      utilGeneral_LMAP,          CLI_COM_NORMAL}, //A35G2_BRC_Casper_0093
    {eCLI_MAIN_LMA,     NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,      utilGeneral_LMAP,          CLI_COM_NORMAL}, //A35G2_BRC_Casper_0093

     //LMS============================================================================================================//LMS
    {eCLI_MAIN_LMS,     NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,      utilGeneral_LMSP,          CLI_COM_NORMAL}, //A35G2_BRC_Casper_0093
    {eCLI_MAIN_LMS,     NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,      utilGeneral_LMSP,          CLI_COM_NORMAL}, //A35G2_BRC_Casper_0093

     //LLK============================================================================================================//LLK
    {eCLI_MAIN_LLK,     NULL,       READ_COMMAND,     DECIAML_2,          edcLOCK_ALL_LENS_MOTORS,      NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_LLK,     NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,      utilGeneral_LELO,          CLI_COM_NORMAL}, //A35G2_CDS_Larry_0035

     //LCB============================================================================================================//LCB
    {eCLI_MAIN_LCB,     NULL,       READ_COMMAND,     DECIAML_2,          edcLENS_CALIBRATION,          NULL,           CLI_COM_WRITE_ONLY},
    {eCLI_MAIN_LCB,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcLENS_CALIBRATION,          NULL,           CLI_COM_WRITE_ONLY},

     //CEL============================================================================================================//CEL
    {eCLI_MAIN_CEL,     NULL,       READ_COMMAND,     DECIAML_2,          edcCEILING_MOUNT,             NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_CEL,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcCEILING_MOUNT,             NULL,           CLI_COM_NORMAL},

     //SOR============================================================================================================//SOR
    {eCLI_MAIN_SOR,     NULL,       READ_COMMAND,     DECIAML_2,          edcREAR_PROJECTION,           NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_SOR,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcREAR_PROJECTION,           NULL,           CLI_COM_NORMAL},

     //MSH============================================================================================================//MSH
    {eCLI_MAIN_MSH,     NULL,       READ_COMMAND,     DECIAML_3,          edcMEMU_HORZ_OFFSET,          NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_MSH,     NULL,       WRITE_COMMAND,    DECIAML_3,          edcMEMU_HORZ_OFFSET,          NULL,           CLI_COM_NORMAL},

    //MSV============================================================================================================//MSV
    {eCLI_MAIN_MSV,     NULL,       READ_COMMAND,     DECIAML_3,          edcMEMU_VERT_OFFSET,          NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_MSV,     NULL,       WRITE_COMMAND,    DECIAML_3,          edcMEMU_VERT_OFFSET,          NULL,          CLI_COM_NORMAL},

	//MBE============================================================================================================//MBE
	{eCLI_MAIN_MBE, 	NULL, 	READ_COMMAND,	  DECIAML_2,		  edcSHOW_MESSAGES, 			NULL,		   CLI_COM_NORMAL},
	{eCLI_MAIN_MBE, 	NULL, 	WRITE_COMMAND,	  DECIAML_2,		  edcSHOW_MESSAGES, 			NULL,		   CLI_COM_NORMAL},
/*
    //MBE============================================================================================================//MBE
    {eCLI_MAIN_MBE,     "USER",     READ_COMMAND,     DECIAML_3,          edcSHOW_MESSAGES,             NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_MBE,     "USER",     WRITE_COMMAND,    DECIAML_3,          edcSHOW_MESSAGES,             NULL,          CLI_COM_NORMAL},
*/
    //MLT============================================================================================================//MLT //T100IR_Casper_0032
	{eCLI_MAIN_MLT, 	NULL, 		READ_COMMAND,	  DECIAML_2,		  edcMENU_LOCATION, 			NULL,		   CLI_COM_NORMAL},
	{eCLI_MAIN_MLT, 	NULL, 		WRITE_COMMAND,	  DECIAML_2,		  edcMENU_LOCATION, 			NULL,		   CLI_COM_NORMAL},

	//OST============================================================================================================//OST
    {eCLI_MAIN_OST,     NULL,       READ_COMMAND,     DECIAML_3,          edcMENU_TRANSPARENCY,         NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_OST,     NULL,       WRITE_COMMAND,    DECIAML_3,          edcMENU_TRANSPARENCY,         NULL,          CLI_COM_NORMAL},

    //SPS============================================================================================================//SPS
    {eCLI_MAIN_SPSG,    NULL,     READ_COMMAND,     DECIAML_2,            edcBACKGROUND_COLOR ,         NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_SPSG,    NULL,     WRITE_COMMAND,    DECIAML_2,            edcBACKGROUND_COLOR,          NULL,          CLI_COM_NORMAL},

    //BGCL============================================================================================================//BGCL
    {eCLI_MAIN_BGCL,    NULL,     READ_COMMAND,     DECIAML_2,            edcBACKGROUND_COLOR,            NULL,          CLI_COM_NORMAL}, //G100_Coda_0050
    {eCLI_MAIN_BGCL,    NULL,     WRITE_COMMAND,    DECIAML_2,            edcBACKGROUND_COLOR,            NULL,          CLI_COM_NORMAL},

    //BGRT============================================================================================================//BGRT
    {eCLI_MAIN_BGRT,    NULL,     READ_COMMAND,     DECIAML_2,            edcON_SCREEN_DISPLAY_RESET,   NULL,          CLI_COM_WRITE_ONLY}, //G100_Coda_0054
    {eCLI_MAIN_BGRT,    NULL,     WRITE_COMMAND,    DECIAML_2,            edcON_SCREEN_DISPLAY_RESET,   NULL,          CLI_COM_WRITE_ONLY | CLI_COM_CHECK_BUSY}, //G100_Coda_0054

	/*
	//SPS============================================================================================================//SPS
	{eCLI_MAIN_SPS, 	"SLCT", 	READ_COMMAND,	  DECIAML_2,		  edcSPLASH_STARTUP,			NULL,		   CLI_COM_NORMAL},
	{eCLI_MAIN_SPS, 	"SLCT", 	WRITE_COMMAND,	  DECIAML_2,		  edcSPLASH_STARTUP,			NULL,		   CLI_COM_NORMAL},
*/
    //PIV============================================================================================================//PIV
    {eCLI_MAIN_PINP,     NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,           utilGeneral_PIV,      CLI_COM_NORMAL},
    {eCLI_MAIN_PINP,     NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,           utilGeneral_PIV,      CLI_COM_NORMAL},

    //PCG============================================================================================================//PCG
    {eCLI_MAIN_PINC,     NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,           utilGeneral_PCG,      CLI_COM_WRITE_ONLY},
    {eCLI_MAIN_PINC,     NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,           utilGeneral_PCG,      CLI_COM_WRITE_ONLY},

    //PWR============================================================================================================//PWR
    {eCLI_MAIN_PWR,     NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,       utilGeneral_PWR,          CLI_COM_NORMAL},
    {eCLI_MAIN_PWR,     NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,       utilGeneral_PWR,          CLI_COM_NORMAL},
    {eCLI_MAIN_PWR,     "SLEP",     READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,   utilGeneral_PWRSLEP,          CLI_COM_NORMAL},
    {eCLI_MAIN_PWR,     "SLEP",     WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,   utilGeneral_PWRSLEP,          CLI_COM_NORMAL},

    //SBPM============================================================================================================//SBPM //G100_Wilsonj_0041
    {eCLI_MAIN_SBPM,    NULL,       READ_COMMAND,     DECIAML_2,          edcSTANDBY_MODE,              NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_SBPM,    NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,   utilGeneral_StandbyMode,      CLI_COM_NORMAL|CLI_COM_NO_REPLY}, //G100_Steven_0072

    //APW============================================================================================================//APW
    {eCLI_MAIN_APW,     NULL,       READ_COMMAND,     DECIAML_2,          edcAC_POWER_ON,               NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_APW,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcAC_POWER_ON,               NULL,          CLI_COM_NORMAL},

    //ASH============================================================================================================//ASH
    {eCLI_MAIN_ASH,     NULL,       READ_COMMAND,     DECIAML_2,          edcAUTO_SHUTDOWN,             NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_ASH,     NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,     utilGeneral_AutoShutDown,   CLI_COM_NORMAL}, //A35G2_BRC_Casper_0111

    //SLP============================================================================================================//SLP
    {eCLI_MAIN_SLP,     NULL,       READ_COMMAND,     DECIAML_2,          edcSLEEP_TIMER,               NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_SLP,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcSLEEP_TIMER,               NULL,          CLI_COM_NORMAL},

    //HAT============================================================================================================//HAT
    {eCLI_MAIN_HIAL,    NULL,       READ_COMMAND,     DECIAML_2,          edcHIGH_ALTITUDE,             NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_HIAL,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcHIGH_ALTITUDE,             NULL,          CLI_COM_NORMAL},

    //FRNT============================================================================================================//FRNT //G100_Wilsonj_0035
    {eCLI_MAIN_FRNT,    NULL,       READ_COMMAND,     DECIAML_2,          edcFRONT_IR,                  NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_FRNT,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcFRONT_IR,                  NULL,          CLI_COM_NORMAL},

    //TOPP============================================================================================================//TOPP //G100_Wilsonj_0035
    {eCLI_MAIN_TOPP,    NULL,       READ_COMMAND,     DECIAML_2,          edcTOP_IR,                    NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_TOPP,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcTOP_IR,                    NULL,          CLI_COM_NORMAL},

    //HDBT============================================================================================================//HDBT //G100_Wilsonj_0035
    {eCLI_MAIN_HDBT,    NULL,       READ_COMMAND,     DECIAML_2,          edcHDBASET_IR,                NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_HDBT,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcHDBASET_IR,                NULL,          CLI_COM_NORMAL},

    //VTT============================================================================================================//VTT
    {eCLI_MAIN_VTT,     NULL,       READ_COMMAND,     DECIAML_2,          edc12V_TRIGGER,               NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_VTT,     NULL,       WRITE_COMMAND,    DECIAML_2,          edc12V_TRIGGER,               NULL,          CLI_COM_NORMAL},

    //NET============================================================================================================//NET
    {eCLI_MAIN_NET,     "HOST",     READ_COMMAND,     DECIAML_2,          edcPROJECTOR_NAME,            NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_NET,     "HOST",     WRITE_COMMAND,    DECIAML_2,          edcPROJECTOR_NAME,            NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_NET,     "SHOW",     READ_COMMAND,     DECIAML_2,          edcSHOW_NETWORK_MESSAGE,      NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_NET,     "SHOW",     WRITE_COMMAND,    DECIAML_2,          edcSHOW_NETWORK_MESSAGE,      NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_NET,     "RSET",     READ_COMMAND,     DECIAML_2,          edcNETWORK_FACTORY_RESET,     NULL,          CLI_COM_WRITE_ONLY},
    {eCLI_MAIN_NET,     "RSET",     WRITE_COMMAND,    DECIAML_2,          edcNETWORK_FACTORY_RESET,     NULL,          CLI_COM_WRITE_ONLY},
    {eCLI_MAIN_NET,     "SDNS",     READ_COMMAND,     DECIAML_2,          edcLAN_SECOND_DNS,            NULL,          CLI_COM_NORMAL | CLI_COM_IGNORE_CTRL_CHECK},
    {eCLI_MAIN_NET,     "SDNS",     WRITE_COMMAND,    DECIAML_2,          edcLAN_SECOND_DNS,            NULL,          CLI_COM_NORMAL},

    //LMAC============================================================================================================//LMAC //G100_Wilsonj_0035
    {eCLI_MAIN_LMAC,    NULL,       READ_COMMAND,     DECIAML_2,          edcLAN_MAC_ADDRESS,           NULL,          CLI_COM_NORMAL | CLI_COM_IGNORE_CTRL_CHECK}, //G100_Coda_0049
    {eCLI_MAIN_LMAC,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcLAN_MAC_ADDRESS,           NULL,          CLI_COM_READ_ONLY},

    //LDHC============================================================================================================//LDHC //G100_Wilsonj_0035
    {eCLI_MAIN_LDHC,    NULL,       READ_COMMAND,     DECIAML_2,          edcLAN_DHCP,                  NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_LDHC,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcLAN_DHCP,                  NULL,          CLI_COM_NORMAL},

    //LIPA============================================================================================================//LIPA //G100_Wilsonj_0035
    {eCLI_MAIN_LIPA,    NULL,       READ_COMMAND,     DECIAML_2,          edcLAN_IP_ADDRESS,            NULL,          CLI_COM_NORMAL | CLI_COM_IGNORE_CTRL_CHECK},
    {eCLI_MAIN_LIPA,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcLAN_IP_ADDRESS,            NULL,          CLI_COM_NORMAL},

    //LSUB============================================================================================================//LSUB //G100_Wilsonj_0035
    {eCLI_MAIN_LSUB,    NULL,       READ_COMMAND,     DECIAML_2,          edcLAN_SUBNET_MASK,           NULL,          CLI_COM_NORMAL | CLI_COM_IGNORE_CTRL_CHECK},
    {eCLI_MAIN_LSUB,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcLAN_SUBNET_MASK,           NULL,          CLI_COM_NORMAL},

    //LGAT============================================================================================================//LGAT //G100_Wilsonj_0035
    {eCLI_MAIN_LGAT,    NULL,       READ_COMMAND,     DECIAML_2,          edcLAN_DEFAULT_GATEWAY,       NULL,          CLI_COM_NORMAL | CLI_COM_IGNORE_CTRL_CHECK},
    {eCLI_MAIN_LGAT,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcLAN_DEFAULT_GATEWAY,       NULL,          CLI_COM_NORMAL},

    //DNSG============================================================================================================//DNSG //G100_Wilsonj_0035
    {eCLI_MAIN_DNSG,    NULL,       READ_COMMAND,     DECIAML_2,          edcLAN_PRIMARY_DNS,           NULL,          CLI_COM_NORMAL | CLI_COM_IGNORE_CTRL_CHECK},
    {eCLI_MAIN_DNSG,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcLAN_PRIMARY_DNS,           NULL,          CLI_COM_NORMAL},

    //LAPY============================================================================================================//LAPY //G100_Wilsonj_0035
    {eCLI_MAIN_LAPY,    NULL,       READ_COMMAND,     DECIAML_2,          edcLAN_APPLY,                 NULL,          CLI_COM_WRITE_ONLY},
    {eCLI_MAIN_LAPY,    NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,      utilGeneral_NetWorkApply,        CLI_COM_NORMAL|CLI_COM_NO_REPLY}, //G100_Steven_0072

	/*
    //NTW============================================================================================================//NTW
    {eCLI_MAIN_NTW,     "SLCT",     READ_COMMAND,     DECIAML_2,          edcWLAN_ENABLE,               NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_NTW,     "SLCT",     WRITE_COMMAND,    DECIAML_2,          edcWLAN_ENABLE,               NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_NTW,     "ETH0",     READ_COMMAND,     DECIAML_2,          edcWLAN_START_IP,             NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_NTW,     "ETH0",     WRITE_COMMAND,    DECIAML_2,          edcWLAN_START_IP,             NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_NTW,     "ENIP",     READ_COMMAND,     DECIAML_2,          edcWLAN_END_IP,               NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_NTW,     "ENIP",     WRITE_COMMAND,    DECIAML_2,          edcWLAN_END_IP,               NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_NTW,     "SUB0",     READ_COMMAND,     DECIAML_2,          edcWLAN_SUBNET_MASK,          NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_NTW,     "SUB0",     WRITE_COMMAND,    DECIAML_2,          edcWLAN_SUBNET_MASK,          NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_NTW,     "GATE",     READ_COMMAND,     DECIAML_2,          edcWLAN_DEFAULT_GATEWAY,      NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_NTW,     "GATE",     WRITE_COMMAND,    DECIAML_2,          edcWLAN_DEFAULT_GATEWAY,      NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_NTW,     "MAC0",     READ_COMMAND,     DECIAML_2,          edcWLAN_MAC_ADDRESS,          NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_NTW,     "MAC0",     WRITE_COMMAND,    DECIAML_2,          edcWLAN_MAC_ADDRESS,          NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_NTW,     "SSID",     READ_COMMAND,     DECIAML_2,          edcWLAN_SSID,                 NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_NTW,     "SSID",     WRITE_COMMAND,    DECIAML_2,          edcWLAN_SSID,                 NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_NTW,     "SETT",     WRITE_COMMAND,    DECIAML_2,          edcWLAN_APPLY,                NULL,          CLI_COM_WRITE_ONLY},
	*/

    //BDR============================================================================================================//BDR
    {eCLI_MAIN_BDR,     NULL,       READ_COMMAND,     DECIAML_2,          edcSERIAL_PORT_BAUD_RATE,     NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_BDR,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcSERIAL_PORT_BAUD_RATE,     NULL,          CLI_COM_NORMAL},

    //SEC============================================================================================================//SEC
    {eCLI_MAIN_SEC,     NULL,       READ_COMMAND,     DECIAML_2,          edcSERIAL_PORT_ECHO,          NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_SEC,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcSERIAL_PORT_ECHO,          NULL,          CLI_COM_NORMAL},

    //SPP============================================================================================================//SPP
    {eCLI_MAIN_SPP,     NULL,       READ_COMMAND,     DECIAML_2,          edcSERIAL_PORT_PATH,          NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_SPP,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcSERIAL_PORT_PATH,          NULL,          CLI_COM_NORMAL},

    //SLA============================================================================================================//SPP
    {eCLI_MAIN_SLA,     NULL,       READ_COMMAND,     DECIAML_2,          edcSLOT_LAN_PATH,             NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_SLA,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcSLOT_LAN_PATH,             NULL,          CLI_COM_NORMAL},

	//PJP============================================================================================================//PJP
	{eCLI_MAIN_PJP, 	NULL,		READ_COMMAND,	  DECIAML_2,		  edcPROJECTOR_ID, 				NULL,		   CLI_COM_NORMAL},
	{eCLI_MAIN_PJP, 	NULL,		WRITE_COMMAND,	  DECIAML_2,		  edcPROJECTOR_ID, 				NULL,		   CLI_COM_NORMAL},

    //ADR============================================================================================================//ADR
    {eCLI_MAIN_ADR,     NULL,       READ_COMMAND,     DECIAML_2,          edcPROJECTOR_ADDRESS,         NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_ADR,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcPROJECTOR_ADDRESS,         NULL,          CLI_COM_NORMAL},

    //KBL============================================================================================================//KBL
    {eCLI_MAIN_KBL,     NULL,       READ_COMMAND,     DECIAML_2,          edcKEYPAD_BACKLIGHT,          NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_KBL,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcKEYPAD_BACKLIGHT,          NULL,          CLI_COM_NORMAL},

    //SBL============================================================================================================//SBL
    {eCLI_MAIN_SBL,     NULL,       READ_COMMAND,     DECIAML_2,          edcSTATUS_LED,                NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_SBL,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcSTATUS_LED,                NULL,          CLI_COM_NORMAL},

    //CCA============================================================================================================//CCA
    {eCLI_MAIN_HGWP,    NULL,       READ_COMMAND,     DECIAML_3,          edcHSG_AUTO_TEST_PATTERN,     NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_HGWP,    NULL,       WRITE_COMMAND,    DECIAML_3,          edcHSG_AUTO_TEST_PATTERN,     NULL,          CLI_COM_NORMAL},

    //HGR============================================================================================================//HGR
    {eCLI_MAIN_HGRH,    NULL,       READ_COMMAND,     DECIAML_3,          edcHSG_RED_HUE,               NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_HGRH,    NULL,       WRITE_COMMAND,    DECIAML_3,          edcHSG_RED_HUE,               NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_HGRS,    NULL,       READ_COMMAND,     DECIAML_3,          edcHSG_RED_SATURATION,        NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_HGRS,    NULL,       WRITE_COMMAND,    DECIAML_3,          edcHSG_RED_SATURATION,        NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_HGRG,    NULL,       READ_COMMAND,     DECIAML_3,          edcHSG_RED_GAIN,              NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_HGRG,    NULL,       WRITE_COMMAND,    DECIAML_3,          edcHSG_RED_GAIN,              NULL,          CLI_COM_NORMAL},

    //HGG============================================================================================================//HGG
    {eCLI_MAIN_HGGH,    NULL,       READ_COMMAND,     DECIAML_3,          edcHSG_GREEN_HUE,             NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_HGGH,    NULL,       WRITE_COMMAND,    DECIAML_3,          edcHSG_GREEN_HUE,             NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_HGGS,    NULL,       READ_COMMAND,     DECIAML_3,          edcHSG_GREEN_SATURATION,      NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_HGGS,    NULL,       WRITE_COMMAND,    DECIAML_3,          edcHSG_GREEN_SATURATION,      NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_HGGG,    NULL,       READ_COMMAND,     DECIAML_3,          edcHSG_GREEN_GAIN,            NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_HGGG,    NULL,       WRITE_COMMAND,    DECIAML_3,          edcHSG_GREEN_GAIN,            NULL,          CLI_COM_NORMAL},

    //HGB============================================================================================================//HGB
    {eCLI_MAIN_HGBH,    NULL,       READ_COMMAND,     DECIAML_3,          edcHSG_BLUE_HUE,              NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_HGBH,    NULL,       WRITE_COMMAND,    DECIAML_3,          edcHSG_BLUE_HUE,              NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_HGBS,    NULL,       READ_COMMAND,     DECIAML_3,          edcHSG_BLUE_SATURATION,       NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_HGBS,    NULL,       WRITE_COMMAND,    DECIAML_3,          edcHSG_BLUE_SATURATION,       NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_HGBG,    NULL,       READ_COMMAND,     DECIAML_3,          edcHSG_BLUE_GAIN,             NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_HGBG,    NULL,       WRITE_COMMAND,    DECIAML_3,          edcHSG_BLUE_GAIN,             NULL,          CLI_COM_NORMAL},

    //HGC============================================================================================================//HGC
    {eCLI_MAIN_HGCH,    NULL,       READ_COMMAND,     DECIAML_3,          edcHSG_CYAN_HUE,              NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_HGCH,    NULL,       WRITE_COMMAND,    DECIAML_3,          edcHSG_CYAN_HUE,              NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_HGCS,    NULL,       READ_COMMAND,     DECIAML_3,          edcHSG_CYAN_SATURATION,       NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_HGCS,    NULL,       WRITE_COMMAND,    DECIAML_3,          edcHSG_CYAN_SATURATION,       NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_HGCG,    NULL,       READ_COMMAND,     DECIAML_3,          edcHSG_CYAN_GAIN,             NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_HGCG,    NULL,       WRITE_COMMAND,    DECIAML_3,          edcHSG_CYAN_GAIN,             NULL,          CLI_COM_NORMAL},

    //HGM============================================================================================================//HGM
    {eCLI_MAIN_HGMH,    NULL,       READ_COMMAND,     DECIAML_3,          edcHSG_MAGENTA_HUE,           NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_HGMH,    NULL,       WRITE_COMMAND,    DECIAML_3,          edcHSG_MAGENTA_HUE,           NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_HGMS,    NULL,       READ_COMMAND,     DECIAML_3,          edcHSG_MAGENTA_SATURATION,    NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_HGMS,    NULL,       WRITE_COMMAND,    DECIAML_3,          edcHSG_MAGENTA_SATURATION,    NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_HGMG,    NULL,       READ_COMMAND,     DECIAML_3,          edcHSG_MAGENTA_GAIN,          NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_HGMG,    NULL,       WRITE_COMMAND,    DECIAML_3,          edcHSG_MAGENTA_GAIN,          NULL,          CLI_COM_NORMAL},

    //HGY============================================================================================================//HGY
    {eCLI_MAIN_HGYH,    NULL,       READ_COMMAND,     DECIAML_3,          edcHSG_YELLOW_HUE,            NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_HGYH,    NULL,       WRITE_COMMAND,    DECIAML_3,          edcHSG_YELLOW_HUE,            NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_HGYS,    NULL,       READ_COMMAND,     DECIAML_3,          edcHSG_YELLOW_SATURATION,     NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_HGYS,    NULL,       WRITE_COMMAND,    DECIAML_3,          edcHSG_YELLOW_SATURATION,     NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_HGYG,    NULL,       READ_COMMAND,     DECIAML_3,          edcHSG_YELLOW_GAIN,           NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_HGYG,    NULL,       WRITE_COMMAND,    DECIAML_3,          edcHSG_YELLOW_GAIN,           NULL,          CLI_COM_NORMAL},

    //HSW============================================================================================================//HSW
    {eCLI_MAIN_HGWR,    NULL,       READ_COMMAND,     DECIAML_3,          edcHSG_WHITE_GAIN_RED,        NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_HGWR,    NULL,       WRITE_COMMAND,    DECIAML_3,          edcHSG_WHITE_GAIN_RED,        NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_HGWG,    NULL,       READ_COMMAND,     DECIAML_3,          edcHSG_WHITE_GAIN_GREEN,      NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_HGWG,    NULL,       WRITE_COMMAND,    DECIAML_3,          edcHSG_WHITE_GAIN_GREEN,      NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_HGWB,    NULL,       READ_COMMAND,     DECIAML_3,          edcHSG_WHITE_GAIN_BLUE,       NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_HGWB,    NULL,       WRITE_COMMAND,    DECIAML_3,          edcHSG_WHITE_GAIN_BLUE,       NULL,          CLI_COM_NORMAL},

    //HSRE============================================================================================================//HSRE
	//{eCLI_MAIN_HSRE, 	NULL, 	    READ_COMMAND,	  DECIAML_2,	  edcHSG_WHITE_GAIN_RESET_DEFAULT,	NULL,		   CLI_COM_WRITE_ONLY},  //A70LV_Doulas_0126 modify
	//{eCLI_MAIN_HSRE, 	NULL, 	    WRITE_COMMAND,	  DECIAML_2,	  edcHSG_WHITE_GAIN_RESET_DEFAULT,	NULL,		   CLI_COM_WRITE_ONLY},  //A70LV_Doulas_0126 modify

	{eCLI_MAIN_CCAG, 	"RRTD", 	READ_COMMAND,	  DECIAML_2,		  edcHSG_RED_RESET_DEFAULT, 	NULL,		   CLI_COM_WRITE_ONLY},  //A70LV_Doulas_0133
	{eCLI_MAIN_CCAG, 	"RRTD", 	WRITE_COMMAND,	  DECIAML_2,		  edcHSG_RED_RESET_DEFAULT, 	NULL,		   CLI_COM_WRITE_ONLY | CLI_COM_CHECK_BUSY},  //A70LV_Doulas_0133
	{eCLI_MAIN_CCAG, 	"GRTD", 	READ_COMMAND,	  DECIAML_2,		  edcHSG_GREEN_RESET_DEFAULT,	NULL,		   CLI_COM_WRITE_ONLY},  //A70LV_Doulas_0133
	{eCLI_MAIN_CCAG, 	"GRTD", 	WRITE_COMMAND,	  DECIAML_2,		  edcHSG_GREEN_RESET_DEFAULT,	NULL,		   CLI_COM_WRITE_ONLY | CLI_COM_CHECK_BUSY},  //A70LV_Doulas_0133
	{eCLI_MAIN_CCAG, 	"BRTD", 	READ_COMMAND,	  DECIAML_2,		  edcHSG_BLUE_RESET_DEFAULT,	NULL,		   CLI_COM_WRITE_ONLY},  //A70LV_Doulas_0133
	{eCLI_MAIN_CCAG, 	"BRTD", 	WRITE_COMMAND,	  DECIAML_2,		  edcHSG_BLUE_RESET_DEFAULT,	NULL,		   CLI_COM_WRITE_ONLY | CLI_COM_CHECK_BUSY},  //A70LV_Doulas_0133
	{eCLI_MAIN_CCAG, 	"CRTD", 	READ_COMMAND,	  DECIAML_2,		  edcHSG_CYAN_RESET_DEFAULT,	NULL,		   CLI_COM_WRITE_ONLY},  //A70LV_Doulas_0133
	{eCLI_MAIN_CCAG, 	"CRTD", 	WRITE_COMMAND,	  DECIAML_2,		  edcHSG_CYAN_RESET_DEFAULT,	NULL,		   CLI_COM_WRITE_ONLY | CLI_COM_CHECK_BUSY},  //A70LV_Doulas_0133
	{eCLI_MAIN_CCAG, 	"MRTD", 	READ_COMMAND,	  DECIAML_2,		  edcHSG_MAGENTA_RESET_DEFAULT, NULL,		   CLI_COM_WRITE_ONLY},  //A70LV_Doulas_0133
	{eCLI_MAIN_CCAG, 	"MRTD", 	WRITE_COMMAND,	  DECIAML_2,		  edcHSG_MAGENTA_RESET_DEFAULT, NULL,		   CLI_COM_WRITE_ONLY | CLI_COM_CHECK_BUSY},  //A70LV_Doulas_0133
	{eCLI_MAIN_CCAG, 	"YRTD", 	READ_COMMAND,	  DECIAML_2,		  edcHSG_YELLOW_RESET_DEFAULT,	NULL,		   CLI_COM_WRITE_ONLY},  //A70LV_Doulas_0133
	{eCLI_MAIN_CCAG, 	"YRTD", 	WRITE_COMMAND,	  DECIAML_2,		  edcHSG_YELLOW_RESET_DEFAULT,	NULL,		   CLI_COM_WRITE_ONLY | CLI_COM_CHECK_BUSY},  //A70LV_Doulas_0133

    //HAR============================================================================================================//HAR //A35G2_BRC_Casper_0018
    {eCLI_MAIN_HAR,     NULL,       READ_COMMAND,     DECIAML_2,          edcHSG_RESET_DEFAULT,          NULL,           CLI_COM_WRITE_ONLY}, //G100_Tim_003, mod, edcHSG_RESET_DEFAULT
    {eCLI_MAIN_HAR,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcHSG_RESET_DEFAULT,          NULL,           CLI_COM_WRITE_ONLY | CLI_COM_CHECK_BUSY}, //G100_Tim_003, mod, edcHSG_RESET_DEFAULT

    //HSR============================================================================================================//HSR  //A35G2_BRC_Casper_0114
    {eCLI_MAIN_HSR,     NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,         utilGeneral_HSRT,       CLI_COM_WRITE_ONLY},
    {eCLI_MAIN_HSR,     NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,         utilGeneral_HSRT,       CLI_COM_WRITE_ONLY | CLI_COM_CHECK_BUSY},

    //HSG============================================================================================================//HSG
    {eCLI_MAIN_HSG,     NULL,       READ_COMMAND,     DECIAML_2,          edcCOLOR_ENHANCEMENT,         NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_HSG,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcCOLOR_ENHANCEMENT,         NULL,           CLI_COM_NORMAL},

    //CCA============================================================================================================//CCA
    {eCLI_MAIN_CCAG,    "WALL",     READ_COMMAND,     DECIAML_2,          edcWALL_COLOR,                NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_CCAG,    "WALL",     WRITE_COMMAND,    DECIAML_2,          edcWALL_COLOR,                NULL,           CLI_COM_NORMAL},

    //HKS============================================================================================================//HKS
    {eCLI_MAIN_HKS,     NULL,       READ_COMMAND,     DECIAML_2,          edcHOT_KEY_SETTINGS,          NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_HKS,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcHOT_KEY_SETTINGS,          NULL,           CLI_COM_NORMAL},

	//LGC============================================================================================================//HKS //T100IR_Casper_0032
	{eCLI_MAIN_LGC, 	NULL,		READ_COMMAND,	  DECIAML_2,		  edcBACKGROUND_COLOR,			NULL,			CLI_COM_NORMAL},
	{eCLI_MAIN_LGC, 	NULL,		WRITE_COMMAND,	  DECIAML_2,		  edcBACKGROUND_COLOR,			NULL,			CLI_COM_NORMAL},

    //PIF============================================================================================================//PIF
    {eCLI_MAIN_PIF,     "NERS",     READ_COMMAND,     DECIAML_2,          edcNATIVE_RESOLUTION,         NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_PIF,     "NERS",     WRITE_COMMAND,    DECIAML_2,          edcNATIVE_RESOLUTION,         NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_PIF,     "FWV6",     READ_COMMAND,     DECIAML_2,          edcPMCU_VERSION,           	NULL,          CLI_COM_READ_ONLY},
    {eCLI_MAIN_PIF,     "FWV6",     WRITE_COMMAND,    DECIAML_2,          edcPMCU_VERSION,           	NULL,          CLI_COM_READ_ONLY},
    //{eCLI_MAIN_PIF,     "FWV9",     READ_COMMAND,     DECIAML_2,          edcMEMC_VERSION,            	NULL,          CLI_COM_READ_ONLY},
    //{eCLI_MAIN_PIF,     "FWV9",     WRITE_COMMAND,    DECIAML_2,          edcMEMC_VERSION,            	NULL,          CLI_COM_READ_ONLY},
    //{eCLI_MAIN_PIF,     "FWVC",    	READ_COMMAND,     DECIAML_2,          edcCAMERA_FW_VERSION,       	NULL,          CLI_COM_READ_ONLY}, //T100_Casper_0067
    //{eCLI_MAIN_PIF,     "FWVC",    	WRITE_COMMAND,    DECIAML_2,          edcCAMERA_FW_VERSION,       	NULL,          CLI_COM_READ_ONLY}, //T100_Casper_0065
    //{eCLI_MAIN_PIF,     "FWVD",    	READ_COMMAND,     DECIAML_2,          edc3GSDI_VERSION,         	NULL,          CLI_COM_READ_ONLY},
    //{eCLI_MAIN_PIF,     "FWVD",    	WRITE_COMMAND,    DECIAML_2,          edc3GSDI_VERSION,         	NULL,          CLI_COM_READ_ONLY},
    {eCLI_MAIN_PIF,     "FWVR",    	READ_COMMAND,     DECIAML_2,          edcRELEASE_VERSION,       	NULL,          CLI_COM_READ_ONLY},
    {eCLI_MAIN_PIF,     "FWVR",    	WRITE_COMMAND,    DECIAML_2,          edcRELEASE_VERSION,       	NULL,          CLI_COM_READ_ONLY},
    {eCLI_MAIN_PIF,     "WHEE",     READ_COMMAND,     DECIAML_3,          edcWHEEL_INDEX_SHOW,          NULL,          CLI_COM_READ_ONLY},
    {eCLI_MAIN_PIF,     "WHEE",     WRITE_COMMAND,    DECIAML_3,          edcWHEEL_INDEX_SHOW,          NULL,          CLI_COM_READ_ONLY},

    //MDNA============================================================================================================//MDNA //G100_Wilsonj_0042
    {eCLI_MAIN_MDNA,    NULL,       READ_COMMAND,     DECIAML_2,          edcMODEL_NAME,                NULL,          CLI_COM_READ_ONLY},
    {eCLI_MAIN_MDNA,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcMODEL_NAME,                NULL,          CLI_COM_READ_ONLY},

    //SERI============================================================================================================//SERI //G100_Wilsonj_0042
    {eCLI_MAIN_SERI,    NULL,       READ_COMMAND,     DECIAML_2,          edcSERIAL_NUMBER,             NULL,          CLI_COM_NORMAL},   //A70LV_Doulas_0227 add //A70LV_Doulas_0125 remove
    //{eCLI_MAIN_PIF,   NULL,       WRITE_COMMAND,    DECIAML_2,          edcSERIAL_NUMBER,             NULL,          CLI_COM_NORMAL},   //A70LV_Doulas_0125 remove

    //FWIS============================================================================================================//FWIS //G100_Wilsonj_0042
    {eCLI_MAIN_FWIS,    NULL,       READ_COMMAND,     DECIAML_2,          edcSMCU_VERSION,          	NULL,          CLI_COM_READ_ONLY},
    {eCLI_MAIN_FWIS,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcSMCU_VERSION,          	NULL,          CLI_COM_READ_ONLY},

    //FWMF============================================================================================================//FWMF //G100_Wilsonj_0042
    {eCLI_MAIN_FWMF,    NULL,       READ_COMMAND,     DECIAML_2,          edcFRONTEND_VERSION,         	NULL,          CLI_COM_READ_ONLY},
    {eCLI_MAIN_FWMF,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcFRONTEND_VERSION,         	NULL,          CLI_COM_READ_ONLY},

    //FWMM============================================================================================================//FWMM //G100_Wilsonj_0042
    //{eCLI_MAIN_FWMM,    NULL,       READ_COMMAND,     DECIAML_2,          edcMOTOR_VERSION,          	NULL,          CLI_COM_READ_ONLY},
    //{eCLI_MAIN_FWMM,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcMOTOR_VERSION,          	NULL,          CLI_COM_READ_ONLY},

    //FWML============================================================================================================//FWML //G100_Wilsonj_0042
    //{eCLI_MAIN_FWML,    NULL,       READ_COMMAND,     DECIAML_2,          edcLD_DRIVER_VERSION,      	NULL,          CLI_COM_READ_ONLY},
    ///{eCLI_MAIN_FWML,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcLD_DRIVER_VERSION,      	NULL,          CLI_COM_READ_ONLY},

    //FWMA============================================================================================================//FWMA //G100_Wilsonj_0042
    {eCLI_MAIN_FWMA,    NULL,       READ_COMMAND,     DECIAML_2,          edcFMCU_VERSION,              NULL,          CLI_COM_READ_ONLY}, //T100_Casper_0065
    {eCLI_MAIN_FWMA,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcFMCU_VERSION,              NULL,          CLI_COM_READ_ONLY},

    //FWMK============================================================================================================//FWMK //G100_Wilsonj_0042
    {eCLI_MAIN_FWMK,    NULL,       READ_COMMAND,     DECIAML_2,          edcKEYPAD_VERSION,          	NULL,          CLI_COM_READ_ONLY},
    {eCLI_MAIN_FWMK,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcKEYPAD_VERSION,          	NULL,          CLI_COM_READ_ONLY},

    //FWLA============================================================================================================//FWLA //G100_Wilsonj_0042
    {eCLI_MAIN_FWLA,    NULL,       READ_COMMAND,     DECIAML_2,          edcLAN_VERSION,               NULL,          CLI_COM_READ_ONLY},
    {eCLI_MAIN_FWLA,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcLAN_VERSION,               NULL,          CLI_COM_READ_ONLY},

    //FWFM============================================================================================================//FWFM //G100_Wilsonj_0042
    {eCLI_MAIN_FWFM,    NULL,    	READ_COMMAND,     DECIAML_2,          edcFORMATER_VERSION,         	NULL,          CLI_COM_READ_ONLY}, //T100_Casper_0067
    {eCLI_MAIN_FWFM,    NULL,    	WRITE_COMMAND,    DECIAML_2,          edcFORMATER_VERSION,        	NULL,          CLI_COM_READ_ONLY},

    //FWFA============================================================================================================//FWFA //G100_Wilsonj_0042
    {eCLI_MAIN_FWFA,    NULL,    	READ_COMMAND,     DECIAML_2,          edcFPGA1_VERSION,             NULL,          CLI_COM_READ_ONLY},
    {eCLI_MAIN_FWFA,    NULL,    	WRITE_COMMAND,    DECIAML_2,          edcFPGA1_VERSION,        	    NULL,          CLI_COM_READ_ONLY},

    //FWFB============================================================================================================//FWFB //G100_Wilsonj_0042
    {eCLI_MAIN_FWFB,    NULL,    	READ_COMMAND,     DECIAML_2,          edcFPGA2_VERSION,             NULL,          CLI_COM_READ_ONLY},
    {eCLI_MAIN_FWFB,    NULL,    	WRITE_COMMAND,    DECIAML_2,          edcFPGA2_VERSION,        	    NULL,          CLI_COM_READ_ONLY},

    //FWFC============================================================================================================//FWFC //G100_Wilsonj_0042
    //{eCLI_MAIN_FWFC,    NULL,    	READ_COMMAND,     DECIAML_2,          edcFPGA3_VERSION,             NULL,          CLI_COM_READ_ONLY},
    //{eCLI_MAIN_FWFC,    NULL,    	WRITE_COMMAND,    DECIAML_2,          edcFPGA3_VERSION,        	    NULL,          CLI_COM_READ_ONLY},

    //FWFX============================================================================================================//FWFX //G100_Wilsonj_0042
    {eCLI_MAIN_FWFX,    NULL,    	READ_COMMAND,     DECIAML_2,          edcXFPGA_VERSION,             NULL,          CLI_COM_READ_ONLY},
    {eCLI_MAIN_FWFX,    NULL,    	WRITE_COMMAND,    DECIAML_2,          edcXFPGA_VERSION,        	    NULL,          CLI_COM_READ_ONLY},

    //FWHD============================================================================================================//FWHD //G100_Wilsonj_0042
    {eCLI_MAIN_FWHD,    NULL,    	READ_COMMAND,     DECIAML_2,          edcHDBASET_VERSION,         	NULL,          CLI_COM_READ_ONLY},
    {eCLI_MAIN_FWHD,    NULL,    	WRITE_COMMAND,    DECIAML_2,          edcHDBASET_VERSION,         	NULL,          CLI_COM_READ_ONLY},

    //DEF============================================================================================================//DEF
    {eCLI_MAIN_DEF,     NULL,       READ_COMMAND,     DECIAML_3,          DATA_CODE_NA,       utilGeneral_DEF,          CLI_COM_WRITE_ONLY | CLI_COM_NEED_SERVICE_CODE_WRITE},     //T100_Doulas_0029
    {eCLI_MAIN_DEF,     NULL,       WRITE_COMMAND,    DECIAML_3,          DATA_CODE_NA,       utilGeneral_DEF,          CLI_COM_WRITE_ONLY | CLI_COM_NEED_SERVICE_CODE_WRITE},     //T100_Doulas_0029

    //ITP============================================================================================================//ITP
    {eCLI_MAIN_ITP,     NULL,       READ_COMMAND,     DECIAML_2,          edcSERVICE_TEST_PATTERN,      NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_ITP,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcSERVICE_TEST_PATTERN,      NULL,          CLI_COM_NORMAL},

    //PSID============================================================================================================//PSID
    {eCLI_MAIN_PSID,    NULL,       READ_COMMAND,     DECIAML_3,          DATA_CODE_NA,  			utilGeneral_PSID, 	CLI_COM_NORMAL | CLI_COM_NEED_SERVICE_CODE_WRITE},     //A70LV_Doulas_0138 modify
    {eCLI_MAIN_PSID,    NULL,       WRITE_COMMAND,    DECIAML_3,          DATA_CODE_NA,  			utilGeneral_PSID, 	CLI_COM_NORMAL | CLI_COM_NEED_SERVICE_CODE_WRITE},

    //FTID============================================================================================================//FTID
    {eCLI_MAIN_FTID,    NULL,       READ_COMMAND,     DECIAML_3,          DATA_CODE_NA,  			utilGeneral_FSID,	CLI_COM_NORMAL | CLI_COM_NEED_SERVICE_CODE_WRITE},
    {eCLI_MAIN_FTID,    NULL,       WRITE_COMMAND,    DECIAML_3,          DATA_CODE_NA,  			utilGeneral_FSID, 	CLI_COM_NORMAL | CLI_COM_NEED_SERVICE_CODE_WRITE},

    //ERR============================================================================================================//ERR
    {eCLI_MAIN_ERR,     NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,            utilGeneral_CLI_SYSELOG,          CLI_COM_READ_ONLY},
    {eCLI_MAIN_ERR,     NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,            utilGeneral_CLI_SYSELOG,          CLI_COM_READ_ONLY},
//    {eCLI_MAIN_ERR,     "CLER",     READ_COMMAND,     DECIAML_2,          edcCLEAR_ERROR_LOG,           NULL,          CLI_COM_WRITE_ONLY},
//    {eCLI_MAIN_ERR,     "CLER",     WRITE_COMMAND,    DECIAML_2,          edcCLEAR_ERROR_LOG,           NULL,          CLI_COM_WRITE_ONLY},

#if 0
    //MDT============================================================================================================//MDT
    {eCLI_MAIN_MDT,     NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,                   NULL,          CLI_COM_READ_ONLY},
    {eCLI_MAIN_MDT,     NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,                   NULL,          CLI_COM_READ_ONLY},
    {eCLI_MAIN_MDT,     "HPOS",     READ_COMMAND,     DECIAML_3,          DATA_CODE_NA,                   NULL,          CLI_COM_NORMAL | CLI_COM_NEED_SERVICE_CODE_WRITE},
    {eCLI_MAIN_MDT,     "HPOS",     WRITE_COMMAND,    DECIAML_3,          DATA_CODE_NA,                   NULL,          CLI_COM_NORMAL | CLI_COM_NEED_SERVICE_CODE_WRITE},
    {eCLI_MAIN_MDT,     "VPOS",     READ_COMMAND,     DECIAML_3,          DATA_CODE_NA,                   NULL,          CLI_COM_NORMAL | CLI_COM_NEED_SERVICE_CODE_WRITE},
    {eCLI_MAIN_MDT,     "VPOS",     WRITE_COMMAND,    DECIAML_3,          DATA_CODE_NA,                   NULL,          CLI_COM_NORMAL | CLI_COM_NEED_SERVICE_CODE_WRITE},
    {eCLI_MAIN_MDT,     "SAVE",     READ_COMMAND,     DECIAML_3,          DATA_CODE_NA,                   NULL,          CLI_COM_NORMAL | CLI_COM_NEED_SERVICE_CODE_WRITE},
    {eCLI_MAIN_MDT,     "SAVE",     WRITE_COMMAND,    DECIAML_3,          DATA_CODE_NA,                   NULL,          CLI_COM_NORMAL | CLI_COM_NEED_SERVICE_CODE_WRITE},
    {eCLI_MAIN_MDT,     "CLER",     READ_COMMAND,     DECIAML_3,          DATA_CODE_NA,                   NULL,          CLI_COM_NORMAL | CLI_COM_NEED_SERVICE_CODE_WRITE},
    {eCLI_MAIN_MDT,     "CLER",     WRITE_COMMAND,    DECIAML_3,          DATA_CODE_NA,                   NULL,          CLI_COM_NORMAL | CLI_COM_NEED_SERVICE_CODE_WRITE},
#endif /* 0 */

    //LD Info============================================================================================================//LD Info
    {eCLI_MAIN_LDI,     "LD01",     WRITE_COMMAND,    DECIAML_2,          edcLD_INFO_01,                NULL,         CLI_COM_READ_ONLY},
    {eCLI_MAIN_LDI,     "LD01",     READ_COMMAND,     DECIAML_2,          edcLD_INFO_01,                NULL,         CLI_COM_READ_ONLY},
    {eCLI_MAIN_LDI,     "LD02",     WRITE_COMMAND,    DECIAML_2,          edcLD_INFO_02,                NULL,         CLI_COM_READ_ONLY},
    {eCLI_MAIN_LDI,     "LD02",     READ_COMMAND,     DECIAML_2,          edcLD_INFO_02,                NULL,         CLI_COM_READ_ONLY},
    {eCLI_MAIN_LDI,     "LD03",     WRITE_COMMAND,    DECIAML_2,          edcLD_INFO_03,                NULL,         CLI_COM_READ_ONLY},
    {eCLI_MAIN_LDI,     "LD03",     READ_COMMAND,     DECIAML_2,          edcLD_INFO_03,                NULL,         CLI_COM_READ_ONLY},
    {eCLI_MAIN_LDI,     "LD04",     WRITE_COMMAND,    DECIAML_2,          edcLD_INFO_04,                NULL,         CLI_COM_READ_ONLY},
    {eCLI_MAIN_LDI,     "LD04",     READ_COMMAND,     DECIAML_2,          edcLD_INFO_04,                NULL,         CLI_COM_READ_ONLY},
    {eCLI_MAIN_LDI,     "LD05",     WRITE_COMMAND,    DECIAML_2,          edcLD_INFO_05,                NULL,         CLI_COM_READ_ONLY},
    {eCLI_MAIN_LDI,     "LD05",     READ_COMMAND,     DECIAML_2,          edcLD_INFO_05,                NULL,         CLI_COM_READ_ONLY},
    {eCLI_MAIN_LDI,     "LD06",     WRITE_COMMAND,    DECIAML_2,          edcLD_INFO_06,                NULL,         CLI_COM_READ_ONLY},
    {eCLI_MAIN_LDI,     "LD06",     READ_COMMAND,     DECIAML_2,          edcLD_INFO_06,                NULL,         CLI_COM_READ_ONLY},
    {eCLI_MAIN_LDI,     "LD07",     WRITE_COMMAND,    DECIAML_2,          edcLD_INFO_07,                NULL,         CLI_COM_READ_ONLY},
    {eCLI_MAIN_LDI,     "LD07",     READ_COMMAND,     DECIAML_2,          edcLD_INFO_07,                NULL,         CLI_COM_READ_ONLY},
    {eCLI_MAIN_LDI,     "LD08",     WRITE_COMMAND,    DECIAML_2,          edcLD_INFO_08,                NULL,         CLI_COM_READ_ONLY},
    {eCLI_MAIN_LDI,     "LD08",     READ_COMMAND,     DECIAML_2,          edcLD_INFO_08,                NULL,         CLI_COM_READ_ONLY},
    //{eCLI_MAIN_LDI,     "LD09",     WRITE_COMMAND,    DECIAML_2,          edcLD_INFO_09,                NULL,         CLI_COM_READ_ONLY},
    //{eCLI_MAIN_LDI,     "LD09",     READ_COMMAND,     DECIAML_2,          edcLD_INFO_09,                NULL,         CLI_COM_READ_ONLY},
    //{eCLI_MAIN_LDI,     "LD10",     WRITE_COMMAND,    DECIAML_2,          edcLD_INFO_10,                NULL,         CLI_COM_READ_ONLY},
    //{eCLI_MAIN_LDI,     "LD10",     READ_COMMAND,     DECIAML_2,          edcLD_INFO_10,                NULL,         CLI_COM_READ_ONLY},
    //{eCLI_MAIN_LDI,     "LD11",     WRITE_COMMAND,    DECIAML_2,          edcLD_INFO_11,                NULL,         CLI_COM_READ_ONLY},
    //{eCLI_MAIN_LDI,     "LD11",     READ_COMMAND,     DECIAML_2,          edcLD_INFO_11,                NULL,         CLI_COM_READ_ONLY},
    //{eCLI_MAIN_LDI,     "LD12",     WRITE_COMMAND,    DECIAML_2,          edcLD_INFO_12,                NULL,         CLI_COM_READ_ONLY},
    //{eCLI_MAIN_LDI,     "LD12",     READ_COMMAND,     DECIAML_2,          edcLD_INFO_12,                NULL,         CLI_COM_READ_ONLY},
    //{eCLI_MAIN_LDI,	  "LD13",	  WRITE_COMMAND,	DECIAML_2,			edcLD_INFO_13,				  NULL, 		CLI_COM_READ_ONLY},
    //{eCLI_MAIN_LDI,	  "LD13",	  READ_COMMAND, 	DECIAML_2,			edcLD_INFO_13,				  NULL, 		CLI_COM_READ_ONLY},
    //{eCLI_MAIN_LDI,	  "LD14",	  WRITE_COMMAND,	DECIAML_2,			edcLD_INFO_14,				  NULL, 		CLI_COM_READ_ONLY},
    //{eCLI_MAIN_LDI,	  "LD14",	  READ_COMMAND, 	DECIAML_2,			edcLD_INFO_14,				  NULL, 		CLI_COM_READ_ONLY},

    //Fan Info============================================================================================================//Fan Info
    {eCLI_MAIN_FAN,     "FA01",     WRITE_COMMAND,    DECIAML_4,          edcFAN_RPM_01,                NULL,         CLI_COM_READ_ONLY},
    {eCLI_MAIN_FAN,     "FA01",     READ_COMMAND,     DECIAML_4,          edcFAN_RPM_01,                NULL,         CLI_COM_READ_ONLY},
    {eCLI_MAIN_FAN,     "FA02",     WRITE_COMMAND,    DECIAML_4,          edcFAN_RPM_02,                NULL,         CLI_COM_READ_ONLY},
    {eCLI_MAIN_FAN,     "FA02",     READ_COMMAND,     DECIAML_4,          edcFAN_RPM_02,                NULL,         CLI_COM_READ_ONLY},
    {eCLI_MAIN_FAN,     "FA03",     WRITE_COMMAND,    DECIAML_4,          edcFAN_RPM_03,                NULL,         CLI_COM_READ_ONLY},
    {eCLI_MAIN_FAN,     "FA03",     READ_COMMAND,     DECIAML_4,          edcFAN_RPM_03,                NULL,         CLI_COM_READ_ONLY},
    {eCLI_MAIN_FAN,     "FA04",     WRITE_COMMAND,    DECIAML_4,          edcFAN_RPM_04,                NULL,         CLI_COM_READ_ONLY},
    {eCLI_MAIN_FAN,     "FA04",     READ_COMMAND,     DECIAML_4,          edcFAN_RPM_04,                NULL,         CLI_COM_READ_ONLY},
    {eCLI_MAIN_FAN,     "FA05",     WRITE_COMMAND,    DECIAML_4,          edcFAN_RPM_05,                NULL,         CLI_COM_READ_ONLY},
    {eCLI_MAIN_FAN,     "FA05",     READ_COMMAND,     DECIAML_4,          edcFAN_RPM_05,                NULL,         CLI_COM_READ_ONLY},
    {eCLI_MAIN_FAN,     "FA06",     WRITE_COMMAND,    DECIAML_4,          edcFAN_RPM_06,                NULL,         CLI_COM_READ_ONLY},
    {eCLI_MAIN_FAN,     "FA06",     READ_COMMAND,     DECIAML_4,          edcFAN_RPM_06,                NULL,         CLI_COM_READ_ONLY},
    {eCLI_MAIN_FAN,     "FA07",     WRITE_COMMAND,    DECIAML_4,          edcFAN_RPM_07,                NULL,         CLI_COM_READ_ONLY},
    {eCLI_MAIN_FAN,     "FA07",     READ_COMMAND,     DECIAML_4,          edcFAN_RPM_07,                NULL,         CLI_COM_READ_ONLY},
    {eCLI_MAIN_FAN,     "FA08",     WRITE_COMMAND,    DECIAML_4,          edcFAN_RPM_08,                NULL,         CLI_COM_READ_ONLY},
    {eCLI_MAIN_FAN,     "FA08",     READ_COMMAND,     DECIAML_4,          edcFAN_RPM_08,                NULL,         CLI_COM_READ_ONLY},
    {eCLI_MAIN_FAN,     "FA09",     WRITE_COMMAND,    STRING_CHAR,        DATA_CODE_NA,                 utilGeneral_FANFA09,         CLI_COM_NORMAL},
    {eCLI_MAIN_FAN,     "FA09",     READ_COMMAND,     STRING_CHAR,        DATA_CODE_NA,                 utilGeneral_FANFA09,         CLI_COM_NORMAL},
    //{eCLI_MAIN_FAN,     "FA10",     WRITE_COMMAND,    DECIAML_4,          edcFAN_RPM_10,                NULL,         CLI_COM_READ_ONLY},
    //{eCLI_MAIN_FAN,     "FA10",     READ_COMMAND,     DECIAML_4,          edcFAN_RPM_10,                NULL,         CLI_COM_READ_ONLY},
//G100_Steven_0049 start
	//{eCLI_MAIN_FAN,     "FA11",     WRITE_COMMAND,    DECIAML_4,          edcFAN_RPM_11,                NULL,         CLI_COM_READ_ONLY},
	//{eCLI_MAIN_FAN,     "FA11",     READ_COMMAND,     DECIAML_4,          edcFAN_RPM_11,                NULL,         CLI_COM_READ_ONLY},
	//{eCLI_MAIN_FAN,     "FA12",     WRITE_COMMAND,    DECIAML_4,          edcFAN_RPM_12,                NULL,         CLI_COM_READ_ONLY},
	//{eCLI_MAIN_FAN,     "FA12",     READ_COMMAND,     DECIAML_4,          edcFAN_RPM_12,                NULL,         CLI_COM_READ_ONLY},
	//{eCLI_MAIN_FAN,     "FA13",     WRITE_COMMAND,    DECIAML_4,          edcFAN_RPM_13,                NULL,         CLI_COM_READ_ONLY},
	//{eCLI_MAIN_FAN,     "FA13",     READ_COMMAND,     DECIAML_4,          edcFAN_RPM_13,                NULL,         CLI_COM_READ_ONLY},
	//{eCLI_MAIN_FAN,     "FA14",     WRITE_COMMAND,    DECIAML_4,          edcFAN_RPM_14,                NULL,         CLI_COM_READ_ONLY},
	//{eCLI_MAIN_FAN,     "FA14",     READ_COMMAND,     DECIAML_4,          edcFAN_RPM_14,                NULL,         CLI_COM_READ_ONLY},
	//{eCLI_MAIN_FAN,     "FA15",     WRITE_COMMAND,    DECIAML_4,          edcFAN_RPM_15,                NULL,         CLI_COM_READ_ONLY},
	//{eCLI_MAIN_FAN,     "FA15",     READ_COMMAND,     DECIAML_4,          edcFAN_RPM_15,                NULL,         CLI_COM_READ_ONLY},
	//{eCLI_MAIN_FAN,     "FA16",     WRITE_COMMAND,    DECIAML_4,          edcFAN_RPM_16,                NULL,         CLI_COM_READ_ONLY},
	//{eCLI_MAIN_FAN,     "FA16",     READ_COMMAND,     DECIAML_4,          edcFAN_RPM_16,                NULL,         CLI_COM_READ_ONLY},
	//G100_Steven_0049 end
//	  {eCLI_MAIN_FAN,	  "FA11",	  WRITE_COMMAND,	DECIAML_4,			edcFAN_RPM_11,				  NULL, 		CLI_COM_READ_ONLY},
//	  {eCLI_MAIN_FAN,	  "FA11",	  READ_COMMAND, 	DECIAML_4,			edcFAN_RPM_11,				  NULL, 		CLI_COM_READ_ONLY},

    //LPM============================================================================================================//LPM
    {eCLI_MAIN_LPM,     NULL,       READ_COMMAND,     DECIAML_2,          edcPOWER_MODE,                NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_LPM,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcPOWER_MODE,                NULL,          CLI_COM_NORMAL},

    //LPP============================================================================================================//LPP
    {eCLI_MAIN_LPP,     NULL,       READ_COMMAND,     DECIAML_2,          edcCONSTANT_POWER_NUMBER,     NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_LPP,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcCONSTANT_POWER_NUMBER,     NULL,          CLI_COM_NORMAL},

    //LPI============================================================================================================//LPP //T100IR_Casper_0029
    {eCLI_MAIN_LPI,     NULL,       READ_COMMAND,     DECIAML_2,          edcCONSTANT_IR_POWER_NUMBER, 	NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_LPI,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcCONSTANT_IR_POWER_NUMBER, 	NULL,          CLI_COM_NORMAL},

    //LLP============================================================================================================//LPP //T100IR_Casper_0029
    {eCLI_MAIN_LLP,     NULL,       READ_COMMAND,     DECIAML_2,          edcIRMODE_BLD_POWER_NUMBER, 	NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_LLP,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcIRMODE_BLD_POWER_NUMBER, 	NULL,          CLI_COM_NORMAL},

    //LLI============================================================================================================//LPP //T100IR_Casper_0029
    {eCLI_MAIN_LLI,     NULL,       READ_COMMAND,     DECIAML_2,          edcIRMODE_IR_POWER_NUMBER, 	NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_LLI,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcIRMODE_IR_POWER_NUMBER, 	NULL,          CLI_COM_NORMAL},

    //BLE============================================================================================================//LPP //T100IR_Casper_0032
    {eCLI_MAIN_BLE,     NULL,       READ_COMMAND,     DECIAML_2,          edcBLD_ENABLE, 				NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_BLE,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcBLD_ENABLE, 				NULL,          CLI_COM_NORMAL},

    //IRE============================================================================================================//LPP //T100IR_Casper_0032
    {eCLI_MAIN_IRE,     NULL,       READ_COMMAND,     DECIAML_2,          edcIRLD_ENABLE, 				NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_IRE,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcIRLD_ENABLE, 				NULL,          CLI_COM_NORMAL},

    //LIF============================================================================================================//LIF
    {eCLI_MAIN_LIF,     "TPHS",     READ_COMMAND,     DECIAML_5,          DATA_CODE_NA,   utilGeneral_LIF_TPHS,          CLI_COM_NORMAL| CLI_COM_NEED_SERVICE_CODE_WRITE}, //T100_Larry_0051
    {eCLI_MAIN_LIF,     "TPHS",     WRITE_COMMAND,    DECIAML_5,          DATA_CODE_NA,   utilGeneral_LIF_TPHS,          CLI_COM_NORMAL| CLI_COM_NEED_SERVICE_CODE_WRITE}, //T100_Larry_0051
    {eCLI_MAIN_LIF,     "LSHS",     READ_COMMAND,     DECIAML_5,          DATA_CODE_NA,   utilGeneral_LIF_LSHS,          CLI_COM_NORMAL| CLI_COM_NEED_SERVICE_CODE_WRITE}, //T100_Larry_0051
    {eCLI_MAIN_LIF,     "LSHS",     WRITE_COMMAND,    DECIAML_5,          DATA_CODE_NA,   utilGeneral_LIF_LSHS,          CLI_COM_NORMAL| CLI_COM_NEED_SERVICE_CODE_WRITE}, //T100_Larry_0051

    //RLC============================================================================================================//RLC
    {eCLI_MAIN_RLC,     NULL,       READ_COMMAND,     DECIAML_2,          edcLIGHT_SENSOR_CALIBRATION,  NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_RLC,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcLIGHT_SENSOR_CALIBRATION,  NULL,          CLI_COM_NORMAL},

    //SIN============================================================================================================//SIN
    //{eCLI_MAIN_SIN,     NULL,       READ_COMMAND,     DECIAML_2,          edcMAIN_INPUT,                NULL,          CLI_COM_NORMAL},    //G100_Wilsonj_0032 modify
    //{eCLI_MAIN_SIN,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcMAIN_INPUT,                NULL,          CLI_COM_NORMAL},    //G100_Wilsonj_0032 modify
    //{eCLI_MAIN_SIN,     "MAIN",     READ_COMMAND,     DECIAML_2,          edcMAIN_INPUT,                NULL,          CLI_COM_NORMAL},     //T100_Doulas_0031 modify
    //{eCLI_MAIN_SIN,     "MAIN",     WRITE_COMMAND,    DECIAML_2,          edcMAIN_INPUT,                NULL,          CLI_COM_NORMAL},     //T100_Doulas_0031 modify
    //{eCLI_MAIN_SIN,     "PIIP",     READ_COMMAND,     DECIAML_2,          edcSUB_INPUT,                 NULL,          CLI_COM_NORMAL},     //T100_Doulas_0031 modify
    //{eCLI_MAIN_SIN,     "PIIP",     WRITE_COMMAND,    DECIAML_2,          edcSUB_INPUT,                 NULL,          CLI_COM_NORMAL},     //T100_Doulas_0031 modify

    //PIP============================================================================================================//PIP
    {eCLI_MAIN_PIP,     NULL,       READ_COMMAND,     DECIAML_2,          edcPIP_PBP_ENABLE,            NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_PIP,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcPIP_PBP_ENABLE,            NULL,          CLI_COM_NORMAL},

    //PPS============================================================================================================//PPS
    {eCLI_MAIN_PPS,     NULL,       READ_COMMAND,     DECIAML_2,          edcSWAP,                      NULL,          CLI_COM_WRITE_ONLY},
    {eCLI_MAIN_PPS,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcSWAP,                      NULL,          CLI_COM_WRITE_ONLY},

    //PHS============================================================================================================//PHS
    {eCLI_MAIN_PHS,     NULL,       READ_COMMAND,     DECIAML_2,          edcSIZE,                      NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_PHS,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcSIZE,                      NULL,          CLI_COM_NORMAL},

    //PPP============================================================================================================//PPP
    {eCLI_MAIN_PPP,     NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,               utilGeneral_PIP_Layout,          CLI_COM_NORMAL},
    {eCLI_MAIN_PPP,     NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,               utilGeneral_PIP_Layout,          CLI_COM_NORMAL},

/*
    //TMG============================================================================================================//TMG
    {eCLI_MAIN_TMG,     NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,                   NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_TMG,     NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,                   NULL,          CLI_COM_NORMAL},
*/
    //SKS============================================================================================================//SKS
    {eCLI_MAIN_SKS,     NULL,       READ_COMMAND,     DECIAML_2,          edcINPUT_KEY,                 NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_SKS,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcINPUT_KEY,                 NULL,          CLI_COM_NORMAL},

    //MTO============================================================================================================//MTO
    {eCLI_MAIN_MTO,     NULL,       READ_COMMAND,     DECIAML_2,          edcMENU_TIME_OUT,             NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_MTO,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcMENU_TIME_OUT,             NULL,          CLI_COM_NORMAL},

    //SST============================================================================================================//SST //G100_Larry_0029 mask
    //{eCLI_MAIN_SST,     NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,               utilGeneral_SST,   CLI_COM_NO_REPLY|CLI_COM_READ_ONLY},    //A70LV_Doulas_0140 modify    //T100_Simon_0021
    //{eCLI_MAIN_SST,     NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,               utilGeneral_SST,   CLI_COM_NO_REPLY|CLI_COM_READ_ONLY},    //A70LV_Doulas_0140 modify    //T100_Simon_0021

    //KEY
    {eCLI_MAIN_KEY,     NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,      utilGeneral_CLI_KEY,          CLI_COM_WRITE_ONLY},

    //TTT HBPU test
    {eCLI_MAIN_TTT,     NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,      utilGeneral_CLI_TTT,          CLI_COM_READ_ONLY},

    //CFU chip FW upgrade
    {eCLI_MAIN_CFU,     NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,      utilGeneral_CFU,           CLI_COM_NORMAL | CLI_COM_NO_REPLY},
    {eCLI_MAIN_CFU,     NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,      utilGeneral_CFU,           CLI_COM_NORMAL | CLI_COM_NO_REPLY},

    //External Flash burn
    {eCLI_MAIN_EXF,     "ADDR",     WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,      utilGeneral_EXFADDR,       CLI_COM_NORMAL | CLI_COM_NO_REPLY},
    {eCLI_MAIN_EXF,     "ADDR",     READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,      utilGeneral_EXFADDR,       CLI_COM_NORMAL | CLI_COM_NO_REPLY},
    {eCLI_MAIN_EXF,     "CHUM",     WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,      utilGeneral_EXFCHUM,       CLI_COM_NORMAL | CLI_COM_NO_REPLY},
    {eCLI_MAIN_EXF,     "CHUM",     READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,      utilGeneral_EXFCHUM,       CLI_COM_NORMAL | CLI_COM_NO_REPLY},
    {eCLI_MAIN_EXF,     "ERBL",     WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,      utilGeneral_EXFERBL,       CLI_COM_NORMAL | CLI_COM_NO_REPLY},
    {eCLI_MAIN_EXF,     "ERBL",     READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,      utilGeneral_EXFERBL,       CLI_COM_NORMAL | CLI_COM_NO_REPLY},
    {eCLI_MAIN_EXF,     "WREL",     WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,      utilGeneral_EXFWREL,       CLI_COM_NORMAL | CLI_COM_NO_REPLY},
    {eCLI_MAIN_EXF,     "WREL",     READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,      utilGeneral_EXFWREL,       CLI_COM_NORMAL | CLI_COM_NO_REPLY},

    //Burnin
    {eCLI_MAIN_BUR,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcBURN_IN,                   NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_BUR,     NULL,       READ_COMMAND,     DECIAML_2,          edcBURN_IN,                   NULL,          CLI_COM_NORMAL},

#if 0
    //GUI
    {eCLI_MAIN_GUI,     "ERAS",     WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,      utilGeneral_Format_GuiData,           CLI_COM_NORMAL | CLI_COM_NO_REPLY},
    {eCLI_MAIN_GUI,     "TXTD",     WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,      utilGeneral_UpdateTextGuiDataByDMA,   CLI_COM_NORMAL | CLI_COM_NO_REPLY},
    {eCLI_MAIN_GUI,     "BMPD",     WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,      utilGeneral_UpdateBitmapGuiDataByDMA, CLI_COM_NORMAL | CLI_COM_NO_REPLY},
    {eCLI_MAIN_GUI,     "ALLD",     WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,      utilGeneral_UpdateAllGuiData,         CLI_COM_NORMAL | CLI_COM_NO_REPLY},

    //MEMC
    {eCLI_MAIN_MMC,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcMEMC,                      NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_MMC,     "DTIL",     READ_COMMAND,     DECIAML_2,          edcMEMC_DETAIL,               NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_MMC,     "DTIL",     WRITE_COMMAND,    DECIAML_2,          edcMEMC_DETAIL,               NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_MMC,     "COLR",     READ_COMMAND,     DECIAML_2,          edcMEMC_COLOR,                NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_MMC,     "COLR",     WRITE_COMMAND,    DECIAML_2,          edcMEMC_COLOR,                NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_MMC,     "MOTN",     READ_COMMAND,     DECIAML_2,          edcMEMC_MOTION,               NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_MMC,     "MOTN",     WRITE_COMMAND,    DECIAML_2,          edcMEMC_MOTION,               NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_MMC,     "DEMO",     READ_COMMAND,     DECIAML_2,          edcMEMC_MOTION_DEMO,          NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_MMC,     "DEMO",     WRITE_COMMAND,    DECIAML_2,          edcMEMC_MOTION_DEMO,          NULL,           CLI_COM_NORMAL},

    //HDR============================================================================================================//BOG
    {eCLI_MAIN_HDR,     NULL,       READ_COMMAND,     DECIAML_2,          edcHDR,                       NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_HDR,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcHDR,                       NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_HDR,     "HDRE",     READ_COMMAND,     DECIAML_2,          edcHDR_AUTOENABLE,            NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_HDR,     "HDRE",     WRITE_COMMAND,    DECIAML_2,          edcHDR_AUTOENABLE,            NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_HDR,     "HDRL",     READ_COMMAND,     DECIAML_2,          edcHDR_LEVEL,                 NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_HDR,     "HDRL",     WRITE_COMMAND,    DECIAML_2,          edcHDR_LEVEL,                 NULL,           CLI_COM_NORMAL},
#endif

    //ILI============================================================================================================//ILI
    {eCLI_MAIN_ILI,     NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,           utilGeneral_ILI,     	CLI_COM_NO_REPLY|CLI_COM_READ_ONLY},  //A70LV_Doulas_0140 modify   //T100_Simon_0021
    {eCLI_MAIN_ILI,     NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,           utilGeneral_ILI,     	CLI_COM_NO_REPLY|CLI_COM_READ_ONLY},  //A70LV_Doulas_0140 modify   //T100_Simon_0021

    //FCT============================================================================================================//FCT  //T100_Doulas_0029
    {eCLI_MAIN_FCT,     "SERN",     READ_COMMAND,     DECIAML_2,          edcSERIAL_NUMBER,                NULL,        CLI_COM_NORMAL}, //SN
    {eCLI_MAIN_FCT,     "SERN",     WRITE_COMMAND,    DECIAML_2,          edcSERIAL_NUMBER,                NULL,        CLI_COM_NORMAL}, //SN
#if 1
    //SHU============================================================================================================//SHU  //T100_Simon_0005
    {eCLI_MAIN_SHU,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcPICTURE_MUTE,                 NULL,        CLI_COM_NORMAL },
    {eCLI_MAIN_SHU,     NULL,       READ_COMMAND,     DECIAML_2,          edcPICTURE_MUTE,                 NULL,        CLI_COM_NORMAL },
    {eCLI_MAIN_SHU,     "LOCK",     WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,     utilGeneral_SHU_LOCK,        CLI_COM_NORMAL }, //A70LV_Larry_0384 //T100_Casper_0093
    {eCLI_MAIN_SHU,     "LOCK",     READ_COMMAND,     DECIAML_2,          edcLENS_DETECTION,               NULL,        CLI_COM_NORMAL }, //A70LV_Larry_0384 //T100_Casper_0093
#endif
    //UID============================================================================================================//UID  //T100_Doulas_0029
    {eCLI_MAIN_UID,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcSERVICE_MODE_FLAG,            NULL,        CLI_COM_NORMAL },
    {eCLI_MAIN_UID,     NULL,       READ_COMMAND,     DECIAML_2,          edcSERVICE_MODE_FLAG,            NULL,        CLI_COM_NORMAL },

    //MIF============================================================================================================//MIF      //T100_Doulas_0031
    //{eCLI_MAIN_MIF,     "ACTS",     READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,     utilGeneral_MIF_ACTS,          CLI_COM_READ_ONLY},
    //{eCLI_MAIN_MIF,     "ACTS",     WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,     utilGeneral_MIF_ACTS,          CLI_COM_READ_ONLY},
    //{eCLI_MAIN_MIF,     "APRT",     READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,     utilGeneral_MIF_APRT,          CLI_COM_READ_ONLY},
    //{eCLI_MAIN_MIF,     "APRT",     WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,     utilGeneral_MIF_APRT,          CLI_COM_READ_ONLY},
    //{eCLI_MAIN_MIF,     "SYNC",     READ_COMMAND,     DECIAML_2,          edcMAIN_SYNC_TYPE,               NULL,          CLI_COM_READ_ONLY},
    //{eCLI_MAIN_MIF,     "SYNC",     WRITE_COMMAND,    DECIAML_2,          edcMAIN_SYNC_TYPE,               NULL,          CLI_COM_READ_ONLY},

    //MSRS============================================================================================================//MSRS //G100_Wilsonj_0042
    {eCLI_MAIN_MSRS,    NULL,       READ_COMMAND,     DECIAML_2,          edcMAIN_RESOLUTION,              NULL,          CLI_COM_READ_ONLY},
    {eCLI_MAIN_MSRS,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcMAIN_RESOLUTION,              NULL,          CLI_COM_READ_ONLY},

    //MSSF============================================================================================================//MSSF //G100_Wilsonj_0042
    {eCLI_MAIN_MSSF,    NULL,       READ_COMMAND,     DECIAML_2,          edcMAIN_SIGNAL_FORMAT,           NULL,          CLI_COM_READ_ONLY},
    {eCLI_MAIN_MSSF,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcMAIN_SIGNAL_FORMAT,           NULL,          CLI_COM_READ_ONLY},

    //MSPC============================================================================================================//MSPC //G100_Wilsonj_0042
    {eCLI_MAIN_MSPC,    NULL,       READ_COMMAND,     DECIAML_2,          edcMAIN_PIXEL_CLOCK,             NULL,          CLI_COM_READ_ONLY},
    {eCLI_MAIN_MSPC,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcMAIN_PIXEL_CLOCK,             NULL,          CLI_COM_READ_ONLY},

    //MSHR============================================================================================================//MSHR //G100_Wilsonj_0042
    {eCLI_MAIN_MSHR,    NULL,       READ_COMMAND,     DECIAML_2,          edcMAIN_HORZ_REFRESH,            NULL,          CLI_COM_READ_ONLY},
    {eCLI_MAIN_MSHR,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcMAIN_HORZ_REFRESH,            NULL,          CLI_COM_READ_ONLY},

    //MSVR============================================================================================================//MSVR //G100_Wilsonj_0042
    {eCLI_MAIN_MSVR,    NULL,       READ_COMMAND,     DECIAML_2,          edcMAIN_VERT_REFRESH,            NULL,          CLI_COM_READ_ONLY},
    {eCLI_MAIN_MSVR,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcMAIN_VERT_REFRESH,            NULL,          CLI_COM_READ_ONLY},

    //MSCS============================================================================================================//MSCS //G100_Wilsonj_0042
    {eCLI_MAIN_MSCS,    NULL,       READ_COMMAND,     DECIAML_2,          edcMAIN_COLOR_SPACE,             NULL,          CLI_COM_READ_ONLY},
    {eCLI_MAIN_MSCS,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcMAIN_COLOR_SPACE,             NULL,          CLI_COM_READ_ONLY},

    //SIF============================================================================================================//SIF      //T100_Doulas_0031
    //{eCLI_MAIN_SIF,     "ACTS",     READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,     utilGeneral_SIF_ACTS,          CLI_COM_READ_ONLY},
    //{eCLI_MAIN_SIF,     "ACTS",     WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,     utilGeneral_SIF_ACTS,          CLI_COM_READ_ONLY},
    //{eCLI_MAIN_SIF,     "APRT",     READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,     utilGeneral_SIF_APRT,          CLI_COM_READ_ONLY},
    //{eCLI_MAIN_SIF,     "APRT",     WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,     utilGeneral_SIF_APRT,          CLI_COM_READ_ONLY},
    //{eCLI_MAIN_SIF,     "SYNC",     READ_COMMAND,     DECIAML_2,          edcSUB_SYNC_TYPE,                NULL,          CLI_COM_READ_ONLY},
    //{eCLI_MAIN_SIF,     "SYNC",     WRITE_COMMAND,    DECIAML_2,          edcSUB_SYNC_TYPE,                NULL,          CLI_COM_READ_ONLY},

    //SSRS============================================================================================================//SSRS //G100_Wilsonj_0042
    {eCLI_MAIN_SSRS,    NULL,       READ_COMMAND,     DECIAML_2,          edcSUB_RESOLUTION,               NULL,          CLI_COM_READ_ONLY},
    {eCLI_MAIN_SSRS,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcSUB_RESOLUTION,               NULL,          CLI_COM_READ_ONLY},

    //SSSF============================================================================================================//SSSF //G100_Wilsonj_0042
    {eCLI_MAIN_SSSF,    NULL,       READ_COMMAND,     DECIAML_2,          edcSUB_SIGNAL_FORMAT,            NULL,          CLI_COM_READ_ONLY},
    {eCLI_MAIN_SSSF,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcSUB_SIGNAL_FORMAT,            NULL,          CLI_COM_READ_ONLY},

    //SSPC============================================================================================================//SSPC //G100_Wilsonj_0042
    {eCLI_MAIN_SSPC,    NULL,       READ_COMMAND,     DECIAML_2,          edcSUB_PIXEL_CLOCK,              NULL,          CLI_COM_READ_ONLY},
    {eCLI_MAIN_SSPC,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcSUB_PIXEL_CLOCK,              NULL,          CLI_COM_READ_ONLY},

    //SSHR============================================================================================================//SSHR //G100_Wilsonj_0042
    {eCLI_MAIN_SSHR,    NULL,       READ_COMMAND,     DECIAML_2,          edcSUB_HORZ_REFRESH,             NULL,          CLI_COM_READ_ONLY},
    {eCLI_MAIN_SSHR,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcSUB_HORZ_REFRESH,             NULL,          CLI_COM_READ_ONLY},

    //SSVR============================================================================================================//SSVR //G100_Wilsonj_0042
    {eCLI_MAIN_SSVR,    NULL,       READ_COMMAND,     DECIAML_2,          edcSUB_VERT_REFRESH,             NULL,          CLI_COM_READ_ONLY},
    {eCLI_MAIN_SSVR,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcSUB_VERT_REFRESH,             NULL,          CLI_COM_READ_ONLY},

    //SSCS============================================================================================================//SSCS //G100_Wilsonj_0042
    {eCLI_MAIN_SSCS,    NULL,       READ_COMMAND,     DECIAML_2,          edcSUB_COLOR_SPACE,              NULL,          CLI_COM_READ_ONLY},
    {eCLI_MAIN_SSCS,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcSUB_COLOR_SPACE,              NULL,          CLI_COM_READ_ONLY},

    //ICI============================================================================================================//ICI          //T100_Doulas_0035
    {eCLI_MAIN_ICI,     NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,          utilGeneral_ICI,         CLI_COM_NO_REPLY|CLI_COM_READ_ONLY},   //T100_Simon_0021
    {eCLI_MAIN_ICI,     NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,          utilGeneral_ICI,         CLI_COM_NO_REPLY|CLI_COM_READ_ONLY},   //T100_Simon_0021

    //SIV============================================================================================================//SIV          //A70LV_Doulas_0139
    {eCLI_MAIN_SIV,     NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,           utilGeneral_SIV,       CLI_COM_READ_ONLY },
    {eCLI_MAIN_SIV,     NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,           utilGeneral_SIV,       CLI_COM_READ_ONLY },

    //LCE
    {eCLI_MAIN_LCE,     NULL,       READ_COMMAND,     DECIAML_3,          edcLAST_SERIAL_CMD_ERRORCODE,    NULL,        CLI_COM_READ_ONLY },     //T100_Simon_0002

    //LSE
    {eCLI_MAIN_LSE,     NULL,       READ_COMMAND,     DECIAML_3,          edcLAST_ERRORCODE,               NULL,        CLI_COM_READ_ONLY },     //T100_Simon_0001

    //OSD
    {eCLI_MAIN_OSD,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcOSD_SHOW,                     NULL,        CLI_COM_NORMAL },        //T100_Simon_0004
    {eCLI_MAIN_OSD,     NULL,       READ_COMMAND,     DECIAML_2,          edcOSD_SHOW,                     NULL,        CLI_COM_NORMAL },        //T100_Simon_0004

    //SNS
    {eCLI_MAIN_SNS,     "SRC0",     WRITE_COMMAND,    DECIAML_2,          edcSOURCE0_NAME,                 NULL,        CLI_COM_NORMAL },        //T100_Simon_0006
    {eCLI_MAIN_SNS,     "SRC0",     READ_COMMAND,     DECIAML_2,          edcSOURCE0_NAME,                 NULL,        CLI_COM_NORMAL },        //T100_Simon_0006
    {eCLI_MAIN_SNS,     "SRC1",     WRITE_COMMAND,    DECIAML_2,          edcSOURCE1_NAME,                 NULL,        CLI_COM_NORMAL },        //T100_Simon_0006
    {eCLI_MAIN_SNS,     "SRC1",     READ_COMMAND,     DECIAML_2,          edcSOURCE1_NAME,                 NULL,        CLI_COM_NORMAL },        //T100_Simon_0006
    {eCLI_MAIN_SNS,     "SRC2",     WRITE_COMMAND,    DECIAML_2,          edcSOURCE2_NAME,                 NULL,        CLI_COM_NORMAL },        //T100_Simon_0006
    {eCLI_MAIN_SNS,     "SRC2",     READ_COMMAND,     DECIAML_2,          edcSOURCE2_NAME,                 NULL,        CLI_COM_NORMAL },        //T100_Simon_0006
    {eCLI_MAIN_SNS,     "SRC3",     WRITE_COMMAND,    DECIAML_2,          edcSOURCE3_NAME,                 NULL,        CLI_COM_NORMAL },        //T100_Simon_0006
    {eCLI_MAIN_SNS,     "SRC3",     READ_COMMAND,     DECIAML_2,          edcSOURCE3_NAME,                 NULL,        CLI_COM_NORMAL },        //T100_Simon_0006
    {eCLI_MAIN_SNS,     "SRC4",     WRITE_COMMAND,    DECIAML_2,          edcSOURCE4_NAME,                 NULL,        CLI_COM_NORMAL },        //T100_Simon_0006
    {eCLI_MAIN_SNS,     "SRC4",     READ_COMMAND,     DECIAML_2,          edcSOURCE4_NAME,                 NULL,        CLI_COM_NORMAL },        //T100_Simon_0006
    {eCLI_MAIN_SNS,     "SRC5",     WRITE_COMMAND,    DECIAML_2,          edcSOURCE5_NAME,                 NULL,        CLI_COM_NORMAL },        //T100_Simon_0006
    {eCLI_MAIN_SNS,     "SRC5",     READ_COMMAND,     DECIAML_2,          edcSOURCE5_NAME,                 NULL,        CLI_COM_NORMAL },        //T100_Simon_0006
    {eCLI_MAIN_SNS,     "SRC6",     WRITE_COMMAND,    DECIAML_2,          edcSOURCE6_NAME,                 NULL,        CLI_COM_NORMAL },        //T100_Simon_0006
    {eCLI_MAIN_SNS,     "SRC6",     READ_COMMAND,     DECIAML_2,          edcSOURCE6_NAME,                 NULL,        CLI_COM_NORMAL },        //T100_Simon_0006
    {eCLI_MAIN_SNS,     "SRC7",     WRITE_COMMAND,    DECIAML_2,          edcSOURCE7_NAME,                 NULL,        CLI_COM_NORMAL },        //T100_Simon_0006
    {eCLI_MAIN_SNS,     "SRC7",     READ_COMMAND,     DECIAML_2,          edcSOURCE7_NAME,                 NULL,        CLI_COM_NORMAL },        //T100_Simon_0006
    {eCLI_MAIN_SNS,     "SRC8",     WRITE_COMMAND,    DECIAML_2,          edcSOURCE8_NAME,                 NULL,        CLI_COM_NORMAL },        //T100_Simon_0006
    {eCLI_MAIN_SNS,     "SRC8",     READ_COMMAND,     DECIAML_2,          edcSOURCE8_NAME,                 NULL,        CLI_COM_NORMAL },        //T100_Simon_0006
    {eCLI_MAIN_SNS,     "SRC9",     WRITE_COMMAND,    DECIAML_2,          edcSOURCE9_NAME,                 NULL,        CLI_COM_NORMAL },        //T100_Simon_0006
    {eCLI_MAIN_SNS,     "SRC9",     READ_COMMAND,     DECIAML_2,          edcSOURCE9_NAME,                 NULL,        CLI_COM_NORMAL },        //T100_Simon_0006
    {eCLI_MAIN_SNS,     "SRCA",     WRITE_COMMAND,    DECIAML_2,          edcSOURCE10_NAME,                NULL,        CLI_COM_NORMAL },        //T100_Simon_0006
    {eCLI_MAIN_SNS,     "SRCA",     READ_COMMAND,     DECIAML_2,          edcSOURCE10_NAME,                NULL,        CLI_COM_NORMAL },        //T100_Simon_0006
    {eCLI_MAIN_SNS,     "SRCB",     WRITE_COMMAND,    DECIAML_2,          edcSOURCE11_NAME,                NULL,        CLI_COM_NORMAL },        //T100_Simon_0006
    {eCLI_MAIN_SNS,     "SRCB",     READ_COMMAND,     DECIAML_2,          edcSOURCE11_NAME,                NULL,        CLI_COM_NORMAL },        //T100_Simon_0006
    {eCLI_MAIN_SNS,     "SRCC",     WRITE_COMMAND,    DECIAML_2,          edcSOURCE12_NAME,                NULL,        CLI_COM_NORMAL },        //T100_Simon_0006
    {eCLI_MAIN_SNS,     "SRCC",     READ_COMMAND,     DECIAML_2,          edcSOURCE12_NAME,                NULL,        CLI_COM_NORMAL },        //T100_Simon_0006
    {eCLI_MAIN_SNS,     "SRCD",     WRITE_COMMAND,    DECIAML_2,          edcSOURCE13_NAME,                NULL,        CLI_COM_NORMAL },        //T100_Simon_0006
    {eCLI_MAIN_SNS,     "SRCD",     READ_COMMAND,     DECIAML_2,          edcSOURCE13_NAME,                NULL,        CLI_COM_NORMAL },        //T100_Simon_0006
    {eCLI_MAIN_SNS,     "SRCE",     WRITE_COMMAND,    DECIAML_2,          edcSOURCE14_NAME,                NULL,        CLI_COM_NORMAL },        //T100_Simon_0006
    {eCLI_MAIN_SNS,     "SRCE",     READ_COMMAND,     DECIAML_2,          edcSOURCE14_NAME,                NULL,        CLI_COM_NORMAL },        //T100_Simon_0006

	//DRD  //T100_Casper_0008
	{eCLI_MAIN_DRD, 	NULL,		WRITE_COMMAND,	  DECIAML_3,		  DATA_CODE_NA,		   utilGeneral_DRD,		CLI_COM_WRITE_ONLY},

	{eCLI_MAIN_DBG, 	NULL,		WRITE_COMMAND,	  DECIAML_3,		  DATA_CODE_NA,		   utilGeneral_Debug,		CLI_COM_NORMAL},         //T100_Simon_0024
    {eCLI_MAIN_DBG,     NULL,       READ_COMMAND,     DECIAML_3,          DATA_CODE_NA,        utilGeneral_Debug,      CLI_COM_NORMAL | CLI_COM_NO_REPLY},         //T100_Simon_0024

	{eCLI_MAIN_DBG, 	"SCHE",		WRITE_COMMAND,	  DECIAML_3,		  DATA_CODE_NA,		   utilGeneral_Debug_Schedule,		CLI_COM_NORMAL},         //T100_Coda_0014	//G100_Coda_0013
    {eCLI_MAIN_DBG,     "SCHE",     READ_COMMAND,     DECIAML_3,          DATA_CODE_NA,        utilGeneral_Debug_Schedule,      CLI_COM_NORMAL | CLI_COM_NO_REPLY},         //T100_Coda_0014


//	{eCLI_MAIN_DBG, 	"MEMC", 	WRITE_COMMAND,	  DECIAML_3,		  DATA_CODE_NA,		   utilGeneral_Debug_MEMC,		CLI_COM_NORMAL},		 //T100_Coda_0014
//	{eCLI_MAIN_DBG, 	"MEMC", 	READ_COMMAND,	  DECIAML_3,		  DATA_CODE_NA,		   utilGeneral_Debug_MEMC,		CLI_COM_NORMAL | CLI_COM_NO_REPLY}, 		//T100_Coda_0014

//	{eCLI_MAIN_DBG, 	"FEIC", 	WRITE_COMMAND,	  DECIAML_3,		  DATA_CODE_NA,		   utilGeneral_Debug_FrontEndBoard, 	CLI_COM_NORMAL},		 //T100_Coda_0014
//	{eCLI_MAIN_DBG, 	"FEIC", 	READ_COMMAND,	  DECIAML_3,		  DATA_CODE_NA,		   utilGeneral_Debug_FrontEndBoard, 	CLI_COM_NORMAL | CLI_COM_NO_REPLY}, 		//T100_Coda_0014

#if 0
	//{eCLI_MAIN_CAM, 	"STAT",		WRITE_COMMAND,	  DECIAML_3,		  DATA_CODE_NA,		              NULL,         CLI_COM_NORMAL},         //T100_Simon_0063
    //{eCLI_MAIN_CAM,     "STAT",     READ_COMMAND,     DECIAML_3,          DATA_CODE_NA,                     NULL,         CLI_COM_NORMAL},         //T100_Simon_0063
    //{eCLI_MAIN_CAM,     "QFUN",     WRITE_COMMAND,    DECIAML_3,          DATA_CODE_NA,                     NULL,         CLI_COM_NORMAL},         //T100_Simon_0063
    //{eCLI_MAIN_CAM,     "QFUN",     READ_COMMAND,     DECIAML_3,          DATA_CODE_NA,                     NULL,         CLI_COM_NORMAL},         //T100_Simon_0063
    {eCLI_MAIN_CAM,     "IPA1",     WRITE_COMMAND,    DECIAML_3,          edcCAMERA_IP_ADDRESS1,          NULL,         CLI_COM_NORMAL},         //T100_Simon_0063
    {eCLI_MAIN_CAM,     "IPA1",     READ_COMMAND,     DECIAML_3,          edcCAMERA_IP_ADDRESS1,          NULL,         CLI_COM_NORMAL},         //T100_Simon_0063
    {eCLI_MAIN_CAM,     "IPA2",     WRITE_COMMAND,    DECIAML_3,          edcCAMERA_IP_ADDRESS2,          NULL,         CLI_COM_NORMAL},         //T100_Simon_0063
    {eCLI_MAIN_CAM,     "IPA2",     READ_COMMAND,     DECIAML_3,          edcCAMERA_IP_ADDRESS2,          NULL,         CLI_COM_NORMAL},         //T100_Simon_0063
    {eCLI_MAIN_CAM,     "IPA3",     WRITE_COMMAND,    DECIAML_3,          edcCAMERA_IP_ADDRESS3,          NULL,         CLI_COM_NORMAL},         //T100_Simon_0063
    {eCLI_MAIN_CAM,     "IPA3",     READ_COMMAND,     DECIAML_3,          edcCAMERA_IP_ADDRESS3,          NULL,         CLI_COM_NORMAL},         //T100_Simon_0063
    {eCLI_MAIN_CAM,     "IPA4",     WRITE_COMMAND,    DECIAML_3,          edcCAMERA_IP_ADDRESS4,          NULL,         CLI_COM_NORMAL},         //T100_Simon_0063
    {eCLI_MAIN_CAM,     "IPA4",     READ_COMMAND,     DECIAML_3,          edcCAMERA_IP_ADDRESS4,          NULL,         CLI_COM_NORMAL},         //T100_Simon_0063
    {eCLI_MAIN_CAM,     "FVER",     WRITE_COMMAND,    DECIAML_3,          edcCAMERA_FW_VERSION,           NULL,         CLI_COM_NORMAL},         //T100_Simon_0063
    {eCLI_MAIN_CAM,     "FVER",     READ_COMMAND,     DECIAML_3,          edcCAMERA_FW_VERSION,           NULL,         CLI_COM_NORMAL},         //T100_Simon_0063
    {eCLI_MAIN_CAM,     "APLY",     WRITE_COMMAND,    DECIAML_3,          edcCAMERA_IP_APPLY,             NULL,         CLI_COM_NORMAL},         //T100_Simon_0063
    {eCLI_MAIN_CAM,     "APLY",     READ_COMMAND,     DECIAML_3,          edcCAMERA_IP_APPLY,             NULL,         CLI_COM_NORMAL},         //T100_Simon_0063
    {eCLI_MAIN_CAM,     "ETH0",     WRITE_COMMAND,    DECIAML_3,          edcCAMERA_IP_ADDR,              NULL,         CLI_COM_NORMAL},         //T100_Simon_0063
    {eCLI_MAIN_CAM,     "ETH0",     READ_COMMAND,     DECIAML_3,          edcCAMERA_IP_ADDR,              NULL,         CLI_COM_NORMAL},         //T100_Simon_0063
#endif

#if 0
	//Ichip debug
    {eCLI_MAIN_ICP,     NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,                     util_ICP,     CLI_COM_NORMAL | CLI_COM_NO_REPLY}, //A70LV_Larry_0086
    {eCLI_MAIN_ICP,     NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,                     util_ICP,     CLI_COM_NORMAL | CLI_COM_NO_REPLY}, //A70LV_Larry_0086

    //Ichip debug
    {eCLI_MAIN_ICD,     NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,                     util_ICD,     CLI_COM_NORMAL | CLI_COM_NO_REPLY},
    {eCLI_MAIN_ICD,     NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,                     util_ICD,     CLI_COM_NORMAL | CLI_COM_NO_REPLY},
#endif

    {eCLI_MAIN_LLM,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcLOW_LATENCY_MODE,            NULL,         CLI_COM_NORMAL },
    {eCLI_MAIN_LLM,     NULL,       READ_COMMAND,     DECIAML_2,          edcLOW_LATENCY_MODE,            NULL,         CLI_COM_NORMAL },

    {eCLI_MAIN_EBL,     "TOPE",     WRITE_COMMAND,    DECIAML_2,          edcBLENDING_TOP_ENABLE,           NULL,         CLI_COM_NORMAL},         //T100_Simon_0063
    {eCLI_MAIN_EBL,     "TOPE",     READ_COMMAND,     DECIAML_2,          edcBLENDING_TOP_ENABLE,           NULL,         CLI_COM_NORMAL},         //T100_Simon_0063
    {eCLI_MAIN_EBL,     "TOPS",     WRITE_COMMAND,    DECIAML_4,          edcBLENDING_TOP_START_PIXEL,      NULL,         CLI_COM_NORMAL},         //T100_Simon_0063
    {eCLI_MAIN_EBL,     "TOPS",     READ_COMMAND,     DECIAML_4,          edcBLENDING_TOP_START_PIXEL,      NULL,         CLI_COM_NORMAL},         //T100_Simon_0063
    {eCLI_MAIN_EBL,     "TOPW",     WRITE_COMMAND,    DECIAML_4,          edcBLENDING_TOP_PIXEL_WIDTH,      NULL,         CLI_COM_NORMAL},         //T100_Simon_0063
    {eCLI_MAIN_EBL,     "TOPW",     READ_COMMAND,     DECIAML_4,          edcBLENDING_TOP_PIXEL_WIDTH,      NULL,         CLI_COM_NORMAL},         //T100_Simon_0063

    {eCLI_MAIN_EBL,     "BTME",     WRITE_COMMAND,    DECIAML_2,          edcBLENDING_BOTTOM_ENABLE,        NULL,         CLI_COM_NORMAL},         //T100_Simon_0063
    {eCLI_MAIN_EBL,     "BTME",     READ_COMMAND,     DECIAML_2,          edcBLENDING_BOTTOM_ENABLE,        NULL,         CLI_COM_NORMAL},         //T100_Simon_0063
    {eCLI_MAIN_EBL,     "BTMS",     WRITE_COMMAND,    DECIAML_4,          edcBLENDING_BOTTOM_START_PIXEL,   NULL,         CLI_COM_NORMAL},         //T100_Simon_0063
    {eCLI_MAIN_EBL,     "BTMS",     READ_COMMAND,     DECIAML_4,          edcBLENDING_BOTTOM_START_PIXEL,   NULL,         CLI_COM_NORMAL},         //T100_Simon_0063
    {eCLI_MAIN_EBL,     "BTMW",     WRITE_COMMAND,    DECIAML_4,          edcBLENDING_BOTTOM_PIXEL_WIDTH,   NULL,         CLI_COM_NORMAL},         //T100_Simon_0063
    {eCLI_MAIN_EBL,     "BTMW",     READ_COMMAND,     DECIAML_4,          edcBLENDING_BOTTOM_PIXEL_WIDTH,   NULL,         CLI_COM_NORMAL},         //T100_Simon_0063

    {eCLI_MAIN_EBL,     "LFTE",     WRITE_COMMAND,    DECIAML_2,          edcBLENDING_LEFT_ENABLE,          NULL,         CLI_COM_NORMAL},         //T100_Simon_0063
    {eCLI_MAIN_EBL,     "LFTE",     READ_COMMAND,     DECIAML_2,          edcBLENDING_LEFT_ENABLE,          NULL,         CLI_COM_NORMAL},         //T100_Simon_0063
    {eCLI_MAIN_EBL,     "LFTS",     WRITE_COMMAND,    DECIAML_4,          edcBLENDING_LEFT_START_PIXEL,     NULL,         CLI_COM_NORMAL},         //T100_Simon_0063
    {eCLI_MAIN_EBL,     "LFTS",     READ_COMMAND,     DECIAML_4,          edcBLENDING_LEFT_START_PIXEL,     NULL,         CLI_COM_NORMAL},         //T100_Simon_0063
    {eCLI_MAIN_EBL,     "LFTW",     WRITE_COMMAND,    DECIAML_4,          edcBLENDING_LEFT_PIXEL_WIDTH,     NULL,         CLI_COM_NORMAL},         //T100_Simon_0063
    {eCLI_MAIN_EBL,     "LFTW",     READ_COMMAND,     DECIAML_4,          edcBLENDING_LEFT_PIXEL_WIDTH,     NULL,         CLI_COM_NORMAL},         //T100_Simon_0063

    {eCLI_MAIN_EBL,     "RHTE",     WRITE_COMMAND,    DECIAML_2,          edcBLENDING_RIGHT_ENABLE,         NULL,         CLI_COM_NORMAL},         //T100_Simon_0063
    {eCLI_MAIN_EBL,     "RHTE",     READ_COMMAND,     DECIAML_2,          edcBLENDING_RIGHT_ENABLE,         NULL,         CLI_COM_NORMAL},         //T100_Simon_0063
    {eCLI_MAIN_EBL,     "RHTS",     WRITE_COMMAND,    DECIAML_4,          edcBLENDING_RIGHT_START_PIXEL,    NULL,         CLI_COM_NORMAL},         //T100_Simon_0063
    {eCLI_MAIN_EBL,     "RHTS",     READ_COMMAND,     DECIAML_4,          edcBLENDING_RIGHT_START_PIXEL,    NULL,         CLI_COM_NORMAL},         //T100_Simon_0063
    {eCLI_MAIN_EBL,     "RHTW",     WRITE_COMMAND,    DECIAML_4,          edcBLENDING_RIGHT_PIXEL_WIDTH,    NULL,         CLI_COM_NORMAL},         //T100_Simon_0063
    {eCLI_MAIN_EBL,     "RHTW",     READ_COMMAND,     DECIAML_4,          edcBLENDING_RIGHT_PIXEL_WIDTH,    NULL,         CLI_COM_NORMAL},         //T100_Simon_0063

    //{eCLI_MAIN_EBL,     "GAMA",     WRITE_COMMAND,    DECIAML_2,          edcBLENDING_GAMMA,                NULL,         CLI_COM_NORMAL},         //T100_Simon_0063
    //{eCLI_MAIN_EBL,     "GAMA",     READ_COMMAND,     DECIAML_2,          edcBLENDING_GAMMA,                NULL,         CLI_COM_NORMAL},         //T100_Simon_0063
    {eCLI_MAIN_EBL,     "GAMA",     WRITE_COMMAND,    DECIAML_2,          edcADV_BLEND_GAMMA,                NULL,         CLI_COM_NORMAL},         //G100_Doulas_0027
    {eCLI_MAIN_EBL,     "GAMA",     READ_COMMAND,     DECIAML_2,          edcADV_BLEND_GAMMA,                NULL,         CLI_COM_NORMAL},         //G100_Doulas_0027


    //FKC============================================================================================================//FKC
    {eCLI_MAIN_FKC,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcCOMPATIBLE_4K,                NULL,        CLI_COM_NORMAL },   //T100_Coda
    {eCLI_MAIN_FKC,     NULL,       READ_COMMAND,     DECIAML_2,          edcCOMPATIBLE_4K,                NULL,        CLI_COM_NORMAL },   //T100_Coda

    //FVI============================================================================================================//FVI
    {eCLI_MAIN_FVI,     NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,         	utilGeneral_FVI,         CLI_COM_NO_REPLY|CLI_COM_READ_ONLY},
    {eCLI_MAIN_FVI,     NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,         	utilGeneral_FVI,         CLI_COM_NO_REPLY|CLI_COM_READ_ONLY},

    //CCT============================================================================================================//CCT
    {eCLI_MAIN_CCT,     NULL,       WRITE_COMMAND,    DECIAML_3,          edcCOLOR_OFFSET,                 NULL,        CLI_COM_NORMAL | CLI_COM_NEED_SERVICE_CODE_WRITE},
    {eCLI_MAIN_CCT,     NULL,       READ_COMMAND,     DECIAML_3,          edcCOLOR_OFFSET,                 NULL,        CLI_COM_NORMAL | CLI_COM_NEED_SERVICE_CODE_WRITE},

	//HOE============================================================================================================//HOE //T100IR_Casper_0032
	{eCLI_MAIN_HOE, 	NULL,		WRITE_COMMAND,	  DECIAML_2,		  edcHDMI_EDID_1, 			   		NULL,		CLI_COM_NORMAL },
	{eCLI_MAIN_HOE, 	NULL,		READ_COMMAND,	  DECIAML_2,		  edcHDMI_EDID_1, 			   		NULL,		CLI_COM_NORMAL },

	//HSE============================================================================================================//HSE //T100IR_Casper_0032
	{eCLI_MAIN_HSE, 	NULL,		WRITE_COMMAND,	  DECIAML_2,		  edcHDMI_EDID_2, 			   		NULL,		CLI_COM_NORMAL },
	{eCLI_MAIN_HSE, 	NULL,		READ_COMMAND,	  DECIAML_2,		  edcHDMI_EDID_2, 			   		NULL,		CLI_COM_NORMAL },

	/*
	//HBE============================================================================================================//HBE //T100IR_Casper_0032
	{eCLI_MAIN_HBE, 	NULL,		WRITE_COMMAND,	  DECIAML_2,		  edcHDBASET_EDID, 			   		NULL,		CLI_COM_NORMAL },
	{eCLI_MAIN_HBE, 	NULL,		READ_COMMAND,	  DECIAML_2,		  edcHDBASET_EDID, 			   		NULL,		CLI_COM_NORMAL },
	*/

    //DYBK============================================================================================================//DYBK //G100_Wilsonj_0032
    {eCLI_MAIN_DYBK,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcDYNAMIC_BLACK,                 NULL,       CLI_COM_NORMAL },
    {eCLI_MAIN_DYBK,    NULL,       READ_COMMAND,     DECIAML_2,          edcDYNAMIC_BLACK,                 NULL,       CLI_COM_NORMAL },

    //EXBK============================================================================================================//EXBK //G100_Wilsonj_0032
    {eCLI_MAIN_EXBK,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcREAL_BLACK,                    NULL,       CLI_COM_NORMAL },
    {eCLI_MAIN_EXBK,    NULL,       READ_COMMAND,     DECIAML_2,          edcREAL_BLACK,                    NULL,       CLI_COM_NORMAL },

    //DCSP============================================================================================================//DCSP //G100_Wilsonj_0032
    {eCLI_MAIN_DCSP,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcDB_SPEED,                      NULL,       CLI_COM_NORMAL },
    {eCLI_MAIN_DCSP,    NULL,       READ_COMMAND,     DECIAML_2,          edcDB_SPEED,                      NULL,       CLI_COM_NORMAL },

    //DCST============================================================================================================//DCST //G100_Wilsonj_0032
    {eCLI_MAIN_DCST,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcDB_STRENGTH,                   NULL,       CLI_COM_NORMAL },
    {eCLI_MAIN_DCST,    NULL,       READ_COMMAND,     DECIAML_2,          edcDB_STRENGTH,                   NULL,       CLI_COM_NORMAL },

    //DCLE============================================================================================================//DCLE //G100_Wilsonj_0032
    {eCLI_MAIN_DCLE,    NULL,       WRITE_COMMAND,    DECIAML_3,          edcDB_LIGHT_LEVEL,                NULL,       CLI_COM_NORMAL },
    {eCLI_MAIN_DCLE,    NULL,       READ_COMMAND,     DECIAML_3,          edcDB_LIGHT_LEVEL,                NULL,       CLI_COM_NORMAL },

    //DCLT============================================================================================================//DCLT //G100_Wilsonj_0032
    {eCLI_MAIN_DCLT,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcLIGHTS_OUT_TIMER_X05,          NULL,       CLI_COM_NORMAL },
    {eCLI_MAIN_DCLT,    NULL,       READ_COMMAND,     DECIAML_2,          edcLIGHTS_OUT_TIMER_X05,          NULL,       CLI_COM_NORMAL },

    //DCLS============================================================================================================//DCLS //G100_Wilsonj_0032
    {eCLI_MAIN_DCLS,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcLIGHTS_OUT_SIGNAL_LEVEL,       NULL,       CLI_COM_NORMAL },
    {eCLI_MAIN_DCLS,    NULL,       READ_COMMAND,     DECIAML_2,          edcLIGHTS_OUT_SIGNAL_LEVEL,       NULL,       CLI_COM_NORMAL },

    //MSRC============================================================================================================//MSRC //G100_Wilsonj_0032
    {eCLI_MAIN_MSRC,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcMAIN_INPUT,                    NULL,       CLI_COM_NORMAL },
    {eCLI_MAIN_MSRC,    NULL,       READ_COMMAND,     DECIAML_2,          edcMAIN_INPUT,                    NULL,       CLI_COM_NORMAL },

    //SSRC============================================================================================================//SSRC //G100_Wilsonj_0032
    {eCLI_MAIN_SSRC,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcSUB_INPUT,                     NULL,       CLI_COM_NORMAL },
    {eCLI_MAIN_SSRC,    NULL,       READ_COMMAND,     DECIAML_2,          edcSUB_INPUT,                     NULL,       CLI_COM_NORMAL },

    //EDMA============================================================================================================//EDMA //G100_Wilsonj_0033
    {eCLI_MAIN_EDMA,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcEDGE_MASK,                     NULL,       CLI_COM_NORMAL },
    {eCLI_MAIN_EDMA,    NULL,       READ_COMMAND,     DECIAML_2,          edcEDGE_MASK,                     NULL,       CLI_COM_NORMAL },

    //IST============================================================================================================//IST   //G100_Julie_0001
    {eCLI_MAIN_IST,     NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,            utilGeneral_IST,    CLI_COM_NORMAL | CLI_COM_NO_REPLY},
    {eCLI_MAIN_IST,     NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,            utilGeneral_IST,    CLI_COM_NORMAL | CLI_COM_NO_REPLY},

    //SPON============================================================================================================//SPON //G100_Wilsonj_0040
    {eCLI_MAIN_SPON,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcSIGNAL_POWER_ON,               NULL,       CLI_COM_NORMAL },
    {eCLI_MAIN_SPON,    NULL,       READ_COMMAND,     DECIAML_2,          edcSIGNAL_POWER_ON,               NULL,       CLI_COM_NORMAL },

    //PINM============================================================================================================//PINM //G100_Wilsonj_0041
    {eCLI_MAIN_PINM,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcSECURITY_TIMER_MONTH,          NULL,       CLI_COM_NORMAL },
    {eCLI_MAIN_PINM,    NULL,       READ_COMMAND,     DECIAML_2,          edcSECURITY_TIMER_MONTH,          NULL,       CLI_COM_NORMAL },

    //PIND============================================================================================================//PIND //G100_Wilsonj_0041
    {eCLI_MAIN_PIND,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcSECURITY_TIMER_DAY,            NULL,       CLI_COM_NORMAL },
    {eCLI_MAIN_PIND,    NULL,       READ_COMMAND,     DECIAML_2,          edcSECURITY_TIMER_DAY,            NULL,       CLI_COM_NORMAL },

    //PINH============================================================================================================//PINH //G100_Wilsonj_0041
    {eCLI_MAIN_PINH,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcSECURITY_TIMER_HOUR,           NULL,       CLI_COM_NORMAL },
    {eCLI_MAIN_PINH,    NULL,       READ_COMMAND,     DECIAML_2,          edcSECURITY_TIMER_HOUR,           NULL,       CLI_COM_NORMAL },

    //LPTH============================================================================================================//LPTH //G100_Wilsonj_0042
    {eCLI_MAIN_LPTH,    NULL,       READ_COMMAND,     DECIAML_2,          edcTOTAL_PROJECTOR_HOURS_INFO,    NULL,       CLI_COM_READ_ONLY},
    {eCLI_MAIN_LPTH,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcTOTAL_PROJECTOR_HOURS_INFO,    NULL,       CLI_COM_READ_ONLY},

    //LEPT============================================================================================================//LEPT //G100_Wilsonj_0042
    {eCLI_MAIN_LEPT,    NULL,       READ_COMMAND,   STRING_CHAR,          DATA_CODE_NA,           utilGeneral_LEPT,       CLI_COM_READ_ONLY}, //G100_Tim_002, mod //A35G2_BRC_Casper_0147
    {eCLI_MAIN_LEPT,    NULL,       WRITE_COMMAND,  STRING_CHAR,          DATA_CODE_NA,           utilGeneral_LEPT,       CLI_COM_READ_ONLY}, //G100_Tim_002, mod //A35G2_BRC_Casper_0147

    //SSTP============================================================================================================//SSTP //G100_Wilsonj_0042
    {eCLI_MAIN_SSTP,    NULL,       READ_COMMAND,     DECIAML_2,          edcSYSTEM_TEMPERATURE,            NULL,       CLI_COM_READ_ONLY},
    {eCLI_MAIN_SSTP,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcSYSTEM_TEMPERATURE,            NULL,       CLI_COM_READ_ONLY},

    //SSAC============================================================================================================//SSAC //G100_Wilsonj_0046
    //{eCLI_MAIN_SSAC,    NULL,       READ_COMMAND,     DECIAML_2,          edcAC_VOLTAGE_INFO,               NULL,       CLI_COM_READ_ONLY},
    //{eCLI_MAIN_SSAC,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcAC_VOLTAGE_INFO,               NULL,       CLI_COM_READ_ONLY},

    //LSNT============================================================================================================//LSNT //G100_Larry_0006
    {eCLI_MAIN_LSNT,    NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,           utilGeneral_LSNT,     CLI_COM_NORMAL | CLI_COM_NEED_SERVICE_CODE_WRITE},
    {eCLI_MAIN_LSNT,    NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,           utilGeneral_LSNT,     CLI_COM_NORMAL | CLI_COM_NEED_SERVICE_CODE_WRITE},

    //LSET============================================================================================================//LSET //G100_Larry_0006
    {eCLI_MAIN_LSET,    NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,           utilGeneral_LSET,     CLI_COM_NORMAL | CLI_COM_NEED_SERVICE_CODE_WRITE},
    {eCLI_MAIN_LSET,    NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,           utilGeneral_LSET,     CLI_COM_NORMAL | CLI_COM_NEED_SERVICE_CODE_WRITE},

    //LSQT============================================================================================================//LSQT //G100_Larry_0006
    {eCLI_MAIN_LSQT,    NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,           utilGeneral_LSQT,     CLI_COM_NORMAL | CLI_COM_NEED_SERVICE_CODE_WRITE},
    {eCLI_MAIN_LSQT,    NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,           utilGeneral_LSQT,     CLI_COM_NORMAL | CLI_COM_NEED_SERVICE_CODE_WRITE},

    //LSCT============================================================================================================//LSCT //G100_Larry_0006
    {eCLI_MAIN_LSCT,    NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,           utilGeneral_LSCT,     CLI_COM_NORMAL | CLI_COM_NEED_SERVICE_CODE_WRITE},
    {eCLI_MAIN_LSCT,    NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,           utilGeneral_LSCT,     CLI_COM_NORMAL | CLI_COM_NEED_SERVICE_CODE_WRITE},

    //PPZM============================================================================================================//PPZM //G100_Wilsonj_0060
    {eCLI_MAIN_PPZM,    NULL,       READ_COMMAND,     DECIAML_2,          edcDIGITAL_ZOOM_PROPORTIONAL,     NULL,       CLI_COM_NORMAL},
    {eCLI_MAIN_PPZM,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcDIGITAL_ZOOM_PROPORTIONAL,     NULL,       CLI_COM_NORMAL},

    //HKSA============================================================================================================//HKSA //G100_Wilsonj_0060
    {eCLI_MAIN_HKST,    NULL,       READ_COMMAND,     DECIAML_2,          edcHOT_KEY_SETTINGS,              NULL,       CLI_COM_NORMAL},
    {eCLI_MAIN_HKST,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcHOT_KEY_SETTINGS,              NULL,       CLI_COM_NORMAL},

    //HKSB============================================================================================================//HKSB //G100_Wilsonj_0060
    {eCLI_MAIN_HKSB,    NULL,       READ_COMMAND,     DECIAML_2,          edcHOT_KEY2_SETTINGS,             NULL,       CLI_COM_NORMAL},
    {eCLI_MAIN_HKSB,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcHOT_KEY2_SETTINGS,             NULL,       CLI_COM_NORMAL},

    //OPD============================================================================================================//OPD
    {eCLI_MAIN_OPD,     NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,            utilGeneral_OPD,      CLI_COM_NORMAL| CLI_COM_NO_REPLY},
    {eCLI_MAIN_OPD,     NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,            utilGeneral_OPD,      CLI_COM_NORMAL| CLI_COM_NO_REPLY},
    {eCLI_MAIN_OPD,     "REST",     READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,            utilGeneral_OPDRSET,  CLI_COM_WRITE_ONLY},
    {eCLI_MAIN_OPD,     "REST",     WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,            utilGeneral_OPDRSET,  CLI_COM_NORMAL},

    //TARG============================================================================================================//TARG  //G100_Julie_00020
    {eCLI_MAIN_TARG,     NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,           utilGeneral_TARG,     CLI_COM_NORMAL| CLI_COM_NO_REPLY},
    {eCLI_MAIN_TARG,     NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,           utilGeneral_TARG,     CLI_COM_NORMAL| CLI_COM_NO_REPLY},

    //CREN============================================================================================================//CREN //G100_Wilsonj_0061
    {eCLI_MAIN_CREN,    NULL,       READ_COMMAND,     DECIAML_2,          edcCrestron,                      NULL,        CLI_COM_NORMAL},
    {eCLI_MAIN_CREN,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcCrestron,                      NULL,        CLI_COM_NORMAL},

    //CRIP============================================================================================================//CRIP //G100_Wilsonj_0061
    {eCLI_MAIN_CRIP,    NULL,       READ_COMMAND,     DECIAML_2,          edcCRESTRON_ADDRESS,              NULL,        CLI_COM_NORMAL},
    {eCLI_MAIN_CRIP,    NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,         utilGeneral_CRIP,        CLI_COM_NORMAL},

    //CRID============================================================================================================//CRID //G100_Wilsonj_0061
    {eCLI_MAIN_CRID,    NULL,       READ_COMMAND,     DECIAML_2,          edcCRESTRON_PID,                  NULL,        CLI_COM_NORMAL},
    {eCLI_MAIN_CRID,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcCRESTRON_PID,                  NULL,        CLI_COM_NORMAL},

    //CRPO============================================================================================================//CRPO //G100_Wilsonj_0061
    {eCLI_MAIN_CRPO,    NULL,       READ_COMMAND,     DECIAML_2,          edcCRESTRON_PORT,                 NULL,        CLI_COM_NORMAL},
    {eCLI_MAIN_CRPO,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcCRESTRON_PORT,                 NULL,        CLI_COM_NORMAL},

    //CRAP============================================================================================================//CRAP //G100_Wilsonj_0061
    {eCLI_MAIN_CRAP,    NULL,       READ_COMMAND,     DECIAML_2,          edcCRESTRON_APPLY,                NULL,        CLI_COM_WRITE_ONLY},
    {eCLI_MAIN_CRAP,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcCRESTRON_APPLY,                NULL,        CLI_COM_WRITE_ONLY},

    //EXEN============================================================================================================//EXEN //G100_Wilsonj_0061
    {eCLI_MAIN_EXEN,    NULL,       READ_COMMAND,     DECIAML_2,          edcExtron,                        NULL,        CLI_COM_NORMAL},
    {eCLI_MAIN_EXEN,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcExtron,                        NULL,        CLI_COM_NORMAL},

    //PJEN============================================================================================================//PJEN //G100_Wilsonj_0061
    {eCLI_MAIN_PJEN,    NULL,       READ_COMMAND,     DECIAML_2,          edcPJ_Link,                       NULL,        CLI_COM_NORMAL},
    {eCLI_MAIN_PJEN,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcPJ_Link,                       NULL,        CLI_COM_NORMAL},

    //PJIP============================================================================================================//PJIP //G100_Wilsonj_0061
    {eCLI_MAIN_PJIP,    NULL,       READ_COMMAND,     DECIAML_2,          edcPJLINK_ADDRESS,                NULL,        CLI_COM_NORMAL},
    {eCLI_MAIN_PJIP,    NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,         utilGeneral_PJIP,        CLI_COM_NORMAL},

    //PJAP============================================================================================================//PJAP //G100_Wilsonj_0061
    {eCLI_MAIN_PJAP,    NULL,       READ_COMMAND,     DECIAML_2,          edcPJLINK_APPLY,                  NULL,        CLI_COM_WRITE_ONLY},
    {eCLI_MAIN_PJAP,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcPJLINK_APPLY,                  NULL,        CLI_COM_WRITE_ONLY},

    //AXEN============================================================================================================//AXEN //G100_Wilsonj_0061
    {eCLI_MAIN_AXEN,    NULL,       READ_COMMAND,     DECIAML_2,          edcAMX,                           NULL,        CLI_COM_NORMAL},
    {eCLI_MAIN_AXEN,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcAMX,                           NULL,        CLI_COM_NORMAL},

    //TEEN============================================================================================================//TEEN //G100_Wilsonj_0061
    {eCLI_MAIN_TEEN,    NULL,       READ_COMMAND,     DECIAML_2,          edcTelnet,                        NULL,        CLI_COM_NORMAL},
    {eCLI_MAIN_TEEN,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcTelnet,                        NULL,        CLI_COM_NORMAL},

    //HTEN============================================================================================================//HTEN //G100_Wilsonj_0061
    {eCLI_MAIN_HTEN,    NULL,       READ_COMMAND,     DECIAML_2,          edcHTTP,                          NULL,        CLI_COM_NORMAL},
    {eCLI_MAIN_HTEN,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcHTTP,                          NULL,        CLI_COM_NORMAL},

    //CNRT============================================================================================================//CNRT //G100_Owen_0055
    {eCLI_MAIN_CNRT,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcNETWORK_CONTROL_RESET,         NULL,        CLI_COM_WRITE_ONLY},

    //LSAT============================================================================================================//LSAT //G100_Larry_0007
    {eCLI_MAIN_LSHS,    NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,           utilGeneral_LSAT,      CLI_COM_READ_ONLY},
    {eCLI_MAIN_LSHS,    NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,           utilGeneral_LSAT,      CLI_COM_READ_ONLY},

    //PMUT============================================================================================================//PMUT //G100_Larry_0007
    {eCLI_MAIN_PMUT,    NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,           utilGeneral_PMUT,      CLI_COM_NORMAL},
    {eCLI_MAIN_PMUT,    NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,           utilGeneral_PMUT,      CLI_COM_NORMAL},

    //SHFI============================================================================================================//SHFI //G100_Larry_0007
    {eCLI_MAIN_SHFI,    NULL,       READ_COMMAND,     DECIAML_2,          edcFADE_IN,                       NULL,        CLI_COM_NORMAL},
    {eCLI_MAIN_SHFI,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcFADE_IN,                       NULL,        CLI_COM_NORMAL},

    //SHFO============================================================================================================//SHFO //G100_Larry_0007
    {eCLI_MAIN_SHFO,    NULL,       READ_COMMAND,     DECIAML_2,          edcFADE_OUT,                      NULL,        CLI_COM_NORMAL},
    {eCLI_MAIN_SHFO,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcFADE_OUT,                      NULL,        CLI_COM_NORMAL},

    //SCRT============================================================================================================//SCRT
    {eCLI_MAIN_SCRT,    NULL,       READ_COMMAND,     DECIAML_2,          edcDIGITAL_ZOOM_RESET,            NULL,        CLI_COM_WRITE_ONLY},
    {eCLI_MAIN_SCRT,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcDIGITAL_ZOOM_RESET,            NULL,        CLI_COM_WRITE_ONLY},

    //GCRT============================================================================================================//GCRT
    {eCLI_MAIN_GCRT,    NULL,       READ_COMMAND,     DECIAML_2,          edcWARP_RESET,                    NULL,        CLI_COM_WRITE_ONLY},
    {eCLI_MAIN_GCRT,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcWARP_RESET,                    NULL,        CLI_COM_WRITE_ONLY | CLI_COM_CHECK_BUSY},

    //LMNS============================================================================================================//LMNS
    {eCLI_MAIN_LMNS,    NULL,       READ_COMMAND,     DECIAML_2,          edcLAN_STATUS,                    NULL,        CLI_COM_READ_ONLY},
    {eCLI_MAIN_LMNS,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcLAN_STATUS,                    NULL,        CLI_COM_READ_ONLY},

//    //LCCR============================================================================================================//LCCR
//    {eCLI_MAIN_LCCR,    NULL,       READ_COMMAND,     DECIAML_2,          edcCrestron,                      NULL,        CLI_COM_NORMAL},
//    {eCLI_MAIN_LCCR,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcCrestron,                      NULL,        CLI_COM_NORMAL},

//    //LCEX============================================================================================================//LCEX
//    {eCLI_MAIN_LCEX,    NULL,       READ_COMMAND,     DECIAML_2,          edcExtron,                        NULL,        CLI_COM_NORMAL},
//    {eCLI_MAIN_LCEX,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcExtron,                        NULL,        CLI_COM_NORMAL},

//    //LCPJ============================================================================================================//LCPJ
//    {eCLI_MAIN_LCPJ,    NULL,       READ_COMMAND,     DECIAML_2,          edcPJ_Link,                       NULL,        CLI_COM_NORMAL},
//    {eCLI_MAIN_LCPJ,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcPJ_Link,                       NULL,        CLI_COM_NORMAL},

//    //LCAX============================================================================================================//LCAX
//    {eCLI_MAIN_LCAX,    NULL,       READ_COMMAND,     DECIAML_2,          edcAMX,                           NULL,        CLI_COM_NORMAL},
//    {eCLI_MAIN_LCAX,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcAMX,                           NULL,        CLI_COM_NORMAL},

//    //LCTE============================================================================================================//LCTE
//    {eCLI_MAIN_LCTE,    NULL,       READ_COMMAND,     DECIAML_2,          edcTelnet,                        NULL,        CLI_COM_NORMAL},
//    {eCLI_MAIN_LCTE,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcTelnet,                        NULL,        CLI_COM_NORMAL},

//    //LCHT============================================================================================================//LCHT
//    {eCLI_MAIN_LCHT,    NULL,       READ_COMMAND,     DECIAML_2,          edcHTTP,                          NULL,        CLI_COM_NORMAL},
//    {eCLI_MAIN_LCHT,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcHTTP,                          NULL,        CLI_COM_NORMAL},

    //LANI============================================================================================================//LANI
    {eCLI_MAIN_LANI,    NULL,       READ_COMMAND,     DECIAML_2,          edcLAN_PATH_SWITCH,               NULL,        CLI_COM_NORMAL},
    {eCLI_MAIN_LANI,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcLAN_PATH_SWITCH,               NULL,        CLI_COM_NORMAL},

    //SYRT============================================================================================================//SYRT
    {eCLI_MAIN_SYRT,    NULL,       READ_COMMAND,     DECIAML_2,          edcSYSTEM_RESET,                  NULL,        CLI_COM_WRITE_ONLY},
    {eCLI_MAIN_SYRT,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcSYSTEM_RESET,                  NULL,        CLI_COM_WRITE_ONLY | CLI_COM_CHECK_BUSY},

    //FRST============================================================================================================//FRST
    {eCLI_MAIN_FRST,    NULL,       READ_COMMAND,     DECIAML_2,          edcFACTORY_RESET,                 NULL,        CLI_COM_WRITE_ONLY},
    {eCLI_MAIN_FRST,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcFACTORY_RESET,                 NULL,        CLI_COM_WRITE_ONLY | CLI_COM_CHECK_BUSY},

    //DPRT============================================================================================================//DPRT
    {eCLI_MAIN_DPRT,    NULL,       READ_COMMAND,     DECIAML_2,          edcINSTALLATION_RESET,            NULL,        CLI_COM_WRITE_ONLY},
    {eCLI_MAIN_DPRT,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcINSTALLATION_RESET,            NULL,        CLI_COM_WRITE_ONLY | CLI_COM_CHECK_BUSY},

    //IMRT============================================================================================================//IMRT
    {eCLI_MAIN_IMRT,    NULL,       READ_COMMAND,     DECIAML_2,          edcImageReset,                    NULL,        CLI_COM_WRITE_ONLY},
    {eCLI_MAIN_IMRT,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcImageReset,                    NULL,        CLI_COM_WRITE_ONLY | CLI_COM_CHECK_BUSY},

    //CMRT============================================================================================================//CMRT
    {eCLI_MAIN_CMRT,    NULL,       READ_COMMAND,     DECIAML_2,          edcCOMMUNICATION_RESET,           NULL,        CLI_COM_WRITE_ONLY},
    {eCLI_MAIN_CMRT,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcCOMMUNICATION_RESET,           NULL,        CLI_COM_WRITE_ONLY},

    //PSRT============================================================================================================//PSRT
    {eCLI_MAIN_PSRT,    NULL,       READ_COMMAND,     DECIAML_2,          edcPOWER_SETTINGS_RESET,          NULL,        CLI_COM_WRITE_ONLY},
    {eCLI_MAIN_PSRT,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcPOWER_SETTINGS_RESET,          NULL,        CLI_COM_WRITE_ONLY | CLI_COM_CHECK_BUSY},

    //BKLV   //blacklevel
    {eCLI_MAIN_BKLV,    "ASEL",     READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,                     utilGeneral_BKLV_ASEL,        CLI_COM_NORMAL},
    {eCLI_MAIN_BKLV,    "ASEL",     WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,                     utilGeneral_BKLV_ASEL,        CLI_COM_NORMAL},
    {eCLI_MAIN_BKLV,    "ENAB",     READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,                     utilGeneral_BKLV_ENAB,        CLI_COM_NORMAL},
    {eCLI_MAIN_BKLV,    "ENAB",     WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,                     utilGeneral_BKLV_ENAB,        CLI_COM_NORMAL},
    {eCLI_MAIN_BKLV,    "TLCX",     READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,                     utilGeneral_BKLV_TLCX,        CLI_COM_NORMAL},
    {eCLI_MAIN_BKLV,    "TLCX",     WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,                     utilGeneral_BKLV_TLCX,        CLI_COM_NORMAL},
    {eCLI_MAIN_BKLV,    "TLCY",     READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,                     utilGeneral_BKLV_TLCY,        CLI_COM_NORMAL},
    {eCLI_MAIN_BKLV,    "TLCY",     WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,                     utilGeneral_BKLV_TLCY,        CLI_COM_NORMAL},
    {eCLI_MAIN_BKLV,    "TRCX",     READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,                     utilGeneral_BKLV_TRCX,        CLI_COM_NORMAL},
    {eCLI_MAIN_BKLV,    "TRCX",     WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,                     utilGeneral_BKLV_TRCX,        CLI_COM_NORMAL},
    {eCLI_MAIN_BKLV,    "TRCY",     READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,                     utilGeneral_BKLV_TRCY,        CLI_COM_NORMAL},
    {eCLI_MAIN_BKLV,    "TRCY",     WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,                     utilGeneral_BKLV_TRCY,        CLI_COM_NORMAL},
    {eCLI_MAIN_BKLV,    "BLCX",     READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,                     utilGeneral_BKLV_BLCX,        CLI_COM_NORMAL},
    {eCLI_MAIN_BKLV,    "BLCX",     WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,                     utilGeneral_BKLV_BLCX,        CLI_COM_NORMAL},
    {eCLI_MAIN_BKLV,    "BLCY",     READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,                     utilGeneral_BKLV_BLCY,        CLI_COM_NORMAL},
    {eCLI_MAIN_BKLV,    "BLCY",     WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,                     utilGeneral_BKLV_BLCY,        CLI_COM_NORMAL},
    {eCLI_MAIN_BKLV,    "BRCX",     READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,                     utilGeneral_BKLV_BRCX,        CLI_COM_NORMAL},
    {eCLI_MAIN_BKLV,    "BRCX",     WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,                     utilGeneral_BKLV_BRCX,        CLI_COM_NORMAL},
    {eCLI_MAIN_BKLV,    "BRCY",     READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,                     utilGeneral_BKLV_BRCY,        CLI_COM_NORMAL},
    {eCLI_MAIN_BKLV,    "BRCY",     WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,                     utilGeneral_BKLV_BRCY,        CLI_COM_NORMAL},
    {eCLI_MAIN_BKLV,    "RLVL",     READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,                     utilGeneral_BKLV_RLVL,        CLI_COM_NORMAL},
    {eCLI_MAIN_BKLV,    "RLVL",     WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,                     utilGeneral_BKLV_RLVL,        CLI_COM_NORMAL},
    {eCLI_MAIN_BKLV,    "GLVL",     READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,                     utilGeneral_BKLV_GLVL,        CLI_COM_NORMAL},
    {eCLI_MAIN_BKLV,    "GLVL",     WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,                     utilGeneral_BKLV_GLVL,        CLI_COM_NORMAL},
    {eCLI_MAIN_BKLV,    "BLVL",     READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,                     utilGeneral_BKLV_BLVL,        CLI_COM_NORMAL},
    {eCLI_MAIN_BKLV,    "BLVL",     WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,                     utilGeneral_BKLV_BLVL,        CLI_COM_NORMAL},
    {eCLI_MAIN_BKLV,    "APLY",     READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,                     utilGeneral_BKLV_APLY,        CLI_COM_NORMAL},
    {eCLI_MAIN_BKLV,    "APLY",     WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,                     utilGeneral_BKLV_APLY,        CLI_COM_NORMAL},
    {eCLI_MAIN_BKLV,    "RSET",     WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,                     utilGeneral_BKLV_RSET,        CLI_COM_NORMAL},
    {eCLI_MAIN_BKLV,    "SAVE",     WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,                     utilGeneral_BKLV_SAVE,        CLI_COM_NORMAL},
    {eCLI_MAIN_BKLV,    "LOAD",     WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,                     utilGeneral_BKLV_LOAD,        CLI_COM_NORMAL},

    {eCLI_MAIN_BKLV,    "DBGG",     READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,                     utilGeneral_BKLV_DBGG,        CLI_COM_NORMAL},
    {eCLI_MAIN_BKLV,    "TEST",     WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,                     utilGeneral_BKLV_TEST,        CLI_COM_NORMAL},

    {eCLI_MAIN_BKLV,    "OSDM",     WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,                     utilGeneral_BKLV_OSDM,        CLI_COM_NORMAL},

    //LMRT============================================================================================================//LMRT		//G100_Coda_0005
    {eCLI_MAIN_LMRT,    NULL,       READ_COMMAND,     DECIAML_2,          edcLensMemoryReset,           NULL,        CLI_COM_WRITE_ONLY},
    {eCLI_MAIN_LMRT,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcLensMemoryReset,           NULL,        CLI_COM_WRITE_ONLY},

    //LERT============================================================================================================//LERT
    {eCLI_MAIN_LERT,    NULL,       READ_COMMAND,     DECIAML_2,          edcLENS_RESET,           NULL,        CLI_COM_WRITE_ONLY},
    {eCLI_MAIN_LERT,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcLENS_RESET,           NULL,        CLI_COM_WRITE_ONLY},

    //BKSW============================================================================================================//BKSW
    {eCLI_MAIN_BKSW,    NULL,       READ_COMMAND,     DECIAML_2,          edcBACKUPINPUT_AUTOSWITCH,           NULL,        CLI_COM_NORMAL},
    {eCLI_MAIN_BKSW,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcBACKUPINPUT_AUTOSWITCH,           NULL,        CLI_COM_NORMAL},

    //BKCS============================================================================================================//BKCS
    {eCLI_MAIN_BKCS,    NULL,       READ_COMMAND,     DECIAML_2,          edcBACKUPINPUT_CURRENT_SOURCE,           NULL,        CLI_COM_READ_ONLY}, //G100_Steven_0060 fix mantis #15464
    {eCLI_MAIN_BKCS,    NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,           				   NULL,        CLI_COM_READ_ONLY},

    //BKFI============================================================================================================//BKFI
    {eCLI_MAIN_BKFI,    NULL,       READ_COMMAND,     DECIAML_2,          edcBACKUPINPUT_PRIMARY_INPUT,           NULL,        CLI_COM_NORMAL},
    {eCLI_MAIN_BKFI,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcBACKUPINPUT_PRIMARY_INPUT,           NULL,        CLI_COM_NORMAL},

    //BKFR============================================================================================================//BKFR
    {eCLI_MAIN_BKFR,    NULL,       READ_COMMAND,     DECIAML_2,          edcBACKUPINPUT_PRIMARY_RESOLUTION,           NULL,        CLI_COM_READ_ONLY},
    {eCLI_MAIN_BKFR,    NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,           NULL,        CLI_COM_READ_ONLY},

    //BKFH============================================================================================================//BKFH
    {eCLI_MAIN_BKFH,    NULL,       READ_COMMAND,     DECIAML_2,          edcBACKUPINPUT_PRIMARY_HORZRATE,           NULL,        CLI_COM_READ_ONLY},
    {eCLI_MAIN_BKFH,    NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,           NULL,        CLI_COM_READ_ONLY},

    //BKFC============================================================================================================//BKFC
    {eCLI_MAIN_BKFC,    NULL,       READ_COMMAND,     DECIAML_2,          edcBACKUPINPUT_PRIMARY_COLORSPACE,           NULL,        CLI_COM_READ_ONLY},
    {eCLI_MAIN_BKFC,    NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,           NULL,        CLI_COM_READ_ONLY},

    //BKFD===========================================================================================================//BKSI
    {eCLI_MAIN_BKFD,    NULL,       READ_COMMAND,     DECIAML_2,          edcFIRST_INPUT_HDR_INFO, NULL,        CLI_COM_READ_ONLY},  //A35G2_BRC_Simon_0004
    {eCLI_MAIN_BKFD,    NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,            NULL,        CLI_COM_READ_ONLY},

    //BKSI============================================================================================================//BKSI
    {eCLI_MAIN_BKSI,    NULL,       READ_COMMAND,     DECIAML_2,          edcBACKUPINPUT_SECONDARY_INPUT,           NULL,        CLI_COM_NORMAL},
    {eCLI_MAIN_BKSI,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcBACKUPINPUT_SECONDARY_INPUT,           NULL,        CLI_COM_NORMAL},

    //BKSR============================================================================================================//BKSR
    {eCLI_MAIN_BKSR,    NULL,       READ_COMMAND,     DECIAML_2,          edcBACKUPINPUT_SECONDARY_RESOLUTION,           NULL,        CLI_COM_READ_ONLY},
    {eCLI_MAIN_BKSR,    NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,           NULL,        CLI_COM_READ_ONLY},

    //BKSH============================================================================================================//BKSH
    {eCLI_MAIN_BKSH,    NULL,       READ_COMMAND,     DECIAML_2,          edcBACKUPINPUT_SECONDARY_HORZRATE,           NULL,        CLI_COM_READ_ONLY},
    {eCLI_MAIN_BKSH,    NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,           NULL,        CLI_COM_READ_ONLY},

    //BKSC============================================================================================================//BKSC
    {eCLI_MAIN_BKSC,    NULL,       READ_COMMAND,     DECIAML_2,          edcBACKUPINPUT_SECONDARY_COLORSPACE,           NULL,        CLI_COM_READ_ONLY},
    {eCLI_MAIN_BKSC,    NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,           NULL,        CLI_COM_READ_ONLY},

    //BKSD============================================================================================================//BKSC
    {eCLI_MAIN_BKSD,    NULL,       READ_COMMAND,     DECIAML_2,          edcSECOND_INPUT_HDR_INFO, NULL,        CLI_COM_READ_ONLY},      //A35G2_BRC_Simon_0004
    {eCLI_MAIN_BKSD,    NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,             NULL,        CLI_COM_READ_ONLY},

    //BKIS============================================================================================================//BKIS
    {eCLI_MAIN_BKIS,    NULL,       READ_COMMAND,     DECIAML_2,          edcBACKUPINPUT_STATUS,            NULL,        CLI_COM_READ_ONLY},
    {eCLI_MAIN_BKIS,    NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,                     NULL,        CLI_COM_READ_ONLY},

    //BKIC============================================================================================================//BKIS
    {eCLI_MAIN_BKIC,    NULL,       READ_COMMAND,     DECIAML_2,          edcBACKUP_INPUT_CHANGE,           NULL,        CLI_COM_READ_ONLY},      //A35G2_BRC_Simon_0004
    {eCLI_MAIN_BKIC,    NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,                     NULL,        CLI_COM_READ_ONLY},

    //HSWP============================================================================================================//HSWP
    //{eCLI_MAIN_HSWP,    NULL,       READ_COMMAND,     DECIAML_2,          edcHSG_AUTO_TEST_PATTERN_WHITE,           NULL,        CLI_COM_NORMAL}, //G100_Larry_0030
    //{eCLI_MAIN_HSWP,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcHSG_AUTO_TEST_PATTERN_WHITE,           NULL,        CLI_COM_NORMAL}, //G100_Larry_0030

    //DCRT============================================================================================================//DCRT
    {eCLI_MAIN_DCRT,    NULL,       READ_COMMAND,     DECIAML_2,          edcDYNAMIC_CONTRAST_RESET,           NULL,        CLI_COM_WRITE_ONLY},
    {eCLI_MAIN_DCRT,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcDYNAMIC_CONTRAST_RESET,           NULL,        CLI_COM_WRITE_ONLY},

    //TDRT============================================================================================================//TDRT
    {eCLI_MAIN_TDRT,    NULL,       READ_COMMAND,     DECIAML_2,          edc3D_Reset,           NULL,        CLI_COM_WRITE_ONLY},
    {eCLI_MAIN_TDRT,    NULL,       WRITE_COMMAND,    DECIAML_2,          edc3D_Reset,           NULL,        CLI_COM_WRITE_ONLY | CLI_COM_CHECK_BUSY},

    //NFRS============================================================================================================//NFRS
    //{eCLI_MAIN_NFRS,    NULL,       READ_COMMAND,     DECIAML_2,          edcNETWORK_FACTORY_RESET,      NULL,        CLI_COM_WRITE_ONLY}, //G100_Coda_0006 //A35G2_BRC_Casper_0018
    {eCLI_MAIN_NFRS,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcNETWORK_FACTORY_RESET,      NULL,        CLI_COM_WRITE_ONLY}, //G100_Coda_0006
    //{eCLI_MAIN_NFRS,    NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,      utilGeneral_NetWorkRst,        CLI_COM_NORMAL|CLI_COM_NO_REPLY}, //G100_Steven_0072

    //BALP============================================================================================================//BALP
    {eCLI_MAIN_BALP,    NULL,       READ_COMMAND,     DECIAML_2,          edcPWRKEY_BACKLIGHT,           NULL,        CLI_COM_NORMAL},
    {eCLI_MAIN_BALP,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcPWRKEY_BACKLIGHT,           NULL,        CLI_COM_NORMAL},

    //---//---G100_Coda_0007
    //PHAS============================================================================================================//PHAS
    {eCLI_MAIN_PHAS,    NULL,       READ_COMMAND,     DECIAML_2,          edcPIXEL_PHASE,           NULL,        CLI_COM_NORMAL},
    {eCLI_MAIN_PHAS,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcPIXEL_PHASE,           NULL,        CLI_COM_NORMAL},

    //HPOS============================================================================================================//HPOS
    {eCLI_MAIN_HPOS,    NULL,       READ_COMMAND,     DECIAML_2,          edcHORZ_POSITION,           NULL,        CLI_COM_NORMAL},
    {eCLI_MAIN_HPOS,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcHORZ_POSITION,           NULL,        CLI_COM_NORMAL},

    //VPOS============================================================================================================//VPOS
    {eCLI_MAIN_VPOS,    NULL,       READ_COMMAND,     DECIAML_2,          edcVERT_POSITION,           NULL,        CLI_COM_NORMAL},
    {eCLI_MAIN_VPOS,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcVERT_POSITION,           NULL,        CLI_COM_NORMAL},

    //DVRS============================================================================================================//DVRS
    {eCLI_MAIN_DVRS,    NULL,       READ_COMMAND,     DECIAML_2,          edcVGA_RESOLUTION,          NULL,        CLI_COM_NORMAL}, //G100_Simon_0038
    {eCLI_MAIN_DVRS,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcVGA_RESOLUTION,          NULL,        CLI_COM_NORMAL},

    //DHOP============================================================================================================//DHOP
    {eCLI_MAIN_DHOP,    NULL,       READ_COMMAND,     DECIAML_2,          edcHDMI_OUT,           NULL,        CLI_COM_NORMAL},
    {eCLI_MAIN_DHOP,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcHDMI_OUT,           NULL,        CLI_COM_NORMAL},

    //DHEA============================================================================================================//DHEA
    {eCLI_MAIN_DHEA,    NULL,       READ_COMMAND,     DECIAML_2,          edcHDMI_EDID_1,           NULL,        CLI_COM_NORMAL},
    {eCLI_MAIN_DHEA,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcHDMI_EDID_1,           NULL,        CLI_COM_NORMAL},

    //DHEB============================================================================================================//DHEB
    {eCLI_MAIN_DHEB,    NULL,       READ_COMMAND,     DECIAML_2,          edcHDMI_EDID_2,           NULL,        CLI_COM_NORMAL},
    {eCLI_MAIN_DHEB,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcHDMI_EDID_2,           NULL,        CLI_COM_NORMAL},

    //DHBT============================================================================================================//DHBT
    //{eCLI_MAIN_DHBT,    NULL,       READ_COMMAND,     DECIAML_2,          edcHDBASET_EDID,           NULL,        CLI_COM_NORMAL},
    //{eCLI_MAIN_DHBT,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcHDBASET_EDID,           NULL,        CLI_COM_NORMAL},

    //ASRS============================================================================================================//ASRS
    {eCLI_MAIN_ASRS,    NULL,       READ_COMMAND,     DECIAML_2,          edcAUTO_SOURCE_RESYNC,           NULL,        CLI_COM_NORMAL},
    {eCLI_MAIN_ASRS,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcAUTO_SOURCE_RESYNC,           NULL,        CLI_COM_NORMAL},

    //HDRG============================================================================================================//HDRG
    {eCLI_MAIN_HDRG,    NULL,       READ_COMMAND,     DECIAML_2,          edcHDR_AUTOENABLE,           NULL,        CLI_COM_NORMAL},
    {eCLI_MAIN_HDRG,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcHDR_AUTOENABLE,           NULL,        CLI_COM_NORMAL},

    //HDRP============================================================================================================//HDRP
    {eCLI_MAIN_HDRP,    NULL,       READ_COMMAND,     DECIAML_2,          edcHDR_LEVEL,           NULL,        CLI_COM_NORMAL},
    {eCLI_MAIN_HDRP,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcHDR_LEVEL,           NULL,        CLI_COM_NORMAL},

    //LLTC============================================================================================================//LLTC
    {eCLI_MAIN_LLTC,    NULL,       READ_COMMAND,     DECIAML_2,          edcLOW_LATENCY_MODE,           NULL,        CLI_COM_NORMAL},
    {eCLI_MAIN_LLTC,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcLOW_LATENCY_MODE,           NULL,        CLI_COM_NORMAL},

    //MNTP============================================================================================================//MNTP
    {eCLI_MAIN_MNTP,    NULL,       READ_COMMAND,     DECIAML_2,          edcMENU_TRANSPARENCY,           NULL,        CLI_COM_NORMAL},
    {eCLI_MAIN_MNTP,    NULL,       WRITE_COMMAND,    DECIAML_2,          edcMENU_TRANSPARENCY,           NULL,        CLI_COM_NORMAL},

	//WWCG============================================================================================================//WWCG
    {eCLI_MAIN_WWCG,     NULL,       READ_COMMAND,     DECIAML_2,          edcADV_WARP_CONTROL,         NULL,           CLI_COM_NORMAL},		//G100_Doulas_0027
    {eCLI_MAIN_WWCG,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcADV_WARP_CONTROL,         NULL,           CLI_COM_NORMAL},

	//WGPG============================================================================================================//WGPG
    {eCLI_MAIN_WGPG,     NULL,       READ_COMMAND,     DECIAML_2,          edcADV_WARP_GRID_POINTS,    	NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_WGPG,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcADV_WARP_GRID_POINTS,     NULL,           CLI_COM_NORMAL},

	//WWIG============================================================================================================//WWIG
    {eCLI_MAIN_WWIG,     NULL,       READ_COMMAND,     DECIAML_2,          edcADV_WARP_INNER,           NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_WWIG,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcADV_WARP_INNER,           NULL,           CLI_COM_NORMAL},

	//WWSG============================================================================================================//WWSG
    {eCLI_MAIN_WWSG,     NULL,       READ_COMMAND,     DECIAML_2,          edcADV_WARP_SHARPNESS,       NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_WWSG,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcADV_WARP_SHARPNESS,       NULL,           CLI_COM_NORMAL},

	//WGCG============================================================================================================//WGCG
    {eCLI_MAIN_WGCG,     NULL,       READ_COMMAND,     DECIAML_2,          edcADV_WARP_GRID_COLOR,      NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_WGCG,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcADV_WARP_GRID_COLOR,      NULL,           CLI_COM_NORMAL},

	//WBCG============================================================================================================//WBCG
    {eCLI_MAIN_WBCG,     NULL,       READ_COMMAND,     DECIAML_2,          edcADV_WARP_GRID_BACKGROUND, NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_WBCG,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcADV_WARP_GRID_BACKGROUND, NULL,           CLI_COM_NORMAL},

	//EBLG+BLAJ1============================================================================================================//EBLG+BLAJ1
	{eCLI_MAIN_EBL,     "BLAJ",     WRITE_COMMAND,    DECIAML_2,    	   edcADV_BLEND_WIDTH,   		NULL,         	CLI_COM_NORMAL},
    {eCLI_MAIN_EBL,     "BLAJ",     READ_COMMAND,     DECIAML_2,    	   edcADV_BLEND_WIDTH,   		NULL,         	CLI_COM_NORMAL},

	//EBLG+OLGN============================================================================================================//EBLG+OLGN
    {eCLI_MAIN_EBL,     "OLGN",     WRITE_COMMAND,    DECIAML_2,    edcADV_BLEND_OVERLAP_GRID_NUMBER,   NULL,         	CLI_COM_NORMAL},
    {eCLI_MAIN_EBL,     "OLGN",     READ_COMMAND,     DECIAML_2,    edcADV_BLEND_OVERLAP_GRID_NUMBER,   NULL,         	CLI_COM_NORMAL},

	//EBLG+BLAL============================================================================================================//EBLG+BLAL
    //{eCLI_MAIN_EBL,     "BLAL",     WRITE_COMMAND,    DECIAML_2,    		edcADV_BLEND_LEFT,   		NULL,         	CLI_COM_NORMAL},
    {eCLI_MAIN_EBL,     "BLAL",     READ_COMMAND,     DECIAML_2,    		edcADV_BLEND_LEFT,   		NULL,         	CLI_COM_NORMAL},

	//EBLG+BLAR============================================================================================================//EBLG+BLAR
    //{eCLI_MAIN_EBL,     "BLAR",     WRITE_COMMAND,    DECIAML_2,    		edcADV_BLEND_RIGHT,   		NULL,         	CLI_COM_NORMAL},
    {eCLI_MAIN_EBL,     "BLAR",     READ_COMMAND,     DECIAML_2,    		edcADV_BLEND_RIGHT,   		NULL,         	CLI_COM_NORMAL},

	//EBLG+BLAT============================================================================================================//EBLG+BLAT
    //{eCLI_MAIN_EBL,     "BLAT",     WRITE_COMMAND,    DECIAML_2,    		edcADV_BLEND_TOP,   		NULL,         	CLI_COM_NORMAL},
    {eCLI_MAIN_EBL,     "BLAT",     READ_COMMAND,     DECIAML_2,    		edcADV_BLEND_TOP,   		NULL,         	CLI_COM_NORMAL},

	//EBLG+BLAB============================================================================================================//EBLG+BLAB
    //{eCLI_MAIN_EBL,     "BLAB",     WRITE_COMMAND,    DECIAML_2,    		edcADV_BLEND_BOTTOM,   		NULL,         	CLI_COM_NORMAL},
    {eCLI_MAIN_EBL,     "BLAB",     READ_COMMAND,     DECIAML_2,    		edcADV_BLEND_BOTTOM,   		NULL,         	CLI_COM_NORMAL},

    //EBLG+BLRS============================================================================================================//EBLG+BLRS
    {eCLI_MAIN_EBL,     "BLRS",     READ_COMMAND,     DECIAML_2,            DATA_CODE_NA,           utilGeneral_BLRS,           CLI_COM_NORMAL},
    {eCLI_MAIN_EBL,     "BLRS",     WRITE_COMMAND,    DECIAML_2,            DATA_CODE_NA,           utilGeneral_BLRS,           CLI_COM_NORMAL},

    //EBLG+OLTP============================================================================================================//EBLG+OLTP
    {eCLI_MAIN_EBL,     "OLTP",     READ_COMMAND,     DECIAML_2,            DATA_CODE_NA,           utilGeneral_OLTP,           CLI_COM_NORMAL},
    {eCLI_MAIN_EBL,     "OLTP",     WRITE_COMMAND,    DECIAML_2,            DATA_CODE_NA,           utilGeneral_OLTP,           CLI_COM_NORMAL},

    //EBLG+OLPU============================================================================================================//EBLG+OLPU
    {eCLI_MAIN_EBL,     "OLPU",     READ_COMMAND,     DECIAML_2,            DATA_CODE_NA,           utilGeneral_OLPU,           CLI_COM_NORMAL},
    {eCLI_MAIN_EBL,     "OLPU",     WRITE_COMMAND,    DECIAML_2,            DATA_CODE_NA,           utilGeneral_OLPU,           CLI_COM_NORMAL},

    //EBLG+OLPD============================================================================================================//EBLG+OLPD
    {eCLI_MAIN_EBL,     "OLPD",     READ_COMMAND,     DECIAML_2,            DATA_CODE_NA,           utilGeneral_OLPD,           CLI_COM_NORMAL},
    {eCLI_MAIN_EBL,     "OLPD",     WRITE_COMMAND,    DECIAML_2,            DATA_CODE_NA,           utilGeneral_OLPD,           CLI_COM_NORMAL},

    //EBLG+OLPL============================================================================================================//EBLG+OLPL
    {eCLI_MAIN_EBL,     "OLPL",     READ_COMMAND,     DECIAML_2,            DATA_CODE_NA,           utilGeneral_OLPL,           CLI_COM_NORMAL},
    {eCLI_MAIN_EBL,     "OLPL",     WRITE_COMMAND,    DECIAML_2,            DATA_CODE_NA,           utilGeneral_OLPL,           CLI_COM_NORMAL},

    //EBLG+OLPR============================================================================================================//EBLG+OLPR
    {eCLI_MAIN_EBL,     "OLPR",     READ_COMMAND,     DECIAML_2,            DATA_CODE_NA,           utilGeneral_OLPR,           CLI_COM_NORMAL},
    {eCLI_MAIN_EBL,     "OLPR",     WRITE_COMMAND,    DECIAML_2,            DATA_CODE_NA,           utilGeneral_OLPR,           CLI_COM_NORMAL},

    //EBLG+OSPU============================================================================================================//EBLG+OSPU
    {eCLI_MAIN_EBL,     "OSPU",     READ_COMMAND,     DECIAML_2,            DATA_CODE_NA,           utilGeneral_OSPU,           CLI_COM_NORMAL},
    {eCLI_MAIN_EBL,     "OSPU",     WRITE_COMMAND,    DECIAML_2,            DATA_CODE_NA,           utilGeneral_OSPU,           CLI_COM_NORMAL},

    //EBLG+OSPD============================================================================================================//EBLG+OSPD
    {eCLI_MAIN_EBL,     "OSPD",     READ_COMMAND,     DECIAML_2,            DATA_CODE_NA,           utilGeneral_OSPD,           CLI_COM_NORMAL},
    {eCLI_MAIN_EBL,     "OSPD",     WRITE_COMMAND,    DECIAML_2,            DATA_CODE_NA,           utilGeneral_OSPD,           CLI_COM_NORMAL},

    //EBLG+OSPL============================================================================================================//EBLG+OSPL
    {eCLI_MAIN_EBL,     "OSPL",     READ_COMMAND,     DECIAML_2,            DATA_CODE_NA,           utilGeneral_OSPL,           CLI_COM_NORMAL},
    {eCLI_MAIN_EBL,     "OSPL",     WRITE_COMMAND,    DECIAML_2,            DATA_CODE_NA,           utilGeneral_OSPL,           CLI_COM_NORMAL},

    //EBLG+OSPR============================================================================================================//EBLG+OSPR
    {eCLI_MAIN_EBL,     "OSPR",     READ_COMMAND,     DECIAML_2,            DATA_CODE_NA,           utilGeneral_OSPR,           CLI_COM_NORMAL},
    {eCLI_MAIN_EBL,     "OSPR",     WRITE_COMMAND,    DECIAML_2,            DATA_CODE_NA,           utilGeneral_OSPR,           CLI_COM_NORMAL},

    //EBLG+REST============================================================================================================//EBLG+REST
    {eCLI_MAIN_EBL,     "REST",     WRITE_COMMAND,    DECIAML_2,    		DATA_CODE_NA,   		utilGeneral_REST,         	CLI_COM_NORMAL},

	//GMSG============================================================================================================//GMSG
    {eCLI_MAIN_GMSG,     NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,                utilGeneral_GMSG,  CLI_COM_NORMAL},
    {eCLI_MAIN_GMSG,     NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,                utilGeneral_GMSG,  CLI_COM_NORMAL},

	//GMAG============================================================================================================//GMAG
    {eCLI_MAIN_GMAG,     NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,                utilGeneral_GMAG,  CLI_COM_NORMAL},
    {eCLI_MAIN_GMAG,     NULL,       WRITE_COMMAND,    DECIAML_2,          DATA_CODE_NA,                utilGeneral_GMAG,  CLI_COM_NORMAL},

	//GMCG============================================================================================================//GMCG
    {eCLI_MAIN_GMCG,     NULL,       READ_COMMAND,     DECIAML_2,          edcWARP_MEMORY_CLEAR,       	NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_GMCG,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcWARP_MEMORY_CLEAR,      	NULL,           CLI_COM_NORMAL},

    //DTCM============================================================================================================//DTCM
    {eCLI_MAIN_DTCM,     NULL,       READ_COMMAND,     DECIAML_2,          edcDATE_MODE,    	NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_DTCM,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcDATE_MODE,     NULL,           CLI_COM_NORMAL},

    //DTSY============================================================================================================//DTSY
    {eCLI_MAIN_DTSY,     NULL,       READ_COMMAND,     DECIAML_2,          edcDATE_YEAR,    	NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_DTSY,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcDATE_YEAR,     NULL,           CLI_COM_NORMAL},

    //DTSM============================================================================================================//DTSM
    {eCLI_MAIN_DTSM,     NULL,       READ_COMMAND,     DECIAML_2,          edcDATE_MONTH,    	NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_DTSM,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcDATE_MONTH,     NULL,           CLI_COM_NORMAL},

    //DTSD============================================================================================================//DTSD
    {eCLI_MAIN_DTSD,     NULL,       READ_COMMAND,     DECIAML_2,          edcDATE_DAY,    	NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_DTSD,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcDATE_DAY,     NULL,           CLI_COM_NORMAL},

    //DTTH============================================================================================================//DTTH
    {eCLI_MAIN_DTTH,     NULL,       READ_COMMAND,     DECIAML_2,          edcDATE_HOUR,    	NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_DTTH,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcDATE_HOUR,     NULL,           CLI_COM_NORMAL},

    //DTTM============================================================================================================//DTTM
    {eCLI_MAIN_DTTM,     NULL,       READ_COMMAND,     DECIAML_2,          edcDATE_MINUTE,    	NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_DTTM,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcDATE_MINUTE,     NULL,           CLI_COM_NORMAL},

    //DTDS============================================================================================================//DTDS
    {eCLI_MAIN_DTDS,     NULL,       READ_COMMAND,     DECIAML_2,          edcDATE_DAYNIGHT_SAVING_TIME,    	NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_DTDS,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcDATE_DAYNIGHT_SAVING_TIME,     NULL,           CLI_COM_NORMAL},

    //DTNS============================================================================================================//DTNS
    {eCLI_MAIN_DTNS,     NULL,       READ_COMMAND,     DECIAML_2,          edcDATE_NTP_SERVER,    	NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_DTNS,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcDATE_NTP_SERVER,     NULL,           CLI_COM_NORMAL},

    //DTTZ============================================================================================================//DTTZ
    {eCLI_MAIN_DTTZ,     NULL,       READ_COMMAND,     DECIAML_2,          edcDATE_ZONE,    	NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_DTTZ,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcDATE_ZONE,     NULL,           CLI_COM_NORMAL},

    //DTUI============================================================================================================//DTUI
    {eCLI_MAIN_DTUI,     NULL,       READ_COMMAND,     DECIAML_2,          edcDATE_UPDATE_INTERVAL,    	NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_DTUI,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcDATE_UPDATE_INTERVAL,     NULL,           CLI_COM_NORMAL},

    //DTAP============================================================================================================//DTAP
    {eCLI_MAIN_DTAP,     NULL,       READ_COMMAND,     DECIAML_2,          edcDATE_APPLY,     NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_DTAP,     NULL,       WRITE_COMMAND,    DECIAML_2,          edcDATE_APPLY,     NULL,           CLI_COM_NORMAL},
//G100_Coda_0042
    //DTIF============================================================================================================//DTIF
    {eCLI_MAIN_DTIF,     NULL,      READ_COMMAND,     DECIAML_2,    		edcREAL_DATE_TIME,   		NULL,         	CLI_COM_READ_ONLY}, //G100_Coda_0047

    //SCME============================================================================================================//SCME
    {eCLI_MAIN_SCHE,     "MODE",       READ_COMMAND,     DECIAML_2,          edcSCHEDULE_MODE,    	NULL,           CLI_COM_NORMAL}, //G100_Coda_0049
    {eCLI_MAIN_SCHE,     "MODE",       WRITE_COMMAND,    DECIAML_2,          edcSCHEDULE_MODE,     NULL,           CLI_COM_NORMAL},

    //SCTD============================================================================================================//SCTD
    {eCLI_MAIN_SCHE,     "WDAY",     READ_COMMAND,     DECIAML_2,    		DATA_CODE_NA,   		utilGeneral_SCHE_ShowToday,      CLI_COM_NO_REPLY|CLI_COM_READ_ONLY}, //G100_Coda_0049


    //SCW1+MODE============================================================================================================//SCW1+MODE
    {eCLI_MAIN_SCW1,    "MODE",         READ_COMMAND,     DECIAML_2,      edcSCHEDULE_MONDAY_ENABLE,  NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_SCW1,    "MODE",         WRITE_COMMAND,    DECIAML_2,      edcSCHEDULE_MONDAY_ENABLE,  NULL,          CLI_COM_NORMAL},

    //SCW2+MODE============================================================================================================//SCW2+MODE
    {eCLI_MAIN_SCW2,    "MODE",         READ_COMMAND,     DECIAML_2,      edcSCHEDULE_TUESDAY_ENABLE,  NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_SCW2,    "MODE",         WRITE_COMMAND,    DECIAML_2,      edcSCHEDULE_TUESDAY_ENABLE,  NULL,          CLI_COM_NORMAL},

    //SCW3+MODE============================================================================================================//SCW3+MODE
    {eCLI_MAIN_SCW3,    "MODE",         READ_COMMAND,     DECIAML_2,      edcSCHEDULE_WEDNESDAY_ENABLE,  NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_SCW3,    "MODE",         WRITE_COMMAND,    DECIAML_2,      edcSCHEDULE_WEDNESDAY_ENABLE,  NULL,          CLI_COM_NORMAL},

    //SCW4+MODE============================================================================================================//SCW4+MODE
    {eCLI_MAIN_SCW4,    "MODE",         READ_COMMAND,     DECIAML_2,      edcSCHEDULE_THURSDAY_ENABLE,  NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_SCW4,    "MODE",         WRITE_COMMAND,    DECIAML_2,      edcSCHEDULE_THURSDAY_ENABLE,  NULL,          CLI_COM_NORMAL},

    //SCW5+MODE============================================================================================================//SCW5+MODE
    {eCLI_MAIN_SCW5,    "MODE",         READ_COMMAND,     DECIAML_2,      edcSCHEDULE_FRIDAY_ENABLE,  NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_SCW5,    "MODE",         WRITE_COMMAND,    DECIAML_2,      edcSCHEDULE_FRIDAY_ENABLE,  NULL,          CLI_COM_NORMAL},

    //SCW6+MODE============================================================================================================//SCW6+MODE
    {eCLI_MAIN_SCW6,    "MODE",         READ_COMMAND,     DECIAML_2,      edcSCHEDULE_SATURDAY_ENABLE,  NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_SCW6,    "MODE",         WRITE_COMMAND,    DECIAML_2,      edcSCHEDULE_SATURDAY_ENABLE,  NULL,          CLI_COM_NORMAL},

    //SCW0+MODE============================================================================================================//SCW0+MODE
    {eCLI_MAIN_SCW0,    "MODE",         READ_COMMAND,     DECIAML_2,      edcSCHEDULE_SUNDAY_ENABLE,  NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_SCW0,    "MODE",         WRITE_COMMAND,    DECIAML_2,      edcSCHEDULE_SUNDAY_ENABLE,  NULL,          CLI_COM_NORMAL},


    //SCW1+EVRT============================================================================================================//SCW1+EVRT
    {eCLI_MAIN_SCW1,    "EVRT",         READ_COMMAND,     DECIAML_2,      DATA_CODE_NA,  utilGeneral_SCW_EVRT,          CLI_COM_WRITE_ONLY},
    {eCLI_MAIN_SCW1,    "EVRT",         WRITE_COMMAND,    DECIAML_2,      DATA_CODE_NA,  utilGeneral_SCW_EVRT,          CLI_COM_NORMAL},

    //SCW2+EVRT============================================================================================================//SCW2+EVRT
    {eCLI_MAIN_SCW2,    "EVRT",         READ_COMMAND,     DECIAML_2,      DATA_CODE_NA,  utilGeneral_SCW_EVRT,          CLI_COM_WRITE_ONLY},
    {eCLI_MAIN_SCW2,    "EVRT",         WRITE_COMMAND,    DECIAML_2,      DATA_CODE_NA,  utilGeneral_SCW_EVRT,          CLI_COM_NORMAL},

    //SCW3+EVRT============================================================================================================//SCW3+EVRT
    {eCLI_MAIN_SCW3,    "EVRT",         READ_COMMAND,     DECIAML_2,      DATA_CODE_NA,  utilGeneral_SCW_EVRT,          CLI_COM_WRITE_ONLY},
    {eCLI_MAIN_SCW3,    "EVRT",         WRITE_COMMAND,    DECIAML_2,      DATA_CODE_NA,  utilGeneral_SCW_EVRT,          CLI_COM_NORMAL},

    //SCW4+EVRT============================================================================================================//SCW4+EVRT
    {eCLI_MAIN_SCW4,    "EVRT",         READ_COMMAND,     DECIAML_2,      DATA_CODE_NA,  utilGeneral_SCW_EVRT,          CLI_COM_WRITE_ONLY},
    {eCLI_MAIN_SCW4,    "EVRT",         WRITE_COMMAND,    DECIAML_2,      DATA_CODE_NA,  utilGeneral_SCW_EVRT,          CLI_COM_NORMAL},

    //SCW5+EVRT============================================================================================================//SCW5+EVRT
    {eCLI_MAIN_SCW5,    "EVRT",         READ_COMMAND,     DECIAML_2,      DATA_CODE_NA,  utilGeneral_SCW_EVRT,          CLI_COM_WRITE_ONLY},
    {eCLI_MAIN_SCW5,    "EVRT",         WRITE_COMMAND,    DECIAML_2,      DATA_CODE_NA,  utilGeneral_SCW_EVRT,          CLI_COM_NORMAL},

    //SCW6+EVRT============================================================================================================//SCW6+EVRT
    {eCLI_MAIN_SCW6,    "EVRT",         READ_COMMAND,     DECIAML_2,      DATA_CODE_NA,  utilGeneral_SCW_EVRT,          CLI_COM_WRITE_ONLY},
    {eCLI_MAIN_SCW6,    "EVRT",         WRITE_COMMAND,    DECIAML_2,      DATA_CODE_NA,  utilGeneral_SCW_EVRT,          CLI_COM_NORMAL},

    //SCW0+EVRT============================================================================================================//SCW0+EVRT
    {eCLI_MAIN_SCW0,    "EVRT",         READ_COMMAND,     DECIAML_2,      DATA_CODE_NA,  utilGeneral_SCW_EVRT,          CLI_COM_WRITE_ONLY},
    {eCLI_MAIN_SCW0,    "EVRT",         WRITE_COMMAND,    DECIAML_2,      DATA_CODE_NA,  utilGeneral_SCW_EVRT,          CLI_COM_NORMAL},


    //SCW1+REST============================================================================================================//SCW1+REST
    {eCLI_MAIN_SCW1,    "REST",         READ_COMMAND,     DECIAML_2,      DATA_CODE_NA,  utilGeneral_SCW_REST,          CLI_COM_WRITE_ONLY},
    {eCLI_MAIN_SCW1,    "REST",         WRITE_COMMAND,    DECIAML_2,      DATA_CODE_NA,  utilGeneral_SCW_REST,          CLI_COM_NORMAL},

    //SCW2+REST============================================================================================================//SCW2+REST
    {eCLI_MAIN_SCW2,    "REST",         READ_COMMAND,     DECIAML_2,      DATA_CODE_NA,  utilGeneral_SCW_REST,          CLI_COM_WRITE_ONLY},
    {eCLI_MAIN_SCW2,    "REST",         WRITE_COMMAND,    DECIAML_2,      DATA_CODE_NA,  utilGeneral_SCW_REST,          CLI_COM_NORMAL},

    //SCW3+REST============================================================================================================//SCW3+REST
    {eCLI_MAIN_SCW3,    "REST",         READ_COMMAND,     DECIAML_2,      DATA_CODE_NA,  utilGeneral_SCW_REST,          CLI_COM_WRITE_ONLY},
    {eCLI_MAIN_SCW3,    "REST",         WRITE_COMMAND,    DECIAML_2,      DATA_CODE_NA,  utilGeneral_SCW_REST,          CLI_COM_NORMAL},

    //SCW4+REST============================================================================================================//SCW4+REST
    {eCLI_MAIN_SCW4,    "REST",         READ_COMMAND,     DECIAML_2,      DATA_CODE_NA,  utilGeneral_SCW_REST,          CLI_COM_WRITE_ONLY},
    {eCLI_MAIN_SCW4,    "REST",         WRITE_COMMAND,    DECIAML_2,      DATA_CODE_NA,  utilGeneral_SCW_REST,          CLI_COM_NORMAL},

    //SCW5+REST============================================================================================================//SCW5+REST
    {eCLI_MAIN_SCW5,    "REST",         READ_COMMAND,     DECIAML_2,      DATA_CODE_NA,  utilGeneral_SCW_REST,          CLI_COM_WRITE_ONLY},
    {eCLI_MAIN_SCW5,    "REST",         WRITE_COMMAND,    DECIAML_2,      DATA_CODE_NA,  utilGeneral_SCW_REST,          CLI_COM_NORMAL},

    //SCW6+REST============================================================================================================//SCW6+REST
    {eCLI_MAIN_SCW6,    "REST",         READ_COMMAND,     DECIAML_2,      DATA_CODE_NA,  utilGeneral_SCW_REST,          CLI_COM_WRITE_ONLY},
    {eCLI_MAIN_SCW6,    "REST",         WRITE_COMMAND,    DECIAML_2,      DATA_CODE_NA,  utilGeneral_SCW_REST,          CLI_COM_NORMAL},

    //SCW0+REST============================================================================================================//SCW0+REST
    {eCLI_MAIN_SCW0,    "REST",         READ_COMMAND,     DECIAML_2,      DATA_CODE_NA,  utilGeneral_SCW_REST,          CLI_COM_WRITE_ONLY},
    {eCLI_MAIN_SCW0,    "REST",         WRITE_COMMAND,    DECIAML_2,      DATA_CODE_NA,  utilGeneral_SCW_REST,          CLI_COM_NORMAL},


    //SCW1+CPWD============================================================================================================//SCW1+CPWD
    {eCLI_MAIN_SCW1,    "CPWD",         READ_COMMAND,     DECIAML_2,      DATA_CODE_NA,  utilGeneral_SCW_CPWD,          CLI_COM_NORMAL},
    {eCLI_MAIN_SCW1,    "CPWD",         WRITE_COMMAND,    DECIAML_2,      DATA_CODE_NA,  utilGeneral_SCW_CPWD,          CLI_COM_NORMAL},

    //SCW2+CPWD============================================================================================================//SCW2+CPWD
    {eCLI_MAIN_SCW2,    "CPWD",         READ_COMMAND,     DECIAML_2,      DATA_CODE_NA,  utilGeneral_SCW_CPWD,          CLI_COM_NORMAL},
    {eCLI_MAIN_SCW2,    "CPWD",         WRITE_COMMAND,    DECIAML_2,      DATA_CODE_NA,  utilGeneral_SCW_CPWD,          CLI_COM_NORMAL},

    //SCW3+REST============================================================================================================//SCW3+CPWD
    {eCLI_MAIN_SCW3,    "CPWD",         READ_COMMAND,     DECIAML_2,      DATA_CODE_NA,  utilGeneral_SCW_CPWD,          CLI_COM_NORMAL},
    {eCLI_MAIN_SCW3,    "CPWD",         WRITE_COMMAND,    DECIAML_2,      DATA_CODE_NA,  utilGeneral_SCW_CPWD,          CLI_COM_NORMAL},

    //SCW4+REST============================================================================================================//SCW4+CPWD
    {eCLI_MAIN_SCW4,    "CPWD",         READ_COMMAND,     DECIAML_2,      DATA_CODE_NA,  utilGeneral_SCW_CPWD,          CLI_COM_NORMAL},
    {eCLI_MAIN_SCW4,    "CPWD",         WRITE_COMMAND,    DECIAML_2,      DATA_CODE_NA,  utilGeneral_SCW_CPWD,          CLI_COM_NORMAL},

    //SCW5+REST============================================================================================================//SCW5+CPWD
    {eCLI_MAIN_SCW5,    "CPWD",         READ_COMMAND,     DECIAML_2,      DATA_CODE_NA,  utilGeneral_SCW_CPWD,          CLI_COM_NORMAL},
    {eCLI_MAIN_SCW5,    "CPWD",         WRITE_COMMAND,    DECIAML_2,      DATA_CODE_NA,  utilGeneral_SCW_CPWD,          CLI_COM_NORMAL},

    //SCW6+REST============================================================================================================//SCW6+CPWD
    {eCLI_MAIN_SCW6,    "CPWD",         READ_COMMAND,     DECIAML_2,      DATA_CODE_NA,  utilGeneral_SCW_CPWD,          CLI_COM_NORMAL},
    {eCLI_MAIN_SCW6,    "CPWD",         WRITE_COMMAND,    DECIAML_2,      DATA_CODE_NA,  utilGeneral_SCW_CPWD,          CLI_COM_NORMAL},

    //SCW0+REST============================================================================================================//SCW0+CPWD
    {eCLI_MAIN_SCW0,    "CPWD",         READ_COMMAND,     DECIAML_2,      DATA_CODE_NA,  utilGeneral_SCW_CPWD,          CLI_COM_NORMAL},
    {eCLI_MAIN_SCW0,    "CPWD",         WRITE_COMMAND,    DECIAML_2,      DATA_CODE_NA,  utilGeneral_SCW_CPWD,          CLI_COM_NORMAL},

//G100_Coda_0047
    //SCW1+EVWR============================================================================================================//SCW1+EVWR
    {eCLI_MAIN_SCW1,    "EVRD",         READ_COMMAND,     DECIAML_2,      DATA_CODE_NA,  utilGeneral_SCW_EVRD,          CLI_COM_NO_REPLY|CLI_COM_READ_ONLY},
    {eCLI_MAIN_SCW1,    "EVWR",         WRITE_COMMAND,    DECIAML_2,      DATA_CODE_NA,  utilGeneral_SCW_EVWR,          CLI_COM_WRITE_ONLY},

    //SCW2+EVWR============================================================================================================//SCW2+EVWR
    {eCLI_MAIN_SCW2,    "EVRD",         READ_COMMAND,     DECIAML_2,      DATA_CODE_NA,  utilGeneral_SCW_EVRD,          CLI_COM_NO_REPLY|CLI_COM_READ_ONLY},
    {eCLI_MAIN_SCW2,    "EVWR",         WRITE_COMMAND,    DECIAML_2,      DATA_CODE_NA,  utilGeneral_SCW_EVWR,          CLI_COM_WRITE_ONLY},

    //SCW3+EVWR============================================================================================================//SCW3+EVWR
    {eCLI_MAIN_SCW3,    "EVRD",         READ_COMMAND,     DECIAML_2,      DATA_CODE_NA,  utilGeneral_SCW_EVRD,          CLI_COM_NO_REPLY|CLI_COM_READ_ONLY},
    {eCLI_MAIN_SCW3,    "EVWR",         WRITE_COMMAND,    DECIAML_2,      DATA_CODE_NA,  utilGeneral_SCW_EVWR,          CLI_COM_WRITE_ONLY},

    //SCW4+EVWR============================================================================================================//SCW4+EVWR
    {eCLI_MAIN_SCW4,    "EVRD",         READ_COMMAND,     DECIAML_2,      DATA_CODE_NA,  utilGeneral_SCW_EVRD,          CLI_COM_NO_REPLY|CLI_COM_READ_ONLY},
    {eCLI_MAIN_SCW4,    "EVWR",         WRITE_COMMAND,    DECIAML_2,      DATA_CODE_NA,  utilGeneral_SCW_EVWR,          CLI_COM_WRITE_ONLY},

    //SCW5+EVWR============================================================================================================//SCW5+EVWR
    {eCLI_MAIN_SCW5,    "EVRD",         READ_COMMAND,     DECIAML_2,      DATA_CODE_NA,  utilGeneral_SCW_EVRD,          CLI_COM_NO_REPLY|CLI_COM_READ_ONLY},
    {eCLI_MAIN_SCW5,    "EVWR",         WRITE_COMMAND,    DECIAML_2,      DATA_CODE_NA,  utilGeneral_SCW_EVWR,          CLI_COM_WRITE_ONLY},

    //SCW6+EVWR============================================================================================================//SCW6+EVWR
    {eCLI_MAIN_SCW6,    "EVRD",         READ_COMMAND,     DECIAML_2,      DATA_CODE_NA,  utilGeneral_SCW_EVRD,          CLI_COM_NO_REPLY|CLI_COM_READ_ONLY},
    {eCLI_MAIN_SCW6,    "EVWR",         WRITE_COMMAND,    DECIAML_2,      DATA_CODE_NA,  utilGeneral_SCW_EVWR,          CLI_COM_WRITE_ONLY},

    //SCW0+EVWR============================================================================================================//SCW0+EVWR
    {eCLI_MAIN_SCW0,    "EVRD",         READ_COMMAND,     DECIAML_2,      DATA_CODE_NA,  utilGeneral_SCW_EVRD,          CLI_COM_NO_REPLY|CLI_COM_READ_ONLY},
    {eCLI_MAIN_SCW0,    "EVWR",         WRITE_COMMAND,    DECIAML_2,      DATA_CODE_NA,  utilGeneral_SCW_EVWR,          CLI_COM_WRITE_ONLY},


    //SCHE+REST============================================================================================================//SCHE+REST
	{eCLI_MAIN_SCHE, 	"REST", 	    READ_COMMAND,	  DECIAML_2,	  edcSCHEDULE_EVENT_RESET_ALL,	NULL,		   CLI_COM_WRITE_ONLY},
	{eCLI_MAIN_SCHE, 	"REST", 	    WRITE_COMMAND,	  DECIAML_2,	  edcSCHEDULE_EVENT_RESET_ALL,	NULL,		   CLI_COM_WRITE_ONLY},

    //ATFC============================================================================================================//ATFC	//G100_Clare_0028
    {eCLI_MAIN_ATFC,     NULL,          READ_COMMAND,     DECIAML_2,      edcAUTO_FOCUS,                NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_ATFC,     NULL,          WRITE_COMMAND,    DECIAML_2,      edcAUTO_FOCUS,                NULL,           CLI_COM_NORMAL},

    //ATWC============================================================================================================//ATWC	//G100_Clare_0028
    {eCLI_MAIN_ATWC,     NULL,          READ_COMMAND,     DECIAML_2,      edcAUTO_COLOR,                NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_ATWC,     NULL,          WRITE_COMMAND,    DECIAML_2,      edcAUTO_COLOR,                NULL,           CLI_COM_NORMAL},

    //SHSP============================================================================================================//SHSP	//G100_Coda
    {eCLI_MAIN_SHSP,     NULL,          READ_COMMAND,     DECIAML_2,      edcSTARTUP_SHUTTER,                NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_SHSP,     NULL,          WRITE_COMMAND,    DECIAML_2,      edcSTARTUP_SHUTTER,                NULL,           CLI_COM_NORMAL}, //G100_Coda_0054

    //LOGC============================================================================================================//LOGC    //G100_Owen_0049
    {eCLI_MAIN_LOGC,     NULL,          READ_COMMAND,     DECIAML_2,      edcLOGO_CHANGE,                NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_LOGC,     NULL,          WRITE_COMMAND,    DECIAML_2,      edcLOGO_CHANGE,                NULL,           CLI_COM_NORMAL},

    //LOGS============================================================================================================//LOGS    //G100_Owen_0049
    {eCLI_MAIN_LOGS,     NULL,          WRITE_COMMAND,    DECIAML_2,      edcLOGO_SAVE,                  NULL,           CLI_COM_WRITE_ONLY},

    //LOGD============================================================================================================//LOGD    //G100_Owen_0049
    {eCLI_MAIN_LOGD,     NULL,          WRITE_COMMAND,    DECIAML_2,      edcLOGO_DELETE,                NULL,           CLI_COM_WRITE_ONLY},

#if defined (CUSTOM_OPTOMA) || defined(CUSTOM_BARCO) //A35G2_BRC_Casper_0039 //A35G2_BRC_Casper_0152
    //CAST============================================================================================================//CAST    //G100_Clare_0036
	{eCLI_MAIN_CAST, 	NULL,		READ_COMMAND,	  DECIAML_2,		  edcCAMERA_MODULE_STATUS, 	        NULL,		CLI_COM_READ_ONLY },
    //AFST============================================================================================================//AFST    //G100_Clare_0036
	{eCLI_MAIN_AFST, 	NULL,		READ_COMMAND,	  DECIAML_2,		  edcAUTO_FOCUS_STATUS, 			NULL,		CLI_COM_READ_ONLY },
    //WCST============================================================================================================//WCST    //G100_Clare_0036
	{eCLI_MAIN_WCST, 	NULL,		READ_COMMAND,	  DECIAML_2,		  edcAUTO_WALL_COLOR_STATUS, 		NULL,		CLI_COM_READ_ONLY },
#endif

    //FWVR============================================================================================================//FWVR    //G100_Clare_0039
    {eCLI_MAIN_FWVR,    NULL,    	READ_COMMAND,     DECIAML_2,          edcRELEASE_VERSION,         	    NULL,          CLI_COM_READ_ONLY},
    //CAVR============================================================================================================//CAVR	//G100_Clare_0039
    //{eCLI_MAIN_CAVR,    NULL,    	READ_COMMAND,     DECIAML_2,          edcCAMERA_FW_VERSION,         	NULL,          CLI_COM_READ_ONLY},

	//UDSA============================================================================================================//UDSA    //G100_Doulas_0062
    {eCLI_MAIN_UDSA,     NULL,          READ_COMMAND,     DECIAML_2,      edcBACKUP_RESTORE_SAVE,           NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_UDSA,     NULL,          WRITE_COMMAND,    DECIAML_2,      edcBACKUP_RESTORE_SAVE,           NULL,           CLI_COM_NORMAL | CLI_COM_CHECK_BUSY},

	//UDLD============================================================================================================//UDLD    //G100_Doulas_0062
    {eCLI_MAIN_UDLD,     NULL,          READ_COMMAND,     DECIAML_2,      DATA_CODE_NA,         utilGeneral_UDLD,           CLI_COM_NORMAL},  //A35G2_BRC_Casper_0057
    {eCLI_MAIN_UDLD,     NULL,          WRITE_COMMAND,    DECIAML_2,      DATA_CODE_NA,         utilGeneral_UDLD,           CLI_COM_NORMAL | CLI_COM_CHECK_BUSY},  //A35G2_BRC_Casper_0057

	//WCOL============================================================================================================//WCOL    //G100_Doulas_0067
    {eCLI_MAIN_WCOL,     NULL,          READ_COMMAND,     DECIAML_2,      DATA_CODE_NA,         utilGeneral_WCOL,           CLI_COM_NORMAL},
    {eCLI_MAIN_WCOL,     NULL,          WRITE_COMMAND,    DECIAML_2,      DATA_CODE_NA,         utilGeneral_WCOL,           CLI_COM_NORMAL}, //A35G2_BRC_Casper_0046

	//WROW============================================================================================================//WROW    //G100_Doulas_0067
    {eCLI_MAIN_WROW,     NULL,          READ_COMMAND,     DECIAML_2,      DATA_CODE_NA,         utilGeneral_WROW,           CLI_COM_NORMAL},
    {eCLI_MAIN_WROW,     NULL,          WRITE_COMMAND,    DECIAML_2,      DATA_CODE_NA,         utilGeneral_WROW,           CLI_COM_NORMAL}, //A35G2_BRC_Casper_0046
 //G100_Steven_0048 start
	//SEQM============================================================================================================//SEQM
    {eCLI_MAIN_SEQM,     NULL,          READ_COMMAND,     DECIAML_2,      DATA_CODE_NA,         utilGeneral_SEQM,           CLI_COM_NORMAL},
    //{eCLI_MAIN_SEQM,     NULL,          WRITE_COMMAND,    DECIAML_2,      DATA_CODE_NA,         utilGeneral_SEQM,           CLI_COM_NORMAL},

	//BRPM============================================================================================================//BRPM
    {eCLI_MAIN_BRPM,     NULL,          READ_COMMAND,     DECIAML_2,      DATA_CODE_NA,         utilGeneral_BRPM,           CLI_COM_NORMAL},
    //{eCLI_MAIN_BRPM,     NULL,          WRITE_COMMAND,    DECIAML_2,      DATA_CODE_NA,         utilGeneral_BRPM,           CLI_COM_NORMAL},

	//BGPM============================================================================================================//BGPM
    {eCLI_MAIN_BGPM,     NULL,          READ_COMMAND,     DECIAML_2,      DATA_CODE_NA,         utilGeneral_BGPM,           CLI_COM_NORMAL},
    //{eCLI_MAIN_BGPM,     NULL,          WRITE_COMMAND,    DECIAML_2,      DATA_CODE_NA,         utilGeneral_BGPM,           CLI_COM_NORMAL},

	//BBPM============================================================================================================//BBPM
    {eCLI_MAIN_BBPM,     NULL,          READ_COMMAND,     DECIAML_2,      DATA_CODE_NA,         utilGeneral_BBPM,           CLI_COM_NORMAL},
    //{eCLI_MAIN_BBPM,     NULL,          WRITE_COMMAND,    DECIAML_2,      DATA_CODE_NA,         utilGeneral_BBPM,           CLI_COM_NORMAL},

	//BYPM============================================================================================================//BYPM
    {eCLI_MAIN_BYPM,     NULL,          READ_COMMAND,     DECIAML_2,      DATA_CODE_NA,         utilGeneral_BYPM,           CLI_COM_NORMAL},
    //{eCLI_MAIN_BYPM,     NULL,          WRITE_COMMAND,    DECIAML_2,      DATA_CODE_NA,         utilGeneral_BYPM,           CLI_COM_NORMAL},

	//RRPM============================================================================================================//RRPM
    {eCLI_MAIN_RRPM,     NULL,          READ_COMMAND,     DECIAML_2,      DATA_CODE_NA,         utilGeneral_RRPM,           CLI_COM_NORMAL},
    //{eCLI_MAIN_RRPM,     NULL,          WRITE_COMMAND,    DECIAML_2,      DATA_CODE_NA,         utilGeneral_RRPM,           CLI_COM_NORMAL},

	//RYPM============================================================================================================//RYPM
    {eCLI_MAIN_RYPM,     NULL,          READ_COMMAND,     DECIAML_2,      DATA_CODE_NA,         utilGeneral_RYPM,           CLI_COM_NORMAL},
    //{eCLI_MAIN_RYPM,     NULL,          WRITE_COMMAND,    DECIAML_2,      DATA_CODE_NA,         utilGeneral_RYPM,           CLI_COM_NORMAL},

	//LEDT============================================================================================================//LEDT
    {eCLI_MAIN_LEDT,     NULL,          READ_COMMAND,     DECIAML_2,      DATA_CODE_NA,         utilGeneral_LEDT,           CLI_COM_NORMAL},
    //{eCLI_MAIN_LEDT,     NULL,          WRITE_COMMAND,    DECIAML_2,      DATA_CODE_NA,         utilGeneral_LEDT,           CLI_COM_NORMAL},

	//SSPA============================================================================================================//SSPA
    {eCLI_MAIN_SSPA,     NULL,          READ_COMMAND,     DECIAML_2,      edcALTIMETRY_VALUE,         NULL,           CLI_COM_NORMAL},
    //{eCLI_MAIN_SSPA,     NULL,          WRITE_COMMAND,    DECIAML_2,      DATA_CODE_NA,         utilGeneral_SSPA,           CLI_COM_NORMAL},

	//SSRH============================================================================================================//SSRH
    {eCLI_MAIN_SSRH,     NULL,          READ_COMMAND,     DECIAML_2,      edcBAROMETRY_VALUE,         NULL,           CLI_COM_NORMAL},
    //{eCLI_MAIN_SSRH,     NULL,          WRITE_COMMAND,    DECIAML_2,      DATA_CODE_NA,         utilGeneral_SSRH,           CLI_COM_NORMAL},

	//DDTP============================================================================================================//DDTP
    {eCLI_MAIN_DDTP,     NULL,          READ_COMMAND,     STRING_CHAR,      DATA_CODE_NA,           utilGeneral_DDTP,           CLI_COM_NORMAL},
    {eCLI_MAIN_DDTP,     NULL,          WRITE_COMMAND,    STRING_CHAR,      DATA_CODE_NA,           utilGeneral_DDTP,           CLI_COM_NORMAL},

	//FANF============================================================================================================//FANF
    //{eCLI_MAIN_FANF,     NULL,          READ_COMMAND,     DECIAML_2,      edcFAN_FILTER,               NULL,           CLI_COM_NORMAL},
    //{eCLI_MAIN_DDTP,     NULL,          WRITE_COMMAND,    DECIAML_2,      DATA_CODE_NA,         utilGeneral_LEDT,           CLI_COM_NORMAL},

	//FPON============================================================================================================//FPON	//G100_Clare_0055
    {eCLI_MAIN_FPON,     NULL,          READ_COMMAND,     DECIAML_2,      edcFAST_POWER_ON,        NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_FPON,     NULL,          WRITE_COMMAND,    DECIAML_2,      edcFAST_POWER_ON,        NULL,           CLI_COM_NORMAL},

    {eCLI_MAIN_ICPG,     NULL,         WRITE_COMMAND,     DECIAML_2,      DATA_CODE_NA,      utilGeneral_ICP,          CLI_COM_NORMAL | CLI_COM_NO_REPLY}, //A70LV_Larry_0086
    {eCLI_MAIN_ICPG,     NULL,         READ_COMMAND,      DECIAML_2,      DATA_CODE_NA,      utilGeneral_ICP,          CLI_COM_NORMAL | CLI_COM_NO_REPLY}, //A70LV_Larry_0086
    {eCLI_MAIN_ICPG,     "SET3",       WRITE_COMMAND,     DECIAML_2,      DATA_CODE_NA,      utilGeneral_ICPC341_SET,  CLI_COM_NORMAL | CLI_COM_NO_REPLY},
    {eCLI_MAIN_ICPG,     "GET3",       WRITE_COMMAND,     DECIAML_2,      DATA_CODE_NA,      utilGeneral_ICPC341_GET,  CLI_COM_NORMAL | CLI_COM_NO_REPLY},

 //A35G2_BRC_Casper_0046
    {eCLI_MAIN_WPTP,     NULL,          READ_COMMAND,     DECIAML_2,      DATA_CODE_NA,        utilGeneral_WPTP,           CLI_COM_NORMAL},
    {eCLI_MAIN_WPTP,     NULL,          WRITE_COMMAND,    DECIAML_2,      DATA_CODE_NA,        utilGeneral_WPTP,           CLI_COM_NORMAL},
    {eCLI_MAIN_WPPU,     NULL,          WRITE_COMMAND,    DECIAML_2,      DATA_CODE_NA,        utilGeneral_WPPU,           CLI_COM_NORMAL},
    {eCLI_MAIN_WPPD,     NULL,          WRITE_COMMAND,    DECIAML_2,      DATA_CODE_NA,        utilGeneral_WPPD,           CLI_COM_NORMAL},
    {eCLI_MAIN_WPPL,     NULL,          WRITE_COMMAND,    DECIAML_2,      DATA_CODE_NA,        utilGeneral_WPPL,           CLI_COM_NORMAL},
    {eCLI_MAIN_WPPR,     NULL,          WRITE_COMMAND,    DECIAML_2,      DATA_CODE_NA,        utilGeneral_WPPR,           CLI_COM_NORMAL},
    {eCLI_MAIN_WPXY,     NULL,          READ_COMMAND,     STRING_CHAR,    DATA_CODE_NA,        utilGeneral_WPXY,           CLI_COM_NORMAL},
    {eCLI_MAIN_WPXY,     NULL,          WRITE_COMMAND,    STRING_CHAR,    DATA_CODE_NA,        utilGeneral_WPXY,           CLI_COM_NORMAL},
    {eCLI_MAIN_WRST,     NULL,          WRITE_COMMAND,    DECIAML_2,      DATA_CODE_NA,        utilGeneral_WRST,           CLI_COM_NORMAL},

//A35G2_BRC_Casper_0047
    //LMCP============================================================================================================//LMCP   //G100_Steven_0098
    {eCLI_MAIN_LMCP,    NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,             utilGeneral_LMCP,          CLI_COM_NO_REPLY|CLI_COM_READ_ONLY}, //G100_Steven_0101
    //GMCC============================================================================================================//GMCC
    {eCLI_MAIN_GMCC,    NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,             utilGeneral_GMCC,          CLI_COM_NO_REPLY|CLI_COM_READ_ONLY}, //G100_Steven_0101
    //UDCC============================================================================================================//UDCC
    {eCLI_MAIN_UDCC,    NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,             utilGeneral_UDCC,          CLI_COM_NO_REPLY|CLI_COM_READ_ONLY}, //G100_Steven_0101

    //HUTP============================================================================================================//HUTP   //G100_Steven_0118 //A35G2_BRC_Casper_0051
    {eCLI_MAIN_HUTP,    NULL,       READ_COMMAND,     DECIAML_2,          edcTHERMAL_SENSOR_2,            NULL,       CLI_COM_READ_ONLY},

    //DDTC============================================================================================================//LMCP   //G100_Steven_0118 //A35G2_BRC_Casper_0051
    {eCLI_MAIN_DDTC,    NULL,       READ_COMMAND,     DECIAML_2,          DATA_CODE_NA,             utilGeneral_DDTC,          CLI_COM_NO_REPLY|CLI_COM_READ_ONLY},

    //G100_Tim_0057, add, start //A35G2_BRC_Casper_0060
    //AHSW============================================================================================================//AHSW
    {eCLI_MAIN_AHSW,    NULL,           READ_COMMAND,     DECIAML_2,      edcAUTO_HDMI_SWITCH,      NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_AHSW,    NULL,           WRITE_COMMAND,    DECIAML_2,      edcAUTO_HDMI_SWITCH,      NULL,           CLI_COM_NORMAL},
    //G100_Tim_0057, add, end

    //G100_Tim_0058, add, start //A35G2_BRC_Casper_0060
    //PJNT============================================================================================================//PJNT
    {eCLI_MAIN_PJNT,    NULL,           READ_COMMAND,     DECIAML_2,      edcPROJ_NATIVE_TIMING,    NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_PJNT,    NULL,           WRITE_COMMAND,    DECIAML_2,      edcPROJ_NATIVE_TIMING,    NULL,           CLI_COM_NORMAL},
    //G100_Tim_0058, add, end
	{eCLI_MAIN_UST,     NULL,       WRITE_COMMAND,    DECIAML_2,        DATA_CODE_NA, 	    utilGeneral_CLI_UST, 	    CLI_COM_NORMAL | CLI_COM_NEED_SERVICE_CODE_WRITE},		//ZU860_Julie_0002 //A35G2_BRC_Casper_0088
    {eCLI_MAIN_UST,     NULL,       READ_COMMAND,     DECIAML_2,        DATA_CODE_NA,	    utilGeneral_CLI_UST,        CLI_COM_NORMAL | CLI_COM_NEED_SERVICE_CODE_WRITE}, //A35G2_BRC_Casper_0088
    {eCLI_MAIN_USTS,    NULL,       WRITE_COMMAND,    DECIAML_2,        DATA_CODE_NA,       utilGeneral_CLI_USTS,       CLI_COM_NORMAL | CLI_COM_NEED_SERVICE_CODE_WRITE},//SNPLU9000_Energy_0011 //A35G2_BRC_Casper_0088
    {eCLI_MAIN_USTS,    NULL,       READ_COMMAND,     DECIAML_2,        DATA_CODE_NA,       utilGeneral_CLI_USTS,       CLI_COM_NORMAL | CLI_COM_NEED_SERVICE_CODE_WRITE},//SNPLU9000_Energy_0011 //A35G2_BRC_Casper_0088

    //BODG=================================================================================================//BODG
    {eCLI_MAIN_BODG,    "CUST",     WRITE_COMMAND,    DECIAML_2,        DATA_CODE_NA,       utilGeneral_BODCUST,       CLI_COM_NORMAL | CLI_COM_NEED_SERVICE_CODE_WRITE},
    {eCLI_MAIN_BODG,    "CUST",     READ_COMMAND,     DECIAML_2,        DATA_CODE_NA,       utilGeneral_BODCUST,       CLI_COM_NORMAL},
    {eCLI_MAIN_BODG,    "PLAT",     WRITE_COMMAND,    DECIAML_2,        DATA_CODE_NA,       utilGeneral_BODPLAT,       CLI_COM_NORMAL | CLI_COM_NEED_SERVICE_CODE_WRITE},
    {eCLI_MAIN_BODG,    "PLAT",     READ_COMMAND,     DECIAML_2,        DATA_CODE_NA,       utilGeneral_BODPLAT,       CLI_COM_NORMAL},

    //OPRA============================================================================================================//OPRA //A65_OPTOMA_Julie_0022//A35G2_Coda_0110
    {eCLI_MAIN_OPRA, 	NULL,		WRITE_COMMAND,	  DECIAML_2, 		DATA_CODE_NA,		utilGeneral_OPDRSET, 	   CLI_COM_NORMAL},
    //{eCLI_MAIN_OPRA, 	NULL,		   READ_COMMAND,	 DECIAML_2, 		 DATA_CODE_NA,				  NULL, 		  CLI_COM_NORMAL},

    //CBMP============================================================================================================//CBMP //A35G2_BRC_Casper_0145
    {eCLI_MAIN_CBMP,    NULL,       WRITE_COMMAND,    DECIAML_4,        DATA_CODE_NA,       utilGeneral_CLI_CBMP,      CLI_COM_WRITE_ONLY | CLI_COM_NO_REPLY},  //A35G2_Simon_0114 //A35G2_BRC_Casper_0145  //A35G2_Simon_0116

	//TDAR============================================================================================================//TDAR
    {eCLI_MAIN_TDAR,    NULL,           READ_COMMAND,     DECIAML_2,      edcSIZE_PRESETS_3D,      NULL,           CLI_COM_NORMAL},    //G100_Steven_0163 //A35G2_BRC_Casper_0146
    {eCLI_MAIN_TDAR,    NULL,           WRITE_COMMAND,    DECIAML_2,      edcSIZE_PRESETS_3D,      NULL,           CLI_COM_NORMAL},    //G100_Steven_0163 //A35G2_BRC_Casper_0146

#if (ENABLE_COLOR_UNIFORMITY == TRUE) //G100_Tim_0012, add, start //A35G2_BRC_Casper_0152
    //ACUC============================================================================================================//ACUC
    {eCLI_MAIN_ACUE,    NULL,           READ_COMMAND,     DECIAML_2,      edcACU_EXECUTE,           NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_ACUE,    NULL,           WRITE_COMMAND,    DECIAML_2,      edcACU_EXECUTE,           NULL,           CLI_COM_NORMAL},

    //ACUS============================================================================================================//ACUS
    {eCLI_MAIN_ACUS,    NULL,           READ_COMMAND,     DECIAML_2,      edcACU_STATUS,            NULL,           CLI_COM_NORMAL},
    //{eCLI_MAIN_ACUS,    NULL,           WRITE_COMMAND,    DECIAML_2,      edcACU_STATUS,            NULL,           CLI_COM_NORMAL}, //not allow to set ACU status via RS232 Cmd

    //ACUT============================================================================================================//ACUT
    {eCLI_MAIN_ACUT,    NULL,           READ_COMMAND,      DECIAML_2,   edcACU_TARGET_SEL,          NULL,           CLI_COM_NORMAL},
    {eCLI_MAIN_ACUT,    NULL,           WRITE_COMMAND,     DECIAML_2,   edcACU_TARGET_SEL,          NULL,           CLI_COM_NORMAL},

    //ACUR============================================================================================================//ACUR
    {eCLI_MAIN_ACUR,    NULL,           READ_COMMAND,     DECIAML_2,      edcACU_RESET,             NULL,           CLI_COM_NORMAL}, //not allow
    {eCLI_MAIN_ACUR,    NULL,           WRITE_COMMAND,    DECIAML_2,      edcACU_RESET,             NULL,           CLI_COM_NORMAL},
#endif //ENABLE_COLOR_UNIFORMITY      //G100_Tim_0012, add, end

};

#define GENERAL_CMD_LUT_NUMBER sizeof(m_sSubLut)/sizeof(sCLI_SUB_LUT)

//G100_Wilsonj_0050 Start
// ==============================================================================
// FUNCTION NAME: utilGeneral_CLI_AddressID
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
void utilGeneral_CLI_AddressID(UINT32 ulAddress)
{
    m_ulAddressID = ulAddress;
}
//G100_Wilsonj_0050 End

// ==============================================================================
// FUNCTION NAME: utilGeneral_CMD_Decode
// DESCRIPTION:
//
//
// Params:
// UINT8* pcString:
// sCLI_GENERAL_FORMAT* sCmdFormat:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/11/08, Larry Create
// --------------------
// ==============================================================================
UINT8 utilGeneral_CMD_Decode(UINT8* pcString, sCLI_GENERAL_FORMAT* sCmdFormat)
{
    UINT8   ucStringLen = 0;
    UINT8   ucErrorCode = eCLI_ERROR_CODE_NO;
    UINT8   ucCmdIndex = 0;
    UINT8   ucDecdoeStep = eCLI_DECODE_STEP_HEADER;
    UINT8   ucCount = 0;
    UINT8   ucFoundSpece = 0;
    UINT16  uiMainCode = 0;
    UINT16  uiSubCode = 0;

    ucStringLen = strlen((char*)pcString);

    if(ucStringLen > GENERAL_CMD_LENGTH_MAX && ucStringLen < GENERAL_CMD_LENGTH_MIN)
    {
        return eCLI_ERROR_CODE_LENGTH;
    }

    do
    {
        switch(ucDecdoeStep)
        {
            case eCLI_DECODE_STEP_HEADER:
                if(pcString[ucCmdIndex] == GENERAL_CMD_HEADER_1)
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
                        case GENERAL_CMD_RPR_CHAR_SIMPLE_ACK:
                            sCmdFormat->ucPrefixCharType = eCLI_PREFIX_CHAR_SIMPLE_ACK;
                            ucCmdIndex++;
                            break;
                        case GENERAL_CMD_RPR_CHAR_FULL_ACK:
                            sCmdFormat->ucPrefixCharType = eCLI_PREFIX_CHAR_FULL_SUM;
                            ucCmdIndex++;
                            break;
                        case GENERAL_CMD_RPR_CHAR_CHECK_SUM:
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

                            if(ucCount >= GENERAL_ADDRESS_LEN_MAX)
                            {
                                return eCLI_ERROR_CODE_DESTADDERR;
                            }
                        }
                        else if(pcString[ucCmdIndex] == GENERAL_CMD_SPACE)
                        {
                            ucCmdIndex++;
							sCmdFormat->ulProjectorAddressID = (UINT32)atoi(sCmdFormat->uiProjectorAddress);
                            break;
                        }
                        else //other
                        {
                            sCmdFormat->ulProjectorAddressID = (UINT32)atoi(sCmdFormat->uiProjectorAddress);
                            //ucDecdoeStep = eCLI_DECODE_STEP_MAIN;
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

                                if(ucCount >= GENERAL_ADDRESS_LEN_MAX)
                                {
                                    return eCLI_ERROR_CODE_SRCADDERR;
                                }
                            }
                            else
                            {
                                break;
                            }
                        }
                        sCmdFormat->ucHasHost = 1;
                        sCmdFormat->ulProjectorHostID = (UINT32)atoi(sCmdFormat->uiProjectorHost);
                    }
                }
                ucDecdoeStep = eCLI_DECODE_STEP_MAIN;
                break;

            case eCLI_DECODE_STEP_MAIN:
                if((ucCmdIndex + GENERAL_MAIN_CMD_LEN) >= ucStringLen) //(XXX > cmd length
                {
                    return eCLI_ERROR_CODE_LENGTH;
                }
                else
                {
                    strncpy((char*)&sCmdFormat->cMainCode[0], (char*)&pcString[ucCmdIndex], GENERAL_MAIN_CMD_LEN);

                    ucCmdIndex += GENERAL_MAIN_CMD_LEN;

                    //Lookup main code
                    for(uiMainCode = 0; uiMainCode <= CLI_MAINLUT_NUMBER; uiMainCode++)
                    {
                        if((m_sMainLut[uiMainCode].uiMainCmdID == eCLI_MAIN_END) || (uiMainCode == CLI_MAINLUT_NUMBER))
                        {
                            return eCLI_ERROR_CODE_FUNCCODENOTFOUND;
                        }
                        else
                        {
                            if(__CheckCmd(sCmdFormat->cMainCode, m_sMainLut[uiMainCode].cMainCmd, GENERAL_MAIN_CMD_LEN))
                            {
                                sCmdFormat->cMainCode[GENERAL_MAIN_CMD_LEN] = '\0';
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

                        if(pcString[ucCmdIndex] == GENERAL_CMD_SUB_CONNECT) //has sub code "+"
                        {
                            ucCmdIndex++;
                            if((ucCmdIndex + GENERAL_SUB_CMD_LEN) >= ucStringLen) //(XXX > cmd length
                            {
                                return eCLI_ERROR_CODE_LENGTH;
                            }
                            strncpy((char*)&sCmdFormat->cSubCode[0], (char*)&pcString[ucCmdIndex], GENERAL_SUB_CMD_LEN);

                            sCmdFormat->cSubCode[GENERAL_SUB_CMD_LEN] = '\0';
                            sCmdFormat->ucHasSubCode = 1;
                            ucCmdIndex += GENERAL_SUB_CMD_LEN;
                        }
                        else if(pcString[ucCmdIndex] == GENERAL_CMD_SPACE && ucFoundSpece)
                        {
                            return eCLI_ERROR_CODE_DATAERROR;
                        }
                        else if(pcString[ucCmdIndex] == GENERAL_CMD_SPACE)
                        {
                            ucFoundSpece = 1;
                            ucCmdIndex++;
                        }
                        else
                        {
                            sCmdFormat->cSubCode[0] = '\0';
                        }

                        for(ucCount = ucCmdIndex; ucCount < ucStringLen; ucCount++) //Cmd Is Read?
                        {
                            if(pcString[ucCount] == GENERAL_CMD_SPACE)
                            {
                                continue;
                            }
                            else if(pcString[ucCount] == GENERAL_CMD_REQUEST)
                            {
                                sCmdFormat->ucIsRead = READ_COMMAND;
                                break;
                            }
                            else
                            {
                                sCmdFormat->ucIsRead = WRITE_COMMAND;
                                break;
                            }
                        }

                        //Lookup sub code
                        for(uiSubCode = 0; uiSubCode < GENERAL_CMD_LUT_NUMBER; uiSubCode++)
                        {
                            if((m_sSubLut[uiSubCode].uiMainCmdID == m_sMainLut[uiMainCode].uiMainCmdID) && (m_sSubLut[uiSubCode].ucCmdIsRead == sCmdFormat->ucIsRead))
                            {
                                if((sCmdFormat->ucHasSubCode == 0) && (m_sSubLut[uiSubCode].cSubCmd == NULL))
                                {
                                    sCmdFormat->uiFuncID = uiSubCode;
									ucDecdoeStep = eCLI_DECODE_STEP_DATA_FORMAT;
                                    break;
                                }
                                else
                                {
                                    if(m_sSubLut[uiSubCode].cSubCmd != NULL)
                                    {
                                        if(__CheckCmd(sCmdFormat->cSubCode, m_sSubLut[uiSubCode].cSubCmd, GENERAL_SUB_CMD_LEN))
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

                        if(uiSubCode == GENERAL_CMD_LUT_NUMBER)
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
                    }
                    break;

                    case eCLI_DECODE_STEP_DATA_FORMAT:
                            if(pcString[ucCmdIndex] == GENERAL_CMD_END_1)
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
                                        if(pcString[ucCount] == GENERAL_CMD_SPACE)
                                        {
                                            if(ucFoundSpece)
                                            {
                                                return eCLI_ERROR_CODE_DATAERROR;
                                            }
                                            ucFoundSpece++;
                                        }
                                        else if(pcString[ucCount] == GENERAL_CMD_DATA_STRING)
                                        {
                                            ucCmdIndex = ucCount;
                                            ucCmdIndex++;
                                            sCmdFormat->ucDataType = eCLI_DATA_TYPE_STRING;
                                            break;
                                        }
                                        else if((pcString[ucCount] == GENERAL_CMD_DATA_P) || (pcString[ucCount] == GENERAL_CMD_DATA_p))
                                        {
                                            ucCmdIndex = ucCount;
                                            ucCmdIndex++;
                                            sCmdFormat->ucDataType = eCLI_DATA_TYPE_P;
                                            break;
                                        }
                                        else if((pcString[ucCount] == GENERAL_CMD_DATA_N) || (pcString[ucCount] == GENERAL_CMD_DATA_n))
                                        {
                                            ucCmdIndex = ucCount;
                                            ucCmdIndex++;
                                            sCmdFormat->ucDataType = eCLI_DATA_TYPE_N;
                                            break;
                                        }
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

                                            if(pcString[ucCount] == GENERAL_CMD_SPACE)
                                            {
                                                ucCmdIndex++;
                                            }

                                            for(ucCount = ucCmdIndex; ucCount < ucStringLen; ucCount++)
                                            {
                                                if(pcString[ucCount] == '-')
                                                {
                                                    //ucDataIndex++;
                                                    ucNegative = 1;
                                                }
                                                else if((pcString[ucCount] == '-') && (ucNegative == 1))
                                                {
                                                    return eCLI_ERROR_CODE_NEGATIVEERR;
                                                }
                                                else if(pcString[ucCount] == '.')
                                                {
                                                    aucDataTemp[ucDataIndex++] = pcString[ucCount];
                                                    ucIsPoint = 1;
                                                }
                                                else if((pcString[ucCount] == '.') && (ucIsPoint == 1))
                                                {
                                                    return eCLI_ERROR_CODE_DECIMALERR;
                                                }
                                                else if(__CheckNum(pcString[ucCount]))
                                                {
                                                    aucDataTemp[ucDataIndex++] = pcString[ucCount];
                                                }
                                                else if((pcString[ucCount] == GENERAL_CMD_END_1) ||
                                                        (pcString[ucCount] == GENERAL_CMD_SPACE)) //end
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
                                        if((ucCmdIndex + GENERAL_CMD_TEXT_SIZE + 2) < ucStringLen) //+2 for end ' " 'and ')'
                                        {
                                            //LOG_MSG(db_DV_FAN, "STRING %d %d %d\n", ucCmdIndex, GENERAL_CMD_TEXT_SIZE, ucStringLen);

                                            return eCLI_ERROR_CODE_STRINGTOOBIG;
                                        }
                                        else
                                        {
                                            UINT8 ucStringIndex = 0;

                                            for(ucCount = ucCmdIndex; ucCount < ucStringLen; ucCount++)
                                            {
                                                if(pcString[ucCount] == GENERAL_CMD_DATA_STRING) //end
                                                {
                                                    sCmdFormat->cTextString[ucStringIndex] = '\0';
                                                    ucCmdIndex = ucCount;
                                                    ucCmdIndex++;

                                                    //LOG_MSG(db_DV_FAN, "STRING %s\n", sCmdFormat->cTestString);
                                                    break;
                                                }
												else if((pcString[ucCount] != GENERAL_CMD_HEADER_1) &&
                                                        (pcString[ucCount] != GENERAL_CMD_END_1) &&
                                                        (pcString[ucCount] != GENERAL_CMD_DIS_CHAR_1))
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
                                }
                                ucDecdoeStep = eCLI_DECODE_STEP_END;
                            }
                        break;

                    case eCLI_DECODE_STEP_END:
                        if(sCmdFormat->ucPrefixCharType == eCLI_PREFIX_CHAR_CHECKSUM)
                        {
                            if(pcString[ucCmdIndex] != GENERAL_CMD_SPACE)
                            {
                                return eCLI_ERROR_CODE_CKSUMNPSPACE;
                            }
                            else
                            {
                                UINT8 ucDataCheckSum = 0;

                                UINT8 aucCheckSum[GENERAL_CHECKSUM_LEN]={'\0'};

                                for(ucCount = 1; ucCount <= ucCmdIndex; ucCount++) //&SYS ~ ' '
                                {
                                    ucDataCheckSum += pcString[ucCount];
                                }

                                ucCmdIndex++;

                                for(ucCount = 0; ucCount < GENERAL_CHECKSUM_LEN; ucCount++)
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

                                if(pcString[ucCmdIndex] == GENERAL_CMD_END_1)
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
                        else if(pcString[ucCmdIndex] == GENERAL_CMD_END_1)
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
// FUNCTION NAME: utilGeneral_CMD_Reply
// DESCRIPTION:
//
//
// Params:
// UINT8 ucChannel:
// UINT8 ucErrorCode:
// sCLI_GENERAL_FORMAT* sCmdFormat:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/11/08, Larry Create
// --------------------
// ==============================================================================
void utilGeneral_CMD_Reply(UINT8 ucChannel, UINT8 ucErrorCode, sCLI_GENERAL_FORMAT* sCmdFormat, char *ucReturnString)
{
    //char ucReturnString[GENERAL_CMD_LENGTH_MAX] = {'\0'};
    UINT8 ucDataType = utilCommonCLI_DataTypeGet(m_sSubLut[sCmdFormat->uiFuncID].uiFunCodeID); //A70LV_Larry_0065

    if((m_sSubLut[sCmdFormat->uiFuncID].uiSpecialFlag & CLI_COM_NO_REPLY) && (ucErrorCode == eCLI_ERROR_CODE_NO))
    {
        return;
    }

    if((ucErrorCode != eCLI_ERROR_CODE_NO) && (ucErrorCode < eCLI_ERROR_NUMBER))
    {
        switch(ucErrorCode)
        {
            case eCLI_ERROR_CODE_SUBCODEERR:
            case eCLI_ERROR_CODE_SUBCODENEED:
                sprintf(ucReturnString, "%cERR \"00000 %s%c%s: %s\"%c", m_sCli_General_Config[ucChannel].cCMD_Hander,  sCmdFormat->cMainCode, GENERAL_CMD_SUB_CONNECT, sCmdFormat->cSubCode, m_sErrorLut[ucErrorCode].pcErrorMsg, m_sCli_General_Config[ucChannel].cCMD_End); //G100_Steven_0130
                break;

            default :
                sprintf(ucReturnString, "%cERR \"00000 %s: %s\"%c", m_sCli_General_Config[ucChannel].cCMD_Hander, sCmdFormat->cMainCode, m_sErrorLut[ucErrorCode].pcErrorMsg, m_sCli_General_Config[ucChannel].cCMD_End);
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
                    sprintf(ucReturnString, "%c", GENERAL_CMD_RPR_CHAR_SIMPLE_ACK);
                    break;

                case eCLI_PREFIX_CHAR_FULL_SUM:
                    switch(sCmdFormat->ucDataType)
                    {
                        case eCLI_DATA_TYPE_DEC:
                        case eCLI_DATA_TYPE_POINT:
                            {
                                char ucaCLITemp[11] = {'\0'};

                                if(sCmdFormat->ucHasSubCode)
                                {
                                    sprintf(ucaCLITemp, "%s%c%s%c", sCmdFormat->cMainCode, GENERAL_CMD_SUB_CONNECT, sCmdFormat->cSubCode, GENERAL_CMD_REPLY);
                                }
                                else
                                {
                                    sprintf(ucaCLITemp, "%s%c", sCmdFormat->cMainCode, GENERAL_CMD_REPLY);
                                }

                                if(sCmdFormat->ucDataType == eCLI_DATA_TYPE_DEC)
                                {
                                    switch(m_sSubLut[sCmdFormat->uiFuncID].ucCmdBytesOnRange)
                                    {
                                        case DECIAML_2:
                                            sprintf(ucReturnString, "%c%s%02d%c", m_sCli_General_Config[ucChannel].cCMD_Hander, ucaCLITemp, sCmdFormat->lData, m_sCli_General_Config[ucChannel].cCMD_End);
                                            break;

                                        case DECIAML_3:
                                            sprintf(ucReturnString, "%c%s%03d%c", m_sCli_General_Config[ucChannel].cCMD_Hander, ucaCLITemp, sCmdFormat->lData, m_sCli_General_Config[ucChannel].cCMD_End);
                                            break;

                                        case DECIAML_4:
                                            sprintf(ucReturnString, "%c%s%04d%c", m_sCli_General_Config[ucChannel].cCMD_Hander, ucaCLITemp, sCmdFormat->lData, m_sCli_General_Config[ucChannel].cCMD_End);
                                            break;

                                        case DECIAML_5:
                                            sprintf(ucReturnString, "%c%s%05d%c", m_sCli_General_Config[ucChannel].cCMD_Hander, ucaCLITemp, sCmdFormat->lData, m_sCli_General_Config[ucChannel].cCMD_End);
                                            break;
                                    }
                                }
                                else
                                {
                                    switch(m_sSubLut[sCmdFormat->uiFuncID].ucCmdBytesOnRange)
                                    {
                                        case DECIAML_2:
                                            sprintf(ucReturnString, "%c%s%02f%c", m_sCli_General_Config[ucChannel].cCMD_Hander, ucaCLITemp, sCmdFormat->fData, m_sCli_General_Config[ucChannel].cCMD_End);
                                            break;

                                        case DECIAML_3:
                                            sprintf(ucReturnString, "%c%s%03f%c", m_sCli_General_Config[ucChannel].cCMD_Hander, ucaCLITemp, sCmdFormat->fData, m_sCli_General_Config[ucChannel].cCMD_End);
                                            break;

                                        case DECIAML_4:
                                            sprintf(ucReturnString, "%c%s%04f%c", m_sCli_General_Config[ucChannel].cCMD_Hander, ucaCLITemp, sCmdFormat->fData, m_sCli_General_Config[ucChannel].cCMD_End);
                                            break;

                                        case DECIAML_5:
                                            sprintf(ucReturnString, "%c%s%05f%c", m_sCli_General_Config[ucChannel].cCMD_Hander, ucaCLITemp, sCmdFormat->fData, m_sCli_General_Config[ucChannel].cCMD_End);
                                            break;
                                    }
                                }
                            }
                            break;

                        case eCLI_DATA_TYPE_STRING:
                            if(sCmdFormat->ucHasSubCode)
                            {
                                sprintf(ucReturnString, "%c%s%c%s%c%s%c", m_sCli_General_Config[ucChannel].cCMD_Hander, sCmdFormat->cMainCode, GENERAL_CMD_SUB_CONNECT, sCmdFormat->cSubCode, GENERAL_CMD_REPLY, sCmdFormat->cTextString, m_sCli_General_Config[ucChannel].cCMD_End);
                            }
                            else
                            {
                                sprintf(ucReturnString, "%c%s%c%s%c", m_sCli_General_Config[ucChannel].cCMD_Hander, sCmdFormat->cMainCode, GENERAL_CMD_REPLY, sCmdFormat->cTextString, m_sCli_General_Config[ucChannel].cCMD_End);
                            }
                            break;

                        default:
                            if(sCmdFormat->ucHasSubCode)
                            {
                                sprintf(ucReturnString, "%c%s%c%s%c%c", m_sCli_General_Config[ucChannel].cCMD_Hander, sCmdFormat->cMainCode, GENERAL_CMD_SUB_CONNECT, sCmdFormat->cSubCode, GENERAL_CMD_REPLY, m_sCli_General_Config[ucChannel].cCMD_End);
                            }
                            else
                            {
                                sprintf(ucReturnString, "%c%s%c%c", m_sCli_General_Config[ucChannel].cCMD_Hander, sCmdFormat->cMainCode, GENERAL_CMD_REPLY, m_sCli_General_Config[ucChannel].cCMD_End);
                            }
                            break;

                    }
                    break;

                default:
                    break;
                }

                //__CMD_Respond(ucChannel, ucReturnString);
                //LOG_MSG(db_ALWAYS, "%s", ucaReturnString);

            }
            else //Read
            {
                char ucCLITemp[32] = {'\0'};
                char ucCLITempAddress[16] = {'\0'};

                if(sCmdFormat->ucHasHost)
                {
                    sprintf(ucCLITempAddress, "%05d %05d", sCmdFormat->ulProjectorHostID, sCmdFormat->ulProjectorAddressID);
                }

                if(sCmdFormat->ucHasSubCode)
                {
                    sprintf(ucCLITemp, "%s%s%c%s%c", ucCLITempAddress, sCmdFormat->cMainCode, GENERAL_CMD_SUB_CONNECT, sCmdFormat->cSubCode, GENERAL_CMD_REPLY);
                }
                else
                {
                    sprintf(ucCLITemp, "%s%s%c", ucCLITempAddress, sCmdFormat->cMainCode, GENERAL_CMD_REPLY);
                }

                switch(ucDataType)
                {
                    case DATATYPE_DIGIT:
                        switch(m_sSubLut[sCmdFormat->uiFuncID].ucCmdBytesOnRange)
                        {
                            case DECIAML_2:
                                sprintf(ucReturnString, "%c%s%02d%c", m_sCli_General_Config[ucChannel].cCMD_Hander, ucCLITemp, sCmdFormat->lData, m_sCli_General_Config[ucChannel].cCMD_End);
                                break;

                            case DECIAML_3:
                                sprintf(ucReturnString, "%c%s%03d%c", m_sCli_General_Config[ucChannel].cCMD_Hander, ucCLITemp, sCmdFormat->lData, m_sCli_General_Config[ucChannel].cCMD_End);
                                break;

                            case DECIAML_4:
                                sprintf(ucReturnString, "%c%s%04d%c", m_sCli_General_Config[ucChannel].cCMD_Hander, ucCLITemp, sCmdFormat->lData, m_sCli_General_Config[ucChannel].cCMD_End);
                                break;

                            case DECIAML_5:
                                sprintf(ucReturnString, "%c%s%05d%c", m_sCli_General_Config[ucChannel].cCMD_Hander, ucCLITemp, sCmdFormat->lData, m_sCli_General_Config[ucChannel].cCMD_End);
                                break;

                            //G100_Tim_002, add, start
                            case STRING_CHAR:
                                sprintf(ucReturnString, "%c%s%s%c", m_sCli_General_Config[ucChannel].cCMD_Hander, ucCLITemp, sCmdFormat->cTextString,  m_sCli_General_Config[ucChannel].cCMD_End);
                                break;
                            //G100_Tim_002, add, end

							default:
								break;
                        }
                        break;

                    case DATATYPE_STRING: //G100_Steven_0073, Basil 要求 uart read command, string log 不加雙引號
                        {
                            sprintf(ucReturnString, "%c%s%s%c", m_sCli_General_Config[ucChannel].cCMD_Hander, ucCLITemp, sCmdFormat->cTextString,  m_sCli_General_Config[ucChannel].cCMD_End);
                        }
                        break;

                    default:
                        break;
                }
                //__CMD_Respond(ucChannel, ucReturnString);

                //LOG_MSG(db_ALWAYS, "%s", ucaReturnString);
            }
    }
    return;
}

// ==============================================================================
// FUNCTION NAME: utilGeneral_CMD_Handle
// DESCRIPTION:
//
//
// Params:
// sCLI_GENERAL_FORMAT* sCmdFormat:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/11/08, Larry Create
// --------------------
// ==============================================================================
UINT8 utilGeneral_CMD_Handle(sCLI_GENERAL_FORMAT* sCmdFormat)
{
    UINT8 ucDataType = 0;

    ucDataType = utilCommonCLI_DataTypeGet(m_sSubLut[sCmdFormat->uiFuncID].uiFunCodeID);

    if(sCmdFormat->uiFuncID < GENERAL_CMD_LUT_NUMBER)
    {
        if(sCmdFormat->ucIsRead == WRITE_COMMAND) //Write command
        {
            #ifdef NO_POWER_OFF_DURING_CAMERA_WORKING			//G100_Tim_0046, add, start
			if( palDataMgr_Get_Camera_Working_Status() == eCAMERA_STATUS_WORKING )
			{
				// not allowed to change setting !!!
				return eCLI_ERROR_CODE_SETFAIL;
			}
			#endif //NO_POWER_OFF_DURING_CAMERA_WORKING 		//G100_Tim_0046, add, end

            if( palDataMgr_Camera_OSD_Lock_Get() == eOSD_LOCKED ) // not allow to do anything. //A35G2_Coda_0052
            {
                if(__CheckCmd(sCmdFormat->cMainCode, POWR_CMD, 4) != 1 ) //G100_Coda_00106
                {
                    //B35LC_Tim_0049, del
                    return eCLI_ERROR_CODE_SETFAIL;
                }
            }

            if(m_sSubLut[sCmdFormat->uiFuncID].uiSpecialFlag & CLI_COM_NEED_SERVICE_CODE_WRITE) //A70LV_Larry_0235
            {
                if(palDataMgr_ServiceModeGet() == ets_OFF)
                {
                    return eCLI_ERROR_CODE_SERVICE_CODE_NEED_WHEN_WRITE;
                }
            }

            if(palDataMgr_PIN_Protect_Checking() == ePASSWORD_PROTECT_LOCKED)        //A70LV_Doulas_0222 Check pin protect
            {
                return eCLI_ERROR_CODE_SERVICE_CODE_NEED_WHEN_WRITE;
            }


            if((m_sSubLut[sCmdFormat->uiFuncID].uiSpecialFlag & CLI_COM_READ_ONLY))
            {
                return eCLI_ERROR_CODE_COMMAND_READ_ONLY;
            }

            if(m_sSubLut[sCmdFormat->uiFuncID].uiFunCodeID != DATA_CODE_NA)
            {
                switch(utilCommonCLI_DataControl(m_sSubLut[sCmdFormat->uiFuncID].uiFunCodeID))
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
            }
            else //special case, ex, hpbu test, SST
            {
                if(m_sSubLut[sCmdFormat->uiFuncID].iCLI_Replay != NULL)
                {
                    return  m_sSubLut[sCmdFormat->uiFuncID].iCLI_Replay(ecmWrite, sCmdFormat);
                }
                else
                {
                    return eCLI_ERROR_CODE_SETFAIL;
                }
            }

            if(m_sSubLut[sCmdFormat->uiFuncID].uiSpecialFlag & CLI_COM_TYPE_PN_ONLY) //lens shift
            {
                switch(sCmdFormat->ucDataType)
                {
                    case eCLI_DATA_TYPE_N:
                    case eCLI_DATA_TYPE_N_STEP:
                        break;

                    case eCLI_DATA_TYPE_N_STOP:
                        break;

                    case eCLI_DATA_TYPE_N_RUN:
                        break;

                    case eCLI_DATA_TYPE_P:
                    case eCLI_DATA_TYPE_P_STEP:
                        break;

                    case eCLI_DATA_TYPE_P_STOP:
                        break;

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

                        sInputData.uiItemIndex = m_sSubLut[sCmdFormat->uiFuncID].uiFunCodeID;
                        sInputData.uValue.lValue = iData;
                        palDataMgr_NotifyGroupingEvent(sInputData);
                        if(eEXEC_CODE_PASS == utilCommonCLI_RangeCheck(m_sSubLut[sCmdFormat->uiFuncID].uiFunCodeID, iData))
                        {
                            if(eEXEC_CODE_PASS == utilCommonCLI_DataConversionSet(m_sSubLut[sCmdFormat->uiFuncID].uiFunCodeID, iData))
                            {
                                return eCLI_ERROR_CODE_NO;
                            }
                            else
                            {
                                return eCLI_ERROR_CODE_DATAOVERRANGE;
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

                        if(eEXEC_CODE_PASS == utilCommonCLI_String_Set(m_sSubLut[sCmdFormat->uiFuncID].uiFunCodeID, (UINT8*)sCmdFormat->cTextString))
                        {
                            return eCLI_ERROR_CODE_NO;
                        }
                    }
					return eCLI_ERROR_CODE_SETFAIL;

                case eCLI_DATA_TYPE_POINT:
                    break;

                case eCLI_DATA_TYPE_N:
                case eCLI_DATA_TYPE_N_STEP:
                    {
                        if(eEXEC_CODE_PASS == utilCommonCLI_Decrease_Data(m_sSubLut[sCmdFormat->uiFuncID].uiFunCodeID))
                        {
                            return eCLI_ERROR_CODE_NO;
                        }
                    }
					return eCLI_ERROR_CODE_SETFAIL;

                case eCLI_DATA_TYPE_N_STOP:
                    break;

                case eCLI_DATA_TYPE_N_RUN:
                    break;

                case eCLI_DATA_TYPE_P:
                case eCLI_DATA_TYPE_P_STEP:
                    {
                        if(eEXEC_CODE_PASS == utilCommonCLI_Increase_Data(m_sSubLut[sCmdFormat->uiFuncID].uiFunCodeID))
                        {
                            return eCLI_ERROR_CODE_NO;
                        }
                    }
					return eCLI_ERROR_CODE_SETFAIL;

                case eCLI_DATA_TYPE_P_STOP:
                    break;

                case eCLI_DATA_TYPE_P_RUN:
                    break;

                default:
                    break;
            }
        }
        else //Read
        {
            if((m_sSubLut[sCmdFormat->uiFuncID].uiSpecialFlag & CLI_COM_WRITE_ONLY))
            {
                return eCLI_ERROR_CODE_COMMAND_WRITE_ONLY;
            }

#if 0
            if(m_sSubLut[sCmdFormat->uiFuncID].uiSpecialFlag & CLI_COM_NEED_SERVICE_CODE_READ)
            {
                //TBD
                return eCLI_ERROR_CODE_SERVICE_CODE_NEED_WHEN_READ;
            }
#endif /* 0 */

            if(m_sSubLut[sCmdFormat->uiFuncID].uiFunCodeID != DATA_CODE_NA)
            {
                switch(utilCommonCLI_DataControl(m_sSubLut[sCmdFormat->uiFuncID].uiFunCodeID))
                {
                    case eFUNC_CONTROL_ENABLE:
                        break;

                    case eFUNC_CONTROL_SOURCE_DEPEND:
                        return eCLI_ERROR_CODE_MUSTDEPENDONSRC;

                    case eFUNC_CONTROL_SERCIVE_PROTECT:
                        return eCLI_ERROR_CODE_SERVICE_CODE_NEED_WHEN_READ;

                    default:
                        if(!(m_sSubLut[sCmdFormat->uiFuncID].uiSpecialFlag & CLI_COM_IGNORE_CTRL_CHECK))
                        {
                            return eCLI_ERROR_CODE_REQUESTFAIL;
                        }
                }
            }
            else //special case, ex, hpbu test, SST
            {
                if(m_sSubLut[sCmdFormat->uiFuncID].iCLI_Replay != NULL)
                {
                    return m_sSubLut[sCmdFormat->uiFuncID].iCLI_Replay(ecmRead, sCmdFormat);

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

                        if(eEXEC_CODE_PASS == utilCommonCLI_DataConversionGet(m_sSubLut[sCmdFormat->uiFuncID].uiFunCodeID, &iData))
                        {
                            sCmdFormat->lData = (INT32)iData;
                            return eCLI_ERROR_CODE_NO;
                        }
                    }
                    return eCLI_ERROR_CODE_REQUESTFAIL;

                case DATATYPE_STRING:
                    {
                        UINT8 ucString[GENERAL_CMD_LENGTH_MAX] = {'\0'};
                        UINT8 ucStrlen = 0;

                        if(eEXEC_CODE_PASS == utilCommonCLI_String_Get(m_sSubLut[sCmdFormat->uiFuncID].uiFunCodeID, ucString))
                        {
                            ucStrlen = strlen((char*)ucString);

                            if(ucStrlen > GENERAL_CMD_TEXT_SIZE)
                            {
                                ucStrlen = GENERAL_CMD_TEXT_SIZE;
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
// FUNCTION NAME: utilGeneral_CLI_Handle
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
UINT8 utilGeneral_CLI_Handle(UINT8 cCh, UINT8 ucData) //A35G2_BRC_Casper_0047
{
    //static UINT8 ucComma = 0; //ZU860_John_0004 fix general CLI and GEC CLI conflict

    if(cCh >= eccNumber)
        return 0;

    if(sGlobalCfg.sFN_CFG.SupportSerialEcho)
    {
        if(m_sCLI_General_Info[cCh].ucSerialPortEcho)
        {
            LOG_MSG(db_ALWAYS, "%c", ucData);
        }
    }

	utilGeneral_CLI_AMX(ucData);
#if 1
    if(ucData == m_sCli_General_Config[cCh].cCMD_Hander || m_sCLI_General_Info[cCh].uiCLI_Poll_Position != 0)
    {
        m_sCLI_General_Info[cCh].ucCLI_DATA[m_sCLI_General_Info[cCh].uiCLI_Poll_Position++] = ucData;

#if 0 //G100_Steven_0102
 //ZU860_John_0004 start fix general CLI and GEC CLI conflict
        if( ucData == ',' && ucComma < 0xFF)
        {
            ucComma++;
        }
 //ZU860_John_0004 end
#endif

        if(ucData == GENERAL_CMD_HEADER_1) //(sys(sys?)
        {
            m_sCLI_General_Info[cCh].uiCLI_Poll_Position = 0;
            m_sCLI_General_Info[cCh].ucCLI_DATA[m_sCLI_General_Info[cCh].uiCLI_Poll_Position++] = ucData;
        }

        if(m_sCLI_General_Info[cCh].uiCLI_Poll_Position > GENERAL_CMD_LENGTH_MAX)
        {
            m_sCLI_General_Info[cCh].uiCLI_Poll_Position = 0;
            return 0;
        }

        if(ucData ==  m_sCli_General_Config[cCh].cCMD_End)
        {
            sCLI_GENERAL_FORMAT sCmdFormat = {0};

#if 0  //G100_Steven_0102
 //ZU860_John_0004 start fix general CLI and GEC CLI conflict
            if (ucComma == 3)
            {
                return 0;
            }
            ucComma = 0;
 //ZU860_John_0004 end
#endif

#if OPEN_WAP
            //ZU860_John_0043 start fix CLI conflict issue
            if(GEC_CommandProcess_Get() == TRUE) //G100_Steven_0007
            {
            	return 0;
            }
            //ZU860_John_0043 end
#endif
            sCmdFormat.ucCmdFrom = cCh; //T100_Simon_0020

            char ucReturnString[GENERAL_CMD_LENGTH_MAX] = {'\0'};

            m_sCLI_General_Info[cCh].ucCLI_DATA[m_sCLI_General_Info[cCh].uiCLI_Poll_Position] = '\0';

            utilGeneral_CLI_Process(cCh, m_sCLI_General_Info[cCh].ucCLI_DATA, ucReturnString, &sCmdFormat, CLI_EXECTUE_READ_CMD | CLI_EXECTUE_WRITE_BUFFER);

            m_sCLI_General_Info[cCh].uiCLI_Poll_Position = 0;

            return 1;
        }
    }
	#endif

    return 0;
}

// ==============================================================================
// FUNCTION NAME: utilGeneral_CLI_Init
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
void utilGeneral_CLI_Init(void)
{
    UINT8 ucCount = 0;

    for(ucCount = 0; ucCount<eccNumber; ucCount++)
    {
        memset(&m_sCLI_General_Info[ucCount], 0, sizeof(sCLI_DATA_INFO));    //G100_Simon_0060
        m_sCLI_General_Info[ucCount].ucSerialPortEcho = 0;
    }

    //GuiCb = Gui_fpCallbackGet();
}

// ==============================================================================
// FUNCTION NAME: utilGeneral_CLI_SerialPortEcho
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
void utilGeneral_CLI_SerialPortEcho(UINT8 eCh, UINT8 ucEcho)
{
	m_sCLI_General_Info[eCh].ucSerialPortEcho = ucEcho;
}

// ==============================================================================
// FUNCTION NAME: utilGeneral_CLI_Execute
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
UINT8 utilGeneral_CLI_Execute(UINT8 cCh, UINT8* pcString, char* pcReturnString, sCLI_GENERAL_FORMAT* sCmdFormat)
{
    UINT8 ucErrorCode = eCLI_ERROR_CODE_NO;

    utilOPD_CLI_EventSet((eOPD_CLI_RS232_LOG + cCh), (char*)pcString);

    //G100_Wilsonj_0050 Start
    if((sCmdFormat->ulProjectorAddressID == 0) ||
       (sCmdFormat->ulProjectorAddressID == m_ulAddressID))
    {
        if(ucErrorCode == eCLI_ERROR_CODE_NO)
        {
            ucErrorCode = utilGeneral_CMD_Handle(sCmdFormat);
        }

        LOG_MSG(db_APP_CLI, "(%s, %d) (%d)\r\n", __FUNCTION__, __LINE__, ucErrorCode);

        utilGeneral_CMD_Reply(cCh, ucErrorCode, sCmdFormat, pcReturnString);
        __CMD_Respond(cCh, pcReturnString);

    	if(ucErrorCode == eCLI_ERROR_CODE_NO)
    	{
            if((edcLENS_CALIBRATION == m_sSubLut[sCmdFormat->uiFuncID].uiFunCodeID) && (sCmdFormat->ucIsRead == WRITE_COMMAND)) //A70LV_Larry_0315
            {
                palDataMgr_UI_EventSend(edcUI_EVENT_MENUOPEN_LENS_CAL_MSG, TRUE, NULL); //GuiCb.fpGui_Send_LensCalibrationMenuOpenCb();
            }
            else if((edcLENS_APPLY_POSITION == m_sSubLut[sCmdFormat->uiFuncID].uiFunCodeID) && (sCmdFormat->ucIsRead == WRITE_COMMAND))
            {
                palDataMgr_UI_EventSend(edcUI_EVENT_MENUOPEN_LENS_MOVING_MSG, TRUE, NULL); //GuiCb.fpGui_Send_LensMovingMenuOpenCb();
            }
            else if( m_sSubLut[sCmdFormat->uiFuncID].uiMainCmdID == eCLI_MAIN_CBMP) //A35G2_BRC_Casper_0145
            {
                //no update OSD
            }
            else if(ucErrorCode == eCLI_ERROR_CODE_NO && sCmdFormat->ucIsRead == WRITE_COMMAND) //A70LV_Larry_0213
            {
                //palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL);
                //palEnvironment_AutoShutDownClear();	//A35G2_BRC_Coda_0015
                UINT8 Reset = 1;
                palDataMgr_Data_Access(edcAUTO_SHUTDOWN_TIMER_RESET, edaWRITE_THROUGH_WITH_ACTION, &Reset);

                if(m_sSubLut[sCmdFormat->uiFuncID].uiMainCmdID != eCLI_MAIN_ICPG)
                {
                    palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD_CLI, TRUE, NULL); //GuiCb.fpGui_SendUpdateOSD_CLI_EventCb(); //A35G2_BRC_Coda_0009
                }
            }
    	}
    }

    return ucErrorCode;
}



// ==============================================================================
// FUNCTION NAME: utilGeneral_CLI_Process
// DESCRIPTION:
//
//
// Params:
// eCLI_CHANNEL eCh:
// UINT8* pcString:
// sCLI_GENERAL_FORMAT* sCmdFormat:
//
// Returns:
//
//
// Modification History
// --------------------
// 2018/11/23, Larry Create
// --------------------
// ==============================================================================
UINT8 utilGeneral_CLI_Process(UINT8 cCh, UINT8* pcString, char* pcReturnString, sCLI_GENERAL_FORMAT* sCmdFormat, UINT16 uiType)
{
    UINT8 ucErrorCode = eCLI_ERROR_CODE_NO;
    LOG_MSG(db_APP_CLI, "cli string = %s\r\n", pcString);  //G100_Wilsonj_0050
    ucErrorCode = utilGeneral_CMD_Decode(pcString, sCmdFormat);
    LOG_MSG(db_APP_CLI, "cli errcode %d, addressID = %d (%d)\r\n", ucErrorCode, sCmdFormat->ulProjectorAddressID, m_ulAddressID);  //G100_Wilsonj_0050

    if(ucErrorCode == eCLI_ERROR_CODE_NO)
    {
        if(sCmdFormat->ucIsRead == WRITE_COMMAND)
        {
            if(uiType & CLI_EXECTUE_WRITE_CMD)
            {
                ucErrorCode = utilGeneral_CLI_Execute(cCh, pcString, pcReturnString, sCmdFormat);

                if(m_sSubLut[sCmdFormat->uiFuncID].uiSpecialFlag & CLI_COM_CHECK_BUSY)
                {
                    m_ucWriteBusy = 0;
                }
            }
            else if(uiType & CLI_EXECTUE_WRITE_BUFFER)
            {
                if(m_ucWriteBusy)
                {
                    ucErrorCode = eCLI_ERROR_CODE_SETFAIL;
                    utilGeneral_CMD_Reply(cCh, ucErrorCode, sCmdFormat, pcReturnString);
                    __CMD_Respond(cCh, pcReturnString);

                    return ucErrorCode;
                }

                if(m_sSubLut[sCmdFormat->uiFuncID].uiSpecialFlag & CLI_COM_CHECK_BUSY)
                {
                    m_ucWriteBusy = 1;
                }

                utilCommon_CLI_Callback(cCh, eCLI_STYLE_GENERAL, pcString, strlen((char*)pcString));//A35G2_Coda_0045
            }
        }
        else //read
        {
            if(uiType & CLI_EXECTUE_READ_CMD)
            {
                ucErrorCode = utilGeneral_CLI_Execute(cCh, pcString,pcReturnString, sCmdFormat);
            }
            else if(uiType & CLI_EXECTUE_READ_BUFFER)
            {
                utilCommon_CLI_Callback(cCh, eCLI_STYLE_GENERAL, pcString, strlen((char*)pcString));//A35G2_Coda_0045
            }
        }
    }
    else
    {
        utilGeneral_CMD_Reply(cCh, ucErrorCode, sCmdFormat, pcReturnString);
        __CMD_Respond(cCh, pcReturnString);
    }

    return ucErrorCode;
}

// ==============================================================================
// FUNCTION NAME: utilGeneral_CLI_AMX
// DESCRIPTION:
//
//
// Params:
// UINT8 ucData:
//
// Returns:
//
//
// Modification History
// --------------------
// 2019/03/15, Casper Create
// --------------------
// ==============================================================================
void utilGeneral_CLI_AMX(UINT8 ucData)
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
			    UINT8 ucModelID = palSystem_ModelIDGet();
			    UINT8 ucString[32] ={0};
			    UINT8 ucModelSwitch = eMODEL_SWITCH_DEFAULT;
			    palDataMgr_Data_Access(edcMODEL_NAME, edaREAD, ucString); //A35G2_CDS_Coda_0031
			    palDataMgr_Data_Access(edcMODEL_SWITCH_NEUTRAL, edaREAD, (UINT8*)&ucModelSwitch); //A35G2_Coda_0063
			    if(ucModelID < eMODEL_TYPE_NUMBER)
			    {
			        if(ucModelSwitch == eMODEL_SWITCH_DEFAULT)
			        {
                        LOG_MSG(db_ALWAYS, AMXB_STR, ucString);
                    }
                    else
                    {
                        LOG_MSG(db_ALWAYS, AMXB_NEUTRAL_STR, ucString);
                    }
                }
                else
                {
                    LOG_MSG(db_ALWAYS, AMXB_STR, "UNKNOWN");
                }
			}
            ucAMX_Step = eCLI_DECODE_AMX_STEP_A;
			break;

		default:
			ucAMX_Step = eCLI_DECODE_AMX_STEP_A;
			break;
	}
}

UINT8 utilGeneral_CheckBit(UINT8 cValue, UINT8 cbit ) //G100_Steven_0101 //A35G2_BRC_Casper_0047
{
	return cValue & (1<<cbit)?1:0;
}



#endif /* GENERAL_MSSC_CLI */




