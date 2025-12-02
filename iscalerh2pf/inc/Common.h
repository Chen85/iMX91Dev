
#ifndef _COMMON_H_
#define _COMMON_H_

#define HICC_VER (2)

#define BIST_ENABLE     1  //HICC2_Steven_0001

#define PALDATAMGR_ACCESS_WITHLOG

////////////// temp ///  //H2 wait review
//#define halWarping_TwistLinkFlag_Get(x) (0)
//#define halWarpOSD_Color_Initial(x)

//#define halWarping_Panel_Update(x)
//#define halWarping_OSD_ON()
//#define halWarping_GetFreezeImageState(x) (0)
//#define halWarping_FreezeImage(x)

//#define palSystem_PowerDown() palSystem_PowerDown_debug((char *)__FUNCTION__, __LINE__) //palSystem_PowerDown_Ori() //palSystem_PowerDown_debug((char *)__FUNCTION__, __LINE__)

//////////////

#define POSIX_COMPLIANT                         0
#define FREERTOS                                1
#define SYSTEM_OS_TYPE                          POSIX_COMPLIANT

#if (SYSTEM_OS_TYPE == FREERTOS)
#include "./FreeRTOS_Kernel/include/FreeRTOS.h"
#include "./FreeRTOS_Kernel/include/task.h"
#include "./FreeRTOS_Kernel/include/queue.h"
#include "./FreeRTOS_Kernel/include/semphr.h"
#include "./FreeRTOS_Kernel/include/event_groups.h"
#define MAILBOX_SIZE                            20  //sizeof(sMAIL_BOX_MESSAGE)
#define MAX_MSG_SIZE                            MAILBOX_SIZE
#define MSG_BUFFER_SIZE                         MAILBOX_SIZE

#elif (SYSTEM_OS_TYPE == POSIX_COMPLIANT)
#include "Posix.h"
#endif
#include "CommonType.h"
#include "hicc_config.h"
#include "type_def.h"
#include "SharedMemCtrl.h"
#include "DataMapping.h"
#include "timeout.h"
#include "MathAPI.h"
#include "utilMathAPI.h"
#include "utilDatabaseAPI.h"
#include "Platform.h"
#include "CommonData.h"
#include "cmd_ap.h"

#include <pthread.h>
#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <mqueue.h>
#include <errno.h>
#include <signal.h>
#include <math.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/time.h>

#include "SystemCfgAccess.h"
#include "CliCfgAccess.h"
#include "KeyCodeList.h"

extern int errno;

#ifdef PALDATAMGR_ACCESS_WITHLOG
extern eEXEC_CODE palDataMgr_Data_Access_WithLog(eDATA_CODE eDataCode, eDATA_ACCESS_MODE eAccessMode, void *pValue, const char *cFuncName, UINT32 ulLineNum);
#define palDataMgr_Data_Access(a,b,c) palDataMgr_Data_Access_WithLog(a,b,c,__FUNCTION__,__LINE__)
#endif
extern eEXEC_CODE palDataMgr_UI_EventSend_Debug(eDATA_CODE DataCode, UINT32 IntData, void *VoidData, const char *func, UINT32 LineNum);
#define palDataMgr_UI_EventSend(a,b,c) palDataMgr_UI_EventSend_Debug(a,b,c,(const char*)__FUNCTION__,(UINT32)__LINE__)

//#include "Release.h"
//#define ASSERT(x)
//#define ASSERT_ALWAYS()
//#define memcpy(a,b,c) (Error: Please using memcpy_core to replace memcpy)

//#define db_ALWAYS db_ASSERT   //formal release

#define CHANGE_PANEL_WORKAROUND

#define MOCKUP_STAGE                            0
#define EVT_STAGE                               1
#define DVT_STAGE                               2
#define PVT_STAGE                               3
#define MP_STAGE                                4
#define SCALER_BOARD_STAGE                      DVT_STAGE
//#define OE_JIG
//#define CURSOR_FIXTURE //HICC2_Doulas_0020
//#define DEMO_SNDS		 //HICC2_Doulas_0021
//#define DMD_BOARD_TEST   //HICC2_Doulas_0127

//scaler define from bb file (SCALER_TYPE)
#ifdef FPGA_F34     //H2PF_Simon_0038
    #define SCALER_FPGA_F34
    #define DATAPATH_PROAV
    #define FPGA_ENTRY_4K
    #define FRAME_SEQUENTIAL_3D_ULTRA
    #define FRC_BYPASS_ENABLE 1
    #define WUXGA240_32X32
    #ifndef SIMULATOR_ISCALER
    #define SAME_RAM_FOR_BEFORE_WARP_OSD_AND_SCALER_OSD
    #endif /* SIMULATOR_ISCALER */
    #define USE_SHM_POSD_SAD   //相關的 OSD address 在 shared memory 裡做存取 (CT 和 PF 要共用)
    #define SCALER_HDR_ENABLE TRUE
#elif defined(C821_C789)
    #define SCALER_ICHIPS
    #define SCALER_C821_C789
	#define DATAPATH_ICHIPS
    #define C789_WARPING_ENABLE
    #define INIT_VCXO949
#elif defined(C341)
    #define SCALER_ICHIPS
    #define SCALER_C341
    #ifndef SIMULATOR_ISCALER
    //#define SAME_RAM_FOR_BEFORE_WARP_OSD_AND_SCALER_OSD
    #endif /* SIMULATOR_ISCALER */
    #define H30_DEVELOP_TEMP_DEFINE
    //#define CAL_ALWAYS_DONE //H30K_David_0008
    #define C341GEO_2D_NO_FRAME_LOCK
    #define DATAPATH_ICHIPS
    #define C341_WARPING_ENABLE
    #define CHECKING_OUTPUT_3D_FIELD            //H30K_Doulas_0038//H30K_Doulas_0035
    #define WARP_ON_USE_FRAME_MEMORY_YUV422 FALSE   //TRUE : 開啟warping時 , Scaler Block 改為 YUV422 (20bit) 處理 , Warp Block 用 30bit 處理    //H2PF_Simon_0137
                                                    //FALSE：開啟warping時 , Scaler/Warp Block皆用24bit處理
    #define DEBUG_CHECK_WPOFV  FALSE
    #define SCALER_HDR_ENABLE  FALSE    //H2PF_Simon_0193
