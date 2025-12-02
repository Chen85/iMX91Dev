#ifndef UTILHOSTAPI_H
#define UTILHOSTAPI_H
// ==============================================================================
// FILE NAME: UTILHOSTAPI.H
// DESCRIPTION:
//
//
// modification history
// --------------------
// 21/11/2013, Leo Create
// --------------------
// ==============================================================================

#include "Common.h"
#include "utilCLICmdAPI.h"
#include "utilCommonMSSCAPI.h"

#define MAX_IPC_PKT     (64)    //A35G2_Simon_0064

#define MAX_MSG_PKT     (10)
#define MSG_RETRY       (5)
#define MAX_PACKET_SIZE (sizeof(sPAYLOAD)/sizeof(UINT8))

#define WRITE_ACK_NEED  (0xFA)
#define WRITE_ACK_REPLY (0xFB)
#define READ_ACK_NEED   (0xFC)
#define READ_ACK_REPLY  (0xFD)

#pragma pack(push)  /* push current alignment to stack */
#pragma pack(1)     /* set alignment to 1 byte boundary */


typedef enum
{
    eMSG_TYPE_UART_1,
    eMSG_TYPE_UART_2,
#if defined(PLATFORM_R70K) || defined(PLATFORM_H30_4K) //HICC2_AC_0002
    eMSG_TYPE_UART_3,
#endif
    eMSG_TYPE_NUMBERS,
} eMSG_TYPE;

typedef enum
{
    eMSG_STATE_MAGIC_NUMBER,
    eMSG_STATE_PACKET_HEADER,
    eMSG_STATE_PACKET_PAYLOAD,
    eMSG_STATE_DATA_READY,
    eMSG_STATE_BAD_PACKET,
    eMSG_STATE_TIMEOUT,
    eMSG_STATE_RUN_OUT_OF_MEMORY,

    eMSG_STATE_NUMBERS,
} eMSG_STATE;


typedef struct
{
    UINT8       ucPacketType;
    UINT8       ucSeqId;
    UINT16      uiPacketSize;
    UINT16      uiChecksum;
} sMSG_PACKET_HEADER;

typedef struct
{
    UINT8                   ucMagicNumber1;
    UINT8                   ucMagicNumber2;
    sMSG_PACKET_HEADER      sPacketHeader;

    union
    {
        sPAYLOAD            sPayLoad;
        UINT8               aucPacketPayload[MAX_PACKET_SIZE];
    }uFormat;
} sMSG_PACKET_FORMAT;

typedef struct
{
    UINT8                   ucMagicNumber1;
    UINT8                   ucMagicNumber2;
    sMSG_PACKET_HEADER      sPacketHeader;
    uMODULE_ID              uModule;
    UINT8                   ucSubCmd;
    UINT8                   ucAckType;
} sMSG_PACKET_ACK_FORMAT;

typedef struct
{
    sMSG_PACKET_FORMAT      sMsgPacket;
    eMSG_STATE              eMsgParsingState;
    UINT16                  wRecivedByteCount;
    UINT16                  wRecivedByteCRC;
} sMSG_STATE_DATA;

#pragma pack(pop)   /* restore original alignment from stack */

#define MSG_PACKET_SIZE sizeof(sMSG_PACKET_FORMAT)  //G100_Simon_0060

