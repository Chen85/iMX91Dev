#ifndef _palCoreVar_H_
#define _palCoreVar_H_


#ifdef __cplusplus
extern "C" {
#endif


#include "Common.h"

typedef struct
{
	UINT32 isSystemOn							:1;
    UINT32 isLightSource                        :1;
    UINT32 isBurnInOnMode					    :1;
    UINT32 isCooling                            :1;
}sCORE_VAR, *PsCORE_VAR;



typedef struct
{
#if (SYSTEM_OS_TYPE == POSIX_COMPLIANT)
    pthread_mutex_t              xMutex ;
#else
    SemaphoreHandle_t            xSemaphore;
    #if(CURRENT_RTOS_TYPE == RTOS_STATIC)
    StaticSemaphore_t            xMutexBuffer;
    #endif
#endif

    sCORE_VAR                    sCoreVar;

}sPAL_CORE_VAR_INFORMATION, *PsPAL_CORE_VAR_INFORMATION;


BOOL palCoreVar_GetSystemPowerOnVar(void);
eEXEC_CODE palCoreVar_SetSystemPowerOnVar(BOOL bValue);

BOOL palCoreVar_GetLightSourceOnVar(void);
eEXEC_CODE palCoreVar_SetLightSourceOnVar(BOOL bValue);

BOOL palCoreVar_GetBurnInOnModeVar(void);
eEXEC_CODE palCoreVar_SetBurnInOnModeVar(BOOL bValue);

BOOL palCoreVar_GetIsCoolingVar(void);
eEXEC_CODE palCoreVar_SetIsCoolingVar(BOOL bValue);


eEXEC_CODE palCoreVar_Init(void);


#ifdef __cplusplus
}
#endif




#endif
