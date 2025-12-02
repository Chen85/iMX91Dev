#ifndef _DVPROAV_H_
#define _DVPROAV_H_


#ifdef __cplusplus
extern "C" {
#endif


#include "Common.h"
#include "utilCommon.h"
#include "utilStorageCfg.h"


#define PROAV_PASS 1
#define ERROR_PROAV_DATA_OUT_OF_RANGE -2

#define CHANNEL_MAX                     4
#define DEF_TIMEOUT                     100
#define DEF_WAIT_TIMEOUT                100//600// Time out(sec) => wait_pivs, wait_povs, wait_pstat //A70LV_Doulas_0003 test

#define PROAV_BRIGHTNESS_MAX        1023
#define PROAV_BRIGHTNESS_DEFAULT    512
#define PROAV_BRIGHTNESS_MIN        0
#define PROAV_CONTRAST_MAX          1023
#define PROAV_CONTRAST_DEFAULT      512
#define PROAV_CONTRAST_MIN          0

// ProAV_Rex_0014 start
#define PROAV_INPUT_BIAS_R_MAX 255
#define PROAV_INPUT_BIAS_R_DEFAULT 128
#define PROAV_INPUT_BIAS_R_MIN 0

#define PROAV_INPUT_BIAS_G_MAX 255
#define PROAV_INPUT_BIAS_G_DEFAULT 128
#define PROAV_INPUT_BIAS_G_MIN 0

#define PROAV_INPUT_BIAS_B_MAX 255
#define PROAV_INPUT_BIAS_B_DEFAULT 128
#define PROAV_INPUT_BIAS_B_MIN 0

#define PROAV_INPUT_GAIN_R_MAX 255
#define PROAV_INPUT_GAIN_R_DEFAULT 64
#define PROAV_INPUT_GAIN_R_MIN 0

#define PROAV_INPUT_GAIN_G_MAX 255
#define PROAV_INPUT_GAIN_G_DEFAULT 64
#define PROAV_INPUT_GAIN_G_MIN 0

#define PROAV_INPUT_GAIN_B_MAX 255
#define PROAV_INPUT_GAIN_B_DEFAULT 64
#define PROAV_INPUT_GAIN_B_MIN 0
// ProAV_Rex_0014 End

//CHECK INPUT
#define DEF_PI_ACTHST_ERR_RANGE         2
#define DEF_PI_ACTHW_ERR_RANGE          2
#define DEF_PI_ACTVST_ERR_RANGE         2
#define DEF_PI_ACTVW_ERR_RANGE          2
#define DEF_PI_HCYCL_ERR_RANGE          5
#define DEF_PI_VCYCL_ERR_RANGE          5
#define DEF_PI_VYNC_FREQ_ERR_RANGE      0.2 //A70LV_Doulas_0078

#define DEF_MWI_4K            	        0x22

///// SDRAM
#define DEF_ISFLD0_4K		            0x00000000
#define DEF_ISFLD1_4K		            0x03000000//0x021E8000// = 4096 * 4 * 2170
#define DEF_ISFLD2_4K	                0x06000000//0x043D0000// = 4096 * 4 * 2170 * 2
#define DEF_ISFLD3_4K  		            0x09000000//0x065B8000// = 4096 * 4 * 2170 * 3

//Roger : Refer to C821_Basic_Funciton_v10.pdf , Page 23
//TODO : Address might need to be caculated dynamically
#define DEF_IPISFLD0CH1_4K		        0x087A0A00// = IPISFLD1 + IPMWI[4:0] x 1024 / 2 = 0x08800000 + 5 x 1024/2(0xA00) = 0x08800A00
#define DEF_IPISFLD1CH1_4K		        0x087A0000// = ISFD3/OSFLD3 End Address = 4096 * 4 * 2165 * 4
#define DEF_IPISFLD2CH1_4K	            0x08A46C00// = IPISFLD3 + IPMWI[4:0] x 1024 / 2 = 0x08AA6200 + 5 x 1024/2(0xA00) = 0x08AA6C00
#define DEF_IPISFLD3CH1_4K  	        0x08A46200// = IPISFLD1 + Memory capacity of De-interlacer(5x1024x(1080+5)/2)(0x2A6200) = 0x08AA6200

#define DEF_IPOSFLD0CH1_4K	    	    DEF_IPISFLD0CH1_4K
#define DEF_IPOSFLD1CH1_4K		        DEF_IPISFLD1CH1_4K
#define DEF_IPOSFLD2CH1_4K		        DEF_IPISFLD2CH1_4K
#define DEF_IPOSFLD3CH1_4K              DEF_IPISFLD3CH1_4K

#define DEF_IPMWICH1_4K	                0x45
#define DEF_IPYOFFCH1_4K   	            0x054D
#define DEF_MVFLDCH1_4K		            0x0923A000

#define DEF_IPISFLD0CH2_4K 		        0x09349E00
#define DEF_IPISFLD1CH2_4K		        0x09349400
#define DEF_IPISFLD2CH2_4K 		        0x095F0000
#define DEF_IPISFLD3CH2_4K 		        0x095EF600

#define DEF_IPOSFLD0CH2_4K 		        DEF_IPISFLD0CH2_4K
#define DEF_IPOSFLD1CH2_4K 		        DEF_IPISFLD1CH2_4K
#define DEF_IPOSFLD2CH2_4K 		        DEF_IPISFLD2CH2_4K
#define DEF_IPOSFLD3CH2_4K 		        DEF_IPISFLD3CH2_4K

#define DEF_IPMWICH2_4K	    	        0x45
#define DEF_IPYOFFCH2_4K   		        0x054D
#define DEF_MVFLDCH2_4K          	    0x09DE3400

#define DEF_IPISFLD0CH3_4K 		        0x09EF3200
#define DEF_IPISFLD1CH3_4K		        0x09EF2800
#define DEF_IPISFLD2CH3_4K 		        0x0A199400
#define DEF_IPISFLD3CH3_4K 		        0x0A198A00

#define DEF_IPOSFLD0CH3_4K 		        DEF_IPISFLD0CH3_4K
#define DEF_IPOSFLD1CH3_4K 		        DEF_IPISFLD1CH3_4K
#define DEF_IPOSFLD2CH3_4K 		        DEF_IPISFLD2CH3_4K
#define DEF_IPOSFLD3CH3_4K 		        DEF_IPISFLD3CH3_4K

#define DEF_IPMWICH3_4K	    	        0x45
#define DEF_IPYOFFCH3_4K   		        0x054D
#define DEF_MVFLDCH3_4K     		    0x0A98C800

#define DEF_IPISFLD0CH4_4K 		        0x0AA9C600
#define DEF_IPISFLD1CH4_4K 		        0x0AA9BC00
#define DEF_IPISFLD2CH4_4K 		        0x0AD42800
#define DEF_IPISFLD3CH4_4K 		        0x0AD41E00

#define DEF_IPOSFLD0CH4_4K 		        DEF_IPISFLD0CH4_4K
#define DEF_IPOSFLD1CH4_4K 		        DEF_IPISFLD1CH4_4K
#define DEF_IPOSFLD2CH4_4K 		        DEF_IPISFLD2CH4_4K
#define DEF_IPOSFLD3CH4_4K 		        DEF_IPISFLD3CH4_4K

#define DEF_IPMWICH4_4K                 0x45
#define DEF_IPYOFFCH4_4K   		        0x054D
#define DEF_MVFLDCH4_4K		            0x0B535C00

#define PROAV_HUE_MAX                180     //A70LV_Doulas_0012
#define PROAV_HUE_DEFAULT            0
#define PROAV_HUE_MIN                -180
#define PROAV_SATURATION_MAX         2.1//3.99     //A70LV_Doulas_0052 modify
#define PROAV_SATURATION_DEFAULT     1
#define PROAV_SATURATION_MIN         0

// OSD
#define DEF_OSDSAD_LAYER0_4K            0x0B650000  //4096x2160 = 0x870000
#define DEF_OSDSAD_LAYER1_4K            0x0BEC0000  //4096x2160 = 0x870000

#if 0
#define DEF_OSDSAD_CH1_4K               0x0C730000  //1024x2160 = 0x21C000
#define DEF_OSDSAD_CH2_4K               0x0C94C000  //1024x2160 = 0x21C000
#define DEF_OSDSAD_CH3_4K               0x0CB68000  //1024x2160 = 0x21C000
#define DEF_OSDSAD_CH4_4K               0x0CD84000  //1024x2160 = 0x21C000
#endif

#define DEF_BITMAPAD_4K                 0x0CFA0000  //for Bitmap used size=4096x2160 0x870000
#define DEF_FONTAD_4K                   0x0D810000
#define DEF_OSDMWI_4K                   0x20
#define DEF_MPROTECT_4K                 0x8B63

// Serial Flash
#define DEF_SFL_OSD_FSAD_4K             0x00000000  //Font start address
#define DEF_SFL_OSD_PSAD_4K             0x00200000  //Bitmap start address

// Serial Flash //A70LV_Larry_0001
//#define DEF_SFL_OSD_FSAD_2K             (TEXT_RAW_DATA_ADDR - EXTERNAL_FLASH_BASEED)     //Font start address
//#define DEF_SFL_OSD_PSAD_2K             (BITMAP_RAW_DATA_ADDR - EXTERNAL_FLASH_BASEED)   //Bitmap start address

//RTCT status
#define		RTCT_THRU				(0x000000)					// 0000_0000_0000_0000 (All Transfer THRU)
#define		RTCT_STOP				(0x088888)					// 1000_1000_1000_1000_1000 (All Transfer STOP)
#ifdef C821_WARPING_ENABLE      //A70LV_Doulas_0099 Modify
#define		RTCT_NORMAL				(0x092165)					// 0010_0001_0101_0101 (WPOVS_PI2VS_PI1VS_PO2VS_PO1VS)
#else
#define		RTCT_NORMAL				(0x002165)					// 0010_0001_0101_0101 (PI2VS_PI1VS_PO2VS_PO1VS)
#endif
#define		RTCT_POVSSTOP			(0x082188)					// 0010_0001_0100_0100 (WPOVS_PI2VS_PI1VS_STOP__STOP_)
#define		RTCT_PIVSTHRU			(0x090065)                  // ICH1/ICH2 Transfer THRU
#define		RTCT_PO1VSGO			(0x052155)
#define		RTCT_WARP			    (0x092159)

//======================== 2K ========================================

#define DEF_MWI_2K	               	    0x20//0x11

#define HFREQ_TOLERANCE     120//100     //A70LV_Doulas_0195 modify
#define VTOTAL_TOLERANCE    2       // in pixels    //A70LV_Doulas_0009

#define IMFH_MCLK_TOLERANCE    5     //A70LV_Doulas_0009
#define IMFV_MCLK_TOLERANCE    100//3     //A70LV_Doulas_0154 modify //A70LV_Doulas_0009

#define HORZ_POSITION_LIMIT_MIN         4   //A70LV_Doulas_0113
#define VERT_POSITION_LIMIT_MIN_PRO     4   //A70LV_Doulas_0113
#define VERT_POSITION_LIMIT_MIN_INT     8   //A70LV_Doulas_0113


//for eWcuRGBMask   //A70LK_Simon_0017
#define RGB_MASK_R BIT2
#define RGB_MASK_G BIT1
#define RGB_MASK_B BIT0


//******************* Reg Define Start ************************//
// Bank=  0(0x00) FNAME=tags\SCQ3B0.tag

//======================== Reg Define End ======================//




//************************* Enum Start *************************//


/*
typedef enum
{
	ePOLARITY_NEGTIVIE,
	ePOLARITY_POSITIVE,
}ePOLARITY;
*/


typedef enum
{
    eFILL_COLOR_RED,
    eFILL_COLOR_GREEN,
    eFILL_COLOR_BLUE,
    eFILL_COLOR_YELLOW,
    eFILL_COLOR_WHITE,
    eFILL_COLOR_BLACK,
    eFILL_COLOR_CYAN,      //A70LV_Doulas_0035
    eFILL_COLOR_MAGENTA,      //A70LV_Doulas_0035
} eFILL_COLOR;



typedef enum
{
    eSCAN_MODE_PROGRESSIVE,
    eSCAN_MODE_INTERLACE,
    eSCAN_MODE_NO_SIGNAL
} eSCAN_MODE;



typedef enum
{
    ePROAV_EXEC_CODE_PASS,                   /* pass */
    ePROAV_EXEC_CODE_FAIL,                   /* general fail indication */
    ePROAV_EXEC_CODE_FATAL,                  /* fatal error; halt application */
    ePROAV_EXEC_CODE_PANEL_ID_NOT_FOUND,
    ePROAV_EXEC_CODE_PANEL_TABEL_INDEX_NOT_FOUND,
    ePROAV_EXEC_CODE_CH1_NOT_INITED,         //Need to init channel 1 before than init others
    ePROAV_EXEC_CODE_CH_OVER_RANGE,
    ePROAV_EXEC_CODE_CH_AUTO_PHASE,      //A70LV_Doulas_0007
} ePROAV_EXEC_CODE;

#if 0//A70LV_Doulas_0076 remove
typedef enum
{
    eCOLOR_FORMAT_RGB,
    eCOLOR_FORMAT_422,
    eCOLOR_FORMAT_422_UVDLY,
    eCOLOR_FORMAT_444,
    eCOLOR_FORMAT_RGB_LIMIT
} eCOLOR_FORMAT;
#endif

typedef enum
{
    eMCT_CH1,
    eMCT_CH2,
    eMCT_LAST,
} eMCT;

typedef enum
{
    ePSQ_STATE_RUN,
    ePSQ_STATE_PAUSE,
    ePSQ_STATE_RESTART,
    ePSQ_STATE_STOP,
    ePSQ_STATE_LAST,
} ePSQ_STATE;

typedef enum
{
    ePIXEL_SHIFT_NORMAL,
    ePIXEL_SHIFT_FRAME_SEQ_RL,
    ePIXEL_SHIFT_FRAME_SEQ_RRLL,
    ePIXEL_SHIFT_FRAME_SEQ_PLUS,
    ePIXEL_SHIFT_DUAL_PIPE_RL,
    ePIXEL_SHIFT_DUAL_PIPE_RRLL,
    ePIXEL_SHIFT_DUAL_PIPE_PLUS,
    ePIXEL_SHIFT_DUAL_PIPE_4K3D,
    ePIXEL_SHIFT_OFF,
    ePIXEL_SHIFT_LAST,

} ePIXEL_SHIFT;

//====================== Enum End ======================//


//===================== Struct Start ===================//
#pragma pack(push)  /* push current alignment to stack */       //A70LV_Doulas_0123
#pragma pack(1)     /* set alignment to 1 byte boundary */      //A70LV_Doulas_0123
typedef struct
{
    UINT16  uiHTotal;      //H Total
    UINT16  uiVTotal;      //V Total
    UINT16  uiHActive;     //H Active size
    UINT16  uiVActive;     //V Active size
    UINT16  uiHStart;      //H Start
    UINT16  uiVStart;      //V Start
    UINT16  uiHSyncWidth;  //H Sync Width
    UINT16  uiVSyncWidth;  //V Sync Width
    UINT16  uiHPol;        //H Polarity(not used)
    UINT16  uiVPol;        //V Active size
    UINT16  uiFrameRate;   //Frame rate(0.01Hz)
    UINT32  ulPixelClock;  //Pixel clock
    UINT8   cDeinterlaceMode;

} sOUTPUT_TIMING_INFO, *PsOUTPUT_TIMING_INFO;

typedef struct      //A70LV_Doulas_0112 modify
{
    UINT32              uiHFreq;
    DOUBLE 		        dFrameRate;
    UINT16		        uiHTotal;
    UINT16		        uiVTotal;
    UINT16		        uiHActive;
    UINT16		        uiVActive;
    UINT16		        uiHStart;
    UINT16		        uiVStart;
    UINT8               eScanMode;

} sINPUT_TIMING_INFO, *PsINPUT_TIMING_INFO;


typedef struct
{
    //eMODE_TABLE_ID            eModeTableId;
    UINT16                    uiModeTableNum;
    sINPUT_TIMING_INFO        sInputTimingInfo;
} sMODE_TABLE, *PsMODE_TABLE;   //A70LV_Doulas_0005 Add


typedef struct
{
    UINT16 uiHst;
    UINT16 uiHw;
    UINT16 uiVst;
    UINT16 uiVw;
    INT16  iHOffset;
    INT16  iVOffset;
} sACTIVE_AREA, *PsACTIVE_AREA;

typedef struct
{
    UINT32 uiBestPAL;
    UINT8  uiBestPhase;
    UINT8  uiCurrentPhase;
    UINT8  ucBestMax;   //A70LV_Doulas_0215
    UINT8  ucBestMin;   //A70LV_Doulas_0215
} sAUTO_PHASE, *PsAUTO_PHASE;     //A70LV_Doulas_0007

//Channel information is a parameter which is only on copy
typedef struct
{
    BOOL                        bInit;
    ePANEL_ID                   ePanelTimingId;           //Pass from up layer to get panel output timing
    UINT8						ucPanelIndex;             //To save panel table index
 //   UINT8						ucQuarterPanelIndex;      //4K To save quarter panel index //A70LV_Doulas_0008 remove
    sOUTPUT_TIMING_INFO         sOutputTimingInfo;
 //   sOUTPUT_TIMING_INFO         sQuarterOutputTimingInfo; //4K  //A70LV_Doulas_0008 remove
    sOUTPUT_TIMING_INFO         sScalerOutputTimingInfo;    //A70LV_Doulas_2000


    UINT32                      ulInterruptEnable;
    BOOL                        bForceSyncReset;
    BOOL                        bOutputChSwap;

 //   eSCALING_MODE               eScalingMode;     //A70LV_Doulas_0013
    sACTIVE_AREA                sIdual_Act_Ofst;//          , g_idual_act_ofst

    UINT8                       ucIPPLLCT;//                , g_ippllct
    UINT8                       ucMCT;//BN_MCTn          , g_mct
    UINT8                       ucISYCT;//B17_ISYCTCHn   , g_isyct
    UINT8                       ucOIMGCT;//B2_OIMGCTCHn  , g_oimgct
    UINT8                       ucICFMT;//B0_ICFMTCHn    , g_icfmt
    sINPUT_TIMING_INFO          sInputTimingInfo;//PIACT
    sINPUT_TIMING_INFO          sScaler_InputTimingInfo;     //A70LV_Doulas_2000
 //   eCOLOR_FORMAT               eColorFormat;     //A70LV_Doulas_0013
    UINT8                       ucInputPort;
 //   INT16                       iBrightness;    //A70LV_Doulas_0013
 //   DOUBLE                      dContrast;      //A70LV_Doulas_0013
 //   DOUBLE                      dColor;         //A70LV_Doulas_0013
 //   INT16                       uiHue;      //A70LV_Doulas_0003
 //   INT8                        ucGamma;
#if (CURRENT_MAIN_BOARD == ICHIP_BOARD)
    UINT32                      ulMclko_Freq;
#else
    UINT32                      ulMclko_Freq;
#endif
    eINPUT_SYNC_TYPE            eInputSyncType;        //A70LV_Doulas_0005
    sACTIVE_AREA                sInputAutoMeasure;     //A70LV_Doulas_0005
    sAUTO_PHASE                 sAutoPhase;            //A70LV_Doulas_0007
    BOOL                        bAutoPosition;         //A70LV_Doulas_0118
 //   eINPUT_SOURCE               ucInputSource;       //A70LV_Doulas_0013  //A70LV_Doulas_0007
 //   eOVER_SCAN_TYPE             ucOverScan;            //A70LV_Doulas_0009
 //   INT16                       iRedOffset;          //A70LV_Doulas_0011
 //   INT16                       iGreenOffset;        //A70LV_Doulas_0011
 //   INT16                       iBlueOffset;         //A70LV_Doulas_0011
 //   DOUBLE                      dRedGain;            //A70LV_Doulas_0011
 //   DOUBLE                      dGreenGain;          //A70LV_Doulas_0011
 //   DOUBLE                      dBlueGain;           //A70LV_Doulas_0011
} sCHANNEL_INFO, *PsCHANNEL_INFO;


typedef struct
{
    eCM_SOURCE_ID               eInputSource;
    eCM_SCALING_MODE_ID         eScalingMode;
    eOVER_SCAN_TYPE             eOverScan;
    //eCOLOR_FORMAT               eColorFormat;

    UINT8                       ucPhase;
    UINT8                       ucTracking;
    UINT8                       ucHoriPosition;     //A70LV_Doulas_0029
    UINT8                       ucVertPosition;     //A70LV_Doulas_0029

    UINT16                      uiDigitalHorzZoom;
    UINT16                      uiDigitalVertZoom;
    UINT8                       uiDigitalHorzShift;
    UINT8                       uiDigitalVertShift;

    UINT8                       ucSharpness;
    UINT8                       ucSkinColor;

    INT16                       iBrightness;
    DOUBLE                      dContrast;
    DOUBLE                      dColor;
    INT16                       iHue;
    INT8                        iRedOffset;
    INT8                        iGreenOffset;
    INT8                        iBlueOffset;
    UINT8                       iRedGain;
    UINT8                       iGreenGain;
    UINT8                       iBlueGain;

    INT8                        cGamma;
    UINT8                       cTemporal_NR;             //A70LV_Doulas_0029
    UINT8                       cMPEG_NR;
    UINT8                       cDetectFilm;
    UINT8                       cPIP_PBP_ON;              //A70LV_Doulas_0030
    UINT8                       cMain_Layout;
    UINT8                       cPIP_Size;
    UINT8                       cFREEZE;
    UINT8                       cTestPattern;             //A70LV_Doulas_0035
    UINT8                       eScanMode;                //A70LV_Doulas_0076
    UINT8                       eVideoYUV;      //A70LV_Doulas_0109
    UINT8                       eVGA_SYNC_TYPE;     //A70LV_Doulas_0112
    UINT8                       eOSD_3D_Enable;     //A70LV_Doulas_0154
    UINT8                       eOSD_3D_SyncOut;
    UINT8                       eOSD_3D_SyncIn;
    UINT8                       eOSD_3D_Invert;
} sCHANNEL_SETTING, *PsCHANNEL_SETTING;     //A70LV_Doulas_0013

typedef struct
{
    char *pString;
    UINT32 dwReg;
} sC821_REG;    //A70LV_Doulas_0002



#pragma pack(pop)                          // restore previous alignment    A70LV_Doulas_0123
//===================== Struct End =====================//


//#define C821REG_NUMBER sizeof(m_sc821Reg)/sizeof(sC821_REG)


BOOL dvProAV_SemaphoreTake(BOOL cEnable, const char *pcSemaphore);
int dvProAV_AccessRaw_Read(UINT8 bank, UINT8 offset, UINT16 len, UINT8 *data);
int dvProAV_AccessRaw_Write(UINT8 bank, UINT8 offset, UINT16 len, UINT8 *data);
int dvProAV_Access_Write(UINT32 regEnum, UINT32 data);
int dvProAV_Access_Read(UINT32 regEnum, UINT32 *data);

UINT32 dvPro_WaitIntct(const eMCT eCH, const UINT32 ulCt);

INT8 dvPro_Brightness_Value_Set (UINT8 ucCH,INT16 iBrightness);    //A70LV_Doulas_0022
INT16 dvPro_Brightness_Value_Get (void);
INT8 dvPro_Contrast_Value_Set (UINT8 ucCH,DOUBLE dContrast);  //A70LV_Doulas_0022
DOUBLE dvPro_Contrast_Value_Get (void);

ePROAV_EXEC_CODE dvPro_Init_Panel(const eMCT eCH, const ePANEL_ID ePanelId);
void dvPro_SetOutputColor_2K(const eMCT eCH, const eCOLOR_FORMAT eColorFmt);
void dvPro_SetInputPort_2K(const eMCT eCH);
void dvPro_SetIdual_2K(const eMCT eCH);
UINT8 dvPro_GetPanelInfo(ePANEL_ID ePanelId, UINT8 *psPanel);
int dvPro_BuildPicture_2K(const eMCT eCH);
void dvPro_SetOverlay_2K(const eMCT eCH, BOOL bIsEnable);

void dvPro_SetBrightness(const eMCT eCH);    //A70LV_Doulas_0022
void dvPro_SetMatrix(eMCT eCH, eCOLOR_FORMAT eOutformat);
BOOL dvPro_PIP_Enable(void);
BOOL dvPro_PIP_PBP_Enable(void);        //A70LV_Doulas_0226

#ifdef __cplusplus
}
#endif


