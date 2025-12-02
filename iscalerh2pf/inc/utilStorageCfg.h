#ifndef _UTILSTORAGECFG_H_
#define _UTILSTORAGECFG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "Common.h"
#include "utilCommon.h"
#include "cmd_ap.h"
#ifdef SCALER_FPGA_F34
#include "utilWarpDemoProAV.h"
#else
#include "utilWarpDemo.h"
#endif

#define SUPPORT_MANUAL_ADJUSTMENT   0   //A70LV_Doulas_0023

#define INPUT_SOURCE_MAX            eCM_SOURCE_NUMBER //15  //A70LV_Doulas_0013
#define DISPLAY_MODE_MAX            eCM_PICTURE_SETTINGS_NUMBER //9//11  //G100_Owen_0006
#define SIGNAL_TYPE_MAX             3   //A70LV_Doulas_0013
#define HSG_CT_MAX                  16  //HSG white gain color temperature

#define MODEL_NAME_MAX_LENGTH       15  //A70LV_Doulas_0013
#define SOURCE_NAME_MAX_LENGTH      19  //A70LV_Doulas_0013
#define AC_VOLTAGE_MAX_LEN              8                   //G100_Tim_0009, add

#define WAVEFORMSTATE_RESERVED_NUM          7   //A70LV_Doulas_0015
#define BURNIN_RESERVED_NUM                 7   //A70LV_Doulas_0015
#define SYSTEM_DEFAULT_RESERVED_NUM         7  //A70LV_John_0072 add color offset default value //A70LV_Doulas_0015
#define SOUCE_DEPEND_RESERVED_NUM           7  //A70LV_Doulas_0015
#define COLOR_SETTING_RESERVED_NUM          7   //G100_Steven_0047 //A70LV_Doulas_0015

#define USER_LIGHT_SETTING_RESERVED_NUM     7
#define USER_OSD_SETTING_RESERVED_NUM       7
#define USER_NETWORK_SETTING_RESERVED_NUM   7
#define USER_IMAGE_SETTING_RESERVED_NUM     7//53      //T100_Doulas_0007
#define USER_WARP_SETTING_RESERVED_NUM      7//82//102     //T100_Doulas_0006 modify
#define USER_COMMON_SETTING_RESERVED_NUM    7//698//700//701     //A70LV_Doulas_0279//T100_Doulas_0009 //T100_Doulas_0008 //T100_Doulas_0002 modify
#define USER_SOURCE_SETTING_RESERVED_NUM    7

#define BACK_RESTORE_ITEM_MAX               200         //A70LV_Doulas_0279
#define BACK_RESTORE_CHECKING_NUM           0x12345678  //A70LV_Doulas_0279
#define BACK_RESTORE_RETENTION              514         //A70LV_Doulas_0280

#define PROSERVICE_PAIR_CODE_LEN        8         //A65_OPTOMA_Jerry_0004 start
#define PROSERVICE_PAIR_ACCOUNT_LEN     23
#define PROSERVICE_WEBSITE_URL_LEN      127
#define PROSERVICE_WEBSITE_WW           "https://oms.optoma.com"
#define PROSERVICE_WEBSITE_CHINA        "https://oms.optoma.com.cn"
#define PROSERVICE_LOCATION_WW          "WorldWide"
#define PROSERVICE_LOCATION_CHINA       "China"
#define BROADCAST_MSG_LEN               50//30       //A65_OPTOMA_Jerry_0004 //A35G2_Coda_0061



typedef enum
{
    edrMAX,
    edrMIN,
    edrDEFAULT,

    edrINVALID,
} eDATA_RANGE_MODE;

#pragma pack(push)  /* push current alignment to stack */
#pragma pack(1)     /* set alignment to 1 byte boundary */


typedef struct Burnin_Information
{
    UINT8  ucBurnInEnable;
    UINT8  ucLampOn;
    UINT8  ucLampOff;
    UINT16 uiBurnInCycle;
    UINT8  ucBurnInMode;  //0: Full, 1:Eco
    UINT8  ucBurnInAlwaysOn;

    UINT8 ucEEPROM_Retention[BURNIN_RESERVED_NUM];      //A70LV_Doulas_0013
    UINT8 ucBurnIn_CRC;               //A70LV_Doulas_0015
} sBURNIN_INFORMATION;

typedef enum
{
    eTIMER_RECORD_BLD = 0,
    eTIMER_RECORD_RLD = 1,
    eTIMER_RECORD_NUMBER = 2,
} eTIMER_RECORD;    //A70LV_Doulas_0013

typedef enum
{
    e3DFS_1080P_120 = 0,//e3DFS_XGA_120,
    e3DFS_Other_120 = 1,
    e3DFS_1080P_60 = 2,
    e3DFS_1080P_50 = 3,
    e3DFS_WUXGA_60 = 4,
    e3DTB = 5,
    e3DFP = 6,
    e3DSbS = 7,
    e3D_Reserve1 = 8,
    e3D_Reserve2 = 9,

    //Active
    e3DFS_MANDATORY_3D = 0, //1.4a mandatory 3D Frame pack, side-by-side, top-bottom
    e3DFS_4K3D = 1,         //A70LK 4K 3D
    e3DFS_FS_120 = 2,       //frame sequential 120hz
    e3DFS_FS_OTHER = 3,
    e3DFS_OTHER = 4,

    //Passive
    e3DFS_MANDATORY_3D_P = 5, //1.4a mandatory 3D Frame pack, side-by-side, top-bottom
    e3DFS_4K3D_P = 6,         //A70LK 4K 3D
    e3DFS_FS_120_P = 7,       //frame sequential 120hz
    e3DFS_FS_OTHER_P = 8,
    e3DFS_OTHER_P = 9,

    e3D_Customer_Timing = 10,
    e3DFRAME_DELAY_INVALID,
}e3DFRAME_DELAY_TIMING;     //A70LV_Doulas_0196 modify//A70LV_Doulas_0013

typedef struct
{
    UINT16  uiRGBGain[3];
    UINT16  uiRGBOffset[3];
    UINT16  uiYUVGain[3];
    UINT16  uiYUVOffset[3];
    UINT16  uiRGBGain2[3];
    UINT16  uiRGBOffset2[3];
    UINT16  uiYUVGain2[3];
    UINT16  uiYUVOffset2[3];
} sADC_CAL_VALUES,*PsADC_CAL_VALUES;       //VGA cal

typedef enum
{
    eLD_SEQ_Y,
    eLD_SEQ_R,
    eLD_SEQ_B,
    eLD_SEQ_G,

    eLD_SEQ_NUMBER,
}eLD_SEQ; //T100_Larry_0022

typedef struct
{
    UINT16  uiVersion;
    UINT8   ucUpdateStart;
    UINT8   ucUpdateComplete;
    UINT8   ucUpdateStatus;

    UINT8   ucEEPROM_Retention[WAVEFORMSTATE_RESERVED_NUM];
    UINT8   ucWAVEFORMSTATE_CRC;    //A70LV_Doulas_0015
} sWAVEFORMSTATE,*PsWAVEFORMSTATE; //Lamp

typedef struct
{
    UINT8  	ucPowerMode;        //light source mode
    UINT8   ucWaveformGain;     //light strength
    UINT8   ucLampIntensity;
    UINT8   ucABPmode;
    UINT8   ucInstantPowerOff;
    UINT8   ucLightCalibrationMode;

    UINT16  uiIntensityPWM[8]; //BLD Y R B G, RLD Y R B G
    UINT16  uiLightSensorIntensity_BLD[eLD_SEQ_NUMBER]; //Y R B G
    UINT16  uiLightSensorIntensity_RLD[eLD_SEQ_NUMBER]; //Y R B G

    UINT8   ucIRWaveformGain;     //light strength
    UINT8   ucIRMode_WaveformGain;     //light strength
    UINT8   ucIRMode_IRWaveformGain;     //light strength

    UINT8   ucIRLD; //T100IR_Casper_0006
    UINT8   ucIRBLD; //T100IR_Casper_0006
    UINT8  	ucConstantBrightness;		//G100_Doulas_0008

    UINT8   ucEEPROM_Retention[USER_LIGHT_SETTING_RESERVED_NUM];
    UINT8   ucLIGHT_SETTING_CRC;
} sLIGHT_SETTING, *PsLIGHT_SETTING;       //A70LV_Doulas_0015

typedef struct
{
    UINT8	ucMenuLocation;
    UINT8   ucOSDTranslucency;
    UINT8   ucOSDTimeout;
    UINT8   ucShowMessages;
    UINT8   ucMenuOffsetX;
    UINT8   ucMenuOffsetY;
    UINT8   ucMenuLockout;
    UINT8   ucMenuTransparency;
    UINT8   ucSearchScreen;     //Splash Screen Setup

    UINT16  uiOPDPeriod;  //HICC2_Steven_0009
    UINT8   ucEEPROM_Retention[USER_OSD_SETTING_RESERVED_NUM];
    UINT8   ucOSD_SETTING_CRC;
} sOSD_SETTING, *PsOSD_SETTING;       //A70LV_Doulas_0015

typedef struct
{
    UINT8   ucShowNetworkMessages;
    UINT8   ucDHCP;

    UINT8   ucEEPROM_Retention[USER_NETWORK_SETTING_RESERVED_NUM];
    UINT8   ucNETWORK_SETTING_CRC;
} sNETWORK_SETTING, *PsNETWORK_SETTING;       //A70LV_Doulas_0015

