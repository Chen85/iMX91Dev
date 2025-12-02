// ==============================================================================
// FILE NAME: UTILCOUNTER.C
// DESCRIPTION:
//
//
// modification history
// --------------------
// 19/03/2013, Leohong written
// --------------------
// ==============================================================================


#include "Common.h"
#include "utilCounterAPI.h"
//#include "Board_RITimer.h"
//#include "Board_GPIO.h"
#include "utilDbgMsg.h"

volatile static sTIMER_EVENT m_asTimerEvent[eTIMER_EVENT_TYPE_NUMBERS];
volatile static UINT64 m_aqwTickCountRef[eCOUNTER_TYPE_NUMBERS];
volatile static UINT64 m_aqwTickCountStart[eCOUNTER_TYPE_NUMBERS];

// ==============================================================================
// FUNCTION NAME: UTILTIMEREVENT_CHECK
// DESCRIPTION:
//
//
// Params:
// eTIMER_EVENT_TYPE eSwTimer:
//
// Returns:
//
//
// modification history
// --------------------
// 2014/04/02, Leo Create
// --------------------
// ==============================================================================
static UINT32 utilTimerEvent_Check(eTIMER_EVENT_TYPE eSwTimer)
{
    if(m_asTimerEvent[eSwTimer].dwTimer > 0)
    {
        m_asTimerEvent[eSwTimer].dwTimer--;
    }

    return m_asTimerEvent[eSwTimer].dwTimer;
}

// ==============================================================================
// FUNCTION NAME: utilCounter_Event_Process
// DESCRIPTION:
//
//
// Params:
// void:
//
// Returns:
//
//
// modification history
// --------------------
// 28/10/2013, Leo Create
// --------------------
// ==============================================================================
INT16 utilCounter_Event_Process(UINT16 uiTick)
{
    UINT8 cCount = 0;

    for(cCount = 0; cCount < eTIMER_EVENT_TYPE_NUMBERS; cCount++)
    {
        if(NULL != m_asTimerEvent[cCount].fpEventCallback)
        {
            if(0 == utilTimerEvent_Check((eTIMER_EVENT_TYPE)cCount))
            {
                // 計數時間達到，觸發Event Callback
                m_asTimerEvent[cCount].fpEventCallback();

                // 需重新載入
                if(m_asTimerEvent[cCount].cReload)
                {
                    m_asTimerEvent[cCount].dwTimer = m_asTimerEvent[cCount].dwReloadTimer;
                }
                // 需重複次數?
                else if(m_asTimerEvent[cCount].wRepeatCount)
                {
                    m_asTimerEvent[cCount].wRepeatCount--;
                    m_asTimerEvent[cCount].dwTimer = m_asTimerEvent[cCount].dwReloadTimer;

                    if(!m_asTimerEvent[cCount].wRepeatCount)
                    {
                        // 重複次數結束，觸發結束Finish Event Callback
                        m_asTimerEvent[cCount].fpEventFinishCallback();

                        // 清除Timer設定
                        m_asTimerEvent[cCount].fpEventCallback = NULL;
                        m_asTimerEvent[cCount].fpEventFinishCallback = NULL;
                    }
                }
                else
                {
                    // 清除Timer設定
                    m_asTimerEvent[cCount].fpEventCallback = NULL;
                    m_asTimerEvent[cCount].fpEventFinishCallback = NULL;
                }
            }
        }
    } // End of For

    return 1;
}


// ==============================================================================
// FUNCTION NAME: UTILCOUNTER_INIT
// DESCRIPTION:
//
//
// Params:
// void:
//
// Returns:
//
//
// modification history
// --------------------
// 28/10/2013, Leo Create
// --------------------
// ==============================================================================
void utilCounter_Init(void)
{
    UINT8 cCount = 0;

    for(cCount = 0; cCount < eTIMER_EVENT_TYPE_NUMBERS; cCount++)
    {
        m_asTimerEvent[cCount].dwTimer = 0;
        m_asTimerEvent[cCount].dwReloadTimer = 0;
        m_asTimerEvent[cCount].cReload = 0;
        m_asTimerEvent[cCount].wRepeatCount = 0;
        m_asTimerEvent[cCount].fpEventCallback = NULL;
        m_asTimerEvent[cCount].fpEventFinishCallback = NULL;
    }
}

// ==============================================================================
// FUNCTION NAME: UTILCOUNTERSET
// DESCRIPTION:
//
//
// Params:
// eCOUNTER_TYPE eCounter:
// UINT16 wMilliSecond:
//
// Returns:
//
//
// modification history
// --------------------
// 19/03/2013, Leohong written
// --------------------
// ==============================================================================
void utilCounterSet(eCOUNTER_TYPE eCounter, UINT32 dwMilliSecond)
{
    //ASSERT(eCOUNTER_TYPE_NUMBERS > eCounter);
    if(eCounter >= eCOUNTER_TYPE_NUMBERS)
    {
        ASSERT_ALWAYS();
        return;
    }
    // store timer start value and
    m_aqwTickCountStart[eCounter] = (UINT64)(dwMilliSecond * portTICK_RATE_MS) ;   //要倒數的 tick 數

    // get current value of timer 1 and store it, reset rollover flag
    m_aqwTickCountRef[eCounter] = xTaskGetTickCount();
}

