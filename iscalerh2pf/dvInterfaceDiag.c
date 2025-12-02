#ifndef BOARD_I2C_DEV_TABLE
#define BOARD_I2C_DEV_TABLE
// ===============================================================================
// FILE NAME: Board_I2C_Dev_Table.h
// DESCRIPTION:
//
//
// Modification History
// --------------------
// 2014/07/14, Leo Create
// --------------------
// ===============================================================================

#include "CommonDef.h"
#include "dvInterfaceDiag.h"
#include "utilCLICmdAPI.h"

volatile static UINT8 m_bRuntimeCheck = 0;  //G100_Steven_0078

volatile static DWORD m_aI2CErrorCnt[eI2C_NUMBER]; //G100_Julie_00014
volatile static DWORD m_aI2CRetryCnt[eI2C_NUMBER]; //G100_Julie_00014
volatile static DWORD m_aI2CTotalCnt[eI2C_NUMBER]; //G100_Julie_00014

volatile static DWORD m_aI2CErrorCnt_StartUp[eI2C_NUMBER] = {0};
volatile static DWORD m_aI2CRetryCnt_StartUp[eI2C_NUMBER] = {0};
volatile static DWORD m_aI2CTotalCnt_StartUp[eI2C_NUMBER] = {0};

volatile static DWORD m_aUARTErrorCnt[eUART_NUMBER];
volatile static DWORD m_aUARTTotalCnt[eUART_NUMBER];

volatile static DWORD m_aSPIErrorCnt[eSPI_NUMBER];
volatile static DWORD m_aSPITotalCnt[eSPI_NUMBER];

int findIndex(BYTE cDevice)
{
	if(cDevice >= eCM_IF_NUMBER)
	    return LUT_INDEX_INVALID;

    for (int i = 0; i < eI2C_NUMBER; i++)
	{
        if (cDevice == m_I2CDeviceLut[i])
		{
            return i;
        }
    }
    return LUT_INDEX_INVALID;
}

void dvI2C_Dev_RetryCount(BYTE cDevice)
{

    INT8 LutIndex = findIndex(cDevice);

    if(LutIndex == LUT_INDEX_INVALID)
    {
        return;
    }

    if(m_bRuntimeCheck)
    {
        if(m_aI2CRetryCnt[LutIndex] < 0xffffffff)
        {
            m_aI2CRetryCnt[LutIndex]++;
        }
    }
    else
    {
        if(m_aI2CRetryCnt_StartUp[LutIndex] < 0xffffffff)
        {
            m_aI2CRetryCnt_StartUp[LutIndex]++;
        }
    }
}

void dvI2C_Dev_ErrorCount(BYTE cDevice)
{
    INT8 LutIndex = findIndex(cDevice);

    if(LutIndex == LUT_INDEX_INVALID)
    {
        return;
    }

    if(m_bRuntimeCheck)
    {
        if(m_aI2CErrorCnt[LutIndex] < 0xffffffff)
        {
            m_aI2CErrorCnt[LutIndex]++;
        }
    }
    else
    {
        if(m_aI2CErrorCnt_StartUp[LutIndex] < 0xffffffff)
        {
            m_aI2CErrorCnt_StartUp[LutIndex]++;
        }
    }
}

void dvI2C_Dev_TotalCount(BYTE cDevice)
{
    INT8 LutIndex = findIndex(cDevice);

    if(LutIndex == LUT_INDEX_INVALID)
    {
        return;
    }

    if(m_bRuntimeCheck)
    {
        if(m_aI2CTotalCnt[LutIndex] < 0xffffffff)
        {
            m_aI2CTotalCnt[LutIndex]++;
        }
    }
    else
    {
        if(m_aI2CTotalCnt_StartUp[LutIndex] < 0xffffffff)
        {
            m_aI2CTotalCnt_StartUp[LutIndex]++;
        }
    }
}

void dvIntfe_Dev_RuntTIme_Count_Init(void)
{
    memset((BYTE*)&m_aI2CErrorCnt, 0, sizeof(m_aI2CErrorCnt));
    memset((BYTE*)&m_aI2CRetryCnt, 0, sizeof(m_aI2CRetryCnt));
    memset((BYTE*)&m_aI2CTotalCnt, 0, sizeof(m_aI2CTotalCnt));

    memset((BYTE*)&m_aI2CErrorCnt_StartUp, 0, sizeof(m_aI2CErrorCnt_StartUp));
    memset((BYTE*)&m_aI2CRetryCnt_StartUp, 0, sizeof(m_aI2CRetryCnt_StartUp));
    memset((BYTE*)&m_aI2CTotalCnt_StartUp, 0, sizeof(m_aI2CTotalCnt_StartUp));

    memset((BYTE*)&m_aUARTErrorCnt, 0, sizeof(m_aUARTErrorCnt));
    memset((BYTE*)&m_aUARTTotalCnt, 0, sizeof(m_aUARTTotalCnt));

    memset((BYTE*)&m_aSPIErrorCnt, 0, sizeof(m_aSPIErrorCnt));
    memset((BYTE*)&m_aSPITotalCnt, 0, sizeof(m_aSPITotalCnt));
}