typedef struct
{
    UINT8   ucHSGAdjustmentEnable;
    UINT8   ucHSGAdjustmentAutoTestPattern;

    UINT8   ucBlankonSignalSwitch ;
    UINT8   ucSyncThreshold;
    UINT8   ucContrastEnhancement;  //0: off ,1:Dynamic Black,2:real black
    UINT8   ucFilm;
    UINT8   ucWallColor;
    UINT8   ucEdgeEnhancement;
    UINT8   ucImageFreeze;

    UINT8   ucCeilingMount;
    UINT8   ucRearProject;

    UINT8   ucTemporalNoiseReduction;   //A70LV_Doulas_0022
    UINT8   ucMPEGNoiseReduction;       //A70LV_Doulas_0022
    UINT8   ucHDR;                      //A70LV_Doulas_0022
    UINT8   ucMEMCBypass;               //A70LV_Doulas_0022
    UINT16  ui3DFRAME_DELAY_TIMING[e3DFRAME_DELAY_INVALID];     //A70LV_Doulas_0022
    UINT16  ui3DSYNC_DELAY_TIMING[e3DFRAME_DELAY_INVALID];     //A70LV_Doulas_0022

    UINT8   ucLightsOutTimer;        //A70LV_Larry_0235
    UINT8   ucLightsOutSignalLevel;  //A70LV_Larry_0235
    UINT8   ucLightsOnThreshold;	 //A70Gen2_Doulas_0044

    UINT8   ucCompatible_4K; //A70LV_Larry_0283

    UINT8   ucHDRLevel;
    UINT8   ucHDREnable;
    UINT8   ucMEMC_Color;
    UINT8   ucMEMC_Motion;
    UINT8   ucMEMC_Motion_Demo;
    UINT8   ucCW_Speed;
    UINT8   ucDBSpeed;
    UINT8   ucDBStrength;
    UINT8   ucDBLightLevel;
    UINT8   ucDBRealBlackEnable;
    UINT8   ucDBSpeedControl;

    UINT8   ucHDMI_EDID_1;
    UINT8   ucHDMI_EDID_2;
    UINT8   ucHDBaseT_EDID;  //G100_Steven_0005
    UINT8   ucHDMI_OUT;			//G100_Doulas_0057
    UINT8   ucHSGAdjustmentAutoTestPattern_2;	//G100_Coda_00109
    UINT8   ucHSGSelected;
	UINT8   ucHSGTestPatternCtrl; //A35G2_Coda_0103
    UINT8   ucCustomizeEDID_HDMI1;  //A35G2_Simon_0091
    UINT8   ucCustomizeEDID_HDMI2;  //A35G2_Simon_0091
    UINT8   ucEEPROM_Retention[USER_IMAGE_SETTING_RESERVED_NUM];
    UINT8   ucIMAGE_SETTING_CRC;
    UINT8   ucHDR_Gamma;    //H2PF_Simon_0193
    UINT8   ucHDR_Dynamic_Contrast;
    UINT8   ucHDR_Mode;
    UINT8   ucHDR_Manual_Level;
    UINT16  uiHDR_Custom_Nit;
    UINT16  uiHDR_Nit_Display;
    UINT16  uiHDR_Nit_Final;
    UINT16  uiHDR_Nit_MaxLimit;
} sIMAGE_SETTING, *PsIMAGE_SETTING;       //A70LV_Doulas_0015

#define WARPING_MEMORY_RESERVED  (14)

typedef struct
{
    UINT8   ucWarpingApply;
    UINT8   ucBlendApply;
	UINT8	ucWarpingName[24];
	UINT8	ucBlendingName[24];
    //UINT8   ucBlendingTopEnable;
    //UINT16  uiBlendingTopStartPixel;
    //UINT16  uiBlendingTopPixelWidth;
    //UINT8   ucBlendingBottomEnable;
    //UINT16  uiBlendingBottomStartPixel;
    //UINT16  uiBlendingBottomPixelWidth;
    //UINT8   ucBlendingLeftEnable;
    //UINT16  uiBlendingLeftStartPixel;
    //UINT16  uiBlendingLeftPixelWidth;
    //UINT8   ucBlendingRightEnable;
    //UINT16  uiBlendingRightStartPixel;
    //UINT16  uiBlendingRightPixelWidth;
    //UINT8   ucHLUT[24];
    //UINT8   ucVLUT[24];

    UINT8   ucRetention[WARPING_MEMORY_RESERVED];
}sWARP_MEMORY; //128 byte //A70LV_Larry_0139

typedef struct
{
    UINT8   ucWarpMode;
    UINT16  uiBlendingAreaOffset;
    UINT16  uiNonBlendingAreaOffset;
    UINT8   ucBlending;
    UINT8   ucMarker;
    UINT8   ucGrid;
    UINT8   ucSolidColor;
    UINT8   ucImageGamma;
    UINT16  uiBlendingGamma;
	UINT8   ucWarpHorzKeystone;
	UINT8   ucWarpVertKeystone;
	UINT8   ucWarpHorzPincushion;
	UINT8   ucWarpVertPincushion;
	UINT8   ucWarpPincushionBarrel;
	UINT8   ucGeometryPCMode;
    UINT16  ui4CornerTopLeftHorz;
    UINT16  ui4CornerTopLeftVert;
    UINT16  ui4CornerTopRightHorz;
    UINT16  ui4CornerTopRightVert;
    UINT16  ui4CornerBottomLeftHorz;
    UINT16  ui4CornerBottomLeftVert;
    UINT16  ui4CornerBottomRightHorz;
    UINT16  ui4CornerBottomRightVert;
    UINT8   ucWarpAutoFilter;
    UINT8   ucWarpFilterH;
    UINT8   ucWarpFilterV;

    UINT8   ucMovePitchMode;

    UINT8   ucBlendingTopEnable;        //A70LV_Doulas_0021
    UINT16  uiBlendingTopStartPixel;
    UINT16  uiBlendingTopPixelWidth;
    UINT8   ucBlendingBottomEnable;
    UINT16  uiBlendingBottomStartPixel;
    UINT16  uiBlendingBottomPixelWidth;
    UINT8   ucBlendingLeftEnable;
    UINT16  uiBlendingLeftStartPixel;
    UINT16  uiBlendingLeftPixelWidth;
    UINT8   ucBlendingRightEnable;
    UINT16  uiBlendingRightStartPixel;
    UINT16  uiBlendingRightPixelWidth;
    UINT8   ucAutoAdjustmentMode;

    UINT8   ucWarpingApplySetting;
    UINT8   ucWarpingSaveSetting;
    UINT8   ucWarpingLastSaved;  //A35G2_CDS_Simon_0008
    UINT8   ucBlendingGamma; //A70LV_Larry_0169
    UINT8   ucGeometryEnable;

    sWARP_MEMORY sWarpMemory[BLENDING_AP_MEM_INVALID]; //A70LV_Larry_0139

	UINT8	ucBlendApplySetting;
	UINT8	ucBlendSaveSetting;
    UINT8   ucBlendLastSaved;  //A35G2_CDS_Simon_0008

	UINT8   ucAdvWarpControl;				//G100_Doulas_0027 start
	UINT8   ucAdvWarpGridPoints;
	UINT8   ucAdvWarpInner;
	UINT8   ucAdvWarpSharpness;
	UINT8   ucAdvWarpGridColor;
	UINT8   ucAdvWarpBackgroundColor;
	UINT8   ucAdvBlendinWidth;
	UINT16  ucAdvBlendingLeft;
	UINT16  ucAdvBlendingRight;
	UINT16  ucAdvBlendingTop;
	UINT16  ucAdvBlendingBottom;
	UINT8   ucAdvBlendingOverlapGridNumber;
	UINT8   ucAdvBlendingGamma;				//G100_Doulas_0027 end
	UINT8   ucAdvWarpInner_LastValue;   //G100_Tim_0055, add //A35G2_BRC_Casper_0051

    UINT8   ucAdvBlackLevelArea;							//A65_OPTOMA_Doulas_0020
    UINT8   ucAdvBlackLevelEnable[BLACKLEVEL_AREA__NUM];	//A65_OPTOMA_Doulas_0020
    UINT8   ucAdvBlackLevelBrightness;						//A65_OPTOMA_Doulas_0020
    UINT8   ucAdvBlackLevelRed[BLACKLEVEL_AREA__NUM];		//A65_OPTOMA_Doulas_0020
    UINT8   ucAdvBlackLevelGreen[BLACKLEVEL_AREA__NUM];		//A65_OPTOMA_Doulas_0020
    UINT8   ucAdvBlackLevelBlue[BLACKLEVEL_AREA__NUM];		//A65_OPTOMA_Doulas_0020
    UINT8   ucAdvBlackLevelBoundary;  						//A35G2_Owen_0002 //A65_OPTOMA_Doulas_0239

    UINT8   ucAwSelectWarpPoint_Col;
    UINT8   ucAwSelectWarpPoint_Row;
    UINT8   ucAwWarpPattern_Show;
    UINT16  uiAwWarpPointMove_Up;
    UINT16  uiAwWarpPointMove_Down;
    UINT16  uiAwWarpPointMove_Left;
    UINT16  uiAwWarpPointMove_Right;
    sDST_AW_WARP_POINT_POSITION sAwWarpPointPosition;
    UINT8   ucAwWarpTableReset;
    UINT8   ucAwShowBlendEffectOnPattern;
    UINT8   ucAwShowOverlapGridOnPattern;

    UINT8   ucAwBlendPattern_Show;
    UINT16  uiAwBlendOverlapSize_Up;
    UINT16  uiAwBlendOverlapSize_Down;
    UINT16  uiAwBlendOverlapSize_Left;
    UINT16  uiAwBlendOverlapSize_Right;
    UINT16  uiAwBlendOverlapOffset_Up;
    UINT16  uiAwBlendOverlapOffset_Down;
    UINT16  uiAwBlendOverlapOffset_Left;
    UINT16  uiAwBlendOverlapOffset_Right;
    UINT8   ucAwBlendReset;
    UINT8   ucAwBlendEnable;

    UINT8   ucEEPROM_Retention[USER_WARP_SETTING_RESERVED_NUM];
    UINT16  ucWARP_SETTING_CRC;
} sWARP_SETTING, *PsWARP_SETTING;       //A70LV_Doulas_0015

