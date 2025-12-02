//------------------------------------------------------------------------------
// Coretronic Confidential Strictly Private
//
// This file is used to define the internal command for Scaler
//
// 2015/05/13, Tim Chen
//------------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include "Telnet.h"

#define CMD_QUERY_LABEL        "?"
#define CMD_QUERY_LABEL_CHAR        '?'
#define CMD_HEAD_LEN                    1
#define CMD_Response_LABEL_CHAR        "!"
#define CMD_Response_LABEL             '!'//B35LB_David_0021 Add
#define INTERNALTELNET_CORE_CMD_START             '[' //B35LB_Kraz_0123, mod : start //B35LB_David_0009 Add Start
#define INTERNALTELNET_CORE_CMD_START_STR         "[" // Steven , Modify forB35LB
#define INTERNALTELNET_CORE_CMD_END               ']'
#define INTERNALTELNET_CORE_CMD_END_STR           "]" //B35LB_Kraz_0123, mod : end
#define INTERNAL_CMD_START              '<'                 //0x3C
#define INTERNAL_CMD_END                '>'                 //0x3E
#define INTERNAL_CMD_START_STR          "<"
#define INTERNAL_CMD_END_STR            ">"//B35LB_David_0009 Add Start
#define SKIP_CHARACTER                  '#'//B35LB_David_0014 Add


#define CMD_HEAD_LEN                    1
#define CMD_TAIL_LEN                    1
#define CMD_FIX_LEN                     (CMD_HEAD_LEN + CMD_TAIL_LEN)
// Command string length *******************************************************
//common command string
#define CMD_STR_NULL                    ""
#define CMD_STR_NOT_USED                ""
#define CMD_STR_NONE                    "NONE"


#define CMD_STR_CRE_WARM_TIMER          "WarmUpTimer"
#define CMD_STR_CRE_COOL_TIMER          "CoolingDownTimer"

#define CMD_STR_CRE_EMGMSG_START        "EMGMSG"
#define CMD_STR_CRE_EMGMSG_CLEAR        "CLRMSG"

//Common_Tim_0331, add, ***
#define CMD_STR_SRC_HK_VGA              "MHK+VGA1"
#define CMD_STR_SRC_HK_BNC              "MHK+BNC1"
#define CMD_STR_SRC_HK_HDMI1            "MHK+HDM1"
#define CMD_STR_SRC_HK_HDMI2            "MHK+HDM2"
#define CMD_STR_SRC_HK_DVID             "MHK+DVID"
#define CMD_STR_SRC_HK_DP               "MHK+DPRT"
#define CMD_STR_SRC_HK_COMPONENT        "MHK+CON1"
#define CMD_STR_SRC_HK_SVIDEO           "MHK+SVDO"
#define CMD_STR_SRC_HK_COMPOSITE        "MHK+COPS"
#define CMD_STR_SRC_HK_PRESENTER        "MHK+RJ45"
#define CMD_STR_SRC_HK_CARD_RD          "MHK+USB1"
#define CMD_STR_SRC_HK_MINI_USB         "MHK+MIUB"
#define CMD_STR_SRC_HK_HDBASET          "MHK+HDBT"
#define CMD_STR_SRC_HK_3GSDI            "MHK+3SDI"
//Common_Tim_0331, add, &&&

#ifdef ENABLE_PJLINK_SECURITY                               //Common_Tim_0367, add, ***
    #define CMD_STR_PJLINK_PWD          "PJPW"
#endif //ENABLE_PJLINK_SECURITY                             //Common_Tim_0367, add, &&&

//different command string *********************************
#ifdef CUSTOMER_CHRISTIE

//Common_Tim_0332, add, ***
#define EXTERNAL_CMD_START              '('                 //0x28
#define EXTERNAL_CMD_END                ')'                 //0x29
//Common_Tim_0332, add, &&&
#define EXTERNAL_CMD_START_STR          "("                 //Common_Tim_0367, add
#define EXTERNAL_CMD_END_STR            ")"                 //Common_Tim_0367, add

#define CMD_STR_MAIN_SRC_M              "SIN"
#define CMD_STR_MAIN_SRC                "SIN+MAIN"
#define CMD_STR_SEC_SRC                 "SIN+PIIP"
#define CMD_STR_PIP_PBP                 "PIP"
#define CMD_STR_LAYOUT                  "PPP"
#define CMD_STR_PIP_SIZE                "PHS"
#define CMD_STR_PIP_SWAP                "PPS"
#define CMD_STR_TEST_PAT                "ITP" //A35LB_sc_0234, mod ITP+WEEB
#define CMD_STR_PWR_SW                  "PWR"
#define CMD_STR_IMG_MUTE                "SHU"
#define CMD_STR_OSD_SW                  "OSD"
#define CMD_STR_INP_PW_RAT              "PWR+PRAT"          //A70LH_sc_0042, add

#define CMD_STR_MCU_VER                 "PIF+FWV1"          //Common_Tim_0499, add, ***
#define CMD_STR_SCALER_VER              CMD_STR_NONE        //A35LB_sc_0236, mod PIF+FWV2
#define CMD_STR_3D_DECODER_VER          "PIF+FWV3"
#define CMD_STR_FORMATTER_VER           "PIF+FWV2"          //A35LB_sc_0236, mod PIF+FWV4
#define CMD_STR_HDBASET_VER             CMD_STR_NONE        //A35LB_sc_0236, mod PIF+FWV5
#define CMD_STR_KEYPAD_VER              CMD_STR_NONE        //A35LB_sc_0236, mod PIF+FWV6
#define CMD_STR_LD_DRV_VER              CMD_STR_NONE        //A35LB_sc_0236, mod PIF+FWV7
#define CMD_STR_LAN_VER                 "PIF+FWV5"          //A35LB_sc_0236, mod PIF+FWV8//Common_Tim_0499, add, &&&
#define CMD_STR_MOTOR_VER               "PIF+FWV4"          //A35LB_sc_0236, add

#define CMD_STR_ASPECT_RATIO            "SZP"
#define CMD_STR_OVERSCAN                "OVS"
#define CMD_STR_AUTO_IMG                "TMG" //A35LB_sc_0234, mod AIM
#define CMD_STR_PIXEL_TRACK             "PXT"
#define CMD_STR_PIXEL_PHASE             "PXP"
#define CMD_STR_HORZ_P                  "HOR"
#define CMD_STR_VERT_P                  "VRT"
#define CMD_STR_D_ZOOM_H                "DZH"
#define CMD_STR_D_ZOOM_V                "DZV"
#define CMD_STR_D_SHIFT_H               "DSH"
#define CMD_STR_D_SHIFT_V               "DSV"
#define CMD_STR_H_KEYSTONE              "WRP+HKST"
#define CMD_STR_V_KEYSTONE              "WRP+VKST"
#define CMD_STR_H_PINCUSHION            "HPC"
#define CMD_STR_V_PINCUSHION            "VPC"
#define CMD_STR_PC_MODE                 "PCM"
#define CMD_STR_4CTLH                   "CNR+TLCX"
#define CMD_STR_4CTLV                   "CNR+TLCY"
#define CMD_STR_4CTRH                   "CNR+TRCX"
#define CMD_STR_4CTRV                   "CNR+TRCY"
#define CMD_STR_4CBLH                   "CNR+BLCX"
#define CMD_STR_4CBLV                   "CNR+BLCY"
#define CMD_STR_4CBRH                   "CNR+BRCX"
#define CMD_STR_4CBRV                   "CNR+BRCY"
#define CMD_STR_4CRST                   "CNR+RSET"
#define CMD_STR_CRV_TA                  "CRV+TARC"
#define CMD_STR_CRV_BA                  "CRV+BARC"
#define CMD_STR_CRV_LA                  "CRV+LARC"
#define CMD_STR_CRV_RA                  "CRV+RARC"
#define CMD_STR_ROTATION                "ROT"
#define CMD_STR_PIN_BAR                 "PCB"
#define CMD_STR_GEO_RST                 "WRE"
#define CMD_STR_AWF                     "AWF"               //Christie_Tim_0490, add, ***
#define CMD_STR_MWF_HF                  "MWF+HORZ"
#define CMD_STR_MWF_VF                  "MWF+VERT"          //Christie_Tim_0490, add, &&&
#define CMD_STR_4C_TLHA_N               "CNR+TLCX N"        //A35LB_Tim_0529, add, ***
#define CMD_STR_4C_TLHA_P               "CNR+TLCX P"
#define CMD_STR_4C_TLVA_N               "CNR+TLCY N"
#define CMD_STR_4C_TLVA_P               "CNR+TLCY P"
#define CMD_STR_4C_TRHA_N               "CNR+TRCX N"
#define CMD_STR_4C_TRHA_P               "CNR+TRCX P"
#define CMD_STR_4C_TRVA_N               "CNR+TRCY N"
#define CMD_STR_4C_TRVA_P               "CNR+TRCY P"
#define CMD_STR_4C_BLHA_N               "CNR+BLCX N"
#define CMD_STR_4C_BLHA_P               "CNR+BLCX P"
#define CMD_STR_4C_BLVA_N               "CNR+BLCY N"
#define CMD_STR_4C_BLVA_P               "CNR+BLCY P"
#define CMD_STR_4C_BRHA_N               "CNR+BRCX N"
#define CMD_STR_4C_BRHA_P               "CNR+BRCX P"
#define CMD_STR_4C_BRVA_N               "CNR+BRCY N"
#define CMD_STR_4C_BRVA_P               "CNR+BRCY P"        //A35LB_Tim_0529, add, &&&


#define CMD_STR_LP_PWR_SET              "LPP"
#define CMD_STR_LP_CUR_LAMP             "LOP"
#define CMD_STR_LP_ECO_MODE             "WSP"
#define CMD_STR_LP_AUTO_SW              "LSF"
#define CMD_STR_LP_AUTO_SW_TIME         "LSF+TIME"
#define CMD_STR_LP_LIFT_SET             "LPL"
#define CMD_STR_LP_RST_1                "LPC+LMP1"  //Common_Tim_0322, mod, LMP11
#define CMD_STR_LP_RST_2                "LPC+LMP2"  //Common_Tim_0322, mod, LMP21
#define CMD_STR_LP_MODE                 "LPM"
#define CMD_STR_LP_INT_SET              "LPI"
#define CMD_STR_LP_SENSOR_CAL           "LLC"       //Common_Tim_0321, mod, LLC1
//A70LH_sc_0010, add start
#define CMD_STR_LP_ConstIntMode         CMD_STR_NONE
#define CMD_STR_LP_RentalMode           CMD_STR_NONE
//A70LH_sc_0010, add end
#define CMD_STR_LS_RLD_CAL              "RLC"               //Christie_Tim_0452, add


#define CMD_STR_BRIGHTNESS              "BRT"
#define CMD_STR_CONTRAST                "CON"
#define CMD_STR_COLOR_SPACE             "CSP"
#define CMD_STR_DETAIL                  "DTL"
#define CMD_STR_PICTURE_SET             "PST"
#define CMD_STR_SAVE_TO_USER            "PST+USER"    //Common_Tim_0322, mod, USER1
#define CMD_STR_DYNAMIC_BLACK           "DIM"
#define CMD_STR_COLOR                   "CLR"
#define CMD_STR_TINT                    "TNT"
#define CMD_STR_NOISE_REDUCE            "NRD"
#define CMD_STR_FLESH_TONE              "FTC"
#define CMD_STR_VIDEO_BLACK_LVL         "VBL"
#define CMD_STR_DETECT_FILM             "FMD"
#define CMD_STR_CLOSED_CAPTIONS         "CLC"
#define CMD_STR_RED_GAIN                "ROG"
#define CMD_STR_GREEN_GAIN              "GOG"
#define CMD_STR_BLUE_GAIN               "BOG"
#define CMD_STR_RED_OFFSET              "ROO"
#define CMD_STR_GREEN_OFFSET            "GOO"
#define CMD_STR_BLUE_OFFSET             "BOO"
#define CMD_STR_SYNC_THRESHOLD          "SYT"
#define CMD_STR_RESET_RGB_G_O           "GOR"       //Common_Tim_0322, mod, GOR1
#define CMD_STR_GAMMA                   "BGC"
#define CMD_STR_BRILLIANT_COLOR         "BCL"
#define CMD_STR_COLOR_TEMP              "CCI"
#define CMD_STR_EDGE_ENHANCE            "EDG"
#define CMD_STR_WHITE_PEAKING           "WPK"
#define CMD_STR_IMG_FREEZE              "FRZ"
#define CMD_STR_COLOR_WHEEL_SPD         "CWS"
#define CMD_STR_COLOR_ENHANCE           "HSG"
#define CMD_STR_3D_ENABLE               "TDE"
#define CMD_STR_3D_INVERT               "TDI"
#define CMD_STR_TI_T3B                  "TDT" //A70LH_sc_0027, add
#define CMD_STR_TI_REAL_BLACK           CMD_STR_NONE //A70LH_sc_0036, add start
#define CMD_STR_TI_FRAME_PACK           CMD_STR_NONE
#define CMD_STR_TI_S_BY_S               CMD_STR_NONE
#define CMD_STR_TI_T_AND_B              CMD_STR_NONE
#define CMD_STR_TI_FRAME_SEQ            CMD_STR_NONE //A70LH_sc_0036, add start
#define CMD_STR_TI_S_T_U                CMD_STR_NONE //A70LH_sc_0091, add
#define CMD_STR_TI_3D_SYNC_OUT          "SOS"               //Christie_Tim_0476, add
#define CMD_STR_TI_FRAME_DELAY          "FDY"               //Christie_Tim_0477, add
#define CMD_STR_TI_3D                   "TDN"               //A35LB_sc_0234, add
#define CMD_STR_TI_1080P24              "TDO"               //A35LB_sc_0234, add
#define CMD_STR_HDR                     "HDR"
#define CMD_STR_MEMC_DETAIL             "MMC+DTIL"
#define CMD_STR_MEMC_COLOR              "MMC+COLR"
#define CMD_STR_MEMC_MOTION             "MMC+MOTN"
#define CMD_STR_MEMC_MOTION_DEMO        "MMC+DEMO"
#define CMD_STR_MENU_TIME_OUT           "MTO"
#define CMD_STR_LENS_TYPE               "LET"
#define CMD_STR_WHEEL_INDEX             "PIF+WHEE"
#define CMD_STR_ERR_LOG                 "ERR"
#define CMD_STR_ERR_LOG_CLEAR           "ERR+CLER1"
#define CMD_STR_FAN1                    "FAN+FA01"
#define CMD_STR_FAN2                    "FAN+FA02"
#define CMD_STR_FAN3                    "FAN+FA03"
#define CMD_STR_FAN4                    "FAN+FA04"
#define CMD_STR_FAN5                    "FAN+FA05"
#define CMD_STR_FAN6                    "FAN+FA06"
#define CMD_STR_FAN7                    "FAN+FA07"
#define CMD_STR_FAN8                    "FAN+FA08"
#define CMD_STR_FAN9                    "FAN+FA09"
#define CMD_STR_FAN10                   "FAN+FA10"
#define CMD_STR_NET_MAC                 "NET+MAC0"
#define CMD_STR_NET_DHCP                "NET+DHCP"
#define CMD_STR_NET_ETH                 "NET+ETH0"
#define CMD_STR_NET_SUB                 "NET+SUB0"
#define CMD_STR_NET_GATE                "NET+GATE"
#define CMD_STR_NET_PDNS                "NET+PDNS"
#define CMD_STR_NET_SDNS                "NET+SDNS"
#define CMD_STR_NET_SETT                "NET+SETT"
#define CMD_STR_NTW_MAC                 "NTW+MAC0"
#define CMD_STR_NTW_SSID                "NTW+SSID"
#define CMD_STR_NTW_SLCT                "NTW+SLCT"
#define CMD_STR_NTW_ETH                 "NTW+ETH0"
#define CMD_STR_NTW_ENIP                "NTW+ENIP"
#define CMD_STR_NTW_SUB                 "NTW+SUB0"
#define CMD_STR_NTW_GATE                "NTW+GATE"
#define CMD_STR_NTW_HOST                "NET+HOST"//T100_Daivd_0004
#define CMD_STR_NET_RSET                "NET+RSET"
#define CMD_STR_INFO_MODEL              "PIF+MDLN"
#define CMD_STR_INFO_RESOL              "PIF+NERS"
#define CMD_STR_INFO_SERIL              "PIF+SNUM"
#define CMD_STR_INFO_FMCU               "PIF+FWV1"
#define CMD_STR_INFO_FIMA               "PIF+FWV2"
#define CMD_STR_INFO_SMCU               "PIF+FWV3"
#define CMD_STR_INFO_MEMC               "PIF+FWV4"
#define CMD_STR_INFO_FORM               "PIF+FWV5"
#define CMD_STR_INFO_HDBT               "PIF+FWV6"


