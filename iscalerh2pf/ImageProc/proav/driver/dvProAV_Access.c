#include "dvProAV_Platform.h"
#include "dvProAV_Access.h"
#include "dvProAV_RegTable.h"
#ifndef QT_CPP
    #include "dvProAV_Interface.h"
    #include "utilDbgMsg.h"
#endif

#include "ProcessMutexData.h"   //H2PF_Simon_0033

#define BLOCK_WRITE_SIZE_MAX 0x4000     // 16384
#define BLOCK_READ_SIZE_MAX (0x4000 - 4)

static pfuncDeviceWrite _pfuncWrite;
static pfuncDeviceRead  _pfuncRead;
static pfuncDeviceBufferWrite _pfuncBufferWrite;

static int m_lastBankId = -1;
static uint08 m_debugMask = 0x00;

static volatile bool m_burstModeEnabled = false;
static volatile bool m_BurstAddInc = false;

static uint32 m_dataWriteMaxCount;
static uint32 m_dataReadMaxCount;

static bool m_bAccessInit = false;
static bool m_bWriteCheck = false;
static int m_nInterface = 1; // 0:PcSpi, 1:McuSpi, 2:PCIE

#ifdef QT_CPP
    static FILE *accessLog;
#endif

UINT32 _Reg_CurrentBankGet(void)
{
    sPROCESS_MUTEX_DATA *sProcessMutexData = (sPROCESS_MUTEX_DATA *)SharedMem_GetMapPtr(eSB_PROCESS_MUTEX_DATA);

    return sProcessMutexData->Scaler_CurrentBank;
}

void _Reg_CurrentBankSet(UINT32 CurrentBank)
{
    sPROCESS_MUTEX_DATA *sProcessMutexData = (sPROCESS_MUTEX_DATA *)SharedMem_GetMapPtr(eSB_PROCESS_MUTEX_DATA);

    sProcessMutexData->Scaler_CurrentBank = (UINT32)CurrentBank;
}


static inline int _ReadWriteStatus(const uint08 *data, uint16 length)
{
    int status = rcSUCCESS;
    uint08 checksum;
    if (m_bWriteCheck && (m_nInterface < 2))
    {
        uint08 ucRespChecksum;
        uint32 chksumRegId[2] = {GET_REG_OFFS(eComPcWdCksum), GET_REG_OFFS(eComMcuWdCksum)};
        checksum = 0;
        for (uint16 i=0; i < length; i++)
        {
            checksum += data[i];
        }
#ifdef QT_CPP
        PROAV_CS_DELAY();
#endif
        status =  (*_pfuncRead)(chksumRegId[m_nInterface], 1, &ucRespChecksum);

        if (ucRespChecksum!=checksum) //  writing data checksum compare, pull high if the checksum match
        {
            DBMSG_ALWAYS("Error: spi#%d write checksum = %02X: responsed checksum =%02X\r\n" , m_nInterface, checksum, ucRespChecksum);
            status = rcERROR;
        }
    }
    return status;
}

