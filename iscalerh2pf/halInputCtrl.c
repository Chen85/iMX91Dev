// ==============================================================================
// FILE NAME: HALINPUTCTRL.C
// DESCRIPTION:
//
//
// modification history
// --------------------
// 23/04/2013, Leohong Create
// --------------------
// ==============================================================================

#if 0

//#include "Board_Gpio.h"
//#include "Board_Gpio_Table_T100.h"
//#include "Board_Timer.h"
//#include "Board_ADC.h"
#include "halKeyCodeList.h"
#include "halInputCtrlAPI.h"
#include "utilDbgMsg.h"
#include "utilCounterAPI.h"
#include "utilMathAPI.h"
#include "utilQueueAPI.h"

#define DEBOUNDCETIMER  (50) //50ms
#define PRESSTHRESHOLD  (2)
#define HOLDTHRESHOLD   (10)
#define NO_KEY          (UINT32)(0xFFFFFFFF)
#define IRREPEATCOUNT   (5)

static UINT16 m_wKeyCheckCount;
static UINT32 m_wLastKey = eKEY_LIST_NUMBER;


static BOOL m_IrKeyFound = FALSE;
static BOOL m_IrKeyHold = FALSE;
static UINT8 m_LastIrKeyCode = 0;
static eIR_STATE m_eIRState = eIR_STATE_HEAD;
static UINT8 m_cIRDataCount = 0;
static UINT8 m_acIRdata[4] = {0};
#ifdef PLATFORM_B35LH //B35LH_Casper_0008
static UINT16 m_wCustomCode = 0;
static UINT16 m_wDefaultCustomCode = 0;
static UINT16 m_wPowerKeyCode = 0;
#else
static UINT16 m_wCustomId = 0;
#endif
static UINT16 m_wIrRepeatCount = 0;
static UINT8 m_cDetectIspKey = FALSE;

static sQUEUE m_asIR_Ring_Buffer;
static UINT32 m_adwIR_Buffer[IR_BUFFER_SIZE];
static UINT32 m_dwLastwCaptureData = 0;

typedef eRESULT(*fpINPUT_CALLBACK)(void *);

fpINPUT_CALLBACK m_fpInputCallback = NULL;



///////////////////////////////////////////////////////////////////////////////
//////////////////////////////  Keypad  ////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#define ADC_KEY_IDEL_VALUE      (1024)
#define ADC_KEYS_PER_CHANNEL    (5)
#define ADC_BASIC_LEVEL         (ADC_KEY_IDEL_VALUE / ADC_KEYS_PER_CHANNEL)
#define ADC_KEY_TOLERANCE       (ADC_KEY_IDEL_VALUE / (ADC_KEYS_PER_CHANNEL * 2))
#define ADC_KEY_NO_KEY          ((ADC_KEY_IDEL_VALUE - ADC_BASIC_LEVEL) + ADC_KEY_TOLERANCE)
#define ADC1_KEY_BIT            ((UINT32)0x0001 << 8)
#define ADC2_KEY_BIT            (((UINT32)0x0001 << 8) << ADC_KEYS_PER_CHANNEL)


volatile static sADC_KEY_BUF m_asKeyAdcFtBuffer[eKEYPAD_ADC_CH_NUMBERS];

#if 0
static sADC_CH_CFG m_asAdcCh_Cfg[eKEYPAD_ADC_CH_NUMBERS] =
{
    {
        eBOARD_ADC_1,           // eAdcID
        eBOARD_ADC_KEYPAD2,     // eAdcChn
        FALSE,                  // cEnableInt
        NULL,                   // pfAdcCallBack
    },
    {
        eBOARD_ADC_0,           // eAdcID
        eBOARD_ADC_KEYPAD1,     // eAdcChn
        FALSE,                  // cEnableInt
        NULL,                   // pfAdcCallBack
    },
};
#endif /* 0 */

//Adc KeyPad code
static const UINT16 m_awAdc1KeyPadLUT[] =
{
    //keLENS,
    (KEY_ADC_LEVEL * 4),
    //keBACK,
    (KEY_ADC_LEVEL * 3),
    //keMENU,
    (KEY_ADC_LEVEL * 2),
    //keHELP,
    (KEY_ADC_LEVEL * 1),
    //keZOOM,
    (KEY_ADC_LEVEL * 0),
};

