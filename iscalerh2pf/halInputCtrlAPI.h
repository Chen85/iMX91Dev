#ifndef HALINPUTCTRLAPI_H
#define HALINPUTCTRLAPI_H
// ==============================================================================
// FILE NAME: HALINPUTCTRLAPI.H
// DESCRIPTION:
//
//
// modification history
// --------------------
// 23/04/2013, Leohong Create
// --------------------
// ==============================================================================

#if 0
#include "Common.h"

#define IR_BIT_SIZE             1   // Bit(High/Low)
#define IR_HEADER_SIZE          1       // Start pulse 9mS High 4.5mS Low
#define IR_BYTE_SIZE            8       // Total bits per data byte(High/Low)
#define IR_DATA_SIZE            32      // Total bits not counting header
#define IR_NOISE                1       // Noise

#define IR_PACKAGE_SIZE         (IR_HEADER_SIZE+IR_DATA_SIZE)
#define IR_BUFFER_SIZE          (128)

#define LEADER_MIN              (1282)   // Expecting 13.5ms +- 5%
#define LEADER_MAX              (1417)

#define BIT_1_MIN               (213)    // Expecting 2.25ms +- 5%
#define BIT_1_MAX               (236)

#define BIT_0_MIN               (106)     // Expecting 1.125ms +- 5%
#define BIT_0_MAX               (117)

#define REPEAT_MIN              (1092)    // Expecting 1.15ms +- 5%
#define REPEAT_MAX              (1207)


//Keypad
#define ADC_KEY_BUF_SIZE        (17)






typedef enum
{
    eIR_STATE_HEAD,
    eIR_STATE_DATA,

    eIR_STATE_NUMBER,
} eIR_STATE;

typedef enum
{
    eKEYINPUT_KEYPAD,
    eKEYINPUT_IR,
    eKEYINPUT_LAN,	//G100_Clare_0056
    eKEYINPUT_HDBaseT,
    eKEYINPUT_CLI,
    eKEYINPUT_WIRED, //A70LK_Larry_0098

    eKEYINPUT_NUMBER,
} eKEY_INPUT_TYPE;

#pragma pack(push)  /* push current alignment to stack */
#pragma pack(1)     /* set alignment to 1 byte boundary */

typedef struct
{
    UINT32  wKeyCode;
    UINT8   eKeyEvent;
    UINT8   eKeyType;
} sKEY_DATA;

#pragma pack(pop)   /* restore original alignment from stack */

typedef enum
{
    eKEYPAD_ADC_CH0,
    eKEYPAD_ADC_CH1,

    eKEYPAD_ADC_CH_NUMBERS,
} eKEYPAD_ADC_CHN;

typedef struct
{
    UINT8  cIndex;
    UINT16 awAdcFilterBuffer[ADC_KEY_BUF_SIZE];
} sADC_KEY_BUF;





UINT32 halInputCtrl_KeyPadGet(void);
void halInputCtrl_Proc(sKEY_DATA *psKeyData);
#if defined(CUSTOM_BARCO) //A35G2_BRC_Casper_0049
void halInputCmdCtrl_Proc(sKEY_DATA *psCmdKeyData);
#endif
//void halInputCtrl_KeyPad_Decode(void);

eRESULT halInputCtrl_Init(void);
//void halInputCtrl_IR_Decode(void);
//eRESULT halInputCtrl_IR_HeadGet(void);
//eRESULT halInputCtrl_IR_DataGet(void);
void halInputCtrl_IR_Flush(void);
void halInputCtrl_IR_Restart(void);
#ifdef PLATFORM_B35LH //B35LH_Casper_0008
void halInputCtrl_IR_CustomCode_Set(UINT16 wCustomCode);
void halInputCtrl_IR_DefaultCustomCode_Set(UINT16 wDefaultCustomCode);
void halInputCtrl_IR_PowerKeyCode_Set(UINT16 wPowerKeyCode);
#else
void halInputCtrl_IR_CustomId_Set(UINT16 wCustomId); //A70LV_Larry_0001
#endif
void halInputCtrl_Register_Callback(eRESULT(*fpCallback)(void *));
void halInputCtrl_IR_PulseInsert(void *pvBuffer);
void halInputCtrl_Detect_ISP_Key(UINT8 cEnable);

UINT16 halInputCtrl_Adc_Get(eKEYPAD_ADC_CHN eAdcChn);
void halInputCtrl_Adc_Buffer_Update(void);

UINT16 halInputCtrl_IR_DefaultGet(void);
UINT16 halInputCtrl_IR_CustomIdGet(UINT8 ucID);

#endif

#endif // HALINPUTCTRLAPI_H

