
#ifndef _UTIL_OPD_TEST_H_
#define _UTIL_OPD_TEST_H_


#include "Board_I2C_Dev_Table.h"
#include "Common.h"

#include "utilCommon.h"
#include "opdCtrlAPI.h"

typedef enum
{
    eMODE_STATE,
    eMODE_WORK,
    eMODE_FAIL,
    eMODE_PASS,

    eMODE_NUMBER,
}eDEVICE_MODE;

//Device module
//imx6, Lpc54113, 54605...
typedef enum
{
	eDEVICE_MODULE_LAN,         //imx6, imx8...
	eDEVICE_MODULE_SYSTEM,      //4337, 54605, 54113...
    eDEVICE_MODULE_FRONTEND,    //54605, 54113...
    eDEVICE_MODULE_MOTOR,       //54605, 54113...
    eDEVICE_MODULE_FMT,         //54605, 54113...
    eDEVICE_MODULE_KEYPAD,      //1113

    eDEVICE_MODULE_NUMBER
}eDEVICE_MODULE;

typedef enum
{
    eDEVICE_ERROR_FRONTEND,
    eDEVICE_ERROR_MOTOR,
    eDEVICE_ERROR_FMT,
    eDEVICE_ERROR_DDP4422,
    eDEVICE_ERROR_SYSTEM,
    eDEVICE_ERROR_VCXO,
    eDEVICE_ERROR_XFPGA,
    eDEVICE_ERROR_NUMBER,
}eDEVICE_ERROR_TABLE;

typedef struct
{
	eCM_IF_TABLE eICDev_List;
	char*  cICDev_string;
	UINT8  ucTemp;
}sICDEV_LIST_TABLE;

typedef struct
{
    UINT8 ucUART_Bus;
    UINT8 ucUART_Data;
} sUART_BUS_TABLE;

#pragma pack(push)  /* push current alignment to stack */
#pragma pack(1)     /* set alignment to 1 byte boundary */

typedef struct
{
    UINT32 ulCount;
    UINT8  ucDevice;

} sI2C_DEV_COUNT;

typedef struct
{
    UINT16 uiRetryCount;
    UINT16 uiErrorCount;
    UINT8  ucDevIndex;
    UINT8  ucCM_DevIndex;
} sI2C_DEV_STRESS;

#pragma pack(pop)   /* restore original alignment from stack */

void utilOPD_Stress_Test_Enable_Set(UINT8 ucType, bool bEnable);

void utilOPD_MasterDevice_Count_Reset(void);
void utilOPD_MasterDevice_Count_Set(BYTE cType, BYTE cDevice);

void utilOPD_UART_SPI_TEST(void);
void utilOPD_IST_TestResult_Record(void);
void utilOPD_UART_SPI_Test_Enable_Set(bool bEnable);
void utilOPD_UartTestCount_Set(UINT16 uiCount);
void utilOPD_I2C_RunTime_RecordEvent(UINT8 ucEvent);
void utilOPD_I2C_Warn_Flag_Set(eDEVICE_ERROR_TABLE ucI2CWarn);   //G100_Julie_0025
void utilOPD_I2C_Error_Flag_Set(eDEVICE_ERROR_TABLE ucI2CError); //G100_Julie_0025
void utilOPD_Runtime_Flag_Set(BOOL bValue);  //G100_Steven_0078
BOOL utilOPD_Runtime_Flag_Get(void);
void utilOPD_I2C_RunTimeStartUp_Log(eI2C_TABLE eI2C_dev, uint32 ulRetryCount, uint32 ulErrorCount, uint32 ulTotalCount, eINTERFACE cType);  //G100_Steven_0078
void utilOPD_Runtime_Cnt_Rst(void); //G100_Steven_0079

#endif /* _UTIL_OPD_H_ */

