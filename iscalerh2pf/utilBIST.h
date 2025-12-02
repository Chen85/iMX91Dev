#ifndef _UTIL_BIST_H_
#define _UTIL_BIST_H_

#define ROW_LENGTH                  (300)
#define LF_OFFSET                   (0)

#define eBIST_MSG_SET_VIA_I2C           0x00 //A70LK_Steven_0006
#define eBIST_MSG_SET_VIA_UART          0x01
#define eBIST_MSG_SET_RESET_COUNT       0xF1
#define eBIST_MSG_SET_FINISH_COUNT      0xF2 //HICC2_Steven_0007
#define eBIST_MSG_SET_SZ                12 //sizeof(sBIST_DATA)    //A70LK_Steven_0005
#define eBIST_MSG_GET_SZ                32 //sizeof(sBIST_DATA)   //A70LK_Steven_0005

#define BIST_IMPORT_FILE           "BIST Import.csv"  //HICC2_Steven_0048
#define BIST_USB_LOG_PATH          "/run/media/sda1"
#define BIST_USB_FAIL              "BIST_FAIL"
#define BIST_LOG_PATH               "/mnt/syslog/BIST"
//#define BIST_USB_LOG_PATH           "/run/media/sda1/BIST"
//#define BIST_USB_FAIL_LOG_PATH      "/run/media/sda1/BIST/BIST_FAIL"
#define Default_SN      			"31211199" //A70LK_Steven_0013

#define I2C_SUCCESS_VALUE               10000
#define I2C_ERROR_VALUE                 0

#define SUB_LAYER_FINISH                3

