#include "halRsu_ProAV.h"
#include "dvProAV_Access.h"
#include "dvProAV_RegTable.h"
#include "dvProAV_Scaler.h"
#include "dvProAV_RemoteSystemUpate.h"
#include "dvProAV_SerialFlash.h"
#include "utilDbgMsg.h"
#include "utilIPCAPI.h"
#include "utilCRCAPI.h"

#define FPGA_BIG_ENDIAN
#define EXTERNAL_FLASH_SECTION_SIZE 0x10000

static BOOL halRsu_SemaphoreTake(const UINT8 ucCH, const char *pcSemaphore)
{
    //xSemaphoreGive(m_sHalScalerInfo[ucCH].xSemaphore);
    BOOL bResult = dvProAV_SemaphoreTake(TRUE, pcSemaphore);

    LOG_MSG(db_HAL_SEMAPHORE, "(func:%s) Semaphore %d \r\n", pcSemaphore, bResult);

    if(bResult != TRUE)
    {
        ASSERT(bResult == TRUE);
    }

    return bResult;
}


static BOOL halRsu_SemaphoreGive(const UINT8 ucCH)
{
    //xSemaphoreGive(m_sHalScalerInfo[ucCH].xSemaphore);

    BOOL bResult = dvProAV_SemaphoreTake(FALSE, __FUNCTION__);

    LOG_MSG(db_HAL_SEMAPHORE, "(func:%s)\r\n",__FUNCTION__);

    if(bResult != TRUE)
    {
        ASSERT(bResult == TRUE);
    }

    return bResult;
}

eRESULT halRsu_Init(void)
{
    eRESULT eResult = rcSUCCESS;

    if(TRUE == halRsu_SemaphoreTake(0, __FUNCTION__))
    {
        eResult &= dvProAV_UmbFlashSel(true);
        //eResult &= dvProAV_UmbDataFmtSet(true);
        eResult &= dvProAV_SfiInit();      // dvProAV_RsuInit();
        //eResult &= dvProAV_UmbDataFmtSet(false);
        eResult &= dvProAV_UmbFlashSel(false);
        halRsu_SemaphoreGive(0);
    }

    return eResult;
}

