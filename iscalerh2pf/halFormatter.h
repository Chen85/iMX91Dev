#ifndef HALFORMATTER_H
#define HALFORMATTER_H

#include "Common.h"


//A70LV_Doulas_0006 start
#define MAX_HSG_RED_HUE         254
#define MIN_HSG_RED_HUE         0
#define INI_HSG_RED_HUE         127

#define MAX_HSG_RED_SAT         254
#define MIN_HSG_RED_SAT         0
#define INI_HSG_RED_SAT         127

#define MAX_HSG_RED_GAIN        254
#define MIN_HSG_RED_GAIN        0
#define INI_HSG_RED_GAIN        127

#define MAX_HSG_GREEN_HUE       254
#define MIN_HSG_GREEN_HUE       0
#define INI_HSG_GREEN_HUE       127

#define MAX_HSG_GREEN_SAT       254
#define MIN_HSG_GREEN_SAT       0
#define INI_HSG_GREEN_SAT       127

#define MAX_HSG_GREEN_GAIN      254
#define MIN_HSG_GREEN_GAIN      0
#define INI_HSG_GREEN_GAIN      127

#define MAX_HSG_BLUE_HUE        254
#define MIN_HSG_BLUE_HUE        0
#define INI_HSG_BLUE_HUE        127

#define MAX_HSG_BLUE_SAT        254
#define MIN_HSG_BLUE_SAT        0
#define INI_HSG_BLUE_SAT        127

#define MAX_HSG_BLUE_GAIN       254
#define MIN_HSG_BLUE_GAIN       0
#define INI_HSG_BLUE_GAIN       127

#define MAX_HSG_CYAN_HUE        254
#define MIN_HSG_CYAN_HUE        0
#define INI_HSG_CYAN_HUE        127

#define MAX_HSG_CYAN_SAT        254
#define MIN_HSG_CYAN_SAT        0
#define INI_HSG_CYAN_SAT        127

#define MAX_HSG_CYAN_GAIN       254
#define MIN_HSG_CYAN_GAIN       0
#define INI_HSG_CYAN_GAIN       127

#define MAX_HSG_MAGENTA_HUE     254
#define MIN_HSG_MAGENTA_HUE     0
#define INI_HSG_MAGENTA_HUE     127

#define MAX_HSG_MAGENTA_SAT     254
#define MIN_HSG_MAGENTA_SAT     0
#define INI_HSG_MAGENTA_SAT     127

#define MAX_HSG_MAGENTA_GAIN    254
#define MIN_HSG_MAGENTA_GAIN    0
#define INI_HSG_MAGENTA_GAIN    127

#define MAX_HSG_YELLOW_HUE      254
#define MIN_HSG_YELLOW_HUE      0
#define INI_HSG_YELLOW_HUE      127

#define MAX_HSG_YELLOW_SAT      254
#define MIN_HSG_YELLOW_SAT      0
#define INI_HSG_YELLOW_SAT      127

#define MAX_HSG_YELLOW_GAIN     254
#define MIN_HSG_YELLOW_GAIN     0
#define INI_HSG_YELLOW_GAIN     127

#define MAX_HSG_WHITE_RED_GAIN  254
#define MIN_HSG_WHITE_RED_GAIN  0
#define INI_HSG_WHITE_RED_GAIN  127

#define MAX_HSG_WHITE_GREEN_GAIN  254
#define MIN_HSG_WHITE_GREEN_GAIN  0
#define INI_HSG_WHITE_GREEN_GAIN  127

#define MAX_HSG_WHITE_BLUE_GAIN   254
#define MIN_HSG_WHITE_BLUE_GAIN   0
#define INI_HSG_WHITE_BLUE_GAIN   127

#define CCA_D_MAILBOX_SIZE 42 // in byte
#define CCA_M_MAILBOX_SIZE 36 // in byte


#define BC_CALIBRATE_ENABLE         0x10000L
#define BC_CALIBRATE_DISABLE        0xFFFF
#define BC_CALIBRATE_RED_ENABLE     0xFFFF
#define BC_CALIBRATE_RED_DISABLE    0
#define BC_CALIBRATE_GREEN_ENABLE   0xFFFF
#define BC_CALIBRATE_GREEN_DISABLE  0
#define BC_CALIBRATE_BLUE_ENABLE    0xFFFF
#define BC_CALIBRATE_BLUE_DISABLE   0
#define BC_CALIBRATE_YELLOW_ENABLE  0xFFF
#define BC_CALIBRATE_YELLOW_DISABLE 0
//A70LV_Doulas_0006 end

#define LIGHTSOUTSIGNALLEVELSTRENGHT  3  //Gray pattern = 48 @ maximum = 4

#define CEILINGWITHREAR  3
#define CEILINGPROJECTOR 2
#define NORMALPROJECTOR  1
#define NORMALWITHREAR   0


#define I2C_DIAG_EEPROM     (0x0001 << 0)
#define I2C_DIAG_EEPROM_XPR (0x0001 << 1)
#define I2C_DIAG_XPR        (0x0001 << 2)
#define I2C_DIAG_XPR_MAX10  (0x0001 << 3)
#define I2C_DIAG_RX24T      (0x0001 << 4)
#define I2C_DIAG_LD         (0x0001 << 5)

typedef enum
{
    DDP_I2C_DIAG_EEPROM,
    DDP_I2C_DIAG_EEPROM_XPR,
    DDP_I2C_DIAG_XPR,
    DDP_I2C_DIAG_XPR_MAX10,
    DDP_I2C_DIAG_RX24T,
    DDP_I2C_DIAG_LD,

    DDP_I2C_DIAG_NUMBER
} eDDP_I2C_DIAG;


// A70LV_Eric.C_0002 Start
typedef enum
{
    BG_BLK = 0,
    BG_WHT,
    BG_GEN,
    BG_RED,
    BG_BLU,
    BG_YLO,
    BG_CYN,
    BG_MGT,

    BG_DISABLE
} eBG_DISPLAY_MODE;
// A70LV_Eric.C_0002 End

