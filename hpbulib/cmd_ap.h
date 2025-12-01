#ifndef CMD_AP_H
#define CMD_AP_H
//=================================================================================================
#include "type_def.h"
#include "hicc_config.h"
#include "CommonType.h"
#include "SharedMemCtrl.h"
#include "hpbulib_file_path.h"
//=================================================================================================
#define OFFSET_HEADER          0
#define OFFSET_CHECKSUM        2
#define OFFSET_LEN             4
#define OFFSET_TOTAL_NUMBER    6

#define OFFSET_SUB_PACKET      8
#define OFFSET_SUB_HEADER      0
#define OFFSET_SUB_ACTION_TYPE 1
#define OFFSET_SUB_DATA_SIZE   3
#define OFFSET_SUB_DATA        5

#define OFFSET_SUB_DATA_TYPE       0
#define OFFSET_SUB_DATA_STATUS     1
#define OFFSET_SUB_DATA_ID         2
#define OFFSET_SUB_DATA_SUB_SIZE   4
#define OFFSET_SUB_DATA_VALUE      6

#define HEADER_PACKET_1        0x55
#define HEADER_PACKET_V1_2     0x55
#define HEADER_PACKET          0x5555
#define HEADER_SUBPACKET       0xAA
//==================== V2 ========================================================================
#define HEADER_PACKET_V2                 0x5655
#define HEADER_PACKET_V2_2               0x56
#define HEADER_PACKET_VER_V1             0x01
#define HEADER_PACKET_VER_V2             0x02

#define SIZE_PACKET_V2_RESERVE           13

#define OFFSET_PACKET_V2_VERSION         6
#define OFFSET_PACKET_V2_INTERFACE       8
#define OFFSET_PACKET_V2_SAVE_DATA_WAY   10
#define OFFSET_PACKET_V2_EXECT_RESULT    12
#define OFFSET_PACKET_V2_RESERVE         13
#define OFFSET_PACKET_V2_TOTAL_NUMBER    26
#define OFFSET_V2_SUB_PACKET             28

#define SIZE_PACKET_V2                   (4096 + OFFSET_V2_SUB_PACKET)

#define GROUPING_MENU_SEARCH_ITEM        30
#define GROUPING_MENU_SELECT_ITEM        8

#define COMMON_LEN_IP_MIN                7
#define COMMON_LEN_SSID_MAX              20
//=================================================================================================

//CUSTOMIZE EDID FILE PATH
#define CUSTOMIZE_EDID_HDMI1_FILE               "/mnt/configs/scaler/EDID/HDMI1_CustomizeEDID.bin"
#define CUSTOMIZE_EDID_HDMI2_FILE               "/mnt/configs/scaler/EDID/HDMI2_CustomizeEDID.bin"
#define CUSTOMIZE_EDID_HDBASET_FILE             "/mnt/configs/scaler/EDID/HDBaseT_CustomizeEDID.bin"
//=================================================================================================
#define NAME_CMD_INTERFACE_NONE            "None"
#define NAME_CMD_INTERFACE_ISCALER         "iScaler"
#define NAME_CMD_INTERFACE_CLI             "Cli"
#define NAME_CMD_INTERFACE_RS232           "Rs232"
#define NAME_CMD_INTERFACE_IR              "Ir"
#define NAME_CMD_INTERFACE_KEYPAD          "KeyPde"
#define NAME_CMD_INTERFACE_TELNET          "Telnet"
#define NAME_CMD_INTERFACE_EXTRON          "Extron"
#define NAME_CMD_INTERFACE_WEB             "Web"
#define NAME_CMD_INTERFACE_PJLINK          "PJLink"
#define NAME_CMD_INTERFACE_SNMP            "Snmp"
#define NAME_CMD_INTERFACE_AMX             "Amx"
#define NAME_CMD_INTERFACE_ART_NET         "ArtNet"
#define NAME_CMD_INTERFACE_CRESTRON_1      "Crestron"
#define NAME_CMD_INTERFACE_CRESTRON_2      "Crestron"
#define NAME_CMD_INTERFACE_MGR_CAMERA      "mgrCamera"
#define NAME_CMD_INTERFACE_MGR_SERVER      "mgrServce"
#define NAME_CMD_INTERFACE_MGR_CLIENT      "mgrClient"
#define NAME_CMD_INTERFACE_PJSEARCH        "PJSearch"
#define NAME_CMD_INTERFACE_FOTA            "Fota"
#define NAME_CMD_INTERFACE_SMART_SERVICE   "SmartService"
#define NAME_CMD_INTERFACE_MGR_LICENSE     "mgrLicense"
#define NAME_CMD_INTERFACE_IOT_AGENT       "iotAgent"
#define NAME_CMD_INTERFACE_AP_TEST         "ApTest"
#define NAME_CMD_INTERFACE_UI_PROCESS      "iScalerUI"
#define NAME_CMD_INTERFACE_UPGRADE         "Upgrade"
#define NAME_CMD_INTERFACE_SCHEDILING      "Scheduling"
#define NAME_CMD_INTERFACE_NOTIFY          "Notify"

//=================================================================================================
#define LOG_ERROR_LAMP_MODULE               0x02010000
#define LOG_ERROR_LAMP_SENSOR               0x02020000
#define LOG_ERROR_TEMP_AMBIENT              0x03010400
#define LOG_ERROR_TEMP_SYSTEM               0x03020300
#define LOG_ERROR_TEMP_LIGHT_MODULE         0x03030300
#define LOG_ERROR_TEMP_DMD                  0x03060300
#define LOG_ERROR_TEMP_TEC                  0x03070300
#define LOG_ERROR_TEMP_RLD                  0x03070700
#define LOG_ERROR_FAN_CALIBRATION           0x03040500
#define LOG_ERROR_FAN_LOCK                  0x03040100
#define LOG_ERROR_FAN_STALL                 0x03040200
#define LOG_ERROR_COVER_INTERLOCK_SWITCH    0x04030200
#define LOG_ERROR_OTHER_WHEEL               0x04010100
#define LOG_ERROR_OTHER_I2C                 0xFF070200
//=================================================================================================

/*
typedef enum
{
    eDATABASE_TYPE_NA,
    eDATABASE_TYPE_STRING,
    eDATABASE_TYPE_U8,
    eDATABASE_TYPE_U16,
    eDATABASE_TYPE_U32,
    eDATABASE_TYPE_U64,
    eDATABASE_TYPE_INT8,
    eDATABASE_TYPE_INT16,
    eDATABASE_TYPE_INT32,
    eDATABASE_TYPE_INT64,
    eDATABASE_TYPE_FLOAT,
    eDATABASE_TYPE_VOID,

    eDATABASE_TYPE_NUMBERS
}_eDataBaseType;
*/
typedef enum
{
    /* 00 */ CMD_ACTION_SET_DATABASE_ITEM = 0,
    /* 01 */ CMD_ACTION_GET_DATABASE_ITEM,
    /* 02 */ CMD_ACTION_SET_DATABASE_ITEM_ACK_DONE,
    /* 03 */ CMD_ACTION_GET_DATABASE_ITEM_ACK_DONE,

    /* 04 */ CMD_ACTION_SET_INFO_DATA,
    /* 05 */ CMD_ACTION_GET_INFO_DATA,

    /* 06 */ CMD_ACTION_SET_POWER_STATUS,
    /* 07 */ CMD_ACTION_GET_POWER_STATUS,

    /* 08 */ CMD_ACTION_SET_DATABASE_ITEM_UPDATE,
    /* 09 */ CMD_ACTION_GET_DATABASE_ITEM_UPDATE,

    /* 10 */ CMD_ACTION_SET_INFO_DATA_UPDATE,
    /* 11 */ CMD_ACTION_GET_INFO_DATA_UPDATE,

    /* 12 */ CMD_ACTION_SET_KEY_CODE,
    /* 13 */ CMD_ACTION_GET_KEY_CODE,

    /* 14 */ CMD_ACTION_SET_MSSC,    // Telnet
    /* 15 */ CMD_ACTION_GET_MSSC,    // Telnet

    /* 16 */ CMD_ACTION_SET_LOG,
    /* 17 */ CMD_ACTION_GET_LOG,

    /* 18 */ CMD_ACTION_SET_BLENDER,
    /* 19 */ CMD_ACTION_GET_BLENDER,

    /* 20 */ CMD_ACTION_SET_STRUCT,
    /* 21 */ CMD_ACTION_GET_STRUCT,

    CMD_ACTION_NUMBER
}_eActionType;

