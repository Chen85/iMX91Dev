#ifndef _DVC341_H_
#define _DVC341_H_


#ifdef __cplusplus
extern "C" {
#endif


#include "Common.h"
#include "utilCommon.h"     //A70LV_Doulas_0015
//#include "MemMap.h"
#include "utilStorageCfg.h"     //A70LV_Doulas_0195
#include "Board_SPI.h"
#include "Ichipc341_Reg.h"
#include "Ichipc341_MemTable.h"

//#define SWAP_VBO_RX0_LANE   //if swap, only support 4K60

#define C341_FILE_ENABLE (FALSE)
#define dbmsg(fmt, ...) dbmsg_ScalerDriver(__FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)

#define VAL_B8_OSYCT3CH1  (0)
//#define C341_SSP_CS eBOARD_SSP_CS1

#define MODIFY_FOR_C341
#define ONLY_TEST_FOR_C341

#define CHANNEL_MAX                     (4)
#define DEF_TIMEOUT                     (100)
#define DEF_WAIT_TIMEOUT	            (100)//600// Time out(sec) => wait_pivs, wait_povs, wait_pstat //A70LV_Doulas_0003 test
#define OUTPUT4K_CH_NUM                 (2)   //LR
#define OUTPUT2K_CH_NUM                 (1)
#define OUTPUT_CH_NUM                   (OUTPUT4K_CH_NUM)
#define CH_BANK_OFFSET                  (16)
#define CH_WPBANK_OFFSET                (5)
#define CH_V_BANK_OFFSET                (16*OUTPUT_CH_NUM)
#define CH_V_WPBANK_OFFSET              (5 *OUTPUT_CH_NUM)


//CHECK INPUT
#define DEF_PI_ACTHST_ERR_RANGE         2
#define DEF_PI_ACTHW_ERR_RANGE          2
#define DEF_PI_ACTVST_ERR_RANGE         2
#define DEF_PI_ACTVW_ERR_RANGE          2
#define DEF_PI_HCYCL_ERR_RANGE          5
#define DEF_PI_VCYCL_ERR_RANGE          5
#define DEF_PI_VYNC_FREQ_ERR_RANGE      0.2 //A70LV_Doulas_0078

//#define DEF_MWI_4K            	        0x22

///// SDRAM
#if 0
#define DEF_ISFLD0_4K		            0x00000000
#define DEF_ISFLD1_4K		            0x03000000//0x021E8000// = 4096 * 4 * 2170
#define DEF_ISFLD2_4K	                0x06000000//0x043D0000// = 4096 * 4 * 2170 * 2
#define DEF_ISFLD3_4K  		            0x09000000//0x065B8000// = 4096 * 4 * 2170 * 3

//Roger : Refer to C341_Basic_Funciton_v10.pdf , Page 23
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

#endif


// Serial Flash //A70LV_Larry_0001
//#define DEF_SFL_OSD_FSAD_2K             (TEXT_RAW_DATA_ADDR - EXTERNAL_FLASH_BASEED)     //Font start address
//#define DEF_SFL_OSD_PSAD_2K             (BITMAP_RAW_DATA_ADDR - EXTERNAL_FLASH_BASEED)   //Bitmap start address

//C821
//RTCT status
#define		RTCT_THRU				(0x000000)					// 0000_0000_0000_0000 (All Transfer THRU)
#define		RTCT_STOP				(0x088888)					// 1000_1000_1000_1000_1000 (All Transfer STOP)
#ifdef C341_WARPING_ENABLE      //A70LV_Doulas_0099 Modify
#define		RTCT_NORMAL				(0x092165)					// 0010_0001_0101_0101 (WPOVS_PI2VS_PI1VS_PO2VS_PO1VS)
#else
#define		RTCT_NORMAL				(0x002165)					// 0010_0001_0101_0101 (PI2VS_PI1VS_PO2VS_PO1VS)
#endif
#define		RTCT_POVSSTOP			(0x082188)					// 0010_0001_0100_0100 (WPOVS_PI2VS_PI1VS_STOP__STOP_)
#define		RTCT_PIVSTHRU			(0x090065)                  // ICH1/ICH2 Transfer THRU
#define		RTCT_PO1VSGO			(0x052155)
#define		RTCT_WARP			    (0x092159)

//C341
#define RTCT_OP_THROUGH         (0x0)
#define RTCT_OP_PIVSCH1         (0x1)
#define RTCT_OP_PIVSCH2         (0x2)
#define RTCT_OP_PIVSCH3         (0x3)
#define RTCT_OP_PIVSCH4         (0x4)
#define RTCT_OP_POVSCH1CH2      (0x9)
#define RTCT_OP_POVSCH3CH4      (0xA)
#define RTCT_OP_WPPOVSCH1CH2    (0xD)
#define RTCT_OP_WPPOVSCH3CH4    (0xE)
#define RTCT_OP_STOP            (0xF)




//======================== 2K ========================================

#define DEF_MWI_2K	               	    0x20//0x11
#define DEF_MWI_4K	               	    0x10
#define DEF_MWI_2K_3D                   0x08    //H30K_Doulas_0001


#define MEASURE_PORT_HTOTAL_MARGIN    2
#define MEASURE_PORT_VTOTAL_MARGIN    2
#define MEASURE_PORT_FREQ_H_MCLK_COUNT_MARGIN     100
#define MEASURE_PORT_FREQ_V_MCLK_COUNT_MARGIN     100


#define IMFH_MCLK_TOLERANCE    0x200//0x0300   //H30K_Doulas_0009
#define IMFV_MCLK_TOLERANCE    0x3000
/*
60.00Hz -> 3300000  (792000000 / 2 / 2 / 60.00)
60.01Hz -> 3299450  (792000000 / 2 / 2 / 60.01)
60.02Hz -> 3298900  (792000000 / 2 / 2 / 60.02)
60.03Hz -> 3298350  (792000000 / 2 / 2 / 60.03)

IMFV: (from C821 B35_IMFVCH1)
1080p60
小黑 pattern gen  => 3300257 (60Hz)
HP 440G2 PC       => 3300307 (60Hz)
804 TV            => 3300364 (60Hz)
804 PC            => 3304005 (59.93Hz)(OSD show 59.92Hz)
Apple tv          => 3303559 (59.94Hz)(OSD show 59.93Hz)
SUMSAMG UBD-M8500 => 3303760 (59.93Hz)(OSD show 59.93Hz)
*/

#define HORZ_POSITION_LIMIT_MIN         4   //A70LV_Doulas_0113
#define VERT_POSITION_LIMIT_MIN_PRO     4   //A70LV_Doulas_0113
#define VERT_POSITION_LIMIT_MIN_INT     8   //A70LV_Doulas_0113

//******************* Reg Define Start ************************//
// Bank=  0(0x00) FNAME=tags\SCQ3B0.tag


//======================== Reg Define End ======================//
#define C341_REG_SIZE           2578    //A65_Owen_0001, dump register used

#define C341_PASS 1
#define ERROR_C341_DATA_OUT_OF_RANGE -2

#define C341_BRIGHTNESS_MAX 511//521      //A70LV_Doulas_0011 modify
#define C341_BRIGHTNESS_DEFAULT 0
#define C341_BRIGHTNESS_MIN -511//-512    //A70LV_Doulas_0011 modify
#define C341_CONTRAST_MAX 2//3.99        //A70LV_Doulas_0035 modify
#define C341_CONTRAST_DEFAULT 1
#define C341_CONTRAST_MIN 0


//Interrupt //A70LV_Larry_0001
#define C341_INT_PI1VS               (0x00000001<<0)
#define C341_INT_PI2VS               (0x00000001<<1)
#define C341_INT_PO1VS               (0x00000001<<2)
#define C341_INT_PO2VS               (0x00000001<<3)
#define C341_INT_PI1VS_STOP          (0x00000001<<4)
#define C341_INT_PI2VS_STOP          (0x00000001<<5)
#define C341_INT_PI1HS_STOP          (0x00000001<<6)
#define C341_INT_PI2HS_STOP          (0x00000001<<7)
#define C341_INT_IMAGE_MEAS_1_STOP   (0x00000001<<8)
#define C341_INT_IMAGE_MEAS_2_STOP   (0x00000001<<9)
#define C341_INT_REFRESH_ERROR       (0x00000001<<10)
#define C341_INT_FONT_EXP_END        (0x00000001<<11)
#define C341_INT_GEN_DPMS1           (0x00000001<<12)
#define C341_INT_VIDEO_FIFO          (0x00000001<<13)
#define C341_INT_VIDEO_MISC          (0x00000001<<14)
#define C341_INT_FLASH_COMP          (0x00000001<<15)
#define C341_INT_SERIAL_CH1          (0x00000001<<16)
#define C341_INT_SERIAL_CH2          (0x00000001<<17)
#define C341_INT_IIC_CH1             (0x00000001<<18)
#define C341_INT_IIC_CH2             (0x00000001<<19)
#define C341_INT_PWM0_COMP           (0x00000001<<20)
#define C341_INT_PWM1_COMP           (0x00000001<<21)
#define C341_INT_PWM2_COMP           (0x00000001<<22)
#define C341_INT_PWM3_COMP           (0x00000001<<23)
#define C341_INT_PULLDOWN_CH1        (0x00000001<<24)
#define C341_INT_PULLDOWN_CH2        (0x00000001<<25)
#define C341_INT_AD_STOP             (0x00000001<<26)
#define C341_INT_WPPOVS              (0x00000001<<27)
#define C341_INT_ERROR_OCC           (0x00000001<<28)



#define C341_RED_OFFSET_MAX         511      //A70LV_Doulas_0011
#define C341_RED_OFFSET_DEFAULT     0
#define C341_RED_OFFSET_MIN         -511
#define C341_GREEN_OFFSET_MAX       511
#define C341_GREEN_OFFSET_DEFAULT   0
#define C341_GREEN_OFFSET_MIN       -511
#define C341_BLUE_OFFSET_MAX        511
#define C341_BLUE_OFFSET_DEFAULT    0
#define C341_BLUE_OFFSET_MIN        -511

#define C341_RED_GAIN_MAX         3.99      //A70LV_Doulas_0011
#define C341_RED_GAIN_DEFAULT     1
#define C341_RED_GAIN_MIN         0
#define C341_GREEN_GAIN_MAX       3.99
#define C341_GREEN_GAIN_DEFAULT   1
#define C341_GREEN_GAIN_MIN       0
#define C341_BLUE_GAIN_MAX        3.99
#define C341_BLUE_GAIN_DEFAULT    1
#define C341_BLUE_GAIN_MIN        0

#define C341_HUE_MAX                180     //A70LV_Doulas_0012
#define C341_HUE_DEFAULT            0
#define C341_HUE_MIN                -180
#define C341_SATURATION_MAX         2.1//3.99     //A70LV_Doulas_0052 modify
#define C341_SATURATION_DEFAULT     1
#define C341_SATURATION_MIN         0




//************************* Enum Start *************************//



typedef enum
{
    eSCALER_EXEC_CODE_PASS,                   /* pass */
    eSCALER_EXEC_CODE_FAIL,                   /* general fail indication */
    eSCALER_EXEC_CODE_FATAL,                  /* fatal error; halt application */
    eSCALER_EXEC_CODE_PANEL_ID_NOT_FOUND,
    eSCALER_EXEC_CODE_PANEL_TABEL_INDEX_NOT_FOUND,
    eSCALER_EXEC_CODE_CH1_NOT_INITED,         //Need to init channel 1 before than init others
    eSCALER_EXEC_CODE_CH_OVER_RANGE,
    eSCALER_EXEC_CODE_CH_AUTO_PHASE,      //A70LV_Doulas_0007
} eSCALER_EXEC_CODE;

//input port
typedef enum
{
    eC341_INPUT_PORT0,
    eC341_INPUT_PORT1,
    eC341_INPUT_PORT2,  //VBO 0
    eC341_INPUT_PORT3,  //VBO 1
    eC341_INPUT_PORT_NUMBER,
} eC341_INPUT_PORT;

//1ch_v = 2ch
typedef enum
{
    eC341_CH_V0,
    eC341_CH_V1,
    eC341_CH_VNUMBER,
} eC341_CH_V;           //window

//
typedef enum
{
    eC341_CH1 = 0,
    eC341_CH2,
    eC341_CH3,
    eC341_CH4,
    eC341_CH_NUMBER,
} eC341_CH;

typedef enum
{
    eC341_MCT_CH1,
    eC341_MCT_CH2,
    eC341_MCT_CH3,
    eC341_MCT_CH4,
    eC341_MCT_LAST,
} eC341_MCT;

#if (C341_FILE_ENABLE == TRUE)
typedef enum
{
    eINPUT_PORT_INFO_STATE_NA       = 0,
    eINPUT_PORT_INFO_STATE_2CH      = 1,
    eINPUT_PORT_INFO_STATE_4CH      = 2,
    eINPUT_PORT_INFO_STATE_1CH      = 4,
    eINPUT_PORT_INFO_STATE_1080I60  = 5,
    eINPUT_PORT_INFO_STATE_1080I50  = 6,
    eINPUT_PORT_INFO_STATE_480I     = 7,
    eINPUT_PORT_INFO_STATE_576I     = 8,

    eINPUT_PORT_INFO_STATE_NUMBER;
}eINPUT_PORT_INFO_STATE; //PI_STATE
#endif



//use for vbo port setting
typedef struct VBO_InfoSt {
	int VICT;
	int VIDSKW;
	int IDUAL;
	int ICHSEL1;
	int ICHSEL2;
	int ICHSEL3;
	int ICHSEL4;
	int LANE_NUM;
	int CH_LANE_NUM;
	int CH;
	int QUADCH_2K;
} VBO_Info;


/*
typedef enum
{
	ePOLARITY_NEGTIVIE,
	ePOLARITY_POSITIVE,
}ePOLARITY;
*/

#if 1
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

#if 1
typedef enum
{
    eMCT_CH1,
    eMCT_CH2,
    eMCT_LAST,
} eMCT;
#endif

typedef enum
{
    ePSQ_STATE_RUN,
    ePSQ_STATE_PAUSE,
    ePSQ_STATE_RESTART,
    ePSQ_STATE_STOP,
    ePSQ_STATE_LAST,
} ePSQ_STATE;


/*	SFLCMD[3:0]: Serial flash command select
b3-b0 Serial flash command select
SFLCMD
[3:0]
Operation
0   Idle (Use this setting when reading/writing from CPU.)
1   DMA transfer (Register read selected in SFLREGSEL  serial Flash write)
2   DMA transfer (Serial Flash read  Register write selected in SFLREGSEL)
3   DMA transfer (Serial Flash read  FONT data write)
    Use this option when expanding 24-bit FONT data to 32 bits. Keep SLFREGSEL at 0.
4   Sector deletion Issue the command followed by address and pole the status.
5   Bulk erase Issue the command and pole the status.
6-9 Reserved
10  DMA transfer (Serial Flash read  internal register write)
    Format of data on the serial Flash: register address, register value, register
    address, register value …
11-13 Reserved
14  Read operation with SFLGCMD (store read data to SFLGDT with the lowest byte
    first): with SFLCNT, set the number of bytes between 1 and 4.
15  Write operation with SFLGCMD (send out write data from SFLGDT with the lowest
byte first): with SFLCNT, set the number of bytes between 1 and 4.*/

typedef enum
{
    eC341_SFLCT_IDLE            = 0,
    eC341_SFLCT_DMA_REG2FLASH   = 1,
    eC341_SFLCT_DMA_FLASH2REG   = 2,

}eC341_SFLCT; //A70LV_Larry_0139

/*
0 DDR3-SDRAM
1 OSD Character Buffer
2 OSD Color Pallete1 CH1
3 OSD Color Pallete1 CH2
4 OSD Color Pallete2 CH1
5 OSD Color Pallete2 CH2
6 Output Image Gamma1 LUT CH1
7 Output Image Gamma1 LUT CH2
8 Output Image Gamma2 LUT CH1
9 Output Image Gamma2 LUT CH2
A Output Image Full Color Gamma LUT CH1
B Output Image Full Color Gamma LUT CH2
C Output Image Uniformity LUT CH1
D Output Image Uniformity LUT CH2
E De-interlacer LUT CH1
F De-interlacer LUT CH2
10 PSQ Instruction Memory
11 Distortion Correction LUT
12 Output Image Gamma3 LUT
13 Edge Blend Gain Gamma LUT
14 Edge Blend Bias LUT
15 Cursor
16 Adaptive Scale Interpolation Filter
*/


typedef enum
{
    eC341_SFLREGSEL_DDR     = 0x00,
    eC341_SFLREGSEL_OCB     = 0x01,
    eC341_SFLREGSEL_CPC11   = 0x02,
    eC341_SFLREGSEL_CPC12   = 0x03,
    eC341_SFLREGSEL_CPC21   = 0x04,
    eC341_SFLREGSEL_CPC22   = 0x05,
    eC341_SFLREGSEL_IGL11   = 0x06,
    eC341_SFLREGSEL_IGL12   = 0x07,
    eC341_SFLREGSEL_IGL21   = 0x08,
    eC341_SFLREGSEL_IGL22   = 0x09,
    eC341_SFLREGSEL_IFCG1   = 0x0A,
    eC341_SFLREGSEL_IFCG2   = 0x0B,
    eC341_SFLREGSEL_IUL1    = 0x0C,
    eC341_SFLREGSEL_IUL2    = 0x0D,
    eC341_SFLREGSEL_DEL1    = 0x0E,
    eC341_SFLREGSEL_DEL2    = 0x0F,
    eC341_SFLREGSEL_PSQ     = 0x10,
    eC341_SFLREGSEL_DCL     = 0x11,
    eC341_SFLREGSEL_IG3L    = 0x12,
    eC341_SFLREGSEL_EBG     = 0x13,
    eC341_SFLREGSEL_EBB     = 0x14,
    eC341_SFLREGSEL_CURSOR  = 0x15,

}eC341_SFLREGSEL; //A70LV_Larry_0139

typedef enum
{
    eINPUT_PORT1,
    eINPUT_PORT2,
    eINPUT_PORT3,
    eINPUT_PORT_LAST,
} eINPUT_PORT;				//R70G2_Doulas_0002



typedef enum
{
    eINPUT_PORT_INFO_STATE_NA       = 0,
    eINPUT_PORT_INFO_STATE_2CH      = 1,
    eINPUT_PORT_INFO_STATE_4CH      = 2,
    eINPUT_PORT_INFO_STATE_1CH      = 4,
    eINPUT_PORT_INFO_STATE_1080I60  = 5,
    eINPUT_PORT_INFO_STATE_1080I50  = 6,
    eINPUT_PORT_INFO_STATE_480I     = 7,
    eINPUT_PORT_INFO_STATE_576I     = 8,

    eINPUT_PORT_INFO_STATE_NUMBER
}eINPUT_PORT_INFO_STATE;


typedef enum
{
	eHDR_EOTF_SDR,
	eHDR_EOTF_TRADITIONAL_GAMMA_HDR,
	eHDR_EOTF_ST2084,  //HDR10 PQ
	eHDR_EOTF_HLG,

	eHDR_EOTF_NUMBER
} eHDR_EOTF_TYPE;

//====================== Enum End ======================//



//===================== Struct Start ===================//
#pragma pack(push)  /* push current alignment to stack */       //A70LV_Doulas_0123
#pragma pack(1)     /* set alignment to 1 byte boundary */      //A70LV_Doulas_0123
typedef struct
{
    UINT8 		        ucFrameRate;
    UINT16		        uiPixClk;
    UINT16		        uiHStart;
    UINT16		        uiHActive;
    UINT16		        uiVStart;
    UINT16		        uiVActive;
    UINT16		        uiHTotal;
    UINT16		        uiVTotal;
    UINT8		        ucHSyncWidth;
    UINT8				ucVSyncWidth;
    UINT8				ucPoRefDiv;
    UINT8				ucPoFbDiv;

    eLOW_LATENCY_MODE   eLow_Latency;   //H2PF_Simon_0187
} sOUTPUT_TIMING_INFO, *PsOUTPUT_TIMING_INFO;


typedef struct      //A70LV_Doulas_0112 modify
{
    DOUBLE                  dHFreq;            //how many H sync in one second  (H Freq)
    DOUBLE 		            dFrameRate;        //how many V sync in one second  (V Freq)
    UINT16		            uiHTotal;
    UINT16		            uiVTotal;
    UINT16		            uiHActive;
    UINT16		            uiVActive;
    UINT16		            uiHStart;
    UINT16		            uiVStart;
    eSCAN_MODE              eScanMode;
    UINT32                  ulHFreqMclkCount;  // H sync frequency MCLK count   //how many MCLK count between 2 H-sync  //IMFH_MCLK_COUNT[eCH]
    UINT32                  ulVFreqMclkCount;  // V sync frequency MCLK count   //how many MCLK count between 2 V-sync  //IMFV_MCLK_COUNT[eCH]
    UINT8                   ucVBOLaneNum;
    FLOAT                   fPICLK;
    eINPUT_PORT_INFO_STATE  lSTATE;  //VBO
} sINPUT_TIMING_INFO, *PsINPUT_TIMING_INFO;



typedef struct
{
    ePANEL_ID           ePanelId;
    sOUTPUT_TIMING_INFO        sOutputTimingInfo;
} sPANEL_INFO, *PsPANEL_INFO;


typedef struct
{
    //eMODE_TABLE_ID            eModeTableId;
    UINT16                    uiModeTableNum;
    sINPUT_TIMING_INFO        sInputTimingInfo;
} sMODE_TABLE, *PsMODE_TABLE;   //A70LV_Doulas_0005 Add


typedef struct
{
    UINT32 ulB0_PI1VS                       : 1;//PIVS
    UINT32 ulB1_PI2VS                       : 1;
    UINT32 ulB2_PI3VS                       : 1;
    UINT32 ulB3_PI4VS                       : 1;
    UINT32 ulB4_PO1VS                       : 1;//POVS
    UINT32 ulB5_PO2VS                       : 1;
    UINT32 ulB6_PO3VS                       : 1;
    UINT32 ulB7_PO4VS                       : 1;

    UINT32 ulB8_PI1VS_STOP_CHG              : 1;//PIVS Stop or Period Changed
    UINT32 ulB9_PI2VS_STOP_CHG              : 1;
    UINT32 ulB10_PI3VS_STOP_CHG             : 1;
    UINT32 ulB11_PI4VS_STOP_CHG             : 1;
    UINT32 ulB12_PI1HS_STOP                 : 1;//PI1HS stop
    UINT32 ulB13_PI2HS_STOP                 : 1;
    UINT32 ulB14_PI3HS_STOP                 : 1;
    UINT32 ulB15_PI4HS_STOP                 : 1;

    UINT32 ulB16_IMG_POS_MEASURE1_END       : 1;//Image Position Measurement 1 End
    UINT32 ulB17_IMG_POS_MEASURE2_END       : 1;//Image Position Measurement 2 End
    UINT32 ulB18_IMG_POS_MEASURE3_END       : 1;//Image Position Measurement 3 End
    UINT32 ulB19_IMG_POS_MEASURE4_END       : 1;//Image Position Measurement 4 End
    UINT32 ulB20_REFRESH_ERROR              : 1;//Refresh Error
    UINT32 ulB21_FONT_EXP_BITBLT_END        : 1;//FONT expansion/Bitblt end
    UINT32 ulB22_RESERVED                   : 1;//Reserved
    UINT32 ulB23_FLASH_WRITE_TRANS_END      : 1;//Serial Flash transfer end (write end)
    UINT32 ulB24_3WIRE_CH1_TRANS_END        : 1;//3-wire serial CH1 transfer end
    UINT32 ulB25_3WIRE_CH2_TRANS_END        : 1;//3-wire serial CH1 transfer end
    UINT32 ulB26_I2C_CTRL_CH1               : 1;//IIC CTRLCH1 interrupt
    UINT32 ulB27_I2C_CTRL_CH2               : 1;//IIC CTRLCH2 interrupt
    UINT32 ulB28_PWM0_END_PULLDOWN_DET_CH1  : 1;//PWM0 measurement end/pulldown CH1 detection
    UINT32 ulB29_PWM1_END_PULLDOWN_DET_CH2  : 1;//PWM1 measurement end/pulldown CH2 detection
    UINT32 ulB30_PWM2_END_PULLDOWN_DET_CH3  : 1;//PWM2 measurement end/pulldown CH3 detection
    UINT32 ulB31_PWM3_END_PULLDOWN_DET_CH4  : 1;//PWM3 measurement end/pulldown CH4 detection
} sINTERRUPT_ENABLE, *PsINTERRUPT_ENABLE;

#if 0
//Scaler information is a parameter which have 4 copy for 4 internal scalers
typedef struct
{

    //    BOOL                        bIsInterlaceIn;
} sCHANNEL_INFO, *PsCHANNEL_INFO;
#endif

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
    sOUTPUT_TIMING_INFO         sOutputTimingInfo;      //PS_ACT (PS_ACTHST, PS_ACTHW, ...)
 //   sOUTPUT_TIMING_INFO         sQuarterOutputTimingInfo; //4K  //A70LV_Doulas_0008 remove
    sOUTPUT_TIMING_INFO         sScalerOutputTimingInfo;   //OWIN    //A70LV_Doulas_2000


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
    sINPUT_TIMING_INFO          sPortInfo;  //PI_PORT_INFO  //for measure Vx1 1-lane timing
    sINPUT_TIMING_INFO          sInputTimingInfo;//PIACT  (PIACT is the active area of the input signal (Same meaning as Data Enable))
    sINPUT_TIMING_INFO          sInterlace_InputTimingInfo;//      , g_pi_act
    sINPUT_TIMING_INFO          sScaler_InputTimingInfo;   //IWIN    //A70LV_Doulas_2000
 //   eCOLOR_FORMAT               eColorFormat;     //A70LV_Doulas_0013
    UINT8                       ucInputPort;
    UINT8                       ucCH_INFO_CHG;  //g_chinfo_chg
    UINT8                       ucInputUpdate;  //PI_UPDATE
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
    UINT32                      ulFMCLK;

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

    UINT8                       ucSCFMFMTENB;  //0:30bit, 1:24bit, 2:20bit, 3:16bit   //0=YUV444 30b,1=YUV444_24bit,2=YUV422_20b,3=YUV422 16b
    UINT8                       ucSCFMFMT;

    UINT8                       ucPRIORITY;    //PM_PRIORITY  (for overlay)
    UINT8                       ucLAYOUT_CHG;  //PM_LAYOUT_CHG (PIP/PBP layout change)
} sCHANNEL_INFO, *PsCHANNEL_INFO;


