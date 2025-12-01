#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>
#include <stdarg.h>

#include "opdCtrlAPI.h"
#include "SystemCfgAccess.h"

#define OPD_VERSION 		"OPD ver"
#define OPD_MODEL_NAME 		"Model Name"
#define OPD_SERIAL_NUMBER 	"Serial Number"
#define OPD_TOTAL_COUNT 	"Total Count"
#define OPD_DAY_TIME        "Log DateTime"
#define OPD_LOG_BUFFER      (8*1024) //8k
#define OPD_LOG_SIZE_MAX	(256*1024)

//=============type of opd files=============
#define OPD_LOG_FILE_NUMBER 100
#define OPD_SELF_TEST_FILE_NUMBER 50 //delete data and store newest data.
#define OPD_ENGINE_FILE_NUMBER 1000
#define OPD_PROJECTOR_FILE_NUMBER 2 //light sensor calibration data.
#define OPD_RUNTIME_TEST_FILE_NUMBER 50 //i2c Retry&Error&Total count //delete data and store newest data.
#define OPD_SOURCE_FILE_NUMBER 3
#define OPD_INTERFACE_LOG_NUMBER 3 //G100_Julie_0048

#define OPD_FILE_TYPE 7 //log, self-test, engine, projector, run-time, source, interface
//=============type of opd files=============

static char m_cOPD_Buffer[OPD_LOG_BUFFER] = {'\0'};
static char m_cOPD_Test_Buffer[OPD_LOG_BUFFER] = {'\0'}; //device self test
static char m_cOPD_RunTime_Buffer[OPD_LOG_BUFFER] = {'\0'}; //i2c run time count
//static char cFEInfo_SourceName[2][24] = {'\0'}; //G100_Steven_0059
static uOPD_DATA m_sOPDDataInfo;
static UINT32 m_ulPartition[eOPD_PACKET_PROGRESS_MAX] = {'\0'};
static UINT32 g_ulTotalSize = 0;
static char   m_cOPDTemperatureName[8][36] = {'\0'};

pthread_mutex_t m_SysCallMutex;
pthread_mutex_t m_OPDMutex;

// ==============================================================================
// FUNCTION NAME: utilSystem_Call
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
// 2020/09/11, Larry Create
// --------------------
// ==============================================================================
void utilOPD_System_Call(const char *pfmtstring, ...)
{
    if(pthread_mutex_lock(&m_SysCallMutex) == 0)
    {
        char cMsg[1024];
        va_list args;

        va_start(args, pfmtstring);
        vsnprintf(cMsg, 1024, pfmtstring, args);

    (void)system(cMsg);

        va_end(args);

        pthread_mutex_unlock(&m_SysCallMutex);
    }
}

#define OPD_SYSTEM_CALL(fmt, ...)   utilOPD_System_Call(fmt, ##__VA_ARGS__)

static BOOL utilOPD_MutexGive(void)
{
    if(pthread_mutex_unlock(&m_OPDMutex) != 0)
    {
        return FALSE;
    }

    return TRUE;
}

static BOOL utilOPD_MutexTake(void)
{
	if (pthread_mutex_lock(&m_OPDMutex) != 0)
    {
        return FALSE;
    }

    return TRUE;
}

const char *m_pcOPD_LOG[] =
{
	/* 00 */ "Error",
	/* 01 */ "Warning",
	/* 02 */ "Snapshot",
	/* 03 */ "Engine",
	/* 04 */ "Projector",
	/* 05 */ "Device_Interface",
	/* 06 */ "Key_Keypad",
	/* 07 */ "Key_IR",
	/* 08 */ "Key_Lan",
	/* 09 */ "Key_HDBaseT",
	/* 10 */ "Key_CLI",
	/* 11 */ "CLI_RS232",
	/* 12 */ "CLI_Telnet",
	/* 13 */ "CLI_HDBaseT",
	/* 14 */ "Telnet",
	/* 15 */ "GUI_Access",
	/* 16 */ "LAN_Access",
	/* 17 */ "Source",
	/* 18 */ "AVI_InfoFrame",
	/* 19 */ "Front_End", //G100_Steven_0059
	/* 20 */ "Run_Time",
	/* 21 */ "System_Status",
	/* 22 */ "FW_Version",
	/* 23 */ "Interface_Info", //G100_Steven_0089
	/* 24 */ "Power_Ready",
	/* 25 */ "Power_Off",
	/* 26 */ "Warning_Up",
	/* 27 */ "Shutter_On",
	/* 28 */ "Shutter_Off",
	/* 29 */ "High_Attitude_Off",
	/* 30 */ "High_Attitude_On",
	/* 31 */ "Input_Detect",
	/* 32 */ "Mutex_Locked",
	/* 33 */ "Task_No_Response",
	/* 34 */ "Datacode",
	/* 35 */ "FPGA_HW_ID",
	/* 36 */ "Voltage_Detect",
	/* 37 */ "Change_Picture_Mode",
	/* 38 */ "Change_Light_Source",
	/* 39 */ "Light_Sensor_Calibration",
	/* 40 */ "Celing_Mount_Off",
	/* 41 */ "Celing_Mount_On",
	/* 42 */ "Celing_Mount_Auto",
	/* 43 */ "Key_Wired",
	/* 44 */ "Geometry",
	/* 45 */ "Key_Back",
    /* 46 */ "Lens_Calibration",
    /* 47 */ "Lens_Memory",
    /* 48 */ "Lens_Optical_Center",
    /* 49 */ "Light Sensor Error",
    /* 50 */ "MCU System State",
    /* 51 */ "HDBT Status",
    /* 52 */ "Infoframe Change",
    /* 53 */ "Source Lost",
    /* 54 */ "Debug"

};

VERIFY_SIZE_OF(m_pcOPD_LOG, sizeof(m_pcOPD_LOG[0])*eOPD_NUMBERS);

const char *m_sPicture_Setting_String[] =
{
	/* 00 */ "eCM_PICTURE_SETTINGS_PRESENTATION",
	/* 01 */ "eCM_PICTURE_SETTINGS_VIDEO",
	/* 02 */ "eCM_PICTURE_SETTINGS_BRIGHT",
	/* 03 */ "eCM_PICTURE_SETTINGS_ENHANCED",
	/* 04 */ "eCM_PICTURE_SETTINGS_REC709",
	/* 05 */ "eCM_PICTURE_SETTINGS_REAL",
	/* 06 */ "eCM_PICTURE_SETTINGS_DICOMSIM",
	/* 07 */ "eCM_PICTURE_SETTINGS_2DHIGHSPEED",
	/* 08 */ "eCM_PICTURE_SETTINGS_3D",
	/* 09 */ "eCM_PICTURE_SETTINGS_BLENDING",
	/* 10 */ "eCM_PICTURE_SETTINGS_USER",
	/* 11 */ "eCM_PICTURE_SETTINGS_HDR",
	/* 12 */ "eCM_PICTURE_SETTINGS_SRGB",
	/* 13 */ "eCM_PICTURE_SETTINGS_SUPER_BRIGHT",
	/* 14 */ "eCM_PICTURE_SETTINGS_SUPER_RED",
	/* 15 */ "eCM_PICTURE_SETTINGS_3D_PASSIVE",
    /* 16 */ "eCM_PICTURE_SETTINGS_HLG",
};

VERIFY_SIZE_OF(m_sPicture_Setting_String, sizeof(m_sPicture_Setting_String[0])*eCM_PICTURE_SETTINGS_NUMBER);

const char *m_sUSER_Picture_Setting_String[] =
{
	/* 00 */ "NOT_SUPPORT",
	/* 01 */ "eCM_USER_PICTURE_SETTINGS_PRESENTATION",
	/* 02 */ "eCM_USER_PICTURE_SETTINGS_VIDEO",
	/* 03 */ "eCM_USER_PICTURE_SETTINGS_BRIGHT",
	/* 04 */ "eCM_USER_PICTURE_SETTINGS_ENHANCED",
	/* 05 */ "eCM_USER_PICTURE_SETTINGS_REC709",
	/* 06 */ "eCM_USER_PICTURE_SETTINGS_REAL",
	/* 07 */ "eCM_USER_PICTURE_SETTINGS_DICOMSIM",
	/* 08 */ "eCM_USER_PICTURE_SETTINGS_2DHIGHSPEED",
	/* 09 */ "eCM_USER_PICTURE_SETTINGS_3D",
	/* 10 */ "eCM_USER_PICTURE_SETTINGS_BLENDING",
	/* 11 */ "eCM_USER_PICTURE_SETTINGS_USER",
	/* 12 */ "eCM_USER_PICTURE_SETTINGS_HDR",
	/* 13 */ "eCM_USER_PICTURE_SETTINGS_SRGB",
	/* 14 */ "eCM_USER_PICTURE_SETTINGS_SUPER_BRIGHT",
	/* 15 */ "eCM_USER_PICTURE_SETTINGS_SUPER_RED",
	/* 16 */ "eCM_USER_PICTURE_SETTINGS_3D_PASSIVE",
    /* 17 */ "eCM_USER_PICTURE_SETTINGS_HLG",
};

VERIFY_SIZE_OF(m_sUSER_Picture_Setting_String, sizeof(m_sUSER_Picture_Setting_String[0])*(eCM_PICTURE_SETTINGS_NUMBER + 1));

const char *m_sSource_String[] =
{
	/* 00 */ "RESERVED",
	/* 01 */ "VGA",
	/* 02 */ "BNC",
	/* 03 */ "HDMI-1",
	/* 04 */ "HDMI-2",
	/* 05 */ "DVI-D",
	/* 06 */ "DP",
	/* 07 */ "3G-SDI",
	/* 08 */ "HDBaseT",
	/* 09 */ "COMPOSITE",
	/* 10 */ "PRESENTER",
	/* 11 */ "CARDREADER",
	/* 12 */ "MINIUSB",
	/* 13 */ "SLOT1",
	/* 14 */ "SLOT2",

};

VERIFY_SIZE_OF(m_sSource_String, sizeof(m_sSource_String[0])*eCM_SOURCE_NUMBER);

const char *m_sPower_Mode_String[] =
{
    /* 00 */ "eCM_POWER_MODE_CONSTANT_POWER",
    /* 01 */ "eCM_POWER_MODE_CONSTANT_INTENSITY",
    /* 02 */ "eCM_POWER_MODE_ECO1",
    /* 03 */ "eCM_POWER_MODE_ECO2",
    /* 04 */ "eCM_POWER_MODE_RENTAL_MODE",
    /* 05 */ "eCM_POWER_MODE_QUIET_MODE",
    /* 06 */ "eCM_POWER_MODE_CUSTOM_MODE",
    /* 07 */ "eCM_POWER_MODE_LOW_BRIGHTNESS_MODE",
    /* 08 */ "eCM_POWER_MODE_RESERVE_01",
    /* 09 */ "eCM_POWER_MODE_RESERVE_02",
    /* 10 */ "eCM_POWER_MODE_100",
    /* 11 */ "eCM_POWER_MODE_50",
    /* 12 */ "eCM_POWER_MODE_30",
    /* 13 */ "eCM_POWER_MODE_20",

};

VERIFY_SIZE_OF(m_sPower_Mode_String, sizeof(m_sPower_Mode_String[0])*eCM_POWER_MODE_NUMBER);

const char *m_sToggle_String[] =
{
    "Off",
    "On",
    "Off",  //CUSTOM_OPTOMA

};

const sOPD_PASSWORD_LUT_TABLE m_sOPD_Password_Table[] =
{
/*BARCORE*/	{"23456789",  "23456789",  "23456789",  "23456789",  "23456789",  "23456789",  "23456789",  "23456789",  "23456789"},
};

VERIFY_SIZE_OF(m_sOPD_Password_Table, sizeof(m_sOPD_Password_Table[0])*eCUSTOMER_SPLASH_INVALID);

int file_size(const char* filename)
{
	int size;

	FILE *fp = fopen(filename,"r");

	if(!fp)
	{
		return -1;
	}

	fseek(fp, 0, SEEK_END);
	size=ftell(fp);
	fclose(fp);

	return size;
}

void utilOPD_CleanBuffer(void)
{
    if(utilOPD_MutexTake())
    {
        memset(m_cOPD_Buffer, '\0', OPD_LOG_BUFFER);

        utilOPD_MutexGive();
    }
}

void utilOPD_Test_CleanBuffer(void)
{
    if(utilOPD_MutexTake())
    {
        memset(m_cOPD_Test_Buffer, '\0', OPD_LOG_BUFFER);

        utilOPD_MutexGive();
    }
}

void utilOPD_RunTime_CleanBuffer(void)
{
    if(utilOPD_MutexTake())
    {
        memset(m_cOPD_RunTime_Buffer, '\0', OPD_LOG_BUFFER);

        utilOPD_MutexGive();
    }
}

void utilOPD_RegulatoryInfo_Set(eDATA_CODE eDataCode, char *pStr, UINT8 cValue)
{
	switch(eDataCode)
	{
		case edcMODEL_NAME:
			sprintf((char*)m_sOPDDataInfo.sRegulatoryInfo.cModelName, "");
			if(pStr == NULL)
			{
				snprintf(m_sOPDDataInfo.sRegulatoryInfo.cModelName, 31, "None");
			}
			else
			{
				snprintf(m_sOPDDataInfo.sRegulatoryInfo.cModelName, 31, "%s", pStr);
			}
			break;

		case edcSERIAL_NUMBER:
			sprintf((char*)m_sOPDDataInfo.sRegulatoryInfo.cSN, "");
			if(pStr == NULL)
			{
				snprintf(m_sOPDDataInfo.sRegulatoryInfo.cSN, 31, "None");
			}
			else
			{
				snprintf(m_sOPDDataInfo.sRegulatoryInfo.cSN, 31, "%s", pStr);
			}
			break;

		case edcLAN_MAC_ADDRESS:
			sprintf((char*)m_sOPDDataInfo.sRegulatoryInfo.cLanMacAdd, "");
			if(pStr == NULL)
			{
				snprintf(m_sOPDDataInfo.sRegulatoryInfo.cLanMacAdd, 31, "None");
			}
			else
			{
				snprintf(m_sOPDDataInfo.sRegulatoryInfo.cLanMacAdd, 31, "%s", pStr);
			}
			break;

		case edcMAIN_INPUT: //A65_OPTOMA_Julie_0081
			m_sOPDDataInfo.sRegulatoryInfo.cInputSource[0] = eCM_SOURCE_VGA;
			if(cValue < eCM_SOURCE_NUMBER)
			{
				m_sOPDDataInfo.sRegulatoryInfo.cInputSource[0] = cValue;
			}
			break;

		case edcPICTURE_SETTINGS: //A65_OPTOMA_Julie_0081
			m_sOPDDataInfo.sRegulatoryInfo.cPictureMode[m_sOPDDataInfo.sRegulatoryInfo.cInputSource[0]] = eCM_PICTURE_SETTINGS_PRESENTATION;
			if(cValue < eCM_PICTURE_SETTINGS_NUMBER)
			{
				m_sOPDDataInfo.sRegulatoryInfo.cPictureMode[m_sOPDDataInfo.sRegulatoryInfo.cInputSource[0]]  = cValue;
			}
			break;

		default:
			break;
	}
}

int utilOPD_UartTotalCount_Set(UINT16 uiCount)
{
	m_sOPDDataInfo.dCount = uiCount;
}

int utilOPD_TotalProjector_Set(UINT32 ulTotalProjectorSec)
{
    m_sOPDDataInfo.sRegulatoryInfo.wTPSec  = ulTotalProjectorSec % 60;
	ulTotalProjectorSec /= 60;
    m_sOPDDataInfo.sRegulatoryInfo.wTPMin  = ulTotalProjectorSec % 60;
	ulTotalProjectorSec /= 60;
    m_sOPDDataInfo.sRegulatoryInfo.wTPHour = ulTotalProjectorSec % 24;
    m_sOPDDataInfo.sRegulatoryInfo.wTPDay  = ulTotalProjectorSec/24;

	return 1;
}

int utilOPD_TotalProjector_Get(UINT16 *wDay, UINT16 *wHour, UINT16 *wMin, UINT16 *wSec)
{

	*wSec  = m_sOPDDataInfo.sRegulatoryInfo.wTPSec;
	*wMin  = m_sOPDDataInfo.sRegulatoryInfo.wTPMin;
	*wHour = m_sOPDDataInfo.sRegulatoryInfo.wTPHour;
	*wDay  = m_sOPDDataInfo.sRegulatoryInfo.wTPDay;

	return 1;
}

int utilOPD_RTCtimer_Get(UINT16 *wtyear, UINT16 *wtmon, UINT16 *wtday, UINT16 *wthour, UINT16 *wtmin, UINT16 *wtsec)
{
	time_t timer;//time_t就是long int 類型
	struct tm *tblock;
	timer = time(NULL);//這一句也可以改成time(&timer);
	tblock = localtime(&timer);

	*wtyear  = tblock->tm_year + 1900;
	*wtmon   = tblock->tm_mon + 1;
	*wtday   = tblock->tm_mday;
	*wthour  = tblock->tm_hour;
	*wtmin   = tblock->tm_min;
	*wtsec   = tblock->tm_sec;

	return 1;
}

int utilOPD_Write2Buffer(UINT8 ucEvent, char *pcStr)
{
    char cStrTemp[1024] = {'\0'};
    UINT16 wDay = 0, wHour = 0, wMin = 0, wSec = 0;
	UINT16 wtyear = 0, wtmon = 0, wtday = 0, wthour = 0, wtmin = 0, wtsec = 0;

	utilOPD_TotalProjector_Get(&wDay, &wHour, &wMin, &wSec);
	utilOPD_RTCtimer_Get(&wtyear, &wtmon, &wtday, &wthour, &wtmin, &wtsec);

    if(utilOPD_MutexTake())
    {
	    snprintf(cStrTemp, 1023 ,"%s,%04d/%02d/%02d-%02d:%02d:%03d,%04d:%02d:%02d:%02d,%s\n", m_pcOPD_LOG[ucEvent], wtyear, wtmon, wtday, wthour, wtmin, wtsec, wDay, wHour, wMin, wSec, pcStr);

        if(strlen(m_cOPD_Buffer) + strlen(cStrTemp) > OPD_LOG_BUFFER)
        {
            utilOPD_MutexGive();
            return 0;
        }

        strncat(m_cOPD_Buffer, cStrTemp, strlen(cStrTemp));

        utilOPD_MutexGive();
    }
    return 1;
}

int utilOPD_Write2TestBuffer(char *pcOPDType, char *pcStr) //device self test result write to buffer
{
    char cStrTemp[1024] = {'\0'};
    UINT16 wDay = 0, wHour = 0, wMin = 0, wSec = 0;
	UINT16 wtyear = 0, wtmon = 0, wtday = 0, wthour = 0, wtmin = 0, wtsec = 0;

	utilOPD_TotalProjector_Get(&wDay, &wHour, &wMin, &wSec);
	utilOPD_RTCtimer_Get(&wtyear, &wtmon, &wtday, &wthour, &wtmin, &wtsec);

    if(utilOPD_MutexTake())
    {
	    snprintf(cStrTemp, 1023 ,"%s,%04d/%02d/%02d-%02d:%02d:%03d,%04d:%02d:%02d:%02d,%s\n", pcOPDType, wtyear, wtmon, wtday, wthour, wtmin, wtsec, wDay, wHour, wMin, wSec, pcStr);

        if(strlen(m_cOPD_Test_Buffer) + strlen(cStrTemp) > OPD_LOG_BUFFER)
        {
            utilOPD_MutexGive();
            return 0;
        }

        strncat(m_cOPD_Test_Buffer, cStrTemp, strlen(cStrTemp));

        utilOPD_MutexGive();
    }
    return 1;
}