DWORD dvI2C_Dev_RunTimeCount_Get(BYTE ucType, BYTE cDevice)
{
    DWORD dwValue = 0;

    if(cDevice > eI2C_NUMBER)
    {
        return 0;
    }

	switch(ucType)
	{
		case eCOUNT_RETRY:
			dwValue = m_aI2CRetryCnt[cDevice];
			break;

		case eCOUNT_ERROR:
			dwValue = m_aI2CErrorCnt[cDevice];
			break;

		case eCOUNT_TOTAL:
			dwValue = m_aI2CTotalCnt[cDevice];
			break;
	};

	return dwValue;
}

DWORD dvI2C_Dev_StartUpCount_Get(BYTE ucType, BYTE cDevice)
{
    DWORD dwValue = 0;

    if(cDevice > eI2C_NUMBER)
    {
        return 0;
    }

	switch(ucType)
	{
		case eCOUNT_RETRY:
			dwValue = m_aI2CRetryCnt_StartUp[cDevice];
			break;

		case eCOUNT_ERROR:
			dwValue = m_aI2CErrorCnt_StartUp[cDevice];
			break;

		case eCOUNT_TOTAL:
			dwValue = m_aI2CTotalCnt_StartUp[cDevice];
			break;
	};

	return dwValue;
}

void dvI2C_Dev_RunTimeCheck(BOOL bValue)
{
    m_bRuntimeCheck = bValue;
}

//========================= uart ============================//A65_OPTOMA_Julie_0023, start.
void dvUART_Dev_RunTimeCount_Set(BYTE cDevice, BYTE cResult)
{
    if(cDevice >= eUART_NUMBER)  //A35G2_Simon_0070 for cppcheck
    {
        return;
    }

	if(m_aUARTTotalCnt[cDevice] < 0xffffffff)
	{
		m_aUARTTotalCnt[cDevice]++;
	}

	if(cResult == eACK_TYPE_ERROR)
	{
		if(m_aUARTErrorCnt[cDevice] < 0xffffffff)
		{
			m_aUARTErrorCnt[cDevice]++;
		}
	}
}

DWORD dvUART_Dev_RunTimeCount_Get(BYTE ucType, BYTE cDevice)
{
    DWORD dwValue = 0;

    if(cDevice > eUART_NUMBER)
    {
        return 0;
    }

	switch(ucType)
	{
		case eCOUNT_ERROR:
			dwValue = m_aUARTErrorCnt[cDevice];
			break;

		case eCOUNT_TOTAL:
			dwValue = m_aUARTTotalCnt[cDevice];
			break;
	};

	return dwValue;
}

//========================= spi ============================//
void dvSPI_Dev_RunTimeCount_Set(BYTE cDevice, BYTE cResult) //SPI_OK=1, SPI_ERROR=0
{
    if(cDevice >= eSPI_NUMBER)  //A35G2_Simon_0070 for cppcheck
    {
        return;
    }

	if(m_aSPITotalCnt[cDevice] < 0xffffffff)
	{
		m_aSPITotalCnt[cDevice]++;
	}

	if(cResult == 0)
	{
		if(m_aSPIErrorCnt[cDevice] < 0xffffffff)
		{
			m_aSPIErrorCnt[cDevice]++;
		}
	}
}

DWORD dvSPI_Dev_RunTimeCount_Get(BYTE ucType, BYTE cDevice)
{
    DWORD dwValue = 0;

    if(cDevice > eSPI_NUMBER)
    {
        return 0;
    }

	switch(ucType)
	{
		case eCOUNT_ERROR:
			dwValue = m_aSPIErrorCnt[cDevice];
			break;

		case eCOUNT_TOTAL:
			dwValue = m_aSPITotalCnt[cDevice];
			break;
	};

	return dwValue;
}
//A65_OPTOMA_Julie_0023, end.

#endif /* BOARD_I2C_DEV_TABLE */

