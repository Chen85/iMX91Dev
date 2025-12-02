#ifndef _UTIL_WARP_DEMO_PROAV_H_
#define _UTIL_WARP_DEMO_PROAV_H_

#include "Common.h"
#include "cmd_ap.h"	//A65_OPTOMA_CL_0001 add for CU multiple selection  //A35G2_CDS_CL_0001

#define NATIVE_RES_ID utilWarp_NativePanel_Get()   //RES_ID__WUXGA //index of native resolution

#define DEF_NUM_CUR_MAX_H	33	//G100_Doulas_0059// Maximum number of horizontal cursors (e.g. 17x9 -> #define DEF_NUM_CUR_MAX_H	17)
#define DEF_NUM_CUR_MAX_V		33	// Maximum number of vertical cursors   (e.g. 17x9 -> #define DEF_NUM_CUR_MAX_V		9)
#define DEF_NUM_CUR_MAX		33	// = (DEF_NUM_CUR_MAX_V > DEF_NUM_CUR_MAX_H) ? DEF_NUM_CUR_MAX_V : DEF_NUM_CUR_MAX_H (e.g. 17x9 -> #define DEF_NUM_CUR_MAX	17)

#define MAX_INTERSECTED_POINT 10    //A65_OPTOMA_Doulas_0020
#define BLACKLEVEL_NODE_MIN  4
#define BLACKLEVEL_NODE_MAX  32
#define BLACKLEVEL_VALUE_MAX 255
#define BLACKLEVEL_VALUE_MIN 0		//A65_OPTOMA_Doulas_0020
#define BLACKLEVEL_VALUE_R 20
#define BLACKLEVEL_VALUE_G 20
#define BLACKLEVEL_VALUE_B 20
#define BLACKLEVEL_UNVALID_INTERSECTED_POINT_VALUE 2000
#define BLACKLEVEL_PALETTE_NUM 16
#define BLACKLEVEL_PALETTE_RGB 3
//OSD Black Level Selection Mode
// 0 : 2 ways for clockwise/counter-clockwise
// 1 : 4 ways for up/down/left/right
#define BLACKLEVEL_SEL_MODE 1///1	//A65_OPTOMA_Doulas_0159 disable up/down/left/right adjustment //A65_OPTOMA_Doulas_0146 //A65_OPTOMA_CL_0003
#define BLACKLEVEL_SEARCH_LEVEL 5  //A65_OPTOMA_CL_0003

#define WARP_DEVICE_CODE    0x18

#define BYTE0			            0xFF
#define BYTE1			            0xFF00
#define BYTE2			            0xFF0000
#define BYTE3			            0xFF000000
#define BYTE4			            0xFF00000000
#define BYTE5			            0xFF0000000000
#define BYTE6			            0xFF000000000000
#define BYTE7			            0xFF00000000000000
#define OFFSET_1BYTE	            8
#define OFFSET_2BYTE	            16
#define OFFSET_3BYTE	            24
#define OFFSET_4BYTE	            32
#define OFFSET_5BYTE	            40
#define OFFSET_6BYTE	            48
#define OFFSET_7BYTE	            56
#define GET_BYTE0(a)                (uint8)(a & 0xFF)
#define GET_BYTE1(a)                (uint8)((a >> OFFSET_1BYTE) & 0xFF)
#define GET_BYTE2(a)                (uint8)((a >> OFFSET_2BYTE) & 0xFF)
#define GET_BYTE3(a)                (uint8)((a >> OFFSET_3BYTE) & 0xFF)
#define GET_BYTE4(a)                (uint8)((a >> OFFSET_4BYTE) & 0xFF)
#define GET_BYTE5(a)                (uint8)((a >> OFFSET_5BYTE) & 0xFF)
#define GET_BYTE6(a)                (uint8)((a >> OFFSET_6BYTE) & 0xFF)
#define GET_BYTE7(a)                (uint8)((a >> OFFSET_7BYTE) & 0xFF)
#define GET_U8(x)                   *(x)
#define GET_LSB_U16(x)              ((*(x+1))<<8)+(*(x))

#define NO_FILE             1		//G100_Doulas_0027
#define WARP_SAVE_FILE_SYSTEM		//G100_Doulas_0027

/* Bit masks. */
#ifndef BIT0
#define BIT0        0x01
#endif

#ifndef BIT1
#define BIT1        0x02
#endif

#ifndef BIT2
#define BIT2        0x04
#endif

#ifndef BIT3
#define BIT3        0x08
#endif

#ifndef BIT4
#define BIT4        0x10
#endif

#ifndef BIT5
#define BIT5        0x20
#endif

#ifndef BIT6
#define BIT6        0x40
#endif

#ifndef BIT7
#define BIT7        0x80
#endif

#ifndef BIT8
#define BIT8      0x0100
#endif

#ifndef BIT9
#define BIT9      0x0200
#endif

#ifndef BIT10
#define BIT10     0x0400
#endif

#ifndef BIT11
#define BIT11     0x0800
#endif

#ifndef BIT12
#define BIT12     0x1000
#endif

#ifndef BIT13
#define BIT13     0x2000
#endif

#ifndef BIT14
#define BIT14     0x4000
#endif

#ifndef BIT15
#define BIT15     0x8000
#endif

#ifndef BIT16
#define BIT16 0x00010000
#endif

#ifndef BIT17
#define BIT17 0x00020000
#endif

#ifndef BIT18
#define BIT18 0x00040000
#endif

#ifndef BIT19
#define BIT19 0x00080000
#endif

#ifndef BIT20
#define BIT20 0x00100000
#endif

#ifndef BIT21
#define BIT21 0x00200000
#endif

#ifndef BIT22
#define BIT22 0x00400000
#endif

#ifndef BIT23
#define BIT23 0x00800000
#endif

#ifndef BIT24
#define BIT24 0x01000000
#endif

#ifndef BIT25
#define BIT25 0x02000000
#endif

#ifndef BIT26
#define BIT26 0x04000000
#endif

#ifndef BIT27
#define BIT27 0x08000000
#endif

#ifndef BIT28
#define BIT28 0x10000000
#endif

#ifndef BIT29
#define BIT29 0x20000000
#endif

#ifndef BIT30
#define BIT30 0x40000000
#endif