typedef struct
{
    eCM_SOURCE_ID               eInputSource;
    eCM_SCALING_MODE_ID         eScalingMode;
    eOVER_SCAN_TYPE             eOverScan;
    eCOLOR_FORMAT               eColorFormat;

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
    INT16                       iRedOffset;
    INT16                       iGreenOffset;
    INT16                       iBlueOffset;
    DOUBLE                      dRedGain;
    DOUBLE                      dGreenGain;
    DOUBLE                      dBlueGain;

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
    UINT8                       cEdgeMask;      //ZU860_Doulas_0004
    UINT8                       eOSD_3D_Mode;       //ZU860_Doulas_0049
#ifdef Low_Latency_All
    UINT8                       ucLow_Latency;	//ZU860_Clare_0152
#endif	/*Low_Latency_All*/
} sCHANNEL_SETTING, *PsCHANNEL_SETTING;     //A70LV_Doulas_0013

#endif

typedef struct
{
    char *pString;
    UINT32 dwReg;
} sC341_REG;    //A70LV_Doulas_0002

typedef struct
{
    UINT16		        uiHActive;
    UINT16		        uiVActive;
    UINT16		        uiHStart;
    UINT16		        uiVStart;
} sSCALER_PANEL_INFO; //A35G2_CDS_Larry_0008

