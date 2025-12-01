#ifndef IPC_LIB_H
#define IPC_LIB_H
//=================================================================================================
#include "dbus/dbus-glib.h"
#include "dbus/dbus.h"
#include "type_def.h"
#include "hicc_config.h"
//=================================================================================================
#define MAX_IPC_NAME_STR          128
#define MAX_NUM_IPC_LISTERN_IFACE 128

typedef enum
{
    DBUS_SIGNAL = 0,
    DBUS_METHOD,

    DBUS_NUM,
}_DBusType;
//=================================================================================================
typedef struct
{
    _DBusType Type;
    char IfaceName[MAX_IPC_NAME_STR];
    char String[MAX_IPC_NAME_STR];
}_DbusIfaceData;

typedef struct
{
    u32_t NUmber;
    _DbusIfaceData *pTable;
}_DbusIfaceTable;

typedef struct
{
    u32_t Number;
    _DbusIfaceData Data[MAX_NUM_IPC_LISTERN_IFACE];
}_DbusIfaceInfo;

typedef struct
{
    u8_t Type;
    char *pWellKnowName;
    char *pIfaceName;
    char *pObjPath;
    char *pName;
    char *pDestination;
}_DbusMsgInfo;

typedef struct
{
    DBusConnection *pCon;
    char DbusName[MAX_IPC_NAME_STR];
    char ObjPath[MAX_IPC_NAME_STR];
    _DbusIfaceInfo IfaceInfo;
}_sDbusInfo;
//=================================================================================================
u8_t Ipc_addSignalMatch(_sDbusInfo *pInfo, u32_t Idx);
u8_t Ipc_checkMsg(DBusMessage *pMsg, _sDbusInfo *pInfo, u32_t IfaceIdx);
u8_t Ipc_clearIface(_sDbusInfo *pInfo, char *pIfaceName, char *pString);
u8_t Ipc_closeConnect(_sDbusInfo *pInfo);
u8_t Ipc_getIfaceIdx(u32_t *pIdx, _sDbusInfo *pInfo, char *pIfaceName, char *pString);
u8_t Ipc_getIfaceNameFromMsg(DBusMessage *pMsg, char **pIfaceName);
u8_t Ipc_getMsgInfo(DBusMessage *pMsg, _DbusMsgInfo *pInfo);
u8_t Ipc_getNameFromMsg(DBusMessage *pMsg, char **pName);
u8_t Ipc_getObjPathFromMsg(DBusMessage *pMsg, char **pObjPath);
u8_t Ipc_getRecvMsgType(DBusMessage *pMsg, u8_t *pType);
u8_t Ipc_getWellKnownNameFromMsg(DBusMessage *pMsg, char **pName);
u8_t Ipc_getDestinationFromMsg(DBusMessage *pMsg, char **pDest);
u8_t Ipc_initConnect(char *pName, _sDbusInfo *pInfo);
u8_t Ipc_readMethodData(DBusMessage *pMsg, void **pData, u32_t *pSize);
u8_t Ipc_readSignalData(DBusMessage *pMsg, char *pValType, void **pData);
u8_t Ipc_replytoMethodCall(DBusMessage *pMsg, _sDbusInfo *pInfo, void *pData, u32_t Size);
u8_t Ipc_sendMethodBlock(DBusMessage **pReplyMsg, _sDbusInfo *pInfo, void *pData, u32_t Size, u32_t IfaceIdx, char *pdestination, s32_t TimeOut);
u8_t Ipc_SendMethodData(_sDbusInfo *pInfo, void *pData, u32_t Size, u32_t IfaceIdx, char *pdestination);
u8_t Ipc_sendSignal(_sDbusInfo *pInfo, char *pData, u32_t Size, u32_t IfaceIdx);
u8_t Ipc_setIface(u32_t *pIdx, _sDbusInfo *pInfo, _DBusType Type, char *pName, char *pString);
u8_t Ipc_writeMethodData(DBusMessage *pMsg, void *pData, u32_t Size);
u8_t Ipc_waitMsg(DBusMessage **pMsg, _sDbusInfo *pInfo);
void Ipc_freeMsg(DBusMessage *pMsg);
void Ipc_initParam(_sDbusInfo *pInfo);
u8_t Ipc_initDbus(_sDbusInfo *pInfo, char *pServFile, char *pDbusName, char *pObjPath, _DbusIfaceTable *pIfaceInfo);
u8_t Ipc_SendMethodDataByDetailData(_sDbusInfo *pInfo, char *pdestination, char *pObjpath, char *pIfaceName, char *pTypeString, void *pData, u32_t Size);
#endif //IPC_LIB_H
