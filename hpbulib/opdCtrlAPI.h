#ifndef OPDCTRLAPI_H
#define OPDCTRLAPI_H
//=================================================================================================
#include "type_def.h"
#include "hicc_config.h"
#include "SharedMemCtrl.h"
#include "CommonDef.h"
//=================================================================================================
#define OPD_VER_MAJOR   (0)
#define OPD_VER_MINOR   (9)
//=================================================================================================

#define USB_DEV_PATH                            "/run/media/sda1"

#define SYSLOG_LOG_PATH                         "/mnt/syslog/"
#define ERROR_LOG_PATH                          "/mnt/syslog/errorlog"
#define OPD_VERSION_TXT                         "/mnt/syslog/OpdVerion.txt"
#define ERROR_LOG_TXT                           "/mnt/syslog/errorlog/ErrorLog.txt"

#define MODEL_CHANGE_LOG_PATH                   "/mnt/syslog/modelchange"
#define MODEL_CHANGE_LOG_TXT                    "/mnt/syslog/modelchange/ModelChange.txt"
#define MODEL_CHANGE_LOG_TXT_RESERVE            "/mnt/syslog/modelchange/ModelChangeReserve.txt"

#define LENS_TYPE_LOG_PATH                    	"/mnt/syslog/lenstype"
#define LENS_TYPE_LOG_TXT                    	"/mnt/syslog/lenstype/lenstype.txt"
#define LENS_TYPE_LOG_TXT_RESERVE            	"/mnt/syslog/lenstype/lenstypeReserve.txt"

#define OPD_LOG_PATH                            "/mnt/syslog/opd"
#define OPD_TAR_PATH                            "/mnt/syslog/opd/tar"

#define OPD_PAD_SWAP_PATH                       "/home/root/swap"
#define OPD_PAD_GEOMETRY_PATH                   "/mnt/configs/Geometry"
#define OPD_PAD_SWAP_FILE_NAME                  "swap.tar"
#define OPD_PAD_GEOMETRY_FILE_NAME              "geometry.tar.gz"

#define SST_INFO_PATH                           "/mnt/syslog/projectorinfo"
#define SST_INFO_TXT                            "/mnt/syslog/projectorinfo/ProjectorInfo.txt"

#define UPGRADE_LOG_PATH                        "/mnt/syslog/upgrade"
#define IMXAP_LOG_PATH                          "/mnt/syslog/imxap"
#define BIST_FILE_PATH                          "/mnt/syslog/BIST"

#define SYSTEM_STORE_PATH                       "/mnt/configs/scaler/system"
#define GUI_STORE_PATH                          "/mnt/configs/scaler/gui"
#define SOURCE_STORE_PATH                       "/mnt/configs/scaler/source"
#define WAP_STORE_PATH                          "/mnt/configs/scaler/WAP"
#define COLOR_TABLE_PATH                        "/usr/configs/scaler/ColorTable"
#define COLOR_TABLE_FACTORY_PATH                "/mnt/configs/scaler/ColorTable"
#define NETWORK_SETUP_PATH                      "/mnt/configs"
#define NETWORK_WEB_ACCOUNT_PATH                "/mnt/configs/account"
#define NETWORK_WEB_PATH                        "/mnt/configs/web"
#define NETWORK_WEB_LOG_PATH                    "/mnt/configs/web/Log"
#define PROJECT_SETTINGS_PATH                   "/mnt/configs/ProjectSettings"

#define ADV_WAPR_PATH                           "/mnt/configs/Warp"

#define DOWNLOAD_OPD_STATUS                     "/tmp/download_opd.status"


#define OPD_COPY_2_USB                          (1)
#define OPD_COPY_2_WEB                          (2)

#pragma pack(push)  /* push current alignment to stack */
#pragma pack(1)     /* set alignment to 1 byte boundary */

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
}sOPD_VIDEO_FORMAT;

//G100_Steven_0059 start
typedef struct {
	UINT32	u32VideoPCLK;       // Pixel Clock
	UINT16	u16VideoHTotal;     // H Total
	UINT16	u16VideoHActive;    // H Active Video
	UINT16	u16VideoVTotal;     // V Total
	UINT16	u16VideoVActive;    // V Active Video
	UINT16  u16VideoVRate;      // V Rate (Hz)
	UINT8 	cPadding[2];
}sOPD_VIDEO_TIMING;
//G100_Steven_0059 end

