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

    eHAL_SCALER_EXEC_CODE_NUMBER,

}eHAL_SCALER_EXEC_CODE;
//======================= Error Code End =======================//

typedef enum
{
    eHAL_SCALER_RGB_ITEM_XPR,
    eHAL_SCALER_RGB_ITEM_LATENCY,
    eHAL_SCALER_RGB_ITEM_GEOMETRY,
    eHAL_SCALER_RGB_ITEM_PANELCHANGE,
    eHAL_SCALER_RGB_ITEM_FRC,
    eHAL_SCALER_RGB_ITEM_COUNT
}eHAL_SCALER_RGB_ITEM; //A70LK_Casper_0008

#define PROAV_SCALER_COLOR_SPACE_RGB_FULL       0x00
#define PROAV_SCALER_COLOR_SPACE_REC601         0x01
#define PROAV_SCALER_COLOR_SPACE_REC709         0x02
#define PROAV_SCALER_COLOR_SPACE_REC2020        0x03
#define PROAV_SCALER_COLOR_SPACE_INVALID        0x04

#define PROAV_SCALER_COLOR_PIXEL_MODE_RGB444    0x00
#define PROAV_SCALER_COLOR_PIXEL_MODE_YUV422    0x02
#define PROAV_SCALER_COLOR_PIXEL_MODE_YUV444    0x01
#define PROAV_SCALER_COLOR_PIXEL_MODE_YUV420    0x03

#define PROAV_SCALER_COLOR_RGB_FULL  0x00
#define PROAV_SCALER_COLOR_RGB_LIMIT 0x01

#define HAL_SCALER_PASS 1
#define HAL_SCALER_DATA_OUT_OF_RANGE -2

//A70LK_Simon_0017
#define SCALER_RGB_MASK_R BIT2
#define SCALER_RGB_MASK_G BIT1
#define SCALER_RGB_MASK_B BIT0

//=============== Struct Start =================//
typedef struct
{
    UINT8   ucColorsSpace;
    UINT8   ucPixelMode;
    UINT8   ucRGBFull;
    UINT8   uc3DMode;
    UINT8   ucHDR;
    DOUBLE  dFrameRate;
}sPROAV_AVI;

typedef struct
{
    UINT16 uiHTotal;
    UINT16 uiHStart;
    UINT16 uiHSize;
    UINT16 uiVTotal;
    UINT16 uiVStart;
    UINT16 uiVSize;
    UINT8  bInt:1;
    UINT8  bReserve:5;
    UINT8  bVPol:1;
    UINT8  bHPol:1;
    UINT8  cHWid;
    UINT8  cVWid;
}sRX_AVI_INFO;

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

}eHAL_COLOR_SETTING;	//A70LK_Doulas_0004

//=============== Struct End ==================//
UINT32 halScaler_VersionGet(void);

UINT32 halScaler_GetScalerErrCode(const UINT8 ucCH);

eHAL_SCALER_EXEC_CODE halScaler_Init(UINT8 ucCH);//A70LV_Doulas_0003//Init 4 channels in once, no matter 4K, 2K

eHAL_SCALER_EXEC_CODE halScaler_PowerStandby(UINT8 ucCH);
eHAL_SCALER_EXEC_CODE halScaler_PowerNormal(UINT8 ucCH, const ePANEL_ID ePanelId);
eHAL_SCALER_EXEC_CODE halScaler_ConfigureForDisplay(UINT8 ucCH, const UINT8 ucColorFmt);
eHAL_SCALER_EXEC_CODE halScaler_ConfigureColor(UINT8 ucCH);
eHAL_SCALER_EXEC_CODE halScaler_ConfigureColorSpace(UINT8 ucCH, UINT8 ucSyncSub);
eHAL_SCALER_EXEC_CODE halScaler_DisplaySFG(UINT8 ucCH, const UINT8 ucFillColor, const BOOL bEnable);

eHAL_SCALER_EXEC_CODE halScaler_DisplaySplash(void);
eHAL_SCALER_EXEC_CODE halScaler_PortConfig(UINT8 ucCH);

eHAL_SCALER_EXEC_CODE halScaler_SetOverlay(UINT8 ucCH, const BOOL bEnable);
eHAL_SCALER_EXEC_CODE halScaler_SetOverlayCoef(UINT8 ucCH, const PUINT8 pucOverLay);