static const UINT16 m_awAdc2KeyPadLUT[] =
{
    //keOSD,
    (KEY_ADC_LEVEL * 4),
    //keFOCUS,
    (KEY_ADC_LEVEL * 3),
    //keINPUT,
    (KEY_ADC_LEVEL * 2),
    //keAUTO,
    (KEY_ADC_LEVEL * 1),
    //keSHUTTER,
    (KEY_ADC_LEVEL * 0),
};

static UINT8 m_cEnableDebug = FALSE;




// ==============================================================================
// FUNCTION NAME: HALINPUTCTRL_KEYPADGET
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
// 19/12/2013, Leo Create
// --------------------
// ==============================================================================
UINT32 halInputCtrl_KeyPadGet(void)
{
    static UINT32_VAL uKeyPad = {0};
    UINT8  cScanIndex = 0;
    UINT16 wAdcData = 0;
    UINT16 wTempData = 0;

    uKeyPad.Val = 0xFFFFFFFF;

#if 0
#if 0
    uKeyPad.bits.b0 = !!(Board_Gpio_Get(pLS_H_STEP));
#else
    uKeyPad.bits.b0 = !!(Board_Gpio_Get(pKEY_PWR));
    uKeyPad.bits.b1 = !!(Board_Gpio_Get(pKEY_UP));
    uKeyPad.bits.b2 = !!(Board_Gpio_Get(pKEY_DOWN));
    uKeyPad.bits.b3 = !!(Board_Gpio_Get(pKEY_LEFT));
    uKeyPad.bits.b4 = !!(Board_Gpio_Get(pKEY_RIGHT));
    uKeyPad.bits.b5 = !!(Board_Gpio_Get(pKEY_ENTER));

    if(TRUE == m_cDetectIspKey)
    {
        uKeyPad.bits.b6 = !!(Board_Gpio_Get(pNXP_A9));
    }

#endif /* 1 */
#endif /* 0 */


    halInputCtrl_Adc_Buffer_Update();


    // 偵測channel 1
    wAdcData = halInputCtrl_Adc_Get(eKEYPAD_ADC_CH0);
    wTempData = 0;

    if(ADC_KEY_NO_KEY > wAdcData)
    {
        for(cScanIndex = 0; cScanIndex < ADC_KEYS_PER_CHANNEL; cScanIndex++)
        {
            wTempData = ABS_DIFF(wAdcData, m_awAdc1KeyPadLUT[cScanIndex]);

            if(ADC_KEY_TOLERANCE > wTempData)
            {
                break;
            }
        }

        uKeyPad.Val = uKeyPad.Val & (~(ADC1_KEY_BIT << cScanIndex));
    }

    // 偵測channel 2
    wAdcData = halInputCtrl_Adc_Get(eKEYPAD_ADC_CH1);
    wTempData = 0;

    if(ADC_KEY_NO_KEY > wAdcData)
    {
        for(cScanIndex = 0; cScanIndex < ADC_KEYS_PER_CHANNEL; cScanIndex++)
        {
            wTempData = ABS_DIFF(wAdcData, m_awAdc2KeyPadLUT[cScanIndex]);

            if(ADC_KEY_TOLERANCE > wTempData)
            {
                break;
            }
        }

        uKeyPad.Val = uKeyPad.Val & (~(ADC2_KEY_BIT << cScanIndex));
    }



    return uKeyPad.Val;
}

// ==============================================================================
// FUNCTION NAME: halInputCtrl_Detect_ISP_Key
// DESCRIPTION:
//
//
// Params:
// UINT8 cEnable:
//
// Returns:
//
//
// Modification History
// --------------------
// 2015/03/05, Leo Create
// --------------------
// ==============================================================================
void halInputCtrl_Detect_ISP_Key(UINT8 cEnable)
{
    m_cDetectIspKey = cEnable;
}

