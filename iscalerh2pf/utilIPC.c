
#include "utilIPCAPI.h"
#include "utilDbgMsg.h"
#include "utilHostAPI.h"
#include "utilDatabaseAPI.h"
#include "appDataMgr.h"
#include "./IPC_DBUS/ipc_scaler.h"
#include "cmd_ap.h"


///////////////// callback function start ///////////////
sUTILIPC_CALLBACK sUtilIPC_Callback ;

void utilIPC_RegCallback(sUTILIPC_CALLBACK fpCallback)
{
    sUtilIPC_Callback = fpCallback;
}
////////////////// callback function end ////////////////


#pragma pack(push)  /* push current alignment to stack */
#pragma pack(1)     /* set alignment to 1 byte boundary */

#if 0
typedef struct
{
    UINT8   ucMagicNumber1;
    UINT8   ucMagicNumber2;
    UINT16  uiChecksum;
    UINT16  uiPacketSize;
    UINT16  uiCLInumber;
    UINT8   ucData[4096];
} sIPC_PACKET;
#endif

typedef _sPacketFormatV2 sIPC_PACKET ;

#pragma pack(pop)   /* restore original alignment from stack */

#define IPC_PAYLOAD_HEADER_SIZE     (5) //ucHeader + ucActionType + uiSize
#define IPC_SENDDATA_HEADER_SIZE 	(4+2) //type(1) + grayout(1) + datacode(2) + size(2)
#define IPC_PACKET_HEADER_SIZE 	    (OFFSET_V2_SUB_PACKET) //Packet Header 0x55 0x55 + Packet Checksum + uiPacketSize + uiCLInumber
#define IPC_PACKET_HEADER 	        (0xAA)

#define LAM_MAIN_CODE (8)
#define LAM_SUB_CODE  (15)

//=================================================================================================


#define IPC_MUTEX_WAIT (3000)

pthread_mutex_t xIPC_Mutex;

typedef struct
{
    UINT8 ucCode;
    char *psString;
} sACTION_MSG, *psACTION_MSG;

typedef struct
{
    UINT8 DataType;
    char *psString;
} sDATATYPE_MSG, *psDATATYPE_MSG;

const sACTION_MSG asAction_msg[] =
{
    {CMD_ACTION_SET_DATABASE_ITEM,          "ITEM_SET"},
    {CMD_ACTION_GET_DATABASE_ITEM,          "ITEM_GET"},
    {CMD_ACTION_SET_DATABASE_ITEM_ACK_DONE, "ITEM_ACKDONE_SET"},
    {CMD_ACTION_GET_DATABASE_ITEM_ACK_DONE, "ITEM_ACKDONE_GET"},
    {CMD_ACTION_SET_INFO_DATA,              "ITEM_INFO_SET"},
    {CMD_ACTION_GET_INFO_DATA,              "ITEM_INFO_GET"},
    {CMD_ACTION_SET_POWER_STATUS,           "POWER_STATE_SET"},
    {CMD_ACTION_GET_POWER_STATUS,           "POWER_STATE_GET"},
    {CMD_ACTION_SET_DATABASE_ITEM_UPDATE,   "ITEM_UPDATE_SET"},
    {CMD_ACTION_GET_DATABASE_ITEM_UPDATE,   "ITEM_UPDATE_GET"},
    {CMD_ACTION_SET_INFO_DATA_UPDATE,       "ITEM_INFO_UPDATE_SET"},
    {CMD_ACTION_GET_INFO_DATA_UPDATE,       "ITEM_INFO_UPDATE_GET"},
    {CMD_ACTION_SET_KEY_CODE,               "KEY_CODE_SET"},
    {CMD_ACTION_GET_KEY_CODE,               "KEY_CODE_GET"},
    {CMD_ACTION_SET_MSSC,                   "MSSC_SET"},
    {CMD_ACTION_GET_MSSC,                   "MSSC_GET"},
    {CMD_ACTION_SET_LOG,                    "SNMP_SET"},
    {CMD_ACTION_GET_LOG,                    "SNMP_GET"},
    {CMD_ACTION_SET_BLENDER,                "BLENDER_SET"},
    {CMD_ACTION_GET_BLENDER,                "BLENDER_GET"},
    {CMD_ACTION_SET_STRUCT,                 "STRUCT_SET"},	//G100_Coda_0016
    {CMD_ACTION_GET_STRUCT,                 "STRUCT_GET"},	//G100_Coda_0016
    {CMD_ACTION_NUMBER,                     "NA"},
};

const sACTION_MSG asDataType_msg[] =
{
    {DATA_TYPE_NA            ,"NA"},
    {DATA_TYPE_STRING        ,"STRING"},
    {DATA_TYPE_UI_DIGIT_8    ,"UINT8"},
    {DATA_TYPE_UI_DIGIT_16   ,"UINT16"},
    {DATA_TYPE_UI_DIGIT_32   ,"UINT32"},
    {DATA_TYPE_UI_DIGIT_64   ,"UINT64"},
    {DATA_TYPE_I_DIGIT_8     ,"INT8"},
    {DATA_TYPE_I_DIGIT_16    ,"INT16"},
    {DATA_TYPE_I_DIGIT_32    ,"INT32"},
    {DATA_TYPE_I_DIGIT_64    ,"INT64"},
    {DATA_TYPE_FLOAT         ,"FLOAT"},
    {DATA_TYPE_VOID          ,"VOID"},
    {DATA_TYPE_STRUCT        ,"STRUCT"},
    {DATA_TYPE_NUMBER        ,"NUMBER"},
};

_ipcCallBack pIpcFunc;