typedef struct
{
    UINT8   ucModelName[MODEL_NAME_MAX_LENGTH + 1];
    UINT8   ucCustomCode[MODEL_NAME_MAX_LENGTH + 1];
    UINT8   ucFOTAName[MODEL_NAME_MAX_LENGTH + 1];

    UINT8	ucLanguage;

    UINT8   ucACPowerOn;
    UINT8   ucStandbyPowerSave;

    UINT8   ucProjectorAddress;
    UINT8   ucKeypadBacklight;
    UINT8   ucStatusLED;

    UINT8   ucAutoOffTime;
    UINT8   ucScreenSaveTime;
    UINT8   ucSleepTimer;

    UINT8   ucChangePIN;
    UINT8   ucSerialPortBaudRate;
    UINT8   ucSerialPortEcho;
    UINT8   ucSerialPortPath;        //HDBaseT or normal RS232

    UINT8   ucIRControl_Top ;
    UINT8   ucIRControl_Front;
    UINT8   ucIRControl_Rear;// HICC2_Bruce_0002
    UINT8   ucIRControl_HDBaseT;
    UINT8   ucHDBaseTEnable;

    UINT8   ucSysErrCode;
    UINT8   uc12VTrigge ;

    UINT8   ucLensAdjust;   //Lens lock
    UINT8   ucLensShift;
    UINT8   ucLensMemoryApply;
    UINT8   ucLensMemorySave;
    UINT8   ucLensSpecialLocation;

    UINT8	ucHighAltitude;
    UINT8   ucKeypadLock;// HICC2_Bruce_0001
    UINT8   ucUSB_Power;// HICC2_Bruce_0004
    UINT16  uiPowerOnCounter;// HICC2_Bruce_0007
    UINT16  uiPowerOffCounter;// HICC2_Bruce_0007
    UINT8  	ucFactoryTestPattern;  //factory mode  //A70LV_Doulas_0024
    UINT8   ucServiceMenuTestPattern;
    UINT8   ucFactoryMenuCustomPattern; //A70LV_Doulas_0024
    UINT8   ucSplashStartupEnable;      //LOGO NUMBER

    UINT8  	ucActuatorSwitch;
    UINT8   ucCoolingDown;          //A70LV_Doulas_0023

    UINT8  	ucOSDTestPattern;       //A70LV_Doulas_0024

    UINT8   ucUartSwitch;

    UINT8   ucBackupRestoreSave;        //A70LV_Doulas_0279
    UINT8   ucBackupRestoreRestore;     //A70LV_Doulas_0279
    UINT8   ucLensDetection;

    UINT8   ucLowLatency;
    UINT8   ucBackupIputSwitch;
    UINT8   ucBackupPrimaryInput;
    UINT8   ucBackupSecondaryInput;

    // for OPTOMA   //ZU860_Doulas_0006
    UINT8   ucProjectorID;
    UINT8   ucBackgroundColor;
    UINT8   ucSignalPowerOn;
    UINT32  ulSecurity_TotalRemainMin;  //G100_Owen_0006
    UINT8   ucStartupShutter;           //G100_Owen_0017
    UINT8   ucAutoSourceResync;			//G100_Doulas_0006
    UINT8   ucSerialPortOut_BaudRate;   //G100_Owen_0023
    UINT8   ucFadeIn_Time;              //G100_Owen_0023
    UINT8   ucFadeOut_Time;             //G100_Owen_0023
    UINT8   ucPwrKeyBacklight;          //G100_Owen_0024
    UINT8   ucLogoChange;               //G100_Owen_0048
    UINT8   ucFast_Power_On;			//G100_Clare_0055

    UINT8   ucAuto_HDMI_Switch;         //G100_Tim_0057, add ,start //A35G2_BRC_Casper_0060
    UINT8   ucInput_Key_Last_Value; 	//A35G2_BRC_Casper_0060
    UINT8   uc3D_Mode_Last_Value;       //G100_Tim_0057, add, end //A35G2_BRC_Casper_0060
    UINT8   ucProj_Native_Timing;       //G100_Tim_0058, ad //A35G2_BRC_Casper_0060

    UINT8 	ucSystemUpdateStatus;   //A65_OPTOMA_Julie_0015
	UINT8 	ucNotification_SystemUpdate;	//A65_OPTOMA_Julie_0015
	UINT8   ucQuickKey;					//A65_OPTOMA_Doulas_0025
	UINT8   ucFotaModelName[MODEL_NAME_MAX_LENGTH + 1];		//A65_OPTOMA_Doulas_0092

    UINT8	ucProServicePairStatus;
    UINT8   ucProServiceBindStatus;
    UINT8   ucProServicePairCode[PROSERVICE_PAIR_CODE_LEN];
    UINT8   ucProServiceAccount[PROSERVICE_PAIR_ACCOUNT_LEN + 1];
    UINT8   ucOSD_Lock; //A65_OPTOMA_Jerry_0005
    UINT8   ucFOTA_Enable; //A65_OPTOMA_Jerry_0005
    UINT8   ucOMS_Service_Mode;

    UINT8   uc2ndLogoReplaced;// R70G2_Bruce#0019
    UINT8   ucService2ndLogoReplaced; //A65_OPTOMA_Julie_0076
	UINT8 	ucSilentFOTA_SystemUpdate; //A65_OPTOMA_Julie_0086 //A35G2_Coda_0061
	UINT8	ucSilentFOTAStatus; //A65_OPTOMA_Julie_0092 //A35G2_Coda_0061

	UINT8   ucLensSpeed;		//A70Gen2_Julie_0065

    UINT8   ucAudioVolume;// R70K_Bruce_0002
    UINT8   ucAudioMute;// R70K_Bruce_0002

	UINT8 	ucEQ_HDMI1;
	UINT8 	ucEQ_HDMI2;
	UINT8 	ucEQ_DVI;
    UINT8   ucBackFocusSwitch;
    UINT8   ucFujiLensDetect;

	UINT8   ucLensDurationTime_Enable;
	UINT16 	uiLensFocusDurationTime;
	UINT16 	uiLensZoomDurationTime;
	UINT16 	uiLensBackFocusDurationTime;

    UINT8   ucEEPROM_Retention[USER_COMMON_SETTING_RESERVED_NUM];
    UINT16  uiCOMMON_SETTING_CRC;       //A70LV_Doulas_0022 //A70LV_Doulas_0017

	UINT8	ucEnergySaving;

	UINT8	ucHDBaseT_Status;
} sCOMMON_SETTING, *PsCOMMON_SETTING;       //A70LV_Doulas_0015

typedef struct
{
    UINT8   ucInputSourceMain;
    UINT8   ucInputSourceSub;

    UINT8   ucPIPEnable;
    UINT8   ucMainLayout;
    UINT8   ucPIPLayout;
    UINT8   ucPBPLayout;
    UINT8   ucPIPSize;

    UINT8   ucSourceKeyOption ;     //auto source/ list source/ change source
    UINT16  uiSourceInfo;
    //UINT8   ucPowerUpSource;
    UINT8   ucSourceLock;


    UINT8   ucEligibleTerminal[INPUT_SOURCE_MAX];
    UINT8   cSourceName[INPUT_SOURCE_MAX][SOURCE_NAME_MAX_LENGTH];

    UINT8   ucCustomKey;
    UINT8   ucCustomKey2;  //G100_Wilsonj_0053
    UINT8   ucBlankKey;

    UINT8   ucSourceHotKeyEnable ;
    UINT8   ucMainSourceHotKey0 ;
    UINT8   ucMainSourceHotKey1 ;
    UINT8   ucMainSourceHotKey2 ;
    UINT8   ucMainSourceHotKey3 ;
    UINT8   ucMainSourceHotKey4 ;
    UINT8   ucMainSourceHotKey5 ;
    UINT8   ucMainSourceHotKey6 ;
    UINT8   ucMainSourceHotKey7 ;
    UINT8   ucMainSourceHotKey8 ;
    UINT8   ucMainSourceHotKey9 ;

    UINT8   ucEEPROM_Retention[USER_SOURCE_SETTING_RESERVED_NUM];
    UINT8   ucSOURCE_SETTING_CRC;
} sSOURCE_SETTING, *PsSOURCE_SETTING;       //A70LV_Doulas_0015