#else
    #error "!!! undefine scaler"
#endif

//define from bb file (OUTPUT_PANEL_CFG)
#ifdef OUTPUT_PANEL_4K
    #define CONFIG_4K_DISPLAY
#endif

//define from bb file (DMD_RESOLUTION)
#ifdef DMD_RES_WUXGA
    #define DMD_WUXGA
#elif defined(DMD_RES_HD)
    #define DMD_DHD
#else
    #error "!!! unknown DMD_RESOLUTION"
#endif


#define DATAMGR_ACCESS_TABLE_ENTRIES (edcINVALID) //(edcINVALID)  //(edcACTION_KEY_CODE + 1) //H2 wait review

#define IS_SINGLE_ISCALER_MODE() palSystem_SingleiScalerModeFlagGet()

#ifdef CUSTOM_BARCO
#define TEC_TESTER_JIG  0  //G100_Steven_0080
#define TEC_TESTER_COUNT 100
#define TEC_TESTER_En 95   //G100_Steven_0080
#endif

#define OPEN_WAP                                1  //G100_Steven_0011

#define QUEUE_SYSTEM_NAME                       "/MQUEUE_SYSTEM"
#define QUEUE_GUI_NAME                          "/MQUEUE_GUI"
#define QUEUE_UI_NAME                           "/MQUEUE_UI"
#define QUEUE_DATAPATH_NAME                     "/MQUEUE_DATAPATH"
#define QUEUE_DATAPATHSUB_NAME                  "/MQUEUE_DATAPATHSUB"
#define QUEUE_CLI_NAME                          "/MQUEUE_CLI"
#define QUEUE_POLLING_NAME                      "/MQUEUE_POLLING"
#define QUEUE_HOST_NAME                         "/MQUEUE_HOST"

#define MAX_MESSAGES                            10

#define MNT_PATH                                "/mnt"
#define CONF_PATH                               "/mnt/configs"
#define CONF_SCALER_PATH   	                    "/mnt/configs/scaler"
#define CONF_SCALER_SYSTEM_PATH	                "/mnt/configs/scaler/system"
#define CONF_SCALER_GUI_PATH   	                "/mnt/configs/scaler/gui"
#define CONF_SCALER_SOURCE_PATH	                "/mnt/configs/scaler/source"
#define CONF_SCALER_WAP_PATH   	                "/mnt/configs/scaler/WAP"   //G100_Steven_0011
#define CONF_SCALER_EDID_PATH                   "/mnt/configs/scaler/EDID"
#define CONF_SCALER_TP_PATH                     "/usr/configs/scaler/TestPattern"  //H2PF_Simon_0057 //H2PF_Simon_0063
#define CONF_SCALER_COLOR_PATH                  "/usr/configs/scaler/ColorTable"
#define CONF_SCALER_COLOR_PATH_FJ               "/mnt/configs/scaler/ColorTable"  // HICC20_Keven_0009
#define CONF_SCALER_GUI_WARPING	                "/mnt/configs/scaler/gui/Warping.conf"//H30K_David_0049



#define CONF_BACKUP
#ifdef CONF_BACKUP
#define CONF_BAK_PATH   	                    "/mnt/configs/bak"
#define CONF_BAK_SYSTEM_PATH	                "/mnt/configs/bak/system"
#define CONF_BAK_GUI_PATH   	                "/mnt/configs/bak/gui"
#define CONF_BAK_SOURCE_PATH	                "/mnt/configs/bak/source"
#define CONF_BAK_WAP_PATH   	                "/mnt/configs/bak/WAP"
#endif /* CONF_BACKUP */

#define BACKUP_PATH                             "/mnt/Backup"
#define BACKUP_SCALER_PATH                      "/mnt/Backup/scaler"
#define BACKUP_SCALER_SYSTEM_PATH               "/mnt/Backup/scaler/system"
#define BACKUP_SCALER_GUI_PATH                  "/mnt/Backup/scaler/gui"
#define BACKUP_SCALER_SOURCE_PATH               "/mnt/Backup/scaler/source"
#define BACKUP_SCALER_LAN_PATH                  "/mnt/Backup/scaler/Lan"
#define BACKUP_WARP_PATH                        "/mnt/Backup/Warp"
#define BACKUP_TAR_GZ                           "/mnt/Backup.tar.gz"

#define SYSLOG_LOG_PATH                         "/mnt/syslog/"
#define ERROR_LOG_PATH                          "/mnt/syslog/errorlog"  //move from "/mnt/configs/errorlog"
#define ERROR_LOG_TXT                           "/mnt/syslog/errorlog/ErrorLog.txt"  //move from "/mnt/configs/errorlog/ErrorLog.txt"

#define MODEL_CHANGE_LOG_PATH                   "/mnt/syslog/modelchange"
#define MODEL_CHANGE_LOG_TXT                    "/mnt/syslog/modelchange/ModelChange.txt"
#define MODEL_CHANGE_LOG_TXT_RESERVE            "/mnt/syslog/modelchange/ModelChangeReserve.txt"

#define OPD_LOG_PATH                            "/mnt/syslog/opd"  //"/mnt/configs/opd"
#define OPD_TAR_PATH                            "/mnt/syslog/opd/tar"

