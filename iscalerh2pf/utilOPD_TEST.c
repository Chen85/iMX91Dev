
#include "Board_Uart.h"

#include "utilOPD_TEST.h"
#include "utilDbgMsg.h"
#include "utilCLICmdAPI.h"
#include "utilHostAPI.h"
#include "utilDataMgrAPI.h"
#include "GEC_EventTable.h"

#include "halMCUCtrlAPI.h"
#include "halLDProc.h"
#include "halScaler.h"
#ifdef SCALER_C821_C789
#include "halC789CtrlAPI.h"
#endif
#include "halFrontEndCtrlAPI.h"

#include "palImgMgr.h"

static BOOL m_I2C_Test_Flag = 0;
static uint32 m_I2C_Error_Count[eI2C_NUMBER] = {0};
static uint32 m_UART_Error_Count[eUART_NUMBER] = {0};
static uint32 m_SPI_Error_Count[eSPI_NUMBER] = {0};

static BOOL m_UART_Test_Flag = 0;

static BYTE m_UART_TX = 0;
static BYTE m_UART_RX = 0;

static UINT16 dUartTest = 0;
static UINT16 dUartTotal = 0;

static BYTE m_aI2CWarnFlag[eDEVICE_ERROR_NUMBER] = {0}; //G100_Julie_0025

//G100_Julie_0002, start.
const char *m_sInterfaceString[] =
{
    "I2C",
    "UART",
    "SPI",
    "USB",
};
VERIFY_SIZE_OF(m_sInterfaceString, sizeof(m_sInterfaceString[0])*eIF_NUMBER);

const char *m_sDeviceModeString[eMODE_NUMBER] =
{
    "STATE",
    "WORK",
    "FAIL",
    "PASS",
};

#define CMD_DEVICEMODE_STRING_MAX  sizeof(m_sDeviceModeString)/sizeof(m_sDeviceModeString[0])
//G100_Julie_0002, end.


//依照案子不同，改變列舉定義字串
//exp, A65 eCM_IF_CLOCK_GENERATOR ->VCXO949, T50 eCM_IF_CLOCK_GENERATOR -> LMK03328
static const sICDEV_LIST_TABLE m_sICdev_List_LutTable[]=
{
	{eCM_IF_STANDBY,                           "Standby",                                    0},//00
    {eCM_IF_KEYPAD,                            "Keypad",                                     0},//01
    {eCM_IF_FORMATTER,                         "Formatter",                                  0},//02
    {eCM_IF_FRONTEND,                          "FrontEnd",                                   0},//03
    {eCM_IF_MOTOR,                             "Motor",                                      0},//04
    {eCM_IF_DDP4422,                           "DDP4422",                                    0},//05
    {eCM_IF_XFPGA,                             "XFPGA",                                      0},//06
    {eCM_IF_FPGA_VIDEO,                        "FPGA_Video",                                 0},//07
    {eCM_IF_FPGA_POWER,                        "FPGA_Power",                                 0},//08
    {eCM_IF_FPGA_LD,                           "FPGA_LD",                                    0},//09
    {eCM_IF_CLOCK_GENERATOR,                   "VCXO949",                                    0},//10
    {eCM_IF_EEPROM_1,                          "EEPROM_1",                                   0},//11
    {eCM_IF_EEPROM_2,                          "EEPROM_2",                                   0},//12
    {eCM_IF_VGA_SWITCH,                        "AVD7604",                                    0},//13
    {eCM_IF_HDBASET_SWITCH,                    "HDBASET(VS100/VS3000)",                      0},//14
    {eCM_IF_DP_SWITCH,                         "IT6563",                                     0},//15
    {eCM_IF_HDMI_SWITCH_1,                     "IT6634_1",                                   0},//16
    {eCM_IF_HDMI_SWITCH_2,                     "IT6634_2",                                   0},//17
    {eCM_IF_RBG_TO_HDMI,                       "IT6613",                                     0},//18
    {eCM_IF_HDMI_TO_RBG_1,                     "IT6805_1",                                   0},//19
    {eCM_IF_HDMI_TO_RBG_2,                     "IT6805_2",                                   0},//20
    {eCM_IF_EDID_HDMI1,                        "EDID_HDMI1",                                 0},//21
    {eCM_IF_EDID_HDMI2,                        "EDID_HDMI2",                                 0},//22
    {eCM_IF_EDID_HDMI3,                        "EDID_HDMI3",                                 0},//23
    {eCM_IF_EDID_DP,                           "EDID_DP",                                    0},//24
    {eCM_IF_EDID_DVI,                          "EDID_DVI",                                   0},//25
    {eCM_IF_EDID_HDBaseT,                      "EDID_HDBaseT",                               0},//26
    {eCM_IF_EDID_VGA,                          "EDID_VGA",                                   0},//27
    {eCM_IF_FAN_CTRL_BOARD,                    "FanCtrlBoard",                               0},//28
    {eCM_IF_TEMPERATURE_SENSOR_1,              "Thermal_Sensor_1",                           0},//29
    {eCM_IF_TEMPERATURE_SENSOR_2,              "Thermal_Sensor_2",                           0},//30
    {eCM_IF_TEMPERATURE_SENSOR_3,              "Thermal_Sensor_3",                           0},//31
    {eCM_IF_TEMPERATURE_SENSOR_4,              "Thermal_Sensor_4",                           0},//32
    {eCM_IF_TEMPERATURE_SENSOR_5,              "Thermal_Sensor_5",                           0},//33
    {eCM_IF_FAN_RPM_1,                         "MAX31790_1",                                 0},//34
    {eCM_IF_FAN_RPM_2,                         "MAX31790_2",                                 0},//35
    {eCM_IF_FAN_RPM_3,                         "MAX31790_3",                                 0},//36
    {eCM_IF_G_SENSOR,                          "G_SENSOR",                                   0},//37
    {eCM_IF_LVPS_MCU,                          "M481",                                       0},//38
    {eCM_IF_PRESSURE_THERMAL_SENSOR_1,         "Pressure_1",                                 0},//39
    {eCM_IF_PRESSURE_THERMAL_SENSOR_2,         "Pressure_2",                                 0},//40
    {eCM_IF_HUMIDITY_SENSOR_1,                 "Humidity_Sensor_1",                          0},//41
    {eCM_IF_ADC_CONVERTER,                     "ADC101",                                     0},//42
    {eCM_IF_MOTOR_DRIVER_1,                    "DRV10983_1",                                 0},//43
    {eCM_IF_MOTOR_DRIVER_2,                    "DRV10983_2",                                 0},//44
    {eCM_IF_IO_EXP_1,                          "PCF8575_1",                                  0},//45
    {eCM_IF_IO_EXP_2,                          "PCF8575_2",                                  0},//46
    {eCM_IF_IO_EXP_3,                          "PCF8575_3",                                  0},//47
    {eCM_IF_ICHIP_1,                   		   "C789",                              		 0},//48
    {eCM_IF_ICHIP_2,                    	   "C821",                                       0},//49
    {eCM_IF_ICHIP_3,                     	   "FPGA",                      				 0},//50
    {eCM_IF_3GSDI_SWITCH,                      "GS2961",                                     0},//51
    {eCM_IF_LD_CONVERT,                        "LD_Convert",                                 0},//52
    {eCM_IF_HDMI_REDRIVER_1,                   "HDMI_Driver_1",                              0},//53
    {eCM_IF_HDMI_REDRIVER_2,                   "HDMI_Driver_2",                              0},//54
    {eCM_IF_HDMI_REDRIVER_3,                   "HDMI_Driver_3",                              0},//55
    {eCM_IF_HDMI_REDRIVER_4,                   "HDMI_Driver_4",                              0},//56
    {eCM_IF_FAN_RPM_4,                         "MAX31790_4",                                 0},//57
    {eCM_IF_HUMIDITY_SENSOR_2,                 "Humidity_Sensor_2",                          0},//58
    {eCM_IF_HUMIDITY_SENSOR_3,                 "Humidity_Sensor_3",                          0},//59
    {eCM_IF_PRESSURE_THERMAL_SENSOR_3,         "Pressure_3",                                 0},//60
    {eCM_IF_HDMI_REDRIVER,                     "PI3HDX1204B1",                               0},//61
    {eCM_IF_PMIC,                              "PMIC_MMPF0100",                              0},//62

};
VERIFY_SIZE_OF(m_sICdev_List_LutTable, sizeof(m_sICdev_List_LutTable[0])*eCM_IF_NUMBER);