UINT16 utilIpc_Unpack(sIPC_PACKET *psIPCPacket, const UINT16 DataCount)
{
    UINT16 uiReturn = 0;

    sIPC_SEND_DATA  *psPayLoad = (sIPC_SEND_DATA*)&psIPCPacket->Value[DataCount];

    if(psPayLoad->Header == IPC_PACKET_HEADER)
    {
        if(psPayLoad->ActionType >= CMD_ACTION_NUMBER)
        {
            LOG_MSG(db_IPC_REV,"<IPC Fail A%d>\n", psPayLoad->ActionType);
            uiReturn = 0;
            return uiReturn;
        }


        if(psPayLoad->ActionType == CMD_ACTION_SET_DATABASE_ITEM ||
           psPayLoad->ActionType == CMD_ACTION_SET_DATABASE_ITEM_ACK_DONE)
        {
            if(psPayLoad->Data.ucType >= DATA_TYPE_NUMBER)
            {
                LOG_MSG(db_IPC_REV,"<IPC Fail A%d T%d>\n", psPayLoad->ActionType
                                                         , psPayLoad->Data.ucType);
                uiReturn = 0;
                return uiReturn;
            }

            LOG_MSG(db_IPC_REV,"<IPC A%d>Datacode Type Status Size = %d %s %d %d\n", psPayLoad->ActionType
                                                                                   , psPayLoad->Data.uiItemIndex  //datacode
                                                                                   , asDataType_msg[psPayLoad->Data.ucType].psString
                                                                                   , psPayLoad->Data.ucGrayoutStatus
                                                                                   , psPayLoad->Data.uiValueSize
                                                                                   );
        }

        if(palDataMgr_NotifyGroupingEvent(psPayLoad->Data) == eEXEC_CODE_PASS) // HICC2_Ashton_0040 // HICC2_Ashton_0042
        {
            LOG_MSG(db_IPC_REV,"<Group Notify> %d %d\n", psPayLoad->Data.uiItemIndex, psPayLoad->Data.uValue.lValue);
            // uiReturn = 0;
            // return uiReturn;
        }

        switch(psPayLoad->ActionType)
        {
            case CMD_ACTION_SET_DATABASE_ITEM:
                {
                    sPAYLOAD sInPayload;

                    if(psIPCPacket->Interface == CMD_INTERFACE_UI_PROCESS)
                    {
                        sInPayload.uModule.sBIT.ucModuleID = eCMD_MODULE_GUI;
                        sInPayload.uModule.sBIT.cRW = CMD_WRITE;

                        if(psIPCPacket->SaveDataWay == eSAVE_DATA_TO_RAM_NO_ACTION)
                        {
                            sInPayload.ucSubCmd = eGUI_MSG_DATA_NO_ACTIVE;
                        }
                        else
                        {
                            sInPayload.ucSubCmd = eGUI_MSG_DATA;
                        }
                    }
                    else
                    {
                        sInPayload.uModule.sBIT.ucModuleID = eCMD_MODULE_LAN;
                        sInPayload.uModule.sBIT.cRW = CMD_WRITE;
                        sInPayload.ucSubCmd = eLAN_MSG_DATA;
                    }

                    sLAN_DATA_FORMAT sLanDataFormat;    //H2PF_Simon_0067
                    sLanDataFormat.uiSendFrom = psIPCPacket->Interface;
                    memcpy(&sLanDataFormat.ucType, &psPayLoad->Data.ucType, psPayLoad->Size);
                    memcpy(&sInPayload.aucData, &sLanDataFormat, sizeof(sLAN_DATA_FORMAT));

                    utilHost_PayloadSend((UINT8*)&sInPayload);

                    uiReturn = IPC_PAYLOAD_HEADER_SIZE + psPayLoad->Size;
                }
                break;

            case CMD_ACTION_GET_DATABASE_ITEM:
                break;

            case CMD_ACTION_SET_DATABASE_ITEM_ACK_DONE:
                {
                    sPAYLOAD sInPayload;

                    if(psIPCPacket->Interface == CMD_INTERFACE_UI_PROCESS)
                    {
                        sInPayload.uModule.sBIT.ucModuleID = eCMD_MODULE_GUI;
                        sInPayload.uModule.sBIT.cRW = CMD_WRITE;

                        if(psIPCPacket->SaveDataWay == eSAVE_DATA_TO_RAM_NO_ACTION)
                        {
                            sInPayload.ucSubCmd = eGUI_MSG_NO_ACTIVE_ACK_DONE;
                        }
                        else
                        {
                            sInPayload.ucSubCmd = eGUI_MSG_ACK_DONE;
                        }
                    }
                    else
                    {
                        sInPayload.uModule.sBIT.ucModuleID = eCMD_MODULE_LAN;
                        sInPayload.uModule.sBIT.cRW = CMD_WRITE;
                        sInPayload.ucSubCmd = eLAN_MSG_ACK_DONE;
                    }

                    sLAN_DATA_FORMAT sLanDataFormat;  //H2PF_Simon_0067
                    sLanDataFormat.uiSendFrom = psIPCPacket->Interface;
                    memcpy(&sLanDataFormat.ucType, &psPayLoad->Data.ucType, psPayLoad->Size);
                    memcpy(&sInPayload.aucData, &sLanDataFormat, sizeof(sLAN_DATA_FORMAT));

                    utilHost_PayloadSend((UINT8*)&sInPayload);

                    uiReturn = IPC_PAYLOAD_HEADER_SIZE + psPayLoad->Size;
                }
                break;

            case CMD_ACTION_GET_DATABASE_ITEM_ACK_DONE:
                break;

            case CMD_ACTION_SET_INFO_DATA:
                break;

            case CMD_ACTION_GET_INFO_DATA:
                break;

            default:
                LOG_MSG(db_ALWAYS,"unknown action type %d\n", psPayLoad->ActionType);
                break;
        }
    }
    else
    {
        LOG_MSG(db_IPC_REV,"IPC_PACKET_HEADER Error\n");
    }

    return uiReturn;
}

void utilIpc_Receive(u8_t *pTrigData, u32_t *pSize, u8_t *pData, u32_t Size)
{
    sIPC_PACKET *psInputData = (sIPC_PACKET*)pData;
    //sIPC_SEND_DATA *psPayLoad;
    UINT16   RecCheckSum = 0;
    UINT16   Count = 0;
    UINT16   DataCount = 0;
    UINT16   uiCount = 0;
    UINT16   uiRecSize = 0;
    UINT16   uiDataSize = 0;

    if(psInputData != NULL && psInputData->Size > IPC_PACKET_HEADER_SIZE)   //G100_Simon_0060
    {
        uiDataSize = psInputData->Size - IPC_PACKET_HEADER_SIZE;
    }

    if((pTrigData == NULL) || (pSize == NULL) || (pData == NULL))
    {
        printf("Input param error when procMethodSetData\n");
        return;
    }

    LOG_MSG(db_IPC_REV,"\n %s\n", __FUNCTION__);
    LOG_MSG(db_IPC_REV,"\n>> RecvSize = %d\n", Size);
    LOG_MSG(db_IPC_REV,">> Header   = 0x%04X\n", psInputData->Header);
    LOG_MSG(db_IPC_REV,">> CheckSum = 0x%04X\n", psInputData->CheckSum);
    LOG_MSG(db_IPC_REV,">> Size     = 0x%04X\n", psInputData->Size);
    LOG_MSG(db_IPC_REV,">> Number   = 0x%04X\n", psInputData->Number);
    LOG_MSG(db_IPC_REV,">> Interface = 0x%04X\n", psInputData->Interface);

    if(psInputData->Header != 0x5655)
    {
        LOG_MSG(db_IPC_REV, "IPC Header Error\n");
        return;
    }

    UINT16 CalCheckSum;
    CmdAp_calcCheckSum(&CalCheckSum, ((UINT8 *)psInputData+OFFSET_LEN), (psInputData->Size - OFFSET_LEN));

    if(CalCheckSum == psInputData->CheckSum)
    {
        DataCount = 0;

        for(Count = 0; (Count < psInputData->Number) && (DataCount < uiDataSize); Count++)
        {
            //psPayLoad = (sIPC_SEND_DATA*)&psInputData->Value[DataCount];

            uiRecSize = utilIpc_Unpack(psInputData, DataCount);

            LOG_MSG(db_IPC_REV,">> uiRecSize  = 0x%04X\n", uiRecSize);

            if(uiRecSize != 0)
            {
                DataCount = DataCount + uiRecSize;
            }

            LOG_MSG(db_IPC_REV,">> next data start  = %d\n", DataCount);
        }
    }
    else
    {
        LOG_MSG(db_ALWAYS,">>IPC Check Sum Fail\n");
    }

    *pSize = sprintf(pTrigData, "Set OK \n") + 1;
}


