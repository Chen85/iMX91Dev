#ifndef APPIAPPROCAPI_H
#define APPIAPPROCAPI_H
// ===============================================================================
// FILE NAME: appIAPProcAPI.h
// DESCRIPTION:
//
//
// Modification History
// --------------------
// 2014/07/01, Leo Create
// --------------------
// ===============================================================================

#include "Common.h"


eRESULT palIapProc_LPCMCU_Upgrade(UINT8 ucIndex, UINT8 *pcUpgradeData, UINT32 dwUpgradeSize);
eRESULT palIapProc_UpgradeExtFlash(UINT8 ucIndex, UINT8 *pcUpgradeData, UINT32 dwUpgradeSize);
eRESULT palIapProc_UpgradeInternal(UINT8 ucIndex, UINT8 *pcUpgradeData, UINT32 dwUpgradeSize);

#endif /* APPIAPPROCAPI_H */

