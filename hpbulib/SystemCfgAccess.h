#ifndef _SYSTEMCFGACCESS_
#define _SYSTEMCFGACCESS_

#include "type_def.h"


#ifndef __UBUNTU_SIMULATOR__
#define SYSTEM_CFG_FILE_PATH             "/usr/configs/ProjectSettings/SystemCfg.cfg"
#else
#define SYSTEM_CFG_FILE_PATH             "../hpbulib/ProjectSettings/SystemCfg.cfg"
#endif /* __UBUNTU_SIMULATOR__ */

//#define CUSTOM_CFG_FILE_PATH             "./ConfigSetting/CustomCfg.cfg"
#define MAX_ITEMNAME_LENGTH (48)
#define MAX_ITEMVALUE_LENGTH (128)//(32)

#define CONF_MAX_ITEM (2048)

#define MAX_FAN_NUMBER_LENGTH 			(21)
#define MAX_LD_NUMBER_LENGTH  			(15)
#define MAX_TEC_NUMBER_LENGTH 			(3)
#define MAX_FILTER_INDEX_NUMBER_LENGTH  (6)
#define MAX_LIGHT_SENSOR_NUMBER_LENGTH  (8)
#define MAX_PUMP_NUMBER_LENGTH  		(2)
#define MAX_DMD_TEMP_NUMBER_LENGTH  	(2)
#define MAX_PRESSRE_NUMBER_LENGTH  		(2)
#define DEVICE_IS_EXIST                  1
#define DEVICE_IS_NOT_EXIST              0


//result status
#define EXEC_PASS (1)
#define NO_SYSCFG_FILE (-1)
#define MODEL_ID_ERROR (-2)



//#define DEBUG_MODE
#ifdef DEBUG_MODE
#define DbgMsg(x, ...) printf(x, ##__VA_ARGS__)
#else
#define DbgMsg(x, ...)
#endif

#pragma pack(push)  /* push current alignment to stack */
#pragma pack(1)     /* set alignment to 1 byte boundary */

#if 0
typedef enum
{
    /* 00 */ eDATA_TYPE_STRING,
    /* 01 */ eDATA_TYPE_UINT8,
    /* 02 */ eDATA_TYPE_UINT16,
    /* 03 */ eDATA_TYPE_UINT32,
    /* 04 */ eDATA_TYPE_UINT64,
    /* 05 */ eDATA_TYPE_INT8,
    /* 06 */ eDATA_TYPE_INT16,
    /* 07 */ eDATA_TYPE_INT32,
    /* 08 */ eDATA_TYPE_INT64,
    /* 09 */ eDATA_TYPE_FLOAT,
    /* 10 */ eDATA_TYPE_DOUBLE,
    /* 11 */ eDATA_TYPE_VOID,

    eDATA_TYPE_UNKNOWN,
}eDATA_TYPE_ID;
#endif

typedef enum
{
    /* 00 */ eDATA_TYPE_STRING,
    /* 01 */ eDATA_TYPE_UINT32,
    /* 02 */ eDATA_TYPE_INT32,
    /* 03 */ eDATA_TYPE_HEX,
    /* 04 */ eDATA_TYPE_FLOAT,
    /* 05 */ eDATA_TYPE_VOID,

    eDATA_TYPE_UNKNOWN,
}eCFG_DATA_TYPE_ID;


typedef struct
{
    char ItemName[MAX_ITEMNAME_LENGTH];
    char ItemValue[MAX_ITEMVALUE_LENGTH];
}sCFG_ITEM_FORMAT;

typedef struct
{
    UINT8 ucLD_Number;
    char acLD_Exist[MAX_LD_NUMBER_LENGTH]; //value'1' is exist ,value'0' is not exist
    UINT8 ucFan_Number;
    char acFan_Exist[MAX_FAN_NUMBER_LENGTH];
    UINT8 ucTEC_Number;
    char acTEC_Exist[MAX_TEC_NUMBER_LENGTH];
	UINT8 ucFilterIndex_Number;
    char acFilterIndex_Exist[MAX_FILTER_INDEX_NUMBER_LENGTH];
	UINT8 ucLightSensor_Number;
	char acLightSensor_Exist[MAX_LIGHT_SENSOR_NUMBER_LENGTH];  //WRGBYRY
	UINT8 ucDMD_Temp_Number;
    char ucDMD_Temp_Exist[MAX_DMD_TEMP_NUMBER_LENGTH];
	UINT8 ucPump_Number;
    char ucPump_Exist[MAX_PUMP_NUMBER_LENGTH];
	UINT8 ucPressure_Number;
    char ucPressure_Exist[MAX_PRESSRE_NUMBER_LENGTH];
}sCFG_DRIVER_INFO;

