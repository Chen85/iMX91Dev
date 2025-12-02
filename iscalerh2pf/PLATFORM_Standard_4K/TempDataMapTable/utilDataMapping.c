/*
1. GUI , Config File 使用 GUI enum (在 CustomDef_BarcoG100.h)
2. CLI 使用 CLI define (在 CustomDefCLI_BarcoG100.h)
3. 其餘皆使用 CM value (在 CommonDef.h)

4. 互轉請參考 DataMapping.h 上方 define
例如 Common Value 轉 GUI Value
則使用 CM2GUI

5. 資料確認請用 utilDataMapping_CMValueRangeCheck
和 utilDataMapping_GuiValueRangeCheck

6. *** 特例 , 由於對接 DDP 且不浪費大型陣列空間
DDP table 使用 Gui enum
例如 sWallColorValues , sCE_TABLE_SETTING_WU 相關 DDP table

------------

增加 Common Data Code Step:
1. 在 CommonDef.h , CustomDef_BarcoG100.h , CustomDefCLI_BarcoG100.h 加入 enum
2. 在 utilDataMapping.c 增加 mapping table
3. 修改 appDataMgr.c 的 m_sDataMgr_Access_LutTable 最大最小預設值
4. 修改此 data code 對應的 function 內容 (ex: palDataMgr_Access_Gamma)
5. 搜尋並確認此 data code 有被用到的地方 , 需照上述說明修改
6. 搜尋 utilDataMgr.c 裡 , 此 eep ram(common value)的數值和 config(Gui Value) 需互相轉換
7. 搜尋 utilDataMgr.c 裡的 default value 是否需要修改
*/


#include "utilDbgMsg.h"
#include "utilDataMapping.h"
#include "CommonAPI.h"

#if defined(CUSTOM_CHRISTIE)
/////////////////////////////////////////////////////////////////////////////////////
// Source ID
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_SourceID[] =
{
    /* 00 */ {(INT32)eGUI_SOURCE_ID_HDMI1,      (INT32)eCM_SOURCE_HDMI1,        (INT32)CLI_SOURCE_HDMI1},
    /* 01 */ {(INT32)eGUI_SOURCE_ID_HDMI2,      (INT32)eCM_SOURCE_HDMI2,        (INT32)CLI_SOURCE_HDMI2},
    /* 02 */ {(INT32)eGUI_SOURCE_ID_DVI,        (INT32)eCM_SOURCE_DVI,          (INT32)CLI_SOURCE_DVI},		//A70Gen2_Doulas_0004 Modify
    /* 02 */ {(INT32)eGUI_SOURCE_ID_DP,         (INT32)eCM_SOURCE_DISPLAYPORT,  (INT32)CLI_SOURCE_DP},		//A70Gen2_Doulas_0004 Modify
    /* 03 */ {(INT32)eGUI_SOURCE_ID_3GSDI,      (INT32)eCM_SOURCE_3GSDI,        (INT32)CLI_SOURCE_3GSDI},
    /* 04 */ {(INT32)eGUI_SOURCE_ID_HDBASET,    (INT32)eCM_SOURCE_HDBASET,      (INT32)CLI_SOURCE_HDBASET},
};

/////////////////////////////////////////////////////////////////////////////////////
// Test Pattern
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_TestPattern[] =
{
    /* 00 */ {(INT32)eGUI_TEST_PATTERN_OFF,                 (INT32)eCM_TEST_PATTERN_OFF,            (INT32)CLI_TEST_PATTERN_OFF},
    /* 01 */ {(INT32)eGUI_TEST_PATTERN_GRID,                (INT32)eCM_TEST_PATTERN_GRID,           (INT32)CLI_TEST_PATTERN_GRID},
    /* 02 */ {(INT32)eGUI_TEST_PATTERN_WHITE,               (INT32)eCM_TEST_PATTERN_WHITE,          (INT32)CLI_TEST_PATTERN_WHITE},
    /* 03 */ {(INT32)eGUI_TEST_PATTERN_BLACK,               (INT32)eCM_TEST_PATTERN_BLACK,          (INT32)CLI_TEST_PATTERN_BLACK},
    /* 04 */ {(INT32)eGUI_TEST_PATTERN_CHECKERBOARD,        (INT32)eCM_TEST_PATTERN_CHECKERBOARD,   (INT32)CLI_TEST_PATTERN_CHECKERBOARD},
    /* 05 */ {(INT32)eGUI_TEST_PATTERN_COLORBAR,            (INT32)eCM_TEST_PATTERN_COLORBAR,       (INT32)CLI_TEST_PATTERN_COLORBAR},
};

/////////////////////////////////////////////////////////////////////////////////////
// Service Test Pattern
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_ServiceTestPattern[] =
{
    /* 00 */ {(INT32)eGUI_SERVICE_TEST_PATTERN_OFF,           	(INT32)eCM_TEST_PATTERN_OFF,            	(INT32)CLI_SERVICE_TEST_PATTERN_OFF},
    /* 01 */ {(INT32)eGUI_SERVICE_TEST_PATTERN_GRID,          	(INT32)eCM_TEST_PATTERN_GRID,     			(INT32)CLI_SERVICE_TEST_PATTERN_GRID},
    /* 02 */ {(INT32)eGUI_SERVICE_TEST_PATTERN_WHITE,        	(INT32)eCM_TEST_PATTERN_WHITE, 	 	 		(INT32)CLI_SERVICE_TEST_PATTERN_WHITE},
    /* 03 */ {(INT32)eGUI_SERVICE_TEST_PATTERN_BLACK,          	(INT32)eCM_TEST_PATTERN_BLACK,     			(INT32)CLI_SERVICE_TEST_PATTERN_BLACK},
    /* 04 */ {(INT32)eGUI_SERVICE_TEST_PATTERN_CHECKBOARD, 		(INT32)eCM_TEST_PATTERN_CHECKERBOARD,	  	(INT32)CLI_SERVICE_TEST_PATTERN_CHECKBOARD},
    /* 05 */ {(INT32)eGUI_SERVICE_TEST_PATTERN_COLORBAR,     	(INT32)eCM_TEST_PATTERN_COLORBAR,         	(INT32)CLI_SERVICE_TEST_PATTERN_COLORBAR},
    /* 06 */ {(INT32)eGUI_SERVICE_TEST_PATTERN_RED,             (INT32)eCM_TEST_PATTERN_RED,            	(INT32)CLI_SERVICE_TEST_PATTERN_RED},
    /* 07 */ {(INT32)eGUI_SERVICE_TEST_PATTERN_GREEN,           (INT32)eCM_TEST_PATTERN_GREEN,          	(INT32)CLI_SERVICE_TEST_PATTERN_GREEN},
    /* 08 */ {(INT32)eGUI_SERVICE_TEST_PATTERN_BLUE,            (INT32)eCM_TEST_PATTERN_BLUE,           	(INT32)CLI_SERVICE_TEST_PATTERN_BLUE},
    /* 09 */ {(INT32)eGUI_SERVICE_TEST_PATTERN_YELLOW,      	(INT32)eCM_TEST_PATTERN_YELLOW,         	(INT32)CLI_SERVICE_TEST_PATTERN_YELLOW},
    /* 10 */ {(INT32)eGUI_SERVICE_TEST_PATTERN_MAGENTA,   		(INT32)eCM_TEST_PATTERN_MAGENTA,        	(INT32)CLI_SERVICE_TEST_PATTERN_MAGENTA},
    /* 11 */ {(INT32)eGUI_SERVICE_TEST_PATTERN_CYAN,            (INT32)eCM_TEST_PATTERN_CYAN,           	(INT32)CLI_SERVICE_TEST_PATTERN_CYAN},
    /* 12 */ {(INT32)eGUI_SERVICE_TEST_PATTERN_BORESIGHT, 	 	(INT32)eCM_TEST_PATTERN_BORESIGHT,	 	  	(INT32)CLI_SERVICE_TEST_PATTERN_BORESIGHT},
    /* 13 */ {(INT32)eGUI_SERVICE_TEST_PATTERN_FULL_SCREEN, 	(INT32)eCM_TEST_PATTERN_FULL_SCREEN,       	(INT32)CLI_SERVICE_TEST_PATTERN_FULL_SCREEN},
    /* 14 */ {(INT32)eGUI_SERVICE_TEST_PATTERN_13POINTS,   		(INT32)eCM_TEST_PATTERN_13POINTS, 	   		(INT32)CLI_SERVICE_TEST_PATTERN_13POINTS},
};

/////////////////////////////////////////////////////////////////////////////////////
// Twist Test Pattern
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_TwistTestPattern[] =
{
    /* 00 */ {(INT32)eGUI_TWIST_TEST_PATTERN_OFF,           (INT32)eCM_TEST_PATTERN_OFF,            (INT32)CLI_TWIST_TEST_PATTERN_OFF},
    /* 01 */ {(INT32)eGUI_TWIST_TEST_PATTERN_GRID,          (INT32)eCM_TEST_PATTERN_GRID,           (INT32)CLI_TWIST_TEST_PATTERN_GRID},
    /* 02 */ {(INT32)eGUI_TWIST_TEST_PATTERN_WHITE,         (INT32)eCM_TEST_PATTERN_WHITE,          (INT32)CLI_TWIST_TEST_PATTERN_WHITE},
    /* 03 */ {(INT32)eGUI_TWIST_TEST_PATTERN_BLACK,         (INT32)eCM_TEST_PATTERN_BLACK,          (INT32)CLI_TWIST_TEST_PATTERN_BLACK},
    /* 04 */ {(INT32)eGUI_TWIST_TEST_PATTERN_CHECKERBOARD,  (INT32)eCM_TEST_PATTERN_CHECKERBOARD,   (INT32)CLI_TWIST_TEST_PATTERN_CHECKERBOARD},
    /* 05 */ {(INT32)eGUI_TWIST_TEST_PATTERN_COLORBAR,      (INT32)eCM_TEST_PATTERN_COLORBAR,       (INT32)CLI_TWIST_TEST_PATTERN_COLORBAR},
    /* 06 */ {(INT32)eGUI_TWIST_TEST_PATTERN_GRAY,          (INT32)eCM_TEST_PATTERN_GRAY,           (INT32)CLI_TWIST_TEST_PATTERN_GRAY},
};

/////////////////////////////////////////////////////////////////////////////////////
// Picture Settings
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_PictureSettings[] =
{
	/* 00 */ {(INT32)eGUI_PICTURE_SETTINGS_VIDEO,    		(INT32)eCM_PICTURE_SETTINGS_VIDEO,   		(INT32)CLI_PICTURE_SETTINGS_VIDEO},
	/* 01 */ {(INT32)eGUI_PICTURE_SETTINGS_BRIGHT,		    (INT32)eCM_PICTURE_SETTINGS_BRIGHT,		    (INT32)CLI_PICTURE_SETTINGS_BRIGHT},
	/* 02 */ {(INT32)eGUI_PICTURE_SETTINGS_ENHANCED,        (INT32)eCM_PICTURE_SETTINGS_ENHANCED,	    (INT32)CLI_PICTURE_SETTINGS_ENHANCED},
	/* 03 */ {(INT32)eGUI_PICTURE_SETTINGS_REC709,		    (INT32)eCM_PICTURE_SETTINGS_REC709,		    (INT32)CLI_PICTURE_SETTINGS_REC709},
	/* 04 */ {(INT32)eGUI_PICTURE_SETTINGS_DICOMSIM,        (INT32)eCM_PICTURE_SETTINGS_DICOMSIM,	    (INT32)CLI_PICTURE_SETTINGS_DICOMSIM},
	/* 05 */ {(INT32)eGUI_PICTURE_SETTINGS_BLENDING,        (INT32)eCM_PICTURE_SETTINGS_BLENDING,	    (INT32)CLI_PICTURE_SETTINGS_BLENDING},
	/* 06 */ {(INT32)eGUI_PICTURE_SETTINGS_HDR,	    		(INT32)eCM_PICTURE_SETTINGS_HDR,	    	(INT32)CLI_PICTURE_SETTINGS_HDR},
	/* 07 */ {(INT32)eGUI_PICTURE_SETTINGS_3D,	    		(INT32)eCM_PICTURE_SETTINGS_3D,	    		(INT32)CLI_PICTURE_SETTINGS_3D},
	/* 08 */ {(INT32)eGUI_PICTURE_SETTINGS_2D_HIGH_SPEED,	(INT32)eCM_PICTURE_SETTINGS_2DHIGHSPEED,	(INT32)CLI_PICTURE_SETTINGS_2D_HIGH_SPEED},
	/* 09 */ {(INT32)eGUI_PICTURE_SETTINGS_USER,	        (INT32)eCM_PICTURE_SETTINGS_USER,		    (INT32)CLI_PICTURE_SETTINGS_USER},
};

/////////////////////////////////////////////////////////////////////////////////////
// Apply User Settings
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_ApplyUserSettings[] =				//G100_Doulas_0066
{
	/* 00 */ {(INT32)eGUI_PRE_USER_VIDEO,		    (INT32)eCM_PICTURE_SETTINGS_VIDEO,		    (INT32)CLI_PRE_USER_VIDEO},
	/* 01 */ {(INT32)eGUI_PRE_USER_BRIGHT,		    (INT32)eCM_PICTURE_SETTINGS_BRIGHT,		    (INT32)CLI_PRE_USER_BRIGHT},
	/* 02 */ {(INT32)eGUI_PRE_USER_ENHANCED,        (INT32)eCM_PICTURE_SETTINGS_ENHANCED,	    (INT32)CLI_PRE_USER_ENHANCED},
	/* 03 */ {(INT32)eGUI_PRE_USER_REC709,		    (INT32)eCM_PICTURE_SETTINGS_REC709,		    (INT32)CLI_PRE_USER_REC709},
	/* 04 */ {(INT32)eGUI_PRE_USER_DICOMSIM,	    (INT32)eCM_PICTURE_SETTINGS_DICOMSIM,	    (INT32)CLI_PRE_USER_DICOMSIM},
	/* 05 */ {(INT32)eGUI_PRE_USER_BLENDING,	    (INT32)eCM_PICTURE_SETTINGS_BLENDING,	    (INT32)CLI_PRE_USER_BLENDING},
	/* 06 */ {(INT32)eGUI_PRE_USER_HDR,			    (INT32)eCM_PICTURE_SETTINGS_HDR,		    (INT32)CLI_PRE_USER_HDR},
	/* 07 */ {(INT32)eGUI_PRE_USER_3D,			    (INT32)eCM_PICTURE_SETTINGS_3D,			    (INT32)CLI_PRE_USER_3D},
	/* 08 */ {(INT32)eGUI_PRE_USER_2D_HIGH_SPEED,   (INT32)eCM_PICTURE_SETTINGS_2DHIGHSPEED,    (INT32)CLI_PRE_USER_2D_HIGH_SPEED},
};


/////////////////////////////////////////////////////////////////////////////////////
// Language
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_Language[] =
{
    /* 00 */ {(INT32)eGUI_LANGUAGE_ENGLISH,     (INT32)eCM_LANGUAGE_ENGLISH,    (INT32)CLI_LANGUAGE_ENGLISH},
    /* 01 */ {(INT32)eGUI_LANGUAGE_FRENCH,      (INT32)eCM_LANGUAGE_FRENCH,     (INT32)CLI_LANGUAGE_FRENCH},
    /* 02 */ {(INT32)eGUI_LANGUAGE_SPANISH,     (INT32)eCM_LANGUAGE_SPANISH,    (INT32)CLI_LANGUAGE_SPANISH},
    /* 03 */ {(INT32)eGUI_LANGUAGE_GERMAN,      (INT32)eCM_LANGUAGE_GERMAN,     (INT32)CLI_LANGUAGE_GERMAN},
    /* 04 */ {(INT32)eGUI_LANGUAGE_ITALIAN,     (INT32)eCM_LANGUAGE_ITALIAN,    (INT32)CLI_LANGUAGE_ITALIAN},
    /* 05 */ {(INT32)eGUI_LANGUAGE_POLSKI,      (INT32)eCM_LANGUAGE_POLSKI,     (INT32)CLI_LANGUAGE_POLSKI},
    /* 06 */ {(INT32)eGUI_LANGUAGE_RUSSIAN,     (INT32)eCM_LANGUAGE_RUSSIAN,    (INT32)CLI_LANGUAGE_RUSSIAN},
    /* 07 */ {(INT32)eGUI_LANGUAGE_S_CHINESE,   (INT32)eCM_LANGUAGE_S_CHINESE,  (INT32)CLI_LANGUAGE_S_CHINESE},
    /* 08 */ {(INT32)eGUI_LANGUAGE_T_CHINESE,   (INT32)eCM_LANGUAGE_T_CHINESE,  (INT32)CLI_LANGUAGE_T_CHINESE},
    /* 09 */ {(INT32)eGUI_LANGUAGE_JAPANESE,    (INT32)eCM_LANGUAGE_JAPANESE,   (INT32)CLI_LANGUAGE_JAPANESE},
    /* 10 */ {(INT32)eGUI_LANGUAGE_KOREAN,      (INT32)eCM_LANGUAGE_KOREAN,     (INT32)CLI_LANGUAGE_KOREAN},
};

/////////////////////////////////////////////////////////////////////////////////////
// Size Presets
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_ScalingMode[] =
{
    /* 00 */ {(INT32)eGUI_SCALING_MODE_AUTO,        (INT32)eCM_SCALING_MODE_AUTO,       (INT32)CLI_SCALING_MODE_AUTO},
    /* 01 */ {(INT32)eGUI_SCALING_MODE_NATIVE,      (INT32)eCM_SCALING_MODE_NATIVE,     (INT32)CLI_SCALING_MODE_NATIVE},
    /* 02 */ {(INT32)eGUI_SCALING_MODE_4_3,         (INT32)eCM_SCALING_MODE_4_3,        (INT32)CLI_SCALING_MODE_4_3},
    /* 03 */ {(INT32)eGUI_SCALING_MODE_LETTER_BOX,  (INT32)eCM_SCALING_MODE_LETTER_BOX, (INT32)CLI_SCALING_MODE_LETTER_BOX},
    /* 04 */ {(INT32)eGUI_SCALING_MODE_FULL_SIZE,   (INT32)eCM_SCALING_MODE_FULL_SIZE,  (INT32)CLI_SCALING_MODE_FULL_SIZE},
    /* 05 */ {(INT32)eGUI_SCALING_MODE_WIDTH,       (INT32)eCM_SCALING_MODE_FULL_WIDTH, (INT32)CLI_SCALING_MODE_FULL_WIDTH},
    /* 06 */ {(INT32)eGUI_SCALING_MODE_HEIGHT,      (INT32)eCM_SCALING_MODE_FULL_HEIGHT,(INT32)CLI_SCALING_MODE_FULL_HEIGHT},
    /* 07 */ {(INT32)eGUI_SCALING_MODE_CUSTOM,      (INT32)eCM_SCALING_MODE_CUSTOM,     (INT32)CLI_SCALING_MODE_CUSTOM},
    /* 08 */ {(INT32)eGUI_SCALING_MODE_3D,          (INT32)eCM_SCALING_MODE_3D,         (INT32)CLI_SCALING_MODE_3D},
    /* 09 */ {(INT32)eGUI_SCALING_MODE_21_9,        (INT32)eCM_SCALING_MODE_21_9,       (INT32)CLI_SCALING_MODE_21_9}, //HICC2_Doulas_0065
};

/////////////////////////////////////////////////////////////////////////////////////
// Hotkey Settings
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_HotkeySettings[] =
{
    /* 00 */ {(INT32)eGUI_HOTKEY_SETTINGS_SIZE_PRESETS,         (INT32)eCM_HOTKEY_SETTINGS_SIZE_PRESETS,            (INT32)CLI_HOTKEY_SETTINGS_SIZE_PRESETS},
    /* 01 */ {(INT32)eGUI_HOTKEY_SETTINGS_PICTURE_SETTINGS,     (INT32)eCM_HOTKEY_SETTINGS_PICTURE_SETTINGS,        (INT32)CLI_HOTKEY_SETTINGS_PICTURE_SETTINGS},
    /* 02 */ {(INT32)eGUI_HOTKEY_SETTINGS_DEFAULT_VALUE,        (INT32)eCM_HOTKEY_SETTINGS_DEFAULT_VALUE,           (INT32)CLI_HOTKEY_SETTINGS_DEFAULT_VALUE},
    /* 03 */ {(INT32)eGUI_HOTKEY_SETTINGS_DETAIL,               (INT32)eCM_HOTKEY_SETTINGS_DETAIL,                  (INT32)CLI_HOTKEY_SETTINGS_DETAIL},
    /* 04 */ {(INT32)eGUI_HOTKEY_SETTINGS_CONTRAST_ENHANCEMENT, (INT32)eCM_HOTKEY_SETTINGS_CONTRAST_ENHANCEMENT,    (INT32)CLI_HOTKEY_SETTINGS_CONTRAST_ENHANCEMENT},
    /* 05 */ {(INT32)eGUI_HOTKEY_SETTINGS_LIGHT_SOURCE_MODE,    (INT32)eCM_HOTKEY_SETTINGS_LIGHT_SOURCE_MODE,       (INT32)CLI_HOTKEY_SETTINGS_LIGHT_SOURCE_MODE},
    /* 06 */ {(INT32)eGUI_HOTKEY_SETTINGS_FREEZE_SCREEN,        (INT32)eCM_HOTKEY_SETTINGS_FREEZE_SCREEN,           (INT32)CLI_HOTKEY_SETTINGS_FREEZE_SCREEN},
    /* 07 */ {(INT32)eGUI_HOTKEY_SETTINGS_INFORMATION,          (INT32)eCM_HOTKEY_SETTINGS_INFORMATION,             (INT32)CLI_HOTKEY_SETTINGS_INFORMATION},
    /* 08 */ {(INT32)eGUI_HOTKEY_SETTINGS_WARPBLEND_DISABLE,    (INT32)eCM_HOTKEY_SETTINGS_WARPBLEND_DISABLE,       (INT32)CLI_HOTKEY_SETTINGS_WARPBLEND_DISABLE},
};

/////////////////////////////////////////////////////////////////////////////////////
// Screen Mode (PIP/PBP)
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_ScreenMode[] =
{
    /* 00 */ {(INT32)eGUI_SCREEN_MODE_OFF, (INT32)eCM_SCREEN_MODE_OFF, (INT32)CLI_SCREEN_MODE_OFF},
    /* 01 */ {(INT32)eGUI_SCREEN_MODE_PIP, (INT32)eCM_SCREEN_MODE_PIP, (INT32)CLI_SCREEN_MODE_PIP},
    /* 02 */ {(INT32)eGUI_SCREEN_MODE_PBP, (INT32)eCM_SCREEN_MODE_PBP, (INT32)CLI_SCREEN_MODE_PBP},
};

/////////////////////////////////////////////////////////////////////////////////////
// Menu Location
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_MenuLocation[] =
{
    /* 00 */ {(INT32)eGUI_MENU_LOCATION_TOP_LEFT,       (INT32)eCM_MENU_LOCATION_TOP_LEFT,      (INT32)CLI_MENU_LOCATION_TOP_LEFT},
    /* 01 */ {(INT32)eGUI_MENU_LOCATION_TOP_RIGHT,      (INT32)eCM_MENU_LOCATION_TOP_RIGHT,     (INT32)CLI_MENU_LOCATION_TOP_RIGHT},
    /* 02 */ {(INT32)eGUI_MENU_LOCATION_CENTER,         (INT32)eCM_MENU_LOCATION_CENTER,        (INT32)CLI_MENU_LOCATION_CENTER},
    /* 03 */ {(INT32)eGUI_MENU_LOCATION_BOTTOM_LEFT,    (INT32)eCM_MENU_LOCATION_BOTTOM_LEFT,   (INT32)CLI_MENU_LOCATION_BOTTOM_LEFT},
    /* 04 */ {(INT32)eGUI_MENU_LOCATION_BOTTOM_RIGHT,   (INT32)eCM_MENU_LOCATION_BOTTOM_RIGHT,  (INT32)CLI_MENU_LOCATION_BOTTOM_RIGHT},
};

/////////////////////////////////////////////////////////////////////////////////////
// OverScan
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_OverScan[] =
{
    /* 00 */ {(INT32)eGUI_OVERSCAN_OFF,  (INT32)eCM_OVERSCAN_OFF,  (INT32)CLI_OVERSCAN_OFF},
    /* 01 */ {(INT32)eGUI_OVERSCAN_ZOOM, (INT32)eCM_OVERSCAN_ZOOM, (INT32)CLI_OVERSCAN_ZOOM},
    /* 02 */ {(INT32)eGUI_OVERSCAN_CROP, (INT32)eCM_OVERSCAN_CROP, (INT32)CLI_OVERSCAN_CROP},
};


/////////////////////////////////////////////////////////////////////////////////////
// Ceiling Mount
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_CeilingMount[] =
{
    /* 00 */  {(INT32)eGUI_CEILING_MOUNT_OFF,    (INT32)eCM_CEILING_MOUNT_OFF,   (INT32)CLI_CEILING_MOUNT_OFF},
    /* 01 */  {(INT32)eGUI_CEILING_MOUNT_ON,     (INT32)eCM_CEILING_MOUNT_ON,    (INT32)CLI_CEILING_MOUNT_ON},
    /* 02 */  {(INT32)eGUI_CEILING_MOUNT_AUTO,   (INT32)eCM_CEILING_MOUNT_AUTO,  (INT32)CLI_CEILING_MOUNT_AUTO},
};

/////////////////////////////////////////////////////////////////////////////////////
// Gamma
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_Gamma[] =   //G100_Simon_0022
{
    /* 00 */ {(INT32)eGUI_GAMMA_VIDEO,          (INT32)eCM_GAMMA_VIDEO,         (INT32)CLI_GAMMA_VIDEO},
    /* 01 */ {(INT32)eGUI_GAMMA_FILM,           (INT32)eCM_GAMMA_FILM,          (INT32)CLI_GAMMA_FILM},
    /* 02 */ {(INT32)eGUI_GAMMA_BRIGHT,         (INT32)eCM_GAMMA_2_2,           (INT32)CLI_GAMMA_BRIGHT},
    /* 03 */ {(INT32)eGUI_GAMMA_CRT,            (INT32)eCM_GAMMA_CRT,           (INT32)CLI_GAMMA_CRT},
    /* 04 */ {(INT32)eGUI_GAMMA_DICOM,          (INT32)eCM_GAMMA_DICOM,         (INT32)CLI_GAMMA_DICOM},
    /* 05 */ {(INT32)eGUI_GAMMA_HDR,            (INT32)eCM_GAMMA_HDR_STANDARD,  (INT32)CLI_GAMMA_HDR}, //A35G2_CDS_Coda_0015

};

/////////////////////////////////////////////////////////////////////////////////////
// Lens Memory Save
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_LensMemorySave[] =
{
    /* 00 */ {(INT32)eGUI_LENS_MEMORY_SAVE_1, (INT32)eCM_LENS_MEMORY_SAVE_1, (INT32)CLI_LENS_MEMORY_SAVE_1},
    /* 01 */ {(INT32)eGUI_LENS_MEMORY_SAVE_2, (INT32)eCM_LENS_MEMORY_SAVE_2, (INT32)CLI_LENS_MEMORY_SAVE_2},
    /* 02 */ {(INT32)eGUI_LENS_MEMORY_SAVE_3, (INT32)eCM_LENS_MEMORY_SAVE_3, (INT32)CLI_LENS_MEMORY_SAVE_3},
    /* 03 */ {(INT32)eGUI_LENS_MEMORY_SAVE_4, (INT32)eCM_LENS_MEMORY_SAVE_4, (INT32)CLI_LENS_MEMORY_SAVE_4},
    /* 04 */ {(INT32)eGUI_LENS_MEMORY_SAVE_5, (INT32)eCM_LENS_MEMORY_SAVE_5, (INT32)CLI_LENS_MEMORY_SAVE_5},
};

/////////////////////////////////////////////////////////////////////////////////////
// Lens Memory Apply
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_LensMemoryApply[] =
{
    /* 00 */ {(INT32)eGUI_LENS_MEMORY_APPLY_1, (INT32)eCM_LENS_MEMORY_APPLY_1, (INT32)CLI_LENS_MEMORY_APPLY_1},
    /* 01 */ {(INT32)eGUI_LENS_MEMORY_APPLY_2, (INT32)eCM_LENS_MEMORY_APPLY_2, (INT32)CLI_LENS_MEMORY_APPLY_2},
    /* 02 */ {(INT32)eGUI_LENS_MEMORY_APPLY_3, (INT32)eCM_LENS_MEMORY_APPLY_3, (INT32)CLI_LENS_MEMORY_APPLY_3},
    /* 03 */ {(INT32)eGUI_LENS_MEMORY_APPLY_4, (INT32)eCM_LENS_MEMORY_APPLY_4, (INT32)CLI_LENS_MEMORY_APPLY_4},
    /* 04 */ {(INT32)eGUI_LENS_MEMORY_APPLY_5, (INT32)eCM_LENS_MEMORY_APPLY_5, (INT32)CLI_LENS_MEMORY_APPLY_5},
};

/////////////////////////////////////////////////////////////////////////////////////
// Orientation Direction
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_OrientationDirection[] =
{
    /* 00 */ {(INT32)eGUI_REAR_PROJECTION_FRONT, (INT32)eCM_REAR_PROJECTION_FRONT, (INT32)CLI_REAR_PROJECTION_FRONT},
    /* 01 */ {(INT32)eGUI_REAR_PROJECTION_REAR,  (INT32)eCM_REAR_PROJECTION_REAR,  (INT32)CLI_REAR_PROJECTION_REAR},
};

/////////////////////////////////////////////////////////////////////////////////////
// EDID Type
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_EDID_Type[] =
{
    /* 00 */ {(INT32)eGUI_EDID_TYPE_V14, (INT32)eCM_EDID_TYPE_V14, (INT32)CLI_EDID_TYPE_V14},
    /* 01 */ {(INT32)eGUI_EDID_TYPE_V20, (INT32)eCM_EDID_TYPE_V20, (INT32)CLI_EDID_TYPE_V20},
#if (CUSTOMIZED_EDID_ENABLE == TRUE) //HICC2_Doulas_0068
    /* 02 */ {(INT32)eGUI_EDID_TYPE_CUSTOMIZED, (INT32)eCM_EDID_TYPE_CUSTOMIZED, (INT32)CLI_EDID_TYPE_CUSTOMIZED},
#endif
};

static sMAPPING_TABLE_FORMAT asMPT_EDID_Type2[] =
{
    /* 00 */ {(INT32)eGUI_EDID_TYPE_V14, (INT32)eCM_EDID_TYPE_V14, (INT32)CLI_EDID_TYPE_V14},
    /* 01 */ {(INT32)eGUI_EDID_TYPE_V20, (INT32)eCM_EDID_TYPE_V20, (INT32)CLI_EDID_TYPE_V20},
};


/////////////////////////////////////////////////////////////////////////////////////
// Blending Gamma
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_BlendingGamma[] =
{
    /* 00 */ {(INT32)eGUI_BLENDING_GAMMA_1_8, (INT32)eCM_BLENDING_GAMMA_1_8, (INT32)CLI_BLENDING_GAMMA_1_8},
    /* 01 */ {(INT32)eGUI_BLENDING_GAMMA_1_9, (INT32)eCM_BLENDING_GAMMA_1_9, (INT32)CLI_BLENDING_GAMMA_1_9},
    /* 02 */ {(INT32)eGUI_BLENDING_GAMMA_2_0, (INT32)eCM_BLENDING_GAMMA_2_0, (INT32)CLI_BLENDING_GAMMA_2_0},
    /* 03 */ {(INT32)eGUI_BLENDING_GAMMA_2_1, (INT32)eCM_BLENDING_GAMMA_2_1, (INT32)CLI_BLENDING_GAMMA_2_1},
    /* 04 */ {(INT32)eGUI_BLENDING_GAMMA_2_2, (INT32)eCM_BLENDING_GAMMA_2_2, (INT32)CLI_BLENDING_GAMMA_2_2},
    /* 05 */ {(INT32)eGUI_BLENDING_GAMMA_2_3, (INT32)eCM_BLENDING_GAMMA_2_3, (INT32)CLI_BLENDING_GAMMA_2_3},
    /* 06 */ {(INT32)eGUI_BLENDING_GAMMA_2_4, (INT32)eCM_BLENDING_GAMMA_2_4, (INT32)CLI_BLENDING_GAMMA_2_4},
};

/////////////////////////////////////////////////////////////////////////////////////
// Color Temperature
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_ColorTemperature[] =
{
    /* 00 */ {(INT32)eGUI_COLOR_TEMPERATURE_WARMEST, (INT32)eCM_COLOR_TEMPERATURE_WARMEST,  (INT32)CLI_COLOR_TEMPERATURE_WARMEST},
    /* 01 */ {(INT32)eGUI_COLOR_TEMPERATURE_WARM,    (INT32)eCM_COLOR_TEMPERATURE_WARM,     (INT32)CLI_COLOR_TEMPERATURE_WARM},
    /* 02 */ {(INT32)eGUI_COLOR_TEMPERATURE_COOL,    (INT32)eCM_COLOR_TEMPERATURE_COOL,     (INT32)CLI_COLOR_TEMPERATURE_COOL},
};

/////////////////////////////////////////////////////////////////////////////////////
// Color Space
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_ColorSpace[] =
{
    /* 00 */ {(INT32)eGUI_COLOR_SPACE_AUTO,         (INT32)eCM_COLOR_SPACE_AUTO,        (INT32)CLI_COLOR_SPACE_AUTO},
    /* 01 */ {(INT32)eGUI_COLOR_SPACE_RGB_FULL,     (INT32)eCM_COLOR_SPACE_RGB_FULL,    (INT32)CLI_COLOR_SPACE_RGB_FULL},
    /* 02 */ {(INT32)eGUI_COLOR_SPACE_RGB_LIMITED,  (INT32)eCM_COLOR_SPACE_RGB_LIMITED, (INT32)CLI_COLOR_SPACE_RGB_LIMITED},
    /* 03 */ {(INT32)eGUI_COLOR_SPACE_YUV_REC709,   (INT32)eCM_COLOR_SPACE_YUV_REC709,  (INT32)CLI_COLOR_SPACE_YUV_REC709},
    /* 04 */ {(INT32)eGUI_COLOR_SPACE_YUV_REC601,   (INT32)eCM_COLOR_SPACE_YUV_REC601,  (INT32)CLI_COLOR_SPACE_YUV_REC601},
};

