// ===============================================================================
// FILE NAME: dvFPGA10M04.c
// DESCRIPTION:
//
//
// Modification History
// --------------------
// 2017/06/23, Larry Create
// --------------------
// ===============================================================================

#include "Board_I2C_Dev_Table.h"
#include "utilDbgMsg.h"
#include "dvFPGA10M04.h"
#include "utilCounterAPI.h"     //A70LV_Doulas_0177
#include "utilCLICmdAPI.h"

#if 0
const sFPGA10M04CFG m_asFPGA10M04Cfg[eFPGA_NUMBER] =
{
    {I2C_FPGA10M04_BUS_0, I2C_FPGA10M04_ADDR_0},
    {I2C_FPGA10M04_BUS_1, I2C_FPGA10M04_ADDR_1},
    {I2C_FPGA10M04_BUS_2, I2C_FPGA10M04_ADDR_2},
};
#endif

// ==============================================================================
// FUNCTION NAME: dvFPGA_Reg_Write
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
// 2017/06/23, Larry Create
// --------------------
// ==============================================================================
eRESULT dvFPGA_Reg_Write(UINT8 ucBus, UINT16 uiReg, UINT16 uiDataNum, UINT8 *pucData)
{
    eRESULT eResult = rcERROR;
    UINT8   ucRetry = 0;

#if 0//(SCALER_BOARD_STAGE > MOCKUP_STAGE)
    do
    {
        eResult = Board_I2C_Master_Write(m_asFPGA10M04Cfg[ucBus].ucBus, m_asFPGA10M04Cfg[ucBus].ucAddress, uiReg, uiDataNum, pucData, I2C_FPGA10M04_FLAG);
    }
    while((ucRetry++ < I2C_FPGA10M04_RETRY) && (eResult != rcSUCCESS));

    ASSERT(eResult == rcSUCCESS);
#else
    eResult = rcSUCCESS;
#endif

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: dvFPGA_Reg_Read
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
// 2017/06/23, Larry Create
// --------------------
// ==============================================================================
eRESULT dvFPGA_Reg_Read(UINT8 ucBus, UINT16 uiReg, UINT16 uiDataNum, UINT8 *pucData)
{
    eRESULT eResult = rcERROR;
    UINT8   ucRetry = 0;

#if 0//(SCALER_BOARD_STAGE > MOCKUP_STAGE)
    do
    {
        eResult = Board_I2C_Master_Read(m_asFPGA10M04Cfg[ucBus].ucBus, m_asFPGA10M04Cfg[ucBus].ucAddress, uiReg, uiDataNum, pucData, I2C_FPGA10M04_FLAG);
    }
    while((ucRetry++ < I2C_FPGA10M04_RETRY) && (eResult != rcSUCCESS));

    ASSERT(eResult == rcSUCCESS);
#else
    eResult = rcSUCCESS;
#endif

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: dvFPGA_Initial
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
// 2017/11/21, Larry Create
// --------------------
// ==============================================================================
void dvFPGA_Initial(void)
{
    UINT8 ucData[4] = {0};

    dvFPGA_Reg_Read(eFPGA_SYSTEM, FPGA_VERSION_REG, 3, ucData);
    //LOG_MSG(db_ALWAYS, "FPGA_REG_VERSION is %d\r\n",ucData[0], ucData[1], ucData[2]);

    dvFPGA_Reg_Read(eFPGA_SYSTEM, FPGA_YEAR_REG, 1, ucData);
    //LOG_MSG(db_ALWAYS, "FPGA_REG_YEAR is %d\r\n",ucData[0]);

    dvFPGA_Reg_Read(eFPGA_SYSTEM, FPGA_MONTH_REG, 1, ucData);
    //LOG_MSG(db_ALWAYS, "FPGA_REG_MON is  %d\r\n",ucData[0]);

    dvFPGA_Reg_Read(eFPGA_SYSTEM, FPGA_DAY_REG, 1, ucData);
    //LOG_MSG(db_ALWAYS, "FPGA_REG_DAY is  %d\r\n",ucData[0]);
}

// ==============================================================================
// FUNCTION NAME: dvFPGA_Version_Read
// DESCRIPTION:
//
//
// Params:
// UINT16 uiReg:
// UINT8 *pucData:
//
// Returns:
//
//
// Modification History
// --------------------
// 2018/05/16, Larry Create
// --------------------
// ==============================================================================
eRESULT dvFPGA_Version_Read(UINT8 *pucData)
{
    return dvFPGA_Reg_Read(eFPGA_SYSTEM, FPGA_VERSION_REG, 3, pucData);
}

// ==============================================================================
// FUNCTION NAME: dvFPGA_System_Read
// DESCRIPTION:
//
//
// Params:
// UINT16 uiReg:
// UINT8 *pucData:
//
// Returns:
//
//
// Modification History
// --------------------
// 2018/01/19, Larry Create
// --------------------
// ==============================================================================
eRESULT dvFPGA_System_Read(UINT16 uiReg, UINT8 *pucData)
{
    return dvFPGA_Reg_Read(eFPGA_SYSTEM, uiReg, 1, pucData);
}

// ==============================================================================
// FUNCTION NAME: dvFPGA_System_Write
// DESCRIPTION:
//
//
// Params:
// UINT16 uiReg:
// UINT8 *pucData:
//
// Returns:
//
//
// Modification History
// --------------------
// 2018/01/19, Larry Create
// --------------------
// ==============================================================================
eRESULT dvFPGA_System_Write(UINT16 uiReg, UINT8 *pucData)
{
    return dvFPGA_Reg_Write(eFPGA_SYSTEM, uiReg, 1, pucData);
}

// ==============================================================================
// FUNCTION NAME: dvFPGA_XillinxFPGA_Reset
// DESCRIPTION:
//
//
// Params:
//
//
//
// Returns:
//
//
// Modification History
// --------------------
// 2018/05/25, A70LV_Doulas_0177
// --------------------
// ==============================================================================
eRESULT dvFPGA_XillinxFPGA_Reset(void)
{
#if 1
    return rcSUCCESS;
#else
    eRESULT eResult1,eResult2,eResult3;
    UINT8   ucData;
    MS_SLEEP(100);                        //A70LV_Doulas_0218 modify//A70LV_Doulas_0193 delay some tims,waiting C789 output ready
    eResult1 = dvFPGA_Reg_Read(eFPGA_SYSTEM, FPGA_XFPGA_REG, 1, &ucData);
    ucData = ucData & 0x7F;
    eResult2 = dvFPGA_Reg_Write(eFPGA_SYSTEM, FPGA_XFPGA_REG, 1, &ucData);
    MS_SLEEP(1);
    ucData = ucData | 0x80;
    eResult3 = dvFPGA_Reg_Write(eFPGA_SYSTEM, FPGA_XFPGA_REG, 1, &ucData);
    MS_SLEEP(10);                        //A70LV_Doulas_0193 avoid image garbage

    if((eResult1 == rcSUCCESS) &&
       (eResult2 == rcSUCCESS) &&
       (eResult3 == rcSUCCESS))
    {
        return rcSUCCESS;
    }
    else
    {
        LOG_MSG(db_ALWAYS, "###dvFPGA_XillinxFPGA_Reset failed!\r\n");
        return rcERROR;
    }
#endif
}

// ==============================================================================
// FUNCTION NAME: dvFPGA_XillinxFPGA_Reset11ms
// DESCRIPTION:
//
//
// Params:
//
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
eRESULT dvFPGA_XillinxFPGA_Reset11ms(void)
{
#if 1
    return rcSUCCESS;
#else
    eRESULT eResult1,eResult2,eResult3;
    UINT8   ucData;

    eResult1 = dvFPGA_Reg_Read(eFPGA_SYSTEM, FPGA_XFPGA_REG, 1, &ucData);
    ucData = ucData & 0x7F;
    eResult2 = dvFPGA_Reg_Write(eFPGA_SYSTEM, FPGA_XFPGA_REG, 1, &ucData);
    MS_SLEEP(11);
    ucData = ucData | 0x80;
    eResult3 = dvFPGA_Reg_Write(eFPGA_SYSTEM, FPGA_XFPGA_REG, 1, &ucData);


    if((eResult1 == rcSUCCESS) &&
       (eResult2 == rcSUCCESS) &&
       (eResult3 == rcSUCCESS))
    {
        return rcSUCCESS;
    }
    else
    {
        LOG_MSG(db_ALWAYS, "###dvFPGA_XillinxFPGA_Reset11ms failed!\r\n");
        return rcERROR;
    }
#endif
}

// ==============================================================================
// FUNCTION NAME: dvFPGA_Reset_Control
// DESCRIPTION:
//
//
// Params:
// UINT16 uiReg:
// UINT16 uiTimeMS:
//
// Returns:
//
//
// Modification History
// --------------------
// 2018/11/02, Larry Create
// --------------------
// ==============================================================================
eRESULT dvFPGA_Reset_Control(UINT16 uiReg, UINT16 uiTimeMS)
{
    UINT8   ucReg = (UINT8)((uiReg >> 8) & 0xFF);
    UINT8   ucDevice = (UINT8)(uiReg & 0xFF);
    UINT8   ucValue = 0xFF;
    eRESULT eResult = rcERROR;

#if 0
    eResult |= dvFPGA_Reg_Read(eFPGA_SYSTEM, (UINT16)ucReg, 1, &ucValue);

    ucValue = ~ucDevice & ucValue;
    eResult |= dvFPGA_Reg_Write(eFPGA_SYSTEM, (UINT16)ucReg, 1, &ucValue);

    MS_SLEEP(uiTimeMS);

    ucValue = ucDevice | ucValue;
    eResult |= dvFPGA_Reg_Write(eFPGA_SYSTEM, (UINT16)ucReg, 1, &ucValue);
#endif
    return eResult;
}