typedef enum
{
    // [0] Chip Config
    /* 00 */ eScalerType,
    /* 01 */ eC789_Output,
    /* 02 */ eMotorController,
    /* 03 */ eMotorDriverWR_WORD_REG,   //write/read I2C using WORD REG ?

    // [1] Chip Device Config
    /* 04 */ eWith_Xilinx_FPGA,
    /* 05 */ eXilinx_FPGA_I2C_BUS,
    /* 06 */ eXilinx_FPGA_I2C_ADDR,
    /* 07 */ eWith_System_MCU,
    /* 08 */ eSystem_I2C_BUS,
    /* 09 */ eSystem_I2C_ADDR,
    /* 10 */ eSystem_I2C_EnableChecksum,
    /* 11 */ eWith_Frontend_MCU,
    /* 12 */ eFrontend_I2C_BUS,
    /* 13 */ eFrontend_I2C_ADDR,
    /* 14 */ eWith_LDDRV_MCU,
    /* 15 */ eLDDRV_I2C_BUS,
    /* 16 */ eLDDRV_I2C_ADDR,
    /* 17 */ eWith_Motor_MCU,
    /* 18 */ eMotor_I2C_BUS,
    /* 19 */ eMotor_I2C_ADDR,
    /* 20 */ eWith_DDP,
    /* 21 */ eDDP_I2C_BUS,
    /* 22 */ eDDP_I2C_ADDR,
    /* 23 */ eWith_CDEC949_MCU,
    /* 24 */ eCDEC949_I2C_BUS,
    /* 25 */ eCDEC949_I2C_ADDR,
    /* 26 */ eWith_FPGA0_MCU,
    /* 27 */ eFPGA0_I2C_BUS,
    /* 28 */ eFPGA0_I2C_ADDR,
    /* 29 */ eWith_FPGA1_MCU,
    /* 30 */ eFPGA1_I2C_BUS,
    /* 31 */ eFPGA1_I2C_ADDR,
    /* 32 */ eWith_FPGA2_MCU,
    /* 33 */ eFPGA2_I2C_BUS,
    /* 34 */ eFPGA2_I2C_ADDR,
    /* 35 */ eWith_CDEC913_0_MCU,
    /* 36 */ eCDEC913_0_I2C_BUS,
    /* 37 */ eCDEC913_0_I2C_ADDR,
    /* 38 */ eWith_CDEC913_1_MCU,
    /* 39 */ eCDEC913_1_I2C_BUS,
    /* 40 */ eCDEC913_1_I2C_ADDR,
    /* 41 */ eWith_CDEC913_2_MCU,
    /* 42 */ eCDEC913_2_I2C_BUS,
    /* 43 */ eCDEC913_2_I2C_ADDR,

    // [2] Behavior Config
    /* 44 */ eCustomerID,
    /* 45 */ eDM_Reset_When_Select,  // - Display Mode
    /* 46 */ eDM_ImmediatelySave,    // - Display Mode

    // Model Name
    /* 47 */ eModelName_AlwaysNeutral,
    /* 48 */ eModelName_DependSN,
    /* 49 */ eModelName_DependNeutral,

    // Model ID
    /* 50 */ eModelID_Max,
    /* 51 */ eModelID_DependSN,

    // Geometry Seettings
    /* 52 */ eGeo_BehaviorType,    //eGEO_BEHAVIOR_TYPE  //0:support Twist/Mystique (Christie), 1:support PToolSet (Barco), 2:support visual suite (Optoma)
    /* 53 */ eGeo_BlackLevelType,  //eGEO_BLACKLEVEL_TYPE  //0:not support (Christie), 1:RS232 CLI only (Barco), 2:Blacklevel OSD (Optoma)
    /* 54 */ eGeo_APWarpFilterType, //eGEO_AP_WARPFILTER_TYPE  //0:auto, 1:manual

    // [3] Software Parameter Config
    /* 55 */ eWarmingUp_Time,
    /* 56 */ eCooling_Time,
    /* 57 */ eWith_FanFilter,
    /* 58 */ eNo_Light_LightSource,

    // [4] Hardware Parameter Config
	/* 59 */ eLD_Infomation,
	/* 60 */ eFan_Infomation,
	/* 61 */ eTEC_Infomation,
	/* 62 */ eFilterIndex_Infomation,
	/* 63 */ eLightSensor_Infomation,
	/* 64 */ eDMD_Temp_Infomation,
	/* 65 */ ePump_Infomation,
	/* 66 */ ePressure_Infomation,

    // Motor Lens Type
    /* 67 */ eMotorLensType,   //eMOTOR_LENSTYPE    //0:A70, 1:A35, 2:H60
    /* 68 */ eLensModuleType,  //0:T100, 1:A70LV, 2:A65, 3:X35_S600, 4:X35_TYPEA, 5:H60, 99:A16

    // LD Num define in MCU
	/* 69 */ eLD_INFO_Num_defined_in_MCU,
	/* 70 */ eBLD_Num_defined_in_MCU,
	/* 71 */ eRLD_Num_defined_in_MCU,

	// Fan Num define in MCU
	/* 72 */ eFan_INFO_Num_defined_in_MCU,

	// Thermal Sensor Num define in MCU
	/* 73 */ eTS_INFO_Num_defined_in_MCU,

	//Thermal Sensor use Segment
	/* 74 */ eLightSensorSegmentType,    //0:R/G/B/RR , 1:R/G/B/Y

	// [5] Special Firmware
	/* 75 */ eOE_JIG_ENABLE,
	/* 76 */ eDEMO_SNDS_ENABLE,
	/* 77 */ eCURSOR_FIXTURE_ENABLE,

    // [6] Datacode Settings
	/* 78 */ eDC_LightSensorInfoType,       //0:WRGBY , 1:RGBYRR
	/* 79 */ eDC_TEC_WithVoltage,           //0:only show Current, 1:show Voltage and Current
    /* 80 */ eDC_ThermalSensorWithFloat,    //0:show XX,  1:show XX.XX
    /* 81 */ eDC_DigitalZoomWithFloat,      //0:show 50~400,  1:show 50.00% ~ 400.00%

    // [7] OPD Log Temperature String
    /* 82 */ eOPD_Temperature_0,
    /* 83 */ eOPD_Temperature_1,
    /* 84 */ eOPD_Temperature_2,
    /* 85 */ eOPD_Temperature_3,
    /* 86 */ eOPD_Temperature_4,
    /* 87 */ eOPD_Temperature_5,
    /* 88 */ eOPD_Temperature_6,
    /* 89 */ eOPD_Temperature_7,

    eSystemItemMaxNumber
}eSYSTEM_CFG_ITEM;