eHAL_SCALER_EXEC_CODE halScaler_Brightness_Set(UINT8 ucCH,UINT8 ucSetting);  //A70LV_Doulas_0022 modify
eHAL_SCALER_EXEC_CODE halScaler_Contrast_Set(UINT8 ucCH,UINT8 ucSetting);    //A70LV_Doulas_0022 modify
INT8 halScaler_Dump_Register(UINT8 cBank);     //A70LV_Doulas_0002
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
eHAL_SCALER_EXEC_CODE halScaler_Freeze_Set(UINT8 ucCH,BOOL bFreezeEn);  //A70LV_Doulas_0004
eHAL_SCALER_EXEC_CODE halScaler_Freeze_Get(UINT8 ucCH,BOOL *bFreezeEn);  //A70LV_Doulas_0004
eHAL_SCALER_EXEC_CODE halScaler_Panel_Set(UINT8 ucCH,ePANEL_ID ePanelId);  //A70LV_Doulas_0005
UINT32 halScaler_Panel_Colck_Set(ePANEL_ID ePanelId);    //A70LV_Doulas_0005

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
eHAL_SCALER_EXEC_CODE halScaler_SizePresets_SetOnlyValue(UINT8 ucSetting); //A70LK_Doulas_0015 Add
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
UINT8 halScaler_PictureSettings_PreUser_Get(UINT8 ucCH);			  //A70LK_Doulas_0004
UINT8 halScaler_SignalType_Get(UINT8 ucCH);
eHAL_SCALER_EXEC_CODE halScaler_ColorSpace_Set(UINT8 ucCH,UINT8 ucSetting);
UINT8 halScaler_ColorSpace_Get(UINT8 ucCH);   //A70LV_Doulas_0069
UINT8 halScaler_ColorSpace_ScalerValueGet(UINT8 ucCH);   //A70LV_Doulas_0069
eHAL_SCALER_EXEC_CODE halScaler_Detail_Set(UINT8 ucCH,UINT8 ucSetting);
INT8 halScaler_3DEnable_Set(UINT8 ucSetting);
UINT8 halScaler_3DEnable_Get(void);
INT8 halScaler_3DMode_Set(UINT8 ucSetting);
UINT8 halScaler_3DMode_Get(void);
INT8 halScaler_3DInvert_Set(UINT8 ucSetting);
INT8 halScaler_3DSyncOut_Set(UINT8 ucSetting);
UINT8 halScaler_3DSyncOut_Get(UINT8 eCH);
INT8 halScaler_3DSyncIn_Set(UINT8 ucSetting);
UINT8 halScaler_3DSyncIn_Get(UINT8 eCH);
eHAL_SCALER_EXEC_CODE halScaler_FrameDelay_Set(UINT16 uiSetting);
eHAL_SCALER_EXEC_CODE halScaler_3DSyncDelay_Set(UINT16 uiSetting);
eHAL_SCALER_EXEC_CODE halScaler_SkinColor_Set(UINT8 ucCH,UINT8 ucSetting);
eHAL_SCALER_EXEC_CODE halScaler_TemporalNoiseReduction_Set(UINT8 ucSetting);
eHAL_SCALER_EXEC_CODE halScaler_MPEGNoiseReduction_Set(UINT8 ucSetting);
eHAL_SCALER_EXEC_CODE halScaler_DetectFilm_Set(UINT8 ucSetting);
eHAL_SCALER_EXEC_CODE halScaler_RGB_Gain_Offset_Set(void);
eHAL_SCALER_EXEC_CODE halScaler_PictureSettings_Set(UINT8 ucCH,UINT8 ucSetting);
void halScaler_PreUserMode_Set(UINT8 ucSetting);
eHAL_SCALER_EXEC_CODE halScaler_ContrastEnhancement_Set(UINT8 ucSetting);
INT8 halScaler_HDREnable_Set(UINT8 ucSetting);
INT8 halScaler_MEMC_Set(UINT8 ucSetting);
eHAL_SCALER_EXEC_CODE halScaler_ImageFreeze_Set(UINT8 ucSetting);
eHAL_SCALER_EXEC_CODE halScaler_Gamma_Set(UINT8 ucCH,UINT8 ucSetting);
eHAL_SCALER_EXEC_CODE halScaler_WhitePeaking_Set(UINT8 ucCH,UINT8 ucSetting);
eHAL_SCALER_EXEC_CODE halScaler_ColorTemperature_Set(UINT8 ucCH,UINT8 ucSetting);
INT8 halScaler_EdgeEnhancement_Set(UINT8 ucSetting);
//INT8 halScaler_ColorWheelSpeed_Get(UINT8 ucCH); // A70LV_Eric.C_0026
//eHAL_SCALER_EXEC_CODE halScaler_ColorWheelSpeed_Set(UINT8 ucCH,UINT8 ucSetting);
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
INT8 halScaler_RedOffset_Value_Get(UINT8 ucCH);
INT8 halScaler_GreenOffset_Value_Get(UINT8 ucCH);
INT8 halScaler_BlueOffset_Value_Get(UINT8 ucCH);
UINT8 halScaler_RedGain_Value_Get(UINT8 ucCH);
UINT8 halScaler_GreenGain_Value_Get(UINT8 ucCH);
UINT8 halScaler_BlueGain_Value_Get(UINT8 ucCH);
eHAL_SCALER_EXEC_CODE halScaler_Temporal_NR_Value_Set(UINT8 ucSetting);
eHAL_SCALER_EXEC_CODE halScaler_MPEG_NR_Value_Set(UINT8 ucSetting);
eHAL_SCALER_EXEC_CODE halScaler_DetectFilm_Value_Set(UINT8 ucSetting);

