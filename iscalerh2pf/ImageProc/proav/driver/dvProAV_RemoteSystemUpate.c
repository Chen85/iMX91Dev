#include "dvProAV_Platform.h"
#include "dvProAV_Access.h"
#include "dvProAV_RegTable.h"
#include <time.h>
#include <stdint.h>
#include "dvProAV_Base.h"
#include "dvProAV_Scaler.h"
#include "dvProAV_GenericSerialFlash.h"

//#define IN_FILE_DEBUG
#ifndef IN_FILE_DEBUG
    #undef DBMSG
    #define DBMSG(...) do{}while(0)
#endif

static SfcInfo m_sfc = {
    "Fpga Flash",
    eRsuRestFlashIp,
    eRsuRorceRestSfls,
    0,
    eRsuFlashOrgAddr,
    eRsuMoveLenMax,
    eRsuMode,
    eRsuActMode,
    eRsuUpBufBz,
    eRsuFlashCsrAdr,
    eRsuFlashCsrCmd,
    eRsuFlashCsrWrData,
    eRsuFlashCsrRdData,
    RSU_SRAM_LINE_BYTES,
    RSU_SRAM_ADDR_SHIFT,
    RSU_SRAM_MIN_BYTES,
    RSU_SRAM_BANK_SIZE,
    0x00000055,  // set CS assert & de-assert both delay to 5
    0x00000002,  // set SPI clock to 1/4 IP clock (133Mhz/2 = 33.25 MHz)
    true,
    true,
    0, // 0 by rest length, 1 by busy
    1  // 0 1-1-1 mode, 1: 1-4-4 mode (command 1 bits, address 4 bits, data 4 bits)
};


int dvProAV_RsuInit(void)
{
    DBMSG("dvProAV_RsuInit\n");
    dvProAV_AccessWrite(REG_RsuCtl, 0x00);
    m_sfc.bTopMode = true;
    m_sfc.b4BytesMode = true;
    dvProAV_GenericSfiInit(&m_sfc);
    return rcSUCCESS;
}

int dvProAV_RsuFlashReadDlySet(uint32 ulDly)
{
    return dvProAV_GenericSfiReadDlySet(&m_sfc, ulDly);
}

int dvProAV_RsuFlashClkRateSet(uint32 ulSpiClockRate)
{
    return dvProAV_GenericSfiClkRateSet(&m_sfc, ulSpiClockRate);
}

int dvProAV_RsuFlashEraseSector(uint32 address, uint32 length)
{
    return dvProAV_GenericSfiEraseSector(&m_sfc, address, length);
}

static int _Rsu_StateWait(uint32 regEnum, uint32 waitstat, uint32 timeout, uint16 delayms)
{
    int status = rcSUCCESS;
    uint32 stat;
    uint08 ucTimerId;
    uint32 currentTime;

    if (TimerRequire(&ucTimerId) == false)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcERROR;
    }

    ResetTime(ucTimerId);
    currentTime = TimeElapsed(ucTimerId);
    while (1)
    {
        status &= dvProAV_AccessRead(regEnum, &stat);
        if (stat == waitstat)
            break;

        currentTime = TimeElapsed(ucTimerId);
        if (currentTime > timeout)
        {
            DBMSG("[ERROR] Wait %08X = %08X over %d msec\n", regEnum, waitstat, timeout);
            status = rcERROR;
            break;
        }
        DelayMSec(delayms);
    }
    TimerRelease(ucTimerId);

    return status;
}

static int _Rsu_StartReconfig(uint32 timeout)
{
    int status = rcSUCCESS;
    uint32 stat;
    uint08 ucTimerId;
    uint32 currentTime;

    if (TimerRequire(&ucTimerId) == false)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcERROR;
    }

    ResetTime(ucTimerId);
    while (1)
    {
        currentTime = TimeElapsed(ucTimerId);

        status &= dvProAV_AccessRead(REG_RsuCtl, &stat);
        if (stat ==0xFF)
        {
                 break;
        }
        status &= dvProAV_AccessWrite(eRsuStaReCfg, 0);
        status &= dvProAV_AccessWrite(eRsuStaReCfg, 1);

        if (currentTime>timeout)
        {
            DBMSG("[ERROR] Start Reconfig over %d msec\n", timeout);
            status = rcERROR;
            break;
        }
    }

    TimerRelease(ucTimerId);
    //DBMSG("[Timer ID = %d] Reconfig time = %d\n",ucTimerId , currentTime);
    return status;
}

int dvProAV_RsuReadImageStatus(uint32 *errStatus)
{
    int status = rcSUCCESS;

    // Set mode to "update error status"
    status &= dvProAV_AccessWrite(eRsuMode, 0);

    // Active the operation sysytem image
    status &= dvProAV_AccessWrite(eRsuActMode, 1);
    status &= dvProAV_AccessWrite(eRsuActMode, 0);

    // wait op busy to 0
    status &= _Rsu_StateWait(eRsuOpBz, 0, 1000, 1);

    // Read error status
    status &= dvProAV_AccessRead(eRsuEreState, errStatus);

    return status;
}

