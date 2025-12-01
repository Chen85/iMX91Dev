#ifndef _COMMONTYPE_H_
#define _COMMONTYPE_H_

#include "type_def.h"
#include "CommonData.h"

// ---------------------------------------------------------------------------
// Web page warp tmp date. No support set alignment to 1
// ---------------------------------------------------------------------------
typedef struct
{
    float x;
    float y;
} WebPoint2f;


#pragma pack(push)  /* push current alignment to stack */
#pragma pack(1)     /* set alignment to 1 byte boundary */

/************************************************************************************************/
//palGui
/************************************************************************************************/


//define for OSD object read status
#define NO_SPECIFIC_READING 0
#define MENU_READING        1
#define ITEM_READING        2
#define BITMAP_READING      3
#define TEXT_READING        4
#define TEXT_STRING_READING 5


//API Return Status
#define GUI_PASS  1
#define GUI_FOUND 1
#define ERROR_GUI_NOT_FOUND -1
#define ERROR_GUI_ITEM_SIZE_OVER_LIMIT -2
#define ERROR_GUI_EMPTY_STRING -3
#define ERROR_GUI_NOT_ALLOW_NEGATIVE -4
#define ERROR_GUI_SEMAPHORE_FAIL -5    //A70LV_Doulas_0014


#define GUI_ON 1
#define GUI_OFF 0

#define CJSON_GET_PASS  1
#define ERROR_CJSON_GET_FAIL  -1
#define ERROR_CJSON_PARSER_FAIL  -2
#define ERROR_CJSON_ITEM_IS_EMPTY  -3
#define ERROR_CJSON_NO_DEFINE_NAMESTRING  -4
#define ERROR_CJSON_OSD_READ_SIZE_OVER_LIMIT  -5
#define ERROR_CJSON_PALETTE_SIZE_NOT_MATCH  -6
#define ERROR_CJSON_FILE_OPEN_FAIL  -7
#define ERROR_OSD_STRUCT_LINK_FAIL  -8


//define for test useage color
#define GUI_C_RED 246
#define GUI_C_GREEN 247
#define GUI_C_BLUE 248
#define GUI_C_CYAN 253
#define GUI_C_YELLOW 249
#define GUI_C_MAGENTA 254
#define GUI_C_WHITE 250
#define GUI_C_BLACK 252 //A70LV_Larry_0046
#define GUI_C_GRAY 239  //T100_Simon_0028

//#define GUI_TRANSPARENT_COLOR_1 253
//#define GUI_TRANSPARENT_COLOR_2 254

#if defined(CUSTOM_BARCO) || defined(CUSTOM_OPTOMA)	 || defined(CUSTOM_FUJIFILM) //ZU860_Clare_0001
#define GUI_C_LIGHT_BLACK 240
#define GUI_C_YESNO_HILIGHT 211
#define GUI_C_YESNO_HILIGHT_BLACK 252
#define GUI_C_ACT_MENU_COLOR 241
#define GUI_C_ACT_ITEM_COLOR 242
#define GUI_C_INA_MENU_COLOR 243
#define GUI_C_INA_ITEM_COLOR 244
#define GUI_C_FOCUS_BAR 245
#if defined(CUSTOM_OPTOMA)	 || defined(CUSTOM_FUJIFILM)
#define GUI_C_SLIDER_BAR 250// BruceLin#20201216, 240                //Slider Bar             [#aaaaaa; 170,170,170]
#define GUI_C_SLIDER_BAR_BACKGROUND 244//239// //A65_OPTOMA_Doulas_0042 Modify [79,79,79] BruceLin#20201216, 252     //Slider Bar Background  [#000000; 0,0,0]
#else
#define GUI_C_SLIDER_BAR 240// BruceLin#20201216, 240                //Slider Bar             [#aaaaaa; 170,170,170] //A35G2_BRC_Casper_0034
#define GUI_C_SLIDER_BAR_BACKGROUND 252// BruceLin#20201216, 252     //Slider Bar Background  [#000000; 0,0,0] //A35G2_BRC_Casper_0034
#endif
#if defined(CUSTOM_FUJIFILM)
#define GUI_C_MENU_GUIDE 252 //241                //Menu Guide [#332f2f; 51,47,47] // Keven recover
#define GUI_C_MAIN_BACKGROUND 242           //Main Menu Background   [#6f6f6f; 111,111,111] // Keven recover
#define GUI_C_SUB_MENU_ITEM 242             //Sub Menu Item          [#393635; 57,54,53] // Keven recover
#define GUI_C_CONFIRM_BACKGROUND 242            //Message Background     [#aaaaaa; 170,170,170]
#else
#define GUI_C_MENU_GUIDE 241                //Menu Guide             [#332f2f; 51,47,47]
#define GUI_C_MAIN_BACKGROUND 239           //Main Menu Background   [#6f6f6f; 111,111,111]
#define GUI_C_SUB_MENU_ITEM 243             //Sub Menu Item          [#393635; 57,54,53]
#endif
#define GUI_C_RED_LINE 246
#define GUI_C_ITEM_ON_FOCUS 238             //Item On Focus          [#007bc7; 0,123,199]
#define GUI_C_MSG_BACKGROUND 240            //Message Background     [#aaaaaa; 170,170,170]
#define GUI_C_SUB_MENU_BACKGROUND 242       //Sub Menu Background    [#3e3a39; 62,58,57]
#else
#define GUI_C_LOGO_COLOR 240       //logo color
#define GUI_C_ACT_MENU_COLOR 241   //submenu item highlight
#define GUI_C_ACT_ITEM_COLOR 242   //main item highlight and submenu item background
#define GUI_C_INA_MENU_COLOR 243   //submenu title
#define GUI_C_INA_ITEM_COLOR 244   //main item background and submenu item grayout color
#define GUI_C_FOCUS_BAR 245        //
#if defined(CUSTOM_BARCO) || defined(CUSTOM_OPTOMA) || defined(CUSTOM_FUJIFILM)
#define GUI_C_SLIDER_BAR 238//250                // white
#elif CUSTOM_CHRISTIE
#define GUI_C_SLIDER_BAR 251//250                // white
#endif
#define GUI_C_SLIDER_BAR_BACKGROUND 250//239     // [137, 137, 137]
#define GUI_C_RED_LINE 246
#endif /* CUSTOM_OPTOMA */

#define GUI_INHITBIT_COLOR 255
#define GUI_STRING_ASCII_FONT_OFFSET 0

typedef enum //T100_Sander_0002
{
    ebiNON,
	ebiFULL,
	ebiHIGHLIGHT,
	ebiFULL2,   //EK816U_626U_Energy_0002 add new rectangle for eiki
    ebiMAX_TYPE
} eBOARDER_INDEX;


typedef enum
{
    ensNON,         //Without sign character display for positive number only
    ensNEGATIVE,    //Display negative sign character only(use space character instead of positive sign character)
    ensDUO,         //Display sign character both negative and positive
	ensSTRING, 		//Add . for Menu  ////T100_Sander_0002

    ensMAX_TYPE
} eNUMERIC_SIGN;

/////////////////////////////////////////////////////////////////////
///ENUM Object Align Type
/////////////////////////////////////////////////////////////////////
typedef enum
{
    eoaLEFT,        //Align left
    eoaCENTERED,    //Centered string
    eoaRIGHT,       //Align right
    eoaLEN_MEMORY_LIST_STRING,	//ZU860_Clare_0027,

    eoaMAX_TYPE
} eOBJECT_ALIGN ;

typedef struct
{
    INT16 iX;
    INT16 iY;
} START_POINT_INFO;

typedef struct
{
    INT16 iWidth;
    INT16 iHeight;
} RECT_SIZE_INFO;

typedef struct
{
    UINT8 ucT_Color1;
    UINT8 ucT_Color2;
    UINT8 ucT_Color3;
    UINT8 ucT_Color4;
} OSD_TRANSPARENCY_COLOR_INFO;


typedef struct
{
    INT16 iX;
    INT16 iY;
} COORDINATE;

typedef struct
{
    COORDINATE sLTR;  //Left to Right coordinate for most language used
    COORDINATE sRTL;  //Right to Left coordinate for Arabic language used
} POSITION;


typedef struct   //H2 wait review
{
    INT16 iWidth;
    INT16 iHeight;
}BLOCK_SIZE_INFO;

typedef struct _BITMAP_INFO
{
    UINT32 ulBitmapID;
    COORDINATE sSrc_Coordinate;  //use to record bitmap's start position in memory
    BLOCK_SIZE_INFO sBitmap_Size ;  //used to record bitmap size information
} BITMAP_INFO;