typedef enum
{
    /* 00 */ CMD_INTERFACE_NONE,
    /* 01 */ CMD_INTERFACE_ISCALER,
    /* 02 */ CMD_INTERFACE_CLI,
    /* 03 */ CMD_INTERFACE_RS232,
    /* 04 */ CMD_INTERFACE_IR,
    /* 05 */ CMD_INTERFACE_KEYPAD,
    /* 06 */ CMD_INTERFACE_TELNET,
    /* 07 */ CMD_INTERFACE_EXTRON,
    /* 08 */ CMD_INTERFACE_WEB,
    /* 09 */ CMD_INTERFACE_PJLINK,
    /* 10 */ CMD_INTERFACE_SNMP,
    /* 11 */ CMD_INTERFACE_AMX,
    /* 12 */ CMD_INTERFACE_ART_NET,
    /* 13 */ CMD_INTERFACE_CRESTRON_V1,
    /* 14 */ CMD_INTERFACE_CRESTRON_V2,
    /* 15 */ CMD_INTERFACE_MGR_CAMERA,
    /* 16 */ CMD_INTERFACE_MGR_SERVER,
    /* 17 */ CMD_INTERFACE_MGR_CLIENT,
    /* 18 */ CMD_INTERFACE_PJSEARCH,
    /* 19 */ CMD_INTERFACE_FOTA,
    /* 20 */ CMD_INTERFACE_SMART_SERVICE,
    /* 21 */ CMD_INTERFACE_MGR_LICENSE,
    /* 22 */ CMD_INTERFACE_IOT_AGENT,
    /* 23 */ CMD_INTERFACE_AP_TEST,
    /* 24 */ CMD_INTERFACE_UI_PROCESS,
    /* 25 */ CMD_INTERFACE_UPGRADE,
    /* 26 */ CMD_INTERFACE_SCHEDULING,
    /* 27 */ CMD_INTERFACE_NOTIFY,
    /* 28 */ CMD_INTERFACE_GROUP,
    /* 29 */ CMD_INTERFACE_LOGREADER,

    CMD_INTERFACE_NUMBER,
}_eInterface;

typedef enum
{
    SYSTEM_POWER_STATUS_RESET = 0,
    SYSTEM_POWER_STATUS_OFF,
    SYSTEM_POWER_STATUS_WARMUP,
    SYSTEM_POWER_STATUS_ON,
    SYSTEM_POWER_STATUS_COOLING,
    SYSTEM_POWER_STATUS_UPGRADE,
    SYSTEM_POWER_STATUS_IDLE,
    SYSTEM_POWER_STATUS_ERROR_SHUTDOWN, //R70G2_RICOH_AC
    SYSTEM_POWER_STATUS_NUMBER,
}_SystemPowerStatus;

typedef enum
{
    DB_STATUS_ENABLE = 0,
    DB_STATUS_DISABLE = 1,

    DB_STATUS_NUMBER
}_eDataBaseStaus;


typedef enum
{
    eEXEC_CODE_FAIL,                         /* general fail indication */
    eEXEC_CODE_PASS,                         /* pass */
    eEXEC_CODE_FATAL,                        /* fatal error; halt application */
    eEXEC_CODE_RTOSERR,                      /* RTOS error */
    eEXEC_CODE_API,                          /* error in call to API function */
    eEXEC_CODE_MUTEX_LOCKED,
    eEXEC_CODE_OVER_RANGE,
    eEXEC_CODE_UNAVALIABLE,                  /* gray out */
	eEXEC_CODE_BIST_BUSY,

    eEXEC_CODE_NUMBER,
} eEXEC_CODE;


typedef enum
{
    //Warp
    /* 00 */ eBLENDER_CLEAR_WARPING = 0,
    /* 01 */ eBLENDER_CONVERT_TP2DTDT,
    /* 02 */ eBLENDER_SETLUT,
    /* 03 */ eBLENDER_GET_HORZ_LUT_INDEX,
    /* 04 */ eBLENDER_SET_HORZ_LUT_INDEX,
    /* 05 */ eBLENDER_GET_VERT_LUT_INDEX,
    /* 06 */ eBLENDER_SET_VERT_LUT_INDEX,

    //Blend
    /* 16 */ eBLENDER_DRAW_EDGE_BLENDING    = 0x10,
    /* 17 */ eBLENDER_CLEAR_EDGE_BLENDING,
    /* 18 */ eBLENDER_SET_BLENDING_TABLE,
    /* 19 */ eBLENDER_CLEAR_BLENDING_TABLE,
    /* 20 */ eBLENDER_SET_BLENDING_GAMMA,

    //Bleck Level
    /* 32 */ eBLENDER_CLEAR_BLACK_LEVEL = 0x20,
    /* 33 */ eBLENDER_SET_BLACK_LEVEL_PALETTE,
    /* 34 */ eBLENDER_ENABLE_BLACK_LEVEL,
    /* 35 */ eBLENDER_SET_BLACK_LEVEL_TABLE,

    //Flash Access
    /* 48 */ eBLENDER_SAVE_WARP_FLASH   = 0x30,
    /* 49 */ eBLENDER_LOAD_WARP_FLASH,
    /* 50 */ eBLENDER_SAVE_BLEND_FLASH,
    /* 51 */ eBLENDER_LOAD_BLEND_FLASH,
    /* 52 */ eBLENDER_SAVE_BLACK_LEVEL_FLASH,
    /* 53 */ eBLENDER_LOAD_BLACK_LEVEL_FLASH,
    /* 54 */ eBLENDER_COLOR_UNIFORMITY_INTERFACE,

    //Warp OSD
    /* 64 */ eBLENDER_SET_OSD_PALETTE   = 0x40,
    /* 65 */ eBLENDER_SET_OSD_PALETTES,
    /* 66 */ eBLENDER_CLEAR_OSD,
    /* 67 */ eBLENDER_DRAW_RECT,
    /* 68 */ eBLENDER_DRAW_RECTS,
    /* 69 */ eBLENDER_DRAW_LINE,
    /* 70 */ eBLENDER_DRAW_LINES,
    /* 71 */ eBLENDER_DRAW_CIRCLE,
    /* 72 */ eBLENDER_DRAW_PNG,
    /* 73 */ eBLENDER_DRAW_SPRITE,
    /* 74 */ eBLENDER_DRAW_SPRITES,
    /* 75 */ eBLENDER_DRAW_COPY_OSD_RECT,
    /* 76 */ eBLENDER_DRAW_CHECKS_ON_OSD,  //full screen
    /* 77 */ eBLENDER_DRAW_STRING,
    /* 78 */ eBLENDER_DRAW_OSD,
    /* 79 */ eBLENDER_DRAW_CHECKBOARD_ON_OSD,
    /* 80 */ eBLENDER_DRAW_BOXES,
  //#ifdef ENABLE_AUTO_COLOR_UNIFORMITY                     //A65_G100_Tim_0013, add, start
    //ACU Test Pattern
    /* 81 */ eBLENDER_TESTPATTERN_OFF,
    /* 82 */ eBLENDER_TESTPATTERN_ACU_WHITE,
    /* 83 */ eBLENDER_TESTPATTERN_ACU_GRID,
    /* 84 */ eBLENDER_TESTPATTERN_ACU_GRID_CENTER,
    /* 85 */ eBLENDER_TESTPATTERN_ACU_BLACK,
  //#endif //ENABLE_AUTO_COLOR_UNIFORMITY                   //A65_G100_Tim_0013, add, start
    /* 86 */ eBLENDER_DRAW_SOLID_COLOR,        //A65_OPTOMA_CL_0002
    /* 87 */ eBLENDER_DRAW_CU_TARGET_SELECT,        //A65_OPTOMA_CL_0002 Test pattern
    /* 88 */ eBLENDER_DRAW_SET_TRANSPARENT_COLOR,        //A65_OPTOMA_CL_0002
    /* 89 */ eBLENDER_DRAW_CIRCLE_GRID,        //A65_OPTOMA_CL_0007
    /* 90 */ eBLENDER_DRAW_IFILL,

    //Other
    /* 112*/ eBLENDER_SCREEN_CAPTURE = 0x70,
    /* 113*/ eBLENDER_REG_DUMP,
    /* 114*/ eBLENDER_FREEZE_IMAGE,

    //Color Uniformity
    /* 128 */ eBLENDER_COLOR_UNIFORMITY_RESET = 0x80,
    /* 129 */ eBLENDER_COLOR_UNIFORMITY_TARGET_SELECT,  //CL add for CU multiple selection

    //Blending startup init
    /* 240 */ eBLENDER_STARTUP_INIT = 0xF0,

    eBLENDER_MAX_NUMBER,
}eBLENDER_CMD_ID;

