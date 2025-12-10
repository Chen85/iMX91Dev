#ifndef _HALSCALER_H_
#define _HALSCALER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "Common.h"
#include "utilStorageCfg.h"
#include "utilCommon.h"


//======================= Error Code Start ======================//
typedef enum
{
    eHAL_SCALER_EXEC_CODE_PASS,         /* pass */
    eHAL_SCALER_EXEC_CODE_FAIL,    /* general fail indication */
    eHAL_SCALER_EXEC_CODE_INIT_FAIL,
    eHAL_SCALER_EXEC_CODE_NOT_INIT,
    eHAL_SCALER_EXEC_CH_OVER_RANGE,

    eHAL_SCALER_EXEC_CODE_DV_SCALER_INIT_FAIL,
    eHAL_SCALER_EXEC_CODE_DV_RX_INIT_FAIL,
    eHAL_SCALER_EXEC_CODE_DV_RX_UNSTABLE,
    eHAL_SCALER_EXEC_CODE_NO_AUTO_PHASE,        //A70LV_Doulas_0007
    eHAL_SCALER_EXEC_CODE_AUTO_PHASE_GOING,     //A70LV_Doulas_0007    
    eHAL_SCALER_EXEC_CODE_HDR_CHANGE,

}eHAL_SCALER_EXEC_CODE;
//======================= Error Code End =======================//
//G100_Steven_0016 start
typedef enum
{
    eHAL_COLOR_SETTING_CS,
    eHAL_COLOR_SETTING_CT,
	eHAL_COLOR_SETTING_GAMMA,
    eHAL_COLOR_SETTING_BRILLIENTCOLOR,
    eHAL_COLOR_SETTING_WHITEPEAKING,
    eHAL_COLOR_SETTING_COLORENHANCEMENT,
    eHAL_COLOR_SETTING_CWSPEED,
    eHAL_COLOR_SETTING_SKINCOLOR,
    eHAL_COLOR_SETTING_SHARPNESS,
    eHAL_COLOR_SETTING_BRIGHTNESS,
    eHAL_COLOR_SETTING_CONTRAST,
    eHAL_COLOR_SETTING_TINT,
    eHAL_COLOR_SETTING_SATURATION,
    eHAL_COLOR_SETTING_REDGAIN,
    eHAL_COLOR_SETTING_GREENGAIN,
    eHAL_COLOR_SETTING_BLUEGAIN,
    eHAL_COLOR_SETTING_REDOFFSET,
    eHAL_COLOR_SETTING_GREENOFFSET,
    eHAL_COLOR_SETTING_BLUEOFFSET

}eHAL_COLOR_SETTING;
//G100_Steven_0016 end

typedef enum
{
    eCLOCKGEN_SEL_C734,
    eCLOCKGEN_SEL_C790,
    eCLOCKGEN_SEL_C821_HD,
    eCLOCKGEN_SEL_C821_WU,
    eCLOCKGEN_SEL_C821_720P,
    eCLOCKGEN_SEL_C821_720P120,
    eCLOCKGEN_SEL_C821_800x600_120,
    eCLOCKGEN_SEL_C821_1080P120,
    eCLOCKGEN_SEL_C821_WUXGA120,
    eCLOCKGEN_SEL_LAST,
}eCLOCKGEN_SEL;


#define HAL_SCALER_PASS 1
#define HAL_SCALER_DATA_OUT_OF_RANGE -2


//=============== Struct Start =================//


//=============== Struct End ==================//
UINT32 halScaler_GetScalerErrCode(const UINT8 ucCH);

eHAL_SCALER_EXEC_CODE halScaler_Init(UINT8 ucCH);//A70LV_Doulas_0003//Init 4 channels in once, no matter 4K, 2K

