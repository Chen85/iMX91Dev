//#define __TEST_BY_MEM__
#ifndef __TEST_BY_MEM__
#else
#include <mem.h>
#include <stdlib.h> //malloc
#endif

#include "dvProAV_Platform.h"
#include "dvProAV_Access.h"
#include "dvProAV_IndirectAccess.h"
#include "dvProAV_RegUtil32.h"
#include "dvProAV_RegTable.h"

//#define IN_FILE_DEBUG
#ifndef IN_FILE_DEBUG
    #undef DBMSG
    #define DBMSG(...) do{}while(0)
#endif

#define Indirect_BANK_COUNT 3

static uint08 bankId = 0;

#ifndef __TEST_BY_MEM__
typedef struct _IndirectBusCtrl{
    uint32 addr;
    uint32 mask;
    uint32 value;
    uint32 read;
    uint32 write;
    uint32 rdThenWr;
    uint32 rdDone;
    uint32 wrDone;
    uint32 rmwDone;
}IndirectBusCtrl;

static const IndirectBusCtrl avlbank[Indirect_BANK_COUNT] =
{
    {
      eVopIoPllAvlAddr,
      eVopIoPllAvlMask,
      eVopIoPllAvlVal,
      eVopIoPllAvlRd,
      eVopIoPllAvlWr,
      eVopIoPllAvlRmw,
      eVopIoPllAvlRdDone,
      eVopIoPllAvlWrDone,
      eVopIoPllAvlRmwDone
    },
    {
      eVopFPllAvlAddr,
      eVopFPllAvlMask,
      eVopFPllAvlVal,
      eVopFPllAvlRd,
      eVopFPllAvlWr,
      eVopFPllAvlRmw,
      eVopFPllAvlRdDone,
      eVopFPllAvlWrDone,
      eVopFPllAvlRmwDone
    },
    {
      eVopGxbTxAvlAddr,
      eVopGxbTxAvlMask,
      eVopGxbTxAvlVal,
      eVopGxbTxAvlRd,
      eVopGxbTxAvlWr,
      eVopGxbTxAvlRmw,
      eVopGxbTxAvlRdDone,
      eVopGxbTxAvlWrDone,
      eVopGxbTxAvlRmwDone
    }
};

static int _IndirectWrite(uint32 u32Addr, uint16 u16WriteLen, uint32 *u32WriteBuffer)
{
    int status;

    status = rcSUCCESS;

    for (int i=0; i<u16WriteLen; i++)
    {
        uint32 addrEnum = avlbank[bankId].addr;
        uint32 maskEnum = avlbank[bankId].mask;
        uint32 valueEnum = avlbank[bankId].value;
        uint32 writeEnum = avlbank[bankId].write;
        DBMSG("AVL Write %02X %04ulX %08ulX...\n", bankId, addr, writeBuffer[i]);
        status &= dvProAV_AccessWrite(addrEnum, u32Addr);
        status &= dvProAV_AccessWrite(maskEnum, 0xFFFFFFFFL);
        status &= dvProAV_AccessWrite(valueEnum, u32WriteBuffer[i]);
        status &= dvProAV_AccessWrite(writeEnum, 1);
    }
    return status;
}

static int _IndirectRead(uint32 u32Addr, uint16 u16ReadLen, uint32 *u32ReadBuffer)
{
    int status;

    status = rcSUCCESS;
    for (int i=0; i<u16ReadLen; i++)
    {
        status &= dvProAV_AccessWrite(avlbank[bankId].addr, u32Addr);
        status &= dvProAV_AccessWrite(avlbank[bankId].mask, 0xFFFFFFFFL);
        status &= dvProAV_AccessWrite(avlbank[bankId].read, 1);
        status &= dvProAV_AccessRead(avlbank[bankId].value, (u32ReadBuffer+i));
    }

    return status;
}
#else

#define Indirect_REG_COUNT  512
#define Indirect_MEM_SIZE (Indirect_BANK_COUNT * Indirect_REG_COUNT)

static uint32 tempREG[Indirect_MEM_SIZE];

