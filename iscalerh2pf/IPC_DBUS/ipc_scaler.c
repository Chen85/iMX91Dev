#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "utilCommon.h"
#include "core_dbus.h"
#include "timeout.h"
#include "ipc_scaler.h"
//=================================================================================================
typedef enum
{
    METHOD_DATA_STATUS_IDLE  = 0,
    METHOD_DATA_STATUS_SEND,
    METHOD_DATA_STATUS_ACK,
    METHOD_DATA_STATUS_ACKDONE
}_METHOD_DATA_STATUS;

//=================================================================================================
typedef struct
{
    char IfaceName[MAX_IPC_NAME_STR];
    char Name[MAX_IPC_NAME_STR];
    void (*MsgMethodProc)(_sDbusInfo *pDbusInfo, _DbusMsgInfo *pMsgInfo, DBusMessage *pMsg, DbusCb Cb);
}_MsgMethodProcTab;

typedef struct
{
    char IfaceName[MAX_IPC_NAME_STR];
    char Name[MAX_IPC_NAME_STR];
    void (*MsgSignalProc)(_sDbusInfo *pDbusInfo, _DbusMsgInfo *pMsgInfo, DBusMessage *pMsg, DbusAckCb Cb);
}_MsgSignalProcTab;

typedef struct
{
    _METHOD_DATA_STATUS Status;
    u8_t  Data[MAX_BUFFER_SIZE];
}_MethodAckInfo;

typedef struct
{
    u32_t DataType;
    char Name[MAX_IPC_NAME_STR];
    char TypeStr[MAX_IPC_NAME_STR];
}_IfaceData;

typedef struct
{
    u32_t NUmber;
    _IfaceData *pTable;
}_IfaceInfo;

typedef struct
{
    u32_t Size;
    u8_t Buf[MAX_BUFFER_SIZE];
}_CmdData;

typedef struct
{
    u32_t InIdx;
    u32_t OutIdx;
    u32_t Number;
    _CmdData CmdData[MAX_SEND_NUMBER];
}_CmdInfo;