#ifndef BIT31
#define BIT31 0x80000000
#endif



////////  callback function start  ////////
typedef UINT8 (*fpDataMgr_ACU_Target_Select_Tmp_Get)(void);
typedef void (*fpGui_OSD_ON_Set)(UINT8 ucVal);
typedef void (*fpGui_SendUpdateOSDEvent)(void);
typedef void (*fpGeo_DrawOsdRect)(eWARPOSD_INSERT_LOCATION eWarpOsdPosition,       UINT16 PosX, UINT16 PosY, UINT16 Width, UINT16 Height, UINT16 ColorIdx);
typedef void (*fpGeo_EnableTestPattern)(eWARPOSD_INSERT_LOCATION ePosition);
typedef void (*fpGeo_ClearTestPattern)(eWARPOSD_INSERT_LOCATION ePosition);
typedef void (*fpGeo_CopyDrawRect)(void);


typedef struct
{
    fpDataMgr_ACU_Target_Select_Tmp_Get fpDataMgr_ACU_Target_Select_Tmp_GetCb;
    fpGui_OSD_ON_Set fpGui_OSD_ON_SetCb;
    fpGui_SendUpdateOSDEvent fpGui_SendUpdateOSDEventCb;
    fpGeo_DrawOsdRect fpGeo_DrawOsdRectCb;
    fpGeo_EnableTestPattern fpGeo_EnableTestPatternCb;
    fpGeo_ClearTestPattern fpGeo_ClearTestPatternCb;
    fpGeo_CopyDrawRect fpGeo_CopyDrawRectCb;
}sUTILWARPDEMO_CALLBACK;

void utilWarpDemo_RegCallback(sUTILWARPDEMO_CALLBACK fpCallback);
/////////  callback function end  /////////


typedef struct {
	int st;
	int end;
} pos_t;

typedef struct
{
    uint16 x;
    uint16 y;
}PointT;  //A65_OPTOMA_Doulas_0020

typedef struct
{
    float x;
    float y;
}Point2f;

typedef struct {
	float a;
	float b;
	float c;
	float d;
}Point4f;

typedef enum
{
 	RES_ID__XGA         = 0,	/* 1024 x  768 */
 	RES_ID__WXGA        = 1,	/* 1280 x  800 */
 	RES_ID__UWHD        = 2,	/* 1920 x  720 */
 	RES_ID__1080P60     = 3,	/* 1920 x 1080 */
 	RES_ID__WUXGA       = 4,	/* 1920 x 1200 */
 	RES_ID__XGA120      = 5,	/* 1024 x  768 @ 120 */
  	RES_ID__720P120     = 6,	/* 1280 x  720 @ 120 */
 	RES_ID__1080P48     = 7,	/* 1920 x 1080 @ 48 */
 	RES_ID__1080P120    = 8,	/* 1920 x 1080 @ 120*/		//G100_Doulas_0027
 	RES_ID__WUXGA120    = 9,	/* 1920 x 1200 @ 120*/		//G100_Doulas_0063
 	RES_ID__WXGA120     = 10,	/* 1280 x  800 @ 120*/		//R70G2_Doulas_0004
 	RES_ID__UHD60       = 11,	/* 3840 x 2160 @ 60*/
 	RES_ID__WQUXGA60    = 12,	/* 3840 x 2400 @ 60*/
 	RES_ID__1080P240    = 13,	/* 1920 x 1080 @ 240*/
 	RES_ID__WUXGA240    = 14,	/* 1920 x 1200 @ 240*/
 	RES_ID__MAX         = 15,
}eRES_ID;

typedef enum
{
	OUTPUT_MODE__OFF,
	OUTPUT_MODE__BYPASS,
	OUTPUT_MODE__NORMAL,
	OUTPUT_MODE__MAX,
}eOUTPUT_MODE;

typedef enum
{
	SET_MCT__BY_CONFIG,	//set MCT by config
	SET_MCT__DISABLE,	//disable MCT
}eSET_MCT;

typedef enum
{
	APPLY_WARP__BY_CONFIG,	//apply pre-calculated warp table
	APPLY_WARP__RE_CALC,	//re-calculate warp table before apply
}eAPPLY_WARP;

typedef enum
{
	APPLY_BLEND__BY_CONFIG,	//apply blend by config
	APPLY_BLEND__DISABLE,	//disable blend
}eAPPLY_BLEND;

typedef enum
{
	APPLY_BLACKLEVEL__BY_CONFIG,	//apply blacklevel by config
	APPLY_BLACKLEVEL__ENABLE,		//enable blacklevel without apply gamma and area
	APPLY_BLACKLEVEL__DISABLE,		//disable blacklevel
}eAPPLY_BLACKLEVEL;    //A65_OPTOMA_Doulas_0020

typedef enum
{
	POL__POS = 0x00,	//positive
	POL__NEG,			//negative
}ePOL;	//polarity

typedef enum
{
	ADJ__MINUS,
	ADJ__PLUS,
	ADJ__INVALID,
}eADJ;

typedef struct
{
	eRES_ID		eVID;
	uint16 	    nFrameRate;	//format:Hz*100
	uint16	    nPixClk;
	uint16	    nHActive;
	uint16	    nVActive;
	uint16	    nHTotal;
	uint16	    nVTotal;
	uint16	    nHStart;
	uint16	    nVStart;
	uint8		nHSyncWidth;
	uint8		nVSyncWidth;
	ePOL		eHPol;
	ePOL		eVPol;
}MODE_TABLE;

typedef enum
{
	E_WpNoErr,
	E_WpErrAngH,	// Error of H-slope
	E_WpErrAngV,	// Error of V-slope
	E_WpErrVShL,	// Error of local V-shrink rate
	E_WpErrVShA,	// Error of average V-shrink rate
	E_WpErrHSh,		// Error of H-shrink rate
	E_WpErrOut,		// Error of outside of ACT
	E_WpErrItv,		// Error of grid interval
	E_WpErrReset,	// Error of other reasons
	E_WpErrOther,	// Error of other reasons
}E_WpErr;