eHAL_SCALER_EXEC_CODE halScaler_PictureSettings_Value_Set(UINT8 ucCH,UINT8 ucSetting);     //A70LV_Doulas_0030

eHAL_SCALER_EXEC_CODE halScaler_MeasureInput(UINT8 ucCH, UINT8 ucInput);      //A70LV_Doulas_0031
eHAL_SCALER_EXEC_CODE halScaler_MeasureCheck(UINT8 ucCH);
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
eHAL_SCALER_EXEC_CODE halScaler_InputSignalFormat_Get(UINT8 ucCH,UINT8 *ucValue);
eHAL_SCALER_EXEC_CODE halScaler_InputAspectRatio_Get(UINT8 ucCH,UINT8 *ucValue);
eHAL_SCALER_EXEC_CODE halScaler_FrontEndColorSpace_Set(UINT8 ucCH,UINT8 ucValue);     //A70LV_Doulas_0076 Add
eHAL_SCALER_EXEC_CODE halScaler_FrontEndColorSpaceInfo_Get(UINT8 ucCH,UINT8* ucValue);    //A70LV_Doulas_0109 Add
eHAL_SCALER_EXEC_CODE halScaler_FrontEndScanMode_Set(UINT8 ucCH,UINT8 ucValue);
//eHAL_SCALER_EXEC_CODE halScaler_FrontEndScanMode_Clear(UINT8 ucCH);      //A70LV_Doulas_0175
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
UINT8 halScaler_HDR_Get(UINT8 ucCH);
eHAL_SCALER_EXEC_CODE halScaler_Position_Run(UINT8 ucCH);       //A70LV_Doulas_0113
UINT8 halScaler_HorzPositionWorkValueGet(UINT8 ucCH);
UINT8 halScaler_VertPositionWorkValueGet(UINT8 ucCH);
eHAL_SCALER_EXEC_CODE halScaler_AutoPsitionStart(UINT8 ucCH,BOOL bAutoEnable);    //A70LV_Doulas_0118
eHAL_SCALER_EXEC_CODE halScaler_Config_NoSignalOutput(UINT8 ucCH, UINT8 ucDisplayOutput);      //A70LV_Doulas_0142  //A70LV_Doulas_0120

UINT16 halScaler_Input_H_Active_Get(UINT8 ucCH);
UINT16 halScaler_Input_V_Active_Get(UINT8 ucCH);
UINT16 halScaler_Input_H_Total_Get(UINT8 ucCH);
UINT16 halScaler_Input_V_Total_Get(UINT8 ucCH);
UINT16 halScaler_Input_H_Start_Get(UINT8 ucCH);
UINT16 halScaler_Input_V_Start_Get(UINT8 ucCH);

eHAL_SCALER_EXEC_CODE halScaler_DDR_InputDataAreaTotalGet(UINT8 eCH,RECT DataArea,UINT32 *ulTotalRed,UINT32 *ulTotalGreen,UINT32 *ulTotalBlue);
eHAL_SCALER_EXEC_CODE halScaler_InputADC_Cali_CalibrationEnableSet(UINT8 eCH,UINT8 ucADC_CAl_Enable);