const char* m_cDeviceString[]=
{
    "iScaler",
    "SYS",
    "FE",
    "MOTOR",
    "FMT",
    "KEYPAD",
};
VERIFY_SIZE_OF(m_cDeviceString, sizeof(m_cDeviceString[0])*eDEVICE_MODULE_NUMBER);

FLOAT utilOPD_Error_Rate_Get(DWORD dData)
{
    float fRate = 0;

    if(dUartTotal == 0)
    {
        dUartTotal = 1;
    }

    fRate = (float)dData * 100 / dUartTotal;

    return fRate;
}

void utilOPD_Stress_Test_Enable_Set(UINT8 ucType, bool bEnable)
{
	if(ucType > eIF_NUMBER)
	{
		return;
	}

	switch(ucType)
	{
		case eIF_I2C:
			m_I2C_Test_Flag = bEnable;
			break;

		case eIF_UART:
		case eIF_SPI:
			m_UART_Test_Flag = bEnable;
			break;
	};
}

UINT8 utilOPD_Stress_Test_Enable_Get(UINT8 ucType)
{
	UINT8 cData;
	if(ucType > eIF_NUMBER)
	{
		return 0;
	}

	switch(ucType)
	{
		case eIF_I2C:
			cData = m_I2C_Test_Flag;
			break;

		case eIF_UART:
		case eIF_SPI:
			cData = m_UART_Test_Flag;
			break;
	};

	return cData;
}

