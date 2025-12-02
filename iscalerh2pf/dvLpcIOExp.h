#ifndef DVLPCIOEXP_H
#define DVLPCIOEXP_H
// ==============================================================================
// FILE NAME: DVLPCIOEXP.H
// DESCRIPTION:
//
//
// modification history
// --------------------
// 2014/03/24, Leo Create
// --------------------
// ==============================================================================


#include "Common.h"

typedef enum
{
    eLPCIOEXP_PORT_0,
    eLPCIOEXP_PORT_1,
    eLPCIOEXP_PORT_2,
    eLPCIOEXP_PORT_3,
    eLPCIOEXP_PORT_4,

    eLPCIOEXP_PORT_NUMBERS,
} eLPCIOEXP_PORT;

typedef enum
{
    // Common Command
    /* 00 */eIOEXP_CMD_IDLE           = 0,     // Check Busy
    /* 01 */eIOEXP_CMD_VERSION        = 1,
    /* 02 */eIOEXP_CMD_GO_BOOTLOADER  = 2,

    /* 16 */eIOEXP_CMD_PORT0_IO_DIR   = 16,
    /* 17 */eIOEXP_CMD_PORT1_IO_DIR,
    /* 18 */eIOEXP_CMD_PORT2_IO_DIR,
    /* 19 */eIOEXP_CMD_PORT3_IO_DIR,
    /* 20 */eIOEXP_CMD_PORT4_IO_DIR,

    /* 21 */eIOEXP_CMD_PORT0_IO_STATE,
    /* 22 */eIOEXP_CMD_PORT1_IO_STATE,
    /* 23 */eIOEXP_CMD_PORT2_IO_STATE,
    /* 24 */eIOEXP_CMD_PORT3_IO_STATE,
    /* 25 */eIOEXP_CMD_PORT4_IO_STATE,
    /* 26 */eIOEXP_CMD_STATUS_LED,
    /* 27 */eIOEXP_CMD_KEYPAD_SENSOR, //A70_Larry_0074

    /* 28 */eIOEXP_CMD_FACTORY_RESET_LVL,                             //STB_JS_0001 add
    /* 29 */eSTB_CMD_MCU_POWER_OFF,	//ZU860_Clare_0008
    /* 30 */eSTB_CMD_BURN_IN_ENABLE,	//ZU860_Clare_0008
    /* 31 */eSTB_CMD_BURN_IN_OFF_TIME,	//ZU860_Clare_0008
    /* 32 */eSTB_CMD_PWR_MODE_STATES,	//ZU860_Clare_0039,
    /* 33 */eSTB_CMD_SIGNAL_PWR_ON_ENABLE,	//ZU860_Clare_0039,
    /* 34 */eSTB_CMD_IR_TOP_ENABLE,	//ZU860_Clare_0039,
    /* 35 */eSTB_CMD_IR_FRONT_ENABLE,	//ZU860_Clare_0039,
    /* 36 */eSTB_CMD_IR_HDBASET_ENABLE,	//ZU860_Clare_0039,
    /* 37 */eSTB_CMD_AC_POWERON_ENABLE,	//ZU860_Clare_0039,
    /* 38 */eSTB_CMD_SERIAL_PORT_BAUD_RATE,	//ZU860_Clare_0042
    /* 39 */eSTB_CMD_PROJECTOR_ADDRESS,	//ZU860_Clare_0042
    /* 40 */eSTB_CMD_OPFU_REBOOT,	//ZU860_Clare_0043
    /* 41 */eSTB_CMD_KEYPAD_VERSION,	//ZU860_Clare_0043
    /* 42 */eSTB_CMD_LASTERROE_12VLOST, //ZU860_Energy_0004
    /* 43 */eSTB_CMD_ERRORLOG_INDEX, //ZU860_Energy_0004
    /* 44 */eSTB_CMD_ERRORLOG_CLEAR, //ZU860_Energy_0004
    /* 45 */eSTB_CMD_SCALER_TIME, //ZU860_Energy_0005
    /* 46 */eSTB_CMD_KEYPAD_LED_ENABLE,	//ZU860_Clare_0063
    /* 47 */eSTB_CMD_SHUTTER_LED,   //EK816U_626U_Owen_0011
    /* 48 */eSTB_CMD_LENS_MODEL, //SNPLU9000_Energy_0006
    /* 49 */eSTB_CMD_RESET_LAN, //SNPLU9000_Energy_0025
    /* 50 */eSTB_CMD_PWR_STATE, //A65_Owen

    eIOEXP_CMD_FAILURE_STATUS         = 0xF0,
    eIOEXP_CMD_I2C_ERR_CNT            = 0xF4,
    eIOEXP_CMD_SHUTDOWN               = 0xF5,

    eIOEXP_CMD_NUMBERS,
} eIOEXP_CMD;

