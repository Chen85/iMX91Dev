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
#include "dvMotorBoard.h"
#include "Board_I2C_Dev_Table.h"
#include "Board_I2C.h"
#include "Board_Uart.h"
#include "utilDbgMsg.h"
#include "utilOPD_TEST.h"
#include "appDataMgr.h"

#define CMD_ID(MODULE, SUBCMD, RW)  ((MODULE<<1|RW)<<8|SUBCMD)
#define MODULE_MOTOR                (1)
// ==============================================================================
// FUNCTION NAME: dvMotorRegWrite
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
// 2017/10/19, Larry Create
// --------------------
// ==============================================================================
eRESULT dvMotorRegWrite(UINT8 reg, UINT16 size, UINT8 *data)
{
    eRESULT eResult = rcSUCCESS;
    UINT8   retry = 0;
	BYTE	eDevID = eCM_IF_MOTOR;

    if(Board_SingleBoard_Get() == FALSE)
    {
        do
        {
            eResult = Board_I2C_Master_Write(MOTOR_I2C_BUS,
                                             MOTOR_I2C_ADDRESS,
                                             CMD_ID(MODULE_MOTOR, reg, 0),
                                             size,
                                             data,
                                             MOTOR_I2C_FLAG);

            if(eResult != rcSUCCESS)  //G100_Steven_0078
            {
            	dvI2C_Dev_RetryCount(eDevID);
			    utilOPD_I2C_Warn_Flag_Set(eDEVICE_ERROR_MOTOR);
            }

            dvI2C_Dev_TotalCount(eDevID);
        }
        while((retry++ < MOTOR_I2C_RETRY) && (eResult != rcSUCCESS));

#ifndef DMD_BOARD_TEST //HICC2_Doulas_0127
        ASSERT(eResult == rcSUCCESS);
#endif
		if(eResult != rcSUCCESS)
		{
			dvI2C_Dev_ErrorCount(eDevID);  //G100_Steven_0078
			utilOPD_I2C_Error_Flag_Set(eDEVICE_ERROR_MOTOR);
#ifndef DMD_BOARD_TEST //HICC2_Doulas_0127
			LOG_MSG(db_ALWAYS, "Motor 0x%02X %s: %d\r\n", reg, __FILE__, __LINE__);
#endif
		}
    }

    return eResult;
}


