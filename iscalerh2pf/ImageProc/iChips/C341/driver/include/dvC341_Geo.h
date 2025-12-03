#ifndef DVC341_GEO_H
#define DVC341_GEO_H

#include "Common.h"
#include "utilCommon.h"
#include "dvC341.h"

//#define DEF_WARPBLOCK_24BIT_PROCESS  //ori is 30bit

//#define WARP_THROUGH_MODE

//===== Warp Param =====//
// Cascade
#define DEF_NUM_CASC	            3
#define DEF_CASC_OLD_TBL	        0
#define DEF_CASC_NEW_TBL	        1

// Warp OSD insert position
#define DEF_WP_OSD_INS	            0// 0 : Input, 1 : Output

//===== Warp Param =====//
#define DEF_WP_LIM_MODE		        0  // 0 : WpLimit OFF, 1 : WpLimit ON
#define DEF_WP_SPACE		        32 // 32, 64
#define DEF_WP_SPACE_BIT	        ((DEF_WP_SPACE == 64) ? 6 : (DEF_WP_SPACE == 32) ? 5 : 4)
#define DEF_WP_SPACE_16P		    16 // 32, 64
#define DEF_WP_SPACE_32P		    32 // 32, 64
#define DEF_WP_SPACE_BIT_16P        4
#define DEF_WP_SPACE_BIT_32P        5

//===== 4 Corner Based =====//
#define DEF_HW_MAX			        3840
#define DEF_VW_MAX			        2400

#define DEF_HW_MAX_GRD		        (DEF_HW_MAX/DEF_WP_SPACE + 1)//121
#define DEF_VW_MAX_GRD		        (DEF_VW_MAX/DEF_WP_SPACE + 1)//76

#define DEF_WP_OUT_HGRD		        3// Warptable horizontal grid of outside // fix
#define DEF_WP_OUT_VGRD		        27// Warptable vertical grid of outside
//#define DEF_WP_OUT_HW		        (DEF_WP_OUT_HGRD << DEF_WP_SPACE_BIT)
//#define DEF_WP_OUT_VW		        (DEF_WP_OUT_VGRD << DEF_WP_SPACE_BIT)

#define DEF_GV_PX_HGRD		        (DEF_WP_OUT_HGRD + DEF_HW_MAX_GRD + DEF_WP_OUT_HGRD)// 3 + 121 + 3 = 127
#define DEF_GV_PX_VGRD		        (DEF_WP_OUT_VGRD + DEF_VW_MAX_GRD + DEF_WP_OUT_VGRD)// 27+ 76 + 27 = 130

#define DEF_HW_DUMMY_GRD	        3// 4096 x 2160 : 1, 3840x2160 : 3
#define DEF_HW_DUMMY		        (DEF_HW_DUMMY_GRD << DEF_WP_SPACE_BIT)// Dummy area(Correction to the outside)

typedef struct {
	int x;
	int y;
} coord_t;


// y= ax + b
typedef struct {
	float a;
	float b;
} line_coef_ab_t;


typedef struct {
	int st;
	int end;
} pos_t;



//===== Initial value =====//
#define DEF_PB_000_INIT	            0
#define DEF_PB_100_GRID_MV_INIT	    5
#define DEF_PB_200_INIT	            0


#define DEF_BLENDING_EDGE_HW	    200// = PM_EGB_HW
#define DEF_BLENDING_EDGE_VW	    200// = PM_EGB_VW


#define DEF_PB_240_INIT	            0
#define DEF_PB_340_INIT	            0
#define DEF_PB_390_INIT	            0
#define DEF_BLENDING_GAIN_GAMMA	    2.2f// = PM_EGB_GAMMA
#define DEF_OUTPUT_GAMMA	        1.0f// = PM_GAMMA
#define DEF_BLENDING_BIAS_GAMMA	    2.2f// = PM_EBIAS_GAMMA
#define DEF_PB_500_INIT	            0
#define DEF_PB_600_INIT	            0

//===== Grid parameter =====//
#define DEF_GRID_SIZE		        16// DEF_GRD_SIZE >= 1
#define DEF_GRID_SIZE_O		        16

//===== 4-Cursor =====//
#define DEF_4CURSOR_SIZE	        16


//Linefeed Base
#define EGBDBD_LINEFEED_BASE  1024   //DBD blending (blending gain)
#define EBIAS_LINEFEED_BASE   1024   //blacklevel (blending bias)
#define OSD_LINEFEED_BASE     128

//MWI Definition
#define OSD_BIAS_MWI		0x01		//linefeed width is EBIASMWI[7:0]x1024 bytes, but 1 byte contain 2 pixels
#define OSD_DBD_8BIT_MWI	0x02		//linefeed width is EGBDBDMWI[7:0]x1024 bytes
#define DEF_MWI			    0x20		// 0x20=32, 32*64=2048
#define DEF_EBIASMWI 		OSD_BIAS_MWI//0x04			//G100_Doulas_0027