#define CMD_STR_LANGUAGE                "LOC+LANG"
#define CMD_STR_CEIL_MOUNT              "CEL"
#define CMD_STR_REAR_PROJ               "SOR"
#define CMD_STR_HIGH_ALTITUDE           "HAT"
#define CMD_STR_12V_TRIGGER             "VTT"  //A70LH_sc_0001, add
#define CMD_STR_HOT_KEY_SET             "HKS"
#define CMD_STR_ZOOM                    "ZOM"
#define CMD_STR_FOCUS                   "FCS"
#define CMD_STR_LENS_SHIFT_V            "LVO"
#define CMD_STR_LENS_SHIFT_H            "LHO"
#define CMD_STR_ZOOM_UP                 "ZOM P"
#define CMD_STR_ZOOM_DOWN               "ZOM N"
#define CMD_STR_FOCUS_IN                "FCS N"
#define CMD_STR_FOCUS_OUT               "FCS P"
#define CMD_STR_LENS_SHIFT_U            "LVO P"
#define CMD_STR_LENS_SHIFT_D            "LVO N"
#define CMD_STR_LENS_SHIFT_L            "LHO P"
#define CMD_STR_LENS_SHIFT_R            "LHO N"
#define CMD_STR_LENS_LOCK               "LCB+LOCK"
#define CMD_STR_LENS_CAL                "LCB+HOME" //Common_Tim_0322, mod, HOME1
#define CMD_STR_MENU_HORZ               "MSH"
#define CMD_STR_MENU_VERT               "MSV"
#define CMD_STR_SHOW_MSG                "MBE+USER"
#define CMD_STR_MENU_TRANSPARENCY       "OST"
#define CMD_STR_BG_COLOR                "SPS+SLCT"
#define CMD_STR_PIN_PROTECT             "PIV"
#define CMD_STR_CHG_PIN                 "PCG"
#define CMD_STR_STANBY_MODE             "PWR+STBM"
#define CMD_STR_AC_PWR_ON               "APW"
#define CMD_STR_AUTO_SHUTDOWN           "ASH"
#define CMD_STR_SLEEP_TIMER             "SLP"
#define CMD_STR_UART_BAUD_RATE          "BDR"
#define CMD_STR_UART_ECHO               "SEC"
#define CMD_STR_FACTORY_RESET           "DEF11"    //Common_Tim_0322, mod, DEF111
#define CMD_STR_CCA_MT_EN               "CCA+MTRA"
#define CMD_STR_CCA_MT_MD_R_INT         "CCA+RDMI"
#define CMD_STR_CCA_MT_MD_R_X           "CCA+RDMX"
#define CMD_STR_CCA_MT_MD_R_Y           "CCA+RDMY"
#define CMD_STR_CCA_MT_MD_G_INT         "CCA+GNMI"
#define CMD_STR_CCA_MT_MD_G_X           "CCA+GNMX"
#define CMD_STR_CCA_MT_MD_G_Y           "CCA+GNMY"
#define CMD_STR_CCA_MT_MD_B_INT         "CCA+BLMI"
#define CMD_STR_CCA_MT_MD_B_X           "CCA+BLMX"
#define CMD_STR_CCA_MT_MD_B_Y           "CCA+BLMY"
#define CMD_STR_CCA_MT_MD_W_INT         "CCA+WHMI"
#define CMD_STR_CCA_MT_MD_W_X           "CCA+WHMX"
#define CMD_STR_CCA_MT_MD_W_Y           "CCA+WHMY"
#define CMD_STR_CCA_MT_TD_R_GAIN        "CCA+RDDG"
#define CMD_STR_CCA_MT_TD_R_X           "CCA+RDDX"
#define CMD_STR_CCA_MT_TD_R_Y           "CCA+RDDY"
#define CMD_STR_CCA_MT_TD_G_GAIN        "CCA+GNDG"
#define CMD_STR_CCA_MT_TD_G_X           "CCA+GNDX"
#define CMD_STR_CCA_MT_TD_G_Y           "CCA+GNDY"
#define CMD_STR_CCA_MT_TD_B_GAIN        "CCA+BLDG"
#define CMD_STR_CCA_MT_TD_B_X           "CCA+BLDX"
#define CMD_STR_CCA_MT_TD_B_Y           "CCA+BLDY"
#define CMD_STR_CCA_MT_TD_W_GAIN        "CCA+WHDG"
#define CMD_STR_CCA_MT_TD_W_X           "CCA+WHDX"
#define CMD_STR_CCA_MT_TD_W_Y           "CCA+WHDY"
#define CMD_STR_CCA_MT_TEST_PAT         "CCA+MTTP"
#define CMD_STR_CCA_MN_EN               "CCA+MANA"
#define CMD_STR_CCA_MN_TEST_PAT         "CCA+MNTP"
#define CMD_STR_CCA_MN_RPR              "CCA+ROFR"
#define CMD_STR_CCA_MN_GPR              "CCA+GOFR"
#define CMD_STR_CCA_MN_BPR              "CCA+BOFR"
#define CMD_STR_CCA_MN_GPG              "CCA+GOFG"
#define CMD_STR_CCA_MN_RPG              "CCA+ROFG"
#define CMD_STR_CCA_MN_BPG              "CCA+BOFG"
#define CMD_STR_CCA_MN_BPB              "CCA+BOFB"
#define CMD_STR_CCA_MN_RPB              "CCA+ROFB"
#define CMD_STR_CCA_MN_GPB              "CCA+GOFB"
#define CMD_STR_CCA_MN_RPW              "CCA+ROFW"
#define CMD_STR_CCA_MN_GPW              "CCA+GOFW"
#define CMD_STR_CCA_MN_BPW              "CCA+BOFW"
#define CMD_STR_PROJ_ADDR               "ADR"
#define CMD_STR_CCA_MT_APPLY            "CCA+CLMA" //Common_Tim_0321, mod, CLMA1
#define CMD_STR_CW_IDX_2X               "CWI+SPX2"
#define CMD_STR_CW_IDX_3X               "CWI+SPX3"
#define CMD_STR_CCA_MT_MD_RESET         "MDR"      //Common_Tim_0321, mod, MDR1
#define CMD_STR_CCA_MT_TD_RESET         "TDR"      //Common_Tim_0321, mod, MDR1
#define CMD_STR_CCA_MN_RESET            "MAR"      //Common_Tim_0321, mod, MAR1
#define CMD_STR_PHOSPHER_W_IDX          "CWI+PF2X" //A70LH_sc_0013, mod CWI+PHSF
#define CMD_STR_FILTER_W_IDX            "CWI+FT2X" //A70LH_sc_0013, mod CWI+FILT
#define CMD_STR_LENS_MEM_APPLY          "LMA"
#define CMD_STR_LENS_MEM_SAVE           "LMS"
#define CMD_STR_KP_BACK_LIGHT           "KBL"
#define CMD_STR_STATUS_LED              "SBL"
#define CMD_STR_UART_PATH               "SPP"
//A70LH_sc_0002, add start
#define CMD_STR_IR_CTRL_TOP             "IRC+TOPP"
#define CMD_STR_IR_CTRL_FRONT           "IRC+FRNT"
#define CMD_STR_IR_CTRL_HDBASET         "IRC+HDBT"
//A70LH_sc_0002, add end
//A70LH_sc_0013, add start
#define CMD_STR_TC_PI3X                 "CWI+PF3X"
#define CMD_STR_TC_FI3X                 "CWI+FT3X"
//A70LH_sc_0013, add end
#define CMD_STR_TC_WallCollor           "CCA+WALL"          //A70LH_sc_0016, add
#define CMD_STR_TC_INS_PWR_OFF          "PWR+ISTF"          //A70LH_sc_0068, add
#define CMD_STR_TC_HDMI_EQ              "HEQ"               //A35LB_sc_0234, add
#define CMD_STR_TC_LOGO_CAPTURE         "LCA"               //A35LB_sc_0234, add
#define CMD_STR_TC_MENU_LOCATION        "MEL"               //A35LB_sc_0234, add

//A70LH_sc_0025, add start
#define CMD_STR_HSG_En                  "CCA+HSGE"
#define CMD_STR_HSG_ATP                 "CCA+MHTP"
#define CMD_STR_HSG_RH                  "CCA+REDH"
#define CMD_STR_HSG_RS                  "CCA+REDS"
#define CMD_STR_HSG_RG                  "CCA+REDG"
#define CMD_STR_HSG_RD                  "CCA+RRTD"
#define CMD_STR_HSG_GH                  "CCA+GREH"
#define CMD_STR_HSG_GS                  "CCA+GRES"
#define CMD_STR_HSG_GG                  "CCA+GREG"
#define CMD_STR_HSG_GD                  "CCA+GRTD"
#define CMD_STR_HSG_BH                  "CCA+BLUH"
#define CMD_STR_HSG_BS                  "CCA+BLUS"
#define CMD_STR_HSG_BG                  "CCA+BLUG"
#define CMD_STR_HSG_BD                  "CCA+BRTD"
#define CMD_STR_HSG_CH                  "CCA+CYAH"
#define CMD_STR_HSG_CS                  "CCA+CYAS"
#define CMD_STR_HSG_CG                  "CCA+CYAG"
#define CMD_STR_HSG_CD                  "CCA+CRTD"
#define CMD_STR_HSG_MH                  "CCA+MAGH"
#define CMD_STR_HSG_MS                  "CCA+MAGS"
#define CMD_STR_HSG_MG                  "CCA+MAGG"
#define CMD_STR_HSG_MD                  "CCA+MRTD"
#define CMD_STR_HSG_YH                  "CCA+YELH"
#define CMD_STR_HSG_YS                  "CCA+YELS"
#define CMD_STR_HSG_YG                  "CCA+YELG"
#define CMD_STR_HSG_YD                  "CCA+YRTD"
#define CMD_STR_HSG_WR                  "CCA+WHRG"
#define CMD_STR_HSG_WG                  "CCA+WHGG"
#define CMD_STR_HSG_WB                  "CCA+WHBG"
#define CMD_STR_HSG_WD                  "CCA+WRTD"
#define CMD_STR_HSG_RTD                 "HAR"
#define CMD_STR_HSG_CE                  "HSG"
//A70LH_sc_0025, add end

#define CMD_STR_MS_LD1                  "LDI+LD01" //A70LH_sc_0029, add start
#define CMD_STR_MS_LD2                  "LDI+LD02"
#define CMD_STR_MS_LD3                  "LDI+LD03"
#define CMD_STR_MS_LD4                  "LDI+LD04"
#define CMD_STR_MS_LD5                  "LDI+LD05"
#define CMD_STR_MS_LD6                  "LDI+LD06"
#define CMD_STR_MS_LD7                  "LDI+LD07"
#define CMD_STR_MS_LD8                  "LDI+LD08"
#define CMD_STR_MS_LD9                  "LDI+LD09"
#define CMD_STR_MS_LD10                 "LDI+LD10" //A70LH_sc_0031,mod
#define CMD_STR_MS_LD11                 "LDI+LD11" //A70LH_sc_0031,mod
#define CMD_STR_MS_LD12                 "LDI+LD12" //A70LH_sc_0031,mod
#define CMD_STR_MS_LD13                 "LDI+LD13" //A70LH_sc_0031,mod//A70LH_sc_0029, add end

#define CMD_STR_TIME_DETECT_MODE        "TMG"
#define CMD_STR_SRC_HOT_KEY_EN          "ESH"
#define CMD_STR_SRC_CHANGE_MODE         "SKS"
//Common_Tim_0331, mov, *&
#define CMD_STR_BLANK_ON_SRC_SW         "BSS"
#define CMD_STR_ENTER_SERVICE_CODE      "UID"
#define CMD_STR_SERIAL_CMD_VERSION      "SIV"
#define CMD_STR_LAST_SERIAL_CMD_ERR     "LCE"
#define CMD_STR_LAST_SYSTEM_ERR         "LSE"
#define CMD_STR_SOURCE_NAME_SET0        "SNS+SRC0"
#define CMD_STR_SOURCE_NAME_SET1        "SNS+SRC1"
#define CMD_STR_SOURCE_NAME_SET2        "SNS+SRC2"
#define CMD_STR_SOURCE_NAME_SET3        "SNS+SRC3"
#define CMD_STR_SOURCE_NAME_SET4        "SNS+SRC4"
#define CMD_STR_SOURCE_NAME_SET5        "SNS+SRC5"
#define CMD_STR_SOURCE_NAME_SET6        "SNS+SRC6"
#define CMD_STR_SOURCE_NAME_SET7        "SNS+SRC7"
#define CMD_STR_SOURCE_NAME_SET8        "SNS+SRC8"
#define CMD_STR_SOURCE_NAME_SET9        "SNS+SRC9"
#define CMD_STR_SERIAL_NUMBER_SET       "FCT+SERN"

#define CMD_STR_NET_SHOW_MSG            "NET+SHOW"
#define CMD_STR_NET_RESTART             "NET+RSTR"          //Common_Tim_0322, mod, NET+RSTR1
#define CMD_STR_NET_F_RESET             CMD_STR_NONE        //B35LB_Kraz_0118, add

#define CMD_STR_NET_SRC_READY_C         "SYS+CARD"
#define CMD_STR_NET_SRC_READY_M         "SYS+MINU"
#define CMD_STR_NET_SRC_READY_P         "SYS+NETP"

#define CMD_STR_TRISTATE                "SIN+TRIS"


#define CMD_STR_MAIN_SRC_QUERY          "SIN+MAIN?"         //A35LB_Tim_0544, add
#define CMD_STR_SEC_SRC_QUERY           "SIN+PIIP?"         //A35LB_Tim_0544, add

#define CMD_STR_WARP_APPLY              "WAP"
#define CMD_STR_WARP_SAVE               "WAS"
#define CMD_STR_WARP_MODE               "WRP+MWWM"
#define CMD_STR_WARP_PITCH              "WRP+MWMP"
#define CMD_STR_WARP_CURSOR             "WRP+MWCA"

#define CMD_STR_BLENDING_TOP_ENABLE     "EBL+TOPE"
#define CMD_STR_BLENDING_TOP_START      "EBL+TOPS"
#define CMD_STR_BLENDING_TOP_HEIGHT     "EBL+TOPW"
#define CMD_STR_BLENDING_BOTTOM_ENABLE  "EBL+BTME"
#define CMD_STR_BLENDING_BOTTOM_START   "EBL+BTMS"
#define CMD_STR_BLENDING_BOTTOM_HEIGHT  "EBL+BTMW"
#define CMD_STR_BLENDING_LEFT_ENABLE    "EBL+LFTE"//T100_Daivd_0004
#define CMD_STR_BLENDING_LEFT_START     "EBL+LFTS"//T100_Daivd_0004
#define CMD_STR_BLENDING_LEFT_HEIGHT    "EBL+LFTW"//T100_Daivd_0004
#define CMD_STR_BLENDING_RIGHT_ENABLE   "EBL+RHTE"
#define CMD_STR_BLENDING_RIGHT_START    "EBL+RHTS"
#define CMD_STR_BLENDING_RIGHT_HEIGHT   "EBL+RHTW"

#define CMD_STR_CRE_LP_1_HOURS          "LIF+LSHS"          //A70LH_Kraz_0270, mod, Light Source Hours
#define CMD_STR_CRE_LP_T_HOURS          "LIF+TPHS"
//Common_Tim_0325, mov, *&
#if A35LS_CHRISTIE                                          //Christie_Tim_0468, mod
#define CMD_STR_CRE_LP_1_HOURS          "LIF+LSHS"          //A70LH_Kraz_0270, mod, Light Source Hours
#define CMD_STR_CRE_LP_2_HOURS          "LIF+REDL"          //A70LH_Kraz_0270, mod, Red Laser Hours
#define CMD_STR_CRE_LP_T_HOURS          "LIF+TPHS"          //A70LH_Kraz_0270, mod, Total Projector Hours
#define CMD_STR_CRE_LC_HOURS            "LIF+LCHS"          //A70LH_Kraz_0272, mod //A70LH_Kraz_0270, add, TEC Life Hours
#elif (A35LB_CHRISTIE)                                                          //Christie_Tim_0504, add, add
#define CMD_STR_CRE_LP_1_HOURS          "LIF+LSHS"          //Light Source Hours
#define CMD_STR_CRE_LP_2_HOURS          "LIF+REDL"          //Red Laser Hours
#define CMD_STR_CRE_LP_T_HOURS          "LIF+TPHS"          //Total Projector Hours
#define CMD_STR_CRE_LC_HOURS            "LIF+LCHS"          //TEC Life Hours    //Christie_Tim_0504, add, &&&
#else
//#define CMD_STR_CRE_LP_1_HOURS          "LPL+LP1H"          // "LIF+LP1H"   <== shall be check
//#define CMD_STR_CRE_LP_2_HOURS          "LPL+LP2H"          // "LIF+LP2H"   <== shall be check
//#define CMD_STR_CRE_LP_T_HOURS          "LPL+LPTH"          //Common_Kraz_0217, add, Total Projector Hour
//#define CMD_STR_CRE_LC_HOURS            CMD_STR_NONE        //A70LH_Kraz_0272, add, TEC Life Hours
#endif
#define CMD_STR_CRE_RESOLUTION          "NAR"
#define CMD_STR_CRE_SYS_TEMP            "SYS+TEMP"

//B35LB_Kraz_0089, mod : start //Common_Tim_0328, add, start
#define CMD_STR_CRE_CMD_MENU            "KEY19"
#define CMD_STR_CRE_CMD_UP              "KEY38"
#define CMD_STR_CRE_CMD_DOWN            "KEY42"
#define CMD_STR_CRE_CMD_LEFT            "KEY39"
#define CMD_STR_CRE_CMD_RIGHT           "KEY41"
#define CMD_STR_CRE_CMD_ENTER           "KEY40"
#define CMD_STR_CRE_CMD_INPUT           "KEY48"
#define CMD_STR_CRE_CMD_EXIT            "KEY20"
#define CMD_STR_CRE_CMD                 "KEY"
#define CMD_CRE_CMD_MENU                19
#define CMD_CRE_CMD_UP                  38
#define CMD_CRE_CMD_DOWN                42
#define CMD_CRE_CMD_LEFT                39
#define CMD_CRE_CMD_RIGHT               41
#define CMD_CRE_CMD_ENTER               40
#define CMD_CRE_CMD_INPUT               48
#define CMD_CRE_CMD_EXIT                20
#define CMD_CRE_CMD_INFO                66
#define CMD_CRE_CMD_AUTO                47
#define CMD_CRE_CMD_NUM1                26
#define CMD_CRE_CMD_NUM2                27
#define CMD_CRE_CMD_NUM3                28
#define CMD_CRE_CMD_NUM4                29
#define CMD_CRE_CMD_NUM5                30
#define CMD_CRE_CMD_NUM6                31
#define CMD_CRE_CMD_NUM7                32
#define CMD_CRE_CMD_NUM8                33
#define CMD_CRE_CMD_NUM9                34
#define CMD_CRE_CMD_NUM0                36
//B35LB_Kraz_0089, mod : end   //Common_Tim_0328, add, end

#else //ENABLE_CORE_CMD

//Common_Tim_0332, add, ***
#define EXTERNAL_CMD_START              '['                 //0x5B
#define EXTERNAL_CMD_END                ']'                 //0x5D
//Common_Tim_0332, add, &&&
#define EXTERNAL_CMD_START_STR          "["                 //Common_Tim_0367, add
#define EXTERNAL_CMD_END_STR            "]"                 //Common_Tim_0367, add

#define CMD_LENGTH_NONE                  (0)
#define CMD_LENGTH_ONE                   (1)               //B35LB_David_0013 add
#define CMD_LENGTH_THREE                 (3)
#define CMD_LENGTH_FOUR                  (4)
#define CMD_LENGTH_EIGHT                 (8)
#define CMD_LENGTH_TEN                  (10)

#define CMD_STR_MAIN_SRC                "MSRC"          //MSRC              //B35LB_Tim_0011, mod, start//B35LB_David_0029 Add
#define CMD_STR_SEC_SRC                 "MSRC+PIIP"          //SSRC//B35LB_David_0029 Add
#define CMD_STR_PIP_PBP                 "PIBP"               //PIBP//B35LB_David_0029 Add
#define CMD_STR_LAYOUT                  "PPPG"               //PILO//B35LB_David_0029 Add
#define CMD_STR_PIP_SIZE                "PHSG"               //PISZ//B35LB_David_0029 Add
#define CMD_STR_PIP_SWAP                "PISW"               //PISW//B35LB_David_0029 Add
#define CMD_STR_TEST_PAT                "TPRN"               //TPRN//B35LB_David_0029 Add
#define CMD_STR_PWR_SW                  "POWR"               //POWR//B35LB_David_0029 Add
#define CMD_STR_IMG_MUTE                "SHUG"               //PMUT//B35LB_David_0029 Add
#define CMD_STR_OSD_SW                  "OSDG"               //CMD_STR_NONE//B35LB_David_0029 Add
#define CMD_STR_INP_PW_RAT              "PWR+PRAT"          //CMD_STR_NONE      //B35LB_Tim_0011, mod, end

#define CMD_STR_MCU_VER                 "PIFG+FWV1"          //Common_Tim_0499, add, ***//B35LB_David_0029 Add
#define CMD_STR_SCALER_VER              CMD_STR_NONE        //A35LB_sc_0236, mod PIF+FWV2
#define CMD_STR_3D_DECODER_VER          "PIFG+FWV3"         //B35LB_David_0029 Add
#define CMD_STR_FORMATTER_VER           "PIFG+FWV2"          //A35LB_sc_0236, mod PIF+FWV4//B35LB_David_0029 Add
#define CMD_STR_HDBASET_VER             CMD_STR_NONE        //A35LB_sc_0236, mod PIF+FWV5
#define CMD_STR_KEYPAD_VER              CMD_STR_NONE        //A35LB_sc_0236, mod PIF+FWV6
#define CMD_STR_LD_DRV_VER              CMD_STR_NONE        //A35LB_sc_0236, mod PIF+FWV7
#define CMD_STR_LAN_VER                 "PIFG+FWV5"          //A35LB_sc_0236, mod PIF+FWV8//Common_Tim_0499, add, &&&//B35LB_David_0029 Add
#define CMD_STR_MOTOR_VER               "PIFG+FWV4"          //A35LB_sc_0236, add//B35LB_David_0029 Add