void dvPro_Set_HUE(eMCT eCH);   //A70LV_Doulas_0022
INT16 dvPro_Get_HUE(eMCT eCH);              //A70LV_Doulas_0022
void dvPro_Set_Saturation(eMCT eCH);  //A70LV_Doulas_0022
DOUBLE dvPro_Get_Saturation(eMCT eCH);         //A70LV_Doulas_0003
void dvPro_Set_PIP_PBP_Layout(UINT8 uiLayout);  //A70LV_Doulas_0003
void dvPro_Set_PIP_PBP_Size(UINT8 uiSize);  //A70LV_Doulas_0003
UINT8 dvPro_Get_PIP_PBP_Layout(void);  //A70LV_Doulas_0003
UINT8 dvPro_Get_PIP_PBP_Size(void);  //A70LV_Doulas_0003

void dvPro_Resync_Init(eMCT eCH);   //A70LV_Doulas_0004
BOOL dvPro_SYNC_LOCK_Compare(const eMCT eCH);   //A70LV_Doulas_0005 modify//A70LV_Doulas_0004
BOOL dvPro_Panel_Change_Setting(eMCT eCH,const ePANEL_ID ePanelId); //A70LV_Doulas_0005
BOOL dvPro_AutoPositionSetting(const eMCT eCH);   //A70LV_Doulas_0005