#define SST_INFO_PATH                           "/mnt/syslog/projectorinfo"
#define SST_INFO_TXT                            "/mnt/syslog/projectorinfo/ProjectorInfo.txt"

#define UPGRADE_LOG_PATH                        "/mnt/syslog/upgrade"
#define IMXAP_LOG_PATH                          "/mnt/syslog/imxap"

#define BIST_FILE_PATH                          "/mnt/syslog/BIST"  //HICC2_Steven_0001

#define CUSTOMER_VERSION_CONF  	                "/mnt/configs/scaler/system/CustomerVersion.conf"
#define COMMON_VERSION_CONF  	                "/mnt/configs/scaler/system/SystemVersion.conf"

#define COLOR_UNIFORMITY_PATH                   "/mnt/configs/scaler/CUDATA/"                       //G100_Tim_0012, add
#define COLOR_UNIFORMITY_DATA0                  "/mnt/configs/scaler/CUDATA/CU_Data0"
#define COLOR_UNIFORMITY_DATA1                  "/mnt/configs/scaler/CUDATA/CU_Data1"
#define COLOR_UNIFORMITY_DATA2                  "/mnt/configs/scaler/CUDATA/CU_Data2"
#define COLOR_UNIFORMITY_DATA3                  "/mnt/configs/scaler/CUDATA/CU_Data3"

#define OMS_DATA_PATH                           "/mnt/configs/scaler/OMS/"          //A65_OPTOMA_Jerry_0005

#ifdef CONF_BACKUP
#define COLOR_BAK_UNIFORMITY_PATH               "/mnt/configs/bak/CUDATA/"
#define OMS_BAK_DATA_PATH                       "/mnt/configs/bak/OMS/"
#endif /* CONF_BACKUP */

#define BLENDING_AP_SAVING_PATH                 "/mnt/configs/Geometry"
#define BLENDING_AP_WARP_SAVE_FILENAME          "/mnt/configs/Geometry/Warp%d"
#define BLENDING_AP_DBD_SAVE_FILENAME           "/mnt/configs/Geometry/Blend%d"
#define BLENDING_AP_BKLEVEL_SAVE_FILENAME       "/mnt/configs/Geometry/BlackLevel%d"

//for Barco command only blacklevel
//////////////////////////////////////////////////////////////////////////////////////////////////////
#define BLENDING_FILE_PATH                      "/mnt/configs/scaler/blending"
#define BLACKLEVEL_FILE                         "/mnt/configs/scaler/blending/blacklevel%d.dat"   //info and palette
#define BLACKLEVEL_RAW_DATA_FILE                "/mnt/configs/scaler/blending/blacklevel%d_raw_data.dat"  //A35G2_Simon_0093
#define BLACKLEVEL_TMP_FILE                     "/mnt/configs/scaler/blending/blacklevel_tmp.dat"
#define BLACKLEVEL_HANDLING_FILE                "/mnt/configs/scaler/blending/blacklevel_handling.dat"
//////////////////////////////////////////////////////////////////////////////////////////////////////

#define ADV_WAPR_PATH                           "/mnt/configs/Warp"


#if 0
#define BLENDING_AP_WARP_TPX_CURRENT_FILENAME       "/tmp/tpx.raw"
#define BLENDING_AP_WARP_TPY_CURRENT_FILENAME       "/tmp/tpy.raw"
#define BLENDING_AP_DBD_TABLE_CURRENT_FILENAME      "/tmp/dbd.raw"
#define BLENDING_AP_BKLEVEL_TABLE_CURRENT_FILENAME  "/tmp/black_level.raw"
#else
#define BLENDING_AP_WARP_TPX_CURRENT_FILENAME           "/mnt/configs/Geometry/Current_tpx.raw"
#define BLENDING_AP_WARP_TPY_CURRENT_FILENAME           "/mnt/configs/Geometry/Current_tpy.raw"
#define BLENDING_AP_DBD_TABLE_CURRENT_FILENAME          "/mnt/configs/Geometry/Current_dbd.raw"
#define BLENDING_AP_BKLEVEL_TABLE_CURRENT_FILENAME      "/mnt/configs/Geometry/Current_black_level.raw"
#define BLENDING_AP_BKLEVEL_PALETTE_CURRENT_FILENAME    "/mnt/configs/Geometry/Current_black_level_palette.raw"
#define BLENDING_AP_CURRENT_INFO_FILENAME               "/mnt/configs/Geometry/Current_INFO"    //A65_OPTOMA_Simon_0002
#endif

#define SCREEN_CAPTURE_PNG_FILENAME                     "/mnt/configs/Geometry/ScreenCapture.png"
#define REGDUMP_FILENAME                                "/mnt/configs/Geometry/regdump.txt"

