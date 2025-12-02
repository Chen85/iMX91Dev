
#ifndef _UTILCOMMON_H_
#define _UTILCOMMON_H_

#include "Common.h"

#if (SYSTEM_OS_TYPE == FREERTOS)
#include "./FreeRTOS_Kernel/include/FreeRTOS.h"
#include "./FreeRTOS_Kernel/include/task.h"
#include "./FreeRTOS_Kernel/include/queue.h"
#include "./FreeRTOS_Kernel/include/semphr.h"
#include "./FreeRTOS_Kernel/include/event_groups.h"
#endif

#include "Release.h"



//#define QUICKLY_POWER_ON	//G100_Doulas_0024
#define SUPPORT_WUXGA_120HZ_3D_PANEL    	//H30K_Doulas_0053//G100_Doulas_0064 not opened
#ifdef SUPPORT_WUXGA_120HZ_3D_PANEL     	//G100_Doulas_0064
    #define AUTO_ASPECT_RATIO_WITH_3D       //G100_Doulas_0064
    #define AUTO_ASPECT_RATIO_WITH_3D_WU120
#endif
#define FRAME_SEQUENTIAL_USE_C821_OUPUT_3D_SYNC       //G100_Doulas_0071

//A35G2_Simon_0063
#ifdef CUSTOM_CHRISTIE
#define ADVANCED_WARP           (TRUE)
#define ADVANCED_WARP_OSD       (FALSE)
#define ADVANCED_WARP_WEB       (TRUE)
#define ADVANCED_BLEND          (FALSE)
#define ADVANCED_BLACK_LEVEL    (FALSE)
#define CMD_ONLY_BLACK_LEVEL    (FALSE)
#elif CUSTOM_BARCO
#define ADVANCED_WARP           (TRUE)
#define ADVANCED_WARP_OSD       (TRUE)
#define ADVANCED_WARP_WEB       (FALSE)
#define ADVANCED_BLEND          (TRUE)
#define ADVANCED_BLACK_LEVEL    (FALSE)
#define CMD_ONLY_BLACK_LEVEL    (TRUE)
#elif CUSTOM_OPTOMA
#define ADVANCED_WARP           (TRUE)
#define ADVANCED_WARP_OSD       (TRUE)
#define ADVANCED_WARP_WEB       (FALSE)
#define ADVANCED_BLEND          (TRUE)
#define ADVANCED_BLACK_LEVEL    (TRUE)
#define CMD_ONLY_BLACK_LEVEL    (FALSE)
#else
#define ADVANCED_WARP           (TRUE)
#define ADVANCED_WARP_OSD       (TRUE)
#define ADVANCED_WARP_WEB       (FALSE)
#define ADVANCED_BLEND          (TRUE)  //C341 wait review first
#define ADVANCED_BLACK_LEVEL    (TRUE)  //simon temp close  //C341 wait review first
#define CMD_ONLY_BLACK_LEVEL    (FALSE)
#endif

#ifdef CUSTOM_WINCOMN     					//A35G2_Simon_0110
#define WARPING_AP_AUTO_FILTER         (FALSE)
#define ADAPTIVE_SCALE_WARP_FILTER_LUT (FALSE)   //A35G2_Simon_0115
#elif defined(CUSTOM_CHRISTIE)
#define WARPING_AP_AUTO_FILTER         (TRUE)
#define ADAPTIVE_SCALE_WARP_FILTER_LUT (TRUE)    //A35G2_Simon_0115
#else
#define WARPING_AP_AUTO_FILTER         (TRUE)
#define ADAPTIVE_SCALE_WARP_FILTER_LUT (FALSE)   //A35G2_Simon_0115
#endif





#define DATA_TYPE_NA             0
#define DATA_TYPE_STRING         1
#define DATA_TYPE_UI_DIGIT_8     2 //unsigned int 8 bits
#define DATA_TYPE_UI_DIGIT_16    3 //unsigned int 16 bits
#define DATA_TYPE_UI_DIGIT_32    4 //unsigned int 32 bits
#define DATA_TYPE_UI_DIGIT_64    5 //unsigned int 64 bits
#define DATA_TYPE_I_DIGIT_8      6 //int 8 bits
#define DATA_TYPE_I_DIGIT_16     7 //int 16 bits
#define DATA_TYPE_I_DIGIT_32     8 //int 32 bits
#define DATA_TYPE_I_DIGIT_64     9 //int 64 bits
#define DATA_TYPE_FLOAT          10


#define DIGITAL_HORZ_ZOOM_DEFAULT   100     //A70LV_Doulas_0009
#define DIGITAL_VERT_ZOOM_DEFAULT   100     //A70LV_Doulas_0009
#define DIGITAL_HORZ_ZOOM_MIN       50      //A70LV_Doulas_0020
#define DIGITAL_VERT_ZOOM_MIN       50
#define DIGITAL_HORZ_ZOOM_MAX       400
#define DIGITAL_VERT_ZOOM_MAX       400

#define DIGITAL_HORZ_SHIFT_DEFAULT   50     //A70LV_Doulas_0009
#define DIGITAL_HORZ_SHIFT_MAX      100     //A70LV_Doulas_0009
#define DIGITAL_HORZ_SHIFT_MIN        0     //A70LV_Doulas_0009
#define DIGITAL_VERT_SHIFT_DEFAULT   50     //A70LV_Doulas_0009
#define DIGITAL_VERT_SHIFT_MAX      100     //A70LV_Doulas_0009
#define DIGITAL_VERT_SHIFT_MIN        0     //A70LV_Doulas_0009

#define HORZ_POSITION_DEFAULT        50     //A70LV_Doulas_0010
#define HORZ_POSITION_MIN             0     //A70LV_Doulas_0010
#define HORZ_POSITION_MAX           100     //A70LV_Doulas_0010
#define VERT_POSITION_DEFAULT        50     //A70LV_Doulas_0010
#define VERT_POSITION_MIN             0     //A70LV_Doulas_0010
#define VERT_POSITION_MAX           100     //A70LV_Doulas_0010

#define BRIGHTNESS_MAX_VALUE 100
#define BRIGHTNESS_DEFAULT_VALUE 50
#define BRIGHTNESS_MIN_VALUE 0

#define CONTRAST_MAX_VALUE 100
#define CONTRAST_DEFAULT_VALUE 50
#define CONTRAST_MIN_VALUE 0

#define RED_OFFSET_MAX_VALUE                100      //A70LV_Doulas_0011
#define RED_OFFSET_DEFAULT_VALUE            50
#define RED_OFFSET_MIN_VALUE                0

#define GREEN_OFFSET_MAX_VALUE              100      //A70LV_Doulas_0011
#define GREEN_OFFSET_DEFAULT_VALUE          50
#define GREEN_OFFSET_MIN_VALUE              0

#define BLUE_OFFSET_MAX_VALUE               100      //A70LV_Doulas_0011
#define BLUE_OFFSET_DEFAULT_VALUE           50
#define BLUE_OFFSET_MIN_VALUE               0

#define RED_GAIN_MAX_VALUE                 100      //A70LV_Doulas_0011
#define RED_GAIN_DEFAULT_VALUE              50
#define RED_GAIN_MIN_VALUE                  0

#define GREEN_GAIN_MAX_VALUE                100      //A70LV_Doulas_0011
#define GREEN_GAIN_DEFAULT_VALUE            50
#define GREEN_GAIN_MIN_VALUE                0

#define BLUE_GAIN_MAX_VALUE                 100      //A70LV_Doulas_0011
#define BLUE_GAIN_DEFAULT_VALUE             50
#define BLUE_GAIN_MIN_VALUE                 0

#define HUE_MAX_VALUE                       100          //A70LV_Doulas_0012
#define HUE_DEFAULT_VALUE                   50
#define HUE_MIN_VALUE                       0
#define SATURATION_MAX_VALUE                100
#define SATURATION_DEFAULT_VALUE            50
#define SATURATION_MIN_VALUE                0

#define WARPMODE_MAX_VALUE 4 //A70LV_Larry_0053
#define WARPMODE_DEFAULT_VALUE 0
#define WARPMODE_MIN_VALUE 0

#define MOVEPITCH_MAX_VALUE 4 //A70LV_Larry_0053
#define MOVEPITCH_DEFAULT_VALUE 0
#define MOVEPITCH_MIN_VALUE 0

#define KEYSTONE_MAX_VALUE 40
#define KEYSTONE_DEFAULT_VALUE 20
#define KEYSTONE_MIN_VALUE 0

#define HORZ_PINCUSHIN_MAX_VALUE        100       //A70LV_Doulas_0020
#define HORZ_PINCUSHIN_DEFAULT_VALUE    50
#define HORZ_PINCUSHIN_MIN_VALUE        0

#define VERT_PINCUSHIN_MAX_VALUE        100       //A70LV_Doulas_0020
#define VERT_PINCUSHIN_DEFAULT_VALUE    50
#define VERT_PINCUSHIN_MIN_VALUE        0

//4 corner
#ifdef CUSTOM_OPTOMA   //temp code for add range     //H2PF_Simon_0130
#define _4CORNER_TL_HORZ_MAX_VALUE        500
#define _4CORNER_TL_HORZ_DEFAULT_VALUE    0
#define _4CORNER_TL_HORZ_MIN_VALUE        0

#define _4CORNER_TL_VERT_MAX_VALUE        250
#define _4CORNER_TL_VERT_DEFAULT_VALUE    0
#define _4CORNER_TL_VERT_MIN_VALUE        0

#define _4CORNER_TR_HORZ_MAX_VALUE        500
#define _4CORNER_TR_HORZ_DEFAULT_VALUE    0
#define _4CORNER_TR_HORZ_MIN_VALUE        0

#define _4CORNER_TR_VERT_MAX_VALUE        250
#define _4CORNER_TR_VERT_DEFAULT_VALUE    0
#define _4CORNER_TR_VERT_MIN_VALUE        0

#define _4CORNER_BL_HORZ_MAX_VALUE        500
#define _4CORNER_BL_HORZ_DEFAULT_VALUE    0
#define _4CORNER_BL_HORZ_MIN_VALUE        0

#define _4CORNER_BL_VERT_MAX_VALUE        250
#define _4CORNER_BL_VERT_DEFAULT_VALUE    0
#define _4CORNER_BL_VERT_MIN_VALUE        0

#define _4CORNER_BR_HORZ_MAX_VALUE        500
#define _4CORNER_BR_HORZ_DEFAULT_VALUE    0
#define _4CORNER_BR_HORZ_MIN_VALUE        0

#define _4CORNER_BR_VERT_MAX_VALUE        250
#define _4CORNER_BR_VERT_DEFAULT_VALUE    0
#define _4CORNER_BR_VERT_MIN_VALUE        0

#else
#define _4CORNER_TL_HORZ_MAX_VALUE        120
#define _4CORNER_TL_HORZ_DEFAULT_VALUE    0
#define _4CORNER_TL_HORZ_MIN_VALUE        0

#define _4CORNER_TL_VERT_MAX_VALUE        80
#define _4CORNER_TL_VERT_DEFAULT_VALUE    0
#define _4CORNER_TL_VERT_MIN_VALUE        0

#define _4CORNER_TR_HORZ_MAX_VALUE        120
#define _4CORNER_TR_HORZ_DEFAULT_VALUE    0
#define _4CORNER_TR_HORZ_MIN_VALUE        0

#define _4CORNER_TR_VERT_MAX_VALUE        80
#define _4CORNER_TR_VERT_DEFAULT_VALUE    0
#define _4CORNER_TR_VERT_MIN_VALUE        0

#define _4CORNER_BL_HORZ_MAX_VALUE        120
#define _4CORNER_BL_HORZ_DEFAULT_VALUE    0
#define _4CORNER_BL_HORZ_MIN_VALUE        0

#define _4CORNER_BL_VERT_MAX_VALUE        80
#define _4CORNER_BL_VERT_DEFAULT_VALUE    0
#define _4CORNER_BL_VERT_MIN_VALUE        0

#define _4CORNER_BR_HORZ_MAX_VALUE        120
#define _4CORNER_BR_HORZ_DEFAULT_VALUE    0
#define _4CORNER_BR_HORZ_MIN_VALUE        0

#define _4CORNER_BR_VERT_MAX_VALUE        80
#define _4CORNER_BR_VERT_DEFAULT_VALUE    0
#define _4CORNER_BR_VERT_MIN_VALUE        0
#endif

#ifdef SCALER_FPGA_F34
//Manual Warp Filter
#define HORZ_FILTER_MAX_VALUE        19
#define HORZ_FILTER_DEFAULT_VALUE    19
#define HORZ_FILTER_MIN_VALUE        0
#else
//Manual Warp Filter
#define HORZ_FILTER_MAX_VALUE        9
#define HORZ_FILTER_DEFAULT_VALUE    0
#define HORZ_FILTER_MIN_VALUE        0
#endif /* SCALER_FPGA_F34 */

#define VERT_FILTER_MAX_VALUE        9
#define VERT_FILTER_DEFAULT_VALUE    0
#define VERT_FILTER_MIN_VALUE        0

//ADV Warp Sharpness					//G100_Doulas_0027
#ifdef SCALER_FPGA_F34
#define WARP_SHARPNESS_MAX_VALUE 			19
#define WARP_SHARPNESS_DEFAULT_VALUE   		19
#define WARP_SHARPNESS_MIN_VALUE 			0
#else
#define WARP_SHARPNESS_MAX_VALUE 			9
#define WARP_SHARPNESS_DEFAULT_VALUE   		9
#define WARP_SHARPNESS_MIN_VALUE 			0
#endif

//ADV BLEND OVERLAP GRID NUMBER			//G100_Doulas_0027
#define BLEND_OVERLAP_GRID_NUMBER_MAX_VALUE 			4
#define BLEND_OVERLAP_GRID_NUMBER_DEFAULT_VALUE   		0
#define BLEND_OVERLAP_GRID_NUMBER_MIN_VALUE 			0

//ADV BLENDING Left/Right/Top/Bottom			//G100_Doulas_0027
#define ADV_BLENDING_LEFT_MAX_VALUE 			960
#define ADV_BLENDING_LEFT_DEFAULT_VALUE   		0
#define ADV_BLENDING_LEFT_MIN_VALUE 			0
#define ADV_BLENDING_RIGHT_MAX_VALUE 			960
#define ADV_BLENDING_RIGHT_DEFAULT_VALUE   		0
#define ADV_BLENDING_RIGHT_MIN_VALUE 			0
#define ADV_BLENDING_TOP_MAX_VALUE 				600
#define ADV_BLENDING_TOP_DEFAULT_VALUE   		0
#define ADV_BLENDING_TOP_MIN_VALUE 				0
#define ADV_BLENDING_BOTTOM_MAX_VALUE 			600
#define ADV_BLENDING_BOTTOM_DEFAULT_VALUE   	0
#define ADV_BLENDING_BOTTOM_MIN_VALUE 			0


//Blending area
#define BLENDING_TOP_START_PIXEL_MAX_VALUE              200       //A70LV_Doulas_0021
#define BLENDING_TOP_START_PIXEL_DEF_VALUE          	0
#define BLENDING_TOP_START_PIXEL_MIN_VALUE              0

#define BLENDING_TOP_PIXEL_WIDTH_MAX_VALUE              500

#define _2K_    //2K Platform

#ifdef SCALER_FPGA_F34 //HICC2_Steven_0055 start
#define BLENDING_TOP_PIXEL_WIDTH_DEF_VALUE          	10
#define BLENDING_TOP_PIXEL_WIDTH_MIN_VALUE              10

#define BLENDING_BOTTOM_START_PIXEL_MAX_VALUE           200
#define BLENDING_BOTTOM_START_PIXEL_DEF_VALUE       	0
#define BLENDING_BOTTOM_START_PIXEL_MIN_VALUE           0

#define BLENDING_BOTTOM_PIXEL_WIDTH_MAX_VALUE           500
#define BLENDING_BOTTOM_PIXEL_WIDTH_DEF_VALUE       	10
#define BLENDING_BOTTOM_PIXEL_WIDTH_MIN_VALUE           10