//=================================================================================================
_MethodAckInfo g_MethodAckInfo = {0};
_CmdInfo g_CmdInfo;
_sDusRecvInfo g_RecvInfo;
pthread_t g_thread_recv;
//-------------------------------------------------------------------------------------------------
void Scaler_setThreadAttr(pthread_attr_t *pAttr, int Param)
{
    pthread_attr_init(pAttr); //初始化執行緒屬性
    pthread_attr_setdetachstate(pAttr, Param);
}
//-------------------------------------------------------------------------------------------------
void Scaler_initCmdInfo(_CmdInfo *pInfo)
{
    u32_t Idx;

    if(pInfo ==  NULL)
    {
        printf("init cmd info fail when Scaler_initCmdInfo \n");
        return;
    }

    pInfo->Number = 0;
    pInfo->InIdx = pInfo->OutIdx = 0;

    for(Idx=0; Idx<MAX_SEND_NUMBER; Idx++)
    {
        pInfo->CmdData[Idx].Size = 0;
        memset(pInfo->CmdData[Idx].Buf, 0, sizeof(pInfo->CmdData[Idx].Buf));
    }
}
//-------------------------------------------------------------------------------------------------
u8_t Scaler_saveCmd(void *pData, u32_t *pSize)
{
    _CmdData *pCmdData;

   if((pData == NULL) || (pSize == NULL))
   {
       printf("Input param error when Scaler_saveCmd\n");
       return 1;
   }

   if(*pSize == 0)
   {
       printf("No data to save when Scaler_saveCmd \n");
       return 2;
   }

   if(g_CmdInfo.Number >= MAX_SEND_NUMBER)
   {
       printf("No space to save when Scaler_saveCmd \n");
       return 3;
   }

   pCmdData = &(g_CmdInfo.CmdData[g_CmdInfo.InIdx]);

   if(*pSize > sizeof(pCmdData->Buf))
   {
       printf("Save data size over when Scaler_saveCmd \n");
       *pSize = sizeof(pCmdData->Buf);
   }

   memcpy(pCmdData->Buf, pData, *pSize);
   pCmdData->Size = *pSize;
   g_CmdInfo.InIdx = (g_CmdInfo.InIdx + 1) % MAX_SEND_NUMBER;
   g_CmdInfo.Number++;
   return 0;
}
//-------------------------------------------------------------------------------------------------
u8_t Scaler_getCmd(void *pData, u32_t *pSize)
{
    _CmdData *pCmdData;

    if((pData == NULL) || (pSize == NULL))
    {
        printf("Input param error when Scaler_getCmd \n");
        return 1;
    }

    if(*pSize == 0)
    {
        printf("No space to get data when Scaler_getCmd \n");
        return 2;
    }

    if(g_CmdInfo.Number == 0)
    {
        *pSize = 0;
        return 0;
    }

    pCmdData = &(g_CmdInfo.CmdData[g_CmdInfo.OutIdx]);

    if(*pSize > pCmdData->Size)
        *pSize = pCmdData->Size;

    memcpy(pData, pCmdData->Buf, *pSize);
    g_CmdInfo.OutIdx = (g_CmdInfo.OutIdx + 1) % MAX_SEND_NUMBER;
    g_CmdInfo.Number --;

    return 0;
}
//----------------------------------------------------------------------------------------------
void Scaler_procMethodSetData(_sDbusInfo *pDbusInfo, _DbusMsgInfo *pMsgInfo, DBusMessage *pMsg, DbusCb CbFun)
{
    u8_t *pData;
    u32_t RecvSize;
    u8_t Buffer[4096];
    u32_t BufSize;

    if((pDbusInfo == NULL) || (pMsgInfo == NULL) || (pMsg == NULL))
    {
        printf("Input param error when Scaler_procMethodSetData \n");
        return;
    }

    Ipc_readMethodData(pMsg, (void **)&pData, &RecvSize);
    // process function
    if(CbFun != NULL)
    {
        BufSize = sizeof(Buffer);
        CbFun(Buffer, &BufSize, pData, RecvSize);
        Ipc_replytoMethodCall(pMsg, pDbusInfo, Buffer, BufSize);
    }
    else
        Ipc_replytoMethodCall(pMsg, pDbusInfo, pData, RecvSize);
}
//-------------------------------------------------------------------------------------------------
void Scaler_procMethodGetData(_sDbusInfo *pDbusInfo, _DbusMsgInfo *pMsgInfo, DBusMessage *pMsg, DbusCb CbFun)
{
    u8_t *pData;
    u32_t Size;
    u8_t Buffer[4096];
    u32_t BufSize;

    if((pDbusInfo == NULL) || (pMsgInfo == NULL) || (pMsg == NULL))
    {
        printf("Input param error when Scaler_procMethodGetData \n");
        return;
    }
    Ipc_readMethodData(pMsg, (void **)&pData, &Size);
    if(CbFun != NULL)
    {
        BufSize = sizeof(Buffer);
        CbFun(Buffer, &BufSize, pData, Size);
        Ipc_replytoMethodCall(pMsg, pDbusInfo, Buffer, BufSize);
    }
    else
        Ipc_replytoMethodCall(pMsg, pDbusInfo, pData, Size);
    // process function
 //   printf("Scaler receive method get data when Scaler_procMethodGetData\n");

}
//-------------------------------------------------------------------------------------------------
void Scaler_procMethodAckDoneData(_sDbusInfo *pDbusInfo, _DbusMsgInfo *pMsgInfo, DBusMessage *pMsg, DbusCb CbFun)
{
    u8_t *pData;
    u32_t Size;
    u8_t Buffer[4096];
    u32_t BufSize;

    if((pDbusInfo == NULL) || (pMsgInfo == NULL) || (pMsg == NULL))
    {
        //DEBUGP("Input param error when Scaler_procMethodAckDoneData \n");
        return;
    }
    Ipc_readMethodData(pMsg, (void **)&pData, &Size);
    // process function
    if(CbFun != NULL)
    {
        BufSize = sizeof(Buffer);
        CbFun(Buffer, &BufSize, pData, Size);
        Ipc_replytoMethodCall(pMsg, pDbusInfo, Buffer, BufSize);
    }
    else
        Ipc_replytoMethodCall(pMsg, pDbusInfo, pData, Size);
   // printf("Scaler receive method ackdone data when Scaler_procMethodAckDoneData \n");
}
//-------------------------------------------------------------------------------------------------
void Scaler_procMethodSetAckDoneData(_sDbusInfo *pDbusInfo, _DbusMsgInfo *pMsgInfo, DBusMessage *pMsg, DbusCb CbFun)
{
    u8_t *pData;
    u32_t Size;
    u32_t RecvSizeTmp;
    u8_t Buffer[4096];
    u32_t BufSize;

    if((pDbusInfo == NULL) || (pMsgInfo == NULL) || (pMsg == NULL))
    {
        //DEBUGP("Input param error when Scaler_procMethodAckDoneData \n");
        return;
    }
    Ipc_readMethodData(pMsg, (void **)&pData, &Size);
    RecvSizeTmp = Size;
    //Scaler_saveCmd(pData, &RecvSizeTmp);  //A35G2_CDS_Coda_0027
    if(CbFun != NULL)
    {
        BufSize = sizeof(Buffer);
        CbFun(Buffer, &BufSize, pData, Size);
        Ipc_replytoMethodCall(pMsg, pDbusInfo, Buffer, BufSize);
    }
    else
        Ipc_replytoMethodCall(pMsg, pDbusInfo, pData, Size);
    // process function
   // printf("Scaler receive method ackdone data when Scaler_procMethodAckDoneData \n");

}
//-------------------------------------------------------------------------------------------------
void Scaler_procSignalSetData(_sDbusInfo *pDbusInfo, _DbusMsgInfo *pMsgInfo, DBusMessage *pMsg, DbusAckCb CbFun)
{
    u8_t *pData;
    u32_t Size;

    if((pDbusInfo == NULL) || (pMsgInfo == NULL) || (pMsg == NULL))
    {
        //DEBUGP("Input param error when Scaler_procSignalSetData \n");
        return;
    }
    Ipc_readMethodData(pMsg, (void **)&pData, &Size);
    if(CbFun == NULL)
        return;
    CbFun(pData, Size);
  //  printf("Scaler receive signal set data when Scaler_procSignalSetData \n");
}
//-------------------------------------------------------------------------------------------------
void Scaler_procSignalGetData(_sDbusInfo *pDbusInfo, _DbusMsgInfo *pMsgInfo, DBusMessage *pMsg, DbusAckCb CbFun)
{
    u8_t *pData;
    u32_t Size;

    if((pDbusInfo == NULL) || (pMsgInfo == NULL) || (pMsg == NULL))
    {
        //DEBUGP("Input param error when Scaler_procSignalGetData \n");
        return;
    }
    Ipc_readMethodData(pMsg, (void **)&pData, &Size);
    if(CbFun == NULL)
        return;
    CbFun(pData, Size);
    // printf("Scaler receive signal get data when Scaler_procSignalGetData \n");
}
//-------------------------------------------------------------------------------------------------
void Scaler_procSignalAckDoneData(_sDbusInfo *pDbusInfo, _DbusMsgInfo *pMsgInfo, DBusMessage *pMsg, DbusAckCb CbFun)
{
    u8_t *pData;
    u32_t Size;

    if((pDbusInfo == NULL) || (pMsgInfo == NULL) || (pMsg == NULL))
    {
        //DEBUGP("Input param error when Scaler_procSignalAckDoneData \n");
        return;
    }
    Ipc_readMethodData(pMsg, (void **)&pData, &Size);
    if(CbFun == NULL)
        return;
    CbFun(pData, Size);

  //  printf("Scaler receive signal ackdone data when Scaler_procSignalAckDoneData \n");
}
//-------------------------------------------------------------------------------------------------
void Scaler_procSignalErrorData(_sDbusInfo *pDbusInfo, _DbusMsgInfo *pMsgInfo, DBusMessage *pMsg, DbusAckCb CbFun)
{
    u8_t *pData;
    u32_t Size;

    if((pDbusInfo == NULL) || (pMsgInfo == NULL) || (pMsg == NULL))
    {
        //DEBUGP("Input param error when Scaler_procSignalErrorData \n");
        return;
    }
    Ipc_readMethodData(pMsg, (void **)&pData, &Size);
    if(CbFun == NULL)
        return;
    CbFun(pData, Size);
  //  printf("Scaler receive signal error data when Scaler_procSignalErrorData \n");
}
//-------------------------------------------------------------------------------------------------
void Scaler_procMethodReturn(_sDbusInfo *pDbusInfo, _DbusMsgInfo *pMsgInfo, DBusMessage *pMsg, DbusAckCb CbFun)
{
    u8_t *pData;
    u32_t Size;

    if((pDbusInfo == NULL) || (pMsgInfo == NULL) || (pMsg == NULL))
    {
        printf("Input param error when Scaler_procMethodReturn \n");
        return;
    }

    Ipc_readMethodData(pMsg, (void **)&pData, &Size);
    if(Size > sizeof(g_MethodAckInfo.Data))
        Size = sizeof(g_MethodAckInfo.Data);
    memcpy(g_MethodAckInfo.Data, pData, Size);
    if(g_MethodAckInfo.Status != METHOD_DATA_STATUS_SEND)
        printf("Scaler get send flag error:%d when Scaler_procMethodReturn \n", g_MethodAckInfo.Status);

    g_MethodAckInfo.Status = METHOD_DATA_STATUS_ACK;
    if(CbFun == NULL)
        return;
    CbFun(pData, Size);
}
//-------------------------------------------------------------------------------------------------
_MsgMethodProcTab g_MsgMethodTable[] =
{
    {NAME_IFACE_METHOD_PROJECTOR, "set",        Scaler_procMethodSetData},
    {NAME_IFACE_METHOD_PROJECTOR, "get",        Scaler_procMethodGetData},
    {NAME_IFACE_METHOD_PROJECTOR, "setackdone", Scaler_procMethodSetAckDoneData},
    {NAME_IFACE_METHOD_PROJECTOR, "ackdone",    Scaler_procMethodAckDoneData},
};