#define CMD_STR_ASPECT_RATIO            "ASPR"
#define CMD_STR_OVERSCAN                "OVSC"
#define CMD_STR_AUTO_IMG                "TMGG"                //B35LB_David_0029 Add
#define CMD_STR_PIXEL_TRACK             "CLCK"
#define CMD_STR_PIXEL_PHASE             "PHAS"
#define CMD_STR_HORZ_P                  "HPOS"
#define CMD_STR_VERT_P                  "VPOS"
#define CMD_STR_D_ZOOM_H                "HDZM"
#define CMD_STR_D_ZOOM_V                "VDZM"
#define CMD_STR_D_SHIFT_H               "HDSH"
#define CMD_STR_D_SHIFT_V               "VDSH"
#define CMD_STR_H_KEYSTONE              "WRPG+HKST"//B35LB_David_0033 Mod
#define CMD_STR_V_KEYSTONE              "WRPG+VKST"//B35LB_David_0033 Mod
#define CMD_STR_H_PINCUSHION            "HPIC"
#define CMD_STR_V_PINCUSHION            "VPIC"
#define CMD_STR_PC_MODE                 "PCMG"          //B35LB_David_0029 Add
#define CMD_STR_4CTLH                   CMD_STR_NONE    // Geo 4 Corner Top Left Horz.
#define CMD_STR_4CTLV                   CMD_STR_NONE    // Geo 4 Corner Top Left Vert.
#define CMD_STR_4CTRH                   CMD_STR_NONE    // Geo 4 Corner Top Right Horz.
#define CMD_STR_4CTRV                   CMD_STR_NONE    // Geo 4 Corner Top Right Vert.
#define CMD_STR_4CBLH                   CMD_STR_NONE    // Geo 4 Corner Bottom  Left Horz.
#define CMD_STR_4CBLV                   CMD_STR_NONE    // Geo 4 Corner Bottom  Left Vert.
#define CMD_STR_4CBRH                   CMD_STR_NONE    // Geo 4 Corner Bottom  Right Horz.
#define CMD_STR_4CBRV                   CMD_STR_NONE    // Geo 4 Corner Bottom  Right Vert.
#define CMD_STR_CRV_TA                  CMD_STR_NONE    // Geo Curve Top Arc
#define CMD_STR_CRV_BA                  CMD_STR_NONE    // Geo Curve Bottom Arc
#define CMD_STR_CRV_LA                  CMD_STR_NONE    // Geo Curve Left Arc
#define CMD_STR_CRV_RA                  CMD_STR_NONE    // Geo Curve Right Arc
#define CMD_STR_ROTATION                CMD_STR_NONE    // Geo Rotation
#define CMD_STR_PIN_BAR                 CMD_STR_NONE    // Geo Pincushion, Barrel
#define CMD_STR_GEO_RST                 "WREG"    // Geo Reset//B35LB_David_0029 Add
#define CMD_STR_AWF                     "AWF"               //Christie_Tim_0490, add, ***
#define CMD_STR_MWF_HF                  "MWF+HORZ"
#define CMD_STR_MWF_VF                  "MWF+VERT"          //Christie_Tim_0490, add, &&&
#define CMD_STR_4C_TLHA_N               "CNRG+TLCX N"        //A35LB_Tim_0529, add, *** //B35LB_David_0033 Mod Start
#define CMD_STR_4C_TLHA_P               "CNRG+TLCX P"
#define CMD_STR_4C_TLVA_N               "CNRG+TLCY N"
#define CMD_STR_4C_TLVA_P               "CNRG+TLCY P"
#define CMD_STR_4C_TRHA_N               "CNRG+TRCX N"
#define CMD_STR_4C_TRHA_P               "CNRG+TRCX P"
#define CMD_STR_4C_TRVA_N               "CNRG+TRCY N"
#define CMD_STR_4C_TRVA_P               "CNRG+TRCY P"
#define CMD_STR_4C_BLHA_N               "CNRG+BLCX N"
#define CMD_STR_4C_BLHA_P               "CNRG+BLCX P"
#define CMD_STR_4C_BLVA_N               "CNRG+BLCY N"
#define CMD_STR_4C_BLVA_P               "CNRG+BLCY P"
#define CMD_STR_4C_BRHA_N               "CNRG+BRCX N"
#define CMD_STR_4C_BRHA_P               "CNRG+BRCX P"
#define CMD_STR_4C_BRVA_N               "CNRG+BRCY N"
#define CMD_STR_4C_BRVA_P               "CNRG+BRCY P"        //A35LB_Tim_0529, add, &&& //B35LB_David_0033 Mod End

#define CMD_STR_LP_PWR_SET              "LPPW"               //B35LB_Tim_0022, mod, for DVT only "LPPW"//B35LB_David_0029 Add
#define CMD_STR_LP_CUR_LAMP             "LPCR"
#define CMD_STR_LP_ECO_MODE             CMD_STR_NONE    // Lamp Eco mode setting
#define CMD_STR_LP_AUTO_SW              "LPAS"
#define CMD_STR_LP_AUTO_SW_TIME         "LPST"
#define CMD_STR_LP_LIFT_SET             "LPL"               //CMD_STR_NONE    //Lamp life setting//B35LB_David_0024 Mod
#define CMD_STR_LP_RST_1                "LPRH"              //Common_Tim_0329, mod, LP1R //Common_Tim_0325, mod, LPRH
#define CMD_STR_LP_RST_2                "LPRH"              //Common_Tim_0329, mod, LP2R //Common_Tim_0325, mod, LPRH
#define CMD_STR_LP_MODE                 "LPMO"               //B35LB_Tim_0022, mod, for DVT only "LPMO"//B35LB_David_0029 Add
#define CMD_STR_LP_INT_SET              "LPAM"
#define CMD_STR_LP_SENSOR_CAL           "LPCA"
#define CMD_STR_LP_ConstIntMode         CMD_STR_NONE        //A70LH_sc_0014, add
#define CMD_STR_LP_RentalMode           CMD_STR_NONE        //A70LH_sc_0014, add
#define CMD_STR_LS_RLD_CAL              CMD_STR_NONE        //Christie_Tim_0452, add

#define CMD_STR_BRIGHTNESS              "BRIG"               //B35LB_Tim_0022, mod, for DVT only "BRIG"//B35LB_David_0029 Add
#define CMD_STR_CONTRAST                "CONT"               //B35LB_Tim_0022, mod, for DVT only "CONT"//B35LB_David_0029 Add
#define CMD_STR_COLOR_SPACE             "CSPA"
#define CMD_STR_DETAIL                  "SHRP"               //B35LB_Tim_0022, mod, for DVT only "SHRP"  //SHARPNESS//B35LB_David_0029 Add
#define CMD_STR_PICTURE_SET             "DPMO"               //B35LB_Tim_0022, mod, for DVT only "DPMO"  //DISPLAY MODE//B35LB_David_0029 Add
#define CMD_STR_SAVE_TO_USER            "DPSU"
#define CMD_STR_USER_MODE_APPLY         "DPAU"               //G100_Steven_0030
#define CMD_STR_DYNAMIC_BLACK           "DYBK"
#define CMD_STR_COLOR                   "COLR"               //B35LB_Tim_0022, mod, for DVT only "COLR"//B35LB_David_0029 Add
#define CMD_STR_TINT                    "TINT"
#define CMD_STR_NOISE_REDUCE            "NORD"
#define CMD_STR_FLESH_TONE              "FTCR"
#define CMD_STR_VIDEO_BLACK_LVL         "VBKL"
#define CMD_STR_DETECT_FILM             "FLMO" // FILM MODE
#define CMD_STR_CLOSED_CAPTIONS         "CCAP"
#define CMD_STR_RED_GAIN                "RGAN"
#define CMD_STR_GREEN_GAIN              "GGAN"
#define CMD_STR_BLUE_GAIN               "BGAN"
#define CMD_STR_RED_OFFSET              "ROFS"
#define CMD_STR_GREEN_OFFSET            "GOFS"
#define CMD_STR_BLUE_OFFSET             "BOFS"
#define CMD_STR_SYNC_THRESHOLD          CMD_STR_NONE    // Sync Threshold
#define CMD_STR_RESET_RGB_G_O           "RSGO"
#define CMD_STR_GAMMA                   "GAMM"
#define CMD_STR_BRILLIANT_COLOR         "BLCL"
#define CMD_STR_COLOR_TEMP              "CTMP"
#define CMD_STR_EDGE_ENHANCE            CMD_STR_NONE    // Edge Enhance
#define CMD_STR_WHITE_PEAKING           "WHPK"
#define CMD_STR_IMG_FREEZE              "FRZE"               //B35LB_Tim_0011, mod, FRZE//B35LB_David_0029 Add
#define CMD_STR_COLOR_WHEEL_SPD         "CWSP"
#define CMD_STR_COLOR_ENHANCE           "CLEH"
#define CMD_STR_3D_ENABLE               "TDEG"              //B35LB_David_0029 Add
#define CMD_STR_3D_INVERT               "TDIG"              //B35LB_David_0029 Add
#define CMD_STR_TI_T3B                  CMD_STR_NONE        //A70LH_sc_0027, add
#define CMD_STR_TI_REAL_BLACK           CMD_STR_NONE        //A70LH_sc_0036, add start
#define CMD_STR_TI_FRAME_PACK           CMD_STR_NONE
#define CMD_STR_TI_S_BY_S               CMD_STR_NONE
#define CMD_STR_TI_T_AND_B              CMD_STR_NONE
#define CMD_STR_TI_FRAME_SEQ            CMD_STR_NONE        //A70LH_sc_0036, add start
#define CMD_STR_TI_S_T_U                CMD_STR_NONE        //A70LH_sc_0091, add
#define CMD_STR_TI_3D_SYNC_OUT          "SOSG"        //Christie_Tim_0476, add//B35LB_David_0029 Add
#define CMD_STR_TI_FRAME_DELAY          "FDYG"        //Christie_Tim_0477, add//B35LB_David_0029 Add
#define CMD_STR_TI_3D                   "TDNG"        //A35LB_sc_0234, add//B35LB_David_0029 Add
#define CMD_STR_TI_1080P24              "TDOG"        //A35LB_sc_0234, add//B35LB_David_0029 Add

#define CMD_STR_LANGUAGE                "LOCG+LANG"   //B35LB_David_0033 Mod
#define CMD_STR_CEIL_MOUNT              "CEMO"               //B35LB_Tim_0022, mod, for DVT only "CEMO"//B35LB_David_0029 Add
#define CMD_STR_REAR_PROJ               "REPJ"
#define CMD_STR_HIGH_ALTITUDE           "HIAT"
#define CMD_STR_12V_TRIGGER             CMD_STR_NONE        //A70LH_sc_0001, add
#define CMD_STR_HOT_KEY_SET             "HKST"
#define CMD_STR_ZOOM_UP                 "ZOMO"              //Common_Tim_0329, mod, ZOMU //Common_Tim_0325, mod,
#define CMD_STR_ZOOM_DOWN               "ZOMI"              //Common_Tim_0329, mod,  ZOMD//Common_Tim_0325, mod,
#define CMD_STR_FOCUS_IN                "FCSI"              //Common_Tim_0325, mod,
#define CMD_STR_FOCUS_OUT               "FCSO"              //Common_Tim_0325, mod,
#define CMD_STR_LENS_SHIFT_U            "LSVU"              //Common_Tim_0325, mod,
#define CMD_STR_LENS_SHIFT_D            "LSVD"              //Common_Tim_0325, mod,
#define CMD_STR_LENS_SHIFT_L            "LSHL"              //Common_Tim_0325, mod,
#define CMD_STR_LENS_SHIFT_R            "LSHR"              //Common_Tim_0325, mod,
#define CMD_STR_LENS_LOCK               "LCBG+LOCK"   //B35LB_David_0033 Mod
#define CMD_STR_LENS_CAL                "LCBG+HOME"   //B35LB_David_0033 Mod
#define CMD_STR_MENU_HORZ               CMD_STR_NONE    // Menu position Horizontal
#define CMD_STR_MENU_VERT               CMD_STR_NONE    // Menu Position Vertical
#define CMD_STR_SHOW_MSG                "MBEG+USER"   //B35LB_David_0033 Mod
#define CMD_STR_MENU_TRANSPARENCY       "MNTP"
#define CMD_STR_BG_COLOR                "SPSG+SLCT"   //B35LB_David_0033 Mod      //Background Color
#define CMD_STR_PIN_PROTECT             "PINP"
#define CMD_STR_CHG_PIN                 "PINC"
#define CMD_STR_STANBY_MODE             "POWR+STBM"      //B35LB_Tim_0022, mod, for DVT only "SBPM"//B35LB_David_0029 Add
#define CMD_STR_AC_PWR_ON               "DPON"          //Direct Power On
#define CMD_STR_AUTO_SHUTDOWN           "APOF"          //Auto Power Off
#define CMD_STR_SLEEP_TIMER             "SLTM"
#define CMD_STR_UART_BAUD_RATE          "SPBR"
#define CMD_STR_UART_ECHO               "SPEC"
#define CMD_STR_FACTORY_RESET           "HGRT"         //B35LB_David_0033 Mod Start
#define CMD_STR_CCA_MT_EN               "CCAG+MTRA"    // Color matching meter adj. enable
#define CMD_STR_CCA_MT_MD_R_INT         "CCAG+RDMI"    // Color matching meter adj. measured data Red intensity
#define CMD_STR_CCA_MT_MD_R_X           "CCAG+RDMX"    // Color matching meter adj. measured data Red X
#define CMD_STR_CCA_MT_MD_R_Y           "CCAG+RDMY"    // Color matching meter adj. measured data Red Y
#define CMD_STR_CCA_MT_MD_G_INT         "CCAG+GNMI"    // Color matching meter adj. measured data Green intensity
#define CMD_STR_CCA_MT_MD_G_X           "CCAG+GNMX"    // Color matching meter adj. measured data Green X
#define CMD_STR_CCA_MT_MD_G_Y           "CCAG+GNMY"    // Color matching meter adj. measured data Green Y
#define CMD_STR_CCA_MT_MD_B_INT         "CCAG+BLMI"    // Color matching meter adj. measured data Blue intensity
#define CMD_STR_CCA_MT_MD_B_X           "CCAG+BLMX"    // Color matching meter adj. measured data Blue X
#define CMD_STR_CCA_MT_MD_B_Y           "CCAG+BLMY"    // Color matching meter adj. measured data Blue Y
#define CMD_STR_CCA_MT_MD_W_INT         "CCAG+WHMI"    // Color matching meter adj. measured data White intensity
#define CMD_STR_CCA_MT_MD_W_X           "CCAG+WHMX"    // Color matching meter adj. measured data White X
#define CMD_STR_CCA_MT_MD_W_Y           "CCAG+WHMY"    // Color matching meter adj. measured data White Y
#define CMD_STR_CCA_MT_TD_R_GAIN        "CCAG+RDDG"    // Color matching meter adj. target data Red Gain
#define CMD_STR_CCA_MT_TD_R_X           "CCAG+RDDX"    // Color matching meter adj. target data Red X
#define CMD_STR_CCA_MT_TD_R_Y           "CCAG+RDDY"    // Color matching meter adj. target data Red Y
#define CMD_STR_CCA_MT_TD_G_GAIN        "CCAG+GNDG"    // Color matching meter adj. target data Green Gain
#define CMD_STR_CCA_MT_TD_G_X           "CCAG+GNDX"    // Color matching meter adj. target data Green X
#define CMD_STR_CCA_MT_TD_G_Y           "CCAG+GNDY"    // Color matching meter adj. target data Green Y
#define CMD_STR_CCA_MT_TD_B_GAIN        "CCAG+BLDG"    // Color matching meter adj. target data Blue Gain
#define CMD_STR_CCA_MT_TD_B_X           "CCAG+BLDX"    // Color matching meter adj. target data Blue X
#define CMD_STR_CCA_MT_TD_B_Y           "CCAG+BLDY"    // Color matching meter adj. target data Blue Y
#define CMD_STR_CCA_MT_TD_W_GAIN        "CCA+WHDG"    // Color matching meter adj. target data White Gain
#define CMD_STR_CCA_MT_TD_W_X           "CCAG+WHDX"    // Color matching meter adj. target data White X
#define CMD_STR_CCA_MT_TD_W_Y           "CCAG+WHDY"    // Color matching meter adj. target data White Y
#define CMD_STR_CCA_MT_TEST_PAT         "CCAG+MTTP"    // Color matching meter adj. target data test pattern
#define CMD_STR_CCA_MN_EN               "CCAG+MANA"
#define CMD_STR_CCA_MN_TEST_PAT         "CCAG+MNTP"
#define CMD_STR_CCA_MN_RPR              "CCAG+ROFR"
#define CMD_STR_CCA_MN_GPR              "CCAG+GOFR"
#define CMD_STR_CCA_MN_BPR              "CCAG+BOFR"
#define CMD_STR_CCA_MN_GPG              "CCAG+GOFG"
#define CMD_STR_CCA_MN_RPG              "CCAG+ROFG"
#define CMD_STR_CCA_MN_BPG              "CCAG+BOFG"
#define CMD_STR_CCA_MN_BPB              "CCAG+BOFB"
#define CMD_STR_CCA_MN_RPB              "CCAG+ROFB"
#define CMD_STR_CCA_MN_GPB              "CCAG+GOFB"
#define CMD_STR_CCA_MN_RPW              "CCAG+ROFW"
#define CMD_STR_CCA_MN_GPW              "CCAG+GOFW"
#define CMD_STR_CCA_MN_BPW              "CCAG+BOFW"   
#define CMD_STR_PROJ_ADDR               "PJAD"              //Common_Tim_0325, mod, CMD_STR_NONE
#define CMD_STR_CCA_MT_APPLY            "CCAG+CLMA"    //B35LB_David_0033 Mod End// Color matching meter adj. apply
#define CMD_STR_CW_IDX_2X               "CWI2"              //Common_Tim_0325, mod, CMD_STR_NONE // Color Wheel Index 2x
#define CMD_STR_CW_IDX_3X               "CWI3"              //Common_Tim_0325, mod, CMD_STR_NONE  // Color Wheel Index 3x
#define CMD_STR_CCA_MT_MD_RESET         CMD_STR_NONE    // Color matching meter adj. measured data reset
#define CMD_STR_CCA_MT_TD_RESET         CMD_STR_NONE    // Color matching meter adj. target data reset
#define CMD_STR_CCA_MN_RESET            "CMRS"
#define CMD_STR_PHOSPHER_W_IDX          "PHWI"              //Common_Tim_0325, mod, CMD_STR_NONE
#define CMD_STR_FILTER_W_IDX            "FIWI"              //Common_Tim_0325, mod, CMD_STR_NONE
#define CMD_STR_LENS_MEM_APPLY          "LMAP"              //Common_Tim_0325, mod, CMD_STR_NONE
#define CMD_STR_LENS_MEM_SAVE           "LMSA"              //Common_Tim_0325, mod, CMD_STR_NONE
#define CMD_STR_KP_BACK_LIGHT           "KLED"
#define CMD_STR_STATUS_LED              "SLED"    // Status LED on/off//B35LB_David_0029 Add
#define CMD_STR_UART_PATH               "SPPA"
//A70LH_sc_0002, add start
#define CMD_STR_IR_CTRL_TOP             "IRCG+TOPP"   //B35LB_David_0033 Mod
#define CMD_STR_IR_CTRL_FRONT           "IRCG+FRNT"   //B35LB_David_0033 Mod
#define CMD_STR_IR_CTRL_HDBASET         "IRCG+HDBT"   //B35LB_David_0033 Mod
//A70LH_sc_0002, add end
//A70LH_sc_0013, add start
#define CMD_STR_TC_PI3X                 CMD_STR_NONE
#define CMD_STR_TC_FI3X                 CMD_STR_NONE
//A70LH_sc_0013, add end
#define CMD_STR_TC_WallCollor           "CCAG+WALL"    //A70LH_sc_0016, add//B35LB_David_0033 Mod
#define CMD_STR_TC_INS_PWR_OFF          CMD_STR_NONE    //B25H+_sc_0070, mod //A70LH_sc_0068, add
#define CMD_STR_TC_HDMI_EQ              "HEQG"    //A35LB_sc_0234, add//B35LB_David_0029 Add
#define CMD_STR_TC_LOGO_CAPTURE         "LCAG"    //A35LB_sc_0234, add//B35LB_David_0029 Add
#define CMD_STR_TC_MENU_LOCATION        "MELG"          //A35LB_sc_0234, add//B35LB_David_0029 Add

//A70LH_sc_0025, add start
#define CMD_STR_HSG_En                  "CCAG+HSGE"   //B35LB_David_0033 Mod Start
#define CMD_STR_HSG_ATP                 "CCAG+MHTP"
#define CMD_STR_HSG_RH                  "CCAG+REDH"
#define CMD_STR_HSG_RS                  "CCAG+REDS"
#define CMD_STR_HSG_RG                  "CCAG+REDG"
#define CMD_STR_HSG_GH                  "CCAG+GREH"
#define CMD_STR_HSG_GS                  "CCAG+GRES"
#define CMD_STR_HSG_GG                  "CCAG+GREG"
#define CMD_STR_HSG_BH                  "CCAG+BLUH"
#define CMD_STR_HSG_BS                  "CCAG+BLUS"
#define CMD_STR_HSG_BG                  "CCAG+BLUG"
#define CMD_STR_HSG_CH                  "CCAG+CYAH"
#define CMD_STR_HSG_CS                  "CCAG+CYAS"
#define CMD_STR_HSG_CG                  "CCAG+CYAG"
#define CMD_STR_HSG_MH                  "CCAG+MAGH"
#define CMD_STR_HSG_MS                  "CCAG+MAGS"
#define CMD_STR_HSG_MG                  "CCAG+MAGG"
#define CMD_STR_HSG_YH                  "CCAG+YELH"
#define CMD_STR_HSG_YS                  "CCAG+YELS"
#define CMD_STR_HSG_YG                  "CCAG+YELG"
#define CMD_STR_HSG_WR                  "CCAG+WHRG"
#define CMD_STR_HSG_WG                  "CCAG+WHGG"
#define CMD_STR_HSG_WB                  "CCAG+WHBG"   //B35LB_David_0033 Mod End
#define CMD_STR_HSG_RTD                 "HIAL"//B35LB_David_0029 Add
#define CMD_STR_HSG_CE                  CMD_STR_NONE
//A70LH_sc_0025, add end