//主控端的i2c, exp lpc54605...
void utilOPD_MasterDevice_Event(eINTERFACE ucType, eCM_IF_TABLE ucCM_Device, eDEVICE_MODE ucInfo, uint32 *pcData) //G100_Julie_0002
{
    uOPD_DATA uOPDData = {0};

    //LOG_MSG(db_ALWAYS,"DeviceLog[%d][%d][%d]\r\n", ucType, ucModule, ucInfo);
    if((ucType >= eIF_NUMBER) || (ucCM_Device >= eCM_IF_NUMBER) || (ucInfo >= eMODE_NUMBER))
    {
        ASSERT_ALWAYS();
        return;
    }

    snprintf(uOPDData.sDev.cDevInterface, 31, "%s", m_sInterfaceString[ucType]);
	snprintf(uOPDData.sDev.cDevName, 31, "%s<->%s", m_cDeviceString[eDEVICE_MODULE_LAN], m_sICdev_List_LutTable[ucCM_Device].cICDev_string);
    snprintf(uOPDData.sDev.cDevInfo, 31, "%s", m_sDeviceModeString[ucInfo]);

    uOPDData.sDev.cPara1 = 0;
    uOPDData.sDev.cPara2 = pcData[0];

    float fRate = utilOPD_Error_Rate_Get(uOPDData.sDev.cPara2);
    uOPDData.sDev.fPara5 = fRate;

    utilOPD_EventSet(eOPD_DEVICE_LOG, &uOPDData);
}

//主控端i2c控制的i2c slave, exp lpc54605的i2c
void utilOPD_SlaverDevice_Event(eINTERFACE ucType, eDEVICE_MODULE cMaster, eCM_IF_TABLE ucCM_SlaverDevice, eDEVICE_MODE ucInfo, uint32 *pcData) //G100_Julie_0002
{
    uOPD_DATA uOPDData = {0};

    if((ucType >= eIF_NUMBER) || (cMaster >= eDEVICE_MODULE_NUMBER) || (ucCM_SlaverDevice >= eCM_IF_NUMBER) ||(ucInfo >= eMODE_NUMBER))
    {
        LOG_MSG(db_ALWAYS,"DeviceLog[%d][%d][%d][%d]\r\n", ucType, cMaster, ucCM_SlaverDevice, ucInfo);
        return;
    }

    snprintf(uOPDData.sDev.cDevInterface, 31, "%s", m_sInterfaceString[ucType]);
	snprintf(uOPDData.sDev.cDevName, 31, "%s<->%s", m_cDeviceString[cMaster], m_sICdev_List_LutTable[ucCM_SlaverDevice].cICDev_string);
    snprintf(uOPDData.sDev.cDevInfo, 31, "%s", m_sDeviceModeString[ucInfo]);

    uOPDData.sDev.cPara1 = pcData[0];
    uOPDData.sDev.cPara2 = pcData[1];
    uOPDData.sDev.cPara3 = pcData[2];

	float fRate = utilOPD_Error_Rate_Get(uOPDData.sDev.cPara2);


    uOPDData.sDev.fPara5 = fRate;

    utilOPD_EventSet(eOPD_DEVICE_LOG, &uOPDData);
}

void utilOPD_UART_SPI_TEST(void)
{
    //uart test
    eRESULT eResult_uart = rcERROR;
    UINT8 ucRand = rand();
    ucRand = ((ucRand > 0 && ucRand < 255) ? ucRand : 99);

    //utilHost_EventWrite(eCMD_MODULE_OPD, eMSG_TYPE_UART_1, eOPD_MSG_UART_STRESS_TEST, eOPD_MSG_UART_STRESS_TEST_SZ, &ucRand, FALSE);
	eResult_uart = utilHost_EventWrite(eCMD_MODULE_OPD, eMSG_TYPE_UART_1, eOPD_MSG_UART_STRESS_TEST, eOPD_MSG_UART_STRESS_TEST_SZ, &ucRand, TRUE); //G100_Julie_0012
	//LOG_MSG(db_ALWAYS,"(%s,%d) result[%d] ucRand[%d]\r\n", __FUNCTION__, __LINE__, eResult_uart, ucRand);
	if(eResult_uart != rcSUCCESS)
	{
	    utilOPD_MasterDevice_Count_Set(eIF_UART, eUART_STANDBY);
	}

    //spi test
    halFrontEndCtrl_SPI_GS2961_Set();
    ucRand = 0xFF;
	halMCUCtrl_OPD_Register_Set(eOPD_MSG_SPI_STRESS_TEST, eOPD_MSG_SPI_STRESS_TEST_SZ, &ucRand);

    UINT32 ulvalue = 0xFFFF, ulvalue2 = 0x11FF;
#ifdef SCALER_C821_C789
    ulvalue = palImgMgr_Reg0_Read_Get();//dvC821_Reg0_Read_Get();
    if(ulvalue != 0)
    {
        utilOPD_MasterDevice_Count_Set(eIF_SPI, eSPI_ICHIP_2);
    }
#endif /* SCALER_C821_C789 */

    ulvalue2 = palImgMgr_WarpReg0_Read_Get();//halC789Ctrl_Reg0_Read_Get();
    if(ulvalue2 != 0)
    {
        utilOPD_MasterDevice_Count_Set(eIF_SPI, eSPI_ICHIP_1);
    }
}

void utilOPD_MasterDevice_Count_Set(BYTE cType, BYTE cDevice)
{
    if(cType > eIF_NUMBER || (cType == eIF_I2C && cDevice > eI2C_NUMBER) || (cType == eIF_UART && cDevice > eUART_NUMBER) || (cType == eIF_SPI && cDevice > eSPI_NUMBER))
    {
        return;
    }

	switch(cType)
	{
		case eIF_I2C:
			if(m_I2C_Error_Count[cDevice] < 0xffffffff)
			{
				m_I2C_Error_Count[cDevice]++;
			}
			break;

		case eIF_UART:
			if(m_UART_Error_Count[cDevice] < 0xffffffff)
			{
				m_UART_Error_Count[cDevice]++;
			}
			break;

		case eIF_SPI:
			if(m_SPI_Error_Count[cDevice] < 0xffffffff)
			{
				m_SPI_Error_Count[cDevice]++;
			}
			break;
	}
}

