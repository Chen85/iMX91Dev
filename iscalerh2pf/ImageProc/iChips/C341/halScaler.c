#include "dvC341.h"       //A70LV_Doulas_0013
#include "dvC341_Geo.h"   //H30K_Doulas_0011
#include "dvC341_OSD.h"
#include "halScaler.h"
//#include "dvC734.h"     //A70LV_Doulas_2000 remove
//#include "dvITE6802.h"
//#include "dvSI9777.h"
//#include "dvIT68051.h"
#include "dvVCXO949.h"
#include "dvCDCE913.h"
#include "utilDbgMsg.h"
//#include "Board_Gpio.h"
//#include "halC789CtrlAPI.h"     //A70LV_Doulas_0079
#include "halFrontEndCtrlAPI.h"     //A70LV_Doulas_0115
#include "halBoardCtrlAPI.h"        //A70LV_Doulas_0179
#include "halWarping.h"
#include "halFormatter.h"
#include "opdCtrlAPI.h"

#include "palSysCtrlMgr.h"  //H2 wait review

#define halC789Ctrl_ForcedSyncResetValueSet(a,b,c)
#define halC789Ctrl_Change_Panel(a)
#define halC789Ctrl_SemaphoreTake(a) TRUE
#define halC789Ctrl_SemaphoreGive(a)
#define halC789Ctrl_FrmaeDelaySet(a)
#define halC789Ctrl_ScreenSaveToImage(a,b,c,d) 1


typedef struct
{
    ePANEL_ID                    ePanelTimingId;
    BOOL                         bScalerInit;
    BOOL                         bScalerPowerNormalReady;  //A35G2_CDS_Simon_0034
    UINT32                       ulScalerErrCode;
    UINT32                       ulRxErrCode;
    UINT32                       ulSpliterErrCode;
    UINT32                       ulVCXOErrCode;
    SemaphoreHandle_t            xSemaphore;

#if(CURRENT_RTOS_TYPE == RTOS_STATIC)
    StaticSemaphore_t            xMutexBuffer;
#endif


}sHAL_SCALER_INFORMATION, *PsHAL_SCALER_INFORMATION;


static sUSER_SYSTEM_SETTING            m_sUserSystemSetting;   //A70LV_Doulas_0013

static sHAL_SCALER_INFORMATION m_sHalScalerInfo[eMCT_LAST];

static UINT8 ucFrontEndColorSpace[eMCT_LAST] = {eCOLOR_FORMAT_RGB,eCOLOR_FORMAT_RGB};       //A70LV_Doulas_0076
static UINT8 ucFrontEndeScanMode[eMCT_LAST] = {eSCAN_MODE_PROGRESSIVE,eSCAN_MODE_PROGRESSIVE};
static UINT8 ucFrontEndVideoYUV[eMCT_LAST] = {eVIDEO_YUV_UNKNOW,eVIDEO_YUV_UNKNOW};   //A70LV_Doulas_0109
static UINT8 ucVGA_SYNC_TYPE[eMCT_LAST] = {eVGA_SYNC_TYPE_UNKNOW,eVGA_SYNC_TYPE_UNKNOW};        //A70LV_Doulas_0124
static BOOL m_ucLogo_Load = FALSE;

#if 0  //simon check cap
static UINT32 m_ulCapturedReg[] =   //G100_Owen_0060
{
    B0_OCFMTCH1, B0_OSCWCTCH1,  //interlance
    B4_HZOOMCTCH1, B4_HZSCLCH1, B4_VZOOMCTCH1, B4_VZSCLCH1, B9_HSHRNKCTCH1, B9_VSHRNKCTCH1, //aspect ratio
    B5_HEGCTCH1, B5_HEGCOEF0CH1, B5_HEGCOEF1CH1, B5_HEGCOEF2CH1, B5_HEGCOEF3CH1, B5_HEGCOEF4CH1, B5_HEGGAINCH1, //sharpness
    B5_VEGCTCH1, B5_VEGCOEF0CH1, B5_VEGCOEF1CH1, B5_VEGCOEF2CH1, B5_VEGCOEF3CH1, B5_VEGCOEF4CH1, B5_VEGGAINCH1,
    B6_BGCTCH1, B6_BIASRCH1, B6_BIASGCH1, B6_BIASBCH1, B6_OGAIN1RCH1, B6_OGAIN1GCH1, B6_OGAIN1BCH1, //gain correction
    B6_BGCT2CH1, B6_OGAIN2RCH1, B6_OGAIN2GCH1, B6_OGAIN2BCH1, B6_OBIAS2RCH1, B6_OBIAS2GCH1, B6_OBIAS2BCH1,
    B2_OACTHSTCH1, B2_OACTHWCH1, B2_OACTVSTCH1, B2_OACTVWCH1, B2_OIMGCTCH1, //output setting
};
#endif

static inline BOOL halScaler_SemaphoreTake(const char *pcFunc)
{
    BOOL bResult = dvC341_SemaphoreTake(TRUE, pcFunc);

    return bResult;
}


static inline BOOL halScaler_SemaphoreGive(const char *pcFunc)
{
    //xSemaphoreGive(m_sHalScalerInfo[ucCH].xSemaphore);
    BOOL bResult = dvC341_SemaphoreTake(FALSE, pcFunc);

    return bResult;
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


eHAL_SCALER_EXEC_CODE halScaler_PowerStandby(void)
{
    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);

    m_sHalScalerInfo[eMCT_CH1].bScalerPowerNormalReady = FALSE;   //A35G2_CDS_Simon_0034
    m_sHalScalerInfo[eMCT_CH2].bScalerPowerNormalReady = FALSE;

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

    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);


    {
        if(ucCH >= eMCT_LAST)
            return eHAL_SCALER_EXEC_CH_OVER_RANGE;
    }


#if (CURRENT_MAIN_BOARD == ICHIP_BOARD)

    iEBopen();

    if(bIs4K == FALSE)
    {
        if(ePanelId == ePANEL_ID_720P_60HZ)
            ieb_setup(1);
        else
            ieb_setup(0);

        ieb_set_hdcpct(0);
    }
    else
    {
        ieb_setup(2);
        ieb_set_hdcpct(1);
    }

#endif

    if(halScaler_SemaphoreTake(__FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[ucCH].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_Init() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(__FUNCTION__);

            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }


        {
            //UINT8 ucOverlay[eMCT_LAST]={0, 1};    //A70LV_Doulas_0008 remove

            m_sHalScalerInfo[ucCH].ePanelTimingId = ePanelId;

#ifdef INIT_VCXO949 //A70LV_Doulas_0005 //A70LV_Doulas_0002 debug remove
            if(ucCH == eMCT_CH1)
            {
                eVCXO_SEL eSel;     //A70LV_Doulas_0072 modify

                switch(m_sHalScalerInfo[ucCH].ePanelTimingId)    //A70LV_Doulas_0094 modify
                {
                    case ePANEL_ID_1080P_60HZ:
                        eSel = eVCXO_SEL_C341_HD;
                        break;

                    case ePANEL_ID_WUXGA_60HZ:
                        eSel = eVCXO_SEL_C341_WU;
                        break;

                    case ePANEL_ID_720P_120HZ:
                        eSel = eVCXO_SEL_C341_720P120;
                        break;

                    case ePANEL_ID_SVGA_120HZ:
                        eSel = eVCXO_SEL_C341_800x600_120;
                        break;

                    case ePANEL_ID_1080P_120HZ:
                        eSel = eVCXO_SEL_C341_1080P120;
                        break;

                    case ePANEL_ID_WUXGA_120HZ:
                        eSel = eVCXO_SEL_C341_WUXGA120;
                        break;

                    default:
                        eSel = eVCXO_SEL_C341_HD;
                        break;
                }

                if((ulExeRet = dvVCXO949_Init(eSel)) != eVCXO_EXEC_CODE_PASS)
                {
                    m_sHalScalerInfo[ucCH].ulVCXOErrCode = ulExeRet;
                    halScaler_SemaphoreGive(__FUNCTION__);
                    return eHAL_SCALER_EXEC_CODE_DV_RX_INIT_FAIL;
                }
            }
#endif

            //halScaler_IintChannelSetting(eMCT_CH1); //ZU860_Doulas_0082
            //halScaler_IintChannelSetting(eMCT_CH2); //ZU860_Doulas_0082
            halScaler_IintChannelSetting(ucCH);
            if(ucCH == eMCT_CH1)    //G100_Owen_0052
            {
                m_ucLogo_Load = FALSE;
            }

            if((ulExeRet = dvC341_Init_4K((eMCT)ucCH, ePanelId)) != eSCALER_EXEC_CODE_PASS)
            {
                LOG_MSG(db_HAL_SCALER, "!!!(func:%s, line:%d)dvC341_Init_4K fail\r\n", __FUNCTION__, __LINE__);

                m_sHalScalerInfo[ucCH].ulScalerErrCode = ulExeRet;
                halScaler_SemaphoreGive(__FUNCTION__);
                return eHAL_SCALER_EXEC_CODE_DV_SCALER_INIT_FAIL;
            }

            LOG_MSG(db_HAL_SCALER, "!!!(func:%s, line:%d)dvC341_Init_4K[%d] done\r\n", __FUNCTION__, __LINE__, ucCH);

            //TODO
            dvC341_SetOverlay_4K((eMCT)ucCH, !!m_sUserSystemSetting.sSourceSetting.ucPIPEnable);  //A70LV_Doulas_0024
            //dvC341_SetOverlayCoef_2K((eMCT)ucCH, ucOverlay);

            LOG_MSG(db_HAL_SCALER, "!!!(func:%s, line:%d)dvC341_SetOverlay_4K[%d] done\r\n", __FUNCTION__, __LINE__, ucCH);

            dvC341_SetInputPort_2K((eMCT)ucCH);
            LOG_MSG(db_HAL_SCALER, "!!!(func:%s, line:%d)dvC341_SetInputPort_2K[%d] done\r\n", __FUNCTION__, __LINE__, ucCH);
        }

        m_sHalScalerInfo[ucCH].bScalerPowerNormalReady = TRUE;   //A35G2_CDS_Simon_0034

        halScaler_SemaphoreGive(__FUNCTION__);

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
    if(halScaler_SemaphoreTake(__FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[ucCH].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(__FUNCTION__);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }


        dvC341_BuildPicture_4K((eMCT)ucCH);

        if(ucCH == eMCT_CH1)    //A70LV_Doulas_0079
        {
            BOOL   bForceSyncReseVal;
            UINT16 uiH_Toral;
            UINT16 uiV_Toral;
            dvC341_ForceSyncReset_Get(&bForceSyncReseVal,&uiH_Toral,&uiV_Toral);

#ifdef C341GEO_2D_NO_FRAME_LOCK   //A70LV_Doulas_0307 modify
            dvC341Geo_ForcedSyncResetValueSet(bForceSyncReseVal,uiH_Toral,uiV_Toral); //H30K_Doulas_0016
            #if 0
     		if(!halScaler_Low_Latency_Get())	//G100_Clare_0008
			{
		        if(bForceSyncReseVal)   //ZU860_Doulas_0054 Modify
                {
                    if((halScaler_IS_3D_Enable() == ets_ON) ||
                        (m_sHalScalerInfo[ucCH].ePanelTimingId == ePANEL_ID_WUXGA_120HZ) ||  //A70LV_Doulas_0377 Add
                       (m_sHalScalerInfo[ucCH].ePanelTimingId == ePANEL_ID_1080P_120HZ))
                    {
                    	halC789Ctrl_ForcedSyncResetValueSet((UINT8)TRUE,uiH_Toral,uiV_Toral);
                    }
					else
					{
                        halC789Ctrl_ForcedSyncResetValueSet((UINT8)FALSE,uiH_Toral,uiV_Toral);
	                }
				}
                else
                {
                    halC789Ctrl_ForcedSyncResetValueSet((UINT8)FALSE,uiH_Toral,uiV_Toral);
                }
			}
			else	//G100_Clare_0008
			{
                if(bForceSyncReseVal)
                {
                    halC789Ctrl_ForcedSyncResetValueSet((UINT8)TRUE,uiH_Toral,uiV_Toral);
                }
			}
			#endif
        }
         //   dvC341_SetOutputColor_2K((eMCT)ucCH, (eCOLOR_FORMAT)ucColorFmt);  //A70LV_Doulas_0002 debug
#else
        //if(bForceSyncReseVal)
        //{
            halC789Ctrl_ForcedSyncResetValueSet((UINT8)TRUE,uiH_Toral,uiV_Toral);
        }
#endif

        halScaler_SemaphoreGive(__FUNCTION__);

    }
    else
    {
        LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore\r\n", __FUNCTION__, __LINE__);
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }


    return eHAL_SCALER_EXEC_CODE_PASS;
}

eHAL_SCALER_EXEC_CODE halScaler_ConfigureForDisplay_ChangePIPLayout(UINT8 ucCH, const UINT8 ucColorFmt)     //A35G2_CDS_Simon_0001
{
    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);

    //bIs4K = halScaler_Is4KPanel(m_sHalScalerInfo[ucCH].ePanelTimingId);

    {
        if(ucCH >= eMCT_LAST)
            return eHAL_SCALER_EXEC_CH_OVER_RANGE;
    }


    /* See if we can obtain the semaphore.  If the semaphore is not
    available wait 10 ticks to see if it becomes free. */
    if(halScaler_SemaphoreTake(__FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[ucCH].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(__FUNCTION__);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }

        dvC341_BuildPicture_4K_ChangePIPLayout((eMCT)ucCH);

        if(ucCH == eMCT_CH1)    //A70LV_Doulas_0079
        {
            BOOL   bForceSyncReseVal;
            UINT16 uiH_Toral;
            UINT16 uiV_Toral;
            dvC341_ForceSyncReset_Get(&bForceSyncReseVal,&uiH_Toral,&uiV_Toral);
#ifdef C789_2D_NO_FRAME_LOCK   //A70LV_Doulas_0307 modify
			#ifdef Low_Latency_All	//G100_Clare_0009
     		if(!halScaler_Is_LowLatencyMode_On())	//G100_Clare_0008   //H2PF_Simon_0187
	        #endif	/*Low_Latency_All*/
			{
		        if(bForceSyncReseVal)   //ZU860_Doulas_0054 Modify
                {
                    if((halScaler_IS_3D_Enable() == ets_ON) ||
                        (m_sHalScalerInfo[ucCH].ePanelTimingId == ePANEL_ID_WUXGA_120HZ) ||  //A70LV_Doulas_0377 Add
                       (m_sHalScalerInfo[ucCH].ePanelTimingId == ePANEL_ID_1080P_120HZ))
                    {
                    	halC789Ctrl_ForcedSyncResetValueSet((UINT8)TRUE,uiH_Toral,uiV_Toral);
                    }
					else
					{
                        halC789Ctrl_ForcedSyncResetValueSet((UINT8)FALSE,uiH_Toral,uiV_Toral);
	                }
				}
                else
                {
                    halC789Ctrl_ForcedSyncResetValueSet((UINT8)FALSE,uiH_Toral,uiV_Toral);
                }
			}
#ifdef Low_Latency_All	//G100_Clare_0009
			else	//G100_Clare_0008
			{
                if(bForceSyncReseVal)
                {
                    halC789Ctrl_ForcedSyncResetValueSet((UINT8)TRUE,uiH_Toral,uiV_Toral);
                }
			}
			#endif	/*Low_Latency_All*/
#else
            if(bForceSyncReseVal)
            {
                halC789Ctrl_ForcedSyncResetValueSet((UINT8)TRUE,uiH_Toral,uiV_Toral);
            }
#endif
        }
         //   dvC341_SetOutputColor_2K((eMCT)ucCH, (eCOLOR_FORMAT)ucColorFmt);  //A70LV_Doulas_0002 debug

        halScaler_SemaphoreGive(__FUNCTION__);

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
    if(halScaler_SemaphoreTake(__FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[ucCH].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(__FUNCTION__);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }

        dvC341_SetAutoFill_2K((eMCT)ucCH, ucFillColor, bEnable);

        halScaler_SemaphoreGive(__FUNCTION__);

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

eHAL_SCALER_EXEC_CODE halScaler_SetOverlay(UINT8 ucCH, UINT8 ucEnable)
{

    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);

    //bIs4K = halScaler_Is4KPanel(m_sHalScalerInfo[ucCH].ePanelTimingId);

    {
        if(ucCH >= eMCT_LAST)
            return eHAL_SCALER_EXEC_CH_OVER_RANGE;


        /* See if we can obtain the semaphore.  If the semaphore is not
        available wait 10 ticks to see if it becomes free. */
        if(halScaler_SemaphoreTake(__FUNCTION__) == TRUE)
        {
            if(!m_sHalScalerInfo[ucCH].bScalerInit)
            {
                LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
                halScaler_SemaphoreGive(__FUNCTION__);
                return eHAL_SCALER_EXEC_CODE_NOT_INIT;
            }

            dvC341_SetOverlay_4K((eMCT)ucCH, ucEnable);
            halScaler_SemaphoreGive(__FUNCTION__);

        }
        else
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore\r\n", __FUNCTION__, __LINE__);
            return eHAL_SCALER_EXEC_CODE_FAIL;
        }
    }


    return eHAL_SCALER_EXEC_CODE_PASS;
}