eHAL_SCALER_EXEC_CODE halScaler_PowerStandby(void);
eHAL_SCALER_EXEC_CODE halScaler_PowerNormal(UINT8 ucCH, const ePANEL_ID ePanelId);
eHAL_SCALER_EXEC_CODE halScaler_ConfigureForDisplay(UINT8 ucCH, const UINT8 ucColorFmt);
eHAL_SCALER_EXEC_CODE halScaler_ConfigureForDisplay_ChangePIPLayout(UINT8 ucCH, const UINT8 ucColorFmt);   //A35G2_CDS_Simon_0001
eHAL_SCALER_EXEC_CODE halScaler_DisplaySFG(UINT8 ucCH, const UINT8 ucFillColor, const BOOL bEnable);

eHAL_SCALER_EXEC_CODE halScaler_DisplaySplash(void);
eHAL_SCALER_EXEC_CODE halScaler_PortConfig(UINT8 ucCH);

eHAL_SCALER_EXEC_CODE halScaler_SetOverlay(UINT8 ucCH, UINT8 ucEnable); //ZU860_Doulas_0017
eHAL_SCALER_EXEC_CODE halScaler_SetOverlayCoef(UINT8 ucCH, const PUINT8 pucOverLay);

eHAL_SCALER_EXEC_CODE halScaler_Brightness_Set(UINT8 ucCH,UINT8 ucSetting);  //A70LV_Doulas_0022 modify
eHAL_SCALER_EXEC_CODE halScaler_Contrast_Set(UINT8 ucCH,UINT8 ucSetting);    //A70LV_Doulas_0022 modify
INT8 halScaler_Dump_Register(void);     //A70LV_Doulas_0002
eHAL_SCALER_EXEC_CODE halScaler_Bypasse_Mode(void);     //A70LV_Doulas_0002
eHAL_SCALER_EXEC_CODE halScaler_Test_Mode(void);     //A70LV_Doulas_0002

eHAL_SCALER_EXEC_CODE halScaler_HUE_Set(UINT8 ucCH,UINT8 ucSetting);  //A70LV_Doulas_0022
eHAL_SCALER_EXEC_CODE halScaler_Saturation_Set(UINT8 ucCH,UINT8 ucSetting);  //A70LV_Doulas_0022
eHAL_SCALER_EXEC_CODE halScaler_PIP_PBP_Layout_Set(UINT8 ucLayout);    //A70LV_Doulas_0024
eHAL_SCALER_EXEC_CODE halScaler_PIP_PBP_Size_Set(UINT8 ucSize);    //A70LV_Doulas_0024
UINT8 halScaler_PIP_PBP_Layout_Get(void);    //A70LV_Doulas_0003
UINT8 halScaler_PIP_PBP_Size_Get(void);    //A70LV_Doulas_0003
void halScaler_Resync_Init(UINT8 eCH);   //A70LV_Doulas_0004
eHAL_SCALER_EXEC_CODE halScaler_SYNC_Lock_Get(UINT8 ucCH);   //A70LV_Doulas_0009 modify
eHAL_SCALER_EXEC_CODE halScaler_CheckChannelSyncCount(UINT8 *ucIsSimilar);
eHAL_SCALER_EXEC_CODE halScaler_Freeze_Set(UINT8 ucCH,BOOL bFreezeEn);  //A70LV_Doulas_0004
eHAL_SCALER_EXEC_CODE halScaler_Freeze_Get(UINT8 ucCH,BOOL *bFreezeEn);  //A70LV_Doulas_0004
eHAL_SCALER_EXEC_CODE halScaler_Panel_Set(UINT8 ucCH,ePANEL_ID ePanelId);  //A70LV_Doulas_0005

#ifdef INIT_VCXO949
UINT32 halScaler_Panel_Colck_Set(ePANEL_ID ePanelId);    //A70LV_Doulas_0005
#endif

eHAL_SCALER_EXEC_CODE halScaler_AutoPhaseStart(UINT8 ucCH);  //A70LV_Doulas_0007
eHAL_SCALER_EXEC_CODE halScaler_AutoPhase(UINT8 ucCH);  //A70LV_Doulas_0007
eHAL_SCALER_EXEC_CODE halScaler_SetInputSource(UINT8 ucCH,UINT8 ucInputSource);  //A70LV_Doulas_0007

