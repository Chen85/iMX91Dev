#ifndef _HALMCUCTRLAPI_H_
#define _HALMCUCTRLAPI_H_
// ==============================================================================
// FILE NAME: HALMCUCTRLAPI.H
// DESCRIPTION:
//
//
// modification history
// --------------------
// 06/02/2014, Larry writen
// 25/04/2014, Larry fixed
// 27/08/2014, Larry Fixed//A70_Larry_0221
// --------------------
// ==============================================================================
#include "Common.h"


#pragma pack(push)  /* push current alignment to stack */
#pragma pack(1)     /* set alignment to 1 byte boundary */


//A70_Larry_0182 end

typedef struct
{
    INT16 nAxisX;
    INT16 nAxisY;
    INT16 nAxisZ;
} sGSENSOR_DATA;

typedef struct //A70_Larry_0269
{
    unsigned char cMode      :4;
    unsigned char cPortrait  :1;
} sGSENSOR_MODE;

typedef enum
{
    eGSENSOR_MODE_TABLETOP,
    eGSENSOR_MODE_CEILINGMOUNT,
    eGSENSOR_MODE_UPWARD,
    eGSENSOR_MODE_DOWNWARD,

    eGSENSOR_MODE_NUMBERS,
}eGSENSOR_MODE;

typedef enum //A70_Larry_0424 add
{
    eGSENSOR_MODE_PORTRAIT_P,
    eGSENSOR_MODE_PORTRAIT_N,

    eGSENSOR_MODE_PORTRAIT_NUMBERS,
}eGSENSOR_PORTRAIT_MODE;


typedef union
{
    UINT8 cFlag;

    struct
    {
        unsigned int cHighAltitude       : 1;
        unsigned int c12V_Trigger        : 1;
        unsigned int cIR_Top_Enable      : 1; //A70LH_Larry_0033
        unsigned int cIR_Fornt_Enable    : 1; //A70LH_Larry_0033
        unsigned int cIR_HD_Enable       : 1; //A70LH_Larry_0033
        unsigned int cAcPowerOn          : 1;
        unsigned int cCommunication_Mode : 1;
        unsigned int cHDBaseTEnable      : 1; //A70LH_Larry_0067
    } sBITS;
} uMCU_INIT_FLAGS; //A70LH_Larry_0013

typedef struct
{
    UINT8 cLightSourceMode; //Constant power//Constant intensity
    UINT8 cLampGain;                 // N.A.
    UINT8 cUartSwitch;
    UINT8 cBacklightLED;
    UINT8 cStatusLED;
    UINT8 cNewHardware;       //A70LH_Doulas_0502 modify
    UINT8 cConstantIntensityStep;
    UINT8 cRemoteID;
    UINT8 cBaudrate;
    uMCU_INIT_FLAGS uInitFlag;
    UINT8 cFanCoolingOffMode; //A70LH_Larry_0090
} sMCU_INIT, *psMCU_INIT; //A70LH_Larry_0013


typedef enum //A70_Larry_0281
{
    eMCU_NVRAM_RRESET_EVENT_INITIALIZE,   // Standard reset (not calibration nodes)
    eMCU_NVRAM_RRESET_EVENT_RESETALL,     // Complete reset including calibration nodes

    eMCU_NVRAM_RRESET_EVENT_NUMBERS,      // used for enumerated type range checking (DO NOT REMOVE)
} eMCU_NVRAM_RRESET_EVENT;

typedef struct
{
    UINT32 dwBANK_1_Full;
    UINT32 dwBANK_1_Eco;
    UINT32 dwBANK_2_Full;
    UINT32 dwBANK_2_Eco;
    UINT32 dwBANK_RLD_Full;
    UINT32 dwBANK_RLD_Eco;
    UINT32 dwPUMP_ON;
} sHOURS_DETIAL;


typedef enum                                                //A70LH_JS_0017 add
{
    eSYS_TH_DMD,    // DMD
    eSYS_TH_DUMMY,  // DUMMY
    eSYS_TH_SYSTEM, // SYSTEM

    eSYS_TH_SENSOR_NUMBERS,
} eSYS_TH_SENSOR;


