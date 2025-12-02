/* 
 * Device Name: Arria10_IOPLL
 * Register Table Version: 17.1
 */

#include "dvProAV_Platform.h"
#include "dvProAV_Access.h"
#include "dvProAV_IndirectRegTable.h"

static const Reg32Desc regArria10DescTable[] = {
//    Register Enumeration,    bank,       Address,  byte,          Mask,    RW,         Init

    // IOPLL
    { eIOPLLPARAM_M        ,   0x00,    0x00000090,     0,    0x0003FFFF,    RW,    0x00000000}, //  0
    { eIOPLLPARAM_N        ,   0x00,    0x000000A0,     0,    0x0003FFFF,    RW,    0x00000000}, //  1
    { eIOPLLPARAM_C_C0     ,   0x00,    0x000000C0,     0,    0x0003FFFF,    RW,    0x00000000}, //  2
    { eIOPLLPARAM_C_C1     ,   0x00,    0x000000C1,     0,    0x0003FFFF,    RW,    0x00000000}, //  3
    { eIOPLLPARAM_C_C2     ,   0x00,    0x000000C2,     0,    0x0003FFFF,    RW,    0x00000000}, //  4
    { eIOPLLPARAM_C_C3     ,   0x00,    0x000000C3,     0,    0x0003FFFF,    RW,    0x00000000}, //  5
    { eIOPLLPARAM_C_C4     ,   0x00,    0x000000C4,     0,    0x0003FFFF,    RW,    0x00000000}, //  6
    { eIOPLLPARAM_C_C5     ,   0x00,    0x000000C5,     0,    0x0003FFFF,    RW,    0x00000000}, //  7
    { eIOPLLPARAM_C_C6     ,   0x00,    0x000000C6,     0,    0x0003FFFF,    RW,    0x00000000}, //  8
    { eIOPLLPARAM_C_C7     ,   0x00,    0x000000C7,     0,    0x0003FFFF,    RW,    0x00000000}, //  9
    { eIOPLLPARAM_C_C8     ,   0x00,    0x000000C8,     0,    0x0003FFFF,    RW,    0x00000000}, // 10
    { eIOPLLPARAM_P_C0     ,   0x00,    0x00000100,     0,    0x0000000F,    RW,    0x00000000}, // 11
    { eIOPLLPARAM_P_C1     ,   0x00,    0x00000101,     0,    0x0000000F,    RW,    0x00000000}, // 12
    { eIOPLLPARAM_P_C2     ,   0x00,    0x00000102,     0,    0x0000000F,    RW,    0x00000000}, // 13
    { eIOPLLPARAM_P_C3     ,   0x00,    0x00000103,     0,    0x0000000F,    RW,    0x00000000}, // 14
    { eIOPLLPARAM_P_C4     ,   0x00,    0x00000104,     0,    0x0000000F,    RW,    0x00000000}, // 15
    { eIOPLLPARAM_P_C5     ,   0x00,    0x00000105,     0,    0x0000000F,    RW,    0x00000000}, // 16
    { eIOPLLPARAM_P_C6     ,   0x00,    0x00000106,     0,    0x0000000F,    RW,    0x00000000}, // 17
    { eIOPLLPARAM_P_C7     ,   0x00,    0x00000107,     0,    0x0000000F,    RW,    0x00000000}, // 18
    { eIOPLLPARAM_P_C8     ,   0x00,    0x00000108,     0,    0x0000000F,    RW,    0x00000000}, // 19
    { eIOPLLPARAM_P_Cn     ,   0x00,    0x0000010F,     0,    0x0000000F,    RW,    0x00000000}, // 20
    { eIOPLLPARAM_LF       ,   0x00,    0x00000040,     0,    0x000003C0,    WO,    0x00000000}, // 21
    { eIOPLLPARAM_CP       ,   0x00,    0x00000020,     0,    0x0000003F,    WO,    0x00000000}, // 22
    { eIOPLLPARAM_Recfg    ,   0x00,    0x00000000,     0,    0xFFFFFFFF,    RW,    0x00000000}, // 23

    // FPLL
    { eFPLLPARAM_000       ,   0x01,    0x00000000,     0,    0x000000FF,    RW,    0x00000000}, // 24
    { eFPLLPARAM_100       ,   0x01,    0x00000100,     0,    0x000000FF,    RW,    0x00000000}, // 25
    { eFPLLPARAM_12B       ,   0x01,    0x0000012B,     0,    0x000000FF,    RW,    0x00000000}, // 26
    { eFPLLPARAM_12C       ,   0x01,    0x0000012C,     0,    0x000000FF,    RW,    0x00000000}, // 27
    { eFPLLPARAM_133       ,   0x01,    0x00000133,     0,    0x0000000C,    RW,    0x00000000}, // 28
    { eFPLLPARAM_134       ,   0x01,    0x00000134,     0,    0x00000070,    RW,    0x00000000}, // 29
    { eFPLLPARAM_135       ,   0x01,    0x00000135,     0,    0x00000007,    RW,    0x00000000}, // 30
    { eFPLLPARAM_136       ,   0x01,    0x00000136,     0,    0x00000004,    RW,    0x00000000}, // 31
    { eFPLLPARAM_280       ,   0x01,    0x00000280,     0,    0x00000001,    RW,    0x00000000}, // 32

    // GXBTX
    { eGXBTXPARAM_000      ,   0x02,    0x00000000,     0,    0x000000FF,    RW,    0x00000000}, // 33
    { eGXBTXPARAM_100      ,   0x02,    0x00000100,     0,    0x00000060,    RW,    0x00000000}, // 34

    { TABLE_END_INDEX      ,   0xFF,    0x000000FF,     0,    0x000000FF,    RW,    0x000000FF}  // End of this table
};

/**
 * @brief       Init Arria10 Register Table
 * @param
 * @return      Arria10 register table pointer
 */
const Reg32Desc *dvProAV_IndirectPllRegTableGet()
{
    return regArria10DescTable;
}
