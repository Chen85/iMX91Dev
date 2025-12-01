#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include <sys/file.h>
#include "SystemCfgAccess.h"
#include "conf.h"
#include "CommonType.h"
#include "hicc_config.h"
#include "GeneralFunc.h"

sCFG_SETTINGS SystemCfg[] =
{
    // [0] Chip Config
    {eScalerType,               	eDATA_TYPE_INT32,   "ScalerType",               	.IntValue=0},
    {eC789_Output,              	eDATA_TYPE_INT32,   "C789_Output",              	.IntValue=eC789_OUTPUT_TTL},
    {eMotorController,          	eDATA_TYPE_INT32,   "MotorController",          	.IntValue=eMOTOR_CONTROLLER_SYS54605},
    {eMotorDriverWR_WORD_REG,       eDATA_TYPE_INT32,   "MotorDriverWR_WORD_REG",       .IntValue=0},

    // [1] Chip I2C config
    {eWith_Xilinx_FPGA,         	eDATA_TYPE_INT32,   "With_Xilinx_FPGA",         	.IntValue=0},
    {eXilinx_FPGA_I2C_BUS,      	eDATA_TYPE_INT32,   "Xilinx_FPGA_I2C_BUS",      	.IntValue=2},
    {eXilinx_FPGA_I2C_ADDR,     	eDATA_TYPE_HEX,     "Xilinx_FPGA_I2C_ADDR",     	.IntValue=0x90},
    {eWith_System_MCU,          	eDATA_TYPE_INT32,   "With_System_MCU",          	.IntValue=1},
    {eSystem_I2C_BUS,           	eDATA_TYPE_INT32,   "System_I2C_BUS",           	.IntValue=2},
    {eSystem_I2C_ADDR,          	eDATA_TYPE_HEX,     "System_I2C_ADDR",          	.IntValue=0x78},
    {eSystem_I2C_EnableChecksum,	eDATA_TYPE_INT32,   "System_I2C_EnableChecksum",	.IntValue=1},
    {eWith_Frontend_MCU,        	eDATA_TYPE_INT32,   "With_Frontend_MCU",        	.IntValue=1},
    {eFrontend_I2C_BUS,         	eDATA_TYPE_INT32,   "Frontend_I2C_BUS",         	.IntValue=0},
    {eFrontend_I2C_ADDR,        	eDATA_TYPE_HEX,     "Frontend_I2C_ADDR",        	.IntValue=0x72},
    {eWith_LDDRV_MCU,           	eDATA_TYPE_INT32,   "With_LDDRV_MCU",           	.IntValue=1},
    {eLDDRV_I2C_BUS,            	eDATA_TYPE_INT32,   "LDDRV_I2C_BUS",            	.IntValue=0},
    {eLDDRV_I2C_ADDR,           	eDATA_TYPE_HEX,     "LDDRV_I2C_ADDR",           	.IntValue=0x74},
    {eWith_Motor_MCU,           	eDATA_TYPE_INT32,   "With_Motor_MCU",           	.IntValue=1},
    {eMotor_I2C_BUS,            	eDATA_TYPE_INT32,   "Motor_I2C_BUS",            	.IntValue=0},
    {eMotor_I2C_ADDR,           	eDATA_TYPE_HEX,     "Motor_I2C_ADDR",           	.IntValue=0x74},
    {eWith_DDP,          	        eDATA_TYPE_INT32,   "With_DDP",          	        .IntValue=1},
    {eDDP_I2C_BUS,              	eDATA_TYPE_INT32,   "DDP_I2C_BUS",              	.IntValue=0},
    {eDDP_I2C_ADDR,             	eDATA_TYPE_HEX,     "DDP_I2C_ADDR",             	.IntValue=0x34},
    {eWith_CDEC949_MCU,         	eDATA_TYPE_INT32,   "With_CDEC949_MCU",         	.IntValue=1},
    {eCDEC949_I2C_BUS,          	eDATA_TYPE_INT32,   "CDEC949_I2C_BUS",          	.IntValue=0},
    {eCDEC949_I2C_ADDR,         	eDATA_TYPE_HEX,     "CDEC949_I2C_ADDR",         	.IntValue=0x64},
    {eWith_FPGA0_MCU,           	eDATA_TYPE_INT32,   "With_FPGA0_MCU",           	.IntValue=1},
    {eFPGA0_I2C_BUS,            	eDATA_TYPE_INT32,   "FPGA0_I2C_BUS",            	.IntValue=2},
    {eFPGA0_I2C_ADDR,           	eDATA_TYPE_HEX,     "FPGA0_I2C_ADDR",           	.IntValue=0x34},
    {eWith_FPGA1_MCU,           	eDATA_TYPE_INT32,   "With_FPGA1_MCU",           	.IntValue=1},
    {eFPGA1_I2C_BUS,            	eDATA_TYPE_INT32,   "FPGA1_I2C_BUS",            	.IntValue=2},
    {eFPGA1_I2C_ADDR,           	eDATA_TYPE_HEX,     "FPGA1_I2C_ADDR",           	.IntValue=0x36},
    {eWith_FPGA2_MCU,           	eDATA_TYPE_INT32,   "With_FPGA2_MCU",           	.IntValue=0},
    {eFPGA2_I2C_BUS,            	eDATA_TYPE_INT32,   "FPGA2_I2C_BUS",            	.IntValue=0},
    {eFPGA2_I2C_ADDR,           	eDATA_TYPE_HEX,     "FPGA2_I2C_ADDR",           	.IntValue=0x00},
    {eWith_CDEC913_0_MCU,           eDATA_TYPE_INT32,   "With_CDEC913_0_MCU",           .IntValue=0},
    {eCDEC913_0_I2C_BUS,            eDATA_TYPE_INT32,   "CDEC913_0_I2C_BUS",            .IntValue=0},
    {eCDEC913_0_I2C_ADDR,           eDATA_TYPE_HEX,     "CDEC913_0_I2C_ADDR",           .IntValue=0x00},
    {eWith_CDEC913_1_MCU,           eDATA_TYPE_INT32,   "With_CDEC913_1_MCU",           .IntValue=0},
    {eCDEC913_1_I2C_BUS,            eDATA_TYPE_INT32,   "CDEC913_1_I2C_BUS",            .IntValue=0},
    {eCDEC913_1_I2C_ADDR,           eDATA_TYPE_HEX,     "CDEC913_1_I2C_ADDR",           .IntValue=0x00},
    {eWith_CDEC913_2_MCU,           eDATA_TYPE_INT32,   "With_CDEC913_2_MCU",           .IntValue=0},
    {eCDEC913_2_I2C_BUS,            eDATA_TYPE_INT32,   "CDEC913_2_I2C_BUS",            .IntValue=0},
    {eCDEC913_2_I2C_ADDR,           eDATA_TYPE_HEX,     "CDEC913_2_I2C_ADDR",           .IntValue=0x00},

    // [2] Behavior Config
    {eCustomerID,                   eDATA_TYPE_HEX,     "CustomerID",     	            .IntValue=0xFF},
    // => Display Mode
    {eDM_Reset_When_Select,         eDATA_TYPE_INT32,   "DM_Reset_When_Select",     	.IntValue=0},
    {eDM_ImmediatelySave,           eDATA_TYPE_INT32,   "DM_ImmediatelySave",       	.IntValue=0},

    // Model Name
    {eModelName_AlwaysNeutral,      eDATA_TYPE_INT32,   "ModelName_AlwaysNeutral",      .IntValue=0},
    {eModelName_DependSN,           eDATA_TYPE_INT32,   "ModelName_DependSN",       	.IntValue=0},
    {eModelName_DependNeutral,      eDATA_TYPE_INT32,   "ModelName_DependNeutral",      .IntValue=0},

    // Model ID
    {eModelID_Max,             		eDATA_TYPE_INT32,   "ModelID_Max",       	    	.IntValue=0},
    {eModelID_DependSN,             eDATA_TYPE_INT32,   "ModelID_DependSN",       	    .IntValue=0},

    //Geometry Settings
	{eGeo_BehaviorType,     	    eDATA_TYPE_INT32,   "Geo_BehaviorType",     	        .IntValue=0},   //eGEO_BEHAVIOR_TYPE
	{eGeo_BlackLevelType,     	    eDATA_TYPE_INT32,   "Geo_BlackLevelType",     	        .IntValue=0},   //eGEO_BLACKLEVEL_TYPE
	{eGeo_APWarpFilterType,         eDATA_TYPE_INT32,   "Geo_APWarpFilterType",             .IntValue=0},   //eGEO_AP_WARPFILTER_TYPE

    // [3] Software Parameter Config
    // =>System
    {eWarmingUp_Time,               eDATA_TYPE_INT32,   "WarmingUp_Time",           	    .IntValue=30},
    {eCooling_Time,             	eDATA_TYPE_INT32,   "Cooling_Time",             	    .IntValue=20},
    {eWith_FanFilter,             	eDATA_TYPE_INT32,   "With_FanFilter",             	    .IntValue=0},

    // =>Light Source
    {eNo_Light_LightSource,     	eDATA_TYPE_INT32,   "No_Light_LightSource",     	    .IntValue=0},

	// LD Infomation //eg: 10,11100111001111 => 10 LD number ; 11100111001111 is LD1 LD2 LD3 LD4 ... exist 1 / not exist 0
	{eLD_Infomation,     	    	eDATA_TYPE_STRING,  "LD_Infomation",     	    	    .StringValue="0"},

	// Fan Infomation
	{eFan_Infomation,     	    	eDATA_TYPE_STRING,  "Fan_Infomation",     	    	    .StringValue="0"},

	// TEC Infomation
	{eTEC_Infomation,     	    	eDATA_TYPE_STRING,  "TEC_Infomation",     	    	    .StringValue="0"},

	// Filter Index Infomation
	{eFilterIndex_Infomation,   	eDATA_TYPE_STRING,  "FilterIndex_Infomation",   	    .StringValue="0"},

	// Light Sensor Infomation  // WRGBYRY ,WRGB is BLD+RLD ,RY is BLD
	{eLightSensor_Infomation,   	eDATA_TYPE_STRING,  "LightSensor_Infomation",  		    .StringValue="0"},

	// DMD Temp Infomation
	{eDMD_Temp_Infomation,     		eDATA_TYPE_STRING,  "DMD_Temp_Infomation",     		    .StringValue="0"},

	// Pump Infomation
	{ePump_Infomation,     			eDATA_TYPE_STRING,  "Pump_Infomation",     			    .StringValue="0"},

	// Pressure Infomation
	{ePressure_Infomation,     		eDATA_TYPE_STRING,  "Pressure_Infomation",     		    .StringValue="0"},

    //Motor
	{eMotorLensType,     	        eDATA_TYPE_INT32,   "MotorLensType",     	            .IntValue=0},
	{eLensModuleType,     	        eDATA_TYPE_INT32,   "LensModuleType",     	            .IntValue=0},

    //LD Num define in MCU
	{eLD_INFO_Num_defined_in_MCU,   eDATA_TYPE_INT32,   "LD_INFO_Num_defined_in_MCU",       .IntValue=14},
	{eBLD_Num_defined_in_MCU,     	eDATA_TYPE_INT32,   "BLD_Num_defined_in_MCU",     	    .IntValue=10},
	{eRLD_Num_defined_in_MCU,     	eDATA_TYPE_INT32,   "RLD_Num_defined_in_MCU",     	    .IntValue=4},

    //Fan Num define in MCU
	{eFan_INFO_Num_defined_in_MCU,  eDATA_TYPE_INT32,   "Fan_INFO_Num_defined_in_MCU",      .IntValue=16},

    //Thermal Sensor Num define in MCU
	{eTS_INFO_Num_defined_in_MCU,   eDATA_TYPE_INT32,   "TS_INFO_Num_defined_in_MCU",       .IntValue=3},

	//Light Sensor use Segment
	{eLightSensorSegmentType,       eDATA_TYPE_INT32,   "LightSensorSegmentType",     	    .IntValue=1},

	// [5] Special Firmware
	{eOE_JIG_ENABLE,     	        eDATA_TYPE_INT32,   "OE_JIG_ENABLE",     	            .IntValue=0},
	{eDEMO_SNDS_ENABLE,     	    eDATA_TYPE_INT32,   "DEMO_SNDS_ENABLE",     	        .IntValue=0},
	{eCURSOR_FIXTURE_ENABLE,     	eDATA_TYPE_INT32,   "CURSOR_FIXTURE_ENABLE",     	    .IntValue=0},

    // [6] DataCode Settings
	{eDC_LightSensorInfoType,     	eDATA_TYPE_INT32,   "DC_LightSensorInfoType",     	    .IntValue=1},
	{eDC_TEC_WithVoltage,     	    eDATA_TYPE_INT32,   "DC_TEC_WithVoltage",     	        .IntValue=1},
	{eDC_ThermalSensorWithFloat,    eDATA_TYPE_INT32,   "DC_ThermalSensorWithFloat",        .IntValue=1},
	{eDC_DigitalZoomWithFloat,      eDATA_TYPE_INT32,   "DC_DigitalZoomWithFloat",          .IntValue=0},

	{eOPD_Temperature_0,            eDATA_TYPE_STRING,  "OPD_Temperature_0",                .StringValue="Temperature Sys"},
	{eOPD_Temperature_1,            eDATA_TYPE_STRING,  "OPD_Temperature_1",                .StringValue="Temperature DMD"},
	{eOPD_Temperature_2,            eDATA_TYPE_STRING,  "OPD_Temperature_2",                .StringValue="Temperature Envi"},
	{eOPD_Temperature_3,            eDATA_TYPE_STRING,  "OPD_Temperature_3",                .StringValue="Temperature Reserve"},
	{eOPD_Temperature_4,            eDATA_TYPE_STRING,  "OPD_Temperature_4",                .StringValue="Temperature DMD2"},
	{eOPD_Temperature_5,            eDATA_TYPE_STRING,  "OPD_Temperature_5",                .StringValue="Temperature Comb"},
	{eOPD_Temperature_6,            eDATA_TYPE_STRING,  "OPD_Temperature_6",                .StringValue="Temperature FPGA"},
	{eOPD_Temperature_7,            eDATA_TYPE_STRING,  "OPD_Temperature_7",                .StringValue="Temperature L1G"}
};