// ==============================================================================
// FUNCTION NAME: dvMotorRegRead
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
// 2017/10/19, Larry Create
// --------------------
// ==============================================================================
eRESULT dvMotorRegRead(UINT8 reg, UINT16 size, UINT8 *data)
{
    eRESULT eResult = rcSUCCESS;
    UINT8   retry = 0;
	BYTE	eDevID = eCM_IF_MOTOR;

    if(Board_SingleBoard_Get() == FALSE)
    {
        do
        {
            eResult = Board_I2C_Master_Read(MOTOR_I2C_BUS,
                                            MOTOR_I2C_ADDRESS,
                                            CMD_ID(MODULE_MOTOR, reg, 1),
                                            size,
                                            data,
                                            MOTOR_I2C_FLAG);
			if(eResult != rcSUCCESS)  //G100_Steven_0078
			{
				dvI2C_Dev_RetryCount(eDevID);
				utilOPD_I2C_Warn_Flag_Set(eDEVICE_ERROR_MOTOR);
			}

		    dvI2C_Dev_TotalCount(eDevID);
        }
        while((retry++ < MOTOR_I2C_RETRY) && (eResult != rcSUCCESS));

#ifndef DMD_BOARD_TEST //HICC2_Doulas_0127
        ASSERT(eResult == rcSUCCESS);
#endif
		if(eResult != rcSUCCESS)
		{
			dvI2C_Dev_ErrorCount(eDevID);  //G100_Steven_0078
			utilOPD_I2C_Error_Flag_Set(eDEVICE_ERROR_MOTOR);
#ifndef DMD_BOARD_TEST //HICC2_Doulas_0127
			LOG_MSG(db_ALWAYS, "Motor 0x%02X %s: %d\r\n", reg, __FILE__, __LINE__);
#endif
		}
    }

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: dvMotor2RegWrite
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
// 2021/04/06, Larry Create
// --------------------
// ==============================================================================
eRESULT dvMotor2RegWrite(UINT8 main, UINT8 reg, UINT16 size, UINT8 *data)
{
    eRESULT eResult = rcSUCCESS;
    UINT8   retry = 0;
	BYTE	eDevID = eCM_IF_MOTOR;

    if(Board_SingleBoard_Get() == FALSE)
    {
        do
        {
            eResult = Board_I2C_Master_Write(MOTOR_I2C_BUS,
                                             MOTOR_I2C_ADDRESS,
                                             CMD_ID(main, reg, 0),
                                             size,
                                             data,
                                             MOTOR_I2C_FLAG);

            if(eResult != rcSUCCESS)  //G100_Steven_0078
            {
            	dvI2C_Dev_RetryCount(eDevID);
    		    utilOPD_I2C_Warn_Flag_Set(eDEVICE_ERROR_MOTOR);
            }

    		dvI2C_Dev_TotalCount(eDevID);
        }
        while((retry++ < MOTOR_I2C_RETRY) && (eResult != rcSUCCESS));
    }

#ifndef DMD_BOARD_TEST //HICC2_Doulas_0127
    ASSERT(eResult == rcSUCCESS);
#endif
	if(eResult != rcSUCCESS)
	{
		dvI2C_Dev_ErrorCount(eDevID);  //G100_Steven_0078
		utilOPD_I2C_Error_Flag_Set(eDEVICE_ERROR_MOTOR);
#ifndef DMD_BOARD_TEST //HICC2_Doulas_0127
		LOG_MSG(db_ALWAYS, "Motor 0x%02X %s: %d\r\n", reg, __FILE__, __LINE__);
#endif
	}

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: dvMotor2RegRead
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
// 2021/04/06, Larry Create
// --------------------
// ==============================================================================
eRESULT dvMotor2RegRead(UINT8 main, UINT8 reg, UINT16 size, UINT8 *data)
{
    eRESULT eResult = rcSUCCESS;
    UINT8   retry = 0;
	BYTE	eDevID = eCM_IF_MOTOR;

    if(Board_SingleBoard_Get() == FALSE)
    {
        do
        {
            eResult = Board_I2C_Master_Read(MOTOR_I2C_BUS,
                                            MOTOR_I2C_ADDRESS,
                                            CMD_ID(main, reg, 1),
                                            size,
                                            data,
                                            MOTOR_I2C_FLAG);
    		if(eResult != rcSUCCESS)  //G100_Steven_0078
    		{
    			dvI2C_Dev_RetryCount(eDevID);
    			utilOPD_I2C_Warn_Flag_Set(eDEVICE_ERROR_MOTOR);
    		}

    		dvI2C_Dev_TotalCount(eDevID);
        }
        while((retry++ < MOTOR_I2C_RETRY) && (eResult != rcSUCCESS));
    }

#ifndef DMD_BOARD_TEST //HICC2_Doulas_0127
    ASSERT(eResult == rcSUCCESS);
#endif
	if(eResult != rcSUCCESS)
	{
		dvI2C_Dev_ErrorCount(eDevID);  //G100_Steven_0078
		utilOPD_I2C_Error_Flag_Set(eDEVICE_ERROR_MOTOR);
#ifndef DMD_BOARD_TEST //HICC2_Doulas_0127
		LOG_MSG(db_ALWAYS, "Motor 0x%02X %s: %d\r\n", reg, __FILE__, __LINE__);
#endif
	}


    return eResult;
}

INT32 dvMotorGecCmd(eMOTOR_GEC_CMD eCMD, UINT32 eSUB1, UINT32 eSUB2)
{
    UINT8 acOutString[64] = {0};
    UINT16 uiStrLeng = 0;
    INT32 iResult = -1;
	UINT16 uiVal;


    switch(eCMD)
    {
        case eCLI_LSM_FOCUS:
            switch(eSUB1)
            {
                case eMFC_STOP:
                    snprintf(acOutString, sizeof(acOutString), "<Focus,%d,0,STOP>", eMFC_STOP);
                    break;

                case eMFC_FAR_SINGLE:
                    snprintf(acOutString, sizeof(acOutString), "<Focus,%d,0,FAR_SINGLE>", eMFC_FAR_SINGLE);
                    break;

                case eMFC_NEAR_SINGLE:
                    snprintf(acOutString, sizeof(acOutString), "<Focus,%d,0,NEAR_SINGLE>", eMFC_NEAR_SINGLE);
                    break;

                case eMFC_FAR_RUN:
                    snprintf(acOutString, sizeof(acOutString), "<Focus,%d,0,FAR_RUN>", eMFC_FAR_RUN);
                    break;

                case eMFC_NEAR_RUN:
                    snprintf(acOutString, sizeof(acOutString), "<Focus,%d,0,NEAR_RUN>", eMFC_NEAR_RUN);
                    break;
            }
            break;

        case eCLI_LSM_ZOOM:
            switch(eSUB1)
            {
                case eMZC_STOP:
                    snprintf(acOutString, sizeof(acOutString), "<Zoom,%d,0,STOP>", eMZC_STOP);
                    break;

                case eMZC_ZOOM_IN_SINGLE:
                    snprintf(acOutString, sizeof(acOutString), "<Zoom,%d,0,ZOOM_IN_SINGLE>", eMZC_ZOOM_IN_SINGLE);
                    break;

                case eMZC_ZOOM_OUT_SINGLE:
                    snprintf(acOutString, sizeof(acOutString), "<Zoom,%d,0,ZOOM_OUT_SINGLE>", eMZC_ZOOM_OUT_SINGLE);
                    break;

                case eMZC_ZOOM_IN_RUN:
                    snprintf(acOutString, sizeof(acOutString), "<Zoom,%d,0,ZOOM_IN_RUN>", eMZC_ZOOM_IN_RUN);
                    break;

                case eMZC_ZOOM_OUT_RUN:
                    snprintf(acOutString, sizeof(acOutString), "<Zoom,%d,0,ZOOM_OUT_RUN>", eMZC_ZOOM_OUT_RUN);
                    break;
            }
            break;

        case eCLI_LSM_H:
            switch(eSUB1)
            {
                case eMSH_STOP:
                    snprintf(acOutString, sizeof(acOutString), "<LsmH,%d,0,STOP>", eMSH_STOP);
                    break;
            #if defined(PLATFORM_R70K) //HICC2_AC_0004 //reverse lens direction for FUJI lens
                case eMSH_H_CALIBRATION:
                    snprintf(acOutString, sizeof(acOutString), "<LsmH,%d,0,H_CALIBRATION>", eMSH_H_CALIBRATION);

                case eMSH_RIGHT_SINGLE_STEP:
                    snprintf(acOutString, sizeof(acOutString), "<LsmH,%d,0,RIGHT_SINGLE_STEP>", eMSH_LEFT_SINGLE_STEP);
                    break;

                case eMSH_LEFT_SINGLE_STEP:
                    snprintf(acOutString, sizeof(acOutString), "<LsmH,%d,0,LEFT_SINGLE_STEP>", eMSH_RIGHT_SINGLE_STEP);
                    break;

                case eMSH_RIGHT_RUN:
                    snprintf(acOutString, sizeof(acOutString), "<LsmH,%d,0,RIGHT_RUN>", eMSH_LEFT_RUN);
                    break;

                case eMSH_LEFT_RUN:
                    snprintf(acOutString, sizeof(acOutString), "<LsmH,%d,0,LEFT_RUN>", eMSH_RIGHT_RUN);
                    break;

                case eMSH_GO_RIGHT_LIMIT:
                    snprintf(acOutString, sizeof(acOutString), "<LsmH,%d,0,GO_RIGHT_LIMIT>", eMSH_GO_LEFT_LIMIT);
                    break;

                case eMSH_GO_LEFT_LIMIT:
                    snprintf(acOutString, sizeof(acOutString), "<LsmH,%d,0,GO_LEFT_LIMIT>", eMSH_GO_RIGHT_LIMIT);
                    break;

                case eMSH_RIGHT_PIXEL:
                    snprintf(acOutString, sizeof(acOutString), "<LsmH,%d,%d,RIGHT_%d_PIXEL>", eMSH_LEFT_PIXEL, eSUB2, eSUB2);
                    break;

                case eMSH_LEFT_PIXEL:
                    snprintf(acOutString, sizeof(acOutString), "<LsmH,%d,%d,LEFT_%d_PIXEL>", eMSH_RIGHT_PIXEL, eSUB2, eSUB2);
                    break;

                case eMSH_RIGHT_STEP:
                    snprintf(acOutString, sizeof(acOutString), "<LsmH,%d,%d,RIGHT_%d_STEP>", eMSH_LEFT_STEP, eSUB2, eSUB2);
                    break;

                case eMSH_LEFT_STEP:
                    snprintf(acOutString, sizeof(acOutString), "<LsmH,%d,%d,LEFT_%d_STEP>", eMSH_RIGHT_STEP, eSUB2, eSUB2);
                    break;
            #else
                case eMSH_H_CALIBRATION:
                    snprintf(acOutString, sizeof(acOutString), "<LsmH,%d,0,H_CALIBRATION>", eMSH_H_CALIBRATION);

                case eMSH_RIGHT_SINGLE_STEP:
                    snprintf(acOutString, sizeof(acOutString), "<LsmH,%d,0,RIGHT_SINGLE_STEP>", eMSH_RIGHT_SINGLE_STEP);
                    break;

                case eMSH_LEFT_SINGLE_STEP:
                    snprintf(acOutString, sizeof(acOutString), "<LsmH,%d,0,LEFT_SINGLE_STEP>", eMSH_LEFT_SINGLE_STEP);
                    break;

                case eMSH_RIGHT_RUN:
                    snprintf(acOutString, sizeof(acOutString), "<LsmH,%d,0,RIGHT_RUN>", eMSH_RIGHT_RUN);
                    break;

                case eMSH_LEFT_RUN:
                    snprintf(acOutString, sizeof(acOutString), "<LsmH,%d,0,LEFT_RUN>", eMSH_LEFT_RUN);
                    break;

                case eMSH_GO_RIGHT_LIMIT:
                    snprintf(acOutString, sizeof(acOutString), "<LsmH,%d,0,GO_RIGHT_LIMIT>", eMSH_GO_RIGHT_LIMIT);
                    break;

                case eMSH_GO_LEFT_LIMIT:
                    snprintf(acOutString, sizeof(acOutString), "<LsmH,%d,0,GO_LEFT_LIMIT>", eMSH_GO_LEFT_LIMIT);
                    break;

                case eMSH_RIGHT_PIXEL:
                    snprintf(acOutString, sizeof(acOutString), "<LsmH,%d,%d,RIGHT_%d_PIXEL>", eMSH_RIGHT_PIXEL, eSUB2, eSUB2);
                    break;

                case eMSH_LEFT_PIXEL:
                    snprintf(acOutString, sizeof(acOutString), "<LsmH,%d,%d,LEFT_%d_PIXEL>", eMSH_LEFT_PIXEL, eSUB2, eSUB2);
                    break;

                case eMSH_RIGHT_STEP:
                    snprintf(acOutString, sizeof(acOutString), "<LsmH,%d,%d,RIGHT_%d_STEP>", eMSH_RIGHT_STEP, eSUB2, eSUB2);
                    break;

                case eMSH_LEFT_STEP:
                    snprintf(acOutString, sizeof(acOutString), "<LsmH,%d,%d,LEFT_%d_STEP>", eMSH_LEFT_STEP, eSUB2, eSUB2);
                    break;
            #endif
            }
            break;

        case eCLI_LSM_V:
            switch(eSUB1)
            {
                case eMSV_STOP:
                    snprintf(acOutString, sizeof(acOutString), "<LsmV,%d,0,STOP>", eMSV_STOP);
                    break;
            #if defined(PLATFORM_R70K) //HICC2_AC_0004 //reverse lens direction for FUJI lens
                case eMSV_V_CALIBRATION:
                    snprintf(acOutString, sizeof(acOutString), "<LsmV,%d,0,V_CALIBRATION>", eMSV_V_CALIBRATION);

                case eMSV_UP_SINGLE_STEP:
                    snprintf(acOutString, sizeof(acOutString), "<LsmV,%d,0,UP_SINGLE_STEP>", eMSV_DOWN_SINGLE_STEP);
                    break;

                case eMSV_DOWN_SINGLE_STEP:
                    snprintf(acOutString, sizeof(acOutString), "<LsmV,%d,0,DOWN_SINGLE_STEP>", eMSV_UP_SINGLE_STEP);
                    break;

                case eMSV_UP_RUN:
                    snprintf(acOutString, sizeof(acOutString), "<LsmV,%d,0,UP_RUN>", eMSV_DOWN_RUN);
                    break;

                case eMSV_DOWN_RUN:
                    snprintf(acOutString, sizeof(acOutString), "<LsmV,%d,0,DOWN_RUN>", eMSV_UP_RUN);
                    break;

                case eMSV_GO_UP_LIMIT:
                    snprintf(acOutString, sizeof(acOutString), "<LsmV,%d,0,GO_UP_LIMIT>", eMSV_GO_DOWN_LIMIT);
                    break;

                case eMSV_GO_DOWN_LIMIT:
                    snprintf(acOutString, sizeof(acOutString), "<LsmV,%d,0,GO_DOWN_LIMIT>", eMSV_GO_UP_LIMIT);
                    break;

                case eMSV_UP_PIXEL:
                    snprintf(acOutString, sizeof(acOutString), "<LsmV,%d,%d,UP_%d_PIXEL>", eMSV_DOWN_PIXEL, eSUB2, eSUB2);
                    break;

                case eMSV_DOWN_PIXEL:
                    snprintf(acOutString, sizeof(acOutString), "<LsmV,%d,%d,DOWN_%d_PIXEL>", eMSV_UP_PIXEL, eSUB2, eSUB2);
                    break;

                case eMSV_UP_STEP:
                    snprintf(acOutString, sizeof(acOutString), "<LsmV,%d,%d,UP_%d_STEP>", eMSV_DOWN_STEP, eSUB2, eSUB2);
                    break;

                case eMSV_DOWN_STEP:
                    snprintf(acOutString, sizeof(acOutString), "<LsmV,%d,%d,DOWN_%d_STEP>", eMSV_UP_STEP, eSUB2, eSUB2);
                    break;
            #else
                case eMSV_V_CALIBRATION:
                    snprintf(acOutString, sizeof(acOutString), "<LsmV,%d,0,V_CALIBRATION>", eMSV_V_CALIBRATION);

                case eMSV_UP_SINGLE_STEP:
                    snprintf(acOutString, sizeof(acOutString), "<LsmV,%d,0,UP_SINGLE_STEP>", eMSV_UP_SINGLE_STEP);
                    break;

                case eMSV_DOWN_SINGLE_STEP:
                    snprintf(acOutString, sizeof(acOutString), "<LsmV,%d,0,DOWN_SINGLE_STEP>", eMSV_DOWN_SINGLE_STEP);
                    break;

                case eMSV_UP_RUN:
                    snprintf(acOutString, sizeof(acOutString), "<LsmV,%d,0,UP_RUN>", eMSV_UP_RUN);
                    break;

                case eMSV_DOWN_RUN:
                    snprintf(acOutString, sizeof(acOutString), "<LsmV,%d,0,DOWN_RUN>", eMSV_DOWN_RUN);
                    break;

                case eMSV_GO_UP_LIMIT:
                    snprintf(acOutString, sizeof(acOutString), "<LsmV,%d,0,GO_UP_LIMIT>", eMSV_GO_UP_LIMIT);
                    break;

                case eMSV_GO_DOWN_LIMIT:
                    snprintf(acOutString, sizeof(acOutString), "<LsmV,%d,0,GO_DOWN_LIMIT>", eMSV_GO_DOWN_LIMIT);
                    break;

                case eMSV_UP_PIXEL:
                    snprintf(acOutString, sizeof(acOutString), "<LsmV,%d,%d,UP_%d_PIXEL>", eMSV_UP_PIXEL, eSUB2, eSUB2);
                    break;

                case eMSV_DOWN_PIXEL:
                    snprintf(acOutString, sizeof(acOutString), "<LsmV,%d,%d,DOWN_%d_PIXEL>", eMSV_DOWN_PIXEL, eSUB2, eSUB2);
                    break;

                case eMSV_UP_STEP:
                    snprintf(acOutString, sizeof(acOutString), "<LsmV,%d,%d,UP_%d_STEP>", eMSV_UP_STEP, eSUB2, eSUB2);
                    break;

                case eMSV_DOWN_STEP:
                    snprintf(acOutString, sizeof(acOutString), "<LsmV,%d,%d,DOWN_%d_STEP>", eMSV_DOWN_STEP, eSUB2, eSUB2);
                    break;
            #endif
            }
            break;

        case eCLI_LSM_CALIBATION:
            switch(eSUB1)
            {
                case eMLC_CALIBRATION:
                    snprintf(acOutString, sizeof(acOutString), "<LsmCalibration,%d,0,CALIBRATION>", eMLC_CALIBRATION);
                    break;

                case eMLC_LSM_CALIBRATION:
                    snprintf(acOutString, sizeof(acOutString), "<LsmCalibration,%d,0,LSM_CALIBRATION>", eMLC_LSM_CALIBRATION);
                    break;

                case eMLC_ZOOMLENS_CALIBRATION:
                    snprintf(acOutString, sizeof(acOutString), "<LsmCalibration,%d,0,ZOOMLENS_CALIBRATION>", eMLC_ZOOMLENS_CALIBRATION);
                    break;

                case eMLC_CALIBRATION_RECTANGLE: //HICC2_AC_0007
                    snprintf(acOutString, sizeof(acOutString), "<LsmCalibration,%d,0,eMLC_CALIBRATION_RECTANGLE>", eMLC_CALIBRATION_RECTANGLE);
                    break;

                case eMLC_LSM_CALIBRATION_DONE_ASK:
                    snprintf(acOutString, sizeof(acOutString), "<LsmCalibration,1,0,>"); //HICC2_AC_0010
                    break;
            }
            break;

        case eCLI_LSM_FOCUS2:
            switch(eSUB1)
            {
                case eMFC_STOP:
                    snprintf(acOutString, sizeof(acOutString), "<Focus2,%d,0,STOP>", eMFC_STOP);
                    break;

                case eMFC_RELAY_RESET:
                    snprintf(acOutString, sizeof(acOutString), "<Focus2,%d,0,RELAY_STOP>", eMFC_RELAY_RESET); //H30K_Julie_0005
                    break;

                case eMFC_FAR_SINGLE:
                    snprintf(acOutString, sizeof(acOutString), "<Focus2,%d,0,FAR_SINGLE>", eMFC_FAR_SINGLE);
                    break;

                case eMFC_NEAR_SINGLE:
                    snprintf(acOutString, sizeof(acOutString), "<Focus2,%d,0,NEAR_SINGLE>", eMFC_NEAR_SINGLE);
                    break;

                case eMFC_FAR_RUN:
                    snprintf(acOutString, sizeof(acOutString), "<Focus2,%d,0,FAR_RUN>", eMFC_FAR_RUN);
                    break;

                case eMFC_NEAR_RUN:
                    snprintf(acOutString, sizeof(acOutString), "<Focus2,%d,0,NEAR_RUN>", eMFC_NEAR_RUN);
                    break;
            }
            break;

        case eCLI_LSM_CFG:
            switch(eSUB1)
            {
                case eMLC_LSM_CONFIG_SUBID_LENSSHIFT:
                    snprintf(acOutString, sizeof(acOutString), "<LsmConfig,1,0,>");
                    break;

                case eMLC_LSM_CONFIG_SUBID_ZOOMLENS:
                    snprintf(acOutString, sizeof(acOutString), "<LsmConfig,2,0,>");
                    break;

                case eMLC_LSM_CONFIG_SUBID_LENS_ID:
                    snprintf(acOutString, sizeof(acOutString), "<LsmConfig,3,0,>");
                    break;

                case eMLC_LSM_CONFIG_SUBID_LENS_NAME:
                    snprintf(acOutString, sizeof(acOutString), "<LsmConfig,4,0,>");
                    break;

                case eMLC_LSM_CONFIG_SUBID_HEADER:
                    snprintf(acOutString, sizeof(acOutString), "<LsmConfig,5,0,>");
                    break;

				case eMLC_LSM_CONFIG_ZOOM_DURATION:
					palDataMgr_Access_Lens_Zoom_DurationTime(edaREAD,&uiVal);
                    snprintf(acOutString, sizeof(acOutString), "<LsmConfig,2,1,%d>", uiVal);
					break;

				case eMLC_LSM_CONFIG_FOCUS_DURATION:
					palDataMgr_Access_Lens_Focus_DurationTime(edaREAD,&uiVal);
                    snprintf(acOutString, sizeof(acOutString), "<LsmConfig,2,2,%d>", uiVal);
                    break;

				case eMLC_LSM_CONFIG_FOCUS2_DURATION:
					palDataMgr_Access_Lens_BackFocus_DurationTime(edaREAD,&uiVal);
                    snprintf(acOutString, sizeof(acOutString), "<LsmConfig,2,3,%d>", uiVal);
                    break;

				case eMLC_LSM_CONFIG_LENS_DETECTION:		//HICC2_Zonic_0009//H30K_Julie_0005
                    if(eSUB2 == 2)
                    {
                        snprintf(acOutString, sizeof(acOutString), "<LensDetection,0,2,>");
                    }
                    else
                    {
                        snprintf(acOutString, sizeof(acOutString), "<LensDetection,0,1,>");
                    }
					break;

				case eMLC_LSM_CONFIG_LENS_DETECTION_BYPASS:	
                    if(eSUB2 == 0)
                    {
                    	snprintf(acOutString, sizeof(acOutString), "<LensDetection,0,3,>");
                    }
                    else if(eSUB2 == 1)	//Bypass On
                    {
                    	snprintf(acOutString, sizeof(acOutString), "<LensDetection,0,3,1>");
                    }
                    else if(eSUB2 == 2)	//Bypass Off
                    {
                    	snprintf(acOutString, sizeof(acOutString), "<LensDetection,0,3,0>");
                    }

                case eMLC_LSM_CONFIG_V_UP_BOUNDARY:
                    snprintf(acOutString, sizeof(acOutString), "<LsmConfig,1,2,1,0,1>");
                    break;

                case eMLC_LSM_CONFIG_V_DOWN_BOUNDARY:
                    snprintf(acOutString, sizeof(acOutString), "<LsmConfig,1,2,1,0,0>");
                    break;

                case eMLC_LSM_CONFIG_H_LEFT_BOUNDARY:
                    snprintf(acOutString, sizeof(acOutString), "<LsmConfig,1,1,1,0,0>");
                    break;

                case eMLC_LSM_CONFIG_H_RIGHT_BOUNDARY:
                    snprintf(acOutString, sizeof(acOutString), "<LsmConfig,1,1,1,0,1>");
                    break;
            }
            break;

        case eCLI_LSM_STATUS:
            switch(eSUB1)
            {
                case eMLC_LSM_STATUS_QUERY:
                    snprintf(acOutString, sizeof(acOutString), "<LsmStatus,,,>");
                    break;

                case eMLC_LSM_STATUS_STOP:
                    snprintf(acOutString, sizeof(acOutString), "<LsmStatus,0,0,0>");
                    break;
            }
            break;

        case eCLI_LSM_POSITION:
			switch(eSUB1)
            {
                case eMLC_LSM_POSITION_GET:
            		snprintf(acOutString, sizeof(acOutString), "<LsmPosition,,,>");
					break;

				case eMLC_LSM_POSITION_MOVE_TO_CENTER:
            		snprintf(acOutString, sizeof(acOutString), "<LsmCenter,1,3,0>");
					break;
			}
            break;

        case eCLI_LSM_CENTER:
            switch(eSUB1)
            {
                case eMLC_LSM_CENTER_GET:
                    snprintf(acOutString, sizeof(acOutString), "<LsmCenter,,,>");
                    break;

                case eMLC_LSM_CENTER_SET:
                    snprintf(acOutString, sizeof(acOutString), "<LsmCenter,1,0,0>");
                    break;

                case eMLC_LSM_CENTER_CLEAR:
                    snprintf(acOutString, sizeof(acOutString), "<LsmCenter,0,0,12345>");
                    break;
            }
            break;

        case eCLI_LSM_MEMORY:
            switch(eSUB1)
            {
                 case eMLC_LSM_MEMORY_GET: //HICC2_AC_0019
                    snprintf(acOutString, sizeof(acOutString), "<LsmMemory,1,0,>");
                    break;

                case eMLC_LSM_MEMORY_SAVE:
                    snprintf(acOutString, sizeof(acOutString), "<LsmMemory,%d,%d,LSM_MEMORY_SAVE>", eMLC_LSM_MEMORY_SAVE, eSUB2);
                    break;

                case eMLC_LSM_MEMORY_APPLY:
                    snprintf(acOutString, sizeof(acOutString), "<LsmMemory,%d,%d,LSM_MEMORY_APPLY>", eMLC_LSM_MEMORY_APPLY, eSUB2);
                    break;

                case eMLC_LSM_MEMORY_CLEAR: //HICC2_AC_0019
                    snprintf(acOutString, sizeof(acOutString), "<LsmMemory,-1,0,0>");
                    break;
            }
            break;

        case eCLI_GEC_VERSION_R:
            switch(eSUB1)
            {
                case eMFC_FW_VERIOSN_LD_DRV:
                    snprintf(acOutString, sizeof(acOutString), "<VersionGet,1,,>");
                    break;

                case eMFC_FW_VERIOSN_LVPS:
                    snprintf(acOutString, sizeof(acOutString), "<VersionGet,2,,>");
                    break;

                case eMFC_FW_VERIOSN_TPLS:
                    snprintf(acOutString, sizeof(acOutString), "<VersionGet,3,,>");
                    break;

                case eMFC_FW_VERIOSN_CFCB:
                    snprintf(acOutString, sizeof(acOutString), "<VersionGet,4,,>");
                    break;

                case eMFC_FW_VERIOSN_LSM:
                    //snprintf(acOutString, sizeof(acOutString), "<VersionGet,5,0,>"); //0: global version
                    snprintf(acOutString, sizeof(acOutString), "<VersionGet,5,1,>"); //1: develop version
                    break;
            }
            break;

        case eCLI_LSM_FACTORY_MENU:
            switch(eSUB1)
            {
                case eMLC_LSM_FACTORY_MENU_LENS:
                    snprintf(acOutString, sizeof(acOutString), "<LensFactoryMenu,1,0,>");
                    break;

                case eMLC_LSM_FACTORY_MENU_ZOOMFOCUS:
                    snprintf(acOutString, sizeof(acOutString), "<LensFactoryMenu,2,0,>");
                    break;
            }
            break;

        case eCLI_LSM_MUTE: //HICC2_AC_0046
            switch(eSUB1)
            {
                case eMLC_LSM_SENSOR_MUTE_H:
                    snprintf(acOutString, sizeof(acOutString), "<LsmMute,1,1,1>");
                    break;

                case eMLC_LSM_SENSOR_ACTIVE_H:
                    snprintf(acOutString, sizeof(acOutString), "<LsmMute,1,1,0>");
                    break;

                case eMLC_LSM_SENSOR_MUTE_V:
                    snprintf(acOutString, sizeof(acOutString), "<LsmMute,2,1,1>");
                    break;

                case eMLC_LSM_SENSOR_ACTIVE_V:
                    snprintf(acOutString, sizeof(acOutString), "<LsmMute,2,1,0>");
                    break;

                case eMLC_LSM_BOUNDARY_MUTE_H:
                    snprintf(acOutString, sizeof(acOutString), "<LsmMute,1,2,1>");
                    break;

                case eMLC_LSM_BOUNDARY_ACTIVE_H:
                    snprintf(acOutString, sizeof(acOutString), "<LsmMute,1,2,0>");
                    break;

                case eMLC_LSM_BOUNDARY_MUTE_V:
                    snprintf(acOutString, sizeof(acOutString), "<LsmMute,2,2,1>");
                    break;

                case eMLC_LSM_BOUNDARY_ACTIVE_V:
                    snprintf(acOutString, sizeof(acOutString), "<LsmMute,2,2,0>");
                    break;
            }
            break;

		case eCLI_GEC_IIC_STATUS:
			snprintf(acOutString, sizeof(acOutString), "<Status,0,0,>");
            break;
    }

    iResult = Board_Uart_Write(eBOARD_UART_UART3, strlen(acOutString)+1, acOutString);
    LOG_MSG(db_APP_INPUTKEY, "dvMotorGecCmd Result:%d, Cmd %s \r\n", iResult, acOutString);

    return iResult;
}