void utilIpc_ReceiveAckDone(u8_t *pTrigData, u32_t *pSize, u8_t *pData, u32_t Size)
{
    sIPC_PACKET     *psInputData = (sIPC_PACKET*)pData;
    //sIPC_SEND_DATA  *psPayLoad;
    UINT16        RecCheckSum = 0;
    UINT16        Count = 0;
    UINT16   DataCount = 0;
    UINT16   uiRecSize = 0;
    UINT16   uiDataSize = 0;

    if(psInputData != NULL && psInputData->Size > IPC_PACKET_HEADER_SIZE)   //G100_Simon_0060
    {
        uiDataSize = psInputData->Size - IPC_PACKET_HEADER_SIZE;
    }

    if((pTrigData == NULL) || (pSize == NULL) || (pData == NULL))
    {
        printf("Input param error when procMethodSetData\n");
        return;
    }

    LOG_MSG(db_IPC_REV,"\n %s\n", __FUNCTION__);
    LOG_MSG(db_IPC_REV,"\n>>> RecvSize = %d\n", Size);
    LOG_MSG(db_IPC_REV,">> Header   = 0x%04X\n", psInputData->Header);
    LOG_MSG(db_IPC_REV,">> CheckSum = 0x%04X\n", psInputData->CheckSum);
    LOG_MSG(db_IPC_REV,">> Size     = %d\n", psInputData->Size);
    LOG_MSG(db_IPC_REV,">> Number   = %d\n", psInputData->Number);
    LOG_MSG(db_IPC_REV,">> Interface = 0x%04X\n", psInputData->Interface);

    if(psInputData->Header != 0x5655)
    {
        LOG_MSG(db_IPC_REV, "IPC Header Error\n");
        return;
    }

    UINT16 CalCheckSum;
    CmdAp_calcCheckSum(&CalCheckSum, ((UINT8 *)psInputData+OFFSET_LEN), (psInputData->Size - OFFSET_LEN));

    if(CalCheckSum == psInputData->CheckSum)
    {
        DataCount = 0;

        for(Count = 0; (Count < psInputData->Number) && (DataCount < uiDataSize); Count++)
        {
            //psPayLoad = (sIPC_SEND_DATA*)&psInputData->Value[DataCount];

            uiRecSize = utilIpc_Unpack(psInputData, DataCount);

            if(uiRecSize != 0)
            {
                DataCount = DataCount + uiRecSize;
            }
        }
    }
    else
    {
        LOG_MSG(db_ALWAYS,">>IPC Check Sum Fail\n");
    }


    *pSize = sprintf(pTrigData, "Set OK \n") + 1;
}

/********************************************/
/* Custom */
/********************************************/

void utilIpc_BuildPacket(sIPC_PACKET *psPacket, UINT8 *pcData, UINT16 uiSize, UINT16 uiNumber, UINT8 ExecResult)
{
    UINT16  uiCount = 0;
    //UINT16  uiPacketSize = uiSize + IPC_PACKET_HEADER_SIZE;  //13 + 28

    _sPacketV2Info PacketInfo;
    CmdAp_initPacketV2Info(&PacketInfo,
                           HEADER_PACKET_VER_V2,
                           CMD_INTERFACE_ISCALER,
                           eSAVE_DATA_TO_EMMC_WITH_ACTION,
                           ExecResult);

    sIPC_SEND_DATA *sSubPacket = (sIPC_SEND_DATA *)pcData;

    _sSubPacketInfo sSubPacketInfo = {
                                        sSubPacket->ActionType,
                                        sSubPacket->Size
                                     };
    _sDataBaseInfo  sDataBaseInfo =  {
                                        sSubPacket->Data.ucType,
                                        sSubPacket->Data.ucGrayoutStatus,
                                        sSubPacket->Data.uiItemIndex,
                                        sSubPacket->Data.uiValueSize
                                     };

    CmdAp_buildV2SendSingleData((u8_t *)psPacket, &PacketInfo, &sSubPacketInfo, &sDataBaseInfo, (u8_t *)sSubPacket->Data.uValue.acString);

#if 0
    sIPC_SEND_DATA *sSubPacket = (sIPC_SEND_DATA *)pcData;
    _sSubPacketConcat SubPacket[1];
    _sPacketV2Info PacketInfoV2;

    CmdAp_initPacketV2Info(&PacketInfoV2,
                           HEADER_PACKET_VER_V2,
                           CMD_INTERFACE_ISCALER,
                           eSAVE_DATA_TO_EMMC_WITH_ACTION);

    CmdAp_buildSubPacketConcat(&SubPacket[0],
                               NULL,
                               CMD_ACTION_SET_DATABASE_ITEM_ACK_DONE,
                               sSubPacket->Data.ucType,
                               sSubPacket->Data.ucGrayoutStatus,
                               sSubPacket->Data.uiItemIndex,
                               &sSubPacket->Data,
                               sizeof(INT32));

    _sSubPacketInfo sSubPacketInfo = {
                                        sSubPacket->ActionType,
                                        sSubPacket->Size
                                     };
    _sDataBaseInfo  sDataBaseInfo  = {
                                        sSubPacket->Data.ucType,
                                        sSubPacket->Data.ucGrayoutStatus,
                                        sSubPacket->Data.uiItemIndex
                                     };

    CmdAp_buildV2SendData(psPacket, &sSubPacketInfo, &sDataBaseInfo, sSubPacket->Data);
#endif
}