/////////////////////////////////////////////////////////////////////////////////////
// Wall Color
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_WallColor[] =
{
    /* 00 */ {(INT32)eGUI_WALL_COLOR_WHITE,         (INT32)eCM_WALL_COLOR_WHITE,            (INT32)CLI_WALL_COLOR_WHITE},
    /* 01 */ {(INT32)eGUI_WALL_COLOR_GRAY130,       (INT32)eCM_WALL_COLOR_GRAY130,          (INT32)CLI_WALL_COLOR_GRAY130},
    /* 02 */ {(INT32)eGUI_WALL_COLOR_LIGHT_YELLOW,  (INT32)eCM_WALL_COLOR_LIGHT_YELLOW,     (INT32)CLI_WALL_COLOR_LIGHT_YELLOW},
};

#if 0
/////////////////////////////////////////////////////////////////////////////////////
// 3D Mode
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_3DMode[] =
{
    /* 00 */ {(INT32)eGUI_3D_MODE_AUTO, (INT32)eCM_3D_MODE_AUTO, (INT32)CLI_3D_MODE_AUTO},
    /* 01 */ {(INT32)eGUI_3D_MODE_OFF,  (INT32)eCM_3D_MODE_OFF,  (INT32)CLI_3D_MODE_OFF},
    /* 02 */ {(INT32)eGUI_3D_MODE_ON,   (INT32)eCM_3D_MODE_ON,   (INT32)CLI_3D_MODE_ON},
};
#endif

/////////////////////////////////////////////////////////////////////////////////////
// 3D Format
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_3DFormat[] =
{
    /* 00 */ {(INT32)eGUI_3D_FORMAT_AUTO,               (INT32)eCM_3D_FORMAT_AUTO,              (INT32)CLI_3D_FORMAT_AUTO},
    /* 01 */ {(INT32)eGUI_3D_FORMAT_FRAME_PACKING,      (INT32)eCM_3D_FORMAT_FRAME_PACKING,     (INT32)CLI_3D_FORMAT_FRAME_PACKING},
    /* 02 */ {(INT32)eGUI_3D_FORMAT_SIDE_BY_SIDE,       (INT32)eCM_3D_FORMAT_SIDE_BY_SIDE,      (INT32)CLI_3D_FORMAT_SIDE_BY_SIDE},
    /* 03 */ {(INT32)eGUI_3D_FORMAT_TOP_AND_BOTTOM,     (INT32)eCM_3D_FORMAT_TOP_AND_BOTTOM,    (INT32)CLI_3D_FORMAT_TOP_AND_BOTTOM},
    /* 04 */ {(INT32)eGUI_3D_FORMAT_FRAME_SEQUENTIAL,   (INT32)eCM_3D_FORMAT_FRAME_SEQUENTIAL,  (INT32)CLI_3D_FORMAT_FRAME_SEQUENTIAL},
    /* 05 */ {(INT32)eGUI_3D_FORMAT_DUALPIPE,           (INT32)eCM_3D_FORMAT_DUALPIPE_3D,       (INT32)CLI_3D_FORMAT_DUALPIPE},
    /* 06 */ {(INT32)eGUI_3D_FORMAT_4K3D,               (INT32)eCM_3D_FORMAT_4K3D,              (INT32)CLI_3D_FORMAT_4K3D},
    /* 07 */ {(INT32)eGUI_3D_FORMAT_4K3D_DUALPIPE,      (INT32)eCM_3D_FORMAT_4K3D_DUALPIPE,     (INT32)CLI_3D_FORMAT_4K3D_DUALPIPE},
    /* 08 */ {(INT32)eGUI_3D_FORMAT_FRAME_OFF,          (INT32)eCM_3D_FORMAT_OFF,               (INT32)CLI_3D_FORMAT_OFF},
};

/////////////////////////////////////////////////////////////////////////////////////
// 3D Tech
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_3DTech[] =
{
    /* 00 */ {(INT32)eGUI_3D_TECH_DLP_LINK, (INT32)eCM_3D_TECH_DLP_LINK, (INT32)CLI_3D_TECH_DLP_LINK},
    /* 01 */ {(INT32)eGUI_3D_TECH_3D_SYNC,  (INT32)eCM_3D_TECH_3D_SYNC,  (INT32)CLI_3D_TECH_3D_SYNC},
};

/////////////////////////////////////////////////////////////////////////////////////
// 3D-2D
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_3D2DView[] =
{
    /* 00 */ {(INT32)eGUI_3D_2D_3D,         (INT32)eCM_3D_2D_3D,        (INT32)CLI_3D_2D_3D},
    /* 01 */ {(INT32)eGUI_3D_2D_2D_LEFT,    (INT32)eCM_3D_2D_2D_LEFT,   (INT32)CLI_3D_2D_2D_LEFT},
    /* 02 */ {(INT32)eGUI_3D_2D_2D_RIGHT,   (INT32)eCM_3D_2D_2D_RIGHT,  (INT32)CLI_3D_2D_2D_RIGHT},
};

/////////////////////////////////////////////////////////////////////////////////////
// 3D Sync Out
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_3DSyncOut[] =
{
    /* 00 */ {(INT32)eGUI_3D_SYNC_OUT_TO_EMITTER,        (INT32)eCM_3D_SYNC_OUT_TO_EMITTER,         (INT32)CLI_3D_SYNC_OUT_TO_EMITTER},
    /* 01 */ {(INT32)eGUI_3D_SYNC_OUT_TO_NEXT_PROJECTOR, (INT32)eCM_3D_SYNC_OUT_TO_NEXT_PROJECTOR,  (INT32)CLI_3D_SYNC_OUT_TO_NEXT_PROJECTOR},
};

/////////////////////////////////////////////////////////////////////////////////////
// 3D LR Reference
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_3DLRReference[] =
{
    /* 00 */ {(INT32)eGUI_3D_LR_REFERENCE_1ST_FRAME,     (INT32)eCM_3D_LR_REFERENCE_1ST_FRAME,   (INT32)CLI_3D_LR_REFERENCE_1ST_FRAME},
    /* 01 */ {(INT32)eGUI_3D_LR_REFERENCE_FIELD_GPIO,    (INT32)eCM_3D_LR_REFERENCE_FIELD_GPIO,  (INT32)CLI_3D_LR_REFERENCE_FIELD_GPIO},
};

/////////////////////////////////////////////////////////////////////////////////////
// 3D Sync Select (3D Sync Type)
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_3DSyncType[] =
{
    /* 00 */ {(INT32)eGUI_3D_SYNC_TYPE_AUTO,        (INT32)eCM_3D_SYNC_TYPE_AUTO,       (INT32)CLI_3D_SYNC_TYPE_AUTO},
    /* 01 */ {(INT32)eGUI_3D_SYNC_TYPE_INTERNAL,    (INT32)eCM_3D_SYNC_TYPE_INTERNAL,   (INT32)CLI_3D_SYNC_TYPE_INTERNAL},
    /* 02 */ {(INT32)eGUI_3D_SYNC_TYPE_EXTERNAL,    (INT32)eCM_3D_SYNC_TYPE_EXTERNAL,   (INT32)CLI_3D_SYNC_TYPE_EXTERNAL},
};

/////////////////////////////////////////////////////////////////////////////////////
// PIP Size
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_PIPSize[] =
{
    /* 00 */ {(INT32)eGUI_PIP_SIZE_SMALL,  (INT32)eCM_PIP_SIZE_SMALL,  (INT32)CLI_PIP_SIZE_SMALL},
    /* 01 */ {(INT32)eGUI_PIP_SIZE_MEDIUM, (INT32)eCM_PIP_SIZE_MEDIUM, (INT32)CLI_PIP_SIZE_MEDIUM},
    /* 02 */ {(INT32)eGUI_PIP_SIZE_LARGE,  (INT32)eCM_PIP_SIZE_LARGE,  (INT32)CLI_PIP_SIZE_LARGE},
};

/////////////////////////////////////////////////////////////////////////////////////
// PIP Layout
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_PIPLayout[] =
{
    /* 00 */ {(INT32)eGUI_MAIN_LAYOUT_PBP_MAIN_LEFT,     (INT32)eCM_MAIN_LAYOUT_PBP_MAIN_LEFT,   (INT32)CLI_MAIN_LAYOUT_PBP_MAIN_LEFT},
    /* 01 */ {(INT32)eGUI_MAIN_LAYOUT_PBP_MAIN_TOP,      (INT32)eCM_MAIN_LAYOUT_PBP_MAIN_TOP,    (INT32)CLI_MAIN_LAYOUT_PBP_MAIN_TOP},
    /* 02 */ {(INT32)eGUI_MAIN_LAYOUT_PBP_MAIN_RIGHT,    (INT32)eCM_MAIN_LAYOUT_PBP_MAIN_RIGHT,  (INT32)CLI_MAIN_LAYOUT_PBP_MAIN_RIGHT},
    /* 03 */ {(INT32)eGUI_MAIN_LAYOUT_PBP_MAIN_BOTTOM,   (INT32)eCM_MAIN_LAYOUT_PBP_MAIN_BOTTOM, (INT32)CLI_MAIN_LAYOUT_PBP_MAIN_BOTTOM},
    /* 04 */ {(INT32)eGUI_MAIN_LAYOUT_PIP_BOTTOM_RIGHT,  (INT32)eCM_MAIN_LAYOUT_PIP_BOTTOM_RIGHT,(INT32)CLI_MAIN_LAYOUT_PIP_BOTTOM_RIGHT},
    /* 05 */ {(INT32)eGUI_MAIN_LAYOUT_PIP_BOTTOM_LEFT,   (INT32)eCM_MAIN_LAYOUT_PIP_BOTTOM_LEFT, (INT32)CLI_MAIN_LAYOUT_PIP_BOTTOM_LEFT},
    /* 06 */ {(INT32)eGUI_MAIN_LAYOUT_PIP_TOP_LEFT,      (INT32)eCM_MAIN_LAYOUT_PIP_TOP_LEFT,    (INT32)CLI_MAIN_LAYOUT_PIP_TOP_LEFT},
    /* 07 */ {(INT32)eGUI_MAIN_LAYOUT_PIP_TOP_RIGHT,     (INT32)eCM_MAIN_LAYOUT_PIP_TOP_RIGHT,   (INT32)CLI_MAIN_LAYOUT_PIP_TOP_RIGHT},
};

/////////////////////////////////////////////////////////////////////////////////////
// Lan Path Switch
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_LanPathSwitch[] =
{
    /* 00 */ {(INT32)eGUI_LAN_PATH_SWITCH_RJ45,     (INT32)eCM_LAN_PATH_SWITCH_RJ45,    (INT32)CLI_LAN_PATH_SWITCH_RJ45},
    /* 01 */ {(INT32)eGUI_LAN_PATH_SWITCH_HDBASET,  (INT32)eCM_LAN_PATH_SWITCH_HDBASET, (INT32)CLI_LAN_PATH_SWITCH_HDBASET},
};

/////////////////////////////////////////////////////////////////////////////////////
// Serial Port Baud Rate
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_SerialPortBaudRate[] =
{
    /* 00 */ {(INT32)eGUI_SERIAL_PORT_BAUD_RATE_1200,   (INT32)eCM_SERIAL_PORT_BAUD_RATE_1200,  (INT32)CLI_SERIAL_PORT_BAUD_RATE_1200},
    /* 01 */ {(INT32)eGUI_SERIAL_PORT_BAUD_RATE_2400,   (INT32)eCM_SERIAL_PORT_BAUD_RATE_2400,  (INT32)CLI_SERIAL_PORT_BAUD_RATE_2400},
    /* 02 */ {(INT32)eGUI_SERIAL_PORT_BAUD_RATE_4800,   (INT32)eCM_SERIAL_PORT_BAUD_RATE_4800,  (INT32)CLI_SERIAL_PORT_BAUD_RATE_4800},
    /* 03 */ {(INT32)eGUI_SERIAL_PORT_BAUD_RATE_9600,   (INT32)eCM_SERIAL_PORT_BAUD_RATE_9600,  (INT32)CLI_SERIAL_PORT_BAUD_RATE_9600},
    /* 04 */ {(INT32)eGUI_SERIAL_PORT_BAUD_RATE_19200,  (INT32)eCM_SERIAL_PORT_BAUD_RATE_19200, (INT32)CLI_SERIAL_PORT_BAUD_RATE_19200},
    /* 05 */ {(INT32)eGUI_SERIAL_PORT_BAUD_RATE_38400,  (INT32)eCM_SERIAL_PORT_BAUD_RATE_38400, (INT32)CLI_SERIAL_PORT_BAUD_RATE_38400},
    /* 06 */ {(INT32)eGUI_SERIAL_PORT_BAUD_RATE_57600,  (INT32)eCM_SERIAL_PORT_BAUD_RATE_57600, (INT32)CLI_SERIAL_PORT_BAUD_RATE_57600},
    /* 07 */ {(INT32)eGUI_SERIAL_PORT_BAUD_RATE_115200, (INT32)eCM_SERIAL_PORT_BAUD_RATE_115200,(INT32)CLI_SERIAL_PORT_BAUD_RATE_115200},
};

/////////////////////////////////////////////////////////////////////////////////////
// OSD Time Out
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_OSDTimeOut[] =
{
    /* 00 */ {(INT32)eGUI_OSD_TIMEOUT_OFF,      (INT32)eCM_OSD_TIMEOUT_OFF,     (INT32)CLI_OSD_TIMEOUT_OFF},
    /* 01 */ {(INT32)eGUI_OSD_TIMEOUT_60_SEC,    (INT32)eCM_OSD_TIMEOUT_60_SEC,   (INT32)CLI_OSD_TIMEOUT_60_SEC},
    /* 02 */ {(INT32)eGUI_OSD_TIMEOUT_180_SEC,   (INT32)eCM_OSD_TIMEOUT_180_SEC,  (INT32)CLI_OSD_TIMEOUT_180_SEC},
};

/////////////////////////////////////////////////////////////////////////////////////
// Back Ground Color
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_BackgroundColor[] =
{
    /* 00 */ {(INT32)eGUI_BACKGROUND_COLOR_BLUE,    (INT32)eCM_BACKGROUND_COLOR_BLUE,   (INT32)CLI_BACKGROUND_COLOR_BLUE},
    /* 01 */ {(INT32)eGUI_BACKGROUND_COLOR_BLACK,   (INT32)eCM_BACKGROUND_COLOR_BLACK,  (INT32)CLI_BACKGROUND_COLOR_BLACK},
    /* 02 */ {(INT32)eGUI_BACKGROUND_COLOR_WHITE,   (INT32)eCM_BACKGROUND_COLOR_WHITE,  (INT32)CLI_BACKGROUND_COLOR_WHITE},
    /* 03 */ {(INT32)eGUI_BACKGROUND_COLOR_LOGO,    (INT32)eCM_BACKGROUND_COLOR_LOGO,   (INT32)CLI_BACKGROUND_COLOR_LOGO},
};

/////////////////////////////////////////////////////////////////////////////////////
// Standby Mode
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_StandbyMode[] =
{
    /* 00 */ {(INT32)eGUI_STANDBY_MODE_STANDBY,         (INT32)eCM_STANDBY_MODE_0_5W,           (INT32)CLI_STANDBY_MODE_STANDBY},
    ///* 01 */ {(INT32)eGUI_STANDBY_MODE_NETWORK_STANDBY, (INT32)eCM_STANDBY_MODE_2W,             (INT32)CLI_STANDBY_MODE_NETWORK_STANDBY},
    /* 02 */ {(INT32)eGUI_STANDBY_MODE_COMMUNICATION,   (INT32)eCM_STANDBY_MODE_COMMUNICATION,  (INT32)CLI_STANDBY_MODE_COMMUNICATION},
};

/////////////////////////////////////////////////////////////////////////////////////
// Power Mode (Light Source Mode)
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_PowerMode[] =
{
    /* 00 */ {(INT32)eGUI_POWER_MODE_CONSTANT_POWER,    (INT32)eCM_POWER_MODE_CONSTANT_POWER,       (INT32)CLI_POWER_MODE_CONSTANT_POWER},
    /* 01 */ {(INT32)eGUI_POWER_MODE_CONSTANT_INTENSITY,(INT32)eCM_POWER_MODE_CONSTANT_INTENSITY,   (INT32)CLI_POWER_MODE_CONSTANT_INTENSITY},
    /* 02 */ {(INT32)eGUI_POWER_MODE_ECO1,               (INT32)eCM_POWER_MODE_ECO1,                 (INT32)CLI_POWER_MODE_ECO1},
    /* 03 */ {(INT32)eGUI_POWER_MODE_ECO2,               (INT32)eCM_POWER_MODE_ECO2,                 (INT32)CLI_POWER_MODE_ECO2},
};

/////////////////////////////////////////////////////////////////////////////////////
// Splash Startup
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_SplashStartup[] =
{
    /* 00 */ {(INT32)eGUI_LOGO_PATTERN_FACTORY_LOGO,  (INT32)eCM_LOGO_PATTERN_FACTORY_LOGO, (INT32)CLI_LOGO_PATTERN_FACTORY_LOGO},
    /* 01 */ {(INT32)eGUI_LOGO_PATTERN_BLUE,          (INT32)eCM_LOGO_PATTERN_BLUE,         (INT32)CLI_LOGO_PATTERN_BLUE},
    /* 02 */ {(INT32)eGUI_LOGO_PATTERN_BLACK,         (INT32)eCM_LOGO_PATTERN_BLACK,        (INT32)CLI_LOGO_PATTERN_BLACK},
    /* 03 */ {(INT32)eGUI_LOGO_PATTERN_WHITE,         (INT32)eCM_LOGO_PATTERN_WHITE,        (INT32)CLI_LOGO_PATTERN_WHITE},
};

/////////////////////////////////////////////////////////////////////////////////////
// Low Latency
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_LowLatency[] =
{
    /* 00 */ {(INT32)eGUI_LOW_LATENCY_MODE_NORMAL,      (INT32)eCM_LOW_LATENCY_MODE_OFF,        (INT32)CLI_LOW_LATENCY_MODE_NORMAL},
    /* 01 */ {(INT32)eGUI_LOW_LATENCY_MODE_TYPICAL,     (INT32)eCM_LOW_LATENCY_MODE_TYPICAL,    (INT32)CLI_LOW_LATENCY_MODE_TYPICAL},
};

/////////////////////////////////////////////////////////////////////////////////////
// Detail
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_Detail[] =
{
    /* 00 */ {(INT32)eGUI_DETAIL_MAXIMUM,   (INT32)eCM_SHARPNESS_LEVEL13_MAXIMUM,    (INT32)CLI_DETAIL_MAXIMUM},
    /* 01 */ {(INT32)eGUI_DETAIL_HIGH,      (INT32)eCM_SHARPNESS_LEVEL11_HIGH,       (INT32)CLI_DETAIL_HIGH},
    /* 02 */ {(INT32)eGUI_DETAIL_NORMAL,    (INT32)eCM_SHARPNESS_LEVEL8_NORMAL,      (INT32)CLI_DETAIL_NORMAL},
    /* 03 */ {(INT32)eGUI_DETAIL_LOW,       (INT32)eCM_SHARPNESS_LEVEL5_LOW,         (INT32)CLI_DETAIL_LOW},
    /* 04 */ {(INT32)eGUI_DETAIL_MINIMUM,   (INT32)eCM_SHARPNESS_LEVEL3_MINIMUM,     (INT32)CLI_DETAIL_MINIMUM},
};

/////////////////////////////////////////////////////////////////////////////////////
// HDR Level
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_HdrLevel[] =
{
    /* 00 */ {(INT32)eGUI_HDR_LEVEL_1,      (INT32)eCM_HDR_LEVEL1,       (INT32)CLI_HDR_LEVEL_1},
    /* 01 */ {(INT32)eGUI_HDR_LEVEL_2,      (INT32)eCM_HDR_LEVEL2,       (INT32)CLI_HDR_LEVEL_2},
    /* 02 */ {(INT32)eGUI_HDR_LEVEL_3,      (INT32)eCM_HDR_LEVEL3,       (INT32)CLI_HDR_LEVEL_3},
    /* 03 */ {(INT32)eGUI_HDR_LEVEL_4,      (INT32)eCM_HDR_LEVEL4,       (INT32)CLI_HDR_LEVEL_4},
};

/////////////////////////////////////////////////////////////////////////////////////
// DateTime ClockMode
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_ClockMode[] =
{
    /* 00 */ {(INT32)eGUI_CLOCK_MODE_USE_NTP_SERVER,      (INT32)eCM_DATETIME_CLOCK_MODE_NTP_SERVER,        (INT32)CLI_CLOCK_MODE_USE_NTP_SERVER},
    /* 01 */ {(INT32)eGUI_CLOCK_MODE_MANUAL,              (INT32)eCM_DATETIME_CLOCK_MODE_MANUAL,            (INT32)CLI_CLOCK_MODE_MANUAL},
};

/////////////////////////////////////////////////////////////////////////////////////
// DateTime Update Interval
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_UpdateInterval[] =
{
    /* 00 */ {(INT32)eGUI_UPDATE_INTERVAL_HOURLY,             (INT32)eCM_DATETIME_UPDATE_INTERVAL_HOURLY,   (INT32)CLI_UPDATE_INTERVAL_HOURLY},
    /* 01 */ {(INT32)eGUI_UPDATE_INTERVAL_DAILY,              (INT32)eCM_DATETIME_UPDATE_INTERVAL_DAILY,    (INT32)CLI_UPDATE_INTERVAL_DAILY},
};

/////////////////////////////////////////////////////////////////////////////////////
// HDMI output
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_HDMIOutput[] =
{
    /* 00 */ {(INT32)eGUI_HDMI_OUT_HDMI1,               (INT32)eCM_HDMI_OUTPUT_HDMI1,     (INT32)CLI_HDMI_OUTPUT_HDMI1},
    /* 01 */ {(INT32)eGUI_HDMI_OUT_HDMI2,               (INT32)eCM_HDMI_OUTPUT_HDMI2,     (INT32)CLI_HDMI_OUTPUT_HDMI2},
};

/////////////////////////////////////////////////////////////////////////////////////
// COPY Event to Weekday
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_ScheduleCopyEventToWeekday[] =
{
    /* 00 */ {(INT32)eGUI_COPY_EVENT_TO_MONDAY,         (INT32)eCM_WEEKDAY_MONDAY,           (INT32)CLI_COPY_EVENT_TO_MONDAY},
    /* 01 */ {(INT32)eGUI_COPY_EVENT_TO_TUESDAY,        (INT32)eCM_WEEKDAY_TUESDAY,          (INT32)CLI_COPY_EVENT_TO_TUESDAY},
    /* 02 */ {(INT32)eGUI_COPY_EVENT_TO_WEDNESDAY,      (INT32)eCM_WEEKDAY_WEDNESDAY,        (INT32)CLI_COPY_EVENT_TO_WEDNESDAY},
    /* 03 */ {(INT32)eGUI_COPY_EVENT_TO_THURSDAY,       (INT32)eCM_WEEKDAY_THURSDAY,         (INT32)CLI_COPY_EVENT_TO_THURSDAY},
    /* 04 */ {(INT32)eGUI_COPY_EVENT_TO_FRIDAY,         (INT32)eCM_WEEKDAY_FRIDAY,           (INT32)CLI_COPY_EVENT_TO_FRIDAY},
    /* 05 */ {(INT32)eGUI_COPY_EVENT_TO_SATURDAY,       (INT32)eCM_WEEKDAY_SATURDAY,         (INT32)CLI_COPY_EVENT_TO_SATURDAY},
    /* 06 */ {(INT32)eGUI_COPY_EVENT_TO_SUNDAY,         (INT32)eCM_WEEKDAY_SUNDAY,           (INT32)CLI_COPY_EVENT_TO_SUNDAY},
};

/////////////////////////////////////////////////////////////////////////////////////
// 3D Mode
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_3DMode[] =
{
    /* 00 */ {(INT32)eGUI_3D_MODE_ACTIVE_3D,  (INT32)eCM_3D_MODE_ON,          (INT32)CLI_3D_MODE_ACTIVE_3D}, //active 3D
    /* 01 */ {(INT32)eGUI_3D_MODE_PASSIVE_3D, (INT32)eCM_3D_MODE_PASSIVE_3D,  (INT32)CLI_3D_MODE_PASSIVE_3D},
};

/////////////////////////////////////////////////////////////////////////////////////
// Warp Memory Save
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_GeoMemorySave[] =
{
    /* 00 */ {(INT32)eGUI_GEO_MEMORY_SAVE_1,           (INT32)eCM_GEO_MEMORY_SAVE_1,            (INT32)CLI_GEO_SAVE_MEMORY1},
    /* 01 */ {(INT32)eGUI_GEO_MEMORY_SAVE_2,           (INT32)eCM_GEO_MEMORY_SAVE_2,            (INT32)CLI_GEO_SAVE_MEMORY2},
    /* 02 */ {(INT32)eGUI_GEO_MEMORY_SAVE_3,           (INT32)eCM_GEO_MEMORY_SAVE_3,            (INT32)CLI_GEO_SAVE_MEMORY3},
    /* 03 */ {(INT32)eGUI_GEO_MEMORY_SAVE_4,           (INT32)eCM_GEO_MEMORY_SAVE_4,            (INT32)CLI_GEO_SAVE_MEMORY4},
};

/////////////////////////////////////////////////////////////////////////////////////
// Warp Memory Apply
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_GeoMemoryApply[] =
{
    /* 00 */ {(INT32)eGUI_GEO_MEMORY_APPLY_OFF,         (INT32)eCM_GEO_MEMORY_APPLY_OFF,          (INT32)CLI_GEO_APPLY_OFF},
    /* 01 */ {(INT32)eGUI_GEO_MEMORY_APPLY_1,           (INT32)eCM_GEO_MEMORY_APPLY_1,            (INT32)CLI_GEO_APPLY_MEMORY1},
    /* 02 */ {(INT32)eGUI_GEO_MEMORY_APPLY_2,           (INT32)eCM_GEO_MEMORY_APPLY_2,            (INT32)CLI_GEO_APPLY_MEMORY2},
    /* 03 */ {(INT32)eGUI_GEO_MEMORY_APPLY_3,           (INT32)eCM_GEO_MEMORY_APPLY_3,            (INT32)CLI_GEO_APPLY_MEMORY3},
    /* 04 */ {(INT32)eGUI_GEO_MEMORY_APPLY_4,           (INT32)eCM_GEO_MEMORY_APPLY_4,            (INT32)CLI_GEO_APPLY_MEMORY4},
};

/////////////////////////////////////////////////////////////////////////////
///////////////////////////  Data Mapping LUT  ////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
sDATA_MAPPING_LUT_FORMAT sDATA_MAPPING_LUT[] =
{
    {edcMAIN_INPUT,  asMPT_SourceID,  MPT_SIZE(asMPT_SourceID)},
    {edcSUB_INPUT,   asMPT_SourceID,  MPT_SIZE(asMPT_SourceID)},
    {edcBACKUPINPUT_CURRENT_SOURCE,  asMPT_SourceID,  MPT_SIZE(asMPT_SourceID)},
    //{edcBACKUPINPUT_PRIMARY_INPUT,  asMPT_SourceID,  MPT_SIZE(asMPT_SourceID)},   //G100_Steven_0021
    //{edcBACKUPINPUT_SECONDARY_INPUT,  asMPT_SourceID,  MPT_SIZE(asMPT_SourceID)}, //G100_Steven_0021

    {edcOSDTEST_PATTERN,  asMPT_TestPattern,  MPT_SIZE(asMPT_TestPattern)},
    {edcSERVICE_TEST_PATTERN,  asMPT_ServiceTestPattern,  MPT_SIZE(asMPT_ServiceTestPattern)},
    {edcTWIST_PATTERN,  asMPT_TwistTestPattern,  MPT_SIZE(asMPT_TwistTestPattern)},

    {edcPICTURE_SETTINGS,  asMPT_PictureSettings,  MPT_SIZE(asMPT_PictureSettings)},
    {edcFORCE_PICTURE_SETTINGS,  asMPT_PictureSettings,  MPT_SIZE(asMPT_PictureSettings)},
    {edcPRE_USER_MODE,  asMPT_PictureSettings,  MPT_SIZE(asMPT_PictureSettings)},
    {edcUSER_COLOR_MODE,  asMPT_ApplyUserSettings,  MPT_SIZE(asMPT_ApplyUserSettings)}, 	//G100_Doulas_0066

    {edcLANGUAGE,  asMPT_Language,  MPT_SIZE(asMPT_Language)},

    {edcSIZE_PRESETS,  asMPT_ScalingMode,  MPT_SIZE(asMPT_ScalingMode)},
    {edcMAIN_ASPECT_RATIO,  asMPT_ScalingMode,  MPT_SIZE(asMPT_ScalingMode)},
    {edcSUB_ASPECT_RATIO,  asMPT_ScalingMode,  MPT_SIZE(asMPT_ScalingMode)},

    {edcHOT_KEY_SETTINGS,  asMPT_HotkeySettings,  MPT_SIZE(asMPT_HotkeySettings)},
    {edcHOT_KEY2_SETTINGS,  asMPT_HotkeySettings,  MPT_SIZE(asMPT_HotkeySettings)},

    {edcPIP_PBP_ENABLE,  asMPT_ScreenMode,  MPT_SIZE(asMPT_ScreenMode)},
    {edcSIZE,  asMPT_PIPSize,  MPT_SIZE(asMPT_PIPSize)},

    {edcMENU_LOCATION,  asMPT_MenuLocation,  MPT_SIZE(asMPT_MenuLocation)},

    {edcCEILING_MOUNT,  asMPT_CeilingMount,  MPT_SIZE(asMPT_CeilingMount)},

    {edcGAMMA,  asMPT_Gamma,  MPT_SIZE(asMPT_Gamma)},

    {edcLENS_SAVE_CURRENT_POSITION,  asMPT_LensMemorySave,  MPT_SIZE(asMPT_LensMemorySave)},
    {edcLENS_APPLY_POSITION,  (asMPT_LensMemoryApply),  MPT_SIZE(asMPT_LensMemoryApply)},

    {edcREAR_PROJECTION,  (asMPT_OrientationDirection),  MPT_SIZE(asMPT_OrientationDirection)},

    {edcHDMI_EDID_1,  asMPT_EDID_Type,  MPT_SIZE(asMPT_EDID_Type)},
    {edcHDMI_EDID_2,  asMPT_EDID_Type,  MPT_SIZE(asMPT_EDID_Type)},
    {edcHDBASET_EDID,  asMPT_EDID_Type2,  MPT_SIZE(asMPT_EDID_Type2)},

    {edcBLENDING_GAMMA,  asMPT_BlendingGamma,  MPT_SIZE(asMPT_BlendingGamma)},

    {edcCOLOR_TEMPERATURE,  asMPT_ColorTemperature,  MPT_SIZE(asMPT_ColorTemperature)},

    {edcCOLOR_SPACE,  asMPT_ColorSpace,  MPT_SIZE(asMPT_ColorSpace)},

    {edcWALL_COLOR,  asMPT_WallColor,  MPT_SIZE(asMPT_WallColor)},

    //{edc3D_MODE,  asMPT_3DMode,  MPT_SIZE(asMPT_3DMode)},
    {edc3D_ENABLE,  asMPT_3DFormat,  MPT_SIZE(asMPT_3DFormat)},
    {edcDLPLink_ON,  asMPT_3DTech,  MPT_SIZE(asMPT_3DTech)},
    {edc3D_2D,  asMPT_3D2DView,  MPT_SIZE(asMPT_3D2DView)},
    {edc3D_SYNC_OUT,  asMPT_3DSyncOut,  MPT_SIZE(asMPT_3DSyncOut)},
    {edc3D_LR_REFERENCE,  asMPT_3DLRReference,  MPT_SIZE(asMPT_3DLRReference)},
    {edc3D_SYNC_TYPE,  asMPT_3DSyncType,  MPT_SIZE(asMPT_3DSyncType)},

    {edcSIZE,  asMPT_PIPSize,  MPT_SIZE(asMPT_PIPSize)},
    {edcMAIN_LAYOUT,  asMPT_PIPLayout,  MPT_SIZE(asMPT_PIPLayout)},

    {edcLAN_PATH_SWITCH,  asMPT_LanPathSwitch,  MPT_SIZE(asMPT_LanPathSwitch)},

    {edcSERIAL_PORT_BAUD_RATE,  asMPT_SerialPortBaudRate,  MPT_SIZE(asMPT_SerialPortBaudRate)},
    {edcSERIAL_PORT_OUT_BAUD_RATE,  asMPT_SerialPortBaudRate,  MPT_SIZE(asMPT_SerialPortBaudRate)},

    {edcMENU_TIME_OUT,  asMPT_OSDTimeOut,  MPT_SIZE(asMPT_OSDTimeOut)},

    {edcBACKGROUND_COLOR,  asMPT_BackgroundColor,  MPT_SIZE(asMPT_BackgroundColor)},

    {edcSTANDBY_MODE,  asMPT_StandbyMode,  MPT_SIZE(asMPT_StandbyMode)},

    {edcPOWER_MODE,  asMPT_PowerMode,  MPT_SIZE(asMPT_PowerMode)},

    {edcSPLASH_STARTUP,  asMPT_SplashStartup,  MPT_SIZE(asMPT_SplashStartup)},

    {edcLOW_LATENCY_MODE,  asMPT_LowLatency,  MPT_SIZE(asMPT_LowLatency)},

    {edcDETAIL,  asMPT_Detail,  MPT_SIZE(asMPT_Detail)},

    {edcHDR_LEVEL,  asMPT_HdrLevel,  MPT_SIZE(asMPT_HdrLevel)},

    {edcDATE_MODE,  asMPT_ClockMode,  MPT_SIZE(asMPT_ClockMode)},

    {edcDATE_UPDATE_INTERVAL,  asMPT_UpdateInterval,  MPT_SIZE(asMPT_UpdateInterval)},

    {edcHDMI_OUT,  asMPT_HDMIOutput,  MPT_SIZE(asMPT_HDMIOutput)},

    {edcSCHEDULE_COPY_EVENT_INDEX, asMPT_ScheduleCopyEventToWeekday, MPT_SIZE(asMPT_ScheduleCopyEventToWeekday)},

	{edc3D_MODE,  asMPT_3DMode,  MPT_SIZE(asMPT_3DMode)},	//A70Gen2_Doulas_0054
};
#define DATA_MAPPING_LUT_SIZE (sizeof(sDATA_MAPPING_LUT)/sizeof(sDATA_MAPPING_LUT_FORMAT))

