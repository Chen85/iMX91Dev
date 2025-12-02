#include "appPoll.h"
#include "appMailBox.h"
#include "appEnvironment.h"
#include "appIllumination.h"
#include "appInputProc.h"
#include "appLedProcAPI.h"
//#include "appGui.h"
#include "appSystem.h" //A70LV_Larry_0112
#include "appDataMgr.h"
#include "appLANProcAPI.h"
#include "utilDbgMsg.h"
#include "utilCounterAPI.h"
#include "utilDataMgrAPI.h"
#include "opdCtrlAPI.h"
#include "utilOPD_TEST.h"
#include "palImgMgr.h" //H30K_Doulas_0067

#include <sys/time.h>

INT16 DataMgr_Poll(UINT16 uiTick)
{
    utilDataMgr_UpdateProcuss_CM();	//HICC2_Doulas_0003
    //printf("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);

    return 2000/POLL_PERIOD;
}

INT16 OPD_Poll(UINT16 uiTick)
{
    //printf("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
    utilOPD_IST_TestResult_Record();
    utilOPD_Write2File();
    utilOPD_Write2TestFile();


    return 5000/POLL_PERIOD; //5s
}

static const PP_CALLBACK m_pPollFunc[] =
{
    palIllumination_Poll,
    palEnvironment_Poll,
    palInputProc_Poll,
    //palLedProc_Poll,
    //appGui_Poll,                       //H2 wait review
    //utilStartupPoll, //A70LV_Larry_0009
    //palEnvironment_ADC_Poll,    //G100_Steven_0015 //A70LV_Doulas_0124
    palLANProc_Poll, //A70LV_Larry_0172
    DataMgr_Poll,
    OPD_Poll,
    //utilUIProcess_Poll,
    //palEnvironment_Motor_Poll,  //G100_Owen_0041
    //DatabaseSync_Poll,
    palImgMgr_Dump_Register_Poll, //H30K_Doulas_0067
    pal_ScalerBlankingLD_Poll,   //HICC2_Doulas_0023
    utilCounter_Event_Process,
};

#define POLLFUN_CCOUNT			sizeof(m_pPollFunc)/sizeof(PP_CALLBACK)


static const UINT8 m_pPollFuncName[POLLFUN_CCOUNT][128] =  //A35G2_Simon_0075
{
    "palIllumination_Poll",
    "palEnvironment_Poll",
    "palInputProc_Poll",
    //"palLedProc_Poll",
    //"appGui_Poll",
    //"utilStartupPoll",
    //"palEnvironment_ADC_Poll",
    "palLANProc_Poll",
    "DataMgr_Poll",
    "OPD_Poll",
    //"palEnvironment_Motor_Poll",
    //"DatabaseSync_Poll",
    "Dump_Register_Poll", //H30K_Doulas_0067
    "ScalerBlankingLD_Poll", //HICC2_Doulas_0023
    "Counter_Event_Process",
};


static sPAL_POLL_INFORMATION       m_sPalPollInfo;

//static const UINT8 m_ucPollFuncCount = sizeof(m_pPollFunc)/sizeof(PP_CALLBACK);

static UINT16 m_uiPollTime[POLLFUN_CCOUNT];        /* polling time array */
static INT16 m_iInterval[POLLFUN_CCOUNT];          /* polling interval array */

struct timeval m_sStart;
struct timeval m_sEnd;

#if (POLLING_TIME_TEST == TRUE)  //A35G2_Simon_0075
INT32 m_lDoingStart[POLLFUN_CCOUNT];
INT32 m_lDoingEnd[POLLFUN_CCOUNT];
#endif

#if (SYSTEM_OS_TYPE == FREERTOS)
void appPoll_TimerHandler(TimerHandle_t xTimer)
{
    xEventGroupSetBits(m_sPalPollInfo.xEventGroupHandle,     /* The event group being updated. */
                       BIT_0);           /* The bits being set. */
}
#endif

void appPoll_Task_Suspend(void)
{
#if (SYSTEM_OS_TYPE == POSIX_COMPLIANT)
	if(m_sPalPollInfo.bTaskPause == FALSE)
	{
        struct timespec s_timeout;
        clock_gettime(CLOCK_REALTIME, &s_timeout);
        util_TimespecAddms(&s_timeout, APP_TASK_SEMPHORE_WAIT);

        if(pthread_mutex_timedlock(&m_sPalPollInfo.xTaskMutex, &s_timeout) == 0)
        {
    		m_sPalPollInfo.bTaskPause = TRUE;
    		pthread_mutex_unlock(&m_sPalPollInfo.xTaskMutex);
        }
        else
        {
            printf("(func:%s, line:%d) fail\r\n", __FUNCTION__, __LINE__);
        }
    }
	else
	{
        printf("(func:%s, line:%d) transport suspend already\r\n", __FUNCTION__, __LINE__);
	}
#else

#endif
}

