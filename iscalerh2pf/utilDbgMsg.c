#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdarg.h>
#include "Common.h"
#include "utilDbgMsg.h"
#if (SYSTEM_OS_TYPE == FREERTOS)
#include "./FreeRTOS_Kernel/include/semphr.h"
#endif
#include "Board_Uart.h"

static sDBGMSG_INFORMATION m_sDbgMsgInfo;

uint8 utilDbgMsgTake(void)
{
    int err;

    pthread_mutex_t *Mutex = &m_sDbgMsgInfo.psDebugMsgMutex->DebugMdg_Mutex;
    struct timespec s_timeout;
    clock_gettime(CLOCK_REALTIME, &s_timeout);
    util_TimespecAddms(&s_timeout, SEMPHORE_WAIT_DELAY);
    err = pthread_mutex_timedlock(Mutex , &s_timeout);

    if(err != 0)
    {
        LOG_MSG(db_ALWAYS, "012 %s pthread_mutex_timedlock fail [%s]\n", __FUNCTION__, strerror(err));
        return 0;
    }

    return 1;  //pass
}

uint8 utilDbgMsgGive(void)
{
    int err;

    pthread_mutex_t *Mutex = &m_sDbgMsgInfo.psDebugMsgMutex->DebugMdg_Mutex;
    err = pthread_mutex_unlock(Mutex);

    if(err != 0)
    {
        LOG_MSG(db_ALWAYS, "%s pthread_mutex_unlock fail [%s]\n", __FUNCTION__, strerror(err));
        return 0;
    }

    return 1;  //pass
}

// ==============================================================================
// FUNCTION NAME: utilDbgMsg_ftrace
// DESCRIPTION:
//
//
// PARAMETERS:
// UINT64 ullBitmask:
// const char *pfmtstring:
// ...:
//
// RETURNS:
//
// ==============================================================================
void utilDbgMsg_ftrace(UINT64 ullBitmask, const char *pfmtstring, ...)
{
    va_list args;
    struct timespec s_timeout;

    if((m_sDbgMsgInfo.ullMask & ullBitmask) || (ullBitmask == db_ALWAYS)) //HICC2_Doulas_0083 Modify
    {
        clock_gettime(CLOCK_REALTIME, &s_timeout);
        util_TimespecAddms(&s_timeout, SEMPHORE_WAIT_DELAY);

        if(utilDbgMsgTake())
        {
#ifndef APP_BACKGROUND
            struct timeval now;
            gettimeofday( &now, NULL );

            va_start(args, pfmtstring);
            vsnprintf(m_sDbgMsgInfo.aucMsg, BUFFER_SIZE, pfmtstring, args);
            printf("(%02d.%02d.%03d)", now.tv_sec/60%100, now.tv_sec%60, (now.tv_usec/1000)%1000);
            printf((char*)m_sDbgMsgInfo.aucMsg);      //A70LV_Doulas_2000 modify
            va_end(args);
#else
            va_start(args, pfmtstring);

            #if (LOG_WITH_TIME == TRUE)
            {
                struct timeval now;
                gettimeofday( &now, NULL );

                snprintf(m_sDbgMsgInfo.aucMsg, BUFFER_SIZE, "(%02d.%02d.%03d)", now.tv_sec/60%100, now.tv_sec%60, (now.tv_usec/1000)%1000);
                Board_Uart_Write(uaPCUART, strlen(m_sDbgMsgInfo.aucMsg), m_sDbgMsgInfo.aucMsg);
            }
            #endif

            vsnprintf(m_sDbgMsgInfo.aucMsg, BUFFER_SIZE, pfmtstring, args);
            Board_Uart_Write(uaPCUART, strlen(m_sDbgMsgInfo.aucMsg), m_sDbgMsgInfo.aucMsg);

            va_end(args);
#endif /* APP_BACKGROUND */

            utilDbgMsgGive();
        }

    }
}

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
void utilSystem_Call(const char *pfmtstring, ...)
{
    if(utilDbgMsgTake())
    {
        va_list args;
        int ret;

        va_start(args, pfmtstring);
        vsnprintf(m_sDbgMsgInfo.aucMsg, BUFFER_SIZE, pfmtstring, args);

        ret = system(m_sDbgMsgInfo.aucMsg);
        if(ret != 0)
        {
            printf("(func:%s, line:%d) system call fail, ret=%d\n", __FUNCTION__, __LINE__, ret);
        }

        va_end(args);

        utilDbgMsgGive();
    }
}