typedef union
{
    UINT8 ucData[12];

    struct
    {
        UINT16 uiRed;
        UINT16 uiGreen;
        UINT16 uiBlue;
        UINT16 uiC1;
        UINT16 uiC2;
        UINT16 uiSense;
    } sSSI;

    struct
    {
        UINT16 uiRed;
        UINT16 uiGreen;
        UINT16 uiBlue;
        UINT16 uiYellow;
        UINT16 uiRLD_Red;
        UINT16 uiRLD_Yellow;
    } sLD;
} uSSI_DRIVER;

typedef struct
{
    UINT16 wBLD_R;
    UINT16 wBLD_G;
    UINT16 wBLD_B;
    UINT16 wBLD_Y;
    UINT16 wRLD_R;
    UINT16 wRLD_Y;
} sLD_PWM;

typedef enum
{
    eCMD_Formatter_External = 0x00,
    eCMD_Formatter_TestPatterns,
    eCMD_Formatter_SolidField,
    eCMD_Formatter_Splash,
    eCMD_Formatter_Video,
    eCMD_Formatter_Blank,
    eCMD_Formatter_DMD_Freeze,
    eCMD_Formatter_Standby,
    eCMD_Formatter_INVALID,
} eCMD_Formatter_ProjectorMode;

#if 0
typedef enum
{
    eCMD_SPLASH_FOCUS_WORD = 0x00,
    eCMD_SPLASH_FOCUS_FULL,
    eCMD_SPLASH_13POINT,
    eCMD_SPLASH_WRGB64,

    eCMD_SPLASH_MAX_NUMBER
} eCMD_SPLASH_NUMBER;
#endif

typedef enum
{
    IMG_ALG_CHI,     /**< Chroma Interpolation              */
    IMG_ALG_CTI,     /**< Chroma Transient Improvement      */
    IMG_ALG_GAMMA,   /**< Gamma Correction                  */
    IMG_ALG_CCA,     /**< Color Coordinate Adjustment       */
    IMG_ALG_BRICOLOR,/**< BrilliantColor(TM)                */
    IMG_ALG_STM,     /**< Spatial Temporal Multiplexing     */
    IMG_ALG_BRS,     /**< BrightSync(TM)                    */
    IMG_ALG_LAST     /**< Marker for final algorithm        */
} IMG_ALGORITHMS;   //A70LV_Doulas_0006

typedef enum
{
    eDDP_DISP_BLACK_LEVEL_OFF       = 0,
    eDDP_DISP_BLACK_LEVEL_DEFAULT   = 128,
} eDDPDISP_DISP_BALCK_LEVEL;        //A70LV_Doulas_0013

typedef enum
{
#ifdef PLATFORM_A70G2

    /*00*/  DDP442X_GAMMA_2P2,
    /*01*/  DDP442X_GAMMA_VIDEO,
    /*02*/  DDP442X_GAMMA_FILM,
    /*03*/  DDP442X_GAMMA_DICOM,
    /*04*/  DDP442X_GAMMA_CRT, //vivid
    /*05*/  DDP442X_GAMMA_MAX_BRIGHT,
    /*06*/  DDP442X_GAMMA_ENHANCED,
    /*07*/  DDP442X_GAMMA_1P8,
    /*08*/  DDP442X_GAMMA_LINEAR,
    /*09*/  DDP442X_GAMMA_PHOTO,
    /*10*/  DDP442X_GAMMA_2P2_2,
    /*11*/  DDP442X_GAMMA_2P4,
    /*12*/  DDP442X_GAMMA_2P6,
    /*13*/  DDP442X_GAMMA_HDR_STANDARD,
    /*14*/  DDP442X_GAMMA_HDR_FILM,
    /*15*/  DDP442X_GAMMA_HDR_BRIGHT,
    /*16*/  DDP442X_GAMMA_HDR_DETAIL,

#elif defined (PLATFORM_H60_2K)	//HICC2_Doulas_0033 Modify//HICC2_Doulas_0001 Add

    /*00*/  DDP442X_GAMMA_1P8,
    /*01*/  DDP442X_GAMMA_1P9,
    /*02*/  DDP442X_GAMMA_2P0,
    /*03*/  DDP442X_GAMMA_2P1,
    /*04*/  DDP442X_GAMMA_2P2,
    /*05*/  DDP442X_GAMMA_2P3,
    /*06*/  DDP442X_GAMMA_2P4,
    /*07*/  DDP442X_GAMMA_2P5,
    /*08*/  DDP442X_GAMMA_2P6,
    /*09*/  DDP442X_GAMMA_VIDEO,
    /*10*/  DDP442X_GAMMA_FILM,
    /*11*/  DDP442X_GAMMA_CRT, //vivid
    /*12*/  DDP442X_GAMMA_DICOM,
    /*13*/  DDP442X_GAMMA_ENHANCED,
    /*14*/  DDP442X_GAMMA_GRAPHIC,
    /*15*/  DDP442X_GAMMA_BLACKBOARD,
    /*16*/  DDP442X_GAMMA_HDR_BRIGHT, //HICC2_Doulas_0100
    /*17*/  DDP442X_GAMMA_HDR_DETAIL, //HICC2_Doulas_0100
    /*18*/  DDP442X_GAMMA_HDR_FILM,
    /*19*/  DDP442X_GAMMA_HDR_STANDARD,

#elif  defined (PLATFORM_A70LK) || defined (PLATFORM_H60_4K) || defined (PLATFORM_H30_4K)  //HICC2_Doulas_0100
    /*00*/  DDP442X_GAMMA_1P8,
    /*01*/  DDP442X_GAMMA_1P9,
    /*02*/  DDP442X_GAMMA_2P0,
    /*03*/  DDP442X_GAMMA_2P1,
    /*04*/  DDP442X_GAMMA_2P2,
    /*05*/  DDP442X_GAMMA_2P3,
    /*06*/  DDP442X_GAMMA_2P4,
    /*07*/  DDP442X_GAMMA_2P5,
    /*08*/  DDP442X_GAMMA_2P6,
    /*09*/  DDP442X_GAMMA_VIDEO,
    /*10*/  DDP442X_GAMMA_FILM,
    /*11*/  DDP442X_GAMMA_CRT, //vivid
    /*12*/  DDP442X_GAMMA_DICOM,
    /*13*/  DDP442X_GAMMA_ENHANCED,
    /*14*/  DDP442X_GAMMA_GRAPHIC,
    /*15*/  DDP442X_GAMMA_BLACKBOARD,
    /*16*/  DDP442X_GAMMA_HDR_BRIGHT,
    /*17*/  DDP442X_GAMMA_HDR_DETAIL,
    /*18*/  DDP442X_GAMMA_HDR_FILM,
    /*19*/  DDP442X_GAMMA_HDR_STANDARD,

#else  //PLATFORM_A35G2

    /*00*/  DDP442X_GAMMA_1P8,
    /*01*/  DDP442X_GAMMA_1P9,
    /*02*/  DDP442X_GAMMA_2P0,
    /*03*/  DDP442X_GAMMA_2P1,
    /*04*/  DDP442X_GAMMA_2P2,
    /*05*/  DDP442X_GAMMA_2P3,
    /*06*/  DDP442X_GAMMA_2P4,
    /*07*/  DDP442X_GAMMA_2P5,
    /*08*/  DDP442X_GAMMA_2P6,
    /*09*/  DDP442X_GAMMA_VIDEO,
    /*10*/  DDP442X_GAMMA_FILM,
    /*11*/  DDP442X_GAMMA_CRT,
    /*12*/  DDP442X_GAMMA_DICOM,
    /*13*/  DDP442X_GAMMA_ENHANCED,
    /*14*/  DDP442X_GAMMA_GRAPHIC,
    /*15*/  DDP442X_GAMMA_BLACKBOARD,
    /*16*/  DDP442X_GAMMA_HDR_BRIGHT,
    /*17*/  DDP442X_GAMMA_HDR_DETAIL,
    /*18*/  DDP442X_GAMMA_HDR_FILM,
    /*19*/  DDP442X_GAMMA_HDR_STANDARD,

#endif

    DDP442X_GAMMA_NUMBER,
}DDP442X_GAMMA_CURVE;       //A70LV_Doulas_0038

