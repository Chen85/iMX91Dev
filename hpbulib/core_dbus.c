#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "type_def.h"
#include "hicc_config.h"
#include "conf.h"
#include "core_dbus.h"
//=================================================================================================
//#define SHOW_DEBUG

#ifdef SHOW_DEBUG
    #ifndef DEBUGP
        #define DEBUGP printf
    #endif //DEBUGP
#else
    #define DEBUGP
#endif //SHOW_DEBUG
//=================================================================================================
u8_t Ipc_initConnect(char *pName, _sDbusInfo *pInfo)
{
    DBusError Err;
    u32_t Ret;

    if(pInfo == NULL)
        return 1;
    dbus_error_init(&Err);
    if((pName == NULL) || (strlen(pName) == 0))
    {
        pInfo->pCon = dbus_bus_get(DBUS_BUS_SESSION, &Err);
    }
    else
    {
        pInfo->pCon = dbus_connection_open(pName, &Err);
        if(pInfo->pCon)
        {
            if (!dbus_bus_register (pInfo->pCon, &Err))
            {
                printf("Open connect fail when Ipc_initConnect\n");
                dbus_error_free (&Err);
                return 2;
            }
        }
        else
            pInfo->pCon = dbus_bus_get(DBUS_BUS_SESSION, &Err);
    }

    if(dbus_error_is_set(&Err))
    {
        printf("Connection Error (%s)\n", Err.message);
        dbus_error_free(&Err);
    }
    if(pInfo->pCon == NULL)
    {
        printf("Ipc_initConnect get session fail \n");
        return 3;
    }
    Ret = dbus_bus_request_name(pInfo->pCon, pInfo->DbusName, DBUS_NAME_FLAG_REPLACE_EXISTING, &Err);
    if(dbus_error_is_set(&Err))
    {
        printf("Name Error (%s)\n", Err.message);
        dbus_error_free(&Err);
    }
    if(Ret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER)
        return 4;
    return 0;
}
//-------------------------------------------------------------------------------------------------
u8_t Ipc_sendSignal(_sDbusInfo *pInfo, char *pData, u32_t Size, u32_t IfaceIdx)
{
    u32_t Serial = 0;
    u32_t Age;
    DBusMessage *pMsg;
    DBusMessageIter Args;

    if((pInfo == NULL) || (pData == NULL) || (IfaceIdx >= MAX_NUM_IPC_LISTERN_IFACE))
    {
        printf("Ipc_SendSignal Input param fail \n");
        return 1;
    }
    if(pInfo->pCon == NULL)
    {
        printf("Ipc_SendSignal Connect name fail \n");
        return 2;
    }
    if(strlen(pInfo->ObjPath) == 0)
    {
        printf("Ipc_SendSignal Obj Path error \n");
        return 3;
    }
    if((strlen(pInfo->IfaceInfo.Data[IfaceIdx].IfaceName) == 0) || (strlen(pInfo->IfaceInfo.Data[IfaceIdx].String) == 0))
    {
        printf("Ipc_SendSignal iface name error \n");
        return 4;
    }

    if(pInfo->IfaceInfo.Data[IfaceIdx].Type != DBUS_SIGNAL)
    {
        printf("Send data type is error when Ipc_sendSignal \n");
        return 5;
    }

    if(Size == 0)
        return 0;
    pMsg = dbus_message_new_signal(pInfo->ObjPath, pInfo->IfaceInfo.Data[IfaceIdx].IfaceName, pInfo->IfaceInfo.Data[IfaceIdx].String);
    if(pMsg == NULL)
    {
        printf("Ipc_SendSignal Message Null \n");
        return 6;
    }

    if(Ipc_writeMethodData(pMsg, pData, Size) == 0)
    {
        if(!dbus_connection_send(pInfo->pCon, pMsg, &Serial))
        {
            printf("Ipc_SendSignal Out of memory \n");
            return 6;
        }
    }
    dbus_connection_flush(pInfo->pCon);
    // free the message and close the connection
    //sync();
    dbus_message_unref(pMsg);
  //  dbus_connection_close(pInfo->pCon);
    return 0;
}
//-------------------------------------------------------------------------------------------------
u8_t Ipc_setIface(u32_t *pIdx, _sDbusInfo *pInfo, _DBusType Type, char *pName, char *pString)
{
    u8_t i;

    if((pIdx == NULL) || (pInfo == NULL) || (Type >= DBUS_NUM) || (pName == NULL) || (pString== NULL))
    {
        printf("Input param error when Ipc_setIface \n");
        return 1;
    }
    if(pInfo->IfaceInfo.Number > MAX_NUM_IPC_LISTERN_IFACE)
    {
        printf("Iface buffer is full when Ipc_setIface\n");
        return 2;
    }

    for(i=0; i<MAX_NUM_IPC_LISTERN_IFACE; i++)
    {
        if((strlen(pInfo->IfaceInfo.Data[i].IfaceName) == 0) && (strlen(pInfo->IfaceInfo.Data[i].String) == 0))
        {
            snprintf(pInfo->IfaceInfo.Data[i].IfaceName, sizeof(pInfo->IfaceInfo.Data[i].IfaceName), "%s",pName);
            snprintf(pInfo->IfaceInfo.Data[i].String, sizeof(pInfo->IfaceInfo.Data[i].String), "%s",pString);
            pInfo->IfaceInfo.Data[i].Type = Type;
            pInfo->IfaceInfo.Number ++;
            *pIdx = i;
            return 0;
        }
    }
    printf("No find empty iface buffer when Ipc_setIface \n");
    *pIdx = MAX_NUM_IPC_LISTERN_IFACE;
    return 3;
}
//-------------------------------------------------------------------------------------------------
u8_t Ipc_clearIface(_sDbusInfo *pInfo, char *pIfaceName, char *pString)
{
    u32_t i;

    if((pInfo == NULL) || (pIfaceName == NULL) || (pString == NULL))
    {
        printf("Input param error when Ipc_clearIface");
        return 1;
    }
    if((strlen(pIfaceName) == 0) || (strlen(pString) == 0))
    {
        printf("Iface name error when Ipc_clearIface\n");
        return 2;
    }

    if(pInfo->IfaceInfo.Number == 0)
        return 0;

    for(i=0; i<MAX_NUM_IPC_LISTERN_IFACE ; i++)
    {
        if((strcmp(pInfo->IfaceInfo.Data[i].IfaceName, pIfaceName) == 0) &&
           (strcmp(pInfo->IfaceInfo.Data[i].String, pString) == 0))
        {
            memset(pInfo->IfaceInfo.Data[i].IfaceName, 0, sizeof(pInfo->IfaceInfo.Data[i].IfaceName));
            memset(pInfo->IfaceInfo.Data[i].String, 0, sizeof(pInfo->IfaceInfo.Data[i].String));
            pInfo->IfaceInfo.Number --;
            return 0;
        }
    }
    printf("No find iface name when Ipc_clearIface \n");
    return 3;
}
//-------------------------------------------------------------------------------------------------
/*
 * DBUS_MESSAGE_TYPE_INVALID       0
 * DBUS_MESSAGE_TYPE_METHOD_CALL   1
 * DBUS_MESSAGE_TYPE_METHOD_RETURN 2
 * DBUS_MESSAGE_TYPE_ERROR         3
 * DBUS_MESSAGE_TYPE_SIGNAL        4
 * DBUS_NUM_MESSAGE_TYPES          5
 * */
