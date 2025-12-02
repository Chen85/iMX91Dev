// ===============================================================================
// FILE NAME: dvA70LV_LDDriver.h
// DESCRIPTION:
//
//
// Modification History
// --------------------
// 2017/10/19, Larry Create
// --------------------
// ===============================================================================


#ifndef DVLDDRIVER_H
#define DVLDDRIVER_H
//#include "CommonDef.h"
#include "Common.h"

typedef enum
{
    eLDBANK_A70LV,

    // ------------------------------------------------------------------------
    eLD_NUMBERS,    // used for enumerated type range
                        // checking (DO NOT REMOVE)
} eLD_ID;

typedef enum
{
    eLD_PWM_R,
    eLD_PWM_G,
    eLD_PWM_B,

    eLD_PWM_NUMBERS,
} eLD_PWM_ITEM;

typedef enum
{
    eLD_BANK_R1,
    eLD_BANK_B1,
    eLD_BANK_B2,

    eLD_BANK_NUMBERS,
} eLD_BANK_ITEM;

eRESULT dvLDDriverRegWrite(UINT8 main, UINT8 reg, UINT16 size, UINT8 *data);
eRESULT dvLDDriverRegRead(UINT8 main, UINT8 reg, UINT16 size, UINT8 *data);

#endif /* DVLDDRIVER_H */


