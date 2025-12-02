#ifndef _APPPOLL_H_
#define _APPPOLL_H_

#ifdef __cplusplus
extern "C" {
#endif


#include "utilStorageCfg.h"
#include "Common.h"

#define POLL_PERIOD					10//10ms
#define POLL_STARTUP_DELAY          1000

#define POLLING_TIME_TEST           (FALSE)  //A35G2_Simon_0075

#if (SYSTEM_OS_TYPE == POSIX_COMPLIANT)

    typedef struct
    {
        pthread_t                       xTaskHandle;         // Task
        pthread_attr_t                  xTaskHandle_attr;    // Task attribute
        pthread_mutex_t                 xTaskMutex;
        pthread_cond_t                  xTaskCond;
        BOOL                            bTaskPause;

        EventGroupHandle_t              xEventGroupHandle;   //unused

        UINT16                          uiTickCounter;
    }sPAL_POLL_INFORMATION, *PsPAL_POLL_INFORMATION;

#else //freertos

    typedef struct
    {
        TimerHandle_t                   xTimerHandle;
        EventGroupHandle_t              xEventGroupHandle;
        TaskHandle_t					xTaskHandle;

    #if(CURRENT_RTOS_TYPE == RTOS_STATIC)
        StaticTask_t                    xTaskBuffer;
        StackType_t                     xStack[APP_POLL_STACK_SIZE];
        StaticEventGroup_t              xCreatedEventGroup;
        StaticTimer_t                   xTimerBuffers;
    #endif

        UINT16                          uiTickCounter;
    }sPAL_POLL_INFORMATION, *PsPAL_POLL_INFORMATION;

#endif


typedef INT16 (*PP_CALLBACK)(UINT16 uiTick);

void appPoll_Task_Suspend(void);
void appPoll_Task_Resume(void);

eEXEC_CODE appPoll_Init(void);
void appPoll_TaskPriorityHighSet(BOOL bEnable); //A70LV_Larry_0004


#ifdef __cplusplus
}
#endif






#endif  //_APPPOLL_H_