//===== Warp Lim =====//
//#define DEF_WPLIMANG_H		        1.00f	// Limitation of H-slope : tan(45deg)
//#define DEF_WPLIMANG_V		        5.67f	// Limitation of V-slope : tan(80deg)
//#define DEF_WPLIMVSH_L		        853		// Limitation of local V-shrink rate : //((Def_Wp_Space==64) ? {64/0.3 *8 = 1706} : {32/0.3 *8 = 853})
//#define DEF_WPLIMVSH_A		        512		// Limitation of averate V-shrink rate : //((Def_Wp_Space==64) ? {64/0.5 *8 = 1024} : {32/0.5 *8 = 512})
//#define DEF_WPLIMHSH		        1536 	// Limitation of H-shrink rate : //((Def_Wp_Space==64) ? {64/(1/6) *8 = 3072} : {32/(1/6) *8 = 1536})
//#define DEF_WPLIMITV_H	            2	    // Limitation of horizontal interval of grid : DEF_WPLIMITV_H > 0
//#define DEF_WPLIMITV_V	            2	    // Limitation of vertical interval of grid : DEF_WPLIMITV_V > 0

//===== Edgeblend Bias =====//
#define DEF_EBIAS_STEP	            1

//===== Edgeblend Gamma =====//
#define DEF_EGBGM_STEP	            0.005

//===== Output Gamma =====//
#define DEF_OGM_NUM	                33 //Interpolation mode, Thirty-One Table Mode
#define DEF_OGM_COEF_STEP	        0.005

//===== Edgeblend Bias Gamma =====//
#define DEF_EBSGM_STEP	            0.005


//===== 4-Cursor =====//
#define Def_4CURSOR_Size	8

//===== Warp Param =====//
#define Def_WpLimitMode		1// 0 : WpLimit OFF, 1 : WpLimit ON
//#define Def_Wp_Space		32// 16, 32
//#define Def_Wp_Space_Bit	5//((Def_Wp_Space==32) ? 5 : 4)

#define Def_HW_Max			2048
#define Def_VW_Max			1200
#define Def_HW_Max_GRD		133// = (Def_HW_Max + Def_Wp_Dummy * 2 + Def_Wp_Space - 1) / Def_Wp_Space + 1	[rounddown]
#define Def_VW_Max_GRD		76// = (Def_VW_Max + Def_Wp_Space - 1) / Def_Wp_Space + 1	[rounddown]
#define Def_HST_Min			4
#define Def_HW_Min			64
#define Def_VST_Min			4
#define Def_VW_Min			4

// divide by 0
#define Def_DIV0 0xFFFFFFFF

// Buffer Length
#define Def_BUF_MAX	256

//===== Address =====//
//#define DEF_EBIASSAD 0x06800000		//G100_Doulas_0027
//#define DEF_EBIASMWI 0x04				//G100_Doulas_0027


//===== Gamma ====//
extern float PM_EBIAS_GAMMA[3];
extern int PM_EBIAS_GAMMA_COLOR;
extern char PM_EBIAS_GMDT2[16][3][16];



//From WPLG Project
#define DEF_MAX_LONG                0x7fffffffL
#define DEF_PI			            3.14159265f
#define	DEF_WPDMYHW		            96

//===== LUT =====//
//#define DEF_LUTSYM      6           // LUT symbol default
#define DEF_LUTCUT_MAX  1.2f        // LUT cutoff max
//#define DEF_LUTCUT_MIN  0.1f        // LUT cutoff min
#define DEF_LUTWIN_MAX  2.0f        // LUT window max
//#define DEF_LUTWIN_MIN  0           // LUT window min
//End from WPLG Project


