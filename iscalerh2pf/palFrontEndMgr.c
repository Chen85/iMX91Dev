#include "Common.h"
#include "halFrontEndCtrlAPI.h"
#include "palFrontEndMgr.h"
#include "utilCLICmdAPI.h"
#include "utilDbgMsg.h"
#include "utilHostAPI.h"
#include "utilOPD_TEST.h"

static UINT8 m_cMainSource = 0xFF;
static UINT8 m_cSubSource = 0xFF;

void palFrontEndCtrl_FocusResync(void)
{
    m_cMainSource = 0xFF;
    m_cSubSource = 0xFF;
}

eRESULT palFrontEndMgr_Version_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halFrontEndCtrl_Version_Get(pcData);

    return eResult;
}

eRESULT palFrontEndMgr_HDBaseTVersion_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halFrontEndCtrl_HDBaseTVersion_Get(pcData);

    return eResult;
}


eRESULT palFrontEndMgr_videoReady_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halFrontEndCtrl_videoReady_Get(pcData);

    return eResult;
}


eRESULT palFrontEndMgr_BackupInput_videoReady_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halFrontEndCtrl_BackupInput_videoReady_Get(pcData);

    return eResult;
}


eRESULT palFrontEndMgr_videoFormat_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halFrontEndCtrl_videoFormat_Get(pcData);

    return eResult;
}


eRESULT palFrontEndMgr_OPDInfo_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halFrontEndCtrl_OPDInfo_Get(pcData);

    return eResult;
}


eRESULT palFrontEndMgr_MainInput_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halFrontEndCtrl_MainInput_Get(pcData);

    return eResult;
}

eRESULT palFrontEndMgr_MainInput_Set(UINT8* pcData, BOOL bCheck, UINT8 cResetSource)
{
    UINT8 cSource = pcData[0];
    eRESULT eResult = rcSUCCESS;

    if((m_cMainSource != cSource) || cResetSource)
    {
        eResult = halFrontEndCtrl_MainInput_Set(&cSource);//, bCheck, cResetSource);

        if(bCheck)
        {
            UINT8 cTemp = 0xff;
            UINT8 cCount = 10;

            MS_SLEEP(20);

            do
            {
                if(rcSUCCESS == halFrontEndCtrl_MainInput_Get(&cTemp))
                {
                    MS_SLEEP(20);
                }
                else
                {
                    ASSERT_ALWAYS();
                }
            }while((cCount--) && (cTemp != cSource));

            m_cMainSource = cSource;

            //LOG_MSG(db_HAL_SOURCE_INFO, "Assert dvFrontEndDriverRegRead timeout\r\n");
        }
        else
        {
            m_cMainSource = 0xFF;
        }
    }

    return eResult;
}


eRESULT palFrontEndMgr_SubInput_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halFrontEndCtrl_SubInput_Get(pcData);

    return eResult;
}

eRESULT palFrontEndMgr_SubInput_Set(UINT8* pcData, BOOL bCheck, UINT8 cResetSource)
{
    UINT8 cSource = pcData[0];
    eRESULT eResult = rcSUCCESS;

    if((m_cSubSource != cSource) || cResetSource)
    {
        eResult = halFrontEndCtrl_SubInput_Set(&cSource);// bCheck, cResetSource);

        if(bCheck)
        {
            UINT8 cTemp = 0xff;
            UINT8 cCount = 10;

            MS_SLEEP(20);

            do
            {
                if(rcSUCCESS == halFrontEndCtrl_SubInput_Get(&cTemp))
                {
                    MS_SLEEP(20);
                }
                else
                {
                    ASSERT_ALWAYS();
                }
            }while((cCount--) && (cTemp != cSource));

            m_cSubSource = cSource;
        }
        else
        {
            m_cSubSource = 0xFF;
        }
    }

    return eResult;
}