UINT32 dvPro_GetAPL(const eMCT eCH,UINT8 *ucMax,UINT8 *ucMin);                   //A70LV_Doulas_0215 modify//A70LV_Doulas_0007
ePROAV_EXEC_CODE dvPro_AutoPhaseStart(const eMCT eCH);   //A70LV_Doulas_0007
ePROAV_EXEC_CODE dvPro_AutoPhase(const eMCT eCH);    //A70LV_Doulas_0007
UINT8 dvPro_CurrentPhaseGet(const eMCT eCH);       //A70LV_Doulas_0007
UINT8 dvPro_BsetPhaseGet(const eMCT eCH);         //A70LV_Doulas_0007
void dvPro_InputSourceSet(const eMCT eCH,UINT8 ucInputSource);   //A70LV_Doulas_0007

void dvPro_Set_Color(const eMCT eCH);      //A70LV_Doulas_0009
void dvPro_SetContrast(const eMCT eCH); //A70LV_Doulas_0009
void dvPro_Scale_Overscan(eOVER_SCAN_TYPE ucOverScan,PsINPUT_TIMING_INFO psScalerInput,PsOUTPUT_TIMING_INFO psScalerOutput);    //A70LV_Doulas_0009
ePROAV_EXEC_CODE dvPro_Overscan_Set(const eMCT eCH,eOVER_SCAN_TYPE ucOverScan);    //A70LV_Doulas_0009
UINT8 dvPro_Set_Sharpness(const eMCT eCH);
ePROAV_EXEC_CODE dvPro_Digital_Horz_Zoom_Set(const eMCT eCH,UINT16 uiDigital_Horz_Zoom);    //A70LV_Doulas_0009
ePROAV_EXEC_CODE dvPro_Digital_Vert_Zoom_Set(const eMCT eCH,UINT16 uiDigital_Vert_Zoom);    //A70LV_Doulas_0009
ePROAV_EXEC_CODE dvPro_Digital_Horz_Shift_Set(const eMCT eCH,UINT16 uiDigital_Horz_Shift);    //A70LV_Doulas_0009
ePROAV_EXEC_CODE dvPro_Digital_Vert_Shift_Set(const eMCT eCH,UINT16 uiDigital_Vert_Shift);    //A70LV_Doulas_0009
ePROAV_EXEC_CODE dvPro_Digital_Horz_Zoom_Get(const eMCT eCH,UINT16 *uiDigital_Horz_Zoom);    //A70LV_Doulas_0009
ePROAV_EXEC_CODE dvPro_Digital_Vert_Zoom_Get(const eMCT eCH,UINT16 *uiDigital_Vert_Zoom);    //A70LV_Doulas_0009
ePROAV_EXEC_CODE dvPro_Digital_Horz_Shift_Get(const eMCT eCH,UINT16 *uiDigital_Horz_Shift);    //A70LV_Doulas_0009
ePROAV_EXEC_CODE dvPro_Digital_Vert_Shift_Get(const eMCT eCH,UINT16 *uiDigital_Vert_Shift);    //A70LV_Doulas_0009
void dvPro_Scale_Digital_Zoom(PsINPUT_TIMING_INFO psScalerInput,PsOUTPUT_TIMING_INFO psScalerOutput);   //A70LV_Doulas_0009