typedef struct
{
	int HST;
	int HW;
	int VST;
	int VW;
}sIMAGE_WINDOW;

typedef struct {
	UINT32	u32VideoPCLK;       // Pixel Clock
	UINT16	u16VideoHTotal;     // H Total
	UINT16	u16VideoHActive;    // H Active Video
	UINT16	u16VideoVTotal;     // V Total
	UINT16	u16VideoVActive;    // V Active Video
	UINT16  u16VideoVRate;      // V Rate (Hz)
}sVIDEO_TIMING2; //H30K_Doulas_0011


//#pragma pack(pop)                          // restore previous alignment    A70LV_Doulas_0123


//===================== Struct End =====================//
void dvC341_UpgradeAccess_Set(BOOL bEnable);		//A65_OPTOMA_Doulas_0126
BOOL dvC341_UpgradeAccess_Get(void);				//A65_OPTOMA_Doulas_0126

BOOL dvC341_SemaphoreTake(BOOL cEnable, const char *pcSemaphore);

void dvC341_RegAccessMode(eREG_ACCMODE eRegAccess);
void dvC341_BurstRead_FixedAdd(const UINT32 ulAddr, UINT16 uiSize, UINT8 *pucData); //A70LV_Larry_0139
void dvC341_BurstWrite_FixedAdd(const UINT32 ulAddr, UINT32 uiSize, UINT8 *pucData); //A70LV_Larry_0061
void dvC341_BurstRead_AddInc(const UINT32 ulAddr, UINT16 uiSize, UINT8 *pucData); //A70LV_Larry_0139
void dvC341_BurstWrite_AddInc(const UINT32 ulAddr, UINT32 uiSize, UINT8 *pucData); //A70LV_Larry_0061
void dvC341_Buffer_Flush(void);
void dvC341_Buffer_Flush_Burst(void);
void dvC341_WriteToBuffer(const UINT32 ulAddr, UINT32 ulData, const UINT8 ucBankOffset);
void dvC341_Write(const UINT32 ulAddr, UINT32 ulData, const UINT8 ucBankOffset);
void dvC341_Write_ch_v(eC341_CH_V ch_v, const UINT32 ulAddr, UINT32 ulData, const UINT8 ucBankOffset);
UINT32 dvC341_Read(const UINT32 ulAddr, const UINT8 ucBankOffset);
//UINT32 dvC341_WaitIntct(const eC341_CH_V eCH, const UINT32 ulCt);  //unused