_MsgSignalProcTab g_MsgSignalTable[] =
{
    {NAME_IFACE_SIGNAL_PROJECTOR , "set",     Scaler_procSignalSetData},
    {NAME_IFACE_SIGNAL_PROJECTOR , "get",     Scaler_procSignalGetData},
    {NAME_IFACE_SIGNAL_PROJECTOR , "error",   Scaler_procSignalErrorData},
};
//-------------------------------------------------------------------------------------------------
DbusCb Scaler_getRecvCb(_sDusRecvInfo *pInfo, char *pIfaceName, _DBusType Type)
{
    if((pInfo == NULL) || (pIfaceName == NULL) || (strlen(pIfaceName) == 0) || (Type >= DBUS_NUM))
    {
        printf("Input param error when App_getRecvCb\n");
        return NULL;
    }

    if(Type != DBUS_METHOD)
    {
        printf("Error type:%d  when App_getRecvCb \n", Type);
        return NULL;
    }

    if(strcmp(pIfaceName, "set") == 0)
        return pInfo->MethodSetCb;
    else if(strcmp(pIfaceName, "get") == 0)
        return pInfo->MethodGetCb;
    else if(strcmp(pIfaceName, "setackdone") == 0)
        return pInfo->MethodSetAckdoneCb;
    else if(strcmp(pIfaceName, "ackdone") == 0)
        return pInfo->MethodAckdoneCb;
    else
    {
        printf("Unknow method iface naem when Scaler_getRecvCb \n");
        return NULL;
    }
}
//-------------------------------------------------------------------------------------------------
DbusAckCb Scaler_getRecvSingalCb(_sDusRecvInfo *pInfo, char *pIfaceName)
{
    if((pInfo == NULL) || (pIfaceName == NULL) || (strlen(pIfaceName) == 0))
    {
        printf("Input param error when App_getRecvSingalCb\n");
        return NULL;
    }

    if(strcmp(pIfaceName, "set") == 0)
        return pInfo->SignalSetCb;
    else if(strcmp(pIfaceName, "get") == 0)
        return pInfo->SignalGetCb;
    else if(strcmp(pIfaceName, "error") == 0)
        return pInfo->SignalErrorCb;
    else
   {
        printf("Unknow signal iface name when Scaler_getRecvSingalCb \n");
        return NULL;
   }
}
//-------------------------------------------------------------------------------------------------
void *Scaler_recvData(void *pData)
{
    _sDbusInfo *pInfo;
    DBusMessage *pMsg;
    _DbusMsgInfo MsgInfo;
    u32_t Idx;
    _sDusRecvInfo *pRecvInfo;

    if(pData == NULL)
        return NULL;

    pRecvInfo = (_sDusRecvInfo *)pData;
    pInfo = &(pRecvInfo->Info);

    while(1)
    {
        //palSystem_TaskMonitorTimerReset(eTID_IPC_RECV_DATA, NULL);  //A35G2_Simon_0075

        if(Ipc_waitMsg(&pMsg, pInfo) != 0)
        {
            usleep(TIMEOUT_WAIT_MSEC * 1000);
            continue;
        }
        if(Ipc_getMsgInfo(pMsg, &MsgInfo) == 0)
        {
            //DEBUGP("Scaler get data. Msg Type:%d Well-know Name:%s ObjPath:%s Iface:%s Name:%s \n",
                   //MsgInfo.Type, MsgInfo.pWellKnowName, MsgInfo.pObjPath, MsgInfo.pIfaceName, MsgInfo.pName);
            switch(MsgInfo.Type)
            {
                case DBUS_MESSAGE_TYPE_METHOD_CALL:
                    for(Idx=0; Idx<ARRAY_SIZE(g_MsgMethodTable); Idx++)
                    {
                        if((strcmp(MsgInfo.pIfaceName, g_MsgMethodTable[Idx].IfaceName) == 0) &&
                           (strcmp(MsgInfo.pName, g_MsgMethodTable[Idx].Name) == 0) &&
                           (g_MsgMethodTable[Idx].MsgMethodProc != NULL))
                        {
                            g_MsgMethodTable[Idx].MsgMethodProc(pInfo, &MsgInfo, pMsg, Scaler_getRecvCb(pRecvInfo, MsgInfo.pName, MsgInfo.Type));
                            break;
                        }
                    }
                    break;
                case DBUS_MESSAGE_TYPE_SIGNAL:
                    for(Idx=0; Idx<ARRAY_SIZE(g_MsgSignalTable); Idx++)
                    {
                        if((strcmp(MsgInfo.pIfaceName, g_MsgSignalTable[Idx].IfaceName) == 0) &&
                           (strcmp(MsgInfo.pName, g_MsgSignalTable[Idx].Name) == 0) &&
                           (g_MsgSignalTable[Idx].MsgSignalProc != NULL))
                        {
                            g_MsgSignalTable[Idx].MsgSignalProc(pInfo, &MsgInfo, pMsg, Scaler_getRecvSingalCb(pRecvInfo, MsgInfo.pName));
                            break;
                        }
                    }
                    break;
                case DBUS_MESSAGE_TYPE_METHOD_RETURN:
                    Scaler_procMethodReturn(pInfo, &MsgInfo, pMsg, pRecvInfo->AckCb);
                    break;
                case DBUS_MESSAGE_TYPE_ERROR:
                    printf("DBUS get error type \n");
                    break;
                case DBUS_MESSAGE_TYPE_INVALID:
                    printf("DBUS get invalid type \n");
                    break;
                default:
                    printf("DBUS get unknow type:%d \n", MsgInfo.Type);
                    break;
            }
        }
        else
        {
            printf("Scaler get data fail. Type:%d, Well-know:%s \n", MsgInfo.Type, MsgInfo.pWellKnowName);
        }
        Ipc_freeMsg(pMsg);
    }
}
//-------------------------------------------------------------------------------------------------
/*
 * Scaler_sendMethodData no wait ackdone
 * */
