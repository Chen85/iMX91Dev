#ifndef DVINTERFACE_DIA_H
#define DVINTERFACE_DIA_H
// ===============================================================================
// FILE NAME: dvInterfaceDiag.h
// DESCRIPTION:
//
//
// Modification History
// --------------------
// 2014/07/14, Leo Create
// --------------------
// ===============================================================================
#include "CommonDef.h"
#include "Board_I2C.h"


#define I2C_RUN_TIME_SIZE 5

typedef enum
{
/*000*/ eCM_IF_STANDBY,
/*001*/ eCM_IF_KEYPAD,
/*002*/ eCM_IF_FORMATTER,
/*003*/ eCM_IF_FRONTEND,
/*004*/ eCM_IF_MOTOR,
/*005*/ eCM_IF_DDP4422,
/*006*/ eCM_IF_XFPGA,
/*007*/ eCM_IF_FPGA_VIDEO,
/*008*/ eCM_IF_FPGA_POWER,
/*009*/ eCM_IF_FPGA_LD,
/*010*/ eCM_IF_CLOCK_GENERATOR, //LMK03328
/*011*/ eCM_IF_EEPROM_1,
/*012*/ eCM_IF_EEPROM_2,
/*013*/ eCM_IF_VGA_SWITCH, //AVD7604
/*014*/ eCM_IF_HDBASET_SWITCH, //VS2000
/*015*/ eCM_IF_DP_SWITCH, //IT6563
/*016*/ eCM_IF_HDMI_SWITCH_1, //IT6634
/*017*/ eCM_IF_HDMI_SWITCH_2, //IT6634
/*018*/ eCM_IF_RBG_TO_HDMI, //IT6613
/*019*/ eCM_IF_HDMI_TO_RBG_1, //IT6805
/*020*/ eCM_IF_HDMI_TO_RBG_2, //IT6805
/*021*/ eCM_IF_EDID_HDMI1,
/*022*/ eCM_IF_EDID_HDMI2,
/*023*/ eCM_IF_EDID_HDMI3,
/*024*/ eCM_IF_EDID_DP,
/*025*/ eCM_IF_EDID_DVI,
/*026*/ eCM_IF_EDID_HDBaseT,
/*027*/ eCM_IF_EDID_VGA,
/*028*/ eCM_IF_FAN_CTRL_BOARD,
/*029*/ eCM_IF_TEMPERATURE_SENSOR_1, //G751,G753,TMP411A,ADC101
/*030*/ eCM_IF_TEMPERATURE_SENSOR_2, //G751,G753,TMP411A,ADC101
/*031*/ eCM_IF_TEMPERATURE_SENSOR_3, //G751,G753,TMP411A,ADC101
/*032*/ eCM_IF_TEMPERATURE_SENSOR_4, //G751,G753,TMP411A,ADC101
/*033*/ eCM_IF_TEMPERATURE_SENSOR_5, //G751,G753,TMP411A,ADC101
/*034*/ eCM_IF_FAN_RPM_1, //MAX31790
/*035*/ eCM_IF_FAN_RPM_2, //MAX31790
/*036*/ eCM_IF_FAN_RPM_3, //MAX31790
/*037*/ eCM_IF_G_SENSOR, //ADXL345,LIS2DH12
/*038*/ eCM_IF_LVPS_MCU, //M481
/*039*/ eCM_IF_PRESSURE_THERMAL_SENSOR_1, //LPS22HH
/*040*/ eCM_IF_PRESSURE_THERMAL_SENSOR_2, //LPS22HH
/*041*/ eCM_IF_HUMIDITY_SENSOR_1,
/*042*/ eCM_IF_ADC_CONVERTER, //ADC101
/*043*/ eCM_IF_MOTOR_DRIVER_1, //DRV10983
/*044*/ eCM_IF_MOTOR_DRIVER_2, //DRV10983
/*045*/ eCM_IF_IO_EXP_1, //PCF8575
/*046*/ eCM_IF_IO_EXP_2, //PCF8575
/*047*/ eCM_IF_IO_EXP_3, //PCF8575
/*048*/ eCM_IF_ICHIP_1, //C789
/*049*/ eCM_IF_ICHIP_2, //C821
/*050*/ eCM_IF_ICHIP_3, //FPGA
/*051*/ eCM_IF_3GSDI_SWITCH, //GS2961
/*052*/ eCM_IF_LD_CONVERT, //LPC54113 LD convert
/*053*/ eCM_IF_HDMI_REDRIVER_1,
/*054*/ eCM_IF_HDMI_REDRIVER_2,
/*055*/ eCM_IF_HDMI_REDRIVER_3,
/*056*/ eCM_IF_HDMI_REDRIVER_4,
/*057*/ eCM_IF_FAN_RPM_4, //MAX31790
/*058*/ eCM_IF_HUMIDITY_SENSOR_2,
/*059*/ eCM_IF_HUMIDITY_SENSOR_3,
/*060*/ eCM_IF_PRESSURE_THERMAL_SENSOR_3, //LPS22HH
/*061*/ eCM_IF_HDMI_REDRIVER,//PI3HDX1204B1
/*062*/ eCM_IF_PMIC,//MMPF0100
/*063*/
/*064*/


    eCM_IF_NUMBER
}eCM_IF_TABLE; //common interface