#define BACKUP_CONFIG_PATH                      "/mnt/configs/BackupRestore"
#define BACKUP_CONFIG_0_PATH                    "/mnt/configs/BackupRestore/configs0"           //HICC2_Simon_0006
#define BACKUP_CONFIG_0_SCALER_EDID_PATH        "/mnt/configs/BackupRestore/configs0/scaler/EDID" // x35G2_Bruce_0005
#define BACKUP_CONFIG_0_SCALER_PATH             "/mnt/configs/BackupRestore/configs0/scaler"
#define BACKUP_CONFIG_0_SCALER_GUI_WARPING		"/mnt/configs/BackupRestore/configs0/scaler/gui/Warping.conf"//H30K_David_0049
#define BACKUP_CONFIG_1_PATH                    "/mnt/configs/BackupRestore/configs1"
#define BACKUP_CONFIG_1_SCALER_EDID_PATH        "/mnt/configs/BackupRestore/configs1/scaler/EDID" // x35G2_Bruce_0005
#define BACKUP_CONFIG_1_SCALER_PATH             "/mnt/configs/BackupRestore/configs1/scaler"
#define BACKUP_CONFIG_1_SCALER_GUI_WARPING		"/mnt/configs/BackupRestore/configs1/scaler/gui/Warping.conf"//H30K_David_0049
#define BACKUP_CONFIG_2_PATH                    "/mnt/configs/BackupRestore/configs2"
#define BACKUP_CONFIG_2_SCALER_PATH             "/mnt/configs/BackupRestore/configs2/scaler"
#define BACKUP_CONFIG_2_SCALER_EDID_PATH        "/mnt/configs/BackupRestore/configs2/scaler/EDID" // x35G2_Bruce_0005
#define BACKUP_CONFIG_2_SCALER_GUI_WARPING		"/mnt/configs/BackupRestore/configs2/scaler/gui/Warping.conf"//H30K_David_0049
#define BACKUP_CONFIG_3_PATH                    "/mnt/configs/BackupRestore/configs3"
#define BACKUP_CONFIG_3_SCALER_PATH             "/mnt/configs/BackupRestore/configs3/scaler"
#define BACKUP_CONFIG_3_SCALER_EDID_PATH        "/mnt/configs/BackupRestore/configs3/scaler/EDID" // x35G2_Bruce_0005
#define BACKUP_CONFIG_3_SCALER_GUI_WARPING		"/mnt/configs/BackupRestore/configs3/scaler/gui/Warping.conf"//H30K_David_0049
#define BACKUP_CONFIG_4_PATH                    "/mnt/configs/BackupRestore/configs4"
#define BACKUP_CONFIG_4_SCALER_PATH             "/mnt/configs/BackupRestore/configs4/scaler"
#define BACKUP_CONFIG_4_SCALER_EDID_PATH        "/mnt/configs/BackupRestore/configs4/scaler/EDID" // x35G2_Bruce_0005
#define BACKUP_CONFIG_4_SCALER_GUI_WARPING		"/mnt/configs/BackupRestore/configs4/scaler/gui/Warping.conf"//H30K_David_0049


#define USB_DEV_PATH                            "/run/media/sda1"


#define XFPGA_NEW_AUTO_MODE_V3_3				1	//G100_Doulas_0001


#define USB_DEV_MENU_UPGRADE_PATH               "/run/media/sda1/Upgrade"

#define DEFAULT_LANIP_FILE_PATH                 "/usr/configs/net.conf"
#define DEFAULT_IPV6_LANIP_FILE_PATH            "/usr/configs/net6.conf" //HICC2_AC_0078 // HICC2_Bruce_0020
#define LAN_VERSION_FILE_PATH                   "/usr/configs/version.conf"
#define RELEASE_VERSION_FILE_PATH               "/usr/configs/ProjectSettings/Release.cfg"
#define LENS_TABLE_FILE_PATH               		"/usr/configs/ProjectSettings/LensType.csv"

#ifndef SIMULATOR_ISCALER
#define OSD_BITMAP_RAWDATA	                    "/usr/configs/scaler/OSD/BitmapRawData.dat"
#define OSD_TEXT_RAWDATA  	                    "/usr/configs/scaler/OSD/TextRawData.dat"
#define BITMAP_STRUCT_JASON	                    "/usr/configs/scaler/OSD/BitmapStruct.json"
#define OSD_STRUCT_JASON  	                    "/usr/configs/scaler/OSD/OSDStruct.json"
#define TEXT_STRUCT_JASON  	                    "/usr/configs/scaler/OSD/TextStruct.json"
#if CUSTOM_CHRISTIE
#define CUSTOM_OSD_BITMAP_RAWDATA	             "/usr/configs/scaler/OSD_Christie/BitmapRawData.dat"
#define CUSTOM_OSD_TEXT_RAWDATA  	             "/usr/configs/scaler/OSD_Christie/TextRawData.dat"
#define CUSTOM_BITMAP_STRUCT_JASON	             "/usr/configs/scaler/OSD_Christie/BitmapStruct.json"
#define CUSTOM_OSD_STRUCT_JASON  	             "/usr/configs/scaler/OSD_Christie/OSDStruct.json"
#define CUSTOM_TEXT_STRUCT_JASON  	             "/usr/configs/scaler/OSD_Christie/TextStruct.json"
#elif CUSTOM_OPTOMA
#define CUSTOM_OSD_BITMAP_RAWDATA	             "/usr/configs/scaler/OSD_Optoma/BitmapRawData.dat"
#define CUSTOM_OSD_TEXT_RAWDATA  	             "/usr/configs/scaler/OSD_Optoma/TextRawData.dat"
#define CUSTOM_BITMAP_STRUCT_JASON	             "/usr/configs/scaler/OSD_Optoma/BitmapStruct.json"
#define CUSTOM_OSD_STRUCT_JASON  	             "/usr/configs/scaler/OSD_Optoma/OSDStruct.json"
#define CUSTOM_TEXT_STRUCT_JASON  	             "/usr/configs/scaler/OSD_Optoma/TextStruct.json"
#else
#define CUSTOM_OSD_BITMAP_RAWDATA	             "/usr/configs/scaler/OSD_Barco/BitmapRawData.dat"
#define CUSTOM_OSD_TEXT_RAWDATA  	             "/usr/configs/scaler/OSD_Barco/TextRawData.dat"
#define CUSTOM_BITMAP_STRUCT_JASON	             "/usr/configs/scaler/OSD_Barco/BitmapStruct.json"
#define CUSTOM_OSD_STRUCT_JASON  	             "/usr/configs/scaler/OSD_Barco/OSDStruct.json"
#define CUSTOM_TEXT_STRUCT_JASON  	             "/usr/configs/scaler/OSD_Barco/TextStruct.json"
#endif