#define BLENDING_LEFT_START_PIXEL_MAX_VALUE             200
#define BLENDING_LEFT_START_PIXEL_DEF_VALUE         	0
#define BLENDING_LEFT_START_PIXEL_MIN_VALUE             0

#define BLENDING_LEFT_PIXEL_WIDTH_MAX_VALUE             800
#define BLENDING_LEFT_PIXEL_WIDTH_DEF_VALUE         	10
#define BLENDING_LEFT_PIXEL_WIDTH_MIN_VALUE             10

#define BLENDING_RIGHT_START_PIXEL_MAX_VALUE            200
#define BLENDING_RIGHT_START_PIXEL_DEF_VALUE        	0
#define BLENDING_RIGHT_START_PIXEL_MIN_VALUE            0

#define BLENDING_RIGHT_PIXEL_WIDTH_MAX_VALUE            800
#define BLENDING_RIGHT_PIXEL_WIDTH_DEF_VALUE        	10
#define BLENDING_RIGHT_PIXEL_WIDTH_MIN_VALUE            10
#else
#define BLENDING_TOP_PIXEL_WIDTH_DEF_VALUE          	4
#define BLENDING_TOP_PIXEL_WIDTH_MIN_VALUE              4

#define BLENDING_BOTTOM_START_PIXEL_MAX_VALUE           200
#define BLENDING_BOTTOM_START_PIXEL_DEF_VALUE       	0
#define BLENDING_BOTTOM_START_PIXEL_MIN_VALUE           0

#define BLENDING_BOTTOM_PIXEL_WIDTH_MAX_VALUE           500
#define BLENDING_BOTTOM_PIXEL_WIDTH_DEF_VALUE       	4
#define BLENDING_BOTTOM_PIXEL_WIDTH_MIN_VALUE           4

#define BLENDING_LEFT_START_PIXEL_MAX_VALUE             200
#define BLENDING_LEFT_START_PIXEL_DEF_VALUE         	0
#define BLENDING_LEFT_START_PIXEL_MIN_VALUE             0

#define BLENDING_LEFT_PIXEL_WIDTH_MAX_VALUE             800
#define BLENDING_LEFT_PIXEL_WIDTH_DEF_VALUE         	4
#define BLENDING_LEFT_PIXEL_WIDTH_MIN_VALUE             4

#define BLENDING_RIGHT_START_PIXEL_MAX_VALUE            200
#define BLENDING_RIGHT_START_PIXEL_DEF_VALUE        	0
#define BLENDING_RIGHT_START_PIXEL_MIN_VALUE            0

#define BLENDING_RIGHT_PIXEL_WIDTH_MAX_VALUE            800
#define BLENDING_RIGHT_PIXEL_WIDTH_DEF_VALUE        	4
#define BLENDING_RIGHT_PIXEL_WIDTH_MIN_VALUE            4
#endif //HICC2_Steven_0055 end

#ifdef SCALER_FPGA_F34
#define FRAME_DELAY_MAX_VALUE               254
#define FRAME_DELAY_DEFAULT_VALUE           0//2            //A70LV_Doulas_0225 modify frame delay default
#define FRAME_DELAY_MIN_VALUE               0

#define SYNC_DELAY_3D_DEFAUL_ACTIVE         188
#define SYNC_DELAY_3D_DEFAUL_PASSIVE        209
#else
#ifdef CUSTOM_OPTOMA
#define FRAME_DELAY_MAX_VALUE               500//200        //H30K_Doulas_0003
#else
#define FRAME_DELAY_MAX_VALUE               500
#endif /* CUSTOM_OPTOMA */
#define FRAME_DELAY_DEFAULT_VALUE           1//2            //A70LV_Doulas_0225 modify frame delay default
#define FRAME_DELAY_MIN_VALUE               1
#endif /* SCALER_FPGA_F34 */

#define SYNC_DELAY_3D_MAX_VALUE             254
#define SYNC_DELAY_3D_DEFAULT_VALUE         0
#define SYNC_DELAY_3D_MIN_VALUE             0

#if defined(CUSTOM_BARCO) || defined(CUSTOM_OPTOMA)  //A35G2_BRC_Casper_0014
#define WHITE_PEAKING_MAX_VALUE             10
#define WHITE_PEAKING_DEFAULT_VALUE         10
#define WHITE_PEAKING_MIN_VALUE             0
#define WHITE_PEAKING_STEP_VALUE            1
#else
#define WHITE_PEAKING_MAX_VALUE             100
#define WHITE_PEAKING_DEFAULT_VALUE         100
#define WHITE_PEAKING_MIN_VALUE             0
#define WHITE_PEAKING_STEP_VALUE            10
#endif


#define LENS_APPLY_POSITION_MAX_VALUE             4      //A70LV_Doulas_0023
#define LENS_APPLY_POSITION_DEFAULT_VALUE         0
#define LENS_APPLY_POSITION_MIN_VALUE             0

#define LENS_SAVE_CURRENT_POSITION_MAX_VALUE      4
#define LENS_SAVE_CURRENT_POSITION_DEFAULT_VALUE  0
#define LENS_SAVE_CURRENT_POSITION_MIN_VALUE      0

#define MENU_TRANSPARENCY_MAX_VALUE               90
#define MENU_TRANSPARENCY_DEFAULT_VALUE           0
#define MENU_TRANSPARENCY_MIN_VALUE               0
#ifdef CUSTOM_OPTOMA
#define MENU_TRANSPARENCY_STEP_VALUE              10// BruceLin#20201130
#else
#define MENU_TRANSPARENCY_STEP_VALUE              5     //A70LV_Doulas_0122
#endif
#define MEMU_HORZ_OFFSET_MAX_VALUE              100
#define MEMU_HORZ_OFFSET_DEFAULT_VALUE          0
#define MEMU_HORZ_OFFSET_MIN_VALUE              0

#define MEMU_VERT_OFFSET_MAX_VALUE              100
#define MEMU_VERT_OFFSET_DEFAULT_VALUE          0
#define MEMU_VERT_OFFSET_MIN_VALUE              0

#ifndef CUSTOM_CHRISTIE
#define PROJECTOR_ADDRESS_MAX_VALUE             99       //G100_Wilsonj_0036
#else
#define PROJECTOR_ADDRESS_MAX_VALUE             9
#endif
#define PROJECTOR_ADDRESS_DEFAULT_VALUE         0
#define PROJECTOR_ADDRESS_MIN_VALUE             0

#define CONSTANT_POWER_NUMBER_MAX_VALUE         100      //A70LV_Doulas_0024
#define CONSTANT_POWER_NUMBER_DEFAULT_VALUE     100
#ifdef CUSTOM_OPTOMA
#define CONSTANT_POWER_NUMBER_MIN_VALUE         10// BruceLin#20201209 //HICC2_AC_0011
#else
#define CONSTANT_POWER_NUMBER_MIN_VALUE         1//0    //A70LV_Doulas_0074 modify
#endif
#ifdef CUSTOM_OPTOMA
#define KEYPAD_BACKLIGHT_MAX_VALUE              eCM_KEYPAD_BACKLIGHT_ALWAYS_OFF
#define KEYPAD_BACKLIGHT_MIN_VALUE              eCM_KEYPAD_BACKLIGHT_ALWAYS_ON
#define KEYPAD_BACKLIGHT_DEF_VALUE              eCM_KEYPAD_BACKLIGHT_ALWAYS_ON

#define HOTKEY1_DEF_VALUE						eCM_HOTKEY_SETTINGS_HDMI1
#define HOTKEY2_DEF_VALUE						eCM_HOTKEY_SETTINGS_HDMI2
#else
#define KEYPAD_BACKLIGHT_MAX_VALUE              eCM_KEYPAD_BACKLIGHT_ALWAYS_OFF
#define KEYPAD_BACKLIGHT_MIN_VALUE              eCM_KEYPAD_BACKLIGHT_5_SECS
#define KEYPAD_BACKLIGHT_DEF_VALUE              eCM_KEYPAD_BACKLIGHT_5_SECS

#define HOTKEY1_DEF_VALUE 						eCM_HOTKEY_SETTINGS_SIZE_PRESETS
#define HOTKEY2_DEF_VALUE						eCM_HOTKEY_SETTINGS_PROJECTOR_ID
#endif

//ZU860_John_0001 start add power offset protection
#define POWER_OFFSET_MAX 						20
#define POWER_OFFSET_DEFAULT 					0
#define POWER_OFFSET_MIN 						0
//ZU860_John_0001 end

//OE CPC
#if 0  //move to Platform.h
#define PW_INDEX_MAX_VALUE 719
#define PW_INDEX_DEFAULT_VALUE 285
#define PW_INDEX_MIN_VALUE 0

#define FW_INDEX_MAX_VALUE 719
#define FW_INDEX_DEFAULT_VALUE 425
#define FW_INDEX_MIN_VALUE 0
#endif

#define VERSION_STRING_MAX_LENGTH 32
#define VERSION_STRING_MIN_LENGTH 0

#define PHASE_MAX_VALUE             100
#define PHASE_DEFAULT_VALUE         50
#define PHASE_MIN_VALUE             0
#define VGA_MAX_PHASE   31  //A70LV_Doulas_0007

#define TRACKING_MAX_VALUE          100
#define TRACKING_DEFAULT_VALUE      50
#define TRACKING_MIN_VALUE          0

//For Burnin
#define FW_BURNIN_TIME_MAX_VALUE 240
#define FW_BURNIN_TIME_ON_DEFAULT_VALUE 120
#define FW_BURNIN_TIME_OFF_DEFAULT_VALUE 15
#define FW_BURNIN_TIME_MIN_VALUE 1

#define FW_BURNIN_CYCLE_MAX_VALUE 999
#define FW_BURNIN_CYCLE_DEFAULT_VALUE 10
#define FW_BURNIN_CYCLE_MIN_VALUE 0

//ManualAdjustment   //A70LV_Doulas_0015
#define MAX_CCN_RED_OF_RED   1000
#define MIN_CCN_RED_OF_RED      0
#define INI_CCN_RED_OF_RED   1000

#define MAX_CCN_GREEN_OF_RED   1000
#define MIN_CCN_GREEN_OF_RED      0
#define INI_CCN_GREEN_OF_RED      0

#define MAX_CCN_BLUE_OF_RED   1000
#define MIN_CCN_BLUE_OF_RED      0
#define INI_CCN_BLUE_OF_RED      0

#define MAX_CCN_RED_OF_GREEN   1000
#define MIN_CCN_RED_OF_GREEN      0
#define INI_CCN_RED_OF_GREEN      0

#define MAX_CCN_GREEN_OF_GREEN   1000
#define MIN_CCN_GREEN_OF_GREEN      0
#define INI_CCN_GREEN_OF_GREEN   1000

#define MAX_CCN_BLUE_OF_GREEN   1000
#define MIN_CCN_BLUE_OF_GREEN      0
#define INI_CCN_BLUE_OF_GREEN      0

#define MAX_CCN_RED_OF_BLUE   1000
#define MIN_CCN_RED_OF_BLUE      0
#define INI_CCN_RED_OF_BLUE      0

#define MAX_CCN_GREEN_OF_BLUE   1000
#define MIN_CCN_GREEN_OF_BLUE      0
#define INI_CCN_GREEN_OF_BLUE      0

#define MAX_CCN_BLUE_OF_BLUE   1000
#define MIN_CCN_BLUE_OF_BLUE      0
#define INI_CCN_BLUE_OF_BLUE   1000

#define MAX_CCN_RED_OF_WHITE   1000
#define MIN_CCN_RED_OF_WHITE      0
#define INI_CCN_RED_OF_WHITE   1000

#define MAX_CCN_GREEN_OF_WHITE   1000
#define MIN_CCN_GREEN_OF_WHITE      0
#define INI_CCN_GREEN_OF_WHITE   1000

#define MAX_CCN_BLUE_OF_WHITE   1000
#define MIN_CCN_BLUE_OF_WHITE      0
#define INI_CCN_BLUE_OF_WHITE   1000

#define HSG_MAX_VALUE          254
#define HSG_DEFAULT_VALUE      127
#define HSG_MIN_VALUE          0

#define EDGE_MASK_MAX_VALUE        10       //ZU860_Doulas_0004
#define EDGE_MASK_DEFAULT_VALUE    0        //ZU860_Doulas_0004
#define EDGE_MASK_MIN_VALUE        0        //ZU860_Doulas_0004

//OSD
#define OSD_TIMEOUT_DEFAULT             3   //G100_Wilsonj_0036
#define OSD_TRANSLUCENCY_DEFAULT        50
#define OSD_MENUOFFSET_X_DEFAULT        0
#define OSD_MENUOFFSET_Y_DEFAULT        0

//Network
#define LAN_IP_MAX_VALUE                255    //A70LV_Doulas_0025
#define LAN_IP_DEFAULT_VALUE            0
#define LAN_IP_MIN_VALUE                0

#define LAN_MASK_MAX_VALUE              255
#define LAN_MASK_DEFAULT_VALUE          0
#define LAN_MASK_MIN_VALUE              0

#define LAN_GATEWAY_MAX_VALUE           255
#define LAN_GATEWAY_DEFAULT_VALUE       0
#define LAN_GATEWAY_MIN_VALUE           0

#define WLAN_START_IP_MAX_VALUE         255
#define WLAN_START_IP_DEFAULT_VALUE     0
#define WLAN_START_IP_MIN_VALUE         0

#define WLAN_END_IP_MAX_VALUE           255
#define WLAN_END_IP_DEFAULT_VALUE       0
#define WLAN_END_IP_MIN_VALUE           0

#define WLAN_MASK_MAX_VALUE             255
#define WLAN_MASK_DEFAULT_VALUE         0
#define WLAN_MASK_MIN_VALUE             0

#define WLAN_GATEWAY_MAX_VALUE          255
#define WLAN_GATEWAY_DEFAULT_VALUE      0
#define WLAN_GATEWAY_MIN_VALUE          0

#define CRESTRON_IP_MAX_VALUE           255    //G100_Wilsonj_0055
#define CRESTRON_IP_DEFAULT_VALUE       0
#define CRESTRON_IP_MIN_VALUE           0

#define CRESTRON_IP_ID_MAX_VALUE        255                 //G100_Tim_0063, add, start //A35G2_BRC_Casper_0073
#define CRESTRON_IP_ID_DEFAULT_VALUE    5
#define CRESTRON_IP_ID_MIN_VALUE        2
#define CRESTRON_IP_ID_STEP_VALUE       10                  //G100_Tim_0063, add, end

#ifdef CUSTOM_OPTOMA
#define CRESTRON_PORT_MAX_VALUE         65535
#define CRESTRON_PORT_DEFAULT_VALUE     41794// BruceLin#20210113
#define CRESTRON_PORT_MIN_VALUE         0
#define CRESTRON_PORT_STEP_VALUE        100
#endif

#define PJ_LINK_IP_MAX_VALUE            255
#define PJ_LINK_IP_DEFAULT_VALUE        0
#define PJ_LINK_IP_MIN_VALUE            0

#define PROJECTION_MODE_MAX_VALUE       3   // HICC20_Keven_0002
#define PROJECTION_MODE_DEFAULT_VALUE   0
#define PROJECTION_MODE_MIN_VALUE       0
#define DDP_PWM_MAX_VALUE               1000
#define DDP_PWM_DEFAULT_VALUE           0
#define DDP_PWM_MIN_VALUE               0

#define DDP_SYSTEM_MODE_MAX_VALUE       15//34  //G100_Julie_0014
#define DDP_SYSTEM_MODE_DEFAULT_VALUE   0
#define DDP_SYSTEM_MODE_MIN_VALUE       0

#define DDP_SPLASH_MAX_VALUE            3	//HICC2_Casper_0021
#define DDP_SPLASH_DEFAULT_VALUE        0
#define DDP_SPLASH_MIN_VALUE            0

#define SYNC_THRESHOLD_MAX_VALUE        100
#define SYNC_THRESHOLD_DEFAULT_VALUE    10//50      //A70LV_Doulas_0110 modify //A70LV_Doulas_0060 modify
#define SYNC_THRESHOLD_MIN_VALUE        0
#define SYNC_THRESHOLD_STEP_VALUE       5       //A70LV_Doulas_0110