UINT32 halRsu_Read_Flash_CRC(UINT32 ulSflAd, UINT32 ulDataSize)
{
    int status = rcSUCCESS;
    UINT32  uiCount = 0;
    UINT32  ulBLOCK = ulDataSize/EXTERNAL_FLASH_SECTION_SIZE;
    UINT32  ulCount = 0;
    UINT32  ulCRC = 0;
    UINT32  ulReadFlashAdd = ulSflAd;
    UINT32  ulSectorSize = 0;
    UINT8   rev = 0;
    UINT8   *pBuf = NULL;

    if(TRUE == halRsu_SemaphoreTake(0, __FUNCTION__))
    {
        pBuf = (UINT8 *)malloc(EXTERNAL_FLASH_SECTION_SIZE);
        dvProAV_UmbFlashSel(true);
        for(ulCount = 0; ulCount < ulBLOCK; ulCount++)
        {
            memset(pBuf, 0, EXTERNAL_FLASH_SECTION_SIZE);

            status = dvProAV_SfiSpi2Flash(ulReadFlashAdd, pBuf, EXTERNAL_FLASH_SECTION_SIZE, IO_READ);//dvProAV_RsuFlashAccess(ulReadFlashAdd, pBuf, EXTERNAL_FLASH_SECTION_SIZE, IO_READ);

            if(status == rcSUCCESS)
            {
                //LOG_MSG(db_UPGRADE, "%s:%d\n", __FUNCTION__, __LINE__);

#ifndef FPGA_BIG_ENDIAN
//little ENDIAN
                for(UINT32 i = 0; i < EXTERNAL_FLASH_SECTION_SIZE; i++)
                {
                    rev = 0;
                    for (int b = 0; b < 8;b++)
                    {
                        rev <<=1;
                        rev |= (pBuf[i]>>b) & 0x01;
                    }
                    pBuf[i] = rev;

                }
#endif /* FPGA_BIG_ENDIAN */
#if 0
            if(ulCount == 0)
            {
                LOG_MSG(db_DV_PROAV_OSD, "%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n", pBuf[0x00], pBuf[0x01], pBuf[0x02], pBuf[0x03], pBuf[0x04], pBuf[0x05], pBuf[0x06], pBuf[0x07], pBuf[0x08], pBuf[0x09], pBuf[0x0A], pBuf[0x0B], pBuf[0x0C], pBuf[0x0D], pBuf[0x0E], pBuf[0x0F]);
                LOG_MSG(db_DV_PROAV_OSD, "%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n", pBuf[0x10], pBuf[0x11], pBuf[0x12], pBuf[0x13], pBuf[0x14], pBuf[0x15], pBuf[0x16], pBuf[0x17], pBuf[0x18], pBuf[0x19], pBuf[0x1A], pBuf[0x1B], pBuf[0x1C], pBuf[0x1D], pBuf[0x1E], pBuf[0x1F]);
                LOG_MSG(db_DV_PROAV_OSD, "%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n", pBuf[0x20], pBuf[0x21], pBuf[0x22], pBuf[0x23], pBuf[0x24], pBuf[0x25], pBuf[0x26], pBuf[0x27], pBuf[0x28], pBuf[0x29], pBuf[0x2A], pBuf[0x2B], pBuf[0x2C], pBuf[0x2D], pBuf[0x2E], pBuf[0x2F]);
                LOG_MSG(db_DV_PROAV_OSD, "%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n", pBuf[0x30], pBuf[0x31], pBuf[0x32], pBuf[0x33], pBuf[0x34], pBuf[0x35], pBuf[0x36], pBuf[0x37], pBuf[0x38], pBuf[0x39], pBuf[0x3A], pBuf[0x3B], pBuf[0x3C], pBuf[0x3D], pBuf[0x3E], pBuf[0x3F]);
                LOG_MSG(db_DV_PROAV_OSD, "%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n", pBuf[0x40], pBuf[0x41], pBuf[0x42], pBuf[0x43], pBuf[0x44], pBuf[0x45], pBuf[0x46], pBuf[0x47], pBuf[0x48], pBuf[0x49], pBuf[0x4A], pBuf[0x4B], pBuf[0x4C], pBuf[0x4D], pBuf[0x4E], pBuf[0x4F]);
                LOG_MSG(db_DV_PROAV_OSD, "%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n", pBuf[0x50], pBuf[0x51], pBuf[0x52], pBuf[0x53], pBuf[0x54], pBuf[0x55], pBuf[0x56], pBuf[0x57], pBuf[0x58], pBuf[0x59], pBuf[0x5A], pBuf[0x5B], pBuf[0x5C], pBuf[0x5D], pBuf[0x5E], pBuf[0x5F]);
                LOG_MSG(db_DV_PROAV_OSD, "%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n", pBuf[0x60], pBuf[0x61], pBuf[0x62], pBuf[0x63], pBuf[0x64], pBuf[0x65], pBuf[0x66], pBuf[0x67], pBuf[0x68], pBuf[0x69], pBuf[0x6A], pBuf[0x6B], pBuf[0x6C], pBuf[0x6D], pBuf[0x6E], pBuf[0x6F]);
                LOG_MSG(db_DV_PROAV_OSD, "%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n", pBuf[0x70], pBuf[0x71], pBuf[0x72], pBuf[0x73], pBuf[0x74], pBuf[0x75], pBuf[0x76], pBuf[0x77], pBuf[0x78], pBuf[0x79], pBuf[0x7A], pBuf[0x7B], pBuf[0x7C], pBuf[0x7D], pBuf[0x7E], pBuf[0x7F]);
                LOG_MSG(db_DV_PROAV_OSD, "%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n", pBuf[0x80], pBuf[0x81], pBuf[0x82], pBuf[0x83], pBuf[0x84], pBuf[0x85], pBuf[0x86], pBuf[0x87], pBuf[0x88], pBuf[0x89], pBuf[0x8A], pBuf[0x8B], pBuf[0x8C], pBuf[0x8D], pBuf[0x8E], pBuf[0x8F]);
                LOG_MSG(db_DV_PROAV_OSD, "%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n", pBuf[0x90], pBuf[0x91], pBuf[0x92], pBuf[0x93], pBuf[0x94], pBuf[0x95], pBuf[0x96], pBuf[0x97], pBuf[0x98], pBuf[0x99], pBuf[0x9A], pBuf[0x9B], pBuf[0x9C], pBuf[0x9D], pBuf[0x9E], pBuf[0x9F]);
                LOG_MSG(db_DV_PROAV_OSD, "%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n", pBuf[0xA0], pBuf[0xA1], pBuf[0xA2], pBuf[0xA3], pBuf[0xA4], pBuf[0xA5], pBuf[0xA6], pBuf[0xA7], pBuf[0xA8], pBuf[0xA9], pBuf[0xAA], pBuf[0xAB], pBuf[0xAC], pBuf[0xAD], pBuf[0xAE], pBuf[0xAF]);
                LOG_MSG(db_DV_PROAV_OSD, "%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n", pBuf[0xB0], pBuf[0xB1], pBuf[0xB2], pBuf[0xB3], pBuf[0xB4], pBuf[0xB5], pBuf[0xB6], pBuf[0xB7], pBuf[0xB8], pBuf[0xB9], pBuf[0xBA], pBuf[0xBB], pBuf[0xBC], pBuf[0xBD], pBuf[0xBE], pBuf[0xBF]);
                LOG_MSG(db_DV_PROAV_OSD, "%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n", pBuf[0xC0], pBuf[0xC1], pBuf[0xC2], pBuf[0xC3], pBuf[0xC4], pBuf[0xC5], pBuf[0xC6], pBuf[0xC7], pBuf[0xC8], pBuf[0xC9], pBuf[0xCA], pBuf[0xCB], pBuf[0xCC], pBuf[0xCD], pBuf[0xCE], pBuf[0xCF]);
                LOG_MSG(db_DV_PROAV_OSD, "%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n", pBuf[0xD0], pBuf[0xD1], pBuf[0xD2], pBuf[0xD3], pBuf[0xD4], pBuf[0xD5], pBuf[0xD6], pBuf[0xD7], pBuf[0xD8], pBuf[0xD9], pBuf[0xDA], pBuf[0xDB], pBuf[0xDC], pBuf[0xDD], pBuf[0xDE], pBuf[0xDF]);
                LOG_MSG(db_DV_PROAV_OSD, "%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n", pBuf[0xE0], pBuf[0xE1], pBuf[0xE2], pBuf[0xE3], pBuf[0xE4], pBuf[0xE5], pBuf[0xE6], pBuf[0xE7], pBuf[0xE8], pBuf[0xE9], pBuf[0xEA], pBuf[0xEB], pBuf[0xEC], pBuf[0xED], pBuf[0xEE], pBuf[0xEF]);
                LOG_MSG(db_DV_PROAV_OSD, "%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n", pBuf[0xF0], pBuf[0xF1], pBuf[0xF2], pBuf[0xF3], pBuf[0xF4], pBuf[0xF5], pBuf[0xF6], pBuf[0xF7], pBuf[0xF8], pBuf[0xF9], pBuf[0xFA], pBuf[0xFB], pBuf[0xFC], pBuf[0xFD], pBuf[0xFE], pBuf[0xFF]);
            }
#endif /* 0 */

                ulCRC = utilCRC16Calc(&pBuf[0], EXTERNAL_FLASH_SECTION_SIZE);
            }
            else
            {
                ASSERT_ALWAYS();
            }
        }


        free(pBuf);
		dvProAV_UmbFlashSel(false);
        halRsu_SemaphoreGive(0);
    }

    return ulCRC;
}



