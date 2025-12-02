#ifndef HALBOARDCTRLAPI_H
#define HALBOARDCTRLAPI_H
// ==============================================================================
// FILE NAME: HALBOARDCTRLAPI.H
// DESCRIPTION:
//
//
// modification history
// --------------------
// 19/12/2013, Leo Create
// --------------------
// ==============================================================================


#include "Common.h"

typedef enum
{
    eMCU_LIST_4337,
    eMCU_LIST_MOTOR,
    eMCU_LIST_KEYPAD,
    eMCU_LIST_PWPRROTECT,   //A70LH_Larry_0090

    eMCU_LIST_BLD_DRV1,     // A70LH_Jonas_0002 added
    eMCU_LIST_BLD_DRV2,     // A70LH_Jonas_0002 added
    eMCU_LIST_RLD_DRV,      // A70LH_Jonas_0002 added
    eMCU_LIST_LD_DRV_A,     //A70LH_JS_0023 add

    eMCU_LIST_NUMBERS,
} eMCU_LIST;

typedef enum
{
    eUART_SW_STB,        //to STB
    eUART_SW_LAN,
	eUART_SW_FMT,
    eUART_SW_LAN_DEBUG,

    eUART_SW_NUMBER,
} eUART_SWITCH;         //G100_Owen_0010//HICC2_Julie_0016

typedef enum
{
    eHAL_BOARD_STATE_STANDBY,

    eHAL_BOARD_STATE_COMMNUNICATION_ON,
    eHAL_BOARD_STATE_COMMNUNICATION_OFF,

    eHAL_BOARD_STATE_PROJ_ON,
    eHAL_BOARD_STATE_PROJ_OFF,

    eHAL_BOARD_STATE_HDBASET_ON,  //A70LH_Larry_0126
    eHAL_BOARD_STATE_HDBASET_OFF, //A70LH_Larry_0126

    eHAL_BOARD_STATE_NUMBERS,
} eHAL_BOARD_STATE;

typedef enum
{
    eDDP_I2C_SW_PW392_TO_DDP,
    eDDP_I2C_SW_LAN_TO_DDP,
    eDDP_I2C_SW_DISABLE,

    eDDP_I2C_SW_NUMBERS,
} eDDP_I2C_SW; //A70LH_Larry_0023

typedef enum
{
    eIR_ENABLE_T,
    eIR_ENABLE_F,
    eIR_ENABLE_HD,
    eIR_ENABLE_ALL,

    eIR_ENABLE_NUMBERS,
} eIR_ENABLE; // A70LH_Larry_0068


#ifdef PLATFORM_B35LH //B35LH_Casper_0008
typedef enum
{
    eB35LH_MODEL_WX = 0,
    eB35LH_MODEL_WU,
    eB35LH_MODEL_INVALID,

} eB35LH_MODEL_TYPE;
#endif

#if 0
typedef enum                                                //A35LH_JS_0044 add
{
    eDMD_DWX = 0,
    eDMD_DWU,
    eDMD_INVALID,

} eDMD_TYPE;
#endif


void halBoard_IR_Enable_Set(eIR_ENABLE eIRPin, UINT8 ucEnable); //A70LV_Larry_0060
void halBoard_R12_Trigger_Set(UINT8 ucEnable); //A70LV_Larry_0060
eRESULT halBoard_Baudrate_Set(UINT32 ulBaudrate); //A70LV_Larry_0060
void halBoard_LED_Flash(void); //A70LV_Larry_0080
void halBoard_DDP_Upgrade(void);
void halBoard_Uart_SW_Set(UINT8 cValue); //A70LV_Larry_0096
UINT8 halBoard_CustomIDGet(void); //A70LV_Larry_0132
void halBoard_ActPwrOff(void); //A70LV_Larry_0160
eRESULT halBoard_FPGA2_Version_Get(UINT8 *pucData); //A70LV_Larry_0161
void halBoard_C789_SW(UINT8 ucValue);
void halBoard_XillinxFPGA_Reset(void);      //A70LV_Doulas_0179
void halBoard_PowerCheckGet(UINT8 *pcData); //A70LV_Larry_0181
void halBoard_XillinxFPGA_Manual_Mode_Set(UINT8 ucPanel);       //A70LV_Doulas_0216
void halBoard_XillinxFPGA_Init_Set(UINT8 ucPanel, UINT8 ucIsNewVersion);
eRESULT halBoard_XillinxFPGA_Read(UINT8 ucReg,UINT8 *ucDataVal);
eRESULT halBoard_XillinxFPGA_Write(UINT8 ucReg,UINT8 *ucDataVal);
eRESULT halBoard_XillinxFPGA_VersionRead(UINT8* ucBLV,UINT8* ucBHV);        //A70LV_Doulas_0218
BOOL halBoard_XillinxFPGA_NewVersionGet(void);                              //A70LV_Doulas_0218
BOOL halBoard_XillinxFPGA_SW_Reset_Flag_Get(void);                          //A70LV_Doulas_0262
BOOL halBoard_XillinxFPGA_Version_BL_Get(UINT8 *ucData);  //A35G2_Simon_0065
BOOL halBoard_XillinxFPGA_Version_BH_Get(UINT8 *ucData);  //A35G2_Simon_0065
BOOL halBoard_XillinxFPGA_DRP_Status_Get(UINT8 *ucData);  //A35G2_Simon_0065
BOOL halBoard_XillinxFPGA_PhaseUnlock_Get(UINT8 *ucData);  //A35G2_Simon_0065
BOOL halBoard_XillinxFPGA_LDVS_Cover_Get(UINT8 *ucData);  //A35G2_Simon_0065

BOOL halBoard_LD_50V_Detection(void); //A70LH_Larry_0066
BOOL halBoard_LVPS_Detection(void);  //A70LH_Larry_0066
void halBoard_LAN_Reset_Set(UINT8 ucValue);
BOOL halBoard_LAN_Ready_Get(void);
void halBoard_LD_Reset_Set(UINT8 ucValue);
void halBoard_FrontEnd_Reset_Set(UINT8 ucValue);
void halBoard_Motor_Reset_Set(UINT8 ucValue);
void halBoard_STB_Power_Off(UINT8 ucValue);
void halBoard_STB_Burn_In_Enable(UINT8 cEnable);
void halBoard_STB_Burn_In_OffTime_Set(UINT16 wTime);
void halBoard_STB_PwrMode_States_Set(UINT8 ucValue);
void halBoard_STB_OPFU_Reboot_Set(UINT8 ucValue);
void halBoard_STB_Power_State_Get(UINT8 *pucValue);

void halBoard_PowerOn(void);
void halBoard_PowerKeyReset(void);
void halBoard_CameraEnableInit(void);
void halBoard_CameraEnableSet(UINT8 ucValue);

void halBoardXFPGA_Bist(void *pvData);  //H30K_Steven_0001

#endif /* HALBOARDCTRLAPI_H */