// ==============================================================================
// FUNCTION NAME: utilDbgMsg_Init
// DESCRIPTION:
//
//
// PARAMETERS:
// UINT64 ullMask:
//
// RETURNS:
//
// ==============================================================================
eEXEC_CODE utilDbgMsg_Init(UINT64 ullMask)
{
    m_sDbgMsgInfo.psDebugMsgMutex = (sPROCESS_MUTEX_DATA *)SharedMem_GetMapPtr(eSB_PROCESS_MUTEX_DATA);

    pthread_mutexattr_init(&m_sDbgMsgInfo.xDebugMs_MutexAttr);
    pthread_mutexattr_setpshared(&m_sDbgMsgInfo.xDebugMs_MutexAttr, PTHREAD_PROCESS_SHARED);  //Mutex for Process
    pthread_mutex_init(&m_sDbgMsgInfo.xDebugMs_Mutex, &m_sDbgMsgInfo.xDebugMs_MutexAttr);

    m_sDbgMsgInfo.psDebugMsgMutex->DebugMdg_Mutex = m_sDbgMsgInfo.xDebugMs_Mutex;

    m_sDbgMsgInfo.ullMask = ullMask;

    return eEXEC_CODE_PASS;
}

// ==============================================================================
// FUNCTION NAME: utilDbgMsg_Set
// DESCRIPTION:
//
//
// Params:
// UINT64 ulMask:
// BYTE cEnable:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/10/26, Larry Create
// --------------------
// ==============================================================================
void utilDbgMsg_Set(UINT64 ullMask, UINT8 cEnable)
{
    if(ullMask == db_ALL)
    {
        if(cEnable)
        {
            m_sDbgMsgInfo.ullMask = db_ALL;
        }
        else
        {
            m_sDbgMsgInfo.ullMask = 0;
        }
    }
    else
    {
        if(cEnable)
        {
            m_sDbgMsgInfo.ullMask |= ullMask;
        }
        else
        {
            m_sDbgMsgInfo.ullMask &= ~ullMask;
        }
    }
}


// ==============================================================================
// FUNCTION NAME: utilDbgMsg_Get
// DESCRIPTION:
//
//
// Params:
// UINT64 ulMask:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/10/26, Larry Create
// --------------------
// ==============================================================================
UINT64 utilDbgMsg_Get(UINT64 ullMask)
{
    if(ullMask == db_ALL)
    {
        return m_sDbgMsgInfo.ullMask;
    }
    else
    {
        return !!(m_sDbgMsgInfo.ullMask & ullMask);
    }
}

// ==============================================================================
// FUNCTION NAME: utilDbgMsg_ReSet
// DESCRIPTION:
//
//
// Params:
// UINT64 ullMask:
// BYTE cEnable:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/11/09, Larry Create
// --------------------
// ==============================================================================
void utilDbgMsg_ReSet(UINT64 ullMask, UINT64 ullMaskCT)
{
    m_sDbgMsgInfo.ullMask = ullMask;
    m_sDbgMsgInfo.psDebugMsgMutex->DebugMdg_Mask = ullMaskCT;
}

// ==============================================================================
// FUNCTION NAME: utilDbgMsgGui_Set
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
// 2024/04/16, Larry Create
// --------------------
// ==============================================================================
void utilDbgMsgCT_Set(UINT64 ullMask, UINT8 cEnable)
{
    if(ullMask == db_ALL)
    {
        if(cEnable)
        {
            m_sDbgMsgInfo.psDebugMsgMutex->DebugMdg_Mask = db_ALL;
        }
        else
        {
            m_sDbgMsgInfo.psDebugMsgMutex->DebugMdg_Mask = 0;
        }
    }
    else
    {
        if(cEnable)
        {
            m_sDbgMsgInfo.psDebugMsgMutex->DebugMdg_Mask |= ullMask;
        }
        else
        {
            m_sDbgMsgInfo.psDebugMsgMutex->DebugMdg_Mask &= ~ullMask;
        }
    }
}

// ==============================================================================
// FUNCTION NAME: utilDbgMsgGui_Get
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
// 2024/04/16, Larry Create
// --------------------
// ==============================================================================
UINT64 utilDbgMsgCT_Get(UINT64 ullMask)
{
    if(ullMask == db_ALL)
    {
        return m_sDbgMsgInfo.psDebugMsgMutex->DebugMdg_Mask;
    }
    else
    {
        return !!(m_sDbgMsgInfo.psDebugMsgMutex->DebugMdg_Mask & ullMask);
    }
}