int dvProAV_RsuSetStartAddress(uint32 address)
{
    return dvProAV_AccessWrite(eRsuCutmPgAdr, address);
}

int dvProAV_RsuReConfig(uint32 customAddr, uint32 watchdogTime)
{
    int status = rcSUCCESS;

    // Set mode to "switch image page"
    status &= dvProAV_AccessWrite(eRsuMode, 1);     // 11 05

    // Select the Image Page
    status &= dvProAV_AccessWrite(eRsuPgSel, 3); // 3 for customer address
    status &= dvProAV_AccessWrite(eRsuCutmPgAdr, customAddr);

    // Enable or Disable Watchdog
    if (watchdogTime == 0)
    {
        status &= dvProAV_AccessWrite(eRsuEnWhDgTime, 0);
    }
    else
    {
        status &= dvProAV_AccessWrite(eRsuEnWhDgTime, 1);
        // to do
        status &= dvProAV_AccessWrite(eRsuWhDgTime, watchdogTime);

    }

    // Active the operation sysytem image
    status &= dvProAV_AccessWrite(eRsuActMode, 1);
    status &= dvProAV_AccessWrite(eRsuActMode, 0);

    // wait op busy == 0
    status &= _Rsu_StateWait(eRsuOpBz, 0, 1000, 1);


    // Start Re-Configuration
    status &= _Rsu_StartReconfig(5000);

    return status;
}

int dvProAV_RsuBuffer2Buffer(int length, int updateAddr)
{
    int status = rcSUCCESS;

    // Read-Buffer to Write-Buffer w/ or w/o update start-address
    if (updateAddr == 0)
        status &= dvProAV_AccessWrite(eRsuBufCtl, 2);
    else
    {
        status &= dvProAV_AccessWrite(eRsuBufCtl, 3);
        // to do : add start address
    }

    //status &= dvProAV_AccessWrite(eRsuMoveLenMaxEn, 1);
    status &= dvProAV_AccessWrite(eRsuMoveLenMax, (uint32)length);

    // Active the operation sysytem image
    status &= dvProAV_AccessWrite(eRsuActMode, 2);
    status &= dvProAV_AccessWrite(eRsuActMode, 0);

    // wait buffer busy == 0
    status &= _Rsu_StateWait(eRsuUpBufBz, 0, 1, 1);

    return status;
}

int dvProAV_RsuBufferAccess(uint08 *data, int length, bool write);
int dvProAV_RsuFlashAccess(uint32_t flashAddress, uint08 *data, uint32_t length, bool write)
{
    int status = rcSUCCESS;
    uint32_t lineCount;
    uint32_t moveMode;
    uint32_t timeout = length * 1;
    uint32_t block;

    if (flashAddress % RSU_SRAM_LINE_BYTES != 0)
    {
        DBMSG("The Flash Address start address should be a %d bytes boundly...\n", RSU_SRAM_LINE_BYTES );
        return rcERROR;
    }

    if ((length % (RSU_SRAM_LINE_BYTES << RSU_SRAM_ADDR_SHIFT)) != 0)
    {
        DBMSG("The length should be a %d bytes boundly...\n", (RSU_SRAM_LINE_BYTES << RSU_SRAM_ADDR_SHIFT));
        return rcERROR;
    }

    flashAddress =  flashAddress / RSU_SRAM_LINE_BYTES;
    lineCount = length / RSU_SRAM_LINE_BYTES;

    dvProAV_GenericSfiEnter4byteAddressingMode(&m_sfc);
    if (write)
    {
        dvProAV_GenericSfiAllSectorUnprotect(&m_sfc);
        dvProAV_GenericSfiWriteMemoryCmd(&m_sfc);
    }
    else
    {
        dvProAV_GenericSfiReadMemoryCmd(&m_sfc);
    }

    moveMode = write ? 1:0;

    // Clear RstLen

    status &= dvProAV_AccessWrite(eRsuRorceRestSfls, 1);
    status &= dvProAV_AccessWrite(eRsuRorceRestSfls, 0);

    for (block = 0; block< (lineCount/1024); block++)
    {
        if (write) dvProAV_RsuBufferAccess(data + block *1024*4, 1024*4, true);
        status &= dvProAV_AccessWrite(eRsuFlashOrgAddr, flashAddress + block*1024); // 25 bits by 4bytes ==> 2^27

        status &= dvProAV_AccessWrite(eRsuMoveLenMax, 1024);
        status &= dvProAV_AccessWrite(eRsuFlashCtl, moveMode);

        // Active the operation sysytem image
        status &= dvProAV_AccessWrite(eRsuActMode, 3);
        status &= dvProAV_AccessWrite(eRsuActMode, 0);

        //Wait flash busy == 0
        status &= _Rsu_StateWait(eRsuUpFlBz, 0, timeout, 1);
        if (write == false) dvProAV_RsuBufferAccess(data + block * 1024 * 4, 1024 * 4, false);
    }

    if (lineCount % 1024 != 0)
    {
        if (write) dvProAV_RsuBufferAccess(data + block * 1024 * 4, (lineCount % 1024) * 4, true);

        status &= dvProAV_AccessWrite(eRsuFlashOrgAddr, flashAddress + block * 1024); // 25 bits by 4bytes ==> 2^27
        status &= dvProAV_AccessWrite(eRsuMoveLenMax, lineCount % 1024);
        status &= dvProAV_AccessWrite(eRsuFlashCtl, moveMode);

        // Active the operation sysytem image
        status &= dvProAV_AccessWrite(eRsuActMode, 3);
        status &= dvProAV_AccessWrite(eRsuActMode, 0);

        //Wait flash busy == 0
        status &= _Rsu_StateWait(eRsuUpFlBz, 0, timeout, 1);

        if (write == false) dvProAV_RsuBufferAccess(data + block * 1024, (lineCount % 1024) * 4, false);
    }

    if (write)
    {
        dvProAV_GenericSfiAllSectorProtection(&m_sfc);
    }

    return status;
}

