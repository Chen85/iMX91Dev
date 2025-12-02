#include "palCoreVar.h"
#include "utilDbgMsg.h"
#include "halLDProc.h"      //A70LV_Doulas_0061

static sPAL_CORE_VAR_INFORMATION    m_sPalCoreVarInfo;                 //Use for system operation




BOOL palCoreVar_GetIsCoolingVar(void)
{
    BOOL bRet = FALSE;

#if (SYSTEM_OS_TYPE == POSIX_COMPLIANT)
    struct timespec s_timeout;
    clock_gettime(CLOCK_REALTIME, &s_timeout);
    util_TimespecAddms(&s_timeout, SEMPHORE_WAIT_DELAY);

    if(pthread_mutex_timedlock(&m_sPalCoreVarInfo.xMutex, &s_timeout) != 0)
    {
        LOG_MSG(db_APP_CORE, "001 %s pthread_mutex_timedlock fail [%s]\n", __FUNCTION__ , strerror(errno));
    }
    else
    {
        bRet = m_sPalCoreVarInfo.sCoreVar.isCooling;
        if(pthread_mutex_unlock(&m_sPalCoreVarInfo.xMutex) != 0)
        {
            LOG_MSG(db_APP_CORE, "%s pthread_mutex_unlock fail [%s]\n", __FUNCTION__ , strerror(errno));
        }
    }
#else
    if(xSemaphoreTake(m_sPalCoreVarInfo.xSemaphore, (TickType_t)SEMPHORE_WAIT_DELAY / portTICK_RATE_MS) == pdTRUE)
    {
        bRet = m_sPalCoreVarInfo.sCoreVar.isCooling;
        xSemaphoreGive(m_sPalCoreVarInfo.xSemaphore);
    }
    else
    {
        LOG_MSG(db_APP_CORE, "(func:%s, line:%d) Can not get semphore\r\n", __FUNCTION__, __LINE__);
    }
#endif

    return bRet;
}

eEXEC_CODE palCoreVar_SetIsCoolingVar(BOOL bValue)
{
#if (SYSTEM_OS_TYPE == POSIX_COMPLIANT)
    struct timespec s_timeout;
    clock_gettime(CLOCK_REALTIME, &s_timeout);
    util_TimespecAddms(&s_timeout, SEMPHORE_WAIT_DELAY);
    if(pthread_mutex_timedlock(&m_sPalCoreVarInfo.xMutex, &s_timeout) != 0)
    {
        LOG_MSG(db_APP_CORE, "002 %s pthread_mutex_timedlock fail [%s]\n", __FUNCTION__ , strerror(errno));
        return eEXEC_CODE_FAIL;
    }
    else
    {
        m_sPalCoreVarInfo.sCoreVar.isCooling = bValue;
        if(pthread_mutex_unlock(&m_sPalCoreVarInfo.xMutex) != 0)
        {
            LOG_MSG(db_APP_CORE, "%s pthread_mutex_unlock fail [%s]\n", __FUNCTION__ , strerror(errno));
        }
    }
#else

    if(xSemaphoreTake(m_sPalCoreVarInfo.xSemaphore, (TickType_t)SEMPHORE_WAIT_DELAY / portTICK_RATE_MS) == pdTRUE)
    {
        m_sPalCoreVarInfo.sCoreVar.isCooling = bValue;
        xSemaphoreGive(m_sPalCoreVarInfo.xSemaphore);
    }
    else
    {
        LOG_MSG(db_APP_CORE, "(func:%s, line:%d) Can not get semphore\r\n", __FUNCTION__, __LINE__);
        return eEXEC_CODE_FAIL;
    }
#endif

    return eEXEC_CODE_PASS;
}



//TODO : This is a example for using appCoreSystem
BOOL palCoreVar_GetSystemPowerOnVar(void)
{
    BOOL bRet = FALSE;

#if (SYSTEM_OS_TYPE == POSIX_COMPLIANT)
    struct timespec s_timeout;
    clock_gettime(CLOCK_REALTIME, &s_timeout);
    util_TimespecAddms(&s_timeout, SEMPHORE_WAIT_DELAY);
    if(pthread_mutex_timedlock(&m_sPalCoreVarInfo.xMutex, &s_timeout) != 0)
    {
        LOG_MSG(db_APP_CORE, "003 %s pthread_mutex_timedlock fail [%s]\n", __FUNCTION__ , strerror(errno));
    }
    else
    {
        bRet = m_sPalCoreVarInfo.sCoreVar.isSystemOn;
        if(pthread_mutex_unlock(&m_sPalCoreVarInfo.xMutex) != 0)
        {
            LOG_MSG(db_APP_CORE, "%s pthread_mutex_unlock fail [%s]\n", __FUNCTION__ , strerror(errno));
        }
    }
#else

    if(xSemaphoreTake(m_sPalCoreVarInfo.xSemaphore, (TickType_t)SEMPHORE_WAIT_DELAY / portTICK_RATE_MS) == pdTRUE)
    {
        bRet = m_sPalCoreVarInfo.sCoreVar.isSystemOn;
        xSemaphoreGive(m_sPalCoreVarInfo.xSemaphore);
    }
    else
    {
        LOG_MSG(db_APP_CORE, "(func:%s, line:%d) Can not get semphore\r\n", __FUNCTION__, __LINE__);
    }

#endif

    return bRet;
}