typedef struct _STRING_INFO
{
    UINT8 ucStringMaxHeight;
    UINT16 uiStringWidth;
    UINT16 uiStringLength;
    UINT32 ulFontOffset;
    UINT16 *puiCharIndex;
} STRING_INFO;


typedef struct _TEXT_INFO
{
    UINT32 ulTextID;
    UINT16 uiTextMaxWidth;
    UINT8 ucTextMaxHeight;
    STRING_INFO *psString;
} TEXT_INFO;


// ---------------------------------------------------------------------------
// color definition data strucutre
// ---------------------------------------------------------------------------
typedef struct tagRGB
{
    UINT8                ucRed;          // red color component
    UINT8                ucGreen;        // green color component
    UINT8                ucBlue;         // blue color component
} RGB, *PRGB;
typedef const RGB *PCRGB;


/////////////////////////////////////////////////////////////////////
///Item Infomation
/////////////////////////////////////////////////////////////////////
typedef struct _ITEM_DRAWING_INFO
{
    UINT16 		uiDisplayStyle;  //Item display style
	POSITION 	sStartPosition;   //Menu start position
    BLOCK_SIZE_INFO 	sSizeInfo;  //Use to record item width and height information.
    UINT8 		ucBitmapCount;  //The number of Bitmap in item
    UINT32 		*pulBitmapID;  //Point to array of Bitmap
    UINT32 		*pulRectangleID;    //Point to array of Rectangle
    UINT8 		ucTextCount;  //The number of Text in item
    UINT32 		*pulTextID;  	//Point to array of Text
	UINT16 		*pulSliderColor; //Point to array of Slider Bar Color //T100_Sander_0002
} ITEM_DRAWING_INFO;


typedef struct _ITEM_INFO
{
    UINT32 	ulItemID;  //Item ID ex. MAIN_MENU 1693597542
    INT16 	iDataCode;  //Item's data code use to sync item's relation data and action function
    UINT32 	ulParentMenuID;  //Parent Menu ID ex. MAIN_MENU_ICOUNT 0
    INT16 	iOperateType;  //Item's operate type
    INT16 	ItemListBitmapNumber1;  //Item's List Number  //T100_Sander_0012
    INT16 	ItemListBitmapNumber2;  //Item's List Number
    INT16 	ItemListTextNumber1;  	//Item's List Number  //T100_Sander_0012
    INT16 	ItemListTextNumber2;  	//Item's List Number  //T100_Sander_0012
    INT16 	ItemGrayOut;  			//Item's GrayOut  		//T100_Sander_0016
    INT16 	ItemControl;
    UINT32 	ulNextMenuID;  //Next level menuID

    ITEM_DRAWING_INFO sItemDrawingInfo;
} ITEM_INFO;


/////////////////////////////////////////////////////////////////////
///Menu Information
/////////////////////////////////////////////////////////////////////
typedef struct _MENU_DRAWING_INFO
{
    UINT16 uiDisplayStyle;  //Menu display style
    POSITION sStartPosition;   //Menu start position
    BLOCK_SIZE_INFO sSizeInfo;   // Menu's size information
    INT16 iItemGap;  // Menu's item gap
    UINT8 ucString_OnFocus_Color;  //Menu's string on focus color index
    UINT8 ucString_NonFocus_Color;  //Menu's string non-focus color index
    UINT8 ucItem_OnFocus_Color;  //Menu's item on focus color index
    UINT8 ucItem_NonFocus_Color;  //Menu's item non-focus color index
} MENU_DRAWING_INFO;


typedef struct _MENU_INFO
{
    UINT32 ulMenuID;  //Menu ID
    UINT16 uiItemCount;  //Menu's Item count
    UINT16 uiOnePageMaxItem;  //Menu's maximum item display count at one page

    MENU_DRAWING_INFO sMenuDrawingInfo;

    ITEM_INFO **ppsMenuItemList;  //Menu's Item list (link at initial function)
} MENU_INFO;


typedef struct _OSD_BITMAP_INFO
{
    START_POINT_INFO sSrc_Position;  //use to record bitmap's start position in memory
    RECT_SIZE_INFO sBitmap_Size ;  //used to record bitmap size information
} OSD_BITMAP_INFO;


//Font set use one byte to indicate his character width
typedef struct _OSD_STRING_INFO
{
    UINT8 ucStringMaxHeight;
    UINT16 uiStringLength;
    UINT32 ulFontOffset;
    UINT16 *puiCharIndex;
    UINT8 *pucCharWdith;
} OSD_STRING_INFO;


/************************************************************************************************/
/************************************************************************************************/
// UI
/************************************************************************************************/
/************************************************************************************************/

typedef enum
{
	eLOGO_OSD_DRIVER_FACTORY_LOGO,
	eLOGO_OSD_DRIVER_NONE,
	eLOGO_OSD_DRIVER_BLUE,
	eLOGO_OSD_DRIVER_RED,
	eLOGO_OSD_DRIVER_GREEN,
	eLOGO_OSD_DRIVER_GRAY,
    eLOGO_OSD_DRIVER_WHITE,
    eLOGO_OSD_DRIVER_BLACK,

	eLOGO_OSD_DRIVER__MAX_NUMBER
} eLOGO_OSD_DRIVER;        //ZU860_Doulas_0006

typedef enum
{
    eOSD_NOT_LOCKED,
    eOSD_LOCKED,
    eOSD_LOCK_NUMBER
} eOSD_LOCK_STATUS;


/************************************************************************************************/
/************************************************************************************************/
//palWarping
/************************************************************************************************/
/************************************************************************************************/

#if 0
typedef enum
{
    eWDT_BEFORE_WARP,
    eWDT_AFTER_WARP,

    eWDT_INVALID,
}eWARPOSD_DRAW_TYPE;
#endif


/************************************************************************************************/
/************************************************************************************************/
//palMotorMgr
/************************************************************************************************/
/************************************************************************************************/

typedef enum
{
    eLENS_TYPE_NA,

    //A70 Type
    eLENS_TYPE_A11,         //1
    eLENS_TYPE_A18,         //2
    eLENS_TYPE_A19,         //3
    eLENS_TYPE_A20,         //4
    eLENS_TYPE_A21,         //5
    eLENS_TYPE_A22,         //6
    eLENS_TYPE_A23,         //7
    eLENS_TYPE_A27,         //8
    eLENS_TYPE_UST,         //9
    eLENS_TYPE_REV0,        //10
    eLENS_TYPE_REV1,        //11
    eLENS_TYPE_REV2,        //12
    eLENS_TYPE_REV3,        //13
    eLENS_TYPE_REV4,        //14
    eLENS_TYPE_REV5,        //15
    eLENS_TYPE_REV6,        //16
    eLENS_TYPE_REV7,        //17

    //A35 Type
    eLENS_TYPE_A01,         //1
    eLENS_TYPE_A02,         //2
    eLENS_TYPE_A05,         //3
    eLENS_TYPE_A03,         //4
    eLENS_TYPE_A06,         //5
    eLENS_TYPE_A13,         //6
    eLENS_TYPE_A15,         //7
    eLENS_TYPE_A15F,        //8
    eLENS_TYPE_A16,         //9
    eLENS_TYPE_A17,         //10
    eLENS_TYPE_KMUST,       //11
    eLENS_TYPE_Y06,         //12
    eLENS_TYPE_A12,         //13
    eLENS_TYPE_REX0,        //14
    eLENS_TYPE_REX1,        //15
    eLENS_TYPE_REX2,        //16
    eLENS_TYPE_REX3,        //17
    eLENS_TYPE_REX4,        //18

    //H60 Type
    eLENS_TYPE_H_A11,       //1
    eLENS_TYPE_H_T20C1,     //2
    eLENS_TYPE_H_A20,       //3
    eLENS_TYPE_H_A21,       //4
    eLENS_TYPE_H_A22,       //5
    eLENS_TYPE_H_A23,       //6
    eLENS_TYPE_H_A27,       //7
    eLENS_TYPE_H_UST,       //8
    eLENS_TYPE_H_FUJI_UST,  //9
    eLENS_TYPE_REH1,        //10
    eLENS_TYPE_REH2,        //11
    eLENS_TYPE_REH3,        //12
    eLENS_TYPE_REH4,        //13
    eLENS_TYPE_REH5,        //14
    eLENS_TYPE_REH6,        //15
    eLENS_TYPE_REH7,        //16
    eLENS_TYPE_REH8,        //17
    eLENS_TYPE_REH9,        //18

    eLENS_TYPE_MAX_NUMBER
} eLENS_TYPE;   //G100_Owen_0017

