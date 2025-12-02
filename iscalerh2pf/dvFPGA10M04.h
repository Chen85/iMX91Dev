// ===============================================================================
// FILE NAME: dvFPGA10M04.h
// DESCRIPTION:
//
//
// Modification History
// --------------------
// 2017/06/23, Larry Create
// --------------------
// ===============================================================================

#ifndef DVFPGA10M04_H
#define DVFPGA10M04_H

#include "Common.h"

typedef enum
{
    eFPGA_VIDEO,
    eFPGA_SYSTEM,
    eFPGA_LD,

    eFPGA_NUMBER,
} eFPGA_ID;

typedef struct
{
    UINT8 ucBus;
    UINT8 ucAddress;
} sFPGA10M04CFG;

#define FPGA_VERSION_REG 0x01
#define FPGA_YEAR_REG    0x02
#define FPGA_MONTH_REG   0x03
#define FPGA_DAY_REG     0x04

#define FPGA_TEMP1_REG   0x20
#define FPGA_TEMP2_REG   0x21
#define FPGA_XFPGA_REG   0x26   //A70LV_Doulas_0177

//eRESULT dvFPGA_Reg_Write(BYTE cBus, WORD wReg, WORD wDataNum, BYTE *pcData);
//eRESULT dvFPGA_Reg_Read(BYTE cBus, WORD wReg, WORD wDataNum, BYTE *pcData);
void dvFPGA_Initial(void);



typedef enum
{
    eFPGA_0_REG_VERSION = FPGA_VERSION_REG,
    eFPGA_0_REG_YEAR    = FPGA_YEAR_REG,
    eFPGA_0_REG_MONTH   = FPGA_MONTH_REG,
    eFPGA_0_REG_DAY     = FPGA_DAY_REG,

    eFPGA_0_REG_VIDEO_SWITCH        =   0x21,
    eFPGA_0_REG_AUTO_FRAME_DELAY    =   0x23,

    eFPGA_0_REG_NUMBER,
} eFPGA_0_REG;

typedef enum
{
    eFPGA_1_REG_VERSION = FPGA_VERSION_REG,
    eFPGA_1_REG_YEAR    = FPGA_YEAR_REG,
    eFPGA_1_REG_MONTH   = FPGA_MONTH_REG,
    eFPGA_1_REG_DAY     = FPGA_DAY_REG,

    eFPGA_1_REG_COM_ENABLE_1                =   0x20,
    eFPGA_1_REG_COM_ENABLE_2                =   0x21,
    eFPGA_1_REG_ACT_ENABLE_1                =   0x22,
    eFPGA_1_REG_ACT_ENABLE_2                =   0x23,
    eFPGA_1_REG_ACT_ENABLE_3                =   0x24,

    eFPGA_1_REG_RESET_1                     =   0x26,
    eFPGA_1_REG_RESET_2                     =   0x27,
    eFPGA_1_REG_UART_SW                     =   0x30,

    eFPGA_1_REG_COM_CHECK                   =   0x41,
    eFPGA_1_REG_ACT_POWER_CHECK_1           =   0x42,
    eFPGA_1_REG_ACT_POWER_CHECK_2           =   0x43,

    eFPGA_1_REG_POWER_RAIL_COM_VS100_1V0    =   0xB0,
    eFPGA_1_REG_POWER_RAIL_COM_1V8          =   0xB1,
    eFPGA_1_REG_POWER_RAIL_COM_3V3          =   0xB2,
    eFPGA_1_REG_POWER_RAIL_COM_821_1V0      =   0xB3,
	eFPGA_1_REG_POWER_RAIL_COM_821_3V3      =   0xB4,
    eFPGA_1_REG_POWER_RAIL_ACT_1V0          =   0xB5,
    eFPGA_1_REG_POWER_RAIL_ACT_789_1V0      =   0xB6,
    eFPGA_1_REG_POWER_RAIL_ACT_1V2          =   0xB7,
    eFPGA_1_REG_POWER_RAIL_ACT_1V5          =   0xB8,
    eFPGA_1_REG_POWER_RAIL_ACT_789_1V5      =   0xB9,
    eFPGA_1_REG_POWER_RAIL_ACT_1V8          =   0xBA,
    eFPGA_1_REG_POWER_RAIL_ACT_2V5          =   0xBB,
    eFPGA_1_REG_POWER_RAIL_ACT_F1V0         =   0xBC,
    eFPGA_1_REG_POWER_RAIL_ACT_1V1          =   0xBD,
    eFPGA_1_REG_POWER_RAIL_ACT_3V3          =   0xBE,

    eFPGA_1_REG_NUMBER,
} eFPGA_1_REG;

typedef enum
{
    ePHY_NSET       = 0x2601,
    eUSB_HUB_NSET   = 0x2602,
    eLAN_NRET       = 0x2604,
    eC821_NSET      = 0x2608,
    eC789_NSET      = 0x2610,
    eFE54113_NSET   = 0x2620,
    eLD54113_NSET   = 0x2640,
    eXFPGA_NSET     = 0x2680,
    eWP1113_NSET    = 0x2701,
    eLENS54113_NSET = 0x2702,

    eFPGA_1_REG_RESET_NUMBER,
} eFPGA_1_REG_RESET;


eRESULT dvFPGA_Version_Read(UINT8 *pucData); //A70LV_Larry_0161
eRESULT dvFPGA_System_Read(UINT16 uiReg, UINT8 *pucData);
eRESULT dvFPGA_System_Write(UINT16 uiReg, UINT8 *pucData);
eRESULT dvFPGA_XillinxFPGA_Reset(void);       //A70LV_Doulas_0177
eRESULT dvFPGA_XillinxFPGA_Reset11ms(void);     //A70LV_Doulas_0216
eRESULT dvFPGA_Reset_Control(UINT16 uiReg, UINT16 uiTimeMS);


#endif /* DVFPGA10M04_H */

