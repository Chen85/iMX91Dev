// ==============================================================================
// FILE NAME: HALMCUCTRL.C
// DESCRIPTION:
//
//
// modification history
// --------------------
// 06/02/2014, Larry writen
// 07//03/2014, A70_Larry_0118 fixed
// 25/04/2014, Larry rename
// 30/05/2014, fix name //A70_Larry_0182
// A70_Larry_0306 fixd
// --------------------
// ==============================================================================

#include "halMCUCtrlAPI.h"
#include "utilCLICmdAPI.h"
#include "utilHostAPI.h"
#include "utilOPD_TEST.h"
#include "utilDbgMsg.h"
#include "dvMCUDriver.h"

#if(BIST_ENABLE) //HICC2_Steven_0001
#include "utilBIST.h"

void halMCU_Bist_Cmd_Send(void *pvData)  //H30K_Steven_0001
{
    eRESULT eResult = rcSUCCESS;

    if(utilBIST_Status_Get()!= eBIST_STATUS_IDLE) //write cmd
    {
    	UINT8 pcData[eBIST_MSG_SET_SZ];

    	sBIST_DEV *psDev = (sBIST_DEV*)pvData;
    	sBIST_CMD sSendCmd = utilBIST_CmdInfo_Get();

    	memset(pcData, 0, eBIST_MSG_SET_SZ);

    	eResult = utilHost_SystemSet(eCMD_MODULE_BIST, eBIST_MSG_SET_VIA_I2C, eBIST_MSG_SET_SZ, (UINT8*)&sSendCmd);
    }
    else   // get data
    {
    	UINT8 pcData[eBIST_MSG_GET_SZ];

    	UINT8 ucIndex =0, temp =0;

    	sBIST_DATA *psGetData = (sBIST_DATA*)pvData;
    	sBIST_CMD sSendCmd = utilBIST_CmdInfo_Get();
    	memset(pcData, 0, eBIST_MSG_GET_SZ);

    	for(ucIndex =0; ucIndex < 10; ucIndex++)
    	{

    		MS_SLEEP(sSendCmd.lCount * 10);

    		utilHost_SystemGet(eCMD_MODULE_BIST, 0, 1, &temp);

    		if(temp == SUB_LAYER_FINISH)
    		{
    			break;
    		}
    	}

    	if(temp != SUB_LAYER_FINISH)
    	{
    		LOG_MSG(db_BIST, "!!! ERROR !!! %s<%d>: %s() Sub Cmd not finish  %d\n", __FILE__, __LINE__, __func__, temp);
    	}
    	else
    	{
    		eResult = utilHost_SystemGet(eCMD_MODULE_BIST, (UINT8)sSendCmd.lBistId, eBIST_MSG_GET_SZ, pcData);
    	}
    	memcpy(psGetData, pcData, sizeof(sBIST_DATA));

    }
}

eRESULT halMCU_BIST_Set(UINT8 ucID_Index, UINT16 ucTestCount, UINT8 ucModule, UINT8 ucLayer) //A70LK_Steven_0005
{
    eRESULT eResult = rcSUCCESS;
    UINT8 pcData[eBIST_MSG_SET_SZ];  //A70LK_Steven_0007

    pcData[0] = ucID_Index;
    pcData[1] = ucTestCount >> 8;
    pcData[2] = ucTestCount & 0xFF;
    pcData[3] = ucModule;  //A70LK_Steven_0004
    pcData[4] = ucLayer;   //A70LK_Steven_0005

    //if(ucID_Index == BIST_STB_MCU_UART)
    //    eResult = utilHost_EventWrite(eCMD_MODULE_BIST, eMSG_TYPE_UART_1, eBIST_MSG_SET_VIA_UART, eBIST_MSG_SET_SZ, pcData, FALSE);
    //else
    	eResult = utilHost_SystemSet(eCMD_MODULE_BIST, eBIST_MSG_SET_VIA_I2C, eBIST_MSG_SET_SZ, pcData);


    return eResult;
}

eRESULT halMCU_BIST_Get(UINT8 ucID_Index, UINT8* pcData)
{
    eRESULT eResult = rcINVALID;

    //if(ucID_Index == BIST_STB_MCU_UART)
    //    eResult = utilHost_EventRead(eCMD_MODULE_BIST, eMSG_TYPE_UART_1, ucID_Index, 1, pcData);
    //else
    	eResult = utilHost_SystemGet(eCMD_MODULE_BIST, ucID_Index, eBIST_MSG_GET_SZ, pcData);

    return eResult;
}

