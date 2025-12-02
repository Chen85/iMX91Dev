// ==============================================================================
// FILE NAME: UITLMATH.C
// DESCRIPTION:
//
//
// modification history
// --------------------
// 02/01/2014, Leo Create
// --------------------
// ==============================================================================


#include "Common.h"
#include "utilDbgMsg.h"
#include "utilCommon.h"	//G100_Doulas_0023

UINT16 util_PositiveInterPlacement(UINT32 dwY_Hi, UINT32 dwY_Lo, UINT32 dwX_In, UINT32 dwX_Hi, UINT32 dwX_Lo) //A70LV_Larry_0142
{
    UINT32 dwY_Out;
    INT64  i64Y_Hi, i64Y_Lo, i64X_In, i64X_Hi, i64X_Lo;

    i64Y_Hi = (INT64)dwY_Hi;
    i64Y_Lo = (INT64)dwY_Lo;
    i64X_In = (INT64)dwX_In;
    i64X_Hi = (INT64)dwX_Hi;
    i64X_Lo = (INT64)dwX_Lo;

    if(dwX_Hi == dwX_Lo)
    {   // avoid overflow
        return (UINT16)dwY_Lo;
    }

    //if((dwY_Hi < dwY_Lo) ||
    //   (dwX_Hi < dwX_Lo) ||
    //   (dwX_In < dwX_Lo))
    //{   //avoid native umber
    //    return 0;
    //}

    dwY_Out = ((i64X_In-i64X_Lo)*(i64Y_Hi-i64Y_Lo))/(i64X_Hi-i64X_Lo)+i64Y_Lo;
    if(dwY_Out > DDP_PWM_MAX_VALUE) //overflow //G100_Doulas_0023 Modify
    {
        dwY_Out = DDP_PWM_MAX_VALUE;
    }
    return (UINT16)dwY_Out;
}


UINT32 CalcChecksum(UINT8 *pnData, UINT32 nSize)
{
	UINT32 i = 0;
	UINT32 checksum = 0;

    for (i = 0; i < nSize; i++)
    {
        checksum += *(pnData+i);
    }
	checksum = ~checksum;

	return checksum;
}


