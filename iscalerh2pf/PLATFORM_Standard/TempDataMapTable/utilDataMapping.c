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

#if defined(PLATFORM_A70G2) || defined(PLATFORM_H60_2K)
/////////////////////////////////////////////////////////////////////////////////////
// Source ID
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_SourceID[] =
{
    /* 00 */ {(INT32)eGUI_SOURCE_ID_HDMI1,      (INT32)eCM_SOURCE_HDMI1,        (INT32)CLI_SOURCE_HDMI1},
    /* 01 */ {(INT32)eGUI_SOURCE_ID_HDMI2,      (INT32)eCM_SOURCE_HDMI2,        (INT32)CLI_SOURCE_HDMI2},
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
    /* 05 */ {(INT32)eGUI_3D_FORMAT_FRAME_OFF,   	    (INT32)eCM_3D_FORMAT_OFF,  			    (INT32)CLI_3D_FORMAT_OFF},
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
    /* 01 */ {(INT32)eGUI_LOW_LATENCY_MODE_2D_ULTRA,    (INT32)eCM_LOW_LATENCY_MODE_TYPICAL,    (INT32)CLI_LOW_LATENCY_MODE_2D_ULTRA},
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
    /* 00 */ {(INT32)eGUI_3D_MODE_ON,         (INT32)eCM_3D_MODE_ON,          (INT32)CLI_3D_MODE_ON},		//active 3D
    /* 01 */ {(INT32)eGUI_3D_MODE_PASSIVE_3D, (INT32)eCM_3D_MODE_PASSIVE_3D,  (INT32)CLI_3D_MODE_PASSIVE_3D},
};


#ifdef CUSTOM_CHRISTIE
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

#else

/////////////////////////////////////////////////////////////////////////////////////
// Warp Memory Save
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_GeoMemorySave[] =
{
    /* 00 */ {(INT32)eGUI_GEO_MEMORY_SAVE_1,           (INT32)eCM_GEO_MEMORY_SAVE_1,            (INT32)CLI_GEO_SAVE_MEMORY1},
    /* 01 */ {(INT32)eGUI_GEO_MEMORY_SAVE_2,           (INT32)eCM_GEO_MEMORY_SAVE_2,            (INT32)CLI_GEO_SAVE_MEMORY2},
    /* 02 */ {(INT32)eGUI_GEO_MEMORY_SAVE_3,           (INT32)eCM_GEO_MEMORY_SAVE_3,            (INT32)CLI_GEO_SAVE_MEMORY3},
    /* 03 */ {(INT32)eGUI_GEO_MEMORY_SAVE_4,           (INT32)eCM_GEO_MEMORY_SAVE_4,            (INT32)CLI_GEO_SAVE_MEMORY4},
    /* 03 */ {(INT32)eGUI_GEO_MEMORY_SAVE_5,           (INT32)eCM_GEO_MEMORY_SAVE_5,            (INT32)CLI_GEO_SAVE_MEMORY5},
};

/////////////////////////////////////////////////////////////////////////////////////
// Warp Memory Apply
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_GeoMemoryApply[] =	//A70Gen2_Doulas_0054
{
    /* 00 */ {(INT32)eGUI_GEO_MEMORY_APPLY_1,           (INT32)eCM_GEO_MEMORY_APPLY_1,            (INT32)CLI_GEO_APPLY_MEMORY1},
    /* 01 */ {(INT32)eGUI_GEO_MEMORY_APPLY_2,           (INT32)eCM_GEO_MEMORY_APPLY_2,            (INT32)CLI_GEO_APPLY_MEMORY2},
    /* 02 */ {(INT32)eGUI_GEO_MEMORY_APPLY_3,           (INT32)eCM_GEO_MEMORY_APPLY_3,            (INT32)CLI_GEO_APPLY_MEMORY3},
    /* 03 */ {(INT32)eGUI_GEO_MEMORY_APPLY_4,           (INT32)eCM_GEO_MEMORY_APPLY_4,            (INT32)CLI_GEO_APPLY_MEMORY4},
    /* 04 */ {(INT32)eGUI_GEO_MEMORY_APPLY_5,           (INT32)eCM_GEO_MEMORY_APPLY_5,            (INT32)CLI_GEO_APPLY_MEMORY5},
};
#endif

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


#else

/////////////////////////////////////////////////////////////////////////////////////
// Source ID
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_SourceID[] =
{
    /* 00 */ {(INT32)eGUI_SOURCE_ID_HDMI1,      (INT32)eCM_SOURCE_HDMI1,        (INT32)CLI_SOURCE_HDMI1},
    /* 01 */ {(INT32)eGUI_SOURCE_ID_HDMI2,      (INT32)eCM_SOURCE_HDMI2,        (INT32)CLI_SOURCE_HDMI2},
    /* 02 */ {(INT32)eGUI_SOURCE_ID_DVI,        (INT32)eCM_SOURCE_DVI,          (INT32)CLI_SOURCE_DVI},
    /* 03 */ {(INT32)eGUI_SOURCE_ID_3GSDI,      (INT32)eCM_SOURCE_3GSDI,        (INT32)CLI_SOURCE_3GSDI},
    /* 04 */ {(INT32)eGUI_SOURCE_ID_HDBASET,    (INT32)eCM_SOURCE_HDBASET,      (INT32)CLI_SOURCE_HDBASET},
};