static int _ProAVScalerWrite(uint08 regId, uint16 length, const uint08 *data)
{
    int status;
    uint32 regAddr;

    if (!m_bAccessInit)
    {
        DBMSG("Driver has not initializied\r\n");
        return rcERROR;
    }

    regAddr = regId & 0x7F; // bit 7 always is 0 for 7 bit address
    status =  rcSUCCESS;

    if (!m_burstModeEnabled)
    {
#ifdef QT_CPP
        uint16 offset = 0;
        while(offset < length)
        {
            if (m_debugMask & 0x01)
            {
                if (m_lastBankId == - 1)
                    DBMSG_ACCESS("W,-- %02X %02X", regAddr + offset , data[offset]);
                else
                    DBMSG_ACCESS("W,%02X %02X %02X", (uint08)m_lastBankId , regAddr + offset , data[offset]);
            }
            //PROAV_CS_DELAY();
            status &= (*_pfuncWrite)(regAddr + offset, 1, &data[offset]);
            status &= _ReadWriteStatus(&data[offset], 1);

            if (m_debugMask & 0x01)
            {
                if (status!=rcSUCCESS)
                {
                    DBMSG_ACCESS(" //write:%d \n", status);
                }
                DBMSG_ACCESS(" \n");
            }
            offset++;
        }
#else
        status &= (*_pfuncWrite)(regAddr, length, &data[0]);

#endif /* 0 */
    }
    else
    {
        uint16 dataLen;
#ifdef QT_CPP
        if (m_debugMask & 0x01)
        {
            if (m_lastBankId == - 1)
                DBMSG_ACCESS("W,%02X", regId);
            else
                DBMSG_ACCESS("W,%02X %02X", (uint08)m_lastBankId, regId );
            for (uint16 i = 0; i< length; i++ )
                DBMSG_ACCESS(" %02X", data[i]);
        }
#endif
        for (uint32 offset = 0; offset < length; offset += m_dataWriteMaxCount)
        {
            if ((length - offset) >= m_dataWriteMaxCount)
                dataLen = (uint16)m_dataWriteMaxCount;
            else
                dataLen = (uint16)(length - offset);
#ifdef QT_CPP
            PROAV_CS_DELAY();
#endif
            status &= _pfuncWrite(regId, dataLen, &data[offset]);
#ifdef QT_CPP
            status &= _ReadWriteStatus(&data[offset], dataLen);
#endif
        }
#ifdef QT_CPP
        if (m_debugMask & 0x01)
        {
            if (status!=rcSUCCESS)
            {
                DBMSG_ACCESS(" //write:%d \n", status);
            }
            DBMSG_ACCESS(" \n");
        }
#endif
    }

    return status;
}

static int _ProAVScalerRead(uint08 regId, uint16 length, uint08 *data)
{
    int status;
    uint32 regAddr;

    if (!m_bAccessInit)
    {
        DBMSG("Driver has not initializied\r\n");
        return rcERROR;
    }

    regAddr = regId & 0x7F; // bit 7 always is 0 for 7 bit address
    status = rcSUCCESS;

    if (!m_burstModeEnabled)
    {
#ifdef QT_CPP
        uint16 offset = 0;
        while(offset < length)
        {
            PROAV_CS_DELAY();
            status &= (*_pfuncRead)(regAddr + offset, 1, &data[offset]);
            if (m_debugMask & 0x02)
            {
                if (m_lastBankId == - 1)
                    DBMSG_ACCESS("R,-- %02X %02X", regAddr + offset, data[offset]);
                else
                    DBMSG_ACCESS("R,%02X %02X %02X", (uint08)m_lastBankId, regAddr + offset, data[offset]);
                if (status!=rcSUCCESS)
                {
                    DBMSG_ACCESS(" //read:%d", status);
                }
                DBMSG_ACCESS(" \n");
            }
            offset++;
        }
#else
        status &= (*_pfuncRead)(regAddr, length, data);

        if(status == 0)
        {
            LOG_MSG(db_ALWAYS, "(%s, %d)m_lastBankId %d\n", __FUNCTION__, __LINE__, m_lastBankId);
        }
#endif /* QT_CPP */
    }
    else
    {
        uint16 dataLen;

        for (uint32 offset = 0; offset < length; offset += m_dataReadMaxCount)
        {
            if ((length - offset) >= m_dataReadMaxCount)
                dataLen = (uint16)m_dataReadMaxCount;
            else
                dataLen = (uint16)(length - offset);
#ifdef QT_CPP
            PROAV_CS_DELAY();
#endif
            status &= _pfuncRead(regId, dataLen, &data[offset]);
        }
#ifdef QT_CPP
        if (m_debugMask & 0x02)
        {
            if (m_lastBankId == - 1)
                DBMSG_ACCESS("R,-- %02X", regId );
            else
                DBMSG_ACCESS("R,%02X %02X", (uint08)m_lastBankId, regId);

            for (uint16 i = 0; i< length; i++ )
                DBMSG_ACCESS(" %02X", data[i]);

            if (status!=rcSUCCESS)
            {
                DBMSG_ACCESS(" //read:%d", status);
            }
            DBMSG_ACCESS(" \n");
        }
#else
        if(status == 0)
        {
            LOG_MSG(db_ALWAYS, "(%s, %d)m_lastBankId %d\n", __FUNCTION__, __LINE__, m_lastBankId);
        }

#endif
    }

    return status;
}

