#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <syslog.h>
#include <sys/file.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "cmd_ap.h"
#include "hpbulib_file_path.h"
#include "conf.h"

//=================================================================================================
//=================================================================================================
//=================================================================================================
u8_t CmdAp_getCmdInterfaceName(char *pName, u32_t MaxLen, _eInterface InterfaceIdx)
{
    char Interface[CMD_INTERFACE_NUMBER][16] = {NAME_CMD_INTERFACE_NONE,          NAME_CMD_INTERFACE_ISCALER,     NAME_CMD_INTERFACE_CLI,        NAME_CMD_INTERFACE_RS232,     NAME_CMD_INTERFACE_IR,
                                                NAME_CMD_INTERFACE_KEYPAD,        NAME_CMD_INTERFACE_TELNET,      NAME_CMD_INTERFACE_EXTRON,     NAME_CMD_INTERFACE_WEB,        NAME_CMD_INTERFACE_PJLINK,
                                                NAME_CMD_INTERFACE_SNMP,          NAME_CMD_INTERFACE_AMX,         NAME_CMD_INTERFACE_ART_NET,    NAME_CMD_INTERFACE_CRESTRON_1, NAME_CMD_INTERFACE_CRESTRON_2,
                                                NAME_CMD_INTERFACE_MGR_CAMERA,    NAME_CMD_INTERFACE_MGR_SERVER,  NAME_CMD_INTERFACE_MGR_CLIENT, NAME_CMD_INTERFACE_PJSEARCH,   NAME_CMD_INTERFACE_FOTA,
                                                NAME_CMD_INTERFACE_SMART_SERVICE, NAME_CMD_INTERFACE_MGR_LICENSE, NAME_CMD_INTERFACE_IOT_AGENT};
    char Idx;

    if((pName == NULL) || (MaxLen == 0) || (InterfaceIdx >=CMD_INTERFACE_NUMBER))
    {
        HICC_PRINTF("Input param fail");
        return 1;
    }

    for(Idx=0; Idx<CMD_INTERFACE_NUMBER; Idx++)
    {
        snprintf(pName, MaxLen, "%s", Interface[Idx]);
        return 0;
    }
    HICC_PRINTF("No find cmd interface name");
    return 2;
}
//-------------------------------------------------------------------------------------------------
u8_t CmdAp_calcCheckSum(u16_t *pCheckSum, u8_t *pData, u32_t Size)
{
    u32_t Idx;

    if((pCheckSum == NULL) || (pData == NULL))
    {
        printf("Input param error when CmdAp_calcCheckSum \n");
        return 1;
    }

    *pCheckSum = 0;
    for(Idx=0; Idx<Size; Idx++)
    {
        *pCheckSum += *(pData+Idx);
  //      printf("Check sum Idx:%d Sum:0x%04X Data:0x%02X \n ", Idx, *pCheckSum, *(pData+Idx));
    }
    return 0;
}
//-------------------------------------------------------------------------------------------------
u8_t CmdAp_analyzePacket(u16_t *pPacketSize, u16_t *pNumber, u8_t **pPayload, u8_t *pData)
{
    _sPacketFormat *pPacketFormat;
    u32_t Idx;
    u16_t CheckSum = 0;
    char DebugStr[128];

    if((pNumber == NULL) || (pPayload == NULL) || (pData == NULL) || (pPacketSize == NULL))
    {
        HICC_PRINTF("Input param error");
        return 1;
    }

    pPacketFormat = (_sPacketFormat *)pData;
    if(pPacketFormat->Header != HEADER_PACKET)
    {
        snprintf(DebugStr, sizeof(DebugStr), "Check Packet header fail. Header: 0x%02X 0x%02X", *pData, *(pData+1));
        HICC_PRINTF(DebugStr);
        return 2;
    }

    CmdAp_calcCheckSum(&CheckSum, (pData+OFFSET_LEN), (pPacketFormat->Size - OFFSET_LEN));

    if(CheckSum != pPacketFormat->CheckSum)
    {
        printf("Check packet checksum fail.Packet:0x%02X Get:0x%02X when CmdAp_analyzePacket \n", CheckSum, pPacketFormat->CheckSum);
        CmdAp_shoeDebugData("Check sum fail", pData, pPacketFormat->Size);
        return 3;
    }

    *pNumber     = pPacketFormat->Number;
    *pPayload    = pPacketFormat->Value;
    *pPacketSize = pPacketFormat->Size;
    return 0;
}
//-------------------------------------------------------------------------------------------------
u8_t CmdAp_analyzePacketV2(_sPacketV2Info *pInfo, u8_t **pPayload,  u8_t *pData)
{
    _sPacketFormat *pPacketFormat;
    _sPacketFormatV2 *pPacketFormat2;
    u32_t Idx;
    u16_t CheckSum = 0;
    char DebugStr[128];
    u16_t Ver, PacketSize, PacketCheckSum;

    if((pInfo == NULL) || (pData == NULL))
    {
        HICC_PRINTF("Input param error");
        return 1;
    }

    if(*pData != HEADER_PACKET_1)
    {
        snprintf(DebugStr, sizeof(DebugStr), "Check Packet header fail. Header: 0x%02X", *pData);
        HICC_PRINTF(DebugStr);
        return 2;
    }
    if(*(pData + 1) == HEADER_PACKET_V1_2)
    {
        pPacketFormat  = (_sPacketFormat *)pData;
        PacketSize     = pPacketFormat->Size;
        PacketCheckSum = pPacketFormat->CheckSum;
        Ver = 1;
    }
    else if(*(pData + 1) == HEADER_PACKET_V2_2)
    {
        pPacketFormat2 = (_sPacketFormatV2 *)pData;
        PacketSize     = pPacketFormat2->Size;
        PacketCheckSum = pPacketFormat2->CheckSum;
        Ver            = pPacketFormat2->Version;
    }
    else
    {
        snprintf(DebugStr, sizeof(DebugStr), "Check Packet header fail. Header: 0x%02X", *(pData+1));
        HICC_PRINTF(DebugStr);
        return 3;
    }

    CmdAp_calcCheckSum(&CheckSum, (pData+OFFSET_LEN), (PacketSize - OFFSET_LEN));

    if(CheckSum != PacketCheckSum)
    {
        printf("Check packet checksum fail.Packet:0x%02X Get:0x%02X when CmdAp_analyzePacket \n", CheckSum, pPacketFormat->CheckSum);
        CmdAp_shoeDebugData("Check sum fail", pData, pPacketFormat->Size);
        return 3;
    }

    pInfo->Size     = PacketSize;
    pInfo->Version  = Ver;

    if(Ver == 1)
    {
        pInfo->Interface   = CMD_INTERFACE_NONE;
        pInfo->SaveDataWay = eSAVE_DATA_NONE;
        pInfo->TotalNum    = pPacketFormat->Number;
        pInfo->ExecResult  = eEXEC_CODE_PASS;
        *pPayload          = pPacketFormat->Value;
    }
    else
    {
        pInfo->Interface   = pPacketFormat2->Interface;
        pInfo->SaveDataWay = pPacketFormat2->SaveDataWay;
        pInfo->TotalNum    = pPacketFormat2->Number;
        pInfo->ExecResult  = pPacketFormat2->ExecResult;
        *pPayload          = pPacketFormat2->Value;
    }

    return 0;
}
//-------------------------------------------------------------------------------------------------
u16_t CmdAp_getProtocolVersion(u8_t *pData)
{
    char DebugStr[128];
    u16_t Version;
    _sPacketFormatV2 *pPacketFormat2;

    if(pData == NULL)
    {
        HICC_PRINTF("Input param error");
        return 0;
    }

    if(*pData != HEADER_PACKET_1)
    {
        snprintf(DebugStr, sizeof(DebugStr), "Check header fail. Data 0x%02X ", *pData);
        HICC_PRINTF(DebugStr);
        return 0;
    }
    if(*(pData+1) == HEADER_PACKET_V1_2)
        Version = HEADER_PACKET_VER_V1;
    else if(*(pData+1) == HEADER_PACKET_V2_2)
    {
        pPacketFormat2 = (_sPacketFormatV2 *)pData;
        Version = pPacketFormat2->Version;
    }
    else
    {
        snprintf(DebugStr, sizeof(DebugStr), "Check header fail. Data 0x%02X ", *(pData+1));
        HICC_PRINTF(DebugStr);
        return 0;
    }
    return Version;
}
//-------------------------------------------------------------------------------------------------
u16_t CmdAp_getPacketV2Size(u8_t *pData)
{
    char DebugStr[128];
    _sPacketFormat  *pPacketV1;
    _sPacketFormatV2 *pPacketV2;


    if(pData == NULL)
    {
        HICC_PRINTF("Input param error");
        return 0;
    }

    if(*pData != HEADER_PACKET_1)
    {
        snprintf(DebugStr, sizeof(DebugStr), "Check packet header fail. Data 0x%02X", *pData);
        HICC_PRINTF(DebugStr);
        return 0;
    }

    if(*(pData + 1) == HEADER_PACKET_V1_2)
    {
        pPacketV1 = (_sPacketFormat *)pData;
        return pPacketV1->Size;
    }
    else if(*(pData + 1) == HEADER_PACKET_V2_2)
    {
        pPacketV2 = (_sPacketFormatV2 *)pData;
        return pPacketV2->Size;
    }
    else
    {
        snprintf(DebugStr, sizeof(DebugStr), "Check packet header fail. Data 0x%02X", *(pData+1));
        HICC_PRINTF(DebugStr);
        return 0;
    }
}
//-------------------------------------------------------------------------------------------------
u8_t CmdAp_getSubPacketData(sDATABASE_ITEM_DATA_FORMAT **pTrigData, u8_t *pData, u16_t NumberIdx)
{
    u16_t Idx, SubPackSize;
    u32_t Offset = 0;

    if((pTrigData == NULL) || (pData == NULL))
    {
        HICC_PRINTF("Input param error");
        return 1;
    }

    for(Idx=0; Idx<NumberIdx; Idx++)
    {
        SubPackSize = *(u16_t *)(pData + Offset + OFFSET_SUB_DATA_SIZE);
        Offset += SubPackSize + OFFSET_SUB_DATA;
    }
    if(*(pData+Offset) != HEADER_SUBPACKET)
    {
        HICC_PRINTF("No find sub packet");
        return 2;
    }

    *pTrigData = (sDATABASE_ITEM_DATA_FORMAT *)(pData + Offset + OFFSET_SUB_DATA);
    return 0;
}
//-------------------------------------------------------------------------------------------------
u8_t CmdAp_getSubPacket(_sSubPacketFormat **pSubPacket, u8_t *pData, u16_t NumberIdx)
{
    u16_t Idx, SubPackSize;
    u32_t Offset = 0;

    if((pSubPacket == NULL) || (pData == NULL))
    {
        HICC_PRINTF("Input param error");
        return 1;
    }

    for(Idx=0; Idx<NumberIdx; Idx++)
    {
        SubPackSize = *(u16_t *)(pData + Offset + OFFSET_SUB_DATA_SIZE);
        Offset += SubPackSize + OFFSET_SUB_DATA;
    }
    if(*(pData+Offset) != HEADER_SUBPACKET)
    {
        HICC_PRINTF("No find sub packet");
        return 2;
    }

    *pSubPacket = (_sSubPacketFormat *)(pData + Offset);
    return 0;
}
//-------------------------------------------------------------------------------------------------
u8_t CmdAp_getData(u16_t *pActType, sDATABASE_ITEM_DATA_FORMAT *pSubPacketData, u16_t *pSize, u8_t *pData)
{
    _sSubPacketFormat *pSubPacket;
    char DebugStr[128];

    if((pActType == NULL) || (pSubPacketData == NULL) || (pData == NULL) || (pSize == NULL))
    {
        HICC_PRINTF("Input param error");
        return 1;
    }

    pSubPacket = (_sSubPacketFormat *)pData;
    if(pSubPacket->Header != HEADER_SUBPACKET)
    {
        snprintf(DebugStr, sizeof(DebugStr), "Check hearder fail. Data:0x%02X", pSubPacket->Header);
        HICC_PRINTF(DebugStr);
        return 2;
    }

    if(pSubPacket->ActionType >= CMD_ACTION_NUMBER)
    {
        snprintf(DebugStr, sizeof(DebugStr), "Unknow action type. Type:0x%02X", pSubPacket->ActionType);
        HICC_PRINTF(DebugStr);
        return 3;
    }

    *pActType = pSubPacket->ActionType;
    *pSize    = pSubPacket->Size;

    memcpy(pSubPacketData, &(pSubPacket->Data), pSubPacket->Size);
    return 0;
}
//-------------------------------------------------------------------------------------------------
u16_t CmdAp_getDbSize(sDATABASE_ITEM_DATA_FORMAT *pDbInfo)
{
    u16_t DbSize;
    char DebugStr[128];

    if(pDbInfo == NULL)
    {
        HICC_PRINTF("Input param error");
        return 0;
    }
    if(pDbInfo->ucType == DATA_TYPE_NA)
        return 0;
    return pDbInfo->uiValueSize;
/*
    if(pDbInfo->ucType == DATA_TYPE_STRING)
        DbSize = strlen(pDbInfo->uValue.acString) + 1;
    else if(( pDbInfo->ucType >= DATA_TYPE_UI_DIGIT_8) && (pDbInfo->ucType <= DATA_TYPE_UI_DIGIT_32))
        DbSize = 4;
    else if(( pDbInfo->ucType >= DATA_TYPE_I_DIGIT_8) && (pDbInfo->ucType <= DATA_TYPE_I_DIGIT_32))
        DbSize = 4;
    else if((pDbInfo->ucType == DATA_TYPE_UI_DIGIT_64) || (pDbInfo->ucType == DATA_TYPE_I_DIGIT_64) || (pDbInfo->ucType == DATA_TYPE_FLOAT))
    {
        snprintf(DebugStr, sizeof(DebugStr), "No support Db Type. Type %d", pDbInfo->ucType);
        HICC_PRINTF(DebugStr);
        DbSize = 4;
    }
    else if(pDbInfo->ucType <= DATA_TYPE_STRUCT)
        DbSize = *((u16_t *)&pDbInfo->uValue.acString[0]);
    else
    {
        snprintf(DebugStr, sizeof(DebugStr), "No support Db Type. Type %d", pDbInfo->ucType);
        HICC_PRINTF(DebugStr);
        return 0;
    }
    return DbSize;
*/
}
//-------------------------------------------------------------------------------------------------
u8_t CmdAp_initPacket(_sPacketFormat *pPacket)
{
    if(pPacket == NULL)
    {
        HICC_PRINTF("Input param error");
        return 1;
    }

    pPacket->Header   = HEADER_PACKET;
    pPacket->CheckSum = 0;
    pPacket->Size     = OFFSET_SUB_PACKET;
    pPacket->Number   = 0;
    return 0;
}
//-------------------------------------------------------------------------------------------------
void CmdAp_buildSubPacketInfo(_sSubPacketInfo *pInfo, _sSubPacketFormat *pFormat)
{
    if((pInfo == NULL) || (pFormat == NULL))
    {
        HICC_PRINTF("Input param error");
        return;
    }

    pInfo->ActionType = pFormat->ActionType;
    pInfo->Size       = pFormat->Size - OFFSET_SUB_DATA_VALUE;
    return;
}
//-------------------------------------------------------------------------------------------------
void CmdAp_initPacketV2Info(_sPacketV2Info *pInfo, u16_t Ver, u16_t Interface, u16_t SaveDataWay, u8_t ExecResult)
{
    if(Ver == 0)
        Ver = HEADER_PACKET_VER_V1;
    else
        pInfo->Version = Ver;
    pInfo->Interface   = Interface;
    pInfo->Size        = 0;
    pInfo->SaveDataWay = SaveDataWay;
    pInfo->ExecResult  = ExecResult;
    pInfo->TotalNum    = 0;
}
//-------------------------------------------------------------------------------------------------
u8_t CmdAp_initPacketV2(u8_t *pPacket, _sPacketV2Info *pInfo)
{
    char DebugStr[128];
    _sPacketFormatV2 *pPacketV2;
    _sPacketFormat *pPacketV1;

    if((pPacket == NULL) || (pInfo == NULL))
    {
        HICC_PRINTF("Input param erorr");
        return 1;
    }

    if(pInfo->Interface >= CMD_INTERFACE_NUMBER)
    {
        snprintf(DebugStr, sizeof(DebugStr), "Init Packet Interface Fail. Idx:%d ", pInfo->Interface);
        HICC_PRINTF(DebugStr);
        return 2;
    }

    if(pInfo->SaveDataWay >= eSAVE_DATA_NUMBER)
    {
        snprintf(DebugStr, sizeof(DebugStr), "Init Packet SaveDataWay Fail. Idx:%d ", pInfo->SaveDataWay);
        HICC_PRINTF(DebugStr);
        return 3;
    }

    if(pInfo->ExecResult >=  eEXEC_CODE_NUMBER)
    {
        snprintf(DebugStr, sizeof(DebugStr), "Init Packet Exect Result Fail. Data:%d ", pInfo->ExecResult);
        HICC_PRINTF(DebugStr);
        return 3;
    }

    if(pInfo->Version < HEADER_PACKET_VER_V2)
    {
        pPacketV1           = (_sPacketFormat *)pPacket;
        pPacketV1->Header   = HEADER_PACKET;
        pPacketV1->CheckSum = 0;
        pPacketV1->Size     = OFFSET_SUB_PACKET;
        pPacketV1->Number   = 0;
    }
    else
    {
        pPacketV2              = (_sPacketFormatV2 *)pPacket;
        pPacketV2->Header      = HEADER_PACKET_V2;
        pPacketV2->CheckSum    = 0;
        pPacketV2->Size        = OFFSET_V2_SUB_PACKET;
        pPacketV2->Version     = pInfo->Version;
        pPacketV2->Interface   = pInfo->Interface;
        pPacketV2->SaveDataWay = pInfo->SaveDataWay;
        pPacketV2->ExecResult  = pInfo->ExecResult;
        pPacketV2->Number      = 0;
        memset(pPacketV2->Reserve, 0, SIZE_PACKET_V2_RESERVE);
    }

    return 0;
}
//-------------------------------------------------------------------------------------------------
u8_t CmdAp_addPacketIdNumber(_sPacketFormat *pPacket, u16_t IdNumber)
{
    if(pPacket == NULL)
    {
        HICC_PRINTF("Input param error");
        return 1;
    }

    pPacket->Number = IdNumber;
    return 0;
}
//-------------------------------------------------------------------------------------------------
u8_t CmdAp_addPacketV2IdNumber(u8_t *pPacket, u16_t IdNumber)
{
    _sPacketFormat *pPacketV1;
    _sPacketFormatV2 *pPacketV2;
    char DedbugStr[128];
    u16_t Ver;

    if(pPacket == NULL)
    {
        HICC_PRINTF("Input param error");
        return 1;
    }

    if(*pPacket != HEADER_PACKET_1)
    {
        snprintf(DedbugStr, sizeof(DedbugStr), "Check packet header fail. Data 0x%02X \n", *pPacket);
        HICC_PRINTF(DedbugStr);
        return 2;
    }
    if(*(pPacket+1) == HEADER_PACKET_V1_2)
    {
        pPacketV1         = (_sPacketFormat *)pPacket;
        pPacketV1->Number = IdNumber;
    }
    else if(*(pPacket+1) == HEADER_PACKET_V2_2)
    {
        pPacketV2         = (_sPacketFormatV2 *)pPacket;
        pPacketV2->Number = IdNumber;
    }
    else
    {
        snprintf(DedbugStr, sizeof(DedbugStr), "Check packet header fail. Data 0x%02X \n", *(pPacket+1));
        HICC_PRINTF(DedbugStr);
        return 2;
    }

    return 0;
}
//-------------------------------------------------------------------------------------------------
u8_t CmdAp_addPacketSize(_sPacketFormat *pPacket, u16_t Size)
{
    if(pPacket == NULL)
    {
        HICC_PRINTF("Input param error");
        return 1;
    }

    pPacket->Size = Size;
    return 0;
}
//-------------------------------------------------------------------------------------------------
u8_t CmdAp_addPacketV2ExecResult(u8_t *pPacket, u8_t Status)
{
    char DedbugStr[128];
    _sPacketFormatV2 *pPacketV2;

    if(pPacket == NULL)
    {
        HICC_PRINTF("Input param error");
        return 1;
    }

    if(*pPacket != HEADER_PACKET_1)
    {
        snprintf(DedbugStr, sizeof(DedbugStr), "Check packet header fail. Data 0x%02X \n", *pPacket);
        HICC_PRINTF(DedbugStr);
        return 2;
    }

    if(*(pPacket+1) == HEADER_PACKET_V1_2)
        return 0;
    else if(*(pPacket+1) == HEADER_PACKET_V2_2)
    {
        pPacketV2             = (_sPacketFormatV2 *)pPacket;
        pPacketV2->ExecResult = Status;
    }
    else
    {
        snprintf(DedbugStr, sizeof(DedbugStr), "Check packet header fail. Data 0x%02X \n", *(pPacket+1));
        HICC_PRINTF(DedbugStr);
        return 3;
    }
    return 0;
}
//-------------------------------------------------------------------------------------------------
u8_t CmdAp_addPacketV2Size(u8_t *pPacket, u16_t Size)
{
    _sPacketFormat *pPacketV1;
    _sPacketFormatV2 *pPacketV2;
    char DedbugStr[128];

    if(pPacket == NULL)
    {
        HICC_PRINTF("Input param error");
        return 1;
    }

    if(*pPacket != HEADER_PACKET_1)
    {
        snprintf(DedbugStr, sizeof(DedbugStr), "Check packet header fail. Data 0x%02X \n", *pPacket);
        HICC_PRINTF(DedbugStr);
        return 2;
    }
    if(*(pPacket+1) == HEADER_PACKET_V1_2)
    {
        pPacketV1       = (_sPacketFormat *)pPacket;
        pPacketV1->Size = Size;
    }
    else if(*(pPacket+1) == HEADER_PACKET_V2_2)
    {
        pPacketV2       = (_sPacketFormatV2 *)pPacket;
        pPacketV2->Size = Size;
    }
    else
    {
        snprintf(DedbugStr, sizeof(DedbugStr), "Check packet header fail. Data 0x%02X \n", *(pPacket+1));
        HICC_PRINTF(DedbugStr);
        return 2;
    }

    return 0;
}
//-------------------------------------------------------------------------------------------------
u8_t CmdAp_addPacketInfo(_sPacketFormat *pPacket, u16_t Size)
{
    u32_t Idx;
    u16_t CheckSum = 0;
    u8_t *pData;

    if(pPacket == NULL)
    {
        HICC_PRINTF("Input param error");
        return 1;
    }

    pPacket->Size  += (Size + OFFSET_SUB_DATA);
    pPacket->Number += 1;

    return 0;
}

