#ifndef DV_PROAV_REG_UTIL32_H
#define DV_PROAV_REG_UTIL32_H

#include "dvProAV_Platform.h"
#include "dvProAV_RegUtil32Config.h"

/* example for register table:

const RegDesc si57xRegTable[]={
    // Register enumeration,    bank ,  address, byte , mask , r/w, init
    { eDcoHiSpeedDivider      , 0x00 , 0x07    , 0x00 , 0xE0 , RW , 0x00},
    { eDcoClkOutDivider       , 0x00 , 0x07    , 0x00 , 0x1F , RW , 0x00},
    ...
    { TABLE_END_INDEX         , 0xFF , 0xFF    , 0xFF , 0xFF , RW , 0xFF} // End of this table
};

*/

typedef int (*funcDevReg32Write)(uint08 bank, uint32 addr, uint16 writeLen, uint32 *writeBuffer);
typedef int (*funcDevReg32Read) (uint08 bank, uint32 addr, uint16 readLen, uint32 *readBuffer);

typedef struct {
    uint32 id;          // enumeric index for register
    uint08 bank;        // bank index
    uint32 offset;      // register address
    uint08 byte;        // byte index of register
    uint32 mask;        // mask for register data
    uint08 rw;          // 0: write(output), 1: read(input), 2: read or write)
    uint32 init;        // initial value
}Reg32Desc;

typedef struct {
    const Reg32Desc *regTable;
    funcDevReg32Write writeFunc;
    funcDevReg32Write readFunc;
}DrvReg32Desc;

#ifdef __cplusplus
extern "C"
{
#endif

int utilReg32DriverInit(uint08 drvEnum, DrvReg32Desc desc);
int utilReg32RawDataRead(uint08 drvEnum, uint08 bank, uint32 adddr, uint16 len, uint32 *data);
int utilReg32RawDataWrite(uint08 drvEnum, uint08 bank, uint32 addr, uint16 len, uint32 *data);
int utilReg32Write(uint08 drvEnum, uint32 regEnum, uint32 *data);
int utilReg32Read(uint08 drvEnum, uint32 regEnum, uint32 *data);
int utilReg32WriteData(uint08 drvEnum, uint32 regEnum, uint32 data);
int utilReg32ReadData(uint08 drvEnum, uint32 regEnum, uint32 *data);


#ifdef __cplusplus
}
#endif

#endif //DV_PROAV_REG_UTIL32_H