//===== Polation Table =====//
#define POLATION_MAX 10
#define POLATION_NUMBER 24
static const UINT8 m_sPolationTable[POLATION_MAX][POLATION_NUMBER] =
{
    //0 //lutset_6s_c03w00
    {0x02,0x03,0x04,0x04,0x05,0x06,0x07,0x08,0x09,0x09,0x0a,0x0b,0x0c,0x0c,0x0d,0x0e,0x0e,0x0f,0x0f,0x11,0x12,0x13,0x13,0x14},
    //1 //lutset_6s_c04w00
    {0xfe,0xff,0xff,0x00,0x01,0x02,0x03,0x05,0x06,0x08,0x09,0x0b,0x0c,0x0e,0x0f,0x10,0x12,0x13,0x14,0x15,0x17,0x16,0x18,0x16},
    //2 //lutset_6s_c05w00
    {0xfa,0xfa,0xfa,0xfb,0xfc,0xfd,0xff,0x00,0x02,0x04,0x06,0x09,0x0c,0x0e,0x11,0x11,0x16,0x18,0x1a,0x1c,0x1e,0x1f,0x1e,0x1e},
    //3 //lutset_6s_c06w00
    {0xfb,0xf9,0xf8,0xf8,0xf7,0xf8,0xf9,0xfb,0xfc,0xff,0x01,0x04,0x08,0x0c,0x10,0x13,0x18,0x1d,0x21,0x24,0x27,0x27,0x28,0x24},
    //4 //lutset_6s_c07w00
    {0x00,0xff,0xfc,0xfa,0xf8,0xf7,0xf6,0xf6,0xf7,0xf8,0xfb,0xfe,0x01,0x06,0x0c,0x12,0x17,0x1d,0x23,0x28,0x2d,0x2f,0x30,0x30},
    //5 //lutset_6s_c08w00
    {0x05,0x04,0x02,0x00,0xfe,0xfb,0xf8,0xf5,0xf5,0xf5,0xf6,0xf8,0xfc,0x00,0x05,0x0d,0x13,0x1a,0x21,0x28,0x2d,0x32,0x36,0x3c},
    //6 //lutset_6s_c09w00
    {0x06,0x07,0x06,0x05,0x03,0x00,0xfe,0xfa,0xf8,0xf6,0xf5,0xf5,0xf7,0xfb,0x00,0x06,0x0e,0x16,0x1e,0x26,0x2d,0x32,0x36,0x40},
    //7 //lutset_6s_c10w00
    {0x02,0x04,0x06,0x07,0x07,0x05,0x03,0x00,0xfd,0xf9,0xf6,0xf4,0xf4,0xf6,0xfa,0x00,0x08,0x11,0x1b,0x25,0x2e,0x37,0x3c,0x40},
    //8 //lutset_6s_c10w10
    {0x00,0x00,0x00,0x01,0x01,0x01,0x01,0x00,0xff,0xfd,0xfb,0xf9,0xf8,0xf9,0xfc,0x00,0x06,0x0f,0x1a,0x26,0x32,0x3a,0x3e,0x40},
    //9 //lutset_6s_c10w15
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xfe,0xfd,0xfc,0xfc,0xfd,0x00,0x05,0x0d,0x18,0x23,0x2e,0x38,0x3e,0x40},
};

#define DTCT_16PIEXL_ALWAYS_USE_TABLE_A
#define DTCT_BASE_ACCESS_ENABLE_ALWAYS_TABLE_A 0x2102

#define DTCT_BASE_ACCESS_ENABLE_32PIEXL_A   0x2200
#define DTCT_BASE_ACCESS_ENABLE_16PIEXL_A   0x2202 // [2:0]DTMD=2(16dot), [5:4]DTSEL=1(TableB), [7:6]TBLMD=2, [10:8]CPUACEN=1(TableA), [13]WPOUTEN=1
#define DTCT_BASE_ACCESS_ENABLE_32PIEXL_B   0x2110
#define DTCT_BASE_ACCESS_ENABLE_16PIEXL_B   0x2112 // [2:0]DTMD=2(16dot), [5:4]DTSEL=0(TableA), [7:6]TBLMD=2, [10:8]CPUACEN=2(TableB), [13]WPOUTEN=1

#define DTCT_BASE_ACCESS_DISABLE_32PIEXL_A  0x2010
#define DTCT_BASE_ACCESS_DISABLE_16PIEXL_A  0x2012 // [2:0]DTMD=2(16dot), [5:4]DTSEL=0(TableA), [7:6]TBLMD=2, [10:8]CPUACEN=0, [13]WPOUTEN=1
#define DTCT_BASE_ACCESS_DISABLE_32PIEXL_B  0x2000
#define DTCT_BASE_ACCESS_DISABLE_16PIEXL_B  0x2002 // [2:0]DTMD=2(16dot), [5:4]DTSEL=1(TableB), [7:6]TBLMD=2, [10:8]CPUACEN=0, [13]WPOUTEN=1

#define DTCT_BASE_ACCESS_TABLE_A     (0x0000)
#define DTCT_BASE_ACCESS_TABLE_B     (0x0010)

#define WARPING_DISTORTION_CORRECTION_TABLE_A DTCT_BASE_ACCESS_TABLE_A
#define WARPING_DISTORTION_CORRECTION_TABLE_B DTCT_BASE_ACCESS_TABLE_B

#define RTCT_WARP_TRANSFER_DISABLE  0xFFF8
#define RTCT_WARP_TRANSFER_ENABLE   0x0005
#define RTCT_WARP_WPPOVS_ENABLE     0x0007

#define DTCT_BASE_WPOUTMD_IMPROVEMENT 0

//for low latency mode     //H2PF_Simon_0150
#define SYRDLY_REG_FOR_LIMITED_WARP (230)
#define FLDDLY_REG_FOR_LIMITED_WARP (229)

//************************* Enum Start *************************//

