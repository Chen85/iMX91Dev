#ifndef IPC_SCALER_H
#define IPC_SCALER_H

#include "core_dbus.h"

//=================================================================================================
#define NAME_IFACE_METHOD_PROJECTOR      "coretronic.method.Projector"
#define NAME_IFACE_SIGNAL_PROJECTOR      "coretronic.singal.Projector"
#define NAME_DBUS_CLI                    "coretronic.dbusname.Cli"
#define NAME_DBUS_SCALER                 "coretronic.dbusname.Scaler"
#define PATH_PROJECTOR_OBJ               "/coretronic/projector/Scaler"

#define PATH_DBUS_PROJECTOR_SERVICE_FILE "/tmp/dbus_serice_name_projector"

#define TIMEOUT_WAIT_MSEC         2    // unit:ms
#define TIMEOUT_SEND_METHOD       5000 // unit:ms
#define MAX_SEND_NUMBER           10
#define MAX_BUFFER_SIZE           (4096+512)
#define WAIT_SEND_METHOD          1    // unit:ms

//=================================================================================================
typedef void (*DbusCb)(u8_t *pTrigData, u32_t *pSize, u8_t *pData, u32_t Size);
typedef void (*DbusAckCb)(u8_t *pData, u32_t Size);
//=================================================================================================
typedef struct
{
    _sDbusInfo Info;
    DbusCb MethodSetCb;
    DbusCb MethodGetCb;
    DbusCb MethodSetAckdoneCb;
    DbusCb MethodAckdoneCb;
    DbusAckCb SignalSetCb;
    DbusAckCb SignalGetCb;
    DbusAckCb SignalErrorCb;
    DbusAckCb AckCb;
}_sDusRecvInfo;

typedef struct
{
    DbusCb MethodSetCb;
    DbusCb MethodGetCb;
    DbusCb MethodSetAckdoneCb;
    DbusCb MethodAckdoneCb;
    DbusAckCb SignalSetCb;
    DbusAckCb SignalGetCb;
    DbusAckCb SignalErrorCb;
    DbusAckCb AckCb;
}_ipcCallBack;
//=================================================================================================
u8_t Scaler_initIpc(_ipcCallBack *pCallBack);
u8_t Scaler_disconnect(void);
u8_t Scaler_sendMethodAckdoneData(void *pData, u32_t Size);
u8_t Scaler_sendMethodSetData(void *pData, u32_t Size);
u8_t Scaler_getCmd(void *pData, u32_t *pSize);
u8_t Scaler_initCb(_ipcCallBack *pInfo);
//=================================================================================================
#endif //IPC_SCALER_H
