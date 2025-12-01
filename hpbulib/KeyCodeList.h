#ifndef _KEYCODELIST_H
#define _KEYCODELIST_H

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

    keFOCUSUP,  //9
    keFOCUSDOWN,
    keZOOMIN,
    keZOOMOUT,
    keLENSHRIGHT,
    keLENSHLEFT,
    keLENSVUP,
    keLENSVDOWN,

    keHOTKEY,  //17
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

    keLENPWR,  //28
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

    keISP,  //42
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
    keSWAP,

    keAUDIO_MUTE, //58 //R70G2_AC_0014
    keVOLUME_UP,   //R70G2_AC_0014
    keVOLUME_DOWN, //R70G2_AC_0014
    keGEOMETRIC,   //R70G2_AC_0014
    keBLEND,       //R70G2_AC_0014
    keSCALE ,      //R70G2_AC_0014
    keALL,         //R70G2_AC_0014
    keHDMI1,    // R70K_Keven_0001
    keHDMI2,    // R70K_Keven_0001
    keHDBaseT,   // R70K_Keven_0001
    kePROJECTION,  // R70K_Keven_0001
    ke4K,     // R70K_Keven_0001
    keKEYSTONE,    // R70K_Keven_0001
    keCORNERFIT,   // R70K_Keven_0001
    keWARPINGBLENDING,  // R70K_Keven_0001
    keUSERSET, // R70K_Keven_0001
    keFOCUS1,  // R70K_Keven_0001
    keFOCUS2,  // R70K_Keven_0001
    keLENSLOCK, // R70K_Keven_0001
    keLAN, // R70K_Keven_0001
    keDEFAULT, // R70K_Keven_0001

    keBACK_FOCUSUP, //79
    keBACK_FOCUSDOWN,

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

    keHOLD_HKEYSTONE_LEFT, //210
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

    keHOLD_0, //228
    keHOLD_1,
    keHOLD_2,
    keHOLD_3,
    keHOLD_4,
    keHOLD_5,
    keHOLD_6,
    keHOLD_7,
    keHOLD_8,
    keHOLD_9,

    keHOLD_TEST, //238
    keHOLD_OSD,
    keHOLD_FOCUS,
    keHOLD_INPUT,
    keHOLD_AUTO,
    keHOLD_SHUTTER,
    keHOLD_LENS,
    keHOLD_BACK,
    keHOLD_HELP,
    keHOLD_ZOOM,

	keHOLD_PRESETS, //248
    keHOLD_ID,
    keHOLD_MODE,
    keHOLD_PATTERN,
    keHOLD_HOTKEY2,

    keHOLD_VOLUME_UP,   //R70G2_AC_0014
    keHOLD_VOLUME_DOWN, //R70G2_AC_0014
    keHOLD_GAMMA,
    keHOLD_BRIGHT,
    keHOLD_CONTRAST,
    keHOLD_PIP,
    keHOLD_PIP_SIZE,
    keHOLD_PIP_LAYOUT,
    keHOLD_INFO,
    keHOLD_LENSLOCK,

    keHOLD_BACK_FOCUSUP, //263
    keHOLD_BACK_FOCUSDOWN,

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

    keRELEASE_VOLUME_UP,    //R70G2_AC_0014
    keRELEASE_VOLUME_DOWN,  //R70G2_AC_0014
    keRELEASE_GAMMA,
    keRELEASE_BRIGHT,
    keRELEASE_CONTRAST,
    keRELEASE_PIP,
    keRELEASE_PIP_SIZE,
    keRELEASE_PIP_LAYOUT,
    keRELEASE_INFO,
    keRELEASE_LENSLOCK,
    keRELEASE_BACK_FOCUSUP,
    keRELEASE_BACK_FOCUSDOWN,

    //Combine Key
    keLAN_UPGRADE = 600,
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

#endif // _KEYCODELIST_H

