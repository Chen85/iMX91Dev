#ifndef UTILCRCAPI_H
#define UTILCRCAPI_H
// ==============================================================================
// FILE NAME: UTILCRCAPI.H
// DESCRIPTION:
//
//
// modification history
// --------------------
// 2014/05/01, Leo Create
// --------------------
// ==============================================================================


#include "Common.h"

UINT16 utilCRC16Calc(UINT8 *paucData, UINT32  ulCount);
UINT16 utilCRC16Calc_New(UINT8 StartVal, UINT8 *paucData, UINT32  ulCount);

#endif /* UTILCRCAPI_H */