typedef enum                                                //A35LH_JS_0036 modify
{
    eLENS_CMDS_STEP_LEFT,   //0
    eLENS_CMDS_STEP_RIGHT,  //1
    eLENS_CMDS_STEP_UP,     //2
    eLENS_CMDS_STEP_DOWN,   //3

    eLENS_CMDS_RUN_LEFT,    //4
    eLENS_CMDS_RUN_RIGHT,   //5
    eLENS_CMDS_RUN_UP,      //6
    eLENS_CMDS_RUN_DOWN,    //7

    eLENS_CMDS_GO_LEFT,     //8
    eLENS_CMDS_GO_RIGHT,    //9
    eLENS_CMDS_GO_TOP,      //10
    eLENS_CMDS_GO_DOWN,     //11

    eLENS_CMDS_CALIBRAION,  //12
    eLENS_CMDS_CANCEL,      //13

    eLENS_CMDS_MOVING,      //14

    eLENS_CMDS_OE_TEST,     //15
    eLENS_CMDS_POSTION_SET, //16
    eLENS_CMDS_POSTION_GET, //17

    eLENS_CMDS_A16_CALIBRATION,  //18  //A70LH_Doulas_0241 add
    eLENS_CMDS_A16_RANGE,        //19  //A70LH_Doulas_0241 add
    eLENS_CMDS_OLD_RANGE,        //20  //A70LH_Doulas_0241 add

    eLENS_CMDS_PIXEL_LEFT,	//21
	eLENS_CMDS_PIXEL_RIGHT,	//22
	eLENS_CMDS_PIXEL_UP, 	//23
	eLENS_CMDS_PIXEL_DOWN,	//24

    eLENS_CMDS_RUN_LEFT_DOUBLE,    //25
    eLENS_CMDS_RUN_RIGHT_DOUBLE,   //26
    eLENS_CMDS_RUN_UP_DOUBLE,      //27
    eLENS_CMDS_RUN_DOWN_DOUBLE,    //28

    eLENS_CMDS_ZOOM_CALIBRAION,    //29
    eLENS_CMDS_FOCUS_CALIBRAION,   //30
    eLENS_CMDS_LENS_ID_CAL,        //31

    eLENS_CMDS_SHIFT_TOP,          //32
    eLENS_CMDS_SHIFT_BOTTOM,       //33
    eLENS_CMDS_SHIFT_CENTER,       //34
    eLENS_CMDS_SHIFT_H_CENTER,     //35
    eLENS_CMDS_SHIFT_V_CENTER,     //36
    eLENS_CMDS_SHIFT_HV_CENTER,    //37

    eLENS_CMDS_RUN_LEFT_WITHOUT_SENSOR,    //38
    eLENS_CMDS_RUN_RIGHT_WITHOUT_SENSOR,   //39
    eLENS_CMDS_RUN_UP_WITHOUT_SENSOR,      //40
    eLENS_CMDS_RUN_DOWN_WITHOUT_SENSOR,    //41

    eLENS_CMDS_RELEASE = 101,      //101

    eLENS_CMDS_NUMBERS,
} eLENS_CMDS;

typedef enum
{
    eZOOMLENS_CMD_ZOOM_IN_STEP,     //0 zoom in 放大
    eZOOMLENS_CMD_ZOOM_IN_RUN,      //1
    eZOOMLENS_CMD_ZOOM_OUT_STEP,    //2 zoom out 縮小
    eZOOMLENS_CMD_ZOOM_OUT_RUN,     //3
    eZOOMLENS_CMD_FOCUS_NEAR_STEP,  //4
    eZOOMLENS_CMD_FOCUS_NEAR_RUN,   //5
    eZOOMLENS_CMD_FOCUS_FAR_STEP,   //6
    eZOOMLENS_CMD_FOCUS_FAR_RUN,    //7
    eZOOMLENS_CMD_ZOOM_GO_POS,      //8
    eZOOMLENS_CMD_FOCUS_GO_POS,     //9
    eZOOMLENS_CMD_ZOOM_CAL,         //10
    eZOOMLENS_CMD_FOCUS_CAL,        //11
    eZOOMLENS_CMD_ZOOM_GO_MAX,      //12
    eZOOMLENS_CMD_FOCUS_GO_MAX,     //13
    eZOOMLENS_CMD_ZOOM_GO_MIN,      //14
    eZOOMLENS_CMD_FOCUS_GO_MIN,     //15
    eZOOMLENS_CMD_ZOOM_GO_CENT,     //16
    eZOOMLENS_CMD_FOCUS_GO_CENT,    //17
    eZOOMLENS_CMD_FOCUS_NEAR_STEP_DURATION,  // 18  //A65_OPTOMA_CL_0011
    eZOOMLENS_CMD_FOCUS_FAR_STEP_DURATION,   //19
    eZOOMLENS_CMD_BACK_FOCUS_NEAR_STEP,  //20
    eZOOMLENS_CMD_BACK_FOCUS_NEAR_RUN,   //21
    eZOOMLENS_CMD_BACK_FOCUS_FAR_STEP,   //22
    eZOOMLENS_CMD_BACK_FOCUS_FAR_RUN,    //23
    eZOOMLENS_CMD_BACK_FOCUS_RELAY_RESET,   //24

    eZOOMLENS_CMD_NUMBERS,
} eZOOMLENS_CMD;


typedef struct //A70_Larry_0284
{
    UINT16  wMaximumValue;
    UINT16  wMinimumValue;
} sMOTOR_LIMIT,*psMOTOR_LIMIT;

typedef struct //A70_Larry_0380 fixed //A70_Larry_0416 fixed
{
    UINT32 wLensHPosition;
    UINT32 wLensVPosition;
    UINT16 wZoomPosition;
    UINT16 wFocusPosition;
    UINT8 cDirH;
    UINT8 cDirV;
    UINT8 cDirZoom;
    UINT8 cDirFocus;
    UINT8 cRecord;
} sLENSMEMORY,*psLENSMEMORY;

typedef struct
{
    UINT32 dwMaxValueH;
    UINT32 dwMinValueH;
    UINT32 dwMaxValueV;
    UINT32 dwMinValueV;
} sLENS_LIMIT;


typedef struct
{
    UINT32 wLensHPosition;
    UINT32 wLensVPosition;
    UINT16 wZoomPosition;
    UINT16 wFocusPosition;
    UINT8 cDirH;
    UINT8 cDirV;
    UINT8 cDirZoom;
    UINT8 cDirFocus;
    UINT16 wZoomMaximumValue;
    UINT16 wZoomMinimumValue;
    UINT16 wFocusMaximumValue;
    UINT16 wFocusMinimumValue;
    UINT16 wBacklashH;
    UINT16 wBacklashV;
    UINT32 ulLensIDAdcValue;
    UINT32 ulLensCenterH;
    UINT32 ulLensCenterV;
    UINT32 ulFullMaxValueH;
    UINT32 ulFullMinValueH;
    UINT32 ulFullMaxValueV;
    UINT32 ulFullMinValueV;
    UINT32 ulActiveMaxValueH;
    UINT32 ulActiveMinValueH;
    UINT32 ulActiveMaxValueV;
    UINT32 ulActiveMinValueV;
    UINT16 wLensSpeed;
    UINT16 wLensEepromVersion;
    UINT8 cHomeDirH;
    UINT8 cHomeDirV;
    UINT8 cSensorStateH;
    UINT8 cSensorStateV;
    UINT32 ulLensCenterOffsetH;
    UINT32 ulLensCenterOffsetV;
    UINT8 cBoundaryCalibration;

}sLENS_INFO; //A70LV_Larry_0287

typedef struct
{
    UINT8  acCoreSN[32];
    UINT8  acVerdorSN[32];
    UINT8  acCustomerSN[32];
    INT32  lLensNvramId;

    INT32  lMaxBrightness;
    INT32  lThrowRatioMax;
    INT32  lThrowRatioMin;
    INT32  lController;
    INT32  lEncoder;

    INT32  alEncoderLimitZoom[2];
    INT32  alEncoderLimitFocus[2];
    INT32  alEncoderLimitFocus2[2];

} sLENS_HEADER;

typedef enum
{
    eLENS_LOCK_ALL_ALLOW,
    eLENS_LOCK_ALL_LOCKED,

    eLENS_LOCK_ALL_MAX_NUMBER
} eLENS_LOCK_ALL_MOTORS;


/************************************************************************************************/
/************************************************************************************************/
//palFWUpgradeMgr
/************************************************************************************************/
/************************************************************************************************/

typedef enum
{
    eLPCMCU_INDEX_FRONT_2K,
    eLPCMCU_INDEX_MOTORBD,
    eLPCMCU_INDEX_SYSTEM,
    eLPCMCU_INDEX_FRONT_4K,
    eLPCMCU_INDEX_FMT_2K,
    eLPCMCU_INDEX_FRONT,

    eLPCMCU_INDEX_NUMBERS,
}eLPCMCU_INDEX;


