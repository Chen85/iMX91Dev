// ==============================================================================
// FILE NAME: DVLPCIOEXP.C
// DESCRIPTION:
//
//
// modification history
// --------------------
// 2014/03/24, Leo Create
// --------------------
// ==============================================================================

#include "Board_I2C.h"
#include "Board_I2C_Dev_Table.h"
#include "dvLpcIOExp.h"
#include "utilDbgMsg.h"

//static BYTE m_acIO_Exp_Dir[eLPCIOEXP_PORT_NUMBERS];
//static BYTE m_acIO_Pin_Data[eLPCIOEXP_PORT_NUMBERS];

// ==============================================================================
// FUNCTION NAME: DVLPCIOEXP_REG_SET
// DESCRIPTION:
//
//
// Params:
// BYTE cReg:
// WORD wSize:
// BYTE *pcData:
//
// Returns:
//
//
// modification history
// --------------------
// 2014/03/24, Leo Create
// --------------------
// ==============================================================================
static eRESULT dvLPCIOExp_Reg_Set(UINT8 cReg, UINT16 wSize, UINT8 *pcData)
{
    eRESULT eResult = rcSUCCESS;
    UINT8    cRetry = 0;

#if 0//(SCALER_BOARD_STAGE > MP_STAGE)
    ASSERT(cReg < eIOEXP_CMD_NUMBERS);

    do
    {
        eResult = Board_I2C_Master_Write(LPC_IOEXP_I2C_BUS,
                                         LPC_IOEXP_I2C_ADDRESS,
                                         cReg,
                                         wSize,
                                         pcData,
                                         LPC_IOEXP_I2C_FLAG);
    }
    while((cRetry++ < LPC_IOEXP_I2C_RETRY) && (eResult != rcSUCCESS));

    ASSERT(eResult == rcSUCCESS);
#endif /* __ICHIP_CONTROL__ */

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: DVLPCIOEXP_REG_GET
// DESCRIPTION:
//
//
// Params:
// BYTE cReg:
// WORD wSize:
// BYTE *pcData:
//
// Returns:
//
//
// modification history
// --------------------
// 2014/03/24, Leo Create
// --------------------
// ==============================================================================
static eRESULT dvLPCIOExp_Reg_Get(UINT8 cReg, UINT16 wSize, UINT8 *pcData)
{
    eRESULT eResult = rcSUCCESS;
    UINT8    cRetry = 0;

#if 0//(SCALER_BOARD_STAGE > MP_STAGE)
    ASSERT(cReg < eIOEXP_CMD_NUMBERS);

    do
    {
        eResult = Board_I2C_Master_Read(LPC_IOEXP_I2C_BUS,
                                        LPC_IOEXP_I2C_ADDRESS,
                                        cReg,
                                        1,
                                        pcData,
                                        LPC_IOEXP_I2C_FLAG);
    }
    while((cRetry++ < LPC_IOEXP_I2C_RETRY) && (eResult != rcSUCCESS));

    ASSERT(eResult == rcSUCCESS);
#endif /* __ICHIP_CONTROL__ */

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: DVLPCIOEXP_INIT
// DESCRIPTION:
//
//
// Params:
// void:
//
// Returns:
//
//
// modification history
// --------------------
// 2014/03/24, Leo Create
// --------------------
// ==============================================================================
void dvLPCIOExp_Init(void)
{
#if 0
    BYTE cPort = 0;

    for(cPort = 0; cPort < eLPCIOEXP_PORT_NUMBERS ; cPort++)
    {
        m_acIO_Exp_Dir[cPort] = 0xFF;
        m_acIO_Pin_Data[cPort] = 0xFF;

        if(eLPCIOEXP_PORT_1 == cPort)
        {
            m_acIO_Pin_Data[cPort] = 0x5F; //For shutter
        }

        dvLPCIOExp_PORT_Dir_Set((eLPCIOEXP_PORT)cPort, m_acIO_Exp_Dir[cPort]);
        dvLPCIOExp_PORT_Set((eLPCIOEXP_PORT)cPort, m_acIO_Pin_Data[cPort]);
    }
#endif /* 0 */

}

// ==============================================================================
// FUNCTION NAME: DVLPCIOEXP_PORT_DIR_SET
// DESCRIPTION:
//
//
// Params:
// eLPCIOEXP_PORT ePort:
// BYTE cDir:
//
// Returns:
//
//
// modification history
// --------------------
// 2014/03/24, Leo Create
// --------------------
// ==============================================================================
void dvLPCIOExp_PORT_Dir_Set(eLPCIOEXP_PORT ePort, UINT8 cDir)
{
    eRESULT eResult = dvLPCIOExp_Reg_Set(eIOEXP_CMD_PORT0_IO_DIR + ePort, 1, &cDir);
}

// ==============================================================================
// FUNCTION NAME: DVLPCIOEXP_PORT_DIR_GET
// DESCRIPTION:
//
//
// Params:
// eLPCIOEXP_PORT ePort:
//
// Returns:
//
//
// modification history
// --------------------
// 2014/03/24, Leo Create
// --------------------
// ==============================================================================
UINT8 dvLPCIOExp_PORT_Dir_Get(eLPCIOEXP_PORT ePort)
{
    UINT8 cData = 0;
    eRESULT eResult = dvLPCIOExp_Reg_Get(eIOEXP_CMD_PORT0_IO_DIR + ePort, 1, &cData);

    return cData;
}

// ==============================================================================
// FUNCTION NAME: DVLPCIOEXP_PORT_SET
// DESCRIPTION:
//
//
// Params:
// eLPCIOEXP_PORT ePort:
// BYTE cData:
//
// Returns:
//
//
// modification history
// --------------------
// 2014/03/24, Leo Create
// --------------------
// ==============================================================================
void dvLPCIOExp_PORT_Set(eLPCIOEXP_PORT ePort, UINT8 cData)
{
    eRESULT eResult = dvLPCIOExp_Reg_Set(eIOEXP_CMD_PORT0_IO_STATE + ePort, 1, &cData);
}

// ==============================================================================
// FUNCTION NAME: DVLPCIOEXP_PORT_GET
// DESCRIPTION:
//
//
// Params:
// eLPCIOEXP_PORT ePort:
//
// Returns:
//
//
// modification history
// --------------------
// 2014/03/24, Leo Create
// --------------------
// ==============================================================================
UINT8 dvLPCIOExp_PORT_Get(eLPCIOEXP_PORT ePort)
{
    UINT8 cData = 0;
    eRESULT eResult = dvLPCIOExp_Reg_Get(eIOEXP_CMD_PORT0_IO_STATE + ePort, 1, &cData);

    return cData;
}

//ZU860_Clare_0008, add, >>>
eRESULT dvLPCIOExp_MCU_Power_Off(UINT8 cData)
{
    eRESULT eResult = dvLPCIOExp_Reg_Set(eSTB_CMD_MCU_POWER_OFF, 1, (UINT8 *)&cData);
    //LOG_MSG(db_ALWAYS, "dvLPCIOExp_MCU_Power_Off\r\n");
    return eResult;
}
eRESULT dvLPCIOExp_Burn_In_Enable(UINT8 cEnable)
{
    eRESULT eResult = dvLPCIOExp_Reg_Set(eSTB_CMD_BURN_IN_ENABLE, 1, (UINT8 *)&cEnable);
    return eResult;
}
eRESULT dvLPCIOExp_Burn_In_OffTime_Set(UINT16 wTime)
{
    eRESULT eResult = dvLPCIOExp_Reg_Set(eSTB_CMD_BURN_IN_OFF_TIME, 2, (UINT8 *)&wTime);
    return eResult;
}
//ZU860_Clare_0008, add, <<<
	//ZU860_Clare_0039, add, >>>
eRESULT dvLPCIOExp_Pwr_Mode_States_Set(UINT8 cData)
{
    eRESULT eResult = dvLPCIOExp_Reg_Set(eSTB_CMD_PWR_MODE_STATES, 1, (UINT8 *)&cData);
    return eResult;
}
eRESULT dvLPCIOExp_Signal_PowerOn_Enable_Set(UINT8 cData)
{
    eRESULT eResult = dvLPCIOExp_Reg_Set(eSTB_CMD_SIGNAL_PWR_ON_ENABLE, 1, (UINT8 *)&cData);
    return eResult;
}
eRESULT dvLPCIOExp_IR_Top_Set(UINT8 cData)
{
    eRESULT eResult = dvLPCIOExp_Reg_Set(eSTB_CMD_IR_TOP_ENABLE, 1, (UINT8 *)&cData);
    return eResult;
}
eRESULT dvLPCIOExp_IR_Front_Set(UINT8 cData)
{
    eRESULT eResult = dvLPCIOExp_Reg_Set(eSTB_CMD_IR_FRONT_ENABLE, 1, (UINT8 *)&cData);
    return eResult;
}
eRESULT dvLPCIOExp_IR_HDBaseT_Set(UINT8 cData)
{
    eRESULT eResult = dvLPCIOExp_Reg_Set(eSTB_CMD_IR_HDBASET_ENABLE, 1, (UINT8 *)&cData);
    return eResult;
}
eRESULT dvLPCIOExp_AC_Power_On_Set(UINT8 cData)
{
    eRESULT eResult = dvLPCIOExp_Reg_Set(eSTB_CMD_AC_POWERON_ENABLE, 1, (UINT8 *)&cData);
    return eResult;
}
eRESULT dvLPCIOExp_Serial_Port_Baud_Rate_Set(UINT8 cData)	//ZU860_Clare_0042
{
    eRESULT eResult = dvLPCIOExp_Reg_Set(eSTB_CMD_SERIAL_PORT_BAUD_RATE, 1, (UINT8 *)&cData);
    return eResult;
}
eRESULT dvLPCIOExp_Projector_Address_Set(UINT16 wData)	//ZU860_Clare_0042
{
    eRESULT eResult = dvLPCIOExp_Reg_Set(eSTB_CMD_PROJECTOR_ADDRESS, 2, (UINT8 *)&wData);
    return eResult;
}
eRESULT dvLPCIOExp_OPFU_Reboot_Set(UINT8 cData)	//ZU860_Clare_0043
{
    eRESULT eResult = dvLPCIOExp_Reg_Set(eSTB_CMD_OPFU_REBOOT, 1, (UINT8 *)&cData);
    return eResult;
}
eRESULT dvLPCIOExp_Keypad_LED_Enable(UINT8 cEnable)	//ZU860_Clare_0063
{
    eRESULT eResult = dvLPCIOExp_Reg_Set(eSTB_CMD_KEYPAD_LED_ENABLE, 1, (UINT8 *)&cEnable);
    return eResult;
}
	//ZU860_Clare_0039, add, <<<

eRESULT dvLPCIOExp_Shutter_LED_Set(UINT8 cData)  //EK816U_626U_Owen_0011
{
    eRESULT eResult = dvLPCIOExp_Reg_Set(eSTB_CMD_SHUTTER_LED, 1, (UINT8 *)&cData);
    return eResult;
}
    
eRESULT dvLPCIOExp_LAN_Reset_Set(UINT8 cData) //SNPLU9000_Energy_0025
{
    eRESULT eResult = dvLPCIOExp_Reg_Set(eSTB_CMD_RESET_LAN, 1, (UINT8 *)&cData);
    return eResult;
}

eRESULT dvLPCIOExp_Power_State_Get(UINT8 *pcData)
{
    eRESULT eResult = dvLPCIOExp_Reg_Get(eSTB_CMD_PWR_STATE, 1, pcData);
    return eResult;
}

// ==============================================================================
// FUNCTION NAME: DVLPCIOEXP_VERSION_GET
// DESCRIPTION:
//
//
// Params:
// BYTE *pcVersion:
//
// Returns:
//
//
// modification history
// --------------------
// 2014/04/15, Leo Create
// --------------------
// ==============================================================================
eRESULT dvLPCIOExp_Version_Get(UINT8 *pcVersion)
{
    eRESULT eResult = rcSUCCESS;
    UINT8    cRetry = 0;

#if 0
    do
    {
        eResult = Board_I2C_Master_Read(LPC_IOEXP_I2C_BUS,
                                        LPC_IOEXP_I2C_ADDRESS,
                                        eIOEXP_CMD_VERSION,
                                        2,
                                        pcVersion,
                                        LPC_IOEXP_I2C_FLAG);
    }
    while((cRetry++ < LPC_IOEXP_I2C_RETRY) && (eResult != rcSUCCESS));
#endif /* 0 */

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: DVLPCIOEXP_STATUSLED_SET
// DESCRIPTION:
//
//
// Params:
// eLED_BEHAVIOR eBehavior:
// eLED_STATUS_STATE eStatus:
//
// Returns:
//
//
// modification history
// --------------------
// 2014/05/06, Leo Create
// --------------------
// ==============================================================================
eRESULT dvLPCIOExp_StatusLed_Set(eLED_LIGHT eLight, eLED_STATUS_LIGHT eStatus)
{
    UINT16 wStatus = (UINT16)((eLight << 8) | (eStatus));
    eRESULT eResult = rcSUCCESS;

    //dvLPCIOExp_Reg_Set(eIOEXP_CMD_STATUS_LED, 2, (BYTE *)&wStatus);

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: dvLPCIOExp_SENSOR_Get
// DESCRIPTION:
//
//
// Params:
// BYTE *pcVersion:
//
// Returns:
//
//
// Modification History
// --------------------
// 2014/12/09, Larry Create
// A70LH_Larry_0170 Fixed
// --------------------
// ==============================================================================
INT16 dvLPCIOExp_Sensor_Get(void)
{
    eRESULT eResult = rcSUCCESS;
    UINT8    cValue  = 127;

    //eResult = dvLPCIOExp_Reg_Get(eIOEXP_CMD_KEYPAD_SENSOR, 1, &cValue);

    if(eResult == rcSUCCESS)
    {
        return (INT16)cValue;
    }
    return 127;
}



