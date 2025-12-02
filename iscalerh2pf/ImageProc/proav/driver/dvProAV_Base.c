#include "dvProAV_Access.h"
#include "dvProAV_Base.h"
#include <math.h>
#ifndef QT_CPP
    #include "utilDbgMsg.h"
#endif

#ifndef IN_FILE_DEBUG
    #undef DBMSG
    #define DBMSG(...) do{}while(0)
#endif

static VopRefClock m_vopRefClock = vrc148p5Mhz; // vrc270Mhz is modified on version 0x0101
static bool m_bDramBank = false;

/**
 * @brief 重置state machine
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_StateMachineReset(void)
{
    int status;
    status = dvProAV_AccessWrite(eBiuStmRst,(uint32)true);
    status = dvProAV_AccessWrite(eBiuStmRst,(uint32)false);
    return status;
}

//-------------------------------------------------------------------------------------------------
// Reference Clock
//-------------------------------------------------------------------------------------------------
/**
 * @brief 設定Reference Clock
 * @param [in] ref : Reference Clock
 *              -vrc148p5Mhz
 *              -vrc270Mhz
 */
void dvProAV_VopRefClockSet(VopRefClock ref)
{
    m_vopRefClock = ref;
}

/**
 * @brief 讀取Ref Clock
 * @return : 回傳當前Reference Clock
 *          -vrc148p5Mhz
 *          -vrc270Mhz
 */
VopRefClock dvProAV_VopRefClockGet(void)
{
    return m_vopRefClock;
}

//-------------------------------------------------------------------------------------------------
// SPI Control
//-------------------------------------------------------------------------------------------------
/**
 * @brief 設定ProAV Scaler SPI控制權
 * @param [in] signal : SPI source
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_ScalerSPISel(ScalerSPISel signal)
{
    int status = rcSUCCESS;
#ifdef QT_CPP
    if (signal >= eScalerSPISelMax)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }

    status &= dvProAV_AccessWrite(eComRegBnkSel, 0xff);
    status &= dvProAV_AccessWrite(eComRegBnkSel, 0x00);
    status &= dvProAV_AccessWrite(eComSpiSrcSelA, signal);
#endif

    return status;
}

#if 0 // removed SPI Boost Mode
/**
 * @brief ProAV SPI Boost Mode Set
 * @param [in] eSpiCh : SPI channel
 *              - eScalerSPI_PC
 *              - eScalerSPI_MCU
 * @param [in] bEnable :
 *              - false : disable
 *              - true : enable SPI boost mode, MISO will reply one clock cycle in advance
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SpiMisoBoostMode(ScalerSPISel eSpiCh, bool bEnable)
{
    int status = rcSUCCESS;
    uint32 ulreg = 0;
    if(eSpiCh >= eScalerSPISelMax)
        return rcERROR;

    ulreg = (eSpiCh == eScalerSPI_MCU)? eBiuMcu0SpiBoost : eBiuPcSpiBoost;
    status &= dvProAV_AccessWrite(ulreg, bEnable);

    return status;
}
#endif

/**
 * @brief ProAV SPI MISO Delay Control Set
 * @param [in] eSpiCh : SPI channel
 *              - eScalerSPI_PC
 *              - eScalerSPI_MCU
 * @param [in] ucDelay : 0~7 * 3.3 delay time, only valid when SPI Boost enable
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SpiMisoDelaySet(ScalerSPISel eSpiCh, uint8 ucDelay)
{
    int status = rcSUCCESS;
    if(ucDelay > 7)
        ucDelay = 7;
    if(eSpiCh >= eScalerSPISelMax)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }
    uint32 ulreg = (eSpiCh == eScalerSPI_MCU)? eBiuMcuSpiSpd : eBiuPcSpiSpd;

    status &= dvProAV_AccessWrite(ulreg, ucDelay);

    return status;
}

/**
 * @brief Caculate Gamma Table
 * @param [in] ucInputBits: input gamma step bits
 * @param [in] ucOutputBits: output gamma step bits
 * @param [in] ucScaleBits: one step scale bits
 * @param [in] power: gamma power
 * @param [in] length: length of gamma table
 * @param [out] data: Gamma Table Pointer
 * @note
 * fn = ROUNDUP(POWER(index/InputBitsMax,GammaPower)*OutputBitsMax,0)
 */
void dvProAV_GammaTableCaculate(uint08 ucInputBits, uint08 ucOutputBits, uint08 ucScaleBits, double power, void *data)
{
    double value;
    uint32 OutputBitsMax = (1 << ucOutputBits) - 1;
    uint32 InputBitsMax = (1 << ucInputBits);
    uint32 CurveBitsMax = (1 << ucScaleBits) - 1;

    for (uint16 index= 0 ;index < InputBitsMax; index++)
    {
        value = (double)index / CurveBitsMax;
        if (ucOutputBits > 16)
            *((uint32 *)data + index) = (uint32)(pow(value, power) * OutputBitsMax);
        else
            *((uint16 *)data + index) = (uint16)(pow(value, power) * OutputBitsMax);
    }
}

/**
 * @brief Get the write error count for interface
 * @return  write error count
 */