typedef enum
{
    eSCH_EVENT_TYPE_OFF = 0,
    eSCH_EVENT_TYPE_POWER,
    eSCH_EVENT_TYPE_INPUT_SOURCE,
    eSCH_EVENT_TYPE_LIGHT_SOURCE,
    eSCH_EVENT_TYPE_SHUTTER,
}_eSCH_EVENT_TYPE;

typedef enum
{
    eSCH_EVENT_POWER_ON = 1,
    eSCH_EVENT_POWER_OFF_STBY,
    eSCH_EVENT_POWER_OFF_NSTBY,
    eSCH_EVENT_POWER_OFF_CSTBY,
    eSCH_EVENT_POWER_OFF,
}_eSCH_EVENT_POWER;

typedef enum
{
    eSCH_EVENT_INPUT_VGA = 1,
    eSCH_EVENT_INPUT_HDMI1,
    eSCH_EVENT_INPUT_HDMI2,
    eSCH_EVENT_INPUT_DVI,
    eSCH_EVENT_INPUT_DP,
    eSCH_EVENT_INPUT_3GSDI,
    eSCH_EVENT_INPUT_HDBT,
    eSCH_EVENT_INPUT_12GSDI,
}_eSCH_EVENT_INPUT;

typedef enum
{
    eSCH_EVENT_LIGHT_NORMAL = 1,
    eSCH_EVENT_LIGHT_ECO,
    eSCH_EVENT_LIGHT_QUIET,
    eSCH_EVENT_LIGHT_CUSTOM,
    eSCH_EVENT_LIGHT_CONSTANT_POWER,
    eSCH_EVENT_LIGHT_CONSTANT_INTENSITY,
    eSCH_EVENT_LIGHT_ECO_2,
    eSCH_EVENT_LIGHT_RENTAL,
}_eSCH_EVENT_LIGHT;

typedef enum
{
    NOTIFY_EVENT_STATUS_OFF = 0,
    NOTIFY_EVENT_STATUS_EMAIL,
    NOTIFY_EVENT_STATUS_TRAP,
    NOTIFY_EVENT_STATUS_EMAIL_TRAP,

    NOTIFY_EVENT_STATUS_NUMBER
}_eNOTIFY_EVENT_STATUS;

typedef enum
{
    eSCH_EVENT_SHUTTER_ON = 1,
    eSCH_EVENT_SHUTTER_OFF,
}_eSCH_EVENT_SHUTTER;
//------------------------------------------------------------------------
typedef enum
{
    /* 00 */ eSTRUCT_DATETIME = 0,
    /* 01 */ eSTRUCT_SCHEDULE,
    /* 02 */ eSTRUCT_EMAIL,
    /* 03 */ eSTRUCT_SNMP,
    /* 04 */ eSTRUCT_CCV2,
    /* 05 */ eSTRUCT_XIO,

    eSTRUCT_MAX_NUMBER
}eSTRUCT_CMD_ID;

typedef enum
{
    eBACKUP_RESTORE_RESULT_PASS,                         /* pass */
    eBACKUP_RESTORE_RESULT_FAIL,                         /* general fail indication */
    eBACKUP_RESTORE_RESULT_INDEX_FAIL,                   /* input index fail */
    eBACKUP_RESTORE_RESULT_MAX,

    eBACKUP_RESTORE_RESULT_PROCESSING = 240,
    eBACKUP_RESTORE_RESULT_MAX_NUMBER = 255,
} eBACKUP_RESTORE_RESULT;		//G100_Doulas_0032 Add

typedef enum
{
/* 00 */    eFOTA_SYSTEM_UPDATE_STATUS_IDLE,
/* 01 */    eFOTA_SYSTEM_UPDATE_STATUS_NEW_FW_EXIST,
/* 02 */    eFOTA_SYSTEM_UPDATE_STATUS_NO_NEW_FW,
/* 03 */    eFOTA_SYSTEM_UPDATE_STATUS_QUERY_FW,
/* 04 */    eFOTA_SYSTEM_UPDATE_STATUS_ERROR_NETWORK_FAIL,
/* 05 */    eFOTA_SYSTEM_UPDATE_STATUS_DOWNLOADING,             //ZU860_Owen_0013
/* 06 */    eFOTA_SYSTEM_UPDATE_STATUS_UPGRADE_READY,           //ZU860_Owen_0013
/* 07 */    eFOTA_SYSTEM_UPDATE_STATUS_DOWNLOAD_FAIL,           //ZU860_Owen_0013
/* 08 */    eFOTA_SYSTEM_UPDATE_STATUS_SERVER_NOT_CONNECTED,    //ZU860_Owen_0013
/* 09 */    eFOTA_SYSTEM_UPDATE_STATUS_ISO_PARSING_FAIL,
/* 10 */    eFOTA_SYSTEM_UPDATE_STATUS_POWER_ON_QUERY,
/* 11 */	eFOTA_SYSTEM_UPDATE_STATUS_HTTP_DISABLE_FAIL,

    eFOTA_SYSTEM_UPDATE_STATUS_MAX_NUMBER,
} eFOTA_SYSTEM_UPDATE_STATUS;			//ZU860_Julie_0003