typedef union
{
    struct
    {
    	char cModelName[32];
		char cSN[32];
		char cLanMacAdd[32];
		UINT16 wTPSec;
		UINT16 wTPMin;
		UINT16 wTPHour;
		UINT16 wTPDay;
		UINT8  cPictureMode[eCM_PICTURE_SETTINGS_NUMBER];
		UINT8  cInputSource[2];
		char   cSourceName[2][32];

    }sRegulatoryInfo;

    struct
    {
        UINT32 dwGECCode;
        char   cString[128];
    }sErrorLog;

    struct
    {
        UINT32 ulLDMinute;
        UINT16 wFanSpeed[24];
        UINT8  cFanPWM[24];
        INT16  wTemperature[8]; //G100:[0]=DMD,[1]=FMT_BD,[2]=System,
        UINT16 wLDVoltage[24];
        UINT16 wLDCurrent[24];
        INT16  wLDTemp[24];
        UINT16 wTECVoltage[3];
        UINT16 wTECCurrent[3];

		UINT16 wPumpSpeed[2];
		UINT8  cPumpPWM[2];
		UINT16 wDWSpeed;
		UINT16 wPWSpeed;

	    UINT16 wLightSenosr[7];		//A65_OPTOMA_Doulas_0105 Modify
		UINT16 wPressSenosr[3];
		UINT16 wHumiditySenosr[3];
		UINT16 wAbsHumiditySenosr[3];
		INT16  iGsensor[3];
		INT8   Voltage[8];
        UINT8  ucPowerMode;        //light source mode
        UINT8  ucHighAltitude;
        UINT8  ucShutter;
        UINT16 m_uiLDPWM[6];
		UINT8  ucPowerLevel;
		UINT8  ucPictureMode;
		UINT8  ucUserPictureMode;
		UINT8  ucDMDAirtight;
		UINT32 ulCLAError0;
		UINT32 ulCLAError1;
	    UINT32 ulCLAGain0;
		UINT32 ulCLAGain1;
		UINT32 ulCLAZData0;
		UINT32 ulCLAZData1;
        UINT8  ucDimPowerFlag;
        UINT8  ucDimPowerLevel;
    	UINT16 wBurnInCycle;

        INT32  lLVPS_InputVoltage;
        INT32  lLVPS_PFC_OutputVoltage;
        INT32  lLVPS_5V_Voltage;
        INT32  lLVPS_12V_Voltage;
        INT32  lLVPS_52V_Voltage;
        INT32  lLVPS_5V_Current;
        INT32  lLVPS_12V_Current;
        INT32  lLVPS_52V_Current;
        INT32  lLVPS_PFC_MOS_Temperature;
        INT32  lLVPS_Bridge_Temperature;
        INT32  lLVPS_Ambient_Temperature;
        INT32  lLVPS_12V_SR_MOSFET_Temperature;
        INT32  lLVPS_52V_SR_MOSFET_Temperature;
        UINT16 uiLVPS_PRIMARY_PROTECTION_STATUS;
        UINT16 uiLVPS_SECONDARY_PROTECTION_STATUS;
        UINT32 ulHDBT_Status[11];
        UINT32 ulINFO_Frame_Change;
        UINT32 ulSource_Lost;
        UINT32 ulDebug;

    }sSnapshotLog;

    struct
    {
		UINT16 wSensorFull_BLD_Y;
		UINT16 wSensorFull_BLD_R;
		UINT16 wSensorFull_BLD_B;
		UINT16 wSensorFull_BLD_G;
		UINT16 wSensorEco_BLD_Y;
		UINT16 wSensorEco_BLD_R;
		UINT16 wSensorEco_BLD_B;
		UINT16 wSensorEco_BLD_G;
		UINT16 wPWMFull_BLD_Y;
		UINT16 wPWMFull_BLD_R;
		UINT16 wPWMFull_BLD_B;
		UINT16 wPWMFull_BLD_G;
		UINT16 wPWMEco_BLD_Y;
		UINT16 wPWMEco_BLD_R;
		UINT16 wPWMEco_BLD_B;
		UINT16 wPWMEco_BLD_G;
		UINT16 wSensorFull_RLD_Y;
		UINT16 wSensorFull_RLD_R;
		UINT16 wSensorFull_RLD_B;
		UINT16 wSensorFull_RLD_G;
		UINT16 wSensorEco_RLD_Y;
		UINT16 wSensorEco_RLD_R;
		UINT16 wSensorEco_RLD_B;
		UINT16 wSensorEco_RLD_G;
		UINT16 wPWMFull_RLD_Y;
		UINT16 wPWMFull_RLD_R;
		UINT16 wPWMFull_RLD_B;
		UINT16 wPWMFull_RLD_G;
		UINT16 wPWMEco_RLD_Y;
		UINT16 wPWMEco_RLD_R;
		UINT16 wPWMEco_RLD_B;
		UINT16 wPWMEco_RLD_G;

		UINT16 wSensorFull_Dynamic_RLD_Y;
		UINT16 wSensorFull_Dynamic_RLD_R;
		UINT16 wSensorFull_Dynamic_RLD_B;
		UINT16 wSensorFull_Dynamic_RLD_G;
		UINT16 wSensorEco_Dynamic_RLD_Y;
		UINT16 wSensorEco_Dynamic_RLD_R;
		UINT16 wSensorEco_Dynamic_RLD_B;
		UINT16 wSensorEco_Dynamic_RLD_G;
		UINT16 wFull_PWM_Dynamic_RLD_Y;
		UINT16 wFull_PWM_Dynamic_RLD_R;
		UINT16 wFull_PWM_Dynamic_RLD_B;
		UINT16 wFull_PWM_Dynamic_RLD_G;
		UINT16 wEco_PWM_Dynamic_RLD_Y;
		UINT16 wEco_PWM_Dynamic_RLD_R;
		UINT16 wEco_PWM_Dynamic_RLD_B;
		UINT16 wEco_PWM_Dynamic_RLD_G;

		UINT16  uiRGBGain_R;
		UINT16  uiRGBGain_G;
		UINT16  uiRGBGain_B;
		UINT16  uiRGBOffset_R;
		UINT16  uiRGBOffset_G;
		UINT16  uiRGBOffset_B;
		UINT16  uiYUVGain_Y;
		UINT16  uiYUVGain_U;
		UINT16  uiYUVGain_V;
		UINT16  uiYUVOffset_Y;
		UINT16  uiYUVOffset_U;
		UINT16  uiYUVOffset_V;
		UINT16  uiRGBGain2_R;
		UINT16  uiRGBGain2_G;
		UINT16  uiRGBGain2_B;
		UINT16  uiRGBOffset2_R;
		UINT16  uiRGBOffset2_G;
		UINT16  uiRGBOffset2_B;
		UINT16  uiYUVGain2_Y;
		UINT16  uiYUVGain2_U;
		UINT16  uiYUVGain2_V;
		UINT16  uiYUVOffset2_Y;
		UINT16  uiYUVOffset2_U;
		UINT16  uiYUVOffset2_V;

		UINT8  cLightSensorTime_Y;
		UINT8  cLightSensorTime_R;
		UINT8  cLightSensorTime_B;
		UINT8  cLightSensorTime_G;
		UINT16 wLightSensorOffset_Y;
		UINT16 wLightSensorOffset_R;
		UINT16 wLightSensorOffset_B;
		UINT16 wLightSensorOffset_G;
		float  fLightSensorGain_Y;
		float  fLightSensorGain_R;
		float  fLightSensorGain_B;
		float  fLightSensorGain_G;
		UINT16 wLightSensorTarget_Y;
		UINT16 wLightSensorTarget_R;
		UINT16 wLightSensorTarget_B;
		UINT16 wLightSensorTarget_G;
		UINT16 wLightSensorTargetRLD_Y;
		UINT16 wLightSensorTargetRLD_R;
		//UINT16 wLightSensorTargetRLD_B; //unused.
		//UINT16 wLightSensorTargetRLD_G; //unused.

		char   cLightSensorCalibStatus[128];

    }sProjectorLog;

    struct
    {
        char    cDevInterface[32];
        char    cDevName[32];
        char    cDevInfo[32];
        UINT32  cPara1; //retry count
        UINT32  cPara2; //error count
        UINT32  cPara3; //i2c status
        UINT32  cPara4; //NULL
        FLOAT   fPara5; //error rate
    }sDev;

    struct
    {
        char   acDataCodeName[64];
        char   acString[256];//[128];		//G100_Doulas_0054 Modify//G100_Doulas_0052 Modify
    }sDataCode;

    struct
    {
        char   cInput[8];
        char   cSourceName[24];
        UINT16 wActiveH;
        UINT16 wActiveV;
        UINT16 wTotalH;
        UINT16 wTotalV;
        UINT16 wStartH;
        UINT16 wStartV;
        char   cPixelClock[32];
        char   cColorSpace[32];
        UINT8  cDeepColor;
    }sSource;

	struct
	{
        char    cDevName[32];
        UINT32  ulRetry; //retry count
        UINT32  ulError; //error count
        UINT32  ulTotal; //total count
        char    cDriveType[16];
	}sRunTime;
//G100_Steven_0059 start
	struct
	{
	//Need to sync with FE cmd: eFE_MSG_OPD_INFO.
	//===========================================
		UINT8  cMainSrc;
		UINT8  cSubSrc;
		UINT8  cPIPEn;
		UINT8  cMainCH;
		UINT8  cSubCH;
		UINT8  cVideoReady;
		UINT8  cBackupInpEn;
		UINT8  cBackupInpAc;
		UINT8  cFPGASW;
		UINT8  c6634RX;
		UINT8  c6634TX;
		UINT8  c6805_0_St;
		UINT8  c6805_1_St;
		UINT8  c6634RstCnt;
		UINT8  c6805RstCnt;
		UINT8  c7604_St;
		UINT8  cCH1_VideoReady;
		UINT8  cCH2_VideoReady;
		UINT8  cPadding[2];
		sOPD_VIDEO_FORMAT   sCH1_VideoFormat;
		sOPD_VIDEO_FORMAT   sCH2_VideoFormat;
		sOPD_VIDEO_TIMING   sCH1_VideoTiming;
		sOPD_VIDEO_TIMING   sCH2_VideoTiming;
	//===========================================
	}sFE_Info; //G100_Steven_0059 end

    struct
    {
		UINT8 cFMCU[16];
		UINT8 cSMCU[16];
		UINT8 cFRONTEND[16];
		UINT8 cFORMATER[24];
		UINT8 cHDBASET[16];
		UINT8 cKEYPAD[16];
		UINT8 cLD_DRIVER[16];
		UINT8 cFIRMWARE[24];
		UINT8 cMEMC[16];
		UINT8 cMOTOR[16];
		UINT8 cLAN[16];
		UINT8 cFPGA1[16];
		UINT8 cFPGA2[16];
		UINT8 cFPGA3[16];
		UINT8 cPMCU[16];
		UINT8 cXFPGA[16];
		UINT8 cRELEASE[16];
		UINT8 c3GSDI[16];
		UINT8 cGEOMETRY[16];
		UINT8 cCamera[16];
		UINT8 cFPGAID[16];

    }sVERSION;

    struct
    {
        char    cOPD_Type[16];
        char    cIC_Name[32];
        char    cResult[8];
        UINT32  ulRetry; //retry count
        UINT32  ulError; //error count
        UINT8   ucI2C_St; //I2C status
        UINT32  ulTotal; //total count
        UINT8   ucErrorRate;
        char    cString[256];

    }sINTERFACE;

    struct
    {
        UINT8   cRxPort;
        UINT8   cAVIInfo[14];
        UINT8   cVSII[8];
        UINT8   cHDRInfo[16];
    }sAVI_Info;

    struct
    {
        UINT32 wLensHPosition;
        UINT32 wLensVPosition;
        UINT16 wZoomPosition;
        UINT16 wFocusPosition;
        UINT8  cDirH;
        UINT8  cDirV;
        UINT8  cDirZoom;
        UINT8  cDirFocus;
        UINT16 wZoomMaximumValue;
        UINT16 wZoomMinimumValue;
        UINT16 wFocusMaximumValue;
        UINT16 wFocusMinimumValue;
        UINT32 ulActiveMaxValueH;
        UINT32 ulActiveMinValueH;
        UINT32 ulActiveMaxValueV;
        UINT32 ulActiveMinValueV;
        UINT16 wBacklashH;
        UINT16 wBacklashV;
    }sLensCalibrationInfo;

    struct
    {
        UINT8  cIndex;
        UINT32 wLensHPosition;
        UINT32 wLensVPosition;
        UINT8  cDirH;
        UINT8  cDirV;
        UINT16 wZoomPosition;
        UINT16 wFocusPosition;
        UINT8  cDirZoom;
        UINT8  cDirFocus;
    }sLensMemInfo;

    struct
    {
        UINT8 ucLensCenterSetting;
        UINT32 wLensHPosition;
        UINT32 wLensVPosition;
        UINT8 cDirH;
        UINT8 cDirV;
    }sLensOpticalCenter;

    char cString[256];
	UINT16 dCount;
	UINT16 awData[32];

    UINT32 ulMCU_SYSTEM_STATE;

}uOPD_DATA;