eRESULT palFrontEndMgr_Backup_AutoSW_Set(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    #if (SCALER_TYPE == FPGA_F34)
    {
        return rcSUCCESS;
    }
    #else
    {
        eResult = halFrontEndCtrl_Backup_AutoSW_Set(pcData);
    }
    #endif

    return eResult;
}


eRESULT palFrontEndMgr_Backup_First_Set(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    #if (SCALER_TYPE == FPGA_F34)
    {
        return rcSUCCESS;
    }
    #else
    {
        eResult = halFrontEndCtrl_Backup_First_Set(pcData);
    }
    #endif

    return eResult;
}

eRESULT palFrontEndMgr_Backup_Second_Set(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    #if (SCALER_TYPE == FPGA_F34)
    {
        return rcSUCCESS;
    }
    #else
    {
        eResult = halFrontEndCtrl_Backup_Second_Set(pcData);
    }
    #endif

    return eResult;
}

eRESULT palFrontEndMgr_Backup_Status_Get(UINT8* pcData) //bit0:Active ; bit1:SWAP
{
    eRESULT eResult = rcSUCCESS;

    #if (SCALER_TYPE == FPGA_F34)
    {
        return rcSUCCESS;
    }
    #else
    {
        eResult = halFrontEndCtrl_Backup_Status_Get(pcData);
    }
    #endif

    return eResult;
}

eRESULT palFrontEndMgr_BackupInput_ScalerTimingCheck_Set(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    #if (SCALER_TYPE == FPGA_F34)
    {
        return rcSUCCESS;
    }
    #else
    {
        eResult = halFrontEndCtrl_BackupInput_ScalerTimingCheck_Set(pcData);
    }
    #endif

    return eResult;
}

eRESULT palFrontEndMgr_HDMI_OUT_Set(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halFrontEndCtrl_HDMI_OUT_Set(pcData);

    return eResult;
}

eRESULT palFrontEndMgr_PIP_Enable_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halFrontEndCtrl_PIP_Enable_Get(pcData);

    return eResult;
}


eRESULT palFrontEndMgr_PIP_Enable_Set(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    UINT8 ucEnable = FALSE;
    if(*pcData)
    {
        ucEnable = TRUE;
    }

    eResult = halFrontEndCtrl_PIP_Enable_Set(&ucEnable);

    return eResult;
}


eRESULT palFrontEndMgr_INPUT_SOURCE_DETECT_Get(UINT16* uiData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halFrontEndCtrl_INPUT_SOURCE_DETECT_Get(uiData);

    return eResult;
}


eRESULT palFrontEndMgr_VGA_H_Total_Get(UINT16* uiData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halFrontEndCtrl_VGA_H_Total_Get(uiData);

    return eResult;
}

eRESULT palFrontEndMgr_VGA_H_Total_Set(UINT16* uiData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halFrontEndCtrl_VGA_H_Total_Set(uiData);

    return eResult;
}

eRESULT palFrontEndMgr_VGA_Phase_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halFrontEndCtrl_VGA_Phase_Get(pcData);

    return eResult;
}

eRESULT palFrontEndMgr_VGA_Phase_Set(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halFrontEndCtrl_VGA_Phase_Set(pcData);

    return eResult;
}


eRESULT palFrontEndMgr_videoFormatSub_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halFrontEndCtrl_videoFormatSub_Get(pcData);

    return eResult;
}

eRESULT palFrontEndMgr_VGA_Sync_Threshold_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halFrontEndCtrl_VGA_Sync_Threshold_Get(pcData);

    return eResult;
}

eRESULT palFrontEndMgr_VGA_Sync_Threshold_Set(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halFrontEndCtrl_VGA_Sync_Threshold_Set(pcData);

    return eResult;
}


eRESULT palFrontEndMgr_VGA_Pixel_Clock_Set(UINT32 udData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halFrontEndCtrl_VGA_Pixel_Clock_Set(udData);

    return eResult;
}

eRESULT palFrontEndMgr_VGA_Gain_Get(UINT16* uiRed,UINT16* uiGreen,UINT16* uiBlue)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halFrontEndCtrl_VGA_Gain_Get(uiRed, uiGreen, uiBlue);

    return eResult;
}


