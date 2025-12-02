#ifndef UTILCLICMDAPI_H
#define UTILCLICMDAPI_H
// ==============================================================================
// FILE NAME: UTILCLICMDAPI.H
// DESCRIPTION:
//
//
// modification history
// --------------------
// 18/11/2013, Leo Create
// --------------------
// ==============================================================================


#include "Common.h"
#include "utilIPCAPI.h"
#include "cmd_ap.h"	//G100_Coda_0016
#include "CommandList.h"
#include "CommandSizeList.h"
#include "utilStorageCfg.h"

#define CMD_READ    1
#define CMD_WRITE   0

#define MODULE_LEN  1
#define CMD_LEN     1

#define NXP_READ_SIZE           (sizeof(sMSG_PACKET_HEADER)/sizeof(UINT8)) + 2//MAGICNUMBERSIZE
#define NXP_READ_ACK_SIZE       sizeof(sMSG_PACKET_ACK_FORMAT)/sizeof(UINT8)
#define NXP_MAX_RETRY_COUNT     5                                                         //A70_Larry_0328
#define NXP_ACK_DELAY_TIME      (300L)                                                        //A70_Larry_0328    //A70_Larry_0406 15L->5L
#define NXP_READ_SHORT_DELAY    (2L)
#define NXP_READ_LONG_DELAY     (50L)

#define NA_IPC_ACTION_TYPE      (0xFFFF)
#define UNKNOWN_CMD_ID          (0xFFFF)

#define WORD_REG    //G100_Julie_0001
#define I2C_Tx_MAX_SIZE         (1024+16)

#pragma pack(push)  /* push current alignment to stack */
#pragma pack(1)     /* set alignment to 1 byte boundary */

typedef union
{
    UINT8 ucModule;

    struct
    {
        unsigned int cRW         :1;
        unsigned int ucModuleID  :7;
    }sBIT;
}uMODULE_ID;


// Payload
typedef struct
{
    uMODULE_ID   uModule;
    UINT8        ucSubCmd;
    UINT8        aucData[1024*4]; //A70LV_Larry_0116
}sPAYLOAD;

//#define LAN_FORMAT_DATA_LEN    4 //ucType + ucEnable + uiDataCode

typedef union
{
    INT32   lDataValue;
    char    acBuffer[MAX_SIZE_DATABASE_DATA];  //H2PF_Simon_0067
}uDATA_FORM;

typedef struct
{
    UINT16      uiSendFrom;  //_eInterface
    UINT8       ucType;
    UINT8       ucEnable;
    UINT16      uiDataCode;
    UINT16      uiDataSize;
    uDATA_FORM  uData;
}sLAN_DATA_FORMAT; //A70LV_Larry_0066

typedef enum
{
    eACK_TYPE_NONACK,
    eACK_TYPE_ACK,
    eACK_TYPE_BADPACKET,
    eACK_TYPE_TIMEOUT,
    eACK_TYPE_UNKNOWN,
    eACK_TYPE_FEEDBACK,
    eACK_TYPE_ERROR,

    eACK_TYPE_NUMBERS,
} eACK_TYPE;

// Cmd table 形式
typedef struct
{
    UINT16 wCmdIndex;
    eRESULT(*pFunc)(sPAYLOAD *, sPAYLOAD *, UINT16 *);
} sCMDCFG;

typedef enum
{
    eSYSTEM_STATE_IDEL,
    eSYSTEM_STATE_STANDBY,
    eSYSTEM_STATE_COMMUNICATION,
    eSYSTEM_STATE_WARMUP,
    eSYSTEM_STATE_POWERON,
    eSYSTEM_STATE_COOLING,
    eSYSTEM_STATE_POWERDOWN,
    eSYSTEM_STATE_UPGRADE,
    eSYSTEM_STATE_UPGRADE_ERROR,
    eSYSTEM_STATE_UPGRADE_DONE,
    eSYSTEM_STATE_UPGRADE_DDP,

    eSYSTEM_STATE_NUMBER,
}eSYSTEM_STATE; //只能往後新增，不能插入，不然更新可能失敗


#define FAN_CMD_MAX_LEN 32
#define FAN_CMD_MAX_UINT16_LEN 16
#define LED_CMD_MAX_LEN 8

