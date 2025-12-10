
#include "Common.h"

#include <math.h>
#include "utilCommon.h"

#ifdef SCALER_C821_C789
#include "dvC789.h"
#include "dvC789_WarpLight.h"
#endif

#include "utilMisc.h"
#include "utilDbgMsg.h"
#include "ProcessMutexData.h"
#include "utilWarpDemo.h"
#include "utilWarpDemo_Char.h"
#include "halWarping.h"
#include "palImgMgr.h"
#include "appDataMgr.h"

#ifdef CUSTOM_BARCO               //A35G2_Simon_0087
#define WARP_LIMIT_ENABLE 1
#else
#define WARP_LIMIT_ENABLE 0 //0 : uncheck warp limit OFF, 1 : check warp limit
#endif

#define WARP_SHARPNESS_MAX       WARP_SHARPNESS_MAX_VALUE//9						//G100_Doulas_0027 Modify
#define WARP_SHARPNESS_DEFAULT   WARP_SHARPNESS_DEFAULT_VALUE//WARP_SHARPNESS_MAX	//G100_Doulas_0027 Modify
#define OVERLAP_GRID_NUM_MAX 	 BLEND_OVERLAP_GRID_NUMBER_MAX_VALUE///4			//G100_Doulas_0027 Modify
#define OVERLAP_GRID_NUM_DEFAULT BLEND_OVERLAP_GRID_NUMBER_DEFAULT_VALUE//0		//G100_Doulas_0027 Modify
#define OVERLAP_GAP_MINIMUM      4 //A35G2_BRC_Casper_0048
#define BLEND_GAMMA_MAX 		 eCM_BLENDING_GAMMA_NUMBER-1 //6					//G100_Doulas_0027 Modify
#define BLEND_GAMMA_DEFAULT      eCM_BLENDING_GAMMA_2_2///4						//G100_Doulas_0027 Modify
#define BLEND_WIDTH_H_MIN 192	//base on native resolution of platform, current setting is for WUXGA
#define BLEND_WIDTH_H_MAX 960	//base on native resolution of platform, current setting is for WUXGA
#define BLEND_WIDTH_V_MIN 120	//base on native resolution of platform, current setting is for WUXGA
#define BLEND_WIDTH_V_MAX 600	//base on native resolution of platform, current setting is for WUXGA
#define BLEND_WIDTH_ADJUST_STEP 4
#define OSD_PRESET_INVALID 5
#define PC_PRESET_INVALID 3


///////////////// callback function start ///////////////
sUTILWARPDEMO_CALLBACK sUtilWarpDemo_Callback ;

void utilWarpDemo_RegCallback(sUTILWARPDEMO_CALLBACK fpCallback)
{
    sUtilWarpDemo_Callback = fpCallback;
}
////////////////// callback function end ////////////////


typedef struct
{
    uint16 nL;
    uint16 nR;
    uint16 nT;
    uint16 nB;
} BLEND_WIDTH;

typedef struct
{
    uint16 nL;
    uint16 nR;
    uint16 nT;
    uint16 nB;
} BLEND_OFFSET; //A35G2_BRC_Casper_0046

typedef struct
{
    uint8 pnHeader[4];
    WARP_CONFIG stWarpConfig;
    uint32 nChecksum;
}WARP_CONFIG_FLASH;

typedef struct
{
    uint8 pnHeader[4];
    OSD_WARP stOsdWarp;
    uint32 nChecksum;
}OSD_WARP_FLASH;

MODE_TABLE m_ModeTable[RES_ID__MAX] =
{
/*							Frame	PixClk	HActive	VActive	HTotal	VTotal	HStart	VStart	HSyncWidth	nVSyncWidth	eHPolarity	eVPolarity		*/
/*	------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
	{	RES_ID__XGA,		6000,	6500, 	1024,	768,	1344,	806,	296, 	35,		136,		6,			POL__NEG,	POL__NEG},
	{	RES_ID__WXGA,		6000,	8350, 	1280,	800,	1680,	831,	328, 	28,		128,		6,			POL__NEG,	POL__POS},
	{	RES_ID__UWHD,		6000,	11175, 	1920,	720,	2496,	748,	480, 	25,		192,		10,			POL__NEG,	POL__POS},
	{	RES_ID__1080P60,	6000,	14850, 	1920,	1080,	2200,	1125,	192, 	41,		44,			5,			POL__POS,	POL__POS},
	//{	RES_ID__WUXGA,	    6000,	15400, 	1920,	1200,	2080,	1235,	112, 	32,		32,			6,			POL__POS,	POL__NEG},
	{	RES_ID__WUXGA,	    6000,	14850, 	1920,	1200,	2004,	1235,	56, 	18,		15,			5,			POL__POS,	POL__NEG},	//A65_OPTOMA_Doulas_0060 Modify -2
	{	RES_ID__XGA120,		12000,	11550, 	1024,	768,	1184,	813,	112, 	42,		32,			4,			POL__POS,	POL__NEG},
	{	RES_ID__720P120,	12000,	13175, 	1280,	720,	1440,	763,	112, 	40,		32,			5,			POL__POS,	POL__NEG},
	{	RES_ID__1080P48,	4800,	14850, 	1920,	1080,	2750,	1125,	192, 	41,		44,			5,			POL__POS,	POL__POS},
	{	RES_ID__1080P120,	12000,	27000, 	1920,	1080,	2000,	1125,	48, 	40,		21,			4,			POL__POS,	POL__NEG},	//G100_Doulas_0027
	{	RES_ID__WUXGA120,	12000,	29700, 	1920,	1200,	2004,	1235,	48, 	19,		15,			5,			POL__POS,	POL__NEG},  //G100_Doulas_0063
	{	RES_ID__WXGA120,	12000,	14850, 	1280,	800,	1460,	847,	56, 	20,		16,		    5,			POL__POS,	POL__NEG},	//R70G2_Doulas_0004
};

//SPI Flash Address
#define WARP_CONFIG_FLASH_ADDR      0x00000000  //size of warp config must less than 12k(0x3000)    //G100_Doulas_0059
#define OSD_WARP_PRESET_FLASH_ADDR  0x00003000  //size of each osd warp must less than 12K(0x3000)
#define OSD_WARP_PRESET_FLASH_SIZE 0x00003000   //size 12K

#if 0 //G100_Doulas_0027
//Memory Address
#define DEF_MWI			    0x20				// 0x20=32, 32*64=2048
#define OSD_BASIC_ADDR		0x06000000	//size=0x300000*2(2048x1200=0x258000; two layers), addr=0x06000000
#define OSD_TEMP_ADDR		0x06600000	//size=0x300000(2048x1200=0x258000; one layers), addr=OSD_BASIC_ADDR+0x300000*2
#define OSD_FONT_ADDR		0x06900000	//size=50k<0x100000, addr=OSD_BASIC_ADDR+0x300000*3
#define OSD_BMP_ADDR		0x06A00000	//size=100k<0x200000, addr=OSD_FONT_ADDR+0x100000
#define RAM_DATA_ADDR		0x06C00000	//size=0x300000(2048x1200=0x258000), addr=OSD_BMP_ADDR+0x200000
#define OSD_BIAS_ADDR		0x06F00000	//size=0x200000(1024x1200=0x12C000), addr=RAM_DATA_ADDR+0x300000
#define OSD_DBD_ADDR		0x07100000	//size=0x300000(2560x1200=0x2EE000, for 10bit:1920/6*8=2560), addr=OSD_BIAS_ADDR+0x200000

//MWI Definition
#define OSD_BIAS_MWI		0x04		//linefeed width is EBIASMWI[7:0]x256 bytes, but 1 byte contain 2 pixels
#define OSD_DBD_8BIT_MWI	0x08		//linefeed width is EGBDBDMWI[7:0]x256 bytes
#endif

//RTCT Definition
#define	RTCT_THRU			0x000000	// 0000_0000_0000_0000 (All Transfer THRU)
#define	RTCT_STOP			0x088888	// 1000_1000_1000_1000_1000 (All Transfer STOP)
#define	RTCT_NORMAL			0x092165	// 0010_0001_0101_0101 (WPOVS_PI2VS_PI1VS_PO2VS_PO1VS)
#define	RTCT_POVSSTOP		0x082188	// 0010_0001_0100_0100 (WPOVS_PI2VS_PI1VS_STOP__STOP_)
#define	RTCT_PO1VSGO		0x052155
#define	RTCT_WARP			0x092159

//===== Warp Param =====//   //G100_Doulas_0039
// divide by 0
#define Def_DIV0 0xFFFFFFFF

#define DEF_NUM_WPMODE		6	// Number of cursor mode, 2x2, 3x3, 5x5, 9x9, 17x17, 33x33   //G100_Doulas_0059
#define DEF_CURMD				{{2,2}, {3,3}, {5,5}, {9,9}, {17,17}, {33,33}}	// Cursor number setting
#define DEF_CUREN_H			{\
					{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},\
					{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},\
					{1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1},\
					{1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1},\
					{1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1},\
					{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}\
                            }// Horizontal cursor enable : 2, 3, 5, 9, 17, 33
#define DEF_CUREN_V			{\
					{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},\
					{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},\
					{1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1},\
					{1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1},\
					{1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1},\
					{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}\
                            }// Vertical cursor enable : 2, 3, 5, 9, 17, 33

const int PS_NUM_GRID[DEF_NUM_WPMODE][2] = DEF_CURMD;
const char PS_HGRID_EN[DEF_NUM_WPMODE][DEF_NUM_CUR_MAX_H] = DEF_CUREN_H;
const char PS_VGRID_EN[DEF_NUM_WPMODE][DEF_NUM_CUR_MAX_V] = DEF_CUREN_V;
typedef struct {
	double a[3];
	double b[3];
	double c[3];
} homography_coef_t;

typedef struct {
	float a;
	float b;
	float c;
	float d;
} spline_coef_t;

//====  System  ====
uint8 m_pnHeader[4] = {0xcc, 0x77, 0x88, 0x78};
WARP_CONFIG m_stWarpConfig;
BOOL m_bOsdWarpParamChanged = FALSE;
struct timespec m_stStartTime;
eOUTPUT_MODE m_eOutputMode = OUTPUT_MODE__MAX;
BOOL m_bOutputNativeRes = TRUE;
float m_fResRatioX;//ratio in H between current res. and native res.
float m_fResRatioY;//ratio in V between current res. and native res.
float m_f3dResRatioX;//ratio in H between 3D res. and native res.   //G100_Doulas_0065
float m_f3dResRatioY;//ratio in V between 3D res. and native res.   //G100_Doulas_0065
uint16 m_nNativeResH;		//width of native resolution        //G100_Doulas_0065
uint16 m_nNativeResV;		//height of native resolution       //G100_Doulas_0065
uint16 m_n3dResH;			//width of 1080P120Hz resolution    //G100_Doulas_0065
uint16 m_n3dResV;			//height of 1080P120Hz resolution   //G100_Doulas_0065
uint8 m_nFlashIdx = 0;

//output timing
eRES_ID m_eCurrentResId;    //G100_Doulas_0065
eRES_ID m_e3dResId;         //G100_Doulas_0065
uint16 PS_PANEL_HS_CYCL;
uint16 PS_PANEL_VS_CYCL;
ePOL PS_PANEL_HS_POL;
ePOL PS_PANEL_VS_POL;
uint16 PS_PANEL_HS_W;		// HSyncWidth
uint16 PS_PANEL_VS_W;		// VSyncWidth
uint16 PS_PANEL_ACT_HST;	// HStart
uint16 PS_PANEL_ACT_HW;		// HActive
uint16 PS_PANEL_ACT_VST;	// VStart
uint16 PS_PANEL_ACT_VW;		// VActive
uint16 PS_PANEL_FV;	//format:Hz*100
INT16 PS_PANEL_ACT_VST_OFFSET = 0;	// VStart offset  G100_Doulas_0043 Add


//====  Warping  ====
eDIR m_eMovePrevDir = DIR__INVALID;
eADJ m_eAdjPrevAction = ADJ__INVALID;   //A65_OPTOMA_Doulas_0020

uint16 m_nMoveContinueCount = 0;
#define MOVE_CONTINUE_TIMEOUT 600	//unit:ms	//kenton_temp_check 200ms for IR control. 600ms for RS232 control
#define MOVE_CONTINUE_MAX 10
#define MOVE_CONTINUE_STEP 2	//max move pitch = MOVE_CONTINUE_MAX * MOVE_CONTINUE_STEP

//====  OSD Test Pattern  ====
#define GRID_LINE_WIDTH 1  //grid line size of ACU grid pattern //A70LK_CL_0001
#define GRID_LINE_WIDTH_SEL 3  //selected grid line size of ACU grid pattern       //H2PF_Simon_0037
#define OSD_BLEND_STR_WIDTH 100
#define OSD_BLEND_STR_HEIGHT 64
#define OSD_BLEND_BORDER_WIDTH 5
#define OSD_NATIVE_CHAR_LEN 0 //use native char length if set the length of each char to 0
#define OSD_FIXED_CHAR_LEN 25 //fiexd length of each char in OSD string

#define OSD_ACU_GRID_SIZE 60 //grid size of ACU grid pattern

eOSD_POS m_eOsdPosition = OSD_POS__INVALID;
ePAT_TYPE m_ePatternType = PAT_TYPE__OFF;
uint32 m_OsdAddrLayer0, m_OsdAddrLayer1, m_OsdAddrLayerCurrent;
uint8 m_nSelCtlPointX;
uint8 m_nSelCtlPointY;
uint8 m_nPointSize;
//color of overlap area
eCOLOR_IDX m_eOverlapColor = COLOR_IDX__DARK_GREEN;
uint8 m_nNumberChar = 100;
eDIR m_eBlendWidthSel = DIR__UP;
uint8 m_nLineFeed = 0;

//===== Warp Param =====//
//#define Def_Wp_Space		16// 16, 32						//G100_Doulas_0027 remove
#define Def_Wp_Space_Bits	((Def_Wp_Space==32) ? 5 : 4)	//G100_Doulas_0027 Modify name Def_Wp_Space_Bit
#define Def_HW_Max_GRDs		121	//Base on 1920				//G100_Doulas_0027 Modify name Def_HW_Max_GRD
#define Def_VW_Max_GRDs		76	//Base on 1200                 //G100_Doulas_0042 Modify name Def_VW_Max_GRD

//===== Warp Lim =====//
#define Def_WPLIMANG_H	1.0	// Limitation of H-slope : tan(45deg)
#define Def_WPLIMANG_V	5.67	// Limitation of V-slope : tan(80deg)
//#define Def_WPLIMVSH_L	(Def_Wp_Space/(1.0/3.0) * 16.0)	// Limitation of local V-shrink rate  //A65_OPTOMA_Doulas_0020//G100_Doulas_0065
#define Def_WPLIMVSH_L	(Def_Wp_Space/(0.6) * 16.0)	// Limitation of local V-shrink rate
#define Def_WPLIMVSH_A	(Def_Wp_Space/(0.8) * 16.0)	// Limitation of averate V-shrink rate for WUXGA@120
//#define Def_WPLIMVSH_A	(Def_Wp_Space/(1.0/2.0) * 16.0)	// Limitation of averate V-shrink rate for WUXGA@60
#define Def_WPLIMHSH	(Def_Wp_Space/(1.0/12.0) * 16.0) 	// Limitation of H-shrink rate
#define Def_WPLIMITV_H	2	// Limitation of horizontal interval of grid : DEF_WPLIMITV_H > 0
#define Def_WPLIMITV_V	2	// Limitation of vertical interval of grid : DEF_WPLIMITV_V > 0


//===== parameters 1 =====//
uint16 PS_WP_HW_GRD;
uint16 PS_WP_VW_GRD;
uint16 NATIVE_WP_HW_GRD;
uint16 NATIVE_WP_VW_GRD;
uint16 THREED_WP_HW_GRD;  //for 3D input timing  //G100_Doulas_0065
uint16 THREED_WP_VW_GRD;  //for 3D input timing  //G100_Doulas_0065

//===== parameters 2 =====//    //G100_Doulas_0039
float PS_CUR_DEF_IX[DEF_NUM_CUR_MAX_H];
float PS_CUR_DEF_IY[DEF_NUM_CUR_MAX_V];
float PS_CUR_DEF_OX[DEF_NUM_CUR_MAX_H];
float PS_CUR_DEF_OY[DEF_NUM_CUR_MAX_V];

//===== Warping ====//
Point2f (*PM_GRID)[DEF_NUM_CUR_MAX_V];
Point2f PM_GRID_OLD[DEF_NUM_CUR_MAX_H][DEF_NUM_CUR_MAX_V];
Point2f PM_GRID_TEMP[DEF_NUM_CUR_MAX_H][DEF_NUM_CUR_MAX_V];

//===== OSD Cursor parameters ====//
uint8 GV_NUM_HGRID;
uint8 GV_NUM_VGRID;

uint8 GV_GRID_EN[DEF_NUM_CUR_MAX_H][DEF_NUM_CUR_MAX_V];

//===== Warp Table parameters ====//
int GV_WARP_TABLE_X[Def_HW_Max_GRDs][Def_VW_Max_GRDs];		//G100_Doulas_0042
int GV_WARP_TABLE_Y[Def_HW_Max_GRDs][Def_VW_Max_GRDs];		//G100_Doulas_0042


spline_coef_t hline[DEF_NUM_CUR_MAX_H + 1][DEF_NUM_CUR_MAX_V];  //G100_Doulas_0059
spline_coef_t vline[DEF_NUM_CUR_MAX_H][DEF_NUM_CUR_MAX_V + 1];
spline_coef_t hline_mv[DEF_NUM_CUR_MAX_H + 1];
spline_coef_t vline_mv[Def_HW_Max_GRD][DEF_NUM_CUR_MAX_V + 1];
spline_coef_t temp_spl_cf[DEF_NUM_CUR_MAX + 1];
Point2f temp_pt[DEF_NUM_CUR_MAX];
float ver_line_x[DEF_NUM_CUR_MAX_H];
float hor_line_y[Def_HW_Max_GRD][DEF_NUM_CUR_MAX_V];

//Black Level   //A65_OPTOMA_Doulas_0020
#define BLACKLEVEL_MOVE_CONTINUE_TIMEOUT 1400
#define SUPPORT_WARP_CONTROL_PC
#define EBIAS_PLT_IDX_MAX 15
float PM_EBIAS_GAMMA[3] = {2.2f, 2.2f, 2.2f};
int PM_EBIAS_GAMMA_COLOR;
char PM_EBIAS_GMDT2[16][3][16];
uint8 PM_EBIAS_GMDT[16][3][16];
float ebgmLevel[16] = { 0, 4, 8, 12, 16, 24, 32, 48, 64, 96, 128, 192, 256, 384, 512, 1024 };

//===== Edgeblend Bias ====//
int                         PM_EBIAS_EN;
int                         PM_EBIAS_SEL_CUR_X;
int                         PM_EBIAS_SEL_CUR_Y;
coord_t                     PM_EBIAS_CUR[2][2];
int                         PM_EBIAS_PLT;
int                         PM_EBIAS[16][3];
int                         PM_EBIAS_AREA_TEST;
int                         PM_EBIAS_CUR_MV;
int                         PM_EBIAS_COLOR;
int                         PM_BOARD;

//===== parameters 1 =====//
int                         PS_WP_HW;
int                         PS_WP_VW;
Node *m_pFirstNode[BLACKLEVEL_AREA__NUM];  //Pointer point to first node of link list
Node *m_pnCurrentNode[BLACKLEVEL_AREA__NUM];
eBLACKLEVEL_AREA m_eAreaSelection = BLACKLEVEL_AREA__BOTTOM;  //default : bottom
//grid and cursor color for blacklevel
eCOLOR_IDX m_eBlackLevelGridColor = COLOR_IDX__BLACKLEVEL_GREEN;
eCOLOR_IDX m_eBlackLevelCursorColor = COLOR_IDX__BLACKLEVEL_MAGENTA;
uint8 m_nBlackLevelPointSize;
//typedef struct {
//	uint16 st;
//	uint16 end;
//} pos_t;
IntersectedPoint m_stIntersectedPoint;
XValueSet m_stXValueSet;
Vertex m_stVertex;
BOOL m_bRewriteOsdBlackLevelArea = FALSE;
uint8 m_ucCUShow;
uint8 m_pnCUSelectEnableTable[ACU_TARGET_X_MAX][ACU_TARGET_Y_MAX] =
{{0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}};
const float HPosRatio[ACU_TARGET_X_MAX] = {0.05, 0.167, 0.278, 0.389, 0.5, 0.611, 0.722, 0.833, 0.95};
const float VPosRatio[ACU_TARGET_Y_MAX] = {0.05, 0.167, 0.333, 0.5, 0.667, 0.833, 0.95};
const float HCenterPosRatio[3] = {0.5, 0.5, 0.722};
const float VCenterPosRatio[3] = {0.5, 0.667, 0.5};
sWARPOSD_PALETTE_DATA m_stUserDefineOsdPalette = {COLOR_IDX__USER_DEFINE, 0, 0, 0};

/*
Color Palette index on OSD drawing of C789
0=>utilWarpDemo(for OSD)(Default) ==> same, skip OSD_Color_Init()
1=>halWarping(for AP blender & Twist) ==>if same, skip halWarpOSD_Color_Initial()
2=>PNG(depend on input png file) ==>update palette every time
3=>draw sprite(Twist only) ==>update palette every time m_sHalWarpingInfo.aulMapSpritePalette(), m_sHalWarpingInfo.aulSpritePalette(write to palette)
*/
eCOLOR_PALETTE_GROUP m_ucColorPalette = COLOR_PALETTE__OSD;

//A65_OPTOMA_CL_0007
//m_pnData : Àx¦sPixel Circleªº¸ê®Æ, ³Ì¤jÀx¦s¥b®|100pixels
uint16 m_pnData[50];
//m_pnDataValue, m_pnDataCount : Àx¦sPixel Circle¤ÀªR«áªºµ²ºc¡A¤À§OÀx¦s­È(value)»P¹ïÀ³ªº­Ó¼Æ(count)
uint16 m_pnDataValue[50];
uint16 m_pnDataCount[50];
sWARPOSD_CIRCLE_GRID_DRAW_INFO *m_psCircleInfo;
sWARPOSD_CHECKBOARD_DRAW_INFO *m_psChecksInfo; //A65_OPTOMA_CL_0015
uint16 m_egBct = 0; //A35G2_CDS_CL_0002//A35G2_Alan_0015
UINT8 ucReApplyBlend = 0;

void EnableTestPattern(BOOL bEnable, eBKG_COLOR eBkgColor, eAPPLY_BLEND eApplyBlend, eAPPLY_BLACKLEVEL eApplyBlackLevel, eOSD_POS bBeforeWarp);   //A65_OPTOMA_Doulas_0020

//===Warp LUT====
const uint8 LUT[10][24]={
	{ 0x02, 0x03, 0x04, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x09, 0x0a, 0x0b, 0x0c, 0x0c, 0x0d, 0x0e, 0x0e, 0x0f, 0x0f, 0x11, 0x12, 0x13, 0x13, 0x14 },// 6s_c0.3_w0.0
	{ 0xfe, 0xff, 0xff, 0x00, 0x01, 0x02, 0x03, 0x05, 0x06, 0x08, 0x09, 0x0b, 0x0c, 0x0e, 0x0f, 0x10, 0x12, 0x13, 0x14, 0x15, 0x17, 0x16, 0x18, 0x16 },// 6s_c0.4_w0.0
	{ 0xfa, 0xfa, 0xfa, 0xfb, 0xfc, 0xfd, 0xff, 0x00, 0x02, 0x04, 0x06, 0x09, 0x0c, 0x0e, 0x11, 0x11, 0x16, 0x18, 0x1a, 0x1c, 0x1e, 0x1f, 0x1e, 0x1e },// 6s_c0.5_w0.0
	{ 0xfb, 0xf9, 0xf8, 0xf8, 0xf7, 0xf8, 0xf9, 0xfb, 0xfc, 0xff, 0x01, 0x04, 0x08, 0x0c, 0x10, 0x13, 0x18, 0x1d, 0x21, 0x24, 0x27, 0x27, 0x28, 0x24 },// 6s_c0.6_w0.0
	{ 0x00, 0xff, 0xfc, 0xfa, 0xf8, 0xf7, 0xf6, 0xf6, 0xf7, 0xf8, 0xfb, 0xfe, 0x01, 0x06, 0x0c, 0x12, 0x17, 0x1d, 0x23, 0x28, 0x2d, 0x2f, 0x30, 0x30 },// 6s_c0.7_w0.0
	{ 0x05, 0x04, 0x02, 0x00, 0xfe, 0xfb, 0xf8, 0xf5, 0xf5, 0xf5, 0xf6, 0xf8, 0xfc, 0x00, 0x05, 0x0d, 0x13, 0x1a, 0x21, 0x28, 0x2d, 0x32, 0x36, 0x3c },// 6s_c0.8_w0.0
	{ 0x06, 0x07, 0x06, 0x05, 0x03, 0x00, 0xfe, 0xfa, 0xf8, 0xf6, 0xf5, 0xf5, 0xf7, 0xfb, 0x00, 0x06, 0x0e, 0x16, 0x1e, 0x26, 0x2d, 0x32, 0x36, 0x40 },// 6s_c0.9_w0.0
	{ 0x02, 0x04, 0x06, 0x07, 0x07, 0x05, 0x03, 0x00, 0xfd, 0xf9, 0xf6, 0xf4, 0xf4, 0xf6, 0xfa, 0x00, 0x08, 0x11, 0x1b, 0x25, 0x2e, 0x37, 0x3c, 0x40 },// 6s_c1.0_w0.0
	{ 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0xff, 0xfd, 0xfb, 0xf9, 0xf8, 0xf9, 0xfc, 0x00, 0x06, 0x0f, 0x1a, 0x26, 0x32, 0x3a, 0x3e, 0x40 },// 6s_c1.0_w1.0
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xfe, 0xfd, 0xfc, 0xfc, 0xfd, 0x00, 0x05, 0x0d, 0x18, 0x23, 0x2e, 0x38, 0x3e, 0x40 } // 6s_c1.0_w1.5
};

const float m_pfBlendGammaTable[7] = {1.8, 1.9, 2.0, 2.1, 2.2, 2.3, 2.4};
const uint8 m_pnOverlapGridNumTable[5] = {4, 6, 8, 10, 12};
BOOL m_bApBlendApply = FALSE;  //A65_OPTOMA_CL_0010
BOOL m_bApBlacklevelApply = FALSE; //A65_OPTOMA_CL_0010
BOOL m_bApOnlyBlendApply = FALSE;  //only blend, no warp

//set timeout to 1000ms
BOOL WaitForReadFlashDone(void)
{
	uint32 nSFLSTAT = 0xFF;
	uint16 nRetry = 100;

	//ensure that the DMABUSY becomes "0"
	do {
		MS_SLEEP(10);
		nSFLSTAT = dvC789_Read(B1_SFLSTAT);
		nRetry--;
		if(nRetry==0)
			LOG_MSG(db_HAL_WARPING,"WaitForReadFlashDone() timeout, nSFLSTAT=%08X\r\n", (unsigned int)nSFLSTAT);
	} while ((nSFLSTAT&BIT0) && (nRetry!=0));

	if(nRetry==0)
		return FALSE;
	else
		return TRUE;
}

//set timeout to 3s, write bias for WUXGA(size=1200K) will take 2.5s
BOOL WaitForWriteFlashDone(void)
{
	uint32 nSFLSTAT = 0xFF;
	uint16 nRetry = 300;

	//ensures that the WBUSY and DMABUSY becomes "0"
	do {
		MS_SLEEP(10);
		nSFLSTAT = dvC789_Read(B1_SFLSTAT);
		nRetry--;
		if(nRetry==0)
			LOG_MSG(db_HAL_WARPING,"WaitForWriteFlashDone() timeout, nSFLSTAT=%02X\r\n", (unsigned int)nSFLSTAT);
	} while ((nSFLSTAT&(BIT0|BIT1)) && (nRetry!=0));


	if(nRetry==0)
		return FALSE;
	else
		return TRUE;
}

//Block Erase Cycle Time (64KB) max is 650ms
BOOL WaitForEraseFlashDone(void)
{
	uint32 nRDSR = 0xFF;
	uint32 nSFLSTAT = 0xFF;
	uint16 nRetry = 200;

	//ensure that the WBUSY becomes "0"
	do {
		MS_SLEEP(10);
		nSFLSTAT = dvC789_Read(B1_SFLSTAT);
		nRetry--;
		if(nRetry==0)
			LOG_MSG(db_HAL_WARPING,"WaitForEraseFlashDone_1() timeout, nSFLSTAT=%02X\r\n", (unsigned int)nSFLSTAT);
	} while ((nSFLSTAT&BIT1) && (nRetry!=0));

	nRetry = 200;
	//Erasing is completed, if a bit[1:0] value of SFLRDSR is 00.
	do {
		MS_SLEEP(10);
		nRDSR = dvC789_Read(B1_SFLRDSR);
		nRetry--;
		if(nRetry==0)
			LOG_MSG(db_HAL_WARPING,"WaitForEraseFlashDone_2() timeout, nRDSR=%02X\r\n", (unsigned int)nRDSR);
	} while ((nRDSR&(BIT0|BIT1)) && (nRetry!=0));

	if(nRetry==0)
		return FALSE;
	else
		return TRUE;
}

BOOL EraseFlashSector(uint32 nFlashAddr, eFLASH_ERASE_MODE eEraseMode)    //G100_Doulas_0039
{
	uint8 nRetry = 4;
	BOOL err = FALSE;

	//workaround for time out of erase action
	do {
		//GIOS  000Fh  Use Serial Flash mode
		dvC789_WriteToBuffer(B1_GIOS, 0x000F);
		//Command Idle.
		dvC789_WriteToBuffer(B1_SFLCT, 0x00);
		//Serial Flash I/F is initialized.
		dvC789_WriteToBuffer(B1_SFLMODE, 0x80);
		//Select Fast_Read and 4-byte mode. Operating frequency set to 50MHz@max.
		dvC789_WriteToBuffer(B1_SFLMODE, 0x13);
		//Erase command set to Serial Flash.
		if(eEraseMode == FLASH_ERASE_MODE__BE64K)
			dvC789_WriteToBuffer(B1_SFLCMDER, 0xDC);
		else if(eEraseMode == FLASH_ERASE_MODE__BE32K)
			dvC789_WriteToBuffer(B1_SFLCMDER, 0x5C);
		else //csfemSE4K
			dvC789_WriteToBuffer(B1_SFLCMDER, 0x21);
		//Write command set of Serial Flash.
		dvC789_WriteToBuffer(B1_SFLCMDWREN, 0x06);
		//RDSR Command set to Serial Flash.
		dvC789_WriteToBuffer(B1_SFLCMDRDSR, 0x05);
		//256Mbit
		dvC789_WriteToBuffer(B1_SFLSZSEL, 0x01);
		//Serial Flash erase address.
		dvC789_WriteToBuffer(B1_SFLAD, nFlashAddr);
		//Sector erase.
		dvC789_WriteToBuffer(B1_SFLCT, 0x04);
		dvC789_Buffer_Flush();

		err = WaitForEraseFlashDone();
		nRetry--;
	} while ((!err) && (nRetry!=0));

	if(nRetry==0)
		return FALSE;
	else
		return TRUE;

}

//Please Erase Flash first before call this function
BOOL Reg2Flash(uint32 nRamAddr, eDMA_TARGET eDmaTarget, uint32 nFlashAddr, uint32 nDataSize)
{
	dvC789_WriteToBuffer(B0_OSDCT, 0x00);
	//GIOS  000Fh  Use Serial Flash mode
	dvC789_WriteToBuffer(B1_GIOS, 0x000F);
	//Command Idle.
	dvC789_WriteToBuffer(B1_SFLCT, 0x00);
	//Serial Flash I/F is initialized.
	dvC789_WriteToBuffer(B1_SFLMODE, 0x80);
	//Select Fast_Read and 4-byte mode. Operating frequency set to 50MHz@max.
	dvC789_WriteToBuffer(B1_SFLMODE, 0x13);
	//Write command set of Serial Flash.
	dvC789_WriteToBuffer(B1_SFLCMDWR, 0x12);
	//Write command set of Serial Flash.
	dvC789_WriteToBuffer(B1_SFLCMDWREN, 0x06);
	//RDSR Command set to Serial Flash.
	dvC789_WriteToBuffer(B1_SFLCMDRDSR, 0x05);
	//256Mbit
	dvC789_WriteToBuffer(B1_SFLSZSEL, 0x01);
	//Serial Flash erase address.
	dvC789_WriteToBuffer(B1_SFLAD, nFlashAddr);
	//Number of bytes to transfer to Serial Flash.
	dvC789_WriteToBuffer(B1_SFLCNT, nDataSize);
	//DMA transfer target
	dvC789_WriteToBuffer(B1_SFLREGSEL, eDmaTarget);
	dvC789_WriteToBuffer(B0_CPURAD, nRamAddr);

	//Set DMA transfer.
	dvC789_WriteToBuffer(B1_SFLCT, 0x01);
	dvC789_Buffer_Flush();

	return WaitForWriteFlashDone();
}

BOOL Flash2Reg(uint32 nRamAddr,eDMA_TARGET eDmaTarget, uint32 nFlashAddr, uint32 nDataSize)
{
	dvC789_WriteToBuffer(B0_OSDCT, 0x00);
	//GIOS  000Fh  Use Serial Flash mode
	dvC789_WriteToBuffer(B1_GIOS, 0x000F);
	//Command Idle.
	dvC789_WriteToBuffer(B1_SFLCT, 0x00);
	//Serial Flash I/F is initialized.
	dvC789_WriteToBuffer(B1_SFLMODE, 0x80);
	//Select Fast_Read and 4-byte mode. Operating frequency set to 50MHz@max.
	dvC789_WriteToBuffer(B1_SFLMODE, 0x13);
	//Read Command setting from Serial Flash.
	dvC789_WriteToBuffer(B1_SFLCMDRD, 0x0C);
	//256Mbit
	dvC789_WriteToBuffer(B1_SFLSZSEL, 0x01);
	//Serial Flash read address.
	dvC789_WriteToBuffer(B1_SFLAD, nFlashAddr);
	//Number of bytes to transfer from Serial Flash.
	dvC789_WriteToBuffer(B1_SFLCNT, nDataSize);
	//DMA transfer target
	dvC789_WriteToBuffer(B1_SFLREGSEL, eDmaTarget);
	dvC789_WriteToBuffer(B0_CPUWAD, nRamAddr);
	//Set DMA transfer.
	dvC789_WriteToBuffer(B1_SFLCT, 0x02);
	dvC789_Buffer_Flush();

	return WaitForReadFlashDone();
}

void Ram2Reg(uint32 nDataSize)
{
	uint32 nStatus = 0;
	uint16 nRetry = 500;

	//data size
	dvC789_WriteToBuffer(B0_BBACTHW, nDataSize-1);
	dvC789_WriteToBuffer(B0_BBACTVW, 0);

	//ram addr
	dvC789_WriteToBuffer(B0_CPURAD, RAM_DATA_ADDR);

	//wait POVS
	dvC789_WriteToBuffer(B0_BBVDLY, 0x00);
	dvC789_WriteToBuffer(B0_BBVSYCT, 0x01);	// POVS

	// SDRAM -> Register
	dvC789_WriteToBuffer(B0_OSDCT, 0x0A);
	dvC789_Buffer_Flush();
	while(TRUE)
	{
		nStatus = dvC789_Read(B0_BOSTAT);
		if((nStatus&BIT0) && (nRetry>=0))
		{
			MS_SLEEP(1);
			nRetry--;
			if(nRetry==0)
			{
				LOG_MSG(db_HAL_WARPING, "Ram2Reg() timeout, B0_BOSTAT=%02X\r\n", (unsigned int)nStatus);
				break;
			}else
				continue;
		}else
			break;
	}

	dvC789_WriteToBuffer(B0_BBVSYCT, 0x00);
	dvC789_WriteToBuffer(B0_OSDCT, 0x00);
	dvC789_Buffer_Flush();

}

void WriteC789Ram(uint8* pData, uint32 nStartAddr, uint32 nCount)
{
	uint32 nIndex = 0;

	dvC789_Write(B0_CPUWAD, nStartAddr);//Ram Address

	for (nIndex = 0; nIndex < nCount; nIndex++) {
		dvC789_WriteToBuffer(B0_CPUDT, pData[nIndex]);
	}

	//Dummy Write
	dvC789_WriteToBuffer(B0_CPUWAD, nStartAddr+nCount);//Ram Address
	dvC789_Buffer_Flush();

}

void ReadC789Ram(uint8* pData, uint32 nStartAddr, uint32 nCount)
{
	uint32 nIndex = 0;
	uint32 buf = 0;

	dvC789_WriteToBuffer(B0_CPURAD, RAM_DATA_ADDR);//Ram Address
	dvC789_WriteToBuffer(B0_CPUDTCTL, 0x01);
	dvC789_Buffer_Flush();

	for (nIndex = 0; nIndex < nCount; nIndex++) {
		buf = dvC789_Read(B0_CPUDT);
		pData[nIndex] = (uint8)buf&0xff;
	}

}

BOOL Dram_Reg2Flash(uint32 nRamAddr, uint32 nFlashAddr, uint32 nSize)
{
	uint16 nRetry = 100;
	uint32 nCPUDTCTL = 0xFF;

	dvC789_WriteToBuffer(B0_CPURAD, nRamAddr);

	dvC789_WriteToBuffer(B0_CPUDTCTL, 0x01);
	dvC789_Buffer_Flush();

	//ensures that the RBUSY becomes "0"
	do {
		MS_SLEEP(10);
		nCPUDTCTL = dvC789_Read(B0_CPUDTCTL);
		nRetry--;
		if(nRetry==0)
			LOG_MSG(db_HAL_WARPING,"Dram_Reg2Flash() timeout, nCPUDTCTL=%02X\r\n", (unsigned int)nCPUDTCTL);
	} while ((nCPUDTCTL&BIT3) && (nRetry!=0));

	return Reg2Flash(nRamAddr, DMA_TARGET__SDRAM, nFlashAddr, nSize);
}

BOOL Dram_Flash2Reg(uint32 nRamAddr, uint32 nFlashAddr, uint32 nSize)
{
	return Flash2Reg(nRamAddr, DMA_TARGET__SDRAM, nFlashAddr, nSize);
}

void SoftReset(void)
{
    dvC789_Write(BN_RSTCT, 0x01);
}



void WpmodeChange(void)  //G100_Doulas_0039
{
	int h, v;
	int PM_WP_MODE;

	PM_WP_MODE = (int)m_stWarpConfig.stOsd.eWarpPoint;

	GV_NUM_HGRID = PS_NUM_GRID[PM_WP_MODE][0];
	GV_NUM_VGRID = PS_NUM_GRID[PM_WP_MODE][1];

	// GV_WPCUR_EN
	for (v = 0; v < DEF_NUM_CUR_MAX_V; v++) {
		for (h = 0; h < DEF_NUM_CUR_MAX_H; h++) {
			if ((v == 0) | (v == (DEF_NUM_CUR_MAX_V - 1))) {
				GV_GRID_EN[h][v] = PS_HGRID_EN[PM_WP_MODE][h];
			}
			else if ((h == 0) | (h == (DEF_NUM_CUR_MAX_H - 1))) {
				GV_GRID_EN[h][v] = PS_VGRID_EN[PM_WP_MODE][v];
			}
			else if ((m_stWarpConfig.stOsd.bWarpInnerOn == FALSE) | (GV_GRID_EN[0][v] == 0)) {
				GV_GRID_EN[h][v] = 0;
			}
			else {
				GV_GRID_EN[h][v] = GV_GRID_EN[h][0];
			}
		}
	}

	if( GV_GRID_EN[m_nSelCtlPointX][m_nSelCtlPointY] == 0 ) {
		m_nSelCtlPointX = 0;
		m_nSelCtlPointY = 0;
	}

	return;
}

int CheckWpLimitA(int *x, int *y, float *diffy_y_sum, float *divn,
	uint16 PS_WP_HW_GRD, uint16 PS_WP_VW_GRD, uint16 PM_IACT_HW, uint16 PM_IACT_VW)
{
	float diffx_x, diffy_x, diffx_y, diffy_y, idiffy_y;
	float tan_h = 0;
	float tan_v = 0;
	int ihw = (PM_IACT_HW << 4);
	int ivw = (PM_IACT_VW << 4);
	int x1 = (*x == PS_WP_HW_GRD) ? *x : *x + 1;
	int y1 = (*y == PS_WP_VW_GRD) ? *y : *y + 1;

	if ((*x < PS_WP_HW_GRD) || (*y < PS_WP_VW_GRD)) {
		if ((GV_WARP_TABLE_X[*x][*y] < 0) && (GV_WARP_TABLE_X[x1][*y] < 0) && (GV_WARP_TABLE_X[*x][y1] < 0) && (GV_WARP_TABLE_X[x1][y1] < 0)) {
			return E_WpNoErr;// Outside of ACT (Horizontal)
		}
		if ((GV_WARP_TABLE_X[*x][*y] > ihw) && (GV_WARP_TABLE_X[x1][*y] > ihw) && (GV_WARP_TABLE_X[*x][y1] > ihw) && (GV_WARP_TABLE_X[x1][y1] > ihw)) {
			return E_WpNoErr;// Outside of ACT (Horizontal)
		}
		if ((GV_WARP_TABLE_Y[*x][*y] < 0) && (GV_WARP_TABLE_Y[x1][*y] < 0) && (GV_WARP_TABLE_Y[*x][y1] < 0) && (GV_WARP_TABLE_Y[x1][y1] < 0)) {
			return E_WpNoErr;// Outside of ACT (Vertical)
		}
		if ((GV_WARP_TABLE_Y[*x][*y] > ivw) && (GV_WARP_TABLE_Y[x1][*y] > ivw) && (GV_WARP_TABLE_Y[*x][y1] > ivw) && (GV_WARP_TABLE_Y[x1][y1] > ivw)) {
			return E_WpNoErr;// Outside of ACT (Vertical)
		}

		if (*x < PS_WP_HW_GRD) {
			diffx_x = (float)(GV_WARP_TABLE_X[x1][*y] - GV_WARP_TABLE_X[*x][*y]);
			diffy_x = (float)(GV_WARP_TABLE_Y[x1][*y] - GV_WARP_TABLE_Y[*x][*y]);

			// Horizontal zoom
			if (diffx_x <= 0) {// The zoom rate is infinite.
				return E_WpErrItv;
			}
			// Horizontal slope
			tan_h = diffy_x / diffx_x;
			if ((tan_h < (-Def_WPLIMANG_H)) || (tan_h > Def_WPLIMANG_H)) {
				return E_WpErrAngH;
			}
			// Horizontal shrink rate
			if (diffx_x > Def_WPLIMHSH) {
				return E_WpErrHSh;
			}
		} // end if(x < PS_WP_HW_GRD)

		if (*y < PS_WP_VW_GRD) {
			diffx_y = (float)(GV_WARP_TABLE_X[*x][y1] - GV_WARP_TABLE_X[*x][*y]);
			diffy_y = (float)(GV_WARP_TABLE_Y[*x][y1] - GV_WARP_TABLE_Y[*x][*y]);

			idiffy_y = diffy_y + diffx_y * (-tan_h);

			// Vertical zoom
			if (diffy_y <= 0) {// The zoom rate is infinite.
				return E_WpErrItv;
			}
			// Vertical local shrink rate
			if (idiffy_y > Def_WPLIMVSH_L) {
				return E_WpErrVShL;
			}
			// Vertical slope
			tan_v = diffx_y / diffy_y;
			if ((tan_v < (-Def_WPLIMANG_V)) || (tan_v > Def_WPLIMANG_V)) {
				return E_WpErrAngV;
			}

			*diffy_y_sum = (*diffy_y_sum) + idiffy_y;
			*divn = (*divn) + 1;
		} // end if(y < PS_WP_VW_GRD)
	} // end if((x < PS_WP_HW_GRD)||(y < PS_WP_VW_GRD))

	return E_WpNoErr;// no error
}

int CheckWpLimitB( float *diffy_y_sum, float *divn )
{
	if( (*diffy_y_sum) > (Def_WPLIMVSH_A * (*divn)) ){
		return E_WpErrVShA;
	}

	return E_WpNoErr;// no error
}

void CalcHomographyCoef(Point2f tl, Point2f tr, Point2f bl, Point2f br, Point2f stp, float hw, float vw, homography_coef_t *coef)
{
	float det;

	tl.x = tl.x - stp.x;
	tl.y = tl.y - stp.y;

	tr.x = tr.x - stp.x;
	tr.y = tr.y - stp.y;

	bl.x = bl.x - stp.x;
	bl.y = bl.y - stp.y;

	br.x = br.x - stp.x;
	br.y = br.y - stp.y;

	//===== calculate homography coefficient =====//
	det = (br.x - tr.x) * (br.y - bl.y) - (br.x - bl.x) * (br.y - tr.y);
	coef->a[0] = ((det == 0) || (hw == 0) ) ? Def_DIV0 : ((tr.x - tl.x) * (br.y - bl.y) - (br.x  - bl.x) * (tr.y - tl.y)) / det / hw;
	coef->b[0] = ((det == 0) || (vw == 0) ) ? Def_DIV0 : ((tr.x - br.x) * (tl.y - bl.y) - (tl.x  - bl.x) * (tr.y - br.y)) / det / vw;
	coef->c[0] = 1.0;
	coef->a[1] = (hw == 0) ? Def_DIV0 : tr.x * coef->a[0] + (tr.x - tl.x) / hw;
	coef->b[1] = (vw == 0) ? Def_DIV0 : bl.x * coef->b[0] + (bl.x - tl.x) / vw;
	coef->c[1] = tl.x;
	coef->a[2] = (hw == 0) ? Def_DIV0 : tr.y * coef->a[0] + (tr.y - tl.y) / hw;
	coef->b[2] = (vw == 0) ? Def_DIV0 : bl.y * coef->b[0] + (bl.y - tl.y) / vw;
	coef->c[2] = tl.y;
}

Point2f Homography(homography_coef_t coef, Point2f pt, Point2f stp)
{
	Point2f opt;
	float denom;

	pt.x = pt.x - stp.x;
	pt.y = pt.y - stp.y;

	denom = coef.a[0] * pt.x + coef.b[0] * pt.y + coef.c[0];
	if (denom < 0.000001) { denom = 0.000001; }
	opt.x = (coef.a[1] * pt.x + coef.b[1] * pt.y + coef.c[1]) / denom + stp.x;
	opt.y = (coef.a[2] * pt.x + coef.b[2] * pt.y + coef.c[2]) / denom + stp.y;

	return opt;
}

void CalcPjConv(Point2f wpcur[][DEF_NUM_CUR_MAX_V], Point2f pjcur[][DEF_NUM_CUR_MAX_V], float fRatioX, float fRatioY)
{
	homography_coef_t cf;
	Point2f tl, tr, bl, br;
	Point2f stp, pt;
	float hw, vw;
	int h, v;
	int hedge, vedge;

	tl = wpcur[0][0];
	tr = wpcur[DEF_NUM_CUR_MAX_H-1][0];
	bl = wpcur[0][DEF_NUM_CUR_MAX_V-1];
	br = wpcur[DEF_NUM_CUR_MAX_H-1][DEF_NUM_CUR_MAX_V-1];

	stp.x = PS_CUR_DEF_IX[0]*fRatioX;
	stp.y = PS_CUR_DEF_IY[0]*fRatioY;

	hw = (PS_CUR_DEF_IX[DEF_NUM_CUR_MAX_H-1]*fRatioX) - stp.x;
	vw = (PS_CUR_DEF_IY[DEF_NUM_CUR_MAX_V-1]*fRatioY)- stp.y;

	CalcHomographyCoef(tl, tr, bl, br, stp, hw, vw, &cf);

	for (v = 0; v < DEF_NUM_CUR_MAX_V; v++) {
		for (h = 0; h < DEF_NUM_CUR_MAX_H; h++) {
			hedge = ((h == 0) || (h == DEF_NUM_CUR_MAX_H - 1)) ? 1 : 0;// Horizontal edge
			vedge = ((v == 0) || (v == DEF_NUM_CUR_MAX_V - 1)) ? 1 : 0;// Vertical edge

			if ((hedge == 1) && (vedge == 1)) {// corner
				pjcur[h][v] = wpcur[h][v];
			}
			else {
				pt.x = PS_CUR_DEF_IX[h]*fRatioX;
				pt.y = PS_CUR_DEF_IY[v]*fRatioY;
				pjcur[h][v] = Homography(cf, pt, stp);
			}
		}
	}
}

int CalcSplineCoef(Point2f p[], int pnum, spline_coef_t cf[])
{
	float u[DEF_NUM_CUR_MAX];
	float h[DEF_NUM_CUR_MAX - 1], vh[DEF_NUM_CUR_MAX - 1];
	float hr[DEF_NUM_CUR_MAX - 1];// Reciprocal of h[].
	float e[DEF_NUM_CUR_MAX - 1], f[DEF_NUM_CUR_MAX - 1];
	float v[DEF_NUM_CUR_MAX - 2], m[3];
	float den;
	float denr;// Reciprocal of den.
	int i;

	if ((pnum < 3) || (pnum > DEF_NUM_CUR_MAX)) { return 1; }// Out of range

	h[0] = p[1].x - p[0].x;
	if (h[0] < 0.000001) { return 1; }// Reversal of the coordinate is not allowed
	hr[0] = 1 / h[0];
	vh[0] = (p[1].y - p[0].y) * hr[0];

	e[0] = 0;
	f[0] = 0;
	m[2] = 0;
	for (i = 1; i <= (pnum - 2); i++) {// i -> p,h,e,f
		h[i] = p[i + 1].x - p[i].x;
		if (h[i] < 0.000001) { return 1; }// Reversal of the coordinate is not allowed
		hr[i] = 1 / h[i];
		vh[i] = (p[i + 1].y - p[i].y) * hr[i];

		v[i - 1] = vh[i] - vh[i - 1];
		m[0] = m[2];
		m[1] = 2 * (h[i - 1] + h[i]);
		m[2] = h[i];

		den = m[1] - e[i - 1] * m[0];
		if (den == 0) { den = 0.000001; }// Div0
		denr = 1 / den;

		e[i] = m[2] * denr;
		f[i] = (v[i - 1] - f[i - 1] * m[0]) * denr;
	}

	u[pnum - 1] = 0;
	for (i = (pnum - 2); i >= 1; i--) {
		u[i] = f[i] - e[i] * u[i + 1];
	}
	u[0] = 0;

	// Spline interpolation from start point to end point
	for (i = 0; i <= (pnum - 2); i++) {
		cf[i + 1].a = (u[i + 1] - u[i]) * hr[i];
		cf[i + 1].b = 3.0 * u[i];
		cf[i + 1].c = vh[i] - (u[i + 1] + 2 * u[i]) * h[i];
		cf[i + 1].d = p[i].y;
	}

	// Before the starting point
	cf[0].a = 0;
	cf[0].b = 0;
	cf[0].c = cf[1].c;
	cf[0].d = cf[1].d;

	// After the ending point
	cf[pnum].a = 0;
	cf[pnum].b = 0;
	cf[pnum].c = cf[pnum - 1].c + cf[pnum - 1].b * h[pnum - 2];
	cf[pnum].d = p[pnum - 1].y;

	return 0;// No Error
}


float CalcSpline(spline_coef_t coef, float x, float x0)
{
	float pt;
	float w;
	float w_sq;// square
	float w_cu;// cube

	w = x - x0;
	w_sq = w * w;
	w_cu = w_sq * w;

	pt = coef.a * w_cu + coef.b * w_sq + coef.c * w + coef.d;

	return pt;
}

int CalcWpCur(Point2f (*PM_GRID)[DEF_NUM_CUR_MAX_V], float fRatioX, float fRatioY)    //G100_Doulas_0059
{
	Point2f cur_pos[DEF_NUM_CUR_MAX_H][DEF_NUM_CUR_MAX_V];
	Point2f temp_pt[DEF_NUM_CUR_MAX];
	spline_coef_t temp_spl_cf[DEF_NUM_CUR_MAX + 1];

	float scla;
	float scl_pj, scl_cur;
	float sclb[2];

	int i, j;
	int h, v;
	int h0, v0;
	float t;
	int area;
	int PM_WP_MODE;

	PM_WP_MODE = (int)m_stWarpConfig.stOsd.eWarpPoint;


	// Calculate the projective transformation for spline interpolation.
	CalcPjConv(PM_GRID, cur_pos, fRatioX, fRatioY);
	for (v = 0; v < DEF_NUM_CUR_MAX_V; v++) {
		for (h = 0; h < DEF_NUM_CUR_MAX_H; h++) {
			if ((PS_HGRID_EN[PM_WP_MODE][h] == 0) || (PS_VGRID_EN[PM_WP_MODE][v] == 0)) {
				PM_GRID[h][v] = cur_pos[h][v];
			}
		}
	}

	// Calculate the coordinates of the cursor on the line where spline interpolation is possible.(Horizontal line)
	if ((GV_NUM_HGRID >= 3) && (GV_NUM_HGRID < DEF_NUM_CUR_MAX_H)) {
		for (v = 0; v < DEF_NUM_CUR_MAX_V; v++) {
			if ((v == 0) || (v == (DEF_NUM_CUR_MAX_V - 1)) || ((m_stWarpConfig.stOsd.bWarpInnerOn == TRUE) && (PS_VGRID_EN[PM_WP_MODE][v] == 1))) {
				area = 0;
				for (h = 0; h < DEF_NUM_CUR_MAX_H; h++) {
					if (PS_HGRID_EN[PM_WP_MODE][h] != 0) {
						temp_pt[area].x = PM_GRID[h][v].x;
						temp_pt[area].y = PM_GRID[h][v].y;

						area = area + 1;
					}
				}
				if (0 != CalcSplineCoef(temp_pt, GV_NUM_HGRID, temp_spl_cf)) {
					return 1;// Reversal of the coordinate is not allowed
				}
				h0 = 0;
				area = 0;
				for (i = 1; i < DEF_NUM_CUR_MAX_H; i++) {
					if (PS_HGRID_EN[PM_WP_MODE][i] != 0) {
						for (h = h0 + 1; h < i; h++) {
							if (cur_pos[i][v].x == cur_pos[h0][v].x) {
								// div0
							}
							else {
								t = (cur_pos[h][v].x - cur_pos[h0][v].x) / (cur_pos[i][v].x - cur_pos[h0][v].x);
								PM_GRID[h][v].x = PM_GRID[h][v].x + (PM_GRID[h0][v].x - cur_pos[h0][v].x) * (1 - t) + (PM_GRID[i][v].x - cur_pos[i][v].x) * t;
							}
							PM_GRID[h][v].y = CalcSpline(temp_spl_cf[area + 1], PM_GRID[h][v].x, PM_GRID[h0][v].x);
						}
						h0 = i;
						area = area + 1;
					}
				}
			}
		}
	}

	// Calculate the coordinates of the cursor on the line where spline interpolation is possible.(Vertical line)
	if ((GV_NUM_VGRID >= 3) && (GV_NUM_VGRID < DEF_NUM_CUR_MAX_V)) {
		for (h = 0; h < DEF_NUM_CUR_MAX_H; h++) {
			if ((h == 0) || (h == (DEF_NUM_CUR_MAX_H - 1)) || ((m_stWarpConfig.stOsd.bWarpInnerOn == TRUE) && (PS_HGRID_EN[PM_WP_MODE][h] == 1))) {
				area = 0;
				for (v = 0; v < DEF_NUM_CUR_MAX_V; v++) {
					if (PS_VGRID_EN[PM_WP_MODE][v] != 0) {
						temp_pt[area].x = PM_GRID[h][v].y;// x -> y
						temp_pt[area].y = PM_GRID[h][v].x;// y -> x

						area = area + 1;
					}
				}
				if (0 != CalcSplineCoef(temp_pt, GV_NUM_VGRID, temp_spl_cf)) {
					return 1;// Reversal of the coordinate is not allowed
				}
				v0 = 0;
				area = 0;
				for (i = 1; i < DEF_NUM_CUR_MAX_V; i++) {
					if (PS_VGRID_EN[PM_WP_MODE][i] != 0) {
						for (v = v0 + 1; v < i; v++) {
							if (cur_pos[h][i].y == cur_pos[h][v0].y) {
								// div0
							}
							else {
								t = (cur_pos[h][v].y - cur_pos[h][v0].y) / (cur_pos[h][i].y - cur_pos[h][v0].y);
								PM_GRID[h][v].y = PM_GRID[h][v].y + (PM_GRID[h][v0].y - cur_pos[h][v0].y) * (1 - t) + (PM_GRID[h][i].y - cur_pos[h][i].y) * t;
							}
							PM_GRID[h][v].x = CalcSpline(temp_spl_cf[area + 1], PM_GRID[h][v].y, PM_GRID[h][v0].y);
						}
						v0 = i;
						area = area + 1;
					}
				}
			}
		}
	}

	// Calculate the internal cursor coordinates.
	v0 = 0;// top side
	for (j = 1; j < DEF_NUM_CUR_MAX_V; j++) {
		if ((j == (DEF_NUM_CUR_MAX_V - 1)) || ((m_stWarpConfig.stOsd.bWarpInnerOn == TRUE) && (PS_VGRID_EN[PM_WP_MODE][j] == 1))) {
			for (v = (v0 + 1); v < j; v++) {
				h0 = 0;// left side
				for (i = 1; i < DEF_NUM_CUR_MAX_H; i++) {
					if ((i == (DEF_NUM_CUR_MAX_H - 1)) || ((m_stWarpConfig.stOsd.bWarpInnerOn == TRUE) && (PS_HGRID_EN[PM_WP_MODE][i] == 1))) {
						for (h = (h0 + 1); h < i; h++) {
							// x
							scla = (cur_pos[i][v].x == cur_pos[h0][v].x) ? 0 : (cur_pos[h][v].x - cur_pos[h0][v].x) / (cur_pos[i][v].x - cur_pos[h0][v].x);
							// top side
							scl_pj = (cur_pos[i][v0].x == cur_pos[h0][v0].x) ? 0 : (cur_pos[h][v0].x - cur_pos[h0][v0].x) / (cur_pos[i][v0].x - cur_pos[h0][v0].x);
							scl_cur = (PM_GRID[i][v0].x == PM_GRID[h0][v0].x) ? 0 : (PM_GRID[h][v0].x - PM_GRID[h0][v0].x) / (PM_GRID[i][v0].x - PM_GRID[h0][v0].x);
							sclb[0] = (scl_pj == 0) ? 0 : scl_cur / scl_pj;
							// bottom side
							scl_pj = (cur_pos[i][j].x == cur_pos[h0][j].x) ? 0 : (cur_pos[h][j].x - cur_pos[h0][j].x) / (cur_pos[i][j].x - cur_pos[h0][j].x);
							scl_cur = (PM_GRID[i][j].x == PM_GRID[h0][j].x) ? 0 : (PM_GRID[h][j].x - PM_GRID[h0][j].x) / (PM_GRID[i][j].x - PM_GRID[h0][j].x);
							sclb[1] = (scl_pj == 0) ? 0 : scl_cur / scl_pj;
							// Calculate the internal cursor : x
							t = (cur_pos[h][j].y == cur_pos[h][v0].y) ? 0 : (cur_pos[h][v].y - cur_pos[h][v0].y) / (cur_pos[h][j].y - cur_pos[h][v0].y);
							PM_GRID[h][v].x = PM_GRID[h0][v].x + (PM_GRID[i][v].x - PM_GRID[h0][v].x) * scla * (sclb[0] * (1 - t) + sclb[1] * t);

							// y
							scla = (cur_pos[h][v].y - cur_pos[h][v0].y) / (cur_pos[h][j].y - cur_pos[h][v0].y);
							// left side
							scl_pj = (cur_pos[h0][j].y == cur_pos[h0][v0].y) ? 0 : (cur_pos[h0][v].y - cur_pos[h0][v0].y) / (cur_pos[h0][j].y - cur_pos[h0][v0].y);
							scl_cur = (PM_GRID[h0][j].y == PM_GRID[h0][v0].y) ? 0 : (PM_GRID[h0][v].y - PM_GRID[h0][v0].y) / (PM_GRID[h0][j].y - PM_GRID[h0][v0].y);
							sclb[0] = (scl_pj == 0) ? 0 : scl_cur / scl_pj;
							// right side
							scl_pj = (cur_pos[i][j].y == cur_pos[i][v0].y) ? 0 : (cur_pos[i][v].y - cur_pos[i][v0].y) / (cur_pos[i][j].y - cur_pos[i][v0].y);
							scl_cur = (PM_GRID[i][j].y == PM_GRID[i][v0].y) ? 0 : (PM_GRID[i][v].y - PM_GRID[i][v0].y) / (PM_GRID[i][j].y - PM_GRID[i][v0].y);
							sclb[1] = (scl_pj == 0) ? 0 : scl_cur / scl_pj;
							// Calculate the internal cursor : y
							t = (cur_pos[h][v].x - cur_pos[h0][v].x) / (cur_pos[i][v].x - cur_pos[h0][v].x);
							PM_GRID[h][v].y = PM_GRID[h][v0].y + (PM_GRID[h][j].y - PM_GRID[h][v0].y) * scla * (sclb[0] * (1 - t) + sclb[1] * t);
						}
						h0 = i;// new left side
					}
				}
			}
			v0 = j;// new top side
		}
	}

    #if 0
	for(int jj=0;jj<33;jj++)
	{
		printf("Y=%d\n", jj);
		for(int ii=0;ii<33;ii++)
		{
			LOG_MSG(db_ALWAYS, "X%d(%4d.%02d, %4d.%02d) \n", ii,
				(int)PM_GRID[ii][jj].x, ((int)(PM_GRID[ii][jj].x*100))%100,
				(int)PM_GRID[ii][jj].y, ((int)(PM_GRID[ii][jj].y*100))%100);
		}
		//LOG_MSG(db_ALWAYS, "\r\n");
		printf("\n---------------------\n");
	}
	#endif

	return 0;
}

int CalcWpTable(Point2f cur_pos[][DEF_NUM_CUR_MAX_V], uint16 PS_WP_HW_GRD, uint16 PS_WP_VW_GRD,
					uint16 PM_IACT_HW, uint16 PM_IACT_VW, float fRatioX, float fRatioY)  //G100_Doulas_0065
{
#if 1	//G100_Doulas_0059
	float x, y;
	int i, j;
	float x0, y0;
	int area;

#if WARP_LIMIT_ENABLE
	float diffy_y_sum, divn;
#endif
	// Limitation
	float tblx_min = -Def_Wp_Space * 12;
	float tblx_max = (PS_CUR_DEF_IX[DEF_NUM_CUR_MAX_H - 1]*fRatioX) + Def_Wp_Space * 12;
	float tbly_min = -Def_Wp_Space * 12;
	float tbly_max = (PS_CUR_DEF_IY[DEF_NUM_CUR_MAX_V - 1]*fRatioY) + Def_Wp_Space * 12;

	char err = 0;

	// spline
	// horizontal line
	for (j = 0; j < DEF_NUM_CUR_MAX_V; j++) {
		for (i = 0; i < DEF_NUM_CUR_MAX_H; i++) {
			temp_pt[i].x = cur_pos[i][j].x;
			temp_pt[i].y = cur_pos[i][j].y;
		}
		if (0 != CalcSplineCoef(temp_pt, DEF_NUM_CUR_MAX_H, temp_spl_cf)) {
			err = E_WpErrItv;
			return err;
		}

		for (i = 0; i < (DEF_NUM_CUR_MAX_H + 1); i++) {
			hline[i][j] = temp_spl_cf[i];
		}
	}
	// vertical line
	for (j = 0; j < DEF_NUM_CUR_MAX_H; j++) {
		for (i = 0; i < DEF_NUM_CUR_MAX_V; i++) {
			temp_pt[i].x = cur_pos[j][i].y;
			temp_pt[i].y = cur_pos[j][i].x;
		}
		if (0 != CalcSplineCoef(temp_pt, DEF_NUM_CUR_MAX_V, temp_spl_cf)) {
			err = E_WpErrItv;
			return err;
		}

		for (i = 0; i < (DEF_NUM_CUR_MAX_V + 1); i++) {
			vline[j][i] = temp_spl_cf[i];
		}
	}

	// table
	for (i = 0; i <= PS_WP_HW_GRD; i++) {
		x0 = (float)(i << Def_Wp_Space_Bit);

		for (j = 0; j < DEF_NUM_CUR_MAX_V; j++) {
			area = 0;
			while (area < DEF_NUM_CUR_MAX_H) {
				if (x0 < cur_pos[area][j].x) {
					break;
				}
				area = area + 1;
			}
			if (area == 0) {
				hor_line_y[i][j] = CalcSpline(hline[area][j], x0, cur_pos[area][j].x);
			}
			else {
				hor_line_y[i][j] = CalcSpline(hline[area][j], x0, cur_pos[area - 1][j].x);
			}
		}
		// vertical line
		for (j = 0; j < DEF_NUM_CUR_MAX_V; j++) {
			temp_pt[j].x = hor_line_y[i][j];
			temp_pt[j].y = ((float)PS_CUR_DEF_IY[j]*fRatioY)- hor_line_y[i][j];
		}
		if (0 != CalcSplineCoef(temp_pt, DEF_NUM_CUR_MAX_V, vline_mv[i])) {
			err = E_WpErrItv;
			return err;
		}
	}

	for (j = PS_WP_VW_GRD; j >= 0; j--) {
		y0 = (float)(j << Def_Wp_Space_Bit);

		for (i = 0; i < DEF_NUM_CUR_MAX_H; i++) {
			area = 0;
			while (area < DEF_NUM_CUR_MAX_V) {
				if (y0 < cur_pos[i][area].y) {
					break;
				}
				area = area + 1;
			}
			if (area == 0) {
				ver_line_x[i] = CalcSpline(vline[i][area], y0, cur_pos[i][area].y);
			}
			else {
				ver_line_x[i] = CalcSpline(vline[i][area], y0, cur_pos[i][area - 1].y);
			}
		}

		// horizontal line
		for (i = 0; i < DEF_NUM_CUR_MAX_H; i++) {
			temp_pt[i].x = ver_line_x[i];
			temp_pt[i].y = ((float)PS_CUR_DEF_IX[i]*fRatioX) - ver_line_x[i];
		}
		if (0 != CalcSplineCoef(temp_pt, DEF_NUM_CUR_MAX_H, hline_mv)) {
			err = E_WpErrItv;
			return err;
		}

#if WARP_LIMIT_ENABLE
		diffy_y_sum = 0;
		divn = 0;
#endif

		for (i = PS_WP_HW_GRD; i >= 0; i--) {
			x0 = (float)(i << Def_Wp_Space_Bit);
			// x
			area = 0;
			while (area < DEF_NUM_CUR_MAX_H) {
				if (x0 < ver_line_x[area]) {
					break;
				}
				area = area + 1;
			}

			if (area == 0) {
				x = CalcSpline(hline_mv[area], x0, ver_line_x[area]);
			}
			else {
				x = CalcSpline(hline_mv[area], x0, ver_line_x[area - 1]);
			}
			x = x + (float)(i << Def_Wp_Space_Bit);
			x = (x > tblx_max) ? tblx_max : (x < tblx_min) ? tblx_min : x;
			GV_WARP_TABLE_X[i][j] = (int)(x * 16 + 0.5);

			// y
			area = 0;
			while (area < DEF_NUM_CUR_MAX_V) {
				if (y0 < hor_line_y[i][area]) {
					break;
				}
				area = area + 1;
			}

			if (area == 0) {
				y = CalcSpline(vline_mv[i][area], y0, hor_line_y[i][area]);
			}
			else {
				y = CalcSpline(vline_mv[i][area], y0, hor_line_y[i][area - 1]);
			}
			y = y + (float)(j << Def_Wp_Space_Bit);
			y = (y > tbly_max) ? tbly_max : (y < tbly_min) ? tbly_min : y;
			GV_WARP_TABLE_Y[i][j] = (int)(y * 16 + 0.5);

#if WARP_LIMIT_ENABLE
			// Check Limit
			if(m_stWarpConfig.stOsd.bWarpLimit) //A35G2_BRC_Casper_0047
			{
				err = CheckWpLimitA( &i, &j, &diffy_y_sum, &divn, PS_WP_HW_GRD, PS_WP_VW_GRD, PM_IACT_HW, PM_IACT_VW );
				if (err != E_WpNoErr)
					return err;
			}
#endif
		}

#if WARP_LIMIT_ENABLE
		// Check Limit
		if(m_stWarpConfig.stOsd.bWarpLimit) //A35G2_BRC_Casper_0047
		{
			err = CheckWpLimitB(&diffy_y_sum, &divn);
			if (err != E_WpNoErr)
				return err;
		}
#endif
	}

	return err;

#else
	spline_coef_t hline[DEF_NUM_CUR_MAX_H + 1][DEF_NUM_CUR_MAX_V];
	spline_coef_t vline[DEF_NUM_CUR_MAX_H][DEF_NUM_CUR_MAX_V + 1];
	spline_coef_t hline_mv[DEF_NUM_CUR_MAX_H + 1];
	//spline_coef_t vline_mv[Def_HW_Max_GRDs][DEF_NUM_CUR_MAX_V + 1];		//G100_Doulas_0042
	spline_coef_t **vline_mv = NULL;									//G100_Doulas_0042 Modify

	Point2f temp_pt[DEF_NUM_CUR_MAX];
	spline_coef_t temp_spl_cf[DEF_NUM_CUR_MAX + 1];

	float x, y;
	int i, j;
	float ver_line_x[DEF_NUM_CUR_MAX_H];
	//float hor_line_y[Def_HW_Max_GRDs][DEF_NUM_CUR_MAX_V];		//G100_Doulas_0042
    float **hor_line_y = NULL;									//G100_Doulas_0042 Modify
	float x0, y0;
	int area;

#if WARP_LIMIT_ENABLE
	float diffy_y_sum, divn;
#endif
	// Limitation
	float tblx_min = -Def_Wp_Space * 12;
	float tblx_max = (PS_CUR_DEF_IX[DEF_NUM_CUR_MAX_H - 1]*fRatioX) + Def_Wp_Space * 12;
	float tbly_min = -Def_Wp_Space * 12;
	float tbly_max = (PS_CUR_DEF_IY[DEF_NUM_CUR_MAX_V - 1]*fRatioY) + Def_Wp_Space * 12;

	char err = 0;

	//G100_Doulas_0042 start
	//init point array 動態記憶體配置
	hor_line_y = (float**)malloc(sizeof(float*)*Def_HW_Max_GRDs);
	for(i=0; i < Def_HW_Max_GRDs; i++)
		hor_line_y[i] = (float*)malloc(sizeof(float)*DEF_NUM_CUR_MAX_V);

	vline_mv = (spline_coef_t**)malloc(sizeof(spline_coef_t*)*Def_HW_Max_GRDs);
	for(i=0; i < Def_HW_Max_GRDs; i++)
		vline_mv[i] = (spline_coef_t*)malloc(sizeof(spline_coef_t)*(DEF_NUM_CUR_MAX_V + 1));
	//G100_Doulas_0042 end

	// spline
	// horizontal line
	for (j = 0; j < DEF_NUM_CUR_MAX_V; j++) {
		for (i = 0; i < DEF_NUM_CUR_MAX_H; i++) {
			temp_pt[i].x = cur_pos[i][j].x;
			temp_pt[i].y = cur_pos[i][j].y;
		}
		if (0 != CalcSplineCoef(temp_pt, DEF_NUM_CUR_MAX_H, temp_spl_cf)) {
			err = E_WpErrItv;
			goto end;//return err;		//G100_Doulas_0042 Modify
		}

		for (i = 0; i < (DEF_NUM_CUR_MAX_H + 1); i++) {
			hline[i][j] = temp_spl_cf[i];
		}
	}
	// vertical line
	for (j = 0; j < DEF_NUM_CUR_MAX_H; j++) {
		for (i = 0; i < DEF_NUM_CUR_MAX_V; i++) {
			temp_pt[i].x = cur_pos[j][i].y;
			temp_pt[i].y = cur_pos[j][i].x;
		}
		if (0 != CalcSplineCoef(temp_pt, DEF_NUM_CUR_MAX_V, temp_spl_cf)) {
			err = E_WpErrItv;
			goto end;//return err;		//G100_Doulas_0042 Modify
		}

		for (i = 0; i < (DEF_NUM_CUR_MAX_V + 1); i++) {
			vline[j][i] = temp_spl_cf[i];
		}
	}

	// table
	for (i = 0; i <= PS_WP_HW_GRD; i++) {
		x0 = (float)(i << Def_Wp_Space_Bits);		//G100_Doulas_0042

		for (j = 0; j < DEF_NUM_CUR_MAX_V; j++) {
			area = 0;
			while (area < DEF_NUM_CUR_MAX_H) {
				if (x0 < cur_pos[area][j].x) {
					break;
				}
				area = area + 1;
			}
			if (area == 0) {
				hor_line_y[i][j] = CalcSpline(hline[area][j], x0, cur_pos[area][j].x);
			}
                        			else {
				hor_line_y[i][j] = CalcSpline(hline[area][j], x0, cur_pos[area - 1][j].x);
			}
		}
		// vertical line
		for (j = 0; j < DEF_NUM_CUR_MAX_V; j++) {
			temp_pt[j].x = hor_line_y[i][j];
			temp_pt[j].y = ((float)PS_CUR_DEF_IY[j]*fRatioY)- hor_line_y[i][j];
		}
		if (0 != CalcSplineCoef(temp_pt, DEF_NUM_CUR_MAX_V, vline_mv[i])) {
			err = E_WpErrItv;
			goto end;//return err;		//G100_Doulas_0042 Modify
		}
	}

	for (j = PS_WP_VW_GRD; j >= 0; j--) {
		y0 = (float)(j << Def_Wp_Space_Bits);		//G100_Doulas_0042

		for (i = 0; i < DEF_NUM_CUR_MAX_H; i++) {
			area = 0;
			while (area < DEF_NUM_CUR_MAX_V) {
				if (y0 < cur_pos[i][area].y) {
					break;
				}
				area = area + 1;
			}
			if (area == 0) {
				ver_line_x[i] = CalcSpline(vline[i][area], y0, cur_pos[i][area].y);
			}
			else {
				ver_line_x[i] = CalcSpline(vline[i][area], y0, cur_pos[i][area - 1].y);
			}
		}

		// horizontal line
		for (i = 0; i < DEF_NUM_CUR_MAX_H; i++) {
			temp_pt[i].x = ver_line_x[i];
			temp_pt[i].y = ((float)PS_CUR_DEF_IX[i]*fRatioX) - ver_line_x[i];
		}
		if (0 != CalcSplineCoef(temp_pt, DEF_NUM_CUR_MAX_H, hline_mv)) {
			err = E_WpErrItv;
			goto end;//return err;		//G100_Doulas_0042 Modify
		}

#if WARP_LIMIT_ENABLE
		diffy_y_sum = 0;
		divn = 0;
#endif

		for (i = PS_WP_HW_GRD; i >= 0; i--) {
			x0 = (float)(i << Def_Wp_Space_Bits);	//G100_Doulas_0042
			// x
			area = 0;
			while (area < DEF_NUM_CUR_MAX_H) {
				if (x0 < ver_line_x[area]) {
					break;
				}
				area = area + 1;
			}

			if (area == 0) {
				x = CalcSpline(hline_mv[area], x0, ver_line_x[area]);
			}
			else {
				x = CalcSpline(hline_mv[area], x0, ver_line_x[area - 1]);
			}
			x = x + (float)(i << Def_Wp_Space_Bits);			//G100_Doulas_0042
			x = (x > tblx_max) ? tblx_max : (x < tblx_min) ? tblx_min : x;
			GV_WARP_TABLE_X[i][j] = (int)(x * 16 + 0.5);

			// y
			area = 0;
			while (area < DEF_NUM_CUR_MAX_V) {
				if (y0 < hor_line_y[i][area]) {
					break;
				}
				area = area + 1;
			}

			if (area == 0) {
				y = CalcSpline(vline_mv[i][area], y0, hor_line_y[i][area]);
			}
			else {
				y = CalcSpline(vline_mv[i][area], y0, hor_line_y[i][area - 1]);
			}
			y = y + (float)(j << Def_Wp_Space_Bits);		//G100_Doulas_0042
			y = (y > tbly_max) ? tbly_max : (y < tbly_min) ? tbly_min : y;
			GV_WARP_TABLE_Y[i][j] = (int)(y * 16 + 0.5);

#if WARP_LIMIT_ENABLE
			// Check Limit
			if(bNative == TRUE)
			{
				err = CheckWpLimitA( &i, &j, &diffy_y_sum, &divn, PS_WP_HW_GRD, PS_WP_VW_GRD, PM_IACT_HW, PM_IACT_VW );
				if (err != E_WpNoErr)
					goto end;//return err;		//G100_Doulas_0042 Modify
			}
#endif
		}

#if WARP_LIMIT_ENABLE
		// Check Limit
		if(bNative == TRUE)
		{
			err = CheckWpLimitB(&diffy_y_sum, &divn);
			if (err != E_WpNoErr)
				goto end;//return err;		//G100_Doulas_0042 Modify
		}
#endif
	}

end:	//G100_Doulas_0042

	//釋放指標陣列
	for(i=0; i < Def_HW_Max_GRDs; i++)  	//G100_Doulas_0042
	{
		free(hor_line_y[i]);
		free(vline_mv[i]);
	}

	//釋放指標
	free(hor_line_y);	//G100_Doulas_0042
	free(vline_mv);		//G100_Doulas_0042

	return err;
#endif
}

uint8 CalcWarpTable(Point2f (*PM_GRID)[DEF_NUM_CUR_MAX_V], uint16 PS_WP_HW_GRD, uint16 PS_WP_VW_GRD,
					uint16 PM_IACT_HW, uint16 PM_IACT_VW, float fRatioX, float fRatioY)  //G100_Doulas_0065
{
	uint8 err = E_WpNoErr;

	if (0 != CalcWpCur(PM_GRID, fRatioX, fRatioY))
		err = E_WpErrItv;
	else
		err = CalcWpTable(PM_GRID, PS_WP_HW_GRD, PS_WP_VW_GRD, PM_IACT_HW, PM_IACT_VW, fRatioX, fRatioY);  //G100_Doulas_0065
		return err;
}

void WriteWarpTable(uint8 nWarpSpace)
{
	int x,y;
	int adsft;
	int x2k_ad_jmp;
	int x2k_grd_jmp;
	int x2k_ad_ysft;
	int temp_i;
	int xad;

    dvC789_Write(BN_RTCT, RTCT_THRU);   //A35G2_CDS_Simon_0034

    UINT16 uiDTCT = (UINT16)dvC789_Read(B5_DTCT);   //A35G2_CDS_Simon_0034

	if( nWarpSpace == 32 ) {
		if((m_e3dResId == RES_ID__1080P120) || (m_e3dResId == RES_ID__WUXGA120) || (m_e3dResId == RES_ID__WXGA120))     //R70G2_Doulas_0004 Modify//G100_Doulas_0065
		{
            if(uiDTCT & WARPING_DISTORTION_CORRECTION_TABLE_B)   //A35G2_CDS_Simon_0034
            {
                dvC789_Write(B5_DTCT, DTCT_BASE_ACCESS_ENABLE_32PIEXL_B | DTCT_BASE_WPOUTMD_IMPROVEMENT);
            }
            else
            {
                dvC789_Write(B5_DTCT, DTCT_BASE_ACCESS_ENABLE_32PIEXL_A | DTCT_BASE_WPOUTMD_IMPROVEMENT);
            }

			//dvC789_WriteToBuffer(B5_DTCT, 0x0c42);//120Hz
		}
		else
		{
            if(uiDTCT & WARPING_DISTORTION_CORRECTION_TABLE_B)   //A35G2_CDS_Simon_0034
            {
                dvC789_Write(B5_DTCT, DTCT_BASE_ACCESS_ENABLE_32PIEXL_B);
            }
            else
            {
                dvC789_Write(B5_DTCT, DTCT_BASE_ACCESS_ENABLE_32PIEXL_A);
            }

			//dvC789_WriteToBuffer(B5_DTCT, 0x0842);//60Hz
		}
		adsft = 6;
		x2k_ad_jmp = 2560;
		x2k_grd_jmp = 64;
		x2k_ad_ysft = 1;
	}
	else {
		if((m_e3dResId == RES_ID__1080P120) || (m_e3dResId == RES_ID__WUXGA120) || (m_e3dResId == RES_ID__WXGA120))      //R70G2_Doulas_0004 Modify//G100_Doulas_0065
		{
            if(uiDTCT & WARPING_DISTORTION_CORRECTION_TABLE_B)   //A35G2_CDS_Simon_0034
            {
                dvC789_Write(B5_DTCT, DTCT_BASE_ACCESS_ENABLE_16PIEXL_B | DTCT_BASE_WPOUTMD_IMPROVEMENT);
            }
            else
            {
                dvC789_Write(B5_DTCT, DTCT_BASE_ACCESS_ENABLE_16PIEXL_A | DTCT_BASE_WPOUTMD_IMPROVEMENT);
            }
		    //dvC789_WriteToBuffer(B5_DTCT, 0x0c40);//120Hz
		}
		else
		{
            if(uiDTCT & WARPING_DISTORTION_CORRECTION_TABLE_B)   //A35G2_CDS_Simon_0034
            {
                dvC789_Write(B5_DTCT, DTCT_BASE_ACCESS_ENABLE_16PIEXL_B);
            }
            else
            {
                dvC789_Write(B5_DTCT, DTCT_BASE_ACCESS_ENABLE_16PIEXL_A);
            }
			//dvC789_WriteToBuffer(B5_DTCT, 0x0840);//60Hz
		}
		adsft = 7;
		x2k_ad_jmp = 10240;    //G100_Doulas_0065
		x2k_grd_jmp = 128;
		x2k_ad_ysft = 1;
	}

	for( y = 0; y <= PS_WP_VW_GRD; y++ ) {
		dvC789_WriteToBuffer(B5_DTAD, (y<<adsft)&0xff);
		dvC789_WriteToBuffer(B5_DTAD, (y>>(8-adsft))&0xff);

		xad = 0;
		for( x = 0; x <= PS_WP_HW_GRD; x++ ) {
			if ( xad == x2k_grd_jmp ) {// x >= 2k
				temp_i = x2k_ad_jmp + (y<<x2k_ad_ysft);
				dvC789_WriteToBuffer(B5_DTAD, temp_i&0xff);
				dvC789_WriteToBuffer(B5_DTAD, (temp_i>>8)&0xff);
			}

			// write register
			dvC789_WriteToBuffer(B5_DTDT, GV_WARP_TABLE_X[x][y]&0xff);
			dvC789_WriteToBuffer(B5_DTDT, (GV_WARP_TABLE_X[x][y]>>8)&0xff);
			dvC789_WriteToBuffer(B5_DTDT, GV_WARP_TABLE_Y[x][y]&0xff);
			dvC789_WriteToBuffer(B5_DTDT, (GV_WARP_TABLE_Y[x][y]>>8)&0xff);
			xad++;
		} // end for x
	} // end for y

	dvC789_Buffer_Flush();

	if( Def_Wp_Space == 32 )
	{
		if((m_e3dResId == RES_ID__1080P120) || (m_e3dResId == RES_ID__WUXGA120) || (m_e3dResId == RES_ID__WXGA120))   //R70G2_Doulas_0004 Modify//G100_Doulas_0065
		{
            if(uiDTCT & WARPING_DISTORTION_CORRECTION_TABLE_B)   //A35G2_CDS_Simon_0034
            {
                dvC789_Write(B5_DTCT, DTCT_BASE_ACCESS_DISABLE_32PIEXL_B | DTCT_BASE_WPOUTMD_IMPROVEMENT);
            }
            else
            {
                dvC789_Write(B5_DTCT, DTCT_BASE_ACCESS_DISABLE_32PIEXL_A | DTCT_BASE_WPOUTMD_IMPROVEMENT);
            }
			//dvC789_WriteToBuffer(B5_DTCT, 0x0c02);//120Hz
		}
		else
		{
            if(uiDTCT & WARPING_DISTORTION_CORRECTION_TABLE_B)   //A35G2_CDS_Simon_0034
            {
                dvC789_Write(B5_DTCT, DTCT_BASE_ACCESS_DISABLE_32PIEXL_B);
            }
            else
            {
                dvC789_Write(B5_DTCT, DTCT_BASE_ACCESS_DISABLE_32PIEXL_A);
            }
			//dvC789_WriteToBuffer(B5_DTCT, 0x0802);//60Hz
		}
	}
	else
	{
		if((m_e3dResId == RES_ID__1080P120) || (m_e3dResId == RES_ID__WUXGA120) || (m_e3dResId == RES_ID__WXGA120))   //R70G2_Doulas_0004 Modify//G100_Doulas_0065
		{
            if(uiDTCT & WARPING_DISTORTION_CORRECTION_TABLE_B)   //A35G2_CDS_Simon_0034
            {
                dvC789_Write(B5_DTCT, DTCT_BASE_ACCESS_DISABLE_16PIEXL_B | DTCT_BASE_WPOUTMD_IMPROVEMENT);
            }
            else
            {
                dvC789_Write(B5_DTCT, DTCT_BASE_ACCESS_DISABLE_16PIEXL_A | DTCT_BASE_WPOUTMD_IMPROVEMENT);
            }
			//dvC789_WriteToBuffer(B5_DTCT, 0x0c00);//120Hz
		}
		else
		{
            if(uiDTCT & WARPING_DISTORTION_CORRECTION_TABLE_B)   //A35G2_CDS_Simon_0034
            {
                dvC789_Write(B5_DTCT, DTCT_BASE_ACCESS_DISABLE_16PIEXL_B);
            }
            else
            {
                dvC789_Write(B5_DTCT, DTCT_BASE_ACCESS_DISABLE_16PIEXL_A);
            }
			//dvC789_WriteToBuffer(B5_DTCT, 0x0800);//60Hz
		}
	}
	//flush register data
	dvC789_Buffer_Flush();
}

void WriteWarpLut(uint8 nHIdx, uint8 nVIdx)
{
	uint8 ii=0;

	if (nHIdx > WARP_SHARPNESS_MAX)
		nHIdx = WARP_SHARPNESS_MAX;

	if (nVIdx > WARP_SHARPNESS_MAX)
		nVIdx = WARP_SHARPNESS_MAX;

    for( ii = 0; ii < 24; ii++ ) {
		dvC789_WriteToBuffer(B6_HLUT0+(ii<<8), LUT[nHIdx][ii]);// HLUT0-23
	}
	for( ii = 0; ii < 24; ii++ ) {
		dvC789_WriteToBuffer(B6_VLUT0+(ii<<8), LUT[nVIdx][ii]);// VLUT0-23
	}
	//flush register data
	dvC789_Buffer_Flush();

	dvC789_AdaptiveScaleFilterLutEnable(FALSE);
}

uint8 CalcOsdWarpTable(void)
{
	uint8 err = E_WpNoErr;
	uint8 nX, nY;    //G100_Doulas_0065

	WpmodeChange();
	if(m_bOutputNativeRes==FALSE)
	{
		//Use native res. to calc. PM_GRID[][] and check warp limit
		err = CalcWarpTable(PM_GRID, NATIVE_WP_HW_GRD, NATIVE_WP_VW_GRD, m_nNativeResH, m_nNativeResV, 1.0, 1.0);   //G100_Doulas_0065
		if(err == E_WpNoErr)
		{
			//base on current res. to scale PM_GRID[][] to PM_GRID_TEMP[][] and use this to calc. the warping table
			for( nY = 0; nY < DEF_NUM_CUR_MAX_V; nY++ )   //G100_Doulas_0059
			{
				for( nX = 0; nX < DEF_NUM_CUR_MAX_H; nX++ )   //G100_Doulas_0059
				{
					PM_GRID_TEMP[nX][nY].x = PM_GRID[nX][nY].x*m_f3dResRatioX;  //G100_Doulas_0065
					PM_GRID_TEMP[nX][nY].y = PM_GRID[nX][nY].y*m_f3dResRatioY;  //G100_Doulas_0065
				}
			}
			err = CalcWarpTable(PM_GRID_TEMP, THREED_WP_HW_GRD, THREED_WP_VW_GRD, m_n3dResH, m_n3dResV, m_f3dResRatioX, m_f3dResRatioY);  //G100_Doulas_0065
		}
	}
	else
        {  //G100_Doulas_0065
		//base on current res. to scale PM_GRID[][] to PM_GRID_TEMP[][] and use this to calc. the warping table
		//if Resolution of 3d & Native is different, then check 3D, else check Native only
		if((m_ModeTable[m_e3dResId].nHActive != m_ModeTable[m_eCurrentResId].nHActive) || (m_ModeTable[m_e3dResId].nVActive != m_ModeTable[m_eCurrentResId].nVActive))
		{
			for( nY = 0; nY < DEF_NUM_CUR_MAX_V; nY++ )
			{
				for( nX = 0; nX < DEF_NUM_CUR_MAX_H; nX++ )
				{
					PM_GRID_TEMP[nX][nY].x = PM_GRID[nX][nY].x*m_f3dResRatioX;
					PM_GRID_TEMP[nX][nY].y = PM_GRID[nX][nY].y*m_f3dResRatioY;
				}
			}
			err = CalcWarpTable(PM_GRID_TEMP, THREED_WP_HW_GRD, THREED_WP_VW_GRD, m_n3dResH, m_n3dResV, m_f3dResRatioX, m_f3dResRatioY);
		}
		if(err == E_WpNoErr)
		{
			err = CalcWarpTable(PM_GRID, NATIVE_WP_HW_GRD, NATIVE_WP_VW_GRD, m_nNativeResH, m_nNativeResV, 1.0, 1.0);
		}
	}

#if 1  //(WARP_LIMIT_ENABLE==1)   //A35G2_Simon_0087
	LOG_MSG(db_HAL_WARPING, "CalcOsdWarpTable(), err=%d\r\n", err);
	if( err != E_WpNoErr )
		memcpy(PM_GRID, PM_GRID_OLD, sizeof(PM_GRID_OLD));
	else
		memcpy(PM_GRID_OLD, PM_GRID, sizeof(PM_GRID_OLD));
#endif
	return err;
}

#if 1//A65_OPTOMA_Doulas_0020 black level
void CalcEgbBlackLevelGammaTable(uint8 color, uint8 plt, BOOL allplt, uint8 pnColor[][BLACKLEVEL_PALETTE_RGB])
{
	uint8 lv;
	uint8 col;
	float coef;
	float blacklevel;
	float gm;

	if ( allplt == FALSE ) {
		for ( col = 0; col < 3; col++ ) {
			if ( (col == color) || (color == 3) ) {
				coef = PM_EBIAS_GAMMA[col];
				blacklevel = (float)pnColor[plt][col];
				for ( lv = 0; lv < 16; lv++ ) {
					gm = pow( (pow( ebgmLevel[lv], coef ) + pow( blacklevel, coef )), 1/coef ) - ebgmLevel[lv];
					PM_EBIAS_GMDT[plt][col][lv] = ( gm < 0 ) ? 0 : ( gm > 255 ) ? 255 : (int)gm;
				}
			}
		}
	}
	else {
		for ( plt = 0; plt < 16; plt++ ) {
			for ( col = 0; col < 3; col++ ) {
				coef = PM_EBIAS_GAMMA[col];
				blacklevel = (float)pnColor[plt][col];
				for ( lv = 0; lv < 16; lv++ ) {
					gm = pow( (pow( ebgmLevel[lv], coef ) + pow( blacklevel, coef )), 1/coef ) - ebgmLevel[lv];
					PM_EBIAS_GMDT[plt][col][lv] = ( gm < 0 ) ? 0 : ( gm > 255 ) ? 255 : (int)gm;
				}
			}
		}
	}
}

void WriteEgbBlackLevelGammaTable(uint8 color, uint8 plt, BOOL allplt)
{
	uint32 nRegVal = 0x0;
	uint8 lv;
	uint8 ad;
	uint8 acct;
	//color 0, 1, 2 = R, G, B
	acct = ( color == 0 ) ? 0x1 : ( color == 1 ) ? 0x3 : ( color == 2 ) ? 0x5 : 0x7;
	nRegVal = dvC789_Read(B7_EGBIASCT);
	nRegVal |= (acct << 1);
	dvC789_Write(B7_EGBIASCT, nRegVal);

	if ( allplt == FALSE ) {
		ad = (plt < 16) ? (plt << 4) : 0;
		//EGBIASAD High nibble : table number (16), Low nibble : level bias (16)
		dvC789_WriteToBuffer(B7_EGBIASAD, ad);

		if (color < 3) {
			for ( lv = 0; lv < 16; lv++ ) {
				dvC789_WriteToBuffer( B7_EGBIASDT, PM_EBIAS_GMDT[plt][color][lv]);
			}
		}
		else {
			for ( lv = 0; lv < 16; lv++ ) {
				dvC789_WriteToBuffer( B7_EGBIASDT, PM_EBIAS_GMDT[plt][0][lv]);
			}
		}
	}
	else {
		dvC789_WriteToBuffer( B7_EGBIASAD, 0x00);

		for ( plt = 0; plt < 16; plt++ ) {
			if (color < 3) {
				for ( lv = 0; lv < 16; lv++ ) {
					dvC789_WriteToBuffer( B7_EGBIASDT, PM_EBIAS_GMDT[plt][color][lv]);
				}
			}
			else {
				for ( lv = 0; lv < 16; lv++ ) {
					dvC789_WriteToBuffer( B7_EGBIASDT, PM_EBIAS_GMDT[plt][0][lv]);
				}
			}
		}
	}
	dvC789_Buffer_Flush();
	nRegVal &= (~(acct << 1));
	dvC789_Write(B7_EGBIASCT, nRegVal);
}

void WPLT( uint8 plt_sel, pos_t x, uint16 y )
{// bias area file
	uint32 nData = 0;
	uint32 ad;
	uint16 bbacthw;
	uint8 fill;

	if ( (x.st >= PS_PANEL_ACT_HW) || (x.end < 0) || (x.st > x.end) ) { return; }

	x.st = ( x.st < 0 ) ? 0 : x.st;
	x.end = ( x.end >= PS_PANEL_ACT_HW ) ? PS_PANEL_ACT_HW - 1 : x.end;

	if ( (x.st & 0x01) == 1 ) {
		ad = OSD_BIAS_ADDR + ( x.st >> 1 ) + y * OSD_BIAS_MWI * 256;
		dvC789_WriteToBuffer(B0_CPURAD, ad);
		dvC789_WriteToBuffer(B0_CPUDTCTL, 0x01);
		dvC789_Buffer_Flush();
		nData = dvC789_Read(B0_CPUDT);
		fill = (nData&0x0f) + ( plt_sel << 4 );
		dvC789_WriteToBuffer(B0_CPUWAD, ad);
		dvC789_WriteToBuffer(B0_CPUDT, fill);//Data write
		dvC789_Buffer_Flush();
		x.st = x.st + 1;
	}

	if ( (x.st <= x.end) && ((x.end & 0x01) == 0) )
	{
		ad = OSD_BIAS_ADDR + ( x.end >> 1 ) + y * OSD_BIAS_MWI * 256;
		dvC789_WriteToBuffer(B0_CPURAD, ad);
		dvC789_WriteToBuffer(B0_CPUDTCTL, 0x01);
		dvC789_Buffer_Flush();
		nData = dvC789_Read(B0_CPUDT);
		fill = (nData&0xf0) + plt_sel;
		dvC789_WriteToBuffer(B0_CPUWAD, ad);
		dvC789_WriteToBuffer(B0_CPUDT, fill);//Data write
		dvC789_Buffer_Flush();
		if(x.end != 0)
		{
			x.end = x.end - 1;
		}
	}

	if ( x.st <= x.end ) {
		ad = OSD_BIAS_ADDR + ( x.st >> 1 ) + y * OSD_BIAS_MWI * 256;
		bbacthw = (x.end >> 1) - (x.st >> 1);
		fill = (plt_sel<<4) + plt_sel;
		dvC789_WriteToBuffer(B0_CPUWAD, ad);
		if ( bbacthw == 0 )
		{
			dvC789_WriteToBuffer(B0_CPUDT, fill);//Data write
			dvC789_Buffer_Flush();
		}
		else
		{
			dvC789_WriteToBuffer(B0_OSDFILL, fill);
			dvC789_WriteToBuffer(B0_BBACTHW, bbacthw);
			dvC789_WriteToBuffer(B0_BBACTVW, 0x00);
			dvC789_WriteToBuffer(B0_OSDCT, 0x01);//FILL write
			dvC789_Buffer_Flush();
			do {nData = dvC789_Read(B0_BOSTAT);}while(nData & 0x01);
			dvC789_Write(B0_OSDCT, 0x00);
		}
	}
}

//串列建立函數
Node* CreateList(PointT *pnArr, uint8 nLen)
{
	uint8 i = 0;
	Node *pFirst=NULL,*pCurrent,*pPrevious;    //A35G2_Simon_0070 for cppcheck

	for(i=0;i<nLen;i++)
	{
		pCurrent = (Node *) malloc(sizeof(Node));//建立新節點
		pCurrent->stPos.x = pnArr[i].x;  //設定節點的資料成員
		pCurrent->stPos.y = pnArr[i].y;  //設定節點的資料成員
		pCurrent->pNext = NULL;
		pCurrent->pPrev = NULL;
		if(i==0)
		{
			pFirst = pCurrent;  //如果是第一個成員把指標frist指向目前的節點
		}
		else
		{
			pPrevious->pNext = pCurrent;//把前一個的next指向目前的節點
			pCurrent->pPrev = pPrevious;  //把目前的節點的previous指向前一個
		}
		pCurrent->pNext = NULL; //把目前的節點的next指向NULL
		pPrevious = pCurrent; //把前一個節點設成目前的節點
	}
	return pFirst;
}

//串列列印函數
void PrintList(Node *pFirst)
{

	if(pFirst==NULL)
	{
		LOG_MSG(db_ALWAYS,"List is empty!\n");
	}
	else
	{
		while(pFirst!=NULL)
		{
			LOG_MSG(db_ALWAYS,"X,Y = %d, %d \t", pFirst->stPos.x, pFirst->stPos.y);
			pFirst=pFirst->pNext;
		}
		LOG_MSG(db_ALWAYS,"\r\n");
	}
}

//釋放串列記憶空間函數
void FreeList(Node *pFirst)
{
	Node *pTmp;

	while(pFirst!=NULL)
	{
		pTmp=pFirst;
		pFirst=pFirst->pNext;
		free(pTmp);
	}
}

 //搜尋節點函數
Node* SearchNode(Node *pFirst, Node *stNode)
{

	while(pFirst!=NULL)
	{
		if((pFirst->pNext == stNode->pNext) && (pFirst->pPrev == stNode->pPrev))
		{
			return pFirst;  //傳回node為該節點的位址
		}
		else
		{
			pFirst=pFirst->pNext; //否則將指標指向下一個節點
		}
	}
	return NULL; //如果找不到符合的節點，則傳回NULL
}

 //插入節點函數
void InsertNode(Node *pNode, PointT nPoint)
{
	Node *pNewNode;

	pNewNode=(Node *) malloc(sizeof(Node));
	pNewNode->stPos.x=nPoint.x;
	pNewNode->stPos.y=nPoint.y;
	//relink new node and next node
	pNewNode->pNext=pNode->pNext;  //origin node's next pointer point to new node
	if(pNode->pNext != NULL)  //this is not last node
	{
		(pNode->pNext)->pPrev = pNewNode;  //origin next node's previous point to new node
	}
	//relink new node and current node
	pNode->pNext=pNewNode;  //origin node's next pointer point to new node
	pNewNode->pPrev = pNode;  //new node previous point to origin node
	m_pnCurrentNode[m_eAreaSelection] = pNewNode;

}

//刪除節點函數
Node* DeleteNode(Node *pFirst, Node *pNode)
{

	if(pFirst==NULL)
	{ //如果串列是空的，則印出Nothing to delete! */
		//LOG_MSG(db_ALWAYS,"Nothing to delete!\r\n");
		return NULL;
	}
	if(pNode==pFirst)
	{  //如果刪除的是第一個節點
		//LOG_MSG(db_ALWAYS, "Delete point first node!!\r\n");
		pFirst=pFirst->pNext;//把first指向下一個節點(NULL)
		pFirst->pPrev = NULL;
	}
	else
	{
		(pNode->pPrev)->pNext = pNode->pNext; //重新設定ptr的next成員
		if(pNode->pNext != NULL)
		{
			(pNode->pNext)->pPrev = pNode->pPrev;  //relink next node's previous pointer
		}
	}
	free(pNode);

	return pFirst;
}

Node* GetLastNodeOfList(Node* pFirst)
{

	if(pFirst==NULL)
	{ //如果串列是空的，則印出Nothing to get! */
		//LOG_MSG(db_ALWAYS,"Nothing to get!\r\n");
		return NULL;
	}

	while (pFirst->pNext != NULL)
	{
		pFirst = pFirst->pNext;
	}

	return pFirst;
}

Node* GetNextNode(Node* pFirst, Node* stNode)
{
	Node *pNextNode;

	if((stNode->pNext == NULL) && (stNode->pPrev== NULL))
	{
		//LOG_MSG(db_ALWAYS, "Current node is invalid!!\r\n");
		return NULL;
	}
	else
	{
		if(stNode->pNext != NULL)
		{
			pNextNode = stNode->pNext;
			//LOG_MSG(db_ALWAYS, "Next node (%d, %d)!!\r\n", pNextNode->stPos.x, pNextNode->stPos.y);
		}
		else
		{
			//LOG_MSG(db_ALWAYS, "Current node is last node!!\r\n");
			return pFirst;
		}

	}
	return pNextNode;

}

Node* GetPreviousNode(Node* pFirst, Node* stNode)
{
	Node *pPreviousNode;

	if((stNode->pNext == NULL) && (stNode->pPrev== NULL))
	{
		//LOG_MSG(db_ALWAYS, "Current node is invalid!!\r\n");
		return NULL;
	}
	else
	{
		if(stNode->pPrev != NULL)
		{
			pPreviousNode = stNode->pPrev;
			//LOG_MSG(db_ALWAYS, "Previous node (%d, %d)!!\r\n", pPreviousNode->stPos.x, pPreviousNode->stPos.y);
		}
		else
		{
			//LOG_MSG(db_ALWAYS, "Current node is first node!!\r\n");
			//return NULL;
			pPreviousNode = GetLastNodeOfList(pFirst);
		}

	}
	return pPreviousNode;

}

#if BLACKLEVEL_SEL_MODE			//A65_OPTOMA_Doulas_0146  //A65_OPTOMA_CL_0003
Node* GetNodeByDirection(eDIR eDir, uint8 nCnt)
{
	Node* pTargetNode;
	Node* pTempFirstNode;
	Node* pTempEndNode;

	uint8 i, j;
	uint8 nIndex[BLACKLEVEL_SEARCH_LEVEL] = {0, 0, 0, 0, 0};
	uint16 nX, nY, nSx, nSy;
	float nRange, nTempRange;

	Node* pnSearchNode[BLACKLEVEL_SEARCH_LEVEL][nCnt];

	pTempFirstNode = m_pFirstNode[m_eAreaSelection];
	if(m_ePatternType == PAT_TYPE__BLACKLEVEL_ADD_POINT)
	{
		pTempEndNode = GetNextNode(m_pFirstNode[m_eAreaSelection], m_pnCurrentNode[m_eAreaSelection]);
		nX = (m_pnCurrentNode[m_eAreaSelection]->stPos.x + pTempEndNode->stPos.x)/2;  //x of current point, current line = (current point x + next point of current point x) /2
		nY = (m_pnCurrentNode[m_eAreaSelection]->stPos.y + pTempEndNode->stPos.y)/2;  //y of current point, current line = (current point y + next point of current point y) /2
	}
	else
	{
		nX = m_pnCurrentNode[m_eAreaSelection]->stPos.x;
		nY = m_pnCurrentNode[m_eAreaSelection]->stPos.y;
	}
	for(i=0;i<nCnt;i++)
	{
		//Current node as center and search in range of +-45 degree in each direction
		if(m_ePatternType == PAT_TYPE__BLACKLEVEL_ADD_POINT)
		{
			pTempEndNode = GetNextNode(m_pFirstNode[m_eAreaSelection], pTempFirstNode);
			nSx= (pTempFirstNode->stPos.x + pTempEndNode->stPos.x)/2;
			nSy= (pTempFirstNode->stPos.y + pTempEndNode->stPos.y)/2;
		}
		else
		{
			nSx= pTempFirstNode->stPos.x;
			nSy= pTempFirstNode->stPos.y;
		}
		if(((eDir == DIR__UP) && (nSy < nY)) || ((eDir == DIR__DOWN) && (nSy > nY)))//A65_OPTOMA_CL_0004
		{
			if (abs(nSy - nY)*10 >= abs(nSx - nX)*105) //slope >= 10.5
			{
				pnSearchNode[0][nIndex[0]] = pTempFirstNode;
				nIndex[0]++;
			}
			else if (abs(nSy - nY)*10 >= abs(nSx - nX)*49) //slope >= 4.9 & slope < 10.5
			{
				pnSearchNode[1][nIndex[1]] = pTempFirstNode;
				nIndex[1]++;
			}
			else if (abs(nSy - nY)*10 >= abs(nSx - nX)*24) //slope >= 2.4 & slope < 4.9
			{
				pnSearchNode[2][nIndex[2]] = pTempFirstNode;
				nIndex[2]++;
			}
			else if (abs(nSy - nY) >= abs(nSx - nX)) //slope >= 1 & slope < 2.4
			{
				pnSearchNode[3][nIndex[3]] = pTempFirstNode;
				nIndex[3]++;
			}
			else
			{
				//slope >= 0 & slope < 1
				pnSearchNode[4][nIndex[4]] = pTempFirstNode;
				nIndex[4]++;
			}
		}
		else if(((eDir == DIR__LEFT) && (nSx < nX)) || ((eDir == DIR__RIGHT) && (nSx > nX)))//A65_OPTOMA_CL_0004
		{
			if (abs(nSy - nY)*100 <= abs(nSx - nX)*9) //slope >= 0 & slope <= 0.09
			{
				pnSearchNode[0][nIndex[0]] = pTempFirstNode;
				nIndex[0]++;
			}
			else if (abs(nSy - nY)*10 <= abs(nSx - nX)*2) //slope >= 0.09 & slope <= 0.2
			{
				pnSearchNode[1][nIndex[1]] = pTempFirstNode;
				nIndex[1]++;
			}
			else if (abs(nSy - nY)*10 <= abs(nSx - nX)*4) //slope >= 0.2 & slope <= 0.4
			{
				pnSearchNode[2][nIndex[2]] = pTempFirstNode;
				nIndex[2]++;
			}
			else if (abs(nSy - nY) <= abs(nSx - nX)) //slope >= 0.4 & slope <= 1
			{
				pnSearchNode[3][nIndex[3]] = pTempFirstNode;
				nIndex[3]++;
			}
			else
			{
				//slope 0 ~ infinite
				pnSearchNode[4][nIndex[4]] = pTempFirstNode;
				nIndex[4]++;
			}
		}
		//next node to qualify
		pTempFirstNode = pTempFirstNode->pNext;
	}
	for(j= 0; j<BLACKLEVEL_SEARCH_LEVEL; j++)
	{
		if(nIndex[j] != 0)
		{
			if(m_ePatternType == PAT_TYPE__BLACKLEVEL_ADD_POINT)
			{
				pTempEndNode = GetNextNode(m_pFirstNode[m_eAreaSelection], pnSearchNode[j][0]);
				nSx= (pnSearchNode[j][0]->stPos.x + pTempEndNode->stPos.x)/2;
				nSy= (pnSearchNode[j][0]->stPos.y + pTempEndNode->stPos.y)/2;
			}
			else
			{
				nSx= pnSearchNode[j][0]->stPos.x;
				nSy= pnSearchNode[j][0]->stPos.y;
			}

			nRange = pow(abs(nSx - nX), 2) + pow(abs(nSy - nY), 2);
			pTargetNode = pnSearchNode[j][0];
			for(i = 0; i< nIndex[j] ; i++)
			{
				if(m_ePatternType == PAT_TYPE__BLACKLEVEL_ADD_POINT)
				{
					pTempEndNode = GetNextNode(m_pFirstNode[m_eAreaSelection], pnSearchNode[j][i]);
					nSx= (pnSearchNode[j][i]->stPos.x + pTempEndNode->stPos.x)/2;
					nSy= (pnSearchNode[j][i]->stPos.y + pTempEndNode->stPos.y)/2;
				}
				else
				{
					nSx= pnSearchNode[j][i]->stPos.x;
					nSy= pnSearchNode[j][i]->stPos.y;
				}
				nTempRange = pow(abs(nSx - nX), 2) + pow(abs(nSy - nY), 2);
				if( nTempRange < nRange)
				{
					nRange = nTempRange;
					pTargetNode = pnSearchNode[j][i];
				}


			}
			return pTargetNode;
		}
		else
		{
			continue;
		}
	}

	//no node to move, return current one
	return m_pnCurrentNode[m_eAreaSelection];

}
#endif

uint8 GetTotalCountOfList(Node* pFirst)
{
	uint8 nCount = 1;  //calculation result below is not including first element.

	if(pFirst == NULL)
		return 0;
	while (pFirst->pNext != NULL)
	{
		nCount++;
		pFirst = pFirst->pNext;
	}
	//LOG_MSG(db_ALWAYS, "Node number = %d !!\r\n", nCount);
	return nCount;
}
BOOL RemovePointFromList(Node** pFirst, Node* stNode)  //A65_OPTOMA_Doulas_0024
{
	Node *pNode;
	BOOL bResult = FALSE;
	uint8 nCount = 0;

	pNode = SearchNode(*pFirst,stNode); //找出節點值的位址 //A65_OPTOMA_Doulas_0024
	if(pNode != NULL)
	{
		*pFirst = DeleteNode(*pFirst,pNode); //刪除節點  //A65_OPTOMA_Doulas_0024
		bResult = TRUE;
	}
	else
	{
		LOG_MSG(db_ASSERT, "No such node to delete !!\r\n");	//A65_OPTOMA_Doulas_0094 Modify
	}
	return bResult;

}
BOOL AddPointToList(Node* pFirst, Node* stNode, PointT nAddPoint)
{
	Node *pNode;
	BOOL bResult = FALSE;
	uint8 nCount = 0;

	pNode=SearchNode(pFirst,stNode); //找出目前節點的位址

	if(pNode != NULL)
	{
		InsertNode(pNode,nAddPoint); //將新節點插入在目前節點之後
		bResult = TRUE;
	}
	else
	{
		LOG_MSG(db_ASSERT, "Can not find entry node (%d, %d) to add !!\r\n", stNode->stPos.x, stNode->stPos.y);		//A65_OPTOMA_Doulas_0094 Modify
	}
	return bResult;

}

YBoundary FindYBoundary(Node* pFirst)
{
	uint8 nCnt = 0;
	uint8 i = 0;
	uint16 nYMin = 1920;
	uint16 nYMax = 0;
	YBoundary nRptPoint;

	nCnt = GetTotalCountOfList(pFirst);
	for(i=0; i< nCnt; i++)
	{
		if(nYMin > pFirst->stPos.y)
			nYMin = pFirst->stPos.y;
		if(nYMax < pFirst->stPos.y)
			nYMax = pFirst->stPos.y;
		pFirst = pFirst->pNext;
	}
	//LOG_MSG(db_ALWAYS, "Find Ymin = %d, Ymax = %d!!\r\n", nYMin, nYMax);
	nRptPoint.nYMin = nYMin;
	nRptPoint.nYMax = nYMax;

	return nRptPoint;
}
void ConvertLinkListToArray(Node* pFirst)
{
	uint8 i = 0;

	m_stVertex.nPointNum = GetTotalCountOfList(pFirst);
	for(i=0; i< m_stVertex.nPointNum; i++)
	{
		m_stVertex.nPoint[i].x = pFirst->stPos.x;
		m_stVertex.nPoint[i].y = pFirst->stPos.y;
		pFirst = pFirst->pNext;
	}
}

void Convert2dTo3d(void)
{
	uint8 i = 0;

	if (m_bOutputNativeRes == FALSE)
	{
		for(i=0; i< m_stVertex.nPointNum; i++)
		{
			m_stVertex.nPoint[i].x = (uint16)(m_stVertex.nPoint[i].x * m_f3dResRatioX);
			m_stVertex.nPoint[i].y = (uint16)(m_stVertex.nPoint[i].y * m_f3dResRatioY);
		}
	}
}

/*
目的：找出水平線Y = a與多邊形PolyDots的交點
輸入：點(X,Y), 多邊形頂點集(pPolyDots[])，及頂點數量(lPolyDotsNum)
輸出：交點座標(最大交點數量為頂點數量-1)

注意：
1. 只算Y值在此兩頂點y值之間的線段
2. 要注意頂點或與邊線重疊
3. 將Y值直接帶入線段，即可求出X值(交點)(斜率不為0或無窮大) x = (y-b)/k
4. 斜率等於0時，則此線段的兩邊頂點即為2個交點
5. 斜率等於無窮大時，交點為兩頂點中的x值，y值為掃瞄值
6. 當找出來的交點數大於2時，需判斷那些線段在多邊形內部，
此時採用的方式為依序將這些交點的x方向從小到大的兩相鄰點取中間值，
再帶入判斷點是否在多邊形內部的判斷式，
可得出此相鄰兩點所連成的線段是否在多邊形內部
*/
BOOL InterSectionWithPoly(uint16 nY, PointT pPolyDots[], uint8 nPolyDotsNum)
{
	uint8 i = 0;
	uint8 j = 0;
	float fStartX, fStartY, fEndX, fEndY;
	float fK;
	float fY;
	uint16 nStartX, nStartY, nEndX, nEndY;
	BOOL bResult = FALSE;

	//init point array value to unvalid value. i.e. >1920
	m_stIntersectedPoint.nPointNum = 0;
	for(i = 0; i< MAX_INTERSECTED_POINT; i++)
	{
		m_stIntersectedPoint.nPoint[i] = BLACKLEVEL_UNVALID_INTERSECTED_POINT_VALUE;
	}

	for(i = 0, j = nPolyDotsNum-1; i < nPolyDotsNum; j = i++)
	{
		nStartX = pPolyDots[i].x;
		nStartY = pPolyDots[i].y;
		nEndX = pPolyDots[j].x;
		nEndY = pPolyDots[j].y;

		//y in the range of Y-axis between adjacent vertexes
		if((nStartY < nY && nEndY >= nY) || (nStartY >= nY && nEndY < nY) || (nStartY > nY && nEndY <= nY) || (nStartY <= nY && nEndY > nY))
		{
			if (nEndX == nStartX)//infinite slope
			{
				m_stIntersectedPoint.nPoint[m_stIntersectedPoint.nPointNum] = nStartX;
				//return a value to say x value of point of intersection is x1(because x1 = x2)
			}
			else //0 or normal slope value
			{
				if(nEndY == nStartY)//slope = 0
				{
					m_stIntersectedPoint.nPoint[m_stIntersectedPoint.nPointNum] = nStartX;
					//return a value to say x value of points of intersection are x1 and x2
					//keep x1, discard x2 because x1 and x2 are corner points. it will be included in adjacent line.
				}
				else
				{
					fStartX = (float)nStartX;
					fStartY = (float)nStartY;
					fEndX = (float)nEndX;
					fEndY = (float)nEndY;
					fY = (float)nY;
					fK = (fEndY - fStartY) / (fEndX - fStartX);  //slope
					m_stIntersectedPoint.nPoint[m_stIntersectedPoint.nPointNum] = (uint16)((fY - (fStartY-fK*fStartX))/fK);  //x=(y-b)/k (i.e. y=kx+b)
					//return a value to say x value of points of intersection is x=(y-b)/k
				}
			}
			m_stIntersectedPoint.nPointNum++;  //+1 if one intersected point has been found
		}
 	}
	return bResult;
}

/*
目的：判斷點(X,Y)是否在多邊形PolyDots內
輸入：點(X,Y), 多邊形頂點集(pPolyDots[])，及頂點數量(lPolyDotsNum)
輸出：TRUE(inside), FALSE(outside), there is not checking point on the line
注意：
1. 只算Y值在此兩頂點間的線段
*/
BOOL JudgePntInPoly(float fX, uint16 nY, PointT pPolyDots[], uint8 nPolyDotsNum)
{

	uint8 i = 0;
	uint8 j = 0;
	float fStartX, fStartY, fEndX, fEndY;
	float fSlope;
	float fY;
	uint16 nStartX, nStartY, nEndX, nEndY;
	BOOL bPntInPoly = FALSE;

	for(i = 0, j = nPolyDotsNum-1; i < nPolyDotsNum; j = i++)
	{
		nStartX = pPolyDots[i].x;
		nStartY = pPolyDots[i].y;
		nEndX = pPolyDots[j].x;
		nEndY = pPolyDots[j].y;

		//same Y value of 2 vertexes and start point in the left side of end point
		if((nStartY == nEndY) && (nStartX < nEndX))
		{
			//Point on the line
			if((nY == nStartY) && (((float)nStartX < fX) && (fX < (float)nEndX)))
				return TRUE;
		}
		//general check for y in the range of Y-axis between adjacent vertexes
		if((nStartY < nY && nEndY >= nY) || (nStartY >= nY && nEndY < nY))
		{
			fStartX = (float)nStartX;
			fStartY = (float)nStartY;
			fEndX = (float)nEndX;
			fEndY = (float)nEndY;
			fY = (float)nY;

			fSlope = fStartX + (fY-fStartY)*(fEndX-fStartX)/(fEndY-fStartY);
			if(fabs(fX-fSlope)<=0.0001)
			{
				return TRUE;
			}
			if(fX < fSlope)
			{
				bPntInPoly = !bPntInPoly;
			}
		}
 	}
	return bPntInPoly;

}

void SortBySelectionMethod(uint16 nCount)
{
	uint16 i = 0;
	uint16 nPosition, nData, nTemp;

	//start sorting
	for (i = 0; i < (nCount - 1); i++) // finding minimum element (n-1) times
	{
		nPosition = i;

		for (nData = i + 1; nData < nCount; nData++)
		{
			if (m_stIntersectedPoint.nPoint[nPosition] > m_stIntersectedPoint.nPoint[nData])
				nPosition = nData;
		}
		if (nPosition != i)
		{
			nTemp = m_stIntersectedPoint.nPoint[i];
			m_stIntersectedPoint.nPoint[i] = m_stIntersectedPoint.nPoint[nPosition];
			m_stIntersectedPoint.nPoint[nPosition] = nTemp;
		}
	}

}

void EliminateRepeatPoint(uint16 pArray[], uint8 nArrayNum, XValueSet* pValueSet)
{
	uint16 j = 0;

	for(j = 0; j<nArrayNum; j++)
	{
		if(j==0)
		{
 			pValueSet->nXValue[pValueSet->nPointNum] = pArray[j];
			pValueSet->nPointNum++;
		}
		else
		{
			//put into array if value is different with previous
			if(pArray[j] != pArray[j-1])
			{
				pValueSet->nXValue[pValueSet->nPointNum] = pArray[j];
				pValueSet->nPointNum++;
			}
 		}
  	}

}

void CheckPointAndDraw(uint8 nPaletteSelect, uint16 nY, PointT pArray[], uint8 nArrayNum, XValueSet* pValueSet)
{
	uint8 nArrayCnt = 0;
	float fCheckX = 0.0;
	uint16 i = 0;
	pos_t nXValue;

	nArrayCnt = pValueSet->nPointNum;

	for(i=0; i<nArrayCnt; i++)
	{
		if(i+1 == nArrayCnt)
		{
			//if i is the last point
			// 1 point
			//use i as a point
			nXValue.st = pValueSet->nXValue[i];
			nXValue.end = pValueSet->nXValue[i];
			WPLT(nPaletteSelect, nXValue, nY);
		}
		else
		{
			fCheckX = ((float)pValueSet->nXValue[i] + (float)pValueSet->nXValue[i+1])/2;
			if(JudgePntInPoly(fCheckX, nY, pArray, nArrayNum))
			{
				// 1 line sector
				//use i & i+1 as a line
				nXValue.st = pValueSet->nXValue[i];
				nXValue.end = pValueSet->nXValue[i+1];
				WPLT(nPaletteSelect, nXValue, nY);
			}
			else
			{
				// 1 point
				//use i as a point
				nXValue.st = pValueSet->nXValue[i];
				nXValue.end = pValueSet->nXValue[i];
				WPLT(nPaletteSelect, nXValue, nY);
			}
		}
	}

}

void ClearBlackLevelArea(void)
{
	uint32 nData = 0;
	uint32 ad;

	dvC789_WriteToBuffer(B0_BBWMWI, 0x08);
	ad = OSD_BIAS_ADDR;
	dvC789_WriteToBuffer(B0_CPUWAD, ad);
	dvC789_WriteToBuffer(B0_OSDFILL, 0x00);
	dvC789_WriteToBuffer(B0_BBACTHW, PS_PANEL_ACT_HW>>1);
	dvC789_WriteToBuffer(B0_BBACTVW, PS_PANEL_ACT_VW);
	dvC789_WriteToBuffer(B0_OSDCT, 0x01);//FILL write
	dvC789_Buffer_Flush();

	do {nData = dvC789_Read(B0_BOSTAT);}while(nData & 0x01);
	dvC789_WriteToBuffer(B0_OSDCT, 0x00);
	dvC789_WriteToBuffer(B0_BBWMWI, m_nLineFeed);
	dvC789_Buffer_Flush();
}

void WriteBlackLevelArea(void)
{
	uint16 i = 0;
	uint16 j = 0;
	uint8 k = 0;
	uint8 nPaletteSelect = 0;
	YBoundary nPt;

	ClearBlackLevelArea();

	for(k=0; k<BLACKLEVEL_AREA__NUM; k++)
	{
		if(m_stWarpConfig.stOsd.stBlackLevel.bEnable[k] == TRUE)
		{
			nPaletteSelect = k+1;
			ConvertLinkListToArray(m_pFirstNode[k]);
			Convert2dTo3d();
			nPt = FindYBoundary(m_pFirstNode[k]);

			//SCAN Y direction
			for(i = nPt.nYMin; i< nPt.nYMax; i++)
			{
				//find intersection point
				InterSectionWithPoly(i, m_stVertex.nPoint, m_stVertex.nPointNum);
				if(m_stIntersectedPoint.nPointNum > 0)
				{
					//sort order
					SortBySelectionMethod(m_stIntersectedPoint.nPointNum);
					//eliminate repeat point
					m_stXValueSet.nPointNum = 0;
					for(j=0;j<MAX_INTERSECTED_POINT;j++)
					{
						m_stXValueSet.nXValue[j] = 0;
					}
					EliminateRepeatPoint(m_stIntersectedPoint.nPoint, m_stIntersectedPoint.nPointNum, &m_stXValueSet);

					if(m_stXValueSet.nPointNum > 0)
					{
						CheckPointAndDraw(nPaletteSelect, i, m_stVertex.nPoint, m_stVertex.nPointNum, &m_stXValueSet);
					}
				}
			}
		}
	}
}

void ApplyBlackLevel(eAPPLY_BLACKLEVEL eFlag)
{
	if((eFlag==APPLY_BLACKLEVEL__DISABLE) || (m_stWarpConfig.eWarpCtrl == WARP_CTRL__BASIC))//A65_OPTOMA_CL_0013
	{
		dvC789_Write(B7_EGBIASCT, 0x0050);
		//LOG_MSG(db_ALWAYS, "ApplyBlackLevel disabled = %d\r\n", m_bBlackLevelOn);
	}
	else if(m_stWarpConfig.eWarpCtrl == WARP_CTRL__ADVANCED)
	{
		if ((m_stWarpConfig.stOsd.stBlackLevel.bEnable[BLACKLEVEL_AREA__BOTTOM] == FALSE &&  //A65_OPTOMA_CL_0013
			m_stWarpConfig.stOsd.stBlackLevel.bEnable[BLACKLEVEL_AREA__TOP] == FALSE))
		{
			dvC789_Write(B7_EGBIASCT, 0x0050);
		}
		else if(eFlag == APPLY_BLACKLEVEL__ENABLE)
		{
			dvC789_Write(B7_EGBIASCT, 0x0051);
		}
		else if(eFlag == APPLY_BLACKLEVEL__BY_CONFIG)
		{
			dvC789_Write(B7_EGBIASCT, 0x0051);
			WriteEgbBlackLevelGammaTable(0, 0, TRUE);
			WriteEgbBlackLevelGammaTable(1, 0, TRUE);
			WriteEgbBlackLevelGammaTable(2, 0, TRUE);
			WriteBlackLevelArea();
			m_bRewriteOsdBlackLevelArea = FALSE;
		}
	}
	else if((m_stWarpConfig.eWarpCtrl == WARP_CTRL__AP) && (m_ePatternType == PAT_TYPE__OFF))  //A65_OPTOMA_CL_0010
	{
	#ifndef CUSTOM_CHRISTIE  //A35G2_CDS_CL_0002
            if(halWarping_TwistLinkFlag_Get() == FALSE)//A65_OPTOMA_CL_0016
            {
                LOG_MSG(db_HAL_WARPING, "%s, Warp control = %d\r\n", __FUNCTION__, m_stWarpConfig.eWarpCtrl);
                m_bApBlacklevelApply = TRUE;
            }
	#endif
	}
}

void UpdateBlackLevelNode(OSD_BlackLevel *pstBlackLevelFlash)
{
	uint8 nCnt = 0;
	uint8 i = 0;
	uint8 j = 0;
	Node* pNode;

	for(j=0; j<BLACKLEVEL_AREA__NUM; j++)
	{
		pNode = m_pFirstNode[j];
		nCnt = pstBlackLevelFlash->nPointNum[j];
		for(i=0; i< nCnt; i++)
		{
			pstBlackLevelFlash->pnPointList[j][i].x = pNode->stPos.x;
			pstBlackLevelFlash->pnPointList[j][i].y = pNode->stPos.y;
			pNode = pNode->pNext;
		}
	}
}

void InitBlackLevelNode(eBLACKLEVEL_AREA eAreaSelect)
{
	uint8 nCnt = 0;

	FreeList(m_pFirstNode[eAreaSelect]);
	m_pFirstNode[eAreaSelect] = NULL;
	nCnt = m_stWarpConfig.stOsd.stBlackLevel.nPointNum[eAreaSelect];
	if(nCnt != 0)
	{
		m_pFirstNode[eAreaSelect] = CreateList(m_stWarpConfig.stOsd.stBlackLevel.pnPointList[eAreaSelect], nCnt);
		m_pnCurrentNode[eAreaSelect] = m_pFirstNode[eAreaSelect];
	}
	CalcEgbBlackLevelGammaTable(3, eAreaSelect+1, FALSE, m_stWarpConfig.stOsd.stBlackLevel.pnColor);
}
#endif

void WriteColorReg(UINT16 WritePaletteIdx , eCOLOR_IDX eColor)
{
	UINT8 PLTData[3] = {0x00, 0x00, 0x00};

    LOG_MSG(db_HAL_WARPING, "(func:%s,line:%d) %d %d\n", __FUNCTION__, __LINE__, WritePaletteIdx, eColor);

	switch(eColor)
	{
		default:
		    LOG_MSG(db_HAL_WARPING, "(func:%s,line:%d)undefined eColor %d\n", __FUNCTION__, __LINE__, eColor);

		case COLOR_IDX__BLACK:
			PLTData[0] = 0x00;
			PLTData[1] = 0x00;
			PLTData[2] = 0x00;
			break;
		case COLOR_IDX__RED:
			PLTData[0] = 0xFF;	//R
			PLTData[1] = 0x00;	//G
			PLTData[2] = 0x00;	//B
			break;
		case COLOR_IDX__GREEN:
			PLTData[0] = 0x00;	//R
			PLTData[1] = 0xFF;	//G
			PLTData[2] = 0x00;	//B
			break;
		case COLOR_IDX__BLUE:
			PLTData[0] = 0x00;	//R
			PLTData[1] = 0x00;	//G
			PLTData[2] = 0xFF;	//B
			break;
		case COLOR_IDX__CYAN:
			PLTData[0] = 0x00;	//R
			PLTData[1] = 0xFF;	//G
			PLTData[2] = 0xFF;	//B
			break;
		case COLOR_IDX__MAGENTA:
			PLTData[0] = 0xFF;	//R
			PLTData[1] = 0x00;	//G
			PLTData[2] = 0xFF;	//B
			break;
		case COLOR_IDX__YELLOW:
			PLTData[0] = 0xFF;	//R
			PLTData[1] = 0xFF;	//G
			PLTData[2] = 0x00;	//B
			break;
		case COLOR_IDX__WHITE:
			PLTData[0] = 0xFF;	//R
			PLTData[1] = 0xFF;	//G
			PLTData[2] = 0xFF;	//B
			break;
		case COLOR_IDX__DARK_GREEN:
			PLTData[0] = 0x00;	//R
			PLTData[1] = 0x3F;	//G
			PLTData[2] = 0x00;	//B
			break;
		case COLOR_IDX__DARK_MAGENTA:
			PLTData[0] = 0x3F;	//R
			PLTData[1] = 0x00;	//G
			PLTData[2] = 0x3F;	//B
			break;
		case COLOR_IDX__DARK_RED:
			PLTData[0] = 0x3F;	//R
			PLTData[1] = 0x00;	//G
			PLTData[2] = 0x00;	//B
			break;
		case COLOR_IDX__DARK_CYAN:
			PLTData[0] = 0x00;	//R
			PLTData[1] = 0x3F;	//G
			PLTData[2] = 0x3F;	//B
			break;
		case COLOR_IDX__BLACKLEVEL_GREEN:  //A65_OPTOMA_Doulas_0020
			PLTData[0] = 0x00;	//R
			PLTData[1] = 0x20;	//G
			PLTData[2] = 0x00;	//B
			break;
		case COLOR_IDX__BLACKLEVEL_MAGENTA:  //A65_OPTOMA_Doulas_0020
			PLTData[0] = 0x20;	//R
			PLTData[1] = 0x00;	//G
			PLTData[2] = 0x20;	//B
			break;
		case COLOR_IDX__BLACKLEVEL_RED:  //A65_OPTOMA_Doulas_0020
			PLTData[0] = 0x20;	//R
			PLTData[1] = 0x00;	//G
			PLTData[2] = 0x00;	//B
			break;
		case COLOR_IDX__BLACKLEVEL_CYAN:  //A65_OPTOMA_Doulas_0020
			PLTData[0] = 0x00;	//R
			PLTData[1] = 0x20;	//G
			PLTData[2] = 0x20;	//B
			break;
		case COLOR_IDX__BLACK_BKG:
			PLTData[0] = 0x00;	//R
			PLTData[1] = 0x00;	//G
			PLTData[2] = 0x00;	//B
			break;

		case COLOR_IDX__GREY192:     //H2PF_Simon_0037
			PLTData[0] = 192;	//R
			PLTData[1] = 192;	//G
			PLTData[2] = 192;	//B
			break;

		case COLOR_IDX__USER_DEFINE:
		    PLTData[0] = m_stUserDefineOsdPalette.ucRed;
		    PLTData[1] = m_stUserDefineOsdPalette.ucGreen;
		    PLTData[2] = m_stUserDefineOsdPalette.ucBlue;
		    break;
	}


    if(WritePaletteIdx < COLOR_IDX__BLINK)
    {
        sPaletteSingleInfo sPlt = {WritePaletteIdx, PLTData[0], PLTData[1], PLTData[2]};
        halWarpOSD_Config(eGCI_OSD, eWOC_Palette_Single, &sPlt, eWRTIE_TO_BUFFER);
	}
	else if(WritePaletteIdx == COLOR_IDX__BLINK)  //for blink color
	{
        halWarpOSD_Config(eGCI_OSD, eWOC_BlinkPaletteSet0_0, PLTData, eWRTIE_TO_BUFFER);
	}
	else if(WritePaletteIdx == COLOR_IDX__BLACK_BLINK)  //for blink color
	{
        halWarpOSD_Config(eGCI_OSD, eWOC_BlinkPaletteSet0_1, PLTData, eWRTIE_TO_BUFFER);
	}
	else if(WritePaletteIdx == 0x100)  //for blink color
	{
        halWarpOSD_Config(eGCI_OSD, eWOC_BlinkPaletteSet1_0, PLTData, eWRTIE_TO_BUFFER);
	}
	else if(WritePaletteIdx == 0x101)  //for blink color
	{
        halWarpOSD_Config(eGCI_OSD, eWOC_BlinkPaletteSet1_1, PLTData, eWRTIE_TO_BUFFER);
	}
	else
	{
	    LOG_MSG(db_HAL_WARPING, "(func:%s,line:%d)Error PLT index %d, eColor %d\n", __FUNCTION__, __LINE__, WritePaletteIdx, eColor);
	}
}


eCOLOR_IDX GridColorPaletteIndexGet(void)
{
    switch(m_stWarpConfig.stOsd.eGridColor)
    {
        case GRID_COLOR__GREEN:
            return COLOR_IDX__GREEN;

        case GRID_COLOR__MAGENTA:
            return COLOR_IDX__MAGENTA;

        case GRID_COLOR__RED:
            return COLOR_IDX__RED;

        case GRID_COLOR__CYAN:
            return COLOR_IDX__CYAN;

        default:
            LOG_MSG(db_HAL_WARPING, "%s get fail (%d)\r\n", __FUNCTION__, m_stWarpConfig.stOsd.eGridColor);
    }

	return COLOR_IDX__GREEN;
}

void InitOsdPatternColor(void)
{
	//Set Palette Color
	WriteColorReg(0, COLOR_IDX__GREEN);
	WriteColorReg(1, COLOR_IDX__MAGENTA);
	WriteColorReg(2, COLOR_IDX__RED);
	WriteColorReg(3, COLOR_IDX__CYAN);
	WriteColorReg(4, COLOR_IDX__BLUE);
	WriteColorReg(5, COLOR_IDX__YELLOW);
	WriteColorReg(6, COLOR_IDX__BLACK);
	WriteColorReg(7, COLOR_IDX__WHITE);
	WriteColorReg(8, COLOR_IDX__DARK_GREEN);
	WriteColorReg(9, COLOR_IDX__DARK_MAGENTA);
	WriteColorReg(10, COLOR_IDX__DARK_RED);
	WriteColorReg(11, COLOR_IDX__DARK_CYAN);
	WriteColorReg(12, COLOR_IDX__BLACKLEVEL_GREEN);    //A65_OPTOMA_Doulas_0020
	WriteColorReg(13, COLOR_IDX__BLACKLEVEL_MAGENTA);  //A65_OPTOMA_Doulas_0020
	WriteColorReg(14, COLOR_IDX__BLACKLEVEL_RED);      //A65_OPTOMA_Doulas_0020
	WriteColorReg(15, COLOR_IDX__BLACKLEVEL_CYAN);     //A65_OPTOMA_Doulas_0020
	WriteColorReg(16, COLOR_IDX__BLACK_BKG);
	WriteColorReg(17, COLOR_IDX__GREY192);  //H2PF_Simon_0037
	WriteColorReg(m_stUserDefineOsdPalette.ucPaletteIndex, COLOR_IDX__USER_DEFINE);
	dvC789_Buffer_Flush();
}

void InitOsdBlinkColor(void)
{
	UINT8 Data = TRUE ;
    halWarpOSD_Config(eGCI_OSD, eWOC_BlinkEnable, &Data, eWRTIE_TO_BUFFER);
	Data = 0x20 ;
    halWarpOSD_Config(eGCI_OSD, eWOC_BlinkTime,   &Data, eWRTIE_TO_BUFFER);
	Data = 0x40 ;
    halWarpOSD_Config(eGCI_OSD, eWOC_BlinkCycle,  &Data, eWRTIE_TO_BUFFER);

	//Color1 of Blink1 Address(idx=240) for warp/blend
	WriteColorReg(COLOR_IDX__BLINK, GridColorPaletteIndexGet());   //A65_OPTOMA_Doulas_0020
	//Color1 of Blink2 Address(idx=241) for blacklevel
	WriteColorReg(COLOR_IDX__BLACK_BLINK, m_eBlackLevelGridColor);

	//Color2 of Blink1 Address(idx=256) for warp/blend
	WriteColorReg(256, m_stWarpConfig.stOsd.eCursorColor);
	//Color2 of Blink2 Address(idx=257) for black level  //A65_OPTOMA_Doulas_0020
	WriteColorReg(257, m_eBlackLevelCursorColor);

	dvC789_Buffer_Flush();
}

void InitOsdTransparentColor(void)
{
	//set BLACK as transparent color  //A65_OPTOMA_CL_0006
	if(m_ucColorPalette == COLOR_PALETTE__OSD)  //A65_OPTOMA_CL_0015
	{
        UINT8 Data = TRUE;
        halWarpOSD_Config(eGCI_OSD, eWOC_TransparentColorEnable, &Data, eWRTIE_TO_BUFFER);
        Data = COLOR_IDX__BLACK_BKG;
        halWarpOSD_Config(eGCI_OSD, eWOC_TransparentColor0, &Data, eWRTIE_TO_BUFFER);
        halWarpOSD_Config(eGCI_OSD, eWOC_TransparentColor1, &Data, eWRTIE_TO_BUFFER);
        halWarpOSD_Config(eGCI_OSD, eWOC_TransparentColor2, &Data, eWRTIE_TO_BUFFER);
        Data = m_eOverlapColor;
        halWarpOSD_Config(eGCI_OSD, eWOC_TransparentColor3, &Data, eWRTIE_TO_CHIP);

	}
	else
	{
        UINT8 Data = TRUE;
        halWarpOSD_Config(eGCI_OSD, eWOC_TransparentColorEnable, &Data, eWRTIE_TO_BUFFER);
        Data = 0;
        halWarpOSD_Config(eGCI_OSD, eWOC_TransparentColor0, &Data, eWRTIE_TO_BUFFER);
        halWarpOSD_Config(eGCI_OSD, eWOC_TransparentColor1, &Data, eWRTIE_TO_BUFFER);
        halWarpOSD_Config(eGCI_OSD, eWOC_TransparentColor2, &Data, eWRTIE_TO_BUFFER);
        Data = m_eOverlapColor;
        halWarpOSD_Config(eGCI_OSD, eWOC_TransparentColor3, &Data, eWRTIE_TO_CHIP);
	}

}
//A65_OPTOMA_CL_0002
void InitGridColor(void)
{
	switch(m_stWarpConfig.stOsd.eGridColor)
	{
		default:
		case GRID_COLOR__GREEN:
			m_eOverlapColor = COLOR_IDX__DARK_GREEN;
			m_eBlackLevelGridColor = COLOR_IDX__BLACKLEVEL_GREEN;     //A65_OPTOMA_Doulas_0020
			m_eBlackLevelCursorColor = COLOR_IDX__BLACKLEVEL_MAGENTA; //A65_OPTOMA_Doulas_0020
			m_stWarpConfig.stOsd.eCursorColor = COLOR_IDX__MAGENTA;
			break;
		case GRID_COLOR__MAGENTA:
			m_eOverlapColor = COLOR_IDX__DARK_MAGENTA;
			m_eBlackLevelGridColor = COLOR_IDX__BLACKLEVEL_MAGENTA;  //A65_OPTOMA_Doulas_0020
			m_eBlackLevelCursorColor = COLOR_IDX__BLACKLEVEL_GREEN;  //A65_OPTOMA_Doulas_0020
			m_stWarpConfig.stOsd.eCursorColor = COLOR_IDX__GREEN;
			break;
		case GRID_COLOR__RED:
			m_eOverlapColor = COLOR_IDX__DARK_RED;
			m_eBlackLevelGridColor = COLOR_IDX__BLACKLEVEL_RED;     //A65_OPTOMA_Doulas_0020
			m_eBlackLevelCursorColor = COLOR_IDX__BLACKLEVEL_CYAN;  //A65_OPTOMA_Doulas_0020
			m_stWarpConfig.stOsd.eCursorColor = COLOR_IDX__CYAN;
			break;
		case GRID_COLOR__CYAN:
			m_eOverlapColor = COLOR_IDX__DARK_CYAN;
			m_eBlackLevelGridColor = COLOR_IDX__BLACKLEVEL_CYAN;   //A65_OPTOMA_Doulas_0020
			m_eBlackLevelCursorColor = COLOR_IDX__BLACKLEVEL_RED;  //A65_OPTOMA_Doulas_0020
			m_stWarpConfig.stOsd.eCursorColor = COLOR_IDX__RED;
			break;
	}

	InitOsdBlinkColor();
	InitOsdTransparentColor();
}

void InitControlPoint(void)
{
	uint8 x,y;

	for( y = 0; y < DEF_NUM_CUR_MAX_V; y++ ) {           //G100_Doulas_0059
		for( x = 0; x < DEF_NUM_CUR_MAX_H; x++ ) {   //G100_Doulas_0059
			PM_GRID[x][y].x = PM_GRID_OLD[x][y].x = PS_CUR_DEF_OX[x];
			PM_GRID[x][y].y = PM_GRID_OLD[x][y].y = PS_CUR_DEF_OY[y];
		}
	}
	return;
}

void InitOsdChar(void)
{
	uint32 ii;

	//Set the write address
	dvC789_WriteToBuffer(B0_CPUWAD, OSD_FONT_ADDR);
	for (ii = 0; ii < m_nCharDataSize; ii++) {
		dvC789_WriteToBuffer(B0_CPUDT, m_pnCharData[ii]);
	}

	//Font size 64x64 mode enable
	dvC789_WriteToBuffer(B0_CBUFMD, 0x40);
	dvC789_Buffer_Flush();
}

void InitOutputTimingParam(void)
{
	PS_PANEL_HS_CYCL = m_ModeTable[m_eCurrentResId].nHTotal;       //G100_Doulas_0065
	PS_PANEL_VS_CYCL = m_ModeTable[m_eCurrentResId].nVTotal;
	PS_PANEL_HS_POL = m_ModeTable[m_eCurrentResId].eHPol;
	PS_PANEL_VS_POL = m_ModeTable[m_eCurrentResId].eVPol;
	PS_PANEL_HS_W = m_ModeTable[m_eCurrentResId].nHSyncWidth;
	PS_PANEL_VS_W = m_ModeTable[m_eCurrentResId].nVSyncWidth;
	PS_PANEL_ACT_HST = m_ModeTable[m_eCurrentResId].nHStart;
	PS_PANEL_ACT_HW = m_ModeTable[m_eCurrentResId].nHActive;
	PS_PANEL_ACT_VST = m_ModeTable[m_eCurrentResId].nVStart;
	PS_PANEL_ACT_VW = m_ModeTable[m_eCurrentResId].nVActive;
	PS_PANEL_FV = m_ModeTable[m_eCurrentResId].nFrameRate;

	PS_WP_HW_GRD = ((PS_PANEL_ACT_HW%16)==0) ? PS_PANEL_ACT_HW/16 : PS_PANEL_ACT_HW/16+1;
	PS_WP_VW_GRD = ((PS_PANEL_ACT_VW%16)==0) ? PS_PANEL_ACT_VW/16 : PS_PANEL_ACT_VW/16+1;

	m_nLineFeed = PS_PANEL_ACT_HW/128;

	m_fResRatioX = PS_PANEL_ACT_HW/(float)m_nNativeResH;  //G100_Doulas_0065
	m_fResRatioY = PS_PANEL_ACT_VW/(float)m_nNativeResV;

	switch(m_eCurrentResId)   //G100_Doulas_0065
	{
		case RES_ID__XGA:
		case RES_ID__XGA120:
			m_nPointSize = 11;
			break;
		case RES_ID__WXGA:
		case RES_ID__720P120:
			m_nPointSize = 15;
			break;
		default:
		case RES_ID__UWHD:
		case RES_ID__WUXGA:
		case RES_ID__1080P60:
		case RES_ID__1080P48:
		case RES_ID__1080P120:	//G100_Doulas_0027
		case RES_ID__WUXGA120:  //G100_Doulas_0063
		case RES_ID__WXGA120:	//R70G2_Doulas_0004
			m_nPointSize = 21;
			break;
	}
	m_nBlackLevelPointSize = 11;  //for blacklevel pattern only //A65_OPTOMA_Doulas_0020
}

void SetOutputTiming(eRES_ID eCurrentResId)    //A65_OPTOMA_Doulas_0020
{
	uint16 nReg16;

	m_eCurrentResId = eCurrentResId;
	if(m_eCurrentResId!=NATIVE_RES_ID)
		m_bOutputNativeRes = FALSE;
	else
		m_bOutputNativeRes = TRUE;

	//Init system parameter related to output resolution
	InitOutputTimingParam();

	//Line Feed
	dvC789_WriteToBuffer(B1_OSDMWI, m_nLineFeed);
	dvC789_WriteToBuffer(B0_BBWMWI, m_nLineFeed);
	dvC789_WriteToBuffer(B1_OSDACTHW, PS_PANEL_ACT_HW);    //A65_OPTOMA_Simon_0003
	dvC789_WriteToBuffer(B1_OSDACTVW, PS_PANEL_ACT_VW);    //A65_OPTOMA_Simon_0003
    dvC789_OSDACTStartConfig(0);  //A65_OPTOMA_Simon_0003
	dvC789_Buffer_Flush();
}

void OsdBlackLevel_ResetConfig(eBLACKLEVEL_AREA eAreaSelect)  //A65_OPTOMA_Doulas_0020
{
	float fShrinkRatio = 0.0;

	m_stWarpConfig.stOsd.stBlackLevel.ucArea = eAreaSelect; //A65_OPTOMA_Doulas_0020
	m_eAreaSelection = eAreaSelect; 						//A65_OPTOMA_Doulas_0020
	m_stWarpConfig.stOsd.stBlackLevel.bEnable[eAreaSelect] = FALSE;
	m_stWarpConfig.stOsd.stBlackLevel.nPointNum[eAreaSelect] = BLACKLEVEL_NODE_MIN;
	//Bottom area shrink 5% from 4 boundary
	//Top area shrink 20% from 4 boundary
	if(eAreaSelect == BLACKLEVEL_AREA__BOTTOM)
	{
		fShrinkRatio = 0.05;
	}
	else
	{
		fShrinkRatio = 0.2;
	}
	// 1st point
	m_stWarpConfig.stOsd.stBlackLevel.pnPointList[eAreaSelect][0].x = (uint16)((float)m_nNativeResH*fShrinkRatio);
	m_stWarpConfig.stOsd.stBlackLevel.pnPointList[eAreaSelect][0].y = (uint16)((float)m_nNativeResV*fShrinkRatio);
	// 2nd point
	m_stWarpConfig.stOsd.stBlackLevel.pnPointList[eAreaSelect][1].x = (uint16)((float)m_nNativeResH*fShrinkRatio);
	m_stWarpConfig.stOsd.stBlackLevel.pnPointList[eAreaSelect][1].y = m_nNativeResV-1 - (uint16)((float)m_nNativeResV*fShrinkRatio);
	// 3rd point
	m_stWarpConfig.stOsd.stBlackLevel.pnPointList[eAreaSelect][2].x = m_nNativeResH-1 - (uint16)((float)m_nNativeResH*fShrinkRatio);
	m_stWarpConfig.stOsd.stBlackLevel.pnPointList[eAreaSelect][2].y = m_nNativeResV-1 - (uint16)((float)m_nNativeResV*fShrinkRatio);
	// 4th point
	m_stWarpConfig.stOsd.stBlackLevel.pnPointList[eAreaSelect][3].x = m_nNativeResH-1 - (uint16)((float)m_nNativeResH*fShrinkRatio);
	m_stWarpConfig.stOsd.stBlackLevel.pnPointList[eAreaSelect][3].y = (uint16)((float)m_nNativeResV*fShrinkRatio);
	//color palette
	m_stWarpConfig.stOsd.stBlackLevel.pnColor[eAreaSelect+1][0] = BLACKLEVEL_VALUE_R;
	m_stWarpConfig.stOsd.stBlackLevel.pnColor[eAreaSelect+1][1] = BLACKLEVEL_VALUE_G;
	m_stWarpConfig.stOsd.stBlackLevel.pnColor[eAreaSelect+1][2] = BLACKLEVEL_VALUE_B;

	m_stWarpConfig.stOsd.stBlackLevel.ucBoundary = ets_ON;  //A35G2_Owen_0002 //A65_OPTOMA_Doulas_0239

}

void ResetWarpConfig(void)   //A65_OPTOMA_Doulas_0020
{
	//uint8 i = 0;

	//reset warp config
	memset(&m_stWarpConfig, 0, sizeof(WARP_CONFIG));
	m_stWarpConfig.eWarpCtrl = WARP_CTRL__BASIC;
	m_stWarpConfig.stOsd.eGridColor = GRID_COLOR__GREEN;
	m_stWarpConfig.stOsd.eCursorColor = COLOR_IDX__MAGENTA;
	m_stWarpConfig.stOsd.eBkgColor = BKG_COLOR__BLACK;
	m_stWarpConfig.stOsd.eWarpPoint= WARP_POINT__2x2;
	m_stWarpConfig.stOsd.bWarpInnerOn = FALSE;
	m_stWarpConfig.stOsd.nWarpSharpness = WARP_SHARPNESS_DEFAULT;
	m_stWarpConfig.stOsd.nOverlapGridNum = OVERLAP_GRID_NUM_DEFAULT;
	m_stWarpConfig.stOsd.nBlendGamma = BLEND_GAMMA_DEFAULT;
	#ifdef CUSTOM_CHRISTIE
	m_stWarpConfig.stOsd.bWarpLimit = TRUE;
	#else
	m_stWarpConfig.stOsd.bWarpLimit = FALSE;   //optoma no limit    //A35G2_Simon_0087
	#endif
#ifdef SUPPORT_WARP_CONTROL_PC
	m_stWarpConfig.stPc.nWarpLutH = WARP_SHARPNESS_DEFAULT;
	m_stWarpConfig.stPc.nWarpLutV = WARP_SHARPNESS_DEFAULT;
	m_stWarpConfig.stPc.eBlendMode = BLEND_MODE__EDGE;
	m_stWarpConfig.stPc.fBlendGamma = 2.2;
	m_stWarpConfig.stPc.bIsDotByDotAreaValid = FALSE;
	m_stWarpConfig.stPc.bBiasOn = FALSE;
	m_stWarpConfig.stPc.bIsBiasAreaValid = FALSE;
#endif
	//for(i = 0; i< BLACKLEVEL_AREA__NUM; i++)
	//{
	//	OsdBlackLevel_ResetConfig(i);
	//}
	OsdBlackLevel_ResetConfig(BLACKLEVEL_AREA__TOP);
	OsdBlackLevel_ResetConfig(BLACKLEVEL_AREA__BOTTOM);

	//init control point
	InitControlPoint();
}

void ResetBlendConfig(void) //A35G2_BRC_Casper_0046
{
	m_stWarpConfig.stOsd.bBlendEnable = FALSE;  //add for Barco Ptool to enable/disable blending
	m_stWarpConfig.stOsd.nBlendWidthL = 0;
	m_stWarpConfig.stOsd.nBlendWidthR = 0;
	m_stWarpConfig.stOsd.nBlendWidthT = 0;
	m_stWarpConfig.stOsd.nBlendWidthB = 0;
	m_stWarpConfig.stOsd.nBlendOffsetL = 0;  //add for Barco Ptool
	m_stWarpConfig.stOsd.nBlendOffsetR = 0;  //add for Barco Ptool
	m_stWarpConfig.stOsd.nBlendOffsetT = 0;  //add for Barco Ptool
	m_stWarpConfig.stOsd.nBlendOffsetB = 0;  //add for Barco Ptool
	m_stWarpConfig.stOsd.nOverlapGridNum = OVERLAP_GRID_NUM_DEFAULT;
	m_stWarpConfig.stOsd.nBlendGamma = BLEND_GAMMA_DEFAULT;
}
void ResetWarpSetting(void) //A35G2_BRC_Casper_0046
{
	m_stWarpConfig.stOsd.eGridColor = GRID_COLOR__GREEN;
	m_stWarpConfig.stOsd.eCursorColor = COLOR_IDX__MAGENTA;
	m_stWarpConfig.stOsd.eBkgColor = BKG_COLOR__BLACK;
	m_stWarpConfig.stOsd.eWarpPoint= WARP_POINT__2x2;
	m_stWarpConfig.stOsd.bWarpInnerOn = TRUE;
	m_stWarpConfig.stOsd.nWarpSharpness = WARP_SHARPNESS_DEFAULT;
	m_stWarpConfig.stOsd.bWarpLimit = TRUE; //A35G2_BRC_Casper_0047
	m_stWarpConfig.stOsd.bShowBlendOnWarpPattern = FALSE;   //G50_Keven_0003 //A35G2_Wesley_0093
	m_stWarpConfig.stOsd.bDrawOverlapGridOnWarpPattern = FALSE; // G50_Keven_0003 //A35G2_Wesley_0093
}

void InitRunTimeParam(void)  //A65_OPTOMA_Doulas_0020
{
	uint8 i = 0;

	//calc warping table
	CalcOsdWarpTable();
	//init grid color
	InitGridColor();
	//reset other setting
	m_nSelCtlPointX = 0;
	m_nSelCtlPointY = 0;
	m_bOsdWarpParamChanged = FALSE;

#if (ADVANCED_BLACK_LEVEL == TRUE)
	for(i = 0; i < BLACKLEVEL_AREA__NUM; i++)
	{
		InitBlackLevelNode(i);
	}

	m_eAreaSelection = m_stWarpConfig.stOsd.stBlackLevel.ucArea; 	//A65_OPTOMA_Doulas_0020
#endif
}

#if 0  //move to utilMathAPI.c
uint32 CalcChecksum(uint8 *pnData, uint32 nSize)
{
	uint32 i = 0;
	uint32 checksum = 0;

    for (i = 0; i < nSize; i++)
    {
        checksum += *(pnData+i);
    }
	checksum = ~checksum;

	return checksum;
}
#endif

uint8 SaveWarpConfig(void)
{
#ifdef WARP_SAVE_FILE_SYSTEM			//G100_Doulas_0027 Modify
	WARP_CONFIG_FLASH stWrite;
	WARP_CONFIG_FLASH stRead;
	UINT8  ucResult;
	uint16 nDataSize = sizeof(WARP_CONFIG_FLASH);

	LOG_MSG(db_HAL_WARPING, "123s (%f,%f)(%d)\r\n",m_stWarpConfig.stOsd.pfControlPoint[16][16].x
	                                              ,m_stWarpConfig.stOsd.pfControlPoint[16][16].y
	                                              ,m_stWarpConfig.stOsdBasic.stWArpGeoPara.m_cWarpAutoFilter);	//G100_Doulas_0047 Modify//G100_Doulas_0028
	//construct data
	memcpy(&(stWrite.pnHeader), m_pnHeader, sizeof(m_pnHeader));
	memcpy(&(stWrite.stWarpConfig), &m_stWarpConfig, sizeof(WARP_CONFIG));
	stWrite.nChecksum = CalcChecksum((uint8*)(&stWrite), sizeof(WARP_CONFIG_FLASH)-sizeof(stWrite.nChecksum));
	ucResult = utilWarp_ADVCurrentWarpSaveFile(&stWrite,nDataSize);
	if(ucResult != FLASH_ACCESS__PASS)
	{
		LOG_MSG(db_HAL_WARPING, "Geo File Access Error (%d)\r\n", __LINE__);
		return ucResult;
	}

	//write data to C789 DRAM
	//WriteC789Ram((uint8*)(&stWrite), RAM_DATA_ADDR, nDataSize);

	//erase Flash
	//if(!EraseFlashSector(WARP_CONFIG_FLASH_ADDR, FLASH_ERASE_MODE__SE4K))
    //   return FLASH_ACCESS__ERASE_FLASH_ERR;

	//write data to FLASH by DMA
	//if(!Dram_Reg2Flash(RAM_DATA_ADDR, WARP_CONFIG_FLASH_ADDR, nDataSize))
    //   return FLASH_ACCESS__RAM_TO_FLASH_ERR;

	//read data from FLASH by DMA
	//if(!Dram_Flash2Reg(RAM_DATA_ADDR, WARP_CONFIG_FLASH_ADDR, nDataSize))
    //    return FLASH_ACCESS__FLASH_TO_RAM_ERR;

	//read data from C789 DRAM
	//ReadC789Ram((uint8*)(&stRead), RAM_DATA_ADDR, nDataSize);
	ucResult = utilWarp_ADVCurrentWarpReloadFile(&stRead);
	if(ucResult != FLASH_ACCESS__PASS)
	{
		LOG_MSG(db_HAL_WARPING, "Geo File Access Error (%d)\r\n", __LINE__);
		return ucResult;
	}

	// compare
	if(memcmp((uint8*)(&stWrite), (uint8*)(&stRead), nDataSize))
	{
		LOG_MSG(db_HAL_WARPING, "Geo File Access Error (%d)\r\n", __LINE__);
        return FLASH_ACCESS__COMPARE_ERR;
    }

	return FLASH_ACCESS__PASS;
#else
    WARP_CONFIG_FLASH stWrite;
	WARP_CONFIG_FLASH stRead;
	uint16 nDataSize = sizeof(WARP_CONFIG_FLASH);
	uint16 nLoop = 0, nId = 0;  //G100_Doulas_0059

	//construct data
	memcpy(&(stWrite.pnHeader), m_pnHeader, sizeof(m_pnHeader));
	memcpy(&(stWrite.stWarpConfig), &m_stWarpConfig, sizeof(WARP_CONFIG));
	stWrite.nChecksum = CalcChecksum((uint8*)(&stWrite), sizeof(WARP_CONFIG_FLASH)-sizeof(stWrite.nChecksum));

	//write data to C789 DRAM
	WriteC789Ram((uint8*)(&stWrite), RAM_DATA_ADDR, nDataSize);

	nLoop = (nDataSize%4096 == 0) ? nDataSize/4096 : nDataSize/4096+1;   //G100_Doulas_0059
	//Erase Flash
	for(nId=0; nId<nLoop; nId++)
	{
		if(!EraseFlashSector(WARP_CONFIG_FLASH_ADDR+(0x1000*nId), FLASH_ERASE_MODE__SE4K))
			return FLASH_ACCESS__ERASE_FLASH_ERR;
	}

	//write data to FLASH by DMA
	if(!Dram_Reg2Flash(RAM_DATA_ADDR, WARP_CONFIG_FLASH_ADDR, nDataSize))
       return FLASH_ACCESS__RAM_TO_FLASH_ERR;

	//read data from FLASH by DMA
	if(!Dram_Flash2Reg(RAM_DATA_ADDR, WARP_CONFIG_FLASH_ADDR, nDataSize))
        return FLASH_ACCESS__FLASH_TO_RAM_ERR;

	//read data from C789 DRAM
	ReadC789Ram((uint8*)(&stRead), RAM_DATA_ADDR, nDataSize);

	// compare
	if(memcmp((uint8*)(&stWrite), (uint8*)(&stRead), nDataSize))
        return FLASH_ACCESS__COMPARE_ERR;

	return FLASH_ACCESS__PASS;
#endif
}

uint8 LoadWarpConfig(void)
{
#ifdef WARP_SAVE_FILE_SYSTEM	//G100_Doulas_0027 Modify
	WARP_CONFIG_FLASH stRead;
	//WARP_CONFIG_FLASH stRead2;
	UINT8  ucResult;
	uint16 nDataSize = sizeof(WARP_CONFIG_FLASH);

	//read data from FLASH by DMA
	//if(!Dram_Flash2Reg(RAM_DATA_ADDR, WARP_CONFIG_FLASH_ADDR, nDataSize))
    //    return FLASH_ACCESS__FLASH_TO_RAM_ERR;

	//read data from C821 DRAM
	//ReadC789Ram((uint8*)(&stRead), RAM_DATA_ADDR, nDataSize);

	ucResult = utilWarp_ADVCurrentWarpReloadFile(&stRead);
	if(ucResult != FLASH_ACCESS__PASS)
	{
		LOG_MSG(db_HAL_WARPING, "Geo File Access Error (%d)\r\n", __LINE__);
		return ucResult;
	}

	//verify header
    if (0 != memcmp(stRead.pnHeader, m_pnHeader, sizeof(m_pnHeader)))
    {
		LOG_MSG(db_HAL_WARPING, "Geo File Access Error (%d)\r\n", __LINE__);
		return FLASH_ACCESS__VERIFY_HEADER_ERR;
	}

	//verify checksum
	if(stRead.nChecksum != CalcChecksum((uint8*)(&stRead), sizeof(WARP_CONFIG_FLASH)-sizeof(stRead.nChecksum)))
	{
		LOG_MSG(db_HAL_WARPING, "Geo File Access Error (%d)\r\n", __LINE__);
	    return FLASH_ACCESS__VERIFY_CHECKSUM_ERR;
	}

	//restore warp config
	memcpy(&m_stWarpConfig, &(stRead.stWarpConfig), sizeof(WARP_CONFIG));

	/*utilWarp_ADVCurrentWarpReloadFile(&stRead2);
	if(memcmp(&stRead,&stRead,nDataSize) == 0)
	{
		LOG_MSG(db_ALWAYS, "memory comp ok\r\n");
	}
	else
	{
		LOG_MSG(db_ALWAYS, "memory comp fail\r\n");
	}*/
	return FLASH_ACCESS__PASS;
#else
	WARP_CONFIG_FLASH stRead;
	uint16 nDataSize = sizeof(WARP_CONFIG_FLASH);

	//read data from FLASH by DMA
	if(!Dram_Flash2Reg(RAM_DATA_ADDR, WARP_CONFIG_FLASH_ADDR, nDataSize))
        return FLASH_ACCESS__FLASH_TO_RAM_ERR;

	//read data from C789 DRAM
	ReadC789Ram((uint8*)(&stRead), RAM_DATA_ADDR, nDataSize);

	//verify header
    if (0 != memcmp(stRead.pnHeader, m_pnHeader, sizeof(m_pnHeader)))
		return FLASH_ACCESS__VERIFY_HEADER_ERR;

	//verify checksum
	if(stRead.nChecksum != CalcChecksum((uint8*)(&stRead), sizeof(WARP_CONFIG_FLASH)-sizeof(stRead.nChecksum)))
		return FLASH_ACCESS__VERIFY_CHECKSUM_ERR;

	//restore warp config
	memcpy(&m_stWarpConfig, &(stRead.stWarpConfig), sizeof(WARP_CONFIG));
	return FLASH_ACCESS__PASS;
#endif
}

BOOL VerifyWarpConfig(void)
{
	uint8 i = 0;  //A65_OPTOMA_Doulas_0020

	if(	m_stWarpConfig.eWarpCtrl>=WARP_CTRL__INVALID)
		return FALSE;

	if(	m_stWarpConfig.stOsd.eGridColor!=GRID_COLOR__GREEN &&
		m_stWarpConfig.stOsd.eGridColor!=GRID_COLOR__MAGENTA&&
		m_stWarpConfig.stOsd.eGridColor!=GRID_COLOR__CYAN&&
		m_stWarpConfig.stOsd.eGridColor!=GRID_COLOR__RED)
		return FALSE;

	if(	m_stWarpConfig.stOsd.eCursorColor!=COLOR_IDX__GREEN &&
		m_stWarpConfig.stOsd.eCursorColor!=COLOR_IDX__MAGENTA&&
		m_stWarpConfig.stOsd.eCursorColor!=COLOR_IDX__CYAN&&
		m_stWarpConfig.stOsd.eCursorColor!=COLOR_IDX__RED)
		return FALSE;

	if(	m_stWarpConfig.stOsd.eBkgColor>=BKG_COLOR__INVALID)
		return FALSE;

	if(	m_stWarpConfig.stOsd.eWarpPoint>=WARP_POINT__INVALID)
		return FALSE;

	if(	m_stWarpConfig.stOsd.nWarpSharpness>WARP_SHARPNESS_MAX)
		return FALSE;

#if 0  //without offset & has a minimum value. 0 <=>min value ~ max value //A35G2_BRC_Casper_0046
	if((m_stWarpConfig.stOsd.nBlendWidthL>0 && m_stWarpConfig.stOsd.nBlendWidthL<BLEND_WIDTH_H_MIN) ||
		m_stWarpConfig.stOsd.nBlendWidthL>BLEND_WIDTH_H_MAX)
		return FALSE;

	if((m_stWarpConfig.stOsd.nBlendWidthR>0 && m_stWarpConfig.stOsd.nBlendWidthR<BLEND_WIDTH_H_MIN) ||
		m_stWarpConfig.stOsd.nBlendWidthR>BLEND_WIDTH_H_MAX)
		return FALSE;

	if((m_stWarpConfig.stOsd.nBlendWidthT>0 && m_stWarpConfig.stOsd.nBlendWidthT<BLEND_WIDTH_V_MIN) ||
		m_stWarpConfig.stOsd.nBlendWidthT>BLEND_WIDTH_V_MAX)
		return FALSE;

	if((m_stWarpConfig.stOsd.nBlendWidthB>0 && m_stWarpConfig.stOsd.nBlendWidthB<BLEND_WIDTH_V_MIN) ||
		m_stWarpConfig.stOsd.nBlendWidthB>BLEND_WIDTH_V_MAX)
		return FALSE;
#else //A35G2_BRC_Casper_0046
        if(m_stWarpConfig.stOsd.nBlendWidthL>BLEND_WIDTH_H_MAX)
            return FALSE;

        if(m_stWarpConfig.stOsd.nBlendWidthR>BLEND_WIDTH_H_MAX)
            return FALSE;

        if(m_stWarpConfig.stOsd.nBlendWidthT>BLEND_WIDTH_V_MAX)
            return FALSE;

        if(m_stWarpConfig.stOsd.nBlendWidthB>BLEND_WIDTH_V_MAX)
            return FALSE;

        if(m_stWarpConfig.stOsd.nBlendOffsetL>BLEND_WIDTH_H_MAX)
            return FALSE;

        if(m_stWarpConfig.stOsd.nBlendOffsetR>BLEND_WIDTH_H_MAX)
            return FALSE;

        if(m_stWarpConfig.stOsd.nBlendOffsetT>BLEND_WIDTH_V_MAX)
            return FALSE;

        if(m_stWarpConfig.stOsd.nBlendOffsetB>BLEND_WIDTH_V_MAX)
            return FALSE;

        if((m_stWarpConfig.stOsd.nBlendWidthL + m_stWarpConfig.stOsd.nBlendOffsetL)>BLEND_WIDTH_H_MAX)
            return FALSE;

        if((m_stWarpConfig.stOsd.nBlendWidthR + m_stWarpConfig.stOsd.nBlendOffsetR)>BLEND_WIDTH_H_MAX)
            return FALSE;

        if((m_stWarpConfig.stOsd.nBlendWidthT + m_stWarpConfig.stOsd.nBlendOffsetT)>BLEND_WIDTH_V_MAX)
            return FALSE;

        if((m_stWarpConfig.stOsd.nBlendWidthB + m_stWarpConfig.stOsd.nBlendOffsetB)>BLEND_WIDTH_V_MAX)
            return FALSE;
#endif

	if( m_stWarpConfig.stOsd.nOverlapGridNum>OVERLAP_GRID_NUM_MAX)
		return FALSE;

	if( m_stWarpConfig.stOsd.nBlendGamma>BLEND_GAMMA_MAX)
		return FALSE;

#if (ADVANCED_BLACK_LEVEL == TRUE)
	for(i=0; i<BLACKLEVEL_AREA__NUM;i++)    //A65_OPTOMA_Doulas_0020
	{
		if((m_stWarpConfig.stOsd.stBlackLevel.nPointNum[i] < 4 )
			|| (m_stWarpConfig.stOsd.stBlackLevel.nPointNum[i] > 32 ))
			return FALSE;
	}

	if(m_stWarpConfig.stOsd.stBlackLevel.ucArea >= BLACKLEVEL_AREA__NUM)   //A65_OPTOMA_Doulas_0020
	{
		return FALSE;
	}
#endif

#if 0 //A35G2_BRC_Casper_0046
#ifdef SUPPORT_WARP_CONTROL_PC
	if(	m_stWarpConfig.stPc.nWarpLutH>WARP_SHARPNESS_MAX)
		return FALSE;

	if(	m_stWarpConfig.stPc.nWarpLutV>WARP_SHARPNESS_MAX)
		return FALSE;

	if(	m_stWarpConfig.stPc.eBlendMode>=BLEND_MODE__INVALID)
		return FALSE;

	if(	m_stWarpConfig.stPc.nBlendWidthL>PS_PANEL_ACT_HW)
		return FALSE;

	if(	m_stWarpConfig.stPc.nBlendWidthR>PS_PANEL_ACT_HW)
		return FALSE;

	if(	m_stWarpConfig.stPc.nBlendWidthT>PS_PANEL_ACT_VW)
		return FALSE;

	if(	m_stWarpConfig.stPc.nBlendWidthB>PS_PANEL_ACT_VW)
		return FALSE;
#endif
#endif
	//LOG_MSG(db_ALWAYS, "123x (%f,%f)\r\n",PM_GRID[16][16].x,PM_GRID[16][16].y);		//G100_Doulas_0028
	if (((int)PM_GRID[DEF_NUM_CUR_MAX_H-1][DEF_NUM_CUR_MAX_V-1].x) == 0 && ((int)PM_GRID[DEF_NUM_CUR_MAX_H-1][DEF_NUM_CUR_MAX_V-1].y) == 0)    //G100_Doulas_0059 Modify
		return FALSE;

	if (CalcOsdWarpTable() != E_WpNoErr)
		return FALSE;

	return TRUE;
}

void ApplyWarp(eAPPLY_WARP eFlag)
{
    LOG_MSG(db_HAL_WARPING, "ApplyWarp (%d %d)\r\n\r\n", eFlag, m_stWarpConfig.eWarpCtrl);

	dvC789_WriteToBuffer(BN_RTCT, RTCT_THRU); //A35G2_BRC_Casper_0046 //A35G2_BRC_Casper_0051
	switch(m_stWarpConfig.eWarpCtrl)
	{
		default:
		case WARP_CTRL__BASIC:
			//currently, behavior is reset warping
			utilWarp_WriteDefaultWarpTable();
			utilWarp_WriteDefaultWarpLut();
			break;

		case WARP_CTRL__ADVANCED:
			if(eFlag==APPLY_WARP__RE_CALC)
				CalcOsdWarpTable();
			WriteWarpTable(Def_Wp_Space);
			WriteWarpLut(m_stWarpConfig.stOsd.nWarpSharpness, m_stWarpConfig.stOsd.nWarpSharpness);
			break;

		case WARP_CTRL__AP:
			utilWarp_WriteDefaultWarpTable();   //A65_OPTOMA_Simon_0003
			utilWarp_WriteDefaultWarpLut();     //A65_OPTOMA_Simon_0003
#if 0//kenton_temp_check
#ifdef SUPPORT_WARP_CONTROL_PC	//kenton_temp_check

			C821_LoadWarpFromFlash(m_stWarpConfig.stPc.nFlashIdx);
			WriteWarpLut(m_stWarpConfig.stPc.nWarpLutH, m_stWarpConfig.stPc.nWarpLutV);
#endif
#endif
			break;
	}
}

void ControlPointIdxToPos(uint8 nPointIdxX, uint8 nPointIdxY, uint16 *nPointPosX, uint16 *nPointPosY)
{
	if(nPointIdxX == 0)
		*nPointPosX = 0;
	else if(nPointIdxX == (DEF_NUM_CUR_MAX_H-1))   //G100_Doulas_0059
		*nPointPosX = PS_PANEL_ACT_HW-m_nPointSize;
	else
		*nPointPosX = (PS_CUR_DEF_OX[nPointIdxX]*m_fResRatioX-m_nPointSize/2);

	if(nPointIdxY == 0)
		*nPointPosY = 0;
	else if(nPointIdxY == (DEF_NUM_CUR_MAX_V-1))    //G100_Doulas_0059
		*nPointPosY = PS_PANEL_ACT_VW-m_nPointSize;
	else
		*nPointPosY = (PS_CUR_DEF_OY[nPointIdxY]*m_fResRatioY-m_nPointSize/2);
}

void ConvertBlendWidth(BLEND_WIDTH *pblend_des)
{
	if (m_bOutputNativeRes == FALSE) {
		pblend_des->nL = (uint16)m_stWarpConfig.stOsd.nBlendWidthL*m_fResRatioX;
		pblend_des->nR = (uint16)m_stWarpConfig.stOsd.nBlendWidthR*m_fResRatioX;
		pblend_des->nT = (uint16)m_stWarpConfig.stOsd.nBlendWidthT*m_fResRatioY;
		pblend_des->nB = (uint16)m_stWarpConfig.stOsd.nBlendWidthB*m_fResRatioY;
	} else {
		pblend_des->nL = m_stWarpConfig.stOsd.nBlendWidthL;
		pblend_des->nR = m_stWarpConfig.stOsd.nBlendWidthR;
		pblend_des->nT = m_stWarpConfig.stOsd.nBlendWidthT;
		pblend_des->nB = m_stWarpConfig.stOsd.nBlendWidthB;
	}
}

void ConvertBlendOffset(BLEND_OFFSET *pblend_des) //A35G2_BRC_Casper_0046
{
	if (m_bOutputNativeRes == FALSE) {
		pblend_des->nL = (uint16)m_stWarpConfig.stOsd.nBlendOffsetL*m_fResRatioX;
		pblend_des->nR = (uint16)m_stWarpConfig.stOsd.nBlendOffsetR*m_fResRatioX;
		pblend_des->nT = (uint16)m_stWarpConfig.stOsd.nBlendOffsetT*m_fResRatioY;
		pblend_des->nB = (uint16)m_stWarpConfig.stOsd.nBlendOffsetB*m_fResRatioY;
	} else {
		pblend_des->nL = m_stWarpConfig.stOsd.nBlendOffsetL;
		pblend_des->nR = m_stWarpConfig.stOsd.nBlendOffsetR;
		pblend_des->nT = m_stWarpConfig.stOsd.nBlendOffsetT;
		pblend_des->nB = m_stWarpConfig.stOsd.nBlendOffsetB;
	}
}



eCOLOR_IDX DrawOsdRect_BackgroundColorIndexGet(void)
{
    #ifdef SCALER_FPGA_F34
    {
        // transparent color always == 255;
        if(m_stWarpConfig.stOsd.eBkgColor == BKG_COLOR__BLACK)
        {
            return COLOR_IDX__BLACK_BKG;
        }
        else if(m_stWarpConfig.stOsd.eBkgColor == BKG_COLOR__TRANSPARENT)
        {
            return COLOR_IDX__TRANSPARENT;
        }
        else
        {
            LOG_MSG(db_HAL_WARPING, "!! eBkgColor %d Error", m_stWarpConfig.stOsd.eBkgColor);
        }
    }
    #else
    {
        return COLOR_IDX__BLACK_BKG;  //設定 iChips (C789:B1_OSDMODE) 的 transparent color register 來開關此顏色是否為透明色
    }
    #endif

	return COLOR_IDX__BLACK_BKG;
}

void DrawOsdRect(eOSD_LAYER_TYPE eOsdLayerType, uint16 nHStart, uint16 nVStart, uint16 nWidth, uint16 nHeight, eCOLOR_IDX eColor)
{
	uint32 nOsdAddr = 0;
	uint32 nOsdAddrStart = 0;
	uint32 nStatus = 0;
	uint16 nRetry = 500;
	if(nWidth==0 || nHeight==0)
		return;

	//draw size
	dvC789_WriteToBuffer(B0_BBACTHW, nWidth-1);	//Fill width is nWidth-1
	dvC789_WriteToBuffer(B0_BBACTVW, nHeight-1); 	//Fill height is nHeight-1

	//osd addr
	if(eOsdLayerType == OSD_LAYER_TYPE__TEMP)
		nOsdAddrStart = OSD_TEMP_ADDR;
	else if(eOsdLayerType == OSD_LAYER_TYPE__MAIN)
		nOsdAddrStart = m_OsdAddrLayerCurrent;
	else
		nOsdAddrStart = (m_OsdAddrLayerCurrent==m_OsdAddrLayer0) ? m_OsdAddrLayer1 : m_OsdAddrLayer0;

	nOsdAddr = nOsdAddrStart+nVStart*PS_PANEL_ACT_HW+nHStart;
	dvC789_WriteToBuffer(B0_CPUWAD, nOsdAddr);

	//file color
	dvC789_WriteToBuffer(B0_OSDFILL, eColor);

	//exec OSDFILL
	dvC789_WriteToBuffer(B0_OSDCT, 0x01);
	dvC789_Buffer_Flush();

	while(TRUE)
	{
		nStatus = dvC789_Read(B0_BOSTAT);
		if((nStatus&BIT0) && (nRetry>=0))
		{
			MS_SLEEP(1);
			nRetry--;
			if(nRetry==0)
			{
				LOG_MSG(db_HAL_WARPING, "DrawOsdRect() timeout, B0_BOSTAT=%02X\r\n", (unsigned int)nStatus);
				break;
			}
			else
				continue;
		}
		else
			break;
	}

	dvC789_Write(B0_OSDCT, 0x00);
}

void DrawOsdRectBorder(uint16 nHStart, uint16 nVStart, uint16 nWidth, uint16 nHeight, eCOLOR_IDX eColor, uint8 nLineWidth)  //H2PF_Simon_0037
{
	//Left
	DrawOsdRect(OSD_LAYER_TYPE__MAIN, nHStart, nVStart, nLineWidth, nHeight, eColor);
	//Right Edge
	DrawOsdRect(OSD_LAYER_TYPE__MAIN, nHStart+nWidth-nLineWidth, nVStart, nLineWidth, nHeight, eColor);
	//Top Edge
	DrawOsdRect(OSD_LAYER_TYPE__MAIN, nHStart, nVStart, nWidth, nLineWidth, eColor);
	//Bottom Edge
	DrawOsdRect(OSD_LAYER_TYPE__MAIN, nHStart, nVStart+nHeight-nLineWidth, nWidth, nLineWidth, eColor);
}

//nLineIdx : line index of C789 char buff
uint16 PreparedOsdString(uint8* pStr, uint8 nCharNum, uint8 nFixedCharLen, eCOLOR_IDX eCharColor,
								eCOLOR_IDX eBkgColor, uint8 nLineIdx, BOOL bDoubleSize)
{
	uint8 nIdx;
	uint8 nDoubleSizeFlag = (bDoubleSize) ? 0x80 : 0x00;
	uint16 nOsdStrLen = 0;
	uint8 nCharLen = 0;

	//set "Line attribute buffer"
	dvC789_WriteToBuffer(B0_CBUFAD, 0x0A00+nLineIdx);
	//Character height and double size flag
	dvC789_WriteToBuffer(B0_CBUFDT, nDoubleSizeFlag+0x3F);

	//character buffer is 32 characters x 16 lines
	//character attribute is 5 bytes
	dvC789_WriteToBuffer(B0_CBUFAD, nLineIdx*32*5);
	for (nIdx = 0; nIdx < nCharNum; nIdx++) {

		//replace the unsupported characters with "?"
		if(pStr[nIdx]<0x20 || pStr[nIdx]>0x7E)
			pStr[nIdx] = 0x3F;

		//Character attribute (5 Bytes)
		//(1st byte) Character code lower byte
		dvC789_WriteToBuffer(B0_CBUFDT, pStr[nIdx]-0x20);
		//(2nd byte) Character code upper byte
		dvC789_WriteToBuffer(B0_CBUFDT, 0x00);
		//(3rd byte) Character color
		dvC789_WriteToBuffer(B0_CBUFDT, eCharColor);
		//(4th byte) Background color
		dvC789_WriteToBuffer(B0_CBUFDT, eBkgColor);
		//(5th byte) Character width and double size flag
		nCharLen = (nFixedCharLen == OSD_NATIVE_CHAR_LEN) ? m_pnCharWidth[pStr[nIdx]-0x20] : nFixedCharLen;
		dvC789_WriteToBuffer(B0_CBUFDT, nDoubleSizeFlag+nCharLen);

		//string length on OSD
		nOsdStrLen+=nCharLen;
	}
	dvC789_Buffer_Flush();

	//there is 1 pixel spacing between each char
	return (bDoubleSize) ? (nOsdStrLen+nCharNum)*2 : nOsdStrLen+nCharNum;
}

void DrawOsdString(uint8 nCharNum, uint16 nHStart, uint16 nVStart, uint8 nLineIdx)
{
	uint32 wad, nRegVal;
	uint16 nRetry = 500;

	if(nCharNum<1)
		return;

	//char buffer size
	dvC789_WriteToBuffer(B0_CBUFHST, 0x00);
	dvC789_WriteToBuffer(B0_CBUFHW, nCharNum-1);
	dvC789_WriteToBuffer(B0_CBUFVST, nLineIdx);
	dvC789_WriteToBuffer(B0_CBUFVW, 0x00);

	//address of char buffer
	dvC789_WriteToBuffer(B0_CPURAD, OSD_FONT_ADDR);

	//address of OSD
	wad = m_OsdAddrLayerCurrent + nVStart * PS_PANEL_ACT_HW + nHStart;
	dvC789_WriteToBuffer(B0_CPUWAD, wad);

	//exec Character development
	dvC789_WriteToBuffer(B0_OSDCT, 0x03);
	dvC789_Buffer_Flush();

	while(TRUE)
	{
		nRegVal = dvC789_Read(B0_BOSTAT);
		if((nRegVal&BIT0) && (nRetry>=0))
		{
			MS_SLEEP(1);
			nRetry--;
			if(nRetry==0)
			{
				LOG_MSG(db_HAL_WARPING,"DrawOsdString() timeout, B0_BOSTAT=%02X\r\n", (unsigned int)nRegVal);
				break;
			}else
				continue;
		}else
			break;
	}

	dvC789_Write(B0_OSDCT, 0x00);
}

//A65_OPTOMA_CL_0007
void CopyOsdRect(eOSD_LAYER_TYPE eOsdLayerType, uint16 nSrcHStart, uint16 nSrcVStart,
								uint16 nDstHStart, uint16 nDstVStart, uint16 nWidth, uint16 nHeight)
{
	uint32 nOsdAddr = 0;
	uint32 nStatus = 0;
	uint16 nRetry = 500;

	if(nWidth==0 || nHeight==0)
		return;

	dvC789_WriteToBuffer(B0_BBWMWI, m_nLineFeed);
	dvC789_WriteToBuffer(B0_BBRMWI, m_nLineFeed);
	dvC789_WriteToBuffer(B0_BBACTHW, nWidth-1);	//Fill width is nWidth-1
	dvC789_WriteToBuffer(B0_BBACTVW, nHeight-1); //Fill height is nHeight-1
	nOsdAddr = m_OsdAddrLayerCurrent+nDstVStart*PS_PANEL_ACT_HW+nDstHStart;
	dvC789_WriteToBuffer(B0_CPUWAD, nOsdAddr);
	if(eOsdLayerType == OSD_LAYER_TYPE__TEMP)
		nOsdAddr = OSD_TEMP_ADDR+nSrcVStart*PS_PANEL_ACT_HW+nSrcHStart;
	else
		nOsdAddr = m_OsdAddrLayerCurrent+nSrcVStart*PS_PANEL_ACT_HW+nSrcHStart;
	dvC789_WriteToBuffer(B0_CPURAD, nOsdAddr);
	dvC789_WriteToBuffer(B0_OSDCT, 0x02);		// BitBLT exe
	dvC789_Buffer_Flush();

	while(TRUE)
	{
		nStatus = dvC789_Read(B0_BOSTAT);
		if((nStatus&BIT0) && (nRetry>=0))
		{
			MS_SLEEP(1);
			nRetry--;
			if(nRetry==0)
			{
				LOG_MSG(db_HAL_WARPING, "CopyOsdRect() timeout, B0_BOSTAT=%02X\r\n", (unsigned int)nStatus);
				break;
			}else
				continue;
		}else
			break;

	}

	dvC789_Write(B0_OSDCT, 0x00);
}

//A65_OPTOMA_CL_0007
//m_pnDataValue, m_pnDataCount : 儲存Pixel Circle分析後的結構，分別儲存值(value)與對應的個數(count)
static uint16 AnalyzePixelCircle(uint16 nDataSize)
{
	uint16 nDataValueSize = 0;
	uint16 nRepeatCount = 0;
	uint16 nCurData = m_pnData[0];
	uint16 nIdx1=0;

	for (nIdx1 = 0; nIdx1 < nDataSize; nIdx1++) {
		if (nCurData == m_pnData[nIdx1])
			nRepeatCount++;
		else {
			m_pnDataValue[nDataValueSize] = nCurData;
			m_pnDataCount[nDataValueSize] = nRepeatCount;
			nDataValueSize++;
			nCurData = m_pnData[nIdx1];
			nRepeatCount = 1;
		}

		if (nIdx1 >= (nDataSize-1)) {
			m_pnDataValue[nDataValueSize] = nCurData;
			m_pnDataCount[nDataValueSize] = nRepeatCount;
			nDataValueSize++;
		}
	}

	return nDataValueSize;
}

//A65_OPTOMA_CL_0007
//計算Pixel Circle的結構
static uint16 CalPixelCircle(uint16 nRadius)
{
	//Printf(MASK_C786, "dvC786CalPixelCircle(), nRadius = %d \r\n", nRadius);
	uint16 nDataValueSize = 0;
	uint16 x = nRadius;
	uint16 y = 0;
	INT16 radiusError = 1-x;
	uint16 nDataSize =0;
	uint16 nIdx1=0;
	uint16 nLoopCount = 0;
	uint16 nDataCountAmount = 0;
	//根據nRadius計算對應Y位置的X的大小，算出1/8圓
	while(x >= y)
	{
		y++;
		if (radiusError<0)
		{
			radiusError += 2 * y - 1;
		}
		else
		{
			x--;
			radiusError += 2 * (y - x + 1);
		}

		m_pnData[nDataSize] = x;
		nDataSize++;
	}

	//根據1/8圓去估算相鄰1/8圓
	//統計1/8圓有相同X值的Y的數量
	nDataValueSize = AnalyzePixelCircle(nDataSize);

	//將兩個1/8圓組成1/4圓
	for(nIdx1=nRadius-1; nIdx1>=nDataSize; nIdx1--)
	{
		if((nDataValueSize-1)>=nLoopCount)
			nDataCountAmount+=m_pnDataCount[nLoopCount];
		else
			nDataCountAmount++;

		m_pnData[nIdx1] = nDataCountAmount;
		nLoopCount++;
	}

	//倒序排列m_pnData，讓數值由小到大排列
	for(nIdx1=0; nIdx1<nRadius; nIdx1++)
		m_pnDataValue[nIdx1] = m_pnData[nRadius-1-nIdx1];
	for(nIdx1=0; nIdx1<nRadius; nIdx1++)
		m_pnData[nIdx1] = m_pnDataValue[nIdx1];

	//統計1/4圓有相同X值的Y的數量
	nDataValueSize = AnalyzePixelCircle(nRadius);

	return nDataValueSize;
}

//A65_OPTOMA_CL_0007
/**************************************************
nRectSizeX	: Grid 寬
nRectSizeY 	: Grid 高
nSpaceX		: 一個Grid加上一個黑Space的大小
nSpaceY		: 一個Grid加上一個黑Space的大小
nOffsetX 		: 起始點,需要位移的量
nOffsetY		: 起始點,需要位移的量
nMinXRect	: X方向要被畫的起始的Rect
nMaxXRect	: X方向要被畫的結束的Rect
nMinYRect	: Y方向要被畫的起始的Rect
nMaxYRect	: Y方向要被畫的結束的Rect
nStepX		: 一次要進幾個Grid + 黑色 的 Step
nStepY		: 一次要進幾個Grid + 黑色 的 Step
***************************************************/
static void DrawApCirclePattern()
{
	uint8 nForeRColor = m_psCircleInfo->color_red;
	uint8 nForeGColor = m_psCircleInfo->color_green;
	uint8 nForeBColor = m_psCircleInfo->color_blue;
	uint16 nRectSizeX = m_psCircleInfo->radius*2;
	uint16 nRectSizeY = m_psCircleInfo->radius*2;
	uint16 nSpaceX = m_psCircleInfo->space;
	uint16 nSpaceY = m_psCircleInfo->space;
	uint16 nOffsetX = m_psCircleInfo->x_offset;
	uint16 nOffsetY = m_psCircleInfo->y_offset;
	uint16 nMinXRect = m_psCircleInfo->x_start;
	uint16 nMaxXRect = m_psCircleInfo->x_end;
	uint16 nMinYRect = m_psCircleInfo->y_start;
	uint16 nMaxYRect = m_psCircleInfo->y_end;
	uint16 nStepX = m_psCircleInfo->x_step;
	uint16 nStepY = m_psCircleInfo->y_step;
	BOOL bShowLastSpot = m_psCircleInfo->bShowLast;
	BOOL bClearOsd = m_psCircleInfo->bClearOSD;

    LOG_MSG(db_ALWAYS, "AF : r, g, b = %d, %d, %d\r\n", nForeRColor, nForeGColor, nForeBColor);
    LOG_MSG(db_ALWAYS, "AF : Xstart, Xend, Xoffset, Xstep = %d, %d, %d, %d\r\n", nMinXRect, nMaxXRect, nOffsetX, nStepX);
    LOG_MSG(db_ALWAYS, "AF : Ystart, Yend, Yoffset, Ystep = %d, %d, %d, %d\r\n", nMinYRect, nMaxYRect, nOffsetY, nStepY);
    LOG_MSG(db_ALWAYS, "AF : Radius, Space, Showlast, Clear = %d, %d, %d, %d\r\n", nRectSizeX, nSpaceX, bShowLastSpot, bClearOsd);

	uint16 nNumRecX = (PS_PANEL_ACT_HW - nOffsetX)/nSpaceX;
	uint16 nNumRecY = (PS_PANEL_ACT_VW - nOffsetY)/nSpaceY;

	// The largest recs to draw is 1 minus these
	uint16 nActualMaxXRec = 0;
	uint16 nActualMaxYRec = 0;
	uint16 nIndex1 = 0, nIndex2 = 0;
	uint16 nXStart = 0, nYStart = 0;
	uint16 nXCur = 0, nYCur = 0;

#if 0
        if(bClearOsd)
        {
            LOG_MSG(db_ALWAYS, "%s, Clear OSD!!\r\n", __FUNCTION__);
            return;
        }
#endif

	//Set customer color to palette
	UINT8 UserColor[3] = {nForeRColor, nForeGColor, nForeBColor};
	halWarpOSD_OSD_PLT_Set(UserColor, COLOR_IDX__USER_DEFINE, 1, eWRTIE_TO_CHIP);

	//clear full screen to black
	DrawOsdRect(OSD_LAYER_TYPE__MAIN, 0, 0, PS_PANEL_ACT_HW, PS_PANEL_ACT_VW, COLOR_IDX__BLACK_BKG);  //black back ground can be transparented

	//如果剩下的空間足以再畫一個rect,就將nNumRect再加一個
	if ((PS_PANEL_ACT_HW - nOffsetX - (nNumRecX * nSpaceX)) >= nRectSizeX)
		nNumRecX++;
	if ((PS_PANEL_ACT_VW - nOffsetY - (nNumRecY * nSpaceY)) >= nRectSizeY)
		nNumRecY++;

	nActualMaxXRec = (nNumRecX < nMaxXRect ) ? nNumRecX : nMaxXRect;
	nActualMaxYRec = (nNumRecY < nMaxYRect ) ? nNumRecY : nMaxYRect;
	nXStart = nOffsetX + nMinXRect * nSpaceX;
	nYStart = nOffsetY + nMinYRect * nSpaceY;

	//先畫第一個Grid
	if(nRectSizeX>100)
	{
	        //diameter >100, draw rectangle
		DrawOsdRect(OSD_LAYER_TYPE__MAIN, nXStart, nYStart, nRectSizeX, nRectSizeY, COLOR_IDX__USER_DEFINE);
	}
	else
	{
		uint16 nCircleDataLen = CalPixelCircle(nRectSizeX/2);
		nOffsetX = 0;	//****共用變數，請確認後面沒用到****
		nOffsetY = 0;	//****共用變數，請確認後面沒用到****
		for(nIndex1 = 0; nIndex1<nCircleDataLen; nIndex1++)
		{
			nOffsetX = (nRectSizeX/2-m_pnDataValue[nIndex1]);
			if(nIndex1 >= 1)
				nOffsetY+=m_pnDataCount[nIndex1-1];
			//上半圓的部分
			  DrawOsdRect(OSD_LAYER_TYPE__MAIN, nXStart+nOffsetX, nYStart+nOffsetY,
							m_pnDataValue[nIndex1]*2, m_pnDataCount[nIndex1], COLOR_IDX__USER_DEFINE);
			//下半圓的部分
			  DrawOsdRect(OSD_LAYER_TYPE__MAIN, nXStart+nOffsetX, nYStart+(nRectSizeX-nOffsetY-m_pnDataCount[nIndex1]),
							m_pnDataValue[nIndex1]*2, m_pnDataCount[nIndex1], COLOR_IDX__USER_DEFINE);
		}
	}

	//nStepX與nStepY為0會造成下面產生無窮迴圈
	if(nStepX==0) nStepX=1;
	if(nStepY==0) nStepY=1;
	for (nIndex2 = nMinYRect; nIndex2 <= nActualMaxYRec; nIndex2+=nStepY)
	{
		nYCur = nYStart+(nIndex2-nMinYRect)*nSpaceY;
		if(nIndex2 == nMinYRect)
		{
			//第一列的其他Grid複製已畫好的第一個Grid
			for (nIndex1 = nMinXRect; nIndex1 <= nActualMaxXRec; nIndex1+=nStepX)
			{
				nXCur = nXStart+(nIndex1-nMinXRect)*nSpaceX;
				if(nIndex1==nMinXRect && nIndex2==nMinYRect)
				{
					continue;
				}

				if(!bShowLastSpot && (nIndex1==(nActualMaxXRec)) && (nIndex2==(nActualMaxYRec)))
				{
					continue;
				}

				CopyOsdRect(OSD_LAYER_TYPE__MAIN, nXStart, nYStart, nXCur, nYCur, nRectSizeX, nRectSizeY);
			}
		}
		else
		{
			//其他列的Grid複製已畫好的第一列
			CopyOsdRect(OSD_LAYER_TYPE__MAIN, nXStart, nYStart, nXStart, nYCur, PS_PANEL_ACT_HW-nXStart, nRectSizeY);

			//將最後一列的最後一個圓使用全黑遮住
			if(!bShowLastSpot && (nIndex2==(nActualMaxYRec)))
			{
				nXCur = nXStart + (nActualMaxXRec-nMinXRect)*nSpaceX;
				DrawOsdRect(OSD_LAYER_TYPE__MAIN, nXCur, nYCur, nRectSizeX, nRectSizeY, COLOR_IDX__BLACK_BKG);
			}
		}
	}
}

void DrawCheckerBoardPattern()  //A65_OPTOMA_CL_0015
{
    INT16 iHorzRes = PS_PANEL_ACT_HW;
    INT16 iVertRes = PS_PANEL_ACT_VW;

    eWARPOSD_DRAW_TYPE ucLayerMode = m_psChecksInfo->ucLayerMode;
    INT16 iRectSize = m_psChecksInfo->iBlockSize;

    LOG_MSG(db_HAL_WARPING, "%s %d %d\n" ,__FUNCTION__, ucLayerMode ,iRectSize);

    //draw full screen black color
    DrawOsdRect(OSD_LAYER_TYPE__MAIN, 0, 0, iHorzRes, iVertRes, COLOR_IDX__BLACK );

    //draw first white rect
    DrawOsdRect(OSD_LAYER_TYPE__MAIN, 0, 0, iRectSize, iRectSize, COLOR_IDX__WHITE);

    //draw all white rect by memory copy
    for(int y = 0 ; y < iVertRes ; y+=iRectSize)
    {
        for(int x = 0 ; x < iHorzRes ; x+=(iRectSize*2) )
        {
            if( ((y/iRectSize) % 2) == 0 )
            {
                CopyOsdRect(OSD_LAYER_TYPE__MAIN, 0, 0, x, y, iRectSize, iRectSize );
            }
            else
            {
                if( x >= iHorzRes )
                    break;

                CopyOsdRect(OSD_LAYER_TYPE__MAIN, 0, 0, x+iRectSize , y, iRectSize, iRectSize );
            }
        }
    }
}

void DrawOsdLinePattern(uint16 nP1X, uint16 nP1Y, uint16 nP2X, uint16 nP2Y, eCOLOR_IDX eColor)
{
	uint16 TLx, TLy, BLx, BLy;
	uint16 rndLx, rndRx, remainder;
	uint16 rndLy, rndRy;
	uint16 x, y, dx, dy, tempr;
	int times;
	int sign;

	dy = abs(nP1Y - nP2Y);
	dx = abs(nP1X - nP2X);

	if(dy<=dx)    //A65_OPTOMA_Doulas_0020
	{
		//this is base on Y
		if(nP2Y>=nP1Y)
		{
			TLx = nP1X;
			TLy = nP1Y;
			BLx = nP2X;
			BLy = nP2Y;
		}
		else
		{
			BLx = nP1X;
			BLy = nP1Y;
			TLx = nP2X;
			TLy = nP2Y;
		}
		dy = BLy - TLy;
		dx = (BLx>=TLx) ? BLx-TLx : TLx-BLx;
		sign = (TLx > BLx) ? -1 : 1;

		y = TLy;
		if(TLy == BLy)
		{
			rndLx = (TLx > BLx) ? BLx : TLx;
			rndRx = (TLx > BLx) ? TLx : BLx;
		}
		else{
			rndLx = TLx;
			rndRx = TLx;
		}
		remainder = 0;
		DrawOsdRect(OSD_LAYER_TYPE__MAIN, rndLx, y, rndRx-rndLx+1, 1, eColor);

		for(y=TLy+1;y<=BLy;y++)
		{
			tempr = remainder + dx;
			times = tempr / dy;
			remainder = (tempr >= dy) ? tempr - times * dy : tempr;
			if(sign == -1)
			{
				rndRx = (tempr >= dy) ? rndLx + sign : rndLx;
				rndLx = (tempr >= dy) ? rndLx + sign * times : rndLx;
			}
			else
			{
				rndLx = (tempr >= dy) ? rndRx + sign : rndRx;
				rndRx = (tempr >= dy) ? rndRx + sign * times : rndRx;
			}
			DrawOsdRect(OSD_LAYER_TYPE__MAIN, rndLx, y, rndRx-rndLx+1, 1, eColor);
		}
	}
	else
	{
		//this is base on X
		if(nP2X>=nP1X)
		{
			TLx = nP1X;
			TLy = nP1Y;
			BLx = nP2X;
			BLy = nP2Y;
		}
		else
		{
			BLx = nP1X;
			BLy = nP1Y;
			TLx = nP2X;
			TLy = nP2Y;
		}
		dx = BLx - TLx;
		dy = (BLy>=TLy) ? BLy-TLy : TLy-BLy;
		sign = (TLy> BLy) ? -1 : 1;

		x = TLx;
		if(TLx == BLx)
		{
			rndLy = (TLy> BLy) ? BLy : TLy;
			rndRy = (TLy > BLy) ? TLy : BLy;
		}
		else
		{
			rndLy = TLy;
			rndRy = TLy;
		}
		remainder = 0;
		DrawOsdRect(OSD_LAYER_TYPE__MAIN, x, rndLy, 1, rndRy-rndLy+1,eColor);

		for(x=TLx+1;x<=BLx;x++)
		{
			tempr = remainder + dy;
			times = tempr / dx;
			remainder = (tempr >= dx) ? tempr - times * dx : tempr;
			if(sign == -1)
			{
				rndRy = (tempr >= dx) ? rndLy + sign : rndLy;
				rndLy = (tempr >= dx) ? rndLy + sign * times : rndLy;
			}
			else
			{
				rndLy = (tempr >= dx) ? rndRy + sign : rndRy;
				rndRy = (tempr >= dx) ? rndRy + sign * times : rndRy;
			}
			DrawOsdRect(OSD_LAYER_TYPE__MAIN, x, rndLy, 1, rndRy-rndLy+1, eColor);
		}
	}

}

// G50_Keven_0003 //A35G2_Wesley_0093 <<<
void DrawOsdWarpGridLinesPatternPtoolset(void)
{
	BLEND_WIDTH stBlendWidth;
	uint8 nPointIdxX, nPointIdxY;
	uint16 nPointPosX, nPointPosY;

	//base on current res. to convert blend width
    ConvertBlendWidth(&stBlendWidth);

	//Fill full black
	DrawOsdRect(OSD_LAYER_TYPE__MAIN, 0, 0, PS_PANEL_ACT_HW, PS_PANEL_ACT_VW, COLOR_IDX__BLACK_BKG);

	//For Barco
	//draw line by grid number only
	//H direction
	nPointIdxY = 0;
	for(nPointIdxX = 0; nPointIdxX < DEF_NUM_CUR_MAX_H; nPointIdxX++)
	{
		if(GV_GRID_EN[nPointIdxX][nPointIdxY]==0)
		{
			continue;
		}
		ControlPointIdxToPos(nPointIdxX, nPointIdxY, &nPointPosX, &nPointPosY);
		if(nPointIdxX == DEF_NUM_CUR_MAX_H - 1)
		{
			nPointPosX = nPointPosX + m_nPointSize -1;
		}
		else if(nPointIdxX != 0)
		{
			nPointPosX = nPointPosX + m_nPointSize/2;
		}
		DrawOsdRect(OSD_LAYER_TYPE__MAIN, nPointPosX, 0, GRID_LINE_WIDTH, PS_PANEL_ACT_VW, GridColorPaletteIndexGet());
	}
	//V direction
	nPointIdxX = 0;
	for(nPointIdxY = 0; nPointIdxY < DEF_NUM_CUR_MAX_V; nPointIdxY++)
	{
		if(GV_GRID_EN[nPointIdxX][nPointIdxY]==0)
		{
			continue;
		}
		ControlPointIdxToPos(nPointIdxX, nPointIdxY, &nPointPosX, &nPointPosY);
		if(nPointIdxY == DEF_NUM_CUR_MAX_V - 1)
		{
			nPointPosY = nPointPosY + m_nPointSize -1;
		}
		else if(nPointIdxY != 0)
		{
			nPointPosY = nPointPosY + m_nPointSize/2;
		}
		DrawOsdRect(OSD_LAYER_TYPE__MAIN, 0, nPointPosY, PS_PANEL_ACT_HW, GRID_LINE_WIDTH, GridColorPaletteIndexGet());
	}

	//draw boundary line of blending area:
	if(stBlendWidth.nL>0)	//Left
		DrawOsdRect(OSD_LAYER_TYPE__MAIN, stBlendWidth.nL, 0, GRID_LINE_WIDTH, PS_PANEL_ACT_VW, m_stWarpConfig.stOsd.eCursorColor);
	if(stBlendWidth.nR>0)	//Right
		DrawOsdRect(OSD_LAYER_TYPE__MAIN, PS_PANEL_ACT_HW-stBlendWidth.nR, 0, GRID_LINE_WIDTH, PS_PANEL_ACT_VW, m_stWarpConfig.stOsd.eCursorColor);
	if(stBlendWidth.nT>0)		//Top
		DrawOsdRect(OSD_LAYER_TYPE__MAIN, 0, stBlendWidth.nT, PS_PANEL_ACT_HW, GRID_LINE_WIDTH, m_stWarpConfig.stOsd.eCursorColor);
	if(stBlendWidth.nB>0)	//Bottom
		DrawOsdRect(OSD_LAYER_TYPE__MAIN, 0, PS_PANEL_ACT_VW-stBlendWidth.nB, PS_PANEL_ACT_HW, GRID_LINE_WIDTH, m_stWarpConfig.stOsd.eCursorColor);
}


void DrawOsdWarpGridLinesPattern(void)
{
	BLEND_WIDTH stBlendWidth;
	uint16 nOffset, nCount;
	uint16 nLGap=0, nRGap=0, nTGap=0, nBGap=0;
	uint8 nLRemain=0, nRRemain=0, nTRemain=0, nBRemain=0;
	uint8 OverlapGridNumIndex = m_stWarpConfig.stOsd.nOverlapGridNum;
	uint8 OverlapGridNum = 0;
	uint8 nNonZero = 0;

	//base on current res. to convert blend width
    ConvertBlendWidth(&stBlendWidth);

	//Fill full black
	DrawOsdRect(OSD_LAYER_TYPE__MAIN, 0, 0, PS_PANEL_ACT_HW, PS_PANEL_ACT_VW, COLOR_IDX__BLACK_BKG);

	if (OverlapGridNumIndex > OVERLAP_GRID_NUM_MAX)
	    OverlapGridNumIndex = OVERLAP_GRID_NUM_MAX;

	OverlapGridNum = m_pnOverlapGridNumTable[OverlapGridNumIndex];

	//Fill the background of blending area:
	if(stBlendWidth.nL>0)	//Left
		DrawOsdRect(OSD_LAYER_TYPE__MAIN, 0, 0, stBlendWidth.nL, PS_PANEL_ACT_VW, m_eOverlapColor);
	if(stBlendWidth.nR>0)	//Right
		DrawOsdRect(OSD_LAYER_TYPE__MAIN, PS_PANEL_ACT_HW-stBlendWidth.nR, 0, stBlendWidth.nR, PS_PANEL_ACT_VW, m_eOverlapColor);
	if(stBlendWidth.nT>0)	//Top
		DrawOsdRect(OSD_LAYER_TYPE__MAIN, 0, 0, PS_PANEL_ACT_HW, stBlendWidth.nT, m_eOverlapColor);
	if(stBlendWidth.nB>0)	//Bottom
		DrawOsdRect(OSD_LAYER_TYPE__MAIN, 0, PS_PANEL_ACT_VW-stBlendWidth.nB, PS_PANEL_ACT_HW, stBlendWidth.nB, m_eOverlapColor);

	if ((stBlendWidth.nL != 0) && (stBlendWidth.nR != 0)) {
		nLGap = stBlendWidth.nL/OverlapGridNum;
		nLRemain = stBlendWidth.nL - nLGap * OverlapGridNum;
		nRGap = stBlendWidth.nR/OverlapGridNum;
		nRRemain = stBlendWidth.nR - nRGap * OverlapGridNum;
	} else if (stBlendWidth.nL != 0) {
		nLGap = nRGap = stBlendWidth.nL/OverlapGridNum;
		nLRemain = stBlendWidth.nL - nLGap * OverlapGridNum;
	} else if (stBlendWidth.nR != 0) {
		nLGap = nRGap = stBlendWidth.nR/OverlapGridNum;
		nRRemain = stBlendWidth.nR - nRGap * OverlapGridNum;
	} else {
		nLGap = nRGap = PS_PANEL_ACT_HW/(DEF_NUM_CUR_MAX_H-1); //default
	}

	if ((stBlendWidth.nT != 0) && (stBlendWidth.nB != 0)) {
		nTGap = stBlendWidth.nT/OverlapGridNum;
		nTRemain = stBlendWidth.nT - nTGap * OverlapGridNum;
		nBGap = stBlendWidth.nB/OverlapGridNum;
		nBRemain = stBlendWidth.nB - nBGap * OverlapGridNum;
	} else if (stBlendWidth.nT != 0) {
		nTGap = nBGap = stBlendWidth.nT/OverlapGridNum;
		nTRemain = stBlendWidth.nT - nTGap * OverlapGridNum;
	} else if (stBlendWidth.nB != 0) {
		nTGap = nBGap = stBlendWidth.nB/OverlapGridNum;
        nBRemain = stBlendWidth.nB - nBGap * OverlapGridNum;
	} else {
		nTGap = nBGap = PS_PANEL_ACT_VW/(DEF_NUM_CUR_MAX_V-1); //default
		if(((float)(PS_PANEL_ACT_VW%(DEF_NUM_CUR_MAX_V-1))/(float)(DEF_NUM_CUR_MAX_V-1)) <= 0.5)
		{
			nNonZero = 1;
		}
		else
		{
			nNonZero = 2;
		}
	}
	nLGap = (nLGap < OVERLAP_GAP_MINIMUM) ? OVERLAP_GAP_MINIMUM : nLGap;
	nRGap = (nRGap < OVERLAP_GAP_MINIMUM) ? OVERLAP_GAP_MINIMUM : nRGap;
	nTGap = (nTGap < OVERLAP_GAP_MINIMUM) ? OVERLAP_GAP_MINIMUM : nTGap;
	nBGap = (nBGap < OVERLAP_GAP_MINIMUM) ? OVERLAP_GAP_MINIMUM : nBGap;

    // draw the center vertical line( 2 pixels ).
	DrawOsdRect(OSD_LAYER_TYPE__MAIN, PS_PANEL_ACT_HW/2-1, 0, 2, PS_PANEL_ACT_VW, GridColorPaletteIndexGet());

    // draw the left vertical line.
	nOffset = 0;
	nCount = 0;
	do {
		DrawOsdRect(OSD_LAYER_TYPE__MAIN, nOffset, 0, GRID_LINE_WIDTH, PS_PANEL_ACT_VW, GridColorPaletteIndexGet());

		nCount++;
		if(nCount == 1)
			nOffset += (nLGap-1);	    // to make sure both of the test pattern and the content are aligned
		else
			nOffset += nLGap;

		if(nCount <= nLRemain)
		    nOffset++;
	}while(nOffset < PS_PANEL_ACT_HW/2);

	// draw the right vertical line
	nOffset = PS_PANEL_ACT_HW-1;
	nCount = 0;
	do
	{
		DrawOsdRect(OSD_LAYER_TYPE__MAIN, nOffset, 0, GRID_LINE_WIDTH, PS_PANEL_ACT_VW, GridColorPaletteIndexGet());

		nCount++;
		if(nCount == OverlapGridNum)
			nOffset -= (nRGap-1);	    // to make sure both of the test pattern and the content are aligned
		else
			nOffset -= nRGap;

		if((nCount <= OverlapGridNum) && (nCount > OverlapGridNum-nRRemain))
            nOffset--;
	}while(nOffset > PS_PANEL_ACT_HW/2);

    // draw the center horizontal line( 2 pixels )
	DrawOsdRect(OSD_LAYER_TYPE__MAIN, 0, PS_PANEL_ACT_VW/2-1, PS_PANEL_ACT_HW, 2, GridColorPaletteIndexGet());

    // draw the top horizontal line
	nOffset = 0;
	nCount = 0;
	do
	{
		DrawOsdRect(OSD_LAYER_TYPE__MAIN, 0, nOffset, PS_PANEL_ACT_HW, GRID_LINE_WIDTH, GridColorPaletteIndexGet());

		nCount++;
		if(nCount == 1)
			nOffset += (nTGap-1);	    // to make sure both of the test pattern and the content are aligned
		else
			nOffset += nTGap;

		if((nNonZero == 1) && (nCount%2))
			nOffset++;
		if((nNonZero == 2) && (nCount%4))
			nOffset++;

		if(nCount <= nTRemain)
		    nOffset++;
	}while(nOffset < PS_PANEL_ACT_VW/2);

    // draw the bottom horizontal line
	nOffset = PS_PANEL_ACT_VW-1;
	nCount = 0;
	do {
		DrawOsdRect(OSD_LAYER_TYPE__MAIN, 0, nOffset, PS_PANEL_ACT_HW, GRID_LINE_WIDTH, GridColorPaletteIndexGet());

		nCount++;
		if(nCount == OverlapGridNum)
			nOffset -= (nBGap-1);	    // to make sure both of the test pattern and the content are aligned
		else
			nOffset -= nBGap;

		if((nNonZero == 1) && (nCount%2))
			nOffset--;
		if((nNonZero == 2) && (nCount%4))
			nOffset--;

		if((nCount <= OverlapGridNum) && (nCount > OverlapGridNum-nBRemain))
            nOffset--;
	} while(nOffset > PS_PANEL_ACT_VW/2);
}


void DrawOsdWarpGridPointsPattern(void)
{
	uint8 nPointIdxX, nPointIdxY;
	uint16 nPointPosX, nPointPosY;

	//control points
	for(nPointIdxY = 0 ;nPointIdxY < DEF_NUM_CUR_MAX_V; nPointIdxY++)
	{
		for(nPointIdxX = 0 ;nPointIdxX < DEF_NUM_CUR_MAX_H; nPointIdxX++)
		{
			if(GV_GRID_EN[nPointIdxX][nPointIdxY]==0)
			{
				continue;
			}

			ControlPointIdxToPos(nPointIdxX, nPointIdxY, &nPointPosX, &nPointPosY);

			if(nPointIdxX==m_nSelCtlPointX && nPointIdxY==m_nSelCtlPointY)
			{
				eCOLOR_IDX eColor = m_stWarpConfig.stOsd.eCursorColor;
				if( m_ePatternType==PAT_TYPE__WARP_MOV_CTRL_POINT )
					eColor = COLOR_IDX__BLINK;

				DrawOsdRect(OSD_LAYER_TYPE__MAIN, nPointPosX, nPointPosY, m_nPointSize, m_nPointSize, eColor);
				//draw line indicator for outer control point of warp grid pattern

				#if 0   //wait review : simon temp remove
				if(!m_stWarpConfig.stOsd.bWarpInnerOn)
				{
					if(nPointIdxX==0 && nPointIdxY==0)	//Top_Left
						DrawOsdLinePattern(nPointPosX+m_nPointSize, nPointPosY+m_nPointSize, PS_PANEL_ACT_HW/2, PS_PANEL_ACT_VW/2, eColor);
					else if(nPointIdxX==(DEF_NUM_CUR_MAX_H-1) && nPointIdxY==0)	//Top_Right
						DrawOsdLinePattern(nPointPosX, nPointPosY+m_nPointSize, PS_PANEL_ACT_HW/2, PS_PANEL_ACT_VW/2, eColor);
					else if(nPointIdxX==(DEF_NUM_CUR_MAX_H-1) && nPointIdxY==(DEF_NUM_CUR_MAX_V-1))	//Bottom_Right
						DrawOsdLinePattern(nPointPosX, nPointPosY, PS_PANEL_ACT_HW/2, PS_PANEL_ACT_VW/2, eColor);
					else if(nPointIdxX==0 && nPointIdxY==(DEF_NUM_CUR_MAX_V-1))	//Bottom_Left
						DrawOsdLinePattern(nPointPosX+m_nPointSize, nPointPosY, PS_PANEL_ACT_HW/2, PS_PANEL_ACT_VW/2, eColor);
					else if(nPointIdxX==0 && nPointIdxY==((DEF_NUM_CUR_MAX_V-1)/2))	//Center of Left Edge
					{
						DrawOsdLinePattern(nPointPosX+m_nPointSize, nPointPosY+m_nPointSize/2-1, PS_PANEL_ACT_HW/2, PS_PANEL_ACT_VW/2-1, eColor);
						DrawOsdLinePattern(nPointPosX+m_nPointSize, nPointPosY+m_nPointSize/2, PS_PANEL_ACT_HW/2, PS_PANEL_ACT_VW/2, eColor);
					}
					else if(nPointIdxX==(DEF_NUM_CUR_MAX_H-1) && nPointIdxY==((DEF_NUM_CUR_MAX_V-1)/2))	//Center of Right Edge
					{
						DrawOsdLinePattern(nPointPosX, nPointPosY+m_nPointSize/2-1, PS_PANEL_ACT_HW/2, PS_PANEL_ACT_VW/2-1, eColor);
						DrawOsdLinePattern(nPointPosX, nPointPosY+m_nPointSize/2, PS_PANEL_ACT_HW/2, PS_PANEL_ACT_VW/2, eColor);
					}
					else if(nPointIdxX==((DEF_NUM_CUR_MAX_H-1)/2) && nPointIdxY==0)	//Center of Top Edge
					{
						DrawOsdLinePattern(nPointPosX+m_nPointSize/2-1, nPointPosY+m_nPointSize, PS_PANEL_ACT_HW/2-1, PS_PANEL_ACT_VW/2, eColor);
						DrawOsdLinePattern(nPointPosX+m_nPointSize/2, nPointPosY+m_nPointSize, PS_PANEL_ACT_HW/2, PS_PANEL_ACT_VW/2, eColor);
					}
					else if(nPointIdxX==((DEF_NUM_CUR_MAX_H-1)/2) && nPointIdxY==(DEF_NUM_CUR_MAX_V-1))	//Center of Bottom Edge
					{
						DrawOsdLinePattern(nPointPosX+m_nPointSize/2-1, nPointPosY, PS_PANEL_ACT_HW/2-1, PS_PANEL_ACT_VW/2, eColor);
						DrawOsdLinePattern(nPointPosX+m_nPointSize/2, nPointPosY, PS_PANEL_ACT_HW/2, PS_PANEL_ACT_VW/2, eColor);
					}
					else if(nPointIdxX==0)	//Left Edge
						DrawOsdLinePattern(nPointPosX+m_nPointSize, nPointPosY+m_nPointSize/2, PS_PANEL_ACT_HW/2, PS_PANEL_ACT_VW/2, eColor);
					else if(nPointIdxX==(DEF_NUM_CUR_MAX_H-1))	//Right Edge
						DrawOsdLinePattern(nPointPosX, nPointPosY+m_nPointSize/2, PS_PANEL_ACT_HW/2, PS_PANEL_ACT_VW/2, eColor);
					else if(nPointIdxY==0)	//Top Edge
						DrawOsdLinePattern(nPointPosX+m_nPointSize/2, nPointPosY+m_nPointSize, PS_PANEL_ACT_HW/2, PS_PANEL_ACT_VW/2, eColor);
					else if(nPointIdxY==(DEF_NUM_CUR_MAX_V-1))	//Bottom Edge
						DrawOsdLinePattern(nPointPosX+m_nPointSize/2, nPointPosY, PS_PANEL_ACT_HW/2, PS_PANEL_ACT_VW/2, eColor);
				}
				#endif
			}
			else
			{
				if(m_ePatternType!=PAT_TYPE__WARP_MOV_CTRL_POINT)
				{
					DrawOsdRect(OSD_LAYER_TYPE__MAIN, nPointPosX, nPointPosY, m_nPointSize, m_nPointSize, GridColorPaletteIndexGet());
				}
			}
		}
	}
}

// G50_Keven_0003 //A35G2_Wesley_0093 <<<

void DrawOsdWarpGridPattern(void)
{
    BLEND_WIDTH stBlendWidth;
	uint16 nOffset, nCount;
	uint16 nLGap=0, nRGap=0, nTGap=0, nBGap=0;
	uint8 nLRemain=0, nRRemain=0, nTRemain=0, nBRemain=0;
	uint8 nPointIdxX, nPointIdxY;
	uint16 nPointPosX, nPointPosY;
	uint8 OverlapGridNumIndex = m_stWarpConfig.stOsd.nOverlapGridNum;
    uint8 OverlapGridNum = 0;
	uint8 nNonZero = 0;    //G100_Doulas_0081

    //G100_Tim_0066, del, *&

	//base on current res. to convert blend width
    ConvertBlendWidth(&stBlendWidth);

	//Fill full black
	DrawOsdRect(OSD_LAYER_TYPE__MAIN, 0, 0, PS_PANEL_ACT_HW, PS_PANEL_ACT_VW, COLOR_IDX__BLACK_BKG);

    //G100_Tim_0066, mod, start
	//Draw grid line
	if(stBlendWidth.nT == 0 && stBlendWidth.nB == 0 && stBlendWidth.nL == 0 && stBlendWidth.nR == 0)
	{
		//For Barco
		//draw line by grid number only no blend widths were set
		//H direction
		nPointIdxY = 0;
		for(nPointIdxX = 0; nPointIdxX < DEF_NUM_CUR_MAX_H; nPointIdxX++)
		{
			if(GV_GRID_EN[nPointIdxX][nPointIdxY]==0)
			{
				continue;
			}
			ControlPointIdxToPos(nPointIdxX, nPointIdxY, &nPointPosX, &nPointPosY);
			if(nPointIdxX == DEF_NUM_CUR_MAX_H - 1)
			{
				nPointPosX = nPointPosX + m_nPointSize -1;
			}
			else if(nPointIdxX != 0)
			{
				nPointPosX = nPointPosX + m_nPointSize/2;
			}
			DrawOsdRect(OSD_LAYER_TYPE__MAIN, nPointPosX, 0, GRID_LINE_WIDTH, PS_PANEL_ACT_VW, GridColorPaletteIndexGet());
		}
		//V direction
		nPointIdxX = 0;
		for(nPointIdxY = 0; nPointIdxY < DEF_NUM_CUR_MAX_V; nPointIdxY++)
		{
			if(GV_GRID_EN[nPointIdxX][nPointIdxY]==0)
			{
				continue;
			}
			ControlPointIdxToPos(nPointIdxX, nPointIdxY, &nPointPosX, &nPointPosY);
			if(nPointIdxY == DEF_NUM_CUR_MAX_V - 1)
			{
				nPointPosY = nPointPosY + m_nPointSize -1;
			}
			else if(nPointIdxY != 0)
			{
				nPointPosY = nPointPosY + m_nPointSize/2;
			}
			DrawOsdRect(OSD_LAYER_TYPE__MAIN, 0, nPointPosY, PS_PANEL_ACT_HW, GRID_LINE_WIDTH, GridColorPaletteIndexGet());
		}
	}
	else  //original way if blend width is not 0. draw blend overlap
	{
		if (OverlapGridNumIndex > OVERLAP_GRID_NUM_MAX)
		    OverlapGridNumIndex = OVERLAP_GRID_NUM_MAX;

		OverlapGridNum = m_pnOverlapGridNumTable[OverlapGridNumIndex];

		//Fill the background of blending area:
		if(stBlendWidth.nL>0)	//Left
			DrawOsdRect(OSD_LAYER_TYPE__MAIN, 0, 0, stBlendWidth.nL, PS_PANEL_ACT_VW, m_eOverlapColor);
		if(stBlendWidth.nR>0)	//Right
			DrawOsdRect(OSD_LAYER_TYPE__MAIN, PS_PANEL_ACT_HW-stBlendWidth.nR, 0, stBlendWidth.nR, PS_PANEL_ACT_VW, m_eOverlapColor);
		if(stBlendWidth.nT>0)		//Top
			DrawOsdRect(OSD_LAYER_TYPE__MAIN, 0, 0, PS_PANEL_ACT_HW, stBlendWidth.nT, m_eOverlapColor);
		if(stBlendWidth.nB>0)	//Bottom
			DrawOsdRect(OSD_LAYER_TYPE__MAIN, 0, PS_PANEL_ACT_VW-stBlendWidth.nB, PS_PANEL_ACT_HW, stBlendWidth.nB, m_eOverlapColor);

		if ((stBlendWidth.nL != 0) && (stBlendWidth.nR != 0)) {
			nLGap = stBlendWidth.nL/OverlapGridNum;
			nLRemain = stBlendWidth.nL - nLGap * OverlapGridNum;
			nRGap = stBlendWidth.nR/OverlapGridNum;
			nRRemain = stBlendWidth.nR - nRGap * OverlapGridNum;
		} else if (stBlendWidth.nL != 0) {
			nLGap = nRGap = stBlendWidth.nL/OverlapGridNum;
			nLRemain = stBlendWidth.nL - nLGap * OverlapGridNum;
		} else if (stBlendWidth.nR != 0) {
			nLGap = nRGap = stBlendWidth.nR/OverlapGridNum;
			nRRemain = stBlendWidth.nR - nRGap * OverlapGridNum;
		} else {
			nLGap = nRGap = PS_PANEL_ACT_HW/(DEF_NUM_CUR_MAX_H-1); //default
		}

		if ((stBlendWidth.nT != 0) && (stBlendWidth.nB != 0)) {
			nTGap = stBlendWidth.nT/OverlapGridNum;
			nTRemain = stBlendWidth.nT - nTGap * OverlapGridNum;
			nBGap = stBlendWidth.nB/OverlapGridNum;
			nBRemain = stBlendWidth.nB - nBGap * OverlapGridNum;
		} else if (stBlendWidth.nT != 0) {
			nTGap = nBGap = stBlendWidth.nT/OverlapGridNum;
			nTRemain = stBlendWidth.nT - nTGap * OverlapGridNum;
		} else if (stBlendWidth.nB != 0) {
			nTGap = nBGap = stBlendWidth.nB/OverlapGridNum;
	        nBRemain = stBlendWidth.nB - nBGap * OverlapGridNum;
		} else {
			nTGap = nBGap = PS_PANEL_ACT_VW/(DEF_NUM_CUR_MAX_V-1); //default
			if(((float)(PS_PANEL_ACT_VW%(DEF_NUM_CUR_MAX_V-1))/(float)(DEF_NUM_CUR_MAX_V-1)) <= 0.5)
			{
				nNonZero = 1;
			}
			else
			{
				nNonZero = 2;
			}
			//LOG_MSG(db_ALWAYS, "nNonZero = %d\r\n", nNonZero);
		}
		//LOG_MSG(db_ALWAYS, "Before gap L = %d, R = %d, T = %d, B = %d\r\n",nLGap, nRGap, nTGap, nBGap);
		nLGap = (nLGap < OVERLAP_GAP_MINIMUM) ? OVERLAP_GAP_MINIMUM : nLGap;
		nRGap = (nRGap < OVERLAP_GAP_MINIMUM) ? OVERLAP_GAP_MINIMUM : nRGap;
		nTGap = (nTGap < OVERLAP_GAP_MINIMUM) ? OVERLAP_GAP_MINIMUM : nTGap;
		nBGap = (nBGap < OVERLAP_GAP_MINIMUM) ? OVERLAP_GAP_MINIMUM : nBGap;
		//LOG_MSG(db_ALWAYS, "After minimum gap L = %d, R = %d, T = %d, B = %d\r\n",nLGap, nRGap, nTGap, nBGap);

		// Draw the vertical line in the between the left and the right lines ( 2 pixels for each )
		DrawOsdRect(OSD_LAYER_TYPE__MAIN, PS_PANEL_ACT_HW/2-1, 0, 2, PS_PANEL_ACT_VW, GridColorPaletteIndexGet());

		// Draw the left vertical line
		nOffset = 0;
		nCount = 0;
		do {
			DrawOsdRect(OSD_LAYER_TYPE__MAIN, nOffset, 0, GRID_LINE_WIDTH, PS_PANEL_ACT_VW, GridColorPaletteIndexGet());

			nCount++;
			if(nCount == 1)
				nOffset += (nLGap-1);	// make sure when the test pattern is aligned, the content is aligned too.
			else
				nOffset += nLGap;

			if(nCount <= nLRemain)
			    nOffset++;
		}while(nOffset < PS_PANEL_ACT_HW/2);

		// Draw the right vertical line
		nOffset = PS_PANEL_ACT_HW-1;
		nCount = 0;
		do
		{
			DrawOsdRect(OSD_LAYER_TYPE__MAIN, nOffset, 0, GRID_LINE_WIDTH, PS_PANEL_ACT_VW, GridColorPaletteIndexGet());

			nCount++;
			if(nCount == OverlapGridNum)
				nOffset -= (nRGap-1);	// make sure when the test pattern is aligned, the content is aligned too.
			else
				nOffset -= nRGap;

			if((nCount <= OverlapGridNum) && (nCount > OverlapGridNum-nRRemain))
	            nOffset--;
		}while(nOffset > PS_PANEL_ACT_HW/2);

		// Draw the horizontal line between the upper and the bottom lines ( 2 pixels for each )
		DrawOsdRect(OSD_LAYER_TYPE__MAIN, 0, PS_PANEL_ACT_VW/2-1, PS_PANEL_ACT_HW, 2, GridColorPaletteIndexGet());

		// Draw the horizontal upper line
		nOffset = 0;
		nCount = 0;
		do
		{
			DrawOsdRect(OSD_LAYER_TYPE__MAIN, 0, nOffset, PS_PANEL_ACT_HW, GRID_LINE_WIDTH, GridColorPaletteIndexGet());
	//		LOG_MSG(db_ALWAYS, "nCount = %d, nOffset = %d, nTGap = %d\r\n", nCount, nOffset, nTGap);

			nCount++;
			if(nCount == 1)
				nOffset += (nTGap-1);	// make sure when the test pattern is aligned, the content is aligned too.
			else
				nOffset += nTGap;

			if((nNonZero == 1) && (nCount%2))
				nOffset++;
			if((nNonZero == 2) && (nCount%4))
				nOffset++;

	//		LOG_MSG(db_ALWAYS, "nCount = %d, nOffset = %d, nTRemain = %d\r\n", nCount, nOffset, nTRemain);
			if(nCount <= nTRemain)
			    nOffset++;
		}while(nOffset < PS_PANEL_ACT_VW/2);

		//Draw the horizontal bottom line
		nOffset = PS_PANEL_ACT_VW-1;
		nCount = 0;
		do {
			DrawOsdRect(OSD_LAYER_TYPE__MAIN, 0, nOffset, PS_PANEL_ACT_HW, GRID_LINE_WIDTH, GridColorPaletteIndexGet());
	//		LOG_MSG(db_ALWAYS, "nCount = %d, nOffset = %d, nBGap = %d\r\n", nCount, nOffset, nBGap);

			nCount++;
			if(nCount == OverlapGridNum)
				nOffset -= (nBGap-1);	// make sure when the test pattern is aligned, the content is aligned too.
			else
				nOffset -= nBGap;

			if((nNonZero == 1) && (nCount%2))
				nOffset--;
			if((nNonZero == 2) && (nCount%4))
				nOffset--;

	//		LOG_MSG(db_ALWAYS, "nCount = %d, nOffset = %d, nBRemain = %d\r\n", nCount, nOffset, nBRemain);
			if((nCount <= OverlapGridNum) && (nCount > OverlapGridNum-nBRemain))
	            nOffset--;
		} while(nOffset > PS_PANEL_ACT_VW/2);
	}
    //G100_Tim_0066, mod, end

	//Draw control point
	if (m_ePatternType!=PAT_TYPE__WARP_NO_CTRL_POINT)
	{
		//control points
		for(nPointIdxY = 0 ;nPointIdxY < DEF_NUM_CUR_MAX_V; nPointIdxY++)   //G100_Doulas_0059
		{
			for(nPointIdxX = 0 ;nPointIdxX < DEF_NUM_CUR_MAX_H; nPointIdxX++)   //G100_Doulas_0059
			{
				if(GV_GRID_EN[nPointIdxX][nPointIdxY]==0)
				{
					continue;
				}

				ControlPointIdxToPos(nPointIdxX, nPointIdxY, &nPointPosX, &nPointPosY);

				if(nPointIdxX==m_nSelCtlPointX && nPointIdxY==m_nSelCtlPointY)
				{
					eCOLOR_IDX eColor = m_stWarpConfig.stOsd.eCursorColor;
					if(m_ePatternType==PAT_TYPE__WARP_MOV_CTRL_POINT)
						eColor = COLOR_IDX__BLINK;

					DrawOsdRect(OSD_LAYER_TYPE__MAIN, nPointPosX, nPointPosY, m_nPointSize, m_nPointSize, eColor);
					//draw line indicator for outer control point of warp grid pattern
					if(!m_stWarpConfig.stOsd.bWarpInnerOn)
					{       //G100_Doulas_0059 Modify
						if(nPointIdxX==0 && nPointIdxY==0)	//Top_Left
							DrawOsdLinePattern(nPointPosX+m_nPointSize, nPointPosY+m_nPointSize, PS_PANEL_ACT_HW/2, PS_PANEL_ACT_VW/2, eColor);
						else if(nPointIdxX==(DEF_NUM_CUR_MAX_H-1) && nPointIdxY==0)	//Top_Right
							DrawOsdLinePattern(nPointPosX, nPointPosY+m_nPointSize, PS_PANEL_ACT_HW/2, PS_PANEL_ACT_VW/2, eColor);
						else if(nPointIdxX==(DEF_NUM_CUR_MAX_H-1) && nPointIdxY==(DEF_NUM_CUR_MAX_V-1))	//Bottom_Right
							DrawOsdLinePattern(nPointPosX, nPointPosY, PS_PANEL_ACT_HW/2, PS_PANEL_ACT_VW/2, eColor);
						else if(nPointIdxX==0 && nPointIdxY==(DEF_NUM_CUR_MAX_V-1))	//Bottom_Left
							DrawOsdLinePattern(nPointPosX+m_nPointSize, nPointPosY, PS_PANEL_ACT_HW/2, PS_PANEL_ACT_VW/2, eColor);
						else if(nPointIdxX==0 && nPointIdxY==((DEF_NUM_CUR_MAX_V-1)/2))	//Center of Left Edge
						{
							DrawOsdLinePattern(nPointPosX+m_nPointSize, nPointPosY+m_nPointSize/2-1, PS_PANEL_ACT_HW/2, PS_PANEL_ACT_VW/2-1, eColor);
							DrawOsdLinePattern(nPointPosX+m_nPointSize, nPointPosY+m_nPointSize/2, PS_PANEL_ACT_HW/2, PS_PANEL_ACT_VW/2, eColor);
						}
						else if(nPointIdxX==(DEF_NUM_CUR_MAX_H-1) && nPointIdxY==((DEF_NUM_CUR_MAX_V-1)/2))	//Center of Right Edge
						{
							DrawOsdLinePattern(nPointPosX, nPointPosY+m_nPointSize/2-1, PS_PANEL_ACT_HW/2, PS_PANEL_ACT_VW/2-1, eColor);
							DrawOsdLinePattern(nPointPosX, nPointPosY+m_nPointSize/2, PS_PANEL_ACT_HW/2, PS_PANEL_ACT_VW/2, eColor);
						}
						else if(nPointIdxX==((DEF_NUM_CUR_MAX_H-1)/2) && nPointIdxY==0)	//Center of Top Edge
						{
							DrawOsdLinePattern(nPointPosX+m_nPointSize/2-1, nPointPosY+m_nPointSize, PS_PANEL_ACT_HW/2-1, PS_PANEL_ACT_VW/2, eColor);
							DrawOsdLinePattern(nPointPosX+m_nPointSize/2, nPointPosY+m_nPointSize, PS_PANEL_ACT_HW/2, PS_PANEL_ACT_VW/2, eColor);
						}
						else if(nPointIdxX==((DEF_NUM_CUR_MAX_H-1)/2) && nPointIdxY==(DEF_NUM_CUR_MAX_V-1))	//Center of Bottom Edge
						{
							DrawOsdLinePattern(nPointPosX+m_nPointSize/2-1, nPointPosY, PS_PANEL_ACT_HW/2-1, PS_PANEL_ACT_VW/2, eColor);
							DrawOsdLinePattern(nPointPosX+m_nPointSize/2, nPointPosY, PS_PANEL_ACT_HW/2, PS_PANEL_ACT_VW/2, eColor);
						}
						else if(nPointIdxX==0)	//Left Edge
							DrawOsdLinePattern(nPointPosX+m_nPointSize, nPointPosY+m_nPointSize/2, PS_PANEL_ACT_HW/2, PS_PANEL_ACT_VW/2, eColor);
						else if(nPointIdxX==(DEF_NUM_CUR_MAX_H-1))	//Right Edge
							DrawOsdLinePattern(nPointPosX, nPointPosY+m_nPointSize/2, PS_PANEL_ACT_HW/2, PS_PANEL_ACT_VW/2, eColor);
						else if(nPointIdxY==0)	//Top Edge
							DrawOsdLinePattern(nPointPosX+m_nPointSize/2, nPointPosY+m_nPointSize, PS_PANEL_ACT_HW/2, PS_PANEL_ACT_VW/2, eColor);
						else if(nPointIdxY==(DEF_NUM_CUR_MAX_V-1))	//Bottom Edge
							DrawOsdLinePattern(nPointPosX+m_nPointSize/2, nPointPosY, PS_PANEL_ACT_HW/2, PS_PANEL_ACT_VW/2, eColor);
					}
				}
				else
				{
					if(m_ePatternType!=PAT_TYPE__WARP_MOV_CTRL_POINT)
					{
						DrawOsdRect(OSD_LAYER_TYPE__MAIN, nPointPosX, nPointPosY, m_nPointSize, m_nPointSize, GridColorPaletteIndexGet());
					}
				}
			}
		}
	}
}

#if (ENABLE_COLOR_UNIFORMITY == TRUE)
//A65_OPTOMA_CL_0001  //A35G2_CDS_CL_0001
void DrawOsdAcuSelectPosition(void)   //H2PF_Simon_0055
{
#if 0
	uint8 i, j;
	uint8 nGridOffset = OSD_ACU_GRID_SIZE/2;

	for(j = 0; j < 7; j++)
	{
		for(i = 0; i < 9; i++)
		{
			if(m_pnCUSelectEnableTable[i][j] != 0)
			{
			        DrawOsdRectBorder((uint16)(HPosRatio[i]*PS_PANEL_ACT_HW) - nGridOffset, (uint16)(VPosRatio[j]*PS_PANEL_ACT_VW) - nGridOffset, OSD_ACU_GRID_SIZE, OSD_ACU_GRID_SIZE, COLOR_IDX__BLACK, GRID_LINE_WIDTH_SEL);   //H2PF_Simon_0037
			}
			else
			{
			    if(!m_ucCUShow)  // 0:show all point, 1:show selected point only
			        DrawOsdRectBorder((uint16)(HPosRatio[i]*PS_PANEL_ACT_HW) - nGridOffset, (uint16)(VPosRatio[j]*PS_PANEL_ACT_VW) - nGridOffset, OSD_ACU_GRID_SIZE, OSD_ACU_GRID_SIZE, COLOR_IDX__GREY192, GRID_LINE_WIDTH);  //H2PF_Simon_0037
			}
		}
	}
#endif

	uint8 i, j;
	uint8 nGridOffset = OSD_ACU_GRID_SIZE/2;

    if(!m_ucCUShow)
    {
        //if(palImgMgr_PictureSettings_Get_NotReturnUser(0) == eCM_PICTURE_SETTINGS_HDR)  //H2PF_Simon_0069
        if(palDataMgr_AcuSelect_HDR_Gamma_Check() == TRUE) //HICC2_Julie_0052
        {
            sPaletteSingleInfo sPlt = {COLOR_IDX__GREY192, 128, 128, 128};
            halWarpOSD_Config(eGCI_OSD, eWOC_Palette_Single, &sPlt, eWRTIE_TO_CHIP);
        }
        else
        {
            sPaletteSingleInfo sPlt = {COLOR_IDX__GREY192, 192, 192, 192};
            halWarpOSD_Config(eGCI_OSD, eWOC_Palette_Single, &sPlt, eWRTIE_TO_CHIP);
        }

        //draw 1st row and copy to 2nd row
        DrawOsdRectBorder((uint16)(HPosRatio[0]*halWarping_HResGet()) - nGridOffset, (uint16)(VPosRatio[0]*halWarping_VResGet()) - nGridOffset, OSD_ACU_GRID_SIZE, OSD_ACU_GRID_SIZE, COLOR_IDX__GREY192, GRID_LINE_WIDTH);  //H2PF_Simon_0037
        for(i = 0; i < 9; i++)
        {
            if(i != 0)
            {
                //copy 1st rectangle to rest rectangle of row 1
                CopyOsdRect(eWDT_AFTER_WARP, (HPosRatio[0]*halWarping_HResGet())- nGridOffset, (VPosRatio[0]*halWarping_VResGet())- nGridOffset
                            , (HPosRatio[i]*halWarping_HResGet())- nGridOffset,(VPosRatio[0]*halWarping_VResGet()) - nGridOffset
                            , OSD_ACU_GRID_SIZE, OSD_ACU_GRID_SIZE);
            }
            //copy 1st row to 2nd row by each rectangle
            CopyOsdRect(eWDT_AFTER_WARP, (HPosRatio[i]*halWarping_HResGet())- nGridOffset, (VPosRatio[0]*halWarping_VResGet())- nGridOffset
                        , (HPosRatio[i]*halWarping_HResGet())- nGridOffset,(VPosRatio[1]*halWarping_VResGet()) - nGridOffset
                        , OSD_ACU_GRID_SIZE, OSD_ACU_GRID_SIZE);
        }
        //copy row 1 to row 7
        CopyOsdRect(eWDT_AFTER_WARP, (HPosRatio[0]*halWarping_HResGet())- nGridOffset, (VPosRatio[0]*halWarping_VResGet())- nGridOffset
                    , (HPosRatio[0]*halWarping_HResGet())- nGridOffset,(VPosRatio[6]*halWarping_VResGet()) - nGridOffset
                    , halWarping_HResGet()-OSD_ACU_GRID_SIZE, OSD_ACU_GRID_SIZE);

        //copy row 2 to row 3~6
        for(i = 2; i < 6; i++)
        {
           CopyOsdRect(eWDT_AFTER_WARP, (HPosRatio[0]*halWarping_HResGet())- nGridOffset, (VPosRatio[1]*halWarping_VResGet())- nGridOffset
                       , (HPosRatio[0]*halWarping_HResGet())- nGridOffset,(VPosRatio[i]*halWarping_VResGet()) - nGridOffset
                       , halWarping_HResGet()-OSD_ACU_GRID_SIZE, OSD_ACU_GRID_SIZE);
        }

    }


    //draw selected
	for(j = 0; j < 7; j++)
	{
		for(i = 0; i < 9; i++)
		{
			if(m_pnCUSelectEnableTable[i][j] != 0)
			{
		        //pink as selected
		         DrawOsdRectBorder((uint16)(HPosRatio[i]*halWarping_HResGet()) - nGridOffset, (uint16)(VPosRatio[j]*halWarping_VResGet()) - nGridOffset, OSD_ACU_GRID_SIZE, OSD_ACU_GRID_SIZE, COLOR_IDX__BLACK, GRID_LINE_WIDTH_SEL);  //H2PF_Simon_0037
			}
		}
	}

}

//A65_OPTOMA_CL_0001  //A35G2_CDS_CL_0001
void DrawOsdAcuBlackBorderMultipleSelectPattern(void)
{
	//fill full white
	DrawOsdRect(OSD_LAYER_TYPE__MAIN, 0, 0, PS_PANEL_ACT_HW, PS_PANEL_ACT_VW, COLOR_IDX__WHITE);
	DrawOsdAcuSelectPosition();
}

#endif

#if (ENABLE_COLOR_UNIFORMITY == TRUE)               //G100_Tim_0012, add, start
// ==============================================================================
// Function:
// DESCRIPTION: Draw the 9x7 white blocks in the black pattern for ACU calibration.
// Params: None
// Returns: None
//
// Modification History
// --------------------
// 2021/06/15, CL create
// --------------------
// ==============================================================================
void DrawOsdAcuGridPattern(void)
{
	uint8 i, j;
	uint8 nGridOffset = OSD_ACU_GRID_SIZE/2;
	//A65_OPTOMA_CL_0001 use percentage * native panel resolution to get x, y coordinate instead of fix number

	//fill full black
	DrawOsdRect(OSD_LAYER_TYPE__MAIN, 0, 0, PS_PANEL_ACT_HW, PS_PANEL_ACT_VW, COLOR_IDX__BLACK);  //A65_OPTOMA_CL_0002

	for(i = 0; i < 9; i++)
	{
		for(j = 0; j < 7; j++)
		{
			DrawOsdRect(OSD_LAYER_TYPE__MAIN, (uint16)(HPosRatio[i]*PS_PANEL_ACT_HW)-nGridOffset, (uint16)(VPosRatio[j]*PS_PANEL_ACT_VW)-nGridOffset, OSD_ACU_GRID_SIZE, OSD_ACU_GRID_SIZE, COLOR_IDX__WHITE);
		}
	}
}

// ==============================================================================
// Function:
// DESCRIPTION: Draw the 3 white blocks in the black pattern for ACU calibration.
// Params: None
// Returns: None
//
// Modification History
// --------------------
// 2021/06/15, CL create
// --------------------
// ==============================================================================
void DrawOsdAcuCenterGridPattern(void)
{
	uint8 i, j;
	uint8 nGridOffset = OSD_ACU_GRID_SIZE/2;
	//A65_OPTOMA_CL_0001 use percentage * native panel resolution to get x, y coordinate instead of fix number

	//fill full black
	DrawOsdRect(OSD_LAYER_TYPE__MAIN, 0, 0, PS_PANEL_ACT_HW, PS_PANEL_ACT_VW, COLOR_IDX__BLACK);  //A65_OPTOMA_CL_0002

	for(i = 0; i < 3; i++)
	{
		DrawOsdRect(OSD_LAYER_TYPE__MAIN, (uint16)(HCenterPosRatio[i]*PS_PANEL_ACT_HW)-nGridOffset, (uint16)(VCenterPosRatio[i]*PS_PANEL_ACT_VW)-nGridOffset, OSD_ACU_GRID_SIZE, OSD_ACU_GRID_SIZE, COLOR_IDX__WHITE);
	}
}

// ==============================================================================
// Function:
// DESCRIPTION: Draw the 9x7 black borders in the white pattern for ACU target selection.
// Params: None
// Returns: None
//
// Modification History
// --------------------
// 2021/06/15, Tim create
// --------------------
// ==============================================================================
void DrawOsdAcuBlackBorderPattern(void)
{
	uint8 i, j;
	uint8 nGridOffset = OSD_ACU_GRID_SIZE/2;
	//A65_OPTOMA_CL_0001 use percentage * native panel resolution to get x, y coordinate instead of fix number
	uint8 acTS = sUtilWarpDemo_Callback.fpDataMgr_ACU_Target_Select_Tmp_GetCb();
    UINT8 aucTS[TARGET_SELECT_LEN + 1] = {0, 0, 0};
    //CL add ucShow = 0 for trigger from OSD, ap set this value from another way
    m_ucCUShow = 0;

    aucTS[0] = acTS / 10;
    aucTS[1] = acTS % 10;

	for(i = 0; i < 9; i++)
	{
		for(j = 0; j < 7; j++)
		{
            if( ( ( aucTS[0] - 1 ) == i ) && ( ( aucTS[1] - 1 ) == j ) )
            {
                m_pnCUSelectEnableTable[i][j] = 1;
            }
            else
            {
                m_pnCUSelectEnableTable[i][j] = 0;
            }
		}
	}
    DrawOsdAcuSelectPosition();
}

#endif //ENABLE_COLOR_UNIFORMITY                    //G100_Tim_0012, add, end

void DrawOsdBlendWidth(eDIR eDir, uint16 nWidth)
{
	char pStr[32];
	uint8 nCharNum = 0;
	uint8 nOsdStrLen = 0;
	uint16 nRectPosX, nRectPosY, nStrPosX;
	uint16 nRectWidth, nRectHeight, nBorderWidth;
	eCOLOR_IDX eStrColor;
	eCOLOR_IDX eStrBgColor = COLOR_IDX__BLACK;
	BOOL bDoubleSize = FALSE;

	//width and height of the string area
	nRectWidth = OSD_BLEND_STR_WIDTH;
	nRectHeight = OSD_BLEND_STR_HEIGHT;
	nBorderWidth = OSD_BLEND_BORDER_WIDTH;

	//Let the OSD of "Blend Width" same size.
	//if(m_eCurrentResId == RES_ID__WUXGA)
	{
		bDoubleSize = TRUE;
		nRectWidth *= 2;
		nRectHeight *= 2;
		nBorderWidth *= 2;
	}


	if(m_eBlendWidthSel == eDir)
		eStrColor = m_stWarpConfig.stOsd.eCursorColor;
	else
		eStrColor = GridColorPaletteIndexGet();

	//start position of the string area
	switch(eDir)
	{
		default:
		case DIR__LEFT:
			nRectPosX = PS_PANEL_ACT_HW/2-nRectWidth-nRectWidth/2;
			nRectPosY = PS_PANEL_ACT_VW/2-nRectHeight/2;
			break;

		case DIR__RIGHT:
			nRectPosX = PS_PANEL_ACT_HW/2+nRectWidth/2;
			nRectPosY = PS_PANEL_ACT_VW/2-nRectHeight/2;
			break;

		case DIR__UP:
			nRectPosX = PS_PANEL_ACT_HW/2-nRectWidth/2;
			nRectPosY = PS_PANEL_ACT_VW/2-nRectHeight-nRectHeight/2;
			break;

		case DIR__DOWN:
			nRectPosX = PS_PANEL_ACT_HW/2-nRectWidth/2;
			nRectPosY = PS_PANEL_ACT_VW/2+nRectHeight/2;
			break;
	}

	//print string in buff and get string length
	nCharNum = sprintf(pStr, "%d", nWidth);
	//set the string to C789 char buff and get the the string length on OSD
	nOsdStrLen = PreparedOsdString((uint8*)pStr, nCharNum, OSD_FIXED_CHAR_LEN, eStrColor, eStrBgColor, eDir, bDoubleSize);
	//start address of the string
	nStrPosX = nRectPosX+nRectWidth-nBorderWidth-nOsdStrLen;
	//back ground color of the string area
	DrawOsdRect(OSD_LAYER_TYPE__MAIN, nRectPosX, nRectPosY, nRectWidth, nRectHeight, eStrBgColor);
	//draw the string from C789 char buff to OSD
	DrawOsdString(nCharNum, nStrPosX, nRectPosY, eDir);
	//draw the border of the string area in select mode
	DrawOsdRectBorder(nRectPosX, nRectPosY, nRectWidth, nRectHeight, COLOR_IDX__WHITE, GRID_LINE_WIDTH);  //H2PF_Simon_0037
}

eCOLOR_IDX GetBlendWidthCursorColor(eDIR eDir)
{
	if((m_ePatternType==PAT_TYPE__SEL_BLEND_WIDTH) && (m_eBlendWidthSel==eDir))
		return m_stWarpConfig.stOsd.eCursorColor;
	else if((m_ePatternType==PAT_TYPE__ADJ_BLEND_WIDTH) && (m_eBlendWidthSel==eDir))
		return COLOR_IDX__BLINK;
	else
		return GridColorPaletteIndexGet();
}

void DrawOsdBlendWidthPattern(void) //A35G2_BRC_Casper_0046
{
    BLEND_WIDTH stBlendWidth;
    BLEND_OFFSET stBlendOffset;
	eCOLOR_IDX eGridColor = GridColorPaletteIndexGet();
	eCOLOR_IDX eCursorColor;
	//base on current res. to convert blend width
    ConvertBlendWidth(&stBlendWidth);
    ConvertBlendOffset(&stBlendOffset);

	//Fill full black
	DrawOsdRect(OSD_LAYER_TYPE__MAIN, 0, 0, PS_PANEL_ACT_HW, PS_PANEL_ACT_VW, COLOR_IDX__BLACK_BKG);
	//Fill the background of blending area:
	if (stBlendWidth.nL > 0)	//Left
		DrawOsdRect(OSD_LAYER_TYPE__MAIN, stBlendOffset.nL, stBlendOffset.nT, stBlendWidth.nL, PS_PANEL_ACT_VW-(stBlendOffset.nT+stBlendOffset.nB), m_eOverlapColor);
	if (stBlendWidth.nR > 0)	//Right
		DrawOsdRect(OSD_LAYER_TYPE__MAIN, PS_PANEL_ACT_HW-(stBlendOffset.nR+stBlendWidth.nR), stBlendOffset.nT, stBlendWidth.nR, PS_PANEL_ACT_VW-(stBlendOffset.nT+stBlendOffset.nB), m_eOverlapColor);
	if (stBlendWidth.nT > 0)		//Top
		DrawOsdRect(OSD_LAYER_TYPE__MAIN, stBlendOffset.nL, stBlendOffset.nT, PS_PANEL_ACT_HW-(stBlendOffset.nL+stBlendOffset.nR), stBlendWidth.nT, m_eOverlapColor);
	if (stBlendWidth.nB > 0)	//Bottom
		DrawOsdRect(OSD_LAYER_TYPE__MAIN, stBlendOffset.nL, PS_PANEL_ACT_VW-(stBlendOffset.nB+stBlendWidth.nB), PS_PANEL_ACT_HW-(stBlendOffset.nL+stBlendOffset.nR), stBlendWidth.nB, m_eOverlapColor);

	//Edge of left blending area
	eCursorColor = GetBlendWidthCursorColor(DIR__LEFT);
	if(stBlendWidth.nL == 0)
	{
		if(stBlendOffset.nL != 0)
		{
			DrawOsdRect(OSD_LAYER_TYPE__MAIN, 0, 0, GRID_LINE_WIDTH, PS_PANEL_ACT_VW, eGridColor);
			DrawOsdRect(OSD_LAYER_TYPE__MAIN, stBlendOffset.nL-GRID_LINE_WIDTH, 0, GRID_LINE_WIDTH, PS_PANEL_ACT_VW, eCursorColor);
		}
		else
		{
			DrawOsdRect(OSD_LAYER_TYPE__MAIN, 0, 0, GRID_LINE_WIDTH, PS_PANEL_ACT_VW, eCursorColor);
		}
	}
	else
	{
		if(stBlendOffset.nL != 0)
		{
			DrawOsdRect(OSD_LAYER_TYPE__MAIN, stBlendOffset.nL-GRID_LINE_WIDTH, 0, GRID_LINE_WIDTH, PS_PANEL_ACT_VW, eGridColor);
		}
		DrawOsdRect(OSD_LAYER_TYPE__MAIN, 0, 0, GRID_LINE_WIDTH, PS_PANEL_ACT_VW, eGridColor);
		DrawOsdRect(OSD_LAYER_TYPE__MAIN, (stBlendOffset.nL+stBlendWidth.nL)-GRID_LINE_WIDTH, 0, GRID_LINE_WIDTH, PS_PANEL_ACT_VW, eCursorColor);
	}

	//Edge of right blending area
	eCursorColor = GetBlendWidthCursorColor(DIR__RIGHT);
	if(stBlendWidth.nR == 0)
	{
		if(stBlendOffset.nR != 0)
		{
			DrawOsdRect(OSD_LAYER_TYPE__MAIN, PS_PANEL_ACT_HW-GRID_LINE_WIDTH, 0, GRID_LINE_WIDTH, PS_PANEL_ACT_VW, eGridColor);
			DrawOsdRect(OSD_LAYER_TYPE__MAIN, PS_PANEL_ACT_HW-stBlendOffset.nR-GRID_LINE_WIDTH, 0, GRID_LINE_WIDTH, PS_PANEL_ACT_VW, eCursorColor);
		}
		else
		{
			DrawOsdRect(OSD_LAYER_TYPE__MAIN, PS_PANEL_ACT_HW-GRID_LINE_WIDTH, 0, GRID_LINE_WIDTH, PS_PANEL_ACT_VW, eCursorColor);
		}
	}
	else
	{
		if(stBlendOffset.nR != 0)
		{
			DrawOsdRect(OSD_LAYER_TYPE__MAIN, PS_PANEL_ACT_HW-stBlendOffset.nR-GRID_LINE_WIDTH, 0, GRID_LINE_WIDTH, PS_PANEL_ACT_VW, eGridColor);
		}
		DrawOsdRect(OSD_LAYER_TYPE__MAIN, PS_PANEL_ACT_HW-GRID_LINE_WIDTH, 0, GRID_LINE_WIDTH, PS_PANEL_ACT_VW, eGridColor);
		DrawOsdRect(OSD_LAYER_TYPE__MAIN, PS_PANEL_ACT_HW-(stBlendOffset.nR+stBlendWidth.nR)-GRID_LINE_WIDTH, 0, GRID_LINE_WIDTH, PS_PANEL_ACT_VW, eCursorColor);
	}

	//Edge of top blending area
	eCursorColor = GetBlendWidthCursorColor(DIR__UP);
	if(stBlendWidth.nT == 0)
	{
		if(stBlendOffset.nT != 0)
		{
			DrawOsdRect(OSD_LAYER_TYPE__MAIN, 0, 0, PS_PANEL_ACT_HW, GRID_LINE_WIDTH, eGridColor);
			DrawOsdRect(OSD_LAYER_TYPE__MAIN, 0, stBlendOffset.nT-GRID_LINE_WIDTH, PS_PANEL_ACT_HW, GRID_LINE_WIDTH, eCursorColor);
		}
		else
		{
			DrawOsdRect(OSD_LAYER_TYPE__MAIN, 0, 0, PS_PANEL_ACT_HW, GRID_LINE_WIDTH, eCursorColor);
		}
	}
	else
	{
		if(stBlendOffset.nT != 0)
		{
			DrawOsdRect(OSD_LAYER_TYPE__MAIN, 0, stBlendOffset.nT-GRID_LINE_WIDTH, PS_PANEL_ACT_HW, GRID_LINE_WIDTH, eGridColor);
		}
		DrawOsdRect(OSD_LAYER_TYPE__MAIN, 0, 0, PS_PANEL_ACT_HW, GRID_LINE_WIDTH, eGridColor);
		DrawOsdRect(OSD_LAYER_TYPE__MAIN, 0, (stBlendOffset.nT+stBlendWidth.nT)-GRID_LINE_WIDTH, PS_PANEL_ACT_HW, GRID_LINE_WIDTH, eCursorColor);
	}

	//Edge of bottom blending area
	eCursorColor = GetBlendWidthCursorColor(DIR__DOWN);
	if(stBlendWidth.nB == 0)
	{
		if(stBlendOffset.nB != 0)
		{
			DrawOsdRect(OSD_LAYER_TYPE__MAIN, 0, PS_PANEL_ACT_VW-GRID_LINE_WIDTH, PS_PANEL_ACT_HW, GRID_LINE_WIDTH, eGridColor);
			DrawOsdRect(OSD_LAYER_TYPE__MAIN, 0, PS_PANEL_ACT_VW-stBlendOffset.nB-GRID_LINE_WIDTH, PS_PANEL_ACT_HW, GRID_LINE_WIDTH, eCursorColor);
		}
		else
		{
			DrawOsdRect(OSD_LAYER_TYPE__MAIN, 0, PS_PANEL_ACT_VW-GRID_LINE_WIDTH, PS_PANEL_ACT_HW, GRID_LINE_WIDTH, eCursorColor);
		}
	}
	else
	{
		if(stBlendOffset.nB != 0)
		{
			DrawOsdRect(OSD_LAYER_TYPE__MAIN, 0, PS_PANEL_ACT_VW-stBlendOffset.nB-GRID_LINE_WIDTH, PS_PANEL_ACT_HW, GRID_LINE_WIDTH, eGridColor);
		}
		DrawOsdRect(OSD_LAYER_TYPE__MAIN, 0, PS_PANEL_ACT_VW-GRID_LINE_WIDTH, PS_PANEL_ACT_HW, GRID_LINE_WIDTH, eGridColor);
		DrawOsdRect(OSD_LAYER_TYPE__MAIN, 0, PS_PANEL_ACT_VW-(stBlendOffset.nB+stBlendWidth.nB)-GRID_LINE_WIDTH, PS_PANEL_ACT_HW, GRID_LINE_WIDTH, eCursorColor);
	}

	if(m_ePatternType != PAT_TYPE__BLEND_NO_CTRL_POINT)
	{
		//left blend width
		DrawOsdBlendWidth(DIR__LEFT, m_stWarpConfig.stOsd.nBlendWidthL);

	//right blend width
	DrawOsdBlendWidth(DIR__RIGHT, m_stWarpConfig.stOsd.nBlendWidthR);

	//top blend width
	DrawOsdBlendWidth(DIR__UP, m_stWarpConfig.stOsd.nBlendWidthT);

	//bottom blend width
	DrawOsdBlendWidth(DIR__DOWN, m_stWarpConfig.stOsd.nBlendWidthB);
	}

}


void DrawFullCursor(uint16 nP1X, uint16 nP1Y, eCURSOR_TYPE eCursorType, eCOLOR_IDX eColor)  //A65_OPTOMA_Doulas_0020
{
	uint8 nFirstBlankPosition, nSecondBlankPosition, nBlankWidth;
	uint8 nSection;
	uint8 i;
	uint16 nStartX, nStartY;
	uint8 nPointSizeX, nPointSizeY;		//A65_OPTOMA_Doulas_0134

	//if(m_ePatternType == PAT_TYPE__BLACKLEVEL_MOV_POINT)	//A65_OPTOMA_Doulas_0134 Modify
	{
		//X first
		if(nP1X - m_nBlackLevelPointSize/2 <= 0)
		{
			nPointSizeX = m_nBlackLevelPointSize -(m_nBlackLevelPointSize/2 - nP1X);
		}
		else if(nP1X + m_nBlackLevelPointSize/2 >= PS_PANEL_ACT_HW)
		{
			nPointSizeX = m_nBlackLevelPointSize - (nP1X  + m_nBlackLevelPointSize/2 - PS_PANEL_ACT_HW);
			nP1X = PS_PANEL_ACT_HW - nPointSizeX;
		}
		else
		{
			nPointSizeX = m_nBlackLevelPointSize;
			nP1X = nP1X - m_nBlackLevelPointSize/2;
		}
		//Y
		if(nP1Y - m_nBlackLevelPointSize/2 <= 0)
		{
			nPointSizeY = m_nBlackLevelPointSize -(m_nBlackLevelPointSize/2 - nP1Y);
		}
		else if(nP1Y + m_nBlackLevelPointSize/2 >= PS_PANEL_ACT_VW)
		{
			nPointSizeY = m_nBlackLevelPointSize - (nP1Y  + m_nBlackLevelPointSize/2 - PS_PANEL_ACT_VW);
			nP1Y = PS_PANEL_ACT_VW - nPointSizeY;
		}
		else
		{
			nPointSizeY = m_nBlackLevelPointSize;
			nP1Y = nP1Y- m_nBlackLevelPointSize/2;
		}
	}
	/*else
	{
		//X first
		if(nP1X - m_nBlackLevelPointSize/2 <= 0)
			nP1X = 0;
		else if(nP1X + m_nBlackLevelPointSize/2 >= PS_PANEL_ACT_HW)
			nP1X = PS_PANEL_ACT_HW - m_nBlackLevelPointSize;
		else
			nP1X = nP1X - m_nBlackLevelPointSize/2;
		//Y
		if(nP1Y - m_nBlackLevelPointSize/2 <= 0)
			nP1Y = 0;
		else if(nP1Y + m_nBlackLevelPointSize/2 >= PS_PANEL_ACT_VW)
			nP1Y = PS_PANEL_ACT_VW - m_nBlackLevelPointSize;
		else
			nP1Y = nP1Y- m_nBlackLevelPointSize/2;
	}*/

	if(eCursorType == CURSOR_TYPE__NORMAL)
	{
		//normal cursor
		//draw solid rectangle
	//	if(m_ePatternType == PAT_TYPE__BLACKLEVEL_MOV_POINT)		//A65_OPTOMA_Doulas_0134 Modify
			DrawOsdRect(OSD_LAYER_TYPE__MAIN, nP1X, nP1Y, nPointSizeX, nPointSizeY, eColor);
	//	else
	//		DrawOsdRect(OSD_LAYER_TYPE__MAIN, nP1X, nP1Y, m_nBlackLevelPointSize, m_nBlackLevelPointSize, eColor);
	}
	else if(eCursorType == CURSOR_TYPE__DELETE)
	{
		//delete cursor
		//draw solid rectangle
		DrawOsdRect(OSD_LAYER_TYPE__MAIN, nP1X, nP1Y, m_nBlackLevelPointSize, m_nBlackLevelPointSize, eColor);
		//fill black
		DrawOsdRect(OSD_LAYER_TYPE__MAIN, nP1X+1, nP1Y+1, m_nBlackLevelPointSize-2, m_nBlackLevelPointSize-2, COLOR_IDX__BLACK);
		//draw backslash
		DrawOsdLinePattern(nP1X, nP1Y, nP1X+m_nBlackLevelPointSize-1, nP1Y+m_nBlackLevelPointSize-1, eColor);
	}
	else if(eCursorType == CURSOR_TYPE__ADD)//add point fill black
	{
		//add cursor
		//Divide one solid line into 5 sections(3 visible, 2 invisible) to become dash one
		nSection = 5;
		nFirstBlankPosition = m_nBlackLevelPointSize/nSection;
		nSecondBlankPosition = m_nBlackLevelPointSize/2 + nFirstBlankPosition;
		nBlankWidth = m_nBlackLevelPointSize/nSection;

		//draw solid rectangle
		DrawOsdRect(OSD_LAYER_TYPE__MAIN, nP1X, nP1Y, m_nBlackLevelPointSize, m_nBlackLevelPointSize, eColor);
		//erase some boundary sector
		for(i=0; i<2; i++)
		{
			//up & bottom
			nStartY = nP1Y+ (m_nBlackLevelPointSize-1)*i;
			DrawOsdRect(OSD_LAYER_TYPE__MAIN, nP1X+nFirstBlankPosition, nStartY, nBlankWidth, GRID_LINE_WIDTH, COLOR_IDX__BLACK);
			DrawOsdRect(OSD_LAYER_TYPE__MAIN, nP1X+nSecondBlankPosition, nStartY, nBlankWidth, GRID_LINE_WIDTH, COLOR_IDX__BLACK);
		}
		for(i=0; i<2; i++)
		{
			//right & left
			nStartX = nP1X+ (m_nBlackLevelPointSize-1)*i;
			DrawOsdRect(OSD_LAYER_TYPE__MAIN, nStartX, nP1Y+nFirstBlankPosition, GRID_LINE_WIDTH, nBlankWidth, COLOR_IDX__BLACK);
			DrawOsdRect(OSD_LAYER_TYPE__MAIN, nStartX, nP1Y+nSecondBlankPosition, GRID_LINE_WIDTH, nBlankWidth, COLOR_IDX__BLACK);
		}
		//fill black
		DrawOsdRect(OSD_LAYER_TYPE__MAIN, nP1X+1, nP1Y+1, m_nBlackLevelPointSize-2, m_nBlackLevelPointSize-2, COLOR_IDX__BLACK);
	}

}

void DrawOsdBlackLevelCursor(eBLACKLEVEL_AREA eAreaSelect)  //A65_OPTOMA_Doulas_0020
{
	uint8 i = 0;
	uint8 nCount = 0;
	Node *pNextNode;
	Node *pTargetNode;
	PointT nMiddlePoint;

	BOOL bDeletePtMode = FALSE;
	eCOLOR_IDX eColor = m_eBlackLevelGridColor;


	pNextNode = m_pFirstNode[eAreaSelect];
	nCount = GetTotalCountOfList(m_pFirstNode[eAreaSelect]);

	if(m_ePatternType == PAT_TYPE__BLACKLEVEL_ADD_POINT)
	{
		//draw dash rectangle at middle
		pTargetNode = GetNextNode(m_pFirstNode[eAreaSelect], m_pnCurrentNode[m_eAreaSelection]);
		if(pTargetNode != NULL)
		{
			nMiddlePoint.x = (uint16)(pTargetNode->stPos.x + m_pnCurrentNode[m_eAreaSelection]->stPos.x)/2;
			nMiddlePoint.y = (uint16)(pTargetNode->stPos.y + m_pnCurrentNode[m_eAreaSelection]->stPos.y)/2;
			//DrawFullCursor(nMiddlePoint.x, nMiddlePoint.y, CURSOR_TYPE__ADD, m_stWarpConfig.stOsd.eCursorColor);  //A65_OPTOMA_Doulas_0097//A65_OPTOMA_Doulas_0024
			DrawFullCursor((uint16)(nMiddlePoint.x*m_fResRatioX), (uint16)(nMiddlePoint.y*m_fResRatioY), CURSOR_TYPE__ADD, m_stWarpConfig.stOsd.eCursorColor);  //A65_OPTOMA_Doulas_0097
		}
	}


	for(i= 0; i<nCount; i++)
	{
		if(pNextNode == m_pnCurrentNode[m_eAreaSelection])
		{
			if(m_ePatternType == PAT_TYPE__BLACKLEVEL_MOV_POINT)
			{
				eColor = COLOR_IDX__BLINK;  //A65_OPTOMA_Doulas_0024
			}
			else
			{
				eColor = m_stWarpConfig.stOsd.eCursorColor;  //A65_OPTOMA_Doulas_0024
			}
			if(m_ePatternType == PAT_TYPE__BLACKLEVEL_ADD_POINT)
			{
				eColor = m_eBlackLevelGridColor;
			}
			if(m_ePatternType == PAT_TYPE__BLACKLEVEL_DEL_POINT)
			{
				bDeletePtMode = TRUE;
			}
		}
		else
		{
			eColor = m_eBlackLevelGridColor;
		}

		if(m_ePatternType == PAT_TYPE__BLACKLEVEL_MOV_POINT)
		{
			//DrawFullCursor(pNextNode->stPos.x, pNextNode->stPos.y, CURSOR_TYPE__NORMAL, eColor);	//A65_OPTOMA_Doulas_0097
			DrawFullCursor((uint16)(pNextNode->stPos.x*m_fResRatioX), (uint16)(pNextNode->stPos.y*m_fResRatioY), CURSOR_TYPE__NORMAL, eColor);	//A65_OPTOMA_Doulas_0097
		}
		else
		{
			if(bDeletePtMode)
			{
				bDeletePtMode = FALSE;
				//DrawFullCursor(pNextNode->stPos.x, pNextNode->stPos.y, CURSOR_TYPE__DELETE, eColor);	//A65_OPTOMA_Doulas_0097
				DrawFullCursor((uint16)(pNextNode->stPos.x*m_fResRatioX), (uint16)(pNextNode->stPos.y*m_fResRatioY), CURSOR_TYPE__DELETE, eColor);	//A65_OPTOMA_Doulas_0097
			}
			else
			{
				//DrawFullCursor(pNextNode->stPos.x, pNextNode->stPos.y, CURSOR_TYPE__NORMAL, eColor);	//A65_OPTOMA_Doulas_0097
				DrawFullCursor((uint16)(pNextNode->stPos.x*m_fResRatioX), (uint16)(pNextNode->stPos.y*m_fResRatioY), CURSOR_TYPE__NORMAL, eColor);	//A65_OPTOMA_Doulas_0097
			}
		}
		pNextNode = pNextNode->pNext;
	}
}

void DrawOsdBlackLevelBoundary(eBLACKLEVEL_AREA eAreaSelect)  //A65_OPTOMA_Doulas_0020
{
	uint8 i = 0;
	uint8 nCount = 0;
	Node *pNextNode;
	eCOLOR_IDX eColorLine = m_eBlackLevelGridColor;



	pNextNode = m_pFirstNode[eAreaSelect];
	nCount = GetTotalCountOfList(m_pFirstNode[eAreaSelect]);

	for(i= 0; i<nCount; i++)
	{
		eColorLine = m_eBlackLevelGridColor;
		if((m_ePatternType == PAT_TYPE__BLACKLEVEL_ADD_POINT)
			&& (pNextNode == m_pnCurrentNode[m_eAreaSelection]))
		{
			eColorLine = m_stWarpConfig.stOsd.eCursorColor;  //A65_OPTOMA_Doulas_0024
		}


		if(i == nCount -1)
		{
			//last node, draw a line between last node and first node
			//DrawOsdLinePattern(pNextNode->stPos.x, pNextNode->stPos.y, m_pFirstNode[eAreaSelect]->stPos.x, m_pFirstNode[eAreaSelect]->stPos.y, eColorLine);	//A65_OPTOMA_Doulas_0097
			DrawOsdLinePattern((uint16)(pNextNode->stPos.x*m_fResRatioX), (uint16)(pNextNode->stPos.y*m_fResRatioY), (uint16)(m_pFirstNode[eAreaSelect]->stPos.x*m_fResRatioX), (uint16)(m_pFirstNode[eAreaSelect]->stPos.y*m_fResRatioY), eColorLine);		//A65_OPTOMA_Doulas_0097
		}
		else
		{
			//DrawOsdLinePattern(pNextNode->stPos.x, pNextNode->stPos.y, pNextNode->pNext->stPos.x, pNextNode->pNext->stPos.y, eColorLine);	//A65_OPTOMA_Doulas_0097
			DrawOsdLinePattern((uint16)(pNextNode->stPos.x*m_fResRatioX), (uint16)(pNextNode->stPos.y*m_fResRatioY), (uint16)(pNextNode->pNext->stPos.x*m_fResRatioX), (uint16)(pNextNode->pNext->stPos.y*m_fResRatioY), eColorLine);	//A65_OPTOMA_Doulas_0097
			pNextNode = pNextNode->pNext;
		}
	}
}

void DrawOsdBlackLevelAreaPattern(eBLACKLEVEL_AREA eAreaSelect)  //A65_OPTOMA_Doulas_0020
{
	//fill full black
	DrawOsdRect(OSD_LAYER_TYPE__MAIN, 0, 0, PS_PANEL_ACT_HW, PS_PANEL_ACT_VW, COLOR_IDX__BLACK_BKG);

	DrawOsdBlackLevelBoundary(eAreaSelect);
	DrawOsdBlackLevelCursor(eAreaSelect);
}

void DrawOsdBlackLevelPreviewPattern(void)  //A65_OPTOMA_Doulas_0020
{
	//fill full black
	DrawOsdRect(OSD_LAYER_TYPE__MAIN, 0, 0, PS_PANEL_ACT_HW, PS_PANEL_ACT_VW, COLOR_IDX__BLACK_BKG);

	//draw selected area
	DrawOsdBlackLevelBoundary(m_eAreaSelection);  //A65_OPTOMA_Doulas_0095 Modify//A65_OPTOMA_Doulas_0024
}

void ApplyBlendGamma(float fGamma)
{
	uint32 nRegVal = 0;
	uint16 ii;
	float fVal;
	uint16 nVal;
	uint16 nDataSize = 0;

	nRegVal = dvC789_Read(B7_EGBCT);
	nRegVal |= 0x0100; //Select R,G,B and Edge blend gamma CPU access enable

	//DRAM Addr
	dvC789_WriteToBuffer(B0_CPUWAD, RAM_DATA_ADDR);

	// RGBNK of B7_EGBCT
	dvC789_WriteToBuffer(B0_CPUDT, 0x00);
	dvC789_WriteToBuffer(B0_CPUDT, 7);
	nDataSize += 2;

	//high byte of B7_EGBCT register addr
	dvC789_WriteToBuffer(B0_CPUDT, 0x09);
	dvC789_WriteToBuffer(B0_CPUDT, GET_BYTE1(nRegVal));
	nDataSize += 2;

	// B7_EGBGMAD register addr
	dvC789_WriteToBuffer(B0_CPUDT, 0x28);
	dvC789_WriteToBuffer(B0_CPUDT, 0x00);
	dvC789_WriteToBuffer(B0_CPUDT, 0x28);
	dvC789_WriteToBuffer(B0_CPUDT, 0x00);
	nDataSize += 4;

	for ( ii = 0; ii < 1024; ii++ )
	{
		fVal = pow( ((float)ii / 1024), 1 / fGamma ) * 65536 + 0.5;
		nVal = ( fVal < 0 ) ? 0 : ( fVal > 65535 ) ? 65535 : (int)fVal;
		// B7_EGBGMDT register addr
		dvC789_WriteToBuffer(B0_CPUDT, 0x29);
		dvC789_WriteToBuffer(B0_CPUDT, GET_BYTE0(nVal));
		dvC789_WriteToBuffer(B0_CPUDT, 0x29);
		dvC789_WriteToBuffer(B0_CPUDT, GET_BYTE1(nVal));
		nDataSize += 4;
	}

	nRegVal &= (~0x0100);
	//high byte of B7_EGBCT register addr
	dvC789_WriteToBuffer(B0_CPUDT, 0x09);
	dvC789_WriteToBuffer(B0_CPUDT, GET_BYTE1(nRegVal));
	nDataSize += 2;

	// RGBNK of B0_BOSTAT
	dvC789_WriteToBuffer(B0_CPUDT, 0x00);
	dvC789_WriteToBuffer(B0_CPUDT, 0x00);
	nDataSize += 2;

	//Reset DRAM Addr
	dvC789_WriteToBuffer(B0_CPUWAD, 0x00);
	dvC789_Buffer_Flush();

	//execute DMA blend gamma
	Ram2Reg(nDataSize);
}


void ApplyEdgeBlendSetting(BLEND_WIDTH stBlendWidth, BLEND_OFFSET stBlendOffset, float fGamma) //A35G2_BRC_Casper_0046
{
	float lcoef_f, rcoef_f, tcoef_f, bcoef_f;
	uint16 lcoef_i, rcoef_i, tcoef_i, bcoef_i;
	uint32 nRegVal = 0x00;

    if((stBlendWidth.nL < 4) && (stBlendOffset.nL != 0))
    {
        stBlendWidth.nL = 3;
    }
	lcoef_f = (stBlendWidth.nL < 3) ? 0 : 1 / ((float)stBlendWidth.nL - 1) * 65536;

    if((stBlendWidth.nR < 4) && (stBlendOffset.nR != 0))
    {
        stBlendWidth.nR = 3;
    }
	rcoef_f = (stBlendWidth.nR < 3) ? 0 : ( 1 - 1 / ((float)stBlendWidth.nR - 1) ) * 65536;

    if((stBlendWidth.nT < 4) && (stBlendOffset.nT != 0))
    {
        stBlendWidth.nT = 3;
    }
	tcoef_f = (stBlendWidth.nT < 3) ? 0 : 1 / ((float)stBlendWidth.nT - 1) * 65536;

    if((stBlendWidth.nB < 4) && (stBlendOffset.nB != 0))
    {
        stBlendWidth.nB = 3;
    }
	bcoef_f = (stBlendWidth.nB < 3) ? 0 : ( 1 - 1 / ((float)stBlendWidth.nB - 1) ) * 65536;

	lcoef_i = (int)lcoef_f;
	rcoef_i = (int)rcoef_f;
	tcoef_i = (int)tcoef_f;
	bcoef_i = (int)bcoef_f;

	if ( lcoef_f > (float)lcoef_i ) { lcoef_i += 1; }
	if ( tcoef_f > (float)tcoef_i ) { tcoef_i += 1; }

	if ( lcoef_i > 0x7fff ) { lcoef_i = 0x7fff; }

	if(rcoef_i != 0)
	{
		if ( rcoef_i < 0x8000 ) { rcoef_i = 0x8000; }
	}

	if ( tcoef_i > 0x7fff ) { tcoef_i = 0x7fff; }

	if(bcoef_i != 0)
	{
		if ( bcoef_i < 0x8000 ) { bcoef_i = 0x8000; }
	}

	dvC789_WriteToBuffer(B7_LEGBHST, stBlendOffset.nL);
	if(lcoef_i == 0)
	{
		dvC789_WriteToBuffer(B7_HEGBINIT, 0x80);
		dvC789_WriteToBuffer(B7_LEGBHW, 0x0000);
		dvC789_WriteToBuffer(B7_LEGBCOEF, 0x0000);
	}
	else
	{
		nRegVal |= BIT0;
		dvC789_WriteToBuffer(B7_HEGBINIT, 0x00);
		dvC789_WriteToBuffer(B7_LEGBHW, stBlendWidth.nL);
		dvC789_WriteToBuffer(B7_LEGBCOEF, lcoef_i);
	}

	dvC789_WriteToBuffer(B7_REGBHST, PS_PANEL_ACT_HW-(stBlendOffset.nR+stBlendWidth.nR));
	if(rcoef_i == 0)
	{
		dvC789_WriteToBuffer(B7_REGBHW, 0x0000);
		dvC789_WriteToBuffer(B7_REGBCOEF, 0x0000);
	}
	else
	{
		nRegVal |= BIT1;
		dvC789_WriteToBuffer(B7_REGBHW, stBlendWidth.nR);
		dvC789_WriteToBuffer(B7_REGBCOEF, rcoef_i);
	}

	dvC789_WriteToBuffer(B7_TEGBVST, stBlendOffset.nT);
	if(tcoef_i == 0)
	{
		dvC789_WriteToBuffer(B7_VEGBINIT, 0x80);
		dvC789_WriteToBuffer(B7_TEGBVW, 0x0000);
		dvC789_WriteToBuffer(B7_TEGBCOEF, 0x0000);
	}
	else
	{
		nRegVal |= BIT2;
		dvC789_WriteToBuffer(B7_VEGBINIT, 0x00);
		dvC789_WriteToBuffer(B7_TEGBVW, stBlendWidth.nT);
		dvC789_WriteToBuffer(B7_TEGBCOEF, tcoef_i);
	}

	dvC789_WriteToBuffer(B7_BEGBVST, PS_PANEL_ACT_VW-(stBlendOffset.nB+stBlendWidth.nB));
	if(bcoef_i == 0)
	{
		dvC789_WriteToBuffer(B7_BEGBVW, 0x0000);
		dvC789_WriteToBuffer(B7_BEGBCOEF, 0x0000);
	}
	else
	{
		nRegVal |= BIT3;
		dvC789_WriteToBuffer(B7_BEGBVW, stBlendWidth.nB);
		dvC789_WriteToBuffer(B7_BEGBCOEF, bcoef_i);
	}
	dvC789_Buffer_Flush();

	if(nRegVal & 0x0F)
	{
		nRegVal |= BIT4;
		ApplyBlendGamma(fGamma);
	}

	dvC789_Write(B7_EGBCT, nRegVal);
}


void ApplyBlend(eAPPLY_BLEND eFlag) //A35G2_BRC_Casper_0046
{
	uint8 nBlendGammaIdx = 0;
	BLEND_WIDTH stBlendWidth;
	BLEND_OFFSET stBlendOffset;
	uint16 uiEgbct = 0;//A35G2_CDS_CL_0002//A35G2_Alan_0015

	LOG_MSG(db_HAL_WARPING, "ApplyBlend (%d %d)\r\n",eFlag, m_stWarpConfig.eWarpCtrl );
	LOG_MSG(db_HAL_WARPING, "m_ePatternType => (%d %d)\r\n\r\n",m_ePatternType , m_stWarpConfig.stOsd.bShowBlendOnWarpPattern);

	if((eFlag==APPLY_BLEND__DISABLE) || (m_stWarpConfig.eWarpCtrl == WARP_CTRL__BASIC)
#ifdef CUSTOM_BARCO //A35G2_Wesley_0093
	|| (m_stWarpConfig.stOsd.bBlendEnable == FALSE)
	//G100_Tim_0086, mark //|| (m_ePatternType != PAT_TYPE__OFF)  //fix ptoolset issue that blending effect will not apply if test pattern is on.
		|| ((m_stWarpConfig.stOsd.nBlendWidthT == 0 && m_stWarpConfig.stOsd.nBlendWidthB == 0 && m_stWarpConfig.stOsd.nBlendWidthL == 0 && m_stWarpConfig.stOsd.nBlendWidthR == 0)
		   && (m_stWarpConfig.stOsd.nBlendOffsetT == 0 && m_stWarpConfig.stOsd.nBlendOffsetB == 0 && m_stWarpConfig.stOsd.nBlendOffsetL == 0 && m_stWarpConfig.stOsd.nBlendOffsetR == 0))
		//G100_Tim_0092, add, start
#endif
        || (    ( m_stWarpConfig.stOsd.bShowBlendOnWarpPattern == FALSE )
             && (   ( m_ePatternType == PAT_TYPE__WARP_SEL_CTRL_POINT )
                 || ( m_ePatternType == PAT_TYPE__WARP_MOV_CTRL_POINT )
                 || ( m_ePatternType == PAT_TYPE__WARP_NO_CTRL_POINT )
                 || ( m_ePatternType == PAT_TYPE__SEL_BLEND_WIDTH )
                 || ( m_ePatternType == PAT_TYPE__ADJ_BLEND_WIDTH )
                 || ( m_ePatternType == PAT_TYPE__BLEND_NO_CTRL_POINT )
                 )
             )
    )
    {
	    #if (ADVANCED_BLEND == FALSE)  //A35G2_CDS_CL_0002//A35G2_Alan_0015
	    {
    		if( (eFlag==APPLY_BLEND__DISABLE)  ||
        		( ( m_stWarpConfig.stOsd.bShowBlendOnWarpPattern == FALSE )
                     && (   ( m_ePatternType == PAT_TYPE__WARP_SEL_CTRL_POINT )
                         || ( m_ePatternType == PAT_TYPE__WARP_MOV_CTRL_POINT )
                         || ( m_ePatternType == PAT_TYPE__WARP_NO_CTRL_POINT )
                         || ( m_ePatternType == PAT_TYPE__SEL_BLEND_WIDTH )
                         || ( m_ePatternType == PAT_TYPE__ADJ_BLEND_WIDTH )
                         || ( m_ePatternType == PAT_TYPE__BLEND_NO_CTRL_POINT )
                         )
                )
            )//get uiEgbct (basic + disable or ap + disable)
    		{
                uiEgbct = (UINT16)dvC789_Read(B7_EGBCT);
                LOG_MSG(db_HAL_WARPING, "Read uiEgbct = %d\n",uiEgbct);
                //save in global variable if value is not zero
                if(uiEgbct)
                {
                    m_egBct = uiEgbct;
                    LOG_MSG(db_HAL_WARPING, "save to global variable = %d\n",m_egBct);
    			}
    			dvC789_Write(B7_EGBCT, 0x00);
    		}
    		else if(eFlag==APPLY_BLEND__BY_CONFIG)//reapply uiEgbct (basic + config)
    		{
    			LOG_MSG(db_HAL_WARPING, "basic + config mode reapply blending = %d\n",m_egBct);
    			//dvC789_Write(B7_EGBCT, m_egBct);
    			if(palDataMgr_CurrentBlendMemoryType() == WARPING_TYPE_AP_MYSTIQUE ||
    			   palDataMgr_CurrentBlendMemoryType() == WARPING_TYPE_AP)
    			{
    			    LOG_MSG(db_HAL_WARPING, "   reapply blend at AP type case 1\r\n");
    			    m_bApBlendApply = TRUE;
    			}
    			else
    			{
    			    LOG_MSG(db_HAL_WARPING, "reapply blend at Normal type\r\n");
        		    halWarp_BlendingSet();
        		}
    		}
    	}
	    #else
	    {
		    dvC789_Write(B7_EGBCT, 0x00);
        }
	    #endif
	}
	else if(m_stWarpConfig.eWarpCtrl == WARP_CTRL__ADVANCED)
	{
	    #if (ADVANCED_BLEND == FALSE)
	    {
    	    if(eFlag==APPLY_BLEND__BY_CONFIG)//reapply uiEgbct (basic + config)
    		{
    		    LOG_MSG(db_HAL_WARPING, "Warp Ctrl is ADVANCED, but (ADVANCED_BLEND == FALSE)\r\n");
    		    if(palDataMgr_CurrentBlendMemoryType() == WARPING_TYPE_AP_MYSTIQUE ||
    			   palDataMgr_CurrentBlendMemoryType() == WARPING_TYPE_AP)
    			{
    			    LOG_MSG(db_HAL_WARPING, "   reapply blend at AP type case 2\r\n");
                    m_bApBlendApply = TRUE;
    			}
    			else
    			{
        			halWarp_BlendingSet();
                }
    		}
	    }
	    #else
	    {
    		nBlendGammaIdx = m_stWarpConfig.stOsd.nBlendGamma;
    		if (nBlendGammaIdx > BLEND_GAMMA_MAX)
    			nBlendGammaIdx = BLEND_GAMMA_MAX;
    		//base on current res. to convert blend width
    		ConvertBlendWidth(&stBlendWidth);
    		ConvertBlendOffset(&stBlendOffset);
    		ApplyEdgeBlendSetting(stBlendWidth, stBlendOffset, m_pfBlendGammaTable[nBlendGammaIdx]);
        }
		#endif
	}
	else if((m_stWarpConfig.eWarpCtrl == WARP_CTRL__AP) && (m_ePatternType == PAT_TYPE__OFF))  //A65_OPTOMA_CL_0010
	{
		#ifdef CUSTOM_CHRISTIE  //A35G2_CDS_CL_0002//A35G2_Alan_0015
            if(eFlag==APPLY_BLEND__BY_CONFIG)//reapply uiEgbct (ap + config)
            {
                LOG_MSG(db_HAL_WARPING, "AP mode: Pattern Off and reapply blend flag set\r\n\r\n");
                m_bApBlendApply = TRUE;   //  //H2PF_Simon_0073

                #if 0
                LOG_MSG(db_HAL_WARPING, "ap + config mode reapply blending = %d\n",m_egBct);
				if(m_egBct)		//A70Gen2_Doulas_0049 Modify
				{
					dvC789_Write(B7_EGBCT, m_egBct);
				}
				#endif
            }
		#else
            if(halWarping_TwistLinkFlag_Get() == FALSE)//A65_OPTOMA_CL_0016
            {
                LOG_MSG(db_HAL_WARPING, "%s, Warp control = %d\r\n", __FUNCTION__, m_stWarpConfig.eWarpCtrl);
                m_bApBlendApply = TRUE;
            }
		#endif
	}
}

#if 0
void EnableTestPattern(BOOL bEnable, eBKG_COLOR eBkgColor, eAPPLY_BLEND eApplyBlend)
{
	uint16 nData;
	uint16 nHST, nVST, nHW, nVW;
	uint8 nOSDMODE;

	m_eOsdPosition = bBeforeWarp;  //CL
	if (bEnable)
	{
		nHST = PS_PANEL_ACT_HST;
		//nVST = PS_PANEL_ACT_VST;	//G100_Doulas_0043 Modify
		nVST = PS_PANEL_ACT_VST + utilWarp_Panel_Vstart_Offset_Get();	//G100_Doulas_0043 Modify
		nHW = PS_PANEL_ACT_HW;
		nVW = PS_PANEL_ACT_VW;

		//output osd pattern setting
		dvC789_WriteToBuffer(B1_OSDSAD, m_OsdAddrLayerCurrent);
		//Insert before warping:When OSDACTHST = 20(0x14), OSD area starts from 21 PICLK after PIHS.
		//nData = nHST-1;
		if(bBeforeWarp == OSD_POS__BEFORE_WARP)
		{
			nData = nHST-1; //input side
		}
		else
		{
			nData = nHST-15; //output side
		}
		dvC789_WriteToBuffer(B1_OSDACTHST, nData);
		dvC789_WriteToBuffer(B1_OSDACTHW, nHW);
		//When OSDACTVST = 4(0x04), OSD area starts from 3 PIHS (or POHS) after PIVS
		//nData = nVST;
		if(bBeforeWarp == OSD_POS__BEFORE_WARP)
		{
			nData = nVST; //input side
		}
		else
		{
			nData = nVST+1; //output side
		}
		dvC789_WriteToBuffer(B1_OSDACTVST, nData);
		dvC789_WriteToBuffer(B1_OSDACTVW, nVW);

		//config OSDMODE
		//if (eBkgColor == BKG_COLOR__TRANSPARENT)
		//	nOSDMODE = 0x03; /*Bitmap OSD is displayed with transparency.*/
		//else
		//	nOSDMODE = 0x02; /*Bitmap OSD is displayed without transparency.*/
		if (eBkgColor == BKG_COLOR__TRANSPARENT)
		{
		        //CL
			if(bBeforeWarp == OSD_POS__BEFORE_WARP)
			{
				nOSDMODE = 0x03; /*Bitmap OSD is displayed with transparency.INPUT SIDE*/
			}
			else
			{
				nOSDMODE = 0x07; /*Bitmap OSD is displayed with transparency. OUTPUT SIDE*/
			}
		}
		else
		{
		        //CL
			if(bBeforeWarp == OSD_POS__BEFORE_WARP)
			{
				nOSDMODE = 0x02; /*Bitmap OSD is displayed without transparency.INPUT SIDE*/
			}
			else
			{
				nOSDMODE = 0x06; /*Bitmap OSD is displayed without transparency.OUTPUT SIDE*/
			}
		}
		dvC789_WriteToBuffer(B1_OSDMODE, nOSDMODE);

		dvC789_Buffer_Flush();

		m_OsdAddrLayerCurrent = ( m_OsdAddrLayerCurrent != m_OsdAddrLayer0 ) ? m_OsdAddrLayer0 : m_OsdAddrLayer1;
	}
	else
	{
		//dvC789_Write(B1_OSDMODE, 0x00);
		if(bBeforeWarp == OSD_POS__BEFORE_WARP)
		{
			dvC789_Write(B1_OSDMODE, 0x00);  //INPUT SIDE
		}
		else
		{
			dvC789_Write(B1_OSDMODE, 0x04);  //OUTPUT SIDE
		}
	}
	ApplyBlend(eApplyBlend);
}
#else
void EnableTestPattern(BOOL bEnable, eBKG_COLOR eBkgColor, eAPPLY_BLEND eApplyBlend, eAPPLY_BLACKLEVEL eApplyBlackLevel, eOSD_POS bBeforeWarp)  //A65_OPTOMA_Doulas_0020
{
	uint16 nData;
	uint16 nHST, nVST, nHW, nVW;
	uint8 nOSDMODE;


	m_eOsdPosition = bBeforeWarp;
	if (bEnable)
	{
		nHST = PS_PANEL_ACT_HST;
		//nVST = PS_PANEL_ACT_VST;	//G100_Doulas_0043 Modify
		nVST = PS_PANEL_ACT_VST + utilWarp_Panel_Vstart_Offset_Get();	//G100_Doulas_0043 Modify
		nHW = PS_PANEL_ACT_HW;
		nVW = PS_PANEL_ACT_VW;

		//output osd pattern setting
		dvC789_WriteToBuffer(B1_OSDSAD, m_OsdAddrLayerCurrent);
		//Insert before warping:When OSDACTHST = 20(0x14), OSD area starts from 21 PICLK after PIHS.
		if(bBeforeWarp == OSD_POS__BEFORE_WARP)  //A65_OPTOMA_Doulas_0020
		{
			nData = nHST-1; //input side
		}
		else
		{
			nData = nHST-15; //output side
		}
		dvC789_WriteToBuffer(B1_OSDACTHST, nData);
		dvC789_WriteToBuffer(B1_OSDACTHW, nHW);
		//When OSDACTVST = 4(0x04), OSD area starts from 3 PIHS (or POHS) after PIVS
		if(bBeforeWarp == OSD_POS__BEFORE_WARP)  //A65_OPTOMA_Doulas_0020
		{
			nData = nVST; //input side
		}
		else
		{
			nData = nVST+1; //output side
		}
		dvC789_WriteToBuffer(B1_OSDACTVST, nData);
		dvC789_WriteToBuffer(B1_OSDACTVW, nVW);

		//config OSDMODE
		if (eBkgColor == BKG_COLOR__TRANSPARENT)  //A65_OPTOMA_Doulas_0020
		{
			if(bBeforeWarp == OSD_POS__BEFORE_WARP)
			{
				nOSDMODE = 0x03; /*Bitmap OSD is displayed with transparency.INPUT SIDE*/
			}
			else
			{
				nOSDMODE = 0x07; /*Bitmap OSD is displayed with transparency. OUTPUT SIDE*/
			}
		}
		else
		{
			if(bBeforeWarp == OSD_POS__BEFORE_WARP)
			{
				nOSDMODE = 0x02; /*Bitmap OSD is displayed without transparency.INPUT SIDE*/
			}
			else
			{
				nOSDMODE = 0x06; /*Bitmap OSD is displayed without transparency.OUTPUT SIDE*/
			}
		}
		dvC789_WriteToBuffer(B1_OSDMODE, nOSDMODE);

		dvC789_Buffer_Flush();

		m_OsdAddrLayerCurrent = ( m_OsdAddrLayerCurrent != m_OsdAddrLayer0 ) ? m_OsdAddrLayer0 : m_OsdAddrLayer1;
	}
	else
	{
		if(bBeforeWarp == OSD_POS__BEFORE_WARP)  //A65_OPTOMA_Doulas_0020
		{
			dvC789_Write(B1_OSDMODE, 0x00);  //INPUT SIDE
		}
		else
		{
			dvC789_Write(B1_OSDMODE, 0x04);  //OUTPUT SIDE
		}
	}
	ApplyBlend(eApplyBlend);
#if (ADVANCED_BLACK_LEVEL == TRUE)
	ApplyBlackLevel(eApplyBlackLevel);    //A65_OPTOMA_Doulas_0020
#endif
}

#endif //ENABLE_COLOR_UNIFORMITY        //G100_Tim_0012, add

uint32 TimeElapsedMs(void)
{
	struct timespec stEndTime;

	clock_gettime(CLOCK_MONOTONIC, &stEndTime);
	return (stEndTime.tv_sec - m_stStartTime.tv_sec)*1000 + (stEndTime.tv_nsec - m_stStartTime.tv_nsec)/1000000;
}

void utilWarp_GetDeviceId(void)
{
	UINT32 nData = 0;
    nData = dvC789_Read(B10_DEVICECODE);
	LOG_MSG(db_HAL_WARPING, "device ID is %02X\r\n", nData);
}

UINT8 utilWarp_SysInit(eRES_ID eCurrentResId, eRES_ID e3dResId)  //A65_OPTOMA_Doulas_0167 Modify//G100_Doulas_0065
{
	uint16 nIdx;
	UINT8 ucResult = FLASH_ACCESS__PASS;	//A65_OPTOMA_Doulas_0167

	LOG_MSG(db_HAL_WARPING, "%s Start\r\n", __FUNCTION__);

	m_e3dResId = e3dResId;                                        //G100_Doulas_0065
	m_nNativeResH = m_ModeTable[NATIVE_RES_ID].nHActive;
	m_nNativeResV = m_ModeTable[NATIVE_RES_ID].nVActive;
	m_n3dResH = m_ModeTable[m_e3dResId].nHActive;
	m_n3dResV = m_ModeTable[m_e3dResId].nVActive;

	NATIVE_WP_HW_GRD = ((m_nNativeResH%16)==0) ? m_nNativeResH/16 : m_nNativeResH/16+1;
	NATIVE_WP_VW_GRD = ((m_nNativeResV%16)==0) ? m_nNativeResV/16 : m_nNativeResV/16+1;
	THREED_WP_HW_GRD = ((m_n3dResH%16)==0) ? m_n3dResH/16 : m_n3dResH/16+1;
	THREED_WP_VW_GRD = ((m_n3dResV%16)==0) ? m_n3dResV/16 : m_n3dResV/16+1;
	m_f3dResRatioX = (float)m_n3dResH/(float)m_nNativeResH;
	m_f3dResRatioY = (float)m_n3dResV/(float)m_nNativeResV;

	for ( nIdx = 0; nIdx < DEF_NUM_CUR_MAX; nIdx++ )
	{
		PS_CUR_DEF_OX[nIdx] = (float)(m_nNativeResH * nIdx) / (float)(DEF_NUM_CUR_MAX_H-1);
		PS_CUR_DEF_OY[nIdx] = (float)(m_nNativeResV * nIdx) / (float)(DEF_NUM_CUR_MAX_V-1);
		PS_CUR_DEF_IX[nIdx] = (float)(m_nNativeResH * nIdx) / (float)(DEF_NUM_CUR_MAX_H-1);
		PS_CUR_DEF_IY[nIdx] = (float)(m_nNativeResV * nIdx) / (float)(DEF_NUM_CUR_MAX_V-1);
	}
	m_OsdAddrLayer0 = OSD_BASIC_ADDR;
	m_OsdAddrLayer1 = OSD_BASIC_ADDR+0x240000;
	m_OsdAddrLayerCurrent = m_OsdAddrLayer0;

	clock_gettime(CLOCK_MONOTONIC, &m_stStartTime);

	PM_GRID = m_stWarpConfig.stOsd.pfControlPoint;   //G100_Doulas_0065

	InitOsdPatternColor();
	InitOsdChar();
	m_ucColorPalette = COLOR_PALETTE__OSD;  //A65_OPTOMA_CL_0007

    m_egBct = 0;
    m_bApBlendApply = FALSE;
    m_bApBlacklevelApply = FALSE;
    m_bApOnlyBlendApply = FALSE;

	//init output timing
	//utilWarp_SetOutputTiming(eCurrentResId);   //A65_OPTOMA_Doulas_0167 remove//G100_Doulas_0065
	SetOutputTiming(eCurrentResId);				//A65_OPTOMA_Doulas_0167
	ApplyWarp(APPLY_WARP__RE_CALC);				//A65_OPTOMA_Doulas_0167

	//kenton_temp_check reset RTCT
	dvC789_WriteToBuffer(BN_RTCT, RTCT_THRU);
	//set BLACK as transparent color
	//halWarpOSD_OSD_Transparent_Set(TRUE,
    //                               COLOR_IDX__BLACK_BKG,
    //                               COLOR_IDX__BLACK_BKG,
    //                               COLOR_IDX__BLACK_BKG,
    //                               m_eOverlapColor
    //                               );

	//Enable color palette
	dvC789_WriteToBuffer(B1_OSDCT, 0x40);
	//address of black level area
	dvC789_WriteToBuffer(B9_EBIASSAD, OSD_BIAS_ADDR);
	//Specify the edge blend bias data memory read linefeed width by EBIASMWI[7:0]x256 bytes
	dvC789_WriteToBuffer(B9_EBIASMWI, OSD_BIAS_MWI);
	////address of dot by dot blending area
	dvC789_WriteToBuffer(B9_EGBDBDSAD, OSD_DBD_ADDR);
	dvC789_WriteToBuffer(B9_EGBDBDMWI, OSD_DBD_8BIT_MWI);
	dvC789_Buffer_Flush();

	//load warp config
	if(LoadWarpConfig()==FLASH_ACCESS__PASS)
	{
		LOG_MSG(db_HAL_WARPING, "123i (%f,%f)(%d)\r\n",m_stWarpConfig.stOsd.pfControlPoint[16][16].x,m_stWarpConfig.stOsd.pfControlPoint[16][16].y,m_stWarpConfig.stOsdBasic.stWArpGeoPara.m_cWarpAutoFilter);	//G100_Doulas_0047 Modify//G100_Doulas_0028
		//load warp config pass than verify it
		if(!VerifyWarpConfig())
		{
			ucResult = FLASH_ACCESS__VERIFY_CONFIG_ERR;	//A65_OPTOMA_Doulas_0167
			//verify warp config fail, reset warp config
			LOG_MSG(db_HAL_WARPING,"verify warp config fail\r\n");
			utilWarp_ResetWarpConfig();
		}
	}
	else
	{
		ucResult = FLASH_ACCESS__FAIL;	//A65_OPTOMA_Doulas_0167
		//load warp config fail, reset warp config
		LOG_MSG(db_HAL_WARPING,"load warp config fail\r\n");
		utilWarp_ResetWarpConfig();
	}

#if 0
#ifdef SUPPORT_WARP_CONTROL_PC
	//calc bias gamma table for all palette
	CalcEgbBiasGammaTable(3, 0, TRUE);
	//clear bias area
	C821_ClearBiasArea();
	//load bias area from FLASH
	if(m_stWarpConfig.stPc.bIsBiasAreaValid)
		C821_LoadBiasAreaFromFlash(m_stWarpConfig.stPc.nFlashIdx);

	//clear dot by dot blending
	C821_ClearDotByDotArea();
	//load dot by dot blending from FLASH
	if(m_stWarpConfig.stPc.bIsDotByDotAreaValid)
		C821_LoadDotByDotAreaFromFlash(m_stWarpConfig.stPc.nFlashIdx);
#endif
#endif

	//apply warp config
	//m_nSelCtlPointX = 0;  //A65_OPTOMA_Doulas_0020
	//m_nSelCtlPointY = 0;  //A65_OPTOMA_Doulas_0020
	//InitGridColor();      //A65_OPTOMA_Doulas_0020
	InitRunTimeParam();     //A65_OPTOMA_Doulas_0020
	dvC789_Write(B1_OSDMODE, 0x00);		//G100_Doulas_0027 debug

	if(m_stWarpConfig.eWarpCtrl == WARP_CTRL__ADVANCED)		//G100_Doulas_0027 Add,basic return
	{
		ApplyWarp(APPLY_WARP__BY_CONFIG);

#if (ADVANCED_BLEND == TRUE)
		ApplyBlend(APPLY_BLEND__BY_CONFIG);
#endif

#if (ADVANCED_BLACK_LEVEL == TRUE)
		ApplyBlackLevel(APPLY_BLACKLEVEL__BY_CONFIG);  //A65_OPTOMA_Doulas_0020
#endif
	}
	else		//A65_OPTOMA_Doulas_0167 Add
	{
		ApplyWarp(APPLY_WARP__RE_CALC);

#if (ADVANCED_BLEND == TRUE)
		ApplyBlend(APPLY_BLEND__BY_CONFIG);
#endif

#if (ADVANCED_BLACK_LEVEL == TRUE)
		ApplyBlackLevel(APPLY_BLACKLEVEL__BY_CONFIG);
#endif
	}
	//ApplyBlackLevel(APPLY_BLACKLEVEL__BY_CONFIG);  //A65_OPTOMA_Doulas_0020

	LOG_MSG(db_HAL_WARPING, "%s End\r\n", __FUNCTION__);

	return ucResult;	//A65_OPTOMA_Doulas_0167
}

uint8 utilWarp_SaveOsdPreset(uint8 nIdx)
{
	OSD_WARP_FLASH stWrite;
	OSD_WARP_FLASH stRead;
	uint16 nDataSize = sizeof(OSD_WARP_FLASH);
	uint32 nFlashAddr = 0;
	uint16 i = 0;
	uint16 nLoop = 0, nId = 0;  //G100_Doulas_0059

	if(nIdx>=OSD_PRESET_INVALID)
		return FLASH_ACCESS__PRESET_IDX_ERR;

	nFlashAddr = OSD_WARP_PRESET_FLASH_ADDR+OSD_WARP_PRESET_FLASH_SIZE*nIdx;  //G100_Doulas_0059

	//construct data
	memcpy(&(stWrite.pnHeader), m_pnHeader, sizeof(m_pnHeader));
	memcpy(&(stWrite.stOsdWarp), &m_stWarpConfig.stOsd, sizeof(OSD_WARP));

	stWrite.nChecksum = CalcChecksum((uint8*)(&stWrite), sizeof(OSD_WARP_FLASH)-sizeof(stWrite.nChecksum));

	//write data to C789 DRAM
	WriteC789Ram((uint8*)(&stWrite), RAM_DATA_ADDR, nDataSize);

	nLoop = (nDataSize%4096 == 0) ? nDataSize/4096 : nDataSize/4096+1;   //G100_Doulas_0059
	//Erase Flash
	for(nId=0; nId<nLoop; nId++)
	{
		if(!EraseFlashSector(nFlashAddr+(0x1000*nId), FLASH_ERASE_MODE__SE4K))
			return FLASH_ACCESS__ERASE_FLASH_ERR;
	}

	//write data to FLASH by DMA
	if(!Dram_Reg2Flash(RAM_DATA_ADDR, nFlashAddr, nDataSize))
        return FLASH_ACCESS__RAM_TO_FLASH_ERR;

	//read data from FLASH by DMA
	if(!Dram_Flash2Reg(RAM_DATA_ADDR, nFlashAddr, nDataSize))
        return FLASH_ACCESS__FLASH_TO_RAM_ERR;

	//read data from C789 DRAM
	ReadC789Ram((uint8*)(&stRead), RAM_DATA_ADDR, nDataSize);

	// compare
	if(memcmp((uint8*)(&stWrite), (uint8*)(&stRead), nDataSize))
        return FLASH_ACCESS__COMPARE_ERR;

    return FLASH_ACCESS__PASS;
}


uint8 utilWarp_LoadOsdPreset(uint8 nIdx)
{
	OSD_WARP_FLASH stRead;
	WARP_CONFIG stWarpConfigBackup;
	uint16 nDataSize = sizeof(OSD_WARP_FLASH);
	uint32 nFlashAddr = 0;

	if(nIdx>=OSD_PRESET_INVALID)
		return FLASH_ACCESS__PRESET_IDX_ERR;

	nFlashAddr = OSD_WARP_PRESET_FLASH_ADDR+OSD_WARP_PRESET_FLASH_SIZE*nIdx;    //G100_Doulas_0059

	//read data from FLASH by DMA
	if(!Dram_Flash2Reg(RAM_DATA_ADDR, nFlashAddr, nDataSize))
        return FLASH_ACCESS__FLASH_TO_RAM_ERR;

	//read data from C789 DRAM
	ReadC789Ram((uint8*)(&stRead), RAM_DATA_ADDR, nDataSize);

	//verify header
    if (0 != memcmp(stRead.pnHeader, m_pnHeader, sizeof(m_pnHeader)))
		return FLASH_ACCESS__VERIFY_HEADER_ERR;

	//verify checksum
	if(stRead.nChecksum != CalcChecksum((uint8*)(&stRead), sizeof(OSD_WARP_FLASH)-sizeof(stRead.nChecksum)))
		return FLASH_ACCESS__VERIFY_CHECKSUM_ERR;

	//backup current warp config
	memcpy(&stWarpConfigBackup, &m_stWarpConfig, sizeof(WARP_CONFIG));

	//load new osd warp
	memcpy(&m_stWarpConfig.stOsd, &(stRead.stOsdWarp), sizeof(OSD_WARP));
	//set warp control to OSD
	m_stWarpConfig.eWarpCtrl = WARP_CTRL__ADVANCED;

	//verify new warp config
	if(!VerifyWarpConfig())
	{
		//restore origin warp config
		memcpy(&m_stWarpConfig, &stWarpConfigBackup, sizeof(WARP_CONFIG));
		return FLASH_ACCESS__VERIFY_CONFIG_ERR;
	}

	//apply new warp config
	//m_nSelCtlPointX = 0;  //A65_OPTOMA_Doulas_0020
	//m_nSelCtlPointY = 0;  //A65_OPTOMA_Doulas_0020
	//InitGridColor();      //A65_OPTOMA_Doulas_0020
    InitRunTimeParam();     //A65_OPTOMA_Doulas_0020
	ApplyWarp(APPLY_WARP__BY_CONFIG);
	ApplyBlend(APPLY_BLEND__BY_CONFIG);
#if (ADVANCED_BLACK_LEVEL == TRUE)
    ApplyBlackLevel(APPLY_BLACKLEVEL__BY_CONFIG);  //A65_OPTOMA_Doulas_0020
#endif
	//save warp config
	//return SaveWarpConfig();
	return FLASH_ACCESS__PASS;  //A65_OPTOMA_Doulas_0020
}

uint8 utilWarp_ClearOsdPreset(void)
{
#ifdef WARP_SAVE_FILE_SYSTEM			//G100_Doulas_0027 Modify
	UINT8  ucResult;
	uint16 i = 0;

	for(i = 0; i < OSD_PRESET_INVALID; i++)
	{
		ucResult = utilWarp_ADVMemoryWarpClearFile(i);  //remove file

		if(ucResult != FLASH_ACCESS__PASS)
        	return FLASH_ACCESS__ERASE_FLASH_ERR;
	}

    return FLASH_ACCESS__PASS;
#else
	uint32 nFlashAddr = 0;
	uint16 i = 0;
	uint16 nDataSize = sizeof(OSD_WARP_FLASH);   //G100_Doulas_0059
	uint16 nLoop = 0, nId = 0;

	for(i = 0; i < OSD_PRESET_INVALID; i++)
	{
		nFlashAddr = OSD_WARP_PRESET_FLASH_ADDR+OSD_WARP_PRESET_FLASH_SIZE*i;   //G100_Doulas_0059
		nLoop = (nDataSize%4096 == 0) ? nDataSize/4096 : nDataSize/4096+1;
		//Erase Flash
		for(nId=0; nId<nLoop; nId++)
		{
			if(!EraseFlashSector(nFlashAddr+(0x1000*nId), FLASH_ERASE_MODE__SE4K))
				return FLASH_ACCESS__ERASE_FLASH_ERR;
		}
	}

    return FLASH_ACCESS__PASS;
#endif
}

void utilWarp_ResetWarpConfig(void) //A35G2_BRC_Casper_0046
{
    #ifndef CUSTOM_CHRISTIE   //A65_OPTOMA_Doulas_0020      //A35G2_Simon_0068
	ResetWarpConfig();
	if(SaveWarpConfig()!=FLASH_ACCESS__PASS)
		LOG_MSG(db_ASSERT,"save warp config fail\r\n");		//A65_OPTOMA_Doulas_0094 Modify

	InitRunTimeParam();
	//apply warp, blend, blacklevel
	ApplyWarp(APPLY_WARP__BY_CONFIG);
	ApplyBlend(APPLY_BLEND__BY_CONFIG);
	ApplyBlackLevel(APPLY_BLACKLEVEL__BY_CONFIG);
    #else
	//reset warp config
	memset(&m_stWarpConfig, 0, sizeof(WARP_CONFIG));
	m_stWarpConfig.eWarpCtrl = WARP_CTRL__BASIC;
    ResetWarpSetting();
    ResetBlendConfig();
//	m_stWarpConfig.stOsd.bBlendEnable = FALSE;  //add for Barco Ptool to enable/disable blending
//	m_stWarpConfig.stOsd.nBlendWidthL = 0;
//	m_stWarpConfig.stOsd.nBlendWidthR = 0;
//	m_stWarpConfig.stOsd.nBlendWidthT = 0;
//	m_stWarpConfig.stOsd.nBlendWidthB = 0;
//	m_stWarpConfig.stOsd.nBlendOffsetL = 0;  //add for Barco Ptool
//	m_stWarpConfig.stOsd.nBlendOffsetR = 0;  //add for Barco Ptool
//	m_stWarpConfig.stOsd.nBlendOffsetT = 0;  //add for Barco Ptool
//	m_stWarpConfig.stOsd.nBlendOffsetB = 0;  //add for Barco Ptool
//	m_stWarpConfig.stOsd.nOverlapGridNum = OVERLAP_GRID_NUM_DEFAULT;
//	m_stWarpConfig.stOsd.nBlendGamma = BLEND_GAMMA_DEFAULT;
#ifdef SUPPORT_WARP_CONTROL_PC
	m_stWarpConfig.stPc.nWarpLutH = WARP_SHARPNESS_DEFAULT;
	m_stWarpConfig.stPc.nWarpLutV = WARP_SHARPNESS_DEFAULT;
	m_stWarpConfig.stPc.eBlendMode = BLEND_MODE__EDGE;
	m_stWarpConfig.stPc.fBlendGamma = 2.2;
	m_stWarpConfig.stPc.bIsDotByDotAreaValid = FALSE;
	m_stWarpConfig.stPc.bBiasOn = FALSE;
	m_stWarpConfig.stPc.bIsBiasAreaValid = FALSE;
#endif
	//init control point
	InitControlPoint();
	if(SaveWarpConfig()!=FLASH_ACCESS__PASS)
		LOG_MSG(db_HAL_WARPING,"save warp config fail\r\n");

	//calc warping table
	CalcOsdWarpTable();
	//init grid color
	InitGridColor();

	//Write init. warp table for Flash
	utilWarp_WriteDefaultWarpTable();
	utilWarp_WriteDefaultWarpLut();

	//reset other setting
	m_nSelCtlPointX = 0;
	m_nSelCtlPointY = 0;
	m_bOsdWarpParamChanged = FALSE;
    #endif
}

//return true: changed
//return false: not changed
BOOL utilWarp_SetWarpCtrl(eWARP_CTRL eWarpCtrl)
{
    LOG_MSG(db_HAL_WARPING, "utilWarp_SetWarpCtrl %d\n", eWarpCtrl);

	if( (eWarpCtrl >= WARP_CTRL__INVALID) ||
		(eWarpCtrl == m_stWarpConfig.eWarpCtrl))
		return FALSE;

	m_stWarpConfig.eWarpCtrl = eWarpCtrl;

	if(eWarpCtrl != WARP_CTRL__ADVANCED)		//G100_Doulas_0027 WARP_CTRL__BASIC not doing
		return FALSE;

	if(eWarpCtrl == WARP_CTRL__ADVANCED) //A35G2_BRC_Casper_0046
	{
		m_stWarpConfig.stOsd.bBlendEnable = TRUE;  //enable when OSD warping enable
	}

	ApplyWarp(APPLY_WARP__BY_CONFIG);

#if (ADVANCED_BLEND == TRUE)
	ApplyBlend(APPLY_BLEND__BY_CONFIG);
#endif

#if (ADVANCED_BLACK_LEVEL == TRUE)
	ApplyBlackLevel(APPLY_BLACKLEVEL__BY_CONFIG);  //A65_OPTOMA_Doulas_0020
#endif

	return TRUE;
}

eWARP_CTRL utilWarp_GetWarpCtrl(void)
{
	return m_stWarpConfig.eWarpCtrl;
}


//return True : Changed
//return False : Not changed
BOOL utilWarp_SetWarpPoint(eWARP_POINT eWarpPoint) //A35G2_BRC_Casper_0046
{
	if( (eWarpPoint >= WARP_POINT__INVALID) ||
		(eWarpPoint == m_stWarpConfig.stOsd.eWarpPoint))
		return FALSE;

    LOG_MSG(db_HAL_WARPING, "%s (eWarpPoint %d)\r\n", __FUNCTION__, eWarpPoint);

	m_stWarpConfig.stOsd.eWarpPoint = eWarpPoint;
    //  if(m_stWarpConfig.stOsd.eWarpPoint==WARP_POINT__2x2)
    //      m_stWarpConfig.stOsd.bWarpInnerOn = FALSE;

	ApplyWarp(APPLY_WARP__RE_CALC);
	m_nSelCtlPointX = 0;
	m_nSelCtlPointY = 0;
	return TRUE;
}

eWARP_POINT utilWarp_GetWarpPoint(void)
{
	return m_stWarpConfig.stOsd.eWarpPoint;
}

//return True : Changed
//return False : Not changed
BOOL utilWarp_SetWarpInner(BOOL bWarpInnerOn) //A35G2_BRC_Casper_0046
{
	if(bWarpInnerOn == m_stWarpConfig.stOsd.bWarpInnerOn)
		return FALSE;

	//control point 2x2 only support warp inner disable
    //  if((bWarpInnerOn == TRUE) &&
    //      (m_stWarpConfig.stOsd.eWarpPoint == WARP_POINT__2x2))
    //      return FALSE;

	m_stWarpConfig.stOsd.bWarpInnerOn = bWarpInnerOn;
	ApplyWarp(APPLY_WARP__RE_CALC);
	m_nSelCtlPointX = 0;
	m_nSelCtlPointY = 0;

	return TRUE;
}

BOOL utilWarp_GetWarpInner(void)
{
	return m_stWarpConfig.stOsd.bWarpInnerOn;
}

//return WARP_LIMITATION__CHANGED & WARP_LIMITATION__EXEC_FAIL : Changed
//return WARP_LIMITATION__NO_CHANGE : Not changed
eWARP_LIMITATION utilWarp_SetWarpLimit(BOOL bWarpLimit) //A35G2_BRC_Casper_0047
{
	if(bWarpLimit == m_stWarpConfig.stOsd.bWarpLimit)
		return WARP_LIMITATION__NO_CHANGE;

	m_stWarpConfig.stOsd.bWarpLimit = bWarpLimit;
	if((bWarpLimit == TRUE) &&
		(CalcOsdWarpTable() != E_WpNoErr))
		return WARP_LIMITATION__EXEC_FAIL;

	return WARP_LIMITATION__CHANGED;
}

BOOL utilWarp_GetWarpLimit(void) //A35G2_BRC_Casper_0047
{
	return m_stWarpConfig.stOsd.bWarpLimit;
}

//return True : Changed
//return False : Not changed
BOOL utilWarp_SetOsdWarpSharpness(uint8 nIdx)
{
	if( (nIdx > WARP_SHARPNESS_MAX) ||
		(nIdx == m_stWarpConfig.stOsd.nWarpSharpness))
		return FALSE;

	m_stWarpConfig.stOsd.nWarpSharpness = nIdx;
	WriteWarpLut(nIdx, nIdx);
	return TRUE;
}

uint8 utilWarp_GetOsdWarpSharpness(void)
{
	return m_stWarpConfig.stOsd.nWarpSharpness;
}

//return True : Changed
//return False : Not changed
BOOL utilWarp_SetOverlapGridNum(uint8 nIdx)
{
	if( (nIdx > OVERLAP_GRID_NUM_MAX) ||
		(nIdx == m_stWarpConfig.stOsd.nOverlapGridNum))
		return FALSE;

	m_stWarpConfig.stOsd.nOverlapGridNum = nIdx;
	return TRUE;
}

uint8 utilWarp_GetOverlapGridNum(void)
{
	return m_stWarpConfig.stOsd.nOverlapGridNum;
}

void utilWarp_SetOutputTiming(eRES_ID eCurrentResId)  //G100_Doulas_0065
{
    #if 1  //A65_OPTOMA_Doulas_0020
	SetOutputTiming(eCurrentResId);
	ApplyWarp(APPLY_WARP__RE_CALC);
	ApplyBlend(APPLY_BLEND__BY_CONFIG);
	ApplyBlackLevel(APPLY_BLACKLEVEL__BY_CONFIG);
    #else
	uint16 nReg16;

	m_eCurrentResId = eCurrentResId;    //G100_Doulas_0065
	if(m_eCurrentResId!=NATIVE_RES_ID)  //G100_Doulas_0065
		m_bOutputNativeRes = FALSE;
	else
		m_bOutputNativeRes = TRUE;

	//Init system parameter related to output resolution
	InitOutputTimingParam();

	//Line Feed
	dvC789_WriteToBuffer(B1_OSDMWI, m_nLineFeed);
	dvC789_WriteToBuffer(B0_BBWMWI, m_nLineFeed);
	dvC789_WriteToBuffer(B1_OSDACTHW, PS_PANEL_ACT_HW);   //A65_OPTOMA_Simon_0003
	dvC789_WriteToBuffer(B1_OSDACTVW, PS_PANEL_ACT_VW);   //A65_OPTOMA_Simon_0003

	dvC789_Buffer_Flush();
    #endif
}

void utilWarp_SelectControlPoint(eDIR eDir)
{
	uint8 cNewX = m_nSelCtlPointX;
	uint8 cNewY = m_nSelCtlPointY;

	do {
		switch (eDir) {
			default:
			case DIR__UP:
				cNewY = (cNewY<=0) ? 0 : cNewY - 1;
				break;
			case DIR__DOWN:
				cNewY = (cNewY>=(DEF_NUM_CUR_MAX_V-1)) ? (DEF_NUM_CUR_MAX_V-1) : cNewY + 1;    //G100_Doulas_0059
				break;
			case DIR__LEFT:
				cNewX = (cNewX<=0) ? 0 : cNewX - 1;
				break;
			case DIR__RIGHT:
				cNewX = (cNewX>=(DEF_NUM_CUR_MAX_H-1)) ? (DEF_NUM_CUR_MAX_H-1) : cNewX + 1;    //G100_Doulas_0059
				break;
		}
	} while (GV_GRID_EN[cNewX][cNewY] == 0);

	//if chanaged, update pattern
	if (m_nSelCtlPointX != cNewX || m_nSelCtlPointY != cNewY) {
		m_nSelCtlPointX = cNewX;
		m_nSelCtlPointY = cNewY;
		utilWarp_ShowOsdPattern(PAT_TYPE__WARP_SEL_CTRL_POINT);
	}
}

//return True : Changed
//return False : Not changed
BOOL utilWarp_MoveControlPoint(eDIR eDir)
{
	float fNewPos;
	uint8 nMovePixel;
	uint8 err = E_WpNoErr;
	dvC789_WriteToBuffer(BN_RTCT, RTCT_THRU);  		//G100_Doulas_0027
	if (eDir!=m_eMovePrevDir)
	{
		m_eMovePrevDir = eDir;
		m_nMoveContinueCount = 0;
		nMovePixel = 1;
		//dbmsg_ftrace( DBM_ALWAYS, "<<<<<<<<<<change dir=%d, nMovePixel=%d>>>>>>>>>> \r\n", m_eMovePrevDir, nMovePixel);
	}
	else
	{
		if(TimeElapsedMs() < MOVE_CONTINUE_TIMEOUT)
		{
			m_nMoveContinueCount++;
			if(m_nMoveContinueCount<MOVE_CONTINUE_MAX)
				nMovePixel = m_nMoveContinueCount*MOVE_CONTINUE_STEP;
			else
				nMovePixel = MOVE_CONTINUE_MAX*MOVE_CONTINUE_STEP;
			//dbmsg_ftrace( DBM_ALWAYS, "<<<<<<<<<<continue move, nMovePixel=%d>>>>>>>>>> \r\n", nMovePixel);
		}
		else
		{
			m_nMoveContinueCount = 0;
			nMovePixel = 1;
			//dbmsg_ftrace( DBM_ALWAYS, "<<<<<<<<<<normal move, nMovePixel=%d>>>>>>>>>> \r\n", nMovePixel);
		}
	}

	if(eDir==DIR__UP)
	{
		fNewPos = PM_GRID[m_nSelCtlPointX][m_nSelCtlPointY].y-nMovePixel;
		#if defined(CUSTOM_CHRISTIE) || defined(CUSTOM_OPTOMA) //A35G2_BRC_Casper_0137 //A35G2_BRC_Casper_0148
		if(m_nSelCtlPointY==0 && fNewPos<=0.0f)
		{
			if(PM_GRID[m_nSelCtlPointX][m_nSelCtlPointY].y>-0.5f && PM_GRID[m_nSelCtlPointX][m_nSelCtlPointY].y<0.5f)
			{
				//dbmsg_ftrace( DBM_ALWAYS, "<<<<<<<<<<top border return, (%d,%d)>>>>>>>>>> \r\n",m_nSelCtlPointX,m_nSelCtlPointY);
				return FALSE;
			}
			else
			{
				//dbmsg_ftrace( DBM_ALWAYS, "<<<<<<<<<<top border touch, (%d,%d)>>>>>>>>>> \r\n",m_nSelCtlPointX,m_nSelCtlPointY);
				PM_GRID[m_nSelCtlPointX][m_nSelCtlPointY].y = 0.0f;
			}
		}
		else
		#endif
		{
			PM_GRID[m_nSelCtlPointX][m_nSelCtlPointY].y = fNewPos;
		}

	}
	if(eDir==DIR__DOWN)
	{
		fNewPos = PM_GRID[m_nSelCtlPointX][m_nSelCtlPointY].y+nMovePixel;
		#if defined(CUSTOM_CHRISTIE) || defined(CUSTOM_OPTOMA) //A35G2_BRC_Casper_0137 //A35G2_BRC_Casper_0148
		if(m_nSelCtlPointY==(DEF_NUM_CUR_MAX_V-1) && fNewPos>=m_nNativeResV)  //G100_Doulas_0065
		{
			if((PM_GRID[m_nSelCtlPointX][m_nSelCtlPointY].y-m_nNativeResV)>-0.5f && (PM_GRID[m_nSelCtlPointX][m_nSelCtlPointY].y-m_nNativeResV)<0.5f)  //G100_Doulas_0065
			{
				//dbmsg_ftrace( DBM_ALWAYS, "<<<<<<<<<<bottom border return, (%d,%d)>>>>>>>>>> \r\n",m_nSelCtlPointX,m_nSelCtlPointY);
				return FALSE;
			}
			else
			{
				//dbmsg_ftrace( DBM_ALWAYS, "<<<<<<<<<<bottom border touch, (%d,%d)>>>>>>>>>> \r\n",m_nSelCtlPointX,m_nSelCtlPointY);
				PM_GRID[m_nSelCtlPointX][m_nSelCtlPointY].y = m_nNativeResV;  //G100_Doulas_0065
			}
		}
		else
		#endif
		{
			PM_GRID[m_nSelCtlPointX][m_nSelCtlPointY].y = fNewPos;
		}

	}
	if(eDir==DIR__LEFT)
	{
		fNewPos = PM_GRID[m_nSelCtlPointX][m_nSelCtlPointY].x-nMovePixel;
		#if defined(CUSTOM_CHRISTIE) || defined(CUSTOM_OPTOMA) //A35G2_BRC_Casper_0137 //A35G2_BRC_Casper_0148
		if(m_nSelCtlPointX==0 && fNewPos<=0.0f)
		{
			if(PM_GRID[m_nSelCtlPointX][m_nSelCtlPointY].x>-0.5f && PM_GRID[m_nSelCtlPointX][m_nSelCtlPointY].x<0.5f)
			{
				//dbmsg_ftrace( DBM_ALWAYS, "<<<<<<<<<<left border return, (%d,%d)>>>>>>>>>> \r\n",m_nSelCtlPointX,m_nSelCtlPointY);
				return FALSE;
			}
			else
			{
				//dbmsg_ftrace( DBM_ALWAYS, "<<<<<<<<<<left border touch, (%d,%d)>>>>>>>>>> \r\n",m_nSelCtlPointX,m_nSelCtlPointY);
				PM_GRID[m_nSelCtlPointX][m_nSelCtlPointY].x = 0.0f;
			}
		}
		else
		#endif
		{
			PM_GRID[m_nSelCtlPointX][m_nSelCtlPointY].x = fNewPos;
		}

	}
	if(eDir==DIR__RIGHT)
	{
		fNewPos = PM_GRID[m_nSelCtlPointX][m_nSelCtlPointY].x+nMovePixel;
		#if defined(CUSTOM_CHRISTIE) || defined(CUSTOM_OPTOMA) //A35G2_BRC_Casper_0137 //A35G2_BRC_Casper_0148
		if(m_nSelCtlPointX==(DEF_NUM_CUR_MAX_H-1) && fNewPos>=m_nNativeResH)  //G100_Doulas_0065
		{
			if((PM_GRID[m_nSelCtlPointX][m_nSelCtlPointY].x-m_nNativeResH)>-0.5f && (PM_GRID[m_nSelCtlPointX][m_nSelCtlPointY].x-m_nNativeResH)<0.5f)  //G100_Doulas_0065
			{
				//dbmsg_ftrace( DBM_ALWAYS, "<<<<<<<<<<right border return, (%d,%d)>>>>>>>>>> \r\n",m_nSelCtlPointX,m_nSelCtlPointY);
				return FALSE;
			}
			else
			{
				//dbmsg_ftrace( DBM_ALWAYS, "<<<<<<<<<<right border touch, (%d,%d)>>>>>>>>>> \r\n",m_nSelCtlPointX,m_nSelCtlPointY);
				PM_GRID[m_nSelCtlPointX][m_nSelCtlPointY].x = m_nNativeResH;  //G100_Doulas_0065
			}
		}
		else
		#endif
		{
			PM_GRID[m_nSelCtlPointX][m_nSelCtlPointY].x = fNewPos;
		}

	}
	err = CalcOsdWarpTable();
	if( err == E_WpNoErr )
	{
		WriteWarpTable(Def_Wp_Space);
	}
	else
	{
		//make the movement be 1 of next adjustment
		m_eMovePrevDir = DIR__INVALID;
		if(nMovePixel==1)
			return FALSE;
	}

	clock_gettime(CLOCK_MONOTONIC, &m_stStartTime);
	m_bOsdWarpParamChanged = TRUE;
	return TRUE;
}

void utilWarp_WriteDefaultWarpLut(void)
{
	WriteWarpLut(WARP_SHARPNESS_DEFAULT, WARP_SHARPNESS_DEFAULT);
}

void utilWarp_WriteDefaultWarpTable(void)
{
	uint16 nIdxX, nIdxY;
	uint16 nDtX, nDtY;

    dvC789_Write(BN_RTCT, RTCT_THRU);   //A35G2_CDS_Simon_0034

    UINT16 uiDTCT = (UINT16)dvC789_Read(B5_DTCT);
    if(uiDTCT & WARPING_DISTORTION_CORRECTION_TABLE_B)   //A35G2_CDS_Simon_0034
    {
        dvC789_Write(B5_DTCT, DTCT_BASE_ACCESS_ENABLE_16PIEXL_B);
    }
    else
    {
        dvC789_Write(B5_DTCT, DTCT_BASE_ACCESS_ENABLE_16PIEXL_A);
    }

	//RGB common correction; 16-pixel Spacing; Select Table A; Access Table A
	//dvC789_WriteToBuffer(B5_DTCT, 0x0040);

	for(nIdxY = 0 ; nIdxY <= PS_WP_VW_GRD; nIdxY++)
	{
		//Start address per line
		//First access  : Set the low-byte of table address to DTAD[7:0]
		dvC789_WriteToBuffer(B5_DTAD, (nIdxY<<7) & 0xFF);
		//Second access : Set the high-byte of table address to DTAD[7:0]
		dvC789_WriteToBuffer(B5_DTAD, (nIdxY>>1) & 0xFF);

		for(nIdxX = 0; nIdxX <= PS_WP_HW_GRD; nIdxX++)
		{
			//Integer; signed 4bits. Decimal place; 4bits
			nDtX = nIdxX << 8;
			nDtY = nIdxY << 8;
			//First access  : Set the low-byte of X-coordinate to DTDT[7:0]
			dvC789_WriteToBuffer(B5_DTDT, GET_BYTE0(nDtX));
			//Second access : Set the high-byte of X-coordinate to DTDT[7:0]
			dvC789_WriteToBuffer(B5_DTDT, GET_BYTE1(nDtX));
			//Third access  : Set the low-byte of Y-coordinate to DTDT[7:0]
			dvC789_WriteToBuffer(B5_DTDT, GET_BYTE0(nDtY));
			//Fourth access  : Set the high-byte of Y-coordinate to DTDT[7:0]
			dvC789_WriteToBuffer(B5_DTDT, GET_BYTE1(nDtY));
		}
	}

	//dvC789_WriteToBuffer(B5_DTCT, 0x0000);
	dvC789_Buffer_Flush();

    if(uiDTCT & WARPING_DISTORTION_CORRECTION_TABLE_B)   //A35G2_CDS_Simon_0034
    {
        dvC789_Write(B5_DTCT, DTCT_BASE_ACCESS_DISABLE_16PIEXL_B);
    }
    else
    {
        dvC789_Write(B5_DTCT, DTCT_BASE_ACCESS_DISABLE_16PIEXL_A);
    }

}

//return true: changed
//return false: not changed
BOOL utilWarp_SetOsdGridColor(eGRID_COLOR eColor, UINT8 ucRedrawPattern)
{
	if(	(eColor != GRID_COLOR__GREEN &&
		 eColor != GRID_COLOR__MAGENTA &&
		 eColor != GRID_COLOR__CYAN &&
		 eColor != GRID_COLOR__RED) ||
		(eColor >= GRID_COLOR__INVALID) ||
		(eColor == m_stWarpConfig.stOsd.eGridColor))
		return FALSE;

	m_stWarpConfig.stOsd.eGridColor = eColor;
	InitGridColor();

	if(ucRedrawPattern)
	{
	    utilWarp_ShowOsdPattern(PAT_TYPE__WARP_SEL_CTRL_POINT);

	    #if 0
    	if((m_ePatternType == PAT_TYPE__WARP_SEL_CTRL_POINT)
    		|| (m_ePatternType == PAT_TYPE__BLEND_NO_CTRL_POINT)) //A35G2_BRC_Casper_0046
    	{
    		utilWarp_ShowOsdPattern(m_ePatternType);
    	}
    	#endif
	}
	return TRUE;
}

eGRID_COLOR utilWarp_GetOsdGridColor(void)
{
	return m_stWarpConfig.stOsd.eGridColor;
}

//return true: changed
//return false: not changed
BOOL utilWarp_SetOsdBkgColor(eBKG_COLOR eColor, UINT8 ucRedrawPattern)
{
	if( (eColor >= BKG_COLOR__INVALID) ||
		(eColor == m_stWarpConfig.stOsd.eBkgColor))
		return FALSE;

	m_stWarpConfig.stOsd.eBkgColor = eColor;

	if(ucRedrawPattern)
	{
	    utilWarp_ShowOsdPattern(PAT_TYPE__WARP_SEL_CTRL_POINT);

	    #if 0
    	if(m_ePatternType == PAT_TYPE__WARP_SEL_CTRL_POINT) //A35G2_BRC_Casper_0046
    	{
    		utilWarp_ShowOsdPattern(m_ePatternType);
    	}
    	#endif
    }

	return TRUE;
}

eBKG_COLOR utilWarp_GetOsdBkgColor(void)
{
	return m_stWarpConfig.stOsd.eBkgColor;
}


//G100_Tim_0023, add, start //A35G2_CDS_Coda_0029
// ==============================================================================
// Function: OSD_Color_Init
// DESCRIPTION: Initial the OSD color, to fix AF color change
// Params: None
// Returns: None
//
// Modification History
// --------------------
// 2021/06/15, CL modified, add new items
// --------------------
// ==============================================================================
void OSD_Color_Init(void)
{
    InitOsdPatternColor();
    InitGridColor();
}
//G100_Tim_0023, add, end

void utilWarp_ShowOsdPattern(ePAT_TYPE eType)
{
	ePAT_TYPE m_ePatternType_Old = m_ePatternType;	//G100_Doulas_0044 Add
	UINT32 ulColor = 0;			//G100_Doulas_0090
	eAPPLY_BLACKLEVEL eApplyBlackLevel = APPLY_BLACKLEVEL__DISABLE;    //A65_OPTOMA_Doulas_0020  //A35G2_CDS_CL_0002
	eAPPLY_BLEND eApplyBlend = APPLY_BLEND__DISABLE;    //A65_OPTOMA_CL_0016  //A35G2_CDS_CL_0002 //A35G2_Alan_0015

	LOG_MSG(db_HAL_WARPING, "(%s,%d) utilWarp_ShowOsdPattern %d (pre %d) Start\r\n", __FUNCTION__, __LINE__, eType, m_ePatternType_Old);		//A65_OPTOMA_Doulas_0095
	m_ePatternType = eType;
	//dvC789_WriteToBuffer(BN_RTCT, RTCT_THRU);  		//A65_OPTOMA_Doulas_0136//G100_Doulas_0027
	dvC789_Write( BN_RTCT, RTCT_THRU);					//A65_OPTOMA_Doulas_0136
	switch(eType)
	{
		default:
		case PAT_TYPE__OFF:
			//sUtilWarpDemo_Callback.fpGui_OSD_ON_SetCb(ets_ON); //###
            //sUtilWarpDemo_Callback.fpGui_SendUpdateOSDEventCb();	//G100_Doulas_0044
            palDataMgr_UI_EventSend(edcOSD_SHOW, TRUE, NULL);
            palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL);
			//save warp control point/blend width if updated.
			if (m_bOsdWarpParamChanged)
			{
#if (ADVANCED_BLACK_LEVEL == TRUE)
				UpdateBlackLevelNode(&(m_stWarpConfig.stOsd.stBlackLevel));  //A65_OPTOMA_Doulas_0020
#endif
				SaveWarpConfig();
				m_bOsdWarpParamChanged = FALSE;
			}

			eApplyBlackLevel = (m_bRewriteOsdBlackLevelArea) ? APPLY_BLACKLEVEL__BY_CONFIG : APPLY_BLACKLEVEL__ENABLE;    //A65_OPTOMA_Doulas_0020
#ifdef CUSTOM_BARCO   //wait review
            ApplyBlend(APPLY_BLEND__BY_CONFIG);  //A35G2_Wesley_0056 Merge G50 V1.80
#endif

#if (ADVANCED_BLACK_LEVEL == TRUE)
			EnableTestPattern(FALSE, BKG_COLOR__BLACK, APPLY_BLEND__BY_CONFIG, eApplyBlackLevel, m_eOsdPosition);
#else
			ulColor = (UINT8)dvC789_Read(B1_BOTRANS3);												//G100_Doulas_0090
			DrawOsdRect(OSD_LAYER_TYPE__MAIN, 0, 0, PS_PANEL_ACT_HW, PS_PANEL_ACT_VW, ulColor);		//G100_Doulas_0090
			#ifndef CUSTOM_CHRISTIE  //A35G2_Simon_0115
			EnableTestPattern(TRUE, BKG_COLOR__TRANSPARENT, APPLY_BLEND__BY_CONFIG, eApplyBlackLevel, m_eOsdPosition);  //A65_OPTOMA_Doulas_0020 //A35G2_Wesley_0056 Merge G50 V1.80
			#endif
			EnableTestPattern(FALSE, BKG_COLOR__BLACK, APPLY_BLEND__BY_CONFIG, eApplyBlackLevel, m_eOsdPosition);  //A65_OPTOMA_Doulas_0020
#endif
			LOG_MSG(db_HAL_WARPING,"PAT_TYPE__OFF end\r\n");		//A65_OPTOMA_Doulas_0136
			break;

		//This belong to color palette set COLOR_PALETTE__OSD
		case PAT_TYPE__WARP_SEL_CTRL_POINT:
			if(m_ePatternType_Old == PAT_TYPE__OFF)		//G100_Doulas_0044 Add
			{
				//sUtilWarpDemo_Callback.fpGui_OSD_ON_SetCb(ets_OFF); //###
            	//sUtilWarpDemo_Callback.fpGui_SendUpdateOSDEventCb();
                palDataMgr_UI_EventSend(edcOSD_SHOW, FALSE, NULL);
                palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL);
			}
			utilWarp_SetColorPaletteGroupIndex(COLOR_PALETTE__OSD); //A65_OPTOMA_CL_0007

            // G50_Keven_0003, add, start //A35G2_Wesley_0093 >>>
            if(m_stWarpConfig.stOsd.bDrawOverlapGridOnWarpPattern)
            {
                DrawOsdWarpGridLinesPattern();
            }
            else
            {
                DrawOsdWarpGridLinesPatternPtoolset();
            }
			DrawOsdWarpGridPointsPattern();
			eApplyBlackLevel = (halWarping_TwistLinkFlag_Get() == TRUE) ? APPLY_BLACKLEVEL__BY_CONFIG : APPLY_BLACKLEVEL__DISABLE;
			EnableTestPattern(TRUE, m_stWarpConfig.stOsd.eBkgColor, APPLY_BLEND__BY_CONFIG, eApplyBlackLevel,OSD_POS__BEFORE_WARP);
            break;
            // G50_Keven_0003, add, end //A35G2_Wesley_0093 <<<

		case PAT_TYPE__WARP_MOV_CTRL_POINT:
            // G50_Keven_0003 //A35G2_Wesley_0093 >>>
			if(m_stWarpConfig.stOsd.bDrawOverlapGridOnWarpPattern)
            {
                DrawOsdWarpGridLinesPattern();
            }
            else
            {
                DrawOsdWarpGridLinesPatternPtoolset();
            }
			DrawOsdWarpGridPointsPattern();
			eApplyBlackLevel = (halWarping_TwistLinkFlag_Get() == TRUE) ? APPLY_BLACKLEVEL__BY_CONFIG : APPLY_BLACKLEVEL__DISABLE;
			EnableTestPattern(TRUE, m_stWarpConfig.stOsd.eBkgColor, APPLY_BLEND__BY_CONFIG, eApplyBlackLevel,OSD_POS__BEFORE_WARP);
            // G50_Keven_0003	 //A35G2_Wesley_0093 <<<
            break;

		case PAT_TYPE__WARP_NO_CTRL_POINT:
			DrawOsdWarpGridLinesPattern();  // G50_Keven_0003, follow Tim's modification //A35G2_Wesley_0093
			EnableTestPattern(TRUE, m_stWarpConfig.stOsd.eBkgColor, APPLY_BLEND__DISABLE, APPLY_BLACKLEVEL__DISABLE, OSD_POS__BEFORE_WARP);  //A65_OPTOMA_Doulas_0020
			break;

		//This belong to color palette set COLOR_PALETTE__OSD
		case PAT_TYPE__SEL_BLEND_WIDTH:
			if(m_ePatternType_Old == PAT_TYPE__OFF)		//G100_Doulas_0044 Add
			{
				//sUtilWarpDemo_Callback.fpGui_OSD_ON_SetCb(ets_OFF); //###
            	//sUtilWarpDemo_Callback.fpGui_SendUpdateOSDEventCb();
                palDataMgr_UI_EventSend(edcOSD_SHOW, FALSE, NULL);
                palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL);
			}
			utilWarp_SetColorPaletteGroupIndex(COLOR_PALETTE__OSD); //A65_OPTOMA_CL_0007

		case PAT_TYPE__ADJ_BLEND_WIDTH:
		case PAT_TYPE__BLEND_NO_CTRL_POINT: //A35G2_BRC_Casper_0046
			DrawOsdBlendWidthPattern();
			EnableTestPattern(TRUE, m_stWarpConfig.stOsd.eBkgColor, APPLY_BLEND__BY_CONFIG, APPLY_BLACKLEVEL__DISABLE, OSD_POS__BEFORE_WARP);  //A65_OPTOMA_Doulas_0020 // G50_Keven_0003 //A35G2_Wesley_0093
			break;

		//This belong to color palette set COLOR_PALETTE__OSD
		case PAT_TYPE__BLACKLEVEL_SEL_POINT:  //A65_OPTOMA_Doulas_0020 // A35G2_CDS_Coda_0028
		//case PAT_TYPE__BLACKLEVEL_MOV_POINT:	//A65_OPTOMA_Doulas_0163 remove
		case PAT_TYPE__BLACKLEVEL_ADD_POINT:
		case PAT_TYPE__BLACKLEVEL_DEL_POINT:
			utilWarp_SetColorPaletteGroupIndex(COLOR_PALETTE__OSD); //A65_OPTOMA_CL_0007
		case PAT_TYPE__BLACKLEVEL_MOV_POINT:	//A65_OPTOMA_Doulas_0163
			#if 0 //A65_OPTOMA_Doulas_0023 remove
			if(m_ePatternType_Old == PAT_TYPE__OFF)		//A65_OPTOMA_Doulas_0020 Add
			{
				sUtilWarpDemo_Callback.fpGui_OSD_ON_SetCb(ets_OFF);
            	sUtilWarpDemo_Callback.fpGui_SendUpdateOSDEventCb();
			}
			#endif
			DrawOsdBlackLevelAreaPattern(m_eAreaSelection);
			EnableTestPattern(TRUE, BKG_COLOR__BLACK, APPLY_BLEND__DISABLE, APPLY_BLACKLEVEL__DISABLE, OSD_POS__AFTER_WARP);
			break;

		//This belong to color palette set COLOR_PALETTE__OSD
		case PAT_TYPE__BLACKLEVEL_PREVIEW:   //A65_OPTOMA_Doulas_0020
			utilWarp_SetColorPaletteGroupIndex(COLOR_PALETTE__OSD); //A65_OPTOMA_CL_0007
			#if 0 //A65_OPTOMA_Doulas_0023 remove
			if(m_ePatternType_Old == PAT_TYPE__OFF)		//A65_OPTOMA_Doulas_0020 Add
			{
				sUtilWarpDemo_Callback.fpGui_OSD_ON_SetCb(ets_OFF);
            	sUtilWarpDemo_Callback.fpGui_SendUpdateOSDEventCb();
			}
			#endif
			DrawOsdBlackLevelPreviewPattern();
			eApplyBlackLevel = (m_bRewriteOsdBlackLevelArea) ? APPLY_BLACKLEVEL__BY_CONFIG : APPLY_BLACKLEVEL__ENABLE;
			EnableTestPattern(TRUE, BKG_COLOR__TRANSPARENT, APPLY_BLEND__DISABLE, eApplyBlackLevel, OSD_POS__AFTER_WARP);
			break;

//		case PAT_TYPE__EMPTY: //A35G2_BRC_Casper_0046
//			//DrawOsdRect(OSD_LAYER_TYPE__MAIN, 0, 0, PS_PANEL_ACT_HW, PS_PANEL_ACT_VW, COLOR_IDX__BLACK);
//			EnableTestPattern(TRUE, BKG_COLOR__BLACK, APPLY_BLEND__DISABLE);
//			break;

#if (ENABLE_COLOR_UNIFORMITY == TRUE) //G100_Tim_0012, add, start

        //This belong to color palette set COLOR_PALETTE__OSD
        case PAT_TYPE__TESTPATTERN_WHITE:
            if(m_ePatternType_Old == PAT_TYPE__OFF)
            {
                //sUtilWarpDemo_Callback.fpGui_OSD_ON_SetCb(ets_OFF); //###
                //sUtilWarpDemo_Callback.fpGui_SendUpdateOSDEventCb();
            	palDataMgr_UI_EventSend(edcOSD_SHOW, FALSE, NULL);
                palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL);
            }
            utilWarp_SetColorPaletteGroupIndex(COLOR_PALETTE__OSD); //A65_OPTOMA_CL_0007
            DrawOsdRect(OSD_LAYER_TYPE__MAIN, 0, 0, PS_PANEL_ACT_HW, PS_PANEL_ACT_VW, COLOR_IDX__WHITE);
            //EnableTestPattern(TRUE, BKG_COLOR__BLACK, APPLY_BLEND__DISABLE, APPLY_BLACKLEVEL__DISABLE, OSD_POS__AFTER_WARP);  //CL
            //A65_OPTOMA_CL_0016
#ifndef CUSTOM_CHRISTIE  //A35G2_CDS_CL_0002//A35G2_Alan_0015
            eApplyBlackLevel = (halWarping_TwistLinkFlag_Get() == TRUE) ? APPLY_BLACKLEVEL__BY_CONFIG : APPLY_BLACKLEVEL__DISABLE;
            eApplyBlend = (halWarping_TwistLinkFlag_Get() == TRUE) ? APPLY_BLEND__BY_CONFIG : APPLY_BLEND__DISABLE;
#endif
            EnableTestPattern(TRUE, BKG_COLOR__BLACK, eApplyBlend, eApplyBlackLevel, OSD_POS__AFTER_WARP);
            break;

        //This belong to color palette set COLOR_PALETTE__OSD
        case PAT_TYPE__TESTPATTERN_BLACK:
            if(m_ePatternType_Old == PAT_TYPE__OFF)
            {
                //sUtilWarpDemo_Callback.fpGui_OSD_ON_SetCb(ets_OFF); //###
                //sUtilWarpDemo_Callback.fpGui_SendUpdateOSDEventCb();
            	palDataMgr_UI_EventSend(edcOSD_SHOW, FALSE, NULL);
                palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL);
            }
            utilWarp_SetColorPaletteGroupIndex(COLOR_PALETTE__OSD); //A65_OPTOMA_CL_0007
            DrawOsdRect(OSD_LAYER_TYPE__MAIN, 0, 0, PS_PANEL_ACT_HW, PS_PANEL_ACT_VW, COLOR_IDX__BLACK);
            //EnableTestPattern(TRUE, BKG_COLOR__BLACK, APPLY_BLEND__DISABLE, APPLY_BLACKLEVEL__DISABLE, OSD_POS__AFTER_WARP);  //CL
            //A65_OPTOMA_CL_0016
            #ifndef CUSTOM_CHRISTIE  //A35G2_CDS_CL_0002//A35G2_Alan_0015
            eApplyBlackLevel = (halWarping_TwistLinkFlag_Get() == TRUE) ? APPLY_BLACKLEVEL__BY_CONFIG : APPLY_BLACKLEVEL__DISABLE;
            eApplyBlend = (halWarping_TwistLinkFlag_Get() == TRUE) ? APPLY_BLEND__BY_CONFIG : APPLY_BLEND__DISABLE;
            #endif
            EnableTestPattern(TRUE, BKG_COLOR__BLACK, eApplyBlend, eApplyBlackLevel, OSD_POS__AFTER_WARP);
            break;

        //This belong to color palette set COLOR_PALETTE__OSD
        case PAT_TYPE__TESTPATTERN_GRID:
            if(m_ePatternType_Old == PAT_TYPE__OFF)
            {
                //sUtilWarpDemo_Callback.fpGui_OSD_ON_SetCb(ets_OFF); //###
                //sUtilWarpDemo_Callback.fpGui_SendUpdateOSDEventCb();
            	palDataMgr_UI_EventSend(edcOSD_SHOW, FALSE, NULL);
                palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL);
            }
            utilWarp_SetColorPaletteGroupIndex(COLOR_PALETTE__OSD); //A65_OPTOMA_CL_0007
            DrawOsdAcuGridPattern();

            //A65_OPTOMA_CL_0016
            #ifndef CUSTOM_CHRISTIE  //A35G2_CDS_CL_0002//A35G2_Alan_0015
            eApplyBlackLevel = (halWarping_TwistLinkFlag_Get() == TRUE) ? APPLY_BLACKLEVEL__BY_CONFIG : APPLY_BLACKLEVEL__DISABLE;
            eApplyBlend = (halWarping_TwistLinkFlag_Get() == TRUE) ? APPLY_BLEND__BY_CONFIG : APPLY_BLEND__DISABLE;
            #endif
            EnableTestPattern(TRUE, BKG_COLOR__BLACK, eApplyBlend, eApplyBlackLevel, OSD_POS__AFTER_WARP);
            break;

        //This belong to color palette set COLOR_PALETTE__OSD
        case PAT_TYPE__TESTPATTERN_GRIDCENTER:
            if(m_ePatternType_Old == PAT_TYPE__OFF)
            {
                //sUtilWarpDemo_Callback.fpGui_OSD_ON_SetCb(ets_OFF); //###
                //sUtilWarpDemo_Callback.fpGui_SendUpdateOSDEventCb();
            	palDataMgr_UI_EventSend(edcOSD_SHOW, FALSE, NULL);
                palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL);
            }
            utilWarp_SetColorPaletteGroupIndex(COLOR_PALETTE__OSD); //A65_OPTOMA_CL_0007
            DrawOsdAcuCenterGridPattern();
    		//A65_OPTOMA_CL_0016
            #ifndef CUSTOM_CHRISTIE  //A35G2_CDS_CL_0002//A35G2_Alan_0015
            eApplyBlackLevel = (halWarping_TwistLinkFlag_Get() == TRUE) ? APPLY_BLACKLEVEL__BY_CONFIG : APPLY_BLACKLEVEL__DISABLE;
            eApplyBlend = (halWarping_TwistLinkFlag_Get() == TRUE) ? APPLY_BLEND__BY_CONFIG : APPLY_BLEND__DISABLE;
            #endif
            EnableTestPattern(TRUE, BKG_COLOR__BLACK, eApplyBlend, eApplyBlackLevel, OSD_POS__AFTER_WARP);
            break;

        //This belong to color palette set COLOR_PALETTE__OSD
        case PAT_TYPE__TESTPATTERN_BLACK_BORDER:
            if(m_ePatternType_Old == PAT_TYPE__OFF)
            {
                //sUtilWarpDemo_Callback.fpGui_OSD_ON_SetCb(ets_OFF); //###
                //sUtilWarpDemo_Callback.fpGui_SendUpdateOSDEventCb();
            	palDataMgr_UI_EventSend(edcOSD_SHOW, FALSE, NULL);
                palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL);
            }
            utilWarp_SetColorPaletteGroupIndex(COLOR_PALETTE__OSD); //A65_OPTOMA_CL_0007
            DrawOsdRect(OSD_LAYER_TYPE__MAIN, 0, 0, PS_PANEL_ACT_HW, PS_PANEL_ACT_VW, COLOR_IDX__WHITE);
            DrawOsdAcuBlackBorderPattern();
            //A65_OPTOMA_CL_0016
            #ifndef CUSTOM_CHRISTIE  //A35G2_CDS_CL_0002//A35G2_Alan_0015
            eApplyBlackLevel = (halWarping_TwistLinkFlag_Get() == TRUE) ? APPLY_BLACKLEVEL__BY_CONFIG : APPLY_BLACKLEVEL__DISABLE;
            eApplyBlend = (halWarping_TwistLinkFlag_Get() == TRUE) ? APPLY_BLEND__BY_CONFIG : APPLY_BLEND__DISABLE;
            #endif
            EnableTestPattern(TRUE, BKG_COLOR__BLACK, eApplyBlend, eApplyBlackLevel, OSD_POS__AFTER_WARP);
            break;

        //A65_OPTOMA_CL_0001 add for CU multiple selection
        //This belong to color palette set COLOR_PALETTE__OSD
        case PAT_TYPE__TESTPATTERN_BLACK_BORDER_MULTIPLESELECT:
            if(m_ePatternType_Old == PAT_TYPE__OFF)
            {
                //sUtilWarpDemo_Callback.fpGui_OSD_ON_SetCb(ets_OFF); //###
                //sUtilWarpDemo_Callback.fpGui_SendUpdateOSDEventCb();
            	palDataMgr_UI_EventSend(edcOSD_SHOW, FALSE, NULL);
                palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL);
            }
            utilWarp_SetColorPaletteGroupIndex(COLOR_PALETTE__OSD); //A65_OPTOMA_CL_0007
            DrawOsdAcuBlackBorderMultipleSelectPattern();
    		//A65_OPTOMA_CL_0016
    		#ifndef CUSTOM_CHRISTIE  //A35G2_CDS_CL_0002//A35G2_Alan_0015
    		eApplyBlackLevel = (halWarping_TwistLinkFlag_Get() == TRUE) ? APPLY_BLACKLEVEL__BY_CONFIG : APPLY_BLACKLEVEL__DISABLE;
    		eApplyBlend = (halWarping_TwistLinkFlag_Get() == TRUE) ? APPLY_BLEND__BY_CONFIG : APPLY_BLEND__DISABLE;
    		#endif
            EnableTestPattern(TRUE, BKG_COLOR__BLACK, eApplyBlend, eApplyBlackLevel, OSD_POS__AFTER_WARP);
            break;

#endif
        //A65_OPTOMA_CL_0002
        //This belong to color palette set COLOR_PALETTE__OSD
        case PAT_TYPE__DRAW_SOLID_COLOR:
            if(m_ePatternType_Old == PAT_TYPE__OFF)
            {
                //sUtilWarpDemo_Callback.fpGui_OSD_ON_SetCb(ets_OFF); //###
                //sUtilWarpDemo_Callback.fpGui_SendUpdateOSDEventCb();
            	palDataMgr_UI_EventSend(edcOSD_SHOW, FALSE, NULL);
                palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL);
            }
            utilWarp_SetColorPaletteGroupIndex(COLOR_PALETTE__OSD); //A65_OPTOMA_CL_0007
            DrawOsdRect(OSD_LAYER_TYPE__MAIN, 0, 0, PS_PANEL_ACT_HW, PS_PANEL_ACT_VW, COLOR_IDX__USER_DEFINE);
		//A65_OPTOMA_CL_0016
		#ifndef CUSTOM_CHRISTIE  //A35G2_CDS_CL_0002//A35G2_Alan_0015
		eApplyBlackLevel = (halWarping_TwistLinkFlag_Get() == TRUE) ? APPLY_BLACKLEVEL__BY_CONFIG : APPLY_BLACKLEVEL__DISABLE;
		eApplyBlend = (halWarping_TwistLinkFlag_Get() == TRUE) ? APPLY_BLEND__BY_CONFIG : APPLY_BLEND__DISABLE;
		#endif
            EnableTestPattern(TRUE, BKG_COLOR__BLACK, eApplyBlend, eApplyBlackLevel, OSD_POS__AFTER_WARP);
            break;
        //A65_OPTOMA_CL_0006
        //This belong to color palette set COLOR_PALETTE__AP
        case PAT_TYPE__DRAW_RECT:
            utilWarp_SetColorPaletteGroupIndex(COLOR_PALETTE__AP); //A65_OPTOMA_CL_0007
            break;
        //A65_OPTOMA_CL_0006
        //This belong to color palette set COLOR_PALETTE__AP
        case PAT_TYPE__DRAW_RECTS:
            utilWarp_SetColorPaletteGroupIndex(COLOR_PALETTE__AP); //A65_OPTOMA_CL_0007
            break;
        //A65_OPTOMA_CL_0006
        //This belong to color palette set COLOR_PALETTE__AP
        case PAT_TYPE__DRAW_CIRCLE:
            utilWarp_SetColorPaletteGroupIndex(COLOR_PALETTE__AP); //A65_OPTOMA_CL_0007
            break;
        //A65_OPTOMA_CL_0006
        //This belong to color palette set COLOR_PALETTE__AP
        case PAT_TYPE__DRAW_CIRCLE_GRID:
            utilWarp_SetColorPaletteGroupIndex(COLOR_PALETTE__AP); //A65_OPTOMA_CL_0007
            DrawApCirclePattern();
		//A65_OPTOMA_CL_0016
		#ifndef CUSTOM_CHRISTIE  //A35G2_CDS_CL_0002//A35G2_Alan_0015
		eApplyBlackLevel = (halWarping_TwistLinkFlag_Get() == TRUE) ? APPLY_BLACKLEVEL__BY_CONFIG : APPLY_BLACKLEVEL__DISABLE;
		eApplyBlend = (halWarping_TwistLinkFlag_Get() == TRUE) ? APPLY_BLEND__BY_CONFIG : APPLY_BLEND__DISABLE;
		#endif
            EnableTestPattern(TRUE, BKG_COLOR__BLACK, eApplyBlend, eApplyBlackLevel, OSD_POS__AFTER_WARP);
            break;
        //A65_OPTOMA_CL_0006
        //This belong to color palette set COLOR_PALETTE__AP
        case PAT_TYPE__DRAW_LINE:
            utilWarp_SetColorPaletteGroupIndex(COLOR_PALETTE__AP); //A65_OPTOMA_CL_0007
            break;
        //A65_OPTOMA_CL_0006
        //This belong to color palette set COLOR_PALETTE__AP
        case PAT_TYPE__DRAW_LINES:
            utilWarp_SetColorPaletteGroupIndex(COLOR_PALETTE__AP); //A65_OPTOMA_CL_0007
            break;
        //A65_OPTOMA_CL_0006  //A65_OPTOMA_CL_0015
        //This belong to color palette set COLOR_PALETTE__OSD
        case PAT_TYPE__DRAW_CHECKBOARD:
            utilWarp_SetColorPaletteGroupIndex(COLOR_PALETTE__OSD); //A65_OPTOMA_CL_0007  //A65_OPTOMA_CL_0015
            DrawCheckerBoardPattern();  //A65_OPTOMA_CL_0015
            if(m_psChecksInfo->ucLayerMode == eWDT_AFTER_WARP)   //A65_OPTOMA_CL_0015
                {
		//A65_OPTOMA_CL_0016
		#ifndef CUSTOM_CHRISTIE  //A35G2_CDS_CL_0002//A35G2_Alan_0015
		eApplyBlackLevel = (halWarping_TwistLinkFlag_Get() == TRUE) ? APPLY_BLACKLEVEL__BY_CONFIG : APPLY_BLACKLEVEL__DISABLE;
		eApplyBlend = (halWarping_TwistLinkFlag_Get() == TRUE) ? APPLY_BLEND__BY_CONFIG : APPLY_BLEND__DISABLE;
		#endif
            EnableTestPattern(TRUE, BKG_COLOR__BLACK, eApplyBlend, eApplyBlackLevel, OSD_POS__AFTER_WARP);
                }
            else
                {
		//A65_OPTOMA_CL_0016
		#ifndef CUSTOM_CHRISTIE  //A35G2_CDS_CL_0002//A35G2_Alan_0015
		eApplyBlackLevel = (halWarping_TwistLinkFlag_Get() == TRUE) ? APPLY_BLACKLEVEL__BY_CONFIG : APPLY_BLACKLEVEL__DISABLE;
		eApplyBlend = (halWarping_TwistLinkFlag_Get() == TRUE) ? APPLY_BLEND__BY_CONFIG : APPLY_BLEND__DISABLE;
		#endif
            EnableTestPattern(TRUE, BKG_COLOR__BLACK, eApplyBlend, eApplyBlackLevel, OSD_POS__BEFORE_WARP);
                }
            break;
        //A65_OPTOMA_CL_0006
        //This belong to color palette set COLOR_PALETTE__AP
        case PAT_TYPE__DRAW_CHECKBOARD_TWIST:
            utilWarp_SetColorPaletteGroupIndex(COLOR_PALETTE__AP); //A65_OPTOMA_CL_0007
            break;
        //A65_OPTOMA_CL_0006
        //This belong to color palette set COLOR_PALETTE__PNG
        case PAT_TYPE__DRAW_PNG:
            utilWarp_SetColorPaletteGroupIndex(COLOR_PALETTE__PNG); //A65_OPTOMA_CL_0007
            break;
        //A65_OPTOMA_CL_0006
        //This belong to color palette set COLOR_PALETTE__SPRITE
        case PAT_TYPE__DRAW_SPRITE:
            utilWarp_SetColorPaletteGroupIndex(COLOR_PALETTE__SPRITE); //A65_OPTOMA_CL_0007
            break;
        //A65_OPTOMA_CL_0006
        //This belong to color palette set COLOR_PALETTE__SPRITE
        case PAT_TYPE__DRAW_SPRITES:
            utilWarp_SetColorPaletteGroupIndex(COLOR_PALETTE__SPRITE); //A65_OPTOMA_CL_0007
            break;
	}

	LOG_MSG(db_HAL_WARPING, "utilWarp_ShowOsdPattern %d End\r\n", eType);		//A65_OPTOMA_Doulas_0095

}

ePAT_TYPE utilWarp_GetOsdPatternType(void)
{
	return m_ePatternType;
}

void utilWarp_SetOsdPatternType(ePAT_TYPE eType)
{
	m_ePatternType = eType;
}


void utilWarp_HideOsdPattern(BOOL bHide)			//A65_OPTOMA_Doulas_0211 Add
{
	uint8 nOSDMODE;

	LOG_MSG(db_HAL_WARPING,"utilWarp_HideOsdPattern %d\r\n",bHide);		//A35G2_Owen_0002 //A65_OPTOMA_Doulas_0239 debug
    nOSDMODE = dvC789_Read(B1_OSDMODE);
    if(bHide)
    {
        nOSDMODE &= 0xfd;
    }
    else if(m_ePatternType != PAT_TYPE__OFF)
    {
        nOSDMODE |= 0x02;
    }

    dvC789_Write(B1_OSDMODE, nOSDMODE);
}

void utilWarp_SelectOsdBlendWidth(eDIR eDir)
{
	if(eDir >= DIR__INVALID)
		eDir = DIR__UP;

	//if chanaged, update pattern
	if(eDir != m_eBlendWidthSel)
	{
		m_eBlendWidthSel = eDir;
		utilWarp_ShowOsdPattern(PAT_TYPE__SEL_BLEND_WIDTH);
	}
}

//return True : Changed
//return False : Not changed
#ifdef CUSTOM_OPTOMA
BOOL utilWarp_AdjustOsdBlendWidth(eADJ eAdj)
{
	uint16 nVal = 0;
	uint16 nMin, nMax;
	BOOL bChanged = FALSE;
	uint16 nMovePixel;  //A65_OPTOMA_Doulas_0020

	if(eAdj >= ADJ__INVALID)
		return FALSE;
	dvC789_WriteToBuffer(BN_RTCT, RTCT_THRU);  		//G100_Doulas_0027

	if (eAdj!=m_eAdjPrevAction)    //A65_OPTOMA_Doulas_0020
	{
		m_eAdjPrevAction = eAdj;
		m_nMoveContinueCount = 0;
		nMovePixel = BLEND_WIDTH_ADJUST_STEP;
	}
	else
	{
		if(TimeElapsedMs() < MOVE_CONTINUE_TIMEOUT)
		{
			m_nMoveContinueCount++;
			if(m_nMoveContinueCount<(MOVE_CONTINUE_MAX/2))
				nMovePixel = BLEND_WIDTH_ADJUST_STEP;
			else
				nMovePixel = 5*BLEND_WIDTH_ADJUST_STEP;
		}
		else
		{
			m_nMoveContinueCount = 0;
			nMovePixel = BLEND_WIDTH_ADJUST_STEP;
		}
	}
	//get value
	if(m_eBlendWidthSel==DIR__LEFT)
		nVal = m_stWarpConfig.stOsd.nBlendWidthL;
	else if(m_eBlendWidthSel==DIR__RIGHT)
		nVal = m_stWarpConfig.stOsd.nBlendWidthR;
	else if(m_eBlendWidthSel==DIR__UP)
		nVal = m_stWarpConfig.stOsd.nBlendWidthT;
	else if(m_eBlendWidthSel==DIR__DOWN)
		nVal = m_stWarpConfig.stOsd.nBlendWidthB;

	//set min/max
	switch(m_eBlendWidthSel)
	{
		default:
		case DIR__LEFT:
		case DIR__RIGHT:
			nMin = BLEND_WIDTH_H_MIN;
			nMax = BLEND_WIDTH_H_MAX;
			break;

		case DIR__UP:
		case DIR__DOWN:
			nMin = BLEND_WIDTH_V_MIN;
			nMax = BLEND_WIDTH_V_MAX;
			break;
	}

	//adjust value
	if(eAdj == ADJ__MINUS)
	{
		if (nVal < nMin+nMovePixel)  //A65_OPTOMA_Doulas_0020
			nVal = 0;
		else
			nVal -= nMovePixel;  //A65_OPTOMA_Doulas_0020
	}
	else
	{
		if(nVal > nMax-nMovePixel)  //A65_OPTOMA_Doulas_0020
			nVal = nMax;
		else
		{
			nVal += nMovePixel;  //A65_OPTOMA_Doulas_0020
			if(nVal < nMin)
				nVal = nMin;
		}
	}

	//set value
	if(m_eBlendWidthSel==DIR__LEFT)
	{
		if(nVal != m_stWarpConfig.stOsd.nBlendWidthL)
		{
			m_stWarpConfig.stOsd.nBlendWidthL = nVal;
			bChanged = TRUE;
		}
	}
	else if(m_eBlendWidthSel==DIR__RIGHT)
	{
		if(nVal != m_stWarpConfig.stOsd.nBlendWidthR)
		{
			m_stWarpConfig.stOsd.nBlendWidthR = nVal;
			bChanged = TRUE;
		}
	}
	else if(m_eBlendWidthSel==DIR__UP)
	{
		if(nVal != m_stWarpConfig.stOsd.nBlendWidthT)
		{
			m_stWarpConfig.stOsd.nBlendWidthT = nVal;
			bChanged = TRUE;
		}
	}
	else if(m_eBlendWidthSel==DIR__DOWN)
	{
		if(nVal != m_stWarpConfig.stOsd.nBlendWidthB)
		{
			m_stWarpConfig.stOsd.nBlendWidthB = nVal;
			bChanged = TRUE;
		}
	}

	if(bChanged)
	{
		m_bOsdWarpParamChanged = TRUE;
		utilWarp_ShowOsdPattern(PAT_TYPE__ADJ_BLEND_WIDTH);
	}

	clock_gettime(CLOCK_MONOTONIC, &m_stStartTime);  //A65_OPTOMA_Doulas_0020
	return bChanged;
}
#else
BOOL utilWarp_AdjustOsdBlendWidth(eADJ eAdj) //A35G2_BRC_Casper_0046
{
	uint16 nVal = 0;
	uint16 nMin = 0, nMax;
	BOOL bChanged = FALSE;

	if(eAdj >= ADJ__INVALID)
		return FALSE;
	dvC789_WriteToBuffer(BN_RTCT, RTCT_THRU);  		//G100_Doulas_0027
	//get value
	if(m_eBlendWidthSel==DIR__LEFT)
		nVal = m_stWarpConfig.stOsd.nBlendWidthL;
	else if(m_eBlendWidthSel==DIR__RIGHT)
		nVal = m_stWarpConfig.stOsd.nBlendWidthR;
	else if(m_eBlendWidthSel==DIR__UP)
		nVal = m_stWarpConfig.stOsd.nBlendWidthT;
	else if(m_eBlendWidthSel==DIR__DOWN)
		nVal = m_stWarpConfig.stOsd.nBlendWidthB;

	//set min/max
	switch(m_eBlendWidthSel)
	{
		default:
		case DIR__LEFT:
			nMax = BLEND_WIDTH_H_MAX-m_stWarpConfig.stOsd.nBlendOffsetL;
			break;
		case DIR__RIGHT:
			nMax = BLEND_WIDTH_H_MAX-m_stWarpConfig.stOsd.nBlendOffsetR;
			break;
		case DIR__UP:
			nMax = BLEND_WIDTH_V_MAX-m_stWarpConfig.stOsd.nBlendOffsetT;
			break;
		case DIR__DOWN:
			nMax = BLEND_WIDTH_V_MAX-m_stWarpConfig.stOsd.nBlendOffsetB;
			break;
	}

	//adjust value
	if(eAdj == ADJ__MINUS)
	{
		if (nVal < nMin+BLEND_WIDTH_ADJUST_STEP)
			nVal = 0;
		else
			nVal -= BLEND_WIDTH_ADJUST_STEP;
	}
	else
	{
		if(nVal > nMax-BLEND_WIDTH_ADJUST_STEP)
			nVal = nMax;
		else
		{
			nVal += BLEND_WIDTH_ADJUST_STEP;
			if(nVal < nMin)
				nVal = nMin;
		}
	}

	//set value
	if(m_eBlendWidthSel==DIR__LEFT)
	{
		if(nVal != m_stWarpConfig.stOsd.nBlendWidthL)
		{
			m_stWarpConfig.stOsd.nBlendWidthL = nVal;
			bChanged = TRUE;
		}
	}
	else if(m_eBlendWidthSel==DIR__RIGHT)
	{
		if(nVal != m_stWarpConfig.stOsd.nBlendWidthR)
		{
			m_stWarpConfig.stOsd.nBlendWidthR = nVal;
			bChanged = TRUE;
		}
	}
	else if(m_eBlendWidthSel==DIR__UP)
	{
		if(nVal != m_stWarpConfig.stOsd.nBlendWidthT)
		{
			m_stWarpConfig.stOsd.nBlendWidthT = nVal;
			bChanged = TRUE;
		}
	}
	else if(m_eBlendWidthSel==DIR__DOWN)
	{
		if(nVal != m_stWarpConfig.stOsd.nBlendWidthB)
		{
			m_stWarpConfig.stOsd.nBlendWidthB = nVal;
			bChanged = TRUE;
		}
	}

	if(bChanged)
	{
		m_bOsdWarpParamChanged = TRUE;
		utilWarp_ShowOsdPattern(PAT_TYPE__ADJ_BLEND_WIDTH);
	}

	return bChanged;
}
#endif

//return True : Changed
//return False : Not changed
BOOL utilWarp_SetOsdBlendGamma(uint8 nIdx)
{
	if( (nIdx > BLEND_GAMMA_MAX) ||
		(nIdx == m_stWarpConfig.stOsd.nBlendGamma))
		return FALSE;

	m_stWarpConfig.stOsd.nBlendGamma = nIdx;
	ApplyBlend(APPLY_BLEND__BY_CONFIG);
	return TRUE;
}

uint8 utilWarp_GetOsdBlendGamma(void)
{
	return m_stWarpConfig.stOsd.nBlendGamma;
}

#if 1   //A65_OPTOMA_Doulas_0020
//return true : changed
//return false : not changed
BOOL utilWarp_OsdBlackLevel_SetBrightness(eADJ eAdj)
{
	uint8 nIdx = m_eAreaSelection+1;
	BOOL bChanged = FALSE;
	uint8 i = 0;

	if(eAdj >= ADJ__INVALID)
		return FALSE;

	for(i=0; i< BLACKLEVEL_PALETTE_RGB; i++)
	{
		if(eAdj == ADJ__MINUS)
		{
			if(m_stWarpConfig.stOsd.stBlackLevel.pnColor[nIdx][i]>BLACKLEVEL_VALUE_MIN)		//A65_OPTOMA_Doulas_0020
			{
				m_stWarpConfig.stOsd.stBlackLevel.pnColor[nIdx][i]--;
				bChanged = TRUE;
			}
		}
		else
		{
			if(m_stWarpConfig.stOsd.stBlackLevel.pnColor[nIdx][i]<BLACKLEVEL_VALUE_MAX)		//A65_OPTOMA_Doulas_0020
			{
				m_stWarpConfig.stOsd.stBlackLevel.pnColor[nIdx][i]++;
				bChanged = TRUE;
			}
		}
	}

	if(bChanged)
	{
		CalcEgbBlackLevelGammaTable(3, nIdx, FALSE, m_stWarpConfig.stOsd.stBlackLevel.pnColor);
		WriteEgbBlackLevelGammaTable( 0, nIdx, FALSE );
		WriteEgbBlackLevelGammaTable( 1, nIdx, FALSE );
		WriteEgbBlackLevelGammaTable( 2, nIdx, FALSE );
		m_bOsdWarpParamChanged = TRUE;
	}

	return bChanged;
}

//return true : changed
//return false : not changed
BOOL utilWarp_OsdBlackLevel_SetRed(uint8 nR)
{
	uint8 nIdx = m_eAreaSelection+1;

	if(nR == m_stWarpConfig.stOsd.stBlackLevel.pnColor[nIdx][0])
	{
		return FALSE;
	}
	m_stWarpConfig.stOsd.stBlackLevel.pnColor[nIdx][0] = nR;
	CalcEgbBlackLevelGammaTable(0, nIdx, FALSE, m_stWarpConfig.stOsd.stBlackLevel.pnColor);
	WriteEgbBlackLevelGammaTable( 0, nIdx, FALSE );
	m_bOsdWarpParamChanged = TRUE;

	return TRUE;
}

uint8 utilWarp_OsdBlackLevel_GetRed(void)
{
	return m_stWarpConfig.stOsd.stBlackLevel.pnColor[m_eAreaSelection+1][0];
}

//return true : changed
//return false : not changed
BOOL utilWarp_OsdBlackLevel_SetGreen(uint8 nG)
{
	uint8 nIdx = m_eAreaSelection+1;

	if(nG == m_stWarpConfig.stOsd.stBlackLevel.pnColor[nIdx][1])
	{
		return FALSE;
	}
	m_stWarpConfig.stOsd.stBlackLevel.pnColor[nIdx][1] = nG;
	CalcEgbBlackLevelGammaTable(1, nIdx, FALSE, m_stWarpConfig.stOsd.stBlackLevel.pnColor);
	WriteEgbBlackLevelGammaTable( 1, nIdx, FALSE );
	m_bOsdWarpParamChanged = TRUE;

	return TRUE;
}

uint8 utilWarp_OsdBlackLevel_GetGreen(void)
{
	return m_stWarpConfig.stOsd.stBlackLevel.pnColor[m_eAreaSelection+1][1];
}

//return true : changed
//return false : not changed
BOOL utilWarp_OsdBlackLevel_SetBlue(uint8 nB)
{
	uint8 nIdx = m_eAreaSelection+1;

	if(nB == m_stWarpConfig.stOsd.stBlackLevel.pnColor[nIdx][2])
	{
		return FALSE;
	}
	m_stWarpConfig.stOsd.stBlackLevel.pnColor[nIdx][2] = nB;
	CalcEgbBlackLevelGammaTable(2, nIdx, FALSE, m_stWarpConfig.stOsd.stBlackLevel.pnColor);
	WriteEgbBlackLevelGammaTable( 2, nIdx, FALSE );
	m_bOsdWarpParamChanged = TRUE;

	return TRUE;
}

uint8 utilWarp_OsdBlackLevel_GetBlue(void)
{
	return m_stWarpConfig.stOsd.stBlackLevel.pnColor[m_eAreaSelection+1][2];
}

BOOL utilWarp_OsdBlackLevel_MovePoint(eDIR eDir)
{
	//update current point and point in list
	// 1. find point in list by compare current point
	// 2. update point value with direction(limit to screen boundary now. x : 0~1919, y : 0~1199)
	// 3. update current point value with point value in list
	Node* pTargetNode;
	uint16 nX = m_pnCurrentNode[m_eAreaSelection]->stPos.x;
	uint16 nY = m_pnCurrentNode[m_eAreaSelection]->stPos.y;
	uint16 nMovePixel;


	pTargetNode = SearchNode(m_pFirstNode[m_eAreaSelection], m_pnCurrentNode[m_eAreaSelection]);
	if(pTargetNode == NULL)
	{
		LOG_MSG(db_ASSERT, "Can not Find point!!\r\n");		//A65_OPTOMA_Doulas_0094 Modify
		return FALSE;
	}

	if (eDir!=m_eMovePrevDir)
	{
		m_eMovePrevDir = eDir;
		m_nMoveContinueCount = 0;
		nMovePixel = 1;
	}
	else
	{
		if(TimeElapsedMs() < BLACKLEVEL_MOVE_CONTINUE_TIMEOUT)
		{
			m_nMoveContinueCount++;
			if(m_nMoveContinueCount<(MOVE_CONTINUE_MAX*2))
			{
				nMovePixel = m_nMoveContinueCount*MOVE_CONTINUE_STEP;
			}
			else
			{
				nMovePixel = (MOVE_CONTINUE_MAX*2)*MOVE_CONTINUE_STEP;
			}
		}
		else
		{
			m_nMoveContinueCount = 0;
			nMovePixel = 1;
		}
	}

	switch (eDir)
	{
		default:
		case DIR__UP:
			nY = (nY<=nMovePixel) ? 0 : nY - nMovePixel;
			break;
		case DIR__DOWN:
			//nY = (nY>=(PS_PANEL_ACT_VW-nMovePixel-1)) ? (PS_PANEL_ACT_VW-1) : nY + nMovePixel;	//A65_OPTOMA_Doulas_0097
			nY = (nY>=(m_nNativeResV-nMovePixel-1)) ? (m_nNativeResV-1) : nY + nMovePixel;			//A65_OPTOMA_Doulas_0097
			break;
		case DIR__LEFT:
			nX = (nX<=nMovePixel) ? 0 : nX - nMovePixel;
			break;
		case DIR__RIGHT:
			//nX = (nX>=(PS_PANEL_ACT_HW-nMovePixel-1)) ? (PS_PANEL_ACT_HW-1) : nX + nMovePixel;	//A65_OPTOMA_Doulas_0097
			nX = (nX>=(m_nNativeResH-nMovePixel-1)) ? (m_nNativeResH-1) : nX + nMovePixel;			//A65_OPTOMA_Doulas_0097
			break;
	}

	if((m_pnCurrentNode[m_eAreaSelection]->stPos.x != nX)
		|| (m_pnCurrentNode[m_eAreaSelection]->stPos.y != nY))
	{
		m_bRewriteOsdBlackLevelArea = TRUE;
		m_bOsdWarpParamChanged = TRUE;
		m_pnCurrentNode[m_eAreaSelection]->stPos.x = nX;
		m_pnCurrentNode[m_eAreaSelection]->stPos.y = nY;
		pTargetNode->stPos.x = nX;
		pTargetNode->stPos.y = nY;
		utilWarp_ShowOsdPattern(PAT_TYPE__BLACKLEVEL_MOV_POINT);
	}
	else
	{
		return FALSE;
	}

	clock_gettime(CLOCK_MONOTONIC, &m_stStartTime);

	return TRUE;
}

//return true : changed
//return false : not changed
BOOL utilWarp_OsdBlackLevel_SetArea(eBLACKLEVEL_AREA eAreaSelect)
{
	if((m_eAreaSelection == eAreaSelect) ||
		(eAreaSelect >= BLACKLEVEL_AREA__NUM))
	{
		return FALSE;
	}
	m_eAreaSelection = eAreaSelect;
	m_bOsdWarpParamChanged = TRUE;									//A65_OPTOMA_Doulas_0020
	m_stWarpConfig.stOsd.stBlackLevel.ucArea = m_eAreaSelection; 	//A65_OPTOMA_Doulas_0020
	return TRUE;
}

eBLACKLEVEL_AREA utilWarp_OsdBlackLevel_GetArea(void)
{
	return (m_eAreaSelection);
}

//return true : changed
//return false : not changed
BOOL utilWarp_OsdBlackLevel_SetEnable(BOOL bEnable)
{
	dvC789_Write( BN_RTCT, RTCT_THRU);	//A65_OPTOMA_Doulas_0136
	if(bEnable == m_stWarpConfig.stOsd.stBlackLevel.bEnable[m_eAreaSelection])
	{
		return FALSE;
	}
	m_stWarpConfig.stOsd.stBlackLevel.bEnable[m_eAreaSelection] = bEnable;
	ApplyBlackLevel(APPLY_BLACKLEVEL__BY_CONFIG);
	return TRUE;
}

BOOL utilWarp_OsdBlackLevel_GetEnable(void)
{
	return m_stWarpConfig.stOsd.stBlackLevel.bEnable[m_eAreaSelection];
}

#if BLACKLEVEL_SEL_MODE			//A65_OPTOMA_Doulas_0146
void utilWarp_OsdBlackLevel_SelectOnThePattern(eDIR eDir)
{
	Node* pTargetNode;

	pTargetNode = GetNodeByDirection(eDir, GetTotalCountOfList(m_pFirstNode[m_eAreaSelection]));

	m_pnCurrentNode[m_eAreaSelection] = pTargetNode;

	utilWarp_ShowOsdPattern(m_ePatternType);
}
#else
void utilWarp_OsdBlackLevel_SelectOnThePattern(eSEQ eSeq)
{
	Node* pTargetNode;

	if(eSeq == SEQ__COUNTERCLOCKWISE)
		pTargetNode = GetNextNode(m_pFirstNode[m_eAreaSelection], m_pnCurrentNode[m_eAreaSelection]);
	else
		pTargetNode = GetPreviousNode(m_pFirstNode[m_eAreaSelection], m_pnCurrentNode[m_eAreaSelection]);

	m_pnCurrentNode[m_eAreaSelection] = pTargetNode;

	utilWarp_ShowOsdPattern(m_ePatternType);
}
#endif

BOOL utilWarp_OsdBlackLevel_IsPointAddable(void)
{
	uint8 nCnt = 0;

	nCnt = m_stWarpConfig.stOsd.stBlackLevel.nPointNum[m_eAreaSelection];
	if(nCnt < BLACKLEVEL_NODE_MAX)
	{
		return TRUE;
	}

	return FALSE;
}

BOOL utilWarp_OsdBlackLevel_IsPointDeletable(void)
{
	uint8 nCnt = 0;

	nCnt = m_stWarpConfig.stOsd.stBlackLevel.nPointNum[m_eAreaSelection];
	if(nCnt > BLACKLEVEL_NODE_MIN)
	{
		return TRUE;
	}

	return FALSE;
}

//return true : changed
//return false : not changed
BOOL utilWarp_OsdBlackLevel_AddPoint(void)
{
	PointT nAddPoint;
	Node* pTargetNode;
	BOOL bResult = FALSE;

	if(!utilWarp_OsdBlackLevel_IsPointAddable())
	{
		return FALSE;
	}
	pTargetNode = GetNextNode(m_pFirstNode[m_eAreaSelection], m_pnCurrentNode[m_eAreaSelection]);
	if(pTargetNode != NULL)
	{
		nAddPoint.x = (uint16)(pTargetNode->stPos.x + m_pnCurrentNode[m_eAreaSelection]->stPos.x)/2;
		nAddPoint.y = (uint16)(pTargetNode->stPos.y + m_pnCurrentNode[m_eAreaSelection]->stPos.y)/2;

		if(AddPointToList(m_pFirstNode[m_eAreaSelection], m_pnCurrentNode[m_eAreaSelection], nAddPoint))
		{
			m_stWarpConfig.stOsd.stBlackLevel.nPointNum[m_eAreaSelection]++;
			bResult = TRUE;
			m_bRewriteOsdBlackLevelArea = TRUE;
			m_bOsdWarpParamChanged = TRUE;
			utilWarp_ShowOsdPattern(PAT_TYPE__BLACKLEVEL_ADD_POINT);
		}
	}

	return bResult;
}

//return true : changed
//return false : not changed
BOOL utilWarp_OsdBlackLevel_DeletePoint(void)
{
	Node* pFirst;
	Node* pTargetNode;
	BOOL bResult = FALSE;

	if(!utilWarp_OsdBlackLevel_IsPointDeletable())
	{
		return FALSE;
	}
	pFirst = m_pFirstNode[m_eAreaSelection];
	pTargetNode = GetNextNode(pFirst, m_pnCurrentNode[m_eAreaSelection]);

	if(RemovePointFromList(&pFirst, m_pnCurrentNode[m_eAreaSelection]))  //A65_OPTOMA_Doulas_0024
	{
		m_pFirstNode[m_eAreaSelection] = pFirst;
		m_stWarpConfig.stOsd.stBlackLevel.nPointNum[m_eAreaSelection]--;
		m_pnCurrentNode[m_eAreaSelection] = pTargetNode;
		bResult = TRUE;
		m_bRewriteOsdBlackLevelArea = TRUE;
		m_bOsdWarpParamChanged = TRUE;
		utilWarp_ShowOsdPattern(PAT_TYPE__BLACKLEVEL_DEL_POINT);
	}

	return bResult;
}

void utilWarp_OsdBlackLevel_ResetConfig(void)
{
	dvC789_Write( BN_RTCT, RTCT_THRU);	//A65_OPTOMA_Doulas_0136
	m_eAreaSelection = BLACKLEVEL_AREA__TOP;
	OsdBlackLevel_ResetConfig(m_eAreaSelection);
	if(SaveWarpConfig()!=FLASH_ACCESS__PASS)
	{
		//LOG_MSG(db_ALWAYS,"save black level config fail\r\n");
	}
	InitBlackLevelNode(m_eAreaSelection);
	ApplyBlackLevel(APPLY_BLACKLEVEL__BY_CONFIG);

	m_eAreaSelection = BLACKLEVEL_AREA__BOTTOM; 			//A65_OPTOMA_Doulas_0020
	OsdBlackLevel_ResetConfig(m_eAreaSelection);
	if(SaveWarpConfig()!=FLASH_ACCESS__PASS)
	{
		//LOG_MSG(db_ALWAYS,"save black level config fail\r\n");
	}
	InitBlackLevelNode(m_eAreaSelection);
	ApplyBlackLevel(APPLY_BLACKLEVEL__BY_CONFIG);
}

OSD_BlackLevel utilWarp_OsdBlackLevel_GetBlackLevelData(void)
{
    return m_stWarpConfig.stOsd.stBlackLevel;
}

BOOL utilWarp_OsdBlackLevel_GetRewriteOsdBlackLevelArea(void)
{
    return m_bRewriteOsdBlackLevelArea;
}

BOOL utilWarp_OsdBlackLevel_GetOsdWarpParamChanged(void)
{
    return m_bOsdWarpParamChanged;
}

void utilWarp_OsdBlackLevel_SetOsdWarpParamChanged(BOOL bValue)
{
    m_bOsdWarpParamChanged = bValue;
}

eBLACKLEVEL_AREA utilWarp_OsdBlackLevel_GetAreaSelection(void)
{
    return m_eAreaSelection;
}

#endif

void utilWarp_PrintWarpConfig(void)  //A65_OPTOMA_Doulas_0020
{
	uint8 nCntX, nCntY, ii, jj;
	uint8 nSpaceH, nSpaceV;



	MS_SLEEP(10);
	LOG_MSG(db_ALWAYS,"Basic Warping Para\r\n");
	LOG_MSG(db_ALWAYS," PM_WPKEYANG_H      		%f \r\n",m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEYANG_H);
	LOG_MSG(db_ALWAYS," PM_WPKEYANG_V     	 	%f \r\n",m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEYANG_V);
	LOG_MSG(db_ALWAYS," PM_WPPINWID_H      		%f \r\n",m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPPINWID_H);
	LOG_MSG(db_ALWAYS," PM_WPPINWID_V      		%f \r\n",m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPPINWID_V);
	LOG_MSG(db_ALWAYS," PM_WPKEY4C_TLX      	%d \r\n",m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEY4C_TLX);
	LOG_MSG(db_ALWAYS," PM_WPKEY4C_TLY      	%d \r\n",m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEY4C_TLY);
	LOG_MSG(db_ALWAYS," PM_WPKEY4C_TRX      	%d \r\n",m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEY4C_TRX);
	LOG_MSG(db_ALWAYS," PM_WPKEY4C_TRY      	%d \r\n",m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEY4C_TRY);
	LOG_MSG(db_ALWAYS," PM_WPKEY4C_TRX      	%d \r\n",m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEY4C_BLX);
	LOG_MSG(db_ALWAYS," PM_WPKEY4C_TRY      	%d \r\n",m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEY4C_BLY);
	LOG_MSG(db_ALWAYS," PM_WPKEY4C_BRX      	%d \r\n",m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEY4C_BRX);
	LOG_MSG(db_ALWAYS," PM_WPKEY4C_BRY      	%d \r\n",m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEY4C_BRY);
	LOG_MSG(db_ALWAYS," m_cWarpAutoFilter      	%d \r\n",m_stWarpConfig.stOsdBasic.stWArpGeoPara.m_cWarpAutoFilter);
	LOG_MSG(db_ALWAYS," m_cWarpFilterSelect_H   %d \r\n",m_stWarpConfig.stOsdBasic.stWArpGeoPara.m_cWarpFilterSelect_H);
	LOG_MSG(db_ALWAYS," m_cWarpFilterSelect_V   %d \r\n",m_stWarpConfig.stOsdBasic.stWArpGeoPara.m_cWarpFilterSelect_V);
	LOG_MSG(db_ALWAYS," m_cWarpAutoFilterAp     %d \r\n",m_stWarpConfig.stOsdBasic.stWArpGeoPara.m_cWarpAutoFilterAp);  //A35G2_Simon_0110
	LOG_MSG(db_ALWAYS," m_cWarpFilterApValue_H  %d \r\n",m_stWarpConfig.stOsdBasic.stWArpGeoPara.m_cWarpFilterApValue_H);  //A35G2_Simon_0110
	LOG_MSG(db_ALWAYS," m_cWarpFilterApValue_V  %d \r\n",m_stWarpConfig.stOsdBasic.stWArpGeoPara.m_cWarpFilterApValue_V);  //A35G2_Simon_0110

	MS_SLEEP(10);
	LOG_MSG(db_ALWAYS,"Basic Blend Para\r\n");
	LOG_MSG(db_ALWAYS," ucBlending_T_Enable  %d \r\n",m_stWarpConfig.stOsdBasic.stBlendingPara.ucBlending_T_Enable);
	LOG_MSG(db_ALWAYS," ucBlending_B_Enable  %d \r\n",m_stWarpConfig.stOsdBasic.stBlendingPara.ucBlending_B_Enable);
	LOG_MSG(db_ALWAYS," ucBlending_L_Enable  %d \r\n",m_stWarpConfig.stOsdBasic.stBlendingPara.ucBlending_L_Enable);
	LOG_MSG(db_ALWAYS," ucBlending_R_Enable  %d \r\n",m_stWarpConfig.stOsdBasic.stBlendingPara.ucBlending_R_Enable);
	LOG_MSG(db_ALWAYS," uiBlending_T_St      %d \r\n",m_stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_T_St);
	LOG_MSG(db_ALWAYS," uiBlending_B_St      %d \r\n",m_stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_B_St);
	LOG_MSG(db_ALWAYS," uiBlending_L_St      %d \r\n",m_stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_L_St);
	LOG_MSG(db_ALWAYS," uiBlending_R_St      %d \r\n",m_stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_R_St);
	LOG_MSG(db_ALWAYS," uiBlending_T_Width   %d \r\n",m_stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_T_Width);
	LOG_MSG(db_ALWAYS," uiBlending_B_Width   %d \r\n",m_stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_B_Width);
	LOG_MSG(db_ALWAYS," uiBlending_L_Width   %d \r\n",m_stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_L_Width);
	LOG_MSG(db_ALWAYS," uiBlending_R_Width   %d \r\n",m_stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_R_Width);
	LOG_MSG(db_ALWAYS," ucBlendingGamma      %d \r\n\r\n",m_stWarpConfig.stOsdBasic.stBlendingPara.ucBlendingGamma);


	MS_SLEEP(10);
	LOG_MSG(db_ALWAYS, "utilWarp_NativePanel_Get() = %d\r\n", utilWarp_NativePanel_Get());
	LOG_MSG(db_ALWAYS, "utilWarp_GetColorPaletteGroupIndex() = %d\r\n", utilWarp_GetColorPaletteGroupIndex());
    LOG_MSG(db_ALWAYS, "utilWarp_GetOsdPatternType() = %d\r\n", utilWarp_GetOsdPatternType());

	LOG_MSG(db_ALWAYS, "Warp Control = %d\r\n", m_stWarpConfig.eWarpCtrl);
	LOG_MSG(db_ALWAYS, "Warp Inner = %d\r\n", m_stWarpConfig.stOsd.bWarpInnerOn);
	LOG_MSG(db_ALWAYS, "Warp Limit = %d\r\n", m_stWarpConfig.stOsd.bWarpLimit);
	LOG_MSG(db_ALWAYS, "Warp Sharpness = %d\r\n", m_stWarpConfig.stOsd.nWarpSharpness);
	LOG_MSG(db_ALWAYS, "Grid Color = %d\r\n", m_stWarpConfig.stOsd.eGridColor);
	LOG_MSG(db_ALWAYS, "Cursor Color = %d\r\n", m_stWarpConfig.stOsd.eCursorColor);
	LOG_MSG(db_ALWAYS, "Background Color = %d\r\n", m_stWarpConfig.stOsd.eBkgColor);
	LOG_MSG(db_ALWAYS, "--------------\r\n");
	MS_SLEEP(10);
	LOG_MSG(db_ALWAYS, "Left Blend Width = %d\r\n", m_stWarpConfig.stOsd.nBlendWidthL);
	LOG_MSG(db_ALWAYS, "Right Blend Width = %d\r\n", m_stWarpConfig.stOsd.nBlendWidthR);
	LOG_MSG(db_ALWAYS, "Top Blend Width = %d\r\n", m_stWarpConfig.stOsd.nBlendWidthT);
	LOG_MSG(db_ALWAYS, "Bottom Blend Width = %d\r\n", m_stWarpConfig.stOsd.nBlendWidthB);
	LOG_MSG(db_ALWAYS, "Overlap Grid Num = %d\r\n", m_stWarpConfig.stOsd.nOverlapGridNum);
	LOG_MSG(db_ALWAYS, "Blend Gamma = %d\r\n", m_stWarpConfig.stOsd.nBlendGamma);
	LOG_MSG(db_ALWAYS, "Blend Enable = %d\r\n", m_stWarpConfig.stOsd.bBlendEnable);
	LOG_MSG(db_ALWAYS, "Left Blend Offset = %d\r\n", m_stWarpConfig.stOsd.nBlendOffsetL);
	LOG_MSG(db_ALWAYS, "Right Blend Offset = %d\r\n", m_stWarpConfig.stOsd.nBlendOffsetR);
	LOG_MSG(db_ALWAYS, "Top Blend Offset = %d\r\n", m_stWarpConfig.stOsd.nBlendOffsetT);
	LOG_MSG(db_ALWAYS, "Bottom Blend Offset = %d\r\n", m_stWarpConfig.stOsd.nBlendOffsetB);
	LOG_MSG(db_ALWAYS, "--------------\r\n");
	MS_SLEEP(10);
	LOG_MSG(db_ALWAYS, "m_e3dResId = %d\r\n", m_e3dResId);
	LOG_MSG(db_ALWAYS, "m_nNativeResH = %d\r\n", m_nNativeResH);
	LOG_MSG(db_ALWAYS, "m_nNativeResV = %d\r\n", m_nNativeResV);
	LOG_MSG(db_ALWAYS, "m_n3dResH = %d\r\n", m_n3dResH);
	LOG_MSG(db_ALWAYS, "m_n3dResV = %d\r\n", m_n3dResV);
	LOG_MSG(db_ALWAYS, "NATIVE_WP_HW_GRD = %d\r\n", NATIVE_WP_HW_GRD);
	LOG_MSG(db_ALWAYS, "NATIVE_WP_VW_GRD = %d\r\n", NATIVE_WP_VW_GRD);
	LOG_MSG(db_ALWAYS, "THREED_WP_HW_GRD = %d\r\n", THREED_WP_HW_GRD);
	LOG_MSG(db_ALWAYS, "THREED_WP_VW_GRD = %d\r\n", THREED_WP_VW_GRD);
	LOG_MSG(db_ALWAYS, "m_f3dResRatioX = %d\r\n", m_f3dResRatioX);
	LOG_MSG(db_ALWAYS, "m_f3dResRatioY = %d\r\n", m_f3dResRatioY);
	LOG_MSG(db_ALWAYS, "--------------\r\n");

	switch(m_stWarpConfig.stOsd.eWarpPoint)
	{
		default:
		case WARP_POINT__2x2:
			nCntX = 2;
			nCntY = 2;
			break;
		case WARP_POINT__3x3:
			nCntX = 3;
			nCntY = 3;
			break;
		case WARP_POINT__5x5:
			nCntX = 5;
			nCntY = 5;
			break;
		case WARP_POINT__9x9:
			nCntX = 9;
			nCntY = 9;
			break;
		case WARP_POINT__17x17:
			nCntX = 17;
			nCntY = 17;
			break;

		case WARP_POINT__33x33:   //G100_Doulas_0059
			nCntX = 33;
			nCntY = 33;
			break;

	}

	LOG_MSG(db_ALWAYS, "Native Res. Warp Points = %d, (%d, %d)\r\n", m_stWarpConfig.stOsd.eWarpPoint, nCntX, nCntY);
	nSpaceH = (DEF_NUM_CUR_MAX_H-1)/(nCntX-1);  //G100_Doulas_0059
	nSpaceV = (DEF_NUM_CUR_MAX_V-1)/(nCntY-1);  //G100_Doulas_0059

	#if 0
	for(jj=0;jj<33;jj++)
	{
		printf("Y=%d\n", jj);
		for(ii=0;ii<33;ii++)
		{
			LOG_MSG(db_ALWAYS, "X%d(%4d.%02d, %4d.%02d) \n", ii,
				(int)PM_GRID[ii][jj].x, ((int)(PM_GRID[ii][jj].x*100))%100,
				(int)PM_GRID[ii][jj].y, ((int)(PM_GRID[ii][jj].y*100))%100);
		}
		//LOG_MSG(db_ALWAYS, "\r\n");
		printf("\n---------------------\n");
	}
	#endif

	#if 0
	for(jj=0;jj<nCntY;jj++)
	{
		printf("Y=%d\n", jj);
		for(ii=0;ii<nCntX;ii++)
		{
			LOG_MSG(db_ALWAYS, "X%d(%4d.%02d, %4d.%02d) \n", ii,
				(int)PM_GRID[ii*nSpaceH][jj*nSpaceV].x, ((int)(PM_GRID[ii*nSpaceH][jj*nSpaceV].x*100))%100,
				(int)PM_GRID[ii*nSpaceH][jj*nSpaceV].y, ((int)(PM_GRID[ii*nSpaceH][jj*nSpaceV].y*100))%100);
		}
		//LOG_MSG(db_ALWAYS, "\r\n");
		printf("\n---------------------\n");
	}
	#endif

	MS_SLEEP(10);
	if(m_bOutputNativeRes==FALSE)
	{
		LOG_MSG(db_ALWAYS, "Current Res. Warp Points = %d, (%d, %d)\r\n", m_stWarpConfig.stOsd.eWarpPoint, nCntX, nCntY);
		for(jj=0;jj<nCntY;jj++)
		{
			for(ii=0;ii<nCntX;ii++)
			{
				LOG_MSG(db_ALWAYS, "(%4d.%02d, %4d.%02d) ",
					(int)PM_GRID_TEMP[ii*nSpaceH][jj*nSpaceV].x, ((int)(PM_GRID_TEMP[ii*nSpaceH][jj*nSpaceV].x*100))%100,
					(int)PM_GRID_TEMP[ii*nSpaceH][jj*nSpaceV].y, ((int)(PM_GRID_TEMP[ii*nSpaceH][jj*nSpaceV].y*100))%100);
			}
			LOG_MSG(db_ALWAYS, "\r\n");
		}
		LOG_MSG(db_ALWAYS, "ratio = (%2d.%02d, %2d.%02d) \r\n",
					(int)m_fResRatioX, ((int)(m_fResRatioX*100))%100,
					(int)m_fResRatioY, ((int)(m_fResRatioY*100))%100);
	}

	MS_SLEEP(10);
	LOG_MSG(db_ALWAYS, "PC Warp Lut (H,V)=(%d,%d)\r\n", m_stWarpConfig.stPc.nWarpLutH, m_stWarpConfig.stPc.nWarpLutV);
	LOG_MSG(db_ALWAYS, "PC Blend Mode = %d\r\n", m_stWarpConfig.stPc.eBlendMode);
	LOG_MSG(db_ALWAYS, "PC Blend Gamma = %d\r\n", (int)(m_stWarpConfig.stPc.fBlendGamma*10));
	LOG_MSG(db_ALWAYS, "PC Blending Width (L,R,T,B)=(%d,%d,%d,%d)\r\n", m_stWarpConfig.stPc.nBlendWidthL,
		m_stWarpConfig.stPc.nBlendWidthR, m_stWarpConfig.stPc.nBlendWidthT, m_stWarpConfig.stPc.nBlendWidthB);
	LOG_MSG(db_ALWAYS, "PC Dot by Dot Blending Area Valid = %d\r\n", m_stWarpConfig.stPc.bIsDotByDotAreaValid);
//	LOG_MSG(db_ALWAYS, "Black Level Enable = %d\r\n", m_stWarpConfig.stPc.bBiasOn);
//	LOG_MSG(db_ALWAYS, "Black Level Area Valid = %d\r\n", m_stWarpConfig.stPc.bIsBiasAreaValid);
//	for(jj=0;jj<16;jj++)
//	{
//		LOG_MSG(db_ALWAYS, "(%2d, %3d, %3d, %3d) \r\n",  jj, m_stWarpConfig.stPc.pnBiasPaletteColor[jj][0],
//			m_stWarpConfig.stPc.pnBiasPaletteColor[jj][1], m_stWarpConfig.stPc.pnBiasPaletteColor[jj][2]);
//	}
	LOG_MSG(db_ALWAYS, "\r\n");

        //A65_OPTOMA_Doulas_0020
	LOG_MSG(db_ALWAYS, "Black Level Area Bottom node number = %d\r\n", m_stWarpConfig.stOsd.stBlackLevel.nPointNum[BLACKLEVEL_AREA__BOTTOM]);
	LOG_MSG(db_ALWAYS, "Black Level Area Bottom nodes\r\n");
	PrintList(m_pFirstNode[BLACKLEVEL_AREA__BOTTOM]);
	LOG_MSG(db_ALWAYS, "Black Level Area Top node number = %d\r\n", m_stWarpConfig.stOsd.stBlackLevel.nPointNum[BLACKLEVEL_AREA__TOP]);
	LOG_MSG(db_ALWAYS, "Black Level Area Top nodes\r\n");
	PrintList(m_pFirstNode[BLACKLEVEL_AREA__TOP]);
	if(m_eAreaSelection == BLACKLEVEL_AREA__BOTTOM)
	{
		LOG_MSG(db_ALWAYS, "Black Level BOTTOM area is selected\r\n");
	}
	else
	{
		LOG_MSG(db_ALWAYS, "Black Level TOP area is selected\r\n");
	}
	LOG_MSG(db_ALWAYS, "Black Level Area Bottom Enable = %d\r\n", m_stWarpConfig.stOsd.stBlackLevel.bEnable[BLACKLEVEL_AREA__BOTTOM]);
	LOG_MSG(db_ALWAYS, "Black Level Area Top Enable = %d\r\n", m_stWarpConfig.stOsd.stBlackLevel.bEnable[BLACKLEVEL_AREA__TOP]);
	for(jj=0;jj<16;jj++)
	{
		LOG_MSG(db_ALWAYS, "(%2d, %3d, %3d, %3d) \r\n",  jj, m_stWarpConfig.stOsd.stBlackLevel.pnColor[jj][0],
		m_stWarpConfig.stOsd.stBlackLevel.pnColor[jj][1], m_stWarpConfig.stOsd.stBlackLevel.pnColor[jj][2]);
	}
}

void utilWarp_Test(uint16 nCmd, uint16 nValue1, uint16 nValue2, uint16 nValue3, uint16 nValue4, uint16 nValue5)
{
	uint32 nVal = 0;

	switch(nCmd)	//A65_OPTOMA_Doulas_0020 Modify
	{
		default:
		case 0:
			LOG_MSG(db_ALWAYS, "system init, 3D ResId = %d\r\n", nValue1);
			utilWarp_SysInit(utilWarp_NativePanel_Get(), (eRES_ID)nValue1);   //G100_Doulas_0065
			break;

		case 1:
			LOG_MSG(db_ALWAYS, "print warp config\r\n");
			utilWarp_PrintWarpConfig(); //A65_OPTOMA_Doulas_0020
			break;

		case 2:   //G100_Doulas_0065
			LOG_MSG(db_ALWAYS, "Change output timing to %d\r\n", nValue1);
			utilWarp_SetOutputTiming((eRES_ID)nValue1);
			ApplyWarp(APPLY_WARP__RE_CALC);
			ApplyBlend(APPLY_BLEND__BY_CONFIG);
			break;

		case 6:
			LOG_MSG(db_ALWAYS, "wirte default warping table\r\n");
			utilWarp_WriteDefaultWarpTable();
			break;

		case 10:
			LOG_MSG(db_ALWAYS, "Clear warp/blend memory -  ");
			nVal = utilWarp_ClearOsdPreset();
			if(FLASH_ACCESS__PASS!=nVal)
				LOG_MSG(db_ALWAYS, "NG, error code = %d\r\n", (uint8)nVal);
			else
				LOG_MSG(db_ALWAYS, "OK\r\n");
			break;

		case 32:
			//enter blacklevel cusor select mode
			//current point in cursor color, others in grid color
			LOG_MSG(db_ALWAYS, "Enter Black Level Area Editor mode \r\n");
			utilWarp_ShowOsdPattern(PAT_TYPE__BLACKLEVEL_SEL_POINT);
			break;
		case 33:
			//select cursor in cursor selecting mode
			#if BLACKLEVEL_SEL_MODE			//A65_OPTOMA_Doulas_0146 Modify
			if((eDIR)nValue1 < DIR__INVALID)
			{
				utilWarp_OsdBlackLevel_SelectOnThePattern((eDIR)nValue1);
			}
			#else
			if((eSEQ)nValue1<SEQ__INVALID)
			{
				utilWarp_OsdBlackLevel_SelectOnThePattern((eSEQ)nValue1);
			}
			#endif
			else
			{
				LOG_MSG(db_ALWAYS, "Parameter is invalid, value = %d \r\n", nValue1);
			}
			break;
		case 34:
			//enter blacklevel cusor moving mode
			LOG_MSG(db_ALWAYS, "Enter Black Level cursor moving mode\r\n" );
			utilWarp_ShowOsdPattern(PAT_TYPE__BLACKLEVEL_MOV_POINT);
			break;
		case 35:
			//moving cursor in cusor moving mode
			LOG_MSG(db_ALWAYS, "Move selected point, dir=%d, run=%d\r\n", nValue1, utilWarp_OsdBlackLevel_MovePoint((eDIR)nValue1));
			break;
		case 36:
			//enter blacklevel add point select mode
			LOG_MSG(db_ALWAYS, "Enter BlackLevel adding point mode\r\n");
			//if count of  point list is 32, show warning message
			if(utilWarp_OsdBlackLevel_IsPointAddable())
			{
				utilWarp_ShowOsdPattern(PAT_TYPE__BLACKLEVEL_ADD_POINT);
			}
			else
			{
				LOG_MSG(db_ALWAYS, "Point number is 32, can not be added any more!!\r\n");
			}
			break;
		case 38:
			//confirm blacklevel add point
			LOG_MSG(db_ALWAYS, "Confirm to add point in select line : ");
			if(utilWarp_OsdBlackLevel_AddPoint())
			{

				LOG_MSG(db_ALWAYS, "Succeed!!\r\n");
			}
			else
			{
				LOG_MSG(db_ALWAYS, "Failure!!\r\n");
			}
			break;
		case 39:
			//enter blacklevel delete point select mode
			LOG_MSG(db_ALWAYS, "Enter BlackLevel Deleting point mode\r\n");
			//if count of  point list is 4, show warning message
			if(utilWarp_OsdBlackLevel_IsPointDeletable())
			{
				utilWarp_ShowOsdPattern(PAT_TYPE__BLACKLEVEL_DEL_POINT);
			}
			else
			{
				LOG_MSG(db_ALWAYS, "Point number is 4, can not be deleted any more!!\r\n");
			}
			break;
		case 41:
			//confirm blacklevel delete point and back to delete point select mode
			LOG_MSG(db_ALWAYS, "Confirm to delete point in selecting node : ");
			if(utilWarp_OsdBlackLevel_DeletePoint())
			{
				LOG_MSG(db_ALWAYS, "Succeed!!\r\n");
			}
			else
			{
				LOG_MSG(db_ALWAYS, "Failure!!\r\n");
			}
			break;
		case 42:
			//exit case 32~41 and apply blacklevel to selected area
			LOG_MSG(db_ALWAYS, "Enter preview mode!!\r\n");
			utilWarp_ShowOsdPattern(PAT_TYPE__BLACKLEVEL_PREVIEW);
			break;

		case 43:
			LOG_MSG(db_ALWAYS, "Apply BlackLevel area disable/enable = %d\r\n", nValue1);
			nVal = utilWarp_OsdBlackLevel_SetEnable((BOOL)nValue1);
			if(nVal)
			{
				LOG_MSG(db_ALWAYS, "changed\r\n");
				SaveWarpConfig();
			}
			else
			{
				LOG_MSG(db_ALWAYS, "no change\r\n");
			}

			break;
		case 44:
			LOG_MSG(db_ALWAYS, "Get current BlackLevel area enable/disable : ");
			if(utilWarp_OsdBlackLevel_GetEnable())
			{
				LOG_MSG(db_ALWAYS, "Enable!! \r\n");
			}
			else
			{
				LOG_MSG(db_ALWAYS, "Disable!! \r\n");
			}
			break;
		case 45:
			LOG_MSG(db_ALWAYS, "Select BlackLevel area %d, ", nValue1);
			nVal = utilWarp_OsdBlackLevel_SetArea((eBLACKLEVEL_AREA)nValue1);
			if(nVal)
			{
				LOG_MSG(db_ALWAYS, "changed\r\n");
				utilWarp_ShowOsdPattern(PAT_TYPE__BLACKLEVEL_PREVIEW);
			}
			else
			{
				LOG_MSG(db_ALWAYS, "no change\r\n");
			}
			break;
		case 46:
			LOG_MSG(db_ALWAYS, "Show current BlackLevel area : ");
			if(utilWarp_OsdBlackLevel_GetArea() == BLACKLEVEL_AREA__BOTTOM)
			{
				LOG_MSG(db_ALWAYS, "Bottom!! \r\n");
			}
			else if(utilWarp_OsdBlackLevel_GetArea() == BLACKLEVEL_AREA__TOP)
			{
				LOG_MSG(db_ALWAYS, "Top!! \r\n");
			}
			else
			{
				LOG_MSG(db_ALWAYS, "Invalid!! \r\n");
			}
			break;

		case 51:
			LOG_MSG(db_ALWAYS, "reset warp config\r\n");
			utilWarp_ResetWarpConfig();
			break;

		case 54:
			LOG_MSG(db_ALWAYS, "warp/blend memory - save current, index = %d, ", nValue1);
			nVal = utilWarp_SaveOsdPreset(nValue1);
			if(FLASH_ACCESS__PASS!=nVal)
				LOG_MSG(db_ALWAYS, "NG, error code = %d\r\n", (uint8)nVal);
			else
				LOG_MSG(db_ALWAYS, "OK\r\n");
			break;

		case 55:
			LOG_MSG(db_ALWAYS, "warp/blend memory - apply, index = %d, ", nValue1);
			nVal = utilWarp_LoadOsdPreset(nValue1);
			if(FLASH_ACCESS__PASS!=nVal)
				LOG_MSG(db_ALWAYS, "NG, error code = %d\r\n", (uint8)nVal);
			else
				LOG_MSG(db_ALWAYS, "OK\r\n");
			break;

		case 56:
			LOG_MSG(db_ALWAYS, "set warp control, value = %d, ", nValue1);
			nVal = utilWarp_SetWarpCtrl((eWARP_CTRL)nValue1);
			if(nVal)
			{
				LOG_MSG(db_ALWAYS, "changed\r\n");
				SaveWarpConfig();
			}
			else
				LOG_MSG(db_ALWAYS, "no change\r\n");
			break;

		case 57:
			LOG_MSG(db_ALWAYS, "get warp control, value = %d\r\n", utilWarp_GetWarpCtrl());
			break;

		case 58:
			LOG_MSG(db_ALWAYS, "show warping pattern in selection mode\r\n");
			utilWarp_ShowOsdPattern(PAT_TYPE__WARP_SEL_CTRL_POINT);
			break;

		case 59:
			LOG_MSG(db_ALWAYS, "select control point, value = %d\r\n", nValue1);
			utilWarp_SelectControlPoint((eDIR)nValue1);
			break;

		case 60:
			LOG_MSG(db_ALWAYS, "show control point pattern in movement mode\r\n");
			utilWarp_ShowOsdPattern(PAT_TYPE__WARP_MOV_CTRL_POINT);
			break;

		case 61:
			LOG_MSG(db_ALWAYS, "move control point, dir=%d, run=%d\r\n", nValue1, utilWarp_MoveControlPoint((eDIR)nValue1));
			break;

		case 62:
			LOG_MSG(db_ALWAYS, "close warping/blending pattern\r\n");
			utilWarp_ShowOsdPattern(PAT_TYPE__OFF);
			break;

		case 63:
			LOG_MSG(db_ALWAYS, "set grid points, value = %d, ", nValue1);
			nVal = utilWarp_SetWarpPoint((eWARP_POINT)nValue1);
			if(nVal)
			{
				LOG_MSG(db_ALWAYS, "changed\r\n");
				SaveWarpConfig();
			}
			else
				LOG_MSG(db_ALWAYS, "no change\r\n");
			break;

		case 64:
			LOG_MSG(db_ALWAYS, "get grid points, value = %d\r\n", utilWarp_GetWarpPoint());
			break;

		case 65:
			LOG_MSG(db_ALWAYS, "set warp inner, value = %d, ", nValue1);
			nVal = utilWarp_SetWarpInner(nValue1);
			if(nVal)
			{
				LOG_MSG(db_ALWAYS, "changed\r\n");
				SaveWarpConfig();
			}
			else
				LOG_MSG(db_ALWAYS, "no change\r\n");
			break;

		case 66:
			LOG_MSG(db_ALWAYS, "get warp inner, value = %d\r\n", utilWarp_GetWarpInner());
			break;

		case 67:
			LOG_MSG(db_ALWAYS, "set warp sharpness, value = %d, ", nValue1);
			nVal = utilWarp_SetOsdWarpSharpness(nValue1);
			if(nVal)
			{
				LOG_MSG(db_ALWAYS, "changed\r\n");
				SaveWarpConfig();
			}
			else
				LOG_MSG(db_ALWAYS, "no change\r\n");
			break;

		case 68:
			LOG_MSG(db_ALWAYS, "get warp sharpness, value = %d\r\n", utilWarp_GetOsdWarpSharpness());
			break;
		case 69:
			LOG_MSG(db_ALWAYS, "warp setting - set grid color, value = %d, ", nValue1);
			nVal = utilWarp_SetOsdGridColor((eGRID_COLOR)nValue1, TRUE);
			if(nVal)
			{
				LOG_MSG(db_ALWAYS, "changed\r\n");
				SaveWarpConfig();
			}
			else
				LOG_MSG(db_ALWAYS, "no change\r\n");
			break;

		case 70:
			LOG_MSG(db_ALWAYS, "warp setting - get grid color, value = %d\r\n", utilWarp_GetOsdGridColor());
			break;

		case 71:
			LOG_MSG(db_ALWAYS, "warp setting - set background color, value = %d, ", nValue1);
			nVal = utilWarp_SetOsdBkgColor((eBKG_COLOR)nValue1, TRUE);
			if(nVal)
			{
				LOG_MSG(db_ALWAYS, "changed\r\n");
				SaveWarpConfig();
			}
			else
				LOG_MSG(db_ALWAYS, "no change\r\n");
			break;

		case 72:
			LOG_MSG(db_ALWAYS, "warp setting - get background color, value = %d\r\n", utilWarp_GetOsdBkgColor());
			break;

		case 73:
			LOG_MSG(db_ALWAYS, "show select blend width pattern\r\n");
			utilWarp_ShowOsdPattern(PAT_TYPE__SEL_BLEND_WIDTH);
			break;

		case 74:
			LOG_MSG(db_ALWAYS, "select blend width, value = %d\r\n", nValue1);
			utilWarp_SelectOsdBlendWidth((eDIR)nValue1);
			break;

		case 75:
			LOG_MSG(db_ALWAYS, "show adjust blend width pattern\r\n");
			utilWarp_ShowOsdPattern(PAT_TYPE__ADJ_BLEND_WIDTH);
			break;

		case 76:
			LOG_MSG(db_ALWAYS, "adjust blend width, dir=%d, run=%d\r\n", nValue1, utilWarp_AdjustOsdBlendWidth((eADJ)nValue1));
			break;

		case 77:
			LOG_MSG(db_ALWAYS, "blend setting - set overlap grid num, value = %d, ", nValue1);
			nVal = utilWarp_SetOverlapGridNum(nValue1);
			if(nVal)
			{
				LOG_MSG(db_ALWAYS, "changed\r\n");
                utilWarp_ReDrawWarpPattern(); //A35G2_BRC_Casper_0047
				SaveWarpConfig();
			}
			else
				LOG_MSG(db_ALWAYS, "no change\r\n");
			break;

		case 78:
			LOG_MSG(db_ALWAYS, "blend setting - get overlap grid num, value = %d\r\n", utilWarp_GetOverlapGridNum());
			break;

		case 79:
			LOG_MSG(db_ALWAYS, "blend setting - set blend gamma, value = %d, ", nValue1);
			nVal = utilWarp_SetOsdBlendGamma(nValue1);
			if(nVal)
			{
				LOG_MSG(db_ALWAYS, "changed\r\n");
				SaveWarpConfig();
			}
			else
				LOG_MSG(db_ALWAYS, "no change\r\n");
			break;

		case 80:
			LOG_MSG(db_ALWAYS, "blend setting - get blend gamma, value = %d\r\n", utilWarp_GetOsdBlendGamma());
			break;

		case 85:
			LOG_MSG(db_ALWAYS, "set black level brightness decrease/increase = %d, ",nValue1);
			nVal = utilWarp_OsdBlackLevel_SetBrightness((eADJ)nValue1);
			if(nVal)
			{
				LOG_MSG(db_ALWAYS, "changed\r\n");
				//TODO
				//After exiting OSD brightness adjust menu
				//save it if value is changed
				if(utilWarp_GetOsdPatternType() == PAT_TYPE__OFF)
				{
					SaveWarpConfig();
				}
			}
			else
			{
				LOG_MSG(db_ALWAYS, "no change!!\r\n");
			}
			break;
		case 87:
			LOG_MSG(db_ALWAYS, "set black level palette color Red = %d, ",nValue1);
			nVal = utilWarp_OsdBlackLevel_SetRed((uint8)nValue1);
			if(nVal)
			{
				LOG_MSG(db_ALWAYS, "changed\r\n");
				//TODO
				//After exiting OSD brightness adjust menu
				//save it if value is changed
				if(utilWarp_GetOsdPatternType() == PAT_TYPE__OFF)
				{
					SaveWarpConfig();
				}
			}
			else
			{
				LOG_MSG(db_ALWAYS, "no change!!\r\n");
			}
			break;

		case 88:
			LOG_MSG(db_ALWAYS, "get black level palette color Red = %d\r\n",
						utilWarp_OsdBlackLevel_GetRed());
			break;
		case 89:
			LOG_MSG(db_ALWAYS, "set black level palette color Green = %d, ",nValue1);
			nVal = utilWarp_OsdBlackLevel_SetGreen((uint8)nValue1);
			if(nVal)
			{
				LOG_MSG(db_ALWAYS, "changed\r\n");
				//TODO
				//After exiting OSD brightness adjust menu
				//save it if value is changed
				if(utilWarp_GetOsdPatternType() == PAT_TYPE__OFF)
				{
					SaveWarpConfig();
				}
			}
			else
			{
				LOG_MSG(db_ALWAYS, "no change!!\r\n");
			}
			break;

		case 90:
			LOG_MSG(db_ALWAYS, "get black level palette color Green = %d\r\n",
						utilWarp_OsdBlackLevel_GetGreen());
			break;
		case 91:
			LOG_MSG(db_ALWAYS, "set black level palette color Blue = %d, ",nValue1);
			nVal = utilWarp_OsdBlackLevel_SetBlue((uint8)nValue1);
			if(nVal)
			{
				LOG_MSG(db_ALWAYS, "changed\r\n");
				//TODO
				//After exiting OSD brightness adjust menu
				//save it if value is changed
				if(utilWarp_GetOsdPatternType() == PAT_TYPE__OFF)
				{
					SaveWarpConfig();
				}
			}
			else
			{
				LOG_MSG(db_ALWAYS, "no change!!\r\n");
			}
			break;

		case 92:
			LOG_MSG(db_ALWAYS, "get black level palette color Blue = %d\r\n",
						utilWarp_OsdBlackLevel_GetBlue());
			break;

		case 94:
			LOG_MSG(db_ALWAYS, "Reset Black Level Area ");
			if(utilWarp_OsdBlackLevel_GetArea() == BLACKLEVEL_AREA__BOTTOM)
			{
				LOG_MSG(db_ALWAYS, "Bottom!!\r\n");
			}
			else
			{
				LOG_MSG(db_ALWAYS, "Top!!\r\n");
			}
			utilWarp_OsdBlackLevel_ResetConfig();
			break;

	#if (ENABLE_COLOR_UNIFORMITY == TRUE)					//G100_Tim_0012, add, start
        case 95:
            if(nValue1 == 0)
            {
                utilWarp_ShowOsdPattern(PAT_TYPE__TESTPATTERN_WHITE);
            }
            else if(nValue1 == 1)
            {
                utilWarp_ShowOsdPattern(PAT_TYPE__TESTPATTERN_GRID);
            }
            else if(nValue1 == 2)
            {
                utilWarp_ShowOsdPattern(PAT_TYPE__TESTPATTERN_GRIDCENTER);
            }
            else if(nValue1 == 3)
            {
                utilWarp_ShowOsdPattern(PAT_TYPE__TESTPATTERN_BLACK);
            }
            break;
	#endif //ENABLE_COLOR_UNIFORMITY						//G100_Tim_0012, add, end

	}
}

void utilBlend_BlendWidthGet(UINT16 *ucBlend_LF,UINT16 *ucBlend_RT,UINT16 *ucBlend_TP,UINT16 *ucBlend_BM)		//G100_Doulas_0027
{
	*ucBlend_LF = m_stWarpConfig.stOsd.nBlendWidthL;
	*ucBlend_RT = m_stWarpConfig.stOsd.nBlendWidthR;
	*ucBlend_TP = m_stWarpConfig.stOsd.nBlendWidthT;
	*ucBlend_BM = m_stWarpConfig.stOsd.nBlendWidthB;
}

void utilBasicWarpSettingSet(sWARP_BASIC stWarpingBasic)		//G100_Doulas_0027
{
	UINT8 ucDataChanged = FALSE;
	if((m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEYANG_H != stWarpingBasic.PM_WPKEYANG_H) ||
	   (m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEYANG_V != stWarpingBasic.PM_WPKEYANG_V) ||
	   (m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPPINWID_H != stWarpingBasic.PM_WPPINWID_H) ||
	   (m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPPINWID_V != stWarpingBasic.PM_WPPINWID_V) ||
	   (m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEY4C_TLX != stWarpingBasic.PM_WPKEY4C_TLX) ||
	   (m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEY4C_TLY != stWarpingBasic.PM_WPKEY4C_TLY) ||
	   (m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEY4C_TRX != stWarpingBasic.PM_WPKEY4C_TRX) ||
	   (m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEY4C_TRY != stWarpingBasic.PM_WPKEY4C_TRY) ||
	   (m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEY4C_BLX != stWarpingBasic.PM_WPKEY4C_BLX) ||
	   (m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEY4C_BLY != stWarpingBasic.PM_WPKEY4C_BLY) ||
	   (m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEY4C_BRX != stWarpingBasic.PM_WPKEY4C_BRX) ||
	   (m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEY4C_BRY != stWarpingBasic.PM_WPKEY4C_BRY) ||
	   (m_stWarpConfig.stOsdBasic.stWArpGeoPara.m_cWarpAutoFilter != stWarpingBasic.m_cWarpAutoFilter) ||
	   (m_stWarpConfig.stOsdBasic.stWArpGeoPara.m_cWarpAutoFilterAp != stWarpingBasic.m_cWarpAutoFilterAp) ||       //A35G2_Simon_0110
	   (m_stWarpConfig.stOsdBasic.stWArpGeoPara.m_cWarpFilterSelect_H != stWarpingBasic.m_cWarpFilterSelect_H) ||
	   (m_stWarpConfig.stOsdBasic.stWArpGeoPara.m_cWarpFilterSelect_V != stWarpingBasic.m_cWarpFilterSelect_V)||    //A35G2_Simon_0110
	   (m_stWarpConfig.stOsdBasic.stWArpGeoPara.m_cWarpFilterApValue_H != stWarpingBasic.m_cWarpFilterApValue_H)||  //A35G2_Simon_0110
	   (m_stWarpConfig.stOsdBasic.stWArpGeoPara.m_cWarpFilterApValue_V != stWarpingBasic.m_cWarpFilterApValue_V)    //A35G2_Simon_0110
	   )
	{
		ucDataChanged = TRUE;	//G100_Doulas_0047 Modify
	}
	memcpy(&m_stWarpConfig.stOsdBasic.stWArpGeoPara, &(stWarpingBasic), sizeof(sWARP_BASIC));
	if(ucDataChanged)	//G100_Doulas_0035 Modify
	{
		//LOG_MSG(db_ALWAYS," +++++++===Change==++++\r\n");
		SaveWarpConfig();	//G100_Doulas_0028 remove
	}
	LOG_MSG(db_HAL_WARPING," PM_WPKEYANG_H      		%f \r\n",m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEYANG_H);
	LOG_MSG(db_HAL_WARPING," PM_WPKEYANG_V     	 	%f \r\n",m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEYANG_V);
	LOG_MSG(db_HAL_WARPING," PM_WPPINWID_H      		%f \r\n",m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPPINWID_H);
	LOG_MSG(db_HAL_WARPING," PM_WPPINWID_V      		%f \r\n",m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPPINWID_V);
	LOG_MSG(db_HAL_WARPING," PM_WPKEY4C_TLX      	%d \r\n",m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEY4C_TLX);
	LOG_MSG(db_HAL_WARPING," PM_WPKEY4C_TLY      	%d \r\n",m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEY4C_TLY);
	LOG_MSG(db_HAL_WARPING," PM_WPKEY4C_TRX      	%d \r\n",m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEY4C_TRX);
	LOG_MSG(db_HAL_WARPING," PM_WPKEY4C_TRY      	%d \r\n",m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEY4C_TRY);
	LOG_MSG(db_HAL_WARPING," PM_WPKEY4C_TRX      	%d \r\n",m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEY4C_BLX);
	LOG_MSG(db_HAL_WARPING," PM_WPKEY4C_TRY      	%d \r\n",m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEY4C_BLY);
	LOG_MSG(db_HAL_WARPING," PM_WPKEY4C_BRX      	%d \r\n",m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEY4C_BRX);
	LOG_MSG(db_HAL_WARPING," PM_WPKEY4C_BRY      	%d \r\n",m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEY4C_BRY);
	LOG_MSG(db_HAL_WARPING," m_cWarpAutoFilter      	%d \r\n",m_stWarpConfig.stOsdBasic.stWArpGeoPara.m_cWarpAutoFilter);
	LOG_MSG(db_HAL_WARPING," m_cWarpFilterSelect_H   %d \r\n",m_stWarpConfig.stOsdBasic.stWArpGeoPara.m_cWarpFilterSelect_H);
	LOG_MSG(db_HAL_WARPING," m_cWarpFilterSelect_V   %d \r\n",m_stWarpConfig.stOsdBasic.stWArpGeoPara.m_cWarpFilterSelect_V);
	LOG_MSG(db_HAL_WARPING," m_cWarpAutoFilterAp      	%d \r\n",m_stWarpConfig.stOsdBasic.stWArpGeoPara.m_cWarpAutoFilterAp);  //A35G2_Simon_0110
	LOG_MSG(db_HAL_WARPING," m_cWarpFilterApValue_H   %d \r\n",m_stWarpConfig.stOsdBasic.stWArpGeoPara.m_cWarpFilterApValue_H);  //A35G2_Simon_0110
	LOG_MSG(db_HAL_WARPING," m_cWarpFilterApValue_V   %d \r\n",m_stWarpConfig.stOsdBasic.stWArpGeoPara.m_cWarpFilterApValue_V);  //A35G2_Simon_0110

}

void utilBasicBlendSettingSet(sBLENDING_BASIC stBlendingBasic)		//G100_Doulas_0027
{
	UINT8 ucDataChanged = FALSE;
	if((m_stWarpConfig.stOsdBasic.stBlendingPara.ucBlending_T_Enable != stBlendingBasic.ucBlending_T_Enable) ||
	   (m_stWarpConfig.stOsdBasic.stBlendingPara.ucBlending_B_Enable != stBlendingBasic.ucBlending_B_Enable) ||
	   (m_stWarpConfig.stOsdBasic.stBlendingPara.ucBlending_L_Enable != stBlendingBasic.ucBlending_L_Enable) ||
	   (m_stWarpConfig.stOsdBasic.stBlendingPara.ucBlending_R_Enable != stBlendingBasic.ucBlending_R_Enable) ||
	   (m_stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_T_St != stBlendingBasic.uiBlending_T_St) ||
	   (m_stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_B_St != stBlendingBasic.uiBlending_B_St) ||
	   (m_stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_L_St != stBlendingBasic.uiBlending_L_St) ||
	   (m_stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_R_St != stBlendingBasic.uiBlending_R_St) ||
	   (m_stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_T_Width != stBlendingBasic.uiBlending_T_Width) ||
	   (m_stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_B_Width != stBlendingBasic.uiBlending_B_Width) ||
	   (m_stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_L_Width != stBlendingBasic.uiBlending_L_Width) ||
	   (m_stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_R_Width != stBlendingBasic.uiBlending_R_Width) ||
	   (m_stWarpConfig.stOsdBasic.stBlendingPara.ucBlendingGamma != stBlendingBasic.ucBlendingGamma) )		//G100_Doulas_0035 Add
	{
		ucDataChanged = TRUE;	//G100_Doulas_0047 Modify
	}
	memcpy(&m_stWarpConfig.stOsdBasic.stBlendingPara, &(stBlendingBasic), sizeof(sBLENDING_BASIC));
	if(ucDataChanged)	//G100_Doulas_0035 Modify
	{
		//LOG_MSG(db_ALWAYS," +++++++===Change==++++\r\n");
		SaveWarpConfig();	//G100_Doulas_0028 remove
	}

	LOG_MSG(db_HAL_WARPING," ucBlending_T_Enable     %d \r\n",m_stWarpConfig.stOsdBasic.stBlendingPara.ucBlending_T_Enable);
	LOG_MSG(db_HAL_WARPING," ucBlending_B_Enable     %d \r\n",m_stWarpConfig.stOsdBasic.stBlendingPara.ucBlending_B_Enable);
	LOG_MSG(db_HAL_WARPING," ucBlending_L_Enable     %d \r\n",m_stWarpConfig.stOsdBasic.stBlendingPara.ucBlending_L_Enable);
	LOG_MSG(db_HAL_WARPING," ucBlending_R_Enable     %d \r\n",m_stWarpConfig.stOsdBasic.stBlendingPara.ucBlending_R_Enable);
	LOG_MSG(db_HAL_WARPING," uiBlending_T_St     	%d \r\n",m_stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_T_St);
	LOG_MSG(db_HAL_WARPING," uiBlending_B_St     	%d \r\n",m_stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_B_St);
	LOG_MSG(db_HAL_WARPING," uiBlending_L_St     	%d \r\n",m_stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_L_St);
	LOG_MSG(db_HAL_WARPING," uiBlending_R_St     	%d \r\n",m_stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_R_St);
	LOG_MSG(db_HAL_WARPING," uiBlending_T_Width     	%d \r\n",m_stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_T_Width);
	LOG_MSG(db_HAL_WARPING," uiBlending_B_Width     	%d \r\n",m_stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_B_Width);
	LOG_MSG(db_HAL_WARPING," uiBlending_L_Width     	%d \r\n",m_stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_L_Width);
	LOG_MSG(db_HAL_WARPING," uiBlending_R_Width     	%d \r\n",m_stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_R_Width);
	LOG_MSG(db_HAL_WARPING," ucBlendingGamma         %d \r\n",m_stWarpConfig.stOsdBasic.stBlendingPara.ucBlendingGamma);

}


void utilBasicWarpSettingGet(sWARP_BASIC *stWarpingBasic)		//G100_Doulas_0027
{
	memcpy(stWarpingBasic,&m_stWarpConfig.stOsdBasic.stWArpGeoPara, sizeof(sWARP_BASIC));

	LOG_MSG(db_HAL_WARPING," PM_WPKEYANG_H      		%f \r\n",m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEYANG_H);	//G100_Doulas_0035 Add
	LOG_MSG(db_HAL_WARPING," PM_WPKEYANG_V     	 	%f \r\n",m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEYANG_V);
	LOG_MSG(db_HAL_WARPING," PM_WPPINWID_H      		%f \r\n",m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPPINWID_H);
	LOG_MSG(db_HAL_WARPING," PM_WPPINWID_V      		%f \r\n",m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPPINWID_V);
	LOG_MSG(db_HAL_WARPING," PM_WPKEY4C_TLX      	%d \r\n",m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEY4C_TLX);
	LOG_MSG(db_HAL_WARPING," PM_WPKEY4C_TLY      	%d \r\n",m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEY4C_TLY);
	LOG_MSG(db_HAL_WARPING," PM_WPKEY4C_TRX      	%d \r\n",m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEY4C_TRX);
	LOG_MSG(db_HAL_WARPING," PM_WPKEY4C_TRY      	%d \r\n",m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEY4C_TRY);
	LOG_MSG(db_HAL_WARPING," PM_WPKEY4C_TRX      	%d \r\n",m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEY4C_BLX);
	LOG_MSG(db_HAL_WARPING," PM_WPKEY4C_TRY      	%d \r\n",m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEY4C_BLY);
	LOG_MSG(db_HAL_WARPING," PM_WPKEY4C_BRX      	%d \r\n",m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEY4C_BRX);
	LOG_MSG(db_HAL_WARPING," PM_WPKEY4C_BRY      	%d \r\n",m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEY4C_BRY);
	LOG_MSG(db_HAL_WARPING," m_cWarpAutoFilter      	%d \r\n",m_stWarpConfig.stOsdBasic.stWArpGeoPara.m_cWarpAutoFilter);
	LOG_MSG(db_HAL_WARPING," m_cWarpFilterSelect_H   %d \r\n",m_stWarpConfig.stOsdBasic.stWArpGeoPara.m_cWarpFilterSelect_H);
	LOG_MSG(db_HAL_WARPING," m_cWarpFilterSelect_V   %d \r\n",m_stWarpConfig.stOsdBasic.stWArpGeoPara.m_cWarpFilterSelect_V);
	LOG_MSG(db_HAL_WARPING," m_cWarpAutoFilterAp      	%d \r\n",m_stWarpConfig.stOsdBasic.stWArpGeoPara.m_cWarpAutoFilterAp);  //A35G2_Simon_0110
	LOG_MSG(db_HAL_WARPING," m_cWarpFilterApValue_H   %d \r\n",m_stWarpConfig.stOsdBasic.stWArpGeoPara.m_cWarpFilterApValue_H);  //A35G2_Simon_0110
	LOG_MSG(db_HAL_WARPING," m_cWarpFilterApValue_V   %d \r\n",m_stWarpConfig.stOsdBasic.stWArpGeoPara.m_cWarpFilterApValue_V);  //A35G2_Simon_0110

}

void utilBasicBlendSettingGet(sBLENDING_BASIC *stBlendingBasic)
{
	memcpy(stBlendingBasic,&m_stWarpConfig.stOsdBasic.stBlendingPara, sizeof(sBLENDING_BASIC));

	LOG_MSG(db_HAL_WARPING," ucBlending_T_Enable     %d \r\n",m_stWarpConfig.stOsdBasic.stBlendingPara.ucBlending_T_Enable);
	LOG_MSG(db_HAL_WARPING," ucBlending_B_Enable     %d \r\n",m_stWarpConfig.stOsdBasic.stBlendingPara.ucBlending_B_Enable);
	LOG_MSG(db_HAL_WARPING," ucBlending_L_Enable     %d \r\n",m_stWarpConfig.stOsdBasic.stBlendingPara.ucBlending_L_Enable);
	LOG_MSG(db_HAL_WARPING," ucBlending_R_Enable     %d \r\n",m_stWarpConfig.stOsdBasic.stBlendingPara.ucBlending_R_Enable);
	LOG_MSG(db_HAL_WARPING," uiBlending_T_St     	%d \r\n",m_stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_T_St);
	LOG_MSG(db_HAL_WARPING," uiBlending_B_St     	%d \r\n",m_stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_B_St);
	LOG_MSG(db_HAL_WARPING," uiBlending_L_St     	%d \r\n",m_stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_L_St);
	LOG_MSG(db_HAL_WARPING," uiBlending_R_St     	%d \r\n",m_stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_R_St);
	LOG_MSG(db_HAL_WARPING," uiBlending_T_Width     	%d \r\n",m_stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_T_Width);
	LOG_MSG(db_HAL_WARPING," uiBlending_B_Width     	%d \r\n",m_stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_B_Width);
	LOG_MSG(db_HAL_WARPING," uiBlending_L_Width     	%d \r\n",m_stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_L_Width);
	LOG_MSG(db_HAL_WARPING," uiBlending_R_Width     	%d \r\n",m_stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_R_Width);
	LOG_MSG(db_HAL_WARPING," ucBlendingGamma         %d \r\n",m_stWarpConfig.stOsdBasic.stBlendingPara.ucBlendingGamma);
}


//save Warp & Blend
uint8 utilWarp_SaveMemoryPreset(uint8 nIdx)					//G100_Doulas_0027
{
#ifdef WARP_SAVE_FILE_SYSTEM
	WARP_CONFIG_FLASH stWrite;
	WARP_CONFIG_FLASH stRead;
	uint16 nDataSize = sizeof(WARP_CONFIG_FLASH);
	UINT8  ucResult;
	//uint32 nFlashAddr = 0;
	//uint16 i = 0;

	if(nIdx>=OSD_PRESET_INVALID)
		return FLASH_ACCESS__PRESET_IDX_ERR;

	//nFlashAddr = OSD_WARP_PRESET_FLASH_ADDR+0x1000*nIdx;

	m_stWarpConfig.eWarpCtrl = WARP_CTRL__ADVANCED;

	//construct data
	memcpy(&(stWrite.pnHeader), m_pnHeader, sizeof(m_pnHeader));
	//memcpy(&(stWrite.stOsdWarp), &m_stWarpConfig.stOsd, sizeof(OSD_WARP));
	//stWrite.nChecksum = CalcChecksum((uint8*)(&stWrite), sizeof(OSD_WARP_FLASH)-sizeof(stWrite.nChecksum));
	memcpy(&(stWrite.stWarpConfig), &m_stWarpConfig, sizeof(WARP_CONFIG));
	stWrite.nChecksum = CalcChecksum((uint8*)(&stWrite), sizeof(WARP_CONFIG_FLASH)-sizeof(stWrite.nChecksum));
	LOG_MSG(db_HAL_WARPING, "123sm (%d)\r\n",m_stWarpConfig.stOsdBasic.stWArpGeoPara.m_cWarpAutoFilter);	//G100_Doulas_0047
	ucResult = utilWarp_ADVMemoryWarpSaveFile(&stWrite,nDataSize,nIdx);
	if(ucResult != FLASH_ACCESS__PASS)
	{
		return ucResult;
	}

	//write data to C789 DRAM
	//WriteC789Ram((uint8*)(&stWrite), RAM_DATA_ADDR, nDataSize);

	//erase Flash
	//if(!EraseFlashSector(nFlashAddr, FLASH_ERASE_MODE__SE4K))
    //    return FLASH_ACCESS__ERASE_FLASH_ERR;

	//write data to FLASH by DMA
	//if(!Dram_Reg2Flash(RAM_DATA_ADDR, nFlashAddr, nDataSize))
    //    return FLASH_ACCESS__RAM_TO_FLASH_ERR;

	//read data from FLASH by DMA
	//if(!Dram_Flash2Reg(RAM_DATA_ADDR, nFlashAddr, nDataSize))
    //    return FLASH_ACCESS__FLASH_TO_RAM_ERR;

	//read data from C789 DRAM
	//ReadC789Ram((uint8*)(&stRead), RAM_DATA_ADDR, nDataSize);
	ucResult = utilWarp_ADVMemoryWarpReloadFile(&stRead,nIdx);
	if(ucResult != FLASH_ACCESS__PASS)
	{
		return ucResult;
	}

	// compare
	if(memcmp((uint8*)(&stWrite), (uint8*)(&stRead), nDataSize))
        return FLASH_ACCESS__COMPARE_ERR;

    return FLASH_ACCESS__PASS;
#else
	WARP_CONFIG_FLASH stWrite;
	WARP_CONFIG_FLASH stRead;
	uint16 nDataSize = sizeof(WARP_CONFIG_FLASH);
	uint32 nFlashAddr = 0;
	uint16 i = 0;

	if(nIdx>=OSD_PRESET_INVALID)
		return FLASH_ACCESS__PRESET_IDX_ERR;

	nFlashAddr = OSD_WARP_PRESET_FLASH_ADDR+0x1000*nIdx;

	//construct data
	memcpy(&(stWrite.pnHeader), m_pnHeader, sizeof(m_pnHeader));
	//memcpy(&(stWrite.stOsdWarp), &m_stWarpConfig.stOsd, sizeof(OSD_WARP));
	//stWrite.nChecksum = CalcChecksum((uint8*)(&stWrite), sizeof(OSD_WARP_FLASH)-sizeof(stWrite.nChecksum));
	memcpy(&(stWrite.stWarpConfig), &m_stWarpConfig, sizeof(WARP_CONFIG));
	stWrite.nChecksum = CalcChecksum((uint8*)(&stWrite), sizeof(WARP_CONFIG_FLASH)-sizeof(stWrite.nChecksum));

	//write data to C789 DRAM
	WriteC789Ram((uint8*)(&stWrite), RAM_DATA_ADDR, nDataSize);

	//erase Flash
	if(!EraseFlashSector(nFlashAddr, FLASH_ERASE_MODE__SE4K))
        return FLASH_ACCESS__ERASE_FLASH_ERR;

	//write data to FLASH by DMA
	if(!Dram_Reg2Flash(RAM_DATA_ADDR, nFlashAddr, nDataSize))
        return FLASH_ACCESS__RAM_TO_FLASH_ERR;

	//read data from FLASH by DMA
	if(!Dram_Flash2Reg(RAM_DATA_ADDR, nFlashAddr, nDataSize))
        return FLASH_ACCESS__FLASH_TO_RAM_ERR;

	//read data from C789 DRAM
	ReadC789Ram((uint8*)(&stRead), RAM_DATA_ADDR, nDataSize);

	// compare
	if(memcmp((uint8*)(&stWrite), (uint8*)(&stRead), nDataSize))
        return FLASH_ACCESS__COMPARE_ERR;

    return FLASH_ACCESS__PASS;
#endif

}


//only save warp (keep blend setting)
uint8 utilWarp_SaveBasicWarpMemoryPreset(uint8 nIdx)
{
	WARP_CONFIG_FLASH stWrite;
	WARP_CONFIG_FLASH stRead;
	uint16 nDataSize = sizeof(WARP_CONFIG_FLASH);
	UINT8  ucResult;
	//uint32 nFlashAddr = 0;
	//uint16 i = 0;

	if(nIdx>=OSD_PRESET_INVALID)
		return FLASH_ACCESS__PRESET_IDX_ERR;

	//construct data
    //header
	memcpy(&(stWrite.pnHeader), m_pnHeader, sizeof(m_pnHeader));

	//load origin data
	WARP_CONFIG_FLASH stPrevios;
	ucResult = utilWarp_ADVMemoryWarpReloadFile(&stPrevios, nIdx);
	if(ucResult == FLASH_ACCESS__PASS)
	{
	    //load origin data
        memcpy(&(stWrite.stWarpConfig), &(stPrevios.stWarpConfig), sizeof(stPrevios.stWarpConfig));

        //replace OSD_BASIC_WARP->sWARP_BASIC data
        memcpy(&(stWrite.stWarpConfig.stOsdBasic.stWArpGeoPara), &m_stWarpConfig.stOsdBasic.stWArpGeoPara , sizeof(m_stWarpConfig.stOsdBasic.stWArpGeoPara));

        //change warp ctrl to Basic
        stWrite.stWarpConfig.eWarpCtrl = WARP_CTRL__BASIC;
	}
	else  //no file or can not read origin data (first time to save)
	{
        memcpy(&(stWrite.stWarpConfig), &m_stWarpConfig, sizeof(WARP_CONFIG));

        stWrite.stWarpConfig.stOsdBasic.stBlendingPara.ucBlending_T_Enable = 0;
        stWrite.stWarpConfig.stOsdBasic.stBlendingPara.ucBlending_B_Enable = 0;
        stWrite.stWarpConfig.stOsdBasic.stBlendingPara.ucBlending_L_Enable = 0;
        stWrite.stWarpConfig.stOsdBasic.stBlendingPara.ucBlending_R_Enable = 0;
        stWrite.stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_T_St = 0;
        stWrite.stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_B_St = 0;
        stWrite.stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_L_St = 0;
        stWrite.stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_R_St = 0;
        stWrite.stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_T_Width = 0;
        stWrite.stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_B_Width = 0;
        stWrite.stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_L_Width = 0;
        stWrite.stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_R_Width = 0;
        stWrite.stWarpConfig.stOsdBasic.stBlendingPara.ucBlendingGamma = eCM_BLENDING_GAMMA_2_2;
        stWrite.stWarpConfig.stOsdBasic.stBlendingPara.ulEGBCT = 0;
	}

    //Cal checksum
	stWrite.nChecksum = CalcChecksum((uint8*)(&stWrite), sizeof(WARP_CONFIG_FLASH)-sizeof(stWrite.nChecksum));

	LOG_MSG(db_HAL_WARPING, "123sm (%d)\r\n",m_stWarpConfig.stOsdBasic.stWArpGeoPara.m_cWarpAutoFilter);	//G100_Doulas_0047
	ucResult = utilWarp_ADVMemoryWarpSaveFile(&stWrite,nDataSize,nIdx);
	if(ucResult != FLASH_ACCESS__PASS)
	{
		return ucResult;
	}

	ucResult = utilWarp_ADVMemoryWarpReloadFile(&stRead,nIdx);
	if(ucResult != FLASH_ACCESS__PASS)
	{
		return ucResult;
	}

	// compare
	if(memcmp((uint8*)(&stWrite), (uint8*)(&stRead), nDataSize))
        return FLASH_ACCESS__COMPARE_ERR;

    return FLASH_ACCESS__PASS;

}

uint8 utilWarp_SaveBasicBlendMemoryPreset(uint8 nIdx)
{
	WARP_CONFIG_FLASH stWrite;
	WARP_CONFIG_FLASH stRead;
	uint16 nDataSize = sizeof(WARP_CONFIG_FLASH);
	UINT8  ucResult;
	//uint32 nFlashAddr = 0;
	//uint16 i = 0;

	if(nIdx>=OSD_PRESET_INVALID)
	{
		LOG_MSG(db_HAL_WARPING, "Geo File Access Error (%d)\r\n", __LINE__);
		return FLASH_ACCESS__PRESET_IDX_ERR;
	}

	//construct data
    //header
	memcpy(&(stWrite.pnHeader), m_pnHeader, sizeof(m_pnHeader));

	//load origin data
	WARP_CONFIG_FLASH stPrevios;
	ucResult = utilWarp_ADVMemoryWarpReloadFile(&stPrevios, nIdx);

	LOG_MSG(db_HAL_WARPING,"\r\n\r\n stPrevios\r\n");
	LOG_MSG(db_HAL_WARPING," Pre ucBlending_T_Enable     %d \r\n",stPrevios.stWarpConfig.stOsdBasic.stBlendingPara.ucBlending_T_Enable);
	LOG_MSG(db_HAL_WARPING," Pre ucBlending_B_Enable     %d \r\n",stPrevios.stWarpConfig.stOsdBasic.stBlendingPara.ucBlending_B_Enable);
	LOG_MSG(db_HAL_WARPING," Pre ucBlending_L_Enable     %d \r\n",stPrevios.stWarpConfig.stOsdBasic.stBlendingPara.ucBlending_L_Enable);
	LOG_MSG(db_HAL_WARPING," Pre ucBlending_R_Enable     %d \r\n",stPrevios.stWarpConfig.stOsdBasic.stBlendingPara.ucBlending_R_Enable);
	LOG_MSG(db_HAL_WARPING," Pre uiBlending_T_St     	%d \r\n",stPrevios.stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_T_St);
	LOG_MSG(db_HAL_WARPING," Pre uiBlending_B_St     	%d \r\n",stPrevios.stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_B_St);
	LOG_MSG(db_HAL_WARPING," Pre uiBlending_L_St     	%d \r\n",stPrevios.stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_L_St);
	LOG_MSG(db_HAL_WARPING," Pre uiBlending_R_St     	%d \r\n",stPrevios.stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_R_St);
	LOG_MSG(db_HAL_WARPING," Pre uiBlending_T_Width     	%d \r\n",stPrevios.stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_T_Width);
	LOG_MSG(db_HAL_WARPING," Pre uiBlending_B_Width     	%d \r\n",stPrevios.stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_B_Width);
	LOG_MSG(db_HAL_WARPING," Pre uiBlending_L_Width     	%d \r\n",stPrevios.stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_L_Width);
	LOG_MSG(db_HAL_WARPING," Pre uiBlending_R_Width     	%d \r\n",stPrevios.stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_R_Width);
	LOG_MSG(db_HAL_WARPING," Pre ucBlendingGamma         %d \r\n",stPrevios.stWarpConfig.stOsdBasic.stBlendingPara.ucBlendingGamma);

	if(ucResult == FLASH_ACCESS__PASS)
	{
	    //load origin data
        memcpy(&(stWrite.stWarpConfig), &(stPrevios.stWarpConfig), sizeof(stPrevios.stWarpConfig));

        //replace OSD_BASIC_WARP->sWARP_BASIC data
        memcpy(&(stWrite.stWarpConfig.stOsdBasic.stBlendingPara), &m_stWarpConfig.stOsdBasic.stBlendingPara , sizeof(m_stWarpConfig.stOsdBasic.stBlendingPara));

        //change warp ctrl to Basic
        stWrite.stWarpConfig.eWarpCtrl = WARP_CTRL__BASIC;
	}
	else  //no file or can not read origin data (first time to save)
	{
	    LOG_MSG(db_HAL_WARPING, "First time to save memory %d\r\n", nIdx);
        memcpy(&(stWrite.stWarpConfig), &m_stWarpConfig, sizeof(WARP_CONFIG));

        stWrite.stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEYANG_H = 0;
        stWrite.stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEYANG_V = 0;
        stWrite.stWarpConfig.stOsdBasic.stWArpGeoPara.WPKEYANG_H_OLD = 0;
        stWrite.stWarpConfig.stOsdBasic.stWArpGeoPara.WPKEYANG_V_OLD = 0;
        stWrite.stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPPINWID_V = 0;
        stWrite.stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPPINWID_V = 0;
        stWrite.stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEY4C_TLX = 0;
        stWrite.stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEY4C_TLY = 0;
        stWrite.stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEY4C_TRX = 0;
        stWrite.stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEY4C_TRY = 0;
        stWrite.stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEY4C_BLX = 0;
        stWrite.stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEY4C_BLY = 0;
        stWrite.stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEY4C_BRX = 0;
        stWrite.stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEY4C_BRY = 0;
        stWrite.stWarpConfig.stOsdBasic.stWArpGeoPara.TABLE_COLOR = 0;
        stWrite.stWarpConfig.stOsdBasic.stWArpGeoPara.POLATION_H = DEFAULT_WAPR_FILTER_LUT_H; //A35G2_Simon_0115
        stWrite.stWarpConfig.stOsdBasic.stWArpGeoPara.POLATION_V = DEFAULT_WAPR_FILTER_LUT_V;
        stWrite.stWarpConfig.stOsdBasic.stWArpGeoPara.m_cWarpFilterSelect_H = DEFAULT_WAPR_FILTER_LUT_H;  //A35G2_Simon_0115
        stWrite.stWarpConfig.stOsdBasic.stWArpGeoPara.m_cWarpFilterSelect_V = DEFAULT_WAPR_FILTER_LUT_V;
        stWrite.stWarpConfig.stOsdBasic.stWArpGeoPara.m_cWarpAutoFilter = 1;
        #if (WARPING_AP_AUTO_FILTER == TRUE)      //A35G2_Simon_0110
        {
            stWrite.stWarpConfig.stOsdBasic.stWArpGeoPara.m_cWarpFilterApValue_H = DEFAULT_WAPR_FILTER_LUT_H;
            stWrite.stWarpConfig.stOsdBasic.stWArpGeoPara.m_cWarpFilterApValue_V = DEFAULT_WAPR_FILTER_LUT_V;
            stWrite.stWarpConfig.stOsdBasic.stWArpGeoPara.m_cWarpAutoFilterAp = TRUE;
        }
        #else
        {
            stWrite.stWarpConfig.stOsdBasic.stWArpGeoPara.m_cWarpFilterApValue_H = DEFAULT_WAPR_FILTER_LUT_H;
            stWrite.stWarpConfig.stOsdBasic.stWArpGeoPara.m_cWarpFilterApValue_V = DEFAULT_WAPR_FILTER_LUT_V;
            stWrite.stWarpConfig.stOsdBasic.stWArpGeoPara.m_cWarpAutoFilterAp = FALSE;
        }
        #endif
	}

	LOG_MSG(db_HAL_WARPING,"\r\n\r\n stWrite\r\n");
	LOG_MSG(db_HAL_WARPING," stWrite ucBlending_T_Enable     %d \r\n",stWrite.stWarpConfig.stOsdBasic.stBlendingPara.ucBlending_T_Enable);
	LOG_MSG(db_HAL_WARPING," stWrite ucBlending_B_Enable     %d \r\n",stWrite.stWarpConfig.stOsdBasic.stBlendingPara.ucBlending_B_Enable);
	LOG_MSG(db_HAL_WARPING," stWrite ucBlending_L_Enable     %d \r\n",stWrite.stWarpConfig.stOsdBasic.stBlendingPara.ucBlending_L_Enable);
	LOG_MSG(db_HAL_WARPING," stWrite ucBlending_R_Enable     %d \r\n",stWrite.stWarpConfig.stOsdBasic.stBlendingPara.ucBlending_R_Enable);
	LOG_MSG(db_HAL_WARPING," stWrite uiBlending_T_St     	%d \r\n",stWrite.stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_T_St);
	LOG_MSG(db_HAL_WARPING," stWrite uiBlending_B_St     	%d \r\n",stWrite.stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_B_St);
	LOG_MSG(db_HAL_WARPING," stWrite uiBlending_L_St     	%d \r\n",stWrite.stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_L_St);
	LOG_MSG(db_HAL_WARPING," stWrite uiBlending_R_St     	%d \r\n",stWrite.stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_R_St);
	LOG_MSG(db_HAL_WARPING," stWrite uiBlending_T_Width     	%d \r\n",stWrite.stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_T_Width);
	LOG_MSG(db_HAL_WARPING," stWrite uiBlending_B_Width     	%d \r\n",stWrite.stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_B_Width);
	LOG_MSG(db_HAL_WARPING," stWrite uiBlending_L_Width     	%d \r\n",stWrite.stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_L_Width);
	LOG_MSG(db_HAL_WARPING," stWrite uiBlending_R_Width     	%d \r\n",stWrite.stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_R_Width);
	LOG_MSG(db_HAL_WARPING," stWrite ucBlendingGamma         %d \r\n",stWrite.stWarpConfig.stOsdBasic.stBlendingPara.ucBlendingGamma);

    //Cal checksum
	stWrite.nChecksum = CalcChecksum((uint8*)(&stWrite), sizeof(WARP_CONFIG_FLASH)-sizeof(stWrite.nChecksum));

	LOG_MSG(db_HAL_WARPING, "123sm (%d)\r\n",m_stWarpConfig.stOsdBasic.stWArpGeoPara.m_cWarpAutoFilter);	//G100_Doulas_0047
	ucResult = utilWarp_ADVMemoryWarpSaveFile(&stWrite,nDataSize,nIdx);
	if(ucResult != FLASH_ACCESS__PASS)
	{
		LOG_MSG(db_HAL_WARPING, "Geo File Access Error (%d)\r\n", __LINE__);
		return ucResult;
	}

	ucResult = utilWarp_ADVMemoryWarpReloadFile(&stRead,nIdx);
	if(ucResult != FLASH_ACCESS__PASS)
	{
		LOG_MSG(db_HAL_WARPING, "Geo File Access Error (%d)\r\n", __LINE__);
		return ucResult;
	}

	// compare
	if(memcmp((uint8*)(&stWrite), (uint8*)(&stRead), nDataSize))
	{
		LOG_MSG(db_HAL_WARPING, "Geo File Access Error (%d)\r\n", __LINE__);
        return FLASH_ACCESS__COMPARE_ERR;
    }

    return FLASH_ACCESS__PASS;

}


uint8 utilWarp_LoadMemoryPreset(uint8 nIdx)			//G100_Doulas_0027
{
#ifdef WARP_SAVE_FILE_SYSTEM
	WARP_CONFIG_FLASH stRead;
	WARP_CONFIG stWarpConfigBackup;
	uint16 nDataSize = sizeof(WARP_CONFIG_FLASH);
	UINT8  ucResult;
	//uint32 nFlashAddr = 0;

	if(nIdx>=OSD_PRESET_INVALID)
		return FLASH_ACCESS__PRESET_IDX_ERR;

	//nFlashAddr = OSD_WARP_PRESET_FLASH_ADDR+0x1000*nIdx;

	//read data from FLASH by DMA
	//if(!Dram_Flash2Reg(RAM_DATA_ADDR, nFlashAddr, nDataSize))
    //    return FLASH_ACCESS__FLASH_TO_RAM_ERR;

	//read data from C789 DRAM
	//ReadC789Ram((uint8*)(&stRead), RAM_DATA_ADDR, nDataSize);
	ucResult = utilWarp_ADVMemoryWarpReloadFile(&stRead,nIdx);
	if(ucResult != FLASH_ACCESS__PASS)
	{
		return ucResult;
	}
	LOG_MSG(db_HAL_WARPING, "123lm (%d)\r\n",m_stWarpConfig.stOsdBasic.stWArpGeoPara.m_cWarpAutoFilter);	//G100_Doulas_0047
	//verify header
    if (0 != memcmp(stRead.pnHeader, m_pnHeader, sizeof(m_pnHeader)))
		return FLASH_ACCESS__VERIFY_HEADER_ERR;

	//verify checksum
	if(stRead.nChecksum != CalcChecksum((uint8*)(&stRead), sizeof(WARP_CONFIG_FLASH)-sizeof(stRead.nChecksum)))
		return FLASH_ACCESS__VERIFY_CHECKSUM_ERR;

	//backup current warp config
	memcpy(&stWarpConfigBackup, &m_stWarpConfig, sizeof(WARP_CONFIG));

	//load new osd warp
	memcpy(&m_stWarpConfig.stOsd, &(stRead.stWarpConfig.stOsd), sizeof(OSD_WARP));
	//load new osd basicwarp
	memcpy(&m_stWarpConfig.stOsdBasic, &(stRead.stWarpConfig.stOsdBasic), sizeof(OSD_BASIC_WARP));
	//load new osd pc
	memcpy(&m_stWarpConfig.stPc, &(stRead.stWarpConfig.stPc), sizeof(PC_WARP));

	//set warp control to OSD
	m_stWarpConfig.eWarpCtrl = stRead.stWarpConfig.eWarpCtrl;

	//verify new warp config
	if(!VerifyWarpConfig())
	{
		//restore origin warp config
		memcpy(&m_stWarpConfig, &stWarpConfigBackup, sizeof(WARP_CONFIG));
		return FLASH_ACCESS__VERIFY_CONFIG_ERR;
	}


	//apply new warp config
	//m_nSelCtlPointX = 0;
	//m_nSelCtlPointY = 0;
	//InitGridColor();
	InitRunTimeParam();     //A65_OPTOMA_Doulas_0020

	if(m_stWarpConfig.eWarpCtrl != WARP_CTRL__ADVANCED)		//basic return
	{
		return SaveWarpConfig();
	}
	ApplyWarp(APPLY_WARP__BY_CONFIG);

#if (ADVANCED_BLEND == TRUE)
	ApplyBlend(APPLY_BLEND__BY_CONFIG);
#endif

#if (ADVANCED_BLACK_LEVEL == TRUE)
	ApplyBlackLevel(APPLY_BLACKLEVEL__BY_CONFIG);  //A65_OPTOMA_Doulas_0020
#endif

#ifdef SUPPORT_WARP_CONTROL_PC	//kenton_temp_check
//	C821_ApplyBias(APPLY_BLEND__BY_CONFIG);  //A65_OPTOMA_Doulas_0020
#endif

	//save warp config
	return SaveWarpConfig();
#else
	WARP_CONFIG_FLASH stRead;
	WARP_CONFIG stWarpConfigBackup;
	uint16 nDataSize = sizeof(WARP_CONFIG_FLASH);
	uint32 nFlashAddr = 0;

	if(nIdx>=OSD_PRESET_INVALID)
		return FLASH_ACCESS__PRESET_IDX_ERR;

	nFlashAddr = OSD_WARP_PRESET_FLASH_ADDR+0x1000*nIdx;

	//read data from FLASH by DMA
	if(!Dram_Flash2Reg(RAM_DATA_ADDR, nFlashAddr, nDataSize))
        return FLASH_ACCESS__FLASH_TO_RAM_ERR;

	//read data from C789 DRAM
	ReadC789Ram((uint8*)(&stRead), RAM_DATA_ADDR, nDataSize);

	//verify header
    if (0 != memcmp(stRead.pnHeader, m_pnHeader, sizeof(m_pnHeader)))
		return FLASH_ACCESS__VERIFY_HEADER_ERR;

	//verify checksum
	if(stRead.nChecksum != CalcChecksum((uint8*)(&stRead), sizeof(WARP_CONFIG_FLASH)-sizeof(stRead.nChecksum)))
		return FLASH_ACCESS__VERIFY_CHECKSUM_ERR;

	//backup current warp config
	memcpy(&stWarpConfigBackup, &m_stWarpConfig, sizeof(WARP_CONFIG));

	//load new osd warp
	memcpy(&m_stWarpConfig.stOsd, &(stRead.stWarpConfig.stOsd), sizeof(OSD_WARP));
	//load new osd basicwarp
	memcpy(&m_stWarpConfig.stOsdBasic, &(stRead.stWarpConfig.stOsdBasic), sizeof(OSD_BASIC_WARP));
	//load new osd pc
	memcpy(&m_stWarpConfig.stPc, &(stRead.stWarpConfig.stPc), sizeof(PC_WARP));

	//set warp control to OSD
	m_stWarpConfig.eWarpCtrl = stRead.stWarpConfig.eWarpCtrl;

	//verify new warp config
	if(!VerifyWarpConfig())
	{
		//restore origin warp config
		memcpy(&m_stWarpConfig, &stWarpConfigBackup, sizeof(WARP_CONFIG));
		return FLASH_ACCESS__VERIFY_CONFIG_ERR;
	}


	//apply new warp config
	m_nSelCtlPointX = 0;
	m_nSelCtlPointY = 0;
	InitGridColor();

	if(m_stWarpConfig.eWarpCtrl != WARP_CTRL__ADVANCED)		//basic return
	{
		return SaveWarpConfig();	//save warp config
	}
	ApplyWarp(APPLY_WARP__BY_CONFIG);
	ApplyBlend(APPLY_BLEND__BY_CONFIG);

#ifdef SUPPORT_WARP_CONTROL_PC	//kenton_temp_check
//	C821_ApplyBias(APPLY_BLEND__BY_CONFIG);  //A65_OPTOMA_Doulas_0020
#endif

	//save warp config
	return SaveWarpConfig();
#endif
}


uint8 utilWarp_LoadBasicWarpMemoryPreset(uint8 nIdx)
{
	WARP_CONFIG_FLASH stRead;
	WARP_CONFIG stWarpConfigBackup;
	uint16 nDataSize = sizeof(WARP_CONFIG_FLASH);
	UINT8  ucResult;
	//uint32 nFlashAddr = 0;

	if(nIdx>=OSD_PRESET_INVALID)
		return FLASH_ACCESS__PRESET_IDX_ERR;

	ucResult = utilWarp_ADVMemoryWarpReloadFile(&stRead,nIdx);
	if(ucResult != FLASH_ACCESS__PASS)
	{
		return ucResult;
	}
	LOG_MSG(db_HAL_WARPING, "123lm (%d)\r\n",m_stWarpConfig.stOsdBasic.stWArpGeoPara.m_cWarpAutoFilter);	//G100_Doulas_0047
	//verify header
    if (0 != memcmp(stRead.pnHeader, m_pnHeader, sizeof(m_pnHeader)))
		return FLASH_ACCESS__VERIFY_HEADER_ERR;

	//verify checksum
	if(stRead.nChecksum != CalcChecksum((uint8*)(&stRead), sizeof(WARP_CONFIG_FLASH)-sizeof(stRead.nChecksum)))
		return FLASH_ACCESS__VERIFY_CHECKSUM_ERR;

	//backup current warp config
	memcpy(&stWarpConfigBackup, &m_stWarpConfig, sizeof(WARP_CONFIG));

	//load new osd warp
	memcpy(&m_stWarpConfig.stOsd, &(stRead.stWarpConfig.stOsd), sizeof(OSD_WARP));
	//only load new osd basic warp
	memcpy(&m_stWarpConfig.stOsdBasic.stWArpGeoPara, &(stRead.stWarpConfig.stOsdBasic.stWArpGeoPara), sizeof(sWARP_BASIC));
	//load new osd pc
	memcpy(&m_stWarpConfig.stPc, &(stRead.stWarpConfig.stPc), sizeof(PC_WARP));

	//set warp control to OSD
	m_stWarpConfig.eWarpCtrl = stRead.stWarpConfig.eWarpCtrl;

	//verify new warp config
	if(!VerifyWarpConfig())
	{
		//restore origin warp config
		memcpy(&m_stWarpConfig, &stWarpConfigBackup, sizeof(WARP_CONFIG));
		return FLASH_ACCESS__VERIFY_CONFIG_ERR;
	}


	//apply new warp config
	m_nSelCtlPointX = 0;
	m_nSelCtlPointY = 0;
	InitGridColor();

#if 0
	if(m_stWarpConfig.eWarpCtrl == WARP_CTRL__BASIC)		//basic return
	{
		return SaveWarpConfig();
	}
	else if(m_stWarpConfig.eWarpCtrl == WARP_CTRL__ADVANCED)
	{
    	ApplyWarp(APPLY_WARP__BY_CONFIG);

    	#if 0
    	ApplyBlend(APPLY_BLEND__BY_CONFIG);
    	#endif
    }


#ifdef SUPPORT_WARP_CONTROL_PC	//kenton_temp_check
//	C821_ApplyBias(APPLY_BLEND__BY_CONFIG);
#endif
#endif


	//save warp config
	return SaveWarpConfig();

}


uint8 utilWarp_LoadBasicBlendMemoryPreset(uint8 nIdx)
{
	WARP_CONFIG_FLASH stRead;
	WARP_CONFIG stWarpConfigBackup;
	uint16 nDataSize = sizeof(WARP_CONFIG_FLASH);
	UINT8  ucResult;
	//uint32 nFlashAddr = 0;

	if(nIdx>=OSD_PRESET_INVALID)
	{
		LOG_MSG(db_HAL_WARPING, "Geo File Access Error (%d)\r\n", __LINE__);
		return FLASH_ACCESS__PRESET_IDX_ERR;
	}

	ucResult = utilWarp_ADVMemoryWarpReloadFile(&stRead,nIdx);
	if(ucResult != FLASH_ACCESS__PASS)
	{
		LOG_MSG(db_HAL_WARPING, "Geo File Access Error (%d)\r\n", __LINE__);
		return ucResult;
	}
	LOG_MSG(db_HAL_WARPING, "123lm (%d)\r\n",m_stWarpConfig.stOsdBasic.stWArpGeoPara.m_cWarpAutoFilter);	//G100_Doulas_0047
	//verify header
    if (0 != memcmp(stRead.pnHeader, m_pnHeader, sizeof(m_pnHeader)))
    {
		LOG_MSG(db_HAL_WARPING, "Geo File Access Error (%d)\r\n", __LINE__);
		return FLASH_ACCESS__VERIFY_HEADER_ERR;
	}

	//verify checksum
	if(stRead.nChecksum != CalcChecksum((uint8*)(&stRead), sizeof(WARP_CONFIG_FLASH)-sizeof(stRead.nChecksum)))
    {
		LOG_MSG(db_HAL_WARPING, "Geo File Access Error (%d)\r\n", __LINE__);
		return FLASH_ACCESS__VERIFY_CHECKSUM_ERR;
    }

	LOG_MSG(db_HAL_WARPING,"\r\n\r\n stPrevios\r\n");
	LOG_MSG(db_HAL_WARPING," stRead ucBlending_T_Enable     %d \r\n",stRead.stWarpConfig.stOsdBasic.stBlendingPara.ucBlending_T_Enable);
	LOG_MSG(db_HAL_WARPING," stRead ucBlending_B_Enable     %d \r\n",stRead.stWarpConfig.stOsdBasic.stBlendingPara.ucBlending_B_Enable);
	LOG_MSG(db_HAL_WARPING," stRead ucBlending_L_Enable     %d \r\n",stRead.stWarpConfig.stOsdBasic.stBlendingPara.ucBlending_L_Enable);
	LOG_MSG(db_HAL_WARPING," stRead ucBlending_R_Enable     %d \r\n",stRead.stWarpConfig.stOsdBasic.stBlendingPara.ucBlending_R_Enable);
	LOG_MSG(db_HAL_WARPING," stRead uiBlending_T_St     	%d \r\n",stRead.stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_T_St);
	LOG_MSG(db_HAL_WARPING," stRead uiBlending_B_St     	%d \r\n",stRead.stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_B_St);
	LOG_MSG(db_HAL_WARPING," stRead uiBlending_L_St     	%d \r\n",stRead.stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_L_St);
	LOG_MSG(db_HAL_WARPING," stRead uiBlending_R_St     	%d \r\n",stRead.stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_R_St);
	LOG_MSG(db_HAL_WARPING," stRead uiBlending_T_Width     	%d \r\n",stRead.stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_T_Width);
	LOG_MSG(db_HAL_WARPING," stRead uiBlending_B_Width     	%d \r\n",stRead.stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_B_Width);
	LOG_MSG(db_HAL_WARPING," stRead uiBlending_L_Width     	%d \r\n",stRead.stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_L_Width);
	LOG_MSG(db_HAL_WARPING," stRead uiBlending_R_Width     	%d \r\n",stRead.stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_R_Width);
	LOG_MSG(db_HAL_WARPING," stRead ucBlendingGamma         %d \r\n",stRead.stWarpConfig.stOsdBasic.stBlendingPara.ucBlendingGamma);


	//backup current warp config
	memcpy(&stWarpConfigBackup, &m_stWarpConfig, sizeof(WARP_CONFIG));

	//load new osd warp
	memcpy(&m_stWarpConfig.stOsd, &(stRead.stWarpConfig.stOsd), sizeof(OSD_WARP));
	//only load new osd basic warp
	memcpy(&m_stWarpConfig.stOsdBasic.stBlendingPara, &(stRead.stWarpConfig.stOsdBasic.stBlendingPara), sizeof(sBLENDING_BASIC));
	//load new osd pc
	memcpy(&m_stWarpConfig.stPc, &(stRead.stWarpConfig.stPc), sizeof(PC_WARP));

	//set warp control to OSD
	m_stWarpConfig.eWarpCtrl = stRead.stWarpConfig.eWarpCtrl;

	//verify new warp config
	if(!VerifyWarpConfig())
	{
		//restore origin warp config
		memcpy(&m_stWarpConfig, &stWarpConfigBackup, sizeof(WARP_CONFIG));
		LOG_MSG(db_HAL_WARPING, "Geo File Access Error (%d)\r\n", __LINE__);
		return FLASH_ACCESS__VERIFY_CONFIG_ERR;
	}

	LOG_MSG(db_HAL_WARPING,"\r\n\r\n stPrevios\r\n");
	LOG_MSG(db_HAL_WARPING," m_stWarpConfig ucBlending_T_Enable     %d \r\n",m_stWarpConfig.stOsdBasic.stBlendingPara.ucBlending_T_Enable);
	LOG_MSG(db_HAL_WARPING," m_stWarpConfig ucBlending_B_Enable     %d \r\n",m_stWarpConfig.stOsdBasic.stBlendingPara.ucBlending_B_Enable);
	LOG_MSG(db_HAL_WARPING," m_stWarpConfig ucBlending_L_Enable     %d \r\n",m_stWarpConfig.stOsdBasic.stBlendingPara.ucBlending_L_Enable);
	LOG_MSG(db_HAL_WARPING," m_stWarpConfig ucBlending_R_Enable     %d \r\n",m_stWarpConfig.stOsdBasic.stBlendingPara.ucBlending_R_Enable);
	LOG_MSG(db_HAL_WARPING," m_stWarpConfig uiBlending_T_St     	%d \r\n",m_stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_T_St);
	LOG_MSG(db_HAL_WARPING," m_stWarpConfig uiBlending_B_St     	%d \r\n",m_stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_B_St);
	LOG_MSG(db_HAL_WARPING," m_stWarpConfig uiBlending_L_St     	%d \r\n",m_stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_L_St);
	LOG_MSG(db_HAL_WARPING," m_stWarpConfig uiBlending_R_St     	%d \r\n",m_stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_R_St);
	LOG_MSG(db_HAL_WARPING," m_stWarpConfig uiBlending_T_Width     	%d \r\n",m_stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_T_Width);
	LOG_MSG(db_HAL_WARPING," m_stWarpConfig uiBlending_B_Width     	%d \r\n",m_stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_B_Width);
	LOG_MSG(db_HAL_WARPING," m_stWarpConfig uiBlending_L_Width     	%d \r\n",m_stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_L_Width);
	LOG_MSG(db_HAL_WARPING," m_stWarpConfig uiBlending_R_Width     	%d \r\n",m_stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_R_Width);
	LOG_MSG(db_HAL_WARPING," m_stWarpConfig ucBlendingGamma         %d \r\n",m_stWarpConfig.stOsdBasic.stBlendingPara.ucBlendingGamma);



	//apply new warp config
	m_nSelCtlPointX = 0;
	m_nSelCtlPointY = 0;
	InitGridColor();

#if 0
	if(m_stWarpConfig.eWarpCtrl == WARP_CTRL__BASIC)		//basic return
	{
		return SaveWarpConfig();
	}
	else if(m_stWarpConfig.eWarpCtrl == WARP_CTRL__ADVANCED)
	{
	    #if 0
    	ApplyWarp(APPLY_WARP__BY_CONFIG);
    	#endif
    	ApplyBlend(APPLY_BLEND__BY_CONFIG);
    }

#ifdef SUPPORT_WARP_CONTROL_PC	//kenton_temp_check
//	C821_ApplyBias(APPLY_BLEND__BY_CONFIG);
#endif
#endif

	//save warp config
	return SaveWarpConfig();

}



void utilWarp_GetADVWarpConfig(OSD_WARP *stADVWarp)		//G100_Doulas_0027
{

	stADVWarp->eGridColor  		= m_stWarpConfig.stOsd.eGridColor;
	//stADVWarp->eCursorColor 	= m_stWarpConfig.stOsd.eCursorColor;
	stADVWarp->eBkgColor 		= m_stWarpConfig.stOsd.eBkgColor;
	stADVWarp->eWarpPoint 		= m_stWarpConfig.stOsd.eWarpPoint;
	stADVWarp->bWarpInnerOn 	= m_stWarpConfig.stOsd.bWarpInnerOn;
	stADVWarp->nWarpSharpness  	= m_stWarpConfig.stOsd.nWarpSharpness;
	stADVWarp->nOverlapGridNum 	= m_stWarpConfig.stOsd.nOverlapGridNum;
	stADVWarp->nBlendGamma     	= m_stWarpConfig.stOsd.nBlendGamma;

	//Black level		//A65_OPTOMA_Doulas_0020
	stADVWarp->stBlackLevel.ucArea  									            = m_stWarpConfig.stOsd.stBlackLevel.ucArea;
	stADVWarp->stBlackLevel.bEnable[BLACKLEVEL_AREA__TOP] 						    = m_stWarpConfig.stOsd.stBlackLevel.bEnable[BLACKLEVEL_AREA__TOP];
	stADVWarp->stBlackLevel.bEnable[BLACKLEVEL_AREA__BOTTOM] 					    = m_stWarpConfig.stOsd.stBlackLevel.bEnable[BLACKLEVEL_AREA__BOTTOM];
	stADVWarp->stBlackLevel.pnColor[BLACKLEVEL_AREA__TOP+1][BLACKLEVEL_BT_RED]  	= m_stWarpConfig.stOsd.stBlackLevel.pnColor[BLACKLEVEL_AREA__TOP+1][BLACKLEVEL_BT_RED];
	stADVWarp->stBlackLevel.pnColor[BLACKLEVEL_AREA__BOTTOM+1][BLACKLEVEL_BT_RED]   = m_stWarpConfig.stOsd.stBlackLevel.pnColor[BLACKLEVEL_AREA__BOTTOM+1][BLACKLEVEL_BT_RED];
	stADVWarp->stBlackLevel.pnColor[BLACKLEVEL_AREA__TOP+1][BLACKLEVEL_BT_GREEN]    = m_stWarpConfig.stOsd.stBlackLevel.pnColor[BLACKLEVEL_AREA__TOP+1][BLACKLEVEL_BT_GREEN];
	stADVWarp->stBlackLevel.pnColor[BLACKLEVEL_AREA__BOTTOM+1][BLACKLEVEL_BT_GREEN] = m_stWarpConfig.stOsd.stBlackLevel.pnColor[BLACKLEVEL_AREA__BOTTOM+1][BLACKLEVEL_BT_GREEN];
	stADVWarp->stBlackLevel.pnColor[BLACKLEVEL_AREA__TOP+1][BLACKLEVEL_BT_BLUE]     = m_stWarpConfig.stOsd.stBlackLevel.pnColor[BLACKLEVEL_AREA__TOP+1][BLACKLEVEL_BT_BLUE];
	stADVWarp->stBlackLevel.pnColor[BLACKLEVEL_AREA__BOTTOM+1][BLACKLEVEL_BT_BLUE]  = m_stWarpConfig.stOsd.stBlackLevel.pnColor[BLACKLEVEL_AREA__BOTTOM+1][BLACKLEVEL_BT_BLUE];
	stADVWarp->stBlackLevel.ucBoundary  									        = m_stWarpConfig.stOsd.stBlackLevel.ucBoundary;		//A35G2_Owen_0002 //A65_OPTOMA_Doulas_0239
}

void utilWarp_SetADVWarpConfig(OSD_WARP stADVWarp)		//G100_Doulas_0027
{
	if(	(stADVWarp.eGridColor >= GRID_COLOR__GREEN) &&
		(stADVWarp.eGridColor <= GRID_COLOR__CYAN))
	{
		if(m_stWarpConfig.stOsd.eGridColor != stADVWarp.eGridColor)
			m_bOsdWarpParamChanged = TRUE;

		m_stWarpConfig.stOsd.eGridColor = stADVWarp.eGridColor;
		//m_stWarpConfig.stOsd.eCursorColor 		= COLOR_IDX__MAGENTA;
		InitGridColor();
	}

	if((m_stWarpConfig.stOsd.eBkgColor 		 != stADVWarp.eBkgColor) ||
	   (m_stWarpConfig.stOsd.eWarpPoint		 != stADVWarp.eWarpPoint) ||
	   (m_stWarpConfig.stOsd.bWarpInnerOn 	 != stADVWarp.bWarpInnerOn) ||
	   (m_stWarpConfig.stOsd.nWarpSharpness  != stADVWarp.nWarpSharpness) ||
	   (m_stWarpConfig.stOsd.nOverlapGridNum != stADVWarp.nOverlapGridNum) ||
	   (m_stWarpConfig.stOsd.nBlendGamma 	 != stADVWarp.nBlendGamma))
		m_bOsdWarpParamChanged = TRUE;

	m_stWarpConfig.stOsd.eBkgColor 			= stADVWarp.eBkgColor;
	m_stWarpConfig.stOsd.eWarpPoint			= stADVWarp.eWarpPoint;
	m_stWarpConfig.stOsd.bWarpInnerOn 		= stADVWarp.bWarpInnerOn;
	m_stWarpConfig.stOsd.nWarpSharpness 	= stADVWarp.nWarpSharpness;
	m_stWarpConfig.stOsd.nOverlapGridNum 	= stADVWarp.nOverlapGridNum;
	m_stWarpConfig.stOsd.nBlendGamma 		= stADVWarp.nBlendGamma;


	if((m_stWarpConfig.stOsd.stBlackLevel.ucArea  											     != stADVWarp.stBlackLevel.ucArea) ||
	   (m_stWarpConfig.stOsd.stBlackLevel.ucBoundary  											 != stADVWarp.stBlackLevel.ucBoundary) ||	//A35G2_Owen_0002 //A65_OPTOMA_Doulas_0239
	   (m_stWarpConfig.stOsd.stBlackLevel.bEnable[BLACKLEVEL_AREA__TOP]  						 != stADVWarp.stBlackLevel.bEnable[BLACKLEVEL_AREA__TOP]) ||
	   (m_stWarpConfig.stOsd.stBlackLevel.bEnable[BLACKLEVEL_AREA__BOTTOM] 					     != stADVWarp.stBlackLevel.bEnable[BLACKLEVEL_AREA__BOTTOM]) ||
	   (m_stWarpConfig.stOsd.stBlackLevel.pnColor[BLACKLEVEL_AREA__TOP+1][BLACKLEVEL_BT_RED]  	 != stADVWarp.stBlackLevel.pnColor[BLACKLEVEL_AREA__TOP+1][BLACKLEVEL_BT_RED]) ||
	   (m_stWarpConfig.stOsd.stBlackLevel.pnColor[BLACKLEVEL_AREA__BOTTOM+1][BLACKLEVEL_BT_RED]  != stADVWarp.stBlackLevel.pnColor[BLACKLEVEL_AREA__BOTTOM+1][BLACKLEVEL_BT_RED]) ||
	   (m_stWarpConfig.stOsd.stBlackLevel.pnColor[BLACKLEVEL_AREA__TOP+1][BLACKLEVEL_BT_GREEN]   != stADVWarp.stBlackLevel.pnColor[BLACKLEVEL_AREA__TOP+1][BLACKLEVEL_BT_GREEN]) ||
	   (m_stWarpConfig.stOsd.stBlackLevel.pnColor[BLACKLEVEL_AREA__BOTTOM+1][BLACKLEVEL_BT_GREEN]!= stADVWarp.stBlackLevel.pnColor[BLACKLEVEL_AREA__BOTTOM+1][BLACKLEVEL_BT_GREEN]) ||
	   (m_stWarpConfig.stOsd.stBlackLevel.pnColor[BLACKLEVEL_AREA__TOP+1][BLACKLEVEL_BT_BLUE]    != stADVWarp.stBlackLevel.pnColor[BLACKLEVEL_AREA__TOP+1][BLACKLEVEL_BT_BLUE]) ||
	   (m_stWarpConfig.stOsd.stBlackLevel.pnColor[BLACKLEVEL_AREA__BOTTOM+1][BLACKLEVEL_BT_BLUE] != stADVWarp.stBlackLevel.pnColor[BLACKLEVEL_AREA__BOTTOM+1][BLACKLEVEL_BT_BLUE])) 	//A65_OPTOMA_Doulas_0020
			m_bOsdWarpParamChanged = TRUE;

	//A65_OPTOMA_Doulas_0020 start
	m_eAreaSelection = stADVWarp.stBlackLevel.ucArea;
	m_stWarpConfig.stOsd.stBlackLevel.ucArea  												 = stADVWarp.stBlackLevel.ucArea;
	m_stWarpConfig.stOsd.stBlackLevel.bEnable[BLACKLEVEL_AREA__TOP]  						 = stADVWarp.stBlackLevel.bEnable[BLACKLEVEL_AREA__TOP];
	m_stWarpConfig.stOsd.stBlackLevel.bEnable[BLACKLEVEL_AREA__BOTTOM] 						 = stADVWarp.stBlackLevel.bEnable[BLACKLEVEL_AREA__BOTTOM];
	m_stWarpConfig.stOsd.stBlackLevel.pnColor[BLACKLEVEL_AREA__TOP+1][BLACKLEVEL_BT_RED]  	 = stADVWarp.stBlackLevel.pnColor[BLACKLEVEL_AREA__TOP+1][BLACKLEVEL_BT_RED];
	m_stWarpConfig.stOsd.stBlackLevel.pnColor[BLACKLEVEL_AREA__BOTTOM+1][BLACKLEVEL_BT_RED]  = stADVWarp.stBlackLevel.pnColor[BLACKLEVEL_AREA__BOTTOM+1][BLACKLEVEL_BT_RED];
	m_stWarpConfig.stOsd.stBlackLevel.pnColor[BLACKLEVEL_AREA__TOP+1][BLACKLEVEL_BT_GREEN]   = stADVWarp.stBlackLevel.pnColor[BLACKLEVEL_AREA__TOP+1][BLACKLEVEL_BT_GREEN];
	m_stWarpConfig.stOsd.stBlackLevel.pnColor[BLACKLEVEL_AREA__BOTTOM+1][BLACKLEVEL_BT_GREEN]= stADVWarp.stBlackLevel.pnColor[BLACKLEVEL_AREA__BOTTOM+1][BLACKLEVEL_BT_GREEN];
	m_stWarpConfig.stOsd.stBlackLevel.pnColor[BLACKLEVEL_AREA__TOP+1][BLACKLEVEL_BT_BLUE]    = stADVWarp.stBlackLevel.pnColor[BLACKLEVEL_AREA__TOP+1][BLACKLEVEL_BT_BLUE];
	m_stWarpConfig.stOsd.stBlackLevel.pnColor[BLACKLEVEL_AREA__BOTTOM+1][BLACKLEVEL_BT_BLUE] = stADVWarp.stBlackLevel.pnColor[BLACKLEVEL_AREA__BOTTOM+1][BLACKLEVEL_BT_BLUE];
	//A65_OPTOMA_Doulas_0020 end
	m_stWarpConfig.stOsd.stBlackLevel.ucBoundary  											 = stADVWarp.stBlackLevel.ucBoundary;	//A35G2_Owen_0002 //A65_OPTOMA_Doulas_0239

	LOG_MSG(db_HAL_WARPING," eWarpPoint      %d \r\n",m_stWarpConfig.stOsd.eWarpPoint);
	LOG_MSG(db_HAL_WARPING," bWarpInnerOn    %d \r\n",m_stWarpConfig.stOsd.bWarpInnerOn);
	LOG_MSG(db_HAL_WARPING," nWarpSharpness  %d \r\n",m_stWarpConfig.stOsd.nWarpSharpness);
	LOG_MSG(db_HAL_WARPING," eGridColor     	%d \r\n",m_stWarpConfig.stOsd.eGridColor);
	LOG_MSG(db_HAL_WARPING," eBkgColor      	%d \r\n",m_stWarpConfig.stOsd.eBkgColor);
	LOG_MSG(db_HAL_WARPING," nOverlapGridNum %d \r\n",m_stWarpConfig.stOsd.nOverlapGridNum);
	LOG_MSG(db_HAL_WARPING," nBlendGamma    	%d \r\n",m_stWarpConfig.stOsd.nBlendGamma);

	if(m_bOsdWarpParamChanged)
	{
		SaveWarpConfig();
		m_bOsdWarpParamChanged = FALSE;
	}
}

INT8 utilWarp_GetFileData(INT8 *cFileName, UINT8 *paucData, UINT32 ulSize)		//G100_Doulas_0027
{
    FILE *pFile = fopen((char *)cFileName, "rb");

    if(pFile == NULL || paucData == NULL || ulSize == 0)
    {
        //ASSERT_ALWAYS();
        return UTILMISC_NO_FILE ;
    }

    memset(paucData, 0, ulSize);
    fread(paucData, 1, ulSize, pFile);

    fclose(pFile);

    return 0;
}

const char *m_cWarpFileName[] =
{
    "ADVWarpMemory0.bin",
    "ADVWarpMemory1.bin",
    "ADVWarpMemory2.bin",
    "ADVWarpMemory3.bin",
    "ADVWarpMemory4.bin",
    "ADVWarpCurrent.bin"
};

#define WARPFILE_NUMBER sizeof(m_cWarpFileName)/sizeof(m_cWarpFileName[0])

UINT8 utilWarp_ADVCurrentWarpReloadFile(void *stRead)		//G100_Doulas_0027
{
#ifndef SIMULATOR_ISCALER
	char cFileName[128] = {'\0'};
    UINT32 ulSize = sizeof(WARP_CONFIG_FLASH);
    UINT8 *pucData = (UINT8 *)malloc(ulSize);

    if(access(ADV_WAPR_PATH, 0) == -1)
    {
        mkdir(ADV_WAPR_PATH, 0777);
    }

    if(pucData == NULL)
    {
        return FLASH_ACCESS__FAIL;
    }

    snprintf(cFileName, 128, "%s/%s\0", ADV_WAPR_PATH, m_cWarpFileName[WARPFILE_NUMBER - 1]);

    if(utilWarp_GetFileData(cFileName, pucData, ulSize) == UTILMISC_NO_FILE)
    {
        free(pucData);
        return FLASH_ACCESS__FAIL;
    }
	WARP_CONFIG_FLASH *sWarpInfo = (WARP_CONFIG_FLASH *)pucData;

	memcpy(stRead, sWarpInfo, sizeof(WARP_CONFIG_FLASH));
	free(pucData);
#endif /* SIMULATOR_ISCALER */
	return FLASH_ACCESS__PASS;
}


UINT8 utilWarp_ADVCurrentWarpSaveFile(void *stWrite,UINT16 uisize)		//G100_Doulas_0027
{
#ifndef SIMULATOR_ISCALER
	FILE *lb;
	char cFileName[128] = {'\0'};

    if(access(ADV_WAPR_PATH, 0) == -1)
    {
        mkdir(ADV_WAPR_PATH, 0777);
    }

    snprintf(cFileName, 128, "%s/%s\0", ADV_WAPR_PATH, m_cWarpFileName[WARPFILE_NUMBER - 1]);

	//if (NULL == (lb = fopen("/mnt/ADVWarpCurrent.bin", "w+")))   //open file
	if (NULL == (lb = fopen(cFileName, "w+")))   //open file
	{
		//LOG_MSG(db_ALWAYS, "save fail\r\n");
		return FLASH_ACCESS__FAIL;
	}

	fwrite( stWrite, uisize, 1, lb);  //write file
	fclose(lb);
	//LOG_MSG(db_ALWAYS, "save ok\r\n");
#endif /* SIMULATOR_ISCALER */
	return FLASH_ACCESS__PASS;
}

UINT8 utilWarp_ADVMemoryWarpReloadFile(void *stRead, UINT8 ucIndex)		//G100_Doulas_0027
{
#ifndef SIMULATOR_ISCALER
	char cFileName[128] = {'\0'};

    UINT32 ulSize = sizeof(WARP_CONFIG_FLASH);
    UINT8 *pucData = (UINT8 *)malloc(ulSize);

    if((pucData == NULL) && (ucIndex > WARPFILE_NUMBER))
    {
        return FLASH_ACCESS__FAIL;
    }

    snprintf((char *)cFileName, 128, "%s/%s\0", ADV_WAPR_PATH, m_cWarpFileName[ucIndex]);

    if(utilWarp_GetFileData(cFileName, pucData, ulSize) == UTILMISC_NO_FILE)		//G100_Doulas_0027
    {
        free(pucData);
        return FLASH_ACCESS__FAIL;
    }

	WARP_CONFIG_FLASH *sWarpInfo = (WARP_CONFIG_FLASH *)pucData;

	memcpy(stRead, sWarpInfo, sizeof(WARP_CONFIG_FLASH));
	free(pucData);
#endif /* SIMULATOR_ISCALER */
	return FLASH_ACCESS__PASS;
}


UINT8 utilWarp_ADVMemoryWarpSaveFile(void *stWrite,UINT16 uisize,UINT8 ucIndex)		//G100_Doulas_0027
{
#ifndef SIMULATOR_ISCALER
	FILE *lb;
	char cFileName[128] = {'\0'};

    if(ucIndex > WARPFILE_NUMBER)
    {
        return FLASH_ACCESS__FAIL;
    }

    snprintf((char *)cFileName, 128, "%s/%s\0", ADV_WAPR_PATH, m_cWarpFileName[ucIndex]);

	if (NULL == (lb = fopen(cFileName, "w+")))   //open file
	{
		//LOG_MSG(db_ALWAYS, "save fail1\r\n");
		return FLASH_ACCESS__FAIL;
	}

	fwrite( stWrite, uisize, 1, lb);  //write file
	fclose(lb);
	//LOG_MSG(db_ALWAYS, "save ok1\r\n");
#endif /* SIMULATOR_ISCALER */
	return FLASH_ACCESS__PASS;
}

UINT8 utilWarp_ADVMemoryWarpClearFile(UINT8 ucIndex)			//G100_Doulas_0027
{
    UINT8 ucResult = FLASH_ACCESS__PASS;
#ifndef SIMULATOR_ISCALER
	char cFileName[128] = {'\0'};

    snprintf((char *)cFileName, 128, "%s/%s\0", ADV_WAPR_PATH, m_cWarpFileName[ucIndex]);

	if(access(cFileName, 0) == 0)	//have file
	{
        remove(cFileName);
	}

#endif /* SIMULATOR_ISCALER */
	return ucResult;
}

UINT8 utilWarp_ADVWarpClearAllFile(void)		//G100_Doulas_0027
{
#ifdef WARP_SAVE_FILE_SYSTEM

#ifndef SIMULATOR_ISCALER
	char cFileName[128] = {'\0'};

    snprintf(cFileName, 128, "%s/%s\0", ADV_WAPR_PATH, m_cWarpFileName[WARPFILE_NUMBER - 1]);

    if(access(cFileName, 0) == 0)	//have file
	{
        remove(cFileName);
	}
	utilWarp_ClearOsdPreset();
#endif /* SIMULATOR_ISCALER */
#else
	//erase current Flash
	if(!EraseFlashSector(WARP_CONFIG_FLASH_ADDR, FLASH_ERASE_MODE__SE4K))
       return FLASH_ACCESS__ERASE_FLASH_ERR;

	utilWarp_ClearOsdPreset();
#endif
}

void utilWarp_Panel_Vstart_Offset_Set(INT16 ucOffset)			//G100_Doulas_0043
{
	PS_PANEL_ACT_VST_OFFSET = ucOffset;
}

INT16 utilWarp_Panel_Vstart_Offset_Get(void)			//G100_Doulas_0043
{
	return PS_PANEL_ACT_VST_OFFSET;
}

UINT8 utilWarp_GridPoint_GetColIndex(void)			//G100_Doulas_0067 //A35G2_BRC_Casper_0046
{
	UINT8 ucData = (DEF_NUM_CUR_MAX_H-1)/(GV_NUM_HGRID-1);
	//LOG_MSG(db_ALWAYS,"utilWarp_Grid_Point_COL_Get = %d,%d\r\n",m_nSelCtlPointX,m_stWarpConfig.stOsd.eWarpPoint);
	return (m_nSelCtlPointX/ucData);
}

UINT8 utilWarp_GridPoint_GetRowIndex(void)			//G100_Doulas_0067
{
	UINT8 ucData = (DEF_NUM_CUR_MAX_V-1)/(GV_NUM_VGRID-1);
	//LOG_MSG(db_ALWAYS,"utilWarp_Grid_Point_ROW_Get = %d,%d\r\n",m_nSelCtlPointY,m_stWarpConfig.stOsd.eWarpPoint);
	return (m_nSelCtlPointY/ucData);
}

//return true : changed
//return false : not changed
BOOL utilWarp_GridPoint_SetColIndex(UINT8 nPointX)
{
	UINT8 ucData = (DEF_NUM_CUR_MAX_H-1)/(GV_NUM_HGRID-1);
	//LOG_MSG(db_ALWAYS,"utilWarp_Grid_Point_COL_Set = %d,%d\r\n",m_nSelCtlPointX,m_stWarpConfig.stOsd.eWarpPoint);
	if(ucData*nPointX > (DEF_NUM_CUR_MAX_H-1))
	{
		return FALSE;
	}
	else
	{
		if(m_stWarpConfig.stOsd.bWarpInnerOn)
		{
			m_nSelCtlPointX = ucData*nPointX;
			if(m_ePatternType == PAT_TYPE__WARP_SEL_CTRL_POINT)
			{
				//utilWarp_ShowOsdPattern(PAT_TYPE__WARP_SEL_CTRL_POINT);   //set row 時再 redraw
			}
		}
		else
		{
			if(((m_nSelCtlPointY == 0) || (m_nSelCtlPointY == (DEF_NUM_CUR_MAX_V-1)))
				||
				((m_nSelCtlPointY != 0) && (m_nSelCtlPointY != (DEF_NUM_CUR_MAX_V-1))
				 && (ucData*nPointX == 0 || ucData*nPointX == (DEF_NUM_CUR_MAX_H-1))))
			{
				m_nSelCtlPointX = ucData*nPointX;
				if(m_ePatternType == PAT_TYPE__WARP_SEL_CTRL_POINT)
				{
					//utilWarp_ShowOsdPattern(PAT_TYPE__WARP_SEL_CTRL_POINT);   //set row 時再 redraw
				}
			}
			else
			{
				return FALSE;
			}
		}
	}
	return TRUE;
}

//return true : changed
//return false : not changed
BOOL utilWarp_GridPoint_SetRowIndex(UINT8 nPointY) //A35G2_BRC_Casper_0046
{
	UINT8 ucData = (DEF_NUM_CUR_MAX_V-1)/(GV_NUM_VGRID-1);
	//LOG_MSG(db_ALWAYS,"utilWarp_Grid_Point_ROW_Set = %d,%d\r\n",m_nSelCtlPointY,m_stWarpConfig.stOsd.eWarpPoint);
	if(ucData*nPointY > (DEF_NUM_CUR_MAX_V-1))
	{
		return FALSE;
	}
	else
	{
		if(m_stWarpConfig.stOsd.bWarpInnerOn)
		{
			m_nSelCtlPointY = ucData*nPointY;
			if(m_ePatternType == PAT_TYPE__WARP_SEL_CTRL_POINT)
			{
				utilWarp_ShowOsdPattern(PAT_TYPE__WARP_SEL_CTRL_POINT);
			}
		}
		else
		{
			if(((m_nSelCtlPointX == 0) || (m_nSelCtlPointX == (DEF_NUM_CUR_MAX_H-1)))
				||
				((m_nSelCtlPointX != 0) && (m_nSelCtlPointX != (DEF_NUM_CUR_MAX_H-1))
				 && (ucData*nPointY == 0 || ucData*nPointY == (DEF_NUM_CUR_MAX_V-1))))
			{
				m_nSelCtlPointY = ucData*nPointY;
				if(m_ePatternType == PAT_TYPE__WARP_SEL_CTRL_POINT)
				{
					utilWarp_ShowOsdPattern(PAT_TYPE__WARP_SEL_CTRL_POINT);
				}
			}
			else
			{
				return FALSE;
			}
		}

	}
	return TRUE;
}

BOOL utilWarp_GridPoint_SetColRowIndex(UINT8 nPointX, UINT8 nPointY)
{
	UINT8 ucData_H = (DEF_NUM_CUR_MAX_H-1)/(GV_NUM_HGRID-1);
	UINT8 ucData_V = (DEF_NUM_CUR_MAX_V-1)/(GV_NUM_VGRID-1);
	//LOG_MSG(db_ALWAYS,"utilWarp_Grid_Point_ROW_Set = %d,%d\r\n",m_nSelCtlPointY,m_stWarpConfig.stOsd.eWarpPoint);
	if(ucData_H*nPointX > (DEF_NUM_CUR_MAX_H-1) ||
	   ucData_V*nPointY > (DEF_NUM_CUR_MAX_V-1))
	{
		return FALSE;
	}
	else
	{
		if(m_stWarpConfig.stOsd.bWarpInnerOn)
		{
			m_nSelCtlPointX = ucData_H*nPointX;
			m_nSelCtlPointY = ucData_V*nPointY;
			if(m_ePatternType == PAT_TYPE__WARP_SEL_CTRL_POINT)
			{
				utilWarp_ShowOsdPattern(PAT_TYPE__WARP_SEL_CTRL_POINT);
			}
		}
		else
		{
			if(
			    (((ucData_H*nPointX == 0) || (ucData_H*nPointX == (DEF_NUM_CUR_MAX_H-1)))
				||
				((ucData_H*nPointX != 0) && (ucData_H*nPointX != (DEF_NUM_CUR_MAX_H-1))
				 && (ucData_V*nPointY == 0 || ucData_V*nPointY == (DEF_NUM_CUR_MAX_V-1))))
				&&
				(((ucData_V*nPointY == 0) || (ucData_V*nPointY == (DEF_NUM_CUR_MAX_V-1)))
				||
				((ucData_V*nPointY != 0) && (ucData_V*nPointY != (DEF_NUM_CUR_MAX_V-1))
				 && (ucData_H*nPointX == 0 || ucData_H*nPointX == (DEF_NUM_CUR_MAX_H-1))))
		    )
			{
				m_nSelCtlPointX = ucData_H*nPointX;
				m_nSelCtlPointY = ucData_V*nPointY;
				if(m_ePatternType == PAT_TYPE__WARP_SEL_CTRL_POINT)
				{
					utilWarp_ShowOsdPattern(PAT_TYPE__WARP_SEL_CTRL_POINT);
				}
			}
			else
			{
				return FALSE;
			}
		}

	}
	return TRUE;
}

INT32 utilWarp_GridPoint_GetColPosition(void) //A35G2_BRC_Casper_0046
{
    if(PM_GRID == NULL || m_nSelCtlPointX >= DEF_NUM_CUR_MAX_H || m_nSelCtlPointY >= DEF_NUM_CUR_MAX_V )
    {
        return 0;
    }

	//LOG_MSG(db_ALWAYS, "m_nSelCtlPointX = %d, m_nSelCtlPointY = %d, X position = %d\r\n", m_nSelCtlPointX, m_nSelCtlPointY, (UINT16)(PM_GRID[m_nSelCtlPointX][m_nSelCtlPointY].x));
	if(m_bOutputNativeRes)
		return((INT32)(PM_GRID[m_nSelCtlPointX][m_nSelCtlPointY].x));
	else
		return((INT32)(PM_GRID_TEMP[m_nSelCtlPointX][m_nSelCtlPointY].x));
}

INT32 utilWarp_GridPoint_GetRowPosition(void) //A35G2_BRC_Casper_0046
{
    if(PM_GRID == NULL || m_nSelCtlPointX >= DEF_NUM_CUR_MAX_H || m_nSelCtlPointY >= DEF_NUM_CUR_MAX_V )
    {
        return 0;
    }

	//LOG_MSG(db_ALWAYS, "m_nSelCtlPointX = %d, m_nSelCtlPointY = %d, Y position = %d\r\n", m_nSelCtlPointX, m_nSelCtlPointY, (UINT16)(PM_GRID[m_nSelCtlPointX][m_nSelCtlPointY].y));
	if(m_bOutputNativeRes)
		return((INT32)(PM_GRID[m_nSelCtlPointX][m_nSelCtlPointY].y));
	else
		return((INT32)(PM_GRID_TEMP[m_nSelCtlPointX][m_nSelCtlPointY].y));
}

//return true : changed
//return false : not changed
BOOL utilWarp_GridPoint_MoveUp(UINT16 nMovePixel) //A35G2_BRC_Casper_0046
{
	int nNewPosX = (int)PM_GRID[m_nSelCtlPointX][m_nSelCtlPointY].x;
	int nNewPosY = (int)PM_GRID[m_nSelCtlPointX][m_nSelCtlPointY].y;

	//LOG_MSG(db_ALWAYS, "Position X = %d, Position Y = %d, nMovePixel = %d\n", nNewPosX, nNewPosY, nMovePixel);
	nNewPosY = nNewPosY - nMovePixel;
	return utilWarp_GridPoint_SetPositionMove(nNewPosX, nNewPosY);
}

//return true : changed
//return false : not changed
BOOL utilWarp_GridPoint_MoveDown(UINT16 nMovePixel) //A35G2_BRC_Casper_0046
{
	int nNewPosX = (int)PM_GRID[m_nSelCtlPointX][m_nSelCtlPointY].x;
	int nNewPosY = (int)PM_GRID[m_nSelCtlPointX][m_nSelCtlPointY].y;

	//LOG_MSG(db_ALWAYS, "Position X = %d, Position Y = %d, nMovePixel = %d\n", nNewPosX, nNewPosY, nMovePixel);
	nNewPosY = nNewPosY+nMovePixel;
	return utilWarp_GridPoint_SetPositionMove(nNewPosX, nNewPosY);
}

//return true : changed
//return false : not changed
BOOL utilWarp_GridPoint_MoveLeft(UINT16 nMovePixel) //A35G2_BRC_Casper_0046
{
	int nNewPosX = (int)PM_GRID[m_nSelCtlPointX][m_nSelCtlPointY].x;
	int nNewPosY = (int)PM_GRID[m_nSelCtlPointX][m_nSelCtlPointY].y;

	//LOG_MSG(db_ALWAYS, "Position X = %d, Position Y = %d, nMovePixel = %d\n", nNewPosX, nNewPosY, nMovePixel);
	nNewPosX = nNewPosX - nMovePixel;
	return utilWarp_GridPoint_SetPositionMove(nNewPosX, nNewPosY);
}

//return true : changed
//return false : not changed
BOOL utilWarp_GridPoint_MoveRight(UINT16 nMovePixel) //A35G2_BRC_Casper_0046
{
	int nNewPosX = (int)PM_GRID[m_nSelCtlPointX][m_nSelCtlPointY].x;
	int nNewPosY = (int)PM_GRID[m_nSelCtlPointX][m_nSelCtlPointY].y;

	//LOG_MSG(db_ALWAYS, "Position X = %d, Position Y = %d, nMovePixel = %d\n", nNewPosX, nNewPosY, nMovePixel);
	nNewPosX = nNewPosX+nMovePixel;
	return utilWarp_GridPoint_SetPositionMove(nNewPosX, nNewPosY);
}


//return true : changed
//return false : not changed
BOOL utilWarp_GridPoint_SetPositionMove(int nPosX, int nPosY) //A35G2_BRC_Casper_0046
{
	uint8 err = E_WpNoErr;
	//LOG_MSG(db_ALWAYS, "Panel H = %d, Panel V = %d\n", m_nNativeResH, m_nNativeResV);
	#if defined(CUSTOM_OPTOMA) //A35G2_BRC_Casper_0137 //A35G2_BRC_Casper_0148
	//validate range
	if(nPosX < 0 || nPosX > m_nNativeResH)
		return FALSE;
	if(nPosY < 0 || nPosY > m_nNativeResV)
		return FALSE;
	#endif
	//assign value
	PM_GRID[m_nSelCtlPointX][m_nSelCtlPointY].x = (float)nPosX;
	PM_GRID[m_nSelCtlPointX][m_nSelCtlPointY].y = (float)nPosY;

	LOG_MSG(db_HAL_WARPING, "Set Position [%d][%d] -> <%f, %f>\r\n", m_nSelCtlPointX, m_nSelCtlPointY, (float)nPosX, (float)nPosY);

	//calculate warp table
	err = CalcOsdWarpTable();
	if( err == E_WpNoErr )
	{
		WriteWarpTable(Def_Wp_Space);
	}
	else
	{
		return FALSE;
	}
	m_bOsdWarpParamChanged = TRUE;
	return TRUE;
}

BOOL utilWarp_GridPoint_SetAllPosition(Point2f Data[DEF_NUM_CUR_MAX_H][DEF_NUM_CUR_MAX_V])
{
	uint8 err = E_WpNoErr;

	//assign value
	memcpy(PM_GRID, Data, sizeof(Point2f)*DEF_NUM_CUR_MAX_H*DEF_NUM_CUR_MAX_V);

	//calculate warp table
	err = CalcOsdWarpTable();
	if( err == E_WpNoErr )
	{
		WriteWarpTable(Def_Wp_Space);
	}
	else
	{
		return FALSE;
	}
	m_bOsdWarpParamChanged = TRUE;
	return TRUE;
}


BOOL utilWarp_GetBlendEnable(void) //A35G2_BRC_Casper_0046
{
	return m_stWarpConfig.stOsd.bBlendEnable;
}

//return true : changed
//return false : not changed
BOOL utilWarp_SetBlendEnable(BOOL bEnable) //A35G2_BRC_Casper_0046
{
	if(bEnable == m_stWarpConfig.stOsd.bBlendEnable)
	{
		return FALSE;
	}
	m_stWarpConfig.stOsd.bBlendEnable = bEnable;
	ApplyBlend(APPLY_BLEND__BY_CONFIG);
	return TRUE;
}

BOOL utilWarp_GetBlendPatternState(void) //A35G2_BRC_Casper_0046
{
	return ((m_ePatternType == PAT_TYPE__BLEND_NO_CTRL_POINT) ? TRUE : FALSE);
}

UINT16 utilWarp_GetOsdBlendWidthTop(void) //A35G2_BRC_Casper_0046
{
	return m_stWarpConfig.stOsd.nBlendWidthT;
}

UINT16 utilWarp_GetOsdBlendWidthDown(void) //A35G2_BRC_Casper_0046
{
	return m_stWarpConfig.stOsd.nBlendWidthB;
}

UINT16 utilWarp_GetOsdBlendWidthLeft(void) //A35G2_BRC_Casper_0046
{
	return m_stWarpConfig.stOsd.nBlendWidthL;
}

UINT16 utilWarp_GetOsdBlendWidthRight(void) //A35G2_BRC_Casper_0046
{
	return m_stWarpConfig.stOsd.nBlendWidthR;
}

//return true : changed
//return false : not changed
BOOL utilWarp_SetOsdBlendWidthTop(UINT16 nOverlapPixel) //A35G2_BRC_Casper_0046
{
	uint16 nVal = 0;
	uint16 nWidth = m_stWarpConfig.stOsd.nBlendWidthT;
	uint16 nOffset = m_stWarpConfig.stOsd.nBlendOffsetT;

	nVal = nOffset + nOverlapPixel;

//	LOG_MSG(db_ALWAYS,"Blend width top origin= %d, new = %d, offset = %d, total = %d\r\n"
//		, nWidth, nOverlapPixel, nOffset, nVal);

	if((nOverlapPixel%BLEND_WIDTH_ADJUST_STEP != 0)
		|| (nVal > BLEND_WIDTH_V_MAX)
		|| (nOverlapPixel > BLEND_WIDTH_V_MAX))
	{
		return FALSE;
	}
	return utilWarp_SetOsdBlendWidth(DIR__UP, nOverlapPixel);
}

//return true : changed
//return false : not changed
BOOL utilWarp_SetOsdBlendWidthDown(UINT16 nOverlapPixel) //A35G2_BRC_Casper_0046
{
	uint16 nVal = 0;

	uint16 nWidth =  m_stWarpConfig.stOsd.nBlendWidthB;
	uint16 nOffset = m_stWarpConfig.stOsd.nBlendOffsetB;

	nVal = nOffset + nOverlapPixel;

//	LOG_MSG(db_ALWAYS,"Blend width buttom origin= %d, new = %d, offset = %d, total = %d\r\n"
//		, nWidth, nOverlapPixel, nOffset, nVal);

	if((nOverlapPixel%BLEND_WIDTH_ADJUST_STEP != 0)
		|| (nVal > BLEND_WIDTH_V_MAX)
		|| (nOverlapPixel > BLEND_WIDTH_V_MAX))
	{
		return FALSE;
	}
	return utilWarp_SetOsdBlendWidth(DIR__DOWN, nOverlapPixel);
}

//return true : changed
//return false : not changed
BOOL utilWarp_SetOsdBlendWidthLeft(UINT16 nOverlapPixel) //A35G2_BRC_Casper_0046
{
	uint16 nVal = 0;
	uint16 nWidth =  m_stWarpConfig.stOsd.nBlendWidthL;
	uint16 nOffset = m_stWarpConfig.stOsd.nBlendOffsetL;

	nVal = nOffset + nOverlapPixel;

//	LOG_MSG(db_ALWAYS,"Blend width left origin= %d, new = %d, offset = %d, total = %d\r\n"
//		, nWidth, nOverlapPixel, nOffset, nVal);

	if((nOverlapPixel%BLEND_WIDTH_ADJUST_STEP != 0)
		|| (nVal > BLEND_WIDTH_H_MAX)
		|| (nOverlapPixel > BLEND_WIDTH_H_MAX))
	{
		return FALSE;
	}
	return utilWarp_SetOsdBlendWidth(DIR__LEFT, nOverlapPixel);
}

//return true : changed
//return false : not changed
BOOL utilWarp_SetOsdBlendWidthRight(UINT16 nOverlapPixel) //A35G2_BRC_Casper_0046
{
	uint16 nVal = 0;
	uint16 nWidth =  m_stWarpConfig.stOsd.nBlendWidthR;
	uint16 nOffset = m_stWarpConfig.stOsd.nBlendOffsetR;

	nVal = nOffset + nOverlapPixel;

//	LOG_MSG(db_ALWAYS,"Blend width right origin= %d, new = %d, offset = %d, total = %d\r\n"
//		, nWidth, nOverlapPixel, nOffset, nVal);

	if((nOverlapPixel%BLEND_WIDTH_ADJUST_STEP != 0)
		|| (nVal > BLEND_WIDTH_H_MAX)
		|| (nOverlapPixel > BLEND_WIDTH_H_MAX))
	{
		return FALSE;
	}
	return utilWarp_SetOsdBlendWidth(DIR__RIGHT, nOverlapPixel);
}

//return true : changed
//return false : not changed
BOOL utilWarp_SetOsdBlendWidth(eDIR eDir, UINT16 nOverlapPixel) //A35G2_BRC_Casper_0046
{
	BOOL bChanged = FALSE;

	//set value
	if(eDir==DIR__LEFT)
	{
		if(nOverlapPixel != m_stWarpConfig.stOsd.nBlendWidthL)
		{
			m_stWarpConfig.stOsd.nBlendWidthL = nOverlapPixel;
			bChanged = TRUE;
		}
	}
	else if(eDir==DIR__RIGHT)
	{
		if(nOverlapPixel != m_stWarpConfig.stOsd.nBlendWidthR)
		{
			m_stWarpConfig.stOsd.nBlendWidthR = nOverlapPixel;
			bChanged = TRUE;
		}
	}
	else if(eDir==DIR__UP)
	{
		if(nOverlapPixel != m_stWarpConfig.stOsd.nBlendWidthT)
		{
			m_stWarpConfig.stOsd.nBlendWidthT = nOverlapPixel;
			bChanged = TRUE;
		}
	}
	else if(eDir==DIR__DOWN)
	{
		if(nOverlapPixel != m_stWarpConfig.stOsd.nBlendWidthB)
		{
			m_stWarpConfig.stOsd.nBlendWidthB = nOverlapPixel;
			bChanged = TRUE;
		}
	}

	if(bChanged)
	{
#if 0   // G50_Keven_0003, follow tim's modify	 //A35G2_Wesley_0093
        if((m_ePatternType == PAT_TYPE__BLEND_NO_CTRL_POINT)
            ||(m_ePatternType == PAT_TYPE__WARP_SEL_CTRL_POINT))
#else
        if(m_ePatternType != PAT_TYPE__OFF)
#endif
		{
			m_bOsdWarpParamChanged = TRUE;
			utilWarp_ShowOsdPattern(m_ePatternType);
		}
		else
		{
			ApplyBlend(APPLY_BLEND__BY_CONFIG);
		}
	}

	return bChanged;

}

UINT16 utilWarp_GetOsdBlendOffsetTop(void) //A35G2_BRC_Casper_0046
{
	return m_stWarpConfig.stOsd.nBlendOffsetT;
}

UINT16 utilWarp_GetOsdBlendOffsetDown(void)
{
	return m_stWarpConfig.stOsd.nBlendOffsetB;
}

UINT16 utilWarp_GetOsdBlendOffsetLeft(void)
{
	return m_stWarpConfig.stOsd.nBlendOffsetL;
}

UINT16 utilWarp_GetOsdBlendOffsetRight(void)
{
	return m_stWarpConfig.stOsd.nBlendOffsetR;
}

//return true : changed
//return false : not changed
BOOL utilWarp_SetOsdBlendOffsetTop(UINT16 nOffsetPixel) //A35G2_BRC_Casper_0046
{
	uint16 nVal = 0;
	uint16 nWidth =  m_stWarpConfig.stOsd.nBlendWidthT;
	uint16 nOffset = m_stWarpConfig.stOsd.nBlendOffsetT;

	nVal = nWidth + nOffsetPixel;

//	LOG_MSG(db_ALWAYS,"Blend offset top origin= %d, new = %d, width = %d, total = %d\r\n"
//		, nOffset, nOffsetPixel, nWidth, nVal);

	if((nOffsetPixel%BLEND_WIDTH_ADJUST_STEP != 0)
		|| (nVal > BLEND_WIDTH_V_MAX)
		|| (nOffsetPixel > BLEND_WIDTH_V_MAX))
	{
		return FALSE;
	}

	return utilWarp_SetOsdBlendOffset(DIR__UP, nOffsetPixel);
}

//return true : changed
//return false : not changed
BOOL utilWarp_SetOsdBlendOffsetDown(UINT16 nOffsetPixel) //A35G2_BRC_Casper_0046
{
	uint16 nVal = 0;
	uint16 nWidth =  m_stWarpConfig.stOsd.nBlendWidthB;
	uint16 nOffset = m_stWarpConfig.stOsd.nBlendOffsetB;

	nVal = nWidth + nOffsetPixel;

//	LOG_MSG(db_ALWAYS,"Blend offset bottom origin= %d, new = %d, width = %d, total = %d\r\n"
//		, nOffset, nOffsetPixel, nWidth, nVal);

	if((nOffsetPixel%BLEND_WIDTH_ADJUST_STEP != 0)
		|| (nVal > BLEND_WIDTH_V_MAX)
		|| (nOffsetPixel > BLEND_WIDTH_V_MAX))
	{
		return FALSE;
	}

	return utilWarp_SetOsdBlendOffset(DIR__DOWN, nOffsetPixel);
}

//return true : changed
//return false : not changed
BOOL utilWarp_SetOsdBlendOffsetLeft(UINT16 nOffsetPixel) //A35G2_BRC_Casper_0046
{
	uint16 nVal = 0;
	uint16 nWidth =  m_stWarpConfig.stOsd.nBlendWidthL;
	uint16 nOffset = m_stWarpConfig.stOsd.nBlendOffsetL;

	nVal = nWidth + nOffsetPixel;

//	LOG_MSG(db_ALWAYS,"Blend offset left origin= %d, new = %d, width = %d, total = %d\r\n"
//		, nOffset, nOffsetPixel, nWidth, nVal);

	if((nOffsetPixel%BLEND_WIDTH_ADJUST_STEP != 0)
		|| (nVal > BLEND_WIDTH_H_MAX)
		|| (nOffsetPixel > BLEND_WIDTH_H_MAX))
	{
		return FALSE;
	}

	return utilWarp_SetOsdBlendOffset(DIR__LEFT, nOffsetPixel);
}

//return true : changed
//return false : not changed
BOOL utilWarp_SetOsdBlendOffsetRight(UINT16 nOffsetPixel) //A35G2_BRC_Casper_0046
{
	uint16 nVal = 0;
	uint16 nWidth =  m_stWarpConfig.stOsd.nBlendWidthR;
	uint16 nOffset = m_stWarpConfig.stOsd.nBlendOffsetR;

	nVal = nWidth + nOffsetPixel;

//	LOG_MSG(db_ALWAYS,"Blend offset right origin= %d, new = %d, width = %d, total = %d\r\n"
//		, nOffset, nOffsetPixel, nWidth, nVal);

	if((nOffsetPixel%BLEND_WIDTH_ADJUST_STEP != 0)
		|| (nVal > BLEND_WIDTH_H_MAX)
		|| (nOffsetPixel > BLEND_WIDTH_H_MAX))
	{
		return FALSE;
	}

	return utilWarp_SetOsdBlendOffset(DIR__RIGHT, nOffsetPixel);
}

//return true : changed
//return false : not changed
BOOL utilWarp_SetOsdBlendOffset(eDIR eDir, UINT16 nOffsetPixel) //A35G2_BRC_Casper_0046
{
	BOOL bChanged = FALSE;

	//set value
	if(eDir==DIR__LEFT)
	{
		if(nOffsetPixel != m_stWarpConfig.stOsd.nBlendOffsetL)
		{
			m_stWarpConfig.stOsd.nBlendOffsetL = nOffsetPixel;
			bChanged = TRUE;
		}
	}
	else if(eDir==DIR__RIGHT)
	{
		if(nOffsetPixel != m_stWarpConfig.stOsd.nBlendOffsetR)
		{
			m_stWarpConfig.stOsd.nBlendOffsetR = nOffsetPixel;
			bChanged = TRUE;
		}
	}
	else if(eDir==DIR__UP)
	{
		if(nOffsetPixel != m_stWarpConfig.stOsd.nBlendOffsetT)
		{
			m_stWarpConfig.stOsd.nBlendOffsetT = nOffsetPixel;
			bChanged = TRUE;
		}
	}
	else if(eDir==DIR__DOWN)
	{
		if(nOffsetPixel != m_stWarpConfig.stOsd.nBlendOffsetB)
		{
			m_stWarpConfig.stOsd.nBlendOffsetB = nOffsetPixel;
			bChanged = TRUE;
		}
	}

	if(bChanged)
	{
#if 0   // G50_Keven_0003	//A35G2_Wesley_0093
        if(m_ePatternType == PAT_TYPE__BLEND_NO_CTRL_POINT)
#else
        if(m_ePatternType != PAT_TYPE__OFF)
#endif

		{
			m_bOsdWarpParamChanged = TRUE;
			utilWarp_ShowOsdPattern(m_ePatternType);
		}
		else
		{
			ApplyBlend(APPLY_BLEND__BY_CONFIG);
		}
	}

	return bChanged;

}


void utilWarp_ResetBlendConfig(void) //A35G2_BRC_Casper_0046
{
	ResetBlendConfig();
	if(SaveWarpConfig()!=FLASH_ACCESS__PASS)
		LOG_MSG(db_HAL_WARPING,"save blend config fail\r\n");

	ApplyBlend(APPLY_BLEND__BY_CONFIG);
	if(m_ePatternType == PAT_TYPE__WARP_SEL_CTRL_POINT || m_ePatternType == PAT_TYPE__BLEND_NO_CTRL_POINT)
	{
		utilWarp_ShowOsdPattern(m_ePatternType);
	}
}

void utilWarp_ResetWarpSetting(void) //A35G2_BRC_Casper_0046
{
	//init control point
	InitControlPoint();

	ResetWarpSetting();
	if(SaveWarpConfig()!=FLASH_ACCESS__PASS)
		LOG_MSG(db_HAL_WARPING,"save blend config fail\r\n");

	ApplyWarp(APPLY_WARP__RE_CALC);
	if(m_ePatternType == PAT_TYPE__WARP_SEL_CTRL_POINT || m_ePatternType == PAT_TYPE__BLEND_NO_CTRL_POINT)
	{
		utilWarp_ShowOsdPattern(m_ePatternType);
	}
}

void utilWarp_ResetWarpControlPoint(void) //A35G2_BRC_Casper_0047
{
	//init control point
	InitControlPoint();
	ApplyWarp(APPLY_WARP__RE_CALC);
}

BOOL utilWarp_GetWarpPatternState(void) //A35G2_BRC_Casper_0046
{
	return ((m_ePatternType == PAT_TYPE__WARP_SEL_CTRL_POINT) ? TRUE : FALSE);
}

void utilWarp_ReDrawWarpPattern(void) //A35G2_BRC_Casper_0047
{
	if(m_ePatternType == PAT_TYPE__WARP_SEL_CTRL_POINT
		|| m_ePatternType == PAT_TYPE__WARP_MOV_CTRL_POINT
		|| m_ePatternType == PAT_TYPE__WARP_NO_CTRL_POINT)
	{
		utilWarp_ShowOsdPattern(m_ePatternType);
	}
}


void utilWarp_OsdBlackLevel_ResetConfigBottom(void)	//A65_OPTOMA_Doulas_0029
{
	UINT8 ucOldSelect = m_eAreaSelection;

	dvC789_Write( BN_RTCT, RTCT_THRU);	//A65_OPTOMA_Doulas_0136
	m_eAreaSelection = BLACKLEVEL_AREA__BOTTOM;
	OsdBlackLevel_ResetConfig(m_eAreaSelection);
	if(SaveWarpConfig()!=FLASH_ACCESS__PASS)
	{
		//LOG_MSG(db_ALWAYS,"save black level config fail\r\n");
	}
	InitBlackLevelNode(m_eAreaSelection);
	ApplyBlackLevel(APPLY_BLACKLEVEL__BY_CONFIG);

	if(ucOldSelect != m_eAreaSelection)
	{
		m_eAreaSelection = ucOldSelect;
		if(SaveWarpConfig()!=FLASH_ACCESS__PASS)
		{

		}
		ApplyBlackLevel(APPLY_BLACKLEVEL__BY_CONFIG);
	}
}

void utilWarp_OsdBlackLevel_ResetConfigTop(void)		//A65_OPTOMA_Doulas_0029
{
	UINT8 ucOldSelect = m_eAreaSelection;

	dvC789_Write( BN_RTCT, RTCT_THRU);	//A65_OPTOMA_Doulas_0136
	m_eAreaSelection = BLACKLEVEL_AREA__TOP;
	OsdBlackLevel_ResetConfig(m_eAreaSelection);
	if(SaveWarpConfig()!=FLASH_ACCESS__PASS)
	{
		//LOG_MSG(db_ALWAYS,"save black level config fail\r\n");
	}
	InitBlackLevelNode(m_eAreaSelection);
	ApplyBlackLevel(APPLY_BLACKLEVEL__BY_CONFIG);

	if(ucOldSelect != m_eAreaSelection)
	{
		m_eAreaSelection = ucOldSelect;
		if(SaveWarpConfig()!=FLASH_ACCESS__PASS)
		{

		}
		ApplyBlackLevel(APPLY_BLACKLEVEL__BY_CONFIG);
	}
}

//A65_OPTOMA_CL_0001 add for CU multiple selection  //A35G2_CDS_CL_0001
void utilWarp_SelectCUGrid(sCOLOR_UNIFORMITY_POINT_SELECT *psCUPointSelect)
{
	uint8 i;

	m_ucCUShow = psCUPointSelect->ucShow;
	for(i = 0; i < 63; i++)
	{
	    m_pnCUSelectEnableTable[i%9][i/9] = psCUPointSelect->ucPoint[i];
	}

}

//A65_OPTOMA_CL_0006
eCOLOR_PALETTE_GROUP utilWarp_GetColorPaletteGroupIndex(void)
{
    return m_ucColorPalette;
}

//A65_OPTOMA_CL_0007
//return true: changed
//return false: not changed
BOOL utilWarp_SetColorPaletteGroupIndex(eCOLOR_PALETTE_GROUP eColorPaletteIndex)
{
	dvC789_Write(B1_OSDMWI, m_nLineFeed);  //A65_OPTOMA_CL_0010
	dvC789_Write(B0_BBWMWI, m_nLineFeed);

    LOG_MSG(db_HAL_WARPING, "Init Color Palette group to %d (pre %d)\r\n", eColorPaletteIndex, m_ucColorPalette);

    if(m_ucColorPalette == eColorPaletteIndex)
    {
        return FALSE;
    }

    LOG_MSG(db_HAL_WARPING, "\r\n ===>  Color Palette group set to %d\r\n\r\n", eColorPaletteIndex);

    m_ucColorPalette = eColorPaletteIndex;
    switch(eColorPaletteIndex)
    {
        case COLOR_PALETTE__OSD:
            OSD_Color_Init();
            break;
        case COLOR_PALETTE__AP:
            halWarpOSD_AP_Color_Initial(TRUE);
            break;

        #if 0
        case COLOR_PALETTE__PNG:
            halWarpOSD_AP_Color_Initial(FALSE);
            break;
        case COLOR_PALETTE__SPRITE:
            halWarpOSD_AP_Color_Initial(FALSE);
            break;
        #endif

        default:
            break;
    }
    return TRUE;
}

//A65_OPTOMA_CL_0007
void utilWarp_CopyCircleInfo(sWARPOSD_CIRCLE_GRID_DRAW_INFO *psCircleInfo)
{
    m_psCircleInfo = psCircleInfo;
}

BOOL utilWarp_GetApBlendApply(void)  //A65_OPTOMA_CL_0010
{
    return m_bApBlendApply;
}

void utilWarp_SetApBlendApply(BOOL bBlendApply)  //A65_OPTOMA_CL_0010
{
    m_bApBlendApply  = bBlendApply;
}

BOOL utilWarp_GetApOnlyBlendApply(void)
{
    return m_bApOnlyBlendApply;
}

void utilWarp_SetApOnlyBlendApply(BOOL bOnlyBlendApply)
{
    m_bApOnlyBlendApply  = bOnlyBlendApply;
}

BOOL utilWarp_GetApBlacklevelApply(void)  //A65_OPTOMA_CL_0010
{
    return m_bApBlacklevelApply;
}

void utilWarp_SetApBlacklevelApply(BOOL bBlacklevelApply)  //A65_OPTOMA_CL_0010
{
    m_bApBlacklevelApply  = bBlacklevelApply;
}

UINT16 utilWarp_C789_OSD_H_Start_Get(void)	//A65_OPTOMA_Doulas_0232
{
	return PS_PANEL_ACT_HST;
}

UINT16 utilWarp_C789_OSD_V_Start_Get(void)	//A65_OPTOMA_Doulas_0232
{
	return PS_PANEL_ACT_VST + utilWarp_Panel_Vstart_Offset_Get();
}

void utilWarp_Black_Level_Boundary_Flag_Set(UINT8 ucBoundary)						//A35G2_Owen_0002 //A65_OPTOMA_Doulas_0239
{
	LOG_MSG(db_HAL_WARPING,"utilWarp_Black_Level_Boundary_Flag_Set %d\r\n",ucBoundary);		//A65_OPTOMA_Doulas_0239
	if(m_stWarpConfig.stOsd.stBlackLevel.ucBoundary != ucBoundary)
	{
		m_bOsdWarpParamChanged = TRUE;
	}
    m_stWarpConfig.stOsd.stBlackLevel.ucBoundary = ucBoundary;
}

//A65_OPTOMA_CL_0015
void utilWarp_CopyChecksInfo(sWARPOSD_CHECKBOARD_DRAW_INFO *psChecksInfo)
{
    m_psChecksInfo = psChecksInfo;
}

UINT8 utilWarp_GetChecksLayerMode(void)
{
    return m_psChecksInfo->ucLayerMode;
}

BOOL utilWarp_SetShowBlendOnWarpPattern(BOOL bShow)
{
    if(bShow == m_stWarpConfig.stOsd.bShowBlendOnWarpPattern)
    {
        return FALSE;
    }

    m_stWarpConfig.stOsd.bShowBlendOnWarpPattern = bShow;
    if(     ( m_ePatternType == PAT_TYPE__WARP_SEL_CTRL_POINT )
        ||  ( m_ePatternType == PAT_TYPE__WARP_MOV_CTRL_POINT )
        ||  ( m_ePatternType == PAT_TYPE__WARP_NO_CTRL_POINT )
        ||  ( m_ePatternType == PAT_TYPE__SEL_BLEND_WIDTH )
        ||  ( m_ePatternType == PAT_TYPE__ADJ_BLEND_WIDTH )
        ||  ( m_ePatternType == PAT_TYPE__BLEND_NO_CTRL_POINT )
    )
    {
        utilWarp_ShowOsdPattern(m_ePatternType);
    }
    return TRUE;
}

BOOL utilWarp_GetShowBlendOnWarpPattern(void)
{
    return m_stWarpConfig.stOsd.bShowBlendOnWarpPattern;
}

BOOL utilWarp_SetDrawOverlapGridOnWarpPattern(BOOL bDraw)
{
	if(bDraw == m_stWarpConfig.stOsd.bDrawOverlapGridOnWarpPattern)
		return FALSE;

	m_stWarpConfig.stOsd.bDrawOverlapGridOnWarpPattern = bDraw;
	if(m_ePatternType == PAT_TYPE__WARP_SEL_CTRL_POINT)
	{
		utilWarp_ShowOsdPattern(m_ePatternType);
	}
	return TRUE;
}

BOOL utilWarp_GetDrawOverlapGridOnWarpPattern(void)
{
	return m_stWarpConfig.stOsd.bDrawOverlapGridOnWarpPattern;
}



UINT32 utilWarp_NativePanel_Get(void)
{
    UINT32 Panel = PANEL_2D_OUTPUT;

    switch(Panel)
	{
	    case ePANEL_ID_WUXGA_60HZ:
	        return RES_ID__WUXGA;

	    case ePANEL_ID_1080P_60HZ:
	        return RES_ID__1080P60;

	    default:
	        LOG_MSG(db_HAL_WARPING, "%s: Get Native Panel Fail (%d)\r\n", __FUNCTION__, Panel);
	}

	return RES_ID__MAX;
}