#define WARPING_APPLY_MAX_VALUE             (eCM_GEO_MEMORY_APPLY_NUMBER-1)//5		//G100_Doulas_0027 Modify
#define WARPING_APPLY_DEFAULT_VALUE         eCM_GEO_MEMORY_APPLY_OFF
#define WARPING_APPLY_MIN_VALUE             eCM_GEO_MEMORY_APPLY_OFF

#define WARPING_SAVE_MAX_VALUE      (eCM_GEO_MEMORY_SAVE_NUMBER-1)//5		//G100_Doulas_0027 Modify
#define WARPING_SAVE_DEFAULT_VALUE  eCM_GEO_MEMORY_SAVE_1
#define WARPING_SAVE_MIN_VALUE      eCM_GEO_MEMORY_SAVE_1

#define BLEND_APPLY_MAX_VALUE             (eCM_GEO_MEMORY_APPLY_NUMBER-1)
#define BLEND_APPLY_DEFAULT_VALUE         eCM_GEO_MEMORY_APPLY_OFF
#define BLEND_APPLY_MIN_VALUE             eCM_GEO_MEMORY_APPLY_OFF

#define BLEND_SAVE_MAX_VALUE      (eCM_GEO_MEMORY_SAVE_NUMBER-1)
#define BLEND_SAVE_DEFAULT_VALUE  eCM_GEO_MEMORY_SAVE_1
#define BLEND_SAVE_MIN_VALUE      eCM_GEO_MEMORY_SAVE_1

#ifdef CUSTOM_OPTOMA
#define SOURCE_DETECT_VGA_SHIFT         1   //A70LV_Doulas_0129
#define SOURCE_DETECT_HDMI1_SHIFT       3   //A70LV_Doulas_0129
#define SOURCE_DETECT_HDMI2_SHIFT       4   //A70LV_Doulas_0129
#define SOURCE_DETECT_DVI_SHIFT         5   //A70LV_Doulas_0129
#define SOURCE_DETECT_3G_SDI_SHIFT      7   //A70LV_Doulas_0129
#define SOURCE_DETECT_HDBASET_SHIFT     8   //A70LV_Doulas_0129
#define SOURCE_DETECT_DISPLAYPORT       6
#define SOURCE_DETECT_SLOT1             13
#define SOURCE_DETECT_SLOT2             14
#endif

#define ADC_RGB1_GAIN0_DEFAULT          802        //A70LV_Doulas_0171
#define ADC_RGB1_GAIN1_DEFAULT          804
#define ADC_RGB1_GAIN2_DEFAULT          804
#define ADC_YUV1_GAIN0_DEFAULT          540
#define ADC_YUV1_GAIN1_DEFAULT          680
#define ADC_YUV1_GAIN2_DEFAULT          535

#define ADC_RGB2_GAIN0_DEFAULT          801        //A70LV_Doulas_0171
#define ADC_RGB2_GAIN1_DEFAULT          801
#define ADC_RGB2_GAIN2_DEFAULT          808
#define ADC_YUV2_GAIN0_DEFAULT          540
#define ADC_YUV2_GAIN1_DEFAULT          680
#define ADC_YUV2_GAIN2_DEFAULT          535

#define DDP_3D_SYNC_CHECK_TIMER             1// 4       //G100_Doulas_0071 Modify//A70LV_Doulas_0191

#define MODE_ADJ_H_START_MAX_VALUE      2000    //A70LV_Doulas_0209
#define MODE_ADJ_H_START_DEFAULT_VALUE  0
#define MODE_ADJ_H_START_MIN_VALUE      0

#define MODE_ADJ_V_START_MAX_VALUE      2000    //A70LV_Doulas_0209
#define MODE_ADJ_V_START_DEFAULT_VALUE  0
#define MODE_ADJ_V_START_MIN_VALUE      0

#define MODE_ADJ_RECORD_INDEX_MAX_VALUE      19    //A70LV_Doulas_0209
#define MODE_ADJ_RECORD_INDEX_DEFAULT_VALUE  0
#define MODE_ADJ_RECORD_INDEX_MIN_VALUE      0

#define PHASE_LOCK_COUNT_DOWN               3//12//8//3	//G100_Doulas_0076//G100_Clare_0050//G100_Clare_0023//A70LV_Doulas_0212
#define PHASE_LOCK_MAX_TIMER                30      //A70LV_Doulas_0212

#define LIGHTS_OUT_TIMER_X05_MAX_VALUE       20
#define LIGHTS_OUT_TIMER_X05_DEFAULT_VALUE   0
#define LIGHTS_OUT_TIMER_X05_MIN_VALUE       0

#define LIGHTS_OUT_SIGNAL_LEVEL_MAX_VALUE      255//5	//A70Gen2_Doulas_0044 Modify
#define LIGHTS_OUT_SIGNAL_LEVEL_DEFAULT_VALUE  78//0	//A70Gen2_Doulas_0044 Modify
#define LIGHTS_OUT_SIGNAL_LEVEL_MIN_VALUE      1            //H30K_Tim_0016, mod, 0

#define LIGHTS_ON_THRESHOLD_MAX_VALUE      	 127		//A70Gen2_Doulas_0044
#define LIGHTS_ON_THRESHOLD_DEFAULT_VALUE  	  28//20	//A70Gen2_Doulas_0049 //A70Gen2_Doulas_0044
#define LIGHTS_ON_THRESHOLD_MIN_VALUE          0		//A70Gen2_Doulas_0044

#define CUSTOM_LOGO_CHRISTIE 0
#define CUSTOM_LOGO_SCREENX  1

#define OSD_H_START_WUGA_60         56  //A70LV_Doulas_0226
#define OSD_H_START_1080P_60        192  //A70LV_Doulas_0226
#define OSD_H_START_1080P_120       24  //A70LV_Doulas_0226
#define OSD_V_START_WUGA_60         18//20  //A65_OPTOMA_Doulas_0060 20-2//A70LV_Doulas_0226
#define OSD_V_START_1080P_60        39//41  //A65_OPTOMA_Doulas_0060 41-2//A70LV_Doulas_0226
#define OSD_V_START_1080P_120       37  //A70LV_Doulas_0226
#define OSD_H_START_WUXGA_120       24  //G100_Doulas_0064
#define OSD_V_START_WUXGA_120       16//10//20  //G100_Doulas_0064


#define BACKUP_RESTORE_MAX_VALUE             4///2   //G100_Doulas_0002 Modify//A70LV_Doulas_0279
#define BACKUP_RESTORE_DEFAULT_VALUE         0
#define BACKUP_RESTORE_MIN_VALUE             0

#define DB_SPEED_MAX_VALUE              255       //HICC2_Steven_0040 //A70LV_Doulas_0327
#define DB_SPEED_DEFAULT_VALUE          1
#define DB_SPEED_MIN_VALUE              1

#define DB_STRENGTH_MAX_VALUE           3       //A70LV_Doulas_0327
#define DB_STRENGTH_DEFAULT_VALUE       2
#define DB_STRENGTH_MIN_VALUE           0

#define DB_LIGHT_LEVEL_MAX_VALUE        100     //A70LV_Doulas_0327
#define DB_LIGHT_LEVEL_DEFAULT_VALUE    100
#define DB_LIGHT_LEVEL_MIN_VALUE        50

#define CCT_MAX_VALUE       110
#define CCT_DEFAULT_VALUE   100
#define CCT_MIN_VALUE       80

//#define CCT_CALIBRATION_TAG     (0x12345678)
#define CCT_NEW_CALIBRATION_TAG (0x11111111) //G50_Casper_0006


#define PROA_RX_1   0
#define PROA_RX_2   1
#define PROA_RX_3   2
#define PROA_RX_4   3

#define RXPORT_CHANNEL_0  PROA_RX_1
#define RXPORT_CHANNEL_1  PROA_RX_3

#define PROJECTOR_CODE_MAX_VALUE             99     //ZU860_Doulas_0004
#define PROJECTOR_CODE_DEFAULT_VALUE         0
#define PROJECTOR_CODE_MIN_VALUE             0

#define SHARPNESS_MAX_VALUE                 15     //ZU860_Doulas_0011
#define SHARPNESS_DEFAULT_VALUE             8
#define SHARPNESS_MIN_VALUE                 1


#ifdef CUSTOM_OPTOMA
#define AUTO_SHUTDOWN_TIMER_STEP_VALUE      1  //A35G2_Coda_0062
#define AUTO_SHUTDOWN_TIMER_MAX_VALUE       180
#define AUTO_SHUTDOWN_TIMER_DEFAULT_VALUE   20       //A35G2_Coda_0100
#define AUTO_SHUTDOWN_TIMER_MIN_VALUE       0

#define SLEEP_TIMER_MAX_VALUE               960
#define SLEEP_TIMER_DEFAULT_VALUE           0
#define SLEEP_TIMER_MIN_VALUE               0
#define SLEEP_TIMER_STEP_VALUE              30      //ZU860_Doulas_0029
#else
#define AUTO_SHUTDOWN_TIMER_STEP_VALUE      5
#define AUTO_SHUTDOWN_TIMER_MAX_VALUE       180
#define AUTO_SHUTDOWN_TIMER_DEFAULT_VALUE   0
#define AUTO_SHUTDOWN_TIMER_MIN_VALUE       0

#define SLEEP_TIMER_MAX_VALUE               990/30  // 990 / 30 = 33
#define SLEEP_TIMER_DEFAULT_VALUE           0
#define SLEEP_TIMER_MIN_VALUE               0
#define SLEEP_TIMER_STEP_VALUE              30      //ZU860_Doulas_0029
#endif


#define SOURCE_LOST_TIME    6   //A70LV_Doulas_0329
#ifdef CUSTOM_OPTOMA
#define SECURITY_MONTH_MAX_VALUE        35// BruceLin#20201210, 12      //ZU860_Doulas_0039
#else
#define SECURITY_MONTH_MAX_VALUE        12      //ZU860_Doulas_0039
#endif
#define SECURITY_MONTH_DEFAULT_VALUE    0       //ZU860_Doulas_0039
#define SECURITY_MONTH_MIN_VALUE        0       //ZU860_Doulas_0039

#define SECURITY_DAY_MAX_VALUE          29      //ZU860_Doulas_0061//ZU860_Doulas_0039
#define SECURITY_DAY_DEFAULT_VALUE      0       //ZU860_Doulas_0039
#define SECURITY_DAY_MIN_VALUE          0       //ZU860_Doulas_0039

#define SECURITY_HOUR_MAX_VALUE         23      //ZU860_Doulas_0061//ZU860_Doulas_0039
#define SECURITY_HOUR_DEFAULT_VALUE     0       //ZU860_Doulas_0039
#define SECURITY_HOUR_MIN_VALUE         0       //ZU860_Doulas_0039

#define LD_HOUR_MINUTE_MAX_VALUE         (6000000 - 1) //A35G2_CDS_CODA_0005
#define LD_HOUR_MINUTE_DEFAULT_VALUE     0
#define LD_HOUR_MINUTE_MIN_VALUE         0

#if defined(CUSTOM_CHRISTIE)  //HICC2_Doulas_0071
#define FADE_TIMER_MAX_VALUE       10
#define FADE_TIMER_DEFAULT_VALUE   0
#define FADE_TIMER_MIN_VALUE       0
#define FADE_TIMER_STEP_VALUE      5
#define FADE_IN_TIMER_QUICK        1
#define FADE_OUT_TIMER_QUICK       1
#elif defined(CUSTOM_OPTOMA)
#define FADE_TIMER_MAX_VALUE       50
#define FADE_TIMER_DEFAULT_VALUE   10
#define FADE_TIMER_MIN_VALUE       0
#define FADE_TIMER_STEP_VALUE      5
#define FADE_IN_TIMER_QUICK        1
#define FADE_OUT_TIMER_QUICK       3
#else
#define FADE_TIMER_MAX_VALUE       50
#define FADE_TIMER_DEFAULT_VALUE   5
#define FADE_TIMER_MIN_VALUE       5
#define FADE_TIMER_STEP_VALUE      5
#define FADE_IN_TIMER_QUICK        1
#define FADE_OUT_TIMER_QUICK       3
#endif

#define OSD_LOCK_TIMEOUT                300     // seconds	//G100_Clare_0022, add
#define OSD_LOCK_DISABLE                0      	//G100_Clare_0022, add


#define DATE_TIME_ZONE_MAX_VAULE          37      //G100_Coda_0037  //G100_Coda_0079
#define DATE_TIME_ZONE_DEFAULT_VALUE      23      //G100_Coda_0037
#define DATE_TIME_ZONE_MIN_VALUE          0       //G100_Coda_0037

#define MAX_EMERGENCY_STRING_LENS         256		//G100_Doulas_0054

#define EDID_TYPE_MASK                  0x03        //HICC2_Doulas_0068
#define HDMI1_EDID_TYPE2P0_SHIFT		0x00		//G100_Doulas_0082
#define HDMI2_EDID_TYPE2P0_SHIFT		0x02		//HICC2_Doulas_0068//G100_Doulas_0082
#define HDBASET_EDID_TYPE2P0_SHIFT		0x04		//HICC2_Doulas_0068//G100_Doulas_0082
#define PROJECTION_NATIVE_TIMING_SHIFT  0x06     	// HICC2_Bruce_0022

#define COLOR_MASK_MAX_VAULE            8
#define COLOR_MASK_DEFAULT_VALUE        0
#define COLOR_MASK_MIN_VALUE            0

#define WARP_CONTROL_BASIC_WARP                     0     //R70G2_David_0035
#define WARP_CONTROL_ADVANCED_WARP            		1     //R70G2_David_0035
#define WARP_CONTROL_AP                  			2     //R70G2_David_0035

#define OPD_PERIOD_MIN_VALUE 0
#define OPD_PERIOD_MAX_VALUE 600

#define BIST_STATUS_INIT     0  //HICC2_Steven_0012
#define	BIST_STATUS_BUSY     1
#define	BIST_STATUS_PASS     2
#ifdef CUSTOM_OPTOMA
#define QUICK_KEY_MIN					0	//A65_OPTOMA_Doulas_0025
#define QUICK_KEY_MAX					9	//A65_OPTOMA_Doulas_0025
#define QUICK_KEY_DEFAULT				0	//A65_OPTOMA_Doulas_0025
#endif

#define ACU_TARGET_X_DEFAULT          5       // center
#define ACU_TARGET_Y_DEFAULT          4       // center
#define ACU_TARGET_X_MIN              1       // top left
#define ACU_TARGET_Y_MIN              1       // top left
#define ACU_TARGET_X_MAX              9       // bottom right
#define ACU_TARGET_Y_MAX              7       // bottom right
#define ACU_TARGET_DEFAULT            ( ( ACU_TARGET_X_DEFAULT * 10 ) + ACU_TARGET_Y_DEFAULT )
#define ACU_TARGET_MIN                ( ( ACU_TARGET_X_MIN * 10 ) + ACU_TARGET_Y_MIN )
#define ACU_TARGET_MAX                ( ( ACU_TARGET_X_MAX * 10 ) + ACU_TARGET_Y_MAX )
#define ACU_RESET_WRITE_CMD           1       // reset
#define TARGET_SELECT_LEN             2       // X,Y
#ifdef CUSTOM_OPTOMA
#define SECURITY_CODE_MAX_LENGTH      4
#else
#define SECURITY_CODE_MAX_LENGTH      5    //A35G2_Simon_0098
#endif
#define FPGA_ID_MAX_LENGTH            9    //A35G2_Simon_0098



//Projector Platform //for A35G2
#define MODULE_8K_PLATFORM          (0x81)  //Read from MCU
#define MODULE_9K_PLATFORM          (0x82)
#define MODULE_11K_PLATFORM         (0x83)

//Projector Platform //for A70G2
#define MODULE_15K_PLATFORM         (0x81) //LHR  //Read from MCU
#define MODULE_19K_PLATFORM         (0x82) //LS
#define MODULE_22K_PLATFORM         (0x83) //LV