typedef union
{
    UINT8    acData[FAN_CMD_MAX_LEN];
    UINT16   awData[FAN_CMD_MAX_UINT16_LEN] ;
} uFANPAYLOAD;


typedef struct //A70_Larry_0229 fixed
{
    UINT8 		cBacklight;
    UINT8        cTimeOut;
    UINT8        cCustomID;
    UINT8        acData[LED_CMD_MAX_LEN];
} sLEDPAYLOAD;

typedef struct
{
    UINT8       ucType;
    UINT8       ucEnable;
    UINT16      uiDataCode;
    _sStructPayload  sStructData;
}sLAN_STRUCT_DATA_FORMAT; 	//G100_Coda_0016

#pragma pack(pop)   /* restore original alignment from stack */


////////  callback function start  ////////
#ifdef PALDATAMGR_ACCESS_WITHLOG
typedef eEXEC_CODE (*fpDataMgr_Data_Access)(eDATA_CODE DataCode, eDATA_ACCESS_MODE eAccessMode, void *pValue, const char *cFuncName, UINT32 ulLineNum);
#else
typedef eEXEC_CODE (*fpDataMgr_Data_Access)(eDATA_CODE DataCode, eDATA_ACCESS_MODE eAccessMode, void *pValue);
#endif
typedef eRESULT (*fpInputProc_BufferInsert)(void *pvKey);
typedef ePOWER_STATE (*fpSystem_PowerStateGet)(void);
typedef UINT8 (*fpSystem_HostReadyGet)(void);
typedef void (*fpSystem_HostReadySet)(UINT8 ucValue);
typedef void (*fpSystem_WarmUp)(void);
typedef void (*fpSystem_PowerDown)(void);
typedef void (*fpSystem_Enter2WMode)(void);
typedef UINT8 (*fpSystem_ModelIDGet)(void);
typedef void (*fpDataMgr_ResetAllToDefault)(void);
typedef eEXEC_CODE (*fpEnvironment_InstantCooling)(void);
typedef void (*fpEnvironment_Fake_Power_Down_Set)(BOOL bFakePD);
typedef BOOL (*fpEnvironment_Fake_Power_Down_Get)(void);