typedef enum
{
	eFOTA_EXECUTE_IDLE,
	eFOTA_EXECUTE_UPGRADE_FW,
	eFOTA_EXECUTE_PARSING_FW,

    eFOTA_EXECUTE_MAX_NUMBER,
} eFOTA_EXECUTE;

typedef enum
{
	CONFIG_FILE_INIT_ALL,
	CONFIG_FILE_INIT_AP_STATUS,   ///mnt/configs/ap_status.conf
	CONFIG_FILE_INIT_CRESTRON,    ///mnt/configs/crestron.conf
	CONFIG_FILE_INIT_PJLINK,      ///mnt/configs/pjlink.conf
	CONFIG_FILE_INIT_NOTIFICATION,///mnt/configs/notifications.conf
	CONFIG_FILE_INIT_SCHEDULE,    ///mnt/configs/schedule.conf
//	CONFIG_FILE_INIT_NETWORK,     ///mnt/configs/net.conf

	CONFIG_FILE_INIT_NUMBER,
} eCONFIG_FILE_INITIAL;

typedef enum
{
    eCES_NONE,
    eCES_EDID_DATA_READY,  //edid data ready from webpage
    eCES_PROCESSING,       //system processing
    eCES_ACTIVE,           //edid write done

    eCES_FILE_ERROR,
    eCES_HEADER_ERROR,
    eCES_CHECKSUM_ERROR,

    eCES_INVALID,

}eCUSTOMIZE_EDID_STATUS;

typedef enum
{
    /* 00 */ eSAVE_DATA_NONE,
    /* 01 */ eSAVE_DATA_TO_RAM_WITH_ACTION,
    /* 02 */ eSAVE_DATA_TO_EMMC_WITH_ACTION,
    /* 03 */ eSAVE_DATA_TO_RAM_NO_ACTION,
    /* 04 */ eSAVE_DATA_TO_EMMC_NO_ACTION,

    eSAVE_DATA_NUMBER,
}eSAVE_DATA_WAY;

typedef enum
{
    eLAN_DATA_PASS = 0,

    eLAN_DATA_IP_OUT_OF_RANGE = 11,
    eLAN_DATA_IP_CLASS_FAIL = 12,

    eLAN_DATA_SUBMASK_OUT_OF_RANGE = 21,
    eLAN_DATA_SUBMASK_BIT_FAIL = 22,
    eLAN_DATA_EMPTY_SUBMASK = 23,

    eLAN_DATA_GATEWAY_OUT_OF_RANGE = 31,
    eLAN_DATA_GATEWAY_NOT_MATCH_IP = 32,
    eLAN_DATA_EMPTY_GATEWAY = 33,

    eLAN_DATA_EMPTY_SUBMASK_AND_GATEWAY = 41,
}eLAN_DATA_STATUS;

typedef enum
{
    eWLAN_DATA_PASS = 0,
    eWLAN_DATA_INPUT_ERROR,

    eWLAN_DATA_SSID_OUT_OF_RANGE = 11,

    eWLAN_DATA_START_IP_OUT_OF_RANGE = 21,
    eWLAN_DATA_START_IP_CLASS_FAIL,
    eWLAN_DATA_END_IP_OUT_OF_RANGE,
    eWLAN_DATA_END_IP_CLASS_FAIL,
    eWLAN_DATA_START_END_IP_OUT_OF_RANGE,
    eWLAN_DATA_CHECK_IP_FAIL,

    eWLAN_DATA_NETMASK_OUT_OF_RANGE = 31,
    eWLAN_DATA_NETMASK_BIT_FAIL,
    eWLAN_DATA_EMPTY_SUBMASK,

    eWLAN_DATA_GATEWAY_OUT_OF_RANGE = 41,
    eWLAN_DATA_GATEWAY_NOT_MATCH_IP,
    eWLAN_DATA_EMPTY_GATEWAY,

    eWLAN_DATA_EMPTY_SUBMASK_AND_GATEWAY = 51,

}eWLAN_DATA_STATUS;

typedef enum
{
    eLAN_IPV6_DATA_PASS = 0,

    eLAN_IPV6_DATA_IP_OUT_OF_RANGE = 11,
    eLAN_IPV6_DATA_IP_CLASS_FAIL = 12,

    eLAN_IPV6_DATA_PREFIX_OUT_OF_RANGE = 21,
    eLAN_IPV6_DATA_PREFIX_BIT_FAIL = 22,
    eLAN_IPV6_DATA_EMPTY_SUBMASK = 23,

    eLAN_IPV6_DATA_GATEWAY_OUT_OF_RANGE = 31,
    eLAN_IPV6_DATA_GATEWAY_NOT_MATCH_IP = 32,
    eLAN_IPV6_DATA_EMPTY_GATEWAY = 33,

    eLAN_IPV6_DATA_EMPTY_SUBMASK_AND_GATEWAY = 41,
}eLAN_IPV6_DATA_STATUS;

typedef enum
{
    GEC_ERROR_TYPE_NONE = 0,
    GEC_ERROR_TYPE_FAN,
    GEC_ERROR_TYPE_LAMP,
    GEC_ERROR_TYPE_TEMP,
    GEC_ERROR_TYPE_COVER,
    GEC_ERROR_TYPE_FILTER,
    GEC_ERROR_TYPE_OTHER,
    GEC_ERROR_TYPE_MAX
}eGEC_ERROR_TYPE;
//=================================================================================================
#pragma pack(push)
#pragma pack(1)

//Blender Data Format Start
typedef struct
{
    UINT16 Width;
    UINT16 Height;
    char FileName_TPx[128];
    char FileName_TPy[128];
} sTP_DATA;

typedef struct
{
    UINT8 ucEnable;
    UINT16 uiOverlap;
    UINT16 uiStart;
} sEDGE_BLENDING, *psEDGE_BLENDING;

typedef struct
{
    sEDGE_BLENDING sTop;
    sEDGE_BLENDING sBottom;
    sEDGE_BLENDING sLeft;
    sEDGE_BLENDING sRight;
} sEDGE_BLENDING_INFO, *psEDGE_BLENDING_INFO;

typedef struct
{
    UINT16 Width;
    UINT16 Height;
    char FileName[128];
} sBLENDING_TABLE_INFO;


//Black Level
typedef struct
{
    UINT8 ucPaletteIndex;
    UINT8 ucRed;
    UINT8 ucGreen;
    UINT8 ucBlue;
} sBLACKLEVEL_PALETTE;

typedef struct
{
    char IP[COMMON_ARRAYSIZE_32];
    char ReadCommunity[COMMON_ARRAYSIZE_128];
    char Location[COMMON_ARRAYSIZE_128];
}_sSnmp;

typedef struct
{
    UINT16 Width;
    UINT16 Height;
    char FileName[128];
} sBLACKLEVEL_TABLE_INFO;

//Color Uniformoty
typedef struct
{
    UINT8 ucWriteEnable;
    char FileName[128];
} sCOLOR_UNIFORMITY_INFO;
//CL add for CU multiple selection
typedef struct
{
    UINT8 ucShow;
    UINT8 ucPoint[63];
} sCOLOR_UNIFORMITY_POINT_SELECT;