typedef enum
{
    eC341GEO_EXEC_CODE_PASS,                   /* pass */
    eC341GEO_EXEC_CODE_FAIL,                   /* general fail indication */
    eC341GEO_EXEC_CODE_FATAL,                  /* fatal error; halt application */
    eC341GEO_EXEC_CODE_PANEL_ID_NOT_FOUND,
    eC341GEO_EXEC_CODE_PANEL_TABEL_INDEX_NOT_FOUND,
}eC341GEO_EXEC_CODE;


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
10  DMA transfer (Serial Flash read to internal register write)
    Format of data on the serial Flash: register address, register value, register
    address, register value …
11-13 Reserved
14  Read operation with SFLGCMD (store read data to SFLGDT with the lowest byte
    first): with SFLCNT, set the number of bytes between 1 and 4.
15  Write operation with SFLGCMD (send out write data from SFLGDT with the lowest
byte first): with SFLCNT, set the number of bytes between 1 and 4.*/

typedef enum
{
    eC341GEO_SFLCT_IDEL            = 0,
    eC341GEO_SFLCT_DMA_REG2FLASH   = 1,
    eC341GEO_SFLCT_DMA_FLASH2REG   = 2,
    eC341GEO_SFLCT_DMA_FLASH2REGF  = 10, //A70LV_Larry_0174

}eC341GEO_SFLCT; //A70LV_Larry_0139

/*
0 DDR3-SDRAM
1 OSD Character Buffer
2 OSD Color Pallete
3 De-inter lacer LUT
4 Distortion Correction LUT
5 Output Image Gamma LUT
6 Output Image Uniformity LUT
7 Adaptive Scale Interpolation Filter LUT
8 Edge Blend Gain Gamma LUT
9 Edge Blend Bias LUT 7 0x2E
A Cursor
*/

typedef enum
{
    eC341GEO_SFLREGSEL_DDR     = 0x00,
    eC341GEO_SFLREGSEL_OCB     = 0x01,
    eC341GEO_SFLREGSEL_OCP     = 0x02,
    eC341GEO_SFLREGSEL_DILL    = 0x03,
    eC341GEO_SFLREGSEL_DCL     = 0x04,
    eC341GEO_SFLREGSEL_OIGL    = 0x05,
    eC341GEO_SFLREGSEL_OIUL    = 0x06,
    eC341GEO_SFLREGSEL_ASIF    = 0x07,
    eC341GEO_SFLREGSEL_EBGL    = 0x08,
    eC341GEO_SFLREGSEL_EBBL    = 0x09,
    eC341GEO_SFLREGSEL_CURSOR  = 0x0A,
    eC341GEO_SFLREGSEL_NA      = 0xFF, //A70LV_Larry_0174

}eC341GEO_SFLREGSEL; //A70LV_Larry_0139

typedef enum
{
    eC341_WARP_MODE_2x2,
    eC341_WARP_MODE_3x3,
    eC341_WARP_MODE_5x5,
    eC341_WARP_MODE_9x9,
    eC341_WARP_MODE_17x17,
}eC341_WARP_MODE;

typedef enum
{
    eC341_WARP_CROSSHATCH_MODE_OFF,
    eC341_WARP_CROSSHATCH_MODE_SINGLE,
    eC341_WARP_CROSSHATCH_MODE_OVERLAY,
    eC341_WARP_CROSSHATCH_MODE_LAST,
}eC341_WARP_CROSSHATCH_MODE;

typedef enum
{
    eC341_COLOR_RED,
    eC341_COLOR_GREEN,
    eC341_COLOR_BLUE,
    eC341_COLOR_LAST,
}eC341_COLOR;

typedef enum
{
	eC341_WARP_OK,
	eC341_WARP_ERR_ANG_H,// Error of H-slope
	eC341_WARP_ERR_ANG_V,// Error of V-slope
	eC341_WARP_ERR_VSH_LOC,// Error of local V-shrink rate
	eC341_WARP_ERR_VSH_AVE,// Error of average V-shrink rate
	eC341_WARP_ERR_HSH,// Error of H-shrink rate
	eC341_WARP_ERR_OUTSIDE,// Error of outside of ACT
	eC341_WARP_ERR_INTERVAL,// Error of grid interval
	eC341_WARP_NGZ,
	eC341_WARP_CURPOS,
	eC341_WARP_ERR// Error of other reasons

}eC341_WARP_ERROR;

typedef enum
{
    eC341_WARP_DIRECTION_UP,
    eC341_WARP_DIRECTION_DOWN,
    eC341_WARP_DIRECTION_LEFT,
    eC341_WARP_DIRECTION_RIGHT,
    eC341_WARP_DIRECTION_LAST,
}eC341_WARP_DIRECTION;

//************************* Enum End *************************//


//===================== Struct Start ===================//

// 4-corner Keystone
typedef struct
{
    INT16 tl_x, tl_y, tr_x, tr_y, bl_x, bl_y, br_x, br_y;
}sWPPOS4C, *PsWPPOS4C;