eRESULT halMCU_BIST_Status_Set(UINT8 cCmd) //HICC2_Steven_0007
{
    UINT8 pcData = 0;
    eRESULT eResult = utilHost_SystemSet(eCMD_MODULE_BIST, cCmd, 1, &pcData); //HICC2_Steven_0007
    return(eResult);
}
#endif //HICC2_Steven_0001

eRESULT halMCU_UartSwitch_Set (UINT8 cData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemSet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_UART_SWITCH, sizeof(UINT8), &cData);

    return eResult;
}

eRESULT halMCU_3D_Sync_In_Select_Set(UINT8 cData) //G100_Steven_0017
{
    eRESULT eResult = rcSUCCESS;

    //e3D_SYNC_INPUT_SELECTION_EXTERNAL_INPUT_3D_SYNC = 0,
    //e3D_SYNC_INPUT_SELECTION_C821_OUTPUT_3D_SYNC,

    eResult = utilHost_SystemSet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_3D_SYNC_OUT, sizeof(UINT8), &cData);

    return eResult;
}

eRESULT halMCU_LAN_Switch_Set(UINT8 cData)  //T100_Simon_0082
{
    eRESULT eResult = rcSUCCESS;

    //0: from Slot1
    //1: from Slot2
    LOG_MSG(db_HAL_MCU,"(func:%s, line:%d) LAN_Switch[%d]\r\n", __FUNCTION__, __LINE__, cData);

	//eResult = utilHost_SystemSet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_SLOTCARD_LANSWITCH, sizeof(UINT8), (UINT8*)&cData);

    return eResult;
}

eRESULT halMCU_SystemReadyGet(UINT8 *pcData)
{
    //0x00 on board
    //0x66 signal board
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemGet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_HOST_READY, 1, pcData);

    return eResult;
}

eRESULT halMCU_SystemReadySet(void)//A70_Larry_0133
{
    eRESULT eResult = rcSUCCESS;
    UINT8 cData = 0;

    eResult = utilHost_SystemSet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_HOST_READY, eSYSTEM_MSG_HOST_READY_SZ, &cData);

    return eResult;
}

eRESULT halMCU_DDP_PowerSet(BOOL bEnable)
{
    eRESULT eResult = rcSUCCESS;

#ifdef PLATFORM_H60_2K

    #ifndef DISABLE_POWERDOWN_DDP
        eResult = utilHost_SystemSet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_DDP_PWR, eSYSTEM_MSG_DDP_PWR_SZ, (UINT8*)&bEnable);
    #endif

#else

    eResult = utilHost_SystemSet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_DDP_PWR, eSYSTEM_MSG_DDP_PWR_SZ, (UINT8*)&bEnable);

#endif

    return eResult;
}

eRESULT halMCU_SystemCooling(void)
{
    eRESULT eResult = rcSUCCESS;
    UINT8 cData = 0;
    //eResult = utilHost_SystemSet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_SYSTEM_COOLING, eSYSTEM_MSG_SYSTEM_COOLING_SZ, &cData);

	//by Uart
	utilHost_EventWrite(eCMD_MODULE_SYSTEM, eMSG_TYPE_UART_1, eSYSTEM_MSG_SYSTEM_COOLING, eSYSTEM_MSG_SYSTEM_COOLING_SZ, (UINT8*)&cData, FALSE);
    return eResult;
}

eRESULT halMCU_SystemPowerOff(void)
{
    eRESULT eResult = rcSUCCESS;
    UINT8 cData = 0;

    //eResult = utilHost_SystemSet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_SYSTEM_POWER_OFF, eSYSTEM_MSG_SYSTEM_POWER_OFF_SZ, &cData);

	//by Uart
	utilHost_EventWrite(eCMD_MODULE_SYSTEM, eMSG_TYPE_UART_1, eSYSTEM_MSG_SYSTEM_POWER_OFF, eSYSTEM_MSG_SYSTEM_POWER_OFF_SZ, (UINT8*)&cData, FALSE);
    return eResult;
}

eRESULT halMCU_DebugHotKey_Set(BOOL bEnable)//A70_Larry_0155
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemSet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_DEBUG_HOTKEY, eSYSTEM_MSG_DEBUG_HOTKEY_SZ, (UINT8*)&bEnable);

    return eResult;
}

