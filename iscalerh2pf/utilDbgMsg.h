#ifndef _UTILDBGMSG_H_
#define _UTILDBGMSG_H_

#ifdef __cplusplus
extern "C" {
#endif

//#include "utilStorageCfg.h"
#include "Common.h"
#define BUFFER_SIZE (1024)

#define LOG_WITH_TIME (FALSE)

#define LOG_MSG(mask, fmt, ...) utilDbgMsg_ftrace(mask, fmt, ##__VA_ARGS__)
//#define LOG_MSG(mask, fmt, ...) printf(fmt, ##__VA_ARGS__)

#define SYSTEM_CALL(fmt, ...)   utilSystem_Call(fmt, ##__VA_ARGS__)

#define _FUNCTION_IN_LOG_       //LOG_MSG(db_TRACE_CODE, ">(func:%s, line:%d)\n" , __FUNCTION__, __LINE__);
#define _FUNCTION_OUT_LOG_      //LOG_MSG(db_TRACE_CODE, "<(func:%s, line:%d)\n" , __FUNCTION__, __LINE__);

typedef struct
{
    sPROCESS_MUTEX_DATA         *psDebugMsgMutex;
    pthread_mutex_t              xDebugMs_Mutex;
    pthread_mutexattr_t          xDebugMs_MutexAttr;

    UINT64                       ullMask;
    char                         aucMsg[BUFFER_SIZE];
}sDBGMSG_INFORMATION, *PsDBGMSG_INFORMATION;

eEXEC_CODE utilDbgMsg_Init(UINT64 ulMask);

void utilDbgMsg_ftrace(UINT64 ulBitmask, const char *pfmtstring, ...);
void utilSystem_Call(const char *pfmtstring, ...);


#define db_ALWAYS                  ((UINT64)0x01 << 0)
#define db_APP_SYSTEM              ((UINT64)0x01 << 1)
#define db_APP_DATAPATH            ((UINT64)0x01 << 2)
#define db_APP_SCALER              ((UINT64)0x01 << 3)
#define db_APP_MAILBOX             ((UINT64)0x01 << 4)
#define db_APP_POLL                ((UINT64)0x01 << 5)
#define db_APP_ENVIRONMENT         ((UINT64)0x01 << 6)
#define db_APP_GUI                 ((UINT64)0x01 << 7)
#define db_APP_ILLUMINATION        ((UINT64)0x01 << 8)
#define db_APP_DATA_MGR            ((UINT64)0x01 << 9)
#define db_APP_CORE                ((UINT64)0x01 << 10)
#define db_APP_CLI                 ((UINT64)0x01 << 11)
#define db_APP_ETHERNET            ((UINT64)0x01 << 12)
#define db_APP_INPUTKEY            ((UINT64)0x01 << 13)
#define db_APP_SCHEDULE            ((UINT64)0x01 << 14)
#define db_APP_MONITOR             ((UINT64)0x01 << 15)
#define db_APP_LAN                 ((UINT64)0x01 << 16)
#define db_APP_CLI_LAN             ((UINT64)0x01 << 17)
#define db_APP_ACK_LAN             ((UINT64)0x01 << 18)
#define db_APP_TASK                ((UINT64)0x01 << 19)
#define db_APP_UI_EVENT            ((UINT64)0x01 << 20)

#define db_HAL_SCALER			   ((UINT64)0x01 << 21)
#define db_HAL_EEP				   ((UINT64)0x01 << 22)
#define db_HAL_WARPING             ((UINT64)0x01 << 23)
#define db_HAL_GUI                 ((UINT64)0x01 << 24)
#define db_HAL_IR                  ((UINT64)0x01 << 25)
#define db_HAL_FORMATTER           ((UINT64)0x01 << 26)
#define db_HAL_LD           	   ((UINT64)0x01 << 27)
#define db_HAL_FAN           	   ((UINT64)0x01 << 28)
#define db_HAL_MCU                 ((UINT64)0x01 << 29)
#define db_HAL_GUI_CK              ((UINT64)0x01 << 30)
#define db_HAL_SOURCE_INFO         ((UINT64)0x01 << 31)
#define db_HAL_PROAV               ((UINT64)0x01 << 32)
#define db_HAL_PROAV_GUI           ((UINT64)0x01 << 33)
#define db_HAL_HDMIRX              ((UINT64)0x01 << 34)
#define db_HAL_SEMAPHORE           ((UINT64)0x01 << 35)
#define db_HAL_LENS                ((UINT64)0x01 << 36)
#define db_HAL_RESERVED16          ((UINT64)0x01 << 37)
#define db_HAL_RESERVED17          ((UINT64)0x01 << 38)
#define db_HAL_BACKUPSOURCE        ((UINT64)0x01 << 39)
#define db_HAL_RESERVED19          ((UINT64)0x01 << 40)


#define db_DV_SEMAPHORE	           ((UINT64)0x01 << 41)
#define db_DV_RESERVED01           ((UINT64)0x01 << 42)
#define db_DV_RESERVED02           ((UINT64)0x01 << 43)
#define db_DV_PROAV_OSD            ((UINT64)0x01 << 44)
#define db_DV_VCXO                 ((UINT64)0x01 << 45)
#define db_DV_I2C                  ((UINT64)0x01 << 46)
#define db_DV_PROAV_WARPING        ((UINT64)0x01 << 47)
#define db_DV_FAN            	   ((UINT64)0x01 << 48)
#define db_DV_SCALER               ((UINT64)0x01 << 49)  //A70LV_Doulas_2000
#define db_DV_SCALER_OSD           ((UINT64)0x01 << 50)  //A70LV_Larry_0001

#define db_UTL_CLI                 ((UINT64)0x01 << 51)
#define db_DV_3D_CONFIG            ((UINT64)0x01 << 52)
#define db_DV_GEO                  ((UINT64)0x01 << 53)     //A70LV_Doulas_0036
#define db_DV_C789                 db_DV_GEO

#define db_UPGRADE                 ((UINT64)0x01 << 54) //A70LV_Larry_0117
#define db_UTL_CMD                 ((UINT64)0x01 << 55)
#define db_ADC_CALIBARATION        ((UINT64)0x01 << 56)     //A70LV_Doulas_0124
#define db_UTL_DATABASE            ((UINT64)0x01 << 57)
#define db_IPC                     ((UINT64)0x01 << 58)
#define db_IPC_REV                 ((UINT64)0x01 << 59)

#define db_TRACE_CODE              ((UINT64)0x01 << 60)

#define db_BIST                    ((UINT64)0x01 << 61)

#define db_KEEP_UART_SW            ((UINT64)0x01 << 62)

#define db_ASSERT                  ((UINT64)0x01 << 63)


#define db_ALL                     (UINT64)(0xFFFFFFFFFFFFFFFFL)

#if 0
#ifdef WARP_ENABLE
#define db_MSG                      db_APP_CLI//db_ALL//db_APP_SYSTEM
#else
#define db_MSG                      db_APP_SYSTEM | db_APP_DATAPATH | db_APP_SCALER | db_HAL_SCALER | db_DV_C734
#endif
#endif

#define db_MSG                      0//db_APP_SYSTEM | db_APP_DATAPATH | db_APP_SCALER | db_HAL_SCALER | db_DV_SI9777 | db_DV_C790 | db_APP_CLI | db_APP_GUI |db_HAL_FORMATTER

/* __ASM("BKPT 0");\ */
#define ASSERT(X) \
    do\
    {\
        if(!(X))\
        {\
            LOG_MSG(db_ALWAYS, "\r\nASSERT: %s: %d\r\n", __FILE__, __LINE__);\
        }\
    }\
    while(0)

#define ASSERT_ALWAYS() \
    do\
    {\
        {\
            LOG_MSG(db_ALWAYS, "\r\nASSERT_ALWAYS: %s: %d\r\n", __FILE__, __LINE__);\
        }\
    }\
    while(0)


void utilDbgMsg_Set(UINT64 ullMask, UINT8 cEnable);
UINT64 utilDbgMsg_Get(UINT64 ullMask);
void utilDbgMsg_ReSet(UINT64 ullMask, UINT64 ullMaskCT);

void utilDbgMsgCT_Set(UINT64 ullMask, UINT8 cEnable);
UINT64 utilDbgMsgCT_Get(UINT64 ullMask);

//#define LPC4357_DEMO_BOARD_DEBUG


#ifdef __cplusplus
}
#endif

#endif  //_UTILDBGMSG_H_