INT8 dvC341_Brightness_Value_Set (UINT8 ucCH,INT16 iBrightness);    //A70LV_Doulas_0022
INT16 dvC341_Brightness_Value_Get (void);
INT8 dvC341_Contrast_Value_Set (UINT8 ucCH,DOUBLE dContrast);  //A70LV_Doulas_0022
DOUBLE dvC341_Contrast_Value_Get (void);

eSCALER_EXEC_CODE dvC341_Init_4K(const eC341_CH_V eCH, const ePANEL_ID ePanelId);
void dvC341_VBO_Init(void);
void dvC341_VBO_Init_3D_Panel(void); //H30K_Doulas_0001
void dvC341_RegInit(void);
void dvC341_SetOutputColor_2K(const eC341_CH_V eCH, const eCOLOR_FORMAT eColorFmt);
void dvC341_SetInputPort_2K(const eC341_CH_V eCH);
void dvC341_SetAutoFill_2K(const eC341_CH_V eCH, const UINT8 ucFillColor, const BOOL bEnable);
void dvC341_SetIdual_2K(const eC341_CH_V eCH);
void dvC341_SetAutoFill_2K(const eC341_CH_V eCH, const UINT8 ucFillColor, const BOOL bEnable);
BOOL dvC341_CheckInput_4K(eC341_CH eCH);
void dvC341_BuildPicture_4K(const eC341_CH_V eCH);
void dvC341_BuildPicture_4K_ChangePIPLayout(const eC341_CH_V eCH);   //A35G2_CDS_Simon_0001
void dvC341_SetOverlay_4K(const eC341_CH_V eCH, UINT8 ucIsEnable);      //ZU860_Doulas_0017
void dvC341_SetOverlayCoef_2K(const eC341_CH_V eCH, PUINT8 pucOverLay);