INT8 utilIpc_BuildUpdateItemPacket(sIPC_PACKET *psPacket, UINT8 *pcData, UINT16 uiSize, UINT16 uiDatacodeNumber)
{
    UINT16  uiCount = 0;
    //UINT16  uiPacketSize = uiSize + IPC_PACKET_HEADER_SIZE;  //13 + 28

    _sPacketV2Info PacketInfo;
    CmdAp_initPacketV2Info(&PacketInfo,
                           HEADER_PACKET_VER_V2,
                           CMD_INTERFACE_ISCALER,
                           eSAVE_DATA_TO_EMMC_WITH_ACTION,
                           eEXEC_CODE_PASS);

    sIPC_SEND_DATA *sSubPacket = (sIPC_SEND_DATA *)pcData;

    _sSubPacketInfo sSubPacketInfo = {
                                        sSubPacket->ActionType,
                                        sSubPacket->Size
                                     };
    _sDataBaseInfo sDataBaseInfo = {
                                        sSubPacket->Data.ucType,
                                        sSubPacket->Data.ucGrayoutStatus,
                                        sSubPacket->Data.uiItemIndex,
                                        sSubPacket->Data.uiValueSize,
                                     };

    //printf("AT = %d\n", sSubPacketInfo.ActionType);
    //printf("Size = %d\n", sSubPacketInfo.Size);
    //printf("Type = %d\n", sDataBaseInfo.Type);
    //printf("Status = %d\n", sDataBaseInfo.Status);
    //printf("Id = %d\n", sDataBaseInfo.Id);

    if((psPacket == NULL) || (sSubPacket == NULL))
    {
        printf("Input param error %d", __LINE__);
        return 1;
    }

    if(CmdAp_initPacketV2((UINT8 *)psPacket, &PacketInfo) != 0)
        return 2;

    if(CmdAp_buildV2SendData((UINT8 *)psPacket, &sSubPacketInfo, &sDataBaseInfo, (u8_t *)sSubPacket->Data.uValue.acString) != 0)
        return 3;

    psPacket->Number = uiDatacodeNumber;  //need update datacode total number

    if(CmdAp_addPacketV2CheckSum((UINT8 *)psPacket) != 0)
        return 4;

    return 0;

}

void utilIpc_BuildSendData(sIPC_SEND_DATA *psPacket, UINT16 uiDataCode, UINT8 ucStatus, UINT8 ucType , UINT16 uiSize, UINT8 *pcData)
{
    psPacket->Header      = IPC_PACKET_HEADER;
    psPacket->ActionType  = CMD_ACTION_SET_DATABASE_ITEM;
    psPacket->Size        = uiSize; //+ IPC_SENDDATA_HEADER_SIZE;
    psPacket->Data.ucType           = ucType;
    psPacket->Data.ucGrayoutStatus  = ucStatus;
    psPacket->Data.uiItemIndex      = uiDataCode;
    psPacket->Data.uiValueSize      = uiSize; //sUtilIPC_Callback.fpDataMgr_DataSizeGetCb(uiDataCode);

    if(uiSize > sizeof(psPacket->Data.uValue.acString))
    {
        LOG_MSG(db_ALWAYS, "(func:%s, line:%d) size error (%d)\n", __FUNCTION__, __LINE__, uiSize);
        uiSize = sizeof(psPacket->Data.uValue.acString);
    }

    memcpy(psPacket->Data.uValue.acString , pcData, uiSize);
}

void utilIpc_BuildSendDataAck(sIPC_SEND_DATA *psPacket, UINT16 uiDataCode, UINT8 ucStatus, UINT8 ucType , UINT16 uiSize, UINT8 *pcData)
{
    psPacket->Header      = IPC_PACKET_HEADER;
    psPacket->ActionType  = CMD_ACTION_SET_DATABASE_ITEM_ACK_DONE;
    psPacket->Size        = uiSize; //+ IPC_SENDDATA_HEADER_SIZE;
    psPacket->Data.ucType           = ucType;
    psPacket->Data.ucGrayoutStatus  = ucStatus;
    psPacket->Data.uiItemIndex      = uiDataCode;
    psPacket->Data.uiValueSize      = uiSize;// sUtilIPC_Callback.fpDataMgr_DataSizeGetCb(uiDataCode);

    if(uiSize > sizeof(psPacket->Data.uValue.acString))
    {
        LOG_MSG(db_ALWAYS, "(func:%s, line:%d) size error (%d)\n", __FUNCTION__, __LINE__, uiSize);
        uiSize = sizeof(psPacket->Data.uValue.acString);
    }

    memcpy(psPacket->Data.uValue.acString, pcData, uiSize);
}

void utilIpc_BuildTelnetReply(sIPC_SEND_DATA *psPacket , UINT16 uiSize, UINT8 *pcData)
{
    psPacket->Header      = IPC_PACKET_HEADER;
    psPacket->ActionType  = CMD_ACTION_SET_DATABASE_ITEM;
    psPacket->Size        = uiSize; // + IPC_SENDDATA_HEADER_SIZE;
    psPacket->Data.ucType           = DATA_TYPE_STRING;
    psPacket->Data.ucGrayoutStatus  = 0;
    psPacket->Data.uiItemIndex      = edcACTION_TELNET_CLI;  //reply
    psPacket->Data.uiValueSize      = uiSize;

    if(uiSize > sizeof(psPacket->Data.uValue.acString))
    {
        LOG_MSG(db_ALWAYS, "(func:%s, line:%d) size error (%d)\n", __FUNCTION__, __LINE__, uiSize);
        uiSize = sizeof(psPacket->Data.uValue.acString);
    }

    memcpy(psPacket->Data.uValue.acString, pcData, uiSize);
}

void utilIpc_BuildSNMP(sIPC_SEND_DATA *psPacket , UINT16 uiSize, UINT8 *pcData)
{
    psPacket->Header      = IPC_PACKET_HEADER;
    psPacket->ActionType  = CMD_ACTION_SET_DATABASE_ITEM;
    psPacket->Size        = uiSize + IPC_SENDDATA_HEADER_SIZE;
    psPacket->Data.ucType           = DATA_TYPE_STRUCT;
    psPacket->Data.ucGrayoutStatus  = 0;
    psPacket->Data.uiItemIndex      = edcACTION_LOG;  //reply
    psPacket->Data.uiValueSize      = uiSize;

    if(uiSize > sizeof(psPacket->Data.uValue.acString))
    {
        LOG_MSG(db_ALWAYS, "(func:%s, line:%d) size error (%d)\n", __FUNCTION__, __LINE__, uiSize);
        uiSize = sizeof(psPacket->Data.uValue.acString);
    }

    memcpy(psPacket->Data.uValue.acString, pcData, uiSize);
}

//gui data code id
void utilIpc_BuildUpdateDataCodeItem(sIPC_SEND_DATA *psPacket , UINT16 uiSize, UINT8 *pcData)
{
    psPacket->Header      = IPC_PACKET_HEADER;
    psPacket->ActionType  = CMD_ACTION_SET_DATABASE_ITEM_UPDATE;
    psPacket->Size        = uiSize ; //+ IPC_SENDDATA_HEADER_SIZE;
    psPacket->Data.ucType           = 0;
    psPacket->Data.ucGrayoutStatus  = 0;
    psPacket->Data.uiItemIndex      = 0;
    psPacket->Data.uiValueSize      = uiSize;

    if(uiSize > sizeof(psPacket->Data.uValue.acString))
    {
        LOG_MSG(db_ALWAYS, "(func:%s, line:%d) size error (%d)\n", __FUNCTION__, __LINE__, uiSize);
        uiSize = sizeof(psPacket->Data.uValue.acString);
    }

    memcpy(psPacket->Data.uValue.acString, pcData, uiSize);
}

