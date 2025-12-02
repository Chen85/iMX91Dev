#include "appMailBox.h"
#include "utilDbgMsg.h"

#if (SYSTEM_OS_TYPE == FREERTOS)
#include "./FreeRTOS_Kernel/include/queue.h"
#endif

//#define DISABLE_OLD_MAILBOX
//#define DISABLE_NEW_MAILBOX

#define MAILBOX_SEND_TIMEOUT        (1000L)
#define MAILBOX_SEMPHORE_TIMEOUT    (5000L)

#if (SYSTEM_OS_TYPE == POSIX_COMPLIANT)

eMAIL_BOX_EXEC_CODE palMailBox_Create_MsgQueue(mqd_t *pmQueueHandle, char *cName, struct mq_attr *pmQueueHandle_attr)
{
#ifdef DISABLE_NEW_MAILBOX
    return eMAIL_BOX_EXEC_CODE_PASS;
#endif

    _FUNCTION_IN_LOG_

    pmQueueHandle_attr->mq_flags   = 0;
    pmQueueHandle_attr->mq_maxmsg  = MAX_MESSAGES;
    pmQueueHandle_attr->mq_msgsize = sizeof(sMAIL_BOX_MESSAGE);
    pmQueueHandle_attr->mq_curmsgs = 0;

    if ((*pmQueueHandle = mq_open(cName, O_RDWR | O_CREAT, 0666, pmQueueHandle_attr)) == -1)
    {
        ASSERT_ALWAYS();
        LOG_MSG(db_APP_MAILBOX, "%s mq_open fail [%s]\n", cName , strerror(errno));

        _FUNCTION_OUT_LOG_
        return eMAIL_BOX_EXEC_CODE_FAIL;
    }

    LOG_MSG(db_APP_MAILBOX, "(func:%s, line:%d)%s created (Queue ID:%d)\n", __FUNCTION__, __LINE__ , cName , *pmQueueHandle);

    _FUNCTION_OUT_LOG_
    return eMAIL_BOX_EXEC_CODE_PASS;
}