eRESULT palFrontEndMgr_VGA_Gain_Set(UINT16 uiRed,UINT16 uiGreen,UINT16 uiBlue)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halFrontEndCtrl_VGA_Gain_Set(uiRed, uiGreen, uiBlue);

    return eResult;
}

eRESULT palFrontEndMgr_VGA_Offset_Get(UINT16* uiRed,UINT16* uiGreen,UINT16* uiBlue)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halFrontEndCtrl_VGA_Offset_Get(uiRed, uiGreen, uiBlue);

    return eResult;
}


eRESULT palFrontEndMgr_VGA_Offset_Set(UINT16 uiRed,UINT16 uiGreen,UINT16 uiBlue)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halFrontEndCtrl_VGA_Offset_Set(uiRed, uiGreen, uiBlue);

    return eResult;
}

eRESULT palFrontEndMgr_FPGA_Version_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halFrontEndCtrl_FPGA_Version_Get(pcData);

    return eResult;
}


eRESULT palFrontEndMgr_Main_Timing_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halFrontEndCtrl_Main_Timing_Get(pcData);

    return eResult;
}


eRESULT palFrontEndMgr_Sub_Timing_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halFrontEndCtrl_Sub_Timing_Get(pcData);

    return eResult;
}

eRESULT palFrontEndMgr_EDID_SN_Set(UINT8 cDevice, UINT8 cType, eCM_NATIVE_TIMING eNativeTiming, UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;
    UINT8 cData[3 + SN_LENGTH] = {0};

    cData[0] = cDevice; // if eCM_SOURCE_RESERVED, set cType = 1 for default mode ( change on frontend)
    cData[1] = cType;
    cData[1] |= (UINT8)eNativeTiming << PROJECTION_NATIVE_TIMING_SHIFT;// HICC2_Bruce_0022
    cData[2] = Board_MCU_ModelID_Get(); //palDataMgr_Model_ID_Get();
    memcpy(&cData[3], pcData, SN_LENGTH);

    eResult = halFrontEndCtrl_EDID_SN_Set(cData);

    return eResult;
}


#if 0
// ==============================================================================
// FUNCTION NAME: palFrontEndMgr_SerialNumber_Set
// DESCRIPTION:
//
//
// Params:
// UINT8* pcData
//
// Returns:
//
//
// Modification History
// --------------------
// A70LV_Doulas_0200
// --------------------
// ==============================================================================
eRESULT palFrontEndMgr_SerialNumber_Set(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    #ifndef WORD_REG
    eResult = dvFrontEndDriverRegWrite(eFE_MSG_SERIALNUMBER_EDID, eFE_MSG_SERIALNUMBER_EDID_SZ, pcData);
    #else
    eResult = utilHost_FrontEndSet(eCMD_MODULE_FE, eFE_MSG_SERIALNUMBER_EDID, eFE_MSG_SERIALNUMBER_EDID_SZ, pcData);
    #endif

    return eResult;
}
#endif

eRESULT palFrontEndMgr_VGA_TIMING_Set(UINT16 uiH_Active,UINT16 uiV_Active,UINT16 uiV_Freq)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halFrontEndCtrl_VGA_TIMING_Set(uiH_Active, uiV_Active, uiV_Freq);

    return eResult;
}


#if 0
// ==============================================================================
// FUNCTION NAME: palFrontEndMgr_EDID_Upgrade_Set
// DESCRIPTION:
//
//
// Params:
// UINT8* pcData:
//
// Returns:
//
//
// Modification History
// --------------------
// 2018/07/27, Larry Create
// --------------------
// ==============================================================================
eRESULT palFrontEndMgr_EDID_Upgrade_Set(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    #ifndef WORD_REG
    eResult = dvFrontEndDriverRegWrite(eFE_MSG_EDID_UPGRADE, eFE_MSG_EDID_UPGRADE_SZ, pcData);
    #else
    eResult = utilHost_FrontEndSet(eCMD_MODULE_FE, eFE_MSG_EDID_UPGRADE, eFE_MSG_EDID_UPGRADE_SZ, pcData);
    #endif

    return eResult;
}

