#ifndef HALKEYCODELIST_H
#define HALKEYCODELIST_H
// ==============================================================================
// FILE NAME: HALKEYCODELIST.H
// DESCRIPTION:
//
//
// modification history
// --------------------
// 23/04/2013, Leohong written
// --------------------
// ==============================================================================

#include "Common.h"

#define KEY_PWR     (~((UINT32)0x0001<<0))
#define KEY_UP      (~((UINT32)0x0001<<1))
#define KEY_DOWN    (~((UINT32)0x0001<<2))
#define KEY_LEFT    (~((UINT32)0x0001<<3))
#define KEY_RIGHT   (~((UINT32)0x0001<<4))
#define KEY_ENTER   (~((UINT32)0x0001<<5))
#define KEY_ISP_PWR (~((UINT32)0x0001<<6))

#define KEY_LAN_UPGRADE         (KEY_PWR&KEY_UP)    //A70_Larry_0011 add for LAN Upgrade
#define KEY_PW392_RESET_EEPROM  (KEY_PWR&KEY_ENTER) //for 392 reset EEPROM//A70_Larry_0020
#define KEY_FAN_CAL             (KEY_PWR&KEY_DOWN)  //for 392 reset EEPROM//A70_Larry_0020
#define KEY_SCALAR_NMI          (KEY_PWR&KEY_RIGHT)
#define KEY_UART_TO_NXP         (KEY_ENTER&KEY_RIGHT)
#define KEY_DDP_UPGRADE         (KEY_PWR&KEY_RIGHT&KEY_LEFT) //DDP upgrade //A70LH_Larry_0073


#define KEY_CODE_ISP        (~((UINT32)0x0001<<7))
#define KEY_CODE_LENS       (~((UINT32)0x0001<<8))
#define KEY_CODE_BACK       (~((UINT32)0x0001<<9))
#define KEY_CODE_MENU       (~((UINT32)0x0001<<10))
#define KEY_CODE_HELP       (~((UINT32)0x0001<<11))
#define KEY_CODE_ZOOM       (~((UINT32)0x0001<<12))
#define KEY_CODE_OSD        (~((UINT32)0x0001<<13))
#define KEY_CODE_FOCUS      (~((UINT32)0x0001<<14))
#define KEY_CODE_INPUT      (~((UINT32)0x0001<<15))
#define KEY_CODE_AUTO       (~((UINT32)0x0001<<16))
#define KEY_CODE_MUTE       (~((UINT32)0x0001<<17))

#define KEY_ADC_IDEL        (1024)
#define KEY_ADC_PRE_CHANNLE (5)
#define KEY_ADC_LEVEL       (KEY_ADC_IDEL / KEY_ADC_PRE_CHANNLE)

#define KEY_PHR (eKEY_EVENT_PRESSED | eKEY_EVENT_HOLD | eKEY_EVENT_RELEASE)
#define KEY_PR  (eKEY_EVENT_PRESSED | eKEY_EVENT_RELEASE)
#define KEY_PH  (eKEY_EVENT_PRESSED | eKEY_EVENT_HOLD)