typedef enum
{
	FLASH_ACCESS__PASS = 0,
	FLASH_ACCESS__FAIL,
	FLASH_ACCESS__ERASE_FLASH_ERR,
	FLASH_ACCESS__RAM_TO_FLASH_ERR,
	FLASH_ACCESS__FLASH_TO_RAM_ERR,
	FLASH_ACCESS__COMPARE_ERR,
	FLASH_ACCESS__VERIFY_HEADER_ERR,
	FLASH_ACCESS__VERIFY_CHECKSUM_ERR,
	FLASH_ACCESS__VERIFY_CONFIG_ERR,
	FLASH_ACCESS__PRESET_IDX_ERR,
} FLASH_ACCESS;

typedef enum
{
	WARP_POINT__2x2,
	WARP_POINT__3x3,
	WARP_POINT__5x5,
	WARP_POINT__9x9,
	WARP_POINT__17x17,
	WARP_POINT__33x33,     //G100_Doulas_0059
	WARP_POINT__INVALID,
}eWARP_POINT;

typedef enum
{
	WARP_LIMITATION__CHANGED,
	WARP_LIMITATION__NO_CHANGE,
	WARP_LIMITATION__EXEC_FAIL,
	WARP_LIMITATION__INVALID,
}eWARP_LIMITATION; //A35G2_BRC_Casper_0047

typedef enum
{
	DIR__UP,
	DIR__DOWN,
	DIR__LEFT,
	DIR__RIGHT,
	DIR__INVALID,
}eDIR;

typedef enum
{
	DMA_TARGET__SDRAM			= 0x00, //SDRAM
	DMA_TARGET__DISTORTION		= 0x11, //Distortion Correction LUT
	DMA_TARGET__INVALID
}eDMA_TARGET;

typedef enum
{
	FLASH_ERASE_MODE__SE4K,		//Sector erase 4KB
	FLASH_ERASE_MODE__BE32K,	//block erase 32KB
	FLASH_ERASE_MODE__BE64K,	//block erase 64KB
	FLASH_ERASE_MODE__INVALID,
}eFLASH_ERASE_MODE;

typedef enum
{
#ifdef SAME_RAM_FOR_BEFORE_WARP_OSD_AND_SCALER_OSD

	/* 000 */ COLOR_IDX__GREEN = 151,
	/* 001 */ COLOR_IDX__MAGENTA = 152,
	/* 002 */ COLOR_IDX__RED = 153,
	/* 003 */ COLOR_IDX__CYAN = 154,
	/* 004 */ COLOR_IDX__BLUE = 155,
	/* 005 */ COLOR_IDX__YELLOW = 156,
	/* 006 */ COLOR_IDX__BLACK = 157,
	/* 007 */ COLOR_IDX__WHITE = 158,
	/* 008 */ COLOR_IDX__DARK_GREEN = 159,
	/* 009 */ COLOR_IDX__DARK_MAGENTA = 160,
	/* 010 */ COLOR_IDX__DARK_RED = 161,
	/* 011 */ COLOR_IDX__DARK_CYAN = 162,
	/* 012 */ COLOR_IDX__BLACKLEVEL_GREEN = 163,    //A65_OPTOMA_Doulas_0020
	/* 013 */ COLOR_IDX__BLACKLEVEL_MAGENTA = 164,  //A65_OPTOMA_Doulas_0020
	/* 014 */ COLOR_IDX__BLACKLEVEL_RED = 165,      //A65_OPTOMA_Doulas_0020
	/* 015 */ COLOR_IDX__BLACKLEVEL_CYAN = 166,     //A65_OPTOMA_Doulas_0020
	/* 016 */ COLOR_IDX__BLACK_BKG = 167,			//black for background
    /* 017 */ COLOR_IDX__GREY192 = 168,   //for ACU sel target grid pattern
//	COLOR_IDX__BMP_START 	= 0x10,	//A65_OPTOMA_Doulas_0020//start index for BMP
	/* 224 */ COLOR_IDX__USER_DEFINE 	= 169,
	/* 240 */ COLOR_IDX__BLINK 		    = 0xF0,
	/* 241 */ COLOR_IDX__BLACK_BLINK    = 0xF1,         //A65_OPTOMA_Doulas_0020

	/* 255 */ COLOR_IDX__TRANSPARENT    = 255,    //for ProAV transparent color index

#else

	/* 000 */ COLOR_IDX__GREEN = 0,
	/* 001 */ COLOR_IDX__MAGENTA = 1,
	/* 002 */ COLOR_IDX__RED = 2,
	/* 003 */ COLOR_IDX__CYAN = 3,
	/* 004 */ COLOR_IDX__BLUE = 4,
	/* 005 */ COLOR_IDX__YELLOW = 5,
	/* 006 */ COLOR_IDX__BLACK = 6,
	/* 007 */ COLOR_IDX__WHITE = 7,
	/* 008 */ COLOR_IDX__DARK_GREEN = 8,
	/* 009 */ COLOR_IDX__DARK_MAGENTA = 9,
	/* 010 */ COLOR_IDX__DARK_RED = 10,
	/* 011 */ COLOR_IDX__DARK_CYAN = 11,
	/* 012 */ COLOR_IDX__BLACKLEVEL_GREEN = 12,    //A65_OPTOMA_Doulas_0020
	/* 013 */ COLOR_IDX__BLACKLEVEL_MAGENTA = 13,  //A65_OPTOMA_Doulas_0020
	/* 014 */ COLOR_IDX__BLACKLEVEL_RED = 14,      //A65_OPTOMA_Doulas_0020
	/* 015 */ COLOR_IDX__BLACKLEVEL_CYAN = 15,     //A65_OPTOMA_Doulas_0020
	/* 016 */ COLOR_IDX__BLACK_BKG = 16,			//black for background
    /* 017 */ COLOR_IDX__GREY192 = 17,   //for ACU sel target grid pattern
//	COLOR_IDX__BMP_START 	= 0x10,	//A65_OPTOMA_Doulas_0020//start index for BMP
	/* 224 */ COLOR_IDX__USER_DEFINE 	= 0xE0,
	/* 240 */ COLOR_IDX__BLINK 		    = 0xF0,
	/* 241 */ COLOR_IDX__BLACK_BLINK    = 0xF1,         //A65_OPTOMA_Doulas_0020

	/* 255 */ COLOR_IDX__TRANSPARENT    = 0xFF,    //for ProAV transparent color index

#endif


	COLOR_IDX__INVALID
}eCOLOR_IDX;