//Projector Platform //for A70LK
#define MODULE_4K13_PLATFORM        (0x84) //Read from MCU
#define MODULE_4K16_PLATFORM        (0x85) //reserve
#define MODULE_4K22_PLATFORM        (0x86)

//Projector Platform //for H60U (H60 2K)
#define MODULE_H60_18K_PLATFORM     (0x88) //Read from MCU //HICC2_Doulas_0013//HICC2_Doulas_0011
#define MODULE_H60_24K_PLATFORM     (0x87) //HICC2_Doulas_0013//HICC2_Doulas_0011
//#define MODULE_ID2_PLATFORM         (0x83)

//Projector Platform //for H60K (H60 4K)
#define MODULE_H60K16_PLATFORM        (0x8A)
#define MODULE_H60K21_PLATFORM        (0x89)
//#define MODULE_H60K16_PLATFORM        (0x85) //reserve

//Projector Platform //for H30K (H30 4K)
#define MODULE_H30_4K_0_PLATFORM        (0x8A)
#define MODULE_H30_4K_1_PLATFORM        (0x89)
#define MODULE_H30_4K_2_PLATFORM        (0x85) //reserve

#define MODULE_NA_PLATFORM          (0xFF)

#ifdef PLATFORM_A70G2
    #define MODULE_TYPE_ID0_PLATFORM      (MODULE_15K_PLATFORM)
    #define MODULE_TYPE_ID1_PLATFORM      (MODULE_19K_PLATFORM)
    #define MODULE_TYPE_ID2_PLATFORM      (MODULE_22K_PLATFORM)
#elif defined(PLATFORM_A35G2)
    #define MODULE_TYPE_ID0_PLATFORM      (MODULE_8K_PLATFORM)
    #define MODULE_TYPE_ID1_PLATFORM      (MODULE_9K_PLATFORM)
    #define MODULE_TYPE_ID2_PLATFORM      (MODULE_11K_PLATFORM)
#elif defined(PLATFORM_A70LK)
    #define MODULE_TYPE_ID0_PLATFORM      (MODULE_4K13_PLATFORM)
    #define MODULE_TYPE_ID1_PLATFORM      (MODULE_4K16_PLATFORM)
    #define MODULE_TYPE_ID2_PLATFORM      (MODULE_4K22_PLATFORM)
#elif defined(PLATFORM_H60_2K)
    #define MODULE_TYPE_ID0_PLATFORM      (MODULE_H60_18K_PLATFORM)	//HICC2_Doulas_0013//HICC2_Doulas_0011
    #define MODULE_TYPE_ID1_PLATFORM      (MODULE_H60_24K_PLATFORM)	//HICC2_Doulas_0013//HICC2_Doulas_0011
    #define MODULE_TYPE_ID2_PLATFORM      (MODULE_NA_PLATFORM)
#elif defined(PLATFORM_H60_4K)
    #define MODULE_TYPE_ID0_PLATFORM      (MODULE_H60K16_PLATFORM)
    #define MODULE_TYPE_ID1_PLATFORM      (MODULE_H60K21_PLATFORM)            //HICC2_Doulas_0089
    #define MODULE_TYPE_ID2_PLATFORM      (MODULE_NA_PLATFORM) //reserve  //HICC2_Doulas_0089
#elif defined(PLATFORM_H30_4K)
    #define MODULE_TYPE_ID0_PLATFORM      (MODULE_H30_4K_0_PLATFORM)
    #define MODULE_TYPE_ID1_PLATFORM      (MODULE_H30_4K_1_PLATFORM)
    #define MODULE_TYPE_ID2_PLATFORM      (MODULE_H30_4K_2_PLATFORM) //reserve
#else
    message "MODULE_TYPE_ID_PLATFORM is undefined !\n"
    #define MODULE_TYPE_ID0_PLATFORM      (MODULE_NA_PLATFORM)
    #define MODULE_TYPE_ID1_PLATFORM      (MODULE_NA_PLATFORM)
    #define MODULE_TYPE_ID2_PLATFORM      (MODULE_NA_PLATFORM)
#endif


#if 0
#if (CURRENT_MAIN_BOARD == CORE_BOARD)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdarg.h>
#include "Chip.h"
#include "GenericTypeDefs.h"
#include "board.h"
#include "utilMathAPI.h"

#endif
#endif /* 0 */




//====================== Enum Start ======================//

#if 0
typedef enum
{
	eSCREEN_OFF,
	eSCREEN_PIP,
	eSCREEN_PBP,
	eSCREEN_LAST,
} eSCREEN_MODE;	//ZU860_Clare_0027,

typedef enum
{
    eLAYOUT_PBP_MAIN_LEFT,
    eLAYOUT_PBP_MAIN_TOP,
    eLAYOUT_PBP_MAIN_RIGHT,
    eLAYOUT_PBP_MAIN_BOTTOM,
    eLAYOUT_PIP_BOTTOM_RIGHT,
    eLAYOUT_PIP_BOTTOM_LEFT,
    eLAYOUT_PIP_TOP_LEFT,
    eLAYOUT_PIP_TOP_RIGHT,
    eLAYOUT_LAST,
} eMAIN_LAYOUT_MODE;


typedef enum
{
    ePIP_SIZE_SMALL,
    ePIP_SIZE_MEDIM,
    ePIP_SIZE_LARGE,
    ePIP_SIZE_LAST,
} ePIP_SIZE_MODE;

#endif

#if 0
typedef enum
{
#if defined(CUSTOM_BARCO) || defined(CUSTOM_OPTOMA)           //A65_Owen_0002   //ZU860_Doulas_0009 modify
    eSCALING_MODE_AUTO,         // begin  //G100_Wilsonj_0017
    eSCALING_MODE_4_3,
    eSCALING_MODE_16_9,
    eSCALING_MODE_16_10,
    eSCALING_MODE_LETTER_BOX,
    eSCALING_MODE_NATIVE,     //end
    eSCALING_MODE_3D,
    eSCALING_MODE_FULL_SIZE,
    eSCALING_MODE_FULL_WIDTH,
    eSCALING_MODE_FULL_HEIGHT,
    eSCALING_MODE_CUSTOM,

    eSCALING_MODE_FIT,
    eSCALING_MODE_ORG,
    eSCALING_MODE_ASPECT,
#else
    eSCALING_MODE_AUTO,
    eSCALING_MODE_NATIVE,
    eSCALING_MODE_4_3,
    eSCALING_MODE_LETTER_BOX,
    eSCALING_MODE_FULL_SIZE,
    eSCALING_MODE_FULL_WIDTH,
    eSCALING_MODE_FULL_HEIGHT,
    eSCALING_MODE_CUSTOM,
    eSCALING_MODE_3D,
    eSCALING_MODE_16_9,
    eSCALING_MODE_16_10,
    eSCALING_MODE_FIT,
    eSCALING_MODE_ORG,
    eSCALING_MODE_ASPECT,
#endif
} eSCALING_MODE;
#endif
typedef enum
{
    eINPUT_SYNC_ON_GREEN,
    eINPUT_PC_SYNC,             //A70LV_Doulas_0112
    eINPUT_SYNC_LAST,
} eINPUT_SYNC_TYPE;


typedef enum
{
#if 0   //ZU860_Doulas_0011 modify  //G100_Wilsonj_0005
	eINPUT_SOURCE_HDMI1,
	eINPUT_SOURCE_HDMI2,
	eINPUT_SOURCE_DVI,
	eINPUT_SOURCE_VGA,
	eINPUT_SOURCE_HDBASET,
	eINPUT_SOURCE_3G_SDI,
#else
	//eINPUT_SOURCE_VGA,
	eINPUT_SOURCE_HDMI1,
	eINPUT_SOURCE_HDMI2,
	eINPUT_SOURCE_DVI,
	//eINPUT_SOURCE_DISPLAYPORT,  //G100_Wilsonj_0005
	eINPUT_SOURCE_3G_SDI,
	eINPUT_SOURCE_HDBASET,
#endif
	eINPUT_SOURCE_MAX_NUMBER, //A70LV_Doulas_0009 //A70LV_Larry_0044
	eINPUT_SOURCE_FIRST  = 0,       //ZU860_Doulas_0011
}eINPUT_SOURCE;


typedef enum //G100_Steven_0021
{
	eBK_INPUT_SOURCE_HDMI1 =0,
	eBK_INPUT_SOURCE_HDMI2,
	eBK_INPUT_SOURCE_DVI,
	eBK_INPUT_SOURCE_DP, //A70Gen2_Julie_0006
	eBK_INPUT_SOURCE_HDBASET,
	eBK_INPUT_SOURCE_MAX_NUMBER,
}eBK_INPUT_SOURCE;

typedef enum
{
	eSOURCE_STATE_SHOW_SOURCE_RESOLUTION,
	eSOURCE_STATE_CHECKING_FOR_SIGNAL,
	eSOURCE_STATE_SEARCHING,
	eSOURCE_STATE_SETTING_UP_IMAGE,
	eSOURCE_STATE_SIGNAL_OUT_OF_RANGE,
}eSOURCE_STATE;


typedef enum
{
	eOVER_SCAN_OFF,
    eOVER_SCAN_ZOOM,
	eOVER_SCAN_CROP,
}eOVER_SCAN_TYPE;

typedef enum
{
    eCONTRAST_ENHANCE_OFF ,
    eCONTRAST_ENHANCE_DYNAMIC_BLACK ,
    eCONTRAST_ENHANCE_REAL_BLACK ,
    eCONTRAST_ENHANCE_DYNAMIC_BLACK_REAL_BLACK,

    eCONTRAST_ENHANCE_MODE  , //end
} eCONTRAST_ENHANCEMode; //A70LV_Doulas_0013 Add

#if 0
typedef enum //G100_Steven_0012
{
    eIFC_WAP_PRESENTATION,
    eIFC_WAP_VIDEO,
    eIFC_WAP_BRIGHT,
    eIFC_WAP_ENHANCED,
    eIFC_WAP_REC709,
    eIFC_WAP_REAL,
    eIFC_WAP_DICOMSIM,
    eIFC_WAP_2D_HIGH_SPEED,
    eIFC_WAP_3D,
    eIFC_WAP_BLENDING,
    eIFC_WAP_USER,
    eIFC_WAP_HDR,
    eIFC_WAP_SRGB,
    eIFC_WAP_SUPER_BRIGHT,
    eIFC_WAP_SUPER_RED,

	eIFC_WAP_NUMBER,
} eIFC_WAP_PresetMode;

typedef enum
{
#if defined(CUSTOM_BARCO) || defined(CUSTOM_OPTOMA)    //A65_Owen_0002  //ZU860_Doulas_0009 modify
	eIFC_PRESENTATION ,
	eIFC_BRIGHT      ,
	//eIFC_SUPERBRIGHT ,  //ZU860_John_0033 add super bright mode (need OSD)  //G100_Wilsonj_0017
    eIFC_CINEMA	     ,
	//eIFC_HDR         ,       //ZU860_Doulas_0063 Modify  //G100_Wilsonj_0017
	eIFC_sRGB        ,
	eIFC_DICOMSIM    ,
	eIFC_BLENDING    ,
	eIFC_USER        ,

	eIFC_3D          ,
	eIFC_2DHIGHSPEED ,  //G100_Wilsonj_0017
	eIFC_NUMBER,
#else
//    eIFC_PRESETATION ,      //A70LV_Doulas_0030 remove
    eIFC_VIDEO       ,
    eIFC_BRIGHT      ,
	eIFC_ENHANCED	 ,
    eIFC_REC709      ,
//    eIFC_REAL        ,    //A70LV_Doulas_0030 remove
    eIFC_DICOMSIM    ,
//    eIFC_2DHIGHSPEED ,    //A70LV_Doulas_0030 remove
//    eIFC_3D          ,    //A70LV_Doulas_0030 remove
    eIFC_BLENDING    ,    //A70LV_Doulas_0030 remove
    eIFC_USER        ,
    eIFC_HDR        ,

    eIFC_3D          ,//= 7,          //A70LV_Doulas_0248 modify//A70LV_Doulas_0192 modify //A70LV_Doulas_0160 with DDP project setting
    eIFC_2DHIGHSPEED ,//  = 8,     //A70LV_Doulas_0248
    eIFC_NUMBER,                //A70LV_Doulas_0286
#endif
} eIFC_PresetMode;   //A70LV_Doulas_0015

typedef enum //G100_Steven_0029 //A35G2_BRC_Casper_0012
{
	eIFC_ApplyUserMode_OFF = 0,
	eIFC_ApplyUserMode_PRESENTATION,
	eIFC_UserApplyUserMode_BRIGHT,
	//eIFC_ApplyUserMode_SUPERBRIGHT,
	eIFC_ApplyUserMode_CINEMA,
	eIFC_ApplyUserMode_HDR,
	eIFC_ApplyUserMode_sRGB,
	eIFC_ApplyUserMode_DICOMSIM,
	eIFC_ApplyUserMode_BLENDING,
	eIFC_ApplyUserMode_3D,
	eIFC_ApplyUserMode_2DHIGHSPEED,
	eIFC_ApplyUserMode_NUMBER,

} eIFC_UserApplyMode;		//G100_Doulas_0066 remove
#endif /* 0 */

typedef enum
{
    eUST_LensInstall_Confirm      = 0x80, //add UST confirm and cancel reminder
    eUST_LensInstall_TestPattern1 = 0x81,
    eUST_LensInstall_TestPattern2 = 0x82,
    eUST_LensInstall_TestPattern3 = 0x83,
    eUST_LensInstall_TestPattern4 = 0x84,
    eUST_LensInstall_Finished     = 0x85,
    eUST_LensInstall_Cancel       = 0x86, //UST confirm and cancel reminder
    eUST_LensInstall_INVALID      = 0x87,
} eUST_LensInstallType;         //A70LV_Doulas_0015 Add

typedef enum
{
#if defined(CUSTOM_CHRISTIE) || defined(CUSTOM_OPTOMA) //A65_Owen_0004
    eSOURCE_KEY_CHANGE_SOURCE,//ChangeSource,
    eSOURCE_KEY_LIST_ALL_SOURCE,//ListAllOfSources,
    eSOURCE_KEY_CHANGE_SOURCE_AUTO,//ChangeSourceWithAuto,
#else
    eSOURCE_KEY_CHANGE_SOURCE,//ChangeSource,
    eSOURCE_KEY_CHANGE_SOURCE_AUTO,//ChangeSourceWithAuto,
#endif
} eSOURCE_KEY_ITEM;         //A70LV_Doulas_0024    //A70LV_Doulas_0015

//OE CPC
typedef enum
{
    ePM_CONSTANT_POWER,//epm_NORMAL,
    ePM_ECO1, //epm_ECO,
    ePM_QUIET,
    ePM_CUSTOM,

    ePM_MAX_NUMBER,

    ePM_CONSTANT_INTENSITY,
    ePM_RENTAL_MODE,
    ePM_ECO2,
} ePOWER_MODE;      //G100_Doulas_0008 Modify//A70LV_Doulas_0024

typedef enum
{
    eas_ON,
    eas_OFF,

    eas_MAX_NUMBER
} eACTUATOR_SWITCH;
#if 0 //HICC2_Doulas_0049
typedef enum
{
    eFACTORY_TP_NONE,
    eFACTORY_TP_WHITE,
    eFACTORY_TP_BLACK,
    eFACTORY_TP_RED,
    eFACTORY_TP_GREEN,
    eFACTORY_TP_BLUE,
    eFACTORY_TP_CYAN,
    eFACTORY_TP_YELLOW,
    eFACTORY_TP_MAGENTA,
    eFACTORY_TP_BLUE_60,
    eFACTORY_TP_GRAY_10,
    eFACTORY_TP_WRGB_64,
    eFACTORY_TP_FLARE,
    eFACTORY_TP_FULL_SREEN_W, //A35G2_BRC_Casper_0088
    eFACTORY_TP_FULL_SREEN_B, //A35G2_BRC_Casper_0088
    eFACTORY_TP_LATERAL_COLOR, //A35G2_BRC_Casper_0088

    eFACTORY_TP_MAX_NUMBER
} eFACTORY_TEST_PATTERN;
#endif
#if 0
typedef enum
{
    /*00*/ elg_ENGLISH,
    /*01*/ elg_S_CHINESE,
    /*02*/ elg_FRENCH,
    /*03*/ elg_GERMAN,
    /*04*/ elg_ITALIAN,
    /*05*/ elg_JAPANESE,
    /*06*/ elg_KOREAN,
    /*07*/ elg_RUSSIAN,
    /*08*/ elg_SPANISH,
    /*09*/ elg_PORTUGUESE,
    /*10*/ elg_INDONESIAN,
    /*11*/ elg_DUTCH,

    elg_MAX_NUMBER
} eLANGUAGE;