UINT8 halScaler_Input_3D_Format_Get(void);           //A70LV_Doulas_0154
void halScaler_Input_3D_Format_Set(UINT8 ucVal);     //A70LV_Doulas_0154
eHAL_SCALER_EXEC_CODE halScaler_Input_2D_SyncOut_Enable(UINT8 ucVal);
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
eRESULT halScaler_ProAV_ColorSpaceSetting(UINT8 ucCH, UINT8 ucColorSpace);
eHAL_SCALER_EXEC_CODE halScaler_Monitor(UINT8 ucCH);
eHAL_SCALER_EXEC_CODE halScaler_AVIInfoFrame(UINT8 ucCH);
eHAL_SCALER_EXEC_CODE halScaler_SourceLock(UINT8 ucCH);
eHAL_SCALER_EXEC_CODE halScaler_SPI_SelectSet(UINT8 ucSelect);
eHAL_SCALER_EXEC_CODE halScaler_InputSelectSet(UINT8 ucCH, UINT8 ucSelect, BOOL BlackSource);
eHAL_SCALER_EXEC_CODE halScaler_BackupRxPortSet(UINT8 ucCH);
eHAL_SCALER_EXEC_CODE halScaler_FrameSyncModeSet(UINT8 ucCH, UINT8 eMode);    // ProAV_Rex_0019
eHAL_SCALER_EXEC_CODE halScaler_FrameSyncModeGet(UINT8* cMode);
eHAL_SCALER_EXEC_CODE halScaler_InputFrameRateSetting(UINT8 ucCH);
#if 0
eHAL_SCALER_EXEC_CODE halScaler_FrameRateSetting(UINT8 sRxPort, UINT16 uiIptHTotal, UINT16 uiIptVTotal, UINT16 uiIptVSize);
#endif
eHAL_SCALER_EXEC_CODE halScaler_HDRLevelSet(UINT8 ucLevel, UINT8 ucHDRSelect);
void halScaler_CurrentSourceRxPortSet(UINT8 ucCH, UINT8 ucRxPort);
UINT8 halScaler_CurrentSourceRxPortGet(UINT8 ucCH, UINT8 ucSelect);
eHAL_SCALER_EXEC_CODE halScaler_RxPortCheck(UINT8 ucRxPort, UINT8 ucChangeSourceReset);
eHAL_SCALER_EXEC_CODE halScaler_RxPortCheckPolling(void);
eHAL_SCALER_EXEC_CODE halScaler_RxPortIsReady(UINT8 ucCH, UINT8 ucSelect);
eHAL_SCALER_EXEC_CODE halScaler_RxPortAVI_InfoCopy(UINT8 ucRxPort, UINT8 *pcData);
eHAL_SCALER_EXEC_CODE halScaler_RxPortAVI_Info(UINT8 ucCH, UINT8 ucSelect, UINT8 *pcData);
eHAL_SCALER_EXEC_CODE halScaler_RxPortVSI_Info(UINT8 ucCH, UINT8 ucSelect, UINT8 *pcData);
void halScaler_RxPortDetTiming(UINT8 ucRxPort, UINT8 *psDetTiming);
UINT8 halScaler_InputCompare(UINT8 ucCH, UINT8 ucSelect);
UINT8 halScaler_SclDatapathSet(UINT8 dataPath);
UINT8 halScaler_SclDatapathCompare(UINT8 dataPath);
UINT8 halScaler_SclVopWindowOffSet(UINT8 ucCH, bool bOff);
bool halScaler_SclVopWindowOffGet(UINT8 ucCH);
UINT8 halScaler_3DModeGet(UINT8 ucRxPort);
UINT8 halScaler_DualPipe3dModeSet(bool bMode);

UINT8 halScaler_InputPortIsReady(UINT8 ucCH, UINT8 ucSelect);
void halScaler_InputPortResolution(UINT8 ucCH, UINT8 ucSelect, UINT8 *pcData);
void halScaler_InputPortHorzRate(UINT8 ucCH, UINT8 ucSelect, UINT8 *pcData);
UINT8 halScaler_InputPortVertRateGet(UINT8 ucCH);
void halScaler_RxPortColorSpace(UINT8 ucRxPort, UINT8 *pcData);
void halScaler_InputPortColorSpace(UINT8 ucCH, UINT8 ucSelect, UINT8 *pcData);

