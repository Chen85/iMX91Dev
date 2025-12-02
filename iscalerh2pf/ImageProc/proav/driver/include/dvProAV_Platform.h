#ifndef DV_PROAV_PLATFORM_H
#define DV_PROAV_PLATFORM_H
// ==============================================================================
// FILE NAME: dvProAV_Platform.H
// DESCRIPTION:
//
//
// modification history
// --------------------
//
// --------------------
// ==============================================================================

#include "Common.h"

#define FPGA_ENTRY_4K
#ifdef GEOMETRY
    #include "GenericTypeDefs.h"
#endif
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#define WO 0
#define RO 1
#define RW 2
#define ACCESS_MODE_READ  1
#define ACCESS_MODE_WRITE 0

#define TABLE_END_INDEX 0xFFFF

typedef unsigned char uint8;
typedef unsigned char uint08;
typedef unsigned short  uint16;
typedef unsigned int uint32;

typedef char int8;
typedef char int08;
typedef short int16;
typedef int int32;
typedef double DOUBLE;

#ifndef NULL
#define NULL 0
#endif
typedef uint8 (*pfuncDeviceRead)(uint32 address, uint32 length, uint8 *data);
typedef uint8 (*pfuncDeviceWrite)(uint32 address, uint32 length, const uint8 *data);
typedef uint8 (*pfuncDeviceBufferWrite)(void);

typedef enum{
  eTimerSystem,
  eTimerWaitTaskFree,
  eTimerAfterReset,
  eTimerDefaultMax,
  eTimerUserTimer = eTimerDefaultMax,
  /*
  Timer id for dynamic required from eTimerDefaultMax to eTimerMax
  */
  eTimerMax=255 // Set the timer maximum count to 255
}eSystemTimer;

typedef enum
{
    ePlatform_4K = 0,
    ePlatform_E4K,
    ePlatform_Max
}ePlatformType;

#ifdef QT_CPP
    #define CLOCKS_PER_MSEC (CLOCKS_PER_SEC/1000) // CLOCKS_PER_SEC = 1000
    #define xGetMSCount() ((uint32)clock() / CLOCKS_PER_MSEC)
#else
    #ifdef GEOMETRY
        #define xGetMSCount() current_timestamp() // Dennis Yeh : ((uint32)clock() / CLOCKS_PER_MSEC)
        #define MS_SLEEP(x) DelayMSec(x)
        long long current_timestamp();
    #endif
#endif

#ifdef QT_CPP
    #define LAN_BOARD_VER 1
    #define MS_SLEEP(x) DelayMSec(x)
    #include <stdio.h>
    #define DBMSG_ACCESS(...) dvProAV_AccessPrint(__VA_ARGS__)
    //#define DBMSG_ACCESS(...) do{ printf(__VA_ARGS__); fflush(stdout);}while(0)
    #define ASSERT_ACCESS(X) dvProAV_AccessPrint("\r\nASSERT: %s: %ld\r\n", __FILE__, __LINE__)
    #define DBMSG(...) do{ printf(__VA_ARGS__); fflush(stdout);}while(0)
    #define ASSERT(X) do { if (X) { dvProAV_AccessPrint("\r\nASSERT: %s: %ld\r\n", __FILE__, __LINE__);}}while(0)
    #define DBMSG_ALWAYS(...) DBMSG(__VA_ARGS__)
    #define ASSERT_ALWAYS() ASSERT(1)
    #define LOG_MSG(...) DBMSG(...)
    #define PROAV_CS_DELAY() do{}while(0) // cs delay = 50M * 3 clock = 60ns
#else
    #include "utilDbgMsg.h"
    #define DBMSG(...) do{ LOG_MSG(db_DV_SCALER, __VA_ARGS__);}while(0)
    //#define ASSERT(X) do { if (!(X)) { LOG_MSG(db_ALWAYS,"\r\nASSERT: %s: %d\r\n", __FILE__, __LINE__);}}while(0)
    #define DBMSG_ALWAYS(...) do {LOG_MSG(db_ALWAYS, __VA_ARGS__);} while(0)
    #define DBMSG_ACCESS(...) do{}while(0)
    #define PROAV_CS_DELAY() do{}while(0)
#endif



#ifdef __cplusplus
extern "C"
{
#endif

void DelayMSec(uint16 milli_seconds);
void ResetTime(uint08 id);
uint32 TimeElapsed(uint08 id);
bool TimerRequire(uint08 *newTimerId);
void TimerRelease(uint08 usedTimerId);
//-------------------------------------------------------------------------------------------------
// common func
//-------------------------------------------------------------------------------------------------
char hextoascii(char hex_byte);

#ifdef __cplusplus
}
#endif

#endif // DV_PROAV_PLATFORM_H_