eRESULT halMCU_AC_Power_On_Set(UINT8 cEnable)
{
    eRESULT eResult = rcSUCCESS;

    LOG_MSG(db_HAL_MCU,"(func:%s, line:%d) AC_Power_On[%d]\r\n", __FUNCTION__, __LINE__, cEnable);

    eResult = utilHost_SystemSet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_AC_PWR_ON, eSYSTEM_MSG_AC_PWR_ON_SZ, &cEnable);

    return eResult;
}

eRESULT halMCU_FujiLens_Install_Abnormal_Set(UINT8 cEnable) //HICC2_Julie_0070//H30K_Julie_0005
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemSet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_FUJILENS_INSTALL_ABNORMAL, eSYSTEM_MSG_AC_PWR_ON_SZ, &cEnable);

    return eResult;
}

eRESULT halMCU_Auto_Power_On_Set(UINT8 cData)
{
    eRESULT eResult = rcSUCCESS;

    LOG_MSG(db_HAL_MCU,"(func:%s, line:%d) Auto_Power_On[%d]\r\n", __FUNCTION__, __LINE__, cData);

    eResult = utilHost_SystemSet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_AUTO_PWR_ON, eSYSTEM_MSG_AUTO_PWR_ON_SZ, &cData);

    return eResult;
}

#ifdef PLATFORM_H30_4K
eRESULT halMCU_Auto_Burn_In_Parameter_Set(void)
{
    eRESULT eResult = rcSUCCESS;
	BYTE cData[eSYSTEM_MSG_BURN_IN_PARAMETER_SZ]={0};
	WORD wCycle;
	BYTE ucBurnInOn;

    LOG_MSG(db_HAL_MCU,"(func:%s, line:%d) Burn_In_Parameter\r\n", __FUNCTION__, __LINE__);

	palDataMgr_Data_Access(edcBURNIN_CYCLE, edaREAD, &wCycle);
	palDataMgr_Data_Access(edcBURNIN_ON, edaREAD, &ucBurnInOn);
	cData[0] = wCycle >> 8;
	cData[1] = wCycle & 0xFF;
	cData[2] = ucBurnInOn;

    eResult = utilHost_SystemSet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_BURN_IN_PARAMETER, eSYSTEM_MSG_BURN_IN_PARAMETER_SZ, cData);

    return eResult;
}
#endif

eRESULT halMCU_Energy_Saving_Set(UINT8 ucData)// HICC2_Bruce_0023
{
    eRESULT eResult = rcSUCCESS;

    LOG_MSG(db_HAL_MCU,"%s(%d)[]\r\n", __FUNCTION__, ucData);

    eResult = utilHost_SystemSet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_ENERGY_SAVING, 1, &ucData);

    return eResult;
}

eRESULT halMCU_Auto_Power_On_Tmer_Set(UINT8 cData)
{
    eRESULT eResult = rcSUCCESS;

    LOG_MSG(db_HAL_MCU,"(func:%s, line:%d) Auto_Power_On_Tmer[%d]\r\n", __FUNCTION__, __LINE__, cData);

    eResult = utilHost_SystemSet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_AUTO_PWR_ON_TIMER, eSYSTEM_MSG_AUTO_PWR_ON_TIMER_SZ, &cData);

    return eResult;
}

eRESULT halMCU_Factory_Reset_Get(UINT8 *pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemGet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_FACTORY_RESET_MODE, eSYSTEM_MSG_FACTORY_RESET_MODE_SZ, pcData);

    return eResult;
}

eRESULT halMCU_Factory_Reset_Set(eMCU_NVRAM_RRESET_EVENT eResetNvram)
{
    eRESULT eResult = rcSUCCESS;

    LOG_MSG(db_HAL_MCU,"(func:%s, line:%d) Factory_Reset[%d]\r\n", __FUNCTION__, __LINE__, eResetNvram);

    eResult = utilHost_SystemSet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_FACTORY_RESET_MODE, eSYSTEM_MSG_FACTORY_RESET_MODE_SZ, (UINT8*)&eResetNvram);

    return eResult;
}

eRESULT halMCU_SystemHourSet(UINT32 dwData)
{
    eRESULT eResult = rcSUCCESS;

    LOG_MSG(db_HAL_MCU,"(func:%s, line:%d) SystemHour[%d]\r\n", __FUNCTION__, __LINE__, dwData);

    eResult = utilHost_SystemSet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_SYSTEM_HOURS, eSYSTEM_MSG_SYSTEM_HOURS_SZ, (UINT8*)&dwData);

    return eResult;
}