eSCALER_EXEC_CODE dvC341_PSQ_3D_LR_Switch(UINT32 ulOSFLD1_L, UINT32 ulOSFLD2_L,
                                        UINT32 ulOSFLD3_L, UINT32 ulOSFLD4_L,
                                        UINT32 ulOSFLD1_R, UINT32 ulOSFLD2_R,
                                        UINT32 ulOSFLD3_R, UINT32 ulOSFLD4_R);

eSCALER_EXEC_CODE dvC341_PSQ_EXE(ePSQ_STATE eState);

void dvC341_SetBrightness(const eC341_CH_V eCH);    //A70LV_Doulas_0022
//void dvC341_SetMatrix(eC341_CH_V eCH, eCOLOR_FORMAT eOutformat);   //unused
BOOL dvC341_PIP_Enable(void);
BOOL dvC341_PIP_PBP_Enable(void);        //A70LV_Doulas_0226
void dvC341_SetIpConv(const eC341_CH_V eCH, const BOOL bEnable);


#ifdef __cplusplus
}
#endif



void C341Test(void);
void dvC341_SetOutputTestPattern(void);
void dvC341_RegDump(void);
void dvC341_Bypass(void);   //A70LV_Doulas_0002
void dvC341_Test(void);     //A70LV_Doulas_0002
void dvC341_Set_HUE(eC341_CH_V eCH);   //A70LV_Doulas_0022
INT16 dvC341_Get_HUE(eC341_CH_V eCH);              //A70LV_Doulas_0022
void dvC341_Set_Saturation(eC341_CH_V eCH);  //A70LV_Doulas_0022
DOUBLE dvC341_Get_Saturation(eC341_CH_V eCH);         //A70LV_Doulas_0003
void dvC341_Set_PIP_PBP_Layout(UINT8 uiLayout);  //A70LV_Doulas_0003
void dvC341_Set_PIP_PBP_Size(UINT8 uiSize);  //A70LV_Doulas_0003
UINT8 dvC341_Get_PIP_PBP_Layout(void);  //A70LV_Doulas_0003
UINT8 dvC341_Get_PIP_PBP_Size(void);  //A70LV_Doulas_0003

void dvC341_Resync_Init(eC341_CH_V eCH);   //A70LV_Doulas_0004
BOOL dvC341_SYNC_LOCK_Compare(const eC341_CH_V eCH);   //A70LV_Doulas_0005 modify//A70LV_Doulas_0004
BOOL dvC341_InputPort_Sync_MCLK_Count_Get(const eC341_CH_V eCH, INT32 *lIMFH , INT32 *lIMFV);
void dvC341_Freeze_Set(eC341_CH_V eCH,BOOL bFreezeEn);     //A70LV_Doulas_0004
BOOL dvC341_Freeze_Get(eC341_CH_V eCH);        //A70LV_Doulas_0004
BOOL dvC341_Panel_Change_LVDS_Stop(void); //A70LV_Doulas_0005
BOOL dvC341_Panel_Change_Setting(eC341_CH_V eCH,const ePANEL_ID ePanelId); //A70LV_Doulas_0005
BOOL dvC341_AutoPositionSetting(const eC341_CH_V eCH);   //A70LV_Doulas_0005

UINT32 dvC341_GetAPL(const eC341_CH_V eCH,UINT8 *ucMax,UINT8 *ucMin);                   //A70LV_Doulas_0215 modify//A70LV_Doulas_0007
eSCALER_EXEC_CODE dvC341_AutoPhaseStart(const eC341_CH_V eCH);   //A70LV_Doulas_0007
eSCALER_EXEC_CODE dvC341_AutoPhase(const eC341_CH_V eCH);    //A70LV_Doulas_0007
UINT8 dvC341_CurrentPhaseGet(const eC341_CH_V eCH);       //A70LV_Doulas_0007
UINT8 dvC341_BsetPhaseGet(const eC341_CH_V eCH);         //A70LV_Doulas_0007
BOOL dvC341_HV_Sync_Measure(const eC341_CH_V eCH);        //A70LV_Doulas_0007
void dvC341_InputSourceSet(const eC341_CH_V eCH,UINT8 ucInputSource);   //A70LV_Doulas_0007

