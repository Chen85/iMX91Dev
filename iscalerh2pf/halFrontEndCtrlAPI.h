// ==============================================================================
// FILE NAME: HALFRONTENDCTRLAPI.H
// DESCRIPTION:
//
//
// modification history
// --------------------
// 11/27/2017  A70LV_Eric.C_0023
// --------------------
// ==============================================================================


#ifndef _HALFRONTENDCTRLAPI_H_
#define _HALFRONTENDCTRLAPI_H_

#include "Common.h"
#include "dvFrontEndDriver.h"

// A70LV_Eric.C_0030 Start
typedef enum
{
    eVIDEO_COLORDEPTH_8BIT = 0,
    eVIDEO_COLORDEPTH_10BIT,
    eVIDEO_COLORDEPTH_12BIT,

    eVIDEO_COLORDEPTH_UNKNOW
} eVIDEO_COLORDEPTH;

typedef enum
{
    eVIDEO_COLORSPACE_RGB = 0,
    eVIDEO_COLORSPACE_Y422,
    eVIDEO_COLORSPACE_Y444,
    eVIDEO_COLORSPACE_Y420,

    eVIDEO_COLORSPACE_UNKNOW
} eVIDEO_COLORSPACE;

typedef enum
{
    eVIDEO_COLORRANGE_FULL = 0, // VESA (Full Range)
    eVIDEO_COLORRANGE_LIMIT,    // CEA (Limit Range)

    eVIDEO_COLORRANGE_UNKNOW
} eVIDEO_COLORRANGE;

typedef enum
{
    eVIDEO_HDMIMODE_HDMI = 0,
    eVIDEO_HDMIMODE_DVI,

    eVIDEO_HDMIMODE_UNKNOW
} eVIDEO_HDMOMODE;

typedef enum
{
    eVIDEO_SCANNING_PROGRESSIVE = 0,
    eVIDEO_SCANNING_INTERLACED,

    eVIDEO_SCANNING_UNKNOW
} eVIDEO_SCANNING;

typedef enum
{
    EOTF_SDR        = 0, // SDR
    EOTF_HDR        = 1, // High Dynamic Range (HDR)
    EOTF_SMPTE      = 2, // SMPTE ST 2084
    EOTF_HLG        = 3, // Hybrid Log-Gamma (HLG)
    EOTF_RESERVE1   = 4,
    EOTF_RESERVE2   = 5,
    EOTF_RESERVE3   = 6,
    EOTF_RESERVE4   = 7
} EOTF_STREAM;

#pragma pack(push)  /* push current alignment to stack */
#pragma pack(1)     /* set alignment to 1 byte boundary */
typedef struct
{
    UINT16 MaxCLL;
    UINT16 MaxFALL;
    UINT16 MaxMasteringNits;
    FLOAT  MinMasteringNits;
}sHDR_LUMINANCE_INFO;
#pragma pack(pop)   /* restore original alignment from stack */


typedef struct {
	UINT32	u32VideoPCLK;       // Pixel Clock
	UINT16	u16VideoHTotal;     // H Total
	UINT16	u16VideoHActive;    // H Active Video
	UINT16	u16VideoVTotal;     // V Total
	UINT16	u16VideoVActive;    // V Active Video
	UINT16  u16VideoVRate;      // V Rate (Hz)
}sVIDEO_TIMING;     //A70LV_Doulas_0176 modify

typedef struct {
//	UINT8  u8VideoReady;
	UINT8  u8VideoColorDepth;       // eVIDEO_COLORDEPTH
	UINT8  u8VideoColorSpace;       // eVIDEO_COLORSPACE
	UINT8  u8VideoDynamicRange;     // eVIDEO_COLORRANGE
	UINT8  u8VideoScanning;         // eVIDEO_SCANNING
	UINT8  u8VideoAVIInfoDet;       // 1: have AVI InfoFrame
	UINT8  u8Video3DFormat;
    UINT8  u8VideoVGASyncType;      // 0:SOG , 1:PC sync    //A70LV_Doulas_0102
    UINT8  u8VideoColorYUV;         // 0:REC601 , 1:REC709 (HDMI/DVI/HDBaseT/3GSDI?) //A70LV_Doulas_0102
    UINT8  u8VideoDownScaling;       //A70LV_Doulas_0154
    UINT8  u8VideoDualPixelMode;     //A70LV_Doulas_0154
    UINT8  u8VideoHDRType;           // 0 = SDR, 1 = HDR //ZU860_Doulas_0100
    UINT8  u8VideoExtentedColorimetry; //ZU860_Doulas_0100
}sVIDEO_FORMAT;

