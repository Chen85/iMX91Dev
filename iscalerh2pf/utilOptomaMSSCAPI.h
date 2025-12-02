#if defined(CUSTOM_OPTOMA)
// ===============================================================================
// FILE NAME: utilChristieMSSCAPI.h
// DESCRIPTION:
//
//
// Modification History
// --------------------
// 2017/10/08, Larry Create
// --------------------
// ===============================================================================

#ifndef UTILOPTOMAMSSCAPI_H
#define UTILOPTOMAMSSCAPI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "Common.h"


typedef enum
{
	eOptomaUART_PRESETATION = 1,
	eOptomaUART_BRIGHT      = 2,
	eOptomaUART_SUPERBRIGHT = 23,//22,  //ZU860_Doulas_0118 modify//ZU860_John_0033 add super bright mode (need OSD)
	eOptomaUART_CINEMA	    = 3,
	eOptomaUART_HDR         = 21,   //ZU860_Doulas_0063 Modify
	eOptomaUART_sRGB        = 4,
	eOptomaUART_DICOMSIM    = 13,
	eOptomaUART_BLENDING    = 19,
	eOptomaUART_2DHIGHSPEED = 18,
	eOptomaUART_USER        = 5,

	eOptomaUART_3D          = 9,
	eOptomaUART_NO_SIGNAL   = 0,

} eOptomaUART_PresetMode;          //ZU860_Doulas_0022

typedef enum
{
	eOptomaUART_WALL_COLOR_OFF              = 0,
	eOptomaUART_WALL_COLOR_BLACKBOARD       = 1,
	eOptomaUART_WALL_COLOR_LIGHT_YELLOW     = 7,
	eOptomaUART_WALL_COLOR_LIGHT_GREEN      = 3,
	eOptomaUART_WALL_COLOR_LIGHT_BLUE       = 4,
	eOptomaUART_WALL_COLOR_PINK             = 5,
	eOptomaUART_WALL_COLOR_GRAY             = 6,

    eOptomaUART_AUTO_WALL_COLOR_EXECUTE     = 9             //B35LC_Tim_0026, add

} eOptomaUART_WALL_COLOR;              //ZU860_Doulas_0022

typedef enum
{
	eOptomaUART_GAMMA_FILM       = 1,
	eOptomaUART_GAMMA_VIDEO      = 2,
	eOptomaUART_GAMMA_GRAPHICS   = 3,
	eOptomaUART_GAMMA_STANDARD   = 4,   //2.2
	eOptomaUART_GAMMA_3D         = 9,
	eOptomaUART_GAMMA_BLACKBOARD = 10,
	eOptomaUART_GAMMA_DICOM      = 11,
	eOptomaUART_GAMMA_HDR_1p8    = 5,       //ZU860_Doulas_0101
	eOptomaUART_GAMMA_HDR_2p0    = 6,       //ZU860_Doulas_0101
	eOptomaUART_GAMMA_HDR_2p4    = 12,      //ZU860_Doulas_0101

} eOptomaUART_GAMMA_SETTING;              //ZU860_Doulas_0022

typedef enum
{
	eOptomaUART_COLOR_TEMPERATURE_WARM          = 4,
	eOptomaUART_COLOR_TEMPERATURE_STANDARD      = 1,
	eOptomaUART_COLOR_TEMPERATURE_COOL          = 2,
} eOptomaUART_COLOR_TEMPERATURE;       //ZU860_Doulas_0022

typedef enum
{
	eOptomaUART_COLOR_TEMPERATURE_RES_WARM      = 3,
	eOptomaUART_COLOR_TEMPERATURE_RES_STANDARD  = 0,
	eOptomaUART_COLOR_TEMPERATURE_RES_COOL      = 1,
} eOptomaUART_COLOR_TEMPERATURE_RESPONSE;       //ZU860_Doulas_0027

typedef enum
{
	eOptomaUART_COLOR_SPACE_AUTO            = 1,
	eOptomaUART_COLOR_SPACE_RGB_RGB         = 2,
	eOptomaUART_COLOR_SPACE_RGB_RGB_VIDEO   = 4,
	eOptomaUART_COLOR_SPACE_YUV             = 3,
	eOptomaUART_COLOR_SPACE_REC709	        = 5,
    eOptomaUART_COLOR_SPACE_REC601	        = 6,

} eOptomaUART_COLOR_SPACE_STATE;       //ZU860_Doulas_0023

typedef enum
{
	eOptomaUART_COLOR_SPEED_2X      = 1,
	eOptomaUART_COLOR_SPEED_3X      = 2,

} eOptomaUART_COLOR_SPEED;                 //ZU860_Doulas_0023

typedef enum
{
	eOptomaUART_RES_COLOR_SPEED_2X      = 2,
	eOptomaUART_RES_COLOR_SPEED_3X      = 3,

} eOptomaUART_RES_COLOR_SPEED;                 //ZU860_Doulas_0050

typedef enum
{
	eOptomaUART_CONSTANT_POWER      = 6,
	eOptomaUART_CONSTANT_INTENSITY  = 7,
	eOptomaUART_ECO                 = 2,
} eOptomaUART_POWER_MODE;          //ZU860_Doulas_0023

typedef enum
{
	eOptomaUART_SCALING_MODE_4_3            = 1,
	eOptomaUART_SCALING_MODE_16_9           = 2,
	eOptomaUART_SCALING_MODE_16_10          = 3,
	eOptomaUART_SCALING_MODE_LETTER_BOX     = 5,
	eOptomaUART_SCALING_MODE_NATIVE         = 6,
	eOptomaUART_SCALING_MODE_AUTO           = 7,
} eOptomaUART_SCALING_MODE;        //ZU860_Doulas_0025

