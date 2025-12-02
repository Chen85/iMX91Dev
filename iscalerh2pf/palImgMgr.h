#ifndef _PALIMGMGR_H_
#define _PALIMGMGR_H_

#include "Common.h"
#include "halScaler.h"
#include "utilStorageCfg.h"


#define LOG_PALIMGMGR_PARAMETER_IS_NULL(x)    if(x == NULL)\
                                              {\
                                                  LOG_MSG(db_ASSERT, "!!!(funcs:%s,line:%d) Parameter is NULL\r\n", __FUNCTION__, __LINE__);\
                                                  return ePAL_IMG_EXEC_CODE_NULLPARA;\
                                              }\

#define LOG_PALIMGMGR_FUNC_UNSUPPORTED     {\
                                               LOG_MSG(db_ASSERT, "!!!(funcs:%s,line:%d) function unsupported\r\n", __FUNCTION__, __LINE__);\
                                           }\

#define GEN_COMPILE_ERROR_IF_NO_HAL_FUNCTION

//======================= Error Code Start ======================//
typedef enum
{
    ePAL_IMG_EXEC_CODE_PASS,         /* pass */
    ePAL_IMG_EXEC_CODE_FAIL,
    ePAL_IMG_EXEC_CODE_INIT_FAIL,
    ePAL_IMG_EXEC_CODE_NOT_INIT,
    ePAL_IMG_EXEC_WINDOW_OVER_RANGE,

    ePAL_IMG_EXEC_CODE_DV_SCALER_INIT_FAIL,
    ePAL_IMG_EXEC_CODE_DV_RX_INIT_FAIL,
    ePAL_IMG_EXEC_CODE_DV_RX_UNSTABLE,
    ePAL_IMG_EXEC_CODE_NO_AUTO_PHASE,
    ePAL_IMG_EXEC_CODE_AUTO_PHASE_GOING,
    ePAL_IMG_EXEC_CODE_HDR_CHANGE,

    ePAL_IMG_EXEC_CODE_NULLPARA,
    ePAL_IMG_EXEC_CODE_UNSUPPORTED,  //no hal function

    ePAL_IMG_EXEC_CODE_NUMBER,

}ePAL_IMG_EXEC_CODE;
//======================= Error Code End =======================//
//G100_Steven_0016 start
typedef enum
{
    ePAL_COLOR_SETTING_CS,
    ePAL_COLOR_SETTING_CT,
	ePAL_COLOR_SETTING_GAMMA,
    ePAL_COLOR_SETTING_BRILLIENTCOLOR,
    ePAL_COLOR_SETTING_WHITEPEAKING,
    ePAL_COLOR_SETTING_COLORENHANCEMENT,
    ePAL_COLOR_SETTING_CWSPEED,
    ePAL_COLOR_SETTING_SKINCOLOR,
    ePAL_COLOR_SETTING_SHARPNESS,
    ePAL_COLOR_SETTING_BRIGHTNESS,
    ePAL_COLOR_SETTING_CONTRAST,
    ePAL_COLOR_SETTING_TINT,
    ePAL_COLOR_SETTING_SATURATION,
    ePAL_COLOR_SETTING_REDGAIN,
    ePAL_COLOR_SETTING_GREENGAIN,
    ePAL_COLOR_SETTING_BLUEGAIN,
    ePAL_COLOR_SETTING_REDOFFSET,
    ePAL_COLOR_SETTING_GREENOFFSET,
    ePAL_COLOR_SETTING_BLUEOFFSET

}ePAL_COLOR_SETTING;
//G100_Steven_0016 end

typedef enum
{
    ePAL_CLOCKGEN_SEL_C734,
    ePAL_CLOCKGEN_SEL_C790,
    ePAL_CLOCKGEN_SEL_C821_HD,
    ePAL_CLOCKGEN_SEL_C821_WU,
    ePAL_CLOCKGEN_SEL_C821_720P,
    ePAL_CLOCKGEN_SEL_C821_720P120,
    ePAL_CLOCKGEN_SEL_C821_800x600_120,
    ePAL_CLOCKGEN_SEL_C821_1080P120,
    ePAL_CLOCKGEN_SEL_C821_WUXGA120,
    ePAL_CLOCKGEN_SEL_LAST,
}ePAL_CLOCKGEN_SEL;

typedef enum
{
    ePAL_SCALER_RGB_ITEM_XPR,
    ePAL_SCALER_RGB_ITEM_LATENCY,
    ePAL_SCALER_RGB_ITEM_GEOMETRY,
    ePAL_SCALER_RGB_ITEM_PANELCHANGE,
    ePAL_SCALER_RGB_ITEM_FRC,
    ePAL_SCALER_RGB_ITEM_COUNT
}ePAL_SCALER_RGB_ITEM;

#define PAL_IMAGE_PASS 1
#define PAL_IMAGE_DATA_OUT_OF_RANGE -2

typedef enum
{
    eIMAGE_WINDOW_0,
    eIMAGE_WINDOW_1,

    eIMAGE_WINDOW_NUMBER,
}eIMAGE_WINDOW;