/************************************************************************************************/
/************************************************************************************************/
//palInputKeyMgr
/************************************************************************************************/
/************************************************************************************************/

typedef struct
{
    UINT32  wKeyCode;
    UINT8   eKeyEvent;
    UINT8   eKeyType;
} sKEY_DATA;

typedef enum
{
    eKEYINPUT_KEYPAD,
    eKEYINPUT_IR,
    eKEYINPUT_LAN,
    eKEYINPUT_HDBaseT,
    eKEYINPUT_CLI,
    eKEYINPUT_WIRED,
    eKEYINPUT_BACK,

    eKEYINPUT_NUMBER,
} eKEY_INPUT_TYPE;


/************************************************************************************************/
/************************************************************************************************/
//palLEDMgr
/************************************************************************************************/
/************************************************************************************************/

typedef enum
{
    eLED_LIST_BLUE_MENU,
    eLED_LIST_RED_MENU,
    eLED_LIST_RED_LEFT,
    eLED_LIST_BLUE_AUTO,
    eLED_LIST_RED_AUTO,
    eLED_LIST_BLUE_UP,
    eLED_LIST_RED_UP,
    eLED_LIST_BLUE_ENTER,
    eLED_LIST_RED_ENTER,
    eLED_LIST_RED_DOWN,
    eLED_LIST_BLUE_INPUT,
    eLED_LIST_RED_INPUT,
    eLED_LIST_BLUE_RIGHT,       //reserve
    eLED_LIST_RED_RIGHT,
    eLED_LIST_BLUE_BACK,
    eLED_LIST_RED_BACK,
    eLED_LIST_BLUE_LENS,
    eLED_LIST_RED_LENS,
    eLED_LIST_BLUE_SHUTTER,
    eLED_LIST_RED_SHUTTER,
    eLED_LIST_BLUE_ZOOM,
    eLED_LIST_RED_ZOOM,         //reserve
    eLED_LIST_BLUE_FOCUS,
    eLED_LIST_RED_FOCUS,        //reserve

    eLED_LIST_BLUE_HELP,
    eLED_LIST_RED_HELP,
    eLED_LIST_BLUE_OSD,
    eLED_LIST_RED_OSD,
    eLED_LIST_BLUE_PWR,
    eLED_LIST_RED_PWR,

    eLED_LIST_STATUS_SHUTTER_RED,
    eLED_LIST_STATUS_SHUTTER_BLUE,

    eLED_LIST_NUMBERS,
} eLED_LIST;

typedef enum
{
    eKEYPAD_LED_BACKLIGHT_05,
    eKEYPAD_LED_BACKLIGHT_10,
    eKEYPAD_LED_BACKLIGHT_20,
    eKEYPAD_LED_BACKLIGHT_30,
    eKEYPAD_LED_ALWAYSON,
    eKEYPAD_LED_ALWAYSOFF,

    eKEYPAD_LED_NUMBERS,
} eKEYPAD_BACKLIGHT;

typedef enum
{
    eSTATUSLED_ALWAYSON,
    eSTATUSLED_ALWAYSOFF,
    eSTATUSLED_WARNINGERROR,

    eSTATUSLED_NUMBERS,
} eLED_STATUS;

typedef enum
{
    eLED_STATUS_POWERON,
    eLED_STATUS_POWEROFF,
    eLED_STATUS_WARMUP,
    eLED_STATUS_LAMPIGNITE,
    eLED_STATUS_NORMAL,
    eLED_STATUS_LAMP_OUT_OF_LIFE,
    eLED_STATUS_COOLING,
    eLED_STATUS_OVER_TEMP_COOLING,
    eLED_STATUS_FANLOCK_COOLING,
    eLED_STATUS_LMPFAIL_COOLING,
    eLED_STATUS_BALLASTFAIL_COOLING,
    eLED_STATUS_CWFAIL_COOLING,
    eLED_STATUS_SHUTTER_ON,
    eLED_STATUS_SHUTTER_OFF,
    eLED_STATUS_FW_UPGRAGE,
    eLED_STATUS_NORMAL_BURNIN_OFF,
    eLED_STATUS_IR_BROADCAST,
    eLED_STATUS_IR_DIRECT,
    eLED_STATUS_LED_ALWAYS_OFF,
    eLED_STATUS_WARNNING,
    eLED_STATUS_NTC_DETECTED,
    eLED_STATUS_DA_WARRING,
    eLED_STATUS_ROTATION_FAIL,
    eLED_STATUS_POWER_LOST,
    eLED_STATUS_FACTORY_RESET_ON,
    eLED_STATUS_LD_OVERTEMP,
    eLED_STATUS_LD_SHUNTCIRCUIT,
    eLED_STATUS_PUMP_FAIL,
    eLED_STATUS_PUMP_WARNNING,
    eLED_STATUS_FAN_STALL,
    eLED_STATUS_LD_OCP,
    eLED_STATUS_NORMAL_BURNIN_ON,
    eLED_STATUS_LENS_UNPLUGGED,
    eLED_STATUS_LENS_INSTALLING,

    eLED_STATUS_NUMBERS,
} eLED_BEHAVIOR;


typedef enum
{
    eBACKLIGHT_EVENT_NORMAL,
	eBACKLIGHT_EVENT_ALWAYS_ON,
	eBACKLIGHT_EVENT_ALWAYS_OFF,

	eBACKLIGHT_EVENT_NUMBER,
}eBACKLIGHT_EVENT;



/************************************************************************************************/
/************************************************************************************************/
//palLEDMgr
/************************************************************************************************/
/************************************************************************************************/
typedef struct
{
    UINT8 ucOsdIndex;
    UINT8 ucTimeZoneStr[8];
    UINT8 ucAreaStr[32];
    UINT8 ucDstStr[32];
} sTIME_ZONE_INFO_LUT;





/************************************************************************************************/
/************************************************************************************************/
//palFormatterMgr
/************************************************************************************************/
/************************************************************************************************/

typedef enum
{
    /*0000*/  eOS_TABLE_TOP,
    /*0001*/  eOS_CEILING_MOUNT,
    /*0002*/  eOS_REAR_PROJECTION,
    /*0003*/  eOS_CEILING_REAR,

    eOS_INVALID,

}eORIENTATION_STATE;

typedef struct
{
    UINT16 	RedGain;
    UINT16 	GreenGain;
    UINT16 	BlueGain;
} sWALL_COLOR_STRUCT;


typedef struct
{
    UINT8  HSG_R_HUE ;// Hue.
    UINT8  HSG_R_SAT;//Saturation.
    UINT8  HSG_R_GAIN;//

    UINT8  HSG_G_HUE ;// Hue.
    UINT8  HSG_G_SAT;//Saturation
    UINT8  HSG_G_GAIN;//

    UINT8  HSG_B_HUE ;// Hue.
    UINT8  HSG_B_SAT;//Saturation.
    UINT8  HSG_B_GAIN;//

    UINT8  HSG_C_HUE ;// Hue.
    UINT8  HSG_C_SAT;//Saturation.
    UINT8  HSG_C_GAIN;//

    UINT8  HSG_M_HUE ;// Hue.
    UINT8  HSG_M_SAT;//Saturation.
    UINT8  HSG_M_GAIN;//

    UINT8  HSG_Y_HUE ;// Hue.
    UINT8  HSG_Y_SAT;//Saturation.
    UINT8  HSG_Y_GAIN;//

    UINT8  HSG_W_R_GAIN;
    UINT8  HSG_W_G_GAIN;
    UINT8  HSG_W_B_GAIN;
}sHSG_SETTING,*PsHSG_SETTING;
typedef struct
{
    UINT8 	ucRedGain;
    UINT8 	ucGreenGain;
    UINT8 	ucBlueGain;
} sHSG_COLOR_TEMPERATURE;

typedef enum
{
    eWAP_POWERLEVEL_100,
    eWAP_POWERLEVEL_90,
    eWAP_POWERLEVEL_80,
    eWAP_POWERLEVEL_70,
    eWAP_POWERLEVEL_60,
    eWAP_POWERLEVEL_50,
    eWAP_POWERLEVEL_40,
    eWAP_POWERLEVEL_30,
    eWAP_POWERLEVEL_20,
    eWAP_POWERLEVEL_10,

    eWAP_POWERLEVEL_NUMBER
}eWAP_POWERLEVEL;


/************************************************************************************************/
/************************************************************************************************/
// System
/************************************************************************************************/
/************************************************************************************************/


typedef enum
{
    ePOWER_STATE_RESET,           /* system is in the reset power state */
    ePOWER_STATE_STANDBY,         /* system is in the standby power state */
    ePOWER_STATE_WARMUP,          /* system is in the warm up state */ //A70LV_Larry_0135
    ePOWER_STATE_ACTIVE,          /* system is in the active power state */
    ePOWER_STATE_COOLING,         /* system is cooling */
    ePOWER_STATE_UPGRADE,         /* system is cooling (for upgrade) */ //A70LV_Larry_0121

    ePOWER_STATE_NUMBER,
}ePOWER_STATE;