typedef enum
{
    /* 00 */ eModel,
    /* 01 */ eMultiSync,
    /* 02 */ eBackupInput,

    eCustomItemMaxNumber,
}eCUSTOM_CFG_ITEM;

typedef enum
{
    /* 00 */ eLD_Number,
    /* 01 */ eFan_Number,
    /* 02 */ eTEC_Number,
	/* 03 */ eFilter_Index_Number,
	/* 04 */ eLight_Sensor_Number,
	/* 05 */ eDMD_Temp_Number,
	/* 06 */ ePump_Number,
	/* 07 */ ePressure_Number,
	/* 08 */ eLD_Exist,
	/* 09 */ eFan_Exist,
	/* 10 */ eTEC_Exist,
	/* 11 */ eFilter_Index_Exist,
	/* 12 */ eLight_Sensor_Exist,
	/* 13 */ eDMD_Temp_Exist,
	/* 14 */ ePump_Exist,
	/* 15 */ ePressure_Exist,

    eDriverItemMaxNumber,
}eDRIVER_CFG_ITEM;


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
typedef enum
{
    eST_ICHIPC821C789,
    eST_PROAV,
    eST_ICHIPC341,

    eST_NUMBER,
}eSCALER_TYPE;


typedef enum
{
    eC789_OUTPUT_TTL,
    eC789_OUTPUT_LVDS,

    eC789_OUTPUT_NUMBER,
}eC789_OUTPUT_TYPE;


typedef enum
{
    eMOTOR_CONTROLLER_SYS54605,      //R70G2
    eMOTOR_CONTROLLER_FMT54605,      //35G2
    eMOTOR_CONTROLLER_MOTOR54113,    //A70G2
    eMOTOR_CONTROLLER_GEC_CMD,       //H30

    eMOTOR_CONTROLLER_NUMBER,
}eMOTOR_CONTROLLER_TYPE;

typedef enum
{
    eMOTOR_LENSTYPE_A70,
    eMOTOR_LENSTYPE_A35,
    eMOTOR_LENSTYPE_H60,

    eMOTOR_LENSTYPE_NUMBER,
}eMOTOR_LENSTYPE;