typedef enum
{
	COLOR_PALETTE__OSD,   //for adv warp pattern
	COLOR_PALETTE__AP,
	COLOR_PALETTE__PNG_BEFORE_WARP,
	COLOR_PALETTE__PNG_AFTER_WARP,
	COLOR_PALETTE__SPRITE,
	COLOR_PALETTE__SCALER_OSD,
	COLOR_PALETTE__INVALID
}eCOLOR_PALETTE_GROUP; //color palette group

typedef enum
{
	GRID_COLOR__GREEN,
	GRID_COLOR__MAGENTA,
	GRID_COLOR__RED,
	GRID_COLOR__CYAN,

	GRID_COLOR__INVALID,
}eGRID_COLOR;

typedef enum
{
	BKG_COLOR__BLACK,
	BKG_COLOR__TRANSPARENT,
	BKG_COLOR__INVALID
}eBKG_COLOR; //background color

typedef enum
{
	OSD_LAYER_TYPE__MAIN,
	OSD_LAYER_TYPE__CURRENT,
	OSD_LAYER_TYPE__TEMP,
	OSD_LAYER_TYPE__BMP,
	OSD_LAYER_TYPE__INVALID,
}eOSD_LAYER_TYPE;

typedef enum
{
	WARP_CTRL__BASIC,
	WARP_CTRL__ADVANCED,
	WARP_CTRL__AP,
	WARP_CTRL__INVALID,
}eWARP_CTRL;	//warp control

typedef enum
{
	/* 00 */ PAT_TYPE__OFF,
	/* 01 */ PAT_TYPE__WARP_SEL_CTRL_POINT,	//select control point
	/* 02 */ PAT_TYPE__WARP_MOV_CTRL_POINT,	//move control point=warping
	/* 03 */ PAT_TYPE__WARP_NO_CTRL_POINT,	//only grid line, no control point
	/* 04 */ PAT_TYPE__SEL_BLEND_WIDTH,		//select blend width
	/* 05 */ PAT_TYPE__ADJ_BLEND_WIDTH,		//adjust blend width
	/* 06 */ PAT_TYPE__BLEND_NO_CTRL_POINT,  //only grid line, no control point //A35G2_BRC_Casper_0046


    /* 07 */ PAT_TYPE__EMPTY,                //solid black color

    /* 08 */ PAT_TYPE__BLACKLEVEL_SEL_POINT,  //blacklevel select control point  //A65_OPTOMA_Doulas_0020
    /* 09 */ PAT_TYPE__BLACKLEVEL_MOV_POINT,  //blacklevel move control point    //A65_OPTOMA_Doulas_0020
    /* 10 */ PAT_TYPE__BLACKLEVEL_ADD_POINT,  //blacklevel add control point     //A65_OPTOMA_Doulas_0020
    /* 11 */ PAT_TYPE__BLACKLEVEL_DEL_POINT,  //blacklevel delete control point  //A65_OPTOMA_Doulas_0020
    /* 12 */ PAT_TYPE__BLACKLEVEL_PREVIEW,  //blacklevel only grid line, no control point  //A65_OPTOMA_Doulas_0020


    /* 13 */ PAT_TYPE__TESTPATTERN_WHITE,          // full white screen
    /* 14 */ PAT_TYPE__TESTPATTERN_BLACK,          // full black screen with 1 pixel white boundary
    /* 15 */ PAT_TYPE__TESTPATTERN_GRID,       // 9x7 white grid with black background
    /* 16 */ PAT_TYPE__TESTPATTERN_GRIDCENTER,   // 3 center white grids with black background
    /* 17 */ PAT_TYPE__TESTPATTERN_BLACK_BORDER, // 9x7 black border with white background
    /* 18 */ PAT_TYPE__TESTPATTERN_BLACK_BORDER_MULTIPLESELECT,  //A65_OPTOMA_CL_0001 add for CU multiple selection //A35G2_CDS_CL_0001
    /* 19 */ PAT_TYPE__DRAW_SOLID_COLOR,  //A65_OPTOMA_CL_0002 //A35G2_CDS_CL_0001
	/* 20 */ PAT_TYPE__DRAW_RECT,  //A65_OPTOMA_CL_0006
	/* 21 */ PAT_TYPE__DRAW_RECTS,  //A65_OPTOMA_CL_0006
	/* 22 */ PAT_TYPE__DRAW_CIRCLE,  //A65_OPTOMA_CL_0006
	/* 23 */ PAT_TYPE__DRAW_CIRCLE_GRID,  //A65_OPTOMA_CL_0007
	/* 24 */ PAT_TYPE__DRAW_LINE,  //A65_OPTOMA_CL_0006
	/* 25 */ PAT_TYPE__DRAW_LINES,  //A65_OPTOMA_CL_0006
	/* 26 */ PAT_TYPE__DRAW_CHECKBOARD,  //A65_OPTOMA_CL_0006
	/* 27 */ PAT_TYPE__DRAW_CHECKBOARD_TWIST,  //A65_OPTOMA_CL_0006
	/* 28 */ PAT_TYPE__DRAW_PNG_BEFORE_WARP,
	/* 29 */ PAT_TYPE__DRAW_PNG_AFTER_WARP,
	/* 30 */ PAT_TYPE__DRAW_SPRITE,  //A65_OPTOMA_CL_0006
	/* 31 */ PAT_TYPE__DRAW_SPRITES,  //A65_OPTOMA_CL_0006
	PAT_TYPE__INVALID
}ePAT_TYPE; //osd test pattern type

typedef enum
{
	OSD_POS__AFTER_WARP,
	OSD_POS__BEFORE_WARP,
	OSD_POS__INVALID,
}eOSD_POS;

typedef enum
{
	BLEND_MODE__EDGE = 0,
	BLEND_MODE__DBD_OUTPUT_SIDE,	//dot by dot output side
	BLEND_MODE__DBD_INPUT_SIDE,		//dot by dot input side
	BLEND_MODE__INVALID,
} eBLEND_MODE;

typedef enum
{
	BLACKLEVEL_BT_RED,
	BLACKLEVEL_BT_GREEN,
	BLACKLEVEL_BT_BLUE,
	BLACKLEVEL_BT__NUM,
}eBLACKLEVEL_BT;  //A65_OPTOMA_Doulas_0020

