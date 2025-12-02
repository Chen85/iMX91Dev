// ===============================================================================
// FILE NAME: Posix.c
// DESCRIPTION:
//
//
// Modification History
// --------------------
// 2020/02/22, Lai Create
// --------------------
// ===============================================================================
#include "Posix.h"

#include <time.h>

#include "utilDbgMsg.h"


UINT64 Posix_Tick_Get(void)
{
    //POSIX : 1ms 有 1000 個 tick
    struct timespec sTime;
    clock_gettime(CLOCK_MONOTONIC, &sTime);
    return (UINT64)(1000000*(sTime.tv_sec)+(sTime.tv_nsec/1000));
}

//Add Milliseconds for timespec struct value
void util_TimespecAddms(struct timespec *s_time ,INT32 lms)
{
    //tv_nsec can not less than zero or greater than 1000 million
    if( (s_time->tv_nsec + ((lms % 1000)*1000000)) > 1000000000)
    {
        s_time->tv_sec  =  s_time->tv_sec  + 1 + (lms / 1000);
        s_time->tv_nsec = (s_time->tv_nsec + ((lms % 1000)*1000000)) % 1000000000 ;
    }
    else
    {
        s_time->tv_sec  = s_time->tv_sec  + (lms / 1000);
        s_time->tv_nsec = s_time->tv_nsec + ((lms % 1000)*1000000) ;
    }

    //check
    if(s_time->tv_sec < 0  ||
       s_time->tv_nsec < 0 ||
       lms < 0)
    {
        ASSERT_ALWAYS();
    }
}