typedef struct
{
    UINT32   dwSystemTime;
    UINT16    wErrorCode;
    UINT16    wReserved;
}sERROR_LOG;

typedef struct
{
    UINT16 wErrorCode;
    char *psErrorString;
} sERROR_MSG, *psERROR_MSG; //A70LH_Larry_0086

typedef struct
{
    UINT8 ucChannel;
    UINT8 ucDrivder;
    UINT16 uiNDivider;
    UINT32 ulPLLNDividerR;
    UINT32 ulPLLNDividerN;
    UINT8 ucPLL_OUT;
    UINT8 ucInputDrivder;
    UINT8 ucSelect;
}sLMK03328_CFG;

#pragma pack(pop)   /* restore original alignment from stack */


eRESULT halMCU_UartSwitch_Set (UINT8 cData);
eRESULT halMCU_LAN_Switch_Set(UINT8 cData);
eRESULT halMCU_SystemReadyGet(UINT8 *pcData);
eRESULT halMCU_SystemReadySet(void);
eRESULT halMCU_DDP_PowerSet(BOOL bEnable);
eRESULT halMCU_SystemCooling(void);
eRESULT halMCU_SystemPowerOff(void);
eRESULT halMCU_DebugHotKey_Set(BOOL bEnable);
eRESULT halMCU_AC_Power_On_Set(UINT8 cEnable);
eRESULT halMCU_Auto_Power_On_Set(UINT8 cData);
eRESULT halMCU_Auto_Power_On_Tmer_Set(UINT8 cData);
eRESULT halMCU_Factory_Reset_Get(UINT8 *pcData);
eRESULT halMCU_Factory_Reset_Set(eMCU_NVRAM_RRESET_EVENT eResetNvram);
eRESULT halMCU_SystemHourSet(UINT32 dwData);
eRESULT halMCU_Communication_Mode_Set(UINT8 ucMode);
eRESULT halMCU_InitSet(sMCU_INIT *psMcuInit);
eRESULT halMCU_OPFU_Set(UINT8 ucEnable);
eRESULT halMCU_OPFU_Get(UINT8 *pcData);
eRESULT halMCU_OPFU_Check_Set(UINT8 ucEnable);
eRESULT halMCU_OPFU_Check_Get(UINT8 *pcData);
eRESULT halMCU_OPFU_UpgradeExtAll(void);
eRESULT halMCU_SystemHourDetialSet(sHOURS_DETIAL *psHourDetial);
eRESULT halMCU_Uart_BaudrateSet(UINT8 cData);
eRESULT halMCU_Uart_Init(UINT8 ucUartPort);
eRESULT halMCU_Uart_DeInit(UINT8 ucUartPort);
eRESULT halMCU_ProjectorAddressSet(UINT8 cData);
eRESULT halMCU_RemoteIDSet(UINT8 cData) ;
eRESULT halMCU_R12_TriggerSet(UINT8 Enable);
eRESULT halMCU_DDP_UpgradeSet(UINT8 Enable);
eRESULT halMCU_PumpHourSet(UINT32 dwData);
UINT8 halMCU_AC_SocketGet(void);
eRESULT halMCU_HDBaseEnableSet(UINT8 cEnable);
eRESULT halMCU_LowLatencyEnableSet(UINT8 cEnable);
eRESULT halMCU_SN_Info_Set(UINT8 *pcData, UINT8 cSize);
eRESULT halMCU_Compatible4K_Set(UINT8 *pcData, UINT8 cSize);
eRESULT halMCU_SlotCardDetect_Set(void);
eRESULT halMCU_TopIR_Enable_Set(UINT8 ucEnable);
eRESULT halMCU_FrontIR_Enable_Set(UINT8 ucEnable);
eRESULT halMCU_RearIR_Enable_Set(UINT8 ucEnable);
eRESULT halMCU_HDBaseTIR_Enable_Set(UINT8 ucEnable);
eRESULT halMCU_LMK03228_Setting(UINT8 *pcData, UINT16 wSize);
eRESULT halMCU_PI3HDX1204_Setting(UINT8 cIndex, UINT8 cValue);
eRESULT halMCUCtrl_Version_Get(UINT8* pcData);
eRESULT halMCUCtrl_KeypadVersion_Get(UINT8* pcData);
eRESULT halMCUCtrl_FPGA1_Version_Get(UINT8* pcData);  //G100_Wilsonj_0025
eRESULT halMCUCtrl_FPGA2_Version_Get(UINT8* pcData);  //G100_Wilsonj_0025
eRESULT halMCUCtrl_FPGA3_Version_Get(UINT8* pcData);  //G100_Wilsonj_0025
eRESULT halMCUCtrl_WheelProtectVersion_Get(UINT8* pcData);
eRESULT halMCUCtrl_HDBaseT_Version_Get(UINT8* pcData);  //G100_Wilsonj_0069
eRESULT halMCUCtrl_SystemState_Get(UINT8* pcData);
eRESULT halMCUCtrl_HWVersion_Get(UINT8* pcData);
eRESULT halMCUCtrl_SystemState_Get(UINT8* pcData);
eRESULT halMCUCtrl_SourceDetVGA_Get(UINT8* pcData);
eRESULT halMCUCtrl_SystemState_Set(UINT8 cData); //G100_Larry_0022
eRESULT halMCU_DB_MessgaePrint(UINT8 *pcData, UINT16 wSize);
eRESULT halMCUCtrl_Serial_Port_RS232_Set(UINT8* pcData);
eRESULT halMCUCtrl_Serial_Port_HDBaseT_Set(UINT8* pcData);
eRESULT halMCUCtrl_Singal_PowerOn_Set(UINT8 ucEnable);
eRESULT halMCUCtrl_Fast_PowerOn_Set(UINT8 ucEnable);	//G100_Coda_00107
eRESULT halMCU_Upgrade_LPC1113_Set(void);  //G100_Wilsonj_0023
eRESULT halMCU_Upgrade_LPC1113_Get(UINT8 cData);  //G100_Wilsonj_0022
eRESULT halMCU_Power_Restart_Set(UINT8 cEnable);		//G100_Doulas_0002
#if 0
//G100_Julie_0002, start.
eRESULT halMCUCtrl_OPD_State_Get(UINT8* pcData);
eRESULT halMCUCtrl_OPD_Work_Get(UINT8* pcData);
eRESULT halMCUCtrl_OPD_Device_Get(UINT8 ucValue, UINT8* pcData);
eRESULT halMCUCtrl_System_Error_Reset(void);
eRESULT halMCUCtrl_System_IST_Set(UINT8 ucCount); //G100_Julie_0001
//G100_Julie_0002, end.
eRESULT halMCUCtrl_OPD_Slave_State_Get(UINT8* pcData);
eRESULT halMCUCtrl_OPD_Slave_Work_Get(UINT8* pcData);
eRESULT halMCUCtrl_OPD_Slave_Device_Get(UINT8 ucValue, UINT8* pcData);
eRESULT halMCUCtrl_System_Read_LD_I2C_INFO(UINT8* pcData);
//G100_Julie_0017, start.
eRESULT halMCUCtrl_System_I2C_Retry_Get(UINT8* pcData);
eRESULT halMCUCtrl_System_I2C_Error_Get(UINT8* pcData);
eRESULT halMCUCtrl_System_I2C_Total_Get(UINT8* pcData);
eRESULT halMCUCtrl_System2FMT_I2C_Retry_Get(UINT8* pcData);
eRESULT halMCUCtrl_System2FMT_I2C_Error_Get(UINT8* pcData);
eRESULT halMCUCtrl_System2FMT_I2C_Total_Get(UINT8* pcData);
//G100_Julie_0017, end.
#endif /* 0 */
eRESULT halMCUCtrl_AC_Voltage_Info_Get(UINT8* pcData);  //G100_Wilsonj_0045
eRESULT halMCU_3D_Sync_In_Select_Set(UINT8 cData); //G100_Steven_0017
eRESULT halMCUCtrl_Model_ID_Get(UINT8* pcData); //G100_Owen_0037
eRESULT halMCUCtrl_Standby_Error_Get(UINT8* pcData); //G100_Julie_0022
eRESULT halMCUCtrl_System_IST_Get(UINT8* pcData);
	//Cassper_ProAV
