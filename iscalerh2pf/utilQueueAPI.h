#ifndef UTILQUEUEAPI_H
#define UTILQUEUEAPI_H
// ==============================================================================
// FILE NAME: UTILQUEUEAPI.H
// DESCRIPTION:
//
//
// modification history
// --------------------
// 18/03/2013, Leohong Create
// --------------------
// ==============================================================================


#include "Common.h"


typedef struct
{
    UINT16 wFront;
    UINT16 wRear;
    UINT16 wItemCount;
    UINT16 wItemSize;
    UINT8 *pcBuffer;
} sQUEUE;

void utilQueueInitial(sQUEUE *psQueue, UINT16 wItemCount, UINT16 wItemSize, UINT8 *pcQueueBuffer);
eRESULT utilQueueWrite(sQUEUE *psQueue, UINT8 *pcData);
eRESULT utilQueueRead(sQUEUE *psQueue, UINT8 *pcData);
UINT16 utilQueueDataCountGet(sQUEUE *psQueue);
BOOL utilIsQueueFull(sQUEUE *psQueue);
void utilQueueFlush(sQUEUE *psQueue);

eRESULT utilQueueMessageInit(mqd_t *pmQueueHandle, char *cName, UINT16 wMsgCount, UINT16 wMsgSize);
eRESULT utilQueueMessageSend(mqd_t mQueueHandle, UINT8 *pcData, UINT16 wMsgSize, UINT16 wTimeOut);
eRESULT utilQueueMessageReceive(mqd_t mQueueHandle, UINT8 *pcData, UINT16 wMsgSize, UINT16 wTimeOut);

#endif // UTILQUEUEAPI_H