typedef enum
{
    DPP_SPLASH_AT_STARTUP,
    DPP_TPG_DISPLAYED,
    DPP_SUSPENDED,
    DPP_BEGIN_SCAN,
    DPP_LOOK_FOR_SYNCS,
    DPP_ATTEMPT_LOCK,
    DPP_CONFIG_SOURCE,
    DPP_MONITOR_SOURCE,
    DPP_STATE_MAX
} DPP_STATE;    //A70LV_Doulas_0191

typedef enum
{
    DISP_EXTERNAL_SOURCE,          /**< External Data Path (ADC, video decoder, or DVI) */
    DISP_SOLID_FIELD_SOURCE,       /**< Solid Field */
    DISP_SPLASH_SOURCE,            /**< Splash Screen */
    DISP_DISPLAY_SOURCE_INVALID    /**< Invalid display source type */
} DISP_DISPLAY_SOURCE;                  //A70LV_Doulas_0216

typedef enum //ZU860_John_0024 add ABP command to DDP
{
    eABP_CONTROL_MODE,

    eABP_TARGET_LIGHTSENSOR_BLD,
    eABP_DEFAULT_PWM_BLD,

    eABP_TARGET_LIGHTSENSOR_RLD,
    eABP_DEFAULT_PWM_RLD,

    eABP_LD_Number
}eABP_LD_Type;

typedef enum
{
    eXPR_OFF_2WAY       = 0,
    eXPR_OFF_4WAY_120   = 1,
    eXPR_OFF_4WAY_240   = 2,
    eXPR_ON             = 3,

    eeXPR_CONFIGURATION_NUMBER
}eXPR_CONFIGURATION;

typedef enum
{
    XPR_4WAY_CMD_FIXEDEN        = 0x00,
    XPR_4WAY_CMD_DACGAIN        = 0x01,
    XPR_4WAY_CMD_SFDELAY        = 0x02,
    XPR_4WAY_CMD_ACTTYPE        = 0x03,
    XPR_4WAY_CMD_OUTPUTSEL      = 0x04,
    XPR_4WAY_CMD_CLOCKWIDTH     = 0x05,
    XPR_4WAY_CMD_DACOFFSET      = 0x06,
    XPR_4WAY_CMD_RAMPLEN        = 0x07,
    XPR_4WAY_CMD_SEGMENTLEN     = 0x08,
    XPR_4WAY_CMD_INVPWMA        = 0x09,
    XPR_4WAY_CMD_INVPWMB        = 0x10,
    XPR_4WAY_CMD_SFFILTERVAL    = 0x11,
    XPR_4WAY_CMD_SFWATCHDOG     = 0x12,
    XPR_4WAY_CMD_SFDELAY_MAX10  = 0x13,

    XPR_4WAY_CMD_NUMBER
}XPR_4WAY_Command;

typedef struct
{
    UINT32 DDP_ST_CW_SPINNING         :1;
    UINT32 DDP_ST_CW_PHASELOCK        :1;
    UINT32 DDP_ST_CW_FREQLOCK         :1;
    UINT32 DDP_ST_LAMPLIT             :1;
    UINT32 DDP_ST_MEM_TST_PASSED      :1;
    UINT32 DDP_ST_RLED_ENABLE         :1;
    UINT32 DDP_ST_GLED_ENABLE         :1;
    UINT32 DDP_ST_BLED_ENABLE         :1;

    UINT32 DDP_ST_SOURCE_ACTIVE       :1;
    UINT32 DDP_ST_SYSTEM_READY        :1;
    UINT32 DDP_ST_FRAME_RATE_CONV_EN  :1;
    UINT32 DDP_ST_SEQ_PHASELOCK       :1;
    UINT32 DDP_ST_SEQ_FREQLOCK        :1;
    UINT32 DDP_ST_SEQ_SEARCH          :1;
    UINT32 DDP_ST_RESERVED_3          :1;
    UINT32 DDP_ST_RESERVED_4          :1;

    UINT32 DDP_ST_RESERVED_5          :8;

    UINT32 DDP_ST_RESERVED_6          :8;
}DDP_SYSTEM_STATUS, *PDDP_SYSTEM_STATUS;        //A70LV_Doulas_0212