#define CMD_STR_MS_LD1                  "LDIG+LD01?" //A70LH_sc_0029, add start //B35LB_David_0033 Mod Start
#define CMD_STR_MS_LD2                  "LDIG+LD02?"
#define CMD_STR_MS_LD3                  "LDIG+LD03?"
#define CMD_STR_MS_LD4                  "LDIG+LD04?"
#define CMD_STR_MS_LD5                  "LDIG+LD05?"
#define CMD_STR_MS_LD6                  "LDIG+LD06?"
#define CMD_STR_MS_LD7                  "LDIG+LD07?"
#define CMD_STR_MS_LD8                  "LDIG+LD08?"
#define CMD_STR_MS_LD9                  "LDIG+LD09?"
#define CMD_STR_MS_LD10                 "LDIG+LD10?"
#define CMD_STR_MS_LD11                 "LDIG+LD11?"
#define CMD_STR_MS_LD12                 "LDIG+LD12?"
#define CMD_STR_MS_LD13                 "LDIG+LD13?" //A70LH_sc_0029, add end //B35LB_David_0033 Mod End

#define CMD_STR_TIME_DETECT_MODE        CMD_STR_NONE    // Timing Detect Mode
#define CMD_STR_SRC_HOT_KEY_EN          CMD_STR_NONE    // Hot key enable
#define CMD_STR_SRC_CHANGE_MODE         "ASRC"  //Auto Source
//Common_Tim_0331, mov, *&
#define CMD_STR_BLANK_ON_SRC_SW         CMD_STR_NONE    // Blank on Signal Switch

#define CMD_STR_NET_SHOW_MSG            "NETG+SHOW"         // Show Network Message //Common_Tim_0365, mod, CMD_STR_NONE //B35LB_David_0033 Mod
#define CMD_STR_NET_RESTART             "NETG+RSTR"         //B35LB_David_0033 Mod
#define CMD_STR_NET_F_RESET             "NETG+RSET"         //B35LB_Kraz_0118, add

#define CMD_STR_NET_SRC_READY_C         CMD_STR_NONE    // Source Card Reader is ready
#define CMD_STR_NET_SRC_READY_M         CMD_STR_NONE    // Source Mini USB is ready
#define CMD_STR_NET_SRC_READY_P         "NETP"          //Common_Tim_0325, mod, CMD_STR_NONE

#define CMD_STR_TRISTATE                "TRIS"          //Common_Tim_0325, mod, CMD_STR_NONE

//Common_Tim_0325, mov, *&
#define CMD_STR_CRE_LP_1_HOURS          "LIFG+LSHS"    //B35LB_David_0033 Mod Start
#define CMD_STR_CRE_LP_2_HOURS          "LIFG+REDL"
#define CMD_STR_CRE_LP_T_HOURS          "LIFG+TPHS"              //Common_Kraz_0217, add, Total Projector Hour
#define CMD_STR_CRE_LC_HOURS            "LIFG+LCHS"    //B35LB_David_0033 Mod End
#define CMD_STR_CRE_RESOLUTION          "NAR"               //B35LB_Kraz_0095, mod, "MSRS"
#define CMD_STR_CRE_SYS_TEMP            "STMP"


//B35LB_Kraz_0108, mod //B35LB_Kraz_0089, mod : start
#define CMD_STR_CRE_CMD_MENU            "KEYG19"             //B35LB_Tim_0011, add, start
#define CMD_STR_CRE_CMD_UP              "KEYG38"
#define CMD_STR_CRE_CMD_DOWN            "KEYG42"
#define CMD_STR_CRE_CMD_LEFT            "KEYG39"
#define CMD_STR_CRE_CMD_RIGHT           "KEYG41"
#define CMD_STR_CRE_CMD_ENTER           "KEYG40"
#define CMD_STR_CRE_CMD_INPUT           "KEYG48"
#define CMD_STR_CRE_CMD_EXIT            "KEYG20"
#define CMD_STR_CRE_CMD_RESYNC          "AIM1"              //B35LB_Tim_0011, add, end

/*                                                          //B35LB_Tim_0011, mark, start
#ifdef CUSTOMER_RICOH
#define CMD_STR_CRE_CMD_MENU            "KEYG27"
#define CMD_STR_CRE_CMD_UP              "KEYG64"
#define CMD_STR_CRE_CMD_DOWN            "KEYG65"
#define CMD_STR_CRE_CMD_LEFT            "KEYG66"
#define CMD_STR_CRE_CMD_RIGHT           "KEYG67"
#define CMD_STR_CRE_CMD_ENTER           "KEYG19"
#define CMD_STR_CRE_CMD_INPUT           "KEYG20"
#define CMD_STR_CRE_CMD_EXIT            "KEYG29"
#elif defined(CUSTOMER_SONNOC)
#define CMD_STR_CRE_CMD_MENU            "KEYG27"
#define CMD_STR_CRE_CMD_UP              "KEYG64"
#define CMD_STR_CRE_CMD_DOWN            "KEYG65"
#define CMD_STR_CRE_CMD_LEFT            "KEYG66"
#define CMD_STR_CRE_CMD_RIGHT           "KEYG67"
#define CMD_STR_CRE_CMD_ENTER           "KEYG19"
#define CMD_STR_CRE_CMD_INPUT           "KEYG20"
#define CMD_STR_CRE_CMD_EXIT            "KEYG29"
#elif defined(CUSTOMER_OPTOMA) || defined(_B35LB_OPTOMA_) //B35LB_Kraz_0108, mod
#define CMD_STR_CRE_CMD_MENU            "KEYG14"
#define CMD_STR_CRE_CMD_UP              "KEYG15"
#define CMD_STR_CRE_CMD_DOWN            "KEYG18"
#define CMD_STR_CRE_CMD_LEFT            "KEYG17"
#define CMD_STR_CRE_CMD_RIGHT           "KEYG16"
#define CMD_STR_CRE_CMD_ENTER           "KEYG20"
#define CMD_STR_CRE_CMD_INPUT           "KEYG24"
#define CMD_STR_CRE_CMD_EXIT            "KEYG42"
#elif defined(CUSTOMER_EIKI) || defined(__DEMO_EIKI_) //B35LB_Kraz_0108, mod
#define CMD_STR_CRE_CMD_MENU            "KEYG21"
#define CMD_STR_CRE_CMD_UP              "KEYG16"
#define CMD_STR_CRE_CMD_DOWN            "KEYG20"
#define CMD_STR_CRE_CMD_LEFT            "KEYG17"
#define CMD_STR_CRE_CMD_RIGHT           "KEYG19"
#define CMD_STR_CRE_CMD_ENTER           "KEYG18"
#define CMD_STR_CRE_CMD_INPUT           "KEYG15"
#define CMD_STR_CRE_CMD_EXIT            "KEYG22"
#endif
*/                                                          //B35LB_Tim_0011, mark, end
//B35LB_Kraz_0089, mod : end


#endif
//different command string &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&

//Command string &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&

// Command parameter length ****************************************************

//common command string
#define CMD_DATA_LEN_NULL               (0)

#define CMD_DATA_LEN_MAIN_SRC           (2)
#define CMD_DATA_LEN_SEC_SRC            (2)
#define CMD_DATA_LEN_PIP_PBP            (1)
#define CMD_DATA_LEN_LAYOUT             (1)
#define CMD_DATA_LEN_PIP_SIZE           (1)
#define CMD_DATA_LEN_PIP_SWAP           (1)
#define CMD_DATA_LEN_TEST_PAT           (2)
#define CMD_DATA_LEN_PWR_SW             (1)
#define CMD_DATA_LEN_IMG_MUTE           (1)
#define CMD_DATA_LEN_OSD_SW             (1)

#define CMD_DATA_LEN_ASPECT_R           (1)
#define CMD_DATA_LEN_OVERSCAN           (1)
#define CMD_DATA_LEN_AUTO_IMG           (1)
#define CMD_DATA_LEN_PIXEL_TRACK        (3)
#define CMD_DATA_LEN_PIXEL_PHASE        (3)
#define CMD_DATA_LEN_HORZ_P             (3)
#define CMD_DATA_LEN_VERT_P             (3)
#define CMD_DATA_LEN_D_ZOOM_H           (3)
#define CMD_DATA_LEN_D_ZOOM_V           (3)
#define CMD_DATA_LEN_D_SHIFT_H          (3)
#define CMD_DATA_LEN_D_SHIFT_V          (3)
#define CMD_DATA_LEN_H_KEYSTONE         (3)
#define CMD_DATA_LEN_V_KEYSTONE         (3)
#define CMD_DATA_LEN_H_PINCUSHION       (3)
#define CMD_DATA_LEN_V_PINCUSHION       (3)
#define CMD_DATA_LEN_PC_MODE            (1)
#define CMD_DATA_LEN_4CTLH              (3)
#define CMD_DATA_LEN_4CTLV              (3)
#define CMD_DATA_LEN_4CTRH              (3)
#define CMD_DATA_LEN_4CTRV              (3)
#define CMD_DATA_LEN_4CBLH              (3)
#define CMD_DATA_LEN_4CBLV              (3)
#define CMD_DATA_LEN_4CBRH              (3)
#define CMD_DATA_LEN_4CBRV              (3)
#define CMD_DATA_LEN_CRV_TA             (3)
#define CMD_DATA_LEN_CRV_BA             (3)
#define CMD_DATA_LEN_CRV_LA             (3)
#define CMD_DATA_LEN_CRV_RA             (3)
#define CMD_DATA_LEN_ROTATION           (2)
#define CMD_DATA_LEN_PIN_BAR            (2)
#define CMD_DATA_LEN_GEO_RST            (1)
#define CMD_DATA_LEN_AWF                (1)                 //Christie_Tim_0490, add, ***
#define CMD_DATA_LEN_MWF_HF             (2)
#define CMD_DATA_LEN_MWF_VF             (2)                 //Christie_Tim_0490, add, &&&
#define CMD_DATA_LEN_4C_TLHA_N          (1)                 //A35LB_Tim_0529, add, ***
#define CMD_DATA_LEN_4C_TLHA_P          (1)
#define CMD_DATA_LEN_4C_TLVA_N          (1)
#define CMD_DATA_LEN_4C_TLVA_P          (1)
#define CMD_DATA_LEN_4C_TRHA_N          (1)
#define CMD_DATA_LEN_4C_TRHA_P          (1)
#define CMD_DATA_LEN_4C_TRVA_N          (1)
#define CMD_DATA_LEN_4C_TRVA_P          (1)
#define CMD_DATA_LEN_4C_BLHA_N          (1)
#define CMD_DATA_LEN_4C_BLHA_P          (1)
#define CMD_DATA_LEN_4C_BLVA_N          (1)
#define CMD_DATA_LEN_4C_BLVA_P          (1)
#define CMD_DATA_LEN_4C_BRHA_N          (1)
#define CMD_DATA_LEN_4C_BRHA_P          (1)
#define CMD_DATA_LEN_4C_BRVA_N          (1)
#define CMD_DATA_LEN_4C_BRVA_P          (1)                 //A35LB_Tim_0529, add, &&&

#define CMD_DATA_LEN_LP_PWR_SET         (3)            //A70LH_sc_0009, mod 2->3
#define CMD_DATA_LEN_LP_CUR_LAMP        (1)
#define CMD_DATA_LEN_LP_ECO_MODE        (1)
#define CMD_DATA_LEN_LP_AUTO_SW         (1)
#define CMD_DATA_LEN_LP_AUTO_SW_TIME    (4)
#define CMD_DATA_LEN_LP_LIFT_SET        (5)            //A70LH_sc_0019, mod 4->5
#define CMD_DATA_LEN_LP_RST_1           (1)
#define CMD_DATA_LEN_LP_RST_2           (1)
#define CMD_DATA_LEN_LP_MODE            (1)
#define CMD_DATA_LEN_LP_INT_SET         (3)            //A70LH_sc_0009, mod 2->3
#define CMD_DATA_LEN_LP_SENSOR_CAL      (1)
//A70LH_sc_0010, add start
#define CMD_DATA_LEN_LP_ConstIntMode    (1)
#define CMD_DATA_LEN_LP_RentalMode      (1)
//A70LH_sc_0010, add end
#define CMD_DATA_LEN_LS_RLD_CAL         (1)                 //Christie_Tim_0452, add

#define CMD_DATA_LEN_BRIGHTNESS         (3)
#define CMD_DATA_LEN_CONTRAST           (3)
#define CMD_DATA_LEN_COLOR_SPACE        (1)
#define CMD_DATA_LEN_DETAIL             (2)             //A35LB_sc_0234, mod 1
#define CMD_DATA_LEN_PICTURE_SET        (2)             //Common_Tim_0325, mod, 1
#define CMD_DATA_LEN_SAVE_TO_USER       (1)
#define CMD_DATA_LEN_USER_APPLY_SET     (2)             //G100_Steven_0030
#define CMD_DATA_LEN_DYNAMIC_BLACK      (1)
#define CMD_DATA_LEN_COLOR              (3)
#define CMD_DATA_LEN_TINT               (3)
#define CMD_DATA_LEN_NOISE_REDUCE       (3)
#define CMD_DATA_LEN_FLESH_TONE         (3)
#define CMD_DATA_LEN_VIDEO_BLACK_LVL    (1)
#define CMD_DATA_LEN_DETECT_FILM        (1)
#define CMD_DATA_LEN_CLOSED_CAPTIONS    (1)
#define CMD_DATA_LEN_RED_GAIN           (3)
#define CMD_DATA_LEN_GREEN_GAIN         (3)
#define CMD_DATA_LEN_BLUE_GAIN          (3)
#define CMD_DATA_LEN_RED_OFFSET         (3)
#define CMD_DATA_LEN_GREEN_OFFSET       (3)
#define CMD_DATA_LEN_BLUE_OFFSET        (3)
#define CMD_DATA_LEN_SYNC_THRESHOLD     (3)
#define CMD_DATA_LEN_RESET_RGB_G_O      (1)
#define CMD_DATA_LEN_GAMMA              (1)
#define CMD_DATA_LEN_BRILLIANT_COLOR    (1)
#define CMD_DATA_LEN_COLOR_TEMP         (1)
#define CMD_DATA_LEN_EDGE_ENHANCE       (1)
#define CMD_DATA_LEN_WHITE_PEAKING      (3)
#define CMD_DATA_LEN_IMG_FREEZE         (1)
#define CMD_DATA_LEN_COLOR_WHEEL_SPD    (1)
#define CMD_DATA_LEN_COLOR_ENHANCE      (1)
#define CMD_DATA_LEN_3D_ENABLE          (1)
#define CMD_DATA_LEN_3D_INVERT          (1)
#define CMD_DATA_LEN_TI_T3B             (1) //A70LH_sc_0027, add
#define CMD_DATA_LEN_TI_REAL_BLACK      (1) //A70LH_sc_0036, add start
#define CMD_DATA_LEN_TI_FRAME_PACK      (1)
#define CMD_DATA_LEN_TI_S_BY_S          (1)
#define CMD_DATA_LEN_TI_T_AND_B         (1)
#define CMD_DATA_LEN_TI_FRAME_SEQ       (1) //A70LH_sc_0036, add end
#define CMD_DATA_LEN_TI_S_T_U           (1) //A70LH_sc_0091, add
#define CMD_DATA_LEN_TI_3D_SYNC_OUT     (1) //Christie_Tim_0476, add
#define CMD_DATA_LEN_TI_FRAME_DELAY     (3) //Christie_Tim_0477, add
#define CMD_DATA_LEN_TI_3D              (1) //A35LB_sc_0234, add
#define CMD_DATA_LEN_TI_1080P24         (1) //A35LB_sc_0234, add

#if (B35LH_GENERIC)                         //B35LH_sc_0108, mod start
    #define CMD_DATA_LEN_LANGUAGE           (2)
#else
    #define CMD_DATA_LEN_LANGUAGE           (1)
#endif                                      //B35LH_sc_0108, mod end
#define CMD_DATA_LEN_CEIL_MOUNT         (1)
#define CMD_DATA_LEN_REAR_PROJ          (1)
#define CMD_DATA_LEN_HIGH_ALTITUDE      (1)
#define CMD_DATA_LEN_12V_TRIGGER        (1)  //A70LH_sc_0001, add
#define CMD_DATA_LEN_HOT_KEY_SET        (1)
#define CMD_DATA_LEN_ZOOM_UP            (1)
#define CMD_DATA_LEN_ZOOM_DOWN          (1)
#define CMD_DATA_LEN_FOCUS_IN           (1)
#define CMD_DATA_LEN_FOCUS_OUT          (1)
#define CMD_DATA_LEN_LENS_SHIFT_U       (1)
#define CMD_DATA_LEN_LENS_SHIFT_D       (1)
#define CMD_DATA_LEN_LENS_SHIFT_L       (1)
#define CMD_DATA_LEN_LENS_SHIFT_R       (1)
#define CMD_DATA_LEN_LENS_LOCK          (1)
#define CMD_DATA_LEN_LENS_CAL           (1)
#define CMD_DATA_LEN_MENU_HORZ          (3)
#define CMD_DATA_LEN_MENU_VERT          (3)
#define CMD_DATA_LEN_SHOW_MSG           (1)
#define CMD_DATA_LEN_MENU_TRANSPARENCY  (2)
#define CMD_DATA_LEN_BG_COLOR           (1)
#define CMD_DATA_LEN_PIN_PROTECT        (7)       //shall be checked again
#define CMD_DATA_LEN_CHG_PIN            (13)       //shall be checked again
#define CMD_DATA_LEN_STANBY_MODE        (1)
#define CMD_DATA_LEN_AC_PWR_ON          (1)
#define CMD_DATA_LEN_AUTO_SHUTDOWN      (3)       //A35LB_sc_0234, mod 1
#define CMD_DATA_LEN_SLEEP_TIMER        (3)       //A35LB_sc_0234, mod 1
#define CMD_DATA_LEN_UART_BAUD_RATE     (1)
#define CMD_DATA_LEN_UART_ECHO          (1)
#define CMD_DATA_LEN_FACTORY_RESET      (1)
#define CMD_DATA_LEN_CCA_MT_EN          (1)
#define CMD_DATA_LEN_CCA_MT_MD_R_INT    (5)
#define CMD_DATA_LEN_CCA_MT_MD_R_X      (3)
#define CMD_DATA_LEN_CCA_MT_MD_R_Y      (3)
#define CMD_DATA_LEN_CCA_MT_MD_G_INT    (5)
#define CMD_DATA_LEN_CCA_MT_MD_G_X      (3)
#define CMD_DATA_LEN_CCA_MT_MD_G_Y      (3)
#define CMD_DATA_LEN_MT_MD_B_INT        (5)
#define CMD_DATA_LEN_MT_MD_B_X          (3)
#define CMD_DATA_LEN_MT_MD_B_Y          (3)
#define CMD_DATA_LEN_MT_MD_W_INT        (5)
#define CMD_DATA_LEN_CCA_MT_MD_W_X      (3)
#define CMD_DATA_LEN_CCA_MT_MD_W_Y      (3)
#define CMD_DATA_LEN_CCA_MT_TD_R_GAIN   (4)
#define CMD_DATA_LEN_CCA_MT_TD_R_X      (3)
#define CMD_DATA_LEN_CCA_MT_TD_R_Y      (3)
#define CMD_DATA_LEN_CCA_MT_TD_G_GAIN   (4)
#define CMD_DATA_LEN_CCA_MT_TD_G_X      (3)
#define CMD_DATA_LEN_CCA_MT_TD_G_Y      (3)
#define CMD_DATA_LEN_CCA_MT_TD_B_GAIN   (4)
#define CMD_DATA_LEN_CCA_MT_TD_B_X      (3)
#define CMD_DATA_LEN_CCA_MT_TD_B_Y      (3)
#define CMD_DATA_LEN_CCA_MT_TD_W_GAIN   (4)
#define CMD_DATA_LEN_CCA_MT_TD_W_X      (3)
#define CMD_DATA_LEN_CCA_MT_TD_W_Y      (3)
#define CMD_DATA_LEN_CCA_MT_TEST_PAT    (1)
#define CMD_DATA_LEN_CCA_MN_EN          (1)
#define CMD_DATA_LEN_CCA_MN_TEST_PAT    (1)
#define CMD_DATA_LEN_CCA_MN_RPR         (4)
#define CMD_DATA_LEN_CCA_MN_GPR         (4)
#define CMD_DATA_LEN_CCA_MN_BPR         (4)
#define CMD_DATA_LEN_CCA_MN_GPG         (4)
#define CMD_DATA_LEN_CCA_MN_RPG         (4)
#define CMD_DATA_LEN_CCA_MN_BPG         (4)
#define CMD_DATA_LEN_CCA_MN_BPB         (4)
#define CMD_DATA_LEN_CCA_MN_RPB         (4)
#define CMD_DATA_LEN_CCA_MN_GPB         (4)
#define CMD_DATA_LEN_CCA_MN_RPW         (4)
#define CMD_DATA_LEN_CCA_MN_GPW         (4)
#define CMD_DATA_LEN_CCA_MN_BPW         (4)
#define CMD_DATA_LEN_PROJ_ADR           (2) //B35LB_David_0013 add
#define CMD_DATA_LEN_CCA_MT_APPLY       (1)
#define CMD_DATA_LEN_CW_IDX_2X          (3)
#define CMD_DATA_LEN_CW_IDX_3X          (3)
#define CMD_DATA_LEN_CCA_MT_MD_RESET    (1)
#define CMD_DATA_LEN_CCA_MT_TD_RESET    (1)
#define CMD_DATA_LEN_CCA_MN_RESET       (1)
#define CMD_DATA_LEN_PHOSPHER_W_IDX     (3)
#define CMD_DATA_LEN_FILTER_W_IDX       (3)
#define CMD_DATA_LEN_LENS_MEM_APPLY     (1)
#define CMD_DATA_LEN_LENS_MEM_SAVE      (1)
#define CMD_DATA_LEN_KP_BACK_LIGHT      (1)
#define CMD_DATA_LEN_STATUS_LED         (1)
#define CMD_DATA_LEN_UART_PATH          (1)
//A70LH_sc_0002, add start
#define CMD_DATA_LEN_IR_CTRL_TOP        (1)
#define CMD_DATA_LEN_IR_CTRL_FRONT      (1)
#define CMD_DATA_LEN_IR_CTRL_HDBASET    (1)
//A70LH_sc_0002, add end
//A70LH_sc_0013, add start
#define CMD_DATA_LEN_TC_PI3X            (3)
#define CMD_DATA_LEN_TC_FI3X            (3)
//A70LH_sc_0013, add end
#define CMD_DATA_LEN_TC_WallCollor      (1)  //A70LH_sc_0016, add
#define CMD_DATA_LEN_TC_INS_PWR_OFF     (1)  //A70LH_sc_0068, add
#define CMD_DATA_LEN_TC_HDMI_EQ         (1)  //A35LB_sc_0234, add
#define CMD_DATA_LEN_TC_LOGO_CAPTURE    (1)  //A35LB_sc_0234, add
#define CMD_DATA_LEN_TC_MENU_LOCATION   (1)  //A35LB_sc_0234, add