// ==============================================================================
// FUNCTION NAME: UTILCOUNTERGET
// DESCRIPTION:
//
//
// Params:
// eCOUNTER_TYPE eCounter:
//
// Returns:
//
//
// modification history
// --------------------
// 19/03/2013, Leohong written
// --------------------
// ==============================================================================
UINT64 utilCounterGet(eCOUNTER_TYPE eCounter)
{
    //ASSERT(eCOUNTER_TYPE_NUMBERS > eCounter);

    UINT64 qwTime = 0;

    // validate arguments in debug mode only
    //ASSERT(eCounter < eCOUNTER_TYPE_NUMBERS);
    if(eCounter >= eCOUNTER_TYPE_NUMBERS)
    {
        ASSERT_ALWAYS();
        return 0;
    }
    // if the count timeout is zero, just return remaining count of zero
    if(m_aqwTickCountStart[eCounter] > 0)
    {
        // compute the timer ticks that have elapsed so far
#if 0
        qwTime = xTaskGetTickCount() - m_aqwTickCountRef[eCounter];
#else
        qwTime = xTaskGetTickCount() ;

        // 如果Ref Count > qwTime, 表示Timer counter已經有Ring
        if(m_aqwTickCountRef[eCounter] > qwTime)
        {
            qwTime = (0xFFFFFFFFFFFFFFFF - m_aqwTickCountRef[eCounter]) + qwTime + 1;
        }
        else
        {
            qwTime = qwTime - m_aqwTickCountRef[eCounter];
        }
#endif /* 0 */

        // compute the counter ticks remaining until timeout
        if(m_aqwTickCountStart[eCounter] > qwTime)
        {
            // this subtraction requires unsigned, same size start and timer values
            qwTime = m_aqwTickCountStart[eCounter] - qwTime;
        }
        else
        {
            // the counter has finished so disable it and return zero
            m_aqwTickCountStart[eCounter] = 0;
            qwTime = 0;
        }
    }

    return qwTime;
}

// ==============================================================================
// FUNCTION NAME: utilCounter_UnReg_TimerEvent
// DESCRIPTION:
//
//
// Params:
// eTIMER_EVENT_TYPE eTimer:
//
// Returns:
//
//
// modification history
// --------------------
// 2014/04/24, Leo Create
// --------------------
// ==============================================================================
void utilCounter_UnReg_TimerEvent(eTIMER_EVENT_TYPE eTimer)
{
    m_asTimerEvent[eTimer].fpEventCallback = NULL;
}

// ==============================================================================
// FUNCTION NAME: utilCounter_Reg_TimerEvent
// DESCRIPTION:
//
//
// Params:
// eTIMER_EVENT_TYPE eTimer:                Timer ID
// UINT32 dwTimer:                           Timer 時間
// UINT8 cReloard:                           重新載入
// WORD wRepeatCount:                       重複次數
// fpTimerEventCallback fpCallback:         Timer 觸發的Event Callback
// fpTimerEventCallback fpFinishCallback:   Timer 結束後觸發的 Finsih Event Callback
//
// Returns:
//
//
// modification history
// --------------------
// 2014/04/25, Leo Create
// --------------------
// ==============================================================================
void utilCounter_Reg_TimerEvent(eTIMER_EVENT_TYPE eTimer,
                                UINT32 dwTimer,
                                UINT8  cReloard,
                                UINT16 wRepeatCount,
                                fpTimerEventCallback fpCallback,
                                fpTimerEventCallback fpFinishCallback)
{
    m_asTimerEvent[eTimer].dwTimer = (dwTimer/10); //10ms
    m_asTimerEvent[eTimer].dwReloadTimer = dwTimer;
    m_asTimerEvent[eTimer].cReload = cReloard;
    m_asTimerEvent[eTimer].wRepeatCount = wRepeatCount;
    m_asTimerEvent[eTimer].fpEventCallback = fpCallback;
    m_asTimerEvent[eTimer].fpEventFinishCallback = fpFinishCallback;

    if(wRepeatCount)
    {
        m_asTimerEvent[eTimer].cReload = FALSE;
    }
}

// ==============================================================================
// FUNCTION NAME: utilCounter_Tick_Get
// DESCRIPTION:
//
//
// Params:
// void:
//
// Returns:
//
//
// Modification History
// --------------------
// 2014/05/27, Leo Create
// --------------------
// ==============================================================================
UINT32 utilCounter_Tick_Get(void)
{
#if (SYSTEM_OS_TYPE == POSIX_COMPLIANT)

    //POSIX : 1ms 有 1000 個 tick
    struct timespec sTime;
    clock_gettime(CLOCK_MONOTONIC, &sTime);
    return (UINT32)(1000000*(sTime.tv_sec)+(sTime.tv_nsec/1000));

#else

    //freertos : 1ms 有 1個 tick
    return (UINT32)xTaskGetTickCount() ;

#endif


}