eRESULT halMCU_CW_Detect_Event_Setting(UINT8 *pcData, UINT16 wSize);
eRESULT halMCU_DualPipe_Set(BOOL ucEnable);
	//Cassper_ProAV
eRESULT halMCUCtrl_System_TemperatureSet(UINT8 *pcData, UINT16 wSize);
eRESULT halMCUCtrl_OPD_Register_Set(UINT8 reg, UINT8 size, UINT8 *pcData);
eRESULT halMCUCtrl_OPD_Register_Get(UINT8 reg, UINT8 size, UINT8* pcData);
#ifdef CUSTOM_BARCO
eRESULT halMCUCtrl_TEC_TESTER_DMD_Get(UINT8* pcData); //G100_Steven_0080
eRESULT halMCUCtrl_TEC_TESTER_RLD1_Get(UINT8* pcData);
eRESULT halMCUCtrl_TEC_TESTER_RLD2_Get(UINT8* pcData);
eRESULT halMCU_TEC_TESTER_En_Set(UINT8 cData); //G100_Steven_0080
#endif
eRESULT halMCU_Power_Restart_Set(UINT8 cEnable);		//G100_Doulas_0002
eRESULT halMCU_LAN_Path_Set(UINT8 ucData);
eRESULT halMCU_UART_ECHO_Set(UINT8 ucData);
eRESULT halMCU_LAN_IP_INFO_Set(UINT8 *pcData);
eRESULT halMCU_ProjectorID_Set(UINT8 *pcData);
eRESULT halMCU_CustomerID_Set(UINT8 cData);
eRESULT halMCU_PlatformID_Set(UINT8 cData);
eRESULT halMCUCtrl_AC_InputGet(UINT8* pcData);
eRESULT halMCU_SerialNumberSet(UINT8 *pcData);
eRESULT halMCU_BIST_Set(UINT8 ucID_Index, UINT16 ucTestCount, UINT8 ucModule, UINT8 ucLayer); //A70LK_Steven_0005
eRESULT halMCU_BIST_Get(UINT8 ucID_Index, UINT8* pcData);
eRESULT halMCU_BIST_Status_Set(UINT8 cCmd); //HICC2_Steven_0007
eRESULT halMCU_DMD_AirTightSet(UINT8 cData);
eRESULT halMCU_DMD_AirTightGet(UINT8 *pcData); //HICC2_Steven_0032
eRESULT halMCU_PIN_Protect_Set(UINT8 pcData); //R70G2_Optoma_AC_0017 //H30K_David_0038
eRESULT halMCU_PIN_Code_Set(UINT8* pcData); //R70G2_Optoma_AC_0017 //H30K_David_0038
eRESULT halMCUCtrl_PIN_Power_On_Get(UINT8* pcData); //R70G2_Optoma_AC_0017 //H30K_David_0038
eRESULT halMCU_FujiLens_Install_Abnormal_Set(UINT8 cEnable);

void halMCUCtrl_Bist(void *pvData);  //H30K_Steven_0001
void halMCU_Bist_Cmd_Send(void *pvData);

#ifdef PLATFORM_H30_4K
eRESULT halMCU_Auto_Burn_In_Parameter_Set(void);
#endif
eRESULT halMCU_Energy_Saving_Set(UINT8 ucData);// HICC2_Bruce_0023
eRESULT halMCUCtrl_HDBT_Status_Get(UINT8* pcData);
eRESULT halMCUCtrl_PowerOn_Recover_Get(UINT8* pcData);

#endif //_HALMCUCTRLAPI_H_