typedef struct
{
    float x, y, z;
}s3DPOS, *Ps3DPOS;

typedef struct
{
    float x, y;
}s2DPOS, *Ps2DPOS;



//************************* Enum End *************************//


//===================== Struct Start ===================//
#if 0
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
#ifdef Low_Latency_All
    UINT8				ucLow_Latency;	//ZU860_Clare_0152
#endif	/*Low_Latency_All*/
}sOUTPUT_TIMING_INFO, *PsOUTPUT_TIMING_INFO;


typedef struct
{
	DOUBLE 		        dFrameRate;
	UINT16		        uiHStart;
	UINT16		        uiHActive;
	UINT16		        uiVStart;
	UINT16		        uiVActive;
    //Input Port Measure Reference CLK by PICLK
	UINT16		        uiHTotal;           //g_ihcycl_i
    //Input Port Measure Reference CLK by MCLK.Depends on B17_DIFCTCH bit[7]
	UINT16		        uiHTotal_MCLK;      //g_ihcycl_m
	UINT16		        uiVTotal;           //g_ivcycl_i

}sINPUT_TIMING_INFO, *PsINPUT_TIMING_INFO;



typedef struct
{
    ePANEL_ID                   ePanelId;
    sOUTPUT_TIMING_INFO         sOutputTimingInfo;
}sPANEL_INFO, *PsPANEL_INFO;
#endif


#if 0
typedef struct
{
    UINT32 ulX;
    UINT32 ulY;
}sCOORDINATE_T, *PsCOORDINATE_T;

typedef struct
{
    float fX;
    float fY;
}sCOORDINATE_F, *PsCOORDINATE_F;

typedef struct
{
	float fA;
	float fB;
	float fC;
	float fD;
}sLINE_COEF_T, * PsLINE_COEF_T;
#endif /* 0 */




//===== Edgeblend Bias ====//
extern int                  PM_EBIAS_EN;
extern int                  PM_EBIAS_SEL_CUR_X;
extern int                  PM_EBIAS_SEL_CUR_Y;
extern coord_t              PM_EBIAS_CUR[2][2];
extern int                  PM_EBIAS_PLT;
extern int                  PM_EBIAS[16][3];
extern int                  PM_EBIAS_AREA_TEST;
extern int                  PM_EBIAS_CUR_MV;
extern int                  PM_EBIAS_COLOR;
extern int                  PM_BOARD;

//===== parameters 1 =====//
extern int                  PS_WP_HW;
extern int                  PS_WP_VW;