//A70LH_sc_0025, add start
#define CMD_DATA_LEN_HSG_En             (1)
#define CMD_DATA_LEN_HSG_ATP            (1)
#define CMD_DATA_LEN_HSG_RH             (3)
#define CMD_DATA_LEN_HSG_RS             (3)
#define CMD_DATA_LEN_HSG_RG             (3)
#define CMD_DATA_LEN_HSG_GH             (3)
#define CMD_DATA_LEN_HSG_GS             (3)
#define CMD_DATA_LEN_HSG_GG             (3)
#define CMD_DATA_LEN_HSG_BH             (3)
#define CMD_DATA_LEN_HSG_BS             (3)
#define CMD_DATA_LEN_HSG_BG             (3)
#define CMD_DATA_LEN_HSG_CH             (3)
#define CMD_DATA_LEN_HSG_CS             (3)
#define CMD_DATA_LEN_HSG_CG             (3)
#define CMD_DATA_LEN_HSG_MH             (3)
#define CMD_DATA_LEN_HSG_MS             (3)
#define CMD_DATA_LEN_HSG_MG             (3)
#define CMD_DATA_LEN_HSG_YH             (3)
#define CMD_DATA_LEN_HSG_YS             (3)
#define CMD_DATA_LEN_HSG_YG             (3)
#define CMD_DATA_LEN_HSG_WR             (3)
#define CMD_DATA_LEN_HSG_WG             (3)
#define CMD_DATA_LEN_HSG_WB             (3)
#define CMD_DATA_LEN_HSG_RTD            (1)
#define CMD_DATA_LEN_HSG_CE             (1)
//A70LH_sc_0025, add end

#define CMD_DATA_LEN_TIME_DETECT_MODE   (1)
#define CMD_DATA_LEN_SRC_HOT_KEY_EN     (1)
#define CMD_DATA_LEN_SRC_CHANGE_MODE    (1)
#define CMD_DATA_LEN_SRC_HK_VGA         (2)
#define CMD_DATA_LEN_SRC_HK_BNC         (2)
#define CMD_DATA_LEN_SRC_HK_HDMI1       (2)
#define CMD_DATA_LEN_SRC_HK_HDMI2       (2)
#define CMD_DATA_LEN_SRC_HK_DVID        (2)
#define CMD_DATA_LEN_SRC_HK_DP          (2)
#define CMD_DATA_LEN_SRC_HK_COMPONENT   (2)
#define CMD_DATA_LEN_SRC_HK_SVIDEO      (2)
#define CMD_DATA_LEN_SRC_HK_COMPOSITE   (2)
#define CMD_DATA_LEN_SRC_HK_PRESENTER   (2)
#define CMD_DATA_LEN_SRC_HK_CARD_RD     (2)
#define CMD_DATA_LEN_SRC_HK_MINI_USB    (2)
#define CMD_DATA_LEN_SRC_HK_HDBASET     (2)
#define CMD_DATA_LEN_SRC_HK_3GSDI       (2)
#define CMD_DATA_LEN_BLANK_ON_SRC_SW    (1)

#define CMD_DATA_LEN_NET_SHOW_MSG       (1)
#define CMD_DATA_LEN_NET_RESTART        (1)


#define CMD_STR_D_LEN_CRE_WARM_TIMER    (2)
#define CMD_STR_D_LEN_CRE_COOL_TIMER    (2)
#define CMD_STR_D_LEN_CRE_LP_1_HOURS    (5)
#define CMD_STR_D_LEN_CRE_LP_2_HOURS    (5)
#define CMD_STR_D_LEN_CRE_LP_T_HOURS    (5)                 //Common_Kraz_0217, add, Total Projector Hour
#define CMD_STR_D_LEN_CRE_LC_HOURS      (5)                 //A70LH_Kraz_0272, add, TEC Life Hours
#define CMD_STR_D_LEN_CRE_RESOLUTION    (16)
#define CMD_STR_D_LEN_CRE_SYS_TEMP      (3)

#define CMD_DATA_LEN_NET_LAN_IP         (12)
#define CMD_DATA_LEN_NET_LAN_MASK       (12)
#define CMD_DATA_LEN_NET_LAN_GATEWAY    (12)
#define CMD_DATA_LEN_NET_PROJ_NAME      (45)
#define CMD_DATA_LEN_NET_LAN_MAC        (18)
#define CMD_DATA_LEN_NET_LAN_ALL        (38)
#define CMD_DATA_LEN_NET_FW_VER         (6)
#define CMD_DATA_LEN_NET_WLAN_EN        (1)
#define CMD_DATA_LEN_NET_WLAN_SIP       (12)
#define CMD_DATA_LEN_NET_WLAN_EIP       (12)
#define CMD_DATA_LEN_NET_WLAN_MASK      (12)
#define CMD_DATA_LEN_NET_WLAN_GATEWAY   (12)
#define CMD_DATA_LEN_NET_WLAN_MAC       (17)
#define CMD_DATA_LEN_NET_WLAN_ALL       (50)
#define CMD_DATA_LEN_NET_WLAN_PLUG_IN   (1)
#define CMD_DATA_LEN_NET_WLAN_SSID      (45)
#define CMD_DATA_LEN_NET_WLAN_KEY       (25)
#define CMD_DATA_LEN_PRESENTER_KEY      (45)

#define CMD_DATA_LEN_NET_SRC_READY_C    (1)
#define CMD_DATA_LEN_NET_SRC_READY_M    (1)
#define CMD_DATA_LEN_NET_SRC_READY_P    (1)

#define CMD_DATA_LEN_NET_TFTP_UPG       (24)

#define CMD_DATA_LEN_TRISTATE           (2)

#define CMD_DATA_LEN_NET_SRC_STA        (3)
#define CMD_DATA_LEN_INP_PW_RAT         (1) //A70LH_sc_0042, add

// Command parameter length &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&*
#define CMD_LEN_NULL                    (1)//B35LB_David_0018 Mod Start
#define CMD_LEN_NONE                    (CMD_FIX_LEN + sizeof(CMD_STR_NONE)             + CMD_LEN_NULL-CMD_LENGTH_ONE)

#define CMD_LEN_MAIN_SRC                (CMD_FIX_LEN + sizeof(CMD_STR_MAIN_SRC)         + CMD_DATA_LEN_MAIN_SRC-CMD_LENGTH_ONE)
#define CMD_LEN_SEC_SRC                 (CMD_FIX_LEN + sizeof(CMD_STR_SEC_SRC)          + CMD_DATA_LEN_SEC_SRC-CMD_LENGTH_ONE)
#define CMD_LEN_PIP_PBP                 (CMD_FIX_LEN + sizeof(CMD_STR_PIP_PBP)          + CMD_DATA_LEN_PIP_PBP-CMD_LENGTH_ONE)
#define CMD_LEN_LAYOUT                  (CMD_FIX_LEN + sizeof(CMD_STR_LAYOUT)           + CMD_DATA_LEN_LAYOUT-CMD_LENGTH_ONE)
#define CMD_LEN_PIP_SIZE                (CMD_FIX_LEN + sizeof(CMD_STR_PIP_SIZE)         + CMD_DATA_LEN_PIP_SIZE-CMD_LENGTH_ONE)
#define CMD_LEN_PIP_SWAP                (CMD_FIX_LEN + sizeof(CMD_STR_PIP_SWAP)         + CMD_DATA_LEN_PIP_SWAP-CMD_LENGTH_ONE)
#define CMD_LEN_TEST_PAT                (CMD_FIX_LEN + sizeof(CMD_STR_TEST_PAT)         + CMD_DATA_LEN_TEST_PAT-CMD_LENGTH_ONE)
#define CMD_LEN_PWR_SW                  (CMD_FIX_LEN + sizeof(CMD_STR_PWR_SW)           + CMD_DATA_LEN_PWR_SW-CMD_LENGTH_ONE)
#define CMD_LEN_IMG_MUTE                (CMD_FIX_LEN + sizeof(CMD_STR_IMG_MUTE)         + CMD_DATA_LEN_IMG_MUTE-CMD_LENGTH_ONE)
#define CMD_LEN_OSD_SW                  (CMD_FIX_LEN + sizeof(CMD_STR_OSD_SW)           + CMD_DATA_LEN_OSD_SW-CMD_LENGTH_ONE)
#define CMD_LEN_ALL_GET_MG              (CMD_FIX_LEN + sizeof(CMD_STR_ALL_GET_MG)       + CMD_DATA_LEN_NULL-CMD_LENGTH_ONE)
#define CMD_LEN_NET_SRC_STA             (CMD_FIX_LEN + sizeof(CMD_STR_NET_SRC_STA)      + CMD_DATA_LEN_NET_SRC_STA-CMD_LENGTH_ONE)
#define CMD_LEN_INP_PW_RAT              (CMD_FIX_LEN + sizeof(CMD_STR_INP_PW_RAT)       + CMD_DATA_LEN_INP_PW_RAT-CMD_LENGTH_ONE)  //A70LH_sc_0042, add

#define CMD_LEN_ASPECT_RATIO            (CMD_FIX_LEN + sizeof(CMD_STR_ASPECT_RATIO)     + CMD_DATA_LEN_ASPECT_R-CMD_LENGTH_ONE)
#define CMD_LEN_OVERSCAN                (CMD_FIX_LEN + sizeof(CMD_STR_OVERSCAN)         + CMD_DATA_LEN_OVERSCAN-CMD_LENGTH_ONE)
#define CMD_LEN_CURRENT_SRC             (6) //shall be checked !!!!
#define CMD_LEN_AUTO_IMG                (CMD_FIX_LEN + sizeof(CMD_STR_AUTO_IMG)         + CMD_DATA_LEN_AUTO_IMG-CMD_LENGTH_ONE)
#define CMD_LEN_PIXEL_TRACK             (CMD_FIX_LEN + sizeof(CMD_STR_PIXEL_TRACK)      + CMD_DATA_LEN_PIXEL_TRACK-CMD_LENGTH_ONE)
#define CMD_LEN_PIXEL_PHASE             (CMD_FIX_LEN + sizeof(CMD_STR_PIXEL_PHASE)      + CMD_DATA_LEN_PIXEL_PHASE-CMD_LENGTH_ONE)
#define CMD_LEN_HORZ_P                  (CMD_FIX_LEN + sizeof(CMD_STR_HORZ_P)           + CMD_DATA_LEN_HORZ_P-CMD_LENGTH_ONE)
#define CMD_LEN_VERT_P                  (CMD_FIX_LEN + sizeof(CMD_STR_VERT_P)           + CMD_DATA_LEN_VERT_P-CMD_LENGTH_ONE)
#define CMD_LEN_D_ZOOM_H                (CMD_FIX_LEN + sizeof(CMD_STR_D_ZOOM_H)         + CMD_DATA_LEN_D_ZOOM_H-CMD_LENGTH_ONE)
#define CMD_LEN_D_ZOOM_V                (CMD_FIX_LEN + sizeof(CMD_STR_D_ZOOM_V)         + CMD_DATA_LEN_D_ZOOM_V-CMD_LENGTH_ONE)
#define CMD_LEN_D_SHIFT_H               (CMD_FIX_LEN + sizeof(CMD_STR_D_SHIFT_H)        + CMD_DATA_LEN_D_SHIFT_H-CMD_LENGTH_ONE)
#define CMD_LEN_D_SHIFT_V               (CMD_FIX_LEN + sizeof(CMD_STR_D_SHIFT_V)        + CMD_DATA_LEN_D_SHIFT_V-CMD_LENGTH_ONE)
#define CMD_LEN_H_KEYSTONE              (CMD_FIX_LEN + sizeof(CMD_STR_H_KEYSTONE)       + CMD_DATA_LEN_H_KEYSTONE-CMD_LENGTH_ONE)
#define CMD_LEN_V_KEYSTONE              (CMD_FIX_LEN + sizeof(CMD_STR_V_KEYSTONE)       + CMD_DATA_LEN_V_KEYSTONE-CMD_LENGTH_ONE)
#define CMD_LEN_H_PINCUSHION            (CMD_FIX_LEN + sizeof(CMD_STR_H_PINCUSHION)     + CMD_DATA_LEN_H_PINCUSHION-CMD_LENGTH_ONE)
#define CMD_LEN_V_PINCUSHION            (CMD_FIX_LEN + sizeof(CMD_STR_V_PINCUSHION)     + CMD_DATA_LEN_V_PINCUSHION-CMD_LENGTH_ONE)
#define CMD_LEN_PC_MODE                 (CMD_FIX_LEN + sizeof(CMD_STR_PC_MODE)          + CMD_DATA_LEN_PC_MODE-CMD_LENGTH_ONE)
#define CMD_LEN_4CTLH                   (CMD_FIX_LEN + sizeof(CMD_STR_4CTLH)            + CMD_DATA_LEN_4CTLH-CMD_LENGTH_ONE)
#define CMD_LEN_4CTLV                   (CMD_FIX_LEN + sizeof(CMD_STR_4CTLV)            + CMD_DATA_LEN_4CTLV-CMD_LENGTH_ONE)
#define CMD_LEN_4CTRH                   (CMD_FIX_LEN + sizeof(CMD_STR_4CTRH)            + CMD_DATA_LEN_4CTRH-CMD_LENGTH_ONE)
#define CMD_LEN_4CTRV                   (CMD_FIX_LEN + sizeof(CMD_STR_4CTRV)            + CMD_DATA_LEN_4CTRV-CMD_LENGTH_ONE)
#define CMD_LEN_4CBLH                   (CMD_FIX_LEN + sizeof(CMD_STR_4CBLH)            + CMD_DATA_LEN_4CBLH-CMD_LENGTH_ONE)
#define CMD_LEN_4CBLV                   (CMD_FIX_LEN + sizeof(CMD_STR_4CBLV)            + CMD_DATA_LEN_4CBLV-CMD_LENGTH_ONE)
#define CMD_LEN_4CBRH                   (CMD_FIX_LEN + sizeof(CMD_STR_4CBRH)            + CMD_DATA_LEN_4CBRH-CMD_LENGTH_ONE)
#define CMD_LEN_4CBRV                   (CMD_FIX_LEN + sizeof(CMD_STR_4CBRV)            + CMD_DATA_LEN_4CBRV-CMD_LENGTH_ONE)
#define CMD_LEN_CRV_TA                  (CMD_FIX_LEN + sizeof(CMD_STR_CRV_TA)           + CMD_DATA_LEN_CRV_TA-CMD_LENGTH_ONE)
#define CMD_LEN_CRV_BA                  (CMD_FIX_LEN + sizeof(CMD_STR_CRV_BA)           + CMD_DATA_LEN_CRV_BA-CMD_LENGTH_ONE)
#define CMD_LEN_CRV_LA                  (CMD_FIX_LEN + sizeof(CMD_STR_CRV_LA)           + CMD_DATA_LEN_CRV_LA-CMD_LENGTH_ONE)
#define CMD_LEN_CRV_RA                  (CMD_FIX_LEN + sizeof(CMD_STR_CRV_RA)           + CMD_DATA_LEN_CRV_RA-CMD_LENGTH_ONE)
#define CMD_LEN_ROTATION                (CMD_FIX_LEN + sizeof(CMD_STR_ROTATION)         + CMD_DATA_LEN_ROTATION-CMD_LENGTH_ONE)
#define CMD_LEN_ALL_GET_TS              (CMD_FIX_LEN + sizeof(CMD_STR_ALL_GET_TS)       + CMD_DATA_LEN_NULL-CMD_LENGTH_ONE)
#define CMD_LEN_GEO_610_SEL1             (5)    //shall be checked !!!!
#define CMD_LEN_GEO_610_SEL2             (13)   //shall be checked !!!!
#define CMD_LEN_PIN_BAR                 (CMD_FIX_LEN + sizeof(CMD_STR_ROTATION)         + CMD_DATA_LEN_PIN_BAR-CMD_LENGTH_ONE)
#define CMD_LEN_GEO_RST                 (CMD_FIX_LEN + sizeof(CMD_STR_GEO_RST)          + CMD_DATA_LEN_GEO_RST-CMD_LENGTH_ONE)
#define CMD_LEN_TS_SPM                  (6)
#define CMD_LEN_TS_AWF                  (CMD_FIX_LEN + sizeof(CMD_STR_AWF)              + CMD_DATA_LEN_AWF-CMD_LENGTH_ONE)    //Christie_Tim_0490, add, ***
#define CMD_LEN_TS_MWF_HF               (CMD_FIX_LEN + sizeof(CMD_STR_MWF_HF)           + CMD_DATA_LEN_MWF_HF-CMD_LENGTH_ONE)
#define CMD_LEN_TS_MWF_VF               (CMD_FIX_LEN + sizeof(CMD_STR_MWF_VF)           + CMD_DATA_LEN_MWF_VF-CMD_LENGTH_ONE) //Christie_Tim_0490, add, &&&
#define CMD_LEN_4C_TLHA_N               (CMD_FIX_LEN + sizeof(CMD_STR_4C_TLHA_N)        + CMD_DATA_LEN_4C_TLHA_N-CMD_LENGTH_ONE)   //A35LB_Tim_0529, add, ***
#define CMD_LEN_4C_TLHA_P               (CMD_FIX_LEN + sizeof(CMD_STR_4C_TLHA_P)        + CMD_DATA_LEN_4C_TLHA_P-CMD_LENGTH_ONE)
#define CMD_LEN_4C_TLVA_N               (CMD_FIX_LEN + sizeof(CMD_STR_4C_TLVA_N)        + CMD_DATA_LEN_4C_TLVA_N-CMD_LENGTH_ONE)
#define CMD_LEN_4C_TLVA_P               (CMD_FIX_LEN + sizeof(CMD_STR_4C_TLVA_P)        + CMD_DATA_LEN_4C_TLVA_P-CMD_LENGTH_ONE)
#define CMD_LEN_4C_TRHA_N               (CMD_FIX_LEN + sizeof(CMD_STR_4C_TRHA_N)        + CMD_DATA_LEN_4C_TRHA_N-CMD_LENGTH_ONE)
#define CMD_LEN_4C_TRHA_P               (CMD_FIX_LEN + sizeof(CMD_STR_4C_TRHA_P)        + CMD_DATA_LEN_4C_TRHA_P-CMD_LENGTH_ONE)
#define CMD_LEN_4C_TRVA_N               (CMD_FIX_LEN + sizeof(CMD_STR_4C_TRVA_N)        + CMD_DATA_LEN_4C_TRVA_N-CMD_LENGTH_ONE)
#define CMD_LEN_4C_TRVA_P               (CMD_FIX_LEN + sizeof(CMD_STR_4C_TRVA_P)        + CMD_DATA_LEN_4C_TRVA_P-CMD_LENGTH_ONE)
#define CMD_LEN_4C_BLHA_N               (CMD_FIX_LEN + sizeof(CMD_STR_4C_BLHA_N)        + CMD_DATA_LEN_4C_BLHA_N-CMD_LENGTH_ONE)
#define CMD_LEN_4C_BLHA_P               (CMD_FIX_LEN + sizeof(CMD_STR_4C_BLHA_P)        + CMD_DATA_LEN_4C_BLHA_P-CMD_LENGTH_ONE)
#define CMD_LEN_4C_BLVA_N               (CMD_FIX_LEN + sizeof(CMD_STR_4C_BLVA_N)        + CMD_DATA_LEN_4C_BLVA_N-CMD_LENGTH_ONE)
#define CMD_LEN_4C_BLVA_P               (CMD_FIX_LEN + sizeof(CMD_STR_4C_BLVA_P)        + CMD_DATA_LEN_4C_BLVA_P-CMD_LENGTH_ONE)
#define CMD_LEN_4C_BRHA_N               (CMD_FIX_LEN + sizeof(CMD_STR_4C_BRHA_N)        + CMD_DATA_LEN_4C_BRHA_N-CMD_LENGTH_ONE)
#define CMD_LEN_4C_BRHA_P               (CMD_FIX_LEN + sizeof(CMD_STR_4C_BRHA_P)        + CMD_DATA_LEN_4C_BRHA_P-CMD_LENGTH_ONE)
#define CMD_LEN_4C_BRVA_N               (CMD_FIX_LEN + sizeof(CMD_STR_4C_BRVA_N)        + CMD_DATA_LEN_4C_BRVA_N-CMD_LENGTH_ONE)
#define CMD_LEN_4C_BRVA_P               (CMD_FIX_LEN + sizeof(CMD_STR_4C_BRVA_P)        + CMD_DATA_LEN_4C_BRVA_P-CMD_LENGTH_ONE)   //A35LB_Tim_0529, add, &&&


