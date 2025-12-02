#include "dvProAV_Platform.h"
#include "dvProAV_Access.h"
#include "dvProAV_RegTable.h"
#include <time.h>
#include <stdint.h>
#include "dvProAV_Base.h"
#include "dvProAV_Scaler.h"
#include "dvProAV_SerialFlash.h"
#include "dvProAV_UmbSerialFlash.h"

//#define IN_FILE_DEBUG
#ifndef IN_FILE_DEBUG
    #undef DBMSG
    #define DBMSG(...) do{}while(0)
#endif

#define FLASH_CS_DELAY 0x00000055
#define FLASH_SPI_CLOCK_RATE 0x00000002
#define FLASH_MODE 0
#define FLASH_4BYTE_MODE 1

//-----------------------------------------------------------------------------
// ProAV Serial Flash Public Functions
//-----------------------------------------------------------------------------
/**
 * @brief DRAM To Flash Access
 * @param [in] u32DramAddress : Dram Address
 * @param [in] u32FlashAddress : Flash Address
 * @param [in] u32Length : data bytes
 * @param [in] bDram2Flash : false = Flash2Dram / true = Dram2Flash
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SfiDram2Flash(uint32 u32DramAddress, uint32 u32FlashAddress, uint32 u32Length, bool bDram2Flash)
{
    int status = rcSUCCESS;
    uint08 ucTimerId;
    eUmbFunc eFunc = (bDram2Flash)? eUmbFunc_DramToFlash : eUmbFunc_FlashToDram;
    bool bBank = dvProAV_DramBankGet();
    uint16 uiReTry = 200;
    uint32 timeout = (u32Length > MEMORYBUFMAXBYTE)? MEMORYBUFMAXBYTE : u32Length;

    if(TimerRequire(&ucTimerId) == false)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }

    if(u32FlashAddress % PROAV_DRAM_LINE_BYTES != 0)
    {
        DBMSG("The Flash Address start address should be a %d bytes boundly...\n", PROAV_DRAM_LINE_BYTES);
        return rcERROR;
    }

    u32DramAddress = u32DramAddress / PROAV_DRAM_LINE_BYTES;

    dvProAV_UmbSfiEnter4byteAddressingMode();
    if(bDram2Flash)
    {
       dvProAV_UmbSfiAllSectorUnprotect();
       dvProAV_UmbSfiWriteMemoryCmd(FLASH_MODE, FLASH_4BYTE_MODE);
    }
    else
    {
        dvProAV_UmbSfiReadMemoryCmd(FLASH_MODE, FLASH_4BYTE_MODE);
    }

    // Clear RstLen
    dvProAV_UmbRst(bBank);

    uint32 ulLen = 0;
    uint16 j = (uint16)(u32Length / MEMORYBUFMAXBYTE);
    if(u32Length % MEMORYBUFMAXBYTE > 0)
        j++;
    for(uint16 i=0; i<j; i++)
    {
        if(u32Length > MEMORYBUFMAXBYTE)
        {
            ulLen = MEMORYBUFMAXBYTE;
            u32Length -= MEMORYBUFMAXBYTE;
        }
        else
        {
            ulLen = u32Length;
        }
        uint16 uiOutLen = (eFunc <= eUmbFunc_FlashToDram)? ((uint16)ulLen / PROAV_DRAM_LINE_BYTES) : (((uint16)ulLen / PROAV_DRAM_LINE_BYTES) * 13);  // any function to Dram
        uint16 uiInLen = ((eFunc & 0x0f) == 0)? ((uint16)ulLen / PROAV_DRAM_LINE_BYTES) : (((uint16)ulLen / PROAV_DRAM_LINE_BYTES) * 13); // Dram to any function

        status &= dvProAV_UmbFuncSel(bBank, eFunc);
        status &= dvProAV_UmbDramAddrSet(bBank, u32DramAddress);
        status &= dvProAV_UmbFlashAddrSet(bBank, u32FlashAddress / 4);
        status &= dvProAV_UmbLenSet(bBank, uiOutLen, uiInLen);
        dvProAV_UmbAction(bBank, eUmbActionMode_Continuous);

        ResetTime(ucTimerId);
        while((dvProAV_UmbStatusGet(bBank) != 0)/* && (uiReTry > 0)*/)
        {
            DelayMSec(1);
            //uiReTry--;
            if (TimeElapsed(ucTimerId) > timeout)
            {
                uint08 ucBusyStatus = dvProAV_UmbStatusGet(bBank);
                LOG_MSG(db_ALWAYS, "[ERROR] ucBusyStatus = 0x%x \n", ucBusyStatus);
                dvProAV_UmbRst(bBank);
                LOG_MSG(db_ALWAYS, "[ERROR] DRAM to Flash over %d msec\n", timeout);
                status = rcERROR;
                break;
            }
        }

        u32FlashAddress += MEMORYBUFMAXBYTE;
        u32DramAddress += (MEMORYBUFMAXBYTE / PROAV_DRAM_LINE_BYTES);
    }
    TimerRelease(ucTimerId);

    if(bDram2Flash)
    {
        dvProAV_UmbSfiAllSectorProtection();
    }

    return status;
}

