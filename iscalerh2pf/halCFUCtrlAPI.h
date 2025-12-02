// ===============================================================================
// FILE NAME: halCFUCtrlAPI.h
// DESCRIPTION:
//
//
// Modification History
// --------------------
// 2018/02/26, Larry Create
// --------------------
// ===============================================================================


#ifndef HALCFUCTRLAPI_H_
#define HALCFUCTRLAPI_H_

#include "Common.h"

#define LPCMCU_APP_FIRMWARE_START   (0x8100) // 32k
#define LPCMCU_APP_FIRMWARE_VERSION (0x8000) // 32k
#define LPCMCU_BL_SIZE              (0x8000) // 32k
#define LPCMCU_BIN_PAGE_BYTE_SIZE   (1024)
#define LPCMCU_BIN_PAGE_SIZE        (LPCMCU_BIN_PAGE_BYTE_SIZE/4)

#define EXT_FLASH_SECTION_SIZE      (0x1000)//4096 Byte

#define LPCMCU_INTERNAL_APP_START   (0x48100) // 32k offset 256k
#define LPCMCU_INTERNAL_APP_VERSION (0x48000) // 32k offset 256k
#define LPCMCU_BIN_PAGE_BYTE_SIZE   (1024)
#define LPCMCU_BIN_PAGE_SIZE        (LPCMCU_BIN_PAGE_BYTE_SIZE/4)


eRESULT halCFUCtrl_Cmd_GoBootloader_Set(eLPCMCU_INDEX eChipIndex, UINT8 *pucData);
eRESULT halCFUCtrl_Cmd_GoBootloader_Get(eLPCMCU_INDEX eChipIndex, UINT8 *pucData);
eRESULT halCFUCtrl_Cmd_BL_Ver_Get(eLPCMCU_INDEX eChipIndex, UINT8 *pucData);
eRESULT halCFUCtrl_Cmd_App_Ver_Set(eLPCMCU_INDEX eChipIndex, UINT8 *pucData);
eRESULT halCFUCtrl_Cmd_RunApp_Set(eLPCMCU_INDEX eChipIndex, UINT8 *pucData);
eRESULT halCFUCtrl_Cmd_IAP_EN_Set(eLPCMCU_INDEX eChipIndex, UINT8 *pucData);
eRESULT halCFUCtrl_Cmd_IAP_EN_Get(eLPCMCU_INDEX eChipIndex, UINT8 *pucData);
eRESULT halCFUCtrl_Cmd_App_Info_Set(eLPCMCU_INDEX eChipIndex, UINT8 *pucData);
eRESULT halCFUCtrl_Cmd_Bin_Address_Set(eLPCMCU_INDEX eChipIndex, UINT8 *pucData);
eRESULT halCFUCtrl_Cmd_Bin_Data_Set(eLPCMCU_INDEX eChipIndex, UINT8 *pucData);
eRESULT halCFUCtrl_Cmd_ProgrammingFinish_Set(eLPCMCU_INDEX eChipIndex, UINT8 *pucData);
eRESULT halCFUCtrl_Cmd_ProgrammingFinish_Get(eLPCMCU_INDEX eChipIndex, UINT8 *pucData);
eRESULT halCFUCtrl_Cmd_BinCheckSum_Get(eLPCMCU_INDEX eChipIndex, UINT8 *pucData);
eRESULT halCFUCtrl_Cmd_BinToFlash_Set(eLPCMCU_INDEX eChipIndex, UINT8 *pucData);
eRESULT halCFUCtrl_Cmd_AppCode_Get(eLPCMCU_INDEX eChipIndex, UINT8 *pucData);
eRESULT halCFUCtrl_Cmd_ExtFlashEnable_Set(eLPCMCU_INDEX eChipIndex, UINT8 *pucData);
eRESULT halCFUCtrl_Cmd_ExtFlashEnable_Get(eLPCMCU_INDEX eChipIndex, UINT8 *pucData);
eRESULT halCFUCtrl_Cmd_OpfuEnable_Set(eLPCMCU_INDEX eChipIndex, UINT8 *pucData);
eRESULT halCFUCtrl_Cmd_OpfuEnable_Get(eLPCMCU_INDEX eChipIndex, UINT8 *pucData);
eRESULT halCFUCtrl_Cmd_SectorSeclect_Set(eLPCMCU_INDEX eChipIndex, UINT8 *pucData);
eRESULT halCFUCtrl_Cmd_SectorSeclect_Get(eLPCMCU_INDEX eChipIndex, UINT8 *pucData);
eRESULT halCFUCtrl_Cmd_SectorCheckSum_Get(eLPCMCU_INDEX eChipIndex, UINT8 *pucData);
eRESULT halCFUCtrl_Cmd_SectorCRC_Get(eLPCMCU_INDEX eChipIndex, UINT8 *pucData);
eRESULT halCFUCtrl_Cmd_SectorErase_Set(eLPCMCU_INDEX eChipIndex, UINT8 *pucData);
eRESULT halCFUCtrl_Cmd_SectorErase_Get(eLPCMCU_INDEX eChipIndex, UINT8 *pucData);
eRESULT halCFUCtrl_Cmd_SectorSize_Set(eLPCMCU_INDEX eChipIndex, UINT8 *pucData);
eRESULT halCFUCtrl_Cmd_SectorSize_Get(eLPCMCU_INDEX eChipIndex, UINT8 *pucData);
eRESULT halCFUCtrl_Cmd_BIN_Write_Set(eLPCMCU_INDEX eChipIndex, UINT8 *pucData);
eRESULT halCFUCtrl_Cmd_BIN_CheckSum_Get(eLPCMCU_INDEX eChipIndex, UINT8 *pucData);
eRESULT halCFUCtrl_Cmd_BIN_CRC_Get(eLPCMCU_INDEX eChipIndex, UINT8 *pucData);
eRESULT halCFUCtrl_Cmd_BIN_Write2Flash_Set(eLPCMCU_INDEX eChipIndex, UINT8 *pucData);
eRESULT halCFUCtrl_Cmd_BIN_Write2Flash_Get(eLPCMCU_INDEX eChipIndex, UINT8 *pucData);
eRESULT halCFUCtrl_Cmd_BIN_UpgradeStart_Set(eLPCMCU_INDEX eChipIndex, UINT8 *pucData);
eRESULT halCFUCtrl_Cmd_UpgradeStart_Get(eLPCMCU_INDEX eChipIndex, UINT8 *pucData);
eRESULT halCFUCtrl_Cmd_UpgradeState_Set(eLPCMCU_INDEX eChipIndex, UINT8 *pucData);
eRESULT halCFUCtrl_Cmd_UpgradeState_Get(eLPCMCU_INDEX eChipIndex, UINT8 *pucData);
eRESULT halCFUCtrl_Cmd_ExtUpgradeDone_Set(eLPCMCU_INDEX eChipIndex, UINT8 *pucData);
eRESULT halCFUCtrl_Cmd_UpgradePercentage_Get(eLPCMCU_INDEX eChipIndex, UINT8 *pucData);

#endif /* HALCFUCTRLAPI_H_ */