void dvPro_Scale_Digital_Zoom_Custom(PsINPUT_TIMING_INFO psScalerInput,PsOUTPUT_TIMING_INFO psScalerOutput);   //A70LV_Doulas_0010
void dvPro_Scale_Setting_Check(const eMCT eCH,PsINPUT_TIMING_INFO psScalerInput,PsOUTPUT_TIMING_INFO psScalerOutput); //A70LV_Doulas_0010
void dvPro_Start_Position_Set(const eMCT eCH,PsINPUT_TIMING_INFO psScalerInput,PsOUTPUT_TIMING_INFO psScalerOutput);   //A70LV_Doulas_0013
ePROAV_EXEC_CODE dvPro_Horz_Start_Position_Set(const eMCT eCH,UINT8 ucPosition);  //A70LV_Doulas_0010
ePROAV_EXEC_CODE dvPro_Vert_Start_Position_Set(const eMCT eCH,UINT8 ucPosition);  //A70LV_Doulas_0010
ePROAV_EXEC_CODE dvPro_Horz_Start_Position_Get(const eMCT eCH,UINT8 *ucPosition);  //A70LV_Doulas_0010
ePROAV_EXEC_CODE dvPro_Vert_Start_Position_Get(const eMCT eCH,UINT8 *ucPosition);  //A70LV_Doulas_0010