sCFG_DRIVER_INFO *DriverItemCfg = NULL;

INT8 InitDriverCfg(void)
{
	//UINT8 *DriverCfgAddr = NULL;
	UINT32 ulFileSize = 0;
    UINT32 Number = 0;

	char  cLD_Info[MAX_ITEMVALUE_LENGTH] = "0";
	char  cFan_Info[MAX_ITEMVALUE_LENGTH] = "0";
	char  cTEC_Info[MAX_ITEMVALUE_LENGTH] = "0";
	char  cFilterIndex_Info[MAX_ITEMVALUE_LENGTH] = "0";
	char  cLightSensor_Info[MAX_ITEMVALUE_LENGTH] = "0";
	char  cDMD_Temp_Info[MAX_ITEMVALUE_LENGTH] = "0";
	char  cPump_Info[MAX_ITEMVALUE_LENGTH] = "0";
	char  cPressure_Info[MAX_ITEMVALUE_LENGTH] = "0";

	INT32 wModeID_Max = 0;  //MODEL_ID_0
	UINT8 ucCount = 0;
	int len = 0;

	char  cLD_Cat[32] = "0";
	char  cFan_Cat[32] = "0";
	char  cTEC_Cat[32] = "0";
	char  cFilterIndex_Cat[32] = "0";
	char  cLightSensor_Cat[32] = "0";
	char  cDMD_Temp_Cat[32] = "0";
	char  cPump_Cat[32] = "0";
	char  cPressure_Cat[32] = "0";
	char  Temp[32] = "0";

	char *pinStr_LD = cLD_Info;
	char *pinStr_Fan = cFan_Info;
	char *pinStr_Tec = cTEC_Info;
	char *pinStr_FilterIndex = cFilterIndex_Info;
	char *pinStr_LightSensor = cLightSensor_Info;
	char *pinStr_DMD_Temp = cDMD_Temp_Info;
	char *pinStr_Pump = cPump_Info;
	char *pinStr_Pressure = cPressure_Info;

	Syscfg_Value_Get(eModelID_Max,&wModeID_Max);
	if(wModeID_Max == 0)
		return MODEL_ID_ERROR;

	//Dynamic Memory Allocation
	ulFileSize = wModeID_Max * sizeof(sCFG_DRIVER_INFO); //memory size

    if(DriverItemCfg == NULL)
    {
	    DriverItemCfg = (sCFG_DRIVER_INFO *)malloc(ulFileSize);
        memset(DriverItemCfg, 0, ulFileSize);
    }
	//printf("\n!!! DriverCfg size (0x%x) !!!\n", ulFileSize);

	//get info data
	Syscfg_Value_Get(eLD_Infomation,cLD_Info);
	Syscfg_Value_Get(eFan_Infomation,cFan_Info);
	Syscfg_Value_Get(eTEC_Infomation,cTEC_Info);
	Syscfg_Value_Get(eFilterIndex_Infomation,cFilterIndex_Info);
	Syscfg_Value_Get(eLightSensor_Infomation,cLightSensor_Info);
	Syscfg_Value_Get(eDMD_Temp_Infomation,cDMD_Temp_Info);
	Syscfg_Value_Get(ePump_Infomation,cPump_Info);
	Syscfg_Value_Get(ePressure_Infomation,cPressure_Info);

	#if 0
	printf("\n!!! ModelID Max (%d) !!!\n", wModeID_Max);
	printf("\n!!! LD  (%s) !!!\n", cLD_Info);
	printf("\n!!! Fan (%s) !!!\n", cFan_Info);
	printf("\n!!! TEC (%s) !!!\n", cTEC_Info);
	printf("\n!!! FI  (%s) !!!\n", cFilterIndex_Info);
	printf("\n!!! LS  (%s) !!!\n", cLightSensor_Info);
	printf("\n!!! DMD (%s) !!!\n", cDMD_Temp_Info);
	printf("\n!!! Pump (%s) !!!\n", cPump_Info);
	printf("\n!!! Pressure (%s) !!!\n", cPressure_Info);
	#endif

	for(ucCount=0 ; ucCount < wModeID_Max ;ucCount++)
	{
		//LD
		memset(Temp, 0, 32);
		sscanf(pinStr_LD, "%[^ ] %n", cLD_Cat, &len);
		sscanf(cLD_Cat, "%d,%s", &Number,Temp);
        DriverItemCfg[ucCount].ucLD_Number = (UINT8)Number;
        if(Number < MAX_LD_NUMBER_LENGTH)
        {
            memcpy((UINT8*)DriverItemCfg[ucCount].acLD_Exist, Temp, Number);
        }
		pinStr_LD += len;
		//printf("\n!!! L%d (%s)(%s) !!!\n",len, pinStr_LD,cLD_Cat);

		//Fan
		memset(Temp, 0, 32);
		sscanf(pinStr_Fan, "%[^ ] %n", cFan_Cat, &len);
		sscanf(cFan_Cat, "%d,%s", &Number,Temp);
        DriverItemCfg[ucCount].ucFan_Number = (UINT8)Number;
        if(Number < MAX_FAN_NUMBER_LENGTH)
        {
            memcpy((UINT8*)DriverItemCfg[ucCount].acFan_Exist, Temp, Number);
        }
		pinStr_Fan += len;
		//printf("\n!!! F%d (%s)(%s) !!!\n",len, pinStr_Fan,cFan_Cat);

		//TEC
		memset(Temp, 0, 32);
		sscanf(pinStr_Tec, "%[^ ] %n", cTEC_Cat, &len);
		sscanf(cTEC_Cat, "%d,%s", &Number,Temp);
        DriverItemCfg[ucCount].ucTEC_Number = (UINT8)Number;
        if(Number < MAX_TEC_NUMBER_LENGTH)
        {
            memcpy((UINT8*)DriverItemCfg[ucCount].acTEC_Exist, Temp, Number);
        }
		pinStr_Tec += len;
		//printf("\n!!! T%d (%s)(%s) !!!\n",len, pinStr_Tec,cTEC_Cat);

		//FilterIndex
		memset(Temp, 0, 32);
		sscanf(pinStr_FilterIndex, "%[^ ] %n", cFilterIndex_Cat, &len);
		sscanf(cFilterIndex_Cat, "%d,%s", &Number,Temp);
        DriverItemCfg[ucCount].ucFilterIndex_Number = (UINT8)Number;
        if(Number < MAX_FILTER_INDEX_NUMBER_LENGTH)
        {
            memcpy((UINT8*)DriverItemCfg[ucCount].acFilterIndex_Exist, Temp, Number);
        }
		pinStr_FilterIndex += len;
		//printf("\n!!! FI%d (%s)(%s) !!!\n",len, pinStr_FilterIndex,cFilterIndex_Cat);

		//LightSensor
		memset(Temp, 0, 32);
		sscanf(pinStr_LightSensor, "%[^ ] %n", cLightSensor_Cat, &len);
		sscanf(cLightSensor_Cat, "%d,%s", &Number,Temp);
        DriverItemCfg[ucCount].ucLightSensor_Number = (UINT8)Number;
        if(Number < MAX_LIGHT_SENSOR_NUMBER_LENGTH)
        {
            memcpy((UINT8*)DriverItemCfg[ucCount].acLightSensor_Exist, Temp, Number);
        }
		pinStr_LightSensor += len;
		//printf("\n!!! FI%d (%s)(%s) !!!\n",len, pinStr_LightSensor,cLightSensor_Cat);

		//DMD Temp
		memset(Temp, 0, 32);
		sscanf(pinStr_DMD_Temp, "%[^ ] %n", cDMD_Temp_Cat, &len);
		sscanf(cDMD_Temp_Cat, "%d,%s", &Number,Temp);
        DriverItemCfg[ucCount].ucDMD_Temp_Number = (UINT8)Number;
        if(Number < MAX_DMD_TEMP_NUMBER_LENGTH)
        {
            memcpy((UINT8*)DriverItemCfg[ucCount].ucDMD_Temp_Exist, Temp, Number);
        }
		pinStr_DMD_Temp += len;
		//printf("\n!!! FI%d (%s)(%s) !!!\n",len, pinStr_DMD_Temp,cDMD_Temp_Cat);

		//Pump
		memset(Temp, 0, 32);
		sscanf(pinStr_Pump, "%[^ ] %n", cPump_Cat, &len);
		sscanf(cPump_Cat, "%d,%s", &Number,Temp);
        DriverItemCfg[ucCount].ucPump_Number = (UINT8)Number;
        if(Number < MAX_PUMP_NUMBER_LENGTH)
        {
            memcpy((UINT8*)DriverItemCfg[ucCount].ucPump_Exist, Temp, Number);
        }
		pinStr_Pump += len;
		//printf("\n!!! FI%d (%s)(%s) !!!\n",len, pinStr_Pump,cPump_Cat);

		//Pressure
		memset(Temp, 0, 32);
		sscanf(pinStr_Pressure, "%[^ ] %n", cPressure_Cat, &len);
		sscanf(cPressure_Cat, "%d,%s", &Number,Temp);
        DriverItemCfg[ucCount].ucPressure_Number = (UINT8)Number;
        if(Number < MAX_PRESSRE_NUMBER_LENGTH)
        {
            memcpy((UINT8*)DriverItemCfg[ucCount].ucPressure_Exist, Temp, Number);
        }
		pinStr_Pressure += len;
		//printf("\n!!! FI%d (%s)(%s) !!!\n",len, pinStr_Pressure,cPressure_Cat);

	}
	#if 0
	printf("\n!!! LD0 (%2d)(%s) !!!\n", DriverItemCfg[0].ucLD_Number,DriverItemCfg[0].acLD_Exist);
	printf("\n!!! LD1 (%2d)(%s) !!!\n", DriverItemCfg[1].ucLD_Number,DriverItemCfg[1].acLD_Exist);
	printf("\n!!! LD2 (%2d)(%s) !!!\n", DriverItemCfg[2].ucLD_Number,DriverItemCfg[2].acLD_Exist);
	printf("\n!!! LD3 (%2d)(%s) !!!\n", DriverItemCfg[3].ucLD_Number,DriverItemCfg[3].acLD_Exist);

	printf("\n!!! Fan0 (%2d)(%s) !!!\n", DriverItemCfg[0].ucFan_Number,DriverItemCfg[0].acFan_Exist);
	printf("\n!!! Fan1 (%2d)(%s) !!!\n", DriverItemCfg[1].ucFan_Number,DriverItemCfg[1].acFan_Exist);
	printf("\n!!! Fan2 (%2d)(%s) !!!\n", DriverItemCfg[2].ucFan_Number,DriverItemCfg[2].acFan_Exist);
	printf("\n!!! Fan3 (%2d)(%s) !!!\n", DriverItemCfg[3].ucFan_Number,DriverItemCfg[3].acFan_Exist);

	printf("\n!!! TEC0 (%2d)(%s) !!!\n", DriverItemCfg[0].ucTEC_Number,DriverItemCfg[0].acTEC_Exist);
	printf("\n!!! TEC1 (%2d)(%s) !!!\n", DriverItemCfg[1].ucTEC_Number,DriverItemCfg[1].acTEC_Exist);
	printf("\n!!! TEC2 (%2d)(%s) !!!\n", DriverItemCfg[2].ucTEC_Number,DriverItemCfg[2].acTEC_Exist);
	printf("\n!!! TEC3 (%2d)(%s) !!!\n", DriverItemCfg[3].ucTEC_Number,DriverItemCfg[3].acTEC_Exist);


	printf("\n!!! Filter0 (%2d)(%s) !!!\n", DriverItemCfg[0].ucFilterIndex_Number,DriverItemCfg[0].acFilterIndex_Exist);
	printf("\n!!! Filter1 (%2d)(%s) !!!\n", DriverItemCfg[1].ucFilterIndex_Number,DriverItemCfg[1].acFilterIndex_Exist);
	printf("\n!!! Filter2 (%2d)(%s) !!!\n", DriverItemCfg[2].ucFilterIndex_Number,DriverItemCfg[2].acFilterIndex_Exist);
	printf("\n!!! Filter3 (%2d)(%s) !!!\n", DriverItemCfg[3].ucFilterIndex_Number,DriverItemCfg[3].acFilterIndex_Exist);

	printf("\n!!! LS0 (%2d)(%s) !!!\n", DriverItemCfg[0].ucLightSensor_Number,DriverItemCfg[0].acLightSensor_Exist);
	printf("\n!!! LS1 (%2d)(%s) !!!\n", DriverItemCfg[1].ucLightSensor_Number,DriverItemCfg[1].acLightSensor_Exist);
	printf("\n!!! LS2 (%2d)(%s) !!!\n", DriverItemCfg[2].ucLightSensor_Number,DriverItemCfg[2].acLightSensor_Exist);
	printf("\n!!! LS3 (%2d)(%s) !!!\n", DriverItemCfg[3].ucLightSensor_Number,DriverItemCfg[3].acLightSensor_Exist);

	printf("\n!!! DMD0 (%2d)(%s) !!!\n", DriverItemCfg[0].ucDMD_Temp_Number,DriverItemCfg[0].ucDMD_Temp_Exist);
	printf("\n!!! DMD1 (%2d)(%s) !!!\n", DriverItemCfg[1].ucDMD_Temp_Number,DriverItemCfg[1].ucDMD_Temp_Exist);
	printf("\n!!! DMD2 (%2d)(%s) !!!\n", DriverItemCfg[2].ucDMD_Temp_Number,DriverItemCfg[2].ucDMD_Temp_Exist);
	printf("\n!!! DMD3 (%2d)(%s) !!!\n", DriverItemCfg[3].ucDMD_Temp_Number,DriverItemCfg[3].ucDMD_Temp_Exist);

	printf("\n!!! Pump0 (%2d)(%s) !!!\n", DriverItemCfg[0].ucPump_Number,DriverItemCfg[0].ucPump_Exist);
	printf("\n!!! Pump1 (%2d)(%s) !!!\n", DriverItemCfg[1].ucPump_Number,DriverItemCfg[1].ucPump_Exist);
	printf("\n!!! Pump2 (%2d)(%s) !!!\n", DriverItemCfg[2].ucPump_Number,DriverItemCfg[2].ucPump_Exist);
	printf("\n!!! Pump3 (%2d)(%s) !!!\n", DriverItemCfg[3].ucPump_Number,DriverItemCfg[3].ucPump_Exist);

	printf("\n!!! Pressure0 (%2d)(%s) !!!\n", DriverItemCfg[0].ucPressure_Number,DriverItemCfg[0].ucPressure_Exist);
	printf("\n!!! Pressure1 (%2d)(%s) !!!\n", DriverItemCfg[1].ucPressure_Number,DriverItemCfg[1].ucPressure_Exist);
	printf("\n!!! Pressure2 (%2d)(%s) !!!\n", DriverItemCfg[2].ucPressure_Number,DriverItemCfg[2].ucPressure_Exist);
	printf("\n!!! Pressure3 (%2d)(%s) !!!\n", DriverItemCfg[3].ucPressure_Number,DriverItemCfg[3].ucPressure_Exist);
	#endif
    return EXEC_PASS;
}