eHAL_SCALER_EXEC_CODE halScaler_RedOffset_Set(UINT8 ucCH,UINT8 ucSetting);  //A70LV_Doulas_0022 modify       //A70LV_Doulas_0011
eHAL_SCALER_EXEC_CODE halScaler_GreenOffset_Set(UINT8 ucCH,UINT8 ucSetting);  //A70LV_Doulas_0022 modify     //A70LV_Doulas_0011
eHAL_SCALER_EXEC_CODE halScaler_BlueOffset_Set(UINT8 ucCH,UINT8 ucSetting);  //A70LV_Doulas_0022 modify      //A70LV_Doulas_0011
eHAL_SCALER_EXEC_CODE halScaler_RedGain_Set(UINT8 ucCH,UINT8 ucSetting);  //A70LV_Doulas_0022 modify          //A70LV_Doulas_0011
eHAL_SCALER_EXEC_CODE halScaler_GreenGain_Set(UINT8 ucCH,UINT8 ucSetting);  //A70LV_Doulas_0022 modify       //A70LV_Doulas_0011
eHAL_SCALER_EXEC_CODE halScaler_BlueGain_Set(UINT8 ucCH,UINT8 ucSetting);  //A70LV_Doulas_0022 modify         //A70LV_Doulas_0011

eHAL_SCALER_EXEC_CODE halScaler_RedOffset_Value_Set(UINT8 ucCH,UINT8 ucSetting);  //A70LV_Doulas_0022 modify     //A70LV_Doulas_0012
eHAL_SCALER_EXEC_CODE halScaler_GreenOffset_Value_Set(UINT8 ucCH,UINT8 ucSetting);  //A70LV_Doulas_0022 modify   //A70LV_Doulas_0012
eHAL_SCALER_EXEC_CODE halScaler_BlueOffset_Value_Set(UINT8 ucCH,UINT8 ucSetting);  //A70LV_Doulas_0022 modify    //A70LV_Doulas_0012
eHAL_SCALER_EXEC_CODE halScaler_RedGain_Value_Set(UINT8 ucCH,UINT8 ucSetting);  //A70LV_Doulas_0022 modify       //A70LV_Doulas_0012
eHAL_SCALER_EXEC_CODE halScaler_GreenGain_Value_Set(UINT8 ucCH,UINT8 ucSetting);  //A70LV_Doulas_0022 modify     //A70LV_Doulas_0012
eHAL_SCALER_EXEC_CODE halScaler_BlueGain_Value_Set(UINT8 ucCH,UINT8 ucSetting);  //A70LV_Doulas_0022 modify       //A70LV_Doulas_0012


eHAL_SCALER_EXEC_CODE halScaler_Brightness_Value_Set(UINT8 ucCH,UINT8 ucSetting);  //A70LV_Doulas_0022 modify  //A70LV_Doulas_0012
eHAL_SCALER_EXEC_CODE halScaler_Contrast_Value_Set(UINT8 ucCH,UINT8 ucSetting);     //A70LV_Doulas_0022
eHAL_SCALER_EXEC_CODE halScaler_HUE_Value_Set(UINT8 ucCH,UINT8 ucSetting);  //A70LV_Doulas_0022
eHAL_SCALER_EXEC_CODE halScaler_Saturation_Value_Set(UINT8 ucCH,UINT8 ucSetting);  //A70LV_Doulas_0022

eHAL_SCALER_EXEC_CODE halScaler_Init_EEPROM_Setting(sUSER_SYSTEM_SETTING m_UserSetting);  //A70LV_Doulas_0016
SemaphoreHandle_t halScaler_xSemaphore_Get(void);  //A70LV_Doulas_0014

eHAL_SCALER_EXEC_CODE halScaler_SizePresets_Set(UINT8 ucSetting);    //A70LV_Doulas_0020
eHAL_SCALER_EXEC_CODE halScaler_SizePresets_SetOnlyValue(UINT8 ucSetting);

