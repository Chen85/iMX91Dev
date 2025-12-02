#include "Common.h"
#include "utilDbgMsg.h"
#include "utilCLICmdAPI.h"
#include "utilHostAPI.h"
#include "halCFUCtrlAPI.h"


eRESULT palFwUpgradeMgr_GoBootloader_Set(eLPCMCU_INDEX ChipIndex, UINT8 *pucData)
{
    eRESULT eResult = rcSUCCESS;

    halCFUCtrl_Cmd_GoBootloader_Set(ChipIndex, pucData);

    return eResult;
}

eRESULT palFwUpgradeMgr_GoBootloader_Get(eLPCMCU_INDEX ChipIndex, UINT8 *pucData)
{
    eRESULT eResult = rcSUCCESS;

    halCFUCtrl_Cmd_GoBootloader_Get(ChipIndex, pucData);

    return eResult;
}

eRESULT palFwUpgradeMgr_BL_Ver_Get(eLPCMCU_INDEX ChipIndex, UINT8 *pucData)
{
    eRESULT eResult = rcSUCCESS;

    halCFUCtrl_Cmd_BL_Ver_Get(ChipIndex, pucData);

    return eResult;
}


eRESULT palFwUpgradeMgr_App_Ver_Set(eLPCMCU_INDEX ChipIndex, UINT8 *pucData)
{
    eRESULT eResult = rcSUCCESS;

    halCFUCtrl_Cmd_App_Ver_Set(ChipIndex, pucData);

    return eResult;
}


eRESULT palFwUpgradeMgr_RunApp_Set(eLPCMCU_INDEX ChipIndex, UINT8 *pucData)
{
    eRESULT eResult = rcSUCCESS;

    halCFUCtrl_Cmd_RunApp_Set(ChipIndex, pucData);

    return eResult;
}


eRESULT palFwUpgradeMgr_IAP_EN_Set(eLPCMCU_INDEX ChipIndex, UINT8 *pucData)
{
    eRESULT eResult = rcSUCCESS;

    halCFUCtrl_Cmd_IAP_EN_Set(ChipIndex, pucData);

    return eResult;
}


eRESULT palFwUpgradeMgr_IAP_EN_Get(eLPCMCU_INDEX ChipIndex, UINT8 *pucData)
{
    eRESULT eResult = rcSUCCESS;

    halCFUCtrl_Cmd_IAP_EN_Get(ChipIndex, pucData);

    return eResult;
}


eRESULT palFwUpgradeMgr_App_Info_Set(eLPCMCU_INDEX ChipIndex, UINT8 *pucData)
{
    eRESULT eResult = rcSUCCESS;

    halCFUCtrl_Cmd_App_Info_Set(ChipIndex, pucData);

    return eResult;
}


eRESULT palFwUpgradeMgr_Bin_Address_Set(eLPCMCU_INDEX ChipIndex, UINT8 *pucData)
{
    eRESULT eResult = rcSUCCESS;

    halCFUCtrl_Cmd_Bin_Address_Set(ChipIndex, pucData);

    return eResult;
}


eRESULT palFwUpgradeMgr_Bin_Data_Set(eLPCMCU_INDEX ChipIndex, UINT8 *pucData)
{
    eRESULT eResult = rcSUCCESS;

    halCFUCtrl_Cmd_Bin_Data_Set(ChipIndex, pucData);

    return eResult;
}


eRESULT palFwUpgradeMgr_ProgrammingFinish_Set(eLPCMCU_INDEX ChipIndex, UINT8 *pucData)
{
    eRESULT eResult = rcSUCCESS;

    halCFUCtrl_Cmd_ProgrammingFinish_Set(ChipIndex, pucData);

    return eResult;
}


eRESULT palFwUpgradeMgr_ProgrammingFinish_Get(eLPCMCU_INDEX ChipIndex, UINT8 *pucData)
{
    eRESULT eResult = rcSUCCESS;

    halCFUCtrl_Cmd_ProgrammingFinish_Get(ChipIndex, pucData);

    return eResult;
}