#if 0
typedef enum
{
    //Press Key
    kePOWERON = 0,
    keSTANDBY,
    keMUTE,

    keMENU,
    keUP,
    keDOWN,
    keLEFT,
    keRIGHT,
    keENTER,

    keFOCUSUP,
    keFOCUSDOWN,
    keZOOMIN,
    keZOOMOUT,
    keLENSHRIGHT,
    keLENSHLEFT,
    keLENSVUP,
    keLENSVDOWN,

    keHOTKEY,
    ke0,
    ke1,
    ke2,
    ke3,
    ke4,
    ke5,
    ke6,
    ke7,
    ke8,
    ke9,

    keLENPWR,
    keINFO,
    keISP_PWR,
    keGAMMA,
    keBRIGHT,
    keCONTRAST,
    kePIP,
    kePIP_SIZE,
    kePIP_LAYOUT,
    keTEST,
    keHKEYSTONE_LEFT,
    keHKEYSTONE_RIGHT,
    keVKEYSTONE_UP,
    keVKEYSTONE_DOWN,

    keISP,
    keOSD,
    keFOCUS,
    keINPUT,
    keAUTO,
    keSHUTTER,
    keLENS,
    keBACK,
    keHELP,
    keZOOM,
    kePRESETS,
    keID,       //G100_Wilsonj_0018
    keMODE,     //G100_Wilsonj_0018
    kePATTERN,  //G100_Wilsonj_0018
    keHOTKEY2,  //G100_Wilsonj_0018


    //Hold Key
    keHOLD_HOTKEY  = 200,
    keHOLD_POWERON,
    keHOLD_STANDBY,
    keHOLD_MUTE,

    keHOLD_MENU,
    keHOLD_UP,
    keHOLD_DOWN,
    keHOLD_LEFT,
    keHOLD_RIGHT,
    keHOLD_ENTER,

    keHOLD_HKEYSTONE_LEFT,
    keHOLD_HKEYSTONE_RIGHT,
    keHOLD_VKEYSTONE_UP,
    keHOLD_VKEYSTONE_DOWN,

    keHOLD_FOCUSUP,
    keHOLD_FOCUSDOWN,
    keHOLD_ZOOMIN,
    keHOLD_ZOOMOUT,
    keHOLD_LENSHRIGHT,
    keHOLD_LENSHLEFT,
    keHOLD_LENSVUP,
    keHOLD_LENSVDOWN,
    keHOLD_SWAP,
    keHOLD_LAN_UPGRADE,
    keHOLD_FAN_CAL,
    keHOLD_SCALAR_NMI,
    keHOLD_UART_TO_NXP,
    keHOLD_RESET_POWER_ON,

    keHOLD_0,
    keHOLD_1,
    keHOLD_2,
    keHOLD_3,
    keHOLD_4,
    keHOLD_5,
    keHOLD_6,
    keHOLD_7,
    keHOLD_8,
    keHOLD_9,

    keHOLD_TEST,
    keHOLD_OSD,
    keHOLD_FOCUS,
    keHOLD_INPUT,
    keHOLD_AUTO,
    keHOLD_SHUTTER,
    keHOLD_LENS,
    keHOLD_BACK,
    keHOLD_HELP,
    keHOLD_ZOOM,

	keHOLD_PRESETS,
    keHOLD_ID,
    keHOLD_MODE,
    keHOLD_PATTERN,
    keHOLD_HOTKEY2,

    //Release Key

    keRELEASE_HOTKEY = 400,
    keRELEASE_POWERON,
    keRELEASE_STANDBY,
    keRELEASE_MUTE,

    keRELEASE_MENU,
    keRELEASE_UP,
    keRELEASE_DOWN,
    keRELEASE_LEFT,
    keRELEASE_RIGHT,
    keRELEASE_ENTER,

    keRELEASE_HKEYSTONE_LEFT,
    keRELEASE_HKEYSTONE_RIGHT,
    keRELEASE_VKEYSTONE_UP,
    keRELEASE_VKEYSTONE_DOWN,

    keRELEASE_FOCUSUP,
    keRELEASE_FOCUSDOWN,
    keRELEASE_ZOOMIN,
    keRELEASE_ZOOMOUT,
    keRELEASE_LENSHRIGHT,
    keRELEASE_LENSHLEFT,
    keRELEASE_LENSVUP,
    keRELEASE_LENSVDOWN,
    keRELEASE_LAN_UPGRADE,
    keRELEASE_SCALAR_NMI,
    keRELEASE_UART_TO_NXP,
    keRELEASE_SWAP,

    keRELEASE_0,
    keRELEASE_1,
    keRELEASE_2,
    keRELEASE_3,
    keRELEASE_4,
    keRELEASE_5,
    keRELEASE_6,
    keRELEASE_7,
    keRELEASE_8,
    keRELEASE_9,

    //Release Key
    keRELEASE_TEST,
    keRELEASE_OSD,
    keRELEASE_FOCUS,
    keRELEASE_INPUT,
    keRELEASE_AUTO,
    keRELEASE_SHUTTER,
    keRELEASE_LENS,
    keRELEASE_BACK,
    keRELEASE_HELP,
    keRELEASE_ZOOM,

	keRELEASE_PRESETS,
    keRELEASE_ID,
    keRELEASE_MODE,
    keRELEASE_PATTERN,
    keRELEASE_HOTKEY2,

    //Combine Key
    keSWAP          = 600,
    keLAN_UPGRADE,
    keSCALAR_NMI,
    keUART_TO_NXP,
    keRESET_POWER_ON,
    keDDP_UPGRADE,


    eKEY_LIST_NUMBER,
} eKEY_LIST;

typedef enum
{
    eKEY_EVENT_PRESSED = 0x01 << 0,
    eKEY_EVENT_HOLD    = 0x01 << 1,
    eKEY_EVENT_RELEASE = 0x01 << 2,

    eKEY_EVENT_NUMBER,
} eKEY_EVENT;
#endif /* 0 */

typedef struct
{
    UINT32 wKeyCode;
    UINT8  cConditions;
    UINT16 wPressKey;
    UINT16 wHoldKey;
    UINT16 wReleaseKey;
} sKEYLUT;



typedef struct
{
    UINT32 wKeyCode;
    UINT32 wKeyCmdCode;
    UINT16 wPressKey;
} sKEYCMDLUT; //A35G2_BRC_Casper_0049

#define CUSTOM_ID_COUNT (100)    //G100_Wilsonj_0036
#define DEFAULT_ID_NUMBER (0)

#ifdef PLATFORM_B35LH //B35LH_Casper_0001
static const WORD m_awCustomCode[CUSTOM_ID_COUNT] =
{
    0x7C00,
    0x7C01,
    0x7C02,
    0x7C03,
    0x7C04,
    0x7C05,
    0x7C06,
    0x7C07,
    0x7C08,
    0x7C09,
};
#else
static const UINT16 m_awCustomCode[CUSTOM_ID_COUNT] =
{
    0x20DF,
    0x32CD,
    0x4F52,
    0x0002,
    0x0003,
    0x5678,
    0xC837,
    0xA55A,
    0x6996,
    0x35CA,
};
#endif