eRESULT halMCU_Communication_Mode_Set(UINT8 ucMode)
{
    eRESULT eResult = rcSUCCESS;

    LOG_MSG(db_HAL_MCU,"(func:%s, line:%d) Comm_Mode[%d]\r\n", __FUNCTION__, __LINE__, ucMode);

    eResult = utilHost_SystemSet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_COMMUNICATION_MODE, eSYSTEM_MSG_COMMUNICATION_MODE_SZ, &ucMode);

    return eResult;
}

eRESULT halMCU_InitSet(sMCU_INIT *psMcuInit) //A70_Larry_0264
{
    eRESULT eResult = rcSUCCESS;

    LOG_MSG(db_HAL_MCU,"(func:%s, line:%d) MCU_Init[%d]\r\n", __FUNCTION__, __LINE__, psMcuInit);

    eResult = utilHost_SystemSet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_MCU_INIT, sizeof(sMCU_INIT)/sizeof(UINT8), (UINT8*)&psMcuInit);

    return eResult;
}

eRESULT halMCU_OPFU_Set(UINT8 ucEnable) //A70_Larry_0379
{
    eRESULT eResult = rcSUCCESS;
    LOG_MSG(db_UPGRADE, "halMCU_OPFU_Set %d\n", ucEnable);  //G100_Simon_0063

    eResult = utilHost_SystemSet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_OPFU, 1, &ucEnable);  //G100_Simon_0063

    return eResult;
}

eRESULT halMCU_OPFU_Get(UINT8 *pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemGet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_OPFU, 1, pcData);

    return eResult;
}

eRESULT halMCU_OPFU_Check_Set(UINT8 ucEnable) //G100_Simon_0064
{
    eRESULT eResult = rcSUCCESS;
    LOG_MSG(db_UPGRADE, "halMCU_OPFU_Check_Set %d\n", ucEnable);

    eResult = utilHost_SystemSet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_OPFU_CHECK, 1, &ucEnable);

    return eResult;
}

eRESULT halMCU_OPFU_Check_Get(UINT8 *pcData) //G100_Simon_0064
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemGet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_OPFU_CHECK, 1, pcData);

    return eResult;
}

eRESULT halMCU_OPFU_UpgradeExtAll(void)
{
    eRESULT eResult = rcSUCCESS;
    LOG_MSG(db_UPGRADE, "(%s, %d)\n", __FUNCTION__, __LINE__);

    eResult = utilHost_SystemSet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_UPGRADE_PERIPHERAL, 0, NULL);

    return eResult;
}



eRESULT halMCU_SystemHourDetialSet(sHOURS_DETIAL *psHourDetial) //A70_Larry_0382
{
    eRESULT eResult = rcSUCCESS;

    //eResult = utilHost_SystemSet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_HOURS_DETIAL, (UINT16)(sizeof(sHOURS_DETIAL)/sizeof(UINT8)), (UINT8*)psHourDetial);

    return eResult;
}

eRESULT halMCU_Uart_BaudrateSet(UINT8 cData) //A70_Larry_0411
{
    eRESULT eResult = rcSUCCESS;

    LOG_MSG(db_HAL_MCU,"halMCU_Uart_BaudrateSet [%d] \n", cData);
    eResult = utilHost_SystemSet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_UART_BAUDRATE, eSYSTEM_MSG_UART_BAUDRATE_SZ, &cData);

    return eResult;
}



eRESULT halMCU_DMD_AirTightSet(UINT8 cData) //A70_Larry_0411
{
    eRESULT eResult = rcSUCCESS;

    //LOG_MSG(db_HAL_MCU,"halMCU_Uart_DMD_AirTightSet [%d] \n", cData);
    eResult = utilHost_SystemSet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_DMD_AIRTIGHT_STATUS, eSYSTEM_MSG_DMD_AIRTIGHT_STATUS_SZ, &cData);

    return eResult;
}

eRESULT halMCU_DMD_AirTightGet(UINT8 *pcData) //HICC2_Steven_0032
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemGet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_DMD_AIRTIGHT_STATUS, 1, pcData);

    return eResult;
}

eRESULT halMCU_ProjectorAddressSet(UINT8 ucData)
{
    eRESULT eResult = rcSUCCESS;

    LOG_MSG(db_HAL_MCU,"halMCU_ProjectorAddressSet [%d] \n", ucData);
    eResult = utilHost_SystemSet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_PROJECTOR_ADDRESS, eSYSTEM_MSG_PROJECTOR_ID_SZ, &ucData);

    return eResult;
}