void dvC341_Set_Color(const eC341_CH_V eCH);      //A70LV_Doulas_0009
void dvC341_SetContrast(const eC341_CH_V eCH); //A70LV_Doulas_0009
void dvC341_Scale_Overscan(eOVER_SCAN_TYPE ucOverScan,PsINPUT_TIMING_INFO psScalerInput,PsOUTPUT_TIMING_INFO psScalerOutput);    //A70LV_Doulas_0009
eSCALER_EXEC_CODE dvC341_Overscan_Set(const eC341_CH_V eCH,eOVER_SCAN_TYPE ucOverScan);    //A70LV_Doulas_0009
void dvC341_Set_Sharpness(const eC341_CH_V eCH);
eSCALER_EXEC_CODE dvC341_Digital_Horz_Zoom_Set(const eC341_CH_V eCH,UINT16 uiDigital_Horz_Zoom);    //A70LV_Doulas_0009
eSCALER_EXEC_CODE dvC341_Digital_Vert_Zoom_Set(const eC341_CH_V eCH,UINT16 uiDigital_Vert_Zoom);    //A70LV_Doulas_0009
eSCALER_EXEC_CODE dvC341_Digital_Horz_Shift_Set(const eC341_CH_V eCH,UINT16 uiDigital_Horz_Shift);    //A70LV_Doulas_0009
eSCALER_EXEC_CODE dvC341_Digital_Vert_Shift_Set(const eC341_CH_V eCH,UINT16 uiDigital_Vert_Shift);    //A70LV_Doulas_0009
eSCALER_EXEC_CODE dvC341_Digital_Horz_Zoom_Get(const eC341_CH_V eCH,UINT16 *uiDigital_Horz_Zoom);    //A70LV_Doulas_0009
eSCALER_EXEC_CODE dvC341_Digital_Vert_Zoom_Get(const eC341_CH_V eCH,UINT16 *uiDigital_Vert_Zoom);    //A70LV_Doulas_0009
eSCALER_EXEC_CODE dvC341_Digital_Horz_Shift_Get(const eC341_CH_V eCH,UINT16 *uiDigital_Horz_Shift);    //A70LV_Doulas_0009
eSCALER_EXEC_CODE dvC341_Digital_Vert_Shift_Get(const eC341_CH_V eCH,UINT16 *uiDigital_Vert_Shift);    //A70LV_Doulas_0009
void dvC341_Scale_Digital_Zoom(PsINPUT_TIMING_INFO psScalerInput,PsOUTPUT_TIMING_INFO psScalerOutput);   //A70LV_Doulas_0009

void dvC341_Scale_Digital_Zoom_Custom(PsINPUT_TIMING_INFO psScalerInput,PsOUTPUT_TIMING_INFO psScalerOutput);   //A70LV_Doulas_0010
void dvC341_Scale_Setting_Check(const eC341_CH_V eCH,PsINPUT_TIMING_INFO psScalerInput,PsOUTPUT_TIMING_INFO psScalerOutput); //A70LV_Doulas_0010
void dvC341_Start_Position_Set(const eC341_CH_V eCH,PsINPUT_TIMING_INFO psScalerInput,PsOUTPUT_TIMING_INFO psScalerOutput);   //A70LV_Doulas_0013
eSCALER_EXEC_CODE dvC341_Horz_Start_Position_Set(const eC341_CH_V eCH,UINT8 ucPosition);  //A70LV_Doulas_0010
eSCALER_EXEC_CODE dvC341_Vert_Start_Position_Set(const eC341_CH_V eCH,UINT8 ucPosition);  //A70LV_Doulas_0010
//eSCALER_EXEC_CODE dvC341_Horz_Start_Position_Get(const eC341_CH_V eCH,UINT8 *ucPosition);  //A70LV_Doulas_0010
//eSCALER_EXEC_CODE dvC341_Vert_Start_Position_Get(const eC341_CH_V eCH,UINT8 *ucPosition);  //A70LV_Doulas_0010
UINT8 dvC341_Horz_Start_Position_Get(const eC341_CH_V eCH); //G100_Julie_0017
UINT8 dvC341_Vert_Start_Position_Get(const eC341_CH_V eCH); //G100_Julie_0017

void dvC341_Set_RGB_Gain(const eC341_CH_V eCH);       //A70LV_Doulas_0011
void dvC341_Set_RGB_Offset(const eC341_CH_V eCH);     //A70LV_Doulas_00011
INT8 dvC341_RedOffset_Value_Set (UINT8 ucCH,INT16 iRedOffset);    //A70LV_Doulas_0022     //A70LV_Doulas_0011
INT8 dvC341_GreenOffset_Value_Set (UINT8 ucCH,INT16 iGreenOffset);//A70LV_Doulas_0022     //A70LV_Doulas_0011
INT8 dvC341_BlueOffset_Value_Set (UINT8 ucCH,INT16 iBlueOffset);  //A70LV_Doulas_0022     //A70LV_Doulas_0011
INT8 dvC341_RedGain_Value_Set (UINT8 ucCH,DOUBLE dRedGain);       //A70LV_Doulas_0022     //A70LV_Doulas_0011
INT8 dvC341_GreenGain_Value_Set (UINT8 ucCH,DOUBLE dGreenGain);   //A70LV_Doulas_0022     //A70LV_Doulas_0011
INT8 dvC341_BlueGain_Value_Set (UINT8 ucCH,DOUBLE dBlueGain);     //A70LV_Doulas_0022

INT8 dvC341_Set_HUE_Value(eC341_CH_V eCH,INT16 iValue);               //A70LV_Doulas_0022
INT8 dvC341_Set_Saturation_Value(eC341_CH_V eCH,DOUBLE dSaturation);  //A70LV_Doulas_0022

eSCALER_EXEC_CODE dvC341_Set_SizePresets_Value(eC341_CH_V eCH,UINT8 ucSizePresets);  //A70LV_Doulas_0020
void dvC341_Sharpness_Value_Set(const eC341_CH_V eCH,UINT8 ucVal);     //A70LV_Doulas_0029
void dvC341_SkinColor_Value_Set(const eC341_CH_V eCH,UINT8 ucVal);
void dvC341_SkinColor_Set(const eC341_CH_V eCH);
void dvC341_Temporal_NR_Value_Set(const eC341_CH_V eCH,UINT8 ucVal);
void dvC341_MPEG_NR_Value_Set(const eC341_CH_V eCH,UINT8 ucVal);
void dvC341_Detect_Film_Value_Set(const eC341_CH_V eCH,UINT8 ucVal);
void dvC341_IintChannelSetting(const eC341_CH_V eCH,sCHANNEL_SETTING m_sCH_Setting);     //A70LV_Doulas_0030

void dvC341_TestPattern_Set(const eC341_CH_V eCH,UINT8 ucVal);     //A70LV_Doulas_0035
void dvC341_Channel1_Auto_Fill_Screen_Set(BOOL bEnable);
void dvC341_Channel2_Auto_Fill_Screen_Set(BOOL bEnable);
void dvC341_Warp_Auto_Fill_Screen_Set(BOOL bEnable);
void dvC341_Auto_Fill_Screen_Init(void);

void dvC341_InputPixelClock_Get(const eC341_CH_V eCH,UINT8 *ucValue);   //A70LV_Doulas_0056
void dvC341_InputResolution_Get(const eC341_CH_V eCH,UINT8 *ucValue);
void dvC341_InputHorzRefresh_Get(const eC341_CH_V eCH,UINT8 *ucValue);
void dvC341_InputVertRefresh_Get(const eC341_CH_V eCH,UINT8 *ucValue);
void dvC341_InputVertRefresh2_Get(const eC341_CH_V eCH,UINT16 *uciValue);      //A70LV_Doulas_0200
void dvC341_InputVertRefresh2_Get_FromMCLKcount(const eC341_CH_V eCH,UINT16 *uciValue);   //A35G2_CDS_Simon_0052
void dvC341_InputSignalFormat_Get(const eC341_CH_V eCH,UINT8 *ucValue);
void dvC341_InputAspectRatio_Get(const eC341_CH_V eCH,UINT8 *ucValue);

