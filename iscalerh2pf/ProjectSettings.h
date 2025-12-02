#ifndef _CUSTOMSETTINGS_H_
#define _CUSTOMSETTINGS_H_

#include "Common.h"
#include "CustomDef.h"
#include "utilCommon.h"
#include "utilStorageCfg.h"

#ifdef GLOBAL_ALLOCATE
    #define SCLASS
#else
    #define SCLASS extern
#endif


typedef enum
{
    eCLI_TYPE_CHRISTIE,
    eCLI_TYPE_BARCO,
    eCLI_TYPE_OPTOMA,
    eCLI_TYPE_GENERIC,

    eCLI_TYPE_NUMBER,

}eCLI_INFO_TYPE;


typedef enum
{
    eDATACODE_SMCU_VERSION_IS_ISCALER_VERSION,
    eDATACODE_SMCU_VERSION_IS_LD_CONVERT_VERSION,

    eDATACODE_SMCU_VERSION_TYPE_NUMBER,

}eDATACODE_SMCU_VERSION_TYPE;



#if defined(PLATFORM_A70G2) || defined(PLATFORM_H60_2K)
typedef enum
{
    eHWVER_VERA = 0,
    eHWVER_VERB,
    eHWVER_VERC,
    eHWVER_VERD,
    eHWVER_VERE,
    eHWVER_VERF,
    eHWVER_VERG,
    eHWVER_VERH,
    eHWVER_VERI,
    eHWVER_VERJ,
    eHWVER_VERK,
    eHWVER_VERL,
    eHWVER_VERM,
    eHWVER_VERN,
    eHWVER_VERO,
    eHWVER_VERP = 15,

    eHWVER_INVALID,
} eHW_VERSION;

#else



typedef enum
{
    eHWVER_M04_VERA = 0,
    eHWVER_M04_VERB,
    eHWVER_M04_VERC,
    eHWVER_M04_VERD,
    eHWVER_M04_VERE,
    eHWVER_M04_VERF,
    eHWVER_M04_VERG,
    eHWVER_M04_VERH,
    eHWVER_M04_VERI,
    eHWVER_M04_VERJ,
    eHWVER_M04_VERK,
    eHWVER_M04_VERL,
    eHWVER_M04_VERM,
    eHWVER_M04_VERN = 13,

    eHWVER_M08_VERA,
    eHWVER_M08_VERB,
    eHWVER_M08_VERC,
    eHWVER_M08_VERD,
    eHWVER_M08_VERE,
    eHWVER_M08_VERF,
    eHWVER_M08_VERG,
    eHWVER_M08_VERH,
    eHWVER_M08_VERI,
    eHWVER_M08_VERJ,
    eHWVER_M08_VERK,
    eHWVER_M08_VERL,
    eHWVER_M08_VERM,
    eHWVER_M08_VERN,

    eHWVER_INVALID,
} eHW_VERSION;

#endif


typedef struct
{
    struct
    {
        UINT8 LD_Info_Num;
        UINT8 BLD_Num;
        UINT8 RLD_Num;
    }sLD_CFG;

    struct
    {
        UINT8 CLI_Type;   //custom type
    }sCLI_CFG;

    struct
    {
        UINT8 SupportSerialEcho;
        UINT8 SupportPin;   //Christie
        UINT8 SupportSourceList;   //Christie
    }sFN_CFG;

    struct
    {
        eDATACODE_SMCU_VERSION_TYPE SMCU_Version_Type;
        UINT8 Language_RemoveFirstStartupFlag;
    }sDataCode_CFG;
}sGLOBAL_SETTINGS;


////////////////////////////////////
//////////// Key /////////////
////////////////////////////////////
SCLASS eKEY_LIST EnterFactoryModeKey[4];
SCLASS eKEY_LIST EnterServiceModeKey[6];
SCLASS eKEY_LIST EnterDealerModeKey[4];
SCLASS UINT8 ServiceCode[4];


////////////////////////////////////
//////////// Formatter /////////////
////////////////////////////////////
SCLASS sWALL_COLOR_STRUCT sWallColorValues[eCM_PICTURE_SETTINGS_NUMBER][eCM_WALL_COLOR_NUMBER]; //HICC2_Casper_0016
//SCLASS sHSG_SETTING sCE_TABLE_SETTING_W16[eGUI_PICTURE_SETTINGS_NUMBER][eCOLOR_ENHANCEMENT_MAX_NUMBER];
//SCLASS sHSG_SETTING sCE_TABLE_SETTING_W20[eGUI_PICTURE_SETTINGS_NUMBER][eCOLOR_ENHANCEMENT_MAX_NUMBER];
SCLASS sHSG_SETTING sCE_TABLE_SETTING_DEFAULT[];


SCLASS sHSG_SETTING sHSG_TABLE_SETTING_MODEL2[];
SCLASS sHSG_SETTING sHSG_TABLE_SETTING_MODEL1[];
SCLASS sHSG_SETTING sHSG_TABLE_SETTING_MODEL0[];
SCLASS UINT8 Gamma_Mapping[];


////////////////////////////////////
//////////// DataMgr ///////////////
////////////////////////////////////
SCLASS sCOLOR_SETTING sColorSetting[];
SCLASS const char *pcPCBA_VER_Str[eHWVER_INVALID];


SCLASS UINT8 FormatterPictureSettingMapping[];

SCLASS sGLOBAL_SETTINGS sGlobalCfg;

void GlobalSettings_Init(void);


#endif  //_CUSTOMSETTINGS_H_