typedef enum
{
	eOptomaUART_4CORNER_TL_RT   = 1,
	eOptomaUART_4CORNER_TL_LT   = 2,
	eOptomaUART_4CORNER_TL_UP   = 3,
	eOptomaUART_4CORNER_TL_DN   = 4,
	eOptomaUART_4CORNER_TR_RT   = 5,
	eOptomaUART_4CORNER_TR_LT   = 6,
	eOptomaUART_4CORNER_TR_UP   = 7,
	eOptomaUART_4CORNER_TR_DN   = 8,
    eOptomaUART_4CORNER_BL_RT   = 9,
	eOptomaUART_4CORNER_BL_LT   = 10,
	eOptomaUART_4CORNER_BL_UP   = 11,
	eOptomaUART_4CORNER_BL_DN   = 12,
    eOptomaUART_4CORNER_BR_RT   = 13,
	eOptomaUART_4CORNER_BR_LT   = 14,
	eOptomaUART_4CORNER_BR_UP   = 15,
	eOptomaUART_4CORNER_BR_DN   = 16,
} eOptomaUART_4CORNER;          //ZU860_Doulas_0026

typedef enum
{
    eOptomaUART_LAYOUT_PIP_TOP_LEFT         = 1,
    eOptomaUART_LAYOUT_PIP_TOP_RIGHT,
    eOptomaUART_LAYOUT_PIP_BOTTOM_LEFT,
    eOptomaUART_LAYOUT_PIP_BOTTOM_RIGHT,
    eOptomaUART_LAYOUT_PBP_MAIN_LEFT,
	eOptomaUART_LAYOUT_PBP_MAIN_TOP,
	eOptomaUART_LAYOUT_PBP_MAIN_RIGHT,
	eOptomaUART_LAYOUT_PBP_MAIN_BOTTOM,
} eOptomaUART_MAIN_LAYOUT_MODE;        //ZU860_Doulas_0026

typedef enum
{
	eOptomaUART_PIP_SIZE_LARGE      = 1,
	eOptomaUART_PIP_SIZE_MEDIM      = 2,
	eOptomaUART_PIP_SIZE_SMALL      = 3,
} eOptomaUART_PIP_SIZE_MODE;       //ZU860_Doulas_0026

typedef enum
{// BruceLin#20210108
	eOptomaUART_MAIN_SOURCE_SET_VGA         = 5,
	eOptomaUART_MAIN_SOURCE_SET_HDMI1       = 1,
	eOptomaUART_MAIN_SOURCE_SET_HDMI2       = 15,
	eOptomaUART_MAIN_SOURCE_SET_DVI_D       = 2,
    eOptomaUART_MAIN_SOURCE_SET_DP          = 20,
    eOptomaUART_MAIN_SOURCE_SET_3G_SDI      = 22,
	eOptomaUART_MAIN_SOURCE_SET_HDBASET     = 21,
	eOptomaUART_MAIN_SOURCE_SET_12G_SDI     = 91, //TBD
} eOptomaUART_MAIN_SOURCE_SET;

typedef enum
{// BruceLin#20210108
    eOptomaUART_MAIN_SOURCE_RES_NO_SIGNAL   = 0,
	eOptomaUART_MAIN_SOURCE_RES_HDMI1       = 7,
	eOptomaUART_MAIN_SOURCE_RES_HDMI2       = 8,
	eOptomaUART_MAIN_SOURCE_RES_DVI_D       = 1,
	eOptomaUART_MAIN_SOURCE_RES_DP          = 15,
    eOptomaUART_MAIN_SOURCE_RES_3G_SDI      = 18,
	eOptomaUART_MAIN_SOURCE_RES_HDBASET     = 16,
	eOptomaUART_MAIN_SOURCE_RES_12G_SDI     = 91, //TBD
} eOptomaUART_MAIN_SOURCE_RESPONSE;

typedef enum
{// BruceLin#20210108
	eOptomaUART_SUB_SOURCE_SET_HDMI1        = 1,
	eOptomaUART_SUB_SOURCE_SET_HDMI2        = 4,
	eOptomaUART_SUB_SOURCE_SET_DVI_D        = 9,
    eOptomaUART_SUB_SOURCE_SET_3G_SDI       = 11,
	eOptomaUART_SUB_SOURCE_SET_HDBASET      = 10,
	eOptomaUART_SUB_SOURCE_SET_12G_SDI      = 91, //TBD
	eOptomaUART_SUB_SOURCE_SET_DISPLAYPORT  = 17, //H30K_Sammy_0002
} eOptomaUART_SUB_SOURCE_SET;

typedef enum
{// BruceLin#20210108
    eOptomaUART_SUB_SOURCE_RES_NO_SIGNAL    = 0,
	eOptomaUART_SUB_SOURCE_RES_HDMI1        = 7,
	eOptomaUART_SUB_SOURCE_RES_HDMI2        = 8,
	eOptomaUART_SUB_SOURCE_RES_DVI_D        = 1,
    eOptomaUART_SUB_SOURCE_RES_3G_SDI       = 17,
	eOptomaUART_SUB_SOURCE_RES_HDBASET      = 16,
	eOptomaUART_SUB_SOURCE_RES_12G_SDI      = 91, //TBD
	eOptomaUART_SUB_SOURCE_RES_DISPLAYPORT  = 15, //H30K_Sammy_0002
} eOptomaUART_SUB_SOURCE_RESPONSE;

typedef enum
{
    eOptomaUART_PROJECTION_FRONT            = 1,
	eOptomaUART_PROJECTION_REAR             = 2,
	eOptomaUART_PROJECTION_CEILING_TOP      = 3,
	eOptomaUART_PROJECTION_REAR_TOP         = 4,
} eOptomaUART_PROJECTION_SETTING;      //ZU860_Doulas_0027