typedef struct
{
	UINT32 DDP_ER_SEQ_ERROR 			 :1;
	UINT32 DDP_ER_PIX_CLO_OORANGE		 :1;
	UINT32 DDP_ER_VSYN_LOST_OORANGE 	 :1;
	UINT32 DDP_ER_DAD1000_THERMAL_FAULT  :1;
	UINT32 DDP_ER_DAD1000_VOLTAGE_FAULT  :1;
	UINT32 DDP_ER_DAD1000_CURRENT_FAULT  :1;
	UINT32 DDP_ER_UART_P0_COMM_ERROR	 :1;
	UINT32 DDP_ER_UART_P1_COMM_ERROR	 :1;

	UINT32 DDP_ER_UART_P2_COMM_ERROR	 :1;
	UINT32 DDP_ER_SSP_P0_COMM_ERROR 	 :1;
	UINT32 DDP_ER_SSP_P1_COMM_ERROR 	 :1;
	UINT32 DDP_ER_SSP_P2_COMM_ERROR 	 :1;
	UINT32 DDP_ER_I2C_P0_COMM_ERROR 	 :1;
	UINT32 DDP_ER_I2C_P1_COMM_ERROR 	 :1;
	UINT32 DDP_ER_I2C_P2_COMM_ERROR 	 :1;
	UINT32 DDP_ER_RESERVED				 :1;

	UINT32 DDP_ER_LAMP_HW_FAILURE		 :1;
	UINT32 DDP_ER_LAMP_PPP_TIMEOUT		 :1;
	UINT32 DDP_ER_ARM7_INIT_ERROR		 :1;
	UINT32 DDP_ER_NO_FREQ_BIN_ERROR 	 :1;
	UINT32 DDP_ER_RESERVED_2			 :4;

	UINT32 DDP_ER_RESERVED_3			 :8;

}DDP_ERROR_STATUS, *PDDP_ERROR_STATUS; //G100_Julie_0012

typedef enum
{
    LR_FIELD_GPIO,                 /**< FIELD GPIO determines L/R (High=Left).                              */
    LR_TOPFIELD,                   /**< Vsync/Hsync alignment determines L/R.                               */
    LR_1ST_FRAME,                  /**< Top frame/field is Left in vertially packed format. 1st half of
                                     *  Frame is Left in horizontally packed format.                        */
    LR_EMBEDDED_ENCODING,          /**< L/R reference is embedded in video data. (see @ref SRC_3D_LR_ENCODING)   */
    UNDEFINED_LR_REFERENCE         /**< Undefined L/R reference.                                            */
} SRC_3D_LR_REFERENCE;			//A70Gen2_Doulas_0023

typedef enum
{
    //XPR section
    FPGAcontrol_UHD_XPRinit				= 0x00,
    FPGAcontrol_UHD_XPRon	 			= 0x01,
    FPGAcontrol_UHD_XPRoff 				= 0x02,
    FPGAcontrol_UHD_isXPRon 			= 0x03,
    FPGAcontrol_UHD_XPRon_TPG 			= 0x04,
    FPGAcontrol_UHD_XPRoff_TPG			= 0x05,
    FPGAcontrol_UHD_XPRoff_TPG_3D		= 0x06,
    FPGAcontrol_UHD_XPR_CalPattern		= 0x07,
    FPGAcontrol_UHD_XPR_Filter			= 0x08,

    //WQXGA section
    FPGAcontrol_WQXGA					= 0x10,
    FPGAcontrol_WQXGA_TPG				= 0x12,
    FPGAcontrol_WQXGA_TPG_3D			= 0x13,

    //Common section
    FPGAcontrol_isVB13DOn				= 0x20,
    FPGAcontrol_isTPGOn					= 0x21,
    FPGAcontrol_is3DTPGOn				= 0x22,
    FPGAcontrol_TPG_pattern				= 0x23,
    FPGAcontrol_TPG_pattern_size		= 0x24,
    FPGAcontrol_getVersion				= 0x25,
    FPGAcontrol_LR_DataSwap				= 0x26,
    FPGAcontrol_VB1_Enable				= 0x27,
    FPGAcontrol_Get_TPG_FrameRate		= 0x28,
    FPGAcontrol_VB1_LaneLocked			= 0x29,
    FPGAcontrol_XPR_Orientation			= 0x30,
    FPGAcontrol_3D_LRSource				= 0x31,
    FPGAcontrol_VideoOutputTristate		= 0x32,
    FPGAcontrol_VB1_SwapOverride		= 0x33,

    //1D Keystone
    FPGAcontrol_1DKeystone				= 0x34,
    FPGAcontrol_1DKeystoneEnable		= 0x35,
    FPGAcontrol_1DPitchRange			= 0x36,

    //Error Handlers for XPR FPGA
    FPGAcontrol_FPGAErrorHandler		= 0x37, // Altera and Xilinx

    FPGAcontrol_4KPlus_Flag = 0x38, // H60_Eddie_0001

    //Close Loop Actuator status
    FPGAcontrol_ActrWriteEep = 0xe0,
    FPGAcontrol_ActrEepI2CProtect = 0xe1,
    FPGAcontrol_ActrEepZData0 = 0xe2,
    FPGAcontrol_ActrEepZData1 = 0xe3,
    FPGAcontrol_ActrEepXData0 = 0xe4,
    FPGAcontrol_ActrEepXData1 = 0xe5,
    FPGAcontrol_ActrEepYData0 = 0xe6,
    FPGAcontrol_ActrEepYData1 = 0xe7,
    FPGAcontrol_ActrAxis0CloseLoopEn    = 0xf0,
    FPGAcontrol_ActrAxis1CloseLoopEn    = 0xf1,
    FPGAcontrol_ActrErrGainAxis0        = 0xf2,
    FPGAcontrol_ActrErrSegLenAxis0      = 0xf3,
    FPGAcontrol_ActrErrGainAxis1        = 0xf4,
    FPGAcontrol_ActrErrSegLenAxis1      = 0xf5,
    FPGAcontrol_ActrSN0 = 0xf6,
    FPGAcontrol_ActrSN1 = 0xf7,
    FPGAcontrol_ActrSN2 = 0xf8,
    FPGAcontrol_ActrSN3 = 0xf9,
    FPGAcontrol_ActrEepTargetAdcAxis0 = 0xfa,
    FPGAcontrol_ActrEepTargetAdcAxis1 = 0xfb,
    FPGAcontrol_ActrClsError0 = 0xfc,       //HICC2_Steven_0051 start
    FPGAcontrol_ActrClsError1 = 0xfd,
    FPGAcontrol_ActrClslpValid = 0xfe,
    FPGAcontrol_ActrClslpErrorReset = 0xff  //HICC2_Steven_0051 end

}FPGACTRL_COMMAND;

