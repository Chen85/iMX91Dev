#ifndef _palMailBox_H_
#define _palMailBox_H_

#ifdef __cplusplus
extern "C" {
#endif


#include "Common.h"


//====================== Enum Start ======================//
typedef enum
{
    eMAIL_BOX_EXEC_CODE_FAIL,                       /* general message failure */
    eMAIL_BOX_EXEC_CODE_PASS,                       /* successful operation */
    eMAIL_BOX_EXEC_CODE_EVENT,                      /* failed to allocate RTOS event */
    eMAIL_BOX_EXEC_CODE_FAIL_SEND,                  /* RTOS failure in sending RTOS message */
    eMAIL_BOX_EXEC_CODE_FAIL_RECV,                  /* RTOS failure in receiving RTOS message */
    eMAIL_BOX_EXEC_CODE_NOMSG,                      /* timeout waiting for message */
    eMAIL_BOX_EXEC_CODE_NORESP                      /* timeout waiting for response */
}eMAIL_BOX_EXEC_CODE;

//====================== Enum End ======================//




//===================== Struct Start ===================//
#if (SYSTEM_OS_TYPE == POSIX_COMPLIANT)

typedef struct
{
    UINT16                  uiMsgID;
    UINT8                   ucWait;
    UINT8                   ucDispose;
    UINT32                  ulParam1;
    UINT32                  *pulParam2;
    pthread_mutex_t         *MailBoxMutex;
    pthread_cond_t          *MailBoxCondWait;
    //pthread_t               threadID;
}sMAIL_BOX_MESSAGE, *PsMAIL_BOX_MESSAGE;

#else  //freertos

typedef struct
{
    UINT16                  uiMsgID;
    UINT8                   ucWait;
    UINT8                   ucDispose;
    UINT32                  ulParam1;
    UINT32                  *pulParam2;
    EventGroupHandle_t      hEventGroupHandle;
}sMAIL_BOX_MESSAGE, *PsMAIL_BOX_MESSAGE;

#endif

//===================== Struct End ===================//


#if (SYSTEM_OS_TYPE == POSIX_COMPLIANT)
eMAIL_BOX_EXEC_CODE palMailBox_Create_MsgQueue(mqd_t *pmQueueHandle, char *cName, struct mq_attr *pmQueueHandle_attr);
eMAIL_BOX_EXEC_CODE palMailBox_Send_MsgQueue(mqd_t mQueueHandle, char *cName, UINT16 uiMsgID, INT32 lBlockTime, UINT32 ulParam1, UINT32 *pulParam2);
eMAIL_BOX_EXEC_CODE palMailBox_Receive_MsgQueue(mqd_t mQueueHandle, char *cName, INT32 lBlockTimeMs, void (*Callback)(UINT16 ulMsgID, UINT32 ulParam1, UINT32 *pulParam2));

#else  //freertos

eMAIL_BOX_EXEC_CODE palMailBox_Create(xQueueHandle* pxQueueHandle, EventGroupHandle_t* pxEventGroupHandle);

eMAIL_BOX_EXEC_CODE palMailBox_Send(xQueueHandle xQueueHandle,
                        EventGroupHandle_t xEventGroupHandle,
                        UINT16 uiMsgID, INT32 lBlockTime,
                        UINT32 ulParam1, UINT32 *pulParam2, BOOL bDispose);

eMAIL_BOX_EXEC_CODE palMailBox_Receive(xQueueHandle xQueueHandle,
                        INT32 lBlockTime,
                        void (*Callback)(UINT16 ulMsgID, UINT32 ulParam1, UINT32 *pulParam2));
#endif

#if 0
eMAIL_BOX_EXEC_CODE palMailBox_Create_Static(xQueueHandle* pxQueueHandle,
                                    EventGroupHandle_t* pxEventGroupHandle,
                                    StaticQueue_t* pxStaticQueue,
                                    PUINT8 pQueueStorageArea,
                                    StaticEventGroup_t* pxCreatedEventGroup);
#endif /* 0 */



#ifdef __cplusplus
}
#endif






#endif  //_palMailBox_H_