/////////////////////////////////////////////////////////////////////////////////////
// Test Pattern
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_TestPattern[] =
{
    /* 00 */ {(INT32)eGUI_TEST_PATTERN_OFF,                 (INT32)eCM_TEST_PATTERN_OFF,            (INT32)CLI_TEST_PATTERN_OFF},
    /* 01 */ {(INT32)eGUI_TEST_PATTERN_GREEN_GRID,          (INT32)eCM_TEST_PATTERN_GREEN_GRID,     (INT32)CLI_TEST_PATTERN_GREEN_GRID},
    /* 02 */ {(INT32)eGUI_TEST_PATTERN_MAGENTA_GRID,        (INT32)eCM_TEST_PATTERN_MAGENTA_GRID,   (INT32)CLI_TEST_PATTERN_MAGENTA_GRID},
    /* 03 */ {(INT32)eGUI_TEST_PATTERN_WHITE_GRID,          (INT32)eCM_TEST_PATTERN_WHITE_GRID,     (INT32)CLI_TEST_PATTERN_WHITE_GRID},
    /* 04 */ {(INT32)eGUI_TEST_PATTERN_WHITE,               (INT32)eCM_TEST_PATTERN_WHITE,          (INT32)CLI_TEST_PATTERN_WHITE},
    /* 05 */ {(INT32)eGUI_TEST_PATTERN_BLACK,               (INT32)eCM_TEST_PATTERN_BLACK,          (INT32)CLI_TEST_PATTERN_BLACK},
    /* 06 */ {(INT32)eGUI_TEST_PATTERN_RED,                 (INT32)eCM_TEST_PATTERN_RED,            (INT32)CLI_TEST_PATTERN_RED},
    /* 07 */ {(INT32)eGUI_TEST_PATTERN_GREEN,               (INT32)eCM_TEST_PATTERN_GREEN,          (INT32)CLI_TEST_PATTERN_GREEN},
    /* 08 */ {(INT32)eGUI_TEST_PATTERN_BLUE,                (INT32)eCM_TEST_PATTERN_BLUE,           (INT32)CLI_TEST_PATTERN_BLUE},
    /* 09 */ {(INT32)eGUI_TEST_PATTERN_YELLOW,              (INT32)eCM_TEST_PATTERN_YELLOW,         (INT32)CLI_TEST_PATTERN_YELLOW},
    /* 10 */ {(INT32)eGUI_TEST_PATTERN_MAGENTA,             (INT32)eCM_TEST_PATTERN_MAGENTA,        (INT32)CLI_TEST_PATTERN_MAGENTA},
    /* 11 */ {(INT32)eGUI_TEST_PATTERN_CYAN,                (INT32)eCM_TEST_PATTERN_CYAN,           (INT32)CLI_TEST_PATTERN_CYAN},
    /* 12 */ {(INT32)eGUI_TEST_PATTERN_ANSI_CONSTRAST_4x4,  (INT32)eCM_TEST_PATTERN_CHECKERBOARD,   (INT32)CLI_TEST_PATTERN_ANSI_CONSTRAST_4x4},
    /* 13 */ {(INT32)eGUI_TEST_PATTERN_COLORBAR,            (INT32)eCM_TEST_PATTERN_COLORBAR,       (INT32)CLI_TEST_PATTERN_COLORBAR},
    /* 14 */ {(INT32)eGUI_TEST_PATTERN_FULL_SCREEN,         (INT32)eCM_TEST_PATTERN_FULL_SCREEN,    (INT32)CLI_TEST_PATTERN_FULL_SCREEN},
};