typedef enum
{
    ePOWER_CONTROL_POWEROFF,
    ePOWER_CONTROL_POWERON,
    ePOWER_CONTROL_UPGRADE,

    ePOWER_CONTROL_INVALID
}ePOWER_CONTROL;

typedef struct
{
    UINT8  PowerStatus;
    UINT32 TimerCount;
}sPOWER_STATUS_INFO;


//Christie OSD: PIN Protect
//Optoma OSD: Security
typedef enum
{
    ePASSWORD_PROTECT_LOCKED = 0,
    ePASSWORD_PROTECT_UNLOCK = 99,
    ePASSWORD_PROTECT_FIRST_STARTUP,//no used

    ePASSWORD_PROTECT_STATUS_NUMBERS = 0xFF,

} ePASSWORD_PROTECT_STATUS;


/************************************************************************************************/
/************************************************************************************************/
// Misc
/************************************************************************************************/
/************************************************************************************************/



/************************************************************************************************/
/************************************************************************************************/
// Source / Input / Datapath
/************************************************************************************************/
/************************************************************************************************/

typedef enum
{
    //PC
    /*00*/ePANEL_ID_XGA_60HZ,
    /*01*/ePANEL_ID_WXGA_60HZ,
    /*02*/ePANEL_ID_1600x1200_60HZ,
    /*03*/ePANEL_ID_WUXGA_60HZ,
    /*04*/ePANEL_ID_WQXGA_60HZ,//2560x1600

    //VIDEO
    /*05*/ePANEL_ID_720P_50HZ,
    /*06*/ePANEL_ID_720P_60HZ,//Done
    /*07*/ePANEL_ID_1080P_50HZ,
    /*08*/ePANEL_ID_1080P_60HZ,//Done
    /*09*/ePANEL_ID_3840x2160_50HZ,//4K2K, 3840x2160
    /*10*/ePANEL_ID_4096x2160_50HZ,//4K2K

    /*11*/ePANEL_ID_3840x2160_60HZ,//4K2K, 3840x2160, Done
    /*12*/ePANEL_ID_3840x2160_30HZ,
    /*13*/ePANEL_ID_3840x2160_60HZ_594MHz, //3840x2160 60Hz 594MHz
    /*14*/ePANEL_ID_3840x2400_60HZ,//4K2K, Done
    /*15*/ePANEL_ID_3840x2400_50HZ,
    /*16*/ePANEL_ID_3840x2400_30HZ,
    /*17*/ePANEL_ID_4096x2160_60HZ,
    /*18*/ePANEL_ID_960x2160_50HZ,//Quarter 4K , 960x2160
    /*19*/ePANEL_ID_960x2160_60HZ,//Quarter 4K , 960x2160, Done

    /*20*/ePANEL_ID_1280x2160_50HZ,//One of third for C790
    /*21*/ePANEL_ID_1280x2160_60HZ,//One of third for C790
    /*22*/ePANEL_ID_2560x1440_60HZ,
    /*23*/ePANEL_ID_2560x1600_60HZ,

    //3D
    /*24*/ePANEL_ID_XGA_120HZ,
    /*25*/ePANEL_ID_WXGA_120HZ,
    /*26*/ePANEL_ID_720P_120HZ,
    /*27*/ePANEL_ID_1080P_120HZ,
    /*28*/ePANEL_ID_1080P_240HZ,
    /*29*/ePANEL_ID_2688x1472_120HZ,//For MST MEMC in front of Scaler
    /*30*/ePANEL_ID_2716x1528_120HZ,//For TI 3D Limitation
    /*31*/ePANEL_ID_SVGA_120HZ,       //A70LV_Doulas_0094 800x600
    /*32*/ePANEL_ID_WUXGA_120HZ,      //A70LV_Doulas_0094 1920x1200
    /*33*/ePANEL_ID_WUXGA_240HZ,
    /*34*/ePANEL_ID_1920x2400_60HZ,   //4K3D dual pipe


    ePANEL_ID_LAST,
} ePANEL_ID;


typedef enum
{
    eSOURCE_WINDOW_MAIN,
    eSOURCE_WINDOW_SUB,
    eSOURCE_WINDOW_BOTH,

    eSOURCE_WINDOW_MAX_NUMBER,
} eSOURCE_WINDOW;


typedef struct
{
    UINT8 cInput0RX0_AVI[8];
    UINT8 cInput0RX0_VSI[8];
    UINT8 cInput0RX1_AVI[8];
    UINT8 cInput0RX1_VSI[8];
    UINT8 cInput0RX2_AVI[8];
    UINT8 cInput0RX2_VSI[8];
    UINT8 cInput0RX3_AVI[8];
    UINT8 cInput0RX3_VSI[8];
    UINT8 cInput1RX0_AVI[8];
    UINT8 cInput1RX0_VSI[8];
    UINT8 cInput1RX1_AVI[8];
    UINT8 cInput1RX1_VSI[8];
    UINT8 cInput1RX2_AVI[8];
    UINT8 cInput1RX2_VSI[8];
    UINT8 cInput1RX3_AVI[8];
    UINT8 cInput1RX3_VSI[8];
}sSOURCE_AVI_INFO;


typedef enum
{
    eINPUT_3D_TYPE_OFF,
    eINPUT_3D_TYPE_FRAMEPACKING,
    eINPUT_3D_TYPE_TOPANDBOTTOM,
    eINPUT_3D_TYPE_SIDEBYSIDE,
    eINPUT_3D_TYPE_FRAME_SEQUENTIAL,    //120hz
    eINPUT_3D_TYPE_DAUL_PIPE,
    eINPUT_3D_TYPE_4K3D_SEQUENTIAL,
    eINPUT_3D_TYPE_4K3D_DAUL_PIPE,

    eINPUT_3D_TYPE_INVALID,
} eINPUT_3D_TYPE;    //A70LV_Doulas_0154

/************************************************************************************************/
/************************************************************************************************/
// Camera
/************************************************************************************************/
/************************************************************************************************/

typedef enum
{
    eCAMERA_STATUS_IDLE = 0,            // camera is idle.
    eCAMERA_STATUS_WORKING,             // camera is working ( AF, AWC, ACU )

    eCAMERA_STATUS_MAX
} eCAMERA_STATUS;

typedef enum
{
    eCAMERA_MODULE_NOT_EXIST,
    eCAMERA_MODULE_EXIST,
    eCAMERA_MODULE_NUMBER
} eCAMERA_MODULE_STATUS;

//For Datacode 741 edcAUTO_COLOR value
typedef enum
{
    eAF_AC_NOT_EXECUTE,
    eAF_AC_EXECUTE,           //with OSD(UI) message, and check previos/current setting after processing AC cal.  (ex: from Christie OSD/Webpage)
    eAF_AC_EXECUTE_NO_CHECK,  //with OSD(UI) message, but no check previos/current setting after processing AC cal. (ex: from Christie RS232 CLI, from Barco/Optoma OSD/Webpage)
    eAF_AC_EXECUTE_NO_UIMSG,  //without OSD(UI) message  (ex: from AP)
    eAF_AC_NUMBER
} eAUTO_IMAGE_SET_EXECUTE;

typedef enum
{
    /* 000 */eAF_STATUS_PROCESSING,
    /* 001 */eAF_STATUS_SUCCESS,
    /* 002 */eAF_HID_CONNECT_ERR,
    /* 003 */eAF_UVC_CONNECT_ERR,
    /* 004 */eAF_READ_INITIAL_FILE_ERR,
    /* 005 */eAF_GET_FOCUS_VALUE_ERR,
    /* 006 */eAF_SET_ROI_ERR,
    /* 007 */eAF_SELECT_ROI_ERR,
    /* 008 */eAF_GET_FOCUS_PATTERN_ERR,
    /* 009 */eAF_GET_COLOR_WHEEL_SPD_ERR,
    /* 010 */eAF_GET_CELLING_REAR_ERR,
    /* 011 */eAF_SEARCH_ROI_ERR,
    /* 012 */eAF_TAKE_PIC_BEFORE_ERR,
    /* 013 */eAF_TAKE_PIC_AFTER_ERR,
    /* 014 */eAF_GET_WRGB_ERR,
    /* 015 */eAF_OVER_EXPOSURE_ERR,
    /* 016 */eAF_WRGB_ERR,
    /* 017 */eAF_SET_AUTO_GAIN_ERR,         //G100_Tim_0047, add, start
    /* 018 */eAF_CAMERA_BUSY_ERR,
    /* 019 */eAF_CAMERA_GENERAL_ERR,        //G100_Tim_0047, add, end
    /* 020 */eAF_SET_CAMERA_ERR,
    /* 021 */eAF_LENSLOCK_ERR,
    /* 022 */eAF_GET_CLEAR_POSITION_ERR,
    /* 023 */eAF_AUTO_TUNE_ERR,

    /* 099 */eAF_STATUS_TIMEOUT = 99,
}eAUTO_FACUS_STATUS;