void dvPro_Set_RGB_Gain(const eMCT eCH);       //A70LV_Doulas_0011
void dvPro_Set_RGB_Offset(const eMCT eCH);     //A70LV_Doulas_00011
INT8 dvPro_RedOffset_Value_Set (UINT8 ucCH, INT8 iRedOffset);    //A70LV_Doulas_0022     //A70LV_Doulas_0011
INT8 dvPro_GreenOffset_Value_Set (UINT8 ucCH, INT8 iGreenOffset);//A70LV_Doulas_0022     //A70LV_Doulas_0011
INT8 dvPro_BlueOffset_Value_Set (UINT8 ucCH, INT8 iBlueOffset);  //A70LV_Doulas_0022     //A70LV_Doulas_0011
INT8 dvPro_RedGain_Value_Set (UINT8 ucCH, UINT8 iRedGain);       //A70LV_Doulas_0022     //A70LV_Doulas_0011
INT8 dvPro_GreenGain_Value_Set (UINT8 ucCH, UINT8 iGreenGain);   //A70LV_Doulas_0022     //A70LV_Doulas_0011
INT8 dvPro_BlueGain_Value_Set (UINT8 ucCH, UINT8 iBlueGain);     //A70LV_Doulas_0022

INT8 dvPro_Set_HUE_Value(eMCT eCH,INT16 iValue);               //A70LV_Doulas_0022
INT8 dvPro_Set_Saturation_Value(eMCT eCH,DOUBLE dSaturation);  //A70LV_Doulas_0022