/////////////////////////////////////////////////////////////////////////////////////
// Service Test Pattern
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_ServiceTestPattern[] =
{
    /* 00 */ {(INT32)eGUI_SERVICE_TEST_PATTERN_OFF,                 (INT32)eCM_TEST_PATTERN_OFF,            (INT32)CLI_SERVICE_TEST_PATTERN_OFF},
    /* 01 */ {(INT32)eGUI_SERVICE_TEST_PATTERN_GREEN_GRID,          (INT32)eCM_TEST_PATTERN_GREEN_GRID,     (INT32)CLI_SERVICE_TEST_PATTERN_GREEN_GRID},
    /* 02 */ {(INT32)eGUI_SERVICE_TEST_PATTERN_MAGENTA_GRID,        (INT32)eCM_TEST_PATTERN_MAGENTA_GRID,   (INT32)CLI_SERVICE_TEST_PATTERN_MAGENTA_GRID},
    /* 03 */ {(INT32)eGUI_SERVICE_TEST_PATTERN_WHITE_GRID,          (INT32)eCM_TEST_PATTERN_WHITE_GRID,     (INT32)CLI_SERVICE_TEST_PATTERN_WHITE_GRID},
    /* 04 */ {(INT32)eGUI_SERVICE_TEST_PATTERN_WHITE,               (INT32)eCM_TEST_PATTERN_WHITE,          (INT32)CLI_SERVICE_TEST_PATTERN_WHITE},
    /* 05 */ {(INT32)eGUI_SERVICE_TEST_PATTERN_BLACK,               (INT32)eCM_TEST_PATTERN_BLACK,          (INT32)CLI_SERVICE_TEST_PATTERN_BLACK},
    /* 06 */ {(INT32)eGUI_SERVICE_TEST_PATTERN_RED,                 (INT32)eCM_TEST_PATTERN_RED,            (INT32)CLI_SERVICE_TEST_PATTERN_RED},
    /* 07 */ {(INT32)eGUI_SERVICE_TEST_PATTERN_GREEN,               (INT32)eCM_TEST_PATTERN_GREEN,          (INT32)CLI_SERVICE_TEST_PATTERN_GREEN},
    /* 08 */ {(INT32)eGUI_SERVICE_TEST_PATTERN_BLUE,                (INT32)eCM_TEST_PATTERN_BLUE,           (INT32)CLI_SERVICE_TEST_PATTERN_BLUE},
    /* 09 */ {(INT32)eGUI_SERVICE_TEST_PATTERN_YELLOW,              (INT32)eCM_TEST_PATTERN_YELLOW,         (INT32)CLI_SERVICE_TEST_PATTERN_YELLOW},
    /* 10 */ {(INT32)eGUI_SERVICE_TEST_PATTERN_MAGENTA,             (INT32)eCM_TEST_PATTERN_MAGENTA,        (INT32)CLI_SERVICE_TEST_PATTERN_MAGENTA},
    /* 11 */ {(INT32)eGUI_SERVICE_TEST_PATTERN_CYAN,                (INT32)eCM_TEST_PATTERN_CYAN,           (INT32)CLI_SERVICE_TEST_PATTERN_CYAN},
    /* 12 */ {(INT32)eGUI_SERVICE_TEST_PATTERN_ANSI_CONSTRAST_4x4,  (INT32)eCM_TEST_PATTERN_CHECKERBOARD,   (INT32)CLI_SERVICE_TEST_PATTERN_ANSI_CONSTRAST_4x4},
    /* 13 */ {(INT32)eGUI_SERVICE_TEST_PATTERN_COLORBAR,            (INT32)eCM_TEST_PATTERN_COLORBAR,       (INT32)CLI_SERVICE_TEST_PATTERN_COLORBAR},
    /* 14 */ {(INT32)eGUI_SERVICE_TEST_PATTERN_FULL_SCREEN,         (INT32)eCM_TEST_PATTERN_FULL_SCREEN,    (INT32)CLI_SERVICE_TEST_PATTERN_FULL_SCREEN},
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
// Picture Settings //A35G2_BRC_Casper_0012
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_PictureSettings[] =
{
	/* 00 */ {(INT32)eGUI_PICTURE_SETTINGS_PRESENTATION,    (INT32)eCM_PICTURE_SETTINGS_PRESENTATION,   (INT32)CLI_PICTURE_SETTINGS_PRESENTATION},
	/* 01 */ {(INT32)eGUI_PICTURE_SETTINGS_BRIGHT,		    (INT32)eCM_PICTURE_SETTINGS_BRIGHT,		    (INT32)CLI_PICTURE_SETTINGS_BRIGHT},
	/* 02 */ {(INT32)eGUI_PICTURE_SETTINGS_CINEMA,		    (INT32)eCM_PICTURE_SETTINGS_VIDEO,		    (INT32)CLI_PICTURE_SETTINGS_CINEMA},
	/* 03 */ {(INT32)eGUI_PICTURE_SETTINGS_HDR,			    (INT32)eCM_PICTURE_SETTINGS_HDR,		    (INT32)CLI_PICTURE_SETTINGS_HDR},
	/* 04 */ {(INT32)eGUI_PICTURE_SETTINGS_SRGB,		    (INT32)eCM_PICTURE_SETTINGS_SRGB,		    (INT32)CLI_PICTURE_SETTINGS_SRGB},
	/* 05 */ {(INT32)eGUI_PICTURE_SETTINGS_DICOMSIM,	    (INT32)eCM_PICTURE_SETTINGS_DICOMSIM,	    (INT32)CLI_PICTURE_SETTINGS_DICOMSIM},
	/* 06 */ {(INT32)eGUI_PICTURE_SETTINGS_BLENDING,	    (INT32)eCM_PICTURE_SETTINGS_BLENDING,	    (INT32)CLI_PICTURE_SETTINGS_BLENDING},
	/* 07 */ {(INT32)eGUI_PICTURE_SETTINGS_3D,			    (INT32)eCM_PICTURE_SETTINGS_3D,			    (INT32)CLI_PICTURE_SETTINGS_3D},
	/* 08 */ {(INT32)eGUI_PICTURE_SETTINGS_2D_HIGH_SPEED,   (INT32)eCM_PICTURE_SETTINGS_2DHIGHSPEED,    (INT32)CLI_PICTURE_SETTINGS_2D_HIGH_SPEED},	//G100_Clare_0001
	/* 09 */ {(INT32)eGUI_PICTURE_SETTINGS_USER,		    (INT32)eCM_PICTURE_SETTINGS_USER,		    (INT32)CLI_PICTURE_SETTINGS_USER},			//G100_Steven_0016 //G100_Clare_0002
};

/////////////////////////////////////////////////////////////////////////////////////
// Apply User Settings
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_ApplyUserSettings[] =				//G100_Doulas_0066 //A35G2_BRC_Casper_0012
{
	/* 00 */ {(INT32)eGUI_PRE_USER_PRESENTATION,    (INT32)eCM_PICTURE_SETTINGS_PRESENTATION,   (INT32)CLI_PRE_USER_PRESENTATION},
	/* 01 */ {(INT32)eGUI_PRE_USER_BRIGHT,		    (INT32)eCM_PICTURE_SETTINGS_BRIGHT,		    (INT32)CLI_PRE_USER_BRIGHT},
	/* 02 */ {(INT32)eGUI_PRE_USER_CINEMA,		    (INT32)eCM_PICTURE_SETTINGS_VIDEO,		    (INT32)CLI_PRE_USER_CINEMA},
	/* 03 */ {(INT32)eGUI_PRE_USER_HDR,			    (INT32)eCM_PICTURE_SETTINGS_HDR,		    (INT32)CLI_PRE_USER_HDR},
	/* 04 */ {(INT32)eGUI_PRE_USER_SRGB,		    (INT32)eCM_PICTURE_SETTINGS_SRGB,		    (INT32)CLI_PRE_USER_SRGB},
	/* 05 */ {(INT32)eGUI_PRE_USER_DICOMSIM,	    (INT32)eCM_PICTURE_SETTINGS_DICOMSIM,	    (INT32)CLI_PRE_USER_DICOMSIM},
	/* 06 */ {(INT32)eGUI_PRE_USER_BLENDING,	    (INT32)eCM_PICTURE_SETTINGS_BLENDING,	    (INT32)CLI_PRE_USER_BLENDING},
	/* 07 */ {(INT32)eGUI_PRE_USER_3D,			    (INT32)eCM_PICTURE_SETTINGS_3D,			    (INT32)CLI_PRE_USER_3D},
	/* 08 */ {(INT32)eGUI_PRE_USER_2D_HIGH_SPEED,   (INT32)eCM_PICTURE_SETTINGS_2DHIGHSPEED,    (INT32)CLI_PRE_USER_2D_HIGH_SPEED},
};


/////////////////////////////////////////////////////////////////////////////////////
// Language
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_Language[] =
{
    /* 00 */ {(INT32)eGUI_LANGUAGE_ENGLISH,     (INT32)eCM_LANGUAGE_ENGLISH,    (INT32)CLI_LANGUAGE_ENGLISH},
    /* 01 */ {(INT32)eGUI_LANGUAGE_S_CHINESE,   (INT32)eCM_LANGUAGE_S_CHINESE,  (INT32)CLI_LANGUAGE_S_CHINESE},
    /* 02 */ {(INT32)eGUI_LANGUAGE_FRENCH,      (INT32)eCM_LANGUAGE_FRENCH,     (INT32)CLI_LANGUAGE_FRENCH},
    /* 03 */ {(INT32)eGUI_LANGUAGE_GERMAN,      (INT32)eCM_LANGUAGE_GERMAN,     (INT32)CLI_LANGUAGE_GERMAN},
    /* 04 */ {(INT32)eGUI_LANGUAGE_ITALIAN,     (INT32)eCM_LANGUAGE_ITALIAN,    (INT32)CLI_LANGUAGE_ITALIAN},
    /* 05 */ {(INT32)eGUI_LANGUAGE_JAPANESE,    (INT32)eCM_LANGUAGE_JAPANESE,   (INT32)CLI_LANGUAGE_JAPANESE},
    /* 06 */ {(INT32)eGUI_LANGUAGE_KOREAN,      (INT32)eCM_LANGUAGE_KOREAN,     (INT32)CLI_LANGUAGE_KOREAN},
    /* 07 */ {(INT32)eGUI_LANGUAGE_RUSSIAN,     (INT32)eCM_LANGUAGE_RUSSIAN,    (INT32)CLI_LANGUAGE_RUSSIAN},
    /* 08 */ {(INT32)eGUI_LANGUAGE_SPANISH,     (INT32)eCM_LANGUAGE_SPANISH,    (INT32)CLI_LANGUAGE_SPANISH},
    /* 09 */ {(INT32)eGUI_LANGUAGE_PORTUGUESE,  (INT32)eCM_LANGUAGE_PORTUGUESE, (INT32)CLI_LANGUAGE_PORTUGUESE},
    /* 10 */ {(INT32)eGUI_LANGUAGE_INDONESIAN,  (INT32)eCM_LANGUAGE_INDONESIAN, (INT32)CLI_LANGUAGE_INDONESIAN},
    /* 11 */ {(INT32)eGUI_LANGUAGE_DUTCH,       (INT32)eCM_LANGUAGE_DUTCH,      (INT32)CLI_LANGUAGE_DUTCH},
};

/////////////////////////////////////////////////////////////////////////////////////
// Size Presets
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_ScalingMode[] =
{
    /* 00 */ {(INT32)eGUI_SCALING_MODE_AUTO,        (INT32)eCM_SCALING_MODE_AUTO,       (INT32)CLI_SCALING_MODE_AUTO},
    /* 01 */ {(INT32)eGUI_SCALING_MODE_4_3,         (INT32)eCM_SCALING_MODE_4_3,        (INT32)CLI_SCALING_MODE_4_3},
    /* 02 */ {(INT32)eGUI_SCALING_MODE_16_9,        (INT32)eCM_SCALING_MODE_16_9,       (INT32)CLI_SCALING_MODE_16_9},
    /* 03 */ {(INT32)eGUI_SCALING_MODE_16_10,       (INT32)eCM_SCALING_MODE_16_10,      (INT32)CLI_SCALING_MODE_16_10},
    /* 04 */ {(INT32)eGUI_SCALING_MODE_LETTER_BOX,  (INT32)eCM_SCALING_MODE_LETTER_BOX, (INT32)CLI_SCALING_MODE_LETTER_BOX},
    /* 05 */ {(INT32)eGUI_SCALING_MODE_NATIVE,      (INT32)eCM_SCALING_MODE_NATIVE,     (INT32)CLI_SCALING_MODE_NATIVE},
//    /* 06 */ {(INT32)eGUI_SCALING_MODE_3D,          (INT32)eCM_SCALING_MODE_3D,         (INT32)CLI_SCALING_MODE_3D},

};

/////////////////////////////////////////////////////////////////////////////////////
// Hotkey Settings
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_HotkeySettings[] =
{
    /* 00 */ {(INT32)eGUI_HOTKEY_SETTINGS_FREEZE_SCREEN,    (INT32)eCM_HOTKEY_SETTINGS_FREEZE_SCREEN,   (INT32)CLI_HOTKEY_SETTINGS_FREEZE_SCREEN},
    /* 01 */ {(INT32)eGUI_HOTKEY_SETTINGS_BLANK_SCREEN,     (INT32)eCM_HOTKEY_SETTINGS_BLANK_SCREEN,    (INT32)CLI_HOTKEY_SETTINGS_BLANK_SCREEN},
    /* 02 */ {(INT32)eGUI_HOTKEY_SETTINGS_SAVE_TO_USER,     (INT32)eCM_HOTKEY_SETTINGS_SAVE_TO_USER,    (INT32)CLI_HOTKEY_SETTINGS_SAVE_TO_USER},
    /* 03 */ {(INT32)eGUI_HOTKEY_SETTINGS_PIP_PBP,          (INT32)eCM_HOTKEY_SETTINGS_PIP_PBP,         (INT32)CLI_HOTKEY_SETTINGS_PIP_PBP},
    /* 04 */ {(INT32)eGUI_HOTKEY_SETTINGS_SIZE_PRESETS,     (INT32)eCM_HOTKEY_SETTINGS_SIZE_PRESETS,    (INT32)CLI_HOTKEY_SETTINGS_SIZE_PRESETS},
    /* 05 */ {(INT32)eGUI_HOTKEY_SETTINGS_SHOW_MESSAGE,     (INT32)eCM_HOTKEY_SETTINGS_SHOW_MESSAGE,    (INT32)CLI_HOTKEY_SETTINGS_SHOW_MESSAGE},
    /* 06 */ {(INT32)eGUI_HOTKEY_SETTINGS_USER_DATA,        (INT32)eCM_HOTKEY_SETTINGS_USER_DATA,       (INT32)CLI_HOTKEY_SETTINGS_USER_DATA},
    /* 07 */ {(INT32)eGUI_HOTKEY_SETTINGS_NETWORK_SETUP,    (INT32)eCM_HOTKEY_SETTINGS_NETWORK_SETUP,   (INT32)CLI_HOTKEY_SETTINGS_NETWORK_SETUP},
    /* 08 */ {(INT32)eGUI_HOTKEY_SETTINGS_PROJECTOR_ID,     (INT32)eCM_HOTKEY_SETTINGS_PROJECTOR_ID,    (INT32)CLI_HOTKEY_SETTINGS_PROJECTOR_ID},
    /* 09 */ {(INT32)eGUI_HOTKEY_SETTINGS_ORIENTATION,      (INT32)eCM_HOTKEY_SETTINGS_ORIENTATION,     (INT32)CLI_HOTKEY_SETTINGS_ORIENTATION},
    /* 10 */ {(INT32)eGUI_HOTKEY_SETTINGS_CUSTOM_RGBCYM,    (INT32)eCM_HOTKEY_SETTINGS_CUSTOM_RGBCYM,   (INT32)CLI_HOTKEY_SETTINGS_CUSTOM_RGBCYM},
    /* 11 */ {(INT32)eGUI_HOTKEY_SETTINGS_MULTI_PROJECTION, (INT32)eCM_HOTKEY_SETTINGS_MULTI_PROJECTION,(INT32)CLI_HOTKEY_SETTINGS_MULTI_PROJECTION},
    /* 12 */ {(INT32)eGUI_HOTKEY_SETTINGS_RESET_SELECTIVE,  (INT32)eCM_HOTKEY_SETTINGS_RESET_SELECTIVE, (INT32)CLI_HOTKEY_SETTINGS_RESET_SELECTIVE}, //G100_Coda_0050
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
    /* 00 */  {(INT32)eGUI_CEILING_MOUNT_AUTO,   (INT32)eCM_CEILING_MOUNT_AUTO,  (INT32)CLI_CEILING_MOUNT_AUTO},
    /* 01 */  {(INT32)eGUI_CEILING_MOUNT_ON,     (INT32)eCM_CEILING_MOUNT_ON,    (INT32)CLI_CEILING_MOUNT_ON},
    /* 02 */  {(INT32)eGUI_CEILING_MOUNT_OFF,    (INT32)eCM_CEILING_MOUNT_OFF,   (INT32)CLI_CEILING_MOUNT_OFF},   //G100_Coda_0048
};

/////////////////////////////////////////////////////////////////////////////////////
// Gamma
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_Gamma[] =   //G100_Simon_0022
{
    /* 00 */ {(INT32)eGUI_GAMMA_1_8,            (INT32)eCM_GAMMA_1_8,           (INT32)CLI_GAMMA_1_8},
    /* 01 */ {(INT32)eGUI_GAMMA_2_0,            (INT32)eCM_GAMMA_2_0,           (INT32)CLI_GAMMA_2_0},
    /* 02 */ {(INT32)eGUI_GAMMA_2_2,            (INT32)eCM_GAMMA_2_2,           (INT32)CLI_GAMMA_2_2},
    /* 03 */ {(INT32)eGUI_GAMMA_2_4,            (INT32)eCM_GAMMA_2_4,           (INT32)CLI_GAMMA_2_4},
    /* 04 */ {(INT32)eGUI_GAMMA_2_6,            (INT32)eCM_GAMMA_2_6,           (INT32)CLI_GAMMA_2_6},
    /* 05 */ {(INT32)eGUI_GAMMA_GRAPHICS,       (INT32)eCM_GAMMA_GRAPHICS,      (INT32)CLI_GAMMA_GRAPHICS},
    /* 06 */ {(INT32)eGUI_GAMMA_VIDEO,          (INT32)eCM_GAMMA_VIDEO,         (INT32)CLI_GAMMA_VIDEO},
    /* 07 */ {(INT32)eGUI_GAMMA_CRT,            (INT32)eCM_GAMMA_CRT,           (INT32)CLI_GAMMA_CRT},
    /* 08 */ {(INT32)eGUI_GAMMA_ENHANCED,       (INT32)eCM_GAMMA_ENHANCED,      (INT32)CLI_GAMMA_ENHANCED},
    /* 09 */ {(INT32)eGUI_GAMMA_FILM,           (INT32)eCM_GAMMA_FILM,          (INT32)CLI_GAMMA_FILM},
    /* 10 */ {(INT32)eGUI_GAMMA_DICOM,          (INT32)eCM_GAMMA_DICOM,         (INT32)CLI_GAMMA_DICOM},
    /* 11 */ {(INT32)eGUI_GAMMA_HDR_STANDARD,   (INT32)eCM_GAMMA_HDR_STANDARD,  (INT32)CLI_GAMMA_HDR},
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
    /* 00 */ {(INT32)eGUI_EDID_TYPE_V14,        (INT32)eCM_EDID_TYPE_V14,        (INT32)CLI_EDID_TYPE_V14},
    /* 01 */ {(INT32)eGUI_EDID_TYPE_V20,        (INT32)eCM_EDID_TYPE_V20,        (INT32)CLI_EDID_TYPE_V20},
#if (CUSTOMIZED_EDID_ENABLE == TRUE)
    /* 02 */ {(INT32)eGUI_EDID_TYPE_CUSTOMIZED, (INT32)eCM_EDID_TYPE_CUSTOMIZED, (INT32)CLI_EDID_TYPE_CUSTOMIZED},  //A35G2_Simon_0091
#endif
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
    /* 00 */ {(INT32)eGUI_WALL_COLOR_OFF,           (INT32)eCM_WALL_COLOR_OFF,          (INT32)CLI_WALL_COLOR_OFF},
    //* 01 */ {(INT32)eGUI_WALL_COLOR_AUTO,          (INT32)eCM_WALL_COLOR_AUTO,         (INT32)CLI_WALL_COLOR_AUTO},	//G100_Clare_0073//G100_Clare_0035
    /* 01 */ {(INT32)eGUI_WALL_COLOR_BLACKBOARD,    (INT32)eCM_WALL_COLOR_BLACKBOARD,   (INT32)CLI_WALL_COLOR_BLACKBOARD},
    /* 02 */ {(INT32)eGUI_WALL_COLOR_LIGHT_YELLOW,  (INT32)eCM_WALL_COLOR_LIGHT_YELLOW, (INT32)CLI_WALL_COLOR_LIGHT_YELLOW},
    /* 03 */ {(INT32)eGUI_WALL_COLOR_LIGHT_GREEN,   (INT32)eCM_WALL_COLOR_LIGHT_GREEN,  (INT32)CLI_WALL_COLOR_LIGHT_GREEN},
    /* 04 */ {(INT32)eGUI_WALL_COLOR_LIGHT_BLUE,    (INT32)eCM_WALL_COLOR_LIGHT_BLUE,   (INT32)CLI_WALL_COLOR_LIGHT_BLUE},
    /* 05 */ {(INT32)eGUI_WALL_COLOR_PINK,          (INT32)eCM_WALL_COLOR_PINK,         (INT32)CLI_WALL_COLOR_PINK},
    /* 06 */ {(INT32)eGUI_WALL_COLOR_GRAY,          (INT32)eCM_WALL_COLOR_GRAY,         (INT32)CLI_WALL_COLOR_GRAY},
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
    /* 01 */ {(INT32)eGUI_OSD_TIMEOUT_5_SEC,    (INT32)eCM_OSD_TIMEOUT_5_SEC,   (INT32)CLI_OSD_TIMEOUT_5_SEC},
    /* 02 */ {(INT32)eGUI_OSD_TIMEOUT_10_SEC,   (INT32)eCM_OSD_TIMEOUT_10_SEC,  (INT32)CLI_OSD_TIMEOUT_10_SEC},
    /* 03 */ {(INT32)eGUI_OSD_TIMEOUT_15_SEC,   (INT32)eCM_OSD_TIMEOUT_15_SEC,  (INT32)CLI_OSD_TIMEOUT_15_SEC},
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
    /* 01 */ {(INT32)eGUI_STANDBY_MODE_NETWORK_STANDBY, (INT32)eCM_STANDBY_MODE_2W,             (INT32)CLI_STANDBY_MODE_NETWORK_STANDBY},
    /* 02 */ {(INT32)eGUI_STANDBY_MODE_COMMUNICATION,   (INT32)eCM_STANDBY_MODE_COMMUNICATION,  (INT32)CLI_STANDBY_MODE_COMMUNICATION},
};

/////////////////////////////////////////////////////////////////////////////////////
// Power Mode (Light Source Mode)
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_PowerMode[] =
{
    /* 00 */ {(INT32)eGUI_POWER_MODE_CONSTANT_POWER,(INT32)eCM_POWER_MODE_CONSTANT_POWER,   (INT32)CLI_POWER_MODE_CONSTANT_POWER},
    /* 01 */ {(INT32)eGUI_POWER_MODE_ECO,           (INT32)eCM_POWER_MODE_ECO1,             (INT32)CLI_POWER_MODE_ECO}, //A35G2_BRC_Casper_0001
    /* 02 */ {(INT32)eGUI_POWER_MODE_CUSTOM_MODE,   (INT32)eCM_POWER_MODE_CUSTOM_MODE,      (INT32)CLI_POWER_MODE_CUSTOM_MODE},	//G100_Clare_0030
};

/////////////////////////////////////////////////////////////////////////////////////
// Splash Startup
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_SplashStartup[] =
{

    /* 01 */ {(INT32)eGUI_LOGO_PATTERN_BLUE,          (INT32)eCM_LOGO_PATTERN_BLUE,         (INT32)CLI_LOGO_PATTERN_BLUE},
    /* 02 */ {(INT32)eGUI_LOGO_PATTERN_BLACK,         (INT32)eCM_LOGO_PATTERN_BLACK,        (INT32)CLI_LOGO_PATTERN_BLACK},
    /* 03 */ {(INT32)eGUI_LOGO_PATTERN_WHITE,         (INT32)eCM_LOGO_PATTERN_WHITE,        (INT32)CLI_LOGO_PATTERN_WHITE},
     /* 00 */ {(INT32)eGUI_LOGO_PATTERN_FACTORY_LOGO,  (INT32)eCM_LOGO_PATTERN_FACTORY_LOGO, (INT32)CLI_LOGO_PATTERN_FACTORY_LOGO},
};

/////////////////////////////////////////////////////////////////////////////////////
// Low Latency
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_LowLatency[] =
{
    /* 00 */ {(INT32)eGUI_LOW_LATENCY_MODE_NORMAL,      (INT32)eCM_LOW_LATENCY_MODE_OFF,        (INT32)CLI_LOW_LATENCY_MODE_NORMAL},
    /* 01 */ {(INT32)eGUI_LOW_LATENCY_MODE_2D_ULTRA,    (INT32)eCM_LOW_LATENCY_MODE_TYPICAL,    (INT32)CLI_LOW_LATENCY_MODE_2D_ULTRA},
};

/////////////////////////////////////////////////////////////////////////////////////
// Detail
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_Detail[] =
{ //A35G2_BRC_Casper_0004 //A35G2_BRC_Casper_0028
    /* 00 */ {(INT32)eGUI_DETAIL_LEVEL0,    (INT32)eCM_ID_INVALID,                  (INT32)CLI_DETAIL_LEVEL0},  //A35G2_BRC_Simon_0002
    /* 01 */ {(INT32)eGUI_DETAIL_LEVEL1,    (INT32)eCM_SHARPNESS_LEVEL1,            (INT32)CLI_DETAIL_LEVEL1},
    /* 02 */ {(INT32)eGUI_DETAIL_LEVEL2,    (INT32)eCM_SHARPNESS_LEVEL2,            (INT32)CLI_DETAIL_LEVEL2},
    /* 03 */ {(INT32)eGUI_DETAIL_LEVEL3,    (INT32)eCM_SHARPNESS_LEVEL3_MINIMUM,    (INT32)CLI_DETAIL_LEVEL3},
    /* 04 */ {(INT32)eGUI_DETAIL_LEVEL4,    (INT32)eCM_SHARPNESS_LEVEL4,            (INT32)CLI_DETAIL_LEVEL4},
    /* 05 */ {(INT32)eGUI_DETAIL_LEVEL5,    (INT32)eCM_SHARPNESS_LEVEL5_LOW,        (INT32)CLI_DETAIL_LEVEL5},
    /* 06 */ {(INT32)eGUI_DETAIL_LEVEL6,    (INT32)eCM_SHARPNESS_LEVEL6,            (INT32)CLI_DETAIL_LEVEL6},
    /* 07 */ {(INT32)eGUI_DETAIL_LEVEL7,    (INT32)eCM_SHARPNESS_LEVEL7,            (INT32)CLI_DETAIL_LEVEL7},
    /* 08 */ {(INT32)eGUI_DETAIL_LEVEL8,    (INT32)eCM_SHARPNESS_LEVEL8_NORMAL,     (INT32)CLI_DETAIL_LEVEL8},
    /* 09 */ {(INT32)eGUI_DETAIL_LEVEL9,    (INT32)eCM_SHARPNESS_LEVEL9,            (INT32)CLI_DETAIL_LEVEL9},
    /* 10 */ {(INT32)eGUI_DETAIL_LEVEL10,   (INT32)eCM_SHARPNESS_LEVEL10,           (INT32)CLI_DETAIL_LEVEL10},
    /* 11 */ {(INT32)eGUI_DETAIL_LEVEL11,   (INT32)eCM_SHARPNESS_LEVEL11_HIGH,      (INT32)CLI_DETAIL_LEVEL11},
    /* 12 */ {(INT32)eGUI_DETAIL_LEVEL12,   (INT32)eCM_SHARPNESS_LEVEL12,           (INT32)CLI_DETAIL_LEVEL12},
    /* 13 */ {(INT32)eGUI_DETAIL_LEVEL13,   (INT32)eCM_SHARPNESS_LEVEL13_MAXIMUM,   (INT32)CLI_DETAIL_LEVEL13},
    /* 14 */ {(INT32)eGUI_DETAIL_LEVEL14,   (INT32)eCM_SHARPNESS_LEVEL14,           (INT32)CLI_DETAIL_LEVEL14},
    /* 15 */ {(INT32)eGUI_DETAIL_LEVEL15,   (INT32)eCM_SHARPNESS_LEVEL15,           (INT32)CLI_DETAIL_LEVEL15},
};

/////////////////////////////////////////////////////////////////////////////////////
// HDR Level
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_HdrLevel[] =
{
    /* 00 */ {(INT32)eGUI_HDR_LEVEL_BRIGHT,     (INT32)eCM_HDR_LEVEL1,       (INT32)CLI_HDR_LEVEL_1},
    /* 01 */ {(INT32)eGUI_HDR_LEVEL_STANDARD,   (INT32)eCM_HDR_LEVEL2,       (INT32)CLI_HDR_LEVEL_2},
    /* 02 */ {(INT32)eGUI_HDR_LEVEL_FILM,       (INT32)eCM_HDR_LEVEL3,       (INT32)CLI_HDR_LEVEL_3},
    /* 03 */ {(INT32)eGUI_HDR_LEVEL_DETAIL,     (INT32)eCM_HDR_LEVEL4,       (INT32)CLI_HDR_LEVEL_4},
}; //A35G2_BRC_Casper_0034

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
// Keypad Backlight //A35G2_BRC_Casper_0007
/////////////////////////////////////////////////////////////////////////////////////
static sMAPPING_TABLE_FORMAT asMPT_KeypadBacklight[] =
{
    /* 00 */ {(INT32)eGUI_KEYPAD_BACKLIGHT_ALWAYS_OFF,      (INT32)eCM_KEYPAD_BACKLIGHT_ALWAYS_OFF,     (INT32)CLI_KEYPAD_BACKLIGHT_ALWAYS_OFF},
    /* 01 */ {(INT32)eGUI_KEYPAD_BACKLIGHT_ALWAYS_ON,       (INT32)eCM_KEYPAD_BACKLIGHT_ALWAYS_ON,      (INT32)CLI_KEYPAD_BACKLIGHT_ALWAYS_ON},
};

/////////////////////////////////////////////////////////////////////////////
///////////////////////////  Data Mapping LUT  ////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
sDATA_MAPPING_LUT_FORMAT sDATA_MAPPING_LUT[] =
{
    {edcMAIN_INPUT,  asMPT_SourceID,  MPT_SIZE(asMPT_SourceID)},
    {edcSUB_INPUT,  asMPT_SourceID,  MPT_SIZE(asMPT_SourceID)},
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

    {edcDETAIL,  asMPT_Detail,  MPT_SIZE(asMPT_Detail)},

    {edcHDR_LEVEL,  asMPT_HdrLevel,  MPT_SIZE(asMPT_HdrLevel)},

    {edcDATE_MODE,  asMPT_ClockMode,  MPT_SIZE(asMPT_ClockMode)},

    {edcDATE_UPDATE_INTERVAL,  asMPT_UpdateInterval,  MPT_SIZE(asMPT_UpdateInterval)},

    {edcHDMI_OUT,  asMPT_HDMIOutput,  MPT_SIZE(asMPT_HDMIOutput)},

    {edcKEYPAD_BACKLIGHT,  asMPT_KeypadBacklight,  MPT_SIZE(asMPT_KeypadBacklight)},  //A35G2_BRC_Casper_0007

    {edcWARP_MEMORY_SAVE,   asMPT_GeoMemorySave,   MPT_SIZE(asMPT_GeoMemorySave)},
    {edcWARP_MEMORY_APPLY,  asMPT_GeoMemoryApply,  MPT_SIZE(asMPT_GeoMemoryApply)},

};
#define DATA_MAPPING_LUT_SIZE (sizeof(sDATA_MAPPING_LUT)/sizeof(sDATA_MAPPING_LUT_FORMAT))

/////////////////////////////////////////////////////////////////////////////
#endif

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