int _IndirectWrite(uint32 addr, uint16 writeLen, uint32 *writeBuffer)
{
    int status;

    status =  rcSUCCESS;

    DBMSG("_Indirect write %02X %04X :", bankId, addr );
    for(int i=0 ; i< writeLen ;i++)
    {
        DBMSG(" %08X", writeBuffer[i]);
    }
    DBMSG("\n");

    if (addr == 0x00)
        bankId = writeBuffer[0];
    int index = 0;
    index += bankId * Indirect_REG_COUNT;
    index += addr;
    memcpy(&tempREG[index], writeBuffer, writeLen*sizeof(uint32));

    return status;
}

int _IndirectRead(uint32 addr, uint16 readLen, uint32 *readBuffer)
{
    int status;

    status =  rcSUCCESS;
    int index = 0;
    index += bankId * Indirect_REG_COUNT;
    index += addr;

    memcpy(readBuffer, &tempREG[index], readLen * sizeof(uint32));

    DBMSG("_Indirect read  %02X %04X :", bankId, addr );
    for(int i=0 ; i< readLen ;i++)
    {
        DBMSG(" %08X", readBuffer[i] );
    }
    DBMSG("\n");

    return status;
}

#endif //__PC_TEST__

static int _IndirectSelectBank(uint08 u8Bank)
{
    if (u8Bank != bankId)
    {
        bankId = u8Bank;
    }

    return rcSUCCESS;
}

static int _IndirectWriteWithBank(uint08 u8Bank, uint32 u32Addr, uint16 u16WriteLen, uint32 *u32WriteBuffer)
{
    int status;
    status = _IndirectSelectBank(u8Bank);

    if (status != rcSUCCESS)
        return status;

    return _IndirectWrite(u32Addr, u16WriteLen, u32WriteBuffer);
}

static int _IndirectReadWithBank(uint08 u8Bank, uint32 u32Addr, uint16 u16ReadLen, uint32 *u32ReadBuffer)
{
    int status;
    status = _IndirectSelectBank(u8Bank);

    if (status != rcSUCCESS)
        return status;

    return _IndirectRead(u32Addr, u16ReadLen, u32ReadBuffer);
}

int dvProAV_IndirectRawDataWrite32bits(uint08 u8Bank, uint32 u32Addr, uint32 u32Data)
{
    return utilReg32RawDataWrite(eReg32DrvArria10, u8Bank, u32Addr, 1, &u32Data);
}

int dvProAV_IndirectRawDataRead32bits(uint08 u8Bank, uint32 u32Addr, uint32 *u32Data)
{
    return utilReg32RawDataRead(eReg32DrvArria10, u8Bank, u32Addr, 1, u32Data);
}

int dvProAV_IndirectRawDataWrite(uint08 u8Bank, uint32 u32Addr, uint16 len, uint32 *u32Data)
{
    return utilReg32RawDataWrite(eReg32DrvArria10, u8Bank, u32Addr, len, u32Data);
}

int dvProAV_IndirectRawDataRead(uint08 u8Bank, uint32 u32Addr, uint16 len, uint32 *u32Data)
{
    return utilReg32RawDataRead(eReg32DrvArria10, u8Bank, u32Addr, len, u32Data);
}

int dvProAV_IndirectRegWriteData(uint16 u16RegEnum, uint32 u32Data)
{
    return utilReg32WriteData(eReg32DrvArria10, u16RegEnum, u32Data);
}

int dvProAV_IndirectRegReadData(uint16 u16RegEnum, uint32 *u32Data)
{
    return utilReg32ReadData(eReg32DrvArria10, u16RegEnum, u32Data);
}

int dvProAV_IndirectRegWrite(uint16 u16RegEnum, uint32 *u32Data)
{
    return utilReg32Write(eReg32DrvArria10, u16RegEnum, u32Data);
}

int dvProAV_IndirectRegRead(uint16 u16RegEnum, uint32 *u32Data)
{
    return utilReg32Read(eReg32DrvArria10, u16RegEnum, u32Data);
}

int dvProAV_IndirectAccessInit(void)
{
    int status;
    DrvReg32Desc drvDesc;

    drvDesc.regTable = dvProAV_IndirectPllRegTableGet();
    drvDesc.readFunc = _IndirectReadWithBank;
    drvDesc.writeFunc = _IndirectWriteWithBank;

    status = utilReg32DriverInit(eReg32DrvArria10, drvDesc);
#ifdef __TEST_BY_MEM__
    memset(tempREG,0, sizeof(tempREG));
#endif


    return status;
}