#else
#if CUSTOM_CHRISTIE
#define OSD_BITMAP_RAWDATA	                    "./iScalerA65-1.0/CUSTOM_CHRISTIE/OSD/BitmapRawData.dat"
#define OSD_TEXT_RAWDATA  	                    "./iScalerA65-1.0/CUSTOM_CHRISTIE/OSD/TextRawData.dat"
#define BITMAP_STRUCT_JASON	                    "./iScalerA65-1.0/CUSTOM_CHRISTIE/OSD/BitmapStruct.json"
#define OSD_STRUCT_JASON  	                    "./iScalerA65-1.0/CUSTOM_CHRISTIE/OSD/OSDStruct.json"
#define TEXT_STRUCT_JASON  	                    "./iScalerA65-1.0/CUSTOM_CHRISTIE/OSD/TextStruct.json"
#define CUSTOM_OSD_BITMAP_RAWDATA	            "./iScalerA65-1.0/CUSTOM_CHRISTIE/OSD/BitmapRawData.dat"
#define CUSTOM_OSD_TEXT_RAWDATA  	            "./iScalerA65-1.0/CUSTOM_CHRISTIE/OSD/TextRawData.dat"
#define CUSTOM_BITMAP_STRUCT_JASON	            "./iScalerA65-1.0/CUSTOM_CHRISTIE/OSD/BitmapStruct.json"
#define CUSTOM_OSD_STRUCT_JASON  	            "./iScalerA65-1.0/CUSTOM_CHRISTIE/OSD/OSDStruct.json"
#define CUSTOM_TEXT_STRUCT_JASON  	            "./iScalerA65-1.0/CUSTOM_CHRISTIE/OSD/TextStruct.json"
#elif CUSTOM_OPTOMA
#define OSD_BITMAP_RAWDATA	                    "./iScalerA65-1.0/CUSTOM_OPTOMA/OSD/BitmapRawData.dat"
#define OSD_TEXT_RAWDATA  	                    "./iScalerA65-1.0/CUSTOM_OPTOMA/OSD/TextRawData.dat"
#define BITMAP_STRUCT_JASON	                    "./iScalerA65-1.0/CUSTOM_OPTOMA/OSD/BitmapStruct.json"
#define OSD_STRUCT_JASON  	                    "./iScalerA65-1.0/CUSTOM_OPTOMA/OSD/OSDStruct.json"
#define TEXT_STRUCT_JASON  	                    "./iScalerA65-1.0/CUSTOM_OPTOMA/OSD/TextStruct.json"
#define CUSTOM_OSD_BITMAP_RAWDATA	            "./iScalerA65-1.0/CUSTOM_OPTOMA/OSD/BitmapRawData.dat"
#define CUSTOM_OSD_TEXT_RAWDATA  	            "./iScalerA65-1.0/CUSTOM_OPTOMA/OSD/TextRawData.dat"
#define CUSTOM_BITMAP_STRUCT_JASON	            "./iScalerA65-1.0/CUSTOM_OPTOMA/OSD/BitmapStruct.json"
#define CUSTOM_OSD_STRUCT_JASON  	            "./iScalerA65-1.0/CUSTOM_OPTOMA/OSD/OSDStruct.json"
#define CUSTOM_TEXT_STRUCT_JASON  	            "./iScalerA65-1.0/CUSTOM_OPTOMA/OSD/TextStruct.json"
#else
#define OSD_BITMAP_RAWDATA	                    "./iScalerA65-1.0/CUSTOM_BARCO/OSD/BitmapRawData.dat"
#define OSD_TEXT_RAWDATA  	                    "./iScalerA65-1.0/CUSTOM_BARCO/OSD/TextRawData.dat"
#define BITMAP_STRUCT_JASON	                    "./iScalerA65-1.0/CUSTOM_BARCO/OSD/BitmapStruct.json"
#define OSD_STRUCT_JASON  	                    "./iScalerA65-1.0/CUSTOM_BARCO/OSD/OSDStruct.json"
#define TEXT_STRUCT_JASON  	                    "./iScalerA65-1.0/CUSTOM_BARCO/OSD/TextStruct.json"
#define CUSTOM_OSD_BITMAP_RAWDATA	            "./iScalerA65-1.0/CUSTOM_BARCO/OSD/BitmapRawData.dat"
#define CUSTOM_OSD_TEXT_RAWDATA  	            "./iScalerA65-1.0/CUSTOM_BARCO/OSD/TextRawData.dat"
#define CUSTOM_BITMAP_STRUCT_JASON	            "./iScalerA65-1.0/CUSTOM_BARCO/OSD/BitmapStruct.json"
#define CUSTOM_OSD_STRUCT_JASON  	            "./iScalerA65-1.0/CUSTOM_BARCO/OSD/OSDStruct.json"
#define CUSTOM_TEXT_STRUCT_JASON  	            "./iScalerA65-1.0/CUSTOM_BARCO/OSD/TextStruct.json"
#endif

#endif /* SIMULATOR_ISCALER */

#define RTC_RESET_PATH                          "/usr/scripts/set_rtc.sh"	//A35G2_BRC_Casper_0092

#define FW_UPGRADE_PATH          	            "/mnt/configs/scaler/FWPackage/"
#define FW_FPGA480          	                "/mnt/configs/scaler/FWPackage/FPGA_480.rpd"
#define FW_MCU_FRONTEND                         "/mnt/configs/scaler/FWPackage/lpc5460x_frondend.bin"
#define FW_MCU_MOTOR          	                "/mnt/configs/scaler/FWPackage/lpc5411x_motor_bd.bin"
#define FW_MCU_EXT          	                "/mnt/configs/scaler/FWPackage/ExtFlash.bin"

#ifndef LOGO_REPLACE
 #ifdef CUSTOM_OPTOMA
  #define LOGO_REPLACE                          1// set to 0 temporary //ac test
 #else
  #define LOGO_REPLACE                          0// 1
 #endif
