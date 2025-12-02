// ===============================================================================
// FILE NAME: dvLDDriver.c
// DESCRIPTION:
//
//
// Modification History
// --------------------
// 2017/11/27, A70LV_Eric.C_0023
// --------------------
// ===============================================================================

//#include "Board.h"
#include "Board_I2C_Dev_Table.h"
#include "Board_I2C.h"
#include "utilDbgMsg.h"
#include "utilOPD_TEST.h"
#include "dvFrontEndDriver.h"

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
// 2017/11/27, A70LV_Eric.C_0023
// --------------------
// ==============================================================================
eRESULT dvFrontEndDriverRegWrite(UINT8 reg, UINT16 size, UINT8 *data)
{
    eRESULT eResult = rcSUCCESS;
    UINT8    retry = 0;
    BYTE eDevID = eCM_IF_FRONTEND;

    if(Board_Stage_Get() > MOCKUP_STAGE)
    {
        do
        {
			dvI2C_Dev_TotalCount(eDevID);

            eResult = Board_I2C_Master_Write(FRONT_END_I2C_BUS,
                                             FRONT_END_I2C_ADDRESS,
                                             reg,
                                             size,
                                             data,
                                             FRONT_END_I2C_FLAG);
			if(eResult != rcSUCCESS) //G100_Steven_0078
			{
				dvI2C_Dev_RetryCount(eDevID);
				utilOPD_I2C_Warn_Flag_Set(eDEVICE_ERROR_FRONTEND);
			}
        }
        while((retry++ < FRONT_END_I2C_RETRY) && (eResult != rcSUCCESS));

        if(eResult != rcSUCCESS)
        {
        	dvI2C_Dev_ErrorCount(eDevID);  //G100_Steven_0078
			utilOPD_I2C_Error_Flag_Set(eDEVICE_ERROR_FRONTEND);
            LOG_MSG(db_DV_I2C, "FrontEnd 0x%02X %s: %d\r\n", reg, __FILE__, __LINE__);
        }
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
eRESULT dvFrontEndDriverRegRead(UINT8 reg, UINT16 size, UINT8 *data)
{
    eRESULT eResult = rcSUCCESS;
    UINT8    retry = 0;
    BYTE eDevID = eCM_IF_FRONTEND;

    if (Board_Stage_Get() > MOCKUP_STAGE)
    {
        do
        {
			dvI2C_Dev_TotalCount(eDevID);

            eResult = Board_I2C_Master_Read(FRONT_END_I2C_BUS,
                                            FRONT_END_I2C_ADDRESS,
                                            reg,
                                            size,
                                            data,
                                            FRONT_END_I2C_FLAG);
			if(eResult != rcSUCCESS)  //G100_Steven_0078
			{
				dvI2C_Dev_RetryCount(eDevID);
				utilOPD_I2C_Warn_Flag_Set(eDEVICE_ERROR_FRONTEND);
			}
        }
        while((retry++ < FRONT_END_I2C_RETRY) && (eResult != rcSUCCESS));

        if(eResult != rcSUCCESS)
        {
        	dvI2C_Dev_ErrorCount(eDevID);  //G100_Steven_0078
			utilOPD_I2C_Error_Flag_Set(eDEVICE_ERROR_FRONTEND);
            LOG_MSG(db_DV_I2C, "FrontEnd 0x%02X %s: %d\r\n", reg, __FILE__, __LINE__);
        }
    }

    return eResult;
}

eRESULT dvFrontEndDriverRegWrite_2ByteReg(UINT8 main, UINT8 reg, UINT16 size, UINT8 *data)  //G100_Julie_0001
{
    eRESULT eResult = rcSUCCESS;
    UINT8    retry = 0;
    BYTE eDevID = eCM_IF_FRONTEND;

    if (Board_Stage_Get() > MOCKUP_STAGE)
    {
        do
        {
			dvI2C_Dev_TotalCount(eDevID);

            eResult = Board_I2C_Master_Write(FRONT_END_I2C_BUS,
                                             FRONT_END_I2C_ADDRESS,
                                             CMD_ID(main, reg, 0),
                                             size,
                                             data,
                                             FRONT_END_I2C_FLAG_2ByteReg);
			if(eResult != rcSUCCESS)  //G100_Steven_0078
			{
				dvI2C_Dev_RetryCount(eDevID);
				utilOPD_I2C_Warn_Flag_Set(eDEVICE_ERROR_FRONTEND);
			}
        }
        while((retry++ < FRONT_END_I2C_RETRY) && (eResult != rcSUCCESS));

        if(eResult != rcSUCCESS)
        {
        	dvI2C_Dev_ErrorCount(eDevID);  //G100_Steven_0078
			utilOPD_I2C_Error_Flag_Set(eDEVICE_ERROR_FRONTEND);
            LOG_MSG(db_DV_I2C, "FrontEnd 0x%02X %s: %d\r\n", reg, __FILE__, __LINE__);
        }
    }

    return eResult;
}

eRESULT dvFrontEndDriverRegRead_2ByteReg(UINT8 main, UINT8 reg, UINT16 size, UINT8 *data)  //G100_Julie_0001
{
    eRESULT eResult = rcSUCCESS;
    UINT8    retry = 0;
    BYTE eDevID = eCM_IF_FRONTEND;

    if (Board_Stage_Get() > MOCKUP_STAGE)
    {
        do
        {
			dvI2C_Dev_TotalCount(eDevID);

            eResult = Board_I2C_Master_Read(FRONT_END_I2C_BUS,
                                            FRONT_END_I2C_ADDRESS,
                                            CMD_ID(main, reg, 1),
                                            size,
                                            data,
                                            FRONT_END_I2C_FLAG_2ByteReg);
			if(eResult != rcSUCCESS)  //G100_Steven_0078
			{
				dvI2C_Dev_RetryCount(eDevID);
				utilOPD_I2C_Warn_Flag_Set(eDEVICE_ERROR_FRONTEND);
			}
        }
        while((retry++ < FRONT_END_I2C_RETRY) && (eResult != rcSUCCESS));

        if(eResult != rcSUCCESS)
        {
        	dvI2C_Dev_ErrorCount(eDevID);  //G100_Steven_0078
			utilOPD_I2C_Error_Flag_Set(eDEVICE_ERROR_FRONTEND);
            LOG_MSG(db_DV_I2C, "FrontEnd 0x%02X %s: %d\r\n", reg, __FILE__, __LINE__);
        }
    }

    return eResult;
}


