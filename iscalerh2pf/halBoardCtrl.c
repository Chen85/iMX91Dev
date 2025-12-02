// ==============================================================================
// FILE NAME: HALBOARDCTRL.C
// DESCRIPTION:
//
//
// modification history
// --------------------
// 19/12/2013, Leo Create
// --------------------
// ==============================================================================

#include "Board_Uart.h"
//#include "Board_WatchDog.h"
#include "halBoardCtrlAPI.h"
#include "utilDbgMsg.h"
#include "utilCounterAPI.h"
#include "utilCommon.h"
#include "CommonDef.h"
#include "dvFPGA10M04.h" //A70LV_Larry_0096
#include "dvXillinxFPGA.h"         //A70LV_Doulas_0216
#include "dvLpcIOExp.h"
#include "gpio.h"

#include "utilBIST.h"

// ==============================================================================
// FUNCTION NAME: halBoard_IR_Enable_Set
// DESCRIPTION:
//
//
// Params:
// eIR_ENABLE eIRPin:
// BYTE cEnable:
//
// Returns:
//
//
// Modification History
// --------------------
// 2015/10/23, Larry Create
// A70LH_Larry_0068 Fixed
// --------------------
// ==============================================================================
void halBoard_IR_Enable_Set(eIR_ENABLE eIRPin, UINT8 ucEnable)
{
#if 0
    switch(eIRPin)
    {
        case eIR_ENABLE_T:
//            Board_Gpio_Set(pIR_T_EN,  ucEnable);
            break;

        case eIR_ENABLE_F:
            Board_Gpio_Set(pIR_F_EN,  ucEnable);
            break;

        case eIR_ENABLE_HD:
//            Board_Gpio_Set(pIR_HD_EN, ucEnable);
            break;

        case eIR_ENABLE_ALL:
//            Board_Gpio_Set(pIR_T_EN,  ucEnable);
            Board_Gpio_Set(pIR_F_EN,  ucEnable);
//            Board_Gpio_Set(pIR_HD_EN, ucEnable);
            break;

        default:
            break;
    }
#endif /* 0 */
}

// ==============================================================================
// FUNCTION NAME: halBoard_R12_Trigger_Set
// DESCRIPTION:
//
//
// Params:
// BYTE cEnable:
//
// Returns:
//
//
// Modification History
// --------------------
// 2015/09/17, Larry Create
// --------------------
// ==============================================================================
void halBoard_R12_Trigger_Set(UINT8 ucEnable)
{
    //Board_Gpio_Set(pR12_EN, ucEnable);
}

// ==============================================================================
// FUNCTION NAME: halBoard_Baudrate_Set
// DESCRIPTION:
//
//
// Params:
// UINT8 ucEnable:
//
// Returns:
//
//
// Modification History
// --------------------
// 2018/01/04, Larry Create
// --------------------
// ==============================================================================
eRESULT halBoard_Baudrate_Set(UINT32 ulBaudrate)
{
    INT8 Ret;
    Ret = Board_Uart_Baudrate_Change(uaPCUART, ulBaudrate);

    if(Ret != UART_OK)
        return rcERROR;

    return rcSUCCESS;
}


// ==============================================================================
// FUNCTION NAME: halBoard_DDP_Upgrade
// DESCRIPTION:
//
//
// Params:
// void:
//
// Returns:
//
//
// Modification History
// --------------------
// 2018/01/26, Larry Create
// --------------------
// ==============================================================================
void halBoard_DDP_Upgrade(void)
{


}

// ==============================================================================
// FUNCTION NAME: halBoard_Uart_SW
// DESCRIPTION:
//
//
// Params:
// UINT8 cValue:
//
// Returns:
//
//
// Modification History
// --------------------
// 2018/01/31, Larry Create
// --------------------
// ==============================================================================
void halBoard_Uart_SW_Set(UINT8 cValue)
{
}


// ==============================================================================
// FUNCTION NAME: halBoard_ModuleTypeGet
// DESCRIPTION:
//
//
// Params:
// void:
//
// Returns:
//
//
// Modification History
// --------------------
// 2018/03/19, Larry Create
// --------------------
// ==============================================================================
UINT8 halBoard_CustomIDGet(void)
{
    //ID 0: H: A70LV/ L:A70LS

    //ID 1/ID 2:  H   |   L
    //     H   :  WU  |   NA
    //     L   :  HD  |   NA


    return 1;

}

