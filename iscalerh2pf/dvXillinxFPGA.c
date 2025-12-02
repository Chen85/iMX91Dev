// ===============================================================================
// FILE NAME: dvLDDriver.c
// DESCRIPTION:
//
//
// Modification History
// --------------------
// 2017/05/26, Casper Create
// --------------------
// ===============================================================================

//#include "Board.h"
#include "dvXillinxFPGA.h"
#include "Board_I2C_Dev_Table.h"
#include "Board_I2C.h"
#include "utilDbgMsg.h"
#include "utilOPD_TEST.h"


// ==============================================================================
// FUNCTION NAME: dvXillinxFPGARegWrite
// DESCRIPTION:
//
//
// Params:
// UINT8 reg:
// UINT16 size:
// UINT8 *data:
//
// Returns:
//
//
// Modification History
// --------------------
// 2018/06/22, Doulas Create
// --------------------
// ==============================================================================
eRESULT dvXillinxFPGARegWrite(UINT8 reg, UINT16 size, UINT8 *data)
{
    eRESULT eResult = rcERROR;
    UINT8    retry = 0;
    BYTE eDevID = eCM_IF_XFPGA;

//    ASSERT(reg < eLDDRVA_CMD_NUMBERS);
    if(Syscfg_Value_Get_Typeint(eWith_Xilinx_FPGA) == 0)
        return rcSUCCESS;

    if (Board_Stage_Get() > EVT_STAGE)
    {
        do
        {
            dvI2C_Dev_TotalCount(eDevID);

            eResult = Board_I2C_Master_Write(XILLINX_FPGA_I2C_BUS,
                                             XILLINX_FPGA_I2C_ADDRESS,
                                             reg,
                                             size,
                                             data,
                                             XILLINX_FPGA_I2C_FLAG);
			if(eResult != rcSUCCESS)  //G100_Steven_0078
			{
				dvI2C_Dev_RetryCount(eDevID);
				utilOPD_I2C_Warn_Flag_Set(eDEVICE_ERROR_XFPGA);
			}
        }
        while((retry++ < XILLINX_FPGA_I2C_RETRY) && (eResult != rcSUCCESS));
    }
    else
    {
        eResult = rcSUCCESS;
    }
	if(eResult != rcSUCCESS)
	{
		dvI2C_Dev_ErrorCount(eDevID);  //G100_Steven_0078
		utilOPD_I2C_Error_Flag_Set(eDEVICE_ERROR_XFPGA);
		LOG_MSG(db_DV_I2C, "XFPGA 0x%02X %s: %d\r\n", reg, __FILE__, __LINE__);
    }

    return eResult;
}


// ==============================================================================
// FUNCTION NAME: dvXillinxFPGARegRead
// DESCRIPTION:
//
//
// Params:
// UINT8 reg:
// UINT16 size:
// UINT8 *data:
//
// Returns:
//
//
// Modification History
// --------------------
// 2018/06/22, Doulas Create
// --------------------
// ==============================================================================
eRESULT dvXillinxFPGARegRead(UINT8 reg, UINT16 size, UINT8 *data)
{
    eRESULT eResult = rcERROR;
    UINT8    retry = 0;
    BYTE eDevID = eCM_IF_XFPGA;

    if(Syscfg_Value_Get_Typeint(eWith_Xilinx_FPGA) == 0)
        return rcSUCCESS;

    if (Board_Stage_Get() > EVT_STAGE)
    {
        do
        {
			dvI2C_Dev_TotalCount(eDevID);

            eResult = Board_I2C_Master_Read(XILLINX_FPGA_I2C_BUS,
                                            XILLINX_FPGA_I2C_ADDRESS,
                                            reg,
                                            size,
                                            data,
                                            XILLINX_FPGA_I2C_FLAG);
			if(eResult != rcSUCCESS)  //G100_Steven_0078
			{
				dvI2C_Dev_RetryCount(eDevID);
				utilOPD_I2C_Warn_Flag_Set(eDEVICE_ERROR_XFPGA);
			}
        }
        while((retry++ < XILLINX_FPGA_I2C_RETRY) && (eResult != rcSUCCESS));
    }
    else
    {
        eResult = rcSUCCESS;
    }

	if(eResult != rcSUCCESS)
	{
		dvI2C_Dev_ErrorCount(eDevID);  //G100_Steven_0078
		utilOPD_I2C_Error_Flag_Set(eDEVICE_ERROR_XFPGA);
		LOG_MSG(db_DV_I2C, "XFPGA 0x%02X %s: %d\r\n", reg, __FILE__, __LINE__);
	}
    return eResult;
}


