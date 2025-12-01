#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/file.h>
#include "SharedMemCtrl.h"
#include "hicc_config.h"
#include "ProcessMutexData.h"

sPROCESS_MUTEX_DATA *psPMD = NULL;

#define PMD_LOG(fmt, ...) PMDLog(__FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)

#define ASSERT()          PMD_LOG(" : ASSERT\r\n");

static void PMDLog(const char *FuncName, int LineNum, char *str,  ...)
{
    va_list args;
    va_start(args, str);

    UINT8 strOffset = 0 ;

    char String[256] = {0};
    strOffset = snprintf(String, sizeof(String), "(%s@%d)", FuncName, LineNum);
    vsnprintf(String+strOffset, sizeof(String)-strOffset, str, args);
    printf(String);
    syslog(LOG_INFO, String);
    va_end(args);
}

ePMD_RESULT utilProcMutexData_Init(void)
{
    psPMD = (sPROCESS_MUTEX_DATA *)SharedMem_GetMapPtr(eSB_PROCESS_MUTEX_DATA);

    if(psPMD == NULL)
    {
        PMD_LOG("PMD Initial Fail\r\n");
        return ePMD_NOT_READY;
    }

    //PMD_LOG("PMD Initial Pass\r\n");
    return ePMD_PASS;
}

//Scaler Register Record
ePMD_RESULT utilProcMutexData_ScalerRegDataRecord(const UINT32 regEnum, const UINT32 data)
{
    if(psPMD == NULL)
    {
        ASSERT();
        return ePMD_NOT_READY;
    }

    (regEnum == psPMD->sSCALER_INFO.Reg_InhibitColorEnable) ? psPMD->sSCALER_INFO.Data_InhibitColorEnable = (INT8)data :
    (regEnum == psPMD->sSCALER_INFO.Reg_InhibitColorIdx)    ? psPMD->sSCALER_INFO.Data_InhibitColorIdx = (UINT16)data  :
    (regEnum);

    return ePMD_PASS;
}


//////////////////////////////////
//InhibitColor Enable
//////////////////////////////////
ePMD_RESULT utilProcMutexData_InhibitColorEnable_RegSet(const UINT32 regEnum)
{
    if(psPMD == NULL)
    {
        ASSERT();
        return ePMD_NOT_READY;
    }

    psPMD->sSCALER_INFO.Reg_InhibitColorEnable = regEnum;
    return ePMD_PASS;
}

ePMD_RESULT utilProcMutexData_InhibitColorEnable_DataSet(INT8 ucData)
{
    if(psPMD == NULL)
    {
        ASSERT();
        return ePMD_NOT_READY;
    }

    psPMD->sSCALER_INFO.Data_InhibitColorEnable = ucData;
    return ePMD_PASS;
}

INT8 utilProcMutexData_InhibitColorEnable_DataGet(void)
{
    if(psPMD == NULL)
    {
        ASSERT();
        return 0xFF;
    }

    return psPMD->sSCALER_INFO.Data_InhibitColorEnable;
}


//////////////////////////////////
//InhibitColor Index
//////////////////////////////////
ePMD_RESULT utilProcMutexData_InhibitColorIdx_RegSet(const UINT32 regEnum)
{
    if(psPMD == NULL)
    {
        ASSERT();
        return ePMD_NOT_READY;
    }

    psPMD->sSCALER_INFO.Reg_InhibitColorIdx = regEnum;
    return ePMD_PASS;
}

ePMD_RESULT utilProcMutexData_InhibitColorIdx_DataSet(UINT16 ucData)
{
    if(psPMD == NULL)
    {
        ASSERT();
        return ePMD_NOT_READY;
    }

    psPMD->sSCALER_INFO.Data_InhibitColorIdx = ucData;
    return ePMD_PASS;
}

UINT16 utilProcMutexData_InhibitColorIdx_DataGet(void)
{
    if(psPMD == NULL)
    {
        ASSERT();
        return 0xFFFF;
    }

    return psPMD->sSCALER_INFO.Data_InhibitColorIdx;
}


// Warping Info
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
void utilProcMutexData_OsdPalette_Get(UINT8 *pPalette)
{
    if(psPMD == NULL)
    {
        ASSERT();
        return ;
    }

    memcpy(pPalette, psPMD->sWARPING_INFO.aiBitmapPalette, sizeof(psPMD->sWARPING_INFO.aiBitmapPalette));

}

void utilProcMutexData_OsdPalette_Set(UINT8 *pPalette)
{
    if(psPMD == NULL)
    {
        ASSERT();
        return ;
    }

    memcpy(psPMD->sWARPING_INFO.aiBitmapPalette, pPalette, sizeof(psPMD->sWARPING_INFO.aiBitmapPalette));

}

int* utilProcMutexData_IS_BLEND_AP_LINK_ShmPtrGet(void)
{
    while(psPMD == NULL)
    {
        ASSERT();
    }

    return &psPMD->sWARPING_INFO.SHM_IS_BLEND_AP_LINK;
}

//WarpOsdLocation == 0 : before wap osd
//WarpOsdLocation == 1 : after wap osd
int* utilProcMutexData_IS_DRAW_WARPOSD_ShmPtrGet(UINT8 WarpOsdLocation)
{
    while(psPMD == NULL)
    {
        ASSERT();
    }

    while(WarpOsdLocation >= sizeof(psPMD->sWARPING_INFO.SHM_IS_DRAW_WARPOSD)/sizeof(psPMD->sWARPING_INFO.SHM_IS_DRAW_WARPOSD[0]))
    {
        ASSERT();
    }

    return &psPMD->sWARPING_INFO.SHM_IS_DRAW_WARPOSD[WarpOsdLocation];
}

int* utilProcMutexData_POSD_SAD_ShmPtrGet(void)
{
    while(psPMD == NULL)
    {
        ASSERT();
    }

    return &psPMD->sWARPING_INFO.SHM_POSD_SAD;
}

int* utilProcMutexData_PWPOSD_SAD_ShmPtrGet(void)
{
    while(psPMD == NULL)
    {
        ASSERT();
    }

    return &psPMD->sWARPING_INFO.SHM_PWPOSD_SAD;
}

int* utilProcMutexData_PWPOSD_SAD_BEFORE_ShmPtrGet(void)
{
    while(psPMD == NULL)
    {
        ASSERT();
    }

    return &psPMD->sWARPING_INFO.SHM_PWPOSD_SAD_BEFORE;
}

int* utilProcMutexData_PWPOSD_SAD_AFTER_ShmPtrGet(void)
{
    while(psPMD == NULL)
    {
        ASSERT();
    }

    return &psPMD->sWARPING_INFO.SHM_PWPOSD_SAD_AFTER;
}