typedef struct
{
    //Manual Adjustment
    UINT8   ucManualAdjustmentEnable;
    UINT8   ucManualAdjustmentAutoTestPattern;
    UINT16  uiRedPartOfRed;
    UINT16  uiGreenPartOfRed;
    UINT16  uiBluePartOfRed;
    UINT16  uiRedPartOfGreen;
    UINT16  uiGreenPartOfGreen;
    UINT16  uiBluePartOfGreen;
    UINT16  uiRedPartOfBlue;
    UINT16  uiGreenPartOfBlue;
    UINT16  uiBluePartOfBlue;
    UINT16  uiRedPartOfWhite;
    UINT16  uiGreenPartOfWhite;
    UINT16  uiBluePartOfWhite;
    UINT8   ucEEPROM_Reserved[2];  //not use for 4x
} sMANUAL_ADJUSTMENT, *PsMANUAL_ADJUSTMENT;       //A70LV_Doulas_0015

typedef struct
{
    UINT8  ucAspectRatio;   //0
    UINT8  ucOverScan;

    UINT8  ucPhase;
    UINT8  ucTracking;
    UINT8  ucHorzPosition;           //A70LV_Doulas_0029
    UINT8  ucVertPosition;   //5     //A70LV_Doulas_0029

    UINT16 uiDigitalHorzZoom;
    UINT16 uiDigitalVertZoom;
    UINT8  uiDigitalHorzShift;
    UINT8  uiDigitalVertShift;

    UINT16 uiCustomDigitalHorzZoom; //10
    UINT16 uiCustomDigitalVertZoom;
    UINT8  uiCustomDigitalHorzShift;
    UINT8  uiCustomDigitalVertShift;

    UINT8  ucPresetMode;    //display Mode
    UINT8  ucBackupPreset;  //15 //When input source is 100hz or 120hz ,backup guiPresetMode
    UINT8  ucSavePreset;    // bit : 0 --> USER, bit:1 --> ISF Day,  bit 2--> ISF Night
    UINT8  ucPreUserMode;   //User mode 所 save的 ucPresetMode

    UINT8  uc3D_Enable;
    UINT8  uc3D_Invert;
    UINT8  uc3D_SyncOut;    //20
    UINT8  uc3D_SyncIn;
    UINT8  uc3D_Reference; //A70LV_Larry_0169
    UINT8  uc3D_2D_View;
    UINT8  ucEdgeMask;         //ZU860_Doulas_0004 Add
    UINT8  uc3D_Mode;      //ZU860_Clare_0024,
    UINT8  uc3D_Tech;       //G100_Owen_0029
    UINT8  ucAutoImage;    //select mode table(Normal/Wide)
    UINT8  ucDigitalZoomProp;   //G100_Owen_0028

    UINT8  ucEEPROM_Retention[SOUCE_DEPEND_RESERVED_NUM];
}sSOURCE_DEPEND_SETTING,*PsSOURCE_DEPEND_SETTING;


typedef struct
{
    UINT8   ucCS;
    UINT8   ucCT;
    UINT8   ucGamma;
    UINT8   ucBrillientColorEnabled;
    UINT8   ucWhitePeaking;
    UINT8   ucColorEnhancement;

    UINT8   ucSkinColor;
    UINT8   ucSharpness;
    UINT8   ucBrightness;
    UINT8   ucContrast;
    UINT8   ucTint;      //Hue
    UINT8   ucSaturation;
    UINT8   ucRedGain;          //10
    UINT8   ucGreenGain;
    UINT8   ucBlueGain;
    UINT8   ucRedOffset;
    UINT8   ucGreenOffset;
    UINT8   ucBlueOffset;
    UINT8   ucWallColorSet;  //G100_Steven_0047

    //UINT8   ucEEPROM_Retention[COLOR_SETTING_RESERVED_NUM];
}sCOLOR_SETTING,*PsCOLOR_SETTING;   //A70LV_Doulas_0015

typedef struct
{
    sLIGHT_SETTING      sLightSetting;       //A70LV_Doulas_0015
    sOSD_SETTING        sOSD_Setting;        //A70LV_Doulas_0015
    sNETWORK_SETTING    sNetworkSetting;     //A70LV_Doulas_0015
    sIMAGE_SETTING      sImageSetting;       //A70LV_Doulas_0015
    sWARP_SETTING       sWarpSetting;        //A70LV_Doulas_0015
    sCOMMON_SETTING     sCommonSetting;      //A70LV_Doulas_0015
    sSOURCE_SETTING     sSourceSetting;      //A70LV_Doulas_0015
#if SUPPORT_MANUAL_ADJUSTMENT       //A70LV_Doulas_0023
    sMANUAL_ADJUSTMENT  sManualAdjustment;   //A70LV_Doulas_0015
#endif

    sSOURCE_DEPEND_SETTING sSourceDependSetting[INPUT_SOURCE_MAX];      //A70LV_Doulas_0015
    sHSG_SETTING           sHSG_setting[INPUT_SOURCE_MAX][DISPLAY_MODE_MAX];
    sHSG_SETTING           sUserHSG_setting[INPUT_SOURCE_MAX][DISPLAY_MODE_MAX];  //G100_Steven_0016
    sHSG_COLOR_TEMPERATURE sHSG_ColorTemperature[DISPLAY_MODE_MAX][HSG_CT_MAX];
    sCOLOR_SETTING         sColorSetting[INPUT_SOURCE_MAX][DISPLAY_MODE_MAX][SIGNAL_TYPE_MAX];
    sCOLOR_SETTING         sUserColorSetting[INPUT_SOURCE_MAX][DISPLAY_MODE_MAX][SIGNAL_TYPE_MAX]; //G100_Steven_0016
    UINT8  ucSOURCE_DEPEND_SETTING_CRC;     //A70LV_Doulas_0015
    UINT8  ucCOLOR_DEPEND_SETTING_CRC;
}sUSER_SYSTEM_SETTING, *PsUSER_SYSTEM_SETTING;  //A70LV_Doulas_0013


typedef struct
{
    //SN
    UINT8   ucSerialNumber[SN_LENGTH];
    UINT8   ucSerialNumberSetCount;

    UINT16  uiLampResets[2]; //0: Lamp #1,  1: Lamp #2
    UINT8   ucLampPowerSwitchState;

    // Language with first on Flag
    UINT8   ucFirstStartupFlag;
    UINT8   ucUSTFirstStartup;

    //PassWord with security enabled.
    // BYTE cSecurityEnable;
    UINT8   ucPINProtect;
    UINT8   ucPassWord[5];

    //color wheel index.    // 0:2x , 1: 3x  //
    UINT16  uiColorWheelIndex[2];
    UINT16  uiFWIndex[2];
    UINT16  uiPWIndex[2];

    //lamp DB
    UINT8   ucDAhomeDual;
    UINT8   ucDAMax;
    UINT8   ucDAMin;
    UINT8   ucDAapertureState;

    //Total projector hour
    UINT32  ulTotalProjectorMinute;
    UINT32  ulLightSourceMinute[eTIMER_RECORD_NUMBER];
	UINT32  ulTotalProjectorMinute_Factory;//A35G2_David_0010
	UINT32  ulLightSourceMinute_Factory[eTIMER_RECORD_NUMBER];//A35G2_David_0010
    UINT32  ulWaterCoolingMinute;

    //ABC
    UINT16  uiTargetBright;         // A70LH_Jonas_0015, 儲存 ABP On 當下的亮度值
    UINT8   ucpwrOff_ABC;       // A70LH_Jonas_0086
    UINT16  ucTargetIntensity[6];       // For R/G/B/Y/W/RLD            // A70LH_Jonas_0090


    UINT8   ucABP_CalibrateFlag;  //A70LV_Doulas_0023 modify

    //sequence select
    UINT8   ucPOWER_OFFSET;
    UINT8   ucCOLOR_OFFSET;

    //lens
    UINT8   ucUST_LensInstallType;    //A16 lens
    UINT8   ucLensType ;

    UINT8   ucDisableSnapshots;
    INT8    cAutoKeystoneOffset;
    UINT8   ucMCU_PorgramFinish;
    UINT64  ulDbMask;
    UINT64  ulDbMaskCT;
    UINT8   ucWF_UpgradeFlag;
    UINT8   ucOPFU_Check;

    UINT8   ucPanel;

    UINT16  uiLightSensorFull_BLD[eLD_SEQ_NUMBER]; //Y R B G
    UINT16  uiLightSensorEco_BLD[eLD_SEQ_NUMBER];  //Y R B G
    UINT16  uiLightFull_PWM_BLD[eLD_SEQ_NUMBER];   //Y R B G
    UINT16  uiLightEco_PWM_BLD[eLD_SEQ_NUMBER];    //Y R B G

    UINT16  uiLightSensorFull_RLD[eLD_SEQ_NUMBER]; //Y R B G
    UINT16  uiLightSensorEco_RLD[eLD_SEQ_NUMBER];  //Y R B G
    UINT16  uiLightFull_PWM_RLD[eLD_SEQ_NUMBER];   //Y R B G
    UINT16  uiLightEco_PWM_RLD[eLD_SEQ_NUMBER];    //Y R B G

    UINT16  uiLightSensorFull_Dynamic_RLD[eLD_SEQ_NUMBER]; //Y R B G
    UINT16  uiLightSensorEco_Dynamic_RLD[eLD_SEQ_NUMBER];  //Y R B G
    UINT16  uiLightFull_PWM_Dynamic_RLD[eLD_SEQ_NUMBER];   //Y R B G
    UINT16  uiLightEco_PWM_Dynamic_RLD[eLD_SEQ_NUMBER];    //Y R B G

    UINT32  ulLightSourceMinuteNormal; //G100_Larry_0006
    UINT32  ulLightSourceMinuteEco;    //G100_Larry_0006
    UINT32  ulLightSourceMinuteQuiet;  //G100_Larry_0006
    UINT32  ulLightSourceMinuteCustom; //G100_Larry_0006

	//UINT32  ulTotalProjectorMinute2; //A70LV_Larry_0268
	//UINT32  ulLightSourceMinute2[eTIMER_RECORD_NUMBER]; //A70LV_Larry_0268
	//UINT32  ulWaterCoolingMinute2; //A70LV_Larry_0268
	//UINT8   ucColorOffset;  //A70LV_John_0072 add color offset default value
	UINT8 	ucEQ_HDMI1;//no use
	UINT8 	ucEQ_HDMI2;//no use
	UINT8 	ucEQ_DVI;//no use

    UINT8   ucLightSensorTime[eLD_SEQ_NUMBER]; //Y R B G         	//A65_OPTOMA_Doulas_0105
    UINT16  uiLightSensorTarget[eLD_SEQ_NUMBER]; //Y R B G        	//A65_OPTOMA_Doulas_0105
    UINT16  uiLightSensorTargetRLD[eLD_SEQ_NUMBER]; //Y R B G    	//A65_OPTOMA_Doulas_0105
    float afiLightSensorGain[eLD_SEQ_NUMBER];   //Y R B G       //A70G2_Owen_0002
    UINT16 uiLightSensorOffset[eLD_SEQ_NUMBER]; //Y R B G       //A70G2_Owen_0002
    UINT8   ucModelReplaceSelect; //A65_OPTOMA_Julie_0062
    UINT8   ucModelSwitchAdjust; //A65_OPTOMA_Julie_0064
	UINT8   ucRegion;  //A65_OPTOMA_Doulas_0027 //Optoma EMEA or non EMEA
	UINT8   ucDefaultLogo;	//A65_OPTOMA_Doulas_0027
	UINT8   ucACU_File_Valid;								  // ACU data is valid or not
	UINT8   ucACU_Display_Mode; 							  // The display mode during ACU Calibration
    UINT8   ucACU_Target_Select;                              // The target for ACU Calibration       //G100_Tim_0048, add
    UINT8   ucOsdBurnInIndex;////A35G2_Coda_0091
    UINT8   ucACU_Data_Enable; //A35G2_Alan_0007
    UINT8   ucBIST; //HICC2_Steven_0004
    UINT8   cEEPROM_Retention[SYSTEM_DEFAULT_RESERVED_NUM];
    UINT8   ucSYSTEM_DEFAULT_CRC;        //A70LV_Doulas_0015

    UINT8   ucLightSensorTime_Second[eLD_SEQ_NUMBER]; //Y R B G    //HICC2_Jacky_0003
    UINT16  uiLightSensorTarget_Second[eLD_SEQ_NUMBER]; //Y R B G  //HICC2_Jacky_0003
    float   afiLightSensorGain_Second[eLD_SEQ_NUMBER];   //Y R B G       //HICC2_Jacky_0003
    UINT16  uiLightSensorOffset_Second[eLD_SEQ_NUMBER]; //Y R B G       //HICC2_Jacky_0003
    UINT16  uiLightSensorInfo[eLD_SEQ_NUMBER]; //HICC2_Jacky_0003
    UINT8   ucLightSensorPosition;       //HICC2_Jacky_0003
}sSYSTEM_DEFAULT,*PsSYSTEM_DEFAULT;     //A70LV_Doulas_0013


