#include <time.h>
#include "dvProAV_Platform.h"

volatile static uint32 m_ulStartTimeMsec[eTimerMax];
volatile static bool m_bUsedFlags[eTimerMax];
volatile static bool m_bTimerInit = false;

static void RequireTimerInitCheck(void)
{
    int id;

    if (m_bTimerInit == true)
        return;

    for (id = 0 ; id < eTimerDefaultMax ; id++)
    {
        m_bUsedFlags[id] = true;
    }

    for (id = eTimerDefaultMax ; id < eTimerMax ; id++)
    {
        m_bUsedFlags[id] = false;
    }
    m_bTimerInit = true;
}

void DelayMSec(uint16 milli_seconds)
{
#ifdef QT_CPP
    // Stroing start time
    uint32 ulStopTimeMsec = xGetMSCount() + milli_seconds ;

    // looping till required time is not acheived
    while (xGetMSCount() < ulStopTimeMsec)
    {
        AppProcessEvent();
    }
#else
    MS_SLEEP(milli_seconds);
#endif /* QT_CPP */
}

void ResetTime(uint08 id)
{
    RequireTimerInitCheck();

    if (m_bUsedFlags[id] == false)
        return;
    m_ulStartTimeMsec[id] = (uint32)xGetMSCount();
}

uint32 TimeElapsed(uint08 id)
{
    uint32 ulTimeMsec;

    RequireTimerInitCheck();

    if (m_bUsedFlags[id] == false)
        return 0;

    ulTimeMsec = (uint32)xGetMSCount();

    if(m_ulStartTimeMsec[id] > ulTimeMsec)
    {
        return (0xFFFFFFFF - m_ulStartTimeMsec[id] + 1 + ulTimeMsec);
    }
    else
    {
        return (ulTimeMsec - m_ulStartTimeMsec[id]);
    }
}

bool TimerRequire(uint08 *newTimerId)
{
    int timerId;

    RequireTimerInitCheck();

    for (timerId = 0 ; timerId < (eTimerMax - eTimerDefaultMax) ; timerId++)
    {
       if (m_bUsedFlags[timerId] == false)
       {
           *newTimerId = (uint08)(timerId + eTimerDefaultMax);
           m_bUsedFlags[*newTimerId] = true;
           return true;
       }
    }
    return false;
}

void TimerRelease(uint08 usedTimerId)
{
    RequireTimerInitCheck();

    if (usedTimerId >= eTimerDefaultMax && usedTimerId < eTimerMax)
    m_bUsedFlags[usedTimerId] = false;
}

#ifdef GEOMETRY
// Dennis Yeh 2019/11/04
long long current_timestamp()
{
    struct timeval te;
    gettimeofday(&te, NULL); // get current time
    long long milliseconds = te.tv_sec*1000LL + te.tv_usec/1000;
    printf("###### milliseconds: %lld\n", milliseconds);
    return milliseconds;
}
#endif

//-------------------------------------------------------------------------------------------------
// common func
//-------------------------------------------------------------------------------------------------
char hextoascii(char hex_byte)
{
    char result;
    if((hex_byte >= 0) && (hex_byte <= 9))          // Ascii數字
        result = hex_byte + 0x30;
    else if((hex_byte >= 10) && (hex_byte <= 15))   // Ascii大寫字母
        result = hex_byte + 0x37;
    else
        result = 0xff;
    return result;
}