// ==============================================================================
// FUNCTION NAME: halBoard_ActPwrOff
// DESCRIPTION:
//
//
// Params:
// void:
//
// Returns:
//
//
// Modification History
// --------------------
// 2018/05/16, Larry Create
// --------------------
// ==============================================================================
void halBoard_ActPwrOff(void)
{

    return;
}

// ==============================================================================
// FUNCTION NAME: halBoard_XillinxFPGA_Reset
// DESCRIPTION:
//
//
// Params:
//
//
// Returns:
//
//
// Modification History
// --------------------
// 2018/05/28, A70LV_Doulas_0179
// --------------------
// ==============================================================================
void halBoard_XillinxFPGA_Reset(void)
{
    dvFPGA_XillinxFPGA_Reset();
    //LOG_MSG(db_ALWAYS, "FPGA_Reset \r\n");
}

// ==============================================================================
// FUNCTION NAME: halBoard_PowerCheckGet
// DESCRIPTION:
//
//
// Params:
// UINT8 *pcData:
//
// Returns:
//
//
// Modification History
// --------------------
// 2018/05/29, Larry Create
// --------------------
// ==============================================================================
void halBoard_PowerCheckGet(UINT8 *pcData)
{
    UINT8 acData[3] = {0};

    dvFPGA_System_Read(eFPGA_1_REG_COM_CHECK, &acData[0]);
    dvFPGA_System_Read(eFPGA_1_REG_ACT_POWER_CHECK_1, &acData[1]);
    dvFPGA_System_Read(eFPGA_1_REG_ACT_POWER_CHECK_2, &acData[2]);

    memcpy(pcData, acData, 3);
}

// ==============================================================================
// FUNCTION NAME: halBoard_XillinxFPGA_Manual_Mode_Set
// DESCRIPTION:
//
//
// Params:
//
//
// Returns:
//
//
// Modification History
// --------------------
// 2018/06/22, A70LV_Doulas_0216
// --------------------
// ==============================================================================
void halBoard_XillinxFPGA_Manual_Mode_Set(UINT8 ucPanel)
{
	#ifndef XFPGA_NEW_AUTO_MODE_V3_3	//G100_Doulas_0001
    UINT8 ucData;
    //dvFPGA_XillinxFPGA_Reset11ms();
    //LOG_MSG(db_ALWAYS, "halBoard_XillinxFPGA_Manual_Mode_Set \r\n");  //A70LV_Doulas_0218
    //MS_SLEEP(100);
    ucData = 0;
    //bXFPAG_SW_RESET_FLAG = TRUE;    //A70LV_Doulas_0262
    dvXillinxFPGARegWrite(eXFPGA_CMD_LOCK_REGS,1,&ucData);

    ucData = 0;
    dvXillinxFPGARegWrite(eXFPGA_CMD_AUTO_DET,1,&ucData);

    if(ucPanel == ePANEL_ID_1080P_120HZ)
    {
        ucData = 0;
    }
    else
    {
        ucData = 1;
    }
    dvXillinxFPGARegWrite(eXFPGA_CMD_DRP_SADDR,1,&ucData);

    ucData = 1;
    dvXillinxFPGARegWrite(eXFPGA_CMD_LVDS_RST,1,&ucData);

    MS_SLEEP(1);

    ucData = 0;
    dvXillinxFPGARegWrite(eXFPGA_CMD_LVDS_RST,1,&ucData);

    ucData = 1;
    dvXillinxFPGARegWrite(eXFPGA_CMD_LOCK_REGS,1,&ucData);
    MS_SLEEP(10);
    //bXFPAG_SW_RESET_FLAG = FALSE;    //A70LV_Doulas_0262
    //LOG_MSG(db_ALWAYS, "FPGA_Reset \r\n");
    #endif
}