uint08 dvProAV_InterfaceWriteErrorCountGet(void)
{
#if (ACCESS_WRITE_CHECK_ENABLED != 0)
    int status = rcSUCCESS;
    uint32 data;

    if (dvProAV_AccessInterfaceGet() >= eAccessIfMcuPCIe)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }

    status &= dvProAV_AccessRead(eBiuSpiErrorCnt, &data); // this count will be set to 0 when regsiter be read.
    if(status != rcSUCCESS)
    {
        data = 0xFF;
    }
    return (uint08)(data & 0xFF);
#else
    return 0;
#endif
}

//-------------------------------------------------------------------------------------------------
// Line Buffer
//-------------------------------------------------------------------------------------------------
/**
 * @brief 設定Dram Bank
 * @param [in] bank : Dram Bank
 *              - 0 : Scaler DRAM
 *              - 1 : Warp DRAM
 * @return
 */
void dvProAV_DramBankSet(DramBank eBank)
{
    if(eBank >= eDramBankMax)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
    }

    m_bDramBank = eBank;
}

/**
 * @brief 讀取目前Dram Bank
 * @return Dram Bank
 */
DramBank dvProAV_DramBankGet(void)
{
    return (DramBank)m_bDramBank;
}

/**
 * @brief 寫入資料到Warp Line Buffer
 * @param [in] bBank : DRAM Bank
 *             - 0 : Scaler DRAM Bank
 *             - 1 : Warp DRAM Bank
 * @param [in] u32Addr : 起始位置
 * @param [in] u8Data : 資料指標
 * @param [in] u32DataLen : 資料長度(Byte)
 * @param [in] bWrite :
 *              - false : read
 *              - true : write
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_LineBufAccess(DramBank eBank, uint32 u32Addr, uint08 *u8Data, uint32 u32DataLen, bool bWrite)
{
    int status = rcSUCCESS;
    if((u32DataLen == 0) || (eBank >= eDramBankMax))
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }

    const uint16 uiLineBufRWMax = 512;  // 一次寫Line Buffer最大長度
    uint32 ulDRAMLen;
    uint16 uiWriteLen;


    ulDRAMLen = u32DataLen / PROAV_DRAM_LINE_BYTES; // Dada Byte to DRAM Len
    if(u32DataLen % PROAV_DRAM_LINE_BYTES > 0)
        ulDRAMLen++;

    for(uint32 ulOffset = 0 ; ulOffset < ulDRAMLen ; ulOffset += uiLineBufRWMax)
    {
        uint32 ulResidual = (uint32)(ulDRAMLen - ulOffset);
        uint32 ulDataOffset;

        if(ulResidual / uiLineBufRWMax > 0)  // 超過一次最大寫入DRAM長度的資料, 只寫最大長度
            uiWriteLen = uiLineBufRWMax;
        else                                // 未滿最大寫入DRAM長度的資料, 寫入剩餘Data
            uiWriteLen = ulResidual % uiLineBufRWMax;

        ulDataOffset = ulOffset * PROAV_DRAM_LINE_BYTES;

        if(bWrite)
            status &= dvProAV_LineBufWrite(eBank, u32Addr + ulOffset, uiWriteLen, u8Data + ulDataOffset);
        else
            status &= dvProAV_LineBufRead(eBank, u32Addr + ulOffset, uiWriteLen, u8Data + ulDataOffset);
    }

    return status;
}

/**
 * @brief 寫入指定長度的資料到Warp Line Buffer
 * @param [in] bBank : DRAM Bank
 *             - 0 : Scaler DRAM Bank
 *             - 1 : Warp DRAM Bank
 * @param [in] u32Addr : 起始位置
 * @param [in] u16Len : 資料長度 = len(1~256)*64byte(DRAM 資料寬度為512 Bits = 64Byte)
 * @param [in] u8Data : 資料指標
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_LineBufWrite(DramBank eBank, uint32 u32Addr, uint16 u16Len, uint08 *u8Data)
{
    int status = rcSUCCESS;
    if((u16Len == 0) || (eBank >= eDramBankMax))
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }
    uint08 ucTimerId = 0;
    uint16 realLen;
    uint32_t timeout = (u16Len > MEMORYBUFMAXBYTE)? MEMORYBUFMAXBYTE : u16Len;
    //uint16 uiOutLen = (eUmbFunc_SpiToDram <= eUmbFunc_FlashToDram)? len : (len * 13);  // any function to Dram
    //uint16 uiInLen = ((eUmbFunc_SpiToDram & 0x0f) == 0)? len : (len * 13); // Dram to any function
#if 1   // Continuou Mode
    realLen = u16Len % (512 + 1);
    realLen = (realLen == 0)? MEMORYBUFMAXBYTE : (realLen * PROAV_DRAM_LINE_BYTES);   // 16384 = 256*64
    status &= dvProAV_UmbFuncSel(eBank, eUmbFunc_SpiToDram);
    status &= dvProAV_UmbDramAddrSet(eBank, u32Addr);
    status &= dvProAV_UmbLenSet(eBank, realLen / PROAV_DRAM_LINE_BYTES, (realLen / PROAV_DRAM_LINE_BYTES * 13));
    status &= dvProAV_UmbAction(eBank, eUmbActionMode_Continuous);

    ResetTime(ucTimerId);
    while(((dvProAV_UmbStatusGet(eBank) & 0x02) != 0x02))
    {
        DelayMSec(1);
        if(TimeElapsed(ucTimerId) > 100)
        {
            status = rcERROR;
            break;
        }
    }
    status &= dvProAV_UmbDataWrite(eBank, realLen, u8Data);

    ResetTime(ucTimerId);
    while((dvProAV_UmbStatusGet(eBank) != 0)/* && (uiReTry > 0)*/)
    {
        DelayMSec(1);
        if(TimeElapsed(ucTimerId) > timeout)
        {
            LOG_MSG(db_ALWAYS, "[ERROR] Line write time out\n");
            LOG_MSG(db_ALWAYS, "Write addr = 0x%x\n", u32Addr);
            LOG_MSG(db_ALWAYS, "Write len = %d\n", u16Len);
            LOG_MSG(db_ALWAYS, "Write busy status = %d\n", dvProAV_UmbStatusGet(eBank));
            dvProAV_UmbRst(eBank);
            status = rcERROR;
            break;
        }
    }