eRESULT halRsu_Write_Flash(uint8* pcExtAddr, UINT32 ulFlash_Addr, UINT32 ulSize)
{
    int status = rcSUCCESS;

    uint8 *pdwExtFlash = pcExtAddr;
    UINT32  ulBLOCK = ulSize/EXTERNAL_FLASH_SECTION_SIZE;
    UINT32  ulCount = 0;
    uint8 *pBuf = pdwExtFlash;

    //LOG_MSG(db_UPGRADE, "(%s) ulFlash_Addr %08x,  ulSize %08x,", __FUNCTION__, ulFlash_Addr, ulSize);
    //LOG_MSG(db_UPGRADE, "%s:%d\n", __FUNCTION__, __LINE__);

    if(TRUE == halRsu_SemaphoreTake(0, __FUNCTION__))
    {
        dvProAV_UmbFlashSel(true);
        for(ulCount = 0; ulCount < ulBLOCK; ulCount++)
        {

#if 0
            if(ulCount == 0)
            {
                LOG_MSG(db_DV_PROAV_OSD, "%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n", pBuf[0x00], pBuf[0x01], pBuf[0x02], pBuf[0x03], pBuf[0x04], pBuf[0x05], pBuf[0x06], pBuf[0x07], pBuf[0x08], pBuf[0x09], pBuf[0x0A], pBuf[0x0B], pBuf[0x0C], pBuf[0x0D], pBuf[0x0E], pBuf[0x0F]);
                LOG_MSG(db_DV_PROAV_OSD, "%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n", pBuf[0x10], pBuf[0x11], pBuf[0x12], pBuf[0x13], pBuf[0x14], pBuf[0x15], pBuf[0x16], pBuf[0x17], pBuf[0x18], pBuf[0x19], pBuf[0x1A], pBuf[0x1B], pBuf[0x1C], pBuf[0x1D], pBuf[0x1E], pBuf[0x1F]);
                LOG_MSG(db_DV_PROAV_OSD, "%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n", pBuf[0x20], pBuf[0x21], pBuf[0x22], pBuf[0x23], pBuf[0x24], pBuf[0x25], pBuf[0x26], pBuf[0x27], pBuf[0x28], pBuf[0x29], pBuf[0x2A], pBuf[0x2B], pBuf[0x2C], pBuf[0x2D], pBuf[0x2E], pBuf[0x2F]);
                LOG_MSG(db_DV_PROAV_OSD, "%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n", pBuf[0x30], pBuf[0x31], pBuf[0x32], pBuf[0x33], pBuf[0x34], pBuf[0x35], pBuf[0x36], pBuf[0x37], pBuf[0x38], pBuf[0x39], pBuf[0x3A], pBuf[0x3B], pBuf[0x3C], pBuf[0x3D], pBuf[0x3E], pBuf[0x3F]);
                LOG_MSG(db_DV_PROAV_OSD, "%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n", pBuf[0x40], pBuf[0x41], pBuf[0x42], pBuf[0x43], pBuf[0x44], pBuf[0x45], pBuf[0x46], pBuf[0x47], pBuf[0x48], pBuf[0x49], pBuf[0x4A], pBuf[0x4B], pBuf[0x4C], pBuf[0x4D], pBuf[0x4E], pBuf[0x4F]);
                LOG_MSG(db_DV_PROAV_OSD, "%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n", pBuf[0x50], pBuf[0x51], pBuf[0x52], pBuf[0x53], pBuf[0x54], pBuf[0x55], pBuf[0x56], pBuf[0x57], pBuf[0x58], pBuf[0x59], pBuf[0x5A], pBuf[0x5B], pBuf[0x5C], pBuf[0x5D], pBuf[0x5E], pBuf[0x5F]);
                LOG_MSG(db_DV_PROAV_OSD, "%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n", pBuf[0x60], pBuf[0x61], pBuf[0x62], pBuf[0x63], pBuf[0x64], pBuf[0x65], pBuf[0x66], pBuf[0x67], pBuf[0x68], pBuf[0x69], pBuf[0x6A], pBuf[0x6B], pBuf[0x6C], pBuf[0x6D], pBuf[0x6E], pBuf[0x6F]);
                LOG_MSG(db_DV_PROAV_OSD, "%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n", pBuf[0x70], pBuf[0x71], pBuf[0x72], pBuf[0x73], pBuf[0x74], pBuf[0x75], pBuf[0x76], pBuf[0x77], pBuf[0x78], pBuf[0x79], pBuf[0x7A], pBuf[0x7B], pBuf[0x7C], pBuf[0x7D], pBuf[0x7E], pBuf[0x7F]);
                LOG_MSG(db_DV_PROAV_OSD, "%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n", pBuf[0x80], pBuf[0x81], pBuf[0x82], pBuf[0x83], pBuf[0x84], pBuf[0x85], pBuf[0x86], pBuf[0x87], pBuf[0x88], pBuf[0x89], pBuf[0x8A], pBuf[0x8B], pBuf[0x8C], pBuf[0x8D], pBuf[0x8E], pBuf[0x8F]);
                LOG_MSG(db_DV_PROAV_OSD, "%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n", pBuf[0x90], pBuf[0x91], pBuf[0x92], pBuf[0x93], pBuf[0x94], pBuf[0x95], pBuf[0x96], pBuf[0x97], pBuf[0x98], pBuf[0x99], pBuf[0x9A], pBuf[0x9B], pBuf[0x9C], pBuf[0x9D], pBuf[0x9E], pBuf[0x9F]);
                LOG_MSG(db_DV_PROAV_OSD, "%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n", pBuf[0xA0], pBuf[0xA1], pBuf[0xA2], pBuf[0xA3], pBuf[0xA4], pBuf[0xA5], pBuf[0xA6], pBuf[0xA7], pBuf[0xA8], pBuf[0xA9], pBuf[0xAA], pBuf[0xAB], pBuf[0xAC], pBuf[0xAD], pBuf[0xAE], pBuf[0xAF]);
                LOG_MSG(db_DV_PROAV_OSD, "%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n", pBuf[0xB0], pBuf[0xB1], pBuf[0xB2], pBuf[0xB3], pBuf[0xB4], pBuf[0xB5], pBuf[0xB6], pBuf[0xB7], pBuf[0xB8], pBuf[0xB9], pBuf[0xBA], pBuf[0xBB], pBuf[0xBC], pBuf[0xBD], pBuf[0xBE], pBuf[0xBF]);
                LOG_MSG(db_DV_PROAV_OSD, "%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n", pBuf[0xC0], pBuf[0xC1], pBuf[0xC2], pBuf[0xC3], pBuf[0xC4], pBuf[0xC5], pBuf[0xC6], pBuf[0xC7], pBuf[0xC8], pBuf[0xC9], pBuf[0xCA], pBuf[0xCB], pBuf[0xCC], pBuf[0xCD], pBuf[0xCE], pBuf[0xCF]);
                LOG_MSG(db_DV_PROAV_OSD, "%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n", pBuf[0xD0], pBuf[0xD1], pBuf[0xD2], pBuf[0xD3], pBuf[0xD4], pBuf[0xD5], pBuf[0xD6], pBuf[0xD7], pBuf[0xD8], pBuf[0xD9], pBuf[0xDA], pBuf[0xDB], pBuf[0xDC], pBuf[0xDD], pBuf[0xDE], pBuf[0xDF]);
                LOG_MSG(db_DV_PROAV_OSD, "%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n", pBuf[0xE0], pBuf[0xE1], pBuf[0xE2], pBuf[0xE3], pBuf[0xE4], pBuf[0xE5], pBuf[0xE6], pBuf[0xE7], pBuf[0xE8], pBuf[0xE9], pBuf[0xEA], pBuf[0xEB], pBuf[0xEC], pBuf[0xED], pBuf[0xEE], pBuf[0xEF]);
                LOG_MSG(db_DV_PROAV_OSD, "%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n", pBuf[0xF0], pBuf[0xF1], pBuf[0xF2], pBuf[0xF3], pBuf[0xF4], pBuf[0xF5], pBuf[0xF6], pBuf[0xF7], pBuf[0xF8], pBuf[0xF9], pBuf[0xFA], pBuf[0xFB], pBuf[0xFC], pBuf[0xFD], pBuf[0xFE], pBuf[0xFF]);
            }
#endif /* 0 */

            status &= dvProAV_SfiSpi2Flash(ulFlash_Addr, pdwExtFlash, EXTERNAL_FLASH_SECTION_SIZE, IO_WRITE);//dvProAV_RsuFlashAccess(ulFlash_Addr, pdwExtFlash, EXTERNAL_FLASH_SECTION_SIZE, IO_WRITE);

            ulFlash_Addr += EXTERNAL_FLASH_SECTION_SIZE;
            pdwExtFlash += EXTERNAL_FLASH_SECTION_SIZE;
        }
        dvProAV_UmbFlashSel(false);
        halRsu_SemaphoreGive(0);
    }

    return status;
}