//KeyPad code
static const sKEYLUT m_acKeyPadLUT[] =
{
    //key code
    //key Condition
    //Press key
    //Hold key
    //Release key
    //Repeat Filter
    {
        KEY_PWR,
        KEY_PHR,
        kePOWERON, keHOLD_POWERON, keRELEASE_POWERON,
    },
    {
        KEY_UP,
        KEY_PHR,
        keUP, keHOLD_UP, keRELEASE_UP,
    },
    {
        KEY_DOWN,
        KEY_PHR,
        keDOWN, keHOLD_DOWN, keRELEASE_DOWN,
    },
    {
        KEY_LEFT,
        KEY_PHR,
        keLEFT, keHOLD_LEFT, keRELEASE_LEFT,
    },
    {
        KEY_RIGHT,
        KEY_PHR,
        keRIGHT, keHOLD_RIGHT, keRELEASE_RIGHT,
    },
    {
        KEY_ENTER,
        KEY_PHR,
        keENTER, keHOLD_ENTER, keRELEASE_ENTER,
    },
    {
        KEY_CODE_ISP,
        KEY_PHR,
        keISP, keHOLD_TEST, keRELEASE_TEST,
    },
    {
        KEY_CODE_OSD,
        KEY_PHR,
        keOSD, keHOLD_OSD, keRELEASE_OSD,
    },
    {
        KEY_CODE_FOCUS,
        KEY_PHR,
        keFOCUS, keHOLD_FOCUS, keRELEASE_FOCUS,
    },
    {
        KEY_CODE_INPUT,
        KEY_PHR,
        keINPUT, keHOLD_INPUT, keRELEASE_INPUT,
    },
    {
        KEY_CODE_AUTO,
        KEY_PHR,
        keAUTO, keHOLD_AUTO, keRELEASE_AUTO,
    },
    {
        KEY_CODE_MUTE,
        KEY_PHR,
        keMUTE, keHOLD_MUTE, keRELEASE_MUTE,
    },
    {
        KEY_CODE_LENS,
        KEY_PHR,
        keLENS, keHOLD_LENS, keRELEASE_LENS,
    },
    {
        KEY_CODE_BACK,
        KEY_PHR,
        keBACK, keHOLD_BACK, keRELEASE_BACK,
    },
    {
        KEY_CODE_MENU,
        KEY_PHR,
        keMENU, keHOLD_MENU, keRELEASE_MENU,
    },
    {
        KEY_CODE_HELP,
        KEY_PHR,
        keHELP, keHOLD_HELP, keRELEASE_HELP,
    },
    {
        KEY_CODE_ZOOM,
        KEY_PHR,
        keZOOM, keHOLD_ZOOM, keRELEASE_ZOOM,
    },
    {
        KEY_LAN_UPGRADE, //A70_Larry_0011 add for LAN Upgrade
        KEY_PHR,
        keLAN_UPGRADE, keHOLD_LAN_UPGRADE, keRELEASE_LAN_UPGRADE,
    },
    {
        KEY_PW392_RESET_EEPROM, //A70_Larry_0020
        KEY_PHR,
        keRESET_POWER_ON, keHOLD_RESET_POWER_ON, keRELEASE_POWERON,
    },
    {
        KEY_FAN_CAL,
        eKEY_EVENT_HOLD,
        eKEY_LIST_NUMBER, keHOLD_FAN_CAL, eKEY_LIST_NUMBER,
    },
    {
        KEY_SCALAR_NMI,
        KEY_PHR,
        keSCALAR_NMI, keHOLD_SCALAR_NMI, keRELEASE_SCALAR_NMI,
    },
    {
        KEY_UART_TO_NXP,
        KEY_PHR,
        keUART_TO_NXP, keHOLD_UART_TO_NXP, keRELEASE_UART_TO_NXP,
    },
    {
        KEY_ISP_PWR,
        eKEY_EVENT_PRESSED,
        keISP_PWR, eKEY_LIST_NUMBER, eKEY_LIST_NUMBER,
    },
    {
        KEY_DDP_UPGRADE,
        eKEY_EVENT_PRESSED,
        keDDP_UPGRADE, eKEY_LIST_NUMBER, eKEY_LIST_NUMBER,
    }, //A70LH_Larry_0073
};

#define KEYPADNUMBER sizeof(m_acKeyPadLUT)/sizeof(sKEYLUT)