typedef struct
{
    UINT8 ucPointX;
    UINT8 ucPointY;
}sCOLOR_UNIFORMITY_POINT_SET;

typedef struct
{
    UINT8 ucPaletteIndex;
    UINT8 ucRed;
    UINT8 ucGreen;
    UINT8 ucBlue;
} sWARPOSD_PALETTE_DATA;

typedef struct
{
    UINT8 ucRed[256];
    UINT8 ucGreen[256];
    UINT8 ucBlue[256];
} sWARPOSD_PALETTE_DATA_ALL;

typedef struct
{
    UINT8 ucLayerMode; //0:Before warp, 1:After Warp
    INT16 iX;
    INT16 iY;
    INT16 iWidth;
    INT16 iHeight;
    INT16 iR;
    INT16 iG;
    INT16 iB;
} sWARPOSD_BOX_DRAW_INFO;

typedef struct
{
    UINT16 ucTotalNumber;
    sWARPOSD_BOX_DRAW_INFO sBox;
} sWARPOSD_BOXES_DRAW_INFO;

typedef struct
{
    UINT8 ucLayerMode; //0:Before warp, 1:After Warp
    INT16 iX;
    INT16 iY;
    INT16 iWidth;
    INT16 iHeight;
    INT16 iColorIndex;
} sWARPOSD_RECT_DRAW_INFO;

typedef struct
{
    UINT16 ucTotalNumber;
    sWARPOSD_RECT_DRAW_INFO sRect;
} sWARPOSD_RECTS_DRAW_INFO;

typedef struct
{
    UINT8 ucLayerMode; //0:Before warp, 1:After Warp
    INT16 iLeft;
    INT16 iTop;
    INT16 iRight;
    INT16 iBottom;
    INT32 lColorIndex;
} sWARPOSD_LINE_DRAW_INFO;

typedef struct
{
    UINT16 ucTotalNumber;
    sWARPOSD_LINE_DRAW_INFO sLine;
} sWARPOSD_LINES_DRAW_INFO;

typedef struct
{
    UINT8 ucLayerMode; //0:Before warp, 1:After Warp
    INT16 iX;
    INT16 iY;
    INT16 iRadius;
    INT16 iColorIndex;
} sWARPOSD_CIRCLE_DRAW_INFO;

typedef struct
{
    UINT8 ucLayerMode; //0:Before warp, 1:After Warp
    UINT8 color_red;
    UINT8 color_green;
    UINT8 color_blue;

    INT16 x_start;
    INT16 x_end;
    INT16 x_offset;
    INT16 x_step;

    INT16 y_start;
    INT16 y_end;
    INT16 y_offset;
    INT16 y_step;

    INT16 radius;
    INT16 space;
    BOOL bShowLast;
    BOOL bClearOSD;
}sWARPOSD_CIRCLE_GRID_DRAW_INFO;  //A65_OPTOMA_CL_0007

typedef struct
{
    UINT8 ucLayerMode; //0:Before warp, 1:After Warp
    INT16 iX;
    INT16 iY;
    INT16 iWidth;
    INT16 iHeight;
    char FileName[128];
} sWARPOSD_PNG_DRAW_INFO;

typedef struct
{
    UINT8 ucLayerMode; //0:Before warp, 1:After Warp
    INT16 iSourceX;
    INT16 iSourceY;
    INT16 iSourceWidth;
    INT16 iSourceHeight;
    INT16 iDestinationX;
    INT16 iDestinationY;
} sWARPOSD_COPY_OSD_RECT_INFO;

typedef struct
{
    UINT8 ucLayerMode; //0:Before warp, 1:After Warp
    INT16 iBlockSize;
} sWARPOSD_CHECKBOARD_DRAW_INFO;

typedef struct
{
    UINT8 ucLayerMode; //0:Before warp, 1:After Warp
    INT16 iSizeX;
    INT16 iSizeY;
    INT16 iShiftX;
    INT16 iShiftY;
} sWARPOSD_CHECKBOARDS_DRAW_INFO;

typedef struct
{
    UINT8 ucLayerMode; //0:Before warp, 1:After Warp
    INT16 nPid;
    INT16 nTop;
    INT16 nLeft;
    char FileName[128];
} sSPRITE_DATA;

typedef struct
{
    UINT8 ucLayerMode; //0:Before warp, 1:After Warp
    UINT16 ucTotalNumber;
    char FileName[128];
} sSPRITES_DATA;


typedef struct
{
    INT32 nPID;
    INT32 nWidth;
    INT32 nHeight;

    unsigned char ucPixData[8192 * 10];
    unsigned char ucPixIndexed[4096 * 10];

    unsigned char ucPixDataR[4096 * 10];
    unsigned char ucPixDataG[4096 * 10];
    unsigned char ucPixDataB[4096 * 10];
    unsigned char ucPixDataA[4096 * 10];
} sPixmap;

typedef union
{
    UINT8 ucData[122892];  // 4 + 4 + 4 + 4096 * 10 + 4096 * 10 + 4096 * 10

    struct
    {
        INT32 nPID;
        INT32 nWidth;
        INT32 nHeight;

        unsigned char ucPixData[8192 * 10];
    } sPixmap_OnOSD;

    struct
    {
        INT32 nPID;
        INT32 nWidth;
        INT32 nHeight;

        unsigned char ucPixDataR[4096 * 10];
        unsigned char ucPixDataG[4096 * 10];
        unsigned char ucPixDataB[4096 * 10];
    } sPixmap_OnMemory;

} sPixmapData;



typedef struct
{
    UINT8 ucLayerMode; //0:Before warp, 1:After Warp
    INT16 nPid;
    INT16 nTop;
    INT16 nLeft;
    sPixmapData sData;
} sSPRITE_INFO_DATA;

typedef struct
{
    INT16 iTotalNumber;
    UINT8 ucBlock;  //data in shared memory block 0 or 1
} sSPRITES_INFO_DATA;

typedef struct
{
    UINT8 ucRed;
    UINT8 ucGreen;
    UINT8 ucBlue;
} sSOLID_COLOR_INFO;

typedef enum
{
    eDATA_SYNC_STATUS_IDLE,

    eDATA_SYNC_STATUS_BLOCK0_SENDER_DONE,
    eDATA_SYNC_STATUS_BLOCK0_RECEIVER_DONE,
    eDATA_SYNC_STATUS_BLOCK0_RECEIVER_FAIL,

    eDATA_SYNC_STATUS_BLOCK1_SENDER_DONE,
    eDATA_SYNC_STATUS_BLOCK1_RECEIVER_DONE,
    eDATA_SYNC_STATUS_BLOCK1_RECEIVER_FAIL,

    eDATA_SYNC_STATUS_INVALID

}eDATA_SYNC_STATUS;

//Blender Data Format End


typedef struct
{
    UINT16 Size;        // == sizeof(sUI_EVENT_FORMAT)
    UINT16 SendFrom;    //datacode or other  (not necessarily required)
    UINT32 Param;       //Extra Parameter    (not necessarily required)
}sUI_EVENT_FORMAT;

/***************************************** File Info *********************************************/

//Filename : Warp1,            Warp2,            Warp3,            Warp4,            Warp5
//Filename : Blend1,           Blend2,           Blend3,           Blend4,           Blend5
//Filename : BlackLevel1,      BlackLevel2,      BlackLevel3,      BlackLevel4,      BlackLevel5
//Filename : ColorUniformity