#define CMD_LEN_LP_PWR_SET              (CMD_FIX_LEN + sizeof(CMD_STR_LP_PWR_SET)       + CMD_DATA_LEN_LP_PWR_SET-CMD_LENGTH_ONE)
#define CMD_LEN_LP_CUR_LAMP             (CMD_FIX_LEN + sizeof(CMD_STR_LP_CUR_LAMP)      + CMD_DATA_LEN_LP_CUR_LAMP-CMD_LENGTH_ONE)
#define CMD_LEN_LP_ECO_MODE             (CMD_FIX_LEN + sizeof(CMD_STR_LP_ECO_MODE)      + CMD_DATA_LEN_LP_ECO_MODE-CMD_LENGTH_ONE)
#define CMD_LEN_LP_AUTO_SW              (CMD_FIX_LEN + sizeof(CMD_STR_LP_AUTO_SW)       + CMD_DATA_LEN_LP_AUTO_SW-CMD_LENGTH_ONE)
#define CMD_LEN_LP_AUTO_SW_TIME         (CMD_FIX_LEN + sizeof(CMD_STR_LP_AUTO_SW_TIME)  + CMD_DATA_LEN_LP_AUTO_SW_TIME-CMD_LENGTH_ONE)
#define CMD_LEN_LP_RST_2                (CMD_FIX_LEN + strlen(CMD_STR_LP_RST_2)         + CMD_DATA_LEN_LP_RST_2)
#define CMD_LEN_ALL_GET_TL              (CMD_FIX_LEN + strlen(CMD_STR_ALL_GET_TL)       + CMD_DATA_LEN_NULL)
#define CMD_LEN_LP_MODE                 (CMD_FIX_LEN + strlen(CMD_STR_LP_MODE)          + CMD_DATA_LEN_LP_MODE)
#define CMD_LEN_LP_INT_SET              (CMD_FIX_LEN + strlen(CMD_STR_LP_INT_SET)       + CMD_DATA_LEN_LP_INT_SET)
#define CMD_LEN_LP_SENSOR_CAL           (CMD_FIX_LEN + strlen(CMD_STR_LP_SENSOR_CAL)    + CMD_DATA_LEN_LP_SENSOR_CAL)
//A70LH_sc_0010, add start
#define CMD_LEN_LP_ConstIntMode         (CMD_FIX_LEN + strlen(CMD_STR_LP_ConstIntMode)  + CMD_DATA_LEN_LP_ConstIntMode)
#define CMD_LEN_LP_RentalMode           (CMD_FIX_LEN + strlen(CMD_STR_LP_RentalMode)    + CMD_DATA_LEN_LP_RentalMode)
//A70LH_sc_0010, add end
#define CMD_LEN_LS_RLD_CAL              (CMD_FIX_LEN + strlen(CMD_STR_LS_RLD_CAL)       + CMD_DATA_LEN_LS_RLD_CAL) //Christie_Tim_0452, add
//B35LB_David_0013 mod start
#define CMD_LEN_BRIGHTNESS              (CMD_FIX_LEN + sizeof(CMD_STR_BRIGHTNESS)       + CMD_DATA_LEN_BRIGHTNESS-CMD_LENGTH_ONE)
#define CMD_LEN_CONTRAST                (CMD_FIX_LEN + sizeof(CMD_STR_CONTRAST)         + CMD_DATA_LEN_CONTRAST-CMD_LENGTH_ONE)
#define CMD_LEN_COLOR_SPACE             (CMD_FIX_LEN + sizeof(CMD_STR_COLOR_SPACE)      + CMD_DATA_LEN_COLOR_SPACE-CMD_LENGTH_ONE)
#define CMD_LEN_DETAIL                  (CMD_FIX_LEN + sizeof(CMD_STR_DETAIL)           + CMD_DATA_LEN_DETAIL-CMD_LENGTH_ONE)
#define CMD_LEN_PICTURE_SET             (CMD_FIX_LEN + sizeof(CMD_STR_PICTURE_SET)      + CMD_DATA_LEN_PICTURE_SET-CMD_LENGTH_ONE)
#define CMD_LEN_SAVE_TO_USER            (CMD_FIX_LEN + sizeof(CMD_STR_SAVE_TO_USER)     + CMD_DATA_LEN_SAVE_TO_USER-CMD_LENGTH_ONE)
#define CMD_LEN_USER_MODE_APPLY         (CMD_FIX_LEN + sizeof(CMD_STR_USER_MODE_APPLY)  + CMD_DATA_LEN_USER_APPLY_SET-CMD_LENGTH_ONE) //G100_Steven_0030
#define CMD_LEN_DYNAMIC_BLACK           (CMD_FIX_LEN + sizeof(CMD_STR_DYNAMIC_BLACK)    + CMD_DATA_LEN_DYNAMIC_BLACK-CMD_LENGTH_ONE)
#define CMD_LEN_COLOR                   (CMD_FIX_LEN + sizeof(CMD_STR_COLOR)            + CMD_DATA_LEN_COLOR-CMD_LENGTH_ONE)
#define CMD_LEN_TINT                    (CMD_FIX_LEN + sizeof(CMD_STR_TINT)             + CMD_DATA_LEN_TINT-CMD_LENGTH_ONE)
#define CMD_LEN_NOISE_REDUCE            (CMD_FIX_LEN + sizeof(CMD_STR_NOISE_REDUCE)     + CMD_DATA_LEN_NOISE_REDUCE-CMD_LENGTH_ONE)
#define CMD_LEN_FLESH_TONE              (CMD_FIX_LEN + sizeof(CMD_STR_FLESH_TONE)       + CMD_DATA_LEN_FLESH_TONE-CMD_LENGTH_ONE)
#define CMD_LEN_VIDEO_BLACK_LVL         (CMD_FIX_LEN + sizeof(CMD_STR_VIDEO_BLACK_LVL)  + CMD_DATA_LEN_VIDEO_BLACK_LVL-CMD_LENGTH_ONE)
#define CMD_LEN_DETECT_FILM             (CMD_FIX_LEN + sizeof(CMD_STR_DETECT_FILM)      + CMD_DATA_LEN_DETECT_FILM-CMD_LENGTH_ONE)
#define CMD_LEN_CLOSED_CAPTIONS         (CMD_FIX_LEN + sizeof(CMD_STR_CLOSED_CAPTIONS)  + CMD_DATA_LEN_CLOSED_CAPTIONS-CMD_LENGTH_ONE)
#define CMD_LEN_RED_GAIN                (CMD_FIX_LEN + sizeof(CMD_STR_RED_GAIN)         + CMD_DATA_LEN_RED_GAIN-CMD_LENGTH_ONE)
#define CMD_LEN_GREEN_GAIN              (CMD_FIX_LEN + sizeof(CMD_STR_GREEN_GAIN)       + CMD_DATA_LEN_GREEN_GAIN-CMD_LENGTH_ONE)
#define CMD_LEN_BLUE_GAIN               (CMD_FIX_LEN + sizeof(CMD_STR_BLUE_GAIN)        + CMD_DATA_LEN_BLUE_GAIN-CMD_LENGTH_ONE)
#define CMD_LEN_RED_OFFSET              (CMD_FIX_LEN + sizeof(CMD_STR_RED_OFFSET)       + CMD_DATA_LEN_RED_OFFSET-CMD_LENGTH_ONE)
#define CMD_LEN_GREEN_OFFSET            (CMD_FIX_LEN + sizeof(CMD_STR_GREEN_OFFSET)     + CMD_DATA_LEN_GREEN_OFFSET-CMD_LENGTH_ONE)
#define CMD_LEN_BLUE_OFFSET             (CMD_FIX_LEN + sizeof(CMD_STR_BLUE_OFFSET)      + CMD_DATA_LEN_BLUE_OFFSET-CMD_LENGTH_ONE)
#define CMD_LEN_SYNC_THRESHOLD          (CMD_FIX_LEN + sizeof(CMD_STR_SYNC_THRESHOLD)   + CMD_DATA_LEN_SYNC_THRESHOLD-CMD_LENGTH_ONE)
#define CMD_LEN_RESET_RGB_G_O           (CMD_FIX_LEN + sizeof(CMD_STR_RESET_RGB_G_O)    + CMD_DATA_LEN_RESET_RGB_G_O-CMD_LENGTH_ONE)
#define CMD_LEN_GAMMA                   (CMD_FIX_LEN + sizeof(CMD_STR_GAMMA)            + CMD_DATA_LEN_GAMMA-CMD_LENGTH_ONE)
#define CMD_LEN_BRILLIANT_COLOR         (CMD_FIX_LEN + sizeof(CMD_STR_BRILLIANT_COLOR)  + CMD_DATA_LEN_BRILLIANT_COLOR-CMD_LENGTH_ONE)
#define CMD_LEN_COLOR_TEMP              (CMD_FIX_LEN + sizeof(CMD_STR_COLOR_TEMP)       + CMD_DATA_LEN_COLOR_TEMP-CMD_LENGTH_ONE)
#define CMD_LEN_EDGE_ENHANCE            (CMD_FIX_LEN + sizeof(CMD_STR_EDGE_ENHANCE)     + CMD_DATA_LEN_EDGE_ENHANCE-CMD_LENGTH_ONE)
#define CMD_LEN_WHITE_PEAKING           (CMD_FIX_LEN + sizeof(CMD_STR_WHITE_PEAKING)    + CMD_DATA_LEN_WHITE_PEAKING-CMD_LENGTH_ONE)
#define CMD_LEN_IMG_FREEZE              (CMD_FIX_LEN + sizeof(CMD_STR_IMG_FREEZE)       + CMD_DATA_LEN_IMG_FREEZE-CMD_LENGTH_ONE)
#define CMD_LEN_COLOR_WHEEL_SPD         (CMD_FIX_LEN + sizeof(CMD_STR_COLOR_WHEEL_SPD)  + CMD_DATA_LEN_COLOR_WHEEL_SPD-CMD_LENGTH_ONE)
#define CMD_LEN_COLOR_ENHANCE           (CMD_FIX_LEN + sizeof(CMD_STR_COLOR_ENHANCE)    + CMD_DATA_LEN_COLOR_ENHANCE-CMD_LENGTH_ONE)
#define CMD_LEN_TI_CRT_SRC              (7) //must be checked !!!
#define CMD_LEN_ALL_GET_TI              (CMD_FIX_LEN + sizeof(CMD_STR_ALL_GET_TI)       + CMD_DATA_LEN_NULL-CMD_LENGTH_ONE)
#define CMD_LEN_3D_ENABLE               (CMD_FIX_LEN + sizeof(CMD_STR_3D_ENABLE)        + CMD_DATA_LEN_3D_ENABLE-CMD_LENGTH_ONE)
#define CMD_LEN_3D_INVERT               (CMD_FIX_LEN + sizeof(CMD_STR_3D_INVERT)        + CMD_DATA_LEN_3D_INVERT-CMD_LENGTH_ONE)
#define CMD_LEN_3D_PICTURE_SET          (6) //must be checked !!!
#define CMD_LEN_3D_DISPLAY              (6) //must be checked !!!
#define CMD_LEN_TI_T3B                  (CMD_FIX_LEN + sizeof(CMD_STR_TI_T3B)           + CMD_DATA_LEN_TI_T3B-CMD_LENGTH_ONE)
#define CMD_LEN_TI_REAL_BLACK           (CMD_FIX_LEN + sizeof(CMD_STR_TI_REAL_BLACK)    + CMD_DATA_LEN_TI_REAL_BLACK-CMD_LENGTH_ONE)
#define CMD_LEN_TI_FRAME_PACK           (CMD_FIX_LEN + sizeof(CMD_STR_TI_FRAME_PACK)    + CMD_DATA_LEN_TI_FRAME_PACK-CMD_LENGTH_ONE)
#define CMD_LEN_TI_S_BY_S               (CMD_FIX_LEN + sizeof(CMD_STR_TI_S_BY_S)        + CMD_DATA_LEN_TI_S_BY_S-CMD_LENGTH_ONE)
#define CMD_LEN_TI_T_AND_B              (CMD_FIX_LEN + sizeof(CMD_STR_TI_T_AND_B)       + CMD_DATA_LEN_TI_T_AND_B-CMD_LENGTH_ONE)
#define CMD_LEN_TI_FRAME_SEQ            (CMD_FIX_LEN + sizeof(CMD_STR_TI_FRAME_SEQ)     + CMD_DATA_LEN_TI_FRAME_SEQ-CMD_LENGTH_ONE)
#define CMD_LEN_TI_S_T_U                (CMD_FIX_LEN + sizeof(CMD_STR_TI_S_T_U)         + CMD_DATA_LEN_TI_S_T_U-CMD_LENGTH_ONE)
#define CMD_LEN_TI_3D_SYNC_OUT          (CMD_FIX_LEN + sizeof(CMD_STR_TI_3D_SYNC_OUT)   + CMD_DATA_LEN_TI_3D_SYNC_OUT-CMD_LENGTH_ONE)
#define CMD_LEN_TI_FRAME_DELAY          (CMD_FIX_LEN + sizeof(CMD_STR_TI_FRAME_DELAY)   + CMD_DATA_LEN_TI_FRAME_DELAY-CMD_LENGTH_ONE)
#define CMD_LEN_TI_3D                   (CMD_FIX_LEN + sizeof(CMD_STR_TI_3D)            + CMD_DATA_LEN_TI_3D-CMD_LENGTH_ONE)
#define CMD_LEN_TI_1080P24              (CMD_FIX_LEN + sizeof(CMD_STR_TI_1080P24)       + CMD_DATA_LEN_TI_1080P24-CMD_LENGTH_ONE)