#if 0 //unused
eHAL_SCALER_EXEC_CODE halScaler_SetOverlayCoef(UINT8 ucCH, const PUINT8 pucOverLay)
{

    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);

    //bIs4K = halScaler_Is4KPanel(m_sHalScalerInfo[ucCH].ePanelTimingId);


    {
        if(ucCH >= eMCT_LAST)
            return eHAL_SCALER_EXEC_CH_OVER_RANGE;


        /* See if we can obtain the semaphore.  If the semaphore is not
        available wait 10 ticks to see if it becomes free. */
        if(halScaler_SemaphoreTake(__FUNCTION__) == TRUE)
        {
            if(!m_sHalScalerInfo[ucCH].bScalerInit)
            {
                LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
                halScaler_SemaphoreGive(__FUNCTION__);
                return eHAL_SCALER_EXEC_CODE_NOT_INIT;
            }

            dvC341_SetOverlayCoef_2K((eMCT)ucCH, pucOverLay);
            halScaler_SemaphoreGive(__FUNCTION__);

        }
        else
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore\r\n", __FUNCTION__, __LINE__);
            return eHAL_SCALER_EXEC_CODE_FAIL;
        }

    }

    return eHAL_SCALER_EXEC_CODE_PASS;
}
#endif


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
    if(halScaler_SemaphoreTake(__FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[ucCH].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(__FUNCTION__);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }

        {
            if(dvC341_CheckInput_4K((eMCT)ucCH))
            {
				dvC341_SetIdual_2K((eMCT)ucCH);
            }
            else
            {
                eResult = eHAL_SCALER_EXEC_CODE_FAIL; //A70LV_Doulas_0002
            }
        }

        halScaler_SemaphoreGive(__FUNCTION__);

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
    if(pthread_mutex_init(&m_sHalScalerInfo[ucCH].xSemaphore, NULL) != 0)
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

    eResult = halScaler_Brightness_Value_Set(ucCH,ucSetting);   //A70LV_Doulas_0012 modify
    if(eResult != eHAL_SCALER_EXEC_CODE_PASS)
        return eResult;

    if(halScaler_SemaphoreTake(__FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[0].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(__FUNCTION__);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }
        //G100_Steven_0079 start
        if(halScaler_PIP_PBP_Enable_Get())  //A70LV_Doulas_0389 Modify
        {
        	//LOG_MSG(db_ALWAYS, "Brightness333_Set CH %d, Val %d\r\n",ucCH);
            dvC341_SetBrightness((eMCT)ucCH);
        }
        else
        {
        	 if(ucCH == eMCT_CH1)
        	 {
        		 dvC341_SetBrightness(eMCT_CH1);       //A70LV_Doulas_0155
        		 dvC341_SetBrightness(eMCT_CH2);       //A70LV_Doulas_0155
        	 }
        } //G100_Steven_0079 end

        halScaler_SemaphoreGive(__FUNCTION__);
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

    if(halScaler_SemaphoreTake(__FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[0].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(__FUNCTION__);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }

        //execute Contrast action function
        if(halScaler_PIP_PBP_Enable_Get()) //G100_Steven_0079 start
        {
            dvC341_SetContrast((eMCT)ucCH);
        }
        else
        {
            if(ucCH == eMCT_CH1)
            {
            	dvC341_SetContrast(eMCT_CH1);     //A70LV_Doulas_0155
            	dvC341_SetContrast(eMCT_CH2);     //A70LV_Doulas_0155
            }
        }//G100_Steven_0079 end

        halScaler_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore\r\n", __FUNCTION__, __LINE__);
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }

    return eResult;
}

INT8 halScaler_Dump_Register(void)     //A70LV_Doulas_0002
{
    if(halScaler_SemaphoreTake(__FUNCTION__) == TRUE)
    {
        dvC341_RegDump();
        halScaler_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore\r\n", __FUNCTION__, __LINE__);
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }

    return HAL_SCALER_PASS;
}

eHAL_SCALER_EXEC_CODE halScaler_Bypasse_Mode(void)     //A70LV_Doulas_0072 modify//A70LV_Doulas_0002
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

//    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);


    /* See if we can obtain the semaphore.  If the semaphore is not
    available wait 10 ticks to see if it becomes free. */
    if(halScaler_SemaphoreTake(__FUNCTION__) == TRUE)
    {
        dvC341_Bypass();
        halScaler_SemaphoreGive(__FUNCTION__);
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
    dvC341_Test();

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
    if(halScaler_SemaphoreTake(__FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[ucCH].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(__FUNCTION__);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }

        dvC341_Set_HUE(eMCT_CH1);        //A70LV_Doulas_0155 //A70LV_Doulas_0008 modify
        dvC341_Set_HUE(eMCT_CH2);        //A70LV_Doulas_0155
        halScaler_SemaphoreGive(__FUNCTION__);
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
    if(halScaler_SemaphoreTake(__FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[ucCH].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(__FUNCTION__);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }

        dvC341_Set_Saturation(eMCT_CH1);        //A70LV_Doulas_0155 modify//A70LV_Doulas_0008 modify
        dvC341_Set_Saturation(eMCT_CH2);        //A70LV_Doulas_0155

        halScaler_SemaphoreGive(__FUNCTION__);

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
    //LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): ucLayout=%d\r\n", __FUNCTION__, __LINE__, ucLayout);

    if(ucLayout > eCM_MAIN_LAYOUT_NUMBER)
        return eHAL_SCALER_EXEC_CH_OVER_RANGE;

    m_sUserSystemSetting.sSourceSetting.ucMainLayout = ucLayout;

    dvC341_Set_PIP_PBP_Layout(ucLayout);

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_PIP_PBP_Size_Set(UINT8 ucSize)    //A70LV_Doulas_0072 modify//A70LV_Doulas_0024
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    //LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): Brightness=%d\r\n", __FUNCTION__, __LINE__, miBrightness);

    if(ucSize >= eCM_PIP_SIZE_NUMBER)
        return eHAL_SCALER_EXEC_CH_OVER_RANGE;

    m_sUserSystemSetting.sSourceSetting.ucPIPSize = ucSize;

    dvC341_Set_PIP_PBP_Size(ucSize);

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
    dvC341_Resync_Init((eMCT)eCH);  //A70LV_Doulas_0008 modify
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
    if(halScaler_SemaphoreTake(__FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[ucCH].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(__FUNCTION__);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }

        bSyncLock = dvC341_SYNC_LOCK_Compare((eMCT)ucCH);    //A70LV_Doulas_0009 modify
        if(bSyncLock)
        {
            eResult = eHAL_SCALER_EXEC_CODE_PASS;
        }
        else
        {
            eResult = eHAL_SCALER_EXEC_CODE_FAIL;
        }

        halScaler_SemaphoreGive(__FUNCTION__);

    }
    else
    {
        //*bSyncLock = FALSE;   //A70LV_Doulas_0009 remove
        LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore(%d)\r\n", __FUNCTION__, __LINE__,ucCH);
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }

    return eResult;
}

//A35G2_CDS_Simon_0041
eHAL_SCALER_EXEC_CODE halScaler_CheckChannelSyncCount(UINT8 *ucIsSimilar)
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

    if(halScaler_SemaphoreTake(__FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[eMCT_CH1].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(__FUNCTION__);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }

        INT32 lCH1_IMFH, lCH1_IMFV = 0;
        INT32 lCH2_IMFH, lCH2_IMFV = 0;
        dvC341_InputPort_Sync_MCLK_Count_Get(eMCT_CH1, &lCH1_IMFH, &lCH1_IMFV);
        dvC341_InputPort_Sync_MCLK_Count_Get(eMCT_CH2, &lCH2_IMFH, &lCH2_IMFV);

        LOG_MSG(db_HAL_SCALER, "<<CH1 %d %d>> <<CH2 %d %d>>\n", lCH1_IMFH, lCH1_IMFV, lCH2_IMFH, lCH2_IMFV);

        if( lCH1_IMFH != 0 &&
            lCH1_IMFH != 0xFFFF &&
            lCH1_IMFV != 0 &&
            lCH1_IMFV != 0xFFFFFF &&
            lCH2_IMFH != 0 &&
            lCH2_IMFH != 0xFFFF &&
            lCH2_IMFV != 0 &&
            lCH2_IMFV != 0xFFFFFF &&
            ABS_DIFF(lCH1_IMFH, lCH2_IMFH) <= IMFH_MCLK_TOLERANCE &&
            ABS_DIFF(lCH1_IMFV, lCH2_IMFV) <= IMFV_MCLK_TOLERANCE
        )
        {
            *ucIsSimilar = TRUE;
        }
        else
        {
            *ucIsSimilar = FALSE;
        }

        halScaler_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore\r\n", __FUNCTION__, __LINE__);
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
    if(halScaler_SemaphoreTake(__FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[ucCH].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(__FUNCTION__);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }

        dvC341_Freeze_Set((eMCT)ucCH,bFreezeEn);    //A70LV_Doulas_0008 modify

        halScaler_SemaphoreGive(__FUNCTION__);

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

    if(!m_sHalScalerInfo[ucCH].bScalerInit)
    {
        LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
        return eHAL_SCALER_EXEC_CODE_NOT_INIT;
    }

    *bFreezeEn = dvC341_Freeze_Get((eMCT)ucCH);     //A70LV_Doulas_0008 modify

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
    if(halScaler_SemaphoreTake(__FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[ucCH].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(__FUNCTION__);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }

        if(ucCH == eMCT_CH1)
        {
    		halFormatter_FRCByPassModeSet(FALSE);   //A35G2_CDS_Simon_0004
            //dvC341_Panel_Change_LVDS_Stop();
            dvC341_Panel_Change_Setting(eMCT_CH1,ePanelId);
            dvC341Geo_Change_Panel(ePanelId); //H30K_Doulas_0010
            halWarping_Panel_Update(ePanelId);

        #ifdef SCALER_C821_C789  //H30K_Doulas_0001
            halC789Ctrl_Change_Panel(ePanelId);     //A70LV_Doulas_0098
        //    halBoard_XillinxFPGA_Reset();       //A70LV_Doulas_0216 remove//A70LV_Doulas_0179
            if(palSysCtrlMgr_XillinxFPGA_NewVersionGet() == TRUE)        //A70LV_Doulas_0218 modify
            {
                palSysCtrlMgr_XillinxFPGA_Manual_Mode_Set(ePanelId);    //A70LV_Doulas_0216
            }
            else
            {
                palSysCtrlMgr_XillinxFPGA_Reset();
            }
        #endif
        }

        halScaler_SemaphoreGive(__FUNCTION__);

    }
    else
    {
        LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore(%d)\r\n", __FUNCTION__, __LINE__,ucCH);
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }

    return eResult;
}

#ifdef INIT_VCXO949
UINT32 halScaler_Panel_Colck_Set(ePANEL_ID ePanelId)    //A70LV_Doulas_0005
{
    UINT32 ulExeRet = eVCXO_EXEC_CODE_PASS;


    switch(ePanelId)
    {
        case ePANEL_ID_WUXGA_60HZ:
            ulExeRet = dvVCXO949_Init(eVCXO_SEL_C341_WU);
            break;

        case ePANEL_ID_1080P_60HZ:
            ulExeRet = dvVCXO949_Init(eVCXO_SEL_C341_HD);
            break;

        case ePANEL_ID_720P_120HZ:      //A70LV_Doulas_0094
            ulExeRet = dvVCXO949_Init(eVCXO_SEL_C341_720P120);
            break;

        case ePANEL_ID_SVGA_120HZ:      //A70LV_Doulas_0094
            ulExeRet = dvVCXO949_Init(eVCXO_SEL_C341_800x600_120);
            break;

        case ePANEL_ID_1080P_120HZ:      //A70LV_Doulas_0094
            ulExeRet = dvVCXO949_Init(eVCXO_SEL_C341_1080P120);
            break;

        case ePANEL_ID_WUXGA_120HZ:      //A70LV_Doulas_0094
            ulExeRet = dvVCXO949_Init(eVCXO_SEL_C341_WUXGA120);
            break;
    }
    return ulExeRet;
}
#endif

eHAL_SCALER_EXEC_CODE halScaler_AutoPhaseStart(UINT8 ucCH)  //A70LV_Doulas_0007
{
    return eHAL_SCALER_EXEC_CODE_FAIL;

    #if 0
    eHAL_SCALER_EXEC_CODE ulExeRet = eHAL_SCALER_EXEC_CODE_PASS;
    eSCALER_EXEC_CODE eResult = eSCALER_EXEC_CODE_PASS;

    if(ucCH >= eMCT_LAST)
            return eHAL_SCALER_EXEC_CH_OVER_RANGE;

    /* See if we can obtain the semaphore.  If the semaphore is not
    available wait 10 ticks to see if it becomes free. */
    if(halScaler_SemaphoreTake(__FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[ucCH].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(__FUNCTION__);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }

        eResult = dvC341_AutoPhaseStart((eMCT)ucCH);    //A70LV_Doulas_0008 modify
        if(eResult == eSCALER_EXEC_CODE_PASS)
        {
            //A70LV_Doulas_0211 modify//A70LV_Doulas_0116 modify
            UINT8 ucCurrentPhase;
            BOOL   bForceSyncReseVal;   //A70LV_Doulas_0251
            UINT16 uiH_Toral;           //A70LV_Doulas_0251
            UINT16 uiV_Toral;           //A70LV_Doulas_0251
            if(ucCH == eMCT_CH1)        //A70LV_Doulas_0251
            {
                dvC341_ForceSyncRese_Get(&bForceSyncReseVal,&uiH_Toral,&uiV_Toral);
#ifdef C789_2D_NO_FRAME_LOCK   //A70LV_Doulas_0307 modify
				#ifdef Low_Latency_All	//G100_Clare_0009
				if(!halScaler_Low_Latency_Get())	//G100_Clare_0008
				#endif	/*Low_Latency_All*/
				{
	                if(bForceSyncReseVal)       //ZU860_Doulas_0054
	                {
	                    if((halScaler_IS_3D_Enable() == ets_ON) ||
	                        (m_sHalScalerInfo[ucCH].ePanelTimingId == ePANEL_ID_WUXGA_120HZ) ||  //A70LV_Doulas_0377 Add
	                       (m_sHalScalerInfo[ucCH].ePanelTimingId == ePANEL_ID_1080P_120HZ))
	                    {
	                    	halC789Ctrl_ForcedSyncResetValueSet((UINT8)TRUE,uiH_Toral,uiV_Toral);
						}
	                    else
	                    {
	                        halC789Ctrl_ForcedSyncResetValueSet((UINT8)FALSE,uiH_Toral,uiV_Toral);
						}
	                }
	                else
	                {
	                    halC789Ctrl_ForcedSyncResetValueSet((UINT8)FALSE,uiH_Toral,uiV_Toral);
	                }
				}
				#ifdef Low_Latency_All	//G100_Clare_0009
				else	//G100_Clare_0008
				{
	                if(bForceSyncReseVal)
	                {
	                    halC789Ctrl_ForcedSyncResetValueSet((UINT8)TRUE,uiH_Toral,uiV_Toral);
	                }
				}
				#endif	/*Low_Latency_All*/
#else
                if(bForceSyncReseVal)
                {
                    halC789Ctrl_ForcedSyncResetValueSet((UINT8)TRUE,uiH_Toral,uiV_Toral);
                }
#endif
            }
            ucCurrentPhase = dvC341_CurrentPhaseGet((eMCT)ucCH);
            halFrontEndCtrl_VGA_Phase_Set(&ucCurrentPhase);
            ulExeRet = eHAL_SCALER_EXEC_CODE_PASS;
        }
        else
        {
            ulExeRet = eHAL_SCALER_EXEC_CODE_NO_AUTO_PHASE;
        }

        halScaler_SemaphoreGive(__FUNCTION__);

    }
    else
    {
        LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore(%d)\r\n", __FUNCTION__, __LINE__,ucCH);
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }

    return ulExeRet;
#endif
}

eHAL_SCALER_EXEC_CODE halScaler_AutoPhase(UINT8 ucCH)  //A70LV_Doulas_0007
{
    return eHAL_SCALER_EXEC_CODE_FAIL;
#if 0
    eHAL_SCALER_EXEC_CODE ulExeRet = eHAL_SCALER_EXEC_CODE_PASS;
    eSCALER_EXEC_CODE eResult = eSCALER_EXEC_CODE_PASS;
    UINT8 ucCurrentPhase;
	//UINT8 ucBestPhase;

    if(ucCH >= eMCT_LAST)
            return eHAL_SCALER_EXEC_CH_OVER_RANGE;

    /* See if we can obtain the semaphore.  If the semaphore is not
    available wait 10 ticks to see if it becomes free. */
    if(halScaler_SemaphoreTake(__FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[ucCH].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(__FUNCTION__);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }

        eResult = dvC341_AutoPhase((eMCT)ucCH);     //A70LV_Doulas_0008 modify
        if(eResult == eSCALER_EXEC_CODE_PASS)
        {
            UINT8 ucBestPhase = dvC341_BsetPhaseGet((eMCT)ucCH);      //A70LV_Doulas_01136//A70LV_Doulas_0008 modify
        //    LOG_MSG(db_ALWAYS, "Phase Set %d \r\n",ucBestPhase);      //A70LV_Doulas_0215 debug
            halFrontEndCtrl_VGA_Phase_Set(&ucBestPhase);        //A70LV_Doulas_0211 //A70LV_Doulas_0116
            ulExeRet = eHAL_SCALER_EXEC_CODE_PASS;
        }
        else if(eResult == eSCALER_EXEC_CODE_CH_AUTO_PHASE)
        {
            ucCurrentPhase = dvC341_CurrentPhaseGet((eMCT)ucCH);    //A70LV_Doulas_0008 modify
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

        halScaler_SemaphoreGive(__FUNCTION__);

    }
    else
    {
        LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore(%d)\r\n", __FUNCTION__, __LINE__,ucCH);
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }

    return ulExeRet;
#endif
}

eHAL_SCALER_EXEC_CODE halScaler_SetInputSource(UINT8 ucCH,UINT8 ucInputSource)  //A70LV_Doulas_0007
{
    eHAL_SCALER_EXEC_CODE ulExeRet;
    UINT8 ucInputSourceChanged;

    ucInputSourceChanged = 0;
    if(ucCH >= eMCT_LAST)
            return eHAL_SCALER_EXEC_CH_OVER_RANGE;

    if(ucCH == eMCT_CH1)
    {
        if(m_sUserSystemSetting.sSourceSetting.ucInputSourceMain != ucInputSource)
        {
            m_sUserSystemSetting.sSourceSetting.ucInputSourceMain = ucInputSource;
            ucInputSourceChanged = 1;
        }
    }
    else
    {
        if(m_sUserSystemSetting.sSourceSetting.ucInputSourceSub != ucInputSource)
        {
            m_sUserSystemSetting.sSourceSetting.ucInputSourceSub = ucInputSource;
            ucInputSourceChanged = 1;
        }
    }

    if(ucInputSourceChanged)
    {
        #if 0
        #ifdef CUSTOM_CHRISTIE
        UINT8 ucData = eHDMI_OUT_MAX_NUMBER;

        if(m_sUserSystemSetting.sSourceSetting.ucInputSourceMain == eCM_SOURCE_HDMI1)
        {
            ucData = eHDMI_OUT_HDMI1;
        }
        else if(m_sUserSystemSetting.sSourceSetting.ucInputSourceMain == eCM_SOURCE_HDMI2)
        {
            ucData = eHDMI_OUT_HDMI2;
        }
        else if(halScaler_PIP_PBP_Enable_Get())
        {
            if(m_sUserSystemSetting.sSourceSetting.ucInputSourceSub == eCM_SOURCE_HDMI1)
            {
                ucData = eHDMI_OUT_HDMI1;
            }
            else if(m_sUserSystemSetting.sSourceSetting.ucInputSourceSub == eCM_SOURCE_HDMI2)
            {
                ucData = eHDMI_OUT_HDMI2;
            }
        }
        halFrontEndCtrl_HDMI_OUT_Set(&ucData);
        #endif
        #endif
        dvC341_InputSourceSet((eMCT)ucCH,ucInputSource);    //A70LV_Doulas_0008 modify
        //halScaler_IintChannelSetting(eMCT_CH1); //ZU860_Doulas_0082
        //halScaler_IintChannelSetting(eMCT_CH2); //ZU860_Doulas_0082
        halScaler_IintChannelSetting(ucCH);
    }
    ulExeRet = eHAL_SCALER_EXEC_CODE_PASS;
    return ulExeRet;
}

//data range 0~100 (default:50)
eHAL_SCALER_EXEC_CODE halScaler_RedOffset_Set(UINT8 ucCH,UINT8 ucSetting)  //A70LV_Doulas_0072 modify//A70LV_Doulas_0029 //A70LV_Doulas_0022 modify   //A70LV_Doulas_0011
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

    eResult = halScaler_RedOffset_Value_Set(ucCH,ucSetting);  //A70LV_Doulas_0012 modify
    if(eResult != eHAL_SCALER_EXEC_CODE_PASS)
        return eResult;

    if(halScaler_SemaphoreTake(__FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[0].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(__FUNCTION__);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }

        dvC341_Set_RGB_Offset(eMCT_CH1);      //A70LV_Doulas_0155 //A70LV_Doulas_0072 modify
        dvC341_Set_RGB_Offset(eMCT_CH2);      //A70LV_Doulas_0155

        halScaler_SemaphoreGive(__FUNCTION__);
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

    if(halScaler_SemaphoreTake(__FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[0].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(__FUNCTION__);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }

        dvC341_Set_RGB_Offset(eMCT_CH1);      //A70LV_Doulas_0155 //A70LV_Doulas_0072 modify
        dvC341_Set_RGB_Offset(eMCT_CH2);      //A70LV_Doulas_0155

        halScaler_SemaphoreGive(__FUNCTION__);
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

    if(halScaler_SemaphoreTake(__FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[0].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(__FUNCTION__);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }

        dvC341_Set_RGB_Offset(eMCT_CH1);      //A70LV_Doulas_0155 //A70LV_Doulas_0072 modify
        dvC341_Set_RGB_Offset(eMCT_CH2);      //A70LV_Doulas_0155

        halScaler_SemaphoreGive(__FUNCTION__);
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

    if(halScaler_SemaphoreTake(__FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[0].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(__FUNCTION__);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }

        dvC341_Set_RGB_Gain(eMCT_CH1);    //A70LV_Doulas_0155 //A70LV_Doulas_0072 modify
        dvC341_Set_RGB_Gain(eMCT_CH2);    //A70LV_Doulas_0155

        halScaler_SemaphoreGive(__FUNCTION__);
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

    if(halScaler_SemaphoreTake(__FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[0].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(__FUNCTION__);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }

        dvC341_Set_RGB_Gain(eMCT_CH1);    //A70LV_Doulas_0155 //A70LV_Doulas_0072 modify
        dvC341_Set_RGB_Gain(eMCT_CH2);    //A70LV_Doulas_0155

        halScaler_SemaphoreGive(__FUNCTION__);
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

    if(halScaler_SemaphoreTake(__FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[0].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(__FUNCTION__);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }

        dvC341_Set_RGB_Gain(eMCT_CH1);    //A70LV_Doulas_0155 //A70LV_Doulas_0072 modify
        dvC341_Set_RGB_Gain(eMCT_CH2);    //A70LV_Doulas_0155

        halScaler_SemaphoreGive(__FUNCTION__);
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
    INT16 iC341_setting;
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    UINT8  ucInputSource = halScaler_InputSource_Get(ucCH);
    UINT8  ucPresetMode = halScaler_PictureSettings_Get(ucCH);
    UINT8  ucSignalType = halScaler_SignalType_Get(ucCH);
    UINT8  ucPresetUserMode = halScaler_PictureSettings_PreUser_Get(ucCH);

    if (ucSetting>RED_OFFSET_MAX_VALUE) // || ucSetting<RED_OFFSET_MIN_VALUE )
        return eHAL_SCALER_EXEC_CH_OVER_RANGE;

    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): RedOffset=%d\r\n", __FUNCTION__, __LINE__, ucSetting);

    if (eCM_PICTURE_SETTINGS_USER != ucPresetMode) //G100_Steven_0034
    	m_sUserSystemSetting.sColorSetting[ucInputSource][ucPresetMode][ucSignalType].ucRedOffset = ucSetting;
    else
    	m_sUserSystemSetting.sUserColorSetting[ucInputSource][ucPresetUserMode][ucSignalType].ucRedOffset = ucSetting;
    //convert range 0~100(default:50) to -511~511(default:0)
    iC341_setting = (INT16)(((DOUBLE)ucSetting/(DOUBLE)(RED_OFFSET_MAX_VALUE - RED_OFFSET_MIN_VALUE))*(C341_RED_OFFSET_MAX - C341_RED_OFFSET_MIN)) + C341_RED_OFFSET_MIN;

    dvC341_RedOffset_Value_Set(eMCT_CH1,iC341_setting);     //A70LV_Doulas_0155
    dvC341_RedOffset_Value_Set(eMCT_CH2,iC341_setting);     //A70LV_Doulas_0155

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_GreenOffset_Value_Set(UINT8 ucCH,UINT8 ucSetting)  //A70LV_Doulas_0072 modify//A70LV_Doulas_0022 modify  //A70LV_Doulas_0012
{
    INT16 iC341_setting;
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    UINT8  ucInputSource = halScaler_InputSource_Get(ucCH);
    UINT8  ucPresetMode = halScaler_PictureSettings_Get(ucCH);
    UINT8  ucSignalType = halScaler_SignalType_Get(ucCH);
    UINT8  ucPresetUserMode = halScaler_PictureSettings_PreUser_Get(ucCH);

    if ( ucSetting>GREEN_OFFSET_MAX_VALUE)//|| ucSetting<GREEN_OFFSET_MIN_VALUE )
        return eHAL_SCALER_EXEC_CH_OVER_RANGE;

    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): GreenOffset=%d\r\n", __FUNCTION__, __LINE__, ucSetting);

    if (eCM_PICTURE_SETTINGS_USER != ucPresetMode) //G100_Steven_0034
    	m_sUserSystemSetting.sColorSetting[ucInputSource][ucPresetMode][ucSignalType].ucGreenOffset = ucSetting;
    else
    	m_sUserSystemSetting.sUserColorSetting[ucInputSource][ucPresetUserMode][ucSignalType].ucGreenOffset = ucSetting;

    //convert range 0~100(default:50) to -511~511(default:0)
    iC341_setting = (INT16)(((DOUBLE)ucSetting/(DOUBLE)(GREEN_OFFSET_MAX_VALUE - GREEN_OFFSET_MIN_VALUE))*(C341_GREEN_OFFSET_MAX - C341_GREEN_OFFSET_MIN)) + C341_GREEN_OFFSET_MIN;

    dvC341_GreenOffset_Value_Set(eMCT_CH1,iC341_setting);       //A70LV_Doulas_0155
    dvC341_GreenOffset_Value_Set(eMCT_CH2,iC341_setting);       //A70LV_Doulas_0155

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_BlueOffset_Value_Set(UINT8 ucCH,UINT8 ucSetting)  //A70LV_Doulas_0072 modify//A70LV_Doulas_0022 modify  //A70LV_Doulas_0012
{
    INT16 iC341_setting;
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    UINT8  ucInputSource = halScaler_InputSource_Get(ucCH);
    UINT8  ucPresetMode = halScaler_PictureSettings_Get(ucCH);
    UINT8  ucSignalType = halScaler_SignalType_Get(ucCH);
    UINT8  ucPresetUserMode = halScaler_PictureSettings_PreUser_Get(ucCH);

    if ( ucSetting>BLUE_OFFSET_MAX_VALUE)// || ucSetting<BLUE_OFFSET_MIN_VALUE )
        return eHAL_SCALER_EXEC_CH_OVER_RANGE;

    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): BlueOffset=%d\r\n", __FUNCTION__, __LINE__, ucSetting);

    if (eCM_PICTURE_SETTINGS_USER != ucPresetMode) //G100_Steven_0034
    	m_sUserSystemSetting.sColorSetting[ucInputSource][ucPresetMode][ucSignalType].ucBlueOffset = ucSetting;
    else
    	m_sUserSystemSetting.sUserColorSetting[ucInputSource][ucPresetUserMode][ucSignalType].ucBlueOffset = ucSetting;
    //convert range 0~100(default:50) to -511~511(default:0)
    iC341_setting = (INT16)(((DOUBLE)ucSetting/(DOUBLE)(BLUE_OFFSET_MAX_VALUE - BLUE_OFFSET_MIN_VALUE))*(C341_BLUE_OFFSET_MAX - C341_BLUE_OFFSET_MIN)) + C341_BLUE_OFFSET_MIN;

    dvC341_BlueOffset_Value_Set(eMCT_CH1,iC341_setting);        //A70LV_Doulas_0155
    dvC341_BlueOffset_Value_Set(eMCT_CH2,iC341_setting);        //A70LV_Doulas_0155

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_RedGain_Value_Set(UINT8 ucCH,UINT8 ucSetting)  //A70LV_Doulas_0072 modify//A70LV_Doulas_0022 modify     //A70LV_Doulas_0012
{
    DOUBLE dC341_setting;
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    UINT8  ucInputSource = halScaler_InputSource_Get(ucCH);
    UINT8  ucPresetMode = halScaler_PictureSettings_Get(ucCH);
    UINT8  ucSignalType = halScaler_SignalType_Get(ucCH);
    UINT8  ucPresetUserMode = halScaler_PictureSettings_PreUser_Get(ucCH);

    if ( ucSetting>RED_GAIN_MAX_VALUE)// || ucSetting<RED_GAIN_MIN_VALUE )
        return eHAL_SCALER_EXEC_CH_OVER_RANGE;    //A70LV_Doulas_0072 modify

    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): RedGain=%d\r\n", __FUNCTION__, __LINE__, ucSetting);

    if (eCM_PICTURE_SETTINGS_USER != ucPresetMode) //G100_Steven_0034
    	m_sUserSystemSetting.sColorSetting[ucInputSource][ucPresetMode][ucSignalType].ucRedGain = ucSetting;
    else
    	m_sUserSystemSetting.sUserColorSetting[ucInputSource][ucPresetUserMode][ucSignalType].ucRedGain = ucSetting;

    //convert range 0~100(default:50) to 0~3.99(default:1)
    if ( ucSetting<RED_GAIN_DEFAULT_VALUE )
    {
        dC341_setting = (DOUBLE)ucSetting/(DOUBLE)(RED_GAIN_DEFAULT_VALUE-RED_GAIN_MIN_VALUE)*(C341_RED_GAIN_DEFAULT-C341_RED_GAIN_MIN);
    }
    else if (ucSetting>RED_GAIN_DEFAULT_VALUE)
    {
        dC341_setting = ((DOUBLE)ucSetting-RED_GAIN_DEFAULT_VALUE)/(DOUBLE)(RED_GAIN_MAX_VALUE-RED_GAIN_DEFAULT_VALUE)*(C341_RED_GAIN_MAX-C341_RED_GAIN_DEFAULT) + C341_RED_GAIN_DEFAULT;
    }
    else
    {
        dC341_setting = C341_RED_GAIN_DEFAULT;
    }

    dvC341_RedGain_Value_Set(eMCT_CH1,dC341_setting);       //A70LV_Doulas_0155
    dvC341_RedGain_Value_Set(eMCT_CH2,dC341_setting);       //A70LV_Doulas_0155
    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_GreenGain_Value_Set(UINT8 ucCH,UINT8 ucSetting)  //A70LV_Doulas_0072 modify//A70LV_Doulas_0022 modify     //A70LV_Doulas_0012
{
    DOUBLE dC341_setting;
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    UINT8  ucInputSource = halScaler_InputSource_Get(ucCH);
    UINT8  ucPresetMode = halScaler_PictureSettings_Get(ucCH);
    UINT8  ucSignalType = halScaler_SignalType_Get(ucCH);
    UINT8  ucPresetUserMode = halScaler_PictureSettings_PreUser_Get(ucCH);

    if ( ucSetting>GREEN_GAIN_MAX_VALUE)// || ucSetting<GREEN_GAIN_MIN_VALUE )
        return eHAL_SCALER_EXEC_CH_OVER_RANGE;    //A70LV_Doulas_0072 modify

    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): GreenGain=%d\r\n", __FUNCTION__, __LINE__, ucSetting);

    if (eCM_PICTURE_SETTINGS_USER != ucPresetMode) //G100_Steven_0034
    	m_sUserSystemSetting.sColorSetting[ucInputSource][ucPresetMode][ucSignalType].ucGreenGain = ucSetting;
    else
    	m_sUserSystemSetting.sUserColorSetting[ucInputSource][ucPresetUserMode][ucSignalType].ucGreenGain = ucSetting;

    //convert range 0~100(default:50) to 0~3.99(default:1)
    if ( ucSetting<GREEN_GAIN_DEFAULT_VALUE )
    {
        dC341_setting = (DOUBLE)ucSetting/(DOUBLE)(GREEN_GAIN_DEFAULT_VALUE-GREEN_GAIN_MIN_VALUE)*(C341_GREEN_GAIN_DEFAULT-C341_GREEN_GAIN_MIN);
    }
    else if (ucSetting>GREEN_GAIN_DEFAULT_VALUE)
    {
        dC341_setting = ((DOUBLE)ucSetting-GREEN_GAIN_DEFAULT_VALUE)/(DOUBLE)(GREEN_GAIN_MAX_VALUE-GREEN_GAIN_DEFAULT_VALUE)*(C341_GREEN_GAIN_MAX-C341_GREEN_GAIN_DEFAULT) + C341_GREEN_GAIN_DEFAULT;
    }
    else
    {
        dC341_setting = C341_GREEN_GAIN_DEFAULT;
    }

    dvC341_GreenGain_Value_Set(eMCT_CH1,dC341_setting);     //A70LV_Doulas_0155
    dvC341_GreenGain_Value_Set(eMCT_CH2,dC341_setting);     //A70LV_Doulas_0155

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_BlueGain_Value_Set(UINT8 ucCH,UINT8 ucSetting)  //A70LV_Doulas_0072 modify//A70LV_Doulas_0022 modify     //A70LV_Doulas_0012
{
    DOUBLE dC341_setting;
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    UINT8  ucInputSource = halScaler_InputSource_Get(ucCH);
    UINT8  ucPresetMode = halScaler_PictureSettings_Get(ucCH);
    UINT8  ucSignalType = halScaler_SignalType_Get(ucCH);
    UINT8  ucPresetUserMode = halScaler_PictureSettings_PreUser_Get(ucCH);

    if ( ucSetting>BLUE_GAIN_MAX_VALUE)// || ucSetting<BLUE_GAIN_MIN_VALUE )
        return eHAL_SCALER_EXEC_CH_OVER_RANGE;    //A70LV_Doulas_0072 modify

    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): BlueGain=%d\r\n", __FUNCTION__, __LINE__, ucSetting);

    if (eCM_PICTURE_SETTINGS_USER != ucPresetMode) //G100_Steven_0034
    	m_sUserSystemSetting.sColorSetting[ucInputSource][ucPresetMode][ucSignalType].ucBlueGain = ucSetting;
    else
    	m_sUserSystemSetting.sUserColorSetting[ucInputSource][ucPresetUserMode][ucSignalType].ucBlueGain = ucSetting;

    //convert range 0~100(default:50) to 0~3.99(default:1)
    if ( ucSetting<BLUE_GAIN_DEFAULT_VALUE )
    {
        dC341_setting = (DOUBLE)ucSetting/(DOUBLE)(BLUE_GAIN_DEFAULT_VALUE-BLUE_GAIN_MIN_VALUE)*(C341_BLUE_GAIN_DEFAULT-C341_BLUE_GAIN_MIN);
    }
    else if (ucSetting>BLUE_GAIN_DEFAULT_VALUE)
    {
        dC341_setting = ((DOUBLE)ucSetting-BLUE_GAIN_DEFAULT_VALUE)/(DOUBLE)(BLUE_GAIN_MAX_VALUE-BLUE_GAIN_DEFAULT_VALUE)*(C341_BLUE_GAIN_MAX-C341_BLUE_GAIN_DEFAULT) + C341_BLUE_GAIN_DEFAULT;
    }
    else
    {
        dC341_setting = C341_BLUE_GAIN_DEFAULT;
    }
    dvC341_BlueGain_Value_Set(eMCT_CH1,dC341_setting);      //A70LV_Doulas_0155
    dvC341_BlueGain_Value_Set(eMCT_CH2,dC341_setting);      //A70LV_Doulas_0155

    return eResult;
}


eHAL_SCALER_EXEC_CODE halScaler_Brightness_Value_Set(UINT8 ucCH,UINT8 ucSetting)  //A70LV_Doulas_0072 modify//A70LV_Doulas_0022 modify  //A70LV_Doulas_0012
{
    INT16 iC341_setting;
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    UINT8  ucInputSource = halScaler_InputSource_Get(ucCH);
    UINT8  ucPresetMode = halScaler_PictureSettings_Get(ucCH); 				//G100_Steven_0079 //A35G2_CDS_Coda_0024
    UINT8  ucSignalType = halScaler_SignalType_Get(ucCH);
    UINT8  ucPresetUserMode = halScaler_PictureSettings_PreUser_Get(ucCH);  //G100_Steven_0079

    if ( ucSetting>BRIGHTNESS_MAX_VALUE)// || ucSetting<BRIGHTNESS_MIN_VALUE )
        return eHAL_SCALER_EXEC_CH_OVER_RANGE;

    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): Brightness=%d\r\n", __FUNCTION__, __LINE__, ucSetting);

    if (eCM_PICTURE_SETTINGS_USER != ucPresetMode) //G100_Steven_0034
    	m_sUserSystemSetting.sColorSetting[ucInputSource][ucPresetMode][ucSignalType].ucBrightness = ucSetting;
    else
    	m_sUserSystemSetting.sUserColorSetting[ucInputSource][ucPresetUserMode][ucSignalType].ucBrightness = ucSetting;

    if(ucInputSource == eCM_SOURCE_VGA) //G100_Owen_0075 : for PL Reid request
    {
        if(ucSetting)
            ucSetting--;
        LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): Brightness=%d\r\n", __FUNCTION__, __LINE__, ucSetting);
    }
    //convert range 0~100(default:50) to -512~512(default:0)
    iC341_setting = (INT16)(((DOUBLE)ucSetting/(DOUBLE)(BRIGHTNESS_MAX_VALUE - BRIGHTNESS_MIN_VALUE))*(C341_BRIGHTNESS_MAX - C341_BRIGHTNESS_MIN)) + C341_BRIGHTNESS_MIN;
    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): c821 Brightness=%d\r\n", __FUNCTION__, __LINE__, iC341_setting);

    if(halScaler_PIP_PBP_Enable_Get()) //G100_Steven_0079 start
    {
    	//LOG_MSG(db_ALWAYS, "Brightness111_Set CH %d, Val %d\r\n",ucCH, iC341_setting);
        dvC341_Brightness_Value_Set(ucCH,iC341_setting);
    }
    else
    {
    	if(ucCH == eMCT_CH1)
    	{
    		dvC341_Brightness_Value_Set(eMCT_CH1,iC341_setting);
    		dvC341_Brightness_Value_Set(eMCT_CH2,iC341_setting);
    	}
    } //G100_Steven_0079 end

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_Contrast_Value_Set(UINT8 ucCH,UINT8 ucSetting)     //A70LV_Doulas_0072 modify//A70LV_Doulas_0022
{
    DOUBLE dC341_setting;
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    UINT8  ucInputSource = halScaler_InputSource_Get(ucCH);
    UINT8  ucPresetMode = halScaler_PictureSettings_Get(ucCH); 				//G100_Steven_0079  //A35G2_CDS_Coda_0024
    UINT8  ucSignalType = halScaler_SignalType_Get(ucCH);
    UINT8  ucPresetUserMode = halScaler_PictureSettings_PreUser_Get(ucCH);  //G100_Steven_0079


    if ( ucSetting>CONTRAST_MAX_VALUE)// || ucSetting<CONTRAST_MIN_VALUE )
        return eHAL_SCALER_EXEC_CH_OVER_RANGE;    //A70LV_Doulas_0072 modify

    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): Contrast=%d\r\n", __FUNCTION__, __LINE__, ucSetting);

    if (eCM_PICTURE_SETTINGS_USER != ucPresetMode) //G100_Steven_0034
    	m_sUserSystemSetting.sColorSetting[ucInputSource][ucPresetMode][ucSignalType].ucContrast = ucSetting;
    else
    	m_sUserSystemSetting.sUserColorSetting[ucInputSource][ucPresetUserMode][ucSignalType].ucContrast = ucSetting;

    //convert range 0~100(default:50) to 0~3.99(default:1)
    if ( ucSetting<CONTRAST_DEFAULT_VALUE )
    {
        dC341_setting = (DOUBLE)ucSetting/(DOUBLE)(CONTRAST_DEFAULT_VALUE-CONTRAST_MIN_VALUE)*(C341_CONTRAST_DEFAULT-C341_CONTRAST_MIN);
    }
    else if (ucSetting>CONTRAST_DEFAULT_VALUE)
    {
        dC341_setting = ((DOUBLE)ucSetting-CONTRAST_DEFAULT_VALUE)/(DOUBLE)(CONTRAST_MAX_VALUE-CONTRAST_DEFAULT_VALUE)*(C341_CONTRAST_MAX-C341_CONTRAST_DEFAULT) + C341_CONTRAST_DEFAULT;
    }
    else
    {
        dC341_setting = C341_CONTRAST_DEFAULT;
    }

    if(halScaler_PIP_PBP_Enable_Get())  //G100_Steven_0079 start //A70LV_Doulas_0389 Modify
    {
        dvC341_Contrast_Value_Set(ucCH,dC341_setting);
    }
    else
    {
    	if(ucCH == eMCT_CH1)
    	{
    		dvC341_Contrast_Value_Set(eMCT_CH1,dC341_setting);      //A70LV_Doulas_0155
    		dvC341_Contrast_Value_Set(eMCT_CH2,dC341_setting);      //A70LV_Doulas_0155
    	}
    } //G100_Steven_0079 end

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_HUE_Value_Set(UINT8 ucCH,UINT8 ucSetting)  //A70LV_Doulas_0072 modify//A70LV_Doulas_0022
{
    INT16 iC341_setting;
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    UINT8  ucInputSource = halScaler_InputSource_Get(ucCH);
    UINT8  ucPresetMode = halScaler_PictureSettings_Get(ucCH);
    UINT8  ucSignalType = halScaler_SignalType_Get(ucCH);
    UINT8  ucPresetUserMode = halScaler_PictureSettings_PreUser_Get(ucCH);

    if(ucCH >= eMCT_LAST)
        return eHAL_SCALER_EXEC_CODE_FAIL;

    if ( ucSetting>HUE_MAX_VALUE)// || ucSetting<HUE_MIN_VALUE )
        return eHAL_SCALER_EXEC_CH_OVER_RANGE;

    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): HUE=%d\r\n", __FUNCTION__, __LINE__, ucSetting);

    if (eCM_PICTURE_SETTINGS_USER != ucPresetMode) //G100_Steven_0034
    	m_sUserSystemSetting.sColorSetting[ucInputSource][ucPresetMode][ucSignalType].ucTint = ucSetting;
    else
    	m_sUserSystemSetting.sUserColorSetting[ucInputSource][ucPresetUserMode][ucSignalType].ucTint = ucSetting;

    //convert range 0~100(default:50) to -180~180(default:0)
    iC341_setting = (INT16)(((DOUBLE)ucSetting/(DOUBLE)(HUE_MAX_VALUE - HUE_MIN_VALUE))*(C341_HUE_MAX - C341_HUE_MIN)) + C341_HUE_MIN;
    dvC341_Set_HUE_Value(eMCT_CH1,iC341_setting);     //A70LV_Doulas_0155
    dvC341_Set_HUE_Value(eMCT_CH2,iC341_setting);     //A70LV_Doulas_0155

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_Saturation_Value_Set(UINT8 ucCH,UINT8 ucSetting)  //A70LV_Doulas_0072 modify//A70LV_Doulas_0022
{
    DOUBLE dC341_setting;
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    UINT8  ucInputSource = halScaler_InputSource_Get(ucCH);
    UINT8  ucPresetMode = halScaler_PictureSettings_Get(ucCH);
    UINT8  ucSignalType = halScaler_SignalType_Get(ucCH);
    UINT8  ucPresetUserMode = halScaler_PictureSettings_PreUser_Get(ucCH);

    if(ucCH >= eMCT_LAST)
            return eHAL_SCALER_EXEC_CODE_FAIL;

    if ( ucSetting> SATURATION_MAX_VALUE)// || ucSetting< SATURATION_MIN_VALUE )
        return eHAL_SCALER_EXEC_CH_OVER_RANGE;    //A70LV_Doulas_0072 modify

    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): Saturation=%d\r\n", __FUNCTION__, __LINE__, ucSetting);

    if (eCM_PICTURE_SETTINGS_USER != ucPresetMode) //G100_Steven_0034
    	m_sUserSystemSetting.sColorSetting[ucInputSource][ucPresetMode][ucSignalType].ucSaturation = ucSetting;
    else
    	m_sUserSystemSetting.sUserColorSetting[ucInputSource][ucPresetUserMode][ucSignalType].ucSaturation = ucSetting;

    //convert range 0~100(default:50) to 0~3.99(default:1)
    if ( ucSetting< SATURATION_DEFAULT_VALUE )
    {
        dC341_setting = (DOUBLE)ucSetting/(DOUBLE)(SATURATION_DEFAULT_VALUE- SATURATION_MIN_VALUE)*(C341_SATURATION_DEFAULT-C341_SATURATION_MIN);
    }
    else if (ucSetting> SATURATION_DEFAULT_VALUE)
    {
        dC341_setting = ((DOUBLE)ucSetting- SATURATION_DEFAULT_VALUE)/(DOUBLE)(SATURATION_MAX_VALUE- SATURATION_DEFAULT_VALUE)*(C341_SATURATION_MAX- C341_SATURATION_DEFAULT) + C341_SATURATION_DEFAULT;
    }
    else
    {
        dC341_setting = C341_SATURATION_DEFAULT;
    }
    dvC341_Set_Saturation_Value(eMCT_CH1,dC341_setting);      //A70LV_Doulas_0155
    dvC341_Set_Saturation_Value(eMCT_CH2,dC341_setting);      //A70LV_Doulas_0155

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

    memcpy(&m_sUserSystemSetting, &m_UserSetting, sizeof(sUSER_SYSTEM_SETTING));    //A70LV_Doulas_0015 modify

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
    eSCALER_EXEC_CODE eC821Resut;
    UINT8 ucInputSource;

    if ( ucSetting >= eCM_SCALING_MODE_NUMBER)// || ucSetting < eSCALING_MODE_AUTO )
        return eHAL_SCALER_EXEC_CH_OVER_RANGE;    //A70LV_Doulas_0072 modify

    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): OverScan=%d\r\n", __FUNCTION__, __LINE__, ucSetting);

    ucInputSource = halScaler_InputSource_Get(eMCT_CH1);
    m_sUserSystemSetting.sSourceDependSetting[ucInputSource].ucAspectRatio = ucSetting;

    dvC341_Set_SizePresets_Value(eMCT_CH2,ucSetting);      //A70LV_Doulas_0155 Add
    eC821Resut = dvC341_Set_SizePresets_Value(eMCT_CH1,ucSetting);
    if(eC821Resut != eSCALER_EXEC_CODE_PASS)
    {
        eResult = eHAL_SCALER_EXEC_CODE_FAIL;
    }

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_Overscan_Set(UINT8 ucSetting)    //A70LV_Doulas_0072 modify
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    eSCALER_EXEC_CODE eC821Resut;
    UINT8 ucInputSource;

    if ( ucSetting > eOVER_SCAN_CROP)// || ucSetting < eOVER_SCAN_OFF )
        return eHAL_SCALER_EXEC_CH_OVER_RANGE;    //A70LV_Doulas_0072 modify

    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): OverScan=%d\r\n", __FUNCTION__, __LINE__, ucSetting);

    ucInputSource = halScaler_InputSource_Get(eMCT_CH1);
    m_sUserSystemSetting.sSourceDependSetting[ucInputSource].ucOverScan = ucSetting;

    dvC341_Overscan_Set(eMCT_CH2,(eOVER_SCAN_TYPE)ucSetting);      //A70LV_Doulas_0155
    eC821Resut = dvC341_Overscan_Set(eMCT_CH1,(eOVER_SCAN_TYPE)ucSetting);
    if(eC821Resut != eSCALER_EXEC_CODE_PASS)
    {
        eResult = eHAL_SCALER_EXEC_CODE_FAIL;
    }

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
#if 0
    UINT8 ucPhase;
    UINT8 ucSetting = halScaler_OSD_PixelPhase_Get();

    ucPhase = (UINT8)(((UINT16)ucSetting - PHASE_DEFAULT_VALUE + (UINT16)dvC341_BsetPhaseGet(eMCT_CH1) + 64)%32);

    halFrontEndCtrl_VGA_Phase_Set(&ucPhase);
#endif
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
    eSCALER_EXEC_CODE eC821Resut;
    UINT8 ucInputSource;

    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): Horz_Start_Position=%d\r\n", __FUNCTION__, __LINE__, ucSetting);

    ucInputSource = halScaler_InputSource_Get(eMCT_CH1);
    m_sUserSystemSetting.sSourceDependSetting[ucInputSource].ucHorzPosition = ucSetting;     //A70LV_Doulas_0029

    dvC341_Horz_Start_Position_Set(eMCT_CH2,ucSetting);     //A70LV_Doulas_0155
    eC821Resut = dvC341_Horz_Start_Position_Set(eMCT_CH1,ucSetting);
    if(eC821Resut != eSCALER_EXEC_CODE_PASS)
    {
        eResult = eHAL_SCALER_EXEC_CODE_FAIL;
    }

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_VertPosition_Set(UINT8 ucSetting)    //A70LV_Doulas_0072 modify
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    eSCALER_EXEC_CODE eC821Resut;
    UINT8 ucInputSource;

    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): Vert_Start_Position=%d\r\n", __FUNCTION__, __LINE__, ucSetting);

    ucInputSource = halScaler_InputSource_Get(eMCT_CH1);
    m_sUserSystemSetting.sSourceDependSetting[ucInputSource].ucVertPosition = ucSetting; //A70LV_Doulas_0029

    dvC341_Vert_Start_Position_Set(eMCT_CH2,ucSetting);     //A70LV_Doulas_0155
    eC821Resut = dvC341_Vert_Start_Position_Set(eMCT_CH1,ucSetting);
    if(eC821Resut != eSCALER_EXEC_CODE_PASS)
    {
        eResult = eHAL_SCALER_EXEC_CODE_FAIL;
    }

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_DigitalHorzZoom_Set(UINT16 uiSetting)    //A70LV_Doulas_0072 modify
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    eSCALER_EXEC_CODE eC821Resut;
    UINT8 ucInputSource;
    UINT8 ucSizePresets;

    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): Digital_Horz_Zoom=%d\r\n", __FUNCTION__, __LINE__, uiSetting);

    ucInputSource = halScaler_InputSource_Get(eMCT_CH1);
    ucSizePresets = halScaler_SizePresets_Get(eMCT_CH1);
    if(ucSizePresets == eCM_SCALING_MODE_CUSTOM)
        m_sUserSystemSetting.sSourceDependSetting[ucInputSource].uiCustomDigitalHorzZoom = uiSetting;
    else
        m_sUserSystemSetting.sSourceDependSetting[ucInputSource].uiDigitalHorzZoom = uiSetting;

    dvC341_Digital_Horz_Zoom_Set(eMCT_CH2,uiSetting);    //A70LV_Doulas_0155
    eC821Resut = dvC341_Digital_Horz_Zoom_Set(eMCT_CH1,uiSetting);
    if(eC821Resut != eSCALER_EXEC_CODE_PASS)
    {
        eResult = eHAL_SCALER_EXEC_CODE_FAIL;
    }

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_DigitalVertZoom_Set(UINT16 uiSetting)   //A70LV_Doulas_0072 modify
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    eSCALER_EXEC_CODE eC821Resut;
    UINT8 ucInputSource;
    UINT8 ucSizePresets;

    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): Digital_Vert_Zoom=%d\r\n", __FUNCTION__, __LINE__, uiSetting);

    ucInputSource = halScaler_InputSource_Get(eMCT_CH1);
    ucSizePresets = halScaler_SizePresets_Get(eMCT_CH1);
    if(ucSizePresets == eCM_SCALING_MODE_CUSTOM)
        m_sUserSystemSetting.sSourceDependSetting[ucInputSource].uiCustomDigitalVertZoom = uiSetting;
    else
        m_sUserSystemSetting.sSourceDependSetting[ucInputSource].uiDigitalVertZoom = uiSetting;

    dvC341_Digital_Vert_Zoom_Set(eMCT_CH2,uiSetting);    //A70LV_Doulas_0155
    eC821Resut = dvC341_Digital_Vert_Zoom_Set(eMCT_CH1,uiSetting);
    if(eC821Resut != eSCALER_EXEC_CODE_PASS)
    {
        eResult = eHAL_SCALER_EXEC_CODE_FAIL;
    }

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_DigitalHorzShift_Set(UINT8 ucSetting)    //A70LV_Doulas_0072 modify
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    eSCALER_EXEC_CODE eC821Resut;
    UINT8 ucInputSource;
    UINT8 ucSizePresets;

    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): Digital_Horz_Shift=%d\r\n", __FUNCTION__, __LINE__, ucSetting);

    ucInputSource = halScaler_InputSource_Get(eMCT_CH1);
    ucSizePresets = halScaler_SizePresets_Get(eMCT_CH1);
    if(ucSizePresets == eCM_SCALING_MODE_CUSTOM)
        m_sUserSystemSetting.sSourceDependSetting[ucInputSource].uiCustomDigitalHorzShift = ucSetting;
    else
        m_sUserSystemSetting.sSourceDependSetting[ucInputSource].uiDigitalHorzShift = ucSetting;

    dvC341_Digital_Horz_Shift_Set(eMCT_CH2,ucSetting);    //A70LV_Doulas_0155
    eC821Resut = dvC341_Digital_Horz_Shift_Set(eMCT_CH1,ucSetting);
    if(eC821Resut != eSCALER_EXEC_CODE_PASS)
    {
        eResult = eHAL_SCALER_EXEC_CODE_FAIL;
    }

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_DigitalVertShift_Set(UINT8 ucSetting)    //A70LV_Doulas_0072 modify
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    eSCALER_EXEC_CODE eC821Resut;
    UINT8 ucInputSource;
    UINT8 ucSizePresets;

    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): Digital_Vert_Shift=%d\r\n", __FUNCTION__, __LINE__, ucSetting);

    ucInputSource = halScaler_InputSource_Get(eMCT_CH1);
    ucSizePresets = halScaler_SizePresets_Get(eMCT_CH1);
    if(ucSizePresets == eCM_SCALING_MODE_CUSTOM)
        m_sUserSystemSetting.sSourceDependSetting[ucInputSource].uiCustomDigitalVertShift = ucSetting;
    else
        m_sUserSystemSetting.sSourceDependSetting[ucInputSource].uiDigitalVertShift = ucSetting;

    dvC341_Digital_Vert_Shift_Set(eMCT_CH2,ucSetting);    //A70LV_Doulas_0155
    eC821Resut = dvC341_Digital_Vert_Shift_Set(eMCT_CH1,ucSetting);
    if(eC821Resut != eSCALER_EXEC_CODE_PASS)
    {
        eResult = eHAL_SCALER_EXEC_CODE_FAIL;
    }

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_AutoImage_Set(UINT8 ucSetting)   //A70LV_Doulas_0112 modify
{
    UINT8 ucInputSource = halScaler_InputSource_Get(eMCT_CH1);
    m_sUserSystemSetting.sSourceDependSetting[ucInputSource].ucAutoImage = ucSetting;
    dvC341_AutoImage_Set(ucInputSource,ucSetting);

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
//G100_Steven_0016
UINT8 halScaler_PictureSettings_PreUser_Get(UINT8 ucCH)
{
    UINT8 ucInputSource = halScaler_InputSource_Get(ucCH);

    return  m_sUserSystemSetting.sSourceDependSetting[ucInputSource].ucPreUserMode;
}
 //G100_Steven_0018
void halScaler_PreUserMode_Set(UINT8 ucSetting)   //A70LV_Doulas_0072 modify//A70LV_Doulas_0069 modify
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    UINT8  ucInputSource = halScaler_InputSource_Get(eMCT_CH1);     //ZU860_Doulas_0082 modify
    UINT8  ucPresetMode = halScaler_PictureSettings_Get(eMCT_CH1);  //ZU860_Doulas_0082 modify
    UINT8  ucSignalType = halScaler_SignalType_Get(eMCT_CH1);       //ZU860_Doulas_0082 modify

    if ( ucSetting >= eCM_PICTURE_SETTINGS_NUMBER )// || ucSetting < eCOLOR_SPACE_AUTO )
    {}
    else
    {
    	m_sUserSystemSetting.sSourceDependSetting[ucInputSource].ucPreUserMode = ucSetting;
    }

}

