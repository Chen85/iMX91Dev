
#include "type_def.h"

//Blend AP is using
#define SHM_BLEND_AP_LINK_STATE        *utilProcMutexData_IS_BLEND_AP_LINK_ShmPtrGet()

//warp osd(before 0 / after 1) is draw ?
#define SHM_DRAW_WARPOSD_STATE(x)      *utilProcMutexData_IS_DRAW_WARPOSD_ShmPtrGet(x)


typedef enum
{
    ePMD_PASS,

    ePMD_NOT_READY,

    ePMD_INVLAID,
}ePMD_RESULT;

ePMD_RESULT utilProcMutexData_Init(void);
ePMD_RESULT utilProcMutexData_ScalerRegDataRecord(const UINT32 regEnum, const UINT32 data);

ePMD_RESULT utilProcMutexData_InhibitColorEnable_RegSet(const UINT32 regEnum);
ePMD_RESULT utilProcMutexData_InhibitColorEnable_DataSet(INT8 ucData);
INT8 utilProcMutexData_InhibitColorEnable_DataGet(void);

ePMD_RESULT utilProcMutexData_InhibitColorIdx_RegSet(const UINT32 regEnum);
ePMD_RESULT utilProcMutexData_InhibitColorIdx_DataSet(UINT16 ucData);
UINT16 utilProcMutexData_InhibitColorIdx_DataGet(void);
void utilProcMutexData_OsdPalette_Get(UINT8 *pPalette);
void utilProcMutexData_OsdPalette_Set(UINT8 *pPalette);

int* utilProcMutexData_IS_BLEND_AP_LINK_ShmPtrGet(void);
int* utilProcMutexData_IS_DRAW_WARPOSD_ShmPtrGet(UINT8 WarpOsdLocation);

int* utilProcMutexData_POSD_SAD_ShmPtrGet(void);
int* utilProcMutexData_PWPOSD_SAD_ShmPtrGet(void);
int* utilProcMutexData_PWPOSD_SAD_BEFORE_ShmPtrGet(void);
int* utilProcMutexData_PWPOSD_SAD_AFTER_ShmPtrGet(void);

