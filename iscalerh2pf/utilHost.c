// ==============================================================================
// FILE NAME: UTILHOST.C
// DESCRIPTION:
//
//
// modification history
// --------------------
// 12/07/2013, Leo written
// --------------------
// ==============================================================================


#include "Board_Uart.h"
#include "Board_SPI.h"
#include "Board_I2C.h"
#include "utilHostAPI.h"
#include "utilCounterAPI.h"
#include "utilDbgMsg.h"
//#include "utilGlobalAPI.h"
#include "utilQueueAPI.h"
#include "utilCommonMSSCAPI.h"
#include "halMotorCtrlAPI.h"
#include "opdCtrlAPI.h"
#include "palMotorMgr.h"
#include "appLedProcAPI.h"
#include "dvMCUDriver.h"
#include "dvFrontEndDriver.h"
#include "dvInterfaceDiag.h"
#include "dvMotorBoard.h"
#include "appEnvironment.h"
#include "CommonType.h"


#define MAGICNUMBER1    0x55    // Magic Number 1
#define MAGICNUMBER2    0x55    // Magic Number 2
#define MAGICNUMBERSIZE (2)     // Magic Number Size

#define HEADERSIZE      ((sizeof(sMSG_PACKET_HEADER)/sizeof(UINT8)) + 2) //Main + sub cmd //A70LV_Larry_0014
#define MODULESIZE      (2) //Main + sub cmd //A70LV_Larry_0014

// 解封包狀態機資訊
static sMSG_STATE_DATA  m_sMsgState[eMSG_TYPE_NUMBERS];     // SM of Packet Type Data

static sQUEUE m_sHostCmdRingBuffer;
static sMSG_PACKET_FORMAT m_asHostCmdBuffer[MAX_MSG_PKT];
static sMSG_PACKET_FORMAT m_sInPacket;
static sMSG_PACKET_FORMAT m_sOutPacket;

static mqd_t m_qMsgACK = 0;

static UINT8 ucWaitRead = FALSE ;

static sQUEUE m_sPayloadRingBuffer;
static sPAYLOAD m_asPayloadBuffer[MAX_IPC_PKT];
static sPAYLOAD m_sInPayloadPacket;
static UINT8 ucIPC_Ready = FALSE;


static TickType_t ulHostStartTicks[eMSG_TYPE_NUMBERS];

pthread_mutex_t axHost_Mutex[eMSG_TYPE_NUMBERS] ;
#define HOST_SEMPHORE_WAIT_DELAY    (5000)
#define HOST_MutexCreate(x)          pthread_mutex_init(&axHost_Mutex[x], NULL)

eRESULT HostMutexTake(uint8 ucBus)
{
    struct timespec s_timeout;
    clock_gettime(CLOCK_REALTIME, &s_timeout);
    util_TimespecAddms(&s_timeout, HOST_SEMPHORE_WAIT_DELAY);

    if(pthread_mutex_timedlock(&axHost_Mutex[ucBus] , &s_timeout) != 0)
    {
        LOG_MSG(db_ALWAYS, "015 %s pthread_mutex_timedlock fail [%s]\n", __FUNCTION__ , strerror(errno));
        return rcERROR;
    }

    return rcSUCCESS;  //pass
}

eRESULT HostMutexGive(uint8 ucBus)
{
    if(pthread_mutex_unlock(&axHost_Mutex[ucBus]) != 0)
    {
        LOG_MSG(db_ALWAYS, "%s pthread_mutex_unlock fail [%s]\n", __FUNCTION__ , strerror(errno));
        return rcERROR;
    }

    return rcSUCCESS;  //pass
}

static void utilHost_ResetTimeInState(eMSG_TYPE eMsgType)
{
    ulHostStartTicks[eMsgType] = xTaskGetTickCount();
}

UINT32 utilHost_TimeElapsedInState(eMSG_TYPE eMsgType)
{
    TickType_t ulTicks;

    ulTicks = xTaskGetTickCount();

    if(ulHostStartTicks[eMsgType] > ulTicks)
    {
        return (0xFFFFFFFF - ulHostStartTicks[eMsgType] + 1 + ulTicks) / portTICK_RATE_MS;
    }
    else
    {
        return (ulTicks - ulHostStartTicks[eMsgType]) / portTICK_RATE_MS;
    }
}

// ==============================================================================
// FUNCTION NAME: utilHost_MsgACK_Init
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
// 2020/08/28, Larry Create
// --------------------
// ==============================================================================
eRESULT utilHost_MsgACK_Init(void)
{
    if(utilQueueMessageInit(&m_qMsgACK, "/MQUEUE_MSGACK", 1, MSG_PACKET_SIZE) == rcSUCCESS)
    {
        return rcSUCCESS;
    }

    return rcERROR;
}

// ==============================================================================
// FUNCTION NAME: utilHost_MsgACK_Send
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
// 2020/08/28, Larry Create
// --------------------
// ==============================================================================
eRESULT utilHost_MsgACK_Send(UINT8 *pcData, UINT16 wSize)
{
    if(utilQueueMessageSend(m_qMsgACK, pcData, wSize, 0) == rcSUCCESS)
    {
        return rcSUCCESS;
    }

    return rcERROR;
}

// ==============================================================================
// FUNCTION NAME: utilHost_MsgACK_Receive
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
// 2020/08/28, Larry Create
// --------------------
// ==============================================================================
eRESULT utilHost_MsgACK_Receive(UINT8 *pcData, UINT16 wSize, UINT16 wTimeOut)
{
    if(utilQueueMessageReceive(m_qMsgACK, pcData, wSize, wTimeOut) == rcSUCCESS)
    {
        return rcSUCCESS;
    }

    return rcERROR;
}