//gui data code id
void utilIpc_BuildUpdateInfoItem(sIPC_SEND_DATA *psPacket , UINT16 uiSize, UINT8 *pcData)
{
    psPacket->Header      = IPC_PACKET_HEADER;
    psPacket->ActionType  = CMD_ACTION_SET_INFO_DATA_UPDATE;
    psPacket->Size        = uiSize ; //+ IPC_SENDDATA_HEADER_SIZE;
    psPacket->Data.ucType           = 0;
    psPacket->Data.ucGrayoutStatus  = 0;
    psPacket->Data.uiItemIndex      = 0;
    psPacket->Data.uiValueSize      = sizeof(UINT32);

    if(uiSize > sizeof(psPacket->Data.uValue.acString))
    {
        LOG_MSG(db_ALWAYS, "(func:%s, line:%d) size error (%d)\n", __FUNCTION__, __LINE__, uiSize);
        uiSize = sizeof(psPacket->Data.uValue.acString);
    }

    memcpy(psPacket->Data.uValue.acString, pcData, uiSize);
}

void utilIpc_BuildPowerState(sIPC_SEND_DATA *psPacket, UINT16 uiDataCode, UINT16 uiSize, UINT8 *pcData)
{
    psPacket->Header      = IPC_PACKET_HEADER;
    psPacket->ActionType  = CMD_ACTION_SET_DATABASE_ITEM;
    psPacket->Size        = uiSize ; //+ IPC_SENDDATA_HEADER_SIZE;
    psPacket->Data.ucType           = DATA_TYPE_STRUCT;
    psPacket->Data.ucGrayoutStatus  = 0;
    psPacket->Data.uiItemIndex      = edcPOWER_STATUS;
    psPacket->Data.uiValueSize      = uiSize;

    if(uiSize > sizeof(psPacket->Data.uValue.acString))
    {
        LOG_MSG(db_ALWAYS, "(func:%s, line:%d) size error (%d)\n", __FUNCTION__, __LINE__, uiSize);
        uiSize = sizeof(psPacket->Data.uValue.acString);
    }

    memcpy(psPacket->Data.uValue.acString, pcData, uiSize);
}

//G100_Coda_0016
void utilIpc_BuildStruct(sIPC_SEND_DATA *psPacket , UINT16 uiStructType, UINT16 uiSize, UINT8 *pcData)
{
    psPacket->Header      = IPC_PACKET_HEADER;
    psPacket->ActionType  = CMD_ACTION_SET_DATABASE_ITEM;
    psPacket->Size        = uiSize ; //+ IPC_SENDDATA_HEADER_SIZE;
    psPacket->Data.ucType           = DATA_TYPE_STRUCT;
    psPacket->Data.ucGrayoutStatus  = 0;
    psPacket->Data.uiItemIndex      = uiStructType;
    psPacket->Data.uiValueSize      = sUtilIPC_Callback.fpDataMgr_DataSizeGetCb(psPacket->Data.uiItemIndex);

    if(uiSize > sizeof(psPacket->Data.uValue.acString))
    {
        LOG_MSG(db_ALWAYS, "(func:%s, line:%d) size error (%d)\n", __FUNCTION__, __LINE__, uiSize);
        uiSize = sizeof(psPacket->Data.uValue.acString);
    }

    memcpy(psPacket->Data.uValue.acString, pcData, uiSize);
}


void utilIpc_BuildV2SubPacketConcat(_sSubPacketConcat *psSubPacket , UINT16 uiStructType, UINT16 uiSize, UINT8 *pcData)
{
#if 0
    _sSubPacketConcat SubPacket[1];
    _sPacketV2Info PacketInfoV2;
    CmdAp_initPacketV2Info(&PacketInfoV2,
                           HEADER_PACKET_VER_V2,
                           CMD_INTERFACE_AP_TEST,
                           eSAVE_DATA_TO_EMMC_WITH_ACTION);

    CmdAp_buildSubPacketConcat(psSubPacket,
                               NULL,
                               CMD_ACTION_SET_DATABASE_ITEM_ACK_DONE,
                               DATA_TYPE_I_DIGIT_32,
                               0,
                               uiDataCode,
                               &iValue,
                               sizeof(INT32));

    App_sendV2Cmd(&SubPacket[0], SEND_ACK_DONE, &PacketInfoV2);
#endif
}


//send data to CLI AP
eEXEC_CODE utilIpc_Send(void *SendData, UINT16 uiSize, UINT16 uiNumber)
{
#if (SYSTEM_OS_TYPE == POSIX_COMPLIANT)
    struct timespec s_timeout;
    sIPC_SEND_DATA *psSendData;
    UINT16 Count = 0;
    UINT16 DataCount = 0;
    UINT16  cDataCount = 0;	 //G100_Coda_0021

    clock_gettime(CLOCK_REALTIME, &s_timeout);
    util_TimespecAddms(&s_timeout, IPC_MUTEX_WAIT);
    if(pthread_mutex_timedlock(&xIPC_Mutex, &s_timeout) != 0)
    {
        ASSERT_ALWAYS();
        return eEXEC_CODE_FAIL;
    }
    else
    {
        sIPC_PACKET sPacket = {0};

        utilIpc_BuildPacket(&sPacket, (UINT8*)SendData, uiSize, uiNumber, eEXEC_CODE_PASS);

        LOG_MSG(db_IPC,"\n %s\n", __FUNCTION__);
        LOG_MSG(db_IPC,">> Header   = 0x%04X\n", sPacket.Header);
        LOG_MSG(db_IPC,">> CheckSum = 0x%04X\n", sPacket.CheckSum);
        LOG_MSG(db_IPC,">> Size     = %d\n", sPacket.Size);
        LOG_MSG(db_IPC,">> Number   = %d\n", sPacket.Number);

        for(Count = 0; Count < sPacket.Number; Count++)
        {
            psSendData = (sIPC_SEND_DATA*)&sPacket.Value[DataCount];

            LOG_MSG(db_IPC,">>Header      = 0x%02X\n", psSendData->Header);
            LOG_MSG(db_IPC,">>ActionType  = %d [%s]\n", psSendData->ActionType, asAction_msg[psSendData->ActionType].psString);
            LOG_MSG(db_IPC,">>Size        = %d\n", psSendData->Size);
            LOG_MSG(db_IPC,">>Type        = %d\n", psSendData->Data.ucType);
            LOG_MSG(db_IPC,">>Status      = %d\n", psSendData->Data.ucGrayoutStatus);
            LOG_MSG(db_IPC,">>DataCode    = %d\n", psSendData->Data.uiItemIndex);
            LOG_MSG(db_IPC,">>DataSize    = %d\n", psSendData->Data.uiValueSize);

            for(cDataCount = 0; cDataCount < (psSendData->Size - IPC_SENDDATA_HEADER_SIZE); cDataCount++)
            {
                LOG_MSG(db_IPC,">>Data[%d]    = 0x%02X\n", cDataCount, psSendData->Data.uValue.acString[cDataCount]);
            }
            DataCount = DataCount + psSendData->Size;
        }

        Scaler_sendMethodSetData((void*)&sPacket, sPacket.Size);

        if(pthread_mutex_unlock(&xIPC_Mutex) != 0)
        {
            ASSERT_ALWAYS();
            return eEXEC_CODE_FAIL;
        }
    }
#else
#endif
    return eEXEC_CODE_PASS;
}