INT8 InitSystemCfg(void)
{
    FILE *fp = fopen( SYSTEM_CFG_FILE_PATH ,"rb");
    if(fp == NULL)
    {
        printf("Error: No SystemCfg.cfg File (%s)\n", SYSTEM_CFG_FILE_PATH);
        return NO_SYSCFG_FILE;
    }
    fclose(fp);

    DbgMsg("\n<SystemCfg>\n");
    LoadValueByDatatype(SystemCfg, eSystemItemMaxNumber, SYSTEM_CFG_FILE_PATH);

	InitDriverCfg();
    return EXEC_PASS;
}

#if 0
INT8 InitCustomCfg(void)
{
    FILE *fp = fopen( CUSTOM_CFG_FILE_PATH ,"rb");
    if(fp == NULL)
    {
        printf("Error: No CustomCfg.cfg File\n");
        return NO_FILE;
    }
    fclose(fp);

    DbgMsg("\n<CustomCfg>\n");
    LoadValueByDatatype(CustomCfg, eCustomItemMaxNumber, CUSTOM_CFG_FILE_PATH);

    return EXEC_PASS;
}
#endif

INT8 LoadValueByDatatype(sCFG_SETTINGS sCfgSettings[], UINT32 MaxFwItem, char *FilePath)
{
    UINT32 FileSize = 0 ;
    UINT16 FileTotalItem = 0 ;
    char *CfgFileItem[CONF_MAX_ITEM][2];

    //get file size
    FILE *fp = fopen(FilePath, "rb");
    if(fp == NULL)
    {
        printf("Error: No File (%s)\n", FilePath);
        return NO_SYSCFG_FILE;
    }
    //flock(fileno(fp), LOCK_SH);
    FileSize = Get_File_Size(FilePath);
    //flock(fileno(fp), LOCK_UN);
    fclose(fp);
    //DbgMsg("FileSize = %d\n", FileSize);

    //read all item of file to "*Item[CONF_MAX_ITEM][2]"
    char *Buffer = (char*)malloc(FileSize+1);
    FileTotalItem = (UINT16)readConf(FilePath, CfgFileItem, CONF_MAX_ITEM, Buffer, FileSize);
    //DbgMsg("FileTotalItem = %d\n", FileTotalItem);

    if(FileTotalItem != MaxFwItem)
    {
        printf("!!!!!!!!!!!!!!!!!!!!!!!     Warning    !!!!!!!!!!!!!!!!!!!!!!\n");
        printf("!!!!!!!!!!!!!!!!!!!!!!!     Warning    !!!!!!!!!!!!!!!!!!!!!!\n");
        printf("File Total Item in SystemCfg.cfg (%d) != FW Item (%d)\n", FileTotalItem, MaxFwItem);
        printf("Please check %s\n", SYSTEM_CFG_FILE_PATH);
        printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
        printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
    }

    //write to SystemCfg[eItemMaxNumber] value

    DbgMsg("================================================\n");
    for(UINT16 index=0 ; index < MaxFwItem ; index++)
    {
        //搜尋 SystemCfg.cfg 裡面的項目名稱，是否有跟 Firmware 的 SystemCfg[] 的 ItemName 相同
        //若有相同的項目名稱 => 則將此 SystemCfg.cfg 的項目數值，更新給 SystemCfg[] 的數值
        for(UINT16 FileIndex=0 ; FileIndex < FileTotalItem ; FileIndex++)
        {
            //取得目前的 SystemCfg.cfg file item 名稱
            sCFG_ITEM_FORMAT sCfgFileItem;
            snprintf(sCfgFileItem.ItemName,  MAX_ITEMNAME_LENGTH,  "%s", CfgFileItem[FileIndex][0]);
            snprintf(sCfgFileItem.ItemValue, MAX_ITEMVALUE_LENGTH, "%s", CfgFileItem[FileIndex][1]);
            //DbgMsg("%s=%s\n", sCfgItem.ItemName, sCfgItem.ItemValue);

            if(strncmp(sCfgFileItem.ItemName, sCfgSettings[index].ItemName, MAX_ITEMNAME_LENGTH) == 0)  //find item
            {
                switch(sCfgSettings[index].eDataType)
                {
                    case eDATA_TYPE_STRING:
                        sprintf(sCfgSettings[index].StringValue, "%s", sCfgFileItem.ItemValue);
                        DbgMsg("%s", sCfgSettings[index].StringValue);
                        break;

                    case eDATA_TYPE_UINT32:
                        sCfgSettings[index].IntValue = (UINT32)Str2Int(sCfgFileItem.ItemValue);
                        DbgMsg("%u", sCfgSettings[index].IntValue);
                        break;

                    case eDATA_TYPE_INT32:
                        sCfgSettings[index].IntValue = (INT32)Str2Int(sCfgFileItem.ItemValue);
                        DbgMsg("%d", sCfgSettings[index].IntValue);
                        break;

                    case eDATA_TYPE_HEX:
                        sCfgSettings[index].IntValue = (INT32)Str2Int(sCfgFileItem.ItemValue);
                        DbgMsg("0x%X", sCfgSettings[index].IntValue);
                        break;

                    case eDATA_TYPE_FLOAT:
                        sCfgSettings[index].FloatValue = (FLOAT)atof(sCfgFileItem.ItemValue);
                        DbgMsg("%f", sCfgSettings[index].FloatValue);
                        break;

                    default:
                        printf("Error: unknown Data Type (%d)\n", index);
                        break;
                }


                break;
            }

            if(FileIndex + 1 == FileTotalItem)  //not found
            {
                printf("Warning: SystemCfg.cfg Item \"%s\" is missing\n", sCfgSettings[index].ItemName);
            }

        }

        //DbgMsg("%s(%s)=", sCfgSettings[index].ItemName, CurItemValue);
        //DbgMsg("\n");

    }

    DbgMsg("================================================\n");

    free(Buffer);
    Buffer = NULL;

    return EXEC_PASS;
}