#endif// R70G2_Bruce#0023

#define TMP_2ND_LOGO_RAWDATA_FILE	            "/tmp/colorcrush.raw"
#define TMP_2ND_LOGO_PALETTE_FILE	            "/tmp/colorcrush.palette"
#define OSD_2ND_LOGO_PALETTE_FILE	            "/mnt/configs/scaler/OSD/palette2ndLogo.bin"  //A65_OPTOMA_Doulas_0145 Modify
#define OSD_2ND_LOGO_SERVICE_PALETTE_FILE	    "/mnt/configs/scaler/OSD/palette2ndLogoService.bin"

#define OSD_2ND_LOGO_BITMAP_SIZE                (1920ul * 1200)
#define OSD_2ND_LOGO_PALETTE_SIZE               (119 * 3)

#define TP_USER_FILE_NAME_ID0
#define TP_USER_FILE_NAME_ID1
#define TP_USER_FILE_NAME_ID2
#define TP_BUILTIN_ACTUATOR_TP_FILE_NAME_ID0    "ActuatorTP0.png"   //H2PF_Simon_0057
#define TP_BUILTIN_ACTUATOR_TP_FILE_NAME_ID1    "ActuatorTP1.png"   //H2PF_Simon_0057

#if defined(CUSTOM_OPTOMA)
#define PROSERVICE_ALPHA_ENABLE //A65_OPTOMA_Jerry_0005
#define OSD_LOCK_ENABLE //A65_OPTOMA_Jerry_0005
#define KEYPAD_LOCK_ENABLE  (1)// HICC2_Bruce_0005
#define KEYPAD_UNLOCK_CNT   (24)
#define ENERGY_SAVING
//#define CLI_ERROR_LOG_ENABLE //A65_OPTOMA_Jerry_0006
#define QUICK_KEY_ENABLE	(1)
#define ENABLE_SINGLE_USER_MODE         // Only one User Mode, no USER-XXX      //H30K_Tim_0005, add
//#define ENABLE_GO_BACK_TO_LAST_MODE     // After 3D, 3D P, 2D HS, HDR, HLG      //H30K_Tim_0006, add
#endif

#define GENERAL_MSSC_CLI

//#define FIXED_MODEL_ID   //test only
#define DISABLE_POWERDOWN_DDP
//#define HOTKET_FOR_TEST_AF_AC_ACU_MESSAGE

#if(defined(CUSTOM_BARCO) || defined(CUSTOM_CHRISTIE))  //HICC2_Doulas_0068
#define CUSTOMIZED_EDID_ENABLE (TRUE)
#else
#define CUSTOMIZED_EDID_ENABLE (FALSE)
#endif

#define LAN_MOTOR_KEY_DOUBLE_RUN
#define NETWORK_IP_DHCP
#define NETWORK_IPV6_ENABLE		(1)

#if (SYSTEM_OS_TYPE != FREERTOS)

#define vTaskPrioritySet(x,y)
#define CLOCKS_PER_MSEC         (CLOCKS_PER_SEC/1000)
#define ClockTickGet()          Posix_Tick_Get()
#define xGetMSCount()           (ClockTickGet() / CLOCKS_PER_MSEC)
#define xTaskGetTickCount()     ClockTickGet()
#define pvPortMalloc(x)         malloc(x)
#define vPortFree(x)            free(x)

#ifndef portLONG
#define portLONG int
#endif

#ifndef portBASE_TYPE
#define portBASE_TYPE long
#endif

#ifndef portSHORT
#define portSHORT short
#endif

#ifndef StackType_t
#define StackType_t unsigned long
#endif

#ifndef BaseType_t
#define BaseType_t long
#endif

#ifndef UBaseType_t
#define UBaseType_t unsigned long
#endif

#ifndef portSTACK_TYPE
#define portSTACK_TYPE unsigned long
#endif

#ifndef portTICK_RATE_MS
#define portTICK_RATE_MS    (CLOCKS_PER_MSEC)   // 1000 in POSIX system
#endif

#ifndef tskIDLE_PRIORITY
#define tskIDLE_PRIORITY    (0)
#endif

#ifndef configTICK_RATE_HZ
#define configTICK_RATE_HZ  (CLOCKS_PER_SEC)   // 1000000 in POSIX system
#endif

#ifndef VERIFY_SIZE_OF //new STATIC_ASSERT macro, check static memory struct
#define VERIFY_SIZE_OF(TYPE, Size) extern UINT8 _VerifySizeof##TYPE[(sizeof(TYPE) == (Size)) / (sizeof(TYPE) == (Size))]
#endif

typedef unsigned long TickType_t;
typedef unsigned char EventGroupHandle_t;
typedef pthread_mutex_t SemaphoreHandle_t;
typedef pthread_mutex_t StaticSemaphore_t;
typedef unsigned char xQueueHandle ;
typedef unsigned char TimerHandle_t;
typedef unsigned char EventBits_t;

typedef INT32 tDATA_CODE;

#endif   //#if (SYSTEM_OS_TYPE != FREERTOS)

#define GPIO_LAN_POWER_ON GPIO5_IO24_CSI0_DAT6 //for V2 LAN board

//#define SEMAPHORE_LOG_DEBUG

#ifndef SIMULATOR_ISCALER
#define PROJECTOR_2K
#define MCU_CLI_TYPE_NA     0
#define MCU_CLI_TYPE_I2C    1
#define MCU_CLI_TYPE_UART   2

#define MCU_CLI_TYPE        MCU_CLI_TYPE_I2C

#define APP_BACKGROUND

#ifdef PLATFORM_H30_4K
#define OE_TOOL
#endif