/////////////////////////////////////////////////////////////////////////////


INT32 utilDataMapping_Init(UINT8 IsCreator)
{
    if(IsCreator)
    {
        DataMapping_DebugModeSet(eDEBUG_FATAL_ERROR);
        DataMapping_Init(sDATA_MAPPING_LUT, DATA_MAPPING_LUT_SIZE);
    }
    else
    {
        DataMapping_DebugModeSet(eDEBUG_FATAL_ERROR);
        DataMapping_ReaderInit();
    }
}

INT32 utilDataMapping_DeInit(void)
{
    DataMapping_DeInit();
}

INT32 utilDataMapping_CMValueRangeCheck(eDATA_CODE eDataCode, INT32 lCM_Value)
{
    if(IS_COMMON_DATA_CODE(eDataCode))
    {
        if(DataMapping_DataRangeCheck(eDataCode, lCM_Value) == RANGE_CHECK_PASS)
        {
            return DATA_RANGE_CHECK_PASS;
        }
    }
    else  //not common data code
    {
        INT32 iMax = CommonAPI_DataCode_MaxValue_Get(eDataCode);
        INT32 iMin = CommonAPI_DataCode_MinValue_Get(eDataCode);

        if((iMax >= lCM_Value) && (lCM_Value >= iMin))
        {
            return DATA_RANGE_CHECK_PASS;
        }
    }

    return DATA_RANGE_CHECK_FAIL;
}

INT32 utilDataMapping_GuiValueRangeCheck(eDATA_CODE eDataCode, INT32 lGui_Value)
{
    if(IS_COMMON_DATA_CODE(eDataCode))
    {
        if( lGui_Value >= 0 && lGui_Value <= (GUI_ITEM_SIZE(eDataCode)-1))
        {
            return DATA_RANGE_CHECK_PASS;
        }
    }
    else  //not common data code
    {
        INT32 iMax = CommonAPI_DataCode_MaxValue_Get(eDataCode);
        INT32 iMin = CommonAPI_DataCode_MinValue_Get(eDataCode);

        if((iMax >= lGui_Value) && (lGui_Value >= iMin))
        {
            return DATA_RANGE_CHECK_PASS;
        }
    }

    return DATA_RANGE_CHECK_FAIL;
}

//end of defined(CUSTOM_CHRISTIE)

#elif defined(CUSTOM_OPTOMA)
typedef enum
{
    /* 00 */ eGM_SOURCE_ID_HDMI1,
    /* 01 */ eGM_SOURCE_ID_HDMI2,
    /* 02 */ eGM_SOURCE_ID_DP,
    /* 03 */ eGM_SOURCE_ID_3GSDI,
    /* 04 */ eGM_SOURCE_ID_12GSDI,
    /* 05 */ eGM_SOURCE_ID_HDBASET,

    eGM_SOURCE_ID_NUMBER,
}eGM_SOURCE_ID;

typedef enum
{
    /* 00 */ eGM_BACKUPINPUT_SOURCE_ID_HDMI1,
    /* 01 */ eGM_BACKUPINPUT_SOURCE_ID_HDMI2,
    /* 02 */ eGM_BACKUPINPUT_SOURCE_ID_DP,
    /* 03 */ eGM_BACKUPINPUT_SOURCE_ID_3GSDI,
    /* 04 */ eGM_BACKUPINPUT_SOURCE_ID_12GSDI,
    /* 05 */ eGM_BACKUPINPUT_SOURCE_ID_HDBASET,

    eGM_BACKUPINPUT_SOURCE_ID_NUMBER,
}eGM_BACKUPINPUT_SOURCE_ID;

typedef enum
{
    /* 00 */ eGM_TEST_PATTERN_OFF,
    /* 01 */ eGM_TEST_PATTERN_GREEN_GRID,
    /* 02 */ eGM_TEST_PATTERN_MAGENTA_GRID,
    /* 03 */ eGM_TEST_PATTERN_WHITE_GRID,
    /* 04 */ eGM_TEST_PATTERN_WHITE,
    /* 05 */ eGM_TEST_PATTERN_BLACK,
    /* 06 */ eGM_TEST_PATTERN_RED,
    /* 07 */ eGM_TEST_PATTERN_GREEN,
    /* 08 */ eGM_TEST_PATTERN_BLUE,
    /* 09 */ eGM_TEST_PATTERN_YELLOW,
    /* 10 */ eGM_TEST_PATTERN_MAGENTA,
    /* 11 */ eGM_TEST_PATTERN_CYAN,
    /* 12 */ eGM_TEST_PATTERN_CHECKERBOARD,
    /* 13 */ eGM_TEST_PATTERN_COLORBAR,
    /* 14 */ eGM_TEST_PATTERN_4K_FULL_SCREEN,

    eGM_TEST_PATTERN_NUMBER,
}eGM_TEST_PATTERN_ID;

typedef enum
{
    /* 00 */ eGM_SERVICE_TEST_PATTERN_OFF,
    /* 01 */ eGM_SERVICE_TEST_PATTERN_GRID,
    /* 02 */ eGM_SERVICE_TEST_PATTERN_WHITE,
    /* 03 */ eGM_SERVICE_TEST_PATTERN_BLACK,
    /* 04 */ eGM_SERVICE_TEST_PATTERN_CHECKBOARD,
    /* 05 */ eGM_SERVICE_TEST_PATTERN_COLORBAR,
    /* 06 */ eGM_SERVICE_TEST_PATTERN_RED,
    /* 07 */ eGM_SERVICE_TEST_PATTERN_GREEN,
    /* 08 */ eGM_SERVICE_TEST_PATTERN_BLUE,
    /* 09 */ eGM_SERVICE_TEST_PATTERN_YELLOW,
    /* 10 */ eGM_SERVICE_TEST_PATTERN_MAGENTA,
    /* 11 */ eGM_SERVICE_TEST_PATTERN_CYAN,
    /* 12 */ eGM_SERVICE_TEST_PATTERN_BORESIGHT,
    /* 13 */ eGM_SERVICE_TEST_PATTERN_FULL_SCREEN,
    /* 14 */ eGM_SERVICE_TEST_PATTERN_13POINTS,

    eGM_SERVICE_TEST_PATTERN_NUMBER,
}eGM_SERVICE_TEST_PATTERN_ID;

typedef enum
{
    /* 00 */ eGM_TWIST_TEST_PATTERN_OFF,
    /* 01 */ eGM_TWIST_TEST_PATTERN_GRID,
    /* 02 */ eGM_TWIST_TEST_PATTERN_WHITE,
    /* 03 */ eGM_TWIST_TEST_PATTERN_BLACK,
    /* 04 */ eGM_TWIST_TEST_PATTERN_CHECKERBOARD,
    /* 05 */ eGM_TWIST_TEST_PATTERN_COLORBAR,
    /* 06 */ eGM_TWIST_TEST_PATTERN_GRAY,

    eGM_TWIST_TEST_PATTERN_NUMBER,
}eGM_TWIST_TEST_PATTERN_ID;


typedef enum
{
    /* 00 */ eGM_PICTURE_SETTINGS_PRESENTATION,
    /* 01 */ eGM_PICTURE_SETTINGS_HDR,
    /* 02 */ eGM_PICTURE_SETTINGS_HLG,
    /* 03 */ eGM_PICTURE_SETTINGS_VIDEO,
    /* 04 */ eGM_PICTURE_SETTINGS_SRGB,
    /* 05 */ eGM_PICTURE_SETTINGS_BRIGHT,
    /* 06 */ eGM_PICTURE_SETTINGS_DICOMSIM,
    /* 07 */ eGM_PICTURE_SETTINGS_BLENDING,
    /* 08 */ eGM_PICTURE_SETTINGS_3D,
    /* 09 */ eGM_PICTURE_SETTINGS_2D_HIGH_SPEED,
    /* 10 */ eGM_PICTURE_SETTINGS_USER,

    eGM_PICTURE_SETTINGS_NUMBER,
}eGM_PICTURE_SETTINGS_ID;

typedef enum
{
    /* 00 */ eGM_PRE_USER_PRESENTATION,
    /* 01 */ eGM_PRE_USER_BRIGHT,
    /* 02 */ eGM_PRE_USER_VIDEO,
    /* 03 */ eGM_PRE_USER_HDR,
    /* 04 */ eGM_PRE_USER_SRGB,
    /* 05 */ eGM_PRE_USER_DICOMSIM,
    /* 06 */ eGM_PRE_USER_BLENDING,
    /* 07 */ eGM_PRE_USER_3D,
    /* 08 */ eGM_PRE_USER_2D_HIGH_SPEED,

    eGM_PRE_USERS_NUMBER,
}eGM_PRE_USER_ID;				//G100_Doulas_0066


typedef enum
{
    /* 00 */ eGM_LANGUAGE_ENGLISH,
    /* 01 */ eGM_LANGUAGE_CZECH,
    /* 02 */ eGM_LANGUAGE_DANISH,
    /* 03 */ eGM_LANGUAGE_DUTCH,
    /* 04 */ eGM_LANGUAGE_FINNISH,
    /* 05 */ eGM_LANGUAGE_FRENCH,
    /* 06 */ eGM_LANGUAGE_GERMAN,
    /* 07 */ eGM_LANGUAGE_GREEK,
    /* 08 */ eGM_LANGUAGE_HUNGARIAN,
    /* 09 */ eGM_LANGUAGE_INDONESIAN,
    /* 10 */ eGM_LANGUAGE_ITALIAN,
    /* 11 */ eGM_LANGUAGE_JAPANESE,
    /* 12 */ eGM_LANGUAGE_KOREAN,
    /* 13 */ eGM_LANGUAGE_NORWEGIAN,
    /* 14 */ eGM_LANGUAGE_POLISH,
    /* 15 */ eGM_LANGUAGE_PORTUGUESE,
    /* 16 */ eGM_LANGUAGE_ROMANIAN,
    /* 17 */ eGM_LANGUAGE_RUSSIAN,
    /* 18 */ eGM_LANGUAGE_S_CHINESE,
    /* 19 */ eGM_LANGUAGE_SPANISH,
    /* 20 */ eGM_LANGUAGE_SWEDISH,
    /* 21 */ eGM_LANGUAGE_THAI,
    /* 22 */ eGM_LANGUAGE_T_CHINESE,
    /* 23 */ eGM_LANGUAGE_TURKISH,
    /* 24 */ eGM_LANGUAGE_VIETNAMESE,

    eGM_LANGUAGE_NUMBER,
}eGM_LANGUAGE_ID;

typedef enum
{
    /* 00 */ eGM_SCALING_MODE_4_3,
    /* 01 */ eGM_SCALING_MODE_16_9,
    /* 02 */ eGM_SCALING_MODE_21_9,
    /* 03 */ eGM_SCALING_MODE_LETTER_BOX,
    /* 04 */ eGM_SCALING_MODE_AUTO,
    /* 05 */ eGM_SCALING_MODE_NATIVE,

    eGM_SCALING_MODE_NUMBER,
}eGM_SCALING_MODE_ID;

typedef enum
{
    /* 00 */ eGM_HOTKEY_SETTINGS_HDMI1,
    /* 01 */ eGM_HOTKEY_SETTINGS_HDMI2,
    /* 02 */ eGM_HOTKEY_SETTINGS_COLOR_MATCHING,
    /* 03 */ eGM_HOTKEY_SETTINGS_COLOR_TEMPERATURE,
    /* 04 */ eGM_HOTKEY_SETTINGS_PROJECTION_ORIENTATION,
    /* 05 */ eGM_HOTKEY_SETTINGS_LIGHT_SOURCE_MODE,
    /* 06 */ eGM_HOTKEY_SETTINGS_FREEZE_SCREEN,
    /* 07 */ eGM_HOTKEY_SETTINGS_NETWORK_SETUP,
    /* 08 */ eGM_HOTKEY_SETTINGS_RESET_SELECTIVE,

    eGM_HOTKEY_SETTINGS_NUMBER,
}eGM_HOTKEY_SETTINGS_ID;

typedef enum
{
    /* 00 */ eGM_SCREEN_MODE_OFF,
    /* 01 */ eGM_SCREEN_MODE_PIP,
    /* 02 */ eGM_SCREEN_MODE_PBP,

    eGM_SCREEN_MODE_NUMBER,
}eGM_SCREEN_MODE_ID;


typedef enum
{
    /* 00 */ eGM_MENU_LOCATION_TOP_LEFT,
    /* 01 */ eGM_MENU_LOCATION_TOP_RIGHT,
    /* 02 */ eGM_MENU_LOCATION_CENTER,
    /* 03 */ eGM_MENU_LOCATION_BOTTOM_LEFT,
    /* 04 */ eGM_MENU_LOCATION_BOTTOM_RIGHT,

    eGM_MENU_LOCATION_NUMBER,
}eGM_MENU_LOCATION_ID;

typedef enum
{
    /* 00 */ eGM_OVERSCAN_OFF,
    /* 01 */ eGM_OVERSCAN_ZOOM,
    /* 02 */ eGM_OVERSCAN_CROP,

    eGM_OVERSCAN_NUMBER,
}eGM_OVERSCAN_ID;

typedef enum
{
    /* 00 */ eGM_COLOR_SPACE_AUTO,
    /* 01 */ eGM_COLOR_SPACE_RGB_FULL,
    /* 02 */ eGM_COLOR_SPACE_RGB_LIMITED,
    /* 03 */ eGM_COLOR_SPACE_YUV_REC709,
    /* 04 */ eGM_COLOR_SPACE_YUV_REC601,

    eGM_COLOR_SPACE_NUMBER,
}eGM_COLOR_SPACE_ID;

typedef enum
{
    /* 00 */ eGM_3D_MODE_OFF,
    /* 01 */ eGM_3D_MODE_AUTO,
    /* 02 */ eGM_3D_MODE_ACTIVE_3D,
    /* 03 */ eGM_3D_MODE_PASSIVE_3D,

    eGM_3D_MODE_NUMBER,
}eGM_3D_MODE_ID;		//A70Gen2_Doulas_0054 Modify

typedef enum
{
    /* 00 */ eGM_3D_FORMAT_AUTO,
    /* 01 */ eGM_3D_FORMAT_FRAME_PACKING,
    /* 02 */ eGM_3D_FORMAT_SIDE_BY_SIDE,
    /* 03 */ eGM_3D_FORMAT_TOP_AND_BOTTOM,
    /* 04 */ eGM_3D_FORMAT_FRAME_SEQUENTIAL,
    /* 05 */ eGM_3D_FORMAT_DUALPIPE,
    /* 06 */ eGM_3D_FORMAT_4K3D,
    /* 07 */ eGM_3D_FORMAT_4K3D_DUALPIPE,
    /* 08 */ eGM_3D_FORMAT_OFF,

    eGM_3D_FORMAT_NUMBER,
}eGM_3D_FORMAT_ID;

typedef enum
{
    /* 00 */ eGM_3D_TECH_DLP_LINK,
    /* 01 */ eGM_3D_TECH_3D_SYNC,

    eGM_3D_TECH_NUMBER,
}eGM_3D_TECH_ID;

typedef enum
{
    /* 00 */ eGM_3D_2D_3D,
    /* 01 */ eGM_3D_2D_2D_LEFT,
    /* 02 */ eGM_3D_2D_2D_RIGHT,

    eGM_3D_2D_NUMBER,
}eGM_3D_2D_ID;

typedef enum
{
    /* 00 */ eGM_3D_SYNC_OUT_TO_EMITTER,
    /* 01 */ eGM_3D_SYNC_OUT_TO_NEXT_PROJECTOR,

    eGM_3D_SYNC_OUT_NUMBER,
}eGM_3D_SYNC_OUT_ID;

typedef enum
{
    /* 00 */ eGM_3D_LR_REFERENCE_1ST_FRAME,
    /* 01 */ eGM_3D_LR_REFERENCE_FIELD_GPIO,

    eGM_3D_LR_REFERENCE_NUMBER,
}eGM_3D_LR_REFERENCE_ID;

typedef enum
{
    /* 00 */ eGM_3D_SYNC_TYPE_AUTO,
    /* 01 */ eGM_3D_SYNC_TYPE_INTERNAL,
    /* 02 */ eGM_3D_SYNC_TYPE_EXTERNAL,

    eGM_3D_SYNC_TYPE_NUMBER,
}eGM_3D_SYNC_TYPE_ID;

typedef enum
{
    /* 00 */ eGM_CONTRAST_ENHANCEMENT_OFF,
    /* 01 */ eGM_CONTRAST_ENHANCEMENT_CE1,
    /* 02 */ eGM_CONTRAST_ENHANCEMENT_CE2,

    eGM_CONTRAST_ENHANCEMENT_NUMBER,
}eGM_CONTRAST_ENHANCEMENT_ID;

typedef enum   //G100_Simon_0022
{
    /* 00 */ eGM_GAMMA_FILM,
    /* 01 */ eGM_GAMMA_GRAPHICS,
    /* 02 */ eGM_GAMMA_1_8,
    /* 03 */ eGM_GAMMA_2_0,
    /* 04 */ eGM_GAMMA_2_2,
    /* 05 */ eGM_GAMMA_2_4,
    /* 06 */ eGM_GAMMA_2_6,
    /* 07 */ eGM_GAMMA_VIVID,
    /* 08 */ eGM_GAMMA_3D,
    /* 09 */ eGM_GAMMA_BLACKBOARD,
    /* 10 */ eGM_GAMMA_DICOM,
    /* 11 */ eGM_GAMMA_HDR,

    eGM_GAMMA_NUMBER,
}eGM_GAMMA_ID;

typedef enum
{
    /* 00 */ eGM_EDGE_ENHANCEMENT_OFF,
    /* 01 */ eGM_EDGE_ENHANCEMENT_NORMAL,
    /* 02 */ eGM_EDGE_ENHANCEMENT_MAXIMUM,

    eGM_EDGE_ENHANCEMENT_NUMBER,
}eGM_EDGE_ENHANCEMENT_ID;

typedef enum
{
    /* 00 */ eGM_COLOR_WHEEL_SPEED_2X,
    /* 01 */ eGM_COLOR_WHEEL_SPEED_3X,

    eGM_COLOR_WHEEL_SPEED_NUMBER,
}eGM_COLOR_WHEEL_SPEED_ID;

typedef enum
{
    /* 00 */ eGM_LENS_TYPE_NA,
    /* 01 */ eGM_LENS_TYPE_A20,
    /* 02 */ eGM_LENS_TYPE_A21,
    /* 03 */ eGM_LENS_TYPE_A22,
    /* 04 */ eGM_LENS_TYPE_A23,
    /* 05 */ eGM_LENS_TYPE_A24,
    /* 05 */ eGM_LENS_TYPE_A25,
    /* 06 */ eGM_LENS_TYPE_A26,

    eGM_LENS_TYPE_NUMBER,
}eGM_LENS_TYPE_ID;


typedef enum
{
    /* 00 */ eGM_CEILING_MOUNT_OFF,
    /* 01 */ eGM_CEILING_MOUNT_ON,
    /* 02 */ eGM_CEILING_MOUNT_AUTO,

    eGM_CEILING_MOUNT_NUMBER,
}eGM_CEILING_MOUNT_ID;


typedef enum
{
    /* 00 */ eGM_REAR_PROJECTION_FRONT,
    /* 01 */ eGM_REAR_PROJECTION_REAR,

    eGM_REAR_PROJECTION_NUMBER,
}eGM_REAR_PROJECTION_ID;

typedef enum
{
    /* 00 */ eGM_SPLASH_STARTUP_FACTORY_LOGO,
    /* 01 */ eGM_SPLASH_STARTUP_BLUE,
    /* 02 */ eGM_SPLASH_STARTUP_BLACK,
    /* 03 */ eGM_SPLASH_STARTUP_WHITE,

    eGM_SPLASH_STARTUP_NUMBER,
}eGM_SPLASH_STARTUP_ID;


typedef enum
{
    /* 00 */ eGM_STANDBY_MODE_ECO,
    /* 01 */ eGM_STANDBY_MODE_ACTIVE,
    /* 02 */ eGM_STANDBY_MODE_COMMUNICATION,

    eGM_STANDBY_MODE_NUMBER,
}eGM_STANDBY_MODE_ID;


typedef enum
{
    /* 00 */ eGM_AUTO_SHUTDOWN_OFF,
    /* 01 */ eGM_AUTO_SHUTDOWN_5MIN,
    /* 02 */ eGM_AUTO_SHUTDOWN_10MIN,
    /* 03 */ eGM_AUTO_SHUTDOWN_15MIN,
    /* 04 */ eGM_AUTO_SHUTDOWN_20MIN,
    /* 05 */ eGM_AUTO_SHUTDOWN_25MIN,
    /* 06 */ eGM_AUTO_SHUTDOWN_30MIN,

    eGM_AUTO_SHUTDOWN_NUMBER,
}eGM_AUTO_SHUTDOWN_ID;

typedef enum
{
    /* 00 */ eGM_SLEEP_TIMER_OFF,
    /* 01 */ eGM_SLEEP_TIMER_2HOURS,
    /* 02 */ eGM_SLEEP_TIMER_4HOURS,
    /* 03 */ eGM_SLEEP_TIMER_6HOURS,

    eGM_SLEEP_TIMER_NUMBER,
}eGM_SLEEP_TIMER_ID;


typedef enum
{
    ///* 00 */ eGM_SERIAL_PORT_BAUD_RATE_1200,
    ///* 01 */ eGM_SERIAL_PORT_BAUD_RATE_2400,
    ///* 02 */ eGM_SERIAL_PORT_BAUD_RATE_4800,
    /* 03 */ eGM_SERIAL_PORT_BAUD_RATE_9600,
    /* 04 */ eGM_SERIAL_PORT_BAUD_RATE_19200,
    /* 05 */ eGM_SERIAL_PORT_BAUD_RATE_38400,
    /* 06 */ eGM_SERIAL_PORT_BAUD_RATE_57600,
    /* 07 */ eGM_SERIAL_PORT_BAUD_RATE_115200,

    eGM_SERIAL_PORT_BAUD_RATE_NUMBER,
}eGM_SERIAL_PORT_BAUD_RATE_ID;


typedef enum
{
    /* 00 */ eGM_SERIAL_PORT_PATH_RS232,
    /* 01 */ eGM_SERIAL_PORT_PATH_HDBASET,
    /* 02 */ eGM_SERIAL_PORT_PATH_SLOT1,
    /* 03 */ eGM_SERIAL_PORT_PATH_SLOT2,

    eGM_SERIAL_PORT_PATH_NUMBER,
}eGM_SERIAL_PORT_PATH_ID;


typedef enum
{
    /* 00 */ eGM_SERIAL_PORT_PATH_FROM_SLOT1,
    /* 01 */ eGM_SERIAL_PORT_PATH_FROM_SLOT2,

    eGM_SERIAL_PORT_PATH_FROM_NUMBER,
}eGM_SERIAL_PORT_PATH_FROM_ID;

typedef enum
{
    /* 00 */ eGM_KEYPAD_BACKLIGHT_5_SECS,
    /* 01 */ eGM_KEYPAD_BACKLIGHT_10_SECS,
    /* 00 */ eGM_KEYPAD_BACKLIGHT_20_SECS,
    /* 03 */ eGM_KEYPAD_BACKLIGHT_30_SECS,
    /* 04 */ eGM_KEYPAD_BACKLIGHT_ALWAYS_ON,
    /* 05 */ eGM_KEYPAD_BACKLIGHT_ALWAYS_OFF,

    eGM_KEYPAD_BACKLIGHT_NUMBER,
}eGM_KEYPAD_BACKLIGHT_ID;


typedef enum
{
    /* 00 */ eGM_STATUS_LED_ALWAYS_ON,
    /* 01 */ eGM_STATUS_LED_ALWAYS_OFF,
    /* 02 */ eGM_STATUS_LED_WARNING_ERRORS_ONLY,

    eGM_STATUS_LED_NUMBER,
}eGM_STATUS_LED_ID;


typedef enum
{
    /* 00 */ eGM_WALL_COLOR_OFF,
    /* 01 */ eGM_WALL_COLOR_BLACKBOARD,
    /* 02 */ eGM_WALL_COLOR_LIGHT_YELLOW,
    /* 03 */ eGM_WALL_COLOR_LIGHT_GREEN,
    /* 04 */ eGM_WALL_COLOR_LIGHT_BLUE,
    /* 05 */ eGM_WALL_COLOR_PINK,
    /* 06 */ eGM_WALL_COLOR_GRAY,

    eGM_WALL_COLOR_NUMBER,
}eGM_WALL_COLOR_ID;


typedef enum
{
    /* 00 */ eGM_POWER_MODE_CONSTANT_POWER,
    /* 01 */ eGM_POWER_MODE_ECO,
    /* 02 */ eGM_POWER_MODE_CUSTOM_MODE,

    eGM_POWER_MODE_NUMBER,
}eGM_POWER_MODE_ID;


typedef enum
{
    /* 00 */ eGM_PIP_SIZE_SMALL,
    /* 01 */ eGM_PIP_SIZE_MEDIUM,
    /* 02 */ eGM_PIP_SIZE_LARGE,

    eGM_PIP_SIZE_NUMBER,
}eGM_PIP_SIZE_ID;


typedef enum
{
    /* 00 */ eGM_MAIN_LAYOUT_PBP_MAIN_LEFT,
    /* 01 */ eGM_MAIN_LAYOUT_PBP_MAIN_TOP,
    /* 02 */ eGM_MAIN_LAYOUT_PBP_MAIN_RIGHT,
    /* 03 */ eGM_MAIN_LAYOUT_PBP_MAIN_BOTTOM,
    /* 04 */ eGM_MAIN_LAYOUT_PIP_BOTTOM_RIGHT,
    /* 05 */ eGM_MAIN_LAYOUT_PIP_BOTTOM_LEFT,
    /* 06 */ eGM_MAIN_LAYOUT_PIP_TOP_LEFT,
    /* 07 */ eGM_MAIN_LAYOUT_PIP_TOP_RIGHT,

    eGM_MAIN_LAYOUT_NUMBER,
}eGM_MAIN_LAYOUT_ID;


typedef enum
{
    /* 00 */ eGM_INPUT_KEY_CHANGE_SOURCE,
    /* 01 */ eGM_INPUT_KEY_LIST_ALL_SOURCE,
    /* 02 */ eGM_INPUT_KEY_AUTO_SOURCE,

    eGM_INPUT_KEY_NUMBER,
}eGM_INPUT_KEY_ID;

typedef enum
{
    /* 00 */ eGM_COLOR_TEMPERATURE_WARM,
    /* 01 */ eGM_COLOR_TEMPERATURE_STANDARD,
    /* 02 */ eGM_COLOR_TEMPERATURE_COOL,
    /* 03 */ eGM_COLOR_TEMPERATURE_COLD,

    eGM_COLOR_TEMPERATURE_NUMBER,
}eGM_COLOR_TEMPERATURE_ID;


typedef enum
{
    /* 00 */ eGM_BLENDING_GAMMA_1_8,
    /* 01 */ eGM_BLENDING_GAMMA_1_9,
    /* 02 */ eGM_BLENDING_GAMMA_2_0,
    /* 03 */ eGM_BLENDING_GAMMA_2_1,
    /* 04 */ eGM_BLENDING_GAMMA_2_2,
    /* 05 */ eGM_BLENDING_GAMMA_2_3,
    /* 06 */ eGM_BLENDING_GAMMA_2_4,

    eGM_BLENDING_GAMMA_NUMBER,
}eGM_BLENDING_GAMMA_ID;

typedef enum
{
    /* 00 */ eGM_BACKGROUND_COLOR_BLUE,
    /* 01 */ eGM_BACKGROUND_COLOR_BLACK,
    /* 02 */ eGM_BACKGROUND_COLOR_RED,
    /* 03 */ eGM_BACKGROUND_COLOR_GREEN,
    /* 04 */ eGM_BACKGROUND_COLOR_GRAY,
    /* 05 */ eGM_BACKGROUND_COLOR_WHITE,
    /* 06 */ eGM_BACKGROUND_COLOR_LOGO,

    eGM_BACKGROUND_COLOR_NUMBER,
}eGM_BACKGROUND_COLOR_ID;

typedef enum
{
    /* 00 */ eGM_BACKUPINPUT_AUTOSWITCH_SEAMLESS,
    /* 01 */ eGM_BACKUPINPUT_AUTOSWITCH_BACKUP,
    /* 02 */ eGM_BACKUPINPUT_AUTOSWITCH_SIGNLE,

    eGM_BACKUPINPUT_AUTOSWITCH_NUMBER,
}eGM_BACKUPINPUT_AUTOSWITCH_ID;


typedef enum
{
    /* 00 */ eGM_EDID_TYPE_V14,
    /* 01 */ eGM_EDID_TYPE_V20,
    /* 02 */ eGM_EDID_TYPE_CUSTOMIZED,

    eGM_EDID_TYPE_NUMBER
}eGM_EDID_TYPE_ID;

typedef enum
{
    /* 00 */ eGM_LAN_PATH_SWITCH_RJ45,
    /* 01 */ eGM_LAN_PATH_SWITCH_HDBASET,

    eGM_LAN_PATH_SWITCH_NUMBER
}eGM_LAN_PATH_SWITCH_ID;


typedef enum
{
    /* 00 */ eGM_OSD_TIMEOUT_OFF,
    /* 01 */ eGM_OSD_TIMEOUT_5_SEC,
    /* 02 */ eGM_OSD_TIMEOUT_10_SEC,
    /* 03 */ eGM_OSD_TIMEOUT_20_SEC,
    /* 04 */ eGM_OSD_TIMEOUT_30_SEC,
    /* 05 */ eGM_OSD_TIMEOUT_60_SEC,

    eGM_OSD_TIMEOUT_NUMBER
}eGM_OSD_TIMEOUT_ID;


typedef enum
{
    /* 00 */ eGM_BACKGROUND_BLUE,
    /* 01 */ eGM_BACKGROUND_BLACK,
    /* 02 */ eGM_BACKGROUND_WHITE,
    /* 03 */ eGM_BACKGROUND_LOGO,

    eGM_BACKGROUND_NUMBER
}eGM_BACKGROUND_ID;


typedef enum
{
    /* 00 */ eGM_LIGHT_SOURCE_MODE_NORMAL,
    /* 01 */ eGM_LIGHT_SOURCE_MODE_ECO_MODE,
    /* 02 */ eGM_LIGHT_SOURCE_MODE_CUSTOM_MODE,

    eGM_LIGHT_SOURCE_MODE_NUMBER
}eGM_LIGHT_SOURCE_MODE_ID;

typedef enum
{
    /* 00 */ eGM_LENS_MEMORY_SAVE_1,
    /* 01 */ eGM_LENS_MEMORY_SAVE_2,
    /* 02 */ eGM_LENS_MEMORY_SAVE_3,
    /* 03 */ eGM_LENS_MEMORY_SAVE_4,
    /* 04 */ eGM_LENS_MEMORY_SAVE_5,

    eGM_LENS_MEMORY_SAVE_NUMBER
}eGM_LENS_MEMORY_SAVE_ID;


typedef enum
{
    /* 00 */ eGM_LENS_MEMORY_APPLY_1,
    /* 01 */ eGM_LENS_MEMORY_APPLY_2,
    /* 02 */ eGM_LENS_MEMORY_APPLY_3,
    /* 03 */ eGM_LENS_MEMORY_APPLY_4,
    /* 04 */ eGM_LENS_MEMORY_APPLY_5,

    eGM_LENS_MEMORY_APPLY_NUMBER
}eGM_LENS_MEMORY_APPLY_ID;

typedef enum
{
    /* 00 */ eGM_LOGO_PATTERN_FACTORY_LOGO,
    /* 01 */ eGM_LOGO_PATTERN_BLUE,
    /* 02 */ eGM_LOGO_PATTERN_BLACK,
    /* 03 */ eGM_LOGO_PATTERN_WHITE,

    eGM_LOGO_PATTERN_NUMBER
}eGM_LOGO_PATTERN;


typedef enum
{
    /* 00 */ eGM_LOW_LATENCY_MODE_NORMAL,
    /* 01 */ eGM_LOW_LATENCY_MODE_2D_ULTRA,
    /* 02 */ eGM_LOW_LATENCY_MODE_ULTRA,

    eGM_LOW_LATENCY_MODE_NUMBER
}eGM_LOW_LATENCY_MODE;

typedef enum
{
    /* 00 */ eGM_DETAIL_MAXIMUM,
    /* 01 */ eGM_DETAIL_HIGH,
    /* 02 */ eGM_DETAIL_NORMAL,
    /* 03 */ eGM_DETAIL_LOW,
    /* 04 */ eGM_DETAIL_MINIMUM,

    eGM_DETAIL_NUMBER
}eGM_DETAIL_ID;

typedef enum
{
    /* 00 */ eGM_HDR_LEVEL_1,
    /* 01 */ eGM_HDR_LEVEL_2,
    /* 02 */ eGM_HDR_LEVEL_3,
    /* 03 */ eGM_HDR_LEVEL_4,

    eGM_HDR_LEVEL_NUMBER
}eGM_HDR_LEVEL;

typedef enum
{
    /* 00 */ eGM_CLOCK_MODE_USE_NTP_SERVER,
    /* 01 */ eGM_CLOCK_MODE_MANUAL,

    eGM_CLOCK_MODE_NUMBER
}eGM_CLOCK_MODE;

typedef enum
{
 /* 00 */   eGM_UPDATE_INTERVAL_HOURLY,
 /* 01 */   eGM_UPDATE_INTERVAL_DAILY,

    eGM_UPDATE_INTERVAL_NUMBER
}eGM_UPDATE_INTERVAL;


typedef enum
{
 /* 00 */    eGM_HDMI_OUT_HDMI1,
 /* 01 */    eGM_HDMI_OUT_HDMI2,

    eGM_HDMI_OUT_MAX_NUMBER
} eGM_HDMI_OUT;