UINT8 halScaler_SignalType_Get(UINT8 ucCH)
{
    UINT8 ucSignalType = 0;

    return ucSignalType;
}

eHAL_SCALER_EXEC_CODE halScaler_ColorSpace_Set(UINT8 ucCH,UINT8 ucSetting)   //A70LV_Doulas_0072 modify//A70LV_Doulas_0069 modify
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    UINT8  ucInputSource = halScaler_InputSource_Get(eMCT_CH1);     //ZU860_Doulas_0082 modify
    UINT8  ucPresetMode = halScaler_PictureSettings_Get(ucCH);  //ZU860_Doulas_0082 modify  //A35G2_CDS_Coda_0024
    UINT8  ucSignalType = halScaler_SignalType_Get(ucCH);       //ZU860_Doulas_0082 modify
    UINT8  ucPresetUserMode = halScaler_PictureSettings_PreUser_Get(ucCH);
    UINT8  ucInputSub = halScaler_InputSource_Get(eMCT_CH2); 	//G100_Steven_0079

    if ( ucSetting >= eCM_COLOR_SPACE_NUMBER )// || ucSetting < eCOLOR_SPACE_AUTO )
        return eHAL_SCALER_EXEC_CH_OVER_RANGE;

    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): ColorSpace Set=%d\r\n", __FUNCTION__, __LINE__, ucSetting);

    //G100_Steven_0079 start
    if((m_sUserSystemSetting.sSourceSetting.ucPIPEnable) && (ucCH == eMCT_CH2))  //H30K_Doulas_0059
    {
    	if (eCM_PICTURE_SETTINGS_USER != ucPresetMode) //G100_Steven_0034
    		m_sUserSystemSetting.sColorSetting[ucInputSub][ucPresetMode][ucSignalType].ucCS = ucSetting;
    	else
    		m_sUserSystemSetting.sUserColorSetting[ucInputSub][ucPresetUserMode][ucSignalType].ucCS = ucSetting;
    }
    else
    {
    	if (eCM_PICTURE_SETTINGS_USER != ucPresetMode) //G100_Steven_0034
    		m_sUserSystemSetting.sColorSetting[ucInputSource][ucPresetMode][ucSignalType].ucCS = ucSetting;
    	else
    		m_sUserSystemSetting.sUserColorSetting[ucInputSource][ucPresetUserMode][ucSignalType].ucCS = ucSetting;
    }//G100_Steven_0079 end

    ucSetting = halScaler_ColorSpace_ScalerValueGet(ucCH);  //A70LV_Doulas_0076 modify