typedef enum //for HICC2.0
{
    BIST_IMX                = 0,    //0x00 iScalr
    BIST_STB_MCU            = 1,    //0x01 STB(SYS)
    BIST_FEM_MCU            = 2,    //0x02 Frontend Module
    BIST_LSM_MCU            = 3,    //0x03 Lens Shift Module
    BIST_FMT_MCU            = 4,    //0x04 Not exist after H30
    BIST_KEY_MCU            = 5,    //0x05 Not exist sfter H30
    BIST_SCALER             = 6,    //0x06 ASIC Scaler C821/C341
    BIST_WARPING            = 7,    //0x07 ASIC Warping C789/C381
    BIST_S_FPGA             = 8,    //0x08 480F34
    BIST_DDP_1              = 9,    //0x09
    BIST_DDP_2              = 10,   //0x0A
    BIST_DDP_UPG            = 11,   //0x0B DDP upgrade path
    BIST_PCF85063           = 12,   //0x0C iMx CLK
    BIST_PMIC               = 13,   //0x0D iMX Power Management
    BIST_CLK_1              = 14,   //0x0E CLK generator CDCE949/LMK03328
    BIST_MAX31790_1         = 15,   //0x0F Fan driver
    BIST_MAX31790_2         = 16,   //0x10 Fan driver
    BIST_MAX31790_3         = 17,   //0x11 Fan driver
    BIST_MAX31790_4         = 18,   //0x12 Fan driver
    BIST_MAX31790_5         = 19,   //0x13 Fan driver
    BIST_FPGA0              = 20,   //0x14 FEM TTL swtich
    BIST_FPGA1              = 21,   //0x15
    BIST_FPGA2              = 22,   //0x16 LD control, wheel protection
    BIST_XFPGA              = 23,   //0x17 XPR/TTL Converter
    BIST_G_SENSOR           = 24,   //0x18 G-sensor ADXL345/LIS2DH12
    BIST_G751_1             = 25,   //0x19 Thermal sensor
    BIST_G751_2             = 26,   //0x1A Thermal sensor
    BIST_G751_3             = 27,   //0x1B Thermal sensor
    BIST_G753_1             = 28,   //0x1C Thermal sensor
    BIST_G753_2             = 29,   //0x1D Thermal sensor
    BIST_ADC101_DMD         = 30,   //0x1E General adc for DMD
    BIST_ADC101_PW          = 31,   //0x1F General adc for PW Photo sensor
    BIST_H_SENSOR_1         = 32,   //0x20 Humidity-sensor HDC1080
    BIST_P_SENSOR_1         = 33,   //0x21 Pressure-sensor LPS22HH
    BIST_EDID_HDMI1         = 34,   //0x22 EDID
    BIST_EDID_HDMI2         = 35,   //0x23 EDID
    BIST_EDID_HDMI3         = 36,   //0x24 EDID
    BIST_EDID_HDMI4         = 37,   //0x25 EDID
    BIST_EDID_DVI           = 38,   //0x26 EDID
    BIST_EDID_DP            = 39,   //0x27 EDID
    BIST_EDID_HDBASET       = 40,   //0x28 EDID
    BIST_EDID_VGA           = 41,   //0x29 EDID
    BIST_EEPROM_STB         = 42,   //0x2A NVRAM
    BIST_EEPROM_FEM         = 43,   //0x2B NVRAM
    BIST_EEPROM_LSM         = 44,   //0x2C NVRAM
    BIST_EEPROM_FMT         = 45,   //0x2D NVRAM
    BIST_EEPROM_DDP         = 46,   //0x2E NVRAM
    BIST_EEPROM_DMD         = 47,   //0x2F NVRAM
    BIST_EEPROM_ACTUATOR    = 48,   //0x30 NVRAM
    BIST_ITE6634_1          = 49,   //0x31 HDMI matrix
    BIST_ITE6634_2          = 50,   //0x32 HDMI matrix
    BIST_ITE6634_3          = 51,   //0x33 HDMI matrix
    BIST_ITE68051_1         = 52,   //0x34 68051/6807
    BIST_ITE68051_2         = 53,   //0x35 68051/6807
    BIST_ADV7604            = 54,   //0x36
    BIST_DP2HDMI_1          = 55,   //0x37 Dp2Hdmi driver ITE6563
    BIST_TMP411A            = 56,   //0x38 DMD NTC ADC
    BIST_HDBASET            = 57,   //0x39 VS100/VS2000/VS3000
    BIST_XG_SDI             = 58,   //0x3A 3G-SDI/12G-SDI
    BIST_AIC3204            = 59,   //0x3B
    BIST_TPA5050            = 60,   //0x3C
    BIST_APA2618            = 61,   //0x3D
    BIST_SCALER_DRAM        = 62,   //0x3E Dram of C821/C341
    BIST_SCALER_FLASH       = 63,   //0x3F Flash of C821/C341
    BIST_WARPING_DRAM       = 64,   //0x40 Dram of C789/C381
    BIST_WARPING_FLASH      = 65,   //0x41 Flash of C789/C381
    BIST_S_FPGA_FLASH       = 66,   //0x42 480F34 OSD Flash
    BIST_FAN_1              = 67,   //0x43
    BIST_FAN_2              = 68,   //0x44
    BIST_FAN_3              = 69,   //0x45
    BIST_FAN_4              = 70,   //0x46
    BIST_FAN_5              = 71,   //0x47
    BIST_FAN_6              = 72,   //0x48
    BIST_FAN_7              = 73,   //0x49
    BIST_FAN_8              = 74,   //0x4A
    BIST_FAN_9              = 75,   //0x4B
    BIST_FAN_10             = 76,   //0x4C
    BIST_FAN_11             = 77,   //0x4D
    BIST_FAN_12             = 78,   //0x4E
    BIST_FAN_13             = 79,   //0x4F
    BIST_FAN_14             = 80,   //0x50
    BIST_FAN_15             = 81,   //0x51
    BIST_FAN_16             = 82,   //0x52
    BIST_FAN_17             = 83,   //0x53
    BIST_REDRIVER_HDMI3     = 84,   //0x54
    BIST_REDRIVER_HDMI1     = 85,   //0x55
    BIST_REDRIVER_HDMI2     = 86,   //0x56
    BIST_LDDRV_MCU          = 87,   //0x57 A65_LD_M481/A70LK_LD_CONVERT_MCU
    BIST_PMUP               = 88,   //0x58
    BIST_FPGA2_UPG          = 89,   //0x59 FPGA2 upgrade port
    BIST_FPGA0_UPG          = 90,   //0x5A FPGA0 upgrade port
    BIST_TPS53647           = 91,   //0x5B
    BIST_CAMERA             = 92,   //0x5C
    BIST_IMX_STB_UART       = 93,   //0x5D uart between STB and IMX
    BIST_IMX_LSM_UART       = 94,   //0x5E uart between LSM and IMX
    BIST_STB_DDP_UART       = 95,   //0x5F uart between DDP and STB(SYS)
    BIST_STB_LVPS_UART      = 96,   //0x60 uart between LVPS and STB
    BIST_FMT_LDDRV_UART     = 97,   //0x61 uart between LDDRV and FMT(STB)
    BIST_CLK_2              = 98,   //0x62 CLK generator CDCE949/LMK03328
    BIST_CLK_3              = 99,   //0x63 CLK generator CDCE949/LMK03328
    BIST_H_SENSOR_2         = 100,  //0x64 Humidity-sensor HDC1080
    BIST_P_SENSOR_2         = 101,  //0x65 Pressure-sensor LPS22HH

    BIST_REG_NUMBERS,
}eBIST_REGEVENT;