typedef enum
{
    eml_TOP_LEFT,
    eml_TOP_RIGHT,
    eml_CENTER,
    eml_BOTTOM_LEFT,
    eml_BOTTOM_RIGHT,

    eml_MAX_NUMBER
} eMENU_LOCATION;
#endif

typedef enum
{
	efc_TOP_LEFT,
	efc_TOP_RIGHT,
	efc_BOTTOM_LEFT,
	efc_BOTTOM_RIGHT,

	ecf_MAX_NUMBER
} eFOUR_CORNER;	//ZU860_Clare_0001

typedef enum
{
    ets_OFF,
    ets_ON,
#if defined (CUSTOM_OPTOMA)//A65_Optoma_John_0006 fix shutter CLI
	ets_OptomaOff,
#endif

    ets_MAX_NUMBER
} eTOGGLE_SETTING;

#if 0
typedef enum
{
    eCEILING_MOUNT_AUTO,
    eCEILING_MOUNT_ON,
    eCEILING_MOUNT_OFF,

    eCEILING_MOUNT_MAX_NUMBER
} eCEILING_MOUNT_SETTING;
#endif

typedef enum
{
	ePROJECTION_CEILING_TOP,
	ePROJECTION_REAR_TOP,
	ePROJECTION_FRONT,
	ePROJECTION_REAR,

	ePROTECTION_MAX_NUMBER
} ePROJECTION_SETTING;	//ZU860_Clare_0001

typedef enum
{
	ePROJECTION_MODE_FRONT,
	ePROJECTION_MODE_FRONT_CEILING,
	ePROJECTION_MODE_REAR,
	ePROJECTION_MODE_REAR_CEILING,

	ePROTECTION_MODE_MAX_NUMBER
} ePROJECTION_MODE;	// HICC20_Keven_0002


#if 0
typedef enum
{
    e3D_SYNC_OUT_TO_EMITTER,
    e3D_SYNC_OUT_TO_NEXT_PROJECTOR,

    e3D_SYNC_OUT_MAX_NUMBER
} e3D_SYNC_OUT_SETTING;

typedef enum
{
    e3D_SYNC_TYPE_AUTO,
    e3D_SYNC_TYPE_INTERNAL,
    e3D_SYNC_TYPE_EXTERNAL,

    e3D_SYNC_TYPE_NUMBER
} e3D_SYNC_TYPE_SETTING;
#endif

typedef enum
{
    eAUTO_IMAGE_NORMAL,
    eAUTO_IMAGE_WIDE,

    eAUTO_IMAGE_MAX_NUMBER
} eAUTO_IMAGE_SETTING;

typedef enum
{
    eSTANDBY_MODE_0d5W,
    eSTANDBY_MODE_2W,
    eSTANDBY_MODE_COMMUNICATION,

    eSTANDBY_MODE_MAX_NUMBER
} eSTANDBY_MODE_SETTING;

typedef enum
{
    eSERIAL_PORT_RS232,
    eSERIAL_PORT_HDBASET,

    eSERIAL_PORT_INVALID,
} eSERIAL_PORT_PATH;
#ifdef CUSTOM_OPTOMA
	#if 0// BruceLin#20201222
	typedef enum
	{
	    eKEYPAD_BACKLIGHT_5_SECS,
	    eKEYPAD_BACKLIGHT_10_SECS,
	    eKEYPAD_BACKLIGHT_20_SECS,
	    eKEYPAD_BACKLIGHT_30_SECS,
	    eKEYPAD_BACKLIGHT_ALWAYS_ON,
	    eKEYPAD_BACKLIGHT_ALWAYS_OFF,

	    eKEYPAD_BACKLIGHT_MAX_NUMBER
	} eKEYPAD_BACKLIGHT_SETTING;
	#endif
#else
typedef enum
{
    eKEYPAD_BACKLIGHT_5_SECS,
    eKEYPAD_BACKLIGHT_10_SECS,
    eKEYPAD_BACKLIGHT_20_SECS,
    eKEYPAD_BACKLIGHT_30_SECS,
    eKEYPAD_BACKLIGHT_ALWAYS_ON,
    eKEYPAD_BACKLIGHT_ALWAYS_OFF,

    eKEYPAD_BACKLIGHT_MAX_NUMBER
} eKEYPAD_BACKLIGHT_SETTING;
#endif

typedef enum
{
    eSTATUS_LED_ALWAYS_ON,
    eSTATUS_LED_ALWAYS_OFF,
    eSTATUS_LED_WARNING_ERRORS_ONLY,

    eSTATUS_LED_MAX_NUMBER
} eSTATUS_LED_SETTING;

typedef enum
{
    eAUTO_OFF_TIME_NEVER,
    eAUTO_OFF_TIME_5MIN,
    eAUTO_OFF_TIME_10MIN,
    eAUTO_OFF_TIME_15MIN,
    eAUTO_OFF_TIME_20MIN,
    eAUTO_OFF_TIME_25MIN,
    eAUTO_OFF_TIME_30MIN,

    eAUTO_OFF_TIME_MAX_NUMBER
} eAUTO_OFF_TIME_SETTING;

#if defined(CUSTOM_BARCO) || defined(CUSTOM_OPTOMA) //A65_Owen_0006
typedef enum //G100_Steven_0026 start
{
    eSLEEP_TIMER_OFF = 0,
	eSLEEP_TIMER_0p5HRS,
	eSLEEP_TIMER_1HRS,
	eSLEEP_TIMER_1p5HRS,
    eSLEEP_TIMER_2HRS,
	eSLEEP_TIMER_2p5HRS,
	eSLEEP_TIMER_3HRS,
	eSLEEP_TIMER_3p5HRS,
    eSLEEP_TIMER_4HRS,
	eSLEEP_TIMER_4p5HRS,
	eSLEEP_TIMER_5HRS,
	eSLEEP_TIMER_5p5HRS,
    eSLEEP_TIMER_6HRS,
	eSLEEP_TIMER_6p5HRS,
	eSLEEP_TIMER_7HRS,
	eSLEEP_TIMER_7p5HRS,
	eSLEEP_TIMER_8HRS,
	eSLEEP_TIMER_8p5HRS,
	eSLEEP_TIMER_9HRS,
	eSLEEP_TIMER_9p5HRS,
	eSLEEP_TIMER_10HRS,
	eSLEEP_TIMER_10p5HRS,
	eSLEEP_TIMER_11HRS,
	eSLEEP_TIMER_11p5HRS,
	eSLEEP_TIMER_12HRS,
	eSLEEP_TIMER_12p5HRS,
	eSLEEP_TIMER_13HRS,
	eSLEEP_TIMER_13p5HRS,
	eSLEEP_TIMER_14HRS,
	eSLEEP_TIMER_14p5HRS,
	eSLEEP_TIMER_15HRS,
	eSLEEP_TIMER_15p5HRS,
	eSLEEP_TIMER_16HRS,

    eSLEEP_TIMER_MAX_NUMBER
} eSLEEP_TIMER_SETTING; //G100_Steven_0026 end
#else
typedef enum //G100_Steven_0026 start
{
    eSLEEP_TIMER_OFF = 0,
    eSLEEP_TIMER_2HRS,
    eSLEEP_TIMER_4HRS,
    eSLEEP_TIMER_6HRS,

    eSLEEP_TIMER_MAX_NUMBER
} eSLEEP_TIMER_SETTING; //G100_Steven_0026 end
#endif

typedef enum
{
    e_SHARPNESS_MAXIMUN, // Maximum
    e_SHARPNESS_HIGH,    // High
    e_SHARPNESS_NORMAL,  //Normal
    e_SHARPNESS_LOW,     //Low
    e_SHARPNESS_MINIMUM, //Minimum

    e_SHARPNESS_MAX_NUMBER
} e_SHARPNESS_SETTING;      //Detail

#if 0
typedef enum
{
    eCOLOR_SPACE_AUTO,
    eCOLOR_SPACE_RGB_RGB,
    eCOLOR_SPACE_RGB_RGB_VIDEO,
//    eCOLOR_SPACE_RGB_REC709,      //A70LV_Doulas_0022 remove
    eCOLOR_SPACE_YUV_REC709,
    eCOLOR_SPACE_YUV_REC601,

    eCOLOR_SPACE_MAX_NUMBER
} eCOLOR_SPACE_STATE;
#endif

typedef enum
{
    eCOLOR_FORMAT_RGB,
    eCOLOR_FORMAT_422,
    eCOLOR_FORMAT_422_UVDLY,
    eCOLOR_FORMAT_420, //A70LK_Nina_0047
    eCOLOR_FORMAT_444,
    eCOLOR_FORMAT_RGB_LIMIT
} eCOLOR_FORMAT;        //A70LV_Doulas_0076 for C821

typedef enum
{
    eCS_RGB_RGB_FULL,
    eCS_RGB_RGB_VIDEO,
    eCS_RGB_OFF,
    eCS_RGB_MAX_NUMBER
} eCOLOR_SPACE_RGB_STATE;       //A70LV_Doulas_0022

typedef enum
{
    eCS_YUV_REC709,
    eCS_YUV_REC601,
    eCS_YUV_OFF,
    eCS_YUV_MAX_NUMBER
} eCOLOR_SPACE_YUV_STATE;

typedef enum //HICC2_Steven_0031
{
    eDMD_AIRTIGHT_OFF = 0,
	eDMD_AIRTIGHT_ON,
	eDMD_AIRTIGHT_FORCE_ON,

	eDMD_AIRTIGHT_MAX_NUMBER,
} eDMD_AIRTIGHT_STATUS;

typedef enum
{
	eVIDEO_YUV_REC601 = 0,
	eVIDEO_YUV_REC709,
	eVIDEO_YUV_EXTENDED_COLORIMETRY,      //ZU860_Doulas_0097

	eVIDEO_YUV_UNKNOW,
	eVIDEO_YUV_BT2020,                    //ZU860_Doulas_0097
} eVIDEO_YUV;        //A70LV_Doulas_0109 for front get value

typedef enum
{
    eVGA_SYNC_TYPE_SOG = 0,
    eVGA_SYNC_TYPE_SEPARATE,

    eVGA_SYNC_TYPE_UNKNOW
} eVGA_SYNC_TYPE;        //A70LV_Doulas_0109

#if 0
typedef enum
{
    e3D_ENABLE_AUTO,
    e3D_ENABLE_FRAMEPACKING,
    e3D_ENABLE_SIDEBYSIDE,
    e3D_ENABLE_TOPANDBOTTOM,
    e3D_ENABLE_FORCE_3D,     //Frame sequential
    e3D_ENABLE_OFF,
} e3DENABLE;
#endif

typedef enum
{
    eSKIN_COLOR_OFF,
    eSKIN_COLOR_LOW,
    eSKIN_COLOR_MIDDLE,
    eSKIN_COLOR_HIGH,

    eSKIN_COLOR_MAX_NUMBER
} eSKIN_COLOR;

typedef enum
{
    eTEMPORAL_NR_OFF,
    eTEMPORAL_NR_LOW,
    eTEMPORAL_NR_MIDDLE,
    eTEMPORAL_NR_HIGH,

    eTEMPORAL_NR_MAX_NUMBER
} eTEMPORAL_NOISE_REDUCTION;

typedef enum
{
    eMPEG_NR_OFF,
    eMPEG_NR_LOW,
    eMPEG_NR_MIDDLE,
    eMPEG_NR_HIGH,

    eMPEG_NR_MAX_NUMBER
} eMPEG_NOISE_REDUCTION;

#if 0
typedef enum
{
    eCOLOR_TEMPERATURE_WARMEST,
    eCOLOR_TEMPERATURE_WARM,
    eCOLOR_TEMPERATURE_COOL,

    eCOLOR_TEMPERATURE_MAX_NUMBER
} eCOLOR_TEMPERATURE;
#endif

typedef enum
{
    eCOLOR_ENHANCEMENT_OFF,
    eCOLOR_ENHANCEMENT_CE1,
    eCOLOR_ENHANCEMENT_CE2,

    eCOLOR_ENHANCEMENT_MAX_NUMBER
} eCOLOR_ENHANCEMENT;

typedef enum
{
    eHDR_AUTO,
    eHDR_OFF,
    eHDR_SDR,
    eHDR_HDR1,
    eHDR_HDR2,
    eHDR_HDR3,
    eHDR_HDR4,

    eHDR_MAX_NUMBER
} eHDR_INDEX;

typedef enum
{
    eHDR_SELECT_OFF,
    eHDR_SELECT_HDR10,
    eHDR_SELECT_HDR10P_LIKE,
    eHDR_SELECT_NUMBER,
} eHDR_SELECT_ID;
typedef enum
{
	eHDR_PICTURE_MODE_BRIGHT,
    eHDR_PICTURE_MODE_STANDARD,
    eHDR_PICTURE_MODE_FILM,
    eHDR_PICTURE_MODE_DETAIL,
//    eHDR_PICTURE_MODE_SMPTE_2084,   //ZU860_Doulas_0100 remove

	eHDR_PICTURE_MODE_MAX_NUMBER,
} eHDR_PICTURE_MODE;        //ZU860_Doulas_0068

typedef enum
{
    eMEMC_DETAIL_OFF,
    eMEMC_DETAIL_1,
    eMEMC_DETAIL_2,
    eMEMC_DETAIL_3,
}eMEMC_DETAIL;

typedef enum
{
    eMEMC_COLOR_OFF,
    eMEMC_COLOR_1,
    eMEMC_COLOR_2,
    eMEMC_COLOR_3,
    eMEMC_COLOR_4,
    eMEMC_COLOR_5,
}eMEMC_COLOR;

typedef enum
{
    eMEMC_MOTION_OFF,
    eMEMC_MOTION_1,
    eMEMC_MOTION_2,
    eMEMC_MOTION_3,
}eMEMC_MOTION;

typedef enum
{
    eMEMC_MOTION_DEMO_OFF,
    eMEMC_MOTION_DEMO_H_SPLIT,
    eMEMC_MOTION_DEMO_V_SPLIT,
}eMEMC_MOTION_DEMO;

typedef enum
{
    eEDGE_ENHANCEMENT_OFF,
    eEDGE_ENHANCEMENT_NORMAL,
    eEDGE_ENHANCEMENT_MAXIMUM,

    eEDGE_ENHANCEMENT_MAX_NUMBER
} eEDGE_ENHANCEMENT;

#if 0
typedef enum
{
    eCOLOR_SPEED_2X,
    eCOLOR_SPEED_3X,

    eCOLOR_SPEED_MAX_NUMBER
} eCOLOR_SPEED;
#endif


#if 0
typedef enum
{
    eLOGO_PATTERN_FACTORY_LOGO,
    eLOGO_PATTERN_BLUE,
    eLOGO_PATTERN_BLACK,
    eLOGO_PATTERN_WHITE,

    eLOGO_PATTERN_MAX_NUMBER
} eLOGO_PATTERN;
#endif

typedef enum
{
    eCOOLING_DOWN_INSTANT_OFF,
    eCOOLING_DOWN_1MIN,
    eCOOLING_DOWN_2MIN,

    eCOOLING_DOWN_MAX_NUMBER
} eCOOLING_DOWN;