typedef struct
{
    BOOL                        bInit;
    ePANEL_ID                   ePanelTimingId;           //Pass from up layer to get panel output timing
    ePANEL_ID                   eLocalPanelTimingId;      //T100_Simon_0044

    UINT8						ucPanelIndex;             //To save panel table index
    sOUTPUT_TIMING_INFO         sOutputTimingInfo;

    UINT32                      ulMclko_Freq;
    UINT8                       ucBypassMode;


    //===== parameters 1 =====//
    UINT16                      uiPanelHst;
    UINT16                      uiPanelVst;
    UINT16                      uiPanelHW;          // PS_WP_HW, Panel Width, 3840
    UINT16                      uiPanelVW;          // PS_WP_VW, Panel Height, 2160
    UINT16                      uiPixelGridHW;      // PS_WP_HW_GRD, Grid Width / 32 pixel mode or 64 pixel mode = how many interpoation grid size
    UINT16                      uiPixelGridVW;      // PS_WP_VW_GRD, Grid Height / 32 pixel mode or 64 pixel mode = how many interpoation grid size
    float                       fPHWHF;
    float                       fPVWHF;


    UINT8                       ucWarpColor;        // GV_WP_TABLE_COLOR, in 32 pixel space, there A,B,C three table can be used.Refer to B14_DTCT
                                                    //                    in 64 pixel space, there A,B,C,D fourc table can be used.
    BOOL                        bWPTblSel;

    UINT8                       POLATION_H;
    UINT8                       POLATION_V;
    UINT8                       m_cWarpFilterSelect_H;
    UINT8                       m_cWarpFilterSelect_V;
    BOOL                        m_cWarpAutoFilter;

    s3DPOS                      pos_i[4];
    s3DPOS                      pos_r;
    s3DPOS                      pos_a[4];
    s2DPOS                      pos_o[4];

    float                       faCursorDefInX[17];             // PS_CUR_DEF_IX, real coordinate in the 3840x2160 canvas
    float                       faCursorDefInY[17];             // PS_CUR_DEF_IY, real coordinate in the 3840x2160 canvas
    float                       faCursorDefOutX[17];            // PS_CUR_DEF_OX, real coordinate in the 3840x2160 canvas
    float                       faCursorDefOutY[17];            // PS_CUR_DEF_OY, real coordinate in the 3840x2160 canvas

//    UINT16                      uiIACT_HW;// PM_IACT_HW;
//    UINT16                      uiIACT_vW;// PM_IACT_VW;

#if 0
    //===== Warping ====//

    sCOORDINATE_F               saGridPos[DEF_NUM_CASC][17][17];// PM_GRID, Cascade
    sCOORDINATE_F               saPrevGridPos[17][17];          // PM_GRID_OLD
#endif /* 0 */



    // EdgeBlending Gain
    UINT16                      uiBlendingEdgeHW;               // PM_EGB_OHW
    UINT16                      uiBlendingEdgeVW;               // PM_EGB_OVW
    BOOL                        bBlendingGainEnable;            // PM_EGB_EN
    BOOL                        bBlendingGainEnable_T;          // PM_EGB_TEN
    BOOL                        bBlendingGainEnable_B;          // PM_EGB_BEN
    BOOL                        bBlendingGainEnable_L;          // PM_EGB_LEN
    BOOL                        bBlendingGainEnable_R;          // PM_EGB_REN

    // EdgeBlending Bias
    BOOL                        bBlendingBiasEnable;            // PM_EBIAS_EN
    UINT8                       ucBlendingBiasCursorX;          // PM_EBIAS_SEL_CUR_X
    UINT8                       ucBlendingBiasCursorY;          // PM_EBIAS_SEL_CUR_Y
    //sCOORDINATE_T               saBlendingBiasCursorPos[2][2];  // PM_EBIAS_CUR

    UINT8                       ucaBlendingBiasPalleteColor[16][eC341_COLOR_LAST]; // PM_EBIAS

    BOOL                        bBlendingBiasAreaTest;              // PM_EBIAS_AREA_TEST
    UINT16                      uiBlendingBiasCursorMoveStep;       // PM_EBIAS_CUR_MV
    UINT8                       ucBlendingBiasColorSel;             // PM_EBIAS_COLOR

    // Blending Gain Gamma
    BOOL                        bBlendingGainGammaEnable;           // PM_EGB_GAMMA_EN
    float                       faBlendingGainGamma[eC341_COLOR_LAST];  // PM_EGB_GAMMA, R, G, B
    UINT8                       ucBlendingGainGammaColorSel;        // PM_EGB_GAMMA_COLOR
    // Output Gamma
    BOOL                        bOutputGammaEable;                  // PM_GAMMA_EN, Output Gamma
    float                       faOutputGamma[DEF_OGM_NUM][3];      // PM_GAMMA, 33 output gamma point
    UINT16                      uiaOutputGammaPixel[DEF_OGM_NUM];   // PM_GAMMA_PIX, Each point pixel pos
    UINT16                      uiOutputGammaPixelSel;              // PM_GAMMA_PIX_SEL
    UINT8                       ucOutputGammaColor;                 // PM_GAMMA_COLOR

    // Blending Bias Gamma
    float                       faBlendingBiasGamma[eC341_COLOR_LAST];              // PM_EBIAS_GAMMA
    UINT8                       ucBlendingGammaColor;                               // PM_EBIAS_GAMMA_COLOR
    UINT16                      uiBlendingBiasGammaData[16][eC341_COLOR_LAST][16];  //PM_EBIAS_GMDT[16][3][16];

    INT16                       iDummyHstReset;                                    // PS_DUMMY_HST_REST
    INT16                       iDummyHendGridReset;                                // PS_DUMMY_HEND_GRD_REST

    //===== Warp Table parameters ====//
    float                       faPosX[DEF_NUM_CASC][DEF_GV_PX_HGRD+1][DEF_GV_PX_VGRD+1];// GV_PX, cascade
    float                       faPosY[DEF_NUM_CASC][DEF_GV_PX_HGRD+1][DEF_GV_PX_VGRD+1];// GV_PY, cascade


    //===== OSD Cursor parameters ====//
    UINT16                      uiNumHGrid;  // GV_NUM_HGRID, number grid, related to warp mode 2x2, 3x3...
    UINT16                      uiNumVGrid;  // GV_NUM_VGRID, number grid, related to warp mode 2x2, 3x3...
    UINT16                      uiNumHGrid_M1;//GV_NUM_HGRID_m1, for calc_h_line_coef(), calc_v_line_coef()
    UINT16                      uiNumVGrid_M1;//GV_NUM_VGRID_m1, for calc_h_line_coef(), calc_v_line_coef()
    UINT16                      uiNumHGrid_M2;//GV_NUM_HGRID_m2, for calc_h_line_coef(), calc_v_line_coef()
    UINT16                      uiNumVGrid_M2;//GV_NUM_VGRID_m2, for calc_h_line_coef(), calc_v_line_coef()
    UINT16                      uiNumHGrid_P1;//GV_NUM_HGRID_p1, for calc_h_line_coef(), calc_v_line_coef()
    UINT16                      uiNumVGrid_P1;//GV_NUM_VGRID_p1, for calc_h_line_coef(), calc_v_line_coef()




#if 0
    //===== Warp Parameters ====//
    sLINE_COEF_T                saWarpVerLine[17][16];// GV_WP_VER_LINE
    sLINE_COEF_T                saWarpHorLine[16][17];// GV_WP_HOR_LINE
    sLINE_COEF_T                saWarpHorLineMove[16];// GV_WP_HOR_LINE_MV
    sLINE_COEF_T                saWarpVerLineMove[DEF_GV_PX_HGRD][16];// GV_WP_VER_LINE_MV
    sCOORDINATE_F               saCalPoint[17][17];// GV_CP
    BOOL                        baHGridCalEnable[17];// GV_HGRID_CLC_EN[17]
    BOOL                        baVGridCalEnable[17];// GV_VGRID_CLC_EN[17]
    //===== Warp_2x2 Parameters ====//
    float                       faPjcA[3];          // GV_PJC_A
    float                       faPjcB[3];          // GV_PJC_B
    float                       faPjcC[3];          // GV_PJC_C
#endif /* 0 */

    //===== Gamma Parameters ====//
    UINT16                      uiTmpGammaTable[1024];// GV_TEMP_GAMMA_TABLE[1024]

    //===== Register parameters =====//
    UINT16                      uiRegRTCT;          // GV_RTCT
    UINT16                      uiRegEGBCT;         // GV_EGBCT;


    //===== Test Pattern ====//
    //eC341GEO_WARP_CROSSHATCH_MODE  ePatternGen;        //PM_PATGEN;
    BOOL                        bEdgeMarkerEnable;  //PM_EGBMK_EN;
    BOOL                        bOfillEnable;       //PM_OFILL_EN;

    UINT8                       ucRegISYCT;  //A35G2_CDS_Simon_0003

    UINT8                       ucMagnifyOSD2X;
    UINT8                       ucIsDefaultWarpTable;

}sDRV_C341GEO_INFO, *PsDRV_C341GEO_INFO;