int utilOPD_Write2RunTimeBuffer(char *cInterfaceType, char *pcStr)
{
    char cStrTemp[1024] = {'\0'};
    UINT16 wDay = 0, wHour = 0, wMin = 0, wSec = 0;
	UINT16 wtyear = 0, wtmon = 0, wtday = 0, wthour = 0, wtmin = 0, wtsec = 0;

	utilOPD_TotalProjector_Get(&wDay, &wHour, &wMin, &wSec);
	utilOPD_RTCtimer_Get(&wtyear, &wtmon, &wtday, &wthour, &wtmin, &wtsec);

    if(utilOPD_MutexTake())
    {
	    snprintf(cStrTemp, 1023 ,"%s,%04d/%02d/%02d-%02d:%02d:%03d,%04d:%02d:%02d:%02d,%s\n", cInterfaceType, wtyear, wtmon, wtday, wthour, wtmin, wtsec, wDay, wHour, wMin, wSec, pcStr);

        if(strlen(m_cOPD_RunTime_Buffer) + strlen(cStrTemp) > OPD_LOG_BUFFER)
        {
            utilOPD_MutexGive();
            return 0;
        }

        strncat(m_cOPD_RunTime_Buffer, cStrTemp, strlen(cStrTemp));

        utilOPD_MutexGive();
    }
    return 1;
}

int utilOPD_Read_TestFile(void)
{
	FILE *pFile = NULL;
	int size = 0;
    int cFile = 0;
    char cStrTemp[OPD_LOG_BUFFER] = {'\0'};
    char cType[64] = {'\0'};
    char cString[1024] = {'\0'}, cTemp[1024] = {'\0'};

    if(utilOPD_MutexTake())
    {

	    uOPD_DATA sDev = {0};
		sprintf(cTemp, "%s/opd_self_test_%04d.bin", OPD_LOG_PATH, 0);

	    size = file_size(cTemp);

	    if(access(cTemp,0)==-1)
	    {
	        utilOPD_MutexGive();
	        return 1;
	    }

	    printf("===================================\n");
	    printf("Open file %s\n", cTemp);
	    printf("===================================\n");

	    pFile = fopen(cTemp, "r");

	    if(pFile == NULL)  //G100_Simon_0092
	    {
    	    printf("file open fail %s\n", cTemp);
	        utilOPD_MutexGive();
	        return 1;
	    }

            while (!feof(pFile))
            {
                if (fgets(cStrTemp,OPD_LOG_BUFFER,pFile) == NULL)
                {
                    break;
                }

                        if(feof(pFile))
                        {
                                break;//return 1;
                        }

	        sscanf(cStrTemp, "%63[^,],%1023[^\n]", cType, cString); //G100_Simon_0060

	        if((strncmp(cType, "I2C", 3) == 0) || (strncmp(cType, "SPI", 3) == 0) || (strncmp(cType, "UART", 4) == 0)) //A65_OPTOMA_Julie_0037
	        {
	            sscanf(cStrTemp, "%31[^,],%*[^,],%*[^,],%31[^,],%31[^,],%d,%d,%d,%*d,%f", sDev.sDev.cDevInterface, sDev.sDev.cDevName, sDev.sDev.cDevInfo, &sDev.sDev.cPara1, &sDev.sDev.cPara2, &sDev.sDev.cPara3, &sDev.sDev.fPara5);  //G100_Simon_0060

	            printf("Interface: %4s, ", sDev.sDev.cDevInterface);
	            printf("Dev: %-25s, ", sDev.sDev.cDevName);
	            printf("Result: %4s, ", sDev.sDev.cDevInfo);
	            printf("Retry Count: %5d, ", sDev.sDev.cPara1);
				printf("Error Count: %5d, ", sDev.sDev.cPara2);
				printf("I2C Status: %3d, ", sDev.sDev.cPara3);
	            printf("Error Rate: %02.2f%%\n", sDev.sDev.fPara5);

    			//MS_SLEEP(10);
    			usleep(10);
	        }
	        else if(strncmp(cType, "Model Name", 10) == 0)
	        {
	            sscanf(cStrTemp, "%*[^,],%1023[^\n]", cString);  //G100_Simon_0060
	            printf("Model Name:    %s\n", cString);
	        }
	        else if(strncmp(cType, "Serial Number", 13) == 0)
	        {
	            sscanf(cStrTemp, "%*[^,],%1023[^\n]", cString);  //G100_Simon_0060
				printf("Serial Number: %s\n", cString);
	        }
			else if(strncmp(cType, "Total Count", 11) == 0)
			{
	            sscanf(cStrTemp, "%*[^,],%1023[^\n]", cString);  //G100_Simon_0060
				printf("Total Count: %s\n", cString);
			}
			else if(strncmp(cType, "Log DateTime", 12) == 0)
			{
	            sscanf(cStrTemp, "%*[^,],%1023[^\n]", cString);  //G100_Simon_0060
				printf("Log DateTime: %s\n", cString);
			}
	    }

	    fclose(pFile);

        utilOPD_MutexGive();
    }

	return 0;
}

int utilOPD_Write2File(void)
{
	FILE *pFile = NULL;
	int size = 0;
	char cTemp[1024] = {'\0'}, cTemp2[1024] = {'\0'};

    if(utilOPD_MutexTake())
    {
		//printf("(%s, %d)\n", __FUNCTION__, __LINE__);
        if(strlen(m_cOPD_Buffer) == 0)
        {
            utilOPD_MutexGive();
            return 0;
        }

	    sprintf(cTemp, "%s/opd_log_%04d.bin", OPD_LOG_PATH, 0);

    	size = file_size(cTemp);

    	if(size > OPD_LOG_SIZE_MAX)
    	{
            int cFile = OPD_LOG_FILE_NUMBER - 1;

            sprintf(cTemp, "%s/opd_log_%04d.bin", OPD_LOG_PATH, cFile);

            if(access(cTemp, 0) == 0)
            {
                if(remove(cTemp) == 0)
                {
                    //printf("Remove %s pass\n", m_pcOPD_File[cFile]);
                }
            }

            for(cFile; cFile > 0; cFile--)
            {
                sprintf(cTemp, "%s/opd_log_%04d.bin", OPD_LOG_PATH, cFile);
		        sprintf(cTemp2, "%s/opd_log_%04d.bin", OPD_LOG_PATH, cFile-1);

                if(access(cTemp2,0) == 0)
        		{
        			if(rename(cTemp2, cTemp) == 0)
                    {
                        //printf("Rename %s pass.\n", m_pcOPD_File[cFile]);
                    }
                    else
                    {
                        //printf("Rename %s fail.\n", m_pcOPD_File[cFile]);
                    }

                    if(remove(cTemp2) == 0)
        			{
        				//printf("Remove %s pass\n", m_pcOPD_File[cFile - 1]);
        			}
        		}
            }
    	}

	    sprintf(cTemp, "%s/opd_log_%04d.bin", OPD_LOG_PATH, 0);

    	if(access(cTemp,0) == -1)
    	{
			UINT16 wtyear = 0, wtmon = 0, wtday = 0, wthour = 0, wtmin = 0, wtsec = 0;

			utilOPD_RTCtimer_Get(&wtyear, &wtmon, &wtday, &wthour, &wtmin, &wtsec);

			pFile = fopen(cTemp, "w+");

			if(pFile != NULL)  //G100_Simon_0092
			{
				fprintf(pFile, "%s,%s,%s,V%02d.%02d\n", OPD_MODEL_NAME, m_sOPDDataInfo.sRegulatoryInfo.cModelName, OPD_VERSION, OPD_VER_MAJOR, OPD_VER_MINOR);
				fprintf(pFile, "%s,%s\n", OPD_SERIAL_NUMBER, m_sOPDDataInfo.sRegulatoryInfo.cSN);
				fprintf(pFile, "%s,%04d/%02d/%02d-%02d:%02d:%03d\n", OPD_DAY_TIME, wtyear, wtmon, wtday, wthour, wtmin, wtsec);

    			fprintf(pFile, "%s,%s,%s,%s\n", "OPD type","DateTime (Y/M/D-H:M:S)","TotalProjector (D:H:M:S)","Data");
                fclose(pFile);
            }
            else
            {
                printf("file open fail %s\n", cTemp);
            }
    	}

    	pFile = fopen(cTemp, "a+");

        if(pFile != NULL)  //G100_Simon_0092
        {
        	fprintf(pFile, "%s", m_cOPD_Buffer);
           	fclose(pFile);
        }
        else
        {
            printf("file open fail %s\n", cTemp);
        }

        utilOPD_MutexGive();
    }

    utilOPD_CleanBuffer();

    return 1;

}

int utilOPD_Write2TestFile(void)
{
	FILE *pFile = NULL;
	int size = 0;
	UINT16 uiData = 0;
	char cTemp[1024] = {'\0'}, cTemp2[1024] = {'\0'};

    if(utilOPD_MutexTake())
    {
        if(strlen(m_cOPD_Test_Buffer) == 0)
        {
            utilOPD_MutexGive();
            return 0;
        }

		sprintf(cTemp, "%s/opd_self_test_%04d.bin", OPD_LOG_PATH, 0);
    	size = file_size(cTemp);

		if(size > 0)
		{
			int cFile = OPD_SELF_TEST_FILE_NUMBER - 1;

			sprintf(cTemp, "%s/opd_self_test_%04d.bin", OPD_LOG_PATH, cFile);

			if(access(cTemp, 0) == 0)
			{
				if(remove(cTemp) == 0)
                {
                    //printf("Remove %s pass\n", m_pcOPD_File[cFile]);
                }
            }

            for(cFile; cFile > 0; cFile--)
            {
			    sprintf(cTemp, "%s/opd_self_test_%04d.bin", OPD_LOG_PATH, cFile);
				sprintf(cTemp2, "%s/opd_self_test_%04d.bin", OPD_LOG_PATH, cFile-1);

				if(access(cTemp2,0) == 0)
				{
					if(rename(cTemp2, cTemp) == 0)
                    {
                        //printf("Rename %s pass.\n", m_pcOPD_File[cFile]);
                    }
                    else
                    {
                        //printf("Rename %s fail.\n", m_pcOPD_File[cFile]);
                    }

					if(remove(cTemp2) == 0)
        			{
        				//printf("Remove %s pass\n", m_pcOPD_File[cFile - 1]);
        			}
        		}
            }
    	}

		sprintf(cTemp, "%s/opd_self_test_%04d.bin", OPD_LOG_PATH, 0);

		if(access(cTemp,0) == -1)
    	{
			UINT16 wtyear = 0, wtmon = 0, wtday = 0, wthour = 0, wtmin = 0, wtsec = 0;

			utilOPD_RTCtimer_Get(&wtyear, &wtmon, &wtday, &wthour, &wtmin, &wtsec);

			pFile = fopen(cTemp, "w+");

            if(pFile != NULL)  //G100_Simon_0092
            {
				fprintf(pFile, "%s,%s,%s,V%02d.%02d\n", OPD_MODEL_NAME, m_sOPDDataInfo.sRegulatoryInfo.cModelName, OPD_VERSION, OPD_VER_MAJOR, OPD_VER_MINOR);
				fprintf(pFile, "%s,%s\n", OPD_SERIAL_NUMBER, m_sOPDDataInfo.sRegulatoryInfo.cSN);
				fprintf(pFile, "%s,%04d/%02d/%02d-%02d:%02d:%03d\n", OPD_DAY_TIME, wtyear, wtmon, wtday, wthour, wtmin, wtsec);

                uiData = m_sOPDDataInfo.dCount;
        		fprintf(pFile, "%s,%d\n", OPD_TOTAL_COUNT, uiData);

    			fprintf(pFile, "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n", "OPD type","DateTime (Y/M/D-H:M:S)","TotalProjector (D:H:M:S)","IC name","Result","retry count","error count","i2c status","null","error rate");
                fclose(pFile);
            }
            else
            {
                printf("file open fail %s\n", cTemp);
            }
    	}

		pFile = fopen(cTemp, "a+");

        if(pFile != NULL)  //G100_Simon_0092
        {
        	fprintf(pFile, "%s", m_cOPD_Test_Buffer);
        	fclose(pFile);
        }
        else
        {
            printf("file open fail %s\n", cTemp);
        }

        utilOPD_MutexGive();
    }

    utilOPD_Test_CleanBuffer();

    utilOPD_Read_TestFile();

    return 1;

}

int utilOPD_Write2EngineFile(UINT8 ucEvent, char *pcStr)
{
	FILE *pFile = NULL;
	int size = 0;
	char cTemp[1024] = {'\0'}, cTemp2[1024] = {'\0'}, cStrTemp[1024] = {'\0'};
	UINT16 wDay = 0, wHour = 0, wMin = 0, wSec = 0;
	UINT16 wtyear = 0, wtmon = 0, wtday = 0, wthour = 0, wtmin = 0, wtsec = 0;

	UINT8 ucIndex = eCM_SOURCE_RESERVED, ucSetting = eCM_PICTURE_SETTINGS_BRIGHT;

    if(utilOPD_MutexTake())
    {

		utilOPD_TotalProjector_Get(&wDay, &wHour, &wMin, &wSec);
		utilOPD_RTCtimer_Get(&wtyear, &wtmon, &wtday, &wthour, &wtmin, &wtsec);

#if 0
		if(ucEvent == eOPD_ENGINE_LOG)
		{
			ucIndex = m_sOPDDataInfo.sRegulatoryInfo.cInputSource[0];
			ucSetting = m_sOPDDataInfo.sRegulatoryInfo.cPictureMode[ucIndex];
			snprintf(cStrTemp, 1023 ,"%s,%04d/%02d/%02d-%02d:%02d:%03d,%04d:%02d:%02d:%02d,%s\n", m_sPicture_Setting_String[ucSetting], wtyear, wtmon, wtday, wthour, wtmin, wtsec, wDay, wHour, wMin, wSec, pcStr);
		}
		else
#endif
		{
			snprintf(cStrTemp, 1023 ,"%s,%04d/%02d/%02d-%02d:%02d:%03d,%04d:%02d:%02d:%02d,%s\n", m_pcOPD_LOG[ucEvent], wtyear, wtmon, wtday, wthour, wtmin, wtsec, wDay, wHour, wMin, wSec, pcStr);
		}


	    if(strlen(cStrTemp) == 0)
	    {
	        utilOPD_MutexGive();
	        return 0;
	    }

	    sprintf(cTemp, "%s/opd_engine_%04d.bin", OPD_LOG_PATH, 0);

		size = file_size(cTemp);

		if(size > OPD_LOG_SIZE_MAX)
		{
	        int cFile = OPD_ENGINE_FILE_NUMBER - 1;

	        sprintf(cTemp, "%s/opd_engine_%04d.bin", OPD_LOG_PATH, cFile);

	        if(access(cTemp, 0) == 0)
	        {
	            if(remove(cTemp) == 0)
	            {
	                //printf("Remove %s pass\n", m_pcOPD_File[cFile]);
	            }
	        }

	        for(cFile; cFile > 0; cFile--)
	        {
	            sprintf(cTemp, "%s/opd_engine_%04d.bin", OPD_LOG_PATH, cFile);
		        sprintf(cTemp2, "%s/opd_engine_%04d.bin", OPD_LOG_PATH, cFile-1);

	            if(access(cTemp2,0) == 0)
	    		{
	    			if(rename(cTemp2, cTemp) == 0)
	                {
	                    //printf("Rename %s pass.\n", m_pcOPD_File[cFile]);
	                }
	                else
	                {
	                    //printf("Rename %s fail.\n", m_pcOPD_File[cFile]);
	                }

	                if(remove(cTemp2) == 0)
	    			{
	    				//printf("Remove %s pass\n", m_pcOPD_File[cFile - 1]);
	    			}
	    		}
	        }
		}

	    sprintf(cTemp, "%s/opd_engine_%04d.bin", OPD_LOG_PATH, 0);

		if(access(cTemp,0) == -1)
		{
			UINT16 wtyear = 0, wtmon = 0, wtday = 0, wthour = 0, wtmin = 0, wtsec = 0;

			utilOPD_RTCtimer_Get(&wtyear, &wtmon, &wtday, &wthour, &wtmin, &wtsec);

			pFile = fopen(cTemp, "w+");

			if(pFile != NULL)  //G100_Simon_0092
			{
				fprintf(pFile, "%s,%s,%s,V%02d.%02d\n", OPD_MODEL_NAME, m_sOPDDataInfo.sRegulatoryInfo.cModelName, OPD_VERSION, OPD_VER_MAJOR, OPD_VER_MINOR);
				fprintf(pFile, "%s,%s\n", OPD_SERIAL_NUMBER, m_sOPDDataInfo.sRegulatoryInfo.cSN);
				fprintf(pFile, "%s,%04d/%02d/%02d-%02d:%02d:%03d\n", OPD_DAY_TIME, wtyear, wtmon, wtday, wthour, wtmin, wtsec);

    			fprintf(pFile, "%s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s\n", //[%s]count=176

    				"OPD type","DateTime (Y/M/D-H:M:S)","TotalProjector (D:H:M:S)","LDMinute",

    				"FanSpeed1","FanSpeed2","FanSpeed3","FanSpeed4","FanSpeed5","FanSpeed6","FanSpeed7","FanSpeed8",
    				"FanSpeed9","FanSpeed10","FanSpeed11","FanSpeed12","FanSpeed13","FanSpeed14","FanSpeed15","FanSpeed16",
    				"FanSpeed17","FanSpeed18","FanSpeed19","FanSpeed20","FanSpeed21","FanSpeed22","FanSpeed23","FanSpeed24",

    				"FanPWM1","FanPWM2","FanPWM3","FanPWM4","FanPWM5","FanPWM6","FanPWM7","FanPWM8",
    				"FanPWM9","FanPWM10","FanPWM11","FanPWM12","FanPWM13","FanPWM14","FanPWM15","FanPWM16",
    				"FanPWM17","FanPWM18","FanPWM19","FanPWM20","FanPWM21","FanPWM22","FanPWM23","FanPWM24",

    				m_cOPDTemperatureName[0],m_cOPDTemperatureName[1],m_cOPDTemperatureName[2],m_cOPDTemperatureName[3],m_cOPDTemperatureName[4],m_cOPDTemperatureName[5],m_cOPDTemperatureName[6],m_cOPDTemperatureName[7],

    				"LDVoltage1","LDVoltage2","LDVoltage3","LDVoltage4","LDVoltage5","LDVoltage6","LDVoltage7","LDVoltage8",
    				"LDVoltage9","LDVoltage10","LDVoltage11","LDVoltage12","LDVoltage13","LDVoltage14","LDVoltage15","LDVoltage16",
    				"LDVoltage17","LDVoltage18","LDVoltage19","LDVoltage20","LDVoltage21","LDVoltage22","LDVoltage23","LDVoltage24",

    				"LDCurrent1","LDCurrent2","LDCurrent3","LDCurrent4","LDCurrent5","LDCurrent6","LDCurrent7","LDCurrent8",
    				"LDCurrent9","LDCurrent10","LDCurrent11","LDCurrent12","LDCurrent13","LDCurrent14","LDCurrent15","LDCurrent16",
    				"LDCurrent17","LDCurrent18","LDCurrent19","LDCurrent20","LDCurrent21","LDCurrent22","LDCurrent23","LDCurrent24",

    				"LDTemp1","LDTemp2","LDTemp3","LDTemp4","LDTemp5","LDTemp6","LDTemp7","LDTemp8",
    				"LDTemp9","LDTemp10","LDTemp11","LDTemp12","LDTemp13","LDTemp14","LDTemp15","LDTemp16",
    				"LDTemp17","LDTemp18","LDTemp19","LDTemp20","LDTemp21","LDTemp22","LDTemp23","LDTemp24",

    				"TECVoltage1","TECVoltage2","TECVoltage3","TECCurrent1","TECCurrent2","TECCurrent3",

    				"PumpSpeed1","PumpSpeed2","PumpPWM1","PumpPWM2","DWSpeed","PWSpeed","LightSensor[W]","LightSensor[R]","LightSensor[G]","LightSensor[B]","LightSensor[Y]","LightSensor[RR]","LightSensor[RY]",

    			    "PressSenosr1","PressSenosr2","PressSenosr3","RelativeHumidity1","RelativeHumidity2","RelativeHumidity3","AbsoluteHumidity1","AbsoluteHumidity2","AbsoluteHumidity3",
    			    "iGsensor[X]","iGsensor[Y]","iGsensor[Z]","AC Voltage","Picture_mode","Light_Source_mode","High_Altitude","Shutter",
                    "LDPWM_BLD_R","LDPWM_BLD_G","LDPWM_BLD_B","LDPWM_BLD_Y","LDPWM_RLD_R","LDPWM_RLD_Y",
					"Constant Power/Brightness Level","User_Picture_mode","DMDAirtight","CLA_ErrorCode0","CLA_ErrorCode1","CLA_Gain0","CLA_Gain1","CLA_ZData0","CLA_ZData1",
					"DimPower","DimPowerLevel","BurnInCycle",
					"LVPS_InputVoltage","LVPS_PFCOutputVoltage","LVPS_5VVoltage","LVPS_12VVoltage","LVPS_52VVoltage",
					"LVPS_5VCurrent","LVPS_12VCurrent","LVPS_52VCurrent","LVPS_PFC_MOS_Temp","LVPS_BridgeTemp",
					"LVPS_AmbientTemp","LVPS_12V_SR_MOSFET_Temp","LVPS_52V_SR_MOSFET_Temp","LVPS_PrimaryProtectionStatus","LVPS_SecondaryProtectionStatus"

    			);		//A65_OPTOMA_Doulas_0105 Modify

    	        fclose(pFile);
    	   }
    	   else
    	   {
               printf("file open fail %s\n", cTemp);
    	   }
		}

		pFile = fopen(cTemp, "a+");

        if(pFile != NULL)  //G100_Simon_0092
        {
    		fprintf(pFile, "%s", cStrTemp);
    		fclose(pFile);
        }
        else
        {
            printf("file open fail %s\n", cTemp);
        }

        utilOPD_MutexGive();
    }

    return 1;
}