#if 0 //G100_Steven_0079
    if((m_sUserSystemSetting.sSourceSetting.ucPIPEnable == ets_ON) && (ucCH == eMCT_CH2)) //G100_Larry_0020
    {
        dvC341_VideoYUV_Set((eMCT)ucCH,ucFrontEndVideoYUV[ucCH]);
        ucSetting = ucFrontEndColorSpace[ucCH];
    }
#endif

    dvC341_ColorSpace_Value_Set((eMCT)ucCH,ucSetting);

    return eResult;
}

UINT8 halScaler_ColorSpace_Get(UINT8 ucCH)   //A70LV_Doulas_0069
{
    //UINT8  ucInputSource = halScaler_InputSource_Get(eMCT_CH1);     //ZU860_Doulas_0082 modify
    //UINT8  ucPresetMode = halScaler_PictureSettings_Get(eMCT_CH1);  //ZU860_Doulas_0082 modify
    //UINT8  ucSignalType = halScaler_SignalType_Get(eMCT_CH1);       //ZU860_Doulas_0082 modify
    UINT8  ucSetting;

    //LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): ColorSpace Get=%d\r\n", __FUNCTION__, __LINE__, ucSetting); //A70LV_Doulas_0241 remove

    //ucSetting = m_sUserSystemSetting.sColorSetting[ucInputSource][ucPresetMode][ucSignalType].ucCS;

    ucSetting = halScaler_ColorSetting_Get(eHAL_COLOR_SETTING_CS, ucCH); //G100_Steven_0016

    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): ColorSpace Get=%d\r\n", __FUNCTION__, __LINE__, ucSetting);

    return ucSetting;
}

UINT8 halScaler_ColorSpace_ScalerValueGet(UINT8 ucCH)   //A70LV_Doulas_0220 modify//A70LV_Doulas_0109 modify //A70LV_Doulas_0069
{
    UINT8  ucSetting = halScaler_ColorSetting_Get(eHAL_COLOR_SETTING_CS, eMCT_CH1);//G100_Larry_0020

    if((m_sUserSystemSetting.sSourceSetting.ucPIPEnable) && (ucCH == eMCT_CH2)) //H30K_Doulas_0059//G100_Larry_0020
    {
        ucSetting = halScaler_ColorSetting_Get(eHAL_COLOR_SETTING_CS, eMCT_CH2); //eCM_COLOR_SPACE_AUTO; //G100_Larry_0020
    }

    switch(ucSetting)
    {
        case eCM_COLOR_SPACE_AUTO:
            dvC341_VideoYUV_Set((eMCT)ucCH,ucFrontEndVideoYUV[ucCH]);
            ucSetting = ucFrontEndColorSpace[ucCH];     //A70LV_Doulas_0076 modify
            break;

        case eCM_COLOR_SPACE_RGB_FULL:
            dvC341_VideoYUV_Set((eMCT)ucCH,eVIDEO_YUV_UNKNOW);
            ucSetting = eCOLOR_FORMAT_RGB;
            break;

        case eCM_COLOR_SPACE_RGB_LIMITED:
            dvC341_VideoYUV_Set((eMCT)ucCH,eVIDEO_YUV_UNKNOW);
            ucSetting = eCOLOR_FORMAT_RGB_LIMIT;
            break;

        case eCM_COLOR_SPACE_YUV_REC709:
            dvC341_VideoYUV_Set((eMCT)ucCH,eVIDEO_YUV_REC709);
            if(ucFrontEndColorSpace[ucCH] == eCOLOR_FORMAT_422)     //A70LV_Doulas_0076 modify
                ucSetting = eCOLOR_FORMAT_422;
            else
                ucSetting = eCOLOR_FORMAT_444;
            break;

        case eCM_COLOR_SPACE_YUV_REC601:
            dvC341_VideoYUV_Set((eMCT)ucCH,eVIDEO_YUV_REC601);
            if(ucFrontEndColorSpace[ucCH] == eCOLOR_FORMAT_422)     //A70LV_Doulas_0076 modify
                ucSetting = eCOLOR_FORMAT_422;
            else
                ucSetting = eCOLOR_FORMAT_444;
            break;

        default:
            dvC341_VideoYUV_Set((eMCT)ucCH,ucFrontEndVideoYUV[ucCH]);
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

    if(halScaler_SemaphoreTake(__FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[0].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(__FUNCTION__);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }

        dvC341_Set_Sharpness(eMCT_CH1);       //A70LV_Doulas_0155
        dvC341_Set_Sharpness(eMCT_CH2);       //A70LV_Doulas_0155

        halScaler_SemaphoreGive(__FUNCTION__);
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
    dvC341_OSD_3D_Enable_Set(eMCT_CH1,ucSetting);
    dvC341_OSD_3D_Enable_Set(eMCT_CH2,ucSetting);       //A70LV_Doulas_0155

    return eHAL_SCALER_EXEC_CODE_PASS;
}

INT8 halScaler_3DInvert_Set(UINT8 ucSetting)
{
    return HAL_SCALER_PASS;
}

INT8 halScaler_3DSyncOut_Set(UINT8 ucSetting)
{
    return HAL_SCALER_PASS;
}

eHAL_SCALER_EXEC_CODE halScaler_FrameDelay_Set(UINT16 uiSetting)     //A70LV_Doulas_0154 modify
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

    if(halScaler_SemaphoreTake(__FUNCTION__) == TRUE) //H30K_Doulas_0003
    {
        if(!m_sHalScalerInfo[0].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(__FUNCTION__);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }

        dvC341_FrameDelaySet((UINT16)uiSetting*8); //H30K_Doulas_0010

        halScaler_SemaphoreGive(__FUNCTION__);
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

    if(halScaler_SemaphoreTake(__FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[0].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(__FUNCTION__);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }

        dvC341_SkinColor_Set((eMCT)ucCH);

        halScaler_SemaphoreGive(__FUNCTION__);
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

    if(halScaler_SemaphoreTake(__FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[0].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(__FUNCTION__);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }

        dvC341_BuildPicture_4K(eMCT_CH1);
        if(m_sUserSystemSetting.sSourceSetting.ucPIPEnable)     //A70LV_Doulas_0080
        {
            dvC341_BuildPicture_4K(eMCT_CH2);
        }
        else if((dvC341_PanelGet() == ePANEL_ID_1080P_120HZ) ||
                (dvC341_PanelGet() == ePANEL_ID_WUXGA_120HZ))  //A70LV_Doulas_0377 Modify //A70LV_Doulas_0155
        {
            dvC341_BuildPicture_4K(eMCT_CH2);
        }

        halScaler_SemaphoreGive(__FUNCTION__);
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

    if(halScaler_SemaphoreTake(__FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[0].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(__FUNCTION__);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }

        dvC341_BuildPicture_4K(eMCT_CH1);
        if(m_sUserSystemSetting.sSourceSetting.ucPIPEnable)     //A70LV_Doulas_0080
        {
            dvC341_BuildPicture_4K(eMCT_CH2);
        }
        else if((dvC341_PanelGet() == ePANEL_ID_1080P_120HZ) ||
                (dvC341_PanelGet() == ePANEL_ID_WUXGA_120HZ))   //A70LV_Doulas_0377 Modify //A70LV_Doulas_0155
        {
            dvC341_BuildPicture_4K(eMCT_CH2);
        }

        halScaler_SemaphoreGive(__FUNCTION__);
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

    if(halScaler_SemaphoreTake(__FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[0].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(__FUNCTION__);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }

        dvC341_BuildPicture_4K(eMCT_CH1);
        dvC341_BuildPicture_4K(eMCT_CH2);
        halScaler_SemaphoreGive(__FUNCTION__);
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

    if(halScaler_SemaphoreTake(__FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[0].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(__FUNCTION__);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }

        //execute Contrast action function
        halScaler_IintChannelSetting(eMCT_CH1); //ZU860_Doulas_0082
        halScaler_IintChannelSetting(eMCT_CH2); //ZU860_Doulas_0082
        halScaler_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore\r\n", __FUNCTION__, __LINE__);
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }

    return eResult;
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

INT8 halScaler_HDR_Set(UINT8 ucSetting)
{
    return HAL_SCALER_PASS;
}

INT8 halScaler_MEMC_Set(UINT8 ucSetting)
{
    return HAL_SCALER_PASS;
}

eHAL_SCALER_EXEC_CODE halScaler_ImageFreeze_Set(UINT8 ucSetting) //A70LV_Doulas_0072 modify
{
    halScaler_Freeze_Set(eMCT_CH1,(BOOL)ucSetting);     //A70LV_Doulas_0029
    halScaler_Freeze_Set(eMCT_CH2,(BOOL)ucSetting);
    return eHAL_SCALER_EXEC_CODE_PASS;
}

eHAL_SCALER_EXEC_CODE halScaler_Gamma_Set(UINT8 ucCH,UINT8 ucSetting)    //A70LV_Doulas_0072 modify//A70LV_Doulas_0038 modify
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    UINT8  ucInputSource = halScaler_InputSource_Get(ucCH);
    UINT8  ucPresetMode = halScaler_PictureSettings_Get(ucCH);
    UINT8  ucSignalType = halScaler_SignalType_Get(ucCH);
    UINT8  ucPresetUserMode = halScaler_PictureSettings_PreUser_Get(ucCH);

    if ( ucSetting >= eCM_GAMMA_NUMBER )    //ZU860_Doulas_0010 modify
        return eHAL_SCALER_EXEC_CH_OVER_RANGE;

    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): Gamma =%d\r\n", __FUNCTION__, __LINE__, ucSetting);

    if (eCM_PICTURE_SETTINGS_USER != ucPresetMode) //G100_Steven_0034
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
    UINT8  ucPresetUserMode = halScaler_PictureSettings_PreUser_Get(ucCH);

    if ( ucSetting > (WHITE_PEAKING_MAX_VALUE/WHITE_PEAKING_STEP_VALUE))    //ZU860_Doulas_0010 modify
        return eHAL_SCALER_EXEC_CH_OVER_RANGE;

    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): WhitePeaking=%d\r\n", __FUNCTION__, __LINE__, ucSetting);

    if (eCM_PICTURE_SETTINGS_USER != ucPresetMode) //G100_Steven_0034
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
    UINT8  ucPresetUserMode = halScaler_PictureSettings_PreUser_Get(ucCH);

    if ( ucSetting >= eCM_COLOR_TEMPERATURE_NUMBER)   //ZU860_Doulas_0010 modify
        return eHAL_SCALER_EXEC_CH_OVER_RANGE;

    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): ColorTemperature=%d\r\n", __FUNCTION__, __LINE__, ucSetting);

    if (eCM_PICTURE_SETTINGS_USER != ucPresetMode) //G100_Steven_0034
    	m_sUserSystemSetting.sColorSetting[ucInputSource][ucPresetMode][ucSignalType].ucCT = ucSetting;
    else
    	m_sUserSystemSetting.sUserColorSetting[ucInputSource][ucPresetUserMode][ucSignalType].ucCT = ucSetting;

    return eResult;
}

INT8 halScaler_EdgeEnhancement_Set(UINT8 ucSetting)
{
    return HAL_SCALER_PASS;
}

// A70LV_Eric.C_0026 Start
INT8 halScaler_ColorWheelSpeed_Get(UINT8 ucCH)
{
    return m_sUserSystemSetting.sImageSetting.ucCW_Speed;
}
// A70LV_Eric.C_0026 End

eHAL_SCALER_EXEC_CODE halScaler_ColorWheelSpeed_Set(UINT8 ucCH,UINT8 ucSetting)      //A70LV_Doulas_0072 modify//A70LV_Doulas_0037 modify
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

    if ( ucSetting >= eCM_COLOR_WHEEL_SPEED_NUMBER )// || ucSetting < eCOLOR_SPEED_2X )
        return eHAL_SCALER_EXEC_CH_OVER_RANGE;

    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): ColorWheelSpeed=%d\r\n", __FUNCTION__, __LINE__, ucSetting);

    m_sUserSystemSetting.sImageSetting.ucCW_Speed = ucSetting;

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_ColorEnhancement_Set(UINT8 ucCH,UINT8 ucSetting)     //A70LV_Doulas_0072 modify//A70LV_Doulas_0038 modify
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    UINT8  ucInputSource = halScaler_InputSource_Get(ucCH);
    UINT8  ucPresetMode = halScaler_PictureSettings_Get(ucCH);
    UINT8  ucSignalType = halScaler_SignalType_Get(ucCH);
    UINT8  ucPresetUserMode = halScaler_PictureSettings_PreUser_Get(ucCH);

    if(ucSetting >= eEDGE_ENHANCEMENT_MAX_NUMBER)
        return eHAL_SCALER_EXEC_CH_OVER_RANGE;

    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): ColorEnhancement=%d\r\n", __FUNCTION__, __LINE__, ucSetting);

    if (eCM_PICTURE_SETTINGS_USER != ucPresetMode) //G100_Steven_0034
    	m_sUserSystemSetting.sColorSetting[ucInputSource][ucPresetMode][ucSignalType].ucColorEnhancement = ucSetting;
    else
    	m_sUserSystemSetting.sUserColorSetting[ucInputSource][ucPresetUserMode][ucSignalType].ucColorEnhancement = ucSetting;

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_ColorEnhancement_Get(UINT8 ucCH,UINT8 *ucSetting)     //A70LV_Doulas_0072 modify//A70LV_Doulas_0044
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    //UINT8  ucInputSource = halScaler_InputSource_Get(ucCH);
    //UINT8  ucPresetMode = halScaler_PictureSettings_Get(ucCH);
    //UINT8  ucSignalType = halScaler_SignalType_Get(ucCH);

    //*ucSetting = m_sUserSystemSetting.sColorSetting[ucInputSource][ucPresetMode][ucSignalType].ucColorEnhancement;
    *ucSetting = halScaler_ColorSetting_Get(eHAL_COLOR_SETTING_COLORENHANCEMENT, ucCH); //G100_Steven_0016

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

#if 1    //ZU860_Doulas_0017
    if(ucSetting > eCM_SCREEN_MODE_PBP)
        return eHAL_SCALER_EXEC_CH_OVER_RANGE;
#else
    if(ucSetting > ets_ON)// || ucSetting < ets_OFF)
        return eHAL_SCALER_EXEC_CH_OVER_RANGE;
#endif

    m_sUserSystemSetting.sSourceSetting.ucPIPEnable = ucSetting;
    halFrontEndCtrl_PIP_Enable_Set(&ucSetting);     //A70LV_Doulas_0120

    eResult = halScaler_SetOverlay(eMCT_CH1,ucSetting);       //ZU860_Doulas_0017//A70LV_Doulas_0157
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

    #ifdef PLATFORM_H30_4K
    #else
    if(ucCH == eMCT_CH2 && halScaler_PIP_PBP_Enable_Get() == 0)
    {
        return eHAL_SCALER_EXEC_CODE_PASS;
    }
    #endif

    if(ucCH == eMCT_CH1)
    {
        ucMCT_CH = eMCT_CH1;
        ucInputSource = halScaler_InputSource_Get(ucMCT_CH);
        hal_sChannelSetting[eMCT_CH1].eInputSource          = (eCM_SOURCE_ID)halScaler_InputSource_Get((UINT8)eMCT_CH1);       //A70LV_Doulas_0072 modify
        hal_sChannelSetting[eMCT_CH1].eScalingMode          = (eCM_SCALING_MODE_ID)m_sUserSystemSetting.sSourceDependSetting[ucInputSource].ucAspectRatio;        //A70LV_Doulas_0072 modify
        hal_sChannelSetting[eMCT_CH1].eOverScan             = (eOVER_SCAN_TYPE)m_sUserSystemSetting.sSourceDependSetting[ucInputSource].ucOverScan;          //A70LV_Doulas_0072 modify
        hal_sChannelSetting[eMCT_CH1].eColorFormat          = (eCOLOR_FORMAT)halScaler_ColorSpace_ScalerValueGet((UINT8)eMCT_CH1);    //A70LV_Doulas_0072 modify//A70LV_Doulas_0069 modify
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
        hal_sChannelSetting[eMCT_CH1].dRedGain              = halScaler_RedGain_Value_Get(ucMCT_CH);
        hal_sChannelSetting[eMCT_CH1].dGreenGain            = halScaler_GreenGain_Value_Get(ucMCT_CH);
        hal_sChannelSetting[eMCT_CH1].dBlueGain             = halScaler_BlueGain_Value_Get(ucMCT_CH);
        hal_sChannelSetting[eMCT_CH1].cGamma                = ets_OFF;
        hal_sChannelSetting[eMCT_CH1].cTemporal_NR          = m_sUserSystemSetting.sImageSetting.ucTemporalNoiseReduction;
        hal_sChannelSetting[eMCT_CH1].cMPEG_NR              = m_sUserSystemSetting.sImageSetting.ucMPEGNoiseReduction;
        hal_sChannelSetting[eMCT_CH1].cDetectFilm           = m_sUserSystemSetting.sImageSetting.ucFilm;
        hal_sChannelSetting[eMCT_CH1].cPIP_PBP_ON           = !!m_sUserSystemSetting.sSourceSetting.ucPIPEnable;
        hal_sChannelSetting[eMCT_CH1].cMain_Layout          = m_sUserSystemSetting.sSourceSetting.ucMainLayout;
        hal_sChannelSetting[eMCT_CH1].cPIP_Size             = m_sUserSystemSetting.sSourceSetting.ucPIPSize;
        hal_sChannelSetting[eMCT_CH1].cFREEZE               = m_sUserSystemSetting.sImageSetting.ucImageFreeze;
        hal_sChannelSetting[eMCT_CH1].cTestPattern          = eCM_TEST_PATTERN_OFF;          //A70LV_Doulas_0035
        hal_sChannelSetting[eMCT_CH1].eScanMode             = ucFrontEndeScanMode[ucCH];        //A70LV_Doulas_0076
        hal_sChannelSetting[eMCT_CH1].eVideoYUV             = eVIDEO_YUV_UNKNOW;    //A70LV_Doulas_0109
        hal_sChannelSetting[eMCT_CH1].eVGA_SYNC_TYPE        = ucVGA_SYNC_TYPE[ucCH];//eVGA_SYNC_TYPE_SOG;//eVGA_SYNC_TYPE_UNKNOW;    //A70LV_Doulas_0280 modify//A70LV_Doulas_0214 modify //A70LV_Doulas_0112
        hal_sChannelSetting[eMCT_CH1].eOSD_3D_Enable        = m_sUserSystemSetting.sSourceDependSetting[ucInputSource].uc3D_Enable;     //A70LV_Doulas_0154
        hal_sChannelSetting[eMCT_CH1].cEdgeMask             = m_sUserSystemSetting.sSourceDependSetting[ucInputSource].ucEdgeMask;     //ZU860_Doulas_0004
        hal_sChannelSetting[eMCT_CH1].eOSD_3D_Mode          = m_sUserSystemSetting.sSourceDependSetting[ucInputSource].uc3D_Mode;   //ZU860_Doulas_0049
		#ifdef Low_Latency_All
        hal_sChannelSetting[eMCT_CH1].ucLow_Latency          = m_sUserSystemSetting.sCommonSetting.ucLowLatency;   //ZU860_Clare_0152
        #endif	/*Low_Latency_All*/
        dvC341_IintChannelSetting((eMCT)ucCH,hal_sChannelSetting[eMCT_CH1]);

        halScaler_ColorSpace_ScalerValueGet((UINT8)eMCT_CH1);       //A70LV_Doulas_0280 update eVideoYUV
        for(ucInputSource = 0 ; ucInputSource < eCM_SOURCE_NUMBER ; ucInputSource++)   //ZU860_Doulas_0011 modify//A70LV_Doulas_0112
            dvC341_AutoImage_Set(ucInputSource,m_sUserSystemSetting.sSourceDependSetting[ucInputSource].ucAutoImage);
    }
    else
    {
        ucMCT_CH = eMCT_CH2;
        ucInputSource = halScaler_InputSource_Get(eMCT_CH1);
        ucInputSource2 = halScaler_InputSource_Get(ucMCT_CH);
        hal_sChannelSetting[eMCT_CH2].eInputSource          = (eCM_SOURCE_ID)halScaler_InputSource_Get((UINT8)eMCT_CH2);       //A70LV_Doulas_0072 modify
        hal_sChannelSetting[eMCT_CH2].eScalingMode          = (eCM_SCALING_MODE_ID)m_sUserSystemSetting.sSourceDependSetting[ucInputSource].ucAspectRatio;       //A70LV_Doulas_0072 modify
        hal_sChannelSetting[eMCT_CH2].eOverScan             = (eOVER_SCAN_TYPE)m_sUserSystemSetting.sSourceDependSetting[ucInputSource].ucOverScan;          //A70LV_Doulas_0072 modify
        hal_sChannelSetting[eMCT_CH2].eColorFormat          = (eCOLOR_FORMAT)halScaler_ColorSpace_ScalerValueGet((UINT8)eMCT_CH2);    //A70LV_Doulas_0072 modify//A70LV_Doulas_0069 modify
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
        //G100_Steven_0088
        if(halScaler_PIP_PBP_Enable_Get()!=ets_OFF)
        {
            hal_sChannelSetting[eMCT_CH2].iBrightness           = halScaler_Brightness_Value_Get(eMCT_CH2); //steven_test
            hal_sChannelSetting[eMCT_CH2].dContrast             = halScaler_Contrast_Value_Get(eMCT_CH2);
            //LOG_MSG(db_ALWAYS, "IintChannelSetting CH2 %d, %d\r\n",hal_sChannelSetting[eMCT_CH2].iBrightness,hal_sChannelSetting[eMCT_CH2].dContrast );
        }
        else
        {
            hal_sChannelSetting[eMCT_CH2].iBrightness           = halScaler_Brightness_Value_Get(eMCT_CH1); //steven_test
            hal_sChannelSetting[eMCT_CH2].dContrast             = halScaler_Contrast_Value_Get(eMCT_CH1);
            //LOG_MSG(db_ALWAYS, "IintChannelSetting CH1 %d, %d\r\n",hal_sChannelSetting[eMCT_CH2].iBrightness,hal_sChannelSetting[eMCT_CH2].dContrast );
        }
        hal_sChannelSetting[eMCT_CH2].dColor                = halScaler_Saturation_Value_Get(eMCT_CH1);
        hal_sChannelSetting[eMCT_CH2].iHue                  = halScaler_HUE_Value_Get(eMCT_CH1);
        hal_sChannelSetting[eMCT_CH2].iRedOffset            = halScaler_RedOffset_Value_Get(eMCT_CH1);
        hal_sChannelSetting[eMCT_CH2].iGreenOffset          = halScaler_GreenOffset_Value_Get(eMCT_CH1);
        hal_sChannelSetting[eMCT_CH2].iBlueOffset           = halScaler_BlueOffset_Value_Get(eMCT_CH1);
        hal_sChannelSetting[eMCT_CH2].dRedGain              = halScaler_RedGain_Value_Get(eMCT_CH1);
        hal_sChannelSetting[eMCT_CH2].dGreenGain            = halScaler_GreenGain_Value_Get(eMCT_CH1);
        hal_sChannelSetting[eMCT_CH2].dBlueGain             = halScaler_BlueGain_Value_Get(eMCT_CH1);
        hal_sChannelSetting[eMCT_CH2].cGamma                = ets_OFF;
        hal_sChannelSetting[eMCT_CH2].cTemporal_NR          = m_sUserSystemSetting.sImageSetting.ucTemporalNoiseReduction;
        hal_sChannelSetting[eMCT_CH2].cMPEG_NR              = m_sUserSystemSetting.sImageSetting.ucMPEGNoiseReduction;
        hal_sChannelSetting[eMCT_CH2].cDetectFilm           = m_sUserSystemSetting.sImageSetting.ucFilm;
        hal_sChannelSetting[eMCT_CH2].cPIP_PBP_ON           = !!m_sUserSystemSetting.sSourceSetting.ucPIPEnable;
        hal_sChannelSetting[eMCT_CH2].cMain_Layout          = m_sUserSystemSetting.sSourceSetting.ucMainLayout;
        hal_sChannelSetting[eMCT_CH2].cPIP_Size             = m_sUserSystemSetting.sSourceSetting.ucPIPSize;
        hal_sChannelSetting[eMCT_CH2].cFREEZE               = m_sUserSystemSetting.sImageSetting.ucImageFreeze;
        hal_sChannelSetting[eMCT_CH2].cTestPattern          = eCM_TEST_PATTERN_OFF;          //A70LV_Doulas_0035
        hal_sChannelSetting[eMCT_CH2].eScanMode             = ucFrontEndeScanMode[ucCH];        //A70LV_Doulas_0076
        hal_sChannelSetting[eMCT_CH2].eVideoYUV             = eVIDEO_YUV_UNKNOW;    //A70LV_Doulas_0109
        hal_sChannelSetting[eMCT_CH2].eVGA_SYNC_TYPE        = ucVGA_SYNC_TYPE[ucCH];//eVGA_SYNC_TYPE_SOG;//eVGA_SYNC_TYPE_UNKNOW;    //A70LV_Doulas_0280 modify//A70LV_Doulas_0214 modify//A70LV_Doulas_0112
        hal_sChannelSetting[eMCT_CH2].eOSD_3D_Enable        = m_sUserSystemSetting.sSourceDependSetting[ucInputSource].uc3D_Enable;     //A70LV_Doulas_0154
		#ifdef Low_Latency_All
        hal_sChannelSetting[eMCT_CH2].ucLow_Latency          = m_sUserSystemSetting.sCommonSetting.ucLowLatency;   //ZU860_Clare_0152
        #endif	/*Low_Latency_All*/
        dvC341_IintChannelSetting((eMCT)ucCH,hal_sChannelSetting[eMCT_CH2]);

        halScaler_ColorSpace_ScalerValueGet((UINT8)eMCT_CH2);       //A70LV_Doulas_0280 update eVideoYUV
    }
    return eResult;
}