u8_t Ipc_getRecvMsgType(DBusMessage *pMsg, u8_t *pType)
{
    if((pMsg == NULL) || (pType == NULL))
    {
        printf("Input param error when ");
        return 1;
    }
    *pType = dbus_message_get_type(pMsg);
    if(DBUS_MESSAGE_TYPE_INVALID == *pType)
    {
        printf("Get message type fail when Ipc_getRecvMsgType \n");
        return 2;
    }
    return 0;
}
//-------------------------------------------------------------------------------------------------
u8_t Ipc_getIfaceIdx(u32_t *pIdx, _sDbusInfo *pInfo, char *pIfaceName, char *pString)
{
    u32_t i;

    if((pIdx == NULL) || (pInfo == NULL) || (pIfaceName == NULL) || (pString == NULL))
    {
        printf("Input param error when Ipc_getIface \n");
        return 1;
    }
    if((strlen(pIfaceName) == 0) || (strlen(pString) == 0))
    {
        printf("Iface name is error when Ipc_getIface");
        return 2;
    }
    if(pInfo->IfaceInfo.Number == 0)
    {
        printf("No find iface buffer is empty when Ipc_getIface \n");
        return 3;
    }

    for(i=0; i<MAX_NUM_IPC_LISTERN_IFACE ; i++)
    {
        if((strcmp(pInfo->IfaceInfo.Data[i].IfaceName, pIfaceName) == 0) &&
           (strcmp(pInfo->IfaceInfo.Data[i].String, pString) == 0))
        {
            *pIdx = i;
            return 0;
        }
    }
    *pIdx = MAX_NUM_IPC_LISTERN_IFACE;
    printf("No find iface index when Ipc_getIface \n");
    return 4;
}
//-------------------------------------------------------------------------------------------------
u8_t Ipc_addSignalMatch(_sDbusInfo *pInfo, u32_t Idx)
{
    DBusError Err;
    char RuleStr[128];

    if((pInfo == NULL) || (Idx >= MAX_NUM_IPC_LISTERN_IFACE))
    {
        printf("Input param error when Ipc_addMatch \n");
        return 1;
    }

    if(pInfo->pCon == NULL)
    {
        printf("Connect error when Ipc_addMatch \n");
        return 2;
    }
    dbus_error_init(&Err);
    snprintf(RuleStr, sizeof(RuleStr), "type='signal',interface='%s'", pInfo->IfaceInfo.Data[Idx].IfaceName);
    dbus_bus_add_match(pInfo->pCon, RuleStr, &Err); // see signals from the given interface
    dbus_connection_flush(pInfo->pCon);
    if(dbus_error_is_set(&Err))
    {
        printf("IpcRecvSignalParn_Match Error (%s) when  \n", Err.message);
        dbus_error_free(&Err);
        return 3;
    }

    return 0;
}
//-------------------------------------------------------------------------------------------------
u8_t Ipc_getObjPathFromMsg(DBusMessage *pMsg, char **pObjPath)
{
    if((pMsg == NULL) || (pObjPath == NULL))
    {
        printf("Input param error when Ipc_getObjPathFromMsg \n");
        return 1;
    }

    *pObjPath = (char *)dbus_message_get_path(pMsg);
    if(*pObjPath == NULL)
    {
        printf("No find object path when Ipc_getObjPathFromMsg \n");
        return 2;
    }
    return 0;
}
//-------------------------------------------------------------------------------------------------
u8_t Ipc_getWellKnownNameFromMsg(DBusMessage *pMsg, char **pName)
{
    if((pMsg == NULL) || (pName == NULL))
    {
        printf("Input param error when Ipc_getWellKnownNameFromMsg \n");
        return 1;
    }

    *pName = (char *)dbus_message_get_sender(pMsg);
    if(*pName == NULL)
    {
        printf("No find Well-Known Name when Ipc_getWellKnownNameFromMsg \n");
        return 2;
    }
    return 0;
}
//-------------------------------------------------------------------------------------------------
u8_t Ipc_getNameFromMsg(DBusMessage *pMsg, char **pName)
{
    if((pMsg == NULL) || (pName == NULL))
    {
        printf("Input param error when Ipc_getNameFromMsg \n");
        return 1;
    }

    *pName = (char *)dbus_message_get_member(pMsg);
    if(*pName ==  NULL)
    {
        printf("No find Message Name when Ipc_getWellKnownNameFromMsg \n");
        return 2;
    }
    return 0;
}
//-------------------------------------------------------------------------------------------------
u8_t Ipc_getDestinationFromMsg(DBusMessage *pMsg, char **pDest)
{
    if((pMsg ==  NULL) || (pDest == NULL))
    {
        printf("Input param error when Ipc_getDestinationFromMsg\n");
        return 0;
    }

    *pDest = (char *)dbus_message_get_destination(pMsg);
    if(*pDest == NULL)
    {
        printf("No find Message Destination when Ipc_getDestinationFromMsg");
        return 2;
    }
    return 0;
}
//-------------------------------------------------------------------------------------------------
u8_t Ipc_getIfaceNameFromMsg(DBusMessage *pMsg, char **pIfaceName)
{
    if((pMsg == NULL) || (pIfaceName == NULL))
    {
        printf("Input param error when Ipc_getNameFromMsg \n");
        return 1;
    }

    *pIfaceName = (char *)dbus_message_get_interface(pMsg);
    if(*pIfaceName ==  NULL)
    {
        printf("No find Iface Name when Ipc_getIfaceNameFromMsg \n");
        return 2;
    }
    return 0;
}
//-------------------------------------------------------------------------------------------------
u8_t Ipc_getMsgInfo(DBusMessage *pMsg, _DbusMsgInfo *pInfo)
{
    if((pMsg == NULL) || (pInfo == NULL))
    {
        printf("Input param error when Ipc_getMsgInfo");
        return 1;
    }

    if(Ipc_getRecvMsgType(pMsg, &(pInfo->Type)) != 0)
    {
        printf("Get message type error when Ipc_getMsgInfo \n");
        return 2;
    }

    if(Ipc_getWellKnownNameFromMsg(pMsg, &(pInfo->pWellKnowName)) != 0)
    {
        printf("Get Well-know name error when Ipc_getMsgInfo \n");
        return 3;
    }

    if((pInfo->Type != DBUS_MESSAGE_TYPE_SIGNAL) && (Ipc_getDestinationFromMsg(pMsg, &(pInfo->pDestination))) != 0)
    {
        printf("Get Destination error when Ipc_getMsgInfo \n");
        return 4;
    }

    if((pInfo->Type == DBUS_MESSAGE_TYPE_METHOD_RETURN) || (pInfo->Type == DBUS_MESSAGE_TYPE_ERROR))
        return 0;

    if(Ipc_getIfaceNameFromMsg(pMsg, &(pInfo->pIfaceName)) != 0)
    {
        printf("Get Iface name error when Ipc_getMsgInfo \n");
        return 5;
    }
    if(Ipc_getObjPathFromMsg(pMsg, &(pInfo->pObjPath)) != 0)
    {
        printf("Get Obj path error when Ipc_getMsgInfo \n");
        return 6;
    }
    if(Ipc_getNameFromMsg(pMsg, &(pInfo->pName)) != 0)
    {
        printf("Get name error when Ipc_getMsgInfo \n");
        return 7;
    }
    return 0;
}