void appPoll_Task_Resume(void)
{
#if (SYSTEM_OS_TYPE == POSIX_COMPLIANT)
	if(m_sPalPollInfo.bTaskPause == TRUE)
	{
        struct timespec s_timeout;
        clock_gettime(CLOCK_REALTIME, &s_timeout);
        util_TimespecAddms(&s_timeout, APP_TASK_SEMPHORE_WAIT);

        if(pthread_mutex_timedlock(&m_sPalPollInfo.xTaskMutex, &s_timeout) == 0)
        {
    		m_sPalPollInfo.bTaskPause = FALSE;
    		pthread_cond_broadcast(&m_sPalPollInfo.xTaskCond);

    		pthread_mutex_unlock(&m_sPalPollInfo.xTaskMutex);
        }
        else
        {
            printf("(func:%s, line:%d) fail\r\n", __FUNCTION__, __LINE__);
        }
    }
	else
	{
        printf("(func:%s, line:%d) transport resume already\r\n", __FUNCTION__, __LINE__);
	}
#else

#endif
}

static void* appPoll_Task(void *pParameters)
{
    UINT8   ucIdx = 0;
    UINT32  uldiff = 0;

    EventBits_t  uxBits;

    LOG_MSG(db_APP_POLL, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);

    MS_SLEEP(POLL_STARTUP_DELAY);

    while(1)
    {
        if(!m_sPalPollInfo.bTaskPause)
        {
#if (SYSTEM_OS_TYPE == FREERTOS)
            uxBits = xEventGroupWaitBits(
                         m_sPalPollInfo.xEventGroupHandle,   /* The event group being tested. */
                         BIT_0,          /* The bits within the event group to wait for. */
                         pdTRUE,         /* BIT_0 should be cleared before returning. */
                         pdFALSE,        /* Don't wait for both bits, either bit will do. */
                         portMAX_DELAY / portTICK_RATE_MS); /* Wait a maximum of 100ms for either bit to be set. */


            LOG_MSG(db_APP_POLL, "(func:%s, line:%d) uxBits %d\r\n", uxBits);

            if(uxBits != BIT_0)
            {
                continue;
            }

            m_sPalPollInfo.uiTickCounter++;

            for(ucIdx = 0; ucIdx < POLLFUN_CCOUNT; ucIdx++)
            {
                if((INT16)(m_sPalPollInfo.uiTickCounter - m_uiPollTime[ucIdx]) >= m_iInterval[ucIdx])
                {
                    m_uiPollTime[ucIdx] += (m_iInterval[ucIdx] = (*m_pPollFunc[ucIdx])(m_sPalPollInfo.uiTickCounter));
                }
            }
#else

#if (SYSTEM_OS_TYPE == POSIX_COMPLIANT)
            pthread_mutex_lock(&m_sPalPollInfo.xTaskMutex);
#endif
            gettimeofday(&m_sEnd, NULL);

            uldiff = 1000 * (m_sEnd.tv_sec-m_sStart.tv_sec) + (m_sEnd.tv_usec-m_sStart.tv_usec)/1000;

            if(uldiff >= POLL_PERIOD) // > 10ms
            {
                m_sPalPollInfo.uiTickCounter++;

                for(ucIdx = 0; ucIdx < POLLFUN_CCOUNT; ucIdx++)
                {
                    if((INT16)(m_sPalPollInfo.uiTickCounter - m_uiPollTime[ucIdx]) >= m_iInterval[ucIdx])
                    {
                        #if (POLLING_TIME_TEST == FALSE)  //A35G2_Simon_0075
                        {
                            palSystem_TaskMonitorTimerReset(eTID_POLLING, (char *)m_pPollFuncName[ucIdx]);
                            m_uiPollTime[ucIdx] += (m_iInterval[ucIdx] = (*m_pPollFunc[ucIdx])(m_sPalPollInfo.uiTickCounter));
                        }
                        #else
                        {
                            palSystem_TaskMonitorTimerReset(eTID_POLLING, (char *)m_pPollFuncName[ucIdx]);

                            m_lDoingStart[ucIdx] = TMO_GetSysRunTime();
                            m_uiPollTime[ucIdx] += (m_iInterval[ucIdx] = (*m_pPollFunc[ucIdx])(m_sPalPollInfo.uiTickCounter));
                            m_lDoingEnd[ucIdx] = TMO_GetSysRunTime();

                            if(m_lDoingEnd[ucIdx] - m_lDoingStart[ucIdx] > 20)
                            {
                                LOG_MSG(db_APP_POLL, "<%d %d>\n", ucIdx,  m_lDoingEnd[ucIdx] - m_lDoingStart[ucIdx]);
                            }
                        }
                        #endif
                    }
                }

                //LOG_MSG(db_ALWAYS, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);

                gettimeofday(&m_sStart, NULL);
            }

#if (SYSTEM_OS_TYPE == POSIX_COMPLIANT)
            pthread_mutex_unlock(&m_sPalPollInfo.xTaskMutex);
#endif
            US_SLEEP(100);

#endif /* 0 */
        }
        else
        {
#if (SYSTEM_OS_TYPE == POSIX_COMPLIANT)
            pthread_cond_wait(&m_sPalPollInfo.xTaskCond, &m_sPalPollInfo.xTaskMutex);
            pthread_mutex_unlock(&m_sPalPollInfo.xTaskMutex);
#endif
        }
    }
}