eRESULT halRsu_Flash_Erase(UINT32 ulFlash_Addr, UINT32 ulSize)
{
    int status = rcSUCCESS;

    if(TRUE == halRsu_SemaphoreTake(0, __FUNCTION__))
    {
        dvProAV_UmbFlashSel(true);
        status = dvProAV_SfiEraseSector(ulFlash_Addr, ulSize);//dvProAV_RsuFlashEraseSector(ulFlash_Addr, ulSize);
        dvProAV_UmbFlashSel(false);
        halRsu_SemaphoreGive(0);
    }

    return status;
}


UINT8 halRsu_Upgrade_Partial(char *filename, const UINT32 ulFlash_Addr, UINT8 bFroce) //A70LV_Larry_0134
{
    UINT8  cResult = rcSUCCESS;
    UINT32 ulBlock = 0;
    UINT32 ulStartBlock = 0;
    UINT32 ulCount = 0;
    UINT32 ulCRC = 0;
    UINT32 ulFirsrCRC = 0;
    UINT32 ulExtCRC = 0;
    UINT32 ulSectorSize = 0; //A70LV_Larry_0158
    FILE *pFile = NULL;
    UINT32 ulFileSize;
    char *cbuf = NULL;
    UINT32 ulUpgradeAdd = ulFlash_Addr;
    UINT32 ulAddr = 0;
    UINT32 ulSize = 0;
    UINT8  ucRetry = 0;
    UINT16 uiPercentage = 20;
    UINT32 uldiv = 1;
    UINT8 ucRetryStep = 0;

    pFile = fopen(filename, "rb");

    if(pFile == NULL) //A70LV_Larry_0001
    {
        printf("%s not found\n", filename);
        ASSERT_ALWAYS();
        return 0;
    }

    halRsu_Init();

    fseek(pFile, 0, SEEK_END);
    ulSize = ftell(pFile);

    rewind(pFile);

    if((ulSize % EXTERNAL_FLASH_SECTION_SIZE == 0))
    {
        ulBlock = ulSize/EXTERNAL_FLASH_SECTION_SIZE;
    }
    else
    {
        ulBlock = (ulSize/EXTERNAL_FLASH_SECTION_SIZE) + 1;
    }

    ulFileSize = ulBlock*EXTERNAL_FLASH_SECTION_SIZE;

    cbuf = (uint8 *)malloc(ulFileSize);

    memset(cbuf, 0xFF, ulFileSize);

    fread(cbuf, 1, ulSize, pFile);

    fclose(pFile);

    /*if(TRUE == halRsu_SemaphoreTake(0, __FUNCTION__))
    {
        //dvProAV_UmbDataFmtSet(true);    //dvProAV_AccessWrite(eRsuSpi2BufOrder,1);
        halRsu_SemaphoreGive(0);
    }*/


    ulUpgradeAdd = ulFlash_Addr + 0x10000;
    ulAddr = FILE_START_ADDRESS + 0x10000;
    ulStartBlock = ulAddr/EXTERNAL_FLASH_SECTION_SIZE; //先燒64k後的位置，最後再回燒起始的64K

    LOG_MSG(db_UPGRADE, "(%s:%d)ulStartBlock %d, ulBlock %d\n", __FUNCTION__, __LINE__, ulStartBlock, ulBlock);

    uldiv = (ulBlock - ulStartBlock) / 60;

    for(ulCount = ulStartBlock; ulCount < ulBlock; ulCount++)
    {
        ulCRC = 0;
        ulFirsrCRC = 0;
        ulSectorSize = 0; //A70LV_Larry_0158
        ulExtCRC = 0;

        //LOG_MSG(db_UPGRADE, "%s:%d\n", __FUNCTION__, __LINE__);

        if(ulCount%uldiv == 0)
        {
            uiPercentage = uiPercentage + 1;
            utilIpc_SendData(eIPC_SEND_DATA_UPDATE_INFO_ITEM, eDI_UPGRADE_PERCENTAGE, (UINT8*)&uiPercentage, eEXEC_CODE_PASS, 2);
        }

        ulExtCRC = utilCRC16Calc(&cbuf[ulAddr], EXTERNAL_FLASH_SECTION_SIZE);

        ulAddr += EXTERNAL_FLASH_SECTION_SIZE;

        ulFirsrCRC = halRsu_Read_Flash_CRC(ulUpgradeAdd, EXTERNAL_FLASH_SECTION_SIZE);

        ucRetry = 0;
        cResult = rcSUCCESS;

        do
        {
            if((ulFirsrCRC != ulExtCRC) || bFroce)
            {
                MS_SLEEP(10);

                if(rcSUCCESS == halRsu_Flash_Erase(ulUpgradeAdd, EXTERNAL_FLASH_SECTION_SIZE))
                {
                    halRsu_Write_Flash(&cbuf[ulCount*EXTERNAL_FLASH_SECTION_SIZE], ulUpgradeAdd, EXTERNAL_FLASH_SECTION_SIZE);
                    MS_SLEEP(10);

                    ulCRC = halRsu_Read_Flash_CRC(ulUpgradeAdd, EXTERNAL_FLASH_SECTION_SIZE);
                    MS_SLEEP(10);

                    if(ulExtCRC != ulCRC)
                    {
                        cResult = rcERROR;
                        LOG_MSG(db_UPGRADE, "flash Copy fail [0x%04x][0x%04x][0x%04x]\n", ulExtCRC, ulCRC, ulFirsrCRC);
                    }
                    else
                    {
                        cResult = rcSUCCESS;
                        LOG_MSG(db_UPGRADE, "flash Copy pass [0x%04x][0x%04x][0x%04x]", ulExtCRC, ulCRC, ulFirsrCRC);
                    }
                }
                else
                {
                    cResult = rcERROR;
                    LOG_MSG(db_UPGRADE, "flash erase fail\n");
                }
            }
            ucRetry++;
            if(ucRetry == 5)
            {
                ucRetryStep++;
                switch(ucRetryStep)
                {
                    case 1: // read clock delay 1T
                        ucRetry = 0;
                        dvProAV_RsuFlashReadDlySet(1);
                        LOG_MSG(db_ALWAYS, "Retry setp : RSU read clock delay 1T\n");
                        break;
                    case 2: // read clock delay 2T
                        ucRetry = 0;
                        dvProAV_RsuFlashReadDlySet(2);
                        LOG_MSG(db_ALWAYS, "Retry setp  : RSU read clock delay 2T\n");
                        break;
                    case 3: // clock rate 133M/6  & read clock delay 0T
                        ucRetry = 0;
                        dvProAV_RsuFlashReadDlySet(0);
                        dvProAV_RsuFlashClkRateSet(3);
                        LOG_MSG(db_ALWAYS, "Retry setp  : RSU clock rate 133M/6  & read clock delay 0T\n");
                        break;
                    case 4: // clock rate 133M/6 & read clock delay 1T
                        ucRetry = 0;
                        dvProAV_RsuFlashReadDlySet(1);
                        LOG_MSG(db_ALWAYS, "Retry setp  : RSU clock rate 133M/6  & read clock delay 1T\n");
                        break;
                    case 5: // clock rate 133M/6 & read clock delay 2T
                        ucRetry = 0;
                        dvProAV_RsuFlashReadDlySet(2);
                        LOG_MSG(db_ALWAYS, "Retry setp  : RSU clock rate 133M/6  & read clock delay 2T\n");
                        break;
                    case 6: // flash r/w fail, set to default value
                        //ucRetry = 5;
                        dvProAV_RsuFlashReadDlySet(0);
                        dvProAV_RsuFlashClkRateSet(2);
                        LOG_MSG(db_ALWAYS, "Retry setp  : RSU flash r/w fail, set to default value\n");
                        break;
                }
            }
        }
        while((ucRetry < 5) && (cResult != rcSUCCESS));
        //MS_SLEEP(10);

        LOG_MSG(db_UPGRADE, "Copy Address 0x%08x %03d/%03d Retry %d\n", ulUpgradeAdd, ulCount, ulBlock, ucRetry);

        ulUpgradeAdd = ulUpgradeAdd + EXTERNAL_FLASH_SECTION_SIZE;
        //ulAddr = ulAddr + EXTERNAL_FLASH_SECTION_SIZE;
    }


    if(cResult == rcSUCCESS)
    {
        ulUpgradeAdd = ulFlash_Addr;
        ulAddr = FILE_START_ADDRESS;
        ulSectorSize = 0;
        ulExtCRC = 0;
        ucRetryStep = 0;

        ulFirsrCRC = halRsu_Read_Flash_CRC(ulUpgradeAdd, EXTERNAL_FLASH_SECTION_SIZE);

        ulExtCRC = utilCRC16Calc(&cbuf[ulAddr], EXTERNAL_FLASH_SECTION_SIZE);

        ulAddr += EXTERNAL_FLASH_SECTION_SIZE;


        ucRetry = 0;
        cResult = rcSUCCESS;

        do
        {
            if((ulFirsrCRC != ulExtCRC) || bFroce)
            {
                MS_SLEEP(10);

                if(rcSUCCESS == halRsu_Flash_Erase(ulUpgradeAdd, EXTERNAL_FLASH_SECTION_SIZE))
                {
                    halRsu_Write_Flash(&cbuf[FILE_START_ADDRESS], ulUpgradeAdd, EXTERNAL_FLASH_SECTION_SIZE);

                    MS_SLEEP(5);

                    ulCRC = halRsu_Read_Flash_CRC(ulUpgradeAdd, EXTERNAL_FLASH_SECTION_SIZE);
                    MS_SLEEP(10);

                    if(ulExtCRC != ulCRC)
                    {
                        cResult = rcERROR;
                        LOG_MSG(db_UPGRADE, "flash Copy fail [0x%04x][0x%04x][0x%04x]", ulExtCRC, ulCRC, ulFirsrCRC);
                    }
                    else
                    {
                        cResult = rcSUCCESS;
                        LOG_MSG(db_UPGRADE, "flash Copy pass [0x%04x][0x%04x][0x%04x]", ulExtCRC, ulCRC, ulFirsrCRC);
                    }
                }
                else
                {
                    cResult = rcERROR;
                    LOG_MSG(db_UPGRADE, "flash erase fail\n");
                }
            }
            ucRetry++;
            if(ucRetry == 5)
            {
                ucRetryStep++;
                switch(ucRetryStep)
                {
                    case 1: // read clock delay 1T
                        ucRetry = 0;
                        dvProAV_RsuFlashReadDlySet(1);
                        LOG_MSG(db_ALWAYS, "Retry setp : RSU read clock delay 1T\n");
                        break;
                    case 2: // read clock delay 2T
                        ucRetry = 0;
                        dvProAV_RsuFlashReadDlySet(2);
                        LOG_MSG(db_ALWAYS, "Retry setp  : RSU read clock delay 2T\n");
                        break;
                    case 3: // clock rate 133M/6  & read clock delay 0T
                        ucRetry = 0;
                        dvProAV_RsuFlashReadDlySet(0);
                        dvProAV_RsuFlashClkRateSet(3);
                        LOG_MSG(db_ALWAYS, "Retry setp  : RSU clock rate 133M/6  & read clock delay 0T\n");
                        break;
                    case 4: // clock rate 133M/6 & read clock delay 1T
                        ucRetry = 0;
                        dvProAV_RsuFlashReadDlySet(1);
                        LOG_MSG(db_ALWAYS, "Retry setp  : RSU clock rate 133M/6  & read clock delay 1T\n");
                        break;
                    case 5: // clock rate 133M/6 & read clock delay 2T
                        ucRetry = 0;
                        dvProAV_RsuFlashReadDlySet(2);
                        LOG_MSG(db_ALWAYS, "Retry setp  : RSU clock rate 133M/6  & read clock delay 2T\n");
                        break;
                    case 6: // flash r/w fail, set to default value
                        //ucRetry = 5;
                        dvProAV_RsuFlashReadDlySet(0);
                        dvProAV_RsuFlashClkRateSet(2);
                        LOG_MSG(db_ALWAYS, "Retry setp  : RSU flash r/w fail, set to default value\n");
                        break;
                }
            }
        }
        while((ucRetry < 5) && (cResult != rcSUCCESS));

        LOG_MSG(db_UPGRADE, "Copy Address 0x%08x %03d/%03d Retry %d\n", ulUpgradeAdd, (ulStartBlock - 1), ulBlock, ucRetry);
    }

    free(cbuf);

    MS_SLEEP(500);

    if(cResult == rcSUCCESS)
    {
        if(TRUE == halRsu_SemaphoreTake(0, __FUNCTION__))
        {
            dvProAV_RsuReConfig(APP_CODE_START_ADDRESS, 0);

            MS_SLEEP(5000);

            dvProAV_ScalerDriverInit();

            halRsu_SemaphoreGive(0);
        }
    }

    LOG_MSG(db_UPGRADE, "FPGA upgrade done");

    return cResult;
}