int utilOPD_Write2ProjectorFile(UINT8 ucEvent, char *pcStr) //G100_Julie_0016
{
	FILE *pFile = NULL;
	int size = 0;
	char cTemp[1024] = {'\0'}, cTemp2[1024] = {'\0'}, cStrTemp[1024] = {'\0'};
	UINT16 wDay = 0, wHour = 0, wMin = 0, wSec = 0;
	UINT16 wtyear = 0, wtmon = 0, wtday = 0, wthour = 0, wtmin = 0, wtsec = 0;

    if(utilOPD_MutexTake())
    {
		utilOPD_TotalProjector_Get(&wDay, &wHour, &wMin, &wSec);
		utilOPD_RTCtimer_Get(&wtyear, &wtmon, &wtday, &wthour, &wtmin, &wtsec);

		snprintf(cStrTemp, 1023 ,"%s,%04d/%02d/%02d-%02d:%02d:%03d,%04d:%02d:%02d:%02d,%s\n", m_pcOPD_LOG[ucEvent], wtyear, wtmon, wtday, wthour, wtmin, wtsec, wDay, wHour, wMin, wSec, pcStr);

	    if(strlen(cStrTemp) == 0)
	    {
	        utilOPD_MutexGive();
	        return 0;
	    }

	    sprintf(cTemp, "%s/opd_projector_%04d.bin", OPD_LOG_PATH, 0);

		size = file_size(cTemp);

		if(size > OPD_LOG_SIZE_MAX)
		{
	        int cFile = OPD_PROJECTOR_FILE_NUMBER - 1;

	        sprintf(cTemp, "%s/opd_projector_%04d.bin", OPD_LOG_PATH, cFile);

	        if(access(cTemp, 0) == 0)
	        {
	            if(remove(cTemp) == 0)
	            {
	                //printf("Remove %s pass\n", m_pcOPD_File[cFile]);
	            }
	        }

	        for(cFile; cFile > 0; cFile--)
	        {
	            sprintf(cTemp, "%s/opd_projector_%04d.bin", OPD_LOG_PATH, cFile);
		        sprintf(cTemp2, "%s/opd_projector_%04d.bin", OPD_LOG_PATH, cFile-1);

	            if(access(cTemp2,0) == 0)
	    		{
	    			if(rename(cTemp2, cTemp) == 0)
	                {
	                    //printf("Rename %s pass.\n", m_pcOPD_File[cFile]);
	                }
	                else
	                {
	                    //printf("Rename %s fail.\n", m_pcOPD_File[cFile]);
	                }

	                if(remove(cTemp2) == 0)
	    			{
	    				//printf("Remove %s pass\n", m_pcOPD_File[cFile - 1]);
	    			}
	    		}
	        }
		}

	    sprintf(cTemp, "%s/opd_projector_%04d.bin", OPD_LOG_PATH, 0);

		if(access(cTemp,0) == -1)
		{
			UINT16 wtyear = 0, wtmon = 0, wtday = 0, wthour = 0, wtmin = 0, wtsec = 0;

			utilOPD_RTCtimer_Get(&wtyear, &wtmon, &wtday, &wthour, &wtmin, &wtsec);

			pFile = fopen(cTemp, "w+");

            if(pFile != NULL)  //G100_Simon_0092
            {
				fprintf(pFile, "%s,%s,%s,V%02d.%02d\n", OPD_MODEL_NAME, m_sOPDDataInfo.sRegulatoryInfo.cModelName, OPD_VERSION, OPD_VER_MAJOR, OPD_VER_MINOR);
				fprintf(pFile, "%s,%s\n", OPD_SERIAL_NUMBER, m_sOPDDataInfo.sRegulatoryInfo.cSN);
				fprintf(pFile, "%s,%04d/%02d/%02d-%02d:%02d:%03d\n", OPD_DAY_TIME, wtyear, wtmon, wtday, wthour, wtmin, wtsec);

    			fprintf(pFile, "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n",
    				"OPD type","DateTime (Y/M/D-H:M:S)","TotalProjector (D:H:M:S)",

    				"SensorFull_BLD_Y","SensorFull_BLD_R","SensorFull_BLD_B","SensorFull_BLD_G",
    				"SensorEco_BLD_Y","SensorEco_BLD_R","SensorEco_BLD_B","SensorEco_BLD_G",

    				"PWMFull_BLD_Y","PWMFull_BLD_R","PWMFull_BLD_B","PWMFull_BLD_G",
    				"PWMEco_BLD_Y","PWMEco_BLD_R","PWMEco_BLD_B","PWMEco_BLD_G",

    				"SensorFull_RLD_Y","SensorFull_RLD_R","SensorFull_RLD_B","SensorFull_RLD_G",
    				"SensorEco_RLD_Y","SensorEco_RLD_R","SensorEco_RLD_B","SensorEco_RLD_G",

    				"PWMFull_RLD_Y","PWMFull_RLD_R","PWMFull_RLD_B","PWMFull_RLD_G",
    				"PWMEco_RLD_Y","PWMEco_RLD_R","PWMEco_RLD_B","PWMEco_RLD_G",

    				"SensorFull_Dynamic_RLD_Y","SensorFull_Dynamic_RLD_R","SensorFull_Dynamic_RLD_B","SensorFull_Dynamic_RLD_G",
    				"SensorEco_Dynamic_RLD_Y","SensorEco_Dynamic_RLD_R","SensorEco_Dynamic_RLD_B","SensorEco_Dynamic_RLD_G",

    				"Full_PWM_Dynamic_RLD_Y","Full_PWM_Dynamic_RLD_R","Full_PWM_Dynamic_RLD_B","Full_PWM_Dynamic_RLD_G",
    				"Eco_PWM_Dynamic_RLD_Y","Eco_PWM_Dynamic_RLD_R","Eco_PWM_Dynamic_RLD_B","Eco_PWM_Dynamic_RLD_G",

					"RGBGain_R","RGBGain_G","RGBGain_B","RGBOffset_R","RGBOffset_G","RGBOffset_B",
					"YUVGain_Y","YUVGain_U","YUVGain_V","YUVOffset_Y","YUVOffset_U","YUVOffset_V",

					"RGBGain2_R","RGBGain2_G","RGBGain2_B","RGBOffset2_R","RGBOffset2_G","RGBOffset2_B",
					"YUVGain2_Y","YUVGain2_U","YUVGain2_V","YUVOffset2_Y","YUVOffset2_U","YUVOffset2_V",

					"LightSensor_Time_Y","LightSensor_Time_R","LightSensor_Time_B","LightSensor_Time_G",
					"LightSensor_Offset_Y","LightSensor_Offset_R","LightSensor_Offset_B","LightSensor_Offset_G",
					"LightSensor_Gain_Y","LightSensor_Gain_R","LightSensor_Gain_B","LightSensor_Gain_G",

					"LightSensor_Target_Y","LightSensor_Target_R","LightSensor_Target_B","LightSensor_Target_G",
					"LightSensor_TargetRLD_Y","LightSensor_TargetRLD_R",//"LightSensor_TargetRLD_B","LightSensor_TargetRLD_G",

					"LightSensor_Calibrate_Status"
				);

    	        fclose(pFile);
    	    }
    	    else
    	    {
                printf("file open fail %s\n", cTemp);
    	    }
		}

		pFile = fopen(cTemp, "a+");

        if(pFile != NULL)  //G100_Simon_0092
        {
    		fprintf(pFile, "%s", cStrTemp);
    		fclose(pFile);
        }
        else
        {
            printf("file open fail %s\n", cTemp);
        }

        utilOPD_MutexGive();
    }

    return 1;
}

int utilOPD_Write2RunTimeFile(void)
{
	FILE *pFile = NULL;
	int size = 0;
	char cTemp[1024] = {'\0'}, cTemp2[1024] = {'\0'};

    if(utilOPD_MutexTake())
    {
        if(strlen(m_cOPD_RunTime_Buffer) == 0)
        {
            utilOPD_MutexGive();
            return 0;
        }

		sprintf(cTemp, "%s/opd_run_time_%04d.bin", OPD_LOG_PATH, 0);
    	size = file_size(cTemp);

		if(size > OPD_LOG_SIZE_MAX)
		{
			int cFile = OPD_RUNTIME_TEST_FILE_NUMBER - 1;

			sprintf(cTemp, "%s/opd_run_time_%04d.bin", OPD_LOG_PATH, cFile);

			if(access(cTemp, 0) == 0)
			{
				if(remove(cTemp) == 0)
                {
                    //printf("Remove %s pass\n", m_pcOPD_File[cFile]);
                }
            }

            for(cFile; cFile > 0; cFile--)
            {
			    sprintf(cTemp, "%s/opd_run_time_%04d.bin", OPD_LOG_PATH, cFile);
				sprintf(cTemp2, "%s/opd_run_time_%04d.bin", OPD_LOG_PATH, cFile-1);

				if(access(cTemp2,0) == 0)
				{
					if(rename(cTemp2, cTemp) == 0)
                    {
                        //printf("Rename %s pass.\n", m_pcOPD_File[cFile]);
                    }
                    else
                    {
                        //printf("Rename %s fail.\n", m_pcOPD_File[cFile]);
                    }

					if(remove(cTemp2) == 0)
        			{
        				//printf("Remove %s pass\n", m_pcOPD_File[cFile - 1]);
        			}
        		}
            }
    	}

		sprintf(cTemp, "%s/opd_run_time_%04d.bin", OPD_LOG_PATH, 0);

		if(access(cTemp,0) == -1)
    	{

			UINT16 wtyear = 0, wtmon = 0, wtday = 0, wthour = 0, wtmin = 0, wtsec = 0;

			utilOPD_RTCtimer_Get(&wtyear, &wtmon, &wtday, &wthour, &wtmin, &wtsec);

			pFile = fopen(cTemp, "w+");

            if(pFile != NULL)  //G100_Simon_0092
            {
				fprintf(pFile, "%s,%s,%s,V%02d.%02d\n", OPD_MODEL_NAME, m_sOPDDataInfo.sRegulatoryInfo.cModelName, OPD_VERSION, OPD_VER_MAJOR, OPD_VER_MINOR);
				fprintf(pFile, "%s,%s\n", OPD_SERIAL_NUMBER, m_sOPDDataInfo.sRegulatoryInfo.cSN);
				fprintf(pFile, "%s,%04d/%02d/%02d-%02d:%02d:%03d\n", OPD_DAY_TIME, wtyear, wtmon, wtday, wthour, wtmin, wtsec);

    			fprintf(pFile, "%s,%s,%s,%s,%s,%s,%s\n", "OPD type","DateTime (Y/M/D-H:M:S)","TotalProjector (D:H:M:S)","Name","Retry count","Error count","Total count");
                fclose(pFile);
            }
            else
            {
                printf("file open fail %s\n", cTemp);
            }
    	}

		pFile = fopen(cTemp, "a+");

        if(pFile != NULL)  //G100_Simon_0092
        {
        	fprintf(pFile, "%s", m_cOPD_RunTime_Buffer);
        	fclose(pFile);
        }
        else
        {
            printf("file open fail %s\n", cTemp);
        }

        utilOPD_MutexGive();
    }

    utilOPD_RunTime_CleanBuffer();

    return 1;

}

int utilOPD_Write2SourceFile(UINT8 ucEvent, char *pcStr) //G100_Julie_0046
{
	FILE *pFile = NULL;
	int size = 0;
	char cTemp[1024] = {'\0'}, cTemp2[1024] = {'\0'}, cStrTemp[1024] = {'\0'};
	UINT16 wDay = 0, wHour = 0, wMin = 0, wSec = 0;
	UINT16 wtyear = 0, wtmon = 0, wtday = 0, wthour = 0, wtmin = 0, wtsec = 0;

    if(utilOPD_MutexTake())
    {
		utilOPD_TotalProjector_Get(&wDay, &wHour, &wMin, &wSec);
		utilOPD_RTCtimer_Get(&wtyear, &wtmon, &wtday, &wthour, &wtmin, &wtsec);

		snprintf(cStrTemp, 1023 ,"%s,%04d/%02d/%02d-%02d:%02d:%03d,%04d:%02d:%02d:%02d,%s\n", m_pcOPD_LOG[ucEvent], wtyear, wtmon, wtday, wthour, wtmin, wtsec, wDay, wHour, wMin, wSec, pcStr);

	    if(strlen(cStrTemp) == 0)
	    {
	        utilOPD_MutexGive();
	        return 0;
	    }

	    sprintf(cTemp, "%s/opd_source_%04d.bin", OPD_LOG_PATH, 0);

		size = file_size(cTemp);

		if(size > OPD_LOG_SIZE_MAX)
		{
	        int cFile = OPD_SOURCE_FILE_NUMBER - 1;

	        sprintf(cTemp, "%s/opd_source_%04d.bin", OPD_LOG_PATH, cFile);

	        if(access(cTemp, 0) == 0)
	        {
	            if(remove(cTemp) == 0)
	            {
	                //printf("Remove %s pass\n", m_pcOPD_File[cFile]);
	            }
	        }

	        for(cFile; cFile > 0; cFile--)
	        {
	            sprintf(cTemp, "%s/opd_source_%04d.bin", OPD_LOG_PATH, cFile);
		        sprintf(cTemp2, "%s/opd_source_%04d.bin", OPD_LOG_PATH, cFile-1);

	            if(access(cTemp2,0) == 0)
	    		{
	    			if(rename(cTemp2, cTemp) == 0)
	                {
	                    //printf("Rename %s pass.\n", m_pcOPD_File[cFile]);
	                }
	                else
	                {
	                    //printf("Rename %s fail.\n", m_pcOPD_File[cFile]);
	                }

	                if(remove(cTemp2) == 0)
	    			{
	    				//printf("Remove %s pass\n", m_pcOPD_File[cFile - 1]);
	    			}
	    		}
	        }
		}

	    sprintf(cTemp, "%s/opd_source_%04d.bin", OPD_LOG_PATH, 0);

		if(access(cTemp,0) == -1)
		{
			UINT16 wtyear = 0, wtmon = 0, wtday = 0, wthour = 0, wtmin = 0, wtsec = 0;

			utilOPD_RTCtimer_Get(&wtyear, &wtmon, &wtday, &wthour, &wtmin, &wtsec);

			pFile = fopen(cTemp, "w+");

            if(pFile != NULL)  //G100_Simon_0092
            {
				fprintf(pFile, "%s,%s,%s,V%02d.%02d\n", OPD_MODEL_NAME, m_sOPDDataInfo.sRegulatoryInfo.cModelName, OPD_VERSION, OPD_VER_MAJOR, OPD_VER_MINOR);
				fprintf(pFile, "%s,%s\n", OPD_SERIAL_NUMBER, m_sOPDDataInfo.sRegulatoryInfo.cSN);
				fprintf(pFile, "%s,%04d/%02d/%02d-%02d:%02d:%03d\n", OPD_DAY_TIME, wtyear, wtmon, wtday, wthour, wtmin, wtsec);

    			fprintf(pFile, "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n", //[%s]count=57

    			    "OPD type","DateTime (Y/M/D-H:M:S)","TotalProjector (D:H:M:S)",

    			    "MainSource","SubSource","PIPEnable","MainChannel","SubChannel","VideoReady","BackupInpEnable","BackupInputActive","FPGASW","6634RX","6634TX","6805_0_St","6805_1_St","6634RstCnt","6805RstCnt","7604_St",

    				"CH1_VideoReady","CH1_VideoPCLK","CH1_VideoHTotal","CH1_VideoHActive","CH1_VideoVTotal","CH1_VideoVActive","CH1_VideoVRate",
    				"CH1_VideoColorDepth","CH1_VideoColorSpace","CH1_VideoDynamicRange","CH1_VideoScanning","CH1_VideoAVIInfoDet","CH1_Video3DFormat","CH1_VideoVGASyncType","CH1_VideoColorYUV","CH1_VideoDownScaling","CH1_VideoDualPixelMode","CH1_VideoHDRType","CH1_VideoExtentedColorimetry",

    				"CH2_VideoReady","CH2_VideoPCLK","CH2_VideoHTotal","CH2_VideoHActive","CH2_VideoVTotal","CH2_VideoVActive","CH2_VideoVRate",
    				"CH2_VideoColorDepth","CH2_VideoColorSpace","CH2_VideoDynamicRange","CH2_VideoScanning","CH2_VideoAVIInfoDet","CH2_Video3DFormat","CH2_VideoVGASyncType","CH2_VideoColorYUV","CH2_VideoDownScaling","CH2_VideoDualPixelMode","CH2_VideoHDRType","CH2_VideoExtentedColorimetry"
    			);

    	        fclose(pFile);
    	   }
    	   else
    	   {
               printf("file open fail %s\n", cTemp);
    	   }
		}

		pFile = fopen(cTemp, "a+");

        if(pFile != NULL)  //G100_Simon_0092
        {
    		fprintf(pFile, "%s", cStrTemp);
    		fclose(pFile);
        }
        else
        {
            printf("file open fail %s\n", cTemp);
        }

        utilOPD_MutexGive();
    }

    return 1;
}