eRESULT palFwUpgradeMgr_BinCheckSum_Get(eLPCMCU_INDEX ChipIndex, UINT8 *pucData)
{
    eRESULT eResult = rcSUCCESS;

    halCFUCtrl_Cmd_BinCheckSum_Get(ChipIndex, pucData);

    return eResult;
}


eRESULT palFwUpgradeMgr_BinToFlash_Set(eLPCMCU_INDEX ChipIndex, UINT8 *pucData)
{
    eRESULT eResult = rcSUCCESS;

    halCFUCtrl_Cmd_BinToFlash_Set(ChipIndex, pucData);

    return eResult;
}


eRESULT palFwUpgradeMgr_ExtFlashEnable_Set(eLPCMCU_INDEX ChipIndex, UINT8 *pucData)
{
    eRESULT eResult = rcSUCCESS;

    halCFUCtrl_Cmd_ExtFlashEnable_Set(ChipIndex, pucData);

    return eResult;
}


eRESULT palFwUpgradeMgr_ExtFlashEnable_Get(eLPCMCU_INDEX ChipIndex, UINT8 *pucData)
{
    eRESULT eResult = rcSUCCESS;

    halCFUCtrl_Cmd_ExtFlashEnable_Get(ChipIndex, pucData);

    return eResult;
}


eRESULT palFwUpgradeMgr_AppCode_Get(eLPCMCU_INDEX ChipIndex, UINT8 *pucData)
{
    eRESULT eResult = rcSUCCESS;

    halCFUCtrl_Cmd_AppCode_Get(ChipIndex, pucData);

    return eResult;
}


eRESULT palFwUpgradeMgr_OpfuEnable_Set(eLPCMCU_INDEX ChipIndex, UINT8 *pucData)
{
    eRESULT eResult = rcSUCCESS;

    halCFUCtrl_Cmd_OpfuEnable_Set(ChipIndex, pucData);

    return eResult;
}


eRESULT palFwUpgradeMgr_OpfuEnable_Get(eLPCMCU_INDEX ChipIndex, UINT8 *pucData)
{
    eRESULT eResult = rcSUCCESS;

    halCFUCtrl_Cmd_OpfuEnable_Get(ChipIndex, pucData);

    return eResult;
}


eRESULT palFwUpgradeMgr_SectorSeclect_Set(eLPCMCU_INDEX ChipIndex, UINT8 *pucData)
{
    eRESULT eResult = rcSUCCESS;

    halCFUCtrl_Cmd_SectorSeclect_Set(ChipIndex, pucData);

    return eResult;
}


eRESULT palFwUpgradeMgr_SectorSeclect_Get(eLPCMCU_INDEX ChipIndex, UINT8 *pucData)
{
    eRESULT eResult = rcSUCCESS;

    halCFUCtrl_Cmd_SectorSeclect_Get(ChipIndex, pucData);

    return eResult;
}


eRESULT palFwUpgradeMgr_SectorCheckSum_Get(eLPCMCU_INDEX ChipIndex, UINT8 *pucData)
{
    eRESULT eResult = rcSUCCESS;

    halCFUCtrl_Cmd_SectorCheckSum_Get(ChipIndex, pucData);

    return eResult;
}


eRESULT palFwUpgradeMgr_SectorCRC_Get(eLPCMCU_INDEX ChipIndex, UINT8 *pucData)
{
    eRESULT eResult = rcSUCCESS;

    halCFUCtrl_Cmd_SectorCRC_Get(ChipIndex, pucData);

    return eResult;
}


eRESULT palFwUpgradeMgr_SectorErase_Set(eLPCMCU_INDEX ChipIndex, UINT8 *pucData)
{
    eRESULT eResult = rcSUCCESS;

    halCFUCtrl_Cmd_SectorErase_Set(ChipIndex, pucData);

    return eResult;
}


eRESULT palFwUpgradeMgr_SectorErase_Get(eLPCMCU_INDEX ChipIndex, UINT8 *pucData)
{
    eRESULT eResult = rcSUCCESS;

    halCFUCtrl_Cmd_SectorErase_Get(ChipIndex, pucData);

    return eResult;
}


