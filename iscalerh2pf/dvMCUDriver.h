// ==============================================================================
// FILE NAME: DVMCUDRIVER.H
// DESCRIPTION:
//
//
// modification history
// --------------------
// 11/27/2017  A70LV_Eric.C_0023
// --------------------
// ==============================================================================


#ifndef DV_MCUDRIVER_H
#define DV_MCUDRIVER_H

#include "Common.h"

eRESULT dvMCUDriverRegWrite(UINT8 main, UINT8 reg, UINT16 size, UINT8 *data);
eRESULT dvMCUDriverRegRead(UINT8 main, UINT8 reg, UINT16 size, UINT8 *data);
void dvMCUDriverI2CEnableChecksum(UINT8 Enable);


#endif /* DV_MCUDRIVER_H */