typedef enum
{
    eBURNIN_TYPE_OFF,
    eBURNIN_TYPE_60_10_2,
    eBURNIN_TYPE_60_10_4,
    eBURNIN_TYPE_60_10_6,
    eBURNIN_TYPE_120_15_5,// HICC2_Bruce_0009
    eBURNIN_TYPE_60_15_18,// HICC2_Bruce_0009
    eBURNIN_TYPE_ALWAYS_ON,// HICC2_Bruce_0009

    eBURNIN_TYPE_MAX_NUMBER
} eBURNIN_TYPE;

typedef enum
{
    eFILL_COLOR_RED2,
    eFILL_COLOR_GREEN2,
    eFILL_COLOR_BLUE2,
    eFILL_COLOR_YELLOW2,
    eFILL_COLOR_WHITE2,
    eFILL_COLOR_BLACK2,
} eFILL_COLOR2;

#if 0
typedef enum
{
#if CUSTOM_BARCO    //A65_Owen_0002   //ZU860_Doulas_0009 modify
	eWALL_COLOR_OFF,
	eWALL_COLOR_BLACKBOARD,
	eWALL_COLOR_LIGHT_YELLOW,
	eWALL_COLOR_LIGHT_GREEN,
	eWALL_COLOR_LIGHT_BLUE,
	eWALL_COLOR_PINK,
	eWALL_COLOR_GRAY,
#else
    eWALL_COLOR_WHITE,
    eWALL_COLOR_GRAY130,
    eWALL_COLOR_LIGHT_YELLOW,
#endif
    eWALL_COLOR_MAX_NUMBER
} eWALL_COLOR;
#endif

#if 0
typedef enum //G100_Steven_0005
{
	eEDID_HDMI_1p4,
	eEDID_HDMI_2p0,

	eEDID_HDMI_MAX_NUMBER
} eEDID_HDMI_SETTING;
#endif

#if 0
typedef enum
{
#if 1  //CUSTOM_BARCO
    //G100_Wilsonj_0051 Start
    eHOTKEY_SETTINGS_FREEZE_SCREEN,
    eHOTKEY_SETTINGS_BLANK_SCREEN,
    eHOTKEY_SETTINGS_SAVE_TO_USER,
    eHOTKEY_SETTINGS_PIP_PBP,
    eHOTKEY_SETTINGS_ASPECT_RATIO,
    eHOTKEY_SETTINGS_PC_MODE,
    eHOTKEY_SETTINGS_SHOW_MESSAGES,
    eHOTKEY_SETTINGS_USER_DATA,
    eHOTKEY_SETTINGS_NETWORK_SETUP,
    eHOTKEY_SETTINGS_PROJECTOR_ID,
    eHOTKEY_SETTINGS_ORIENTATION,
    //eHOTKEY_SETTINGS_LOGO_CAPTURE,  //G100_Wilsonj_0054 mask
    eHOTKEY_SETTINGS_CUSTOM_RGBCYM,
    eHOTKEY_SETTINGS_MULTI_PROJECTION,
    eHOTKEY_SETTINGS_RESET_SELECTIVE,
    //G100_Wilsonj_0051 End
#else
    eHOTKEY_SETTINGS_SIZE_PRESETS,
    eHOTKEY_SETTINGS_PICTURE_SETTINGS,
    eHOTKEY_SETTINGS_DEFAULT_VALUE,
    eHOTKEY_SETTINGS_DETAIL, //A70LV_Larry_0111
    eHOTKEY_SETTINGS_CONTRAST_ENHANCEMENT,
    eHOTKEY_SETTINGS_LIGHT_SOURCE_MODE,
    eHOTKEY_SETTINGS_FREEZE_SCREEN, //A70LV_Larry_0111
    eHOTKEY_SETTINGS_INFORMATION,
#endif
    eHOTKEY_SETTINGS_MAX_NUMBER
} eHOTKEY_SETTINGS;
#endif


typedef enum
{
#if CUSTOM_BARCO    //A65_Owen_0002 //ZU860_Doulas_0031
	eSERVICE_TP_OFF,
	eSERVICE_TP_GREEN_GRID,
	eSERVICE_TP_MAGENTA_GRID,
	eSERVICE_TP_WHITE_GRID,
	eSERVICE_TP_WHITE,
	eSERVICE_TP_BLACK,
	eSERVICE_TP_RED,
	eSERVICE_TP_GREEN,
	eSERVICE_TP_BLUE,
	eSERVICE_TP_YELLOW,
	eSERVICE_TP_MAGENTA,
	eSERVICE_TP_CYAN,
	eSERVICE_TP_MAX_NUMBER,

	eSERVICE_TP_GRID,
	eSERVICE_TP_CHECKERBOARD,
	eSERVICE_TP_COLOR_BAR,
	eSERVICE_TP_BORESIGHT,
	eSERVICE_TP_FULL_SCREEN,
	eSERVICE_TP_GRAY,  //T100_Simon_0028
	eSERVICE_TP_WHITE_GRID_4CORNER,    //ZU860_Doulas_0094
#else
	/* 00 */ eSERVICE_TP_OFF,
	/* 01 */ eSERVICE_TP_GRID,
	/* 02 */ eSERVICE_TP_WHITE,
	/* 03 */ eSERVICE_TP_BLACK,
	/* 04 */ eSERVICE_TP_CHECKERBOARD,
	/* 05 */ eSERVICE_TP_COLOR_BAR,
	/* 06 */ eSERVICE_TP_RED,
	/* 07 */ eSERVICE_TP_GREEN,
	/* 08 */ eSERVICE_TP_BLUE,
	/* 09 */ eSERVICE_TP_YELLOW,
	/* 10 */ eSERVICE_TP_MAGENTA,
	/* 11 */ eSERVICE_TP_CYAN,
    /* 12 */ eSERVICE_TP_FULL_SCREEN,
    /* 13 */ eSERVICE_TP_GREEN_GRID,
    /* 14 */ eSERVICE_TP_MAGENTA_GRID,
    /* 15 */ eSERVICE_TP_WHITE_GRID,
    /* 16 */ eSERVICE_TP_WHITE_GRID_4CORNER,
    /* 17 */ eSERVICE_TP_BORESIGHT,
    /* 18 */ eSERVICE_TP_GRAY,
    /* 19 */ eSERVICE_TP_13POINTS,

    eSERVICE_TP_MAX_NUMBER
#endif
} eSERVICE_TEST_PATTERN;

typedef enum
{
    eLIGHT_SENSOR_CALIBRATION_MODE_DEFAULT,
    eLIGHT_SENSOR_CALIBRATION_MODE_MANUAL,

    eLIGHT_SENSOR_CALIBRATION_MODE_MAX
} eLIGHT_SENSOR_CALIBRATION_MODE;      //A70LV_Doulas_0024

typedef enum
{
    eOSD_TP_OFF,
#if defined(CUSTOM_BARCO) || defined(CUSTOM_OPTOMA)    //G100_Wilsonj_0017
    eOSD_TP_GREEN_GRID,
    eOSD_TP_MAGENTA_GRID,
    eOSD_TP_WHITE_GRID,
    eOSD_TP_WHITE,
    eOSD_TP_BLACK,
    eOSD_TP_RED,
    eOSD_TP_GREEN,
    eOSD_TP_BLUE,
    eOSD_TP_YELLOW,
    eOSD_TP_MAGENTA,
    eOSD_TP_CYAN,
#else
    eOSD_TP_GRID,
    eOSD_TP_WHITE,
    eOSD_TP_BLACK,
    eOSD_TP_CHECKERBOARD,
    eOSD_TP_COLOR_BAR,
#endif

    eOSD_TP_MAX_NUMBER
} eOSD_TEST_PATTERN;

typedef enum
{
    eSOURCE_MESSAGE_NOT_SUPPORTED,
    eSOURCE_MESSAGE_SOURCE_SIGNAL_REQUIRED,
    eSOURCE_MESSAGE_DETECTING,
    eSOURCE_MESSAGE_FREEZE,
    eSOURCE_MESSAGE_OVER_TEMPERATURE, //A70LV_Larry_0357
    eSOURCE_MESSAGE_FAN_STALL,   //A70LV_Larry_0357
    eSOURCE_MESSAGE_PUMP_STALL,  //A70LV_Larry_0357


    eSOURCE_MESSAGE_NUMBER
}eSOURCE_MESSAGE; //A70LV_Larry_0110

typedef enum
{
    eMODE_TABLE_ID_INDEX_SOG,
    eMODE_TABLE_ID_INDEX_PC_WIDE,       //A70LV_Doulas_0123 modify
    eMODE_TABLE_ID_INDEX_PC_NORMAL,     //A70LV_Doulas_0123 modify
} eMODE_TABLE_ID_INDEX;     //A70LV_Doulas_0112

typedef enum
{
    eAPP_ADC_STATE_IDLE,
    eAPP_ADC_STATE_CALIBRATION,
    eAPP_ADC_STATE_INVALID
} eAPP_ADC_STATE;   //A70LV_Doulas_0124

typedef enum
{
    nbSUCCESS,
    nbERROR,
    nbPENDING,
    nbINVALID
} eNBRESULT;        //A70LV_Doulas_0124

typedef enum
{
    calstateIDLE,
    calstateLEVELINIT,
    calstateLEVELPROCESS,
    calstateINVALID                    // used for enumerated type range
} eCAL_CTRL_STATE;      //A70LV_Doulas_0124


#ifdef Low_Latency_All
typedef enum
{
    eINPUT_3D_TYPE2_OFF,
    eINPUT_3D_TYPE2_1P4,
    eINPUT_3D_TYPE2_FRAME_SEQUENTIAL,    //120hz
} eINPUT_3D_TYPE2;    //A70LV_Doulas_0154
#endif	/*Low_Latency_All*/

typedef enum
{
    adcIDLE,
    adcCALIBRATE,
    adcINVALID                         // Must be LAST value. Used for range
    // checking (DO NOT REMOVE)
} eADC_MGR_STATE;       //A70LV_Doulas_0124

typedef enum
{
    eVIDEO_3D_FORMAT_2D = 0,
    eVIDEO_3D_FORMAT_FP,
    eVIDEO_3D_FORMAT_TB,
    eVIDEO_3D_FORMAT_SS,

    eVIDEO_3D_FORMAT_UNKNOW = 0xFF
} eVIDEO_3D_FORMAT;     //A70LV_Doulas_0154

typedef enum
{
    e3D_SYNC_INPUT_SELECTION_EXTERNAL_INPUT_3D_SYNC = 0,    //A70LV_Doulas_0274 modify
    e3D_SYNC_INPUT_SELECTION_C821_OUTPUT_3D_SYNC,           //A70LV_Doulas_0274 modify

    e3D_SYNC_INPUT_SELECTION_UNKNOW
} e3D_SYNC_INPUT_SELECTION;     //A70LV_Doulas_0154

#if 0
typedef enum
{
    e3D_REFERENCE_1ST_FRAME,
    e3D_REFERENCE_FIELD_GPIO,

} e3D_REFERENCE; //A70LV_Larry_0169
#endif

#if 0
typedef enum
{
    eBLENDING_GAMMA_1_8,
    eBLENDING_GAMMA_1_9,
    eBLENDING_GAMMA_2_0,
    eBLENDING_GAMMA_2_1,
    eBLENDING_GAMMA_2_2,
    eBLENDING_GAMMA_2_3,
    eBLENDING_GAMMA_2_4,

} eBLENDING_GAMMA; //A70LV_Larry_0169
#endif

typedef enum
{
    /* 00 */ eSPLASH_PICTURE_CHRISTIE_LOGO = 0,
    /* 01 */ eSPLASH_PICTURE_SCREENX_LOGO,
    /* 02 */ eSPLASH_PICTURE_FULL_SCREEN,
    /* 03 */ eSPLASH_PICTURE_BORESIGHT_0,
    /* 04 */ eSPLASH_PICTURE_BORESIGHT_1,
    /* 05 */ eSPLASH_PICTURE_BORESIGHT_2,
    /* 06 */ eSPLASH_PICTURE_BORESIGHT_3,
    /* 07 */ eSPLASH_PICTURE_TP_GRID_HD,
    /* 08 */ eSPLASH_PICTURE_TP_GRID_WU,
    /* 09 */ eSPLASH_PICTURE_TP_13POINTS,
    eSPLASH_PICTURE_NUMBER,
} eSPLASH_PICTURE; //A70LV_Larry_0233

#if 0
typedef enum
{
    eTWIST_TP_OFF,
    eTWIST_TP_GRID,
    eTWIST_TP_WHITE,
    eTWIST_TP_BLACK,
    eTWIST_TP_CHECKERBOARD,
    eTWIST_TP_COLOR_BAR,
    eTWIST_TP_GRAY,

    eTWIST_TP_MAX_NUMBER
} eTWIST_TEST_PATTERN;      //T100_Simon_0028   //T100_Simon_0029
#endif

typedef enum
{
    eSOURCE_MODE_SEAMLESS,
    eSOURCE_MODE_BACKUP,
    eSOURCE_MODE_PIPPOP,
    eSOURCE_MODE_SIGNLE,

    eSOURCE_MODE_NUMBER,
}eSOURCE_MODE; //FPGA configure

typedef enum
{
    eBACKUP_SOURCE_STATUS_INAVTIVE,
    eBACKUP_SOURCE_STATUS_AVTIVE,

    eBACKUP_SOURCE_STATUS_NUMBER,
}eBACKUP_SOURCE_STATUS;

typedef enum
{
    eBACKUP_INPUT_NO_CHANGE,
    eBACKUP_INPUT_CHANGED,

    eBACKUP_INPUT_CHANGE_NUMBER,
}eBACKUP_INPUT_CHANGE;	//A65_OPTOMA_Doulas_0115

typedef enum
{
    eBACKUP_INPUT_HDR_NO,
    eBACKUP_INPUT_HDR_YES,
    eBACKUP_INPUT_HDR_NO_SIGNAL,

    eBACKUP_INPUT_HDR_NUMBER,
}eBACKUP_INPUT_HDR_STATUS;


typedef enum
{
    eWAPRING_AP_GROUP_2D,
    eWAPRING_AP_GROUP_3D,
    eWAPRING_AP_GROUP_4K3D,
    eWAPRING_AP_GROUP_4K3D_DUALPIPE,
    eWAPRING_AP_GROUP_HIGH_SPEED,

    eWAPRING_AP_GROUP_NUMBER,
} eWAPRING_AP_GROUP;

#if 0
typedef enum    //T100_Simon_0038
{
    eUSER_SOURCE_ID_VGA         = 1,
    eUSER_SOURCE_ID_BNC         = 2,
    eUSER_SOURCE_ID_HDMI1       = 3,
    eUSER_SOURCE_ID_HDMI2       = 4,
    eUSER_SOURCE_ID_DVI         = 5,
    eUSER_SOURCE_ID_DISPLAYPORT = 6,
    eUSER_SOURCE_ID_3GSDI       = 7,
    eUSER_SOURCE_ID_HDBASET     = 8,
    eUSER_SOURCE_ID_COMPOSITE   = 9,
    eUSER_SOURCE_ID_PRESENTER   = 10,
    eUSER_SOURCE_ID_CARDREADER  = 11,
    eUSER_SOURCE_ID_MINIUSB     = 12,
    eUSER_SOURCE_ID_SLOT1       = 13,
    eUSER_SOURCE_ID_SLOT2       = 14,
}eUSER_SOURCE_ID;
#endif

typedef enum
{
	eHSG_COLOR_RED,
	eHSG_COLOR_GREEN,
	eHSG_COLOR_BLUE,
	eHSG_COLOR_CYAN,
	eHSG_COLOR_MAGENTA,
	eHSG_COLOR_YELLOW,
	eHSG_COLOR_WHITE,
} eHSG_COLOR;

typedef enum
{
	eSLOT_LAN_PATH_FROM_SLOT1,
	eSLOT_LAN_PATH_FROM_SLOT2,
	eSLOT_LAN_PATH_INVALID,
} eSLOT_LAN_PATH;       //ZU860_Doulas_0004