static int _ProAVScalerSelectBank(const uint08 bankId)
{
    if ((int)bankId != (int)_Reg_CurrentBankGet())
    {
        int status;

        status = _ProAVScalerWrite(0x00, 1, &bankId);
        if (status == rcSUCCESS)
        {
            m_lastBankId = bankId;
            _Reg_CurrentBankSet((UINT32)m_lastBankId);
        }
        return status;
    }

    return rcSUCCESS;
}

static int _ProAVScalerWriteWithBank(const uint08 bankId, const uint08 regId, const uint16 writeLen,const uint08 *writeBuffer)
{
    int status;
    status = _ProAVScalerSelectBank(bankId);

    if (status != rcSUCCESS)
        return status;

    return _ProAVScalerWrite(regId, writeLen, writeBuffer);
}

static int _ProAVScalerReadWithBank(const uint08 bankId, const uint08 regId, const uint16 readLen, uint08 *readBuffer)
{
    int status;
    status = _ProAVScalerSelectBank(bankId);

    if (status != rcSUCCESS)
        return status;

    return _ProAVScalerRead(regId, readLen, readBuffer);
}

/**
 * @brief 經由區段值與位移值寫入一個位元組的原始ProAV暫存器值
 * @param bank 區段值
 * @param offset 位移值
 * @param data 暫存器的值
 * @return 成功(rcSUCCESS)或失敗(rcERROR)
 */
int dvProAV_AccessRawDataWriteByte(const uint08 bank, const uint08 offset, uint08 data)
{
    return _ProAVScalerWriteWithBank(bank, offset, 1, &data);
}

/**
 * @brief 經由區段值與位移值讀取一個位元組的原始ProAV暫存器值
 * @param bank 區段值
 * @param offset 位移值
 * @param data 暫存器的值
 * @return 成功(rcSUCCESS)或失敗(rcERROR)
 */
int dvProAV_AccessRawDataReadByte(const uint08 bank, const uint08 offset, uint08 *data)
{
    return _ProAVScalerReadWithBank(bank, offset, 1, data);
}

/**
 * @brief 經由區段值與位移值寫入指定長度的原始ProAV暫存器值
 * @param bank 區段值
 * @param offset 位移值
 * @param data 暫存器的值
 * @return 成功(rcSUCCESS)或失敗(rcERROR)
 */
int dvProAV_AccessRawDataWrite(const uint08 bank, const uint08 offset, const uint16 len, uint08 *data)
{
    return _ProAVScalerWriteWithBank(bank, offset, len, data);
}

/**
 * @brief 經由區段值與位移值讀取指定長度的原始ProAV暫存器值
 * @param bank 區段值
 * @param offset 位移值
 * @param data 暫存器的值
 * @return 成功(rcSUCCESS)或失敗(rcERROR)
 */
int dvProAV_AccessRawDataRead(const uint08 bank, const uint08 offset, const uint16 len, uint08 *data)
{
    return _ProAVScalerReadWithBank(bank, offset, len, data);
}

static uint08 _RegShiftBitsGet(uint08 mask)
{
    uint08 i;
    for (i = 0; i < 8; i ++)
    {
        if (mask & 1)
            break;

        mask = mask >> 1;
    }
    return i;
}

static int _RegDescGet(const uint32 regEnum, RegDesc *desc)
{
    desc->bank   = GET_REG_BANK(regEnum);
    desc->offset = GET_REG_OFFS(regEnum);
    desc->mask   = GET_REG_MASK(regEnum);
    desc->rw     = GET_REG_RDWR(regEnum);
    desc->count  = GET_REG_SIZE(regEnum);
    if (desc->count == 0) desc->count = 64;

    return rcSUCCESS;
}