typedef struct
{
    UINT8 ucEnableFlag;
    UINT8 uc3DFlag;
    UINT16 ucWidth;
    UINT16 ucHeight;
    FLOAT afTable_TpX[121*76];
    FLOAT afTable_TpY[121*76];
    UINT8 aucLutTable[100];
}sWarpFileInfo;

typedef struct
{
    UINT8 ucEnableFlag;
    UINT8 uc3DFlag;
    UINT16 ucWidth;
    UINT16 ucHeight;
    UINT8 aucTable[3840*2160];
    UINT8 aucEdgeBlend[62];
    UINT8 aucEGBCT[6];
}sBlendFileInfo;

typedef struct
{
    UINT8 ucEnableFlag;
    UINT8 uc3DFlag;
    UINT16 ucWidth;
    UINT16 ucHeight;
    UINT8 aucTable[3840*2160];
    UINT8 aucBiasPalette[48];
    UINT8 aucEGBIASCT[6];
}sBlackLevelFileInfo;

typedef struct
{
    UINT8 ucEnableFlag;
    UINT8 uc3DFlag;
    UINT16 ucWidth;
    UINT16 ucHeight;
    UINT8 aucTable_TableTop[1900];
    UINT8 aucTable_CeilingMount[1900];
    UINT8 aucTable_RearProjection[1900];
    UINT8 aucTable_RearCeiling[1900];
}sColorUniformityFileInfo;


#define HICC2_GEO_FILE_HEADER ((UINT16)0x5502)

#define MAX_WARP_WIDTH  (129)
#define MAX_WARP_HEIGHT (76)
typedef struct
{
    UINT16 Header;
    UINT8 ucEnableFlag;
    UINT8 uc3DFlag;
    UINT16 ucWidth;
    UINT16 ucHeight;
    FLOAT afTable_TpX[MAX_WARP_WIDTH*MAX_WARP_HEIGHT];
    FLOAT afTable_TpY[MAX_WARP_WIDTH*MAX_WARP_HEIGHT];
    UINT8 HorzFilterIdx;
    UINT8 VertFilterIdx;
    UINT8 aucLutTable[100];
    UINT16 uiChecksum;
}sWarpFileInfo_H2;

#define MAX_DBD_BLEND_WIDTH  (4096)
#define MAX_DBD_BLEND_HEIGHT (2400)
typedef struct
{
    UINT16 Header;
    UINT8 ucEnableFlag;
    UINT8 uc3DFlag;
    UINT16 ucWidth;
    UINT16 ucHeight;
    UINT8 aucTable[MAX_DBD_BLEND_WIDTH * MAX_DBD_BLEND_HEIGHT];
    UINT8 aucEdgeBlend[62];
    UINT8 aucEGBCT[6];
    UINT16 uiChecksum;
}sBlendFileInfo_H2;


#define MAX_BLACKLEVEL_WIDTH  (4096)
#define MAX_BLACKLEVEL_HEIGHT (2400)
typedef struct
{
    UINT16 Header;
    UINT8 ucEnableFlag;
    UINT8 uc3DFlag;
    UINT16 ucWidth;
    UINT16 ucHeight;
    UINT8 aucTable[MAX_BLACKLEVEL_WIDTH * MAX_BLACKLEVEL_HEIGHT];
    UINT8 aucBiasPalette[48];
    UINT8 aucEGBIASCT[6];
    UINT16 uiChecksum;
}sBlackLevelFileInfo_H2;


typedef struct
{
    UINT16 Header;
    UINT8 ucEnableFlag;
    UINT8 uc3DFlag;
    UINT16 ucWidth;
    UINT16 ucHeight;
    UINT8 aucTable_TableTop[1900];
    UINT8 aucTable_CeilingMount[1900];
    UINT8 aucTable_RearProjection[1900];
    UINT8 aucTable_RearCeiling[1900];
    UINT16 uiChecksum;
}sColorUniformityFileInfo_H2;


/*
typedef struct
{
    u8_t Type;
    u8_t Status;
    u16_t Id;
    u8_t Data[2048];
}_sDataBaseFormat;
*/

typedef struct
{
    u8_t  Header;
    u16_t ActionType;
    u16_t Size;
    sDATABASE_ITEM_DATA_FORMAT Data;
}_sSubPacketFormat;

typedef struct
{
    u16_t Header;
    u16_t CheckSum;
    u16_t Size;
    u16_t Number;
    u8_t  Value[4096];
}_sPacketFormat;

typedef struct
{
    u16_t Header;
    u16_t CheckSum;
    u16_t Size;
    u16_t Version;
    u16_t Interface;
    u16_t SaveDataWay;
    u8_t ExecResult;
    u8_t Reserve[SIZE_PACKET_V2_RESERVE];
    u16_t Number;
    u8_t  Value[4096];
}_sPacketFormatV2;

typedef struct SubPacketConcat
{
    _eActionType ActionType;
    u16_t Size;
    sDATABASE_ITEM_DATA_FORMAT  Database;
    struct SubPacketConcat *pNextSubPacket;
}_sSubPacketConcat;

typedef struct
{
    s8_t Mode[COMMON_ARRAYSIZE_8];
    s8_t NTP[COMMON_ARRAYSIZE_64];
    s8_t Timezone[COMMON_ARRAYSIZE_32];
    s8_t Interval[COMMON_ARRAYSIZE_16];
    s8_t Year[COMMON_ARRAYSIZE_8];
    s8_t Month[COMMON_ARRAYSIZE_4];
    s8_t Day[COMMON_ARRAYSIZE_4];
    s8_t Hour[COMMON_ARRAYSIZE_4];
    s8_t Minute[COMMON_ARRAYSIZE_4];
    s8_t Second[COMMON_ARRAYSIZE_4];
    s8_t DST[COMMON_ARRAYSIZE_4];
    s8_t Dstzone[COMMON_ARRAYSIZE_32];
}_sDateTime;
//------------------------------------------------------------------------
typedef struct
{
    s8_t Time[COMMON_ARRAYSIZE_8];
    u8_t Func;
    u8_t Event;
}_DayEvent;

typedef struct
{
    s8_t Enable[2];
    _DayEvent EventList[COMMON_ARRAYSIZE_16];
}_WeekDay;

typedef struct
{
    s8_t ScheduleModeEn[2];
    s8_t TodaySchedule[COMMON_ARRAYSIZE_16];
    _WeekDay Weekday[7];
}_sSchedule;
//------------------------------------------------------------------------
typedef struct
{
    u32_t SmtpEn;
    char  SmtpService[COMMON_ARRAYSIZE_64];
    u32_t SmtpPort;
    u32_t SmtpTls;
    u32_t SmtpStarttls;
    char  SmtpAuthMode[COMMON_ARRAYSIZE_64];
    char  SmtpUsername[COMMON_ARRAYSIZE_64];
    char  SmtpPassword[COMMON_ARRAYSIZE_64];
    char  SendMail[COMMON_ARRAYSIZE_64];
    char  Recipient1Email[COMMON_ARRAYSIZE_64];
    char  Recipient2Email[COMMON_ARRAYSIZE_64];
}_sEmail;
//------------------------------------------------------------------------
typedef struct
{
    u32_t Interface;
    u32_t SecureEn;
    char Address[COMMON_ARRAYSIZE_64];
    u32_t IpId;
    u32_t Port;
    u32_t RoomViewIpId;
    char Vc4UserName[COMMON_ARRAYSIZE_64];
    char Vc4UserPassword[COMMON_ARRAYSIZE_64];
    char Vc4RoomId[COMMON_ARRAYSIZE_64];
    char FusionUrl[COMMON_ARRAYSIZE_512];
    char AssignName[COMMON_ARRAYSIZE_64];
    char RoomName[COMMON_ARRAYSIZE_64];
    char Location[COMMON_ARRAYSIZE_64];
    u32_t FusionPort;
    u32_t AutoDiscovery;
    u32_t CertVerify;
}_sCcv2;
//------------------------------------------------------------------------
typedef struct
{
    char Url[COMMON_ARRAYSIZE_512];
    char ReportedModel[COMMON_ARRAYSIZE_512];
}_sXioCloud;
//------------------------------------------------------------------------
typedef union
{
    _sDateTime DateTime;
    _sSchedule Schedule;
    _sEmail    Email;
    _sSnmp     Snmp;
    _sCcv2     Ccv2;
    _sXioCloud XioCloud;
}_sData;