typedef enum
{
    /* 000 */eAWC_STATUS_PROCESSING,
    /* 001 */eAWC_STATUS_SUCCESS,
    /* 002 */eAWC_HID_CONNECT_ERR,
    /* 003 */eAWC_UVC_CONNECT_ERR,
    /* 004 */eAWC_READ_INITIAL_FILE_ERR,
    /* 005 */eAWC_GET_COLOR_WHEEL_SPD_ERR,
    /* 006 */eAWC_AWB_ERR,
    /* 007 */eAWC_SEARCH_ROI_ERR,
    /* 008 */eAWC_GET_WRGB_ERR,
    /* 009 */eAWC_SET_GAIN_ERR,
    /* 010 */eAWC_CAMERA_BUSY_ERR,          //G100_Tim_0047, add //A35G2_BRC_Casper_0126
    /* 011 */eAWC_CAMERA_GENERAL_ERR,       //G100_Tim_0047, add //A35G2_BRC_Casper_0126
    /* 012 */eAWC_INPUTSOURCE_ERR,
    /* 013 */eAWC_NOTEVER_ERR,
    /* 014 */eAWC_COMMAND_PARA_ERR,
    /* 015 */eAWC_TUNE_EXPOSURE_ERR,
    /* 016 */eAWC_TUNE_WHITEBALANCE_ERR,
    /* 017 */eAWC_AUTOTUNE_OVEREXPOSURE,
    /* 018 */eAWC_AUTOTUNE_UNDEREXPOSURE,
    /* 019 */eAWC_LOCK3A_ERR,
    /* 020 */eAWC_TAKEPICTURE_ERR,
    /* 021 */eAWC_SETSHUTTER_ERR,

    /* 099 */eAWC_STATUS_TIMEOUT = 99,

} eAUTO_WALL_COLOR_STATUS;//A35G2_Coda_0052 //A35G2_Coda_0055

typedef enum
{
    /* 000 */eACU_STATUS_PROCESSING,             // ACU calibrating
    /* 001 */eACU_STATUS_SUCCESS,                // successfully
    /* 002 */eACU_STATUS_TARGET_POSITION_ERR,
    /* 003 */eACU_STATUS_HID_CONNECT_ERR,
    /* 004 */eACU_STATUS_SYSTEM_ERR,
    /* 005 */eACU_STATUS_TEST_PATTERN_ERR,              //deprecated, id:5
    /* 006 */eACU_STATUS_CAMERA_CONFIG_ERR,
    /* 007 */eACU_STATUS_GET_CAMERA_ERR,                //deprecated, id:7
    /* 008 */eACU_STATUS_CAMERA_ROI_ERR,
    /* 009 */eACU_STATUS_CAMERA_ROUGH_AUTO_TUNE_ERR,    //deprecated, id:9
    /* 010 */eACU_STATUS_CAMERA_AUTO_TUNE_ERR,
    /* 011 */eACU_STATUS_FIND_POINTS_ERR,
    /* 012 */eACU_STATUS_TAKE_PICTURE_ERR,
    /* 013 */eACU_STATUS_GET_RGB_ERR,
    /* 014 */eACU_STATUS_CALCULATE_UNIFORMITY_ERR,
    /* 015 */eACU_STATUS_BUSY_ERR,
    /* 016 */eACU_STATUS_COLOR_WHEEL_SPEED_NOT_SUPPORT_ERR,
    /* 017 */eACU_STATUS_GAMMA_NOT_SUPPORT_ERR,
    /* 018 */eACU_STATUS_OVER_EXPOSURE_ERR,
    /* 019 */eACU_STATUS_UNDER_EXPOSURE_ERR,
    /* 020 */eACU_STATUS_FUNCTION_NOT_ENABLE_ERR,
    /* 021 */eACU_STATUS_FIND_TARGET_ERR,

    /* 099 */eACU_STATUS_TIME_OUT = 99,          // time out
    eACU_STATUS_NONE                    // no ACU result
}eACU_RESULT_STATUS;


typedef enum
{
    /* 000 */eGCS_STATUS_PROCESSING = 0,
    /* 001 */eGCS_STATUS_SUCCESS = 1,
    /* 002 */eGCS_STATUS_FUNCTION_FAIL = 2,
    /* 003 */eGCS_STATUS_STATE_BUSY = 3,
    /* 004 */eGCS_STATUS_STATE_FAIL = 4,
    /* 005 */eGCS_STATUS_PRJECTOR_CONNET_FAIL = 5,
    /* 006 */eGCS_STATUS_PRJECTOR_REMOTE_CONNECTION_DISCONNETED = 6,
    /* 007 */eGCS_STATUS_PRJECTOR_INNER_CONNECTION_DISCONNETED = 7,
    /* 008 */eGCS_STATUS_CONFIGURATION_ERROR = 8,
    /* 009 */eGCS_STATUS_CONFIGURATION_INFO_NO_MASTER = 9,
    /* 010 */eGCS_STATUS_COMMAND_TIMEOUT = 10,
    /* 011 */eGCS_STATUS_COMMAND_PARAMETER_ERROR = 11,
    /* 012 */eGCS_STATUS_COMMAND_SALVE_NOT_SUPPORT = 12,
    /* 013 */eGCS_STATUS_COMMAND_FUNCTION_IS_NOT_RUNNING = 13,
    /* 014 */eGCS_STATUS_COMMAND_FUNCTION_IS_NOT_SUPPORT = 14,
    /* 015 */eGCS_STATUS_USER_ABORT = 15,

    /* 099 */eGCS_STATUS_TIMEOUT = 99,
} eGROUPING_CLIENT_STATUS;

typedef enum
{
    eAC_APPLY_CALIBRATED = 0x00,
    eAC_APPLY_PREVIOUS = 0x01,

    eAC_APPLY_IDLE = 0xFF
}eAC_APPLY_TYPE;


//For Datacode 819 edcACU_EXECUTE value
typedef enum
{
    eACU_NOT_EXECUTED,        // The ACU calibration is not executed
    eACU_EXECUTING,           // Execute the ACU calibration with OSD(UI) Msg, and check previos/current setting after processing ACU cal.  (ex: from Christie OSD/Webpage)
    eACU_EXECUTING_NO_CHECK,  // Execute the ACU calibration with OSD(UI) Msg, but no check previos/current setting after processing ACU cal. (ex: from Christie RS232 CLI, from Barco/Optoma OSD/Webpage)
    eACU_EXECUTING_NO_UIMSG,  // Execute the ACU calibration without OSD(UI) Msg   (ex: from AP)

    eACU_EXECUTE_MAX
} eACU_EXECUTE_STATUS;

typedef enum
{
    eACU_RESET_NOT_YET,                 // The ACU data is not reset yet
    eACU_RESET_DONE,                    // The ACU data is reset

    eACU_RESET_MAX
} eACU_RESET_STATUS;

typedef enum
{
    eACU_APPLY_CALIBRATED = 0x00,
    eACU_APPLY_PREVIOUS = 0x01,

    eACU_APPLY_IDLE = 0xFF
}eACU_APPLY_TYPE;