typedef enum
{
	BLACKLEVEL_AREA__BOTTOM,
	BLACKLEVEL_AREA__TOP,
	BLACKLEVEL_AREA__NUM,
}eBLACKLEVEL_AREA;  //A65_OPTOMA_Doulas_0020

typedef enum
{
	CURSOR_TYPE__NORMAL,
	CURSOR_TYPE__DELETE,
	CURSOR_TYPE__ADD,
	CURSOR_TYPE__INVALID,
}eCURSOR_TYPE;  //A65_OPTOMA_Doulas_0020

typedef enum
{
	SEQ__COUNTERCLOCKWISE,
	SEQ__CLOCKWISE,
	SEQ__INVALID,
}eSEQ;  //A65_OPTOMA_Doulas_0020

typedef struct
{
    uint16 nYMin;
    uint16 nYMax;
}YBoundary;  //A65_OPTOMA_Doulas_0020

typedef struct
{
    uint8 nPointNum;
    PointT nPoint[BLACKLEVEL_NODE_MAX];
}Vertex;  //A65_OPTOMA_Doulas_0020

typedef struct
{
    uint8 nPointNum;
    uint16 nPoint[MAX_INTERSECTED_POINT];
}IntersectedPoint;  //A65_OPTOMA_Doulas_0020

typedef struct
{
    uint8 nPointNum;
    uint16 nXValue[MAX_INTERSECTED_POINT];
}XValueSet;  //A65_OPTOMA_Doulas_0020

typedef struct node
{
	PointT		stPos;
	struct node	*pPrev;
	struct node	*pNext;
}Node;  //A65_OPTOMA_Doulas_0020

typedef struct
{
	BOOL		bEnable[BLACKLEVEL_AREA__NUM];  //enable to apply blacklevel in C789
	uint8		nPointNum[BLACKLEVEL_AREA__NUM];
	PointT      pnPointList[BLACKLEVEL_AREA__NUM][BLACKLEVEL_NODE_MAX];  // 4~32 points
	uint8		pnColor[BLACKLEVEL_PALETTE_NUM][BLACKLEVEL_PALETTE_RGB];	// 16 Sets, 3bytes per set
	uint8       ucArea;				//A65_OPTOMA_Doulas_0020
	uint8       ucBoundary;			//A35G2_Owen_0002 //A65_OPTOMA_Doulas_0239
}OSD_BlackLevel;  //A65_OPTOMA_Doulas_0020

typedef struct
{
    float PM_WPKEYANG_H;
    float PM_WPKEYANG_V;
    float WPKEYANG_H_OLD;
    float WPKEYANG_V_OLD;
    float PM_WPPINWID_H;
    float PM_WPPINWID_V;
    INT16 PM_WPKEY4C_TLX;
    INT16 PM_WPKEY4C_TLY;
    INT16 PM_WPKEY4C_TRX;
    INT16 PM_WPKEY4C_TRY;
    INT16 PM_WPKEY4C_BLX;
    INT16 PM_WPKEY4C_BLY;
    INT16 PM_WPKEY4C_BRX;
    INT16 PM_WPKEY4C_BRY;
    UINT8 TABLE_COLOR;

    UINT8 POLATION_H;  //warp filter: auto calculated value
    UINT8 POLATION_V;  //warp filter: auto calculated value
    UINT8 m_cWarpFilterSelect_H;  //warp filter: OSD select value
    UINT8 m_cWarpFilterSelect_V;  //warp filter: OSD select value
    UINT8 m_cWarpFilterApValue_H;  //warp filter: AP select(or auto) value   //A35G2_Simon_0110
    UINT8 m_cWarpFilterApValue_V;  //warp filter: AP select(or auto) value   //A35G2_Simon_0110
    BOOL m_cWarpAutoFilter;     //Warp auto filter on/off
    BOOL m_cWarpAutoFilterAp;   //Warp AP auto filter on/off                 //A35G2_Simon_0110

 //   str_wppos4c WPPOS4C;
 //   str_pos3d pos_i[4];
 //   str_pos3d pos_r;
 //   str_pos3d pos_a[4];
 //   str_pos2d pos_o[4];

}sWARP_BASIC;

typedef struct
{
    UINT8   ucBlending_T_Enable;
    UINT8   ucBlending_B_Enable;
    UINT8   ucBlending_L_Enable;
    UINT8   ucBlending_R_Enable;
    UINT16  uiBlending_T_St;
    UINT16  uiBlending_B_St;
    UINT16  uiBlending_L_St;
    UINT16  uiBlending_R_St;
    UINT16  uiBlending_T_Width;
    UINT16  uiBlending_B_Width;
    UINT16  uiBlending_L_Width;
    UINT16  uiBlending_R_Width;
    UINT8   ucBlendingGamma;
    UINT32  ulEGBCT;
}sBLENDING_BASIC;


typedef struct
{
	sWARP_BASIC stWArpGeoPara;
	sBLENDING_BASIC stBlendingPara;
}
OSD_BASIC_WARP;	//G100_Doulas_0027 Add


typedef struct
{
	eWARP_POINT	eWarpPoint;
	BOOL		bWarpInnerOn;
	BOOL	    bWarpLimit; //A35G2_BRC_Casper_0047
	Point2f 	pfControlPoint[DEF_NUM_CUR_MAX_H][DEF_NUM_CUR_MAX_V];    //G100_Doulas_0059 Modify
	uint8		nWarpSharpness;
	eGRID_COLOR	eGridColor;
	eCOLOR_IDX	eCursorColor;
	eBKG_COLOR	eBkgColor;
	BOOL	    bBlendEnable;  //add for Barco Ptool to enable/disable blending //A35G2_BRC_Casper_0046
	uint16		nBlendWidthL;
	uint16		nBlendWidthR;
	uint16		nBlendWidthT;
	uint16		nBlendWidthB;
    uint8       bDrawOverlapGridOnWarpPattern;  // G50_Keven_0003 //A35G2_Wesley_0093
	uint8       bShowBlendOnWarpPattern;        // G50_Keven_0003 //A35G2_Wesley_0093
	uint16	    nBlendOffsetL;  //add for Barco Ptool //A35G2_BRC_Casper_0046
	uint16		nBlendOffsetR;  //add for Barco Ptool //A35G2_BRC_Casper_0046
	uint16		nBlendOffsetT;  //add for Barco Ptool //A35G2_BRC_Casper_0046
	uint16		nBlendOffsetB;  //add for Barco Ptool //A35G2_BRC_Casper_0046
	uint8		nOverlapGridNum;
	uint8		nBlendGamma;
	OSD_BlackLevel	stBlackLevel;  //A65_OPTOMA_Doulas_0020
}
OSD_WARP;