INT8 Syscfg_Value_Match(eSYSTEM_CFG_ITEM Item, ...)
{
    va_list args;
    va_start(args, Item);

    INT8 Match = FALSE;

    switch(SystemCfg[Item].eDataType)
    {
        case eDATA_TYPE_STRING:
            if(strncmp(SystemCfg[Item].StringValue , va_arg(args, char *), MAX_ITEMVALUE_LENGTH) == 0)
            {
                Match = TRUE;
            }
            break;

        case eDATA_TYPE_UINT32:
        case eDATA_TYPE_INT32:
        case eDATA_TYPE_HEX:
            if(SystemCfg[Item].IntValue == va_arg(args, int))
            {
                Match = TRUE;
            }
            break;

        case eDATA_TYPE_FLOAT:
            if(fabs((DOUBLE)SystemCfg[Item].FloatValue - va_arg(args, double)) < 0.001)
            {
                Match = TRUE;
            }
            break;

        default:
            printf("Error: unknown Data Type (%d)\n", Item);
            break;
    }

    va_end(args);

    return Match;
}

void Syscfg_Value_Get(eSYSTEM_CFG_ITEM Item, void *Data)
{
    switch(SystemCfg[Item].eDataType)
    {
        case eDATA_TYPE_STRING:
            snprintf(Data, MAX_ITEMVALUE_LENGTH, "%s", SystemCfg[Item].StringValue);
            break;

        case eDATA_TYPE_UINT32:
        case eDATA_TYPE_INT32:
        case eDATA_TYPE_HEX:
            *(INT32 *)Data = SystemCfg[Item].IntValue;
            break;

        case eDATA_TYPE_FLOAT:
            *(DOUBLE *)Data = SystemCfg[Item].FloatValue;
            break;

        default:
            printf("Error: unknown Data Type (%d)\n", Item);
            break;
    }
}