typedef struct
{
    //timing save
    //UINT8  ucMode_Flag[5];  //A70LV_Doulas_0194 remove
    //UINT8  ucTable_ID[5];
    //UINT16 uiMode_ID[5];
    //INT16  uiHStart[5];
    //INT16  uiVStart[5];
    UINT8  ucNewMA_Mode_Flag[20];
    UINT8  ucNewMA_Source_ID[20];
    UINT8  ucNewMA_Table_ID[20];
    UINT16 uiNewMA_Mode_ID[20];
    UINT16 uiNewMA_HPeriod[20];
    UINT16 uiNewMA_VTotal[20];
    UINT16 uiNewMA_HTotal[20];
    UINT16 uiNewMA_VActive[20];
    UINT16 uiNewMA_HActive[20];
    INT16  uiNewMA_HStart[20];
    INT16  uiNewMA_VStart[20];
}sTIMING_TABLE,*PsTIMING_TABLE; //Timing table  //A70LV_Doulas_0013


typedef struct sLAYOUT_VERSION
{
    UINT8 ucSubminor;
    UINT8 ucMinor;
    UINT8 ucMajor;
    UINT8 ucFirmwareID[17];

} sLAYOUT_VERSION,*PsLAYOUT_VERSION;    //A70LV_Doulas_0017

typedef struct
{
    UINT8   ucDAC_AWC0;
    UINT32  ulSubframeDelay_AWC0;
    UINT16  uiSegmentLength_AWC0;
    UINT32  uiEepromTargetADC_AWC0;
    UINT8   ucDAC_AWC1;
    UINT32  ulSubframeDelay_AWC1;
    UINT16  uiSegmentLength_AWC1;
    UINT32  uiEepromTargetADC_AWC1;

    UINT8   ucCloseLoopEn_Axis0;
    UINT32  ucCloseLoopGain_Axis0;
    UINT32  ucCloseLoopSegmentLength_Axis0;
    UINT8   ucCloseLoopEn_Axis1;
    UINT32  ucCloseLoopGain_Axis1;
    UINT32  ucCloseLoopSegmentLength_Axis1;
    UINT8  ucCloseLoopSN[32];
    UINT8  ucXPRVersion[8]; //HICC2_Steven_0039
    UINT32  ucXPRCloseLoopErrCode0; //HICC2_Steven_0051
    UINT32  ucXPRCloseLoopErrCode1; //HICC2_Steven_0051
    UINT32  uiEepromZData0;
    UINT32  uiEepromZData1;
    UINT8  ucWriteEEPROM_Flag;
    UINT32  uiEepromXData0;
    UINT32  uiEepromXData1;
    UINT32  uiEepromYData0;
    UINT32  uiEepromYData1;
} sACTUATOR_SETTING;

typedef struct
{
    sADC_CAL_VALUES sADC_cal_values;       //VGA cal
    sWAVEFORMSTATE sWaveformstate; //Lamp
    sBURNIN_INFORMATION sBurin_Information;
    sSYSTEM_DEFAULT sSystemDefault;

    sUSER_SYSTEM_SETTING sUserSystemSetting;
    sTIMING_TABLE sTiming_Table;      //A70LV_Doulas_0194 Add

    uCCT_TABLE uCCT_Table;
    sTEC_GATING_INFO sTecGating;
    sACTUATOR_SETTING sActuatorSetting;

    sLAYOUT_VERSION sLayoutVersion;    //A70LV_Doulas_0017 modify
}sEEPROM_SETTINGS, *psEEPROM_SETTINGS;      //A70LV_Doulas_0015 modify

#define EEPROM_SIZE  sizeof(sEEPROM_SETTINGS)/sizeof(UINT8)
#define LD_INFO_STRING (30)
//#define LD_INFO_NUM (12)   //H2 wait review : 1
//#define BLD_NUM (8) //G100_Steven_0003  //H2 wait review : 1
//#define RLD_NUM (4)

typedef struct
{
    UINT8 ucStatus;
    UINT8 ucResolution[24];
    UINT8 ucHorzRate[24];
    UINT8 ucColorSpace[24];
}sBACKUPINPUT_INFO;

//---//G100_Coda_0010 start
typedef struct
{
    UINT16 uwTimeMin;	//G100_Coda_0011
    UINT8 ucFunctionType;
    UINT8 ucEventIndex;
}sWEEKDAY_EVENT;

typedef struct
{
    UINT8 ucWeekdayEnable;
    UINT8 ucIsEventExist;
    sWEEKDAY_EVENT sWeekDayEvent[16];
}sWEEKDAY_INFO;

typedef struct
{
    UINT8 ucScheduleMode;
    sWEEKDAY_INFO sWeekday_Info[7];
}sSCHEDULE;//G100_Coda_0009
//---//G100_Coda_0010 end

typedef struct
{
    UINT16 uwYear;
    UINT8 ucMonth;
    UINT8 ucDay;
    UINT8 ucHour;
    UINT8 ucMinute;
    UINT8 ucSecond;
    UINT8 ucClockMode;
    UINT8 ucNtpServer;
    UINT8 ucDaylightSavingTime;
    UINT8 ucTimeZone;
    UINT8 ucUpdateInterval;
}sDATE_TIME_INFO;////G100_Coda_0033

typedef struct
{
    UINT8  CurMotorLensID;
    UINT8  CurMotorLensModule;
    UINT16 CurMotorLensCalStatus;
    UINT8  CurMotorLensMemoryStatus;
    UINT8  CurMotorLensMemoryRecord;
    UINT8  CurMotorLensMovingState;
}sCURRENT_INFO;