eEXEC_CODE palCoreVar_SetSystemPowerOnVar(BOOL bValue)
{
#if (SYSTEM_OS_TYPE == POSIX_COMPLIANT)
    struct timespec s_timeout;
    clock_gettime(CLOCK_REALTIME, &s_timeout);
    util_TimespecAddms(&s_timeout, SEMPHORE_WAIT_DELAY);
    if(pthread_mutex_timedlock(&m_sPalCoreVarInfo.xMutex, &s_timeout) != 0)
    {
        LOG_MSG(db_APP_CORE, "004 %s pthread_mutex_timedlock fail [%s]\n", __FUNCTION__ , strerror(errno));
        return eEXEC_CODE_FAIL;
    }
    else
    {
        m_sPalCoreVarInfo.sCoreVar.isSystemOn = bValue;
        if(pthread_mutex_unlock(&m_sPalCoreVarInfo.xMutex) != 0)
        {
            LOG_MSG(db_APP_CORE, "%s pthread_mutex_unlock fail [%s]\n", __FUNCTION__ , strerror(errno));
        }
    }
#else
    if(xSemaphoreTake(m_sPalCoreVarInfo.xSemaphore, (TickType_t)SEMPHORE_WAIT_DELAY / portTICK_RATE_MS) == pdTRUE)
    {
        m_sPalCoreVarInfo.sCoreVar.isSystemOn = bValue;
        xSemaphoreGive(m_sPalCoreVarInfo.xSemaphore);
    }
    else
    {
        LOG_MSG(db_APP_CORE, "(func:%s, line:%d) Can not get semphore\r\n", __FUNCTION__, __LINE__);
        return eEXEC_CODE_FAIL;
    }
#endif

    return eEXEC_CODE_PASS;
}

BOOL palCoreVar_GetLightSourceOnVar(void)
{
    BOOL bRet = FALSE;

    //if(xSemaphoreTake(m_sAppCoreVarInfo.xSemaphore, (TickType_t)SEMPHORE_WAIT_DELAY / portTICK_RATE_MS) == pdTRUE) //A70LV_Larry_0081 mask
    //{
        bRet = m_sPalCoreVarInfo.sCoreVar.isLightSource;
        //xSemaphoreGive(m_sAppCoreVarInfo.xSemaphore);
    //}
    //else
    //{
        //LOG_MSG(db_APP_CORE, "(func:%s, line:%d) Can not get semphore\r\n", __FUNCTION__, __LINE__);
    //}

    return bRet;
}
eEXEC_CODE palCoreVar_SetLightSourceOnVar(BOOL bValue)
{
#if (SYSTEM_OS_TYPE == POSIX_COMPLIANT)
    struct timespec s_timeout;
    clock_gettime(CLOCK_REALTIME, &s_timeout);
    util_TimespecAddms(&s_timeout, SEMPHORE_WAIT_DELAY);
    if(pthread_mutex_timedlock(&m_sPalCoreVarInfo.xMutex, &s_timeout) != 0)
    {
        LOG_MSG(db_APP_CORE, "005 %s pthread_mutex_timedlock fail [%s]\n", __FUNCTION__ , strerror(errno));
        return eEXEC_CODE_FAIL;
    }
    else
    {
        m_sPalCoreVarInfo.sCoreVar.isLightSource = bValue;
        if(pthread_mutex_unlock(&m_sPalCoreVarInfo.xMutex) != 0)
        {
            LOG_MSG(db_APP_CORE, "%s pthread_mutex_unlock fail [%s]\n", __FUNCTION__ , strerror(errno));
        }
    }
#else

    if(xSemaphoreTake(m_sPalCoreVarInfo.xSemaphore, (TickType_t)SEMPHORE_WAIT_DELAY / portTICK_RATE_MS) == pdTRUE)
    {
        m_sPalCoreVarInfo.sCoreVar.isLightSource = bValue;
        //halLDCtrl_LD_SourceEnable_Set(eLDBANK_A70LV,(UINT8)bValue);     //A70LV_Doulas_0061 //A70LV_Larry_0081 mask
        xSemaphoreGive(m_sPalCoreVarInfo.xSemaphore);
    }
    else
    {
        LOG_MSG(db_APP_CORE, "(func:%s, line:%d) Can not get semphore\r\n", __FUNCTION__, __LINE__);
        return eEXEC_CODE_FAIL;
    }
#endif

    return eEXEC_CODE_PASS;
}

