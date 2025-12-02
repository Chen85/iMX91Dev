// ==============================================================================
// FILE NAME: DVFRONTENDDRIVER.H
// DESCRIPTION:
//
//
// modification history
// --------------------
// 11/27/2017  A70LV_Eric.C_0023
// --------------------
// ==============================================================================


#ifndef DVFRONTENDDRIVER_H
#define DVFRONTENDDRIVER_H

#include "Common.h"
#include "dvFrontEndDriver.h"

eRESULT dvFrontEndDriverRegWrite(UINT8 reg, UINT16 size, UINT8 *data);
eRESULT dvFrontEndDriverRegRead(UINT8 reg, UINT16 size, UINT8 *data);
eRESULT dvFrontEndDriverRegWrite_2ByteReg(UINT8 main, UINT8 reg, UINT16 size, UINT8 *data); //G100_Julie_0001
eRESULT dvFrontEndDriverRegRead_2ByteReg(UINT8 main, UINT8 reg, UINT16 size, UINT8 *data);  //G100_Julie_0001


#endif /* DVFRONTENDDRIVER_H */