eRESULT halMCU_RemoteIDSet(UINT8 cData)
{
    eRESULT eResult = rcSUCCESS;

    LOG_MSG(db_HAL_MCU,"halMCU_RemoteIDSet [%d] \n", cData);
    eResult = utilHost_SystemSet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_REMOTE_ID, eSYSTEM_MSG_REMOTE_ID_SZ, &cData);

    return eResult;
}

eRESULT halMCU_R12_TriggerSet(UINT8 Enable)
{
    eRESULT eResult = rcSUCCESS;

    LOG_MSG(db_HAL_MCU,"halMCU_R12_TriggerSet [%d] \n", Enable);
    eResult = utilHost_SystemSet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_R12_TRIGGER, eSYSTEM_MSG_R12_TRIGGER_SZ, &Enable);

    return eResult;
}

eRESULT halMCU_DDP_UpgradeSet(UINT8 Enable)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemSet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_DDP_UPGRADE_ENABLE, eSYSTEM_MSG_DDP_UPGRADE_ENABLE_SZ, (UINT8*)&Enable);

    return eResult;
}

eRESULT halMCU_PumpHourSet(UINT32 dwData)
{
    eRESULT eResult = rcSUCCESS;

    //eResult = utilHost_SystemSet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_PUMP_HOURS, sizeof(UINT32), (UINT8*)&dwData);

    return eResult;
}

eRESULT halMCU_HDBaseEnableSet(UINT8 cEnable) //A70LH_Larry_0067
{
    eRESULT eResult = rcSUCCESS;

    LOG_MSG(db_HAL_MCU,"(func:%s, line:%d) HDBaseEnable[%d]\r\n", __FUNCTION__, __LINE__, cEnable);

    eResult = utilHost_SystemSet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_HDBASET_EN, eSYSTEM_MSG_HDBASET_EN_SZ, &cEnable);

    return eResult;
}

eRESULT halMCU_SN_Info_Set(UINT8 *pcData, UINT8 cSize)
{
    eRESULT eResult = rcSUCCESS;

	//eResult = utilHost_SystemSet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_SN_INFO, cSize, (UINT8*)pcData);

    return eResult;
}

eRESULT halMCU_SlotCardDetect_Set(void)
{
    eRESULT eResult = rcSUCCESS;

    UINT8 ucData = 0 ;

	//eResult = utilHost_SystemSet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_SLOTCARD_REDETECT, 1, &ucData);

    return eResult;

}

eRESULT halMCU_TopIR_Enable_Set(UINT8 ucEnable)
{
    eRESULT eResult = rcSUCCESS;

    LOG_MSG(db_HAL_MCU,"halMCU_TopIR_Enable_Set [%d]\n",ucEnable);
    eResult = utilHost_SystemSet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_IR_TOP_ENABLE, eSYSTEM_MSG_IR_TOP_ENABLE_SZ, &ucEnable);

    return eResult;
}

eRESULT halMCU_FrontIR_Enable_Set(UINT8 ucEnable)
{
    eRESULT eResult = rcSUCCESS;

    LOG_MSG(db_HAL_MCU,"halMCU_FrontIR_Enable_Set [%d]\n",ucEnable);
    eResult = utilHost_SystemSet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_IR_FORNT_ENABLE, eSYSTEM_MSG_IR_FORNT_ENABLE_SZ, &ucEnable);

    return eResult;
}

eRESULT halMCU_RearIR_Enable_Set(UINT8 ucEnable)
{
    eRESULT eResult = rcSUCCESS;

    LOG_MSG(db_HAL_MCU,"halMCU_RearIR_Enable_Set [%d]\n",ucEnable);
    eResult = utilHost_SystemSet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_IR_REAR_ENABLE, eSYSTEM_MSG_IR_REAR_ENABLE_SZ, &ucEnable);

    return eResult;
}

eRESULT halMCU_HDBaseTIR_Enable_Set(UINT8 ucEnable)
{
    eRESULT eResult = rcSUCCESS;

    LOG_MSG(db_HAL_MCU,"halMCU_HDBaseTIR_Enable_Set [%d]\n",ucEnable);
    eResult = utilHost_SystemSet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_IR_HD_ENABLE, eSYSTEM_MSG_IR_HD_ENABLE_SZ, &ucEnable);

    return eResult;
}

eRESULT halMCUCtrl_Version_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemGet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_SYSTEM_VERSION, eSYSTEM_MSG_SYSTEM_VERSION_SZ, pcData);

    return eResult;
}