#if 0
#if defined(CUSTOM_BARCO) || defined(CUSTOM_OPTOMA)     //A65_Owen_0002
typedef enum
{
    eBACKGROUND_BLUE,
    eBACKGROUND_BLACK,
    eBACKGROUND_WHITE,
    eBACKGROUND_LOGO,
} eBACKGROUND_COLOR;        //ZU860_Doulas_0004
#else
typedef enum
{
	eBACKGROUND_LOGO,   //ZU860_Doulas_0012 modify//ZU860_Doulas_0006
	eBACKGROUND_BLUE,
	eBACKGROUND_NONE,
	eBACKGROUND_WHITE,  //ZU860_Doulas_0006
    eBACKGROUND_RED,
    eBACKGROUND_GREEN,
	eBACKGROUND_GRAY,

} eBACKGROUND_COLOR;        //ZU860_Doulas_0004
#endif
#endif

#if 0
typedef enum
{
	eMENU_TIME_OUT_OFF,
	eMENU_TIME_OUT_5SEC,
	eMENU_TIME_OUT_10SEC,
	eMENU_TIME_OUT_15SEC,

	eMENU_TIME_OUT_MAX_NUMBER
} eMENU_TIME_OUT;   //ZU860_Doulas_0033
#endif

#if 0
typedef enum
{
    e3D_2D_3D,
	e3D_2D_L,
	e3D_2D_R,
} e3D_2D;       //ZU860_Doulas_0036
#endif

#if 0
#if defined(CUSTOM_BARCO) || defined(CUSTOM_OPTOMA) //A65_Owen_0002
typedef enum
{
    e3D_TECH_DLP_LINK,
	e3D_TECH_3D_SYNC,
	e3D_TECH_MAX_NUMBER,
} e3D_TECH;       //ZU860_Doulas_0038

#else
typedef enum
{
    e3D_TECH_3D_SYNC,
	e3D_TECH_DLP_LINK,
	e3D_TECH_MAX_NUMBER,
} e3D_TECH;       //ZU860_Doulas_0038
#endif
#endif

typedef enum
{
	eCOLOR_DEPTH_8BIT,
	eCOLOR_DEPTH_10BIT,
	eCOLOR_DEPTH_12BIT,
	eCOLOR_DEPTH_16BIT,

	eCOLOR_DEPTH_MAX_NUMBER
} eCOLOR_DEPTH;       //ZU860_Doulas_0097

typedef enum
{
	eHDR_SETTING_SDR,
	eHDR_SETTING_HDR,

	eHDR_SETTING_MAX_NUMBER
} eHDR_SETTING;       //ZU860_Doulas_0097

typedef enum
{
	eEXTENDED_COLORIMETRY_xvYCC601,
	eEXTENDED_COLORIMETRY_xvYCC709,
	eEXTENDED_COLORIMETRY_sYCC601,
	eEXTENDED_COLORIMETRY_opYCC601,
	eEXTENDED_COLORIMETRY_opRGB,
	eEXTENDED_COLORIMETRY_BT2020_YcCbcCrc,
	eEXTENDED_COLORIMETRY_BT2020RGB_YCbCr,

	eEXTENDED_COLORIMETRY_MAX_NUMBER
} eEXTENDED_COLORIMETRY;  //ZU860_Doulas_0100

typedef enum
{
    DDP_ASPECT_FILL         = 0,        //fill (uses DMD aspect ratio)
    DDP_ASPECT_NATIVE       = 1,        //native
    DDP_ASPECT_4_3          = 2,        //4:3
    DDP_ASPECT_16_9         = 3,        //16:9
    DDP_ASPECT_ANAMORPHIC   = 4,        //anamorphic
    DDP_ASPECT_MAX
} DDP_ASPECT_RATIO_TYPE;        //ZU860_Doulas_0119

typedef enum
{
    eWARPING_SETTING_ALL_OFF,
    eWARPING_SETTING_ALL_ON,
    eWARPING_SETTING_BLEND_OFF,

    eWARPING_SETTING_MAX_NUMBER
} eWARPING_SETTING;             //ZU860_Owen_0011

typedef enum
{
    eUSER_DATA_SAVE0,
    eUSER_DATA_SAVE1,
    eUSER_DATA_SAVE2,
    eUSER_DATA_SAVE3,
    eUSER_DATA_SAVE4,

    eUSER_DATA_SAVE_MAX_NUMBER
} eUSER_DATA_SAVE_SETTING;             //G100_Doulas_0002

typedef enum
{
    eUSER_DATA_LOAD0,
    eUSER_DATA_LOAD1,
    eUSER_DATA_LOAD2,
    eUSER_DATA_LOAD3,
    eUSER_DATA_LOAD4,

    eUSER_DATA_LOAD_MAX_NUMBER
} eUSER_DATA_LOAD_SETTING;             //G100_Doulas_0002

typedef enum
{
    eLENS_MODEL_CORE,
	eLENS_MODEL_OTHER,
	eLENS_MODEL_NUMBER,
} eLENS_MODEL;       //SNPLU9000_Energy_0006

typedef enum
{
    eBACKUP_RESTORE_NORMAL,
    eBACKUP_RESTORE_CHANGE_PROJECTOR,	//換機台
    eBACKUP_RESTORE_CHANGE_MAIN_BOARD,	//換版子

    eBACKUP_RESTORE_MAX_NUMBER
} e_BACKUP_RESTORE_TYPE;	//G100_Doulas_0004


typedef enum
{
	_4CORNER_DEFAULT_VALUE,
	_4CORNER_LEFT_TOP = 0,
	_4CORNER_RIGHT_TOP,
	_4CORNER_LEFT_BOTTOM,
	_4CORNER_RIGHT_BOTTOM

} _4CORNER_CURRENT_LOCATION;	//ZU860_Clare_0001

//G100_Wilsonj_0007 Start
typedef enum
{
    eITEM_0,
    eITEM_1,
    eITEM_2,
    eITEM_3,
    eITEM_4,
    eITEM_5,
    eITEM_6,
    eITEM_7,
    eITEM_8,
    eITEM_9,
    eITEM_10,
    eITEM_11,
    eITEM_12,
    eITEM_13,
    eITEM_14,
    eITEM_15,
    eITEM_16,
    eITEM_17,

    eITEM_NUMBER
}eITEM;
//G100_Wilsonj_0007 End

#if 0
typedef enum
{
    eLAN_PATH_RJ45,
    eLAN_PATH_HDBT,
    eLAN_PATH_NUMBER,
} eLAN_PATH;                //G100_Owen_0026
#endif

typedef enum
{
    eLAN_STATUS_DISCONNECT,
    eLAN_STATUS_CONNECT,
    eLAN_STATUS_NUMBER,
} eLAN_STATUS;              //G100_Owen_0028

typedef enum 	//G100_Coda_0013
{
    eSCHEDULE_EVENT_FUNCTION_OFF,
    eSCHEDULE_EVENT_FUNCTION_POWER_SETTINGS,
    eSCHEDULE_EVENT_FUNCTION_INPUT_SOURCE,
    eSCHEDULE_EVENT_FUNCTION_LIGHT_SOURCE_MODE,
    eSCHEDULE_EVENT_FUNCTION_SHUTTER,

    eSCHEDULE_EVENT_FUNCTION_NUMBER,

}eSCHEDULE_EVENT_FUNCTION;	 //G100_Coda_0021

typedef enum 	//G100_Coda_0013
{
    eSCHEDULE_EVENT_CONTENT_OFF,
    eSCHEDULE_EVENT_CONTENT_1,
    eSCHEDULE_EVENT_CONTENT_2,
    eSCHEDULE_EVENT_CONTENT_3,
    eSCHEDULE_EVENT_CONTENT_4,
    eSCHEDULE_EVENT_CONTENT_5,
    eSCHEDULE_EVENT_CONTENT_6,
    eSCHEDULE_EVENT_CONTENT_7,
    eSCHEDULE_EVENT_CONTENT_8,
    eSCHEDULE_EVENT_CONTENT_9,
    eSCHEDULE_EVENT_CONTENT_10,
    eSCHEDULE_EVENT_CONTENT_NUMBER,

} eSCHEDULE_EVENT_CONTENT;


typedef enum 	//G100_Coda_0013
{
    eNTP_SERVER_LIST_1,
    eNTP_SERVER_LIST_2,
    eNTP_SERVER_LIST_3,
    eNTP_SERVER_LIST_4,
    eNTP_SERVER_LIST_NUMBER,

}eNTP_SERVER_LIST;	 ////G100_Coda_0033


#ifdef PLATFORM_A70G2

typedef enum
{
	//warning
    eWARNING_MESSAGE_PUMP_SPEED_WARN,
    eWARNING_MESSAGE_FAN1_SPEED_WARN,
    eWARNING_MESSAGE_FAN2_SPEED_WARN,
    eWARNING_MESSAGE_FAN3_SPEED_WARN,
    eWARNING_MESSAGE_FAN4_SPEED_WARN,
    eWARNING_MESSAGE_FAN5_SPEED_WARN,
    eWARNING_MESSAGE_FAN6_SPEED_WARN,
    eWARNING_MESSAGE_FAN7_SPEED_WARN,
    eWARNING_MESSAGE_FAN8_SPEED_WARN,
    eWARNING_MESSAGE_FAN9_SPEED_WARN,
    eWARNING_MESSAGE_FAN10_SPEED_WARN,
    eWARNING_MESSAGE_FAN11_SPEED_WARN,
    eWARNING_MESSAGE_FAN12_SPEED_WARN,
    eWARNING_MESSAGE_SYS_TEMP_WARN,
    eWARNING_MESSAGE_DMD_TEMP_WARN,
    eWARNING_MESSAGE_BLD_TEMP_WARN,
    eWARNING_MESSAGE_RLD_TEMP_WARN,
	eWARNING_MESSAGE_AMBIENT_TEMP_WARN,

	//error
    eWARNING_MESSAGE_PUMP_SPEED_ERROR,
    eWARNING_MESSAGE_FAN1_SPEED_ERROR,
    eWARNING_MESSAGE_FAN2_SPEED_ERROR,
    eWARNING_MESSAGE_FAN3_SPEED_ERROR,
    eWARNING_MESSAGE_FAN4_SPEED_ERROR,
    eWARNING_MESSAGE_FAN5_SPEED_ERROR,
    eWARNING_MESSAGE_FAN6_SPEED_ERROR,
    eWARNING_MESSAGE_FAN7_SPEED_ERROR,
    eWARNING_MESSAGE_FAN8_SPEED_ERROR,
    eWARNING_MESSAGE_FAN9_SPEED_ERROR,
    eWARNING_MESSAGE_FAN10_SPEED_ERROR,
    eWARNING_MESSAGE_FAN11_SPEED_ERROR,
    eWARNING_MESSAGE_FAN12_SPEED_ERROR,
    eWARNING_MESSAGE_SYS_TEMP_ERROR,
    eWARNING_MESSAGE_DMD_TEMP_ERROR,
    eWARNING_MESSAGE_BLD_TEMP_ERROR,
    eWARNING_MESSAGE_RLD_TEMP_ERROR,
    eWARNING_MESSAGE_AMBIENT_TEMP_ERROR,


    eWARNING_MESSAGE_NUMBER
}eWARNING_MESSAGE;	//A70LV_Julie_0008

#elif defined(PLATFORM_A35G2)  //35G2
typedef enum
{
    eWARNING_MESSAGE_PUMP_SPEED_WARN,
    eWARNING_MESSAGE_FAN1_SPEED_WARN,
    eWARNING_MESSAGE_FAN2_SPEED_WARN,
    eWARNING_MESSAGE_FAN3_SPEED_WARN,
    eWARNING_MESSAGE_FAN4_SPEED_WARN,
    eWARNING_MESSAGE_FAN5_SPEED_WARN,
    eWARNING_MESSAGE_FAN6_SPEED_WARN,
    eWARNING_MESSAGE_FAN7_SPEED_WARN,
    eWARNING_MESSAGE_FAN8_SPEED_WARN,
    eWARNING_MESSAGE_FAN9_SPEED_WARN,
    eWARNING_MESSAGE_FAN10_SPEED_WARN,
    eWARNING_MESSAGE_SYS_TEMP_WARN,
    eWARNING_MESSAGE_DMD_TEMP_WARN,
    eWARNING_MESSAGE_BLD_TEMP_WARN,
    eWARNING_MESSAGE_RLD_TEMP_WARN,
    eWARNING_MESSAGE_PUMP_SPEED_ERROR,
    eWARNING_MESSAGE_FAN1_SPEED_ERROR,
    eWARNING_MESSAGE_FAN2_SPEED_ERROR,
    eWARNING_MESSAGE_FAN3_SPEED_ERROR,
    eWARNING_MESSAGE_FAN4_SPEED_ERROR,
    eWARNING_MESSAGE_FAN5_SPEED_ERROR,
    eWARNING_MESSAGE_FAN6_SPEED_ERROR,
    eWARNING_MESSAGE_FAN7_SPEED_ERROR,
    eWARNING_MESSAGE_FAN8_SPEED_ERROR,
    eWARNING_MESSAGE_FAN9_SPEED_ERROR,
    eWARNING_MESSAGE_FAN10_SPEED_ERROR,
    eWARNING_MESSAGE_SYS_TEMP_ERROR,
    eWARNING_MESSAGE_DMD_TEMP_ERROR,
    eWARNING_MESSAGE_BLD_TEMP_ERROR,
    eWARNING_MESSAGE_RLD_TEMP_ERROR,

    eWARNING_MESSAGE_NUMBER
}eWARNING_MESSAGE;	//A70LV_Julie_0008

#else

typedef enum
{
	//warning
    eWARNING_MESSAGE_PUMP_SPEED_WARN,
    eWARNING_MESSAGE_FAN1_SPEED_WARN,
    eWARNING_MESSAGE_FAN2_SPEED_WARN,
    eWARNING_MESSAGE_FAN3_SPEED_WARN,
    eWARNING_MESSAGE_FAN4_SPEED_WARN,
    eWARNING_MESSAGE_FAN5_SPEED_WARN,
    eWARNING_MESSAGE_FAN6_SPEED_WARN,
    eWARNING_MESSAGE_FAN7_SPEED_WARN,
    eWARNING_MESSAGE_FAN8_SPEED_WARN,
    eWARNING_MESSAGE_FAN9_SPEED_WARN,
    eWARNING_MESSAGE_FAN10_SPEED_WARN,
    eWARNING_MESSAGE_FAN11_SPEED_WARN,
    eWARNING_MESSAGE_FAN12_SPEED_WARN,
    eWARNING_MESSAGE_SYS_TEMP_WARN,
    eWARNING_MESSAGE_DMD_TEMP_WARN,
    eWARNING_MESSAGE_BLD_TEMP_WARN,
    eWARNING_MESSAGE_RLD_TEMP_WARN,
	eWARNING_MESSAGE_AMBIENT_TEMP_WARN,

	//error
    eWARNING_MESSAGE_PUMP_SPEED_ERROR,
    eWARNING_MESSAGE_FAN1_SPEED_ERROR,
    eWARNING_MESSAGE_FAN2_SPEED_ERROR,
    eWARNING_MESSAGE_FAN3_SPEED_ERROR,
    eWARNING_MESSAGE_FAN4_SPEED_ERROR,
    eWARNING_MESSAGE_FAN5_SPEED_ERROR,
    eWARNING_MESSAGE_FAN6_SPEED_ERROR,
    eWARNING_MESSAGE_FAN7_SPEED_ERROR,
    eWARNING_MESSAGE_FAN8_SPEED_ERROR,
    eWARNING_MESSAGE_FAN9_SPEED_ERROR,
    eWARNING_MESSAGE_FAN10_SPEED_ERROR,
    eWARNING_MESSAGE_FAN11_SPEED_ERROR,
    eWARNING_MESSAGE_FAN12_SPEED_ERROR,
    eWARNING_MESSAGE_SYS_TEMP_ERROR,
    eWARNING_MESSAGE_DMD_TEMP_ERROR,
    eWARNING_MESSAGE_BLD_TEMP_ERROR,
    eWARNING_MESSAGE_RLD_TEMP_ERROR,
    eWARNING_MESSAGE_AMBIENT_TEMP_ERROR,


    eWARNING_MESSAGE_NUMBER
}eWARNING_MESSAGE;	//A70LV_Julie_0008