typedef enum
{
/* 00 */    eOPD_ERROR_LOG,
/* 01 */    eOPD_WARNING_LOG,
/* 02 */    eOPD_SNAPSHOT_LOG,
/* 03 */    eOPD_ENGINE_LOG,
/* 04 */    eOPD_PROJECTOR_LOG,
/* 05 */    eOPD_DEVICE_LOG,
/* 06 */    eOPD_KEY_KEYPAD_LOG,
/* 07 */    eOPD_KEY_IR_LOG,
/* 08 */    eOPD_KEY_LAN_LOG,
/* 09 */    eOPD_KEY_HDBaseT_LOG,
/* 10 */    eOPD_KEY_CLI_LOG,
/* 11 */    eOPD_CLI_RS232_LOG,
/* 12 */    eOPD_CLI_TELNET_LOG,
/* 13 */    eOPD_CLI_HDBASET_LOG,
/* 14 */    eOPD_TELNET_LOG,
/* 15 */    eOPD_ACCESS_DATA_GUI,
/* 16 */    eOPD_ACCESS_DATA_LAN,
/* 17 */    eOPD_SOURCE_LOG,
/* 18 */    eOPD_AVI_INFOFRAME_LOG,
/* 19 */    eOPD_FrontEnd_LOG, //eOPD_HDMI_VENDOR_LOG, //G100_Steven_0059
/* 20 */    eOPD_RUNTIME_LOG,
/* 21 */    eOPD_SYSTEM_STATUS_LOG,
/* 22 */    eOPD_FW_VERSION_LOG,
/* 23 */    eOPD_INTERFACE_LOG,
/* 24 */    eOPD_POWER_ON_LOG, //G100_Steven_0089
/* 25 */    eOPD_POWER_OFF_LOG,
/* 26 */	eOPD_POWER_STARTUP_LOG,
/* 27 */	eOPD_SHUTTER_ON_LOG,
/* 28 */	eOPD_SHUTTER_OFF_LOG,
/* 29 */	eOPD_HIGH_ALTITUDE_OFF,
/* 30 */	eOPD_HIGH_ALTITUDE_ON,
/* 31 */	eOPD_INPUT_DETECT_LOG,
/* 32 */	eOPD_MUTEX_LOCKED_LOG,
/* 33 */	eOPD_TASK_NO_RESPONSE_LOG,
/* 34 */	eOPD_ACCESS_DATACODE_LOG,
/* 35 */    eOPD_HW_CHECK_LOG,
/* 36 */	eOPD_VOLTAGE_DETECT,
/* 37 */	eOPD_PICTURE_MODE_LOG,
/* 38 */	eOPD_LIGHT_SOURCE_LOG,
/* 39 */	eOPD_LIGHT_SENSOR_CALIBRATION,
/* 40 */	eOPD_CELING_MOUNT_OFF_LOG,
/* 41 */	eOPD_CELING_MOUNT_ON_LOG,
/* 42 */	eOPD_CELING_MOUNT_AUTO_LOG,
/* 43 */    eOPD_KEY_WIDE_LOG,
/* 44 */    eOPD_GEOMETRY_LOG,
/* 45 */    eOPD_KEY_BACK_LOG,
/* 46 */    eOPD_LENS_CALIBRATION_LOG,
/* 47 */    eOPD_LENS_MEMORY_LOG,
/* 48 */    eOPD_LENS_OPTICA_CENTER_LOG,
/* 49 */    eOPD_LIGHT_SENSOR_ERROR_LOG,
/* 50 */    eOPD_MCU_SYSTEM_STATE_LOG,
/* 51 */    eOPD_HDBT_STATUS_LOG,
/* 52 */    eOPD_INFOFRAME_CHANGE_LOG,
/* 53 */    eOPD_SOURCE_LOST_LOG,
/* 54 */    eOPD_DEBUG_LOG,

    eOPD_NUMBERS,
} eOPD_LOG_EVENT;