static int _RegArrayNoMaskWrite(const uint32 regEnum, const uint08 offset, const uint08 *data, uint16 count)
{
    RegDesc desc;

    if (_RegDescGet(regEnum, &desc) == rcERROR || desc.rw == RO)
        return rcERROR;

    if (_ProAVScalerWriteWithBank(desc.bank, desc.offset + offset, count, data) == rcERROR)
        return rcERROR;

    return rcSUCCESS;
}

static int _RegArrayNoMaskRead(const uint32 regEnum, const uint08 offset, uint08 *data, uint16 count)
{
    RegDesc desc;

    if (_RegDescGet(regEnum, &desc) == rcERROR || desc.rw == WO)
        return rcERROR;

    if (_ProAVScalerReadWithBank(desc.bank, desc.offset + offset, count, data) == rcERROR)
        return rcERROR;

    return rcSUCCESS;
}

static int _RegArrayWrite(const uint32 regEnum, uint08 *data)
{
    uint08 lastByte = 0;
    uint08 lastIndex;
    RegDesc desc;

    _RegDescGet(regEnum,&desc);
    lastIndex =  desc.count - 1;

    if (desc.rw == RO)
    {
        ASSERT_ALWAYS();
        return rcERROR;
    }

    if((desc.rw != WO) && (desc.mask != 0xFF)) // Read Last Byte
    {
        if (_ProAVScalerReadWithBank(desc.bank, desc.offset + lastIndex , 1, &lastByte) == rcERROR)
        {
            ASSERT_ALWAYS();
            return rcERROR;
        }
    }

    data[lastIndex] <<= _RegShiftBitsGet(desc.mask);
    data[lastIndex] = (data[lastIndex] & desc.mask) | (lastByte & ~desc.mask);

    if (_ProAVScalerWriteWithBank(desc.bank, desc.offset , desc.count, data) == rcERROR)
    {
        ASSERT_ALWAYS();
        return rcERROR;
    }
    return rcSUCCESS;
}

static int _RegArrayRead(const uint32 regEnum, uint08 *data)
{
    uint08 lastByte;
    uint08 lastIndex;
    RegDesc desc;

    _RegDescGet(regEnum,&desc);
    lastIndex =  desc.count - 1;

    if (desc.rw == WO)
        return rcERROR;

    uint08 count =desc.count;

    if (_ProAVScalerReadWithBank(desc.bank, desc.offset , count, data) == rcERROR)
        return rcERROR;

    // Apply mask to the last byte
    lastByte = data[lastIndex] & desc.mask;
    data[lastIndex] = (uint08)(lastByte >> _RegShiftBitsGet(desc.mask));

    return rcSUCCESS;
}

/**
 * @brief 經由列舉值寫入32位元內所需長度的原始ProAV暫存器值
 * @param bank 區段值
 * @param offset 位移值
 * @param data 暫存器的值
 * @return 成功(rcSUCCESS)或失敗(rcERROR)
 */
int dvProAV_AccessWrite(const uint32 regEnum, const uint32 data)
{
    int i;
    uint08 ucData[4] = {0};
    RegDesc desc;

    _RegDescGet(regEnum,&desc);

    if (desc.count == 0 || desc.count > 4)
        return rcERROR;

    if(m_BurstAddInc == BURST_FIX_ADDR)
    {
        dvProAV_AccessBurstAddrIncEnable(BURST_INC_ADDR);
    }

    for (i = 0; i < desc.count; i++)
    {
        ucData[i] = (uint08)((data >> (i * 8)) & 0xff);
    }

    utilProcMutexData_ScalerRegDataRecord(regEnum, data);  //H2PF_Simon_0033

    return _RegArrayWrite(regEnum, ucData);
}

/**
 * @brief 經由列舉值讀取32位元內所需長度的原始ProAV暫存器值
 * @param bank 區段值
 * @param offset 位移值
 * @param data 暫存器的值
 * @return 成功(rcSUCCESS)或失敗(rcERROR)
 */