//======================================================

BOOL dvC341Geo_SemaphoreTake(BOOL cEnable, const char *pcSemaphore);

#if 0
UINT32 dvC341Geo_Read(const UINT32 ulAddr, const UINT8 ucBankOffset);
void dvC341Geo_Write(UINT32 ulAddr, UINT32 ulData, const UINT8 ucBankOffset);
void dvC341Geo_Buffer_Flush(void);
void dvC341Geo_WriteToBuffer(const UINT32 ulAddr, UINT32 ulData, const UINT8 ucBankOffset);
void dvC341Geo_BurstRead_FixedAdd(const UINT32 ulAddr, UINT16 uiSize, UINT8 *pucData);
void dvC341Geo_BurstWrite_FixedAdd(const UINT32 ulAddr, UINT16 uiSize, UINT8 *pucData);
void dvC341Geo_BurstRead_AddInc(const UINT32 ulAddr, UINT16 uiSize, UINT8 *pucData);
void dvC341Geo_BurstWrite_AddInc(const UINT32 ulAddr, UINT16 uiSize, UINT8 *pucData);
#endif /* 0 */

#ifdef Low_Latency_All
eC341GEO_EXEC_CODE dvC341Geo_Init(const ePANEL_ID ePanelId, eLOW_LATENCY_MODE bbLow_Latency);	//ZU860_Clare_0152//A70LV_Doulas_0036
#else
eC341GEO_EXEC_CODE dvC341Geo_Init(const ePANEL_ID ePanelId);      //A70LV_Doulas_0036
#endif	/*Low_Latency_All*/
void dvC341Geo_InputSwapSet(UINT8 ucVal);
void dvC341Geo_TestPatternSet(UINT8 ucVal);
void dvC341Geo_TestMode(void);
void dvC341Geo_V_Start_Checking(void);  //A70LV_Doulas_0042
void dvC341Geo_ForcedSyncResetValueSet(UINT8 ucForcedSyncReset,UINT16 uiH_Total,UINT16 uiV_Total);     //A70LV_Doulas_0079
void dvC341Geo_OutputEnableSet(UINT8 ucEnable);
UINT8 dvC341Geo_OutputEnableGet(void);
eC341GEO_EXEC_CODE dvC341Geo_Change_Panel(const ePANEL_ID ePanelId);       //A70LV_Doulas_0098
void dvC341Geo_FlashEraseSector(UINT32 ulSflAd, UINT32 ulDataSize); //A70LV_Larry_0139
void dvC341Geo_DMA_Transfer(UINT32 ulAddress, UINT32 ulSize, UINT8 ucRegsel, UINT32 ulDmaCnt); //A70LV_Larry_0139
void dvC341Geo_FlashWrite(UINT32 ulAddress, UINT16 uiSize, UINT8 *pucData); //A70LV_Larry_0174
void dvC341Geo_FlashRead(UINT32 ulAddress, UINT16 uiSize, UINT8 *pucData); //A70LV_Larry_0174
void dvC341Geo_FrmaeDelaySet(UINT8 ucValue);       //A70LV_Doulas_0154
UINT16 dvC341Geo_Output_V_Total_Get(void);   //A70LV_Doulas_0154
void dvC341Geo_RegDump(void);      //A70LV_Doulas_0170
UINT8 dvC341Geo_OutputChanged_Get(void);        //A70LV_Doulas_0307
void dvC341Geo_OutputChanged_Set(UINT8 ucVal);        //A70LV_Doulas_0307
void dvC341Geo_ColorUniformity_Control(UINT8 ucEnable, UINT8 ucCPUAccessEnable);
UINT8 dvC341Geo_ColorUniformityEnable_Get(void);    //A65_OPTOMA_CL_0001  //A35G2_CDS_CL_0001
void dvC341Geo_ColorUniformityEnable_Set(UINT8 ucVal);
void dvC341Geo_COMS_Output_Set(UINT8 ucEnable);     //A70LV_Doulas_0361
UINT8 dvC341Geo_OutputV_FreqGet(void);     //A70LV_Doulas_0367
void dvC341Geo_Test(UINT16 *puiTtlCnt, UINT16 *puiErrCnt);
#ifdef Low_Latency_All
void dvC341Geo_Low_Latency_Set(eLOW_LATENCY_MODE eLLMode);	//ZU860_Clare_0152
eLOW_LATENCY_MODE dvC341Geo_Low_Latency_Get(void);	//ZU860_Clare_0152
#endif	/*Low_Latency_All*/