eRESULT halMCUCtrl_KeypadVersion_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemGet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_KEYPAD_VERSION, eSYSTEM_MSG_KEYPAD_VERSION_SZ, pcData);

    return eResult;
}

eRESULT halMCUCtrl_WheelProtectVersion_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemGet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_PWPROTECT_VERSION, eSYSTEM_MSG_WHEELPROTECT_VERSION_SZ, pcData);

    return eResult;
}

eRESULT halMCUCtrl_FPGA1_Version_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemGet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_FPGA1_VERSION, eSYSTEM_MSG_FPGA1_VERSION_SZ, pcData);

    return eResult;
}

eRESULT halMCUCtrl_FPGA2_Version_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemGet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_FPGA2_VERSION, eSYSTEM_MSG_FPGA2_VERSION_SZ, pcData);

    return eResult;
}

eRESULT halMCUCtrl_FPGA3_Version_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemGet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_FPGA3_VERSION, eSYSTEM_MSG_FPGA3_VERSION_SZ, pcData);

    return eResult;
}

eRESULT halMCUCtrl_HDBaseT_Version_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemGet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_HDBASET_VERSION, eSYSTEM_MSG_HDBASET_VERSION_SZ, pcData);

    return eResult;
}

eRESULT halMCUCtrl_HDBT_Status_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemGet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_HDBT_STATUS, eSYSTEM_MSG_SYSTEM_HDBT_STATUS_SZ, pcData);
    return eResult;
}

eRESULT halMCUCtrl_PowerOn_Recover_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemGet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_POWERON_RECOVER, 1, pcData);
    return eResult;
}


eRESULT halMCUCtrl_HWVersion_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemGet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_HW_VERSION, eSYSTEM_MSG_HW_VERSION_SZ, pcData);

    return eResult;
}

eRESULT halMCUCtrl_SystemState_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    //eResult = utilHost_SystemGet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_SYSTEM_STATE, eSYSTEM_MSG_SYSTEM_STATE_SZ, pcData);

	//by uart
    eResult = utilHost_EventRead(eCMD_MODULE_SYSTEM, eMSG_TYPE_UART_1, eSYSTEM_MSG_SYSTEM_STATE, eSYSTEM_MSG_SYSTEM_STATE_SZ, pcData);
    return eResult;
}

eRESULT halMCUCtrl_SourceDetVGA_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemGet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_VGADET, eSYSTEM_MSG_VGADET_SZ, pcData);

    return eResult;
}

eRESULT halMCUCtrl_SystemState_Set(UINT8 cData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemSet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_SYSTEM_STATE, eSYSTEM_MSG_SYSTEM_STATE_SZ, &cData);

    return eResult;
}

eRESULT halMCUCtrl_Serial_Port_RS232_Set(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    //printf("halMCUCtrl_Serial_Port_RS232_Set %d \n", pcData);
//    eResult = utilHost_SystemSet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_RS232, eSYSTEM_MSG_RS232_SZ, pcData);

    return eResult;
}

eRESULT halMCUCtrl_Serial_Port_HDBaseT_Set(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    //printf("halMCUCtrl_Serial_Port_HDBaseT_Set %d \n", pcData);
//    eResult = utilHost_SystemSet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_HDBASET, 1, pcData);

    return eResult;
}


eRESULT halMCUCtrl_System_IST_Set(UINT8 ucCount)
{
    eRESULT eResult = rcSUCCESS;

    //LOG_MSG(db_HAL_MCU,"(func:%s, line:%d) System i2c[%d]\r\n", __FUNCTION__, __LINE__, ucCount);

    eResult = utilHost_SystemSet(eCMD_MODULE_OPD, eOPD_MSG_I2C_STRESS_TEST, 1, &ucCount);

    return eResult;
}

eRESULT halMCUCtrl_System_IST_Get(UINT8* pcData)
{
	eRESULT eResult = rcSUCCESS;

	eResult = utilHost_SystemGet(eCMD_MODULE_OPD, eOPD_MSG_I2C_STRESS_TEST, 0, pcData);

	return eResult;
}


eRESULT halMCU_LMK03228_Setting(UINT8 *pcData, UINT16 wSize)
{
    eRESULT eResult = rcSUCCESS;
    UINT8 cData = 0;

    eResult = utilHost_SystemSet(eCMD_MODULE_SCALER, eSCALER_MSG_LMK03328_0, wSize, pcData);

    return eResult;
}

