#ifndef PALFWUPGRADEMGR_H_
#define PALFWUPGRADEMGR_H_

#include "Common.h"

eRESULT palFwUpgradeMgr_GoBootloader_Set(eLPCMCU_INDEX ChipIndex, UINT8 *pucData);
eRESULT palFwUpgradeMgr_GoBootloader_Get(eLPCMCU_INDEX ChipIndex, UINT8 *pucData);
eRESULT palFwUpgradeMgr_BL_Ver_Get(eLPCMCU_INDEX ChipIndex, UINT8 *pucData);
eRESULT palFwUpgradeMgr_App_Ver_Set(eLPCMCU_INDEX ChipIndex, UINT8 *pucData);
eRESULT palFwUpgradeMgr_RunApp_Set(eLPCMCU_INDEX ChipIndex, UINT8 *pucData);
eRESULT palFwUpgradeMgr_IAP_EN_Set(eLPCMCU_INDEX ChipIndex, UINT8 *pucData);
eRESULT palFwUpgradeMgr_IAP_EN_Get(eLPCMCU_INDEX ChipIndex, UINT8 *pucData);
eRESULT palFwUpgradeMgr_App_Info_Set(eLPCMCU_INDEX ChipIndex, UINT8 *pucData);
eRESULT palFwUpgradeMgr_Bin_Address_Set(eLPCMCU_INDEX ChipIndex, UINT8 *pucData);
eRESULT palFwUpgradeMgr_Bin_Data_Set(eLPCMCU_INDEX ChipIndex, UINT8 *pucData);
eRESULT palFwUpgradeMgr_ProgrammingFinish_Set(eLPCMCU_INDEX ChipIndex, UINT8 *pucData);
eRESULT palFwUpgradeMgr_ProgrammingFinish_Get(eLPCMCU_INDEX ChipIndex, UINT8 *pucData);
eRESULT palFwUpgradeMgr_BinCheckSum_Get(eLPCMCU_INDEX ChipIndex, UINT8 *pucData);
eRESULT palFwUpgradeMgr_BinToFlash_Set(eLPCMCU_INDEX ChipIndex, UINT8 *pucData);
eRESULT palFwUpgradeMgr_ExtFlashEnable_Set(eLPCMCU_INDEX ChipIndex, UINT8 *pucData);
eRESULT palFwUpgradeMgr_ExtFlashEnable_Get(eLPCMCU_INDEX ChipIndex, UINT8 *pucData);
eRESULT palFwUpgradeMgr_AppCode_Get(eLPCMCU_INDEX ChipIndex, UINT8 *pucData);
eRESULT palFwUpgradeMgr_OpfuEnable_Set(eLPCMCU_INDEX ChipIndex, UINT8 *pucData);
eRESULT palFwUpgradeMgr_OpfuEnable_Get(eLPCMCU_INDEX ChipIndex, UINT8 *pucData);
eRESULT palFwUpgradeMgr_SectorSeclect_Set(eLPCMCU_INDEX ChipIndex, UINT8 *pucData);
eRESULT palFwUpgradeMgr_SectorSeclect_Get(eLPCMCU_INDEX ChipIndex, UINT8 *pucData);
eRESULT palFwUpgradeMgr_SectorCheckSum_Get(eLPCMCU_INDEX ChipIndex, UINT8 *pucData);
eRESULT palFwUpgradeMgr_SectorCRC_Get(eLPCMCU_INDEX ChipIndex, UINT8 *pucData);
eRESULT palFwUpgradeMgr_SectorErase_Set(eLPCMCU_INDEX ChipIndex, UINT8 *pucData);
eRESULT palFwUpgradeMgr_SectorErase_Get(eLPCMCU_INDEX ChipIndex, UINT8 *pucData);
eRESULT palFwUpgradeMgr_SectorSize_Get(eLPCMCU_INDEX ChipIndex, UINT8 *pucData);
eRESULT palFwUpgradeMgr_BIN_Write_Set(eLPCMCU_INDEX ChipIndex, UINT8 *pucData);
eRESULT palFwUpgradeMgr_BIN_CheckSum_Get(eLPCMCU_INDEX ChipIndex, UINT8 *pucData);
eRESULT palFwUpgradeMgr_BIN_CRC_Get(eLPCMCU_INDEX ChipIndex, UINT8 *pucData);
eRESULT palFwUpgradeMgr_BIN_Write2Flash_Set(eLPCMCU_INDEX ChipIndex, UINT8 *pucData);
eRESULT palFwUpgradeMgr_BIN_Write2Flash_Get(eLPCMCU_INDEX ChipIndex, UINT8 *pucData);
eRESULT palFwUpgradeMgr_BIN_UpgradeStart_Set(eLPCMCU_INDEX ChipIndex, UINT8 *pucData);
eRESULT palFwUpgradeMgr_UpgradeStart_Get(eLPCMCU_INDEX ChipIndex, UINT8 *pucData);
eRESULT palFwUpgradeMgr_UpgradeState_Set(eLPCMCU_INDEX ChipIndex, UINT8 *pucData);
eRESULT palFwUpgradeMgr_UpgradeState_Get(eLPCMCU_INDEX ChipIndex, UINT8 *pucData);
eRESULT palFwUpgradeMgr_ExtUpgradeDone_Set(eLPCMCU_INDEX ChipIndex, UINT8 *pucData);
eRESULT palFwUpgradeMgr_UpgradePercentage_Get(eLPCMCU_INDEX ChipIndex, UINT8 *pucData);

#endif /* PALFWUPGRADEMGR_H_ */

