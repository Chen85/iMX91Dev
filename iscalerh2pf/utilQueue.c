// ==============================================================================
// FILE NAME: UTILQUEUE.C
// DESCRIPTION:
//                          Queue
//          -------------------------------------
// <---Front|   |   |   |   |   |   |   |   |   |<----Rear
//          -------------------------------------
//
// modification history
// --------------------
// 18/03/2013, Leohong Create
// --------------------
// ==============================================================================


#include "utilQueueAPI.h"
#include "utilDbgMsg.h"


// ==============================================================================
// FUNCTION NAME: UTILQUEUEINITIAL
// DESCRIPTION:
//
// wSize must be multiple of wItemSize
//
// Params:
// sQUEUE *psQueue:
// UINT16 wItemCount:
// UINT16 wItemSize:
// UINT8 *pcQueueBuffer:
//
// Returns:
//
//
// modification history
// --------------------
// 29/10/2013, Leo Create
// --------------------
// ==============================================================================
void utilQueueInitial(sQUEUE *psQueue, UINT16 wItemCount, UINT16 wItemSize, UINT8 *pcQueueBuffer)
{
    ASSERT(0 != wItemCount);
    ASSERT(0 != wItemSize);
    ASSERT(NULL != psQueue);
    ASSERT(NULL != pcQueueBuffer);

    psQueue->wItemCount = wItemCount;
    psQueue->wFront = psQueue->wRear = 0;
    psQueue->wItemSize = wItemSize;
    psQueue->pcBuffer = pcQueueBuffer;
}

// ==============================================================================
// FUNCTION NAME: UTILQUEUEWRITE
// DESCRIPTION:
//
//
// Params:
// sQUEUE *psQueue:
// UINT8 *pcData:
//
// Returns:
//
//
// modification history
// --------------------
// 29/10/2013, Leo Create
// --------------------
// ==============================================================================
eRESULT utilQueueWrite(sQUEUE *psQueue, UINT8 *pcData)
{
    ASSERT(NULL != psQueue);
    ASSERT(NULL != pcData);

    if(NULL == psQueue || NULL == pcData)
    {
        return rcERROR;
    }

    if(psQueue->wFront == (psQueue->wRear + 1) % psQueue->wItemCount)
    {
        return rcERROR;
    }
    else
    {
        memcpy((psQueue->pcBuffer + psQueue->wRear * psQueue->wItemSize),
               pcData,
               psQueue->wItemSize);

        psQueue->wRear = (psQueue->wRear + 1) % psQueue->wItemCount;
    }

    return rcSUCCESS;
}

// ==============================================================================
// FUNCTION NAME: UTILQUEUEREAD
// DESCRIPTION:
//
//
// Params:
// sQUEUE *psQueue:
// UINT8 *pcData:
//
// Returns:
//
//
// modification history
// --------------------
// 29/10/2013, Leo Create
// --------------------
// ==============================================================================
eRESULT utilQueueRead(sQUEUE *psQueue, UINT8 *pcData)
{
    ASSERT(NULL != psQueue);
    ASSERT(NULL != pcData);

    if(NULL == psQueue || NULL == pcData)
    {
        return rcERROR;
    }

    if(psQueue->wFront == psQueue->wRear)
    {
        return rcERROR;
    }
    else
    {
        memcpy(pcData,
               (psQueue->pcBuffer + psQueue->wFront * psQueue->wItemSize),
               psQueue->wItemSize);

        psQueue->wFront = (psQueue->wFront + 1) % psQueue->wItemCount;
        return rcSUCCESS;
    }
}

// ==============================================================================
// FUNCTION NAME: UTILQUEUEDATACOUNTGET
// DESCRIPTION:
//
//
// Params:
// sQUEUE *psQueue:
//
// Returns:
//
//
// modification history
// --------------------
// 29/10/2013, Leo Create
// --------------------
// ==============================================================================
UINT16 utilQueueDataCountGet(sQUEUE *psQueue)
{
    UINT16 wSize = 0;

    ASSERT(NULL != psQueue);

    if(psQueue->wFront == psQueue->wRear)
    {
        wSize = 0;
    }
    else
    {
        if(psQueue->wRear > psQueue->wFront)
        {
            wSize = psQueue->wRear - psQueue->wFront;
        }
        else
        {
            wSize = psQueue->wItemCount + psQueue->wFront - psQueue->wRear;
        }
    }

    return wSize;
}