eMAIL_BOX_EXEC_CODE palMailBox_Send_MsgQueue(mqd_t mQueueHandle, char *cName, UINT16 uiMsgID, INT32 lBlockTime, UINT32 ulParam1, UINT32 *pulParam2)
{
    _FUNCTION_IN_LOG_

    sMAIL_BOX_MESSAGE sMailBoxMsg;
    sMailBoxMsg.uiMsgID             = uiMsgID;
    sMailBoxMsg.ulParam1            = ulParam1;
    sMailBoxMsg.pulParam2           = pulParam2;
    sMailBoxMsg.ucWait              = (lBlockTime >= 0);
    sMailBoxMsg.MailBoxMutex        = NULL;
    sMailBoxMsg.MailBoxCondWait     = NULL;

    //printf("sizeof(sMailBoxMsg) = %d\n", sizeof(sMailBoxMsg));

    //LOG_MSG(db_APP_MAILBOX, "Send to <%d><%s>(%d)(%ld)(%ld)<%ld> from <%d>\n", mQueueHandle
                                                                             //, cName
                                                                             //, sMailBoxMsg.uiMsgID
                                                                             //, sMailBoxMsg.ulParam1
                                                                             //, *sMailBoxMsg.pulParam2
                                                                             //, lBlockTime
                                                                             //, pthread_self());

#ifdef DISABLE_NEW_MAILBOX
        return eMAIL_BOX_EXEC_CODE_PASS;
#endif

    // if no need to wait receiver done
    if(sMailBoxMsg.ucWait == 0)
    {
        struct timespec s_timeout = {0,0};
        clock_gettime(CLOCK_REALTIME, &s_timeout);
        util_TimespecAddms(&s_timeout, MAILBOX_SEND_TIMEOUT);  // 當 queue 已經 full , 且已經等待 1 秒 , 就回傳錯誤 (: Connection timed out)

        //Start to send mailbox
        if( mq_timedsend(mQueueHandle, (char const *)&sMailBoxMsg, sizeof(sMailBoxMsg), 0, &s_timeout) == -1)
        {
            ASSERT_ALWAYS();

            //: Connection timed out
            LOG_MSG(db_APP_MAILBOX, "%s mq_send fail [%s]\n", cName , strerror(errno));

            _FUNCTION_OUT_LOG_
            return eMAIL_BOX_EXEC_CODE_FAIL;
        }
    }
    else    //need to wait receiver done
    {
        //create temp mutex
        pthread_mutex_t *SyncMutex = NULL;//(pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));

        //create temp cond  //condition variable
        pthread_cond_t  *SyncCond  = NULL;//(pthread_cond_t *)malloc(sizeof(pthread_cond_t));

        SyncMutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));

        if(SyncMutex == NULL)
        {
            return eMAIL_BOX_EXEC_CODE_FAIL;
        }

        SyncCond = (pthread_cond_t *)malloc(sizeof(pthread_cond_t));

        if(SyncCond == NULL)
        {
	        free(SyncMutex);
            return eMAIL_BOX_EXEC_CODE_FAIL;
        }

        //initial temp mutex
        if(pthread_mutex_init(SyncMutex, NULL) == -1)
        {
			free(SyncMutex);
            free(SyncCond);
            ASSERT_ALWAYS();
            LOG_MSG(db_APP_MAILBOX, "%d create temp mutex fail\n", pthread_self());
            _FUNCTION_OUT_LOG_
            return eMAIL_BOX_EXEC_CODE_FAIL;
        }

        //initial temp cond
        if(pthread_cond_init(SyncCond, NULL) == -1)
        {
			free(SyncMutex);
            free(SyncCond);
            ASSERT_ALWAYS();
            LOG_MSG(db_APP_MAILBOX, "%d create temp cond fail\n", pthread_self());
            _FUNCTION_OUT_LOG_
            return eMAIL_BOX_EXEC_CODE_FAIL;
        }

        sMailBoxMsg.MailBoxMutex    = SyncMutex;
        sMailBoxMsg.MailBoxCondWait = SyncCond;

        struct timespec s_MutexTimeout = {0,0};
        clock_gettime(CLOCK_REALTIME, &s_MutexTimeout);
        util_TimespecAddms(&s_MutexTimeout, MAILBOX_SEMPHORE_TIMEOUT);

        //mutex lock
        if(pthread_mutex_timedlock(sMailBoxMsg.MailBoxMutex , &s_MutexTimeout) != 0)
        {
            LOG_MSG(db_ALWAYS, "009 %s pthread_mutex_timedlock fail [%s]\n", __FUNCTION__ , strerror(errno));

            free(SyncMutex);
            free(SyncCond);
            _FUNCTION_OUT_LOG_
            return eMAIL_BOX_EXEC_CODE_FAIL;
        }


        struct timespec s_timeout = {0,0};
        clock_gettime(CLOCK_REALTIME, &s_timeout);
        util_TimespecAddms(&s_timeout, MAILBOX_SEND_TIMEOUT);  // 當 queue 已經 full , 且已經等待 1 秒 , 就回傳錯誤 (: Connection timed out)

        //start to send mailbox
        if( mq_timedsend(mQueueHandle, (char const *)&sMailBoxMsg, sizeof(sMailBoxMsg), 0, &s_timeout) == -1)
        {
            ASSERT_ALWAYS();

            //: Connection timed out
            LOG_MSG(db_APP_MAILBOX, "%s mq_send fail [%s]\n", cName , strerror(errno));

            pthread_mutex_unlock(sMailBoxMsg.MailBoxMutex);
            pthread_mutex_destroy(sMailBoxMsg.MailBoxMutex);
            free(SyncMutex);
            free(SyncCond);

            _FUNCTION_OUT_LOG_
            return eMAIL_BOX_EXEC_CODE_FAIL;
        }

        //wait receiver done
        LOG_MSG(db_APP_MAILBOX, "%d [wait receiver start]\n" , pthread_self());

        //struct timespec s_WaitTime = {0,0};
        //clock_gettime(CLOCK_REALTIME, &s_WaitTime);
        //util_TimespecAddms(&s_WaitTime, lBlockTime);
        //if(pthread_cond_timedwait(sMailBoxMsg.MailBoxCondWait, sMailBoxMsg.MailBoxMutex, &s_WaitTime) != 0)   //wait receiver done
        if(pthread_cond_wait(sMailBoxMsg.MailBoxCondWait, sMailBoxMsg.MailBoxMutex) == -1)   //wait receiver done
        {
            LOG_MSG(db_ALWAYS, "%d pthread_cond_timedwait fail [%s]\n", pthread_self() , strerror(errno));
            LOG_MSG(db_ALWAYS, "%s no receive or time out\n", cName);

            pthread_mutex_unlock (sMailBoxMsg.MailBoxMutex);
            pthread_mutex_destroy(sMailBoxMsg.MailBoxMutex);
            free(SyncMutex);
            free(SyncCond);

            _FUNCTION_OUT_LOG_
            return eMAIL_BOX_EXEC_CODE_FAIL;
        }
        else
        {
            //Normal Condition (receiver already signal)
            pthread_mutex_unlock (sMailBoxMsg.MailBoxMutex);
            pthread_mutex_destroy(sMailBoxMsg.MailBoxMutex);
            free(SyncMutex);
            free(SyncCond);
        }

        LOG_MSG(db_APP_MAILBOX, "%d [%s wait receiver end]\n" , pthread_self() , cName);

    }


    _FUNCTION_OUT_LOG_
    return eMAIL_BOX_EXEC_CODE_PASS;
}

