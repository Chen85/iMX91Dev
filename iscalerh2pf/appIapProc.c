// ===============================================================================
// FILE NAME: appIAPProc.c
// DESCRIPTION:
//
//
// Modification History
// --------------------
// 2014/07/01, Leo Create
// --------------------
// ===============================================================================
#include "appIapProcAPI.h"
#include "utilDbgMsg.h"
#include "utilCRCAPI.h"
#include "halCFUCtrlAPI.h"
#include "utilCounterAPI.h"

// ==============================================================================
// FUNCTION NAME: palIapProc_LPCMCU_upgrade
// DESCRIPTION:
//
//
// Params:
// UINT8 ucIndex:
//
// Returns:
//
//
// Modification History
// --------------------
// 2018/03/01, Larry Create
// --------------------
// ==============================================================================
eRESULT palIapProc_LPCMCU_Upgrade(UINT8 ucIndex, UINT8 *pcUpgradeData, UINT32 dwUpgradeSize)
{
    eRESULT eResult = rcERROR;
    UINT32  uiBufferIdx;
    UINT32  uiAppAddr;
    UINT32  uiAppBinIdx, uiAppBinEnd;
    UINT32  uiCheckSum, uiGetCheckSum, uiFlashCheckSum, uiAllCheckSum;
    UINT32  uiTemp;
    UINT8   ucRetry = 0;
    UINT8   aucData[8] = {0};
    UINT8*  pucExtData = NULL;
    UINT32  ulSize;
    UINT8   ucBIN_File[1024+16] = {0};

    if(ucIndex > eLPCMCU_INDEX_NUMBERS)
    {
        return rcERROR;
    }

    ulSize = ((dwUpgradeSize/LPCMCU_BIN_PAGE_BYTE_SIZE)+1)*LPCMCU_BIN_PAGE_BYTE_SIZE;

    pucExtData = malloc((ulSize * sizeof(UINT8)) + LPCMCU_BIN_PAGE_BYTE_SIZE);
    if(pucExtData == NULL)
    {
        return rcERROR;
    }
    memset(pucExtData, 0, (ulSize + LPCMCU_BIN_PAGE_BYTE_SIZE));
    memcpy(pucExtData, pcUpgradeData, dwUpgradeSize);

    aucData[0] = 1;

    if(rcSUCCESS != halCFUCtrl_Cmd_GoBootloader_Set(ucIndex, aucData))
    {
        free(pucExtData);
        return rcERROR;
    }

    MS_SLEEP(100);
    for(ucRetry = 0; ucRetry < 20; ucRetry++)
    {
        aucData[0] = 0;
        halCFUCtrl_Cmd_GoBootloader_Get(ucIndex, aucData);
        LOG_MSG(db_UPGRADE, "status %d=%d\r\n", ucRetry, aucData[0]);
        if(aucData[0] == 3) // 3: BootCode Running
        {
            break;
        }
        MS_SLEEP(500);
    }

    halCFUCtrl_Cmd_BL_Ver_Get(ucIndex, aucData);

    LOG_MSG(db_UPGRADE, "BootCode Vers:0x%02X.%02X\r\n", aucData[1], aucData[0]);
    MS_SLEEP(5);

    LOG_MSG(db_UPGRADE, "lpc MCU Id %d size 0x%08X\r\n", ucIndex, ulSize);

    //ulSize = ulSize - LPCMCU_BL_SIZE;

    *((UINT32*)&aucData[0]) = LPCMCU_APP_FIRMWARE_START;
    *((UINT32*)&aucData[4]) = ulSize - LPCMCU_BL_SIZE;

    halCFUCtrl_Cmd_App_Info_Set(ucIndex, aucData);

    LOG_MSG(db_UPGRADE, "App size=%d\r\n", *((UINT32*)&aucData[4]));
    MS_SLEEP(5);

    aucData[0] = 1;

    halCFUCtrl_Cmd_IAP_EN_Set(ucIndex, aucData);

    MS_SLEEP(500);
    for(ucRetry = 0; ucRetry < 20; ucRetry++)
    {
        aucData[0] = 0;

        halCFUCtrl_Cmd_IAP_EN_Get(ucIndex, aucData);
        LOG_MSG(db_UPGRADE, "cRetry %d=%d\r\n", ucRetry, aucData[0]);
        if(aucData[0] != 0)
        {
            break;
        }
        MS_SLEEP(1000);
    }

    uiAppBinIdx   = LPCMCU_APP_FIRMWARE_START;
    uiAppBinEnd   = ulSize - 1; //
    uiAllCheckSum = 0;
    uiAppAddr = LPCMCU_APP_FIRMWARE_START;

    while(uiAppBinIdx < uiAppBinEnd)
    {
        eResult = rcERROR;
        //LOG_MSG(db_UPGRADE, "\r\ncAppAddr=0x%X\r\n", uiAppAddr);
        uiCheckSum    = 0;
        uiGetCheckSum = 0;

        for(uiBufferIdx = 0; uiBufferIdx < LPCMCU_BIN_PAGE_BYTE_SIZE; uiBufferIdx++)
        {
            ucBIN_File[uiBufferIdx] = pucExtData[uiAppBinIdx + uiBufferIdx];
            uiCheckSum += ucBIN_File[uiBufferIdx];
        }
        uiAllCheckSum += uiCheckSum;

        // Write to slave

        halCFUCtrl_Cmd_Bin_Address_Set(ucIndex, (UINT8*)&uiAppAddr);
        MS_SLEEP(10);
        halCFUCtrl_Cmd_Bin_Data_Set(ucIndex, ucBIN_File);
        MS_SLEEP(20);
        halCFUCtrl_Cmd_BinCheckSum_Get(ucIndex, (UINT8*)&uiGetCheckSum);
        MS_SLEEP(10);

        //LOG_MSG(db_UPGRADE, "Checksum 0x%X 0x%X\r\n", uiCheckSum, uiGetCheckSum);

        // Verify checksum
        uiTemp = (uiGetCheckSum + uiCheckSum); // must be 0 //A70LV_Larry_0356

        LOG_MSG(db_UPGRADE, "0x%06X Verify checksum 0x%08X+0x%X=0x%08X\r\n", uiAppAddr, uiCheckSum, uiGetCheckSum, uiTemp);

        if(0 == uiTemp)
        {
            eResult = rcSUCCESS;
            uiFlashCheckSum = 0;
            halCFUCtrl_Cmd_BinToFlash_Set(ucIndex, (UINT8*)&uiAppAddr);
            MS_SLEEP(20);
            halCFUCtrl_Cmd_AppCode_Get(ucIndex, (UINT8*)&uiFlashCheckSum);
            MS_SLEEP(5);

            LOG_MSG(db_UPGRADE, "READBACK 0x%08X\r\n", uiFlashCheckSum);
            if(uiGetCheckSum != uiFlashCheckSum)
            {
				LOG_MSG(db_UPGRADE, "CheckSum mismatched  0x%08X\r\n", uiFlashCheckSum);
                eResult = rcERROR;
            }
            else
            {
                uiAppBinIdx += LPCMCU_BIN_PAGE_BYTE_SIZE;
                uiAppAddr += LPCMCU_BIN_PAGE_BYTE_SIZE;
            }
        }

        if(eResult == rcERROR)
        {
            break;
        }
    }

    if(rcSUCCESS == eResult)
    {
        // Set App Version

        memcpy(aucData, &pucExtData[LPCMCU_APP_FIRMWARE_VERSION], 4);

        halCFUCtrl_Cmd_App_Ver_Set(ucIndex, aucData);

        MS_SLEEP(20);

        // End Process
        halCFUCtrl_Cmd_ProgrammingFinish_Set(ucIndex, aucData);
        MS_SLEEP(200);

        uiFlashCheckSum = 0;

        halCFUCtrl_Cmd_ProgrammingFinish_Get(ucIndex, (UINT8*)&uiFlashCheckSum);
        MS_SLEEP(100);

        uiTemp = uiFlashCheckSum + uiAllCheckSum;
        LOG_MSG(db_UPGRADE, "App Checksum 0x%X+0x%X=%d\r\n",
                            uiFlashCheckSum,
                            uiAllCheckSum,
                            uiTemp);

        if(uiTemp != 0)
        {
            LOG_MSG(db_UPGRADE, "Lpc Mcu Upgrade %d Done! Result 0\r\n", ucIndex);
            free(pucExtData);
            return rcERROR;
        }

        halCFUCtrl_Cmd_RunApp_Set(ucIndex, aucData);

        MS_SLEEP(1000);
    }

    LOG_MSG(db_UPGRADE, "Lpc54113 Upgrade %d Done! Result %d\r\n", ucIndex, eResult);
    free(pucExtData);

    return eResult;
}