eRESULT halMCU_PI3HDX1204_Setting(UINT8 cIndex, UINT8 cValue)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemSet(eCMD_MODULE_SCALER, (eSCALER_MSG_PI3HDX1204_0+cIndex) , 1, &cValue);

    return eResult;
}

eRESULT halMCU_DB_MessgaePrint(UINT8 *pcData, UINT16 wSize)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemUartWrite(eCMD_MODULE_HOST, eHOST_MSG_DB_MESSAGE, wSize, pcData, FALSE);

    return eResult;
}

eRESULT halMCU_CW_Detect_Event_Setting(UINT8 *pcData, UINT16 wSize)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemSet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_CW_DETECT, 1, pcData);

    return eResult;
}

eRESULT halMCU_DualPipe_Set(BOOL ucEnable)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemSet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_DUAL_PIPE, 1, (UINT8 *)&ucEnable);

    return eResult;
}

eRESULT halMCUCtrl_System_TemperatureSet(UINT8 *pcData, UINT16 wSize)
{
    eRESULT eResult = rcSUCCESS;

    //by uart
    eResult = utilHost_SystemUartWrite(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_TEMPERATURE, wSize, pcData, FALSE);

    return eResult;
}

eRESULT halMCUCtrl_Singal_PowerOn_Set(UINT8 ucEnable)
{
    eRESULT eResult = rcERROR;

    eResult = utilHost_SystemSet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_SIGNAL_PWR_ON_ENABLE, 1, &ucEnable);

    return eResult;
}

eRESULT halMCUCtrl_Fast_PowerOn_Set(UINT8 ucEnable)	//G100_Coda_00107
{
    eRESULT eResult = rcERROR;

    eResult = utilHost_SystemSet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_FAST_POWER_ON_ENABLE, 1, &ucEnable);

    return eResult;
}


//G100_Wilsonj_0022 Start
eRESULT halMCU_Upgrade_LPC1113_Set(void)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemSet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_UPGRADE_PERIPHERAL, 0, NULL);

    return eResult;
}

eRESULT halMCU_Upgrade_LPC1113_Get(UINT8 cData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemSet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_UPGRADE_PERIPHERAL, 1, &cData);

    return eResult;
}
//G100_Wilsonj_0022 End

eRESULT halMCU_Power_Restart_Set(UINT8 cEnable)		//G100_Doulas_0002
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemSet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_POWER_RESTART, 1, &cEnable);

    //printf("halMCU_AC_Power_On_Set %d\n",cEnable);

    return eResult;
}


//G100_Wilsonj_0045 Start
eRESULT halMCUCtrl_AC_Voltage_Info_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemGet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_AC_INPUT_DET, eSYSTEM_MSG_AC_INPUT_DET_SZ, pcData);

    return eResult;
}
//G100_Wilsonj_0045 End

eRESULT halMCUCtrl_Model_ID_Get(UINT8* pcData)  //G100_Owen_0037
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemGet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_MODEL_ID, eSYSTEM_MSG_MODEL_ID_SZ, pcData);

    return eResult;
}

#ifdef CUSTOM_BARCO
//G100_Steven_0080
eRESULT halMCUCtrl_TEC_TESTER_DMD_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemGet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_TEC_TESTER_DMD, eSYSTEM_MSG_TEC_TESTER_SZ, pcData);

    return eResult;
}

eRESULT halMCUCtrl_TEC_TESTER_RLD1_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemGet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_TEC_TESTER_RLD1, eSYSTEM_MSG_TEC_TESTER_SZ, pcData);

    return eResult;
}

eRESULT halMCUCtrl_TEC_TESTER_RLD2_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemGet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_TEC_TESTER_RLD2, eSYSTEM_MSG_TEC_TESTER_SZ, pcData);

    return eResult;
}

eRESULT halMCU_TEC_TESTER_En_Set(UINT8 cData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemSet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_TEC_TESTER_ENABLE, sizeof(UINT8), &cData);

    return eResult;
}
//G100_Steven_0080
#endif

eRESULT halMCUCtrl_Standby_Error_Get(UINT8* pcData) //G100_Julie_0022
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemGet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_STANDBY_ERROR_FLAG, eSYSTEM_MSG_STANDBY_ERROR_FLAG_SZ, pcData);

    return eResult;
}

eRESULT halMCUCtrl_OPD_Register_Set(UINT8 reg, UINT8 size, UINT8 *pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemSet(eCMD_MODULE_OPD, reg, size, pcData);

    return eResult;
}

eRESULT halMCUCtrl_OPD_Register_Get(UINT8 reg, UINT8 size, UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemGet(eCMD_MODULE_OPD, reg, size, pcData);

    return eResult;
}