int dvProAV_AccessRead(const uint32 regEnum, uint32 *data)
{
    int i;
    int status;
    uint08 ucData[4] = {0, 0, 0, 0};

    RegDesc desc;

    _RegDescGet(regEnum,&desc);

    if(m_BurstAddInc == BURST_FIX_ADDR)
    {
        dvProAV_AccessBurstAddrIncEnable(BURST_INC_ADDR);
    }


    if (desc.count == 0 || desc.count > 4)
        return rcERROR;

     status = _RegArrayRead(regEnum, ucData);

     *data = 0;
     for (i = (desc.count - 1); i >= 0; i--)
     {
         if (i == (desc.count - 1))
            *data = ucData[i] & 0xff;
         else
            *data = (*data << 8) | (ucData[i] & 0xff);
     }

     return status;
}

/**
 * @brief 設定突發(Burst)模式時寫入暫存器位址為固定或自動增加
 * @param enable
 *        - true: 自動增加暫存器位址
 *        - false: 固定暫存器位址
 * @return 成功(rcSUCCESS)或失敗(rcERROR)
 */
int dvProAV_AccessBurstAddrIncEnable(const bool enable)
{
    uint8  data[4] = {0};
    int status;

    data[0] = enable ? 0 : 1;

    m_BurstAddInc = enable;
    status = _RegArrayWrite(eBiuSpiBrstWrAdrFix, data);
    return status;
}

/**
 * @brief 取得目前突發(Burst)模式時寫入暫存器位址為固定或自動增加的設定值
 * @param enable
 *        - true: 自動增加暫存器位址
 *        - false: 固定暫存器位址
 * @return 成功(rcSUCCESS)或失敗(rcERROR)
 */
int dvProAV_AccessBurstAddrIncEnabledGet(bool *enable)
{
    int status;
    uint32 fixedAddr;

    status = dvProAV_AccessRead(eBiuSpiBrstWrAdrFix, &fixedAddr);

    *enable = (fixedAddr != 0) ? false : true;
    return status;
}

/**
 * @brief 以突發(Burst)模式寫入指定的長度資料與模式
 * @param regEnum 暫存器列舉值
 * @param data 資料緩衝區指標
 * @param count 寫入的資料長度
 * @param autoIncAddrEnabled 暫存器位址為固定或自動增加的設定值
 * @return 成功(rcSUCCESS)或失敗(rcERROR)
 */
int dvProAV_AccessBurstWrite(const uint32 regEnum, uint08 *data, const uint16 count, const bool incAddrEnabled)
{
    int status = rcSUCCESS;

    if(m_BurstAddInc != incAddrEnabled)
    {
        status &= dvProAV_AccessBurstAddrIncEnable(incAddrEnabled);
    }
    dvProAV_AccessBurstModeEnable(true);
    status &= _RegArrayNoMaskWrite(regEnum, 0, data, count);
    dvProAV_AccessBurstModeEnable(false);

    return status;
}

/**
 * @brief 以突發(Burst)模式讀取指定的長度資料與模式
 * @param regEnum 暫存器列舉值
 * @param data 資料緩衝區指標
 * @param count 讀取的資料長度
 * @param autoIncAddrEnabled 暫存器位址為固定或自動增加的設定值
 * @return 成功(rcSUCCESS)或失敗(rcERROR)
 */
int dvProAV_AccessBurstRead(const uint32 regEnum, uint08 *data, const uint16 count, const bool incAddrEnabled)
{
    int status = rcSUCCESS;

    if(m_BurstAddInc != incAddrEnabled)
    {
        status &= dvProAV_AccessBurstAddrIncEnable(incAddrEnabled);
    }
    dvProAV_AccessBurstModeEnable(true);
    status &= _RegArrayNoMaskRead(regEnum, 0, data, count);
    dvProAV_AccessBurstModeEnable(false);

    return status;
}