eMAIL_BOX_EXEC_CODE palMailBox_Receive_MsgQueue(mqd_t mQueueHandle, char *cName, INT32 lBlockTimeMs, void (*Callback)(UINT16 ulMsgID, UINT32 ulParam1, UINT32 *pulParam2))
{
#ifdef DISABLE_NEW_MAILBOX
    return eMAIL_BOX_EXEC_CODE_PASS;
#endif

    _FUNCTION_IN_LOG_

    sMAIL_BOX_MESSAGE sMailBoxMsg;
    UINT8 ucRecMailBoxPass = FALSE ;

    if(lBlockTimeMs == 0)   //always block if no receive msg
    {
        if(mq_receive(mQueueHandle, (char *)&sMailBoxMsg, sizeof(sMailBoxMsg), NULL) == -1)
        {
            ASSERT_ALWAYS();
            LOG_MSG(db_APP_MAILBOX, "%s mq_receive fail [%s]\n", cName , strerror(errno));
            _FUNCTION_OUT_LOG_
            return eMAIL_BOX_EXEC_CODE_FAIL;
        }
        else
        {
            ucRecMailBoxPass = TRUE ;
        }
    }
    else  //return No Msg when lBlockTimeMs timeout
    {
        struct timespec s_timeout;
        clock_gettime(CLOCK_REALTIME, &s_timeout);
        util_TimespecAddms(&s_timeout , lBlockTimeMs);

        // lBlockTimeMs 時間 timeout 時 , 且 queue 為 empty , 就回傳 No Msg (: Connection timed out)
        if(mq_timedreceive(mQueueHandle, (char *)&sMailBoxMsg, sizeof(sMailBoxMsg), NULL, &s_timeout) == -1)
        {
            if(errno == ETIMEDOUT)   //Connection timed out    //no message
            {
                //LOG_MSG(db_APP_MAILBOX, "mq_timedreceive timeout [%s][ID:%d] (No Msg)(%ld)\n", cName , mQueueHandle , lBlockTimeMs);
                _FUNCTION_OUT_LOG_
                return eMAIL_BOX_EXEC_CODE_NOMSG;
            }
            else
            {
                ASSERT_ALWAYS();
                LOG_MSG(db_APP_MAILBOX, "mq_timedreceive fail [%s][ID:%d](%s)(%ld)\n", cName , mQueueHandle , strerror(errno), lBlockTimeMs);
                _FUNCTION_OUT_LOG_
                return eMAIL_BOX_EXEC_CODE_FAIL;
            }
        }
        else
        {
            ucRecMailBoxPass = TRUE ;
        }
    }


    //already get mailbox
    if(ucRecMailBoxPass == TRUE)
    {
        //LOG_MSG(db_APP_MAILBOX, "Rec <%d><%s>(%d)(%ld)(%ld)<%d><%ld> by <%d>\n", mQueueHandle
                                                                               //, cName
                                                                               //, sMailBoxMsg.uiMsgID
                                                                               //, sMailBoxMsg.ulParam1
                                                                               //, *sMailBoxMsg.pulParam2
                                                                               //, sMailBoxMsg.ucWait
                                                                               //, lBlockTimeMs
                                                                               //, pthread_self());

        //do callback function
        (*Callback)(sMailBoxMsg.uiMsgID, sMailBoxMsg.ulParam1, sMailBoxMsg.pulParam2);

        //need to signal sender when finishing callback function
        if(sMailBoxMsg.ucWait)
        {
            struct timespec s_MutexTimeout;
            clock_gettime(CLOCK_REALTIME, &s_MutexTimeout);
            util_TimespecAddms(&s_MutexTimeout, MAILBOX_SEMPHORE_TIMEOUT);

            //lock mutex
            if(pthread_mutex_timedlock(sMailBoxMsg.MailBoxMutex, &s_MutexTimeout) != 0)
            {
                LOG_MSG(db_ALWAYS, "010 %s pthread_mutex_timedlock fail [%s]\n", __FUNCTION__ , strerror(errno));
                pthread_mutex_unlock(sMailBoxMsg.MailBoxMutex);
                return eMAIL_BOX_EXEC_CODE_FAIL;
            }

            //tell sender the receiver is finish doing callback function
            pthread_cond_signal(sMailBoxMsg.MailBoxCondWait);
            LOG_MSG(db_APP_MAILBOX, "receiver %d signal done\n", pthread_self());

            //unlock mutex
            if(pthread_mutex_unlock(sMailBoxMsg.MailBoxMutex) != 0)
            {
                LOG_MSG(db_ALWAYS, "%s pthread_mutex_unlock fail [%s]\n", __FUNCTION__ , strerror(errno));
            }
        }

    }


    _FUNCTION_OUT_LOG_
    return eMAIL_BOX_EXEC_CODE_PASS;
}


