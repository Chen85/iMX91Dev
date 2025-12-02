#include "dvProAV_RegUtil32.h"

static Reg32Desc descArray[MAX_CONTINUE_REG32_NUMS];
static uint32 curData[MAX_CONTINUE_REG32_NUMS];
static uint32 newData[MAX_CONTINUE_REG32_NUMS];

static DrvReg32Desc drvDesc[eReg32DrvNumber];

static uint08 utilReg32ShiftBitsGet(uint32 mask)
{
    uint08 i;
    for (i = 0; i < 32; i ++)
    {
        if (mask & 0x01)
            break;

        mask = mask >> 1;
    }
    return i;
}

int utilReg32DriverInit(uint08 drvEnum, DrvReg32Desc desc)
{
    if (drvEnum < eReg32DrvNumber)
    {
        drvDesc[drvEnum].regTable = desc.regTable;
        drvDesc[drvEnum].readFunc = desc.readFunc;
        drvDesc[drvEnum].writeFunc = desc.writeFunc;
        return rcSUCCESS;
    }
    return rcERROR;
}

static uint08 utilReg32ByteCountGet(uint08 drvEnum, uint32 regEnum)
{
    uint16 i;
    int    stop;
    uint08 usedBytes;
    const Reg32Desc *regTable = drvDesc[drvEnum].regTable;

    i = 0;
    stop = 0;
    usedBytes = 0;
    while (stop != 1 && regTable != 0 /*Table index is NULL*/)
    {
        if (regTable[i].id == TABLE_END_INDEX)
            break;

        if (regEnum == regTable[i].id)
        {
            usedBytes += 1;
        }

        i++;
    }
    return usedBytes;
}

static int utilReg32DescGet(uint08 drvEnum, uint32 regEnum, uint08 byteIndex, Reg32Desc *desc)
{
    uint16 i;
    int    stop;
    const Reg32Desc *regTable = drvDesc[drvEnum].regTable;

    i = 0;
    stop = 0;
    while (stop != 1 && regTable != 0 /*Table index is NULL*/)
    {
        if (regTable[i].id == TABLE_END_INDEX)
            break;

        if (regEnum == regTable[i].id)
        {
            if (regTable[i].byte == byteIndex)
            {
                desc->id     = regTable[i].id;
                desc->bank   = regTable[i].bank;
                desc->offset = regTable[i].offset;
                desc->mask   = regTable[i].mask;
                desc->byte   = regTable[i].byte;
                desc->init   = regTable[i].init;
                desc->rw     = regTable[i].rw;
                break;
            }
        }
        i++;
    }
    return 1; // rcSUCCESS
}

int utilReg32RawDataRead(uint08 drvEnum, uint08 bank, uint32 addr, uint16 len, uint32 *data)
{
    return (*drvDesc[drvEnum].readFunc)(bank, addr, len, data);
}

int utilReg32RawDataWrite(uint08 drvEnum, uint08 bank, uint32 addr, uint16 len, uint32 *data)
{
    return (*drvDesc[drvEnum].writeFunc)(bank, addr, len, data);
}

int utilReg32Write(uint08 drvEnum, uint32 regEnum, uint32 *data)
{
    uint08 byteCount;    
    uint08 i;

    byteCount = utilReg32ByteCountGet( drvEnum, regEnum);

    if (byteCount > MAX_CONTINUE_REG32_NUMS) // one can read 8'regDesc
        return rcERROR;

    // Read register description and current register data
    for (i=0 ; i < byteCount ; i++)
    {
        uint32 temp;

        if (utilReg32DescGet(drvEnum, regEnum, i, &descArray[i]) == rcERROR)
            return rcERROR;

        if (descArray[i].rw == WO)
        {
            curData[i] = 0;
            continue;
        }

        if ((*drvDesc[drvEnum].readFunc)(descArray[i].bank, descArray[i].offset, 1, &temp) == rcERROR)
            return rcERROR;
        curData[i] = temp;
    }

    for (i=0 ; i < byteCount ; i++)
    {
        uint08 shift = utilReg32ShiftBitsGet(descArray[i].mask);
        uint08 index = descArray[i].byte;

        if (index >= byteCount) // Check index range
            return rcERROR;
        newData[i] = data[index] << shift;
        newData[i] = (newData[i] & descArray[i].mask) | (curData[i] & ~descArray[i].mask);

        if (descArray[i].rw == RO)
            return rcERROR;

        if ((*drvDesc[drvEnum].writeFunc)(descArray[i].bank, descArray[i].offset, 1, &newData[i]) == rcERROR)
            return rcERROR;
    }

    return rcSUCCESS;
}

int utilReg32Read(uint08 drvEnum, uint32 regEnum, uint32 *data)
{
    uint08 byteCount;
    uint08 i;

    byteCount = utilReg32ByteCountGet(drvEnum, regEnum);

    if (byteCount > MAX_CONTINUE_REG32_NUMS) // maximum is 8 bytes
        return rcERROR;

    // Read register description and current register data
    for (i=0 ; i < byteCount ; i++)
    {
        if (utilReg32DescGet(drvEnum, regEnum, i, &descArray[i]) == rcERROR)
            return rcERROR;

        if (descArray[i].rw == WO)
            return rcERROR;

        if ((*drvDesc[drvEnum].readFunc)(descArray[i].bank, descArray[i].offset, 1, &curData[i]) == rcERROR)
            return rcERROR;

    }

    for (i=0 ; i < byteCount ; i++)
    {
        uint08 shift = utilReg32ShiftBitsGet(descArray[i].mask);
        uint08 index = descArray[i].byte;

        if (index >= byteCount) // Check index range
            return rcERROR;
        data[index] = curData[i] & descArray[i].mask;
        data[index] >>= shift;
    }

    return rcSUCCESS;
}

int utilReg32WriteData(uint08 drvEnum, uint32 regEnum, uint32 data)
{
    return utilReg32Write(drvEnum, regEnum, &data);
}

int utilReg32ReadData(uint08 drvEnum, uint32 regEnum, uint32 *data)
{
    int status;

    status = utilReg32Read(drvEnum, regEnum, curData);
    *data = curData[0];
    return status;
}