typedef struct {
	UINT8  u8VideoReady;
	sVIDEO_TIMING   sVideoTiming;
	sVIDEO_FORMAT   sVideoFormat;
}sVIDEO_INFO;
// A70LV_Eric.C_0030 End

//A35G2_Simon_0091
typedef union
{
    UINT8 ucData[257];

    struct
    {
        UINT8 ucSourceID;
        UINT8 ucEDID[256];
    }sEDID_DATA;
}uCUSTOMIZE_EDID_DATA;

typedef enum
{
    //PC
    eTIMEING_XGA_60HZ,
    eTIMEING_WXGA_60HZ,
    eTIMEING_1600x1200_60HZ,
    eTIMEING_WUXGA_60HZ,
    eTIMEING_WQXGA_60HZ,//2560x1600

    //VIDEO
    eTIMEING_720P_50HZ,
    eTIMEING_720P_60HZ,//Done
    eTIMEING_1080P_50HZ,
    eTIMEING_1080P_60HZ,//Done
    eTIMEING_3840x2160_50HZ,//4K2K, 3840x1260
    eTIMEING_4096x2160_50HZ,//4K2K, 3840x1260
    eTIMEING_3840x2160_60HZ,//4K2K, 3840x1260, Done
    eTIMEING_4096x2160_60HZ,
    eTIMEING_960x2160_50HZ,//Quarter 4K , 960x2160
    eTIMEING_960x2160_60HZ,//Quarter 4K , 960x2160, Done
    eTIMEING_1280x2160_50HZ,//One of third for C790
    eTIMEING_1280x2160_60HZ,//One of third for C790

    //3D
    eTIMEING_XGA_120HZ,
    eTIMEING_WXGA_120HZ,
    eTIMEING_720P_120HZ,
    eTIMEING_1080P_120HZ,
    eTIMEING_2688x1472_120HZ,//For MST MEMC in front of Scaler
    eTIMEING_2716x1528_120HZ,//For TI 3D Limitation
    eTIMEING_SVGA_120HZ,       //A70LV_Doulas_0094 800x600
    eTIMEING_WUXGA_120HZ,      //A70LV_Doulas_0094 1920x1200
    eTIMEING_LAST,
} eTIMEING;

typedef enum
{
	eITE6634_HotPlugDelay_800,
	eITE6634_HotPlugDelay_1600,
	eITE6634_HotPlugDelay_1200,
	eITE6634_HotPlugDelay_400

} eITE6634_HOT_PLUG_DELAY;	//G100_Doulas_0007


#pragma pack(push)  /* push current alignment to stack */
#pragma pack(1)     /* set alignment to 1 byte boundary */

typedef struct {
	UINT16  wHTotal;
	UINT16  wHActive;
    UINT16  wHStart;
    UINT16  wVTotal;
    UINT16  wVActive;
    UINT16  wVStart;
    UINT8  cInterlace;
    UINT16  wPixelCLK;
}sPROAV_VIDEO_INFO;

#pragma pack(pop)   /* restore original alignment from stack */

#define PROAV_VIDEO_SIZE sizeof(sPROAV_VIDEO_INFO)/sizeof(BYTE)


#if 1 //HICC2_Doulas_0010//Depends on X35Gen2 design
eRESULT halFrontEndCtrl_Version_Get(UINT8* pcData);
#endif
eRESULT halFrontEndCtrl_HDBaseTVersion_Get(UINT8* pcData);      //A70LV_Doulas_0076
eRESULT halFrontEndCtrl_videoReady_Get(UINT8* pcData);  // A70LV_Eric.C_0030
eRESULT halFrontEndCtrl_videoFormat_Get(UINT8* pcData); // A70LV_Eric.C_0030
eRESULT halFrontEndCtrl_MainInput_Get(UINT8* pcData);   //A70LV_Doulas_0084
eRESULT halFrontEndCtrl_MainInput_Set(UINT8* pcData);   //A70LV_Doulas_0084