//=============== Struct Start =================//

typedef union
{
    //palImgMgr_PowerStandby
    struct
    {
        UINT8 ucCH;
    }sPowerStandby;

    //palImgMgr_Dump_Register
    struct
    {
        UINT8 ucCH;
    }sDump_Register;

    //palImgMgr_MeasureInput
    struct
    {
        UINT8 ucCH;
        UINT8 ucInput;
    }sMeasureInput;

    //palImgMgr_Config_NoSignalOutput
    struct
    {
        eSOURCE_WINDOW eWindow;
        UINT8 ucDisplayOutput;
    }sConfig_NoSignalOutput;




}uPALIMGMGR_INFO;




#if 0
typedef struct
{
    UINT16		        uiHActive;
    UINT16		        uiVActive;
    UINT16		        uiHStart;
    UINT16		        uiVStart;
} sPANEL_INFO;
#endif

//=============== Struct End ==================//

ePAL_IMG_EXEC_CODE palImgMgr_ExeResult(const char* FunctionName, eHAL_SCALER_EXEC_CODE eResult);
UINT32 palImgMgr_GetScalerErrCode(const UINT8 ucCH);
ePAL_IMG_EXEC_CODE palImgMgr_PowerStandby(uPALIMGMGR_INFO *psInfo);
ePAL_IMG_EXEC_CODE palImgMgr_PowerNormal(UINT8 ucCH, const ePANEL_ID ePanelId);
ePAL_IMG_EXEC_CODE palImgMgr_ConfigureForDisplay(UINT8 ucCH, const UINT8 ucColorFmt);
ePAL_IMG_EXEC_CODE palImgMgr_ConfigureForDisplay_ChangePIPLayout(UINT8 ucCH, const UINT8 ucColorFmt);
ePAL_IMG_EXEC_CODE palImgMgr_ConfigureColor(UINT8 ucCH);
ePAL_IMG_EXEC_CODE palImgMgr_ConfigureColorSpace(UINT8 ucCH, UINT8 ucSyncSub);
ePAL_IMG_EXEC_CODE palImgMgr_DisplaySFG(UINT8 ucCH, const UINT8 ucFillColor, const BOOL bEnable);
ePAL_IMG_EXEC_CODE palImgMgr_DisplaySplash(void);
ePAL_IMG_EXEC_CODE palImgMgr_SetOverlay(UINT8 ucCH, const BOOL bEnable);
ePAL_IMG_EXEC_CODE palImgMgr_SetOverlayCoef(UINT8 ucCH, const PUINT8 pucOverLay);
ePAL_IMG_EXEC_CODE palImgMgr_PortConfig(UINT8 ucCH);
ePAL_IMG_EXEC_CODE palImgMgr_Brightness_Set(UINT8 ucCH,UINT8 ucSetting);
ePAL_IMG_EXEC_CODE palImgMgr_Contrast_Set(UINT8 ucCH, UINT8 ucSetting);
INT8 palImgMgr_Dump_Register(uPALIMGMGR_INFO *psInfo);
ePAL_IMG_EXEC_CODE palImgMgr_Bypasse_Mode(void);
ePAL_IMG_EXEC_CODE palImgMgr_Test_Mode(void);
ePAL_IMG_EXEC_CODE palImgMgr_HUE_Set(UINT8 ucCH,UINT8 ucSetting);
ePAL_IMG_EXEC_CODE palImgMgr_Saturation_Set(UINT8 ucCH,UINT8 ucSetting);
ePAL_IMG_EXEC_CODE palImgMgr_PIP_PBP_Layout_Set(UINT8 ucLayout);
ePAL_IMG_EXEC_CODE palImgMgr_PIP_PBP_Size_Set(UINT8 ucSize);
UINT8 palImgMgr_PIP_PBP_Layout_Get(void);
UINT8 palImgMgr_PIP_PBP_Size_Get(void);
void palImgMgr_Resync_Init(UINT8 eCH);
ePAL_IMG_EXEC_CODE palImgMgr_SYNC_Lock_Get(UINT8 ucCH);
ePAL_IMG_EXEC_CODE palImgMgr_CheckChannelSyncCount(UINT8 *ucIsSimilar);
ePAL_IMG_EXEC_CODE palImgMgr_Freeze_Set(UINT8 ucCH, BOOL bFreezeEn);
ePAL_IMG_EXEC_CODE palImgMgr_Freeze_Get(UINT8 ucCH, BOOL *bFreezeEn);
ePAL_IMG_EXEC_CODE palImgMgr_Panel_Set(UINT8 ucCH, ePANEL_ID ePanelId);
UINT32 palImgMgr_Panel_Colck_Set(ePANEL_ID ePanelId);
ePAL_IMG_EXEC_CODE palImgMgr_AutoPhaseStart(UINT8 ucCH);
ePAL_IMG_EXEC_CODE palImgMgr_AutoPhase(UINT8 ucCH);
ePAL_IMG_EXEC_CODE palImgMgr_SetInputSource(UINT8 ucCH, UINT8 ucInputSource);
ePAL_IMG_EXEC_CODE palImgMgr_RedOffset_Set(UINT8 ucCH, UINT8 ucSetting);
ePAL_IMG_EXEC_CODE palImgMgr_GreenOffset_Set(UINT8 ucCH, UINT8 ucSetting);
ePAL_IMG_EXEC_CODE palImgMgr_BlueOffset_Set(UINT8 ucCH, UINT8 ucSetting);
ePAL_IMG_EXEC_CODE palImgMgr_RedGain_Set(UINT8 ucCH, UINT8 ucSetting);
ePAL_IMG_EXEC_CODE palImgMgr_GreenGain_Set(UINT8 ucCH, UINT8 ucSetting);
ePAL_IMG_EXEC_CODE palImgMgr_BlueGain_Set(UINT8 ucCH, UINT8 ucSetting);
ePAL_IMG_EXEC_CODE palImgMgr_RedOffset_Value_Set(UINT8 ucCH, UINT8 ucSetting);
ePAL_IMG_EXEC_CODE palImgMgr_GreenOffset_Value_Set(UINT8 ucCH, UINT8 ucSetting);
ePAL_IMG_EXEC_CODE palImgMgr_BlueOffset_Value_Set(UINT8 ucCH, UINT8 ucSetting);
ePAL_IMG_EXEC_CODE palImgMgr_RedGain_Value_Set(UINT8 ucCH, UINT8 ucSetting);
ePAL_IMG_EXEC_CODE palImgMgr_GreenGain_Value_Set(UINT8 ucCH, UINT8 ucSetting);
ePAL_IMG_EXEC_CODE palImgMgr_BlueGain_Value_Set(UINT8 ucCH, UINT8 ucSetting);
ePAL_IMG_EXEC_CODE palImgMgr_Brightness_Value_Set(UINT8 ucCH, UINT8 ucSetting);
ePAL_IMG_EXEC_CODE palImgMgr_Contrast_Value_Set(UINT8 ucCH, UINT8 ucSetting);
ePAL_IMG_EXEC_CODE palImgMgr_HUE_Value_Set(UINT8 ucCH, UINT8 ucSetting);
ePAL_IMG_EXEC_CODE palImgMgr_Saturation_Value_Set(UINT8 ucCH, UINT8 ucSetting);
ePAL_IMG_EXEC_CODE palImgMgr_Init_EEPROM_Setting(sUSER_SYSTEM_SETTING m_UserSetting);
ePAL_IMG_EXEC_CODE palImgMgr_SizePresets_Set(UINT8 ucSetting);
ePAL_IMG_EXEC_CODE palImgMgr_SizePresets_SetOnlyValue(UINT8 ucSetting);
ePAL_IMG_EXEC_CODE palImgMgr_Overscan_Set(UINT8 ucSetting);
ePAL_IMG_EXEC_CODE palImgMgr_PixelTrack_Set(UINT8 ucSetting);
ePAL_IMG_EXEC_CODE palImgMgr_PixelPhase_Set(UINT8 ucSetting);
ePAL_IMG_EXEC_CODE palImgMgr_PixelPhase_Action(void);
UINT8 palImgMgr_OSD_PixelPhase_Get(void);
ePAL_IMG_EXEC_CODE palImgMgr_HorzPosition_Set(UINT8 ucSetting);
ePAL_IMG_EXEC_CODE palImgMgr_VertPosition_Set(UINT8 ucSetting);
ePAL_IMG_EXEC_CODE palImgMgr_DigitalHorzZoom_Set(UINT16 uiSetting);
ePAL_IMG_EXEC_CODE palImgMgr_DigitalVertZoom_Set(UINT16 uiSetting);
ePAL_IMG_EXEC_CODE palImgMgr_DigitalHorzShift_Set(UINT8 ucSetting);
ePAL_IMG_EXEC_CODE palImgMgr_DigitalVertShift_Set(UINT8 ucSetting);
ePAL_IMG_EXEC_CODE palImgMgr_AutoImage_Set(UINT8 ucSetting);
UINT8 palImgMgr_InputSource_Get(UINT8 ucCH);
UINT8 palImgMgr_SizePresets_Get(UINT8 ucCH);
UINT8 palImgMgr_PictureSettings_Get(UINT8 ucCH);
UINT8 palImgMgr_PictureSettings_PreUser_Get(UINT8 ucCH);
UINT8 palImgMgr_SignalType_Get(UINT8 ucCH);
ePAL_IMG_EXEC_CODE palImgMgr_ColorSpace_Set(UINT8 ucCH, UINT8 ucSetting);
UINT8 palImgMgr_ColorSpace_Get(UINT8 ucCH);
UINT8 palImgMgr_ColorSpace_ScalerValueGet(UINT8 ucCH);
ePAL_IMG_EXEC_CODE palImgMgr_Detail_Set(UINT8 ucCH, UINT8 ucSetting);
ePAL_IMG_EXEC_CODE palImgMgr_3DEnable_Set(UINT8 ucSetting);
UINT8 palImgMgr_3DEnable_Get(void);
ePAL_IMG_EXEC_CODE palImgMgr_3DMode_Set(UINT8 ucSetting);
UINT8 palImgMgr_3DMode_Get(void);
INT8 palImgMgr_3DInvert_Set(UINT8 ucSetting);
INT8 palImgMgr_3DSyncOut_Set(UINT8 ucSetting);
UINT8 palImgMgr_3DSyncOut_Get(UINT8 eCH);
INT8 palImgMgr_3DSyncIn_Set(UINT8 ucSetting);
UINT8 palImgMgr_3DSyncIn_Get(UINT8 eCH);
UINT8 palImgMgr_IS_3D_Enable(void);
UINT8 palImgMgr_3D_Type(void);
ePAL_IMG_EXEC_CODE palImgMgr_FrameDelay_Set(UINT16 uiSetting);
ePAL_IMG_EXEC_CODE palImgMgr_3DSyncDelay_Set(UINT16 uiSetting);
ePAL_IMG_EXEC_CODE palImgMgr_SkinColor_Set(UINT8 ucCH, UINT8 ucSetting);
ePAL_IMG_EXEC_CODE palImgMgr_TemporalNoiseReduction_Set(UINT8 ucSetting);
ePAL_IMG_EXEC_CODE palImgMgr_MPEGNoiseReduction_Set(UINT8 ucSetting);
ePAL_IMG_EXEC_CODE palImgMgr_DetectFilm_Set(UINT8 ucSetting);
ePAL_IMG_EXEC_CODE palImgMgr_RGB_Gain_Offset_Set(void);
ePAL_IMG_EXEC_CODE palImgMgr_PictureSettings_Set(UINT8 ucCH, UINT8 ucSetting);
void palImgMgr_PreUserMode_Set(UINT8 ucSetting);
ePAL_IMG_EXEC_CODE palImgMgr_ContrastEnhancement_Set(UINT8 ucSetting);
INT8 palImgMgr_HDREnable_Set(UINT8 ucSetting);
eEXEC_CODE palImg_HDR_Set(UINT8 ucHDRDetect,   //HDR Auto , Disable
                                eCM_HDR_GAMMA_FUNCTION eHDRGamma,
                                eCM_HDR_MODE eHDRMode,
                                UINT16 uiNit,        //mastering max luminance
                                UINT16 uiMaxLimit);   //monitor luminance

