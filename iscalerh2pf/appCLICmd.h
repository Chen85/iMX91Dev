#ifndef _APPCLICMD_H_
#define _APPCLICMD_H_

#ifdef __cplusplus
extern "C" {
#endif


//#include "utilStorageCfg.h"
#include "Common.h"
#include "appMailBox.h"

#if (SYSTEM_OS_TYPE == POSIX_COMPLIANT)
    typedef struct
    {
        pthread_t                       xTaskHandle;       // Task
        pthread_attr_t                  xTaskHandle_attr;  // Task attribute
        mqd_t                           xMsgQueue;         // queue descriptors
        struct mq_attr                  xMsgQueue_attr;    // queue attribute

        EventGroupHandle_t              xEventGroupHandle; //unused

        INT32							lPollPeriod;       // polling period

    }sAPP_CLICMD_INFORMATION, *PsAPP_CLICMD_INFORMATION;
#else  //freertos
    typedef struct
    {

        TaskHandle_t					xTaskHandle;
        xQueueHandle                    xMsgQueue;
        EventGroupHandle_t              xEventGroupHandle;

    #if(CURRENT_RTOS_TYPE == RTOS_STATIC)
        StaticQueue_t                   xStaticQueue;
        UINT8                           ucQueueStorageArea[MAIL_BOX_QUEUE_SIZE * sizeof(sMAIL_BOX_MESSAGE)];
        StaticEventGroup_t              xCreatedEventGroup;
        StaticTask_t                    xTaskBuffer;
        StackType_t                     xStack[APP_CLI_STACK_SIZE];
    #endif

        INT32							lPollPeriod;

    }sAPP_CLICMD_INFORMATION, *PsAPP_CLICMD_INFORMATION;
#endif

eEXEC_CODE appCLICmd_Init(void);
void appCLICmd_SerialPortEcho(UINT8 ucEcho); //A70LV_Larry_0060
void appCLICmd_ProjectorAddress(UINT8 ucAddress);



#ifdef __cplusplus
}
#endif






#endif  //_APPCLICMD_H_
