#ifndef _PALFRONTENDMGR_H_
#define _PALFRONTENDMGR_H_

#include "Common.h"
#include "utilCommon.h"

#define HDMI_OUT_DISABLE    (9)

void palFrontEndCtrl_FocusResync(void);
eRESULT palFrontEndMgr_Version_Get(UINT8* pcData);
eRESULT palFrontEndMgr_HDBaseTVersion_Get(UINT8* pcData);
eRESULT palFrontEndMgr_videoReady_Get(UINT8* pcData);
eRESULT palFrontEndMgr_BackupInput_videoReady_Get(UINT8* pcData);
eRESULT palFrontEndMgr_videoFormat_Get(UINT8* pcData);
eRESULT palFrontEndMgr_OPDInfo_Get(UINT8* pcData);
eRESULT palFrontEndMgr_MainInput_Get(UINT8* pcData);
eRESULT palFrontEndMgr_MainInput_Set(UINT8* pcData, BOOL bCheck, UINT8 cResetSource);
eRESULT palFrontEndMgr_SubInput_Get(UINT8* pcData);
eRESULT palFrontEndMgr_SubInput_Set(UINT8* pcData, BOOL bCheck, UINT8 cResetSource);
eRESULT palFrontEndMgr_Backup_AutoSW_Set(UINT8* pcData);
eRESULT palFrontEndMgr_Backup_First_Set(UINT8* pcData);
eRESULT palFrontEndMgr_HDMI_OUT_Set(UINT8* pcData);
eRESULT palFrontEndMgr_Backup_Second_Set(UINT8* pcData);
eRESULT palFrontEndMgr_Backup_Status_Get(UINT8* pcData);
eRESULT palFrontEndMgr_BackupInput_ScalerTimingCheck_Set(UINT8* pcData);
eRESULT palFrontEndMgr_PIP_Enable_Get(UINT8* pcData);
eRESULT palFrontEndMgr_PIP_Enable_Set(UINT8* pcData);
eRESULT palFrontEndMgr_INPUT_SOURCE_DETECT_Get(UINT16* uiData);
eRESULT palFrontEndMgr_VGA_H_Total_Get(UINT16* uiData);
eRESULT palFrontEndMgr_VGA_H_Total_Set(UINT16* uiData);
eRESULT palFrontEndMgr_VGA_Phase_Get(UINT8* pcData);
eRESULT palFrontEndMgr_VGA_Phase_Set(UINT8* pcData);
eRESULT palFrontEndMgr_videoFormatSub_Get(UINT8* pcData);
eRESULT palFrontEndMgr_VGA_Sync_Threshold_Get(UINT8* pcData);
eRESULT palFrontEndMgr_VGA_Sync_Threshold_Set(UINT8* pcData);
eRESULT palFrontEndMgr_VGA_Pixel_Clock_Set(UINT32 udData);
eRESULT palFrontEndMgr_VGA_Gain_Get(UINT16* uiRed,UINT16* uiGreen,UINT16* uiBlue);
eRESULT palFrontEndMgr_VGA_Gain_Set(UINT16 uiRed,UINT16 uiGreen,UINT16 uiBlue);
eRESULT palFrontEndMgr_VGA_Offset_Get(UINT16* uiRed,UINT16* uiGreen,UINT16* uiBlue);
eRESULT palFrontEndMgr_VGA_Offset_Set(UINT16 uiRed,UINT16 uiGreen,UINT16 uiBlue);
eRESULT palFrontEndMgr_FPGA_Version_Get(UINT8* pcData);
eRESULT palFrontEndMgr_Main_Timing_Get(UINT8* pcData);
eRESULT palFrontEndMgr_Sub_Timing_Get(UINT8* pcData);
eRESULT palFrontEndMgr_EDID_SN_Set(UINT8 cDevice, UINT8 cType, eCM_NATIVE_TIMING eNativeTiming, UINT8* pcData);
eRESULT palFrontEndMgr_VGA_TIMING_Set(UINT16 uiH_Active,UINT16 uiV_Active,UINT16 uiV_Freq);
eRESULT palFrontEndMgr_EQMode_HDMI1_Set(UINT8 ucValue);
eRESULT palFrontEndMgr_EQMode_HDMI2_Set(UINT8 ucValue);
eRESULT palFrontEndMgr_EQMode_DVI_Set(UINT8 ucValue);
eRESULT palFrontEndMgr_HotPlugEnable_Set(UINT8 ucEnable);
eRESULT palFrontEndMgr_ForceHotPlug_Set(UINT8 ucForce);
eRESULT palFrontEndMgr_Model_Name_Set(UINT8* pcData);
eRESULT palFrontEndMgr_IST_Set(UINT8 ucCount);
eRESULT palFrontEndMgr_IST_Get(UINT8 *ucCount);
eRESULT palFrontEndMgr_OPD_State_Get(UINT8 *ucCount);
eRESULT palFrontEndMgr_OPD_Work_Get(UINT8 *ucCount);
eRESULT palFrontEndMgr_OPD_Device_Get(UINT8 ucValue, UINT8* pcData);
eRESULT palFrontEndMgr_Error_Reset(void);
eRESULT palFrontEndMgr_SPI_GS2961_Set(void);
eRESULT palFrontEndMgr_SPI_GS2961_Get(UINT8 *ucCount);
eRESULT palFrontEndMgr_I2C_Retry_Get(UINT16 uiSize, UINT8* pcData);
eRESULT palFrontEndMgr_I2C_Error_Get(UINT16 uiSize,UINT8* pcData);
eRESULT palFrontEndMgr_I2C_Total_Get(UINT8* pcData);
eRESULT palFrontEndMgr_AutoHDMISwitch_Set(UINT8* pcData);
eRESULT palFrontEndMgr_CustomizedEDID_Set(UINT8* pcData);
eRESULT palFrontEndMgr_Model_Switch_Enable_Set(UINT8 ucSwitch);
eRESULT palFrontEndMgr_AVIInfoFrame_DVI_Get(UINT8* pcData);
eRESULT palFrontEndMgr_AVIInfoFrame_HDMI1_Get(UINT8* pcData);
eRESULT palFrontEndMgr_AVIInfoFrame_HDMI2_Get(UINT8* pcData);
eRESULT palFrontEndMgr_AVIInfoFrame_HDBASET_Get(UINT8* pcData);
eRESULT palFrontEndMgr_AVIInfoFrame_DP_Get(UINT8* pcData);
eRESULT palFrontEndMgr_Model_Switch_Enable_Set(UINT8 ucSwitch);


#endif /*_PALFRONTENDMGR_H_*/