typedef enum
{
	eOptomaUART_LENS_LOCK_ALL_ALLOW     = 2,    //ZU860_Doulas_0065 modify
	eOptomaUART_LENS_LOCK_ALL_LOCKED    = 1,    //ZU860_Doulas_0065 modify
} eOptomaUART_LENS_LOCK_ALL_MOTORS;        //ZU860_Doulas_0028


typedef enum
{
	eOptomaUART_LENS_LOCK_RESPONSE_ALL_ALLOW     = 1,
	eOptomaUART_LENS_LOCK_RESPONSE_ALL_LOCKED    = 0,
} eOptomaUART_LENS_LOCK_ALL_MOTORS_RESPONSE;     //ZU860_Doulas_0065

typedef enum
{
	eOptomaUART_LENS_SHIFT_UP       = 3,
	eOptomaUART_LENS_SHIFT_DOWN     = 4,
	eOptomaUART_LENS_SHIFT_LEFT     = 5,
	eOptomaUART_LENS_SHIFT_RIGHT    = 6,
} eOptomaUART_LENS_SHIFT;        //ZU860_Doulas_0028

typedef enum
{
#if 1 //correspond to remote key reverse. //A65_OPTOMA_Julie_0059
	eOptomaUART_ZOOM_PLUS           = 2,
	eOptomaUART_ZOOM_MINUS          = 1,
#else
	eOptomaUART_ZOOM_PLUS           = 1,
	eOptomaUART_ZOOM_MINUS          = 2,
#endif
} eOptomaUART_ZOOM;        //ZU860_Doulas_0028

typedef enum
{
	eOptomaUART_FOCUS_PLUS          = 1,
	eOptomaUART_FOCUS_MINUS         = 2,
	eOptomaUART_AUTO_FOCUS_EXECUTE  = 3,                    //B35LC_Tim_0026, add
} eOptomaUART_FOCUS;        //ZU860_Doulas_0028

typedef enum
{
    eOptomaUART_STANDBY_MODE_2W                = 1,    //avtice
	eOptomaUART_STANDBY_MODE_0d5W              = 0,    //ECO
	eOptomaUART_STANDBY_MODE_COMMUNICATION     = 3,
} eOptomaUART_STANDBY_MODE_SETTING;        //ZU860_Doulas_0030

typedef enum
{
    eOptomaUART_APPLY_POSITION_1                = 1,
    eOptomaUART_APPLY_POSITION_2                = 2,
    eOptomaUART_APPLY_POSITION_3                = 3,
    eOptomaUART_APPLY_POSITION_4                = 4,
	eOptomaUART_APPLY_POSITION_5                = 5,
} eOptomaUART_APPLY_POSITION;        //ZU860_Doulas_0030

typedef enum
{
    eOptomaUART_SAVE_CURRENT_POSITION_1         = 1,
    eOptomaUART_SAVE_CURRENT_POSITION_2         = 2,
    eOptomaUART_SAVE_CURRENT_POSITION_3         = 3,
    eOptomaUART_SAVE_CURRENT_POSITION_4         = 4,
	eOptomaUART_SAVE_CURRENT_POSITION_5         = 5,
} eOptomaUART_SAVE_CURRENT_POSITION;        //ZU860_Doulas_0030

typedef enum
{
	eOptomaUART_OSD_TP_OFF              = 0,
    eOptomaUART_OSD_TP_GREEN_GRID       = 3,
    eOptomaUART_OSD_TP_MAGENTA_GRID     = 4,
    eOptomaUART_OSD_TP_WHITE_GRID       = 1,
    eOptomaUART_OSD_TP_WHITE            = 2,
    eOptomaUART_OSD_TP_BLACK            = 11,
    eOptomaUART_OSD_TP_RED              = 5,
    eOptomaUART_OSD_TP_GREEN            = 6,
    eOptomaUART_OSD_TP_BLUE             = 7,
    eOptomaUART_OSD_TP_YELLOW           = 8,
    eOptomaUART_OSD_TP_MAGENTA          = 9,
    eOptomaUART_OSD_TP_CYAN             = 10,
} eOptomaUART_OSD_TEST_PATTERN;    //ZU860_Doulas_0031

typedef enum
{
    eOptomaUART_FRONT_IR_OFF            = 4,
    eOptomaUART_FRONT_IR_ON             = 5,
    eOptomaUART_TOP_IR_OFF              = 6,
    eOptomaUART_TOP_IR_ON               = 7,
	eOptomaUART_HDBASET_IR_OFF          = 10,
	eOptomaUART_HDBASET_IR_ON           = 9,
	eOptomaUART_REAR_IR_OFF             = 8,
	eOptomaUART_REAR_IR_ON              = 11,
} eOptomaUART_IR_FUNCTION;        //ZU860_Doulas_0032

typedef enum
{
	eOptomaUART_HOTKEY_SETTINGS_SIZE_PRESETS    = 1,
	eOptomaUART_HOTKEY_SETTINGS_FREEZE_SCREEN   = 2,
} eOptomaUART_HOTKEY_SETTINGS;     //ZU860_Doulas_0032

typedef enum
{
	eOptomaUART_ml_TOP_LEFT                    = 1,
	eOptomaUART_ml_TOP_RIGHT                   = 2,
	eOptomaUART_ml_CENTER                      = 3,
	eOptomaUART_ml_BOTTOM_LEFT                 = 4,
	eOptomaUART_ml_BOTTOM_RIGHT                = 5,
} eOptomaUART_MENU_LOCATION;       //ZU860_Doulas_0032