eHAL_SCALER_EXEC_CODE halScaler_Overscan_Set(UINT8 ucSetting);
eHAL_SCALER_EXEC_CODE halScaler_PixelTrack_Set(UINT8 ucSetting);
eHAL_SCALER_EXEC_CODE halScaler_PixelPhase_Set(UINT8 ucSetting);
eHAL_SCALER_EXEC_CODE halScaler_PixelPhase_Action(void);        //A70LV_Doulas_0117
UINT8 halScaler_OSD_PixelPhase_Get(void);     //A70LV_Doulas_0116
eHAL_SCALER_EXEC_CODE halScaler_HorzPosition_Set(UINT8 ucSetting);
eHAL_SCALER_EXEC_CODE halScaler_VertPosition_Set(UINT8 ucSetting);
eHAL_SCALER_EXEC_CODE halScaler_DigitalHorzZoom_Set(UINT16 uiSetting);
eHAL_SCALER_EXEC_CODE halScaler_DigitalVertZoom_Set(UINT16 uiSetting);
eHAL_SCALER_EXEC_CODE halScaler_DigitalHorzShift_Set(UINT8 ucSetting);
eHAL_SCALER_EXEC_CODE halScaler_DigitalVertShift_Set(UINT8 ucSetting);
eHAL_SCALER_EXEC_CODE halScaler_AutoImage_Set(UINT8 ucSetting);     //A70LV_Doulas_0112 modify
UINT8 halScaler_InputSource_Get(UINT8 ucCH);
UINT8 halScaler_SizePresets_Get(UINT8 ucCH);

UINT8 halScaler_PictureSettings_Get(UINT8 ucCH);     //A70LV_Doulas_0022
UINT8 halScaler_SignalType_Get(UINT8 ucCH);
eHAL_SCALER_EXEC_CODE halScaler_ColorSpace_Set(UINT8 ucCH,UINT8 ucSetting);
UINT8 halScaler_ColorSpace_Get(UINT8 ucCH);   //A70LV_Doulas_0069
UINT8 halScaler_ColorSpace_ScalerValueGet(UINT8 ucCH);   //A70LV_Doulas_0069
eHAL_SCALER_EXEC_CODE halScaler_Detail_Set(UINT8 ucCH,UINT8 ucSetting);
INT8 halScaler_3DEnable_Set(UINT8 ucSetting);
INT8 halScaler_3DInvert_Set(UINT8 ucSetting);
INT8 halScaler_3DSyncOut_Set(UINT8 ucSetting);
eHAL_SCALER_EXEC_CODE halScaler_FrameDelay_Set(UINT16 uiSetting);
eHAL_SCALER_EXEC_CODE halScaler_SkinColor_Set(UINT8 ucCH,UINT8 ucSetting);
eHAL_SCALER_EXEC_CODE halScaler_TemporalNoiseReduction_Set(UINT8 ucSetting);
eHAL_SCALER_EXEC_CODE halScaler_MPEGNoiseReduction_Set(UINT8 ucSetting);
eHAL_SCALER_EXEC_CODE halScaler_DetectFilm_Set(UINT8 ucSetting);
eHAL_SCALER_EXEC_CODE halScaler_RGB_Gain_Offset_Set(void);
eHAL_SCALER_EXEC_CODE halScaler_PictureSettings_Set(UINT8 ucCH,UINT8 ucSetting);
eHAL_SCALER_EXEC_CODE halScaler_ContrastEnhancement_Set(UINT8 ucSetting);
INT8 halScaler_HDR_Set(UINT8 ucSetting);
INT8 halScaler_MEMC_Set(UINT8 ucSetting);
eHAL_SCALER_EXEC_CODE halScaler_ImageFreeze_Set(UINT8 ucSetting);
eHAL_SCALER_EXEC_CODE halScaler_Gamma_Set(UINT8 ucCH,UINT8 ucSetting);
eHAL_SCALER_EXEC_CODE halScaler_WhitePeaking_Set(UINT8 ucCH,UINT8 ucSetting);
eHAL_SCALER_EXEC_CODE halScaler_ColorTemperature_Set(UINT8 ucCH,UINT8 ucSetting);
INT8 halScaler_EdgeEnhancement_Set(UINT8 ucSetting);
INT8 halScaler_ColorWheelSpeed_Get(UINT8 ucCH); // A70LV_Eric.C_0026
eHAL_SCALER_EXEC_CODE halScaler_ColorWheelSpeed_Set(UINT8 ucCH,UINT8 ucSetting);
eHAL_SCALER_EXEC_CODE halScaler_ColorEnhancement_Set(UINT8 ucCH,UINT8 ucSetting);
eHAL_SCALER_EXEC_CODE halScaler_ColorEnhancement_Get(UINT8 ucCH,UINT8 *ucSetting);     //A70LV_Doulas_0044