eEXEC_CODE utilIpc_SendAckdone(void *SendData, UINT16 uiSize, UINT16 uiNumber, UINT8 ucExecResult)
{
#if (SYSTEM_OS_TYPE == POSIX_COMPLIANT)
    struct timespec s_timeout;
    sIPC_SEND_DATA *psSendData;
    UINT16 Count = 0;
    UINT16 DataCount = 0;

    clock_gettime(CLOCK_REALTIME, &s_timeout);
    util_TimespecAddms(&s_timeout, IPC_MUTEX_WAIT);
    if(pthread_mutex_timedlock(&xIPC_Mutex, &s_timeout) != 0)
    {
        ASSERT_ALWAYS();
        return eEXEC_CODE_FAIL;
    }
    else
    {
        sIPC_PACKET sPacket = {0};

        utilIpc_BuildPacket(&sPacket, (UINT8*)SendData, uiSize, uiNumber, ucExecResult);

        LOG_MSG(db_IPC,"\n %s\n", __FUNCTION__);

        LOG_MSG(db_IPC,">> Header   = 0x%04X\n", sPacket.Header);
        LOG_MSG(db_IPC,">> CheckSum = 0x%04X\n", sPacket.CheckSum);
        LOG_MSG(db_IPC,">> Size     = %d\n",     sPacket.Size);
        LOG_MSG(db_IPC,">> Exec Ret = %d\n",     sPacket.ExecResult);
        LOG_MSG(db_IPC,">> Number   = %d\n",     sPacket.Number);

        for(Count = 0; Count < sPacket.Number; Count++)
        {
            psSendData = (sIPC_SEND_DATA*)&sPacket.Value[DataCount];

            LOG_MSG(db_IPC," >>Header      = 0x%02X\n",  psSendData->Header);
            LOG_MSG(db_IPC," >>ActionType  = %d [%s]\n", psSendData->ActionType, asAction_msg[psSendData->ActionType].psString);
            LOG_MSG(db_IPC," >>Size        = %d\n",      psSendData->Size);
            LOG_MSG(db_IPC," >>Type        = %d\n",  psSendData->Data.ucType);
            LOG_MSG(db_IPC," >>Status      = %d\n",  psSendData->Data.ucGrayoutStatus);
            LOG_MSG(db_IPC," >>DataCode    = %d\n",  psSendData->Data.uiItemIndex);
            LOG_MSG(db_IPC," >>DataSize    = %d\n",  psSendData->Data.uiValueSize);
            LOG_MSG(db_IPC," >>Data        = 0x%02X\n",  psSendData->Data.uValue.acString[0]);

            DataCount = DataCount + psSendData->Size;
        }

        Scaler_sendMethodAckdoneData((void*)&sPacket, sPacket.Size);

        if(pthread_mutex_unlock(&xIPC_Mutex) != 0)
        {
            ASSERT_ALWAYS();

            return eEXEC_CODE_FAIL;
        }
    }
#else
#endif
    return eEXEC_CODE_PASS;

}

eEXEC_CODE utilIpc_SendUpdateItem(void *pSubPacket, UINT16 uiSize, UINT16 uiNumber)
{
#if (SYSTEM_OS_TYPE == POSIX_COMPLIANT)
    struct timespec s_timeout;
    sIPC_SEND_DATA *psSendData;
    UINT16 Count = 0;
    UINT16 DataCount = 0;
    UINT8 cResult = 0;

    clock_gettime(CLOCK_REALTIME, &s_timeout);
    util_TimespecAddms(&s_timeout, IPC_MUTEX_WAIT);
    if(pthread_mutex_timedlock(&xIPC_Mutex, &s_timeout) != 0)
    {
        ASSERT_ALWAYS();
        return eEXEC_CODE_FAIL;
    }
    else
    {
        sIPC_PACKET sPacket = {0};
        INT8 Ret = utilIpc_BuildUpdateItemPacket(&sPacket, (UINT8*)pSubPacket, uiSize, uiNumber);
        //utilIpc_BuildPacket(&sPacket, (UINT8*)pSubPacket, uiSize, uiNumber);

        //sPacket.Number = uiNumber; //total datacode number need to update

        LOG_MSG(db_IPC,"\n %s\n", __FUNCTION__);

        LOG_MSG(db_IPC,">> Header   = 0x%04X\n", sPacket.Header);
        LOG_MSG(db_IPC,">> CheckSum = 0x%04X\n", sPacket.CheckSum);
        LOG_MSG(db_IPC,">> Size     = %d\n",     sPacket.Size);
        LOG_MSG(db_IPC,">> Number   = %d\n",     sPacket.Number);   //total datacode number need to update

        psSendData = (sIPC_SEND_DATA*)&sPacket.Value[DataCount];

        LOG_MSG(db_IPC," >>Header      = 0x%02X\n",  psSendData->Header);
        LOG_MSG(db_IPC," >>ActionType  = %d [%s]\n", psSendData->ActionType, asAction_msg[psSendData->ActionType].psString);
        LOG_MSG(db_IPC," >>Size        = %d\n",      psSendData->Size);
        LOG_MSG(db_IPC," >>Type        = %d\n",  psSendData->Data.ucType);
        LOG_MSG(db_IPC," >>Status      = %d\n",  psSendData->Data.ucGrayoutStatus);
        LOG_MSG(db_IPC," >>DataCode    = %d\n",  psSendData->Data.uiItemIndex);
        LOG_MSG(db_IPC," >>DataSize    = %d\n",  psSendData->Data.uiValueSize);

        for(Count = 0; Count < psSendData->Size - 4 ; Count++)
        {
            LOG_MSG(db_IPC,">>[%03d]uiData    = 0x%02X\n", Count, psSendData->Data.uValue.acString[Count]);
        }

        cResult = Scaler_sendMethodSetData((void*)&sPacket, sPacket.Size);

        if((pthread_mutex_unlock(&xIPC_Mutex) != 0) || (cResult != 0))
        {
            ASSERT_ALWAYS();

            return eEXEC_CODE_FAIL;
        }
    }
#else
#endif
    return eEXEC_CODE_PASS;

}