//G100_Steven_0005
eRESULT palFrontEndMgr_EDID_Upgrade_Single_Set(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;


    //LOG_MSG(db_ALWAYS, "EDID %s\r\n",pcData);

    #ifndef WORD_REG
    eResult = dvFrontEndDriverRegWrite(eFE_MSG_EDID_UPGRADE_SINGLE, eFE_MSG_EDID_UPGRADE_SZ, pcData);
    #else
    eResult = utilHost_FrontEndSet(eCMD_MODULE_FE, eFE_MSG_EDID_UPGRADE_SINGLE, eFE_MSG_EDID_UPGRADE_SZ, pcData);
    #endif

    return eResult;
}
#endif

eRESULT palFrontEndMgr_EQMode_HDMI1_Set(UINT8 ucValue)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halFrontEndCtrl_EQMode_HDMI1_Set(ucValue);

    return eResult;
}


eRESULT palFrontEndMgr_EQMode_HDMI2_Set(UINT8 ucValue)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halFrontEndCtrl_EQMode_HDMI2_Set(ucValue);

    return eResult;
}


eRESULT palFrontEndMgr_EQMode_DVI_Set(UINT8 ucValue)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halFrontEndCtrl_EQMode_DVI_Set(ucValue);

    return eResult;
}


eRESULT palFrontEndMgr_HotPlugEnable_Set(UINT8 ucEnable)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halFrontEndCtrl_HotPlugEnable_Set(ucEnable);

    return eResult;
}


eRESULT palFrontEndMgr_ForceHotPlug_Set(UINT8 ucForce)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halFrontEndCtrl_ForceHotPlug_Set(ucForce);

    return eResult;
}


eRESULT palFrontEndMgr_Model_Name_Set(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halFrontEndCtrl_Model_Name_Set(pcData);

    return eResult;
}



eRESULT palFrontEndMgr_IST_Set(UINT8 ucCount)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halFrontEndCtrl_IST_Set(ucCount);

    return eResult;
}


//G100_Julie_0002, start.
eRESULT palFrontEndMgr_IST_Get(UINT8 *ucCount)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halFrontEndCtrl_IST_Get(ucCount);

    return eResult;
}


eRESULT palFrontEndMgr_OPD_State_Get(UINT8 *ucCount)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halFrontEndCtrl_OPD_State_Get(ucCount);

    return eResult;
}


eRESULT palFrontEndMgr_OPD_Work_Get(UINT8 *ucCount)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halFrontEndCtrl_OPD_Work_Get(ucCount);

    return eResult;
}


eRESULT palFrontEndMgr_OPD_Device_Get(UINT8 ucValue, UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halFrontEndCtrl_OPD_Device_Get(ucValue, pcData);

    return eResult;
}


eRESULT palFrontEndMgr_Error_Reset(void)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halFrontEndCtrl_Error_Reset();

    return eResult;
}

//G100_Julie_0002, end.

eRESULT palFrontEndMgr_SPI_GS2961_Set(void)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halFrontEndCtrl_SPI_GS2961_Set();

    return eResult;
}


eRESULT palFrontEndMgr_SPI_GS2961_Get(UINT8 *ucCount)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halFrontEndCtrl_SPI_GS2961_Get(ucCount);

    return eResult;
}


eRESULT palFrontEndMgr_I2C_Retry_Get(UINT16 uiSize, UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halFrontEndCtrl_I2C_Retry_Get(uiSize, pcData);

    return eResult;
}


eRESULT palFrontEndMgr_I2C_Error_Get(UINT16 uiSize,UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halFrontEndCtrl_I2C_Error_Get(uiSize, pcData);

    return eResult;
}