#if defined(CUSTOM_CHRISTIE)
static const sKEYLUT m_acIRKeyLUT[] =
{
    //key code
    //key Condition
    //Press key
    //Hold key
    //Release key
    {0x0039,        KEY_PHR,                kePOWERON,              keHOLD_POWERON,         keRELEASE_POWERON,          },
    {0x003A,        KEY_PHR,                keSTANDBY,              keHOLD_STANDBY,         keRELEASE_STANDBY,          },
    {0x0002,        KEY_PHR,                keMUTE,                 keHOLD_MUTE,            keRELEASE_MUTE,             },
    {0x0013,        KEY_PHR,                keMENU,                 keHOLD_MENU,            keRELEASE_MENU,             },
    {0x0014,        KEY_PHR,                keBACK,                 keHOLD_BACK,            keRELEASE_BACK,             },
    {0x0026,        KEY_PHR,                keUP,                   keHOLD_UP,              keRELEASE_UP,               },
    {0x0027,        KEY_PHR,                keLEFT,                 keHOLD_LEFT,            keRELEASE_LEFT,             },
    {0x0028,        KEY_PHR,                keENTER,                keHOLD_ENTER,           keRELEASE_ENTER,            },
    {0x0029,        KEY_PHR,                keRIGHT,                keHOLD_RIGHT,           keRELEASE_RIGHT,            },
    {0x002A,        KEY_PHR,                keDOWN,                 keHOLD_DOWN,            keRELEASE_DOWN,             },
    {0x0005,        KEY_PHR,                keFOCUSUP,              keHOLD_FOCUSUP,         keRELEASE_FOCUSUP,          },
    {0x0006,        KEY_PHR,                keFOCUSDOWN,            keHOLD_FOCUSDOWN,       keRELEASE_FOCUSDOWN,        },
    {0x000A,        KEY_PHR,                keZOOMIN,               keHOLD_ZOOMIN,          keRELEASE_ZOOMIN,           },
    {0x0009,        KEY_PHR,                keZOOMOUT,              keHOLD_ZOOMOUT,         keRELEASE_ZOOMOUT,          },
    {0x000D,        KEY_PHR,                keLENSHLEFT,            keHOLD_LENSHLEFT,       keRELEASE_LENSHLEFT,        },
    {0x000E,        KEY_PHR,                keLENSHRIGHT,           keHOLD_LENSHRIGHT,      keRELEASE_LENSHRIGHT,       },
    {0x0012,        KEY_PHR,                keLENSVUP,              keHOLD_LENSVUP,         keRELEASE_LENSVUP,          },
    {0x0011,        KEY_PHR,                keLENSVDOWN,            keHOLD_LENSVDOWN,       keRELEASE_LENSVDOWN,        },
    {0x0041,        KEY_PHR,                keHOTKEY,               keHOLD_HOTKEY,          keRELEASE_HOTKEY,           },
    {0x0042,        eKEY_EVENT_PRESSED,     keINFO,                 eKEY_LIST_NUMBER,       eKEY_LIST_NUMBER,           },
    {0x0024,        eKEY_EVENT_PRESSED,     ke0,                    eKEY_LIST_NUMBER,       eKEY_LIST_NUMBER,           },
    {0x001A,        eKEY_EVENT_PRESSED,     ke1,                    eKEY_LIST_NUMBER,       eKEY_LIST_NUMBER,           },
    {0x001B,        KEY_PHR,                ke2,                    keHOLD_2,               keRELEASE_2,                },
    {0x001C,        eKEY_EVENT_PRESSED,     ke3,                    eKEY_LIST_NUMBER,       eKEY_LIST_NUMBER,           },
    {0x001D,        KEY_PHR,                ke4,                    keHOLD_4,               keRELEASE_4,                },
    {0x001E,        eKEY_EVENT_PRESSED,     ke5,                    eKEY_LIST_NUMBER,       eKEY_LIST_NUMBER,           },
    {0x001F,        KEY_PHR,                ke6,                    keHOLD_6,               keRELEASE_6,                },
    {0x0020,        eKEY_EVENT_PRESSED,     ke7,                    eKEY_LIST_NUMBER,       eKEY_LIST_NUMBER,           },
    {0x0021,        KEY_PHR,                ke8,                    keHOLD_8,               keRELEASE_8,                },
    {0x0022,        eKEY_EVENT_PRESSED,     ke9,                    eKEY_LIST_NUMBER,       eKEY_LIST_NUMBER,           },
    {0x002B,        KEY_PHR,                keSWAP,                 keHOLD_SWAP,            keRELEASE_SWAP,             },
    {0x0023,        KEY_PHR,                keHELP,                 keHOLD_HELP,            keRELEASE_HELP,             },
    {0x002F,        KEY_PHR,                keAUTO,                 eKEY_LIST_NUMBER,       eKEY_LIST_NUMBER,           },
    {0x0030,        KEY_PHR,                keINPUT,                eKEY_LIST_NUMBER,       eKEY_LIST_NUMBER,           },
    {0x0031,        KEY_PHR,                keOSD,                  keHOLD_OSD,             keRELEASE_OSD,              },
    {0x0017,        KEY_PHR,                keGAMMA,                keHOLD_GAMMA,           keRELEASE_GAMMA,            },
    {0x0019,        KEY_PHR,                keBRIGHT,               keHOLD_BRIGHT,          keRELEASE_BRIGHT,           },
    {0x0018,        KEY_PHR,                keCONTRAST,             keHOLD_CONTRAST,        keRELEASE_CONTRAST,         },
    {0x000F,        KEY_PHR,                kePIP,                  keHOLD_PIP,             keRELEASE_PIP,              },
    {0x0043,        KEY_PHR,                kePIP_SIZE,             keHOLD_PIP_SIZE,        keRELEASE_PIP_SIZE,         },
    {0x0044,        KEY_PHR,                kePIP_LAYOUT,           keHOLD_PIP_LAYOUT,      keRELEASE_PIP_LAYOUT,       },
    {0x0001,        KEY_PHR,                keTEST,                 keHOLD_TEST,            keRELEASE_TEST,             },
    {0x0045,        KEY_PHR,                keHKEYSTONE_LEFT,       keHOLD_HKEYSTONE_LEFT,  keRELEASE_HKEYSTONE_LEFT,   },
    {0x0046,        KEY_PHR,                keHKEYSTONE_RIGHT,      keHOLD_HKEYSTONE_RIGHT, keRELEASE_HKEYSTONE_RIGHT,  },
    {0x0047,        KEY_PHR,                keVKEYSTONE_UP,         keHOLD_VKEYSTONE_UP,    keRELEASE_VKEYSTONE_UP,     },
    {0x0048,        KEY_PHR,                keVKEYSTONE_DOWN,       keHOLD_VKEYSTONE_DOWN,  keRELEASE_VKEYSTONE_DOWN,   },
};
#elif defined(CUSTOM_BARCO)
static const sKEYLUT m_acIRKeyLUT[] =
{
    //key code
    //key Condition
    //Press key
    //Hold key
    //Release key
    {0x0012,  KEY_PHR,                    kePOWERON,            keHOLD_POWERON,        keRELEASE_POWERON,           },
    {0x0002,  KEY_PHR,                    keSTANDBY,            keHOLD_STANDBY,        keRELEASE_STANDBY,           },
    {0x0031,  KEY_PHR,                    ke1,                  keHOLD_1,              eKEY_LIST_NUMBER,            },
    {0x0032,  KEY_PH,                     ke2,                  keHOLD_2,              keRELEASE_2,        		    },
    {0x0033,  KEY_PHR,                    ke3,                  keHOLD_3,              eKEY_LIST_NUMBER,            },
    {0x0034,  KEY_PH,                     ke4,                  keHOLD_4,              keRELEASE_4,        		    },
    {0x0035,  KEY_PHR,                    ke5,                  eKEY_LIST_NUMBER,      eKEY_LIST_NUMBER,            },
    {0x0036,  KEY_PH,                     ke6,                  keHOLD_6,              keRELEASE_6,        		    },
    {0x0037,  KEY_PHR,                    ke7,                  keHOLD_7,              eKEY_LIST_NUMBER,            },
    {0x0038,  KEY_PH,                     ke8,                  keHOLD_8,              keRELEASE_8,        		    },
    {0x0039,  KEY_PHR,                    ke9,                  keHOLD_9,              eKEY_LIST_NUMBER,            },
    {0x004C,  eKEY_EVENT_PRESSED,         keINFO,               eKEY_LIST_NUMBER,      eKEY_LIST_NUMBER,            },
    {0x0030,  eKEY_EVENT_PRESSED,         ke0,                  eKEY_LIST_NUMBER,      eKEY_LIST_NUMBER,            },
    {0x0023,  eKEY_EVENT_PRESSED,         keMODE,               eKEY_LIST_NUMBER,      eKEY_LIST_NUMBER,            },
    {0x0015,  KEY_PHR,                    keAUTO,               eKEY_LIST_NUMBER,      eKEY_LIST_NUMBER,            },
    {0x0014,  KEY_PHR,                    keINPUT,              eKEY_LIST_NUMBER,      eKEY_LIST_NUMBER,            },
    {0x0040,  KEY_PHR,                    keUP,                 keHOLD_UP,             keRELEASE_UP,       		    },
    {0x0042,  KEY_PHR,                    keLEFT,               keHOLD_LEFT,           keRELEASE_LEFT,     		    },
    {0x0013,  KEY_PHR,                    keENTER,              eKEY_LIST_NUMBER,      eKEY_LIST_NUMBER,            },
    {0x0043,  KEY_PHR,                    keRIGHT,              keHOLD_RIGHT,          keRELEASE_RIGHT,    		    },
    {0x0041,  KEY_PHR,                    keDOWN,               keHOLD_DOWN,           keRELEASE_DOWN,     		    },
    {0x001B,  KEY_PHR,                    keMENU,               keUP,                  eKEY_LIST_NUMBER,            },
    {0x001D,  KEY_PHR,                    keBACK,               eKEY_LIST_NUMBER,      eKEY_LIST_NUMBER,            },
    {0x001C,  KEY_PHR,                    kePATTERN,            eKEY_LIST_NUMBER,      eKEY_LIST_NUMBER,            },
    {0x000B,  KEY_PHR,                    keBRIGHT,             eKEY_LIST_NUMBER,      eKEY_LIST_NUMBER,            },
    {0x000C,  KEY_PHR,                    keCONTRAST,           eKEY_LIST_NUMBER,      eKEY_LIST_NUMBER,            },
    {0x0050,  KEY_PHR,                    keID,                 eKEY_LIST_NUMBER,      eKEY_LIST_NUMBER,            },
    {0x000E,  KEY_PHR,                    keLENSHLEFT,          keHOLD_LENSHLEFT,      keRELEASE_LENSHLEFT, 		},
    {0x000F,  KEY_PHR,                    keLENSHRIGHT,         keHOLD_LENSHRIGHT,     keRELEASE_LENSHRIGHT, 		},
    {0x0016,  KEY_PHR,                    keFOCUSUP,            keHOLD_FOCUSUP,        keRELEASE_FOCUSUP, 			},
    {0x001E,  KEY_PHR,                    keLENSVUP,            keHOLD_LENSVUP,        keRELEASE_LENSVUP,  		    },
    {0x001F,  KEY_PHR,                    keLENSVDOWN,          keHOLD_LENSVDOWN,      keRELEASE_LENSVDOWN, 		},
    {0x0017,  KEY_PHR,                    keFOCUSDOWN,          keHOLD_FOCUSDOWN,      keRELEASE_FOCUSDOWN, 		},
    {0x002C,  KEY_PHR,                    keHKEYSTONE_LEFT,     keHOLD_HKEYSTONE_LEFT, keRELEASE_HKEYSTONE_LEFT,    },
    {0x002D,  KEY_PHR,                    keHKEYSTONE_RIGHT,    keHOLD_HKEYSTONE_RIGHT,keRELEASE_HKEYSTONE_RIGHT,   },
    {0x002E,  KEY_PHR,                    keVKEYSTONE_UP,       keHOLD_VKEYSTONE_UP,   keRELEASE_VKEYSTONE_UP,      },
    {0x002F,  KEY_PHR,                    keVKEYSTONE_DOWN,     keHOLD_VKEYSTONE_DOWN, keRELEASE_VKEYSTONE_DOWN,    },
    {0x0011,  KEY_PHR,                    keMUTE,               eKEY_LIST_NUMBER,      eKEY_LIST_NUMBER,  		    },
    {0x001A,  KEY_PHR,                    keHOTKEY,             keHOLD_HOTKEY,         keRELEASE_HOTKEY,            },
    {0x0052,  KEY_PHR,                    keHOTKEY2,            eKEY_LIST_NUMBER,      eKEY_LIST_NUMBER,            },
    {0x0018,  KEY_PHR,                    keZOOMIN,             keHOLD_ZOOMIN,         keRELEASE_ZOOMIN,            },  //G100_Owen_0025 //G100_Owen_0019
    {0x0019,  KEY_PHR,                    keZOOMOUT,            keHOLD_ZOOMOUT,        keRELEASE_ZOOMOUT,           },  //G100_Owen_0025 //G100_Owen_0019
    {0x000A,  KEY_PHR,                    keGAMMA,              eKEY_LIST_NUMBER,      eKEY_LIST_NUMBER,            },  //G100_Tim_0001, add
    {0x000D,  KEY_PHR,                    kePIP,                eKEY_LIST_NUMBER,      eKEY_LIST_NUMBER,            },  //G100_Tim_0001, add
};
#elif defined(CUSTOM_OPTOMA)
static const sKEYLUT m_acIRKeyLUT[] =
{
    //key code
    //key Condition
    //Press key
    //Hold key
    //Release key
    {0x0002,  KEY_PHR,                    kePOWERON,		      keHOLD_POWERON,             keRELEASE_POWERON,                },
    {0x002E,  eKEY_EVENT_PRESSED,         keSTANDBY,     		  keHOLD_STANDBY,             keRELEASE_STANDBY,    		    }, //T100IR_Casper_0017
    {0x0072,  KEY_PHR,                    ke1,                    keHOLD_1,                   eKEY_LIST_NUMBER,                 },
    {0x0073,  KEY_PH,                     ke2,                    keHOLD_2,                   keRELEASE_2,        		        },
    {0x0074,  KEY_PHR,                    ke3,                    keHOLD_3,                   eKEY_LIST_NUMBER,             	},
    {0x0075,  KEY_PH,                     ke4,                    keHOLD_4,                   keRELEASE_4,        		        },
    {0x0077,  KEY_PHR,                    ke5,                    eKEY_LIST_NUMBER,           eKEY_LIST_NUMBER,             	},
    {0x0078,  KEY_PH,                     ke6,                    keHOLD_6,                   keRELEASE_6,        		        },
    {0x0079,  KEY_PHR,                    ke7,                    keHOLD_7,                   eKEY_LIST_NUMBER,             	},
    {0x0080,  KEY_PH,                     ke8,                    keHOLD_8,                   keRELEASE_8,        		        },
    {0x0081,  KEY_PHR,                    ke9,                    keHOLD_9,                   eKEY_LIST_NUMBER,             	},
    {0x0082,  eKEY_EVENT_PRESSED,         keINFO,                 eKEY_LIST_NUMBER,           eKEY_LIST_NUMBER,                 },
    {0x0025,  eKEY_EVENT_PRESSED,         ke0,                    eKEY_LIST_NUMBER,           eKEY_LIST_NUMBER,             	},
    {0x0005,  eKEY_EVENT_PRESSED,         keMODE,               eKEY_LIST_NUMBER,      eKEY_LIST_NUMBER,            },
    {0x0004,  KEY_PHR,                    keAUTO,                 eKEY_LIST_NUMBER,           eKEY_LIST_NUMBER,             	},
    {0x0018,  KEY_PHR,                    keINPUT,                eKEY_LIST_NUMBER,           eKEY_LIST_NUMBER,             	},
    {0x000F,  KEY_PHR,                    keUP,                   keHOLD_UP,                  keRELEASE_UP,       		        },
    {0x0011,  KEY_PHR,                    keLEFT,                 keHOLD_LEFT,                keRELEASE_LEFT,     		        },
    {0x0014,  KEY_PHR,                    keENTER,                eKEY_LIST_NUMBER,           eKEY_LIST_NUMBER,             	},
    {0x0010,  KEY_PHR,                    keRIGHT,                keHOLD_RIGHT,               keRELEASE_RIGHT,    		        },
    {0x0012,  KEY_PHR,                    keDOWN,                 keHOLD_DOWN,                keRELEASE_DOWN,     		        },
    {0x000E,  KEY_PHR,                    keMENU,                 keUP,                       eKEY_LIST_NUMBER,             	},
    {0x002A,  KEY_PHR,                    keBACK,                 eKEY_LIST_NUMBER,           eKEY_LIST_NUMBER,             	},
    {0x002B,  KEY_PHR,                    keGAMMA,                eKEY_LIST_NUMBER,           eKEY_LIST_NUMBER,             	},
    {0x0028,  KEY_PHR,                    keBRIGHT,               eKEY_LIST_NUMBER,           eKEY_LIST_NUMBER,             	},
    {0x0029,  KEY_PHR,                    keCONTRAST,             eKEY_LIST_NUMBER,           eKEY_LIST_NUMBER,             	},
    {0x0043,  KEY_PHR,                    kePIP,                  eKEY_LIST_NUMBER,           eKEY_LIST_NUMBER,                 },
    {0x0041,  KEY_PHR,                    keLENSHLEFT,            keHOLD_LENSHLEFT,           keRELEASE_LENSHLEFT, 		        },
    {0x0042,  KEY_PHR,                    keLENSHRIGHT,           keHOLD_LENSHRIGHT,          keRELEASE_LENSHRIGHT, 		    },
    {0x0086,  KEY_PHR,                    keFOCUSUP,              keHOLD_FOCUSUP,             keRELEASE_FOCUSUP, 				},
    {0x0034,  KEY_PHR,                    keLENSVUP,              keHOLD_LENSVUP,             keRELEASE_LENSVUP,  		        },
    {0x0032,  KEY_PHR,                    keLENSVDOWN,            keHOLD_LENSVDOWN,           keRELEASE_LENSVDOWN, 		        },
    {0x0026,  KEY_PHR,                    keFOCUSDOWN,            keHOLD_FOCUSDOWN,           keRELEASE_FOCUSDOWN, 				},
    {0x0087,  KEY_PHR,                    keHKEYSTONE_LEFT,       keHOLD_HKEYSTONE_LEFT,      keRELEASE_HKEYSTONE_LEFT,         },
    {0x0051,  KEY_PHR,                    keHKEYSTONE_RIGHT,      keHOLD_HKEYSTONE_RIGHT,     keRELEASE_HKEYSTONE_RIGHT,        },
    {0x0052,  KEY_PHR,                    keZOOMIN,               keHOLD_ZOOMIN,              keRELEASE_ZOOMIN,             	},
    {0x0053,  KEY_PHR,                    keVKEYSTONE_UP,         keHOLD_VKEYSTONE_UP,        keRELEASE_VKEYSTONE_UP,           },
    {0x0054,  KEY_PHR,                    keVKEYSTONE_DOWN,       keHOLD_VKEYSTONE_DOWN,      keRELEASE_VKEYSTONE_DOWN,         },
    {0x0055,  KEY_PHR,                    keZOOMOUT,              keHOLD_ZOOMOUT,             keRELEASE_ZOOMOUT,                },
    {0x0056,  KEY_PHR,                    keMUTE,                 eKEY_LIST_NUMBER,           eKEY_LIST_NUMBER,  		        },
    {0x0057,  KEY_PHR,                    keHOTKEY,               keHOLD_HOTKEY,              keRELEASE_HOTKEY,             	},
    {0x0058,  KEY_PHR,                    kePATTERN,              eKEY_LIST_NUMBER,           eKEY_LIST_NUMBER,             	},
};
#else
static const sKEYLUT m_acIRKeyLUT[] = //christie
{
    //key code
    //key Condition
    //Press key
    //Hold key
    //Release key
    {0x0039,        KEY_PHR,                kePOWERON,              keHOLD_POWERON,         keRELEASE_POWERON,          },
    {0x003A,        KEY_PHR,                keSTANDBY,              keHOLD_STANDBY,         keRELEASE_STANDBY,          },
    {0x0002,        KEY_PHR,                keMUTE,                 keHOLD_MUTE,            keRELEASE_MUTE,             },
    {0x0013,        KEY_PHR,                keMENU,                 keHOLD_MENU,            keRELEASE_MENU,             },
    {0x0014,        KEY_PHR,                keBACK,                 keHOLD_BACK,            keRELEASE_BACK,             },
    {0x0026,        KEY_PHR,                keUP,                   keHOLD_UP,              keRELEASE_UP,               },
    {0x0027,        KEY_PHR,                keLEFT,                 keHOLD_LEFT,            keRELEASE_LEFT,             },
    {0x0028,        KEY_PHR,                keENTER,                keHOLD_ENTER,           keRELEASE_ENTER,            },
    {0x0029,        KEY_PHR,                keRIGHT,                keHOLD_RIGHT,           keRELEASE_RIGHT,            },
    {0x002A,        KEY_PHR,                keDOWN,                 keHOLD_DOWN,            keRELEASE_DOWN,             },
    {0x0005,        KEY_PHR,                keFOCUSUP,              keHOLD_FOCUSUP,         keRELEASE_FOCUSUP,          },
    {0x0006,        KEY_PHR,                keFOCUSDOWN,            keHOLD_FOCUSDOWN,       keRELEASE_FOCUSDOWN,        },
    {0x000A,        KEY_PHR,                keZOOMIN,               keHOLD_ZOOMIN,          keRELEASE_ZOOMIN,           },
    {0x0009,        KEY_PHR,                keZOOMOUT,              keHOLD_ZOOMOUT,         keRELEASE_ZOOMOUT,          },
    {0x000D,        KEY_PHR,                keLENSHLEFT,            keHOLD_LENSHLEFT,       keRELEASE_LENSHLEFT,        },
    {0x000E,        KEY_PHR,                keLENSHRIGHT,           keHOLD_LENSHRIGHT,      keRELEASE_LENSHRIGHT,       },
    {0x0012,        KEY_PHR,                keLENSVUP,              keHOLD_LENSVUP,         keRELEASE_LENSVUP,          },
    {0x0011,        KEY_PHR,                keLENSVDOWN,            keHOLD_LENSVDOWN,       keRELEASE_LENSVDOWN,        },
    {0x0041,        KEY_PHR,                keHOTKEY,               keHOLD_HOTKEY,          keRELEASE_HOTKEY,           },
    {0x0042,        eKEY_EVENT_PRESSED,     keINFO,                 eKEY_LIST_NUMBER,       eKEY_LIST_NUMBER,           },
    {0x0024,        eKEY_EVENT_PRESSED,     ke0,                    eKEY_LIST_NUMBER,       eKEY_LIST_NUMBER,           },
    {0x001A,        eKEY_EVENT_PRESSED,     ke1,                    eKEY_LIST_NUMBER,       eKEY_LIST_NUMBER,           },
    {0x001B,        KEY_PHR,                ke2,                    keHOLD_2,               keRELEASE_2,                },
    {0x001C,        eKEY_EVENT_PRESSED,     ke3,                    eKEY_LIST_NUMBER,       eKEY_LIST_NUMBER,           },
    {0x001D,        KEY_PHR,                ke4,                    keHOLD_4,               keRELEASE_4,                },
    {0x001E,        eKEY_EVENT_PRESSED,     ke5,                    eKEY_LIST_NUMBER,       eKEY_LIST_NUMBER,           },
    {0x001F,        KEY_PHR,                ke6,                    keHOLD_6,               keRELEASE_6,                },
    {0x0020,        eKEY_EVENT_PRESSED,     ke7,                    eKEY_LIST_NUMBER,       eKEY_LIST_NUMBER,           },
    {0x0021,        KEY_PHR,                ke8,                    keHOLD_8,               keRELEASE_8,                },
    {0x0022,        eKEY_EVENT_PRESSED,     ke9,                    eKEY_LIST_NUMBER,       eKEY_LIST_NUMBER,           },
    {0x002B,        KEY_PHR,                keSWAP,                 keHOLD_SWAP,            keRELEASE_SWAP,             },
    {0x0023,        KEY_PHR,                keHELP,                 keHOLD_HELP,            keRELEASE_HELP,             },
    {0x002F,        KEY_PHR,                keAUTO,                 eKEY_LIST_NUMBER,       eKEY_LIST_NUMBER,           },
    {0x0030,        KEY_PHR,                keINPUT,                eKEY_LIST_NUMBER,       eKEY_LIST_NUMBER,           },
    {0x0031,        KEY_PHR,                keOSD,                  keHOLD_OSD,             keRELEASE_OSD,              },
    {0x0017,        KEY_PHR,                keGAMMA,                keHOLD_GAMMA,           keRELEASE_GAMMA,            },
    {0x0019,        KEY_PHR,                keBRIGHT,               keHOLD_BRIGHT,          keRELEASE_BRIGHT,           },
    {0x0018,        KEY_PHR,                keCONTRAST,             keHOLD_CONTRAST,        keRELEASE_CONTRAST,         },
    {0x000F,        KEY_PHR,                kePIP,                  keHOLD_PIP,             keRELEASE_PIP,              },
    {0x0043,        KEY_PHR,                kePIP_SIZE,             keHOLD_PIP_SIZE,        keRELEASE_PIP_SIZE,         },
    {0x0044,        KEY_PHR,                kePIP_LAYOUT,           keHOLD_PIP_LAYOUT,      keRELEASE_PIP_LAYOUT,       },
    {0x0001,        KEY_PHR,                keTEST,                 keHOLD_TEST,            keRELEASE_TEST,             },
    {0x0045,        KEY_PHR,                keHKEYSTONE_LEFT,       keHOLD_HKEYSTONE_LEFT,  keRELEASE_HKEYSTONE_LEFT,   },
    {0x0046,        KEY_PHR,                keHKEYSTONE_RIGHT,      keHOLD_HKEYSTONE_RIGHT, keRELEASE_HKEYSTONE_RIGHT,  },
    {0x0047,        KEY_PHR,                keVKEYSTONE_UP,         keHOLD_VKEYSTONE_UP,    keRELEASE_VKEYSTONE_UP,     },
    {0x0048,        KEY_PHR,                keVKEYSTONE_DOWN,       keHOLD_VKEYSTONE_DOWN,  keRELEASE_VKEYSTONE_DOWN,   },
};
#endif