#if 0
// ==============================================================================
// FUNCTION NAME: HALINPUTCTRL_KEYPAD_DECODE
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
// 23/04/2013, Leohong Create
// --------------------
// ==============================================================================
void halInputCtrl_KeyPad_Decode(void)
{
    sKEY_DATA sKeyStatus;

    sKeyStatus.eKeyType = eKEYINPUT_KEYPAD;
    sKeyStatus.eKeyEvent = eKEY_EVENT_NUMBER;
    sKeyStatus.wKeyCode = halInputCtrl_KeyPadGet();

    //If new key pressed Reset the deboundce Timer
    if(sKeyStatus.wKeyCode != m_wLastKey)
    {
        utilCounterSet(eCOUNTER_TYPE_KEY, DEBOUNDCETIMER);

        //check key release
        if(m_wKeyCheckCount >= HOLDTHRESHOLD)
        {
            m_wKeyCheckCount = 0;
            sKeyStatus.wKeyCode = m_wLastKey;
            sKeyStatus.eKeyEvent = eKEY_EVENT_RELEASE;

            //Process Key
            halInputCtrl_Proc(&sKeyStatus);
            return;
        }
    }

    //check key press with 100ms debounce
    if((sKeyStatus.wKeyCode == m_wLastKey) && (NO_KEY != sKeyStatus.wKeyCode))
    {
        if(0 == utilCounterGet(eCOUNTER_TYPE_KEY))
        {
            utilCounterSet(eCOUNTER_TYPE_KEY, DEBOUNDCETIMER);

            m_wKeyCheckCount++;

            if(m_wKeyCheckCount >= HOLDTHRESHOLD)
            {
                m_wKeyCheckCount = HOLDTHRESHOLD;
                sKeyStatus.wKeyCode = m_wLastKey;
                sKeyStatus.eKeyEvent = eKEY_EVENT_HOLD;

                //Process Key
                halInputCtrl_Proc(&sKeyStatus);
            }
            else if(PRESSTHRESHOLD == m_wKeyCheckCount)
            {
                sKeyStatus.eKeyEvent = eKEY_EVENT_PRESSED;

                //Process Key
                halInputCtrl_Proc(&sKeyStatus);
            }
        }

        m_wLastKey = sKeyStatus.wKeyCode;
    }
    else
    {
        m_wKeyCheckCount = 0;
        m_wLastKey = sKeyStatus.wKeyCode;
        utilCounterSet(eCOUNTER_TYPE_KEY, 0);
    }
}
#endif /* 0 */

// ==============================================================================
// FUNCTION NAME: HALINPUTCTRL_PROC
// DESCRIPTION:
//
//
// Params:
// sKEY_DATA sKeyStatus:
//
// Returns:
//
//
// modification history
// --------------------
// 23/04/2013, Leohong Create
// --------------------
// ==============================================================================
void halInputCtrl_Proc(sKEY_DATA *psKeyData)
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
    #ifdef PLATFORM_B35LH //B35LH_Casper_0008

    //check Power Key //B70_Larry_012 start
    if(m_wPowerKeyCode == psKeyData->wKeyCode)
    {
        cKeyCondition = psKeyData->eKeyEvent & (eKEY_EVENT_PRESSED | eKEY_EVENT_HOLD | eKEY_EVENT_RELEASE);
        switch(cKeyCondition)
        {
            case eKEY_EVENT_PRESSED:
                wKeyCode = kePOWERON;
                sOutPutKey.eKeyEvent = eKEY_EVENT_PRESSED;
                break;

            case eKEY_EVENT_HOLD:
                wKeyCode = keHOLD_POWERON;
                sOutPutKey.eKeyEvent = eKEY_EVENT_HOLD;
                break;

            case eKEY_EVENT_RELEASE:
                wKeyCode = keRELEASE_POWERON;
                sOutPutKey.eKeyEvent = eKEY_EVENT_RELEASE;
                break;

            default:
                //Wrong condition
                ASSERT_ALWAYS();
                return;
        }

        sOutPutKey.wKeyCode = wKeyCode;
        m_fpInputCallback(&sOutPutKey);

        return;
    }
    //B70_Larry_012 end
    #endif

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
}

#if defined(CUSTOM_BARCO) //A35G2_BRC_Casper_0049
void halInputCmdCtrl_Proc(sKEY_DATA *psCmdKeyData)
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
#endif