UINT32 halRsu_FPGAVersionGet(void)
{
    int status = rcSUCCESS;
    uint32 mainVer = 0;
    uint32 subVer = 0;
    uint32 minorVer = 0;
    uint32 version = 0;

    if(TRUE == halRsu_SemaphoreTake(0, __FUNCTION__))
    {
        // Read ProAV FPGA Version
        status &= dvProAV_AccessRead(eComPrjMainRev, &mainVer);
        status &= dvProAV_AccessRead(eComPrjSubRev, &subVer);
        status &= dvProAV_AccessRead(eComPrjMinorRev, &minorVer);

        halRsu_SemaphoreGive(0);
    }

    LOG_MSG(db_UPGRADE,"ProAV Scaler Driver, Version = %ld.%ld.%ld\n", mainVer, subVer, minorVer);

    version = (uint32) (minorVer << 16 | mainVer << 8 | subVer);

    return version;
}

#if 0
UINT32 rsu_upgrade_erase(char *filename, const UINT32 ulFlash_Addr) //A70LV_Larry_0134
{
    UINT32 ulBlock = 0;
    UINT32 ulStartBlock = 0;
    UINT32 ulCount = 0;
    UINT32 ulFirsrCheckSum = 0;
    UINT32 ulExtSheckSum = 0;
    UINT32 ulUpgradeAdd = ulFlash_Addr;
    UINT32 ulSize = 0;
    FILE *pFile = NULL;

    pFile = fopen(filename, "rb");

    if(pFile == NULL) //A70LV_Larry_0001
    {
        return 0;
    }

    fseek(pFile, 0, SEEK_END);
    ulSize = ftell(pFile);

    fclose(pFile);

    if((ulSize % EXTERNAL_FLASH_SECTION_SIZE == 0))
    {
        ulBlock = ulSize/EXTERNAL_FLASH_SECTION_SIZE;
    }
    else
    {
        ulBlock = (ulSize/EXTERNAL_FLASH_SECTION_SIZE) + 1;
    }

    dvProAV_UmbDataFmtSet(true);    //dvProAV_AccessWrite(eRsuSpi2BufOrder,1);

    ulStartBlock = 0;

    for(ulCount = ulStartBlock; ulCount < ulBlock; ulCount++)
    {

        if(rcSUCCESS == dvProAV_SfiEraseSector(ulUpgradeAdd, EXTERNAL_FLASH_SECTION_SIZE))//dvProAV_RsuFlashEraseSector(ulUpgradeAdd, EXTERNAL_FLASH_SECTION_SIZE))
        {
            LOG_MSG(db_UPGRADE, "flash erase 0x%08x\n", ulUpgradeAdd);
        }
        else
        {
            LOG_MSG(db_UPGRADE, "flash erase fail\n");
        }

        ulUpgradeAdd = ulUpgradeAdd + EXTERNAL_FLASH_SECTION_SIZE;
    }


    return 1;
}

void rsu_flash_upgrade_1(void)
{
    printf("Load %s\n", FPGA_FW_01);

    rsu_upgrade_partial(FPGA_FW_01, 0x04000000, 0);
}

void rsu_flash_upgrade_2(void)
{
    rsu_upgrade_partial(FPGA_FW_02, 0x04000000, 0);
}
#endif /* 0 */