/*
CUSTOMER_ID_CHRISTIE    = 0x01
CUSTOMER_ID_OPTOMA      = 0x02
CUSTOMER_ID_BARCO       = 0x03
CUSTOMER_ID_DONVIEW     = 0x04
CUSTOMER_ID_EIKI        = 0x05
CUSTOMER_ID_RICOH       = 0x06
CUSTOMER_ID_KRINDA      = 0x07
CUSTOMER_ID_HUALUSUMPO  = 0x08
CUSTOMER_ID_AVIC        = 0x09
CUSTOMER_ID_DHN         = 0x0A
CUSTOMER_ID_SHARP       = 0x0C
CUSTOMER_ID_NEC         = 0x0D
CUSTOMER_ID_VIEWSONIC   = 0x0F
CUSTOMER_ID_FUJIFILM    = 0x10
*/

INT32 Syscfg_Value_Get_Typeint(eSYSTEM_CFG_ITEM Item)
{
    if(SystemCfg[Item].eDataType != eDATA_TYPE_INT32  &&
       SystemCfg[Item].eDataType != eDATA_TYPE_UINT32 &&
       SystemCfg[Item].eDataType != eDATA_TYPE_HEX)
    {
        DbgMsg("%s Error\n", __FUNCTION__);
        return -1;
    }

    return SystemCfg[Item].IntValue;
}

void Syscfg_Value_Set(eSYSTEM_CFG_ITEM Item, ...)
{
    va_list args;
    va_start(args, Item);

    switch(SystemCfg[Item].eDataType)
    {
        case eDATA_TYPE_STRING:
            snprintf(SystemCfg[Item].StringValue, MAX_ITEMVALUE_LENGTH, "%s", va_arg(args, char *));
            break;

        case eDATA_TYPE_UINT32:
        case eDATA_TYPE_INT32:
        case eDATA_TYPE_HEX:
            SystemCfg[Item].IntValue = va_arg(args, int);
            break;

        case eDATA_TYPE_FLOAT:
            SystemCfg[Item].FloatValue = va_arg(args, double);
            break;

        default:
            printf("Error: unknown Data Type (%d)\n", Item);
            break;
    }

    va_end(args);

}


void Syscfg_SaveToFile(void)
{
    SaveToCfgFile(SystemCfg, sizeof(SystemCfg)/sizeof(SystemCfg[0]), SYSTEM_CFG_FILE_PATH);
}

//SystemCfg : write SystemCfg[] to "SystemCfg.cfg" file
void SaveToCfgFile(sCFG_SETTINGS sCfgSettings[], UINT16 TotalItem, char *FilePath)
{
    char *Item[TotalItem][2];
    char Value[TotalItem][MAX_ITEMVALUE_LENGTH] ;

    for(UINT16 index=0 ; index<TotalItem ; index++)
    {
        switch(sCfgSettings[index].eDataType)
        {
            case eDATA_TYPE_STRING:
                snprintf(Value[index], MAX_ITEMVALUE_LENGTH, "%s", sCfgSettings[index].StringValue);
                break;

            case eDATA_TYPE_UINT32:
            case eDATA_TYPE_INT32:
                snprintf(Value[index], MAX_ITEMVALUE_LENGTH, "%d", sCfgSettings[index].IntValue);
                break;

            case eDATA_TYPE_HEX:
                snprintf(Value[index], MAX_ITEMVALUE_LENGTH, "0x%X", sCfgSettings[index].IntValue);
                break;

            case eDATA_TYPE_FLOAT:
                snprintf(Value[index], MAX_ITEMVALUE_LENGTH, "%f", sCfgSettings[index].FloatValue);
                break;
        }

        Item[index][0] = (char *)sCfgSettings[index].ItemName;
        Item[index][1] = Value[index];

        //printf("W %s=%s\n", Item[index][0], Item[index][1]);
    }

    //write to file
    writeConf(FilePath, Item, TotalItem);

}