typedef struct
{
    UINT32  UI_Refresh;
    UINT32  UI_DrawSplash;
    UINT32  UI_PIN_Protect_Enable;
    UINT32  UI_CurrentSplash;
    UINT32  UI_WaitProcessMsgExit;
    UINT32  UI_LensMovingTimeOut;
    UINT32  UI_MenuOpenLensMovingMsg;
    UINT32  UI_LensShiftExit;
    UINT32  UI_LensCalTimeOut;
    UINT32  UI_MenuOpenLensCalMsg;
    UINT32  UI_LensCalMsgExit;
    UINT32  UI_LenRunBorder;
    UINT32  UI_PowerIncreaseMsg;
    UINT32  UI_NetworkWaitMsgExit;
    UINT32  UI_HSG_TP_ON_CHECK;
    UINT32  UI_HSG_TP_OFF;

    UINT32  UI_MenuOpenWaitMsg;
    UINT32  UI_TestPattern;
    UINT32  UI_EmergencyShow;

    UINT32  UI_DisconnectTwistMsg;
    UINT32  UI_HSGTestPattern;
    UINT32  UI_OSDExit;
    UINT32  UI_DrawPIPLayout;
    UINT32  UI_AutoFocusMsg;
    UINT32  UI_AutoColorUniformityMsg;
    UINT32  UI_OpenFreezeMsg;
    UINT32  UI_DrawPIPLayoutPosition;

    UINT32  UI_Init;
    UINT32  UI_Standby;
    UINT32  UI_OSDOpen;
    UINT32  UI_List_All_Source;
    UINT32  UI_Source_Menu_Open;
    UINT32  UI_PanelChange;
    UINT32  UI_Source_Message;
    UINT32  UI_ABC_Result;
    UINT32  UI_UpdateOsdIfNoMenu;
    UINT32  UI_UpdateOsdIfNoSplash;
    UINT32  UI_TestPatternOn;
    UINT32  UI_TestPatternOff;
    UINT32  UI_AutoFocusStatus;  //unused
    UINT32  UI_AutoWallColorStatus;  //unused
    UINT32  UI_AutoWallColorMsg;  //unused
    UINT32  UI_AF_Cal_Complete;
    UINT32  UI_AF_Cal_Fail;
    UINT32  UI_AC_Cal_Complete;
    UINT32  UI_AC_Cal_Fail;
    UINT32  UI_ACU_Cal_Complete;
    UINT32  UI_ACU_Cal_Fail;
    UINT32  UI_ACM_Cal_Complete;
    UINT32  UI_ACM_Cal_Fail;
    UINT32  UI_BurnInPatternUpdate;
    UINT32  UI_Sub_Soruce_Infor_MSG;
    UINT32  UI_SecurityShutDown_Query;
    UINT32  UI_Auto_Power_Off_Exit;
    UINT32  UI_UpdateOsdCLI;
    UINT32  UI_UpdateOsdPolling;
    UINT32  UI_Unknow_Filter_Message; //HICC2_Doulas_0060
    UINT32  UI_FirstEventNumer;
    UINT32  UI_DuplicateCheck;
    UINT32  UI_Service_Code_Menu_Open;
    UINT32  UI_PinProtectClear;
    UINT32  UI_DataCodeAndUpdateCheck;
    UINT32  UI_AutoBrightnessCalMsg;
    UINT32  UI_KeypadLock;// HICC2_Bruce_0005
    UINT32  UI_OSD_Lock;// HICC2_Bruce_0015
    UINT32  UI_System_Update;// HICC2_Bruce_0010
    UINT32  UI_ProService_Pair;// HICC2_Bruce_0010
    UINT32  UI_ProService_Bind;// HICC2_Bruce_0010
	UINT32  UI_LogoReplaceStatus;// HICC2_Bruce_0013
	UINT32	UI_FOTA;
    UINT32  UI_BackFocus_Menu_Open;

}sUI_EVENT_INFO;

//Available functions Info
typedef struct
{
    UINT32 AFN_MainInput;
    UINT32 AFN_SubInput;
    UINT32 AFN_SizePresets;
    UINT32 AFN_PictureSetttings;
    UINT32 AFN_WarpMemoryApply;
    UINT32 AFN_BlendMemoryApply;
    UINT32 AFN_LensMemoryApply;
    UINT32 AFN_BackupRestoreRestore; //HICC2_Doulas_0039
    UINT32 AFN_UserMode;
    UINT32 AFN_Gamma;
    UINT32 AFN_MainColorSpace;
    UINT32 AFN_SubColorSpace;
    UINT32 AFN_3DEnable; //H30K_Doulas_0068
}sAFN_INFO;

typedef struct
{
    UINT8 ART_NET_Enable;
    UINT8 ART_NET_Net;
	UINT8 ART_NET_Subnet;
	UINT8 ART_NET_Universe;
	UINT16 ART_NET_StartAddress;
	UINT8 ART_NET_ChannelSettings;
    UINT8 ART_NET_User1_CH1;
	UINT8 ART_NET_User1_CH2;
	UINT8 ART_NET_User1_CH3;
	UINT8 ART_NET_User1_CH4;
	UINT8 ART_NET_User1_CH5;
	UINT8 ART_NET_User1_CH6;
	UINT8 ART_NET_User1_CH7;
	UINT8 ART_NET_User1_CH8;
	UINT8 ART_NET_User1_CH9;
	UINT8 ART_NET_User1_CH10;
	UINT8 ART_NET_User1_CH11;
	UINT8 ART_NET_User1_CH12;
	UINT8 ART_NET_User1_CH13;
	UINT8 ART_NET_User1_CH14;
	UINT8 ART_NET_User1_CH15;
	UINT8 ART_NET_User1_CH16;
    UINT8 ART_NET_User2_CH1;
	UINT8 ART_NET_User2_CH2;
	UINT8 ART_NET_User2_CH3;
	UINT8 ART_NET_User2_CH4;
	UINT8 ART_NET_User2_CH5;
	UINT8 ART_NET_User2_CH6;
	UINT8 ART_NET_User2_CH7;
	UINT8 ART_NET_User2_CH8;
	UINT8 ART_NET_User2_CH9;
	UINT8 ART_NET_User2_CH10;
	UINT8 ART_NET_User2_CH11;
	UINT8 ART_NET_User2_CH12;
	UINT8 ART_NET_User2_CH13;
	UINT8 ART_NET_User2_CH14;
	UINT8 ART_NET_User2_CH15;
	UINT8 ART_NET_User2_CH16;
	UINT8 ART_NET_Reset_User1;
	UINT8 ART_NET_Reset_User2;
	UINT8 ART_NET_Reset;
}sART_NET_INFO;

typedef struct
{
    INT32  lInputVoltage;
    INT32  lPFC_OutputVoltage;
    INT32  l5V_Voltage;
    INT32  l12V_Voltage;
    INT32  l52V_Voltage;
    INT32  l5V_Current;
    INT32  l12V_Current;
    INT32  l52V_Current;

    INT32  lPFC_MOS_Temperature;
    INT32  lBridge_Temperature;
    INT32  lLVPS_Ambient_Temperature;
    INT32  l12V_SR_MOSFET_Temperature;
    INT32  l52V_SR_MOSFET_Temperature;

    UINT16 uiPRIMARY_PROTECTION_STATUS;
    UINT16 uiSECONDARY_PROTECTION_STATUS;
    UINT16 uiIO_STATUS;
} sLVPS_INFO; //HICC2_Doulas_0165

#define MAX_LD_SW_PORT 16