//========================================== Modified by MCU module.
typedef enum
{
	eI2C_STANDBY,//LPC54605,bus3,0x78
	eI2C_FRONDEND,//LPC54605,bus1,0x72
	eI2C_MOTOR,//LPC54113//bus1,0x76
	eI2C_DDP4422,//bus1,0x34
	eI2C_FMT,
	eI2C_CLOCK_GENERATOR,
	eI2C_PMIC,//MMPF0100//bus2,0x10,
	eI2C_XFPGA,//bus3,0x??

    eI2C_NUMBER
}eI2C_TABLE;

static const eCM_IF_TABLE m_I2CDeviceLut[] =
{
	eCM_IF_STANDBY,
	eCM_IF_FRONTEND,
	eCM_IF_MOTOR,
	eCM_IF_DDP4422,
	eCM_IF_FORMATTER,
	eCM_IF_CLOCK_GENERATOR,
	eCM_IF_PMIC,
	eCM_IF_XFPGA,
};
VERIFY_SIZE_OF(m_I2CDeviceLut, sizeof(m_I2CDeviceLut[0])*eI2C_NUMBER);

typedef enum
{
	eUART_STANDBY,

    eUART_NUMBER
}eUART_TABLE;

static const eCM_IF_TABLE m_UARTDeviceLut[] =
{
	eCM_IF_STANDBY,
};
VERIFY_SIZE_OF(m_UARTDeviceLut, sizeof(m_UARTDeviceLut[0])*eUART_NUMBER);

typedef enum
{
	eSPI_ICHIP_1, //C789
	eSPI_ICHIP_2, //C821

    eSPI_NUMBER
}eSPI_TABLE;

static const eCM_IF_TABLE m_SPIDeviceLut[] =
{
#ifdef SCALER_C821_C789
	eCM_IF_ICHIP_1,
#else //FPGA
	eCM_IF_ICHIP_3,
#endif
	eCM_IF_ICHIP_2,
};
VERIFY_SIZE_OF(m_SPIDeviceLut, sizeof(m_SPIDeviceLut[0])*eSPI_NUMBER);


//========================================== Modified by MCU module.

typedef enum
{
    eCOUNT_RETRY,
    eCOUNT_ERROR,
    eCOUNT_TOTAL,

    eCOUNT_INVALID,
} eCOUNT_TYPE;

typedef enum
{
    eIF_I2C,
    eIF_UART,
    eIF_SPI,
    eIF_USB,

    eIF_NUMBER,
}eINTERFACE;

void dvI2C_Dev_RetryCount(BYTE cDevice);
void dvI2C_Dev_ErrorCount(BYTE cDevice);
void dvI2C_Dev_TotalCount(BYTE cDevice);
void dvIntfe_Dev_RuntTIme_Count_Init(void);
DWORD dvI2C_Dev_RunTimeCount_Get(BYTE ucType, BYTE cDevice);
DWORD dvI2C_Dev_StartUpCount_Get(BYTE ucType, BYTE cDevice);
void dvI2C_Dev_RunTimeCheck(BOOL bValue);
void dvSPI_Dev_RunTimeCount_Set(BYTE cDevice, BYTE cResult);
DWORD dvSPI_Dev_RunTimeCount_Get(BYTE ucType, BYTE cDevice);
void dvUART_Dev_RunTimeCount_Set(BYTE cDevice, BYTE cResult);
DWORD dvUART_Dev_RunTimeCount_Get(BYTE ucType, BYTE cDevice);


#endif /* DVINTERFACE_DIA_H */