BOOL palCoreVar_GetBurnInOnModeVar(void)
{
    BOOL bRet = FALSE;

#if (SYSTEM_OS_TYPE == POSIX_COMPLIANT)
    struct timespec s_timeout;
    clock_gettime(CLOCK_REALTIME, &s_timeout);
    util_TimespecAddms(&s_timeout, SEMPHORE_WAIT_DELAY);
    if(pthread_mutex_timedlock(&m_sPalCoreVarInfo.xMutex, &s_timeout) != 0)
    {
        LOG_MSG(db_APP_CORE, "006 %s pthread_mutex_timedlock fail [%s]\n", __FUNCTION__ , strerror(errno));
    }
    else
    {
        bRet = m_sPalCoreVarInfo.sCoreVar.isBurnInOnMode;
        if(pthread_mutex_unlock(&m_sPalCoreVarInfo.xMutex) != 0)
        {
            LOG_MSG(db_APP_CORE, "%s pthread_mutex_unlock fail [%s]\n", __FUNCTION__ , strerror(errno));
        }
    }
#else
    if(xSemaphoreTake(m_sPalCoreVarInfo.xSemaphore, (TickType_t)SEMPHORE_WAIT_DELAY / portTICK_RATE_MS) == pdTRUE)
    {
        bRet = m_sPalCoreVarInfo.sCoreVar.isBurnInOnMode;
        xSemaphoreGive(m_sPalCoreVarInfo.xSemaphore);
    }
    else
    {
        LOG_MSG(db_APP_CORE, "(func:%s, line:%d) Can not get semphore\r\n", __FUNCTION__, __LINE__);
    }
#endif

    return bRet;
}
eEXEC_CODE palCoreVar_SetBurnInOnModeVar(BOOL bValue)
{
#if (SYSTEM_OS_TYPE == POSIX_COMPLIANT)
    struct timespec s_timeout;
    clock_gettime(CLOCK_REALTIME, &s_timeout);
    util_TimespecAddms(&s_timeout, SEMPHORE_WAIT_DELAY);
    if(pthread_mutex_timedlock(&m_sPalCoreVarInfo.xMutex, &s_timeout) != 0)
    {
        LOG_MSG(db_APP_CORE, "007 %s pthread_mutex_timedlock fail [%s]\n", __FUNCTION__ , strerror(errno));
        return eEXEC_CODE_FAIL;
    }
    else
    {
        m_sPalCoreVarInfo.sCoreVar.isBurnInOnMode = bValue;
        if(pthread_mutex_unlock(&m_sPalCoreVarInfo.xMutex) != 0)
        {
            LOG_MSG(db_APP_CORE, "%s pthread_mutex_unlock fail [%s]\n", __FUNCTION__ , strerror(errno));
        }
    }
#else
    if(xSemaphoreTake(m_sPalCoreVarInfo.xSemaphore, (TickType_t)SEMPHORE_WAIT_DELAY / portTICK_RATE_MS) == pdTRUE)
    {
        m_sPalCoreVarInfo.sCoreVar.isBurnInOnMode = bValue;
        xSemaphoreGive(m_sPalCoreVarInfo.xSemaphore);
    }
    else
    {
        LOG_MSG(db_APP_CORE, "(func:%s, line:%d) Can not get semphore\r\n", __FUNCTION__, __LINE__);
        return eEXEC_CODE_FAIL;
    }
#endif

    return eEXEC_CODE_PASS;
}




eEXEC_CODE palCoreVar_Init(void)
{

    LOG_MSG(db_APP_CORE, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);

#if (SYSTEM_OS_TYPE == POSIX_COMPLIANT)

    if(pthread_mutex_init(&m_sPalCoreVarInfo.xMutex, NULL) == -1)
    {
        LOG_MSG(db_APP_CORE, "%s pthread_mutex_init fail [%s]\n", __FUNCTION__ , strerror(errno));
        return eEXEC_CODE_FAIL;
    }

#else

    #if(CURRENT_RTOS_TYPE == RTOS_STATIC)
        m_sPalCoreVarInfo.xSemaphore = xSemaphoreCreateMutexStatic(&m_sPalCoreVarInfo.xMutexBuffer);
    #else
        m_sPalCoreVarInfo.xSemaphore = xSemaphoreCreateMutex();
    #endif

    if(m_sPalCoreVarInfo.xSemaphore == NULL)
    {
        return eEXEC_CODE_FAIL;
    }

#endif

    memset(&m_sPalCoreVarInfo.sCoreVar, 0, sizeof(sCORE_VAR));

    return eEXEC_CODE_PASS;

}