typedef struct
{
    struct
    {
    	UINT16 uiFanRPM[20];
    	UINT16 uiFanDuty[20];
    	//UINT16  uiFanLock;
    	UINT16 uiPump;
        UINT8  ucFanFilter; //A70LV_Larry_0287
        UINT8  ucHighAltitudeEnable;
    }sFAN_INFO;

    struct
    {
    	//UINT8 ucThermalSensorTemperature[7]; //T100_Casper_0001
    	INT16   iThermalSensorTemperature[8]; //A70LV_Larry_0033
    	UINT8   ucDimmingMode; //T100_Casper_0004
    }sTHERMAL_SENSOR_INFO;

    struct
    {
        UINT8  uiLD_INFO_Num;   //LD_INFO_NUM  define in MCU
        UINT8  uiBLD_Num;       //BLD_NUM  define in MCU
        UINT8  uiRLD_Num;       //RLD_NUM  define in MCU
        UINT16 uiLD_Voltage[MAX_LD_SW_PORT];
        UINT16 uiLD_Current[MAX_LD_SW_PORT];
        INT16  uiLDTemperature[MAX_LD_SW_PORT];
    }sLD_INFO;

    struct
    {
        UINT16 uiTecVoltage[3];
        UINT16 uiTecCurrent[3];
    }sTEC_INFO;	//G100_Clare_0017

    struct
    {
        UINT16 uiLightSenosr[7];
    }sLIGHT_SENSOR_INFO; //A70LV_Larry_0033

    struct      //A70LV_Doulas_0027
    {
        BOOL    bSourceActive[2];       //[0]:main , [1]:sub
        UINT8   ucSourceState[2];       //0:resolution , 1:Checking for Signal , 2:Seraching , 3:Setting Up Image , 4:Signal Out of Range
        UINT8   ucInputColorSpace[2];
        UINT16  uiInputHorzRes[2];
        UINT16  uiInputVertRes[2];
        UINT16  uiInputHorzFreq[2];
        UINT16  uiInputVertFreq[2];
        UINT16  uiInputPixelClock[2];
        UINT16  uiOutputHorzRes;
        UINT16  uiOutputVertRes;
        UINT16  uiOutputHorzFreq;
        UINT16  uiOutputVertFreq;
        eINPUT_3D_TYPE e3DConfigType;
    }sSOURCE_INFO;

    struct
    {
        UINT8   ucOSD_Show;                 //T100_Simon_0004   //remote "OSD" key to enable/disable
        UINT8   ucPictureMute;              //T100_Simon_0005   //1 : blank screen
        UINT8   ucLastSerialCmdErrorCode;   //T100_Simon_0002
        UINT8   ucColorMask;
        UINT16  uiPanel_H_Resolution;
        UINT16  uiPanel_V_Resolution;
        UINT8   ucPanel_Is3DOutput;
        eORIENTATION_STATE eOrientationState;
    }sSYSTEM_VARIABLE;

    struct
    {
        UINT8  ucSystemMode;
        UINT8  ucSpoke;
        UINT8  ucSplash;
    }sDDP_SETTING; //A70LV_Larry_0048

    struct
    {
        UINT8 m_System_Version[8];
        UINT8 m_LD_Version[8];
        UINT8 m_FrontEndVersion[8];
        UINT8 m_FMCUVersion[8];
        UINT8 m_FormatVersion[24];
        UINT8 m_Motor_Version[8];
        UINT8 m_KeyPad_Version[8];
        UINT8 m_LAN_Version[8];
        UINT8 m_PW_Version[8];
        UINT8 m_FPGA1_Version[8];
        UINT8 m_FPGA2_Version[8];
        UINT8 m_FPGA3_Version[8];
        UINT8 m_XFPGA_Version[16];
        UINT8 m_Geometry_Version[8];
        UINT8 m_HDbaseT_Version[16];
        UINT8 m_STB_Version[8];
		UINT8 m_Camera_Version[16];     //G100_Clare_0024, add
		UINT8 m_LD_ConvertVersion[8];
        UINT8 m_OsdVersion[16];
        UINT8 m_ReleaseVersion[16];
        UINT8 m_OESN1[VERSION_STRING_MAX_LENGTH];           //H30K_Tim_0011, add
        UINT8 m_OESN2[VERSION_STRING_MAX_LENGTH];           //H30K_Tim_0011, add
    }sVERSION; //A70LV_Larry_0159

    struct
    {
        UINT8 ucGeoClear;
        UINT8 ucGeoDrawOn;
        UINT8 ucGeoFreeze;

        UINT8 ucAdvWarpPatternIndex;   //H2PF_Simon_0114
    }sGEO_INFO;  //H2PF_Simon_0038

    sLENS_INFO sLensInfo; //A70LV_Larry_0287
    sSOURCE_AVI_INFO sSourceAVIInfo;

    UINT8 ucLightSensorCalibrationProcess; //A70LV_Larry_0152
    UINT8 ucTwistLink; //A70LV_Larry_0168
    UINT8 ucTwistTestPattern;
    UINT8 ucPowerStatusInfo[3];
    UINT8 ucNetworkWaitMessage; //A70LV_Larry_0278
    UINT8 ucNetworkResetFlag; //A70LV_Larry_0278
    UINT8 ucHDR_CurrentStatus;
    UINT8 sGensorMode;
    UINT8 ucUIBlendingEnable;

    sBACKUPINPUT_INFO sPrimaryInfo;
    sBACKUPINPUT_INFO sSecondaryInfo;
    UINT8 ucBackupInputStatus;
    UINT8 ucConfirmation;     // for Yes/No	//ZU860_Clare_0019,
    UINT8 ucCurrentSecurityCode[5];
    UINT8 ucSecurityMonth;        //G100_Owen_0006
    UINT8 ucSecurityDay;          //G100_Owen_0006
    UINT8 ucSecurityHour;         //G100_Owen_0006
	UINT8 ucSecurityPollCount;// HICC2_Bruce_0016

    sDATE_TIME_INFO sDateTimeInfo;   //G100_Coda_0033
     _sDateTime sLanDateTimePacket;  //G100_Coda_0033
    UINT16 wPress[3];					//G100_Clare_0017  //HICC2_Steven_0014//HICC2_Julie_0006
    INT16 iGsensor[3];				//G100_Clare_0017
    UINT8 ucModelRegion;
    UINT8 uc4cornerIndex;
    UINT8 ucCurrentWeekday;			//G100_Coda_0014
    UINT8 ucCurrentEventListNum;		//G100_Coda_0014
    UINT8 ucCurrentCopyWeekdayNum; //G100_Coda_0025
    UINT16 uiDuplicate_TimeMin;
    UINT8 ucDuplicate_FuncType;
    UINT8 ucDuplicate_EventIdx;
    sSCHEDULE sScheduleInfo;		//G100_Coda_0009
    sLAN_SCHEDULE_PACKET sLanSchedulePacket;	 //G100_Coda_0021
    sDATE_TIME_INFO sTempDateTimeInfo;
    UINT16 wHumidity[3];				//G100_Clare_0047  //HICC2_Steven_0014//HICC2_Julie_0006
    UINT8 ucTE_ID;					//G100_Clare_0047
    UINT8 ucSystemProtectStatus;	//G100_Coda_0095
    UINT8 ucLensCenterSetting;
    UINT8 ucLensCalibrationDoing;
    INT8 ucAC_Voltage[AC_VOLTAGE_MAX_LEN];                  //G100_Tim_0009, add
    eDATA_SYNC_STATUS eDataSyncStatus ;
    UINT8 ucHW_ID;
    _sEmail sLanEmailPacket;			//A65_OPTOMA_Julie_0024
    UINT8 ucSystem_Busy;                //HICC2_Doulas_0119

	UINT8 ucEmail_Notify_FanError;		//A65_OPTOMA_Julie_0024
	UINT8 ucEmail_Notify_Power;			//A65_OPTOMA_Julie_0024
	UINT8 ucEmail_Notify_VideoLoss;		//A65_OPTOMA_Julie_0024
	UINT8 ucEmail_Notify_Laser;			//A65_OPTOMA_Julie_0024
	UINT8 ucEmail_Notify_TermalSensors;	//A35G2_Simon_0084

    _sSnmp     sLanSnmpPacket;      //HICC2_Simon_0021
    _sCcv2     sLanCcv2Packet;      //HICC2_Simon_0021
    _sXioCloud sLanXioCloudPacket;  //HICC2_Simon_0021

    UINT32 ulProServiceErrorCode;
    UINT8 ucProServiceLocation;
    UINT8 ucCurrentReplaceModelCode[8];	 //A65_OPTOMA_Julie_0056
    UINT8 ucProServiceTemperatureStatus; //A35G2_Coda_0048
    UINT8 ucProServiceFanStatus;  //A35G2_Coda_0048
	UINT8 ucDimPower;
    UINT8   ucDimPowerFlag; //A70LK_Larry_0119
	UINT8   ucConstantPowerlevelForDimPower; //A70LK_Larry_0119

    UINT8   ucActuatorFixedOut_AWC0;
    UINT8   ucActuatorFixedOut_AWC1;
    UINT8   ucActuatorPattern; //H2PF_Simon_0057
    UINT8   ucActuatorPatternChange;
    UINT8   ucXPRFilter;

    UINT8 ucBroadCastMsg[BROADCAST_MSG_LEN + 1];            //Optoma_Tim_0040, add
    UINT8 ucNetworkStatusMessage;

    UINT8 ucHDRDemo;

    sKEY_DATA InputKeyID;
    ePOWER_CONTROL PowerControl;
    sPOWER_STATUS_INFO PowerState;

    sCURRENT_INFO sCurInfo;
    sUI_EVENT_INFO sUIEventInfo;
    //sAFN_INFO sAFNInfo;
    //sART_NET_INFO sArtNetInfo;// HICC2_Bruce_0017

    UINT16 wAbsoluteHumidity[3];//HICC2_Julie_0006
    UINT16 wWheelSpeed[2];

    UINT8 ucCamera_OSD_Locked;
    UINT8 ucCamera_Module_Status;
    UINT8 ucAuto_Focus_Execute;
    UINT8 ucAuto_Focus_Status;
    UINT8 ucAuto_Wall_Color_Execute;
    UINT8 ucAuto_Wall_Color_Status;
    UINT8 ucAuto_Wall_Color_Apply;

    UINT8 ucACU_Exectue;
    UINT8 ucACU_Status;
    UINT8 ucACU_Target_Tmp;
    UINT8 ucACU_Reset_Status;
    UINT8 ucACU_Target_Status;
    UINT8 ucACU_Apply;
    UINT8 ucACU_DisableBlend;

    UINT8  ucDMD_Airtight_Status; //HICC2_Steven_0031
    UINT8  cOmsServicemode;
    UINT8  cSupportGroup;

    UINT8  cGroupingEnable;
    UINT8  cGroupingEnableDone;
    UINT8  cGroupingSearch;
    UINT8  cGroupingSearchDone;
    UINT8  cGroupingSelect;
    UINT8  cGroupingSelectDone;
    UINT8  cGroupingSelectSave;
    UINT8  cGroupingSelectSaveDone;
    UINT8  cGroupingReset;
    UINT8  cGroupingResetDone;
    UINT8  cGroupingIdentify[32];
    UINT8  cGroupingIdentifyDone;
    UINT8  cGroupingMember[32];
    UINT8  cGroupingMemberDone;
    UINT8  cGroupingPage;
    UINT8  cGroupingSaveResult;
    UINT8  cGroupingStatus;
    UINT8  cGroupingProjectorName[32];
    UINT8  cGroupingName[32];
    UINT16 uiGroupingID;
    UINT8  cGroupingFreeze;
    UINT8  cGroupingPictureMute;
    UINT8  cGroupingDB;
    UINT8  cGroupingSchedule;
    UINT8  cGroupingAutoFocus;
    UINT8  cGroupingAutoColor;
    UINT8  cGroupingAutoColorExecute;
    UINT8  cGroupingPowerStatus;
    UINT8  cGroupingStandbyMode;
    UINT8  cGroupingMainInput;
    UINT8  cGroupingPowerMode;
    UINT8  cGroupingEventFreeze;
    UINT8  cGroupingEventShutter;
    UINT8  cGroupingEventDB;
    UINT8  cGroupingEventSchedule;
    UINT8  cGroupingEventAutoFocus;
    UINT8  cGroupingEventAutoColorMatch;
    UINT8  cGroupingSetLeader[32];
    UINT8  cGroupingSetLeaderDone;
    UINT8  cGroupingAutoColorMatch;
    UINT8  cGroupingAutoColorSelect;
    _sGROUPING_CLIENT_RESULT sGroupingAutoColorMatchStatus;
    _sGROUPING_CLIENT_RESULT sGroupingAutoFocusStatus;
    UINT8  cGroupingScheduleShutter;

    sGROUPING_SEARCH_INFO sGroupingMenuSearch;
    sGROUPING_SELECT_INFO sGroupingMenuSelect;

    _sGroupSearchInfo sGroupingSearch;
    _sGroupSelectInfo sGroupingSelect;

    UINT8   cLensHeaderVerdorSN[MOTOR_SN_NUMBER];
    sLENS_HEADER sLensHeaderInfo;
    UINT8   cTecConfigVersion;
    UINT8 ucQuickKeyLock;// HICC2_Bruce_0011
    UINT8 ucEditNumber_5_Digits;
    sLVPS_INFO sLVPS_Info;      //HICC2_Doulas_0165

}sSYSTEM_VALUES;