typedef enum
{
    /* 00 */ eGM_COPY_EVENT_TO_MONDAY,
    /* 01 */ eGM_COPY_EVENT_TO_TUESDAY,
    /* 02 */ eGM_COPY_EVENT_TO_WEDNESDAY,
    /* 03 */ eGM_COPY_EVENT_TO_THURSDAY,
    /* 04 */ eGM_COPY_EVENT_TO_FRIDAY,
    /* 05 */ eGM_COPY_EVENT_TO_SATURDAY,
    /* 06 */ eGM_COPY_EVENT_TO_SUNDAY,

    eGM_COPY_EVENT_TO_NUMBER
} eGM_COPY_EVENT_TO;


typedef enum
{
    /* 00 */ eGM_GEO_MEMORY_SAVE_1,
    /* 01 */ eGM_GEO_MEMORY_SAVE_2,
    /* 02 */ eGM_GEO_MEMORY_SAVE_3,
    /* 03 */ eGM_GEO_MEMORY_SAVE_4,
    /* 04 */ eGM_GEO_MEMORY_SAVE_5,

    eGM_GEO_MEMORY_SAVE_NUMBER
}eGM_GEO_MEMORY_SAVE_ID;

typedef enum
{
    /* 00 */ eGM_GEO_MEMORY_APPLY_1,
    /* 01 */ eGM_GEO_MEMORY_APPLY_2,
    /* 02 */ eGM_GEO_MEMORY_APPLY_3,
    /* 03 */ eGM_GEO_MEMORY_APPLY_4,
    /* 04 */ eGM_GEO_MEMORY_APPLY_5,

    eGM_GEO_MEMORY_APPLY_NUMBER
}eGM_GEO_MEMORY_APPLY_ID;

typedef enum
{
    /* 00 */ eGM_SOURCE_KEY_CHANGE_SOURCE,
    /* 01 */ eGM_SOURCE_KEY_LIST_ALL_SOURCE,
    /* 02 */ eGM_SOURCE_KEY_AUTO_SOURCE,

    eGM_SOURCE_KEY_NUMBER
}eGM_SOURCE_KEY_ID;

typedef enum
{
    /* 00 */ eGM_CHANGE_LOGO_DEFAULT,
    /* 01 */ eGM_CHANGE_LOGO_NEUTRAL,
    /* 02 */ eGM_CHANGE_LOGO_USER,
    ///* 03 */ eGM_CHANGE_LOGO_CAPTURED,

    eGM_CHANGE_LOGO_NUMBER
}eGM_CHANGE_LOGO_ID;

/////////////////////////////////////////////////////////////////////////////////////
// Source ID
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_SourceID[] =
{
    /* 00 */ {(INT32)eGUI_SOURCE_ID_HDMI1,      (INT32)eCM_SOURCE_HDMI1,        (INT32)CLI_SOURCE_HDMI1,      (INT32)eGM_SOURCE_ID_HDMI1},
    /* 01 */ {(INT32)eGUI_SOURCE_ID_HDMI2,      (INT32)eCM_SOURCE_HDMI2,        (INT32)CLI_SOURCE_HDMI2,      (INT32)eGM_SOURCE_ID_HDMI2},
    /* 02 */ {(INT32)eGUI_SOURCE_ID_DP,         (INT32)eCM_SOURCE_DISPLAYPORT,  (INT32)CLI_SOURCE_DP,         (INT32)eGM_SOURCE_ID_DP},
    /* 03 */ {(INT32)GUI_VALUE_INVALID,         (INT32)eCM_SOURCE_3GSDI,        (INT32)CLI_VALUE_INVALID,     (INT32)eGM_SOURCE_ID_3GSDI},
    /* 04 */ {(INT32)eGUI_SOURCE_ID_12GSDI,     (INT32)eCM_SOURCE_12GSDI,       (INT32)CLI_SOURCE_12GSDI,     (INT32)eGM_SOURCE_ID_12GSDI},
    /* 05 */ {(INT32)eGUI_SOURCE_ID_HDBASET,    (INT32)eCM_SOURCE_HDBASET,      (INT32)CLI_SOURCE_HDBASET,    (INT32)eGM_SOURCE_ID_HDBASET},
};

/////////////////////////////////////////////////////////////////////////////////////
// BackupInput Source ID
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_BackupInput_SourceID[] = //HICC2_Doulas_0027
{
    /* 00 */ {(INT32)eGUI_BACKUPINPUT_SOURCE_ID_HDMI1,      (INT32)eCM_SOURCE_HDMI1,        (INT32)CLI_SOURCE_HDMI1,    (INT32)eGM_BACKUPINPUT_SOURCE_ID_HDMI1},
    /* 01 */ {(INT32)eGUI_BACKUPINPUT_SOURCE_ID_HDMI2,      (INT32)eCM_SOURCE_HDMI2,        (INT32)CLI_SOURCE_HDMI2,    (INT32)eGM_BACKUPINPUT_SOURCE_ID_HDMI2},
    /* 02 */ {(INT32)eGUI_BACKUPINPUT_SOURCE_ID_DP,         (INT32)eCM_SOURCE_DISPLAYPORT,  (INT32)CLI_SOURCE_DP,       (INT32)eGM_BACKUPINPUT_SOURCE_ID_DP},
    /* 03 */ {(INT32)GUI_VALUE_INVALID,                     (INT32)eCM_SOURCE_3GSDI,        (INT32)CLI_VALUE_INVALID,   (INT32)eGM_BACKUPINPUT_SOURCE_ID_3GSDI},
    /* 04 */ {(INT32)eGUI_BACKUPINPUT_SOURCE_ID_12GSDI,     (INT32)eCM_SOURCE_12GSDI,       (INT32)CLI_SOURCE_12GSDI,   (INT32)eGM_BACKUPINPUT_SOURCE_ID_12GSDI},
    /* 05 */ {(INT32)eGUI_BACKUPINPUT_SOURCE_ID_HDBASET,    (INT32)eCM_SOURCE_HDBASET,      (INT32)CLI_SOURCE_HDBASET,  (INT32)eGM_BACKUPINPUT_SOURCE_ID_HDBASET},
};

/////////////////////////////////////////////////////////////////////////////////////
// Test Pattern
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_TestPattern[] =
{
    /* 00 */ {(INT32)eGUI_TEST_PATTERN_OFF,                 (INT32)eCM_TEST_PATTERN_OFF,                (INT32)CLI_TEST_PATTERN_OFF,                (INT32)eGM_TEST_PATTERN_OFF},
    /* 01 */ {(INT32)eGUI_TEST_PATTERN_GREEN_GRID,          (INT32)eCM_TEST_PATTERN_GREEN_GRID,         (INT32)CLI_TEST_PATTERN_GREEN_GRID,         (INT32)eGM_TEST_PATTERN_GREEN_GRID},
    /* 02 */ {(INT32)eGUI_TEST_PATTERN_MAGENTA_GRID,        (INT32)eCM_TEST_PATTERN_MAGENTA_GRID,       (INT32)CLI_TEST_PATTERN_MAGENTA_GRID,       (INT32)eGM_TEST_PATTERN_MAGENTA_GRID},
    /* 03 */ {(INT32)eGUI_TEST_PATTERN_WHITE_GRID,          (INT32)eCM_TEST_PATTERN_WHITE_GRID,         (INT32)CLI_TEST_PATTERN_WHITE_GRID,         (INT32)eGM_TEST_PATTERN_WHITE_GRID},
    /* 04 */ {(INT32)eGUI_TEST_PATTERN_WHITE,               (INT32)eCM_TEST_PATTERN_WHITE,              (INT32)CLI_TEST_PATTERN_WHITE,              (INT32)eGM_TEST_PATTERN_WHITE},
    /* 05 */ {(INT32)eGUI_TEST_PATTERN_BLACK,               (INT32)eCM_TEST_PATTERN_BLACK,              (INT32)CLI_TEST_PATTERN_BLACK,              (INT32)eGM_TEST_PATTERN_BLACK},
    /* 06 */ {(INT32)eGUI_TEST_PATTERN_RED,                 (INT32)eCM_TEST_PATTERN_RED,                (INT32)CLI_TEST_PATTERN_RED,                (INT32)eGM_TEST_PATTERN_RED},
    /* 07 */ {(INT32)eGUI_TEST_PATTERN_GREEN,               (INT32)eCM_TEST_PATTERN_GREEN,              (INT32)CLI_TEST_PATTERN_GREEN,              (INT32)eGM_TEST_PATTERN_GREEN},
    /* 08 */ {(INT32)eGUI_TEST_PATTERN_BLUE,                (INT32)eCM_TEST_PATTERN_BLUE,               (INT32)CLI_TEST_PATTERN_BLUE,               (INT32)eGM_TEST_PATTERN_BLUE},
    /* 09 */ {(INT32)eGUI_TEST_PATTERN_YELLOW,              (INT32)eCM_TEST_PATTERN_YELLOW,             (INT32)CLI_TEST_PATTERN_YELLOW,             (INT32)eGM_TEST_PATTERN_YELLOW},
    /* 10 */ {(INT32)eGUI_TEST_PATTERN_MAGENTA,             (INT32)eCM_TEST_PATTERN_MAGENTA,            (INT32)CLI_TEST_PATTERN_MAGENTA,            (INT32)eGM_TEST_PATTERN_MAGENTA},
    /* 11 */ {(INT32)eGUI_TEST_PATTERN_CYAN,                (INT32)eCM_TEST_PATTERN_CYAN,               (INT32)CLI_TEST_PATTERN_CYAN,               (INT32)eGM_TEST_PATTERN_CYAN},
    /* 12 */ {(INT32)eGUI_TEST_PATTERN_CHECKERBOARD,        (INT32)eCM_TEST_PATTERN_CHECKERBOARD,       (INT32)CLI_TEST_PATTERN_CHECKERBOARD,       (INT32)eGM_TEST_PATTERN_CHECKERBOARD},
    /* 13 */ {(INT32)eGUI_TEST_PATTERN_COLORBAR,            (INT32)eCM_TEST_PATTERN_COLORBAR,           (INT32)CLI_TEST_PATTERN_COLORBAR,           (INT32)eGM_TEST_PATTERN_COLORBAR},
    /* 14 */ {(INT32)eGUI_TEST_PATTERN_4K_FULL_SCREEN,      (INT32)eCM_TEST_PATTERN_FULL_SCREEN,        (INT32)CLI_TEST_PATTERN_4K_FULL_SCREEN,     (INT32)eGM_TEST_PATTERN_4K_FULL_SCREEN},
};

/////////////////////////////////////////////////////////////////////////////////////
// Service Test Pattern
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_ServiceTestPattern[] =
{
    /* 00 */ {(INT32)eGUI_SERVICE_TEST_PATTERN_OFF,           	(INT32)eCM_TEST_PATTERN_OFF,            	(INT32)CLI_SERVICE_TEST_PATTERN_OFF,          (INT32)eGM_SERVICE_TEST_PATTERN_OFF},
    /* 01 */ {(INT32)eGUI_SERVICE_TEST_PATTERN_GRID,          	(INT32)eCM_TEST_PATTERN_GRID,     			(INT32)CLI_SERVICE_TEST_PATTERN_GRID,         (INT32)eGM_SERVICE_TEST_PATTERN_GRID},
    /* 02 */ {(INT32)eGUI_SERVICE_TEST_PATTERN_WHITE,        	(INT32)eCM_TEST_PATTERN_WHITE, 	 	 		(INT32)CLI_SERVICE_TEST_PATTERN_WHITE,        (INT32)eGM_SERVICE_TEST_PATTERN_WHITE},
    /* 03 */ {(INT32)eGUI_SERVICE_TEST_PATTERN_BLACK,          	(INT32)eCM_TEST_PATTERN_BLACK,     			(INT32)CLI_SERVICE_TEST_PATTERN_BLACK,        (INT32)eGM_SERVICE_TEST_PATTERN_BLACK},
    /* 04 */ {(INT32)eGUI_SERVICE_TEST_PATTERN_CHECKBOARD, 		(INT32)eCM_TEST_PATTERN_CHECKERBOARD,	  	(INT32)CLI_SERVICE_TEST_PATTERN_CHECKBOARD,   (INT32)eGM_SERVICE_TEST_PATTERN_CHECKBOARD},
    /* 05 */ {(INT32)eGUI_SERVICE_TEST_PATTERN_COLORBAR,     	(INT32)eCM_TEST_PATTERN_COLORBAR,         	(INT32)CLI_SERVICE_TEST_PATTERN_COLORBAR,     (INT32)eGM_SERVICE_TEST_PATTERN_COLORBAR},
    /* 06 */ {(INT32)eGUI_SERVICE_TEST_PATTERN_RED,             (INT32)eCM_TEST_PATTERN_RED,            	(INT32)CLI_SERVICE_TEST_PATTERN_RED,          (INT32)eGM_SERVICE_TEST_PATTERN_RED},
    /* 07 */ {(INT32)eGUI_SERVICE_TEST_PATTERN_GREEN,           (INT32)eCM_TEST_PATTERN_GREEN,          	(INT32)CLI_SERVICE_TEST_PATTERN_GREEN,        (INT32)eGM_SERVICE_TEST_PATTERN_GREEN},
    /* 08 */ {(INT32)eGUI_SERVICE_TEST_PATTERN_BLUE,            (INT32)eCM_TEST_PATTERN_BLUE,           	(INT32)CLI_SERVICE_TEST_PATTERN_BLUE,         (INT32)eGM_SERVICE_TEST_PATTERN_BLUE},
    /* 09 */ {(INT32)eGUI_SERVICE_TEST_PATTERN_YELLOW,      	(INT32)eCM_TEST_PATTERN_YELLOW,         	(INT32)CLI_SERVICE_TEST_PATTERN_YELLOW,       (INT32)eGM_SERVICE_TEST_PATTERN_YELLOW},
    /* 10 */ {(INT32)eGUI_SERVICE_TEST_PATTERN_MAGENTA,   		(INT32)eCM_TEST_PATTERN_MAGENTA,        	(INT32)CLI_SERVICE_TEST_PATTERN_MAGENTA,      (INT32)eGM_SERVICE_TEST_PATTERN_MAGENTA},
    /* 11 */ {(INT32)eGUI_SERVICE_TEST_PATTERN_CYAN,            (INT32)eCM_TEST_PATTERN_CYAN,           	(INT32)CLI_SERVICE_TEST_PATTERN_CYAN,         (INT32)eGM_SERVICE_TEST_PATTERN_CYAN},
    /* 12 */ {(INT32)eGUI_SERVICE_TEST_PATTERN_BORESIGHT, 	 	(INT32)eCM_TEST_PATTERN_BORESIGHT,	 	  	(INT32)CLI_SERVICE_TEST_PATTERN_BORESIGHT,    (INT32)eGM_SERVICE_TEST_PATTERN_BORESIGHT},
    /* 13 */ {(INT32)eGUI_SERVICE_TEST_PATTERN_FULL_SCREEN, 	(INT32)eCM_TEST_PATTERN_FULL_SCREEN,       	(INT32)CLI_SERVICE_TEST_PATTERN_FULL_SCREEN,  (INT32)eGM_SERVICE_TEST_PATTERN_FULL_SCREEN},
    /* 14 */ {(INT32)eGUI_SERVICE_TEST_PATTERN_13POINTS,   		(INT32)eCM_TEST_PATTERN_13POINTS, 	   		(INT32)CLI_SERVICE_TEST_PATTERN_13POINTS,     (INT32)eGM_SERVICE_TEST_PATTERN_13POINTS},
};

/////////////////////////////////////////////////////////////////////////////////////
// Twist Test Pattern
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_TwistTestPattern[] =
{
    /* 00 */ {(INT32)eGUI_TWIST_TEST_PATTERN_OFF,           (INT32)eCM_TEST_PATTERN_OFF,            (INT32)CLI_TWIST_TEST_PATTERN_OFF,             (INT32)eGM_TWIST_TEST_PATTERN_OFF},
    /* 01 */ {(INT32)eGUI_TWIST_TEST_PATTERN_GRID,          (INT32)eCM_TEST_PATTERN_GRID,           (INT32)CLI_TWIST_TEST_PATTERN_GRID,            (INT32)eGM_TWIST_TEST_PATTERN_GRID},
    /* 02 */ {(INT32)eGUI_TWIST_TEST_PATTERN_WHITE,         (INT32)eCM_TEST_PATTERN_WHITE,          (INT32)CLI_TWIST_TEST_PATTERN_WHITE,           (INT32)eGM_TWIST_TEST_PATTERN_WHITE},
    /* 03 */ {(INT32)eGUI_TWIST_TEST_PATTERN_BLACK,         (INT32)eCM_TEST_PATTERN_BLACK,          (INT32)CLI_TWIST_TEST_PATTERN_BLACK,           (INT32)eGM_TWIST_TEST_PATTERN_BLACK},
    /* 04 */ {(INT32)eGUI_TWIST_TEST_PATTERN_CHECKERBOARD,  (INT32)eCM_TEST_PATTERN_CHECKERBOARD,   (INT32)CLI_TWIST_TEST_PATTERN_CHECKERBOARD,    (INT32)eGM_TWIST_TEST_PATTERN_CHECKERBOARD},
    /* 05 */ {(INT32)eGUI_TWIST_TEST_PATTERN_COLORBAR,      (INT32)eCM_TEST_PATTERN_COLORBAR,       (INT32)CLI_TWIST_TEST_PATTERN_COLORBAR,        (INT32)eGM_TWIST_TEST_PATTERN_COLORBAR},
    /* 06 */ {(INT32)eGUI_TWIST_TEST_PATTERN_GRAY,          (INT32)eCM_TEST_PATTERN_GRAY,           (INT32)CLI_TWIST_TEST_PATTERN_GRAY,            (INT32)eGM_TWIST_TEST_PATTERN_GRAY},
};

/////////////////////////////////////////////////////////////////////////////////////
// Picture Settings
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_PictureSettings[] =
{
	/* 00 */ {(INT32)eGUI_PICTURE_SETTINGS_PRESENTATION,    (INT32)eCM_PICTURE_SETTINGS_PRESENTATION,   (INT32)CLI_PICTURE_SETTINGS_PRESENTATION,   (INT32)eGM_PICTURE_SETTINGS_PRESENTATION},
    /* 01 */ {(INT32)eGUI_PICTURE_SETTINGS_HDR,             (INT32)eCM_PICTURE_SETTINGS_HDR,            (INT32)CLI_PICTURE_SETTINGS_HDR,            (INT32)eGM_PICTURE_SETTINGS_HDR},
    /* 02 */ {(INT32)eGUI_PICTURE_SETTINGS_HLG,             (INT32)eCM_PICTURE_SETTINGS_HLG,            (INT32)CLI_PICTURE_SETTINGS_HLG,            (INT32)eGM_PICTURE_SETTINGS_HLG},
    /* 03 */ {(INT32)eGUI_PICTURE_SETTINGS_CINEMA,          (INT32)eCM_PICTURE_SETTINGS_VIDEO,          (INT32)CLI_PICTURE_SETTINGS_CINEMA,         (INT32)eGM_PICTURE_SETTINGS_VIDEO},
	/* 04 */ {(INT32)eGUI_PICTURE_SETTINGS_SRGB,            (INT32)eCM_PICTURE_SETTINGS_SRGB,           (INT32)CLI_PICTURE_SETTINGS_SRGB,           (INT32)eGM_PICTURE_SETTINGS_SRGB},
    /* 05 */ {(INT32)eGUI_PICTURE_SETTINGS_BRIGHT,          (INT32)eCM_PICTURE_SETTINGS_BRIGHT,         (INT32)CLI_PICTURE_SETTINGS_BRIGHT,         (INT32)eGM_PICTURE_SETTINGS_BRIGHT},
    /* 06 */ {(INT32)eGUI_PICTURE_SETTINGS_DICOMSIM,        (INT32)eCM_PICTURE_SETTINGS_DICOMSIM,       (INT32)CLI_PICTURE_SETTINGS_DICOMSIM,       (INT32)eGM_PICTURE_SETTINGS_DICOMSIM},
	/* 07 */ {(INT32)eGUI_PICTURE_SETTINGS_BLENDING,        (INT32)eCM_PICTURE_SETTINGS_BLENDING,       (INT32)CLI_PICTURE_SETTINGS_BLENDING,       (INT32)eGM_PICTURE_SETTINGS_BLENDING},
	/* 08 */ {(INT32)eGUI_PICTURE_SETTINGS_3D,              (INT32)eCM_PICTURE_SETTINGS_3D,             (INT32)CLI_PICTURE_SETTINGS_3D,             (INT32)eGM_PICTURE_SETTINGS_3D},
	/* 09 */ {(INT32)eGUI_PICTURE_SETTINGS_2D_HIGH_SPEED,   (INT32)eCM_PICTURE_SETTINGS_2DHIGHSPEED,    (INT32)CLI_PICTURE_SETTINGS_2D_HIGH_SPEED,  (INT32)eGM_PICTURE_SETTINGS_2D_HIGH_SPEED},	//G100_Clare_0001
	/* 10 */ {(INT32)eGUI_PICTURE_SETTINGS_USER,            (INT32)eCM_PICTURE_SETTINGS_USER,           (INT32)CLI_PICTURE_SETTINGS_USER,           (INT32)eGM_PICTURE_SETTINGS_USER},			//G100_Steven_0016 //G100_Clare_0002
};

/////////////////////////////////////////////////////////////////////////////////////
// Apply User Settings
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_ApplyUserSettings[] =                //G100_Doulas_0066
{
	/* 00 */ {(INT32)eGUI_PRE_USER_PRESENTATION,    (INT32)eCM_PICTURE_SETTINGS_PRESENTATION,   (INT32)CLI_PRE_USER_PRESENTATION,   (INT32)eGM_PRE_USER_PRESENTATION},
	/* 01 */ {(INT32)eGUI_PRE_USER_BRIGHT,          (INT32)eCM_PICTURE_SETTINGS_BRIGHT,         (INT32)CLI_PRE_USER_BRIGHT,         (INT32)eGM_PRE_USER_BRIGHT},
	/* 02 */ {(INT32)eGUI_PRE_USER_CINEMA,          (INT32)eCM_PICTURE_SETTINGS_VIDEO,          (INT32)CLI_PRE_USER_CINEMA,         (INT32)eGM_PRE_USER_VIDEO},
	/* 03 */ {(INT32)eGUI_PRE_USER_HDR,             (INT32)eCM_PICTURE_SETTINGS_HDR,            (INT32)CLI_PRE_USER_HDR,            (INT32)eGM_PRE_USER_HDR},
	/* 04 */ {(INT32)eGUI_PRE_USER_SRGB,            (INT32)eCM_PICTURE_SETTINGS_SRGB,           (INT32)CLI_PRE_USER_SRGB,           (INT32)eGM_PRE_USER_SRGB},
	/* 05 */ {(INT32)eGUI_PRE_USER_DICOMSIM,        (INT32)eCM_PICTURE_SETTINGS_DICOMSIM,       (INT32)CLI_PRE_USER_DICOMSIM,       (INT32)eGM_PRE_USER_DICOMSIM},
	/* 06 */ {(INT32)eGUI_PRE_USER_BLENDING,        (INT32)eCM_PICTURE_SETTINGS_BLENDING,       (INT32)CLI_PRE_USER_BLENDING,       (INT32)eGM_PRE_USER_BLENDING},
	/* 07 */ {(INT32)eGUI_PRE_USER_3D,              (INT32)eCM_PICTURE_SETTINGS_3D,             (INT32)CLI_PRE_USER_3D,             (INT32)eGM_PRE_USER_3D},
	/* 08 */ {(INT32)eGUI_PRE_USER_2D_HIGH_SPEED,   (INT32)eCM_PICTURE_SETTINGS_2DHIGHSPEED,    (INT32)CLI_PRE_USER_2D_HIGH_SPEED,  (INT32)eGM_PRE_USER_2D_HIGH_SPEED},
};

/////////////////////////////////////////////////////////////////////////////////////
// Language
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_Language[] =
{
    /* 00 */ {(INT32)eGUI_LANGUAGE_ENGLISH,     (INT32)eCM_LANGUAGE_ENGLISH,    (INT32)CLI_LANGUAGE_ENGLISH,    (INT32)eGM_LANGUAGE_ENGLISH},
    /* 01 */ {(INT32)eGUI_LANGUAGE_CZECH,       (INT32)eCM_LANGUAGE_CZECH,      (INT32)CLI_LANGUAGE_CZECH,      (INT32)eGM_LANGUAGE_CZECH},
    /* 02 */ {(INT32)eGUI_LANGUAGE_DANISH,      (INT32)eCM_LANGUAGE_DANISH,     (INT32)CLI_LANGUAGE_DANISH,     (INT32)eGM_LANGUAGE_DANISH},
    /* 03 */ {(INT32)eGUI_LANGUAGE_DUTCH,       (INT32)eCM_LANGUAGE_DUTCH,      (INT32)CLI_LANGUAGE_DUTCH,      (INT32)eGM_LANGUAGE_DUTCH},
    /* 04 */ {(INT32)eGUI_LANGUAGE_FINNISH,     (INT32)eCM_LANGUAGE_FINNISH,    (INT32)CLI_LANGUAGE_FINNISH,    (INT32)eGM_LANGUAGE_FINNISH},
    /* 05 */ {(INT32)eGUI_LANGUAGE_FRENCH,      (INT32)eCM_LANGUAGE_FRENCH,     (INT32)CLI_LANGUAGE_FRENCH,     (INT32)eGM_LANGUAGE_FRENCH},
    /* 06 */ {(INT32)eGUI_LANGUAGE_GERMAN,      (INT32)eCM_LANGUAGE_GERMAN,     (INT32)CLI_LANGUAGE_GERMAN,     (INT32)eGM_LANGUAGE_GERMAN},
    /* 07 */ {(INT32)eGUI_LANGUAGE_GREEK,       (INT32)eCM_LANGUAGE_GREEK,      (INT32)CLI_LANGUAGE_GREEK,      (INT32)eGM_LANGUAGE_GREEK},
    /* 08 */ {(INT32)eGUI_LANGUAGE_HUNGARIAN,   (INT32)eCM_LANGUAGE_HUNGARIAN,  (INT32)CLI_LANGUAGE_HUNGARIAN,  (INT32)eGM_LANGUAGE_HUNGARIAN},
    /* 09 */ {(INT32)eGUI_LANGUAGE_INDONESIAN,  (INT32)eCM_LANGUAGE_INDONESIAN, (INT32)CLI_LANGUAGE_INDONESIAN, (INT32)eGM_LANGUAGE_INDONESIAN},
    /* 10 */ {(INT32)eGUI_LANGUAGE_ITALIAN,     (INT32)eCM_LANGUAGE_ITALIAN,    (INT32)CLI_LANGUAGE_ITALIAN,    (INT32)eGM_LANGUAGE_ITALIAN},
    /* 11 */ {(INT32)eGUI_LANGUAGE_JAPANESE,    (INT32)eCM_LANGUAGE_JAPANESE,   (INT32)CLI_LANGUAGE_JAPANESE,   (INT32)eGM_LANGUAGE_JAPANESE},
    /* 12 */ {(INT32)eGUI_LANGUAGE_KOREAN,      (INT32)eCM_LANGUAGE_KOREAN,     (INT32)CLI_LANGUAGE_KOREAN,     (INT32)eGM_LANGUAGE_KOREAN},
    /* 13 */ {(INT32)eGUI_LANGUAGE_NORWEGIAN,   (INT32)eCM_LANGUAGE_NORWEGIAN,  (INT32)CLI_LANGUAGE_NORWEGIAN,  (INT32)eGM_LANGUAGE_NORWEGIAN}, //A65_OPTOMA_Julie_0036
    /* 14 */ {(INT32)eGUI_LANGUAGE_POLISH,      (INT32)eCM_LANGUAGE_POLSKI,     (INT32)CLI_LANGUAGE_POLISH,     (INT32)eGM_LANGUAGE_POLISH},    //A65_OPTOMA_Julie_0036
    /* 15 */ {(INT32)eGUI_LANGUAGE_PORTUGUESE,  (INT32)eCM_LANGUAGE_PORTUGUESE, (INT32)CLI_LANGUAGE_PORTUGUESE, (INT32)eGM_LANGUAGE_PORTUGUESE},
    /* 16 */ {(INT32)eGUI_LANGUAGE_ROMANIAN,    (INT32)eCM_LANGUAGE_ROMANIAN,   (INT32)CLI_LANGUAGE_ROMANIAN,   (INT32)eGM_LANGUAGE_ROMANIAN},
    /* 17 */ {(INT32)eGUI_LANGUAGE_RUSSIAN,     (INT32)eCM_LANGUAGE_RUSSIAN,    (INT32)CLI_LANGUAGE_RUSSIAN,    (INT32)eGM_LANGUAGE_RUSSIAN},
    /* 18 */ {(INT32)eGUI_LANGUAGE_S_CHINESE,   (INT32)eCM_LANGUAGE_S_CHINESE,  (INT32)CLI_LANGUAGE_S_CHINESE,  (INT32)eGM_LANGUAGE_S_CHINESE},
    /* 19 */ {(INT32)eGUI_LANGUAGE_SPANISH,     (INT32)eCM_LANGUAGE_SPANISH,    (INT32)CLI_LANGUAGE_SPANISH,    (INT32)eGM_LANGUAGE_SPANISH},
    /* 20 */ {(INT32)eGUI_LANGUAGE_SWEDISH,     (INT32)eCM_LANGUAGE_SWEDISH,    (INT32)CLI_LANGUAGE_SWEDISH,    (INT32)eGM_LANGUAGE_SWEDISH},
    /* 21 */ {(INT32)eGUI_LANGUAGE_THAI,        (INT32)eCM_LANGUAGE_THAILAND,   (INT32)CLI_LANGUAGE_THAI,       (INT32)eGM_LANGUAGE_THAI},  //A65_OPTOMA_Julie_0036
    /* 22 */ {(INT32)eGUI_LANGUAGE_T_CHINESE,   (INT32)eCM_LANGUAGE_T_CHINESE,  (INT32)CLI_LANGUAGE_T_CHINESE,  (INT32)eGM_LANGUAGE_T_CHINESE},
    /* 23 */ {(INT32)eGUI_LANGUAGE_TURKISH,     (INT32)eCM_LANGUAGE_TURKISH,    (INT32)CLI_LANGUAGE_TURKISH,    (INT32)eGM_LANGUAGE_TURKISH},
    /* 24 */ {(INT32)eGUI_LANGUAGE_VIETNAMESE,  (INT32)eCM_LANGUAGE_VIETNAMESE, (INT32)CLI_LANGUAGE_VIETNAMESE, (INT32)eGM_LANGUAGE_VIETNAMESE},
};

/////////////////////////////////////////////////////////////////////////////////////
// Size Presets
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_ScalingMode[] =
{
    /* 00 */ {(INT32)eGUI_SCALING_MODE_4_3,         (INT32)eCM_SCALING_MODE_4_3,        (INT32)CLI_SCALING_MODE_4_3,          (INT32)eGM_SCALING_MODE_4_3},
    /* 01 */ {(INT32)eGUI_SCALING_MODE_16_9,        (INT32)eCM_SCALING_MODE_16_9,       (INT32)CLI_SCALING_MODE_16_9,         (INT32)eGM_SCALING_MODE_16_9},
    /* 02 */ {(INT32)eGUI_SCALING_MODE_21_9,        (INT32)eCM_SCALING_MODE_21_9,       (INT32)CLI_SCALING_MODE_21_9,         (INT32)eGM_SCALING_MODE_21_9},
    /* 03 */ {(INT32)eGUI_SCALING_MODE_LETTER_BOX,  (INT32)eCM_SCALING_MODE_LETTER_BOX, (INT32)CLI_SCALING_MODE_LETTER_BOX,   (INT32)eGM_SCALING_MODE_LETTER_BOX},
    /* 04 */ {(INT32)eGUI_SCALING_MODE_AUTO,        (INT32)eCM_SCALING_MODE_AUTO,       (INT32)CLI_SCALING_MODE_AUTO,         (INT32)eGM_SCALING_MODE_AUTO},
    /* 05 */ {(INT32)eGUI_SCALING_MODE_NATIVE,      (INT32)eCM_SCALING_MODE_NATIVE,     (INT32)CLI_SCALING_MODE_NATIVE,       (INT32)eGM_SCALING_MODE_NATIVE},
};

/////////////////////////////////////////////////////////////////////////////////////
// Hotkey Settings
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_HotkeySettings[] =
{
    /* 00 */ {(INT32)eGUI_HOTKEY_SETTINGS_HDMI1,                    (INT32)eCM_HOTKEY_SETTINGS_HDMI1,               (INT32)CLI_HOTKEY_SETTINGS_HDMI1,                   (INT32)eGM_HOTKEY_SETTINGS_HDMI1},
	/* 01 */ {(INT32)eGUI_HOTKEY_SETTINGS_HDMI2,                    (INT32)eCM_HOTKEY_SETTINGS_HDMI2,               (INT32)CLI_HOTKEY_SETTINGS_HDMI2,                   (INT32)eGM_HOTKEY_SETTINGS_HDMI2},
    /* 02 */ {(INT32)eGUI_HOTKEY_SETTINGS_COLOR_MATCHING,           (INT32)eCM_HOTKEY_SETTINGS_CUSTOM_RGBCYM,       (INT32)CLI_HOTKEY_SETTINGS_COLOR_MATCHING,          (INT32)eGM_HOTKEY_SETTINGS_COLOR_MATCHING},
    /* 03 */ {(INT32)eGUI_HOTKEY_SETTINGS_COLOR_TEMPERATURE,        (INT32)eCM_HOTKEY_SETTINGS_COLOR_TEMPERATURE,   (INT32)CLI_HOTKEY_SETTINGS_COLOR_TEMPERATURE,       (INT32)eGM_HOTKEY_SETTINGS_COLOR_TEMPERATURE},
    /* 04 */ {(INT32)eGUI_HOTKEY_SETTINGS_PROJECTION_ORIENTATION,   (INT32)eCM_HOTKEY_SETTINGS_ORIENTATION,         (INT32)CLI_HOTKEY_SETTINGS_PROJECTION_ORIENTATION,  (INT32)eGM_HOTKEY_SETTINGS_PROJECTION_ORIENTATION},
    /* 05 */ {(INT32)eGUI_HOTKEY_SETTINGS_LIGHT_SOURCE_MODE,        (INT32)eCM_HOTKEY_SETTINGS_LIGHT_SOURCE_MODE,   (INT32)CLI_HOTKEY_SETTINGS_LIGHT_SOURCE_MODE,       (INT32)eGM_HOTKEY_SETTINGS_LIGHT_SOURCE_MODE},
    /* 06 */ {(INT32)eGUI_HOTKEY_SETTINGS_FREEZE_SCREEN,            (INT32)eCM_HOTKEY_SETTINGS_FREEZE_SCREEN,       (INT32)CLI_HOTKEY_SETTINGS_FREEZE_SCREEN,           (INT32)eGM_HOTKEY_SETTINGS_FREEZE_SCREEN},
	/* 07 */ {(INT32)eGUI_HOTKEY_SETTINGS_NETWORK_SETUP,            (INT32)eCM_HOTKEY_SETTINGS_NETWORK_SETUP,       (INT32)CLI_HOTKEY_SETTINGS_NETWORK_SETUP,           (INT32)eGM_HOTKEY_SETTINGS_NETWORK_SETUP},
    /* 08 */ {(INT32)eGUI_HOTKEY_SETTINGS_RESET_SELECTIVE,          (INT32)eCM_HOTKEY_SETTINGS_RESET_SELECTIVE,     (INT32)CLI_HOTKEY_SETTINGS_RESET_SELECTIVE,         (INT32)eGM_HOTKEY_SETTINGS_RESET_SELECTIVE},
};