eEXEC_CODE utilIpc_Init(void)
{
#if (SYSTEM_OS_TYPE == POSIX_COMPLIANT)

    Scaler_initCb(&pIpcFunc);

    pIpcFunc.MethodSetCb = utilIpc_Receive;
    pIpcFunc.MethodSetAckdoneCb = utilIpc_ReceiveAckDone;

    utilHost_IPC_Init();

    Scaler_initIpc(&pIpcFunc);

    if(pthread_mutex_init(&xIPC_Mutex, NULL) == -1)
    {
        ASSERT_ALWAYS();

        return eEXEC_CODE_FAIL;
    }

    return eEXEC_CODE_PASS;

#endif

    //Ipc_closeConnect(&Info);
}

INT8 utilIpc_SendData(eIPC_SEND_DATA eIPCData, UINT16 uiCmdID, void *vData, UINT8 ExecResult, UINT16 uiSendDataSize)
{
    eRESULT eRet = rcSUCCESS ;
    UINT16 uiDataSize = 0 ;
    UINT8 ucType;
    UINT8 ucEnable;
    INT32 iData;

    if(utilHost_IPC_Ready() != TRUE)
    {
        LOG_MSG(db_APP_LAN, "IPC not ready (%d %d)\n", eIPCData, uiCmdID);
        return rcERROR;
    }

    switch(eIPCData)
    {
        case eIPC_SEND_DATA_POWER_STATE:
           {
                sIPC_SEND_DATA  sPacket;
                UINT8           ucCount = 0;
                eEXEC_CODE      eExecCode;

                iData = (*(INT32 *)vData);

                uiDataSize = IPC_SEND_DATA_LEN + uiSendDataSize;

                utilIpc_BuildPowerState(&sPacket, uiCmdID, uiSendDataSize, vData);

                do
                {
                    eExecCode = utilIpc_Send((UINT8 *)&sPacket, uiDataSize, 1);

                    if(eExecCode != eEXEC_CODE_PASS)
                    {
                        MS_SLEEP(1000);
                    }
                }while((ucCount++ < 10) && (eExecCode != eEXEC_CODE_PASS));

                LOG_MSG(db_APP_LAN, "Send eNETWORK_DATA_POWER_STATE(%d)(%d)\r\n" , uiCmdID, iData);
                //while(1);
            }
            break;

        case eIPC_SEND_DATA_TELNET_REPLY:
            {
                sIPC_SEND_DATA sPacket;

                uiDataSize = IPC_SEND_DATA_LEN + uiSendDataSize;

                utilIpc_BuildTelnetReply(&sPacket, uiSendDataSize, vData);
                utilIpc_Send((UINT8 *)&sPacket, uiDataSize, 1);

                LOG_MSG(db_APP_LAN, "TELNET_REPLY %s\r\n", vData);
            }
            break;

        case eIPC_SEND_DATA_SNMP:  //T100_Simon_0039 //A70LV_Larry_0352 modify
            {
                sIPC_SEND_DATA sPacket;

                uiDataSize = IPC_SEND_DATA_LEN + uiSendDataSize;

                utilIpc_BuildSNMP(&sPacket, uiSendDataSize, vData);
                utilIpc_Send((UINT8 *)&sPacket, uiDataSize, 1);

                LOG_MSG(db_APP_LAN, "DATA_SNMP %s\r\n", vData);
            }
            break;

        case eIPC_SEND_DATA_SEND_DATACODE_WITH_DATA:
        case eIPC_SEND_DATA_ACK_DONE: //A70LV_Larry_0238
            {
                ucType   = (UINT8)sUtilIPC_Callback.fpDataMgr_Data_TypeCb((eDATA_CODE)uiCmdID);
                ucEnable = (UINT8)sUtilIPC_Callback.fpDataMgr_DataCode_ControlCb((eDATA_CODE)uiCmdID);

                switch(ucType)
                {
                    case DATA_TYPE_STRING:
                        {
                            LOG_MSG(db_APP_LAN, "Send Network Staring(%03d)(%s)\r\n" , uiCmdID, vData);

                            uiDataSize = IPC_SEND_DATA_LEN + uiSendDataSize;
                        }
                        break;

                    case DATA_TYPE_UI_DIGIT_8:
                    case DATA_TYPE_UI_DIGIT_16:
                    case DATA_TYPE_UI_DIGIT_32:
                    case DATA_TYPE_I_DIGIT_8:
                    case DATA_TYPE_I_DIGIT_16:
                    case DATA_TYPE_I_DIGIT_32:
                    case DATA_TYPE_FLOAT:
                        {
                            switch(ucType) //A70LV_Larry_0136
                            {
                                case DATA_TYPE_UI_DIGIT_8:
                                    {
                                        UINT8 Temp = (*(UINT8 *)vData);
                                        iData = (INT32)Temp;
                                    }
                                    break;
                                case DATA_TYPE_UI_DIGIT_16:
                                    {
                                        UINT16 Temp = (*(UINT16 *)vData);
                                        iData = (INT32)Temp;
                                    }
                                    break;
                                case DATA_TYPE_UI_DIGIT_32:
                                    {
                                        UINT32 Temp = (*(UINT32 *)vData);
                                        iData = (INT32)Temp;
                                    }
                                    break;
                                case DATA_TYPE_I_DIGIT_8:
                                    {
                                        INT8 Temp = (*(INT8 *)vData);
                                        iData = (INT32)Temp;
                                    }
                                    break;
                                case DATA_TYPE_I_DIGIT_16:
                                    {
                                        INT16 Temp = (*(INT16 *)vData);
                                        iData = (INT32)Temp;
                                    }
                                    break;
                                case DATA_TYPE_I_DIGIT_32:
                                    {
                                        INT32 Temp = (*(INT32 *)vData);
                                        iData = (INT32)Temp;
                                    }
                                    break;
                                case DATA_TYPE_FLOAT:
                                    {
                                        float Temp = (*(float *)vData);
                                        iData = (INT32)Temp;
                                    }
                                    break;

                                default:
                                    break;
                            }

                            uiSendDataSize = sizeof(INT32);

                            uiDataSize = IPC_SEND_DATA_LEN + sizeof(INT32);

                            LOG_MSG(db_APP_LAN, "Send Network Dec(%03d)(%d)\r\n" , uiCmdID, iData);

                        }
                        break;

                    case DATA_TYPE_STRUCT:
                        {
                            LOG_MSG(db_APP_LAN, "Send Network Structure (%d)\r\n" , uiCmdID);

                            uiSendDataSize = palDataMgr_DataSizeGet(uiCmdID);

                            uiDataSize = IPC_SEND_DATA_LEN + palDataMgr_DataSizeGet(uiCmdID);
                        }
                        break;

                    case DATA_TYPE_NA:
                    default:
                        break;
                }

                if(uiDataSize)
                {
                    if(eIPCData == eIPC_SEND_DATA_SEND_DATACODE_WITH_DATA)
                    {
                        sIPC_SEND_DATA sPacket;

                        utilIpc_BuildSendData(&sPacket,
                                              uiCmdID,
                                              ucEnable,
                                              ucType,
                                              uiSendDataSize,
                                              vData);

                        utilIpc_Send(&sPacket, uiDataSize, 1);
                    }
                    else  //ACK done
                    {
                        sIPC_SEND_DATA sSendData;

                        utilIpc_BuildSendDataAck(&sSendData,
                                                uiCmdID,
                                                ucEnable,
                                                ucType,
                                                uiSendDataSize,   //4+4
                                                vData);

                        utilIpc_SendAckdone(&sSendData, uiDataSize, 1, ExecResult);  //9+4
                    }
                }
            }
            break;

        case eIPC_SEND_DATA_LAN_APPLY:
            {
                ucType   = DATA_TYPE_STRING;
                ucEnable = 1;
                uiDataSize = IPC_SEND_DATA_LEN + uiSendDataSize;
                sIPC_SEND_DATA sPacket;

                utilIpc_BuildSendData(&sPacket,
                                      uiCmdID,
                                      ucEnable,
                                      ucType,
                                      uiSendDataSize,
                                      vData);

                utilIpc_Send((UINT8 *)&sPacket, uiDataSize, 1);

                LOG_MSG(db_APP_LAN, "LAN APPLY(%d)(%s)\r\n" , uiCmdID, vData);
            }
            break;

        case eIPC_SEND_DATA_UPDATE_DATACODE_ITEM:
            {
                if(uiSendDataSize/2 == 0)
                    break;

                sIPC_SEND_DATA sSubPacket;
                eEXEC_CODE eResult = eEXEC_CODE_FAIL;
                UINT8 cCount = 0;

                uiDataSize = IPC_SEND_DATA_LEN + uiSendDataSize;
                utilIpc_BuildUpdateDataCodeItem(&sSubPacket, uiSendDataSize, vData);

                for(cCount = 0; (cCount < 3) && (eResult == eEXEC_CODE_FAIL); cCount++)
                {
                    eResult = utilIpc_SendUpdateItem(&sSubPacket, uiDataSize, uiSendDataSize/2);

                    if(eResult == eEXEC_CODE_FAIL)
                    {
                        MS_SLEEP(1000);
                    }
                }
            }
            break;

        case eIPC_SEND_DATA_UPDATE_INFO_ITEM:
            {
                sIPC_SEND_DATA sPacket;

                //utilDatabase_WriteInformationData((UINT32)uiCmdID, DATA_TYPE_UI_DIGIT_16, 0, vData);    //A35G2_CDS_Simon_0011 remove

                uiDataSize = IPC_SEND_DATA_LEN + uiSendDataSize;
                utilIpc_BuildUpdateInfoItem(&sPacket, uiSendDataSize, vData);   //A35G2_CDS_Simon_0011
                utilIpc_SendUpdateItem(&sPacket, uiDataSize, uiSendDataSize/2);
            }
            break;

        case eIPC_SEND_DATA_LAN_RESET:
            {
                ucType   = DATA_TYPE_UI_DIGIT_8;
                ucEnable = 1;
                uiDataSize = IPC_SEND_DATA_LEN + uiSendDataSize;
                sIPC_SEND_DATA sPacket;

                utilIpc_BuildSendData(&sPacket,
                                      uiCmdID,
                                      ucEnable,
                                      ucType,
                                      uiSendDataSize,
                                      vData);

                utilIpc_Send((UINT8 *)&sPacket, uiDataSize, 1);

                LOG_MSG(db_APP_LAN, "LAN RESET\n");
            }
            break;

        case eIPC_SEND_DATA_STRUCT: //G100_Coda_0016
            {
                sIPC_SEND_DATA sPacket;
                UINT16 uiStructType;

                uiDataSize = IPC_SEND_DATA_LEN + uiSendDataSize;

                uiStructType = uiCmdID;

                utilIpc_BuildStruct(&sPacket, uiStructType, uiSendDataSize, vData);
                utilIpc_Send((UINT8 *)&sPacket, uiDataSize, 1);

                LOG_MSG(db_APP_LAN, "eIPC_SEND_DATA_STRUCT type = %d, size = %d\r\n", uiStructType, uiDataSize);
            }
            break;

        case eIPC_SEND_DATA_UI_EVENT:
            break;

        default:
            break;
    }

    return eRet ;
}