eRESULT palFrontEndMgr_I2C_Total_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halFrontEndCtrl_I2C_Total_Get(pcData);

    return eResult;
}



eRESULT palFrontEndMgr_AutoHDMISwitch_Set(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halFrontEndCtrl_AutoHDMISwitch_Set(pcData);

    return eResult;
}


eRESULT palFrontEndMgr_CustomizedEDID_Set(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halFrontEndCtrl_CustomizedEDID_Set(pcData);

    return eResult;
}


eRESULT palFrontEndMgr_AVIInfoFrame_DVI_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

#if defined(PLATFORM_A70G2) || defined(PLATFORM_H60_2K)
    eResult = halFrontEndCtrl_AVIInfoFrame_Input0RX0_Get(pcData);
#else
    LOG_MSG(db_HAL_SOURCE_INFO, "(funcs:%s, line:%d) undefined\r\n", __FUNCTION__, __LINE__);
    eResult = rcERROR;
#endif

    return eResult;
}


eRESULT palFrontEndMgr_AVIInfoFrame_HDMI1_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

#if defined(PLATFORM_A70LK) || defined(PLATFORM_H60_4K) || defined(PLATFORM_H30_4K)
    eResult = halFrontEndCtrl_AVIInfoFrame_Input0RX1_Get(pcData);
#elif defined(PLATFORM_A70G2) || defined(PLATFORM_H60_2K)
    eResult = halFrontEndCtrl_AVIInfoFrame_Input0RX1_Get(pcData);
#else
    LOG_MSG(db_HAL_SOURCE_INFO, "(funcs:%s, line:%d) undefined\r\n", __FUNCTION__, __LINE__);
    eResult = rcERROR;
#endif

    return eResult;
}


eRESULT palFrontEndMgr_AVIInfoFrame_HDMI2_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

#if defined(PLATFORM_A70LK) || defined(PLATFORM_H60_4K) || defined(PLATFORM_H30_4K)
    eResult = halFrontEndCtrl_AVIInfoFrame_Input0RX2_Get(pcData);
#elif defined(PLATFORM_A70G2) || defined(PLATFORM_H60_2K)
    eResult = halFrontEndCtrl_AVIInfoFrame_Input0RX2_Get(pcData);
#else
    LOG_MSG(db_HAL_SOURCE_INFO, "(funcs:%s, line:%d) undefined\r\n", __FUNCTION__, __LINE__);
    eResult = rcERROR;
#endif

    return eResult;
}


eRESULT palFrontEndMgr_AVIInfoFrame_HDBASET_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

#if defined(PLATFORM_A70LK) || defined(PLATFORM_H60_4K) || defined(PLATFORM_H30_4K)
    eResult = halFrontEndCtrl_AVIInfoFrame_Input0RX3_Get(pcData);
#elif defined(PLATFORM_A70G2) || defined(PLATFORM_H60_2K)
    eResult = halFrontEndCtrl_AVIInfoFrame_Input0RX3_Get(pcData);
#else
    LOG_MSG(db_HAL_SOURCE_INFO, "(funcs:%s, line:%d) undefined\r\n", __FUNCTION__, __LINE__);
    eResult = rcERROR;
#endif

    return eResult;
}


eRESULT palFrontEndMgr_AVIInfoFrame_DP_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

#if defined(PLATFORM_A70LK) || defined(PLATFORM_H60_4K) || defined(PLATFORM_H30_4K) || defined(PLATFORM_H60_2K) //HICC2_Doulas_0150
    eResult = halFrontEndCtrl_AVIInfoFrame_Input0RX0_Get(pcData);
#else
    LOG_MSG(db_HAL_SOURCE_INFO, "(funcs:%s, line:%d) undefined\r\n", __FUNCTION__, __LINE__);
    eResult = rcERROR;
#endif

    return eResult;
}

eRESULT palFrontEndMgr_Model_Switch_Enable_Set(UINT8 ucSwitch)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halFrontEndCtrl_Model_Switch_Enable_Set(ucSwitch);

    return eResult;
}