ePAL_IMG_EXEC_CODE palImgMgr_HDR10_Process(UINT16 uiHDRContentMaxNit);
ePAL_IMG_EXEC_CODE palImgMgr_HLG_Process(UINT16 uiMonitorMaxNit);
ePAL_IMG_EXEC_CODE palImgMgr_HDR_Disable(void);
ePAL_IMG_EXEC_CODE palImgMgr_SDREnhance_Process(void);
void palImgMgr_HdrMaxMasteringLuminanceGet(UINT16 *ucData);
INT8 palImgMgr_MEMC_Set(UINT8 ucSetting);
ePAL_IMG_EXEC_CODE palImgMgr_ImageFreeze_Set(UINT8 ucSetting);
ePAL_IMG_EXEC_CODE palImgMgr_Gamma_Set(UINT8 ucCH, UINT8 ucSetting);
ePAL_IMG_EXEC_CODE palImgMgr_WhitePeaking_Set(UINT8 ucCH, UINT8 ucSetting);
ePAL_IMG_EXEC_CODE palImgMgr_ColorTemperature_Set(UINT8 ucCH, UINT8 ucSetting);
INT8 palImgMgr_EdgeEnhancement_Set(UINT8 ucSetting);
INT8 palImgMgr_ColorWheelSpeed_Get(UINT8 ucCH);
ePAL_IMG_EXEC_CODE palImgMgr_ColorWheelSpeed_Set(UINT8 ucCH, UINT8 ucSetting);
ePAL_IMG_EXEC_CODE palImgMgr_ColorEnhancement_Set(UINT8 ucCH, UINT8 ucSetting);
ePAL_IMG_EXEC_CODE palImgMgr_ColorEnhancement_Get(UINT8 ucCH, UINT8 *ucSetting);
ePAL_IMG_EXEC_CODE palImgMgr_HSGEnable_Set(UINT8 ucSetting);
ePAL_IMG_EXEC_CODE palImgMgr_HSGEnable_Get(UINT8 *ucSetting);
ePAL_IMG_EXEC_CODE palImgMgr_HSG_Set(UINT8 ucCH, sHSG_SETTING ucSetting);
ePAL_IMG_EXEC_CODE palImgMgr_HSG_All_Set(UINT8 ucInputSource, UINT8 ucPresetMode, sHSG_SETTING ucSetting);
ePAL_IMG_EXEC_CODE palImgMgr_HSG_Get(sHSG_SETTING *m_sHSG_Setting);
ePAL_IMG_EXEC_CODE palImgMgr_PIP_PBP_Enable_Set(UINT8 ucSetting);
UINT8 palImgMgr_PIP_PBP_Enable_Get(void);
ePAL_IMG_EXEC_CODE palImgMgr_IintChannelSetting(UINT8 ucCH);
UINT16 palImgMgr_DigitalHorzZoom_Get(void);
UINT16 palImgMgr_DigitalVertZoom_Get(void);
UINT8 palImgMgr_DigitalHorzShift_Get(void);
UINT8 palImgMgr_DigitalVertShift_Get(void);
ePAL_IMG_EXEC_CODE palImgMgr_Detail_Value_Set(UINT8 ucCH, UINT8 ucSetting);
UINT8 palImgMgr_Detail_Value_Get(UINT8 ucCH);
ePAL_IMG_EXEC_CODE palImgMgr_SkinColor_Value_Set(UINT8 ucCH, UINT8 ucSetting);
UINT8 palImgMgr_SkinColor_Value_Get(UINT8 ucCH);
INT16 palImgMgr_Brightness_Value_Get(UINT8 ucCH);
DOUBLE palImgMgr_Contrast_Value_Get(UINT8 ucCH);
INT16 palImgMgr_HUE_Value_Get(UINT8 ucCH);
DOUBLE palImgMgr_Saturation_Value_Get(UINT8 ucCH);
INT8 palImgMgr_RedOffset_Value_Get(UINT8 ucCH);
INT8 palImgMgr_GreenOffset_Value_Get(UINT8 ucCH);
INT8 palImgMgr_BlueOffset_Value_Get(UINT8 ucCH);
UINT8 palImgMgr_RedGain_Value_Get(UINT8 ucCH);
UINT8 palImgMgr_GreenGain_Value_Get(UINT8 ucCH);
UINT8 palImgMgr_BlueGain_Value_Get(UINT8 ucCH);
ePAL_IMG_EXEC_CODE palImgMgr_Temporal_NR_Value_Set(UINT8 ucSetting);
ePAL_IMG_EXEC_CODE palImgMgr_MPEG_NR_Value_Set(UINT8 ucSetting);
ePAL_IMG_EXEC_CODE palImgMgr_DetectFilm_Value_Set(UINT8 ucSetting);
ePAL_IMG_EXEC_CODE palImgMgr_PictureSettings_Value_Set(UINT8 ucCH, UINT8 ucSetting);
ePAL_IMG_EXEC_CODE palImgMgr_MeasureInput(uPALIMGMGR_INFO *psInfo);
ePAL_IMG_EXEC_CODE palImgMgr_MeasureCheck(UINT8 ucCH);
ePAL_IMG_EXEC_CODE palImgMgr_OSD_TestPattern(const UINT8 ucTestPattern);
ePAL_IMG_EXEC_CODE palImgMgr_OSD_TestPattern_Set(UINT8 ucCH, UINT8 ucSetting);
ePAL_IMG_EXEC_CODE palImgMgr_Service_TestPattern(const UINT8 ucTestPattern);
ePAL_IMG_EXEC_CODE palImgMgr_Service_TestPattern_Set(UINT8 ucCH, UINT8 ucSetting);
UINT8 palImgMgr_PictureSettings_Get_NotReturnUser(UINT8 ucCH);
ePAL_IMG_EXEC_CODE palImgMgr_InputPixelClock_Get(UINT8 ucCH, UINT8 *ucValue);
ePAL_IMG_EXEC_CODE palImgMgr_InputResolution_Get(UINT8 ucCH, UINT8 *ucValue);
ePAL_IMG_EXEC_CODE palImgMgr_InputHorzRefresh_Get(UINT8 ucCH, UINT8 *ucValue);
ePAL_IMG_EXEC_CODE palImgMgr_InputVertRefresh_Get(UINT8 ucCH, UINT8 *ucValue);
ePAL_IMG_EXEC_CODE palImgMgr_InputVertRefresh2_Get(UINT8 ucCH, UINT16 *uiValue);
ePAL_IMG_EXEC_CODE palImgMgr_InputVertRefresh2_Get_FromSyncCount(UINT8 ucCH, UINT16 *uiValue);
ePAL_IMG_EXEC_CODE palImgMgr_InputSignalFormat_Get(UINT8 ucCH, UINT8 *ucValue);
ePAL_IMG_EXEC_CODE palImgMgr_InputAspectRatio_Get(UINT8 ucCH, UINT8 *ucValue);
ePAL_IMG_EXEC_CODE palImgMgr_FrontEndColorSpace_Set(UINT8 ucCH, UINT8 ucValue);
ePAL_IMG_EXEC_CODE palImgMgr_FrontEndColorSpaceInfo_Get(UINT8 ucCH, UINT8* ucValue);
ePAL_IMG_EXEC_CODE palImgMgr_FrontEndScanMode_Set(UINT8 ucCH, UINT8 ucValue);
ePAL_IMG_EXEC_CODE palImgMgr_FrontEndScanMode_Clear(UINT8 ucCH);
BOOL palImgMgr_ForcedSyncResetType_Get(void);
ePAL_IMG_EXEC_CODE palImgMgr_ForcedSyncResetDisable_Set(void);
BOOL palImgMgr_IsInterlaced_Get(UINT8 ucCH);
ePAL_IMG_EXEC_CODE palImgMgr_FrontEndVideoYUV_Set(UINT8 ucCH, UINT8 ucValue);
ePAL_IMG_EXEC_CODE palImgMgr_Horz_Start_Position_Get(UINT8 ucCH, UINT16* uiVal);
ePAL_IMG_EXEC_CODE palImgMgr_Vert_Start_Position_Get(UINT8 ucCH, UINT16* uiVal);
ePAL_IMG_EXEC_CODE palImgMgr_VGA_H_Total_Get(UINT8 ucCH, UINT16* uiVal);
ePAL_IMG_EXEC_CODE palImgMgr_VGA_V_Total_Get(UINT8 ucCH, UINT16* uiVal);
ePAL_IMG_EXEC_CODE palImgMgr_VGA_SYNC_TYPE_Set(UINT8 ucCH, UINT8 ucVal);
UINT8 palImgMgr_VGA_SYNC_TYPE_Get(UINT8 ucCH);
ePAL_IMG_EXEC_CODE palImgMgr_SyncThreshold_Set(UINT8 ucSetting);
UINT8 palImgMgr_SyncThreshold_Get(void);
UINT32 palImgMgr_uiHFreq_Get(UINT8 ucCH);
UINT8 palImgMgr_HDR_Get(UINT8 ucCH);
ePAL_IMG_EXEC_CODE palImgMgr_Position_Run(UINT8 ucCH);
UINT8 palImgMgr_HorzPositionWorkValueGet(UINT8 ucCH);
UINT8 palImgMgr_VertPositionWorkValueGet(UINT8 ucCH);
ePAL_IMG_EXEC_CODE palImgMgr_AutoPsitionStart(UINT8 ucCH, BOOL bAutoEnable);
ePAL_IMG_EXEC_CODE palImgMgr_Config_NoSignalOutput(uPALIMGMGR_INFO *psInfo);
UINT16 palImgMgr_Input_H_Active_Get(UINT8 ucCH);
UINT16 palImgMgr_Input_V_Active_Get(UINT8 ucCH);
UINT16 palImgMgr_Input_H_Total_Get(UINT8 ucCH);
UINT16 palImgMgr_Input_V_Total_Get(UINT8 ucCH);
UINT16 palImgMgr_Input_H_Start_Get(UINT8 ucCH);
UINT16 palImgMgr_Input_V_Start_Get(UINT8 ucCH);
ePAL_IMG_EXEC_CODE palImgMgr_DDR_InputDataAreaTotalGet(UINT8 eCH, RECT DataArea, UINT32 *ulTotalRed, UINT32 *ulTotalGreen, UINT32 *ulTotalBlue);
ePAL_IMG_EXEC_CODE palImgMgr_InputADC_Cali_CalibrationEnableSet(UINT8 eCH, UINT8 ucADC_CAl_Enable);
UINT8 palImgMgr_Input_3D_Format_Get(void);
void palImgMgr_Input_3D_Format_Set(UINT8 ucVal);
ePAL_IMG_EXEC_CODE palImgMgr_Input_2D_SyncOut_Enable(UINT8 ucVal);
ePAL_IMG_EXEC_CODE palImgMgr_3D_InputPort_Set(void);
UINT8 palImgMgr_Input_3D_Format_Config_Get(void);
ePAL_IMG_EXEC_CODE palImgMgr_InputHorzPeriod_Get(UINT8 ucCH, UINT32* udVal);
ePAL_IMG_EXEC_CODE palImgMgr_VGA_ModeTable_Get(UINT8 ucCH, UINT8* ucVal);
ePAL_IMG_EXEC_CODE palImgMgr_VGA_ModeTableNumber_Get(UINT8 ucCH, UINT16* uiVal);
ePAL_IMG_EXEC_CODE palImgMgr_ModeAdjusmenttHorzStart_Get(UINT16* uiVal);
ePAL_IMG_EXEC_CODE palImgMgr_ModeAdjusmenttVertStart_Get(UINT16* uiVal);
ePAL_IMG_EXEC_CODE palImgMgr_ModeAdjusmenttHorzStart_Set(UINT16 uiVal);
ePAL_IMG_EXEC_CODE palImgMgr_ModeAdjusmenttVertStart_Set(UINT16 uiVal);
ePAL_IMG_EXEC_CODE palImgMgr_ModeAdjusmenttEnableSetting(void);
ePAL_IMG_EXEC_CODE palImgMgr_ModeAdjusmenttDisableSetting(void);
ePAL_IMG_EXEC_CODE palImgMgr_Init_Mode_Adjustment_EEPROM_Setting(sTIMING_TABLE sTiming_Table);
ePAL_IMG_EXEC_CODE palImgMgr_Input3D_Timing_Get(UINT8 ucCH, UINT8* ucVal);
ePAL_IMG_EXEC_CODE palImgMgr_ConfigureForPosition(UINT8 ucCH);
ePAL_IMG_EXEC_CODE palImgMgr_ConfigureForScaler(UINT8 ucCH);
ePAL_IMG_EXEC_CODE palImgMgr_EdgeMask_Set(UINT8 ucSetting);
void palImgMgr_ProAV_ColorSpaceConversion(UINT8 ucCH, UINT8 ucColorSpace, UINT8 ucPixelMode, UINT8 ucRGBMode);
eRESULT palImgMgr_ProAV_ColorSpaceSetting(UINT8 ucCH, UINT8 ucColorSpace);
ePAL_IMG_EXEC_CODE palImgMgr_Monitor(UINT8 ucCH);
ePAL_IMG_EXEC_CODE palImgMgr_AVIInfoFrame(UINT8 ucCH);
ePAL_IMG_EXEC_CODE palImgMgr_SourceLock(UINT8 ucCH);
ePAL_IMG_EXEC_CODE palImgMgr_InputSelectSet(UINT8 ucCH, UINT8 ucSelect, BOOL BlackSource);
ePAL_IMG_EXEC_CODE palImgMgr_BackupRxPortSet(UINT8 ucCH);
ePAL_IMG_EXEC_CODE palImgMgr_FrameSyncModeSet(UINT8 ucCH, UINT8 eMode);
ePAL_IMG_EXEC_CODE palImgMgr_FrameSyncModeGet(UINT8* cMode);
ePAL_IMG_EXEC_CODE palImgMgr_InputFrameRateSetting(UINT8 ucCH);
void palImgMgr_CurrentSourceRxPortSet(UINT8 ucCH, UINT8 ucRxPort);
UINT8 palImgMgr_CurrentSourceRxPortGet(UINT8 ucCH, UINT8 ucSelect);
void palImgMgr_RxPortClear(UINT8 ucRxPort);
ePAL_IMG_EXEC_CODE palImgMgr_RxPortCheck(UINT8 ucRxPort, UINT8 ucChangeSourceReset);
ePAL_IMG_EXEC_CODE palImgMgr_RxPortCheckPolling(void);
ePAL_IMG_EXEC_CODE palImgMgr_RxPortIsReady(UINT8 ucCH, UINT8 ucSelect);
void palImgMgr_RxPortDetTiming(UINT8 ucRxPort, UINT8 *psDetTiming);
UINT8 palImgMgr_InputCompare(UINT8 ucCH, UINT8 ucSelect);
UINT8 palImgMgr_SclDatapathSet(UINT8 dataPath);
UINT8 palImgMgr_SclDatapathCompare(UINT8 dataPath);
UINT8 palImgMgr_SclVopWindowOffSet(UINT8 ucCH, bool bOff);
UINT8 palImgMgr_3DModeGet(UINT8 ucRxPort);
UINT8 palImgMgr_InputPortIsReady(UINT8 ucCH, UINT8 ucSelect);
UINT8 palImgMgr_LogoCaptureSet(void);
UINT8 palImgMgr_LogoDisplayEn(bool bEnable);
UINT8 palImgMgr_ScreenOff(UINT8 ucCH, BOOL bEnable);
UINT8 palImgMgr_Dram2Flash(UINT32 dramAddr, UINT32 flashAddr, UINT32 len);
UINT8 palImgMgr_Flash2Dram(UINT32 dramAddr, UINT32 flashAddr, UINT32 len);
UINT8 palImgMgr_BackupInputCheck(UINT8 ucMain, UINT8 ucSub);
void palImgMgr_RxPortInfoPrint(void);
UINT16 palImgMgr_FpgaTempGet(void);
void palImgMgr_FpgaTempADCRst(void);
void palImgMgr_BackupPrimaryInput_Set(UINT8 ucInput);
UINT8 palImgMgr_BackupPrimaryInput_Get(void);
void palImgMgr_BackupSecondaryInput_Set(UINT8 ucInput);
UINT8 palImgMgr_BackupSecondaryInput_Get(void);
UINT8 palImgMgr_CurrentBackupSourceGet(void);
UINT8 palImgMgr_LogoCapturePanel(UINT16 uiPanel);
BOOL palImgMgr_LowLatencyEnable(void);
UINT8 palImgMgr_LensLockAllMotors_Get(void);
ePAL_IMG_EXEC_CODE palImgMgr_AntiSmearSet(UINT8 cMode, bool bEnable);
ePAL_IMG_EXEC_CODE palImgMgr_MEMCBypassSet(UINT8 ucEnable);
ePAL_IMG_EXEC_CODE palImgMgr_MEMCColorSet(UINT8 ucColor);
ePAL_IMG_EXEC_CODE palImgMgr_MEMCMotionSet(UINT8 ucMotion);
ePAL_IMG_EXEC_CODE palImgMgr_MEMCDemoSet(UINT8 ucDemo);
void palImgMgr_BackupSwitchSet(UINT8 ucValue);
UINT8 palImgMgr_BackupSwitchGet(void);
ePAL_IMG_EXEC_CODE palImgMgr_BackupSizePresetSetting(void);
BOOL palImgMgr_AutoExt_3D_SyncSetting(void);
void palImgMgr_ConfigurePanelSize(uint16 uiPanelID);
BOOL palImgMgr_Rx_Compare(UINT8 ucCH, UINT8 ucSelect);
ePAL_IMG_EXEC_CODE palImgMgr_PixelShiftEnable_Set(UINT8 ucEnable);
ePAL_IMG_EXEC_CODE palImgMgr_PixelShiftCalPattern_Set(UINT8 ucEnable);
ePAL_IMG_EXEC_CODE palImgMgr_Default4K3DSEnable_Set(UINT8 ucMode);
UINT8 palImgMgr_ColorSetting_Get(eHAL_COLOR_SETTING  ucNode, UINT8 ucCH);
int palImgMgr_FpgaDataRGBLevelGet(uint16 *RGB_LEVEL_Max, uint16 *RGB_LEVEL_Min);
void palImgMgr_RxPortReset(void);
void palImgMgr_DisableRGB(ePAL_SCALER_RGB_ITEM RGB_Item);
void palImgMgr_EnableRGB(void);
void palImgMgr_RGBEnableItemSet(ePAL_SCALER_RGB_ITEM RGB_Item);
void palImgMgr_RGBMaskSet(UINT8 ucMask);
ePAL_IMG_EXEC_CODE palImgMgr_4K3D_DualPipeOrientationSet(UINT8 ucFRC_Bypass,BOOL bRear,BOOL bCeiling);
void palImgMgr_RGBSwitchSet(bool bSwitch);
BOOL palImgMgr_RGBSwitchGet(void);
eRESULT palImgMgr_Version_Get(uint32 *ulVersiong);
void palImgMgr_PixSftModSet(bool bSwitch);
BOOL palImgMgr_PixModIsYUV420(UINT8 ucCH, UINT8 ucSelect);
BOOL palImgMgr_IsVsync120or100Hz_Get(UINT8 ucCH);
BOOL palImgMgr_IsAspectRation16_9_Get(void);
BOOL palImgMgr_IsAspectRation16_10_Get(void);
ePAL_IMG_EXEC_CODE palImgMgr_SourceOutputOff(UINT8 ucCH);
UINT8 palImgMgr_OutputVsyncFreqGetForTwistOn(void);
ePAL_IMG_EXEC_CODE palImgMgr_Low_Latency_Set(eLOW_LATENCY_MODE eLLMode);
eLOW_LATENCY_MODE palImgMgr_Low_Latency_Get(void);
UINT32 palImgMgr_Reg0_Read_Get(void);
UINT32 palImgMgr_WarpReg0_Read_Get(void);
UINT8 palImgMgr_OPD_Test(UINT8 ucData);
ePAL_IMG_EXEC_CODE palImgMgr_CaptureImage(void);
ePAL_IMG_EXEC_CODE palImgMgr_CaptureImage_Show(BOOL bEnable);
ePAL_IMG_EXEC_CODE palImgMgr_CaptureImage_Del(BOOL bSourceLock);
ePAL_IMG_EXEC_CODE palImgMgr_ScreenSaveToImage(void);
ePAL_IMG_EXEC_CODE palImgMgr_Auto_Fill_Screen_Init(void);
ePAL_IMG_EXEC_CODE palImgMgr_Auto_Fill_Screen_Setting(BOOL bSetting);
void palImgMgr_Test(UINT16 *puiTtlCnt, UINT16 *puiErrCnt);
void palImgMgr_PIPPBPPaniel_InfoSet(BYTE cCH, UINT16 uiHActive, UINT16 uiVActive, UINT16 uiHStart, UINT16 uiVStart);
void palImgMgr_PowerNormalReadyGet(BYTE ucCH);
void palImgMgr_3D_Reset(void);
UINT32 palImgMgr_ICP_Register_Read(UINT32 ulAddr);
void palImgMgr_ICP_Register_Write(UINT32 ulAddr, UINT32 ucData);
ePAL_IMG_EXEC_CODE palImgMgr_ClockGen_Init(ePAL_CLOCKGEN_SEL eClockgenSel);
void palImgMgr_PowerNormalDone(BOOL bDone);
UINT16 palImgMgr_FrameDelay_Max_Get(void);
UINT16 palImgMgr_PanelOutputVTotal_Get(void);
UINT8 palImgMgr_WB_Black_Screen_Set(UINT8 ucData);
UINT8 palImgMgr_WB_Black_Screen_Get(void);
void palImgMgr_WB_OutputEnableSet(UINT8 ucEnable);
UINT8 palImgMgr_WB_OutputEnableGet(void);
void palImgMgr_WB_OutputEnable_Set(UINT8 ucEnable);
UINT8 palImgMgr_WB_OutputChenged_Get(void);
void palImgMgr_WB_Freeze(UINT8 ucEnable);
void palImgMgr_HDRDemo(void);
BOOL palImgMgr_Get_Dump_Register_Flag(void); //H30K_Doulas_0067
void palImgMgr_Set_Dump_Register_Flag(BOOL bDump_Register_Flag); //H30K_Doulas_0067
INT16 palImgMgr_Dump_Register_Poll(UINT16 uiTick); //H30K_Doulas_0067


#endif //_PALIMGMGR_H_