eHAL_SCALER_EXEC_CODE halScaler_HSGEnable_Set(UINT8 ucSetting);    //A70LV_Doulas_0023
eHAL_SCALER_EXEC_CODE halScaler_HSGEnable_Get(UINT8 *ucSetting);    //A70LV_Doulas_0044
eHAL_SCALER_EXEC_CODE halScaler_HSG_Set(UINT8 ucCH,sHSG_SETTING ucSetting);
eHAL_SCALER_EXEC_CODE halScaler_HSG_All_Set(UINT8 ucInputSource,UINT8  ucPresetMode,sHSG_SETTING ucSetting);       //A70LV_Doulas_0038
eHAL_SCALER_EXEC_CODE halScaler_HSG_Get(sHSG_SETTING *m_sHSG_Setting);       //A70LV_Doulas_0038

eHAL_SCALER_EXEC_CODE halScaler_PIP_PBP_Enable_Set(UINT8 ucSetting);      //A70LV_Doulas_0024
UINT8 halScaler_PIP_PBP_Enable_Get(void);               //A70LV_Doulas_0056
eHAL_SCALER_EXEC_CODE halScaler_IintChannelSetting(UINT8 ucCH);      //A70LV_Doulas_0030 modify  //A70LV_Doulas_0029
UINT16 halScaler_DigitalHorzZoom_Get(void);
UINT16 halScaler_DigitalVertZoom_Get(void);
UINT8 halScaler_DigitalHorzShift_Get(void);
UINT8 halScaler_DigitalVertShift_Get(void);
eHAL_SCALER_EXEC_CODE halScaler_Detail_Value_Set(UINT8 ucCH,UINT8 ucSetting);
UINT8 halScaler_Detail_Value_Get(UINT8 ucCH);
eHAL_SCALER_EXEC_CODE halScaler_SkinColor_Value_Set(UINT8 ucCH,UINT8 ucSetting);
UINT8 halScaler_SkinColor_Value_Get(UINT8 ucCH);
INT16 halScaler_Brightness_Value_Get(UINT8 ucCH);
DOUBLE halScaler_Contrast_Value_Get(UINT8 ucCH);
INT16 halScaler_HUE_Value_Get(UINT8 ucCH);
DOUBLE halScaler_Saturation_Value_Get(UINT8 ucCH);
INT16 halScaler_RedOffset_Value_Get(UINT8 ucCH);
INT16 halScaler_GreenOffset_Value_Get(UINT8 ucCH);
INT16 halScaler_BlueOffset_Value_Get(UINT8 ucCH);
DOUBLE halScaler_RedGain_Value_Get(UINT8 ucCH);
DOUBLE halScaler_GreenGain_Value_Get(UINT8 ucCH);
DOUBLE halScaler_BlueGain_Value_Get(UINT8 ucCH);
eHAL_SCALER_EXEC_CODE halScaler_Temporal_NR_Value_Set(UINT8 ucSetting);
eHAL_SCALER_EXEC_CODE halScaler_MPEG_NR_Value_Set(UINT8 ucSetting);
eHAL_SCALER_EXEC_CODE halScaler_DetectFilm_Value_Set(UINT8 ucSetting);

eHAL_SCALER_EXEC_CODE halScaler_PictureSettings_Value_Set(UINT8 ucCH,UINT8 ucSetting);     //A70LV_Doulas_0030

