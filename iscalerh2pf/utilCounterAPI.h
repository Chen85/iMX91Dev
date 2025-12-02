#ifndef UTILCOUNTERAPI_H
#define UTILCOUNTERAPI_H
// ==============================================================================
// FILE NAME: UTILSYSCOUNTERAPI.H
// DESCRIPTION:
//
//
// modification history
// --------------------
// 19/03/2013, Leohong written
// --------------------
// ==============================================================================


#include "Common.h"
//#include "Board_SystemTick.h"

//#define MS_TO_TICKS(t) (UINT64)(((UINT64)(t) * SystemCoreClock)/1000)

#define ONE_SECOND  (1000)
#define ONE_MINUTE  (60 * ONE_SECOND)
#define ONE_HOUR    (60 * ONE_MINUTE)

typedef enum
{
    /* 00 */ eCOUNTER_TYPE_CNT_EVENT,
    /* 01 */ eCOUNTER_TYPE_HOTKEY,
    /* 02 */ eCOUNTER_TYPE_GUI_KEY,
    /* 03 */ eCOUNTER_TYPE_MSG_LENS,
    /* 04 */ eCOUNTER_TYPE_MSG_FOCUS,
    /* 05 */ eCOUNTER_TYPE_MSG_ZOOM,
    /* 06 */ eCOUNTER_TYPE_OSD_GRAY_OUT_MONITOR, //A35G2_CDS_Larry_0055 //A35G2_BRC_Casper_0101
    /* 07 */ eCOUNTER_TYPE_CLI_CMD_PROC_TASK_MONITOR,  //A35G2_Simon_0075
    /* 08 */ eCOUNTER_TYPE_CLI_CMD_DECODE_TASK_MONITOR,
    /* 09 */ eCOUNTER_TYPE_DATAPATH_TASK_MONITOR,
    /* 10 */ eCOUNTER_TYPE_SUB_DATAPATH_TASK_MONITOR,
    /* 11 */ eCOUNTER_TYPE_GUI_TASK_MONITOR,
    /* 12 */ eCOUNTER_TYPE_HOST_PATH_TASK_MONITOR,
    /* 13 */ eCOUNTER_TYPE_POLLING_TASK_MONITOR,
    /* 14 */ eCOUNTER_TYPE_IPC_RECV_DATA_TASK_MONITOR,
    /* 15 */ eCOUNTER_TYPE_SYSTEM_TASK_MONITOR,
    /* 16 */ eCOUNTER_TYPE_FACTORY_RESET, //R70K_AC_0048 //A70LK_Jacky_0050
    /* 17 */ eCOUNTER_TYPE_ENVIRONMENT,
    /* 18 */ eCOUNTER_TYPE_APP_LENS, //HICC2_Casper_0030 //HICC2_Casper_0040
    /* 19 */ eCOUNTER_TYPE_LENS_SHIFT_CONTROL, 

    eCOUNTER_TYPE_NUMBERS,
} eCOUNTER_TYPE;

typedef enum
{
    eTIMER_EVENT_TYPE_LAMP_LED_BLINK,
    eTIMER_EVENT_TYPE_BURNIN_LED_BLINK,
    eTIMER_EVENT_TYPE_R_FAN_RPM_FETCH,
    eTIMER_EVENT_SYSTEM_COUNT,
    eTIMER_EVENT_BLACKING_DISABLE_COUNT,

    eTIMER_EVENT_TYPE_NUMBERS,
} eTIMER_EVENT_TYPE;

typedef void (*fpTimerEventCallback)(void);

typedef struct
{
    UINT32 dwTimer;
    UINT32 dwReloadTimer;
    UINT8  cReload;
    UINT16 wRepeatCount;
    fpTimerEventCallback fpEventCallback;
    fpTimerEventCallback fpEventFinishCallback;
} sTIMER_EVENT;

void utilCounter_Init(void);
void utilCounterSet(eCOUNTER_TYPE eCounter, UINT32 dwMilliSecond);
UINT64 utilCounterGet(eCOUNTER_TYPE eCounter);

void utilCounter_Reg_TimerEvent(eTIMER_EVENT_TYPE eTimer,
                                UINT32 dwTimer,
                                UINT8 cReloard,
                                UINT16 wRepeatCount,
                                fpTimerEventCallback fpCallback,
                                fpTimerEventCallback fpFinishCallback);

void utilCounter_UnReg_TimerEvent(eTIMER_EVENT_TYPE eTimer);
UINT32 utilCounter_Tick_Get(void);
INT16 utilCounter_Event_Process(UINT16 uiTick);


#endif // UTILCOUNTERAPI_H

