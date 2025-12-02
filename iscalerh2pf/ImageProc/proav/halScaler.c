#include "dvProAV.h"       //A70LV_Doulas_0013
#include "halScaler.h"
#include "utilDbgMsg.h"
#include "halFrontEndCtrlAPI.h"     //A70LV_Doulas_0115
#include "halMCUCtrlAPI.h"
#include "halWarping.h"

#include "dvProAV_Interface.h"
#include "dvProAV_Scaler.h"
#include "dvProAV_OSD.h"
#include "dvProAV_HdmiTx.h"
#include "dvProAV_HdmiRx.h"
#include "dvProAV_SerialFlash.h"
#include "dvProAV_HdrGammaTable.h"
#include "dvProAV_Reg.h"
#include "dvProAV_PixelShiftWith3d.h"
#include "dvProAV_Hdr.h"
#include "opdCtrlAPI.h"

typedef struct
{
    ePANEL_ID                    ePanelTimingId;
    BOOL                         bScalerInit;
    UINT32                       ulScalerErrCode;
    UINT32                       ulRxErrCode;
    UINT32                       ulSpliterErrCode;
    UINT32                       ulVCXOErrCode;
    UINT8                        ucExt3DSyncCount;
    UINT8                        ucExt3DSyncSetting;
    UINT8                        ucCurrentRXPort;
    UINT8                        ucRGBMask;  //A70LK_Simon_0017

    SemaphoreHandle_t            xSemaphore;
#if(CURRENT_RTOS_TYPE == RTOS_STATIC)
    StaticSemaphore_t            xMutexBuffer;
#endif


}sHAL_SCALER_INFORMATION, *PsHAL_SCALER_INFORMATION;

typedef struct
{
    UINT8 ucColorSpace[eMCT_LAST];
    UINT8 ucScanMode[eMCT_LAST];
    UINT8 ucVideoYUV[eMCT_LAST];
    UINT8 ucVGA_SYNC_TYPE[eMCT_LAST];
    UINT8 ucRxPortStatus[eRxPort_Max];
    UINT8 ucRxPortStatusCount[eRxPort_Max];
    DetTmg sRxPortDetTiming[eRxPort_Max];
    sPROAV_AVI sRxPortAVI_Info[eRxPort_Max];
    sPROAV_AVI sRxPortAVIMonitor_Info[eRxPort_Max]; //A70LK_Larry_0133
    UINT8 ucMonitorState[eMCT_LAST];
    UINT8 ucAVIInfo[eRxPort_Max][28];
    UINT8 ucVSIInfo[8];
}sAVI_INFO;

static sUSER_SYSTEM_SETTING            m_sUserSystemSetting;   //A70LV_Doulas_0013

static sHAL_SCALER_INFORMATION m_sHalScalerInfo[eMCT_LAST];

static sAVI_INFO m_sProAV_Info = {0};
static UINT8 m_cHDMIReDriverCheck[eRxPort_Max] = {0};
static char cSemaphore[256] = {'\0'};

static UINT8 m_ucBackupPrimaryInput = eCM_SOURCE_NUMBER;
static UINT8 m_ucBackupSecondaryInput = eCM_SOURCE_NUMBER;

static bool RGBSwitch_Item[eHAL_SCALER_RGB_ITEM_COUNT] = {false}; //A70LK_Casper_0008
static bool RGBTransSwitch = false; //A70LK_Casper_0011
static UINT8 m_ucLMK03328_Select = eLMK03328PLL_REF_SELECT_SECR;

#define PROAV_DET_TIMING_SIZE sizeof(DetTmg)/sizeof(UINT8)
#define PROAV_AVI_INFO_SIZE sizeof(sPROAV_AVI)/sizeof(UINT8)
#define AVI_INFO_SIZE       sizeof(AVI_INFOFRAME)/sizeof(UINT8)

void halScaler_ProAV_ColorSpaceConversion(UINT8 ucCH, UINT8 ucColorSpace, UINT8 ucPixelMode, UINT8 ucRGBMode);
eHAL_SCALER_EXEC_CODE halScaler_HDRLevelSetting(UINT8 ucLevel);

static BOOL halScaler_SemaphoreTake(const UINT8 ucCH, const char *pcFunc)
{
    //xSemaphoreGive(m_sHalScalerInfo[ucCH].xSemaphore);

    BOOL bResult = dvProAV_SemaphoreTake(TRUE, pcFunc);

    LOG_MSG(db_HAL_SEMAPHORE, "(func:%s) Semaphore %d \r\n", pcFunc, bResult);

    if(bResult != TRUE)
    {
        LOG_MSG(db_ALWAYS, "(func:%s): SemaphoreTake Fail, (func:%s) is locked\n", pcFunc, cSemaphore);
        ASSERT(bResult == TRUE);
    }
    else
    {
        sprintf(cSemaphore, "%s", pcFunc);
    }

    return bResult;
}

static BOOL halScaler_SemaphoreGive(const UINT8 ucCH)
{
    //xSemaphoreGive(m_sHalScalerInfo[ucCH].xSemaphore);

    BOOL bResult = dvProAV_SemaphoreTake(FALSE, __FUNCTION__);

    LOG_MSG(db_HAL_SEMAPHORE, "(func:%s)\r\n", __FUNCTION__);

    if(bResult != TRUE)
    {
        ASSERT(bResult == TRUE);
    }

    return bResult;
}

// ==============================================================================
// FUNCTION NAME: halScaler_VersionGet
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
// 2020/06/19, Larry Create
// --------------------
// ==============================================================================
UINT32 halScaler_VersionGet(void)
{
    return dvProAV_SclVersionGet();
}

///////////////////////////////////////////////////////////////////////////////////
///@brief halScaler_GetScalerErrCode :
///@   This function is only used when app layer get eHAL_SCALER_EXEC_CODE_FAIL and
///@   want to know what happen on the driver layer
///@
///@param UINT8 ucCH - Channel ID
///@return UINT32 - Driver layer error code
///@author Roger Chen
///////////////////////////////////////////////////////////////////////////////////

UINT32 halScaler_GetScalerErrCode(const UINT8 ucCH)
{
    if(ucCH >= eMCT_LAST)
        return 0;

    return  m_sHalScalerInfo[ucCH].ulScalerErrCode;
}

///////////////////////////////////////////////////////////////////////////////////
///@brief halScaler_PowerStandby :
///@param ePANEL_ID ePanelId - Panel Identifier
///@return eEXEC_CODE - Fail : eEXEC_CODE_FAIL , Pass : eEXEC_CODE_PASS
///@author Roger Chen
///////////////////////////////////////////////////////////////////////////////////


eHAL_SCALER_EXEC_CODE halScaler_PowerStandby(UINT8 ucCH)
{
    if(ucCH == eMCT_CH1)
    {
        memset(&m_sProAV_Info.ucColorSpace[0], 0, sizeof(m_sProAV_Info)/sizeof(UINT8));

        m_ucBackupPrimaryInput = eCM_SOURCE_NUMBER;
        m_ucBackupSecondaryInput = eCM_SOURCE_NUMBER;
        m_ucLMK03328_Select = eLMK03328PLL_REF_SELECT_SECR;
        RGBTransSwitch = false;
    }

    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
    return eHAL_SCALER_EXEC_CODE_PASS;
}


///////////////////////////////////////////////////////////////////////////////////
///@brief halScaler_PowerNormal : Init Scaler 4 Chs in once, no matter 2K/4K
///@param ePANEL_ID ePanelId - Panel Identifier
///@return eEXEC_CODE - Fail : eEXEC_CODE_FAIL , Pass : eEXEC_CODE_PASS
///@author Roger Chen
///////////////////////////////////////////////////////////////////////////////////
eHAL_SCALER_EXEC_CODE halScaler_PowerNormal(UINT8 ucCH, const ePANEL_ID ePanelId)
{
    UINT32 ulExeRet;

    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) ePanelId ucCH %d ePanelId %d\r\n", __FUNCTION__, __LINE__,ucCH, ePanelId);

    {
        if(ucCH >= eMCT_LAST)
            return eHAL_SCALER_EXEC_CH_OVER_RANGE;
    }

    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[ucCH].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_Init() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(0);

            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }

#ifndef SIMULATOR_ISCALER
        {
            //UINT8 ucOverlay[eMCT_LAST]={0, 1};    //A70LV_Doulas_0008 remove

            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) ePanelId %d\r\n", __FUNCTION__, __LINE__, ePanelId);

            m_sHalScalerInfo[ucCH].ePanelTimingId = ePanelId;
            m_sHalScalerInfo[ucCH].ucExt3DSyncCount = 5;
            m_sHalScalerInfo[ucCH].ucRGBMask = 0;  //A70LK_Simon_0017

            halScaler_IintChannelSetting((eMCT)ucCH);   //A70LV_Doulas_0030

            if((eMCT)ucCH == eMCT_CH1)
            {
                memset(&m_sProAV_Info.ucColorSpace[0], 0, sizeof(m_sProAV_Info)/sizeof(UINT8));
                memset(&m_cHDMIReDriverCheck[0], 3, sizeof(m_cHDMIReDriverCheck)/sizeof(UINT8));

                for (int i = 0; i < eHAL_SCALER_RGB_ITEM_COUNT; i++)  //A70LK_Casper_0008
                {
                    RGBSwitch_Item[i]  = true;
                }

                dvProAV_AccessReconfig();
                dvProAV_HdmiRxInit();
                dvProAV_ScalerDriverInit();
                dvProAV_HdmiTxDriverInit();
                dvProAV_SfiInit();  // ProAV_Rex_0037
                m_sUserSystemSetting.sImageSetting.ucMEMCBypass = 1;
                m_sHalScalerInfo[ucCH].ucExt3DSyncSetting = 0xFF;
                m_sHalScalerInfo[ucCH].ucCurrentRXPort = RXPORT_CHANNEL_0;
            }
            else
            {
                m_sHalScalerInfo[ucCH].ucCurrentRXPort = RXPORT_CHANNEL_1;
            }

            if((ulExeRet = dvPro_Init_Panel((eMCT)ucCH, ePanelId)) != ePROAV_EXEC_CODE_PASS)
            {
                m_sHalScalerInfo[ucCH].ulScalerErrCode = ulExeRet;
                halScaler_SemaphoreGive(0);
                return eHAL_SCALER_EXEC_CODE_DV_SCALER_INIT_FAIL;
            }
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) \r\n", __FUNCTION__, __LINE__);

            //TODO
            dvPro_SetOverlay_2K((eMCT)ucCH, m_sUserSystemSetting.sSourceSetting.ucPIPEnable);  //A70LV_Doulas_0024
            //dvPro_SetOverlayCoef_2K((eMCT)ucCH, ucOverlay);

            dvPro_SetInputPort_2K((eMCT)ucCH);

            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) \r\n", __FUNCTION__, __LINE__);
        }
#endif /* SIMULATOR_ISCALER */

        halScaler_SemaphoreGive(0);

    }
    else
    {
        LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore\r\n", __FUNCTION__, __LINE__);
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }

    return eHAL_SCALER_EXEC_CODE_PASS;
}

eHAL_SCALER_EXEC_CODE halScaler_ConfigureForDisplay(UINT8 ucCH, const UINT8 ucColorFmt)
{
    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);

    //bIs4K = halScaler_Is4KPanel(m_sHalScalerInfo[ucCH].ePanelTimingId);

    {
        if(ucCH >= eMCT_LAST)
            return eHAL_SCALER_EXEC_CH_OVER_RANGE;
    }


    /* See if we can obtain the semaphore.  If the semaphore is not
    available wait 10 ticks to see if it becomes free. */
    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[ucCH].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(0);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }

        dvPro_BuildPicture_2K((eMCT)ucCH);

        halScaler_SemaphoreGive(0);

    }
    else
    {
        LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore\r\n", __FUNCTION__, __LINE__);
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }


    return eHAL_SCALER_EXEC_CODE_PASS;
}

eHAL_SCALER_EXEC_CODE halScaler_ConfigureColor(UINT8 ucCH)
{
    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);

    //bIs4K = halScaler_Is4KPanel(m_sHalScalerInfo[ucCH].ePanelTimingId);

    {
        if(ucCH >= eMCT_LAST)
            return eHAL_SCALER_EXEC_CH_OVER_RANGE;
    }


    /* See if we can obtain the semaphore.  If the semaphore is not
    available wait 10 ticks to see if it becomes free. */
    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[ucCH].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(0);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }

        if((ucCH == eMCT_CH2) && (m_sUserSystemSetting.sSourceSetting.ucPIPEnable == ets_OFF))
        {

        }
        else
        {
            dvPro_Set_RGB_Gain((eMCT)ucCH);
            dvPro_Set_RGB_Offset((eMCT)ucCH);
            dvPro_SetBrightness((eMCT)ucCH);
            dvPro_SetContrast((eMCT)ucCH);
            dvPro_Set_Color((eMCT)ucCH);
        }
        halScaler_SemaphoreGive(0);

    }
    else
    {
        LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore\r\n", __FUNCTION__, __LINE__);
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }

    return eHAL_SCALER_EXEC_CODE_PASS;
}

eHAL_SCALER_EXEC_CODE halScaler_ConfigureColorSpace(UINT8 ucCH, UINT8 ucSyncSub)
{
    UINT8 ucColorSpace = 0;

    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);

    //bIs4K = halScaler_Is4KPanel(m_sHalScalerInfo[ucCH].ePanelTimingId);

    {
        if(ucCH >= eMCT_LAST)
            return eHAL_SCALER_EXEC_CH_OVER_RANGE;
    }

    /* See if we can obtain the semaphore.  If the semaphore is not
    available wait 10 ticks to see if it becomes free. */
    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[ucCH].bScalerInit)
        {
            halScaler_SemaphoreGive(0);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }

        ucColorSpace = halScaler_ColorSpace_Get(ucCH);

        halScaler_ProAV_ColorSpaceSetting((eMCT)ucCH, ucColorSpace);

        if((ucCH == eMCT_CH1) && ucSyncSub)
        {
            halScaler_ProAV_ColorSpaceSetting(eMCT_CH2, ucColorSpace);
        }

#if 0
        if(ucCH == eMCT_CH2 && m_sUserSystemSetting.sSourceSetting.ucPIPEnable)
        {
            ucColorSpace = eCM_COLOR_SPACE_AUTO;
        }
#endif /* 0 */

        halScaler_SemaphoreGive(0);

    }
    else
    {
        LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore\r\n", __FUNCTION__, __LINE__);
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }

    return eHAL_SCALER_EXEC_CODE_PASS;
}

eHAL_SCALER_EXEC_CODE halScaler_DisplaySFG(UINT8 ucCH,
                            const UINT8 ucFillColor, const BOOL bEnable)
{

    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);

    //bIs4K = halScaler_Is4KPanel(m_sHalScalerInfo[ucCH].ePanelTimingId);

    {
        if(ucCH >= eMCT_LAST)
            return eHAL_SCALER_EXEC_CH_OVER_RANGE;
    }

    /* See if we can obtain the semaphore.  If the semaphore is not
    available wait 10 ticks to see if it becomes free. */
    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[ucCH].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(0);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }


        halScaler_SemaphoreGive(0);

    }
    else
    {
        LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore\r\n", __FUNCTION__, __LINE__);
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }


    return eHAL_SCALER_EXEC_CODE_PASS;
}

eHAL_SCALER_EXEC_CODE halScaler_DisplaySplash(void)
{
    return eHAL_SCALER_EXEC_CODE_PASS;
}

eHAL_SCALER_EXEC_CODE halScaler_SetOverlay(UINT8 ucCH, const BOOL bEnable)
{

    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);

    //bIs4K = halScaler_Is4KPanel(m_sHalScalerInfo[ucCH].ePanelTimingId);

    {
        if(ucCH >= eMCT_LAST)
            return eHAL_SCALER_EXEC_CH_OVER_RANGE;


        /* See if we can obtain the semaphore.  If the semaphore is not
        available wait 10 ticks to see if it becomes free. */
        if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
        {
            if(!m_sHalScalerInfo[ucCH].bScalerInit)
            {
                LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
                halScaler_SemaphoreGive(0);
                return eHAL_SCALER_EXEC_CODE_NOT_INIT;
            }

            dvPro_SetOverlay_2K((eMCT)ucCH, bEnable);

            halScaler_SemaphoreGive(0);

        }
        else
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore\r\n", __FUNCTION__, __LINE__);
            return eHAL_SCALER_EXEC_CODE_FAIL;
        }
    }


    return eHAL_SCALER_EXEC_CODE_PASS;
}

eHAL_SCALER_EXEC_CODE halScaler_SetOverlayCoef(UINT8 ucCH, const PUINT8 pucOverLay)
{

    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);

    //bIs4K = halScaler_Is4KPanel(m_sHalScalerInfo[ucCH].ePanelTimingId);


    {
        if(ucCH >= eMCT_LAST)
            return eHAL_SCALER_EXEC_CH_OVER_RANGE;


        /* See if we can obtain the semaphore.  If the semaphore is not
        available wait 10 ticks to see if it becomes free. */
        if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
        {
            if(!m_sHalScalerInfo[ucCH].bScalerInit)
            {
                LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
                halScaler_SemaphoreGive(0);
                return eHAL_SCALER_EXEC_CODE_NOT_INIT;
            }

            halScaler_SemaphoreGive(0);
        }
        else
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore\r\n", __FUNCTION__, __LINE__);
            return eHAL_SCALER_EXEC_CODE_FAIL;
        }

    }

    return eHAL_SCALER_EXEC_CODE_PASS;
}



eHAL_SCALER_EXEC_CODE halScaler_PortConfig(UINT8 ucCH)
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS; //A70LV_Doulas_0002

//    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);

    //bIs4K = halScaler_Is4KPanel(m_sHalScalerInfo[ucCH].ePanelTimingId);

    {
        if(ucCH >= eMCT_LAST)
            return eHAL_SCALER_EXEC_CH_OVER_RANGE;
    }


    /* See if we can obtain the semaphore.  If the semaphore is not
    available wait 10 ticks to see if it becomes free. */
    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[ucCH].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(0);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }


#ifdef __ICHIP_CONTROL__
        {
            if(dvPro_CheckInput_2K((eMCT)ucCH))
            {
				dvPro_SetIdual_2K((eMCT)ucCH);
            }
            else
            {
                eResult = eHAL_SCALER_EXEC_CODE_FAIL; //A70LV_Doulas_0002
            }
        }
#endif /* __ICHIP_CONTROL__ */

        halScaler_SemaphoreGive(0);

    }
    else
    {
        LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore(%d)\r\n", __FUNCTION__, __LINE__,ucCH);
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }


    return eResult;//eHAL_SCALER_EXEC_CODE_PASS;  //A70LV_Doulas_0002
}

eHAL_SCALER_EXEC_CODE halScaler_Init(UINT8 ucCH)//A70LV_Doulas_0003 //Init 4 channels in once, no matter 4K, 2K
{
  //  UINT8 ucCH;   //A70LV_Doulas_0003

    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);

  //  for(ucCH = eMCT_CH1; ucCH < eMCT_LAST; ucCH++)    //A70LV_Doulas_0003 remove
    {
        m_sHalScalerInfo[ucCH].bScalerInit = TRUE;

#if (SYSTEM_OS_TYPE == POSIX_COMPLIANT)
    if(pthread_mutex_init(&m_sHalScalerInfo[ucCH].xSemaphore, NULL) == -1)
    {
        LOG_MSG(db_ALWAYS, "%s pthread_mutex_init fail [%s]\n", __FUNCTION__ , strerror(errno));
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }
#else
    #if(CURRENT_RTOS_TYPE == RTOS_STATIC)
        m_sHalScalerInfo[ucCH].xSemaphore = xSemaphoreCreateMutexStatic(&m_sHalScalerInfo[ucCH].xMutexBuffer);
    #else
        m_sHalScalerInfo[ucCH].xSemaphore = xSemaphoreCreateMutex();
    #endif

    if(m_sHalScalerInfo[ucCH].xSemaphore == NULL)
        return eHAL_SCALER_EXEC_CODE_FAIL;
#endif

    }

    return eHAL_SCALER_EXEC_CODE_PASS;
}

//Hal Brightness data range 0~100 (default:50)
eHAL_SCALER_EXEC_CODE halScaler_Brightness_Set(UINT8 ucCH,UINT8 ucSetting)  //A70LV_Doulas_0072 modify //A70LV_Doulas_0022 modify  //A70LV_Doulas_0009 modify
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

    eResult = halScaler_Brightness_Value_Set(ucCH, ucSetting);   //A70LV_Doulas_0012 modify
    if(eResult != eHAL_SCALER_EXEC_CODE_PASS)
        return eResult;

    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[0].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(0);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }

        if(m_sUserSystemSetting.sSourceSetting.ucPIPEnable)
        {
            dvPro_SetBrightness(ucCH);
        }
        else
        {
            dvPro_SetBrightness(eMCT_CH1);
            dvPro_SetBrightness(eMCT_CH2);
        }

        halScaler_SemaphoreGive(0);
    }
    else
    {
        LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore\r\n", __FUNCTION__, __LINE__);
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }

    return eResult;
}

//Hal Contrast data range 0~100 (default:50)
eHAL_SCALER_EXEC_CODE halScaler_Contrast_Set(UINT8 ucCH,UINT8 ucSetting)    //A70LV_Doulas_0072 modify//A70LV_Doulas_0022 //A70LV_Doulas_0009 modify
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

    eResult = halScaler_Contrast_Value_Set(ucCH,ucSetting);   //A70LV_Doulas_0012 modify
    if(eResult != eHAL_SCALER_EXEC_CODE_PASS)
        return eResult;

    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[0].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(0);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }

        //execute Contrast action function
        if(m_sUserSystemSetting.sSourceSetting.ucPIPEnable)
        {
            dvPro_SetContrast(ucCH);
        }
        else
        {
            dvPro_SetContrast(eMCT_CH1);
            dvPro_SetContrast(eMCT_CH2);
        }

        halScaler_SemaphoreGive(0);
    }
    else
    {
        LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore\r\n", __FUNCTION__, __LINE__);
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }

    return eResult;
}

INT8 halScaler_Dump_Register(UINT8 cBank)     //A70LV_Doulas_0002
{
    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        dvPro_RegDump(cBank);

        halScaler_SemaphoreGive(0);
    }

    return HAL_SCALER_PASS;
}

eHAL_SCALER_EXEC_CODE halScaler_Bypasse_Mode(void)     //A70LV_Doulas_0072 modify//A70LV_Doulas_0002
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

//    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);


    /* See if we can obtain the semaphore.  If the semaphore is not
    available wait 10 ticks to see if it becomes free. */
    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        halScaler_SemaphoreGive(0);
    }
    else
    {
        LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore\r\n", __FUNCTION__, __LINE__);
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_Test_Mode(void)     //A70LV_Doulas_0072 modify//A70LV_Doulas_0002
{
#ifdef __ICHIP_CONTROL__
    dvPro_Test();
#endif /* __ICHIP_CONTROL__ */

    return eHAL_SCALER_EXEC_CODE_PASS;
}

eHAL_SCALER_EXEC_CODE halScaler_HUE_Set(UINT8 ucCH,UINT8 ucSetting)  //A70LV_Doulas_0072 modify//A70LV_Doulas_0022 modify
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

    eResult = halScaler_HUE_Value_Set(ucCH,ucSetting);  //A70LV_Doulas_0012 modify
    if(eResult != eHAL_SCALER_EXEC_CODE_PASS)
        return eResult;

    /* See if we can obtain the semaphore.  If the semaphore is not
    available wait 10 ticks to see if it becomes free. */
    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[ucCH].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(0);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }

        dvPro_Set_HUE(eMCT_CH1);
        dvPro_Set_HUE(eMCT_CH2);

        halScaler_SemaphoreGive(0);
    }
    else
    {
        LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore(%d)\r\n", __FUNCTION__, __LINE__,ucCH);
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_Saturation_Set(UINT8 ucCH,UINT8 ucSetting)  //A70LV_Doulas_0072 modify//A70LV_Doulas_0022
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

    eResult = halScaler_Saturation_Value_Set(ucCH,ucSetting);  //A70LV_Doulas_0012 modify
    if(eResult != eHAL_SCALER_EXEC_CODE_PASS)
        return eResult;

    /* See if we can obtain the semaphore.  If the semaphore is not
    available wait 10 ticks to see if it becomes free. */
    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[ucCH].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(0);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }

        dvPro_Set_Saturation(eMCT_CH1);        //A70LV_Doulas_0155 modify//A70LV_Doulas_0008 modify
        dvPro_Set_Saturation(eMCT_CH2);        //A70LV_Doulas_0155

        halScaler_SemaphoreGive(0);
    }
    else
    {
        LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore(%d)\r\n", __FUNCTION__, __LINE__,ucCH);
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_PIP_PBP_Layout_Set(UINT8 ucLayout)    //A70LV_Doulas_0072 modify//A70LV_Doulas_0024
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    //LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): Brightness=%d\r\n", __FUNCTION__, __LINE__, miBrightness);

    if(ucLayout > eCM_MAIN_LAYOUT_PIP_TOP_RIGHT)
        return eHAL_SCALER_EXEC_CH_OVER_RANGE;

    m_sUserSystemSetting.sSourceSetting.ucMainLayout = ucLayout;

    dvPro_Set_PIP_PBP_Layout(ucLayout);

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_PIP_PBP_Size_Set(UINT8 ucSize)    //A70LV_Doulas_0072 modify//A70LV_Doulas_0024
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    //LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): Brightness=%d\r\n", __FUNCTION__, __LINE__, miBrightness);

    if(ucSize > eCM_PIP_SIZE_LARGE)
        return eHAL_SCALER_EXEC_CH_OVER_RANGE;

    m_sUserSystemSetting.sSourceSetting.ucPIPSize = ucSize;

    dvPro_Set_PIP_PBP_Size(ucSize);

    return eResult;
}

UINT8 halScaler_PIP_PBP_Layout_Get(void)    //A70LV_Doulas_0003
{
    return m_sUserSystemSetting.sSourceSetting.ucMainLayout;     //A70LV_Doulas_0024
}

UINT8 halScaler_PIP_PBP_Size_Get(void)    //A70LV_Doulas_0003
{
    return m_sUserSystemSetting.sSourceSetting.ucPIPSize;   //A70LV_Doulas_0024
}

void halScaler_Resync_Init(UINT8 eCH)   //A70LV_Doulas_0004
{
    dvPro_Resync_Init((eMCT)eCH);  //A70LV_Doulas_0008 modify
}

eHAL_SCALER_EXEC_CODE halScaler_SYNC_Lock_Get(UINT8 ucCH)  //A70LV_Doulas_0009
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    BOOL bSyncLock = FALSE; //A70LV_Doulas_0009

//    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);

    if(ucCH >= eMCT_LAST)
            return eHAL_SCALER_EXEC_CH_OVER_RANGE;

    /* See if we can obtain the semaphore.  If the semaphore is not
    available wait 10 ticks to see if it becomes free. */
    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[ucCH].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(0);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }

        bSyncLock = dvPro_SYNC_LOCK_Compare((eMCT)ucCH);    //A70LV_Doulas_0009 modify

        if(bSyncLock)
        {
            eResult = eHAL_SCALER_EXEC_CODE_PASS;
        }
        else
        {
            eResult = eHAL_SCALER_EXEC_CODE_FAIL;
        }

        halScaler_SemaphoreGive(0);

    }
    else
    {
        //*bSyncLock = FALSE;   //A70LV_Doulas_0009 remove
        LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore(%d)\r\n", __FUNCTION__, __LINE__,ucCH);
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_Freeze_Set(UINT8 ucCH,BOOL bFreezeEn)  //A70LV_Doulas_0072 modify//A70LV_Doulas_0004
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

//    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);

    if(ucCH >= eMCT_LAST)
        return eHAL_SCALER_EXEC_CH_OVER_RANGE;

    /* See if we can obtain the semaphore.  If the semaphore is not
    available wait 10 ticks to see if it becomes free. */
    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[ucCH].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(0);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }

        dvProAV_SclInputFreezeSet(eSclSrcIn_MainSub, bFreezeEn);

        halScaler_SemaphoreGive(0);

    }
    else
    {
        LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore(%d)\r\n", __FUNCTION__, __LINE__,ucCH);
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_Freeze_Get(UINT8 ucCH,BOOL *bFreezeEn)  //A70LV_Doulas_0072 modify//A70LV_Doulas_0004
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

//    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);

    if(ucCH >= eMCT_LAST)
            return eHAL_SCALER_EXEC_CH_OVER_RANGE;

    /* See if we can obtain the semaphore.  If the semaphore is not
    available wait 10 ticks to see if it becomes free. */
    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[ucCH].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(0);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }

        *bFreezeEn = dvProAV_SclInputFreezeGet((SclEntity)ucCH);     //A70LV_Doulas_0008 modify

        halScaler_SemaphoreGive(0);
    }
    else
    {
        *bFreezeEn = FALSE;
        LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore(%d)\r\n", __FUNCTION__, __LINE__,ucCH);
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_Panel_Set(UINT8 ucCH,ePANEL_ID ePanelId)  //A70LV_Doulas_0072 modify//A70LV_Doulas_0005
{
    //UINT32 ulExeRet;
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

//    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);

    if(ucCH >= eMCT_LAST)
            return eHAL_SCALER_EXEC_CH_OVER_RANGE;

    /* See if we can obtain the semaphore.  If the semaphore is not
    available wait 10 ticks to see if it becomes free. */
    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[ucCH].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(0);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }

        if((eMCT)ucCH == eMCT_CH1)
        {
            dvPro_Panel_Change_Setting((eMCT)ucCH, ePanelId);
        }

        halScaler_SemaphoreGive(0);

    }
    else
    {
        LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore(%d)\r\n", __FUNCTION__, __LINE__,ucCH);
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }

    return eResult;
}

#if 0

UINT32 halScaler_Panel_Colck_Set(ePANEL_ID ePanelId)    //A70LV_Doulas_0005
{
    UINT32 ulExeRet = eVCXO_EXEC_CODE_PASS;

#ifdef 0
    switch(ePanelId)
    {
        case ePANEL_ID_WUXGA_60HZ:
            ulExeRet = dvVCXO949_Init(eVCXO_SEL_C821_WU);
            break;

        case ePANEL_ID_1080P_60HZ:
            ulExeRet = dvVCXO949_Init(eVCXO_SEL_C821_HD);
            break;

        case ePANEL_ID_720P_120HZ:      //A70LV_Doulas_0094
            ulExeRet = dvVCXO949_Init(eVCXO_SEL_C821_720P120);
            break;

        case ePANEL_ID_SVGA_120HZ:      //A70LV_Doulas_0094
            ulExeRet = dvVCXO949_Init(eVCXO_SEL_C821_800x600_120);
            break;

        case ePANEL_ID_1080P_120HZ:      //A70LV_Doulas_0094
            ulExeRet = dvVCXO949_Init(eVCXO_SEL_C821_1080P120);
            break;

        case ePANEL_ID_WUXGA_120HZ:      //A70LV_Doulas_0094
            ulExeRet = dvVCXO949_Init(eVCXO_SEL_C821_WUXGA120);
            break;
    }
#endif /* 0 */
    return ulExeRet;
}
#endif /* 0 */

eHAL_SCALER_EXEC_CODE halScaler_AutoPhaseStart(UINT8 ucCH)  //A70LV_Doulas_0007
{
    eHAL_SCALER_EXEC_CODE ulExeRet = eHAL_SCALER_EXEC_CODE_PASS;

    if(ucCH >= eMCT_LAST)
            return eHAL_SCALER_EXEC_CH_OVER_RANGE;

    /* See if we can obtain the semaphore.  If the semaphore is not
    available wait 10 ticks to see if it becomes free. */
    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[ucCH].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(0);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }

#ifdef __ICHIP_CONTROL__
        eResult = dvPro_AutoPhaseStart((eMCT)ucCH);    //A70LV_Doulas_0008 modify
        if(eResult == ePROAV_EXEC_CODE_PASS)
        {
            //A70LV_Doulas_0211 modify//A70LV_Doulas_0116 modify
            UINT8 ucCurrentPhase;
            BOOL   bForceSyncReseVal;   //A70LV_Doulas_0251
            UINT16 uiH_Toral;           //A70LV_Doulas_0251
            UINT16 uiV_Toral;           //A70LV_Doulas_0251
            if(ucCH == eMCT_CH1)        //A70LV_Doulas_0251
            {
                dvPro_ForceSyncRese_Get(&bForceSyncReseVal,&uiH_Toral,&uiV_Toral);
                if(bForceSyncReseVal)
                {
                    halC789Ctrl_ForcedSyncResetValueSet((UINT8)TRUE,uiH_Toral,uiV_Toral);
                }
            }
            ucCurrentPhase = dvPro_CurrentPhaseGet((eMCT)ucCH);
            halFrontEndCtrl_VGA_Phase_Set(&ucCurrentPhase);
            ulExeRet = eHAL_SCALER_EXEC_CODE_PASS;
        }
        else
        {
            ulExeRet = eHAL_SCALER_EXEC_CODE_NO_AUTO_PHASE;
        }
#else
        ulExeRet = eHAL_SCALER_EXEC_CODE_NO_AUTO_PHASE;
#endif /* __ICHIP_CONTROL__ */

        halScaler_SemaphoreGive(0);

    }
    else
    {
        LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore(%d)\r\n", __FUNCTION__, __LINE__,ucCH);
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }

    return ulExeRet;
}

eHAL_SCALER_EXEC_CODE halScaler_AutoPhase(UINT8 ucCH)  //A70LV_Doulas_0007
{
    eHAL_SCALER_EXEC_CODE ulExeRet = eHAL_SCALER_EXEC_CODE_PASS;

    if(ucCH >= eMCT_LAST)
        return eHAL_SCALER_EXEC_CH_OVER_RANGE;

    /* See if we can obtain the semaphore.  If the semaphore is not
    available wait 10 ticks to see if it becomes free. */
    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[ucCH].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(0);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }

#ifdef __ICHIP_CONTROL__
        eResult = dvPro_AutoPhase((eMCT)ucCH);     //A70LV_Doulas_0008 modify
        if(eResult == ePROAV_EXEC_CODE_PASS)
        {
            UINT8 ucBestPhase = dvPro_BsetPhaseGet((eMCT)ucCH);      //A70LV_Doulas_01136//A70LV_Doulas_0008 modify
        //    LOG_MSG(db_ALWAYS, "Phase Set %d \r\n",ucBestPhase);      //A70LV_Doulas_0215 debug
            halFrontEndCtrl_VGA_Phase_Set(&ucBestPhase);        //A70LV_Doulas_0211 //A70LV_Doulas_0116
            ulExeRet = eHAL_SCALER_EXEC_CODE_PASS;
        }
        else if(eResult == ePROAV_EXEC_CODE_CH_AUTO_PHASE)
        {
            ucCurrentPhase = dvPro_CurrentPhaseGet((eMCT)ucCH);    //A70LV_Doulas_0008 modify
            if(ucCurrentPhase <= VGA_MAX_PHASE)
            {
                halFrontEndCtrl_VGA_Phase_Set(&ucCurrentPhase);     //A70LV_Doulas_0211 //A70LV_Doulas_0116 modify
            }
            ulExeRet = eHAL_SCALER_EXEC_CODE_AUTO_PHASE_GOING;
        }
        else
        {
            ulExeRet = eHAL_SCALER_EXEC_CODE_PASS;
        }
#endif /* __ICHIP_CONTROL__ */

        halScaler_SemaphoreGive(0);

    }
    else
    {
        LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore(%d)\r\n", __FUNCTION__, __LINE__,ucCH);
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }

    return ulExeRet;
}

eHAL_SCALER_EXEC_CODE halScaler_SetInputSource(UINT8 ucCH,UINT8 ucInputSource)  //A70LV_Doulas_0007
{
    eHAL_SCALER_EXEC_CODE ulExeRet;

    if(ucCH >= eMCT_LAST)
            return eHAL_SCALER_EXEC_CH_OVER_RANGE;

    if(ucCH == eMCT_CH1)
    {
        UINT8 ucPreInputSource = m_sUserSystemSetting.sSourceSetting.ucInputSourceMain;
        m_sUserSystemSetting.sSourceSetting.ucInputSourceMain = ucInputSource;
        if( (m_sUserSystemSetting.sCommonSetting.ucBackupIputSwitch != ets_OFF) ||
            (m_sUserSystemSetting.sSourceSetting.ucPIPEnable != ets_OFF) ||
            (dvPro_Input_3D_Format_Config_Get() != eINPUT_3D_TYPE_OFF) ||
            ((m_ucBackupPrimaryInput != ucInputSource) &&
             (m_ucBackupSecondaryInput != ucInputSource) &&
             (ucPreInputSource != ucInputSource)))
        {
            halScaler_RxPortClear(m_sHalScalerInfo[ucCH].ucCurrentRXPort);
        }
    }
    else
    {
        UINT8 ucPreInputSource = m_sUserSystemSetting.sSourceSetting.ucInputSourceSub;
        m_sUserSystemSetting.sSourceSetting.ucInputSourceSub = ucInputSource;
        if(ucPreInputSource != ucInputSource)
        {
            if(m_sUserSystemSetting.sSourceSetting.ucPIPEnable)
            {
                halScaler_RxPortClear(m_sHalScalerInfo[ucCH].ucCurrentRXPort);
            }
        }
    }

    ulExeRet = eHAL_SCALER_EXEC_CODE_PASS;

    //dvProAV_SclVipSourceSet((eMCT)ucCH, ucInputSource);
    //dvProAV_SclVipSourceSet((eMCT)ucCH, 1); //for debug

    halScaler_IintChannelSetting(ucCH);      //A70LV_Doulas_0030

    return ulExeRet;
}

//data range 0~100 (default:50)
eHAL_SCALER_EXEC_CODE halScaler_RedOffset_Set(UINT8 ucCH,UINT8 ucSetting)  //A70LV_Doulas_0072 modify//A70LV_Doulas_0029 //A70LV_Doulas_0022 modify   //A70LV_Doulas_0011
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

    eResult = halScaler_RedOffset_Value_Set(ucCH,ucSetting);  //A70LV_Doulas_0012 modify
    if(eResult != eHAL_SCALER_EXEC_CODE_PASS)
        return eResult;

    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[0].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(0);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }

        dvPro_Set_RGB_Offset(eMCT_CH1);      //A70LV_Doulas_0155 //A70LV_Doulas_0072 modify
        dvPro_Set_RGB_Offset(eMCT_CH2);      //A70LV_Doulas_0155

        halScaler_SemaphoreGive(0);
    }
    else
    {
        LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore\r\n", __FUNCTION__, __LINE__);
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }

    return eResult;
}

//data range 0~100 (default:50)
eHAL_SCALER_EXEC_CODE halScaler_GreenOffset_Set(UINT8 ucCH,UINT8 ucSetting)  //A70LV_Doulas_0072 modify//A70LV_Doulas_0029 //A70LV_Doulas_0022 modify   //A70LV_Doulas_0011
{
    //eMCT eCH;
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

    eResult = halScaler_GreenOffset_Value_Set(ucCH,ucSetting);  //A70LV_Doulas_0012 modify
    if(eResult != eHAL_SCALER_EXEC_CODE_PASS)
        return eResult;

    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[0].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(0);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }

        dvPro_Set_RGB_Offset(eMCT_CH1);      //A70LV_Doulas_0155 //A70LV_Doulas_0072 modify
        dvPro_Set_RGB_Offset(eMCT_CH2);      //A70LV_Doulas_0155

        halScaler_SemaphoreGive(0);
    }
    else
    {
        LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore\r\n", __FUNCTION__, __LINE__);
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }

    return eResult;
}

//data range 0~100 (default:50)
eHAL_SCALER_EXEC_CODE halScaler_BlueOffset_Set(UINT8 ucCH,UINT8 ucSetting)  //A70LV_Doulas_0072 modify//A70LV_Doulas_0029 //A70LV_Doulas_0022 modify   //A70LV_Doulas_0011
{
    //eMCT eCH;
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

    eResult = halScaler_BlueOffset_Value_Set(ucCH,ucSetting);  //A70LV_Doulas_0012 modify
    if(eResult != eHAL_SCALER_EXEC_CODE_PASS)
        return eResult;

    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[0].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(0);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }

        dvPro_Set_RGB_Offset(eMCT_CH1);      //A70LV_Doulas_0155 //A70LV_Doulas_0072 modify
        dvPro_Set_RGB_Offset(eMCT_CH2);      //A70LV_Doulas_0155

        halScaler_SemaphoreGive(0);
    }
    else
    {
        LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore\r\n", __FUNCTION__, __LINE__);
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }

    return eResult;
}


//Hal Contrast data range 0~100 (default:50)
eHAL_SCALER_EXEC_CODE halScaler_RedGain_Set(UINT8 ucCH,UINT8 ucSetting)  //A70LV_Doulas_0072 modify//A70LV_Doulas_0029 //A70LV_Doulas_0022 modify      //A70LV_Doulas_0011
{
    //eMCT eCH;
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

    eResult = halScaler_RedGain_Value_Set(ucCH,ucSetting);  //A70LV_Doulas_0012 modify
    if(eResult != eHAL_SCALER_EXEC_CODE_PASS)
        return eResult;

    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[0].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(0);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }

        dvPro_Set_RGB_Gain(eMCT_CH1);    //A70LV_Doulas_0155 //A70LV_Doulas_0072 modify
        dvPro_Set_RGB_Gain(eMCT_CH2);    //A70LV_Doulas_0155

        halScaler_SemaphoreGive(0);
    }
    else
    {
        LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore\r\n", __FUNCTION__, __LINE__);
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }

    return eResult;
}

//Hal Contrast data range 0~100 (default:50)
eHAL_SCALER_EXEC_CODE halScaler_GreenGain_Set(UINT8 ucCH,UINT8 ucSetting)  //A70LV_Doulas_0072 modify//A70LV_Doulas_0029 //A70LV_Doulas_0022 modify      //A70LV_Doulas_0011
{
    //eMCT eCH;
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

    eResult = halScaler_GreenGain_Value_Set(ucCH,ucSetting);  //A70LV_Doulas_0012 modify
    if(eResult != eHAL_SCALER_EXEC_CODE_PASS)
        return eResult;

    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[0].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(0);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }

        dvPro_Set_RGB_Gain(eMCT_CH1);    //A70LV_Doulas_0155 //A70LV_Doulas_0072 modify
        dvPro_Set_RGB_Gain(eMCT_CH2);    //A70LV_Doulas_0155

        halScaler_SemaphoreGive(0);
    }
    else
    {
        LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore\r\n", __FUNCTION__, __LINE__);
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }

    return eResult;
}

//Hal Contrast data range 0~100 (default:50)
eHAL_SCALER_EXEC_CODE halScaler_BlueGain_Set(UINT8 ucCH,UINT8 ucSetting)  //A70LV_Doulas_0072 modify//A70LV_Doulas_0029 //A70LV_Doulas_0022 modify      //A70LV_Doulas_0011
{
    //eMCT eCH;
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

    eResult = halScaler_BlueGain_Value_Set(ucCH,ucSetting);  //A70LV_Doulas_0012 modify
    if(eResult != eHAL_SCALER_EXEC_CODE_PASS)
        return eResult;

    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[0].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(0);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }

        dvPro_Set_RGB_Gain(eMCT_CH1);    //A70LV_Doulas_0155 //A70LV_Doulas_0072 modify
        dvPro_Set_RGB_Gain(eMCT_CH2);    //A70LV_Doulas_0155

        halScaler_SemaphoreGive(0);
    }
    else
    {
        LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore\r\n", __FUNCTION__, __LINE__);
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }

    return eResult;
}


eHAL_SCALER_EXEC_CODE halScaler_RedOffset_Value_Set(UINT8 ucCH,UINT8 ucSetting)  //A70LV_Doulas_0072 modify//A70LV_Doulas_0022 modify  //A70LV_Doulas_0012
{
    INT16 iSetting;
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    UINT8  ucInputSource = halScaler_InputSource_Get(ucCH);
    UINT8  ucPresetMode = halScaler_PictureSettings_Get(ucCH);
    UINT8  ucSignalType = halScaler_SignalType_Get(ucCH);
	UINT8  ucPresetUserMode = halScaler_PictureSettings_PreUser_Get(ucCH);	//A70LK_Doulas_0004

    if (ucSetting>RED_OFFSET_MAX_VALUE) // || ucSetting<RED_OFFSET_MIN_VALUE )
        return eHAL_SCALER_EXEC_CH_OVER_RANGE;

    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): RedOffset=%d\r\n", __FUNCTION__, __LINE__, ucSetting);

	if (eCM_PICTURE_SETTINGS_USER != ucPresetMode) //A70LK_Doulas_0004 Modify
    	m_sUserSystemSetting.sColorSetting[ucInputSource][ucPresetMode][ucSignalType].ucRedOffset = ucSetting;
	else
    	m_sUserSystemSetting.sUserColorSetting[ucInputSource][ucPresetUserMode][ucSignalType].ucRedOffset = ucSetting;

    if(ucSetting == RED_OFFSET_DEFAULT_VALUE)
    {
        iSetting = PROAV_INPUT_BIAS_R_DEFAULT;
    }
    else
    {
        iSetting = (INT16)(((DOUBLE)ucSetting/(DOUBLE)(RED_OFFSET_MAX_VALUE - RED_OFFSET_MIN_VALUE))*(PROAV_INPUT_BIAS_R_MAX - PROAV_INPUT_BIAS_R_MIN)) + PROAV_INPUT_BIAS_R_MIN;
    }

    dvPro_RedOffset_Value_Set(eMCT_CH1, (INT8)iSetting);     //A70LV_Doulas_0155
    dvPro_RedOffset_Value_Set(eMCT_CH2, (INT8)iSetting);     //A70LV_Doulas_0155

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_GreenOffset_Value_Set(UINT8 ucCH,UINT8 ucSetting)  //A70LV_Doulas_0072 modify//A70LV_Doulas_0022 modify  //A70LV_Doulas_0012
{
    INT16 iSetting;
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    UINT8  ucInputSource = halScaler_InputSource_Get(ucCH);
    UINT8  ucPresetMode = halScaler_PictureSettings_Get(ucCH);
    UINT8  ucSignalType = halScaler_SignalType_Get(ucCH);
	UINT8  ucPresetUserMode = halScaler_PictureSettings_PreUser_Get(ucCH);	//A70LK_Doulas_0004

    if ( ucSetting>GREEN_OFFSET_MAX_VALUE)//|| ucSetting<GREEN_OFFSET_MIN_VALUE )
        return eHAL_SCALER_EXEC_CH_OVER_RANGE;

    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): GreenOffset=%d\r\n", __FUNCTION__, __LINE__, ucSetting);

	if (eCM_PICTURE_SETTINGS_USER != ucPresetMode) //A70LK_Doulas_0004 Modify
    	m_sUserSystemSetting.sColorSetting[ucInputSource][ucPresetMode][ucSignalType].ucGreenOffset = ucSetting;
	else
    	m_sUserSystemSetting.sUserColorSetting[ucInputSource][ucPresetUserMode][ucSignalType].ucGreenOffset = ucSetting;

    if(ucSetting == GREEN_OFFSET_DEFAULT_VALUE)
    {
        iSetting = PROAV_INPUT_BIAS_G_DEFAULT;
    }
    else
    {
        iSetting = (INT16)(((DOUBLE)ucSetting/(DOUBLE)(GREEN_OFFSET_MAX_VALUE - GREEN_OFFSET_MIN_VALUE))*(PROAV_INPUT_BIAS_G_MAX - PROAV_INPUT_BIAS_G_MIN)) + PROAV_INPUT_BIAS_G_MIN;
    }
    dvPro_GreenOffset_Value_Set(eMCT_CH1, (INT8)iSetting);       //A70LV_Doulas_0155
    dvPro_GreenOffset_Value_Set(eMCT_CH2, (INT8)iSetting);       //A70LV_Doulas_0155

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_BlueOffset_Value_Set(UINT8 ucCH,UINT8 ucSetting)  //A70LV_Doulas_0072 modify//A70LV_Doulas_0022 modify  //A70LV_Doulas_0012
{
    INT16 iSetting;
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    UINT8  ucInputSource = halScaler_InputSource_Get(ucCH);
    UINT8  ucPresetMode = halScaler_PictureSettings_Get(ucCH);
    UINT8  ucSignalType = halScaler_SignalType_Get(ucCH);
	UINT8  ucPresetUserMode = halScaler_PictureSettings_PreUser_Get(ucCH);	//A70LK_Doulas_0004

    if ( ucSetting>BLUE_OFFSET_MAX_VALUE)// || ucSetting<BLUE_OFFSET_MIN_VALUE )
        return eHAL_SCALER_EXEC_CH_OVER_RANGE;

    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): BlueOffset=%d\r\n", __FUNCTION__, __LINE__, ucSetting);

	if (eCM_PICTURE_SETTINGS_USER != ucPresetMode) //A70LK_Doulas_0004 Modify
    	m_sUserSystemSetting.sColorSetting[ucInputSource][ucPresetMode][ucSignalType].ucBlueOffset = ucSetting;
	else
    	m_sUserSystemSetting.sUserColorSetting[ucInputSource][ucPresetUserMode][ucSignalType].ucBlueOffset = ucSetting;

    if(ucSetting == BLUE_OFFSET_DEFAULT_VALUE)
    {
        iSetting = PROAV_INPUT_BIAS_B_DEFAULT;
    }
    else
    {
        iSetting = (INT16)(((DOUBLE)ucSetting/(DOUBLE)(BLUE_OFFSET_MAX_VALUE - BLUE_OFFSET_MIN_VALUE))*(PROAV_INPUT_BIAS_B_MAX - PROAV_INPUT_BIAS_B_MIN)) + PROAV_INPUT_BIAS_B_MIN;
    }

    dvPro_BlueOffset_Value_Set(eMCT_CH1, (INT8)iSetting);        //A70LV_Doulas_0155
    dvPro_BlueOffset_Value_Set(eMCT_CH2, (INT8)iSetting);        //A70LV_Doulas_0155

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_RedGain_Value_Set(UINT8 ucCH,UINT8 ucSetting)  //A70LV_Doulas_0072 modify//A70LV_Doulas_0022 modify     //A70LV_Doulas_0012
{
    INT16 iSetting;
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    UINT8  ucInputSource = halScaler_InputSource_Get(ucCH);
    UINT8  ucPresetMode = halScaler_PictureSettings_Get(ucCH);
    UINT8  ucSignalType = halScaler_SignalType_Get(ucCH);
	UINT8  ucPresetUserMode = halScaler_PictureSettings_PreUser_Get(ucCH);	//A70LK_Doulas_0004

    if ( ucSetting>RED_GAIN_MAX_VALUE)// || ucSetting<RED_GAIN_MIN_VALUE )
        return eHAL_SCALER_EXEC_CH_OVER_RANGE;    //A70LV_Doulas_0072 modify

    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): RedGain=%d\r\n", __FUNCTION__, __LINE__, ucSetting);

	if (eCM_PICTURE_SETTINGS_USER != ucPresetMode) //A70LK_Doulas_0004 Modify
    	m_sUserSystemSetting.sColorSetting[ucInputSource][ucPresetMode][ucSignalType].ucRedGain = ucSetting;
	else
    	m_sUserSystemSetting.sUserColorSetting[ucInputSource][ucPresetUserMode][ucSignalType].ucRedGain = ucSetting;

    if(ucSetting == RED_GAIN_DEFAULT_VALUE)
    {
        iSetting = PROAV_INPUT_GAIN_R_DEFAULT;
    }
    else if(ucSetting>RED_GAIN_DEFAULT_VALUE)
    {
        iSetting = (INT16)((DOUBLE)(ucSetting-RED_GAIN_DEFAULT_VALUE))*(PROAV_INPUT_GAIN_R_MAX-PROAV_INPUT_GAIN_R_DEFAULT)/(DOUBLE)(RED_GAIN_MAX_VALUE-RED_GAIN_DEFAULT_VALUE) + PROAV_INPUT_GAIN_R_DEFAULT;
    }
    else
    {
        iSetting = (INT16)((DOUBLE)ucSetting/(DOUBLE)(RED_GAIN_DEFAULT_VALUE-RED_GAIN_MIN_VALUE)*(PROAV_INPUT_GAIN_R_DEFAULT-PROAV_INPUT_GAIN_R_MIN));
    }

    dvPro_RedGain_Value_Set(eMCT_CH1, (UINT8)iSetting);       //A70LV_Doulas_0155
    dvPro_RedGain_Value_Set(eMCT_CH2, (UINT8)iSetting);       //A70LV_Doulas_0155

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_GreenGain_Value_Set(UINT8 ucCH,UINT8 ucSetting)  //A70LV_Doulas_0072 modify//A70LV_Doulas_0022 modify     //A70LV_Doulas_0012
{
    INT16 iSetting;
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    UINT8  ucInputSource = halScaler_InputSource_Get(ucCH);
    UINT8  ucPresetMode = halScaler_PictureSettings_Get(ucCH);
    UINT8  ucSignalType = halScaler_SignalType_Get(ucCH);
	UINT8  ucPresetUserMode = halScaler_PictureSettings_PreUser_Get(ucCH);	//A70LK_Doulas_0004

    if ( ucSetting>GREEN_GAIN_MAX_VALUE)// || ucSetting<GREEN_GAIN_MIN_VALUE )
        return eHAL_SCALER_EXEC_CH_OVER_RANGE;    //A70LV_Doulas_0072 modify

    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): GreenGain=%d\r\n", __FUNCTION__, __LINE__, ucSetting);

	if (eCM_PICTURE_SETTINGS_USER != ucPresetMode) //A70LK_Doulas_0004 Modify
    	m_sUserSystemSetting.sColorSetting[ucInputSource][ucPresetMode][ucSignalType].ucGreenGain = ucSetting;
	else
    	m_sUserSystemSetting.sUserColorSetting[ucInputSource][ucPresetUserMode][ucSignalType].ucGreenGain = ucSetting;

    if(ucSetting == GREEN_GAIN_DEFAULT_VALUE)
    {
        iSetting = PROAV_INPUT_GAIN_G_DEFAULT;
    }
    else if(ucSetting>GREEN_GAIN_DEFAULT_VALUE)
    {
        iSetting = (INT16)((DOUBLE)(ucSetting-GREEN_GAIN_DEFAULT_VALUE))*(PROAV_INPUT_GAIN_G_MAX-PROAV_INPUT_GAIN_G_DEFAULT)/(DOUBLE)(GREEN_GAIN_MAX_VALUE-GREEN_GAIN_DEFAULT_VALUE) + PROAV_INPUT_GAIN_G_DEFAULT;
    }
    else
    {
        iSetting = (INT16)((DOUBLE)ucSetting/(DOUBLE)(GREEN_GAIN_DEFAULT_VALUE-GREEN_GAIN_MIN_VALUE)*(PROAV_INPUT_GAIN_G_DEFAULT-PROAV_INPUT_GAIN_G_MIN));
    }

    dvPro_GreenGain_Value_Set(eMCT_CH1, (UINT8)iSetting);     //A70LV_Doulas_0155
    dvPro_GreenGain_Value_Set(eMCT_CH2, (UINT8)iSetting);     //A70LV_Doulas_0155

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_BlueGain_Value_Set(UINT8 ucCH,UINT8 ucSetting)  //A70LV_Doulas_0072 modify//A70LV_Doulas_0022 modify     //A70LV_Doulas_0012
{
    INT16 iSetting;
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    UINT8  ucInputSource = halScaler_InputSource_Get(ucCH);
    UINT8  ucPresetMode = halScaler_PictureSettings_Get(ucCH);
    UINT8  ucSignalType = halScaler_SignalType_Get(ucCH);
	UINT8  ucPresetUserMode = halScaler_PictureSettings_PreUser_Get(ucCH);	//A70LK_Doulas_0004

    if ( ucSetting>BLUE_GAIN_MAX_VALUE)// || ucSetting<BLUE_GAIN_MIN_VALUE )
        return eHAL_SCALER_EXEC_CH_OVER_RANGE;    //A70LV_Doulas_0072 modify

    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): BlueGain=%d\r\n", __FUNCTION__, __LINE__, ucSetting);

	if (eCM_PICTURE_SETTINGS_USER != ucPresetMode) //A70LK_Doulas_0004 Modify
    	m_sUserSystemSetting.sColorSetting[ucInputSource][ucPresetMode][ucSignalType].ucBlueGain = ucSetting;
	else
    	m_sUserSystemSetting.sUserColorSetting[ucInputSource][ucPresetUserMode][ucSignalType].ucBlueGain = ucSetting;

    if(ucSetting == BLUE_GAIN_DEFAULT_VALUE)
    {
        iSetting = PROAV_INPUT_GAIN_B_DEFAULT;
    }
    else if(ucSetting>BLUE_GAIN_DEFAULT_VALUE)
    {
        iSetting = (INT16)((DOUBLE)(ucSetting-BLUE_GAIN_DEFAULT_VALUE))*(PROAV_INPUT_GAIN_B_MAX-PROAV_INPUT_GAIN_B_DEFAULT)/(DOUBLE)(BLUE_GAIN_MAX_VALUE-BLUE_GAIN_DEFAULT_VALUE) + PROAV_INPUT_GAIN_B_DEFAULT;
    }
    else
    {
        iSetting = (INT16)((DOUBLE)ucSetting/(DOUBLE)(BLUE_GAIN_DEFAULT_VALUE-BLUE_GAIN_MIN_VALUE)*(PROAV_INPUT_GAIN_B_DEFAULT-PROAV_INPUT_GAIN_B_MIN));
    }

    dvPro_BlueGain_Value_Set(eMCT_CH1, (UINT8)iSetting);      //A70LV_Doulas_0155
    dvPro_BlueGain_Value_Set(eMCT_CH2, (UINT8)iSetting);      //A70LV_Doulas_0155

    return eResult;
}


eHAL_SCALER_EXEC_CODE halScaler_Brightness_Value_Set(UINT8 ucCH, UINT8 ucSetting)  //A70LV_Doulas_0072 modify//A70LV_Doulas_0022 modify  //A70LV_Doulas_0012
{
    UINT16 uiSetting;
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    UINT8  ucInputSource = halScaler_InputSource_Get(ucCH);
    UINT8  ucPresetMode = halScaler_PictureSettings_Get(ucCH);
    UINT8  ucSignalType = halScaler_SignalType_Get(ucCH);
	UINT8  ucPresetUserMode = halScaler_PictureSettings_PreUser_Get(ucCH);	//A70LK_Doulas_0004

    if ( ucSetting>BRIGHTNESS_MAX_VALUE)// || ucSetting<BRIGHTNESS_MIN_VALUE )
        return eHAL_SCALER_EXEC_CH_OVER_RANGE;

    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): Brightness=%d\r\n", __FUNCTION__, __LINE__, ucSetting);

	if (eCM_PICTURE_SETTINGS_USER != ucPresetMode) //A70LK_Doulas_0004 Modify
    	m_sUserSystemSetting.sColorSetting[ucInputSource][ucPresetMode][ucSignalType].ucBrightness = ucSetting;
	else
    	m_sUserSystemSetting.sUserColorSetting[ucInputSource][ucPresetUserMode][ucSignalType].ucBrightness = ucSetting;

    if(BRIGHTNESS_DEFAULT_VALUE == ucSetting)
    {
        uiSetting = PROAV_BRIGHTNESS_DEFAULT;
    }
    else
    {
        uiSetting = (UINT16)(((DOUBLE)(PROAV_BRIGHTNESS_MAX - PROAV_BRIGHTNESS_MIN)/(DOUBLE)(BRIGHTNESS_MAX_VALUE - BRIGHTNESS_MIN_VALUE))*(DOUBLE)ucSetting) + PROAV_BRIGHTNESS_MIN;
    }

    //convert range 0~100(default:50) to 0~255(default:128)

    if(m_sUserSystemSetting.sSourceSetting.ucPIPEnable)
    {
        dvPro_Brightness_Value_Set(ucCH, (INT16)uiSetting);
    }
    else
    {
        dvPro_Brightness_Value_Set(eMCT_CH1, (INT16)uiSetting);
        dvPro_Brightness_Value_Set(eMCT_CH2, (INT16)uiSetting);
    }

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_Contrast_Value_Set(UINT8 ucCH, UINT8 ucSetting)     //A70LV_Doulas_0072 modify//A70LV_Doulas_0022
{
    DOUBLE dSetting;
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    UINT8  ucInputSource = halScaler_InputSource_Get(ucCH);
    UINT8  ucPresetMode = halScaler_PictureSettings_Get(ucCH);
    UINT8  ucSignalType = halScaler_SignalType_Get(ucCH);
	UINT8  ucPresetUserMode = halScaler_PictureSettings_PreUser_Get(ucCH);	//A70LK_Doulas_0004

    if ( ucSetting>CONTRAST_MAX_VALUE)// || ucSetting<CONTRAST_MIN_VALUE )
        return eHAL_SCALER_EXEC_CH_OVER_RANGE;    //A70LV_Doulas_0072 modify

    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): Contrast=%d\r\n", __FUNCTION__, __LINE__, ucSetting);

	if (eCM_PICTURE_SETTINGS_USER != ucPresetMode) //A70LK_Doulas_0004 Modify
    	m_sUserSystemSetting.sColorSetting[ucInputSource][ucPresetMode][ucSignalType].ucContrast = ucSetting;
	else
    	m_sUserSystemSetting.sUserColorSetting[ucInputSource][ucPresetUserMode][ucSignalType].ucContrast = ucSetting;

    if(CONTRAST_DEFAULT_VALUE == ucSetting)
    {
        dSetting = PROAV_CONTRAST_DEFAULT;
    }
    else
    {
        dSetting = (DOUBLE)(((DOUBLE)(PROAV_CONTRAST_MAX - PROAV_CONTRAST_MIN)/(DOUBLE)(CONTRAST_MAX_VALUE - CONTRAST_MIN_VALUE))*(DOUBLE)ucSetting) + PROAV_CONTRAST_MIN;
    }

    if(m_sUserSystemSetting.sSourceSetting.ucPIPEnable)
    {
        dvPro_Contrast_Value_Set(ucCH, dSetting);
    }
    else
    {
        dvPro_Contrast_Value_Set(eMCT_CH1, dSetting);
        dvPro_Contrast_Value_Set(eMCT_CH2, dSetting);
    }

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_HUE_Value_Set(UINT8 ucCH,UINT8 ucSetting)  //A70LV_Doulas_0072 modify//A70LV_Doulas_0022
{
    INT16 iSetting;
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    UINT8  ucInputSource = halScaler_InputSource_Get(ucCH);
    UINT8  ucPresetMode = halScaler_PictureSettings_Get(ucCH);
    UINT8  ucSignalType = halScaler_SignalType_Get(ucCH);
	UINT8  ucPresetUserMode = halScaler_PictureSettings_PreUser_Get(ucCH);	//A70LK_Doulas_0004

    if(ucCH >= eMCT_LAST)
            return eHAL_SCALER_EXEC_CODE_FAIL;

    if ( ucSetting>HUE_MAX_VALUE)// || ucSetting<HUE_MIN_VALUE )
        return eHAL_SCALER_EXEC_CH_OVER_RANGE;

    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): HUE=%d\r\n", __FUNCTION__, __LINE__, ucSetting);

	if (eCM_PICTURE_SETTINGS_USER != ucPresetMode) //A70LK_Doulas_0004 Modify
    	m_sUserSystemSetting.sColorSetting[ucInputSource][ucPresetMode][ucSignalType].ucTint = ucSetting;
	else
    	m_sUserSystemSetting.sUserColorSetting[ucInputSource][ucPresetUserMode][ucSignalType].ucTint = ucSetting;

    if(ucSetting == HUE_DEFAULT_VALUE)
    {
        iSetting = PROAV_HUE_DEFAULT;
    }
    else
    {
        iSetting = (INT16)(((INT16)ucSetting/(DOUBLE)(HUE_MAX_VALUE - HUE_MIN_VALUE))*(PROAV_HUE_MAX - PROAV_HUE_MIN)) + PROAV_HUE_MIN;
    }

    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): HUE=%d\r\n", __FUNCTION__, __LINE__, iSetting);
    //convert range 0~100(default:50) to -180~180(default:0)
    dvPro_Set_HUE_Value(eMCT_CH1, iSetting);     //A70LV_Doulas_0155
    dvPro_Set_HUE_Value(eMCT_CH2, iSetting);     //A70LV_Doulas_0155

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_Saturation_Value_Set(UINT8 ucCH,UINT8 ucSetting)  //A70LV_Doulas_0072 modify//A70LV_Doulas_0022
{
    DOUBLE dSetting;

    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    UINT8  ucInputSource = halScaler_InputSource_Get(ucCH);
    UINT8  ucPresetMode = halScaler_PictureSettings_Get(ucCH);
    UINT8  ucSignalType = halScaler_SignalType_Get(ucCH);
	UINT8  ucPresetUserMode = halScaler_PictureSettings_PreUser_Get(ucCH);	//A70LK_Doulas_0004

    if(ucCH >= eMCT_LAST)
            return eHAL_SCALER_EXEC_CODE_FAIL;

    if ( ucSetting> SATURATION_MAX_VALUE)// || ucSetting< SATURATION_MIN_VALUE )
        return eHAL_SCALER_EXEC_CH_OVER_RANGE;    //A70LV_Doulas_0072 modify

    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): Saturation=%d\r\n", __FUNCTION__, __LINE__, ucSetting);

	if (eCM_PICTURE_SETTINGS_USER != ucPresetMode) //A70LK_Doulas_0004 Modify
    	m_sUserSystemSetting.sColorSetting[ucInputSource][ucPresetMode][ucSignalType].ucSaturation = ucSetting;
	else
    	m_sUserSystemSetting.sUserColorSetting[ucInputSource][ucPresetUserMode][ucSignalType].ucSaturation = ucSetting;

    //convert range 0~100(default:50) to 0~3.99(default:1)
    if( ucSetting< SATURATION_DEFAULT_VALUE )
    {
        dSetting = (DOUBLE)ucSetting/(DOUBLE)(SATURATION_DEFAULT_VALUE- SATURATION_MIN_VALUE)*(PROAV_SATURATION_DEFAULT-PROAV_SATURATION_MIN);
    }
    else if (ucSetting> SATURATION_DEFAULT_VALUE)
    {
        dSetting = ((DOUBLE)ucSetting- SATURATION_DEFAULT_VALUE)/(DOUBLE)(SATURATION_MAX_VALUE- SATURATION_DEFAULT_VALUE)*(PROAV_SATURATION_MAX- PROAV_SATURATION_DEFAULT) + PROAV_SATURATION_DEFAULT;
    }
    else
    {
        dSetting = PROAV_SATURATION_DEFAULT;
    }

    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): Saturation=%f\r\n", __FUNCTION__, __LINE__, dSetting);

//#ifdef __ICHIP_CONTROL__
    dvPro_Set_Saturation_Value(eMCT_CH1,dSetting);      //A70LV_Doulas_0155
    dvPro_Set_Saturation_Value(eMCT_CH2,dSetting);      //A70LV_Doulas_0155
//#endif /* __ICHIP_CONTROL__ */

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_Init_EEPROM_Setting(sUSER_SYSTEM_SETTING m_UserSetting)  //A70LV_Doulas_0072 modify //A70LV_Doulas_0016
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d)%d(%d,%d,%d,%d)(%d,%d,%d,%d)(%d,%d,%d,%d)(%d,%d,%d,%d) \r\n", __FUNCTION__, __LINE__,EEPROM_SIZE,
        sizeof(sADC_CAL_VALUES),
        sizeof(sWAVEFORMSTATE),
        sizeof(sBURNIN_INFORMATION),
        sizeof(sSYSTEM_DEFAULT),

        sizeof(sLIGHT_SETTING),
        sizeof(sOSD_SETTING),
        sizeof(sNETWORK_SETTING),
        sizeof(sIMAGE_SETTING),
        sizeof(sWARP_SETTING),
        sizeof(sCOMMON_SETTING),
        sizeof(sSOURCE_SETTING),
        sizeof(sMANUAL_ADJUSTMENT),

        sizeof(sSOURCE_DEPEND_SETTING),
        sizeof(sHSG_SETTING),
        sizeof(sCOLOR_SETTING),
        sizeof(sUSER_SYSTEM_SETTING));  //A70LV_Doulas_0015 Add

    //memcpy(&m_sUserSystemSetting.sImageSetting, &m_UserSetting.sImageSetting, sizeof(m_sUserSystemSetting.sImageSetting));
    //memcpy(&m_sUserSystemSetting.sCommonSetting, &m_UserSetting.sCommonSetting, sizeof(m_sUserSystemSetting.sCommonSetting));
    //memcpy(&m_sUserSystemSetting.sSourceSetting, &m_UserSetting.sSourceSetting, sizeof(m_sUserSystemSetting.sSourceSetting));
    //memcpy(&m_sUserSystemSetting.sSourceDependSetting, &m_UserSetting.sSourceDependSetting, sizeof(m_sUserSystemSetting.sSourceDependSetting));
    //memcpy(&m_sUserSystemSetting.sColorSetting, &m_UserSetting.sColorSetting, sizeof(m_sUserSystemSetting.sColorSetting));
    //memcpy(&m_sUserSystemSetting.sHSG_setting, &m_UserSetting.sHSG_setting, sizeof(m_sUserSystemSetting.sHSG_setting));
	memcpy(&m_sUserSystemSetting, &m_UserSetting, sizeof(sUSER_SYSTEM_SETTING));	//A70LK_Doulas_0004 Modify


/*    LOG_MSG(db_HAL_SCALER, "(%d,%d)(%d,%d)(%d,%d)%d \r\n",m_sUserSystemSetting.sCommonSetting.ucLanguage,
                m_sUserSystemSetting.sCommonSetting.ucStandbyPowerSave,
                m_sUserSystemSetting.sCommonSetting.ucSerialPortBaudRate,
                m_sUserSystemSetting.sCommonSetting.ucIRControl_Top,
                m_sUserSystemSetting.sCommonSetting.ucIRControl_Front,
                m_sUserSystemSetting.sCommonSetting.ucIRControl_HDBaseT,
                m_sUserSystemSetting.sCommonSetting.ucCOMMON_SETTING_CRC);  //A70LV_Doulas_0017 test
*/
    return eResult;
}

SemaphoreHandle_t halScaler_xSemaphore_Get(void)  //A70LV_Doulas_0014
{
#if (SYSTEM_OS_TYPE == POSIX_COMPLIANT)
    return m_sHalScalerInfo[eMCT_CH1].xSemaphore;
#else
    #if(CURRENT_RTOS_TYPE == RTOS_STATIC)
        return m_sHalScalerInfo[eMCT_CH1].xSemaphore;
    #else
        return m_sHalScalerInfo[eMCT_CH1].xSemaphore;
    #endif
#endif
}

eHAL_SCALER_EXEC_CODE halScaler_SizePresets_Set(UINT8 ucSetting) //A70LV_Doulas_0020
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    ePROAV_EXEC_CODE eC821Resut;
    UINT8 ucInputSource;

    if ( ucSetting >= eCM_SCALING_MODE_NUMBER)// || ucSetting < eCM_SCALING_MODE_AUTO )
        return eHAL_SCALER_EXEC_CH_OVER_RANGE;    //A70LV_Doulas_0072 modify

    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): OverScan=%d\r\n", __FUNCTION__, __LINE__, ucSetting);

    ucInputSource = halScaler_InputSource_Get(eMCT_CH1);
    m_sUserSystemSetting.sSourceDependSetting[ucInputSource].ucAspectRatio = ucSetting;

    dvPro_Set_SizePresets_Value(eMCT_CH2,ucSetting);      //A70LV_Doulas_0155 Add
    eC821Resut = dvPro_Set_SizePresets_Value(eMCT_CH1,ucSetting);
    if(eC821Resut != ePROAV_EXEC_CODE_PASS)
    {
        eResult = eHAL_SCALER_EXEC_CODE_FAIL;
    }

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_SizePresets_SetOnlyValue(UINT8 ucSetting) //A70LK_Doulas_0015 Add
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    ePROAV_EXEC_CODE eC821Resut;
    UINT8 ucInputSource;

    if ( ucSetting >= eCM_SCALING_MODE_NUMBER)// || ucSetting < eCM_SCALING_MODE_AUTO )
        return eHAL_SCALER_EXEC_CH_OVER_RANGE;

    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): halScaler_SizePresets_SetOnlyValue=%d\r\n", __FUNCTION__, __LINE__, ucSetting);

    ucInputSource = halScaler_InputSource_Get(eMCT_CH1);
    m_sUserSystemSetting.sSourceDependSetting[ucInputSource].ucAspectRatio = ucSetting;

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_Overscan_Set(UINT8 ucSetting)    //A70LV_Doulas_0072 modify
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    UINT8 ucInputSource;

    if ( ucSetting > eOVER_SCAN_CROP)// || ucSetting < eOVER_SCAN_OFF )
        return eHAL_SCALER_EXEC_CH_OVER_RANGE;    //A70LV_Doulas_0072 modify

    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): OverScan=%d\r\n", __FUNCTION__, __LINE__, ucSetting);

    ucInputSource = halScaler_InputSource_Get(eMCT_CH1);
    m_sUserSystemSetting.sSourceDependSetting[ucInputSource].ucOverScan = ucSetting;

#ifdef __ICHIP_CONTROL__
    dvPro_Overscan_Set(eMCT_CH2,(eOVER_SCAN_TYPE)ucSetting);      //A70LV_Doulas_0155
    eC821Resut = dvPro_Overscan_Set(eMCT_CH1,(eOVER_SCAN_TYPE)ucSetting);
    if(eC821Resut != ePROAV_EXEC_CODE_PASS)
    {
        eResult = eHAL_SCALER_EXEC_CODE_FAIL;
    }
#endif /* __ICHIP_CONTROL__ */

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_PixelTrack_Set(UINT8 ucSetting)     //A70LV_Doulas_0072 modify
{
    return eHAL_SCALER_EXEC_CODE_FAIL;
}

eHAL_SCALER_EXEC_CODE halScaler_PixelPhase_Set(UINT8 ucSetting)     //A70LV_Doulas_0116 modify //A70LV_Doulas_0072 modify
{
    UINT8 ucInputSource = halScaler_InputSource_Get(eMCT_CH1);
    //UINT8 ucPhase;
    m_sUserSystemSetting.sSourceDependSetting[ucInputSource].ucPhase = ucSetting;

    return eHAL_SCALER_EXEC_CODE_PASS;
}

eHAL_SCALER_EXEC_CODE halScaler_PixelPhase_Action(void)     //A70LV_Doulas_0117
{
    UINT8 ucPhase;
    UINT8 ucSetting = halScaler_OSD_PixelPhase_Get();

    ucPhase = (UINT8)(((UINT16)ucSetting - PHASE_DEFAULT_VALUE + (UINT16)dvPro_BsetPhaseGet(eMCT_CH1) + 64)%32);

#ifdef __ICHIP_CONTROL__
    halFrontEndCtrl_VGA_Phase_Set(&ucPhase);
#endif /* __ICHIP_CONTROL__ */
    return eHAL_SCALER_EXEC_CODE_PASS;
}

UINT8 halScaler_OSD_PixelPhase_Get(void)     //A70LV_Doulas_0116
{
    UINT8 ucInputSource = halScaler_InputSource_Get(eMCT_CH1);

    return m_sUserSystemSetting.sSourceDependSetting[ucInputSource].ucPhase;
}

eHAL_SCALER_EXEC_CODE halScaler_HorzPosition_Set(UINT8 ucSetting)    //A70LV_Doulas_0072 modify
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    ePROAV_EXEC_CODE eProAVResut;
    UINT8 ucInputSource;

    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): Horz_Start_Position=%d\r\n", __FUNCTION__, __LINE__, ucSetting);

    ucInputSource = halScaler_InputSource_Get(eMCT_CH1);
    m_sUserSystemSetting.sSourceDependSetting[ucInputSource].ucHorzPosition = ucSetting;     //A70LV_Doulas_0029

    dvPro_Horz_Start_Position_Set(eMCT_CH2,ucSetting);     //A70LV_Doulas_0155
    eProAVResut = dvPro_Horz_Start_Position_Set(eMCT_CH1,ucSetting);
    if(eProAVResut != ePROAV_EXEC_CODE_PASS)
    {
        eResult = eHAL_SCALER_EXEC_CODE_FAIL;
    }

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_VertPosition_Set(UINT8 ucSetting)    //A70LV_Doulas_0072 modify
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    ePROAV_EXEC_CODE eProAVResut;
    UINT8 ucInputSource;

    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): Vert_Start_Position=%d\r\n", __FUNCTION__, __LINE__, ucSetting);

    ucInputSource = halScaler_InputSource_Get(eMCT_CH1);
    m_sUserSystemSetting.sSourceDependSetting[ucInputSource].ucVertPosition = ucSetting; //A70LV_Doulas_0029

    dvPro_Vert_Start_Position_Set(eMCT_CH2,ucSetting);     //A70LV_Doulas_0155
    eProAVResut = dvPro_Vert_Start_Position_Set(eMCT_CH1,ucSetting);
    if(eProAVResut != ePROAV_EXEC_CODE_PASS)
    {
        eResult = eHAL_SCALER_EXEC_CODE_FAIL;
    }

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_DigitalHorzZoom_Set(UINT16 uiSetting)    //A70LV_Doulas_0072 modify
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    ePROAV_EXEC_CODE eScalerResut;
    UINT8 ucInputSource;
    UINT8 ucSizePresets;

    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): Digital_Horz_Zoom=%d\r\n", __FUNCTION__, __LINE__, uiSetting);

    ucInputSource = halScaler_InputSource_Get(eMCT_CH1);
    ucSizePresets = halScaler_SizePresets_Get(eMCT_CH1);

	if(ucSizePresets == eCM_SCALING_MODE_CUSTOM)	//A70LK_Doulas_0015 Modify
		m_sUserSystemSetting.sSourceDependSetting[ucInputSource].uiCustomDigitalHorzZoom = uiSetting;
	else
 		m_sUserSystemSetting.sSourceDependSetting[ucInputSource].uiDigitalHorzZoom = uiSetting;

    dvPro_Digital_Horz_Zoom_Set(eMCT_CH2,uiSetting);    //A70LV_Doulas_0155
    eScalerResut = dvPro_Digital_Horz_Zoom_Set(eMCT_CH1,uiSetting);
    if(eScalerResut != ePROAV_EXEC_CODE_PASS)
    {
        eResult = eHAL_SCALER_EXEC_CODE_FAIL;
    }

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_DigitalVertZoom_Set(UINT16 uiSetting)   //A70LV_Doulas_0072 modify
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    ePROAV_EXEC_CODE eScalerResut;
    UINT8 ucInputSource;
    UINT8 ucSizePresets;

    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): Digital_Vert_Zoom=%d\r\n", __FUNCTION__, __LINE__, uiSetting);

    ucInputSource = halScaler_InputSource_Get(eMCT_CH1);
    ucSizePresets = halScaler_SizePresets_Get(eMCT_CH1);

	if(ucSizePresets == eCM_SCALING_MODE_CUSTOM)	//A70LK_Doulas_0015 Modify
		m_sUserSystemSetting.sSourceDependSetting[ucInputSource].uiCustomDigitalVertZoom = uiSetting;
	else
 		m_sUserSystemSetting.sSourceDependSetting[ucInputSource].uiDigitalVertZoom = uiSetting;

    dvPro_Digital_Vert_Zoom_Set(eMCT_CH2,uiSetting);    //A70LV_Doulas_0155
    eScalerResut = dvPro_Digital_Vert_Zoom_Set(eMCT_CH1,uiSetting);
    if(eScalerResut != ePROAV_EXEC_CODE_PASS)
    {
        eResult = eHAL_SCALER_EXEC_CODE_FAIL;
    }

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_DigitalHorzShift_Set(UINT8 ucSetting)    //A70LV_Doulas_0072 modify
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    ePROAV_EXEC_CODE eScalerResut;
    UINT8 ucInputSource;
    UINT8 ucSizePresets;

    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): Digital_Horz_Shift=%d\r\n", __FUNCTION__, __LINE__, ucSetting);

    ucInputSource = halScaler_InputSource_Get(eMCT_CH1);
    ucSizePresets = halScaler_SizePresets_Get(eMCT_CH1);

	if(ucSizePresets == eCM_SCALING_MODE_CUSTOM)	//A70LK_Doulas_0015 Modify
		m_sUserSystemSetting.sSourceDependSetting[ucInputSource].uiCustomDigitalHorzShift = ucSetting;
	else
		m_sUserSystemSetting.sSourceDependSetting[ucInputSource].uiDigitalHorzShift = ucSetting;

    dvPro_Digital_Horz_Shift_Set(eMCT_CH2,ucSetting);    //A70LV_Doulas_0155
    eScalerResut = dvPro_Digital_Horz_Shift_Set(eMCT_CH1,ucSetting);
    if(eScalerResut != ePROAV_EXEC_CODE_PASS)
    {
        eResult = eHAL_SCALER_EXEC_CODE_FAIL;
    }

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_DigitalVertShift_Set(UINT8 ucSetting)    //A70LV_Doulas_0072 modify
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    ePROAV_EXEC_CODE eScalerResut;
    UINT8 ucInputSource;
    UINT8 ucSizePresets;

    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): Digital_Vert_Shift=%d\r\n", __FUNCTION__, __LINE__, ucSetting);

    ucInputSource = halScaler_InputSource_Get(eMCT_CH1);
    ucSizePresets = halScaler_SizePresets_Get(eMCT_CH1);

	if(ucSizePresets == eCM_SCALING_MODE_CUSTOM)	//A70LK_Doulas_0015 Modify
		m_sUserSystemSetting.sSourceDependSetting[ucInputSource].uiCustomDigitalVertShift = ucSetting;
	else
   		m_sUserSystemSetting.sSourceDependSetting[ucInputSource].uiDigitalVertShift = ucSetting;

    dvPro_Digital_Vert_Shift_Set(eMCT_CH2,ucSetting);    //A70LV_Doulas_0155
    eScalerResut = dvPro_Digital_Vert_Shift_Set(eMCT_CH1,ucSetting);
    if(eScalerResut != ePROAV_EXEC_CODE_PASS)
    {
        eResult = eHAL_SCALER_EXEC_CODE_FAIL;
    }

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_AutoImage_Set(UINT8 ucSetting)   //A70LV_Doulas_0112 modify
{
    UINT8 ucInputSource = halScaler_InputSource_Get(eMCT_CH1);
    m_sUserSystemSetting.sSourceDependSetting[ucInputSource].ucAutoImage = ucSetting;
#ifdef __ICHIP_CONTROL__
    dvPro_AutoImage_Set(ucInputSource,ucSetting);
#endif /* __ICHIP_CONTROL__ */
    return eHAL_SCALER_EXEC_CODE_PASS;
}

UINT8 halScaler_InputSource_Get(UINT8 ucCH)
{
    UINT8 ucInputSource;

    if(ucCH == eMCT_CH1)
        ucInputSource = m_sUserSystemSetting.sSourceSetting.ucInputSourceMain;
    else
        ucInputSource = m_sUserSystemSetting.sSourceSetting.ucInputSourceSub;  //A70LV_Doulas_0024
    return ucInputSource;
}

UINT8 halScaler_SizePresets_Get(UINT8 ucCH)
{
    UINT8 ucInputSource;
    ucInputSource = halScaler_InputSource_Get(ucCH);
    return (UINT8)m_sUserSystemSetting.sSourceDependSetting[ucInputSource].ucAspectRatio;
}

UINT8 halScaler_PictureSettings_Get(UINT8 ucCH)     //A70LV_Doulas_0022
{
    UINT8 ucInputSource = halScaler_InputSource_Get(ucCH);

    return m_sUserSystemSetting.sSourceDependSetting[ucInputSource].ucPresetMode;
}

UINT8 halScaler_PictureSettings_PreUser_Get(UINT8 ucCH)			  //A70LK_Doulas_0004
{
    UINT8 ucInputSource = halScaler_InputSource_Get(ucCH);

    return  m_sUserSystemSetting.sSourceDependSetting[ucInputSource].ucPreUserMode;
}

UINT8 halScaler_SignalType_Get(UINT8 ucCH)
{
    UINT8 ucSignalType = 0;

    return ucSignalType;
}

eHAL_SCALER_EXEC_CODE halScaler_ColorSpace_Set(UINT8 ucCH,UINT8 ucSetting)   //A70LV_Doulas_0072 modify//A70LV_Doulas_0069 modify
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    UINT8  ucInputSource = halScaler_InputSource_Get(ucCH);
    UINT8  ucPresetMode = halScaler_PictureSettings_Get(ucCH);
    UINT8  ucSignalType = halScaler_SignalType_Get(ucCH);
	UINT8  ucPresetUserMode = halScaler_PictureSettings_PreUser_Get(ucCH);	//A70LK_Doulas_0004
    UINT8  ucInputSub = halScaler_InputSource_Get(eMCT_CH2); 	//G100_Steven_0079

    if(ucSetting >= eCM_COLOR_SPACE_NUMBER)
        return eHAL_SCALER_EXEC_CH_OVER_RANGE;

    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): [%d] ucInputSource = %d, ColorSpace Set=%d\r\n", __FUNCTION__, __LINE__, ucCH, ucInputSource, ucSetting);

    if((m_sUserSystemSetting.sSourceSetting.ucPIPEnable == ets_ON) && (ucCH == eMCT_CH2))
    {
    	if (eCM_PICTURE_SETTINGS_USER != ucPresetMode) //G100_Steven_0034
    		m_sUserSystemSetting.sColorSetting[ucInputSub][ucPresetMode][ucSignalType].ucCS = ucSetting;
    	else
    		m_sUserSystemSetting.sUserColorSetting[ucInputSub][ucPresetUserMode][ucSignalType].ucCS = ucSetting;
    }
    else if(ucCH == eMCT_CH1)
    {
    	if (eCM_PICTURE_SETTINGS_USER != ucPresetMode) //A70LK_Doulas_0004 Modify
        	m_sUserSystemSetting.sColorSetting[ucInputSource][ucPresetMode][ucSignalType].ucCS = ucSetting;
    	else
    		m_sUserSystemSetting.sUserColorSetting[ucInputSource][ucPresetUserMode][ucSignalType].ucCS = ucSetting;
    }

    return eResult;
}

UINT8 halScaler_ColorSpace_Get(UINT8 ucCH)   //A70LK_Doulas_0004 Modify//A70LV_Doulas_0069
{
    //UINT8  ucInputSource = halScaler_InputSource_Get(ucCH);
    //UINT8  ucPresetMode = halScaler_PictureSettings_Get(ucCH);
    //UINT8  ucSignalType = halScaler_SignalType_Get(ucCH);
    UINT8  ucSetting;

    //LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): [%d] ucInputSource = %d, ColorSpace Get=%d\r\n", __FUNCTION__, __LINE__, ucCH, ucInputSource, ucSetting);

    //ucSetting = m_sUserSystemSetting.sColorSetting[ucInputSource][ucPresetMode][ucSignalType].ucCS;
	ucSetting = halScaler_ColorSetting_Get(eHAL_COLOR_SETTING_CS, ucCH);

    return ucSetting;
}

UINT8 halScaler_ColorSpace_ScalerValueGet(UINT8 ucCH)   //A70LV_Doulas_0220 modify//A70LV_Doulas_0109 modify //A70LV_Doulas_0069
{
    UINT8  ucSetting = halScaler_ColorSpace_Get(eMCT_CH1);

#if 0
    if((m_sUserSystemSetting.sSourceSetting.ucPIPEnable != ets_OFF) && (ucCH == eMCT_CH2))
    {
        ucSetting = eCM_COLOR_SPACE_AUTO;
    }
#endif

    switch(ucSetting)
    {
        case eCM_COLOR_SPACE_AUTO:
            dvPro_VideoYUV_Set((eMCT)ucCH,m_sProAV_Info.ucVideoYUV[ucCH]);
            ucSetting = m_sProAV_Info.ucColorSpace[ucCH];     //A70LV_Doulas_0076 modify
            break;

        case eCM_COLOR_SPACE_RGB_FULL:
            dvPro_VideoYUV_Set((eMCT)ucCH,eVIDEO_YUV_UNKNOW);
            ucSetting = eCOLOR_FORMAT_RGB;
            break;

        case eCM_COLOR_SPACE_RGB_LIMITED:
            dvPro_VideoYUV_Set((eMCT)ucCH,eVIDEO_YUV_UNKNOW);
            ucSetting = eCOLOR_FORMAT_RGB_LIMIT;
            break;

        case eCM_COLOR_SPACE_YUV_REC709:
            dvPro_VideoYUV_Set((eMCT)ucCH,eVIDEO_YUV_REC709);
            if(m_sProAV_Info.ucColorSpace[ucCH] == eCOLOR_FORMAT_422)     //A70LV_Doulas_0076 modify
                ucSetting = eCOLOR_FORMAT_422;
            else
                ucSetting = eCOLOR_FORMAT_444;
            break;

        case eCM_COLOR_SPACE_YUV_REC601:
            dvPro_VideoYUV_Set((eMCT)ucCH,eVIDEO_YUV_REC601);
            if(m_sProAV_Info.ucColorSpace[ucCH] == eCOLOR_FORMAT_422)     //A70LV_Doulas_0076 modify
                ucSetting = eCOLOR_FORMAT_422;
            else
                ucSetting = eCOLOR_FORMAT_444;
            break;

        default:
            dvPro_VideoYUV_Set((eMCT)ucCH,m_sProAV_Info.ucVideoYUV[ucCH]);
            ucSetting = eCOLOR_FORMAT_RGB;
            break;
    }
    return ucSetting;
}

eHAL_SCALER_EXEC_CODE halScaler_Detail_Set(UINT8 ucCH,UINT8 ucSetting)   //A70LV_Doulas_0029 modify
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

    eResult = halScaler_Detail_Value_Set(ucCH,ucSetting);
    if(eResult != eHAL_SCALER_EXEC_CODE_PASS)
        return eResult;

    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[0].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(0);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }

        dvPro_Set_Sharpness(eMCT_CH1);
        dvPro_Set_Sharpness(eMCT_CH2);

        halScaler_SemaphoreGive(0);
    }
    else
    {
        LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore\r\n", __FUNCTION__, __LINE__);
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }

    return eResult;
}

INT8 halScaler_3DEnable_Set(UINT8 ucSetting)        //A70LV_Doulas_0154 modify
{
    UINT8  ucInputSource = halScaler_InputSource_Get(eMCT_CH1);
    m_sUserSystemSetting.sSourceDependSetting[ucInputSource].uc3D_Enable = ucSetting;
    dvPro_OSD_3D_Enable_Set(eMCT_CH1,ucSetting);
    dvPro_OSD_3D_Enable_Set(eMCT_CH2,ucSetting);       //A70LV_Doulas_0155

    return eHAL_SCALER_EXEC_CODE_PASS;
}

UINT8 halScaler_3DEnable_Get(void)
{
    UINT8  ucInputSource = halScaler_InputSource_Get(eMCT_CH1);

    return m_sUserSystemSetting.sSourceDependSetting[ucInputSource].uc3D_Enable;
}

INT8 halScaler_3DMode_Set(UINT8 ucSetting)
{
    UINT8  ucInputSource = halScaler_InputSource_Get(eMCT_CH1);
    m_sUserSystemSetting.sSourceDependSetting[ucInputSource].uc3D_Mode = ucSetting;
    //dvC821_OSD_3D_Mode_Set(eMCT_CH1,ucSetting);
    //dvC821_OSD_3D_Mode_Set(eMCT_CH2,ucSetting);

    return eHAL_SCALER_EXEC_CODE_PASS;
}

UINT8 halScaler_3DMode_Get(void)
{
    UINT8  ucInputSource = halScaler_InputSource_Get(eMCT_CH1);
    return m_sUserSystemSetting.sSourceDependSetting[ucInputSource].uc3D_Mode;
}

INT8 halScaler_3DInvert_Set(UINT8 ucSetting)
{
    UINT8  ucInputSource = halScaler_InputSource_Get(eMCT_CH1);
    m_sUserSystemSetting.sSourceDependSetting[ucInputSource].uc3D_Invert = ucSetting;

    dvPro_OSD_3D_InvertSet(eMCT_CH1, ucSetting);
    dvPro_OSD_3D_InvertSet(eMCT_CH2, ucSetting);

    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[0].bScalerInit)
        {
            halScaler_SemaphoreGive(0);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }

        if(halScaler_3DSyncOut_Get(eMCT_CH1) == eCM_3D_SYNC_OUT_TO_EMITTER)
        {
            dvProAV_SclIntSyncPolSet((bool)ucSetting);
            dvProAV_SclExtSyncPolSet((bool)ucSetting);
        }

        halScaler_SemaphoreGive(0);
    }
    else
    {
        LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore\r\n", __FUNCTION__, __LINE__);
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }
    return HAL_SCALER_PASS;
}

INT8 halScaler_3DSyncOut_Set(UINT8 ucSetting)
{
    UINT8  ucInputSource = halScaler_InputSource_Get(eMCT_CH1);
    m_sUserSystemSetting.sSourceDependSetting[ucInputSource].uc3D_SyncOut = ucSetting;

    m_sHalScalerInfo[eMCT_CH1].ucExt3DSyncCount = 5;
    m_sHalScalerInfo[eMCT_CH1].ucExt3DSyncSetting = 0xFF;

    dvPro_OSD_3D_Sync_OutSet(eMCT_CH1, ucSetting);
    dvPro_OSD_3D_Sync_OutSet(eMCT_CH2, ucSetting);

#if 1
    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[0].bScalerInit)
        {
            halScaler_SemaphoreGive(0);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }

        dvPro_OSD_3D_Sync_Setting(m_sUserSystemSetting.sSourceDependSetting[eMCT_CH1].uc3D_Enable);

        halScaler_SemaphoreGive(0);
    }
    else
    {
        LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore\r\n", __FUNCTION__, __LINE__);
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }
#endif

    return HAL_SCALER_PASS;
}

UINT8 halScaler_3DSyncOut_Get(UINT8 eCH)
{
    UINT8  ucInputSource = halScaler_InputSource_Get(eCH);
    return m_sUserSystemSetting.sSourceDependSetting[ucInputSource].uc3D_SyncOut;
}


INT8 halScaler_3DSyncIn_Set(UINT8 ucSetting)
{
    UINT8  ucInputSource = halScaler_InputSource_Get(eMCT_CH1);
    m_sUserSystemSetting.sSourceDependSetting[ucInputSource].uc3D_SyncIn = ucSetting;

    m_sHalScalerInfo[eMCT_CH1].ucExt3DSyncCount = 5;
    m_sHalScalerInfo[eMCT_CH1].ucExt3DSyncSetting = 0xFF;

    dvPro_OSD_3D_Sync_InSet(eMCT_CH1,ucSetting);
    dvPro_OSD_3D_Sync_InSet(eMCT_CH2,ucSetting);

#if 1
    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[0].bScalerInit)
        {
            halScaler_SemaphoreGive(0);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }

        dvPro_OSD_3D_Sync_Setting(m_sUserSystemSetting.sSourceDependSetting[eMCT_CH1].uc3D_Enable);

        halScaler_SemaphoreGive(0);
    }
    else
    {
        LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore\r\n", __FUNCTION__, __LINE__);
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }
#endif

    return HAL_SCALER_PASS;
}

UINT8 halScaler_3DSyncIn_Get(UINT8 eCH)
{
    UINT8  ucInputSource = halScaler_InputSource_Get(eCH);

    return m_sUserSystemSetting.sSourceDependSetting[ucInputSource].uc3D_SyncIn;
}

eHAL_SCALER_EXEC_CODE halScaler_FrameDelay_Set(UINT16 uiSetting)     //A70LV_Doulas_0154 modify
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[0].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(0);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }

        //LOG_MSG(db_ALWAYS, "halScaler_FrameDelay_Set %d\n", uiSetting);

        //dvProAV_Scl3dIntSynSw(1);
        dvProAV_Scl3dExtSyncDelaySet(0);//for 3D glass sync delay reg
        dvProAV_SclFrameSyncDelaySet((UINT8)uiSetting); //for frame delay reg
        //dvProAV_Scl3dLRDelaySet((UINT8)uiSetting);

#ifdef __ICHIP_CONTROL__
        halC789Ctrl_FrmaeDelaySet((UINT8)uiSetting);
#endif /* __ICHIP_CONTROL__ */

        halScaler_SemaphoreGive(0);
    }
    else
    {
        LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore\r\n", __FUNCTION__, __LINE__);
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_3DSyncDelay_Set(UINT16 uiSetting)
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[0].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(0);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }

        //LOG_MSG(db_ALWAYS, "halScaler_FrameDelay_Set %d\n", uiSetting);

        //dvProAV_Scl3dIntSynSw(1);
       // dvProAV_Scl3dExtSyncDelaySet(0);//for 3D glass sync delay reg
        //dvProAV_SclFrameSyncDelaySet(0); //for frame delay reg
        dvProAV_Scl3dLRDelaySet((UINT8)uiSetting);

#ifdef __ICHIP_CONTROL__
        halC789Ctrl_FrmaeDelaySet((UINT8)uiSetting);
#endif /* __ICHIP_CONTROL__ */

        halScaler_SemaphoreGive(0);
    }
    else
    {
        LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore\r\n", __FUNCTION__, __LINE__);
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_SkinColor_Set(UINT8 ucCH,UINT8 ucSetting)    //A70LV_Doulas_0072 modify//A70LV_Doulas_0029
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

    eResult = halScaler_SkinColor_Value_Set(ucCH,ucSetting);
    if(eResult != eHAL_SCALER_EXEC_CODE_PASS)
        return eResult;

    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[0].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(0);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }

#ifdef __ICHIP_CONTROL__
        dvPro_SkinColor_Set((eMCT)ucCH);
#endif /* __ICHIP_CONTROL__ */

        halScaler_SemaphoreGive(0);
    }
    else
    {
        LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore\r\n", __FUNCTION__, __LINE__);
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_TemporalNoiseReduction_Set(UINT8 ucSetting)  //A70LV_Doulas_0072 modify//A70LV_Doulas_0029
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

    eResult = halScaler_Temporal_NR_Value_Set(ucSetting);
    if(eResult != eHAL_SCALER_EXEC_CODE_PASS)
        return eResult;

    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[0].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(0);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }

#ifdef __ICHIP_CONTROL__
        dvPro_BuildPicture_2K(eMCT_CH1);
        if(m_sUserSystemSetting.sSourceSetting.ucPIPEnable)     //A70LV_Doulas_0080
        {
            dvPro_BuildPicture_2K(eMCT_CH2);
        }
        else if(dvPro_PanelGet() == ePANEL_ID_1080P_120HZ)     //A70LV_Doulas_0155
        {
            dvPro_BuildPicture_2K(eMCT_CH2);
        }
#endif /* __ICHIP_CONTROL__ */

        halScaler_SemaphoreGive(0);
    }
    else
    {
        LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore\r\n", __FUNCTION__, __LINE__);
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }
    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_MPEGNoiseReduction_Set(UINT8 ucSetting)  //A70LV_Doulas_0072 modify//A70LV_Doulas_0029
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

    eResult = halScaler_MPEG_NR_Value_Set(ucSetting);
    if(eResult != eHAL_SCALER_EXEC_CODE_PASS)
        return eResult;

    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[0].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(0);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }

#ifdef __ICHIP_CONTROL__
        dvPro_BuildPicture_2K(eMCT_CH1);
        if(m_sUserSystemSetting.sSourceSetting.ucPIPEnable)     //A70LV_Doulas_0080
        {
            dvPro_BuildPicture_2K(eMCT_CH2);
        }
        else if(dvPro_PanelGet() == ePANEL_ID_1080P_120HZ)     //A70LV_Doulas_0155
        {
            dvPro_BuildPicture_2K(eMCT_CH2);
        }
#endif /* __ICHIP_CONTROL__ */

        halScaler_SemaphoreGive(0);
    }
    else
    {
        LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore\r\n", __FUNCTION__, __LINE__);
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }
    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_DetectFilm_Set(UINT8 ucSetting)  //A70LV_Doulas_0072 modify//A70LV_Doulas_0029
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

    eResult = halScaler_DetectFilm_Value_Set(ucSetting);
    if(eResult != eHAL_SCALER_EXEC_CODE_PASS)
        return eResult;

    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[0].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(0);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }

#ifdef __ICHIP_CONTROL__
        dvPro_BuildPicture_2K(eMCT_CH1);
        dvPro_BuildPicture_2K(eMCT_CH2);
#endif /* __ICHIP_CONTROL__ */

        halScaler_SemaphoreGive(0);
    }
    else
    {
        LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore\r\n", __FUNCTION__, __LINE__);
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }
    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_RGB_Gain_Offset_Set(void)    //A70LV_Doulas_0072 modify//A70LV_Doulas_0049 modify
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    UINT8  ucInputSource = halScaler_InputSource_Get(eMCT_CH1);
    UINT8  ucPresetMode = halScaler_PictureSettings_Get(eMCT_CH1);
    UINT8  ucSignalType = halScaler_SignalType_Get(eMCT_CH1);

    halScaler_IintChannelSetting(eMCT_CH1);
    halScaler_IintChannelSetting(eMCT_CH2);
    eResult = halScaler_RedOffset_Set(eMCT_CH1,m_sUserSystemSetting.sColorSetting[ucInputSource][ucPresetMode][ucSignalType].ucRedOffset);
    eResult |= halScaler_RedGain_Set(eMCT_CH1,m_sUserSystemSetting.sColorSetting[ucInputSource][ucPresetMode][ucSignalType].ucRedGain);
    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_PictureSettings_Set(UINT8 ucCH,UINT8 ucSetting)      //A70LV_Doulas_0072 modify//A70LV_Doulas_0030 modify
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

    eResult = halScaler_PictureSettings_Value_Set(ucCH,ucSetting);
    if(eResult != eHAL_SCALER_EXEC_CODE_PASS)
        return eResult;

    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[0].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(0);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }

        //execute Contrast action function
        halScaler_IintChannelSetting(ucCH);

        halScaler_SemaphoreGive(0);
    }
    else
    {
        LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore\r\n", __FUNCTION__, __LINE__);
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }

    return eResult;
}

void halScaler_PreUserMode_Set(UINT8 ucSetting)
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    UINT8  ucInputSource = halScaler_InputSource_Get(eMCT_CH1);
    UINT8  ucPresetMode = halScaler_PictureSettings_Get(eMCT_CH1);
    UINT8  ucSignalType = halScaler_SignalType_Get(eMCT_CH1);

    if ( ucSetting >= eCM_PICTURE_SETTINGS_NUMBER )
    {
		//do nothing
	}
    else
    {
    	m_sUserSystemSetting.sSourceDependSetting[ucInputSource].ucPreUserMode = ucSetting;
    }

}

eHAL_SCALER_EXEC_CODE halScaler_ContrastEnhancement_Set(UINT8 ucSetting)     //A70LV_Doulas_0072 modify//A70LV_Doulas_0046 modify
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

    if ( ucSetting > eCONTRAST_ENHANCE_REAL_BLACK)// || ucSetting < eCONTRAST_ENHANCE_OFF )
        return eHAL_SCALER_EXEC_CH_OVER_RANGE;

    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): ContrastEnhancement =%d\r\n", __FUNCTION__, __LINE__, ucSetting);

    m_sUserSystemSetting.sImageSetting.ucContrastEnhancement = ucSetting;

    return eResult;
}

INT8 halScaler_HDREnable_Set(UINT8 ucSetting)
{
    m_sUserSystemSetting.sImageSetting.ucHDREnable = ucSetting;

    return HAL_SCALER_PASS;
}

INT8 halScaler_MEMC_Set(UINT8 ucSetting)
{
    return HAL_SCALER_PASS;
}

eHAL_SCALER_EXEC_CODE halScaler_ImageFreeze_Set(UINT8 ucSetting) //A70LV_Doulas_0072 modify
{
    halScaler_Freeze_Set(eMCT_CH1,(BOOL)ucSetting);     //A70LV_Doulas_0029
    //halScaler_Freeze_Set(eMCT_CH2,(BOOL)ucSetting);
    return eHAL_SCALER_EXEC_CODE_PASS;
}

eHAL_SCALER_EXEC_CODE halScaler_Gamma_Set(UINT8 ucCH,UINT8 ucSetting)    //A70LV_Doulas_0072 modify//A70LV_Doulas_0038 modify
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    UINT8  ucInputSource = halScaler_InputSource_Get(ucCH);
    UINT8  ucPresetMode = halScaler_PictureSettings_Get(ucCH);
    UINT8  ucSignalType = halScaler_SignalType_Get(ucCH);
	UINT8  ucPresetUserMode = halScaler_PictureSettings_PreUser_Get(ucCH);	//A70LK_Doulas_0004

    if (ucSetting >= eCM_GAMMA_NUMBER)
        return eHAL_SCALER_EXEC_CH_OVER_RANGE;

    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): Gamma =%d\r\n", __FUNCTION__, __LINE__, ucSetting);

	if (eCM_PICTURE_SETTINGS_USER != ucPresetMode) //A70LK_Doulas_0004 Modify
    	m_sUserSystemSetting.sColorSetting[ucInputSource][ucPresetMode][ucSignalType].ucGamma = ucSetting;
	else
    	m_sUserSystemSetting.sUserColorSetting[ucInputSource][ucPresetUserMode][ucSignalType].ucGamma = ucSetting;

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_WhitePeaking_Set(UINT8 ucCH,UINT8 ucSetting)     //A70LV_Doulas_0072 modify//A70LV_Doulas_0038 modify
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    UINT8  ucInputSource = halScaler_InputSource_Get(ucCH);
    UINT8  ucPresetMode = halScaler_PictureSettings_Get(ucCH);
    UINT8  ucSignalType = halScaler_SignalType_Get(ucCH);
    UINT8  ucPresetUserMode = halScaler_PictureSettings_PreUser_Get(ucCH);	//A70LK_Doulas_0004

    if ( ucSetting > WHITE_PEAKING_MIN_VALUE || ucSetting < WHITE_PEAKING_MAX_VALUE )
        return eHAL_SCALER_EXEC_CH_OVER_RANGE;

    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): WhitePeaking=%d\r\n", __FUNCTION__, __LINE__, ucSetting);

	if (eCM_PICTURE_SETTINGS_USER != ucPresetMode) //A70LK_Doulas_0004 Modify
    	m_sUserSystemSetting.sColorSetting[ucInputSource][ucPresetMode][ucSignalType].ucWhitePeaking = ucSetting;
	else
    	m_sUserSystemSetting.sUserColorSetting[ucInputSource][ucPresetUserMode][ucSignalType].ucWhitePeaking = ucSetting;

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_ColorTemperature_Set(UINT8 ucCH,UINT8 ucSetting)     //A70LV_Doulas_0072 modify//A70LV_Doulas_0038 modify
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    UINT8  ucInputSource = halScaler_InputSource_Get(ucCH);
    UINT8  ucPresetMode = halScaler_PictureSettings_Get(ucCH);
    UINT8  ucSignalType = halScaler_SignalType_Get(ucCH);
    UINT8  ucPresetUserMode = halScaler_PictureSettings_PreUser_Get(ucCH);	//A70LK_Doulas_0004

    if (ucSetting >= eCM_COLOR_TEMPERATURE_NUMBER)
        return eHAL_SCALER_EXEC_CH_OVER_RANGE;

    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): ColorTemperature=%d\r\n", __FUNCTION__, __LINE__, ucSetting);

	if (eCM_PICTURE_SETTINGS_USER != ucPresetMode) //A70LK_Doulas_0004 Modify
    	m_sUserSystemSetting.sColorSetting[ucInputSource][ucPresetMode][ucSignalType].ucCT = ucSetting;
	else
    	m_sUserSystemSetting.sUserColorSetting[ucInputSource][ucPresetUserMode][ucSignalType].ucCT = ucSetting;

    return eResult;
}

INT8 halScaler_EdgeEnhancement_Set(UINT8 ucSetting)
{
    return HAL_SCALER_PASS;
}

#if 0
// A70LV_Eric.C_0026 Start
INT8 halScaler_ColorWheelSpeed_Get(UINT8 ucCH)
{
    UINT8  ucInputSource = 0;//halScaler_InputSource_Get(ucCH);     //A70LV_Doulas_0263 modify
    UINT8  ucPresetMode = 0;//halScaler_PictureSettings_Get(ucCH);  //A70LV_Doulas_0263 modify
    UINT8  ucSignalType = 0;//halScaler_SignalType_Get(ucCH);       //A70LV_Doulas_0263 modify
    UINT8  ucVal = 0;

    ucVal = m_sUserSystemSetting.sColorSetting[ucInputSource][ucPresetMode][ucSignalType].ucColorEnhancement_CWSpeed;
    return (ucVal & 0x0F);
}
// A70LV_Eric.C_0026 End

eHAL_SCALER_EXEC_CODE halScaler_ColorWheelSpeed_Set(UINT8 ucCH,UINT8 ucSetting)      //A70LV_Doulas_0072 modify//A70LV_Doulas_0037 modify
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    UINT8  ucInputSource = 0;//halScaler_InputSource_Get(ucCH);     //A70LV_Doulas_0263 modify
    UINT8  ucPresetMode = 0;//halScaler_PictureSettings_Get(ucCH);  //A70LV_Doulas_0263 modify
    UINT8  ucSignalType = 0;//halScaler_SignalType_Get(ucCH);       //A70LV_Doulas_0263 modify
    UINT8  ucVal;

    if ( ucSetting > eCOLOR_SPEED_3X)// || ucSetting < eCOLOR_SPEED_2X )
        return eHAL_SCALER_EXEC_CH_OVER_RANGE;

    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): SkinColor=%d\r\n", __FUNCTION__, __LINE__, ucSetting);

    ucVal = m_sUserSystemSetting.sColorSetting[ucInputSource][ucPresetMode][ucSignalType].ucColorEnhancement_CWSpeed;
    ucVal = (ucVal & 0xF0) + 0x0F&(ucSetting);
    m_sUserSystemSetting.sColorSetting[ucInputSource][ucPresetMode][ucSignalType].ucColorEnhancement_CWSpeed = ucVal;

    return eResult;
}
#endif /* 0 */

eHAL_SCALER_EXEC_CODE halScaler_ColorEnhancement_Set(UINT8 ucCH,UINT8 ucSetting)     //A70LV_Doulas_0072 modify//A70LV_Doulas_0038 modify
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    UINT8  ucInputSource = halScaler_InputSource_Get(ucCH);
    UINT8  ucPresetMode = halScaler_PictureSettings_Get(ucCH);
    UINT8  ucSignalType = halScaler_SignalType_Get(ucCH);
    UINT8  ucPresetUserMode = halScaler_PictureSettings_PreUser_Get(ucCH);	//A70LK_Doulas_0004

    if (ucSetting >= eEDGE_ENHANCEMENT_MAX_NUMBER)
        return eHAL_SCALER_EXEC_CH_OVER_RANGE;

    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): ColorEnhancement=%d\r\n", __FUNCTION__, __LINE__, ucSetting);

	if (eCM_PICTURE_SETTINGS_USER != ucPresetMode) //A70LK_Doulas_0004 Modify
    	m_sUserSystemSetting.sColorSetting[ucInputSource][ucPresetMode][ucSignalType].ucColorEnhancement = ucSetting;
	else
		m_sUserSystemSetting.sUserColorSetting[ucInputSource][ucPresetUserMode][ucSignalType].ucColorEnhancement = ucSetting;

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_ColorEnhancement_Get(UINT8 ucCH,UINT8 *ucSetting)     //A70LK_Doulas_0004 Modify//A70LV_Doulas_0072 modify//A70LV_Doulas_0044
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    //UINT8  ucInputSource = halScaler_InputSource_Get(ucCH);
    //UINT8  ucPresetMode = halScaler_PictureSettings_Get(ucCH);
    //UINT8  ucSignalType = halScaler_SignalType_Get(ucCH);
    //UINT8  ucVal;

    //*ucSetting = m_sUserSystemSetting.sColorSetting[ucInputSource][ucPresetMode][ucSignalType].ucColorEnhancement;
	*ucSetting = halScaler_ColorSetting_Get(eHAL_COLOR_SETTING_COLORENHANCEMENT, ucCH);

    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): ColorEnhancement=%d\r\n", __FUNCTION__, __LINE__, *ucSetting);

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_HSGEnable_Set(UINT8 ucSetting)    //A70LV_Doulas_0072 modify//A70LV_Doulas_0038 //A70LV_Doulas_0023
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

    if(ucSetting > ets_ON)
        return eHAL_SCALER_EXEC_CH_OVER_RANGE;

    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): HSGEnable_Set =%d\r\n", __FUNCTION__, __LINE__, ucSetting);

    m_sUserSystemSetting.sImageSetting.ucHSGAdjustmentEnable = ucSetting;

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_HSGEnable_Get(UINT8 *ucSetting)    //A70LV_Doulas_0072 modify//A70LV_Doulas_0044
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

    *ucSetting = m_sUserSystemSetting.sImageSetting.ucHSGAdjustmentEnable;
    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): HSGEnable_Get =%d\r\n", __FUNCTION__, __LINE__ ,*ucSetting);

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_HSG_Set(UINT8 ucCH,sHSG_SETTING ucSetting)       //A70LV_Doulas_0072 modify//A70LV_Doulas_0038
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    UINT8  ucInputSource = halScaler_InputSource_Get(ucCH);
    UINT8  ucPresetMode = halScaler_PictureSettings_Get(ucCH);
    UINT8  ucPresetUserMode = halScaler_PictureSettings_PreUser_Get(ucCH);//G100_Steven_0033

    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): HSG_Set \r\n", __FUNCTION__, __LINE__);

    if(ucPresetMode != eCM_PICTURE_SETTINGS_USER )//G100_Steven_0033 start
    {
    	memcpy(&m_sUserSystemSetting.sHSG_setting[ucInputSource][ucPresetMode], &ucSetting, sizeof(sHSG_SETTING));
    }
    else
    {
    	memcpy(&m_sUserSystemSetting.sUserHSG_setting[ucInputSource][ucPresetUserMode], &ucSetting, sizeof(sHSG_SETTING));
    }//G100_Steven_0033 end

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_HSG_All_Set(UINT8 ucInputSource,UINT8  ucPresetMode,sHSG_SETTING ucSetting)       //A70LV_Doulas_0072 modify//A70LV_Doulas_0038
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): HSG_All_Set \r\n", __FUNCTION__, __LINE__);

    memcpy(&m_sUserSystemSetting.sHSG_setting[ucInputSource][ucPresetMode], &ucSetting, sizeof(sHSG_SETTING));

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_HSG_Get(sHSG_SETTING *m_sHSG_Setting)       //A70LV_Doulas_0072 modify//A70LV_Doulas_0038
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    UINT8  ucInputSource = halScaler_InputSource_Get(eMCT_CH1);
    UINT8  ucPresetMode = halScaler_PictureSettings_Get(eMCT_CH1);
    UINT8  ucPresetUserMode = halScaler_PictureSettings_PreUser_Get(eMCT_CH1);

    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): halScaler_HSG_Get \r\n", __FUNCTION__, __LINE__);

    if (eCM_PICTURE_SETTINGS_USER != ucPresetMode)  //G100_Steven_0020 start
    {
    	memcpy(m_sHSG_Setting, &m_sUserSystemSetting.sHSG_setting[ucInputSource][ucPresetMode], sizeof(sHSG_SETTING));
    }
    else
    {
    	memcpy(m_sHSG_Setting, &m_sUserSystemSetting.sUserHSG_setting[ucInputSource][ucPresetUserMode], sizeof(sHSG_SETTING));
    }  //G100_Steven_0020 end

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_PIP_PBP_Enable_Set(UINT8 ucSetting)      //A70LV_Doulas_0072 modify//A70LV_Doulas_0024
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;


    if(ucSetting >= ets_MAX_NUMBER)
        return eHAL_SCALER_EXEC_CH_OVER_RANGE;

    m_sUserSystemSetting.sSourceSetting.ucPIPEnable = ucSetting;
    halFrontEndCtrl_PIP_Enable_Set(&ucSetting);     //A70LV_Doulas_0120

    eResult = halScaler_SetOverlay(eMCT_CH1,(BOOL)ucSetting);       //A70LV_Doulas_0157

    if(ucSetting)
    {
        halScaler_LogoDisplayEn(false);
        //halScaler_SclVopWindowOffSet(eSOURCE_WINDOW_MAIN, false);
        //halScaler_SclVopWindowOffSet(eSOURCE_WINDOW_SUB, false);
    }
    else
    {
        if(m_sUserSystemSetting.sCommonSetting.ucBackupIputSwitch)
        {
            halScaler_BackupPrimaryInput_Set(eCM_SOURCE_NUMBER);
            halScaler_BackupSecondaryInput_Set(eCM_SOURCE_NUMBER);
            halScaler_RxPortClear(RXPORT_CHANNEL_0);
            halScaler_RxPortClear(RXPORT_CHANNEL_1);
        }
    }
    return eResult;
}

UINT8 halScaler_PIP_PBP_Enable_Get(void)      //A70LV_Doulas_0056
{
    return m_sUserSystemSetting.sSourceSetting.ucPIPEnable;
}

eHAL_SCALER_EXEC_CODE halScaler_IintChannelSetting(UINT8 ucCH)      //A70LV_Doulas_0155 modify//A70LV_Doulas_0072 modify//A70LV_Doulas_0030 modify //A70LV_Doulas_0029
{
    sCHANNEL_SETTING hal_sChannelSetting[eMCT_LAST];
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    UINT8  ucInputSource;
    UINT8 ucInputSource2;
    UINT8  ucMCT_CH;

    if(ucCH == eMCT_CH1)
    {
        ucMCT_CH = eMCT_CH1;
        ucInputSource = halScaler_InputSource_Get(ucMCT_CH);
        hal_sChannelSetting[eMCT_CH1].eInputSource          = (eCM_SOURCE_ID)halScaler_InputSource_Get((UINT8)eMCT_CH1);       //A70LV_Doulas_0072 modify
        hal_sChannelSetting[eMCT_CH1].eScalingMode          = (eCM_SCALING_MODE_ID)m_sUserSystemSetting.sSourceDependSetting[ucInputSource].ucAspectRatio;        //A70LV_Doulas_0072 modify
        hal_sChannelSetting[eMCT_CH1].eOverScan             = (eOVER_SCAN_TYPE)m_sUserSystemSetting.sSourceDependSetting[ucInputSource].ucOverScan;          //A70LV_Doulas_0072 modify
        //hal_sChannelSetting[eMCT_CH1].eColorFormat          = (eCOLOR_FORMAT)halScaler_ColorSpace_ScalerValueGet((UINT8)eMCT_CH1);    //A70LV_Doulas_0072 modify//A70LV_Doulas_0069 modify
        hal_sChannelSetting[eMCT_CH1].ucPhase               = m_sUserSystemSetting.sSourceDependSetting[ucInputSource].ucPhase;
        hal_sChannelSetting[eMCT_CH1].ucTracking            = m_sUserSystemSetting.sSourceDependSetting[ucInputSource].ucTracking;
        hal_sChannelSetting[eMCT_CH1].ucHoriPosition        = m_sUserSystemSetting.sSourceDependSetting[ucInputSource].ucHorzPosition;
        hal_sChannelSetting[eMCT_CH1].ucVertPosition        = m_sUserSystemSetting.sSourceDependSetting[ucInputSource].ucVertPosition;
        hal_sChannelSetting[eMCT_CH1].uiDigitalHorzZoom     = halScaler_DigitalHorzZoom_Get();
        hal_sChannelSetting[eMCT_CH1].uiDigitalVertZoom     = halScaler_DigitalVertZoom_Get();
        hal_sChannelSetting[eMCT_CH1].uiDigitalHorzShift    = halScaler_DigitalHorzShift_Get();
        hal_sChannelSetting[eMCT_CH1].uiDigitalVertShift    = halScaler_DigitalVertShift_Get();
        hal_sChannelSetting[eMCT_CH1].ucSharpness           = halScaler_Detail_Value_Get(ucMCT_CH);
        hal_sChannelSetting[eMCT_CH1].ucSkinColor           = halScaler_SkinColor_Value_Get(ucMCT_CH);
        hal_sChannelSetting[eMCT_CH1].iBrightness           = halScaler_Brightness_Value_Get(ucMCT_CH);
        hal_sChannelSetting[eMCT_CH1].dContrast             = halScaler_Contrast_Value_Get(ucMCT_CH);
        hal_sChannelSetting[eMCT_CH1].dColor                = halScaler_Saturation_Value_Get(ucMCT_CH);
        hal_sChannelSetting[eMCT_CH1].iHue                  = halScaler_HUE_Value_Get(ucMCT_CH);
        hal_sChannelSetting[eMCT_CH1].iRedOffset            = halScaler_RedOffset_Value_Get(ucMCT_CH);
        hal_sChannelSetting[eMCT_CH1].iGreenOffset          = halScaler_GreenOffset_Value_Get(ucMCT_CH);
        hal_sChannelSetting[eMCT_CH1].iBlueOffset           = halScaler_BlueOffset_Value_Get(ucMCT_CH);
        hal_sChannelSetting[eMCT_CH1].iRedGain              = halScaler_RedGain_Value_Get(ucMCT_CH);
        hal_sChannelSetting[eMCT_CH1].iGreenGain            = halScaler_GreenGain_Value_Get(ucMCT_CH);
        hal_sChannelSetting[eMCT_CH1].iBlueGain             = halScaler_BlueGain_Value_Get(ucMCT_CH);
        hal_sChannelSetting[eMCT_CH1].cGamma                = ets_OFF;
        hal_sChannelSetting[eMCT_CH1].cTemporal_NR          = m_sUserSystemSetting.sImageSetting.ucTemporalNoiseReduction;
        hal_sChannelSetting[eMCT_CH1].cMPEG_NR              = m_sUserSystemSetting.sImageSetting.ucMPEGNoiseReduction;
        hal_sChannelSetting[eMCT_CH1].cDetectFilm           = m_sUserSystemSetting.sImageSetting.ucFilm;
        hal_sChannelSetting[eMCT_CH1].cPIP_PBP_ON           = m_sUserSystemSetting.sSourceSetting.ucPIPEnable;
        hal_sChannelSetting[eMCT_CH1].cMain_Layout          = m_sUserSystemSetting.sSourceSetting.ucMainLayout;
        hal_sChannelSetting[eMCT_CH1].cPIP_Size             = m_sUserSystemSetting.sSourceSetting.ucPIPSize;
        hal_sChannelSetting[eMCT_CH1].cFREEZE               = m_sUserSystemSetting.sImageSetting.ucImageFreeze;
        hal_sChannelSetting[eMCT_CH1].cTestPattern          = eCM_TEST_PATTERN_OFF;          //A70LV_Doulas_0035
        hal_sChannelSetting[eMCT_CH1].eScanMode             = m_sProAV_Info.ucScanMode[ucCH];        //A70LV_Doulas_0076
        hal_sChannelSetting[eMCT_CH1].eVideoYUV             = eVIDEO_YUV_UNKNOW;    //A70LV_Doulas_0109
        hal_sChannelSetting[eMCT_CH1].eVGA_SYNC_TYPE        = m_sProAV_Info.ucVGA_SYNC_TYPE[ucCH];//eVGA_SYNC_TYPE_SOG;//eVGA_SYNC_TYPE_UNKNOW;    //A70LV_Doulas_0280 modify//A70LV_Doulas_0214 modify //A70LV_Doulas_0112
        hal_sChannelSetting[eMCT_CH1].eOSD_3D_Enable        = m_sUserSystemSetting.sSourceDependSetting[ucInputSource].uc3D_Enable;     //A70LV_Doulas_0154
        hal_sChannelSetting[eMCT_CH1].eOSD_3D_SyncOut       = halScaler_3DSyncOut_Get(eMCT_CH1);
        hal_sChannelSetting[eMCT_CH1].eOSD_3D_SyncIn        = halScaler_3DSyncIn_Get(eMCT_CH1);

        dvPro_IintChannelSetting((eMCT)ucCH,hal_sChannelSetting[eMCT_CH1]);

        halScaler_ColorSpace_ScalerValueGet((UINT8)eMCT_CH1);       //A70LV_Doulas_0280 update eVideoYUV
        for(ucInputSource = eCM_SOURCE_VGA; ucInputSource < eCM_SOURCE_NUMBER ; ucInputSource++)   //A70LV_Doulas_0112
            dvPro_AutoImage_Set(ucInputSource,m_sUserSystemSetting.sSourceDependSetting[ucInputSource].ucAutoImage);
    }
    else
    {
        ucMCT_CH = eMCT_CH2;
        ucInputSource = halScaler_InputSource_Get(eMCT_CH1);
        ucInputSource2 = halScaler_InputSource_Get(ucMCT_CH);
        hal_sChannelSetting[eMCT_CH2].eInputSource          = (eCM_SOURCE_ID)halScaler_InputSource_Get((UINT8)eMCT_CH2);       //A70LV_Doulas_0072 modify
        hal_sChannelSetting[eMCT_CH2].eScalingMode          = (eCM_SCALING_MODE_ID)m_sUserSystemSetting.sSourceDependSetting[ucInputSource].ucAspectRatio;       //A70LV_Doulas_0072 modify
        hal_sChannelSetting[eMCT_CH2].eOverScan             = (eOVER_SCAN_TYPE)m_sUserSystemSetting.sSourceDependSetting[ucInputSource].ucOverScan;          //A70LV_Doulas_0072 modify
        //hal_sChannelSetting[eMCT_CH2].eColorFormat          = (eCOLOR_FORMAT)halScaler_ColorSpace_ScalerValueGet((UINT8)eMCT_CH2);    //A70LV_Doulas_0072 modify//A70LV_Doulas_0069 modify
        hal_sChannelSetting[eMCT_CH2].ucPhase               = m_sUserSystemSetting.sSourceDependSetting[ucInputSource2].ucPhase;
        hal_sChannelSetting[eMCT_CH2].ucTracking            = m_sUserSystemSetting.sSourceDependSetting[ucInputSource2].ucTracking;
        hal_sChannelSetting[eMCT_CH2].ucHoriPosition        = m_sUserSystemSetting.sSourceDependSetting[ucInputSource].ucHorzPosition;
        hal_sChannelSetting[eMCT_CH2].ucVertPosition        = m_sUserSystemSetting.sSourceDependSetting[ucInputSource].ucVertPosition;
        hal_sChannelSetting[eMCT_CH2].uiDigitalHorzZoom     = halScaler_DigitalHorzZoom_Get();
        hal_sChannelSetting[eMCT_CH2].uiDigitalVertZoom     = halScaler_DigitalVertZoom_Get();
        hal_sChannelSetting[eMCT_CH2].uiDigitalHorzShift    = halScaler_DigitalHorzShift_Get();
        hal_sChannelSetting[eMCT_CH2].uiDigitalVertShift    = halScaler_DigitalVertShift_Get();
        hal_sChannelSetting[eMCT_CH2].ucSharpness           = halScaler_Detail_Value_Get(eMCT_CH1);
        hal_sChannelSetting[eMCT_CH2].ucSkinColor           = halScaler_SkinColor_Value_Get(eMCT_CH1);
        hal_sChannelSetting[eMCT_CH2].iBrightness           = halScaler_Brightness_Value_Get(eMCT_CH1);
        hal_sChannelSetting[eMCT_CH2].dContrast             = halScaler_Contrast_Value_Get(eMCT_CH1);
        hal_sChannelSetting[eMCT_CH2].dColor                = halScaler_Saturation_Value_Get(eMCT_CH1);
        hal_sChannelSetting[eMCT_CH2].iHue                  = halScaler_HUE_Value_Get(eMCT_CH1);
        hal_sChannelSetting[eMCT_CH2].iRedOffset            = halScaler_RedOffset_Value_Get(eMCT_CH1);
        hal_sChannelSetting[eMCT_CH2].iGreenOffset          = halScaler_GreenOffset_Value_Get(eMCT_CH1);
        hal_sChannelSetting[eMCT_CH2].iBlueOffset           = halScaler_BlueOffset_Value_Get(eMCT_CH1);
        hal_sChannelSetting[eMCT_CH2].iRedGain              = halScaler_RedGain_Value_Get(eMCT_CH1);
        hal_sChannelSetting[eMCT_CH2].iGreenGain            = halScaler_GreenGain_Value_Get(eMCT_CH1);
        hal_sChannelSetting[eMCT_CH2].iBlueGain             = halScaler_BlueGain_Value_Get(eMCT_CH1);
        hal_sChannelSetting[eMCT_CH2].cGamma                = ets_OFF;
        hal_sChannelSetting[eMCT_CH2].cTemporal_NR          = m_sUserSystemSetting.sImageSetting.ucTemporalNoiseReduction;
        hal_sChannelSetting[eMCT_CH2].cMPEG_NR              = m_sUserSystemSetting.sImageSetting.ucMPEGNoiseReduction;
        hal_sChannelSetting[eMCT_CH2].cDetectFilm           = m_sUserSystemSetting.sImageSetting.ucFilm;
        hal_sChannelSetting[eMCT_CH2].cPIP_PBP_ON           = m_sUserSystemSetting.sSourceSetting.ucPIPEnable;
        hal_sChannelSetting[eMCT_CH2].cMain_Layout          = m_sUserSystemSetting.sSourceSetting.ucMainLayout;
        hal_sChannelSetting[eMCT_CH2].cPIP_Size             = m_sUserSystemSetting.sSourceSetting.ucPIPSize;
        hal_sChannelSetting[eMCT_CH2].cFREEZE               = m_sUserSystemSetting.sImageSetting.ucImageFreeze;
        hal_sChannelSetting[eMCT_CH2].cTestPattern          = eCM_TEST_PATTERN_OFF;          //A70LV_Doulas_0035
        hal_sChannelSetting[eMCT_CH2].eScanMode             = m_sProAV_Info.ucScanMode[ucCH];        //A70LV_Doulas_0076
        hal_sChannelSetting[eMCT_CH2].eVideoYUV             = eVIDEO_YUV_UNKNOW;    //A70LV_Doulas_0109
        hal_sChannelSetting[eMCT_CH2].eVGA_SYNC_TYPE        = m_sProAV_Info.ucVGA_SYNC_TYPE[ucCH];//eVGA_SYNC_TYPE_SOG;//eVGA_SYNC_TYPE_UNKNOW;    //A70LV_Doulas_0280 modify//A70LV_Doulas_0214 modify//A70LV_Doulas_0112
        hal_sChannelSetting[eMCT_CH2].eOSD_3D_Enable        = m_sUserSystemSetting.sSourceDependSetting[ucInputSource].uc3D_Enable;     //A70LV_Doulas_0154
        hal_sChannelSetting[eMCT_CH2].eOSD_3D_SyncOut       = halScaler_3DSyncOut_Get(eMCT_CH2);
        hal_sChannelSetting[eMCT_CH2].eOSD_3D_SyncIn        = halScaler_3DSyncIn_Get(eMCT_CH2);

        dvPro_IintChannelSetting((eMCT)ucCH,hal_sChannelSetting[eMCT_CH2]);

        halScaler_ColorSpace_ScalerValueGet((UINT8)eMCT_CH2);       //A70LV_Doulas_0280 update eVideoYUV
    }
    return eResult;
}

UINT16 halScaler_DigitalHorzZoom_Get(void)
{
    //eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    //ePROAV_EXEC_CODE eC821Resut;
    UINT8 ucInputSource;
    UINT8 ucSizePresets;
    UINT16 uiSetting;

    //LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): Digital_Horz_Zoom=%d\r\n", __FUNCTION__, __LINE__, uiSetting);

    ucInputSource = halScaler_InputSource_Get(eMCT_CH1);
    ucSizePresets = halScaler_SizePresets_Get(eMCT_CH1);

	if(ucSizePresets == eCM_SCALING_MODE_CUSTOM)	//A70LK_Doulas_0015 Modify
		uiSetting = m_sUserSystemSetting.sSourceDependSetting[ucInputSource].uiCustomDigitalHorzZoom;
	else
 		uiSetting = m_sUserSystemSetting.sSourceDependSetting[ucInputSource].uiDigitalHorzZoom;

    return uiSetting;
}

UINT16 halScaler_DigitalVertZoom_Get(void)
{
    //eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    //ePROAV_EXEC_CODE eC821Resut;
    UINT8 ucInputSource;
    UINT8 ucSizePresets;
    UINT16 uiSetting;

    //LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): Digital_Vert_Zoom=%d\r\n", __FUNCTION__, __LINE__, uiSetting);

    ucInputSource = halScaler_InputSource_Get(eMCT_CH1);
    ucSizePresets = halScaler_SizePresets_Get(eMCT_CH1);

	if(ucSizePresets == eCM_SCALING_MODE_CUSTOM)	//A70LK_Doulas_0015 Modify
		uiSetting = m_sUserSystemSetting.sSourceDependSetting[ucInputSource].uiCustomDigitalVertZoom;
	else
  		uiSetting = m_sUserSystemSetting.sSourceDependSetting[ucInputSource].uiDigitalVertZoom;

    return uiSetting;
}

UINT8 halScaler_DigitalHorzShift_Get(void)
{
    //eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    //ePROAV_EXEC_CODE eC821Resut;
    UINT8 ucInputSource;
    UINT8 ucSizePresets;
    UINT8 ucSetting;

    //LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): Digital_Horz_Shift=%d\r\n", __FUNCTION__, __LINE__, ucSetting);

    ucInputSource = halScaler_InputSource_Get(eMCT_CH1);
    ucSizePresets = halScaler_SizePresets_Get(eMCT_CH1);

	if(ucSizePresets == eCM_SCALING_MODE_CUSTOM)	//A70LK_Doulas_0015 Modify
		ucSetting = m_sUserSystemSetting.sSourceDependSetting[ucInputSource].uiCustomDigitalHorzShift;
	else
   		ucSetting = m_sUserSystemSetting.sSourceDependSetting[ucInputSource].uiDigitalHorzShift;

    return ucSetting;
}

UINT8 halScaler_DigitalVertShift_Get(void)
{
    //eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    //ePROAV_EXEC_CODE eC821Resut;
    UINT8 ucInputSource;
    UINT8 ucSizePresets;
    UINT8 ucSetting;

    //LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): Digital_Vert_Shift=%d\r\n", __FUNCTION__, __LINE__, ucSetting);

    ucInputSource = halScaler_InputSource_Get(eMCT_CH1);
    ucSizePresets = halScaler_SizePresets_Get(eMCT_CH1);

	if(ucSizePresets == eCM_SCALING_MODE_CUSTOM)	//A70LK_Doulas_0015 Modify
		ucSetting = m_sUserSystemSetting.sSourceDependSetting[ucInputSource].uiCustomDigitalVertShift;
	else
  		ucSetting = m_sUserSystemSetting.sSourceDependSetting[ucInputSource].uiDigitalVertShift;

    return ucSetting;
}

eHAL_SCALER_EXEC_CODE halScaler_Detail_Value_Set(UINT8 ucCH,UINT8 ucSetting)     //A70LV_Doulas_0072 modify
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    UINT8  ucInputSource = halScaler_InputSource_Get(ucCH);
    UINT8  ucPresetMode = halScaler_PictureSettings_Get(ucCH);
    UINT8  ucSignalType = halScaler_SignalType_Get(ucCH);
    UINT8  ucPresetUserMode = halScaler_PictureSettings_PreUser_Get(ucCH);	//A70LK_Doulas_0004

    if ( ucSetting > SHARPNESS_MAX_VALUE || ucSetting < SHARPNESS_MIN_VALUE )
        return eHAL_SCALER_EXEC_CH_OVER_RANGE;

    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): Detail=%d\r\n", __FUNCTION__, __LINE__, ucSetting);

	if (eCM_PICTURE_SETTINGS_USER != ucPresetMode) //A70LK_Doulas_0004 Modify
    	m_sUserSystemSetting.sColorSetting[ucInputSource][ucPresetMode][ucSignalType].ucSharpness = ucSetting;
	else
    	m_sUserSystemSetting.sUserColorSetting[ucInputSource][ucPresetUserMode][ucSignalType].ucSharpness = ucSetting;

    dvPro_Sharpness_Value_Set(eMCT_CH1,ucSetting);       //A70LV_Doulas_0155
    dvPro_Sharpness_Value_Set(eMCT_CH2,ucSetting);       //A70LV_Doulas_0155

    return eResult;
}

UINT8 halScaler_Detail_Value_Get(UINT8 ucCH)		//A70LK_Doulas_0004 Modify
{
    //UINT8  ucInputSource = halScaler_InputSource_Get(ucCH);
    //UINT8  ucPresetMode = halScaler_PictureSettings_Get(ucCH);
    //UINT8  ucSignalType = halScaler_SignalType_Get(ucCH);

    //return m_sUserSystemSetting.sColorSetting[ucInputSource][ucPresetMode][ucSignalType].ucSharpness;
	return halScaler_ColorSetting_Get(eHAL_COLOR_SETTING_SHARPNESS, ucCH);
}

eHAL_SCALER_EXEC_CODE halScaler_SkinColor_Value_Set(UINT8 ucCH,UINT8 ucSetting)  //A70LV_Doulas_0072 modify//A70LV_Doulas_0029 modify
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    UINT8  ucInputSource = halScaler_InputSource_Get(ucCH);
    UINT8  ucPresetMode = halScaler_PictureSettings_Get(ucCH);
    UINT8  ucSignalType = halScaler_SignalType_Get(ucCH);
    UINT8  ucPresetUserMode = halScaler_PictureSettings_PreUser_Get(ucCH);	//A70LK_Doulas_0004

    if ( ucSetting > eSKIN_COLOR_HIGH)// || ucSetting < eSKIN_COLOR_OFF )
        return eHAL_SCALER_EXEC_CH_OVER_RANGE;

    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): SkinColor=%d\r\n", __FUNCTION__, __LINE__, ucSetting);

	if (eCM_PICTURE_SETTINGS_USER != ucPresetMode) //A70LK_Doulas_0004 Modify
    	m_sUserSystemSetting.sColorSetting[ucInputSource][ucPresetMode][ucSignalType].ucSkinColor = ucSetting;
	else
    	m_sUserSystemSetting.sUserColorSetting[ucInputSource][ucPresetUserMode][ucSignalType].ucSkinColor = ucSetting;

#ifdef __ICHIP_CONTROL__
    dvPro_SkinColor_Value_Set(eMCT_CH1,ucSetting);       //A70LV_Doulas_0155
    dvPro_SkinColor_Value_Set(eMCT_CH2,ucSetting);       //A70LV_Doulas_0155
#endif /* __ICHIP_CONTROL__ */

    return eResult;
}

UINT8 halScaler_SkinColor_Value_Get(UINT8 ucCH)		//A70LK_Doulas_0004 Modify
{
    //UINT8  ucInputSource = halScaler_InputSource_Get(ucCH);
    //UINT8  ucPresetMode = halScaler_PictureSettings_Get(ucCH);
    //UINT8  ucSignalType = halScaler_SignalType_Get(ucCH);

    //return m_sUserSystemSetting.sColorSetting[ucInputSource][ucPresetMode][ucSignalType].ucSkinColor;
    return halScaler_ColorSetting_Get(eHAL_COLOR_SETTING_SKINCOLOR, ucCH);
}

INT16 halScaler_Brightness_Value_Get(UINT8 ucCH)		//A70LK_Doulas_0004 Modify
{
    INT16 iSetting = 100;
    //UINT8  ucInputSource = halScaler_InputSource_Get(ucCH);
    //UINT8  ucPresetMode = halScaler_PictureSettings_Get(ucCH);
    //UINT8  ucSignalType = halScaler_SignalType_Get(ucCH);
    UINT8  ucSetting;

    //LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): Brightness=%d %d %d %d\r\n", __FUNCTION__, __LINE__, ucSetting);

    //ucSetting = m_sUserSystemSetting.sColorSetting[ucInputSource][ucPresetMode][ucSignalType].ucBrightness;
	ucSetting = halScaler_ColorSetting_Get(eHAL_COLOR_SETTING_BRIGHTNESS, ucCH);

    if(BRIGHTNESS_DEFAULT_VALUE == ucSetting)
    {
        iSetting = PROAV_BRIGHTNESS_DEFAULT;
    }
    else
    {
        iSetting = (INT16)(((DOUBLE)ucSetting/(DOUBLE)(BRIGHTNESS_MAX_VALUE - BRIGHTNESS_MIN_VALUE))*(PROAV_BRIGHTNESS_MAX - PROAV_BRIGHTNESS_MIN)) + PROAV_BRIGHTNESS_MIN;
    }

    //convert range 0~100(default:50) to -512~512(default:0)

    return iSetting;
}

DOUBLE halScaler_Contrast_Value_Get(UINT8 ucCH)	//A70LK_Doulas_0004 Modify
{
    DOUBLE dSetting = 100;
    //UINT8  ucInputSource = halScaler_InputSource_Get(ucCH);
    //UINT8  ucPresetMode = halScaler_PictureSettings_Get(ucCH);
    //UINT8  ucSignalType = halScaler_SignalType_Get(ucCH);
    UINT8  ucSetting;

    //LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): Contrast=%d\r\n", __FUNCTION__, __LINE__, ucSetting);

    //ucSetting = m_sUserSystemSetting.sColorSetting[ucInputSource][ucPresetMode][ucSignalType].ucContrast;
	ucSetting = halScaler_ColorSetting_Get(eHAL_COLOR_SETTING_CONTRAST, ucCH);

    if(CONTRAST_DEFAULT_VALUE == ucSetting)
    {
        dSetting = PROAV_CONTRAST_DEFAULT;
    }
    else
    {
        dSetting = (DOUBLE)(((DOUBLE)ucSetting/(DOUBLE)(CONTRAST_MAX_VALUE - CONTRAST_MIN_VALUE))*(PROAV_CONTRAST_MAX - PROAV_CONTRAST_MIN)) + PROAV_CONTRAST_MIN;
    }

    //convert range 0~100(default:50) to 0~3.99(default:1)

    return dSetting;
}

INT16 halScaler_HUE_Value_Get(UINT8 ucCH)		//A70LK_Doulas_0004 Modify
{
    INT16 iSetting;
    //UINT8  ucInputSource = halScaler_InputSource_Get(ucCH);
    //UINT8  ucPresetMode = halScaler_PictureSettings_Get(ucCH);
    //UINT8  ucSignalType = halScaler_SignalType_Get(ucCH);
    UINT8  ucSetting;

    //LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): HUE=%d\r\n", __FUNCTION__, __LINE__, ucSetting);

    //ucSetting = (INT16)m_sUserSystemSetting.sColorSetting[ucInputSource][ucPresetMode][ucSignalType].ucTint;
	ucSetting = halScaler_ColorSetting_Get(eHAL_COLOR_SETTING_TINT, ucCH);

    //convert range 0~100(default:50) to -180~180(default:0)

    if(ucSetting == HUE_DEFAULT_VALUE)
    {
        iSetting = PROAV_HUE_DEFAULT;
    }
    else
    {
        iSetting = (INT16)(((DOUBLE)ucSetting/(DOUBLE)(HUE_MAX_VALUE - HUE_MIN_VALUE))*(PROAV_HUE_MAX - PROAV_HUE_MIN)) + PROAV_HUE_MIN;
    }

    return iSetting;
}

DOUBLE halScaler_Saturation_Value_Get(UINT8 ucCH)		//A70LK_Doulas_0004 Modify
{
    DOUBLE dSetting = 100;
    //UINT8  ucInputSource = halScaler_InputSource_Get(ucCH);
    //UINT8  ucPresetMode = halScaler_PictureSettings_Get(ucCH);
    //UINT8  ucSignalType = halScaler_SignalType_Get(ucCH);
    UINT8  ucSetting;

    //LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): Saturation=%d\r\n", __FUNCTION__, __LINE__, ucSetting);

    //ucSetting = (DOUBLE)m_sUserSystemSetting.sColorSetting[ucInputSource][ucPresetMode][ucSignalType].ucSaturation;
	ucSetting = halScaler_ColorSetting_Get(eHAL_COLOR_SETTING_SATURATION, ucCH);

    //convert range 0~100(default:50) to 0~3.99(default:1)
    if ( ucSetting < SATURATION_DEFAULT_VALUE )
    {
        dSetting = (DOUBLE)ucSetting/(DOUBLE)(SATURATION_DEFAULT_VALUE- SATURATION_MIN_VALUE)*(PROAV_SATURATION_DEFAULT - PROAV_SATURATION_MIN);
    }
    else if (ucSetting> SATURATION_DEFAULT_VALUE)
    {
        dSetting = ((DOUBLE)ucSetting- SATURATION_DEFAULT_VALUE)/(DOUBLE)(SATURATION_MAX_VALUE- SATURATION_DEFAULT_VALUE)*(PROAV_SATURATION_MAX- PROAV_SATURATION_DEFAULT) + PROAV_SATURATION_DEFAULT;
    }
    else
    {
        dSetting = PROAV_SATURATION_DEFAULT;
    }


    return dSetting;
}

INT8 halScaler_RedOffset_Value_Get(UINT8 ucCH)		//A70LK_Doulas_0004 Modify
{
    INT16  iSetting;
    //UINT8  ucInputSource = halScaler_InputSource_Get(ucCH);
    //UINT8  ucPresetMode = halScaler_PictureSettings_Get(ucCH);
    //UINT8  ucSignalType = halScaler_SignalType_Get(ucCH);
    //UINT8  ucSetting = m_sUserSystemSetting.sColorSetting[ucInputSource][ucPresetMode][ucSignalType].ucRedOffset;
	UINT8  ucSetting = halScaler_ColorSetting_Get(eHAL_COLOR_SETTING_REDOFFSET, ucCH);

    if(ucSetting == RED_OFFSET_DEFAULT_VALUE)
    {
        iSetting = PROAV_INPUT_BIAS_R_DEFAULT;
    }
    else
    {
        iSetting = (INT16)(((DOUBLE)ucSetting/(DOUBLE)(RED_OFFSET_MAX_VALUE - RED_OFFSET_MIN_VALUE))*(PROAV_INPUT_BIAS_R_MAX - PROAV_INPUT_BIAS_R_MIN)) + PROAV_INPUT_BIAS_R_MIN;
    }

    return (UINT8)iSetting;
}

INT8 halScaler_GreenOffset_Value_Get(UINT8 ucCH)		//A70LK_Doulas_0004 Modify
{
    INT16  iSetting;
    //UINT8  ucInputSource = halScaler_InputSource_Get(ucCH);
    //UINT8  ucPresetMode = halScaler_PictureSettings_Get(ucCH);
    //UINT8  ucSignalType = halScaler_SignalType_Get(ucCH);
    //UINT8  ucSetting = m_sUserSystemSetting.sColorSetting[ucInputSource][ucPresetMode][ucSignalType].ucGreenOffset;
	UINT8  ucSetting = halScaler_ColorSetting_Get(eHAL_COLOR_SETTING_GREENOFFSET, ucCH);

    if(ucSetting == GREEN_OFFSET_DEFAULT_VALUE)
    {
        iSetting = PROAV_INPUT_BIAS_G_DEFAULT;
    }
    else
    {
        iSetting = (INT16)(((DOUBLE)ucSetting/(DOUBLE)(GREEN_OFFSET_MAX_VALUE - GREEN_OFFSET_MIN_VALUE))*(PROAV_INPUT_BIAS_G_MAX - PROAV_INPUT_BIAS_G_MIN)) + PROAV_INPUT_BIAS_G_MIN;
    }

    return (UINT8)iSetting;
}

INT8 halScaler_BlueOffset_Value_Get(UINT8 ucCH)		//A70LK_Doulas_0004 Modify
{
    INT16  iSetting;
    //UINT8  ucInputSource = halScaler_InputSource_Get(ucCH);
    //UINT8  ucPresetMode = halScaler_PictureSettings_Get(ucCH);
    ///UINT8  ucSignalType = halScaler_SignalType_Get(ucCH);
    //UINT8  ucSetting = m_sUserSystemSetting.sColorSetting[ucInputSource][ucPresetMode][ucSignalType].ucBlueOffset;
	UINT8  ucSetting = halScaler_ColorSetting_Get(eHAL_COLOR_SETTING_BLUEOFFSET, ucCH);

    if(ucSetting == BLUE_OFFSET_DEFAULT_VALUE)
    {
        iSetting = PROAV_INPUT_BIAS_B_DEFAULT;
    }
    else
    {
        iSetting = (INT16)(((DOUBLE)ucSetting/(DOUBLE)(BLUE_OFFSET_MAX_VALUE - BLUE_OFFSET_MIN_VALUE))*(PROAV_INPUT_BIAS_B_MAX - PROAV_INPUT_BIAS_B_MIN)) + PROAV_INPUT_BIAS_B_MIN;
    }

    return (UINT8)iSetting;
}

UINT8 halScaler_RedGain_Value_Get(UINT8 ucCH)		//A70LK_Doulas_0004 Modify
{
    INT16  iSetting;
    //UINT8  ucInputSource = halScaler_InputSource_Get(ucCH);
    //UINT8  ucPresetMode = halScaler_PictureSettings_Get(ucCH);
    //UINT8  ucSignalType = halScaler_SignalType_Get(ucCH);
    //UINT8  ucSetting = m_sUserSystemSetting.sColorSetting[ucInputSource][ucPresetMode][ucSignalType].ucRedGain;
	UINT8  ucSetting = halScaler_ColorSetting_Get(eHAL_COLOR_SETTING_REDGAIN, ucCH);

    if(ucSetting == RED_GAIN_DEFAULT_VALUE)
    {
        iSetting = PROAV_INPUT_GAIN_R_DEFAULT;
    }
    else if(ucSetting > RED_GAIN_DEFAULT_VALUE) // > 50
    {
        iSetting = (INT16)((DOUBLE)(ucSetting-RED_GAIN_DEFAULT_VALUE))*(PROAV_INPUT_GAIN_R_MAX-PROAV_INPUT_GAIN_R_DEFAULT)/(DOUBLE)(RED_GAIN_MAX_VALUE-RED_GAIN_DEFAULT_VALUE) + PROAV_INPUT_GAIN_R_DEFAULT;
    }
    else // < 50
    {
        iSetting = (INT16)((DOUBLE)ucSetting/(DOUBLE)(RED_GAIN_DEFAULT_VALUE-RED_GAIN_MIN_VALUE)*(PROAV_INPUT_GAIN_R_DEFAULT-PROAV_INPUT_GAIN_R_MIN));
    }

    return (UINT8)iSetting;
}

UINT8 halScaler_GreenGain_Value_Get(UINT8 ucCH)		//A70LK_Doulas_0004 Modify
{
    INT16  iSetting;
    //UINT8  ucInputSource = halScaler_InputSource_Get(ucCH);
    //UINT8  ucPresetMode = halScaler_PictureSettings_Get(ucCH);
    //UINT8  ucSignalType = halScaler_SignalType_Get(ucCH);
    //UINT8  ucSetting = m_sUserSystemSetting.sColorSetting[ucInputSource][ucPresetMode][ucSignalType].ucGreenGain;
	UINT8  ucSetting = halScaler_ColorSetting_Get(eHAL_COLOR_SETTING_GREENGAIN, ucCH);

    if(ucSetting == GREEN_GAIN_DEFAULT_VALUE)
    {
        iSetting = PROAV_INPUT_GAIN_G_DEFAULT;
    }
    else if(ucSetting > GREEN_GAIN_DEFAULT_VALUE) // > 50
    {
        iSetting = (INT16)((DOUBLE)(ucSetting-GREEN_GAIN_DEFAULT_VALUE))*(PROAV_INPUT_GAIN_G_MAX-PROAV_INPUT_GAIN_G_DEFAULT)/(DOUBLE)(GREEN_GAIN_MAX_VALUE-GREEN_GAIN_DEFAULT_VALUE) + PROAV_INPUT_GAIN_G_DEFAULT;
    }
    else // < 50
    {
        iSetting = (INT16)((DOUBLE)ucSetting/(DOUBLE)(GREEN_GAIN_DEFAULT_VALUE-GREEN_GAIN_MIN_VALUE)*(PROAV_INPUT_GAIN_G_DEFAULT-PROAV_INPUT_GAIN_G_MIN));
    }

    return (UINT8)iSetting;
}

UINT8 halScaler_BlueGain_Value_Get(UINT8 ucCH)		//A70LK_Doulas_0004 Modify
{
    INT16  iSetting;
    //UINT8  ucInputSource = halScaler_InputSource_Get(ucCH);
    //UINT8  ucPresetMode = halScaler_PictureSettings_Get(ucCH);
    //UINT8  ucSignalType = halScaler_SignalType_Get(ucCH);
    //UINT8  ucSetting = m_sUserSystemSetting.sColorSetting[ucInputSource][ucPresetMode][ucSignalType].ucBlueGain;
	UINT8  ucSetting = halScaler_ColorSetting_Get(eHAL_COLOR_SETTING_BLUEGAIN, ucCH);

    if(ucSetting == BLUE_GAIN_DEFAULT_VALUE)
    {
        iSetting = PROAV_INPUT_GAIN_B_DEFAULT;
    }
    else if(ucSetting > BLUE_GAIN_DEFAULT_VALUE) // > 50
    {
        iSetting = (INT16)((DOUBLE)(ucSetting-BLUE_GAIN_DEFAULT_VALUE))*(PROAV_INPUT_GAIN_B_MAX-PROAV_INPUT_GAIN_B_DEFAULT)/(DOUBLE)(BLUE_GAIN_MAX_VALUE-BLUE_GAIN_DEFAULT_VALUE) + PROAV_INPUT_GAIN_B_DEFAULT;
    }
    else // < 50
    {
        iSetting = (INT16)((DOUBLE)ucSetting/(DOUBLE)(BLUE_GAIN_DEFAULT_VALUE-BLUE_GAIN_MIN_VALUE)*(PROAV_INPUT_GAIN_B_DEFAULT-PROAV_INPUT_GAIN_B_MIN));
    }

    return (UINT8)iSetting;
}

eHAL_SCALER_EXEC_CODE halScaler_Temporal_NR_Value_Set(UINT8 ucSetting)   //A70LV_Doulas_0029 modify
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

    if ( ucSetting > eTEMPORAL_NR_HIGH)// || ucSetting < eTEMPORAL_NR_OFF )
        return eHAL_SCALER_EXEC_CH_OVER_RANGE;

    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): Temporal NR=%d\r\n", __FUNCTION__, __LINE__, ucSetting);

    m_sUserSystemSetting.sImageSetting.ucTemporalNoiseReduction = ucSetting;

    dvPro_Temporal_NR_Value_Set(eMCT_CH1,ucSetting);
    dvPro_Temporal_NR_Value_Set(eMCT_CH2,ucSetting);

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_MPEG_NR_Value_Set(UINT8 ucSetting)   //A70LV_Doulas_0029 modify
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

    if ( ucSetting > eMPEG_NR_HIGH)// || ucSetting < eMPEG_NR_OFF )
        return eHAL_SCALER_EXEC_CH_OVER_RANGE;

    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): MPEG NR=%d\r\n", __FUNCTION__, __LINE__, ucSetting);

    m_sUserSystemSetting.sImageSetting.ucMPEGNoiseReduction = ucSetting;

    dvPro_MPEG_NR_Value_Set(eMCT_CH1,ucSetting);
    dvPro_MPEG_NR_Value_Set(eMCT_CH2,ucSetting);

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_DetectFilm_Value_Set(UINT8 ucSetting)        //A70LV_Doulas_0029 modify
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

    if ( ucSetting > ets_ON)// || ucSetting < ets_OFF )
        return eHAL_SCALER_EXEC_CH_OVER_RANGE;

    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): Detect Film=%d\r\n", __FUNCTION__, __LINE__, ucSetting);

    m_sUserSystemSetting.sImageSetting.ucFilm = ucSetting;

    dvPro_Detect_Film_Value_Set(eMCT_CH1,ucSetting);
    dvPro_Detect_Film_Value_Set(eMCT_CH2,ucSetting);

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_PictureSettings_Value_Set(UINT8 ucCH,UINT8 ucSetting)     //A70LV_Doulas_0072 modify//A70LV_Doulas_0030
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    UINT8  ucInputSource = halScaler_InputSource_Get(ucCH);

    if ( ucSetting >= eCM_PICTURE_SETTINGS_NUMBER)// || ucSetting< eIFC_VIDEO )
        return eHAL_SCALER_EXEC_CH_OVER_RANGE;

    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): PictureSettings =%d\r\n", __FUNCTION__, __LINE__, ucSetting);

    m_sUserSystemSetting.sSourceDependSetting[ucInputSource].ucPresetMode = ucSetting;

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_MeasureInput(UINT8 ucCH, UINT8 ucInput)      //A70LV_Doulas_0031
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    sINPUT_TIMING_INFO sInputTiming;

//    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);

    //bIs4K = halScaler_Is4KPanel(m_sHalScalerInfo[ucCH].ePanelTimingId);

    {
        if(ucCH >= eMCT_LAST)
            return eHAL_SCALER_EXEC_CH_OVER_RANGE;
    }

    /* See if we can obtain the semaphore.  If the semaphore is not
    available wait 10 ticks to see if it becomes free. */
    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[ucCH].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(0);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }

        if(halScaler_RxPortIsReady(ucCH, ucInput) == eHAL_SCALER_EXEC_CODE_PASS)
        {
            UINT8 ucRx = halScaler_CurrentSourceRxPortGet(ucCH, ucInput);
            DetTmg sRxPorTiming = {0};

            halScaler_RxPortDetTiming(ucRx, (UINT8*)&sRxPorTiming);

            if(sRxPorTiming.Interlace == eSCAN_MODE_INTERLACE)
            {
               sInputTiming.eScanMode = eSCAN_MODE_INTERLACE;
               //sVideoInfo.wVStart = sVideoInfo.wVStart*2;
               //sVideoInfo.wVActive = sVideoInfo.wVActive*2;
            }
            else
            {
               sInputTiming.eScanMode = eSCAN_MODE_PROGRESSIVE;
            }

            sInputTiming.uiHStart  = sRxPorTiming.Hs.Start;
            sInputTiming.uiHActive = sRxPorTiming.Hs.Size;
            sInputTiming.uiVStart  = sRxPorTiming.Vs.Start;
            sInputTiming.uiVActive = sRxPorTiming.Vs.Size;
            sInputTiming.uiHTotal  = sRxPorTiming.Hs.Total;
            sInputTiming.uiVTotal  = sRxPorTiming.Vs.Total;
            sInputTiming.dFrameRate = m_sProAV_Info.sRxPortAVI_Info[ucRx].dFrameRate;

            dvPro_InputTimingSetting((eMCT)ucCH, TRUE, &sInputTiming);
        }
        else
        {
            dvProAV_SclOpuScreenOffSet((SclEntity)ucCH, TRUE);
            dvPro_InputTimingSetting((eMCT)ucCH, FALSE, NULL);
			dvPro_NoSignal((eMCT)ucCH, TRUE);

            eResult = eHAL_SCALER_EXEC_CODE_FAIL;
        }

        halScaler_SemaphoreGive(0);

    }
    else
    {
        LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore(%d)\r\n", __FUNCTION__, __LINE__,ucCH);
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }


    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_MeasureCheck(UINT8 ucCH)
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    int status = rcSUCCESS;
    //UINT8 portstatus = 0;
    //DetTmg sCurrentTiming = {0};
    UINT8 ucColorsSpace = 0;
    UINT8 ucPixelMode = 0;
    UINT8 ucRGBFull = 0;
    UINT8 uc3DMode = 0;
    UINT8 ucHDR = 0;
    AVI_INFOFRAME sAVIInfo;
#if 0
    HDR_INFOFRAME sHDRInfo;
#else
    HDR_INFOFRAME_PROAV sHDRInfo;
#endif
    UINT8 ucVSIInfo[8];
    RxPort sRxPort;

    if(ucCH >= eMCT_LAST)
        return eHAL_SCALER_EXEC_CH_OVER_RANGE;


    /* See if we can obtain the semaphore.  If the semaphore is not
    available wait 10 ticks to see if it becomes free. */
    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[ucCH].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(0);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }

        if(halScaler_RxPortIsReady((SclEntity)ucCH, halScaler_InputSource_Get(ucCH)))
        {
            halScaler_SemaphoreGive(0);
            return eHAL_SCALER_EXEC_CODE_FAIL;
        }

        status &= dvProAV_SclVipSourceGet((SclEntity)ucCH, &sRxPort);
        status &= dvProAV_SrcInputVSIInfoGet(sRxPort, ucVSIInfo);
        status &= dvProAV_HdmiRxAviGet(sRxPort, &sAVIInfo);
        #if 0
        status &= dvProAV_HdmiRxHdrGet(sRxPort, &sHDRInfo);
        #else
        status &= dvProAV_SclInputHDRInfoGet(sRxPort, &sHDRInfo);
        #endif

        ucColorsSpace = dvProAV_HdmiRxColorInfo2ColorSpace(&sAVIInfo);
        ucPixelMode = dvProAV_HdmiRxColorInfo2PixelMode(sAVIInfo.byte1.Y);
        //ucRGBFull = dvProAV_HdmiRxColorInfo2RGBMode(sAVIInfo.byte2.C, sAVIInfo.byte3.Q);
        ucRGBFull = dvProAV_HdmiRxColorInfo2RGBMode(sAVIInfo.vic, sAVIInfo.byte3.Q);
        uc3DMode = dvProAV_SclVSIInfo23DMode(ucVSIInfo);
        ucHDR = sHDRInfo.eotf & 0x07;

        switch(uc3DMode)
        {
            case eScl3DMode_FramePacking:
                LOG_MSG(db_HAL_SOURCE_INFO, "3DMode_FramePacking\r\n");
                break;

            case eScl3DMode_SideBySide:
                LOG_MSG(db_HAL_SOURCE_INFO, "3DMode_SideBySide\r\n");
                break;

            case eScl3DMode_TopAndBottom:
                LOG_MSG(db_HAL_SOURCE_INFO, "3DMode_TopAndBottom\r\n");
                break;

            case eScl3DMode_Off:
                LOG_MSG(db_HAL_SOURCE_INFO, "3DMode_Off\r\n");
                break;
        }

        //LOG_MSG(db_HAL_SOURCE_INFO, "[%d][%d]ucColorsSpace[%d] ucPixelMode[%d] Vic[%d] Q[%d]\r\n", ucCH, sRxPort, ucColorsSpace, ucPixelMode, sAVIInfo.vic, sAVIInfo.byte3.Q);
        LOG_MSG(db_HAL_SOURCE_INFO, "(%s, %d)[%d][%d]ucColorsSpace[%d] ucPixelMode[%d] Vic[%d] Q[%d] HDR[%d]\r\n", __FUNCTION__, __LINE__, ucCH, sRxPort, ucColorsSpace, ucPixelMode, sAVIInfo.vic, sAVIInfo.byte3.Q, ucHDR);
#if 1
        if(sHDRInfo.eotf & 0x07)
        {

            LOG_MSG(db_HAL_SOURCE_INFO, "HDR Info [%d][%d][%d][%d][%d][%d][%d][%d][%d][%d][%d][%d][%d][%d]\r\n",
                sHDRInfo.eotf,
                sHDRInfo.metaID,
                sHDRInfo.prim_x0,
                sHDRInfo.prim_y0,
                sHDRInfo.prim_x1,
                sHDRInfo.prim_y1,
                sHDRInfo.prim_x2,
                sHDRInfo.prim_y2,
                sHDRInfo.white_px,
                sHDRInfo.white_py,
                sHDRInfo.maxMstLum,
                sHDRInfo.minMstLum,
                sHDRInfo.maxContLv,
                sHDRInfo.maxFrmAvLv
            );
        }
#endif /* 0 */

        //if((AVIInfo[0] != AVIInfo[1]) && (AVIInfo[1] != AVIInfo[2]))
        {
            if((m_sProAV_Info.sRxPortAVI_Info[sRxPort].ucColorsSpace != ucColorsSpace) ||
               (m_sProAV_Info.sRxPortAVI_Info[sRxPort].ucPixelMode != ucPixelMode) ||
               (m_sProAV_Info.sRxPortAVI_Info[sRxPort].ucRGBFull != ucRGBFull) ||
               (m_sProAV_Info.sRxPortAVI_Info[sRxPort].uc3DMode != uc3DMode))
            {
                m_sProAV_Info.sRxPortAVI_Info[sRxPort].ucColorsSpace = ucColorsSpace;
                m_sProAV_Info.sRxPortAVI_Info[sRxPort].ucPixelMode = ucPixelMode;
                m_sProAV_Info.sRxPortAVI_Info[sRxPort].ucRGBFull = ucRGBFull;
                halScaler_ProAV_ColorSpaceConversion(ucCH, m_sProAV_Info.sRxPortAVI_Info[sRxPort].ucColorsSpace, m_sProAV_Info.sRxPortAVI_Info[sRxPort].ucPixelMode, m_sProAV_Info.sRxPortAVI_Info[sRxPort].ucRGBFull);

                if(dvPro_Input_3D_Format_Config_Get() && (ucCH == eMCT_CH2))
                {
                    halScaler_ProAV_ColorSpaceSetting(ucCH, halScaler_ColorSpace_Get(ucCH));
                }
                else
                {
                    halScaler_ProAV_ColorSpaceSetting(ucCH, halScaler_ColorSpace_Get(ucCH));
                }
            }

            if(m_sProAV_Info.sRxPortAVI_Info[sRxPort].ucHDR != ucHDR)
            {
                m_sProAV_Info.sRxPortAVI_Info[sRxPort].ucHDR = ucHDR;

                if(ucCH == eMCT_CH1)
                {
                    /*
                    if(m_sProAV_Info.sRxPortAVI_Info[sRxPort].ucHDR)
                    {
                        //printf("ucHDRLevel = %d\n", m_sUserSystemSetting.sImageSetting.ucHDRLevel);
                        halScaler_HDRLevelSetting(m_sUserSystemSetting.sImageSetting.ucHDRLevel);
                    }
                    else
                    {
                        //printf("SDR\n");
                        halScaler_HDRLevelSetting(eCM_HDR_LEVEL_SDR);
                    }
                    */
                    eResult = eHAL_SCALER_EXEC_CODE_HDR_CHANGE;
                }
            }

            if(m_sProAV_Info.sRxPortAVI_Info[sRxPort].uc3DMode != uc3DMode)
            {
                m_sProAV_Info.sRxPortAVI_Info[sRxPort].uc3DMode = uc3DMode;
                eResult = eHAL_SCALER_EXEC_CODE_FAIL;
            }
        }

        halScaler_SemaphoreGive(0);

    }
    else
    {
        LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore(%d)\r\n", __FUNCTION__, __LINE__,ucCH);
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }


    return eResult;
}


eHAL_SCALER_EXEC_CODE halScaler_OSD_TestPattern(const UINT8 ucTestPattern) //A70LV_Doulas_0035
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[0].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(0);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }

        eResult = halScaler_OSD_TestPattern_Set(eMCT_CH1,ucTestPattern);

        halScaler_SemaphoreGive(0);
    }
    else
    {
        LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore\r\n", __FUNCTION__, __LINE__);
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_OSD_TestPattern_Set(UINT8 ucCH,UINT8 ucSetting)      //A70LV_Doulas_0072 modify
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

    if ( ucSetting >= eCM_TEST_PATTERN_NUMBER)// || ucSetting < eSERVICE_TP_OFF )
        return eHAL_SCALER_EXEC_CH_OVER_RANGE;

    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): GreenOffset=%d\r\n", __FUNCTION__, __LINE__, ucSetting);

    m_sUserSystemSetting.sCommonSetting.ucOSDTestPattern = ucSetting;

    if(eCM_TEST_PATTERN_OFF == ucSetting)
    {
        dvProAV_SclDefaultTestPatternSet(0, ucSetting);
    }
    else
    {
        dvProAV_SclDefaultTestPatternSet(1, ucSetting);
    }

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_Service_TestPattern(const UINT8 ucTestPattern)
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[0].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(0);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }

#ifdef __ICHIP_CONTROL__
        eResult = halScaler_Service_TestPattern_Set(eMCT_CH1,ucTestPattern);
#endif /* __ICHIP_CONTROL__ */

        halScaler_SemaphoreGive(0);
    }
    else
    {
        LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore\r\n", __FUNCTION__, __LINE__);
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_Service_TestPattern_Set(UINT8 ucCH,UINT8 ucSetting)      //A70LV_Doulas_0072 modify
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

    if ( ucSetting > eSERVICE_TP_FULL_SCREEN)// || ucSetting < eSERVICE_TP_OFF )
        return eHAL_SCALER_EXEC_CH_OVER_RANGE;

    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): GreenOffset=%d\r\n", __FUNCTION__, __LINE__, ucSetting);

    m_sUserSystemSetting.sCommonSetting.ucServiceMenuTestPattern = ucSetting;
#ifdef __ICHIP_CONTROL__
    dvPro_TestPattern_Set((eMCT)ucCH,ucSetting);
#endif /* __ICHIP_CONTROL__ */

    return eResult;
}

UINT8 halScaler_PictureSettings_Get_NotReturnUser(UINT8 ucCH)     //A70LV_Doulas_0044
{
    UINT8 ucInputSource = halScaler_InputSource_Get(ucCH);
    UINT8 ucVal;

	if(m_sUserSystemSetting.sSourceDependSetting[ucInputSource].ucPresetMode == eCM_PICTURE_SETTINGS_USER)	//A70LK_Doulas_0004 Modify
        ucVal = m_sUserSystemSetting.sSourceDependSetting[ucInputSource].ucPreUserMode;
    else
        ucVal = m_sUserSystemSetting.sSourceDependSetting[ucInputSource].ucPresetMode;

    return ucVal;
}

eHAL_SCALER_EXEC_CODE halScaler_InputPixelClock_Get(UINT8 ucCH,UINT8 *ucValue)     //A70LV_Doulas_0056
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

    dvPro_InputPixelClock_Get((eMCT)ucCH,ucValue);     //A70LV_Doulas_0072 modify

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_InputResolution_Get(UINT8 ucCH,UINT8 *ucValue)
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

    dvPro_InputResolution_Get((eMCT)ucCH,ucValue);     //A70LV_Doulas_0072 modify

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_InputHorzRefresh_Get(UINT8 ucCH,UINT8 *ucValue)
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

    dvPro_InputHorzRefresh_Get((eMCT)ucCH,ucValue);      //A70LV_Doulas_0072 modify

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_InputVertRefresh_Get(UINT8 ucCH,UINT8 *ucValue)
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

    dvPro_InputVertRefresh_Get((eMCT)ucCH,ucValue);      //A70LV_Doulas_0072 modify

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_InputVertRefresh2_Get(UINT8 ucCH,UINT16 *uiValue)     //A70LV_Doulas_0200
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

    dvPro_InputVertRefresh2_Get((eMCT)ucCH,uiValue);
    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_InputSignalFormat_Get(UINT8 ucCH,UINT8 *ucValue)
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

    dvPro_InputSignalFormat_Get((eMCT)ucCH,ucValue);     //A70LV_Doulas_0072 modify

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_InputAspectRatio_Get(UINT8 ucCH,UINT8 *ucValue)
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

    dvPro_InputAspectRatio_Get((eMCT)ucCH,ucValue);        //A70LV_Doulas_0072 modify

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_FrontEndColorSpace_Set(UINT8 ucCH,UINT8 ucValue)     //A70LV_Doulas_0076 Add
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

    m_sProAV_Info.ucColorSpace[ucCH] = ucValue;
    halScaler_ColorSpace_Set(ucCH,halScaler_ColorSpace_Get(ucCH));
    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_FrontEndColorSpaceInfo_Get(UINT8 ucCH,UINT8* ucValue)     //A70LV_Doulas_0109 Add
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    UINT8 ucVal;
    char aucString[VERSION_STRING_MAX_LENGTH];

    switch(m_sProAV_Info.ucColorSpace[ucCH])
    {
        case eCOLOR_FORMAT_RGB:
            sprintf(aucString, "RGB\0");
            memcpy(ucValue, aucString, strlen(aucString)+1);
            break;

        case eCOLOR_FORMAT_RGB_LIMIT:
            sprintf(aucString, "RGB Video\0");
            memcpy(ucValue, aucString, strlen(aucString)+1);
            break;

        default:
            if(m_sProAV_Info.ucVideoYUV[ucCH] == eVIDEO_YUV_REC601)
            {
                sprintf(aucString, "REC601\0");
                memcpy(ucValue, aucString, strlen(aucString)+1);
            }
            else if(m_sProAV_Info.ucVideoYUV[ucCH] == eVIDEO_YUV_REC709)
            {
                sprintf(aucString, "REC709\0");
                memcpy(ucValue, aucString, strlen(aucString)+1);
            }
            else
            {
#ifdef __ICHIP_CONTROL__
                ucVal = dvPro_VideoYUV_Get((eMCT)ucCH);
#endif /* __ICHIP_CONTROL__ */
                if(ucVal == eVIDEO_YUV_REC601)
                {
                    sprintf(aucString, "REC601\0");
                    memcpy(ucValue, aucString, strlen(aucString)+1);
                }
                else
                {
                    sprintf(aucString, "REC709\0");
                    memcpy(ucValue, aucString, strlen(aucString)+1);
                }
            }
            break;
    }
    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_FrontEndScanMode_Set(UINT8 ucCH,UINT8 ucValue)
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

    if(ucValue < eSCAN_MODE_NO_SIGNAL)
    {
        m_sProAV_Info.ucScanMode[ucCH] = ucValue;
#ifdef __ICHIP_CONTROL__
        dvPro_FrontEndScanMode_Set((eMCT)ucCH,ucValue);
#endif /* __ICHIP_CONTROL__ */
    }
    else
        eResult = eHAL_SCALER_EXEC_CODE_FAIL;

    return eResult;
}

#if 0
eHAL_SCALER_EXEC_CODE halScaler_FrontEndScanMode_Clear(UINT8 ucCH)      //A70LV_Doulas_0175
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

    //LOG_MSG(db_ALWAYS, "@@@# %d ,%d \r\n",ucFrontEndeScanMode[ucCH],dvPro_Input_V_Active_Get((eMCT)ucCH));       //A70LV_Doulas_0181
#ifdef __ICHIP_CONTROL__
    if((m_sProAV_Info.ucScanMode[ucCH] == eSCAN_MODE_INTERLACE) &&
       (dvPro_Input_V_Active_Get((eMCT)ucCH) == 288))    //A70LV_Doulas_0181 modify re-sync 576i V-start error issue,check 576i
    {
        m_sProAV_Info.ucScanMode[ucCH] = eSCAN_MODE_NO_SIGNAL;
        dvPro_FrontEndScanMode_Set((eMCT)ucCH,eSCAN_MODE_NO_SIGNAL);
    }
#endif /* __ICHIP_CONTROL__ */
    return eResult;
}
#endif /* 0 */

BOOL halScaler_ForcedSyncResetType_Get(void)    //A70LV_Doulas_0079
{
#ifdef __ICHIP_CONTROL__
    return dvPro_ForcedSyncResetType_Get();
#else
    return FALSE;
#endif /* __ICHIP_CONTROL__ */
}

eHAL_SCALER_EXEC_CODE halScaler_ForcedSyncResetDisable_Set(void)
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    //BOOL   bForceSyncReseVal;
    //BOOL   bForceSyncReseVal2;  //A70LV_Doulas_0259
    //UINT16 uiH_Toral;
    //UINT16 uiV_Toral;

    if(halScaler_ForcedSyncResetType_Get() == FALSE)
        return eResult;

    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[0].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(0);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }
#ifdef __ICHIP_CONTROL__
        //A70LV_Doulas_0083 modify
        dvPro_ForceSyncRese_Get(&bForceSyncReseVal2,&uiH_Toral,&uiV_Toral);     //A70LV_Doulas_0259//A70LV_Doulas_0168 remove
        //if(bForceSyncReseVal)   //A70LV_Doulas_0168 remove
        {
            dvPro_ForcedSyncResetDisable_Set();    //disable ForcedSyncReset and reset C821 H-total & V-tatal
            dvPro_ForceSyncRese_Get(&bForceSyncReseVal,&uiH_Toral,&uiV_Toral);     //get C821 H-total & V-total
            //if(bForceSyncReseVal)     //A70LV_Doulas_0168 remove
            {
                halC789Ctrl_ForcedSyncResetValueSet((UINT8)TRUE,uiH_Toral,uiV_Toral);
            }
        }
        if(bForceSyncReseVal2)          //A70LV_Doulas_0259
        {
            if(halBoard_XillinxFPGA_NewVersionGet() == TRUE)
            {
                halBoard_XillinxFPGA_Manual_Mode_Set((UINT8)dvPro_PanelGet());
            }
            else
            {
                halBoard_XillinxFPGA_Reset();
            }
        }
#else

#endif /* __ICHIP_CONTROL__ */

        halScaler_SemaphoreGive(0);
    }
    else
    {
        LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore\r\n", __FUNCTION__, __LINE__);
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }

    return eResult;
}

BOOL halScaler_IsInterlaced_Get(UINT8 ucCH)    //A70LV_Doulas_0092
{
#ifdef __ICHIP_CONTROL__
    return dvPro_IsInterlaced_Get((eMCT)ucCH);
#else
    return FALSE;
#endif /* __ICHIP_CONTROL__ */
}

eHAL_SCALER_EXEC_CODE halScaler_FrontEndVideoYUV_Set(UINT8 ucCH,UINT8 ucValue)     //A70LV_Doulas_0109 Add
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    m_sProAV_Info.ucVideoYUV[ucCH] = ucValue;
    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_VGA_H_Total_Get(UINT8 ucCH,UINT16* uiVal)     //A70LV_Doulas_0112
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
#ifdef __ICHIP_CONTROL__
    *uiVal = dvPro_VGA_H_Total_Get((eMCT)ucCH);
#else
    *uiVal = 0;
#endif /* __ICHIP_CONTROL__ */
    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_VGA_V_Total_Get(UINT8 ucCH,UINT16* uiVal)     //A70LV_Doulas_0195
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
#ifdef __ICHIP_CONTROL__
    *uiVal = dvPro_VGA_V_Total_Get((eMCT)ucCH);
#else
    *uiVal = 0;
#endif /* __ICHIP_CONTROL__ */
    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_VGA_SYNC_TYPE_Set(UINT8 ucCH,UINT8 ucVal)     //A70LV_Doulas_0112
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
#ifdef __ICHIP_CONTROL__
    dvPro_VGA_SYNC_TYPE_Set((eMCT)ucCH,ucVal);
#endif /* __ICHIP_CONTROL__ */
    m_sProAV_Info.ucVGA_SYNC_TYPE[ucCH] = ucVal;          //A70LV_Doulas_0124
    return eResult;
}

UINT8 halScaler_VGA_SYNC_TYPE_Get(UINT8 ucCH)     //A70LV_Doulas_0124
{
    return m_sProAV_Info.ucVGA_SYNC_TYPE[ucCH];
}

eHAL_SCALER_EXEC_CODE halScaler_SyncThreshold_Set(UINT8 ucSetting)     //A70LV_Doulas_0114
{
    m_sUserSystemSetting.sImageSetting.ucSyncThreshold = ucSetting;
    return eHAL_SCALER_EXEC_CODE_PASS;
}

UINT8 halScaler_SyncThreshold_Get(void)     //A70LV_Doulas_0114
{
    return m_sUserSystemSetting.sImageSetting.ucSyncThreshold;
}

UINT32 halScaler_uiHFreq_Get(UINT8 ucCH)    //A70LV_Doulas_0115
{
#ifdef __ICHIP_CONTROL__
    return dvPro_uiHFreq_Get((eMCT)ucCH);
#else
    return 0;
#endif /* __ICHIP_CONTROL__ */
}

UINT8 halScaler_HDR_Get(UINT8 ucCH)
{
    UINT8 ucRxPort = halScaler_CurrentSourceRxPortGet(ucCH, halScaler_InputSource_Get(ucCH));

    if(m_sProAV_Info.sRxPortAVI_Info[ucRxPort].ucHDR)
    {
        return 1;
    }

    return 0;
}

UINT8 halScaler_HDR_EOTF_Get(UINT8 ucCH)
{
    HDR_INFOFRAME_PROAV sHDRInfo;
    UINT8 ucRxPort = halScaler_CurrentSourceRxPortGet(ucCH, halScaler_InputSource_Get(ucCH));
    dvProAV_SclInputHDRInfoGet(ucRxPort, &sHDRInfo);
    return sHDRInfo.eotf;
}
eHAL_SCALER_EXEC_CODE halScaler_Position_Run(UINT8 ucCH)    //A70LV_Doulas_0113
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    BOOL bHorzPositionSettingWork = FALSE;
    BOOL bVertPositionSettingWork = FALSE;

    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[0].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_Position_Run() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(0);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }

        //dvPro_Position_Run((eMCT)ucCH);      //A70LV_Doulas_0154 remove
        bHorzPositionSettingWork = dvPro_CheckingHorzPositionSettingWork((eMCT)ucCH);
        bVertPositionSettingWork = dvPro_CheckingVertPositionSettingWork((eMCT)ucCH);

        //halScaler_ConfigureForScaler(ucCH);
        //dvPro_BuildPicture_2K((eMCT)ucCH);

        if(bHorzPositionSettingWork && bVertPositionSettingWork)
            eResult = eHAL_SCALER_EXEC_CODE_PASS;
        else
            eResult = eHAL_SCALER_EXEC_CODE_FAIL;

        halScaler_SemaphoreGive(0);
    }
    else
    {
        LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore\r\n", __FUNCTION__, __LINE__);
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }
    return eResult;
}

UINT8 halScaler_HorzPositionWorkValueGet(UINT8 ucCH)
{
    return dvPro_HorzPositionWorkValueGet((eMCT)ucCH);
}

UINT8 halScaler_VertPositionWorkValueGet(UINT8 ucCH)
{
    return dvPro_VertPositionWorkValueGet((eMCT)ucCH);
}

eHAL_SCALER_EXEC_CODE halScaler_AutoPsitionStart(UINT8 ucCH,BOOL bAutoEnable)    //A70LV_Doulas_0118
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
#ifdef __ICHIP_CONTROL__
    dvPro_AutoPsitionStart((eMCT)ucCH,bAutoEnable);
#endif /* __ICHIP_CONTROL__ */
    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_Config_NoSignalOutput(UINT8 ucCH, UINT8 ucDisplayOutput)      //A70LV_Doulas_0142 //A70LV_Doulas_0120
{
    //BOOL bIs4K = FALSE;
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);

    //bIs4K = halScaler_Is4KPanel(m_sHalScalerInfo[ucCH].ePanelTimingId);

    if(ucCH >= eMCT_LAST)
        return eHAL_SCALER_EXEC_CH_OVER_RANGE;


    /* See if we can obtain the semaphore.  If the semaphore is not
    available wait 10 ticks to see if it becomes free. */
    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[ucCH].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(0);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }

        dvPro_Config_NoSignalOutput((eMCT)ucCH, ucDisplayOutput);   //A70LV_Doulas_0142

        halScaler_SemaphoreGive(0);

    }
    else
    {
        LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore(%d)\r\n", __FUNCTION__, __LINE__,ucCH);
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }


    return eResult;
}

UINT16 halScaler_Input_H_Active_Get(UINT8 ucCH)
{
    return dvPro_Input_H_Active_Get((eMCT)ucCH);
}

UINT16 halScaler_Input_V_Active_Get(UINT8 ucCH)
{
    return dvPro_Input_V_Active_Get((eMCT)ucCH);
}

UINT16 halScaler_Input_H_Total_Get(UINT8 ucCH)
{
    return dvPro_Input_H_Total_Get((eMCT)ucCH);
}

UINT16 halScaler_Input_V_Total_Get(UINT8 ucCH)
{
    return dvPro_Input_V_Total_Get((eMCT)ucCH);
}

UINT16 halScaler_Input_H_Start_Get(UINT8 ucCH)
{
    return dvPro_Input_H_Start_Get((eMCT)ucCH);
}

UINT16 halScaler_Input_V_Start_Get(UINT8 ucCH)
{
    return dvPro_Input_V_Start_Get((eMCT)ucCH);
}

eHAL_SCALER_EXEC_CODE halScaler_DDR_InputDataAreaTotalGet(UINT8 eCH,RECT DataArea,UINT32 *ulTotalRed,UINT32 *ulTotalGreen,UINT32 *ulTotalBlue)         //A70LV_Doulas_0124
{
    //eMCT eCH;
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;


    if(eCH >= eMCT_LAST)
        return eHAL_SCALER_EXEC_CODE_FAIL;

    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[0].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(0);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }

#ifdef __ICHIP_CONTROL__
        dvPro_DDR_InputDataAreaTotalGet((eMCT)eCH ,DataArea ,ulTotalRed ,ulTotalGreen ,ulTotalBlue);
#endif /* __ICHIP_CONTROL__ */

        halScaler_SemaphoreGive(0);
    }
    else
    {
        LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore\r\n", __FUNCTION__, __LINE__);
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_InputADC_Cali_CalibrationEnableSet(UINT8 eCH,UINT8 ucADC_CAl_Enable)
{
    //eMCT eCH;
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;


    if(eCH >= eMCT_LAST)
        return eHAL_SCALER_EXEC_CODE_FAIL;

    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[0].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(0);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }

#ifdef __ICHIP_CONTROL__
        dvPro_InputADC_Cali_CalibrationEnableSet((eMCT)eCH ,ucADC_CAl_Enable);
#endif /* __ICHIP_CONTROL__ */

        halScaler_SemaphoreGive(0);
    }
    else
    {
        LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore\r\n", __FUNCTION__, __LINE__);
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }

    return eResult;
}

UINT8 halScaler_Input_3D_Format_Get(void)           //A70LV_Doulas_0154
{
    return dvPro_Input_3D_Format_Get();
}

void halScaler_Input_3D_Format_Set(UINT8 ucVal)     //A70LV_Doulas_0154
{
    dvPro_Input_3D_Format_Set(ucVal);
}

eHAL_SCALER_EXEC_CODE halScaler_Input_2D_SyncOut_Enable(UINT8 ucVal)
{
    //eMCT eCH;
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        if(ucVal)
        {
            dvProAV_Scl2dSyncEn(true);
        }
        else
        {
            dvProAV_Scl2dSyncEn(false);
        }

        halScaler_SemaphoreGive(0);
    }
    else
    {
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_3D_InputPort_Set(void)     //A70LV_Doulas_0154
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[0].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(0);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }

#ifdef __ICHIP_CONTROL__
        dvPro_3D_InputPort_Set();
#endif /* __ICHIP_CONTROL__ */

        halScaler_SemaphoreGive(0);
    }
    else
    {
        LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore\r\n", __FUNCTION__, __LINE__);
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }

    return eResult;
}

UINT8 halScaler_Input_3D_Format_Config_Get(void)           //A70LV_Doulas_0159
{
    return dvPro_Input_3D_Format_Config_Get();
}

eHAL_SCALER_EXEC_CODE halScaler_InputHorzPeriod_Get(UINT8 ucCH,UINT32* udVal)     //A70LV_Doulas_0195
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
#ifdef __ICHIP_CONTROL__
    *udVal = dvPro_InputHorzPeriod_Get((eMCT)ucCH);
#else
    *udVal = 0;
#endif /* __ICHIP_CONTROL__ */
    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_VGA_ModeTable_Get(UINT8 ucCH,UINT8* ucVal)     //A70LV_Doulas_0195
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
#ifdef __ICHIP_CONTROL__
    *ucVal = dvPro_VGA_ModeTable_Get((eMCT)ucCH);
#else
    *ucVal = 0;
#endif /* __ICHIP_CONTROL__ */
    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_VGA_ModeTableNumber_Get(UINT8 ucCH,UINT16* uiVal)     //A70LV_Doulas_0195
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
#ifdef __ICHIP_CONTROL__
    *uiVal = dvPro_VGA_ModeTableNumber_Get((eMCT)ucCH);
#else
    *uiVal = 0;
#endif /* __ICHIP_CONTROL__ */
    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_ModeAdjusmenttHorzStart_Get(UINT16* uiVal)     //A70LV_Doulas_0195
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
#ifdef __ICHIP_CONTROL__
    *uiVal = dvPro_ModeAdjusmenttHorzStart_Get();
#else
    *uiVal = 0;
#endif /* __ICHIP_CONTROL__ */
    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_ModeAdjusmenttVertStart_Get(UINT16* uiVal)     //A70LV_Doulas_0195
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
#ifdef __ICHIP_CONTROL__
    *uiVal = dvPro_ModeAdjusmenttVertStart_Get();
#else
    *uiVal = 0;
#endif /* __ICHIP_CONTROL__ */
    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_ModeAdjusmenttHorzStart_Set(UINT16 uiVal)     //A70LV_Doulas_0195
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
#ifdef __ICHIP_CONTROL__
    dvPro_ModeAdjusmenttHorzStart_Set(uiVal);
#endif /* __ICHIP_CONTROL__ */
    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_ModeAdjusmenttVertStart_Set(UINT16 uiVal)     //A70LV_Doulas_0195
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
#ifdef __ICHIP_CONTROL__
    dvPro_ModeAdjusmenttVertStart_Set(uiVal);
#endif /* __ICHIP_CONTROL__ */
    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_ModeAdjusmenttEnableSetting(void)         //A70LV_Doulas_0195
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
#ifdef __ICHIP_CONTROL__
    dvPro_ModeAdjusmenttEnableSetting();
#endif /* __ICHIP_CONTROL__ */
    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_ModeAdjusmenttDisableSetting(void)         //A70LV_Doulas_0195
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
#ifdef __ICHIP_CONTROL__
    dvPro_ModeAdjusmenttDisableSetting();
#endif /* __ICHIP_CONTROL__ */
    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_Init_Mode_Adjustment_EEPROM_Setting(sTIMING_TABLE sTiming_Table)  //A70LV_Doulas_0195
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

#ifdef __ICHIP_CONTROL__
    dvPro_Init_Mode_Adjustment_EEPROM_Setting(sTiming_Table);
#endif /* __ICHIP_CONTROL__ */

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_Input3D_Timing_Get(UINT8 ucCH,UINT8* ucVal)     //A70LV_Doulas_0196
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
#ifdef __ICHIP_CONTROL__
    *ucVal = dvPro_Input3D_Timing_Get((eMCT)ucCH);
#else

    UINT8 uc3D_Enable = 0;
    UINT8 uc3D_DelaySelect = 0;

    uc3D_Enable = halScaler_3DEnable_Get();

    switch(uc3D_Enable)
    {
        case eCM_3D_FORMAT_AUTO:
            switch(dvPro_Input_3D_Format_Get())  //A70LV_Doulas_0162 //A70LV_Doulas_0159 modify
            {
                case eINPUT_3D_TYPE_FRAMEPACKING:
                case eINPUT_3D_TYPE_SIDEBYSIDE:
                case eINPUT_3D_TYPE_TOPANDBOTTOM:
                    uc3D_DelaySelect = e3DFS_MANDATORY_3D;
                    break;

                case eINPUT_3D_TYPE_FRAME_SEQUENTIAL:
                    //if(dvPro_InputRataIntegerGet(dvPro_InputFrameRate_Get((eMCT)ucCH)) == 120)
                    if(dvPro_InputRataIntegerGet(m_sProAV_Info.sRxPortAVI_Info[(RxPort)halScaler_CurrentSourceRxPortGet(ucCH, halScaler_InputSource_Get(ucCH))].dFrameRate) == 120)
                    {
                        uc3D_DelaySelect = e3DFS_FS_120;
                    }
                    else
                    {
                        uc3D_DelaySelect = e3DFS_FS_OTHER;
                    }
                    break;

                default:
                    uc3D_DelaySelect = e3DFS_OTHER;
                    break;
            }
            break;

        case eCM_3D_FORMAT_FRAME_SEQUENTIAL:
            //if(dvPro_InputRataIntegerGet(dvPro_InputFrameRate_Get((eMCT)ucCH)) == 120)
            if(dvPro_InputRataIntegerGet(m_sProAV_Info.sRxPortAVI_Info[(RxPort)halScaler_CurrentSourceRxPortGet(ucCH, halScaler_InputSource_Get(ucCH))].dFrameRate) == 120)
            {
                uc3D_DelaySelect = e3DFS_FS_120;
            }
            else
            {
                uc3D_DelaySelect = e3DFS_FS_OTHER;
            }
            break;

        case eCM_3D_FORMAT_4K3D:
        case eCM_3D_FORMAT_4K3D_DUALPIPE:
            uc3D_DelaySelect = e3DFS_4K3D;
            break;

        default:
            uc3D_DelaySelect = e3DFS_OTHER;
            break;
    }

    if(halScaler_3DMode_Get() == eCM_3D_MODE_PASSIVE_3D) //passive 3D
    {
        uc3D_DelaySelect += e3DFS_MANDATORY_3D_P;
    }

    *ucVal= uc3D_DelaySelect;
#endif /* __ICHIP_CONTROL__ */

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_ConfigureForPosition(UINT8 ucCH)     //A70LV_Doulas_0238
{
    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);


    /* See if we can obtain the semaphore.  If the semaphore is not
    available wait 10 ticks to see if it becomes free. */
    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[ucCH].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(0);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }

        dvPro_BuildPicture_2K((eMCT)ucCH);

        halScaler_SemaphoreGive(0);

    }
    else
    {
        LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore\r\n", __FUNCTION__, __LINE__);
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }


    return eHAL_SCALER_EXEC_CODE_PASS;
}

eHAL_SCALER_EXEC_CODE halScaler_ConfigureForScaler(UINT8 ucCH)     //A70LV_Doulas_0284
{
    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);


    /* See if we can obtain the semaphore.  If the semaphore is not
    available wait 10 ticks to see if it becomes free. */
    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[ucCH].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(0);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }

        dvPro_BuildPicture_2K((eMCT)ucCH);

        halScaler_SemaphoreGive(0);

    }
    else
    {
        LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore\r\n", __FUNCTION__, __LINE__);
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }


    return eHAL_SCALER_EXEC_CODE_PASS;
}

void halScaler_ProAV_ColorSpaceConversion(UINT8 ucCH, UINT8 ucColorSpace, UINT8 ucPixelMode, UINT8 ucRGBMode)
{
    switch(ucColorSpace)
    {
        case PROAV_SCALER_COLOR_SPACE_RGB_FULL:
            if(ucRGBMode)
            {
                halScaler_FrontEndColorSpace_Set((eMCT)ucCH, (UINT8)eCOLOR_FORMAT_RGB);
            }
            else
            {
                halScaler_FrontEndColorSpace_Set((eMCT)ucCH, (UINT8)eCOLOR_FORMAT_RGB_LIMIT);
            }
            break;

        case PROAV_SCALER_COLOR_SPACE_REC601:
            halScaler_FrontEndVideoYUV_Set((eMCT)ucCH, (UINT8)eVIDEO_YUV_REC601);

            switch(ucPixelMode)
            {
                case PROAV_SCALER_COLOR_PIXEL_MODE_YUV422:
                    halScaler_FrontEndColorSpace_Set((eMCT)ucCH, (UINT8)eCOLOR_FORMAT_422);
                    break;

                case PROAV_SCALER_COLOR_PIXEL_MODE_YUV444:
                    halScaler_FrontEndColorSpace_Set((eMCT)ucCH, (UINT8)eCOLOR_FORMAT_444);
                    break;

                case PROAV_SCALER_COLOR_PIXEL_MODE_YUV420:
                    halScaler_FrontEndColorSpace_Set((eMCT)ucCH, (UINT8)eCOLOR_FORMAT_420); //A70LK_Nina_0047
                    break;

                case PROAV_SCALER_COLOR_PIXEL_MODE_RGB444:
                default:
                    if(ucRGBMode)
                    {
                        halScaler_FrontEndColorSpace_Set((eMCT)ucCH, (UINT8)eCOLOR_FORMAT_RGB);
                    }
                    else
                    {
                        halScaler_FrontEndColorSpace_Set((eMCT)ucCH, (UINT8)eCOLOR_FORMAT_RGB_LIMIT);
                    }
                    break;
            }
            break;

        case PROAV_SCALER_COLOR_SPACE_REC709:
        case PROAV_SCALER_COLOR_SPACE_REC2020:
            halScaler_FrontEndVideoYUV_Set((eMCT)ucCH, (UINT8)eVIDEO_YUV_REC709);

            switch(ucPixelMode)
            {
                case PROAV_SCALER_COLOR_PIXEL_MODE_YUV422:
                    halScaler_FrontEndColorSpace_Set((eMCT)ucCH, (UINT8)eCOLOR_FORMAT_422);
                    break;

                case PROAV_SCALER_COLOR_PIXEL_MODE_YUV444:
                    halScaler_FrontEndColorSpace_Set((eMCT)ucCH, (UINT8)eCOLOR_FORMAT_444);
                    break;

                case PROAV_SCALER_COLOR_PIXEL_MODE_YUV420:
                    halScaler_FrontEndColorSpace_Set((eMCT)ucCH, (UINT8)eCOLOR_FORMAT_420); //A70LK_Nina_0047
                    break;

                case PROAV_SCALER_COLOR_PIXEL_MODE_RGB444:
                default:
                    if(ucRGBMode)
                    {
                        halScaler_FrontEndColorSpace_Set((eMCT)ucCH, (UINT8)eCOLOR_FORMAT_RGB);
                    }
                    else
                    {
                        halScaler_FrontEndColorSpace_Set((eMCT)ucCH, (UINT8)eCOLOR_FORMAT_RGB_LIMIT);
                    }
                    break;
            }
            break;

        case PROAV_SCALER_COLOR_SPACE_INVALID:
            break;

        default:
            break;
    }
}

eRESULT halScaler_ProAV_ColorSpaceSetting(UINT8 ucCH, UINT8 ucColorSpace)
{
    eRESULT status = rcSUCCESS;
    UINT8 colorspace = 0;
    UINT8 pixelmode = 0;
    UINT8 rgbmode = PROAV_SCALER_COLOR_RGB_LIMIT;
    RxPort sRxPort;

#if 0
    if((m_sUserSystemSetting.sSourceSetting.ucPIPEnable == 0) && (ucCH == eMCT_CH2))
    {
        return rcSUCCESS;
    }
#endif

    LOG_MSG(db_HAL_PROAV, "ucColorSpace [%d] ucFrontEndColorSpace[%d] = %d\n", ucColorSpace, ucCH, m_sProAV_Info.ucColorSpace[ucCH]);

    switch(ucColorSpace)
    {
        case eCM_COLOR_SPACE_AUTO:
            switch(m_sProAV_Info.ucColorSpace[ucCH])
            {
                case eCOLOR_FORMAT_422:
                    pixelmode = PROAV_SCALER_COLOR_PIXEL_MODE_YUV422;
                    if(m_sProAV_Info.ucVideoYUV[ucCH] == eVIDEO_YUV_REC601)
                    {
                    	colorspace = PROAV_SCALER_COLOR_SPACE_REC601;
                    }
                    else
                    {
                    	colorspace = PROAV_SCALER_COLOR_SPACE_REC709;
                    }
                    rgbmode = PROAV_SCALER_COLOR_RGB_FULL;
                    break;

                case eCOLOR_FORMAT_420: //A70LK_Nina_0047
                    pixelmode = PROAV_SCALER_COLOR_PIXEL_MODE_YUV420;
                    if(m_sProAV_Info.ucVideoYUV[ucCH] == eVIDEO_YUV_REC601)
                    {
                        colorspace = PROAV_SCALER_COLOR_SPACE_REC601;
                    }
                    else
                    {
                        colorspace = PROAV_SCALER_COLOR_SPACE_REC709;
                    }
                    rgbmode = PROAV_SCALER_COLOR_RGB_FULL;
                    break;

                case eCOLOR_FORMAT_444:
                    pixelmode = PROAV_SCALER_COLOR_PIXEL_MODE_YUV444;
                    if(m_sProAV_Info.ucVideoYUV[ucCH] == eVIDEO_YUV_REC601)
                    {
                        colorspace = PROAV_SCALER_COLOR_SPACE_REC601;
                    }
                    else
                    {
                        colorspace = PROAV_SCALER_COLOR_SPACE_REC709;
                    }
                    rgbmode = PROAV_SCALER_COLOR_RGB_FULL;
                    break;

                case eCOLOR_FORMAT_RGB:
                    pixelmode = PROAV_SCALER_COLOR_PIXEL_MODE_RGB444;
                    colorspace = PROAV_SCALER_COLOR_SPACE_RGB_FULL;
                    rgbmode = PROAV_SCALER_COLOR_RGB_FULL;
                    break;

                case eCOLOR_FORMAT_RGB_LIMIT:
                    pixelmode = PROAV_SCALER_COLOR_PIXEL_MODE_RGB444;
                    colorspace = PROAV_SCALER_COLOR_SPACE_RGB_FULL;
                    rgbmode = PROAV_SCALER_COLOR_RGB_LIMIT;
                    break;
                default:

                    break;
            }
            break;

        case eCM_COLOR_SPACE_RGB_FULL:
            pixelmode = PROAV_SCALER_COLOR_PIXEL_MODE_RGB444;
            colorspace = PROAV_SCALER_COLOR_SPACE_RGB_FULL;
            rgbmode = PROAV_SCALER_COLOR_RGB_FULL;
            break;

        case eCM_COLOR_SPACE_RGB_LIMITED:
            pixelmode = PROAV_SCALER_COLOR_PIXEL_MODE_RGB444;
            colorspace = PROAV_SCALER_COLOR_SPACE_RGB_FULL;
            rgbmode = PROAV_SCALER_COLOR_RGB_LIMIT;
            break;

        case eCM_COLOR_SPACE_YUV_REC709:
            colorspace = PROAV_SCALER_COLOR_SPACE_REC709;
            if(m_sProAV_Info.ucColorSpace[ucCH] == eCOLOR_FORMAT_422)
            {
                pixelmode = PROAV_SCALER_COLOR_PIXEL_MODE_YUV422;
            }
            else if(m_sProAV_Info.ucColorSpace[ucCH] == eCOLOR_FORMAT_420) //A70LK_Nina_0047
            {
                pixelmode = PROAV_SCALER_COLOR_PIXEL_MODE_YUV420;
            }
            else
            {
                pixelmode = PROAV_SCALER_COLOR_PIXEL_MODE_YUV444;
            }
            rgbmode = PROAV_SCALER_COLOR_RGB_FULL;
            break;

        case eCM_COLOR_SPACE_YUV_REC601:
            colorspace = PROAV_SCALER_COLOR_SPACE_REC601;
            if(m_sProAV_Info.ucColorSpace[ucCH] == eCOLOR_FORMAT_422)
            {
                pixelmode = PROAV_SCALER_COLOR_PIXEL_MODE_YUV422;
            }
            else if(m_sProAV_Info.ucColorSpace[ucCH] == eCOLOR_FORMAT_420) //A70LK_Nina_0047
            {
                pixelmode = PROAV_SCALER_COLOR_PIXEL_MODE_YUV420;
            }
            else
            {
                pixelmode = PROAV_SCALER_COLOR_PIXEL_MODE_YUV444;
            }
            rgbmode = PROAV_SCALER_COLOR_RGB_FULL;
            break;

        default:
            pixelmode = PROAV_SCALER_COLOR_PIXEL_MODE_RGB444;
            colorspace = PROAV_SCALER_COLOR_SPACE_RGB_FULL;
            break;
    }


    //LOG_MSG(db_HAL_PROAV, "colorspace [%d] [%d] [%d]\n", colorspace, pixelmode, rgbmode);

    status &= dvProAV_SclVipSourceGet((SclEntity)ucCH, &sRxPort);

    sRxPort = (RxPort)halScaler_CurrentSourceRxPortGet(ucCH, halScaler_InputSource_Get(ucCH));

    if(sRxPort > eRxPort_3)
    {
        return  rcERROR;
    }

    LOG_MSG(db_HAL_PROAV, "sRxPort [%d] colorspace [%d] [%d] [%d]\n", sRxPort, colorspace, pixelmode, rgbmode);

    status &= dvProAV_SrcColorSpaceSet(sRxPort, colorspace);
    status &= dvProAV_SrcPixelModeSet(sRxPort, pixelmode);
    status &= dvProAV_SrcLim2FulSet(sRxPort, rgbmode);

    if(ucCH == eMCT_CH1)
    {
        if((m_ucBackupSecondaryInput < (UINT8)eCM_SOURCE_NUMBER) &&
           (m_sUserSystemSetting.sCommonSetting.ucBackupIputSwitch == 1) &&
           (m_sUserSystemSetting.sSourceSetting.ucPIPEnable == 0) &&
           (halScaler_Input_3D_Format_Get() == eINPUT_3D_TYPE_OFF))
        {
            if(halScaler_InputPortIsReady(eSOURCE_WINDOW_SUB, m_ucBackupSecondaryInput))
            {
                if(halScaler_InputSource_Get(ucCH) != m_ucBackupSecondaryInput)
                {
                    LOG_MSG(db_HAL_PROAV, "Back up color\n");

                    sRxPort = (RxPort)halScaler_CurrentSourceRxPortGet(ucCH, m_ucBackupSecondaryInput);

                    if(sRxPort > eRxPort_3)
                    {
                        return  rcERROR;
                    }

                    status &= dvProAV_SrcColorSpaceSet(sRxPort, colorspace);
                    status &= dvProAV_SrcPixelModeSet(sRxPort, pixelmode);
                    status &= dvProAV_SrcLim2FulSet(sRxPort, rgbmode);
                }
            }
        }
    }

    return status;
}

eHAL_SCALER_EXEC_CODE halScaler_Monitor(UINT8 ucCH)
{
    eHAL_SCALER_EXEC_CODE  eScaler_Status = eHAL_SCALER_EXEC_CODE_FAIL;
    int status = rcSUCCESS;
    UINT8 portstatus;
    DetTmg sCurrentTiming = {0};
    AVI_INFOFRAME sAVIInfo;
    UINT8 ucVSIInfo[8];
    RxPort sRxPort;

    /* See if we can obtain the semaphore.  If the semaphore is not
    available wait 10 ticks to see if it becomes free. */
    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[ucCH].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(0);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }

        //if(ucCH == eMCT_CH2)
        //{
            //halScaler_SemaphoreGive(0);
            //return eHAL_SCALER_EXEC_CODE_PASS;
        //}
#if 0 //A70LK_Larry_0133 mask
        status &= dvProAV_SclInputPortInfoGet((SclEntity)ucCH, &portstatus, &sCurrentTiming);

	    LOG_MSG(db_HAL_PROAV, "==============================\n");
        LOG_MSG(db_HAL_PROAV, "InputPortInfoUpdate Port=%d: \n", ucCH);
        LOG_MSG(db_HAL_PROAV, "PortStatus: %d \n", portstatus);
        LOG_MSG(db_HAL_PROAV, "HTotal: %d \n",sCurrentTiming.Hs.Size);
        LOG_MSG(db_HAL_PROAV, "VTotal: %d \n",sCurrentTiming.Vs.Size);
        LOG_MSG(db_HAL_PROAV, "HStart: %d \n",sCurrentTiming.Hs.Start);
        LOG_MSG(db_HAL_PROAV, "VStart: %d \n",sCurrentTiming.Vs.Start);
        LOG_MSG(db_HAL_PROAV, "HSize: %d \n",sCurrentTiming.Hs.Size);
        LOG_MSG(db_HAL_PROAV, "VSize: %d \n",sCurrentTiming.Vs.Size);
        LOG_MSG(db_HAL_PROAV, "HSync: %d \n",sCurrentTiming.Hs.Sync);
        LOG_MSG(db_HAL_PROAV, "VSync: %d \n",sCurrentTiming.Vs.Sync);
        LOG_MSG(db_HAL_PROAV, "Interlace: %d\n",sCurrentTiming.Interlace);
#endif /* 0 */

        //if(sVideoInfo.wPixelCLK > 29000)
        {
            //dvProAV_HdmiRxMode2p0Set(sRxPort, true);
        }
        //else
        {
            //dvProAV_HdmiRxMode2p0Set(sRxPort, false);
        }

#if 1
        status &= dvProAV_SclVipSourceGet((SclEntity)ucCH, &sRxPort);
        if(sRxPort >= eRxPort_Max) //A70LK_Larry_0133
        {
            sRxPort = 0;
            ASSERT_ALWAYS();
        }
        status &= dvProAV_SrcInputVSIInfoGet(sRxPort, ucVSIInfo);
        status &= dvProAV_HdmiRxAviGet(sRxPort, &sAVIInfo);

        //colorspace = dvProAV_HdmiRxColorInfo2ColorSpace(AVIInfo);
        //pixelmode = dvProAV_HdmiRxColorInfo2PixelMode(AVIInfo[0]);
        //rgblimit = dvProAV_HdmiRxColorInfo2RGBMode(AVIInfo);

        m_sProAV_Info.sRxPortAVI_Info[sRxPort].ucColorsSpace = dvProAV_HdmiRxColorInfo2ColorSpace(&sAVIInfo);
        m_sProAV_Info.sRxPortAVI_Info[sRxPort].ucPixelMode = dvProAV_HdmiRxColorInfo2PixelMode(sAVIInfo.byte1.Y);
        //m_sProAV_Info.sRxPortAVI_Info[sRxPort].ucRGBFull = dvProAV_HdmiRxColorInfo2RGBMode(sAVIInfo.byte2.C, sAVIInfo.byte3.Q);
        m_sProAV_Info.sRxPortAVI_Info[sRxPort].ucRGBFull = dvProAV_HdmiRxColorInfo2RGBMode(sAVIInfo.vic, sAVIInfo.byte3.Q);
        m_sProAV_Info.sRxPortAVI_Info[sRxPort].uc3DMode = dvProAV_SclVSIInfo23DMode(ucVSIInfo);

        //if(halScaler_ColorSpace_Get(eSOURCE_WINDOW_MAIN) == eCM_COLOR_SPACE_AUTO)
        //{
            //UINT8 colorspace = dvProAV_HdmiRxColorInfo2ColorSpace(colorInfo);
            //UINT8 pixelmode = dvProAV_HdmiRxColorInfo2PixelMode(colorInfo);
            //status &= dvProAV_SclIpuColorSpaceSet((eMCT)ucCH, colorspace);
            //status &= dvProAV_SclIpuPixelModeSet((eMCT)ucCH, pixelmode);
        //}
#endif /* 0 */

        LOG_MSG(db_HAL_SOURCE_INFO, "(%s, %d)[%d][%d]ucRxPortStatus[%d]\r\n", __FUNCTION__, __LINE__, ucCH, sRxPort, m_sProAV_Info.ucRxPortStatus[sRxPort]);
        if (m_sProAV_Info.ucRxPortStatus[sRxPort] == eInpStatusChanged)
        {
            //status &= dvProAV_SclInputPortInfoClear((SclEntity)ucCH);
            status &= dvProAV_SclOpuScreenOffSet((SclEntity)ucCH, true);

            eScaler_Status = eHAL_SCALER_EXEC_CODE_PASS;
        }
        else if(m_sProAV_Info.ucRxPortStatus[sRxPort] != eInpStatusStable)
        {
            status &= dvProAV_SclOpuScreenOffSet((SclEntity)ucCH, true);
        }

        if (m_sProAV_Info.ucRxPortStatus[sRxPort] == eInpStatusStable)
        {
            eScaler_Status = eHAL_SCALER_EXEC_CODE_PASS;
        }

        if(eScaler_Status == eHAL_SCALER_EXEC_CODE_PASS)
        {
            LOG_MSG(db_HAL_PROAV, "Conversion [%02x][%02x][%02x]\n", m_sProAV_Info.sRxPortAVI_Info[sRxPort].ucColorsSpace, m_sProAV_Info.sRxPortAVI_Info[sRxPort].ucPixelMode, m_sProAV_Info.sRxPortAVI_Info[sRxPort].ucRGBFull);
            LOG_MSG(db_HAL_SOURCE_INFO, "(%s, %d)[%d][%d]ucColorsSpace[%d] ucPixelMode[%d] Vic[%d] Q[%d]\r\n", __FUNCTION__, __LINE__, ucCH, sRxPort, m_sProAV_Info.sRxPortAVIMonitor_Info[sRxPort].ucColorsSpace, m_sProAV_Info.sRxPortAVIMonitor_Info[sRxPort].ucPixelMode, sAVIInfo.vic, sAVIInfo.byte3.Q);

            if((m_sProAV_Info.sRxPortAVI_Info[sRxPort].ucColorsSpace != m_sProAV_Info.sRxPortAVIMonitor_Info[sRxPort].ucColorsSpace) ||
               (m_sProAV_Info.sRxPortAVI_Info[sRxPort].ucPixelMode != m_sProAV_Info.sRxPortAVIMonitor_Info[sRxPort].ucPixelMode) ||
               (m_sProAV_Info.sRxPortAVI_Info[sRxPort].ucRGBFull != m_sProAV_Info.sRxPortAVIMonitor_Info[sRxPort].ucRGBFull))
            {
                m_sProAV_Info.sRxPortAVIMonitor_Info[sRxPort].ucColorsSpace = m_sProAV_Info.sRxPortAVI_Info[sRxPort].ucColorsSpace;
                m_sProAV_Info.sRxPortAVIMonitor_Info[sRxPort].ucPixelMode = m_sProAV_Info.sRxPortAVI_Info[sRxPort].ucPixelMode;
                m_sProAV_Info.sRxPortAVIMonitor_Info[sRxPort].ucRGBFull = m_sProAV_Info.sRxPortAVI_Info[sRxPort].ucRGBFull;
                halScaler_ProAV_ColorSpaceConversion(ucCH, m_sProAV_Info.sRxPortAVI_Info[sRxPort].ucColorsSpace, m_sProAV_Info.sRxPortAVI_Info[sRxPort].ucPixelMode, m_sProAV_Info.sRxPortAVI_Info[sRxPort].ucRGBFull);

                if(dvPro_Input_3D_Format_Config_Get() && (ucCH == eMCT_CH2))
                {
                    halScaler_ProAV_ColorSpaceSetting(ucCH, halScaler_ColorSpace_Get(ucCH));
                }
                else
                {
                    halScaler_ProAV_ColorSpaceSetting(ucCH, halScaler_ColorSpace_Get(ucCH));
                }
            }
#if 0
            if(m_sUserSystemSetting.sSourceSetting.ucPIPEnable)
            {
                halScaler_ProAV_ColorSpaceSetting(ucCH, halScaler_ColorSpace_Get(ucCH));
            }
            else
            {
                if(ucCH == eMCT_CH1)
                {
                    halScaler_ProAV_ColorSpaceSetting(ucCH, halScaler_ColorSpace_Get(eSOURCE_WINDOW_MAIN));
                }
                else
                {
                    halScaler_ProAV_ColorSpaceSetting(ucCH, eCM_COLOR_SPACE_AUTO);
                }
            }
#endif /* 0 */
        }

        if (status != rcSUCCESS)
        {
            LOG_MSG(db_HAL_PROAV, "(func:%s, line:%d) Monitor Error\r\n", __FUNCTION__, __LINE__);
        }
        LOG_MSG(db_HAL_PROAV, "portstatus %d\r\n", portstatus);

        halScaler_SemaphoreGive(0);

    }
    else
    {
        LOG_MSG(db_HAL_PROAV, "(func:%s, line:%d) Can not get semphore\r\n", __FUNCTION__, __LINE__);
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }


    return eScaler_Status;
}

eHAL_SCALER_EXEC_CODE halScaler_AVIInfoFrame(UINT8 ucCH)
{
    eHAL_SCALER_EXEC_CODE  eScaler_Status = eHAL_SCALER_EXEC_CODE_FAIL;
    int status = rcSUCCESS;
    //UINT8 portstatus;
    sINPUT_TIMING_INFO sInputTiming;
    AVI_INFOFRAME sAVIInfo;
    UINT8 ucVSIInfo[8];
    RxPort sRxPort;

    /* See if we can obtain the semaphore.  If the semaphore is not
    available wait 10 ticks to see if it becomes free. */
    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[ucCH].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(0);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }

        status &= dvProAV_SclVipSourceGet((SclEntity)ucCH, &sRxPort);
        status &= dvProAV_SrcInputVSIInfoGet(sRxPort, ucVSIInfo);
        status &= dvProAV_HdmiRxAviGet(sRxPort, &sAVIInfo);


        m_sProAV_Info.sRxPortAVI_Info[sRxPort].ucColorsSpace = dvProAV_HdmiRxColorInfo2ColorSpace(&sAVIInfo);
        m_sProAV_Info.sRxPortAVI_Info[sRxPort].ucPixelMode = dvProAV_HdmiRxColorInfo2PixelMode(sAVIInfo.byte1.Y);
        //m_sProAV_Info.sRxPortAVI_Info[sRxPort].ucRGBFull = dvProAV_HdmiRxColorInfo2RGBMode(sAVIInfo.byte2.C, sAVIInfo.byte3.Q);
        m_sProAV_Info.sRxPortAVI_Info[sRxPort].ucRGBFull = dvProAV_HdmiRxColorInfo2RGBMode(sAVIInfo.vic, sAVIInfo.byte3.Q);
        m_sProAV_Info.sRxPortAVI_Info[sRxPort].uc3DMode = dvProAV_SclVSIInfo23DMode(ucVSIInfo);
        m_sProAV_Info.sRxPortAVIMonitor_Info[sRxPort].ucColorsSpace = m_sProAV_Info.sRxPortAVI_Info[sRxPort].ucColorsSpace; //A70LK_Larry_0133
        m_sProAV_Info.sRxPortAVIMonitor_Info[sRxPort].ucPixelMode = m_sProAV_Info.sRxPortAVI_Info[sRxPort].ucPixelMode; //A70LK_Larry_0133
        m_sProAV_Info.sRxPortAVIMonitor_Info[sRxPort].ucRGBFull = m_sProAV_Info.sRxPortAVI_Info[sRxPort].ucRGBFull; //A70LK_Larry_0133
        m_sProAV_Info.sRxPortAVIMonitor_Info[sRxPort].uc3DMode = m_sProAV_Info.sRxPortAVI_Info[sRxPort].uc3DMode; //A70LK_Larry_0133

        LOG_MSG(db_HAL_PROAV, "Conversion [%02x][%02x][%02x]\n", m_sProAV_Info.sRxPortAVI_Info[sRxPort].ucColorsSpace,  m_sProAV_Info.sRxPortAVI_Info[sRxPort].ucPixelMode, m_sProAV_Info.sRxPortAVI_Info[sRxPort].ucRGBFull);

		LOG_MSG(db_HAL_PROAV, "==============================\n");
        halScaler_ProAV_ColorSpaceConversion(ucCH, m_sProAV_Info.sRxPortAVI_Info[sRxPort].ucColorsSpace, m_sProAV_Info.sRxPortAVI_Info[sRxPort].ucPixelMode, m_sProAV_Info.sRxPortAVI_Info[sRxPort].ucRGBFull);

        if(dvPro_Input_3D_Format_Config_Get() && (ucCH == eMCT_CH2))
        {
            halScaler_ProAV_ColorSpaceSetting(ucCH, halScaler_ColorSpace_Get(ucCH));
        }
        else
        {
            halScaler_ProAV_ColorSpaceSetting(ucCH, halScaler_ColorSpace_Get(ucCH));
        }

#if 0
        if(m_sUserSystemSetting.sSourceSetting.ucPIPEnable)
        {
            halScaler_ProAV_ColorSpaceSetting(ucCH, halScaler_ColorSpace_Get(ucCH));
        }
        else
        {
            if(ucCH == eMCT_CH1)
            {
                halScaler_ProAV_ColorSpaceSetting(ucCH, halScaler_ColorSpace_Get(eSOURCE_WINDOW_MAIN));
            }
            else
            {
                halScaler_ProAV_ColorSpaceSetting(ucCH, eCM_COLOR_SPACE_AUTO);
            }
        }
#endif /* 0 */

        #if 0
        // get pixel mode
        if(dvProAV_SrcPixelModeGet(sRxPort) == eAVI_PIXEL_MODE_YCbCr420)    // 420 mode H Total size start x2
        {
            dvPro_InputTimingGet((eMCT)ucCH, &sInputTiming);
            sInputTiming.uiHTotal *= 2;
            sInputTiming.uiHActive *= 2;
            sInputTiming.uiHStart *= 2;
            dvPro_InputTimingSetting((eMCT)ucCH, TRUE, &sInputTiming);
        }
        #endif
        halScaler_SemaphoreGive(0);

    }
    else
    {
        LOG_MSG(db_HAL_PROAV, "(func:%s, line:%d) Can not get semphore\r\n", __FUNCTION__, __LINE__);
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }


    return eScaler_Status;
}

eHAL_SCALER_EXEC_CODE halScaler_SourceLock(UINT8 ucCH)
{
    eHAL_SCALER_EXEC_CODE  eScaler_Status = eHAL_SCALER_EXEC_CODE_FAIL;
    int status = rcSUCCESS;
    //UINT8 portstatus;
    //DetTmg sCurrentTiming = {0};

    /* See if we can obtain the semaphore.  If the semaphore is not
    available wait 10 ticks to see if it becomes free. */
    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[ucCH].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(0);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }

        status  = rcSUCCESS;

        status &= dvProAV_SclVopScreenOffSet(false);
        status &= dvProAV_SclLogoDisplayEn(false);

        // 先關閉主視窗與子視窗畫面, 避免使用者看到暫態畫面
        status &= dvProAV_SclOpuScreenOffSet((SclEntity)ucCH, true); // Turn Off

        // 設定主視窗
        status &= dvPro_BuildPicture_2K((eMCT)ucCH);

        MS_SLEEP(50); //等OPU sync完成，避免看到圖方大縮小

        // 開啟主視窗與子視窗畫面
        status &= dvProAV_SclOpuScreenOffSet((SclEntity)ucCH, false); // Turn On
        status &= dvProAV_SclVopWindowOffSet((SclEntity)ucCH, false); // Turn On

        // 切換到正常訊號畫面
		//dvPro_NoSignal((eMCT)ucCH, FALSE);

        if (status != rcSUCCESS)
        {
            LOG_MSG(db_HAL_PROAV, "WindowConfig Error\n");
        }

        eScaler_Status = eHAL_SCALER_EXEC_CODE_PASS;

        if (status != rcSUCCESS)
        {
            LOG_MSG(db_HAL_PROAV, "(func:%s, line:%d) Monitor Error\r\n", __FUNCTION__, __LINE__);
        }


        halScaler_SemaphoreGive(0);

    }
    else
    {
        LOG_MSG(db_HAL_PROAV, "(func:%s, line:%d) Can not get semphore\r\n", __FUNCTION__, __LINE__);
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }


    return eScaler_Status;
}


eHAL_SCALER_EXEC_CODE halScaler_SPI_SelectSet(UINT8 ucSelect)
{
    /* See if we can obtain the semaphore.  If the semaphore is not
    available wait 10 ticks to see if it becomes free. */
    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[0].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(0);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }

        dvProAV_ScalerSPISel((ScalerSPISel)ucSelect);

        halScaler_SemaphoreGive(0);
    }
    else
    {
        LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore\r\n", __FUNCTION__, __LINE__);
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }


    return eHAL_SCALER_EXEC_CODE_PASS;
}


eHAL_SCALER_EXEC_CODE halScaler_InputSelectSet(UINT8 ucCH, UINT8 ucSelect, BOOL BlackSource)
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    //BOOL bHorzPositionSettingWork = FALSE;
    //BOOL bVertPositionSettingWork = FALSE;
    UINT8 ucRx = 0;

    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[0].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_Position_Run() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(0);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }

        ucRx = halScaler_CurrentSourceRxPortGet(ucCH, ucSelect);

        if(ucRx > eRxPort_3)
        {
            halScaler_SemaphoreGive(0);
            return eHAL_SCALER_EXEC_CODE_FAIL;
        }

        if(BlackSource)
        {
            dvProAV_SclVopWindowOffSet((SclEntity)ucCH, false);
            dvProAV_SclOpuScreenOffSet((SclEntity)ucCH, true);
        }
        dvProAV_SclVipSourceSet((SclEntity)ucCH, (RxPort)ucRx);

        halScaler_SemaphoreGive(0);

        //LOG_MSG(db_ALWAYS, "(func:%s, line:%d) input CH:%d, Source:%d, RX:%d\r\n", __FUNCTION__, __LINE__ , ucCH, ucSelect, ucRx);
    }
    else
    {
        LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore\r\n", __FUNCTION__, __LINE__);
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }
    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_BackupRxPortSet(UINT8 ucCH)
{
    //BOOL bHorzPositionSettingWork = FALSE;
    //BOOL bVertPositionSettingWork = FALSE;

    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[0].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_Position_Run() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(0);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }

        //dvProAV_SclVipSourceGet(ucCH, (RxPort*)&ucRx);
        //backup下要有Vsync參考才能變更size presets
        dvProAV_SclVipSourceSet(eSclEntity_Main, (ucCH == eSclEntity_Main) ? eRxPort_0 : eRxPort_2);

        halScaler_SemaphoreGive(0);

        //LOG_MSG(db_ALWAYS, "(func:%s, line:%d) input CH:%d, Source:%d, RX:%d\r\n", __FUNCTION__, __LINE__ , ucCH, ucSelect, ucRx);
    }
    else
    {
        LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore\r\n", __FUNCTION__, __LINE__);
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }
    return eHAL_SCALER_EXEC_CODE_PASS;
}

eHAL_SCALER_EXEC_CODE halScaler_FrameSyncModeSet(UINT8 ucCH, UINT8 eMode)    // ProAV_Rex_0032
{
    eHAL_SCALER_EXEC_CODE eHalResult = eHAL_SCALER_EXEC_CODE_PASS;
    int eResult = rcSUCCESS;
    UINT32 ulRefClk = 0, ulPLLRefClk = 0;
    UINT32 ulPLLNDividerR = 0, ulPLLNDividerN = 0, ulTargetRatioN = 0;
    UINT16 uiNDivider = 0, uiTargetRatioInt = 0;
    RxPort sRxPort = eRxPort_0;
    UINT8 ucTimes = 0;
    UINT8 ucIs5994 = 0;
    bool bSupFrameSync = false;
    UINT8 ucIptFrameRate = 0, ucOptFrameRate = 0, ucInputDivider = 0;
    double iInputFrameRate = 0;
    UINT8 ucCurrentMode = 0;
    //UINT8 ucCurrentPort = 0;
    BOOL bOptType = (Board_SingleBoard_Get() == FALSE) ? TRUE:FALSE;

    PanelTiming sPanelInfo = {0};

    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        // 判斷是否為支援frame sync mode的frame rate
        if(eMode != eSclSyncMode_Off)
        {
            //ucIptFrameRate = dvPro_InputRataIntegerGet(dvPro_InputFrameRate_Get((eMCT)ucCH));
            iInputFrameRate = m_sProAV_Info.sRxPortAVI_Info[(RxPort)halScaler_CurrentSourceRxPortGet(ucCH, halScaler_InputSource_Get(ucCH))].dFrameRate;
            ucIptFrameRate = dvPro_InputRataIntegerGet(iInputFrameRate);
            ucOptFrameRate = (UINT8)dvPro_PanelFrameRateGet();

            dvProAV_SclVipSourceGet((SclEntity)ucCH, &sRxPort);    // get source Rx port

            for(UINT8 ucCount = 0; ucCount < 5; ucCount++)
            {
                UINT32 ulTemp = dvProAV_HdmiRxTmdsClkGet(sRxPort);

                if(ulTemp > ulPLLRefClk)
                {
                    ulPLLRefClk = ulTemp;
                }
                MS_SLEEP(5);
            }

            ulPLLRefClk = (ulPLLRefClk * 100);
            ulPLLRefClk <<= (dvProAV_HdmiRxBCRGet(sRxPort))? 2 : 0;
            ulPLLRefClk *= (dvProAV_SclOverSampleGet(sRxPort))? 5 : 1;
            ulPLLRefClk /= 4;

            LOG_MSG(db_HAL_SCALER, "ucIptFrameRate = %f\r\n", m_sProAV_Info.sRxPortAVI_Info[(RxPort)halScaler_CurrentSourceRxPortGet(ucCH, halScaler_InputSource_Get(ucCH))].dFrameRate);
            LOG_MSG(db_HAL_SCALER, "ucOptFrameRate = %d\r\n", ucOptFrameRate);
            LOG_MSG(db_HAL_SCALER, "ulPLLRefClk = %d\r\n", ulPLLRefClk);

            if((ucIptFrameRate == 0) || (ulPLLRefClk == 0) || (ucOptFrameRate == 0))	//Cassper_ProAV
            {
                halScaler_SemaphoreGive(0);
                //ASSERT_ALWAYS();
                return eHAL_SCALER_EXEC_CODE_FAIL;
            }

            if(ucIptFrameRate == ucOptFrameRate)
            {
                bSupFrameSync = true;
            }
            else
            {
                if(ucOptFrameRate > ucIptFrameRate) // ProAV_Rex_0032
                {
                    bSupFrameSync = ((ucOptFrameRate % ucIptFrameRate) == 0)? true : false;
                    ucTimes = ucOptFrameRate / ucIptFrameRate;
                    ucTimes--;
                }
                else
                {
                    bSupFrameSync = ((ucIptFrameRate % ucOptFrameRate) == 0)? true : false;
                    ucTimes = 0;
                }
            }
            if(bSupFrameSync)
                LOG_MSG(db_HAL_SCALER, "Frame rate = %d, It's support frame sync mode\r\n", ucIptFrameRate);
            else
                LOG_MSG(db_ALWAYS, "Frame rate = %d, Not support frame sync mode\r\n", ucIptFrameRate);
        }
        else if(eMode == eSclSyncMode_Off)
        {
            bSupFrameSync = true;
        }

        //iTemp = dvPro_InputFrameRate_Get((eMCT)ucCH);

        if((iInputFrameRate > 23.97 && iInputFrameRate < 23.99) ||
            (iInputFrameRate > 29.96 && iInputFrameRate < 29.98) ||
            (iInputFrameRate > 59.92) && (iInputFrameRate < 59.96) || //59.94hz
            (iInputFrameRate > 119.86) && (iInputFrameRate < 119.90)|| //59.94hz*2 = 119.88hz
            (iInputFrameRate > 239.72) && (iInputFrameRate < 239.80))   //59.94hz*4 = 239.76hz
        {
            ucIs5994 = 1;
        }
        else
        {
            ucIs5994 = 0;
        }

        if(ePROAV_EXEC_CODE_PASS != dvPro_GetPanelInfo(dvPro_PanelGet(), (UINT8*)&sPanelInfo))
        {
            halScaler_SemaphoreGive(0);
            ASSERT_ALWAYS();
            return eHAL_SCALER_EXEC_CODE_FAIL;
        }


        if(bSupFrameSync && bOptType) // if bOptType 1 is on board, else single board debug
        {
            sLMK03328_CFG sLMK03328Cfg = {0};

            // get PLL refence clock    // ProAV_Rex_0025 start

#if 0
            if(ePROAV_EXEC_CODE_PASS == dvPro_GetPanelInfo(dvPro_PanelGet(), (UINT8*)&sPanelInfo))
            {
                ulRefClk = sPanelInfo.HTotal*sPanelInfo.VTotal;
                ulRefClk = ulRefClk*(sPanelInfo.FrameRate/100);
                ulRefClk = ulRefClk/60; //用600M做基準換算
                ulRefClk = ulRefClk/4;
                ulRefClk = (ulRefClk/100);

                if(ucIs5994)
                {
                    ulRefClk = ulRefClk*119.87;
                }
                else
                {
                    ulRefClk = ulRefClk*119.99;
                }

                if(ulRefClk < ((VX1_REF_CLK*3)/4))
                {
                    ulRefClk = ulRefClk*2;
                }
                else if(ulRefClk > (VX1_REF_CLK + VX1_REF_CLK/2))
                {
                    ulRefClk = ulRefClk/2;
                }


                //if(ucIs5994)   //59.94hz*4 = 239.76hz
                //{
                    //if(ulRefClk > VX1_REF_CLK_5994)
                    //{
                        //ulRefClk = VX1_REF_CLK_5994;
                    //}
                //}
                //else
                {
                    if(ulRefClk > VX1_REF_CLK_6000)
                    {
                        ulRefClk = VX1_REF_CLK_6000;
                    }
                }
                ulRefClk = VX1_REF_CLK_6000;
            }
            else
            {
                if(ucIs5994)   //59.94hz*4 = 239.76hz
                {
                    ulRefClk = VX1_REF_CLK_5994;
                }
                else
                {
                    ulRefClk = VX1_REF_CLK_6000;
                }
            }
#else
            if(ucIs5994)   //59.94hz*4 = 239.76hz
            {
                ulRefClk = VX1_REF_CLK_5994;
            }
            else
            {
                ulRefClk = VX1_REF_CLK_6000;
            }
#endif /* 0 */

            if(eMode == eSclSyncMode_Off)
            {
                ulPLLRefClk = VCXO_REF_CLK;
                ulRefClk = VX1_REF_CLK;
            }

            ucInputDivider = (ulPLLRefClk < PLL_LMK03328_INPUT_CLK_MIN)? 1 :
                             (ulPLLRefClk < PLL_LMK03328_INPUT_CLK_MAX)? 2 :
                             ((ulPLLRefClk * 2) % PLL_LMK03328_INPUT_CLK_MIN > 0)? ((ulPLLRefClk * 2) / PLL_LMK03328_INPUT_CLK_MIN) + 1 : ((ulPLLRefClk * 2) / PLL_LMK03328_INPUT_CLK_MIN);

            LOG_MSG(db_HAL_SCALER, "Front ucInputDrivder = %d\r\n", ucInputDivider);

            // PLL parameter calculation
            ulPLLNDividerR = ulPLLRefClk / ucInputDivider;
            uiTargetRatioInt = ulRefClk / ulPLLNDividerR;
            ulTargetRatioN = ulRefClk % ulPLLNDividerR;
            uiNDivider = (uiTargetRatioInt * PLL_LMK03328_OUTPUT_DIVIDER) + ((ulTargetRatioN * PLL_LMK03328_OUTPUT_DIVIDER) / ulPLLNDividerR);
            ulPLLNDividerN = ((ulTargetRatioN * PLL_LMK03328_OUTPUT_DIVIDER) % ulPLLNDividerR);

            //LOG_MSG(db_ALWAYS, "sRxPort = %d\r\n", sRxPort);
            //LOG_MSG(db_ALWAYS, "ulRefClk = %d\r\n", ulRefClk);
            //LOG_MSG(db_ALWAYS, "ulPLLRefClk = %d\r\n", ulPLLRefClk);
            //LOG_MSG(db_ALWAYS, "uiNDivider = %d\r\n", uiNDivider);
            //LOG_MSG(db_ALWAYS, "ulPLLNDividerR = %d\r\n", ulPLLNDividerR);
            //LOG_MSG(db_ALWAYS, "ulPLLNDividerN = %d\r\n", ulPLLNDividerN);

            sLMK03328Cfg.ucChannel = eLMK03328PLL_CH1;
            sLMK03328Cfg.ucDrivder = PLL_LMK03328_RDIVIDER;
            sLMK03328Cfg.ucPLL_OUT= PLL_LMK03328_PLL_OUT_DIVIDER;
            sLMK03328Cfg.ucSelect = ((eMode == eSclSyncMode_Off)? eLMK03328PLL_REF_SELECT_SECR:eLMK03328PLL_REF_SELECT_PRIR);
            sLMK03328Cfg.uiNDivider = uiNDivider;
            sLMK03328Cfg.ulPLLNDividerN = ulPLLNDividerN;
            sLMK03328Cfg.ulPLLNDividerR = ulPLLNDividerR;
            sLMK03328Cfg.ucInputDrivder = ucInputDivider - 1;

            LOG_MSG(db_HAL_SCALER, "HTotal = %d, VTotal = %d\n", sPanelInfo.HTotal, sPanelInfo.VTotal);
            LOG_MSG(db_HAL_SCALER, "ucIs5994 = %d\r\n", ucIs5994);
            LOG_MSG(db_HAL_SCALER, "ulPLLRefClk = %d\r\n", ulPLLRefClk);
            LOG_MSG(db_HAL_SCALER, "ucChannel = %d\r\n", sLMK03328Cfg.ucChannel);
            LOG_MSG(db_HAL_SCALER, "ucDrivder = %d\r\n", sLMK03328Cfg.ucDrivder);
            LOG_MSG(db_HAL_SCALER, "ucPLL_OUT = %d\r\n", sLMK03328Cfg.ucPLL_OUT);
            LOG_MSG(db_HAL_SCALER, "ucSelect = %d\r\n", sLMK03328Cfg.ucSelect);
            LOG_MSG(db_HAL_SCALER, "uiNDivider = %d\r\n", sLMK03328Cfg.uiNDivider);
            LOG_MSG(db_HAL_SCALER, "ulPLLNDividerN = %d\r\n", sLMK03328Cfg.ulPLLNDividerN);
            LOG_MSG(db_HAL_SCALER, "ulPLLNDividerR = %d\r\n", sLMK03328Cfg.ulPLLNDividerR);
            LOG_MSG(db_HAL_SCALER, "ucInputDrivder = %d\r\n", sLMK03328Cfg.ucInputDrivder);
            LOG_MSG(db_HAL_SCALER, "ulRefClk = %d\r\n", ulRefClk);
            // ProAV_Rex_0025 end

            // PLL setting

            if(rcSUCCESS == dvProAV_SclFrameSyncModeGet(&ucCurrentMode))
            {
                //dvProAV_SclExtPllSourceGet(&ucCurrentPort);

                if((ucCurrentMode != eMode) || (m_ucLMK03328_Select != sLMK03328Cfg.ucSelect))// || sRxPort != ucCurrentPort)
                {
                    //halFrontEndCtrl_Lmk03328_PLLSet(eLMK03328PLL_CH1, PLL_LMK03328_RDIVIDER, uiNDivider, ulPLLNDividerR, ulPLLNDividerN, PLL_LMK03328_PLL_OUT_DIVIDER); // set 03328 PLL divider
                    //halFrontEndCtrl_Lmk03328_PLLSourceSelect(((eMode == eFrameSync_Off)? eLMK03328PLL_REF_SELECT_SECR:eLMK03328PLL_REF_SELECT_PRIR), eLMK03328PLL_CH1); // set 03328 PLL refence source
                    m_ucLMK03328_Select = sLMK03328Cfg.ucSelect;
                    dvProAV_OSD_SW_Reset(true);
                    halMCU_LMK03228_Setting((UINT8*)&sLMK03328Cfg, sizeof(sLMK03328Cfg));

                    MS_SLEEP(200);

#if 0
                    //if(ucTimes != 0)
                    //{
                        //3D不開AB buffer
                        switch(dvPro_Input_3D_Format_Config_Get())
                        {
                            case eINPUT_3D_TYPE_FRAMEPACKING:
                            case eINPUT_3D_TYPE_TOPANDBOTTOM:
                            case eINPUT_3D_TYPE_SIDEBYSIDE:
                                ucTimes = 0xFF;
                                break;
                        }
                    //}
#endif /* 0 */

                    // Frame sync mode setting
                    eResult = dvProAV_SclFrameSyncModeSet((SclEntity)ucCH, sRxPort, (SclFrameSyncMode)eMode, ucTimes);   // set frame sync mode

                    if(eResult == rcSUCCESS)
                    {
#if 1
                        LOG_MSG(db_HAL_SCALER, "dvPro_PanelGet() = %d\r\n", dvPro_PanelGet());
                        //if(dvPro_PanelGet() == PANEL_3D_OUTPUT)
                        LOG_MSG(db_HAL_SCALER, "dFrameRate = %f\r\n", iInputFrameRate);

                        if((iInputFrameRate > 0) && (eMode != eSclSyncMode_Off))
                        {
                            double dInputFrameRate = (double)((UINT16)(iInputFrameRate*100))/100; //去小數點第3位
                            double dFrameNum = round((double)sPanelInfo.FrameRate / 100.0 / iInputFrameRate);
                            double dVopVtotal = ((double)ulRefClk * 400.0 / (double)dFrameNum / (double)sPanelInfo.HTotal /dInputFrameRate);

                            LOG_MSG(db_HAL_SCALER, "sPanelInfo.HTotal = %d, sPanelInfo.VTotal = %d dInputFrameRate = %f, dVopVtotal = %f\r\n", sPanelInfo.HTotal, sPanelInfo.VTotal, dInputFrameRate, dVopVtotal);

                            UINT32 lVopVtotal = (UINT32)ceil(dVopVtotal);

                            if((dvPro_PanelGet() == PANEL_2D_OUTPUT) && (lVopVtotal & 0x01))
                            {
                                lVopVtotal += 1;
                            }

                            LOG_MSG(db_HAL_SCALER, "FrameNum = %f, VopVtotal = %d\r\n", dFrameNum, lVopVtotal);
                            dvProAV_AccessWrite(eVopVTot, lVopVtotal);
                        }
                        else
                        {
                            dvProAV_AccessWrite(eVopVTot, sPanelInfo.VTotal);
                        }
#else


                        if(dvPro_PanelGet() == PANEL_2D_OUTPUT)
                        {
                            if(ucIs5994)
                            {
                                dvProAV_AccessWrite(eVopVTot,    sPanelInfo.VTotal + 2);
                            }
                            else
                            {
                                dvProAV_AccessWrite(eVopVTot,    sPanelInfo.VTotal);
                            }
                        }

                        if(dvPro_PanelGet() == PANEL_2D_HIGHSPEED)
                        {
                            if(ucIs5994)
                            {
                                dvProAV_AccessWrite(eVopVTot,    sPanelInfo.VTotal + 1);
                            }
                            else
                            {
                                dvProAV_AccessWrite(eVopVTot,    sPanelInfo.VTotal);
                            }
                        }

                        LOG_MSG(db_HAL_SCALER, "ucTimes = %d\r\n", ucTimes);
#endif
                        eHalResult = eHAL_SCALER_EXEC_CODE_PASS;
                    }
                    else
                    {
                        if((dvPro_PanelGet() == PANEL_2D_OUTPUT) || (dvPro_PanelGet() == PANEL_2D_HIGHSPEED))
                        {
                            dvProAV_AccessWrite(eVopVTot, sPanelInfo.VTotal);
                        }
                        LOG_MSG(db_HAL_SCALER, "vcxo_clk unlock\r\n");

                        eHalResult = eHAL_SCALER_EXEC_CODE_FAIL;
                    }
                }
                else
                {
                    //ASSERT(eMode == eSclSyncMode_Off);
                    eResult = dvProAV_SclFrameSyncModeSet((SclEntity)ucCH, sRxPort, (SclFrameSyncMode)eMode, ucTimes);
                    LOG_MSG(db_HAL_SCALER, "\n Frame sync mode as same as setting\r\n");
                }
            }
        }
        else
        {
            if(bOptType)
            {
                dvProAV_SclFrameSyncModeSet((SclEntity)ucCH, sRxPort, eSclSyncMode_Off, ucTimes);   // set frame sync mode

                if((dvPro_PanelGet() == PANEL_2D_OUTPUT) || (dvPro_PanelGet() == PANEL_2D_HIGHSPEED))
                {
                    dvProAV_AccessWrite(eVopVTot, sPanelInfo.VTotal);
                }
            }
            else //for single board debug
            {
                if(eMode == eSclSyncMode_Off)
                {
                    dvProAV_SclAbBufModeSet(eSclEntity_Main, true);
                    dvProAV_SclAbBufModeSet(eSclEntity_Sub, true);
                }
                else
                {
                    if(bSupFrameSync)
                    {
                        dvProAV_SclAbBufModeSet(eSclEntity_Main, false);
                        dvProAV_SclAbBufModeSet(eSclEntity_Sub, false);
                    }
                    else
                    {
                        dvProAV_SclAbBufModeSet(eSclEntity_Main, true);
                        dvProAV_SclAbBufModeSet(eSclEntity_Sub, true);
                    }
                }
            }
        }
        dvProAV_OSD_SW_Reset(false);

        halScaler_SemaphoreGive(0);
    }
    else
    {
        LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore\r\n", __FUNCTION__, __LINE__);
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }

    return eHalResult;
}

eHAL_SCALER_EXEC_CODE halScaler_FrameSyncModeGet(UINT8* cMode)
{
    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        dvProAV_SclFrameSyncModeGet(cMode);

        halScaler_SemaphoreGive(0);
    }
    else
    {
        LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore\r\n", __FUNCTION__, __LINE__);
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }

    return eHAL_SCALER_EXEC_CODE_PASS;
}

#if 0
eHAL_SCALER_EXEC_CODE halScaler_FrameSyncMode_TestSet(UINT8 ucCH, UINT8 eMode, bool bOptType)    // ProAV_Rex_0032
{
    UINT32 ulRefClk = 0, ulPLLRefClk = 0;
    UINT32 ulPLLNDividerR = 0, ulPLLNDividerN = 0, ulTargetRatioN = 0;
    UINT16 uiNDivider = 0, uiTargetRatioInt = 0;
    RxPort sRxPort = eRxPort_0;
    UINT8 ucTimes = 0;
    UINT8 ucIs5994 = 0;
    bool bSupFrameSync = 1;
    UINT8 ucIptFrameRate = 0, ucOptFrameRate = 0, ucInputDivider = 0;
    double iTemp = 0;
    PanelTiming sPanelInfo = {0};

    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        // 判斷是否為支援frame sync mode的frame rate
        if(eMode != eSclSyncMode_Off)
        {
            ucIptFrameRate = (UINT8)dvPro_InputFrameRate_Get((eMCT)ucCH);
            ucOptFrameRate = (UINT8)dvPro_PanelFrameRateGet();

            dvProAV_SclVipSourceGet((SclEntity)ucCH, &sRxPort);    // get source Rx port

            ulPLLRefClk = (dvProAV_HdmiRxTmdsClkGet(sRxPort) * 100);
            ulPLLRefClk <<= (dvProAV_HdmiRxBCRGet(sRxPort))? 2 : 0;
            ulPLLRefClk *= (dvProAV_SclOverSampleGet(sRxPort))? 5 : 1;
            ulPLLRefClk /= 4;

            if((ucIptFrameRate >= 23) && (ucIptFrameRate <= 24))
            {
                ucIptFrameRate = 24;
            }

            if((ucIptFrameRate >= 29) && (ucIptFrameRate <= 31))
            {
                ucIptFrameRate = 30;
            }

            if((ucIptFrameRate >= 59) && (ucIptFrameRate <= 61))
            {
                ucIptFrameRate = 60;
            }

            if((ucIptFrameRate >= 119) && (ucIptFrameRate <= 121))
            {
                ucIptFrameRate = 120;
            }

            if((ucIptFrameRate >= 238) && (ucIptFrameRate <= 242))
            {
                ucIptFrameRate = 240;
            }

            LOG_MSG(db_HAL_SCALER, "ucIptFrameRate = %d\r\n", ucIptFrameRate);
            LOG_MSG(db_HAL_SCALER, "ucOptFrameRate = %d\r\n", ucOptFrameRate);
            LOG_MSG(db_HAL_SCALER, "ulPLLRefClk = %d\r\n", ulPLLRefClk);

            if((ucIptFrameRate == 0) || (ulPLLRefClk == 0) || (ucOptFrameRate == 0))	//Cassper_ProAV
            {
                halScaler_SemaphoreGive(0);
                ASSERT_ALWAYS();
                return eHAL_SCALER_EXEC_CODE_FAIL;
            }

            if(ucIptFrameRate == ucOptFrameRate)
            {
                bSupFrameSync = true;
            }
            else
            {
                if(ucOptFrameRate > ucIptFrameRate) // ProAV_Rex_0032
                {
                    bSupFrameSync = ((ucOptFrameRate % ucIptFrameRate) == 0)? true : false;
                    ucTimes = ucOptFrameRate / ucIptFrameRate;
                    ucTimes--;
                }
                else
                {
                    bSupFrameSync = ((ucIptFrameRate % ucOptFrameRate) == 0)? true : false;
                    ucTimes = 0;
                }
            }
            if(bSupFrameSync)
                LOG_MSG(db_HAL_SCALER, "Frame rate = %d, It's support frame sync mode\r\n", ucIptFrameRate);
            else
                LOG_MSG(db_ALWAYS, "Frame rate = %d, Not support frame sync mode\r\n", ucIptFrameRate);
        }

        iTemp = dvPro_InputFrameRate_Get((eMCT)ucCH);

        if((iTemp > 29.96 && iTemp < 29.98) ||
            (iTemp > 59.93) && (iTemp < 59.95) || //59.94hz
            (iTemp > 119.86) && (iTemp < 119.90)|| //59.94hz*2 = 119.88hz
            (iTemp > 239.72) && (iTemp < 239.80))   //59.94hz*4 = 239.76hz
        {
            ucIs5994 = 1;
        }
        else
        {
            ucIs5994 = 0;
        }

        if(ePROAV_EXEC_CODE_PASS != dvPro_GetPanelInfo(dvPro_PanelGet(), (UINT8*)&sPanelInfo))
        {
            halScaler_SemaphoreGive(0);
            ASSERT_ALWAYS();
            return eHAL_SCALER_EXEC_CODE_FAIL;
        }


        if(bSupFrameSync)
        {
            sLMK03328_CFG sLMK03328Cfg = {0};

            // get PLL refence clock    // ProAV_Rex_0025 start

#if 0
            if(ePROAV_EXEC_CODE_PASS == dvPro_GetPanelInfo(dvPro_PanelGet(), (UINT8*)&sPanelInfo))
            {
                ulRefClk = sPanelInfo.HTotal*sPanelInfo.VTotal;
                ulRefClk = ulRefClk*(sPanelInfo.FrameRate/100);
                ulRefClk = ulRefClk/60; //用600M做基準換算
                ulRefClk = ulRefClk/4;
                ulRefClk = (ulRefClk/100);

                if(ucIs5994)
                {
                    ulRefClk = ulRefClk*119.87;
                }
                else
                {
                    ulRefClk = ulRefClk*119.99;
                }

                if(ulRefClk < ((VX1_REF_CLK*3)/4))
                {
                    ulRefClk = ulRefClk*2;
                }
                else if(ulRefClk > (VX1_REF_CLK + VX1_REF_CLK/2))
                {
                    ulRefClk = ulRefClk/2;
                }


                //if(ucIs5994)   //59.94hz*4 = 239.76hz
                //{
                    //if(ulRefClk > VX1_REF_CLK_5994)
                    //{
                        //ulRefClk = VX1_REF_CLK_5994;
                    //}
                //}
                //else
                {
                    if(ulRefClk > VX1_REF_CLK_6000)
                    {
                        ulRefClk = VX1_REF_CLK_6000;
                    }
                }
                ulRefClk = VX1_REF_CLK_6000;
            }
            else
            {
                if(ucIs5994)   //59.94hz*4 = 239.76hz
                {
                    ulRefClk = VX1_REF_CLK_5994;
                }
                else
                {
                    ulRefClk = VX1_REF_CLK_6000;
                }
            }
#else
            ulRefClk = VX1_REF_CLK_6000;
#endif /* 0 */

            if(eMode == eSclSyncMode_Off)
            {
                ulPLLRefClk = VCXO_REF_CLK;
                ulRefClk = VX1_REF_CLK_6000;
            }

            ucInputDivider = (ulPLLRefClk < PLL_LMK03328_INPUT_CLK_MIN)? 1 :
                             (ulPLLRefClk < PLL_LMK03328_INPUT_CLK_MAX)? 2 :
                             ((ulPLLRefClk * 2) % PLL_LMK03328_INPUT_CLK_MIN > 0)? ((ulPLLRefClk * 2) / PLL_LMK03328_INPUT_CLK_MIN) + 1 : ((ulPLLRefClk * 2) / PLL_LMK03328_INPUT_CLK_MIN);

            LOG_MSG(db_HAL_SCALER, "Front ucInputDrivder = %d\r\n", ucInputDivider);

            // PLL parameter calculation
            ulPLLNDividerR = ulPLLRefClk / ucInputDivider;
            uiTargetRatioInt = ulRefClk / ulPLLNDividerR;
            ulTargetRatioN = ulRefClk % ulPLLNDividerR;
            uiNDivider = (uiTargetRatioInt * PLL_LMK03328_OUTPUT_DIVIDER) + ((ulTargetRatioN * PLL_LMK03328_OUTPUT_DIVIDER) / ulPLLNDividerR);
            ulPLLNDividerN = ((ulTargetRatioN * PLL_LMK03328_OUTPUT_DIVIDER) % ulPLLNDividerR);

            //LOG_MSG(db_ALWAYS, "sRxPort = %d\r\n", sRxPort);
            //LOG_MSG(db_ALWAYS, "ulRefClk = %d\r\n", ulRefClk);
            //LOG_MSG(db_ALWAYS, "ulPLLRefClk = %d\r\n", ulPLLRefClk);
            //LOG_MSG(db_ALWAYS, "uiNDivider = %d\r\n", uiNDivider);
            //LOG_MSG(db_ALWAYS, "ulPLLNDividerR = %d\r\n", ulPLLNDividerR);
            //LOG_MSG(db_ALWAYS, "ulPLLNDividerN = %d\r\n", ulPLLNDividerN);

            sLMK03328Cfg.ucChannel = eLMK03328PLL_CH1;
            sLMK03328Cfg.ucDrivder = PLL_LMK03328_RDIVIDER;
            sLMK03328Cfg.ucPLL_OUT= PLL_LMK03328_PLL_OUT_DIVIDER;
            sLMK03328Cfg.ucSelect = ((eMode == eSclSyncMode_Off)? eLMK03328PLL_REF_SELECT_SECR:eLMK03328PLL_REF_SELECT_PRIR);
            sLMK03328Cfg.uiNDivider = uiNDivider;
            sLMK03328Cfg.ulPLLNDividerN = ulPLLNDividerN;
            sLMK03328Cfg.ulPLLNDividerR = ulPLLNDividerR;
            sLMK03328Cfg.ucInputDrivder = ucInputDivider - 1;

            LOG_MSG(db_HAL_SCALER, "HTotal = %d, VTotal = %d\n", sPanelInfo.HTotal, sPanelInfo.VTotal);
            LOG_MSG(db_HAL_SCALER, "ucIs5994 = %d\r\n", ucIs5994);
            LOG_MSG(db_HAL_SCALER, "ulPLLRefClk = %d\r\n", ulPLLRefClk);
            LOG_MSG(db_HAL_SCALER, "ucChannel = %d\r\n", sLMK03328Cfg.ucChannel);
            LOG_MSG(db_HAL_SCALER, "ucDrivder = %d\r\n", sLMK03328Cfg.ucDrivder);
            LOG_MSG(db_HAL_SCALER, "ucPLL_OUT = %d\r\n", sLMK03328Cfg.ucPLL_OUT);
            LOG_MSG(db_HAL_SCALER, "ucSelect = %d\r\n", sLMK03328Cfg.ucSelect);
            LOG_MSG(db_HAL_SCALER, "uiNDivider = %d\r\n", sLMK03328Cfg.uiNDivider);
            LOG_MSG(db_HAL_SCALER, "ulPLLNDividerN = %d\r\n", sLMK03328Cfg.ulPLLNDividerN);
            LOG_MSG(db_HAL_SCALER, "ulPLLNDividerR = %d\r\n", sLMK03328Cfg.ulPLLNDividerR);
            LOG_MSG(db_HAL_SCALER, "ucInputDrivder = %d\r\n", sLMK03328Cfg.ucInputDrivder);
            LOG_MSG(db_HAL_SCALER, "ulRefClk = %d\r\n", ulRefClk);
            // ProAV_Rex_0025 end

            // PLL setting

            //halFrontEndCtrl_Lmk03328_PLLSet(eLMK03328PLL_CH1, PLL_LMK03328_RDIVIDER, uiNDivider, ulPLLNDividerR, ulPLLNDividerN, PLL_LMK03328_PLL_OUT_DIVIDER); // set 03328 PLL divider
            //halFrontEndCtrl_Lmk03328_PLLSourceSelect(((eMode == eFrameSync_Off)? eLMK03328PLL_REF_SELECT_SECR:eLMK03328PLL_REF_SELECT_PRIR), eLMK03328PLL_CH1); // set 03328 PLL refence source
            //dvProAV_OSD_SW_Reset(true);
            halMCU_LMK03228_Setting((UINT8*)&sLMK03328Cfg, sizeof(sLMK03328Cfg));

            MS_SLEEP(200);

            // Frame sync mode setting
            //dvProAV_SclFrameSyncModeSet((SclEntity)ucCH, sRxPort, (SclFrameSyncMode)eMode, ucTimes);   // set frame sync mode

            if(ucIs5994)
            {
                //dvProAV_AccessWrite(eVopVTot,    sPanelInfo.VTotal + 2);
            }
            else
            {
                //dvProAV_AccessWrite(eVopVTot,    sPanelInfo.VTotal);
            }
            LOG_MSG(db_HAL_SCALER, "ucTimes = %d\r\n", ucTimes);
            //dvProAV_SclFrameSyncModeSet(ucCH, sRxPort, eFrameSync_Off, ucTimes);
        }
        else
        {
            //dvProAV_SclFrameSyncModeSet((SclEntity)ucCH, sRxPort, eSclSyncMode_Off, ucTimes);   // set frame sync mode
            if(ucIs5994)
            {
                //dvProAV_AccessWrite(eVopVTot,    sPanelInfo.VTotal + 2);
            }
            else
            {
                //dvProAV_AccessWrite(eVopVTot,    sPanelInfo.VTotal);
            }
        }
        //dvProAV_OSD_SW_Reset(false);

        halScaler_SemaphoreGive(0);
    }
    else
    {
        LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore\r\n", __FUNCTION__, __LINE__);
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }

    return eHAL_SCALER_EXEC_CODE_PASS;
}
#endif

eHAL_SCALER_EXEC_CODE halScaler_InputFrameRateSetting(UINT8 ucCH)
{
    UINT32 ulTMDSClk = 0, ulRxPixelRepet = 0, ulColorDepth = 0;
    DOUBLE dFrameRateTemp;
    RxPort sRxPort = eRxPort_0;
    sINPUT_TIMING_INFO sInputTiming;

    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        dvPro_InputTimingGet((eMCT)ucCH, &sInputTiming);
        // get input frame rate
        dvProAV_SclVipSourceGet((SclEntity)ucCH, &sRxPort);    // get source Rx port
        ulTMDSClk = (dvProAV_HdmiRxTmdsClkGet(sRxPort));
        ulRxPixelRepet = dvProAV_HdmiRxPixelRepetGet(sRxPort);

        if((ulRxPixelRepet == 0) || (sInputTiming.uiHTotal == 0) || (sInputTiming.uiVTotal == 0)) //A70LK_Larry_0133
        {
            halScaler_SemaphoreGive(0);
            ASSERT_ALWAYS();
            return eHAL_SCALER_EXEC_CODE_FAIL;
        }

        ulTMDSClk *= (dvProAV_HdmiRxBCRGet(sRxPort))? 4 : 1;   // ProAV_Rex_0024
        ulTMDSClk /= ulRxPixelRepet;     // ProAV_Rex_0021
        ulColorDepth = dvProAV_HdmiRxColorDepthGet(sRxPort);    // ProAV_Rex_0065, start
        if((m_sProAV_Info.sRxPortAVI_Info[sRxPort].ucPixelMode == PROAV_SCALER_COLOR_PIXEL_MODE_YUV420))// && (ulColorDepth == 8))
        {
            sInputTiming.dFrameRate = (DOUBLE)((((DOUBLE)ulTMDSClk * 10000 * 8) / (DOUBLE)ulColorDepth / (DOUBLE)(sInputTiming.uiHTotal / 2) / (DOUBLE)sInputTiming.uiVTotal) + 0.005f);
        }
        else
        {
            sInputTiming.dFrameRate = (DOUBLE)((((DOUBLE)ulTMDSClk * 10000 * 8) / (DOUBLE)ulColorDepth / (DOUBLE)sInputTiming.uiHTotal / (DOUBLE)sInputTiming.uiVTotal) + 0.005f);
        }
        dFrameRateTemp = sInputTiming.dFrameRate;

        #if 0
        if(dvProAV_SrcPixelModeGet(sRxPort) == eAVI_PIXEL_MODE_YCbCr420)
        {
            sInputTiming.dFrameRate *= 2;
            dFrameRateTemp = sInputTiming.dFrameRate;
            if(ulColorDepth == 8)
            {
                dFrameRateTemp = sInputTiming.dFrameRate; // for 4K60 YVU420
                sInputTiming.dFrameRate /= 4;
            }
        }   // ProAV_Rex_0065, end
        #endif
        dvPro_InputFrameRate_Set((eMCT)ucCH, sInputTiming.dFrameRate);
        //LOG_MSG(db_ALWAYS, "Input Frame Rate = %2.2f\r\n", sInputTiming.dFrameRate);
        // set O/I Frame Rate Ratio

        UINT8 uc3DMode = halScaler_3DEnable_Get();

        if(uc3DMode == eCM_3D_FORMAT_4K3D_DUALPIPE)
        {
            dvProAV_SrcFrameRateRatioSet(sRxPort, sInputTiming.dFrameRate, 240.0, sInputTiming.uiVTotal, dvPro_PanelVTotalGet());
        }
        else
        {
            dvProAV_SrcFrameRateRatioSet(sRxPort, sInputTiming.dFrameRate, dvPro_PanelFrameRateGet(), sInputTiming.uiVTotal, dvPro_PanelVTotalGet());
        }

        sInputTiming.dFrameRate = dFrameRateTemp;

        dvPro_InputTimingSetting((eMCT)ucCH, TRUE, &sInputTiming);

        halScaler_SemaphoreGive(0);
    }
    else
    {
        LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore\r\n", __FUNCTION__, __LINE__);
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }

    return eHAL_SCALER_EXEC_CODE_PASS;
}

#if 0
eHAL_SCALER_EXEC_CODE halScaler_FrameRateSetting(UINT8 sRxPort, UINT16 uiIptHTotal, UINT16 uiIptVTotal, UINT16 uiIptVSize)
{
    UINT32 ulTMDSClk = 0, ulRxPixelRepet = 0;
    DOUBLE dInputFrameRate;

    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        ulRxPixelRepet = dvProAV_HdmiRxPixelRepetGet(sRxPort);

        if(ulRxPixelRepet == 0)
        {
            halScaler_SemaphoreGive(0);
            ASSERT_ALWAYS();
            return eHAL_SCALER_EXEC_CODE_FAIL;
        }

        // get input frame rate
        ulTMDSClk = (dvProAV_HdmiRxTmdsClkGet((RxPort)sRxPort));
        ulTMDSClk *= (dvProAV_HdmiRxBCRGet((RxPort)sRxPort))? 4 : 1;   // ProAV_Rex_0024
        ulTMDSClk /= ulRxPixelRepet;     // ProAV_Rex_0021
        dInputFrameRate = (DOUBLE)((((DOUBLE)ulTMDSClk * 10000 * 8) / (DOUBLE)dvProAV_HdmiRxColorDepthGet((RxPort)sRxPort) / (DOUBLE)uiIptHTotal / (DOUBLE)uiIptVTotal) + 0.005f);  // ProAV_Rex_0016
        //LOG_MSG(db_ALWAYS, "Input Frame Rate = %2.2f\r\n", dInputFrameRate);
        // set O/I Frame Rate Ratio
        dvProAV_SrcFrameRateRatioSet((RxPort)sRxPort, dInputFrameRate, dvPro_PanelFrameRateGet(), uiIptVTotal, dvPro_PanelVTotalGet());

        halScaler_SemaphoreGive(0);
    }
    else
    {
        LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore\r\n", __FUNCTION__, __LINE__);
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }

    return eHAL_SCALER_EXEC_CODE_PASS;
}
#endif

eHAL_SCALER_EXEC_CODE halScaler_HDRLevelSetting(UINT8 ucLevel)
{
    SclEntity ucCH = eSclEntity_Main;
    UINT8 ucHDR_EOTF = 0;

    ucHDR_EOTF = halScaler_HDR_EOTF_Get(ucCH);

    if(m_sUserSystemSetting.sImageSetting.ucHDREnable == eHDR_SELECT_OFF ||
       ucLevel == eCM_HDR_LEVEL_SDR)
    {
        dvProAV_HdrCscTransEn((SclEntity)ucCH, eHdrCscP3to709, false);
        dvProAV_Hdr10pToneMappingSet((SclEntity)ucCH, eHDRType_Off, ucLevel-1);
    }
    else if((ucLevel > eCM_HDR_LEVEL_SDR && ucLevel < eCM_HDR_LEVEL_NUMBER) &&
            (m_sUserSystemSetting.sImageSetting.ucHDREnable == eHDR_SELECT_HDR10) && (halScaler_HDR_Get(eSOURCE_WINDOW_MAIN) == eHDR_SETTING_HDR))
    {
        switch(ucHDR_EOTF)
        {
            case eEotfType_Sdr:
            case eEotfType_TraditionalGammaHDR:
            {
            }
            break;

            case eEotfType_St2084Hdr:
            {
                dvProAV_HdrCscTransEn((SclEntity)ucCH, eHdrCsc2020to709, true);
                dvProAV_Hdr10pToneMappingSet((SclEntity)ucCH, eHDRType_10, ucLevel-1);
            }
            break;

            case eEotfType_Hlg:
        	{
                dvProAV_HdrCscTransEn((SclEntity)ucCH, eHdrCsc2020to709, true);
            }
            break;

            default:
                break;
        }
    }
    else if((ucLevel > eCM_HDR_LEVEL_SDR && ucLevel < eCM_HDR_LEVEL_NUMBER) &&
             m_sUserSystemSetting.sImageSetting.ucHDREnable == eHDR_SELECT_HDR10P_LIKE && (halScaler_HDR_Get(eSOURCE_WINDOW_MAIN) == eHDR_SETTING_HDR))
    {
        switch(ucHDR_EOTF)
        {
            case eEotfType_Sdr:
            case eEotfType_TraditionalGammaHDR:
            {
        	}
            break;

            case eEotfType_St2084Hdr:
        	{
                dvProAV_HdrCscTransEn((SclEntity)ucCH, eHdrCsc2020to709, true);
                dvProAV_Hdr10pToneMappingSet((SclEntity)ucCH, eHDRType_10p, ucLevel-1);
        	}
            break;

            case eEotfType_Hlg:
        	{
                dvProAV_HdrCscTransEn((SclEntity)ucCH, eHdrCsc2020to709, true);
            }
            break;

            default:
                break;
        }
    }
    else if((ucLevel > eCM_HDR_LEVEL_SDR && ucLevel < eCM_HDR_LEVEL_NUMBER) && (halScaler_HDR_Get(eSOURCE_WINDOW_MAIN) == eHDR_SETTING_SDR) &&
            m_sUserSystemSetting.sImageSetting.ucHDREnable == eHDR_SELECT_HDR10P_LIKE)
    {
        dvProAV_HdrCscTransEn((SclEntity)ucCH, eHdrCscP3to709, false);
        dvProAV_Hdr10pToneMappingSet((SclEntity)ucCH, eHDRType_Sdr10p, ucLevel-1);
    }

    return eHAL_SCALER_EXEC_CODE_PASS;
}


eHAL_SCALER_EXEC_CODE halScaler_HDRLevelSet(UINT8 ucLevel, UINT8 ucHDRSelect)
{
    SclEntity ucCH = eSclEntity_Main;

    if(ucLevel > eCM_HDR_LEVEL_NUMBER)
    {
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }

    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        UINT8 aucData[HDR_TABLE_SIZE] = {0};

        LOG_MSG(db_HAL_SCALER, "HDR Level = %d\r\n", ucLevel);

        if(ucLevel == eCM_HDR_LEVEL_SDR)
        {
            m_sUserSystemSetting.sImageSetting.ucHDREnable  = eHDR_SELECT_OFF;
            halScaler_HDRLevelSetting(eCM_HDR_LEVEL_SDR);
        }
        else
        {
            m_sUserSystemSetting.sImageSetting.ucHDREnable  = ucHDRSelect;
            halScaler_HDRLevelSetting(ucLevel);
        }

        halScaler_SemaphoreGive(0);
    }
    else
    {
        LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore\r\n", __FUNCTION__, __LINE__);
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }

    return eHAL_SCALER_EXEC_CODE_PASS;
}

void halScaler_CurrentSourceRxPortSet(UINT8 ucCH, UINT8 ucRxPort)
{
    //因為從FPGA900改成FPGA480，port減少2個，有些功能需要特別去記錄對應的RX port
    //例如 backup input, seamlss switch

    m_sHalScalerInfo[ucCH].ucCurrentRXPort = ucRxPort;
    //halScaler_RxPortClear(m_sHalScalerInfo[ucCH].ucCurrentRXPort);

    LOG_MSG(db_HAL_HDMIRX, "CH = %d, Current RxPort = %d\r\n", ucCH, ucRxPort);
}

UINT8 halScaler_CurrentSourceRxPortGet(UINT8 ucCH, UINT8 ucSelect)
{
    UINT8 ucRx = 0;

#if 0
    UINT16 uiMainSource = m_sUserSystemSetting.sSourceSetting.ucInputSourceMain;

    if(ucCH == eMCT_CH1) // Main source
    {
        switch((eCM_SOURCE_ID)ucSelect)
        {
            case eCM_SOURCE_HDMI1:
                ucRx = PROA_RX_1;
                break;

            case eCM_SOURCE_HDMI2:
                ucRx = PROA_RX_1;
                break;

            case eCM_SOURCE_DISPLAYPORT:
                ucRx = PROA_RX_1;
                break;

            case eCM_SOURCE_12GSDI:
                ucRx = PROA_RX_1;
                break;

            case eCM_SOURCE_HDBASET:
                ucRx = PROA_RX_1;
                break;

            default:
                ucRx = 0xff; //no source
                break;
        }
    }
    else //Sub Source
    {
        switch((eCM_SOURCE_ID)ucSelect)
        {
            case eCM_SOURCE_HDMI1:
            	ucRx = PROA_RX_3;
			    break;

            case eCM_SOURCE_HDMI2:
				ucRx = PROA_RX_3;
			    break;

            case eCM_SOURCE_DISPLAYPORT:
				ucRx = PROA_RX_3;
			    break;

            case eCM_SOURCE_12GSDI:
            	ucRx = PROA_RX_3;
			    break;

            case eCM_SOURCE_HDBASET:
				ucRx = PROA_RX_3;
			    break;

            default:
                ucRx = 0xff; //no source
                break;
        }
    }
#endif /* 0 */
    //預設main RX1, sub RX3

    switch((eCM_SOURCE_ID)ucSelect)
    {
        case eCM_SOURCE_HDMI1:
        case eCM_SOURCE_HDMI2:
        case eCM_SOURCE_DISPLAYPORT:
        case eCM_SOURCE_12GSDI:
        case eCM_SOURCE_HDBASET:
            ucRx =  m_sHalScalerInfo[ucCH].ucCurrentRXPort;
            break;

        default:
            ucRx = 0xff; //no source
            break;
    }

    return ucRx;
}

#if 0
UINT8 halScaler_CurrentSourceRxPortGet(UINT8 ucCH, UINT8 ucSelect)
{
    UINT8 ucRx = 0;

    switch((eCM_SOURCE_ID)ucSelect)
    {
        case eCM_SOURCE_HDMI1:
#ifdef FPGA_ENTRY_4K
            if((eMCT)ucCH == eMCT_CH2)
            {
                ucRx = PROA_RX_3;
            }
            else
            {
                ucRx = PROA_RX_1;
            }
#else
            ucRx = PROA_RX_1;
#endif /* FPGA_ENTRY_4K */
            break;

        case eCM_SOURCE_HDMI2:
#ifdef FPGA_ENTRY_4K
            if((eMCT)ucCH == eMCT_CH2)
            {
                ucRx = PROA_RX_3;
            }
            else
            {
                ucRx = PROA_RX_1;
            }
#else
            ucRx = PROA_RX_2;
#endif /* FPGA_ENTRY_4K */
            break;

        case eCM_SOURCE_DVI:
            if((eMCT)ucCH == eMCT_CH2)
            {
#ifdef FPGA_ENTRY_4K
                ucRx = PROA_RX_3;
#else
                ucRx = PROA_RX_4;
#endif /* FPGA_ENTRY_4K */
            }
            else
            {
                ucRx = PROA_RX_3;
            }
            break;

        case eCM_SOURCE_DISPLAYPORT:
            if((eMCT)ucCH == eMCT_CH2)
            {
#ifdef FPGA_ENTRY_4K
                ucRx = PROA_RX_3;
#else
                ucRx = PROA_RX_4;
#endif /* FPGA_ENTRY_4K */
            }
            else
            {
                ucRx = PROA_RX_3;
            }
            break;

        case eCM_SOURCE_3GSDI:
            if((eMCT)ucCH == eMCT_CH2)
            {
#ifdef FPGA_ENTRY_4K
                ucRx = PROA_RX_3;
#else
                ucRx = PROA_RX_4;
#endif /* FPGA_ENTRY_4K */
            }
            else
            {
                ucRx = PROA_RX_3;
            }
            break;

        case eCM_SOURCE_HDBASET:
            if((eMCT)ucCH == eMCT_CH2)
            {
#ifdef FPGA_ENTRY_4K
                ucRx = PROA_RX_3;
#else
                ucRx = PROA_RX_4;
#endif /* FPGA_ENTRY_4K */
            }
            else
            {
                ucRx = PROA_RX_3;
            }
            break;

        default:
            ucRx = 0xff; //no source
            break;
    }

    return ucRx;
}
#endif /* 0 */

void halScaler_RxPortClear(UINT8 ucRxPort)
{
    if(ucRxPort < eRxPort_Max)
    {
        m_sProAV_Info.ucRxPortStatus[ucRxPort] = eInpStatusUnstable;

        m_sProAV_Info.ucRxPortStatusCount[ucRxPort] = 5;
        memset((UINT8*)&m_sProAV_Info.sRxPortDetTiming[ucRxPort], 0, PROAV_DET_TIMING_SIZE);
        memset((UINT8*)&m_sProAV_Info.sRxPortAVI_Info[ucRxPort], 0, PROAV_AVI_INFO_SIZE);
        memset((UINT8*)&m_sProAV_Info.sRxPortAVIMonitor_Info[ucRxPort], 0, PROAV_AVI_INFO_SIZE); //A70LK_Larry_0133
        memset((UINT8*)&m_sProAV_Info.ucAVIInfo[ucRxPort], 0, AVI_INFO_SIZE);

    }
}

eHAL_SCALER_EXEC_CODE halScaler_RxPortCheck(UINT8 ucRxPort, UINT8 ucChangeSourceReset)
{
    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        if(m_sUserSystemSetting.sCommonSetting.ucAutoSourceResync == 0 || ucChangeSourceReset == FALSE)
        {
            if(ucRxPort < eRxPort_Max)
            {
                int status = rcSUCCESS;
                UINT32 changed = 0;
                UINT32 uiRxPortChanged[eRxPort_Max] = {eBiuRx0TmgChg, eBiuRx1TmgChg, eBiuRx2TmgChg, eBiuRx3TmgChg};

                dvProAV_SclInputPortInfoClear(ucRxPort);

                MS_SLEEP(50);
                status = dvProAV_AccessRead(uiRxPortChanged[ucRxPort], &changed);

                if(status == rcSUCCESS)
                {
                    if(changed != 0)
                    {
                        halScaler_RxPortClear(ucRxPort);
                    }
                }

                dvProAV_SclInputPortInfoClear(ucRxPort);
            }
        }
        else
        {
             halScaler_RxPortClear(ucRxPort);
             dvProAV_SclInputPortInfoClear(ucRxPort);
        }

        halScaler_SemaphoreGive(0);

        return eHAL_SCALER_EXEC_CODE_PASS;
    }
    else
    {
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }
}


eHAL_SCALER_EXEC_CODE halScaler_RxPortCheckPolling(void)
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    int status = rcSUCCESS;

    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        InputPortStatus PortStatus = eInpStatusUnstable;
        RxPort ucRxPort = eRxPort_0;
        UINT32 uiRxPortActive[eRxPort_Max] = {eRx0InfoChActive, eRx1InfoChActive, eRx2InfoChActive, eRx3InfoChActive};
        UINT32 uiRxPortChanged[eRxPort_Max] = {eBiuRx0TmgChg, eBiuRx1TmgChg, eBiuRx2TmgChg, eBiuRx3TmgChg};
        UINT32 changed = 0;
        UINT32 stable = 0;
        AVI_INFOFRAME sAVIInfo;
        UINT8 ucVSIInfo[8];
        HDR_INFOFRAME_PROAV sHDRInfo;

        for(ucRxPort = eRxPort_0; ucRxPort < eRxPort_Max; ucRxPort++)
        {
#ifdef FPGA_ENTRY_4K
            if(ucRxPort == eRxPort_1 || ucRxPort == eRxPort_3)
            {
                continue;
            }
#endif /* FPGA_ENTRY_4K */

            status = rcSUCCESS;
            changed = 255;
            stable = 255;

            status = dvProAV_AccessRead(uiRxPortActive[ucRxPort], &stable);

            if(m_sProAV_Info.ucRxPortStatusCount[ucRxPort])
            {
                LOG_MSG(db_HAL_HDMIRX, "[%d][%d]stable = %d\r\n", m_sProAV_Info.ucRxPortStatusCount[ucRxPort], ucRxPort, stable);
            }

            if(status == rcSUCCESS)
            {
                if(stable)
                {
                    status = dvProAV_AccessRead(uiRxPortChanged[ucRxPort], &changed);

                    if(m_sProAV_Info.ucRxPortStatusCount[ucRxPort])
                    {
                        LOG_MSG(db_HAL_HDMIRX, "[%d][%d]changed = %d\r\n", m_sProAV_Info.ucRxPortStatusCount[ucRxPort], ucRxPort, changed);
                    }

                    if(status == rcSUCCESS)
                    {
                        if(changed == 1)
                        {
                            PortStatus = eInpStatusChanged;
                        }
                        else
                        {
                            PortStatus = eInpStatusStable;
                        }
                    }
                    else
                    {
                        PortStatus = eInpStatusUnstable;
                    }
                }
                else
                {
                    PortStatus = eInpStatusUnstable;

                    //Check HDMI TMDS
                    if(m_cHDMIReDriverCheck[ucRxPort])
                    {
                        UINT32 ulTMDSClk = 0;
                        BOOL   bBitClockRatio = false; //false 1.4, true 2.0

                        ulTMDSClk = dvProAV_HdmiRxTmdsClkGet(ucRxPort);
                        if(ulTMDSClk != 0)
                            LOG_MSG(db_HAL_HDMIRX, "[%d] TMDSClk = %d (%d)\r\n", ucRxPort, ulTMDSClk, m_cHDMIReDriverCheck[ucRxPort]);

                        if(ulTMDSClk > 14800) //TMDS is over 1080p60 148.5M
                        {
                            bBitClockRatio = dvProAV_HdmiRxBCRGet(ucRxPort);

                            if(bBitClockRatio == false) //HDMI 1.4
                            {
                                m_cHDMIReDriverCheck[ucRxPort]--;

                                if(m_cHDMIReDriverCheck[ucRxPort] == 0)
                                {
                                    LOG_MSG(db_HAL_HDMIRX, "[%d] Set ReDriver\r\n", ucRxPort);
                                    halMCU_PI3HDX1204_Setting(ucRxPort, 1);
                                }
                            }

                        }
                    }
                }

                if(PortStatus == eInpStatusStable)
                {
                    if((m_sProAV_Info.ucRxPortStatus[ucRxPort] != PortStatus) && (m_sProAV_Info.ucRxPortStatusCount[ucRxPort] == 0))
                    {
                        uOPD_DATA ucOPDData = {0};
                        UINT32 value = 0;
                        UINT32 ulTMDSClk = 0, ulColorDepth = 0, ulRxPixelRepet = 0;
                        BOOL   bHDMI2_0 = 0;
                        DOUBLE dInputFrameRate = 0;
                        UINT8  cCurrentSource = 0;
#if 1
#if 0
                        UINT32 uiRxPortHToal[eRxPort_Max]  =    {eInBrgDetHTot0,    eInBrgDetHTot1,     eInBrgDetHTot2,     eInBrgDetHTot3  };
                        UINT32 uiRxPortHStart[eRxPort_Max] =    {eInBrgDetHSt0,     eInBrgDetHSt1,      eInBrgDetHSt2,      eInBrgDetHSt3   };
                        UINT32 uiRxPortHSize[eRxPort_Max]  =    {eInBrgDetHSize0,   eInBrgDetHSize1,    eInBrgDetHSize2,    eInBrgDetHSize3 };
                        UINT32 uiRxPortHPol[eRxPort_Max]   =    {eInBrgDetHs0Pol,   eInBrgDetHs1Pol,    eInBrgDetHs2Pol,    eInBrgDetHs3Pol };
                        UINT32 uiRxPortHWid[eRxPort_Max]   =    {eInBrgDetHs0Wid,   eInBrgDetHs1Wid,    eInBrgDetHs2Wid,    eInBrgDetHs3Wid };
                        UINT32 uiRxPortVToal[eRxPort_Max]  =    {eInBrgDetVTot0,    eInBrgDetVTot1,     eInBrgDetVTot2,     eInBrgDetVTot3  };
                        UINT32 uiRxPortVStart[eRxPort_Max] =    {eInBrgDetVSt0,     eInBrgDetVSt1,      eInBrgDetVSt2,      eInBrgDetVSt3   };
                        UINT32 uiRxPortVSize[eRxPort_Max]  =    {eInBrgDetVSize0,   eInBrgDetVSize1,    eInBrgDetVSize2,    eInBrgDetVSize3 };
                        UINT32 uiRxPortVPol[eRxPort_Max]   =    {eInBrgDetVs0Pol,   eInBrgDetVs1Pol,    eInBrgDetVs2Pol,    eInBrgDetVs3Pol };
                        UINT32 uiRxPortVWid[eRxPort_Max]   =    {eInBrgDetVs0Wid,   eInBrgDetVs1Wid,    eInBrgDetVs2Wid,    eInBrgDetVs3Wid };
                        UINT32 uiRxPortInt[eRxPort_Max]    =    {eInBrgDetIntlc0,   eInBrgDetIntlc1,    eInBrgDetIntlc2,    eInBrgDetIntlc3 };


                        LOG_MSG(db_HAL_SOURCE_INFO, "ucRxPort %d is ready\r\n", ucRxPort);

                        dvProAV_AccessRead(uiRxPortHToal[ucRxPort], &value);
                        m_sProAV_Info.sRxPortDetTiming[ucRxPort].Hs.Total = (uint16)value;
                        dvProAV_AccessRead(uiRxPortHStart[ucRxPort], &value);
                        m_sProAV_Info.sRxPortDetTiming[ucRxPort].Hs.Start = (uint16)value;
                        dvProAV_AccessRead(uiRxPortHSize[ucRxPort], &value);
                        m_sProAV_Info.sRxPortDetTiming[ucRxPort].Hs.Size = (uint16)value;
                        dvProAV_AccessRead(uiRxPortHPol[ucRxPort], &value);
                        m_sProAV_Info.sRxPortDetTiming[ucRxPort].Polarity = (uint8)value;
                        dvProAV_AccessRead(uiRxPortHWid[ucRxPort], &value);
                        m_sProAV_Info.sRxPortDetTiming[ucRxPort].Hs.Sync = (uint16)value;
                        dvProAV_AccessRead(uiRxPortVToal[ucRxPort], &value);
                        m_sProAV_Info.sRxPortDetTiming[ucRxPort].Vs.Total = (uint16)value;
                        dvProAV_AccessRead(uiRxPortVStart[ucRxPort], &value);
                        m_sProAV_Info.sRxPortDetTiming[ucRxPort].Vs.Start = (uint16)value;
                        dvProAV_AccessRead(uiRxPortVSize[ucRxPort], &value);
                        m_sProAV_Info.sRxPortDetTiming[ucRxPort].Vs.Size = (uint16)value;
                        dvProAV_AccessRead(uiRxPortVPol[ucRxPort], &value);
                        m_sProAV_Info.sRxPortDetTiming[ucRxPort].Polarity = (uint8)value;
                        dvProAV_AccessRead(uiRxPortVWid[ucRxPort], &value);
                        m_sProAV_Info.sRxPortDetTiming[ucRxPort].Vs.Sync = (uint16)value;
                        dvProAV_AccessRead(uiRxPortInt[ucRxPort], &value);
                        m_sProAV_Info.sRxPortDetTiming[ucRxPort].Interlace = (bool)value;
#else
                        sRX_AVI_INFO sRxPortInfo = {0};

                        UINT32 uiRxInfo[eRxPort_Max]  =    {eInBrgDetHTot0,    eInBrgDetHTot1,     eInBrgDetHTot2,     eInBrgDetHTot3  };

                        dvProAV_AccessBurstRead(uiRxInfo[ucRxPort], (UINT8*)&sRxPortInfo, 15, true);

                        if((sRxPortInfo.uiHTotal == 0) ||
                           (sRxPortInfo.uiHStart == 0) ||
                           (sRxPortInfo.uiHSize == 0)  ||
                           (sRxPortInfo.uiVTotal == 0) ||
                           (sRxPortInfo.uiVStart == 0) ||
                           (sRxPortInfo.uiVSize == 0))
                        {
                            halScaler_SemaphoreGive(0);
                            LOG_MSG(db_HAL_SOURCE_INFO,"Video info is zero\n");
                            return eHAL_SCALER_EXEC_CODE_FAIL;
                        }

                        m_sProAV_Info.sRxPortDetTiming[ucRxPort].Hs.Total = sRxPortInfo.uiHTotal;
                        m_sProAV_Info.sRxPortDetTiming[ucRxPort].Hs.Start = sRxPortInfo.uiHStart;
                        m_sProAV_Info.sRxPortDetTiming[ucRxPort].Hs.Size  = sRxPortInfo.uiHSize;
                        m_sProAV_Info.sRxPortDetTiming[ucRxPort].Polarity = sRxPortInfo.bHPol;
                        m_sProAV_Info.sRxPortDetTiming[ucRxPort].Hs.Sync  = sRxPortInfo.cHWid;
                        m_sProAV_Info.sRxPortDetTiming[ucRxPort].Vs.Total = sRxPortInfo.uiVTotal;
                        m_sProAV_Info.sRxPortDetTiming[ucRxPort].Vs.Start = sRxPortInfo.uiVStart;
                        m_sProAV_Info.sRxPortDetTiming[ucRxPort].Vs.Size  = sRxPortInfo.uiVSize;
                        m_sProAV_Info.sRxPortDetTiming[ucRxPort].Polarity = sRxPortInfo.bVPol;
                        m_sProAV_Info.sRxPortDetTiming[ucRxPort].Vs.Sync  = sRxPortInfo.cVWid;
                        m_sProAV_Info.sRxPortDetTiming[ucRxPort].Interlace= sRxPortInfo.bInt;
#endif /* 0 */
                        // seamless switch setting
                        // AVI & VSI info frame get & setting
                        dvProAV_SrcInputVSIInfoGet(ucRxPort, ucVSIInfo);
                        dvProAV_HdmiRxAviGet(ucRxPort, &sAVIInfo);
                        //halScaler_ProAV_ColorSpaceConversion(ucCH, m_sHalScalerInfo[ucCH].ucColorsSpace, m_sHalScalerInfo[ucCH].ucPixelMode, m_sHalScalerInfo[ucCH].ucRGBFull);
                        //halScaler_ProAV_ColorSpaceSetting(ucCH, halScaler_ColorSpace_Get(eSOURCE_WINDOW_MAIN));
                        m_sProAV_Info.sRxPortAVI_Info[ucRxPort].ucColorsSpace = dvProAV_HdmiRxColorInfo2ColorSpace(&sAVIInfo);
                        m_sProAV_Info.sRxPortAVI_Info[ucRxPort].ucPixelMode = dvProAV_HdmiRxColorInfo2PixelMode(sAVIInfo.byte1.Y);
                        //m_sProAV_Info.sRxPortAVI_Info[ucRxPort].ucRGBFull = dvProAV_HdmiRxColorInfo2RGBMode(sAVIInfo.byte2.C, sAVIInfo.byte3.Q);
                        m_sProAV_Info.sRxPortAVI_Info[ucRxPort].ucRGBFull = dvProAV_HdmiRxColorInfo2RGBMode(sAVIInfo.vic, sAVIInfo.byte3.Q);
                        m_sProAV_Info.sRxPortAVI_Info[ucRxPort].uc3DMode = dvProAV_SclVSIInfo23DMode(ucVSIInfo);

                        memcpy(&m_sProAV_Info.ucAVIInfo[ucRxPort], (UINT8*)&sAVIInfo, AVI_INFO_SIZE);
                        memcpy(m_sProAV_Info.ucVSIInfo, ucVSIInfo, 8);

                        dvProAV_SclInputHDRInfoGet(ucRxPort, &sHDRInfo);
                        m_sProAV_Info.sRxPortAVI_Info[ucRxPort].ucHDR = sHDRInfo.eotf & 0x07;

                        if(m_sProAV_Info.sRxPortAVI_Info[ucRxPort].ucPixelMode == PROAV_SCALER_COLOR_PIXEL_MODE_YUV420)    // 420 mode H Total size start x2
                        {
                            m_sProAV_Info.sRxPortDetTiming[ucRxPort].Hs.Total *= 2;
                            m_sProAV_Info.sRxPortDetTiming[ucRxPort].Hs.Start *= 2;
                            m_sProAV_Info.sRxPortDetTiming[ucRxPort].Hs.Sync *= 2;
                            m_sProAV_Info.sRxPortDetTiming[ucRxPort].Hs.Size *= 2;
                        }

                        ucOPDData.sAVI_Info.cRxPort = ucRxPort;
                        memcpy(&ucOPDData.sAVI_Info.cAVIInfo[0], &sAVIInfo.checksum, 14);
                        memcpy(&ucOPDData.sAVI_Info.cVSII[0], &ucVSIInfo[0], 8);
                        memcpy(&ucOPDData.sAVI_Info.cHDRInfo[0], &sHDRInfo, 16);

                        utilOPD_EventSet(eOPD_AVI_INFOFRAME_LOG, &ucOPDData);

                        LOG_MSG(db_HAL_SOURCE_INFO,"RxPort     = %d\n",     ucRxPort);
                        LOG_MSG(db_HAL_SOURCE_INFO,"HTotal     = %d\n",     m_sProAV_Info.sRxPortDetTiming[ucRxPort].Hs.Total);
                        LOG_MSG(db_HAL_SOURCE_INFO,"HActive    = %d\n",     m_sProAV_Info.sRxPortDetTiming[ucRxPort].Hs.Size);
                        LOG_MSG(db_HAL_SOURCE_INFO,"HStart     = %d\n",     m_sProAV_Info.sRxPortDetTiming[ucRxPort].Hs.Start);
                        LOG_MSG(db_HAL_SOURCE_INFO,"VTotal     = %d\n",     m_sProAV_Info.sRxPortDetTiming[ucRxPort].Vs.Total);
                        LOG_MSG(db_HAL_SOURCE_INFO,"VActive    = %d\n",     m_sProAV_Info.sRxPortDetTiming[ucRxPort].Vs.Size);
                        LOG_MSG(db_HAL_SOURCE_INFO,"VStart     = %d\n",     m_sProAV_Info.sRxPortDetTiming[ucRxPort].Vs.Start);
                        LOG_MSG(db_HAL_SOURCE_INFO,"Interlace  = %d\n",     m_sProAV_Info.sRxPortDetTiming[ucRxPort].Interlace);

                        LOG_MSG(db_HAL_SOURCE_INFO,"sAVIInfo = [%02x][%02x][%02x][%02x][%02x][%02x][%02x][%02x][%02x][%02x][%02x][%02x][%02x][%02x]\n",
                                                                    m_sProAV_Info.ucAVIInfo[ucRxPort][0],  // Byte  0: checksum
                                                                    m_sProAV_Info.ucAVIInfo[ucRxPort][1],  // Byte  1: [Y2] Y1 Y0 A0 B1 B0 S1 S0
                                                                    m_sProAV_Info.ucAVIInfo[ucRxPort][2],  // Byte  2: C1 C0 M1 M0 R3 R2 R1 R0
                                                                    m_sProAV_Info.ucAVIInfo[ucRxPort][3],  // Byte  3: ITC EC2 EC1 EC0 Q1 Q0 SC1 SC0
                                                                    m_sProAV_Info.ucAVIInfo[ucRxPort][4],  // Byte  4: [VIC7] VIC6 VIC5 VIC4 VIC3 VIC2 VIC1 VIC0;
                                                                    m_sProAV_Info.ucAVIInfo[ucRxPort][5],  // Byte  5: YQ1 YQ0 CN1 CN0 PR3 PR2 PR1 PR0
                                                                    m_sProAV_Info.ucAVIInfo[ucRxPort][6],  // Byte  6: ETB07-ETB00
                                                                    m_sProAV_Info.ucAVIInfo[ucRxPort][7],  // Byte  7: ETB15-ETB08
                                                                    m_sProAV_Info.ucAVIInfo[ucRxPort][8],  // Byte  8: SBB07-SBB00
                                                                    m_sProAV_Info.ucAVIInfo[ucRxPort][9],  // Byte  9: SBB15-SBB08
                                                                    m_sProAV_Info.ucAVIInfo[ucRxPort][10], // Byte 10: ELB07-ELB00
                                                                    m_sProAV_Info.ucAVIInfo[ucRxPort][11], // Byte 11: ELB15-ELB08
                                                                    m_sProAV_Info.ucAVIInfo[ucRxPort][12], // Byte 12: SRB07-SRB00
                                                                    m_sProAV_Info.ucAVIInfo[ucRxPort][13]  // Byte 13: SRB15-SRB08
                                                                    );

                        LOG_MSG(db_HAL_SOURCE_INFO,"ucRxPort = %d [%02x][%02x][%02x][%02x][%02x][%02x][%02x][%02x]\n", ucRxPort,
                                                                    m_sProAV_Info.ucVSIInfo[0],
                                                                    m_sProAV_Info.ucVSIInfo[1],
                                                                    m_sProAV_Info.ucVSIInfo[2],
                                                                    m_sProAV_Info.ucVSIInfo[3],
                                                                    m_sProAV_Info.ucVSIInfo[4],
                                                                    m_sProAV_Info.ucVSIInfo[5],
                                                                    m_sProAV_Info.ucVSIInfo[6],
                                                                    m_sProAV_Info.ucVSIInfo[7]
                                                                    );


                        LOG_MSG(db_HAL_SOURCE_INFO,"Colorimetry %d\n", sAVIInfo.byte2.C);
                        LOG_MSG(db_HAL_SOURCE_INFO,"ExtendedColorimetry %d\n", sAVIInfo.byte3.EC);
                        LOG_MSG(db_HAL_SOURCE_INFO,"PixelMode %d\n", sAVIInfo.byte1.Y);
                        LOG_MSG(db_HAL_SOURCE_INFO,"ucColorsSpace %d\n", m_sProAV_Info.sRxPortAVI_Info[ucRxPort].ucColorsSpace);
                        LOG_MSG(db_HAL_SOURCE_INFO,"ucPixelMode %d\n", m_sProAV_Info.sRxPortAVI_Info[ucRxPort].ucPixelMode);
                        LOG_MSG(db_HAL_SOURCE_INFO,"ucRGBFull %d\n", m_sProAV_Info.sRxPortAVI_Info[ucRxPort].ucRGBFull);
                        LOG_MSG(db_HAL_SOURCE_INFO,"uc3DMode %d\n", m_sProAV_Info.sRxPortAVI_Info[ucRxPort].uc3DMode);


                        status &= (UINT8)dvProAV_SclBackupSourceGet(&cCurrentSource);

                        if((cCurrentSource == 0) && (dvProAV_SclDatapathGet() != eSclPath_BackupSw))
                        {
							//backup下不更新
                            // parameter setting
                            dvProAV_SrcUpdatSet(ucRxPort, true);
                            dvProAV_SrcTimingSet(ucRxPort, &m_sProAV_Info.sRxPortDetTiming[ucRxPort]);
                            dvProAV_SrcUpdatSet(ucRxPort, false);
                        }

                        // frame rate ratio setting
                        //halScaler_FrameRateSetting(ucRxPort, m_sProAV_Info.sRxPortDetTiming[ucRxPort].Hs.Total, m_sProAV_Info.sRxPortDetTiming[ucRxPort].Vs.Total, m_sProAV_Info.sRxPortDetTiming[ucRxPort].Vs.Size);
                        // get input frame rate
                        ulTMDSClk = (dvProAV_HdmiRxTmdsClkGet(ucRxPort));

                        bHDMI2_0 = dvProAV_HdmiRxBCRGet(ucRxPort);

                        ulTMDSClk *= (bHDMI2_0)? 4 : 1;   // ProAV_Rex_0024

                        ulRxPixelRepet = dvProAV_HdmiRxPixelRepetGet((RxPort)ucRxPort);

                        if(ulRxPixelRepet == 0)
                        {
                            halScaler_SemaphoreGive(0);
                            ASSERT_ALWAYS();
                            return eHAL_SCALER_EXEC_CODE_FAIL;
                        }

                        ulTMDSClk /= ulRxPixelRepet;     // ProAV_Rex_0021
                        ulColorDepth = dvProAV_HdmiRxColorDepthGet((RxPort)ucRxPort);
                        #if 0
                        if((m_sProAV_Info.sRxPortAVI_Info[ucRxPort].ucPixelMode == eAVI_PIXEL_MODE_YCbCr420) && (ulColorDepth == 8))
                        {
                            ulTMDSClk /= 4;
                        }
                        #endif

                        LOG_MSG(db_HAL_SOURCE_INFO,"ulTMDSClk %d\n", ulTMDSClk);
                        LOG_MSG(db_HAL_SOURCE_INFO,"bHDMI2_0 %d\n", bHDMI2_0);

                        if((m_sProAV_Info.sRxPortDetTiming[ucRxPort].Vs.Total == 0) || (m_sProAV_Info.sRxPortDetTiming[ucRxPort].Hs.Total == 0) || (ulColorDepth == 0))
                        {
                            ASSERT_ALWAYS();
                        }

                        if((m_sProAV_Info.sRxPortAVI_Info[ucRxPort].ucPixelMode == PROAV_SCALER_COLOR_PIXEL_MODE_YUV420))// && (ulColorDepth == 8))
                        {
                            dInputFrameRate = ((((DOUBLE)ulTMDSClk * 10000 * 8) / (DOUBLE)ulColorDepth / (DOUBLE)(m_sProAV_Info.sRxPortDetTiming[ucRxPort].Hs.Total / 2) / (DOUBLE)m_sProAV_Info.sRxPortDetTiming[ucRxPort].Vs.Total) + 0.005f);
                        }
                        else
                        {
                            dInputFrameRate = ((((DOUBLE)ulTMDSClk * 10000 * 8) / (DOUBLE)ulColorDepth / (DOUBLE)m_sProAV_Info.sRxPortDetTiming[ucRxPort].Hs.Total / (DOUBLE)m_sProAV_Info.sRxPortDetTiming[ucRxPort].Vs.Total) + 0.005f);
                        }
                        LOG_MSG(db_HAL_SOURCE_INFO, "Input Frame Rate = %2.2f\r\n", dInputFrameRate);
                        m_sProAV_Info.sRxPortAVI_Info[ucRxPort].dFrameRate = dInputFrameRate;
                        // set O/I Frame Rate Ratio
                        dvProAV_SrcFrameRateRatioSet(ucRxPort, dInputFrameRate, dvPro_PanelFrameRateGet(), m_sProAV_Info.sRxPortDetTiming[ucRxPort].Vs.Total, dvPro_PanelVTotalGet());

                        m_sProAV_Info.ucRxPortStatus[ucRxPort] = PortStatus;

#endif /* 0 */
                    }

                    if(m_sProAV_Info.ucRxPortStatusCount[ucRxPort] > 0)
                    {
                        m_sProAV_Info.ucRxPortStatusCount[ucRxPort]--;
                    }
                }
                else
                {

#if 0
                    if(m_sProAV_Info.ucRxPortStatusCount[ucRxPort] < 5)
                    {
                        m_sProAV_Info.ucRxPortStatusCount[ucRxPort]++;

                        if(m_sProAV_Info.ucRxPortStatusCount[ucRxPort] == 5)
                        {
                            m_sProAV_Info.ucRxPortStatus[ucRxPort] = PortStatus;
                            //m_sProAV_Info.ucRxPortStatusCount[ucRxPort] = 5;
                            memset((UINT8*)&m_sProAV_Info.sRxPortDetTiming[ucRxPort], 0, PROAV_DET_TIMING_SIZE);
                            memset((UINT8*)&m_sProAV_Info.sRxPortAVI_Info[ucRxPort], 0, PROAV_AVI_INFO_SIZE);
                        }
                    }
#endif /* 0 */
                    halScaler_RxPortClear(ucRxPort);
                }
            }
            else
            {
                LOG_MSG(db_HAL_HDMIRX, "(%s, %d) Fail\r\n", __FUNCTION__, __LINE__);
            }

            dvProAV_AccessWrite(uiRxPortChanged[ucRxPort], (uint32)1); // clear flag
        }


        halScaler_SemaphoreGive(0);

    }
    else
    {
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_RxPortIsReady(UINT8 ucCH, UINT8 ucSelect)
{
    UINT8 ucRx = halScaler_CurrentSourceRxPortGet(ucCH, ucSelect);

    if(ucRx > PROA_RX_4)
    {
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }

    if(m_sProAV_Info.ucRxPortStatus[ucRx] == eInpStatusStable)
    {
        return eHAL_SCALER_EXEC_CODE_PASS;
    }

    return eHAL_SCALER_EXEC_CODE_FAIL;
}

eHAL_SCALER_EXEC_CODE halScaler_RxPortAVI_InfoCopy(UINT8 ucRxPort, UINT8 *pcData)
{
    if(ucRxPort > PROA_RX_4)
    {
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }

    if(m_sProAV_Info.ucRxPortStatus[ucRxPort] == eInpStatusStable)
    {
        memcpy(pcData, (UINT8*)&m_sProAV_Info.ucAVIInfo[ucRxPort], AVI_INFO_SIZE);
        return eHAL_SCALER_EXEC_CODE_PASS;
    }
    return eHAL_SCALER_EXEC_CODE_FAIL;
}


eHAL_SCALER_EXEC_CODE halScaler_RxPortAVI_Info(UINT8 ucCH, UINT8 ucSelect, UINT8 *pcData)
{
    UINT8 ucRx = halScaler_CurrentSourceRxPortGet(ucCH, ucSelect);

    if(ucRx > PROA_RX_4)
    {
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }

    if(m_sProAV_Info.ucRxPortStatus[ucRx] == eInpStatusStable)
    {
        halScaler_RxPortAVI_InfoCopy(ucRx, pcData);
        return eHAL_SCALER_EXEC_CODE_PASS;
    }
    return eHAL_SCALER_EXEC_CODE_FAIL;
}

eHAL_SCALER_EXEC_CODE halScaler_RxPortVSI_Info(UINT8 ucCH, UINT8 ucSelect, UINT8 *pcData)
{
    UINT8 ucRx = halScaler_CurrentSourceRxPortGet(ucCH, ucSelect);

    if(ucRx > PROA_RX_4)
    {
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }

    if(m_sProAV_Info.ucRxPortStatus[ucRx] == eInpStatusStable)
    {
        memcpy(pcData, m_sProAV_Info.ucVSIInfo, 8);
        return eHAL_SCALER_EXEC_CODE_PASS;
    }
    return eHAL_SCALER_EXEC_CODE_FAIL;
}

// ==============================================================================
// FUNCTION NAME: halScaler_RxPortDetTiming
// DESCRIPTION:
//
//
// Params:
// UINT8 ucCH:
// UINT8 *psDetTiming:
//
// Returns:
//
//
// Modification History
// --------------------
// 2019/09/30, Larry Create
// --------------------
// ==============================================================================
void halScaler_RxPortDetTiming(UINT8 ucRxPort, UINT8 *psDetTiming)
{
    memcpy(psDetTiming, (UINT8*)&m_sProAV_Info.sRxPortDetTiming[ucRxPort], PROAV_DET_TIMING_SIZE);
}

// ==============================================================================
// FUNCTION NAME: halScaler_InputCompare
// DESCRIPTION:
//
//
// Params:
// UINT8 ucCH:
// UINT8 ucRxPort:
//
// Returns:
//
//
// Modification History
// --------------------
// 2019/09/30, Larry Create
// --------------------
// ==============================================================================
UINT8 halScaler_InputCompare(UINT8 ucCH, UINT8 ucSelect)
{
    SclIpuDim s_mSeting = {0};
    UINT8 ucRx = halScaler_CurrentSourceRxPortGet(ucCH, ucSelect);
    UINT8 cResult = 0;

    if(ucRx > PROA_RX_4)
    {
        return 0;
    }

    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        dvProAV_SclIpuDimGet((SclEntity)ucCH, &s_mSeting);

        LOG_MSG(db_HAL_SOURCE_INFO,"HActive    = %d %d\n", s_mSeting.HSmpSize, m_sProAV_Info.sRxPortDetTiming[ucRx].Hs.Size);
        LOG_MSG(db_HAL_SOURCE_INFO,"HStart     = %d %d\n", s_mSeting.HSmpSt, m_sProAV_Info.sRxPortDetTiming[ucRx].Hs.Start);
        LOG_MSG(db_HAL_SOURCE_INFO,"VActive    = %d %d\n", s_mSeting.VSmpSize, m_sProAV_Info.sRxPortDetTiming[ucRx].Vs.Size);
        LOG_MSG(db_HAL_SOURCE_INFO,"VStart     = %d %d\n", s_mSeting.VSmpSt, m_sProAV_Info.sRxPortDetTiming[ucRx].Vs.Start);
        LOG_MSG(db_HAL_SOURCE_INFO,"Select = %d, Rx = %d, 3DMode = %d\n", ucSelect, ucRx, m_sProAV_Info.sRxPortAVI_Info[ucRx].uc3DMode);

        if(((s_mSeting.HSmpSt == m_sProAV_Info.sRxPortDetTiming[ucRx].Hs.Start) &&
            (s_mSeting.HSmpSize == m_sProAV_Info.sRxPortDetTiming[ucRx].Hs.Size) &&
            (s_mSeting.VSmpSt == m_sProAV_Info.sRxPortDetTiming[ucRx].Vs.Start) &&
            (s_mSeting.VSmpSize == m_sProAV_Info.sRxPortDetTiming[ucRx].Vs.Size))||
            (dvProAV_SclDatapathGet() == eSclPath_SeamlessSw))
        {
            if(m_sProAV_Info.sRxPortAVI_Info[ucRx].uc3DMode == eScl3DMode_Off)
            {
                LOG_MSG(db_HAL_SOURCE_INFO,"Seamless switch\n");
                cResult = 1;
            }
        }

        halScaler_SemaphoreGive(0);

        return cResult;
	}
    else
    {
        return cResult;
    }
}

// ==============================================================================
// FUNCTION NAME: halScaler_SclDatapathSet
// DESCRIPTION:
//
//
// Params:
// UINT8 dataPath:
//
// Returns:
//
//
// Modification History
// --------------------
// 2019/11/15, Rex Create
// --------------------
// ==============================================================================
UINT8 halScaler_SclDatapathSet(UINT8 dataPath)
{
    UINT8 cResult = 0;
    UINT8 cMode = 0;

    if(dataPath == eSOURCE_MODE_SEAMLESS)
    {
        cMode = eSclPath_SeamlessSw;
    }
    else if(dataPath == eSOURCE_MODE_BACKUP)
    {
        cMode = eSclPath_BackupSw;
    }
    else if(dataPath == eSOURCE_MODE_PIPPOP)
    {
        cMode = eSclPath_Pop;
        m_sUserSystemSetting.sCommonSetting.ucBackupIputSwitch = 0;
    }
    else
    {
        cMode = eSclPath_Signle;
    }

    LOG_MSG(db_HAL_BACKUPSOURCE, "(%s, %d) dataPath = %d, Mode = %d\r\n", __FUNCTION__, __LINE__, dataPath, cMode);

    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        cResult = dvProAV_SclDatapathSet((SclDataPath)cMode);

        halScaler_SemaphoreGive(0);

        return cResult;
    }

    return cResult;
}

// ==============================================================================
// FUNCTION NAME: halScaler_SclDatapathCompare
// DESCRIPTION:
//
//
// Params:
// UINT8 dataPath:
//
// Returns:
//
//
// Modification History
// --------------------
// 2019/11/15, Rex Create
// --------------------
// ==============================================================================
UINT8 halScaler_SclDatapathCompare(UINT8 dataPath)
{
    UINT8 cResult = 0;
    UINT8 cMode = 0;

    if(dataPath == eSOURCE_MODE_SEAMLESS)
    {
        cMode = eSclPath_SeamlessSw;
    }
    else if(dataPath == eSOURCE_MODE_BACKUP)
    {
        cMode = eSclPath_BackupSw;
    }
    else
    {
        cMode = eSclPath_Signle;
    }

    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        if(cMode != dvProAV_SclDatapathGet())
        {
            cResult = 1;
        }

        halScaler_SemaphoreGive(0);

        return cResult;
    }

    return cResult;
}



// ==============================================================================
// FUNCTION NAME: halScaler_SclVopWindowOffSet
// DESCRIPTION:
//
//
// Params:
// UINT8 dataPath:
//
// Returns:
//
//
// Modification History
// --------------------
// 2019/11/27, Rex Create
// --------------------
// ==============================================================================
UINT8 halScaler_SclVopWindowOffSet(UINT8 ucCH, bool bOff)
{
    UINT8 cResult = 0;

    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        cResult = dvProAV_SclVopWindowOffSet((SclEntity)ucCH, bOff);

        halScaler_SemaphoreGive(0);

        return cResult;
    }
    else
    {
        return cResult;
    }


    return cResult;
}

// ==============================================================================
// FUNCTION NAME: halScaler_SclVopWindowOffGet
// DESCRIPTION:
//
//
// Params:
// UINT8 dataPath:
//
// Returns:
//
//
// Modification History
// --------------------
// 2019/11/28, Rex Create
// --------------------
// ==============================================================================
bool halScaler_SclVopWindowOffGet(UINT8 ucCH)   // ProAV_Rex_0046
{
    bool bOffStatus = 0;

    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        bOffStatus = dvProAV_SclVopWindowOffGet((SclEntity)ucCH);

        halScaler_SemaphoreGive(0);

        return bOffStatus;
    }
    else
    {
        return bOffStatus;
    }
}

// ==============================================================================
// FUNCTION NAME: halScaler_3DModeGet
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
// 2019/12/02, Larry Create
// --------------------
// ==============================================================================
UINT8 halScaler_3DModeGet(UINT8 ucRxPort)
{
    switch(m_sProAV_Info.sRxPortAVI_Info[ucRxPort].uc3DMode)
    {
        case eScl3DMode_FramePacking:
            return eVIDEO_3D_FORMAT_FP;

        case eScl3DMode_SideBySide:
            return eVIDEO_3D_FORMAT_SS;

        case eScl3DMode_TopAndBottom:
            return eVIDEO_3D_FORMAT_TB;

        default:
            return eVIDEO_3D_FORMAT_2D;
    }

    //return m_sProAV_Info.sRxPortAVI_Info[ucRxPort].uc3DMode;
}

// ==============================================================================
// FUNCTION NAME: halScaler_DualPipe3dModeSet
// DESCRIPTION:
//
//
// Params:
// UINT8 dataPath:
//
// Returns:
//
//
// Modification History
// --------------------
// 2019/12/12, Rex Create
// --------------------
// ==============================================================================
UINT8 halScaler_DualPipe3dModeSet(bool bMode)   // ProAV_Rex_0052
{
    UINT8 cResult = 0;

    if(Board_SingleBoard_Get() == FALSE)
    {
        cResult = halScaler_FrameSyncModeSet(eSclEntity_Main, bMode);    // Vx1
    }
    else
    {
        cResult = halScaler_FrameSyncModeSet(eSclEntity_Main, bMode);    // HDMI
    }

    //cResult = dvProAV_SclDatapathSet((bMode)? eSclPath_Pop : eSclPath_Signle);
    cResult = dvProAV_SclDatapathSet((bMode)? eSclPath_Pop : eSclPath_SeamlessSw);

    cResult = dvProAV_SclDualPipe3dModeSet(bMode);

    return cResult;
}

UINT8 halScaler_InputPortIsReady(UINT8 ucCH, UINT8 ucSelect)
{
    UINT8 ucRxPort = halScaler_CurrentSourceRxPortGet(ucCH, ucSelect);

    if(ucRxPort > PROA_RX_4)
    {
        return 0;
    }

    if(m_sProAV_Info.ucRxPortStatus[ucRxPort] == eInpStatusStable)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

void halScaler_InputPortResolution(UINT8 ucCH, UINT8 ucSelect, UINT8 *pcData)
{
    UINT8 ucRxPort = halScaler_CurrentSourceRxPortGet(ucCH, ucSelect);

    if(ucRxPort > PROA_RX_4)
    {
        sprintf(pcData, "-\0");
        return;
    }

    if(m_sProAV_Info.ucRxPortStatus[ucRxPort] == eInpStatusStable)
    {
        sprintf(pcData, "%d x %d\0", m_sProAV_Info.sRxPortDetTiming[ucRxPort].Hs.Size, m_sProAV_Info.sRxPortDetTiming[ucRxPort].Vs.Size);
    }
    else
    {
        sprintf(pcData, "-\0");
    }
}

void halScaler_InputPortHorzRate(UINT8 ucCH, UINT8 ucSelect, UINT8 *pcData)
{
    UINT8 ucRxPort = halScaler_CurrentSourceRxPortGet(ucCH, ucSelect);

    if(ucRxPort > PROA_RX_4)
    {
        sprintf(pcData, "-\0");
        return;
    }

    if(m_sProAV_Info.ucRxPortStatus[ucRxPort] == eInpStatusStable)
    {
        DOUBLE dbVal = m_sProAV_Info.sRxPortAVI_Info[ucRxPort].dFrameRate * m_sProAV_Info.sRxPortDetTiming[ucRxPort].Vs.Total;
        sprintf(pcData, "%d.%03dkHz\0", (UINT32)dbVal/1000,(UINT32)dbVal%1000);
    }
    else
    {
        sprintf(pcData, "-\0");
    }
}

UINT8 halScaler_InputPortVertRateGet(UINT8 ucCH)
{
    UINT8 ucIptFrameRate = 0;
    DOUBLE dIptFrameRate = 0.0;

    dIptFrameRate = m_sProAV_Info.sRxPortAVI_Info[(RxPort)halScaler_CurrentSourceRxPortGet(eMCT_CH1, halScaler_InputSource_Get(eMCT_CH1))].dFrameRate;

    if(dIptFrameRate < 5.0)
    {
        return ucIptFrameRate;
    }

    ucIptFrameRate = dvPro_InputRataIntegerGet(dIptFrameRate);

    return ucIptFrameRate;
}

void halScaler_RxPortColorSpace(UINT8 ucRxPort, UINT8 *pcData)
{
    if(ucRxPort > PROA_RX_4)
    {
        sprintf(pcData, "-\0");
        return;
    }

    if(m_sProAV_Info.ucRxPortStatus[ucRxPort] == eInpStatusStable)
    {
        switch(m_sProAV_Info.sRxPortAVI_Info[ucRxPort].ucColorsSpace)
        {
            case RX_COLORSPACE_RGB:
                if(m_sProAV_Info.sRxPortAVI_Info[ucRxPort].ucRGBFull)
                {
                    sprintf(pcData, "RGB Full\0");
                }
                else
                {
                    sprintf(pcData, "RGB Limit\0");
                }
                break;

            case RX_COLORSPACE_REC601:
            case RX_COLORSPACE_REC709:
            case RX_COLORSPACE_REC2020:
                sprintf(pcData, "YUV\0");
                break;

            default:
                sprintf(pcData, "Unknow\0");
                break;
        }
    }
    else
    {
        sprintf(pcData, "-\0");
    }
}

void halScaler_InputPortColorSpace(UINT8 ucCH, UINT8 ucSelect, UINT8 *pcData)
{
    UINT8 ucRxPort = halScaler_CurrentSourceRxPortGet(ucCH, ucSelect);

    if(ucRxPort > PROA_RX_4)
    {
        sprintf(pcData, "-\0");
        return;
    }

    //#define PROAV_SCALER_COLOR_PIXEL_MODE_RGB444    0x00
    //#define PROAV_SCALER_COLOR_PIXEL_MODE_YUV422    0x02
    //#define PROAV_SCALER_COLOR_PIXEL_MODE_YUV444    0x01
    //#define PROAV_SCALER_COLOR_PIXEL_MODE_YUV420    0x03

    if(m_sProAV_Info.ucRxPortStatus[ucRxPort] == eInpStatusStable)
    {
        switch(m_sProAV_Info.sRxPortAVI_Info[ucRxPort].ucColorsSpace)
        {
            case RX_COLORSPACE_RGB:
                if(m_sProAV_Info.sRxPortAVI_Info[ucRxPort].ucRGBFull)
                {
                    sprintf(pcData, "RGB Full\0");
                }
                else
                {
                    sprintf(pcData, "RGB Limit\0");
                }
                break;

            case RX_COLORSPACE_REC601:
            case RX_COLORSPACE_REC709:
            case RX_COLORSPACE_REC2020:
                if(m_sProAV_Info.sRxPortAVI_Info[ucRxPort].ucPixelMode == PROAV_SCALER_COLOR_PIXEL_MODE_YUV444)
                {
                    sprintf(pcData, "YUV444\0");
                }
                else if(m_sProAV_Info.sRxPortAVI_Info[ucRxPort].ucPixelMode == PROAV_SCALER_COLOR_PIXEL_MODE_YUV422)
                {
                    sprintf(pcData, "YUV422\0");
                }
                else if(m_sProAV_Info.sRxPortAVI_Info[ucRxPort].ucPixelMode == PROAV_SCALER_COLOR_PIXEL_MODE_YUV420)
                {
                    sprintf(pcData, "YUV420\0");
                }
                else
                {
                    sprintf(pcData, "Unknow\0");
                }
                break;

            default:
                sprintf(pcData, "Unknow\0");
                break;
        }
    }
    else
    {
        sprintf(pcData, "-\0");
    }
}

// ==============================================================================
// FUNCTION NAME: halScaler_LogoCaptureSet
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
// 2019/12/20, Rex Create
// --------------------
// ==============================================================================
UINT8 halScaler_LogoCaptureSet(void) // ProAV_Rex_0054
{
    UINT8 cResult = 0;

    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        BOOL bLogo = 0;

        //Disable logo
        bLogo = dvProAV_OSD_EnableGet();
        if(bLogo)
        {
            dvProAV_OSD_Enable(FALSE);
        }

        dvProAV_SclLogoCaptureEn();
        while(dvProAV_SclLogoCaptureBusyStatusGet())    // check busy
        {
            MS_SLEEP(1);
        }

        if(bLogo)
        {
            dvProAV_OSD_Enable(TRUE);
        }

        halScaler_SemaphoreGive(0);

        return cResult;
    }

    return cResult;
}

// ==============================================================================
// FUNCTION NAME: halScaler_LogoDisplaySet
// DESCRIPTION:
//
//
// Params:
// bool bEnable:
//
// Returns:
//
//
// Modification History
// --------------------
// 2019/12/20, Rex Create
// --------------------
// ==============================================================================
UINT8 halScaler_LogoDisplayEn(bool bEnable) // ProAV_Rex_0054
{
    UINT8 cResult = 0;

    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        cResult = dvProAV_SclLogoDisplayEn(bEnable);

        halScaler_SemaphoreGive(0);

        return cResult;
    }

    return cResult;
}

// ==============================================================================
// FUNCTION NAME: halScaler_ScreenOff
// DESCRIPTION:
//
//
// Params:
// bool bEnable:
//
// Returns:
//
//
// Modification History
// --------------------
// 2020/03/16, Larry Create
// --------------------
// ==============================================================================
UINT8 halScaler_ScreenOff(UINT8 ucCH, bool bEnable)
{
    UINT8 cResult = 0;

    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        //dvProAV_SclOpuScreenOffSet(0, bEnable);
        //dvProAV_SclOpuScreenOffSet(1, bEnable);
        cResult = dvProAV_SclOpuScreenOffSet((SclEntity)ucCH, bEnable);

        halScaler_SemaphoreGive(0);

        return cResult;
    }
    else
    {
        return cResult;
    }

    return cResult;
}

// ==============================================================================
// FUNCTION NAME: halScaler_Dram2Flash
// DESCRIPTION:
//
//
// Params:
// uint32 dramAddr:
// uint32 flashAddr:
// uint32 len:
//
// Returns:
//
//
// Modification History
// --------------------
// 2019/12/20, Rex Create
// --------------------
// ==============================================================================
UINT8 halScaler_Dram2Flash(UINT32 dramAddr, UINT32 flashAddr, UINT32 len)
{
    UINT8 cResult = rcSUCCESS;
    uint32 start_sector_addr = flashAddr / FLASH_SECTOR_SIZE * FLASH_SECTOR_SIZE;
    uint32 stop_sector_addr = (flashAddr + len + FLASH_SECTOR_SIZE) / FLASH_SECTOR_SIZE * FLASH_SECTOR_SIZE;

    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        dramAddr = dramAddr*PROAV_DRAM_LINE_BYTES;

        for(uint32 offset=0; offset < (stop_sector_addr - start_sector_addr); offset+=FLASH_SECTOR_SIZE)
        {
            cResult &= dvProAV_SfiEraseSector(start_sector_addr + offset, FLASH_SECTOR_SIZE);
            cResult &= dvProAV_SfiDram2Flash(dramAddr + offset, flashAddr + offset, FLASH_SECTOR_SIZE, IO_WRITE);
        }

        halScaler_SemaphoreGive(0);

        return cResult;
    }

    return rcERROR;
}

// ==============================================================================
// FUNCTION NAME: halScaler_Flash2Dram
// DESCRIPTION:
//
//
// Params:
// UINT32 dramAddr:
// UINT32 flashAddr:
// UINT32 len:
//
// Returns:
//
//
// Modification History
// --------------------
// 2019/12/20, Rex Create
// --------------------
// ==============================================================================
UINT8 halScaler_Flash2Dram(UINT32 dramAddr, UINT32 flashAddr, UINT32 len)
{
    UINT8 cResult = rcSUCCESS;
    uint32 start_sector_addr = flashAddr / FLASH_SECTOR_SIZE * FLASH_SECTOR_SIZE;
    uint32 stop_sector_addr = (flashAddr + len + FLASH_SECTOR_SIZE) / FLASH_SECTOR_SIZE * FLASH_SECTOR_SIZE;

#if 0
    for(uint32 offset=0; offset < (stop_sector_addr - start_sector_addr); offset+=FLASH_SECTOR_SIZE)
    {
        printf("halScaler_Flash2Dram offset %d\n", offset);
    }
#endif /* 0 */

    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        for(uint32 offset=0; offset < (stop_sector_addr - start_sector_addr); offset+=FLASH_SECTOR_SIZE)
        {
            cResult = dvProAV_SfiDram2Flash(dramAddr + offset, flashAddr + offset, FLASH_SECTOR_SIZE, IO_READ);
        }

        halScaler_SemaphoreGive(0);

        return cResult;
    }

    return rcERROR;
}

// ==============================================================================
// FUNCTION NAME: halScaler_BackupInputCheck
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
// 2020/02/05, Larry Create
// --------------------
// ==============================================================================
UINT8 halScaler_BackupInputCheck(UINT8 ucMain, UINT8 ucSub)
{
    UINT8 ucPrimary = halScaler_CurrentSourceRxPortGet(eMCT_CH1, ucMain);
    UINT8 ucSecondary = halScaler_CurrentSourceRxPortGet(eMCT_CH2, ucSub);

    if(ucMain == ucSub)
    {
    	return eHAL_SCALER_EXEC_CODE_FAIL;
    }

    if(//(m_sProAV_Info.sRxPortDetTiming[ucPrimary].Hs.Total == m_sProAV_Info.sRxPortDetTiming[ucSecondary].Hs.Total) &&
       (m_sProAV_Info.sRxPortDetTiming[ucPrimary].Hs.Size == m_sProAV_Info.sRxPortDetTiming[ucSecondary].Hs.Size) &&
       //(m_sProAV_Info.sRxPortDetTiming[ucPrimary].Hs.Start == m_sProAV_Info.sRxPortDetTiming[ucSecondary].Hs.Start) &&
       //(m_sProAV_Info.sRxPortDetTiming[ucPrimary].Vs.Total == m_sProAV_Info.sRxPortDetTiming[ucSecondary].Vs.Total) &&
       (m_sProAV_Info.sRxPortDetTiming[ucPrimary].Vs.Size == m_sProAV_Info.sRxPortDetTiming[ucSecondary].Vs.Size)  &&
       //(m_sProAV_Info.sRxPortDetTiming[ucPrimary].Vs.Start == m_sProAV_Info.sRxPortDetTiming[ucSecondary].Vs.Start) &&
       //(m_sProAV_Info.sRxPortAVI_Info[ucPrimary].ucColorsSpace == m_sProAV_Info.sRxPortAVI_Info[ucSecondary].ucColorsSpace) && // moved to below
       (m_sProAV_Info.sRxPortDetTiming[ucPrimary].Interlace == 0) &&// R70K_Bruce_0025
       (m_sProAV_Info.sRxPortDetTiming[ucSecondary].Interlace == 0) &&// R70K_Bruce_0025
        (halScaler_HDR_Get(eMCT_CH1) == halScaler_HDR_Get(eMCT_CH2)))
    {
        if(ABS(m_sProAV_Info.sRxPortAVI_Info[ucPrimary].dFrameRate - m_sProAV_Info.sRxPortAVI_Info[ucSecondary].dFrameRate) > (m_sProAV_Info.sRxPortAVI_Info[ucPrimary].dFrameRate) * 1 / 100)
        {
            return eHAL_SCALER_EXEC_CODE_FAIL;
        }

        if ((m_sProAV_Info.sRxPortAVI_Info[ucPrimary].ucColorsSpace == RX_COLORSPACE_UNKNOW) ||
            (m_sProAV_Info.sRxPortAVI_Info[ucSecondary].ucColorsSpace == RX_COLORSPACE_UNKNOW))
        {
            return eHAL_SCALER_EXEC_CODE_FAIL;
        }

        if((m_sProAV_Info.sRxPortAVI_Info[ucPrimary].ucColorsSpace == RX_COLORSPACE_RGB) ||
           (m_sProAV_Info.sRxPortAVI_Info[ucSecondary].ucColorsSpace == RX_COLORSPACE_RGB))
        {
            if((m_sProAV_Info.sRxPortAVI_Info[ucPrimary].ucColorsSpace == m_sProAV_Info.sRxPortAVI_Info[ucSecondary].ucColorsSpace) &&
               (m_sProAV_Info.sRxPortAVI_Info[ucPrimary].ucRGBFull == m_sProAV_Info.sRxPortAVI_Info[ucSecondary].ucRGBFull))
            {
               return eHAL_SCALER_EXEC_CODE_PASS;
            }
            return eHAL_SCALER_EXEC_CODE_FAIL;
        }
        else if((m_sProAV_Info.sRxPortAVI_Info[ucPrimary].ucPixelMode == PROAV_SCALER_COLOR_PIXEL_MODE_YUV420) ||
                (m_sProAV_Info.sRxPortAVI_Info[ucSecondary].ucPixelMode == PROAV_SCALER_COLOR_PIXEL_MODE_YUV420))
        {
            if(m_sProAV_Info.sRxPortAVI_Info[ucPrimary].ucPixelMode == m_sProAV_Info.sRxPortAVI_Info[ucSecondary].ucPixelMode)
            {
               return eHAL_SCALER_EXEC_CODE_PASS;
            }

            return eHAL_SCALER_EXEC_CODE_FAIL;
        }
        return eHAL_SCALER_EXEC_CODE_PASS;
    }

    return eHAL_SCALER_EXEC_CODE_FAIL;
}

void halScaler_RxPortInfoPrint(void)
{
    for(UINT8 ucCount = 0; ucCount < 4; ucCount++ )
    {
        LOG_MSG(db_ALWAYS, "\n====================\n");
        LOG_MSG(db_ALWAYS, "RxPort =        [%d]\n", ucCount);
        LOG_MSG(db_ALWAYS, "Hs.Total =      [%d]\n", m_sProAV_Info.sRxPortDetTiming[ucCount].Hs.Total);
        LOG_MSG(db_ALWAYS, "Hs.Size =       [%d]\n", m_sProAV_Info.sRxPortDetTiming[ucCount].Hs.Size);
        LOG_MSG(db_ALWAYS, "Hs.Start =      [%d]\n", m_sProAV_Info.sRxPortDetTiming[ucCount].Hs.Start);
        LOG_MSG(db_ALWAYS, "Vs.Total =      [%d]\n", m_sProAV_Info.sRxPortDetTiming[ucCount].Vs.Total);
        LOG_MSG(db_ALWAYS, "Vs.Size =       [%d]\n", m_sProAV_Info.sRxPortDetTiming[ucCount].Vs.Size);
        LOG_MSG(db_ALWAYS, "Vs.Start =      [%d]\n", m_sProAV_Info.sRxPortDetTiming[ucCount].Vs.Start);
        LOG_MSG(db_ALWAYS, "Interlace =     [%d]\n", m_sProAV_Info.sRxPortDetTiming[ucCount].Interlace);
        LOG_MSG(db_ALWAYS, "ColorsSpace =   [%d]\n", m_sProAV_Info.sRxPortAVI_Info[ucCount].ucColorsSpace);
        LOG_MSG(db_ALWAYS, "ucRGBFull =     [%d]\n", m_sProAV_Info.sRxPortAVI_Info[ucCount].ucRGBFull);
        LOG_MSG(db_ALWAYS, "\n====================\n");
    }
}

// ==============================================================================
// FUNCTION NAME: halScaler_SclVopWindowOffSet
// DESCRIPTION:
//
//
// Params:
// UINT8 dataPath:
//
// Returns:
//
//
// Modification History
// --------------------
// 2019/11/27, Rex Create
// --------------------
// ==============================================================================
UINT16 halScaler_FpgaTempGet(void)
{
    UINT16 uiTemp = 0;
    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        uiTemp = dvProAV_SclFpgaTempGet();

        halScaler_SemaphoreGive(0);
    }

    return uiTemp;
}

// ==============================================================================
// FUNCTION NAME: halScaler_FpgaTempADCRst
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
// 2023/01/19, Larry Create
// --------------------
// ==============================================================================
void halScaler_FpgaTempADCRst(void)
{
    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        dvProAV_SclFpgaTempADCRst();

        halScaler_SemaphoreGive(0);
    }
}

// ==============================================================================
// FUNCTION NAME: halScaler_BackupInputSet
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
// 2020/02/06, Larry Create
// --------------------
// ==============================================================================
void halScaler_BackupPrimaryInput_Set(UINT8 ucInput)
{
    m_ucBackupPrimaryInput = ucInput;
}

UINT8 halScaler_BackupPrimaryInput_Get(void)
{
    return m_ucBackupPrimaryInput;
}

// ==============================================================================
// FUNCTION NAME: halScaler_BackupInputSet
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
// 2020/02/06, Larry Create
// --------------------
// ==============================================================================
void halScaler_BackupSecondaryInput_Set(UINT8 ucInput)
{
    m_ucBackupSecondaryInput = ucInput;
}

UINT8 halScaler_BackupSecondaryInput_Get(void)
{
    return m_ucBackupSecondaryInput;
}

// ==============================================================================
// FUNCTION NAME: halScaler_CurrentBackupSourceGet
// DESCRIPTION:
//
//
// Params:
// void:
//
// Returns:
//
//
// Modification History
// --------------------
// 2020/07/06, Larry Create
// --------------------
// ==============================================================================
UINT8 halScaler_CurrentBackupSourceGet(void)
{
    UINT8 cResult = rcSUCCESS;
    UINT8 cCurrentSource = 0;

    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        cResult = (UINT8)dvProAV_SclBackupSourceGet(&cCurrentSource);

        halScaler_SemaphoreGive(0);
    }

    return cCurrentSource;
}


// ==============================================================================
// FUNCTION NAME: halScaler_LogoCaptureTest
// DESCRIPTION:
//
//
// Params:
// void:
//
// Returns:
//
//
// Modification History
// --------------------
// 2020/03/13, Larry Create
// --------------------
// ==============================================================================
UINT8 halScaler_LogoCapturePanel(UINT16 uiPanel)
{
    UINT8 cResult = rcSUCCESS;
    uint32 len = ((3840 * 2160 * 2) / 64);
    uint32 ulCpyAddr = 0;
    uint32 ulPasAdd = 0;
    PanelTiming sPanelInfo = {0};

    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        dvProAV_DramBankSet(0);

        LOG_MSG(db_ALWAYS, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);

        cResult &= dvProAV_SclLogoDisplayEn(false);

        halScaler_SemaphoreGive(0);
    }

    if(ePROAV_EXEC_CODE_PASS == dvPro_GetPanelInfo(uiPanel, (UINT8*)&sPanelInfo))
    {
        if(uiPanel == PANEL_2D_OUTPUT)
        {
            len = 2*sPanelInfo.HSize*sPanelInfo.VSize;

            for(uint16 count = 0; count < sPanelInfo.VSize; count++)
            {
                ulCpyAddr = (DRAM_LOGO_CAPTURE_ADDR * PROAV_DRAM_LINE_BYTES + 2*sPanelInfo.HSize*count) / PROAV_DRAM_LINE_BYTES;
                ulPasAdd  = (DRAM_LOGO_DISPLAY0_2D_ADDR * PROAV_DRAM_LINE_BYTES + 2*sPanelInfo.HSize*count) / PROAV_DRAM_LINE_BYTES;

                cResult = dvProAV_UmbDramToDram(eDramBank0, (sPanelInfo.HSize*2), ulCpyAddr, ulPasAdd);
            }

            cResult &= halScaler_Dram2Flash(DRAM_LOGO_DISPLAY0_2D_ADDR, FLASH_LOGO0_2D_ADDR, len);
        }
        else if(uiPanel == PANEL_3D_OUTPUT)
        {
            PanelTiming sPanel2D = {0};
            dvPro_GetPanelInfo(PANEL_2D_OUTPUT, (UINT8*)&sPanel2D);

            for(uint16 count = 0; count < sPanelInfo.VSize; count++)
            {
                ulCpyAddr = (DRAM_LOGO_CAPTURE_ADDR * PROAV_DRAM_LINE_BYTES + 2*sPanel2D.HSize*count) / PROAV_DRAM_LINE_BYTES;
                ulPasAdd  = (DRAM_LOGO_DISPLAY0_3D_ADDR * PROAV_DRAM_LINE_BYTES + 2*sPanelInfo.HSize*count) / PROAV_DRAM_LINE_BYTES;

                cResult = dvProAV_UmbDramToDram(eDramBank0, (sPanelInfo.HSize*2), ulCpyAddr, ulPasAdd);
            }

            len = 2*sPanelInfo.HSize*sPanelInfo.VSize;

            cResult &= halScaler_Dram2Flash(DRAM_LOGO_DISPLAY0_3D_ADDR, FLASH_LOGO0_3D_ADDR, len);
        }
    }

    return cResult;
}

// ==============================================================================
// FUNCTION NAME: halScaler_LowLatencyEnable
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
// 2020/05/13, Larry Create
// --------------------
// ==============================================================================
BOOL halScaler_LowLatencyEnable(void)
{
    UINT8 ucIptFrameRate = 0, ucOptFrameRate = 0;
    DOUBLE dIptFrameRate = 0.0;
    UINT8 uc3DMode = 0;
    UINT8 ucRate = 0;
    BOOL  cEnale = FALSE;

    //dIptFrameRate = dvPro_InputFrameRate_Get(eMCT_CH1);
    dIptFrameRate = m_sProAV_Info.sRxPortAVI_Info[(RxPort)halScaler_CurrentSourceRxPortGet(eMCT_CH1, halScaler_InputSource_Get(eMCT_CH1))].dFrameRate;
    ucOptFrameRate = (UINT8)dvPro_PanelFrameRateGet();
    uc3DMode = dvPro_Input_3D_Format_Config_Get();

    if((dIptFrameRate < 5.0) || (ucOptFrameRate == 0))
    {
        return FALSE;
    }

    ucIptFrameRate = dvPro_InputRataIntegerGet(dIptFrameRate);
    ucRate = ucOptFrameRate/ucIptFrameRate;

    if((ucOptFrameRate >= ucIptFrameRate) && (ucOptFrameRate % ucIptFrameRate == 0))
    {
        cEnale = TRUE;
    }

    //3D mode
    if((uc3DMode != eINPUT_3D_TYPE_OFF) && (cEnale == TRUE))
    {
        //if(ucRate != 1 && (ucRate % 2))
        //{
            //120 out, 24 in
            //cEnale = FALSE;
        //}

        if(ucOptFrameRate <= 60)
        {
            if(ucIptFrameRate > 60)
            {
                cEnale = FALSE;
            }
        }
        else if(ucOptFrameRate <= 120)
        {
            if(ucIptFrameRate > 120)
            {
                cEnale = FALSE;
            }
        }
        else if(ucOptFrameRate <= 240)
        {
            if(ucIptFrameRate > 240)
            {
                cEnale = FALSE;
            }
        }
    }

    return cEnale;
}

UINT8 halScaler_LensLockAllMotors_Get(void)
{
    return m_sUserSystemSetting.sCommonSetting.ucLensAdjust;
}

eHAL_SCALER_EXEC_CODE halScaler_AntiSmearSet(UINT8 cMode, bool bEnable)
{
    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        if(bEnable)
        {
            dvProAV_SclAbBufModeSet(0, true);
            dvProAV_SclAtsBlankPolSet(1);

            dvProAV_SclAtsBlankDelaySet(0, 0);

            dvProAV_SclAtsBlankEn(true);
            dvProAV_SclAtsBlankmode(cMode);

            LOG_MSG(db_HAL_RESERVED16, "cMode %d\n", cMode);
        }
        else
        {
            dvProAV_SclAtsBlankEn(false);
        }

        halScaler_SemaphoreGive(0);
    }
    else
    {
        LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore\r\n", __FUNCTION__, __LINE__);
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }

    return eHAL_SCALER_EXEC_CODE_PASS;
}

eHAL_SCALER_EXEC_CODE halScaler_MEMCBypassSet(UINT8 ucEnable)
{
    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        if(m_sUserSystemSetting.sImageSetting.ucMEMCBypass != ucEnable)
        {
            m_sUserSystemSetting.sImageSetting.ucMEMCBypass = ucEnable;
            dvProAV_SclMemcBypassSet(ucEnable);

            //printf("MemcBypassSet %d\n\n" , ucEnable);
            //halMemc_Bypass_Set(ucEnable);
        }
        halScaler_SemaphoreGive(0);
    }

    return eHAL_SCALER_EXEC_CODE_PASS;
}

eHAL_SCALER_EXEC_CODE halScaler_MEMCColorSet(UINT8 ucColor)
{
    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        if(m_sUserSystemSetting.sImageSetting.ucMEMC_Color != ucColor)
        {
            m_sUserSystemSetting.sImageSetting.ucMEMC_Color = ucColor;

            //halMemc_Color_Set(ucColor);
        }
        halScaler_SemaphoreGive(0);
    }

    return eHAL_SCALER_EXEC_CODE_PASS;
}

eHAL_SCALER_EXEC_CODE halScaler_MEMCMotionSet(UINT8 ucMotion)
{
    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        if(m_sUserSystemSetting.sImageSetting.ucMEMC_Motion != ucMotion)
        {
            m_sUserSystemSetting.sImageSetting.ucMEMC_Motion = ucMotion;

            //halMemc_Motion_Set(ucMotion);
        }

        halScaler_SemaphoreGive(0);
    }

    return eHAL_SCALER_EXEC_CODE_PASS;
}

eHAL_SCALER_EXEC_CODE halScaler_MEMCDemoSet(UINT8 ucDemo)
{
    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        if(m_sUserSystemSetting.sImageSetting.ucMEMC_Motion_Demo != ucDemo)
        {
            m_sUserSystemSetting.sImageSetting.ucMEMC_Motion_Demo = ucDemo;

            //halMemc_Demo_Set(ucDemo);
        }
        halScaler_SemaphoreGive(0);
    }

    return eHAL_SCALER_EXEC_CODE_PASS;
}

void halScaler_BackupSwitchSet(UINT8 ucValue)
{
	halScaler_SclDatapathSet(eSOURCE_MODE_SIGNLE);
	m_sUserSystemSetting.sCommonSetting.ucBackupIputSwitch = ucValue;
}

UINT8 halScaler_BackupSwitchGet(void)
{
    return m_sUserSystemSetting.sCommonSetting.ucBackupIputSwitch;
}

eHAL_SCALER_EXEC_CODE halScaler_BackupSizePresetSetting(void)
{
    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        SclIpuDim ipuDim = {0};
        SclOpuDim opuDim = {0};
        DetTmg sRxPorTiming = {0};
        RxPort ucRxPort = eRxPort_0;

        dvProAV_SclVipSourceGet(eSclEntity_Main, &ucRxPort);

        dvProAV_SclOpuDimGet(eSclEntity_Main, &opuDim);
        dvProAV_SclIpuDimGet(eSclEntity_Main, &ipuDim);
        halScaler_RxPortDetTiming(ucRxPort, (UINT8*)&sRxPorTiming);

        sRxPorTiming.Hs.Size = ipuDim.HSmpSize;
        sRxPorTiming.Hs.Start = ipuDim.HSmpSt;
        sRxPorTiming.Vs.Size = ipuDim.VSmpSize;
        sRxPorTiming.Vs.Start = ipuDim.VSmpSt;
        dvProAV_SclUpdatingSet(eSclEntity_Sub, true);

        dvProAV_SclOpuDimSet(eSclEntity_Sub, opuDim);
        dvProAV_SclIpuDimSet(eSclEntity_Sub, ipuDim);

        dvProAV_SrcUpdatSet((ucRxPort == eRxPort_0) ? eRxPort_2 : eRxPort_0, true);
        dvProAV_SrcTimingSet((ucRxPort == eRxPort_0) ? eRxPort_2 : eRxPort_0, &sRxPorTiming);
        dvProAV_SrcUpdatSet((ucRxPort == eRxPort_0) ? eRxPort_2 : eRxPort_0, false);

        dvProAV_SclUpdatingSet(eSclEntity_Sub, false);

        dvPro_Set_Sharpness(eMCT_CH2);

        halScaler_SemaphoreGive(0);
    }

    return eHAL_SCALER_EXEC_CODE_PASS;
}

BOOL halScaler_AutoExt_3D_SyncSetting(void)
{
    BOOL eDet = FALSE;
    BOOL cInvert = (BOOL)dvPro_OSD_3D_InvertGet(eMCT_CH1);

    if(dvPro_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_FRAME_SEQUENTIAL || dvPro_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_4K3D_SEQUENTIAL)
    {
        if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
        {
            if(dvProAV_Scl3dNoExtSyncStatus())
            {
                eDet = FALSE;
            }
            else
            {
                eDet = TRUE;
            }

            halScaler_SemaphoreGive(0);
        }

        if(dvPro_OSD_3D_Sync_InGet(eMCT_CH1) == eCM_3D_SYNC_TYPE_AUTO)
        {
            if(!eDet) //no 3d sync in
            {
                if(m_sHalScalerInfo[eMCT_CH1].ucExt3DSyncCount)
                {
                    m_sHalScalerInfo[eMCT_CH1].ucExt3DSyncCount--;

                    if((m_sHalScalerInfo[eMCT_CH1].ucExt3DSyncCount == 0) &&
                       (m_sHalScalerInfo[eMCT_CH1].ucExt3DSyncSetting != e3dSyncInSwMode_InternalSync))
                    {
                        m_sHalScalerInfo[eMCT_CH1].ucExt3DSyncSetting = e3dSyncInSwMode_InternalSync;

                        if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
                        {
                            if(dvPro_OSD_3D_Sync_OutGet(eMCT_CH1) == eCM_3D_SYNC_OUT_TO_EMITTER)
                            {
                                dvProAV_SclIntSyncPolSet(cInvert);
                                dvProAV_SclExtSyncPolSet(cInvert);
                                dvProAV_Scl3dSyncOutSwModeSet(e3dSyncOutSwMode_3dSyncInDelay);
                                LOG_MSG(db_DV_3D_CONFIG, "6 OutSw 3dSyncInDelay\n");
                            }
                            else //next projector
                            {
                                dvProAV_SclIntSyncPolSet(false);
                                dvProAV_SclExtSyncPolSet(true);
                                dvProAV_Scl3dSyncOutSwModeSet(e3dSyncOutSwMode_InternalSync);
                                LOG_MSG(db_DV_3D_CONFIG, "6 OutSw InternalSync)\n");
                            }

                            dvProAV_Scl3dSyncInSwModeSet(e3dSyncInSwMode_InternalSync);

                            if(dvPro_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_4K3D_SEQUENTIAL)
                            {
                                dvProAV_Scl3dIntVsModeSel(1);
                            }
                            LOG_MSG(db_DV_3D_CONFIG, "6 InSw InternalSync\n");

                            halScaler_SemaphoreGive(0);
                        }
                    }
                }
            }
            else
            {
                if(m_sHalScalerInfo[eMCT_CH1].ucExt3DSyncCount < 10)
                {
                    m_sHalScalerInfo[eMCT_CH1].ucExt3DSyncCount++;

                    if((m_sHalScalerInfo[eMCT_CH1].ucExt3DSyncCount == 10) &&
                       (m_sHalScalerInfo[eMCT_CH1].ucExt3DSyncSetting != e3dSyncInSwMode_ExternalSync))
                    {
                        m_sHalScalerInfo[eMCT_CH1].ucExt3DSyncSetting = e3dSyncInSwMode_ExternalSync;
                        if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
                        {
                            if(dvPro_OSD_3D_Sync_OutGet(eMCT_CH1) == eCM_3D_SYNC_OUT_TO_EMITTER)
                            {
                                dvProAV_SclIntSyncPolSet(cInvert);
                                dvProAV_SclExtSyncPolSet(cInvert);
                                dvProAV_Scl3dSyncOutSwModeSet(e3dSyncOutSwMode_3dSyncInDelay);

                                //if(dvPro_InputRataIntegerGet(dvPro_InputFrameRate_Get(eMCT_CH1)) == 60)
                                if(dvPro_InputRataIntegerGet(m_sProAV_Info.sRxPortAVI_Info[(RxPort)halScaler_CurrentSourceRxPortGet(eMCT_CH1, halScaler_InputSource_Get(eMCT_CH1))].dFrameRate) == 60)
                                {
                                    dvProAV_SclFrameSeq3dsyncDubSet(true);
                                }

                                LOG_MSG(db_DV_3D_CONFIG, "6 OutSw 3dSyncInDelay\n");

                            }
                            else //next projector
                            {
                                dvProAV_SclIntSyncPolSet(false);
                                dvProAV_SclExtSyncPolSet(true);
                                dvProAV_Scl3dSyncOutSwModeSet(e3dSyncOutSwMode_ExternalSync);
                                dvProAV_SclFrameSeq3dsyncDubSet(false);
                                LOG_MSG(db_DV_3D_CONFIG, "6 OutSw ExternalSync\n");
                            }

                            if(dvPro_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_4K3D_SEQUENTIAL)
                            {
                                dvProAV_Scl3dIntVsModeSel(0);
                            }

                            dvProAV_Scl3dSyncInSwModeSet(e3dSyncInSwMode_ExternalSync);
                            LOG_MSG(db_DV_3D_CONFIG, "6 InSw ExternalSync\n");

                            halScaler_SemaphoreGive(0);
                        }
                    }
                }
            }
        }
    }
    else
    {
        eDet = FALSE;
    }

    return eDet;
}

void halScaler_ConfigurePanelSize(uint16 uiPanelID)
{
    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        PanelTiming sPanelInfo = {0};

        if(ePROAV_EXEC_CODE_PASS == dvPro_GetPanelInfo(uiPanelID, (UINT8*)&sPanelInfo))
        {
            dvProAV_AccessWrite(eOpmHDspSize, (uint32)sPanelInfo.HSize);
            dvProAV_AccessWrite(eOpmVDspSize, (uint32)sPanelInfo.VSize);
        }

        halScaler_SemaphoreGive(0);
    }
}

BOOL halScaler_Rx_Compare(UINT8 ucCH, UINT8 ucSelect)	//A70LK_Doulas_0001
{
	RxPort ucRxPort = eRxPort_0;
	RxPort ucRx_Old = eRxPort_0;
	int status = rcSUCCESS;

	if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[ucCH].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(0);
            return FALSE;
        }
		status = dvProAV_SclVipSourceGet(ucCH, &ucRx_Old);
		ucRxPort = (RxPort)halScaler_CurrentSourceRxPortGet(ucCH, ucSelect);
		//LOG_MSG(db_ALWAYS, "RX (%d,%d)\r\n",ucRx_Old,ucRxPort);

		halScaler_SemaphoreGive(0);
		if(status == rcSUCCESS)
		{
			if(ucRx_Old != ucRxPort)
				return TRUE;
			else
				return FALSE;
		}
		else
		{
			return FALSE;
		}
	}
	else
    {
        LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore(%d)\r\n", __FUNCTION__, __LINE__,ucCH);
        return FALSE;
    }

    return FALSE;
}

#if 0
//for Test
BOOL halScaler_4K3D_Set(UINT8 ucCH, UINT8 ucEnable)
{
	if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[ucCH].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(0);
            return FALSE;
        }

        dvProAV_SclFrameSeq3dModeSet(true);

        dvProAV_Scl3dIntVsModeSel(1);

        dvProAV_Scl_4K3dLrCtl(2);

        dvProAV_AccessWrite(0x81A01002L, 1);

		halScaler_SemaphoreGive(0);

	}
	else
    {
        LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore(%d)\r\n", __FUNCTION__, __LINE__,ucCH);
        return FALSE;
    }

    return FALSE;
}

BOOL halScaler_4K3DLRLR_Set(UINT8 ucCH, UINT8 ucEnable)
{
	if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[ucCH].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(0);
            return FALSE;
        }

        dvProAV_SclFrameSeq3dModeSet(true);
        dvProAV_SclDualPipe3dModeSet(false);

        dvProAV_Scl3dLrSyncDouble(FALSE);
        dvProAV_Scl_4K3dLrCtl(2);
        dvProAV_Scl3dIntVsModeSel(1);
        dvProAV_PixSft3dMdSel(ePixSftMd_2W4K3D);

        //dvProAV_AccessWrite(0x81A010FFL, 0x83);
        dvProAV_AccessWrite(eWcuWopLnDly, 3550);  // Line delay from WIP to WOP

        //dvProAV_AccessWrite(0x81A051FFL, 0x10);
        //dvProAV_AccessWrite(0x81A052FFL, 0x23);
        //dvProAV_AccessWrite(0x81A056FFL, 0x01);
        //dvProAV_AccessWrite(0x81A057FFL, 0x32);

        dvPro_PixelShift_Setting(ePIXEL_SHIFT_FRAME_SEQ_RL);

        halScaler_SemaphoreGive(0);
	}
	else
    {
        LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore(%d)\r\n", __FUNCTION__, __LINE__,ucCH);
        return FALSE;
    }

    return FALSE;
}

BOOL halScaler_4K3DLLRR_Set(UINT8 ucCH, UINT8 ucEnable)
{
	if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[ucCH].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(0);
            return FALSE;
        }

        dvProAV_SclFrameSeq3dModeSet(true);
        dvProAV_SclDualPipe3dModeSet(false);

        dvProAV_Scl3dLrSyncDouble(TRUE);
        dvProAV_Scl_4K3dLrCtl(2);
        dvProAV_Scl3dIntVsModeSel(1);
        dvProAV_PixSft3dMdSel(ePixSftMd_2W4K3D);

        //dvProAV_AccessWrite(0x81A010FFL, 0x83);
        dvProAV_AccessWrite(eWcuWopLnDly, 3550);  // Line delay from WIP to WOP

        //dvProAV_AccessWrite(0x81A051FFL, 0x30);
        //dvProAV_AccessWrite(0x81A052FFL, 0x21);
        //dvProAV_AccessWrite(0x81A056FFL, 0x01);
        //dvProAV_AccessWrite(0x81A057FFL, 0x23);

        dvPro_PixelShift_Setting(ePIXEL_SHIFT_DUAL_PIPE_RL);

        halScaler_SemaphoreGive(0);
	}
	else
    {
        LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore(%d)\r\n", __FUNCTION__, __LINE__,ucCH);
        return FALSE;
    }

    return FALSE;
}

BOOL halScaler_4K3DLRLR_DualPipe_Set(UINT8 ucCH, UINT8 ucEnable)
{
	if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[ucCH].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(0);
            return FALSE;
        }

        BYTE cData = 4;
        halFrontEndCtrl_SubInput_Set((UINT8 *)&cData, TRUE);

        dvPro_OSD_3D_Enable_Set(eMCT_CH1, eCM_3D_FORMAT_DUALPIPE_3D);
        dvPro_BuildPicture_2K(eMCT_CH1);

        dvProAV_Scl3dLrSyncDouble(FALSE);
        dvProAV_Scl_4K3dLrCtl(2);
        dvProAV_Scl3dIntVsModeSel(1);
        dvProAV_PixSft3dMdSel(ePixSftMd_2W4K3D);

        //dvProAV_AccessWrite(0x81A010FFL, 0x83);
        dvProAV_AccessWrite(eWcuWopLnDly, 3550);  // Line delay from WIP to WOP

        //dvProAV_AccessWrite(0x81A051FFL, 0x10);
        //dvProAV_AccessWrite(0x81A052FFL, 0x23);
        //dvProAV_AccessWrite(0x81A056FFL, 0x01);
        //dvProAV_AccessWrite(0x81A057FFL, 0x32);

        dvPro_PixelShift_Setting(ePIXEL_SHIFT_DUAL_PIPE_RL);

        dvPro_OSD_3D_Enable_Set(eMCT_CH1, eCM_3D_FORMAT_AUTO);

        halScaler_SemaphoreGive(0);

	}
	else
    {
        LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore(%d)\r\n", __FUNCTION__, __LINE__,ucCH);
        return FALSE;
    }

    return FALSE;
}

BOOL halScaler_4K3DLLRR_DualPipe_Set(UINT8 ucCH, UINT8 ucEnable)
{
	if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[ucCH].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(0);
            return FALSE;
        }

        BYTE cData = 4;
        halFrontEndCtrl_SubInput_Set((UINT8 *)&cData, TRUE);

        dvPro_OSD_3D_Enable_Set(eMCT_CH1, eCM_3D_FORMAT_DUALPIPE_3D);
        dvPro_BuildPicture_2K(eMCT_CH1);


        dvProAV_Scl3dLrSyncDouble(TRUE);
        dvProAV_Scl_4K3dLrCtl(2);
        dvProAV_Scl3dIntVsModeSel(1);
        dvProAV_PixSft3dMdSel(ePixSftMd_2W4K3D);

        //dvProAV_AccessWrite(0x81A010FFL, 0x83);
        dvProAV_AccessWrite(eWcuWopLnDly, 3550);  // Line delay from WIP to WOP

        //dvProAV_AccessWrite(0x81A051FFL, 0x30);
        //dvProAV_AccessWrite(0x81A052FFL, 0x21);
        //dvProAV_AccessWrite(0x81A056FFL, 0x01);
        //dvProAV_AccessWrite(0x81A057FFL, 0x23);

        dvPro_PixelShift_Setting(ePIXEL_SHIFT_DUAL_PIPE_RRLL);

        dvPro_OSD_3D_Enable_Set(eMCT_CH1, eCM_3D_FORMAT_AUTO);

        halScaler_SemaphoreGive(0);
	}
	else
    {
        LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore(%d)\r\n", __FUNCTION__, __LINE__,ucCH);
        return FALSE;
    }

    return FALSE;
}
#endif /* 0 */

eHAL_SCALER_EXEC_CODE halScaler_PixelShiftEnable_Set(UINT8 ucEnable)
{
	if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        //搭VPD XPR和4K3D B' mode才需要設定
        dvProAV_PixSftWcuEn(ucEnable);

        halScaler_SemaphoreGive(0);
	}
	else
    {
        LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore\r\n", __FUNCTION__, __LINE__);
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }

    return eHAL_SCALER_EXEC_CODE_PASS;
}

eHAL_SCALER_EXEC_CODE halScaler_PixelShiftCalPattern_Set(UINT8 ucEnable)
{
	if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        //Actuator cal pattern
        dvProAV_PixSftCalPatnEn((BOOL)ucEnable);

        halScaler_SemaphoreGive(0);
	}
	else
    {
        LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore\r\n", __FUNCTION__, __LINE__);
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }

    return eHAL_SCALER_EXEC_CODE_PASS;
}

eHAL_SCALER_EXEC_CODE halScaler_Default4K3DSEnable_Set(UINT8 ucMode)
{
	if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {

        switch(ucMode)
        {
            case eCM_3D_FORMAT_4K3D:
                dvProAV_PixSftModSel(0);
                dvProAV_Scl_4K3dLrCtl(2);
                dvProAV_4K3dDpBprnMdCtl(0);
#ifndef VPD_XPR_ENABLE
                if(Board_SingleBoard_Get() == FALSE)
                {
                    dvProAV_PixSft3dMdSel(ePixSftMd_4W4K3DPlus);
                }
#endif /* VPD_XPR_ENABLE */
                break;

            case eCM_3D_FORMAT_4K3D_DUALPIPE:
                dvProAV_PixSftModSel(0);
                dvProAV_Scl_4K3dLrCtl(1);
                dvProAV_4K3dDpBprnMdCtl(1);
#ifndef VPD_XPR_ENABLE
                if(Board_SingleBoard_Get() == FALSE)
                {
                    dvProAV_PixSft3dMdSel(ePixSftMd_2D);
                }
#endif /* VPD_XPR_ENABLE */
                break;

            default:
                dvProAV_PixSftModSel(1);
                dvProAV_Scl_4K3dLrCtl(0);
                dvProAV_4K3dDpBprnMdCtl(0);
#ifndef VPD_XPR_ENABLE
                if(Board_SingleBoard_Get() == FALSE)
                {
                    dvProAV_PixSft3dMdSel(ePixSftMd_2D);
                }
#endif /* VPD_XPR_ENABLE */
                break;
        }

        halScaler_SemaphoreGive(0);
	}
	else
    {
        LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore\r\n", __FUNCTION__, __LINE__);
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }

    return eHAL_SCALER_EXEC_CODE_PASS;
}

UINT8 halScaler_ColorSetting_Get(eHAL_COLOR_SETTING  ucNode, UINT8 ucCH) 	//A70LK_Doulas_0004
{
    UINT8  ucInputSource = halScaler_InputSource_Get(ucCH);
    UINT8  ucPresetMode = halScaler_PictureSettings_Get(ucCH);
    UINT8  ucSignalType = halScaler_SignalType_Get(ucCH);
    UINT8  ucPresetUserMode = halScaler_PictureSettings_PreUser_Get(ucCH);
    UINT8  ucValue;

    //LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): ColorSpace Get=%d\r\n", __FUNCTION__, __LINE__, ucSetting);
	//LOG_MSG(db_ALWAYS, "(mode:%d, mode:%d) input %d\r\n",ucPresetMode,ucPresetUserMode,ucInputSource);

    if (eCM_PICTURE_SETTINGS_USER != ucPresetMode)
    {

    	//LOG_MSG(db_ALWAYS, "(halScaler_ColorSetting_Get)\r\n");
    	switch(ucNode)
    	{
            case eHAL_COLOR_SETTING_CS:
            	ucValue = m_sUserSystemSetting.sColorSetting[ucInputSource][ucPresetMode][ucSignalType].ucCS;
            	break;

            case eHAL_COLOR_SETTING_CT:
            	ucValue = m_sUserSystemSetting.sColorSetting[ucInputSource][ucPresetMode][ucSignalType].ucCT;
            	break;

            case eHAL_COLOR_SETTING_GAMMA:
            	ucValue = m_sUserSystemSetting.sColorSetting[ucInputSource][ucPresetMode][ucSignalType].ucGamma;
            	break;

            case eHAL_COLOR_SETTING_BRILLIENTCOLOR:
            	ucValue = m_sUserSystemSetting.sColorSetting[ucInputSource][ucPresetMode][ucSignalType].ucBrillientColorEnabled;
            	break;

            case eHAL_COLOR_SETTING_WHITEPEAKING:
            	ucValue = m_sUserSystemSetting.sColorSetting[ucInputSource][ucPresetMode][ucSignalType].ucWhitePeaking;
            	break;

            case eHAL_COLOR_SETTING_COLORENHANCEMENT:
            	ucValue = m_sUserSystemSetting.sColorSetting[ucInputSource][ucPresetMode][ucSignalType].ucColorEnhancement;
            	break;

            case eHAL_COLOR_SETTING_SKINCOLOR:
            	ucValue = m_sUserSystemSetting.sColorSetting[ucInputSource][ucPresetMode][ucSignalType].ucSkinColor;
            	break;

            case eHAL_COLOR_SETTING_SHARPNESS:
            	ucValue = m_sUserSystemSetting.sColorSetting[ucInputSource][ucPresetMode][ucSignalType].ucSharpness;
            	break;

            case eHAL_COLOR_SETTING_BRIGHTNESS:
            	ucValue = m_sUserSystemSetting.sColorSetting[ucInputSource][ucPresetMode][ucSignalType].ucBrightness;
            	break;

            case eHAL_COLOR_SETTING_CONTRAST:
            	ucValue = m_sUserSystemSetting.sColorSetting[ucInputSource][ucPresetMode][ucSignalType].ucContrast;
            	break;


            case eHAL_COLOR_SETTING_TINT:
            	ucValue = m_sUserSystemSetting.sColorSetting[ucInputSource][ucPresetMode][ucSignalType].ucTint;
            	break;

            case eHAL_COLOR_SETTING_SATURATION:
            	ucValue = m_sUserSystemSetting.sColorSetting[ucInputSource][ucPresetMode][ucSignalType].ucSaturation;
            	break;

            case eHAL_COLOR_SETTING_REDGAIN:
            	ucValue = m_sUserSystemSetting.sColorSetting[ucInputSource][ucPresetMode][ucSignalType].ucRedGain;
            	break;


            case eHAL_COLOR_SETTING_GREENGAIN:
            	ucValue = m_sUserSystemSetting.sColorSetting[ucInputSource][ucPresetMode][ucSignalType].ucGreenGain;
            	break;

            case eHAL_COLOR_SETTING_BLUEGAIN:
            	ucValue = m_sUserSystemSetting.sColorSetting[ucInputSource][ucPresetMode][ucSignalType].ucBlueGain;
            	break;

            case eHAL_COLOR_SETTING_REDOFFSET:
            	ucValue = m_sUserSystemSetting.sColorSetting[ucInputSource][ucPresetMode][ucSignalType].ucRedOffset;
            	break;

            case eHAL_COLOR_SETTING_GREENOFFSET:
            	ucValue = m_sUserSystemSetting.sColorSetting[ucInputSource][ucPresetMode][ucSignalType].ucGreenOffset;
            	break;

            case eHAL_COLOR_SETTING_BLUEOFFSET:
            	ucValue = m_sUserSystemSetting.sColorSetting[ucInputSource][ucPresetMode][ucSignalType].ucBlueOffset;
            	break;
    	}
    }
    else
    {
    	//LOG_MSG(db_ALWAYS, "(halScaler_ColorSetting_Get user)\r\n");
    	switch(ucNode)
    	{
            case eHAL_COLOR_SETTING_CS:
            	ucValue = m_sUserSystemSetting.sUserColorSetting[ucInputSource][ucPresetUserMode][ucSignalType].ucCS;
            	break;

            case eHAL_COLOR_SETTING_CT:
            	ucValue = m_sUserSystemSetting.sUserColorSetting[ucInputSource][ucPresetUserMode][ucSignalType].ucCT;
            	break;

            case eHAL_COLOR_SETTING_GAMMA:
            	ucValue = m_sUserSystemSetting.sUserColorSetting[ucInputSource][ucPresetUserMode][ucSignalType].ucGamma;
            	break;

            case eHAL_COLOR_SETTING_BRILLIENTCOLOR:
            	ucValue = m_sUserSystemSetting.sUserColorSetting[ucInputSource][ucPresetUserMode][ucSignalType].ucBrillientColorEnabled;
            	break;


            case eHAL_COLOR_SETTING_WHITEPEAKING:
            	ucValue = m_sUserSystemSetting.sUserColorSetting[ucInputSource][ucPresetUserMode][ucSignalType].ucWhitePeaking;
            	break;

            case eHAL_COLOR_SETTING_COLORENHANCEMENT:
            	ucValue = m_sUserSystemSetting.sUserColorSetting[ucInputSource][ucPresetUserMode][ucSignalType].ucColorEnhancement;
            	break;

            case eHAL_COLOR_SETTING_SKINCOLOR:
            	ucValue = m_sUserSystemSetting.sUserColorSetting[ucInputSource][ucPresetUserMode][ucSignalType].ucSkinColor;
            	break;

            case eHAL_COLOR_SETTING_SHARPNESS:
            	ucValue = m_sUserSystemSetting.sUserColorSetting[ucInputSource][ucPresetUserMode][ucSignalType].ucSharpness;
            	break;

            case eHAL_COLOR_SETTING_BRIGHTNESS:
            	ucValue = m_sUserSystemSetting.sUserColorSetting[ucInputSource][ucPresetUserMode][ucSignalType].ucBrightness;
            	//LOG_MSG(db_ALWAYS, "(BRIGHTNESS %dr)\r\n",ucValue);
            	break;

            case eHAL_COLOR_SETTING_CONTRAST:
            	ucValue = m_sUserSystemSetting.sUserColorSetting[ucInputSource][ucPresetUserMode][ucSignalType].ucContrast;
            	break;

            case eHAL_COLOR_SETTING_TINT:
            	ucValue = m_sUserSystemSetting.sUserColorSetting[ucInputSource][ucPresetUserMode][ucSignalType].ucTint;
            	break;

            case eHAL_COLOR_SETTING_SATURATION:
            	ucValue = m_sUserSystemSetting.sUserColorSetting[ucInputSource][ucPresetUserMode][ucSignalType].ucSaturation;
            	break;

            case eHAL_COLOR_SETTING_REDGAIN:
            	ucValue = m_sUserSystemSetting.sUserColorSetting[ucInputSource][ucPresetUserMode][ucSignalType].ucRedGain;
            	break;

            case eHAL_COLOR_SETTING_GREENGAIN:
            	ucValue = m_sUserSystemSetting.sUserColorSetting[ucInputSource][ucPresetUserMode][ucSignalType].ucGreenGain;
            	break;

            case eHAL_COLOR_SETTING_BLUEGAIN:
            	ucValue = m_sUserSystemSetting.sUserColorSetting[ucInputSource][ucPresetUserMode][ucSignalType].ucBlueGain;
            	break;

            case eHAL_COLOR_SETTING_REDOFFSET:
            	ucValue = m_sUserSystemSetting.sUserColorSetting[ucInputSource][ucPresetUserMode][ucSignalType].ucRedOffset;
            	break;

            case eHAL_COLOR_SETTING_GREENOFFSET:
            	ucValue = m_sUserSystemSetting.sUserColorSetting[ucInputSource][ucPresetUserMode][ucSignalType].ucGreenOffset;
            	break;

            case eHAL_COLOR_SETTING_BLUEOFFSET:
            	ucValue = m_sUserSystemSetting.sUserColorSetting[ucInputSource][ucPresetUserMode][ucSignalType].ucBlueOffset;
            	break;
    	}
    }

    return ucValue;
}

// ==============================================================================
// FUNCTION NAME: halScaler_FpgaDataRGBLevelGet
// DESCRIPTION:
//
//
// Params:
// UINT8 dataPath:
//
// Returns:
//
//
// Modification History
// --------------------
// 2022/11/16, Doulas Create
// --------------------
// ==============================================================================
int halScaler_FpgaDataRGBLevelGet(uint16 *RGB_LEVEL_Max,uint16 *RGB_LEVEL_Min)  			//A70LK_Doulas_0012
{
    int status = rcERROR;
	DetLv VopRgbLv;

    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
		status = dvProAV_SclRgbLvGet(&VopRgbLv);
		if(status == rcSUCCESS)
		{
			RGB_LEVEL_Min[0] = VopRgbLv.Min.LvR;
			RGB_LEVEL_Min[1] = VopRgbLv.Min.LvG;
			RGB_LEVEL_Min[2] = VopRgbLv.Min.LvB;
			RGB_LEVEL_Max[0] = VopRgbLv.Max.LvR;
			RGB_LEVEL_Max[1] = VopRgbLv.Max.LvG;
			RGB_LEVEL_Max[2] = VopRgbLv.Max.LvB;
		}
        halScaler_SemaphoreGive(0);
    }

    return status;
}

// ==============================================================================
// FUNCTION NAME: halScaler_RxPortReset
// DESCRIPTION:
//
//
// Params:
// void:
//
// Returns:
//
//
// Modification History
// --------------------
// 2022/12/20, Larry Create
// --------------------
// ==============================================================================
void halScaler_RxPortReset(void)
{
    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        dvProAV_HdmiRxIpReset(true);
        dvProAV_HdmiRxIpReset(false);

        halScaler_SemaphoreGive(0);
    }
}

// ==============================================================================
// FUNCTION NAME: halScaler_DisableRGB
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
// 2022/12/22, Larry Create
// --------------------
// ==============================================================================
void halScaler_DisableRGB(eHAL_SCALER_RGB_ITEM RGB_Item) //A70LK_Casper_0008
{
    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        UINT8 ucMask = (RGB_MASK_R | RGB_MASK_G | RGB_MASK_B);
        RGBSwitch_Item[RGB_Item] = false; //A70LK_Casper_0008
        LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) RGBSwitchItem[%d] Turn off\r\n",__FUNCTION__, __LINE__, RGB_Item); //A70LK_Casper_0012
        dvPro_RGBMask(ucMask);  //A70LK_Simon_0017

        halScaler_SemaphoreGive(0);
    }
}

// ==============================================================================
// FUNCTION NAME: halScaler_EnableRGB
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
// 2023/02/02, Larry Create
// --------------------
// ==============================================================================
void halScaler_EnableRGB(void)
{
    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        bool all_on = true;
        int i = 0;

        for (i = 0; i < eHAL_SCALER_RGB_ITEM_COUNT; i++) //A70LK_Casper_0008
        {

    		LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) i = %d RGBSwitch_Item = %d\r\n",__FUNCTION__, __LINE__, i, RGBSwitch_Item[i]); //A70LK_Casper_0012
            if (!RGBSwitch_Item[i])
            {
                all_on = false;
                break;
            }
        }

        if (all_on) //A70LK_Casper_0008
        {
    		LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) RGBSwitchItem All Turn on\r\n",__FUNCTION__, __LINE__); //A70LK_Casper_0012
            dvPro_RGBMask(m_sHalScalerInfo[0].ucRGBMask & 0x07);   //A70LK_Simon_0017
        }

        halScaler_SemaphoreGive(0);
    }
}

void halScaler_RGBEnableItemSet(eHAL_SCALER_RGB_ITEM RGB_Item) //A70LK_Casper_0008
{
    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) RGBSwitchItem[%d] Turn on\r\n",__FUNCTION__, __LINE__, RGB_Item); //A70LK_Casper_0012
    RGBSwitch_Item[RGB_Item] = true;
}

void halScaler_RGBMaskSet(UINT8 ucMask)  //A70LK_Simon_0017
{
    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        m_sHalScalerInfo[0].ucRGBMask = ucMask;
        dvPro_RGBMask(m_sHalScalerInfo[0].ucRGBMask & 0x07);

        halScaler_SemaphoreGive(0);
    }
}

eHAL_SCALER_EXEC_CODE halScaler_4K3D_DualPipeOrientationSet(UINT8 ucFRC_Bypass,BOOL bRear,BOOL bCeiling)	//A70LK_Doulas_0017
{
    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);


	//if(halScaler_3DEnable_Get() != eCM_3D_FORMAT_4K3D_DUALPIPE)
	//	return eHAL_SCALER_EXEC_CODE_PASS;

    /* See if we can obtain the semaphore.  If the semaphore is not
    available wait 10 ticks to see if it becomes free. */
    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
    	int status = rcSUCCESS;

		#if 0
        LOG_MSG(db_ALWAYS, "Setting (%d)\r\n",ucColorFmt);
		switch(ucColorFmt)
		{
			case 0:
				status &= dvProAV_4K3dVopSubFrmIndOdrSet(0,1,2,3);
				break;
			case 1:
				status &= dvProAV_4K3dVopSubFrmIndOdrSet(0,1,3,2);
				break;
			case 2:
				status &= dvProAV_4K3dVopSubFrmIndOdrSet(0,2,1,3);
				break;
			case 3:
				status &= dvProAV_4K3dVopSubFrmIndOdrSet(0,2,3,1);
				break;
			case 4:
				status &= dvProAV_4K3dVopSubFrmIndOdrSet(0,3,1,2);
				break;
			case 5:
				status &= dvProAV_4K3dVopSubFrmIndOdrSet(0,3,2,1);
				break;

			case 6:
				status &= dvProAV_4K3dVopSubFrmIndOdrSet(1,0,2,3);
				break;
			case 7:
				status &= dvProAV_4K3dVopSubFrmIndOdrSet(1,0,3,2);
				break;
			case 8:
				status &= dvProAV_4K3dVopSubFrmIndOdrSet(1,2,0,3);
				break;
			case 9:
				status &= dvProAV_4K3dVopSubFrmIndOdrSet(1,2,3,0);
				break;
			case 10:
				status &= dvProAV_4K3dVopSubFrmIndOdrSet(1,3,0,2);
				break;
			case 11:
				status &= dvProAV_4K3dVopSubFrmIndOdrSet(1,3,2,0);
				break;

			case 12:
				status &= dvProAV_4K3dVopSubFrmIndOdrSet(2,0,1,3);
				break;
			case 13:
				status &= dvProAV_4K3dVopSubFrmIndOdrSet(2,0,3,1);
				break;
			case 14:
				status &= dvProAV_4K3dVopSubFrmIndOdrSet(2,1,0,3);
				break;
			case 15:
				status &= dvProAV_4K3dVopSubFrmIndOdrSet(2,1,3,0);
				break;
			case 16:
				status &= dvProAV_4K3dVopSubFrmIndOdrSet(2,3,0,1);
				break;
			case 17:
				status &= dvProAV_4K3dVopSubFrmIndOdrSet(2,3,1,0);
				break;

			case 18:
				status &= dvProAV_4K3dVopSubFrmIndOdrSet(3,0,1,2);
				break;
			case 19:
				status &= dvProAV_4K3dVopSubFrmIndOdrSet(3,0,2,1);
				break;
			case 20:
				status &= dvProAV_4K3dVopSubFrmIndOdrSet(3,1,0,2);
				break;
			case 21:
				status &= dvProAV_4K3dVopSubFrmIndOdrSet(3,1,2,0);
				break;
			case 22:
				status &= dvProAV_4K3dVopSubFrmIndOdrSet(3,2,0,1);
				break;
			case 23:
				status &= dvProAV_4K3dVopSubFrmIndOdrSet(3,2,1,0);
				break;

			//default:
		}
		#else
		LOG_MSG(db_HAL_SCALER, "FRC(%d),Cei(%d),Rea(%d)\r\n", ucFRC_Bypass,bCeiling,bRear);
		if(ucFRC_Bypass)  //FRC_bypass enable
		{
            status &= dvProAV_PixSftDdpDlySet(0); //DDP FRC 設多少就要設多少

			if(bCeiling)
			{
				if(bRear)
				{
					status &= dvProAV_4K3dVopSubFrmIndOdrSet(0,2,3,1);
				}
				else
				{
					status &= dvProAV_4K3dVopSubFrmIndOdrSet(1,3,2,0);
				}
			}
			else
			{
				if(bRear)
				{
					status &= dvProAV_4K3dVopSubFrmIndOdrSet(3,1,0,2);
				}
				else
				{
					status &= dvProAV_4K3dVopSubFrmIndOdrSet(2,0,1,3);//(1,3,2,0);
				}
			}
		}
		else
		{
            status &= dvProAV_PixSftDdpDlySet(1); //DDP FRC 設多少就要設多少

			if(bCeiling)
			{
				if(bRear)
				{
					status &= dvProAV_4K3dVopSubFrmIndOdrSet(2,3,1,0);//17
				}
				else
				{
					status &= dvProAV_4K3dVopSubFrmIndOdrSet(3,2,0,1);//22
				}
			}
			else
			{
				if(bRear)
				{
					status &= dvProAV_4K3dVopSubFrmIndOdrSet(1,0,2,3);
				}
				else
				{
					status &= dvProAV_4K3dVopSubFrmIndOdrSet(0,1,3,2);
				}
			}
		}
		#endif

        halScaler_SemaphoreGive(0);

		if(status != rcSUCCESS)
		{
			LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Setting fail\r\n", __FUNCTION__, __LINE__);
			return eHAL_SCALER_EXEC_CODE_FAIL;
		}
    }
    else
    {
        LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore\r\n", __FUNCTION__, __LINE__);
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }

    return eHAL_SCALER_EXEC_CODE_PASS;
}

void halScaler_RGBSwitchSet(bool bSwitch)  //A70LK_Casper_0011
{
    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        RGBTransSwitch = bSwitch;
        dvPro_RGBLSMdSet(RGBTransSwitch);

        halScaler_SemaphoreGive(0);
    }
}

BOOL halScaler_RGBSwitchGet(void)  //A70LK_Casper_0011
{
    return RGBTransSwitch;
}

eRESULT halScaler_Version_Get(uint32 *ulVersiong) //A70LK_Steven_0012 start
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    int status = rcSUCCESS;
    uint32 mainVer = 0, subVer = 0, minorVer = 0;

    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[0].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(0);
            eResult = eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }

        // Read ProAV FPGA Version
        status &= dvProAV_AccessRead(eComPrjMainRev, &mainVer);
        status &= dvProAV_AccessRead(eComPrjSubRev, &subVer);
        status &= dvProAV_AccessRead(eComPrjMinorRev, &minorVer);

        *ulVersiong = (uint32) (minorVer << 16 | mainVer << 8 | subVer);

        halScaler_SemaphoreGive(0);
    }
    else
    {
        LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore\r\n", __FUNCTION__, __LINE__);
        eResult = eHAL_SCALER_EXEC_CODE_FAIL;
    }

    if (eResult != eHAL_SCALER_EXEC_CODE_PASS || status != rcSUCCESS)
    {
    	return rcERROR;
    }
    else
    {
    	return rcSUCCESS;
    }

}  //A70LK_Steven_0012 end

void halScaler_PixSftModSet(bool bSwitch)
{
    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        //-false : by scaler
        //-true  : by TI XPR
        dvProAV_PixSftModSel(bSwitch);

        halScaler_SemaphoreGive(0);
    }
}

BOOL halScaler_PixModIsYUV420(UINT8 ucCH, UINT8 ucSelect)
{
    UINT8 ucRxPort = halScaler_CurrentSourceRxPortGet(ucCH, ucSelect);

    if(ucRxPort > PROA_RX_4)
    {
        return FALSE;
    }

    if(m_sProAV_Info.sRxPortAVI_Info[ucRxPort].ucPixelMode == PROAV_SCALER_COLOR_PIXEL_MODE_YUV420)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }

}

void halScaler_AutoSourceResyncSet(UINT8 ucAutoSourceResync)
{
    m_sUserSystemSetting.sCommonSetting.ucAutoSourceResync = ucAutoSourceResync;
}

UINT8 halScaler_AutoSourceResyncGet(void)
{
    return m_sUserSystemSetting.sCommonSetting.ucAutoSourceResync;
}

UINT32 halScaler_Reg0_Read_Get(void)
{
    UINT32 ulRet = 0;
    UINT32 ulCount;
    UINT32 ulError = 0;
    UINT32 uldata = 0;

    if(halScaler_SemaphoreTake(0 , __FUNCTION__) == TRUE)
    {
        for(ulCount = 0; ulCount < 10; ulCount++)
        {
            dvProAV_AccessRawDataWrite(0, 0, 1, (uint08*)&ulCount);

            dvProAV_AccessRawDataRead(0, 0, 1, (uint08*)&uldata);

            if(uldata != ulCount)
            {
                ulError++;
            }
        }

        halScaler_SemaphoreGive(0);
    }

    return ulError;
}

void halScaler_SetHDRDemo(void)
{
    if(halScaler_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        dvPro_SetHDRDemo();

        halScaler_SemaphoreGive(0);
    }
}