void halBoard_XillinxFPGA_Init_Set(UINT8 ucPanel, UINT8 ucIsNewVersion)
{
    //LOG_MSG(db_ALWAYS, "halBoard_XillinxFPGA_Init_Set \r\n");  //A70LV_Doulas_0218
    dvFPGA_XillinxFPGA_Reset11ms();
    MS_SLEEP(1);                                         //A70LV_Doulas_0218
    if(ucIsNewVersion)        //A70LV_Doulas_0218
    {
	#ifdef XFPGA_NEW_AUTO_MODE_V3_3	//G100_Doulas_0001
		UINT8 ucData;

		ucData = 7;
		halBoard_XillinxFPGA_Write(eXFPGA_CMD_LVDS_VID_MAP_SEL,&ucData);

		ucData = 0;
		halBoard_XillinxFPGA_Write(eXFPGA_CMD_DDP_VID_MAP_SWAP,&ucData);

		ucData = 1;
		halBoard_XillinxFPGA_Write(eXFPGA_CMD_DRP_SADDR,&ucData);

		ucData = 1;
		halBoard_XillinxFPGA_Write(eXFPGA_CMD_AUTO_DET,&ucData);

		ucData = 0;
		halBoard_XillinxFPGA_Write(eXFPGA_CMD_PHASE_UNLOCK,&ucData);

		ucData = 0;
		halBoard_XillinxFPGA_Write(eXFPGA_CMD_LVDS_COVER,&ucData);


		ucData = 2;
		halBoard_XillinxFPGA_Write(eXFPGA_CMD_LVDS_TOL_VAL,&ucData);

		ucData = 10;//3;
		halBoard_XillinxFPGA_Write(eXFPGA_CMD_LVDS_LOCK_VAL,&ucData);

		ucData = 2;
		halBoard_XillinxFPGA_Write(eXFPGA_CMD_LVDS_ERR_MULT,&ucData);

		ucData = 0x0f;
		halBoard_XillinxFPGA_Write(eXFPGA_CMD_LVDS_25,&ucData);


		ucData = 1;
		dvXillinxFPGARegWrite(eXFPGA_CMD_LVDS_RST,1,&ucData);
		MS_SLEEP(1);
		ucData = 0;
		dvXillinxFPGARegWrite(eXFPGA_CMD_LVDS_RST,1,&ucData);
	#endif
        halBoard_XillinxFPGA_Manual_Mode_Set(ucPanel);
    }
}

eRESULT halBoard_XillinxFPGA_Read(UINT8 ucReg,UINT8 *ucDataVal)
{
    eRESULT eResult = rcERROR;
    eResult = dvXillinxFPGARegRead(ucReg,1,ucDataVal);
    return eResult;
}

eRESULT halBoard_XillinxFPGA_Write(UINT8 ucReg,UINT8 *ucDataVal)
{
    UINT8 ucData;
    eRESULT eResult = rcERROR;

    ucData = 0;
    dvXillinxFPGARegWrite(eXFPGA_CMD_LOCK_REGS,1,&ucData);

    eResult = dvXillinxFPGARegWrite(ucReg,1,ucDataVal);

 //   ucData = 1;
 //   dvXillinxFPGARegWrite(eXFPGA_CMD_LOCK_REGS,1,&ucData);    //A70LV_Doulas_0262 remove
    return eResult;
}

#if 0
eRESULT halBoard_XillinxFPGA_VersionRead(UINT8* ucBLV,UINT8* ucBHV)        //A70LV_Doulas_0218
{
    UINT8 ucBL_VER = 0xff;
    UINT8 ucBH_VER = 0xff;
    eRESULT eResult = rcERROR;
    eRESULT eResult1 = rcERROR;
    eRESULT eResult2 = rcERROR;


    if((m_ucBL_VER == 0xff) &&
       (m_ucBH_VER == 0xff))
    {

        eResult1 = halBoard_XillinxFPGA_Read(eXFPGA_CMD_BL_VER,&ucBL_VER);
        eResult2 = halBoard_XillinxFPGA_Read(eXFPGA_CMD_BH_VER,&ucBH_VER);
        if((eResult1 == rcSUCCESS ) &&(eResult2 == rcSUCCESS))
        {
            if((ucBL_VER != 0xff) && (ucBH_VER != 0xff))
            {
                m_ucBL_VER = ucBL_VER;
                m_ucBH_VER = ucBH_VER;
                *ucBLV = m_ucBL_VER;
                *ucBHV = m_ucBH_VER;
                eResult = rcSUCCESS;
            }
        }
    }
    else
    {
        *ucBLV = m_ucBL_VER;
        *ucBHV = m_ucBH_VER;
        eResult = rcSUCCESS;
    }

    return eResult;
}
#endif