typedef enum
{
	eOptomaUART_MENU_TIME_OUT_OFF              = 0,
	eOptomaUART_MENU_TIME_OUT_5SEC             = 1,
	eOptomaUART_MENU_TIME_OUT_10SEC            = 3,
	eOptomaUART_MENU_TIME_OUT_15SEC            = 4,
} eOptomaUART_MENU_TIME_OUT;   //ZU860_Doulas_0033

typedef enum
{
	eOptomaUART_LOGO_PATTERN_FACTORY_LOGO       = 1,
	eOptomaUART_LOGO_PATTERN_NEUTRAL            = 3,
} eOptomaUART_LOGO_PATTERN;        //ZU860_Doulas_0033

typedef enum
{
	eOptomaUART_BACKGROUND_NONE            = 0,
	eOptomaUART_BACKGROUND_BLUE            = 1,
	eOptomaUART_BACKGROUND_RED             = 3,
	eOptomaUART_BACKGROUND_GREEN           = 4,
	eOptomaUART_BACKGROUND_GRAY            = 6,
	eOptomaUART_BACKGROUND_LOGO            = 7,
} eOptomaUART_BACKGROUND_COLOR;        //ZU860_Doulas_0033

typedef enum
{
	eOptomaUART_SERIAL_PORT_RS232          = 1,
	eOptomaUART_SERIAL_PORT_HDBASET        = 2,
} eOptomaUART_SERIAL_PORT_PATH;        //ZU860_Doulas_0033

typedef enum
{
	eOptomaUART_RESYNC_GO          = 1,
} eOptomaUART_RESYNC;        //ZU860_Doulas_0034

typedef enum
{
	eOptomaUART_KEY_IR_POWER_ON                 = 1,
    eOptomaUART_KEY_IR_POWER_OFF                = 2,

	eOptomaUART_KEY_IR_UP                       = 10,
    eOptomaUART_KEY_IR_LEFT                     = 11,
    eOptomaUART_KEY_IR_ENTER                    = 12,
    eOptomaUART_KEY_IR_RIGHT                    = 13,
    eOptomaUART_KEY_IR_DOWN                     = 14,
    eOptomaUART_KEY_IR_V_KEYTONE_PLUS           = 15,
    eOptomaUART_KEY_IR_V_KEYTONE_MINUS          = 16,
    //eOptomaUART_KEY_IR_VOLUME_DOWN              = 17,
    //eOptomaUART_KEY_IR_VOLUME_UP                = 18,
    eOptomaUART_KEY_IR_BRIGHTNESS               = 19,
    eOptomaUART_KEY_IR_MENU                     = 20,
    //eOptomaUART_KEY_IR_ZOOM                     = 21,
    //eOptomaUART_KEY_IR_DVI                      = 22,
    //eOptomaUART_KEY_IR_VGA_1                    = 23,
    eOptomaUART_KEY_IR_AV_MUTE                  = 24,
    //eOptomaUART_KEY_IR_S_VIDEO                  = 25,
    //eOptomaUART_KEY_IR_VGA_2                    = 26,
    //eOptomaUART_KEY_IR_VIDEO                    = 27,
    eOptomaUART_KEY_IR_CONTRAST                 = 28,
    //eOptomaUART_KEY_IR_FREEZE                   = 30,
    //eOptomaUART_KEY_IR_LENS_SHIFT               = 31,
    eOptomaUART_KEY_IR_ZOOM_MINUS               = 32,
    eOptomaUART_KEY_IR_ZOOM_PLUS                = 33,
    eOptomaUART_KEY_IR_FOCUS_PLUS               = 34,
    eOptomaUART_KEY_IR_FOCUS_MINUS              = 35,
    eOptomaUART_KEY_IR_MODE                     = 36,
    //eOptomaUART_KEY_IR_ASPECT_RATIO             = 37,
    //eOptomaUART_KEY_IR_12V_TRIGGER_ON           = 38,
    //eOptomaUART_KEY_IR_12V_TRIGGER_OFF          = 39,
    eOptomaUART_KEY_IR_INFO                     = 40,
    eOptomaUART_KEY_IR_RE_SYNC                  = 41,
    eOptomaUART_KEY_IR_HDMI1                    = 42,
    eOptomaUART_KEY_IR_HDMI2                    = 43,
    //eOptomaUART_KEY_IR_BNC                      = 44,
    //eOptomaUART_KEY_IR_COMPONENT                = 45,
    eOptomaUART_KEY_IR_SOURCE                   = 47,
    eOptomaUART_KEY_IR_1                        = 51,
    eOptomaUART_KEY_IR_2                        = 52,
    eOptomaUART_KEY_IR_3                        = 53,
    eOptomaUART_KEY_IR_4                        = 54,
    eOptomaUART_KEY_IR_5                        = 55,
    eOptomaUART_KEY_IR_6                        = 56,
    eOptomaUART_KEY_IR_7                        = 57,
    eOptomaUART_KEY_IR_8                        = 58,
    eOptomaUART_KEY_IR_9                        = 59,
    eOptomaUART_KEY_IR_0                        = 60,
    eOptomaUART_KEY_IR_GAMMA                    = 61,
    eOptomaUART_KEY_IR_PIP                      = 63,
    eOptomaUART_KEY_IR_LENS_H_LEFT              = 64,
    eOptomaUART_KEY_IR_LENS_H_RIGHT             = 65,
    eOptomaUART_KEY_IR_LENS_V_LEFT              = 66,
    eOptomaUART_KEY_IR_LENS_V_RIGHT             = 67,
    eOptomaUART_KEY_IR_H_KEYSTONE_PUUS          = 68,
    eOptomaUART_KEY_IR_H_KEYSTONE_MINUS         = 69,
    eOptomaUART_KEY_IR_HOTKEY_USER1             = 70,
    eOptomaUART_KEY_IR_HOTKEY_USER2             = 71,
    //eOptomaUART_KEY_IR_HOTKEY_USER3             = 72,
    eOptomaUART_KEY_IR_PATTERN                  = 73,
    eOptomaUART_KEY_IR_EXIT                     = 74,
    //eOptomaUART_KEY_IR_HDMI3                    = 75,
    eOptomaUART_KEY_IR_DISPLAY_PORT             = 76,
    eOptomaUART_KEY_IR_MUTE                     = 77,
    //eOptomaUART_KEY_IR_3D                       = 78,
    //eOptomaUART_KEY_IR_DB                       = 79,
    //eOptomaUART_KEY_IR_SLEEP_TIMER              = 80,
    //eOptomaUART_KEY_IR_HOME                     = 81,
    eOptomaUART_KEY_IR_RETURN                   = 82,
} eOptomaUART_KEY_IR;        //ZU860_Doulas_0035