int dvProAV_RsuBufferAccess(uint08 *data, int length, bool write) //SPI read/write buffer
{
    int status = rcSUCCESS;
    int i;

    if (write == false)
    {
        // Buffer Read from Sram
        status &= dvProAV_AccessWrite(eRsuBufCtl, 0);
    }
    else
    {
        // Buffer Write to Sram
        status &= dvProAV_AccessWrite(eRsuBufCtl, 0x01);
    }

    // Set the move length by 32 bits(4 byte)
    if (length % 4 !=0)
    {
        return rcERROR;
    }

    status &= dvProAV_AccessWrite(eRsuMoveLenMax, (uint32)(length / 4));

    // Active the operation sysytem image
    status &= dvProAV_AccessWrite(eRsuActMode, 2);
    status &= dvProAV_AccessWrite(eRsuActMode, 0);


    // wait buffer busy == 1
    status &= _Rsu_StateWait(eRsuUpBufBz, 1, 4000, 1);

    if (write == false)
    {
        for (i = 0; i <(length/256);i++)
        {
            status &= dvProAV_AccessBurstRead(eRsuRdBufData , (uint08 *)data + i * 256, 256, BURST_FIX_ADDR); // Fixed Address if last parameter is false
        }
        if (length%256 != 0)
        {
            status &= dvProAV_AccessBurstRead(eRsuRdBufData , (uint08 *)data + i * 256, length % 256, BURST_FIX_ADDR); // Fixed Address if last parameter is false
        }
    }
    else
    {

        for (i = 0; i <(length/256);i++)
        {
            status &= dvProAV_AccessBurstWrite(eRsuWrBufData , (uint08 *)data + i * 256, 256, BURST_FIX_ADDR); // Fixed Address if last parameter is false
        }
        if (length%256 != 0)
        {
            status &= dvProAV_AccessBurstWrite(eRsuWrBufData , (uint08 *)data + i * 256, length % 256, BURST_FIX_ADDR); // Fixed Address if last parameter is false
        }
    }

    // wait buffer busy == 0 , to check if need it or not
    status &= _Rsu_StateWait(eRsuUpBufBz, 0, 4000, 1);

    return status;
}

#if 0
int dvProAV_RsuFlashRead(uint32 offset, uint32 length, uint08 *data, int reversed)
{
    int status = rcSUCCESS;

    if ((offset %(64*1024)) != 0 || (length %(64*1024)) != 0)
    {
        return rcERROR;
    }

    status &= dvProAV_RsuFlashAccess(offset, data, length, false);
    if (reversed!=0)
    {
        for (uint32 i = 0; i < length; i++)
        {
            uint8_t rev = 0;
            for (int b = 0; b < 8;b++)
            {
                rev <<=1;
                rev |= (data[i]>>b) & 0x01;
            }
            data[i] = rev;
        }
    }
    return status;
}

int dvProAV_RsuFlashWrite(uint32 offset, uint32 length, uint08 *data, int reversed)
{
    int status = rcSUCCESS;
    uint32_t sector;

    if ((offset %(64*1024)) != 0 || (length %(64*1024)) != 0)
    {
        return rcERROR;
    }

    if (reversed)
        status &= dvProAV_AccessWrite(eRsuSpi2BufOrder,1);
    else
        status &= dvProAV_AccessWrite(eRsuSpi2BufOrder,0);

    for (sector = 0 ; sector < length/(64*1024); sector++)
    {
        status &= dvProAV_RsuFlashEraseSector(offset + sector * 64*1024, 64*1024);
        status &= dvProAV_RsuFlashAccess(offset + sector * 64*1024, data, 64*1024, true);
    }
    return status;
}
#endif