#if 0
BOOL halBoard_XillinxFPGA_NewVersionGet(void)        //A70LV_Doulas_0218
{
    UINT8 ucBL_VER = 0xff;
    UINT8 ucBH_VER = 0xff;
    eRESULT eResult = rcERROR;
    UINT8 ucVal = 0;

    for(; ucVal<3 ; ucVal++)
    {
        eResult  = halBoard_XillinxFPGA_Version_BL_Get(&ucBL_VER);
        eResult &= halBoard_XillinxFPGA_Version_BH_Get(&ucBH_VER);

        if(eResult == rcSUCCESS)
        {
            if((ucBH_VER == 0) &&(ucBL_VER < 3))
            {
                //LOG_MSG(db_ALWAYS, "halBoard_XillinxFPGA_NewVersionGet OLD1 \r\n");
                return FALSE;
            }
            else
            {
                //LOG_MSG(db_ALWAYS, "halBoard_XillinxFPGA_NewVersionGet NEW \r\n");
                return TRUE;
            }
        }
        eResult = rcERROR;
    }
    //LOG_MSG(db_ALWAYS, "halBoard_XillinxFPGA_NewVersionGet OLD2 \r\n");
    return FALSE;
}
#endif

BOOL halBoard_XillinxFPGA_SW_Reset_Flag_Get(void)     //A70LV_Doulas_0262
{
    //return bXFPAG_SW_RESET_FLAG;
    return FALSE;
}

BOOL halBoard_XillinxFPGA_Version_BL_Get(UINT8 *ucData)  //A35G2_Simon_0065
{
    eRESULT eResult = rcERROR;
    eResult = halBoard_XillinxFPGA_Read(eXFPGA_CMD_BL_VER, ucData);
    return eResult;
}

BOOL halBoard_XillinxFPGA_Version_BH_Get(UINT8 *ucData)  //A35G2_Simon_0065
{
    eRESULT eResult = rcERROR;
    eResult = halBoard_XillinxFPGA_Read(eXFPGA_CMD_BH_VER, ucData);
    return eResult;
}

BOOL halBoard_XillinxFPGA_DRP_Status_Get(UINT8 *ucData)  //A35G2_Simon_0065
{
    eRESULT eResult = rcERROR;
    eResult = halBoard_XillinxFPGA_Read(eXFPGA_CMD_DRP_STATUS, ucData);
    return eResult;
}

BOOL halBoard_XillinxFPGA_PhaseUnlock_Get(UINT8 *ucData)  //A35G2_Simon_0065
{
    eRESULT eResult = rcERROR;
    eResult = halBoard_XillinxFPGA_Read(eXFPGA_CMD_PHASE_UNLOCK, ucData);
    return eResult;
}

BOOL halBoard_XillinxFPGA_LDVS_Cover_Get(UINT8 *ucData)  //A35G2_Simon_0065
{
    eRESULT eResult = rcERROR;
    eResult = halBoard_XillinxFPGA_Read(eXFPGA_CMD_LVDS_COVER, ucData);
    return eResult;
}


// ==============================================================================
// FUNCTION NAME: halBoard_LD_50V_Detection
// DESCRIPTION:
//
//
// Params:
// void:
//
// Returns:
//
//
// Modification History
// --------------------
// 2015/10/22, Larry Create
// --------------------
// ==============================================================================
BOOL halBoard_LD_50V_Detection(void)
{
    return 0;
}

// ==============================================================================
// FUNCTION NAME: halBoard_LVPS_Detection
// DESCRIPTION:
//
//
// Params:
// void:
//
// Returns:
//
//
// Modification History
// --------------------
// 2015/10/22, Larry Create
// --------------------
// ==============================================================================
BOOL halBoard_LVPS_Detection(void)                          //A35LH_JS_0052 modify
{
    return 0;
}

// ==============================================================================
// FUNCTION NAME: halBoard_LAN_Ready_Set
// DESCRIPTION:
//
//
// Params:
// UINT8 ucValue:
//
// Returns:
//
//
// Modification History
// --------------------
// 2018/11/06, Larry Create
// --------------------
// ==============================================================================
void halBoard_LAN_Reset_Set(UINT8 ucValue)
{

}

// ==============================================================================
// FUNCTION NAME: halBoard_LAN_Ready_Set
// DESCRIPTION:
//
//
// Params:
// UINT8 ucValue:
//
// Returns:
//
//
// Modification History
// --------------------
// 2018/11/06, Larry Create
// --------------------
// ==============================================================================
BOOL halBoard_LAN_Ready_Get(void)
{
    return 1;
}