/**
 * @brief SPI To Flash Access
 * @param [in] flashAddress : Flash Address
 * @param [in/out] ucData : data point
 * @param [in] length : data bytes
 * @param [in] bWrFlash : false = Flash2SPI / true = SPI2Flash
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SfiSpi2Flash(uint32 u32FlashAddress, uint08 *ucData, uint32 u32Length, bool bWrFlash)
{
    int status = rcSUCCESS;
    uint08 ucTimerId;
    eUmbFunc eFunc = (bWrFlash)? eUmbFunc_SpiToFlash : eUmbFunc_FlashToSpi;
    bool bBank = dvProAV_DramBankGet();
    uint16 uiReTry = 200;
    uint32 timeout = (u32Length > MEMORYBUFMAXBYTE)? MEMORYBUFMAXBYTE : u32Length;

    if (u32FlashAddress % PROAV_DRAM_LINE_BYTES != 0)
    {
        DBMSG("The Flash Address start address should be a %d bytes boundly...\n", PROAV_DRAM_LINE_BYTES);
        return rcERROR;
    }

    if (TimerRequire(&ucTimerId) == false)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }

    //dramAddress = dramAddress / PROAV_DRAM_LINE_BYTES;

    dvProAV_UmbSfiEnter4byteAddressingMode();
    if(bWrFlash)
    {
       dvProAV_UmbSfiAllSectorUnprotect();
       dvProAV_UmbSfiWriteMemoryCmd(FLASH_MODE, FLASH_4BYTE_MODE);
    }
    else
    {
        dvProAV_UmbSfiReadMemoryCmd(FLASH_MODE, FLASH_4BYTE_MODE);
    }

    // Clear RstLen
    dvProAV_UmbRst(bBank);

    uint16 uiLen = 0;
    uint16 uiLoop = (uint16)(u32Length / MEMORYBUFMAXBYTE);
    if(u32Length % MEMORYBUFMAXBYTE > 0)
        uiLoop++;
    for(uint16 uiCnt=0; uiCnt<uiLoop; uiCnt++)
    {
        if(u32Length > MEMORYBUFMAXBYTE)
        {
            uiLen = MEMORYBUFMAXBYTE;
            u32Length -= MEMORYBUFMAXBYTE;
        }
        else
        {
            uiLen = (uint16)u32Length;
        }
        uint16 uiOutLen = (eFunc <= eUmbFunc_FlashToDram)? ((uint16)uiLen / PROAV_DRAM_LINE_BYTES) : (((uint16)uiLen / PROAV_DRAM_LINE_BYTES) * 13);  // any function to Dram
        uint16 uiInLen = ((eFunc & 0x0f) == 0)? ((uint16)uiLen / PROAV_DRAM_LINE_BYTES) : (((uint16)uiLen / PROAV_DRAM_LINE_BYTES) * 13); // Dram to any function

        status &= dvProAV_UmbFuncSel(bBank, eFunc);
        status &= dvProAV_UmbFlashAddrSet(bBank, u32FlashAddress / 4);
        status &= dvProAV_UmbLenSet(bBank, uiOutLen, uiInLen);
        status &= dvProAV_UmbAction(bBank, eUmbActionMode_Continuous);
        if(bWrFlash)
            status &= dvProAV_UmbDataWrite(bBank, uiLen, ucData + (uiCnt * MEMORYBUFMAXBYTE));
        else
        {
            while(((dvProAV_UmbStatusGet(bBank) & 0x08) != 0x08)/* && (uiReTry > 0)*/)
            {
                DelayMSec(1);
            }
            status &= dvProAV_UmbDataRead(bBank, uiLen, ucData + (uiCnt * MEMORYBUFMAXBYTE));
        }

        ResetTime(ucTimerId);
        while((dvProAV_UmbStatusGet(bBank) != 0)/* && (uiReTry > 0)*/)
        {
            DelayMSec(1);
            //uiReTry--;
            if (TimeElapsed(ucTimerId) > timeout)
            {
                dvProAV_UmbRst(bBank);
                DBMSG("[ERROR] Sector erase over %d msec\n", timeout);
                status = rcERROR;
                break;
            }
        }

        u32FlashAddress += MEMORYBUFMAXBYTE;
    }

    if(bWrFlash)
    {
        dvProAV_UmbSfiAllSectorProtection();
    }

    TimerRelease(ucTimerId);
    return status;
}