#ifdef CONFIG_4K_DISPLAY
    #ifdef DMD_WUXGA
    #define PANEL_2D_HIGHSPEED  ePANEL_ID_WUXGA_240HZ
    #define PANEL_2D_OUTPUT     ePANEL_ID_3840x2400_60HZ
    #define PANEL_3D_OUTPUT     ePANEL_ID_WUXGA_120HZ
    #else
    #define PANEL_2D_HIGHSPEED  ePANEL_ID_1080P_240HZ
    #define PANEL_2D_OUTPUT     ePANEL_ID_3840x2160_60HZ
    #define PANEL_3D_OUTPUT     ePANEL_ID_1080P_120HZ
    #endif /* DMD_WUXGA */
#else
    #define PANEL_2D_HIGHSPEED  ePANEL_ID_1080P_120HZ //HICC2_Doulas_0133
    #define PANEL_3D_OUTPUT     ePANEL_ID_1080P_120HZ
    #ifdef DMD_WUXGA
    #define PANEL_2D_OUTPUT     ePANEL_ID_WUXGA_60HZ
    #else
    #define PANEL_2D_OUTPUT     ePANEL_ID_1080P_60HZ
    #endif
#endif /* CONFIG_4K_DISPLAY */



#ifdef CURSOR_FIXTURE
    #ifdef PLATFORM_H30_4K
        #undef PANEL_2D_HIGHSPEED
        #undef PANEL_2D_OUTPUT
        #undef PANEL_3D_OUTPUT
        #define PANEL_2D_HIGHSPEED  ePANEL_ID_3840x2400_60HZ
        #define PANEL_2D_OUTPUT     ePANEL_ID_WUXGA_240HZ
        #define PANEL_3D_OUTPUT     ePANEL_ID_WUXGA_120HZ
    #endif
#endif



//#define NO_INTERFACE

#define WARP_ENABLE

#if defined(CUSTOM_OPTOMA) || defined(CUSTOM_BARCO) || defined(CUSTOM_CHRISTIE)
#define ENABLE_COLOR_UNIFORMITY (TRUE)    //G100_Simon_0090  //A35G2_CDS_CL_0001
#else
#define ENABLE_COLOR_UNIFORMITY (FALSE)    //G100_Simon_0090  //A35G2_CDS_CL_0001
#endif
//#define C789_STRESS_TEST

#define NO_POWER_OFF_DURING_CAMERA_WORKING                  //G100_Tim_0046, add

#else
#define PROJECTOR_2K
//#define DMD_DHD
#define DMD_WUXGA  //G100_Wilsonj_0001
//#define CONFIG_4K_DISPLAY
#define MCU_CLI_TYPE_NA     0
#define MCU_CLI_TYPE_I2C    1
#define MCU_CLI_TYPE_UART   2

#define MCU_CLI_TYPE        MCU_CLI_TYPE_NA
//#define APP_BACKGROUND
#define FIXED_MODEL_ID

#ifdef CONFIG_4K_DISPLAY
    #ifdef DMD_WUXGA
    #define PANEL_2D_HIGHSPEED  ePANEL_ID_WUXGA_240HZ
    #define PANEL_2D_OUTPUT ePANEL_ID_3840x2400_60HZ
    #define PANEL_3D_OUTPUT ePANEL_ID_WUXGA_120HZ
    #else
    #define PANEL_2D_HIGHSPEED  ePANEL_ID_1080P_240HZ
    #define PANEL_2D_OUTPUT ePANEL_ID_3840x2160_60HZ
    #define PANEL_3D_OUTPUT ePANEL_ID_1080P_120HZ
    #endif /* DMD_WUXGA */
#else
    #define PANEL_2D_HIGHSPEED  ePANEL_ID_1080P_240HZ
    #define PANEL_3D_OUTPUT ePANEL_ID_1080P_120HZ
    #ifdef DMD_WUXGA
    #define PANEL_2D_OUTPUT ePANEL_ID_WUXGA_60HZ
    #else
    #define PANEL_2D_OUTPUT ePANEL_ID_1080P_60HZ
    #endif
#endif /* CONFIG_4K_DISPLAY */

#define NO_INTERFACE

#define WARP_ENABLE

#if defined(CUSTOM_OPTOMA) || defined(CUSTOM_BARCO) || defined(CUSTOM_CHRISTIE)
#define ENABLE_COLOR_UNIFORMITY (TRUE)    //G100_Simon_0090  //A35G2_CDS_CL_0001
#else
#define ENABLE_COLOR_UNIFORMITY (FALSE)    //G100_Simon_0090  //A35G2_CDS_CL_0001
#endif

#define NO_POWER_OFF_DURING_CAMERA_WORKING                  //G100_Tim_0046, add

#endif /* SIMULATOR_ISCALER */


#define MAIL_BOX_QUEUE_SIZE                     10
#define SEMPHORE_WAIT_DELAY                     2000 //1000

#define APP_SYSMON_STACK_SIZE                   (1024 * 1000)
#define APP_DATAPATH_STACK_SIZE                 (1024 * 1000)
#define APP_POLL_STACK_SIZE                     (1024 * 1000)
#define APP_GUI_STACK_SIZE                      (1024 * 1000)
#define APP_CLI_STACK_SIZE                      (1024 * 1000)
#define APP_HOST_STACK_SIZE                     (1024 * 1000)

#define APP_SYSMON_PRIORITY                     tskIDLE_PRIORITY + 6 //HIGH   //ori  tskIDLE_PRIORITY + 7
#define APP_DATAPATH_PRIORITY                   tskIDLE_PRIORITY + 6 //LOWER
#define APP_POLL_PRIORITY                       tskIDLE_PRIORITY + 6 //LOWER
#define APP_GUI_PRIORITY                        tskIDLE_PRIORITY + 6 //LOWER
#define APP_CLI_PRIORITY                        tskIDLE_PRIORITY + 6 //LOWER
#define APP_HOST_PRIORITY                       tskIDLE_PRIORITY + 6 //LOWER