typedef enum
{
    eOptomaUART_3D_TECH_DLP_LINK            = 1,
	eOptomaUART_3D_TECH_3D_SYNC             = 3,
} eOptomaUART_3D_TECH;       //ZU860_Doulas_0038

typedef enum
{
    eOptomaUART_3D_MODE_ON                  = 4,
	eOptomaUART_3D_MODE_OFF                 = 0,
} eOptomaUART_3D_MODE;       //ZU860_Doulas_0113

typedef enum
{
    eOptomaUART_LENS_CALIBRATION_NO             = 0,    //ZU860_Doulas_0065
    eOptomaUART_LENS_CALIBRATION_NORMAL         = 1,
	eOptomaUART_LENS_CALIBRATION_UST            = 3,
} eOptomaUART_LENS_CALIBRATION;       //ZU860_Doulas_0048

typedef enum
{
	eOptomaUART_3D_ENABLE_AUTO                  = 0,
	eOptomaUART_3D_ENABLE_FRAMEPACKING          = 7,
	eOptomaUART_3D_ENABLE_SIDEBYSIDE            = 1,
	eOptomaUART_3D_ENABLE_TOPANDBOTTOM          = 2,
	eOptomaUART_3D_ENABLE_Force3D               = 3,
	eOptomaUART_3D_ENABLE_OFF                   = 8,
} eOptomaUART_3DENABLE;    //ZU860_Doulas_0050

typedef enum
{
	eOptomaUART_3D_REFERENCE_1ST_FRAME      = 1,
	eOptomaUART_3D_REFERENCE_FIELD_GPIO     = 0,
} eOptomaUART_3D_REFERENCE;    //ZU860_Doulas_0063

typedef enum
{
	eOptomaUART_BRIGHTNESS_AD_ADD           = 2,
	eOptomaUART_BRIGHTNESS_AD_DECREASE      = 1,
} eOptomaUART_BRIGHTNESS_AD;    //ZU860_Doulas_0063

typedef enum
{
	eOptomaUART_CONTRAST_AD_ADD           = 2,
	eOptomaUART_CONTRAST_AD_DECREASE      = 1,
} eOptomaUART_CONTRAST_AD;    //ZU860_Doulas_0063

typedef enum
{
	eOptomaUART_OUTPUT_3D_STATE_2D          = 0,
	eOptomaUART_OUTPUT_3D_STATE_3D          = 1,
} eOptomaUART_OUTPUT_3D_STATE;      //ZU860_Doulas_0065

typedef enum
{
    eOptomaUART_LANG_ENGLISH        = 1,
    eOptomaUART_LANG_GERMAN,
    eOptomaUART_LANG_FRENCH,
    eOptomaUART_LANG_ITALIAN,
    eOptomaUART_LANG_SPANISH,
    eOptomaUART_LANG_PORTUGUESE,
    eOptomaUART_LANG_POLSKI,
    eOptomaUART_LANG_DUTCH,
    eOptomaUART_LANG_SWEDISH,
    eOptomaUART_LANG_DANISH,
    eOptomaUART_LANG_FINNISH,
    eOptomaUART_LANG_GREEK,
    eOptomaUART_LANG_T_CHINESE,
    eOptomaUART_LANG_S_CHINESE,
    eOptomaUART_LANG_JAPANESE,
    eOptomaUART_LANG_KOREAN,
    eOptomaUART_LANG_RUSSIAN,
    eOptomaUART_LANG_HUNGARIAN,
    eOptomaUART_LANG_CZECH,
    eOptomaUART_LANG_THAILAND       = 21,
    eOptomaUART_LANG_TURKISH        = 22,
    eOptomaUART_LANG_VIETNAMESE     = 25,
    eOptomaUART_LANG_INDONESIAN     = 26,
    eOptomaUART_LANG_ROMANIAN       = 27,
    eOptomaUART_LANG_SLOVAK         = 28,
} eOptomaUART_LANGUAGE;      //ZU860_Owen_0009

typedef enum
{
    eOptomaUART_AUTOMATIC_1         = 3,
    eOptomaUART_AUTOMATIC_2         = 4
} eOptomaUART_AUTOMATIC;    //ZU860_Owen_0019

typedef enum
{
    eOptomaUART_WARPSETTING_ALLOFF      = 0,
    eOptomaUART_WARPSETTING_ALLON       = 3,
    eOptomaUART_WARPSETTING_BLENDOFF    = 4
} eOptomaUART_WARPSETTING;    //ZU860_Owen_0019

typedef enum
{
	eOptomaUART_HSG_RESET_ALL          	= 1,
	eOptomaUART_HSG_RESET_RGBCMY        = 3,
	eOptomaUART_HSG_RESET_WHITE         = 4,
	eOptomaUART_HSG_RESET_RED         	= 5,
	eOptomaUART_HSG_RESET_GREEN        	= 6,
	eOptomaUART_HSG_RESET_BLUE         	= 7,
	eOptomaUART_HSG_RESET_CYAN         	= 8,
	eOptomaUART_HSG_RESET_MAGENTA       = 9,
	eOptomaUART_HSG_RESET_YELLOW        = 10,

} eOptomaUART_HSG_RESET;        //ZU860_Doulas_0034//H30K_David_0042