eRESULT halFormatter_MutexInit(void);
//ZU860_John_0024 start add ABP command to DDP
void cmdGetHalfword(UINT16* data_out, UINT8* cmd_buffer);
void cmdPutHalfword(UINT16 data_out, UINT8* cmd_buffer);
void cmdGetWord(UINT32* data_out, UINT8* cmd_buffer);
void cmdPutWord(UINT32 data_out, UINT8* cmd_buffer);
//ZU860_John_0024 end

eRESULT halFormatter_GetSSIPWMDriverLevels_1(uSSI_DRIVER *uSSI);     //A70LV_Doulas_0006 Add
eRESULT halFormatter_SetSSIPWMDriverLevels_1(uSSI_DRIVER *uSSI);
eRESULT halFormatter_GetSSIPWMDriverLevels_2(uSSI_DRIVER *uSSI);
eRESULT halFormatter_SetSSIPWMDriverLevels_2(uSSI_DRIVER *uSSI);

eRESULT halFormatter_SetDisplaySFGColor(UINT16 uiSFG_Red,UINT16 uiSFG_Green,UINT16 uiSFG_Blue);     //A70LV_Doulas_0006 Add
eRESULT halFormatter_GetDisplaySFGColor(UINT16 *uiSFG_Red,UINT16 *uiSFG_Green,UINT16 *uiSFG_Blue);  //A70LV_Doulas_0006 Add

eRESULT halFormatter_PW_Index_Get(UINT16 *uiPWIndex);   //A70LV_Doulas_0006 modify
eRESULT halFormatter_FW_Index_Get(UINT16 *uiFWIndex);   //A70LV_Doulas_0006 modify

eRESULT halFormatter_PW_Index_Set(UINT16 uiPWIndex);    //A70LV_Doulas_0006 modify
eRESULT halFormatter_Actuator_Switch_Set(UINT8 ucActuator);
eRESULT halFormatter_FW_Index_Set(UINT16 uiFWIndex);    //A70LV_Doulas_0006 modify
eRESULT halFormatter_PowerMode_Set(UINT8 ucPowerMode);  //A70LV_Doulas_0006 modify
eRESULT halFormatter_Actuator_Switch(UINT8 ucActuator); //A70LV_Doulas_0006 modify
eRESULT halFormatter_Actuator_FixedOut_Set(UINT8 ucCh, UINT8 ucFixedOut);
eRESULT halFormatter_Actuator_FixedOut_Get(UINT8 ucCh, UINT8 *ucFixedOut);
eRESULT halFormatter_Actuator_Gain_Set(UINT8 ucCh, UINT8 ucActuatorGain);
eRESULT halFormatter_Actuator_Gain_Get(UINT8 ucCh, UINT8 *ucActuatorGain);
eRESULT halFormatter_Actuator_Segment_Length_Set(UINT8 ucCh, UINT16 uiSeg);
eRESULT halFormatter_Actuator_Segment_Length_Get(UINT8 ucCh, UINT16 *uiSeg);
eRESULT halFormatter_Actuator_SubFrameDelay_Set(UINT8 ucCh, UINT32 ulDelay);
eRESULT halFormatter_Actuator_SubFrameDelay_Get(UINT8 ucCh, UINT32 *ulDelay);

eRESULT halFormatter_Standby(void); //A70LV_Larry_0212
eRESULT halFormatter_TestPattern(UINT8 ucPattern);      //A70LV_Doulas_0006 modify

eRESULT halFormatter_HorFlip(BOOL bH_Flip);        //A70LV_Doulas_0006 modify
eRESULT halFormatter_VerFlip(BOOL bV_Flip);        //A70LV_Doulas_0006 modify

//A70LV_Doulas_0006
void halFormatter_AsicReadySet(BOOL bBnable);
BOOL halFormatter_AsicReadyGet(void); //HICC2_Doulas_0085
eRESULT halFormatter_HorFlipGet(BOOL *bH_Flip);
eRESULT halFormatter_VerFlipGet(BOOL *bV_Flip);

eRESULT halFormatter_SystemModeSet(UINT8 cSystemMode);
eRESULT halFormatter_SystemModeGet(UINT8 *cSystemMode);
eRESULT halFormatter_SplashLoad(UINT8 cNum);
eRESULT halFormatter_dv4422_TestPattern(UINT8 cTestPattern);
eRESULT halFormatter_CW_SpokeTest(UINT8 cEnable);
eRESULT halFormatter_SPLASH_SCRRENSet(UINT8 ucSplashStartupEnable);
eRESULT halFormatter_IMG_AlgorithmSet(IMG_ALGORITHMS eAlgo, BOOL bState);
eRESULT halFormatter_IMG_GammaLutSet(UINT8 cGamma);
eRESULT halFormatter_IMG_GammaLutGet(UINT8* cGamma);