typedef struct
{
	uint8		nWarpLutH;
	uint8		nWarpLutV;
	eBLEND_MODE	eBlendMode;
	float		fBlendGamma;
	uint16		nBlendWidthL;
	uint16		nBlendWidthR;
	uint16		nBlendWidthT;
	uint16		nBlendWidthB;
	uint8		nFlashIdx;
	uint8		bIsDotByDotAreaValid;
	uint8		bBiasOn;
	uint8		bIsBiasAreaValid;
	uint8		pnBiasPaletteColor[16][3];	// 16 Sets, 3bytes per set
}
PC_WARP;

typedef struct
{
	eWARP_CTRL	eWarpCtrl;
	OSD_BASIC_WARP stOsdBasic;		//G100_Doulas_0027 Add
	OSD_WARP	stOsd;
	PC_WARP		stPc;
}
WARP_CONFIG;



#if 1



void utilWarp_GetDeviceId(void);
UINT8 utilWarp_SysInit(eRES_ID eCurrentResId, eRES_ID e3dResId);    //G100_Doulas_0065
uint8 utilWarp_SaveWarpConfig(void);  //A65_OPTOMA_Doulas_0020
uint8 utilWarp_SaveOsdPreset(uint8 nIdx);
uint8 utilWarp_LoadOsdPreset(uint8 nIdx);
uint8 utilWarp_ClearOsdPreset(void);
void utilWarp_ResetWarpConfig(void);
BOOL utilWarp_SetWarpCtrl(eWARP_CTRL eWarpCtrl);
eWARP_CTRL utilWarp_GetWarpCtrl(void);
BOOL utilWarp_SetWarpPoint(eWARP_POINT eWarpPoint);
eWARP_POINT utilWarp_GetWarpPoint(void);
BOOL utilWarp_SetWarpInner(BOOL bWarpInnerOn);
BOOL utilWarp_GetWarpInner(void);
BOOL utilWarp_SetOsdWarpSharpness(uint8 nIdx);
uint8 utilWarp_GetOsdWarpSharpness(void);
BOOL utilWarp_SetOverlapGridNum(uint8 nIdx);
uint8 utilWarp_GetOverlapGridNum(void);
void utilWarp_SetOutputTiming(eRES_ID eRes);
void utilWarp_SelectControlPoint(eDIR eDir);
BOOL utilWarp_MoveControlPoint(eDIR eDir);
void utilWarp_WriteDefaultWarpLut(void);
void utilWarp_WriteDefaultWarpTable(void);
BOOL utilWarp_SetOsdGridColor(eGRID_COLOR eColor, UINT8 RedrawPattern);
eGRID_COLOR utilWarp_GetOsdGridColor(void);
BOOL utilWarp_SetOsdBkgColor(eBKG_COLOR eColor, UINT8 ucRedrawPattern);
eBKG_COLOR utilWarp_GetOsdBkgColor(void);
void utilWarp_ShowOsdPattern(ePAT_TYPE eType);
ePAT_TYPE utilWarp_GetOsdPatternType(void);
void utilWarp_SetOsdPatternType(ePAT_TYPE eType);
void utilWarp_HideOsdPattern(BOOL bHide);	//A65_OPTOMA_Doulas_0211
void utilWarp_SelectOsdBlendWidth(eDIR eDir);
BOOL utilWarp_AdjustOsdBlendWidth(eADJ eAdj);
BOOL utilWarp_SetOsdBlendGamma(uint8 nIdx);
uint8 utilWarp_GetOsdBlendGamma(void);

void utilWarp_OsdBlackLevel_ResetConfig(void);        //A65_OPTOMA_Doulas_0020
BOOL utilWarp_OsdBlackLevel_SetEnable(BOOL bEnable);
BOOL utilWarp_OsdBlackLevel_GetEnable(void);
#if BLACKLEVEL_SEL_MODE			//A65_OPTOMA_Doulas_0146
void utilWarp_OsdBlackLevel_SelectOnThePattern(eDIR eDir);
#else
void utilWarp_OsdBlackLevel_SelectOnThePattern(eSEQ eSeq);
#endif
BOOL utilWarp_OsdBlackLevel_MovePoint(eDIR eDir);
BOOL utilWarp_OsdBlackLevel_IsPointAddable(void);
BOOL utilWarp_OsdBlackLevel_IsPointDeletable(void);
BOOL utilWarp_OsdBlackLevel_AddPoint(void);
BOOL utilWarp_OsdBlackLevel_DeletePoint(void);
BOOL utilWarp_OsdBlackLevel_SetArea(eBLACKLEVEL_AREA eAreaSelect);
eBLACKLEVEL_AREA utilWarp_OsdBlackLevel_GetArea(void);
BOOL utilWarp_OsdBlackLevel_SetBrightness(eADJ eAdj);
BOOL utilWarp_OsdBlackLevel_SetRed(uint8 nR);
uint8 utilWarp_OsdBlackLevel_GetRed(void);
BOOL utilWarp_OsdBlackLevel_SetGreen(uint8 nG);
uint8 utilWarp_OsdBlackLevel_GetGreen(void);
BOOL utilWarp_OsdBlackLevel_SetBlue(uint8 nB);
uint8 utilWarp_OsdBlackLevel_GetBlue(void);
OSD_BlackLevel utilWarp_OsdBlackLevel_GetBlackLevelData(void);
BOOL utilWarp_OsdBlackLevel_GetRewriteOsdBlackLevelArea(void);
BOOL utilWarp_OsdBlackLevel_GetOsdWarpParamChanged(void);
void utilWarp_OsdBlackLevel_SetOsdWarpParamChanged(BOOL bValue);
eBLACKLEVEL_AREA utilWarp_OsdBlackLevel_GetAreaSelection(void);
void utilWarp_PrintWarpConfig(void);