typedef enum
{
	eOptomaUART_GRID_COLOR_GREEN      	= 1,
	eOptomaUART_GRID_COLOR_MAGENTA      = 2,
	eOptomaUART_GRID_COLOR_RED      	= 3,
	eOptomaUART_GRID_COLOR_CYAN      	= 4,

} eOptomaUART_GRID_COLOR;   	//A65_OPTOMA_Doulas_0076

typedef enum
{
	eOptomaUART_GRID_BACKGROUND_BLACK      		= 1,
	eOptomaUART_GRID_BACKGROUND_TRANSPARENT     = 2,

} eOptomaUART_GRID_BACKGROUND;   	//A65_OPTOMA_Doulas_0076

typedef enum
{
	eOptomaUART_GRID_POINTS_2x2      	= 1,
	eOptomaUART_GRID_POINTS_3x3      	= 2,
	eOptomaUART_GRID_POINTS_5x5      	= 3,
	eOptomaUART_GRID_POINTS_9x9      	= 4,
	eOptomaUART_GRID_POINTS_17x17      	= 5,

} eOptomaUART_GRID_POINTS;   	//A65_OPTOMA_Doulas_0076

typedef enum
{
	eOptomaUART_OVERLAP_GRID_NUM_4      = 1,
	eOptomaUART_OVERLAP_GRID_NUM_6     	= 2,
	eOptomaUART_OVERLAP_GRID_NUM_8      = 3,
	eOptomaUART_OVERLAP_GRID_NUM_10     = 4,
	eOptomaUART_OVERLAP_GRID_NUM_12     = 5,

} eOptomaUART_OVERLAP_GRID_NUM;   	//A65_OPTOMA_Doulas_0076  Overlap_Grid_Number

typedef enum
{
	eOptomaUART_BLEND_GAMMA_1_P_8      		= 1,
	eOptomaUART_BLEND_GAMMA_1_P_9      		= 2,
	eOptomaUART_BLEND_GAMMA_2_P_0      		= 3,
	eOptomaUART_BLEND_GAMMA_2_P_1      		= 4,
	eOptomaUART_BLEND_GAMMA_2_P_2      		= 5,
	eOptomaUART_BLEND_GAMMA_2_P_3      		= 6,
	eOptomaUART_BLEND_GAMMA_2_P_4      		= 7,

} eOptomaUART_BLEND_GAMMA;   	//A65_OPTOMA_Doulas_0076

typedef enum
{
	eOptomaUART_WARP_MEMORY_SAVE1      	= 1,
	eOptomaUART_WARP_MEMORY_SAVE2     	= 2,
	eOptomaUART_WARP_MEMORY_SAVE3      	= 3,
	eOptomaUART_WARP_MEMORY_SAVE4      	= 4,
	eOptomaUART_WARP_MEMORY_SAVE5      	= 5,

} eOptomaUART_WARP_MEMORY_SAVE;   	//A65_OPTOMA_Doulas_0076

typedef enum
{
	eOptomaUART_WARP_MEMORY_APPLY1      = 1,
	eOptomaUART_WARP_MEMORY_APPLY2     	= 2,
	eOptomaUART_WARP_MEMORY_APPLY3      = 3,
	eOptomaUART_WARP_MEMORY_APPLY4      = 4,
	eOptomaUART_WARP_MEMORY_APPLY5      = 5,

} eOptomaUART_WARP_MEMORY_APPLY;   	//A65_OPTOMA_Doulas_0076

typedef enum
{
	eOptomaUART_HDMI_OUT_HDMI1      = 5,
	eOptomaUART_HDMI_OUT_HDMI2     	= 6,
} eOptomaUART_HDMI_OUT;   	//A65_OPTOMA_Doulas_0077

typedef enum
{
	eOptomaUART_HDMI1_EDID_1_4      = 1,
	eOptomaUART_HDMI1_EDID_2_0     	= 2,
} eOptomaUART_HDMI1_EDID;   	//A65_OPTOMA_Doulas_0077

typedef enum
{
	eOptomaUART_HDMI2_EDID_1_4      = 1,
	eOptomaUART_HDMI2_EDID_2_0     	= 2,
} eOptomaUART_HDMI2_EDID;   	//A65_OPTOMA_Doulas_0077

typedef enum
{
	eOptomaUART_HDBASET_EDID_1_4     = 1,
	eOptomaUART_HDBASET_EDID_2_0     = 2,
} eOptomaUART_HDBASET_EDID;   	//A65_OPTOMA_Doulas_0077

typedef enum
{
	eOptomaUART_BACKUP_RESTORE_MEMORY1      = 1,
	eOptomaUART_BACKUP_RESTORE_MEMORY2     	= 2,
	eOptomaUART_BACKUP_RESTORE_MEMORY3      = 3,
	eOptomaUART_BACKUP_RESTORE_MEMORY4      = 4,
	eOptomaUART_BACKUP_RESTORE_MEMORY5      = 5,

} eOptomaUART_BACKUP_RESTORE; //A65_OPTOMA_Julie_0037

typedef enum
{
	eOptomaUART_BACKLIGHT_KEYPAD_OFF	= 0,
	eOptomaUART_BACKLIGHT_KEYPAD_ON		= 1,
	eOptomaUART_BACKLIGHT_PWRKEY_OFF	= 3,
	eOptomaUART_BACKLIGHT_PWRKEY_ON		= 4,

} eOptomaUART_BACKLIGHT; //A65_OPTOMA_Julie_0037

