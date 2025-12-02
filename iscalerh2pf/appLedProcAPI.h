#ifndef APPLEDPROCAPI_H
#define APPLEDPROCAPI_H
// ==============================================================================
// FILE NAME: APPLEDPROCAPI.H
// DESCRIPTION:
//
//
// modification history
// --------------------
// 27/05/2013, Leohong written
// 21/08/2013 Larry fix //A70_Larry_0027
// --------------------
// ==============================================================================


#include "Common.h"
#include "halLEDCtrlAPI.h"

#define LED_POLLING_PERIOD 100

#define eLED_LIST_MENU      ((UINT32)0x01<<1)
#define eLED_LIST_LEFT      ((UINT32)0x01<<2)
#define eLED_LIST_AUTO      ((UINT32)0x01<<4)
#define eLED_LIST_UP        ((UINT32)0x01<<6)
#define eLED_LIST_ENTER     ((UINT32)0x01<<8)
#define eLED_LIST_DOWN      ((UINT32)0x01<<9)
#define eLED_LIST_INPUT     ((UINT32)0x01<<11)
#define eLED_LIST_RIGHT     ((UINT32)0x01<<5)
#define eLED_LIST_BACK      ((UINT32)0x01<<15)
#define eLED_LIST_LENS      ((UINT32)0x01<<0)
#define eLED_LIST_MUTE      ((UINT32)0x01<<16) //Shutter
#define eLED_LIST_ZOOM      ((UINT32)0x01<<20)
#define eLED_LIST_FOCUS     ((UINT32)0x01<<22)
#define eLED_LIST_HELP      ((UINT32)0x01<<25)
#define eLED_LIST_OSD       ((UINT32)0x01<<26)

typedef enum
{
    eLED_EVENT_IDEL,
    eLED_EVENT_STATUS_LED,
    eLED_EVENT_SHUTTER_LED,
    eLED_EVENT_KEYPAD_LED,

    eLED_EVENT_UPDATE,
    eLED_EVENT_BACKLIGHT_ACTIVE,
    eLED_EVENT_BACKLIGHT_TIMER_UPDATE,

    eLED_EVENT_NUMBER,
} eLED_EVENT;

typedef enum
{
    eLED_CUSTOMID_TYPE_BROADCAST,
    eLED_CUSTOMID_TYPE_DIRECT,
    eLED_CUSTOMID_TYPE_NONE,
    eLED_CUSTOMID_TYPE_NUMBERS,
} eLED_CUSTOMID_TYPE;

typedef enum
{
    eLED_SHUTTER_ON,
    eLED_SHUTTER_OFF,
    eLED_SHUTTER_FAIL,
    eLED_SHUTTER_CLOSE,

    eLED_SHUTTER_NUMBERS,
} eLED_SHUTTER;

typedef struct
{
    UINT8  cBacklightStatus;
    UINT8  cTimerCount;
    UINT8  cCustomID; //A70_Larry_0031
} sLED_DATA;

typedef enum
{
    eSTATUS_LED_COLOR_NULL = 0,

    eSTATUS_LED_COLOR_GREEN = 10,
    eSTATUS_LED_COLOR_BLUE,
    eSTATUS_LED_COLOR_YELLOW,
    eSTATUS_LED_COLOR_RED,
    eSTATUS_LED_COLOR_WHITE,

    eSTATUS_LED_COLOR_NUMBER,
}eSTATUS_LED_COLOR; //A35LS_Larry_0085

typedef enum
{
    ePWRKEY_LED_ALWAYSOFF,
    ePWRKEY_LED_ALWAYSON,

    ePWRKEY_LED_NUMBERS,
}ePWRKEY_LED;

UINT8 palLedProc_StatusLedColorGet(void);
void palLedProc_Status_LED_State_Set(eLED_STATUS eStatus);
void palLedProc_BacklightKeypad_Set(eKEYPAD_BACKLIGHT eBackLightLED);
void palLedProc_LED_Behavior_Set(eLED_BEHAVIOR eBehavior);
UINT8 palLedProc_LED_Behavior_Get(void);
void palLedProc_CustomType_Set(eLED_CUSTOMID_TYPE  eCustomType);
void palLedProc_Shutter_LED_Set(eLED_SHUTTER eLedShutter);
UINT8 palLedProc_Shutter_LED_Get(void);
void palLedProc_SingleLed_Set(UINT32 ulSingleKeyPadLed);
void palLedProc_Init(void);
INT16 palLedProc_Poll(UINT16 uiTick);
void palLedProc_EventSet(eLED_EVENT eEvent);


#endif /* APPLEDPROCAPI_H */