// ==============================================================================
// FUNCTION NAME: UTILISQUEUEFULL
// DESCRIPTION:
//
//
// Params:
// sQUEUE *psQueue:
//
// Returns:
//
//
// modification history
// --------------------
// 29/10/2013, Leo Create
// --------------------
// ==============================================================================
BOOL utilIsQueueFull(sQUEUE *psQueue)
{
    ASSERT(NULL != psQueue);

    if(psQueue->wFront == (psQueue->wRear + 1) % psQueue->wItemCount)
    {
        return TRUE;
    }

    return FALSE;
}

// ==============================================================================
// FUNCTION NAME: UTILQUEUEFLUSH
// DESCRIPTION:
//
//
// Params:
// sQUEUE *psQueue:
//
// Returns:
//
//
// modification history
// --------------------
// 29/10/2013, Leo Create
// --------------------
// ==============================================================================
void utilQueueFlush(sQUEUE *psQueue)
{
    ASSERT(NULL != psQueue);

    psQueue->wFront = psQueue->wRear = 0;
}

// ==============================================================================
// FUNCTION NAME: utilMessageQueueInitial
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
// 2020/08/27, Larry Create
// --------------------
// ==============================================================================
eRESULT utilQueueMessageInit(mqd_t *pmQueueHandle, char *cName, UINT16 wMsgCount, UINT16 wMsgSize)
{
    struct mq_attr attr;
    attr.mq_maxmsg = wMsgCount;
    attr.mq_msgsize = wMsgSize;

    if((*pmQueueHandle = mq_open(cName, O_RDWR | O_CREAT | O_NONBLOCK, 0666, &attr)) == -1)
    {
        return rcERROR;
    }

    return rcSUCCESS;
}

// ==============================================================================
// FUNCTION NAME: utilMessageQueueSend
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
// 2020/08/27, Larry Create
// --------------------
// ==============================================================================
eRESULT utilQueueMessageSend(mqd_t mQueueHandle, UINT8 *pcData, UINT16 wMsgSize, UINT16 wTimeOut)
{    
    eRESULT eResult = rcERROR;

    if(wTimeOut)
    {
        while(wTimeOut--)
        {
            if(mq_send(mQueueHandle, (char const *)pcData, wMsgSize, 0) < 0)
            {
                MS_SLEEP(1);
            }
            return rcSUCCESS;
        }
        
        eResult = rcERROR;
    }
    else
    {
        if(mq_send(mQueueHandle, (char const *)pcData, wMsgSize, 0) < 0)
        {
            eResult = rcERROR;
        }
        else
        {
            eResult = rcSUCCESS;
        }
    }

    return eResult;

}

// ==============================================================================
// FUNCTION NAME: utilMessageQueueReceive
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
// 2020/08/27, Larry Create
// --------------------
// ==============================================================================
eRESULT utilQueueMessageReceive(mqd_t mQueueHandle, UINT8 *pcData, UINT16 wMsgSize, UINT16 wTimeOut)
{
    UINT8 *pcTemp = NULL;

    pcTemp = malloc(wMsgSize);

    if(pcTemp == NULL)
    {
        return rcERROR;
    }
        
    if(wTimeOut)
    {
        while(wTimeOut--)
        {
            if(mq_receive(mQueueHandle, (char *)pcTemp, wMsgSize, NULL) < 0)
            {
                MS_SLEEP(1);
            }
            else
            {
                memcpy(pcData, pcTemp, wMsgSize);
                free(pcTemp);
                return rcSUCCESS;
            }
        }
    }
    else
    {
        if(mq_receive(mQueueHandle, (char *)pcTemp, wMsgSize, NULL) < 0)
        {
            
        }
        else
        {
            memcpy(pcData, pcTemp, wMsgSize);
            free(pcTemp);
            return rcSUCCESS;
        }
    }
    
    free(pcTemp);
    return rcERROR;
}