#define APP_TASK_SEMPHORE_WAIT                  (180 * 1000)//15000 //180s       //G100_Owen_0076 //G100_Simon_0005

#define CORE_BOARD                              0
#define ICHIP_BOARD                             1
#define NO_BOARD                                2
#define CURRENT_MAIN_BOARD                      CORE_BOARD

#define USB_WRITE                               0
#define SPI_WRITE                               1
#define CURRENT_WRITE_MODE                      USB_WRITE


//RTOS Use Behavior.Memory allocate dynamic or static
#define RTOS_DYNAMIC                            0
#define RTOS_STATIC                             1
#define CURRENT_RTOS_TYPE                       RTOS_DYNAMIC


#define PLATFORM_4K                             0
#define PLATFORM_4K_C790                        1
#define PLATFORM_2K                             2
#define PLATFORM_2K_C787                        3
#define CURRENT_PLATFORM                        PLATFORM_2K//PLATFORM_4K_C790//PLATFORM_4K A70LV_Doulas_2000 modify

#define LCD_PANEL                               0
#define DLP_PANEL                               1
#define PLATFORM_PANEL                          DLP_PANEL //LCD_PANEL


#define LOG_TABLE_ENABLE                        0
#define LOG_QT_TABLE_ENABLE                     0
#define LOG_WARP_TABLE_ENABLE                   0
#define LOG_MVY_COEF_ENABLE                     0
#define LOG_H_LINE_ENABLE                       0
#define LOG_H_LINE_COEF_ENABLE                  0
#define LOG_LUT_ENABLE                          0
#define LOG_GRID_OSD_ENABLE                     0

#define LOG_ITE6802_REG                         0

#define CORRECT_FOCUS_DIRECTION //A35G2_Simon_0109  //need FMT54605 version >= E01.10

#define WAP_PICTURE_MODE_MAX 16

#ifndef NULL_VALUE
    #define NULL_VALUE 0
#endif

#define EVT2_STAGE

#if (CURRENT_MAIN_BOARD == ICHIP_BOARD)
//#include <time.h>// timer (wait timeout)
//#include <windows.h>// for VC Sleep() used
//#include "iEBfunc.h"


typedef int BOOL, *PBOOL ;   // fg
typedef double DOUBLE, *PDOUBLE ;
//#include <basetsd.h>
#define TRUE            1
#define FALSE            0



//#define MS_SLEEP(x)    Sleep(x)

//#define LOG_MSG(mask, fmt, ...)  appDbgMsg_ftrace(mask, fmt, __VA_ARGS__)

#else

#if (SYSTEM_OS_TYPE == POSIX_COMPLIANT)
#define US_SLEEP(x)   usleep(x)
#define MS_SLEEP(x)   US_SLEEP(x * 1000)
#define vTaskDelay(x) MS_SLEEP(x)
#else
#define MS_SLEEP(x)  vTaskDelay((configTICK_RATE_HZ * x) / 1000)  //sys_msleep(x)
#endif
typedef float FLOAT, *PFLOAT ;
//#define LOG_MSG(mask, fmt, ...)
//#define ASSERT(x)   if(x == 0) return;
#endif

#if (CURRENT_MAIN_BOARD == CORE_BOARD)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdarg.h>
//#include "Chip.h"
//#include "GenericTypeDefs.h"
//#include "board.h"

#endif

#define min(A,B)    ((A) < (B) ? (A) : (B))

#define BIT_0		(1 << 0)
#define BIT_1		(1 << 1)
#define BIT_2		(1 << 2)
#define BIT_3		(1 << 3)
#define BIT_4		(1 << 4)
#define BIT_5		(1 << 5)
#define BIT_6		(1 << 6)
#define BIT_7		(1 << 7)
#define STRINGER(x) #x
#define RGB888TORGB565(r,g,b) (((r & 0b11111000) << 8) | ((g & 0b11111100) << 3) | (b >> 3))

//G100_John_0003 start fix EDID version change issue
//Customer define
#define SN_LENGTH                   28 //Should mapping to FrondEnd
 //G100_John_0003 end

#define MOTOR_SN_NUMBER         (32) //Should mapping to Motor SN_NUMBER

#define FACTORY_RESET_NUMBER        (0x55)
#define SINGLE_BOARD_NUMBER         (0x66)
#define SYSTEM_CHECK_NUMBER         (0x77)

//====================== Enum Start ======================//

typedef enum
{
    LOW = 0,
    HIGH,

    ePIN_STATE_NUMBERS,
} ePIN_STATE;



#if 0	//G100_Doulas_0032 move to hpbulib
typedef enum
{
    eBACKUP_RESTORE_RESULT_PASS,                         /* pass */
    eBACKUP_RESTORE_RESULT_FAIL,                         /* general fail indication */
    eBACKUP_RESTORE_RESULT_INDEX_FAIL,                   /* input index fail */
    eBACKUP_RESTORE_RESULT_MAX,
} eBACKUP_RESTORE_RESULT;		//G100_Doulas_0031 Add
#endif

//====================== Enum End ======================//


//===================== Struct Start ==================//


//====================== Struct End ====================//
BOOL Board_SingleBoard_Get(void);
void Board_SingleBoard_Set(BOOL bValue);
void Board_MCU_ModelID_Set(UINT8 ucModelID);
UINT8 Board_MCU_ModelID_Get(void);
void Board_ModelID_Set(UINT8 ucModelID);
UINT8 Board_ModelID_Get(void);

UINT8 Board_Stage_Get(void);

#define Board_Stage_Get() DVT_STAGE

#endif   //_COMMON_H_