// ==============================================================================
// FUNCTION NAME: palIapProc_UpgradeExtFlash
// DESCRIPTION:
//
//
// Params:
//
// Returns:
//
//
// Modification History
// --------------------
// 2020/05/28, Larry Create
// --------------------
// ==============================================================================
eRESULT palIapProc_UpgradeExtFlash(UINT8 ucIndex, UINT8 *pcUpgradeData, UINT32 dwUpgradeSize)
{
    eRESULT eResult = rcERROR;
    UINT8   cValue = 1;
    UINT32  dwSectionIndex = 0;
    UINT32  dwCurrentSection = 0;
    UINT32  dwSectionBlock = 0;
    UINT16  wCRC = 0;
    UINT32  wCheckSum = 0;
    UINT32  wTotalCheckSum = 0;
    UINT16  wUpgradeCRC = 0;
    UINT8  *pucExtData = NULL;
    UINT8   cUpdataData[1024 + 4] = {0};
    UINT32  dwAdd = 0;
    UINT32  dwDataAdd = 0;
    UINT32  ulSize = 0;
    UINT16  wRetry = 0;
	static  UINT16 wCRCTmp = 0xFF;


    ulSize = ((dwUpgradeSize/EXT_FLASH_SECTION_SIZE)+1)*EXT_FLASH_SECTION_SIZE;
    dwSectionBlock = ulSize/EXT_FLASH_SECTION_SIZE;

    LOG_MSG(db_UPGRADE, "Block Number %d, ulSize %d\r\n", dwSectionBlock, ulSize);

    if(dwSectionBlock == 0 || ulSize == 0)
    {
        return rcSUCCESS;
    }

    pucExtData = malloc(ulSize);
    if(pucExtData == NULL)
    {
        return rcERROR;
    }

    memset(pucExtData, 0xff, ulSize);
    memcpy(pucExtData, pcUpgradeData, dwUpgradeSize);

    halCFUCtrl_Cmd_ExtFlashEnable_Set(ucIndex, &cValue);

    MS_SLEEP(10);

    //set section index 0
    dwSectionIndex = 0;
    halCFUCtrl_Cmd_SectorSeclect_Set(ucIndex, (UINT8 *)&dwSectionIndex);

    wRetry = 0;
    do
    {
        MS_SLEEP(10);
        halCFUCtrl_Cmd_SectorSeclect_Get(ucIndex, (UINT8*)&dwCurrentSection);
        wRetry++;
    }while((wRetry < 10) && (dwCurrentSection != dwSectionIndex));

    if(wRetry == 10)
    {
        free(pucExtData);
        return rcERROR;
    }

    halCFUCtrl_Cmd_SectorErase_Set(eLPCMCU_INDEX_SYSTEM, &cValue);

    for(dwSectionIndex = 1; dwSectionIndex < dwSectionBlock; dwSectionIndex++)
    {
        dwDataAdd = dwSectionIndex*EXT_FLASH_SECTION_SIZE;

        halCFUCtrl_Cmd_SectorSeclect_Set(ucIndex, (UINT8*)&dwSectionIndex);

        wRetry = 0;
        do
        {
            MS_SLEEP(10);
            halCFUCtrl_Cmd_SectorSeclect_Get(ucIndex, (UINT8*)&dwCurrentSection);
            wRetry++;
        }while((wRetry < 10) && (dwCurrentSection != dwSectionIndex));

        if(wRetry == 10)
        {
            free(pucExtData);
            return rcERROR;
        }

        //wUpgradeCRC = utilCRC16Calc(&pucExtData[dwDataAdd], EXT_FLASH_SECTION_SIZE);
		wUpgradeCRC = utilCRC16Calc_New((UINT8)wCRCTmp, &pucExtData[dwDataAdd], EXT_FLASH_SECTION_SIZE);
		wCRCTmp = wUpgradeCRC & 0xFF;

        //halCFUCtrl_Cmd_SectorCheckSum_Get(ucIndex, (UINT8*)&wCheckSum);

        halCFUCtrl_Cmd_SectorCRC_Get(ucIndex, (UINT8*)&wCRC);

        LOG_MSG(db_UPGRADE, "Index %03d/%d, 0x%04x 0x%04x\r\n", dwSectionIndex, dwSectionBlock, wCRC, wUpgradeCRC);

        if(wUpgradeCRC != wCRC)
        {
            //halCFUCtrl_Cmd_SectorErase_Set(eLPCMCU_INDEX_SYSTEM, &cValue);

            //MS_SLEEP(10);

            dwAdd = 0;

            memcpy(&cUpdataData[0], (pucExtData + dwDataAdd), 1024);
            memcpy(&cUpdataData[1024], (UINT8*)&dwAdd, 4);
            halCFUCtrl_Cmd_BIN_Write_Set(eLPCMCU_INDEX_SYSTEM, cUpdataData);

            dwAdd = dwAdd + 1024;
            dwDataAdd = dwDataAdd + 1024;

            memcpy(&cUpdataData[0], (pucExtData + dwDataAdd), 1024);
            memcpy(&cUpdataData[1024], (UINT8*)&dwAdd, 4);
            halCFUCtrl_Cmd_BIN_Write_Set(eLPCMCU_INDEX_SYSTEM, cUpdataData);

            dwAdd = dwAdd + 1024;
            dwDataAdd = dwDataAdd + 1024;

            memcpy(&cUpdataData[0], (pucExtData + dwDataAdd), 1024);
            memcpy(&cUpdataData[1024], (UINT8*)&dwAdd, 4);
            halCFUCtrl_Cmd_BIN_Write_Set(eLPCMCU_INDEX_SYSTEM, cUpdataData);

            dwAdd = dwAdd + 1024;
            dwDataAdd = dwDataAdd + 1024;

            memcpy(&cUpdataData[0], (pucExtData + dwDataAdd), 1024);
            memcpy(&cUpdataData[1024], (UINT8*)&dwAdd, 4);
            halCFUCtrl_Cmd_BIN_Write_Set(eLPCMCU_INDEX_SYSTEM, cUpdataData);

            MS_SLEEP(10);

            wCRC = 0;
            halCFUCtrl_Cmd_BIN_CRC_Get(eLPCMCU_INDEX_SYSTEM, (UINT8*)&wCRC);

            if(wUpgradeCRC == wCRC)
            {
                cValue = 1;
                halCFUCtrl_Cmd_BIN_Write2Flash_Set(eLPCMCU_INDEX_SYSTEM, &cValue);
                MS_SLEEP(40);
            }
            else
            {
                LOG_MSG(db_UPGRADE, "Index %03d/%d fail, 0x%04x 0x%04x\r\n", dwSectionIndex, dwSectionBlock, wCRC, wUpgradeCRC);
                free(pucExtData);
                return rcERROR;
            }
        }

    }

    dwSectionIndex = 0;
    halCFUCtrl_Cmd_SectorSeclect_Set(ucIndex, (UINT8 *)&dwSectionIndex);

    wRetry = 0;
    do
    {
        MS_SLEEP(10);
        halCFUCtrl_Cmd_SectorSeclect_Get(ucIndex, (UINT8*)&dwCurrentSection);
        wRetry++;
    }while((wRetry < 10) && (dwCurrentSection != dwSectionIndex));

    if(wRetry == 10)
    {
        free(pucExtData);
        return rcERROR;
    }

    dwDataAdd = 0;
    dwAdd = 0;

    //wUpgradeCRC = utilCRC16Calc(&pucExtData[dwDataAdd], EXT_FLASH_SECTION_SIZE);
	wUpgradeCRC = utilCRC16Calc_New((UINT8)wCRCTmp, &pucExtData[dwDataAdd], EXT_FLASH_SECTION_SIZE);
	wCRCTmp = wUpgradeCRC & 0xFF;

    memcpy(&cUpdataData[0], (pucExtData + dwDataAdd), 1024);
    memcpy(&cUpdataData[1024], (UINT8*)&dwAdd, 4);
    halCFUCtrl_Cmd_BIN_Write_Set(eLPCMCU_INDEX_SYSTEM, cUpdataData);

    dwAdd = dwAdd + 1024;
    dwDataAdd = dwDataAdd + 1024;

    memcpy(&cUpdataData[0], (pucExtData + dwDataAdd), 1024);
    memcpy(&cUpdataData[1024], (UINT8*)&dwAdd, 4);
    halCFUCtrl_Cmd_BIN_Write_Set(eLPCMCU_INDEX_SYSTEM, cUpdataData);

    dwAdd = dwAdd + 1024;
    dwDataAdd = dwDataAdd + 1024;

    memcpy(&cUpdataData[0], (pucExtData + dwDataAdd), 1024);
    memcpy(&cUpdataData[1024], (UINT8*)&dwAdd, 4);
    halCFUCtrl_Cmd_BIN_Write_Set(eLPCMCU_INDEX_SYSTEM, cUpdataData);

    dwAdd = dwAdd + 1024;
    dwDataAdd = dwDataAdd + 1024;

    memcpy(&cUpdataData[0], (pucExtData + dwDataAdd), 1024);
    memcpy(&cUpdataData[1024], (UINT8*)&dwAdd, 4);
    halCFUCtrl_Cmd_BIN_Write_Set(eLPCMCU_INDEX_SYSTEM, cUpdataData);

    MS_SLEEP(10);

    wCRC = 0;
    halCFUCtrl_Cmd_BIN_CRC_Get(eLPCMCU_INDEX_SYSTEM, (UINT8*)&wCRC);

    LOG_MSG(db_UPGRADE, "Index 000/%d, 0x%04x 0x%04x\r\n", dwSectionBlock, wCRC, wUpgradeCRC);

    if(wUpgradeCRC == wCRC)
    {
        cValue = 1;
        halCFUCtrl_Cmd_BIN_Write2Flash_Set(eLPCMCU_INDEX_SYSTEM, &cValue);
        MS_SLEEP(40);
        halCFUCtrl_Cmd_ExtUpgradeDone_Set(eLPCMCU_INDEX_SYSTEM, &cValue);
    }
    else
    {
        free(pucExtData);
        return rcERROR;
    }


    free(pucExtData);
    return rcSUCCESS;

}