// ==============================================================================
// FUNCTION NAME: UTILHOST_INPUTBUFFERREAD
// DESCRIPTION:
//
//
// Params:
// eMSG_TYPE eMsgType:
// UINT8 *pcBuffer:
//
// Returns:
//
//
// modification history
// --------------------
// 12/07/2013, Leo written
// --------------------
// ==============================================================================
static eRESULT utilHost_InputBufferRead(eMSG_TYPE eMsgType, UINT8 *pcBuffer)
{
    eRESULT eResult = rcERROR;

    switch(eMsgType)
    {
        case eMSG_TYPE_UART_1:
            if(Board_Uart_Read(uaMCU, 1, pcBuffer))
            {
                LOG_MSG(db_UTL_CMD, "R[%02x]\r\n", *pcBuffer);
                eResult = rcSUCCESS;
            }
            break;

        case eMSG_TYPE_UART_2: //A70LV_Larry_0014
            break;

#if defined(PLATFORM_R70K) || defined(PLATFORM_H30_4K)
        case eMSG_TYPE_UART_3:
            if(Board_Uart_Read(uaMotor, 1, pcBuffer))
            {
                LOG_MSG(db_UTL_CMD, "UART_3 R[%02x]\r\n", *pcBuffer);
                eResult = rcSUCCESS;
            }
            break;
#endif

        default:
            ASSERT_ALWAYS();
            break;
    }

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: UTILHOST_OUTPUTBUFFERWRITE
// DESCRIPTION:
//
//
// Params:
// eMSG_TYPE eMsgType:
// UINT16 wSize:
// UINT8 *pcBuffer:
//
// Returns:
//
//
// modification history
// --------------------
// 15/07/2013, Leo written
// --------------------
// ==============================================================================
static eRESULT utilHost_OutputBufferWrite(eMSG_TYPE eMsgType, UINT16 wSize, UINT8 *pcBuffer)
{
    eRESULT eResult = rcERROR;

    switch(eMsgType)
    {
        case eMSG_TYPE_UART_1:
            eResult = Board_Uart_Write(uaMCU, wSize, pcBuffer);
            break;

        case eMSG_TYPE_UART_2: //A70LV_Larry_0014
            break;

        default:
            ASSERT_ALWAYS();
            break;
    }

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: UTILHOST_STATERESET
// DESCRIPTION:
//
//
// Params:
// eMSG_TYPE eMsgType:
//
// Returns:
//
//
// modification history
// --------------------
// 12/07/2013, Leo written
// --------------------
// ==============================================================================
static void utilHost_StateReset(eMSG_TYPE eMsgType)
{
    memset(&m_sMsgState[eMsgType], 0, sizeof(sMSG_STATE_DATA));  //G100_Simon_0060
}

// ==============================================================================
// FUNCTION NAME: UTILHOST_MESSAGESEND
// DESCRIPTION:
//
//
// Params:
// sMSG_PACKET_FORMAT *psPacket:
// UINT16 wSize:
// UINT8 *pcBuffer:
//
// Returns:
//
//
// modification history
// --------------------
// 15/07/2013, Leo written
// --------------------
// ==============================================================================
void utilHost_MessageSend(sMSG_PACKET_FORMAT *psPacket)
{
    utilHost_OutputBufferWrite((eMSG_TYPE)psPacket->sPacketHeader.ucPacketType,
                               (psPacket->sPacketHeader.uiPacketSize + HEADERSIZE + MAGICNUMBERSIZE),
                               (UINT8 *)psPacket);
}

// ==============================================================================
// FUNCTION NAME: UTILHOST_STATEPROCESS
// DESCRIPTION:
//
// |---------------------------------------------------------------------------|
// |Magic Number|                       Packet Header           |Packet Payload|
// |---------------------------------------------------------------------------|
// |            |Packet Type|Packet Seq |Packet Size|Packet CRC |   Data       |
// |---------------------------------------------------------------------------|
// |  2 bytes   |   1 byte  |   1 byte  |   2 bytes |   2 bytes |   N bytes    |
// |---------------------------------------------------------------------------|
//
// Params:
// eMSG_TYPE eMsgType:
// sMSG_PACKET_FORMAT *psMsgPacket:
//
// Returns:
//
//
// modification history
// --------------------
// 12/07/2013, Leo written
// --------------------
// ==============================================================================
static eMSG_STATE utilHost_StateProcess(eMSG_TYPE eMsgType, sMSG_STATE_DATA *psMsgData)
{
    UINT8 cReadBuffer = 0;

    while(rcSUCCESS == utilHost_InputBufferRead(eMsgType, &cReadBuffer))
    {
        //utilCounterSet(m_sMsgTimeoutCount[eMsgType], ONE_SECOND); //A70LV_Larry_0149

        utilHost_ResetTimeInState(eMsgType);

        switch(m_sMsgState[eMsgType].eMsgParsingState)
        {
            case eMSG_STATE_MAGIC_NUMBER:
            {
                psMsgData->sMsgPacket.ucMagicNumber2 = cReadBuffer;

                if((MAGICNUMBER1 == psMsgData->sMsgPacket.ucMagicNumber1) && (MAGICNUMBER2 == psMsgData->sMsgPacket.ucMagicNumber2))
                {
                    m_sMsgState[eMsgType].eMsgParsingState = eMSG_STATE_PACKET_HEADER;
                    m_sMsgState[eMsgType].wRecivedByteCount = 0;
                    m_sMsgState[eMsgType].wRecivedByteCRC = 0;
                }
                else
                {
                    psMsgData->sMsgPacket.ucMagicNumber1 = psMsgData->sMsgPacket.ucMagicNumber2;
                }
            }
            break;

            case eMSG_STATE_PACKET_HEADER:
            {
                UINT8 *pcBuffer = (UINT8 *)&psMsgData->sMsgPacket.sPacketHeader;
                *(pcBuffer + m_sMsgState[eMsgType].wRecivedByteCount++) = cReadBuffer;

                if(HEADERSIZE == m_sMsgState[eMsgType].wRecivedByteCount)
                {
                    if((MAX_PACKET_SIZE - MODULESIZE) >= psMsgData->sMsgPacket.sPacketHeader.uiPacketSize) //A70LV_Larry_0014
                    {
                        m_sMsgState[eMsgType].wRecivedByteCount = 0;
                        m_sMsgState[eMsgType].eMsgParsingState = eMSG_STATE_PACKET_PAYLOAD;

                        // 計算CRC
                        m_sMsgState[eMsgType].wRecivedByteCRC += psMsgData->sMsgPacket.sPacketHeader.ucPacketType;
                        m_sMsgState[eMsgType].wRecivedByteCRC += psMsgData->sMsgPacket.sPacketHeader.ucSeqId;
                        m_sMsgState[eMsgType].wRecivedByteCRC += (psMsgData->sMsgPacket.sPacketHeader.uiPacketSize & 0xFF00) >> 8;
                        m_sMsgState[eMsgType].wRecivedByteCRC += (psMsgData->sMsgPacket.sPacketHeader.uiPacketSize & 0x00FF);
                        m_sMsgState[eMsgType].wRecivedByteCRC += psMsgData->sMsgPacket.uFormat.sPayLoad.uModule.ucModule;
                        m_sMsgState[eMsgType].wRecivedByteCRC += psMsgData->sMsgPacket.uFormat.sPayLoad.ucSubCmd;

                        // 不帶Payload資料,檢查CRC
                        if(0 == psMsgData->sMsgPacket.sPacketHeader.uiPacketSize)
                        {
                            if(0 == (0xFFFF & (psMsgData->sMsgPacket.sPacketHeader.uiChecksum +
                                               m_sMsgState[eMsgType].wRecivedByteCRC)))
                            {
                                m_sMsgState[eMsgType].eMsgParsingState = eMSG_STATE_DATA_READY;
                            }
                            else
                            {
                                LOG_MSG(db_UTL_CMD, "CRC ERROR 1\r\n");
                                m_sMsgState[eMsgType].eMsgParsingState = eMSG_STATE_BAD_PACKET;
                            }

                            return m_sMsgState[eMsgType].eMsgParsingState;
                        }
                    }
                    else
                    {
                        LOG_MSG(db_UTL_CMD, "OVER SIZE\r\n");
                        m_sMsgState[eMsgType].eMsgParsingState = eMSG_STATE_BAD_PACKET;
                        return eMSG_STATE_BAD_PACKET;
                    }
                }
            }
            break;

            case eMSG_STATE_PACKET_PAYLOAD:
            {
                m_sMsgState[eMsgType].wRecivedByteCRC += cReadBuffer;
                psMsgData->sMsgPacket.uFormat.sPayLoad.aucData[m_sMsgState[eMsgType].wRecivedByteCount++] = cReadBuffer;

                if(psMsgData->sMsgPacket.sPacketHeader.uiPacketSize == m_sMsgState[eMsgType].wRecivedByteCount)
                {
                    if(0 == (0xFFFF & (psMsgData->sMsgPacket.sPacketHeader.uiChecksum +
                                       m_sMsgState[eMsgType].wRecivedByteCRC)))
                    {
                        m_sMsgState[eMsgType].eMsgParsingState = eMSG_STATE_DATA_READY;
                    }
                    else
                    {
                        LOG_MSG(db_UTL_CMD, "CRC ERROR 2\r\n");
                        m_sMsgState[eMsgType].eMsgParsingState = eMSG_STATE_BAD_PACKET;
                    }

                    return m_sMsgState[eMsgType].eMsgParsingState;
                }
            }
            break;

            default:
                return m_sMsgState[eMsgType].eMsgParsingState;
                //break;
        }
    }

    // Check Timeout
    if((utilHost_TimeElapsedInState(eMsgType) > ONE_SECOND) //A70LV_Larry_0149
        && (eMSG_STATE_MAGIC_NUMBER != m_sMsgState[eMsgType].eMsgParsingState))
    {
        m_sMsgState[eMsgType].eMsgParsingState = eMSG_STATE_TIMEOUT;
    }

    return m_sMsgState[eMsgType].eMsgParsingState;
}

// ==============================================================================
// FUNCTION NAME: UTILHOST_CHECKSUMCALC
// DESCRIPTION:
//
//
// Params:
// sMSG_PACKET_FORMAT *psPacket:
// UINT16 wSize:
// UINT8 *pcBuffer:
//
// Returns:
//
//
// modification history
// --------------------
// 15/07/2013, Leo written
// --------------------
// ==============================================================================
static void utilHost_CheckSumCalc(sMSG_PACKET_FORMAT *psPacket, UINT16 wSize, UINT8 *pcBuffer)
{
    psPacket->sPacketHeader.uiChecksum = 0;
    psPacket->sPacketHeader.uiChecksum += psPacket->sPacketHeader.ucPacketType;
    psPacket->sPacketHeader.uiChecksum += psPacket->sPacketHeader.ucSeqId;
    psPacket->sPacketHeader.uiChecksum += (psPacket->sPacketHeader.uiPacketSize & 0xFF00) >> 8;
    psPacket->sPacketHeader.uiChecksum += (psPacket->sPacketHeader.uiPacketSize & 0x00FF);
    psPacket->sPacketHeader.uiChecksum += psPacket->uFormat.sPayLoad.uModule.ucModule;
    psPacket->sPacketHeader.uiChecksum += psPacket->uFormat.sPayLoad.ucSubCmd;

    while(wSize--)
    {
        psPacket->sPacketHeader.uiChecksum += *(pcBuffer++);
    }

    psPacket->sPacketHeader.uiChecksum = (~psPacket->sPacketHeader.uiChecksum + 1) & 0xFFFF;
}

// ==============================================================================
// FUNCTION NAME: UTILHOST_ACKSEND
// DESCRIPTION:
//
// |---------------------------------------------------------------------------|
// |Magic Number|                       Packet Header           |   ACK        |
// |---------------------------------------------------------------------------|
// |            |Packet Type|Packet Seq |Packet Size|Packet CRC |   Data       |
// |---------------------------------------------------------------------------|
// |  2 bytes   |   1 byte  |   1 byte  |   2 bytes |   2 bytes |   1 bytes    |
// |---------------------------------------------------------------------------|
//
// Ack 封包大小 = Header Size + 1 Byte Ack
//
// Params:
// eMSG_TYPE eMsgType:
// eACK_TYPE eAckType:
//
// Returns:
//
//
// modification history
// --------------------
// 15/07/2013, Leo written
// --------------------
// ==============================================================================
static void utilHost_AckSend(sMSG_PACKET_FORMAT *psPacket, eACK_TYPE eAckType)
{
    // Ack 封包大小 = 1 Byte Ack
    psPacket->sPacketHeader.uiPacketSize = 1;
    psPacket->uFormat.sPayLoad.aucData[0] = (UINT8)eAckType;

    utilHost_CheckSumCalc(psPacket,
                          psPacket->sPacketHeader.uiPacketSize,
                          psPacket->uFormat.sPayLoad.aucData);

    // Push in Output Ring Buffer
    utilHost_MessageSend(psPacket);
}

// ==============================================================================
// FUNCTION NAME: utilHost_Packet_UnPack
// DESCRIPTION:
//
//
// Params:
// eMSG_TYPE eMsgType:
//
// Returns:
//
//
// modification history
// --------------------
// 2014/03/30, Leo Create
// --------------------
// ==============================================================================
static void utilHost_Packet_UnPack(eMSG_TYPE eMsgType)
{
    eMSG_STATE eMsgParsingState = eMSG_STATE_MAGIC_NUMBER;

    eMsgParsingState = utilHost_StateProcess(eMsgType, &m_sMsgState[eMsgType]);

    switch(eMsgParsingState)
    {
        case eMSG_STATE_MAGIC_NUMBER:
        case eMSG_STATE_PACKET_HEADER:
        case eMSG_STATE_PACKET_PAYLOAD:
            // Packet unpacking
            break;

        case eMSG_STATE_DATA_READY:
        {
            if((eMsgType == eMSG_TYPE_UART_1) &&
               ((m_sMsgState[eMsgType].sMsgPacket.sPacketHeader.ucSeqId == WRITE_ACK_REPLY) ||
                (m_sMsgState[eMsgType].sMsgPacket.sPacketHeader.ucSeqId == READ_ACK_REPLY)))
            {
                utilHost_MsgACK_Send((UINT8 *)&m_sMsgState[eMsgType].sMsgPacket, MSG_PACKET_SIZE);
            }
            else
            {
                if(rcERROR == utilQueueWrite(&m_sHostCmdRingBuffer, (UINT8 *)&m_sMsgState[eMsgType].sMsgPacket))
                {
                    LOG_MSG(db_UTL_CMD, "Queue Full\r\n");
                }
            }
            // Restart parsing process
            utilHost_StateReset(eMsgType);
        }
        break;

        case eMSG_STATE_BAD_PACKET:
            // Build Ack and push in output ring buffer
            LOG_MSG(db_UTL_CMD, "BAD_PACKET\r\n");
            //m_sMsgState[eMsgType].sMsgPacket.sPacketHeader.ucPacketType = eMsgType;
            //utilHost_AckSend(&m_sMsgState[eMsgType].sMsgPacket, eACK_TYPE_BADPACKET);
            utilHost_StateReset(eMsgType);
            break;

        case eMSG_STATE_TIMEOUT:
            LOG_MSG(db_UTL_CMD, "TIMEOUT\r\n");
            // Build Ack and push in output ring buffer
            //m_sMsgState[eMsgType].sMsgPacket.sPacketHeader.ucPacketType = eMsgType;
            //utilHost_AckSend(&m_sMsgState[eMsgType].sMsgPacket, eACK_TYPE_TIMEOUT);  //A70LV_Larry_0068
            utilHost_StateReset(eMsgType);
            break;

        case eMSG_STATE_RUN_OUT_OF_MEMORY:
            // Build Ack and push in output ring buffer
            //m_sMsgState[eMsgType].sMsgPacket.sPacketHeader.ucPacketType = eMsgType;
            //utilHost_AckSend(&m_sMsgState[eMsgType].sMsgPacket, eACK_TYPE_BADPACKET);
            utilHost_StateReset(eMsgType);
            break;

        default:
            break;
    }
}

// ==============================================================================
// FUNCTION NAME: UTILHOST_CMD_EXCUTE
// DESCRIPTION:
//
//
// Params:
// void:
//
// Returns:
//
//
// modification history
// --------------------
// 2014/03/30, Leo Create
// --------------------
// ==============================================================================
static void utilHost_Cmd_Execute(void)
{
    eACK_TYPE eAckType = eACK_TYPE_NUMBERS;

    if(rcSUCCESS == utilQueueRead(&m_sHostCmdRingBuffer, (UINT8 *)&m_sInPacket))
    {
        // 拷貝InPacket中資訊給OutPacket
        m_sOutPacket = m_sInPacket;

        switch(m_sInPacket.sPacketHeader.ucPacketType)
        {
            //用UART 不回應 ACK
            case eMSG_TYPE_UART_1:
            case eMSG_TYPE_UART_2:
            case 4:
                {
                    if((m_sInPacket.sPacketHeader.ucSeqId == WRITE_ACK_REPLY) ||
                       (m_sInPacket.sPacketHeader.ucSeqId == READ_ACK_REPLY))
                    {
                        //utilHost_MsgACK_Send((UINT8 *)&m_sInPacket, MSG_PACKET_SIZE);
                    }
                    else
                    {
                        eAckType = utilCLICmd_Execute((sPAYLOAD *)&m_sInPacket.uFormat.aucPacketPayload,
                                                      (sPAYLOAD *)&m_sOutPacket.uFormat.sPayLoad.aucData,
                                                      (UINT16 *)&m_sOutPacket.sPacketHeader.uiPacketSize);

                        switch(eAckType)
                        {
                            case eACK_TYPE_NONACK:
                                break;

                            case eACK_TYPE_ACK:
                                if(m_sInPacket.sPacketHeader.ucSeqId == WRITE_ACK_NEED)
                                {
                                    m_sOutPacket.sPacketHeader.ucSeqId = WRITE_ACK_REPLY;
                                    utilHost_AckSend(&m_sOutPacket, eACK_TYPE_ACK);
                                }
                                break;

                            case eACK_TYPE_UNKNOWN:
                                //utilHost_AckSend(&m_sInPacket, eACK_TYPE_UNKNOWN);  //A70LV_Larry_0068
                                break;

                            case eACK_TYPE_FEEDBACK:
                                {
                                    if(m_sOutPacket.sPacketHeader.ucSeqId == READ_ACK_NEED)
                                    {
                                        m_sOutPacket.sPacketHeader.ucSeqId = READ_ACK_REPLY;
                                    }
                                    // Build Output Packet
                                    utilHost_CheckSumCalc(&m_sOutPacket,
                                                           m_sOutPacket.sPacketHeader.uiPacketSize,
                                                           m_sOutPacket.uFormat.sPayLoad.aucData);

                                    utilHost_MessageSend(&m_sOutPacket);

                                    if(utilDbgMsg_Get(db_UTL_CMD))
                                    {
                                        UINT16 wCount = 0;

                                        LOG_MSG(db_UTL_CMD, "CRC=0x%04x, Main = %d, Sub = %d, Size=%d,(",
                                                                   m_sOutPacket.sPacketHeader.uiChecksum,
                                                                   m_sOutPacket.uFormat.sPayLoad.uModule.sBIT.ucModuleID,
                                                                   m_sOutPacket.uFormat.sPayLoad.ucSubCmd,
                                                                   m_sOutPacket.sPacketHeader.uiPacketSize);

                                        for(wCount = 0; wCount < m_sOutPacket.sPacketHeader.uiPacketSize; wCount++)
                                        {
                                            LOG_MSG(db_UTL_CMD, "%x,", m_sOutPacket.uFormat.sPayLoad.aucData[wCount]);
                                        }

                                        LOG_MSG(db_UTL_CMD, ")\r\n");
                                    }
                                }
                                break;

                            case eACK_TYPE_ERROR:
                                utilHost_AckSend(&m_sInPacket, eACK_TYPE_ERROR);
                                break;

                            default:
                                ASSERT_ALWAYS();
                                break;
                        }

                        dvUART_Dev_RunTimeCount_Set(eUART_STANDBY, eAckType); //A65_OPTOMA_Julie_0023
                    }
                }
                break;

            default:
                break;
        } // End of Switch
    }
}

// ==============================================================================
// FUNCTION NAME: utilHost_IPC_Execute
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
// 2020/03/13, Larry Create
// --------------------
// ==============================================================================
static void utilHost_IPC_Execute(void)
{
    eACK_TYPE eAckType = eACK_TYPE_NUMBERS;

    if(rcSUCCESS == utilQueueRead(&m_sPayloadRingBuffer, (UINT8 *)&m_sInPayloadPacket))
    {
        sPAYLOAD sOutPayloadPacket;
        UINT16 uiSize = 0;

        eAckType = utilCLICmd_Execute(&m_sInPayloadPacket,
                                      &sOutPayloadPacket,
                                      (UINT16 *)&uiSize);

        switch(eAckType)
        {
            case eACK_TYPE_NONACK:
                break;

            case eACK_TYPE_ACK:
                break;

            case eACK_TYPE_UNKNOWN:
                break;

            case eACK_TYPE_FEEDBACK:
                break;

            case eACK_TYPE_ERROR:
                break;

            default:
                ASSERT_ALWAYS();
                break;
        }

    }
}

// ==============================================================================
// FUNCTION NAME: UTILHOST_INIT
// DESCRIPTION:
//
// 初始化Host 介面
//
// Params:
// eMSG_TYPE eMsgType:
//
// Returns:
//
//
// modification history
// --------------------
// 12/07/2013, Leo written
// --------------------
// ==============================================================================
void utilHost_Init(eMSG_TYPE eMsgType)
{
    HOST_MutexCreate(eMsgType);

    utilHost_MsgACK_Init();

    // Output command ring buffer
    utilQueueInitial(&m_sHostCmdRingBuffer,
                     MAX_MSG_PKT,
                     (sizeof(sMSG_PACKET_FORMAT) / sizeof(UINT8)),
                     (UINT8 *)m_asHostCmdBuffer);

    utilHost_StateReset(eMsgType);
}

// ==============================================================================
// FUNCTION NAME: utilHostIPC_Init
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
// 2020/06/15, Larry Create
// --------------------
// ==============================================================================
void utilHost_IPC_Init(void)
{
    //IPC
    utilQueueInitial(&m_sPayloadRingBuffer,
                     MAX_IPC_PKT,
                     (sizeof(sPAYLOAD)),
                     (UINT8 *)m_asPayloadBuffer);

    ucIPC_Ready = TRUE;
}

// ==============================================================================
// FUNCTION NAME: utilHost_IPC_Ready
// DESCRIPTION:
//
//
// Params:
// void:
//
// Returns:
//
//
// Modification History
// --------------------
// 2020/06/15, Larry Create
// --------------------
// ==============================================================================
UINT8 utilHost_IPC_Ready(void)
{
    return ucIPC_Ready;
}
// ==============================================================================
// FUNCTION NAME: UTILHOST_PROCESS
// DESCRIPTION:
//
//
// Params:
// void:
//
// Returns:
//
//
// modification history
// --------------------
// 12/07/2013, Leo written
// --------------------
// ==============================================================================
void utilHost_UnPackProcess(void)
{
    utilHost_Packet_UnPack(eMSG_TYPE_UART_1);
}

// ==============================================================================
// FUNCTION NAME: utilHost_ExecuteProcess
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
// 2020/09/02, Larry Create
// --------------------
// ==============================================================================
void utilHost_ExecuteProcess(void)
{
    utilHost_Cmd_Execute();
    utilHost_IPC_Execute();
}


void utilHost_PacketBuild(sMSG_PACKET_FORMAT *psPacket, eMSG_TYPE eMsgType, UINT8 ucSeqID, UINT16 wDataSize, UINT8 *pcBuffer)
{
    psPacket->ucMagicNumber1 = MAGICNUMBER1;
    psPacket->ucMagicNumber2 = MAGICNUMBER2;
    psPacket->sPacketHeader.ucPacketType = eMsgType;
    psPacket->sPacketHeader.ucSeqId = ucSeqID;
    psPacket->sPacketHeader.uiPacketSize = wDataSize;
    psPacket->uFormat.sPayLoad.uModule.ucModule = pcBuffer[0];
    psPacket->uFormat.sPayLoad.ucSubCmd = *(pcBuffer + 1);    //A35G2_Simon_0070 for cppcheck

    memcpy((UINT8 *)&psPacket->uFormat.sPayLoad.aucData, pcBuffer + 2, wDataSize);   //A35G2_Simon_0071 for cppcheck
    utilHost_CheckSumCalc(psPacket, wDataSize, psPacket->uFormat.sPayLoad.aucData);
}


// ==============================================================================
// FUNCTION NAME: UTILHOSTCTRPACKETCRCCHECK
// DESCRIPTION:
//
//
// Params:
// sMSGPACKETFORMAT *psPacket:
//
// Returns:
//
//
// modification history
// --------------------
// 04/12/2013, Larry written//A70_Larry_0084
// A70_LEO_0004
// --------------------
// ==============================================================================
eRESULT utilHostCtrlPacketChecksumCheck(sMSG_PACKET_FORMAT *psPacket)
{
    eRESULT eResult = rcSUCCESS;
    UINT16 uiChecksum = 0;
    PUINT8 pBuffer = psPacket->uFormat.sPayLoad.aucData;
    UINT16 wCount = psPacket->sPacketHeader.uiPacketSize;

    uiChecksum = 0;
    uiChecksum += psPacket->sPacketHeader.ucPacketType;
    uiChecksum += psPacket->sPacketHeader.ucSeqId;
    uiChecksum += (psPacket->sPacketHeader.uiPacketSize&0xFF00)>>8;
    uiChecksum += (psPacket->sPacketHeader.uiPacketSize&0x00FF);
    uiChecksum += psPacket->sPacketHeader.uiChecksum;
    uiChecksum += psPacket->uFormat.sPayLoad.uModule.ucModule;
    uiChecksum += psPacket->uFormat.sPayLoad.ucSubCmd;

    while(wCount--)
    {
        uiChecksum += *(pBuffer++);
    }

    if((uiChecksum & 0xFFFF))
    {
        eResult = rcERROR;
    }

    return eResult;
}


void utilHost_WaitRead_FlagSet(UINT8 ucWait)
{
    ucWaitRead = ucWait ;
}

// ==============================================================================
// FUNCTION NAME: utilHost_PayloadSend
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
// 2020/03/13, Larry Create
// --------------------
// ==============================================================================
eRESULT utilHost_PayloadSend(UINT8 *pcData)
{
    return utilQueueWrite(&m_sPayloadRingBuffer, pcData);
}

// ==============================================================================
// FUNCTION NAME: utilHost_EventWrite
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
// 2020/08/28, Larry Create
// --------------------
// ==============================================================================
eRESULT utilHost_EventWrite(const eCMD_MODULE eModule, const UINT8 cType, const UINT8 cSubCmd, const UINT16 wDataSize, UINT8 *pcData, BOOL bACK)
{
    eRESULT             eResult = rcSUCCESS;
    sMSG_PACKET_FORMAT  sOutPacket;
    sPAYLOAD            sPayLoad;
    UINT8               ucSeqID = 0;

    sPayLoad.uModule.sBIT.cRW         = CMD_WRITE;
    sPayLoad.uModule.sBIT.ucModuleID  = eModule;
    sPayLoad.ucSubCmd                 = cSubCmd;

    //LOG_MSG(db_ALWAYS,"eModule[%d] cSubCmd[%d] pcData[%d] bACK[%d]\r\n", eModule, cSubCmd, pcData[0], bACK);

    if(bACK)
    {
        if(rcSUCCESS != HostMutexTake(cType))
        {
            ASSERT_ALWAYS();
            return rcERROR;
        }
        utilHost_MsgACK_Receive((UINT8*)&sOutPacket, MSG_PACKET_SIZE, 0); //clean buffer
        ucSeqID = WRITE_ACK_NEED;
    }

    memcpy(sPayLoad.aucData, pcData, wDataSize);

    utilHost_PacketBuild(&sOutPacket, (eMSG_TYPE)cType, ucSeqID, wDataSize, (UINT8*)&sPayLoad);
    utilHost_MessageSend(&sOutPacket);

    if(bACK)
    {
        memset((UINT8*)&sOutPacket, 0, MSG_PACKET_SIZE);

        if(rcSUCCESS == utilHost_MsgACK_Receive((UINT8*)&sOutPacket, MSG_PACKET_SIZE, 100))
        {
            if((sOutPacket.sPacketHeader.ucSeqId == WRITE_ACK_REPLY) &&
               (sOutPacket.uFormat.sPayLoad.uModule.sBIT.ucModuleID == eModule) &&
               (sOutPacket.uFormat.sPayLoad.ucSubCmd == cSubCmd) &&
               (sOutPacket.uFormat.sPayLoad.aucData[0] == eACK_TYPE_ACK))
            {
                //ASSERT_ALWAYS();
                eResult = rcSUCCESS;
            }
            else
            {
                ASSERT_ALWAYS();
                eResult = rcERROR;
            }
        }
        else
        {
           ASSERT_ALWAYS();
           eResult = rcERROR;
        }

        HostMutexGive(cType);
    }

    return eResult;
}

eRESULT utilHost_EventRead(const eCMD_MODULE eModule, const UINT8 cType, const UINT8 cSubCmd, const UINT16 wDataSize, UINT8 *pcData)
{
    eRESULT             eResult = rcSUCCESS;
    sMSG_PACKET_FORMAT  sOutPacket;
    sPAYLOAD            sPayLoad;
    UINT8               ucSeqID = 0;

    sPayLoad.uModule.sBIT.cRW         = CMD_READ;
    sPayLoad.uModule.sBIT.ucModuleID  = eModule;
    sPayLoad.ucSubCmd                 = cSubCmd;

    if(HostMutexTake(cType) == rcSUCCESS)
    {
        ucSeqID = READ_ACK_NEED;

        utilHost_MsgACK_Receive((UINT8*)&sOutPacket, MSG_PACKET_SIZE, 0); //clean buffer

        utilHost_PacketBuild(&sOutPacket, (eMSG_TYPE)cType, ucSeqID, 0, (UINT8*)&sPayLoad);
        utilHost_MessageSend(&sOutPacket);

        memset((UINT8*)&sOutPacket, 0, MSG_PACKET_SIZE);

        if(rcSUCCESS == utilHost_MsgACK_Receive((UINT8*)&sOutPacket, MSG_PACKET_SIZE, 100))
        {
            if((sOutPacket.sPacketHeader.ucSeqId == READ_ACK_REPLY) &&
               (sOutPacket.uFormat.sPayLoad.uModule.sBIT.ucModuleID == eModule) &&
               (sOutPacket.uFormat.sPayLoad.ucSubCmd == cSubCmd))
            {
                //printf("%d, %d", sOutPacket.uFormat.sPayLoad.aucData[0], sOutPacket.uFormat.sPayLoad.aucData[1]);
                //ASSERT_ALWAYS();
                memcpy(pcData, &sOutPacket.uFormat.sPayLoad.aucData[0], wDataSize);
                eResult = rcSUCCESS;
            }
            else
            {
                ASSERT_ALWAYS();
                eResult = rcERROR;
            }
        }
        else
        {
           ASSERT_ALWAYS();
           eResult = rcERROR;
        }

        HostMutexGive(cType);
    }
    else
    {
        eResult = rcERROR;
    }

    return eResult;

}

eRESULT utilHost_SystemUartWrite(eCMD_MODULE eModule, UINT8 cSubCmd, UINT16 wDataSize, UINT8 *pcData, BOOL bACK)
{
#if (MCU_CLI_TYPE == MCU_CLI_TYPE_NA)
	return rcSUCCESS;
#else
	return utilHost_EventWrite(eModule, eMSG_TYPE_UART_1, cSubCmd, wDataSize, pcData, FALSE);
#endif /* MCU_CLI_TYPE */
}

eRESULT utilHost_SystemUartRead(eCMD_MODULE eModule, UINT8 cSubCmd, UINT16 wDataSize, UINT8 *pcData)
{
#if (MCU_CLI_TYPE == MCU_CLI_TYPE_NA)
	return rcSUCCESS;
#else
	return utilHost_EventRead(eModule, eMSG_TYPE_UART_1, cSubCmd, wDataSize, pcData);
#endif /* MCU_CLI_TYPE */
}

eRESULT utilHost_SystemSet(eCMD_MODULE eModule, UINT8 cSubCmd, UINT16 wDataSize, UINT8 *pcData)
{
#if (MCU_CLI_TYPE == MCU_CLI_TYPE_UART)
    return utilHost_EventWrite(eModule, eMSG_TYPE_UART_1, cSubCmd, wDataSize, pcData, FALSE);
#elif (MCU_CLI_TYPE == MCU_CLI_TYPE_I2C)
    return dvMCUDriverRegWrite(eModule, cSubCmd, wDataSize, pcData);
#else
    return rcSUCCESS;
#endif /* MCU_CLI_TYPE */
}

eRESULT utilHost_SystemGet(eCMD_MODULE eModule, UINT8 cSubCmd, UINT16 wDataSize, UINT8 *pcData)
{
#if (MCU_CLI_TYPE == MCU_CLI_TYPE_UART)
    return utilHost_EventRead(eModule, eMSG_TYPE_UART_1, cSubCmd, wDataSize, pcData);
#elif (MCU_CLI_TYPE == MCU_CLI_TYPE_I2C)
    return dvMCUDriverRegRead(eModule, cSubCmd, wDataSize, pcData);
#else
    return rcSUCCESS;
#endif /* MCU_CLI_TYPE */
}

eRESULT utilHost_FrontEndSet(eCMD_MODULE eModule, UINT8 cSubCmd, UINT16 wDataSize, UINT8 *pcData) //G100_Julie_0001
{
    return dvFrontEndDriverRegWrite_2ByteReg(eModule, cSubCmd, wDataSize, pcData);
}

eRESULT utilHost_FrontEndGet(eCMD_MODULE eModule, UINT8 cSubCmd, UINT16 wDataSize, UINT8 *pcData) //G100_Julie_0001
{
    return dvFrontEndDriverRegRead_2ByteReg(eModule, cSubCmd, wDataSize, pcData);
}

eRESULT utilHost_MotorSet(eCMD_MODULE eModule, UINT8 cSubCmd, UINT16 wDataSize, UINT8 *pcData) //G100_Julie_0001
{
    return dvFrontEndDriverRegWrite_2ByteReg(eModule, cSubCmd, wDataSize, pcData);
}

eRESULT utilHost_MotorGet(eCMD_MODULE eModule, UINT8 cSubCmd, UINT16 wDataSize, UINT8 *pcData) //G100_Julie_0001
{
    return dvFrontEndDriverRegRead_2ByteReg(eModule, cSubCmd, wDataSize, pcData);
}


void utilHost_test(void)
{
    static UINT32 sulTotal = 0, sulErrCnt = 0;
    UINT8 ucIdx, ucWData, ucRData;

    for(ucIdx=0; ucIdx<0xFF; ucIdx++)   //G100_Owen_0080
    {
        ucWData = ucIdx;
        utilHost_EventWrite(eCMD_MODULE_OPD, eMSG_TYPE_UART_1, eOPD_MSG_UART_STRESS_TEST, eOPD_MSG_UART_STRESS_TEST_SZ, &ucWData, FALSE);
        MS_SLEEP(10);
        utilHost_EventRead(eCMD_MODULE_OPD, eMSG_TYPE_UART_1, eOPD_MSG_UART_STRESS_TEST, eOPD_MSG_UART_STRESS_TEST_SZ, &ucRData);
        sulTotal++;
        if(ucWData != ucRData)
        {
            sulErrCnt++;
            LOG_MSG(db_ALWAYS, "UART1 W[%X] != R[%X]\r\n", ucWData, ucRData);
        }
    }

    LOG_MSG(db_ALWAYS, "UART1 Total[%d], Error[%d]\r\n", sulTotal, sulErrCnt);
}

void utilHost_Read_test(UINT8 *pcData)
{
    utilHost_EventRead(eCMD_MODULE_OPD, eMSG_TYPE_UART_1, eOPD_MSG_UART_STRESS_TEST, eOPD_MSG_UART_STRESS_TEST_SZ, pcData);
	//utilHost_EventWrite(eCMD_MODULE_SYSTEM, eMSG_TYPE_UART_1, eOPD_MSG_UART_STRESS_TEST, eOPD_MSG_UART_STRESS_TEST_SZ, &ucRand, TRUE);

}

#if defined(PLATFORM_R70K) || defined(PLATFORM_H30_4K) //HICC2_AC_0002

eCLI_ERROR_CODE utilCommon_VersionGet(sGEC_CLI_COMMON_FORMAT* psCmdFormat)
{
    eCLI_ERROR_CODE eErrorCode = eCLI_ERROR_CODE_NO;
    INT32 alData[8] = {0};
    UINT8 aucVerString[32] = {0};

    sscanf(psCmdFormat->acTextString, "%d,%d,%d", &alData[0], &alData[1], &alData[2]);
    LOG_MSG(db_HAL_MCU, "VersionGet %d %d %d\r\n", alData[0], alData[1], alData[2]);

    sprintf((char*)aucVerString, "M%02d.%02d", alData[0], alData[1]);
    palDataMgr_Data_Access(edcMOTOR_VERSION, edaWRITE_RAM_ONLY_NO_ACTION, aucVerString);

    return eErrorCode;
}

eCLI_ERROR_CODE utilCommon_LsmStatus(sGEC_CLI_COMMON_FORMAT* psCmdFormat)
{
    eCLI_ERROR_CODE eErrorCode = eCLI_ERROR_CODE_NO;
    INT32 alData[8] = {0};

    sscanf(psCmdFormat->acTextString, "%d,%d,%d,%d", &alData[0], &alData[1], &alData[2], &alData[3]);
    LOG_MSG(db_HAL_MCU, "LsmStatus %d \r\n", alData[0]);

    if(alData[0] == 0) //0: idle(finished), other: moving(not finished)
    {
        palMotor_LSM_CalibrationStatus_Set(0);
        palMotor_LSM_CalibrationDone_Set(0);
    }
    else
    {
        palMotor_LSM_CalibrationStatus_Set(1);
        palMotor_LSM_CalibrationDone_Set(1);
    }

    return eErrorCode;
}

eCLI_ERROR_CODE utilCommon_LsmPosition(sGEC_CLI_COMMON_FORMAT* psCmdFormat)
{
    eCLI_ERROR_CODE eErrorCode = eCLI_ERROR_CODE_NO;
    INT32 alData[8] = {0};

    sscanf(psCmdFormat->acTextString, "%d,%d", &alData[0], &alData[1]);

    if(psCmdFormat->lData == 1) //Lens
    {
        if(psCmdFormat->fData == 1) //Lens H
        {
            sLENS_INFO sLensInfo = {0};

            //palDataMgr_Data_Access(edcCUR_H_POSITION, edaWRITE_THROUGH_WITH_ACTION, &alData[0]);
            //palDataMgr_Data_Access(edcCUR_H_DIR, edaWRITE_THROUGH_WITH_ACTION, &alData[1]);
            LOG_MSG(db_HAL_MCU, "LSM write H Position:%d, H Dir:%d\r\n", alData[0], alData[1]);
            if(utilCounterGet(eCOUNTER_TYPE_APP_LENS) == 0)
            {
                utilCounterSet(eCOUNTER_TYPE_APP_LENS, 2000);
            }
            palDataMgr_Data_Access(edcLENS_INFO, edaREAD, &sLensInfo);
            sLensInfo.wLensHPosition = alData[0];
            sLensInfo.cDirH = alData[1];
            palDataMgr_Data_Access(edcLENS_INFO, edaWRITE_RAM_ONLY_NO_ACTION, &sLensInfo);
        }
        else if(psCmdFormat->fData == 2) //Lens V
        {
            sLENS_INFO sLensInfo = {0};

        #if defined(PLATFORM_H30_4K) //for H30 Lens V direction
            //alData[1] = !alData[1];
        #endif

            //palDataMgr_Data_Access(edcCUR_V_POSITION, edaWRITE_THROUGH_WITH_ACTION, &alData[0]);
            //palDataMgr_Data_Access(edcCUR_V_DIR, edaWRITE_THROUGH_WITH_ACTION, &alData[1]);
            LOG_MSG(db_HAL_MCU, "LSM write V Position:%d, V Dir:%d\r\n", alData[0], alData[1]);

            if(utilCounterGet(eCOUNTER_TYPE_APP_LENS) == 0)
            {
                utilCounterSet(eCOUNTER_TYPE_APP_LENS, 2000);
            }
            palDataMgr_Data_Access(edcLENS_INFO, edaREAD, &sLensInfo);
            sLensInfo.wLensVPosition = alData[0];
            sLensInfo.cDirV = alData[1];
            palDataMgr_Data_Access(edcLENS_INFO, edaWRITE_RAM_ONLY_NO_ACTION, &sLensInfo);
        }
    }
    else if(psCmdFormat->lData == 2) //ZoomFocus
    {
        if(psCmdFormat->fData == 1) //Zoom
        {
            sLENS_INFO sLensInfo = {0};

            LOG_MSG(db_HAL_MCU, "LSM write wZoomPosition:%d, wZoomDir:%d \r\n", alData[0], alData[1]);
            palDataMgr_Data_Access(edcLENS_INFO, edaREAD, &sLensInfo);
            sLensInfo.wZoomPosition = alData[0];
            sLensInfo.cDirZoom = alData[1];
            palDataMgr_Data_Access(edcLENS_INFO, edaWRITE_RAM_ONLY_NO_ACTION, &sLensInfo);
        }
        else if(psCmdFormat->fData == 2) //Focus
        {
            sLENS_INFO sLensInfo = {0};

            LOG_MSG(db_HAL_MCU, "LSM write wFocusPosition:%d, wFocusDir:%d \r\n", alData[0], alData[1]);
            palDataMgr_Data_Access(edcLENS_INFO, edaREAD, &sLensInfo);
            sLensInfo.wFocusPosition = alData[0];
            sLensInfo.cDirFocus = alData[1];
            palDataMgr_Data_Access(edcLENS_INFO, edaWRITE_RAM_ONLY_NO_ACTION, &sLensInfo);
        }
    }

    return eErrorCode;
}

eCLI_ERROR_CODE utilCommon_LsmCenter(sGEC_CLI_COMMON_FORMAT* psCmdFormat)
{
    eCLI_ERROR_CODE eErrorCode = eCLI_ERROR_CODE_NO;
    INT32 alData[8] = {0};
    sLENS_INFO sLensInfo = {0};

    sscanf(psCmdFormat->acTextString, "%d,%d,%d,%d,%d,%d,%d,%d",
        &alData[0], &alData[1], &alData[2], &alData[3], &alData[4], &alData[5], &alData[6], &alData[7]);

    return eErrorCode;
}

eCLI_ERROR_CODE utilCommon_CalibrationDoneGet(sGEC_CLI_COMMON_FORMAT* psCmdFormat)
{
    eCLI_ERROR_CODE eErrorCode = eCLI_ERROR_CODE_NO;
    INT32 alData[8] = {0};

    sscanf(psCmdFormat->acTextString, "%d,%d,%d,%d", &alData[0], &alData[1], &alData[2], &alData[3]);
    LOG_MSG(db_HAL_MCU, "LSM Lens Calibration Done %d \r\n", alData[0]);

    if(alData[0] == 1) //1: done, other: not finished
    {
        palEnvironment_LensCalFlag_Set(1);
        palMotor_LSM_CalibrationDone_Set(1);
    }
    else
    {
        palEnvironment_LensCalFlag_Set(0);
        palMotor_LSM_CalibrationDone_Set(0);
    }

    return eErrorCode;
}

eCLI_ERROR_CODE utilCommon_LsmConfig(sGEC_CLI_COMMON_FORMAT* psCmdFormat) //HICC2_AC_0010
{
    eCLI_ERROR_CODE eErrorCode = eCLI_ERROR_CODE_NO;
    INT32 alData[8] = {0};

    sscanf(psCmdFormat->acTextString, "%d,%d,%d,%d,%d", &alData[0], &alData[1], &alData[2], &alData[3], &alData[4]);
    LOG_MSG(db_HAL_MCU, "LsmConfig Module:%d, LensID:%d, ADC_HardwareIdentifier: %d, EEPROM_VERSION:%d.%d\r\n",
        alData[0], alData[1], alData[2], alData[3], alData[4]); //HICC2_AC_0021

    halMotor_LSM_Lens_Module_Set(alData[0]);
    halMotor_LSM_Lens_ID_Set(alData[1]);
    //palDataMgr_Data_Access(edcLENS_TYPE, edaWRITE_RAM_ONLY_WITH_ACTION, &alData[1]);// HICC2_Bruce_0005 remove

    return eErrorCode;
}

eCLI_ERROR_CODE utilCommon_LensI2CStatus(sGEC_CLI_COMMON_FORMAT* psCmdFormat)
{
    eCLI_ERROR_CODE eErrorCode = eCLI_ERROR_CODE_NO;
    INT32 alData[8] = {0};
	UINT8 uDeviceName[15] = {0};

    sscanf(psCmdFormat->acTextString, "%d,%d,%d,%s", &alData[0], &alData[1], &alData[2], uDeviceName);

    LOG_MSG(db_HAL_MCU, "alDeviceName: %s, strlen = %d\r\n", uDeviceName, strlen(uDeviceName));
	if(strncmp(uDeviceName, "LENS_NVRAM", strlen("LENS_NVRAM")) == 0)
	{
    	LOG_MSG(db_HAL_MCU, "I2C Status : %s => %d %d %d\r\n", psCmdFormat->acTextString,alData[0], alData[1], alData[2]);
		if(alData[0] == 0 && alData[1] == 0 && alData[2] == 0)
			halMotor_LSM_NVRAM_Status_Set(TRUE);
	}


    return eErrorCode;
}

eCLI_ERROR_CODE utilCommon_LensFactoryMenu(sGEC_CLI_COMMON_FORMAT* psCmdFormat)
{
    eCLI_ERROR_CODE eErrorCode = eCLI_ERROR_CODE_NO;
    INT32 alData[21] = {0};
    sLENS_INFO sLensInfo = {0};

    sscanf(psCmdFormat->acTextString, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", //HICC2_AC_0046
        &alData[0], &alData[1], &alData[2], &alData[3], &alData[4],&alData[5], &alData[6], &alData[7], &alData[8], &alData[9],
        &alData[10], &alData[11], &alData[12], &alData[13], &alData[14],&alData[15], &alData[16], &alData[17], &alData[18], &alData[19], &alData[20]);

    if(psCmdFormat->lData == 1) //Lens Information
    {
#if defined(PLATFORM_H30_4K) //for H30 Lens V direction
        //alData[6] = !alData[6];
#endif
        LOG_MSG(db_HAL_MCU, "LensFactoryMenu CalShiftDone|CalLensDone:%d, CalBound:%d, OpticalOffsetEn %d \r\n", alData[0], alData[1], alData[2]);
        LOG_MSG(db_HAL_MCU, "LensFactoryMenu CurH:%d, DirH:%d, CurV:%d, DirV:%d, \r\n", alData[3], alData[4], alData[5], alData[6]);
        LOG_MSG(db_HAL_MCU, "LensFactoryMenu BacklashH:%d, BacklashV:%d, HomeDirH:%d, HomeDirV:%d, \r\n", alData[7], alData[8], alData[9], alData[10]);
        LOG_MSG(db_HAL_MCU, "LensFactoryMenu SensorStateH:%d, SensorStateV:%d, \r\n", alData[11], alData[12]);
        LOG_MSG(db_HAL_MCU, "LensFactoryMenu CenterH:%d, CenterV:%d, OffsetH:%d, OffsetV:%d, \r\n", alData[13], alData[14], alData[15], alData[16]);
        LOG_MSG(db_HAL_MCU, "LensFactoryMenu LeftBound:%d, RightBound:%d, DownBound:%d, UpBound:%d, \r\n", alData[17], alData[18], alData[19], alData[20]);

        palDataMgr_Data_Access(edcLENS_INFO, edaREAD, &sLensInfo); //HICC2_AC_0046
        sLensInfo.wLensHPosition = alData[3];
        sLensInfo.wLensVPosition = alData[5];
        sLensInfo.cDirH = (UINT8)alData[4];
        sLensInfo.cDirV = (UINT8)alData[6];
        sLensInfo.wBacklashH = alData[7];
        sLensInfo.wBacklashV = alData[8];
        sLensInfo.cHomeDirH = alData[9];
        sLensInfo.cHomeDirV = alData[10];
        sLensInfo.cSensorStateH = alData[11];
        sLensInfo.cSensorStateV = alData[12];
        sLensInfo.ulLensCenterH = alData[13];
        sLensInfo.ulLensCenterV = alData[14];
        sLensInfo.ulActiveMaxValueH = alData[17];
        sLensInfo.ulActiveMinValueH = alData[18];
        sLensInfo.ulActiveMaxValueV = alData[19];
        sLensInfo.ulActiveMinValueV = alData[20];

        halMotor_LSM_Center_Config_Set(alData[2]);
        palDataMgr_Data_Access(edcLENS_CENTER_SETTING, edaWRITE_RAM_ONLY_NO_ACTION, &alData[2]); //HICC2_AC_0089
        palDataMgr_Data_Access(edcCUR_H_POSITION, edaWRITE_THROUGH_WITH_ACTION, &alData[3]);
        palDataMgr_Data_Access(edcCUR_H_DIR, edaWRITE_THROUGH_WITH_ACTION, &alData[4]);
        palDataMgr_Data_Access(edcCUR_V_POSITION, edaWRITE_THROUGH_WITH_ACTION, &alData[5]);
        palDataMgr_Data_Access(edcCUR_V_DIR, edaWRITE_THROUGH_WITH_ACTION, &alData[6]);
        palDataMgr_Data_Access(edcHOME_H_DIR, edaWRITE_THROUGH_WITH_ACTION, &alData[9]);
        palDataMgr_Data_Access(edcHOME_V_DIR, edaWRITE_THROUGH_WITH_ACTION, &alData[10]);
        palDataMgr_Data_Access(edcLENS_INFO, edaWRITE_RAM_ONLY_NO_ACTION, &sLensInfo);
    }
    else if(psCmdFormat->lData == 2) //ZoomFocus Information
    {
        LOG_MSG(db_HAL_MCU, "LensFactoryMenu ZoomPosition:%d, ZoomDir:%d, FocusPosition:%d, FocusDir:%d \r\n",
            alData[0], alData[1], alData[2], alData[3]);

        LOG_MSG(db_HAL_MCU, "LensFactoryMenu LimitZoomOut:%d, LimitZoomIn:%d, LimitFocusNear:%d, LimitFocusFar:%d \r\n",
            alData[4], alData[5], alData[6], alData[7]);
        LOG_MSG(db_HAL_MCU, "LensFactoryMenu Lens_ID_ADC:%d, \r\n", alData[8]);

        palDataMgr_Data_Access(edcLENS_INFO, edaREAD, &sLensInfo);
        sLensInfo.wZoomPosition = (UINT16)alData[0];
        sLensInfo.cDirZoom = (UINT16)alData[1];
        sLensInfo.wFocusPosition = (UINT8)alData[2];
        sLensInfo.cDirFocus = (UINT8)alData[3];
        sLensInfo.wZoomMaximumValue = (UINT16)alData[4];
        sLensInfo.wZoomMinimumValue = (UINT16)alData[5];
        sLensInfo.wFocusMaximumValue = (UINT16)alData[6];
        sLensInfo.wFocusMinimumValue = (UINT16)alData[7];
        sLensInfo.ulLensIDAdcValue = (UINT32)alData[8];
        palDataMgr_Data_Access(edcLENS_INFO, edaWRITE_RAM_ONLY_NO_ACTION, &sLensInfo);
    }
    return eErrorCode;
}

eCLI_ERROR_CODE utilCommon_LsmMemory(sGEC_CLI_COMMON_FORMAT* psCmdFormat) //HICC2_AC_0019
{
    eCLI_ERROR_CODE eErrorCode = eCLI_ERROR_CODE_NO;
    INT32 alData[8] = {0};

    sscanf(psCmdFormat->acTextString, "%d", &alData[0]);
    LOG_MSG(db_HAL_MCU, "LsmMemory :%d \r\n", alData[0]);
    palMotor_LSM_Memory_Set(alData[0]);
    palEnvironment_LensMemorySavingCondition_Set(alData[0]);
    return eErrorCode;
}

eCLI_ERROR_CODE utilCommon_LensDetection(sGEC_CLI_COMMON_FORMAT* psCmdFormat) //HICC2_AC_0021
{
    eCLI_ERROR_CODE eErrorCode = eCLI_ERROR_CODE_NO;
    INT32 alData[8] = {0};
	BOOL bIsShutter;
	static BOOL bIsUnplung = FALSE;
    UINT8 ucSubCmd = 0;

    ucSubCmd = psCmdFormat->fData;
    sscanf(psCmdFormat->acTextString, "%d", &alData[0]);
    LOG_MSG(db_HAL_MCU, "LensDetection :%d, ucSubCmd[%d]\r\n", alData[0], ucSubCmd);

    if(ucSubCmd == 2) //H30K_Julie_0005
    {
        palMotor_LensLock_State_Set(alData[0]);
    }
    else
    {
    	UINT8 ucOSDLensDetection;

    	palDataMgr_Data_Access(edcLENS_DETECTION, edaREAD, &ucOSDLensDetection);
        palMotor_LSM_Lens_Plug_Set(!alData[0]);

#ifndef CURSOR_FIXTURE
        palDataMgr_Data_Access(edcPICTURE_MUTE, edaREAD, &bIsShutter);
        if(alData[0] == 0)
        {
            if(bIsShutter == 0)
            {
            	if(ucOSDLensDetection == 1)
				{
	                bIsShutter = 1;
	                palDataMgr_Data_Access(edcPICTURE_MUTE, edaWRITE_RAM_ONLY_WITH_ACTION, &bIsShutter);
				}
            }
        }
#endif

        switch(alData[0])
        {
            case eMLC_LSM_DETECTION_UNPLUGED:
                bIsUnplung = TRUE;
                palLedProc_LED_Behavior_Set(eLED_STATUS_LENS_UNPLUGGED);
                break;

            case eMLC_LSM_DETECTION_MONITOR:
                if(bIsUnplung)
                {
                    bIsUnplung = FALSE;
                    palLedProc_LED_Behavior_Set(eLED_STATUS_SHUTTER_ON);
                }
                break;

            default:
                palLedProc_LED_Behavior_Set(eLED_STATUS_LENS_INSTALLING);
                break;
        }
        palMotor_LensDetection_State_Set(alData[0]);

        #ifdef CURSOR_FIXTURE
        if(alData[0] == eMLC_LSM_DETECTION_MONITOR)
        {
            UINT8 ucVal;
            palDataMgr_Data_Access(edcLENS_DURATIONTIME_ENABLE, edaREAD, &ucVal);
            if(ucVal == ets_ON)
            {
                halMotor_LSM_Zoom_Duration_Set();
                halMotor_LSM_Focus_Duration_Set();
                halMotor_LSM_BackFocus_Duration_Set();
            }
        }
        #endif
    }

    return eErrorCode;
}

sGEC_CLI_DATA_INFO m_sCliInfo_Gec[eMSG_TYPE_NUMBERS];

const static sCLI_GEC_LUT m_asGecCmdLut[] =
{
      //ulCmdID                   pcCmdString                                               uiSpecialFlag
      //Coretronic==========================================================================================
      //{Example,                 "12345678901234567890", utilCommon_SYS,                   CLI_COM_NORMAL},
/*00*/{eCLI_GEC_HELP,             "Help",                 NULL,                             CLI_COM_NORMAL},
/*01*/{eCLI_GEC_TESTER,           "TesterMode",           NULL,                             CLI_COM_NORMAL},
/*02*/{eCLI_GEC_POWER,            "power",                NULL,                             CLI_COM_NORMAL},
/*03*/{eCLI_GEC_RESET_ALL,        "FactoryResetAll",      NULL,                             CLI_COM_NORMAL},
/*04*/{eCLI_GEC_DBMK,             "DebugLog",             NULL,                             CLI_COM_NORMAL},
/*05*/{eCLI_GEC_ELOG,             "Elog",                 NULL,                             CLI_COM_NORMAL},
/*06*/{eCLI_GEC_IIC_STATUS,       "IICstatus",            utilCommon_LensI2CStatus,         CLI_COM_NORMAL},
/*07*/{eCLI_GEC_TIME,             "Time",                 NULL,                             CLI_COM_NORMAL},
/*08*/{eCLI_GEC_DEV_TEST,         "DevTest",              NULL,                             CLI_COM_NORMAL},
/*09*/{eCLI_GEC_OPFU,             "OPFU",                 NULL,                             CLI_COM_NORMAL},
/*10*/{eCLI_GEC_LPF_AC,           "LPFAC",                NULL,                             CLI_COM_NORMAL},
/*11*/{eCLI_GEC_TEST_PWM,         "TestPwm",              NULL,                             CLI_COM_NORMAL},
/*12*/{eCLI_GEC_TEST_ADC,         "TestAdc",              NULL,                             CLI_COM_NORMAL},
/*13*/{eCLI_GEC_MODEL_NAME_R,     "ModelNameGet",         NULL,                             CLI_COM_NORMAL},
/*14*/{eCLI_GEC_VERSION_R,        "VersionGet",           utilCommon_VersionGet,            CLI_COM_NORMAL},
/*15*/{eCLI_GEC_SERIAL_NUMER_R,   "SerialNumberGet",      NULL,                             CLI_COM_NORMAL},
/*16*/{eCLI_GEC_PID,              "PID",                  NULL,                             CLI_COM_NORMAL},
/*17*/{eCLI_GEC_SINE,             "Sine",                 NULL,                             CLI_COM_NORMAL},
/*18*/{eCLI_LSM_PROFILE_TEST,     "ProfileTest",          NULL,                             CLI_COM_NORMAL},
/*19*/{eCLI_LSM_CFG,              "LsmConfig",            utilCommon_LsmConfig,             CLI_COM_NORMAL},
/*20*/{eCLI_LSM_STATUS,           "LsmStatus",            utilCommon_LsmStatus,             CLI_COM_NORMAL},
/*21*/{eCLI_LSM_CALIBATION,       "LsmCalibration",       utilCommon_CalibrationDoneGet,    CLI_COM_NORMAL},
/*22*/{eCLI_LSM_CALIBATION_SET,   "LsmCalibrationSet",    NULL,                             CLI_COM_NORMAL},
/*23*/{eCLI_LSM_CENTER,           "LsmCenter",            utilCommon_LsmCenter,             CLI_COM_NORMAL},
/*24*/{eCLI_LSM_POSITION,         "LsmPosition",          utilCommon_LsmPosition,           CLI_COM_NORMAL},
/*25*/{eCLI_LSM_MEMORY,           "LsmMemory",            utilCommon_LsmMemory,             CLI_COM_NORMAL},
/*26*/{eCLI_LSM_H,                "LsmH",                 NULL,                             CLI_COM_NORMAL},
/*27*/{eCLI_LSM_V,                "LsmV",                 NULL,                             CLI_COM_NORMAL},
/*28*/{eCLI_LSM_ZOOM,             "Zoom",                 NULL,                             CLI_COM_NORMAL},
/*29*/{eCLI_LSM_FOCUS,            "Focus",                NULL,                             CLI_COM_NORMAL},
/*30*/{eCLI_LSM_FOCUS2,           "Focus2",               NULL,                             CLI_COM_NORMAL},
/*31*/{eCLI_LSM_FACTORY_MENU,     "LensFactoryMenu",      utilCommon_LensFactoryMenu,       CLI_COM_NORMAL},
/*32*/{eCLI_LSM_LENS_DETECTION,   "LensDetection",        utilCommon_LensDetection,         CLI_COM_NORMAL}

};

#define GEC_CMD_LUT_NUMBER      sizeof(m_asGecCmdLut)/sizeof(sCLI_GEC_LUT)

void utilMssc_CmdDecode_Gec(sCLI_FORMAT_GEC* psCmdFormat, UINT8* pcString)
{
    INT32 lErrorCode;
    INT32 lTemp;
    INT32 lDecdoeStep;
    INT32 lStringLen;
    INT32 lCmdIndex;
    INT32 lIndex;

    lDecdoeStep = eCLI_DECODE_STEP_GEC_CMD;
    lStringLen  = strlen((char*)pcString);
    lCmdIndex   = 0;                           //did not check HEADER again
    lIndex      = 0;

    LOG_MSG(db_HAL_MCU," receive LSM cmd %s\r\n", pcString);

    do
    {
        lCmdIndex++;
        if((pcString[lCmdIndex] == ',') &&
           (lDecdoeStep < eCLI_DECODE_STEP_GEC_DATA))
        {
            lDecdoeStep++;
            lIndex = 0;
            continue;
        }
        else if(pcString[lCmdIndex] == '\0')
        {
            if(lDecdoeStep == eCLI_DECODE_STEP_GEC_DATA)
            {
                lDecdoeStep = eCLI_DECODE_STEP_GEC_END;
                break;
            }
            else
            {//format is error
                psCmdFormat->lErrorCode = eCLI_ERROR_CODE_FORMATERR;
                return;
            }
        }

        if(lDecdoeStep == eCLI_DECODE_STEP_GEC_CMD)
        {
            psCmdFormat->acCommand[lIndex] = pcString[lCmdIndex];
        }
        else if(lDecdoeStep == eCLI_DECODE_STEP_GEC_SUB1)
        {
            psCmdFormat->acSub1[lIndex] = pcString[lCmdIndex];
        }
        else if(lDecdoeStep == eCLI_DECODE_STEP_GEC_SUB2)
        {
            psCmdFormat->acSub2[lIndex] = pcString[lCmdIndex];
        }
        else if(lDecdoeStep == eCLI_DECODE_STEP_GEC_DATA)
        {
            psCmdFormat->acTextString[lIndex] = pcString[lCmdIndex];
        }
        else
        {//format is error
            psCmdFormat->lErrorCode = eCLI_ERROR_CODE_FORMATERR;
            return;
        }
        lIndex++;

    }while(lCmdIndex < lStringLen);

    //LOG_MSG(db_HAL_LSM, "Cmd: %s\r\n",  psCmdFormat->acCommand);
    //LOG_MSG(db_HAL_LSM, "Sub1: %s\r\n", psCmdFormat->acSub1);
    //LOG_MSG(db_HAL_LSM, "Sub2: %s\r\n", psCmdFormat->acSub2);
    //LOG_MSG(db_HAL_LSM, "Data: %s\r\n", psCmdFormat->acTextString);

    psCmdFormat->lAccessMode = ecmWrite;
    lErrorCode = eCLI_ERROR_CODE_DATAERROR;
    for(lCmdIndex = 0; lCmdIndex < GEC_CMD_LUT_NUMBER; lCmdIndex++)
    {
        lStringLen = strlen(m_asGecCmdLut[lCmdIndex].pcCmdString);
        lTemp = strncasecmp(psCmdFormat->acCommand, m_asGecCmdLut[lCmdIndex].pcCmdString, lStringLen);
        if(lTemp == 0) //string are match
        {
            if(lStringLen == strlen(psCmdFormat->acCommand)) //double check string length
            {
                psCmdFormat->lFuncID = lCmdIndex;
                lErrorCode = eCLI_ERROR_CODE_NO;
                break;
            }
        }
    }

    psCmdFormat->lErrorCode = lErrorCode;
    if(lErrorCode != eCLI_ERROR_CODE_NO)
    {
        return;
    }

    if(((psCmdFormat->acSub1[0] == '0') && (psCmdFormat->acSub1[0] == 'x')) ||
       ((psCmdFormat->acSub1[0] == '0') && (psCmdFormat->acSub1[0] == 'X')))
    {
        sscanf(psCmdFormat->acSub1, "%X", &psCmdFormat->lSubCode1);
    }
    else if(psCmdFormat->acSub1[0] == '\0')
    {
        psCmdFormat->lSubCode1 = -1;
        psCmdFormat->lAccessMode = ecmRead;
    }
    else
    {
        sscanf(psCmdFormat->acSub1, "%d", &psCmdFormat->lSubCode1);
    }

    if(((psCmdFormat->acSub2[0] == '0') && (psCmdFormat->acSub2[0] == 'x')) ||
       ((psCmdFormat->acSub2[0] == '0') && (psCmdFormat->acSub2[0] == 'X')))
    {
        sscanf(psCmdFormat->acSub2, "%X", &psCmdFormat->lSubCode2);
    }
    else if(psCmdFormat->acSub2[0] == '\0')
    {
        psCmdFormat->lSubCode2 = -1;
        psCmdFormat->lAccessMode = ecmRead;
    }
    else
    {
        sscanf(psCmdFormat->acSub2, "%d", &psCmdFormat->lSubCode2);
    }

    if(psCmdFormat->acTextString[0] == ',')
    {// data is null
        psCmdFormat->lAccessMode = ecmRead;
    }

    LOG_MSG(db_HAL_MCU,
            "CMD=%d, R/W=%d, SubCode1=%d, SubCode2=%d, Error=%d Data=%s\r\n",
            psCmdFormat->lFuncID,
            psCmdFormat->lAccessMode,
            psCmdFormat->lSubCode1,
            psCmdFormat->lSubCode2,
            psCmdFormat->lErrorCode,
            psCmdFormat->acTextString);
}

static void utilMssc_CmdHandle_Gec(sCLI_FORMAT_GEC* psCmdFormat)
{
    UINT32 ulErrorCode = eCLI_ERROR_CODE_NO;
    sGEC_CLI_COMMON_FORMAT sCommanFormat;

    memset((BYTE*)&sCommanFormat, 0, sizeof(sGEC_CLI_COMMON_FORMAT));
    if(psCmdFormat->lFuncID >= GEC_CMD_LUT_NUMBER)
    {
        psCmdFormat->lErrorCode = eCLI_ERROR_CODE_REQUESTFAIL;
        return;
    }

    psCmdFormat->ulFlag = m_asGecCmdLut[psCmdFormat->lFuncID].ulSpecialFlag;
    if(m_asGecCmdLut[psCmdFormat->lFuncID].iCLI_Replay == NULL)
    {
        psCmdFormat->lErrorCode = eCLI_ERROR_CODE_REQUESTFAIL;
        return;
    }

    if(psCmdFormat->lAccessMode == ecmWrite) //Write command
    {
        if((psCmdFormat->ulFlag & CLI_COM_READ_ONLY))
        {
            psCmdFormat->lErrorCode = eCLI_ERROR_CODE_REQUESTFAIL;
            return;
        }

        sCommanFormat.eAccessMode = ecmWrite;
        sCommanFormat.ucCmdFrom = psCmdFormat->lCmdFrom;
        //used smaller one between CLI_DATA_SIZE and GEC_DATA_SIZE
        snprintf(sCommanFormat.acTextString, GEC_DATA_SIZE, "%s", psCmdFormat->acTextString);
        sCommanFormat.lData = psCmdFormat->lSubCode1;
        sCommanFormat.fData = psCmdFormat->lSubCode2;
        ulErrorCode = m_asGecCmdLut[psCmdFormat->lFuncID].iCLI_Replay(&sCommanFormat);

        //used smaller one between CLI_DATA_SIZE and GEC_DATA_SIZE
        snprintf(psCmdFormat->acTextString, GEC_DATA_SIZE, "%s", sCommanFormat.acTextString);
        psCmdFormat->lAccessMode = sCommanFormat.eAccessMode;
        psCmdFormat->lErrorCode = ulErrorCode;
    }
    else //Read
    {
        if((psCmdFormat->ulFlag & CLI_COM_WRITE_ONLY))
        {
            psCmdFormat->lErrorCode = eCLI_ERROR_CODE_REQUESTFAIL;
            return;
        }

        sCommanFormat.eAccessMode = ecmRead;
        sCommanFormat.ucCmdFrom = psCmdFormat->lCmdFrom;
        sCommanFormat.lData = psCmdFormat->lSubCode1;
        sCommanFormat.fData = psCmdFormat->lSubCode2;

        ulErrorCode = m_asGecCmdLut[psCmdFormat->lFuncID].iCLI_Replay(&sCommanFormat);

        //used smaller one between CLI_DATA_SIZE and GEC_DATA_SIZE
        snprintf(psCmdFormat->acTextString, GEC_DATA_SIZE, "%s", sCommanFormat.acTextString);
        psCmdFormat->lSubCode1 = sCommanFormat.lData;
        psCmdFormat->lSubCode2 = sCommanFormat.fData;

        psCmdFormat->lAccessMode = sCommanFormat.eAccessMode;
        psCmdFormat->lErrorCode = ulErrorCode;
    }
}

void utilMssc_Process_Gec(sCLI_FORMAT_GEC* psCmdFormat, UINT8* pcString)
{
    psCmdFormat->lErrorCode = eCLI_ERROR_CODE_NO;

    utilMssc_CmdDecode_Gec(psCmdFormat, pcString);

    if(psCmdFormat->lErrorCode == eCLI_ERROR_CODE_NO)
    {
        utilMssc_CmdHandle_Gec(psCmdFormat);
    }
#if 0
    if(utilMssc_CmdReply_Gec(psCmdFormat) >= 0)
    {
        utilMssc_CmdRespond_Gec(psCmdFormat);
    }
#endif
}

eMSG_STATE utilHost_GEC_StateProcess(eMSG_TYPE eMsgType)
{
    UINT8 cReadBuffer = 0;

    while(rcSUCCESS == utilHost_InputBufferRead(eMsgType, &cReadBuffer))
    {
        if(cReadBuffer == GEC_CMD_HEADER_1)
        {
            m_sCliInfo_Gec[eMsgType].uiCLI_Front         = 1;
            m_sCliInfo_Gec[eMsgType].uiCLI_Poll_Position = 1;
            m_sCliInfo_Gec[eMsgType].ucCLI_DATA[0] = cReadBuffer;
        }
        else if(cReadBuffer ==  GEC_CMD_END_1)
        {
            sCLI_FORMAT_GEC sCmdFormat = {0};

            m_sCliInfo_Gec[eMsgType].uiCLI_End = 1;
            if((m_sCliInfo_Gec[eMsgType].uiCLI_Front == 0) ||
               (m_sCliInfo_Gec[eMsgType].uiCLI_Poll_Position < 4))
            {
                m_sCliInfo_Gec[eMsgType].uiCLI_Poll_Position = 0;
                return eMSG_STATE_BAD_PACKET;
            }

            m_sCliInfo_Gec[eMsgType].ucCLI_DATA[m_sCliInfo_Gec[eMsgType].uiCLI_Poll_Position] = ','; //for API strtok
            m_sCliInfo_Gec[eMsgType].ucCLI_DATA[m_sCliInfo_Gec[eMsgType].uiCLI_Poll_Position+1] = '\0';
            sCmdFormat.lCmdFrom = eMsgType;
            sCmdFormat.lErrorCode = eCLI_ERROR_CODE_NO;

            utilMssc_Process_Gec(&sCmdFormat, m_sCliInfo_Gec[eMsgType].ucCLI_DATA);

            m_sCliInfo_Gec[eMsgType].uiCLI_Poll_Position = 0;
            m_sCliInfo_Gec[eMsgType].uiCLI_Front         = 0;
            m_sCliInfo_Gec[eMsgType].uiCLI_End           = 0;
        }
        else
        {
            if(m_sCliInfo_Gec[eMsgType].uiCLI_Front == 0)
            {
                //No Header found
                return eMSG_STATE_BAD_PACKET;
            }

            if(m_sCliInfo_Gec[eMsgType].uiCLI_Poll_Position >= (CLI_DATA_SIZE-2))
            {
                m_sCliInfo_Gec[eMsgType].uiCLI_Poll_Position = 0;
                return eMSG_STATE_BAD_PACKET;
            }

            m_sCliInfo_Gec[eMsgType].ucCLI_DATA[m_sCliInfo_Gec[eMsgType].uiCLI_Poll_Position] = cReadBuffer;
            m_sCliInfo_Gec[eMsgType].uiCLI_Poll_Position++;
        }
    }
}

#endif