ePROAV_EXEC_CODE dvPro_Set_SizePresets_Value(eMCT eCH,UINT8 ucSizePresets);  //A70LV_Doulas_0020
void dvPro_Sharpness_Value_Set(const eMCT eCH,UINT8 ucVal);     //A70LV_Doulas_0029
void dvPro_SkinColor_Value_Set(const eMCT eCH,UINT8 ucVal);
void dvPro_SkinColor_Set(const eMCT eCH);
void dvPro_Temporal_NR_Value_Set(const eMCT eCH,UINT8 ucVal);
void dvPro_MPEG_NR_Value_Set(const eMCT eCH,UINT8 ucVal);
void dvPro_Detect_Film_Value_Set(const eMCT eCH,UINT8 ucVal);
void dvPro_IintChannelSetting(const eMCT eCH,sCHANNEL_SETTING m_sCH_Setting);     //A70LV_Doulas_0030

void dvPro_InputFrameRate_Set(const eMCT eCH, DOUBLE dFrameRate);
DOUBLE dvPro_InputFrameRate_Get(const eMCT eCH);
void dvPro_InputPixelClock_Get(const eMCT eCH,UINT8 *ucValue);   //A70LV_Doulas_0056
UINT16 dvPro_InputVSize_Get(const eMCT eCH);
void dvPro_InputResolution_Get(const eMCT eCH,UINT8 *ucValue);
void dvPro_InputHorzRefresh_Get(const eMCT eCH,UINT8 *ucValue);
void dvPro_InputVertRefresh_Get(const eMCT eCH,UINT8 *ucValue);
void dvPro_InputVertRefresh2_Get(const eMCT eCH,UINT16 *uciValue);      //A70LV_Doulas_0200
void dvPro_InputSignalFormat_Get(const eMCT eCH,UINT8 *ucValue);
void dvPro_InputAspectRatio_Get(const eMCT eCH,UINT8 *ucValue);

