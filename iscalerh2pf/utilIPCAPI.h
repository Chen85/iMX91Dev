#ifndef _UTILIPCAPI_H_
#define _UTILIPCAPI_H_

#include "Common.h"
#include "utilCommon.h"
#include "SharedMemCtrl.h"
#include "cmd_ap.h"

//#define TIMEOUT_WAIT_MSEC         2    // unit:ms
//#define TIMEOUT_SEND_METHOD       1000 // unit:ms
//#define MAX_SEND_NUMBER           100//10
//#define MAX_BUFFER_SIZE           (128+512)//(4096+512)
//#define WAIT_SEND_METHOD          1    // unit:ms


////////  callback function start  ////////
typedef eCAMERA_STATUS (*fpDataMgr_Get_Camera_Working_Status)(void);
typedef void (*fpEnvironment_Camera_Force_Update_Power_Status_Set)(void);
typedef void (*fpSystem_PowerDownDebug)(void);
typedef void (*fpSystem_PowerKey)(void);
typedef void (*fpSystem_FwUpgradeMode)(UINT8 ucSelect);
typedef eRESULT (*fpInputProc_InputKeyListID)(eKEY_LIST eKeyListID);
typedef UINT8 (*fpDataMgr_Data_Type)(eDATA_CODE eDataCode);
typedef eFUNC_CONTROL (*fpDataMgr_DataCode_Control)(eDATA_CODE eDataCode);
typedef UINT16 (*fpDataMgr_DataSizeGet)(eDATA_CODE eDataCode);

typedef struct
{
    fpDataMgr_Get_Camera_Working_Status fpDataMgr_Get_Camera_Working_StatusCb;
    fpEnvironment_Camera_Force_Update_Power_Status_Set fpEnvironment_Camera_Force_Update_Power_Status_SetCb;
    fpSystem_PowerDownDebug fpSystem_PowerDownCb;
    fpSystem_PowerKey fpSystem_PowerKeyCb;
    fpSystem_FwUpgradeMode fpSystem_FwUpgradeModeCb;
    fpInputProc_InputKeyListID fpInputProc_InputKeyListIDCb;
    fpDataMgr_Data_Type fpDataMgr_Data_TypeCb;
    fpDataMgr_DataCode_Control fpDataMgr_DataCode_ControlCb;
    fpDataMgr_DataSizeGet fpDataMgr_DataSizeGetCb;
}sUTILIPC_CALLBACK;

void utilIPC_RegCallback(sUTILIPC_CALLBACK fpCallback);
/////////  callback function end  /////////



#pragma pack(push)  /* push current alignment to stack */
#pragma pack(1)     /* set alignment to 1 byte boundary */

#if 0
typedef struct
{
    UINT8   ucHeader;
    UINT16  ucActionType;
    UINT16  uiSize;
    UINT8   ucType;
    UINT8   ucStatus;
    UINT16  uiDataCode;
    union
    {
        UINT8   ucData[2048];
        UINT16  uiData[1024];
        UINT32  ulData[512];
    };
}sIPC_SEND_DATA;
#endif

typedef _sSubPacketFormat sIPC_SEND_DATA;

#pragma pack(pop)   /* restore original alignment from stack */

#define IPC_SEND_DATA_LEN (11) //1(header) + 2(Action Type) + 2(size) + 1(type) + 1(enable) + 2(datacode) + 2(datasize)



typedef enum
{
    eIPC_SEND_DATA_POWER_STATE,
    eIPC_SEND_DATA_SEND_DATACODE_WITH_DATA,
    eIPC_SEND_DATA_TELNET_REPLY,
    eIPC_SEND_DATA_SNMP,
    eIPC_SEND_DATA_ACK_DONE,
    eIPC_SEND_DATA_LAN_APPLY,
    eIPC_SEND_DATA_UPDATE_DATACODE_ITEM,
    eIPC_SEND_DATA_UPDATE_INFO_ITEM,
    eIPC_SEND_DATA_LAN_RESET,
    eIPC_SEND_DATA_STRUCT,	//G100_Coda_0016
    eIPC_SEND_DATA_UI_EVENT,

}eIPC_SEND_DATA;


void utilIpc_BuildSendData(sIPC_SEND_DATA *psPacket, UINT16 uiDataCode, UINT8 ucStatus, UINT8 ucType , UINT16 uiSize, UINT8 *pcData);
void utilIpc_BuildSendDataAck(sIPC_SEND_DATA *psPacket, UINT16 uiDataCode, UINT8 ucStatus, UINT8 ucType , UINT16 uiSize, UINT8 *pcData);
void utilIpc_BuildTelnetReply(sIPC_SEND_DATA *psPacket , UINT16 uiSize, UINT8 *pcData);
void utilIpc_BuildSNMP(sIPC_SEND_DATA *psPacket , UINT16 uiSize, UINT8 *pcData);
void utilIpc_BuildUpdateDataCodeItem(sIPC_SEND_DATA *psPacket , UINT16 uiSize, UINT8 *pcData);
void utilIpc_BuildUpdateInfoItem(sIPC_SEND_DATA *psPacket , UINT16 uiSize, UINT8 *pcData);
void utilIpc_BuildPowerState(sIPC_SEND_DATA *psPacket, UINT16 uiDataCode, UINT16 uiSize, UINT8 *pcData);
void utilIpc_BuildStruct(sIPC_SEND_DATA *psPacket , UINT16 uiStructType, UINT16 uiSize, UINT8 *pcData);	//G100_Coda_0016
eEXEC_CODE utilIpc_Init(void);
eEXEC_CODE utilIpcProcSend(void *SendData, UINT16 uiSize, UINT16 uiNumber);
eEXEC_CODE utilIpc_Send(void *SendData, UINT16 uiSize, UINT16 uiNumber);
eEXEC_CODE utilIpc_SendAckdone(void *SendData, UINT16 uiSize, UINT16 uiNumber, UINT8 ucExecResult);
eEXEC_CODE utilIpc_SendUpdateItem(void *SendData, UINT16 uiSize, UINT16 uiNumber);
INT8 utilIpc_SendData(eIPC_SEND_DATA eIPCData, UINT16 uiCmdID, void *vData, UINT8 ExecResult, UINT16 uiSendDataSize);
eRESULT utilIpc_NotifyStructureDatatypeDatacode(eDATA_CODE DataCode);
eEXEC_CODE utilIpc_NotifyDatacodeChanged(eDATA_CODE DataCode);


#endif //_UTILIPCAPI_H_