void utilHost_Init(eMSG_TYPE eMsgType);
void utilHost_UnPackProcess(void);
void utilHost_ExecuteProcess(void);
void utilHost_MessageSend(sMSG_PACKET_FORMAT *psPacket);
void utilHost_PacketBuild(sMSG_PACKET_FORMAT *psPacket, eMSG_TYPE eMsgType, UINT8 ucSeqID, UINT16 wDataSize, UINT8 *pcBuffer);
eRESULT utilHostCtrlPacketChecksumCheck(sMSG_PACKET_FORMAT *psPacket);
eRESULT utilHost_PayloadSend(UINT8 *pcData);
void utilHost_IPC_Init(void);
UINT8 utilHost_IPC_Ready(void);
eRESULT utilHost_EventWrite(const eCMD_MODULE eModule, const UINT8 cType, const UINT8 cSubCmd, const UINT16 wDataSize, UINT8 *pcData, BOOL bACK);
eRESULT utilHost_EventRead(const eCMD_MODULE eModule, const UINT8 cType, const UINT8 cSubCmd, const UINT16 wDataSize, UINT8 *pcData);
eRESULT utilHost_SystemUartWrite(eCMD_MODULE eModule, UINT8 cSubCmd, UINT16 wDataSize, UINT8 *pcData, BOOL bACK);
eRESULT utilHost_SystemUartRead(eCMD_MODULE eModule, UINT8 cSubCmd, UINT16 wDataSize, UINT8 *pcData);
eRESULT utilHost_SystemSet(eCMD_MODULE eModule, UINT8 cSubCmd, UINT16 wDataSize, UINT8 *pcData);
eRESULT utilHost_SystemGet(eCMD_MODULE eModule, UINT8 cSubCmd, UINT16 wDataSize, UINT8 *pcData);
eRESULT utilHost_FrontEndSet(eCMD_MODULE eModule, UINT8 cSubCmd, UINT16 wDataSize, UINT8 *pcData);
eRESULT utilHost_FrontEndGet(eCMD_MODULE eModule, UINT8 cSubCmd, UINT16 wDataSize, UINT8 *pcData);
void utilHost_test(void);
static void utilHost_Cmd_Execute(void);

#if defined(PLATFORM_R70K) || defined(PLATFORM_H30_4K) //HICC2_AC_0002
#define GEC_CMD_HEADER_1        '<'
#define GEC_CMD_END_1           '>'

#define TESTER_CMD_HEADER_1     '*'
#define TESTER_CMD_END_1        '\r'

#define GEC_UART_WINDOW     (50)
#define GEC_CMD_SIZE        (31)
#define GEC_SUB_SIZE        (7)
#define GEC_DATA_SIZE       (191)

#define CLI_COM_NORMAL              (0)

typedef enum
{
    eCLI_DECODE_STEP_GEC_HEADER = 0,
    eCLI_DECODE_STEP_GEC_CMD,
    eCLI_DECODE_STEP_GEC_SUB1,
    eCLI_DECODE_STEP_GEC_SUB2,
    eCLI_DECODE_STEP_GEC_DATA,
    eCLI_DECODE_STEP_GEC_END,

}eCLI_DECODE_STEP_GEC;

typedef struct
{
    UINT8  ucInterFace;
    UINT8  ucCLI_DATA[255];
    UINT16 uiCLI_Front;
    UINT16 uiCLI_End;
    UINT16 uiCLI_Poll_Position;
    UINT16 uiCLI_Length;
}sGEC_CLI_DATA_INFO;

typedef struct
{
    INT32  lCmdFrom;                            //from PC or telnet ...
    char   acCommand[GEC_CMD_SIZE + 1];         //+1 for '\0'
    char   acSub1[GEC_SUB_SIZE + 1];
    char   acSub2[GEC_SUB_SIZE + 1];

    char   acTextString[GEC_DATA_SIZE + 1];     //store text prameter in Data. //+1 for '\0'
    char   acReturnString[GEC_DATA_SIZE + 1];

    INT32  lFuncID;
    INT32  lAccessMode;                         //Write, Read
    INT32  lSubCode1;
    INT32  lSubCode2;
    INT32  lErrorCode;
    UINT32 ulFlag;
}sCLI_FORMAT_GEC;

typedef struct
{
    eCLI_MODE   eAccessMode;
    UINT8       ucCmdFrom;
    char        acTextString[GEC_DATA_SIZE];
    INT32       lData;
    FLOAT       fData;

}sGEC_CLI_COMMON_FORMAT;

typedef struct
{
    UINT32 ulCmdID;
    char*  pcCmdString;

    eCLI_ERROR_CODE (*iCLI_Replay)(sGEC_CLI_COMMON_FORMAT* psCmdFormat);

    UINT32 ulSpecialFlag; //read only, write only,...

}sCLI_GEC_LUT;

eMSG_STATE utilHost_GEC_StateProcess(eMSG_TYPE eMsgType);
#endif

#endif //UTILHOSTPROCAPI_H