typedef struct
{
    UINT8 ucLAN_MAC_Address[20];
    UINT8 ucWLAN_MAC_Address[20];
    UINT8 ucWLAN_SSID[32];

    UINT8 ucLAN_DHCP;
    UINT8 ucLAN_IP[4];
    UINT8 ucLAN_MASK[4];
    UINT8 ucLAN_GATEWAY[4];
    UINT8 ucLAN_Primary_DNS[4];
    UINT8 ucLAN_Secondary_DNS[4];

    UINT8 ucLAN_DHCP_Show;
    UINT8 ucLAN_IP_Show[4];
    UINT8 ucLAN_MASK_Show[4];
    UINT8 ucLAN_GATEWAY_Show[4];
    UINT8 ucLAN_DNS_Show[4];
    UINT8 ucLAN_Primary_DNS_Show[4];
    UINT8 ucLAN_Secondary_DNS_Show[4];

    UINT8 ucWLAN_Enable;
    UINT8 ucWLAN_Start_IP[4];
    UINT8 ucWLAN_End_IP[4];
    UINT8 ucWLAN_MASK[4];
    UINT8 ucWLAN_GATEWAY[4];

    UINT8 ucWLAN_Enable_Show;
    UINT8 ucWLAN_Start_IP_Show[4];
    UINT8 ucWLAN_End_IP_Show[4];
    UINT8 ucWLAN_MASK_Show[4];
    UINT8 ucWLAN_GATEWAY_Show[4];

    //G100_Wilsonj_0061 Start
    UINT8 ucCrestron_Enable;
    UINT8 ucCrestron_IP[4];
    UINT16 ucCrestronIP_ID;
    UINT16 ucCrestronIP_Port;

    UINT8 ucCrestron_Enable_Show;
    UINT8 ucCrestron_IP_Show[4];
    UINT16 ucCrestronIP_ID_Show;
    UINT16 ucCrestronIP_Port_Show;

    UINT8 ucPJLink_Enable;
    UINT8 ucPJLink_IP[4];

    UINT8 ucPJLink_Enable_Show;
    UINT8 ucPJLink_IP_Show[4];
    UINT8 ucPJLink_IPv6[64];

    UINT8 ucExtron_Enable;
    UINT8 ucAMX_Enable;
    UINT8 ucTelnet_Enable;
    UINT8 ucHTTP_Enable;

    UINT8 ucExtron_Enable_Show;
    UINT8 ucAMX_Enable_Show;
    UINT8 ucTelnet_Enable_Show;
    UINT8 ucHTTP_Enable_Show;
    //G100_Wilsonj_0061 End

    UINT8 ucLAN_Path;           //G100_Owen_0026
    UINT8 ucLAN_Path_Show;      //G100_Owen_0027
    UINT8 ucLAN_Status;         //G100_Owen_0026
    UINT8 ucWLAN_Link_Status;   //R70K_AC_0092

    UINT8 ucPJLink_SecurityEnable;				//A65_OPTOMA_Julie_0034 //A35G2_Coda_0048
    char ucPJLink_SecurityPasssword[64];		//A65_OPTOMA_Julie_0034 //A35G2_Coda_0048

    UINT8 ucPJLink_SecurityEnable_Show;			//A65_OPTOMA_Julie_0034 //A35G2_Coda_0048
    char ucPJLink_SecurityPasssword_Show[64];	//A65_OPTOMA_Julie_0034 //A35G2_Coda_0048

    UINT8 ucLAN_IPV6_DHCP; //HICC2_AC_0050 // HICC2_Bruce_0020
    UINT8 ucLAN_IPV6_Prefix_Length;
    UINT16 ucLAN_IPV6_IP[8];
    UINT16 ucLAN_IPV6_GATEWAY[8];
    UINT16 ucLAN_IPV6_DNS[8];

    UINT8 ucLAN_IPV6_DHCP_Show;
    UINT8 ucLAN_IPV6_Prefix_Length_Show;
    UINT16 ucLAN_IPV6_IP_Show[8];
    UINT16 ucLAN_IPV6_GATEWAY_Show[8];
    UINT16 ucLAN_IPV6_DNS_Show[8];

}sLAN_INFO_VALUES;


///////////////////////////////////////////////////////////////////////////////////
///@brief sEEPROM_SETTINGS : EEPROM data structure
///@author Roger Chen
///////////////////////////////////////////////////////////////////////////////////


#if 0
typedef struct
{
	sSYSTEM_VERSION sSysVersion;
	sUSER_SYSTEM_SETTINGS sUserSysSettings;
}sEEPROM_SETTINGS, *PsEEPROM_SETTINGS;
#endif


typedef struct
{
    struct
    {
        UINT8 ucStartupState;
        UINT8 ucDefaultPanelId;
    }System;

    struct
    {
        UINT8  ucEnableEnvironmentPoll;
        UINT8  ucCheckFanLock;
        UINT8  ucCheckOvertemp;
        UINT8  ucInstalledFans;
    }Environment;

    struct
    {
        UINT16  uiSplashAtStartupTimeout;
    }Datapath;

    struct
    {
        UINT16 uiCooldownDelay;
    }Illum;

    struct
    {
        UINT8 ucNetworkIsReady;             //if ready, network start to sync data with system.
        UINT8 ucNetworkIsReceivePowerOnCmd; //when TRUE, means network has already recevie power on command from system.
    }Network; //A70LV_Larry_0135

}sSYSTEM_CONFIGURATION, *PsSYSTEM_CONFIGURATION;


typedef struct
{
    UINT16  uiMODEL_INDEX;
    UINT32  udBACKUP_RESTORE_CHECKING;
    UINT8   ucSCALER_VERSION[32];
    UINT8   ucLAN_VERSION[32];
    UINT8   ucFORMATER_VERSION[32];
    UINT8   ucFRONTEND_VERSION[32];
}sBACKUP_RESTORE_VERSION,*PsBACKUP_RESTORE_VERSION;         //A70LV_Doulas_0279

typedef struct
{
    UINT16  udDataCode;
    UINT8   ucSize;
    union
    {
        UINT32 DWord;
        UINT16 Words[2];
        UINT8  Bytes[4];
    } DataValue;
}sBACKUP_RESTORE_ITEM,*PsBACKUP_RESTORE_ITEM;         //A70LV_Doulas_0279

typedef struct
{
    sBACKUP_RESTORE_VERSION sRestoreVersion;
    sBACKUP_RESTORE_ITEM    sRestoreItem[BACK_RESTORE_ITEM_MAX];
    UINT8                   Retention[BACK_RESTORE_RETENTION];      //A70LV_Doulas_0280 modify
}sBACKUP_RESTORE,*PsBACKUP_RESTORE;         //A70LV_Doulas_0279 total 2048 bytes


typedef struct
{
    UINT32 ulItemIndex;
    UINT16 uiStartPositionX;
    UINT16 uiStartPositionY;
    UINT16 uiParentID;
    UINT16 uiNextID;
    UINT8  ucLayoutStyle;
    UINT8  ucOperateStyle;
    UINT16 uiDataCode;
    UINT8  ucControl;
}sITEM_INFO;

typedef struct
{
    UINT16 uiMenuIndex;
    UINT16 uiStartPositionX;
    UINT16 uiStartPositionY;
    UINT16 uiHeight;
    UINT16 uiWidth;
    UINT8  ucMenuCount;
    UINT16 uiItemStart;
    UINT16 uiItemEnd;
}sMENU_INFO;


#pragma pack(pop)                          // restore previous alignment


#ifdef __cplusplus
}
#endif






#endif  //_UTILSTORAGECFG_H_