#else   // Stage Mode
    uint16 uiReTry = 500;

    realLen = u16Len % (512 + 1);
    realLen = (realLen == 0)? MEMORYBUFMAXBYTE : (realLen * PROAV_DRAM_LINE_BYTES);   // 16384 = 256*64

    status &= dvProAV_UmbFuncSel(eBank, eUmbFunc_SpiToDram);
    status &= dvProAV_UmbDramAddrSet(eBank, u32Addr);
    status &= dvProAV_UmbLenSet(eBank, realLen / PROAV_DRAM_LINE_BYTES, (realLen / PROAV_DRAM_LINE_BYTES * 13));
    status &= dvProAV_UmbAction(eBank, eUmbActionMode_Stage);
    while(((dvProAV_UmbStatusGet(eBank) & 0x02) != 0x02))
    {
        DelayMSec(1);
    }
    status &= dvProAV_UmbDataWrite(eBank, realLen, u8Data);
    while(((dvProAV_UmbStatusGet(eBank) & 0x04) != 0x04))
    {
        DelayMSec(1);
    }
    status &= dvProAV_UmbAction(eBank, eUmbActionMode_Stage);
    while(((dvProAV_UmbStatusGet(eBank) & 0x08) != 0x08))
    {
        DelayMSec(1);
    }
    //status &= dvProAV_UmbDataRead(eDramBank0, len, ucRData);
    while((dvProAV_UmbStatusGet(eDramBank0) != 0) && (uiReTry > 0))
    {
        DelayMSec(1);
        uiReTry--;
        if(uiReTry == 0)
        {
            dvProAV_UmbRst(eDramBank0);
            status = rcERROR;
        }
    }
#endif

    return status;
}

/**
 * @brief 讀取指定長度的資料到Warp Line Buffer
 * @param [in] bBank : DRAM Bank
 *             - 0 : Scaler DRAM Bank
 *             - 1 : Warp DRAM Bank
 * @param [in] u32Addr : 起始位置 = address
 * @param [in] u16Len : 資料長度 = len(1~256)*64byte(DRAM 資料寬度為512 Bits = 64Byte)
 * @param [in] u8Data : 資料
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_LineBufRead(DramBank eBank, uint32 u32Addr, uint16 u16Len, uint08 *u8Data)
{
    int status = rcSUCCESS;
    uint08 ucTimerId = 0;
    uint16 realLen;
    uint32_t timeout = (u16Len > MEMORYBUFMAXBYTE)? MEMORYBUFMAXBYTE : u16Len;
    //uint16 uiOutLen = (eUmbFunc_SpiToDram <= eUmbFunc_FlashToDram)? len : (len * 13);  // any function to Dram
    //uint16 uiInLen = ((eUmbFunc_SpiToDram & 0x0f) == 0)? len : (len * 13); // Dram to any function
#if 1   // Continuou Mode
    realLen = u16Len % (512 + 1);
    realLen = (realLen == 0)? MEMORYBUFMAXBYTE : (realLen * PROAV_DRAM_LINE_BYTES);
    status &= dvProAV_UmbFuncSel(eBank, eUmbFunc_DramToSpi);
    status &= dvProAV_UmbDramAddrSet(eBank, u32Addr);
    status &= dvProAV_UmbLenSet(eBank, (realLen / PROAV_DRAM_LINE_BYTES * 13), realLen / PROAV_DRAM_LINE_BYTES);
    status &= dvProAV_UmbAction(eBank, eUmbActionMode_Continuous);
    while(((dvProAV_UmbStatusGet(eBank) & 0x08) != 0x08))
    {
        DelayMSec(1);
    }
    status &= dvProAV_UmbDataRead(eBank, realLen, u8Data);

    ResetTime(ucTimerId);
    while((dvProAV_UmbStatusGet(eBank) != 0)/* && (uiReTry > 0)*/)
    {
        DelayMSec(1);
        if(TimeElapsed(ucTimerId) > timeout)
        {
            LOG_MSG(db_ALWAYS, "[ERROR] Line read time out\n");
            LOG_MSG(db_ALWAYS, "Read addr = 0x%x\n", u32Addr);
            LOG_MSG(db_ALWAYS, "Read len = %d\n", u16Len);
            LOG_MSG(db_ALWAYS, "Read busy status = %d\n", dvProAV_UmbStatusGet(eBank));
            dvProAV_UmbRst(eBank);
            status = rcERROR;
            break;
        }
    }