eHAL_SCALER_EXEC_CODE halScaler_MeasureInput(UINT8 ucCH);      //A70LV_Doulas_0031
eHAL_SCALER_EXEC_CODE halScaler_OSD_TestPattern(const UINT8 ucTestPattern);      //A70LV_Doulas_0035
eHAL_SCALER_EXEC_CODE halScaler_OSD_TestPattern_Set(UINT8 ucCH,UINT8 ucSetting);
eHAL_SCALER_EXEC_CODE halScaler_Service_TestPattern(const UINT8 ucTestPattern);
eHAL_SCALER_EXEC_CODE halScaler_Service_TestPattern_Set(UINT8 ucCH,UINT8 ucSetting);
UINT8 halScaler_PictureSettings_Get_NotReturnUser(UINT8 ucCH);     //A70LV_Doulas_0044

eHAL_SCALER_EXEC_CODE halScaler_InputPixelClock_Get(UINT8 ucCH,UINT8 *ucValue);     //A70LV_Doulas_0056
eHAL_SCALER_EXEC_CODE halScaler_InputResolution_Get(UINT8 ucCH,UINT8 *ucValue);
eHAL_SCALER_EXEC_CODE halScaler_InputHorzRefresh_Get(UINT8 ucCH,UINT8 *ucValue);
eHAL_SCALER_EXEC_CODE halScaler_InputVertRefresh_Get(UINT8 ucCH,UINT8 *ucValue);
eHAL_SCALER_EXEC_CODE halScaler_InputVertRefresh2_Get(UINT8 ucCH,UINT16 *uiValue);     //A70LV_Doulas_0200
eHAL_SCALER_EXEC_CODE halScaler_InputVertRefresh2_Get_FromSyncCount(UINT8 ucCH,UINT16 *uiValue);   //A35G2_CDS_Simon_0052
eHAL_SCALER_EXEC_CODE halScaler_InputSignalFormat_Get(UINT8 ucCH,UINT8 *ucValue);
eHAL_SCALER_EXEC_CODE halScaler_InputAspectRatio_Get(UINT8 ucCH,UINT8 *ucValue);
eHAL_SCALER_EXEC_CODE halScaler_FrontEndColorSpace_Set(UINT8 ucCH,UINT8 ucValue);     //A70LV_Doulas_0076 Add
eHAL_SCALER_EXEC_CODE halScaler_FrontEndColorSpace_Get(UINT8 ucCH,UINT8* ucValue);     //ZU860_Doulas_0108 Add
eHAL_SCALER_EXEC_CODE halScaler_FrontEndColorSpaceInfo_Get(UINT8 ucCH,UINT8* ucValue);    //A70LV_Doulas_0109 Add
eHAL_SCALER_EXEC_CODE halScaler_FrontEndScanMode_Set(UINT8 ucCH,UINT8 ucValue);
eHAL_SCALER_EXEC_CODE halScaler_FrontEndScanMode_Clear(UINT8 ucCH);      //A70LV_Doulas_0175
BOOL halScaler_ForcedSyncResetType_Get(void);    //A70LV_Doulas_0079
eHAL_SCALER_EXEC_CODE halScaler_ForcedSyncResetDisable_Set(void);
BOOL halScaler_IsInterlaced_Get(UINT8 ucCH);    //A70LV_Doulas_0092
eHAL_SCALER_EXEC_CODE halScaler_FrontEndVideoYUV_Set(UINT8 ucCH,UINT8 ucValue);     //A70LV_Doulas_0109 Add
eHAL_SCALER_EXEC_CODE halScaler_VGA_H_Total_Get(UINT8 ucCH,UINT16* uiVal);     //A70LV_Doulas_0112
eHAL_SCALER_EXEC_CODE halScaler_VGA_V_Total_Get(UINT8 ucCH,UINT16* uiVal);     //A70LV_Doulas_0195
eHAL_SCALER_EXEC_CODE halScaler_VGA_SYNC_TYPE_Set(UINT8 ucCH,UINT8 ucVal);     //A70LV_Doulas_0112
UINT8 halScaler_VGA_SYNC_TYPE_Get(UINT8 ucCH);                           //A70LV_Doulas_0124
eHAL_SCALER_EXEC_CODE halScaler_SyncThreshold_Set(UINT8 ucSetting);     //A70LV_Doulas_0114
UINT8 halScaler_SyncThreshold_Get(void);                                //A70LV_Doulas_0114
UINT32 halScaler_uiHFreq_Get(UINT8 ucCH);                       //A70LV_Doulas_0115
eHAL_SCALER_EXEC_CODE halScaler_Position_Run(UINT8 ucCH);       //A70LV_Doulas_0113
UINT8 halScaler_HorzPositionWorkValueGet(UINT8 ucCH);
UINT8 halScaler_VertPositionWorkValueGet(UINT8 ucCH);
eHAL_SCALER_EXEC_CODE halScaler_AutoPsitionStart(UINT8 ucCH,BOOL bAutoEnable);    //A70LV_Doulas_0118
eHAL_SCALER_EXEC_CODE halScaler_Config_NoSignalOutput(UINT8 ucCH, UINT8 ucDisplayOutput);      //A70LV_Doulas_0142  //A70LV_Doulas_0120  //A35G2_CDS_Simon_0017
UINT16 halScaler_Input_H_Active_Get(UINT8 ucCH);     //A70LV_Doulas_0124
UINT16 halScaler_Input_V_Active_Get(UINT8 ucCH);
eHAL_SCALER_EXEC_CODE halScaler_DDR_InputDataAreaTotalGet(UINT8 eCH,RECT DataArea,UINT32 *ulTotalRed,UINT32 *ulTotalGreen,UINT32 *ulTotalBlue);
eHAL_SCALER_EXEC_CODE halScaler_InputADC_Cali_CalibrationEnableSet(UINT8 eCH,UINT8 ucADC_CAl_Enable);