eRESULT halFrontEndCtrl_SubInput_Get(UINT8* pcData);        //A70LV_Doulas_0102
eRESULT halFrontEndCtrl_SubInput_Set(UINT8* pcData);
eRESULT halFrontEndCtrl_PIP_Enable_Get(UINT8* pcData);
eRESULT halFrontEndCtrl_PIP_Enable_Set(UINT8* pcData);
eRESULT halFrontEndCtrl_INPUT_SOURCE_DETECT_Get(UINT16* uiData);
eRESULT halFrontEndCtrl_VGA_H_Total_Get(UINT16* uiData);
eRESULT halFrontEndCtrl_VGA_H_Total_Set(UINT16* uiData);
eRESULT halFrontEndCtrl_VGA_Phase_Get(UINT8* pcData);
eRESULT halFrontEndCtrl_VGA_Phase_Set(UINT8* pcData);
eRESULT halFrontEndCtrl_videoFormatSub_Get(UINT8* pcData);      //A70LV_Doulas_0112
eRESULT halFrontEndCtrl_VGA_Sync_Threshold_Get(UINT8* pcData);
eRESULT halFrontEndCtrl_VGA_Sync_Threshold_Set(UINT8* pcData);
eRESULT halFrontEndCtrl_VGA_Pixel_Clock_Set(UINT32 udData);     //A70LV_Doulas_0115

eRESULT halFrontEndCtrl_VGA_Gain_Get(UINT16* uiRed,UINT16* uiGreen,UINT16* uiBlue);      //A70LV_Doulas_0124
eRESULT halFrontEndCtrl_VGA_Gain_Set(UINT16 uiRed,UINT16 uiGreen,UINT16 uiBlue);
eRESULT halFrontEndCtrl_VGA_Offset_Get(UINT16* uiRed,UINT16* uiGreen,UINT16* uiBlue);
eRESULT halFrontEndCtrl_VGA_Offset_Set(UINT16 uiRed,UINT16 uiGreen,UINT16 uiBlue);
eRESULT halFrontEndCtrl_FPGA_Version_Get(UINT8* pcData);        //A70LV_Doulas_0176
eRESULT halFrontEndCtrl_Main_Timing_Get(UINT8* pcData);         //A70LV_Doulas_0176
eRESULT halFrontEndCtrl_Sub_Timing_Get(UINT8* pcData);          //A70LV_Doulas_0176
eRESULT halFrontEndCtrl_EDID_SN_Set(UINT8* pcData);
//eRESULT halFrontEndCtrl_SerialNumber_Set(UINT8* pcData);                                    //A70LV_Doulas_0200
eRESULT halFrontEndCtrl_VGA_TIMING_Set(UINT16 uiH_Active,UINT16 uiV_Active,UINT16 uiV_Freq);//A70LV_Doulas_0200
//eRESULT halFrontEndCtrl_EDID_Upgrade_Set(UINT8* pcData); //A70LV_Larry_0290
//eRESULT halFrontEndCtrl_videoReady_Get(UINT8* pcData);
//eRESULT halFrontEndCtrl_EDID_Upgrade_Single_Set(UINT8* pcData); //G100_Steven_0005
eRESULT halFrontEndCtrl_OPD_Register_Set(UINT8 reg, UINT8 size, UINT8* pcData);
eRESULT halFrontEndCtrl_OPD_Register_Get(UINT8 reg, UINT8 size, UINT8* pcData);
eRESULT halFrontEndCtrl_IST_Set(UINT8 ucCount); //G100_Julie_0001
eRESULT halFrontEndCtrl_EQMode_HDMI1_Set(UINT8 ucValue);
eRESULT halFrontEndCtrl_EQMode_HDMI2_Set(UINT8 ucValue);
eRESULT halFrontEndCtrl_EQMode_DVI_Set(UINT8 ucValue);
eRESULT halFrontEndCtrl_HotPlugEnable_Set(UINT8 ucEnable);		//G100_Doulas_0006
eRESULT halFrontEndCtrl_ForceHotPlug_Set(UINT8 ucForce); 			//G100_Doulas_0007
eRESULT halFrontEndCtrl_Model_Name_Set(UINT8* pcData); //A35G2_CDS_Coda_0032

//G100_Julie_0002, start.
eRESULT halFrontEndCtrl_IST_Get(UINT8 *ucCount);
eRESULT halFrontEndCtrl_OPD_State_Get(UINT8 *ucCount);
eRESULT halFrontEndCtrl_OPD_Work_Get(UINT8 *ucCount);
eRESULT halFrontEndCtrl_OPD_Device_Get(UINT8 ucValue, UINT8* pcData);
eRESULT halFrontEndCtrl_Error_Reset(void);
//G100_Julie_0002, end.
eRESULT halFrontEndCtrl_SPI_GS2961_Set(void); //G100_Julie_0004
eRESULT halFrontEndCtrl_SPI_GS2961_Get(UINT8 *ucCount); //G100_Julie_0004