void Drivercfg_Value_Set(UINT8 ucModelID,eDRIVER_CFG_ITEM Item,UINT8 ucIndex,UINT8 ucValue)
{
	UINT8 ucModeID_Number = ucModelID -1; //MODEL_ID_0
	char cStringTmp[MAX_ITEMVALUE_LENGTH] = "\0";
	char cStringBuf[MAX_ITEMVALUE_LENGTH] = "\0";
	UINT8 ucCount = 0;
	INT32 wModeID_Max = 0;
	Syscfg_Value_Get(eModelID_Max,&wModeID_Max);

	switch(Item)
    {
        case eLD_Exist:
			if((ucIndex < MAX_LD_NUMBER_LENGTH) &&
			   (ucModelID <= wModeID_Max) &&
			   (ucModelID != 0))
			{
				if(ucValue)
				{
					if(DriverItemCfg[ucModeID_Number].acLD_Exist[ucIndex] != '1')
						DriverItemCfg[ucModeID_Number].ucLD_Number++;
					DriverItemCfg[ucModeID_Number].acLD_Exist[ucIndex] = '1';
				}
				else
				{
					if(DriverItemCfg[ucModeID_Number].acLD_Exist[ucIndex] == '1')
						DriverItemCfg[ucModeID_Number].ucLD_Number--;
					DriverItemCfg[ucModeID_Number].acLD_Exist[ucIndex] = '0';
				}

				for( ;ucCount < wModeID_Max ; ucCount++)
				{
				    if(ucCount == (wModeID_Max-1))
					{
						snprintf(cStringTmp, MAX_ITEMVALUE_LENGTH, "%d,%s",DriverItemCfg[ucCount].ucLD_Number,DriverItemCfg[ucCount].acLD_Exist);
					}
					else
					{
						snprintf(cStringTmp, MAX_ITEMVALUE_LENGTH, "%d,%s ",DriverItemCfg[ucCount].ucLD_Number,DriverItemCfg[ucCount].acLD_Exist);
					}
					strcat(cStringBuf, cStringTmp);
					//printf("\n!!! (%s)(%s)!!!\n", cStringBuf,cStringTmp);
				}
				snprintf(SystemCfg[eLD_Infomation].StringValue, MAX_ITEMVALUE_LENGTH, "%s",cStringBuf);
				//printf("\n!!! LD(%s)(%d)!!!\n", SystemCfg[eLD_Infomation].StringValue,strlen(cStringBuf));
			}
            break;

		case eFan_Exist:
			if((ucIndex < MAX_FAN_NUMBER_LENGTH) &&
			   (ucModelID <= wModeID_Max) &&
			   (ucModelID != 0))
			{
				if(ucValue)
				{
					if(DriverItemCfg[ucModeID_Number].acFan_Exist[ucIndex] != '1')
						DriverItemCfg[ucModeID_Number].ucFan_Number++;
					DriverItemCfg[ucModeID_Number].acFan_Exist[ucIndex] = '1';
				}
				else
				{
					if(DriverItemCfg[ucModeID_Number].acFan_Exist[ucIndex] == '1')
						DriverItemCfg[ucModeID_Number].ucFan_Number--;
					DriverItemCfg[ucModeID_Number].acFan_Exist[ucIndex] = '0';
				}

				for( ;ucCount < wModeID_Max ; ucCount++)
				{
				    if(ucCount == (wModeID_Max-1))
					{
						snprintf(cStringTmp, MAX_ITEMVALUE_LENGTH, "%d,%s",DriverItemCfg[ucCount].ucFan_Number,DriverItemCfg[ucCount].acFan_Exist);
					}
					else
					{
						snprintf(cStringTmp, MAX_ITEMVALUE_LENGTH, "%d,%s ",DriverItemCfg[ucCount].ucFan_Number,DriverItemCfg[ucCount].acFan_Exist);
					}
					strcat(cStringBuf, cStringTmp);
					//printf("\n!!! (%s)(%s)!!!\n", cStringBuf,cStringTmp);
				}
				snprintf(SystemCfg[eFan_Infomation].StringValue, MAX_ITEMVALUE_LENGTH, "%s",cStringBuf);
				//printf("\n!!! Fan(%s)(%d)!!!\n", SystemCfg[eFan_Infomation].StringValue,strlen(cStringBuf));
			}
            break;

		case eTEC_Exist:
			if((ucIndex < MAX_TEC_NUMBER_LENGTH) &&
			   (ucModelID <= wModeID_Max) &&
			   (ucModelID != 0))
			{
				if(ucValue)
				{
					if(DriverItemCfg[ucModeID_Number].acTEC_Exist[ucIndex] != '1')
						DriverItemCfg[ucModeID_Number].ucTEC_Number++;
					DriverItemCfg[ucModeID_Number].acTEC_Exist[ucIndex] = '1';
				}
				else
				{
					if(DriverItemCfg[ucModeID_Number].acTEC_Exist[ucIndex] == '1')
						DriverItemCfg[ucModeID_Number].ucTEC_Number--;
					DriverItemCfg[ucModeID_Number].acTEC_Exist[ucIndex] = '0';
				}

				for( ;ucCount < wModeID_Max ; ucCount++)
				{
				    if(ucCount == (wModeID_Max-1))
					{
						snprintf(cStringTmp, MAX_ITEMVALUE_LENGTH, "%d,%s",DriverItemCfg[ucCount].ucTEC_Number,DriverItemCfg[ucCount].acTEC_Exist);
					}
					else
					{
						snprintf(cStringTmp, MAX_ITEMVALUE_LENGTH, "%d,%s ",DriverItemCfg[ucCount].ucTEC_Number,DriverItemCfg[ucCount].acTEC_Exist);
					}
					strcat(cStringBuf, cStringTmp);
					//printf("\n!!! (%s)(%s)!!!\n", cStringBuf,cStringTmp);
				}
				snprintf(SystemCfg[eTEC_Infomation].StringValue, MAX_ITEMVALUE_LENGTH, "%s",cStringBuf);
				//printf("\n!!! TEC(%s)(%d)!!!\n", SystemCfg[eTEC_Infomation].StringValue,strlen(cStringBuf));
			}
            break;

		case eFilter_Index_Exist:
			if((ucIndex < MAX_FILTER_INDEX_NUMBER_LENGTH) &&
			   (ucModelID <= wModeID_Max) &&
			   (ucModelID != 0))
			{
				if(ucValue)
				{
					if(DriverItemCfg[ucModeID_Number].acFilterIndex_Exist[ucIndex] != '1')
						DriverItemCfg[ucModeID_Number].ucFilterIndex_Number++;
					DriverItemCfg[ucModeID_Number].acFilterIndex_Exist[ucIndex] = '1';
				}
				else
				{
					if(DriverItemCfg[ucModeID_Number].acFilterIndex_Exist[ucIndex] == '1')
						DriverItemCfg[ucModeID_Number].ucFilterIndex_Number--;
					DriverItemCfg[ucModeID_Number].acFilterIndex_Exist[ucIndex] = '0';
				}

				for( ;ucCount < wModeID_Max ; ucCount++)
				{
				    if(ucCount == (wModeID_Max-1))
					{
						snprintf(cStringTmp, MAX_ITEMVALUE_LENGTH, "%d,%s",DriverItemCfg[ucCount].ucFilterIndex_Number,DriverItemCfg[ucCount].acFilterIndex_Exist);
					}
					else
					{
						snprintf(cStringTmp, MAX_ITEMVALUE_LENGTH, "%d,%s ",DriverItemCfg[ucCount].ucFilterIndex_Number,DriverItemCfg[ucCount].acFilterIndex_Exist);
					}
					strcat(cStringBuf, cStringTmp);
					//printf("\n!!! (%s)(%s)!!!\n", cStringBuf,cStringTmp);
				}
				snprintf(SystemCfg[eFilterIndex_Infomation].StringValue, MAX_ITEMVALUE_LENGTH, "%s",cStringBuf);
				//printf("\n!!! FilterIndex(%s)(%d)!!!\n", SystemCfg[eFilterIndex_Infomation].StringValue,strlen(cStringBuf));
			}
            break;

		case eLight_Sensor_Exist:
			if((ucIndex < MAX_LIGHT_SENSOR_NUMBER_LENGTH) &&
			   (ucModelID <= wModeID_Max) &&
			   (ucModelID != 0))
			{
				if(ucValue)
				{
					if(DriverItemCfg[ucModeID_Number].acLightSensor_Exist[ucIndex] != '1')
						DriverItemCfg[ucModeID_Number].ucLightSensor_Number++;
					DriverItemCfg[ucModeID_Number].acLightSensor_Exist[ucIndex] = '1';
				}
				else
				{
					if(DriverItemCfg[ucModeID_Number].acLightSensor_Exist[ucIndex] == '1')
						DriverItemCfg[ucModeID_Number].ucLightSensor_Number--;
					DriverItemCfg[ucModeID_Number].acLightSensor_Exist[ucIndex] = '0';
				}

				for( ;ucCount < wModeID_Max ; ucCount++)
				{
				    if(ucCount == (wModeID_Max-1))
					{
						snprintf(cStringTmp, MAX_ITEMVALUE_LENGTH, "%d,%s",DriverItemCfg[ucCount].ucLightSensor_Number,DriverItemCfg[ucCount].acLightSensor_Exist);
					}
					else
					{
						snprintf(cStringTmp, MAX_ITEMVALUE_LENGTH, "%d,%s ",DriverItemCfg[ucCount].ucLightSensor_Number,DriverItemCfg[ucCount].acLightSensor_Exist);
					}
					strcat(cStringBuf, cStringTmp);
					//printf("\n!!! (%s)(%s)!!!\n", cStringBuf,cStringTmp);
				}
				snprintf(SystemCfg[eLightSensor_Infomation].StringValue, MAX_ITEMVALUE_LENGTH, "%s",cStringBuf);
				//printf("\n!!! LigitSensor(%s)(%d)!!!\n", SystemCfg[eLightSensor_Infomation].StringValue,strlen(cStringBuf));
			}
            break;

		case eDMD_Temp_Exist:
			if((ucIndex < MAX_DMD_TEMP_NUMBER_LENGTH) &&
			   (ucModelID <= wModeID_Max) &&
			   (ucModelID != 0))
			{
				if(ucValue)
				{
					if(DriverItemCfg[ucModeID_Number].ucDMD_Temp_Exist[ucIndex] != '1')
						DriverItemCfg[ucModeID_Number].ucDMD_Temp_Number++;
					DriverItemCfg[ucModeID_Number].ucDMD_Temp_Exist[ucIndex] = '1';
				}
				else
				{
					if(DriverItemCfg[ucModeID_Number].ucDMD_Temp_Exist[ucIndex] == '1')
						DriverItemCfg[ucModeID_Number].ucDMD_Temp_Number--;
					DriverItemCfg[ucModeID_Number].ucDMD_Temp_Exist[ucIndex] = '0';
				}

				for( ;ucCount < wModeID_Max ; ucCount++)
				{
				    if(ucCount == (wModeID_Max-1))
					{
						snprintf(cStringTmp, MAX_ITEMVALUE_LENGTH, "%d,%s",DriverItemCfg[ucCount].ucDMD_Temp_Number,DriverItemCfg[ucCount].ucDMD_Temp_Exist);
					}
					else
					{
						snprintf(cStringTmp, MAX_ITEMVALUE_LENGTH, "%d,%s ",DriverItemCfg[ucCount].ucDMD_Temp_Number,DriverItemCfg[ucCount].ucDMD_Temp_Exist);
					}
					strcat(cStringBuf, cStringTmp);
					//printf("\n!!! (%s)(%s)!!!\n", cStringBuf,cStringTmp);
				}
				snprintf(SystemCfg[eDMD_Temp_Infomation].StringValue, MAX_ITEMVALUE_LENGTH, "%s",cStringBuf);
				//printf("\n!!! DMD temp(%s)(%d)!!!\n", SystemCfg[eDMD_Temp_Infomation].StringValue,strlen(cStringBuf));
			}
            break;

		case ePump_Exist:
			if((ucIndex < MAX_PUMP_NUMBER_LENGTH) &&
			   (ucModelID <= wModeID_Max) &&
			   (ucModelID != 0))
			{
				if(ucValue)
				{
					if(DriverItemCfg[ucModeID_Number].ucPump_Exist[ucIndex] != '1')
						DriverItemCfg[ucModeID_Number].ucPump_Number++;
					DriverItemCfg[ucModeID_Number].ucPump_Exist[ucIndex] = '1';
				}
				else
				{
					if(DriverItemCfg[ucModeID_Number].ucPump_Exist[ucIndex] == '1')
						DriverItemCfg[ucModeID_Number].ucPump_Number--;
					DriverItemCfg[ucModeID_Number].ucPump_Exist[ucIndex] = '0';
				}

				for( ;ucCount < wModeID_Max ; ucCount++)
				{
				    if(ucCount == (wModeID_Max-1))
					{
						snprintf(cStringTmp, MAX_ITEMVALUE_LENGTH, "%d,%s",DriverItemCfg[ucCount].ucPump_Number,DriverItemCfg[ucCount].ucPump_Exist);
					}
					else
					{
						snprintf(cStringTmp, MAX_ITEMVALUE_LENGTH, "%d,%s ",DriverItemCfg[ucCount].ucPump_Number,DriverItemCfg[ucCount].ucPump_Exist);
					}
					strcat(cStringBuf, cStringTmp);
					//printf("\n!!! (%s)(%s)!!!\n", cStringBuf,cStringTmp);
				}
				snprintf(SystemCfg[ePump_Infomation].StringValue, MAX_ITEMVALUE_LENGTH, "%s",cStringBuf);
				//printf("\n!!! Pump(%s)(%d)!!!\n", SystemCfg[ePump_Infomation].StringValue,strlen(cStringBuf));
			}
            break;

		case ePressure_Exist:
			if((ucIndex < MAX_PUMP_NUMBER_LENGTH) &&
			   (ucModelID <= wModeID_Max) &&
			   (ucModelID != 0))
			{
				if(ucValue)
				{
					if(DriverItemCfg[ucModeID_Number].ucPressure_Exist[ucIndex] != '1')
						DriverItemCfg[ucModeID_Number].ucPressure_Number++;
					DriverItemCfg[ucModeID_Number].ucPressure_Exist[ucIndex] = '1';
				}
				else
				{
					if(DriverItemCfg[ucModeID_Number].ucPressure_Exist[ucIndex] == '1')
						DriverItemCfg[ucModeID_Number].ucPressure_Number--;
					DriverItemCfg[ucModeID_Number].ucPressure_Exist[ucIndex] = '0';
				}

				for( ;ucCount < wModeID_Max ; ucCount++)
				{
				    if(ucCount == (wModeID_Max-1))
					{
						snprintf(cStringTmp, MAX_ITEMVALUE_LENGTH, "%d,%s",DriverItemCfg[ucCount].ucPressure_Number,DriverItemCfg[ucCount].ucPressure_Exist);
					}
					else
					{
						snprintf(cStringTmp, MAX_ITEMVALUE_LENGTH, "%d,%s ",DriverItemCfg[ucCount].ucPressure_Number,DriverItemCfg[ucCount].ucPressure_Exist);
					}
					strcat(cStringBuf, cStringTmp);
					//printf("\n!!! (%s)(%s)!!!\n", cStringBuf,cStringTmp);
				}
				snprintf(SystemCfg[ePressure_Infomation].StringValue, MAX_ITEMVALUE_LENGTH, "%s",cStringBuf);
				//printf("\n!!! Pressure(%s)(%d)!!!\n", SystemCfg[ePressure_Infomation].StringValue,strlen(cStringBuf));
			}
            break;

		default:
            printf("Error: unknown Item (%d)\n", Item);
            break;
	}
}