#else   //freertos


eMAIL_BOX_EXEC_CODE palMailBox_Create(xQueueHandle *pxQueueHandle, EventGroupHandle_t *pxEventGroupHandle)
{
    //LOG_MSG(db_APP_MAILBOX, "(func:%s, line:%d) \r\n", __FUNCTION__, __LINE__);
#ifdef DISABLE_OLD_MAILBOX
    return eMAIL_BOX_EXEC_CODE_PASS;
#endif

    *pxQueueHandle = xQueueCreate(MAIL_BOX_QUEUE_SIZE , sizeof(sMAIL_BOX_MESSAGE));

    if(*pxQueueHandle == NULL)
    {
        return eMAIL_BOX_EXEC_CODE_FAIL;
    }

    if((*pxEventGroupHandle = xEventGroupCreate()) == NULL)
    {
        return eMAIL_BOX_EXEC_CODE_EVENT;
    }

    xEventGroupClearBits(*pxEventGroupHandle,    /* The event group being updated. */
                         BIT_0);         /* The bits being cleared. */

    return eMAIL_BOX_EXEC_CODE_PASS;
}

eMAIL_BOX_EXEC_CODE palMailBox_Send(xQueueHandle xQueueHandle, EventGroupHandle_t xEventGroupHandle, UINT16 uiMsgID, INT32 lBlockTime, UINT32 ulParam1, UINT32 *pulParam2, BOOL bDispose)
{

#ifdef DISABLE_OLD_MAILBOX
    return eMAIL_BOX_EXEC_CODE_PASS;
#endif

    sMAIL_BOX_MESSAGE sMailBoxMsg; /* mailbox message */
    //eMAIL_BOX_EXEC_CODE     eExecCode;                                  /* completion code */
    EventBits_t             uxBits;

    //LOG_MSG(db_APP_MAILBOX, "(func:%s, line:%d) \r\n", __FUNCTION__, __LINE__);
    sMailBoxMsg.uiMsgID = uiMsgID;
    sMailBoxMsg.ucWait = (lBlockTime >= 0);

    if(xQueueHandle == NULL) //A70LV_Larry_0118
    {
        return eMAIL_BOX_EXEC_CODE_FAIL;
    }

    if(sMailBoxMsg.ucWait)                              /* if caller waits for completion */
    {
        sMailBoxMsg.hEventGroupHandle = xEventGroupHandle;
        xEventGroupClearBits(
            xEventGroupHandle,    /* The event group being updated. */
            BIT_0);         /* The bits being cleared. */

    }

    sMailBoxMsg.ucDispose = (UINT8)bDispose;
    sMailBoxMsg.ulParam1 = ulParam1;
    sMailBoxMsg.pulParam2 = pulParam2;

    /****************************************************/
    /* Send the message and wait for complete if needed.*/
    /****************************************************/


    if(xQueueSend(xQueueHandle, &sMailBoxMsg, 50) != pdPASS)
    {
        //LOG_MSG(db_APP_MAILBOX, "(func:%s, line:%d) : Cannot send event[%d] Fail!\r\n", __FUNCTION__, __LINE__, uiMsgID);
		//vPortFree(psMailBoxMsg);
        return eMAIL_BOX_EXEC_CODE_FAIL_SEND;
    }

	//printf("sMailBoxMsg.ucWait %d\n", sMailBoxMsg.ucWait);

    if(sMailBoxMsg.ucWait)                               /* if waiting for complete */
    {
        uxBits = xEventGroupWaitBits(
                     xEventGroupHandle,   /* The event group being tested. */
                     BIT_0,          /* The bits within the event group to wait for. */
                     pdTRUE,         /* BIT_0 should be cleared before returning. */
                     pdFALSE,        /* Don't wait for both bits, either bit will do. */
                     lBlockTime / portTICK_RATE_MS); /* Wait a maximum of 100ms for either bit to be set. */

        if(uxBits != BIT_0)
        {
            //LOG_MSG(db_APP_MAILBOX, "(func:%s, line:%d) : Timeout waiting for response!\r\n", __FUNCTION__, __LINE__, uiMsgID);
			//vPortFree(psMailBoxMsg);
            return eMAIL_BOX_EXEC_CODE_NORESP;
        }
    }

	//vPortFree(psMailBoxMsg);
    return eMAIL_BOX_EXEC_CODE_PASS;
}