#define CMD_LEN_LANGUAGE                (CMD_FIX_LEN + sizeof(CMD_STR_LANGUAGE)         + CMD_DATA_LEN_LANGUAGE-CMD_LENGTH_ONE)
#define CMD_LEN_CEIL_MOUNT              (CMD_FIX_LEN + sizeof(CMD_STR_CEIL_MOUNT)       + CMD_DATA_LEN_CEIL_MOUNT-CMD_LENGTH_ONE)
#define CMD_LEN_REAR_PROJ               (CMD_FIX_LEN + sizeof(CMD_STR_REAR_PROJ)        + CMD_DATA_LEN_REAR_PROJ-CMD_LENGTH_ONE)
#define CMD_LEN_HIGH_ALTITUDE           (CMD_FIX_LEN + sizeof(CMD_STR_HIGH_ALTITUDE)    + CMD_DATA_LEN_HIGH_ALTITUDE-CMD_LENGTH_ONE)
#define CMD_LEN_12V_TRIGGER             (CMD_FIX_LEN + sizeof(CMD_STR_12V_TRIGGER)      + CMD_DATA_LEN_12V_TRIGGER-CMD_LENGTH_ONE)
#define CMD_LEN_HOT_KEY_SET             (CMD_FIX_LEN + sizeof(CMD_STR_HOT_KEY_SET)      + CMD_DATA_LEN_HOT_KEY_SET-CMD_LENGTH_ONE)
#define CMD_LEN_ZOOM_UP                 (CMD_FIX_LEN + sizeof(CMD_STR_ZOOM_UP)          + CMD_DATA_LEN_ZOOM_UP-CMD_LENGTH_ONE)
#define CMD_LEN_ZOOM_DOWN               (CMD_FIX_LEN + sizeof(CMD_STR_ZOOM_DOWN)        + CMD_DATA_LEN_ZOOM_DOWN-CMD_LENGTH_ONE)
#define CMD_LEN_FOCUS_IN                (CMD_FIX_LEN + sizeof(CMD_STR_FOCUS_IN)         + CMD_DATA_LEN_FOCUS_IN-CMD_LENGTH_ONE)
#define CMD_LEN_FOCUS_OUT               (CMD_FIX_LEN + sizeof(CMD_STR_FOCUS_OUT)        + CMD_DATA_LEN_FOCUS_OUT-CMD_LENGTH_ONE)
#define CMD_LEN_LENS_SHIFT_U            (CMD_FIX_LEN + sizeof(CMD_STR_LENS_SHIFT_U)     + CMD_DATA_LEN_LENS_SHIFT_U-CMD_LENGTH_ONE)
#define CMD_LEN_LENS_SHIFT_D            (CMD_FIX_LEN + sizeof(CMD_STR_LENS_SHIFT_D)     + CMD_DATA_LEN_LENS_SHIFT_D-CMD_LENGTH_ONE)
#define CMD_LEN_LENS_SHIFT_L            (CMD_FIX_LEN + sizeof(CMD_STR_LENS_SHIFT_L)     + CMD_DATA_LEN_LENS_SHIFT_L-CMD_LENGTH_ONE)
#define CMD_LEN_LENS_SHIFT_R            (CMD_FIX_LEN + sizeof(CMD_STR_LENS_SHIFT_R)     + CMD_DATA_LEN_LENS_SHIFT_R-CMD_LENGTH_ONE)
#define CMD_LEN_LENS_LOCK               (CMD_FIX_LEN + sizeof(CMD_STR_LENS_LOCK)        + CMD_DATA_LEN_LENS_LOCK-CMD_LENGTH_ONE)
#define CMD_LEN_LENS_CAL                (CMD_FIX_LEN + sizeof(CMD_STR_LENS_CAL)         + CMD_DATA_LEN_LENS_CAL-CMD_LENGTH_ONE)
#define CMD_LEN_MENU_HORZ               (CMD_FIX_LEN + sizeof(CMD_STR_MENU_HORZ)        + CMD_DATA_LEN_MENU_HORZ-CMD_LENGTH_ONE)
#define CMD_LEN_MENU_VERT               (CMD_FIX_LEN + sizeof(CMD_STR_MENU_VERT)        + CMD_DATA_LEN_MENU_VERT-CMD_LENGTH_ONE)
#define CMD_LEN_SHOW_MSG                (CMD_FIX_LEN + sizeof(CMD_STR_SHOW_MSG)         + CMD_DATA_LEN_SHOW_MSG-CMD_LENGTH_ONE)
#define CMD_LEN_MENU_TRANSPARENCY       (CMD_FIX_LEN + sizeof(CMD_STR_MENU_TRANSPARENCY)+ CMD_DATA_LEN_MENU_TRANSPARENCY-CMD_LENGTH_ONE)
#define CMD_LEN_BG_COLOR                (CMD_FIX_LEN + sizeof(CMD_STR_BG_COLOR)         + CMD_DATA_LEN_BG_COLOR-CMD_LENGTH_ONE)
#define CMD_LEN_PIN_PROTECT             (CMD_FIX_LEN + sizeof(CMD_STR_PIN_PROTECT)      + CMD_DATA_LEN_PIN_PROTECT-CMD_LENGTH_ONE)
#define CMD_LEN_CHG_PIN                 (CMD_FIX_LEN + sizeof(CMD_STR_CHG_PIN)          + CMD_DATA_LEN_CHG_PIN-CMD_LENGTH_ONE)
#define CMD_LEN_STANBY_MODE             (CMD_FIX_LEN + sizeof(CMD_STR_STANBY_MODE)      + CMD_DATA_LEN_STANBY_MODE-CMD_LENGTH_ONE)
#define CMD_LEN_AC_PWR_ON               (CMD_FIX_LEN + sizeof(CMD_STR_AC_PWR_ON)        + CMD_DATA_LEN_AC_PWR_ON-CMD_LENGTH_ONE)
#define CMD_LEN_AUTO_SHUTDOWN           (CMD_FIX_LEN + sizeof(CMD_STR_AUTO_SHUTDOWN)    + CMD_DATA_LEN_AUTO_SHUTDOWN-CMD_LENGTH_ONE)
#define CMD_LEN_SLEEP_TIMER             (CMD_FIX_LEN + sizeof(CMD_STR_SLEEP_TIMER)      + CMD_DATA_LEN_SLEEP_TIMER-CMD_LENGTH_ONE)
#define CMD_LEN_UART_BAUD_RATE          (CMD_FIX_LEN + sizeof(CMD_STR_UART_BAUD_RATE)   + CMD_DATA_LEN_UART_BAUD_RATE-CMD_LENGTH_ONE)
#define CMD_LEN_UART_ECHO               (CMD_FIX_LEN + sizeof(CMD_STR_UART_ECHO)        + CMD_DATA_LEN_UART_ECHO-CMD_LENGTH_ONE)
#define CMD_LEN_FACTORY_RESET           (CMD_FIX_LEN + sizeof(CMD_STR_FACTORY_RESET)    + CMD_DATA_LEN_FACTORY_RESET-CMD_LENGTH_ONE)
#define CMD_LEN_CCA_MT_EN               (CMD_FIX_LEN + sizeof(CMD_STR_CCA_MT_EN)        + CMD_DATA_LEN_CCA_MT_EN-CMD_LENGTH_ONE)
#define CMD_LEN_CCA_MT_MD_R_INT         (CMD_FIX_LEN + sizeof(CMD_STR_CCA_MT_MD_R_INT)  + CMD_DATA_LEN_CCA_MT_MD_R_INT-CMD_LENGTH_ONE)
#define CMD_LEN_CCA_MT_MD_R_X           (CMD_FIX_LEN + sizeof(CMD_STR_CCA_MT_MD_R_X)    + CMD_DATA_LEN_CCA_MT_MD_R_X-CMD_LENGTH_ONE)
#define CMD_LEN_CCA_MT_MD_R_Y           (CMD_FIX_LEN + sizeof(CMD_STR_CCA_MT_MD_R_Y)    + CMD_DATA_LEN_CCA_MT_MD_R_Y-CMD_LENGTH_ONE)
#define CMD_LEN_CCA_MT_MD_G_INT         (CMD_FIX_LEN + sizeof(CMD_STR_CCA_MT_MD_G_INT)  + CMD_DATA_LEN_CCA_MT_MD_G_INT-CMD_LENGTH_ONE)
#define CMD_LEN_CCA_MT_MD_G_X           (CMD_FIX_LEN + sizeof(CMD_STR_CCA_MT_MD_G_X)    + CMD_DATA_LEN_CCA_MT_MD_G_X-CMD_LENGTH_ONE)
#define CMD_LEN_CCA_MT_MD_G_Y           (CMD_FIX_LEN + sizeof(CMD_STR_CCA_MT_MD_G_Y)    + CMD_DATA_LEN_CCA_MT_MD_G_Y-CMD_LENGTH_ONE)
#define CMD_LEN_CCA_MT_MD_B_INT         (CMD_FIX_LEN + sizeof(CMD_STR_CCA_MT_MD_B_INT)  + CMD_DATA_LEN_MT_MD_B_INT-CMD_LENGTH_ONE)
#define CMD_LEN_CCA_MT_MD_B_X           (CMD_FIX_LEN + sizeof(CMD_STR_CCA_MT_MD_B_X)    + CMD_DATA_LEN_MT_MD_B_X-CMD_LENGTH_ONE)
#define CMD_LEN_CCA_MT_MD_B_Y           (CMD_FIX_LEN + sizeof(CMD_STR_CCA_MT_MD_B_Y)    + CMD_DATA_LEN_MT_MD_B_Y-CMD_LENGTH_ONE)
#define CMD_LEN_CCA_MT_MD_W_INT         (CMD_FIX_LEN + sizeof(CMD_STR_CCA_MT_MD_W_INT)  + CMD_DATA_LEN_MT_MD_W_INT-CMD_LENGTH_ONE)
#define CMD_LEN_CCA_MT_MD_W_X           (CMD_FIX_LEN + sizeof(CMD_STR_CCA_MT_MD_W_X)    + CMD_DATA_LEN_CCA_MT_MD_W_X-CMD_LENGTH_ONE)
#define CMD_LEN_CCA_MT_MD_W_Y           (CMD_FIX_LEN + sizeof(CMD_STR_CCA_MT_MD_W_Y)    + CMD_DATA_LEN_CCA_MT_MD_W_Y-CMD_LENGTH_ONE)
#define CMD_LEN_CCA_MT_TD_R_GAIN        (CMD_FIX_LEN + sizeof(CMD_STR_CCA_MT_TD_R_GAIN) + CMD_DATA_LEN_CCA_MT_TD_R_GAIN-CMD_LENGTH_ONE)
#define CMD_LEN_CCA_MT_TD_R_X           (CMD_FIX_LEN + sizeof(CMD_STR_CCA_MT_TD_R_X)    + CMD_DATA_LEN_CCA_MT_TD_R_X-CMD_LENGTH_ONE)
#define CMD_LEN_CCA_MT_TD_R_Y           (CMD_FIX_LEN + sizeof(CMD_STR_CCA_MT_TD_R_Y)    + CMD_DATA_LEN_CCA_MT_TD_R_Y-CMD_LENGTH_ONE)
#define CMD_LEN_CCA_MT_TD_G_GAIN        (CMD_FIX_LEN + sizeof(CMD_STR_CCA_MT_TD_G_GAIN) + CMD_DATA_LEN_CCA_MT_TD_G_GAIN-CMD_LENGTH_ONE)
#define CMD_LEN_CCA_MT_TD_G_X           (CMD_FIX_LEN + sizeof(CMD_STR_CCA_MT_TD_G_X)    + CMD_DATA_LEN_CCA_MT_TD_G_X-CMD_LENGTH_ONE)
#define CMD_LEN_CCA_MT_TD_G_Y           (CMD_FIX_LEN + sizeof(CMD_STR_CCA_MT_TD_G_Y)    + CMD_DATA_LEN_CCA_MT_TD_G_Y-CMD_LENGTH_ONE)
#define CMD_LEN_CCA_MT_TD_B_GAIN        (CMD_FIX_LEN + sizeof(CMD_STR_CCA_MT_TD_B_GAIN) + CMD_DATA_LEN_CCA_MT_TD_B_GAIN-CMD_LENGTH_ONE)
#define CMD_LEN_CCA_MT_TD_B_X           (CMD_FIX_LEN + sizeof(CMD_STR_CCA_MT_TD_B_X)    + CMD_DATA_LEN_CCA_MT_TD_B_X-CMD_LENGTH_ONE)
#define CMD_LEN_CCA_MT_TD_B_Y           (CMD_FIX_LEN + sizeof(CMD_STR_CCA_MT_TD_B_Y)    + CMD_DATA_LEN_CCA_MT_TD_B_Y-CMD_LENGTH_ONE)
#define CMD_LEN_CCA_MT_TD_W_GAIN        (CMD_FIX_LEN + sizeof(CMD_STR_CCA_MT_TD_W_GAIN) + CMD_DATA_LEN_CCA_MT_TD_W_GAIN-CMD_LENGTH_ONE)
#define CMD_LEN_CCA_MT_TD_W_X           (CMD_FIX_LEN + sizeof(CMD_STR_CCA_MT_TD_W_X)    + CMD_DATA_LEN_CCA_MT_TD_W_X-CMD_LENGTH_ONE)
#define CMD_LEN_CCA_MT_TD_W_Y           (CMD_FIX_LEN + sizeof(CMD_STR_CCA_MT_TD_W_Y)    + CMD_DATA_LEN_CCA_MT_TD_W_Y-CMD_LENGTH_ONE)
#define CMD_LEN_EBL_BLENDING            (11)
#define CMD_LEN_EBL_MAKER               (11)
#define CMD_LEN_EBL_GRID                (11)
#define CMD_LEN_EBL_OU                  (13)
#define CMD_LEN_EBL_OD                  (13)
#define CMD_LEN_EBL_OL                  (13)
#define CMD_LEN_EBL_OR                  (13)
#define CMD_LEN_EBL_BAO                 (15)
#define CMD_LEN_EBL_NBAO                (15)
#define CMD_LEN_EBL_CURVE               (13)
#define CMD_LEN_PIN_ENABLE              (13)
#define CMD_LEN_ALL_GET_TC              (CMD_FIX_LEN + sizeof(CMD_STR_ALL_GET_TC)       + CMD_DATA_LEN_NULL-CMD_LENGTH_ONE)
#define CMD_LEN_CCA_MT_TEST_PAT         (CMD_FIX_LEN + sizeof(CMD_STR_CCA_MT_TEST_PAT)  + CMD_DATA_LEN_CCA_MT_TEST_PAT-CMD_LENGTH_ONE)
#define CMD_LEN_CCA_MN_EN               (CMD_FIX_LEN + sizeof(CMD_STR_CCA_MN_EN)        + CMD_DATA_LEN_CCA_MN_EN-CMD_LENGTH_ONE)
#define CMD_LEN_CCA_MN_TEST_PAT         (CMD_FIX_LEN + sizeof(CMD_STR_CCA_MN_TEST_PAT)  + CMD_DATA_LEN_CCA_MN_TEST_PAT-CMD_LENGTH_ONE)
#define CMD_LEN_CCA_MN_RPR              (CMD_FIX_LEN + sizeof(CMD_STR_CCA_MN_RPR)       + CMD_DATA_LEN_CCA_MN_RPR-CMD_LENGTH_ONE)
#define CMD_LEN_CCA_MN_GPR              (CMD_FIX_LEN + sizeof(CMD_STR_CCA_MN_GPR)       + CMD_DATA_LEN_CCA_MN_GPR-CMD_LENGTH_ONE)
#define CMD_LEN_CCA_MN_BPR              (CMD_FIX_LEN + sizeof(CMD_STR_CCA_MN_BPR)       + CMD_DATA_LEN_CCA_MN_BPR-CMD_LENGTH_ONE)
#define CMD_LEN_CCA_MN_GPG              (CMD_FIX_LEN + sizeof(CMD_STR_CCA_MN_GPG)       + CMD_DATA_LEN_CCA_MN_GPG-CMD_LENGTH_ONE)
#define CMD_LEN_CCA_MN_RPG              (CMD_FIX_LEN + sizeof(CMD_STR_CCA_MN_RPG)       + CMD_DATA_LEN_CCA_MN_RPG-CMD_LENGTH_ONE)
#define CMD_LEN_CCA_MN_BPG              (CMD_FIX_LEN + sizeof(CMD_STR_CCA_MN_BPG)       + CMD_DATA_LEN_CCA_MN_BPG-CMD_LENGTH_ONE)
#define CMD_LEN_CCA_MN_BPB              (CMD_FIX_LEN + sizeof(CMD_STR_CCA_MN_BPB)       + CMD_DATA_LEN_CCA_MN_BPB-CMD_LENGTH_ONE)
#define CMD_LEN_CCA_MN_RPB              (CMD_FIX_LEN + sizeof(CMD_STR_CCA_MN_RPB)       + CMD_DATA_LEN_CCA_MN_RPB-CMD_LENGTH_ONE)
#define CMD_LEN_CCA_MN_GPB              (CMD_FIX_LEN + sizeof(CMD_STR_CCA_MN_GPB)       + CMD_DATA_LEN_CCA_MN_GPB-CMD_LENGTH_ONE)
#define CMD_LEN_CCA_MN_RPW              (CMD_FIX_LEN + sizeof(CMD_STR_CCA_MN_RPW)       + CMD_DATA_LEN_CCA_MN_RPW-CMD_LENGTH_ONE)
#define CMD_LEN_CCA_MN_GPW              (CMD_FIX_LEN + sizeof(CMD_STR_CCA_MN_GPW)       + CMD_DATA_LEN_CCA_MN_GPW-CMD_LENGTH_ONE)
#define CMD_LEN_CCA_MN_BPW              (CMD_FIX_LEN + sizeof(CMD_STR_CCA_MN_BPW)       + CMD_DATA_LEN_CCA_MN_BPW-CMD_LENGTH_ONE)
#define CMD_LEN_EBL_COLOR               (11)
#define CMD_LEN_EBL_GAMMA               (11)
#define CMD_LEN_PROJ_ADDR               (CMD_FIX_LEN + sizeof(CMD_STR_PROJ_ADDR)        + CMD_DATA_LEN_PROJ_ADR-CMD_LENGTH_ONE)
#define CMD_LEN_CCA_MT_APPLY            (CMD_FIX_LEN + sizeof(CMD_STR_CCA_MT_APPLY)     + CMD_DATA_LEN_CCA_MT_APPLY-CMD_LENGTH_ONE)
#define CMD_LEN_CW_IDX_2X               (CMD_FIX_LEN + sizeof(CMD_STR_CW_IDX_2X)        + CMD_DATA_LEN_CW_IDX_2X-CMD_LENGTH_ONE)
#define CMD_LEN_CW_IDX_3X               (CMD_FIX_LEN + sizeof(CMD_STR_CW_IDX_3X)        + CMD_DATA_LEN_CW_IDX_3X-CMD_LENGTH_ONE)
#define CMD_LEN_ERR_LOG                 (5)
#define CMD_LEN_MOD_ADJ                 (5)
#define CMD_LEN_CCA_MT_MD_RESET         (CMD_FIX_LEN + sizeof(CMD_STR_CCA_MT_MD_RESET)  + CMD_DATA_LEN_CCA_MT_MD_RESET-CMD_LENGTH_ONE)
#define CMD_LEN_CCA_MT_TD_RESET         (CMD_FIX_LEN + sizeof(CMD_STR_CCA_MT_TD_RESET)  + CMD_DATA_LEN_CCA_MT_TD_RESET-CMD_LENGTH_ONE)
#define CMD_LEN_CCA_MN_RESET            (CMD_FIX_LEN + sizeof(CMD_STR_CCA_MN_RESET)     + CMD_DATA_LEN_CCA_MN_RESET-CMD_LENGTH_ONE)
#define CMD_LEN_PHOSPHER_W_IDX          (CMD_FIX_LEN + sizeof(CMD_STR_PHOSPHER_W_IDX)   + CMD_DATA_LEN_PHOSPHER_W_IDX-CMD_LENGTH_ONE)
#define CMD_LEN_FILTER_W_IDX            (CMD_FIX_LEN + sizeof(CMD_STR_FILTER_W_IDX)     + CMD_DATA_LEN_FILTER_W_IDX-CMD_LENGTH_ONE)
#define CMD_LEN_LENS_MEM_APPLY          (CMD_FIX_LEN + sizeof(CMD_STR_LENS_MEM_APPLY)   + CMD_DATA_LEN_LENS_MEM_APPLY-CMD_LENGTH_ONE)
#define CMD_LEN_LENS_MEM_SAVE           (CMD_FIX_LEN + sizeof(CMD_STR_LENS_MEM_SAVE)    + CMD_DATA_LEN_LENS_MEM_SAVE-CMD_LENGTH_ONE)
#define CMD_LEN_KP_BACK_LIGHT           (CMD_FIX_LEN + sizeof(CMD_STR_KP_BACK_LIGHT)    + CMD_DATA_LEN_KP_BACK_LIGHT-CMD_LENGTH_ONE)
#define CMD_LEN_STATUS_LED              (CMD_FIX_LEN + sizeof(CMD_STR_STATUS_LED)       + CMD_DATA_LEN_STATUS_LED-CMD_LENGTH_ONE)
#define CMD_LEN_UART_PATH               (CMD_FIX_LEN + sizeof(CMD_STR_UART_PATH)        + CMD_DATA_LEN_UART_PATH-CMD_LENGTH_ONE)

//A70LH_sc_0002, add start
#define CMD_LEN_IR_CTRL_TOP             (CMD_FIX_LEN + sizeof(CMD_STR_IR_CTRL_TOP)      + CMD_DATA_LEN_IR_CTRL_TOP-CMD_LENGTH_ONE)
#define CMD_LEN_IR_CTRL_FRONT           (CMD_FIX_LEN + sizeof(CMD_STR_IR_CTRL_FRONT)    + CMD_DATA_LEN_IR_CTRL_FRONT-CMD_LENGTH_ONE)
#define CMD_LEN_IR_CTRL_HDBASET         (CMD_FIX_LEN + sizeof(CMD_STR_IR_CTRL_HDBASET)  + CMD_DATA_LEN_IR_CTRL_HDBASET-CMD_LENGTH_ONE)
//A70LH_sc_0002, add end
//A70LH_sc_0013, add start
#define CMD_LEN_TC_PI3X                 (CMD_FIX_LEN + sizeof(CMD_STR_TC_PI3X)          + CMD_DATA_LEN_TC_PI3X-CMD_LENGTH_ONE)
#define CMD_LEN_TC_FI3X                 (CMD_FIX_LEN + sizeof(CMD_STR_TC_FI3X)          + CMD_DATA_LEN_TC_FI3X-CMD_LENGTH_ONE)
//A70LH_sc_0013, add end
#define CMD_LEN_TC_WallCollor           (CMD_FIX_LEN + sizeof(CMD_STR_TC_WallCollor)    + CMD_DATA_LEN_TC_WallCollor-CMD_LENGTH_ONE)
#define CMD_LEN_TC_INS_PWR_OFF          (CMD_FIX_LEN + sizeof(CMD_STR_TC_INS_PWR_OFF)   + CMD_DATA_LEN_TC_INS_PWR_OFF-CMD_LENGTH_ONE)
#define CMD_LEN_TC_HDMI_EQ              (CMD_FIX_LEN + sizeof(CMD_STR_TC_HDMI_EQ)       + CMD_DATA_LEN_TC_HDMI_EQ-CMD_LENGTH_ONE)
#define CMD_LEN_TC_LOGO_CAPTURE         (CMD_FIX_LEN + sizeof(CMD_STR_TC_LOGO_CAPTURE)  + CMD_DATA_LEN_TC_LOGO_CAPTURE-CMD_LENGTH_ONE)
#define CMD_LEN_TC_MENU_LOCATION        (CMD_FIX_LEN + sizeof(CMD_STR_TC_MENU_LOCATION) + CMD_DATA_LEN_TC_MENU_LOCATION-CMD_LENGTH_ONE)
//B35LB_David_0013 mod end
//A70LH_sc_0025, add start
#define CMD_LEN_HSG_En                  (CMD_FIX_LEN + strlen(CMD_STR_HSG_En)           + CMD_DATA_LEN_HSG_En)
#define CMD_LEN_HSG_ATP                 (CMD_FIX_LEN + strlen(CMD_STR_HSG_ATP)          + CMD_DATA_LEN_HSG_ATP)
#define CMD_LEN_HSG_RH                  (CMD_FIX_LEN + strlen(CMD_STR_HSG_RH)           + CMD_DATA_LEN_HSG_RH)
#define CMD_LEN_HSG_RS                  (CMD_FIX_LEN + strlen(CMD_STR_HSG_RS)           + CMD_DATA_LEN_HSG_RS)
#define CMD_LEN_HSG_RG                  (CMD_FIX_LEN + strlen(CMD_STR_HSG_RG)           + CMD_DATA_LEN_HSG_RG)
#define CMD_LEN_HSG_GH                  (CMD_FIX_LEN + strlen(CMD_STR_HSG_GH)           + CMD_DATA_LEN_HSG_GH)
#define CMD_LEN_HSG_GS                  (CMD_FIX_LEN + strlen(CMD_STR_HSG_GS)           + CMD_DATA_LEN_HSG_GS)
#define CMD_LEN_HSG_GG                  (CMD_FIX_LEN + strlen(CMD_STR_HSG_GG)           + CMD_DATA_LEN_HSG_GG)
#define CMD_LEN_HSG_BH                  (CMD_FIX_LEN + strlen(CMD_STR_HSG_BH)           + CMD_DATA_LEN_HSG_BH)
#define CMD_LEN_HSG_BS                  (CMD_FIX_LEN + strlen(CMD_STR_HSG_BS)           + CMD_DATA_LEN_HSG_BS)
#define CMD_LEN_HSG_BG                  (CMD_FIX_LEN + strlen(CMD_STR_HSG_BG)           + CMD_DATA_LEN_HSG_BG)
#define CMD_LEN_HSG_CH                  (CMD_FIX_LEN + strlen(CMD_STR_HSG_CH)           + CMD_DATA_LEN_HSG_CH)
#define CMD_LEN_HSG_CS                  (CMD_FIX_LEN + strlen(CMD_STR_HSG_CS)           + CMD_DATA_LEN_HSG_CS)
#define CMD_LEN_HSG_CG                  (CMD_FIX_LEN + strlen(CMD_STR_HSG_CG)           + CMD_DATA_LEN_HSG_CG)
#define CMD_LEN_HSG_MH                  (CMD_FIX_LEN + strlen(CMD_STR_HSG_MH)           + CMD_DATA_LEN_HSG_MH)
#define CMD_LEN_HSG_MS                  (CMD_FIX_LEN + strlen(CMD_STR_HSG_MS)           + CMD_DATA_LEN_HSG_MS)
#define CMD_LEN_HSG_MG                  (CMD_FIX_LEN + strlen(CMD_STR_HSG_MG)           + CMD_DATA_LEN_HSG_MG)
#define CMD_LEN_HSG_YH                  (CMD_FIX_LEN + strlen(CMD_STR_HSG_YH)           + CMD_DATA_LEN_HSG_YH)
#define CMD_LEN_HSG_YS                  (CMD_FIX_LEN + strlen(CMD_STR_HSG_YS)           + CMD_DATA_LEN_HSG_YS)
#define CMD_LEN_HSG_YG                  (CMD_FIX_LEN + strlen(CMD_STR_HSG_YG)           + CMD_DATA_LEN_HSG_YG)
#define CMD_LEN_HSG_WR                  (CMD_FIX_LEN + strlen(CMD_STR_HSG_WR)           + CMD_DATA_LEN_HSG_WR)
#define CMD_LEN_HSG_WG                  (CMD_FIX_LEN + strlen(CMD_STR_HSG_WG)           + CMD_DATA_LEN_HSG_WG)
#define CMD_LEN_HSG_WB                  (CMD_FIX_LEN + strlen(CMD_STR_HSG_WB)           + CMD_DATA_LEN_HSG_WB)
#define CMD_LEN_HSG_RTD                 (CMD_FIX_LEN + strlen(CMD_STR_HSG_RTD)          + CMD_DATA_LEN_HSG_RTD)
#define CMD_LEN_HSG_CE                  (CMD_FIX_LEN + strlen(CMD_STR_HSG_CE)           + CMD_DATA_LEN_HSG_CE)
#define CMD_LEN_ALL_GET_HSG             (CMD_FIX_LEN + strlen(CMD_STR_ALL_GET_HSG)      + CMD_DATA_LEN_NULL)
//A70LH_sc_0025, add end