typedef struct
{
    char* cLog;
	char* cEngine;
	char* cProjector;
	char* cSelf_Test;
	char* cI2C_RunTime;
	char* cUpgrade;
	char* cSource;
	char* cInterface;
	char* cBIST;

} sOPD_PASSWORD_LUT_TABLE; //G100_Julie_00021

typedef enum
{
    eCUSTOMER_SPLASH_BARCORE,


    eCUSTOMER_SPLASH_INVALID,
}eCUSTOMER_SPLASH_TYPE; //G100_Julie_00021

typedef enum
{
    eOPD_PACKET_PROGRESS_OPD_START = 0,
    eOPD_PACKET_PROGRESS_OPD_LOG,
    eOPD_PACKET_PROGRESS_OPD_SELF_TEST,
    eOPD_PACKET_PROGRESS_OPD_ENGINE,
    eOPD_PACKET_PROGRESS_OPD_PROJECTOR,
    eOPD_PACKET_PROGRESS_OPD_RUNTIME,
    eOPD_PACKET_PROGRESS_OPD_SOURCE,
    eOPD_PACKET_PROGRESS_OPD_INTERFACE,
    eOPD_PACKET_PROGRESS_UPGRADE,
    eOPD_PACKET_PROGRESS_IMXAP,
    eOPD_PACKET_PROGRESS_SYSTEMINFO,
    eOPD_PACKET_PROGRESS_GUISTORE,
    eOPD_PACKET_PROGRESS_SOURCESTORE,
    eOPD_PACKET_PROGRESS_WAP_STORE,
	eOPD_PACKET_PROGRESS_BIST,
	eOPD_PACKET_PROGRESS_COLORTABLE,
    eOPD_PACKET_PROGRESS_COLORTABLE_FACTORY,
    eOPD_PACKET_PROGRESS_NETWORK_SETUP,
    eOPD_PACKET_PROGRESS_WEB_ACCOUNT,
    eOPD_PACKET_PROGRESS_WEB_LOG,
    eOPD_PACKET_PROGRESS_ADV_WARP_LOG,
    eOPD_PACKET_PROGRESS_PROJECT_SETTING,

    eOPD_PACKET_PROGRESS_MAX,
}eOPD_PACKET_PROGRESS;