void dvPro_FrontEndScanMode_Set(const eMCT eCH,UINT8 ucVal);     //A70LV_Doulas_0076
void dvPro_ForcedSyncResetDisable_Set(void);       //A70LV_Doulas_0079
BOOL dvPro_ForcedSyncResetType_Get(void);
void dvPro_ForceSyncRese_Get(BOOL *bForceSyncReseVal,UINT16 *uiH_Toral,UINT16 *uiV_Toral);
BOOL dvPro_IsInterlaced_Get(const eMCT eCH);       //A70LV_Doulas_0092
UINT16 dvPro_PanelVSize_Get(void);
DOUBLE dvPro_PanelFrameRateGet(void);
UINT16 dvPro_PanelHTotalGet(void);
UINT16 dvPro_PanelVTotalGet(void);
UINT32 dvPro_PanelPixelClockGet(void);
ePANEL_ID dvPro_PanelGet(void);       //A70LV_Doulas_0098
void dvPro_VideoYUV_Set(const eMCT eCH,UINT8 ucVal);     //A70LV_Doulas_0109
UINT8 dvPro_VideoYUV_Get(const eMCT eCH);   //A70LV_Doulas_0109
UINT16 dvPro_VGA_H_Total_Get(const eMCT eCH);                  //A70LV_Doulas_0112
void dvPro_VGA_SYNC_TYPE_Set(const eMCT eCH,UINT8 ucVal);
void dvPro_AutoImage_Set(UINT8 ucSource,UINT8 ucVal);
UINT8 dvPro_AutoImage_Get(void);
UINT32 dvPro_uiHFreq_Get(const eMCT eCH);      //A70LV_Doulas_0115
BOOL dvPro_CheckingHorzPositionSettingWork(eMCT eCH);
BOOL dvPro_CheckingVertPositionSettingWork(eMCT eCH);
UINT8 dvPro_HorzPositionWorkValueGet(eMCT eCH);
UINT8 dvPro_VertPositionWorkValueGet(eMCT eCH);
void dvPro_AutoPsitionStart(eMCT eCH,BOOL bAutoEnable);     //A70LV_Doulas_0118
void dvPro_Config_NoSignalOutput(const eMCT eCH, UINT8 ucDisplayOutput);   //A70LV_Doulas_0142 //A70LV_Doulas_0120