void dvC341_ColorSpace_Value_Set(const eC341_CH_V eCH,UINT8 ucVal);     //A70LV_Doulas_0069
void dvC341_FrontEndScanMode_Set(const eC341_CH_V eCH,UINT8 ucVal);     //A70LV_Doulas_0076
void dvC341_ForcedSyncResetDisable_Set(void);       //A70LV_Doulas_0079
void dvC341_ForcedSyncResetDisable_Set2(UINT16 uiH_Toral,UINT16 uiV_Toral);     //ZU860_Doulas_0138
BOOL dvC341_ForcedSyncResetType_Get(void);
void dvC341_ForceSyncReset_Get(BOOL *bForceSyncReseVal,UINT16 *uiH_Toral,UINT16 *uiV_Toral);
BOOL dvC341_IsInterlaced_Get(const eC341_CH_V eCH);       //A70LV_Doulas_0092
ePANEL_ID dvC341_PanelGet(void);       //A70LV_Doulas_0098
void dvC341_VideoYUV_Set(const eC341_CH_V eCH,UINT8 ucVal);     //A70LV_Doulas_0109
UINT8 dvC341_VideoYUV_Get(const eC341_CH_V eCH);   //A70LV_Doulas_0109
UINT16 dvC341_VGA_H_Total_Get(const eC341_CH_V eCH);                  //A70LV_Doulas_0112
void dvC341_VGA_SYNC_TYPE_Set(const eC341_CH_V eCH,UINT8 ucVal);
void dvC341_AutoImage_Set(UINT8 ucSource,UINT8 ucVal);
UINT8 dvC341_AutoImage_Get(void);
DOUBLE dvC341_uiHFreq_Get(const eC341_CH_V eCH);      //A70LV_Doulas_0115
void dvC341_Position_Run(eC341_CH_V eCH);             //A70LV_Doulas_0113
BOOL dvC341_CheckingHorzPositionSettingWork(eC341_CH_V eCH);
BOOL dvC341_CheckingVertPositionSettingWork(eC341_CH_V eCH);
UINT8 dvC341_HorzPositionWorkValueGet(eC341_CH_V eCH);
UINT8 dvC341_VertPositionWorkValueGet(eC341_CH_V eCH);
void dvC341_AutoPsitionStart(eC341_CH_V eCH,BOOL bAutoEnable);     //A70LV_Doulas_0118
void dvC341_Config_NoSignalOutput(const eC341_CH_V eCH, UINT8 ucDisplayOutput);   //A70LV_Doulas_0142 //A70LV_Doulas_0120  //A35G2_CDS_Simon_0017
void dvC341_FlashEraseSector(UINT32 ulSflAd, UINT32 ulDataSize); //A70LV_Larry_0139
void dvC341_DMA_Transfer(UINT32 ulAddress, UINT32 ulSize, UINT8 ucRegsel, UINT32 ulDmaCnt); //A70LV_Larry_0139

void dvC341_DDR_InputDataGet(eC341_CH_V eCH,UINT16 uiPixelHPosition,UINT16 uiPixelVPosition);      //A70LV_Doulas_0124
void dvC341_DDR_InputDataAreaTotalGet(eC341_CH_V eCH,RECT DataArea,UINT32 *ulTotalRed,UINT32 *ulTotalGreen,UINT32 *ulTotalBlue);
void dvC341_InputADC_Cali_CalibrationEnableSet(eC341_CH_V eCH,UINT8 ucADC_CAl_Enable);
UINT16 dvC341_Input_H_Active_Get(const eC341_CH_V eCH);
UINT16 dvC341_Input_V_Active_Get(const eC341_CH_V eCH);

void dvC341_Set_Movement_NR_Flag(const eC341_CH_V eCH,UINT8 ucVal);     //A70LV_Doulas_0145
void dvC341_Set_Horizontal_NR_Flag(const eC341_CH_V eCH,UINT8 ucVal);
void dvC341_Set_Vertical_NR_Flag(const eC341_CH_V eCH,UINT8 ucVal);
void dvC341_Set_Diagonal_Interpolation_Flag(const eC341_CH_V eCH,UINT8 ucVal);

void dvC341_Input_3D_Format_Set(UINT8 ucVal);       //A70LV_Doulas_0154
UINT8 dvC341_Input_3D_Format_Get(void);             //A70LV_Doulas_0154
UINT8 dvC341_Input_3D_Format_Config_Get(void);      //A70LV_Doulas_0159
void dvC341_SetInputMemoryBuffer_2K(eC341_CH_V eCH);      //A70LV_Doulas_0154
void dvC341_OSD_3D_Enable_Set(const eC341_CH_V eCH,UINT8 ucVal);     //A70LV_Doulas_0154
BOOL dvC341_AspectRatioIs3DModeGet(void);           //A70LV_Doulas_0154
void dvC341_3D_InputPort_Set(void);     //A70LV_Doulas_0154
UINT32 dvC341_InputHorzPeriod_Get(const eC341_CH_V eCH);    //A70LV_Doulas_0195
UINT16 dvC341_VGA_V_Total_Get(const eC341_CH_V eCH);
UINT8 dvC341_VGA_ModeTable_Get(const eC341_CH_V eCH);
UINT16 dvC341_VGA_ModeTableNumber_Get(const eC341_CH_V eCH);
UINT16 dvC341_ModeAdjusmenttHorzStart_Get(void);
UINT16 dvC341_ModeAdjusmenttVertStart_Get(void);
void dvC341_ModeAdjusmenttHorzStart_Set(UINT16 uiHStart);
void dvC341_ModeAdjusmenttVertStart_Set(UINT16 uiVStart);
void dvC341_ModeAdjusmenttEnableSetting(void);
void dvC341_ModeAdjusmenttDisableSetting(void);
void dvC341_Init_Mode_Adjustment_EEPROM_Setting(sTIMING_TABLE sTiming_Table);
BOOL dvC341_Find_Mode_Adjustment_table(eC341_CH_V eCH,sMODE_TABLE *m_FindModeTable,UINT8 *ucModeTableID);
UINT8 dvC341_Input3D_Timing_Get(const eC341_CH_V eCH);   //A70LV_Doulas_0196
void dvC341_BuildPosition_2K(eC341_CH_V eCH);      //A70LV_Doulas_0238
void dvC341_BuildScaler_2K(eC341_CH_V eCH);   //A70LV_Doulas_0284
void dvC341_Scale_EdgeMask(UINT8 ucEdgeMask,PsINPUT_TIMING_INFO psScalerInput,PsOUTPUT_TIMING_INFO psScalerOutput);      //ZU860_Doulas_0004
eSCALER_EXEC_CODE dvC341_EdgeMask_Set(const eC341_CH_V eCH,UINT8 ucEdgeMask);    //ZU860_Doulas_0004
void dvC341_OSD_3D_Mode_Set(const eC341_CH_V eCH,UINT8 ucVal);     //ZU860_Doulas_0049
BOOL dvC341_IsVsync120or100Hz_Get(const eC341_CH_V eCH);   //ZU860_Doulas_0104
BOOL dvC341_IsAspectRation16_9_Get(void);  //ZU860_Doulas_0119
BOOL dvC341_IsAspectRation16_10_Get(void);    //ZU860_Doulas_0120
void dvC341_SetInputPort_Output_Off(const eC341_CH_V eCH);     //ZU860_Doulas_0124
void dvC341_SetInputPort_Output_On(const eC341_CH_V eCH);    //A35G2_CDS_Simon_0005
void dvC341_Set_Color_Correction(eC341_CH_V eCH,
	INT32 blgain,		//blue gain
	INT32 grgain,		//green gain
	INT32 flgain,		//fleshness(skin) gain
	UINT8 mode		//0..blue/1..green/2..flesh/3..all
);      //EK816U_626U_Doulas_0004
UINT8 dvC341_OutputVsyncFreqGetForTwistOn(void);   //ZU860_Doulas_0138
UINT32 dvC341_Reg0_Read_Get(void);
void dvC341_Cmd_Test(UINT16 *puiTtlCnt, UINT16 *puiErrCnt);