/////////////////////////////////////////////////////////////////////////////////////
// Screen Mode (PIP/PBP)
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_ScreenMode[] =
{
    /* 00 */ {(INT32)eGUI_SCREEN_MODE_OFF, (INT32)eCM_SCREEN_MODE_OFF, (INT32)CLI_SCREEN_MODE_OFF,  (INT32)eGM_SCREEN_MODE_OFF},
    /* 01 */ {(INT32)eGUI_SCREEN_MODE_PIP, (INT32)eCM_SCREEN_MODE_PIP, (INT32)CLI_SCREEN_MODE_PIP,  (INT32)eGM_SCREEN_MODE_PIP},
    /* 02 */ {(INT32)eGUI_SCREEN_MODE_PBP, (INT32)eCM_SCREEN_MODE_PBP, (INT32)CLI_SCREEN_MODE_PBP,  (INT32)eGM_SCREEN_MODE_PBP},
};

/////////////////////////////////////////////////////////////////////////////////////
// Menu Location
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_MenuLocation[] =
{
    /* 00 */ {(INT32)eGUI_MENU_LOCATION_TOP_LEFT,       (INT32)eCM_MENU_LOCATION_TOP_LEFT,      (INT32)CLI_MENU_LOCATION_TOP_LEFT,       (INT32)eGM_MENU_LOCATION_TOP_LEFT},
    /* 01 */ {(INT32)eGUI_MENU_LOCATION_TOP_RIGHT,      (INT32)eCM_MENU_LOCATION_TOP_RIGHT,     (INT32)CLI_MENU_LOCATION_TOP_RIGHT,      (INT32)eGM_MENU_LOCATION_TOP_RIGHT},
    /* 02 */ {(INT32)eGUI_MENU_LOCATION_CENTER,         (INT32)eCM_MENU_LOCATION_CENTER,        (INT32)CLI_MENU_LOCATION_CENTER,         (INT32)eGM_MENU_LOCATION_CENTER},
    /* 03 */ {(INT32)eGUI_MENU_LOCATION_BOTTOM_LEFT,    (INT32)eCM_MENU_LOCATION_BOTTOM_LEFT,   (INT32)CLI_MENU_LOCATION_BOTTOM_LEFT,    (INT32)eGM_MENU_LOCATION_BOTTOM_LEFT},
    /* 04 */ {(INT32)eGUI_MENU_LOCATION_BOTTOM_RIGHT,   (INT32)eCM_MENU_LOCATION_BOTTOM_RIGHT,  (INT32)CLI_MENU_LOCATION_BOTTOM_RIGHT,   (INT32)eGM_MENU_LOCATION_BOTTOM_RIGHT},
};

/////////////////////////////////////////////////////////////////////////////////////
// OverScan
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_OverScan[] =
{
    /* 00 */ {(INT32)eGUI_OVERSCAN_OFF,  (INT32)eCM_OVERSCAN_OFF,  (INT32)CLI_OVERSCAN_OFF,    (INT32)eGM_OVERSCAN_OFF},
    /* 01 */ {(INT32)eGUI_OVERSCAN_ZOOM, (INT32)eCM_OVERSCAN_ZOOM, (INT32)CLI_OVERSCAN_ZOOM,   (INT32)eGM_OVERSCAN_ZOOM},
    /* 02 */ {(INT32)eGUI_OVERSCAN_CROP, (INT32)eCM_OVERSCAN_CROP, (INT32)CLI_OVERSCAN_CROP,   (INT32)eGM_OVERSCAN_CROP},
};


/////////////////////////////////////////////////////////////////////////////////////
// Ceiling Mount
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_CeilingMount[] =
{
    /* 00 */  {(INT32)eGUI_CEILING_MOUNT_AUTO,   (INT32)eCM_CEILING_MOUNT_AUTO,  (INT32)CLI_CEILING_MOUNT_AUTO,   (INT32)eGM_CEILING_MOUNT_AUTO},
    /* 01 */  {(INT32)eGUI_CEILING_MOUNT_ON,     (INT32)eCM_CEILING_MOUNT_ON,    (INT32)CLI_CEILING_MOUNT_ON,     (INT32)eGM_CEILING_MOUNT_ON},
    /* 02 */  {(INT32)eGUI_CEILING_MOUNT_OFF,    (INT32)eCM_CEILING_MOUNT_OFF,   (INT32)CLI_CEILING_MOUNT_OFF,    (INT32)eGM_CEILING_MOUNT_OFF},
};

/////////////////////////////////////////////////////////////////////////////////////
// Gamma
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_Gamma[] =   //G100_Simon_0022
{
    /* 00 */ {(INT32)eGUI_GAMMA_FILM,           (INT32)eCM_GAMMA_FILM,          (INT32)CLI_GAMMA_FILM,          (INT32)eGM_GAMMA_FILM},
    /* 01 */ {(INT32)eGUI_GAMMA_GRAPHICS,       (INT32)eCM_GAMMA_GRAPHICS,      (INT32)CLI_GAMMA_GRAPHICS,      (INT32)eGM_GAMMA_GRAPHICS},
    /* 02 */ {(INT32)eGUI_GAMMA_1_8,            (INT32)eCM_GAMMA_1_8,           (INT32)CLI_GAMMA_1_8,           (INT32)eGM_GAMMA_1_8},
    /* 03 */ {(INT32)eGUI_GAMMA_2_0,            (INT32)eCM_GAMMA_2_0,           (INT32)CLI_GAMMA_2_0,           (INT32)eGM_GAMMA_2_0},
    /* 04 */ {(INT32)eGUI_GAMMA_2_2,            (INT32)eCM_GAMMA_2_2,           (INT32)CLI_GAMMA_2_2,           (INT32)eGM_GAMMA_2_2},
    /* 05 */ {(INT32)eGUI_GAMMA_2_4,            (INT32)eCM_GAMMA_2_4,           (INT32)CLI_GAMMA_2_4,           (INT32)eGM_GAMMA_2_4},
    /* 06 */ {(INT32)eGUI_GAMMA_2_6,            (INT32)eCM_GAMMA_2_6,           (INT32)CLI_GAMMA_2_6,           (INT32)eGM_GAMMA_2_6},
    /* 07 */ {(INT32)eGUI_GAMMA_VIVID,          (INT32)eCM_GAMMA_CRT,/*VIVID*/  (INT32)CLI_GAMMA_VIVID,         (INT32)eGM_GAMMA_VIVID},
    /* 08 */ {(INT32)eGUI_GAMMA_3D,             (INT32)eCM_GAMMA_3D,            (INT32)CLI_GAMMA_3D,            (INT32)eGM_GAMMA_3D},
    /* 09 */ {(INT32)eGUI_GAMMA_BLACKBOARD,     (INT32)eCM_GAMMA_BLACKBOARD,    (INT32)CLI_GAMMA_BLACKBOARD,    (INT32)eGM_GAMMA_BLACKBOARD},
    /* 10 */ {(INT32)GUI_VALUE_INVALID,         (INT32)eCM_GAMMA_DICOM,         (INT32)CLI_VALUE_INVALID,       (INT32)eGM_GAMMA_DICOM},   
    /* 11 */ {(INT32)eGUI_GAMMA_HDR,            (INT32)eCM_GAMMA_HDR_STANDARD,  (INT32)CLI_GAMMA_HDR,           (INT32)eGM_GAMMA_HDR},
};

/////////////////////////////////////////////////////////////////////////////////////
// Lens Memory Save
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_LensMemorySave[] =
{
    /* 00 */ {(INT32)eGUI_LENS_MEMORY_SAVE_1, (INT32)eCM_LENS_MEMORY_SAVE_1, (INT32)CLI_LENS_MEMORY_SAVE_1,   (INT32)eGM_LENS_MEMORY_SAVE_1},
    /* 01 */ {(INT32)eGUI_LENS_MEMORY_SAVE_2, (INT32)eCM_LENS_MEMORY_SAVE_2, (INT32)CLI_LENS_MEMORY_SAVE_2,   (INT32)eGM_LENS_MEMORY_SAVE_2},
    /* 02 */ {(INT32)eGUI_LENS_MEMORY_SAVE_3, (INT32)eCM_LENS_MEMORY_SAVE_3, (INT32)CLI_LENS_MEMORY_SAVE_3,   (INT32)eGM_LENS_MEMORY_SAVE_3},
    /* 03 */ {(INT32)eGUI_LENS_MEMORY_SAVE_4, (INT32)eCM_LENS_MEMORY_SAVE_4, (INT32)CLI_LENS_MEMORY_SAVE_4,   (INT32)eGM_LENS_MEMORY_SAVE_4},
    /* 04 */ {(INT32)eGUI_LENS_MEMORY_SAVE_5, (INT32)eCM_LENS_MEMORY_SAVE_5, (INT32)CLI_LENS_MEMORY_SAVE_5,   (INT32)eGM_LENS_MEMORY_SAVE_5},
};

/////////////////////////////////////////////////////////////////////////////////////
// Lens Memory Apply
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_LensMemoryApply[] =
{
    /* 00 */ {(INT32)eGUI_LENS_MEMORY_APPLY_1, (INT32)eCM_LENS_MEMORY_APPLY_1, (INT32)CLI_LENS_MEMORY_APPLY_1,  (INT32)eGM_LENS_MEMORY_APPLY_1},
    /* 01 */ {(INT32)eGUI_LENS_MEMORY_APPLY_2, (INT32)eCM_LENS_MEMORY_APPLY_2, (INT32)CLI_LENS_MEMORY_APPLY_2,  (INT32)eGM_LENS_MEMORY_APPLY_2},
    /* 02 */ {(INT32)eGUI_LENS_MEMORY_APPLY_3, (INT32)eCM_LENS_MEMORY_APPLY_3, (INT32)CLI_LENS_MEMORY_APPLY_3,  (INT32)eGM_LENS_MEMORY_APPLY_3},
    /* 03 */ {(INT32)eGUI_LENS_MEMORY_APPLY_4, (INT32)eCM_LENS_MEMORY_APPLY_4, (INT32)CLI_LENS_MEMORY_APPLY_4,  (INT32)eGM_LENS_MEMORY_APPLY_4},
    /* 04 */ {(INT32)eGUI_LENS_MEMORY_APPLY_5, (INT32)eCM_LENS_MEMORY_APPLY_5, (INT32)CLI_LENS_MEMORY_APPLY_5,  (INT32)eGM_LENS_MEMORY_APPLY_5},
};

/////////////////////////////////////////////////////////////////////////////////////
// Orientation Direction
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_OrientationDirection[] =
{
    /* 00 */ {(INT32)eGUI_REAR_PROJECTION_FRONT, (INT32)eCM_REAR_PROJECTION_FRONT, (INT32)CLI_REAR_PROJECTION_FRONT,   (INT32)eGM_REAR_PROJECTION_FRONT},
    /* 01 */ {(INT32)eGUI_REAR_PROJECTION_REAR,  (INT32)eCM_REAR_PROJECTION_REAR,  (INT32)CLI_REAR_PROJECTION_REAR,    (INT32)eGM_REAR_PROJECTION_REAR},
};

/////////////////////////////////////////////////////////////////////////////////////
// EDID Type
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_EDID_Type[] =
{
    /* 00 */ {(INT32)eGUI_EDID_TYPE_V14,        (INT32)eCM_EDID_TYPE_V14,           (INT32)CLI_EDID_TYPE_V14,           (INT32)eGM_EDID_TYPE_V14},
    /* 01 */ {(INT32)eGUI_EDID_TYPE_V20,        (INT32)eCM_EDID_TYPE_V20,           (INT32)CLI_EDID_TYPE_V20,           (INT32)eGM_EDID_TYPE_V20},
    ///* 02 */ {(INT32)eGUI_EDID_TYPE_CUSTOMIZED, (INT32)eCM_EDID_TYPE_CUSTOMIZED,    (INT32)CLI_EDID_TYPE_CUSTOMIZED,    (INT32)eGM_EDID_TYPE_CUSTOMIZED},
};

/////////////////////////////////////////////////////////////////////////////////////
// Blending Gamma
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_BlendingGamma[] =
{
    /* 00 */ {(INT32)eGUI_BLENDING_GAMMA_1_8, (INT32)eCM_BLENDING_GAMMA_1_8, (INT32)CLI_BLENDING_GAMMA_1_8,  (INT32)eGM_BLENDING_GAMMA_1_8},
    /* 01 */ {(INT32)eGUI_BLENDING_GAMMA_1_9, (INT32)eCM_BLENDING_GAMMA_1_9, (INT32)CLI_BLENDING_GAMMA_1_9,  (INT32)eGM_BLENDING_GAMMA_1_9},
    /* 02 */ {(INT32)eGUI_BLENDING_GAMMA_2_0, (INT32)eCM_BLENDING_GAMMA_2_0, (INT32)CLI_BLENDING_GAMMA_2_0,  (INT32)eGM_BLENDING_GAMMA_2_0},
    /* 03 */ {(INT32)eGUI_BLENDING_GAMMA_2_1, (INT32)eCM_BLENDING_GAMMA_2_1, (INT32)CLI_BLENDING_GAMMA_2_1,  (INT32)eGM_BLENDING_GAMMA_2_1},
    /* 04 */ {(INT32)eGUI_BLENDING_GAMMA_2_2, (INT32)eCM_BLENDING_GAMMA_2_2, (INT32)CLI_BLENDING_GAMMA_2_2,  (INT32)eGM_BLENDING_GAMMA_2_2},
    /* 05 */ {(INT32)eGUI_BLENDING_GAMMA_2_3, (INT32)eCM_BLENDING_GAMMA_2_3, (INT32)CLI_BLENDING_GAMMA_2_3,  (INT32)eGM_BLENDING_GAMMA_2_3},
    /* 06 */ {(INT32)eGUI_BLENDING_GAMMA_2_4, (INT32)eCM_BLENDING_GAMMA_2_4, (INT32)CLI_BLENDING_GAMMA_2_4,  (INT32)eGM_BLENDING_GAMMA_2_4},
};

/////////////////////////////////////////////////////////////////////////////////////
// Color Temperature
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_ColorTemperature[] =
{
    /* 00 */ {(INT32)eGUI_COLOR_TEMPERATURE_WARM,       (INT32)eCM_COLOR_TEMPERATURE_WARMEST,   (INT32)CLI_COLOR_TEMPERATURE_WARM,      (INT32)eGM_COLOR_TEMPERATURE_WARM},
    /* 01 */ {(INT32)eGUI_COLOR_TEMPERATURE_STANDARD,   (INT32)eCM_COLOR_TEMPERATURE_WARM,      (INT32)CLI_COLOR_TEMPERATURE_STANDARD,  (INT32)eGM_COLOR_TEMPERATURE_STANDARD},
    /* 02 */ {(INT32)eGUI_COLOR_TEMPERATURE_COOL,       (INT32)eCM_COLOR_TEMPERATURE_COOL,    (INT32)CLI_COLOR_TEMPERATURE_COOL,      (INT32)eGM_COLOR_TEMPERATURE_COOL},
    /* 03 */ {(INT32)eGUI_COLOR_TEMPERATURE_COLD,       (INT32)eCM_COLOR_TEMPERATURE_NATIVE,      (INT32)CLI_COLOR_TEMPERATURE_COLD,      (INT32)eGM_COLOR_TEMPERATURE_COLD},
};

/////////////////////////////////////////////////////////////////////////////////////
// Color Space
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_ColorSpace[] =
{
    /* 00 */ {(INT32)eGUI_COLOR_SPACE_AUTO,         (INT32)eCM_COLOR_SPACE_AUTO,        (INT32)CLI_COLOR_SPACE_AUTO,          (INT32)eGM_COLOR_SPACE_AUTO},
    /* 01 */ {(INT32)eGUI_COLOR_SPACE_RGB_FULL,     (INT32)eCM_COLOR_SPACE_RGB_FULL,    (INT32)CLI_COLOR_SPACE_RGB_FULL,      (INT32)eGM_COLOR_SPACE_RGB_FULL},
    /* 02 */ {(INT32)eGUI_COLOR_SPACE_RGB_LIMITED,  (INT32)eCM_COLOR_SPACE_RGB_LIMITED, (INT32)CLI_COLOR_SPACE_RGB_LIMITED,   (INT32)eGM_COLOR_SPACE_RGB_LIMITED},
    /* 03 */ {(INT32)eGUI_COLOR_SPACE_YUV_REC709,   (INT32)eCM_COLOR_SPACE_YUV_REC709,  (INT32)CLI_COLOR_SPACE_YUV_REC709,    (INT32)eGM_COLOR_SPACE_YUV_REC709},
    /* 04 */ {(INT32)eGUI_COLOR_SPACE_YUV_REC601,   (INT32)eCM_COLOR_SPACE_YUV_REC601,  (INT32)CLI_COLOR_SPACE_YUV_REC601,    (INT32)eGM_COLOR_SPACE_YUV_REC601},
};

/////////////////////////////////////////////////////////////////////////////////////
// Wall Color
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_WallColor[] =
{
    /* 00 */ {(INT32)eGUI_WALL_COLOR_OFF,           (INT32)eCM_WALL_COLOR_OFF,          (INT32)CLI_WALL_COLOR_OFF,		   	(INT32)eGM_WALL_COLOR_OFF},
    /* 01 */ {(INT32)eGUI_WALL_COLOR_BLACKBOARD,    (INT32)eCM_WALL_COLOR_BLACKBOARD,   (INT32)CLI_WALL_COLOR_BLACKBOARD,  	(INT32)eGM_WALL_COLOR_BLACKBOARD},
    /* 02 */ {(INT32)eGUI_WALL_COLOR_LIGHT_YELLOW,  (INT32)eCM_WALL_COLOR_LIGHT_YELLOW, (INT32)CLI_WALL_COLOR_LIGHT_YELLOW,	(INT32)eGM_WALL_COLOR_LIGHT_YELLOW},
    /* 03 */ {(INT32)eGUI_WALL_COLOR_LIGHT_GREEN,   (INT32)eCM_WALL_COLOR_LIGHT_GREEN,  (INT32)CLI_WALL_COLOR_LIGHT_GREEN, 	(INT32)eGM_WALL_COLOR_LIGHT_GREEN},
    /* 04 */ {(INT32)eGUI_WALL_COLOR_LIGHT_BLUE,    (INT32)eCM_WALL_COLOR_LIGHT_BLUE,   (INT32)CLI_WALL_COLOR_LIGHT_BLUE,  	(INT32)eGM_WALL_COLOR_LIGHT_BLUE},
    /* 05 */ {(INT32)eGUI_WALL_COLOR_PINK,          (INT32)eCM_WALL_COLOR_PINK,         (INT32)CLI_WALL_COLOR_PINK,        	(INT32)eGM_WALL_COLOR_PINK},
    /* 06 */ {(INT32)eGUI_WALL_COLOR_GRAY,          (INT32)eCM_WALL_COLOR_GRAY,         (INT32)CLI_WALL_COLOR_GRAY,        	(INT32)eGM_WALL_COLOR_GRAY},
};

/////////////////////////////////////////////////////////////////////////////////////
// 3D Format
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_3DFormat[] =
{
    /* 00 */ {(INT32)eGUI_3D_FORMAT_AUTO,               (INT32)eCM_3D_FORMAT_AUTO,              (INT32)CLI_3D_FORMAT_AUTO,              (INT32)eGM_3D_FORMAT_AUTO},
    /* 01 */ {(INT32)eGUI_3D_FORMAT_FRAME_PACKING,      (INT32)eCM_3D_FORMAT_FRAME_PACKING,     (INT32)CLI_3D_FORMAT_FRAME_PACKING,     (INT32)eGM_3D_FORMAT_FRAME_PACKING},
    /* 02 */ {(INT32)eGUI_3D_FORMAT_SIDE_BY_SIDE,       (INT32)eCM_3D_FORMAT_SIDE_BY_SIDE,      (INT32)CLI_3D_FORMAT_SIDE_BY_SIDE,      (INT32)eGM_3D_FORMAT_SIDE_BY_SIDE},
    /* 03 */ {(INT32)eGUI_3D_FORMAT_TOP_AND_BOTTOM,     (INT32)eCM_3D_FORMAT_TOP_AND_BOTTOM,    (INT32)CLI_3D_FORMAT_TOP_AND_BOTTOM,    (INT32)eGM_3D_FORMAT_TOP_AND_BOTTOM},
    /* 04 */ {(INT32)eGUI_3D_FORMAT_FRAME_SEQUENTIAL,   (INT32)eCM_3D_FORMAT_FRAME_SEQUENTIAL,  (INT32)CLI_3D_FORMAT_FRAME_SEQUENTIAL,  (INT32)eGM_3D_FORMAT_FRAME_SEQUENTIAL},
    /* 05 */ {(INT32)GUI_VALUE_INVALID,   	            (INT32)eCM_3D_FORMAT_DUALPIPE_3D,  		(INT32)CLI_VALUE_INVALID,               (INT32)eGM_3D_FORMAT_DUALPIPE},
    /* 06 */ {(INT32)GUI_VALUE_INVALID,   	            (INT32)eCM_3D_FORMAT_4K3D,  			(INT32)CLI_VALUE_INVALID,               (INT32)eGM_3D_FORMAT_4K3D},
    /* 07 */ {(INT32)GUI_VALUE_INVALID,   	            (INT32)eCM_3D_FORMAT_4K3D_DUALPIPE,  	(INT32)CLI_VALUE_INVALID,               (INT32)eGM_3D_FORMAT_4K3D_DUALPIPE},
    /* 08 */ {(INT32)GUI_VALUE_INVALID,   	            (INT32)eCM_3D_FORMAT_OFF,  			    (INT32)CLI_VALUE_INVALID,               (INT32)eGM_3D_FORMAT_OFF},
};

/////////////////////////////////////////////////////////////////////////////////////
// 3D Tech
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_3DTech[] =
{
    /* 00 */ {(INT32)eGUI_3D_TECH_DLP_LINK, (INT32)eCM_3D_TECH_DLP_LINK, (INT32)CLI_3D_TECH_DLP_LINK,  (INT32)eGM_3D_TECH_DLP_LINK},
    /* 01 */ {(INT32)eGUI_3D_TECH_3D_SYNC,  (INT32)eCM_3D_TECH_3D_SYNC,  (INT32)CLI_3D_TECH_3D_SYNC,   (INT32)eGM_3D_TECH_3D_SYNC},
};

/////////////////////////////////////////////////////////////////////////////////////
// 3D-2D
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_3D2DView[] =
{
    /* 00 */ {(INT32)eGUI_3D_2D_3D,         (INT32)eCM_3D_2D_3D,        (INT32)CLI_3D_2D_3D,         (INT32)eGM_3D_2D_3D},
    /* 01 */ {(INT32)eGUI_3D_2D_2D_LEFT,    (INT32)eCM_3D_2D_2D_LEFT,   (INT32)CLI_3D_2D_2D_LEFT,    (INT32)eGM_3D_2D_2D_LEFT},
    /* 02 */ {(INT32)eGUI_3D_2D_2D_RIGHT,   (INT32)eCM_3D_2D_2D_RIGHT,  (INT32)CLI_3D_2D_2D_RIGHT,   (INT32)eGM_3D_2D_2D_RIGHT},
};

/////////////////////////////////////////////////////////////////////////////////////
// 3D Sync Out
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_3DSyncOut[] =
{
    /* 00 */ {(INT32)eGUI_3D_SYNC_OUT_TO_EMITTER,        (INT32)eCM_3D_SYNC_OUT_TO_EMITTER,         (INT32)CLI_3D_SYNC_OUT_TO_EMITTER,          (INT32)eGM_3D_SYNC_OUT_TO_EMITTER},
    /* 01 */ {(INT32)eGUI_3D_SYNC_OUT_TO_NEXT_PROJECTOR, (INT32)eCM_3D_SYNC_OUT_TO_NEXT_PROJECTOR,  (INT32)CLI_3D_SYNC_OUT_TO_NEXT_PROJECTOR,   (INT32)eGM_3D_SYNC_OUT_TO_NEXT_PROJECTOR},
};

/////////////////////////////////////////////////////////////////////////////////////
// 3D LR Reference
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_3DLRReference[] =
{
    /* 00 */ {(INT32)eGUI_3D_LR_REFERENCE_1ST_FRAME,     (INT32)eCM_3D_LR_REFERENCE_1ST_FRAME,   (INT32)CLI_3D_LR_REFERENCE_1ST_FRAME,     (INT32)eGM_3D_LR_REFERENCE_1ST_FRAME},
    /* 01 */ {(INT32)eGUI_3D_LR_REFERENCE_FIELD_GPIO,    (INT32)eCM_3D_LR_REFERENCE_FIELD_GPIO,  (INT32)CLI_3D_LR_REFERENCE_FIELD_GPIO,    (INT32)eGM_3D_LR_REFERENCE_FIELD_GPIO},
};

/////////////////////////////////////////////////////////////////////////////////////
// 3D Sync Select (3D Sync Type)
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_3DSyncType[] =
{
    /* 00 */ {(INT32)eGUI_3D_SYNC_TYPE_AUTO,        (INT32)eCM_3D_SYNC_TYPE_AUTO,       (INT32)CLI_3D_SYNC_TYPE_AUTO,       (INT32)eGM_3D_SYNC_TYPE_AUTO},
    /* 01 */ {(INT32)eGUI_3D_SYNC_TYPE_INTERNAL,    (INT32)eCM_3D_SYNC_TYPE_INTERNAL,   (INT32)CLI_3D_SYNC_TYPE_INTERNAL,   (INT32)eGM_3D_SYNC_TYPE_INTERNAL},
    /* 02 */ {(INT32)eGUI_3D_SYNC_TYPE_EXTERNAL,    (INT32)eCM_3D_SYNC_TYPE_EXTERNAL,   (INT32)CLI_3D_SYNC_TYPE_EXTERNAL,   (INT32)eGM_3D_SYNC_TYPE_EXTERNAL},
};

/////////////////////////////////////////////////////////////////////////////////////
// PIP Size
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_PIPSize[] =
{
    /* 00 */ {(INT32)eGUI_PIP_SIZE_SMALL,  (INT32)eCM_PIP_SIZE_SMALL,  (INT32)CLI_PIP_SIZE_SMALL,    (INT32)eGM_PIP_SIZE_SMALL},
    /* 01 */ {(INT32)eGUI_PIP_SIZE_MEDIUM, (INT32)eCM_PIP_SIZE_MEDIUM, (INT32)CLI_PIP_SIZE_MEDIUM,   (INT32)eGM_PIP_SIZE_MEDIUM},
    /* 02 */ {(INT32)eGUI_PIP_SIZE_LARGE,  (INT32)eCM_PIP_SIZE_LARGE,  (INT32)CLI_PIP_SIZE_LARGE,    (INT32)eGM_PIP_SIZE_LARGE},
};

/////////////////////////////////////////////////////////////////////////////////////
// PIP Layout
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_PIPLayout[] =
{
    /* 00 */ {(INT32)eGUI_MAIN_LAYOUT_PBP_MAIN_LEFT,     (INT32)eCM_MAIN_LAYOUT_PBP_MAIN_LEFT,   (INT32)CLI_MAIN_LAYOUT_PBP_MAIN_LEFT,     (INT32)eGM_MAIN_LAYOUT_PBP_MAIN_LEFT},
    /* 01 */ {(INT32)eGUI_MAIN_LAYOUT_PBP_MAIN_TOP,      (INT32)eCM_MAIN_LAYOUT_PBP_MAIN_TOP,    (INT32)CLI_MAIN_LAYOUT_PBP_MAIN_TOP,      (INT32)eGM_MAIN_LAYOUT_PBP_MAIN_TOP},
    /* 02 */ {(INT32)eGUI_MAIN_LAYOUT_PBP_MAIN_RIGHT,    (INT32)eCM_MAIN_LAYOUT_PBP_MAIN_RIGHT,  (INT32)CLI_MAIN_LAYOUT_PBP_MAIN_RIGHT,    (INT32)eGM_MAIN_LAYOUT_PBP_MAIN_RIGHT},
    /* 03 */ {(INT32)eGUI_MAIN_LAYOUT_PBP_MAIN_BOTTOM,   (INT32)eCM_MAIN_LAYOUT_PBP_MAIN_BOTTOM, (INT32)CLI_MAIN_LAYOUT_PBP_MAIN_BOTTOM,   (INT32)eGM_MAIN_LAYOUT_PBP_MAIN_BOTTOM},
    /* 04 */ {(INT32)eGUI_MAIN_LAYOUT_PIP_BOTTOM_RIGHT,  (INT32)eCM_MAIN_LAYOUT_PIP_BOTTOM_RIGHT,(INT32)CLI_MAIN_LAYOUT_PIP_BOTTOM_RIGHT,  (INT32)eGM_MAIN_LAYOUT_PIP_BOTTOM_RIGHT},
    /* 05 */ {(INT32)eGUI_MAIN_LAYOUT_PIP_BOTTOM_LEFT,   (INT32)eCM_MAIN_LAYOUT_PIP_BOTTOM_LEFT, (INT32)CLI_MAIN_LAYOUT_PIP_BOTTOM_LEFT,   (INT32)eGM_MAIN_LAYOUT_PIP_BOTTOM_LEFT},
    /* 06 */ {(INT32)eGUI_MAIN_LAYOUT_PIP_TOP_LEFT,      (INT32)eCM_MAIN_LAYOUT_PIP_TOP_LEFT,    (INT32)CLI_MAIN_LAYOUT_PIP_TOP_LEFT,      (INT32)eGM_MAIN_LAYOUT_PIP_TOP_LEFT},
    /* 07 */ {(INT32)eGUI_MAIN_LAYOUT_PIP_TOP_RIGHT,     (INT32)eCM_MAIN_LAYOUT_PIP_TOP_RIGHT,   (INT32)CLI_MAIN_LAYOUT_PIP_TOP_RIGHT,     (INT32)eGM_MAIN_LAYOUT_PIP_TOP_RIGHT},
};

/////////////////////////////////////////////////////////////////////////////////////
// Lan Path Switch
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_LanPathSwitch[] =
{
    /* 00 */ {(INT32)eGUI_LAN_PATH_SWITCH_RJ45,     (INT32)eCM_LAN_PATH_SWITCH_RJ45,    (INT32)CLI_LAN_PATH_SWITCH_RJ45,     (INT32)eGM_LAN_PATH_SWITCH_RJ45},
    /* 01 */ {(INT32)eGUI_LAN_PATH_SWITCH_HDBASET,  (INT32)eCM_LAN_PATH_SWITCH_HDBASET, (INT32)CLI_LAN_PATH_SWITCH_HDBASET,  (INT32)eGM_LAN_PATH_SWITCH_HDBASET},
};

/////////////////////////////////////////////////////////////////////////////////////
// Serial Port Baud Rate
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_SerialPortBaudRate[] =
{
    //* 00 */ {(INT32)eGUI_SERIAL_PORT_BAUD_RATE_1200,   (INT32)eCM_SERIAL_PORT_BAUD_RATE_1200,  (INT32)CLI_SERIAL_PORT_BAUD_RATE_1200,     (INT32)eGM_SERIAL_PORT_BAUD_RATE_1200},
    //* 01 */ {(INT32)eGUI_SERIAL_PORT_BAUD_RATE_2400,   (INT32)eCM_SERIAL_PORT_BAUD_RATE_2400,  (INT32)CLI_SERIAL_PORT_BAUD_RATE_2400,     (INT32)eGM_SERIAL_PORT_BAUD_RATE_2400},
    //* 02 */ {(INT32)eGUI_SERIAL_PORT_BAUD_RATE_4800,   (INT32)eCM_SERIAL_PORT_BAUD_RATE_4800,  (INT32)CLI_SERIAL_PORT_BAUD_RATE_4800,     (INT32)eGM_SERIAL_PORT_BAUD_RATE_4800},
    /* 03 */ {(INT32)eGUI_SERIAL_PORT_BAUD_RATE_9600,   (INT32)eCM_SERIAL_PORT_BAUD_RATE_9600,  (INT32)CLI_SERIAL_PORT_BAUD_RATE_9600,     (INT32)eGM_SERIAL_PORT_BAUD_RATE_9600},
    /* 04 */ {(INT32)eGUI_SERIAL_PORT_BAUD_RATE_19200,  (INT32)eCM_SERIAL_PORT_BAUD_RATE_19200, (INT32)CLI_SERIAL_PORT_BAUD_RATE_19200,    (INT32)eGM_SERIAL_PORT_BAUD_RATE_19200},
    /* 05 */ {(INT32)eGUI_SERIAL_PORT_BAUD_RATE_38400,  (INT32)eCM_SERIAL_PORT_BAUD_RATE_38400, (INT32)CLI_SERIAL_PORT_BAUD_RATE_38400,    (INT32)eGM_SERIAL_PORT_BAUD_RATE_38400},
    /* 06 */ {(INT32)eGUI_SERIAL_PORT_BAUD_RATE_57600,  (INT32)eCM_SERIAL_PORT_BAUD_RATE_57600, (INT32)CLI_SERIAL_PORT_BAUD_RATE_57600,    (INT32)eGM_SERIAL_PORT_BAUD_RATE_57600},
    /* 07 */ {(INT32)eGUI_SERIAL_PORT_BAUD_RATE_115200, (INT32)eCM_SERIAL_PORT_BAUD_RATE_115200,(INT32)CLI_SERIAL_PORT_BAUD_RATE_115200,   (INT32)eGM_SERIAL_PORT_BAUD_RATE_115200},
};