typedef enum
{
    eOptomaUART_BLACK_LEVEL_RESET_BOTTOM  = 3,
    eOptomaUART_BLACK_LEVEL_RESET_TOP     = 5,
    eOptomaUART_BLACK_LEVEL_RESET_ALL	  = 1,
    
 } eOptomaUART_BLACK_LEVEL_RESET; //A65_OPTOMA_Julie_0046

 typedef enum
{
	eOptomaUART_BACKUP_INPUT_HMI1			= 7,
	eOptomaUART_BACKUP_INPUT_HMI2			= 8,
	eOptomaUART_BACKUP_INPUT_DVI	        = 1, //A35G2_Coda_0057 //A35G2_Coda_0102
	eOptomaUART_BACKUP_INPUT_HDBASET		= 15,
	
} eOptomaUART_BACKUP_INPUT_SOURCE;		//A65_OPTOMA_Doulas_0100

typedef enum	
{
	eOptomaUART_SCH_EVENT_POWER_ON			= 01,
	eOptomaUART_SCH_EVENT_POWER_OFF_STBY	= 02,
	eOptomaUART_SCH_EVENT_POWER_OFF_NSTBY	= 03,
	eOptomaUART_SCH_EVENT_POWER_OFF_CSTBY	= 04,
	eOptomaUART_SCH_EVENT_POWER_OFF			= 05,
	
} eOptomaUART_SCHEDULE_FUNCTION_POWER; //A65_OPTOMA_Julie_0084//A35G2_Coda_0045

typedef enum
{
	eOptomaUART_SCH_EVENT_INPUT_VGA			= 05,
	eOptomaUART_SCH_EVENT_INPUT_HDMI1		= 01,
	eOptomaUART_SCH_EVENT_INPUT_HDMI2		= 15,
	eOptomaUART_SCH_EVENT_INPUT_DVI  		= 02,
	eOptomaUART_SCH_EVENT_INPUT_DP  		= 20,
	eOptomaUART_SCH_EVENT_INPUT_3GSDI		= 22,
	eOptomaUART_SCH_EVENT_INPUT_HDBT		= 21,
	
} eOptomaUART_SCHEDULE_FUNCTION_INPUT; //A65_OPTOMA_Julie_0084
	
typedef enum
{
	eOptomaUART_SCH_EVENT_LIGHT_NORMAL		= 01,
	eOptomaUART_SCH_EVENT_LIGHT_ECO  		= 02,
	eOptomaUART_SCH_EVENT_LIGHT_CUSTOM		= 03,

} eOptomaUART_SCHEDULE_FUNCTION_LIGHT; //A65_OPTOMA_Julie_0084

typedef enum
{
	eOptomaUART_SCH_EVENT_SHUTTER_ON		= 01,
	eOptomaUART_SCH_EVENT_SHUTTER_OFF  		= 02,

} eOptomaUART_SCHEDULE_FUNCTION_SHUTTER; //A65_OPTOMA_Julie_0084

typedef enum
{
	eOptomaUART_CLOCK_NTP_SERVER		= 01,
	eOptomaUART_CLOCK_MANUAL	  		= 03,

} eOptomaUART_CLOCK_MODE; //A65_OPTOMA_Julie_0084

typedef enum
{
	eOptomaUART_INTERVAL_HOURLY		= 01,
	eOptomaUART_INTERVAL_DAILY 		= 03,

} eOptomaUART_UPDATE_INTERVAL; //A65_OPTOMA_Julie_0084

typedef enum
{
	eOptomaUART_LOGO_CHANGE_DEFAULT		= 1,
	eOptomaUART_LOGO_CHANGE_NEUTRAL		= 3,
	eOptomaUART_LOGO_CHANGE_CUSTOM 		= 4,
	eOptomaUART_LOGO_CHANGE_CAPTURED	= 2,

} eOptomaUART_LOGO_CHANGE;	//A65_OPTOMA_Julie_0088


#define OPTOMA_CMD_HEADER_1       '~'
#define OPTOMA_CMD_END_1          '\r'

#define OPTOMA_CMD_TEXT_SIZE      (150)
#define OPTOMA_MAIN_CMD_LEN       (3)
#define OPTOMA_ADDRESS_LEN_MAX    (2)


typedef struct
{
    char cMainCode[OPTOMA_MAIN_CMD_LEN + 1]; //+1 for '\0'

    UINT8 ucCmdFrom; //from PC or telnet ...    //T100_Simon_0020
    char  cTextString[OPTOMA_CMD_TEXT_SIZE + 1]; //store text prameter in Data. //+1 for '\0'
    INT32 lData;
    float fData;
    UINT8 ucHasSubCode;
    UINT8 ucDataType; // 0--> Decimal, 1:String.
    UINT8 ucIsRead;  //Write, Read
    UINT8 ucPrefixCharType; //$, #, &

    char uiProjectorAddress[OPTOMA_ADDRESS_LEN_MAX + 1]; //+1 for '\0'
    UINT32 ulProjectorAddressID;

    UINT16 uiFuncID;
}sCLI_OPTOMA_FORMAT;