typedef struct
{
    u32_t Pid;
    _sData Data;
}_sStructPayload;



//----------------------------------- Datacode Structure Data ----------------------------------------------------

//for Datacode edcPIPPBP_MAIN_PANEL_INFO
//for Datacode edcPIPPBP_SUB_PANEL_INFO
typedef struct
{
    UINT16 HActive;
    UINT16 VActive;
    UINT16 HStart;
    UINT16 VStart;
} sDST_PIPPBP_PANEL_INFO;

//for Datacode edcCUR_LENS_INFO
typedef struct
{
    sLENS_INFO sLensInfo;
} sDST_LENS_INFO;

//for Datacode edcMOTOR_LENS_HEADER_INFO
typedef struct
{
    sLENS_HEADER sLensHeaderInfo;
} sDST_LENS_HEADER_INFO;

//for Datacode edcCUR_SOURCE_AVI_INFO
typedef struct
{
    sSOURCE_AVI_INFO sAVIInfo;
} sDST_SOURCE_AVI_INFO;

//only for test
typedef struct
{
    UINT8 Data[1024];
} sDST_DATA2K;

//for Datacode edcPOWER_STATUS
typedef struct
{
    sPOWER_STATUS_INFO  sPowerStatus;
} sDST_POWER_STATE;

//for edcLIGHTSENSOR_TIME
typedef struct
{
    UINT8 LightSensorTime[4];
} sDST_LIGHT_SENSOR_TIME;

//for edcLIGHTSENSOR_GAIN
typedef struct
{
    FLOAT LightSensorGain[4];
} sDST_LIGHT_SENSOR_GAIN;

//for edcLIGHTSENSOR_OFFSET
typedef struct
{
    UINT16 LightSensorOffset[4];
} sDST_LIGHT_SENSOR_OFFSET;

//for edcLIGHTSENSOR_TARGET_FACTORY
typedef struct
{
    UINT16 LightSensorTarget[4];
} sDST_LIGHT_SENSOR_TARGET;

//for edcLIGHTSENSOR_TARGET_RLD
typedef struct
{
    UINT16 LightSensorTargetRLD[4];
} sDST_LIGHT_SENSOR_TARGET_RLD;

//for edcLIGHTSENSOR_INFO_SECOND //HICC2_Jacky_0003
typedef struct
{
    UINT16 LightSensorInfo[4];
} sDST_LIGHT_SENSOR_INFO;

//for edcUI_SCHEDULE_EVENT_INFO
typedef struct
{
    UINT8 ucCurrentEventListNum;
    UINT8 ucCurrentCopyWeekdayNum;
} sDST_SCHEDULE_EVENT_INFO;

//for SCHEDULE_DUPLICATE_INFO
typedef struct
{
    UINT16 uiDuplicate_TimeMin;
    UINT8 ucDuplicate_FuncType;
    UINT8 ucDuplicate_EventIdx;
} sDST_SCHEDULE_DUPLICATE_INFO;

//for edcACTION_LOG
typedef struct
{
    UINT32 ulTime;
    UINT32 ulEvent;
} sDST_ACTION_LOG;

//for edcAW_WARP_POINT_POSITION
typedef struct
{
    INT32 PosX;
    INT32 PosY;
} sDST_AW_WARP_POINT_POSITION;

typedef struct
{
    UINT16 FanControlInfo[16];
    UINT16 FanPWM[40];
    UINT16 FanRPM[40];
    UINT16 LDTemp[32];
    UINT16 LDVoltage[32];
    UINT16 LDCurrent[32];
    UINT16 PumpRPM[4];
    UINT16 TEC_PWM[8];
    UINT16 TEC_Voltage[8];
    UINT16 TEC_Current[8];
    UINT16 LightSensor[8];
    UINT16 FanSystemTemp;
    UINT16 FanDMDTemp1;
    UINT16 FanEnviTemp;
    UINT16 FanDMDTemp2;
    UINT16 FanPressure;
    UINT16 FanHumidity1;
    UINT16 FanHumidity2;
    UINT16 FanABSHumidity;
    UINT16 WheelSpeed[4];
} sDST_FAN_CONTROL;

typedef struct
{
    INT32  lInputVoltage;
    INT32  lPFC_OutputVoltage;
    INT32  l5V_Voltage;
    INT32  l12V_Voltage;
    INT32  l52V_Voltage;
    INT32  l5V_Current;
    INT32  l12V_Current;
    INT32  l52V_Current;

    INT32  lPFC_MOS_Temperature;
    INT32  lBridge_Temperature;
    INT32  lLVPS_Ambient_Temperature;
    INT32  l12V_SR_MOSFET_Temperature;
    INT32  l52V_SR_MOSFET_Temperature;

    UINT16 uiPRIMARY_PROTECTION_STATUS;
    UINT16 uiSECONDARY_PROTECTION_STATUS;
    UINT16 uiIO_STATUS;
} sDST_LVPS_INFO;

#define AW_WARP_H_POINT_NUM 33
#define AW_WARP_V_POINT_NUM 33

typedef struct
{
    WebPoint2f ControlPoint[AW_WARP_H_POINT_NUM][AW_WARP_V_POINT_NUM];
    u32_t GridNum;
    u32_t InnerEn;
    u32_t Sharpness;
    u32_t GridColor;
    u32_t GridBackground;
    char HeaderStr[32];
} sDST_AW_WARP_POINT_ALL_POSITION;

//----------------------------------- HICC V2  ----------------------------------------------------
typedef struct
{
    u16_t Size;
    u16_t Version;
    u16_t Interface;
    u16_t SaveDataWay;
    u8_t  ExecResult;
    u16_t TotalNum;
}_sPacketV2Info;

typedef struct
{
    u16_t ActionType;
    u16_t Size;       // Data base size
}_sSubPacketInfo;

typedef struct
{
    u8_t  Type;
    u16_t Status;
    u16_t Id;
    u16_t Size;
}_sDataBaseInfo;

//-----------------------------------GROUP DATA-------------------------------------
typedef struct
{
    u32_t ProjectorStatus;
    u32_t GroupStatus;
    u32_t GroupId;
    u32_t GroupVersion;
    char SN[COMMON_ARRAYSIZE_32];
    char ModeName[COMMON_ARRAYSIZE_32];
    char ProjectorName[COMMON_ARRAYSIZE_32];
    char GroupName[COMMON_ARRAYSIZE_32];
    char Ip[COMMON_ARRAYSIZE_32];
    char Mac[COMMON_ARRAYSIZE_32];
    s32_t Fd;
    u32_t CameraStatus;
    char Note[COMMON_ARRAYSIZE_64];
}_sProjectorInfo;

