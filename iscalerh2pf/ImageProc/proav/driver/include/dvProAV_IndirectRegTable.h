#ifndef DV_PROAV_INDIRECT_REG_TABLE_H
#define DV_PROAV_INDIRECT_REG_TABLE_H

#include "dvProAV_Platform.h"
#include "dvProAV_RegUtil32.h"

typedef enum{
    eIOPLLPARAM_M,        // 0
    eIOPLLPARAM_N,        // 1
    eIOPLLPARAM_C_C0,     // 2
    eIOPLLPARAM_C_C1,     // 3
    eIOPLLPARAM_C_C2,     // 4
    eIOPLLPARAM_C_C3,     // 5
    eIOPLLPARAM_C_C4,     // 6
    eIOPLLPARAM_C_C5,     // 7
    eIOPLLPARAM_C_C6,     // 8
    eIOPLLPARAM_C_C7,     // 9
    eIOPLLPARAM_C_C8,     // 10
    eIOPLLPARAM_P_C0,     // 11
    eIOPLLPARAM_P_C1,     // 12
    eIOPLLPARAM_P_C2,     // 13
    eIOPLLPARAM_P_C3,     // 14
    eIOPLLPARAM_P_C4,     // 15
    eIOPLLPARAM_P_C5,     // 16
    eIOPLLPARAM_P_C6,     // 17
    eIOPLLPARAM_P_C7,     // 18
    eIOPLLPARAM_P_C8,     // 19
    eIOPLLPARAM_P_Cn,     // 20, All C counters
    eIOPLLPARAM_LF,       // 21
    eIOPLLPARAM_CP,       // 22
    eIOPLLPARAM_Recfg,    // 23
    eFPLLPARAM_000,       // 24
    eFPLLPARAM_100,       // 25
    eFPLLPARAM_12B,       // 26
    eFPLLPARAM_12C,       // 27
    eFPLLPARAM_133,       // 28
    eFPLLPARAM_134,       // 29
    eFPLLPARAM_135,       // 30
    eFPLLPARAM_136,       // 31
    eFPLLPARAM_280,       // 32
    eGXBTXPARAM_000,      // 33
    eGXBTXPARAM_100,      // 34
    ePllReg_Numbers
}PLL_REG_ENUM;

typedef struct _IOPLLBWLookup
{
    uint08  M;
    uint32  LowLP;
    uint32  LowCP;
    uint32  MedLP;
    uint32  MedCP;
    uint32  HiLP;
    uint32  HiCP;
} IOPLLBWLookup;

typedef struct _IOPLLRecfgReg
{
    uint08  M;
    uint08  N;
    uint16  CntCx[9];
    uint08  BW;
    uint08  CP;
} IOPLLRecfgReg;

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief       Init Pll Register Table
 * @param[out]  return Pll register table pointer
 * @return
 */
const Reg32Desc *dvProAV_IndirectPllRegTableGet(void);

#ifdef __cplusplus
}
#endif

#endif // DV_PROAV_INDIRECT_REG_TABLE_H