// ==============================================================================
// FUNCTION NAME: halBoard_XilinxFPGA_Upgrade
// DESCRIPTION:
//
//
// Params:
// void:
//
// Returns:
//
//
// Modification History
// --------------------
// 2018/09/07, Larry Create
// --------------------
// ==============================================================================
void halBoard_FPGA_Reset_Set(UINT8 ucValue)
{

}

	//ZU860_Clare_0027, >>>
// ==============================================================================
// FUNCTION NAME: halBoard_STB_Power_Off
// DESCRIPTION:
//
//
// Params:
// UINT8 ucValue:
//
// Returns:
//
//
// Modification History
// --------------------
// 2019/01/24, Clare Create
// --------------------
// ==============================================================================
void halBoard_STB_Power_Off(UINT8 ucValue)
{
    dvLPCIOExp_MCU_Power_Off(ucValue);	//ZU860_Doulas_0001
}
	//ZU860_Clare_0027, <<<
void halBoard_STB_Burn_In_Enable(UINT8 cEnable)
{
    dvLPCIOExp_Burn_In_Enable(cEnable);
}
void halBoard_STB_Burn_In_OffTime_Set(UINT16 wTime)
{
    dvLPCIOExp_Burn_In_OffTime_Set(wTime);
}
	//ZU860_Clare_0039, add, >>>
void halBoard_STB_PwrMode_States_Set(UINT8 ucValue)
{
    dvLPCIOExp_Pwr_Mode_States_Set(ucValue);
}
void halBoard_STB_OPFU_Reboot_Set(UINT8 ucValue)	//ZU860_Clare_0043
{
    dvLPCIOExp_OPFU_Reboot_Set(ucValue);
}

void halBoard_STB_Power_State_Get(UINT8 *pucValue)	//ZU860_Clare_0043
{
    dvLPCIOExp_Power_State_Get(pucValue);
}

// ==============================================================================
// FUNCTION NAME: halBoard_PowerOn
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
// 2020/06/17, Larry Create
// --------------------
// ==============================================================================
void halBoard_PowerOn(void)
{
    GPIO_Write(GPIO_LAN_POWER_ON, GPIO_HIGH);
    MS_SLEEP(5000); // need to keep low 3 sec
    GPIO_Write(GPIO_LAN_POWER_ON, GPIO_LOW);
}

// ==============================================================================
// FUNCTION NAME: halBoard_PowerReset
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
// 2020/06/17, Larry Create
// --------------------
// ==============================================================================
void halBoard_PowerKeyReset(void)
{
    //printf("%s, %d GPIO %d\n",__FUNCTION__, __LINE__, GPIO_LAN_POWER_ON);
    //default value LOW

    GPIO_Open(GPIO_LAN_POWER_ON, GPIO_OUT);
    GPIO_Write(GPIO_LAN_POWER_ON, GPIO_LOW);
}

// ==============================================================================
// FUNCTION NAME: halBoard_CameraEnableInit
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
// 2021/02/02, Larry Create
// --------------------
// ==============================================================================
void halBoard_CameraEnableInit(void)
{
    GPIO_Open(GPIO7_IO12_GPIO_17, GPIO_OUT);
    GPIO_Write(GPIO7_IO12_GPIO_17, GPIO_LOW);
}

// ==============================================================================
// FUNCTION NAME: halBoard_CameraEnableSet
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
// 2021/02/02, Larry Create
// --------------------
// ==============================================================================
void halBoard_CameraEnableSet(UINT8 ucValue)
{
    if(ucValue)
    {
        GPIO_Write(GPIO7_IO12_GPIO_17, GPIO_HIGH);
    }
    else
    {
        GPIO_Write(GPIO7_IO12_GPIO_17, GPIO_LOW);
    }
}

void halBoardXFPGA_Bist(void *pvData) //H30K_Steven_0001
{
	UINT8 aucVersion[8] = {0};
	eRESULT lResult1 = rcINVALID, lResult2 = rcINVALID;;

	//XFPGA Version
	lResult1  = halBoard_XillinxFPGA_Version_BL_Get(&aucVersion[0]);
	lResult2  = halBoard_XillinxFPGA_Version_BH_Get(&aucVersion[1]);

	sBIST_DEV *psDev = (sBIST_DEV*)pvData;
	psDev->lValid = 1;

	psDev->lCommuError = 0;

	if(lResult1 == rcSUCCESS && lResult2 == rcSUCCESS)
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