eRESULT halFormatter_DB_EnableSet(UINT8 cDB);
eRESULT halFormatter_DB_EnableGet(UINT8 *cDB);
eRESULT halFormatter_DB_SetAperture(UINT16 cPosition);
eRESULT halFormatter_DB_GetAperture(UINT16 *cPosition);
eRESULT halFormatter_DB_SetMinMaxAperture(UINT16 ucMinPosition,UINT16 ucMaxPosition);
eRESULT halFormatter_DB_GetMinMaxAperture(UINT16 *ucMinPosition,UINT16 *ucMaxPosition);
eRESULT halFormatter_DB_SetOpenCloseAperture(UINT8 cOpenClose);
eRESULT halFormatter_DB_GetCurrentAperture(UINT16 *ucCurrentAperture);
eRESULT halFormatter_SYS_AppVersionGet(UINT16 *wDDP);
eRESULT halFormatter_VersionSaveGet(UINT8 *pcData);
eRESULT halFormatter_SEQ_RevisionGet(UINT32 *ucData);
eRESULT halFormatter_3D_3DModeSet(BOOL cEnable);
eRESULT halFormatter_3D_3DModeGet(BOOL *pbEnable);
eRESULT halFormatter_IMG_WhitePeakingSet(UINT8 cWhitePeak);
eRESULT halFormatter_IMG_WhitePeakingGet(UINT8 *cWhitePeak);
eRESULT halFormatter_3D_RefPolaritySet( BOOL bRef3D, BOOL bOddEven, BOOL bTopField, BOOL bInvert );
eRESULT halFormatter_3D_RefPolarityGet( BOOL *bRef3D, BOOL *bOddEven, BOOL *bTopField, BOOL *bInvert );
eRESULT halFormatter_3D_DLPLinkPulseSet(BOOL bEnable);
eRESULT halFormatter_3D_DLPLinkPulseGet(BOOL *pbEnable);
eRESULT halFormatter_DISP_FreezeSet(UINT8 cFreeze);
eRESULT halFormatter_DISP_FreezeGet(UINT8 *pcFreeze);
eRESULT halFormatter_DB_SetBorderCfg(UINT16 ucScalerOutHS,UINT16 ucScalerOutVS,UINT16 ucScalerOutHW,UINT16 ucScalerOutVW);
eRESULT halFormatter_DB_SetClipPixels(UINT16 ucClipPixels) ;
eRESULT halFormatter_IsColorWheelSpinning(BOOL *cSpin);
eRESULT halFormatter_HSGSet(UINT8* hsgdata);
INT16 halFormatter_HSGdata_Trans(INT8 cHSG_OSDvalue , BOOL bHue);
eRESULT halFormatter_HSGValueSet(sHSG_SETTING hsgdata);
eRESULT halFormatter_CCADataSend(UINT8 *data, UINT16 wSize);
eRESULT halFormatter_CCADataGet(UINT8 *data);
eRESULT halFormatter_HSG_UnitGainSet(void);
eRESULT halFormatter_SYS_SystemStatusGet(UINT8 *pcStatus);
eRESULT halFormatter_DISP_BlackLevelSet(UINT8 bValue);
eRESULT halFormatter_DISP_BlackLevelGet(UINT8 *bValue);
eRESULT halFormatter_DISP_BlackLevelThresholdSet(UINT8 bValue);
eRESULT halFormatter_DISP_BlackLevelThresholdGet(UINT8 *bValue);
eRESULT halFormatter_DISP_BlackStatusSet(UINT8 bValue);
eRESULT halFormatter_DISP_BlackPowerStatusGet(UINT8 *bValue);
eRESULT halFormatter_ILL_AuxSyncSet( BOOL bEnable );
eRESULT halFormatter_ILL_AuxSyncGet( BOOL *pbEnable );
eRESULT halFormatter_ILL_AuxSyncTypeSet(UINT8 *pcSyncType);
eRESULT halFormatter_ILL_AuxSyncTypeGet(UINT8 *pcSyncType);
eRESULT halFormatter_ILL_BoostRLDEnableSet(UINT8 cEn);
eRESULT halFormatter_ILL_BoostRLDEnableGet(UINT8 *pcEn);
eRESULT halFormatter_DMD_BGDisplayModeSet(UINT8 cMode);
eRESULT halFormatter_DMD_BGDisplayModeGet(UINT8 *pcMode);
eRESULT halFormatter_SensorTimingGet(UINT16 *SensorStartDelay, UINT16 *RedSensorTiming ,UINT16 *GreenSensorTiming,UINT16 *BlueSensorTiming,UINT16 *YellowSensorTiming);
eRESULT halFormatter_SensorTimingSet(UINT16 SensorStartDelay, UINT16 RedSensorTiming ,UINT16 GreenSensorTiming,UINT16 BlueSensorTiming,UINT16 YellowSensorTiming);
eRESULT halFormatter_SensorDataGet(UINT16 *RedSensorData ,UINT16 *GreenSensorData,UINT16 *BlueSensorData,UINT16 *YellowSensorData);
eRESULT halFormatter_ImageRGBGainGet(UINT16 *CSCRedGain ,UINT16 *CSCGreenGain,UINT16 *CSCBlueGain);
eRESULT halFormatter_ImageRGBGainSet(UINT16 CSCRedGain ,UINT16 CSCGreenGain,UINT16 CSCBlueGain);
eRESULT halFormatter_DynamicBlackUsePWMGet(UINT16 *PWM_FULL, UINT16 *PWM_ECO);
eRESULT halFormatter_DynamicBlackUsePWMSet(UINT16 *PWM_FULL, UINT16 *PWM_ECO);
eRESULT halFormatter_DynamicBlackTableGet(UINT8 *DA_Table);
eRESULT halFormatter_DynamicBlackTableSet(UINT8 DA_Table);
eRESULT halFormatter_BCCalibrateSet(BOOL bEnable,UINT32 dRed,UINT32 dGreen,UINT32 dBlue,UINT32 dYellow);
eRESULT halFormatter_SegmentColorSet(UINT8 cSegmentColorEnabled,UINT8 cSegmentColor);
eRESULT halFormatter_SegmentColorGet(UINT8 *cSegmentColorEnabled,UINT8 *cSegmentColor);
eRESULT halFormatter_DBSpeedSet(UINT16 cDB_Speed);
eRESULT halFormatter_DBSpeedGet(UINT16 *cDB_Speed);
eRESULT halFormatter_ALC_RGB_LevelGet(UINT16 *RGBLevel);
eRESULT halFormatter_CWIndexClockGet(UINT32 *dClockPeriod,UINT32 *dCW1_Frequence);
eRESULT halFormatter_3D_SYNC_SelectionSet(UINT8 c3D_SYNC_Selection);
eRESULT halFormatter_3D_SYNC_SelectionGet(UINT8 *c3D_SYNC_Selection);
eRESULT halFormatter_3D_SYNC_OutSelectionSet(UINT8 c3D_SYNC_Out);
eRESULT halFormatter_3D_SYNC_OutSelectionGet(UINT8 *c3D_SYNC_Out);
eRESULT halFormatter_3D_SYNC_InputMonitorGet(BOOL *b3DSyncAvaliable,BOOL *b3DSyncLost,UINT8 *cdatapath_state);
eRESULT halFormatter_3D_ViewingModeSet(UINT8 cViewingMode);
eRESULT halFormatter_3D_ViewingModeGet(UINT8 *cViewingMode);
eRESULT halFormatter_ConstantPowerSet(UINT8 cPower);