void utilWarp_Test(uint16 nCmd, uint16 nValue1, uint16 nValue2, uint16 nValue3, uint16 nValue4, uint16 nValue5);
//G100_Doulas_0027 start
void utilBlend_BlendWidthGet(UINT16 *ucBlend_LF,UINT16 *ucBlend_RT,UINT16 *ucBlend_TP,UINT16 *ucBlend_BM);
void utilBasicWarpSettingSet(sWARP_BASIC stWarpingBasic);
void utilBasicWarpSettingGet(sWARP_BASIC *stWarpingBasic);
void utilBasicBlendSettingSet(sBLENDING_BASIC stBlendingBasic);
void utilBasicBlendSettingGet(sBLENDING_BASIC *stBlendingBasic);
uint8 utilWarp_SaveMemoryPreset(uint8 nIdx);
uint8 utilWarp_LoadMemoryPreset(uint8 nIdx);
uint8 utilWarp_SaveBasicWarpMemoryPreset(uint8 nIdx);
uint8 utilWarp_SaveBasicBlendMemoryPreset(uint8 nIdx);
uint8 utilWarp_LoadBasicWarpMemoryPreset(uint8 nIdx);
uint8 utilWarp_LoadBasicBlendMemoryPreset(uint8 nIdx);
void utilWarp_GetADVWarpConfig(OSD_WARP *stADVWarp);
void utilWarp_SetADVWarpConfig(OSD_WARP stADVWarp);
UINT8 utilWarp_ADVCurrentWarpReloadFile(void *stRead);
UINT8 utilWarp_ADVCurrentWarpSaveFile(void *stWrite,UINT16 uisize);
UINT8 utilWarp_ADVMemoryWarpReloadFile(void *stRead,UINT8 ucIndex);
UINT8 utilWarp_ADVMemoryWarpSaveFile(void *stWrite,UINT16 uisize,UINT8 ucIndex);
UINT8 utilWarp_ADVMemoryWarpClearFile(UINT8 ucIndex);
UINT8 utilWarp_ADVWarpClearAllFile(void);
//G100_Doulas_0027 end
void utilWarp_Panel_Vstart_Offset_Set(INT16 ucOffset);	//G100_Doulas_0043
INT16 utilWarp_Panel_Vstart_Offset_Get(void);				//G100_Doulas_0043
//command for Barco Ptool set //A35G2_BRC_Casper_0046
BOOL utilWarp_GetWarpPatternState(void);
UINT8 utilWarp_GridPoint_GetColIndex(void);			//G100_Doulas_0067
UINT8 utilWarp_GridPoint_GetRowIndex(void);			//G100_Doulas_0067
BOOL utilWarp_GridPoint_SetColIndex(UINT8 nPointX);
BOOL utilWarp_GridPoint_SetRowIndex(UINT8 nPointY);
BOOL utilWarp_GridPoint_SetColRowIndex(UINT8 nPointX, UINT8 nPointY);
INT32 utilWarp_GridPoint_GetColPosition(void);
INT32 utilWarp_GridPoint_GetRowPosition(void);
BOOL utilWarp_GridPoint_MoveUp(UINT16 nMovePixel);
BOOL utilWarp_GridPoint_MoveDown(UINT16 nMovePixel);
BOOL utilWarp_GridPoint_MoveLeft(UINT16 nMovePixel);
BOOL utilWarp_GridPoint_MoveRight(UINT16 nMovePixel);
BOOL utilWarp_GridPoint_SetPositionMove(int nPosX, int nPosY);
BOOL utilWarp_GetBlendEnable(void);
BOOL utilWarp_SetBlendEnable(BOOL bEnable);
BOOL utilWarp_GetBlendPatternState(void);
UINT16 utilWarp_GetOsdBlendWidthTop(void);
UINT16 utilWarp_GetOsdBlendWidthDown(void);
UINT16 utilWarp_GetOsdBlendWidthLeft(void);
UINT16 utilWarp_GetOsdBlendWidthRight(void);
BOOL utilWarp_SetOsdBlendWidthTop(UINT16 nOverlapPixel);
BOOL utilWarp_SetOsdBlendWidthDown(UINT16 nOverlapPixel);
BOOL utilWarp_SetOsdBlendWidthLeft(UINT16 nOverlapPixel);
BOOL utilWarp_SetOsdBlendWidthRight(UINT16 nOverlapPixel);
BOOL utilWarp_SetOsdBlendWidth(eDIR eDir, UINT16 nOverlapPixel);
UINT16 utilWarp_GetOsdBlendOffsetTop(void);
UINT16 utilWarp_GetOsdBlendOffsetDown(void);
UINT16 utilWarp_GetOsdBlendOffsetLeft(void);
UINT16 utilWarp_GetOsdBlendOffsetRight(void);
BOOL utilWarp_SetOsdBlendOffsetTop(UINT16 nOffsetPixel);
BOOL utilWarp_SetOsdBlendOffsetDown(UINT16 nOffsetPixel);
BOOL utilWarp_SetOsdBlendOffsetLeft(UINT16 nOffsetPixel);
BOOL utilWarp_SetOsdBlendOffsetRight(UINT16 nOffsetPixel);
BOOL utilWarp_SetOsdBlendOffset(eDIR eDir, UINT16 nOffsetPixel);
void utilWarp_ResetBlendConfig(void);
void utilWarp_ResetWarpSetting(void);
BOOL utilWarp_GetWarpLimit(void); //A35G2_BRC_Casper_0047
eWARP_LIMITATION utilWarp_SetWarpLimit(BOOL bWarpLimit); //A35G2_BRC_Casper_0047
void utilWarp_ResetWarpControlPoint(void); //A35G2_BRC_Casper_0047
void utilWarp_ReDrawWarpPattern(void); //A35G2_BRC_Casper_0047

UINT8 utilWarp_Grid_Point_COL_Get(void);			//G100_Doulas_0067
UINT8 utilWarp_Grid_Point_ROW_Get(void);			//G100_Doulas_0067
void utilWarp_OsdBlackLevel_ResetConfigBottom(void);	//A65_OPTOMA_Doulas_0029
void utilWarp_OsdBlackLevel_ResetConfigTop(void);		//A65_OPTOMA_Doulas_0029

