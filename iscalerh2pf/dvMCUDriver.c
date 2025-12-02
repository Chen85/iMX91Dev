// ===============================================================================
// FILE NAME: dvMCUDriver.c
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
#include "utilCLICmdAPI.h"
#include "utilOPD_TEST.h"
#include "dvMCUDriver.h"

#define CMD_ID(MODULE, SUBCMD, RW) ((MODULE<<1|RW)<<8|SUBCMD)

//#define ENABLE_I2C_CHECKSUM
//UINT8 EnableChecksum = FALSE;

// ==============================================================================
// FUNCTION NAME: dvMCUDriverRegWrite
// DESCRIPTION:
//
//
// Params:
// UINT8 module:
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
eRESULT dvMCUDriverRegWrite(UINT8 main, UINT8 reg, UINT16 size, UINT8 *data)
{
    eRESULT eResult = rcSUCCESS;
    UINT8    retry = 0, ucWData[1026];
    uint16 uiWIndex = size;
    BYTE eDevID = eCM_IF_STANDBY;

    //ASSERT(reg < eSYSTEM_MSG_NUMBERS);

    if(Board_Stage_Get() > MOCKUP_STAGE)
    {
#ifndef NO_INTERFACE
        if(Board_I2C_MCUEnable())
        {
            uiWIndex = CheckSumCal_Packet(ucWData, data, size);

            if(uiWIndex != 0)
            {
                do
                {
    				dvI2C_Dev_TotalCount(eDevID);

                    eResult = Board_I2C_Master_Write(LPC_54605_I2C_BUS,
                                                    LPC_54605_I2C_ADDRESS,
                                                    CMD_ID(main, reg, 0),
                                                    uiWIndex,
                                                    ucWData,
                                                    LPC_54605_I2C_FLAG);
    		        if(eResult != rcSUCCESS)  //G100_Steven_0078
    		        {
    		        	dvI2C_Dev_RetryCount(eDevID);
    					utilOPD_I2C_Warn_Flag_Set(eDEVICE_ERROR_SYSTEM);
    		        }
                }
                while((retry++ < LPC_54605_I2C_RETRY) && (eResult != rcSUCCESS));
            }
            //LOG_MSG(db_ALWAYS, "MCU_W[%d][%d] ucWData[%d] uiWIndex[%d]\r\n", main, reg, ucWData[uiWIndex-1], uiWIndex);
        }
        else
        {
            do
            {
                dvI2C_Dev_TotalCount(eDevID);

                eResult = Board_I2C_Master_Write(LPC_54605_I2C_BUS,
                                                 LPC_54605_I2C_ADDRESS,
                                                 CMD_ID(main, reg, 0),
                                                 size,
                                                 data,
                                                 LPC_54605_I2C_FLAG);
    	        if(eResult != rcSUCCESS)  //G100_Steven_0078
    	        {
    	        	dvI2C_Dev_RetryCount(eDevID);
    				utilOPD_I2C_Warn_Flag_Set(eDEVICE_ERROR_SYSTEM);
    	        }
            }
            while((retry++ < LPC_54605_I2C_RETRY) && (eResult != rcSUCCESS));

        }

        if(eResult != rcSUCCESS)
        {
        	dvI2C_Dev_ErrorCount(eDevID);  //G100_Steven_0078
			utilOPD_I2C_Error_Flag_Set(eDEVICE_ERROR_SYSTEM);
            LOG_MSG(db_ALWAYS, "MCUDrv 0x%02X %s: %d\r\n", reg, __FILE__, __LINE__);
        }

#endif /* 0 */
    }

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: dvLDDriverRegRead
// DESCRIPTION:
//
//
// Params:
// UINT8 module:
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
eRESULT dvMCUDriverRegRead(UINT8 main, UINT8 reg, UINT16 size, UINT8 *data)
{
    eRESULT eResult = rcSUCCESS, eResult_unPt = rcSUCCESS;
    UINT8   retry = 0, ucRData[1026];
    UINT16 uiRIndex = size;
    BYTE eDevID = eCM_IF_STANDBY;

    //ASSERT(reg < eSYSTEM_MSG_NUMBERS);

    if(Board_Stage_Get() > MOCKUP_STAGE)
    {
#ifndef NO_INTERFACE

        if(Board_I2C_MCUEnable())
            uiRIndex = size + 1;

        do
        {
            dvI2C_Dev_TotalCount(eDevID);

            eResult = Board_I2C_Master_Read(LPC_54605_I2C_BUS,
                                            LPC_54605_I2C_ADDRESS,
                                            CMD_ID(main, reg, 1),
                                            uiRIndex,
                                            ucRData,
                                            LPC_54605_I2C_FLAG);
	        if(eResult != rcSUCCESS)  //G100_Steven_0078
	        {
	        	dvI2C_Dev_RetryCount(eDevID);
				utilOPD_I2C_Warn_Flag_Set(eDEVICE_ERROR_SYSTEM);
	        }
        }
        while((retry++ < LPC_54605_I2C_RETRY) && (eResult != rcSUCCESS));

        if(Board_I2C_MCUEnable())
        {
            eResult_unPt = CheckSumCal_unpacket(ucRData, uiRIndex);

            if(eResult_unPt != rcSUCCESS)
            {
                LOG_MSG(db_ALWAYS, "Rx Error.(%s: %d:)\r\n", __FILE__, __LINE__);
         	    LOG_MSG(db_ALWAYS,"main[%d] sub[%d] size[%d](+1)\r\n", main >> 1, reg , uiRIndex);
            }
    		else
    		{
    		    memcpy(data, ucRData, size);    //G100_Clare_0032
    		}
		}
		else
		{
		    memcpy(data, ucRData, size);    //G100_Clare_0032
        }

        if(eResult != rcSUCCESS)
        {
        	dvI2C_Dev_ErrorCount(eDevID);  //G100_Steven_0078
			utilOPD_I2C_Error_Flag_Set(eDEVICE_ERROR_SYSTEM);
            LOG_MSG(db_ALWAYS, "MCUDrv main[%d] 0x%02X %s: %d\r\n", main, reg, __FILE__, __LINE__);
        }
#endif /* 0 */
    }

    return eResult;
}


#if 0
void dvMCUDriverI2CEnableChecksum(UINT8 Enable)
{
    EnableChecksum = Enable;
}
#endif /* 0 */