eRESULT utilIpc_NotifyStructureDatatypeDatacode(eDATA_CODE DataCode)
{
    eRESULT eRet = rcSUCCESS;
    eDATA_TYPE DataType = (eDATA_TYPE)palDataMgr_Data_Type(DataCode);

    if(DataType == DATA_TYPE_STRUCT)   //notify by set datacode IPC command
    {
        UINT8 Data[MAX_SIZE_DATABASE_DATA] = {0};
        palDataMgr_Data_Access(DataCode, edaREAD, Data);
        UINT16 DataSize = palDataMgr_DataSizeGet(DataCode);
        eRet = utilIpc_SendData(eIPC_SEND_DATA_SEND_DATACODE_WITH_DATA, DataCode, Data, eEXEC_CODE_PASS, DataSize);

        //LOG_MSG(db_ALWAYS, "Notify Struct datacode %d\n", DataCode);
    }
    else
    {
        eRet = rcERROR;
    }

    return eRet;
}

eEXEC_CODE utilIpc_NotifyDatacodeChanged(eDATA_CODE DataCode)
{
    eRESULT eRet = rcSUCCESS;

    //write to database
    palDataMgr_ImportDataToDatabase(DataCode);

    //get current data
    eDATA_TYPE DataType = (eDATA_TYPE)palDataMgr_Data_Type(DataCode);

    if(DataType == DATA_TYPE_STRUCT)   //notify by set datacode IPC command
    {
        eRet = utilIpc_NotifyStructureDatatypeDatacode(DataCode);
    }
    else    //notify by IPC command
    {
        eRet = utilIpc_SendData(eIPC_SEND_DATA_UPDATE_DATACODE_ITEM, 0, &DataCode, eEXEC_CODE_PASS, sizeof(UINT16)*1);
        //palLANProcSendToLAN((UINT16)DataCode);
    }

    if(eRet != rcSUCCESS)
    {
        LOG_MSG(db_ALWAYS, "!!! utilIpc_NotifyDatacodeChanged %d Fail\n", DataCode);
        return eEXEC_CODE_FAIL;
    }

    return eEXEC_CODE_PASS;
}