typedef struct
{
    _sProjectorInfo Info[MAX_PROJECTOR_GROUP_SEARCH_NUM];
    u32_t Cnt;
}_sGroupSearchInfo;

typedef struct
{
    _sProjectorInfo Info[MAX_PROJECTOR_GROUP_SELECT_NUM];
    u32_t master;
    u32_t Cnt;
}_sGroupSelectInfo;

typedef struct
{
    UINT8 ucEnable;
    UINT8 ucCameraStatus;
    UINT8 ucGroupStatus;
    UINT16 uiFd;
    UINT16 uiGroupId;
    char  cName[32];
    char  cIP[20];
}sGROUPING_INFO;

typedef struct
{
    UINT8 ucCnt;
    UINT8 ucGroupingStatus;
    UINT8 ucMenuUpdate;
    sGROUPING_INFO sItem[GROUPING_MENU_SEARCH_ITEM];
}sGROUPING_SEARCH_INFO;

typedef struct
{
    UINT8 ucCnt;
    sGROUPING_INFO sItem[GROUPING_MENU_SELECT_ITEM];
}sGROUPING_SELECT_INFO;

typedef struct
{
    INT32 clientRet;
    INT32 functionRet;
    char failProjectorIp[COMMON_ARRAYSIZE_128];
}_sGROUPING_CLIENT_RESULT;

//-----------------------------------GROUP DATA END-------------------------------------
//#pragma pack()
#pragma pack(pop)

//=================================================================================================
u8_t CmdAp_initPacket(_sPacketFormat *pPacket);
void CmdAp_initPacketV2Info(_sPacketV2Info *pInfo, u16_t Ver, u16_t Interface, u16_t SaveDataWay, u8_t ExecResult);
u8_t CmdAp_initPacketV2(u8_t *pPacket, _sPacketV2Info *pInfo);
u8_t CmdAp_initSubPacket(_sSubPacketFormat *pSubPacket);
u8_t CmdAp_analyzePacket(u16_t *pPacketSize, u16_t *pNumber, u8_t **pPayload, u8_t *pData);
u8_t CmdAp_analyzePacketV2(_sPacketV2Info *pInfo, u8_t **pPayload,  u8_t *pData);
u8_t CmdAp_addSubPacketInfo(_sSubPacketFormat *pSubPacket, u16_t Size, u16_t ActType);
u8_t CmdAp_addPacketInfo(_sPacketFormat *pPacket, u16_t Size);
u8_t CmdAp_addPacketCheckSum(_sPacketFormat *pPacket);
u8_t CmdAp_addPacketV2CheckSum(u8_t *pPacket);
u8_t CmdAp_addPacketV2ExecResult(u8_t *pPacket, u8_t Status);
u8_t CmdAp_addPacketIdNumber(_sPacketFormat *pPacket, u16_t IdNumber);
u8_t CmdAp_addPacketSize(_sPacketFormat *pPacket, u16_t Size);
u8_t CmdAp_buildDatabase(sDATABASE_ITEM_DATA_FORMAT *pDatabase, _eDataBaseStaus Status, u8_t Type, u16_t Id, u8_t *pData);
u8_t CmdAp_buildBlender(sDATABASE_ITEM_DATA_FORMAT *pDatabase, _eDataBaseStaus Status, u8_t Type, u16_t Id, u8_t *pData, u16_t DataSize);
u8_t CmdAp_buildSendData(_sPacketFormat *pPacket, _eActionType ActionType, u16_t Size, u16_t Id, u8_t Status, u8_t *pData, u8_t Type);
u8_t CmdAp_buildV2SendData(u8_t *pPacket, _sSubPacketInfo *pSubPacketInfo, _sDataBaseInfo *pDbInfo, u8_t *pData);
u8_t CmdAp_buildV2SendSingleData(u8_t *pPacket, _sPacketV2Info *pPacketInfo, _sSubPacketInfo *pSubPackInfo, _sDataBaseInfo *pDbInfo, u8_t *pData);
u8_t CmdAp_buildSendSingleData(_sPacketFormat *pPacket, _eActionType ActionType, u16_t Size, u16_t Id, u8_t Status, u8_t *pData, u8_t Type);
u8_t CmdAp_buildSubPacketConcat(_sSubPacketConcat *pNow, _sSubPacketConcat *pNext, _eActionType ActionType, u8_t DbType, u8_t Status, u16_t Id, u8_t *pValue, u16_t ValueSize);
void CmdAp_buildSubPacketInfo(_sSubPacketInfo *pInfo, _sSubPacketFormat *pFormat);
u16_t CmdAp_getDbSize(sDATABASE_ITEM_DATA_FORMAT *pDbInfo);
u16_t CmdAp_getPacketV2Size(u8_t *pData);
u16_t CmdAp_getProtocolVersion(u8_t *pData);
u8_t CmdAp_getSubPacketData(sDATABASE_ITEM_DATA_FORMAT **pTrigData, u8_t *pData, u16_t Number);
u8_t CmdAp_getSubPacket(_sSubPacketFormat **pSubPacket, u8_t *pData, u16_t NumberIdx);
u8_t CmdAp_getData(u16_t *pActType, sDATABASE_ITEM_DATA_FORMAT *pSubPacketData, u16_t *pSize, u8_t *pData);
u8_t CmdAp_parseSubPacketConcat(_sPacketFormat *pPacket, char *ActStr ,_sSubPacketConcat *pData);
u8_t CmdAp_parseSubPacketV2Concat(u8_t *pPacket, char *ActStr ,_sSubPacketConcat *pData, _sPacketV2Info *pPacketInfo);
void CmdAp_shoeDebugData(char *pTitle, u8_t *pData, u32_t Size);
u8_t CmdAp_calcCheckSum(u16_t *pCheckSum, u8_t *pData, u32_t Size);
u32_t CmdAp_checkVersionDataCode(u32_t *pMinStrLen, u16_t Id);
u8_t CmdAp_checkIpInfo(char *pIpAddr, char *pSubnetMask, char *pGateway);
u8_t CmdAp_checkIpInfoNoStrict(char *pIpAddr, char *pSubnetMask, char *pGateway);
u8_t CmdAp_checkIpInfoOptoma(char *pIpAddr, char *pSubnetMask, char *pGateway);
u8_t CmdAp_lanSuggestedApply(u8_t faildEvent, char *pIpAddr, char *pSubnetMask, char *pGateway);
u8_t CmdAp_checkWlanInfo(char* pSsid, char *pPasswd, char *pStartIpAddr, char *pEndIpAddr,char *pSubnetMask, char *pGateway);
u8_t CmdAp_wlanSuggestedApply(u8_t faildEvent, char *pIpAddr, char *pSubnetMask, char *pGateway);
u8_t CmdAp_parseGecErrorType(u32_t Log);
u8_t CmdAp_checkIpv6Info(char *pIpAddr, u32_t Prefix, char *pGateway);
u8_t CmdAp_lanIpv6SuggestedApply(u8_t faildEvent, char *pIpAddr, u32_t *pPrefix, char *pGateway);
//=================================================================================================
#endif //CMD_AP_H