UINT16 dvPro_Input_H_Active_Get(const eMCT eCH);
UINT16 dvPro_Input_V_Active_Get(const eMCT eCH);
UINT16 dvPro_Input_H_Total_Get(const eMCT eCH);
UINT16 dvPro_Input_V_Total_Get(const eMCT eCH);
UINT16 dvPro_Input_H_Start_Get(const eMCT eCH);
UINT16 dvPro_Input_V_Start_Get(const eMCT eCH);

void dvPro_Input_3D_Format_Set(UINT8 ucVal);       //A70LV_Doulas_0154
UINT8 dvPro_Input_3D_Format_Get(void);             //A70LV_Doulas_0154
UINT8 dvPro_Input_3D_Format_Config_Get(void);      //A70LV_Doulas_0159
void dvPro_OSD_3D_Enable_Set(const eMCT eCH,UINT8 ucVal);     //A70LV_Doulas_0154
void dvPro_OSD_3D_Sync_OutSet(const eMCT eCH, UINT8 ucVal);
UINT8 dvPro_OSD_3D_Sync_OutGet(const eMCT eCH);
void dvPro_OSD_3D_Sync_InSet(const eMCT eCH, UINT8 ucVal);
UINT8 dvPro_OSD_3D_Sync_InGet(const eMCT eCH);
void dvPro_OSD_3D_InvertSet(const eMCT eCH, UINT8 ucVal);
UINT8 dvPro_OSD_3D_InvertGet(const eMCT eCH);
int dvPro_OSD_3D_Sync_Setting(UINT8 uc3DMode);
BOOL dvPro_AspectRatioIs3DModeGet(void);           //A70LV_Doulas_0154
void dvPro_3D_InputPort_Set(void);     //A70LV_Doulas_0154
UINT32 dvPro_InputHorzPeriod_Get(const eMCT eCH);    //A70LV_Doulas_0195
UINT16 dvPro_VGA_V_Total_Get(const eMCT eCH);
UINT8 dvPro_VGA_ModeTable_Get(const eMCT eCH);
UINT16 dvPro_VGA_ModeTableNumber_Get(const eMCT eCH);
UINT16 dvPro_ModeAdjusmenttHorzStart_Get(void);
UINT16 dvPro_ModeAdjusmenttVertStart_Get(void);
void dvPro_ModeAdjusmenttHorzStart_Set(UINT16 uiHStart);
void dvPro_ModeAdjusmenttVertStart_Set(UINT16 uiVStart);
void dvPro_ModeAdjusmenttDisableSetting(void);
void dvPro_Init_Mode_Adjustment_EEPROM_Setting(sTIMING_TABLE sTiming_Table);
//void dvPro_BuildPosition_2K(eMCT eCH);      //A70LV_Doulas_0238
//void dvPro_BuildScaler_2K(eMCT eCH);   //A70LV_Doulas_0284
void dvPro_NoSignal(const eMCT eCH, BOOL bTrue);
void dvPro_InputTimingSetting(const eMCT eCH, BOOL bSourceLock, PsINPUT_TIMING_INFO psInputTiming);
void dvPro_InputTimingGet(const eMCT eCH, sINPUT_TIMING_INFO *psInputTiming);
int dvPro_PixelShift_Setting(ePIXEL_SHIFT ePixelShift);
UINT8 dvPro_InputRataIntegerGet(DOUBLE ucIptFrameRate);
int dvPro_RGBMask(UINT8 ucMask);  //A70LK_Simon_0017
int dvPro_RGBLSMdSet(bool bSwitch); //A70LK_Casper_0011
void dvPro_SetHDRDemo(void);


#endif