typedef enum
{
    eOPD_PACKET_STATUS_IDLE = 0,
    eOPD_PACKET_STATUS_PROGRESS,
    eOPD_PACKET_STATUS_TAR,
    eOPD_PACKET_STATUS_DONE,
    eOPD_PACKET_STATUS_FINISH,
    eOPD_PACKET_STATUS_ERROR,

    eOPD_PACKET_PROGRESS_STATUS_INVALID,
}eOPD_PACKET_PROGRESS_STATUS;

#pragma pack(pop)                          // restore previous alignment

int utilOPD_Write2File(void);
int utilOPD_Write2TestFile(void);
int utilOPD_Write2RunTimeFile(void);
int utilOPD_Download_File(UINT32 dSelect); //G100_Julie_0015
int utilOPD_EventSet(UINT8 ucEvent, uOPD_DATA *puData);

void utilOPD_Initialization(void);
int utilOPD_CLI_EventSet(UINT8 ucEvent, char *pcString);
void utilOPD_ResetAllToDefault(void); //G100_Julie_0024
int utilOPD_CopyToUsb(void);
void utilOPD_PackageTarFile(UINT8 ucSplash, UINT8 ucData); //G100_Julie_0027
void utilOPD_SetSourceName(UINT8 cMain, UINT8 cSub); //G100_Steven_0059
int utilOPD_Write_ModelChangeLog(const char *pcString, const char *pcString2); //A65_OPTOMA_Julie_0066
int utilOPD_Write_LensTypeLog(int value1, int value2, int value3);
void utilOPD_RegulatoryInfo_Set(eDATA_CODE eDataCode, char *pStr, UINT8 cValue);
int utilOPD_TotalProjector_Set(UINT32 ulTotalProjectorSec);
int utilOPD_UartTotalCount_Set(UINT16 uiCount);

int utilOPD_OPDFile_ReplaceSN(char *pcString);

#endif //OPDCTRLAPI_H