//#if defined(CUSTOM_BARCO) //A35G2_BRC_Casper_0049
static const sKEYCMDLUT m_acIRCmdKeyLUT[] =
{
    //key code
    //key command
    //Press Key
    {0x0012,  0x0012,  kePOWERON},
    {0x0002,  0x0032,  keSTANDBY},
    {0x0031,  0x0033,  ke1},
    {0x0032,  0x0034,  ke2},
    {0x0033,  0x0035,  ke3},
    {0x0034,  0x0036,  ke4},
    {0x0035,  0x0037,  ke5},
    {0x0036,  0x0038,  ke6},
    {0x0037,  0x0039,  ke7},
    {0x0038,  0x003A,  ke8},
    {0x0039,  0x003B,  ke9},
    {0x004C,  0x0028,  keINFO},
    {0x0030,  0x003C,  ke0},
    {0x0023,  0x0024,  keMODE},
    {0x0015,  0x0029,  keAUTO},
    {0x0014,  0x002E,  keINPUT},
    {0x0040,  0x000A,  keUP},
    {0x0042,  0x000B,  keLEFT},
    {0x0013,  0x000C,  keENTER},
    {0x0043,  0x000D,  keRIGHT},
    {0x0041,  0x000E,  keDOWN},
    {0x001B,  0x0014,  keMENU},
    {0x001D,  0x0048,  keBACK},
    {0x001C,  0x0047,  kePATTERN},
    {0x000B,  0x0013,  keBRIGHT},
    {0x000C,  0x003E,  keCONTRAST},
    {0x0050,  0x0050,  keID},
    {0x000E,  0x0040,  keLENSHLEFT},
    {0x000F,  0x0041,  keLENSHRIGHT},
    {0x0016,  0x0022,  keFOCUSUP},
    {0x001E,  0x0042,  keLENSVUP},
    {0x001F,  0x0043,  keLENSVDOWN},
    {0x0017,  0x0023,  keFOCUSDOWN},
    {0x002C,  0x0044,  keHKEYSTONE_LEFT},
    {0x002D,  0x0045,  keHKEYSTONE_RIGHT},
    {0x002E,  0x000F,  keVKEYSTONE_UP},
    {0x002F,  0x0010,  keVKEYSTONE_DOWN},
    {0x0011,  0x0018,  keMUTE},
    {0x001A,  0x0046,  keHOTKEY},
    {0x0052,  0x0052,  keHOTKEY2},
    {0x0018,  0x0020,  keZOOMIN},
    {0x0019,  0x0021,  keZOOMOUT},
    {0x000A,  0x003D,  keGAMMA},
    {0x000D,  0x003F,  kePIP},
};
//#endif

#define IRKEYPADNUMBER sizeof(m_acIRKeyLUT)/sizeof(sKEYLUT)

#endif // HALKEYCODELIST_H