/**
 * @brief 設定ProAV存取的函式與初始值
 * @param interfaceType:
 *        0:PcSpi
 *        1:McuSpi
 *        2:McuPCIe(i.mx)
 * @param pfuncRead
 * @param pfuncWrite
 * @return 成功(rcSUCCESS)或失敗(rcERROR)
 */
#ifndef QT_CPP
int dvProAV_AccessInit(int nInterfaceType ,pfuncDeviceRead pfuncRead, pfuncDeviceWrite pfuncWrite, pfuncDeviceBufferWrite pfuncBufferWrite)  // ProAV_Rex_0064
#else
int dvProAV_AccessInit(int nInterfaceType, pfuncDeviceRead pfuncRead, pfuncDeviceWrite pfuncWrite)
#endif
{
    _pfuncWrite = pfuncWrite;
    _pfuncRead = pfuncRead;
#ifndef QT_CPP
    _pfuncBufferWrite = pfuncBufferWrite;
#else
    m_nInterface = nInterfaceType;
#endif
    m_lastBankId = -1;
    _Reg_CurrentBankSet(0xFFFFFFFF);
    m_debugMask = 0x00;
    m_burstModeEnabled = false;
    m_dataWriteMaxCount = BLOCK_WRITE_SIZE_MAX;
    m_dataReadMaxCount = BLOCK_READ_SIZE_MAX;
    m_BurstAddInc = false;

    m_bAccessInit = true;
    return rcSUCCESS;
}

int dvProAV_AccessReconfig(void)
{
    m_lastBankId = -1;
    _Reg_CurrentBankSet(0xFFFFFFFF);
    m_debugMask = 0x00;
    m_burstModeEnabled = false;
    m_dataWriteMaxCount = BLOCK_WRITE_SIZE_MAX;
    m_dataReadMaxCount = BLOCK_READ_SIZE_MAX;
    m_BurstAddInc = false;

    m_bAccessInit = true;
    return rcSUCCESS;
}

/**
 * @brief 讀取存取介面類型
 * @return interfaceType:
 *        0:PcSpi
 *        1:McuSpi
 *        3:McuPCIe(i.mx)
 */
int dvProAV_AccessInterfaceGet(void)
{
    return m_nInterface;
}

/**
 * @brief 設定ProAV除錯遮罩
 * @param mask 除錯遮罩
 * @return (無)
 */
void dvProAV_AccessDebugMaskSet(uint08 mask)
{
    m_debugMask = mask;
}

/**
 * @brief 設定使用ProAV突發(Burst)模式時為真實突發模式或使用單一模式模擬
 * @param enable
 *        - true : 真實突發模式
 *        - false　:　單一模式模擬
 * @return (無)
 */
void dvProAV_AccessBurstModeEnable(bool enable)
{
    m_burstModeEnabled = enable;
}

/**
 * @brief 設定使Write Check模式
 * @param enable
 *        - true : Write Check模式
 *        - false　:　無Write Check模式
 * @return (無)
 */
void dvProAV_AccessWriteCheckEnableSet(bool enable)
{
    m_bWriteCheck = enable;
}

#ifdef QT_CPP
/**
 * @brief 打開暫存器讀寫紀錄檔
 * @return 成功(rcSUCCESS)或失敗(rcERROR)
 */
int dvProAV_AccessLogOpen(void)
{
    accessLog = fopen("ProavAccess.log", "w");
    return (accessLog == 0) ? rcERROR:rcSUCCESS;
}

/**
 * @brief 列印到暫存器讀寫紀錄檔
 * @param (同printf)
 * @return 列印資料長度
 */
int dvProAV_AccessPrint(const char *message, ...)
{
    va_list args;
    int length;

    if (accessLog == 0 || m_debugMask ==0)
        return 0;

    va_start(args, message);
    length = vfprintf(accessLog, message, args); // print to file
    fflush(accessLog);
    va_end (args);
    return length;
}

/**
 * @brief 關閉暫存器讀寫紀錄檔
 * @return (無)
 */
void dvProAV_AccessLogClose(void)
{
    if (accessLog)
    {
        fclose(accessLog);
    }
}
#endif