INT16 Drivercfg_Value_Get(UINT8 ucModelID,eDRIVER_CFG_ITEM Item,UINT8 ucIndex)
{
	INT16 iValue = -1;
	UINT8 ucModeID_Number = ucModelID -1;  //MODEL_ID_0
	INT32 wModeID_Max = 0;
	Syscfg_Value_Get(eModelID_Max,&wModeID_Max);

	if((ucModeID_Number >= wModeID_Max) || (ucModelID == 0))
		return iValue;

	switch(Item)
    {
        case eLD_Number:
			iValue = (INT16)DriverItemCfg[ucModeID_Number].ucLD_Number;
            break;

		case eFan_Number:
			iValue = (INT16)DriverItemCfg[ucModeID_Number].ucFan_Number;
			break;

        case eTEC_Number:
			iValue = (INT16)DriverItemCfg[ucModeID_Number].ucTEC_Number;
			break;

		case eFilter_Index_Number:
			iValue = (INT16)DriverItemCfg[ucModeID_Number].ucFilterIndex_Number;
			break;

		case eLight_Sensor_Number:
			iValue = (INT16)DriverItemCfg[ucModeID_Number].ucLightSensor_Number;
			break;

		case eDMD_Temp_Number:
			iValue = (INT16)DriverItemCfg[ucModeID_Number].ucDMD_Temp_Number;
			break;

		case ePump_Number:
			iValue = (INT16)DriverItemCfg[ucModeID_Number].ucPump_Number;
			break;

		case ePressure_Number:
			iValue = (INT16)DriverItemCfg[ucModeID_Number].ucPressure_Number;
			break;

		case eLD_Exist:
			if(ucIndex < MAX_LD_NUMBER_LENGTH)
			{
				if(DriverItemCfg[ucModeID_Number].acLD_Exist[ucIndex] == '1')
				{
					iValue = DEVICE_IS_EXIST;
				}
				else
				{
					iValue = DEVICE_IS_NOT_EXIST;
				}
			}
			break;

		case eFan_Exist:
			if(ucIndex < MAX_FAN_NUMBER_LENGTH)
			{
				if(DriverItemCfg[ucModeID_Number].acFan_Exist[ucIndex] == '1')
				{
					iValue = DEVICE_IS_EXIST;
				}
				else
				{
					iValue = DEVICE_IS_NOT_EXIST;
				}
			}
			break;

		case eTEC_Exist:
			if(ucIndex < MAX_TEC_NUMBER_LENGTH)
			{
				if(DriverItemCfg[ucModeID_Number].acTEC_Exist[ucIndex] == '1')
				{
					iValue = DEVICE_IS_EXIST;
				}
				else
				{
					iValue = DEVICE_IS_NOT_EXIST;
				}
			}
			break;

		case eFilter_Index_Exist:
			if(ucIndex < MAX_FILTER_INDEX_NUMBER_LENGTH)
			{
				if(DriverItemCfg[ucModeID_Number].acFilterIndex_Exist[ucIndex] == '1')
				{
					iValue = DEVICE_IS_EXIST;
				}
				else
				{
					iValue = DEVICE_IS_NOT_EXIST;
				}
			}
			break;

		case eLight_Sensor_Exist:
			if(ucIndex < MAX_LIGHT_SENSOR_NUMBER_LENGTH)
			{
				if(DriverItemCfg[ucModeID_Number].acLightSensor_Exist[ucIndex] == '1')
				{
					iValue = DEVICE_IS_EXIST;
				}
				else
				{
					iValue = DEVICE_IS_NOT_EXIST;
				}
			}
			break;

		case eDMD_Temp_Exist:
			if(ucIndex < MAX_DMD_TEMP_NUMBER_LENGTH)
			{
				if(DriverItemCfg[ucModeID_Number].ucDMD_Temp_Exist[ucIndex] == '1')
				{
					iValue = DEVICE_IS_EXIST;
				}
				else
				{
					iValue = DEVICE_IS_NOT_EXIST;
				}
			}
			break;

		case ePump_Exist:
			if(ucIndex < MAX_PUMP_NUMBER_LENGTH)
			{
				if(DriverItemCfg[ucModeID_Number].ucPump_Exist[ucIndex] == '1')
				{
					iValue = DEVICE_IS_EXIST;
				}
				else
				{
					iValue = DEVICE_IS_NOT_EXIST;
				}
			}
			break;

		case ePressure_Exist:
			if(ucIndex < MAX_PRESSRE_NUMBER_LENGTH)
			{
				if(DriverItemCfg[ucModeID_Number].ucPressure_Exist[ucIndex] == '1')
				{
					iValue = DEVICE_IS_EXIST;
				}
				else
				{
					iValue = DEVICE_IS_NOT_EXIST;
				}
			}
			break;

        default:
            printf("Error: unknown Item (%d)\n", Item);
            break;
    }

	return iValue;
}