UINT16 halScaler_DigitalHorzZoom_Get(void)
{
    //eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    //eSCALER_EXEC_CODE eC821Resut;
    UINT8 ucInputSource;
    UINT8 ucSizePresets;
    UINT16 uiSetting;

    //LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): Digital_Horz_Zoom=%d\r\n", __FUNCTION__, __LINE__, uiSetting);

    ucInputSource = halScaler_InputSource_Get(eMCT_CH1);
    ucSizePresets = halScaler_SizePresets_Get(eMCT_CH1);
    if(ucSizePresets == eCM_SCALING_MODE_CUSTOM)
        uiSetting = m_sUserSystemSetting.sSourceDependSetting[ucInputSource].uiCustomDigitalHorzZoom;
    else
        uiSetting = m_sUserSystemSetting.sSourceDependSetting[ucInputSource].uiDigitalHorzZoom;

    return uiSetting;
}

UINT16 halScaler_DigitalVertZoom_Get(void)
{
    //eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    //eSCALER_EXEC_CODE eC821Resut;
    UINT8 ucInputSource;
    UINT8 ucSizePresets;
    UINT16 uiSetting;

    //LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): Digital_Vert_Zoom=%d\r\n", __FUNCTION__, __LINE__, uiSetting);

    ucInputSource = halScaler_InputSource_Get(eMCT_CH1);
    ucSizePresets = halScaler_SizePresets_Get(eMCT_CH1);
    if(ucSizePresets == eCM_SCALING_MODE_CUSTOM)
        uiSetting = m_sUserSystemSetting.sSourceDependSetting[ucInputSource].uiCustomDigitalVertZoom;
    else
        uiSetting = m_sUserSystemSetting.sSourceDependSetting[ucInputSource].uiDigitalVertZoom;

    return uiSetting;
}

UINT8 halScaler_DigitalHorzShift_Get(void)
{
    //eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    //eSCALER_EXEC_CODE eC821Resut;
    UINT8 ucInputSource;
    UINT8 ucSizePresets;
    UINT8 ucSetting;

    //LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): Digital_Horz_Shift=%d\r\n", __FUNCTION__, __LINE__, ucSetting);

    ucInputSource = halScaler_InputSource_Get(eMCT_CH1);
    ucSizePresets = halScaler_SizePresets_Get(eMCT_CH1);
    if(ucSizePresets == eCM_SCALING_MODE_CUSTOM)
        ucSetting = m_sUserSystemSetting.sSourceDependSetting[ucInputSource].uiCustomDigitalHorzShift;
    else
        ucSetting = m_sUserSystemSetting.sSourceDependSetting[ucInputSource].uiDigitalHorzShift;

    return ucSetting;
}

UINT8 halScaler_DigitalVertShift_Get(void)
{
    //eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    //eSCALER_EXEC_CODE eC821Resut;
    UINT8 ucInputSource;
    UINT8 ucSizePresets;
    UINT8 ucSetting;

    //LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): Digital_Vert_Shift=%d\r\n", __FUNCTION__, __LINE__, ucSetting);

    ucInputSource = halScaler_InputSource_Get(eMCT_CH1);
    ucSizePresets = halScaler_SizePresets_Get(eMCT_CH1);
    if(ucSizePresets == eCM_SCALING_MODE_CUSTOM)
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
    UINT8  ucVal;
    UINT8  ucPresetUserMode = halScaler_PictureSettings_PreUser_Get(ucCH);

#if 1    //ZU860_Doulas_0011 modify
    if ( ucSetting > SHARPNESS_MAX_VALUE || ucSetting < SHARPNESS_MIN_VALUE )
        return eHAL_SCALER_EXEC_CH_OVER_RANGE;
#else
    if ( ucSetting > e_SHARPNESS_MINIMUM)// || ucSetting < e_SHARPNESS_MAXIMUN )
        return eHAL_SCALER_EXEC_CH_OVER_RANGE;
#endif

    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): Detail=%d\r\n", __FUNCTION__, __LINE__, ucSetting);

    if (eCM_PICTURE_SETTINGS_USER != ucPresetMode) //G100_Steven_0034
    	m_sUserSystemSetting.sColorSetting[ucInputSource][ucPresetMode][ucSignalType].ucSharpness = ucSetting;
    else
    	m_sUserSystemSetting.sUserColorSetting[ucInputSource][ucPresetUserMode][ucSignalType].ucSharpness = ucSetting;

    dvC341_Sharpness_Value_Set(eMCT_CH1,ucSetting);       //A70LV_Doulas_0155
    dvC341_Sharpness_Value_Set(eMCT_CH2,ucSetting);       //A70LV_Doulas_0155
    return eResult;
}

UINT8 halScaler_Detail_Value_Get(UINT8 ucCH)
{
    //UINT8  ucInputSource = halScaler_InputSource_Get(ucCH);
    //UINT8  ucPresetMode = halScaler_PictureSettings_Get(ucCH);
    //UINT8  ucSignalType = halScaler_SignalType_Get(ucCH);

    //return m_sUserSystemSetting.sColorSetting[ucInputSource][ucPresetMode][ucSignalType].ucSharpness;
    return halScaler_ColorSetting_Get(eHAL_COLOR_SETTING_SHARPNESS, ucCH); //G100_Steven_0016
}

eHAL_SCALER_EXEC_CODE halScaler_SkinColor_Value_Set(UINT8 ucCH,UINT8 ucSetting)  //A70LV_Doulas_0072 modify//A70LV_Doulas_0029 modify
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    UINT8  ucInputSource = halScaler_InputSource_Get(ucCH);
    UINT8  ucPresetMode = halScaler_PictureSettings_Get(ucCH);
    UINT8  ucSignalType = halScaler_SignalType_Get(ucCH);
    UINT8  ucVal;
    UINT8  ucPresetUserMode = halScaler_PictureSettings_PreUser_Get(ucCH);

    if ( ucSetting > eSKIN_COLOR_HIGH)// || ucSetting < eSKIN_COLOR_OFF )
        return eHAL_SCALER_EXEC_CH_OVER_RANGE;

    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): SkinColor=%d\r\n", __FUNCTION__, __LINE__, ucSetting);

    if (eCM_PICTURE_SETTINGS_USER != ucPresetMode) //G100_Steven_0034
    	m_sUserSystemSetting.sColorSetting[ucInputSource][ucPresetMode][ucSignalType].ucSkinColor = ucSetting;
    else
    	m_sUserSystemSetting.sUserColorSetting[ucInputSource][ucPresetUserMode][ucSignalType].ucSkinColor = ucSetting;

    dvC341_SkinColor_Value_Set(eMCT_CH1,ucSetting);       //A70LV_Doulas_0155
    dvC341_SkinColor_Value_Set(eMCT_CH2,ucSetting);       //A70LV_Doulas_0155

    return eResult;
}

UINT8 halScaler_SkinColor_Value_Get(UINT8 ucCH)
{
    //UINT8  ucInputSource = halScaler_InputSource_Get(ucCH);
    //UINT8  ucPresetMode = halScaler_PictureSettings_Get(ucCH);
    //UINT8  ucSignalType = halScaler_SignalType_Get(ucCH);

    //return m_sUserSystemSetting.sColorSetting[ucInputSource][ucPresetMode][ucSignalType].ucSkinColor;
    return halScaler_ColorSetting_Get(eHAL_COLOR_SETTING_SKINCOLOR, ucCH); //G100_Steven_0016
}

INT16 halScaler_Brightness_Value_Get(UINT8 ucCH)
{
    INT16 iC341_setting;
    //UINT8  ucInputSource = halScaler_InputSource_Get(ucCH);
    //UINT8  ucPresetMode = halScaler_PictureSettings_Get(ucCH);
    //UINT8  ucSignalType = halScaler_SignalType_Get(ucCH);
    UINT8  ucSetting;

    ucSetting = halScaler_ColorSetting_Get(eHAL_COLOR_SETTING_BRIGHTNESS, ucCH); //G100_Steven_0016
   // ucSetting = m_sUserSystemSetting.sColorSetting[ucInputSource][ucPresetMode][ucSignalType].ucBrightness;

    //convert range 0~100(default:50) to -512~512(default:0)
    iC341_setting = (INT16)(((DOUBLE)ucSetting/(DOUBLE)(BRIGHTNESS_MAX_VALUE - BRIGHTNESS_MIN_VALUE))*(C341_BRIGHTNESS_MAX - C341_BRIGHTNESS_MIN)) + C341_BRIGHTNESS_MIN;

    return iC341_setting;
}

DOUBLE halScaler_Contrast_Value_Get(UINT8 ucCH)
{
    DOUBLE dC341_setting;
    //UINT8  ucInputSource = halScaler_InputSource_Get(ucCH);
    //UINT8  ucPresetMode = halScaler_PictureSettings_Get(ucCH);
    //UINT8  ucSignalType = halScaler_SignalType_Get(ucCH);
    UINT8  ucSetting;

    //LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): Contrast=%d\r\n", __FUNCTION__, __LINE__, ucSetting);

    //ucSetting = m_sUserSystemSetting.sColorSetting[ucInputSource][ucPresetMode][ucSignalType].ucContrast;
    ucSetting = halScaler_ColorSetting_Get(eHAL_COLOR_SETTING_CONTRAST, ucCH); //G100_Steven_0016

    //convert range 0~100(default:50) to 0~3.99(default:1)
    if ( ucSetting<CONTRAST_DEFAULT_VALUE )
    {
        dC341_setting = (DOUBLE)ucSetting/(DOUBLE)(CONTRAST_DEFAULT_VALUE-CONTRAST_MIN_VALUE)*(C341_CONTRAST_DEFAULT-C341_CONTRAST_MIN);
    }
    else if (ucSetting>CONTRAST_DEFAULT_VALUE)
    {
        dC341_setting = ((DOUBLE)ucSetting-CONTRAST_DEFAULT_VALUE)/(DOUBLE)(CONTRAST_MAX_VALUE-CONTRAST_DEFAULT_VALUE)*(C341_CONTRAST_MAX-C341_CONTRAST_DEFAULT) + C341_CONTRAST_DEFAULT;
    }
    else
    {
        dC341_setting = C341_CONTRAST_DEFAULT;
    }

    return dC341_setting;
}

INT16 halScaler_HUE_Value_Get(UINT8 ucCH)
{
    INT16 iC341_setting;
    //UINT8  ucInputSource = halScaler_InputSource_Get(ucCH);
    //UINT8  ucPresetMode = halScaler_PictureSettings_Get(ucCH);
    //UINT8  ucSignalType = halScaler_SignalType_Get(ucCH);
    UINT8  ucSetting;

    //LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): HUE=%d\r\n", __FUNCTION__, __LINE__, ucSetting);

    //ucSetting = m_sUserSystemSetting.sColorSetting[ucInputSource][ucPresetMode][ucSignalType].ucTint;
    ucSetting = halScaler_ColorSetting_Get(eHAL_COLOR_SETTING_TINT, ucCH); //G100_Steven_0016

    //convert range 0~100(default:50) to -180~180(default:0)
    iC341_setting = (INT16)(((DOUBLE)ucSetting/(DOUBLE)(HUE_MAX_VALUE - HUE_MIN_VALUE))*(C341_HUE_MAX - C341_HUE_MIN)) + C341_HUE_MIN;

    return iC341_setting;
}

DOUBLE halScaler_Saturation_Value_Get(UINT8 ucCH)
{
    DOUBLE dC341_setting;
    //UINT8  ucInputSource = halScaler_InputSource_Get(ucCH);
    //UINT8  ucPresetMode = halScaler_PictureSettings_Get(ucCH);
    //UINT8  ucSignalType = halScaler_SignalType_Get(ucCH);
    UINT8  ucSetting;

    //LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): Saturation=%d\r\n", __FUNCTION__, __LINE__, ucSetting);

    //ucSetting = m_sUserSystemSetting.sColorSetting[ucInputSource][ucPresetMode][ucSignalType].ucSaturation;

    ucSetting = halScaler_ColorSetting_Get(eHAL_COLOR_SETTING_SATURATION, ucCH); //G100_Steven_0016
    //convert range 0~100(default:50) to 0~3.99(default:1)
    if ( ucSetting< SATURATION_DEFAULT_VALUE )
    {
        dC341_setting = (DOUBLE)ucSetting/(DOUBLE)(SATURATION_DEFAULT_VALUE- SATURATION_MIN_VALUE)*(C341_SATURATION_DEFAULT-C341_SATURATION_MIN);
    }
    else if (ucSetting> SATURATION_DEFAULT_VALUE)
    {
        dC341_setting = ((DOUBLE)ucSetting- SATURATION_DEFAULT_VALUE)/(DOUBLE)(SATURATION_MAX_VALUE- SATURATION_DEFAULT_VALUE)*(C341_SATURATION_MAX- C341_SATURATION_DEFAULT) + C341_SATURATION_DEFAULT;
    }
    else
    {
        dC341_setting = C341_SATURATION_DEFAULT;
    }

    return dC341_setting;
}

INT16 halScaler_RedOffset_Value_Get(UINT8 ucCH)
{
    INT16 iC341_setting;
    //eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    //UINT8  ucInputSource = halScaler_InputSource_Get(ucCH);
    //UINT8  ucPresetMode = halScaler_PictureSettings_Get(ucCH);
    //UINT8  ucSignalType = halScaler_SignalType_Get(ucCH);
    UINT8  ucSetting;

    //LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): RedOffset=%d\r\n", __FUNCTION__, __LINE__, ucSetting);

    //ucSetting = m_sUserSystemSetting.sColorSetting[ucInputSource][ucPresetMode][ucSignalType].ucRedOffset;
    ucSetting = halScaler_ColorSetting_Get(eHAL_COLOR_SETTING_REDOFFSET, ucCH); //G100_Steven_0016

    //convert range 0~100(default:50) to -511~511(default:0)
    iC341_setting = (INT16)(((DOUBLE)ucSetting/(DOUBLE)(RED_OFFSET_MAX_VALUE - RED_OFFSET_MIN_VALUE))*(C341_RED_OFFSET_MAX - C341_RED_OFFSET_MIN)) + C341_RED_OFFSET_MIN;

    return iC341_setting;
}

INT16 halScaler_GreenOffset_Value_Get(UINT8 ucCH)
{
    INT16 iC341_setting;
    //eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    //UINT8  ucInputSource = halScaler_InputSource_Get(ucCH);
    //UINT8  ucPresetMode = halScaler_PictureSettings_Get(ucCH);
    //UINT8  ucSignalType = halScaler_SignalType_Get(ucCH);
    UINT8  ucSetting;

    //LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): GreenOffset=%d\r\n", __FUNCTION__, __LINE__, ucSetting);

    //ucSetting = m_sUserSystemSetting.sColorSetting[ucInputSource][ucPresetMode][ucSignalType].ucGreenOffset;
    ucSetting = halScaler_ColorSetting_Get(eHAL_COLOR_SETTING_GREENOFFSET, ucCH); //G100_Steven_0016

    //convert range 0~100(default:50) to -511~511(default:0)
    iC341_setting = (INT16)(((DOUBLE)ucSetting/(DOUBLE)(GREEN_OFFSET_MAX_VALUE - GREEN_OFFSET_MIN_VALUE))*(C341_GREEN_OFFSET_MAX - C341_GREEN_OFFSET_MIN)) + C341_GREEN_OFFSET_MIN;

    return iC341_setting;
}

INT16 halScaler_BlueOffset_Value_Get(UINT8 ucCH)
{
    INT16 iC341_setting;
    //eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    //UINT8  ucInputSource = halScaler_InputSource_Get(ucCH);
    //UINT8  ucPresetMode = halScaler_PictureSettings_Get(ucCH);
    //UINT8  ucSignalType = halScaler_SignalType_Get(ucCH);
    UINT8  ucSetting;

    //LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): BlueOffset=%d\r\n", __FUNCTION__, __LINE__, ucSetting);

    //ucSetting = m_sUserSystemSetting.sColorSetting[ucInputSource][ucPresetMode][ucSignalType].ucBlueOffset;

    ucSetting = halScaler_ColorSetting_Get(eHAL_COLOR_SETTING_BLUEOFFSET, ucCH); //G100_Steven_0016

    //convert range 0~100(default:50) to -511~511(default:0)
    iC341_setting = (INT16)(((DOUBLE)ucSetting/(DOUBLE)(BLUE_OFFSET_MAX_VALUE - BLUE_OFFSET_MIN_VALUE))*(C341_BLUE_OFFSET_MAX - C341_BLUE_OFFSET_MIN)) + C341_BLUE_OFFSET_MIN;

    return iC341_setting;
}

DOUBLE halScaler_RedGain_Value_Get(UINT8 ucCH)
{
    DOUBLE dC341_setting;
    //eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    //UINT8  ucInputSource = halScaler_InputSource_Get(ucCH);
    //UINT8  ucPresetMode = halScaler_PictureSettings_Get(ucCH);
    //UINT8  ucSignalType = halScaler_SignalType_Get(ucCH);
    UINT8  ucSetting;

    //LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): RedGain=%d\r\n", __FUNCTION__, __LINE__, ucSetting);

    //ucSetting = m_sUserSystemSetting.sColorSetting[ucInputSource][ucPresetMode][ucSignalType].ucRedGain;
    ucSetting = halScaler_ColorSetting_Get(eHAL_COLOR_SETTING_REDGAIN, ucCH); //G100_Steven_0016

    //convert range 0~100(default:50) to 0~3.99(default:1)
    if ( ucSetting<RED_GAIN_DEFAULT_VALUE )
    {
        dC341_setting = (DOUBLE)ucSetting/(DOUBLE)(RED_GAIN_DEFAULT_VALUE-RED_GAIN_MIN_VALUE)*(C341_RED_GAIN_DEFAULT-C341_RED_GAIN_MIN);
    }
    else if (ucSetting>RED_GAIN_DEFAULT_VALUE)
    {
        dC341_setting = ((DOUBLE)ucSetting-RED_GAIN_DEFAULT_VALUE)/(DOUBLE)(RED_GAIN_MAX_VALUE-RED_GAIN_DEFAULT_VALUE)*(C341_RED_GAIN_MAX-C341_RED_GAIN_DEFAULT) + C341_RED_GAIN_DEFAULT;
    }
    else
    {
        dC341_setting = C341_RED_GAIN_DEFAULT;
    }

    return dC341_setting;
}

