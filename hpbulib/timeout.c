/*
 * timeout.c
 *
 *  Created on: Mar 7, 2018
 *      Author: bennytseng
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

#include "type_def.h"
#include "hicc_config.h"
#include "timeout.h"
//=============================================================================
u32_t TMO_GetSysRunTimeSec(void)
{
    struct timespec ts;

    if(clock_gettime(CLOCK_MONOTONIC, &ts) != 0)
        return 0;

    return ts.tv_sec;
}
//=============================================================================
u32_t TMO_GetSysRunTime(void)
{
    struct timespec ts;
    u32_t NowTime;

    if(clock_gettime(CLOCK_MONOTONIC, &ts) != 0)
        return 0;

    NowTime = ts.tv_sec*1000 + ts.tv_nsec/1000000;     // msec unit
    return NowTime;
}
//=============================================================================
// unit: msec
u32_t TMO_GetPeriod(u32_t timer)
{
    return TMO_GetSysRunTime() - timer;
}
//=============================================================================