typedef enum
{
	eOPT_STANDBY_MODE,
	eOPT_WARMING_UP,
	eOPT_COOLING_DOWN,
	eOPT_OUT_OF_RANGE,
	eOPT_LIGHTSOURCE_FAIL,
	eOPT_THERMAL_SWITCH_ERROR,
	eOPT_FAN_LOCK,
	eOPT_OVER_TEMPERATURE_1,
	eOPT_LIGHTSOURCE_HOURS_RUNNING_OUT,
	eOPT_COVER_OPEN,
	eOPT_LIGHTSOURCE_IGNITE_FAIL,
	eOPT_FORMAT_BOARD_POWER_ON_FAIL,
	eOPT_COLOR_WHEEL_UNEXPECTED_STOP,
	eOPT_OVER_TEMPERATURE_2,
	eOPT_FAN_1_LOCK,
	eOPT_FAN_2_LOCK,
	eOPT_FAN_3_LOCK,
	eOPT_FAN_4_LOCK,
	eOPT_FAN_5_LOCK,
	eOPT_LAN_FAIL_THEN_RESTART,
	eOPT_LD_LOWER_THAN_60,
	eOPT_LD_NTC_1_OVER_TEMPERATURE,
	eOPT_LD_NTC_2_OVER_TEMPERATURE,
	eOPT_HIGH_AMBIENT_TEMPERATURE,
	eOPT_SYSTEM_READY,
	eOPT_NULL,
	eOPT_FAN_6_LOCK,
	eOPT_FAN_7_LOCK,
	eOPT_FAN_8_LOCK,
	eOPT_FAN_9_LOCK,
	eOPT_FAN_10_LOCK,
	eOPT_FAN_11_LOCK,
	eOPT_FAN_12_LOCK,
	eOPT_FAN_13_LOCK,
	eOPT_FAN_14_LOCK,

	eOPT_INFO_INVALID

}eOPTOMA_INFO_INDEX;

typedef struct
{
    UINT8 ucINFO_Index;
    char *cINFO_String;
	
}sOPTOMA_INFO_TABLE;

static const sOPTOMA_INFO_TABLE sOptoma_System_Auto_Send_Lut[eOPT_INFO_INVALID] =
{   
	{ eOPT_STANDBY_MODE,						"INFO0\r"}, //HICC2_AC_0013
	{ eOPT_WARMING_UP,							"INFO1\r"},
	{ eOPT_COOLING_DOWN,						"INFO2\r"},
	{ eOPT_OUT_OF_RANGE,						"INFO3\r"},
	{ eOPT_LIGHTSOURCE_FAIL,					"INFO4\r"},
	{ eOPT_THERMAL_SWITCH_ERROR,				"INFO5\r"},
	{ eOPT_FAN_LOCK,							"INFO6\r"},
	{ eOPT_OVER_TEMPERATURE_1,					"INFO7\r"},
	{ eOPT_LIGHTSOURCE_HOURS_RUNNING_OUT,		"INFO8\r"},
	{ eOPT_COVER_OPEN,							"INFO9\r"},
	{ eOPT_LIGHTSOURCE_IGNITE_FAIL, 			"INFO10\r"},
	{ eOPT_FORMAT_BOARD_POWER_ON_FAIL,			"INFO11\r"},
	{ eOPT_COLOR_WHEEL_UNEXPECTED_STOP, 		"INFO12\r"},
	{ eOPT_OVER_TEMPERATURE_2,					"INFO13\r"},
	{ eOPT_FAN_1_LOCK,							"INFO14\r"},
	{ eOPT_FAN_2_LOCK,							"INFO15\r"},
	{ eOPT_FAN_3_LOCK,							"INFO16\r"},
	{ eOPT_FAN_4_LOCK,							"INFO17\r"},
	{ eOPT_FAN_5_LOCK,							"INFO18\r"},
	{ eOPT_LAN_FAIL_THEN_RESTART,				"INFO19\r"},
	{ eOPT_LD_LOWER_THAN_60,					"INFO20\r"},
	{ eOPT_LD_NTC_1_OVER_TEMPERATURE,			"INFO21\r"},
	{ eOPT_LD_NTC_2_OVER_TEMPERATURE,			"INFO22\r"},
	{ eOPT_HIGH_AMBIENT_TEMPERATURE,			"INFO23\r"},
	{ eOPT_SYSTEM_READY,						"INFO24\r"},
	{ eOPT_NULL,								"INFO25\r"},
	{ eOPT_FAN_6_LOCK,							"INFO26\r"},
	{ eOPT_FAN_7_LOCK,							"INFO27\r"},
	{ eOPT_FAN_8_LOCK,							"INFO28\r"},
	{ eOPT_FAN_9_LOCK,							"INFO29\r"},
	{ eOPT_FAN_10_LOCK, 						"INFO30\r"},
	{ eOPT_FAN_11_LOCK, 						"INFO31\r"},
	{ eOPT_FAN_12_LOCK, 						"INFO32\r"},
	{ eOPT_FAN_13_LOCK, 						"INFO33\r"},
	{ eOPT_FAN_14_LOCK, 						"INFO34\r"},

};

void utilOptoma_CLI_Init(void);
void utilOptoma_CLI_AddressID(UINT32 ulAddress);
UINT8 utilOptoma_CLI_Handle(UINT8 eCh, UINT8 ucData);
void utilOptoma_CLI_SerialPortEcho(UINT8 eCh, UINT8 ucEcho); //A70LV_Larry_0060
UINT8 utilOptoma_CLI_Process(UINT8 cCh, UINT8* pcString, char* pcReturnString, sCLI_OPTOMA_FORMAT* sCmdFormat, UINT16 uiType);
void utilOptoma_CLI_StringOutput(UINT8 eCh, char *ucOutputString);       //ZU860_Doulas_0043
UINT8 utilOptoma_CLI_Execute(UINT8 cCh, UINT8* pcString, char* pcReturnString, sCLI_OPTOMA_FORMAT* sCmdFormat);
void utilOptoma_OutputInfoMsg(char *ucOutputString);
void palSystem_Optoma_Error_Message(UINT32 ulErrorIndex);


#ifdef __cplusplus
}
#endif

#endif /* UTILOPTOMAMSSCAPI_H */

#endif //defined(CUSTOM_OPTOMA)