eRESULT halMCU_LAN_Path_Set(UINT8 ucData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemSet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_LAN_PATH, eSYSTEM_MSG_LAN_PATH_SZ, &ucData);

    return eResult;
}

eRESULT halMCU_UART_ECHO_Set(UINT8 ucData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemSet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_UART_ECHO, eSYSTEM_MSG_LAN_PATH_SZ, &ucData);

    return eResult;
}

eRESULT halMCU_LAN_IP_INFO_Set(UINT8 *pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemSet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_LAN_IP_INFO, eSYSTEM_MSG_LAN_IP_INFO_SZ, pcData);

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halMCU_ProjectorID_Set
// DESCRIPTION:
//
//
// Params:
//
// Returns:
//
//
// Modification History
// --------------------
// 2021/11/04, Larry Create
// --------------------
// ==============================================================================
eRESULT halMCU_ProjectorID_Set(UINT8 *pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemSet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_PROJECTOR_ID, 4, pcData);

    if(eResult == rcSUCCESS)
    {
        MS_SLEEP(50);
        //等待系統設定完成
    }

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halMCU_CustomerID_Set
// DESCRIPTION:
//
//
// Params:
//
// Returns:
//
//
// Modification History
// --------------------
// 2021/11/04, Larry Create
// --------------------
// ==============================================================================
eRESULT halMCU_CustomerID_Set(UINT8 cData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemSet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_CUSTOMER_ID, 1, &cData);

    if(eResult == rcSUCCESS)
    {
        MS_SLEEP(50);
        //等待系統設定完成
    }

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halMCU_PlatformID_Set
// DESCRIPTION:
//
//
// Params:
//
// Returns:
//
//
// Modification History
// --------------------
// 2021/11/04, Larry Create
// --------------------
// ==============================================================================
eRESULT halMCU_PlatformID_Set(UINT8 cData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemSet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_PLATFORM_ID, 1, &cData);

    if(eResult == rcSUCCESS)
    {
        MS_SLEEP(50);
        //等待系統設定完成
    }

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halMCUCtrl_AC_InputGet
// DESCRIPTION:
//
//
// Params:
//
// Returns:
//
//
// Modification History
// --------------------
// 2022/09/20, Larry Create
// --------------------
// ==============================================================================
eRESULT halMCUCtrl_AC_InputGet(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemGet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_AC_INPUT_DET, 1, pcData);

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halMCU_SerialNumberSet
// DESCRIPTION:
//
//
// Params:
//
// Returns:
//
//
// Modification History
// --------------------
// 2022/09/22, Larry Create
// --------------------
// ==============================================================================
eRESULT halMCU_SerialNumberSet(UINT8 *pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemSet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_SNID, eSYSTEM_MSG_SNID_SZ, pcData);

    return eResult;
}

eRESULT halMCU_PIN_Protect_Set(UINT8 pcData) //R70G2_Optoma_AC_0017//H30K_David_0038
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemSet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_PIN_PROTECT, eSYSTEM_MSG_PIN_PROTECT_SZ, &pcData);

    return eResult;
}

eRESULT halMCU_PIN_Code_Set(UINT8* pcData) //R70G2_Optoma_AC_0017//H30K_David_0038
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemSet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_PIN_CODE, eSYSTEM_MSG_PIN_CODE_SZ, pcData);

    return eResult;
}

eRESULT halMCUCtrl_PIN_Power_On_Get(UINT8* pcData) //R70G2_Optoma_AC_0017//H30K_David_0038
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemGet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_PIN_POWER_ON, eSYSTEM_MSG_PIN_POWER_ON_SZ, pcData);


    return eResult;
}

void halMCUCtrl_Bist(void *pvData)  //H30K_Steven_0001
{
    eRESULT lResult = rcSUCCESS;
    UINT8 aucData[4];
    sBIST_DEV *psDev = (sBIST_DEV*)pvData;

    psDev->lValid = 1;
    //psDev->lLayer--;

    lResult = utilHost_SystemGet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_SYSTEM_VERSION, eSYSTEM_MSG_SYSTEM_VERSION_SZ, aucData);

    psDev->lCommuError = 0;

    if(lResult == rcSUCCESS)
    {
        psDev->lError      = 0;
        psDev->lData       = I2C_SUCCESS_VALUE;
    }
    else
    {
        psDev->lError      = 1;
        psDev->lData       = I2C_ERROR_VALUE;
    }
}