BYTE utilOPD_MasterDevice_Count_Get(BYTE cType, BYTE cDevice, UINT32 *ucData)
{
	UINT32 dCntValue = 0;
	BYTE   cIndex = 0;

    if(cType > eIF_NUMBER || (cType == eIF_I2C && cDevice > eI2C_NUMBER) || (cType == eIF_UART && cDevice > eUART_NUMBER) || (cType == eIF_SPI && cDevice > eSPI_NUMBER))
    {
        return eCM_IF_NUMBER;
    }

	switch(cType)
	{
		case eIF_I2C:
			dCntValue = m_I2C_Error_Count[cDevice];
            cIndex = m_I2CDeviceLut[cDevice];
			break;

		case eIF_UART:
			dCntValue = m_UART_Error_Count[cDevice];
            cIndex = m_UARTDeviceLut[cDevice];
			break;

		case eIF_SPI:
			dCntValue = m_SPI_Error_Count[cDevice];
            cIndex = m_SPIDeviceLut[cDevice];
			break;
	}

	*ucData = dCntValue;

	return cIndex;
}

void utilOPD_MasterDevice_Count_Reset(void) //G100_Julie_0002
{
    memset((BYTE*)&m_I2C_Error_Count,  0, sizeof(m_I2C_Error_Count));
    memset((BYTE*)&m_UART_Error_Count, 0, sizeof(m_UART_Error_Count));
    memset((BYTE*)&m_SPI_Error_Count,  0, sizeof(m_SPI_Error_Count));
}

void utilOPD_UartTestCount_Set(UINT16 uiCount)
{
    dUartTest = dUartTotal = uiCount;
    utilOPD_UartTotalCount_Set(uiCount); //A65_OPTOMA_Julie_0080
}

void utilOPD_I2C_RunTime_Log(eDEVICE_MODULE eModule, UINT8 ucCM_Device, uint32 ulRetryCount, uint32 ulErrorCount, uint32 ulTotalCount, eINTERFACE cType)
{
    uOPD_DATA uOPDData = {0};

    if(eModule > eDEVICE_MODULE_NUMBER || cType > eIF_NUMBER)
    {
        ASSERT_ALWAYS();
        return;
    }

    switch(eModule)
    {
        case eDEVICE_MODULE_LAN:
        case eDEVICE_MODULE_FRONTEND:
        case eDEVICE_MODULE_SYSTEM:
        case eDEVICE_MODULE_MOTOR:
        case eDEVICE_MODULE_FMT:
        case eDEVICE_MODULE_KEYPAD:
            if(ucCM_Device < eCM_IF_NUMBER)
            {
                snprintf(uOPDData.sRunTime.cDevName, 31, "%s<->%s", m_cDeviceString[eModule], m_sICdev_List_LutTable[ucCM_Device].cICDev_string);
            }
            else
            {

				//LOG_MSG(db_ALWAYS,"(%s,%d) ucCM_Device[%d]\r\n", __FUNCTION__, __LINE__, ucCM_Device);

                snprintf(uOPDData.sRunTime.cDevName, 31, "%s_Unknow_device", m_cDeviceString[eModule]);
            }
            break;

        default:
            break;
    }

	uOPDData.sRunTime.ulRetry = ulRetryCount;
	uOPDData.sRunTime.ulError = ulErrorCount;
	uOPDData.sRunTime.ulTotal = ulTotalCount;

	snprintf(uOPDData.sRunTime.cDriveType, 31, "%s", m_sInterfaceString[cType]);

    utilOPD_EventSet(eOPD_RUNTIME_LOG, &uOPDData);
}
 //G100_Steven_0078 start
void utilOPD_I2C_RunTimeStartUp_Log(eI2C_TABLE eI2C_dev, uint32 ulRetryCount, uint32 ulErrorCount, uint32 ulTotalCount, eINTERFACE cType)
{
    uOPD_DATA uOPDData = {0};

    if(eI2C_dev >= eI2C_NUMBER || cType > eIF_NUMBER)
    {
        ASSERT_ALWAYS();
        return;
    }

    if(m_I2CDeviceLut[eI2C_dev] < eCM_IF_NUMBER)
    {
        snprintf(uOPDData.sRunTime.cDevName, 31, "(StartUp)%s", m_sICdev_List_LutTable[m_I2CDeviceLut[eI2C_dev]].cICDev_string);
    }
    else
    {
        snprintf(uOPDData.sRunTime.cDevName, 31, "(StartUp)Unknow device");
    }

	uOPDData.sRunTime.ulRetry = ulRetryCount;
	uOPDData.sRunTime.ulError = ulErrorCount;
	uOPDData.sRunTime.ulTotal = ulTotalCount;

	snprintf(uOPDData.sRunTime.cDriveType, 31, "%s", m_sInterfaceString[cType]);

    utilOPD_EventSet(eOPD_RUNTIME_LOG, &uOPDData);
}
 //G100_Steven_0078 end