// ==============================================================================
// FUNCTION NAME: palIapProc_UpgradeInternal
// DESCRIPTION:
//
//
// Params:
//
// Returns:
//
//
// Modification History
// --------------------
// 2020/05/28, Larry Create
// --------------------
// ==============================================================================
eRESULT palIapProc_UpgradeInternal(UINT8 ucIndex, UINT8 *pcUpgradeData, UINT32 dwUpgradeSize)
{
    eRESULT eResult = rcERROR;
    UINT32  uiBufferIdx;
    UINT32  uiAppAddr;
    UINT32  uiAppBinIdx, uiAppBinEnd;
    UINT32  uiCheckSum, uiGetCheckSum, uiFlashCheckSum, uiAllCheckSum;
    UINT32  uiTemp;
    UINT8   ucRetry = 0;
    UINT8   aucData[8] = {0};
    UINT8*  pucExtData = NULL;
    UINT32  ulSize;
    UINT8   ucBIN_File[1024+16] = {0};

    if(ucIndex > eLPCMCU_INDEX_NUMBERS)
    {
        return rcERROR;
    }

    ulSize = ((dwUpgradeSize/LPCMCU_BIN_PAGE_BYTE_SIZE)+1)*LPCMCU_BIN_PAGE_BYTE_SIZE;

    pucExtData = malloc((ulSize * sizeof(UINT8)) + LPCMCU_BIN_PAGE_BYTE_SIZE);
    memset(pucExtData, 0, (ulSize + LPCMCU_BIN_PAGE_BYTE_SIZE));
    memcpy(pucExtData, pcUpgradeData, dwUpgradeSize);

    aucData[0] = 1;

    halCFUCtrl_Cmd_BL_Ver_Get(ucIndex, aucData);

    LOG_MSG(db_UPGRADE, "BootCode Vers:0x%02X.%02X\r\n", aucData[1], aucData[0]);
    MS_SLEEP(5);

    LOG_MSG(db_UPGRADE, "lpc MCU Id %d size 0x%08X\r\n", ucIndex, ulSize);

    //ulSize = ulSize - LPCMCU_BL_SIZE;

    *((UINT32*)&aucData[0]) = LPCMCU_INTERNAL_APP_START;
    *((UINT32*)&aucData[4]) = ulSize - LPCMCU_BL_SIZE;

    halCFUCtrl_Cmd_App_Info_Set(ucIndex, aucData);

    LOG_MSG(db_UPGRADE, "App size=%d\r\n", *((UINT32*)&aucData[4]));
    MS_SLEEP(5);

    aucData[0] = 1;

    halCFUCtrl_Cmd_IAP_EN_Set(ucIndex, aucData);

    MS_SLEEP(500);
    for(ucRetry = 0; ucRetry < 20; ucRetry++)
    {
        aucData[0] = 0;

        halCFUCtrl_Cmd_IAP_EN_Get(ucIndex, aucData);
        LOG_MSG(db_UPGRADE, "cRetry %d=%d\r\n", ucRetry, aucData[0]);
        if(aucData[0] != 0)
        {
            break;
        }
        MS_SLEEP(1000);
    }

    uiAppBinIdx   = LPCMCU_APP_FIRMWARE_START;
    uiAppBinEnd   = ulSize - 1; //
    uiAllCheckSum = 0;
    uiAppAddr = LPCMCU_INTERNAL_APP_START;

    while(uiAppBinIdx < uiAppBinEnd)
    {
        eResult = rcERROR;
        //LOG_MSG(db_UPGRADE, "\r\ncAppAddr=0x%X\r\n", uiAppAddr);
        uiCheckSum    = 0;
        uiGetCheckSum = 0;

        for(uiBufferIdx = 0; uiBufferIdx < LPCMCU_BIN_PAGE_BYTE_SIZE; uiBufferIdx++)
        {
            ucBIN_File[uiBufferIdx] = pucExtData[uiAppBinIdx + uiBufferIdx];
            uiCheckSum += ucBIN_File[uiBufferIdx];
        }
        uiAllCheckSum += uiCheckSum;

        // Write to slave

        halCFUCtrl_Cmd_Bin_Address_Set(ucIndex, (UINT8*)&uiAppAddr);
        MS_SLEEP(10);
        halCFUCtrl_Cmd_Bin_Data_Set(ucIndex, ucBIN_File);
        MS_SLEEP(20);
        halCFUCtrl_Cmd_BinCheckSum_Get(ucIndex, (UINT8*)&uiGetCheckSum);
        MS_SLEEP(10);

        //LOG_MSG(db_UPGRADE, "Checksum 0x%X 0x%X\r\n", uiCheckSum, uiGetCheckSum);

        // Verify checksum
        uiTemp = (uiGetCheckSum + uiCheckSum); // must be 0 //A70LV_Larry_0356

        LOG_MSG(db_UPGRADE, "0x%06X Verify checksum 0x%08X+0x%X=0x%08X\r\n", uiAppAddr, uiCheckSum, uiGetCheckSum, uiTemp);

        if(0 == uiTemp)
        {
            eResult = rcSUCCESS;
            uiFlashCheckSum = 0;
            halCFUCtrl_Cmd_BinToFlash_Set(ucIndex, (UINT8*)&uiAppAddr);
            MS_SLEEP(20);
            halCFUCtrl_Cmd_AppCode_Get(ucIndex, (UINT8*)&uiFlashCheckSum);
            MS_SLEEP(5);

            LOG_MSG(db_UPGRADE, "READBACK 0x%08X\r\n", uiFlashCheckSum);
            if(uiGetCheckSum != uiFlashCheckSum)
            {
                LOG_MSG(db_UPGRADE, "CheckSum mismatched  0x%08X\r\n", uiFlashCheckSum);
                eResult = rcERROR;
            }
            else
            {
                uiAppBinIdx += LPCMCU_BIN_PAGE_BYTE_SIZE;
                uiAppAddr += LPCMCU_BIN_PAGE_BYTE_SIZE;
            }
        }

        if(eResult == rcERROR)
        {
            break;
        }
    }

    if(rcSUCCESS == eResult)
    {
        // Set App Version

        memcpy(aucData, &pucExtData[LPCMCU_APP_FIRMWARE_VERSION], 4);

        halCFUCtrl_Cmd_App_Ver_Set(ucIndex, aucData);

        MS_SLEEP(20);

        // End Process
        halCFUCtrl_Cmd_ProgrammingFinish_Set(ucIndex, aucData);
        MS_SLEEP(200);

        uiFlashCheckSum = 0;

        halCFUCtrl_Cmd_ProgrammingFinish_Get(ucIndex, (UINT8*)&uiFlashCheckSum);
        MS_SLEEP(100);

        uiTemp = uiFlashCheckSum + uiAllCheckSum;
        LOG_MSG(db_UPGRADE, "App Checksum 0x%X+0x%X=%d\r\n",
                            uiFlashCheckSum,
                            uiAllCheckSum,
                            uiTemp);

        if(uiTemp != 0)
        {
            LOG_MSG(db_UPGRADE, "Lpc Mcu Upgrade %d Done! Result 0\r\n", ucIndex);
            free(pucExtData);
            return rcERROR;
        }

        //halCFUCtrl_Cmd_RunApp_Set(ucIndex, aucData);

        MS_SLEEP(100);
    }

    LOG_MSG(db_UPGRADE, "Lpc54113 Upgrade %d Done! Result %d\r\n", ucIndex, eResult);
    free(pucExtData);

    return eResult;
}