typedef void (*fpGui_SendKeyEvent)(UINT8 ucType, eKEY_LIST eKey);
typedef UINT8 (*fpDataMgr_Data_Type)(eDATA_CODE eDataCode);
typedef eFUNC_CONTROL (*fpDataMgr_DataCode_Control)(eDATA_CODE eDataCode);
typedef UINT8 (*fpGui_PIN_Protect_Checking)(void);
typedef UINT8 (*fpEnvironment_NetworkIsReceivePowerOnCmd_Get)(void);
typedef char* (*fpDataMgr_DataCodeStringGet)(eDATA_CODE eDataCode);
typedef eEXEC_CODE (*fpDataMgr_Data_Range_Get)(eDATA_CODE eDataCode, eDATA_RANGE_MODE eRange, tDATA_CODE *pValue);
typedef INT8 (*fpDataMgr_IsNetworkDataCode)(eDATA_CODE eDataCode);
//typedef utilDataM(*fping_CMValueRangeCheck;
typedef eEXEC_CODE (*fpGui_DataCode_Value_Set)(UINT16 uiDataCode, UINT8 ucAccessMode, tDATA_CODE iValue);
typedef eEXEC_CODE (*fpGui_DataCode_Value_Get)(UINT16 uiDataCode, tDATA_CODE *piValue);
typedef void (*fpDataMgr_OPDEvent)(UINT8 ucEvent, eDATA_CODE eDataCode);
typedef eEXEC_CODE (*fpGui_DataCode_String_Set)(UINT16 uiDataCode, char *cDynamicString);
typedef void (*fpDataMgr_LanArgCopy2)(void);
typedef void (*fpDataMgr_LanArg_Send_Mcu)(UINT16 uiDataCode);
typedef void (*fpDataMgr_WLanArgCopy2)(void);
typedef void (*fpDataMgr_CrestronArgCopy2)(void);
typedef void (*fpDataMgr_PJLinkArgCopy2)(void);
typedef void (*fpGui_SendUpdateOSDEvent)(void);
typedef void (*fpGui_Send_LensCalibrationMenuOpen)(void);
typedef void (*fpGui_Send_LensMovingMenuOpen)(void);
typedef BOOL (*fpGui_Get_MenuState_IsOSD_Open)(void);
typedef void (*fpGui_Send_OSD_Exit)(void);
typedef void (*fpLANProcUpdateOSD_Set)(BOOL bFlag);
typedef void (*fpLANProcSendToLAN)(UINT16 uiDataCode);
typedef void (*fpEnvironment_AutoShutDownClear)(void);
typedef void (*fpLANProcSendAckDone)(UINT16 uiDataCode, _eInterface eSendFrom, eEXEC_CODE ucExecResult);  //H2PF_Simon_0067
typedef void (*fpDataMgr_LAN_IP_Copy2_All)(void);
typedef void (*fpDataMgr_DateTime_Unpack_LanPacket)(_sDateTime * sLanDateTimePacket);
typedef void (*fpDataMgr_Schedule_Unpack_LanPacket)(sLAN_SCHEDULE_PACKET * sLanSchudelePacket) ;
typedef eEXEC_CODE (*fpDataMgr_ACU_Target_Select_Set)(UINT8 ucValue);
typedef void (*fpDataPath_BackupConfigFlag)(void);
typedef void (*fpDataMgr_XFPGA_LVDS_COVER_Set)(UINT8 ucLVDS_COVER_En);
typedef void (*fpDataPath_InputDetectSet)(UINT16 uiData);
typedef void (*fpDataMgr_OPDInputPlugInState)(UINT8 ucEvent);
typedef void (*fpDataPath_LastInputDetectSet)(UINT16 uiData);
typedef INT32 (*fpDataMapping_CMValueRangeCheck)(eDATA_CODE eDataCode, INT32 lCM_Value);
typedef void (*fpGui_SendPinProtectClearEvent)(void);
typedef void (*fpGui_SendDataCodeAndUpdateEvent)(UINT16 uiDataCode);
typedef void (*fpGui_Send_ABP_Cal_Msg_Open)(void);
typedef void (*fpDataMgr_IPV6LanArgCopy2)(void);// HICC2_Bruce_0020
typedef void (*fpDataMgr_IPV6LanArg_Send_Mcu)(UINT16 uiDataCode);


#pragma pack(push)  /* push current alignment to stack */
#pragma pack(1)     /* set alignment to 1 byte boundary */