/****************************************************************************/
/* Receive an application mailbox message.                                  */
/*                                                                          */
/*       mbxID: Mailbox from which to receive message                       */
/*   blocktime: Milliseconds to wait for message                            */
/*  mbCallback: Callback when message is received                           */
/*       msgID: Mailbox-specific message identifer                          */
/*        parm: Untyped parameter                                           */
/****************************************************************************/

eMAIL_BOX_EXEC_CODE palMailBox_Receive(xQueueHandle xQueueHandle, INT32 lBlockTime, void (*Callback)(UINT16 ulMsgID, UINT32 ulParam1, UINT32 *pulParam2))
{

#ifdef DISABLE_OLD_MAILBOX
    return eMAIL_BOX_EXEC_CODE_PASS;
#endif


    sMAIL_BOX_MESSAGE sMailBoxMsg; /* mailbox message */
    //eMAIL_BOX_EXEC_CODE     eExecCode;
    //EventBits_t             uxBits;
    UINT32                  ulWaitTime = 0;


    if(lBlockTime == 0)
    {
        ulWaitTime = portMAX_DELAY;
    }
    else
    {
        ulWaitTime = lBlockTime;
    }

    /****************************************************/
    /* Handle successful message receive.               */
    /****************************************************/

    if(xQueueReceive(xQueueHandle, &sMailBoxMsg, ulWaitTime) == pdPASS)
    {
        //LOG_MSG(db_APP_MAILBOX, "(func:%s, line:%d): Get Event [%d]!\r\n", __FUNCTION__, __LINE__, sMailBoxMsg.uiMsgID);

        (*Callback)(sMailBoxMsg.uiMsgID, sMailBoxMsg.ulParam1, sMailBoxMsg.pulParam2);

        if(sMailBoxMsg.ucWait)                 /* if sender is waiting for a response */
        {
            /*uxBits = */xEventGroupSetBits(
                         sMailBoxMsg.hEventGroupHandle,     /* The event group being updated. */
                         BIT_0);           /* The bits being set. */
        }

        if(sMailBoxMsg.ucDispose)                       /* if message parm points to disposable memory */
        {
            free((void *)sMailBoxMsg.ulParam1);
        }
    }
    else                                             /* if error in receive */
    {
    	//PortFree(psMailBoxMsg);
        return eMAIL_BOX_EXEC_CODE_NOMSG;
    }

	//vPortFree(psMailBoxMsg);
    return eMAIL_BOX_EXEC_CODE_PASS;
}


#if 0
eMAIL_BOX_EXEC_CODE palMailBox_Create_Static(xQueueHandle *pxQueueHandle,
                                             EventGroupHandle_t *pxEventGroupHandle,
                                             StaticQueue_t *pxStaticQueue,
                                             PUINT8 pQueueStorageArea,
                                             StaticEventGroup_t *pxCreatedEventGroup)
{
    //LOG_MSG(db_APP_MAILBOX, "(func:%s, line:%d) \r\n", __FUNCTION__, __LINE__);


    /* Create a queue capable of containing 10 uint64_t values. */
    *pxQueueHandle = xQueueCreateStatic(MAIL_BOX_QUEUE_SIZE,
                                        sizeof(sMAIL_BOX_MESSAGE),
                                        pQueueStorageArea,
                                        pxStaticQueue);

    if(*pxQueueHandle == NULL)
    {
        return eMAIL_BOX_EXEC_CODE_FAIL;
    }


    if((*pxEventGroupHandle = xEventGroupCreateStatic(pxCreatedEventGroup)) == NULL)
    {
        return eMAIL_BOX_EXEC_CODE_EVENT;
    }

    xEventGroupClearBits(*pxEventGroupHandle,    /* The event group being updated. */
                         BIT_0);         /* The bits being cleared. */

    return eMAIL_BOX_EXEC_CODE_PASS;
}
#endif /* 0 */



#endif