eRESULT palFwUpgradeMgr_SectorSize_Get(eLPCMCU_INDEX ChipIndex, UINT8 *pucData)
{
    eRESULT eResult = rcSUCCESS;

    halCFUCtrl_Cmd_SectorSize_Get(ChipIndex, pucData);

    return eResult;
}


eRESULT palFwUpgradeMgr_BIN_Write_Set(eLPCMCU_INDEX ChipIndex, UINT8 *pucData)
{
    eRESULT eResult = rcSUCCESS;

    halCFUCtrl_Cmd_BIN_Write_Set(ChipIndex, pucData);

    return eResult;
}


eRESULT palFwUpgradeMgr_BIN_CheckSum_Get(eLPCMCU_INDEX ChipIndex, UINT8 *pucData)
{
    eRESULT eResult = rcSUCCESS;

    halCFUCtrl_Cmd_BIN_CheckSum_Get(ChipIndex, pucData);

    return eResult;
}


eRESULT palFwUpgradeMgr_BIN_CRC_Get(eLPCMCU_INDEX ChipIndex, UINT8 *pucData)
{
    eRESULT eResult = rcSUCCESS;

    halCFUCtrl_Cmd_BIN_CRC_Get(ChipIndex, pucData);

    return eResult;
}


eRESULT palFwUpgradeMgr_BIN_Write2Flash_Set(eLPCMCU_INDEX ChipIndex, UINT8 *pucData)
{
    eRESULT eResult = rcSUCCESS;

    halCFUCtrl_Cmd_BIN_Write2Flash_Set(ChipIndex, pucData);

    return eResult;
}


eRESULT palFwUpgradeMgr_BIN_Write2Flash_Get(eLPCMCU_INDEX ChipIndex, UINT8 *pucData)
{
    eRESULT eResult = rcSUCCESS;

    halCFUCtrl_Cmd_BIN_Write2Flash_Get(ChipIndex, pucData);

    return eResult;
}


eRESULT palFwUpgradeMgr_BIN_UpgradeStart_Set(eLPCMCU_INDEX ChipIndex, UINT8 *pucData)
{
    eRESULT eResult = rcSUCCESS;

    halCFUCtrl_Cmd_BIN_UpgradeStart_Set(ChipIndex, pucData);

    return eResult;
}


eRESULT palFwUpgradeMgr_UpgradeStart_Get(eLPCMCU_INDEX ChipIndex, UINT8 *pucData)
{
    eRESULT eResult = rcSUCCESS;

    halCFUCtrl_Cmd_UpgradeStart_Get(ChipIndex, pucData);

    return eResult;
}


eRESULT palFwUpgradeMgr_UpgradeState_Set(eLPCMCU_INDEX ChipIndex, UINT8 *pucData)
{
    eRESULT eResult = rcSUCCESS;

    halCFUCtrl_Cmd_UpgradeState_Set(ChipIndex, pucData);

    return eResult;
}


eRESULT palFwUpgradeMgr_UpgradeState_Get(eLPCMCU_INDEX ChipIndex, UINT8 *pucData)
{
    eRESULT eResult = rcSUCCESS;

    halCFUCtrl_Cmd_UpgradeState_Get(ChipIndex, pucData);

    return eResult;
}


eRESULT palFwUpgradeMgr_ExtUpgradeDone_Set(eLPCMCU_INDEX ChipIndex, UINT8 *pucData)
{
    eRESULT eResult = rcSUCCESS;

    halCFUCtrl_Cmd_ExtUpgradeDone_Set(ChipIndex, pucData);

    return eResult;
}


eRESULT palFwUpgradeMgr_UpgradePercentage_Get(eLPCMCU_INDEX ChipIndex, UINT8 *pucData)
{
    eRESULT eResult = rcSUCCESS;

    halCFUCtrl_Cmd_UpgradePercentage_Get(ChipIndex, pucData);

    return eResult;
}