// ==============================================================================
// FUNCTION NAME: HALINPUTCTRL_INIT
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
// 29/10/2013, Leo Create
// --------------------
// ==============================================================================
eRESULT halInputCtrl_Init(void)
{
    halInputCtrl_IR_Restart();

    // Global Input Multiplexer Array (GIMA)
    //Board_Gpio_Gima_PinMux((sGIMA_PIN_CFG *)m_asGimaMuxCfgTable[0].psGimaCgf, m_asGimaMuxCfgTable[0].uiPinNum); //A70LV_Larry_0023
    //Board_Gpio_IntPin_Select((sGPIO_INT_CFG *)m_asGpio_INT_Table, GPIO_INT_PIN_NUMBER); //A70LV_Larry_0018

    utilQueueInitial(&m_asIR_Ring_Buffer, IR_BUFFER_SIZE, sizeof(UINT32) / sizeof(UINT8), (UINT8*)&m_adwIR_Buffer);


    //Board_Timer_Register_Callback(eBOARD_TIMER_TIMER1, halInputCtrl_IR_PulseInsert);

    return rcSUCCESS;
}

#if 0
// ==============================================================================
// FUNCTION NAME: HALINPUTCTRL_IR_HEADGET
// DESCRIPTION:
//
//
// Params:
//
// Returns:
//
//
// modification history
// --------------------
// 23/04/2013, Leohong Create
// --------------------
// ==============================================================================
eRESULT halInputCtrl_IR_HeadGet(void)
{
    eRESULT eResult = rcERROR;
    sKEY_DATA sKeyStatus;
    UINT32 dwPulseWidth = 0;

    eResult = (eRESULT)utilQueueRead(&m_asIR_Ring_Buffer, (UINT8*)&dwPulseWidth);

    while(rcSUCCESS == eResult)
    {
        if((dwPulseWidth >= LEADER_MIN) && (dwPulseWidth <= LEADER_MAX))
        {
            //reset repeat flag
            m_wIrRepeatCount = 0;
            m_IrKeyFound = FALSE;
            return rcSUCCESS;
        }

        //look for repeat
        //look for leader
        if(m_IrKeyFound)
        {
            if((dwPulseWidth >= REPEAT_MIN) && (dwPulseWidth <= REPEAT_MAX))
            {
                utilCounterSet(eCOUNTER_TYPE_IRDECODE, 400); //A70LV_Larry_0144

                if(m_wIrRepeatCount >= IRREPEATCOUNT)
                {
                    m_wIrRepeatCount = IRREPEATCOUNT;
                    sKeyStatus.wKeyCode = m_LastIrKeyCode;
                    sKeyStatus.eKeyEvent = eKEY_EVENT_HOLD;
                    sKeyStatus.eKeyType = eKEYINPUT_IR;
                    m_IrKeyHold = TRUE;

                    //Process Key
                    halInputCtrl_Proc(&sKeyStatus);
                    return rcERROR;
                }
                else
                {
                    m_wIrRepeatCount++;
                }
            }
        }

        eResult = (eRESULT)utilQueueRead(&m_asIR_Ring_Buffer, (UINT8*)&dwPulseWidth);
    }

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: HALINPUTCTRL_IR_DATAGET
// DESCRIPTION:
//
//
// Params:
//
// Returns:
//
//
// modification history
// --------------------
// 23/04/2013, Leohong Create
// --------------------
// ==============================================================================
eRESULT halInputCtrl_IR_DataGet(void)
{
    sKEY_DATA sKeyStatus;
    UINT32 dwPulseWidth = 0;
    UINT8 cBit = 0;
    #ifdef PLATFORM_B35LH //B35LH_Casper_0008
    UINT16 wCustomCode = 0;
    #else
    UINT16 cCustomId = 0;
    #endif

    while(rcSUCCESS == (eRESULT)utilQueueRead(&m_asIR_Ring_Buffer, (UINT8*)&dwPulseWidth))
    {
        if((dwPulseWidth >= BIT_0_MIN) && (dwPulseWidth <= BIT_0_MAX))
        {
            cBit = 0;
        }
        else if((dwPulseWidth >= BIT_1_MIN) && (dwPulseWidth <= BIT_1_MAX))
        {
            cBit = 1;
        }
        else
        {
            m_cIRDataCount = 0;
            return rcERROR;
        }

        m_acIRdata[(m_cIRDataCount / IR_BYTE_SIZE)] |= (cBit << (m_cIRDataCount % IR_BYTE_SIZE));
        m_cIRDataCount++;

        if(m_cIRDataCount >= IR_DATA_SIZE)
        {
            m_cIRDataCount = 0;
    #ifdef PLATFORM_B35LH //B35LH_Casper_0008
            wCustomCode = (m_acIRdata[0] << 8) | m_acIRdata[1];
    #else
            cCustomId = (m_acIRdata[0] << 8) | m_acIRdata[1];
    #endif

        #ifdef PLATFORM_B35LH //B35LH_Casper_0008
            if((m_wDefaultCustomCode == wCustomCode) || (m_wCustomCode == wCustomCode))
        #else
            if((m_awCustomCode[m_wCustomId] == cCustomId) || (m_awCustomCode[DEFAULT_ID_NUMBER] == cCustomId))
        #endif
           {
                m_IrKeyFound = TRUE;
                sKeyStatus.wKeyCode = m_acIRdata[2];
                sKeyStatus.eKeyEvent = eKEY_EVENT_PRESSED;
                sKeyStatus.eKeyType = eKEYINPUT_IR;

                //Process Key
                halInputCtrl_Proc(&sKeyStatus);

                //Set data for repeat condition
                if(m_LastIrKeyCode != sKeyStatus.wKeyCode)
                {
                    m_LastIrKeyCode = sKeyStatus.wKeyCode;
                }
            }

            LOG_MSG(db_HAL_IR,
                    "0x%02X,0x%02X,0x%02X,0x%02X\r\n",
                    m_acIRdata[0], m_acIRdata[1],
                    m_acIRdata[2], m_acIRdata[3]);

            return rcSUCCESS;
        }
    }

    return rcBUSY;
}

// ==============================================================================
// FUNCTION NAME: HALINPUTCTRL_IR_DECODE
// DESCRIPTION:
//
//
// Params:
//
// Returns:
//
//
// modification history
// --------------------
// 23/04/2013, Leohong Create
// --------------------
// ==============================================================================
void halInputCtrl_IR_Decode(void)
{
    sKEY_DATA sKeyStatus;

    if(utilCounterGet(eCOUNTER_TYPE_IRDECODE) == 0)
    {
        if(m_IrKeyHold)
        {
            sKeyStatus.wKeyCode = m_LastIrKeyCode;
            sKeyStatus.eKeyEvent = eKEY_EVENT_RELEASE;
            sKeyStatus.eKeyType = eKEYINPUT_IR;

            //Process Key
            halInputCtrl_Proc(&sKeyStatus);
        }

        halInputCtrl_IR_Restart();
    }

    switch(m_eIRState)
    {
        case eIR_STATE_HEAD:
            if(rcSUCCESS == halInputCtrl_IR_HeadGet())
            {
                m_eIRState = eIR_STATE_DATA;
                m_cIRDataCount = 0;
                utilCounterSet(eCOUNTER_TYPE_IRDECODE, 200);
                halInputCtrl_IR_Flush();

                LOG_MSG(db_HAL_IR, "\r\nIR Head\r\n");

            }
            break;

        case eIR_STATE_DATA:
            if(rcBUSY != halInputCtrl_IR_DataGet())
            {
                m_eIRState = eIR_STATE_HEAD;
            }

            break;

        default:
            ASSERT_ALWAYS();
            m_eIRState = eIR_STATE_HEAD;
            break;
    }
}
#endif /* 0 */

// ==============================================================================
// FUNCTION NAME: HALINPUTCTRL_IR_FLUSH
// DESCRIPTION:
//
//
// Params:
//
// Returns:
//
//
// modification history
// --------------------
// 23/04/2013, Leohong Create
// --------------------
// ==============================================================================
void halInputCtrl_IR_Flush(void)
{
    m_acIRdata[0] = m_acIRdata[1] = m_acIRdata[2] = m_acIRdata[3] = 0;
}

// ==============================================================================
// FUNCTION NAME: HALINPUTCTRL_IR_RESTART
// DESCRIPTION:
//
//
// Params:
//
// Returns:
//
//
// modification history
// --------------------
// 23/04/2013, Leohong Create
// --------------------
// ==============================================================================
void halInputCtrl_IR_Restart(void)
{
    halInputCtrl_IR_Flush();
    m_cIRDataCount = 0;
    m_wIrRepeatCount = 0;
    m_eIRState = eIR_STATE_HEAD;
    m_IrKeyFound = FALSE;
    m_IrKeyHold = FALSE;
}

#ifdef PLATFORM_B35LH //B35LH_Casper_0008
// ==============================================================================
// FUNCTION NAME: halInputCtrl_IR_CustomCode_Set
// DESCRIPTION:
//
//
// Params:
// UINT8 cCustomId:
//
// Returns:
//
//
// modification history
// --------------------
// 23/04/2013, Leohong Create
// //Fixed B70_Larry_012
// --------------------
// ==============================================================================
void halInputCtrl_IR_CustomCode_Set(UINT16 wCustomCode)
{
    m_wCustomCode = wCustomCode;
}

// ==============================================================================
// FUNCTION NAME: halInputCtrl_IR_DefaultCustomCode_Set
// DESCRIPTION:
//
//
// Params:
// UINT16 wDefaultCustomCode:
//
// Returns:
//
//
// Modification History
// --------------------
// 2015/07/09, Larry Create
// --------------------
// ==============================================================================
void halInputCtrl_IR_DefaultCustomCode_Set(UINT16 wDefaultCustomCode)
{
    m_wDefaultCustomCode = wDefaultCustomCode;
}

// ==============================================================================
// FUNCTION NAME: halInputCtrl_IR_PowerKeyCode_Set
// DESCRIPTION:
//
//
// Params:
// UINT16 wPowerKeyCode:
//
// Returns:
//
//
// Modification History
// --------------------
// 2015/07/09, Larry Create
// --------------------
// ==============================================================================
void halInputCtrl_IR_PowerKeyCode_Set(UINT16 wPowerKeyCode)
{
    m_wPowerKeyCode = wPowerKeyCode;
}

#else
// ==============================================================================
// FUNCTION NAME: halInputCtrl_IR_CustomId_Set
// DESCRIPTION:
//
//
// Params:
// UINT8 cCustomId:
//
// Returns:
//
//
// modification history
// --------------------
// 23/04/2013, Leohong Create
// --------------------
// ==============================================================================
void halInputCtrl_IR_CustomId_Set(UINT16 wCustomId)
{
    m_wCustomId = wCustomId;
}
#endif
// ==============================================================================
// FUNCTION NAME: HALINPUTCTRL_IR_PULSEINSERT
// DESCRIPTION:
//
//
// Params:
// void *pvBuffer:
//
// Returns:
//
//
// modification history
// --------------------
// 24/12/2013, Leo Create
// --------------------
// ==============================================================================
void halInputCtrl_IR_PulseInsert(void *pvBuffer)
{
    UINT32 dwPulseWidth = 0;
    UINT32 *pdwPulseCount = (UINT32 *)pvBuffer;

    dwPulseWidth = *pdwPulseCount - m_dwLastwCaptureData;

    if(rcERROR == (eRESULT)utilQueueWrite(&m_asIR_Ring_Buffer, (UINT8*)&dwPulseWidth))
    {
        //ASSERT_ALWAYS();
    }

    m_dwLastwCaptureData = *pdwPulseCount;
}

// ==============================================================================
// FUNCTION NAME: HALINPUTCTRL_REGISTER_CALLBACK
// DESCRIPTION:
//
//
// Params:
// void (*fpCallback)(UINT8*):
//
// Returns:
//
//
// modification history
// --------------------
// 24/12/2013, Leo Create
// --------------------
// ==============================================================================
void halInputCtrl_Register_Callback(eRESULT(*fpCallback)(void *))
{
    m_fpInputCallback = fpCallback;
}

//ADC
#if 1
// ==============================================================================
// FUNCTION NAME: halInputCtrl_Adc_Buffer_Update
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
// 2015/03/23, Leo Create
// --------------------
// ==============================================================================
void halInputCtrl_Adc_Buffer_Update(void)
{
    UINT16 wAdcValue = 0;

    //if(rcSUCCESS == Board_ADC_Value_Read(eBOARD_ADC_0, m_asAdcCh_Cfg[eKEYPAD_ADC_CH0].eAdcChn, &wAdcValue))
    //{
        //m_asKeyAdcFtBuffer[eKEYPAD_ADC_CH0].awAdcFilterBuffer[m_asKeyAdcFtBuffer[eKEYPAD_ADC_CH0].cIndex++] = wAdcValue;
        //m_asKeyAdcFtBuffer[eKEYPAD_ADC_CH0].cIndex %= ADC_KEY_BUF_SIZE;
    //}

//    LOG_MSG(db_HAL_IR, "ADC - CH0 - %d" , wAdcValue);

    wAdcValue = 0;

    //if(rcSUCCESS == Board_ADC_Value_Read(eBOARD_ADC_0, m_asAdcCh_Cfg[eKEYPAD_ADC_CH1].eAdcChn, &wAdcValue))
    //{
        //m_asKeyAdcFtBuffer[eKEYPAD_ADC_CH1].awAdcFilterBuffer[m_asKeyAdcFtBuffer[eKEYPAD_ADC_CH1].cIndex++] = wAdcValue;
        //m_asKeyAdcFtBuffer[eKEYPAD_ADC_CH1].cIndex %= ADC_KEY_BUF_SIZE;
    //}

//    LOG_MSG(db_HAL_IR, "   CH1 - %d\r\n" , wAdcValue);
}

// ==============================================================================
// FUNCTION NAME: halInputCtrl_Adc_Init
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
// 2015/03/23, Leo Create
// --------------------
// ==============================================================================
void halInputCtrl_Adc_Init(void)
{
    // 清空 Adc Filter Buffer
    memset((UINT8 *)&m_asKeyAdcFtBuffer, 0, (sizeof(sADC_KEY_BUF))*eKEYPAD_ADC_CH_NUMBERS); //G100_Simon_0060
}

// ==============================================================================
// FUNCTION NAME: halInputCtrl_Adc_Get
// DESCRIPTION:
//
//
// Params:
// eKEYPAD_ADC_CHN eAdcChn:
//
// Returns:
//
//
// Modification History
// --------------------
// 2015/03/23, Leo Create
// --------------------
// ==============================================================================
UINT16 halInputCtrl_Adc_Get(eKEYPAD_ADC_CHN eAdcChn)
{
    UINT16 awBuffer[ADC_KEY_BUF_SIZE];
    UINT8 cCount = 0;

    for(cCount = 0; cCount < ADC_KEY_BUF_SIZE; cCount++)
    {
        awBuffer[cCount] = m_asKeyAdcFtBuffer[eAdcChn].awAdcFilterBuffer[cCount];
    }

    // qsort(awBuffer, ADC_FT_BUF_SIZE, sizeof(WORD), WordCompare);
    utilQSort(awBuffer, ADC_KEY_BUF_SIZE, sizeof(UINT16));

    // 取中間值
    return awBuffer[((ADC_KEY_BUF_SIZE >> 1) + 1)];
}

// ==============================================================================
// FUNCTION NAME: halInputCtrl_Debug_Enable
// DESCRIPTION:
//
//
// Params:
// BYTE cEnable:
//
// Returns:
//
//
// Modification History
// --------------------
// 2015/03/24, Leo Create
// --------------------
// ==============================================================================
void halInputCtrl_Debug_Enable(UINT8 cEnable)
{
    if(TRUE == cEnable)
    {
        m_cEnableDebug = TRUE;
    }
    else
    {
        m_cEnableDebug = FALSE;
    }
}

// ==============================================================================
// FUNCTION NAME: halInputCtrl_Debug_Enable_Get
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
// 2015/03/24, Leo Create
// --------------------
// ==============================================================================
UINT8 halInputCtrl_Debug_Enable_Get(void)
{
    return m_cEnableDebug;
}
#endif

// ==============================================================================
// FUNCTION NAME: halInputCtrl_IR_Default
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
// 2017/11/03, Larry Create
// --------------------
// ==============================================================================
UINT16 halInputCtrl_IR_DefaultGet(void)
{
    return m_awCustomCode[DEFAULT_ID_NUMBER];
}

// ==============================================================================
// FUNCTION NAME: halInputCtrl_IR_CustomId
// DESCRIPTION:
//
//
// Params:
// UINT8 ucID:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/11/03, Larry Create
// --------------------
// ==============================================================================
UINT16 halInputCtrl_IR_CustomIdGet(UINT8 ucID)
{
    if(CUSTOM_ID_COUNT > ucID)
    {
        return m_awCustomCode[ucID];
    }

    return m_awCustomCode[DEFAULT_ID_NUMBER];
}

#endif