#ifdef Low_Latency_All
void dvC341_Set_Flip(const eC341_CH_V eCH);	//ZU860_Clare_0152
void dvC341_Low_Latency_Set(const eC341_CH_V eCH,UINT8 ucVal);     //ZU860_Doulas_00152
#endif	/*Low_Latency_All*/

void dvC341_FlashRead(UINT32 ulAddress, UINT16 uiSize, UINT8 *pucData); //G100_Owen_0072
void dvC341_InputImage_Idle_Set(eC341_CH_V eCH, BOOL bEnable);    //G100_Owen_0048
void dvC341_RegTbl_to_Flash(UINT32 ulSFLAddr, UINT32 *pulRegTbl, UINT16 uiTblSize);   //G100_Owen_0060
void dvC341_CapturedIMG_Output_Set(BOOL bEnable);   //G100_Owen_0048
void dvC341_CapturedIMG_Reset(UINT32 ulSFLAddr, UINT32 *pulRegTbl, UINT16 uiTblSize);    //G100_Owen_0060 //G100_Owen_0059
void dvC341_Auto_Fill_Screen_Setting(BOOL bSetting); //G100_Steven_0050
void dvC341_PIPPBPPanelInfo_Setting(const eC341_CH_V eCH, sSCALER_PANEL_INFO *psPanelInfo);
void dvC341_PIPPBPPanelInfo_Get(const eC341_CH_V eCH, sSCALER_PANEL_INFO *psPanelInfo);
BOOL dvC341_SpecialTiming_Check_1280_960_85(const eC341_CH_V eCH, UINT32 *ulValue);
void dvC341_WaitOneVsyncTime(eC341_CH_V eCH); //A35G2_Simon_0113 //A35G2_BRC_Casper_0146
void dvC341_InputPort_Set(void);
void dvC341_PowerNormalDone(BOOL bDone);
UINT32 dvC341_CurrentBankGet(void);
void dvC341_CurrentBankSet(UINT32 CurrentBank);

INT8 dvC341_wait1_pivs(eC341_CH_V eCH_V);
INT8 dvC341_wait1_povs(eC341_CH_V eCH_V);
INT8 dvC341_wait1_povs_allch(void);
void dvC341_reset_vborx(int rx_num, int reg_lane, float fpclk, int reset);
void dvC341_set_sypol(int kind, int port);
void dvC341_set_idual(int ch_v, int port, int reset);
void dvC341_set_mct(int ch_v, int ipoff);
void dbmsg_ScalerDriver(const char *FunctionName, INT32 LineNum, char *str,  ...);
void dvC341_set_rtct_thru(void);
void dvC341_set_rtct_stop(void);
void dvC341_set_rtct_normal(void);
void dvC341_set_scfmfmt(eC341_CH_V ch_v, UINT8 FrameMemoryFormat);
void dvC341_set_lut_hshrnk(INT32 hsscl, eC341_CH_V ch_v);
void dvC341_set_lut_vshrnk(INT32 hsscl, eC341_CH_V ch_v);
void dvC341_Set_IP(const eC341_CH_V ch_v);
void dvC341_set_VBOInput(eC341_CH_V ch_v, INT32 port);
INT32 dvC341_Get_VBOInfo(INT32 port, INT32 vtotal, INT32 htotal, eC341_CH_V ch_v); //H30K_Doulas_0011
void dvC341Test(void);
void bkup_reg(void);
void check_wrlvlphct(void);
void check_gdsmsdly(void);
void check_DDR_Stress(void);
void dvC341_SetInputMemoryBuffer_4K(eC341_CH_V ch_v);
UINT8 dvC341_IS_3D_Enable(void);         //H30K_Doulas_0002
UINT16 dvC341_OutputActiveHW_1CH(void);
void dvC341_FrameDelaySet(UINT16 uiValue);        //H30K_Doulas_0010//H30K_Doulas_0003
void dvC341_In120_Out240_CalFunction(UINT8 ch_v,UINT16 *uiSYRDLY,UINT16 *uiFLDDLY); //H30K_Doulas_0012
void dvC341_FrontEnd_Timing_Set(UINT8 ucCH ,sVIDEO_TIMING2 sFrontEnd_Timing); //H30K_Doulas_0011
eSCALER_EXEC_CODE dvC341_3DPOFLD_Check(UINT8 *ucIsReconfig);  //H30K_Doulas_0035
UINT16 dvC341_Get_Panel_H_Active(void);   //H2PF_Simon_0107
BOOL dvC341_Timing_Detect_Fail_By_Front_End(eC341_CH_V ch_v); //H30K_Doulas_0050
void dvC341_ScalerFrameMemoryProcSet(UINT8 Enable30bit);
UINT8 dvC341_ScalerFrameMemoeyFormat_Get(void);
eSCAN_MODE dvC341_ScanMode_Get(int ch_v);

UINT8 HDR_Demo_SplitScreenMode_Get(void);
void HDR_Demo_SplitScreenMode_Set(UINT8 value);
void dvC341_degamma_HLG(int sdr_max, int hlg_max);
void dvC341_HDR_degamma_PQ(int sdr_max); //sdr_max 為影片metadata 紀錄的最大亮度 (MaxCCL or Mastering display luminance)
void degamma_PQ_strength(int sdr_max, double tone_mapping_strength);
void dvC341_degamma_SDREnhance(int sdr_max);
void dvC341_HLG_y_lut(int hlg_max);
void dvC341_Panelgamma_BT709_Gamma22(void);
void dvC341_Panelgamma_BT709_Gamma22_OnlyCH1(void);
void panelgamma_BT709(void);
void panelgamma_BT709_NA(void);
void dvC341_GamutBT2020ToREC709(void);
void GamutRGBBT2020ToREC709(void);
void dvC341_GamutBT2020ToREC709_OnlyCH1(void);
void GamutDCIP3ToREC709(void);
void dvC341_HLG_BT2020(void);
void dvC341_HLG_BT2020_OnlyCH1(void);
void dvC341_HDR_Init(void);
void dvC341_HDR_Config(void);
void dvC341_HDR_Disable(void);


#define dvC341Geo_Write(a,b,c) dvC341_Write(a,b,c*CH_WPBANK_OFFSET)
#define dvC341Geo_Read(a,c) dvC341_Read(a,c*CH_WPBANK_OFFSET)
#define dvC341Geo_Buffer_Flush() dvC341_Buffer_Flush()
#define dvC341Geo_WriteToBuffer(a,b,c) dvC341_WriteToBuffer(a,b,c*CH_WPBANK_OFFSET) //c = ch1, ch2...
#define dvC341Geo_BurstRead_FixedAdd(a,b,c) dvC341_BurstRead_FixedAdd(a,b,c)
#define dvC341Geo_BurstWrite_FixedAdd(a,b,c) dvC341_BurstWrite_FixedAdd(a,b,c)
#define dvC341Geo_BurstRead_AddInc(a,b,c) dvC341_BurstRead_AddInc(a,b,c)
#define dvC341Geo_BurstWrite_AddInc(a,b,c) dvC341_BurstWrite_AddInc(a,b,c)

#endif