void utilOPD_I2C_RunTime_RecordEvent(UINT8 ucEvent)
{
	UINT8 ucIndex = 0;
    UINT8 ucTotal = 0;
    eRESULT eResult = rcSUCCESS;

    uint32 ulRetryCount, ulErrorCount, ulTotalCount;

    if(ucEvent == eOPD_RUNTIME_LOG)
	{
	    //LAN for i2c
        for(ucIndex = 0; ucIndex < eI2C_NUMBER; ucIndex++)
        {
             //between imx6 & system, i2c retry & error & total count.
    	    ulRetryCount = dvI2C_Dev_RunTimeCount_Get(eCOUNT_RETRY, ucIndex);
    	    ulErrorCount = dvI2C_Dev_RunTimeCount_Get(eCOUNT_ERROR, ucIndex);
    	    ulTotalCount = dvI2C_Dev_RunTimeCount_Get(eCOUNT_TOTAL, ucIndex);
    		utilOPD_I2C_RunTime_Log(eDEVICE_MODULE_LAN,
                                    m_I2CDeviceLut[ucIndex],
                                    ulRetryCount,
                                    ulErrorCount,
                                    ulTotalCount,
                                    eIF_I2C);
        }

        //LAN for uart //A65_OPTOMA_Julie_0023, start.
        utilOPD_I2C_RunTime_Log(eDEVICE_MODULE_LAN,
                                m_UARTDeviceLut[eCM_IF_STANDBY],
                                0,
                                dvUART_Dev_RunTimeCount_Get(eCOUNT_ERROR, eUART_STANDBY),
                                dvUART_Dev_RunTimeCount_Get(eCOUNT_TOTAL, eUART_STANDBY),
                                eIF_UART);

        //LAN for spi
        for(ucIndex = 0; ucIndex < eSPI_NUMBER; ucIndex++)
        {
            ulErrorCount = dvSPI_Dev_RunTimeCount_Get(eCOUNT_ERROR, ucIndex);
            ulTotalCount = dvSPI_Dev_RunTimeCount_Get(eCOUNT_TOTAL, ucIndex);
            utilOPD_I2C_RunTime_Log(eDEVICE_MODULE_LAN,
                                    m_SPIDeviceLut[ucIndex],
                                    0,
                                    ulErrorCount,
                                    ulTotalCount,
                                    eIF_SPI);
        }//A65_OPTOMA_Julie_0023, end.

        //System lpc54605
        ucTotal = 0;
        eResult = halMCUCtrl_OPD_Register_Get(eOPD_MSG_I2C_NUMBER, 1, (UINT8*)&ucTotal);

        if((eResult == rcSUCCESS) && (ucTotal != 0) && (ucTotal < 32))
        {
            sI2C_DEV_COUNT *psI2C_DevRetry = (sI2C_DEV_COUNT *)malloc(sizeof(sI2C_DEV_COUNT)*(ucTotal + 1));
            sI2C_DEV_COUNT *psI2C_DevError = (sI2C_DEV_COUNT *)malloc(sizeof(sI2C_DEV_COUNT)*(ucTotal + 1));
            sI2C_DEV_COUNT *psI2C_DevTotal = (sI2C_DEV_COUNT *)malloc(sizeof(sI2C_DEV_COUNT)*(ucTotal + 1));

            if((psI2C_DevRetry == NULL) || (psI2C_DevError == NULL) || (psI2C_DevTotal == NULL))
            {
                return;
            }

    		halMCUCtrl_OPD_Register_Get(eOPD_MSG_I2C_RUN_TIME_RETRY, sizeof(sI2C_DEV_COUNT)*ucTotal, (UINT8*)psI2C_DevRetry);
    		halMCUCtrl_OPD_Register_Get(eOPD_MSG_I2C_RUN_TIME_ERROR, sizeof(sI2C_DEV_COUNT)*ucTotal, (UINT8*)psI2C_DevError);
            halMCUCtrl_OPD_Register_Get(eOPD_MSG_I2C_RUN_TIME_TOTAL, sizeof(sI2C_DEV_COUNT)*ucTotal, (UINT8*)psI2C_DevTotal);

            for(ucIndex = 0; ucIndex < ucTotal; ucIndex++)
            {
                utilOPD_I2C_RunTime_Log(eDEVICE_MODULE_SYSTEM,
                                        psI2C_DevRetry[ucIndex].ucDevice,
                                        psI2C_DevRetry[ucIndex].ulCount,
                                        psI2C_DevError[ucIndex].ulCount,
                                        psI2C_DevTotal[ucIndex].ulCount,
                                    	eIF_I2C);
            }

            free(psI2C_DevRetry);
            free(psI2C_DevError);
            free(psI2C_DevTotal);
        }

		#ifndef PLATFORM_H30_4K
        //FMT lpc54113(=system slave)
        ucTotal = 0;
        eResult = halLDCtrl_OPD_Register_Get(eOPD_MSG_SLAVE_I2C_NUMBER, 1, (UINT8*)&ucTotal);

        if((eResult == rcSUCCESS) && (ucTotal != 0) && (ucTotal < 32))
        {
            sI2C_DEV_COUNT *psI2C_DevRetry = (sI2C_DEV_COUNT *)malloc(sizeof(sI2C_DEV_COUNT)*(ucTotal + 1));
            sI2C_DEV_COUNT *psI2C_DevError = (sI2C_DEV_COUNT *)malloc(sizeof(sI2C_DEV_COUNT)*(ucTotal + 1));
            sI2C_DEV_COUNT *psI2C_DevTotal = (sI2C_DEV_COUNT *)malloc(sizeof(sI2C_DEV_COUNT)*(ucTotal + 1));

    		halLDCtrl_OPD_Register_Get(eOPD_MSG_I2C_RUN_TIME_SLAVE_RETRY, sizeof(sI2C_DEV_COUNT)*ucTotal, (UINT8*)psI2C_DevRetry);
    		halLDCtrl_OPD_Register_Get(eOPD_MSG_I2C_RUN_TIME_SLAVE_ERROR, sizeof(sI2C_DEV_COUNT)*ucTotal, (UINT8*)psI2C_DevError);
            halLDCtrl_OPD_Register_Get(eOPD_MSG_I2C_RUN_TIME_SLAVE_TOTAL, sizeof(sI2C_DEV_COUNT)*ucTotal, (UINT8*)psI2C_DevTotal);

            for(ucIndex = 0; ucIndex < ucTotal; ucIndex++)
            {
                utilOPD_I2C_RunTime_Log(eDEVICE_MODULE_FMT,
                                        psI2C_DevRetry[ucIndex].ucDevice,
                                        psI2C_DevRetry[ucIndex].ulCount,
                                        psI2C_DevError[ucIndex].ulCount,
                                        psI2C_DevTotal[ucIndex].ulCount,
                                    	eIF_I2C);
            }

            free(psI2C_DevRetry);
            free(psI2C_DevError);
            free(psI2C_DevTotal);
        }
		#endif

        //FrontEnd_i2c_start.
        ucTotal = 0;
        eResult = halFrontEndCtrl_OPD_Register_Get(eOPD_MSG_I2C_NUMBER, 1, (UINT8*)&ucTotal);

        if((eResult == rcSUCCESS) && (ucTotal != 0) && (ucTotal < 32))
        {
            sI2C_DEV_COUNT *psI2C_DevRetry = (sI2C_DEV_COUNT *)malloc(sizeof(sI2C_DEV_COUNT)*(ucTotal + 1));
            sI2C_DEV_COUNT *psI2C_DevError = (sI2C_DEV_COUNT *)malloc(sizeof(sI2C_DEV_COUNT)*(ucTotal + 1));
            sI2C_DEV_COUNT *psI2C_DevTotal = (sI2C_DEV_COUNT *)malloc(sizeof(sI2C_DEV_COUNT)*(ucTotal + 1));

            if((psI2C_DevRetry == NULL) || (psI2C_DevError == NULL) || (psI2C_DevTotal == NULL))
            {
                return;
            }

    		halFrontEndCtrl_OPD_Register_Get(eOPD_MSG_I2C_RUN_TIME_RETRY, sizeof(sI2C_DEV_COUNT)*ucTotal, (UINT8*)psI2C_DevRetry);
    		halFrontEndCtrl_OPD_Register_Get(eOPD_MSG_I2C_RUN_TIME_ERROR, sizeof(sI2C_DEV_COUNT)*ucTotal, (UINT8*)psI2C_DevError);
            halFrontEndCtrl_OPD_Register_Get(eOPD_MSG_I2C_RUN_TIME_TOTAL, sizeof(sI2C_DEV_COUNT)*ucTotal, (UINT8*)psI2C_DevTotal);

            for(ucIndex = 0; ucIndex < ucTotal; ucIndex++)
            {
                utilOPD_I2C_RunTime_Log(eDEVICE_MODULE_FRONTEND,
                                        psI2C_DevRetry[ucIndex].ucDevice,
                                        psI2C_DevRetry[ucIndex].ulCount,
                                        psI2C_DevError[ucIndex].ulCount,
                                        psI2C_DevTotal[ucIndex].ulCount,
                                    	eIF_I2C);
            }

            free(psI2C_DevRetry);
            free(psI2C_DevError);
            free(psI2C_DevTotal);
        }

        for(ucIndex = 0; ucIndex < eI2C_NUMBER; ucIndex++)
        {
             //between imx6 & system, i2c retry & error & total count.
    	    ulRetryCount = dvI2C_Dev_StartUpCount_Get(eCOUNT_RETRY, ucIndex);
    	    ulErrorCount = dvI2C_Dev_StartUpCount_Get(eCOUNT_ERROR, ucIndex);
    	    ulTotalCount = dvI2C_Dev_StartUpCount_Get(eCOUNT_TOTAL, ucIndex);
    		utilOPD_I2C_RunTimeStartUp_Log((eI2C_TABLE)ucIndex, ulRetryCount, ulErrorCount, ulTotalCount, eIF_I2C);
        }
	}
}