typedef enum  //HICC2_Steven_0012
{
	eBIST_STATUS_INIT                = 0,
	eBIST_STATUS_START,
	eBIST_STATUS_IDLE,
	eBIST_STATUS_END,

    BIST_STATUS_Number
}eMSG_BIST_STATUS;

typedef struct
{
	unsigned int Row;
	unsigned int Col_Nums;
	unsigned char GetCommand;
    unsigned int FailCount;

	unsigned int Length;
	unsigned int GetSeek;
	unsigned int PutSeek;
	unsigned char *GetBuffer;
	unsigned char *PutBuffer;
} type_BIST_Handle;

typedef struct
{
	char Title[20];
	char AsciiValue[20];
	int IntegerValue;
} type_Column_Info;

typedef struct
{
	unsigned char ID;
    unsigned char Device;
    unsigned char BUS;
    unsigned char Unit;
    unsigned char Used;
    unsigned char Test;
    unsigned char TestCount;
    unsigned char CriterionUpper;
    unsigned char CriterionLow;
    unsigned char Result;
    unsigned char Testfail;
    unsigned char Commfail;
    unsigned char Layer;
    unsigned char StatisticsMax;
    unsigned char StatisticsAvg;
    unsigned char StatisticsMin;
    unsigned char RepairCode;
} type_Title_Index;

typedef int (*fpBIST_GetPU32)(unsigned long *pdata);
typedef struct
{
    int ID;
    fpBIST_GetPU32 fpGetPU32;
} type_BIST_GetPU32;

typedef void (*pBIST)(void*);
typedef struct
{
    INT32  lBistId;
    INT32  lDelay_mS;
    INT32  lDevId;              //If set Negative value, the test loop will be execuced in next layer.
    pBIST  pProc;
} sBIST_LUT;

typedef struct
{
    INT32  lTotalCnt;
    INT32  lCommuErrCnt;
    INT32  lErrorCnt;
    INT32  lMax;
    INT32  lMin;
    INT32  lAvg;
    INT32  lLayer;
    INT32  lValid;
} sBIST_DATA;

typedef struct
{
    INT32  lDevId;
    INT32  lError;          //I2C error or Data not match
    INT32  lCommuError;     //Data not match
    INT32  lLayer;
    INT32  lValid;
    INT32  lData;
} sBIST_DEV;

typedef struct
{
	const char *title;
	unsigned char *index_ptr;
} Title_Map;

typedef struct
{
    INT32  lBistId;
    INT32  lLayer;
    INT32  lCount;
} sBIST_CMD;


int utilBIST_Handle(char led_en);
void utilBIST_FolderNameGet(char *Report_name); //HICC2_Steven_0048

void utilBIST_ReportRename(void); //A70LK_Steven_0013
void utilBIST_ReportNameGet(char *Report_name);
int util_BIST_Check_Result(BOOL IsTimes, INT32 lUpper, INT32 lLow, INT32 lTotalError);
void utilBIST_Status_Set(eMSG_BIST_STATUS eStatus); //HICC2_Steven_0007
eMSG_BIST_STATUS utilBIST_Status_Get(void );  //HICC2_Steven_0008


eRESULT utilMotor_BIST_Get(void); //A70LK_Steven_0003
eRESULT utilDDP_BIST_Get(void);   //A70LK_Steven_0003
eRESULT utilDDP_BIST_DDPUSB_Test(void);     //A70LK_Steven_0011
eRESULT utilDDP_BIST_CameraUSB_Test(void);  //A70LK_Steven_0011
eRESULT utilProAV_BIST_Test(void);   //A70LK_Steven_0012

eRESULT utilBIST_XFPGA(UINT8 ucRegEvent, UINT8 *pcData);
eRESULT utilBIST_FMT_MCU(UINT8 ucRegEvent, UINT8 *pcData);
eRESULT utilBIST_Motor_MCU(UINT8 ucRegEvent, UINT8 *pcData);
eRESULT utilBIST_DDP(UINT8 ucRegEvent, UINT8 *pcData);


sBIST_CMD utilBIST_CmdInfo_Get(void);


#endif /* _UTIL_BIST_H_ */