u8_t Scaler_sendMethodSetData(void *pData, u32_t Size)
{
    u32_t IfaceIdx;
    u32_t Timer;
    u32_t TimeOut = FALSE;
    _sDbusInfo *pInfo;

    if(pData == NULL)
    {
        printf("Input param error when Scaler_sendMethodSetData \n");
        return 1;
    }

    if(Size == 0)
    {
        //DEBUGP("No data to send when Scaler_sendMethodSetData \n");
        return 0;
    }

    pInfo = &(g_RecvInfo.Info);

    if(g_MethodAckInfo.Status != METHOD_DATA_STATUS_IDLE)
        return 0;

    Ipc_getIfaceIdx(&IfaceIdx, pInfo, NAME_IFACE_METHOD_PROJECTOR, "set");
    g_MethodAckInfo.Status = METHOD_DATA_STATUS_SEND;
    if(Ipc_SendMethodData(pInfo, pData, Size, IfaceIdx, NAME_DBUS_CLI))
    {
        printf("Send method data no callback error when Scaler_sendMethodSetData\n");
        return 2;
    }
    Timer = TMO_GetSysRunTime();
    while(1)
    {
        if(g_MethodAckInfo.Status == METHOD_DATA_STATUS_ACK )
            break;
        if(TMO_GetPeriod(Timer) > TIMEOUT_SEND_METHOD)
        {
            TimeOut = TRUE;
            break;
        }
        usleep(WAIT_SEND_METHOD * 1000);
    }
    g_MethodAckInfo.Status = METHOD_DATA_STATUS_IDLE;

    if(TimeOut == TRUE)
    {
        printf("App send data timeout when Scaler_sendMethodSetData\n");
        return 3;
    }
    return 0;
}
//-------------------------------------------------------------------------------------------------
u8_t Scaler_sendMethodAckdoneData(void *pData, u32_t Size)
{
    u32_t IfaceIdx;
    u32_t Timer;
    u32_t TimeOut = FALSE;
    _sDbusInfo *pInfo;

    if(pData == NULL)
    {
        printf("Input param error when Scaler_sendMethodAckdoneData \n");
        return 1;
    }

    if(Size == 0)
    {
        //DEBUGP("No data to send when Scaler_sendMethodAckdoneData \n");
        return 0;
    }

    pInfo = &(g_RecvInfo.Info);

    if(g_MethodAckInfo.Status != METHOD_DATA_STATUS_IDLE)
        return 0;

    Ipc_getIfaceIdx(&IfaceIdx, pInfo, NAME_IFACE_METHOD_PROJECTOR, "ackdone");
    g_MethodAckInfo.Status = METHOD_DATA_STATUS_SEND;

    if(Ipc_SendMethodData(pInfo, pData, Size, IfaceIdx, NAME_DBUS_CLI))
    {
        printf("Send method data no callback error when App_sendData\n");
        return 2;
    }
    Timer = TMO_GetSysRunTime();
    while(1)
    {
        if(g_MethodAckInfo.Status == METHOD_DATA_STATUS_ACK )
            break;
        if(TMO_GetPeriod(Timer) > TIMEOUT_SEND_METHOD)
        {
            TimeOut = TRUE;
            break;
        }
        usleep(WAIT_SEND_METHOD * 1000);
    }
    g_MethodAckInfo.Status = METHOD_DATA_STATUS_IDLE;

    if(TimeOut == TRUE)
    {
        printf("App send data timeout when App_sendData\n");
        return 3;
    }
    return 0;
}
//-------------------------------------------------------------------------------------------------
void Scaler_initMethodAckInfo(_MethodAckInfo *pInfo)
{
    if(pInfo == NULL)
        return;
    pInfo->Status = METHOD_DATA_STATUS_IDLE;
    memset(pInfo->Data, 0, sizeof(pInfo->Data));
}
//-------------------------------------------------------------------------------------------------
_DbusIfaceData g_ScalerIfaceInfo[] =
{
    {DBUS_SIGNAL, NAME_IFACE_SIGNAL_PROJECTOR, "set"},
    {DBUS_SIGNAL, NAME_IFACE_SIGNAL_PROJECTOR, "get"},
    {DBUS_SIGNAL, NAME_IFACE_SIGNAL_PROJECTOR, "error"},
    {DBUS_METHOD, NAME_IFACE_METHOD_PROJECTOR, "set"},
    {DBUS_METHOD, NAME_IFACE_METHOD_PROJECTOR, "setackdone"},
    {DBUS_METHOD, NAME_IFACE_METHOD_PROJECTOR, "ackdone"},
    {DBUS_METHOD, NAME_IFACE_METHOD_PROJECTOR, "get"},
};
//-------------------------------------------------------------------------------------------------
u8_t Scaler_initRecvCb(_sDusRecvInfo *pInfo)
{
    if(pInfo == NULL)
    {
        printf("Input param error when App_initRecvInfo \n");
    }
    else    //G100_Simon_0060
    {
        pInfo->MethodSetCb        = NULL;
        pInfo->MethodGetCb        = NULL;
        pInfo->MethodSetAckdoneCb = NULL;
        pInfo->MethodAckdoneCb    = NULL;
        pInfo->SignalSetCb        = NULL;
        pInfo->SignalGetCb        = NULL;
        pInfo->SignalErrorCb      = NULL;
        pInfo->AckCb              = NULL;
    }

    return 0;
}
//-------------------------------------------------------------------------------------------------
u8_t Scaler_initCb(_ipcCallBack *pInfo)
{
    if(pInfo == NULL)
    {
        printf("Input param error when Scaler_initCb\n");
        return 1;
    }

    pInfo->MethodSetCb        = NULL;
    pInfo->MethodGetCb        = NULL;
    pInfo->MethodSetAckdoneCb = NULL;
    pInfo->MethodAckdoneCb    = NULL;
    pInfo->SignalSetCb        = NULL;
    pInfo->SignalGetCb        = NULL;
    pInfo->SignalErrorCb      = NULL;
    pInfo->AckCb              = NULL;

}
//-------------------------------------------------------------------------------------------------
u8_t Scaler_initIpc(_ipcCallBack *pCallBack)
{
    pthread_attr_t ThreadAttr;
     _DbusIfaceTable IfaceInfo;

    if(pCallBack == NULL)
    {
        printf("Input param error when App_initIpc \n");
        return 1;
    }
    Scaler_initMethodAckInfo(&g_MethodAckInfo);

    IfaceInfo.NUmber = ARRAY_SIZE(g_ScalerIfaceInfo);
    IfaceInfo.pTable = g_ScalerIfaceInfo;;
    Scaler_initRecvCb(&g_RecvInfo);
    g_RecvInfo.MethodSetCb        = pCallBack->MethodSetCb;
    g_RecvInfo.MethodGetCb        = pCallBack->MethodGetCb;
    g_RecvInfo.MethodSetAckdoneCb = pCallBack->MethodSetAckdoneCb;
    g_RecvInfo.MethodAckdoneCb    = pCallBack->MethodAckdoneCb;
    g_RecvInfo.SignalErrorCb      = pCallBack->SignalErrorCb;
    g_RecvInfo.SignalSetCb        = pCallBack->SignalSetCb;
    g_RecvInfo.SignalGetCb        = pCallBack->SignalGetCb;
    g_RecvInfo.AckCb              = pCallBack->AckCb;

    Ipc_initDbus(&(g_RecvInfo.Info), PATH_DBUS_PROJECTOR_SERVICE_FILE, NAME_DBUS_SCALER, PATH_PROJECTOR_OBJ, &IfaceInfo);

    Scaler_setThreadAttr(&ThreadAttr, PTHREAD_CREATE_DETACHED);
    pthread_create(&g_thread_recv, &ThreadAttr, Scaler_recvData, (void *)&g_RecvInfo);
    return 0;
}
//-------------------------------------------------------------------------------------------------
u8_t Scaler_disconnect(void)
{
    Ipc_closeConnect(&(g_RecvInfo.Info));
}
//-------------------------------------------------------------------------------------------------
/*
s32_t main(void)
{
    _sDbusInfo Info;
    pthread_attr_t ThreadAttr;
    pthread_t thread_recv;
    _DataInfo DataInfo;
    _DbusIfaceTable IfaceInfo;
    u8_t TestBuf[4096+512];
    u32_t TestBufSize;

    Scaler_initCmdInfo(&g_CmdInfo);

    IfaceInfo.NUmber = ARRAY_SIZE(g_ScalerIfaceInfo);
    IfaceInfo.pTable = g_ScalerIfaceInfo;
    Ipc_initDbus(&Info, PATH_DBUS_PROJECTOR_SERVICE_FILE, NAME_DBUS_SCALER, PATH_PROJECTOR_OBJ, &IfaceInfo);

    Scaler_initMethodAckInfo(&g_MethodAckInfo);

    Scaler_setThreadAttr(&ThreadAttr, PTHREAD_CREATE_DETACHED);
    pthread_create(&thread_recv, &ThreadAttr, Scaler_recvData, (void *)&Info);

    while(1)
    {
        TestBufSize = sizeof(TestBuf);
        if(Scaler_getCmd(TestBuf, &TestBufSize) != 0)
        {
            usleep(10*1000);
            continue;
        }
        if(TestBufSize == 0)
        {
            usleep(10*1000);
            continue;
        }

        usleep(10*1000);
        Scaler_sendMethodData(&Info, TestBuf, TestBufSize, NAME_IFACE_METHOD_PROJECTOR, "ackdone");
    }
    Ipc_closeConnect(&Info);
}
*/