int utilOPD_Write2InterfaceLogFile(char *pcOPDType, char *pcStr) //G100_Julie_0048
{
	FILE *pFile = NULL;
	int size = 0;
	char cTemp[1024] = {'\0'}, cTemp2[1024] = {'\0'}, cStrTemp[1024] = {'\0'};
	UINT16 wDay = 0, wHour = 0, wMin = 0, wSec = 0;
	UINT16 wtyear = 0, wtmon = 0, wtday = 0, wthour = 0, wtmin = 0, wtsec = 0;

    if(utilOPD_MutexTake())
    {
		utilOPD_TotalProjector_Get(&wDay, &wHour, &wMin, &wSec);
		utilOPD_RTCtimer_Get(&wtyear, &wtmon, &wtday, &wthour, &wtmin, &wtsec);
        snprintf(cStrTemp, 1023 ,"%s,%04d/%02d/%02d-%02d:%02d:%03d,%04d:%02d:%02d:%02d,%s\n",
            pcOPDType, wtyear, wtmon, wtday, wthour, wtmin, wtsec, wDay, wHour, wMin, wSec, pcStr);

	    if(strlen(cStrTemp) == 0)
	    {
	        utilOPD_MutexGive();
	        return 0;
	    }

	    sprintf(cTemp, "%s/opd_interface_log_%04d.bin", OPD_LOG_PATH, 0);

		size = file_size(cTemp);

		if(size > OPD_LOG_SIZE_MAX)
		{
	        int cFile = OPD_INTERFACE_LOG_NUMBER - 1;

	        sprintf(cTemp, "%s/opd_interface_log_%04d.bin", OPD_LOG_PATH, cFile);

	        if(access(cTemp, 0) == 0)
	        {
	            if(remove(cTemp) == 0)
	            {
	                //printf("Remove %s pass\n", m_pcOPD_File[cFile]);
	            }
	        }

	        for(cFile; cFile > 0; cFile--)
	        {
	            sprintf(cTemp, "%s/opd_interface_log_%04d.bin", OPD_LOG_PATH, cFile);
		        sprintf(cTemp2, "%s/opd_interface_log_%04d.bin", OPD_LOG_PATH, cFile-1);

	            if(access(cTemp2,0) == 0)
	    		{
	    			if(rename(cTemp2, cTemp) == 0)
	                {
	                    //printf("Rename %s pass.\n", m_pcOPD_File[cFile]);
	                }
	                else
	                {
	                    //printf("Rename %s fail.\n", m_pcOPD_File[cFile]);
	                }

	                if(remove(cTemp2) == 0)
	    			{
	    				//printf("Remove %s pass\n", m_pcOPD_File[cFile - 1]);
	    			}
	    		}
	        }
		}

	    sprintf(cTemp, "%s/opd_interface_log_%04d.bin", OPD_LOG_PATH, 0);

		if(access(cTemp,0) == -1)
		{
			UINT16 wtyear = 0, wtmon = 0, wtday = 0, wthour = 0, wtmin = 0, wtsec = 0;

			utilOPD_RTCtimer_Get(&wtyear, &wtmon, &wtday, &wthour, &wtmin, &wtsec);

			pFile = fopen(cTemp, "w+");

            if(pFile != NULL)  //G100_Simon_0092
            {
				fprintf(pFile, "%s,%s,%s,V%02d.%02d\n", OPD_MODEL_NAME, m_sOPDDataInfo.sRegulatoryInfo.cModelName, OPD_VERSION, OPD_VER_MAJOR, OPD_VER_MINOR);
                fprintf(pFile, "%s,%s\n", OPD_SERIAL_NUMBER, m_sOPDDataInfo.sRegulatoryInfo.cSN);
                fprintf(pFile, "%s,%04d/%02d/%02d-%02d:%02d:%03d\n", OPD_DAY_TIME, wtyear, wtmon, wtday, wthour, wtmin, wtsec);

                fprintf(pFile, "%s\n", "OPD type,DateTime (Y/M/D-H:M:S),TotalProjector (D:H:M:S),IC name,Result,retry count,error count,i2c status,error rate");

                fclose(pFile);
    	    }
    	    else
    	    {
    	        printf("file open fail %s\n", cTemp);
    	    }
		}

		pFile = fopen(cTemp, "a+");

        if(pFile != NULL)  //G100_Simon_0092
        {
    		fprintf(pFile, "%s", cStrTemp);
    		fclose(pFile);
        }
        else
        {
            printf("file open fail %s\n", cTemp);
        }

        utilOPD_MutexGive();
    }

    return 1;
}

int utilOPD_Write_ModelChangeLog(const char *pcString, const char *pcString2) //A65_OPTOMA_Julie_0066
{
	FILE *pFile = NULL;
	int size = 0;
	char cTemp[1024] = {'\0'}, cTemp2[1024] = {'\0'}, cStrTemp[1024] = {'\0'};
	UINT16 wtyear = 0, wtmon = 0, wtday = 0, wthour = 0, wtmin = 0, wtsec = 0;

	utilOPD_RTCtimer_Get(&wtyear, &wtmon, &wtday, &wthour, &wtmin, &wtsec);

    if(utilOPD_MutexTake())
    {
		snprintf(cStrTemp, 127, "%04d/%02d/%02d-%02d:%02d:%03d, %s, %s\n", wtyear, wtmon, wtday, wthour, wtmin, wtsec, pcString, pcString2);

	    if(strlen(cStrTemp) == 0)
	    {
	        utilOPD_MutexGive();
	        return 0;
	    }

	    sprintf(cTemp, "%s", MODEL_CHANGE_LOG_TXT);
		sprintf(cTemp2, "%s", MODEL_CHANGE_LOG_TXT_RESERVE);

		size = file_size(cTemp);

		if(size > OPD_LOG_SIZE_MAX)
		{
 	        if(access(cTemp2, 0) == 0)
	        {
	            if(remove(cTemp2) == 0)
	            {
	                //printf("Remove %s pass\n", m_pcOPD_File[cFile]);
	            }
	        }

			if(access(cTemp,0) == 0)
			{
				if(rename(cTemp, cTemp2) == 0)
				{
					//printf("Rename %s pass.\n", m_pcOPD_File[cFile]);
				}
				else
				{
					//printf("Rename %s fail.\n", m_pcOPD_File[cFile]);
				}

				if(remove(cTemp) == 0)
				{
					//printf("Remove %s pass\n", m_pcOPD_File[cFile - 1]);
				}
			}
		}

	    sprintf(cTemp, "%s", MODEL_CHANGE_LOG_TXT);

		pFile = fopen(cTemp, "a+");

        if(pFile != NULL)  //G100_Simon_0092
        {
    		fprintf(pFile, "%s", cStrTemp);
    		fclose(pFile);
        }
        else
        {
            printf("file open fail %s\n", cTemp);
        }

        utilOPD_MutexGive();
    }

    return 1;
}

int utilOPD_Write_LensTypeLog(int value1, int value2, int value3)
{
	FILE *pFile = NULL;
	int size = 0;
	char cTemp[1024] = {'\0'}, cTemp2[1024] = {'\0'}, cStrTemp[1024] = {'\0'};
	UINT16 wtyear = 0, wtmon = 0, wtday = 0, wthour = 0, wtmin = 0, wtsec = 0;

	utilOPD_RTCtimer_Get(&wtyear, &wtmon, &wtday, &wthour, &wtmin, &wtsec);

    if(utilOPD_MutexTake())
    {
		snprintf(cStrTemp, 127, "%04d/%02d/%02d-%02d:%02d:%03d, [%d, %d, %d]\n", wtyear, wtmon, wtday, wthour, wtmin, wtsec, value1, value2, value3);

	    if(strlen(cStrTemp) == 0)
	    {
	        utilOPD_MutexGive();
	        return 0;
	    }

	    sprintf(cTemp, "%s", LENS_TYPE_LOG_TXT);
		sprintf(cTemp2, "%s", LENS_TYPE_LOG_TXT_RESERVE);

		size = file_size(cTemp);

		if(size > OPD_LOG_SIZE_MAX)
		{
 	        if(access(cTemp2, 0) == 0)
	        {
	            if(remove(cTemp2) == 0)
	            {
	                //printf("Remove %s pass\n", m_pcOPD_File[cFile]);
	            }
	        }

			if(access(cTemp,0) == 0)
			{
				if(rename(cTemp, cTemp2) == 0)
				{
					//printf("Rename %s pass.\n", m_pcOPD_File[cFile]);
				}
				else
				{
					//printf("Rename %s fail.\n", m_pcOPD_File[cFile]);
				}

				if(remove(cTemp) == 0)
				{
					//printf("Remove %s pass\n", m_pcOPD_File[cFile - 1]);
				}
			}
		}

	    sprintf(cTemp, "%s", LENS_TYPE_LOG_TXT);

		pFile = fopen(cTemp, "a+");

        if(pFile != NULL)  //G100_Simon_0092
        {
    		fprintf(pFile, "%s", cStrTemp);
    		fclose(pFile);
        }
        else
        {
            printf("file open fail %s\n", cTemp);
        }

        utilOPD_MutexGive();
    }

    return 1;
}

int utilOPD_Write2TempFile(char *pcStr) //G100_Julie_0015
{
	FILE *pFile = NULL;
	int size = 0;
	int cFile = 0;
	char cTemp[1024] = {'\0'}, cTemp2[1024] = {'\0'};

    if(strlen(pcStr) == 0)
    {
        return 0;
    }

    sprintf(cTemp, "/tmp/log_%04d.bin", 0);

	size = file_size(cTemp);

	if(size > OPD_LOG_SIZE_MAX)
	{
        cFile = OPD_LOG_FILE_NUMBER - 1;

        sprintf(cTemp, "/tmp/log_%04d.bin", cFile);

        if(access(cTemp, 0) == 0)
        {
            if(remove(cTemp) == 0)
            {
                //printf("Remove %s pass\n", m_pcOPD_File[cFile]);
            }
        }

        for(cFile; cFile > 0; cFile--)
        {
            sprintf(cTemp, "/tmp/log_%04d.bin", cFile);
	        sprintf(cTemp2, "/tmp/log_%04d.bin", cFile-1);

            if(access(cTemp2,0) == 0)
    		{
    			if(rename(cTemp2, cTemp) == 0)
                {
                    //printf("Rename %s pass.\n", m_pcOPD_File[cFile]);
                }
                else
                {
                    //printf("Rename %s fail.\n", m_pcOPD_File[cFile]);
                }

                if(remove(cTemp2) == 0)
    			{
    				//printf("Remove %s pass\n", m_pcOPD_File[cFile - 1]);
    			}
    		}
        }
	}

    sprintf(cTemp, "/tmp/log_%04d.bin", 0);

	pFile = fopen(cTemp, "a+");

    if(pFile != NULL)  //G100_Simon_0092
    {
    	fprintf(pFile, "%s", pcStr);
    	fclose(pFile);
    }
    else
    {
        printf("file open fail %s\n", cTemp);
    }

    return 1;
}

int utilOPD_Download_File(UINT32 dSelect) //G100_Julie_0015
{
	FILE *pFile = NULL;
	int size = 0;
	int cFile = OPD_LOG_FILE_NUMBER - 1;
	int cSelNum = eOPD_NUMBERS - 1;
	char cStrTemp[OPD_LOG_BUFFER] = {'\0'};  //G100_Simon_0058
    char cType[64] = {'\0'};
    char cString[1024] = {'\0'}, cTemp[1024] = {'\0'};

	for(cFile; cFile >= 0; cFile--)
	{
	    sprintf(cTemp, "/tmp/log_%04d.bin", cFile);

		if(access(cTemp, 0) == 0)
		{
			if(remove(cTemp) == 0)
			{
				//printf("Remove %s pass\n", m_pcOPD_File[cFile]);
			}
		}
	}

	cFile = OPD_LOG_FILE_NUMBER - 1;

	for(cFile; cFile >= 0; cFile--)
    {
        uOPD_DATA sDev = {0};
		sprintf(cTemp, "%s/opd_log_%04d.bin", OPD_LOG_PATH, cFile);

        size = file_size(cTemp);

        if(access(cTemp,0)==-1)
        {
            continue;//return 1;
        }

        pFile = fopen(cTemp, "r");

        if(pFile != NULL)  //G100_Simon_0092
        {
            while (!feof(pFile))
            {
                if (fgets(cStrTemp, OPD_LOG_BUFFER, pFile) == NULL)
                {
                    break;
                }

                        if(feof(pFile))
                        {
                                break;//return 1;
    			}

                sscanf(cStrTemp, "%63[^,],%1023[^\n]", cType, cString);

    			for(cSelNum; cSelNum >= 0; cSelNum--)
    			{
    			    if(((dSelect >> cSelNum) & 0x01) == 1)
    				{
    					if(strncmp(cType, m_pcOPD_LOG[cSelNum], strlen(m_pcOPD_LOG[cSelNum])) == 0)
    					{
    					    utilOPD_Write2TempFile(cStrTemp);
    					}
    					break;
    				}
    			}
            }

            fclose(pFile);
        }
        else
        {
            printf("file open fail %s\n", cTemp);
        }
    }

	return 0;
}