#endif


typedef enum
{
    eERROR_MESSAGE_CHECK,
    eERROR_MESSAGE_SHUTDOWN,

    eERROR_MESSAGE_NUMBER
}eERROR_MESSAGE;	//A70LV_Julie_0008

typedef enum
{
    eCUSTOM_REGION_EMEA,
    eCUSTOM_REGION_NON_EMEA,
    eCUSTOM_REGION_NUMBERS,
} eCUSTOM_REGION_SETTING;		//A65_OPTOMA_Doulas_0027

typedef enum
{
    eCUSTOM_SPLASH_LOGO_DEFAULT,
    eCUSTOM_SPLASH_LOGO_BLUE,
    eCUSTOM_SPLASH_LOGO_USER,
    eCUSTOM_SPLASH_LOGO_NUMBERS,
} eCUSTOM_SPLASH_LOGO;		//A65_OPTOMA_Doulas_0027

typedef enum
{
    eBLACK_LEVEL_RESET_BOTTOM,
    eBLACK_LEVEL_RESET_TOP,
    eBLACK_LEVEL_RESET_ALL,
    eBLACK_LEVEL_RESET_NUMBERS,
} eBLACK_LEVEL_RESET;		//A65_OPTOMA_Doulas_0029

typedef enum //A35G2_Alan_0002
{
    eACU_SET_DATA_DISABLE = 0,          // The ACU data is Disable
	eACU_SET_DATA_ENABLE,				// The ACU data is Enaable
    eACU_DATA_MAX
} eACU_DATA_ENABLE_STATUS;


typedef enum
{
    eACU_FILE_INVALID = 0,              // The ACU data is invalid
    eACU_FILE_0_VALID = (1 << 0),       // The ACU data in file 0 is valid
    eACU_FILE_1_VALID = (1 << 1),       // The ACU data in file 1 is valid
    eACU_FILE_2_VALID = (1 << 2),       // The ACU data in file 2 is valid
    eACU_FILE_3_VALID = (1 << 3),       // The ACU data in file 3 is valid

    eACU_FILE_MAX = (1 << 4)
} eACU_DATA_STATUS;

//G100_Tim_0014, add, start
typedef enum
{
    eACU_SET_ICHIP_ONLY = 0,            // shall not save to file !!
    eACU_SAVE_FILE_0,
    eACU_SAVE_FILE_1,
    eACU_SAVE_FILE_2,
    eACU_SAVE_FILE_3,

    eACU_SAVE_FILE_MAX
} eACU_FILE_LIST;

typedef enum
{
    eUPDATE_POWER_STATUS_ACTIVE,        // send Power Active to LAN again.

    eUPDATE_POWER_STATUS_MAX
} eUPDATE_POWER_STATUS;                 // reference from ePOWER_STATE
//G100_Tim_0046, add, end

//A65_OPTOMA_Jerry_0004 start
typedef enum
{
    eCMD_KEY_DATACODE_NOT_ALLOWED_TO_EXECUTE,
    eCMD_KEY_DATACODE_ALLOWED_TO_EXECUTE,
    eCMD_KEY_DATACODE_INVALID
} eCMD_KEY_DATACODE_EXECUTE_STATUS;

typedef enum
{
    eFOTA_DISABLE,
    eFOTA_ENABLE,
    eFOTA_NUMBER
} eFOTA_ENABLE_STATUS;

typedef enum
{
    ePROSERVICE_PAIR_STATUS_IDLE = 0,
    ePROSERVICE_PAIR_STATUS_PAIR_REQUEST_TO_SERVER = 1,
    ePROSERVICE_PAIR_STATUS_PAIR_CODE_READY_FROM_SERVER = 2, // need 1 datacode to save pin code :  char array [8]
    ePROSERVICE_PAIR_STATUS_PAIR_CODE_CONFIRM_TO_SERVER = 3,
    ePROSERVICE_PAIR_STATUS_PAIR_CODE_SUCCESS_FROM_SERVER = 4,
    ePROSERVICE_PAIR_STATUS_PAIR_CODE_USER_ABORTED = 5,     //ZU860_Tim_0091, add
    ePROSERVICE_PAIR_STATUS_PAIR_CODE_ERROR = 10,
    ePROSERVICE_PAIR_STATUS_MAX_NUMBER = 100000,
} ePROSERVICE_PAIR_STATUS; // 1 data code to save binding status : UINT8

typedef enum
{
    ePROSERVICE_STATUS_IDLE = 0,
    ePROSERVICE_STATUS_NO_BINDING = 1,
    ePROSERVICE_STATUS_BINDING = 2,
    ePROSERVICE_STATUS_QUERY = 3,
    ePROSERVICE_STATUS_ERROR_REGISTER_LOST = 10,
    ePROSERVICE_STATUS_ERROR_NETWORK_TIMEOUT = 11,          //Optoma_Tim_0070, add
    ePROSERVICE_STATUS_MAX_NUMBER = 99,
} ePROSERVICE_STATUS; // 1 data code to save status :UINT8

typedef enum
{
    ePROSERVICE_LOCATION_WORLDWIDE = 0,
    ePROSERVICE_LOCATION_CHINA = 1,
} ePROSERVICE_LOCATION;

//G100_Tim_0057, add, start //A35G2_BRC_Casper_0060
typedef enum
{
    eSOURCE_READY_NO_SOURCE = 0,
    eSOURCE_READY_MAIN_ONLY,
    eSOURCE_READY_SUB_ONLY,
    eSOURCE_READY_BOTH,

    eSOURCE_READY__MAX
} eSOURCE_READY_STATUS;
//G100_Tim_0057, add, end

#if 0 // HICC2_Bruce_0022
typedef enum
{
    eNATIVE_TIMING_WU = 0,
    eNATIVE_TIMING_1080P,

    eNATIVE_TIMING_MAX
} eNATIVE_TIMING_STATUS;
#endif //G100_Tim_0058, add, end

typedef enum
{
    ePROSERVICE_TEMPERATURE_STATUS_GREEN,
    ePROSERVICE_TEMPERATURE_STATUS_ORANGE,
    ePROSERVICE_TEMPERATURE_STATUS_RED,
    ePROSERVICE_TEMPERATURE_STATUS_MAX_NUMBER
} ePROSERVICE_TEMPERATURE_STATUS;
//A65_OPTOMA_Jerry_0004 end //A35G2_Coda_0052

typedef enum
{
    eLENS_SPEED_AUTO,
    eLENS_SPEED_NORMAL,
    eLENS_SPEED_UST,

    eLENS_SPEED_NUMBER
} eLENS_SPEED_STATUS;

typedef enum
{
    eCOUNTRY_ORIGIN_WW,
    eCOUNTRY_ORIGIN_CHINA,
    eCOUNTRY_ORIGIN_TWN,
    eCOUNTRY_ORIGIN_NUMBER,
}eCOUNTRY_ORIGIN; ////A35G2_CDS_Coda_0010


#ifdef CUSTOM_CHRISTIE
typedef enum
{
    eMODEL_TYPE_DWU880GS,
    eMODEL_TYPE_DWU1100GS,
    eMODEL_TYPE_DWU1300GS, //TBD

    eMODEL_TYPE_NUMBER,
}eMODEL_TYPE;

#elif CUSTOM_OPTOMA
typedef enum
{
    eMODEL_TYPE_DWU880GS, //TBD
    eMODEL_TYPE_DWU1100GS, //TBD
    eMODEL_TYPE_DWU1300GS, //TBD

    eMODEL_TYPE_NUMBER,
}eMODEL_TYPE;

typedef enum
{
    eMODEL_REPLACE_W19,
    eMODEL_REPLACE_W16,

    eMODEL_REPLACE_UNKNOW,
}eMODEL_REPLACE_INDEX; //A65_OPTOMA_Julie_0056

#elif  CUSTOM_BARCO
typedef enum
{
    eMODEL_TYPE_DWU880GS,  //TBD
    eMODEL_TYPE_DWU1100GS, //TBD
    eMODEL_TYPE_DWU1300GS, //TBD

    eMODEL_TYPE_NUMBER,
}eMODEL_TYPE;
#else
typedef enum
{
    eMODEL_TYPE_NA, //TBD

    eMODEL_TYPE_NUMBER,
}eMODEL_TYPE;

#endif /* 0 */



typedef enum
{
    eGCI_OSD,
    eGCI_AP,

    eGCI_INVALID

}eGEO_CTRL_INTERFACE;


typedef enum
{
    eWRTIE_TO_RAM,     //only RAM
    eWRTIE_TO_BUFFER,  //RAM + BUFFER
    eWRTIE_TO_CHIP,    //RAM + BUFFER + CHIP register

    eWRTIE_NUMBER,

}eWARP_OSD_SET_ACTION;  //H2PF_Simon_0038

typedef enum
{
    eWOC_Palette_Single,    //Data Type : sPaletteSingleInfo
    eWOC_Palette_All,
    eWOC_Palette_RamDataToChip,

    eWOC_TransparentColorEnable,
    eWOC_TransparentColor0,
    eWOC_TransparentColor1,
    eWOC_TransparentColor2,
    eWOC_TransparentColor3,

    eWOC_BlinkEnable,
    eWOC_BlinkTime,
    eWOC_BlinkCycle,
    eWOC_BlinkPaletteSet0_0,  //for 0xF0  palette
    eWOC_BlinkPaletteSet0_1,  //for 0xF1  palette
    eWOC_BlinkPaletteSet1_0,  //for 0x100 palette
    eWOC_BlinkPaletteSet1_1,  //for 0x101 palette

    eWOC_Number,

}eWARPOSD_CONFIG;


typedef enum
{
    ehpd_OFF = 0,
    ehpd_ON,
	ehpd_LATER,
    ehpd_ACTION_NUMBERS
} eHPDAction; //A70LK_Nina_0024

typedef enum
{
    eLD_SEG_BLD_R,
    eLD_SEG_BLD_G,
    eLD_SEG_BLD_B,
    eLD_SEG_BLD_Y,
    eLD_SEG_RLD_R,
    eLD_SEG_RLD_Y,

    eLD_SEG_NUMBERS,
} eLD_SEG; //A70LV_Larry_0084

typedef enum
{
    eWAP_LEVEL_100,
    eWAP_LEVEL_50,
    eWAP_LEVEL_30,
    eWAP_LEVEL_10,

    eWAP_LEVEL__MAXNUMBER
}eWAP_Level; //G50_Casper_0006


typedef enum
{
    eLLM_OFF,                            //no framelock
    eLLM_LV1_FAST_WITH_FULL_WARPING,     //framelock 且 warping 可調整角度最大 (可調整角度同 eLLM_OFF)
    eLLM_LV2_FAST_WITH_LIMITED_WARPING,  //framelock, warping 可調整角度小，但 latency 低 (iChips 平台在 scaler+warping 設定為 0.2 frame)
    eLLM_LV3_FAST_WITHOUT_WARPING,       //framelock, bypass warping block

    eLLM_INVALID
}eLOW_LATENCY_MODE;      //H2PF_Simon_0150

//====================== Enum End ======================//


//===================== Struct Start ==================//
typedef struct
{
    UINT16                wLeft;
    UINT16                wTop;
    UINT16                wRight;
    UINT16                wBottom;
} RECT, *PRECT;     //A70LV_Doulas_0124

typedef struct
{
    UINT8  ucNewMA_Mode_Flag;
    UINT8  ucNewMA_Source_ID;
    UINT8  ucNewMA_Table_ID;
    UINT16 uiNewMA_Mode_ID;
    UINT16 uiNewMA_HPeriod;
    UINT16 uiNewMA_VTotal;
    UINT16 uiNewMA_HTotal;
    UINT16 uiNewMA_VActive;
    UINT16 uiNewMA_HActive;
    INT16  uiNewMA_HStart;
    INT16  uiNewMA_VStart;
}sMODE_ADJUSTMENT_TABLE,*PsMODE_ADJUSTMENT_TABLE;     //A70LV_Doulas_0195


typedef struct
{
    UINT8 ucFunctionType;
    UINT8 ucFunctionTypeStr[50];
    UINT32 ulOSDTextId;
} sEVENT_TYPE_TRANSLATION_TEXT_ID_LUT;	//G100_Coda_0018

typedef struct
{
    UINT8 ucEventIndex;
    UINT8 ucEventIndexStr[50];
    UINT32 ulOSDTextId;
    UINT32 ulCommonScheduleEventIndex;
    UINT16 uwCliValue;
} sEVENT_LIST_TRANSLATION_TEXT_ID_LUT;	//G100_Coda_0018

	 //G100_Coda_0021 //G100_Coda_0036
typedef struct
{
    UINT8 Time[8];
    UINT8 Func;
    UINT8 Event;
}sLAN_SCHEDULE_DayEvent;

typedef struct
{
    UINT8 Enable[2];
    sLAN_SCHEDULE_DayEvent EventList[16];
}sLAN_WeekDay;

typedef struct
{
    UINT8 ScheduleModeEn[2];
    UINT8 TodaySchedule[16];
    sLAN_WeekDay Weekday[7];
}sLAN_SCHEDULE_PACKET;

typedef struct
{
    u32_t Pid;
    sLAN_SCHEDULE_PACKET Data;
}sLAN_SCHEDULE_StructPayload;

#if 0
typedef struct
{
    UINT8 ucOsdIndex;
    UINT8 ucTimeZoneStr[8];
    UINT8 ucAreaStr[32];
    UINT8 ucDstStr[32];
} sTIME_ZONE_INFO_LUT;
#endif

//#ifdef CUSTOM_BARCO
typedef struct //G100_Steven_0084
{
    UINT8 ucTEC_GATING_CNT;
    UINT8 ucTEC_GATING_CYCLE[FW_BURNIN_CYCLE_DEFAULT_VALUE];

} sTEC_GATING_INFO;
//#endif

typedef union
{
    UINT8 aucTemp[1024]; //G50_Casper_0006
    struct
    {
        UINT32  ulCCT_Tag;
        UINT16  uiCCT_CRC;
        UINT16  uiPWM100Data[eCM_PICTURE_SETTINGS_NUMBER][eLD_SEG_NUMBERS];
        UINT16  uiPWM50Data[eCM_PICTURE_SETTINGS_NUMBER][eLD_SEG_NUMBERS];
        UINT16  uiPWM30Data[eCM_PICTURE_SETTINGS_NUMBER][eLD_SEG_NUMBERS];
        UINT16  uiPWM10Data[eCM_PICTURE_SETTINGS_NUMBER][eLD_SEG_NUMBERS];
    } sDataStruct;
}uCCT_TABLE; //A70LV_Larry_0412


typedef struct
{
    eWARNING_MESSAGE CommandIndex;
    char *GEC_Command;
}sWARNING_MESSAGE_CLI;

typedef struct
{
    eERROR_MESSAGE CommandIndex;
    char *GEC_Command;
}sERROR_MESSAGE_CLI;



#define WARP_OSD_TOTAL_PALETTE (256)
#define WARP_OSD_PALETTE_ELEMENT (3)  //R G B
#define WARP_OSD_BLINK_COLOR_MAX (2)

typedef struct
{
    UINT8 Palette[WARP_OSD_TOTAL_PALETTE][WARP_OSD_PALETTE_ELEMENT];

    BOOL  TransparentColorEnable;
    UINT8 TransparentColor[4];  //palette index for transparent color

    BOOL  ucBlinkEnable;
    UINT8 ucBlinkTime;
    UINT8 ucBlinkCycle;
    UINT8 ucBlinkColorSet0[WARP_OSD_BLINK_COLOR_MAX][WARP_OSD_PALETTE_ELEMENT];  //for palette  F0h ~  FFh
    UINT8 ucBlinkColorSet1[WARP_OSD_BLINK_COLOR_MAX][WARP_OSD_PALETTE_ELEMENT];  //for palette 100h ~ 10Fh
}sWARPOSD_CONFIG_INFO;

//====================== Struct End ====================//


#endif   //_UTILCOMMON_H_