DOUBLE halScaler_GreenGain_Value_Get(UINT8 ucCH)
{
    DOUBLE dC341_setting;
    //eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    //UINT8  ucInputSource = halScaler_InputSource_Get(ucCH);
    //UINT8  ucPresetMode = halScaler_PictureSettings_Get(ucCH);
    //UINT8  ucSignalType = halScaler_SignalType_Get(ucCH);
    UINT8  ucSetting;

    //LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): GreenGain=%d\r\n", __FUNCTION__, __LINE__, ucSetting);

    //ucSetting = m_sUserSystemSetting.sColorSetting[ucInputSource][ucPresetMode][ucSignalType].ucGreenGain;
    ucSetting = halScaler_ColorSetting_Get(eHAL_COLOR_SETTING_GREENGAIN, ucCH); //G100_Steven_0016

    //convert range 0~100(default:50) to 0~3.99(default:1)
    if ( ucSetting<GREEN_GAIN_DEFAULT_VALUE )
    {
        dC341_setting = (DOUBLE)ucSetting/(DOUBLE)(GREEN_GAIN_DEFAULT_VALUE-GREEN_GAIN_MIN_VALUE)*(C341_GREEN_GAIN_DEFAULT-C341_GREEN_GAIN_MIN);
    }
    else if (ucSetting>GREEN_GAIN_DEFAULT_VALUE)
    {
        dC341_setting = ((DOUBLE)ucSetting-GREEN_GAIN_DEFAULT_VALUE)/(DOUBLE)(GREEN_GAIN_MAX_VALUE-GREEN_GAIN_DEFAULT_VALUE)*(C341_GREEN_GAIN_MAX-C341_GREEN_GAIN_DEFAULT) + C341_GREEN_GAIN_DEFAULT;
    }
    else
    {
        dC341_setting = C341_GREEN_GAIN_DEFAULT;
    }

    return dC341_setting;
}

DOUBLE halScaler_BlueGain_Value_Get(UINT8 ucCH)
{
    DOUBLE dC341_setting;
    //eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    //UINT8  ucInputSource = halScaler_InputSource_Get(ucCH);
    //UINT8  ucPresetMode = halScaler_PictureSettings_Get(ucCH);
    //UINT8  ucSignalType = halScaler_SignalType_Get(ucCH);
    UINT8  ucSetting;

    //LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): BlueGain=%d\r\n", __FUNCTION__, __LINE__, ucSetting);

    //ucSetting = m_sUserSystemSetting.sColorSetting[ucInputSource][ucPresetMode][ucSignalType].ucBlueGain;

    ucSetting = halScaler_ColorSetting_Get(eHAL_COLOR_SETTING_BLUEGAIN, ucCH); //G100_Steven_0016

    //convert range 0~100(default:50) to 0~3.99(default:1)
    if ( ucSetting<BLUE_GAIN_DEFAULT_VALUE )
    {
        dC341_setting = (DOUBLE)ucSetting/(DOUBLE)(BLUE_GAIN_DEFAULT_VALUE-BLUE_GAIN_MIN_VALUE)*(C341_BLUE_GAIN_DEFAULT-C341_BLUE_GAIN_MIN);
    }
    else if (ucSetting>BLUE_GAIN_DEFAULT_VALUE)
    {
        dC341_setting = ((DOUBLE)ucSetting-BLUE_GAIN_DEFAULT_VALUE)/(DOUBLE)(BLUE_GAIN_MAX_VALUE-BLUE_GAIN_DEFAULT_VALUE)*(C341_BLUE_GAIN_MAX-C341_BLUE_GAIN_DEFAULT) + C341_BLUE_GAIN_DEFAULT;
    }
    else
    {
        dC341_setting = C341_BLUE_GAIN_DEFAULT;
    }

    return dC341_setting;
}

eHAL_SCALER_EXEC_CODE halScaler_Temporal_NR_Value_Set(UINT8 ucSetting)   //A70LV_Doulas_0029 modify
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

    if ( ucSetting > eTEMPORAL_NR_HIGH)// || ucSetting < eTEMPORAL_NR_OFF )
        return eHAL_SCALER_EXEC_CH_OVER_RANGE;

    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): Temporal NR=%d\r\n", __FUNCTION__, __LINE__, ucSetting);

    m_sUserSystemSetting.sImageSetting.ucTemporalNoiseReduction = ucSetting;

    dvC341_Temporal_NR_Value_Set(eMCT_CH1,ucSetting);
    dvC341_Temporal_NR_Value_Set(eMCT_CH2,ucSetting);

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_MPEG_NR_Value_Set(UINT8 ucSetting)   //A70LV_Doulas_0029 modify
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

    if ( ucSetting > eMPEG_NR_HIGH)// || ucSetting < eMPEG_NR_OFF )
        return eHAL_SCALER_EXEC_CH_OVER_RANGE;

    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): MPEG NR=%d\r\n", __FUNCTION__, __LINE__, ucSetting);

    m_sUserSystemSetting.sImageSetting.ucMPEGNoiseReduction = ucSetting;

    dvC341_MPEG_NR_Value_Set(eMCT_CH1,ucSetting);
    dvC341_MPEG_NR_Value_Set(eMCT_CH2,ucSetting);

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_DetectFilm_Value_Set(UINT8 ucSetting)        //A70LV_Doulas_0029 modify
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

    if ( ucSetting > ets_ON)// || ucSetting < ets_OFF )
        return eHAL_SCALER_EXEC_CH_OVER_RANGE;

    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): Detect Film=%d\r\n", __FUNCTION__, __LINE__, ucSetting);

    m_sUserSystemSetting.sImageSetting.ucFilm = ucSetting;

    dvC341_Detect_Film_Value_Set(eMCT_CH1,ucSetting);
    dvC341_Detect_Film_Value_Set(eMCT_CH2,ucSetting);

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_PictureSettings_Value_Set(UINT8 ucCH,UINT8 ucSetting)     //A70LV_Doulas_0072 modify//A70LV_Doulas_0030
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    UINT8  ucInputSource = halScaler_InputSource_Get(ucCH);

    if ( ucSetting>= eCM_PICTURE_SETTINGS_NUMBER)// || ucSetting< eIFC_VIDEO )
        return eHAL_SCALER_EXEC_CH_OVER_RANGE;

    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): PictureSettings =%d\r\n", __FUNCTION__, __LINE__, ucSetting);

    m_sUserSystemSetting.sSourceDependSetting[ucInputSource].ucPresetMode = ucSetting;

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_MeasureInput(UINT8 ucCH)      //A70LV_Doulas_0031
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