#else   // Stage Mode
    uint16 uiReTry = 500;

    realLen = u16Len % (512 + 1);
    realLen = (realLen == 0)? MEMORYBUFMAXBYTE : (realLen * PROAV_DRAM_LINE_BYTES);

    status &= dvProAV_UmbFuncSel(eBank, eUmbFunc_DramToSpi);
    status &= dvProAV_UmbDramAddrSet(eBank, u32Addr);
    status &= dvProAV_UmbLenSet(eBank, (realLen / PROAV_DRAM_LINE_BYTES * 13), realLen / PROAV_DRAM_LINE_BYTES);
    status &= dvProAV_UmbAction(eBank, eUmbActionMode_Stage);
    uiReTry = 500;
    while(((dvProAV_UmbStatusGet(eBank) & 0x04) != 0x04))
    {
        DelayMSec(5);
        uiReTry--;
        if(uiReTry == 0)
        {
            dvProAV_UmbRst(eBank);
            return rcERROR;
        }
    }
    status &= dvProAV_UmbAction(eBank, eUmbActionMode_Stage);
    status &= dvProAV_UmbAction(eBank, eUmbActionMode_Stage);
    while(((dvProAV_UmbStatusGet(eDramBank0) & 0x08) != 0x08))
    {
        DelayMSec(1);
    }
    status &= dvProAV_UmbDataRead(eBank, realLen, u8Data);
    while((dvProAV_UmbStatusGet(eDramBank0) != 0) && (uiReTry > 0))
    {
        DelayMSec(1);
        uiReTry--;
        if(uiReTry == 0)
        {
            dvProAV_UmbRst(eDramBank0);
            status = rcERROR;
        }
    }
#endif
    return status;
}

// ==============================================================================
// FUNCTION NAME: dvProAV_WarpDramWriteEanle
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
// 2019/12/20, Larry Create
// --------------------
// ==============================================================================
int dvProAV_DramWriteEanle(DramBank eBank, uint32 addr, uint8 WriteEn)
{
    int status = rcSUCCESS;
    if(eBank >= eDramBankMax)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }

    if(WriteEn)
    {
        status &= dvProAV_AccessWrite((eBank)? eWcuD2LnRWAdr : eDcuLnRWAdr, (addr >> 1));

        MS_SLEEP(2);
        status &= dvProAV_AccessWrite((eBank)? eWcuD2LnWrEn : eDcuLnWrEn, (uint32)true);
    }
    else
    {
        status &= dvProAV_AccessWrite((eBank)? eWcuD2LnWrEn : eDcuLnWrEn, (uint32)false);
    }

    return status;
}

// ==============================================================================
// FUNCTION NAME: dvProAV_WarpDramReadEanle
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
// 2019/12/20, Larry Create
// --------------------
// ==============================================================================
int dvProAV_DramReadEanle(DramBank eBank, uint32 addr, uint8 ReadEn)
{
    int status = rcSUCCESS;
    if(eBank >= eDramBankMax)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }

    if(ReadEn)
    {
        status &= dvProAV_AccessWrite((eBank)? eWcuD2LnRWAdr : eDcuLnRWAdr, (addr >> 1));

        MS_SLEEP(2);
        status &= dvProAV_AccessWrite((eBank)? eWcuD2LnRdEn : eDcuLnRdEn, (uint32)true);

    }
    else
    {
        status &= dvProAV_AccessWrite((eBank)? eWcuD2LnRdEn : eDcuLnRdEn, (uint32)false);
    }

    return status;
}

//-------------------------------------------------------------------------------------------------
// DRAM Clear
//-------------------------------------------------------------------------------------------------
/**
 * @brief Get OSD busy state
 * @return ulBusyStatus : 0=unbusy, 1=busy
 */
static bool dvProAV_ClearDramBusyGet(void)
{
    UINT32 ulBusyStatus = 0;
    int status = rcSUCCESS;
    status &= dvProAV_AccessRead(eOsdBusy, &ulBusyStatus);
    if(status != rcSUCCESS)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
    }
    return (bool)ulBusyStatus;
}

/**
 * @brief Get OSD Vsync Wait state
 * @return ulBusyStatus : 0=unbusy, 1=busy
 */
static BOOL dvProAV_ClearDramVsStateGet(void)
{
    UINT32 ulBusyStatus = 0;
    int status = rcSUCCESS;
    status &= dvProAV_AccessRead(eOsdVSBusy, &ulBusyStatus);
    if(status != rcSUCCESS)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
    }
    return (BOOL)ulBusyStatus;
}