int utilOPD_EventSet(UINT8 ucEvent, uOPD_DATA *puData)
{
    char str[1024];

    switch(ucEvent)
    {
        case eOPD_ERROR_LOG:
        case eOPD_WARNING_LOG:
            {
                snprintf(str, 1023, "0x%08x,%s", puData->sErrorLog.dwGECCode, puData->sErrorLog.cString);
                utilOPD_Write2Buffer(ucEvent, str);
            }
            break;

        case eOPD_SNAPSHOT_LOG:
        case eOPD_ENGINE_LOG:
        case eOPD_POWER_ON_LOG: //G100_Steven_0089
        case eOPD_POWER_OFF_LOG:
		case eOPD_POWER_STARTUP_LOG:
		case eOPD_SHUTTER_ON_LOG:
		case eOPD_SHUTTER_OFF_LOG:
		case eOPD_HIGH_ALTITUDE_OFF:
		case eOPD_HIGH_ALTITUDE_ON:
		case eOPD_PICTURE_MODE_LOG:
		case eOPD_LIGHT_SOURCE_LOG:
		case eOPD_LIGHT_SENSOR_CALIBRATION:
		case eOPD_CELING_MOUNT_OFF_LOG:
		case eOPD_CELING_MOUNT_ON_LOG:
		case eOPD_CELING_MOUNT_AUTO_LOG:
            {
                snprintf(str, 1023, "%ld,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%s,%s,%s,%s,%s,%d,%d,%d,%d,%d,%d,%d,%s,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%d,%d",

                puData->sSnapshotLog.ulLDMinute,
                puData->sSnapshotLog.wFanSpeed[0],
                puData->sSnapshotLog.wFanSpeed[1],
                puData->sSnapshotLog.wFanSpeed[2],
                puData->sSnapshotLog.wFanSpeed[3],
                puData->sSnapshotLog.wFanSpeed[4],
                puData->sSnapshotLog.wFanSpeed[5],
                puData->sSnapshotLog.wFanSpeed[6],
                puData->sSnapshotLog.wFanSpeed[7],
                puData->sSnapshotLog.wFanSpeed[8],
                puData->sSnapshotLog.wFanSpeed[9],
                puData->sSnapshotLog.wFanSpeed[10],
                puData->sSnapshotLog.wFanSpeed[11],
                puData->sSnapshotLog.wFanSpeed[12],
                puData->sSnapshotLog.wFanSpeed[13],
                puData->sSnapshotLog.wFanSpeed[14],
                puData->sSnapshotLog.wFanSpeed[15],
                puData->sSnapshotLog.wFanSpeed[16],
                puData->sSnapshotLog.wFanSpeed[17],
                puData->sSnapshotLog.wFanSpeed[18],
                puData->sSnapshotLog.wFanSpeed[19],
                puData->sSnapshotLog.wFanSpeed[20],
                puData->sSnapshotLog.wFanSpeed[21],
                puData->sSnapshotLog.wFanSpeed[22],
                puData->sSnapshotLog.wFanSpeed[23],
                puData->sSnapshotLog.cFanPWM[0],
                puData->sSnapshotLog.cFanPWM[1],
                puData->sSnapshotLog.cFanPWM[2],
                puData->sSnapshotLog.cFanPWM[3],
                puData->sSnapshotLog.cFanPWM[4],
                puData->sSnapshotLog.cFanPWM[5],
                puData->sSnapshotLog.cFanPWM[6],
                puData->sSnapshotLog.cFanPWM[7],
                puData->sSnapshotLog.cFanPWM[8],
                puData->sSnapshotLog.cFanPWM[9],
                puData->sSnapshotLog.cFanPWM[10],
                puData->sSnapshotLog.cFanPWM[11],
                puData->sSnapshotLog.cFanPWM[12],
                puData->sSnapshotLog.cFanPWM[13],
                puData->sSnapshotLog.cFanPWM[14],
                puData->sSnapshotLog.cFanPWM[15],
                puData->sSnapshotLog.cFanPWM[16],
                puData->sSnapshotLog.cFanPWM[17],
                puData->sSnapshotLog.cFanPWM[18],
                puData->sSnapshotLog.cFanPWM[19],
                puData->sSnapshotLog.cFanPWM[20],
                puData->sSnapshotLog.cFanPWM[21],
                puData->sSnapshotLog.cFanPWM[22],
                puData->sSnapshotLog.cFanPWM[23],
                puData->sSnapshotLog.wTemperature[0],
                puData->sSnapshotLog.wTemperature[1],
                puData->sSnapshotLog.wTemperature[2],
                puData->sSnapshotLog.wTemperature[3],
                puData->sSnapshotLog.wTemperature[4],
                puData->sSnapshotLog.wTemperature[5],
                puData->sSnapshotLog.wTemperature[6],
                puData->sSnapshotLog.wTemperature[7],
                puData->sSnapshotLog.wLDVoltage[0],
                puData->sSnapshotLog.wLDVoltage[1],
                puData->sSnapshotLog.wLDVoltage[2],
                puData->sSnapshotLog.wLDVoltage[3],
                puData->sSnapshotLog.wLDVoltage[4],
                puData->sSnapshotLog.wLDVoltage[5],
                puData->sSnapshotLog.wLDVoltage[6],
                puData->sSnapshotLog.wLDVoltage[7],
                puData->sSnapshotLog.wLDVoltage[8],
                puData->sSnapshotLog.wLDVoltage[9],
                puData->sSnapshotLog.wLDVoltage[10],
                puData->sSnapshotLog.wLDVoltage[11],
                puData->sSnapshotLog.wLDVoltage[12],
                puData->sSnapshotLog.wLDVoltage[13],
                puData->sSnapshotLog.wLDVoltage[14],
                puData->sSnapshotLog.wLDVoltage[15],
                puData->sSnapshotLog.wLDVoltage[16],
                puData->sSnapshotLog.wLDVoltage[17],
                puData->sSnapshotLog.wLDVoltage[18],
                puData->sSnapshotLog.wLDVoltage[19],
                puData->sSnapshotLog.wLDVoltage[20],
                puData->sSnapshotLog.wLDVoltage[21],
                puData->sSnapshotLog.wLDVoltage[22],
                puData->sSnapshotLog.wLDVoltage[23],
                puData->sSnapshotLog.wLDCurrent[0],
                puData->sSnapshotLog.wLDCurrent[1],
                puData->sSnapshotLog.wLDCurrent[2],
                puData->sSnapshotLog.wLDCurrent[3],
                puData->sSnapshotLog.wLDCurrent[4],
                puData->sSnapshotLog.wLDCurrent[5],
                puData->sSnapshotLog.wLDCurrent[6],
                puData->sSnapshotLog.wLDCurrent[7],
                puData->sSnapshotLog.wLDCurrent[8],
                puData->sSnapshotLog.wLDCurrent[9],
                puData->sSnapshotLog.wLDCurrent[10],
                puData->sSnapshotLog.wLDCurrent[11],
                puData->sSnapshotLog.wLDCurrent[12],
                puData->sSnapshotLog.wLDCurrent[13],
                puData->sSnapshotLog.wLDCurrent[14],
                puData->sSnapshotLog.wLDCurrent[15],
                puData->sSnapshotLog.wLDCurrent[16],
                puData->sSnapshotLog.wLDCurrent[17],
                puData->sSnapshotLog.wLDCurrent[18],
                puData->sSnapshotLog.wLDCurrent[19],
                puData->sSnapshotLog.wLDCurrent[20],
                puData->sSnapshotLog.wLDCurrent[21],
                puData->sSnapshotLog.wLDCurrent[22],
                puData->sSnapshotLog.wLDCurrent[23],
                puData->sSnapshotLog.wLDTemp[0],
                puData->sSnapshotLog.wLDTemp[1],
                puData->sSnapshotLog.wLDTemp[2],
                puData->sSnapshotLog.wLDTemp[3],
                puData->sSnapshotLog.wLDTemp[4],
                puData->sSnapshotLog.wLDTemp[5],
                puData->sSnapshotLog.wLDTemp[6],
                puData->sSnapshotLog.wLDTemp[7],
                puData->sSnapshotLog.wLDTemp[8],
                puData->sSnapshotLog.wLDTemp[9],
                puData->sSnapshotLog.wLDTemp[10],
                puData->sSnapshotLog.wLDTemp[11],
                puData->sSnapshotLog.wLDTemp[12],
                puData->sSnapshotLog.wLDTemp[13],
                puData->sSnapshotLog.wLDTemp[14],
                puData->sSnapshotLog.wLDTemp[15],
				puData->sSnapshotLog.wLDTemp[16],
				puData->sSnapshotLog.wLDTemp[17],
				puData->sSnapshotLog.wLDTemp[18],
				puData->sSnapshotLog.wLDTemp[19],
				puData->sSnapshotLog.wLDTemp[20],
				puData->sSnapshotLog.wLDTemp[21],
				puData->sSnapshotLog.wLDTemp[22],
				puData->sSnapshotLog.wLDTemp[23],

                puData->sSnapshotLog.wTECVoltage[0],
                puData->sSnapshotLog.wTECVoltage[1],
                puData->sSnapshotLog.wTECVoltage[2],
                puData->sSnapshotLog.wTECCurrent[0],
                puData->sSnapshotLog.wTECCurrent[1],
                puData->sSnapshotLog.wTECCurrent[2],

                puData->sSnapshotLog.wPumpSpeed[0],
                puData->sSnapshotLog.wPumpSpeed[1],
                puData->sSnapshotLog.cPumpPWM[0],
                puData->sSnapshotLog.cPumpPWM[1],
                puData->sSnapshotLog.wDWSpeed,
                puData->sSnapshotLog.wPWSpeed,

				puData->sSnapshotLog.wLightSenosr[0],
				puData->sSnapshotLog.wLightSenosr[1],
				puData->sSnapshotLog.wLightSenosr[2],
				puData->sSnapshotLog.wLightSenosr[3],
				puData->sSnapshotLog.wLightSenosr[4],
				puData->sSnapshotLog.wLightSenosr[5],
				puData->sSnapshotLog.wLightSenosr[6],

				puData->sSnapshotLog.wPressSenosr[0],
				puData->sSnapshotLog.wPressSenosr[1],
				puData->sSnapshotLog.wPressSenosr[2],
				puData->sSnapshotLog.wHumiditySenosr[0],
				puData->sSnapshotLog.wHumiditySenosr[1],
				puData->sSnapshotLog.wHumiditySenosr[2],
                puData->sSnapshotLog.wAbsHumiditySenosr[0],
                puData->sSnapshotLog.wAbsHumiditySenosr[1],
                puData->sSnapshotLog.wAbsHumiditySenosr[2],
				puData->sSnapshotLog.iGsensor[0],
				puData->sSnapshotLog.iGsensor[1],
				puData->sSnapshotLog.iGsensor[2],
				puData->sSnapshotLog.Voltage,
                m_sPicture_Setting_String[m_sOPDDataInfo.sRegulatoryInfo.cPictureMode[m_sOPDDataInfo.sRegulatoryInfo.cInputSource[0]]],
                m_sPower_Mode_String[puData->sSnapshotLog.ucPowerMode],
                m_sToggle_String[puData->sSnapshotLog.ucHighAltitude],
                m_sToggle_String[puData->sSnapshotLog.ucShutter],
                puData->sSnapshotLog.m_uiLDPWM[0],
                puData->sSnapshotLog.m_uiLDPWM[1],
                puData->sSnapshotLog.m_uiLDPWM[2],
                puData->sSnapshotLog.m_uiLDPWM[3],
                puData->sSnapshotLog.m_uiLDPWM[4],
                puData->sSnapshotLog.m_uiLDPWM[5],
				puData->sSnapshotLog.ucPowerLevel,
				m_sUSER_Picture_Setting_String[puData->sSnapshotLog.ucUserPictureMode],
		        puData->sSnapshotLog.ucDMDAirtight,
		        puData->sSnapshotLog.ulCLAError0,
		        puData->sSnapshotLog.ulCLAError1,
		        puData->sSnapshotLog.ulCLAGain0,
		        puData->sSnapshotLog.ulCLAGain1,
		        puData->sSnapshotLog.ulCLAZData0,
		        puData->sSnapshotLog.ulCLAZData1,
		        puData->sSnapshotLog.ucDimPowerFlag,
		        puData->sSnapshotLog.ucDimPowerLevel,
		        puData->sSnapshotLog.wBurnInCycle,

                puData->sSnapshotLog.lLVPS_InputVoltage,
                puData->sSnapshotLog.lLVPS_PFC_OutputVoltage,
                puData->sSnapshotLog.lLVPS_5V_Voltage,
                puData->sSnapshotLog.lLVPS_12V_Voltage,
                puData->sSnapshotLog.lLVPS_52V_Voltage,
                puData->sSnapshotLog.lLVPS_5V_Current,
                puData->sSnapshotLog.lLVPS_12V_Current,
                puData->sSnapshotLog.lLVPS_52V_Current,
                puData->sSnapshotLog.lLVPS_PFC_MOS_Temperature,
                puData->sSnapshotLog.lLVPS_Bridge_Temperature,
                puData->sSnapshotLog.lLVPS_Ambient_Temperature,
                puData->sSnapshotLog.lLVPS_12V_SR_MOSFET_Temperature,
                puData->sSnapshotLog.lLVPS_52V_SR_MOSFET_Temperature,
                puData->sSnapshotLog.uiLVPS_PRIMARY_PROTECTION_STATUS,
                puData->sSnapshotLog.uiLVPS_SECONDARY_PROTECTION_STATUS

				);		//A65_OPTOMA_Doulas_0105 Modify

 				if(ucEvent < eOPD_ENGINE_LOG || (ucEvent > eOPD_ENGINE_LOG && ucEvent <= eOPD_INTERFACE_LOG) ||
					(ucEvent >= eOPD_INPUT_DETECT_LOG && ucEvent <= eOPD_VOLTAGE_DETECT))
				{
					utilOPD_Write2Buffer(ucEvent, str);
				}

 				if(ucEvent == eOPD_SNAPSHOT_LOG || ucEvent == eOPD_ENGINE_LOG ||
					(ucEvent >= eOPD_POWER_ON_LOG && ucEvent <= eOPD_HIGH_ALTITUDE_ON) ||
					(ucEvent >= eOPD_PICTURE_MODE_LOG && ucEvent <= eOPD_CELING_MOUNT_AUTO_LOG))
				{
					utilOPD_Write2EngineFile(ucEvent, str);
				}
            }
            break;

        case eOPD_PROJECTOR_LOG:
            {
                snprintf(str, 1023, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%f,%f,%f,%f,%d,%d,%d,%d,%d,%d,%s",

				puData->sProjectorLog.wSensorFull_BLD_Y,
				puData->sProjectorLog.wSensorFull_BLD_R,
				puData->sProjectorLog.wSensorFull_BLD_B,
				puData->sProjectorLog.wSensorFull_BLD_G,
				puData->sProjectorLog.wSensorEco_BLD_Y,
				puData->sProjectorLog.wSensorEco_BLD_R,
				puData->sProjectorLog.wSensorEco_BLD_B,
				puData->sProjectorLog.wSensorEco_BLD_G,
				puData->sProjectorLog.wPWMFull_BLD_Y,
				puData->sProjectorLog.wPWMFull_BLD_R,
				puData->sProjectorLog.wPWMFull_BLD_B,
				puData->sProjectorLog.wPWMFull_BLD_G,
				puData->sProjectorLog.wPWMEco_BLD_Y,
				puData->sProjectorLog.wPWMEco_BLD_R,
				puData->sProjectorLog.wPWMEco_BLD_B,
				puData->sProjectorLog.wPWMEco_BLD_G,
				puData->sProjectorLog.wSensorFull_RLD_Y,
				puData->sProjectorLog.wSensorFull_RLD_R,
				puData->sProjectorLog.wSensorFull_RLD_B,
				puData->sProjectorLog.wSensorFull_RLD_G,
				puData->sProjectorLog.wSensorEco_RLD_Y,
				puData->sProjectorLog.wSensorEco_RLD_R,
				puData->sProjectorLog.wSensorEco_RLD_B,
				puData->sProjectorLog.wSensorEco_RLD_G,
				puData->sProjectorLog.wPWMFull_RLD_Y,
				puData->sProjectorLog.wPWMFull_RLD_R,
				puData->sProjectorLog.wPWMFull_RLD_B,
				puData->sProjectorLog.wPWMFull_RLD_G,
				puData->sProjectorLog.wPWMEco_RLD_Y,
				puData->sProjectorLog.wPWMEco_RLD_R,
				puData->sProjectorLog.wPWMEco_RLD_B,
				puData->sProjectorLog.wPWMEco_RLD_G,

				puData->sProjectorLog.wSensorFull_Dynamic_RLD_Y,
				puData->sProjectorLog.wSensorFull_Dynamic_RLD_R,
				puData->sProjectorLog.wSensorFull_Dynamic_RLD_B,
				puData->sProjectorLog.wSensorFull_Dynamic_RLD_G,
				puData->sProjectorLog.wSensorEco_Dynamic_RLD_Y,
				puData->sProjectorLog.wSensorEco_Dynamic_RLD_R,
				puData->sProjectorLog.wSensorEco_Dynamic_RLD_B,
				puData->sProjectorLog.wSensorEco_Dynamic_RLD_G,
				puData->sProjectorLog.wFull_PWM_Dynamic_RLD_Y,
				puData->sProjectorLog.wFull_PWM_Dynamic_RLD_R,
				puData->sProjectorLog.wFull_PWM_Dynamic_RLD_B,
				puData->sProjectorLog.wFull_PWM_Dynamic_RLD_G,
				puData->sProjectorLog.wEco_PWM_Dynamic_RLD_Y,
				puData->sProjectorLog.wEco_PWM_Dynamic_RLD_R,
				puData->sProjectorLog.wEco_PWM_Dynamic_RLD_B,
				puData->sProjectorLog.wEco_PWM_Dynamic_RLD_G,

				 puData->sProjectorLog.uiRGBGain_R,
				 puData->sProjectorLog.uiRGBGain_G,
				 puData->sProjectorLog.uiRGBGain_B,
				 puData->sProjectorLog.uiRGBOffset_R,
				 puData->sProjectorLog.uiRGBOffset_G,
				 puData->sProjectorLog.uiRGBOffset_B,
				 puData->sProjectorLog.uiYUVGain_Y,
				 puData->sProjectorLog.uiYUVGain_U,
				 puData->sProjectorLog.uiYUVGain_V,
				 puData->sProjectorLog.uiYUVOffset_Y,
				 puData->sProjectorLog.uiYUVOffset_U,
				 puData->sProjectorLog.uiYUVOffset_V,
				 puData->sProjectorLog.uiRGBGain2_R,
				 puData->sProjectorLog.uiRGBGain2_G,
				 puData->sProjectorLog.uiRGBGain2_B,
				 puData->sProjectorLog.uiRGBOffset2_R,
				 puData->sProjectorLog.uiRGBOffset2_G,
				 puData->sProjectorLog.uiRGBOffset2_B,
				 puData->sProjectorLog.uiYUVGain2_Y,
				 puData->sProjectorLog.uiYUVGain2_U,
				 puData->sProjectorLog.uiYUVGain2_V,
				 puData->sProjectorLog.uiYUVOffset2_Y,
				 puData->sProjectorLog.uiYUVOffset2_U,
				 puData->sProjectorLog.uiYUVOffset2_V,

				puData->sProjectorLog.cLightSensorTime_Y,
				puData->sProjectorLog.cLightSensorTime_R,
				puData->sProjectorLog.cLightSensorTime_B,
				puData->sProjectorLog.cLightSensorTime_G,
				puData->sProjectorLog.wLightSensorOffset_Y,
				puData->sProjectorLog.wLightSensorOffset_R,
				puData->sProjectorLog.wLightSensorOffset_B,
				puData->sProjectorLog.wLightSensorOffset_G,
				puData->sProjectorLog.fLightSensorGain_Y,
				puData->sProjectorLog.fLightSensorGain_R,
				puData->sProjectorLog.fLightSensorGain_B,
				puData->sProjectorLog.fLightSensorGain_G,
				puData->sProjectorLog.wLightSensorTarget_Y,
				puData->sProjectorLog.wLightSensorTarget_R,
				puData->sProjectorLog.wLightSensorTarget_B,
				puData->sProjectorLog.wLightSensorTarget_G,
				puData->sProjectorLog.wLightSensorTargetRLD_Y,
				puData->sProjectorLog.wLightSensorTargetRLD_R,
				//puData->sProjectorLog.wLightSensorTargetRLD_B,
				//puData->sProjectorLog.wLightSensorTargetRLD_G,
				puData->sProjectorLog.cLightSensorCalibStatus

				);

                utilOPD_Write2ProjectorFile(ucEvent, str);
            }
            break;

        case eOPD_DEVICE_LOG:
            {
                snprintf(str, 1023, "%s,%s,%d,%d,%d,%d,%2.2f", puData->sDev.cDevName, puData->sDev.cDevInfo, puData->sDev.cPara1, puData->sDev.cPara2, puData->sDev.cPara3, puData->sDev.cPara4, puData->sDev.fPara5);
                //utilOPD_Write2Buffer(ucEvent, str);
                utilOPD_Write2TestBuffer(puData->sDev.cDevInterface, str);
            }
            break;

		case eOPD_RUNTIME_LOG:
			{
				snprintf(str, 1023, "%s,%d,%d,%d", puData->sRunTime.cDevName, puData->sRunTime.ulRetry, puData->sRunTime.ulError, puData->sRunTime.ulTotal);
				utilOPD_Write2RunTimeBuffer(puData->sRunTime.cDriveType, str);
			}
			break;

        case eOPD_KEY_KEYPAD_LOG:
        case eOPD_KEY_IR_LOG:
        case eOPD_KEY_LAN_LOG:
        case eOPD_KEY_HDBaseT_LOG:
        case eOPD_KEY_CLI_LOG:
        case eOPD_KEY_WIDE_LOG:
        case eOPD_KEY_BACK_LOG:
        case eOPD_CLI_RS232_LOG:
        case eOPD_CLI_TELNET_LOG:
        case eOPD_CLI_HDBASET_LOG:
        case eOPD_TELNET_LOG:
		case eOPD_SYSTEM_STATUS_LOG:
		case eOPD_INPUT_DETECT_LOG:
		case eOPD_MUTEX_LOCKED_LOG:
		case eOPD_GEOMETRY_LOG:
            {
                utilOPD_Write2Buffer(ucEvent, puData->cString);
                //utilOPD_Write2File();
            }
            break;

		case eOPD_TASK_NO_RESPONSE_LOG:
            {
                utilOPD_Write2Buffer(ucEvent, puData->cString);
                utilOPD_Write2File();
            }
            break;

        case eOPD_ACCESS_DATA_GUI:
        case eOPD_ACCESS_DATA_LAN:
        case eOPD_ACCESS_DATACODE_LOG:
            {
                snprintf(str, 1023, "%s,%s", puData->sDataCode.acDataCodeName, puData->sDataCode.acString);
                utilOPD_Write2Buffer(ucEvent, str);
		    }
            break;

        case eOPD_SOURCE_LOG:
            {
                snprintf(str, 1023, "%s,%s,%d,%d,%d,%d,%d,%d,%s,%s,%d",
                puData->sSource.cInput,
                puData->sSource.cSourceName,
                puData->sSource.wActiveH,
                puData->sSource.wActiveV,
                puData->sSource.wTotalH,
                puData->sSource.wTotalV,
                puData->sSource.wStartH,
                puData->sSource.wStartV,
                puData->sSource.cPixelClock,
                puData->sSource.cColorSpace,
                puData->sSource.cDeepColor);

                utilOPD_Write2Buffer(ucEvent, str);
            }
            break;

        case eOPD_FrontEnd_LOG: //G100_Steven_0059 start
        	{

	            snprintf(str, 1023, "%s,%s,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",//2+52

				m_sOPDDataInfo.sRegulatoryInfo.cSourceName[0],
				m_sOPDDataInfo.sRegulatoryInfo.cSourceName[1],
				puData->sFE_Info.cPIPEn,
				puData->sFE_Info.cMainCH,
				puData->sFE_Info.cSubCH,
				puData->sFE_Info.cVideoReady,
				puData->sFE_Info.cBackupInpEn,
				puData->sFE_Info.cBackupInpAc,
				puData->sFE_Info.cFPGASW,
				puData->sFE_Info.c6634RX,
				puData->sFE_Info.c6634TX,
				puData->sFE_Info.c6805_0_St,
				puData->sFE_Info.c6805_1_St,
				puData->sFE_Info.c6634RstCnt,
				puData->sFE_Info.c6805RstCnt,
				puData->sFE_Info.c7604_St,

				puData->sFE_Info.cCH1_VideoReady,
				puData->sFE_Info.sCH1_VideoTiming.u32VideoPCLK,
				puData->sFE_Info.sCH1_VideoTiming.u16VideoHTotal,
				puData->sFE_Info.sCH1_VideoTiming.u16VideoHActive,
				puData->sFE_Info.sCH1_VideoTiming.u16VideoVTotal,
				puData->sFE_Info.sCH1_VideoTiming.u16VideoVActive,
				puData->sFE_Info.sCH1_VideoTiming.u16VideoVRate,
				puData->sFE_Info.sCH1_VideoFormat.u8VideoColorDepth,
				puData->sFE_Info.sCH1_VideoFormat.u8VideoColorSpace,
				puData->sFE_Info.sCH1_VideoFormat.u8VideoDynamicRange,
				puData->sFE_Info.sCH1_VideoFormat.u8VideoScanning,
				puData->sFE_Info.sCH1_VideoFormat.u8VideoAVIInfoDet,
				puData->sFE_Info.sCH1_VideoFormat.u8Video3DFormat,
				puData->sFE_Info.sCH1_VideoFormat.u8VideoVGASyncType,
				puData->sFE_Info.sCH1_VideoFormat.u8VideoColorYUV,
				puData->sFE_Info.sCH1_VideoFormat.u8VideoDownScaling,
				puData->sFE_Info.sCH1_VideoFormat.u8VideoDualPixelMode,
				puData->sFE_Info.sCH1_VideoFormat.u8VideoHDRType,
				puData->sFE_Info.sCH1_VideoFormat.u8VideoExtentedColorimetry,
				puData->sFE_Info.cCH2_VideoReady,
				puData->sFE_Info.sCH2_VideoTiming.u32VideoPCLK,
				puData->sFE_Info.sCH2_VideoTiming.u16VideoHTotal,
				puData->sFE_Info.sCH2_VideoTiming.u16VideoHActive,
				puData->sFE_Info.sCH2_VideoTiming.u16VideoVTotal,
				puData->sFE_Info.sCH2_VideoTiming.u16VideoVActive,
				puData->sFE_Info.sCH2_VideoTiming.u16VideoVRate,
				puData->sFE_Info.sCH2_VideoFormat.u8VideoColorDepth,
				puData->sFE_Info.sCH2_VideoFormat.u8VideoColorSpace,
				puData->sFE_Info.sCH2_VideoFormat.u8VideoDynamicRange,
				puData->sFE_Info.sCH2_VideoFormat.u8VideoScanning,
				puData->sFE_Info.sCH2_VideoFormat.u8VideoAVIInfoDet,
				puData->sFE_Info.sCH2_VideoFormat.u8Video3DFormat,
				puData->sFE_Info.sCH2_VideoFormat.u8VideoVGASyncType,
				puData->sFE_Info.sCH2_VideoFormat.u8VideoColorYUV,
				puData->sFE_Info.sCH2_VideoFormat.u8VideoDownScaling,
				puData->sFE_Info.sCH2_VideoFormat.u8VideoDualPixelMode,
				puData->sFE_Info.sCH2_VideoFormat.u8VideoHDRType,
				puData->sFE_Info.sCH2_VideoFormat.u8VideoExtentedColorimetry);

	            utilOPD_Write2Buffer(ucEvent, str);
				utilOPD_Write2SourceFile(ucEvent, str); //G100_Julie_0046
        	}
        	break; //G100_Steven_0059 end

        case eOPD_AVI_INFOFRAME_LOG:
            {
                UINT16 uiCount = 0;
                INT16 iStrintCount = 0;

                iStrintCount = sprintf(str, "RxPort%d,", puData->sAVI_Info.cRxPort);

                for(uiCount = 0; uiCount < 14; uiCount++)
                {
                    iStrintCount += sprintf(str + iStrintCount, ",%02x", puData->sAVI_Info.cAVIInfo[uiCount]);
                }

                iStrintCount += sprintf(str + iStrintCount, ",");
                for(uiCount = 0; uiCount < 8; uiCount++)
                {
                    iStrintCount += sprintf(str + iStrintCount, ",%02x", puData->sAVI_Info.cVSII[uiCount]);
                }

                iStrintCount += sprintf(str + iStrintCount, ",");
                for(uiCount = 0; uiCount < 16; uiCount++)
                {
                    iStrintCount += sprintf(str + iStrintCount, ",%02x", puData->sAVI_Info.cHDRInfo[uiCount]);
                }

                utilOPD_Write2Buffer(ucEvent, str);
            }
            break;

		case eOPD_FW_VERSION_LOG: //G100_Julie_0044
			{
				snprintf(str, 1023, "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s",

				puData->sVERSION.cFMCU,
				puData->sVERSION.cSMCU,
				puData->sVERSION.cFRONTEND,
				puData->sVERSION.cFORMATER,
				puData->sVERSION.cHDBASET,
				puData->sVERSION.cKEYPAD,
				puData->sVERSION.cLD_DRIVER,
				puData->sVERSION.cFIRMWARE,
				puData->sVERSION.cMEMC,
				puData->sVERSION.cMOTOR,
				puData->sVERSION.cLAN,
				puData->sVERSION.cFPGA1,
				puData->sVERSION.cFPGA2,
				puData->sVERSION.cFPGA3,
				puData->sVERSION.cPMCU,
				puData->sVERSION.cXFPGA,
				puData->sVERSION.cRELEASE,
				puData->sVERSION.c3GSDI,
				puData->sVERSION.cGEOMETRY,
				puData->sVERSION.cCamera);

            	utilOPD_Write2Buffer(ucEvent, str);
			}
			break;

		case eOPD_HW_CHECK_LOG:
			{
				snprintf(str, 1023, "%s",puData->sVERSION.cFPGAID);

            	utilOPD_Write2Buffer(ucEvent, str);
			}
			break;
        case eOPD_INTERFACE_LOG: //G100_Julie_0048
            {
                snprintf(str, 1023, "%s", puData->sINTERFACE.cString);
                utilOPD_Write2InterfaceLogFile(puData->sINTERFACE.cOPD_Type, str);
            }
            break;

		case eOPD_VOLTAGE_DETECT:
			{
                snprintf(str, 1023, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
									 puData->awData[0], puData->awData[1], puData->awData[2], puData->awData[3], puData->awData[4],
									  puData->awData[5], puData->awData[6], puData->awData[7], puData->awData[8], puData->awData[9],
									   puData->awData[10], puData->awData[11], puData->awData[12], puData->awData[13], puData->awData[14]);

				utilOPD_Write2Buffer(ucEvent, str);
				//printf("(func:%s, line:%d): str(%s)\r\n", __FUNCTION__, __LINE__, str);
				utilOPD_Write2File();
			}
			break;

        case eOPD_LENS_CALIBRATION_LOG:
            {
				snprintf(str, 1023, "HP %d,VP %d,HD %d,VD %d,ZP %d,FP %d,ZD %d,FD %d,ZoomMax %d,ZoomMin %d,FocusMax %d,FocusMin %d,MaxH %d,MinH %d,MaxV %d,MinV %d,BacklashH %d, V %d",
                                    puData->sLensCalibrationInfo.wLensHPosition,
                                    puData->sLensCalibrationInfo.wLensVPosition,
                                    puData->sLensCalibrationInfo.cDirH,
                                    puData->sLensCalibrationInfo.cDirV,
                                    puData->sLensCalibrationInfo.wZoomPosition,
                                    puData->sLensCalibrationInfo.wFocusPosition,
                                    puData->sLensCalibrationInfo.cDirZoom,
                                    puData->sLensCalibrationInfo.cDirFocus,
                                    puData->sLensCalibrationInfo.wZoomMaximumValue,
                                    puData->sLensCalibrationInfo.wZoomMinimumValue,
                                    puData->sLensCalibrationInfo.wFocusMaximumValue,
                                    puData->sLensCalibrationInfo.wFocusMinimumValue,
                                    puData->sLensCalibrationInfo.ulActiveMaxValueH,
                                    puData->sLensCalibrationInfo.ulActiveMinValueH,
                                    puData->sLensCalibrationInfo.ulActiveMaxValueV,
                                    puData->sLensCalibrationInfo.ulActiveMinValueV,
                                    puData->sLensCalibrationInfo.wBacklashH,
                                    puData->sLensCalibrationInfo.wBacklashV);

            	utilOPD_Write2Buffer(ucEvent, str);
			}
			break;

        case eOPD_LENS_MEMORY_LOG:
            {
				snprintf(str, 1023, "%d, HP %d,VP %d,HD %d,VD %d,ZP %d,FP %d,ZD %d,FD %d",
                                    puData->sLensMemInfo.cIndex,
                                    puData->sLensMemInfo.wLensHPosition,
                                    puData->sLensMemInfo.wLensVPosition,
                                    puData->sLensMemInfo.cDirH,
                                    puData->sLensMemInfo.cDirV,
                                    puData->sLensMemInfo.wZoomPosition,
                                    puData->sLensMemInfo.wFocusPosition,
                                    puData->sLensMemInfo.cDirZoom,
                                    puData->sLensMemInfo.cDirFocus);

            	utilOPD_Write2Buffer(ucEvent, str);
			}
			break;

        case eOPD_LENS_OPTICA_CENTER_LOG:
            {
				snprintf(str, 1023, "%d HP %d,VP %d,HD %d,VD %d",
                                    puData->sLensOpticalCenter.ucLensCenterSetting,
                                    puData->sLensOpticalCenter.wLensHPosition,
                                    puData->sLensOpticalCenter.wLensVPosition,
                                    puData->sLensOpticalCenter.cDirH,
                                    puData->sLensOpticalCenter.cDirV);

            	utilOPD_Write2Buffer(ucEvent, str);
			}
			break;

        case eOPD_LIGHT_SENSOR_ERROR_LOG:
            {
                snprintf(str, 1023, "%d,%d,%d,%d",
                puData->sSnapshotLog.wLightSenosr[0],
				puData->sSnapshotLog.wLightSenosr[1],
				puData->sSnapshotLog.wLightSenosr[2],
				puData->sSnapshotLog.wLightSenosr[3]);

                utilOPD_Write2Buffer(ucEvent, str);
            }
            break;

        case eOPD_HDBT_STATUS_LOG:
            {
                snprintf(str, 1023, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
                puData->sSnapshotLog.ulHDBT_Status[0],puData->sSnapshotLog.ulHDBT_Status[1],puData->sSnapshotLog.ulHDBT_Status[2],puData->sSnapshotLog.ulHDBT_Status[3]
                ,puData->sSnapshotLog.ulHDBT_Status[4],puData->sSnapshotLog.ulHDBT_Status[5],puData->sSnapshotLog.ulHDBT_Status[6],puData->sSnapshotLog.ulHDBT_Status[7]
                ,puData->sSnapshotLog.ulHDBT_Status[8],puData->sSnapshotLog.ulHDBT_Status[9],puData->sSnapshotLog.ulHDBT_Status[10]);

                utilOPD_Write2Buffer(ucEvent, str);
            }
            break;

        case eOPD_INFOFRAME_CHANGE_LOG:
            {
                snprintf(str, 1023, "%d",
                puData->sSnapshotLog.ulINFO_Frame_Change);

                utilOPD_Write2Buffer(ucEvent, str);
            }
            break;

        case eOPD_SOURCE_LOST_LOG:
            {
                snprintf(str, 1023, "%d",
                puData->sSnapshotLog.ulSource_Lost);

                utilOPD_Write2Buffer(ucEvent, str);
            }
            break;

        case eOPD_DEBUG_LOG:
            {
                snprintf(str, 1023, "%d",
                puData->sSnapshotLog.ulDebug);

                utilOPD_Write2Buffer(ucEvent, str);
            }
            break;

        case eOPD_MCU_SYSTEM_STATE_LOG:
            {
                snprintf(str, 1023, "%d,", puData->ulMCU_SYSTEM_STATE);
                utilOPD_Write2Buffer(ucEvent, str);
            }
            break;

    }

    return 0;
}