//    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);

    //bIs4K = halScaler_Is4KPanel(m_sHalScalerInfo[ucCH].ePanelTimingId);

    {
        if(ucCH >= eMCT_LAST)
            return eHAL_SCALER_EXEC_CH_OVER_RANGE;
    }

    /* See if we can obtain the semaphore.  If the semaphore is not
    available wait 10 ticks to see if it becomes free. */
    if(halScaler_SemaphoreTake(__FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[ucCH].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(__FUNCTION__);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }

        if(dvC341_CheckInput_4K((eMCT)ucCH))
        {
			dvC341_SetIdual_2K((eMCT)ucCH);
            eResult = eHAL_SCALER_EXEC_CODE_PASS;
        }
        else
        {
            eResult = eHAL_SCALER_EXEC_CODE_FAIL;
        }

        halScaler_SemaphoreGive(__FUNCTION__);

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

    if(halScaler_SemaphoreTake(__FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[0].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(__FUNCTION__);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }

        eResult = halScaler_OSD_TestPattern_Set(eMCT_CH1,ucTestPattern);

        halScaler_SemaphoreGive(__FUNCTION__);
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

    if ( ucSetting > eSERVICE_TP_COLOR_BAR)// || ucSetting < eSERVICE_TP_OFF )
        return eHAL_SCALER_EXEC_CH_OVER_RANGE;

    LOG_MSG(db_ALWAYS, "(func:%s, line:%d): GreenOffset=%d\r\n", __FUNCTION__, __LINE__, ucSetting);

    m_sUserSystemSetting.sCommonSetting.ucOSDTestPattern = ucSetting;
    dvC341_TestPattern_Set((eMCT)ucCH,ucSetting);

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_Service_TestPattern(const UINT8 ucTestPattern)
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

    if(halScaler_SemaphoreTake(__FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[0].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(__FUNCTION__);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }

        eResult = halScaler_Service_TestPattern_Set(eMCT_CH1,ucTestPattern);

        halScaler_SemaphoreGive(__FUNCTION__);
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
    dvC341_TestPattern_Set((eMCT)ucCH,ucSetting);

    return eResult;
}

UINT8 halScaler_PictureSettings_Get_NotReturnUser(UINT8 ucCH)     //A70LV_Doulas_0044
{
    UINT8 ucInputSource = halScaler_InputSource_Get(ucCH);
    UINT8 ucVal;

    if(m_sUserSystemSetting.sSourceDependSetting[ucInputSource].ucPresetMode == eCM_PICTURE_SETTINGS_USER)
        ucVal = m_sUserSystemSetting.sSourceDependSetting[ucInputSource].ucPreUserMode;
    else
        ucVal = m_sUserSystemSetting.sSourceDependSetting[ucInputSource].ucPresetMode;

    return ucVal;
}

eHAL_SCALER_EXEC_CODE halScaler_InputPixelClock_Get(UINT8 ucCH,UINT8 *ucValue)     //A70LV_Doulas_0056
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

    dvC341_InputPixelClock_Get((eMCT)ucCH,ucValue);     //A70LV_Doulas_0072 modify

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_InputResolution_Get(UINT8 ucCH,UINT8 *ucValue)
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

    dvC341_InputResolution_Get((eMCT)ucCH,ucValue);     //A70LV_Doulas_0072 modify

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_InputHorzRefresh_Get(UINT8 ucCH,UINT8 *ucValue)
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

    dvC341_InputHorzRefresh_Get((eMCT)ucCH,ucValue);      //A70LV_Doulas_0072 modify

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_InputVertRefresh_Get(UINT8 ucCH,UINT8 *ucValue)
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

    dvC341_InputVertRefresh_Get((eMCT)ucCH,ucValue);      //A70LV_Doulas_0072 modify

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_InputVertRefresh2_Get(UINT8 ucCH,UINT16 *uiValue)     //A70LV_Doulas_0200
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

    dvC341_InputVertRefresh2_Get((eMCT)ucCH,uiValue);

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_InputVertRefresh2_Get_FromSyncCount(UINT8 ucCH,UINT16 *uiValue)       //A35G2_CDS_Simon_0052
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

    if(halScaler_SemaphoreTake(__FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[0].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(__FUNCTION__);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }

        dvC341_InputVertRefresh2_Get_FromMCLKcount((eMCT)ucCH, uiValue);

        halScaler_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore\r\n", __FUNCTION__, __LINE__);
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_InputSignalFormat_Get(UINT8 ucCH,UINT8 *ucValue)
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

    dvC341_InputSignalFormat_Get((eMCT)ucCH,ucValue);     //A70LV_Doulas_0072 modify

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_InputAspectRatio_Get(UINT8 ucCH,UINT8 *ucValue)
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

    dvC341_InputAspectRatio_Get((eMCT)ucCH,ucValue);        //A70LV_Doulas_0072 modify

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_FrontEndColorSpace_Set(UINT8 ucCH,UINT8 ucValue)     //A70LV_Doulas_0076 Add
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

    ucFrontEndColorSpace[ucCH] = ucValue;
    halScaler_ColorSpace_Set(ucCH,halScaler_ColorSpace_Get(ucCH));
    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_FrontEndColorSpace_Get(UINT8 ucCH,UINT8* ucValue)     //A70LV_Doulas_0318 Add
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

    *ucValue = ucFrontEndColorSpace[ucCH];

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_FrontEndColorSpaceInfo_Get(UINT8 ucCH,UINT8* ucValue)     //A70LV_Doulas_0109 Add
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    UINT8 ucVal;
    char aucString[VERSION_STRING_MAX_LENGTH];

    switch(ucFrontEndColorSpace[ucCH])
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
            if(ucFrontEndVideoYUV[ucCH] == eVIDEO_YUV_REC601)
            {
                sprintf(aucString, "REC601\0");
                memcpy(ucValue, aucString, strlen(aucString)+1);
            }
            else if(ucFrontEndVideoYUV[ucCH] == eVIDEO_YUV_REC709)
            {
                sprintf(aucString, "REC709\0");
                memcpy(ucValue, aucString, strlen(aucString)+1);
            }
            else
            {
                ucVal = dvC341_VideoYUV_Get((eMCT)ucCH);

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
        ucFrontEndeScanMode[ucCH] = ucValue;
        dvC341_FrontEndScanMode_Set((eMCT)ucCH,ucValue);
    }
    else
        eResult = eHAL_SCALER_EXEC_CODE_FAIL;

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_FrontEndScanMode_Clear(UINT8 ucCH)      //A70LV_Doulas_0175
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

    //LOG_MSG(db_ALWAYS, "@@@# %d ,%d \r\n",ucFrontEndeScanMode[ucCH],dvC341_Input_V_Active_Get((eMCT)ucCH));       //A70LV_Doulas_0181
    if((ucFrontEndeScanMode[ucCH] == eSCAN_MODE_INTERLACE) &&
       (dvC341_Input_V_Active_Get((eMCT)ucCH) == 288))    //A70LV_Doulas_0181 modify re-sync 576i V-start error issue,check 576i
    {
        ucFrontEndeScanMode[ucCH] = eSCAN_MODE_NO_SIGNAL;
        dvC341_FrontEndScanMode_Set((eMCT)ucCH,eSCAN_MODE_NO_SIGNAL);
    }
    return eResult;
}

BOOL halScaler_ForcedSyncResetType_Get(void)    //A70LV_Doulas_0079
{
    return dvC341_ForcedSyncResetType_Get();
}

eHAL_SCALER_EXEC_CODE halScaler_ForcedSyncResetDisable_Set(void)
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    BOOL   bForceSyncReseVal;
    BOOL   bForceSyncReseVal2;  //A70LV_Doulas_0259
    UINT16 uiH_Toral;
    UINT16 uiV_Toral;
#ifdef C789_2D_NO_FRAME_LOCK   //A70LV_Doulas_0307
    UINT16 uiH_Toral2;
    UINT16 uiV_Toral2;
#endif

    if(halScaler_ForcedSyncResetType_Get() == FALSE)
        return eResult;

    if(halScaler_SemaphoreTake(__FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[0].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(__FUNCTION__);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }
#ifdef C789_2D_NO_FRAME_LOCK   //A70LV_Doulas_0307 modify
        dvC341_ForceSyncReset_Get(&bForceSyncReseVal2,&uiH_Toral2,&uiV_Toral2);
		#ifdef Low_Latency_All	//G100_Clare_0009
		if(!halScaler_Is_LowLatencyMode_On())   //H2PF_Simon_0187
        #endif	/*Low_Latency_All*/
		{
            if(halC789Ctrl_TwistLinkFlag_Get())     //A70LV_Doulas_0367 Modify
            {
                //LOG_MSG(db_ALWAYS,"halScaler_ForcedSyncResetDisable_Set =%d,%d \r\n",uiH_Toral2,uiV_Toral2);
                dvC341_ForcedSyncResetDisable_Set2(uiH_Toral2,uiV_Toral2);
            }
            else
            {
                dvC341_ForcedSyncResetDisable_Set();    //disable ForcedSyncReset and reset C821 H-total & V-tatal
            }
            dvC341_ForceSyncReset_Get(&bForceSyncReseVal,&uiH_Toral,&uiV_Toral);     //get C821 H-total & V-total
            //if(bForceSyncReseVal)     //A70LV_Doulas_0168 remove
            {
                if((halScaler_IS_3D_Enable() == ets_ON) ||
                   (m_sHalScalerInfo[eMCT_CH1].ePanelTimingId == ePANEL_ID_WUXGA_120HZ) ||  //A70LV_Doulas_0377 Add
                   (m_sHalScalerInfo[eMCT_CH1].ePanelTimingId == ePANEL_ID_1080P_120HZ))
                {
                    halC789Ctrl_ForcedSyncResetValueSet((UINT8)TRUE,uiH_Toral,uiV_Toral);
                }
                else
                {
					if(bForceSyncReseVal2)
						halC789Ctrl_ForcedSyncResetValueSet((UINT8)FALSE,uiH_Toral2,uiV_Toral2);
					else
						halC789Ctrl_ForcedSyncResetValueSet((UINT8)FALSE,uiH_Toral,uiV_Toral);
				}
            }
        }
		#ifdef Low_Latency_All	//G100_Clare_0009
		else	//G100_Clare_0008
		{
            dvC341_ForcedSyncResetDisable_Set();    //disable ForcedSyncReset and reset C821 H-total & V-tatal
            dvC341_ForceSyncReset_Get(&bForceSyncReseVal,&uiH_Toral,&uiV_Toral);     //get C821 H-total & V-total
            //if(bForceSyncReseVal)     //A70LV_Doulas_0168 remove
            {
                halC789Ctrl_ForcedSyncResetValueSet((UINT8)TRUE,uiH_Toral,uiV_Toral);
            }
		}
		#endif	/*Low_Latency_All*/
#else
        //A70LV_Doulas_0083 modify
        dvC341_ForceSyncReset_Get(&bForceSyncReseVal2,&uiH_Toral,&uiV_Toral);     //A70LV_Doulas_0259//A70LV_Doulas_0168 remove
        //if(bForceSyncReseVal)   //A70LV_Doulas_0168 remove
        {
            dvC341_ForcedSyncResetDisable_Set();    //disable ForcedSyncReset and reset C821 H-total & V-tatal
            dvC341_ForceSyncReset_Get(&bForceSyncReseVal,&uiH_Toral,&uiV_Toral);     //get C821 H-total & V-total
            dvC341Geo_ForcedSyncResetValueSet(bForceSyncReseVal,uiH_Toral,uiV_Toral); //H30K_Doulas_0016
        }
#endif
        if(bForceSyncReseVal2)          //A70LV_Doulas_0259
        {
            if(palSysCtrlMgr_XillinxFPGA_NewVersionGet() == TRUE)            //H2 wait review
            {
                palSysCtrlMgr_XillinxFPGA_Manual_Mode_Set((UINT8)dvC341_PanelGet());
            }
            else
            {
                palSysCtrlMgr_XillinxFPGA_Reset();
            }
        }

        halScaler_SemaphoreGive(__FUNCTION__);
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
    return dvC341_IsInterlaced_Get((eMCT)ucCH);
}

eHAL_SCALER_EXEC_CODE halScaler_FrontEndVideoYUV_Set(UINT8 ucCH,UINT8 ucValue)     //A70LV_Doulas_0109 Add
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

    ucFrontEndVideoYUV[ucCH] = ucValue;
    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_Horz_Start_Position_Get(UINT8 ucCH,UINT16* uiVal) //G100_Julie_0017
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

    *uiVal = dvC341_Horz_Start_Position_Get((eMCT)ucCH);

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_Vert_Start_Position_Get(UINT8 ucCH,UINT16* uiVal) //G100_Julie_0017
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

    *uiVal = dvC341_Vert_Start_Position_Get((eMCT)ucCH);

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_VGA_H_Total_Get(UINT8 ucCH,UINT16* uiVal)     //A70LV_Doulas_0112
{
    return eHAL_SCALER_EXEC_CODE_PASS;

    #if 0
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

    *uiVal = dvC341_VGA_H_Total_Get((eMCT)ucCH);

    return eResult;
    #endif
}

eHAL_SCALER_EXEC_CODE halScaler_VGA_V_Total_Get(UINT8 ucCH,UINT16* uiVal)     //A70LV_Doulas_0195
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

    *uiVal = dvC341_VGA_V_Total_Get((eMCT)ucCH);

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_VGA_SYNC_TYPE_Set(UINT8 ucCH,UINT8 ucVal)     //A70LV_Doulas_0112
{
    return eHAL_SCALER_EXEC_CODE_PASS;

#if 0
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    dvC341_VGA_SYNC_TYPE_Set((eMCT)ucCH,ucVal);
    ucVGA_SYNC_TYPE[ucCH] = ucVal;          //A70LV_Doulas_0124
    return eResult;
#endif
}

UINT8 halScaler_VGA_SYNC_TYPE_Get(UINT8 ucCH)     //A70LV_Doulas_0124
{
    return ucVGA_SYNC_TYPE[ucCH];
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
    return (UINT32)dvC341_uiHFreq_Get((eMCT)ucCH);
}

eHAL_SCALER_EXEC_CODE halScaler_Position_Run(UINT8 ucCH)    //A70LV_Doulas_0113
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    BOOL bHorzPositionSettingWork = FALSE;
    BOOL bVertPositionSettingWork = FALSE;

    if(halScaler_SemaphoreTake(__FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[0].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_Position_Run() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(__FUNCTION__);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }

        //dvC341_Position_Run((eMCT)ucCH);      //A70LV_Doulas_0154 remove
        bHorzPositionSettingWork = dvC341_CheckingHorzPositionSettingWork((eMCT)ucCH);
        bVertPositionSettingWork = dvC341_CheckingVertPositionSettingWork((eMCT)ucCH);

        if(bHorzPositionSettingWork && bVertPositionSettingWork)
            eResult = eHAL_SCALER_EXEC_CODE_PASS;
        else
            eResult = eHAL_SCALER_EXEC_CODE_FAIL;

        halScaler_SemaphoreGive(__FUNCTION__);
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
    return dvC341_HorzPositionWorkValueGet((eMCT)ucCH);
}

UINT8 halScaler_VertPositionWorkValueGet(UINT8 ucCH)
{
    return dvC341_VertPositionWorkValueGet((eMCT)ucCH);
}

eHAL_SCALER_EXEC_CODE halScaler_AutoPsitionStart(UINT8 ucCH,BOOL bAutoEnable)    //A70LV_Doulas_0118
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

    dvC341_AutoPsitionStart((eMCT)ucCH,bAutoEnable);

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_Config_NoSignalOutput(UINT8 ucCH, UINT8 ucDisplayOutput)      //A70LV_Doulas_0142 //A70LV_Doulas_0120  //A35G2_CDS_Simon_0017
{
    //BOOL bIs4K = FALSE;
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);

    //bIs4K = halScaler_Is4KPanel(m_sHalScalerInfo[ucCH].ePanelTimingId);

    if(ucCH >= eMCT_LAST)
        return eHAL_SCALER_EXEC_CH_OVER_RANGE;


    /* See if we can obtain the semaphore.  If the semaphore is not
    available wait 10 ticks to see if it becomes free. */
    if(halScaler_SemaphoreTake(__FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[ucCH].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(__FUNCTION__);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }

        dvC341_Config_NoSignalOutput((eMCT)ucCH, ucDisplayOutput);   //A70LV_Doulas_0142  //A35G2_CDS_Simon_0017
        LOG_MSG(db_DV_SCALER, "(func:%s, line:%d) dvC341_Config_NoSignalOutput %d\r\n", __FUNCTION__, __LINE__, ucCH);

        halScaler_SemaphoreGive(__FUNCTION__);

    }
    else
    {
        LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore(%d)\r\n", __FUNCTION__, __LINE__,ucCH);
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }


    return eResult;
}

UINT16 halScaler_Input_H_Active_Get(UINT8 ucCH)     //A70LV_Doulas_0124
{
    return dvC341_Input_H_Active_Get((eMCT)ucCH);
}

UINT16 halScaler_Input_V_Active_Get(UINT8 ucCH)     //A70LV_Doulas_0124
{
    return dvC341_Input_V_Active_Get((eMCT)ucCH);
}

eHAL_SCALER_EXEC_CODE halScaler_DDR_InputDataAreaTotalGet(UINT8 eCH,RECT DataArea,UINT32 *ulTotalRed,UINT32 *ulTotalGreen,UINT32 *ulTotalBlue)         //A70LV_Doulas_0124
{
    //eMCT eCH;
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;


    if(eCH >= eMCT_LAST)
        return eHAL_SCALER_EXEC_CODE_FAIL;

    if(halScaler_SemaphoreTake(__FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[0].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(__FUNCTION__);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }

        dvC341_DDR_InputDataAreaTotalGet((eMCT)eCH ,DataArea ,ulTotalRed ,ulTotalGreen ,ulTotalBlue);

        halScaler_SemaphoreGive(__FUNCTION__);
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

    if(halScaler_SemaphoreTake(__FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[0].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(__FUNCTION__);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }

        dvC341_InputADC_Cali_CalibrationEnableSet((eMCT)eCH ,ucADC_CAl_Enable);

        halScaler_SemaphoreGive(__FUNCTION__);
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
    return dvC341_Input_3D_Format_Get();
}

void halScaler_Input_3D_Format_Set(UINT8 ucVal)     //A70LV_Doulas_0154
{
    dvC341_Input_3D_Format_Set(ucVal);
}

#ifdef Low_Latency_All
UINT8 halScaler_Aspect_Ratio_Is_3D_Mode_Get(void)	//ZU860_Clare_0152
{
	return dvC341_AspectRatioIs3DModeGet();
}
#endif	/*Low_Latency_All*/

eHAL_SCALER_EXEC_CODE halScaler_3D_InputPort_Set(void)     //A70LV_Doulas_0154
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

    if(halScaler_SemaphoreTake(__FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[0].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(__FUNCTION__);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }

        dvC341_3D_InputPort_Set();

        halScaler_SemaphoreGive(__FUNCTION__);
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
    return dvC341_Input_3D_Format_Config_Get();
}

eHAL_SCALER_EXEC_CODE halScaler_InputHorzPeriod_Get(UINT8 ucCH,UINT32* udVal)     //A70LV_Doulas_0195
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

    *udVal = dvC341_InputHorzPeriod_Get((eMCT)ucCH);

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_VGA_ModeTable_Get(UINT8 ucCH,UINT8* ucVal)     //A70LV_Doulas_0195
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

    *ucVal = dvC341_VGA_ModeTable_Get((eMCT)ucCH);

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_VGA_ModeTableNumber_Get(UINT8 ucCH,UINT16* uiVal)     //A70LV_Doulas_0195
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

    *uiVal = dvC341_VGA_ModeTableNumber_Get((eMCT)ucCH);

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_ModeAdjusmenttHorzStart_Get(UINT16* uiVal)     //A70LV_Doulas_0195
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

    *uiVal = dvC341_ModeAdjusmenttHorzStart_Get();

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_ModeAdjusmenttVertStart_Get(UINT16* uiVal)     //A70LV_Doulas_0195
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

    *uiVal = dvC341_ModeAdjusmenttVertStart_Get();

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_ModeAdjusmenttHorzStart_Set(UINT16 uiVal)     //A70LV_Doulas_0195
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

    dvC341_ModeAdjusmenttHorzStart_Set(uiVal);

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_ModeAdjusmenttVertStart_Set(UINT16 uiVal)     //A70LV_Doulas_0195
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

    dvC341_ModeAdjusmenttVertStart_Set(uiVal);

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_ModeAdjusmenttEnableSetting(void)         //A70LV_Doulas_0195
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

    dvC341_ModeAdjusmenttEnableSetting();

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_ModeAdjusmenttDisableSetting(void)         //A70LV_Doulas_0195
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

    dvC341_ModeAdjusmenttDisableSetting();

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_Init_Mode_Adjustment_EEPROM_Setting(sTIMING_TABLE sTiming_Table)  //A70LV_Doulas_0195
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

    dvC341_Init_Mode_Adjustment_EEPROM_Setting(sTiming_Table);

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_Input3D_Timing_Get(UINT8 ucCH,UINT8* ucVal)     //A70LV_Doulas_0196
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

    *ucVal = dvC341_Input3D_Timing_Get((eMCT)ucCH);

    return eResult;
}

eHAL_SCALER_EXEC_CODE halScaler_ConfigureForPosition(UINT8 ucCH)     //A70LV_Doulas_0238
{
    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);


    /* See if we can obtain the semaphore.  If the semaphore is not
    available wait 10 ticks to see if it becomes free. */
    if(halScaler_SemaphoreTake(__FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[ucCH].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(__FUNCTION__);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }

        dvC341_BuildPosition_2K((eMCT)ucCH);

        halScaler_SemaphoreGive(__FUNCTION__);

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
    if(halScaler_SemaphoreTake(__FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[ucCH].bScalerInit)
        {
            LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_PowerNormal() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(__FUNCTION__);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }

        dvC341_BuildScaler_2K((eMCT)ucCH);

        halScaler_SemaphoreGive(__FUNCTION__);

    }
    else
    {
        LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore\r\n", __FUNCTION__, __LINE__);
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }


    return eHAL_SCALER_EXEC_CODE_PASS;
}

eHAL_SCALER_EXEC_CODE halScaler_EdgeMask_Set(UINT8 ucSetting)       //ZU860_Doulas_0004
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    eSCALER_EXEC_CODE eC821Resut;
    UINT8 ucInputSource;

    if ( ucSetting > EDGE_MASK_MAX_VALUE)
        return eHAL_SCALER_EXEC_CH_OVER_RANGE;

    //LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): EdgeMask=%d\r\n", __FUNCTION__, __LINE__, ucSetting);

    ucInputSource = halScaler_InputSource_Get(eMCT_CH1);
    m_sUserSystemSetting.sSourceDependSetting[ucInputSource].ucEdgeMask = ucSetting;

    dvC341_EdgeMask_Set(eMCT_CH2,ucSetting);
    eC821Resut = dvC341_EdgeMask_Set(eMCT_CH1,ucSetting);
    if(eC821Resut != eSCALER_EXEC_CODE_PASS)
    {
        eResult = eHAL_SCALER_EXEC_CODE_FAIL;
    }

    return eResult;
}

INT8 halScaler_3DMode_Set(UINT8 ucSetting)        //ZU860_Doulas_0049
{
    UINT8  ucInputSource = halScaler_InputSource_Get(eMCT_CH1);
    m_sUserSystemSetting.sSourceDependSetting[ucInputSource].uc3D_Mode = ucSetting;
    dvC341_OSD_3D_Mode_Set(eMCT_CH1,ucSetting);
    dvC341_OSD_3D_Mode_Set(eMCT_CH2,ucSetting);

    return eHAL_SCALER_EXEC_CODE_PASS;
}

UINT8 halScaler_IS_3D_Enable(void)         //ZU860_Doulas_0054
{
	UINT8  ucValue;
	UINT8  uc3D_Enable = ets_OFF;
    UINT8  uc3D_Mode = ets_OFF;
    UINT8  ucInputSource = halScaler_InputSource_Get(eMCT_CH1);
    UINT8  ucPIP_Mode = ets_OFF;    //ZU860_Doulas_0104

    ucValue = m_sUserSystemSetting.sSourceDependSetting[ucInputSource].uc3D_Enable;
    uc3D_Mode = m_sUserSystemSetting.sSourceDependSetting[ucInputSource].uc3D_Mode;
    ucPIP_Mode = m_sUserSystemSetting.sSourceSetting.ucPIPEnable;        //ZU860_Doulas_0104

    if(uc3D_Mode == eCM_3D_MODE_OFF)                            //H30K_Doulas_0024//ZU860_Doulas_0049
    {
        ucValue = eCM_3D_FORMAT_OFF;
    }
    else if(ucPIP_Mode)   //ZU860_Doulas_0104 Add
    {
        ucValue = eCM_3D_FORMAT_OFF;
    }

	switch(ucValue)
	{
		case eCM_3D_FORMAT_AUTO:
			//get frontend 3D infomation to check 3D on/off
			switch(halScaler_Input_3D_Format_Get())
			{
				case eINPUT_3D_TYPE_FRAMEPACKING:
				case eINPUT_3D_TYPE_SIDEBYSIDE:
				case eINPUT_3D_TYPE_TOPANDBOTTOM:
				//case eINPUT_3D_TYPE_FRAME_SEQUENTIAL:   //H30K_Doulas_0009//ZU860_Doulas_0049 remove
					uc3D_Enable = ets_ON;
					break;

				default:
					uc3D_Enable = ets_OFF;
					break;
			}
			break;

		case eCM_3D_FORMAT_FRAME_PACKING:
			uc3D_Enable = ets_ON;
			break;

		case eCM_3D_FORMAT_SIDE_BY_SIDE:
			uc3D_Enable = ets_ON;
			break;

		case eCM_3D_FORMAT_TOP_AND_BOTTOM:
			uc3D_Enable = ets_ON;
			break;

		case eCM_3D_FORMAT_FRAME_SEQUENTIAL:
			uc3D_Enable = ets_ON;
			break;

		case eCM_3D_FORMAT_OFF:
			uc3D_Enable = ets_OFF;
			break;

		default:
			break;
	}

	return uc3D_Enable;
}


#ifdef Low_Latency_All
UINT8 halScaler_3D_Type(void)
{
	UINT8  ucValue;
	UINT8  uc3D_Enable = ets_OFF;
    UINT8  uc3D_Mode = ets_OFF;
    UINT8  ucInputSource = halScaler_InputSource_Get(eMCT_CH1);
    UINT8  ucPIP_Mode = ets_OFF;    //ZU860_Doulas_0104

    ucValue = m_sUserSystemSetting.sSourceDependSetting[ucInputSource].uc3D_Enable;
    uc3D_Mode = m_sUserSystemSetting.sSourceDependSetting[ucInputSource].uc3D_Mode;
    ucPIP_Mode = m_sUserSystemSetting.sSourceSetting.ucPIPEnable;        //ZU860_Doulas_0104

    if(uc3D_Mode == eCM_3D_MODE_OFF)                            //H30K_Doulas_0024//ZU860_Doulas_0049
    {
        ucValue = eCM_3D_FORMAT_OFF;
    }
    else if(ucPIP_Mode)   //ZU860_Doulas_0104 Add
    {
        ucValue = eCM_3D_FORMAT_OFF;
    }

	switch(ucValue)
	{
		case eCM_3D_FORMAT_AUTO:
			//get frontend 3D infomation to check 3D on/off
			switch(halScaler_Input_3D_Format_Get())
			{
				case eINPUT_3D_TYPE_FRAMEPACKING:
				case eINPUT_3D_TYPE_SIDEBYSIDE:
				case eINPUT_3D_TYPE_TOPANDBOTTOM:
				//case eINPUT_3D_TYPE_FRAME_SEQUENTIAL:   //ZU860_Doulas_0049 remove
					uc3D_Enable = eINPUT_3D_TYPE2_1P4;
					break;

			    //case eINPUT_3D_TYPE_FRAME_SEQUENTIAL:   //H30K_Doulas_0001
                //    uc3D_Enable = eINPUT_3D_TYPE2_FRAME_SEQUENTIAL;   //H30K_Doulas_0009
                //    break;

				default:
					uc3D_Enable = eINPUT_3D_TYPE2_OFF;
					break;
			}
			break;

		case eCM_3D_FORMAT_FRAME_PACKING:
			uc3D_Enable = eINPUT_3D_TYPE2_1P4;
			break;

		case eCM_3D_FORMAT_SIDE_BY_SIDE:
			uc3D_Enable = eINPUT_3D_TYPE2_1P4;
			break;

		case eCM_3D_FORMAT_TOP_AND_BOTTOM:
			uc3D_Enable = eINPUT_3D_TYPE2_1P4;
			break;

		case eCM_3D_FORMAT_FRAME_SEQUENTIAL:
			uc3D_Enable = eINPUT_3D_TYPE2_FRAME_SEQUENTIAL;  //H30K_Doulas_0001
			break;

		case eCM_3D_FORMAT_OFF:
			uc3D_Enable = eINPUT_3D_TYPE2_OFF;
			break;

		default:
			break;
	}

	return uc3D_Enable;

}
#endif	/*Low_Latency_All*/

BOOL halScaler_IsVsync120or100Hz_Get(UINT8 ucCH)   //ZU860_Doulas_0104
{
    BOOL bRet = FALSE;

    if(halScaler_SemaphoreTake(__FUNCTION__) == TRUE)
    {
        bRet = dvC341_IsVsync120or100Hz_Get((eMCT)ucCH);

        halScaler_SemaphoreGive(__FUNCTION__);
    }

    return bRet;
}

BOOL halScaler_IsAspectRation16_9_Get(void)     //ZU860_Doulas_0119
{
    return dvC341_IsAspectRation16_9_Get();
}

BOOL halScaler_IsAspectRation16_10_Get(void)     //ZU860_Doulas_0120
{
    return dvC341_IsAspectRation16_10_Get();
}

eHAL_SCALER_EXEC_CODE halScaler_SourceOutputOff(UINT8 ucCH)     //ZU860_Doulas_0124
{
    //LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
    if(ucCH >= eMCT_LAST)
        return eHAL_SCALER_EXEC_CH_OVER_RANGE;

    if(halScaler_SemaphoreTake(__FUNCTION__) == TRUE)
    {
        if(!m_sHalScalerInfo[ucCH].bScalerInit)
        {
            //LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) : Please call halScaler_Init() before this call\r\n", __FUNCTION__, __LINE__);
            halScaler_SemaphoreGive(__FUNCTION__);
            return eHAL_SCALER_EXEC_CODE_NOT_INIT;
        }
        dvC341_SetInputPort_Output_Off((eMCT)ucCH);

        halScaler_SemaphoreGive(__FUNCTION__);

    }
    else
    {
        //LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore\r\n", __FUNCTION__, __LINE__);
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }

    return eHAL_SCALER_EXEC_CODE_PASS;
}

UINT8 halScaler_OutputVsyncFreqGetForTwistOn(void)     //A70LV_Doulas_0329
{
    UINT8 ucRet = 0;

    if(halScaler_SemaphoreTake(__FUNCTION__) == TRUE)
    {
        ucRet = dvC341_OutputVsyncFreqGetForTwistOn();

        halScaler_SemaphoreGive(__FUNCTION__);
    }

    return ucRet;
}

INT8 halScaler_TestDDR_Parameter(void)
{
	if(halScaler_SemaphoreTake(__FUNCTION__) == TRUE)
	{
		//LOG_MSG_ZO("dvC341_Read(B2_VBOTX1CTL, 0) = 0x%X\r\n", dvC341_Read(B2_VBOTX1CTL, 0));
		bkup_reg();
		dvC341_Write(B4_OSDCT,0x00,0);
		dvC341_Write(BN_RGBNK, 0xb2, 0);//bank 178

		check_wrlvlphct();
		check_gdsmsdly();

		halScaler_SemaphoreGive(__FUNCTION__);
	}
}


INT8 halScaler_TestDDR_Stress(void)
{
	if(halScaler_SemaphoreTake(__FUNCTION__) == TRUE)
	{
		//LOG_MSG_ZO("halScaler_TestDDR_Stress\r\n");
		bkup_reg();
		dvC341_Write(B4_OSDCT,0x00,0);
		dvC341_Write(BN_RGBNK, 0xb2, 0);//bank 178

		check_DDR_Stress();

		halScaler_SemaphoreGive(__FUNCTION__);
	}
}
INT8 halScaler_SPI_Stress(void)
{
	if(halScaler_SemaphoreTake(__FUNCTION__) == TRUE)
	{
		//LOG_MSG_ZO("halScaler_TestDDR_Stress\r\n");
		UINT8 data_write = 0;
		UINT8 data_write_burst[255];
		UINT8 data_read;
		UINT16 j = 0;
		#if 1
		//while(1)
		{
			for(j = 0 ; j <= 0xFF ; j++)
			{
				data_write = j;
				dvC341_Write(B18_MEMCHKDT_0, data_write, 0);
				data_read = dvC341_Read(B18_MEMCHKDT_0, 0);
				if(data_write != data_read)
				{
					LOG_MSG(db_ALWAYS, "SPI Fail ! 0x%X 0x%X\r\n", data_read, data_write);
				}
				else
				{
					//LOG_MSG(db_ALWAYS, "SPI Pass ! 0x%X 0x%X\r\n", data_read, data_write);
				}
			}
		}
		#else
		dvC341_Burst_test();
		#endif




		//LOG_MSG_ZO("Done\r\n");

		halScaler_SemaphoreGive(__FUNCTION__);
	}
}



#ifdef Low_Latency_All
eHAL_SCALER_EXEC_CODE halScaler_Low_Latency_Set(eLOW_LATENCY_MODE eLLMode)     //H2PF_Simon_0150   //H2PF_Simon_0187
{
    if(halScaler_SemaphoreTake(__FUNCTION__) == TRUE)
    {
        BOOL   bForceSyncReseVal;   //H30K_Doulas_0016
        UINT16 uiH_Toral;
        UINT16 uiV_Toral;

        LOG_MSG(db_HAL_SCALER, "Scaler eLLMode %d\r\n", eLLMode);

        if(eLLMode >= eLLM_INVALID)   //H2PF_Simon_0187
        {
            eLLMode = eLLM_OFF;
        }

        m_sUserSystemSetting.sCommonSetting.ucLowLatency = (UINT8)(eLLMode);

        dvC341_Low_Latency_Set(eMCT_CH1, !!m_sUserSystemSetting.sCommonSetting.ucLowLatency);
        dvC341_Low_Latency_Set(eMCT_CH2, !!m_sUserSystemSetting.sCommonSetting.ucLowLatency);

        dvC341_Set_Flip(eMCT_CH1);
        dvC341_Set_Flip(eMCT_CH2);
        dvC341Geo_Low_Latency_Set(eLLMode);  //H30K_Doulas_0010

        if(eLLMode == eLLM_LV3_FAST_WITHOUT_WARPING)
        {
            dvc341Geo_BypassModeSet(TRUE);
        }
        else
        {
            dvc341Geo_BypassModeSet(FALSE);
            dvC341_ForceSyncReset_Get(&bForceSyncReseVal,&uiH_Toral,&uiV_Toral);       //H30K_Doulas_0016
            dvC341Geo_ForcedSyncResetValueSet(bForceSyncReseVal,uiH_Toral,uiV_Toral); //H30K_Doulas_0016
        }

        halScaler_SemaphoreGive(__FUNCTION__);
    }

    return eHAL_SCALER_EXEC_CODE_PASS;
}

eLOW_LATENCY_MODE halScaler_Low_Latency_Get(void)   //H2PF_Simon_0187
{
	return (eLOW_LATENCY_MODE)m_sUserSystemSetting.sCommonSetting.ucLowLatency;     //H2PF_Simon_0150
}

UINT8 halScaler_Is_LowLatencyMode_On(void)   //H2PF_Simon_0187
{
    if((eLOW_LATENCY_MODE)m_sUserSystemSetting.sCommonSetting.ucLowLatency > eLLM_OFF &&
       (eLOW_LATENCY_MODE)m_sUserSystemSetting.sCommonSetting.ucLowLatency < eLLM_INVALID)
    {
        return TRUE;
    }

    return FALSE;
}




#endif	/*Low_Latency_All*/
//G100_Steven_0016 start
UINT8 halScaler_ColorSetting_Get(eHAL_COLOR_SETTING  ucNode, UINT8 ucCH) //test
{
    UINT8  ucInputSource = halScaler_InputSource_Get(eMCT_CH1);     //G100_Steven_0079 //ZU860_Doulas_0082 modify
    UINT8  ucPresetMode = halScaler_PictureSettings_Get(eMCT_CH1);  //G100_Steven_0079 //ZU860_Doulas_0082 modify
    UINT8  ucSignalType = halScaler_SignalType_Get(eMCT_CH1);       //G100_Steven_0079 //ZU860_Doulas_0082 modify
    UINT8  ucPresetUserMode = halScaler_PictureSettings_PreUser_Get(eMCT_CH1); //G100_Steven_0079
    UINT8  ucValue;
    UINT8  ucInputSub = halScaler_InputSource_Get(eMCT_CH2);

    //LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): ColorSpace Get=%d\r\n", __FUNCTION__, __LINE__, ucSetting); //A70LV_Doulas_0241 remove
	//LOG_MSG(db_ALWAYS, "(mode:%d, mode:%d) input %d\r\n",ucPresetMode,ucPresetUserMode,ucInputSource);

    if(eMCT_CH1 != ucCH) //G100_Steven_0079
    {
    	ucInputSource = ucInputSub;
    	if((ucNode == eHAL_COLOR_SETTING_CS) ||
    	    (ucNode == eHAL_COLOR_SETTING_BRIGHTNESS) ||
    	    (ucNode == eHAL_COLOR_SETTING_CONTRAST))  //A35G2_CDS_Coda_0024
    	{
            ucPresetMode = halScaler_PictureSettings_Get(ucCH);  //A35G2_CDS_Coda_0024
            ucPresetUserMode = halScaler_PictureSettings_PreUser_Get(ucCH); //A35G2_CDS_Coda_0024
            ucSignalType = halScaler_SignalType_Get(ucCH);       //A35G2_CDS_Coda_0024
        }
    }


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
}//G100_Steven_0016 end

UINT32 halScaler_Reg0_Read_Get(void)
{
    UINT32 ulRet = 0;

    if(halScaler_SemaphoreTake(__FUNCTION__) == TRUE)
    {
        ulRet = dvC341_Reg0_Read_Get();

        halScaler_SemaphoreGive(__FUNCTION__);
    }

    return ulRet;
}

UINT8 halScaler_OPD_Test(UINT8 ucData)  //G100_Owen_0137
{
    return 0;

#if 0
    UINT8 ucRet = 0;

    if(halScaler_SemaphoreTake(__FUNCTION__) == TRUE)
    {
        dvC341_Write(B1_TESTG, ucData, 0);
        MS_SLEEP(5);
        ucRet = (dvC341_Read(B1_TESTG, 0) & 0xFF);

        halScaler_SemaphoreGive(__FUNCTION__);
    }

    return ucRet;
#endif
}

eHAL_SCALER_EXEC_CODE halScaler_CaptureImage(void)
{
#if 0
    UINT32 ulOFLDAddr = 0;  //G100_Owen_0060

    if(halScaler_SemaphoreTake(__FUNCTION__) == TRUE)
    {
        UINT32 ulOFLDReg[4] = {B13_OSFLD0CH1,B13_OSFLD1CH1,B13_OSFLD2CH1,B13_OSFLD3CH1};
        UINT32 ulOFLDIdx = 0;

        dvC341_InputImage_Idle_Set(0, TRUE);
        dvC341_InputImage_Idle_Set(1, TRUE);
        dvC341_Freeze_Set(0, TRUE);
        dvC341_Freeze_Set(1, TRUE); //G100_Coda_0097

        dvC341_SerialFlash_Erase_Sector((DEF_LOGOREG_ADDR & 0xFFFF0000), 0x10000);      //G100_Owen_0072 : Erase register table first
        dvC341_SerialFlash_Erase_Sector(DEF_LOGOSFL_ADDR, DEF_LOGOSFL_SIZE);            //G100_Owen_0051
        ulOFLDIdx = (dvC341_Read(B35_STATECH1, 0) & 0x03);
        LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) ulOFLDIdx[%d]\n", __FUNCTION__, __LINE__, ulOFLDIdx);
        ulOFLDAddr = dvC341_Read(ulOFLDReg[ulOFLDIdx], 0);
        OSD_DMA_DDR3_WriteTo_Flash(ulOFLDAddr, DEF_LOGOSFL_ADDR, 0, DEF_LOGODDR_SIZE);  //G100_Owen_0051
        dvC341_RegTbl_to_Flash(DEF_LOGOREG_ADDR, m_ulCapturedReg, sizeof(m_ulCapturedReg)/sizeof(UINT32));  //G100_Owen_0060
        dvC341_Freeze_Set(0, FALSE);
        dvC341_Freeze_Set(1, FALSE); //G100_Coda_0097
        dvC341_InputImage_Idle_Set(0, FALSE);
        dvC341_InputImage_Idle_Set(1, FALSE);
        m_ucLogo_Load = FALSE;

        LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d)\n", __FUNCTION__, __LINE__);
        halScaler_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        //LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore\r\n", __FUNCTION__, __LINE__);
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }
#endif

    return eHAL_SCALER_EXEC_CODE_PASS;
}