#define CMD_LEN_ALL_GET_MSC             (CMD_FIX_LEN + strlen(CMD_STR_ALL_GET_MSC)      + CMD_DATA_LEN_NULL)
//B35LB_David_0013 add start
#define CMD_LEN_ALL_GET_MSV             (CMD_FIX_LEN + sizeof(CMD_STR_ALL_GET_MSV)      + CMD_DATA_LEN_NULL-CMD_LENGTH_ONE)
#define CMD_LEN_MCU_VER                 (CMD_FIX_LEN + sizeof(CMD_STR_MCU_VER)          + VER_STR_LEN-CMD_LENGTH_ONE)
#define CMD_LEN_SCALER_VER              (CMD_FIX_LEN + sizeof(CMD_STR_SCALER_VER)       + VER_STR_LEN-CMD_LENGTH_ONE)
#define CMD_LEN_3D_DECODER_VER          (CMD_FIX_LEN + sizeof(CMD_STR_3D_DECODER_VER)   + VER_STR_LEN-CMD_LENGTH_ONE)
#define CMD_LEN_FORMATTER_VER           (CMD_FIX_LEN + sizeof(CMD_STR_FORMATTER_VER)    + VER_STR_LEN-CMD_LENGTH_ONE)
#define CMD_LEN_HDBASET_VER             (CMD_FIX_LEN + sizeof(CMD_STR_HDBASET_VER)      + VER_STR_LEN-CMD_LENGTH_ONE)
#define CMD_LEN_KEYPAD_VER              (CMD_FIX_LEN + sizeof(CMD_STR_KEYPAD_VER)       + VER_STR_LEN-CMD_LENGTH_ONE)
#define CMD_LEN_LD_DRV_VER              (CMD_FIX_LEN + sizeof(CMD_STR_LD_DRV_VER)       + VER_STR_LEN-CMD_LENGTH_ONE)
#define CMD_LEN_MOTOR_VER               (CMD_FIX_LEN + sizeof(CMD_STR_MOTOR_VER)        + VER_STR_LEN-CMD_LENGTH_ONE)
//B35LB_David_0013 add end
#define CMD_LEN_ALL_GET_MSMS            (CMD_FIX_LEN + strlen(CMD_STR_ALL_GET_MSMS)     + CMD_DATA_LEN_NULL)
#define CMD_LEN_ALL_GET_MSSS            (CMD_FIX_LEN + strlen(CMD_STR_ALL_GET_MSSS)     + CMD_DATA_LEN_NULL)

#define CMD_LEN_TIME_DETECT_MODE        (CMD_FIX_LEN + strlen(CMD_STR_TIME_DETECT_MODE) + CMD_DATA_LEN_TIME_DETECT_MODE)
#define CMD_LEN_SRC_HOT_KEY_EN          (CMD_FIX_LEN + strlen(CMD_STR_SRC_HOT_KEY_EN)   + CMD_DATA_LEN_SRC_HOT_KEY_EN)
#define CMD_LEN_SRC_CHANGE_MODE         (CMD_FIX_LEN + strlen(CMD_STR_SRC_CHANGE_MODE)  + CMD_DATA_LEN_SRC_CHANGE_MODE)
#define CMD_LEN_SRC_HK_VGA              (CMD_FIX_LEN + strlen(CMD_STR_SRC_HK_VGA)       + CMD_DATA_LEN_SRC_HK_VGA)
#define CMD_LEN_SRC_HK_BNC              (CMD_FIX_LEN + strlen(CMD_STR_SRC_HK_BNC)       + CMD_DATA_LEN_SRC_HK_BNC)
#define CMD_LEN_SRC_HK_HDMI1            (CMD_FIX_LEN + strlen(CMD_STR_SRC_HK_HDMI1)     + CMD_DATA_LEN_SRC_HK_HDMI1)
#define CMD_LEN_SRC_HK_HDMI2            (CMD_FIX_LEN + strlen(CMD_STR_SRC_HK_HDMI2)     + CMD_DATA_LEN_SRC_HK_HDMI2)
#define CMD_LEN_SRC_HK_DVID             (CMD_FIX_LEN + strlen(CMD_STR_SRC_HK_DVID)      + CMD_DATA_LEN_SRC_HK_DVID)
#define CMD_LEN_SRC_HK_DP               (CMD_FIX_LEN + strlen(CMD_STR_SRC_HK_DP)        + CMD_DATA_LEN_SRC_HK_DP)
#define CMD_LEN_SRC_HK_COMPONENT        (CMD_FIX_LEN + strlen(CMD_STR_SRC_HK_COMPONENT) + CMD_DATA_LEN_SRC_HK_COMPONENT)
#define CMD_LEN_SRC_HK_SVIDEO           (CMD_FIX_LEN + strlen(CMD_STR_SRC_HK_SVIDEO)    + CMD_DATA_LEN_SRC_HK_SVIDEO)
#define CMD_LEN_SRC_HK_COMPOSITE        (CMD_FIX_LEN + strlen(CMD_STR_SRC_HK_COMPOSITE) + CMD_DATA_LEN_SRC_HK_COMPOSITE)
#define CMD_LEN_SRC_HK_PRESENTER        (CMD_FIX_LEN + strlen(CMD_STR_SRC_HK_PRESENTER) + CMD_DATA_LEN_SRC_HK_PRESENTER)
#define CMD_LEN_SRC_HK_CARD_RD          (CMD_FIX_LEN + strlen(CMD_STR_SRC_HK_CARD_RD)   + CMD_DATA_LEN_SRC_HK_CARD_RD)
#define CMD_LEN_SRC_HK_MINI_USB         (CMD_FIX_LEN + strlen(CMD_STR_SRC_HK_MINI_USB)  + CMD_DATA_LEN_SRC_HK_MINI_USB)
#define CMD_LEN_SRC_HK_HDBASET          (CMD_FIX_LEN + strlen(CMD_STR_SRC_HK_HDBASET)   + CMD_DATA_LEN_SRC_HK_HDBASET)
#define CMD_LEN_SRC_HK_3GSDI            (CMD_FIX_LEN + strlen(CMD_STR_SRC_HK_3GSDI)     + CMD_DATA_LEN_SRC_HK_3GSDI)
#define CMD_LEN_BLANK_ON_SRC_SW         (CMD_FIX_LEN + strlen(CMD_STR_BLANK_ON_SRC_SW)  + CMD_DATA_LEN_BLANK_ON_SRC_SW)
#define CMD_LEN_ALL_GET_TP              (CMD_FIX_LEN + strlen(CMD_STR_ALL_GET_TP)       + CMD_DATA_LEN_NULL)

#define CMD_LEN_ALL_GET_TT              (CMD_FIX_LEN + strlen(CMD_STR_ALL_GET_TT)       + CMD_DATA_LEN_NULL)

#define CMD_LEN_NET_SHOW_MSG            (CMD_FIX_LEN + strlen(CMD_STR_NET_SHOW_MSG)     + CMD_DATA_LEN_NET_SHOW_MSG)
#define CMD_LEN_ALL_GET_NS              (CMD_FIX_LEN + strlen(CMD_STR_ALL_GET_NS)       + CMD_DATA_LEN_NULL)
#define CMD_LEN_NET_RESTART             (CMD_FIX_LEN + strlen(CMD_STR_NET_RESTART)      + CMD_DATA_LEN_NET_RESTART)

//shall be checked ***
#define CMD_LEN_CRE_SRC_HOTKEY          (10)
#define CMD_LEN_CRE_WARM_TIMER          (CMD_FIX_LEN + strlen(CMD_STR_CRE_WARM_TIMER)   + CMD_STR_D_LEN_CRE_WARM_TIMER)
#define CMD_LEN_CRE_COOL_TIMER          (CMD_FIX_LEN + strlen(CMD_STR_CRE_COOL_TIMER)   + CMD_STR_D_LEN_CRE_COOL_TIMER)
#define CMD_LEN_CRE_LP_1_HOURS          (CMD_FIX_LEN + strlen(CMD_STR_CRE_LP_1_HOURS)   + CMD_STR_D_LEN_CRE_LP_1_HOURS)
#define CMD_LEN_CRE_LP_2_HOURS          (CMD_FIX_LEN + strlen(CMD_STR_CRE_LP_2_HOURS)   + CMD_STR_D_LEN_CRE_LP_2_HOURS)
#define CMD_LEN_CRE_LP_T_HOURS          (CMD_FIX_LEN + strlen(CMD_STR_CRE_LP_T_HOURS)   + CMD_STR_D_LEN_CRE_LP_T_HOURS) //Common_Kraz_0217, add, Total Projector Hour
#define CMD_LEN_CRE_LC_HOURS            (CMD_FIX_LEN + strlen(CMD_STR_CRE_LC_HOURS)     + CMD_STR_D_LEN_CRE_LC_HOURS)   //A70LH_Kraz_0272, add, TEC Life Hours
#define CMD_LEN_CRE_RESOLUTION          (CMD_FIX_LEN + strlen(CMD_STR_CRE_RESOLUTION)   + CMD_STR_D_LEN_CRE_RESOLUTION)
#define CMD_LEN_CRE_SYS_TEMP            (CMD_FIX_LEN + strlen(CMD_STR_CRE_SYS_TEMP)     + CMD_STR_D_LEN_CRE_SYS_TEMP)
//shall be checked &&&

#define CMD_LEN_ALL_GET_MS              (CMD_FIX_LEN + sizeof(CMD_STR_ALL_GET_MS)       + CMD_DATA_LEN_NULL-CMD_LENGTH_ONE)
#define CMD_LEN_MS_LD1                  (CMD_FIX_LEN + sizeof(CMD_STR_MS_LD1)           + CMD_DATA_LEN_NULL-CMD_LENGTH_ONE) //A70LH_sc_0029, add start
#define CMD_LEN_MS_LD2                  (CMD_FIX_LEN + sizeof(CMD_STR_MS_LD2)           + CMD_DATA_LEN_NULL-CMD_LENGTH_ONE)
#define CMD_LEN_MS_LD3                  (CMD_FIX_LEN + sizeof(CMD_STR_MS_LD3)           + CMD_DATA_LEN_NULL-CMD_LENGTH_ONE)
#define CMD_LEN_MS_LD4                  (CMD_FIX_LEN + sizeof(CMD_STR_MS_LD4)           + CMD_DATA_LEN_NULL-CMD_LENGTH_ONE)
#define CMD_LEN_MS_LD5                  (CMD_FIX_LEN + sizeof(CMD_STR_MS_LD5)           + CMD_DATA_LEN_NULL-CMD_LENGTH_ONE)
#define CMD_LEN_MS_LD6                  (CMD_FIX_LEN + sizeof(CMD_STR_MS_LD6)           + CMD_DATA_LEN_NULL-CMD_LENGTH_ONE)
#define CMD_LEN_MS_LD7                  (CMD_FIX_LEN + sizeof(CMD_STR_MS_LD7)           + CMD_DATA_LEN_NULL-CMD_LENGTH_ONE)
#define CMD_LEN_MS_LD8                  (CMD_FIX_LEN + sizeof(CMD_STR_MS_LD8)           + CMD_DATA_LEN_NULL-CMD_LENGTH_ONE)
#define CMD_LEN_MS_LD9                  (CMD_FIX_LEN + sizeof(CMD_STR_MS_LD9)           + CMD_DATA_LEN_NULL-CMD_LENGTH_ONE)
#define CMD_LEN_MS_LD10                 (CMD_FIX_LEN + sizeof(CMD_STR_MS_LD10)          + CMD_DATA_LEN_NULL-CMD_LENGTH_ONE)
#define CMD_LEN_MS_LD11                 (CMD_FIX_LEN + sizeof(CMD_STR_MS_LD11)          + CMD_DATA_LEN_NULL-CMD_LENGTH_ONE)
#define CMD_LEN_MS_LD12                 (CMD_FIX_LEN + sizeof(CMD_STR_MS_LD12)          + CMD_DATA_LEN_NULL-CMD_LENGTH_ONE)
#define CMD_LEN_MS_LD13                 (CMD_FIX_LEN + sizeof(CMD_STR_MS_LD13)          + CMD_DATA_LEN_NULL-CMD_LENGTH_ONE) //A70LH_sc_0029, add end

#define CMD_LEN_NET_LAN_IP              (CMD_FIX_LEN + sizeof(CMD_STR_NET_LAN_IP)       + CMD_DATA_LEN_NET_LAN_IP-CMD_LENGTH_ONE)
#define CMD_LEN_NET_LAN_MASK            (CMD_FIX_LEN + sizeof(CMD_STR_NET_LAN_MASK)     + CMD_DATA_LEN_NET_LAN_MASK-CMD_LENGTH_ONE)
#define CMD_LEN_NET_LAN_GATEWAY         (CMD_FIX_LEN + sizeof(CMD_STR_NET_LAN_GATEWAY)  + CMD_DATA_LEN_NET_LAN_GATEWAY-CMD_LENGTH_ONE)
#define CMD_LEN_NET_PROJ_NAME           (CMD_FIX_LEN + sizeof(CMD_STR_NET_PROJ_NAME)    + CMD_DATA_LEN_NET_PROJ_NAME-CMD_LENGTH_ONE)
#define CMD_LEN_NET_LAN_MAC             (CMD_FIX_LEN + sizeof(CMD_STR_NET_LAN_MAC)      + CMD_DATA_LEN_NET_LAN_MAC-CMD_LENGTH_ONE)
#define CMD_LEN_NET_LAN_ALL             (CMD_FIX_LEN + sizeof(CMD_STR_NET_LAN_ALL)      + CMD_DATA_LEN_NET_LAN_ALL-CMD_LENGTH_ONE)
#define CMD_LEN_NET_FW_VER              (CMD_FIX_LEN + sizeof(CMD_STR_NET_FW_VER)       + CMD_DATA_LEN_NET_FW_VER-CMD_LENGTH_ONE)
#define CMD_LEN_NET_WLAN_EN             (CMD_FIX_LEN + sizeof(CMD_STR_NET_WLAN_EN)      + CMD_DATA_LEN_NET_WLAN_EN-CMD_LENGTH_ONE)
#define CMD_LEN_NET_WLAN_SIP            (CMD_FIX_LEN + sizeof(CMD_STR_NET_WLAN_SIP)     + CMD_DATA_LEN_NET_WLAN_SIP-CMD_LENGTH_ONE)
#define CMD_LEN_NET_WLAN_EIP            (CMD_FIX_LEN + sizeof(CMD_STR_NET_WLAN_EIP)     + CMD_DATA_LEN_NET_WLAN_EIP-CMD_LENGTH_ONE)
#define CMD_LEN_NET_WLAN_MASK           (CMD_FIX_LEN + sizeof(CMD_STR_NET_WLAN_MASK)    + CMD_DATA_LEN_NET_WLAN_MASK-CMD_LENGTH_ONE)
#define CMD_LEN_NET_WLAN_GATEWAY        (CMD_FIX_LEN + sizeof(CMD_STR_NET_WLAN_GATEWAY) + CMD_DATA_LEN_NET_WLAN_GATEWAY-CMD_LENGTH_ONE)
#define CMD_LEN_NET_WLAN_MAC            (CMD_FIX_LEN + sizeof(CMD_STR_NET_WLAN_MAC)     + CMD_DATA_LEN_NET_WLAN_MAC-CMD_LENGTH_ONE)
#define CMD_LEN_NET_WLAN_ALL            (CMD_FIX_LEN + sizeof(CMD_STR_NET_WLAN_ALL)     + CMD_DATA_LEN_NET_WLAN_ALL-CMD_LENGTH_ONE)
#define CMD_LEN_NET_WLAN_PLUG_IN        (CMD_FIX_LEN + sizeof(CMD_STR_NET_WLAN_PLUG_IN) + CMD_DATA_LEN_NET_WLAN_PLUG_IN-CMD_LENGTH_ONE)
#define CMD_LEN_NET_WLAN_SSID           (CMD_FIX_LEN + sizeof(CMD_STR_NET_WLAN_SSID)    + CMD_DATA_LEN_NET_WLAN_SSID-CMD_LENGTH_ONE)
#define CMD_LEN_NET_WLAN_KEY            (CMD_FIX_LEN + sizeof(CMD_STR_NET_WLAN_KEY)     + CMD_DATA_LEN_NET_WLAN_KEY-CMD_LENGTH_ONE)
#define CMD_LEN_PRESENTER_KEY           (CMD_FIX_LEN + sizeof(CMD_STR_PRESENTER_KEY)    + CMD_DATA_LEN_PRESENTER_KEY-CMD_LENGTH_ONE)
//B35LB_David_0018 Mod End
#define CMD_LEN_NET_INIT_OK             (CMD_FIX_LEN + sizeof(CMD_STR_NET_INIT_OK)      + CMD_DATA_LEN_NULL)        //B35LB_Tim_0001, mod, *&

#define CMD_LEN_NET_TFTP_UPG            (CMD_FIX_LEN + strlen(CMD_STR_NET_TFTP_UPG)     + CMD_DATA_LEN_NET_TFTP_UPG)

#define CMD_LEN_TRISTATE                (CMD_FIX_LEN + strlen(CMD_STR_TRISTATE)         + CMD_DATA_LEN_TRISTATE)

#define CMD_LEN_SNMP_TRAP_Q             (CMD_FIX_LEN + strlen(CMD_STR_SNMP_TRAP_Q)      + CMD_DATA_LEN_NULL)

#define CMD_LEN_CRE_KEY_CMD             (sizeof(CMD_STR_CRE_CMD_MENU)) //B35LB_Kraz_0089, mod, strlen //Common_Tim_0328, add

// Command string length &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
//B35LB_David_0010 Add End