void utilOPD_Initialization(void)
{
	FILE *pFileFd;
    UINT8 cCount = 0;

    if(pthread_mutex_init(&m_OPDMutex, NULL) != 0)
    {
        printf("(func:%s, line:%d): Create xTaskMutex Fail!\r\n", __FUNCTION__, __LINE__);
    }

    if(pthread_mutex_init(&m_SysCallMutex, NULL) != 0)
    {
        printf("(func:%s, line:%d): Create xTaskMutex Fail!\r\n", __FUNCTION__, __LINE__);
    }

    if(access(OPD_LOG_PATH, 0)==-1)//access函式是檢視檔案是不是存在
	{
		if(mkdir(OPD_LOG_PATH, 0777))//如果不存在就用mkdir函式來建立
		{
	        printf("(%s, %d) Creat file bag failed!!!", __FUNCTION__, __LINE__);
		}
	}

	pFileFd = fopen(OPD_VERSION_TXT, "w");
	if(pFileFd)
	{
	    fprintf(pFileFd, "V%02d.%02d\n", OPD_VER_MAJOR, OPD_VER_MINOR);
	    fclose(pFileFd);
    }

	srand(time(NULL));

	snprintf(m_sOPDDataInfo.sRegulatoryInfo.cModelName, 31, "None");
	snprintf(m_sOPDDataInfo.sRegulatoryInfo.cSN, 31, "None");
	snprintf(m_sOPDDataInfo.sRegulatoryInfo.cLanMacAdd, 31, "None");
	m_sOPDDataInfo.sRegulatoryInfo.cInputSource[0] = 0;
	m_sOPDDataInfo.sRegulatoryInfo.cInputSource[1] = 0;
	for(cCount = 0; cCount < eCM_PICTURE_SETTINGS_NUMBER; cCount++)
	{
		m_sOPDDataInfo.sRegulatoryInfo.cPictureMode[cCount] = 0;
	}

	for(cCount = 0; cCount < 8; cCount++)
	{
	    char cString[64] = {'\0'};

	    memset(m_cOPDTemperatureName[cCount], '\0', sizeof(m_cOPDTemperatureName[cCount]));
        Syscfg_Value_Get((eSYSTEM_CFG_ITEM)(eOPD_Temperature_0 + cCount), (void*)cString);
    	snprintf(m_cOPDTemperatureName[cCount], 36, "%s", cString);
	}

	//printf("(%s, %d)\n", __FUNCTION__, __LINE__);

    utilOPD_CleanBuffer();
    utilOPD_Test_CleanBuffer();
	utilOPD_RunTime_CleanBuffer();
}

int utilOPD_CLI_EventSet(UINT8 ucEvent, char *pcString)
{
    switch(ucEvent)
    {
		case eOPD_KEY_KEYPAD_LOG:
		case eOPD_KEY_IR_LOG:
		case eOPD_KEY_LAN_LOG:
		case eOPD_KEY_HDBaseT_LOG:
		case eOPD_KEY_CLI_LOG:
		case eOPD_KEY_WIDE_LOG:
        case eOPD_KEY_BACK_LOG:
		case eOPD_CLI_RS232_LOG:
		case eOPD_CLI_TELNET_LOG:
		case eOPD_CLI_HDBASET_LOG:
        case eOPD_TELNET_LOG:
            utilOPD_Write2Buffer(ucEvent, pcString);
            break;

        default:
            break;
    }

    return 1;
}

void utilOPD_ResetAllToDefault(void) //G100_Julie_0024
{
	if(access("/mnt/configs/opd", 0) == 0)
	{
		OPD_SYSTEM_CALL("rm -fr %s", "/mnt/configs/opd");
	}
	if(access("/mnt/configs/errorlog", 0) == 0)
	{
		OPD_SYSTEM_CALL("rm -fr %s", "/mnt/configs/errorlog");
	}

	if(access(OPD_LOG_PATH, 0) == 0)
	{
		OPD_SYSTEM_CALL("rm -fr %s", OPD_LOG_PATH);
	}
    if(access(OPD_LOG_PATH, 0)==-1)//access函式是檢視檔案是不是存在
	{
	    mkdir(OPD_LOG_PATH, 0777);//不存在就用mkdir函式來建立
	}

	if(access(ERROR_LOG_PATH, 0) == 0)
	{
		OPD_SYSTEM_CALL("rm -fr %s", ERROR_LOG_PATH);
	}
	if(access(ERROR_LOG_PATH, 0) == -1)
	{
		mkdir(ERROR_LOG_PATH, 0777);
	}

	if(access(SST_INFO_PATH, 0) == 0)
	{
		OPD_SYSTEM_CALL("rm -fr %s", SST_INFO_PATH);
	}
    if(access(SST_INFO_PATH, 0) == -1)
    {
        mkdir(SST_INFO_PATH, 0777);
    }

	if(access(UPGRADE_LOG_PATH, 0) == 0)  //G100_Julie_0040
	{
		OPD_SYSTEM_CALL("rm -fr %s", UPGRADE_LOG_PATH);
	}
    if(access(UPGRADE_LOG_PATH, 0) == -1)
    {
        mkdir(UPGRADE_LOG_PATH, 0777);
    }

	if(access(BIST_FILE_PATH, 0) == 0)
	{
		OPD_SYSTEM_CALL("rm -fr %s", BIST_FILE_PATH);
	}
    if(access(BIST_FILE_PATH, 0) == -1)
    {
        mkdir(BIST_FILE_PATH, 0777);
    }

	if(access(IMXAP_LOG_PATH, 0) == 0)  //G100_Julie_0042
	{
		OPD_SYSTEM_CALL("rm -fr %s", IMXAP_LOG_PATH);
	}
    if(access(IMXAP_LOG_PATH, 0) == -1)
    {
        mkdir(IMXAP_LOG_PATH, 0777);
    }
}

// ==============================================================================
// FUNCTION NAME: utilOPD_CopyToUsb
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
// 2020/08/31, Larry Create
// --------------------
// ==============================================================================
int utilOPD_CopyToUsb(void)
{
    if(access(USB_DEV_PATH, 0) == -1)
    {
        printf("usb not found\r\n");
        return 1;
    }
    else
    {
        char temp[128] = {'\0'};
        char filename[64] = {'\0'};
		UINT8 ucString[20] = {0}, ucString_model[20] = {0};
		char cMAC[20] = {'\0'}, str[3], *endptr;
		UINT8 acMAC[6] = {0}, ucIndex = 0 ;

		UINT16 wtyear = 0, wtmon = 0, wtday = 0, wthour = 0, wtmin = 0, wtsec = 0;
		utilOPD_RTCtimer_Get(&wtyear, &wtmon, &wtday, &wthour, &wtmin, &wtsec);

        printf("usb is ready\r\n");

        sprintf(filename, "OPD_%s_%s_%s_%d%d%d", m_sOPDDataInfo.sRegulatoryInfo.cModelName, m_sOPDDataInfo.sRegulatoryInfo.cSN, m_sOPDDataInfo.sRegulatoryInfo.cLanMacAdd, wtyear, wtmon, wtday);

        sprintf(temp, "%s/%s", USB_DEV_PATH, filename);

        if(access(temp, 0) == -1)
        {
        }
        else
        {
			OPD_SYSTEM_CALL("rm -r %s", temp);
        }

		OPD_SYSTEM_CALL("mkdir -m 755 %s", temp);
		OPD_SYSTEM_CALL("cp -R %s %s;sync", OPD_LOG_PATH, temp);
		OPD_SYSTEM_CALL("cp -R %s %s;sync", UPGRADE_LOG_PATH, temp);
		OPD_SYSTEM_CALL("cp -R %s %s;sync", IMXAP_LOG_PATH, temp);

		OPD_SYSTEM_CALL("cp -R %s %s;sync", SYSTEM_STORE_PATH, temp);
		OPD_SYSTEM_CALL("cp -R %s %s;sync", GUI_STORE_PATH, temp);
		OPD_SYSTEM_CALL("cp -R %s %s;sync", WAP_STORE_PATH, temp);
		OPD_SYSTEM_CALL("cp -R %s %s;sync", BIST_FILE_PATH, temp);
		//OPD_SYSTEM_CALL("sync");

		printf("usb copy file done.\r\n");
    }

    return 0;
}