#if 0 //ZU860_John_0013 sync with DDP API 9.0
eRESULT halFormatter_PictureModeSet(UINT8 ucPictureMode);     //A70LV_Doulas_0037
eRESULT halFormatter_ColorTemperatureSet(UINT8 ucColorTemperature);
eRESULT halFormatter_ColorWheelSpeedSet(UINT8 ucColorWheelSpeed);
eRESULT halFormatter_InputSourceSet(UINT8 ucInputSource);
eRESULT halFormatter_PowerModeSet(UINT8 ucPowerMode,UINT8 ucPowerModeNumber);
#endif
eRESULT halFormatter_ColorEnhancementSet(UINT8 ucColorEnhancement);      //A70LV_Doulas_0038

eRESULT halFormatter_PWM_Set(UINT8 cIndex, UINT16 uiValue);
UINT16 halFormatter_PWM_Get(UINT8 cIndex);
eRESULT halFormatter_PWM_Update(void); //A70LV_Larry_0076
UINT16 halFormatter_RLD_PWM_Set(UINT16 uiRValue,UINT16 uiYValue); //G100_Doulas_0023 Modify//A70LV_Larry_0363
BOOL halDDP4422_IsAsicReady(void);

eRESULT halFormatter_WallColorSet(UINT8 ucWallColor);      //A70LV_Doulas_0044
eRESULT halFormatter_ParameterSet(UINT8 ucPowerMode,UINT8 ucPowerModeNumber,UINT8 ucPictureMode,UINT8 ucColorTemperature,UINT8 ucColorWheelSpeed,UINT8 ucInputSource);    //A70LV_Doulas_0054
eRESULT halFormatter_CeilingMount_RearProjectSet(BOOL bCeilingMount,BOOL bRearProject);    //A70LV_Doulas_0062
eRESULT halFormatter_CeilingMount_RearProjectGet(UINT8 *pucValue);
eRESULT halFormatter_COLOR_ENHANCEMENT_ValueGet(UINT8 ucDispalyMode,UINT8 ucCE,sHSG_SETTING *sHSG);    //A70LV_Doulas_0074
eRESULT halFormatter_CE_Add_HSG_Set(UINT8 ucDispalyMode,UINT8 ucCE,sHSG_SETTING sHSG);
eRESULT halFormatter_Upgrade_Set(UINT8 ucEnable); //A70LV_Larry_0138
eRESULT halFormatter_3D_FormatSet(UINT8 c3D_Format);        //A70LV_Doulas_0172
eRESULT halFormatter_3D_FormatGet(UINT8 *c3D_Format);       //A70LV_Doulas_0172
eRESULT halFormatter_3DSourceConfigSet(UINT8 *c3D_Source)         ;		//A70Gen2_Doulas_0023
eRESULT halFormatter_3DSourceConfigGet(UINT8 *c3D_Source);		        //A70Gen2_Doulas_0023
eRESULT halFormatter_ChannelSourceSet(UINT8 cSource);       //A70LV_Doulas_0187
eRESULT halFormatter_LightsOutTimerSet(UINT8 cLightOutTimer); //A70LV_Larry_0236
eRESULT halFormatter_ABPINFOSet(UINT8 ucLDType, UINT8 *ucSensorData);  //ZU860_John_0024 add ABP command to DDP
eRESULT halFormatter_ABPINFOGet(UINT8 *ucStatus);                      //ZU860_John_0024 add ABP command to DDP