/**
 * @brief dvProAV_ClearDram
 * @param [in] bBank : DRAM Bank
                - 0 : Scaler DRAM Bank
                - 1 : Warp DRAM Bank
 * @param [in] u32Addr : Dram start address
 * @param [in] u32LenByte : byets
 * @param [in] u8Data : Dram data value
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_ClearDram(DramBank eBank, uint32 u32Addr, uint32 u32LenByte, uint8 u8Data)
{
    int status = rcSUCCESS;
    if(eBank >= eDramBankMax)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }

    // 畫透明色方式清DRAM
    if(eBank == eDramBank0)
    {
        status &= dvProAV_AccessWrite(eOsdInhiColorEn, false);  // 關閉禁止色

        uint32 uiTranNum = ((u32LenByte + (32 * PROAV_DRAM_LINE_BYTES) - 1) / (32 * PROAV_DRAM_LINE_BYTES));     // (Hsize*Vsize)/(一次寫入32個DRAM位置*DRAM位置Byte數), 無條件進位
        status &= dvProAV_AccessWrite(eOsdBmpFillClr, (uint16)u8Data);                  // 設定填入顏色
        status &= dvProAV_AccessWrite(eOsdTranNum, uiTranNum);                          // 設定尺寸
        status &= dvProAV_AccessWrite(eOsdMemWrAdr, u32Addr);                           // 設定寫入DRAM address
        status &= dvProAV_AccessWrite(eOsdTranTrig, true);                              // 開始寫入

        while((dvProAV_ClearDramVsStateGet() != 0) || (dvProAV_ClearDramBusyGet() != 0))  // check busy
        {

        }

        status &= dvProAV_AccessWrite(eOsdInhiColorEn, true);  // 開啟禁止色
    }
    else
    {
        uint8 *buf;
        buf = (uint08 *)malloc(u32LenByte);
        memset(buf, u8Data, u32LenByte);
        status &= dvProAV_LineBufAccess(eDramBank1, u32Addr, buf, u32LenByte, true);
        free(buf);
    }
    return (status == rcSUCCESS);
}

//-----------------------------------------------------------------------------
// Universal Memory Buffer-Scaler
//-----------------------------------------------------------------------------
int dvProAV_UmbSpiToSpi(uint16 u16Len, uint08 *ucWData, uint08 *ucRData)
{
    int status = rcSUCCESS;
    if(u16Len == 0)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }
    uint16 uiReTry = 500;
    eUmbFunc eFunc = eUmbFunc_SpiToSpi;
    uint16 uiOutLen = (eFunc <= eUmbFunc_FlashToDram)? (u16Len / PROAV_DRAM_LINE_BYTES) : ((u16Len / PROAV_DRAM_LINE_BYTES) * 13);  // any function to Dram
    uint16 uiInLen = ((eFunc & 0x0f) == 0)? (u16Len / PROAV_DRAM_LINE_BYTES) : ((u16Len / PROAV_DRAM_LINE_BYTES) * 13); // Dram to any function


    status &= dvProAV_UmbFuncSel(eDramBank0, eFunc);
    status &= dvProAV_UmbLenSet(eDramBank0, uiOutLen, uiInLen);
    status &= dvProAV_UmbAction(eDramBank0, eUmbActionMode_Stage);
    while(((dvProAV_UmbStatusGet(eDramBank0) & 0x02) != 0x02))
    {
        DelayMSec(1);
    }
    status &= dvProAV_UmbDataWrite(eDramBank0, u16Len, ucWData);
    while(((dvProAV_UmbStatusGet(eDramBank0) & 0x04) != 0x04))
    {
        DelayMSec(1);
    }
    status &= dvProAV_UmbAction(eDramBank0, eUmbActionMode_Stage);
    while(((dvProAV_UmbStatusGet(eDramBank0) & 0x08) != 0x08))
    {
        DelayMSec(1);
    }
    status &= dvProAV_UmbDataRead(eDramBank0, u16Len, ucRData);
    while((dvProAV_UmbStatusGet(eDramBank0) != 0) && (uiReTry > 0))
    {
        DelayMSec(1);
        uiReTry--;
        if(uiReTry == 0)
        {
            dvProAV_UmbRst(eDramBank0);
            status = rcERROR;
        }
    }

    return status;
}

/**
 * @brief Universal Memory Buffer reset
 * @param [in] bBank : DRAM Bank
 *              - 0 : Scaler DRAM Bank
 *              - 1 : Warp DRAM Bank
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_UmbRst(DramBank eBank)
{
    int status = rcSUCCESS;
    if(eBank >= eDramBankMax)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }
    status &= dvProAV_AccessWrite((eBank)?eUMBWBufCoreRst : eUMBSBufCoreRst, true);
    status &= dvProAV_AccessWrite((eBank)?eUMBWBufCoreRst : eUMBSBufCoreRst, false);
    return status;
}

/**
 * @brief Universal Memory Buffer Action mode
 * @param [in] bBank : DRAM Bank
 *              - 0 : Scaler DRAM Bank
 *              - 1 : Warp DRAM Bank
 * @param [in] eMode : Action mode
 *              -eUmbActionMode_Continuous
 *              -eUmbActionMode_Stage
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_UmbAction(DramBank eBank, eUmbActionMode eMode)
{
    int status = rcSUCCESS;
    if((eBank >= eDramBankMax) || ((eMode != eUmbActionMode_Continuous) && (eMode != eUmbActionMode_Stage)))
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }
    status &= dvProAV_AccessWrite((eBank)?eUMBWBufAct : eUMBSBufAct, eMode);
    return status;
}

/**
 * @brief Universal Memory Buffer function select
 * @param [in] bBank : DRAM Bank
 *              - 0 : Scaler DRAM Bank
 *              - 1 : Warp DRAM Bank
 * @param [in] eFunc
 *              -eUmbFunc_DramToDram = 0x00,
 *              -eUmbFunc_SpiToDram = 0x01,
 *              -eUmbFunc_FlashToDram = 0x02,
 *              -eUmbFunc_DramToSpi = 0x10,
 *              -eUmbFunc_SpiToSpi = 0x11,
 *              -eUmbFunc_FlashToSpi = 0x12,
 *              -eUmbFunc_DramToFlash = 0x20,
 *              -eUmbFunc_SpiToFlash = 0x21,
 *              -eUmbFunc_FlashToFlash = 0x22,
 *              -eUmbFunc_DramToGamma = 0x30,
 *              -eUmbFunc_SpiToGamma = 0x31,
 *              -eUmbFunc_FlashToGamma = 0x32,
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_UmbFuncSel(DramBank eBank, eUmbFunc eFunc)
{
    int status = rcSUCCESS;
    if((eBank >= eDramBankMax) || ((eFunc > eUmbFunc_FlashToDram) && (eFunc < eUmbFunc_DramToSpi)) ||
                                  ((eFunc > eUmbFunc_FlashToSpi) && (eFunc < eUmbFunc_DramToFlash)) ||
                                  ((eFunc > eUmbFunc_FlashToFlash) && (eFunc < eUmbFunc_DramToGamma)) ||
                                   (eFunc > eUmbFunc_FlashToGamma))
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }
    status &= dvProAV_AccessWrite((eBank)?REG_UMBWUmbBufSelPort : REG_UMBSUmbBufSelPort, eFunc);
    return status;
}

/**
 * @brief Universal Memory Buffer Length Set
 * @param [in] bBank : DRAM Bank
 *              - 0 : Scaler DRAM Bank
 *              - 1 : Warp DRAM Bank
 * @param [in] uiOutLen : Buffer output length, buffer max size 512bit x 512
 * @param [in] uiInLen : Buffer input length, buffer max size 512bit x 512
 *              DRAM -> 1 pkc*n, 1 pkc represent 64 bytes, length max is 512
 *              CPU, FLASH, gamma -> 13 pkc*n, 13 pkc represent 12x5 bytes + 4 bytes, length max is 6656
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_UmbLenSet(DramBank eBank, uint16 uiOutLen, uint16 uiInLen)
{
    int status = rcSUCCESS;
    if(eBank >= eDramBankMax)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }
    uint32 ulLen = (uiOutLen & 0x1fff);
    ulLen = ((ulLen << 16) & 0x1fff0000) | (uiInLen & 0x1fff);
    status &= dvProAV_AccessBurstWrite((eBank)?eUMBWBufLen : eUMBSBufLen, (uint08 *)(&ulLen), 4, BURST_FIX_ADDR);
    return status;
}

/**
 * @brief Get Universal Memory Buffer state
 * @param [in] bBank : DRAM Bank
 *              - 0 : Scaler DRAM Bank
 *              - 1 : Warp DRAM Bank
 * @return ulStatus : status
 *          -bit 0 : buffer core busy
 *          -bit 1 : writing (FSM)
 *          -bit 2 : waiting (FSM)
 *          -bit 3 : reading (FSM)
 */