eEXEC_CODE appPoll_Init(void)
{
    UINT8 ucIdx = 0;
    BaseType_t xReturned;

    LOG_MSG(db_APP_POLL, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);

    /****************************************************/
    /* Initialize polling times. Stagger the initial    */
    /* polling times to spread out the processing done  */
    /* in any one polling interval.                     */
    /****************************************************/
    for(ucIdx = 0; ucIdx < POLLFUN_CCOUNT; ucIdx++)
    {
        m_uiPollTime[ucIdx] = ucIdx;               /* initial polling time */
        m_iInterval[ucIdx] = 0;                    /* initial polling interval */
    }

    /****************************************************/
    /* Allocate resources and start the polling task.   */
    /****************************************************/
    LOG_MSG(db_APP_POLL, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);

#if (SYSTEM_OS_TYPE == FREERTOS)
    if((m_sPalPollInfo.xEventGroupHandle = xEventGroupCreate()) == NULL)
    {
        LOG_MSG(db_APP_POLL, "(func:%s, line:%d) : Create Poll Task Event Fail!\r\n", __FUNCTION__, __LINE__);
        return eEXEC_CODE_FAIL;
    }

    xEventGroupClearBits(m_sPalPollInfo.xEventGroupHandle,    /* The event group being updated. */
                         BIT_0);
#endif

    LOG_MSG(db_APP_POLL, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);

#if (SYSTEM_OS_TYPE == POSIX_COMPLIANT)

    //Create Thread Pasue
    if(pthread_mutex_init(&m_sPalPollInfo.xTaskMutex, NULL) != 0)
    {
        LOG_MSG(db_ALWAYS, "(func:%s, line:%d): Create xTaskMutex Fail!\r\n", __FUNCTION__, __LINE__);
    }
    m_sPalPollInfo.bTaskPause = FALSE;

    //Create Thread
    pthread_attr_init(&m_sPalPollInfo.xTaskHandle_attr);
    pthread_attr_setstacksize(&m_sPalPollInfo.xTaskHandle_attr, APP_POLL_STACK_SIZE);
    pthread_attr_setdetachstate( &m_sPalPollInfo.xTaskHandle_attr, PTHREAD_CREATE_DETACHED );
    pthread_cond_init(&m_sPalPollInfo.xTaskCond, NULL); //A35G2_BRC_Casper_0051
    INT16 iCreateThreadError = 0 ;
    iCreateThreadError = pthread_create(&m_sPalPollInfo.xTaskHandle, &m_sPalPollInfo.xTaskHandle_attr, appPoll_Task, (void*) NULL);

    if(iCreateThreadError != 0)
    {
        LOG_MSG(db_APP_POLL, "(func:%s, line:%d): Create Thread Fail!\r\n", __FUNCTION__, __LINE__);
        return eEXEC_CODE_FAIL;
    }

    LOG_MSG(db_APP_POLL, "(func:%s, line:%d): Create Thread Pass! (ID:%d)\r\n", __FUNCTION__, __LINE__ , m_sPalPollInfo.xTaskHandle);

#else

    xReturned = xTaskCreate(appPoll_Task,
                            "POLTSK",
                            APP_POLL_STACK_SIZE,
                            NULL,
                            APP_POLL_PRIORITY,
                            &m_sPalPollInfo.xTaskHandle);


    if(xReturned != pdPASS)
    {
        LOG_MSG(db_APP_POLL, "(func:%s, line:%d): Create Poll Task Fail!\r\n", __FUNCTION__, __LINE__);

        if(m_sPalPollInfo.xEventGroupHandle != NULL)
        {
            vEventGroupDelete(m_sPalPollInfo.xEventGroupHandle);
        }

        //if(m_sPalPollInfo.xTimerHandle != NULL)
        //{
           // xTimerDelete(m_sPalPollInfo.xTimerHandle, 0);
        //}

        return eEXEC_CODE_FAIL;
    }

#endif

    LOG_MSG(db_APP_POLL, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);

    gettimeofday(&m_sStart, NULL);
    gettimeofday(&m_sEnd, NULL);

    palLedProc_Init();

    return eEXEC_CODE_PASS;
}

// ==============================================================================
// FUNCTION NAME: appPoll_TaskPriorityHighSet
// DESCRIPTION:
//
//
// Params:
// BOOL bEnable:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/10/31, Larry Create
// --------------------
// ==============================================================================
void appPoll_TaskPriorityHighSet(BOOL bEnable)
{
    if(bEnable)
    {
        vTaskPrioritySet(m_sPalPollInfo.xTaskHandle, (APP_POLL_PRIORITY+2));
    }
    else
    {
        vTaskPrioritySet(m_sPalPollInfo.xTaskHandle, (APP_POLL_PRIORITY));
    }
}