typedef enum
{
    eACM_RESULT_PROCESSING = 0,                          // 0
    eACM_RESULT_SUCCESS = 1,                             // 1

    //for AF Error message[[
    eACM_AF_STATUS_PROCESSING = 30,
    eACM_AF_STATUS_SUCCESS,
    eACM_AF_HID_CONNECT_ERR,
    eACM_AF_UVC_CONNECT_ERR,
    eACM_AF_READ_INITIAL_FILE_ERR,
    eACM_AF_GET_FOCUS_VALUE_ERR,
    eACM_AF_SET_ROI_ERR,
    eACM_AF_SELECT_ROI_ERR,
    eACM_AF_GET_FOCUS_PATTERN_ERR,
    eACM_AF_GET_COLOR_WHEEL_SPD_ERR,
    eACM_AF_GET_CELLING_REAR_ERR,
    eACM_AF_SEARCH_ROI_ERR,
    eACM_AF_TAKE_PIC_BEFORE_ERR,
    eACM_AF_TAKE_PIC_AFTER_ERR,
    eACM_AF_GET_WRGB_ERR,
    eACM_AF_OVER_EXPOSURE_ERR,
    eACM_AF_WRGB_ERR,
    eACM_AF_SET_AUTO_GAIN_ERR,
    eACM_AF_CAMERA_BUSY_ERR,
    eACM_AF_CAMERA_GENERAL_ERR,
    eACM_AF_SET_CAMERA_ERR,
    eACM_AF_LENSLOCK_ERR,
    eACM_AF_GET_CLEAR_POSITION_ERR,
    eACM_AF_AUTO_TUNE_ERR,
    //]]

    eACM_RESULT_IDLE = 99,                               // 99
    //-- ACM Warning
    eACM_RESULT_AUTO_TUNE_OVER_ITERATION_LIMIT = 100,    // 100
    eACM_RESULT_INITIAL_CAMERA_TUNE_GAIN_ISOVEREXPOSURE, // 101
    eACM_RESULT_AUTO_WALL_COLOR_NOT_REACH_TARGET,        // 102

    //-- Common Error
    eACM_RESULT_FAIL = 200,                              // 200
    eACM_RESULT_PARAMETER_INVALID,                       // 201
    eACM_RESULT_TUNING_OVER_PROJECTOR_LIMIT,             // 202
    eACM_RESULT_TUNING_NOT_DEFINE_COLOR,                 // 203
    eACM_RESULT_POWER_LEVEL_NOT_CONFIGURABLE,            // 204

    //-- Camera Error
    eACM_RESULT_CONNECT_CAMERA_FAIL = 300,               // 300
    eACM_RESULT_INITIAL_CAMERA_FAIL,                     // 301
    eACM_RESULT_INITIAL_CAMERA_SEARCH_ROI_FAIL,          // 302
    eACM_RESULT_INITIAL_CAMERA_SET_AWB_FAIL,             // 303
    eACM_RESULT_INITIAL_CAMERA_GETCOLORWHEELSPEED_FAIL,  // 304
    eACM_RESULT_CAMERA_BUSY,                             // 305
    eACM_RESULT_CAMERA_SETTING_FAIL,                     // 306

    //-- Tuning Camera Gain Error
    eACM_RESULT_AUTO_TUNE_CAMERA_GAIN_ISOVEREXPOSURE = 400,  // 400, Tune Gain is Over Exposure.
    eACM_RESULT_AUTO_TUNE_CAMERA_GAIN_FAIL,                  // 401, Tuning Gain Fail
    eACM_RESULT_AUTO_TUNE_CAMERA_GAIN_RGB_UNREASONABLE,      // 402,
    eACM_RESULT_AUTO_TUNE_CAMERA_GAIN_ISUNDEREXPOSURE,       // 403, Tune Gain is Under Exposure.

    //-- Auto Color Error --
    //eACM_RESULT_AUTO_WALL_COLOR_NOT_REACH_TARGET = 500, // Revise as Warning Code.

    eACM_RESULT_USER_ABORT = 900,

    eACM_RESULT_NUM
}eACM_RESULT;

typedef enum
{
    eACM_ER_MSG_PROJECTOR_STATE,
    eACM_ER_MSG_PROJECTOR_COMMUNICATION_FAIL,
    eACM_ER_MSG_OUT_OF_RANGE,
    eACM_ER_MSG_CAMERA_DISCONNECT,
    eACM_ER_MSG_CALIBRATION_FAIL,
    eACM_ER_MSG_IMAGE_RECOGNITION_ERROR,
    eACM_ER_MSG_CAMERA_BUSY,
    eACM_ER_MSG_IMAGE_IS_OVEREXPOSED,
    eACM_ER_MSG_IMAGE_IS_UNDEREXPOSED,
    eACM_ER_MSG_COLORWHEEL_NOTSUPPORT, //for auto focus
    eACM_ER_MSG_IMAGE_CAPTURE_ERROR, //for auto focus
    eACM_ER_MSG_LENSLOCK_NOTSUPPORT, //for auto focus

    eACM_ER_MSG_NUM
}eACM_ERROR_MESSAGE;

/************************************************************************************************/
/************************************************************************************************/
// Datacode access
/************************************************************************************************/
/************************************************************************************************/

typedef enum
{
    edaREAD,  //Read data from RAM
    edaWRITE_RAM_ONLY_WITH_ACTION,  //write data to memory only and execute action function
    edaWRITE_THROUGH_WITH_ACTION,  //sync data to storage device(EEPROM) and execute action function
    edaWRITE_RAM_ONLY_NO_ACTION,  //write data to memory only
    edaWRITE_THROUGH_NO_ACTION,  //sync data to storage device(EEPROM)

    edaINVALID,
} eDATA_ACCESS_MODE;

typedef enum
{
    rcERROR     = 0,
    rcSUCCESS   = 1,
    rcBUSY      = 2,

    rcINVALID,
} eRESULT;

typedef enum
{
    eMODEL_SWITCH_DEFAULT,
    eMODEL_SWITCH_NEUTRAL,
    eMODEL_SWITCH_UNKNOW,
}eMODEL_SWITCH_INDEX; //A35G2_Coda_0063


typedef enum
{
    eDMD_TYPE_S600,
    eDMD_TYPE_TYPEA,
    eDMD_TYPE_NUMBER,
}eDMD_TYPE; //A35G2_Coda_0090

typedef enum
{
    eMODEL_REGION_0,
    eMODEL_REGION_1,
    eMODEL_REGION_2,

    eMODEL_REGION_NUMBER,
}eMODEL_REGION_INDEX;

#define MODEL_TYPE_NUMBER (MODEL_ID_INVALID)
#define MODEL_REGION_NUMBER (eMODEL_REGION_NUMBER)
//#define MODEL_NAME_MAX_LENGTH (20)




/************************************************************************************************/
/************************************************************************************************/
// Geometry
/************************************************************************************************/
/************************************************************************************************/
#define BLENDING_AP_GEOMETRY_PATH                     "/mnt/configs/Geometry"
#define BLENDING_AP_HICC2_SAVING_PATH                 "/mnt/configs/Geometry/HICC2"
#define BLENDING_AP_HICC2_WARP_SAVE_FILENAME          "/mnt/configs/Geometry/HICC2/Warp%d"
#define BLENDING_AP_HICC2_DBD_SAVE_FILENAME           "/mnt/configs/Geometry/HICC2/Blend%d"
#define BLENDING_AP_HICC2_BKLEVEL_SAVE_FILENAME       "/mnt/configs/Geometry/HICC2/BlackLevel%d"

#define WEB_PAGE_WARP_TMP_FILENAME                    "/tmp/warp_data.bin"

//warp data is from...
#define WARPING_TYPE_NA                     (0)
#define WARPING_TYPE_OSD                    (0xB0) //176   //Christie Advanced (Web warping) 或 Barco/Optoma 的 Basic/Advanced
#define WARPING_TYPE_AP                     (0xA0) //160   //other AP (blender / Visual Suite)
#define WARPING_TYPE_OSD_MYSTIQUE           (0xB1) //177   //Christie Basic
#define WARPING_TYPE_AP_MYSTIQUE            (0xA1) //161   //Christie Twist/Mystique

typedef enum
{
    BLENDING_AP_MEM_CUR_IDX = 0,
    BLENDING_AP_MEM1_IDX = 1,
    BLENDING_AP_MEM2_IDX = 2,
    BLENDING_AP_MEM3_IDX = 3,
    BLENDING_AP_MEM4_IDX = 4,
    BLENDING_AP_MEM5_IDX = 5,

    BLENDING_AP_MEM_INVALID,
}eBLENDING_AP_MEM_IDX;

typedef enum
{
    eWAT_ALL,
    eWAT_WARP_TABLE,
    eWAT_WARP_FILTER,

    eWAT_INVALID,
}eWARP_FUNC_AP_APPLY_TYPE;

typedef enum
{
    eBAT_ALL,

    eBAT_INVALID,
}eBLEND_FUNC_AP_APPLY_TYPE;

typedef enum
{
    eLAT_ALL,
    eLAT_BLACKLEVEL_PALETTE,
    eLAT_BLACKLEVEL_PALETTE0,
    eLAT_BLACKLEVEL_PALETTE1,
    eLAT_BLACKLEVEL_PALETTE2,
    eLAT_BLACKLEVEL_PALETTE3,
    eLAT_BLACKLEVEL_PALETTE4,
    eLAT_BLACKLEVEL_PALETTE5,
    eLAT_BLACKLEVEL_PALETTE6,
    eLAT_BLACKLEVEL_PALETTE7,
    eLAT_BLACKLEVEL_PALETTE8,
    eLAT_BLACKLEVEL_PALETTE9,
    eLAT_BLACKLEVEL_PALETTE10,
    eLAT_BLACKLEVEL_PALETTE11,
    eLAT_BLACKLEVEL_PALETTE12,
    eLAT_BLACKLEVEL_PALETTE13,
    eLAT_BLACKLEVEL_PALETTE14,
    eLAT_BLACKLEVEL_PALETTE15,
    eLAT_BLACKLEVEL_TABLE,

    eLAT_INVALID,
}eBLACKLEVEL_FUNC_AP_APPLY_TYPE;