uint08 dvProAV_UmbStatusGet(DramBank eBank)
{
    if(eBank >= eDramBankMax)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }
    UINT32 ulStatus = 0;
    dvProAV_AccessRead((eBank)?eUMBWStatus : eUMBSStatus, &ulStatus);
    return (uint08)(ulStatus & 0x0f);
}

/**
 * @brief dvProAV_UmbDataFmtSet
 * @param [in] bBigEnd :
 *              - 0 : little-endian
 *              - 1 : big-endian
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_UmbDataFmtSet(bool bBigEnd)
{
    int status = rcSUCCESS;
    status &= dvProAV_AccessWrite(eUMBSSpiDatCtrl, bBigEnd);
    return status;
}

/**
 * @brief Universal Memory Buffer Data Write
 * @param [in] bBank : DRAM Bank
 *              - 0 : Scaler DRAM Bank
 *              - 1 : Warp DRAM Bank
 * @param [in] ulWLen : write data byte
 * @param [in] ucData : write data array
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_UmbDataWrite(DramBank eBank, uint16 ulWLen, uint08 *ucData)
{
    int status = rcSUCCESS;
    if((eBank >= eDramBankMax) || (ulWLen > MEMORYBUFMAXBYTE))
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }

    status &= dvProAV_AccessBurstWrite((eBank)?eUMBWSpiDataPort : eUMBSSpiDataPort, ucData, ulWLen, BURST_FIX_ADDR);
    return status;
}

/**
 * @brief Universal Memory Buffer Data Read
 * @param [in] bBank : DRAM Bank
 *              - 0 : Scaler DRAM Bank
 *              - 1 : Warp DRAM Bank
 * @param [in] ulWLen : read data byte
 * @param [out] ucData : read data array
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_UmbDataRead(DramBank eBank, uint16 ulRLen, uint08 *ucData)
{
    int status = rcSUCCESS;
    if((eBank >= eDramBankMax) || (ulRLen > MEMORYBUFMAXBYTE))
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }

    status &= dvProAV_AccessBurstRead((eBank)?eUMBWSpiDataPort : eUMBSSpiDataPort, ucData, ulRLen, BURST_FIX_ADDR);
    return status;
}

/**
 * @brief DRAM Address Set
 * @param [in] bBank : DRAM Bank
 *              - 0 : Scaler DRAM Bank
 *              - 1 : Warp DRAM Bank
 * @param [in] ulDramAddr : DRAM Address, 64byte data pack
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_UmbDramAddrSet(DramBank eBank, uint32 ulDramAddr)
{
    int status = rcSUCCESS;
    if(eBank >= eDramBankMax)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }

    ulDramAddr = ulDramAddr & 0x01ffffff;
    status &= dvProAV_AccessBurstWrite((eBank)?eUMBWDrmAdrPort : eUMBSDrmAdrPort, (uint08 *)(&ulDramAddr), 4, BURST_FIX_ADDR);
    return status;
}

/**
 * @brief dvProAV_UmbDramToDram
 * @param [in] bBank : DRAM Bank
 *              - 0 : Scaler DRAM Bank
 *              - 1 : Warp DRAM Bank
 * @param [in] u16len : Data Byte
 * @param [in] ulCpyAddr : Copy DRAM Address
 * @param [in] ulPasAddr : Paste DRAM Address
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_UmbDramToDram(DramBank eBank, uint16 u16len, uint32 ulCpyAddr, uint32 ulPasAddr)
{
    int status = rcSUCCESS;
    if((eBank >= eDramBankMax) || (u16len == 0))
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }
    uint16 uiReTry = 500;
    eUmbFunc eFunc = eUmbFunc_DramToDram;
    uint16 uiOutLen = (eFunc <= eUmbFunc_FlashToDram)? (u16len / PROAV_DRAM_LINE_BYTES) : ((u16len / PROAV_DRAM_LINE_BYTES) * 13);  // any function to Dram
    uint16 uiInLen = ((eFunc & 0x0f) == 0)? (u16len / PROAV_DRAM_LINE_BYTES) : ((u16len / PROAV_DRAM_LINE_BYTES) * 13); // Dram to any function

    status &= dvProAV_UmbFuncSel(eBank, eFunc);
    status &= dvProAV_UmbLenSet(eBank, uiOutLen, uiInLen);
    status &= dvProAV_UmbDramAddrSet(eBank, ulCpyAddr);
    status &= dvProAV_UmbAction(eBank, eUmbActionMode_Stage);

    uiReTry = 500;
    while(((dvProAV_UmbStatusGet(eBank) & 0x04) != 0x04))
    {
        DelayMSec(5);
        uiReTry--;
        if(uiReTry == 0)
        {
            dvProAV_UmbRst(eBank);
            return rcERROR;
        }
    }

    status &= dvProAV_UmbDramAddrSet(eBank, ulPasAddr);
    status &= dvProAV_UmbAction(eBank, eUmbActionMode_Stage);

    uiReTry = 500;
    while((dvProAV_UmbStatusGet(eBank) != 0) && (uiReTry > 0))
    {
        DelayMSec(5);
        uiReTry--;
        if(uiReTry == 0)
        {
            LOG_MSG(db_ALWAYS, "[ERROR] Dram to Dram time out\n");
            dvProAV_UmbRst(eBank);

            return rcERROR;
        }
    }

    return status;
}

//-----------------------------------------------------------------------------
// Universal Memory Buffer - Flash
//-----------------------------------------------------------------------------
/**
 * @brief serial flash Ip reset
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_UmbFlashIpRst(void)
{
    int status = rcSUCCESS;
    status &= dvProAV_AccessWrite(eUMBSSfcIpRst, true);
    status &= dvProAV_AccessWrite(eUMBSSfcIpRst, false);
    return status;
}

/**
 * @brief Serial Flash Select
 * @param [in] bFlashSel :
 *              -0 : common FLASH
 *              -1 : boot FLASH
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_UmbFlashSel(bool bFlashSel)
{
    int status = rcSUCCESS;
    status &= dvProAV_AccessWrite(eUMBSSfcIpSel, bFlashSel);
    return status;
}

/**
 * @brief Flash Csr Command Set
 * @param [in] ucCmd : CSR command
 *              -0X : finish
 *              -10 : read
 *              -11 : write
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_UmbFlashCsrCmdSet(uint08 ucCmd)
{
    int status = rcSUCCESS;
    if((ucCmd >= 4))
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }

    status &= dvProAV_AccessWrite(eUMBSCsrCmd, ucCmd);
    return status;
}

/**
 * @brief Flash Csr Address Set
 * @param [in] ucAddr : CSR address
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_UmbFlashCsrAddrSet(uint08 ucAddr)
{
    int status = rcSUCCESS;
    if((ucAddr >= 32))
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }

    status &= dvProAV_AccessWrite(eUMBSCsrAdr, ucAddr);
    return status;
}

/**
 * @brief Flash Csr Data Write
 * @param [in] ulWLen : write csr data byte
 * @param [in] ucData : write csr data array
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_UmbFlashCsrWrite(uint16 ulWLen, uint08 *ucData)
{
    int status = rcSUCCESS;
    if(ulWLen > MEMORYBUFMAXBYTE)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }

    status &= dvProAV_AccessBurstWrite(eUMBSCsrDatPort, ucData, ulWLen, BURST_FIX_ADDR);
    return status;
}

/**
 * @brief Flash Csr Data Read
 * @param [in] ulRLen : read csr data byte
 * @param [out] ucData : read csr data array
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_UmbFlashCsrRead(uint16 ulRLen, uint08 *ucData)
{
    int status = rcSUCCESS;
    if(ulRLen > MEMORYBUFMAXBYTE)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }

    status &= dvProAV_AccessBurstRead(eUMBSCsrDatPort, ucData, ulRLen, BURST_FIX_ADDR);
    return status;
}

/**
 * @brief Flash Address Set
 * @param [in] bBank : DRAM Bank
 *              - 0 : Scaler DRAM Bank
 *              - 1 : Warp DRAM Bank
 * @param [in] ulFlashAddr : Flash Address, 4byte data pack
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_UmbFlashAddrSet(DramBank eBank, uint32 ulFlashAddr)
{
    int status = rcSUCCESS;
    if((eBank >= eDramBankMax) || (ulFlashAddr > 0x01ffffff))
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }

    status &= dvProAV_AccessBurstWrite((eBank)?eUMBWFlsAdrPort : eUMBSFlsAdrPort, (uint08 *)(&ulFlashAddr), 4, BURST_FIX_ADDR);
    return status;
}

//-----------------------------------------------------------------------------
// Universal Memory Buffer - Gamma
//-----------------------------------------------------------------------------
/**
 * @brief Universal Memory Buffer Hdr Type Set
 * @param ucHdrType : HDR type
 *          -0: HDR gamma 1
 *          -1: HDR gamma 30
 *          -2: HDR gamma 31
 *          -3: HDR gamma 4
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_UmbHdrTypeSet(uint08 ucHdrType)
{
    int status = rcSUCCESS;
    if((ucHdrType >= 4))
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }
    status &= dvProAV_AccessWrite(eUMBSHdrType, ucHdrType);
    return status;
}

/**
 * @brief Universal Memory Buffer Gamma Rgb Same Table Set
 * @param bTableType : (HDR mode not applicable)
 *          -false : RGB use the same
 *          -true : different table
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_UmbRgbTableTypeSet(bool bTableType)
{
    int status = rcSUCCESS;
    status &= dvProAV_AccessWrite(eUMBSRgbSep, bTableType);
    return status;
}

/**
 * @brief Universal Memory Buffer Gamma Type Set
 * @param bGammaType : Package of gamma table type
 *          -false : General image gamma table (take 12 bits for every 2 bytes)
 *          -true : HDR gamma table (take 26 bits for every 4 bytes)
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_UmbGammaTypeSet(bool bGammaType)
{
    int status = rcSUCCESS;
    status &= dvProAV_AccessWrite(eUMBSPkcType, bGammaType);
    return status;
}

/**
 * @brief Gamma Table Write
 * @param [in] eFunc : only accept the following three functions
 *              - eUmbFunc_DramToGamma
 *              - eUmbFunc_SpiToGamma
 *              - eUmbFunc_FlashToGamma
 * @param [in] ulAddr : different functions of "eFunc" have different meanings
 *              - eUmbFunc_DramToGamma : ulAddr = Dram Address
 *              - eUmbFunc_SpiToGamma : ulAddr = Invalid
 *              - eUmbFunc_FlashToGamma : ulAddr = Flash Address
 * @param [in] eGammaType
 *              - eUmbGammaType_VopGamma
 *              - eUmbGammaType_Hdr1
 *              - eUmbGammaType_Hdr30
 *              - eUmbGammaType_Hdr31
 *              - eUmbGammaType_Hdr4
 * @param [in] len : gamma table bytes
 * @param [in] ucData : data point
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_UmbGammaWrite(eUmbFunc eFunc, uint32 ulAddr, eUmbGammaType eGammaType, uint16 len, uint08 *ucData)
{
    int status = rcSUCCESS;
    if((eFunc < eUmbFunc_DramToGamma) || (eFunc > eUmbFunc_FlashToGamma))
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }
    if(eGammaType >= eUmbGammaType_Max)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }
    uint16 uiReTry = 500;
    uint16 uiOutLen = (eFunc <= eUmbFunc_FlashToDram)? (len / PROAV_DRAM_LINE_BYTES) : ((len / PROAV_DRAM_LINE_BYTES) * 13);  // any function to Dram
    uint16 uiInLen = ((eFunc & 0x0f) == 0)? (len / PROAV_DRAM_LINE_BYTES) : ((len / PROAV_DRAM_LINE_BYTES) * 13); // Dram to any function

    if(eGammaType == eUmbGammaType_VopGamma)
    {
        status &= dvProAV_UmbGammaTypeSet(false);
    }
    else
    {
        status &= dvProAV_UmbGammaTypeSet(true);
        status &= dvProAV_UmbHdrTypeSet((uint08)(eGammaType - 1));
    }

    status &= dvProAV_UmbFuncSel(eDramBank0, eFunc);
    if(eFunc == eUmbFunc_DramToGamma)
        status &= dvProAV_UmbDramAddrSet(eDramBank0, ulAddr);
    if(eFunc == eUmbFunc_FlashToGamma)
        status &= dvProAV_UmbFlashAddrSet(eDramBank0, ulAddr);
    status &= dvProAV_UmbLenSet(eDramBank0, uiOutLen, uiInLen);
    status &= dvProAV_UmbAction(eDramBank0, eUmbActionMode_Continuous);
    if(eFunc == eUmbFunc_SpiToGamma)
    {
        uiReTry = 500;

        while(((dvProAV_UmbStatusGet(eDramBank0) & 0x02) != 0x02))
        {

            DelayMSec(1);
            uiReTry--;
            if(uiReTry == 0)
            {
                dvProAV_UmbRst(eDramBank0);
                return rcERROR;
            }
        }
        status &= dvProAV_UmbDataWrite(eDramBank0, len, ucData);
    }

    uiReTry = 500;

    while((dvProAV_UmbStatusGet(eDramBank0) != 0) && (uiReTry > 0))
    {
        DelayMSec(1);
        uiReTry--;
        if(uiReTry == 0)
        {
            dvProAV_UmbRst(eDramBank0);
            status = rcERROR;
        }
    }

    return status;
}