/**
 * @brief Flash to Flash
 * @param [in] u16Length : Data Byte
 * @param [in] ulCpyAddr : Copy DRAM Address
 * @param [in] ulPasAddr : Paste DRAM Address
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SfiFlashToFlash(uint16 u16Length, uint32 ulCpyAddr, uint32 ulPasAddr)
{
    int status = rcSUCCESS;
    uint16 uiReTry = 500;
    eUmbFunc eFunc = eUmbFunc_FlashToFlash;
    uint16 uiOutLen = (eFunc <= eUmbFunc_FlashToDram)? (u16Length / PROAV_DRAM_LINE_BYTES) : ((u16Length / PROAV_DRAM_LINE_BYTES) * 13);  // any function to Dram
    uint16 uiInLen = ((eFunc & 0x0f) == 0)? (u16Length / PROAV_DRAM_LINE_BYTES) : ((u16Length / PROAV_DRAM_LINE_BYTES) * 13); // Dram to any function

    status &= dvProAV_UmbFuncSel(eDramBank0, eFunc);
    status &= dvProAV_UmbLenSet(eDramBank0, uiOutLen, uiInLen);
    status &= dvProAV_UmbFlashAddrSet(eDramBank0, ulCpyAddr);
    status &= dvProAV_UmbAction(eDramBank0, eUmbActionMode_Stage);
    while(((dvProAV_UmbStatusGet(eDramBank0) & 0x04) != 0x04))
    {
        DelayMSec(1);
    }
    status &= dvProAV_UmbFlashAddrSet(eDramBank0, ulPasAddr);
    status &= dvProAV_UmbAction(eDramBank0, eUmbActionMode_Stage);
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
 * @brief SPI To DRAM Access
 * @param [in] u32Address : DRAM address
 * @param [in/out] u8Data : data point
 * @param [in] u32Length :
 * @param [in] bWrite : flase = DRAM2SPI / true = SPI2DRAM
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SfiDramAccess(uint32 u32Address, uint08 *u8Data, uint32 u32Length, bool bWrite)
{
    int status = rcSUCCESS;
    uint16 lineCount, wlength = 0x8000;

    if(bWrite)
    {
        wlength = 0x8000;
    }

    if ((u32Address % PROAV_DRAM_LINE_BYTES) != 0)
    {
        DBMSG("The start address should be a %d bytes boundly...\n", m_sfc.u16LineBytes << m_sfc.u08AddrShift);
        return rcERROR;
    }

    if (u32Length % PROAV_DRAM_LINE_BYTES)
    {
        DBMSG("The Line Buf Length should be a %d bytes boundly...\n", m_sfc.u16LineBytes << m_sfc.u08AddrShift);
        return rcERROR;
    }

    for(uint32 i = 0; i < u32Length; i += wlength)
    {
        uint32 restLen;
        restLen = u32Length - i;
        if (restLen >= wlength)
            lineCount = (uint16)(wlength / PROAV_DRAM_LINE_BYTES);
        else
            lineCount = (uint16)(restLen / PROAV_DRAM_LINE_BYTES);

        if (bWrite)
            status &= dvProAV_LineBufWrite(dvProAV_DramBankGet(), (u32Address + i) / PROAV_DRAM_LINE_BYTES, lineCount, u8Data + i);
        else
            status &= dvProAV_LineBufRead(dvProAV_DramBankGet(), (u32Address + i) / PROAV_DRAM_LINE_BYTES, lineCount, u8Data + i);
    }
    return status;
}

/**
 * @brief Flash Erase Sector
 * @param u32Address : Flash address
 * @param u32Length : erase sector length
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SfiEraseSector(uint32 u32Address, uint32 u32Length)
{
    return dvProAV_UmbSfiEraseSector(u32Address, u32Length);
}

int dvProAV_SfiReadDlySet(uint32 ulDly)
{
    return dvProAV_UmbSfiReadDlySet(ulDly);
}

int dvProAV_SfiClkRateSet(uint32 ulSpiClockRate)
{
    return dvProAV_UmbSfiClkRateSet(ulSpiClockRate);
}

/**
 * @brief Serial Flash Csr Read
 * @param [in] ucCsrAddr
 * @return CSR data
 */
int dvProAV_SfiCsrRead(uint8 ucCsrAddr)
{
    return dvProAV_FlashCsrReadData(ucCsrAddr);
}

/**
 * @brief Serial Flash Initial
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SfiInit(void)
{
    DBMSG("dvProAV_SfiInit\n");
    return dvProAV_UmbSfiInit(FLASH_MODE, FLASH_CS_DELAY, FLASH_SPI_CLOCK_RATE);
    //dvProAV_SfiReadDlySet(1);
}