typedef enum
{
    eWIL_BEFORE_WARP,
    eWIL_AFTER_WARP,

    eWIL_INVALID,
}eWARPOSD_INSERT_LOCATION;

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
    /* 12 */ PAT_TYPE__BLACKLEVEL_PREVIEW,    //blacklevel only grid line, no control point  //A65_OPTOMA_Doulas_0020

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
	/* 28 */ PAT_TYPE__DRAW_PNG,  //A65_OPTOMA_CL_0006
	/* 29 */ PAT_TYPE__DRAW_PNG_BEFORE_WARP,
	/* 30 */ PAT_TYPE__DRAW_PNG_AFTER_WARP,
	/* 31 */ PAT_TYPE__DRAW_SPRITE,  //A65_OPTOMA_CL_0006
	/* 32 */ PAT_TYPE__DRAW_SPRITES,  //A65_OPTOMA_CL_0006
    /* 33 */ PAT_TYPE__BLACKLEVEL_BOUNDARY,

	PAT_TYPE__INVALID

}ePAT_TYPE; //adv warp osd test pattern type






/************************************************************************************************/
/************************************************************************************************/
// 3D Lut
/************************************************************************************************/
/************************************************************************************************/

#define CONF_3DLUT_PATH         "/mnt/configs/scaler/_3DLut"
#define CONF_3DLUT_FILE0_PATH   "/mnt/configs/scaler/_3DLut/User3DLut0.lut"
#define CONF_3DLUT_FILE1_PATH   "/mnt/configs/scaler/_3DLut/User3DLut1.lut"
#define CONF_3DLUT_FILE2_PATH   "/mnt/configs/scaler/_3DLut/User3DLut2.lut"
#define CONF_3DLUT_FILE3_PATH   "/mnt/configs/scaler/_3DLut/User3DLut3.lut"
#define CONF_3DLUT_FILE_PATH    "/mnt/configs/scaler/_3DLut/User3DLut%d.lut"

/************************************************************************************************/
/************************************************************************************************/
// Test Pattern
/************************************************************************************************/
/************************************************************************************************/

typedef enum
{
    /* 00 */ eTID_OFF,

    /* 01 */ eTID_OSD_GRID,
    /* 02 */ eTID_OSD_WHITE,
    /* 03 */ eTID_OSD_BLACK,
    /* 04 */ eTID_OSD_CHECKERBOARD,
    /* 05 */ eTID_OSD_COLORBAR,
    /* 06 */ eTID_OSD_RED,
    /* 07 */ eTID_OSD_GREEN,
    /* 08 */ eTID_OSD_BLUE,
    /* 09 */ eTID_OSD_YELLOW,
    /* 10 */ eTID_OSD_MAGENTA,
    /* 11 */ eTID_OSD_CYAN,
    /* 12 */ eTID_OSD_FULL_SCREEN,
    /* 13 */ eTID_OSD_4K_FULL_SCREEN,     //if real 4K OSD, use char width 1 pixel
    /* 14 */ eTID_OSD_4K_FULL_SCREEN_2P,  //if real 4K OSD, use char width 2 pixel
    /* 15 */ eTID_OSD_GREEN_GRID,
    /* 16 */ eTID_OSD_MAGENTA_GRID,
    /* 17 */ eTID_OSD_WHITE_GRID,
    /* 18 */ eTID_OSD_WHITE_GRID_4CORNER,
    /* 19 */ eTID_OSD_BORESIGHT,
    /* 20 */ eTID_OSD_GRAY,
    /* 21 */ eTID_OSD_13POINTS,
    /* 22 */ eTID_OSD_H_RAMP,
    /* 23 */ eTID_OSD_V_RAMP,
    /* 24 */ eTID_OSD_RED_GRID,
    /* 25 */ eTID_OSD_BLUE_GRID,
    /* 26 */ eTID_OSD_CHECKERBOARD_2,
    /* 27 */ eTID_OSD_COLORBAR_2,
    /* 28 */ eTID_OSD_COLORBAR_3,
    /* 29 */ eTID_OSD_ASPECT_FRAME,

    /* 100 */ eTID_HSG_RED = 100,
    /* 101 */ eTID_HSG_GREEN,
    /* 102 */ eTID_HSG_BLUE,
    /* 103 */ eTID_HSG_CYAN,
    /* 104 */ eTID_HSG_MAGENTA,
    /* 105 */ eTID_HSG_YELLOW,
    /* 106 */ eTID_HSG_WHITE,

    /* 120 */ eTID_DDP_WHITE = 120,
    /* 121 */ eTID_DDP_BLACK,
    /* 122 */ eTID_DDP_RED,
    /* 123 */ eTID_DDP_GREEN,
    /* 124 */ eTID_DDP_BLUE,
    /* 125 */ eTID_DDP_CYAN,
    /* 126 */ eTID_DDP_YELLOW,
    /* 127 */ eTID_DDP_MAGENTA,
    /* 128 */ eTID_DDP_BLUE_60,
    /* 129 */ eTID_DDP_GRAY_10,
    /* 130 */ eTID_DDP_WRGB_64,
    /* 131 */ eTID_DDP_FLARE,
    /* 132 */ eTID_DDP_FULL_SCREEN_W,
    /* 133 */ eTID_DDP_FULL_SCREEN_B,
    /* 134 */ eTID_DDP_LATERAL_COLOR,
    /* 135 */ eTID_DDP_FOCUS_WORD,
    /* 136 */ eTID_DDP_13POINTS,
    /* 137 */ eTID_DDP_FULL_COLOR,
    /* 138 */ eTID_DDP_BORESIGHT,
    /* 139 */ eTID_DDP_ACTUATOR_CAL,

    eTID_INVALID,

}eTEST_PATTERN_ID;

typedef enum
{
    /* 00 */ eTDF_SCALER_OSD,
    /* 01 */ eTDF_FRONTEND,
    /* 02 */ eTDF_DDP,
    /* 03 */ eTDF_WARPING,

    eTDF_INVALID,

}eTEST_PATTERN_DRAW_FROM;

typedef enum
{
    /* 00 */ eOSD_UPDATE,
    /* 01 */ eOSD_UPDATE_CLI,
    /* 02 */ eOSD_UPDATE_NO_MENU,
    /* 03 */ eOSD_UPDATE_NO_SPLASH,
    /* 04 */ eOSD_UPDATE_POLL_EVENT,

    eOSD_UPDATE_INVALID,

}eOSD_UPDATE_STATE;

typedef enum
{
    eLENS_ID_AUTO_DETECT    = -2,
    eLENS_ID_NO_LENS        = -1,
    eLENS_ID_NVRAM_ID       = 0,    //0
    eLENS_ID_AUST,                  //1
    eLENS_ID_A20,                   //2
    eLENS_ID_A21,                   //3
    eLENS_ID_A22,                   //4
    eLENS_ID_A23,                   //5
    eLENS_ID_A25,                   //6
    eLENS_ID_A26,                   //7
    eLENS_ID_A11,                   //8
    eLENS_ID_A27,                   //9
    eLENS_ID_A18,                   //10
    eLENS_ID_A19,                   //11
    eLENS_ID_EN68,                  //12
    eLENS_ID_T20C1,                 //13
    eLENS_ID_T20C2,                 //14
    eLENS_ID_B20U100,               //15

    eLENS_ID_NUMBERS,               // used for enumerated type range checking (DO NOT REMOVE)

	eLENS_ID_RESERVED_17,
	eLENS_ID_RESERVED_18,
	eLENS_ID_RESERVED_19,

	eLENS_ID_NVRAM_B20U100,
	eLENS_ID_NVRAM_T20C1,
	eLENS_ID_NVRAM_T20C2,
	eLENS_ID_NVRAM_B10R500,
	eLENS_ID_NVRAM_B10S100,
    eLENS_ID_NVRAM_ID_B10R501,               //25, B10R501 = B10R500 + HW encoder.
    eLENS_ID_NVRAM_ID_SD138A,               //26

	eLENS_ID_NVRAM_NUMBERS,
} eLENS_ID;

#define cUNKNOWN			"Unknown\0"
#define cUNDEFINED			"Undefined\0"
#define cNOT_SUPPORTED		"Not supported\0"

/************************************************************************************************/
/************************************************************************************************/
//
/************************************************************************************************/
/************************************************************************************************/


#pragma pack(pop)                          // restore previous alignment

#endif   //_COMMONTYPE_H_