UINT8 halScaler_LogoCaptureSet(void);
UINT8 halScaler_LogoDisplayEn(bool bEnable);
UINT8 halScaler_ScreenOff(UINT8 ucCH, bool bEnable);
UINT8 halScaler_Dram2Flash(UINT32 dramAddr, UINT32 flashAddr, UINT32 len);
UINT8 halScaler_Flash2Dram(UINT32 dramAddr, UINT32 flashAddr, UINT32 len);
UINT8 halScaler_BackupInputCheck(UINT8 ucMain, UINT8 ucSub);
eHAL_SCALER_EXEC_CODE halScaler_BackupSizePresetSetting(void);
void halScaler_RxPortInfoPrint(void);
UINT16 halScaler_FpgaTempGet(void);
void halScaler_FpgaTempADCRst(void);
void halScaler_BackupPrimaryInput_Set(UINT8 ucInput);
UINT8 halScaler_BackupPrimaryInput_Get(void);
void halScaler_BackupSecondaryInput_Set(UINT8 ucInput);
UINT8 halScaler_BackupSecondaryInput_Get(void);
UINT8 halScaler_CurrentBackupSourceGet(void);
UINT8 halScaler_LogoCapturePanel(UINT16 uiPanel);
BOOL halScaler_LowLatencyEnable(void);
eHAL_SCALER_EXEC_CODE halScaler_AntiSmearSet(UINT8 cMode, bool bEnable);
eHAL_SCALER_EXEC_CODE halScaler_MEMCBypassSet(UINT8 ucEnable);
eHAL_SCALER_EXEC_CODE halScaler_MEMCColorSet(UINT8 ucColor);
eHAL_SCALER_EXEC_CODE halScaler_MEMCMotionSet(UINT8 ucMotion);
eHAL_SCALER_EXEC_CODE halScaler_MEMCDemoSet(UINT8 ucDemo);
void halScaler_BackupSwitchSet(UINT8 ucValue);
UINT8 halScaler_BackupSwitchGet(void);
BOOL halScaler_AutoExt_3D_SyncSetting(void);
void halScaler_ConfigurePanelSize(uint16 uiPanelID);
BOOL halScaler_Rx_Compare(UINT8 ucCH, UINT8 ucSelect);
//BOOL halScaler_4K3D_Set(UINT8 ucCH, UINT8 ucEnable);
UINT8 halScaler_ColorSetting_Get(eHAL_COLOR_SETTING  ucNode, UINT8 ucCH);	//A70LK_Doulas_0004
#if 0
eHAL_SCALER_EXEC_CODE halScaler_FrameSyncMode_TestSet(UINT8 ucCH, UINT8 eMode, bool bOptType);	 // ProAV_Rex_0032
#endif
//BOOL halScaler_4K3DLLRR_DualPipe_Set(UINT8 ucCH, UINT8 ucEnable);
//BOOL halScaler_4K3DLRLR_DualPipe_Set(UINT8 ucCH, UINT8 ucEnable);
//BOOL halScaler_4K3DLLRR_Set(UINT8 ucCH, UINT8 ucEnable);
//BOOL halScaler_4K3DLRLR_Set(UINT8 ucCH, UINT8 ucEnable);
eHAL_SCALER_EXEC_CODE halScaler_PixelShiftEnable_Set(UINT8 ucEnable);
eHAL_SCALER_EXEC_CODE halScaler_PixelShiftCalPattern_Set(UINT8 ucEnable);
eHAL_SCALER_EXEC_CODE halScaler_Default4K3DSEnable_Set(UINT8 ucMode);
int halScaler_FpgaDataRGBLevelGet(uint16 *RGB_LEVEL_Max,uint16 *RGB_LEVEL_Min);				//A70LK_Doulas_0012
void halScaler_RxPortReset(void);
void halScaler_DisableRGB(eHAL_SCALER_RGB_ITEM RGB_Item); //A70LK_Casper_0008
void halScaler_EnableRGB(void);
void halScaler_RGBEnableItemSet(eHAL_SCALER_RGB_ITEM RGB_Item); //A70LK_Casper_0008
void halScaler_RGBMaskSet(UINT8 ucMask);  //A70LK_Simon_0017
void halScaler_RxPortClear(UINT8 ucRxPort);
void halScaler_RGBSwitchSet(bool bSwitch); //A70LK_Casper_0011
BOOL halScaler_RGBSwitchGet(void); //A70LK_Casper_0011

eHAL_SCALER_EXEC_CODE halScaler_4K3D_DualPipeOrientationSet(UINT8 ucFRC_Bypass,BOOL bRear,BOOL bCeiling); //A70LK_Doulas_0017

eRESULT halScaler_Version_Get(uint32 *ulVersiong);  //A70LK_Steven_0012
void halScaler_PixSftModSet(bool bSwitch);
BOOL halScaler_PixModIsYUV420(UINT8 ucCH, UINT8 ucSelect); //A70LK_Nina_0047
void halScaler_ProAV_ColorSpaceConversion(UINT8 ucCH, UINT8 ucColorSpace, UINT8 ucPixelMode, UINT8 ucRGBMode);
void halScaler_AutoSourceResyncSet(UINT8 ucAutoSourceResync);
UINT8 halScaler_AutoSourceResyncGet(void);

UINT32 halScaler_Reg0_Read_Get(void);
void halScaler_SetHDRDemo(void);


#ifdef __cplusplus
}
#endif



#endif