void utilOPD_TarProgress_Init(void)
{
	memset(m_ulPartition, 0, sizeof(m_ulPartition));
	g_ulTotalSize = 0;
}

int utilOPD_TarProgress_AddProgress(UINT8 ucPartIndex, char* pPath, char* pFilePrefix)
{
	FILE *pTempFd = NULL;
	char cTempCmd[512] = {'\0'};
	char cPartSize[128] = {'\0'};
	UINT16 uiIndex;
	UINT32 ulPartSize;

	if(ucPartIndex >=  eOPD_PACKET_PROGRESS_MAX)
	{
		printf("OPD add unknown Progress: %d\r\n", ucPartIndex);
		return -1;
	}

	if(access(pPath, R_OK) != 0)
	{
		printf("OPD Path not found: %s\r\n", pPath);
		return -1;
	}

	memset(cTempCmd, '\0', sizeof(cTempCmd));
	memset(cPartSize, '\0', sizeof(cPartSize));

	snprintf(cTempCmd, sizeof(cTempCmd),
		"find %s -iname \"%s*\" -type f -exec stat -c%%s {} \\; | awk '{sum+=$1} END {print sum}'",
 	   	pPath, pFilePrefix);

	pTempFd = popen(cTempCmd, "r");
	if(!pTempFd)
	{
	    printf("OPD popen failed\n");
	    return -1;
	}

	if(fgets(cPartSize, sizeof(cPartSize), pTempFd))
	{
	    cPartSize[strcspn(cPartSize, "\n")] = 0;
	    if(strspn(cPartSize, "0123456789") == strlen(cPartSize))
	    {
	        m_ulPartition[ucPartIndex] = strtoul(cPartSize, NULL, 10);
	    }
	    else
	    {
	        m_ulPartition[ucPartIndex] = 0;
	    }
	}
	else
	{
	    m_ulPartition[ucPartIndex] = 0;
	}

	g_ulTotalSize = 0;
	for(uiIndex = 0; uiIndex < eOPD_PACKET_PROGRESS_MAX; uiIndex++)
	{
		g_ulTotalSize += m_ulPartition[uiIndex];
	}

	pclose(pTempFd);
	return 0;
}

int utilOPD_TarProgress_AdvAddProgress(UINT8 ucPartIndex, char* pPath, char* pFilePrefix, char* pFileSuffix, UINT8 ucFolderDepth)
{
	FILE *pTempFd = NULL;
	char cTempCmd[512] = {'\0'};
	char cPartSize[128] = {'\0'};
	UINT16 uiIndex;
	UINT32 ulPartSize;

	if(ucPartIndex >=  eOPD_PACKET_PROGRESS_MAX)
	{
		printf("OPD add unknown Progress: %d\r\n", ucPartIndex);
		return -1;
	}

	if(access(pPath, R_OK) != 0)
	{
		printf("OPD Path not found: %s\r\n", pPath);
		return -1;
	}

	memset(cTempCmd, '\0', sizeof(cTempCmd));
	memset(cPartSize, '\0', sizeof(cPartSize));

	snprintf(cTempCmd, sizeof(cTempCmd),
		"find %s -maxdepth %d -type f -iname \"%s*%s\" -exec stat -c%%s {} \\; | awk '{s+=$1} END {print s}'",
		pPath, ucFolderDepth, pFilePrefix, pFileSuffix);

	pTempFd = popen(cTempCmd, "r");
	if(!pTempFd)
	{
	    printf("OPD popen failed\n");
	    return -1;
	}

	if(fgets(cPartSize, sizeof(cPartSize), pTempFd))
	{
	    cPartSize[strcspn(cPartSize, "\n")] = 0;
	    if(strspn(cPartSize, "0123456789") == strlen(cPartSize))
	    {
	        m_ulPartition[ucPartIndex] = strtoul(cPartSize, NULL, 10);
	    }
	    else
	    {
	        m_ulPartition[ucPartIndex] = 0;
	    }
	}
	else
	{
	    m_ulPartition[ucPartIndex] = 0;
	}

	g_ulTotalSize = 0;
	for(uiIndex = 0; uiIndex < eOPD_PACKET_PROGRESS_MAX; uiIndex++)
	{
		g_ulTotalSize += m_ulPartition[uiIndex];
	}

	pclose(pTempFd);
	return 0;
}

int utilOPD_TarProgress_WriteProgress(UINT8 ucPartStatus, UINT8 ucPartIndex)
{
	FILE *pDownloadFp = NULL;
	static UINT64 m_sullTempSum = 0;
	UINT64 ullCalProgress = 0;
	UINT8 ucIndex;

	if(ucPartIndex >= eOPD_PACKET_PROGRESS_MAX)
	{
		printf("OPD write unknown Progress: %d\r\n", ucPartIndex);
		return -1;
	}
	switch(ucPartStatus)
	{
		case eOPD_PACKET_STATUS_PROGRESS:
			{
				// Progress: 0~90%
				if(g_ulTotalSize == 0)
				{
					ullCalProgress = 0;
				}
				else
				{
					if(ucPartIndex == eOPD_PACKET_PROGRESS_OPD_START)
						m_sullTempSum = 0;
					else
						m_sullTempSum += m_ulPartition[ucPartIndex];

					if(m_sullTempSum > g_ulTotalSize)
						m_sullTempSum = g_ulTotalSize;

		    		ullCalProgress = (UINT64)((UINT64)(m_sullTempSum*100)/(UINT64)g_ulTotalSize);
			    	ullCalProgress = (UINT64)((UINT64)(ullCalProgress*9)/(UINT64)10);
				}
			}
			break;
		case eOPD_PACKET_STATUS_TAR:
			{
				// Progress: 90~100%
				ullCalProgress = 90;
			}
			break;
		case eOPD_PACKET_STATUS_DONE:
			{
				ullCalProgress = 100;
			}
			break;
		case eOPD_PACKET_STATUS_FINISH:
		default:
			{
				ullCalProgress = 0;
			}
			break;
	}

	if(utilOPD_MutexTake())
	{
		pDownloadFp = fopen(DOWNLOAD_OPD_STATUS, "wb");

		if(pDownloadFp != NULL)
		{
		    fprintf(pDownloadFp, "{\n");
		    fprintf(pDownloadFp, "\"Status\": \"SUCCESS\",\n");
		    fprintf(pDownloadFp, "\"ErrMsg\": \"\",\n");
		    fprintf(pDownloadFp, "\"Stage\": \"%d\",\n", ucPartStatus);
		    fprintf(pDownloadFp, "\"PartIndex\": \"%d\",\n", ucPartIndex);
		    fprintf(pDownloadFp, "\"TotalProgress\": \"%llu\"\n", ullCalProgress);
		    fprintf(pDownloadFp, "}\n");

	    	fclose(pDownloadFp);
		}
		else
		{
			perror("Failed to access download status");
		}
	}
	utilOPD_MutexGive();
    return 0;
}

void utilOPD_PackageTarPadFile(void)
{
    char TarSwapCmd[512] = {0};
    char TarGeometryCmd[512] = {0};

    if(access(OPD_PAD_SWAP_PATH, F_OK) == 0)
    {
        snprintf(TarSwapCmd, sizeof(TarSwapCmd), "tar -cvf %s/%s %s > /dev/null 2>&1;", IMXAP_LOG_PATH, OPD_PAD_SWAP_FILE_NAME, OPD_PAD_SWAP_PATH);
        (void)system(TarSwapCmd);
    }
    if(access(OPD_PAD_GEOMETRY_PATH, F_OK) == 0)
    {
        snprintf(TarGeometryCmd, sizeof(TarGeometryCmd), "tar -czvf %s/%s %s > /dev/null 2>&1;", IMXAP_LOG_PATH, OPD_PAD_GEOMETRY_FILE_NAME, OPD_PAD_GEOMETRY_PATH);
        (void)system(TarGeometryCmd);
    }
}