// STATUS LED 有下列幾種行為
typedef enum
{
    eLED_LIGHT_GOOD,
    eLED_LIGHT_FILTER,
    eLED_LIGHT_WARNING,
    eLED_LIGHT_ERROR,
    eLED_LIGHT_OFF,

    eLED_LIGHT_NUMBERS,
} eLED_LIGHT;

// STATUS LED 有下列幾種狀態
typedef enum
{
    /* 00 */ eLED_STATUS_STATE_START_UP,
    /* 01 */ eLED_STATUS_STATE_LAMP_ON,
    /* 02 */ eLED_STATUS_STATE_COOLDOWN,
    /* 03 */ eLED_STATUS_STATE_STANDBY,
    /* 04 */ eLED_STATUS_STATE_AC_OFF,
    /* 05 */ eLED_STATUS_STATE_BROADCAST,
    /* 06 */ eLED_STATUS_STATE_DIRECT_CMD,
    /* 07 */ eLED_STATUS_STATE_UPGRADE_FW,

    /* 08 */ eLED_STATUS_STATE_COLOR_PUR,

    /* 09 */ eLED_STATUS_STATE_NUMBERS,
} eLED_STATUS_LIGHT;

void dvLPCIOExp_Init(void);

void dvLPCIOExp_PORT_Dir_Set(eLPCIOEXP_PORT ePort, UINT8 cDir);
UINT8 dvLPCIOExp_PORT_Dir_Get(eLPCIOEXP_PORT ePort);

void dvLPCIOExp_PORT_Set(eLPCIOEXP_PORT ePort, UINT8 cData);
UINT8 dvLPCIOExp_PORT_Get(eLPCIOEXP_PORT ePort);
eRESULT dvLPCIOExp_Version_Get(UINT8 *pcVersion);
eRESULT dvLPCIOExp_StatusLed_Set(eLED_LIGHT eLight, eLED_STATUS_LIGHT eStatus); //A70_Larry_0027
INT16 dvLPCIOExp_Sensor_Get(void); //A70_Larry_0074
eRESULT dvLPCIOExp_MCU_Power_Off(UINT8 cData);
eRESULT dvLPCIOExp_Burn_In_Enable(UINT8 cEnable);	//ZU860_Clare_0008
eRESULT dvLPCIOExp_Burn_In_OffTime_Set(UINT16 wTime);	//ZU860_Clare_0008
eRESULT dvLPCIOExp_Pwr_Mode_States_Set(UINT8 cData);
eRESULT dvLPCIOExp_Signal_PowerOn_Enable_Set(UINT8 cData);	//ZU860_Clare_0040
eRESULT dvLPCIOExp_IR_Top_Set(UINT8 cData);	//ZU860_Clare_0040
eRESULT dvLPCIOExp_IR_Front_Set(UINT8 cData);	//ZU860_Clare_0040
eRESULT dvLPCIOExp_IR_HDBaseT_Set(UINT8 cData);	//ZU860_Clare_0040
eRESULT dvLPCIOExp_AC_Power_On_Set(UINT8 cData);	//ZU860_Clare_0042
eRESULT dvLPCIOExp_Serial_Port_Baud_Rate_Set(UINT8 cData);	//ZU860_Clare_0042
eRESULT dvLPCIOExp_Projector_Address_Set(UINT16 wData);	//ZU860_Clare_0042
eRESULT dvLPCIOExp_OPFU_Reboot_Set(UINT8 cData);
eRESULT dvLPCIOExp_SendLog(UINT32 ulEvent, UINT32 ulTime); //ZU860_Energy_0004
eRESULT dvLPCIOExp_ErrorCode_Clear(UINT8 cData); //ZU860_Energy_0004
eRESULT dvLPCIOExp_SystemTime_Set(UINT32 ulTime);	//ZU860_Energy_0005
UINT8 dvLPCIOExp_12VDetectFlag_Get(void); //ZU860_Energy_0005
void dvLPCIOExp_ErrCode_Change(void); //ZU860_Energy_0005
eRESULT dvLPCIOExp_Keypad_LED_Enable(UINT8 cEnable);	//ZU860_Clare_0063
eRESULT dvLPCIOExp_Shutter_LED_Set(UINT8 cData); //EK816U_626U_Owen_0011
eRESULT dvLPCIOExp_LAN_Reset_Set(UINT8 cData);
eRESULT dvLPCIOExp_Power_State_Get(UINT8 *pcData);

#endif /* DVLPCIOEXP_H */

