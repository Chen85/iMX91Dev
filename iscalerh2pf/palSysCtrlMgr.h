#ifndef PALSYSCTRLMGR_H
#define PALSYSCTRLMGR_H

#include "Common.h"
#include "CommonDef.h"

eRESULT palSysCtrlMgr_Baudrate_Set(eCM_SERIAL_PORT_BAUD_RATE_ID eBaudrateIdx);
void palSysCtrlMgr_XillinxFPGA_Reset(void); //unused
void palSysCtrlMgr_PowerCheckGet(UINT8 *pcData); //unused
void palSysCtrlMgr_XillinxFPGA_Manual_Mode_Set(UINT8 ucPanel); //unused
void palSysCtrlMgr_XillinxFPGA_Init_Set(UINT8 ucPanel); //unused
eRESULT palSysCtrlMgr_XillinxFPGA_VersionRead(UINT8* ucBLV, UINT8* ucBHV);
BOOL palSysCtrlMgr_XillinxFPGA_NewVersionGet(void);
BOOL palSysCtrlMgr_XillinxFPGA_SW_Reset_Flag_Get(void)     ;
BOOL palSysCtrlMgr_XillinxFPGA_Version_BL_Get(UINT8 *ucData);
BOOL palSysCtrlMgr_XillinxFPGA_Version_BH_Get(UINT8 *ucData);
BOOL palSysCtrlMgr_XillinxFPGA_DRP_Status_Get(UINT8 *ucData);
BOOL palSysCtrlMgr_XillinxFPGA_PhaseUnlock_Get(UINT8 *ucData);
BOOL palSysCtrlMgr_XillinxFPGA_LDVS_Cover_Get(UINT8 *ucData);
void palSysCtrlMgr_PowerOn(void);
void palSysCtrlMgr_PowerKeyReset(void);
void palSysCtrlMgr_CameraEnableInit(void);
void palSysCtrlMgr_CameraEnableSet(UINT8 ucValue);
eRESULT palSysCtrl_ModelID_Get(UINT8* ModelID);
eRESULT palSysCtrlMgr_DMD_AirTightSet(UINT8 ucValue); //HICC2_Steven_0031
eRESULT palSysCtrlMgr_DMD_AirTightGet(UINT8 *ucData); //HICC2_Steven_0032


#endif /* PALSYSCTRLMGR_H */