void utilOPD_I2C_Warn_Flag_Set(eDEVICE_ERROR_TABLE ucI2CWarn) //G100_Julie_0025
{
    UINT32 ulErrorIndex = 0;
	ulErrorIndex = I2Cwarn01 + ucI2CWarn;

	if(m_aI2CWarnFlag[ucI2CWarn] == 0)
	{
	    m_aI2CWarnFlag[ucI2CWarn] = 1;

		utilDataMgr_WriteGecLog_CM(ulErrorIndex); //HICC2_Doulas_0031
	}
}

void utilOPD_I2C_Error_Flag_Set(eDEVICE_ERROR_TABLE ucI2CError) //G100_Julie_0025
{
    UINT32 ulErrorIndex = 0 ;
	ulErrorIndex = I2CError01 + ucI2CError;

	utilDataMgr_WriteGecLog_CM(ulErrorIndex); //HICC2_Doulas_0031
}

void utilOPD_Runtime_Cnt_Rst(void)
{
    dvIntfe_Dev_RuntTIme_Count_Init();
} //G100_Steven_0079

 //G100_Steven_0078 start
void utilOPD_Runtime_Flag_Set(BOOL bValue)
{
    dvI2C_Dev_RunTimeCheck(bValue);
}

void utilOPD_IST_TestResult_Record(void)
{
    UINT8 ucCount = 0xFF;
    UINT8 ucGetData[8] = {0}, eType = eIF_NUMBER;
	WORD  uiErrorData = 0, uiRetryData = 0;
    DWORD dState = 0, dWork = 0, dValue[4] = {0};
    eRESULT eResult = rcSUCCESS;

    if((utilOPD_Stress_Test_Enable_Get(eIF_I2C) == TRUE) && (utilOPD_Stress_Test_Enable_Get(eIF_UART) == TRUE))
    {
		UINT8 pcData = 1, ucDeviceCnt = 0, ucValue[32] = {0};
		UINT8 cDevice = eCM_IF_NUMBER;

		//need to check with 546x and 54113_LD.
		halMCUCtrl_OPD_Register_Set(eOPD_MSG_I2C_SLAVE_INFO, eOPD_MSG_I2C_SLAVE_INFO_SZ, &pcData);

        eType = eIF_I2C;

        //System_i2c_start.
		cDevice = utilOPD_MasterDevice_Count_Get(eIF_I2C, eI2C_STANDBY, &dValue[0]);

        if(dValue[0] != 0)
        {
			utilOPD_MasterDevice_Event(eType, cDevice, eMODE_FAIL, dValue);
        }
        else
        {
            utilOPD_MasterDevice_Event(eType, cDevice, eMODE_PASS, dValue);
        }

        eResult = halMCUCtrl_OPD_Register_Get(eOPD_MSG_I2C_NUMBER, 1, &ucDeviceCnt);

        if(eResult == rcSUCCESS && ucDeviceCnt < 32)
        {
            sI2C_DEV_STRESS sI2C_Stress;

            for(UINT8 ucSYS = 0; ucSYS < ucDeviceCnt; ucSYS++)
            {
				halMCUCtrl_OPD_Register_Get((UINT8)(eOPD_MSG_I2C_STRESS_DEVICE_1 + ucSYS), 6, (UINT8*)&sI2C_Stress);

				dValue[0] = sI2C_Stress.uiRetryCount;
				dValue[1] = sI2C_Stress.uiErrorCount;
				dValue[2] = sI2C_Stress.ucDevIndex;
				cDevice = sI2C_Stress.ucCM_DevIndex;

				if(sI2C_Stress.uiErrorCount != 0)
                {
                    utilOPD_SlaverDevice_Event(eType, eDEVICE_MODULE_SYSTEM, cDevice, eMODE_FAIL, dValue);
                }
                else
                {
                    utilOPD_SlaverDevice_Event(eType, eDEVICE_MODULE_SYSTEM, cDevice, eMODE_PASS, dValue);
                }
            }
        }

		#ifndef PLATFORM_H30_4K
        //FMT54113_i2c_start.(=system slave)
        eResult = halLDCtrl_OPD_Register_Get(eOPD_MSG_SLAVE_I2C_NUMBER, 1, &ucDeviceCnt);

        if(eResult == rcSUCCESS && ucDeviceCnt < 32)
        {
           sI2C_DEV_STRESS sI2C_Stress;

           for(UINT8 ucSYS = 0; ucSYS < ucDeviceCnt; ucSYS++)
           {
               halLDCtrl_OPD_Register_Get((UINT8)(eOPD_MSG_I2C_SLAVE_DEVICE_1 + ucSYS), 6, (UINT8*)&sI2C_Stress);

               dValue[0] = sI2C_Stress.uiRetryCount;
               dValue[1] = sI2C_Stress.uiErrorCount;
               dValue[2] = sI2C_Stress.ucDevIndex;
               cDevice = sI2C_Stress.ucCM_DevIndex;

               if(sI2C_Stress.uiErrorCount != 0)
               {
                   utilOPD_SlaverDevice_Event(eType, eDEVICE_MODULE_FMT, cDevice, eMODE_FAIL, dValue);
               }
               else
               {
                   utilOPD_SlaverDevice_Event(eType, eDEVICE_MODULE_FMT, cDevice, eMODE_PASS, dValue);
               }
           }
        }

		dValue[0] = dValue[1] = dValue[2] = dState = dWork = 0;
		ucGetData[0] = ucGetData[1] = ucGetData[2] = ucGetData[3] = 0;
		#endif

        //FrontEnd_i2c_start.
		cDevice = utilOPD_MasterDevice_Count_Get(eIF_I2C, eI2C_FRONDEND, &dValue[0]);

        if(dValue[0] != 0)
        {
			utilOPD_MasterDevice_Event(eType, cDevice, eMODE_FAIL, dValue);
        }
        else
        {
            utilOPD_MasterDevice_Event(eType, cDevice, eMODE_PASS, dValue);
        }

        eResult = halFrontEndCtrl_OPD_Register_Get(eOPD_MSG_I2C_NUMBER, 1, &ucDeviceCnt);

        if(eResult == rcSUCCESS && ucDeviceCnt < 32)
        {
            sI2C_DEV_STRESS sI2C_Stress;

            for(UINT8 ucSYS = 0; ucSYS < ucDeviceCnt; ucSYS++)
            {
				halFrontEndCtrl_OPD_Register_Get((UINT8)(eOPD_MSG_I2C_STRESS_DEVICE_1 + ucSYS), 6, (UINT8*)&sI2C_Stress);

				dValue[0] = sI2C_Stress.uiRetryCount;
				dValue[1] = sI2C_Stress.uiErrorCount;
				dValue[2] = sI2C_Stress.ucDevIndex;
				cDevice = sI2C_Stress.ucCM_DevIndex;

				if(sI2C_Stress.uiErrorCount != 0)
                {
                    utilOPD_SlaverDevice_Event(eType, eDEVICE_MODULE_FRONTEND, cDevice, eMODE_FAIL, dValue);
                }
                else
                {
                    utilOPD_SlaverDevice_Event(eType, eDEVICE_MODULE_FRONTEND, cDevice, eMODE_PASS, dValue);
                }
            }
        }

        //DDP4422_i2c_start.
		cDevice = utilOPD_MasterDevice_Count_Get(eIF_I2C, eI2C_DDP4422, &dValue[0]);

        //dValue[0] = utilOPD_I2C_Error_Count_Get();
        if(dValue[0] != 0)
        {
			utilOPD_MasterDevice_Event(eType, cDevice, eMODE_FAIL, dValue);
		}
		else
		{
			utilOPD_MasterDevice_Event(eType, cDevice, eMODE_PASS, dValue);
		}

        //halMCUCtrl_System_Error_Reset();
		halMCUCtrl_OPD_Register_Set(eOPD_MSG_EVENT_RESET, eOPD_MSG_EVENT_RESET_SZ, &ucCount);
		utilOPD_Stress_Test_Enable_Set(eIF_I2C, FALSE);

        eType = eIF_UART;

        //System_uart_start.
        cDevice = utilOPD_MasterDevice_Count_Get(eIF_UART, eUART_STANDBY, &dValue[0]);

		//LOG_MSG(db_ALWAYS,"(%s,%d) dValue[%d]\r\n", __FUNCTION__, __LINE__, dValue[0]);
        if(dValue[0] != 0)
        {
			utilOPD_MasterDevice_Event(eType, cDevice, eMODE_FAIL, dValue);
        }
        else
        {
			utilOPD_MasterDevice_Event(eType, cDevice, eMODE_PASS, dValue);
        }
        dValue[0] = 0;

        eType = eIF_SPI;

#ifdef SCALER_C821_C789
        //C821&C789_SPI_start.
        cDevice = utilOPD_MasterDevice_Count_Get(eIF_SPI, eSPI_ICHIP_2, &dValue[0]);
        if(dValue[0] != 0)
        {
			utilOPD_MasterDevice_Event(eType, cDevice, eMODE_FAIL, dValue);
        }
        else
        {
			utilOPD_MasterDevice_Event(eType, cDevice, eMODE_PASS, dValue);
        }
#endif /* SCALER_C821_C789 */
        dValue[0] = 0;
        cDevice = utilOPD_MasterDevice_Count_Get(eIF_SPI, eSPI_ICHIP_1, &dValue[0]);
        if(dValue[0] != 0)
        {
			utilOPD_MasterDevice_Event(eType, cDevice, eMODE_FAIL, dValue);
        }
        else
        {
			utilOPD_MasterDevice_Event(eType, cDevice, eMODE_PASS, dValue);
        }
        dValue[0] = 0;

        //GS2961_SPI_start.
        UINT8 ucSPIError;
		halMCUCtrl_OPD_Register_Get(eOPD_MSG_SPI_STRESS_TEST, eOPD_MSG_SPI_STRESS_TEST_SZ, &ucSPIError);

        dValue[0] = ucSPIError;

        if(dValue[0] != 0)
        {
            utilOPD_SlaverDevice_Event(eType, eDEVICE_MODULE_SYSTEM, eCM_IF_3GSDI_SWITCH, eMODE_FAIL, dValue);
			//utilOPD_MasterDevice_Event(eType, eCM_IF_3GSDI_SWITCH, eMODE_FAIL, dValue);
        }
        else
        {
            utilOPD_SlaverDevice_Event(eType, eDEVICE_MODULE_SYSTEM, eCM_IF_3GSDI_SWITCH, eMODE_PASS, dValue);
			//utilOPD_MasterDevice_Event(eType, eCM_IF_3GSDI_SWITCH, eMODE_PASS, dValue);
        }
        dValue[0] = 0;

		pcData = 0xFF;
        halMCUCtrl_OPD_Register_Get(eOPD_MSG_EVENT_RESET, eOPD_MSG_EVENT_RESET_SZ, &pcData);
		utilOPD_Stress_Test_Enable_Set(eIF_UART, FALSE);
    }
}