eHAL_SCALER_EXEC_CODE halScaler_CaptureImage_Show(BOOL bEnable)    //G100_Owen_0048
{
#if 0
    if(halScaler_SemaphoreTake(__FUNCTION__) == TRUE)
    {
        if(bEnable)
        {
            UINT8 aucData[2] = {0xFF};

            dvC341_FlashRead(DEF_LOGOREG_ADDR, 2, aucData);
            if((aucData[0] == 0) && (aucData[1] == 0))  //G100_Owen_0072
            {
                if(!m_ucLogo_Load)
                {
                    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d)\n", __FUNCTION__, __LINE__);
                    OSD_DMA_SerialFlash_Set(DEF_LOGOSFL_ADDR, DEF_ISFLD0CH3_2K, 0, DEF_LOGODDR_SIZE);   //G100_Owen_0051
                    m_ucLogo_Load = TRUE;
                }

                dvC341_DMA_Transfer(DEF_LOGOREG_ADDR, 130, 0, 0xA); //G100_Owen_0060
                dvC341_CapturedIMG_Output_Set(TRUE);
                dvC341_Channel1_Auto_Fill_Screen_Set(FALSE);
            }
            else
            {
                dvC341_CapturedIMG_Output_Set(FALSE);
                dvC341_Auto_Fill_Screen_Init();
                LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) read reg tbl failed!\n", __FUNCTION__, __LINE__);
            }
        }
        else
        {
            dvC341_CapturedIMG_Output_Set(FALSE);
            dvC341_Auto_Fill_Screen_Init(); //G100_Owen_0059
        }
        halScaler_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        //LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore\r\n", __FUNCTION__, __LINE__);
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }
    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d)\n", __FUNCTION__, __LINE__);
#endif

    return eHAL_SCALER_EXEC_CODE_PASS;
}

eHAL_SCALER_EXEC_CODE halScaler_CaptureImage_Del(BOOL bSourceLock)    //G100_Owen_0048
{
#if 0
    if(halScaler_SemaphoreTake(__FUNCTION__) == TRUE)
    {
        dvC341_SerialFlash_Erase_Sector((DEF_LOGOREG_ADDR & 0xFFFF0000), 0x10000);  //G100_Owen_0059
        dvC341_CapturedIMG_Reset(DEF_LOGOREG_ADDR, m_ulCapturedReg, sizeof(m_ulCapturedReg)/sizeof(UINT32));    //G100_Owen_0060

        if(!bSourceLock)    //G100_Owen_0058
        {
            dvC341_CapturedIMG_Output_Set(FALSE);
            dvC341_Auto_Fill_Screen_Init(); //G100_Owen_0059
        }

        halScaler_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        //LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d) Can not get semphore\r\n", __FUNCTION__, __LINE__);
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }
#endif

    return eHAL_SCALER_EXEC_CODE_PASS;
}


eHAL_SCALER_EXEC_CODE halScaler_ScreenSaveToImage(void)
{// R70G2_Bruce#0022
    INT16 xpos = 0, ypos = 0, width = 1920, height = 1200;
    UINT8 ret = 0;
#if 1
    if(halWarping_GetFreezeImageState() == FALSE)
    {
        halWarping_FreezeImage(TRUE);
    }

    ret = halC789Ctrl_ScreenSaveToImage(xpos, ypos, width, height);

    if(halWarping_GetFreezeImageState() == TRUE)
    {
        halWarping_FreezeImage(FALSE);
    }

    //halC789Ctrl_ScreenFreeze();// R70G2_Bruce#0022
    //ret = halC789Ctrl_ScreenSaveToImage(xpos, ypos, width, height);
    //halC789Ctrl_ScreenUnFreeze();// R70G2_Bruce#0022
#else
    if(halScaler_SemaphoreTake(__FUNCTION__) == TRUE)
    {
        LOG_MSG(db_HAL_RESERVED17, "%s() pass.\n", __FUNCTION__);
        halC789Ctrl_ScreenFreeze();// R70G2_Bruce#0022
        ret = halC789Ctrl_ScreenSaveToImage(xpos, ypos, width, height);
        halC789Ctrl_ScreenUnFreeze();// R70G2_Bruce#0022

        halScaler_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        LOG_MSG(db_HAL_RESERVED17, "%s() fail !!!\n", __FUNCTION__);
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }
#endif
    if (ret == 0)
        return eHAL_SCALER_EXEC_CODE_FAIL;

    return eHAL_SCALER_EXEC_CODE_PASS;
}

eHAL_SCALER_EXEC_CODE halScaler_C341_Auto_Fill_Screen_Init(void) //G100_Steven_0051 start
{
    if(halScaler_SemaphoreTake(__FUNCTION__) == TRUE)
    {
    	dvC341_Auto_Fill_Screen_Init();
        halScaler_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }
    return eHAL_SCALER_EXEC_CODE_PASS;
}

eHAL_SCALER_EXEC_CODE halScaler_C341_Auto_Fill_Screen_Setting(BOOL bSetting)
{
    if(halScaler_SemaphoreTake(__FUNCTION__) == TRUE)
    {
    	dvC341_Auto_Fill_Screen_Setting(bSetting);
        halScaler_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }
    return eHAL_SCALER_EXEC_CODE_PASS;
} //G100_Steven_0051 end

void halScaler_Test(UINT16 *puiTtlCnt, UINT16 *puiErrCnt)   //G100_Owen_0080
{
    if(halScaler_SemaphoreTake(__FUNCTION__) == TRUE)
    {
        dvC341_Cmd_Test(puiTtlCnt, puiErrCnt);

        halScaler_SemaphoreGive(__FUNCTION__);
    }
}

eHAL_SCALER_EXEC_CODE halScaler_SizePresets_SetOnlyValue(UINT8 ucSetting)
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;
    UINT8 ucInputSource;

    if ( ucSetting >= eCM_SCALING_MODE_NUMBER)// || ucSetting < eSCALING_MODE_AUTO )
        return eHAL_SCALER_EXEC_CH_OVER_RANGE;    //A70LV_Doulas_0072 modify

    LOG_MSG(db_HAL_SCALER, "(func:%s, line:%d): OverScan=%d\r\n", __FUNCTION__, __LINE__, ucSetting);

    ucInputSource = halScaler_InputSource_Get(eMCT_CH1);
    m_sUserSystemSetting.sSourceDependSetting[ucInputSource].ucAspectRatio = ucSetting;

    return eResult;
}

void halScaler_PIPPBPPaniel_InfoSet(BYTE cCH, UINT16 uiHActive, UINT16 uiVActive, UINT16 uiHStart, UINT16 uiVStart) //A35G2_CDS_Larry_0008
{
    sSCALER_PANEL_INFO sPanelInfo;

    sPanelInfo.uiHActive = uiHActive;
    sPanelInfo.uiVActive = uiVActive;
    sPanelInfo.uiHStart = uiHStart;
    sPanelInfo.uiVStart = uiVStart;

    dvC341_PIPPBPPanelInfo_Setting(cCH, &sPanelInfo);

}

void halScaler_PIPPBPPaniel_InfoGet(BYTE cCH, sSCALER_PANEL_INFO *psInfo)
{
    dvC341_PIPPBPPanelInfo_Get(cCH, psInfo);
}

UINT8 halScaler_PowerNormalReadyGet(BYTE ucCH)  //A35G2_CDS_Simon_0034
{
    return (UINT8)m_sHalScalerInfo[ucCH].bScalerPowerNormalReady ;
}

void halScaler_3D_Reset(void)  //G100_Steven_0109, fixed 0016205 //A35G2_BRC_Casper_0048
{
	UINT8 ucCount = 0, uc3D_Enable = 0, uc3D_Mode = 0;
	UINT8 ucInputSource = halScaler_InputSource_Get(eMCT_CH1);

	uc3D_Enable =  m_sUserSystemSetting.sSourceDependSetting[ucInputSource].uc3D_Enable;
	uc3D_Mode =  m_sUserSystemSetting.sSourceDependSetting[ucInputSource].uc3D_Mode;

	for(ucCount = 0; ucCount < INPUT_SOURCE_MAX ; ucCount++)
	{
		m_sUserSystemSetting.sSourceDependSetting[ucCount].uc3D_Enable = uc3D_Enable;
		m_sUserSystemSetting.sSourceDependSetting[ucCount].uc3D_Mode = uc3D_Mode;
	}
}

UINT32 halScaler_ICP_Register_Read(UINT32 ulAddr) //A65_OPTOMA_Julie_0050
{
	UINT32 ulData = 0;

    if(halScaler_SemaphoreTake(__FUNCTION__) == TRUE)
	{
		ulData = dvC341_Read(ulAddr, 0);

		halScaler_SemaphoreGive(__FUNCTION__);
	}

	return ulData;
}

void halScaler_ICP_Register_Write(UINT32 ulAddr, UINT32 ucData) //A65_OPTOMA_Julie_0050
{
    if(halScaler_SemaphoreTake(__FUNCTION__) == TRUE)
	{
		dvC341_Write(ulAddr, (UINT32)ucData, 0);

		halScaler_SemaphoreGive(__FUNCTION__);
	}
}

eHAL_SCALER_EXEC_CODE halScaler_ClockGen_Init(eCLOCKGEN_SEL eClockgenSel)
{
    eVCXO_SEL eCDEC949_Sel;

#ifdef INIT_VCXO949

    switch(eClockgenSel)
    {
        case eCLOCKGEN_SEL_C734:                eCDEC949_Sel = eVCXO_SEL_C734;              break;
        case eCLOCKGEN_SEL_C790:                eCDEC949_Sel = eVCXO_SEL_C790;              break;
        case eCLOCKGEN_SEL_C341_HD:             eCDEC949_Sel = eVCXO_SEL_C341_HD;           break;
        case eCLOCKGEN_SEL_C341_WU:             eCDEC949_Sel = eVCXO_SEL_C341_WU;           break;
        case eCLOCKGEN_SEL_C341_720P:           eCDEC949_Sel = eVCXO_SEL_C341_720P;         break;
        case eCLOCKGEN_SEL_C341_720P120:        eCDEC949_Sel = eVCXO_SEL_C341_720P120;      break;
        case eCLOCKGEN_SEL_C341_800x600_120:    eCDEC949_Sel = eVCXO_SEL_C341_800x600_120;  break;
        case eCLOCKGEN_SEL_C341_1080P120:       eCDEC949_Sel = eVCXO_SEL_C341_1080P120;     break;
        case eCLOCKGEN_SEL_C341_WUXGA120:       eCDEC949_Sel = eVCXO_SEL_C341_WUXGA120;     break;

        default:
            LOG_MSG(db_HAL_SCALER, "unknown Clock Gen Sel %d\n", eClockgenSel);
            return eHAL_SCALER_EXEC_CODE_FAIL;
    }

    if(dvVCXO949_Init(eCDEC949_Sel) != eVCXO_EXEC_CODE_PASS)
    {
        LOG_MSG(db_HAL_SCALER, "CDEC949 init fail %d\n", eCDEC949_Sel);
        return eHAL_SCALER_EXEC_CODE_FAIL;
    }

#endif

    return eHAL_SCALER_EXEC_CODE_PASS;
}

void halScaler_PowerNormalDone(BOOL bDone)  //R70G2_Donview_Sammy_0040
{
    dvC341_PowerNormalDone(bDone);
}


void halScaler_AutoSourceResyncSet(UINT8 ucAutoSourceResync)
{
    m_sUserSystemSetting.sCommonSetting.ucAutoSourceResync = ucAutoSourceResync;
}

UINT8 halScaler_AutoSourceResyncGet(void)
{
    return m_sUserSystemSetting.sCommonSetting.ucAutoSourceResync;
}

eHAL_SCALER_EXEC_CODE halScaler_HV_Total_Get(BOOL *bForceSyncReseVal,UINT16 *uiH_Toral,UINT16 *uiV_Toral) //H30K_Doulas_0003
{
    eHAL_SCALER_EXEC_CODE eResult = eHAL_SCALER_EXEC_CODE_PASS;

    dvC341_ForceSyncReset_Get(bForceSyncReseVal, uiH_Toral, uiV_Toral);

    return eResult;
}

void halScaler_FrontEnd_Timing_Set(UINT8 ucCH ,sVIDEO_TIMING sFrontEnd_Timing) //H30K_Doulas_0011
{
    sVIDEO_TIMING2 sFrontEnd_Timing2;
    //memcpy(&sFrontEnd_Timing2, &sFrontEnd_Timing, sizeof(sVIDEO_TIMING));
    sFrontEnd_Timing2.u16VideoHActive  = sFrontEnd_Timing.u16VideoHActive;
    sFrontEnd_Timing2.u16VideoHTotal   = sFrontEnd_Timing.u16VideoHTotal;
    sFrontEnd_Timing2.u16VideoVActive  = sFrontEnd_Timing.u16VideoVActive;
    sFrontEnd_Timing2.u16VideoVRate    = sFrontEnd_Timing.u16VideoVRate;
    sFrontEnd_Timing2.u16VideoVTotal   = sFrontEnd_Timing.u16VideoVTotal;
    sFrontEnd_Timing2.u32VideoPCLK     = sFrontEnd_Timing.u32VideoPCLK;
    dvC341_FrontEnd_Timing_Set(ucCH,sFrontEnd_Timing2);
}

eHAL_SCALER_EXEC_CODE halScaler_STATECH1(void) //H30K_Doulas_0035  //A35G2_Simon_0121 only for debug
{
    if(halScaler_SemaphoreTake(__FUNCTION__) == TRUE)
	{
        UINT32 OSYCT = dvC341_Read(B8_OSYCTCH1, 0);
        LOG_MSG(db_ALWAYS, "OSYCT 0x%04X\r\n", OSYCT);

        dvC341_wait1_pivs(0); //H30K_Doulas_0037
        UINT32 ofld = (dvC341_Read(B171_STATECH1, 0) >> 1) & 0x01;
        LOG_MSG(db_ALWAYS, "ofld %d\r\n", ofld);

		halScaler_SemaphoreGive(__FUNCTION__);
	}
	else
	{
        return eHAL_SCALER_EXEC_CODE_FAIL;
	}

    return eHAL_SCALER_EXEC_CODE_PASS;
}

//only for side-by-side and top-bottom format (input 60Hz or 50Hz)
eHAL_SCALER_EXEC_CODE halScaler_3DFLD_Check(UINT8 *ucIsReconfig)   //H30K_Doulas_0035//A35G2_Simon_0121
{
    if(halScaler_SemaphoreTake(__FUNCTION__) == TRUE)
	{
	    UINT16 uiV_Freq = 0; //H30K_Doulas_0038
        halScaler_InputVertRefresh2_Get(eSOURCE_WINDOW_MAIN,&uiV_Freq); //H30K_Doulas_0038
        if((((uiV_Freq > 5960) && (uiV_Freq < 6030)) || ((uiV_Freq > 4960) && (uiV_Freq < 5030))) &&  // 50/60hz
            (halScaler_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_TOPANDBOTTOM ||
             halScaler_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_FRAMEPACKING ||
             halScaler_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_SIDEBYSIDE)
        ) //H30K_Doulas_0038 Modify
        {
            dvC341_3DPOFLD_Check(ucIsReconfig);
        }
        else
        {
            *ucIsReconfig = FALSE;
        }

		halScaler_SemaphoreGive(__FUNCTION__);
	}
	else
	{
        return eHAL_SCALER_EXEC_CODE_FAIL;
	}

    return eHAL_SCALER_EXEC_CODE_PASS;
}


UINT8 halScaler_Check_WPOFV(void)   //H2PF_Simon_0187
{
    if(halScaler_SemaphoreTake(__FUNCTION__) == TRUE)
	{
		UINT32 WPOFVCH1 = dvC341_Read(B179_WPOFVCH1, 0);
		UINT32 WPOFVCH2 = dvC341_Read(B179_WPOFVCH2, 0);

        if(WPOFVCH1 > 2600 || WPOFVCH2 > 2600)
        {
            LOG_MSG(db_ALWAYS, "<%d %d>\r\n", WPOFVCH1, WPOFVCH2);
		    halScaler_SemaphoreGive(__FUNCTION__);
            return TRUE;
        }

		halScaler_SemaphoreGive(__FUNCTION__);
        return FALSE;
	}
}

//H2PF_Simon_0193 Start
eHAL_SCALER_EXEC_CODE halScaler_HLG_Degamma(int sdr_max, int hlg_max)
{
    if(halScaler_SemaphoreTake(__FUNCTION__) == TRUE)
	{
        dvC341_degamma_HLG(sdr_max, hlg_max);

		halScaler_SemaphoreGive(__FUNCTION__);
	}
    else
    {
        return eHAL_SCALER_EXEC_CODE_MUTEX_LOCKED;
    }

    return eHAL_SCALER_EXEC_CODE_PASS;
}

eHAL_SCALER_EXEC_CODE halScaler_HLG_y_lut(int hlg_max)
{
    if(halScaler_SemaphoreTake(__FUNCTION__) == TRUE)
	{
        dvC341_HLG_y_lut(hlg_max);

		halScaler_SemaphoreGive(__FUNCTION__);
	}
    else
    {
        return eHAL_SCALER_EXEC_CODE_MUTEX_LOCKED;
    }

    return eHAL_SCALER_EXEC_CODE_PASS;
}

eHAL_SCALER_EXEC_CODE halScaler_Panelgamma_BT709_Gamma22(void)
{
    if(halScaler_SemaphoreTake(__FUNCTION__) == TRUE)
	{
        dvC341_Panelgamma_BT709_Gamma22();

		halScaler_SemaphoreGive(__FUNCTION__);
	}
    else
    {
        return eHAL_SCALER_EXEC_CODE_MUTEX_LOCKED;
    }

    return eHAL_SCALER_EXEC_CODE_PASS;
}

eHAL_SCALER_EXEC_CODE halScaler_Panelgamma_BT709_Gamma22_OnlyCH1(void)
{
    if(halScaler_SemaphoreTake(__FUNCTION__) == TRUE)
	{
        dvC341_Panelgamma_BT709_Gamma22_OnlyCH1();

		halScaler_SemaphoreGive(__FUNCTION__);
	}
    else
    {
        return eHAL_SCALER_EXEC_CODE_MUTEX_LOCKED;
    }

    return eHAL_SCALER_EXEC_CODE_PASS;
}

eHAL_SCALER_EXEC_CODE halScaler_HDR10_Degamma_PQ(UINT16 uiHDRContentMaxNit)
{
    if(halScaler_SemaphoreTake(__FUNCTION__) == TRUE)
	{
        LOG_MSG(db_APP_DATAPATH, "%s <HDRContentMaxNit %d>\r\n", __func__, uiHDRContentMaxNit);

        dvC341_HDR_degamma_PQ(uiHDRContentMaxNit);

		halScaler_SemaphoreGive(__FUNCTION__);
	}
    else
    {
        return eHAL_SCALER_EXEC_CODE_MUTEX_LOCKED;
    }

    return eHAL_SCALER_EXEC_CODE_PASS;
}

eHAL_SCALER_EXEC_CODE halScaler_HDR10_Process(UINT16 uiHDRContentMaxNit)
{
    if(halScaler_SemaphoreTake(__FUNCTION__) == TRUE)
	{
        LOG_MSG(db_APP_DATAPATH, "%s <HDRContentMaxNit %d>\r\n", __func__, uiHDRContentMaxNit);

        dvC341_HDR_Disable();
        dvC341_HDR_degamma_PQ(uiHDRContentMaxNit);
        dvC341_Panelgamma_BT709_Gamma22();

        dvC341_GamutBT2020ToREC709();

        #if 0
        UINT8 ucColorSpace = 0 ;
        halScaler_FrontEndColorSpace_Get(eSOURCE_WINDOW_MAIN,&ucColorSpace);
        if(ucColorSpace == eCOLOR_FORMAT_RGB_LIMIT || ucColorSpace == eCOLOR_FORMAT_RGB )
        {
            GamutRGBBT2020ToREC709();
        }
        else
        {
            dvC341_GamutBT2020ToREC709();
        }
        #endif

		halScaler_SemaphoreGive(__FUNCTION__);
	}
    else
    {
        return eHAL_SCALER_EXEC_CODE_MUTEX_LOCKED;
    }

    return eHAL_SCALER_EXEC_CODE_PASS;
}

eHAL_SCALER_EXEC_CODE halScaler_HLG_Process(UINT16 uiMonitorMaxNit)
{
    if(halScaler_SemaphoreTake(__FUNCTION__) == TRUE)
	{
        dvC341_HDR_Disable();
        dvC341_degamma_HLG(1000, 1000);
        dvC341_HLG_y_lut(uiMonitorMaxNit);
        dvC341_HLG_BT2020();
        dvC341_Panelgamma_BT709_Gamma22();
        dvC341_GamutBT2020ToREC709();

		halScaler_SemaphoreGive(__FUNCTION__);
	}
    else
    {
        return eHAL_SCALER_EXEC_CODE_MUTEX_LOCKED;
    }

    return eHAL_SCALER_EXEC_CODE_PASS;
}

eHAL_SCALER_EXEC_CODE halScaler_SDREnhance_Process(void)
{
    if(halScaler_SemaphoreTake(__FUNCTION__) == TRUE)
	{
        dvC341_degamma_SDREnhance(10000);
        dvC341_Panelgamma_BT709_Gamma22();

		halScaler_SemaphoreGive(__FUNCTION__);
	}
    else
    {
        return eHAL_SCALER_EXEC_CODE_MUTEX_LOCKED;
    }

    return eHAL_SCALER_EXEC_CODE_PASS;
}

eHAL_SCALER_EXEC_CODE halScaler_HDR_Disable(void)
{
    if(halScaler_SemaphoreTake(__FUNCTION__) == TRUE)
	{
        dvC341_HDR_Disable();

		halScaler_SemaphoreGive(__FUNCTION__);
	}
    else
    {
        return eHAL_SCALER_EXEC_CODE_MUTEX_LOCKED;
    }

    return eHAL_SCALER_EXEC_CODE_PASS;

}

eHAL_SCALER_EXEC_CODE halScaler_SDREnhance(UINT32 Value)
{
    if(halScaler_SemaphoreTake(__FUNCTION__) == TRUE)
	{
        dvC341_degamma_SDREnhance(Value);

		halScaler_SemaphoreGive(__FUNCTION__);
	}
    else
    {
        return eHAL_SCALER_EXEC_CODE_MUTEX_LOCKED;
    }

    return eHAL_SCALER_EXEC_CODE_PASS;
}

eHAL_SCALER_EXEC_CODE halScaler_GamutBT2020ToREC709(void)
{
    if(halScaler_SemaphoreTake(__FUNCTION__) == TRUE)
	{
        dvC341_GamutBT2020ToREC709();

		halScaler_SemaphoreGive(__FUNCTION__);
	}
    else
    {
        return eHAL_SCALER_EXEC_CODE_MUTEX_LOCKED;
    }

    return eHAL_SCALER_EXEC_CODE_PASS;
}

eHAL_SCALER_EXEC_CODE halScaler_GamutB2020ToREC709_OnlyCH1(void)
{
    if(halScaler_SemaphoreTake(__FUNCTION__) == TRUE)
	{
        dvC341_GamutBT2020ToREC709_OnlyCH1();

		halScaler_SemaphoreGive(__FUNCTION__);
	}
    else
    {
        return eHAL_SCALER_EXEC_CODE_MUTEX_LOCKED;
    }

    return eHAL_SCALER_EXEC_CODE_PASS;
}

eHAL_SCALER_EXEC_CODE halScaler_HLG_BT2020_OnlyCH1(void)
{
    if(halScaler_SemaphoreTake(__FUNCTION__) == TRUE)
	{
        dvC341_HLG_BT2020_OnlyCH1();

		halScaler_SemaphoreGive(__FUNCTION__);
	}
    else
    {
        return eHAL_SCALER_EXEC_CODE_MUTEX_LOCKED;
    }

    return eHAL_SCALER_EXEC_CODE_PASS;
}

eHAL_SCALER_EXEC_CODE halScaler_HDR_Reset(void)
{
    if(halScaler_SemaphoreTake(__FUNCTION__) == TRUE)
	{
        dvC341_HDR_Init();
        dvC341_HDR_Disable();

		halScaler_SemaphoreGive(__FUNCTION__);
	}
    else
    {
        return eHAL_SCALER_EXEC_CODE_MUTEX_LOCKED;
    }

    return eHAL_SCALER_EXEC_CODE_PASS;
}

//H2PF_Simon_0193 End