typedef enum
{
    eGEO_SUPPORT_TWIST_MYSTIQUE,    //christie
    eGEO_SUPPORT_PTOOLSET,          //barco
    eGEO_SUPPORT_VISUAL_SUITE,      //optoma
    eGEO_SUPPORT_CORE_BLEND_AP_AND_BASIC_WARP_NO_MEMORY,  //fuji

    eGEO_BEHAVIOR_NUMBER
}eGEO_BEHAVIOR_TYPE;

typedef enum
{
    eGEO_BLACKLEVEL_NOT_SUPPORT,
    eGEO_BLACKLEVEL_CLI_ONLY,
    eGEO_BLACKLEVEL_OSD,
    eGEO_BLACKLEVEL_AP_ONLY,

    eGEO_BLACKLEVEL_NUMBER
}eGEO_BLACKLEVEL_TYPE;

typedef enum
{
    eGEO_AP_AUTO_WARPFILTER,
    eGEO_AP_MANUAL_WARPFILTER,

    eGEO_AP_WARPFILTER_NUMBER
}eGEO_AP_WARPFILTER_TYPE;

typedef enum
{
    eDC_LIGHT_SENSOR_INFO_TYPE_0,  //W_R_G_B_Y
    eDC_LIGHT_SENSOR_INFO_TYPE_1,  //R_G_B_Y_RR

    eDC_LIGHT_SENSOR_INFO_TYPE_NUMBER,
}eDC_LIGHT_SENSOR_INFO_TYPE;

typedef enum
{
    eDC_TEC_SHOW_TYPE_CURRENT,
    eDC_TEC_SHOW_TYPE_CURRENT_AND_VOLTAGE,

    eDC_TEC_SHOW_TYPE_NUMBER,
}eDC_TEC_SHOW_TYPE;

typedef enum
{
    eDC_LIGHT_SENSOR_SEGMENT_TYPE_0,  //R/G/B/RR
    eDC_LIGHT_SENSOR_SEGMENT_TYPE_1,  //R/G/B/Y

    eDC_LIGHT_SENSOR_SEGMENT_TYPE_NUMBER,
}eDC_LIGHT_SENSOR_SEGMENT_TYPE;


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


typedef struct
{
    UINT32 Index;
    eCFG_DATA_TYPE_ID eDataType;
    const char ItemName[MAX_ITEMNAME_LENGTH];
    union
    {
        char StringValue[MAX_ITEMVALUE_LENGTH];
        INT32  IntValue;
        DOUBLE FloatValue;
    };
}sCFG_SETTINGS;

#pragma pack(pop)                          // restore previous alignment

INT8 InitSystemCfg(void);
INT8 LoadValueByDatatype(sCFG_SETTINGS sCfgSettings[], UINT32 MaxItem, char *FilePath);
INT8 Syscfg_Value_Match(eSYSTEM_CFG_ITEM Item, ...);
void Syscfg_Value_Get(eSYSTEM_CFG_ITEM Item, void *Data);
INT32 Syscfg_Value_Get_Typeint(eSYSTEM_CFG_ITEM Item);
void Syscfg_Value_Set(eSYSTEM_CFG_ITEM Item, ...);
void Syscfg_SaveToFile(void);
void SaveToCfgFile(sCFG_SETTINGS sCfgSettings[], UINT16 TotalItem, char *FilePath);
void Drivercfg_Value_Set(UINT8 ucModelID,eDRIVER_CFG_ITEM Item,UINT8 ucIndex,UINT8 ucValue);
INT16 Drivercfg_Value_Get(UINT8 ucModelID,eDRIVER_CFG_ITEM Item,UINT8 ucIndex);
void Syscfg_PrintSystemCfg(void);

#define CFG_CUSTOMER_ID                 Syscfg_Value_Get_Typeint(eCustomerID)
#define CFG_GEO_BEHAVIORTYPE            Syscfg_Value_Get_Typeint(eGeo_BehaviorType)   //eGEO_BEHAVIOR_TYPE
#define CFG_GEO_BLACKLEVELTYPE          Syscfg_Value_Get_Typeint(eGeo_BlackLevelType)   //eGEO_BLACKLEVEL_TYPE
#define CFG_WITH_FANFILTER              Syscfg_Value_Get_Typeint(eWith_FanFilter)
#define CFG_DIGITAL_ZOOM_WITH_FLOAT     Syscfg_Value_Get_Typeint(eDC_DigitalZoomWithFloat)

#endif // _SYSTEMCFGACCESS_