//-------------------------------------------------------------------------------------------------
void Ipc_freeMsg(DBusMessage *pMsg)
{
    dbus_message_unref(pMsg);
}
//-------------------------------------------------------------------------------------------------
u8_t Ipc_closeConnect(_sDbusInfo *pInfo)
{
    if(pInfo == NULL)
    {
        printf("Input param error when Ipc_closeConnect \n");
        return 1;
    }
    if(pInfo->pCon == NULL)
    {
        printf("No find dbus connect when Ipc_closeConnect \n");
        return 2;
    }
    dbus_connection_unref(pInfo->pCon);

    return 0;
}
//-------------------------------------------------------------------------------------------------
u8_t Ipc_writeMethodData(DBusMessage *pMsg, void *pData, u32_t Size)
{
    u8_t *pDataTmp;
    DBusMessageIter Arg, SubArg;

    if((pMsg == NULL) || (pData == NULL) || (Size == 0))
    {
        printf("Input param fail when Ipc_writeMethodData \n");
        return 1;
    }

    pDataTmp = (u8_t *)pData;
    dbus_message_iter_init_append(pMsg,&Arg);
    dbus_message_iter_open_container(&Arg, DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE_AS_STRING, &SubArg);
    dbus_message_iter_append_fixed_array(&SubArg, DBUS_TYPE_BYTE, &pDataTmp, Size);
    dbus_message_iter_close_container(&Arg, &SubArg);
    return 0;
}
//-------------------------------------------------------------------------------------------------
u8_t Ipc_readMethodData(DBusMessage *pMsg, void **pData, u32_t *pSize)
{
    DBusMessageIter Arg, SubArg;

    if((pMsg == NULL) || (pData == NULL) || (pSize == NULL))
    {
        printf("Input param error when Ipc_readMethodData \n");
        return 1;
    }
    dbus_message_iter_init(pMsg,&Arg);
    dbus_message_iter_recurse(&Arg, &SubArg);
    dbus_message_iter_get_fixed_array(&SubArg, pData, pSize);
    return 0;
}
//-------------------------------------------------------------------------------------------------
/*
u8_t Ipc_readSignalData(DBusMessage *pMsg, char *pValType, void **pData)
{
    DBusMessageIter Args;
    void *pDataTmp;

    if((pMsg == NULL) || (pValType == NULL) || (pData == NULL))
    {
        printf("input param error when Ipc_readSignalData \n");
        return 1;
    }

    if (dbus_message_iter_init(pMsg, &Args) == 0)
    {
        printf("Message Has No Parameters when Ipc_readSignalData\n");
        return 2;
    }
    *pValType = dbus_message_iter_get_arg_type(&Args);
    dbus_message_iter_get_basic(&Args, pData);
    return 0;
}
*/
//-------------------------------------------------------------------------------------------------
u8_t Ipc_replytoMethodCall(DBusMessage *pMsg, _sDbusInfo *pInfo, void *pData, u32_t Size)
{
    DBusMessage *pReply;
    u32_t Serial = 0;

    if((pMsg == NULL) || (pInfo == NULL) || (pInfo->pCon == NULL) || (pData == NULL) || (Size == 0))
    {
        printf("Input paran error when Ipc_replytoMethodCall \n");
        return 1;
    }

    pReply = dbus_message_new_method_return(pMsg);
    Ipc_writeMethodData(pReply, pData, Size);

    if( dbus_connection_send(pInfo->pCon, pReply, &Serial) == 0)
    {
        printf("Out of Memory when Ipc_replytoMethodCall /n");
        return 2;
    }

    dbus_connection_flush (pInfo->pCon);
    dbus_message_unref(pReply);
    return 0;
}
//-------------------------------------------------------------------------------------------------
u8_t Ipc_sendMethodBlock(DBusMessage **pReplyMsg, _sDbusInfo *pInfo, void *pData, u32_t Size, u32_t IfaceIdx, char *pdestination, s32_t TimeOut)
{
    DBusMessage *pmsg;
    DBusError Err;
    void *pDataTmp;
    u32_t RealSize;

    if((pReplyMsg == NULL) || (pInfo == NULL) || (pData == NULL) ||
       (Size == 0) || (IfaceIdx >= MAX_NUM_IPC_LISTERN_IFACE) || (pdestination == NULL) || (strlen(pdestination) == 0))
    {
        printf("Input param error when Ipc_sendMethodBlock \n");
        return 1;
    }
    if(pInfo->IfaceInfo.Data[IfaceIdx].Type != DBUS_METHOD)
    {
        printf("Dbus type error when Ipc_sendMethodBlock \n");
        return 2;
    }
    dbus_error_init(&Err);
    //针对目的地地址，请参考图，创建一个method call消息。 Constructs a new message to invoke a method on a remote object.
    pmsg = dbus_message_new_method_call(pdestination, pInfo->ObjPath, pInfo->IfaceInfo.Data[IfaceIdx].IfaceName, pInfo->IfaceInfo.Data[IfaceIdx].String);
    if(pmsg == NULL)
    {
        printf("Message NULL when Ipc_sendMethodBlock \n");
        return 3;
    }
    if(Ipc_writeMethodData(pmsg, pData, Size) != 0)
    {
        printf("Write data fail when Ipc_sendMethodBlock \n");
        return 4;
    }
    *pReplyMsg = dbus_connection_send_with_reply_and_block(pInfo->pCon, pmsg, TimeOut, &Err);
    dbus_message_unref(pmsg);
    if(*pReplyMsg == NULL)
    {
        dbus_error_free (&Err);
        printf("Reply Null when  Ipc_sendMethodBlock\n");
        return 7;
    }
    return 0;
}
//-------------------------------------------------------------------------------------------------
/*
 u8_t Ipc_sendMethod(void (*CallBack)(DBusPendingCall *pending, void *user_data), _sDbusInfo *pInfo, void *pData, u32_t Size, u32_t IfaceIdx, char *pdestination)
{
    DBusMessage *pmsg;
    void *pDataTmp;
    u32_t RealSize;
    DBusPendingCall *pending;

    if((CallBack == NULL) || (pInfo == NULL) || (pData == NULL) ||
       (Size == 0) || (IfaceIdx >= MAX_NUM_IPC_LISTERN_IFACE) || (pdestination == NULL) || (strlen(pdestination) == 0))
    {
        DEBUGP("Input param error when Ipc_sendMethodBlock \n");
        return 1;
    }

    if(pInfo->IfaceInfo.Data[IfaceIdx].Type != DBUS_METHOD)
    {
        DEBUGP("Dbus type error when Ipc_sendMethodBlock \n");
        return 2;
    }
    //针对目的地地址，请参考图，创建一个method call消息。 Constructs a new message to invoke a method on a remote object.
    pmsg = dbus_message_new_method_call(pdestination, pInfo->ObjPath, pInfo->IfaceInfo.Data[IfaceIdx].IfaceName, pInfo->IfaceInfo.Data[IfaceIdx].String);
    if(pmsg == NULL)
    {
        DEBUGP("Message NULL when Ipc_sendMethodBlock \n");
        return 3;
    }
    if(Ipc_writeMethodData(pmsg, pData, Size) != 0)
    {
        DEBUGP("Write data fail when Ipc_sendMethodBlock \n");
        return 4;
    }
    //发送消息并获得reply的handle 。Queues a message to send, as with dbus_connection_send() , but also returns a DBusPendingCall used to receive a reply to the message.
    if(!dbus_connection_send_with_reply(pInfo->pCon, pmsg, &pending, -1)){
        DEBUGP("Out of Memory! when Ipc_sendMethodBlock \n");
        return 5;
    }
    if(pending == NULL) {
        DEBUGP("Pending Call NULL: connection is disconnected when Ipc_sendMethodBlock \n");
        dbus_message_unref(pmsg);
        return 6;
    }

    dbus_pending_call_set_notify(pending, CallBack, NULL, NULL);
    dbus_pending_call_unref(pending);
    dbus_message_unref(pmsg);

    return 0;
}
*/
//-------------------------------------------------------------------------------------------------
u8_t Ipc_SendMethodData(_sDbusInfo *pInfo, void *pData, u32_t Size, u32_t IfaceIdx, char *pdestination)
{
    DBusMessage *pmsg;
    void *pDataTmp;
    u32_t RealSize;

    if((pInfo == NULL) || (pData == NULL) ||
       (Size == 0) || (IfaceIdx >= MAX_NUM_IPC_LISTERN_IFACE) || (pdestination == NULL) || (strlen(pdestination) == 0))
    {
        printf("Input param error when Ipc_SendMethodData \n");
        return 1;
    }

    if(pInfo->IfaceInfo.Data[IfaceIdx].Type != DBUS_METHOD)
    {
        printf("Dbus type error when Ipc_SendMethodData \n");
        return 2;
    }
    //针对目的地地址，请参考图，创建一个method call消息。 Constructs a new message to invoke a method on a remote object.
    pmsg = dbus_message_new_method_call(pdestination, pInfo->ObjPath, pInfo->IfaceInfo.Data[IfaceIdx].IfaceName, pInfo->IfaceInfo.Data[IfaceIdx].String);
    if(pmsg == NULL)
    {
        printf("Message NULL when Ipc_SendMethodData \n");
        return 3;
    }
    if(Ipc_writeMethodData(pmsg, pData, Size) != 0)
    {
        printf("Write data fail when Ipc_SendMethodData \n");
        return 4;
    }
    //发送消息并获得reply的handle 。Queues a message to send, as with dbus_connection_send() , but also returns a DBusPendingCall used to receive a reply to the message.
    if(!dbus_connection_send(pInfo->pCon, pmsg, NULL))
    {
        printf("Out of Memory! when Ipc_SendMethodData \n");
        return 5;
    }
    dbus_connection_flush(pInfo->pCon);
    //sync();
    dbus_message_unref(pmsg);
    return 0;
}
//-------------------------------------------------------------------------------------------------
u8_t Ipc_SendMethodDataByDetailData(_sDbusInfo *pInfo, char *pdestination, char *pObjpath, char *pIfaceName, char *pTypeString, void *pData, u32_t Size)
{
    DBusMessage *pmsg;
    void *pDataTmp;
    u32_t RealSize;

    if((pdestination == NULL) || (pObjpath == NULL) || (pIfaceName == NULL) || (pTypeString == NULL) || (pData == NULL))
    {
        printf("Input param error when Ipc_SendMethodDataByDetailData \n");
        return 1;
    }

    if(Size == 0)
    {
        printf("No method data to send when Ipc_SendMethodDataByDetailData \n");
        return 0;
    }

    //针对目的地地址，请参考图，创建一个method call消息。 Constructs a new message to invoke a method on a remote object.
    pmsg = dbus_message_new_method_call(pdestination, pObjpath, pIfaceName, pTypeString);
    if(pmsg == NULL)
    {
        printf("Message NULL when Ipc_SendMethodDataByDetailData \n");
        return 3;
    }
    if(Ipc_writeMethodData(pmsg, pData, Size) != 0)
    {
        printf("Write data fail when Ipc_SendMethodDataByDetailData \n");
        return 4;
    }
    //发送消息并获得reply的handle 。Queues a message to send, as with dbus_connection_send() , but also returns a DBusPendingCall used to receive a reply to the message.
    if(!dbus_connection_send(pInfo->pCon, pmsg, NULL))
    {
        printf("Out of Memory! when dbus_connection_send when Ipc_SendMethodDataByDetailData\n");
        return 5;
    }
    dbus_connection_flush(pInfo->pCon);
    dbus_message_unref(pmsg);
    return 0;
}
//-------------------------------------------------------------------------------------------------
u8_t Ipc_waitMsg(DBusMessage **pMsg, _sDbusInfo *pInfo)
{
    if((pMsg == NULL) || (pInfo == NULL) || (pInfo->pCon == NULL))
    {
        printf("Input param error when Ipc_waitMethodMsg \n");
        return 1;
    }

    dbus_connection_read_write (pInfo->pCon, 0);
    *pMsg = dbus_connection_pop_message (pInfo->pCon);
    if(*pMsg != NULL)
        return 0;
    return 2;
}
//-------------------------------------------------------------------------------------------------
u8_t Ipc_checkMsg(DBusMessage *pMsg, _sDbusInfo *pInfo, u32_t IfaceIdx)
{
    if((pMsg == NULL) || (IfaceIdx >= MAX_NUM_IPC_LISTERN_IFACE))
    {
        printf("Input param error when Ipc_checkMsg \n");
        return 1;
    }
    if(dbus_message_is_method_call(pMsg, pInfo->IfaceInfo.Data[IfaceIdx].IfaceName, pInfo->IfaceInfo.Data[IfaceIdx].String))
        return 0;
    return 2;
}
//-------------------------------------------------------------------------------------------------
u8_t Ipc_getDbusServName(char *pName, u32_t Size, char *pFilePath)
{
    struct stat StatBuf;
    char Buffer[1024];
    char *pStr, *pStr1;

    if((pName == NULL) || (pFilePath == NULL))
    {
        printf("Input param error when Ipc_getDbusServName \n");
        return 1;
    }
    if(Size == 0)
    {
        printf("No space to save dbus service name when Ipc_getDbusServName \n");
        return 2;
    }
    if(stat(pFilePath, &StatBuf) != 0)
    {
        printf("No find Dbus name file when Ipc_getDbusServName \n");
        return 3;
    }

    pStr = readArgValue(pFilePath, "DBUS_SESSION_BUS_ADDRESS", Buffer, sizeof(Buffer));
    if((pStr == NULL) || (strlen(Buffer) == 0))
    {
        printf("No find DBUS_SESSION_BUS_ADDRESS when Ipc_getDbusServName \n");
        return 4;
    }
    pStr1 = strchr(pStr, ',');
    if(pStr1 == NULL)
    {
        printf("No find , when Ipc_getDbusServName \n");
        return 5;
    }
    *pStr1 = '\0';
    snprintf(pName, Size, "%s", pStr);
    return 0;
}
//-------------------------------------------------------------------------------------------------
void Ipc_initParam(_sDbusInfo *pInfo)
{
    if(pInfo == NULL)
        return;
    memset(pInfo, 0, sizeof(_sDbusInfo));
}
//-------------------------------------------------------------------------------------------------
u8_t Ipc_initDbus(_sDbusInfo *pInfo, char *pServFile, char *pDbusName, char *pObjPath, _DbusIfaceTable *pIfaceInfo)
{
    char ServName[MAX_IPC_NAME_STR];
    u32_t Idx;
    u32_t IfaceIdx;
    _DbusIfaceData *pIfaceData;

    if((pInfo == NULL) || (pServFile == NULL) ||(pDbusName == NULL) || (pObjPath == NULL) || (pIfaceInfo == NULL))
    {
        printf("Input param error when Ipc_initDbus \n");
        return 1;
    }

    if((strlen(pDbusName) == 0) || (strlen(pObjPath) == 0) || (strlen(pServFile) == 0))
    {
        printf("Set Dbus name, ObjPath or find service file fail when Ipc_initDbus\n");
        return 2;
    }

    Ipc_initParam(pInfo);
    snprintf(pInfo->DbusName, sizeof(pInfo->DbusName), "%s", pDbusName);
    snprintf(pInfo->ObjPath, sizeof(pInfo->ObjPath), "%s", pObjPath);
    if(Ipc_getDbusServName(ServName, sizeof(ServName), pServFile) != 0)
    {
        printf("Get dbus service name fail when Ipc_initDbus\n");
        return 3;
    }
    if(Ipc_initConnect(ServName, pInfo) != 0)
    {
        printf("Set init connect fail when Ipc_initDbus. Connect name:%s \n", ServName);
        return 4;
    }

    for(Idx=0; Idx<(pIfaceInfo->NUmber); Idx++)
    {
        pIfaceData = (_DbusIfaceData *)(pIfaceInfo->pTable + Idx);
        Ipc_setIface(&IfaceIdx, pInfo, pIfaceData->Type, pIfaceData->IfaceName, pIfaceData->String);
        if(pIfaceData->Type == DBUS_SIGNAL)
            Ipc_addSignalMatch(pInfo, IfaceIdx);
    }
    return 0;
}