//-------------------------------------------------------------------------------------------------
u8_t CmdAp_addPacketV2Info(u8_t *pPacket, u16_t Size)
{
    _sPacketFormat *pPacketV1;
    _sPacketFormatV2 *pPacketV2;
    char DedbugStr[128];

    if(pPacket == NULL)
    {
        HICC_PRINTF("Input param error");
        return 1;
    }

    if(*pPacket != HEADER_PACKET_1)
    {
        snprintf(DedbugStr, sizeof(DedbugStr), "Check packet header fail. Data 0x%02X \n", *pPacket);
        HICC_PRINTF(DedbugStr);
        return 2;
    }
    if(*(pPacket+1) == HEADER_PACKET_V1_2)
    {
        pPacketV1       = (_sPacketFormat *)pPacket;
        pPacketV1->Size += (Size + OFFSET_SUB_DATA);
        pPacketV1->Number += 1;
    }
    else if(*(pPacket+1) == HEADER_PACKET_V2_2)
    {
        pPacketV2       = (_sPacketFormatV2 *)pPacket;
        pPacketV2->Size += (Size + OFFSET_SUB_DATA);
        pPacketV2->Number += 1;
    }
    else
    {
        snprintf(DedbugStr, sizeof(DedbugStr), "Check packet header fail. Data 0x%02X \n", *(pPacket+1));
        HICC_PRINTF(DedbugStr);
        return 2;
    }

    return 0;
}
//-------------------------------------------------------------------------------------------------
u8_t CmdAp_initSubPacket(_sSubPacketFormat *pSubPacket)
{
    if(pSubPacket == NULL)
    {
        HICC_PRINTF("Input param error");
        return 0;
    }

    pSubPacket->Header     = HEADER_SUBPACKET;
    pSubPacket->ActionType = CMD_ACTION_NUMBER;
    pSubPacket->Size       = 0;
    return 0;
}
//-------------------------------------------------------------------------------------------------
u8_t CmdAp_addSubPacketInfo(_sSubPacketFormat *pSubPacket, u16_t Size, u16_t ActType)
{
    if((pSubPacket == NULL) || (ActType >= CMD_ACTION_NUMBER))
    {
        HICC_PRINTF("Input param error");
        return 1;
    }

    pSubPacket->ActionType = ActType;
    pSubPacket->Size = Size;
    return 0;
}
//-------------------------------------------------------------------------------------------------
u8_t CmdAp_addSubPacketData(sDATABASE_ITEM_DATA_FORMAT *pDtatBase, u8_t Type, u8_t Status, u16_t DataId, u8_t *pData, u16_t Size)
{
    u16_t SizeTmp;

    if((pDtatBase == NULL) || (pData  == NULL))
    {
        HICC_PRINTF("Input param error");
        return 1;
    }

    pDtatBase->ucType          = Type;
    pDtatBase->ucGrayoutStatus = Status;
    pDtatBase->uiItemIndex     = DataId;

    if(Type == DATA_TYPE_STRING)
    {
        snprintf((pDtatBase->uValue).acString, sizeof((pDtatBase->uValue).acString), "%s", pData);
        SizeTmp = strlen((pDtatBase->uValue).acString) + 1;
    }
    else if((Type == DATA_TYPE_UI_DIGIT_64) || (Type == DATA_TYPE_I_DIGIT_64))
    {
        memcpy(&((pDtatBase->uValue).lValue), pData, 8);
        SizeTmp = 8;
    }
    else if((Type >= DATA_TYPE_UI_DIGIT_8) && (Type <= DATA_TYPE_I_DIGIT_32))
    {
        memcpy(&((pDtatBase->uValue).lValue), pData, 4);
        SizeTmp = 4;
    }
    else
    {
        memcpy((pDtatBase->uValue).acString, pData, Size);
        SizeTmp = Size;
    }
    pDtatBase->uiValueSize = SizeTmp;
}
//-------------------------------------------------------------------------------------------------
u8_t CmdAp_addPacketCheckSum(_sPacketFormat *pPacket)
{
    u16_t CheckSum;
    u32_t Idx;
    u8_t *pData;

    if(pPacket == NULL)
    {
        HICC_PRINTF("Input param error");
        return 1;
    }
    CmdAp_calcCheckSum(&CheckSum, (u8_t *)&(pPacket->Size), pPacket->Size - OFFSET_LEN);
    pPacket->CheckSum = CheckSum;
#if 0
    pData = (u8_t *)pPacket;
    CmdAp_shoeDebugData("Cmd checkSum ", pData, pPacket->Size);
#endif
    return 0;
}
//-------------------------------------------------------------------------------------------------
u8_t CmdAp_addPacketV2CheckSum(u8_t *pPacket)
{
    u16_t CheckSum;
    u8_t *pData;
    _sPacketFormat *pPacketV1;
    _sPacketFormatV2 *pPacketV2;
    char DedbugStr[128];

    if(pPacket == NULL)
    {
        HICC_PRINTF("Input param error");
        return 1;
    }

    if(*pPacket != HEADER_PACKET_1)
    {
        snprintf(DedbugStr, sizeof(DedbugStr), "Check packet header fail. Data 0x%02X \n", *pPacket);
        HICC_PRINTF(DedbugStr);
        return 2;
    }
    if(*(pPacket+1) == HEADER_PACKET_V1_2)
    {
        pPacketV1       = (_sPacketFormat *)pPacket;
        CmdAp_calcCheckSum(&CheckSum, (u8_t *)&(pPacketV1->Size), pPacketV1->Size - OFFSET_LEN);
        pPacketV1->CheckSum = CheckSum;
    }
    else if(*(pPacket+1) == HEADER_PACKET_V2_2)
    {
        pPacketV2       = (_sPacketFormatV2 *)pPacket;
        CmdAp_calcCheckSum(&CheckSum, (u8_t *)&(pPacketV2->Size), pPacketV2->Size - OFFSET_LEN);
        pPacketV2->CheckSum = CheckSum;
    }
    else
    {
        snprintf(DedbugStr, sizeof(DedbugStr), "Check packet header fail. Data 0x%02X \n", *(pPacket+1));
        HICC_PRINTF(DedbugStr);
        return 2;
    }

#if 0
    pData = (u8_t *)pPacket;
    CmdAp_shoeDebugData("Cmd checkSum ", pData, pPacket->Size);
#endif
    return 0;
}
//-------------------------------------------------------------------------------------------------
u8_t CmdAp_buildSendData(_sPacketFormat *pPacket, _eActionType ActionType, u16_t Size, u16_t Id, u8_t Status, u8_t *pData, u8_t Type)
{
    sDATABASE_ITEM_DATA_FORMAT *pDataBase;
    _sSubPacketFormat *pSubPacket;
    u32_t OffsetLen = 0;
    u32_t Idx;
    u16_t SubPackSize;
    u16_t SizeTmp;

    if(pPacket == NULL)
    {
        printf("Input param error when CmdAp_buildSendData\n");
        return 1;
    }

    for(Idx=0; Idx<(pPacket->Number); Idx++)
    {
        SubPackSize = *((u16_t *)(pPacket->Value+OffsetLen+OFFSET_SUB_DATA_SIZE));
        OffsetLen += SubPackSize + OFFSET_SUB_DATA;
    }

    pSubPacket = (_sSubPacketFormat *)((&(pPacket->Value[0]) + OffsetLen));
    pDataBase  = (sDATABASE_ITEM_DATA_FORMAT *)((&(pPacket->Value[0]) + OffsetLen + OFFSET_SUB_DATA));
    CmdAp_initSubPacket(pSubPacket);
    CmdAp_addSubPacketInfo(pSubPacket, Size + OFFSET_SUB_DATA_VALUE, ActionType);
    if(pData != NULL)
    {
        if((Type == DATA_TYPE_I_DIGIT_64) || ((Type == DATA_TYPE_UI_DIGIT_64)))
        {
            memcpy(&(pDataBase->uValue.lValue), pData, 8);
            SizeTmp = 8;
        }
        else if((Type >= DATA_TYPE_UI_DIGIT_8) && ((Type <= DATA_TYPE_I_DIGIT_32)))
        {
            memcpy(&(pDataBase->uValue.lValue), pData, 4);
            SizeTmp = 4;
        }
        else if(Type == DATA_TYPE_STRING)
        {
            snprintf(pDataBase->uValue.acString, sizeof(pDataBase->uValue.acString), "%s", pData);
            SizeTmp = strlen(pDataBase->uValue.acString) + 1;
        }
        else if(Type == DATA_TYPE_VOID)
        {
            memcpy(pDataBase->uValue.acString, pData, Size);
            SizeTmp = Size;
        }
        else
            printf("Unknow database type:%d ID:%d when CmdAp_buildSendData\n", Type, Id);
    }
    pDataBase->ucType          = Type;
    pDataBase->ucGrayoutStatus = Status;
    pDataBase->uiItemIndex     = Id;
    pDataBase->uiValueSize     = SizeTmp;

    CmdAp_addPacketInfo(pPacket, Size + OFFSET_SUB_DATA_VALUE);

    return 0;
}
//-------------------------------------------------------------------------------------------------
u8_t CmdAp_buildV2SendData(u8_t *pPacket, _sSubPacketInfo *pSubPacketInfo, _sDataBaseInfo *pDbInfo, u8_t *pData)
{
    _sPacketFormatV2 *pPacketV2;
    _sPacketFormat  *pPacketV1;
    sDATABASE_ITEM_DATA_FORMAT *pDataBase;
    _sSubPacketFormat *pSubPacket;
    u32_t OffsetLen = 0;
    u32_t Idx;
    u16_t SubPackSize;
    char DebugStr[128];
    u8_t *pValue;
    u8_t Type, Status;
    u16_t  Id, Size, DbSize;
    _eActionType ActionType;
    char DedbugStr[128];
    u16_t Number;

    if((pPacket == NULL) || (pSubPacketInfo == NULL) || (pDbInfo == NULL))
    {
        HICC_PRINTF("Input param error");
        return 1;
    }

    if(*pPacket != HEADER_PACKET_1)
    {
        snprintf(DedbugStr, sizeof(DedbugStr), "Check packet header fail. Data 0x%02X \n", *pPacket);
        HICC_PRINTF(DedbugStr);
        return 3;
    }
    if(*(pPacket+1) == HEADER_PACKET_V1_2)
    {
        pPacketV1 = (_sPacketFormat *)pPacket;
        pValue    = &(pPacketV1->Value[0]);
        Number    = pPacketV1->Number;
    }
    else if(*(pPacket+1) == HEADER_PACKET_V2_2)
    {
        pPacketV2 = (_sPacketFormatV2 *)pPacket;
        pValue    = &(pPacketV2->Value[0]);
        Number    = pPacketV2->Number;
    }
    else
    {
        snprintf(DedbugStr, sizeof(DedbugStr), "Check packet header fail. Data 0x%02X \n", *(pPacket+1));
        HICC_PRINTF(DedbugStr);
        return 4;
    }

    ActionType = pSubPacketInfo->ActionType;
    Size       = pSubPacketInfo->Size;
    Type       = pDbInfo->Type;
    Status     = pDbInfo->Status;
    Id         = pDbInfo->Id;

    for(Idx=0; Idx<Number; Idx++)
    {
        SubPackSize = *((u16_t *)(pValue + OffsetLen+OFFSET_SUB_DATA_SIZE));
        OffsetLen += SubPackSize + OFFSET_SUB_DATA;
    }

    pSubPacket = (_sSubPacketFormat *)((pValue + OffsetLen));
    pDataBase  = (sDATABASE_ITEM_DATA_FORMAT *)((pValue + OffsetLen + OFFSET_SUB_DATA));
    CmdAp_initSubPacket(pSubPacket);
    if(pData != NULL)
    {
        if((Type == DATA_TYPE_I_DIGIT_64) || ((Type == DATA_TYPE_UI_DIGIT_64)))
        {
            memcpy(&(pDataBase->uValue.lValue), pData, 8);
            DbSize = 8;
        }
        else if((Type >= DATA_TYPE_UI_DIGIT_8) && ((Type <= DATA_TYPE_I_DIGIT_32)))
        {
            memcpy(&(pDataBase->uValue.lValue), pData, 4);
            DbSize = 4;
        }
        else if(Type == DATA_TYPE_STRING)
        {
            snprintf(pDataBase->uValue.acString, sizeof(pDataBase->uValue.acString), "%s", pData);
            DbSize = strlen(pDataBase->uValue.acString) + 1;
        }
        else if(Type == DATA_TYPE_VOID)
        {
            memcpy(pDataBase->uValue.acString, pData, Size);
            DbSize = Size;
        }
        else if(Type <= DATA_TYPE_STRUCT)
        {
            memcpy(pDataBase->uValue.acString, pData, Size);
            DbSize = Size;
        }
        else
        {
            memcpy(pDataBase->uValue.acString, pData, Size);
            DbSize = Size;
            snprintf(DebugStr, sizeof(DebugStr), "Unknow database type:%d ID:%d", Type, Id);
            HICC_PRINTF(DebugStr);
        }
    }
    CmdAp_addSubPacketInfo(pSubPacket, Size + OFFSET_SUB_DATA_VALUE, ActionType);
    pDataBase->ucType          = Type;
    pDataBase->ucGrayoutStatus = Status;
    pDataBase->uiItemIndex     = Id;
    pDataBase->uiValueSize     = DbSize;

    CmdAp_addPacketV2Info(pPacket, Size + OFFSET_SUB_DATA_VALUE);

    return 0;
}
//-------------------------------------------------------------------------------------------------
u8_t CmdAp_buildSendSingleData(_sPacketFormat *pPacket, _eActionType ActionType, u16_t Size, u16_t Id, u8_t Status, u8_t *pData, u8_t Type)
{
    if(pPacket == NULL)
    {
        HICC_PRINTF("Input param error");
        return 1;
    }

    if(CmdAp_initPacket(pPacket) != 0)
        return 2;
    if(CmdAp_buildSendData(pPacket, ActionType, Size, Id, Status, pData, Type) != 0)
        return 3;
    if(CmdAp_addPacketCheckSum(pPacket) != 0)
        return 4;
    return 0;
}
//-------------------------------------------------------------------------------------------------
u8_t CmdAp_buildV2SendSingleData(u8_t *pPacket, _sPacketV2Info *pPacketInfo, _sSubPacketInfo *pSubPackInfo, _sDataBaseInfo *pDbInfo, u8_t *pData)
{
    if((pPacket == NULL) || (pPacketInfo == NULL) || (pSubPackInfo == NULL) || (pDbInfo == NULL) || (pData == NULL))
    {
        HICC_PRINTF("Input param error");
        return 1;
    }

    if(CmdAp_initPacketV2(pPacket, pPacketInfo) != 0)
        return 2;
    if(CmdAp_buildV2SendData(pPacket, pSubPackInfo, pDbInfo, pData) != 0)
        return 3;
    if(CmdAp_addPacketV2CheckSum(pPacket) != 0)
        return 4;
    return 0;
}
//-------------------------------------------------------------------------------------------------
u8_t CmdAp_buildSubPacketConcat(_sSubPacketConcat *pNow, _sSubPacketConcat *pNext, _eActionType ActionType, u8_t DbType, u8_t Status, u16_t Id, u8_t *pValue, u16_t ValueSize)
{
    if((pNow == NULL) || (ActionType >= CMD_ACTION_NUMBER))
    {
        printf("Input param error when CmdAp_buildSubPacketConcat\n");
        return 1;
    }

    pNow->ActionType               = ActionType;
    pNow->Database.uiItemIndex     = Id;
    pNow->Database.ucGrayoutStatus = Status;
    pNow->Database.ucType          = DbType;
    pNow->Size            = 0;
    if((ActionType == CMD_ACTION_GET_DATABASE_ITEM)          || (ActionType == CMD_ACTION_GET_INFO_DATA)     ||
       (ActionType == CMD_ACTION_GET_DATABASE_ITEM_ACK_DONE) || (ActionType  == CMD_ACTION_GET_POWER_STATUS) ||
       (ActionType == CMD_ACTION_GET_MSSC)                   || (ActionType  == CMD_ACTION_GET_BLENDER)      ||
       (ActionType == CMD_ACTION_GET_STRUCT))
    {
        pNow->Database.uiValueSize = 0;
        pNow->Size += 0;
        pNow->pNextSubPacket = pNext;
    }
    else if((ActionType == CMD_ACTION_SET_DATABASE_ITEM)         || (ActionType == CMD_ACTION_SET_INFO_DATA)    ||
           (ActionType == CMD_ACTION_SET_DATABASE_ITEM_ACK_DONE) || (ActionType == CMD_ACTION_SET_POWER_STATUS) ||
           (ActionType == CMD_ACTION_SET_KEY_CODE)               || (ActionType == CMD_ACTION_SET_MSSC)         ||
           (ActionType == CMD_ACTION_SET_BLENDER)                || (ActionType == CMD_ACTION_SET_STRUCT)       ||
           (ActionType == CMD_ACTION_SET_LOG))
    {
        pNow->Database.uiValueSize = ValueSize;
        pNow->Size += ValueSize;
        memcpy(pNow->Database.uValue.acString, pValue, pNow->Size);
        pNow->pNextSubPacket = pNext;
    }

    return 0;
}
//-------------------------------------------------------------------------------------------------
u8_t CmdAp_parseSubPacketConcat(_sPacketFormat *pPacket, char *ActStr ,_sSubPacketConcat *pData)
{
    _sSubPacketConcat *pTmpData;
    sDATABASE_ITEM_DATA_FORMAT *pDataBase;

    if((pPacket == NULL) || (pData == NULL) || (ActStr == NULL))
    {
        printf("Input param error when CmdAp_parseSubPacketConcat \n");
        return 1;
    }

    CmdAp_initPacket(pPacket);
    pTmpData = pData;
    while(pTmpData != NULL)
    {
        pDataBase = &(pTmpData->Database);
        if((pTmpData->ActionType == CMD_ACTION_GET_DATABASE_ITEM)          || (pTmpData->ActionType == CMD_ACTION_GET_INFO_DATA)    ||
           (pTmpData->ActionType == CMD_ACTION_GET_DATABASE_ITEM_ACK_DONE) || (pTmpData->ActionType == CMD_ACTION_GET_POWER_STATUS) ||
           (pTmpData->ActionType == CMD_ACTION_GET_MSSC)                   || (pTmpData->ActionType == CMD_ACTION_GET_BLENDER)      ||
           (pTmpData->ActionType == CMD_ACTION_GET_STRUCT))
        {
            CmdAp_buildSendData(pPacket, pTmpData->ActionType, pTmpData->Size, pDataBase->uiItemIndex, 0, NULL, pDataBase->ucType);
            sprintf(ActStr, "get");
        }
        else if((pTmpData->ActionType == CMD_ACTION_SET_DATABASE_ITEM)          || (pTmpData->ActionType == CMD_ACTION_SET_INFO_DATA)    ||
                (pTmpData->ActionType == CMD_ACTION_SET_DATABASE_ITEM_ACK_DONE) || (pTmpData->ActionType == CMD_ACTION_SET_POWER_STATUS) ||
                (pTmpData->ActionType == CMD_ACTION_SET_KEY_CODE)               || (pTmpData->ActionType == CMD_ACTION_SET_MSSC)         ||
                (pTmpData->ActionType == CMD_ACTION_SET_BLENDER)                || (pTmpData->ActionType == CMD_ACTION_SET_STRUCT))
        {
            CmdAp_buildSendData(pPacket, pTmpData->ActionType, pTmpData->Size, pDataBase->uiItemIndex, pDataBase->ucGrayoutStatus, pDataBase->uValue.acString, pDataBase->ucType);
            sprintf(ActStr, "set");
        }
        else
        {
            printf("No support action type %d when CmdAp_parseSubPacketConcat \n", pData->ActionType);
            return 2;
        }
        pTmpData = pTmpData->pNextSubPacket;
    }
    CmdAp_addPacketCheckSum(pPacket);
    return 0;
}
//-------------------------------------------------------------------------------------------------
u8_t CmdAp_parseSubPacketV2Concat(u8_t *pPacket, char *ActStr ,_sSubPacketConcat *pData, _sPacketV2Info *pPacketInfo)
{
    _sSubPacketConcat *pTmpData;
    sDATABASE_ITEM_DATA_FORMAT *pDataBase;
    char DebugStr[128];
    _sDataBaseInfo DbInfo;
    _sSubPacketInfo SubPacketInfo;

    if((pPacket == NULL) || (pData == NULL) || (ActStr == NULL) || (pPacketInfo == NULL))
    {
        printf("Input param error when CmdAp_parseSubPacketConcat \n");
        return 1;
    }

    CmdAp_initPacketV2(pPacket, pPacketInfo);
    pTmpData = pData;
    while(pTmpData != NULL)
    {
        pDataBase                = &(pTmpData->Database);
        SubPacketInfo.ActionType = pTmpData->ActionType;
        SubPacketInfo.Size       = pTmpData->Size;
        DbInfo.Type              = pDataBase->ucType;
        DbInfo.Id                = pDataBase->uiItemIndex;

        if((pTmpData->ActionType == CMD_ACTION_GET_DATABASE_ITEM)          || (pTmpData->ActionType == CMD_ACTION_GET_INFO_DATA)    ||
           (pTmpData->ActionType == CMD_ACTION_GET_DATABASE_ITEM_ACK_DONE) || (pTmpData->ActionType == CMD_ACTION_GET_POWER_STATUS) ||
           (pTmpData->ActionType == CMD_ACTION_GET_MSSC)                   || (pTmpData->ActionType == CMD_ACTION_GET_BLENDER)      ||
           (pTmpData->ActionType == CMD_ACTION_GET_STRUCT))
        {
            DbInfo.Status = eFUNC_CONTROL_ENABLE;
            CmdAp_buildV2SendData(pPacket, &SubPacketInfo, &DbInfo, NULL);
            sprintf(ActStr, "get");
        }
        else if((pTmpData->ActionType == CMD_ACTION_SET_DATABASE_ITEM)          || (pTmpData->ActionType == CMD_ACTION_SET_INFO_DATA)    ||
                (pTmpData->ActionType == CMD_ACTION_SET_DATABASE_ITEM_ACK_DONE) || (pTmpData->ActionType == CMD_ACTION_SET_POWER_STATUS) ||
                (pTmpData->ActionType == CMD_ACTION_SET_KEY_CODE)               || (pTmpData->ActionType == CMD_ACTION_SET_MSSC)         ||
                (pTmpData->ActionType == CMD_ACTION_SET_BLENDER)                || (pTmpData->ActionType == CMD_ACTION_SET_STRUCT))
        {
            DbInfo.Status = pDataBase->ucGrayoutStatus;
            CmdAp_buildV2SendData(pPacket, &SubPacketInfo, &DbInfo, pDataBase->uValue.acString);
            sprintf(ActStr, "set");
        }
        else
        {
            snprintf(DebugStr, sizeof(DebugStr), "No support action type %d", pData->ActionType);
            HICC_PRINTF(DebugStr);
            return 2;
        }
        pTmpData = pTmpData->pNextSubPacket;
    }
    CmdAp_addPacketV2CheckSum(pPacket);
    return 0;
}
//-------------------------------------------------------------------------------------------------
u8_t CmdAp_buildDatabase(sDATABASE_ITEM_DATA_FORMAT *pDatabase, _eDataBaseStaus Status, u8_t Type, u16_t Id, u8_t *pData)
{
    u32_t Size, SizeTmp;
    char DebugStr[128];

    if((pDatabase == NULL) || (Status >= DB_STATUS_NUMBER) || (pData == NULL))
    {
        HICC_PRINTF("Input param error");
        return 1;
    }

    pDatabase->ucGrayoutStatus = Status;
    pDatabase->ucType          = Type;
    pDatabase->uiItemIndex     = Id;

    if(Type == DATA_TYPE_STRING)
    {
        Size = snprintf(pDatabase->uValue.acString, sizeof(pDatabase->uValue.acString), "%s", pData);
        pDatabase->uValue.acString[Size] = '\0';
        SizeTmp = Size + 1;
    }
    else if((Type == DATA_TYPE_UI_DIGIT_64) || (Type == DATA_TYPE_I_DIGIT_64))
    {
        memcpy(pDatabase->uValue.acString, pData, 8);
        SizeTmp = 8;
    }
    else if((Type >= DATA_TYPE_UI_DIGIT_8) && (Type <= DATA_TYPE_I_DIGIT_32))
    {
        memcpy(pDatabase->uValue.acString, pData, 4);
        SizeTmp = 4;
    }
    else
    {
        snprintf(DebugStr, sizeof(DebugStr), "No support database type. Type:%d ", Type);
        HICC_PRINTF(DebugStr);
        memset(pDatabase->uValue.acString, 0, sizeof(pDatabase->uValue.acString));
        SizeTmp = strlen(pDatabase->uValue.acString)  + 1;
        return 2;
    }
    pDatabase->uiValueSize = SizeTmp;
    return 0;
}
//-------------------------------------------------------------------------------------------------
u8_t CmdAp_buildV2Database(sDATABASE_ITEM_DATA_FORMAT *pDatabase, _eDataBaseStaus Status, u8_t Type, u16_t Id, u8_t *pData, u32_t Size)
{
    if((pDatabase == NULL) || (pData == NULL))
    {
        HICC_PRINTF("Input param error ");
        return 1;
    }
    pDatabase->ucGrayoutStatus = Status;
    pDatabase->ucType          = Type;
    pDatabase->uiItemIndex     = Id;
    pDatabase->uiValueSize     = Size;
    if(Size == 0)
        memset(pDatabase->uValue.acString, 0, Size);
    else
        memcpy(pDatabase->uValue.acString, pData, Size);
    return 0;
}
//-------------------------------------------------------------------------------------------------
u8_t CmdAp_buildBlender(sDATABASE_ITEM_DATA_FORMAT *pDatabase, _eDataBaseStaus Status, u8_t Type, u16_t Id, u8_t *pData, u16_t DataSize)
{
    u32_t Size;

    if((pDatabase == NULL) || (Status >= DB_STATUS_NUMBER) || (pData == NULL))
    {
        printf("Input param error when CmdAp_buildBlender \n");
        return 1;
    }

    pDatabase->ucGrayoutStatus = Status;
    pDatabase->ucType          = Type;
    pDatabase->uiItemIndex     = Id;
    pDatabase->uiValueSize     = DataSize;
    memcpy(pDatabase->uValue.acString, pData, DataSize);

    return 0;
}
//-------------------------------------------------------------------------------------------------
void CmdAp_shoeDebugData(char *pTitle, u8_t *pData, u32_t Size)
{
    u32_t Idx;

    if((pTitle == NULL) || (Size == 0))
    return;

    printf("%s Size:%d Data: ", pTitle, Size);
    for(Idx=0; Idx<Size; Idx++)
    {
        if(Idx%8 == 0)
            printf("\n");
        printf("0x%02X ", *(pData+Idx));
    }
    printf("\n");
}
//-------------------------------------------------------------------------------------------------
u32_t CmdAp_checkVersionDataCode(u32_t *pMinStrLen, u16_t Id)
{
    if(pMinStrLen == NULL)
    {
        HICC_PRINTF("Input param error");
        return 1;
    }
    if((Id != edcFMCU_VERSION)          && (Id != edcSMCU_VERSION)      && (Id != edcFRONTEND_VERSION) &&
       (Id != edcFORMATER_VERSION)      && (Id != edcHDBASET_VERSION)   && (Id != edcKEYPAD_VERSION) &&
       (Id != edcLD_DRIVER_VERSION)     && (Id != edcFIRMWARE)          && (Id != edcMEMC_VERSION) &&
       (Id != edcMOTOR_VERSION)         && (Id != edcLAN_VERSION)       && (Id != edcFPGA1_VERSION) &&
       (Id != edcFPGA2_VERSION)         && (Id != edcFPGA3_VERSION)     && (Id != edcPMCU_VERSION) &&
       (Id != edcXFPGA_VERSION)         && (Id != edcRELEASE_VERSION)   && (Id != edc3GSDI_VERSION) &&
       (Id != edcGEOMETRY_VERSION)      && (Id != edcCAMERA_FW_VERSION) && (Id != edcM481_VERSION) &&
       (Id != edcOSD_FIRMWARE_VERSION)  && (Id != edcXPR_VERSION))
    {
          return 2;
    }
    if(Id == edcFIRMWARE)
    {
        *pMinStrLen = 9;
        return 0;
    }
    *pMinStrLen = 1;
    return 0;
}
//-------------------------------------------------------------------------------------------------
u8_t CmdAp_checkIpInfo(char *pIpAddr, char *pSubnetMask, char *pGateway)
{
    u32_t ip[4];
    u32_t subnet[4];
    u32_t gateway[4];
    u32_t ipClass = 0;
    u32_t u32Ip;
    u32_t u32SubMask;
    u32_t u32Gateway;
    u32_t i;
    u8_t noSubmask = 0, noGateway = 0;

    // Check IP Address
    sscanf(pIpAddr, "%d.%d.%d.%d", &ip[0], &ip[1], &ip[2], &ip[3]);

    for(i = 0; i < 4; i++)
    {
        if(ip[i] > 0xFF)
            return eLAN_DATA_IP_OUT_OF_RANGE;
    }

    if(ip[0] >= 0 && ip[0] <= 127)
        ipClass = 1;
    else if(ip[0] >= 128 && ip[0] <= 191)
        ipClass = 2;
    else if(ip[0] >= 192 && ip[0] <= 223)
        ipClass = 3;
    else
        return eLAN_DATA_IP_CLASS_FAIL;

    // Check Subnet Mask
    if(strcmp(pSubnetMask, "0.0.0.0") == 0)
    {
        noSubmask = 1;
        memset(subnet, 0, sizeof(subnet));

        if(ipClass == 1)
        {
            subnet[0] = 0xFF;
        }
        else if(ipClass == 2)
        {
            subnet[0] = 0xFF;
            subnet[1] = 0xFF;
        }
        else if(ipClass == 3)
        {
            subnet[0] = 0xFF;
            subnet[1] = 0xFF;
            subnet[2] = 0xFF;
        }
    }
    else
    {
        if((strcmp(pSubnetMask, "255.255.255.254") == 0) || (strcmp(pSubnetMask, "255.255.255.255") == 0))
            return eLAN_DATA_SUBMASK_OUT_OF_RANGE;

        sscanf(pSubnetMask, "%d.%d.%d.%d", &subnet[0], &subnet[1], &subnet[2], &subnet[3]);

        for(i = 0; i < 4; i++)
        {
            if(subnet[i] > 0xFF)
                return eLAN_DATA_SUBMASK_OUT_OF_RANGE;
        }

        u32SubMask = (subnet[0] << 24) + (subnet[1] << 16) + (subnet[2] << 8) + subnet[3];
        u32SubMask = ~u32SubMask + 1;

        if((u32SubMask & (u32SubMask - 1)) != 0)
            return eLAN_DATA_SUBMASK_BIT_FAIL;
    }

    // Check Gateway
    if(strcmp(pGateway, "0.0.0.0") == 0)
    {
        noGateway = 1;
        for(i = 0; i < 4; i++)
        {
            gateway[i] = ip[i];
        }
    }
    else
    {
        sscanf(pGateway, "%d.%d.%d.%d", &gateway[0], &gateway[1], &gateway[2], &gateway[3]);

        for(i = 0; i < 4; i++)
        {
            if(gateway[i] > 0xFF)
                return eLAN_DATA_GATEWAY_OUT_OF_RANGE;
        }
    }

    if(noSubmask && noGateway)
        return eLAN_DATA_EMPTY_SUBMASK_AND_GATEWAY;
    else if(noSubmask)
        return eLAN_DATA_EMPTY_SUBMASK;
    else if(noGateway)
        return eLAN_DATA_EMPTY_GATEWAY;

    u32Ip = (ip[0] << 24) + (ip[1] << 16) + (ip[2] << 8) + ip[3];
    u32SubMask = (subnet[0] << 24) + (subnet[1] << 16) + (subnet[2] << 8) + subnet[3];
    u32Gateway = (gateway[0] << 24) + (gateway[1] << 16) + (gateway[2] << 8) + gateway[3];

    for(i = 0; i < 32; i++)
    {
        if(BIT_CHECK(u32SubMask, 31-i))
        {
            if(BIT_CHECK(u32Gateway, 31-i) != BIT_CHECK(u32Ip, 31-i))
                return eLAN_DATA_GATEWAY_NOT_MATCH_IP;
        }
        else
        {
            break;
        }
    }

    return eLAN_DATA_PASS;
}
//-------------------------------------------------------------------------------------------------
u8_t CmdAp_checkIpInfoNoStrict(char *pIpAddr, char *pSubnetMask, char *pGateway)
{
    u32_t ip[4], subnet[4], gateway[4];
    u32_t u32Ip, u32SubMask, u32Gateway;
    u32_t i;

// IP
    sscanf(pIpAddr, "%d.%d.%d.%d", &ip[0], &ip[1], &ip[2], &ip[3]);
    for(i = 0; i < 4; i++)
    {
        if(ip[i] > 0xFF)
            return eLAN_DATA_IP_OUT_OF_RANGE;
    }
    u32Ip = (ip[0] << 24) + (ip[1] << 16) + (ip[2] << 8) + ip[3];
    if(u32Ip == 0xFFFFFFFF)
    {
        return eLAN_DATA_IP_OUT_OF_RANGE;
    }

// Subnet mask
    sscanf(pSubnetMask, "%d.%d.%d.%d", &subnet[0], &subnet[1], &subnet[2], &subnet[3]);
    for(i = 0; i < 4; i++)
    {
        if(subnet[i] > 0xFF)
            return eLAN_DATA_SUBMASK_OUT_OF_RANGE;
    }
    u32SubMask = (subnet[0] << 24) + (subnet[1] << 16) + (subnet[2] << 8) + subnet[3];
    if((u32SubMask | (u32SubMask - 1)) != 0xFFFFFFFF)
    {
        return eLAN_DATA_SUBMASK_BIT_FAIL;
    }

// Gateway
    sscanf(pGateway, "%d.%d.%d.%d", &gateway[0], &gateway[1], &gateway[2], &gateway[3]);
    for(i = 0; i < 4; i++)
    {
        if(gateway[i] > 0xFF)
            return eLAN_DATA_GATEWAY_OUT_OF_RANGE;
    }
    u32Gateway = (gateway[0] << 24) + (gateway[1] << 16) + (gateway[2] << 8) + gateway[3];
    if(u32Gateway == 0xFFFFFFFF)
    {
        return eLAN_DATA_GATEWAY_OUT_OF_RANGE;
    }

    return eLAN_DATA_PASS;
}
//-------------------------------------------------------------------------------------------------
u8_t CmdAp_checkIpInfoOptoma(char *pIpAddr, char *pSubnetMask, char *pGateway)
{
    u32_t ip[4];
    u32_t subnet[4];
    u32_t gateway[4];
    u32_t ipClass = 0;
    u32_t u32Ip;
    u32_t u32SubMask;
    u32_t u32Gateway;
    u32_t i;
    u8_t noSubmask = 0, noGateway = 0;

    // Check IP Address
    sscanf(pIpAddr, "%d.%d.%d.%d", &ip[0], &ip[1], &ip[2], &ip[3]);

    for(i = 0; i < 4; i++)
    {
        if(ip[i] > 0xFF)
            return eLAN_DATA_IP_OUT_OF_RANGE;
    }

    if(ip[0] >= 0 && ip[0] <= 127)
        ipClass = 1;
    else if(ip[0] >= 128 && ip[0] <= 191)
        ipClass = 2;
    else if(ip[0] >= 192 && ip[0] <= 223)
        ipClass = 3;
    else
        return eLAN_DATA_IP_CLASS_FAIL;

    // Check Subnet Mask
    if(strcmp(pSubnetMask, "0.0.0.0") == 0)
    {
        noSubmask = 1;
        memset(subnet, 0, sizeof(subnet));

        if(ipClass == 1)
        {
            subnet[0] = 0xFF;
        }
        else if(ipClass == 2)
        {
            subnet[0] = 0xFF;
            subnet[1] = 0xFF;
        }
        else if(ipClass == 3)
        {
            subnet[0] = 0xFF;
            subnet[1] = 0xFF;
            subnet[2] = 0xFF;
        }
    }
    else
    {
        if((strcmp(pSubnetMask, "255.255.255.254") == 0) || (strcmp(pSubnetMask, "255.255.255.255") == 0))
            return eLAN_DATA_SUBMASK_OUT_OF_RANGE;

        sscanf(pSubnetMask, "%d.%d.%d.%d", &subnet[0], &subnet[1], &subnet[2], &subnet[3]);

        for(i = 0; i < 4; i++)
        {
            if(subnet[i] > 0xFF)
                return eLAN_DATA_SUBMASK_OUT_OF_RANGE;
        }

        u32SubMask = (subnet[0] << 24) + (subnet[1] << 16) + (subnet[2] << 8) + subnet[3];
        u32SubMask = ~u32SubMask + 1;

        if((u32SubMask & (u32SubMask - 1)) != 0)
            return eLAN_DATA_SUBMASK_BIT_FAIL;
    }

    // Check Gateway
    if(strcmp(pGateway, "0.0.0.0") == 0)
    {
        noGateway = 1;
        for(i = 0; i < 4; i++)
        {
            gateway[i] = ip[i];
        }
    }
    else
    {
        sscanf(pGateway, "%d.%d.%d.%d", &gateway[0], &gateway[1], &gateway[2], &gateway[3]);

        for(i = 0; i < 4; i++)
        {
            if(gateway[i] > 0xFF)
                return eLAN_DATA_GATEWAY_OUT_OF_RANGE;
        }
    }

    u32Ip = (ip[0] << 24) + (ip[1] << 16) + (ip[2] << 8) + ip[3];
    u32SubMask = (subnet[0] << 24) + (subnet[1] << 16) + (subnet[2] << 8) + subnet[3];
    u32Gateway = (gateway[0] << 24) + (gateway[1] << 16) + (gateway[2] << 8) + gateway[3];

    for(i = 0; i < 32; i++)
    {
        if(BIT_CHECK(u32SubMask, 31-i))
        {
            if(BIT_CHECK(u32Gateway, 31-i) != BIT_CHECK(u32Ip, 31-i))
                return eLAN_DATA_GATEWAY_NOT_MATCH_IP;
        }
        else
        {
            break;
        }
    }

    return eLAN_DATA_PASS;
}
//-------------------------------------------------------------------------------------------------
u8_t CmdAp_lanSuggestedApply(u8_t faildEvent, char *pIpAddr, char *pSubnetMask, char *pGateway)
{
    char suggested_ip[MAX_LENGTH_NET_INFO_STR], suggested_subnet[MAX_LENGTH_NET_INFO_STR], suggested_gateway[MAX_LENGTH_NET_INFO_STR];
    u32_t ip[4], subnet[4], gateway[4];
    u32_t ipClass = 0;
    u8_t Buffer[1024];
    u32_t Argc;
    char *pArgv[32][2];
    char DebugStr[128];

    if((pIpAddr == NULL) || (pSubnetMask == NULL) || (pGateway == NULL))
        return 1;

    sscanf(pIpAddr, "%d.%d.%d.%d", &ip[0], &ip[1], &ip[2], &ip[3]);
    sscanf(pSubnetMask, "%d.%d.%d.%d", &subnet[0], &subnet[1], &subnet[2], &subnet[3]);
    sscanf(pGateway, "%d.%d.%d.%d", &gateway[0], &gateway[1], &gateway[2], &gateway[3]);

    memset(suggested_subnet, 0, sizeof(suggested_subnet));
    if(ip[0] >= 0 && ip[0] <= 127)
    {
        suggested_subnet[0] = 0xFF;
    }
    else if(ip[0] >= 128 && ip[0] <= 191)
    {
        suggested_subnet[0] = 0xFF;
        suggested_subnet[1] = 0xFF;
    }
    else if(ip[0] >= 192 && ip[0] <= 223)
    {
        suggested_subnet[0] = 0xFF;
        suggested_subnet[1] = 0xFF;
        suggested_subnet[2] = 0xFF;
    }

    switch(faildEvent)
    {
        case eLAN_DATA_PASS:
            {
                // Do nothing
            }
            break;
        case eLAN_DATA_EMPTY_SUBMASK:
            {
                sprintf(pSubnetMask, "%d.%d.%d.%d", suggested_subnet[0], suggested_subnet[1], suggested_subnet[2], suggested_subnet[3]);
            }
            break;
        case eLAN_DATA_EMPTY_GATEWAY:
            {
                snprintf(pGateway, MAX_LENGTH_NET_INFO_STR, "%s", pIpAddr);
            }
            break;
        case eLAN_DATA_EMPTY_SUBMASK_AND_GATEWAY:
            {
                sprintf(pSubnetMask, "%d.%d.%d.%d", suggested_subnet[0], suggested_subnet[1], suggested_subnet[2], suggested_subnet[3]);
                snprintf(pGateway, MAX_LENGTH_NET_INFO_STR, "%s", pIpAddr);
            }
            break;
        default:
            {
                if(access(PATH_DEFAULT_LAN_CONFIG_FILE,0) == 0)
                {
                    Argc = readConf(PATH_DEFAULT_LAN_CONFIG_FILE, pArgv, 32, Buffer, sizeof(Buffer));
                    if(Argc == 0)
                    {
                        return 2;
                    }
                    snprintf(pIpAddr, COMMON_ARRAYSIZE_32, "%s", findArgValue(NAME_ITEM_LAN_IP, pArgv, Argc));
                    snprintf(pSubnetMask, COMMON_ARRAYSIZE_32, "%s", findArgValue(NAME_ITEM_LAN_SUBMASK, pArgv, Argc));
                    snprintf(pGateway, COMMON_ARRAYSIZE_32, "%s", findArgValue(NAME_ITEM_LAN_GATEWAY, pArgv, Argc));
                }
                else
                {
                    return 3;
                }
            }
            break;
    }

    return 0;
}
//-------------------------------------------------------------------------------------------------
u8_t CmdAp_checkWlanInfo(char* pSsid, char* pPasswd, char *pStartIpAddr, char *pEndIpAddr, char *pSubnetMask, char *pGateway)
{
    struct sockaddr_in sa;
    u32_t startip[4];
    u32_t endip[4];
    u32_t subnet[4];
    u32_t gateway[4];
    u32_t i;
    u8_t checkStartStatus, checkEndStatus;

    if((strlen(pStartIpAddr) < COMMON_LEN_IP_MIN) || (strlen(pEndIpAddr) < COMMON_LEN_IP_MIN) || (strlen(pSubnetMask) < COMMON_LEN_IP_MIN) || (strlen(pGateway) < COMMON_LEN_IP_MIN))
        return eWLAN_DATA_INPUT_ERROR;

    if((strlen(pSsid) > COMMON_LEN_SSID_MAX) && (strlen(pSsid) == 0))
        return eWLAN_DATA_SSID_OUT_OF_RANGE;

    // Check start IP Address
    sscanf(pStartIpAddr, "%d.%d.%d.%d", &startip[0], &startip[1], &startip[2], &startip[3]);
    for(i = 0; i < 4; i++)
    {
        if(startip[i] > 0xFF)
            return eWLAN_DATA_START_IP_OUT_OF_RANGE;
    }
    if(inet_pton(AF_INET, pStartIpAddr, &(sa.sin_addr)) != 1)
    {
        return eWLAN_DATA_START_IP_CLASS_FAIL;
    }

    // Check end IP Address
    sscanf(pEndIpAddr, "%d.%d.%d.%d", &endip[0], &endip[1], &endip[2], &endip[3]);
    for(i = 0; i < 4; i++)
    {
        if(endip[i] > 0xFF)
            return eWLAN_DATA_END_IP_OUT_OF_RANGE;
    }
    if(inet_pton(AF_INET, pEndIpAddr, &(sa.sin_addr)) != 1)
    {
        return eWLAN_DATA_END_IP_CLASS_FAIL;
    }

    // Check ip range
    for(i = 0; i < 4; i++)
    {
        if(i < 3)
        {
            if(startip[i] != endip[i])
                return eWLAN_DATA_START_END_IP_OUT_OF_RANGE;
        }
        else
        {
            if(startip[i] > endip[i])
                return eWLAN_DATA_START_END_IP_OUT_OF_RANGE;
        }
    }

    // Check subnet mask Address
    sscanf(pSubnetMask, "%d.%d.%d.%d", &subnet[0], &subnet[1], &subnet[2], &subnet[3]);
    for(i = 0; i < 4; i++)
    {
        if(subnet[i] > 0xFF)
            return eWLAN_DATA_NETMASK_OUT_OF_RANGE;
    }

    // Check gateway Address
    sscanf(pGateway, "%d.%d.%d.%d", &gateway[0], &gateway[1], &gateway[2], &gateway[3]);
    for(i = 0; i < 4; i++)
    {
        if(gateway[i] > 0xFF)
            return eWLAN_DATA_GATEWAY_OUT_OF_RANGE;
    }

    checkStartStatus = CmdAp_checkIpInfo(pStartIpAddr, pSubnetMask, pGateway);
    checkEndStatus = CmdAp_checkIpInfo(pEndIpAddr, pSubnetMask, pGateway);
    if((checkStartStatus == eLAN_DATA_EMPTY_SUBMASK) || (checkEndStatus == eLAN_DATA_EMPTY_SUBMASK))
        return eWLAN_DATA_EMPTY_SUBMASK;
    else if((checkStartStatus == eLAN_DATA_EMPTY_GATEWAY) || (checkEndStatus == eLAN_DATA_EMPTY_GATEWAY))
        return eWLAN_DATA_EMPTY_GATEWAY;
    else if((checkStartStatus == eWLAN_DATA_EMPTY_SUBMASK_AND_GATEWAY) || (checkEndStatus == eWLAN_DATA_EMPTY_SUBMASK_AND_GATEWAY))
        return eWLAN_DATA_EMPTY_SUBMASK_AND_GATEWAY;
    else if((checkStartStatus != eLAN_DATA_PASS) || (checkEndStatus != eLAN_DATA_PASS))
        return eWLAN_DATA_CHECK_IP_FAIL;

    return eWLAN_DATA_PASS;
}
//-------------------------------------------------------------------------------------------------
u8_t CmdAp_wlanSuggestedApply(u8_t faildEvent, char *pIpAddr, char *pSubnetMask, char *pGateway)
{
    char suggested_ip[MAX_LENGTH_NET_INFO_STR], suggested_subnet[MAX_LENGTH_NET_INFO_STR], suggested_gateway[MAX_LENGTH_NET_INFO_STR];
    u32_t ip[4], subnet[4], gateway[4];
    u32_t ipClass = 0;
    u8_t Buffer[1024];

    if((pIpAddr == NULL) || (pSubnetMask == NULL) || (pGateway == NULL))
        return 1;

    sscanf(pIpAddr, "%d.%d.%d.%d", &ip[0], &ip[1], &ip[2], &ip[3]);
    sscanf(pSubnetMask, "%d.%d.%d.%d", &subnet[0], &subnet[1], &subnet[2], &subnet[3]);
    sscanf(pGateway, "%d.%d.%d.%d", &gateway[0], &gateway[1], &gateway[2], &gateway[3]);

    memset(suggested_subnet, 0, sizeof(suggested_subnet));
    if(ip[0] >= 0 && ip[0] <= 127)
    {
        suggested_subnet[0] = 0xFF;
    }
    else if(ip[0] >= 128 && ip[0] <= 191)
    {
        suggested_subnet[0] = 0xFF;
        suggested_subnet[1] = 0xFF;
    }
    else if(ip[0] >= 192 && ip[0] <= 223)
    {
        suggested_subnet[0] = 0xFF;
        suggested_subnet[1] = 0xFF;
        suggested_subnet[2] = 0xFF;
    }

    switch(faildEvent)
    {
        case eWLAN_DATA_EMPTY_SUBMASK:
            {
                sprintf(pSubnetMask, "%d.%d.%d.%d", suggested_subnet[0], suggested_subnet[1], suggested_subnet[2], suggested_subnet[3]);
            }
            break;
        case eWLAN_DATA_EMPTY_GATEWAY:
            {
                snprintf(pGateway, MAX_LENGTH_NET_INFO_STR, "%s", pIpAddr);
            }
            break;
        case eWLAN_DATA_EMPTY_SUBMASK_AND_GATEWAY:
            {
                sprintf(pSubnetMask, "%d.%d.%d.%d", suggested_subnet[0], suggested_subnet[1], suggested_subnet[2], suggested_subnet[3]);
                snprintf(pGateway, MAX_LENGTH_NET_INFO_STR, "%s", pIpAddr);
            }
            break;
    }

    return 0;
}
//-------------------------------------------------------------------------------------------------
u8_t CmdAp_parseGecErrorType(u32_t Log)
{
    u8_t ErrorType = GEC_ERROR_TYPE_NONE;

    if((Log & 0xFF000000) == 0x02000000)
    {
        switch(Log & 0xFFFF0000)
        {
            case LOG_ERROR_LAMP_MODULE:
            case LOG_ERROR_LAMP_SENSOR:
                ErrorType = GEC_ERROR_TYPE_LAMP;
                break;
        }
    }
    else
    {
        switch(Log & 0xFFFFFF00)
        {
            case LOG_ERROR_TEMP_AMBIENT:
            case LOG_ERROR_TEMP_SYSTEM:
            case LOG_ERROR_TEMP_LIGHT_MODULE:
            case LOG_ERROR_TEMP_DMD:
            case LOG_ERROR_TEMP_TEC:
            case LOG_ERROR_TEMP_RLD:
                ErrorType = GEC_ERROR_TYPE_TEMP;
                break;

            case LOG_ERROR_FAN_CALIBRATION:
            case LOG_ERROR_FAN_LOCK:
            case LOG_ERROR_FAN_STALL:
                ErrorType = GEC_ERROR_TYPE_FAN;
                break;

            case LOG_ERROR_COVER_INTERLOCK_SWITCH:
                ErrorType = GEC_ERROR_TYPE_COVER;
                break;

            case LOG_ERROR_OTHER_WHEEL:
            case LOG_ERROR_OTHER_I2C:
                ErrorType = GEC_ERROR_TYPE_OTHER;
                break;
        }
    }

    return ErrorType;
}
//-------------------------------------------------------------------------------------------------
u8_t CmdAp_checkIpv6Info(char *pIpAddr, u32_t Prefix, char *pGateway)
{
    struct in6_addr addr, gw, netmask, ipNet, gwNet;
    u32_t i;
    u32_t fullBytes = Prefix / 8;
    u32_t remainder = Prefix % 8;

    if(inet_pton(AF_INET6, pIpAddr, &addr) != 1)
    {
        return eLAN_IPV6_DATA_IP_OUT_OF_RANGE;
    }
    else if(IN6_IS_ADDR_UNSPECIFIED(&addr))
    {
        return eLAN_IPV6_DATA_IP_CLASS_FAIL;
    }
    else if(IN6_IS_ADDR_MULTICAST(&addr))
    {
        return eLAN_IPV6_DATA_IP_CLASS_FAIL;
    }
    else if(IN6_IS_ADDR_LOOPBACK(&addr))
    {
        return eLAN_IPV6_DATA_IP_CLASS_FAIL;
    }
    else if(IN6_IS_ADDR_LINKLOCAL(&addr))
    {
        return eLAN_IPV6_DATA_IP_CLASS_FAIL;
    }

    if(Prefix > 128)
    {
        return eLAN_IPV6_DATA_PREFIX_OUT_OF_RANGE;
    }

    if((pGateway != NULL) && (strlen(pGateway) > 0))
    {
        if(inet_pton(AF_INET6, pGateway, &gw) != 1)
        {
            return eLAN_IPV6_DATA_GATEWAY_OUT_OF_RANGE;
        }
        else if(IN6_IS_ADDR_UNSPECIFIED(&gw))
        {
            return eLAN_IPV6_DATA_EMPTY_GATEWAY;
        }
        else if(IN6_IS_ADDR_MULTICAST(&gw))
        {
            return eLAN_IPV6_DATA_EMPTY_GATEWAY;
        }
        else if(IN6_IS_ADDR_LOOPBACK(&gw))
        {
            return eLAN_IPV6_DATA_EMPTY_GATEWAY;
        }
        else if(!IN6_IS_ADDR_LINKLOCAL(&gw))
        {
            if(Prefix > 0)
            {
                memset(&netmask, 0, sizeof(netmask));
                for (i = 0; i < fullBytes; i++)
                {
                    netmask.s6_addr[i] = 0xFF;
                }
                if(remainder > 0 && fullBytes < 16)
                {
                    netmask.s6_addr[fullBytes] = (0xFF << (8 - remainder)) & 0xFF;
                }

                for (i = 0; i < 16; i++)
                {
                    ipNet.s6_addr[i] = addr.s6_addr[i] & netmask.s6_addr[i];
                    gwNet.s6_addr[i] = gw.s6_addr[i] & netmask.s6_addr[i];
                }
                if(memcmp(&ipNet, &gwNet, sizeof(ipNet)) != 0)
                {
                    return eLAN_IPV6_DATA_GATEWAY_NOT_MATCH_IP;
                }
            }
        }
    }
    return eLAN_IPV6_DATA_PASS;
}
//-------------------------------------------------------------------------------------------------
u8_t CmdAp_lanIpv6SuggestedApply(u8_t faildEvent, char *pIpAddr, u32_t *pPrefix, char *pGateway)
{
    char suggested_ip[MAX_LENGTH_NET_INFO_STR], suggested_subnet[MAX_LENGTH_NET_INFO_STR], suggested_gateway[MAX_LENGTH_NET_INFO_STR];
    u8_t Buffer[1024];
    u32_t Argc;
    char *pArgv[32][2];
    char DebugStr[128];

    if((pIpAddr == NULL) || (pGateway == NULL))
        return 1;

    switch(faildEvent)
    {
        case eLAN_IPV6_DATA_PASS:
            {
                // Do nothing
            }
            break;
        default:
            {
                if(access(PATH_DEFAULT_LAN_IPV6_CONFIG_FILE,0) == 0)
                {
                    Argc = readConf(PATH_DEFAULT_LAN_IPV6_CONFIG_FILE, pArgv, 32, Buffer, sizeof(Buffer));
                    if(Argc == 0)
                    {
                        return 2;
                    }
                    snprintf(pIpAddr, INET6_ADDRSTRLEN, "%s", findArgValue(NAME_ITEM_LAN_IPV6_IP, pArgv, Argc));
                    *pPrefix = atoi(findArgValue(NAME_ITEM_LAN_IPV6_SUBMASK, pArgv, Argc));
                    snprintf(pGateway, INET6_ADDRSTRLEN, "%s", findArgValue(NAME_ITEM_LAN_IPV6_GATEWAY, pArgv, Argc));
                }
                else
                {
                    return 3;
                }
            }
            break;
    }

    return 0;
}
