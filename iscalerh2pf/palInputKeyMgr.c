#include "halKeyCodeList.h"
#include "halInputCtrlAPI.h"
#include "utilDbgMsg.h"
#include "utilCounterAPI.h"
#include "utilMathAPI.h"
#include "utilQueueAPI.h"
#include "palInputKeyMgr.h"

#if 0
#ifdef PLATFORM_B35LH
static UINT16 m_wCustomCode = 0;
static UINT16 m_wDefaultCustomCode = 0;
static UINT16 m_wPowerKeyCode = 0;
#else
static UINT16 m_wCustomId = 0;
#endif
#endif

static UINT16 m_wCustomId = 0;

typedef eRESULT(*fpINPUT_CALLBACK)(void *);

fpINPUT_CALLBACK m_fpInputCallback = NULL;


eRESULT palInputKeyMgr_Init(void)
{
    return rcSUCCESS;
}

void palInputKeyMgr_Register_Callback(eRESULT(*fpCallback)(void *))
{
    m_fpInputCallback = fpCallback;
}

void palInputKeyMgr_Proc(sKEY_DATA *psKeyData)
{
    const sKEYLUT *psKeyLut = NULL;
    sKEY_DATA sOutPutKey = {0};
    UINT8  cKeyIndex = 0;
    UINT16 wKeyCode = 0;
    UINT8  cKeyCondition = 0;
    UINT16 wKeyNumber = 0;

    switch(psKeyData->eKeyType)
    {
        case eKEYINPUT_KEYPAD:
            psKeyLut = m_acKeyPadLUT;
            wKeyNumber = KEYPADNUMBER;
            sOutPutKey.eKeyType = eKEYINPUT_KEYPAD;
            break;

        case eKEYINPUT_IR:
        case eKEYINPUT_LAN:             //G100_Clare_0056
            psKeyLut = m_acIRKeyLUT;
            wKeyNumber = IRKEYPADNUMBER;
            sOutPutKey.eKeyType = eKEYINPUT_IR;
            break;

        default:
            ASSERT_ALWAYS();
            break;
    }
#if defined(CUSTOM_CHRISTIE)
    //Check Event Founded
    for(cKeyIndex = 0; cKeyIndex < wKeyNumber; cKeyIndex++)
    {
        if(psKeyLut[cKeyIndex].wKeyCode == psKeyData->wKeyCode)
        {
            cKeyCondition = psKeyData->eKeyEvent & psKeyLut[cKeyIndex].cConditions;

            switch(cKeyCondition)
            {
                case eKEY_EVENT_PRESSED:
                    wKeyCode = psKeyLut[cKeyIndex].wPressKey;
                    sOutPutKey.eKeyEvent = eKEY_EVENT_PRESSED;
                    break;

                case eKEY_EVENT_HOLD:
                    wKeyCode = psKeyLut[cKeyIndex].wHoldKey;
                    sOutPutKey.eKeyEvent = eKEY_EVENT_HOLD;
                    break;

                case eKEY_EVENT_RELEASE:
                    wKeyCode = psKeyLut[cKeyIndex].wReleaseKey;
                    sOutPutKey.eKeyEvent = eKEY_EVENT_RELEASE;
                    break;

                default:
                    //Wrong condition
                    ASSERT_ALWAYS();
                    return;
            }

            //Found Key
            if(eKEY_LIST_NUMBER > wKeyCode)
            {
                if(NULL != m_fpInputCallback)
                {
                    sOutPutKey.wKeyCode = wKeyCode;
                    m_fpInputCallback(&sOutPutKey);
                }
            }

            return;
        }
    }
#elif defined(CUSTOM_OPTOMA)

    wKeyCode = psKeyData->wKeyCode;

    //Found Key
    if(eKEY_LIST_NUMBER > wKeyCode)
    {
        if(NULL != m_fpInputCallback)
        {
            sOutPutKey.wKeyCode = wKeyCode;
            sOutPutKey.eKeyEvent = eKEY_EVENT_PRESSED;
            m_fpInputCallback(&sOutPutKey);
        }
    }
    else
    {
        return;
    }
#endif
}

void palInputKeyMgr_CmdProc(sKEY_DATA *psCmdKeyData)
{
    const sKEYCMDLUT *psKeyCmdLut = m_acIRCmdKeyLUT;
    sKEY_DATA sOutPutKey = {0};
    UINT8  cKeyIndex = 0;
    UINT16 wKeyCode = 0;
    UINT16 wKeyNumber = IRKEYPADNUMBER;
    eRESULT eResult = rcSUCCESS;

    sOutPutKey.eKeyType = eKEYINPUT_IR;
    sOutPutKey.eKeyEvent = eKEY_EVENT_PRESSED;

    //Check Event Founded
    for(cKeyIndex = 0; cKeyIndex < wKeyNumber; cKeyIndex++)
    {
        if(psKeyCmdLut[cKeyIndex].wKeyCmdCode == psCmdKeyData->wKeyCode)
        {

            wKeyCode = psKeyCmdLut[cKeyIndex].wPressKey;

            LOG_MSG(db_APP_INPUTKEY, "wKeyCode = %d ,psKeyCmdLutCmdCode = %d ucKeyCmdCode = %d\r\n",wKeyCode, psKeyCmdLut[cKeyIndex].wKeyCmdCode,psCmdKeyData->wKeyCode);

            //Found Key
            if(eKEY_LIST_NUMBER > wKeyCode)
            {
                if(NULL != m_fpInputCallback)
                {
                    sOutPutKey.wKeyCode = wKeyCode;
                    m_fpInputCallback(&sOutPutKey);
                }
            }

            return;
        }
    }
}


void palInputKeyMgr_IR_CustomId_Set(UINT16 wCustomId)
{
    m_wCustomId = wCustomId;
}


UINT16 palInputKeyMgr_IR_DefaultGet(void)
{
    return m_awCustomCode[DEFAULT_ID_NUMBER];
}

UINT16 palInputKeyMgr_IR_CustomIdGet(UINT8 ucID)
{
    if(CUSTOM_ID_COUNT > ucID)
    {
        return m_awCustomCode[ucID];
    }

    return m_awCustomCode[DEFAULT_ID_NUMBER];
}