/////////////////////////////////////////////////////////////////////////////////////
// OSD Time Out
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_OSDTimeOut[] =
{
    /* 00 */ {(INT32)eGUI_OSD_TIMEOUT_OFF,       (INT32)eCM_OSD_TIMEOUT_OFF,      (INT32)CLI_OSD_TIMEOUT_OFF,      (INT32)eGM_OSD_TIMEOUT_OFF},
    /* 01 */ {(INT32)eGUI_OSD_TIMEOUT_5_SEC,     (INT32)eCM_OSD_TIMEOUT_5_SEC,    (INT32)CLI_OSD_TIMEOUT_5_SEC,    (INT32)eGM_OSD_TIMEOUT_5_SEC},
    /* 02 */ {(INT32)eGUI_OSD_TIMEOUT_10_SEC,    (INT32)eCM_OSD_TIMEOUT_10_SEC,   (INT32)CLI_OSD_TIMEOUT_10_SEC,   (INT32)eGM_OSD_TIMEOUT_10_SEC},
    /* 03 */ {(INT32)eGUI_OSD_TIMEOUT_20_SEC,    (INT32)eCM_OSD_TIMEOUT_20_SEC,   (INT32)CLI_OSD_TIMEOUT_20_SEC,   (INT32)eGM_OSD_TIMEOUT_20_SEC},
    /* 04 */ {(INT32)eGUI_OSD_TIMEOUT_30_SEC,    (INT32)eCM_OSD_TIMEOUT_30_SEC,   (INT32)CLI_OSD_TIMEOUT_30_SEC,   (INT32)eGM_OSD_TIMEOUT_30_SEC},
    /* 05 */ {(INT32)eGUI_OSD_TIMEOUT_60_SEC,    (INT32)eCM_OSD_TIMEOUT_60_SEC,   (INT32)CLI_OSD_TIMEOUT_60_SEC,   (INT32)eGM_OSD_TIMEOUT_60_SEC},
};

/////////////////////////////////////////////////////////////////////////////////////
// Back Ground Color
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_BackgroundColor[] =
{
    /* 00 */ {(INT32)eGUI_BACKGROUND_COLOR_BLACK,   (INT32)eCM_BACKGROUND_COLOR_BLACK,  (INT32)CLI_BACKGROUND_COLOR_BLACK,  (INT32)eGM_BACKGROUND_COLOR_BLACK},
    /* 01 */ {(INT32)eGUI_BACKGROUND_COLOR_BLUE,    (INT32)eCM_BACKGROUND_COLOR_BLUE,   (INT32)CLI_BACKGROUND_COLOR_BLUE,   (INT32)eGM_BACKGROUND_COLOR_BLUE},
    /* 02 */ {(INT32)eGUI_BACKGROUND_COLOR_RED,     (INT32)eCM_BACKGROUND_COLOR_RED,    (INT32)CLI_BACKGROUND_COLOR_RED,    (INT32)eGM_BACKGROUND_COLOR_RED},
    /* 03 */ {(INT32)eGUI_BACKGROUND_COLOR_GREEN,   (INT32)eCM_BACKGROUND_COLOR_GREEN,  (INT32)CLI_BACKGROUND_COLOR_GREEN,  (INT32)eGM_BACKGROUND_COLOR_GREEN},
    /* 04 */ {(INT32)eGUI_BACKGROUND_COLOR_GRAY,    (INT32)eCM_BACKGROUND_COLOR_GRAY,   (INT32)CLI_BACKGROUND_COLOR_GRAY,   (INT32)eGM_BACKGROUND_COLOR_GRAY},
    /* 05 */ {(INT32)eGUI_BACKGROUND_COLOR_WHITE,   (INT32)eCM_BACKGROUND_COLOR_WHITE,  (INT32)CLI_BACKGROUND_COLOR_WHITE,  (INT32)eGM_BACKGROUND_COLOR_WHITE},
    /* 06 */ {(INT32)eGUI_BACKGROUND_COLOR_LOGO,    (INT32)eCM_BACKGROUND_COLOR_LOGO,   (INT32)CLI_BACKGROUND_COLOR_LOGO,   (INT32)eGM_BACKGROUND_COLOR_LOGO},
};

/////////////////////////////////////////////////////////////////////////////////////
// Standby Mode
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_StandbyMode[] =
{
    /* 00 */ {(INT32)eGUI_STANDBY_MODE_ECO,             (INT32)eCM_STANDBY_MODE_0_5W,           (INT32)CLI_STANDBY_MODE_ECO,            (INT32)eGM_STANDBY_MODE_ECO},
    /* 01 */ {(INT32)eGUI_STANDBY_MODE_ACTIVE,          (INT32)eCM_STANDBY_MODE_2W,             (INT32)CLI_STANDBY_MODE_ACTIVE,         (INT32)eGM_STANDBY_MODE_ACTIVE},
    /* 02 */ {(INT32)eGUI_STANDBY_MODE_COMMUNICATION,   (INT32)eCM_STANDBY_MODE_COMMUNICATION,  (INT32)CLI_STANDBY_MODE_COMMUNICATION,  (INT32)eGM_STANDBY_MODE_COMMUNICATION},
};

/////////////////////////////////////////////////////////////////////////////////////
// Power Mode (Light Source Mode)
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_PowerMode[] =
{
    /* 00 */ {(INT32)eGUI_POWER_MODE_CONSTANT_POWER,    (INT32)eCM_POWER_MODE_CONSTANT_POWER,   (INT32)CLI_POWER_MODE_CONSTANT_POWER,   (INT32)eGM_POWER_MODE_CONSTANT_POWER},
    /* 01 */ {(INT32)eGUI_POWER_MODE_ECO,               (INT32)eCM_POWER_MODE_ECO1,             (INT32)CLI_POWER_MODE_ECO,              (INT32)eGM_POWER_MODE_ECO},
    /* 02 */ {(INT32)eGUI_POWER_MODE_CUSTOM_MODE,       (INT32)eCM_POWER_MODE_CUSTOM_MODE,      (INT32)CLI_POWER_MODE_CUSTOM_MODE,      (INT32)eGM_POWER_MODE_CUSTOM_MODE},
};

/////////////////////////////////////////////////////////////////////////////////////
// Splash Startup
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_SplashStartup[] =
{
    /* 00 */ {(INT32)eGUI_LOGO_PATTERN_FACTORY_LOGO,  (INT32)eCM_LOGO_PATTERN_FACTORY_LOGO, (INT32)CLI_LOGO_PATTERN_FACTORY_LOGO,  (INT32)eGM_LOGO_PATTERN_FACTORY_LOGO},
    /* 01 */ {(INT32)eGUI_LOGO_PATTERN_BLUE,          (INT32)eCM_LOGO_PATTERN_BLUE,         (INT32)CLI_LOGO_PATTERN_BLUE,          (INT32)eGM_LOGO_PATTERN_BLUE},
    /* 02 */ {(INT32)eGUI_LOGO_PATTERN_BLACK,         (INT32)eCM_LOGO_PATTERN_BLACK,        (INT32)CLI_LOGO_PATTERN_BLACK,         (INT32)eGM_LOGO_PATTERN_BLACK},
    /* 03 */ {(INT32)eGUI_LOGO_PATTERN_WHITE,         (INT32)eCM_LOGO_PATTERN_WHITE,        (INT32)CLI_LOGO_PATTERN_WHITE,         (INT32)eGM_LOGO_PATTERN_WHITE},
};

/////////////////////////////////////////////////////////////////////////////////////
// Low Latency
/////////////////////////////////////////////////////////////////////////////////////

static sMAPPING_TABLE_FORMAT asMPT_LowLatency[] =
{
    /* 00 */ {(INT32)eGUI_LOW_LATENCY_MODE_NORMAL,     (INT32)eCM_LOW_LATENCY_MODE_OFF,        (INT32)CLI_LOW_LATENCY_MODE_NORMAL,    (INT32)eGM_LOW_LATENCY_MODE_NORMAL},
    ///* 01 */ {(INT32)eGUI_LOW_LATENCY_MODE_2D_ULTRA,   (INT32)eCM_LOW_LATENCY_MODE_TYPICAL,    (INT32)CLI_LOW_LATENCY_MODE_2D_ULTRA,  (INT32)eGM_LOW_LATENCY_MODE_2D_ULTRA},
    /* 02 */ {(INT32)eGUI_LOW_LATENCY_MODE_ULTRA,      (INT32)eCM_LOW_LATENCY_MODE_TYPICAL,      (INT32)CLI_LOW_LATENCY_MODE_2D_ULTRA,     (INT32)eGM_LOW_LATENCY_MODE_ULTRA},
};

/////////////////////////////////////////////////////////////////////////////////////
// Detail
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_Detail[] =
{
    /* 00 */ {(INT32)eGUI_DETAIL_MAXIMUM,   (INT32)eCM_SHARPNESS_LEVEL15,       (INT32)CLI_DETAIL_MAXIMUM,   (INT32)eGM_DETAIL_MAXIMUM},
    /* 01 */ {(INT32)eGUI_DETAIL_HIGH,      (INT32)eCM_SHARPNESS_LEVEL12,       (INT32)CLI_DETAIL_HIGH,      (INT32)eGM_DETAIL_HIGH},
    /* 02 */ {(INT32)eGUI_DETAIL_NORMAL,    (INT32)eCM_SHARPNESS_LEVEL8_NORMAL, (INT32)CLI_DETAIL_NORMAL,    (INT32)eGM_DETAIL_NORMAL},
    /* 03 */ {(INT32)GUI_VALUE_INVALID,     (INT32)eCM_SHARPNESS_LEVEL5_LOW,    (INT32)CLI_VALUE_INVALID,    (INT32)eGM_DETAIL_LOW},
    /* 04 */ {(INT32)GUI_VALUE_INVALID,     (INT32)eCM_SHARPNESS_LEVEL1,        (INT32)CLI_VALUE_INVALID,    (INT32)eGM_DETAIL_MINIMUM},
};

/////////////////////////////////////////////////////////////////////////////////////
// HDR Level
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_HdrLevel[] =
{
    /* 00 */ {(INT32)eGUI_HDR_LEVEL_1,      (INT32)eCM_HDR_LEVEL1,       (INT32)CLI_HDR_LEVEL_1,   (INT32)eGM_HDR_LEVEL_1},
    /* 01 */ {(INT32)eGUI_HDR_LEVEL_2,      (INT32)eCM_HDR_LEVEL2,       (INT32)CLI_HDR_LEVEL_2,   (INT32)eGM_HDR_LEVEL_2},
    /* 02 */ {(INT32)eGUI_HDR_LEVEL_3,      (INT32)eCM_HDR_LEVEL3,       (INT32)CLI_HDR_LEVEL_3,   (INT32)eGM_HDR_LEVEL_3},
    /* 03 */ {(INT32)eGUI_HDR_LEVEL_4,      (INT32)eCM_HDR_LEVEL4,       (INT32)CLI_HDR_LEVEL_4,   (INT32)eGM_HDR_LEVEL_4},
};

/////////////////////////////////////////////////////////////////////////////////////
// DateTime ClockMode
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_ClockMode[] =
{
    /* 00 */ {(INT32)eGUI_CLOCK_MODE_USE_NTP_SERVER,      (INT32)eCM_DATETIME_CLOCK_MODE_NTP_SERVER,        (INT32)CLI_CLOCK_MODE_USE_NTP_SERVER,    (INT32)eGM_CLOCK_MODE_USE_NTP_SERVER},
    /* 01 */ {(INT32)eGUI_CLOCK_MODE_MANUAL,              (INT32)eCM_DATETIME_CLOCK_MODE_MANUAL,            (INT32)CLI_CLOCK_MODE_MANUAL,            (INT32)eGM_CLOCK_MODE_MANUAL},
};

/////////////////////////////////////////////////////////////////////////////////////
// DateTime Update Interval
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_UpdateInterval[] =
{
    /* 00 */ {(INT32)eGUI_UPDATE_INTERVAL_HOURLY,             (INT32)eCM_DATETIME_UPDATE_INTERVAL_HOURLY,   (INT32)CLI_UPDATE_INTERVAL_HOURLY,   (INT32)eGM_UPDATE_INTERVAL_HOURLY},
    /* 01 */ {(INT32)eGUI_UPDATE_INTERVAL_DAILY,              (INT32)eCM_DATETIME_UPDATE_INTERVAL_DAILY,    (INT32)CLI_UPDATE_INTERVAL_DAILY,    (INT32)eGM_UPDATE_INTERVAL_DAILY},
};

/////////////////////////////////////////////////////////////////////////////////////
// HDMI output
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_HDMIOutput[] =
{
    /* 00 */ {(INT32)eGUI_HDMI_OUT_HDMI1,               (INT32)eCM_HDMI_OUTPUT_HDMI1,     (INT32)CLI_HDMI_OUTPUT_HDMI1,    (INT32)eGM_HDMI_OUT_HDMI1},
    /* 01 */ {(INT32)eGUI_HDMI_OUT_HDMI2,               (INT32)eCM_HDMI_OUTPUT_HDMI2,     (INT32)CLI_HDMI_OUTPUT_HDMI2,    (INT32)eGM_HDMI_OUT_HDMI2},
};

/////////////////////////////////////////////////////////////////////////////////////
// COPY Event to Weekday
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_ScheduleCopyEventToWeekday[] =
{
    /* 00 */ {(INT32)eGUI_COPY_EVENT_TO_MONDAY,         (INT32)eCM_WEEKDAY_MONDAY,           (INT32)CLI_COPY_EVENT_TO_MONDAY,      (INT32)eGM_COPY_EVENT_TO_MONDAY},
    /* 01 */ {(INT32)eGUI_COPY_EVENT_TO_TUESDAY,        (INT32)eCM_WEEKDAY_TUESDAY,          (INT32)CLI_COPY_EVENT_TO_TUESDAY,     (INT32)eGM_COPY_EVENT_TO_TUESDAY},
    /* 02 */ {(INT32)eGUI_COPY_EVENT_TO_WEDNESDAY,      (INT32)eCM_WEEKDAY_WEDNESDAY,        (INT32)CLI_COPY_EVENT_TO_WEDNESDAY,   (INT32)eGM_COPY_EVENT_TO_WEDNESDAY},
    /* 03 */ {(INT32)eGUI_COPY_EVENT_TO_THURSDAY,       (INT32)eCM_WEEKDAY_THURSDAY,         (INT32)CLI_COPY_EVENT_TO_THURSDAY,    (INT32)eGM_COPY_EVENT_TO_THURSDAY},
    /* 04 */ {(INT32)eGUI_COPY_EVENT_TO_FRIDAY,         (INT32)eCM_WEEKDAY_FRIDAY,           (INT32)CLI_COPY_EVENT_TO_FRIDAY,      (INT32)eGM_COPY_EVENT_TO_FRIDAY},
    /* 05 */ {(INT32)eGUI_COPY_EVENT_TO_SATURDAY,       (INT32)eCM_WEEKDAY_SATURDAY,         (INT32)CLI_COPY_EVENT_TO_SATURDAY,    (INT32)eGM_COPY_EVENT_TO_SATURDAY},
    /* 06 */ {(INT32)eGUI_COPY_EVENT_TO_SUNDAY,         (INT32)eCM_WEEKDAY_SUNDAY,           (INT32)CLI_COPY_EVENT_TO_SUNDAY,      (INT32)eGM_COPY_EVENT_TO_SUNDAY},
};

/////////////////////////////////////////////////////////////////////////////////////
// 3D Mode
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_3DMode[] =	//A70Gen2_Doulas_0054
{
    /* 00 */ {(INT32)eGUI_3D_MODE_OFF,  (INT32)eCM_3D_MODE_OFF,         (INT32)CLI_3D_MODE_OFF,     (INT32)eGM_3D_MODE_OFF},
    /* 01 */ {(INT32)eGUI_3D_MODE_AUTO, (INT32)eCM_3D_MODE_ON,          (INT32)CLI_3D_MODE_AUTO,    (INT32)eGM_3D_MODE_AUTO},
};

/////////////////////////////////////////////////////////////////////////////////////
// Warp Memory Save
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_GeoMemorySave[] =
{
    /* 00 */ {(INT32)eGUI_GEO_MEMORY_SAVE_1,            (INT32)eCM_GEO_MEMORY_SAVE_1,           (INT32)CLI_GEO_SAVE_MEMORY1,        (INT32)eGM_GEO_MEMORY_SAVE_1},
    /* 01 */ {(INT32)eGUI_GEO_MEMORY_SAVE_2,            (INT32)eCM_GEO_MEMORY_SAVE_2,           (INT32)CLI_GEO_SAVE_MEMORY2,        (INT32)eGM_GEO_MEMORY_SAVE_2},
    /* 02 */ {(INT32)eGUI_GEO_MEMORY_SAVE_3,            (INT32)eCM_GEO_MEMORY_SAVE_3,           (INT32)CLI_GEO_SAVE_MEMORY3,        (INT32)eGM_GEO_MEMORY_SAVE_3},
    /* 03 */ {(INT32)eGUI_GEO_MEMORY_SAVE_4,            (INT32)eCM_GEO_MEMORY_SAVE_4,           (INT32)CLI_GEO_SAVE_MEMORY4,        (INT32)eGM_GEO_MEMORY_SAVE_4},
    /* 04 */ {(INT32)eGUI_GEO_MEMORY_SAVE_5,            (INT32)eCM_GEO_MEMORY_SAVE_5,           (INT32)CLI_GEO_SAVE_MEMORY5,        (INT32)eGM_GEO_MEMORY_SAVE_5},
};

/////////////////////////////////////////////////////////////////////////////////////
// Warp Memory Apply
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_GeoMemoryApply[] =
{
    /* 00 */ {(INT32)eGUI_GEO_MEMORY_APPLY_1,           (INT32)eCM_GEO_MEMORY_APPLY_1,          (INT32)CLI_GEO_APPLY_MEMORY1,       (INT32)eGM_GEO_MEMORY_APPLY_1},
    /* 01 */ {(INT32)eGUI_GEO_MEMORY_APPLY_2,           (INT32)eCM_GEO_MEMORY_APPLY_2,          (INT32)CLI_GEO_APPLY_MEMORY2,       (INT32)eGM_GEO_MEMORY_APPLY_2},
    /* 02 */ {(INT32)eGUI_GEO_MEMORY_APPLY_3,           (INT32)eCM_GEO_MEMORY_APPLY_3,          (INT32)CLI_GEO_APPLY_MEMORY3,       (INT32)eGM_GEO_MEMORY_APPLY_3},
    /* 03 */ {(INT32)eGUI_GEO_MEMORY_APPLY_4,           (INT32)eCM_GEO_MEMORY_APPLY_4,          (INT32)CLI_GEO_APPLY_MEMORY4,       (INT32)eGM_GEO_MEMORY_APPLY_4},
    /* 04 */ {(INT32)eGUI_GEO_MEMORY_APPLY_5,           (INT32)eCM_GEO_MEMORY_APPLY_5,          (INT32)CLI_GEO_APPLY_MEMORY5,       (INT32)eGM_GEO_MEMORY_APPLY_5},
};

/////////////////////////////////////////////////////////////////////////////////////
// Input Key
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_InputKey[] =
{
    /* 00 */ {(INT32)GUI_VALUE_INVALID,                 (INT32)eCM_SOURCE_KEY_CHANGE_SOURCE,    (INT32)CLI_VALUE_INVALID,               (INT32)eGM_SOURCE_KEY_CHANGE_SOURCE},
    /* 01 */ {(INT32)eGUI_SOURCE_KEY_LIST_ALL_SOURCE,   (INT32)eCM_SOURCE_KEY_LIST_ALL_SOURCE,  (INT32)CLI_SOURCE_KEY_LIST_ALL_SOURCE,  (INT32)eGM_SOURCE_KEY_LIST_ALL_SOURCE},
    /* 02 */ {(INT32)eGUI_SOURCE_KEY_AUTO_SOURCE,       (INT32)eCM_SOURCE_KEY_AUTO_SOURCE,      (INT32)CLI_SOURCE_KEY_AUTO_SOURCE,      (INT32)eGM_SOURCE_KEY_AUTO_SOURCE},
};

/////////////////////////////////////////////////////////////////////////////////////
// Change Logo
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_LogoChange[] =
{
    /* 00 */ {(INT32)eGUI_CHANGE_LOGO_DEFAULT,      (INT32)eCM_CHANGE_LOGO_DEFAULT,     (INT32)CLI_CHANGE_LOGO_DEFAULT,     (INT32)eGM_CHANGE_LOGO_DEFAULT},
    /* 01 */ {(INT32)eGUI_CHANGE_LOGO_NEUTRAL,      (INT32)eCM_CHANGE_LOGO_NEUTRAL,     (INT32)CLI_CHANGE_LOGO_NEUTRAL,     (INT32)eGM_CHANGE_LOGO_NEUTRAL},
    /* 02 */ {(INT32)eGUI_CHANGE_LOGO_USER,         (INT32)eCM_CHANGE_LOGO_USER,        (INT32)CLI_CHANGE_LOGO_USER,        (INT32)eGM_CHANGE_LOGO_USER},
    ///* 03 */ {(INT32)eGUI_CHANGE_LOGO_CAPTURED,     (INT32)eCM_CHANGE_LOGO_CAPTURED,    (INT32)CLI_CHANGE_LOGO_CAPTURED,    (INT32)eGM_CHANGE_LOGO_CAPTURED},
};

/////////////////////////////////////////////////////////////////////////////
///////////////////////////  Data Mapping LUT  ////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
sDATA_MAPPING_LUT_FORMAT sDATA_MAPPING_LUT[] =
{
    {edcMAIN_INPUT,  asMPT_SourceID,  MPT_SIZE(asMPT_SourceID)},
    {edcSUB_INPUT,   asMPT_SourceID,  MPT_SIZE(asMPT_SourceID)},
    {edcBACKUPINPUT_CURRENT_SOURCE,   asMPT_BackupInput_SourceID,  MPT_SIZE(asMPT_BackupInput_SourceID)},  //HICC2_Doulas_0027
    {edcBACKUPINPUT_PRIMARY_INPUT,    asMPT_BackupInput_SourceID,  MPT_SIZE(asMPT_BackupInput_SourceID)},   //HICC2_Doulas_0027//G100_Steven_0021
    {edcBACKUPINPUT_SECONDARY_INPUT,  asMPT_BackupInput_SourceID,  MPT_SIZE(asMPT_BackupInput_SourceID)}, //HICC2_Doulas_0027//G100_Steven_0021

    {edcOSDTEST_PATTERN,  asMPT_TestPattern,  MPT_SIZE(asMPT_TestPattern)},
    {edcSERVICE_TEST_PATTERN,  asMPT_ServiceTestPattern,  MPT_SIZE(asMPT_ServiceTestPattern)},
    {edcTWIST_PATTERN,  asMPT_TwistTestPattern,  MPT_SIZE(asMPT_TwistTestPattern)},

    {edcPICTURE_SETTINGS,  asMPT_PictureSettings,  MPT_SIZE(asMPT_PictureSettings)},
    {edcFORCE_PICTURE_SETTINGS,  asMPT_PictureSettings,  MPT_SIZE(asMPT_PictureSettings)},
    {edcPRE_USER_MODE,  asMPT_PictureSettings,  MPT_SIZE(asMPT_PictureSettings)},
    {edcUSER_COLOR_MODE,  asMPT_ApplyUserSettings,  MPT_SIZE(asMPT_ApplyUserSettings)}, 	//G100_Doulas_0066

    {edcLANGUAGE,  asMPT_Language,  MPT_SIZE(asMPT_Language)},

    {edcSIZE_PRESETS,  asMPT_ScalingMode,  MPT_SIZE(asMPT_ScalingMode)},
    {edcMAIN_ASPECT_RATIO,  asMPT_ScalingMode,  MPT_SIZE(asMPT_ScalingMode)},
    {edcSUB_ASPECT_RATIO,  asMPT_ScalingMode,  MPT_SIZE(asMPT_ScalingMode)},

    {edcHOT_KEY_SETTINGS,  asMPT_HotkeySettings,  MPT_SIZE(asMPT_HotkeySettings)},
    {edcHOT_KEY2_SETTINGS,  asMPT_HotkeySettings,  MPT_SIZE(asMPT_HotkeySettings)},

    {edcPIP_PBP_ENABLE,  asMPT_ScreenMode,  MPT_SIZE(asMPT_ScreenMode)},
    {edcSIZE,  asMPT_PIPSize,  MPT_SIZE(asMPT_PIPSize)},

    {edcMENU_LOCATION,  asMPT_MenuLocation,  MPT_SIZE(asMPT_MenuLocation)},

    {edcCEILING_MOUNT,  asMPT_CeilingMount,  MPT_SIZE(asMPT_CeilingMount)},

    {edcGAMMA,  asMPT_Gamma,  MPT_SIZE(asMPT_Gamma)},

    {edcLENS_SAVE_CURRENT_POSITION,  asMPT_LensMemorySave,  MPT_SIZE(asMPT_LensMemorySave)},
    {edcLENS_APPLY_POSITION,  (asMPT_LensMemoryApply),  MPT_SIZE(asMPT_LensMemoryApply)},

    {edcREAR_PROJECTION,  (asMPT_OrientationDirection),  MPT_SIZE(asMPT_OrientationDirection)},

    {edcHDMI_EDID_1,  asMPT_EDID_Type,  MPT_SIZE(asMPT_EDID_Type)},
    {edcHDMI_EDID_2,  asMPT_EDID_Type,  MPT_SIZE(asMPT_EDID_Type)},
    {edcHDBASET_EDID,  asMPT_EDID_Type,  MPT_SIZE(asMPT_EDID_Type)},

    {edcBLENDING_GAMMA,  asMPT_BlendingGamma,  MPT_SIZE(asMPT_BlendingGamma)},

    {edcCOLOR_TEMPERATURE,  asMPT_ColorTemperature,  MPT_SIZE(asMPT_ColorTemperature)},

    {edcCOLOR_SPACE,  asMPT_ColorSpace,  MPT_SIZE(asMPT_ColorSpace)},

    {edcWALL_COLOR,  asMPT_WallColor,  MPT_SIZE(asMPT_WallColor)},

    //{edc3D_MODE,  asMPT_3DMode,  MPT_SIZE(asMPT_3DMode)},
    {edc3D_ENABLE,  asMPT_3DFormat,  MPT_SIZE(asMPT_3DFormat)},
    {edcDLPLink_ON,  asMPT_3DTech,  MPT_SIZE(asMPT_3DTech)},
    {edc3D_2D,  asMPT_3D2DView,  MPT_SIZE(asMPT_3D2DView)},
    {edc3D_SYNC_OUT,  asMPT_3DSyncOut,  MPT_SIZE(asMPT_3DSyncOut)},
    {edc3D_LR_REFERENCE,  asMPT_3DLRReference,  MPT_SIZE(asMPT_3DLRReference)},
    {edc3D_SYNC_TYPE,  asMPT_3DSyncType,  MPT_SIZE(asMPT_3DSyncType)},

    {edcSIZE,  asMPT_PIPSize,  MPT_SIZE(asMPT_PIPSize)},
    {edcMAIN_LAYOUT,  asMPT_PIPLayout,  MPT_SIZE(asMPT_PIPLayout)},

    {edcLAN_PATH_SWITCH,  asMPT_LanPathSwitch,  MPT_SIZE(asMPT_LanPathSwitch)},

    {edcSERIAL_PORT_BAUD_RATE,  asMPT_SerialPortBaudRate,  MPT_SIZE(asMPT_SerialPortBaudRate)},
    {edcSERIAL_PORT_OUT_BAUD_RATE,  asMPT_SerialPortBaudRate,  MPT_SIZE(asMPT_SerialPortBaudRate)},

    {edcMENU_TIME_OUT,  asMPT_OSDTimeOut,  MPT_SIZE(asMPT_OSDTimeOut)},

    {edcBACKGROUND_COLOR,  asMPT_BackgroundColor,  MPT_SIZE(asMPT_BackgroundColor)},

    {edcSTANDBY_MODE,  asMPT_StandbyMode,  MPT_SIZE(asMPT_StandbyMode)},

    {edcPOWER_MODE,  asMPT_PowerMode,  MPT_SIZE(asMPT_PowerMode)},

    {edcSPLASH_STARTUP,  asMPT_SplashStartup,  MPT_SIZE(asMPT_SplashStartup)},

    {edcLOW_LATENCY_MODE,  asMPT_LowLatency,  MPT_SIZE(asMPT_LowLatency)},

    // 0~15, {edcDETAIL,  asMPT_Detail,  MPT_SIZE(asMPT_Detail)},

    {edcHDR_LEVEL,  asMPT_HdrLevel,  MPT_SIZE(asMPT_HdrLevel)},

    {edcDATE_MODE,  asMPT_ClockMode,  MPT_SIZE(asMPT_ClockMode)},

    {edcDATE_UPDATE_INTERVAL,  asMPT_UpdateInterval,  MPT_SIZE(asMPT_UpdateInterval)},

    {edcHDMI_OUT,  asMPT_HDMIOutput,  MPT_SIZE(asMPT_HDMIOutput)},

    {edcSCHEDULE_COPY_EVENT_INDEX, asMPT_ScheduleCopyEventToWeekday, MPT_SIZE(asMPT_ScheduleCopyEventToWeekday)},

	{edc3D_MODE,  asMPT_3DMode,  MPT_SIZE(asMPT_3DMode)},	//A70Gen2_Doulas_0054

    {edcWARP_MEMORY_SAVE,   asMPT_GeoMemorySave,   MPT_SIZE(asMPT_GeoMemorySave)},
    {edcWARP_MEMORY_APPLY,  asMPT_GeoMemoryApply,  MPT_SIZE(asMPT_GeoMemoryApply)},

    {edcBLEND_MEMORY_SAVE,   asMPT_GeoMemorySave,   MPT_SIZE(asMPT_GeoMemorySave)},
    {edcBLEND_MEMORY_APPLY,  asMPT_GeoMemoryApply,  MPT_SIZE(asMPT_GeoMemoryApply)},

    {edcINPUT_KEY, asMPT_InputKey, MPT_SIZE(asMPT_InputKey)},
    {edcLOGO_CHANGE, asMPT_LogoChange, MPT_SIZE(asMPT_LogoChange)},
};
#define DATA_MAPPING_LUT_SIZE (sizeof(sDATA_MAPPING_LUT)/sizeof(sDATA_MAPPING_LUT_FORMAT))

/////////////////////////////////////////////////////////////////////////////

INT32 utilDataMapping_Init(UINT8 IsCreator)
{
    if(IsCreator)
    {
        DataMapping_DebugModeSet(eDEBUG_FATAL_ERROR);
        DataMapping_Init(sDATA_MAPPING_LUT, DATA_MAPPING_LUT_SIZE);
    }
    else
    {
        DataMapping_DebugModeSet(eDEBUG_FATAL_ERROR);
        DataMapping_ReaderInit();
    }
}

INT32 utilDataMapping_DeInit(void)
{
    DataMapping_DeInit();
}

INT32 utilDataMapping_CMValueRangeCheck(eDATA_CODE eDataCode, INT32 lCM_Value)
{
    if(IS_COMMON_DATA_CODE(eDataCode))
    {
        if(DataMapping_DataRangeCheck(eDataCode, lCM_Value) == RANGE_CHECK_PASS)
        {
            return DATA_RANGE_CHECK_PASS;
        }
    }
    else  //not common data code
    {
        INT32 iMax = CommonAPI_DataCode_MaxValue_Get(eDataCode);
        INT32 iMin = CommonAPI_DataCode_MinValue_Get(eDataCode);

        if((iMax >= lCM_Value) && (lCM_Value >= iMin))
        {
            return DATA_RANGE_CHECK_PASS;
        }
    }

    return DATA_RANGE_CHECK_FAIL;
}

INT32 utilDataMapping_GuiValueRangeCheck(eDATA_CODE eDataCode, INT32 lGui_Value)
{
    if(IS_COMMON_DATA_CODE(eDataCode))
    {
        if( lGui_Value >= 0 && lGui_Value <= (GUI_ITEM_SIZE(eDataCode)-1))
        {
            return DATA_RANGE_CHECK_PASS;
        }
    }
    else  //not common data code
    {
        INT32 iMax = CommonAPI_DataCode_MaxValue_Get(eDataCode);
        INT32 iMin = CommonAPI_DataCode_MinValue_Get(eDataCode);

        if((iMax >= lGui_Value) && (lGui_Value >= iMin))
        {
            return DATA_RANGE_CHECK_PASS;
        }
    }

    return DATA_RANGE_CHECK_FAIL;
}

//end of defined(CUSTOM_OPTOMA)

#else

/////////////////////////////////////////////////////////////////////////////////////
// Source ID
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_SourceID[] =
{
    /* 00 */ {(INT32)eGUI_SOURCE_ID_HDMI1,      (INT32)eCM_SOURCE_HDMI1,        (INT32)CLI_SOURCE_HDMI1},
    /* 01 */ {(INT32)eGUI_SOURCE_ID_HDMI2,      (INT32)eCM_SOURCE_HDMI2,        (INT32)CLI_SOURCE_HDMI2},
    /* 02 */ {(INT32)eGUI_SOURCE_ID_DVI,        (INT32)eCM_SOURCE_DVI,          (INT32)CLI_SOURCE_DVI},		//A70Gen2_Doulas_0004 Modify
    /* 02 */ {(INT32)eGUI_SOURCE_ID_DP,         (INT32)eCM_SOURCE_DISPLAYPORT,  (INT32)CLI_SOURCE_DP},		//A70Gen2_Doulas_0004 Modify
    /* 03 */ {(INT32)eGUI_SOURCE_ID_3GSDI,      (INT32)eCM_SOURCE_3GSDI,        (INT32)CLI_SOURCE_3GSDI},
    /* 04 */ {(INT32)eGUI_SOURCE_ID_HDBASET,    (INT32)eCM_SOURCE_HDBASET,      (INT32)CLI_SOURCE_HDBASET},
};