typedef struct
{
    fpDataMgr_Data_Access fpDataMgr_Data_AccessCb;
    fpInputProc_BufferInsert fpInputProc_BufferInsertCb;
    fpSystem_PowerStateGet fpSystem_PowerStateGetCb;
    fpSystem_HostReadyGet fpSystem_HostReadyGetCb;
    fpSystem_HostReadySet fpSystem_HostReadySetCb;
    fpSystem_WarmUp fpSystem_WarmUpCb;
    fpSystem_PowerDown fpSystem_PowerDownCb;
    fpSystem_Enter2WMode fpSystem_Enter2WModeCb;
    fpSystem_ModelIDGet fpSystem_ModelIDGetCb;

    fpDataMgr_ResetAllToDefault fpDataMgr_ResetAllToDefaultCb;

    fpEnvironment_InstantCooling fpEnvironment_InstantCoolingCb;
    fpEnvironment_Fake_Power_Down_Set fpEnvironment_Fake_Power_Down_SetCb;
    fpEnvironment_Fake_Power_Down_Get fpEnvironment_Fake_Power_Down_GetCb;

    fpGui_SendKeyEvent fpGui_SendKeyEventCb;
    fpDataMgr_Data_Type fpDataMgr_Data_TypeCb;
    fpDataMgr_DataCode_Control fpDataMgr_DataCode_ControlCb;
    fpGui_PIN_Protect_Checking fpGui_PIN_Protect_CheckingCb;
    fpEnvironment_NetworkIsReceivePowerOnCmd_Get fpEnvironment_NetworkIsReceivePowerOnCmd_GetCb;
    fpDataMgr_DataCodeStringGet fpDataMgr_DataCodeStringGetCb;
    fpDataMgr_Data_Range_Get fpDataMgr_Data_Range_GetCb;
    fpDataMgr_IsNetworkDataCode fpDataMgr_IsNetworkDataCodeCb;
    fpGui_DataCode_Value_Set fpGui_DataCode_Value_SetCb;
    fpGui_DataCode_Value_Get fpGui_DataCode_Value_GetCb;
    fpDataMgr_OPDEvent fpDataMgr_OPDEventCb;
    fpGui_DataCode_String_Set fpGui_DataCode_String_SetCb;
    fpDataMgr_LanArgCopy2 fpDataMgr_LanArgCopy2Cb;
    fpDataMgr_LanArg_Send_Mcu fpDataMgr_LanArg_Send_McuCb;
    fpDataMgr_WLanArgCopy2 fpDataMgr_WLanArgCopy2Cb;
    fpDataMgr_CrestronArgCopy2 fpDataMgr_CrestronArgCopy2Cb;
    fpDataMgr_PJLinkArgCopy2 fpDataMgr_PJLinkArgCopy2Cb;
    fpGui_SendUpdateOSDEvent fpGui_SendUpdateOSDEventCb;
    fpGui_Send_LensCalibrationMenuOpen fpGui_Send_LensCalibrationMenuOpenCb;
    fpGui_Send_LensMovingMenuOpen fpGui_Send_LensMovingMenuOpenCb;
    fpGui_Get_MenuState_IsOSD_Open fpGui_Get_MenuState_IsOSD_OpenCb;
    fpGui_Send_OSD_Exit fpGui_Send_OSD_ExitCb;
    fpLANProcUpdateOSD_Set fpLANProcUpdateOSD_SetCb;
    fpLANProcSendToLAN fpLANProcSendToLANCb;
    fpEnvironment_AutoShutDownClear fpEnvironment_AutoShutDownClearCb;
    fpLANProcSendAckDone fpLANProcSendAckDoneCb;
    fpDataMgr_LAN_IP_Copy2_All fpDataMgr_LAN_IP_Copy2_AllCb;
    fpDataMgr_DateTime_Unpack_LanPacket fpDataMgr_DateTime_Unpack_LanPacketCb;
    fpDataMgr_Schedule_Unpack_LanPacket fpDataMgr_Schedule_Unpack_LanPacketCb;
    fpDataMgr_ACU_Target_Select_Set fpDataMgr_ACU_Target_Select_SetCb;
    fpDataPath_BackupConfigFlag fpDataPath_BackupConfigFlagCb;
    fpDataMgr_XFPGA_LVDS_COVER_Set fpDataMgr_XFPGA_LVDS_COVER_SetCb;
    fpDataPath_InputDetectSet fpDataPath_InputDetectSetCb;
    fpDataMgr_OPDInputPlugInState fpDataMgr_OPDInputPlugInStateCb;
    fpDataPath_LastInputDetectSet fpDataPath_LastInputDetectSetCb;
    fpDataMapping_CMValueRangeCheck fpDataMapping_CMValueRangeCheckCb;
    fpGui_SendPinProtectClearEvent fpGui_SendPinProtectClearEventCb;
    fpGui_SendDataCodeAndUpdateEvent fpGui_SendDataCodeAndUpdateEventCb;
    fpGui_Send_ABP_Cal_Msg_Open fpGui_Send_ABP_Cal_Msg_OpenCb;
    fpDataMgr_IPV6LanArgCopy2 fpDataMgr_IPV6LanArgCopy2Cb;// HICC2_Bruce_0020
    fpDataMgr_IPV6LanArg_Send_Mcu fpDataMgr_IPV6LanArg_Send_McuCb;

}sUTILCLICMD_CALLBACK;

#pragma pack(pop)   /* restore original alignment from stack */

void utilCLICmd_RegCallback(sUTILCLICMD_CALLBACK fpCallback);
/////////  callback function end  /////////


eACK_TYPE utilCLICmd_Execute(sPAYLOAD *psInData, sPAYLOAD *psOutData, UINT16 *pwByteCount);
eRESULT CheckSumCal_unpacket(BYTE *pcBuffer, WORD wByteCount);
UINT16 CheckSumCal_Packet(uint8 *psOutData, uint8 *psInData, UINT16 wSize);

void DrawPNG_Test(void);

#endif /* UTILCLICMDAPI_H */