#if (ENABLE_COLOR_UNIFORMITY == TRUE)   //G100_Tim_0012, add, start
void DrawOsdAcuGridPattern(void);
void DrawOsdAcuCenterGridPattern(void);
void DrawOsdAcuBlackBorderPattern(void);
#endif //ENABLE_COLOR_UNIFORMITY        //G100_Tim_0012, add, end
void OSD_Color_Init(void);              //G100_Tim_0023, add
void EnableTestPattern(BOOL bEnable, eBKG_COLOR eBkgColor, eAPPLY_BLEND eApplyBlend, eAPPLY_BLACKLEVEL eApplyBlackLevel, eOSD_POS bBeforeWarp);

void utilWarp_SelectCUGrid(sCOLOR_UNIFORMITY_POINT_SELECT *psCUPointSelect);  //A65_OPTOMA_CL_0001 add for CU multiple selection  //A35G2_CDS_CL_0001
void utilWarp_SetOsdTransparentColor(eCOLOR_IDX eIndex);  //A65_OPTOMA_CL_0002 //A35G2_CDS_CL_0001
eCOLOR_PALETTE_GROUP utilWarp_GetColorPaletteGroupIndex(void);  //A65_OPTOMA_CL_0006
BOOL utilWarp_SetColorPaletteGroupIndex(eCOLOR_PALETTE_GROUP eColorPaletteIndex);  //A65_OPTOMA_CL_0006
void utilWarp_CopyCircleInfo(sWARPOSD_CIRCLE_GRID_DRAW_INFO *psCircleInfo);  //A65_OPTOMA_CL_0007
UINT8 utilWarp_GetChecksLayerMode(void);
BOOL utilWarp_GetApBlendApply(void);  //A65_OPTOMA_CL_0010
void utilWarp_SetApBlendApply(BOOL bBlendApply);  //A65_OPTOMA_CL_0010
BOOL utilWarp_GetApOnlyBlendApply(void);
void utilWarp_SetApOnlyBlendApply(BOOL bOnlyBlendApply);
BOOL utilWarp_GetApBlacklevelApply(void);  //A65_OPTOMA_CL_0010
void utilWarp_SetApBlacklevelApply(BOOL bBlacklevelApply);  //A65_OPTOMA_CL_0010
UINT16 utilWarp_C789_OSD_H_Start_Get(void);		//A65_OPTOMA_Doulas_0232
UINT16 utilWarp_C789_OSD_V_Start_Get(void);		//A65_OPTOMA_Doulas_0232
void utilWarp_Black_Level_Boundary_Flag_Set(UINT8 ucBoundary);		 //A35G2_Owen_0002 //A65_OPTOMA_Doulas_0239
void utilWarp_CopyChecksInfo(sWARPOSD_CHECKBOARD_DRAW_INFO *psChecksInfo);  //A65_OPTOMA_CL_0015

BOOL utilWarp_SetShowBlendOnWarpPattern(BOOL bShow);        //GG50_Keven_0003, add           //A35G2_Wesley_0093
BOOL utilWarp_GetShowBlendOnWarpPattern(void);        //G50_Keven_0003, add //A35G2_Wesley_0093
BOOL utilWarp_SetDrawOverlapGridOnWarpPattern(BOOL bDraw); // G50_Keven_0003, add //A35G2_Wesley_0093
BOOL utilWarp_GetDrawOverlapGridOnWarpPattern(void); // G50_Keven_0003, add //A35G2_Wesley_0093
UINT32 utilWarp_NativePanel_Get(void);
BOOL utilWarp_GridPoint_SetAllPosition(Point2f Data[DEF_NUM_CUR_MAX_H][DEF_NUM_CUR_MAX_V]);


#else

/////////////////////////////////////////////////////////////////////////////
//for compiler error
/////////////////////////////////////////////////////////////////////////////

#define utilWarp_GetOsdPatternType()    0
#define utilWarp_SelectCUGrid(a)
#define utilWarp_CopyChecksInfo(a)
#define halADVWarpSettingSet(a)
#define utilBlend_BlendWidthGet(a,b,c,d)

//#define utilBasicWarpSettingGet(a)  palGeo_WarpingParameterAllGet(a);
//#define utilBasicBlendSettingGet(a) palGeo_BlendParameterAllGet(a);
#define utilWarp_CopyCircleInfo(a)
#define halADVWarpSettingGet(a) 0
#define utilWarp_ADVWarpClearAllFile() 0
#define utilWarp_MoveControlPoint(a) 0
#define utilWarp_PrintWarpConfig()
#define utilWarp_SelectControlPoint(a)
#define utilWarp_GridPoint_GetColIndex() 0
#define utilWarp_GetWarpPatternState() 0
#define halAdvWarpGP_Move(a,b) 0
#define utilWarp_GridPoint_GetColPosition() 0
#define utilWarp_GridPoint_GetRowPosition() 0
#define utilWarp_GetBlendEnable() 0
#define utilWarp_GetBlendPatternState() 0
#define halAdvBlending_Width(a,b) 0
#define utilWarp_GetOsdBlendWidthTop() 0
#define utilWarp_GetOsdBlendWidthDown() 0
#define utilWarp_GetOsdBlendWidthLeft() 0
#define utilWarp_GetOsdBlendWidthRight() 0
#define halAdvBlending_Offset(a,b) 0
#define utilWarp_GetOsdBlendOffsetTop() 0
#define utilWarp_GetOsdBlendOffsetDown() 0
#define utilWarp_GetOsdBlendOffsetLeft() 0
#define utilWarp_GetOsdBlendOffsetRight() 0
#define utilWarp_GridPoint_GetRowIndex() 0


#define utilWarp_SetApBlendApply(a)
#define utilWarp_SetApOnlyBlendApply(a)
#define utilWarp_SetApBlacklevelApply(a)
#define utilWarp_ShowOsdPattern(a)
#define utilWarp_GetApBlendApply()
#define utilWarp_GetApOnlyBlendApply()
#define utilWarp_GetApBlacklevelApply()

#define utilWarp_GetShowBlendOnWarpPattern() 0
#define utilWarp_SetShowBlendOnWarpPattern(a) 0
#define utilWarp_GetDrawOverlapGridOnWarpPattern() 0
#define utilWarp_SetDrawOverlapGridOnWarpPattern(a) 0

#endif





#endif /* _UTIL_WARP_DEMO_PROAV_H_ */