void utilOPD_PackageTarFile(UINT8 ucSplash, UINT8 ucData) //G100_Julie_0027 //G100_Julie_00020
{
	char temp[128] = {'\0'}, cTemp[1024] = {'\0'}, String[32] = {'\0'};
	char filename[64] = {'\0'}, cPassword[16] = {'\0'};

	int size = 0, cFile = 0;
	UINT8 ucPack, ucString[20] = {0}, ucString_model[20] = {0};
	char cMAC[20] = {'\0'}, str[3], *endptr;
	UINT8 acMAC[6] = {0}, ucIndex = 0;
	UINT16 wtyear = 0, wtmon = 0, wtday = 0, wthour = 0, wtmin = 0, wtsec = 0;
	utilOPD_RTCtimer_Get(&wtyear, &wtmon, &wtday, &wthour, &wtmin, &wtsec);

	OPD_SYSTEM_CALL("find /*.opd -type f -exec rm -r {} +; sync");

	if(access(OPD_TAR_PATH, 0)==-1)//access函式是檢視檔案是不是存在
	{
		if(mkdir(OPD_TAR_PATH, 0777))//如果不存在就用mkdir函式來建立
		{
			//printf("(%s, %d) Creat file bag failed!!!", __FUNCTION__, __LINE__);
		}
	}
	else
	{
		OPD_SYSTEM_CALL("rm -fr %s/*; sync", OPD_TAR_PATH); //delet tar sub-folder
	}

    utilOPD_PackageTarPadFile();

	utilOPD_TarProgress_Init();

	utilOPD_TarProgress_AddProgress(eOPD_PACKET_PROGRESS_OPD_LOG, 		OPD_LOG_PATH, 			"opd_log");
	utilOPD_TarProgress_AddProgress(eOPD_PACKET_PROGRESS_OPD_SELF_TEST, OPD_LOG_PATH, 			"opd_self_test");
	utilOPD_TarProgress_AddProgress(eOPD_PACKET_PROGRESS_OPD_ENGINE, 	OPD_LOG_PATH, 			"opd_engine");
	utilOPD_TarProgress_AddProgress(eOPD_PACKET_PROGRESS_OPD_PROJECTOR, OPD_LOG_PATH, 			"opd_projector");
	utilOPD_TarProgress_AddProgress(eOPD_PACKET_PROGRESS_OPD_RUNTIME, 	OPD_LOG_PATH, 			"opd_run_time");
	utilOPD_TarProgress_AddProgress(eOPD_PACKET_PROGRESS_OPD_SOURCE, 	OPD_LOG_PATH, 			"opd_source");
	utilOPD_TarProgress_AddProgress(eOPD_PACKET_PROGRESS_OPD_INTERFACE, OPD_LOG_PATH, 			"opd_interface_log");
	utilOPD_TarProgress_AddProgress(eOPD_PACKET_PROGRESS_UPGRADE, 		UPGRADE_LOG_PATH, 		"");
	utilOPD_TarProgress_AddProgress(eOPD_PACKET_PROGRESS_IMXAP, 		IMXAP_LOG_PATH, 		"");
	utilOPD_TarProgress_AddProgress(eOPD_PACKET_PROGRESS_SYSTEMINFO, 	SYSTEM_STORE_PATH, 		"");
	utilOPD_TarProgress_AddProgress(eOPD_PACKET_PROGRESS_GUISTORE, 		GUI_STORE_PATH, 		"");
	utilOPD_TarProgress_AddProgress(eOPD_PACKET_PROGRESS_SOURCESTORE, 	SOURCE_STORE_PATH, 		"");
	utilOPD_TarProgress_AddProgress(eOPD_PACKET_PROGRESS_WAP_STORE, 	WAP_STORE_PATH, 		"");
	utilOPD_TarProgress_AddProgress(eOPD_PACKET_PROGRESS_BIST, 	        BIST_FILE_PATH, 		"");
	utilOPD_TarProgress_AddProgress(eOPD_PACKET_PROGRESS_COLORTABLE, 	COLOR_TABLE_PATH, 		"");
	utilOPD_TarProgress_AddProgress(eOPD_PACKET_PROGRESS_COLORTABLE_FACTORY, 	COLOR_TABLE_FACTORY_PATH, 	"");
	utilOPD_TarProgress_AdvAddProgress(eOPD_PACKET_PROGRESS_NETWORK_SETUP, 		NETWORK_SETUP_PATH, 		"",		"conf",		1);
	utilOPD_TarProgress_AdvAddProgress(eOPD_PACKET_PROGRESS_WEB_ACCOUNT,		NETWORK_WEB_ACCOUNT_PATH,	"", 	"",	     	99);
	utilOPD_TarProgress_AdvAddProgress(eOPD_PACKET_PROGRESS_WEB_LOG,			NETWORK_WEB_PATH,			"", 	"",	     	99);
	utilOPD_TarProgress_AdvAddProgress(eOPD_PACKET_PROGRESS_ADV_WARP_LOG,		ADV_WAPR_PATH,			    "", 	"",	     	99);
	utilOPD_TarProgress_AddProgress(eOPD_PACKET_PROGRESS_PROJECT_SETTING, 	PROJECT_SETTINGS_PATH,	"");

	utilOPD_TarProgress_WriteProgress(eOPD_PACKET_STATUS_PROGRESS,      eOPD_PACKET_PROGRESS_OPD_START);
    //opd file copy
	for(ucPack = eOPD_PACKET_PROGRESS_OPD_LOG; ucPack < eOPD_PACKET_PROGRESS_UPGRADE; ucPack++)
	{
		switch(ucPack)
		{
			//printf("(%s,%d) ucPack[%d]\r\n", __FUNCTION__, __LINE__, ucPack);
			case eOPD_PACKET_PROGRESS_OPD_LOG:
				{
					cFile = OPD_LOG_FILE_NUMBER - 1;
					memcpy(String,"opd_log",strlen("opd_log")+1);
					memcpy(cPassword, m_sOPD_Password_Table[ucSplash].cLog, strlen(m_sOPD_Password_Table[ucSplash].cLog)+1);
				}
				break;

			case eOPD_PACKET_PROGRESS_OPD_SELF_TEST:
				{
					cFile = OPD_SELF_TEST_FILE_NUMBER - 1;
					memcpy(String,"opd_self_test",strlen("opd_self_test")+1);
					memcpy(cPassword, m_sOPD_Password_Table[ucSplash].cSelf_Test, strlen(m_sOPD_Password_Table[ucSplash].cSelf_Test)+1);
				}
				break;

			case eOPD_PACKET_PROGRESS_OPD_ENGINE:
				{
					cFile = OPD_ENGINE_FILE_NUMBER - 1;
					memcpy(String,"opd_engine",strlen("opd_engine")+1);
					memcpy(cPassword, m_sOPD_Password_Table[ucSplash].cEngine, strlen(m_sOPD_Password_Table[ucSplash].cEngine)+1);
				}
				break;

			case eOPD_PACKET_PROGRESS_OPD_PROJECTOR:
				{
					cFile = OPD_PROJECTOR_FILE_NUMBER - 1;
					memcpy(String,"opd_projector",strlen("opd_projector")+1);
					memcpy(cPassword, m_sOPD_Password_Table[ucSplash].cProjector, strlen(m_sOPD_Password_Table[ucSplash].cProjector)+1);
				}
				break;

			case eOPD_PACKET_PROGRESS_OPD_RUNTIME:
				{
					cFile = OPD_RUNTIME_TEST_FILE_NUMBER - 1;
					memcpy(String,"opd_run_time",strlen("opd_run_time")+1);
					memcpy(cPassword, m_sOPD_Password_Table[ucSplash].cI2C_RunTime, strlen(m_sOPD_Password_Table[ucSplash].cI2C_RunTime)+1);
				}
				break;

			case eOPD_PACKET_PROGRESS_OPD_SOURCE: //G100_Julie_0046
				{
					cFile = OPD_SOURCE_FILE_NUMBER - 1;
					memcpy(String,"opd_source",strlen("opd_source")+1);
					memcpy(cPassword, m_sOPD_Password_Table[ucSplash].cSource, strlen(m_sOPD_Password_Table[ucSplash].cSource)+1);
				}
				break;

			case eOPD_PACKET_PROGRESS_OPD_INTERFACE: //A65_OPTOMA_Julie_0029
				{
					cFile = OPD_INTERFACE_LOG_NUMBER - 1;
					memcpy(String,"opd_interface_log",strlen("opd_interface_log")+1);
					memcpy(cPassword, m_sOPD_Password_Table[ucSplash].cInterface, strlen(m_sOPD_Password_Table[ucSplash].cInterface)+1);
				}
				break;

			default:
				printf("\r\nASSERT_ALWAYS: %s: %d %d\r\n", __FILE__, __LINE__, ucPack);
				break;
		}

		sprintf(filename, "%s_%04d%02d%02d", String, wtyear, wtmon, wtday);

		sprintf(temp, "%s/%s", OPD_LOG_PATH, filename);

		if(access(temp, 0) == -1)
		{
		}
		else
		{
			OPD_SYSTEM_CALL("rm -r %s", temp);
		}

		OPD_SYSTEM_CALL("mkdir -m 755 %s", temp);

		for(cFile; cFile >= 0; cFile--)
		{
			sprintf(cTemp, "%s/%s_%04d.bin", OPD_LOG_PATH, String, cFile);

			size = file_size(cTemp);

			if(access(cTemp,0)==-1)
			{
				continue;
			}

			OPD_SYSTEM_CALL("cp -f %s %s;sync", cTemp, temp);
		}

		if(file_size(cTemp) == -1)
		{
			//printf("%s NULL\r\n", cTemp);
			OPD_SYSTEM_CALL("rm -r %s/%s;sync", OPD_LOG_PATH, filename); //delet folder
		}
		else
		{
			//printf("%s\r\n", filename);
			OPD_SYSTEM_CALL("zip -rjP[%s] %s.zip %s/%s;sync", cPassword, filename, OPD_LOG_PATH, filename); //zip folder
			OPD_SYSTEM_CALL("mv -f %s.zip %s;sync", filename, OPD_TAR_PATH); //move zip file to tar file
			OPD_SYSTEM_CALL("rm -r %s/%s;sync", OPD_LOG_PATH, filename); //delet folder
		}
		utilOPD_TarProgress_WriteProgress(eOPD_PACKET_STATUS_PROGRESS, ucPack);
	}
	//opd file end

	//Upgrade file copy  //G100_Julie_0040
	memcpy(String,"Upgrade",strlen("Upgrade")+1);
	memcpy(cPassword, m_sOPD_Password_Table[ucSplash].cUpgrade, strlen(m_sOPD_Password_Table[ucSplash].cUpgrade)+1);

	sprintf(filename, "%s_%04d%02d%02d", String, wtyear, wtmon, wtday);
	OPD_SYSTEM_CALL("zip -rjP[%s] %s.zip %s;sync", cPassword, filename, UPGRADE_LOG_PATH); //zip folder
	OPD_SYSTEM_CALL("mv -f %s.zip %s;sync", filename, OPD_TAR_PATH); //move zip file to tar file
	utilOPD_TarProgress_WriteProgress(eOPD_PACKET_STATUS_PROGRESS, eOPD_PACKET_PROGRESS_UPGRADE);
	//Upgrade file end

    //imxap file copy  //G100_Julie_0042
	memcpy(String,"imxAP",strlen("imxAP")+1);

	sprintf(filename, "%s_%04d%02d%02d", String, wtyear, wtmon, wtday);
	OPD_SYSTEM_CALL("zip -rjP[%s] %s.zip %s;sync", cPassword, filename, IMXAP_LOG_PATH); //zip folder
	OPD_SYSTEM_CALL("mv -f %s.zip %s;sync", filename, OPD_TAR_PATH); //move zip file to tar file
	utilOPD_TarProgress_WriteProgress(eOPD_PACKET_STATUS_PROGRESS, eOPD_PACKET_PROGRESS_IMXAP);
	//imxap file end

#if 0
	//SystemCal file copy  //A65_OPTOMA_Doulas_0105
	memcpy(String,"SystemCal",strlen("SystemCal")+1);

	sprintf(filename, "%s_%d%d%d", String, wtyear, wtmon, wtday);
	OPD_SYSTEM_CALL("zip -rjP[%s] %s.zip %s;sync", cPassword, filename, "/mnt/configs/scaler/system/SystemCal.conf"); //zip folder
	OPD_SYSTEM_CALL("mv -f %s.zip %s;sync", filename, OPD_TAR_PATH); //move zip file to tar file
	//SystemCal file end
#endif /* 0 */

	//SystemInfo file copy  //A65_OPTOMA_Doulas_0105
	memcpy(String,"SystemInfo",strlen("SystemInfo")+1);

	sprintf(filename, "%s_%04d%02d%02d", String, wtyear, wtmon, wtday);
	OPD_SYSTEM_CALL("zip -rjP[%s] %s.zip %s;sync", cPassword, filename, SYSTEM_STORE_PATH); //zip folder
	OPD_SYSTEM_CALL("mv -f %s.zip %s;sync", filename, OPD_TAR_PATH); //move zip file to tar file
	utilOPD_TarProgress_WriteProgress(eOPD_PACKET_STATUS_PROGRESS, eOPD_PACKET_PROGRESS_SYSTEMINFO);
	//SystemInfo file end

	//GUIStore file copy
	memcpy(String,"GUIStore",strlen("GUIStore")+1);

	sprintf(filename, "%s_%04d%02d%02d", String, wtyear, wtmon, wtday);
	OPD_SYSTEM_CALL("zip -rjP[%s] %s.zip %s;sync", cPassword, filename, GUI_STORE_PATH); //zip folder
	OPD_SYSTEM_CALL("mv -f %s.zip %s;sync", filename, OPD_TAR_PATH); //move zip file to tar file
	utilOPD_TarProgress_WriteProgress(eOPD_PACKET_STATUS_PROGRESS, eOPD_PACKET_PROGRESS_GUISTORE);
	//GUIStore file end

	//SourceStore file copy
	memcpy(String,"SourceStore",strlen("SourceStore")+1);

	sprintf(filename, "%s_%04d%02d%02d", String, wtyear, wtmon, wtday);
	OPD_SYSTEM_CALL("zip -rjP[%s] %s.zip %s;sync", cPassword, filename, SOURCE_STORE_PATH); //zip folder
	OPD_SYSTEM_CALL("mv -f %s.zip %s;sync", filename, OPD_TAR_PATH); //move zip file to tar file
	utilOPD_TarProgress_WriteProgress(eOPD_PACKET_STATUS_PROGRESS, eOPD_PACKET_PROGRESS_SOURCESTORE);
	//SOURCMStore file end

	//WAPStore file copy  //A65_OPTOMA_Doulas_0105
	memcpy(String,"WAPStore",strlen("WAPStore")+1);

	sprintf(filename, "%s_%04d%02d%02d", String, wtyear, wtmon, wtday);
	OPD_SYSTEM_CALL("zip -rjP[%s] %s.zip %s;sync", cPassword, filename, WAP_STORE_PATH); //zip folder
	OPD_SYSTEM_CALL("mv -f %s.zip %s;sync", filename, OPD_TAR_PATH); //move zip file to tar file
	utilOPD_TarProgress_WriteProgress(eOPD_PACKET_STATUS_PROGRESS, eOPD_PACKET_PROGRESS_WAP_STORE);
	//WAPStore file end

	//BIST file copy
	memcpy(String,"BIST",strlen("BIST")+1);
	memcpy(cPassword, m_sOPD_Password_Table[ucSplash].cBIST, strlen(m_sOPD_Password_Table[ucSplash].cBIST)+1);

	sprintf(filename, "%s_%04d%02d%02d", String, wtyear, wtmon, wtday);
	OPD_SYSTEM_CALL("zip -rjP[%s] %s.zip %s;sync", cPassword, filename, BIST_FILE_PATH); //zip folder
	OPD_SYSTEM_CALL("mv -f %s.zip %s;sync", filename, OPD_TAR_PATH); //move zip file to tar file
	utilOPD_TarProgress_WriteProgress(eOPD_PACKET_STATUS_PROGRESS, eOPD_PACKET_PROGRESS_BIST);
	//BIST file end

	//color table copy
    if(access(COLOR_TABLE_PATH, R_OK) == 0)
    {
    	memcpy(String,"ColorTable",strlen("ColorTable")+1);

    	sprintf(filename, "%s_%04d%02d%02d", String, wtyear, wtmon, wtday);
    	OPD_SYSTEM_CALL("zip -rjP[%s] %s.zip %s;sync", cPassword, filename, COLOR_TABLE_PATH); //zip folder
    	OPD_SYSTEM_CALL("mv -f %s.zip %s;sync", filename, OPD_TAR_PATH); //move zip file to tar file
    	utilOPD_TarProgress_WriteProgress(eOPD_PACKET_STATUS_PROGRESS, eOPD_PACKET_PROGRESS_COLORTABLE);
    }
	//color table end

	//color table factory copy
	if(access(COLOR_TABLE_FACTORY_PATH, R_OK) == 0)
    {
    	memcpy(String,"ColorTableFactory",strlen("ColorTableFactory")+1);

    	sprintf(filename, "%s_%04d%02d%02d", String, wtyear, wtmon, wtday);
    	OPD_SYSTEM_CALL("zip -rjP[%s] %s.zip %s;sync", cPassword, filename, COLOR_TABLE_FACTORY_PATH); //zip folder
    	OPD_SYSTEM_CALL("mv -f %s.zip %s;sync", filename, OPD_TAR_PATH); //move zip file to tar file
    	utilOPD_TarProgress_WriteProgress(eOPD_PACKET_STATUS_PROGRESS, eOPD_PACKET_PROGRESS_COLORTABLE);
    }
	//color table factory end

	//network setup copy
	snprintf(String, 32, "%s", "NetworkSetup");

	snprintf(filename, 64, "%s_%04d%02d%02d", String, wtyear, wtmon, wtday);
	OPD_SYSTEM_CALL(
        "find %s -maxdepth 1 -type f -name '*.conf' ! -name 'notifications.conf' -print0 | xargs -0 zip -j -P[%s] %s.zip || true; sync",
	    NETWORK_SETUP_PATH, cPassword, filename
    );
	OPD_SYSTEM_CALL("mv -f %s.zip %s;sync", filename, OPD_TAR_PATH); //move zip file to tar file
	utilOPD_TarProgress_WriteProgress(eOPD_PACKET_STATUS_PROGRESS, eOPD_PACKET_PROGRESS_NETWORK_SETUP);
	//network setup end

	//network account copy
	snprintf(String, 32, "%s", "NetworkAccount");

	snprintf(filename, 64, "%s_%04d%02d%02d", String, wtyear, wtmon, wtday);
	OPD_SYSTEM_CALL("zip -rP[%s] %s.zip %s/* || true; sync", cPassword, filename, NETWORK_WEB_ACCOUNT_PATH); //zip folder
	OPD_SYSTEM_CALL("mv -f %s.zip %s;sync", filename, OPD_TAR_PATH); //move zip file to tar file
	utilOPD_TarProgress_WriteProgress(eOPD_PACKET_STATUS_PROGRESS, eOPD_PACKET_PROGRESS_WEB_ACCOUNT);
	//network account end

	//network log copy
	if(access(NETWORK_WEB_PATH, R_OK) == 0)
	{
		snprintf(String, 32, "%s", "NetworkLog");

		snprintf(filename, 64, "%s_%04d%02d%02d", String, wtyear, wtmon, wtday);
		OPD_SYSTEM_CALL("zip -rP[%s] %s.zip %s;sync", cPassword, filename, NETWORK_WEB_PATH); //zip folder
		OPD_SYSTEM_CALL("mv -f %s.zip %s;sync", filename, OPD_TAR_PATH); //move zip file to tar file
		utilOPD_TarProgress_WriteProgress(eOPD_PACKET_STATUS_PROGRESS, eOPD_PACKET_PROGRESS_WEB_LOG);
	}
	//network log end

	//Warp file
	if(access(ADV_WAPR_PATH, R_OK) == 0)
	{
    	memcpy(String,"AdvWarpStore",strlen("AdvWarpStore")+1);

    	sprintf(filename, "%s_%04d%02d%02d", String, wtyear, wtmon, wtday);
    	OPD_SYSTEM_CALL("zip -rjP[%s] %s.zip %s;sync", cPassword, filename, ADV_WAPR_PATH); //zip folder
    	OPD_SYSTEM_CALL("mv -f %s.zip %s;sync", filename, OPD_TAR_PATH); //move zip file to tar file
    	utilOPD_TarProgress_WriteProgress(eOPD_PACKET_STATUS_PROGRESS, eOPD_PACKET_PROGRESS_ADV_WARP_LOG);
    }
	//Warp file end

	//ProjectSettings copy
	if(access(PROJECT_SETTINGS_PATH, R_OK) == 0)
	{
    	memcpy(String,"ProjectSettings",strlen("ProjectSettings")+1);

    	sprintf(filename, "%s_%04d%02d%02d", String, wtyear, wtmon, wtday);
    	OPD_SYSTEM_CALL("zip -rjP[%s] %s.zip %s;sync", cPassword, filename, PROJECT_SETTINGS_PATH); //zip folder
    	OPD_SYSTEM_CALL("mv -f %s.zip %s;sync", filename, OPD_TAR_PATH); //move zip file to tar file
    	utilOPD_TarProgress_WriteProgress(eOPD_PACKET_STATUS_PROGRESS, eOPD_PACKET_PROGRESS_PROJECT_SETTING);
    }
	//ProjectSettings copy end

	OPD_SYSTEM_CALL("cp -f %s %s;sync", ERROR_LOG_TXT, OPD_TAR_PATH);
	OPD_SYSTEM_CALL("cp -f %s %s;sync", SST_INFO_TXT, OPD_TAR_PATH);
	OPD_SYSTEM_CALL("cp -f %s %s;sync", MODEL_CHANGE_LOG_TXT, OPD_TAR_PATH);

	//sprintf(temp, "%s_%s_%s_%d%d%d", m_sOPDDataInfo.sRegulatoryInfo.cModelName, m_sOPDDataInfo.sRegulatoryInfo.cSN, m_sOPDDataInfo.sRegulatoryInfo.cLanMacAdd, wtyear, wtmon, wtday);
	sprintf(temp, "%s_%s_%04d%02d%02d%02d%02d%02d", m_sOPDDataInfo.sRegulatoryInfo.cModelName, m_sOPDDataInfo.sRegulatoryInfo.cSN, wtyear, wtmon, wtday, wthour, wtmin, wtsec);//YYYYMMDDHHmmSS

	OPD_SYSTEM_CALL("tar -cvPf '%s'.tar -C%s/ tar; sync", temp, OPD_LOG_PATH); //.tar
	utilOPD_TarProgress_WriteProgress(eOPD_PACKET_STATUS_TAR, 0);

	switch(ucData)
	{
	    case OPD_COPY_2_USB: //usb
	        if(access(USB_DEV_PATH, 0) == -1)
            {
                printf("usb not found\r\n");
            }
            else
            {
                OPD_SYSTEM_CALL("mv '%s'.tar %s; sync", temp, USB_DEV_PATH); // copy to usb
            }
			break;

		case OPD_COPY_2_WEB: //web
			OPD_SYSTEM_CALL("mv '%s'.tar %s; sync", temp, "/tmp/");
			break;

        default:
            break;
	}

	OPD_SYSTEM_CALL("rm -fr %s;sync", OPD_TAR_PATH); //delet folder

	printf("usb copy file done.\r\n");
	utilOPD_TarProgress_WriteProgress(eOPD_PACKET_STATUS_DONE, 0);
	sleep(2);
	utilOPD_TarProgress_WriteProgress(eOPD_PACKET_STATUS_FINISH, 0);
}

void utilOPD_SetSourceName(UINT8 cMain, UINT8 cSub) //A65_OPTOMA_Julie_0081 //G100_Steven_0059
{
    uOPD_DATA uOPDData = {0};

	if(cMain < eCM_SOURCE_NUMBER)
	{
		m_sOPDDataInfo.sRegulatoryInfo.cInputSource[0] = cMain;
		snprintf(m_sOPDDataInfo.sRegulatoryInfo.cSourceName[0], 31, "%s", m_sSource_String[cMain]);
	}
	else
	{
		m_sOPDDataInfo.sRegulatoryInfo.cInputSource[0] = eCM_SOURCE_RESERVED;
		sprintf((char *)m_sOPDDataInfo.sRegulatoryInfo.cSourceName[0], "NA");
	}

	if(cSub < eCM_SOURCE_NUMBER)
	{
		m_sOPDDataInfo.sRegulatoryInfo.cInputSource[1] = cSub;
		snprintf(m_sOPDDataInfo.sRegulatoryInfo.cSourceName[1], 31, "%s", m_sSource_String[cSub]);
	}
	else
	{
		m_sOPDDataInfo.sRegulatoryInfo.cInputSource[1] = eCM_SOURCE_RESERVED;
		sprintf((char *)m_sOPDDataInfo.sRegulatoryInfo.cSourceName[1], "NA");
	}

}//G100_Steven_0059

int utilOPD_OPDFile_ReplaceSN(char *pcString)
{
        char cStrTemp[64] = {'\0'};
        snprintf(cStrTemp, sizeof(cStrTemp), "%s", pcString);

	FILE *pFile, *pFile2;
	unsigned long size = 0;
	unsigned short offset = 0;
	char *buffer;
	char text[1024];
	char cTemp[1024] = {'\0'};

	//File copy and modify string.
	utilOPD_Write2File();
	utilOPD_Write2TestFile();
	utilOPD_Write2RunTimeFile();

	usleep(100);

	if(utilOPD_MutexTake())
	{
		char cTempFile[64] = {"/mnt/syslog/opd/temp.bin"};

		for(UINT8 cFileType = 0; cFileType < OPD_FILE_TYPE; cFileType++)
		{
			UINT16 cFileEndNum[8] = {OPD_LOG_FILE_NUMBER,
									OPD_SELF_TEST_FILE_NUMBER,
									OPD_ENGINE_FILE_NUMBER,
									OPD_PROJECTOR_FILE_NUMBER,
									OPD_RUNTIME_TEST_FILE_NUMBER,
									OPD_SOURCE_FILE_NUMBER,
									OPD_INTERFACE_LOG_NUMBER,
									0};

			char cFileTextName[8][64] = {"/mnt/syslog/opd/opd_log",
										 "/mnt/syslog/opd/opd_self_test",
										 "/mnt/syslog/opd/opd_engine",
										 "/mnt/syslog/opd/opd_projector",
										 "/mnt/syslog/opd/opd_run_time",
										 "/mnt/syslog/opd/opd_source",
										 "/mnt/syslog/opd/opd_interface_log",
										 ""};

			for(UINT16 cFileNum = 0; cFileNum < cFileEndNum[cFileType]; cFileNum++)
			{
				sprintf(cTemp, "%s_%04d.bin", cFileTextName[cFileType], cFileNum);

				if(access(cTemp,0) == 0)
				{
					pFile = fopen(cTemp, "r");
					pFile2 = fopen(cTempFile, "w");

					if(pFile != NULL)
					{
                                                fseek(pFile, 0, SEEK_END);
                                                size = ftell(pFile);
                                                fseek(pFile, 0, SEEK_SET);
                                                buffer = malloc(size + 1);
                                                size_t readCount = fread(buffer, size, 1, pFile);
                                                (void)readCount;
                                                fseek(pFile, 0, SEEK_SET);

						offset = 0;
						for(char count = 0; count < 2; count++)
						{
                                                        if (fgets(text, 1023, pFile) == NULL)
                                                        {
                                                            break;
                                                        }
                                                        offset += strlen(text);

							if(count == 1)
							{
								sprintf(text, "%s,%s\n", OPD_SERIAL_NUMBER, cStrTemp);
							}
							fwrite(text, strlen(text), 1, pFile2);
						}

						fclose(pFile);
						fwrite(&buffer[offset], (size - offset), 1, pFile2);

						fclose(pFile2);
						free(buffer);
					}
				}

				if(access(cTempFile,0) == 0)
				{
					OPD_SYSTEM_CALL("rm -r %s;sync", cTemp); //delet folder

					if (rename(cTempFile, cTemp) == 0) //rename (old,new)
						printf("Temp File rename \"%s\" pass.\n", cTemp);
					else
						perror("rename");
				}

			}
		}

		if(access(cTempFile, 0) == 0)
		{
			if(remove(cTempFile) == 0)
			{
				//printf("Remove %s pass\n", cTempFile);
			}
		}

		utilOPD_MutexGive();
	}


	#if 0
	//Linux cmd popen and sed replace.
	FILE * fp;
	if ((fp = popen("ls -a /mnt/syslog/opd", "r")) == NULL)
	{
		perror("open failed!");
		return -1;
	}

	char but[256];
	while (fgets(but, 255, fp) != NULL)
	{
		printf("%s", but);
		char file1[64] = {"/mnt/syslog/opd/opd_log_0000.bin"};
		char file2[64] = {"/mnt/syslog/opd/opd_self_test_0000.bin"};
		char file3[64] = {"/mnt/syslog/opd/opd_engine_0000.bin"};
		char file4[64] = {"/mnt/syslog/opd/opd_projector_0000.bin"};
		char file5[64] = {"/mnt/syslog/opd/opd_run_time_0000.bin"};
		char file6[64] = {"/mnt/syslog/opd/opd_source_0000.bin"};
		char file7[64] = {"/mnt/syslog/opd/opd_interface_log_0000.bin"};

		if ((strcmp(but, file1) == 0) || (strcmp(but, file2) == 0) || (strcmp(but, file3) == 0) || (strcmp(but, file4) == 0) || (strcmp(but, file5) == 0) || (strcmp(but, file6) == 0) || (strcmp(but, file7) == 0))
		{
			if(utilOPD_MutexTake())
			{
				OPD_SYSTEM_CALL("sed -i \"2c %s,%s\" %s/%s", OPD_SERIAL_NUMBER, cStrTemp, OPD_LOG_PATH, but);
			}

			utilOPD_MutexGive();
		}
		else
		{
			OPD_SYSTEM_CALL("sed -i \"2c %s,%s\" %s/%s", OPD_SERIAL_NUMBER, cStrTemp, OPD_LOG_PATH, but);
		}
	}

	if (pclose(fp) == -1)
	{
		perror("close failed!");
		return -2;
	}
	#endif

    return 0;
}