eRESULT halFormatter_IMG_AlgorithmEnableSet(UINT8* cAlg);        //A70LV_Doulas_0235
eRESULT halFormatter_IMG_AlgorithmEnableGet(UINT8* cAlg);        //A70LV_Doulas_0235
eRESULT halFormatter_Projection_Mode_Set(UINT8 ucProjection_Mode);
eRESULT halFormatter_Projection_Mode_Get(UINT8 *ucProjection_Mode);
eRESULT halFormatter_SlaveProjection_Mode_Get(UINT8 *ucProjection_Mode);
eRESULT halFormatter_SRCDescriptionGet(UINT8 *pcSRC); //T100_Casper_0087
eRESULT halFormatter_APCCT_Set(UINT8* cCCT); //ZU860_John_0018 add auto tuning of corrected color temperature//A70LV_John_0077 add CCT function for AP
eRESULT halFormatter_APCCT_Get(UINT8* cCCT); //ZU860_John_0018 add auto tuning of corrected color temperature//A70LV_John_0077 add CCT function for AP
eRESULT halFormatter_XPR_Set(UINT8 ucEnable);
eRESULT halFormatter_XPR_3DLR_Set(UINT8 ucEnable);
eRESULT halFormatter_XPR_DataSwap_Set(UINT8 ucEnable);
eRESULT halFormatter_WAP_SetGet(UINT8 ucWap_Mode,eWAP_POWERLEVEL eWap_Level); //G50_Casper_0006
eRESULT halFormatter_WAP_AllSet(eWAP_POWERLEVEL eWap_Level,UINT16 *ucPWM); //G50_Casper_0006
eRESULT halFormatter_WAP_Set(UINT8 ucWap_Mode,eWAP_POWERLEVEL eWap_Level,UINT8 ucWap_LDSeq,UINT16 ucPWM); //G50_Casper_0006
eRESULT halFormatter_XPR_CalibrationPattern_Set(UINT8 ucEnable);
eRESULT halFormatter_XPR_CalibrationPattern_Get(UINT8 *ucValue);
eRESULT halFormatter_XPR_ErrorStatus_Set(void);
eRESULT halFormatter_XPR_ErrorStatus_Get(UINT8 *ucValue);
eRESULT halFormatter_XPR_Filter_Set(UINT8 ucDatax); //A70LK_Casper_0002
eRESULT halFormatter_XPR_Filter_Get(UINT8 *ucValue); //A70LK_Casper_0002
eRESULT halFormatter_XPR_Close_Loop_Enable_Axis0_Set(UINT8 ucDatax);
eRESULT halFormatter_XPR_Close_Loop_Enable_Axis0_Get(UINT8 *ucValue);
eRESULT halFormatter_XPR_Close_Loop_Enable_Axis1_Set(UINT8 ucDatax);
eRESULT halFormatter_XPR_Close_Loop_Enable_Axis1_Get(UINT8 *ucValue);
eRESULT halFormatter_XPR_Gain_of_Axis0_Get(UINT32 *ucValue);
eRESULT halFormatter_XPR_Gain_of_Axis1_Get(UINT32 *ucValue);
eRESULT halFormatter_XPR_Segment_Length_of_Axis0_Get(UINT32 *ucValue);
eRESULT halFormatter_XPR_Segment_Length_of_Axis1_Get(UINT32 *ucValue);
eRESULT halFormatter_XPR_Eeprom_TargerAdc_Axis0_Get(UINT32 *ucValue);
eRESULT halFormatter_XPR_Eeprom_TargerAdc_Axis1_Get(UINT32 *ucValue);
eRESULT halFormatter_XPR_Eeprom_ZData0_Get(UINT32 *ucValue);
eRESULT halFormatter_XPR_Eeprom_ZData1_Get(UINT32 *ucValue);
eRESULT halFormatter_XPR_Eeprom_XData0_Get(UINT32 *ucValue);
eRESULT halFormatter_XPR_Eeprom_XData1_Get(UINT32 *ucValue);
eRESULT halFormatter_XPR_Eeprom_YData0_Get(UINT32 *ucValue);
eRESULT halFormatter_XPR_Eeprom_YData1_Get(UINT32 *ucValue);
eRESULT halFormatter_XPR_Gain_Write_EEPROM(void);
eRESULT halFormatter_WAP_Get(UINT16* ucPWM); //G50_Casper_0006
eRESULT halFormatter_DB_StrengthSet(UINT8 ucValue);         //A70LV_Doulas_0327
eRESULT halFormatter_DB_StrengthGet(UINT8 *ucValue);        //A70LV_Doulas_0327
eRESULT halFormatter_FRC_BypassSet(UINT8 ucValue);
eRESULT halFormatter_FRC_BypassGet(UINT8 *ucValue);
UINT8 halFormatter_FRC_Bypass_Flag_Get(void);
eRESULT halFormatter_BlankSignalSwitchSet(UINT8 ucEnable);         //ZU860_Doulas_0083
eRESULT halFormatter_BlankSignalSwitchGet(UINT8 *ucEnable);        //ZU860_Doulas_0083
eRESULT halFormatter_AspectRatio_Set(UINT8* ucVAl); //ZU860_Doulas_0119
eRESULT halFormatter_AspectRatio_Get(UINT8* ucVAl); //ZU860_Doulas_0119
eRESULT halFormatter_Smooth_Enable(BOOL bEnable, UINT8 ucValue);	//G100_Clare_0004
UINT8 halFormatter_Smooth_Enable_Get(UINT8* ucVAl);	//G100_Clare_0004
eRESULT halFormatter_GetRLD_Light(UINT16 *RLD_Light);		//G100_Doulas_0010
#ifdef Low_Latency_All
eRESULT halFormatter_FRCByPassModeSet(BOOL ucVAl);//SNPLU9000_Energy_0061
eRESULT halFormatter_FRCByPassModeGet(UINT8* ucVAl);//SNPLU9000_Energy_0061
eRESULT halFormatter_LDFadeInStartupSet(UINT8 ucShutterStartup, UINT8 ucShutterFadeIn);		//G100_Doulas_0024
eRESULT halFormatter_SplashAtStartupTimeoutSet(UINT8 ucTimeout);							//G100_Doulas_0024


#endif	/*Low_Latency_All*/
#if 0
void halFormatter_SKUTypeSet(BOOL ucVAl);	//G100_Clare_0015
BOOL halFormatter_SKUTypeGet(void);	//G100_Doulas_0080
#endif
eRESULT halFormatter_ColorOffsetSet(UINT8 cOffset); //T100_Casper_0055
void halFormatter_DimPower_SetDefault(void);
eRESULT halFormatter_DimPower_Set(BOOL bEnable, UINT8 ucValue, BOOL bCheck);

eRESULT halFormatter_GetI2C_Diag(UINT8 *pdata);
eRESULT halFormatter_RX24TVersion_Get(UINT16* ucVersion); //HICC2_Doulas_0018
eRESULT halFormatter_Extern_3D_SYNC_In_Double_Set(UINT8 ucDouble); //H30K_Doulas_0007
eRESULT halFormatter_XPR_SN_Get(UINT8 *ucValue);
eRESULT halFormatter_XPR_Version_Get(UINT8 *ucValue); //HICC2_Steven_0039
eRESULT halFormatter_AutoLockResync(void);
eRESULT halFormatter_GetI2C_Diag(UINT8 *pdata);

eRESULT halFormatter_XPR_CloseLoop_ErrorCode0(UINT32 *ucValue); //HICC2_Steven_0051 start
eRESULT halFormatter_XPR_CloseLoop_ErrorCode1(UINT32 *ucValue);
eRESULT halFormatter_XPR_CloseLoop_Valid(UINT8 *ucValue);
eRESULT halFormatter_XPR_Close_Loop_Error_Reset_Set(UINT8 ucDatax); //HICC2_Steven_0051 end

UINT8 halFormatter_Get_Current_DLPLink_Pulse(void);

void halFormatter_Bist(void *pvData);

#endif // HALFORMATTER_H

