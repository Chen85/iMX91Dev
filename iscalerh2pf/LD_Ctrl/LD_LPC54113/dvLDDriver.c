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
#include "dvLDDriver.h"
#include "Board_I2C_Dev_Table.h"
#include "Board_I2C.h"
#include "utilDbgMsg.h"
#include "utilOPD_TEST.h"

#define CMD_ID(MODULE, SUBCMD, RW) ((MODULE<<1|RW)<<8|SUBCMD)

// ==============================================================================
// FUNCTION NAME: dvLDDriverRegWrite
// DESCRIPTION:
//
//
// Params:
// UINT8 LD_num:
// UINT8 reg:
// UINT16 size:
// UINT8 *data:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/10/19, Larry Create
// --------------------
// ==============================================================================
eRESULT dvLDDriverRegWrite(UINT8 main, UINT8 reg, UINT16 size, UINT8 *data)
{
    eRESULT eResult = rcSUCCESS;
    UINT8    retry = 0;
    BYTE eDevID = eCM_IF_FORMATTER;

    if(Syscfg_Value_Get_Typeint(eWith_LDDRV_MCU) != TRUE)
    {
        return rcSUCCESS;
    }

    do
    {
		dvI2C_Dev_TotalCount(eDevID);

        eResult = Board_I2C_Master_Write(LDDRV_I2C_BUS,
                                         LDDRV_I2C_ADDRESS,
                                         CMD_ID(main, reg, 0),
                                         size,
                                         data,
                                         LDDRV_I2C_FLAG);

        if(eResult != rcSUCCESS)
		{
			dvI2C_Dev_RetryCount(eDevID);
			utilOPD_I2C_Warn_Flag_Set(eDEVICE_ERROR_FMT);
		}
    }
    while((retry++ < LDDRV_I2C_RETRY) && (eResult != rcSUCCESS));

    if(eResult != rcSUCCESS)
    {
        dvI2C_Dev_ErrorCount(eDevID);
        utilOPD_I2C_Error_Flag_Set(eDEVICE_ERROR_FMT);
        LOG_MSG(db_ALWAYS, "main %d 0x%02X %s: %d\r\n", main, reg, __FILE__, __LINE__);
    }

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: dvLDDriverRegRead
// DESCRIPTION:
//
//
// Params:
// UINT8 LD_Num:
// UINT8 reg:
// UINT16 size:
// UINT8 *data:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/10/19, Larry Create
// --------------------
// ==============================================================================
eRESULT dvLDDriverRegRead(UINT8 main, UINT8 reg, UINT16 size, UINT8 *data)
{
    eRESULT eResult = rcSUCCESS;
    UINT8    retry = 0;
    BYTE eDevID = eCM_IF_FORMATTER;

    if(Syscfg_Value_Get_Typeint(eWith_LDDRV_MCU) != TRUE)
    {
        *data = 0;
        return rcSUCCESS;
    }

    do
    {
		dvI2C_Dev_TotalCount(eDevID);

        eResult = Board_I2C_Master_Read(LDDRV_I2C_BUS,
                                        LDDRV_I2C_ADDRESS,
                                        CMD_ID(main, reg, 1),
                                        size,
                                        data,
                                        LDDRV_I2C_FLAG);

        if(eResult != rcSUCCESS)
		{
			dvI2C_Dev_RetryCount(eDevID);
			utilOPD_I2C_Warn_Flag_Set(eDEVICE_ERROR_FMT);
		}
    }
    while((retry++ < LDDRV_I2C_RETRY) && (eResult != rcSUCCESS));

    if(eResult != rcSUCCESS)
    {
        dvI2C_Dev_ErrorCount(eDevID);
        utilOPD_I2C_Error_Flag_Set(eDEVICE_ERROR_FMT);
        LOG_MSG(db_ALWAYS, "main %d 0x%02X %s: %d\r\n", main, reg, __FILE__, __LINE__);
    }

    return eResult;
}