/////////////////////////////////////////////////////////////////////////////////////
// Test Pattern
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_TestPattern[] =
{
    /* 00 */ {(INT32)eGUI_TEST_PATTERN_OFF,                 (INT32)eCM_TEST_PATTERN_OFF,            (INT32)CLI_TEST_PATTERN_OFF},
    /* 01 */ {(INT32)eGUI_TEST_PATTERN_GRID,                (INT32)eCM_TEST_PATTERN_GRID,           (INT32)CLI_TEST_PATTERN_GRID},
    /* 02 */ {(INT32)eGUI_TEST_PATTERN_WHITE,               (INT32)eCM_TEST_PATTERN_WHITE,          (INT32)CLI_TEST_PATTERN_WHITE},
    /* 03 */ {(INT32)eGUI_TEST_PATTERN_BLACK,               (INT32)eCM_TEST_PATTERN_BLACK,          (INT32)CLI_TEST_PATTERN_BLACK},
    /* 04 */ {(INT32)eGUI_TEST_PATTERN_CHECKERBOARD,        (INT32)eCM_TEST_PATTERN_CHECKERBOARD,   (INT32)CLI_TEST_PATTERN_CHECKERBOARD},
    /* 05 */ {(INT32)eGUI_TEST_PATTERN_COLORBAR,            (INT32)eCM_TEST_PATTERN_COLORBAR,       (INT32)CLI_TEST_PATTERN_COLORBAR},
};

/////////////////////////////////////////////////////////////////////////////////////
// Service Test Pattern
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_ServiceTestPattern[] =
{
    /* 00 */ {(INT32)eGUI_SERVICE_TEST_PATTERN_OFF,           	(INT32)eCM_TEST_PATTERN_OFF,            	(INT32)CLI_SERVICE_TEST_PATTERN_OFF},
    /* 01 */ {(INT32)eGUI_SERVICE_TEST_PATTERN_GRID,          	(INT32)eCM_TEST_PATTERN_GRID,     			(INT32)CLI_SERVICE_TEST_PATTERN_GRID},
    /* 02 */ {(INT32)eGUI_SERVICE_TEST_PATTERN_WHITE,        	(INT32)eCM_TEST_PATTERN_WHITE, 	 	 		(INT32)CLI_SERVICE_TEST_PATTERN_WHITE},
    /* 03 */ {(INT32)eGUI_SERVICE_TEST_PATTERN_BLACK,          	(INT32)eCM_TEST_PATTERN_BLACK,     			(INT32)CLI_SERVICE_TEST_PATTERN_BLACK},
    /* 04 */ {(INT32)eGUI_SERVICE_TEST_PATTERN_CHECKBOARD, 		(INT32)eCM_TEST_PATTERN_CHECKERBOARD,	  	(INT32)CLI_SERVICE_TEST_PATTERN_CHECKBOARD},
    /* 05 */ {(INT32)eGUI_SERVICE_TEST_PATTERN_COLORBAR,     	(INT32)eCM_TEST_PATTERN_COLORBAR,         	(INT32)CLI_SERVICE_TEST_PATTERN_COLORBAR},
    /* 06 */ {(INT32)eGUI_SERVICE_TEST_PATTERN_RED,             (INT32)eCM_TEST_PATTERN_RED,            	(INT32)CLI_SERVICE_TEST_PATTERN_RED},
    /* 07 */ {(INT32)eGUI_SERVICE_TEST_PATTERN_GREEN,           (INT32)eCM_TEST_PATTERN_GREEN,          	(INT32)CLI_SERVICE_TEST_PATTERN_GREEN},
    /* 08 */ {(INT32)eGUI_SERVICE_TEST_PATTERN_BLUE,            (INT32)eCM_TEST_PATTERN_BLUE,           	(INT32)CLI_SERVICE_TEST_PATTERN_BLUE},
    /* 09 */ {(INT32)eGUI_SERVICE_TEST_PATTERN_YELLOW,      	(INT32)eCM_TEST_PATTERN_YELLOW,         	(INT32)CLI_SERVICE_TEST_PATTERN_YELLOW},
    /* 10 */ {(INT32)eGUI_SERVICE_TEST_PATTERN_MAGENTA,   		(INT32)eCM_TEST_PATTERN_MAGENTA,        	(INT32)CLI_SERVICE_TEST_PATTERN_MAGENTA},
    /* 11 */ {(INT32)eGUI_SERVICE_TEST_PATTERN_CYAN,            (INT32)eCM_TEST_PATTERN_CYAN,           	(INT32)CLI_SERVICE_TEST_PATTERN_CYAN},
    /* 12 */ {(INT32)eGUI_SERVICE_TEST_PATTERN_BORESIGHT, 	 	(INT32)eCM_TEST_PATTERN_BORESIGHT,	 	  	(INT32)CLI_SERVICE_TEST_PATTERN_BORESIGHT},
    /* 13 */ {(INT32)eGUI_SERVICE_TEST_PATTERN_FULL_SCREEN, 	(INT32)eCM_TEST_PATTERN_FULL_SCREEN,       	(INT32)CLI_SERVICE_TEST_PATTERN_FULL_SCREEN},
    /* 14 */ {(INT32)eGUI_SERVICE_TEST_PATTERN_13POINTS,   		(INT32)eCM_TEST_PATTERN_13POINTS, 	   		(INT32)CLI_SERVICE_TEST_PATTERN_13POINTS},
};

/////////////////////////////////////////////////////////////////////////////////////
// Twist Test Pattern
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_TwistTestPattern[] =
{
    /* 00 */ {(INT32)eGUI_TWIST_TEST_PATTERN_OFF,           (INT32)eCM_TEST_PATTERN_OFF,            (INT32)CLI_TWIST_TEST_PATTERN_OFF},
    /* 01 */ {(INT32)eGUI_TWIST_TEST_PATTERN_GRID,          (INT32)eCM_TEST_PATTERN_GRID,           (INT32)CLI_TWIST_TEST_PATTERN_GRID},
    /* 02 */ {(INT32)eGUI_TWIST_TEST_PATTERN_WHITE,         (INT32)eCM_TEST_PATTERN_WHITE,          (INT32)CLI_TWIST_TEST_PATTERN_WHITE},
    /* 03 */ {(INT32)eGUI_TWIST_TEST_PATTERN_BLACK,         (INT32)eCM_TEST_PATTERN_BLACK,          (INT32)CLI_TWIST_TEST_PATTERN_BLACK},
    /* 04 */ {(INT32)eGUI_TWIST_TEST_PATTERN_CHECKERBOARD,  (INT32)eCM_TEST_PATTERN_CHECKERBOARD,   (INT32)CLI_TWIST_TEST_PATTERN_CHECKERBOARD},
    /* 05 */ {(INT32)eGUI_TWIST_TEST_PATTERN_COLORBAR,      (INT32)eCM_TEST_PATTERN_COLORBAR,       (INT32)CLI_TWIST_TEST_PATTERN_COLORBAR},
    /* 06 */ {(INT32)eGUI_TWIST_TEST_PATTERN_GRAY,          (INT32)eCM_TEST_PATTERN_GRAY,           (INT32)CLI_TWIST_TEST_PATTERN_GRAY},
};

/////////////////////////////////////////////////////////////////////////////////////
// Picture Settings
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_PictureSettings[] =
{
	/* 00 */ {(INT32)eGUI_PICTURE_SETTINGS_VIDEO,    		(INT32)eCM_PICTURE_SETTINGS_VIDEO,   		(INT32)CLI_PICTURE_SETTINGS_VIDEO},
	/* 01 */ {(INT32)eGUI_PICTURE_SETTINGS_BRIGHT,		    (INT32)eCM_PICTURE_SETTINGS_BRIGHT,		    (INT32)CLI_PICTURE_SETTINGS_BRIGHT},
	/* 02 */ {(INT32)eGUI_PICTURE_SETTINGS_ENHANCED,        (INT32)eCM_PICTURE_SETTINGS_ENHANCED,	    (INT32)CLI_PICTURE_SETTINGS_ENHANCED},
	/* 03 */ {(INT32)eGUI_PICTURE_SETTINGS_REC709,		    (INT32)eCM_PICTURE_SETTINGS_REC709,		    (INT32)CLI_PICTURE_SETTINGS_REC709},
	/* 04 */ {(INT32)eGUI_PICTURE_SETTINGS_DICOMSIM,        (INT32)eCM_PICTURE_SETTINGS_DICOMSIM,	    (INT32)CLI_PICTURE_SETTINGS_DICOMSIM},
	/* 05 */ {(INT32)eGUI_PICTURE_SETTINGS_BLENDING,        (INT32)eCM_PICTURE_SETTINGS_BLENDING,	    (INT32)CLI_PICTURE_SETTINGS_BLENDING},
	/* 06 */ {(INT32)eGUI_PICTURE_SETTINGS_HDR,	    		(INT32)eCM_PICTURE_SETTINGS_HDR,	    	(INT32)CLI_PICTURE_SETTINGS_HDR},
	/* 07 */ {(INT32)eGUI_PICTURE_SETTINGS_3D,	    		(INT32)eCM_PICTURE_SETTINGS_3D,	    		(INT32)CLI_PICTURE_SETTINGS_3D},
	/* 08 */ {(INT32)eGUI_PICTURE_SETTINGS_2D_HIGH_SPEED,	(INT32)eCM_PICTURE_SETTINGS_2DHIGHSPEED,	(INT32)CLI_PICTURE_SETTINGS_2D_HIGH_SPEED},
	/* 09 */ {(INT32)eGUI_PICTURE_SETTINGS_USER,	        (INT32)eCM_PICTURE_SETTINGS_USER,		    (INT32)CLI_PICTURE_SETTINGS_USER},
};

/////////////////////////////////////////////////////////////////////////////////////
// Apply User Settings
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_ApplyUserSettings[] =				//G100_Doulas_0066
{
	/* 00 */ {(INT32)eGUI_PRE_USER_VIDEO,		    (INT32)eCM_PICTURE_SETTINGS_VIDEO,		    (INT32)CLI_PRE_USER_VIDEO},
	/* 01 */ {(INT32)eGUI_PRE_USER_BRIGHT,		    (INT32)eCM_PICTURE_SETTINGS_BRIGHT,		    (INT32)CLI_PRE_USER_BRIGHT},
	/* 02 */ {(INT32)eGUI_PRE_USER_ENHANCED,        (INT32)eCM_PICTURE_SETTINGS_ENHANCED,	    (INT32)CLI_PRE_USER_ENHANCED},
	/* 03 */ {(INT32)eGUI_PRE_USER_REC709,		    (INT32)eCM_PICTURE_SETTINGS_REC709,		    (INT32)CLI_PRE_USER_REC709},
	/* 04 */ {(INT32)eGUI_PRE_USER_DICOMSIM,	    (INT32)eCM_PICTURE_SETTINGS_DICOMSIM,	    (INT32)CLI_PRE_USER_DICOMSIM},
	/* 05 */ {(INT32)eGUI_PRE_USER_BLENDING,	    (INT32)eCM_PICTURE_SETTINGS_BLENDING,	    (INT32)CLI_PRE_USER_BLENDING},
	/* 06 */ {(INT32)eGUI_PRE_USER_HDR,			    (INT32)eCM_PICTURE_SETTINGS_HDR,		    (INT32)CLI_PRE_USER_HDR},
	/* 07 */ {(INT32)eGUI_PRE_USER_3D,			    (INT32)eCM_PICTURE_SETTINGS_3D,			    (INT32)CLI_PRE_USER_3D},
	/* 08 */ {(INT32)eGUI_PRE_USER_2D_HIGH_SPEED,   (INT32)eCM_PICTURE_SETTINGS_2DHIGHSPEED,    (INT32)CLI_PRE_USER_2D_HIGH_SPEED},
};


/////////////////////////////////////////////////////////////////////////////////////
// Language
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_Language[] =
{
    /* 00 */ {(INT32)eGUI_LANGUAGE_ENGLISH,     (INT32)eCM_LANGUAGE_ENGLISH,    (INT32)CLI_LANGUAGE_ENGLISH},
    /* 01 */ {(INT32)eGUI_LANGUAGE_FRENCH,      (INT32)eCM_LANGUAGE_FRENCH,     (INT32)CLI_LANGUAGE_FRENCH},
    /* 02 */ {(INT32)eGUI_LANGUAGE_SPANISH,     (INT32)eCM_LANGUAGE_SPANISH,    (INT32)CLI_LANGUAGE_SPANISH},
    /* 03 */ {(INT32)eGUI_LANGUAGE_GERMAN,      (INT32)eCM_LANGUAGE_GERMAN,     (INT32)CLI_LANGUAGE_GERMAN},
    /* 04 */ {(INT32)eGUI_LANGUAGE_ITALIAN,     (INT32)eCM_LANGUAGE_ITALIAN,    (INT32)CLI_LANGUAGE_ITALIAN},
    /* 05 */ {(INT32)eGUI_LANGUAGE_POLSKI,      (INT32)eCM_LANGUAGE_POLSKI,     (INT32)CLI_LANGUAGE_POLSKI},
    /* 06 */ {(INT32)eGUI_LANGUAGE_RUSSIAN,     (INT32)eCM_LANGUAGE_RUSSIAN,    (INT32)CLI_LANGUAGE_RUSSIAN},
    /* 07 */ {(INT32)eGUI_LANGUAGE_S_CHINESE,   (INT32)eCM_LANGUAGE_S_CHINESE,  (INT32)CLI_LANGUAGE_S_CHINESE},
    /* 08 */ {(INT32)eGUI_LANGUAGE_T_CHINESE,   (INT32)eCM_LANGUAGE_T_CHINESE,  (INT32)CLI_LANGUAGE_T_CHINESE},
    /* 09 */ {(INT32)eGUI_LANGUAGE_JAPANESE,    (INT32)eCM_LANGUAGE_JAPANESE,   (INT32)CLI_LANGUAGE_JAPANESE},
    /* 10 */ {(INT32)eGUI_LANGUAGE_KOREAN,      (INT32)eCM_LANGUAGE_KOREAN,     (INT32)CLI_LANGUAGE_KOREAN},
};

/////////////////////////////////////////////////////////////////////////////////////
// Size Presets
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_ScalingMode[] =
{
    /* 00 */ {(INT32)eGUI_SCALING_MODE_AUTO,        (INT32)eCM_SCALING_MODE_AUTO,       (INT32)CLI_SCALING_MODE_AUTO},
    /* 01 */ {(INT32)eGUI_SCALING_MODE_NATIVE,      (INT32)eCM_SCALING_MODE_NATIVE,     (INT32)CLI_SCALING_MODE_NATIVE},
    /* 02 */ {(INT32)eGUI_SCALING_MODE_4_3,         (INT32)eCM_SCALING_MODE_4_3,        (INT32)CLI_SCALING_MODE_4_3},
    /* 03 */ {(INT32)eGUI_SCALING_MODE_LETTER_BOX,  (INT32)eCM_SCALING_MODE_LETTER_BOX, (INT32)CLI_SCALING_MODE_LETTER_BOX},
    /* 04 */ {(INT32)eGUI_SCALING_MODE_FULL_SIZE,   (INT32)eCM_SCALING_MODE_FULL_SIZE,  (INT32)CLI_SCALING_MODE_FULL_SIZE},
    /* 05 */ {(INT32)eGUI_SCALING_MODE_WIDTH,       (INT32)eCM_SCALING_MODE_FULL_WIDTH, (INT32)CLI_SCALING_MODE_FULL_WIDTH},
    /* 06 */ {(INT32)eGUI_SCALING_MODE_HEIGHT,      (INT32)eCM_SCALING_MODE_FULL_HEIGHT,(INT32)CLI_SCALING_MODE_FULL_HEIGHT},
    /* 07 */ {(INT32)eGUI_SCALING_MODE_CUSTOM,      (INT32)eCM_SCALING_MODE_CUSTOM,     (INT32)CLI_SCALING_MODE_CUSTOM},
    /* 08 */ {(INT32)eGUI_SCALING_MODE_3D,          (INT32)eCM_SCALING_MODE_3D,         (INT32)CLI_SCALING_MODE_3D},
    /* 09 */ {(INT32)eGUI_SCALING_MODE_21_9,        (INT32)eCM_SCALING_MODE_21_9,       (INT32)CLI_SCALING_MODE_21_9}, //HICC2_Doulas_0065
};

/////////////////////////////////////////////////////////////////////////////////////
// Hotkey Settings
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_HotkeySettings[] =
{
    /* 00 */ {(INT32)eGUI_HOTKEY_SETTINGS_SIZE_PRESETS,         (INT32)eCM_HOTKEY_SETTINGS_SIZE_PRESETS,            (INT32)CLI_HOTKEY_SETTINGS_SIZE_PRESETS},
    /* 01 */ {(INT32)eGUI_HOTKEY_SETTINGS_PICTURE_SETTINGS,     (INT32)eCM_HOTKEY_SETTINGS_PICTURE_SETTINGS,        (INT32)CLI_HOTKEY_SETTINGS_PICTURE_SETTINGS},
    /* 02 */ {(INT32)eGUI_HOTKEY_SETTINGS_DEFAULT_VALUE,        (INT32)eCM_HOTKEY_SETTINGS_DEFAULT_VALUE,           (INT32)CLI_HOTKEY_SETTINGS_DEFAULT_VALUE},
    /* 03 */ {(INT32)eGUI_HOTKEY_SETTINGS_DETAIL,               (INT32)eCM_HOTKEY_SETTINGS_DETAIL,                  (INT32)CLI_HOTKEY_SETTINGS_DETAIL},
    /* 04 */ {(INT32)eGUI_HOTKEY_SETTINGS_CONTRAST_ENHANCEMENT, (INT32)eCM_HOTKEY_SETTINGS_CONTRAST_ENHANCEMENT,    (INT32)CLI_HOTKEY_SETTINGS_CONTRAST_ENHANCEMENT},
    /* 05 */ {(INT32)eGUI_HOTKEY_SETTINGS_LIGHT_SOURCE_MODE,    (INT32)eCM_HOTKEY_SETTINGS_LIGHT_SOURCE_MODE,       (INT32)CLI_HOTKEY_SETTINGS_LIGHT_SOURCE_MODE},
    /* 06 */ {(INT32)eGUI_HOTKEY_SETTINGS_FREEZE_SCREEN,        (INT32)eCM_HOTKEY_SETTINGS_FREEZE_SCREEN,           (INT32)CLI_HOTKEY_SETTINGS_FREEZE_SCREEN},
    /* 07 */ {(INT32)eGUI_HOTKEY_SETTINGS_INFORMATION,          (INT32)eCM_HOTKEY_SETTINGS_INFORMATION,             (INT32)CLI_HOTKEY_SETTINGS_INFORMATION},
    /* 08 */ {(INT32)eGUI_HOTKEY_SETTINGS_WARPBLEND_DISABLE,    (INT32)eCM_HOTKEY_SETTINGS_WARPBLEND_DISABLE,       (INT32)CLI_HOTKEY_SETTINGS_WARPBLEND_DISABLE},
};

/////////////////////////////////////////////////////////////////////////////////////
// Screen Mode (PIP/PBP)
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_ScreenMode[] =
{
    /* 00 */ {(INT32)eGUI_SCREEN_MODE_OFF, (INT32)eCM_SCREEN_MODE_OFF, (INT32)CLI_SCREEN_MODE_OFF},
    /* 01 */ {(INT32)eGUI_SCREEN_MODE_PIP, (INT32)eCM_SCREEN_MODE_PIP, (INT32)CLI_SCREEN_MODE_PIP},
    /* 02 */ {(INT32)eGUI_SCREEN_MODE_PBP, (INT32)eCM_SCREEN_MODE_PBP, (INT32)CLI_SCREEN_MODE_PBP},
};

/////////////////////////////////////////////////////////////////////////////////////
// Menu Location
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_MenuLocation[] =
{
    /* 00 */ {(INT32)eGUI_MENU_LOCATION_TOP_LEFT,       (INT32)eCM_MENU_LOCATION_TOP_LEFT,      (INT32)CLI_MENU_LOCATION_TOP_LEFT},
    /* 01 */ {(INT32)eGUI_MENU_LOCATION_TOP_RIGHT,      (INT32)eCM_MENU_LOCATION_TOP_RIGHT,     (INT32)CLI_MENU_LOCATION_TOP_RIGHT},
    /* 02 */ {(INT32)eGUI_MENU_LOCATION_CENTER,         (INT32)eCM_MENU_LOCATION_CENTER,        (INT32)CLI_MENU_LOCATION_CENTER},
    /* 03 */ {(INT32)eGUI_MENU_LOCATION_BOTTOM_LEFT,    (INT32)eCM_MENU_LOCATION_BOTTOM_LEFT,   (INT32)CLI_MENU_LOCATION_BOTTOM_LEFT},
    /* 04 */ {(INT32)eGUI_MENU_LOCATION_BOTTOM_RIGHT,   (INT32)eCM_MENU_LOCATION_BOTTOM_RIGHT,  (INT32)CLI_MENU_LOCATION_BOTTOM_RIGHT},
};

/////////////////////////////////////////////////////////////////////////////////////
// OverScan
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_OverScan[] =
{
    /* 00 */ {(INT32)eGUI_OVERSCAN_OFF,  (INT32)eCM_OVERSCAN_OFF,  (INT32)CLI_OVERSCAN_OFF},
    /* 01 */ {(INT32)eGUI_OVERSCAN_ZOOM, (INT32)eCM_OVERSCAN_ZOOM, (INT32)CLI_OVERSCAN_ZOOM},
    /* 02 */ {(INT32)eGUI_OVERSCAN_CROP, (INT32)eCM_OVERSCAN_CROP, (INT32)CLI_OVERSCAN_CROP},
};


/////////////////////////////////////////////////////////////////////////////////////
// Ceiling Mount
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_CeilingMount[] =
{
    /* 00 */  {(INT32)eGUI_CEILING_MOUNT_OFF,    (INT32)eCM_CEILING_MOUNT_OFF,   (INT32)CLI_CEILING_MOUNT_OFF},
    /* 01 */  {(INT32)eGUI_CEILING_MOUNT_ON,     (INT32)eCM_CEILING_MOUNT_ON,    (INT32)CLI_CEILING_MOUNT_ON},
    /* 02 */  {(INT32)eGUI_CEILING_MOUNT_AUTO,   (INT32)eCM_CEILING_MOUNT_AUTO,  (INT32)CLI_CEILING_MOUNT_AUTO},
};

/////////////////////////////////////////////////////////////////////////////////////
// Gamma
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_Gamma[] =   //G100_Simon_0022
{
    /* 00 */ {(INT32)eGUI_GAMMA_VIDEO,          (INT32)eCM_GAMMA_VIDEO,         (INT32)CLI_GAMMA_VIDEO},
    /* 01 */ {(INT32)eGUI_GAMMA_FILM,           (INT32)eCM_GAMMA_FILM,          (INT32)CLI_GAMMA_FILM},
    /* 02 */ {(INT32)eGUI_GAMMA_BRIGHT,         (INT32)eCM_GAMMA_2_2,           (INT32)CLI_GAMMA_BRIGHT},
    /* 03 */ {(INT32)eGUI_GAMMA_CRT,            (INT32)eCM_GAMMA_CRT,           (INT32)CLI_GAMMA_CRT},
    /* 04 */ {(INT32)eGUI_GAMMA_DICOM,          (INT32)eCM_GAMMA_DICOM,         (INT32)CLI_GAMMA_DICOM},
    /* 05 */ {(INT32)eGUI_GAMMA_HDR,            (INT32)eCM_GAMMA_HDR_STANDARD,  (INT32)CLI_GAMMA_HDR}, //A35G2_CDS_Coda_0015

};

/////////////////////////////////////////////////////////////////////////////////////
// Lens Memory Save
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_LensMemorySave[] =
{
    /* 00 */ {(INT32)eGUI_LENS_MEMORY_SAVE_1, (INT32)eCM_LENS_MEMORY_SAVE_1, (INT32)CLI_LENS_MEMORY_SAVE_1},
    /* 01 */ {(INT32)eGUI_LENS_MEMORY_SAVE_2, (INT32)eCM_LENS_MEMORY_SAVE_2, (INT32)CLI_LENS_MEMORY_SAVE_2},
    /* 02 */ {(INT32)eGUI_LENS_MEMORY_SAVE_3, (INT32)eCM_LENS_MEMORY_SAVE_3, (INT32)CLI_LENS_MEMORY_SAVE_3},
    /* 03 */ {(INT32)eGUI_LENS_MEMORY_SAVE_4, (INT32)eCM_LENS_MEMORY_SAVE_4, (INT32)CLI_LENS_MEMORY_SAVE_4},
    /* 04 */ {(INT32)eGUI_LENS_MEMORY_SAVE_5, (INT32)eCM_LENS_MEMORY_SAVE_5, (INT32)CLI_LENS_MEMORY_SAVE_5},
};

/////////////////////////////////////////////////////////////////////////////////////
// Lens Memory Apply
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_LensMemoryApply[] =
{
    /* 00 */ {(INT32)eGUI_LENS_MEMORY_APPLY_1, (INT32)eCM_LENS_MEMORY_APPLY_1, (INT32)CLI_LENS_MEMORY_APPLY_1},
    /* 01 */ {(INT32)eGUI_LENS_MEMORY_APPLY_2, (INT32)eCM_LENS_MEMORY_APPLY_2, (INT32)CLI_LENS_MEMORY_APPLY_2},
    /* 02 */ {(INT32)eGUI_LENS_MEMORY_APPLY_3, (INT32)eCM_LENS_MEMORY_APPLY_3, (INT32)CLI_LENS_MEMORY_APPLY_3},
    /* 03 */ {(INT32)eGUI_LENS_MEMORY_APPLY_4, (INT32)eCM_LENS_MEMORY_APPLY_4, (INT32)CLI_LENS_MEMORY_APPLY_4},
    /* 04 */ {(INT32)eGUI_LENS_MEMORY_APPLY_5, (INT32)eCM_LENS_MEMORY_APPLY_5, (INT32)CLI_LENS_MEMORY_APPLY_5},
};

/////////////////////////////////////////////////////////////////////////////////////
// Orientation Direction
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_OrientationDirection[] =
{
    /* 00 */ {(INT32)eGUI_REAR_PROJECTION_FRONT, (INT32)eCM_REAR_PROJECTION_FRONT, (INT32)CLI_REAR_PROJECTION_FRONT},
    /* 01 */ {(INT32)eGUI_REAR_PROJECTION_REAR,  (INT32)eCM_REAR_PROJECTION_REAR,  (INT32)CLI_REAR_PROJECTION_REAR},
};

/////////////////////////////////////////////////////////////////////////////////////
// EDID Type
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_EDID_Type[] =
{
    /* 00 */ {(INT32)eGUI_EDID_TYPE_V14, (INT32)eCM_EDID_TYPE_V14, (INT32)CLI_EDID_TYPE_V14},
    /* 01 */ {(INT32)eGUI_EDID_TYPE_V20, (INT32)eCM_EDID_TYPE_V20, (INT32)CLI_EDID_TYPE_V20},
#if (CUSTOMIZED_EDID_ENABLE == TRUE) //HICC2_Doulas_0068
    /* 02 */ {(INT32)eGUI_EDID_TYPE_CUSTOMIZED, (INT32)eCM_EDID_TYPE_CUSTOMIZED, (INT32)CLI_EDID_TYPE_CUSTOMIZED},
#endif
};

static sMAPPING_TABLE_FORMAT asMPT_EDID_Type2[] =
{
    /* 00 */ {(INT32)eGUI_EDID_TYPE_V14, (INT32)eCM_EDID_TYPE_V14, (INT32)CLI_EDID_TYPE_V14},
    /* 01 */ {(INT32)eGUI_EDID_TYPE_V20, (INT32)eCM_EDID_TYPE_V20, (INT32)CLI_EDID_TYPE_V20},
};


/////////////////////////////////////////////////////////////////////////////////////
// Blending Gamma
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_BlendingGamma[] =
{
    /* 00 */ {(INT32)eGUI_BLENDING_GAMMA_1_8, (INT32)eCM_BLENDING_GAMMA_1_8, (INT32)CLI_BLENDING_GAMMA_1_8},
    /* 01 */ {(INT32)eGUI_BLENDING_GAMMA_1_9, (INT32)eCM_BLENDING_GAMMA_1_9, (INT32)CLI_BLENDING_GAMMA_1_9},
    /* 02 */ {(INT32)eGUI_BLENDING_GAMMA_2_0, (INT32)eCM_BLENDING_GAMMA_2_0, (INT32)CLI_BLENDING_GAMMA_2_0},
    /* 03 */ {(INT32)eGUI_BLENDING_GAMMA_2_1, (INT32)eCM_BLENDING_GAMMA_2_1, (INT32)CLI_BLENDING_GAMMA_2_1},
    /* 04 */ {(INT32)eGUI_BLENDING_GAMMA_2_2, (INT32)eCM_BLENDING_GAMMA_2_2, (INT32)CLI_BLENDING_GAMMA_2_2},
    /* 05 */ {(INT32)eGUI_BLENDING_GAMMA_2_3, (INT32)eCM_BLENDING_GAMMA_2_3, (INT32)CLI_BLENDING_GAMMA_2_3},
    /* 06 */ {(INT32)eGUI_BLENDING_GAMMA_2_4, (INT32)eCM_BLENDING_GAMMA_2_4, (INT32)CLI_BLENDING_GAMMA_2_4},
};

/////////////////////////////////////////////////////////////////////////////////////
// Color Temperature
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_ColorTemperature[] =
{
    /* 00 */ {(INT32)eGUI_COLOR_TEMPERATURE_WARMEST, (INT32)eCM_COLOR_TEMPERATURE_WARMEST,  (INT32)CLI_COLOR_TEMPERATURE_WARMEST},
    /* 01 */ {(INT32)eGUI_COLOR_TEMPERATURE_WARM,    (INT32)eCM_COLOR_TEMPERATURE_WARM,     (INT32)CLI_COLOR_TEMPERATURE_WARM},
    /* 02 */ {(INT32)eGUI_COLOR_TEMPERATURE_COOL,    (INT32)eCM_COLOR_TEMPERATURE_COOL,     (INT32)CLI_COLOR_TEMPERATURE_COOL},
};

/////////////////////////////////////////////////////////////////////////////////////
// Color Space
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_ColorSpace[] =
{
    /* 00 */ {(INT32)eGUI_COLOR_SPACE_AUTO,         (INT32)eCM_COLOR_SPACE_AUTO,        (INT32)CLI_COLOR_SPACE_AUTO},
    /* 01 */ {(INT32)eGUI_COLOR_SPACE_RGB_FULL,     (INT32)eCM_COLOR_SPACE_RGB_FULL,    (INT32)CLI_COLOR_SPACE_RGB_FULL},
    /* 02 */ {(INT32)eGUI_COLOR_SPACE_RGB_LIMITED,  (INT32)eCM_COLOR_SPACE_RGB_LIMITED, (INT32)CLI_COLOR_SPACE_RGB_LIMITED},
    /* 03 */ {(INT32)eGUI_COLOR_SPACE_YUV_REC709,   (INT32)eCM_COLOR_SPACE_YUV_REC709,  (INT32)CLI_COLOR_SPACE_YUV_REC709},
    /* 04 */ {(INT32)eGUI_COLOR_SPACE_YUV_REC601,   (INT32)eCM_COLOR_SPACE_YUV_REC601,  (INT32)CLI_COLOR_SPACE_YUV_REC601},
};

/////////////////////////////////////////////////////////////////////////////////////
// Wall Color
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_WallColor[] =
{
    /* 00 */ {(INT32)eGUI_WALL_COLOR_WHITE,         (INT32)eCM_WALL_COLOR_WHITE,            (INT32)CLI_WALL_COLOR_WHITE},
    /* 01 */ {(INT32)eGUI_WALL_COLOR_GRAY130,       (INT32)eCM_WALL_COLOR_GRAY130,          (INT32)CLI_WALL_COLOR_GRAY130},
    /* 02 */ {(INT32)eGUI_WALL_COLOR_LIGHT_YELLOW,  (INT32)eCM_WALL_COLOR_LIGHT_YELLOW,     (INT32)CLI_WALL_COLOR_LIGHT_YELLOW},
};

#if 0
/////////////////////////////////////////////////////////////////////////////////////
// 3D Mode
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_3DMode[] =
{
    /* 00 */ {(INT32)eGUI_3D_MODE_AUTO, (INT32)eCM_3D_MODE_AUTO, (INT32)CLI_3D_MODE_AUTO},
    /* 01 */ {(INT32)eGUI_3D_MODE_OFF,  (INT32)eCM_3D_MODE_OFF,  (INT32)CLI_3D_MODE_OFF},
    /* 02 */ {(INT32)eGUI_3D_MODE_ON,   (INT32)eCM_3D_MODE_ON,   (INT32)CLI_3D_MODE_ON},
};
#endif

