/*
 * utilBISTTable_SYS.h
 *
 *  Created on: 2025年7月18日
 *      Author: steven.kao
 */

#ifndef UTILBISTTABLE_SYS_H_
#define UTILBISTTABLE_SYS_H_

#if 1//def UTILBIST_C

#include "halMCUCtrlAPI.h"
#include "halFormatter.h"
#include "halFrontEndCtrlAPI.h"
#include "halBoardCtrlAPI.h"

#define NEXT_LAYER      -1
#define NONE            0
#define CURRENT      	1


static const sBIST_LUT m_asBistLut[] =
{   //BistId                Period  DevId           Test Api
    {BIST_IMX,              10,     NONE,           NULL},
    {BIST_STB_MCU,          10,     CURRENT,        halMCUCtrl_Bist},
    {BIST_FEM_MCU,          10,     CURRENT,     	halFrontEnd_Bist},
    {BIST_LSM_MCU,          10,     NEXT_LAYER,     halMCU_Bist_Cmd_Send},
    {BIST_FMT_MCU,          10,     NONE,           NULL},
    {BIST_KEY_MCU,          10,     NONE,           NULL},
    {BIST_SCALER,           10,     NONE,           NULL},
    {BIST_WARPING,          10,     NONE,           NULL},
    {BIST_S_FPGA,           10,     NONE,           NULL},
    {BIST_DDP_1,            10,     CURRENT,        halFormatter_Bist},
    {BIST_DDP_2,            10,     NONE,           NULL},
    {BIST_DDP_UPG,          10,     NONE,           NULL},
    {BIST_PCF85063,         10,     NONE,           NULL},
    {BIST_PMIC,             10,     NONE,           NULL},
    {BIST_CLK_1,            10,     NEXT_LAYER,    	halMCU_Bist_Cmd_Send},
    {BIST_MAX31790_1,       10,     NEXT_LAYER,    	halMCU_Bist_Cmd_Send},
    {BIST_MAX31790_2,       10,     NEXT_LAYER,    	halMCU_Bist_Cmd_Send},
    {BIST_MAX31790_3,       10,     NEXT_LAYER,    	halMCU_Bist_Cmd_Send},
    {BIST_MAX31790_4,       10,     NONE,           NULL},
    {BIST_MAX31790_5,       10,     NONE,           NULL},
    {BIST_FPGA0,            10,     NONE,           NULL},
    {BIST_FPGA1,            10,     NONE,           NULL},
    {BIST_FPGA2,            10,     NEXT_LAYER,     halMCU_Bist_Cmd_Send},
    {BIST_XFPGA,            10,     CURRENT,        halBoardXFPGA_Bist},
    {BIST_G_SENSOR,         10,     NEXT_LAYER,    	halMCU_Bist_Cmd_Send},
    {BIST_G751_1,           10,     NEXT_LAYER,     halMCU_Bist_Cmd_Send},
    {BIST_G751_2,           10,     NONE,           NULL},
    {BIST_G751_3,           10,     NONE,           NULL},
    {BIST_G753_1,           10,     NONE,           NULL},
    {BIST_G753_2,           10,     NONE,           NULL},
    {BIST_ADC101_DMD,       10,     NEXT_LAYER,     halMCU_Bist_Cmd_Send},
    {BIST_ADC101_PW,        10,     NEXT_LAYER,     halMCU_Bist_Cmd_Send},
    {BIST_H_SENSOR_1,       10,     NEXT_LAYER,     halMCU_Bist_Cmd_Send},
    {BIST_P_SENSOR_1,       10,     NEXT_LAYER,     halMCU_Bist_Cmd_Send},
    {BIST_EDID_HDMI1,       10,     NONE,           NULL},
    {BIST_EDID_HDMI2,       10,     NONE,           NULL},
    {BIST_EDID_HDMI3,       10,     NONE,           NULL},
    {BIST_EDID_HDMI4,       10,     NONE,           NULL},
    {BIST_EDID_DVI,         10,     NONE,           NULL},
    {BIST_EDID_DP,          10,     NONE,           NULL},
    {BIST_EDID_HDBASET,     10,     NONE,           NULL},
    {BIST_EDID_VGA,         10,     NONE,           NULL},
    {BIST_EEPROM_STB,       10,     NEXT_LAYER,     halMCU_Bist_Cmd_Send},
    {BIST_EEPROM_FEM,       10,     NEXT_LAYER,     halFrontEnd_Bist_Cmd_Send},
    {BIST_EEPROM_LSM,       10,    NEXT_LAYER,     	halMCU_Bist_Cmd_Send},//dvLsm_NvramBist},
    {BIST_EEPROM_FMT,       10,     NONE,           NULL},
    {BIST_EEPROM_DDP,       10,     NONE,           NULL},
    {BIST_EEPROM_DMD,       10,     NEXT_LAYER,     halMCU_Bist_Cmd_Send},
    {BIST_EEPROM_ACTUATOR,  10,     NONE,           NULL},
    {BIST_ITE6634_1,        10,     NEXT_LAYER,     halFrontEnd_Bist_Cmd_Send},
    {BIST_ITE6634_2,        10,     NONE,           NULL},
    {BIST_ITE6634_3,        10,     NONE,           NULL},
    {BIST_ITE68051_1,       10,     NEXT_LAYER,     halFrontEnd_Bist_Cmd_Send},
    {BIST_ITE68051_2,       10,     NEXT_LAYER,     halFrontEnd_Bist_Cmd_Send},
    {BIST_ADV7604,          10,     NONE,           NULL},
    {BIST_DP2HDMI_1,        10,     NONE,           NULL},
    {BIST_TMP411A,          10,     NEXT_LAYER,     halMCU_Bist_Cmd_Send},
    {BIST_HDBASET,          10,     NEXT_LAYER,    	halMCU_Bist_Cmd_Send},
    {BIST_XG_SDI,           10,     NONE,           NULL},
    {BIST_AIC3204,          10,     NONE,           NULL},
    {BIST_TPA5050,          10,     NONE,           NULL},
    {BIST_APA2618,          10,     NONE,           NULL},
    {BIST_SCALER_DRAM,      10,     NONE,           NULL},
    {BIST_SCALER_FLASH,     10,     NONE,           NULL},
    {BIST_WARPING_DRAM,     10,     NONE,           NULL},
    {BIST_WARPING_FLASH,    10,     NONE,           NULL},
    {BIST_S_FPGA_FLASH,     10,     NONE,           NULL},
    {BIST_FAN_1,            10,     NEXT_LAYER,     halMCU_Bist_Cmd_Send},
    {BIST_FAN_2,            10,     NEXT_LAYER,     halMCU_Bist_Cmd_Send},
    {BIST_FAN_3,            10,     NEXT_LAYER,     halMCU_Bist_Cmd_Send},
    {BIST_FAN_4,            10,     NEXT_LAYER,     halMCU_Bist_Cmd_Send},
    {BIST_FAN_5,            10,     NEXT_LAYER,     halMCU_Bist_Cmd_Send},
    {BIST_FAN_6,            10,     NEXT_LAYER,     halMCU_Bist_Cmd_Send},
    {BIST_FAN_7,            10,     NEXT_LAYER,     halMCU_Bist_Cmd_Send},
    {BIST_FAN_8,            10,     NEXT_LAYER,     halMCU_Bist_Cmd_Send},
    {BIST_FAN_9,            10,     NEXT_LAYER,     halMCU_Bist_Cmd_Send},
    {BIST_FAN_10,           10,     NEXT_LAYER,     halMCU_Bist_Cmd_Send},
    {BIST_FAN_11,           10,     NONE,           NULL},
    {BIST_FAN_12,           10,     NONE,           NULL},
    {BIST_FAN_13,           10,     NONE,           NULL},
    {BIST_FAN_14,           10,     NONE,           NULL},
    {BIST_FAN_15,           10,     NONE,           NULL},
    {BIST_FAN_16,           10,     NONE,           NULL},
    {BIST_FAN_17,           10,     NONE,           NULL},
    {BIST_REDRIVER_HDMI3,   10,     NONE,           NULL},
    {BIST_REDRIVER_HDMI1,   10,     NONE,           NULL},
    {BIST_REDRIVER_HDMI2,   10,     NONE,           NULL},
    {BIST_LDDRV_MCU,        10,     NONE,           NULL},
    {BIST_PMUP,             10,     NONE,           NULL},
    {BIST_FPGA2_UPG,        10,     NEXT_LAYER,     halMCU_Bist_Cmd_Send},
    {BIST_FPGA0_UPG,        10,     NONE,           NULL},
    {BIST_TPS53647,         10,     NEXT_LAYER,     halMCU_Bist_Cmd_Send},
    {BIST_CAMERA,           10,     NONE,           NULL},
    {BIST_IMX_STB_UART,     10,     NONE,           NULL},
    {BIST_IMX_LSM_UART,     10,     NONE,           NULL},
    {BIST_STB_DDP_UART,     10,     NONE,           NULL},
    {BIST_STB_LVPS_UART,    10,     NONE,           NULL},
    {BIST_FMT_LDDRV_UART,   10,     NONE,           NULL},
    {BIST_CLK_2,            10,     NEXT_LAYER,     halMCU_Bist_Cmd_Send},
    {BIST_CLK_3,            10,     NEXT_LAYER,     halMCU_Bist_Cmd_Send},
    {BIST_H_SENSOR_2,       10,     NONE,           NULL},
    {BIST_P_SENSOR_2,       10,     NONE,           NULL}
};

VERIFY_SIZE_OF(m_asBistLut, sizeof(m_asBistLut[0]) * BIST_REG_NUMBERS);
#else

#endif /* UTILBIST_C */



#endif /* UTILBISTTABLE_SYS_H_ */