UINT8 halScaler_Input_3D_Format_Get(void);           //A70LV_Doulas_0154
void halScaler_Input_3D_Format_Set(UINT8 ucVal);     //A70LV_Doulas_0154
eHAL_SCALER_EXEC_CODE halScaler_3D_InputPort_Set(void);              //A70LV_Doulas_0154
//UINT16 halScaler_Output_V_Total_Get(void);     //A70LV_Doulas_0154
UINT8 halScaler_Input_3D_Format_Config_Get(void);           //A70LV_Doulas_0159
eHAL_SCALER_EXEC_CODE halScaler_InputHorzPeriod_Get(UINT8 ucCH,UINT32* udVal);     //A70LV_Doulas_0195
eHAL_SCALER_EXEC_CODE halScaler_VGA_ModeTable_Get(UINT8 ucCH,UINT8* ucVal);
eHAL_SCALER_EXEC_CODE halScaler_VGA_ModeTableNumber_Get(UINT8 ucCH,UINT16* uiVal);
eHAL_SCALER_EXEC_CODE halScaler_ModeAdjusmenttHorzStart_Get(UINT16* uiVal);
eHAL_SCALER_EXEC_CODE halScaler_ModeAdjusmenttVertStart_Get(UINT16* uiVal);
eHAL_SCALER_EXEC_CODE halScaler_ModeAdjusmenttHorzStart_Set(UINT16 uiVal);
eHAL_SCALER_EXEC_CODE halScaler_ModeAdjusmenttVertStart_Set(UINT16 uiVal);
eHAL_SCALER_EXEC_CODE halScaler_ModeAdjusmenttEnableSetting(void);
eHAL_SCALER_EXEC_CODE halScaler_ModeAdjusmenttDisableSetting(void);
eHAL_SCALER_EXEC_CODE halScaler_Init_Mode_Adjustment_EEPROM_Setting(sTIMING_TABLE sTiming_Table);
eHAL_SCALER_EXEC_CODE halScaler_Input3D_Timing_Get(UINT8 ucCH,UINT8* ucVal);     //A70LV_Doulas_0196
eHAL_SCALER_EXEC_CODE halScaler_ConfigureForPosition(UINT8 ucCH);     //A70LV_Doulas_0238
eHAL_SCALER_EXEC_CODE halScaler_ConfigureForScaler(UINT8 ucCH);     //A70LV_Doulas_0284
eHAL_SCALER_EXEC_CODE halScaler_EdgeMask_Set(UINT8 ucSetting);       //ZU860_Doulas_0004
INT8 halScaler_3DMode_Set(UINT8 ucSetting);        //ZU860_Doulas_0049
UINT8 halScaler_IS_3D_Enable(void);         //ZU860_Doulas_0054
#ifdef Low_Latency_All
UINT8 halScaler_3D_Type(void);
#endif	/*Low_Latency_All*/
BOOL halScaler_IsVsync120or100Hz_Get(UINT8 ucCH);   //ZU860_Doulas_0104
BOOL halScaler_IsAspectRation16_9_Get(void);   //ZU860_Doulas_0119
BOOL halScaler_IsAspectRation16_10_Get(void);     //ZU860_Doulas_0120
eHAL_SCALER_EXEC_CODE halScaler_SourceOutputOff(UINT8 ucCH);     //ZU860_Doulas_0124
UINT8 halScaler_OutputVsyncFreqGetForTwistOn(void);     //A70LV_Doulas_0329
#ifdef Low_Latency_All
INT8 halScaler_Low_Latency_Set(UINT8 ucSetting);	//ZU860_Clare_0152
BOOL halScaler_Low_Latency_Get(void);	//ZU860_Clare_0152
UINT8 halScaler_Aspect_Ratio_Is_3D_Mode_Get(void);	//ZU860_Clare_0152
#endif	/*Low_Latency_All*/
#ifdef __cplusplus
}
#endif
UINT8 halScaler_PictureSettings_PreUser_Get(UINT8 ucCH);
UINT8 halScaler_ColorSetting_Get(eHAL_COLOR_SETTING  ucNode, UINT8 ucCH);
UINT32 halScaler_Reg0_Read_Get(void);
UINT8 halScaler_OPD_Test(UINT8 ucData);
void halScaler_PreUserMode_Set(UINT8 ucSetting);  //G100_Steven_0018