eRESULT halFrontEndCtrl_Backup_AutoSW_Set(UINT8* pcData); //G100_Steven_0016
eRESULT halFrontEndCtrl_Backup_First_Set(UINT8* pcData);  //G100_Steven_0016
eRESULT halFrontEndCtrl_Backup_Second_Set(UINT8* pcData); //G100_Steven_0016
eRESULT halFrontEndCtrl_HDMI_OUT_Set(UINT8* pcData);
eRESULT halFrontEndCtrl_I2C_Retry_Get(UINT16 uiSize, UINT8* pcData); //G100_Julie_0017
eRESULT halFrontEndCtrl_I2C_Error_Get(UINT16 uiSize, UINT8* pcData); //G100_Julie_0017
eRESULT halFrontEndCtrl_I2C_Total_Get(UINT8* pcData); //G100_Julie_0017
eRESULT halFrontEndCtrl_Backup_Status_Get(UINT8* pcData);
eRESULT halFrontEndCtrl_OPDInfo_Get(UINT8* pcData); //G100_Steven_0059
eRESULT halFrontEndCtrl_BackupInput_videoReady_Get(UINT8* pcData); //G100_Steven_0065
eRESULT halFrontEndCtrl_BackupInput_ScalerTimingCheck_Set(UINT8* pcData); //A35G2_CDS_Simon_0041
eRESULT halFrontEndCtrl_AutoHDMISwitch_Set(UINT8* pcData);  //A35G2_BRC_Simon_0006
eRESULT halFrontEndCtrl_AVIInfoFrame_Input0RX0_Get(UINT8* pcData);
eRESULT halFrontEndCtrl_AVIInfoFrame_Input0RX1_Get(UINT8* pcData);
eRESULT halFrontEndCtrl_AVIInfoFrame_Input0RX2_Get(UINT8* pcData);
eRESULT halFrontEndCtrl_AVIInfoFrame_Input0RX3_Get(UINT8* pcData);
eRESULT halFrontEndCtrl_AVIInfoFrame_Input1RX0_Get(UINT8* pcData);
eRESULT halFrontEndCtrl_AVIInfoFrame_Input1RX1_Get(UINT8* pcData);
eRESULT halFrontEndCtrl_AVIInfoFrame_Input1RX2_Get(UINT8* pcData);
eRESULT halFrontEndCtrl_AVIInfoFrame_Input1RX3_Get(UINT8* pcData);
eRESULT halFrontEndCtrl_Model_Switch_Enable_Set(UINT8 ucSwitch);   //A65_OPTOMA_Julie_0065
eRESULT halFrontEndCtrl_CustomizedEDID_Set(UINT8* pcData); //A35G2_Simon_0091
eRESULT halFrontEndCtrl_AudioVol_Set(UINT8 vol);      //R70G2_Sammy_0004
eRESULT halFrontEndCtrl_AudioMute_Set(UINT8 mute);    //R70G2_Sammy_0004
eRESULT halFrontEndCtrl_AudioPlay_Set(UINT8* pcPlay);		//R70G2CDS_Sammy_0010

eRESULT halFrontEndCtrl_BIST_Set(UINT8 ucID_Index, UINT16 ucTestCount, UINT8 ucModule, UINT8 ucLayer); //HICC2_Steven_0001
eRESULT halFrontEndCtrl_BIST_Get(UINT8 ucID_Index, UINT8* pcData);
eRESULT halFrontEndCtrl_BIST_Status_Set(UINT8 cCmd); //HICC2_Steven_0007
eRESULT halFrontEndCtrl_Factory_Reset_Set(UINT8 cReset);

eRESULT halFrontEndCtrl_HDR_EOTF_STREAM_Get(UINT8* pcData);
eRESULT halFrontEndCtrl_HDR_LuminanceInfo_Get(UINT8* pcData);

void halFrontEnd_Bist_Cmd_Send(void *pvData);  //H30K_Steven_0001
void halFrontEnd_Bist(void *pvData);

#endif /*_HALFRONTENDCTRLAPI_H_*/