void Syscfg_PrintSystemCfg(void)
{
    printf("ID : Name - Value\r\n");

    for(UINT16 idx=0 ; idx < eSystemItemMaxNumber ; idx++)
    {
        switch(SystemCfg[idx].eDataType)
        {
            case eDATA_TYPE_STRING:
                printf("%03d: %s - %s\r\n", idx, SystemCfg[idx].ItemName, SystemCfg[idx].StringValue);
                break;

            case eDATA_TYPE_UINT32:
                printf("%03d: %s - %u\r\n", idx, SystemCfg[idx].ItemName, SystemCfg[idx].IntValue);
                break;

            case eDATA_TYPE_INT32:
                printf("%03d: %s - %d\r\n", idx, SystemCfg[idx].ItemName, SystemCfg[idx].IntValue);
                break;

            case eDATA_TYPE_HEX:
                printf("%03d: %s - 0x%X\r\n", idx, SystemCfg[idx].ItemName, SystemCfg[idx].IntValue);
                break;

            case eDATA_TYPE_FLOAT:
                printf("%03d: %s - %f\r\n", idx, SystemCfg[idx].ItemName, SystemCfg[idx].FloatValue);
                break;

            default:
                printf("%03d: %s unknown type\r\n", idx, SystemCfg[idx].ItemName);
                break;

        }
    }
}


#if 0
int main(void)
{
    InitSystemCfg();

    //SYSCFG_VALUE_SET(eFrontend_I2C_ADDR, 0x83);
    //SYSCFG_VALUE_SET(eTestString, "Change New String");
    //SYSCFG_VALUE_SET(eTestFloat, 4.0);

    printf("%d %d %x", SYSCFG_VALUE_GET_TYPEINT(eWith_Frontend_MCU)
                     , SYSCFG_VALUE_GET_TYPEINT(eFrontend_I2C_BUS)
                     , SYSCFG_VALUE_GET_TYPEINT(eFrontend_I2C_ADDR)
                     );
    SaveToCfgFile(SystemCfg, sizeof(SystemCfg)/sizeof(SystemCfg[0]), SYSTEM_CFG_FILE_PATH);

    InitSystemCfg();
    //InitCustomCfg();

#if 0
    printf("--- %d\n", SYSCFG_VALUE_MATCH(eTestString, "ABC543210"));
    printf("--- %d\n", SYSCFG_VALUE_MATCH(eTestString, "ABC54321"));
    printf("--- %d\n", SYSCFG_VALUE_MATCH(eTestInt32, -36));
    printf("--- %d\n", SYSCFG_VALUE_MATCH(eTestInt32, 36));
    printf("--- %d\n", SYSCFG_VALUE_MATCH(eTestUInt32, 74));
    printf("--- %d\n", SYSCFG_VALUE_MATCH(eTestUInt32, -74));
    printf("--- %d\n", SYSCFG_VALUE_MATCH(eTestFloat, 10.8));
    printf("--- %d\n", SYSCFG_VALUE_MATCH(eTestFloat, 10.0));

    char data1[MAX_VALUE_STRING_LENGTH] ;
    SYSCFG_VALUE_GET(eTestString, data1);
    printf(">> %s\n", data1);

    INT32 data2 ;
    SYSCFG_VALUE_GET(eTestInt32, &data2);
    printf(">> %d\n", data2);

    DOUBLE data3 ;
    SYSCFG_VALUE_GET(eTestFloat, &data3);
    printf(">> %f\n", data3);
#endif


    return 0;
}
#endif