eHAL_SCALER_EXEC_CODE halScaler_Horz_Start_Position_Get(UINT8 ucCH,UINT16* uiVal); //G100_Julie_0017
eHAL_SCALER_EXEC_CODE halScaler_Vert_Start_Position_Get(UINT8 ucCH,UINT16* uiVal); //G100_Julie_0017
eHAL_SCALER_EXEC_CODE halScaler_CaptureImage(void);                 //G100_Owen_0048
eHAL_SCALER_EXEC_CODE halScaler_CaptureImage_Show(BOOL bEnable);    //G100_Owen_0048
eHAL_SCALER_EXEC_CODE halScaler_CaptureImage_Del(BOOL bSourceLock); //G100_Owen_0058
eHAL_SCALER_EXEC_CODE halScaler_ScreenSaveToImage(void);
eHAL_SCALER_EXEC_CODE halScaler_C821_Auto_Fill_Screen_Init(void);    		  //G100_Steven_0051
eHAL_SCALER_EXEC_CODE halScaler_C821_Auto_Fill_Screen_Setting(BOOL bSetting); //G100_Steven_0051
void halScaler_Test(UINT16 *puiTtlCnt, UINT16 *puiErrCnt);  //G100_Owen_0080
void halScaler_PIPPBPPaniel_InfoSet(BYTE cCH, UINT16 uiHActive, UINT16 uiVActive, UINT16 uiHStart, UINT16 uiVStart); //A35G2_CDS_Larry_0008
UINT8 halScaler_PowerNormalReadyGet(BYTE ucCH);
void halScaler_3D_Reset(void);  //G100_Steven_0109 //A35G2_BRC_Casper_0048
eHAL_SCALER_EXEC_CODE halScaler_ClockGen_Init(eCLOCKGEN_SEL eClockgenSel);
void halScaler_PowerNormalDone(BOOL bDone);

UINT32 halScaler_ICP_Register_Read(UINT32 ulAddr);				 //A65_OPTOMA_Julie_0050
void halScaler_ICP_Register_Write(UINT32 ulAddr, UINT32 ucData); //A65_OPTOMA_Julie_0050

void halScaler_AutoSourceResyncSet(UINT8 ucAutoSourceResync);
UINT8 halScaler_AutoSourceResyncGet(void);

#endif