/////////////////////////////////////////////////////////////////////////////////////
// 3D Format
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_3DFormat[] =
{
    /* 00 */ {(INT32)eGUI_3D_FORMAT_AUTO,               (INT32)eCM_3D_FORMAT_AUTO,              (INT32)CLI_3D_FORMAT_AUTO},
    /* 01 */ {(INT32)eGUI_3D_FORMAT_FRAME_PACKING,      (INT32)eCM_3D_FORMAT_FRAME_PACKING,     (INT32)CLI_3D_FORMAT_FRAME_PACKING},
    /* 02 */ {(INT32)eGUI_3D_FORMAT_SIDE_BY_SIDE,       (INT32)eCM_3D_FORMAT_SIDE_BY_SIDE,      (INT32)CLI_3D_FORMAT_SIDE_BY_SIDE},
    /* 03 */ {(INT32)eGUI_3D_FORMAT_TOP_AND_BOTTOM,     (INT32)eCM_3D_FORMAT_TOP_AND_BOTTOM,    (INT32)CLI_3D_FORMAT_TOP_AND_BOTTOM},
    /* 04 */ {(INT32)eGUI_3D_FORMAT_FRAME_SEQUENTIAL,   (INT32)eCM_3D_FORMAT_FRAME_SEQUENTIAL,  (INT32)CLI_3D_FORMAT_FRAME_SEQUENTIAL},
    /* 05 */ {(INT32)eGUI_3D_FORMAT_DUALPIPE,           (INT32)eCM_3D_FORMAT_DUALPIPE_3D,       (INT32)CLI_3D_FORMAT_DUALPIPE},
    /* 06 */ {(INT32)eGUI_3D_FORMAT_4K3D,               (INT32)eCM_3D_FORMAT_4K3D,              (INT32)CLI_3D_FORMAT_4K3D},
    /* 07 */ {(INT32)eGUI_3D_FORMAT_4K3D_DUALPIPE,      (INT32)eCM_3D_FORMAT_4K3D_DUALPIPE,     (INT32)CLI_3D_FORMAT_4K3D_DUALPIPE},
    /* 08 */ {(INT32)eGUI_3D_FORMAT_FRAME_OFF,          (INT32)eCM_3D_FORMAT_OFF,               (INT32)CLI_3D_FORMAT_OFF},
};

/////////////////////////////////////////////////////////////////////////////////////
// 3D Tech
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_3DTech[] =
{
    /* 00 */ {(INT32)eGUI_3D_TECH_DLP_LINK, (INT32)eCM_3D_TECH_DLP_LINK, (INT32)CLI_3D_TECH_DLP_LINK},
    /* 01 */ {(INT32)eGUI_3D_TECH_3D_SYNC,  (INT32)eCM_3D_TECH_3D_SYNC,  (INT32)CLI_3D_TECH_3D_SYNC},
};

/////////////////////////////////////////////////////////////////////////////////////
// 3D-2D
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_3D2DView[] =
{
    /* 00 */ {(INT32)eGUI_3D_2D_3D,         (INT32)eCM_3D_2D_3D,        (INT32)CLI_3D_2D_3D},
    /* 01 */ {(INT32)eGUI_3D_2D_2D_LEFT,    (INT32)eCM_3D_2D_2D_LEFT,   (INT32)CLI_3D_2D_2D_LEFT},
    /* 02 */ {(INT32)eGUI_3D_2D_2D_RIGHT,   (INT32)eCM_3D_2D_2D_RIGHT,  (INT32)CLI_3D_2D_2D_RIGHT},
};

/////////////////////////////////////////////////////////////////////////////////////
// 3D Sync Out
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_3DSyncOut[] =
{
    /* 00 */ {(INT32)eGUI_3D_SYNC_OUT_TO_EMITTER,        (INT32)eCM_3D_SYNC_OUT_TO_EMITTER,         (INT32)CLI_3D_SYNC_OUT_TO_EMITTER},
    /* 01 */ {(INT32)eGUI_3D_SYNC_OUT_TO_NEXT_PROJECTOR, (INT32)eCM_3D_SYNC_OUT_TO_NEXT_PROJECTOR,  (INT32)CLI_3D_SYNC_OUT_TO_NEXT_PROJECTOR},
};

/////////////////////////////////////////////////////////////////////////////////////
// 3D LR Reference
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_3DLRReference[] =
{
    /* 00 */ {(INT32)eGUI_3D_LR_REFERENCE_1ST_FRAME,     (INT32)eCM_3D_LR_REFERENCE_1ST_FRAME,   (INT32)CLI_3D_LR_REFERENCE_1ST_FRAME},
    /* 01 */ {(INT32)eGUI_3D_LR_REFERENCE_FIELD_GPIO,    (INT32)eCM_3D_LR_REFERENCE_FIELD_GPIO,  (INT32)CLI_3D_LR_REFERENCE_FIELD_GPIO},
};

/////////////////////////////////////////////////////////////////////////////////////
// 3D Sync Select (3D Sync Type)
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_3DSyncType[] =
{
    /* 00 */ {(INT32)eGUI_3D_SYNC_TYPE_AUTO,        (INT32)eCM_3D_SYNC_TYPE_AUTO,       (INT32)CLI_3D_SYNC_TYPE_AUTO},
    /* 01 */ {(INT32)eGUI_3D_SYNC_TYPE_INTERNAL,    (INT32)eCM_3D_SYNC_TYPE_INTERNAL,   (INT32)CLI_3D_SYNC_TYPE_INTERNAL},
    /* 02 */ {(INT32)eGUI_3D_SYNC_TYPE_EXTERNAL,    (INT32)eCM_3D_SYNC_TYPE_EXTERNAL,   (INT32)CLI_3D_SYNC_TYPE_EXTERNAL},
};

/////////////////////////////////////////////////////////////////////////////////////
// PIP Size
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_PIPSize[] =
{
    /* 00 */ {(INT32)eGUI_PIP_SIZE_SMALL,  (INT32)eCM_PIP_SIZE_SMALL,  (INT32)CLI_PIP_SIZE_SMALL},
    /* 01 */ {(INT32)eGUI_PIP_SIZE_MEDIUM, (INT32)eCM_PIP_SIZE_MEDIUM, (INT32)CLI_PIP_SIZE_MEDIUM},
    /* 02 */ {(INT32)eGUI_PIP_SIZE_LARGE,  (INT32)eCM_PIP_SIZE_LARGE,  (INT32)CLI_PIP_SIZE_LARGE},
};

/////////////////////////////////////////////////////////////////////////////////////
// PIP Layout
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_PIPLayout[] =
{
    /* 00 */ {(INT32)eGUI_MAIN_LAYOUT_PBP_MAIN_LEFT,     (INT32)eCM_MAIN_LAYOUT_PBP_MAIN_LEFT,   (INT32)CLI_MAIN_LAYOUT_PBP_MAIN_LEFT},
    /* 01 */ {(INT32)eGUI_MAIN_LAYOUT_PBP_MAIN_TOP,      (INT32)eCM_MAIN_LAYOUT_PBP_MAIN_TOP,    (INT32)CLI_MAIN_LAYOUT_PBP_MAIN_TOP},
    /* 02 */ {(INT32)eGUI_MAIN_LAYOUT_PBP_MAIN_RIGHT,    (INT32)eCM_MAIN_LAYOUT_PBP_MAIN_RIGHT,  (INT32)CLI_MAIN_LAYOUT_PBP_MAIN_RIGHT},
    /* 03 */ {(INT32)eGUI_MAIN_LAYOUT_PBP_MAIN_BOTTOM,   (INT32)eCM_MAIN_LAYOUT_PBP_MAIN_BOTTOM, (INT32)CLI_MAIN_LAYOUT_PBP_MAIN_BOTTOM},
    /* 04 */ {(INT32)eGUI_MAIN_LAYOUT_PIP_BOTTOM_RIGHT,  (INT32)eCM_MAIN_LAYOUT_PIP_BOTTOM_RIGHT,(INT32)CLI_MAIN_LAYOUT_PIP_BOTTOM_RIGHT},
    /* 05 */ {(INT32)eGUI_MAIN_LAYOUT_PIP_BOTTOM_LEFT,   (INT32)eCM_MAIN_LAYOUT_PIP_BOTTOM_LEFT, (INT32)CLI_MAIN_LAYOUT_PIP_BOTTOM_LEFT},
    /* 06 */ {(INT32)eGUI_MAIN_LAYOUT_PIP_TOP_LEFT,      (INT32)eCM_MAIN_LAYOUT_PIP_TOP_LEFT,    (INT32)CLI_MAIN_LAYOUT_PIP_TOP_LEFT},
    /* 07 */ {(INT32)eGUI_MAIN_LAYOUT_PIP_TOP_RIGHT,     (INT32)eCM_MAIN_LAYOUT_PIP_TOP_RIGHT,   (INT32)CLI_MAIN_LAYOUT_PIP_TOP_RIGHT},
};

/////////////////////////////////////////////////////////////////////////////////////
// Lan Path Switch
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_LanPathSwitch[] =
{
    /* 00 */ {(INT32)eGUI_LAN_PATH_SWITCH_RJ45,     (INT32)eCM_LAN_PATH_SWITCH_RJ45,    (INT32)CLI_LAN_PATH_SWITCH_RJ45},
    /* 01 */ {(INT32)eGUI_LAN_PATH_SWITCH_HDBASET,  (INT32)eCM_LAN_PATH_SWITCH_HDBASET, (INT32)CLI_LAN_PATH_SWITCH_HDBASET},
};

/////////////////////////////////////////////////////////////////////////////////////
// Serial Port Baud Rate
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_SerialPortBaudRate[] =
{
    /* 00 */ {(INT32)eGUI_SERIAL_PORT_BAUD_RATE_1200,   (INT32)eCM_SERIAL_PORT_BAUD_RATE_1200,  (INT32)CLI_SERIAL_PORT_BAUD_RATE_1200},
    /* 01 */ {(INT32)eGUI_SERIAL_PORT_BAUD_RATE_2400,   (INT32)eCM_SERIAL_PORT_BAUD_RATE_2400,  (INT32)CLI_SERIAL_PORT_BAUD_RATE_2400},
    /* 02 */ {(INT32)eGUI_SERIAL_PORT_BAUD_RATE_4800,   (INT32)eCM_SERIAL_PORT_BAUD_RATE_4800,  (INT32)CLI_SERIAL_PORT_BAUD_RATE_4800},
    /* 03 */ {(INT32)eGUI_SERIAL_PORT_BAUD_RATE_9600,   (INT32)eCM_SERIAL_PORT_BAUD_RATE_9600,  (INT32)CLI_SERIAL_PORT_BAUD_RATE_9600},
    /* 04 */ {(INT32)eGUI_SERIAL_PORT_BAUD_RATE_19200,  (INT32)eCM_SERIAL_PORT_BAUD_RATE_19200, (INT32)CLI_SERIAL_PORT_BAUD_RATE_19200},
    /* 05 */ {(INT32)eGUI_SERIAL_PORT_BAUD_RATE_38400,  (INT32)eCM_SERIAL_PORT_BAUD_RATE_38400, (INT32)CLI_SERIAL_PORT_BAUD_RATE_38400},
    /* 06 */ {(INT32)eGUI_SERIAL_PORT_BAUD_RATE_57600,  (INT32)eCM_SERIAL_PORT_BAUD_RATE_57600, (INT32)CLI_SERIAL_PORT_BAUD_RATE_57600},
    /* 07 */ {(INT32)eGUI_SERIAL_PORT_BAUD_RATE_115200, (INT32)eCM_SERIAL_PORT_BAUD_RATE_115200,(INT32)CLI_SERIAL_PORT_BAUD_RATE_115200},
};

/////////////////////////////////////////////////////////////////////////////////////
// OSD Time Out
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_OSDTimeOut[] =
{
    /* 00 */ {(INT32)eGUI_OSD_TIMEOUT_OFF,      (INT32)eCM_OSD_TIMEOUT_OFF,     (INT32)CLI_OSD_TIMEOUT_OFF},
    /* 01 */ {(INT32)eGUI_OSD_TIMEOUT_60_SEC,    (INT32)eCM_OSD_TIMEOUT_60_SEC,   (INT32)CLI_OSD_TIMEOUT_60_SEC},
    /* 02 */ {(INT32)eGUI_OSD_TIMEOUT_180_SEC,   (INT32)eCM_OSD_TIMEOUT_180_SEC,  (INT32)CLI_OSD_TIMEOUT_180_SEC},
};

/////////////////////////////////////////////////////////////////////////////////////
// Back Ground Color
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_BackgroundColor[] =
{
    /* 00 */ {(INT32)eGUI_BACKGROUND_COLOR_BLUE,    (INT32)eCM_BACKGROUND_COLOR_BLUE,   (INT32)CLI_BACKGROUND_COLOR_BLUE},
    /* 01 */ {(INT32)eGUI_BACKGROUND_COLOR_BLACK,   (INT32)eCM_BACKGROUND_COLOR_BLACK,  (INT32)CLI_BACKGROUND_COLOR_BLACK},
    /* 02 */ {(INT32)eGUI_BACKGROUND_COLOR_WHITE,   (INT32)eCM_BACKGROUND_COLOR_WHITE,  (INT32)CLI_BACKGROUND_COLOR_WHITE},
    /* 03 */ {(INT32)eGUI_BACKGROUND_COLOR_LOGO,    (INT32)eCM_BACKGROUND_COLOR_LOGO,   (INT32)CLI_BACKGROUND_COLOR_LOGO},
};

/////////////////////////////////////////////////////////////////////////////////////
// Standby Mode
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_StandbyMode[] =
{
    /* 00 */ {(INT32)eGUI_STANDBY_MODE_STANDBY,         (INT32)eCM_STANDBY_MODE_0_5W,           (INT32)CLI_STANDBY_MODE_STANDBY},
    ///* 01 */ {(INT32)eGUI_STANDBY_MODE_NETWORK_STANDBY, (INT32)eCM_STANDBY_MODE_2W,             (INT32)CLI_STANDBY_MODE_NETWORK_STANDBY},
    /* 02 */ {(INT32)eGUI_STANDBY_MODE_COMMUNICATION,   (INT32)eCM_STANDBY_MODE_COMMUNICATION,  (INT32)CLI_STANDBY_MODE_COMMUNICATION},
};

/////////////////////////////////////////////////////////////////////////////////////
// Power Mode (Light Source Mode)
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_PowerMode[] =
{
    /* 00 */ {(INT32)eGUI_POWER_MODE_CONSTANT_POWER,    (INT32)eCM_POWER_MODE_CONSTANT_POWER,       (INT32)CLI_POWER_MODE_CONSTANT_POWER},
    /* 01 */ {(INT32)eGUI_POWER_MODE_CONSTANT_INTENSITY,(INT32)eCM_POWER_MODE_CONSTANT_INTENSITY,   (INT32)CLI_POWER_MODE_CONSTANT_INTENSITY},
    /* 02 */ {(INT32)eGUI_POWER_MODE_ECO1,               (INT32)eCM_POWER_MODE_ECO1,                 (INT32)CLI_POWER_MODE_ECO1},
    /* 03 */ {(INT32)eGUI_POWER_MODE_ECO2,               (INT32)eCM_POWER_MODE_ECO2,                 (INT32)CLI_POWER_MODE_ECO2},
};

/////////////////////////////////////////////////////////////////////////////////////
// Splash Startup
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_SplashStartup[] =
{
    /* 00 */ {(INT32)eGUI_LOGO_PATTERN_FACTORY_LOGO,  (INT32)eCM_LOGO_PATTERN_FACTORY_LOGO, (INT32)CLI_LOGO_PATTERN_FACTORY_LOGO},
    /* 01 */ {(INT32)eGUI_LOGO_PATTERN_BLUE,          (INT32)eCM_LOGO_PATTERN_BLUE,         (INT32)CLI_LOGO_PATTERN_BLUE},
    /* 02 */ {(INT32)eGUI_LOGO_PATTERN_BLACK,         (INT32)eCM_LOGO_PATTERN_BLACK,        (INT32)CLI_LOGO_PATTERN_BLACK},
    /* 03 */ {(INT32)eGUI_LOGO_PATTERN_WHITE,         (INT32)eCM_LOGO_PATTERN_WHITE,        (INT32)CLI_LOGO_PATTERN_WHITE},
};

/////////////////////////////////////////////////////////////////////////////////////
// Low Latency
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_LowLatency[] =
{
    /* 00 */ {(INT32)eGUI_LOW_LATENCY_MODE_NORMAL,      (INT32)eCM_LOW_LATENCY_MODE_OFF,        (INT32)CLI_LOW_LATENCY_MODE_NORMAL},
    /* 01 */ {(INT32)eGUI_LOW_LATENCY_MODE_TYPICAL,     (INT32)eCM_LOW_LATENCY_MODE_TYPICAL,    (INT32)CLI_LOW_LATENCY_MODE_TYPICAL},
};

/////////////////////////////////////////////////////////////////////////////////////
// Detail
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_Detail[] =
{
    /* 00 */ {(INT32)eGUI_DETAIL_MAXIMUM,   (INT32)eCM_SHARPNESS_LEVEL13_MAXIMUM,    (INT32)CLI_DETAIL_MAXIMUM},
    /* 01 */ {(INT32)eGUI_DETAIL_HIGH,      (INT32)eCM_SHARPNESS_LEVEL11_HIGH,       (INT32)CLI_DETAIL_HIGH},
    /* 02 */ {(INT32)eGUI_DETAIL_NORMAL,    (INT32)eCM_SHARPNESS_LEVEL8_NORMAL,      (INT32)CLI_DETAIL_NORMAL},
    /* 03 */ {(INT32)eGUI_DETAIL_LOW,       (INT32)eCM_SHARPNESS_LEVEL5_LOW,         (INT32)CLI_DETAIL_LOW},
    /* 04 */ {(INT32)eGUI_DETAIL_MINIMUM,   (INT32)eCM_SHARPNESS_LEVEL3_MINIMUM,     (INT32)CLI_DETAIL_MINIMUM},
};

/////////////////////////////////////////////////////////////////////////////////////
// HDR Level
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_HdrLevel[] =
{
    /* 00 */ {(INT32)eGUI_HDR_LEVEL_1,      (INT32)eCM_HDR_LEVEL1,       (INT32)CLI_HDR_LEVEL_1},
    /* 01 */ {(INT32)eGUI_HDR_LEVEL_2,      (INT32)eCM_HDR_LEVEL2,       (INT32)CLI_HDR_LEVEL_2},
    /* 02 */ {(INT32)eGUI_HDR_LEVEL_3,      (INT32)eCM_HDR_LEVEL3,       (INT32)CLI_HDR_LEVEL_3},
    /* 03 */ {(INT32)eGUI_HDR_LEVEL_4,      (INT32)eCM_HDR_LEVEL4,       (INT32)CLI_HDR_LEVEL_4},
};

/////////////////////////////////////////////////////////////////////////////////////
// DateTime ClockMode
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_ClockMode[] =
{
    /* 00 */ {(INT32)eGUI_CLOCK_MODE_USE_NTP_SERVER,      (INT32)eCM_DATETIME_CLOCK_MODE_NTP_SERVER,        (INT32)CLI_CLOCK_MODE_USE_NTP_SERVER},
    /* 01 */ {(INT32)eGUI_CLOCK_MODE_MANUAL,              (INT32)eCM_DATETIME_CLOCK_MODE_MANUAL,            (INT32)CLI_CLOCK_MODE_MANUAL},
};

/////////////////////////////////////////////////////////////////////////////////////
// DateTime Update Interval
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_UpdateInterval[] =
{
    /* 00 */ {(INT32)eGUI_UPDATE_INTERVAL_HOURLY,             (INT32)eCM_DATETIME_UPDATE_INTERVAL_HOURLY,   (INT32)CLI_UPDATE_INTERVAL_HOURLY},
    /* 01 */ {(INT32)eGUI_UPDATE_INTERVAL_DAILY,              (INT32)eCM_DATETIME_UPDATE_INTERVAL_DAILY,    (INT32)CLI_UPDATE_INTERVAL_DAILY},
};

/////////////////////////////////////////////////////////////////////////////////////
// HDMI output
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_HDMIOutput[] =
{
    /* 00 */ {(INT32)eGUI_HDMI_OUT_HDMI1,               (INT32)eCM_HDMI_OUTPUT_HDMI1,     (INT32)CLI_HDMI_OUTPUT_HDMI1},
    /* 01 */ {(INT32)eGUI_HDMI_OUT_HDMI2,               (INT32)eCM_HDMI_OUTPUT_HDMI2,     (INT32)CLI_HDMI_OUTPUT_HDMI2},
};

/////////////////////////////////////////////////////////////////////////////////////
// COPY Event to Weekday
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_ScheduleCopyEventToWeekday[] =
{
    /* 00 */ {(INT32)eGUI_COPY_EVENT_TO_MONDAY,         (INT32)eCM_WEEKDAY_MONDAY,           (INT32)CLI_COPY_EVENT_TO_MONDAY},
    /* 01 */ {(INT32)eGUI_COPY_EVENT_TO_TUESDAY,        (INT32)eCM_WEEKDAY_TUESDAY,          (INT32)CLI_COPY_EVENT_TO_TUESDAY},
    /* 02 */ {(INT32)eGUI_COPY_EVENT_TO_WEDNESDAY,      (INT32)eCM_WEEKDAY_WEDNESDAY,        (INT32)CLI_COPY_EVENT_TO_WEDNESDAY},
    /* 03 */ {(INT32)eGUI_COPY_EVENT_TO_THURSDAY,       (INT32)eCM_WEEKDAY_THURSDAY,         (INT32)CLI_COPY_EVENT_TO_THURSDAY},
    /* 04 */ {(INT32)eGUI_COPY_EVENT_TO_FRIDAY,         (INT32)eCM_WEEKDAY_FRIDAY,           (INT32)CLI_COPY_EVENT_TO_FRIDAY},
    /* 05 */ {(INT32)eGUI_COPY_EVENT_TO_SATURDAY,       (INT32)eCM_WEEKDAY_SATURDAY,         (INT32)CLI_COPY_EVENT_TO_SATURDAY},
    /* 06 */ {(INT32)eGUI_COPY_EVENT_TO_SUNDAY,         (INT32)eCM_WEEKDAY_SUNDAY,           (INT32)CLI_COPY_EVENT_TO_SUNDAY},
};

/////////////////////////////////////////////////////////////////////////////////////
// 3D Mode
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_3DMode[] =
{
    /* 00 */ {(INT32)eGUI_3D_MODE_ACTIVE_3D,  (INT32)eCM_3D_MODE_ON,          (INT32)CLI_3D_MODE_ACTIVE_3D}, //active 3D
    /* 01 */ {(INT32)eGUI_3D_MODE_PASSIVE_3D, (INT32)eCM_3D_MODE_PASSIVE_3D,  (INT32)CLI_3D_MODE_PASSIVE_3D},
};

/////////////////////////////////////////////////////////////////////////////////////
// Warp Memory Save
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_GeoMemorySave[] =
{
    /* 00 */ {(INT32)eGUI_GEO_MEMORY_SAVE_1,           (INT32)eCM_GEO_MEMORY_SAVE_1,            (INT32)CLI_GEO_SAVE_MEMORY1},
    /* 01 */ {(INT32)eGUI_GEO_MEMORY_SAVE_2,           (INT32)eCM_GEO_MEMORY_SAVE_2,            (INT32)CLI_GEO_SAVE_MEMORY2},
    /* 02 */ {(INT32)eGUI_GEO_MEMORY_SAVE_3,           (INT32)eCM_GEO_MEMORY_SAVE_3,            (INT32)CLI_GEO_SAVE_MEMORY3},
    /* 03 */ {(INT32)eGUI_GEO_MEMORY_SAVE_4,           (INT32)eCM_GEO_MEMORY_SAVE_4,            (INT32)CLI_GEO_SAVE_MEMORY4},
};

/////////////////////////////////////////////////////////////////////////////////////
// Warp Memory Apply
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_GeoMemoryApply[] =
{
    /* 00 */ {(INT32)eGUI_GEO_MEMORY_APPLY_OFF,         (INT32)eCM_GEO_MEMORY_APPLY_OFF,          (INT32)CLI_GEO_APPLY_OFF},
    /* 01 */ {(INT32)eGUI_GEO_MEMORY_APPLY_1,           (INT32)eCM_GEO_MEMORY_APPLY_1,            (INT32)CLI_GEO_APPLY_MEMORY1},
    /* 02 */ {(INT32)eGUI_GEO_MEMORY_APPLY_2,           (INT32)eCM_GEO_MEMORY_APPLY_2,            (INT32)CLI_GEO_APPLY_MEMORY2},
    /* 03 */ {(INT32)eGUI_GEO_MEMORY_APPLY_3,           (INT32)eCM_GEO_MEMORY_APPLY_3,            (INT32)CLI_GEO_APPLY_MEMORY3},
    /* 04 */ {(INT32)eGUI_GEO_MEMORY_APPLY_4,           (INT32)eCM_GEO_MEMORY_APPLY_4,            (INT32)CLI_GEO_APPLY_MEMORY4},
};

/////////////////////////////////////////////////////////////////////////////
///////////////////////////  Data Mapping LUT  ////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
sDATA_MAPPING_LUT_FORMAT sDATA_MAPPING_LUT[] =
{
    {edcMAIN_INPUT,  asMPT_SourceID,  MPT_SIZE(asMPT_SourceID)},
    {edcSUB_INPUT,   asMPT_SourceID,  MPT_SIZE(asMPT_SourceID)},
    {edcBACKUPINPUT_CURRENT_SOURCE,  asMPT_SourceID,  MPT_SIZE(asMPT_SourceID)},
    //{edcBACKUPINPUT_PRIMARY_INPUT,  asMPT_SourceID,  MPT_SIZE(asMPT_SourceID)},   //G100_Steven_0021
    //{edcBACKUPINPUT_SECONDARY_INPUT,  asMPT_SourceID,  MPT_SIZE(asMPT_SourceID)}, //G100_Steven_0021

    {edcOSDTEST_PATTERN,  asMPT_TestPattern,  MPT_SIZE(asMPT_TestPattern)},
    {edcSERVICE_TEST_PATTERN,  asMPT_ServiceTestPattern,  MPT_SIZE(asMPT_ServiceTestPattern)},
    {edcTWIST_PATTERN,  asMPT_TwistTestPattern,  MPT_SIZE(asMPT_TwistTestPattern)},

    {edcPICTURE_SETTINGS,  asMPT_PictureSettings,  MPT_SIZE(asMPT_PictureSettings)},
    {edcFORCE_PICTURE_SETTINGS,  asMPT_PictureSettings,  MPT_SIZE(asMPT_PictureSettings)},
    {edcPRE_USER_MODE,  asMPT_PictureSettings,  MPT_SIZE(asMPT_PictureSettings)},
    {edcUSER_COLOR_MODE,  asMPT_ApplyUserSettings,  MPT_SIZE(asMPT_ApplyUserSettings)}, 	//G100_Doulas_0066

    {edcLANGUAGE,  asMPT_Language,  MPT_SIZE(asMPT_Language)},

    {edcSIZE_PRESETS,  asMPT_ScalingMode,  MPT_SIZE(asMPT_ScalingMode)},
    {edcMAIN_ASPECT_RATIO,  asMPT_ScalingMode,  MPT_SIZE(asMPT_ScalingMode)},
    {edcSUB_ASPECT_RATIO,  asMPT_ScalingMode,  MPT_SIZE(asMPT_ScalingMode)},

    {edcHOT_KEY_SETTINGS,  asMPT_HotkeySettings,  MPT_SIZE(asMPT_HotkeySettings)},
    {edcHOT_KEY2_SETTINGS,  asMPT_HotkeySettings,  MPT_SIZE(asMPT_HotkeySettings)},

    {edcPIP_PBP_ENABLE,  asMPT_ScreenMode,  MPT_SIZE(asMPT_ScreenMode)},
    {edcSIZE,  asMPT_PIPSize,  MPT_SIZE(asMPT_PIPSize)},

    {edcMENU_LOCATION,  asMPT_MenuLocation,  MPT_SIZE(asMPT_MenuLocation)},

    {edcCEILING_MOUNT,  asMPT_CeilingMount,  MPT_SIZE(asMPT_CeilingMount)},

    {edcGAMMA,  asMPT_Gamma,  MPT_SIZE(asMPT_Gamma)},

    {edcLENS_SAVE_CURRENT_POSITION,  asMPT_LensMemorySave,  MPT_SIZE(asMPT_LensMemorySave)},
    {edcLENS_APPLY_POSITION,  (asMPT_LensMemoryApply),  MPT_SIZE(asMPT_LensMemoryApply)},

    {edcREAR_PROJECTION,  (asMPT_OrientationDirection),  MPT_SIZE(asMPT_OrientationDirection)},

    {edcHDMI_EDID_1,  asMPT_EDID_Type,  MPT_SIZE(asMPT_EDID_Type)},
    {edcHDMI_EDID_2,  asMPT_EDID_Type,  MPT_SIZE(asMPT_EDID_Type)},
    {edcHDBASET_EDID,  asMPT_EDID_Type2,  MPT_SIZE(asMPT_EDID_Type2)},

    {edcBLENDING_GAMMA,  asMPT_BlendingGamma,  MPT_SIZE(asMPT_BlendingGamma)},

    {edcCOLOR_TEMPERATURE,  asMPT_ColorTemperature,  MPT_SIZE(asMPT_ColorTemperature)},

    {edcCOLOR_SPACE,  asMPT_ColorSpace,  MPT_SIZE(asMPT_ColorSpace)},

    {edcWALL_COLOR,  asMPT_WallColor,  MPT_SIZE(asMPT_WallColor)},

    //{edc3D_MODE,  asMPT_3DMode,  MPT_SIZE(asMPT_3DMode)},
    {edc3D_ENABLE,  asMPT_3DFormat,  MPT_SIZE(asMPT_3DFormat)},
    {edcDLPLink_ON,  asMPT_3DTech,  MPT_SIZE(asMPT_3DTech)},
    {edc3D_2D,  asMPT_3D2DView,  MPT_SIZE(asMPT_3D2DView)},
    {edc3D_SYNC_OUT,  asMPT_3DSyncOut,  MPT_SIZE(asMPT_3DSyncOut)},
    {edc3D_LR_REFERENCE,  asMPT_3DLRReference,  MPT_SIZE(asMPT_3DLRReference)},
    {edc3D_SYNC_TYPE,  asMPT_3DSyncType,  MPT_SIZE(asMPT_3DSyncType)},

    {edcSIZE,  asMPT_PIPSize,  MPT_SIZE(asMPT_PIPSize)},
    {edcMAIN_LAYOUT,  asMPT_PIPLayout,  MPT_SIZE(asMPT_PIPLayout)},

    {edcLAN_PATH_SWITCH,  asMPT_LanPathSwitch,  MPT_SIZE(asMPT_LanPathSwitch)},

    {edcSERIAL_PORT_BAUD_RATE,  asMPT_SerialPortBaudRate,  MPT_SIZE(asMPT_SerialPortBaudRate)},
    {edcSERIAL_PORT_OUT_BAUD_RATE,  asMPT_SerialPortBaudRate,  MPT_SIZE(asMPT_SerialPortBaudRate)},

    {edcMENU_TIME_OUT,  asMPT_OSDTimeOut,  MPT_SIZE(asMPT_OSDTimeOut)},

    {edcBACKGROUND_COLOR,  asMPT_BackgroundColor,  MPT_SIZE(asMPT_BackgroundColor)},

    {edcSTANDBY_MODE,  asMPT_StandbyMode,  MPT_SIZE(asMPT_StandbyMode)},

    {edcPOWER_MODE,  asMPT_PowerMode,  MPT_SIZE(asMPT_PowerMode)},

    {edcSPLASH_STARTUP,  asMPT_SplashStartup,  MPT_SIZE(asMPT_SplashStartup)},

    {edcLOW_LATENCY_MODE,  asMPT_LowLatency,  MPT_SIZE(asMPT_LowLatency)},

    {edcDETAIL,  asMPT_Detail,  MPT_SIZE(asMPT_Detail)},

    {edcHDR_LEVEL,  asMPT_HdrLevel,  MPT_SIZE(asMPT_HdrLevel)},

    {edcDATE_MODE,  asMPT_ClockMode,  MPT_SIZE(asMPT_ClockMode)},

    {edcDATE_UPDATE_INTERVAL,  asMPT_UpdateInterval,  MPT_SIZE(asMPT_UpdateInterval)},

    {edcHDMI_OUT,  asMPT_HDMIOutput,  MPT_SIZE(asMPT_HDMIOutput)},

    {edcSCHEDULE_COPY_EVENT_INDEX, asMPT_ScheduleCopyEventToWeekday, MPT_SIZE(asMPT_ScheduleCopyEventToWeekday)},

	{edc3D_MODE,  asMPT_3DMode,  MPT_SIZE(asMPT_3DMode)},	//A70Gen2_Doulas_0054
};
#define DATA_MAPPING_LUT_SIZE (sizeof(sDATA_MAPPING_LUT)/sizeof(sDATA_MAPPING_LUT_FORMAT))

/////////////////////////////////////////////////////////////////////////////


INT32 utilDataMapping_Init(UINT8 IsCreator)
{
    if(IsCreator)
    {
        DataMapping_DebugModeSet(eDEBUG_FATAL_ERROR);
        DataMapping_Init(sDATA_MAPPING_LUT, DATA_MAPPING_LUT_SIZE);
    }
    else
    {
        DataMapping_DebugModeSet(eDEBUG_FATAL_ERROR);
        DataMapping_ReaderInit();
    }
}

INT32 utilDataMapping_DeInit(void)
{
    DataMapping_DeInit();
}

INT32 utilDataMapping_CMValueRangeCheck(eDATA_CODE eDataCode, INT32 lCM_Value)
{
    if(IS_COMMON_DATA_CODE(eDataCode))
    {
        if(DataMapping_DataRangeCheck(eDataCode, lCM_Value) == RANGE_CHECK_PASS)
        {
            return DATA_RANGE_CHECK_PASS;
        }
    }
    else  //not common data code
    {
        INT32 iMax = CommonAPI_DataCode_MaxValue_Get(eDataCode);
        INT32 iMin = CommonAPI_DataCode_MinValue_Get(eDataCode);

        if((iMax >= lCM_Value) && (lCM_Value >= iMin))
        {
            return DATA_RANGE_CHECK_PASS;
        }
    }

    return DATA_RANGE_CHECK_FAIL;
}

INT32 utilDataMapping_GuiValueRangeCheck(eDATA_CODE eDataCode, INT32 lGui_Value)
{
    if(IS_COMMON_DATA_CODE(eDataCode))
    {
        if( lGui_Value >= 0 && lGui_Value <= (GUI_ITEM_SIZE(eDataCode)-1))
        {
            return DATA_RANGE_CHECK_PASS;
        }
    }
    else  //not common data code
    {
        INT32 iMax = CommonAPI_DataCode_MaxValue_Get(eDataCode);
        INT32 iMin = CommonAPI_DataCode_MinValue_Get(eDataCode);

        if((iMax >= lGui_Value) && (lGui_Value >= iMin))
        {
            return DATA_RANGE_CHECK_PASS;
        }
    }

    return DATA_RANGE_CHECK_FAIL;
}

#endif