void dvC341Geo_SetEdgebldBiasCursorEnable(UINT8 ucEnable);
void dvC341Geo_SetEdgebldBiasCursorPos( int selx, int sely, int dx, int dy );
void dvC341Geo_EdgebldBiasEnable(INT8 cEnable);
void dvC341Geo_SetEdgebldBiasPalette( int plt, int r, int g, int b );
void dvC341Geo_BlackLevel_AreaWrite( int plt_sel, coord_t P1, coord_t P2, coord_t P3, coord_t P4 , int iDataWrite);
void WPLT2( int plt_sel, pos_t x, int y );
line_coef_ab_t CalcCoef( const coord_t *p0, const coord_t *p1 );
pos_t CalcLineX( const int *y_crt, const coord_t *p_b, const coord_t *p_t, const line_coef_ab_t *coef );
pos_t XMinMax( const pos_t x[], int num );
pos_t YMinMax( const coord_t p[], int num );
char *FileNameEBAreaBak( int num );
int LoadEgbBiasArea( char fn[] ) ;
void dvC341Geo_CalcEdgebldBiasGammaTable( int color, int plt, int allplt );
void dvC341Geo_EdgebldBiasGammaTable( int plt );
void dvC341Geo_EdgebldBiasGammaTableClear( void );
int dvC341Geo_LoadEgbBiasArea( char fn[] );
void dvC341Geo_Blacklevel_Palette_Set(INT8 cPaletteIdx, UINT8 cRed, UINT8 cGreen, UINT8 cBlue);

char HexToChar( int val );
int CharToHex( char ch );
void dvC341Geo_Dummy(void);
void dvC341Geo_SetOSDMode(bool bFront);
void dvC341Geo_OSDACTStartConfig(bool bFront);
UINT32 dvC341Geo_V_Start_Get(void);    //G100_Owen_0078
//void dvC341Geo_ConfigMemAD(void);
void dvC341Geo_ISYCT_Set(UINT8 uiRegValue);  //A35G2_CDS_Simon_0003
UINT8 dvC341Geo_ISYCT_Get(void);  //A35G2_CDS_Simon_0003
void dvC341Geo_AdaptiveScaleFilterLutInit(void); //A65_OPTOMA_CL_0018  //A35G2_Simon_0115
void dvC341Geo_AdaptiveScaleFilterLutEnable(UINT8 ucEnable);  //A35G2_Simon_0115
void dvC341Geo_WPLT2( int plt_sel, pos_t x, int y );   // bias area file
void dvC341Geo_Freeze(UINT8 ucEnable);
UINT32 dvC341Geo_FrameMemoryDrawStartAddrGet(void);
void dvC341Geo_ConfigOutput(void);
UINT8 dvC341Geo_IsMagnifyOSD2X(void);
void dvC341Geo_EnableMagnifyOSD2X(UINT8 Enable2X);
void dvc341Geo_FrameMemoryProc30Bits(UINT8 Enable30bit);
UINT8 dvc341Geo_IsFrameMemoryProc30Bits(void);
void dvc341Geo_IsDefaultWarpTable_Set(UINT8 IsDefault);
void dvc341Geo_BypassModeSet(UINT8 BypassWarpingBlock);

#endif /* DVC341Geo_GEO_H */

