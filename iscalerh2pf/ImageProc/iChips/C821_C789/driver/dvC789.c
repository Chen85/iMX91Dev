// ===============================================================================
// FILE NAME: dvC789.c
// DESCRIPTION:
//
//
// Modification History
// --------------------
// 2017/09/13, Larry Create
// --------------------
// ===============================================================================


#include "dvC789.h"
#include "Board_SPI.h"
#include "utilDbgMsg.h"
#include "utilCounterAPI.h"
#include "opdCtrlAPI.h"
//#include "MemMap.h"

static UINT8 m_ucCurrentBank = 0;
static UINT32 ulV_START;    //A70LV_Doulas_0042

#define BURST_MODE_FIXED_ADDRESS    (0x01 << 6)  //1 Serial burst mode: fixed address
#define BURST_MODE_AUTO_INC_ADDRESS (0x01 << 7)  //2 Serial burst mode: address automatic increment

#define SPI_BUFFER_SIZE             (4 * 1024)
#define CONTROL_789

static UINT8 aucWriteBuffer[SPI_BUFFER_SIZE + 0x100] = {0};
static UINT16 uiWriteIdx = 0;

#ifdef C789_2D_NO_FRAME_LOCK   //A70LV_Doulas_0307
static UINT8  m_ucForcedSyncReset = TRUE;
static UINT16 m_uiH_Total = 2200;
static UINT16 m_uiV_Total = 1125;
#endif
static UINT8  m_ucOutputChanged = FALSE;    //A70LV_Doulas_0307
static UINT8  m_ucFrmaeDelay = 0xff;

#if 0 //A70LV_Larry_0332 mask
static TickType_t ulC789StartTicks;

static void dvC789_TimeInState(void)
{
    ulC789StartTicks = xTaskGetTickCount();
    return;
}

UINT32 dvC789_TimeElapsedInState(void)
{
    TickType_t ulTicks;

    ulTicks = xTaskGetTickCount();

    if(ulC789StartTicks > ulTicks)
    {
        return (0xFFFFFFFF - ulC789StartTicks + 1 + ulTicks) / portTICK_RATE_MS;
    }
    else
    {
        return (ulTicks - ulC789StartTicks) / portTICK_RATE_MS;
    }
}
#endif /* 0 */

static const sPANEL_INFO m_sPanelTable[] =
{
    //PC
    {
        ePANEL_ID_XGA_60HZ,             //Timing Id
        {
            60,                         //FrameRate
            6500, 		                //PixelClock
            296,		                //HStart
            1024,	                    //HActive
            36,		                    //VStart
            768,		                //VActive
            1344, 	                    //HTotal
            806,		                //VTotal
            67,			                //H SyncWidth
            5,                          //V SyncWidth
            //TODO
            4,                          //POREFDIV
            8,                          //POFBDIV
        },
    },
    {
        ePANEL_ID_WXGA_60HZ,            //Timing Id
        {
            60,			                //FrameRate
            8350, 		                //PixelClock
            328,		                //HStart
            1280,	                    //HActive
            29,		                    //VStart
            800,		                //VActive
            1680, 	                    //HTotal
            831,		                //VTotal
            63,		                    //H SyncWidth
            5,                          //V SyncWidth
            //TODO
            4,                          //POREFDIV
            8,                          //POFBDIV
        },
    },
    {
        ePANEL_ID_1600x1200_60HZ,       //Timing Id
        {
            60,			                //FrameRate
            16200, 		                //PixelClock
            496,		                //HStart
            1600,	                    //HActive
            49,		                    //VStart
            1200,	                    //VActive
            2160, 	                    //HTotal
            1250,	                    //VTotal
            95,                         //H SyncWidth
            2,                          //V SyncWidth
            //TODO
            4,                          //POREFDIV
            8,                          //POFBDIV
        },
    },
    {   //A70LV_Doulas_0098 modify
        ePANEL_ID_WUXGA_60HZ,           //Timing Id
        {
            60,			                //FrameRate
            14850,//15412, 		                //PixelClock    154128000
            56,//112,		                //HStart
            1920,	                    //HActive
            20-2,//32,		                    //VStart        //A65_OPTOMA_Doulas_0060 Modify//A70LV_Doulas_0170 TEST
            1200,	                    //VActive
            2004,//2080, 	                    //HTotal
            1235,	                    //VTotal
            15,                         //H SyncWidth
            5,                          //V SyncWidth
            //TODO
            2,                          //POREFDIV
            11,//10,                         //POFBDIV
        },
    },
    //TODO
    {
        ePANEL_ID_WQXGA_60HZ,           //Timing Id
        {
            60,			                //FrameRate
            0,                          //PixelClock
            0,	                        //HStart
            2560,                       //HActive
            0,                          //VStart
            1600,                       //VActive
            0,                          //HTotal
            0,                          //VTotal
            0,                          //H SyncWidth
            0,                          //V SyncWidth
            //TODO
            4,                          //POREFDIV
            8,                          //POFBDIV
        },
    },

    //VIDEO
    //TODO
    {
        ePANEL_ID_720P_50HZ,            //Timing Id
        {
            50,			                //FrameRate
            0, 		                    //PixelClock
            0,		                    //HStart
            1280,	                    //HActive
            0,		                    //VStart
            720,		                //VActive
            0, 	                        //HTotal
            0,		                    //VTotal
            0,			                //H SyncWidth
            0,                          //V SyncWidth
            //TODO
            4,                          //POREFDIV
            8,                          //POFBDIV
        },
    },
    {
        ePANEL_ID_720P_60HZ,            //Timing Id
        {
            60,                         //FrameRate
            9504,                       //PixelClock   7425 //CLK 95040000Hz (POCLK 15840000)
            150,                        //HStart
            1280,                       //HActive
            10,                         //VStart
            720,                        //VActive
#if 0
            1650,                       //HTotal
            750,                        //VTotal
#else
            1650,                       //HTotal
            960,                        //VTotal

#endif
            40,                         //H SyncWidth
            5,                          //V SyncWidth
            //TODO
            1,                          //POREFDIV
            6,                          //POFBDIV
        },
    },
    //TODO
    {
        ePANEL_ID_1080P_50HZ,           //Timing Id
        {
            50,			                //FrameRate
            0,                          //PixelClock
            0,	                        //HStart
            1920,                       //HActive
            0,                          //VStart
            1080,                       //VActive
            0,                          //HTotal
            0,                          //VTotal
            0,                          //H SyncWidth
            0,                          //V SyncWidth
            //TODO
            4,                          //POREFDIV
            8,                          //POFBDIV
        },
    },

    {
        ePANEL_ID_1080P_60HZ,           //Timing Id
        {
            60,                         //FrameRate
            14850,                      //PixelClock
            192,                        //HStart
            1920,                       //HActive
            41-2,                         //VStart		//A65_OPTOMA_Doulas_0060 Modify
            1080,                       //VActive
#if 1
            2200,                       //HTotal
            1125,                       //VTotal
#else
            2220,
            1237,
#endif
            21,                         //H SyncWidth
            4,                          //V SyncWidth
            //TODO
            2,                          //POREFDIV
            11,                         //POFBDIV
        },
    },
    //Done
    {
        ePANEL_ID_3840x2160_50HZ,       //Timing Id
        {
            50,			                //FrameRate
            59400,                      //PixelClock
            384,	                    //HStart
            3840,                       //HActive
            82,                         //VStart
            2160,                       //VActive
            5280,                       //HTotal
            2250,                       //VTotal
            88,                         //H SyncWidth
            10,                         //V SyncWidth
            4,                          //POREFDIV
            8,                          //POFBDIV
        },
    },

    //Done
    {
        ePANEL_ID_3840x2160_60HZ,           //Timing Id
        {
            60,			                //FrameRate
            59400,                      //PixelClock
            384,	                    //HStart
            3840,                       //HActive
            82,                         //VStart
            2160,                       //VActive
            4400,                       //HTotal
            2250,                       //VTotal
            88,                         //H SyncWidth
            10,                         //V SyncWidth
            4,                          //POREFDIV
            8,                          //POFBDIV
        },
    },
    /*Use 166M reference to generate 148.5 pixel clock then 1100 active area need to generate 1229 h total*/
    //1100 * 166/148.5 = 1229.6 = 1230 = 04ceh; 04ceh - 2 = 04cch
    {
        ePANEL_ID_960x2160_60HZ,        //Timing Id
        {
            60,			                //FrameRate
            14850,                      //PixelClock
            64/*44*/,	                //HStart
            960,                        //HActive
            25/*6*/,                    //VStart
            2160,                       //VActive
            1230/*1152*/,               //HTotal
            2248/*2171*/,               //VTotal
            21,                         //H SyncWidth
            4,                          //V SyncWidth
            4,                          //POREFDIV
            8,                          //POFBDIV
        },
    },

    //3D
    {
        ePANEL_ID_XGA_120HZ,            //Timing Id
        {
            120,                        //FrameRate
            11550,                      //PixelClock
            112,                        //HStart
            1024,                       //HActive
            42,                         //VStart
            768,                        //VActive
            1184,                       //HTotal
            813,                        //VTotal
            15,                         //H SyncWidth
            4,                          //V SyncWidth
            //TODO
            4,                          //POREFDIV
            8,                          //POFBDIV
        }
    },
    {
        ePANEL_ID_WXGA_120HZ,           //Timing Id
        {
            120,                        //FrameRate
            1462,                       //PixelClock
            112,                        //HStart
            1280,                       //HActive
            44,                         //VStart
            800,                        //VActive
            1440,                       //HTotal
            847,                        //VTotal
            15,                         //H SyncWidth
            6,                          //V SyncWidth
            //TODO
            4,                          //POREFDIV
            8,                          //POFBDIV
        },
    },
    //TODO
    {
        ePANEL_ID_720P_120HZ,           //Timing Id
        {
            120,                        //FrameRate
            14850,                       //PixelClock
            150,                        //HStart
            1280,                       //HActive
            10,                         //VStart
            720,                        //VActive
            1650,                       //HTotal
            750,                       //VTotal
            40,                         //H SyncWidth
            5,                          //V SyncWidth
            //TODO
            2,//4,                          //POREFDIV  A70LV_Doulas_0094
            11,//8,                         //POFBDIV   A70LV_Doulas_0094
        },
    },

    //A70LV_Doulas_0098 //A70LV_Doulas_0094
    {
        ePANEL_ID_1080P_120HZ,          //Timing Id
        {
            120,                        //FrameRate
            27000,//29700,                      //PixelClock
            48,//192,                        //HStart
            1920,                       //HActive
            41,                         //VStart
            1080,                       //VActive
            2000,//2200,                       //HTotal
            1125,                       //VTotal
            21,                         //H SyncWidth
            4,                          //V SyncWidth
            //TODO
            2,                          //POREFDIV
            20,//22,                         //POFBDIV
        },
    },
    //TODO
    {
        ePANEL_ID_2688x1472_120HZ,      //Timing Id
        {
            120,                        //FrameRate
            0,                          //PixelClock
            0,                          //HStart
            2688,                       //HActive
            0,                          //VStart
            1472,                       //VActive
            0,                          //HTotal
            0,                          //VTotal
            0,                          //H SyncWidth
            0,                          //V SyncWidth
            //TODO
            4,                          //POREFDIV
            8,                          //POFBDIV
        },
    },
    //TODO
    {
        ePANEL_ID_2716x1528_120HZ,      //Timing Id
        {
            120,                        //FrameRate
            0,                          //PixelClock
            0,                          //HStart
            2716,                       //HActive
            0,                          //VStart
            1528,                       //VActive
            0,                          //HTotal
            0,                          //VTotal
            0,                          //H SyncWidth
            0,                          //V SyncWidth
            //TODO
            4,                          //POREFDIV
            8,                          //POFBDIV
        },
    },
    //A70LV_Doulas_0094
    {
        ePANEL_ID_SVGA_120HZ,           //Timing Id
        {
            120,                        //FrameRate
            8395,                       //PixelClock
            60,                        //HStart
            800,                       //HActive
            10,                         //VStart
            600,                        //VActive
            1088,                       //HTotal
            643,                       //VTotal
            40,                         //H SyncWidth
            5,                          //V SyncWidth
            1,                          //POREFDIV
            5,                         //POFBDIV
        },
    },
    //A70LV_Doulas_0094
    {
        ePANEL_ID_WUXGA_120HZ,           //Timing Id
        #if 1     //A70LV_Doulas_0377 Modify
        {
            120,			            //FrameRate
            29700,//30825, 		                //PixelClock    154128000x2
            48,//112,		                //HStart
            1920,	                    //HActive
            20,//32,		                    //VStart
            1200,	                    //VActive
            2004,//2080, 	                    //HTotal
            1235,	                    //VTotal
            15,                         //H SyncWidth
            5,                          //V SyncWidth
            2,                          //POREFDIV
            22,//20,                         //POFBDIV
        },
        #else
        {
            120,			            //FrameRate
            30825, 		                //PixelClock    154128000x2
            112,		                //HStart
            1920,	                    //HActive
            32,		                    //VStart
            1200,	                    //VActive
            2080, 	                    //HTotal
            1235,	                    //VTotal
            15,                         //H SyncWidth
            5,                          //V SyncWidth
            2,                          //POREFDIV
            20,                         //POFBDIV
        },
        #endif
    },
};

static const UINT8 m_ucPanelTblCount = sizeof(m_sPanelTable) / sizeof(sPANEL_INFO);

static sDRV_C789_INFO m_sDrvC789Info;
static UINT8 ucC789_OutputEnable = TRUE;    //A70LV_Doulas_0079
static UINT8 ucColorUniformityEnable = 0;

static char cLastTakeC789Semaphore[128] = {'\0'};  //A35G2_Simon_0112

#if 1
static const sICHIP_DUMP m_sICHIPC789_DUMP[] =		//G100_Doulas_0027
{
//    {"START             \0",	0x12345678},
    {"BN_RGBNK          \0",	BN_RGBNK},
    {"BN_MCT            \0",	BN_MCT},
    {"BN_RTCT           \0",	BN_RTCT},
    {"BN_DDR3CT         \0",	BN_DDR3CT},
    {"BN_MEMCT          \0",	BN_MEMCT},
    {"BN_RSTCT          \0",	BN_RSTCT},
    {"B0_CLKCTRL        \0",	B0_CLKCTRL},
    {"B0_CMOSOUTSETUP   \0",	B0_CMOSOUTSETUP},
    {"B0_POCLKCT        \0",	B0_POCLKCT},
    {"B0_POPLLCT        \0",	B0_POPLLCT},
    {"B0_POREFDIV       \0",	B0_POREFDIV},
    {"B0_POFBDIV        \0",	B0_POFBDIV},
    {"B0_IPPLLCT        \0",	B0_IPPLLCT},
    {"B0_IPREFDIV       \0",	B0_IPREFDIV},
    {"B0_IPFBDIV        \0",	B0_IPFBDIV},
    {"B0_LVDSOCLKCT     \0",	B0_LVDSOCLKCT},
    {"B0_LVDSOSETUP     \0",	B0_LVDSOSETUP},
    {"B0_LVDSO1CT       \0",	B0_LVDSO1CT},
    {"B0_LVDSO2CT       \0",	B0_LVDSO2CT},
    {"B0_LVDSTXCT1      \0",	B0_LVDSTXCT1},
    {"B0_LVDSTXCT2      \0",	B0_LVDSTXCT2},
    {"B0_LVDSI1CT       \0",	B0_LVDSI1CT},
    {"B0_LVDSI2CT       \0",	B0_LVDSI2CT},
    {"B0_LVDSRXCT1      \0",	B0_LVDSRXCT1},
    {"B0_LVDSRXCT2      \0",	B0_LVDSRXCT2},
    {"B0_DDR3CT2        \0",	B0_DDR3CT2},
    {"B0_DDR3CT2_0      \0",	B0_DDR3CT2_0},
    {"B0_DDR3CT2_1      \0",	B0_DDR3CT2_1},
    {"B0_DDR3CT2_2      \0",	B0_DDR3CT2_2},
    {"B0_DDR3CT2_3      \0",	B0_DDR3CT2_3},
    {"B0_DDR3CT2_4      \0",	B0_DDR3CT2_4},
    {"B0_DDR3CT2_5      \0",	B0_DDR3CT2_5},
    {"B0_DDR3OCDDIP     \0",	B0_DDR3OCDDIP},
    {"B0_DDR3OCDDIN     \0",	B0_DDR3OCDDIN},
    {"B0_DDR3LVLCT      \0",	B0_DDR3LVLCT},
    {"B0_DDR3WRLVLPHCT  \0",	B0_DDR3WRLVLPHCT},
    {"B0_DDR3WRTMGCT    \0",	B0_DDR3WRTMGCT},
    {"B0_DDR3RDTMGCT    \0",	B0_DDR3RDTMGCT},
    {"B0_DDR3VREFCT     \0",	B0_DDR3VREFCT},
    {"B0_DDR3DLLCT      \0",	B0_DDR3DLLCT},
    {"B0_DDR3ODTCT      \0",	B0_DDR3ODTCT},
    {"B0_DDR3ZQCT       \0",	B0_DDR3ZQCT},
    {"B0_OCFMT          \0",	B0_OCFMT},
    {"B0_ICFMT          \0",	B0_ICFMT},
    {"B0_ISCWCT         \0",	B0_ISCWCT},
    {"B0_OSDCT          \0",	B0_OSDCT},
    {"B0_BOWINH         \0",	B0_BOWINH},
    {"B0_OSDFILL        \0",	B0_OSDFILL},
    {"B0_CPURAD         \0",	B0_CPURAD},
    {"B0_CPUWAD         \0",	B0_CPUWAD},
    {"B0_CPUDT          \0",	B0_CPUDT},
    {"B0_CPUDTCTL       \0",	B0_CPUDTCTL},
    {"B0_BOSTAT         \0",	B0_BOSTAT},
    {"B0_BBVDLY         \0",	B0_BBVDLY},
    {"B0_BBVSYCT        \0",	B0_BBVSYCT},
    {"B0_BBWMWI         \0",	B0_BBWMWI},
    {"B0_BBRMWI         \0",	B0_BBRMWI},
    {"B0_BOACCT         \0",	B0_BOACCT},
    {"B0_BBACTHW        \0",	B0_BBACTHW},
    {"B0_BBACTVW        \0",	B0_BBACTVW},
    {"B0_CBUFHST        \0",	B0_CBUFHST},
    {"B0_CBUFHW         \0",	B0_CBUFHW},
    {"B0_CBUFVST        \0",	B0_CBUFVST},
    {"B0_CBUFVW         \0",	B0_CBUFVW},
    {"B0_CBUFAD         \0",	B0_CBUFAD},
    {"B0_CBUFDT         \0",	B0_CBUFDT},
    {"B0_CBUFMD         \0",	B0_CBUFMD},
    {"B0_MPROTECT       \0",	B0_MPROTECT},
    {"B0_RNGBUFWCT      \0",	B0_RNGBUFWCT},
    {"B0_LINEDRAWCT     \0",	B0_LINEDRAWCT},
    {"B0_COORDH1        \0",	B0_COORDH1},
    {"B0_COORDV1        \0",	B0_COORDV1},
    {"B0_COORDH2        \0",	B0_COORDH2},
    {"B0_COORDV2        \0",	B0_COORDV2},
    {"B0_CPURAD2        \0",	B0_CPURAD2},
    {"B0_BBALUMWI       \0",	B0_BBALUMWI},
    {"B0_BBMODE         \0",	B0_BBMODE},
    {"B0_BBBLENDC       \0",	B0_BBBLENDC},
    {"B0_BBMAXMINCT     \0",	B0_BBMAXMINCT},
    {"B0_BBBINTHR       \0",	B0_BBBINTHR},
    {"B1_OSDMODE        \0",	B1_OSDMODE},
    {"B1_OSDCT          \0",	B1_OSDCT},
    {"B1_BOTRANS0       \0",	B1_BOTRANS0},
    {"B1_BOTRANS1       \0",	B1_BOTRANS1},
    {"B1_BOTRANS2       \0",	B1_BOTRANS2},
    {"B1_BOTRANS3       \0",	B1_BOTRANS3},
    {"B1_OSDACTHST      \0",	B1_OSDACTHST},
    {"B1_OSDACTHW       \0",	B1_OSDACTHW},
    {"B1_OSDACTVST      \0",	B1_OSDACTVST},
    {"B1_OSDACTVW       \0",	B1_OSDACTVW},
    {"B1_OSDSAD         \0",	B1_OSDSAD},
    {"B1_OSDMWI         \0",	B1_OSDMWI},
    {"B1_PLTAD          \0",	B1_PLTAD},
    {"B1_PLTDT          \0",	B1_PLTDT},
    {"B1_BLINKTIME      \0",	B1_BLINKTIME},
    {"B1_BLINKCYCL      \0",	B1_BLINKCYCL},
    {"B1_OSDGAINR0      \0",	B1_OSDGAINR0},
    {"B1_OSDGAING0      \0",	B1_OSDGAING0},
    {"B1_OSDGAINB0      \0",	B1_OSDGAINB0},
    {"B1_OSDBIASR0      \0",	B1_OSDBIASR0},
    {"B1_OSDBIASG0      \0",	B1_OSDBIASG0},
    {"B1_OSDBIASB0      \0",	B1_OSDBIASB0},
    {"B1_OSDGAINR1      \0",	B1_OSDGAINR1},
    {"B1_OSDGAING1      \0",	B1_OSDGAING1},
    {"B1_OSDGAINB1      \0",	B1_OSDGAINB1},
    {"B1_OSDBIASR1      \0",	B1_OSDBIASR1},
    {"B1_OSDBIASG1      \0",	B1_OSDBIASG1},
    {"B1_OSDBIASB1      \0",	B1_OSDBIASB1},
    {"B1_OSDGAINR2      \0",	B1_OSDGAINR2},
    {"B1_OSDGAING2      \0",	B1_OSDGAING2},
    {"B1_OSDGAINB2      \0",	B1_OSDGAINB2},
    {"B1_OSDBIASR2      \0",	B1_OSDBIASR2},
    {"B1_OSDBIASG2      \0",	B1_OSDBIASG2},
    {"B1_OSDBIASB2      \0",	B1_OSDBIASB2},
    {"B1_OSDGAINR3      \0",	B1_OSDGAINR3},
    {"B1_OSDGAING3      \0",	B1_OSDGAING3},
    {"B1_OSDGAINB3      \0",	B1_OSDGAINB3},
    {"B1_OSDBIASR3      \0",	B1_OSDBIASR3},
    {"B1_OSDBIASG3      \0",	B1_OSDBIASG3},
    {"B1_OSDBIASB3      \0",	B1_OSDBIASB3},
    {"B1_OSDBLCT        \0",	B1_OSDBLCT},
    {"B1_OSDBLENDC      \0",	B1_OSDBLENDC},
    {"B1_OSDSCRLCT      \0",	B1_OSDSCRLCT},
    {"B1_OSDSCRLSPD     \0",	B1_OSDSCRLSPD},
    {"B1_OSDSCRLDST     \0",	B1_OSDSCRLDST},
    {"B1_OSDSCRLHW      \0",	B1_OSDSCRLHW},
    {"B1_OSDSCRLVW      \0",	B1_OSDSCRLVW},
    {"B1_PGCTRL         \0",	B1_PGCTRL},
    {"B1_PGPTCTRL       \0",	B1_PGPTCTRL},
    {"B1_HRAMPCNT       \0",	B1_HRAMPCNT},
    {"B1_VRAMPCNT       \0",	B1_VRAMPCNT},
    {"B1_HLIMITMIN      \0",	B1_HLIMITMIN},
    {"B1_VLIMITMIN      \0",	B1_VLIMITMIN},
    {"B1_HLIMITMAX      \0",	B1_HLIMITMAX},
    {"B1_VLIMITMAX      \0",	B1_VLIMITMAX},
    {"B1_PGCOLR         \0",	B1_PGCOLR},
    {"B1_PGCOLG         \0",	B1_PGCOLG},
    {"B1_PGCOLB         \0",	B1_PGCOLB},
    {"B1_PGCOLLSR       \0",	B1_PGCOLLSR},
    {"B1_PGCOLLSG       \0",	B1_PGCOLLSG},
    {"B1_PGCOLLSB       \0",	B1_PGCOLLSB},
    {"B1_SFLMODE        \0",	B1_SFLMODE},
    {"B1_SFLCT          \0",	B1_SFLCT},
    {"B1_SFLCMDWR       \0",	B1_SFLCMDWR},
    {"B1_SFLCMDRD       \0",	B1_SFLCMDRD},
    {"B1_SFLCMDER       \0",	B1_SFLCMDER},
    {"B1_SFLCMDWREN     \0",	B1_SFLCMDWREN},
    {"B1_SFLCMDWRSR     \0",	B1_SFLCMDWRSR},
    {"B1_SFLCMDRDSR     \0",	B1_SFLCMDRDSR},
    {"B1_SFLWRSR        \0",	B1_SFLWRSR},
    {"B1_SFLWRRH        \0",	B1_SFLWRRH},
    {"B1_SFLAD          \0",	B1_SFLAD},
    {"B1_SFLDT          \0",	B1_SFLDT},
    {"B1_SFLDTCTL       \0",	B1_SFLDTCTL},
    {"B1_SFLSTAT        \0",	B1_SFLSTAT},
    {"B1_SFLRDSR        \0",	B1_SFLRDSR},
    {"B1_SFLCNT         \0",	B1_SFLCNT},
    {"B1_SFLREGSEL      \0",	B1_SFLREGSEL},
    {"B1_SFLTEST        \0",	B1_SFLTEST},
    {"B1_SFLDVDLY       \0",	B1_SFLDVDLY},
    {"B1_SFLSZSEL       \0",	B1_SFLSZSEL},
    {"B1_GIOC           \0",	B1_GIOC},
    {"B1_GIOO           \0",	B1_GIOO},
    {"B1_GIOI           \0",	B1_GIOI},
    {"B1_GIOS           \0",	B1_GIOS},
    {"B1_GIOPDCT        \0",	B1_GIOPDCT},
    {"B1_INTEN          \0",	B1_INTEN},
    {"B1_INTRST         \0",	B1_INTRST},
    {"B1_INTSTAT        \0",	B1_INTSTAT},
    {"B1_TESTG          \0",	B1_TESTG},
    {"B2_OSYCT          \0",	B2_OSYCT},
    {"B2_OHCYCL         \0",	B2_OHCYCL},
    {"B2_OVCYCL         \0",	B2_OVCYCL},
    {"B2_OSYCT2         \0",	B2_OSYCT2},
    {"B2_OSYCT3         \0",	B2_OSYCT3},
    {"B2_OHSCT          \0",	B2_OHSCT},
    {"B2_OVSCT          \0",	B2_OVSCT},
    {"B2_OVDLY          \0",	B2_OVDLY},
    {"B2_OVHDLY         \0",	B2_OVHDLY},
    {"B2_OHDLY          \0",	B2_OHDLY},
    {"B2_SYRDLY         \0",	B2_SYRDLY},
    {"B2_OVCYCLMIN      \0",	B2_OVCYCLMIN},
    {"B2_SYVRSTCT       \0",	B2_SYVRSTCT},
    {"B2_FLDDLY         \0",	B2_FLDDLY},
    {"B2_FLDRTCT        \0",	B2_FLDRTCT},
    {"B2_FLDRTCT2       \0",	B2_FLDRTCT2},
    {"B2_OSWP           \0",	B2_OSWP},
    {"B2_OYUVOCT        \0",	B2_OYUVOCT},
    {"B2_OINTCT         \0",	B2_OINTCT},
    {"B2_OEMBCT         \0",	B2_OEMBCT},
    {"B2_OAOI0HST       \0",	B2_OAOI0HST},
    {"B2_OAOI0HEND      \0",	B2_OAOI0HEND},
    {"B2_OAOI0VST       \0",	B2_OAOI0VST},
    {"B2_OAOI0VEND      \0",	B2_OAOI0VEND},
    {"B2_OAOI1HST       \0",	B2_OAOI1HST},
    {"B2_OAOI1HEND      \0",	B2_OAOI1HEND},
    {"B2_OAOI1VST       \0",	B2_OAOI1VST},
    {"B2_OAOI1VEND      \0",	B2_OAOI1VEND},
    {"B2_OAOI2HST       \0",	B2_OAOI2HST},
    {"B2_OAOI2HEND      \0",	B2_OAOI2HEND},
    {"B2_OAOI2VST       \0",	B2_OAOI2VST},
    {"B2_OAOI2VEND      \0",	B2_OAOI2VEND},
    {"B2_OACTHST        \0",	B2_OACTHST},
    {"B2_OACTHW         \0",	B2_OACTHW},
    {"B2_OACTVST        \0",	B2_OACTVST},
    {"B2_OACTVW         \0",	B2_OACTVW},
    {"B2_OIMGCT         \0",	B2_OIMGCT},
    {"B2_OFILL          \0",	B2_OFILL},
    {"B2_OBKGD0         \0",	B2_OBKGD0},
    {"B2_OBKGD1         \0",	B2_OBKGD1},
    {"B2_OBKGD2         \0",	B2_OBKGD2},
    {"B2_AUTOFILLCT     \0",	B2_AUTOFILLCT},
    {"B2_OCSCCF00       \0",	B2_OCSCCF00},
    {"B2_OCSCCF01       \0",	B2_OCSCCF01},
    {"B2_OCSCCF02       \0",	B2_OCSCCF02},
    {"B2_OCSCCF10       \0",	B2_OCSCCF10},
    {"B2_OCSCCF11       \0",	B2_OCSCCF11},
    {"B2_OCSCCF12       \0",	B2_OCSCCF12},
    {"B2_OCSCCF20       \0",	B2_OCSCCF20},
    {"B2_OCSCCF21       \0",	B2_OCSCCF21},
    {"B2_OCSCCF22       \0",	B2_OCSCCF22},
    {"B2_OBIASR         \0",	B2_OBIASR},
    {"B2_OBIASG         \0",	B2_OBIASG},
    {"B2_OBIASB         \0",	B2_OBIASB},
    {"B2_OVPHCT         \0",	B2_OVPHCT},
    {"B2_OVADJCT        \0",	B2_OVADJCT},
    {"B2_GENLOCKCT      \0",	B2_GENLOCKCT},
    {"B2_GLGAIN         \0",	B2_GLGAIN},
    {"B2_GLTEST         \0",	B2_GLTEST},
    {"B2_OVPHRDT        \0",	B2_OVPHRDT},
    {"B2_ACTCT          \0",	B2_ACTCT},
    {"B2_ATBGAIN        \0",	B2_ATBGAIN},
    {"B2_HISTGRAMCT     \0",	B2_HISTGRAMCT},
    {"B2_PTLEVELTH      \0",	B2_PTLEVELTH},
    {"B2_ATBPI          \0",	B2_ATBPI},
    {"B3_IIMGCT         \0",	B3_IIMGCT},
    {"B3_ISYCT          \0",	B3_ISYCT},
    {"B3_IVLNDLY        \0",	B3_IVLNDLY},
    {"B3_IVSKDLY        \0",	B3_IVSKDLY},
    {"B3_IHSKDLY        \0",	B3_IHSKDLY},
    {"B3_AUTOFLD        \0",	B3_AUTOFLD},
    {"B3_DIFCT          \0",	B3_DIFCT},
    {"B3_DHCYCL         \0",	B3_DHCYCL},
    {"B3_DVDLY          \0",	B3_DVDLY},
    {"B3_ICPNCT         \0",	B3_ICPNCT},
    {"B3_ICPNCT2        \0",	B3_ICPNCT2},
    {"B3_IACTHST        \0",	B3_IACTHST},
    {"B3_IACTHW         \0",	B3_IACTHW},
    {"B3_IACTVST        \0",	B3_IACTVST},
    {"B3_IACTVW         \0",	B3_IACTVW},
    {"B3_IAOIHST        \0",	B3_IAOIHST},
    {"B3_IAOIHEND       \0",	B3_IAOIHEND},
    {"B3_IAOIVST        \0",	B3_IAOIVST},
    {"B3_IAOIVEND       \0",	B3_IAOIVEND},
    {"B3_ISWP           \0",	B3_ISWP},
    {"B3_ICOLORCT       \0",	B3_ICOLORCT},
    {"B3_IBIASR         \0",	B3_IBIASR},
    {"B3_IBIASG         \0",	B3_IBIASG},
    {"B3_IBIASB         \0",	B3_IBIASB},
    {"B3_IGAINR         \0",	B3_IGAINR},
    {"B3_IGAING         \0",	B3_IGAING},
    {"B3_IGAINB         \0",	B3_IGAINB},
    {"B3_ICSC1CF00      \0",	B3_ICSC1CF00},
    {"B3_ICSC1CF01      \0",	B3_ICSC1CF01},
    {"B3_ICSC1CF02      \0",	B3_ICSC1CF02},
    {"B3_ICSC1CF10      \0",	B3_ICSC1CF10},
    {"B3_ICSC1CF11      \0",	B3_ICSC1CF11},
    {"B3_ICSC1CF12      \0",	B3_ICSC1CF12},
    {"B3_ICSC1CF20      \0",	B3_ICSC1CF20},
    {"B3_ICSC1CF21      \0",	B3_ICSC1CF21},
    {"B3_ICSC1CF22      \0",	B3_ICSC1CF22},
    {"B3_ICSC2CF00      \0",	B3_ICSC2CF00},
    {"B3_ICSC2CF01      \0",	B3_ICSC2CF01},
    {"B3_ICSC2CF02      \0",	B3_ICSC2CF02},
    {"B3_ICSC2CF10      \0",	B3_ICSC2CF10},
    {"B3_ICSC2CF11      \0",	B3_ICSC2CF11},
    {"B3_ICSC2CF12      \0",	B3_ICSC2CF12},
    {"B3_ICSC2CF20      \0",	B3_ICSC2CF20},
    {"B3_ICSC2CF21      \0",	B3_ICSC2CF21},
    {"B3_ICSC2CF22      \0",	B3_ICSC2CF22},
    {"B3_UVLPCOEF0      \0",	B3_UVLPCOEF0},
    {"B3_UVLPCOEF1      \0",	B3_UVLPCOEF1},
    {"B3_UVLPCOEF2      \0",	B3_UVLPCOEF2},
    {"B3_IFILLCT        \0",	B3_IFILLCT},
    {"B3_IFILL          \0",	B3_IFILL},
    {"B3_PMODE          \0",	B3_PMODE},
    {"B3_PIH0           \0",	B3_PIH0},
    {"B3_PIV0           \0",	B3_PIV0},
    {"B3_PIH1           \0",	B3_PIH1},
    {"B3_PIV1           \0",	B3_PIV1},
    {"B3_PTH            \0",	B3_PTH},
    {"B3_IPIXVALPOSH    \0",	B3_IPIXVALPOSH},
    {"B3_IPIXVALPOSV    \0",	B3_IPIXVALPOSV},
    {"B3_ACTCT          \0",	B3_ACTCT},
    {"B3_ATBGAIN        \0",	B3_ATBGAIN},
    {"B3_PTLEVELTH      \0",	B3_PTLEVELTH},
    {"B3_IFLPCT         \0",	B3_IFLPCT},
    {"B3_IDVLPCT        \0",	B3_IDVLPCT},
    {"B3_IDVLPACTHST    \0",	B3_IDVLPACTHST},
    {"B3_IDVLPACTHW     \0",	B3_IDVLPACTHW},
    {"B3_IDVLPACTVST    \0",	B3_IDVLPACTVST},
    {"B3_IDVLPACTVW     \0",	B3_IDVLPACTVW},
    {"B3_I420TEST       \0",	B3_I420TEST},
    {"B4_IPOHCYCL       \0",	B4_IPOHCYCL},
    {"B4_IPOVCYCL       \0",	B4_IPOVCYCL},
    {"B4_IPSYRDLY       \0",	B4_IPSYRDLY},
    {"B4_IPOACTHST      \0",	B4_IPOACTHST},
    {"B4_IPOACTHW       \0",	B4_IPOACTHW},
    {"B4_IPOACTVST      \0",	B4_IPOACTVST},
    {"B4_IPOACTVW       \0",	B4_IPOACTVW},
    {"B4_MVNRCT         \0",	B4_MVNRCT},
    {"B4_HMVNRCFA       \0",	B4_HMVNRCFA},
    {"B4_HMVNRCFB       \0",	B4_HMVNRCFB},
    {"B4_HMVNRCFC       \0",	B4_HMVNRCFC},
    {"B4_HMVNRCFD       \0",	B4_HMVNRCFD},
    {"B4_VFILCFC        \0",	B4_VFILCFC},
    {"B4_VFILCFD        \0",	B4_VFILCFD},
    {"B4_HFILCFA        \0",	B4_HFILCFA},
    {"B4_HFILCFB        \0",	B4_HFILCFB},
    {"B4_HFILCFC        \0",	B4_HFILCFC},
    {"B4_HFILCFD        \0",	B4_HFILCFD},
    {"B4_XVFILMD        \0",	B4_XVFILMD},
    {"B4_XNRCUT         \0",	B4_XNRCUT},
    {"B4_LUTWAD         \0",	B4_LUTWAD},
    {"B4_LUTCF          \0",	B4_LUTCF},
    {"B4_LUTWR          \0",	B4_LUTWR},
    {"B4_IPADJCT        \0",	B4_IPADJCT},
    {"B4_IPTEST         \0",	B4_IPTEST},
    {"B4_PDCTRL1        \0",	B4_PDCTRL1},
    {"B4_PDCTRL2        \0",	B4_PDCTRL2},
    {"B4_PDMV22TH       \0",	B4_PDMV22TH},
    {"B4_PDMV23TH       \0",	B4_PDMV23TH},
    {"B4_PDSQTH         \0",	B4_PDSQTH},
    {"B4_MVACCSEL       \0",	B4_MVACCSEL},
    {"B4_PDACTSEL       \0",	B4_PDACTSEL},
    {"B4_PD22RSTCT      \0",	B4_PD22RSTCT},
    {"B4_PD23RSTTH      \0",	B4_PD23RSTTH},
    {"B4_PD2332CTRL     \0",	B4_PD2332CTRL},
    {"B4_PD23322CTRL    \0",	B4_PD23322CTRL},
    {"B4_PD23232CTRL    \0",	B4_PD23232CTRL},
    {"B4_PD33CTRL       \0",	B4_PD33CTRL},
    {"B4_PD55CTRL       \0",	B4_PD55CTRL},
    {"B4_PD87CTRL       \0",	B4_PD87CTRL},
    {"B4_COMBCT         \0",	B4_COMBCT},
    {"B4_COMBLTH        \0",	B4_COMBLTH},
    {"B4_COMBHTH        \0",	B4_COMBHTH},
    {"B4_MVCMBCT        \0",	B4_MVCMBCT},
    {"B4_MVCMBTH        \0",	B4_MVCMBTH},
    {"B4_MVCMBABSTH     \0",	B4_MVCMBABSTH},
    {"B4_MVCMBPREV      \0",	B4_MVCMBPREV},
    {"B4_PD22ADJ        \0",	B4_PD22ADJ},
    {"B4_PDTEST         \0",	B4_PDTEST},
    {"B4_PDNRCT         \0",	B4_PDNRCT},
    {"B4_PDMVNRCFA      \0",	B4_PDMVNRCFA},
    {"B4_PDMVNRCFB      \0",	B4_PDMVNRCFB},
    {"B4_PDMVNRCFC      \0",	B4_PDMVNRCFC},
    {"B4_PDMVNRCFD      \0",	B4_PDMVNRCFD},
    {"B4_PDMV22NEWTH    \0",	B4_PDMV22NEWTH},
    {"B4_PDMV23NEWTH    \0",	B4_PDMV23NEWTH},
    {"B4_PDSCLMD        \0",	B4_PDSCLMD},
    {"B4_DIAGCT0        \0",	B4_DIAGCT0},
    {"B4_DIAGCT1        \0",	B4_DIAGCT1},
    {"B4_DIAGCT2        \0",	B4_DIAGCT2},
    {"B4_DIAGTH0        \0",	B4_DIAGTH0},
    {"B4_DIAGTH1        \0",	B4_DIAGTH1},
    {"B4_DIAGTH2        \0",	B4_DIAGTH2},
    {"B4_DIAGTH3        \0",	B4_DIAGTH3},
    {"B4_DIAGTH4        \0",	B4_DIAGTH4},
    {"B4_DIAGTH5        \0",	B4_DIAGTH5},
    {"B4_DIAGTH6        \0",	B4_DIAGTH6},
    {"B4_DIAGTH7        \0",	B4_DIAGTH7},
    {"B4_DIAGTH8        \0",	B4_DIAGTH8},
    {"B4_DIAGTH9        \0",	B4_DIAGTH9},
    {"B4_DIAGTH10       \0",	B4_DIAGTH10},
    {"B4_DIAGTH11       \0",	B4_DIAGTH11},
    {"B4_DIAGTH12       \0",	B4_DIAGTH12},
    {"B4_DIAGAD0        \0",	B4_DIAGAD0},
    {"B4_DIAGAD1        \0",	B4_DIAGAD1},
    {"B4_DIAGAD2        \0",	B4_DIAGAD2},
    {"B4_DIAGAD3        \0",	B4_DIAGAD3},
    {"B4_MVNRMODE       \0",	B4_MVNRMODE},
    {"B4_MVNRTH         \0",	B4_MVNRTH},
    {"B4_IPHMON         \0",	B4_IPHMON},
    {"B4_ACTMSKCT       \0",	B4_ACTMSKCT},
    {"B4_IPHSYRDLY      \0",	B4_IPHSYRDLY},
    {"B5_DTCT           \0",	B5_DTCT},
    {"B5_DTAD           \0",	B5_DTAD},
    {"B5_DTDT           \0",	B5_DTDT},
    {"B5_DTSFTXR        \0",	B5_DTSFTXR},
    {"B5_DTSFTXG        \0",	B5_DTSFTXG},
    {"B5_DTSFTXB        \0",	B5_DTSFTXB},
    {"B5_DTSFTYR        \0",	B5_DTSFTYR},
    {"B5_DTSFTYG        \0",	B5_DTSFTYG},
    {"B5_DTSFTYB        \0",	B5_DTSFTYB},
    {"B5_HADPCT         \0",	B5_HADPCT},
    {"B5_HADPCOEF0      \0",	B5_HADPCOEF0},
    {"B5_HADPCOEF1      \0",	B5_HADPCOEF1},
    {"B5_HADPPOS0       \0",	B5_HADPPOS0},
    {"B5_HADPPOS1       \0",	B5_HADPPOS1},
    {"B5_VADPCT         \0",	B5_VADPCT},
    {"B5_VADPCOEF0      \0",	B5_VADPCOEF0},
    {"B5_VADPCOEF1      \0",	B5_VADPCOEF1},
    {"B5_VADPPOS0       \0",	B5_VADPPOS0},
    {"B5_VADPPOS1       \0",	B5_VADPPOS1},
    {"B5_WPACCT         \0",	B5_WPACCT},
    {"B5_FRCCT          \0",	B5_FRCCT},
    {"B5_WPBKGDCT       \0",	B5_WPBKGDCT},
    {"B5_GMCT           \0",	B5_GMCT},
    {"B5_GMBNKSEL       \0",	B5_GMBNKSEL},
    {"B5_GMAD           \0",	B5_GMAD},
    {"B5_GMDT           \0",	B5_GMDT},
    {"B5_ERDFCT         \0",	B5_ERDFCT},
    {"B5_UCCT           \0",	B5_UCCT},
    {"B5_UCAD           \0",	B5_UCAD},
    {"B5_UCDT           \0",	B5_UCDT},
    {"B5_ERDFCT2        \0",	B5_ERDFCT2},
    {"B5_WPLGCT         \0",	B5_WPLGCT},
    {"B5_WPLGACTHW      \0",	B5_WPLGACTHW},
    {"B5_WPLGACTVW      \0",	B5_WPLGACTVW},
    {"B5_WPLGHBOUND0    \0",	B5_WPLGHBOUND0},
    {"B5_WPLGHBOUND1    \0",	B5_WPLGHBOUND1},
    {"B5_WPLGA          \0",	B5_WPLGA},
    {"B5_WPLGB          \0",	B5_WPLGB},
    {"B5_WPLGC          \0",	B5_WPLGC},
    {"B5_WPLGD          \0",	B5_WPLGD},
    {"B5_WPLGE          \0",	B5_WPLGE},
    {"B5_WPLGF          \0",	B5_WPLGF},
    {"B5_WPLGG          \0",	B5_WPLGG},
    {"B5_WPLGH          \0",	B5_WPLGH},
    {"B5_WPLGI          \0",	B5_WPLGI},
    {"B5_WCSCT          \0",	B5_WCSCT},
    {"B5_WCSAD          \0",	B5_WCSAD},
    {"B5_WCSDT          \0",	B5_WCSDT},
    {"B5_GPLCT          \0",	B5_GPLCT},
    {"B5_GPLSFT         \0",	B5_GPLSFT},
    {"B5_GPLCX          \0",	B5_GPLCX},
    {"B5_GPLCY          \0",	B5_GPLCY},
    {"B5_GPLMULT        \0",	B5_GPLMULT},
    {"B5_3DCTRL         \0",	B5_3DCTRL},
    {"B5_O3DLACTHW      \0",	B5_O3DLACTHW},
    {"B5_O3DMACTHW      \0",	B5_O3DMACTHW},
    {"B5_O3DRACTHW      \0",	B5_O3DRACTHW},
    {"B5_M3DLACTHST     \0",	B5_M3DLACTHST},
    {"B5_M3DLACTHEND    \0",	B5_M3DLACTHEND},
    {"B5_M3DRACTHST     \0",	B5_M3DRACTHST},
    {"B5_M3DRACTHEND    \0",	B5_M3DRACTHEND},
    {"B5_O3DSMLACTHW    \0",	B5_O3DSMLACTHW},
    {"B5_O3DSMLMSKHW    \0",	B5_O3DSMLMSKHW},
    {"B5_WPACTHW        \0",	B5_WPACTHW},
    {"B5_WPACTVW        \0",	B5_WPACTVW},
    {"B6_HLUT0          \0",	B6_HLUT0},
    {"B6_HLUT1          \0",	B6_HLUT1},
    {"B6_HLUT2          \0",	B6_HLUT2},
    {"B6_HLUT3          \0",	B6_HLUT3},
    {"B6_HLUT4          \0",	B6_HLUT4},
    {"B6_HLUT5          \0",	B6_HLUT5},
    {"B6_HLUT6          \0",	B6_HLUT6},
    {"B6_HLUT7          \0",	B6_HLUT7},
    {"B6_HLUT8          \0",	B6_HLUT8},
    {"B6_HLUT9          \0",	B6_HLUT9},
    {"B6_HLUT10         \0",	B6_HLUT10},
    {"B6_HLUT11         \0",	B6_HLUT11},
    {"B6_HLUT12         \0",	B6_HLUT12},
    {"B6_HLUT13         \0",	B6_HLUT13},
    {"B6_HLUT14         \0",	B6_HLUT14},
    {"B6_HLUT15         \0",	B6_HLUT15},
    {"B6_HLUT16         \0",	B6_HLUT16},
    {"B6_HLUT17         \0",	B6_HLUT17},
    {"B6_HLUT18         \0",	B6_HLUT18},
    {"B6_HLUT19         \0",	B6_HLUT19},
    {"B6_HLUT20         \0",	B6_HLUT20},
    {"B6_HLUT21         \0",	B6_HLUT21},
    {"B6_HLUT22         \0",	B6_HLUT22},
    {"B6_HLUT23         \0",	B6_HLUT23},
    {"B6_VLUT0          \0",	B6_VLUT0},
    {"B6_VLUT1          \0",	B6_VLUT1},
    {"B6_VLUT2          \0",	B6_VLUT2},
    {"B6_VLUT3          \0",	B6_VLUT3},
    {"B6_VLUT4          \0",	B6_VLUT4},
    {"B6_VLUT5          \0",	B6_VLUT5},
    {"B6_VLUT6          \0",	B6_VLUT6},
    {"B6_VLUT7          \0",	B6_VLUT7},
    {"B6_VLUT8          \0",	B6_VLUT8},
    {"B6_VLUT9          \0",	B6_VLUT9},
    {"B6_VLUT10         \0",	B6_VLUT10},
    {"B6_VLUT11         \0",	B6_VLUT11},
    {"B6_VLUT12         \0",	B6_VLUT12},
    {"B6_VLUT13         \0",	B6_VLUT13},
    {"B6_VLUT14         \0",	B6_VLUT14},
    {"B6_VLUT15         \0",	B6_VLUT15},
    {"B6_VLUT16         \0",	B6_VLUT16},
    {"B6_VLUT17         \0",	B6_VLUT17},
    {"B6_VLUT18         \0",	B6_VLUT18},
    {"B6_VLUT19         \0",	B6_VLUT19},
    {"B6_VLUT20         \0",	B6_VLUT20},
    {"B6_VLUT21         \0",	B6_VLUT21},
    {"B6_VLUT22         \0",	B6_VLUT22},
    {"B6_VLUT23         \0",	B6_VLUT23},
    {"B6_H2LUT0         \0",	B6_H2LUT0},
    {"B6_H2LUT1         \0",	B6_H2LUT1},
    {"B6_H2LUT2         \0",	B6_H2LUT2},
    {"B6_H2LUT3         \0",	B6_H2LUT3},
    {"B6_H2LUT4         \0",	B6_H2LUT4},
    {"B6_H2LUT5         \0",	B6_H2LUT5},
    {"B6_H2LUT6         \0",	B6_H2LUT6},
    {"B6_H2LUT7         \0",	B6_H2LUT7},
    {"B6_H2LUT8         \0",	B6_H2LUT8},
    {"B6_H2LUT9         \0",	B6_H2LUT9},
    {"B6_H2LUT10        \0",	B6_H2LUT10},
    {"B6_H2LUT11        \0",	B6_H2LUT11},
    {"B6_H2LUT12        \0",	B6_H2LUT12},
    {"B6_H2LUT13        \0",	B6_H2LUT13},
    {"B6_H2LUT14        \0",	B6_H2LUT14},
    {"B6_H2LUT15        \0",	B6_H2LUT15},
    {"B6_H2LUT16        \0",	B6_H2LUT16},
    {"B6_H2LUT17        \0",	B6_H2LUT17},
    {"B6_H2LUT18        \0",	B6_H2LUT18},
    {"B6_H2LUT19        \0",	B6_H2LUT19},
    {"B6_H2LUT20        \0",	B6_H2LUT20},
    {"B6_H2LUT21        \0",	B6_H2LUT21},
    {"B6_H2LUT22        \0",	B6_H2LUT22},
    {"B6_H2LUT23        \0",	B6_H2LUT23},
    {"B6_V2LUT0         \0",	B6_V2LUT0},
    {"B6_V2LUT1         \0",	B6_V2LUT1},
    {"B6_V2LUT2         \0",	B6_V2LUT2},
    {"B6_V2LUT3         \0",	B6_V2LUT3},
    {"B6_V2LUT4         \0",	B6_V2LUT4},
    {"B6_V2LUT5         \0",	B6_V2LUT5},
    {"B6_V2LUT6         \0",	B6_V2LUT6},
    {"B6_V2LUT7         \0",	B6_V2LUT7},
    {"B6_V2LUT8         \0",	B6_V2LUT8},
    {"B6_V2LUT9         \0",	B6_V2LUT9},
    {"B6_V2LUT10        \0",	B6_V2LUT10},
    {"B6_V2LUT11        \0",	B6_V2LUT11},
    {"B6_V2LUT12        \0",	B6_V2LUT12},
    {"B6_V2LUT13        \0",	B6_V2LUT13},
    {"B6_V2LUT14        \0",	B6_V2LUT14},
    {"B6_V2LUT15        \0",	B6_V2LUT15},
    {"B6_V2LUT16        \0",	B6_V2LUT16},
    {"B6_V2LUT17        \0",	B6_V2LUT17},
    {"B6_V2LUT18        \0",	B6_V2LUT18},
    {"B6_V2LUT19        \0",	B6_V2LUT19},
    {"B6_V2LUT20        \0",	B6_V2LUT20},
    {"B6_V2LUT21        \0",	B6_V2LUT21},
    {"B6_V2LUT22        \0",	B6_V2LUT22},
    {"B6_V2LUT23        \0",	B6_V2LUT23},
    {"B6_SCLADPCT       \0",	B6_SCLADPCT},
    {"B6_SCLADPAD       \0",	B6_SCLADPAD},
    {"B6_SCLADPDT       \0",	B6_SCLADPDT},
    {"B6_SCLADPDGTH0    \0",	B6_SCLADPDGTH0},
    {"B6_SCLADPDGTH1    \0",	B6_SCLADPDGTH1},
    {"B6_SCLADPDGTH2    \0",	B6_SCLADPDGTH2},
    {"B6_SCLADPDGTH3    \0",	B6_SCLADPDGTH3},
    {"B6_SCLADPDGCF0    \0",	B6_SCLADPDGCF0},
    {"B6_SCLADPDGCF1    \0",	B6_SCLADPDGCF1},
    {"B6_SCLADPDGCF2    \0",	B6_SCLADPDGCF2},
    {"B6_SCLADPDGCF3    \0",	B6_SCLADPDGCF3},
    {"B6_WPERRCT        \0",	B6_WPERRCT},
    {"B6_WPHSHLIM       \0",	B6_WPHSHLIM},
    {"B6_WPVSHLIM       \0",	B6_WPVSHLIM},
    {"B6_WPHANGLIM      \0",	B6_WPHANGLIM},
    {"B6_WPVANGLIM      \0",	B6_WPVANGLIM},
    {"B7_EGBCT          \0",	B7_EGBCT},
    {"B7_LEGBHST        \0",	B7_LEGBHST},
    {"B7_LEGBHW         \0",	B7_LEGBHW},
    {"B7_REGBHST        \0",	B7_REGBHST},
    {"B7_REGBHW         \0",	B7_REGBHW},
    {"B7_TEGBVST        \0",	B7_TEGBVST},
    {"B7_TEGBVW         \0",	B7_TEGBVW},
    {"B7_BEGBVST        \0",	B7_BEGBVST},
    {"B7_BEGBVW         \0",	B7_BEGBVW},
    {"B7_LEGBMKCT       \0",	B7_LEGBMKCT},
    {"B7_REGBMKCT       \0",	B7_REGBMKCT},
    {"B7_TEGBMKCT       \0",	B7_TEGBMKCT},
    {"B7_BEGBMKCT       \0",	B7_BEGBMKCT},
    {"B7_HEGBINIT       \0",	B7_HEGBINIT},
    {"B7_LEGBCOEF       \0",	B7_LEGBCOEF},
    {"B7_REGBCOEF       \0",	B7_REGBCOEF},
    {"B7_VEGBINIT       \0",	B7_VEGBINIT},
    {"B7_TEGBCOEF       \0",	B7_TEGBCOEF},
    {"B7_BEGBCOEF       \0",	B7_BEGBCOEF},
    {"B7_EGBGMAD        \0",	B7_EGBGMAD},
    {"B7_EGBGMDT        \0",	B7_EGBGMDT},
    {"B7_EGBGMMCCT      \0",	B7_EGBGMMCCT},
    {"B7_EGBIASCT       \0",	B7_EGBIASCT},
    {"B7_EGBIASAD       \0",	B7_EGBIASAD},
    {"B7_EGBIASDT       \0",	B7_EGBIASDT},
    {"B8_CURCT          \0",	B8_CURCT},
    {"B8_CURTLX         \0",	B8_CURTLX},
    {"B8_CURTLY         \0",	B8_CURTLY},
    {"B8_CURTRX         \0",	B8_CURTRX},
    {"B8_CURTRY         \0",	B8_CURTRY},
    {"B8_CURBLX         \0",	B8_CURBLX},
    {"B8_CURBLY         \0",	B8_CURBLY},
    {"B8_CURBRX         \0",	B8_CURBRX},
    {"B8_CURBRY         \0",	B8_CURBRY},
    {"B8_CURHW          \0",	B8_CURHW},
    {"B8_CURVW          \0",	B8_CURVW},
    {"B8_CURBLINKTIME   \0",	B8_CURBLINKTIME},
    {"B8_CURBLINKCYCL   \0",	B8_CURBLINKCYCL},
    {"B8_CURCPUAD       \0",	B8_CURCPUAD},
    {"B8_CURCPUDT       \0",	B8_CURCPUDT},
    {"B8_CURGAINR       \0",	B8_CURGAINR},
    {"B8_CURGAING       \0",	B8_CURGAING},
    {"B8_CURGAINB       \0",	B8_CURGAINB},
    {"B8_CURBIASR       \0",	B8_CURBIASR},
    {"B8_CURBIASG       \0",	B8_CURBIASG},
    {"B8_CURBIASB       \0",	B8_CURBIASB},
    {"B8_CURPLTDTR0     \0",	B8_CURPLTDTR0},
    {"B8_CURPLTDTR1     \0",	B8_CURPLTDTR1},
    {"B8_CURPLTDTR2     \0",	B8_CURPLTDTR2},
    {"B8_CURPLTDTR3     \0",	B8_CURPLTDTR3},
    {"B8_CURPLTDTR4     \0",	B8_CURPLTDTR4},
    {"B8_CURPLTDTR5     \0",	B8_CURPLTDTR5},
    {"B8_CURPLTDTR6     \0",	B8_CURPLTDTR6},
    {"B8_CURPLTDTR7     \0",	B8_CURPLTDTR7},
    {"B8_CURPLTDTR8     \0",	B8_CURPLTDTR8},
    {"B8_CURPLTDTR9     \0",	B8_CURPLTDTR9},
    {"B8_CURPLTDTR10    \0",	B8_CURPLTDTR10},
    {"B8_CURPLTDTR11    \0",	B8_CURPLTDTR11},
    {"B8_CURPLTDTR12    \0",	B8_CURPLTDTR12},
    {"B8_CURPLTDTR13    \0",	B8_CURPLTDTR13},
    {"B8_CURPLTDTR14    \0",	B8_CURPLTDTR14},
    {"B8_CURPLTDTR15    \0",	B8_CURPLTDTR15},
    {"B8_CURPLTDTG0     \0",	B8_CURPLTDTG0},
    {"B8_CURPLTDTG1     \0",	B8_CURPLTDTG1},
    {"B8_CURPLTDTG2     \0",	B8_CURPLTDTG2},
    {"B8_CURPLTDTG3     \0",	B8_CURPLTDTG3},
    {"B8_CURPLTDTG4     \0",	B8_CURPLTDTG4},
    {"B8_CURPLTDTG5     \0",	B8_CURPLTDTG5},
    {"B8_CURPLTDTG6     \0",	B8_CURPLTDTG6},
    {"B8_CURPLTDTG7     \0",	B8_CURPLTDTG7},
    {"B8_CURPLTDTG8     \0",	B8_CURPLTDTG8},
    {"B8_CURPLTDTG9     \0",	B8_CURPLTDTG9},
    {"B8_CURPLTDTG10    \0",	B8_CURPLTDTG10},
    {"B8_CURPLTDTG11    \0",	B8_CURPLTDTG11},
    {"B8_CURPLTDTG12    \0",	B8_CURPLTDTG12},
    {"B8_CURPLTDTG13    \0",	B8_CURPLTDTG13},
    {"B8_CURPLTDTG14    \0",	B8_CURPLTDTG14},
    {"B8_CURPLTDTG15    \0",	B8_CURPLTDTG15},
    {"B8_CURPLTDTB0     \0",	B8_CURPLTDTB0},
    {"B8_CURPLTDTB1     \0",	B8_CURPLTDTB1},
    {"B8_CURPLTDTB2     \0",	B8_CURPLTDTB2},
    {"B8_CURPLTDTB3     \0",	B8_CURPLTDTB3},
    {"B8_CURPLTDTB4     \0",	B8_CURPLTDTB4},
    {"B8_CURPLTDTB5     \0",	B8_CURPLTDTB5},
    {"B8_CURPLTDTB6     \0",	B8_CURPLTDTB6},
    {"B8_CURPLTDTB7     \0",	B8_CURPLTDTB7},
    {"B8_CURPLTDTB8     \0",	B8_CURPLTDTB8},
    {"B8_CURPLTDTB9     \0",	B8_CURPLTDTB9},
    {"B8_CURPLTDTB10    \0",	B8_CURPLTDTB10},
    {"B8_CURPLTDTB11    \0",	B8_CURPLTDTB11},
    {"B8_CURPLTDTB12    \0",	B8_CURPLTDTB12},
    {"B8_CURPLTDTB13    \0",	B8_CURPLTDTB13},
    {"B8_CURPLTDTB14    \0",	B8_CURPLTDTB14},
    {"B8_CURPLTDTB15    \0",	B8_CURPLTDTB15},
    {"B9_OSFLD0         \0",	B9_OSFLD0},
    {"B9_OSFLD1         \0",	B9_OSFLD1},
    {"B9_OSFLD2         \0",	B9_OSFLD2},
    {"B9_OSFLD3         \0",	B9_OSFLD3},
    {"B9_ISFLD0         \0",	B9_ISFLD0},
    {"B9_ISFLD1         \0",	B9_ISFLD1},
    {"B9_ISFLD2         \0",	B9_ISFLD2},
    {"B9_ISFLD3         \0",	B9_ISFLD3},
    {"B9_OMWI           \0",	B9_OMWI},
    {"B9_IMWI           \0",	B9_IMWI},
    {"B9_MRTOSFLD0      \0",	B9_MRTOSFLD0},
    {"B9_MRTOSFLD1      \0",	B9_MRTOSFLD1},
    {"B9_MRTOSFLD2      \0",	B9_MRTOSFLD2},
    {"B9_MRTOSFLD3      \0",	B9_MRTOSFLD3},
    {"B9_MRTISFLD0      \0",	B9_MRTISFLD0},
    {"B9_MRTISFLD1      \0",	B9_MRTISFLD1},
    {"B9_MRTISFLD2      \0",	B9_MRTISFLD2},
    {"B9_MRTISFLD3      \0",	B9_MRTISFLD3},
    {"B9_MRTMWI         \0",	B9_MRTMWI},
    {"B9_MRTCT          \0",	B9_MRTCT},
    {"B9_MRTACTHW       \0",	B9_MRTACTHW},
    {"B9_MRTACTVW       \0",	B9_MRTACTVW},
    {"B9_MRTVDLY        \0",	B9_MRTVDLY},
    {"B9_EGBDBDSAD      \0",	B9_EGBDBDSAD},
    {"B9_EGBDBDMWI      \0",	B9_EGBDBDMWI},
    {"B9_EBIASSAD       \0",	B9_EBIASSAD},
    {"B9_EBIASMWI       \0",	B9_EBIASMWI},
    {"B9_UCDBDSAD       \0",	B9_UCDBDSAD},
    {"B9_UCDBDMWI       \0",	B9_UCDBDMWI},
    {"B9_IPOSFLD0       \0",	B9_IPOSFLD0},
    {"B9_IPOSFLD1       \0",	B9_IPOSFLD1},
    {"B9_IPOSFLD2       \0",	B9_IPOSFLD2},
    {"B9_IPOSFLD3       \0",	B9_IPOSFLD3},
    {"B9_IPISFLD0       \0",	B9_IPISFLD0},
    {"B9_IPISFLD1       \0",	B9_IPISFLD1},
    {"B9_IPISFLD2       \0",	B9_IPISFLD2},
    {"B9_IPISFLD3       \0",	B9_IPISFLD3},
    {"B9_IPMWI          \0",	B9_IPMWI},
    {"B9_IPYOFF         \0",	B9_IPYOFF},
    {"B9_MVFLD          \0",	B9_MVFLD},
    {"B9_WTRSADR        \0",	B9_WTRSADR},
    {"B9_WTRSADG        \0",	B9_WTRSADG},
    {"B9_WTRSADB        \0",	B9_WTRSADB},
    {"B9_WTRMWI         \0",	B9_WTRMWI},
    {"B9_WTRCT          \0",	B9_WTRCT},
    {"B10_IFH           \0",	B10_IFH},
    {"B10_IFV           \0",	B10_IFV},
    {"B10_IHW           \0",	B10_IHW},
    {"B10_IVW           \0",	B10_IVW},
    {"B10_IMFH          \0",	B10_IMFH},
    {"B10_IMFV          \0",	B10_IMFV},
    {"B10_IHVPHDF0      \0",	B10_IHVPHDF0},
    {"B10_IHVPHDF1      \0",	B10_IHVPHDF1},
    {"B10_FLDHIST       \0",	B10_FLDHIST},
    {"B10_PSTAT         \0",	B10_PSTAT},
    {"B10_PVAL0         \0",	B10_PVAL0},
    {"B10_PVAL1         \0",	B10_PVAL1},
    {"B10_PH0           \0",	B10_PH0},
    {"B10_PV0           \0",	B10_PV0},
    {"B10_PH1           \0",	B10_PH1},
    {"B10_PV1           \0",	B10_PV1},
    {"B10_IPIXR         \0",	B10_IPIXR},
    {"B10_IPIXG         \0",	B10_IPIXG},
    {"B10_IPIXB         \0",	B10_IPIXB},
    {"B10_IPTCNT        \0",	B10_IPTCNT},
    {"B10_IAPL0         \0",	B10_IAPL0},
    {"B10_IMAXFILTR     \0",	B10_IMAXFILTR},
    {"B10_IMINFILTR     \0",	B10_IMINFILTR},
    {"B10_STATE         \0",	B10_STATE},
    {"B10_IPFLDMON      \0",	B10_IPFLDMON},
    {"B10_IFACTHST      \0",	B10_IFACTHST},
    {"B10_IFACTHW       \0",	B10_IFACTHW},
    {"B10_IFACTVST      \0",	B10_IFACTVST},
    {"B10_IFACTVW       \0",	B10_IFACTVW},
    {"B10_OHISTGRAM     \0",	B10_OHISTGRAM},
    {"B10_OHISTGRAM_0   \0",	B10_OHISTGRAM_0},
    {"B10_OHISTGRAM_1   \0",	B10_OHISTGRAM_1},
    {"B10_OHISTGRAM_2   \0",	B10_OHISTGRAM_2},
    {"B10_OHISTGRAM_3   \0",	B10_OHISTGRAM_3},
    {"B10_OHISTGRAM_4   \0",	B10_OHISTGRAM_4},
    {"B10_OHISTGRAM_5   \0",	B10_OHISTGRAM_5},
    {"B10_OHISTGRAM_6   \0",	B10_OHISTGRAM_6},
    {"B10_OHISTGRAM_7   \0",	B10_OHISTGRAM_7},
    {"B10_OAPL0         \0",	B10_OAPL0},
    {"B10_MAXFILTR      \0",	B10_MAXFILTR},
    {"B10_MINFILTR      \0",	B10_MINFILTR},
    {"B10_OPTCNT        \0",	B10_OPTCNT},
    {"B10_OFV           \0",	B10_OFV},
    {"B10_IPOFV         \0",	B10_IPOFV},
    {"B10_OLNUM         \0",	B10_OLNUM},
    {"B10_ILNUM         \0",	B10_ILNUM},
    {"B10_LVDSRXMON     \0",	B10_LVDSRXMON},
    {"B10_DEVICECODE    \0",	B10_DEVICECODE},
    {"B10_AUTOFILLSTATE \0",	B10_AUTOFILLSTATE},
    {"B10_PD22CNT1      \0",	B10_PD22CNT1},
    {"B10_PD23CNT1      \0",	B10_PD23CNT1},
    {"B10_PD23CNT2      \0",	B10_PD23CNT2},
    {"B10_PD23CNT3      \0",	B10_PD23CNT3},
    {"B10_PD23CNT4      \0",	B10_PD23CNT4},
    {"B10_PD23CNT5      \0",	B10_PD23CNT5},
    {"B10_PDMON         \0",	B10_PDMON},
    {"B10_IPMON         \0",	B10_IPMON},
    {"B10_IPHCNT1       \0",	B10_IPHCNT1},
    {"B10_IPHCNT2       \0",	B10_IPHCNT2},
    {"B10_IPHCNT3       \0",	B10_IPHCNT3},
    {"B10_OSDSCSAD      \0",	B10_OSDSCSAD},
    {"B10_DDR3STATE     \0",	B10_DDR3STATE},
    {"B10_DDR3LVLSTATE  \0",	B10_DDR3LVLSTATE},
    {"B10_DDR3OCDDOP    \0",	B10_DDR3OCDDOP},
    {"B10_DDR3OCDDON    \0",	B10_DDR3OCDDON},
    {"B10_DDR3STATE     \0",	B10_DDR3STATE},
    {"B10_DDR3LVLSTATE  \0",	B10_DDR3LVLSTATE},
    {"B10_DDR3OCDDOP    \0",	B10_DDR3OCDDOP},
    {"B10_DDR3OCDDON    \0",	B10_DDR3OCDDON},
   // {"END               \0",	0xFFFFFFFF}
};
#define C789REG_NUMBER sizeof(m_sICHIPC789_DUMP)/sizeof(sICHIP_DUMP)		//G100_Doulas_0024
#endif

void dvC789_OPDMutexLockEvent(UINT16 uiEvent, const char *pcFunc)  //A35G2_Simon_0112
{
    uOPD_DATA uOPDData ;

    snprintf(uOPDData.cString, 128,  "%s,%s", pcFunc, cLastTakeC789Semaphore);

    utilOPD_EventSet(uiEvent, &uOPDData);
}

BOOL dvC789_SemaphoreTake(BOOL cEnable, const char *pcSemaphore)
{
    BOOL bResult = FALSE;

    if(cEnable)
    {
        bResult = Board_SSP_SemaphoreTake(eBOARD_SSP_CS_C789);

        if(bResult == FALSE)
        {
            dvC789_OPDMutexLockEvent(eOPD_MUTEX_LOCKED_LOG, pcSemaphore);   //A35G2_Simon_0112
            LOG_MSG(db_DV_SEMAPHORE, "(func:%s) is Fail, last take semaphore func %s\r\n", pcSemaphore, cLastTakeC789Semaphore);
        }
        else
        {
            sprintf(cLastTakeC789Semaphore, "%s", pcSemaphore);
        }
    }
    else
    {
        bResult = Board_SSP_SemaphoreGive(eBOARD_SSP_CS_C789);

        sprintf(cLastTakeC789Semaphore, "\0");
    }

    return bResult;
}

UINT32 dvC789_Read(UINT32 ulAddr)
{
    UINT8 ucBank, ucReg, ucSize;
    UINT8 aucWBuffer[2], aucRBuffer[2];
    UINT32 ulData;

    dvC789_Buffer_Flush();  //A35G2_CDS_Simon_0058

    ulData = 0;
    ucBank = (ulAddr>>16)&0xff;
    ucReg  = (ulAddr>>8)&0xff;
    ucSize = ulAddr&0xff;

    if((ucBank != m_ucCurrentBank) && (ucReg >= 0x08))
    {
        m_ucCurrentBank = ucBank;

        aucWBuffer[0] = 0x80;
        aucWBuffer[1] = ucBank;

//       Board_SSP_Master_Write(eBOARD_SSP_CS_C789, 2, aucWBuffer);
#ifdef CONTROL_789
        SPI_SendWriteCmd(eBOARD_SSP_CS_C789, aucWBuffer, 2, 1);
#endif
    }

    ucReg = ucReg + ucSize -1;
    while ( ucSize > 0 )
    {
        ucSize--;
        ulData = (ulData << 8);
        aucWBuffer[0] = ucReg;

//       Board_SSP_Master_Read(eBOARD_SSP_CS_C789, 2, aucWBuffer, aucRBuffer);
        SPI_SendReadCmd(eBOARD_SSP_CS_C789, aucWBuffer, aucRBuffer, 2, 2);

        ucReg--; //addr++;
        ulData += (aucRBuffer[1] & 0xFF);
   }

    return ulData;
}

void dvC789_Write(UINT32 ulAddr, UINT32 ulData)
{
    UINT8 ucBank, ucReg, ucSize, ucIdx;
    UINT8 aucWBuffer[16];

    dvC789_Buffer_Flush();  //A35G2_CDS_Simon_0058

	ucBank = (ulAddr>>16)&0xff;
	ucReg = (ulAddr>>8)&0xff;
	ucSize = ulAddr&0xff;

	if(ucReg == 0x00)
	{
		m_ucCurrentBank = ulData & 0x3F;
	}
	else if((ucBank != m_ucCurrentBank) && (ucReg >= 0x08))
	{
		m_ucCurrentBank = ucBank;
		aucWBuffer[0] = 0x80;
        aucWBuffer[1] = ucBank;

//       Board_SSP_Master_Write(eBOARD_SSP_CS_C789, 2, aucWBuffer);
#ifdef CONTROL_789
        SPI_SendWriteCmd(eBOARD_SSP_CS_C789, aucWBuffer, 2, 1);
#endif
	}

    ucIdx = 0;
	while(ucSize > 0)
	{
	    aucWBuffer[ucIdx++] = ucReg | 0x80;
        aucWBuffer[ucIdx++] = (UINT8)(ulData & 0xFF);

		ucReg++;
		ulData = (ulData >> 8);
		ucSize--;
	}
#ifdef CONTROL_789
    SPI_SendWriteCmd(eBOARD_SSP_CS_C789, aucWBuffer, ucIdx, 1);
#endif

	return;
}

void dvC789_Buffer_Flush(void)
{
    if(uiWriteIdx)
    {
        SPI_SendWriteCmd(eBOARD_SSP_CS_C789, aucWriteBuffer, uiWriteIdx, 1);
        uiWriteIdx = 0;
    }
}

void dvC789_WriteToBuffer(UINT32 ulAddr, UINT32 ulData)
{
    UINT8 ucBank, ucReg, ucSize;

    ucBank = (ulAddr>>16)&0xff;
    ucReg = (ulAddr>>8)&0xff;
    ucSize = ulAddr&0xff;

    if(ucReg == 0x00)
    {
        m_ucCurrentBank = ulData & 0x3F;

		//kenton_temp_check
		aucWriteBuffer[uiWriteIdx++] = 0x80;				//G100_Doulas_0027
        aucWriteBuffer[uiWriteIdx++] = m_ucCurrentBank;		//G100_Doulas_0027
        dvC789_Buffer_Flush();								//G100_Doulas_0027
    }
    else if((ucBank != m_ucCurrentBank) && (ucReg >= 0x08))
    {
        m_ucCurrentBank = ucBank;
        aucWriteBuffer[uiWriteIdx++] = 0x80;
        aucWriteBuffer[uiWriteIdx++] = ucBank;

        dvC789_Buffer_Flush();
    }

    while(ucSize > 0)
    {
        aucWriteBuffer[uiWriteIdx++] = ucReg | 0x80;
        aucWriteBuffer[uiWriteIdx++] = (UINT8)(ulData & 0xFF);

        if(uiWriteIdx >= SPI_BUFFER_SIZE)
        {
            dvC789_Buffer_Flush();
        }

        ucReg++;
        ulData = (ulData >> 8);
        ucSize--;
    }
}

void dvC789_BurstRead_FixedAdd(const UINT32 ulAddr, UINT16 uiSize, UINT8 *pucData) //A70LV_Larry_0139
{
    UINT8 ucBank, ucReg, aucWBuffer[2];
    UINT8 *pcBuffer = malloc(uiSize + 4);

    dvC789_Buffer_Flush();

    //-------------------------------------------------------------
    ucBank = (ulAddr >> 16) & 0x3F;
    ucReg = (ulAddr >> 8) & 0xFF;

    ucBank = BURST_MODE_FIXED_ADDRESS | ucBank;
    //b7-b6
    //0 Normal mode
    //1 Serial burst mode: address fixed
    //2 Serial burst mode: address auto increment
    //3 T.B.D
    //-------------------------------------------------------------
    if(ucReg == 0x00)
    {
        m_ucCurrentBank = *pucData & 0x3F;
        m_ucCurrentBank = BURST_MODE_FIXED_ADDRESS | m_ucCurrentBank;
    }
    else if((ucBank != m_ucCurrentBank) && (ucReg >= 0x08))//Change to appropriate bank
    {
        m_ucCurrentBank = ucBank;

        aucWBuffer[0] = 0x80;
        aucWBuffer[1] = ucBank;
        SPI_SendWriteCmd(eBOARD_SSP_CS_C789, aucWBuffer, 2, 1);
    }

    //-------------------------------------------------------------
    aucWBuffer[0] = ucReg;
    SPI_SendReadCmd(eBOARD_SSP_CS_C789, aucWBuffer, pcBuffer, 2, uiSize + 1);

    memcpy(pucData, &pcBuffer[1], uiSize);//0 is dummy
    free(pcBuffer);
}

void dvC789_BurstWrite_FixedAdd(const UINT32 ulAddr, UINT16 uiSize, UINT8 *pucData)
{
    UINT8 ucBank, ucReg;
    UINT8 *pcBuffer = malloc(uiSize + 4);

    dvC789_Buffer_Flush();

    //-------------------------------------------------------------
    ucBank = (ulAddr >> 16) & 0x3F;
    ucReg = (ulAddr >> 8) & 0xFF;

    ucBank = BURST_MODE_FIXED_ADDRESS | ucBank;
    //b7-b6
    //0 Normal mode
    //1 Serial burst mode: address fixed
    //2 Serial burst mode: address auto increment
    //3 T.B.D
    //-------------------------------------------------------------
    if(ucReg == 0x00)
    {
        m_ucCurrentBank = *pucData & 0x3F;
        m_ucCurrentBank = BURST_MODE_FIXED_ADDRESS | m_ucCurrentBank;
    }
    else if((ucBank != m_ucCurrentBank) && (ucReg >= 0x08))//Change to appropriate bank
    {
        m_ucCurrentBank = ucBank;

        pcBuffer[0] = 0x80;
        pcBuffer[1] = ucBank;

        SPI_SendWriteCmd(eBOARD_SSP_CS_C789, pcBuffer, 2, 1);
    }

    //-------------------------------------------------------------
    pcBuffer[0] = ucReg | 0x80;
    memcpy(&pcBuffer[1], pucData, uiSize);
    SPI_SendBurstWriteCmd(eBOARD_SSP_CS_C789, pcBuffer, (uiSize+1), 1);

    free(pcBuffer);

}

void dvC789_BurstRead_AddInc(const UINT32 ulAddr, UINT16 uiSize, UINT8 *pucData)
{
    UINT8 aucWBuffer[2] = {0};
    UINT8 ucBank, ucReg;
    UINT8 *pcBuffer = malloc(uiSize + 4);

    dvC789_Buffer_Flush();

    //-------------------------------------------------------------
    ucBank = (ulAddr >> 16) & 0x3F;
    ucReg = (ulAddr >> 8) & 0xFF;

    ucBank = BURST_MODE_AUTO_INC_ADDRESS | ucBank;
    //b7-b6
    //0 Normal mode
    //1 Serial burst mode: address fixed
    //2 Serial burst mode: address auto increment
    //3 T.B.D

    //-------------------------------------------------------------
    if(ucReg == 0x00)
    {
        m_ucCurrentBank = *pucData & 0x3F;
        m_ucCurrentBank = BURST_MODE_AUTO_INC_ADDRESS | m_ucCurrentBank;
    }
    else if((ucBank != m_ucCurrentBank) && (ucReg >= 0x08))//Change to appropriate bank
    {
        m_ucCurrentBank = ucBank;

        aucWBuffer[0] = 0x80;
        aucWBuffer[1] = ucBank;

        SPI_SendWriteCmd(eBOARD_SSP_CS_C789, aucWBuffer, 2, 1);
    }

    //-------------------------------------------------------------
    aucWBuffer[0] = ucReg;
    SPI_SendReadCmd(eBOARD_SSP_CS_C789, aucWBuffer, pcBuffer, 2, (uiSize+1));
    memcpy(pucData, &pcBuffer[1], uiSize); //0 is dummy

    free(pcBuffer);
}

void dvC789_BurstWrite_AddInc(const UINT32 ulAddr, UINT16 uiSize, UINT8 *pucData)
{
    UINT8 ucBank, ucReg;
    UINT8 *pcBuffer = malloc(uiSize + 4);

    dvC789_Buffer_Flush();

    //-------------------------------------------------------------
    ucBank = (ulAddr >> 16) & 0x3F;
    ucReg = (ulAddr >> 8) & 0xFF;

    ucBank = BURST_MODE_AUTO_INC_ADDRESS | ucBank;
    //b7-b6
    //0 Normal mode
    //1 Serial burst mode: address fixed
    //2 Serial burst mode: address auto increment
    //3 T.B.D

    //Board_SSP_SemaphoreTake(eBOARD_SSP_CS_C789);

    //-------------------------------------------------------------
    if(ucReg == 0x00)
    {
        m_ucCurrentBank = *pucData & 0x3F;
        m_ucCurrentBank = BURST_MODE_AUTO_INC_ADDRESS | m_ucCurrentBank;
    }
    else if((ucBank != m_ucCurrentBank) && (ucReg >= 0x08))//Change to appropriate bank
    {
        m_ucCurrentBank = ucBank;

        pcBuffer[0] = 0x80;
        pcBuffer[1] = ucBank;

        SPI_SendWriteCmd(eBOARD_SSP_CS_C789, pcBuffer, 2, 1);
    }

    //-------------------------------------------------------------
    pcBuffer[0] = ucReg | 0x80;
    memcpy(&pcBuffer[1], pucData, uiSize);
    SPI_SendBurstWriteCmd(eBOARD_SSP_CS_C789, pcBuffer, (uiSize+1), 1);
    free(pcBuffer);
}

static eC789_EXEC_CODE dvC789_GetPanelIndex(ePANEL_ID ePanelId, PUINT8 pucIndex)
{
    UINT8 ucIndex = 0;

    if(ePanelId < ePANEL_ID_LAST)
    {
        for(ucIndex = 0; ucIndex < m_ucPanelTblCount; ucIndex++)
        {
            if(m_sPanelTable[ucIndex].ePanelId == ePanelId)
            {
                *pucIndex = ucIndex;
                return eC789_EXEC_CODE_PASS;
            }
        }
    }
    else
    {
        return eC789_EXEC_CODE_PANEL_ID_NOT_FOUND;
    }

    return eC789_EXEC_CODE_PANEL_TABEL_INDEX_NOT_FOUND;
}

static void dvC789_GetOutputTiming(const UINT8 ucPanelIndex)
{
    LOG_MSG(db_DV_GEO, "(func:%s, line:%d)Output Res[%d x %d %dHz]\n", __FUNCTION__, __LINE__,
                    m_sPanelTable[ucPanelIndex].sOutputTimingInfo.uiHActive,
                    m_sPanelTable[ucPanelIndex].sOutputTimingInfo.uiVActive,
                    m_sPanelTable[ucPanelIndex].sOutputTimingInfo.ucFrameRate);

    m_sDrvC789Info.sOutputTimingInfo.ucFrameRate = m_sPanelTable[ucPanelIndex].sOutputTimingInfo.ucFrameRate;
    m_sDrvC789Info.sOutputTimingInfo.uiPixClk = m_sPanelTable[ucPanelIndex].sOutputTimingInfo.uiPixClk;
    m_sDrvC789Info.sOutputTimingInfo.uiHStart = m_sPanelTable[ucPanelIndex].sOutputTimingInfo.uiHStart;
    m_sDrvC789Info.sOutputTimingInfo.uiHActive = m_sPanelTable[ucPanelIndex].sOutputTimingInfo.uiHActive;
    m_sDrvC789Info.sOutputTimingInfo.uiVStart = m_sPanelTable[ucPanelIndex].sOutputTimingInfo.uiVStart;
    m_sDrvC789Info.sOutputTimingInfo.uiVActive = m_sPanelTable[ucPanelIndex].sOutputTimingInfo.uiVActive;
    m_sDrvC789Info.sOutputTimingInfo.uiHTotal = m_sPanelTable[ucPanelIndex].sOutputTimingInfo.uiHTotal;
    m_sDrvC789Info.sOutputTimingInfo.uiVTotal = m_sPanelTable[ucPanelIndex].sOutputTimingInfo.uiVTotal;
    m_sDrvC789Info.sOutputTimingInfo.ucHSyncWidth = m_sPanelTable[ucPanelIndex].sOutputTimingInfo.ucHSyncWidth;
    m_sDrvC789Info.sOutputTimingInfo.ucVSyncWidth = m_sPanelTable[ucPanelIndex].sOutputTimingInfo.ucVSyncWidth;

    ulV_START = m_sDrvC789Info.sOutputTimingInfo.uiVStart;    //A70LV_Doulas_0042
}

static void dvC789_ConfigInput(void)        //A70LV_Doulas_0036
{
    LOG_MSG(db_DV_GEO , "(func:%s, line:%d)\n", __FUNCTION__, __LINE__);

    m_sDrvC789Info.ucRegISYCT = 0x31;    //A35G2_CDS_Simon_0003

//    dvC789_Write(B0_IP2SCT,     0x00, 0);

//    dvC789_Write(BN_MCT,        0x51, 0);    //Enable Image Output

    dvC789_Write(B3_ISYCT,      m_sDrvC789Info.ucRegISYCT);  //0x31    //A35G2_CDS_Simon_0003
    dvC789_Write(B3_AUTOFLD,    0x80);   //Field auto recognition
    dvC789_Write(B3_DIFCT,      0x83);   //Enable DE

    dvC789_Write(B3_IACTHST,    m_sDrvC789Info.sOutputTimingInfo.uiHStart-7);//-8);  //A70LV_Doulas_0045 modify
    dvC789_Write(B3_IACTHW,     m_sDrvC789Info.sOutputTimingInfo.uiHActive);
    dvC789_Write(B3_IACTVST,    m_sDrvC789Info.sOutputTimingInfo.uiVStart);
    dvC789_Write(B3_IACTVW,     m_sDrvC789Info.sOutputTimingInfo.uiVActive);

    dvC789_Write(B3_ISWP,	0x00);

    //Must reset Input Block
//    dvC789_Write(BN_RSTCT,      0x0C, 0);
//    dvC789_Write(B0_IP2SCT,     0x02, 0);

}

static void dvC789_ConfigOutput(void)
{

    UINT16 uiOHScycl;
    UINT16 uiOVScycl;
    //UINT16 uiOVScyclmin;  //A70LV_Doulas_0072 modify
    LOG_MSG(db_DV_GEO , "(func:%s, line:%d)\n", __FUNCTION__, __LINE__);

#ifdef C789_2D_NO_FRAME_LOCK   //A70LV_Doulas_0307 modify
	#ifdef Low_Latency_All
	if(!dvC789_Low_Latency_Get())	//G100_Clare_0008
	#endif	/*Low_Latency_All*/
	{
    	dvC789_Write(B2_OSYCT, 0x0311);
	}
	#ifdef Low_Latency_All
	else
	{
		dvC789_Write(B2_OSYCT, 0x0313);  //enable force sync reset
	}
	#endif	/*Low_Latency_All*/
#else
    dvC789_Write(B2_OSYCT, 0x0313);//0x0311);//A70LV_Doulas_0045 modify Add VS forced sync reset //Select AOI0 as active output
#endif

    uiOHScycl =  m_sDrvC789Info.sOutputTimingInfo.uiHTotal;
    uiOVScycl =  m_sDrvC789Info.sOutputTimingInfo.uiVTotal;
//    uiOVScyclmin = uiOVScycl - 10;

    dvC789_Write(B2_OHCYCL, (uiOHScycl - 2));

    //forced sync reset on
	#ifdef Low_Latency_All
	if(dvC789_Low_Latency_Get())	//G100_Clare_0008
	{
		dvC789_Write(B2_SYRDLY, 0x08);
	}
	else
	{
		dvC789_Write(B2_SYRDLY, 0x02);
	}
	#else
	dvC789_Write(B2_SYRDLY, 0x02);
	#endif	/*Low_Latency_All*/
    dvC789_Write(B2_FLDDLY, 0x01);
#ifdef C789_2D_NO_FRAME_LOCK   //A70LV_Doulas_0307 modify

	#ifdef Low_Latency_All
	if(!dvC789_Low_Latency_Get())	//G100_Clare_0008
	#endif	/*Low_Latency_All*/
    {
    	dvC789_Write(B2_OVCYCL, (uiOVScycl - 2));   //forced sync reset off
	}
	#ifdef Low_Latency_All
	else
    {
    	dvC789_Write(B2_OVCYCL, (uiOVScycl*1.1 - 2));
	}
	#endif	/*Low_Latency_All*/
#else
    dvC789_Write(B2_OVCYCL, (uiOVScycl*1.1 - 2));
#endif


    dvC789_Write(B2_OSYCT3, 0x80);//Trigger forced sync reset in unit of HS.

    dvC789_Write(B2_OHSCT, m_sDrvC789Info.sOutputTimingInfo.ucHSyncWidth/2 -1);
    dvC789_Write(B2_OVSCT, m_sDrvC789Info.sOutputTimingInfo.ucVSyncWidth-1);

    #if 1//0//A70LV_Doulas_0098     //A70LV_Doulas_0079
    //dvC789_Write(B2_SYRDLY, 0x02);    //A70LV_Doulas_0083 modify
    //dvC789_Write(B2_FLDDLY, 0x01);
    //dvC789_Write(B2_OVCYCL, (uiOVScycl*1.1 - 2));
    #else
    //forced sync reset off
    dvC789_Write(B2_SYRDLY, 0x00);
    //dvC789_Write(B2_OVCYCLMIN, (uiOVScyclmin - 1), 0);
    dvC789_Write(B2_FLDDLY, 0x20);
    #endif

    dvC789_Write(B2_OAOI0HST, m_sDrvC789Info.sOutputTimingInfo.uiHStart-7);  //A70LV_Doulas_0045 modify
    dvC789_Write(B2_OAOI0HEND, (m_sDrvC789Info.sOutputTimingInfo.uiHStart -7+    //A70LV_Doulas_0045 modify
                                   m_sDrvC789Info.sOutputTimingInfo.uiHActive));
    dvC789_Write(B2_OAOI0VST, m_sDrvC789Info.sOutputTimingInfo.uiVStart);
    dvC789_Write(B2_OAOI0VEND, (m_sDrvC789Info.sOutputTimingInfo.uiVStart +
                                   m_sDrvC789Info.sOutputTimingInfo.uiVActive));

    dvC789_Write(B2_OACTHST, m_sDrvC789Info.sOutputTimingInfo.uiHStart-7);   //A70LV_Doulas_0045 modify
    dvC789_Write(B2_OACTHW, m_sDrvC789Info.sOutputTimingInfo.uiHActive);
    dvC789_Write(B2_OACTVST, m_sDrvC789Info.sOutputTimingInfo.uiVStart+1);
    dvC789_Write(B2_OACTVW, m_sDrvC789Info.sOutputTimingInfo.uiVActive);

}

static void dvC789_ConfigClock(const UINT8 ucPoRefDiv,const UINT8 ucPoFbDiv)
{
    LOG_MSG(db_DV_GEO , "(func:%s, line:%d)\n", __FUNCTION__, __LINE__);

    dvC789_Write(B0_CLKCTRL, 0x00);
	if(Syscfg_Value_Get_Typeint(eC789_Output) == 0) //TTL
	{
        dvC789_Write(B0_CMOSOUTSETUP, /*0x00*/0x50);    //R70G2_Sammy
        dvC789_Write(B0_POCLKCT,  /*0x00*/0x01);        //R70G2_Sammy
    }
    else  //LVDS
    {
        dvC789_Write(B0_CMOSOUTSETUP, 0x00);    //R70G2_Sammy
        dvC789_Write(B0_POCLKCT,  0x00);        //R70G2_Sammy
    }
    if((m_sDrvC789Info.ePanelTimingId == ePANEL_ID_1080P_120HZ) ||
       (m_sDrvC789Info.ePanelTimingId == ePANEL_ID_WUXGA_120HZ))     //A70LV_Doulas_0094
    {
        dvC789_Write(B0_POPLLCT,  0x4D);          //Enable PLL Control    //PLL Frquency Range From 160-310 MHz
    }
    else
    {
        dvC789_Write(B0_POPLLCT,  0x49);          //Enable PLL Control    //PLL Frquency Range From 80-160 MHz
    }
    dvC789_Write(B0_POREFDIV, ucPoRefDiv);
    dvC789_Write(B0_POFBDIV,  ucPoFbDiv);

    if((m_sDrvC789Info.ePanelTimingId == ePANEL_ID_1080P_120HZ) ||
       (m_sDrvC789Info.ePanelTimingId == ePANEL_ID_WUXGA_120HZ))     //A70LV_Doulas_0094
    {
        dvC789_Write(B0_IPPLLCT,  0x8D);    //PLL 160-310 MHz
    }
    else
    {
        dvC789_Write(B0_IPPLLCT,  0x89);    //PLL 80-160 MHz
    }
    dvC789_Write(B0_IPREFDIV, ucPoRefDiv);
    dvC789_Write(B0_IPFBDIV,  ucPoFbDiv);



}

static void dvC789_ConfigLVDS(void)
{
    LOG_MSG(db_DV_GEO , "(func:%s, line:%d)\n", __FUNCTION__, __LINE__);
// Dual LVDS input

    dvC789_Write(B0_LVDSI1CT, 0x00);
    dvC789_Write(B0_LVDSI2CT, 0x00);
    if((m_sDrvC789Info.ePanelTimingId == ePANEL_ID_1080P_120HZ) ||
       (m_sDrvC789Info.ePanelTimingId == ePANEL_ID_WUXGA_120HZ))     //A70LV_Doulas_0094
    {
        dvC789_Write(B0_LVDSRXCT1, 0x8083);     //Input clock range 140-166 MHz
        dvC789_Write(B0_LVDSRXCT2, 0x8083);     //Input clock range 140-166 MHz
    }
    else
    {
        dvC789_Write(B0_LVDSRXCT1, 0x8081);     //Input clock range 60-100 MHz
        dvC789_Write(B0_LVDSRXCT2, 0x8081);     //Input clock range 60-100 MHz
    }

    MS_SLEEP(1);
    if((m_sDrvC789Info.ePanelTimingId == ePANEL_ID_1080P_120HZ) ||
       (m_sDrvC789Info.ePanelTimingId == ePANEL_ID_WUXGA_120HZ))     //A70LV_Doulas_0094
    {
        dvC789_Write(B0_LVDSRXCT1, 0xC083);     //Input clock range 140-166 MHz
        dvC789_Write(B0_LVDSRXCT2, 0xC083);     //Input clock range 140-166 MHz
    }
    else
    {
        dvC789_Write(B0_LVDSRXCT1, 0xC081);     //Input clock range 60-100 MHz
        dvC789_Write(B0_LVDSRXCT2, 0xC081);     //Input clock range 60-100 MHz
    }

    MS_SLEEP(50);

    if((m_sDrvC789Info.ePanelTimingId == ePANEL_ID_1080P_120HZ) ||
       (m_sDrvC789Info.ePanelTimingId == ePANEL_ID_WUXGA_120HZ))     //A70LV_Doulas_0094
    {
        dvC789_Write(B0_IPREFDIV, 0x08);    //A70LV_Doulas_0098
        dvC789_Write(B0_IPFBDIV, 0x10);
        dvC789_Write(B0_IPPLLCT, 0x4d);
    }
    else
    {
        dvC789_Write(B0_IPREFDIV, 0x04);
        dvC789_Write(B0_IPFBDIV, 0x08);
        dvC789_Write(B0_IPPLLCT, 0x49);
    }

    MS_SLEEP(1);
    dvC789_Write(BN_MCT, 0x51);
    dvC789_Write(BN_RSTCT, 0x04);

    MS_SLEEP(1);
    dvC789_Write(B0_LVDSI1CT, 0x03);
    dvC789_Write(B0_LVDSI2CT, 0x00);

// Dual LVDS output
    dvC789_Write(B0_LVDSOCLKCT,    0x01);
    dvC789_Write(B0_LVDSOSETUP,    0x07);//0x05);   //A70LV_Doulas_0045 modify
    dvC789_Write(B0_LVDSO1CT,      0x00);
    dvC789_Write(B0_LVDSO2CT,      0x00);
	if(Syscfg_Value_Get_Typeint(eC789_Output) == 0) //TTL
	{
	    dvC789_Write(B0_LVDSTXCT1,     0x00);//     ; FRANGECH1[1:0]=3(140-166 MHz)
	    dvC789_Write(B0_LVDSTXCT2,     0x00);//     ; FRANGECH1[1:0]=3(140-166 MHz)
	 	dvC789_Write(B2_OSWP,     	   0x04);
	}
	else
	{
	    if((m_sDrvC789Info.ePanelTimingId == ePANEL_ID_1080P_120HZ) ||
	       (m_sDrvC789Info.ePanelTimingId == ePANEL_ID_WUXGA_120HZ))     //A70LV_Doulas_0094
	    {
	        dvC789_Write(B0_LVDSTXCT1,     0x0B);//     ; FRANGECH1[1:0]=3(140-166 MHz)
	        dvC789_Write(B0_LVDSTXCT2,     0x0B);//     ; FRANGECH1[1:0]=3(140-166 MHz)
	    }
	    else
	    {
	        dvC789_Write(B0_LVDSTXCT1,     0x09);//    ; FRANGECH1[1:0]=1(60-100MHz)
	        dvC789_Write(B0_LVDSTXCT2,     0x09);//    ; FRANGECH2[1:0]=1(60-100MHz)
	    }
	}


}

void dvC789_ConfigMemAD(void)
{
    LOG_MSG(db_DV_GEO , "(func:%s, line:%d)\n", __FUNCTION__, __LINE__);
    //A70LV_Doulas_0079 modify

	#ifdef Low_Latency_All
	if(m_sDrvC789Info.sOutputTimingInfo.ucLow_Latency)
	{
		dvC789_Write(B9_OSFLD0, DEF_OSFLD0);
		dvC789_Write(B9_OSFLD1, DEF_OSFLD1);
		dvC789_Write(B9_OSFLD2, DEF_OSFLD2);
		dvC789_Write(B9_OSFLD3, DEF_OSFLD3);
	}
	else
	{
		dvC789_Write(B9_OSFLD0, DEF_OSFLD3);
		dvC789_Write(B9_OSFLD1, DEF_OSFLD0);
		dvC789_Write(B9_OSFLD2, DEF_OSFLD1);
		dvC789_Write(B9_OSFLD3, DEF_OSFLD2);
	}
	#else
	dvC789_Write(B9_OSFLD0, DEF_OSFLD3);
    dvC789_Write(B9_OSFLD1, DEF_OSFLD0);
    dvC789_Write(B9_OSFLD2, DEF_OSFLD1);
    dvC789_Write(B9_OSFLD3, DEF_OSFLD2);
	#endif	/*Low_Latency_All*/
    dvC789_Write(B9_ISFLD0, DEF_ISFLD0);
    dvC789_Write(B9_ISFLD1, DEF_ISFLD1);
    dvC789_Write(B9_ISFLD2, DEF_ISFLD2);
    dvC789_Write(B9_ISFLD3, DEF_ISFLD3);

    dvC789_Write(B9_IMWI, 0x00);



}

static void dvC789_ConfigBasicMemory(void)
{
    LOG_MSG(db_DV_GEO , "(func:%s, line:%d)\n", __FUNCTION__, __LINE__);

    dvC789_Write(B0_DDR3CT2_0,	0x22);
    dvC789_Write(B0_DDR3CT2_1,	0x44);
    dvC789_Write(B0_DDR3CT2_2,	0x22);
    dvC789_Write(B0_DDR3CT2_3,	0x77);
    dvC789_Write(B0_DDR3CT2_4,	0x07);
    dvC789_Write(B0_DDR3CT2_5,	0x30);

    dvC789_Write(B0_DDR3OCDDIP,	0x3f);
    dvC789_Write(B0_DDR3OCDDIN,	0x3f);
    dvC789_Write(B0_DDR3LVLCT,	0x00);
    dvC789_Write(B0_DDR3WRLVLPHCT,	0x0202);
    dvC789_Write(B0_DDR3WRTMGCT,	0x00);
    dvC789_Write(B0_DDR3RDTMGCT,	0x0000);
    dvC789_Write(B0_DDR3VREFCT,	0x09);
    dvC789_Write(B0_DDR3DLLCT,	0x0b);
    dvC789_Write(B0_DDR3ODTCT,	0x11);
    dvC789_Write(B0_DDR3ZQCT,	0x01);

//; C789BN registers
    dvC789_Write(BN_RGBNK,0x0a);
    dvC789_Write(BN_MCT,0x11);
    dvC789_Write(BN_RTCT,0x00);
    dvC789_Write(BN_DDR3CT,	0x00);
    dvC789_Write(BN_MEMCT,	0x00);
    dvC789_Write(BN_RSTCT,	0x00);

//; DDR3 Memory Initialization
    dvC789_Write(BN_DDR3CT,0x84);
    MS_SLEEP(1);//Sleep 1
    dvC789_Write(BN_DDR3CT,0xd4);//0xc4);
    MS_SLEEP(1);//Sleep 1
    dvC789_Write(BN_MEMCT,0x69);//0x61);


}

static void dvC789Ctrl_Init_Default_Register(void)
{
    UINT16 uiData = 0;
    UINT8  ucData = 0;

    LOG_MSG(db_DV_GEO , "(func:%s, line:%d)\n", __FUNCTION__, __LINE__);

    dvC789_WriteToBuffer(B0_CPUDT,	0x40);
    dvC789_WriteToBuffer(B0_CPUDTCTL,	0x60);

    dvC789_WriteToBuffer(B1_PGCTRL,	0x00);

    dvC789_WriteToBuffer(B1_SFLDTCTL,	0xa0);
    dvC789_WriteToBuffer(B1_SFLSTAT,	0x22);

    dvC789_WriteToBuffer(B1_GIOI,	0x32);
    dvC789_WriteToBuffer(B1_GIOS,	0x0F); //Use Serial Flash mode //A70LV_Larry_0139

    dvC789_WriteToBuffer(B2_OVPHRDT,	0x0a7fff);
    dvC789_Buffer_Flush();
    //if((m_sDrvC789Info.ePanelTimingId == ePANEL_ID_1080P_120HZ) ||
       //(m_sDrvC789Info.ePanelTimingId == ePANEL_ID_WUXGA_120HZ))     //A70LV_Doulas_0098
    //{

    uiData = (UINT16)dvC789_Read(B5_DTCT);   //A70LV_Larry_0177
    dvC789_Write(B5_DTCT,	uiData | 0x0C00); //A70LV_Larry_0177

    ucData = (UINT8)dvC789_Read(B5_WPACCT); //A70LV_Larry_0177
    dvC789_Write(B5_WPACCT,	ucData | 0x30); //A70LV_Larry_0177

    //}
    //else
    //{
        //dvC789_Write(B5_DTCT,	0x0008);
        //dvC789_Write(B5_WPACCT,	0x00);
    //}

#if 1
    dvC789_WriteToBuffer(B6_HLUT0,	0x02);
    dvC789_WriteToBuffer(B6_HLUT1,	0x04);
    dvC789_WriteToBuffer(B6_HLUT2,	0x06);
    dvC789_WriteToBuffer(B6_HLUT3,	0x07);
    dvC789_WriteToBuffer(B6_HLUT4,	0x07);
    dvC789_WriteToBuffer(B6_HLUT5,	0x05);
    dvC789_WriteToBuffer(B6_HLUT6,	0x03);
    dvC789_WriteToBuffer(B6_HLUT7,	0x00);
    dvC789_WriteToBuffer(B6_HLUT8,	0xfd);
    dvC789_WriteToBuffer(B6_HLUT9,	0xf9);
    dvC789_WriteToBuffer(B6_HLUT10,	0xf6);
    dvC789_WriteToBuffer(B6_HLUT11,	0xf4);
    dvC789_WriteToBuffer(B6_HLUT12,	0xf4);
    dvC789_WriteToBuffer(B6_HLUT13,	0xf6);
    dvC789_WriteToBuffer(B6_HLUT14,	0xfa);
    dvC789_WriteToBuffer(B6_HLUT15,	0x00);
    dvC789_WriteToBuffer(B6_HLUT16,	0x08);
    dvC789_WriteToBuffer(B6_HLUT17,	0x11);
    dvC789_WriteToBuffer(B6_HLUT18,	0x1b);
    dvC789_WriteToBuffer(B6_HLUT19,	0x25);
    dvC789_WriteToBuffer(B6_HLUT20,	0x2e);
    dvC789_WriteToBuffer(B6_HLUT21,	0x37);
    dvC789_WriteToBuffer(B6_HLUT22,	0x3c);
    dvC789_WriteToBuffer(B6_HLUT23,	0x40);
    dvC789_WriteToBuffer(B6_VLUT0,	0x02);
    dvC789_WriteToBuffer(B6_VLUT1,	0x04);
    dvC789_WriteToBuffer(B6_VLUT2,	0x06);
    dvC789_WriteToBuffer(B6_VLUT3,	0x07);
    dvC789_WriteToBuffer(B6_VLUT4,	0x07);
    dvC789_WriteToBuffer(B6_VLUT5,	0x05);
    dvC789_WriteToBuffer(B6_VLUT6,	0x03);
    dvC789_WriteToBuffer(B6_VLUT7,	0x00);
    dvC789_WriteToBuffer(B6_VLUT8,	0xfd);
    dvC789_WriteToBuffer(B6_VLUT9,	0xf9);
    dvC789_WriteToBuffer(B6_VLUT10,	0xf6);
    dvC789_WriteToBuffer(B6_VLUT11,	0xf4);
    dvC789_WriteToBuffer(B6_VLUT12,	0xf4);
    dvC789_WriteToBuffer(B6_VLUT13,	0xf6);
    dvC789_WriteToBuffer(B6_VLUT14,	0xfa);
    dvC789_WriteToBuffer(B6_VLUT15,	0x00);
    dvC789_WriteToBuffer(B6_VLUT16,	0x08);
    dvC789_WriteToBuffer(B6_VLUT17,	0x11);
    dvC789_WriteToBuffer(B6_VLUT18,	0x1b);
    dvC789_WriteToBuffer(B6_VLUT19,	0x25);
    dvC789_WriteToBuffer(B6_VLUT20,	0x2e);
    dvC789_WriteToBuffer(B6_VLUT21,	0x37);
    dvC789_WriteToBuffer(B6_VLUT22,	0x3c);
    dvC789_WriteToBuffer(B6_VLUT23,	0x40);
#endif /* 0 */

    dvC789_WriteToBuffer(B7_EGBCT, 0x0080); //A70LV_Larry_0318

/*
    dvC789_WriteToBuffer(B9_OSFLD1,	0x0980);
    dvC789_WriteToBuffer(B9_OSFLD2,	0x1300);
    dvC789_WriteToBuffer(B9_OSFLD3,	0x1c80);
    dvC789_WriteToBuffer(B9_ISFLD0,	0x00000000);
    dvC789_WriteToBuffer(B9_ISFLD1,	0x01300000);
    dvC789_WriteToBuffer(B9_ISFLD2,	0x02600000);
    dvC789_WriteToBuffer(B9_ISFLD3,	0x03900000);
*/
    dvC789_WriteToBuffer(B9_IPOSFLD0,	0x04c00b00);
    dvC789_WriteToBuffer(B9_IPOSFLD1,	0x04c00000);
    dvC789_WriteToBuffer(B9_IPOSFLD2,	0x04eee100);
    dvC789_WriteToBuffer(B9_IPOSFLD3,	0x04eed600);
    dvC789_WriteToBuffer(B9_IPISFLD0,	0x04c00b00);
    dvC789_WriteToBuffer(B9_IPISFLD1,	0x04c00000);
    dvC789_WriteToBuffer(B9_IPISFLD2,	0x04eee100);
    dvC789_WriteToBuffer(B9_IPISFLD3,	0x04eed600);
    dvC789_WriteToBuffer(B9_IPMWI,	0x8b);
    dvC789_WriteToBuffer(B9_IPYOFF,	0x02ee);
    dvC789_WriteToBuffer(B9_MVFLD,	0x057b8000);
    dvC789_Buffer_Flush();
}

static void dvC789Ctrl_Change_Panel_Init(void) //A70LV_Larry_0177
{
    //UINT16 uiData = 0;
    UINT8  ucData = 0;

    dvC789_Write(B0_CPUDT,	0x40);
    dvC789_Write(B0_CPUDTCTL,	0x60);

    dvC789_Write(B1_PGCTRL,	0x00);

    dvC789_Write(B1_SFLDTCTL,	0xa0);
    dvC789_Write(B1_SFLSTAT,	0x22);

    dvC789_Write(B1_GIOI,	0x32);
    dvC789_Write(B1_GIOS,	0x0F); //Use Serial Flash mode //A70LV_Larry_0139

    dvC789_Write(B2_OVPHRDT,	0x0a7fff);
    //if((m_sDrvC789Info.ePanelTimingId == ePANEL_ID_1080P_120HZ) ||
       //(m_sDrvC789Info.ePanelTimingId == ePANEL_ID_WUXGA_120HZ))     //A70LV_Doulas_0098
    //{

    //uiData = (UINT16)dvC789_Read(B5_DTCT);
    //dvC789_Write(B5_DTCT,	uiData | 0x0C00);

    ucData = (UINT8)dvC789_Read(B5_WPACCT);
    dvC789_Write(B5_WPACCT,	ucData | 0x30);

    //}
    //else
    //{
        //dvC789_Write(B5_DTCT,	0x0008);
        //dvC789_Write(B5_WPACCT,	0x00);
    //}

#if 0
    dvC789_Write(B6_HLUT0,	0x02);
    dvC789_Write(B6_HLUT1,	0x04);
    dvC789_Write(B6_HLUT2,	0x06);
    dvC789_Write(B6_HLUT3,	0x07);
    dvC789_Write(B6_HLUT4,	0x07);
    dvC789_Write(B6_HLUT5,	0x05);
    dvC789_Write(B6_HLUT6,	0x03);
    dvC789_Write(B6_HLUT7,	0x00);
    dvC789_Write(B6_HLUT8,	0xfd);
    dvC789_Write(B6_HLUT9,	0xf9);
    dvC789_Write(B6_HLUT10,	0xf6);
    dvC789_Write(B6_HLUT11,	0xf4);
    dvC789_Write(B6_HLUT12,	0xf4);
    dvC789_Write(B6_HLUT13,	0xf6);
    dvC789_Write(B6_HLUT14,	0xfa);
    dvC789_Write(B6_HLUT15,	0x00);
    dvC789_Write(B6_HLUT16,	0x08);
    dvC789_Write(B6_HLUT17,	0x11);
    dvC789_Write(B6_HLUT18,	0x1b);
    dvC789_Write(B6_HLUT19,	0x25);
    dvC789_Write(B6_HLUT20,	0x2e);
    dvC789_Write(B6_HLUT21,	0x37);
    dvC789_Write(B6_HLUT22,	0x3c);
    dvC789_Write(B6_HLUT23,	0x40);
    dvC789_Write(B6_VLUT0,	0x02);
    dvC789_Write(B6_VLUT1,	0x04);
    dvC789_Write(B6_VLUT2,	0x06);
    dvC789_Write(B6_VLUT3,	0x07);
    dvC789_Write(B6_VLUT4,	0x07);
    dvC789_Write(B6_VLUT5,	0x05);
    dvC789_Write(B6_VLUT6,	0x03);
    dvC789_Write(B6_VLUT7,	0x00);
    dvC789_Write(B6_VLUT8,	0xfd);
    dvC789_Write(B6_VLUT9,	0xf9);
    dvC789_Write(B6_VLUT10,	0xf6);
    dvC789_Write(B6_VLUT11,	0xf4);
    dvC789_Write(B6_VLUT12,	0xf4);
    dvC789_Write(B6_VLUT13,	0xf6);
    dvC789_Write(B6_VLUT14,	0xfa);
    dvC789_Write(B6_VLUT15,	0x00);
    dvC789_Write(B6_VLUT16,	0x08);
    dvC789_Write(B6_VLUT17,	0x11);
    dvC789_Write(B6_VLUT18,	0x1b);
    dvC789_Write(B6_VLUT19,	0x25);
    dvC789_Write(B6_VLUT20,	0x2e);
    dvC789_Write(B6_VLUT21,	0x37);
    dvC789_Write(B6_VLUT22,	0x3c);
    dvC789_Write(B6_VLUT23,	0x40);
#endif /* 0 */

/*
    dvC789_Write(B9_OSFLD1,	0x0980);
    dvC789_Write(B9_OSFLD2,	0x1300);
    dvC789_Write(B9_OSFLD3,	0x1c80);
    dvC789_Write(B9_ISFLD0,	0x00000000);
    dvC789_Write(B9_ISFLD1,	0x01300000);
    dvC789_Write(B9_ISFLD2,	0x02600000);
    dvC789_Write(B9_ISFLD3,	0x03900000);
*/
    dvC789_Write(B9_IPOSFLD0,	0x04c00b00);
    dvC789_Write(B9_IPOSFLD1,	0x04c00000);
    dvC789_Write(B9_IPOSFLD2,	0x04eee100);
    dvC789_Write(B9_IPOSFLD3,	0x04eed600);
    dvC789_Write(B9_IPISFLD0,	0x04c00b00);
    dvC789_Write(B9_IPISFLD1,	0x04c00000);
    dvC789_Write(B9_IPISFLD2,	0x04eee100);
    dvC789_Write(B9_IPISFLD3,	0x04eed600);
    dvC789_Write(B9_IPMWI,	0x8b);
    dvC789_Write(B9_IPYOFF,	0x02ee);
    dvC789_Write(B9_MVFLD,	0x057b8000);

#if 0
//; C789B10 registers
    dvC789_Write(B10_IFH,	0x0897);
    dvC789_Write(B10_IFV,	0x0465);
    dvC789_Write(B10_IHW,	0x2c);
    dvC789_Write(B10_IVW,	0x05);
    dvC789_Write(B10_IMFH,	0x0b92);
    dvC789_Write(B10_IMFV,	0x32dcb8);
//C789B10	IHVPHDF0	0000h
//C789B10	IHVPHDF1	0000h
    dvC789_Write(B10_FLDHIST,	0xff);
    dvC789_Write(B10_PSTAT,	0x02);
//C789B10	PVAL0	00h
//C789B10	PVAL1	00h
    dvC789_Write(B10_PH0,	0x3fff);
    dvC789_Write(B10_PV0,	0x3fff);
//C789B10	PH1	0000h
//C789B10	PV1	0000h
//C789B10	IPIXR	0000h
//C789B10	IPIXG	0000h
//C789B10	IPIXB	0000h
    dvC789_Write(B10_IPTCNT,	0x166f65);
    dvC789_Write(B10_IAPL0,	0x06645d8d);
    dvC789_Write(B10_IMAXFILTR,	0xfe);
//C789B10	IMINFILTR	00h
    dvC789_Write(B10_STATE,	0x29);
//C789B10	IPFLDMON	00h
    dvC789_Write(B10_IFACTHST,	0x00b9);
    dvC789_Write(B10_IFACTHW,	0x0780);
    dvC789_Write(B10_IFACTVST,	0x002a);
    dvC789_Write(B10_IFACTVW,	0x0438);
//    dvC789_Write(B10_OHISTGRAM,	0x1107085c);
    dvC789_Write(B10_OHISTGRAM_0,	0x5c);
    dvC789_Write(B10_OHISTGRAM_1,	0x08);
    dvC789_Write(B10_OHISTGRAM_2,	0x07);
    dvC789_Write(B10_OHISTGRAM_3,	0x11);
//C789B10	OHISTGRAM_4	00h
//C789B10	OHISTGRAM_5	00h
//C789B10	OHISTGRAM_6	00h
//C789B10	OHISTGRAM_7	00h
    dvC789_Write(B10_OAPL0,	0x06645d8d);
    dvC789_Write(B10_MAXFILTR,	0xfe);
//C789B10	MINFILTR	00h
    dvC789_Write(B10_OPTCNT,	0x166f60);
    dvC789_Write(B10_OFV,	0x0464);
//C789B10	IPOFV	0000h
    dvC789_Write(B10_OLNUM,	0x0101);
    dvC789_Write(B10_ILNUM,	0x0171);
//C789B10	LVDSRXMON	00h
    dvC789_Write(B10_DEVICECODE,	0x20);
#endif

}

#ifdef Low_Latency_All
eC789_EXEC_CODE dvC789_Init(const ePANEL_ID ePanelId, BOOL bLow_Latency)
#else
eC789_EXEC_CODE dvC789_Init(const ePANEL_ID ePanelId)
#endif	/*Low_Latency_All*/
{
    UINT8 ucTblIndex = 0;
    eC789_EXEC_CODE eExecCode = eC789_EXEC_CODE_PASS;

    if((eExecCode = dvC789_GetPanelIndex(ePanelId, &ucTblIndex)) != eC789_EXEC_CODE_PASS)
    {
        LOG_MSG(db_DV_GEO , "(func:%s, line:%d) C789 Get Panel Timing Index Fail = %d\n", __FUNCTION__, __LINE__, eExecCode);
        return eExecCode;
    }

#if 1
    dvC789_Write(BN_RSTCT, 0x01);//Soft Reset
    MS_SLEEP(120);
    dvC789_Write(BN_RSTCT, 0x00);
	#ifdef Low_Latency_All
	dvC789_Low_Latency_Set(bLow_Latency);
	#endif	/*Low_Latency_All*/
#endif

#ifdef C789_2D_NO_FRAME_LOCK   //A70LV_Doulas_0307
	#ifdef Low_Latency_All	//G100_Clare_0009
	if(!dvC789_Low_Latency_Get())	//G100_Clare_0008
	#endif	/*Low_Latency_All*/
	{
	    m_ucForcedSyncReset = FALSE;
	    m_uiH_Total = m_sDrvC789Info.sOutputTimingInfo.uiHTotal;
	    m_uiV_Total = m_sDrvC789Info.sOutputTimingInfo.uiVTotal;
	}
#endif

    m_sDrvC789Info.bInit = TRUE;
    m_sDrvC789Info.ePanelTimingId = ePanelId;
    m_sDrvC789Info.ulMclko_Freq = 396000000; //Should be the same with memory clock 12x66 = 792M (792/2)
    m_sDrvC789Info.ucRegISYCT = 0x31;    //A35G2_CDS_Simon_0003

    dvC789_GetOutputTiming(ucTblIndex);
    dvC789_ConfigClock(m_sPanelTable[ucTblIndex].sOutputTimingInfo.ucPoRefDiv,
                     m_sPanelTable[ucTblIndex].sOutputTimingInfo.ucPoFbDiv);

    dvC789_ConfigBasicMemory();
    dvC789Ctrl_Init_Default_Register();
    dvC789_ConfigMemAD();
    dvC789_ConfigInput();
    dvC789_ConfigOutput();
    dvC789_ConfigLVDS();
    dvC789_ConfigInput();       //A70LV_Doulas_0256 re-cover image error on the power on


    PS_WP_HW = 1920 ;
    PS_WP_VW = 1200 ;
    PM_BOARD = 0;
    PM_EBIAS_AREA_TEST = 0 ;
	PM_EBIAS_GAMMA[0] = 2.2;
	PM_EBIAS_GAMMA[1] = 2.2;
	PM_EBIAS_GAMMA[2] = 2.2;
    dvC789_Write( B9_EBIASSAD, DEF_EBIASSAD&0x1fffffff /*DEF_EBIASSAD&0x1fffffff*/ );
    dvC789_Write( B9_EBIASMWI, DEF_EBIASMWI&0xff /*DEF_EBIASMWI&0xff*/ );

    //OSD Init
    dvC789_Write( B1_OSDSAD, DEF_OSDSAD&0x1fffffff );
    dvC789_Write( B1_OSDMWI, DEF_OSDMWI&0xff );
    dvC789_Write( B0_BBWMWI, DEF_OSDMWI&0xff );
    dvC789_Write( B1_OSDCT, 0x40 );
    dvC789_Write( B1_OSDACTHW, 1920);
    dvC789_Write( B1_OSDACTVW, 1200);
    dvC789_OSDACTStartConfig(1);      //G100_Simon_0007
    dvC789_Write( B1_OSDMODE, 0x00 );     //G100_Simon_0007

    return eExecCode;        //A70LV_Doulas_0072
}

void dvC789_InputSwapSet(UINT8 ucVal)
{
   dvC789_Write(B3_ISWP,     ucVal);
}

void dvC789_TestPatternSet(UINT8 ucVal)
{
    switch(ucVal)
    {
        default:
        case 0:
            dvC789_Write(B1_PGCTRL,	0x00);
            break;

        case 1:     //Horizon Stripe
            dvC789_Write(B1_PGCTRL,	0x0e);

            dvC789_Write(B1_HLIMITMAX,	0x0001);

            dvC789_Write(B1_PGCOLR,	0xFF);
            dvC789_Write(B1_PGCOLG,	0xFF);
            dvC789_Write(B1_PGCOLB,	0xFF);
            dvC789_Write(B1_PGCOLLSR,	0x03);
            dvC789_Write(B1_PGCOLLSG,	0x03);
            dvC789_Write(B1_PGCOLLSB,	0x03);
            break;

        case 2:     //Vertical Stripe
            dvC789_Write(B1_PGCTRL,	0x0f);

            dvC789_Write(B1_VLIMITMAX,	0x0001);

            dvC789_Write(B1_PGCOLR,	0xFF);
            dvC789_Write(B1_PGCOLG,	0xFF);
            dvC789_Write(B1_PGCOLB,	0xFF);
            dvC789_Write(B1_PGCOLLSR,	0x03);
            dvC789_Write(B1_PGCOLLSG,	0x03);
            dvC789_Write(B1_PGCOLLSB,	0x03);
            break;

        case 3: //Horizon Color bar
            dvC789_Write(B1_PGCTRL,	0x01);
            dvC789_Write(B1_HLIMITMAX,(UINT32)(m_sDrvC789Info.sOutputTimingInfo.uiHActive/8));
            break;

        case 4: //Vertical Color bar
            dvC789_Write(B1_PGCTRL,	0x02);
            dvC789_Write(B1_VLIMITMAX,(UINT32)(m_sDrvC789Info.sOutputTimingInfo.uiVActive/8));
            break;

        case 5: //Horizon RAMP
            dvC789_Write(B1_PGCTRL,	0x03);
            dvC789_Write(B1_PGPTCTRL,0x00);
            dvC789_Write(B1_HLIMITMAX,0x3ff);  //A70LV_Doulas_0040 modify
            dvC789_Write(B1_HRAMPCNT,0x01);
            break;

        case 6: //Checker
            dvC789_Write(B1_PGCTRL,	0x05);
            dvC789_Write(B1_HLIMITMAX,(UINT32)(m_sDrvC789Info.sOutputTimingInfo.uiHActive/4));
            dvC789_Write(B1_VLIMITMAX,(UINT32)(m_sDrvC789Info.sOutputTimingInfo.uiVActive/4));

            dvC789_Write(B1_PGCOLR,0xff);
            dvC789_Write(B1_PGCOLG,0xff);
            dvC789_Write(B1_PGCOLB,0xff);

            dvC789_Write(B1_PGCOLLSR,0x03);
            dvC789_Write(B1_PGCOLLSG,0x03);
            dvC789_Write(B1_PGCOLLSB,0x03);
            break;
    }
}

#if 0
void dvC789_TestMode(void)
{
    UINT32 ulIFH,ulIFV;
    UINT32 ulIMFH,ulIMFV;
    UINT32 ulPH0,ulPV0;
    UINT32 ulVal;
    UINT32 ulHW,ulVW;

    ulIFH = dvC789_Read(B10_IFH);
    ulIFV = dvC789_Read(B10_IFV);
    ulIMFH = dvC789_Read(B10_IMFH);
    ulIMFV = dvC789_Read(B10_IMFV);


   // dvC789_Write(B3_PTH,0x04);    //A70LV_Doulas_0042
    dvC789_Write(B3_PMODE,0x81);//0x81);
  //  dvC789_Write(B3_PMODE,0xA6);
    ulVal = 0xA1;
 //   MS_SLEEP(120);
    //while(ulVal!=0x81)
    {
        ulVal = (INT32)dvC789_Read(B3_PMODE);   //A70LV_Doulas_0072 modify
    }


    ulPH0 = dvC789_Read(B10_IFACTHST);
    ulPV0 = dvC789_Read(B10_IFACTVST);

    ulHW = dvC789_Read(B10_IFACTHW);
    ulVW = dvC789_Read(B10_IFACTVW);

    //LOG_MSG(db_DV_GEO , "(func:%s, line:%d) C789 Get (%d,%d)(0x%x,0x%x)P(0x%x,0x%x)(0x%x)(0x%x)(0x%x,0x%x)\n", __FUNCTION__, __LINE__, ulIFH,ulIFV,ulIMFH,ulIMFV
            //,ulPH0,ulPV0,dvC789_Read(B2_OAOI0VST),ulVal,ulHW,ulVW);     //A70LV_Doulas_0157 debug
}
#endif

void dvC789_V_Start_Checking(void)  //A70LV_Doulas_0042
{
    UINT32 ulVal = 0;
    UINT32 ulVal2 = 0;
    UINT32 ulVal3 = 0;
    UINT8  ucCount = 0;
    UINT8 ucRTCT = 0;

//    dvC789_Write(B3_PMODE,0x81);
    for(;ucCount < 3; ucCount++)
    {
        ulVal = dvC789_Read(B10_IFACTVST);
        ulVal2 = dvC789_Read(B10_IFACTVST);
        ulVal3 = dvC789_Read(B10_IFACTVST);
        if((ulVal == ulVal2) && (ulVal == ulVal3) && (ulVal > 3) &&
		   ((ulVal + m_sDrvC789Info.sOutputTimingInfo.uiVActive) < m_sDrvC789Info.sOutputTimingInfo.uiVTotal))	//G100_Doulas_0084 Modify
        {
			ucCount = 4;
        }
        else
        {
            LOG_MSG(db_DV_GEO, "dvC789_V_Start_Checking Vstart Get (0x%x)(0x%x)(0x%x)\n",ulVal,ulVal2,ulVal3);
			ulVal = m_sDrvC789Info.sOutputTimingInfo.uiVStart;		//G100_Doulas_0084 Add
        }
		MS_SLEEP(1);	//G100_Doulas_0084 Add
    }
    if(ulVal != ulV_START)
    {
        ucRTCT = dvC789_Read(BN_RTCT);      //A70LV_Doulas_0120
        dvC789_WriteToBuffer(BN_RTCT, 0);           //A70LV_Doulas_0120
        dvC789_WriteToBuffer(B3_IACTVST,    ulVal);
        dvC789_WriteToBuffer(B2_OAOI0VST, ulVal);
        dvC789_WriteToBuffer(B2_OAOI0VEND, (ulVal + m_sDrvC789Info.sOutputTimingInfo.uiVActive));
        dvC789_WriteToBuffer(B2_OACTVST, ulVal+1);
        dvC789_WriteToBuffer(BN_RTCT, ucRTCT);      //A70LV_Doulas_0120
        dvC789_Buffer_Flush();
        ulV_START = ulVal;
    }

    LOG_MSG(db_DV_GEO, "(func:%s, line:%d) C789 Vstart Get (0x%x)\n", __FUNCTION__, __LINE__, ulVal);
}

void dvC789_ForcedSyncResetValueSet(UINT8 ucForcedSyncReset,UINT16 uiH_Total,UINT16 uiV_Total)     //A70LV_Doulas_0079
{
#ifdef C789_2D_NO_FRAME_LOCK   //A70LV_Doulas_0307 modify
    #if 1
		#ifdef Low_Latency_All	//G100_Clare_0009
		if(!dvC789_Low_Latency_Get())	//G100_Clare_0008
		#endif	/*Low_Latency_All*/
		{
	        if(m_ucForcedSyncReset == ucForcedSyncReset)
	        {
	            m_ucOutputChanged = FALSE;
	        }
	        else
	        {
	            m_ucOutputChanged = TRUE;
	        }

	        //m_ucForcedSyncReset = ucForcedSyncReset;   //A35G2_CDS_Simon_0033 move to bottom
	        //m_uiH_Total = uiH_Total;
	        //m_uiV_Total = uiV_Total;
	        //m_uiH_Total = m_uiH_Total;   //G100_Simon_0060 avoid Cppcheck warning
	        //m_uiV_Total = m_uiV_Total;   //G100_Simon_0060 avoid Cppcheck warning

	        if(ucForcedSyncReset)
	        {
	            dvC789_WriteToBuffer(B2_OSYCT, 0x0313);
	            if((m_sDrvC789Info.ePanelTimingId == ePANEL_ID_1080P_120HZ) ||
	               (m_sDrvC789Info.ePanelTimingId == ePANEL_ID_WUXGA_120HZ))    //A70LV_Doulas_0377 Modify//A70LV_Doulas_0154
	            {
	                dvC789_WriteToBuffer(B2_OHCYCL, (uiH_Total*2 - 2));
	            }
	            else
	            {
	                dvC789_WriteToBuffer(B2_OHCYCL, (uiH_Total - 2));
	            }
	            dvC789_WriteToBuffer(B2_OVCYCL, (uiV_Total*1.1 - 2));

	            //dvC789_Write(B2_SYRDLY, 0x02);        //A70LV_Doulas_0154 remove
	            dvC789_WriteToBuffer(B2_FLDDLY, 0x01);
                dvC789_Buffer_Flush();
	            LOG_MSG(db_DV_GEO , "(func:%s,%d) dvC789_ForcedSyncResetValueSet (%d,%d)\n", __FUNCTION__, __LINE__, uiH_Total,uiV_Total);   //A70LV_Doulas_0154
	        }
	        else
	        {
	            dvC789_WriteToBuffer(B2_OSYCT, 0x0311);

	            if((m_sDrvC789Info.ePanelTimingId == ePANEL_ID_1080P_120HZ) ||
	               (m_sDrvC789Info.ePanelTimingId == ePANEL_ID_WUXGA_120HZ))    //A70LV_Doulas_0377 Modify
	            {
	                dvC789_WriteToBuffer(B2_OHCYCL, (uiH_Total*2 - 2));
	                dvC789_WriteToBuffer(B2_OVCYCL, (uiV_Total - 2));
	            }
	            else
	            {
	                uiH_Total = m_sDrvC789Info.sOutputTimingInfo.uiHTotal;
	                uiV_Total = m_sDrvC789Info.sOutputTimingInfo.uiVTotal;
	                dvC789_WriteToBuffer(B2_OHCYCL, (uiH_Total - 2));
	                dvC789_WriteToBuffer(B2_OVCYCL, (uiV_Total - 2));
	            }
				dvC789_WriteToBuffer(B2_SYRDLY, 0x00);
                dvC789_Buffer_Flush();
                LOG_MSG(db_DV_GEO , "(func:%s,%d) dvC789_ForcedSyncResetValueSet (%d,%d)\n", __FUNCTION__, __LINE__, uiH_Total,uiV_Total);
	        }
		}
		#ifdef Low_Latency_All	//G100_Clare_0009
		else	//G100_Clare_0008, modify, >>>
		{
			if(ucForcedSyncReset)
	        {
	            dvC789_WriteToBuffer(B2_OSYCT, 0x0313);
	            if((m_sDrvC789Info.ePanelTimingId == ePANEL_ID_1080P_120HZ) ||
	               (m_sDrvC789Info.ePanelTimingId == ePANEL_ID_WUXGA_120HZ))    //A70LV_Doulas_0377 Modify//A70LV_Doulas_0154
	            {
	                dvC789_WriteToBuffer(B2_OHCYCL, (uiH_Total*2 - 2));
	            }
	            else
	            {
	                dvC789_WriteToBuffer(B2_OHCYCL, (uiH_Total - 2));
	            }
	            dvC789_WriteToBuffer(B2_OVCYCL, (uiV_Total*1.1 - 2));

	            //dvC789_Write(B2_SYRDLY, 0x02);        //A70LV_Doulas_0154 remove
	            dvC789_WriteToBuffer(B2_FLDDLY, 0x01);
                dvC789_Buffer_Flush();
	            LOG_MSG(db_DV_GEO , "(func:%s,%d)  (%d,%d)\n", __FUNCTION__, __LINE__, uiH_Total,uiV_Total);   //A70LV_Doulas_0154
	        }
	        else
	        {
	            dvC789_WriteToBuffer(B2_OSYCT, 0x0313);
	            if((m_sDrvC789Info.ePanelTimingId == ePANEL_ID_1080P_120HZ) ||
	               (m_sDrvC789Info.ePanelTimingId == ePANEL_ID_WUXGA_120HZ))    //A70LV_Doulas_0377 Modify//A70LV_Doulas_0154
	            {
	                dvC789_WriteToBuffer(B2_OHCYCL, (uiH_Total*2 - 2));
	            }
	            else
	            {
	                dvC789_WriteToBuffer(B2_OHCYCL, (uiH_Total - 2));
	            }
	            dvC789_WriteToBuffer(B2_OVCYCL, (uiV_Total - 2));
	       		dvC789_WriteToBuffer(B2_SYRDLY, 0x08);
                dvC789_Buffer_Flush();
                LOG_MSG(db_DV_GEO , "(func:%s,%d) dvC789_ForcedSyncResetValueSet (%d,%d)\n", __FUNCTION__, __LINE__, uiH_Total,uiV_Total);
	        }	//G100_Clare_0008, modify, <<<
		}
		#endif	/*Low_Latency_All*/
    #else
        if((m_ucForcedSyncReset == ucForcedSyncReset) &&
           (m_uiH_Total == uiH_Total) &&
           (m_uiV_Total == uiV_Total))
        {
            m_ucOutputChanged = FALSE;
        }
        else
        {
            m_ucOutputChanged = TRUE;
        }

        m_ucForcedSyncReset = ucForcedSyncReset;
        m_uiH_Total = uiH_Total;
        m_uiV_Total = uiV_Total;

        if(ucForcedSyncReset)
        {
            dvC789_Write(B2_OSYCT, 0x0313);
            if(m_sDrvC789Info.ePanelTimingId == ePANEL_ID_1080P_120HZ)  //A70LV_Doulas_0154
            {
                dvC789_Write(B2_OHCYCL, (uiH_Total*2 - 2));
            }
            else
            {
                dvC789_Write(B2_OHCYCL, (uiH_Total - 2));
            }
            dvC789_Write(B2_OVCYCL, (uiV_Total*1.1 - 2));

            //dvC789_Write(B2_SYRDLY, 0x02);        //A70LV_Doulas_0154 remove
            dvC789_Write(B2_FLDDLY, 0x01);
            LOG_MSG(db_DV_GEO , "(func:%s,%d) dvC789_ForcedSyncResetValueSet (%d,%d)\n", __FUNCTION__, __LINE__, uiH_Total,uiV_Total);   //A70LV_Doulas_0154
        }
        else
        {
            dvC789_Write(B2_OSYCT, 0x0311);

            if(m_sDrvC789Info.ePanelTimingId == ePANEL_ID_1080P_120HZ)
            {
                dvC789_Write(B2_OHCYCL, (uiH_Total*2 - 2));
                dvC789_Write(B2_OVCYCL, (uiV_Total - 2));
            }
            else
            {
                dvC789_Write(B2_OHCYCL, (uiH_Total - 2));
                dvC789_Write(B2_OVCYCL, (uiV_Total - 2));
            }

            dvC789_Write(B2_SYRDLY, 0x00);
        }
    #endif
#else
        if(ucForcedSyncReset)
        {
            dvC789_Write(B2_OSYCT, 0x0313);
            if((m_sDrvC789Info.ePanelTimingId == ePANEL_ID_1080P_120HZ) ||
               (m_sDrvC789Info.ePanelTimingId == ePANEL_ID_WUXGA_120HZ))    //A70LV_Doulas_0377 Modify//A70LV_Doulas_0154
            {
                dvC789_Write(B2_OHCYCL, (uiH_Total*2 - 2));
            }
            else
            {
                dvC789_Write(B2_OHCYCL, (uiH_Total - 2));
            }
            dvC789_Write(B2_OVCYCL, (uiV_Total*1.1 - 2));

            //dvC789_Write(B2_SYRDLY, 0x02);        //A70LV_Doulas_0154 remove
            dvC789_Write(B2_FLDDLY, 0x01);
            LOG_MSG(db_DV_GEO , "(func:%s,%d) dvC789_ForcedSyncResetValueSet (%d,%d)\n", __FUNCTION__, __LINE__, uiH_Total,uiV_Total);   //A70LV_Doulas_0154
        }
        else
        {
            dvC789_Write(B2_OSYCT, 0x0313);
            if((m_sDrvC789Info.ePanelTimingId == ePANEL_ID_1080P_120HZ) ||
               (m_sDrvC789Info.ePanelTimingId == ePANEL_ID_WUXGA_120HZ))    //A70LV_Doulas_0377 Modify//A70LV_Doulas_0154
            {
                dvC789_Write(B2_OHCYCL, (uiH_Total*2 - 2));
            }
            else
            {
                dvC789_Write(B2_OHCYCL, (uiH_Total - 2));
            }
            dvC789_Write(B2_OVCYCL, (uiV_Total - 2));
            dvC789_Write(B2_SYRDLY, 0x00);
        }
#endif

    m_ucForcedSyncReset = ucForcedSyncReset;   //A35G2_CDS_Simon_0033
    m_uiH_Total = uiH_Total;
    m_uiV_Total = uiV_Total;

}

void dvC789_OutputEnableSet(UINT8 ucEnable)
{
    dvC789_Write(B2_OFILL, 0x000000);

    if(!ucEnable)
    {
        dvC789_Write(B2_OIMGCT, 0x01);
    }
    else
    {
        dvC789_Write(B2_OIMGCT, 0x00);
    }

    //dvC789_Write(BN_MCT, 0x50+ucEnable);      //A70LV_Doulas_0326 remove
    ucC789_OutputEnable = ucEnable;
}

UINT8 dvC789_OutputEnableGet(void)
{
    return ucC789_OutputEnable;
}

eC789_EXEC_CODE dvC789_Change_Panel(const ePANEL_ID ePanelId)       //A70LV_Doulas_0098
{
    UINT8 ucTblIndex = 0;
    eC789_EXEC_CODE eExecCode = eC789_EXEC_CODE_PASS;
    UINT8 aucLUT[48] = {0};
    UINT8 aucBlendingEdge[30] = {0};
    UINT8 ucRTCT = 0;
    UINT16 uiEGBCT  = 0;
    UINT16 uiDTCT  = 0; //A70LV_Larry_0308
    UINT16 uiUCCT  = 0;
//    UINT8 u8Idx;

    if((eExecCode = dvC789_GetPanelIndex(ePanelId, &ucTblIndex)) != eC789_EXEC_CODE_PASS)
    {
        LOG_MSG(db_DV_GEO , "(func:%s, line:%d) C789 Get Panel Timing Index Fail = %d\n", __FUNCTION__, __LINE__, eExecCode);
        return eExecCode;
    }



    uiDTCT = (UINT16)dvC789_Read(B5_DTCT); //A70LV_Larry_0308

    if(ucColorUniformityEnable)
    {
        uiUCCT = (UINT16)dvC789_Read(B5_UCCT);
    }

    dvC789_BurstRead_AddInc(B6_HLUT0, 48, aucLUT); //A70LV_Larry_0177
//    for(u8Idx=0; u8Idx<48; u8Idx++)
//    {
//        aucLUT[u8Idx] = (UINT8)dvC789_Read(B6_HLUT0+0x0100*u8Idx);
//    }
    dvC789_BurstRead_AddInc(B7_LEGBHST, 30, aucBlendingEdge); //A70LV_Larry_0177
//    for(u8Idx=0; u8Idx<30; u8Idx++)
//    {
//        aucBlendingEdge[u8Idx] = (UINT8)dvC789_Read(B7_LEGBHST+0x0100*u8Idx);
//    }

    uiEGBCT = (UINT16)dvC789_Read(B7_EGBCT); //A70LV_Larry_0177

    ucRTCT = (UINT8)dvC789_Read(BN_RTCT); //A70LV_Larry_0177
    dvC789_Write(BN_RTCT,0x00); //A70LV_Larry_0177

#if 1
    dvC789_Write(BN_RSTCT, 0x01);//Soft Reset
    MS_SLEEP(120);
    dvC789_Write(BN_RSTCT, 0x00);
#endif



    m_sDrvC789Info.bInit = TRUE;
    m_sDrvC789Info.ePanelTimingId = ePanelId;
    m_sDrvC789Info.ulMclko_Freq = 396000000;   //Should be the same with memory clock 12x66 = 792M (792/2)
    m_sDrvC789Info.ucRegISYCT = 0x31;    //A35G2_CDS_Simon_0003

    dvC789_GetOutputTiming(ucTblIndex);
    dvC789_ConfigClock(m_sPanelTable[ucTblIndex].sOutputTimingInfo.ucPoRefDiv,
                     m_sPanelTable[ucTblIndex].sOutputTimingInfo.ucPoFbDiv);

    PS_WP_HW = m_sPanelTable[ucTblIndex].sOutputTimingInfo.uiHActive ;  //A35G2_CDS_Simon_0023
    PS_WP_VW = m_sPanelTable[ucTblIndex].sOutputTimingInfo.uiVActive ;

    dvC789_ConfigBasicMemory();

    dvC789Ctrl_Change_Panel_Init(); //A70LV_Larry_0177



    dvC789_BurstWrite_AddInc(B6_HLUT0, 48, aucLUT); //A70LV_Larry_0177
//    for(u8Idx=0; u8Idx<48; u8Idx++)
//    {
//        dvC789_Write(B6_HLUT0+0x0100*u8Idx,aucLUT[u8Idx]);
//    }
    dvC789_BurstWrite_AddInc(B7_LEGBHST, 30, aucBlendingEdge); //A70LV_Larry_0177
//    for(u8Idx=0; u8Idx<30; u8Idx++)
//    {
//        dvC789_Write(B7_LEGBHST+0x0100*u8Idx,aucBlendingEdge[u8Idx]);
//    }
    dvC789_Write(B7_EGBCT, uiEGBCT); //A70LV_Larry_0177




    dvC789_ConfigMemAD();
    dvC789_ConfigInput();
    dvC789_ConfigOutput();
    dvC789_ConfigLVDS();



    dvC789_Write(B5_DTCT, uiDTCT); //A70LV_Larry_0308

    dvC789_Write(B9_EGBDBDSAD, TWIST_DEF_BLEND_ADDRESS);  //A70LV_Larry_0456

    if(ucColorUniformityEnable)
    {
        dvC789_Write(B5_UCCT,   uiUCCT);
    }

    dvC789_Write(BN_RTCT, ucRTCT); //A70LV_Larry_0177

    dvC789_OSDACTStartConfig(FALSE);  //A35G2_CDS_Simon_0023

    LOG_MSG(db_DV_GEO , "(func:%s, line:%d) C789 Panel Change = %d\n", __FUNCTION__, __LINE__, eExecCode);
    m_ucOutputChanged = TRUE;   //A70LV_Doulas_0307
    return eExecCode;
}

void dvC789_FlashEraseSector(UINT32 ulSflAd, UINT32 ulDataSize) //A70LV_Larry_0139
{
    UINT32 ulEraseAddrStart = ulSflAd;
    UINT32 ulEraseAddrEnd   = ulSflAd + ulDataSize;
    UINT8  ucSFLSTAT;
    UINT8  ucSFLRDSR;

    UINT16 ucRetryCount = 200;

    if((ulEraseAddrStart & 0x0000FFFF) || (ulEraseAddrEnd & 0x0000FFFF))
    {
        LOG_MSG(db_DV_GEO, "Not matching 64k\n");
        return;
    }



    for(ulEraseAddrStart = ulSflAd; ulEraseAddrStart < ulEraseAddrEnd; ulEraseAddrStart += 0x00010000) //64k
    {
        dvC789_Write(B1_SFLSZSEL, 0x01);
        dvC789_Write(B1_SFLCT, 0x00);
        dvC789_Write(B1_SFLMODE, 0x80);
        dvC789_Write(B1_SFLMODE, 0x13);
        dvC789_Write(B1_SFLCT, 0x00);
        dvC789_Write(B1_SFLCMDWR, 0x12);  //Set the page write command to the serial Flash connected
        dvC789_Write(B1_SFLCMDRD, 0x0C);  //Set a read or fast read command to be executed for the serial Flash connected
        dvC789_Write(B1_SFLCMDER, 0xDC);  //Set a selector erase or bulk (chip) erase command to be executed for the serial Flash connected
        dvC789_Write(B1_SFLCMDWREN, 0x06);  //Set the value of WREN command of the serial Flash connected
        dvC789_Write(B1_SFLCMDRDSR, 0x05);  //Set the value of RDSR command of the serial Flash connected
        dvC789_Write(B1_SFLAD, ulEraseAddrStart);  //Specify the serial Flash address. After accessing, auto increment starts
        dvC789_Write(B1_SFLCT, 0x04);

//A70LV_Larry_0336 [[
        ucRetryCount = 200;
        //
        do
        {
            ucSFLSTAT = dvC789_Read(B1_SFLSTAT); //A70LV_Larry_0054
            MS_SLEEP(10);
        } while (((ucSFLSTAT & 0x02) != 0x00) && (ucRetryCount--));

        ASSERT(ucRetryCount != 0);

        //


        ucRetryCount = 200;
        //
        do
        {
            ucSFLRDSR = dvC789_Read(B1_SFLRDSR); //A70LV_Larry_0054
            MS_SLEEP(10);
        } while (((ucSFLRDSR & 0x03) != 0x00) && (ucRetryCount--));

        ASSERT(ucRetryCount != 0);

        //
//A70LV_Larry_0336 ]]
    }
//A70LV_Larry_0332 ]]


    return;
}

void dvC789_DMA_Transfer(UINT32 ulAddress, UINT32 ulSize, UINT8 ucRegsel, UINT32 ulDmaCnt) //A70LV_Larry_0139
{
    UINT8  ucSFLSTAT = 0;
    UINT8  ucREAD_SFLSTAT = 0;
    UINT16 ucRetryCount = 1000;

    if(ulDmaCnt == eC789_SFLCT_DMA_REG2FLASH) //A70LV_Larry_0227
    {
        ucSFLSTAT = 0x03; //1:WBUSY 0;DMA BUSY
    }
    else
    {
        ucSFLSTAT = 0x01;
    }



    dvC789_Write(B1_SFLMODE,0x80);
    dvC789_Write(B1_SFLMODE,0x15);
    dvC789_Write(B1_SFLCT,0x00);
    dvC789_Write(B1_SFLCMDWR,0x12);
    dvC789_Write(B1_SFLCMDRD,0x0C);
    dvC789_Write(B1_SFLCMDWREN,0x06);
    dvC789_Write(B1_SFLCMDRDSR,0x05);
    dvC789_Write(B1_SFLAD,ulAddress);
    dvC789_Write(B1_SFLCNT,ulSize);

    if(ucRegsel != eC789_SFLREGSEL_NA) //A70LV_Larry_0174
    {
        dvC789_Write(B1_SFLREGSEL,ucRegsel);
    }

    dvC789_Write(B1_SFLSZSEL,0x01);
    dvC789_Write(B1_SFLCT,ulDmaCnt);

//A70LV_Larry_0336 [[
    //if((ulDmaCnt == eC789_SFLCT_DMA_REG2FLASH) && (ucRegsel == eC789_SFLREGSEL_DCL))
    //{
        //MS_SLEEP(1000);
    //}

    do
    {
		// NOP
        MS_SLEEP(10);
        ucREAD_SFLSTAT = dvC789_Read(B1_SFLSTAT);

	}while(((ucREAD_SFLSTAT & ucSFLSTAT) != 0) && (ucRetryCount--)) ; // busy waiting

    ASSERT(ucRetryCount != 0);

    dvC789_Write(BN_RGBNK, 0x00); //A70LV_Larry_0318
    dvC789_Write(B1_SFLCT,0x00); //A70LV_Larry_0227


}

void dvC789_FlashWrite(UINT32 ulAddress, UINT16 uiSize, UINT8 *pucData) //A70LV_Larry_0174
{
    UINT8   ucData = 0;
    UINT16  uiCount = 0;
    UINT8   ucSFLDTCTL = 0xff;
    UINT8   ucSFLSTAT = 0xff;



    dvC789_Write(B1_SFLCT, 0x00);
    dvC789_Write(B1_SFLMODE,0x80);
    dvC789_Write(B1_SFLMODE,0x15);

    dvC789_Write(B1_SFLCMDWR,0x12);
    dvC789_Write(B1_SFLCMDWREN,0x06);
    dvC789_Write(B1_SFLCMDRDSR,0x05);
    dvC789_Write(B1_SFLSZSEL,0x01);
    dvC789_Write(B1_SFLAD,ulAddress);

//A70LV_Larry_0336 [[
    uiCount = 200;

    //
    do
    {
        MS_SLEEP(10);
        ucSFLDTCTL = dvC789_Read(B1_SFLDTCTL);
    }
    while(((ucSFLDTCTL & 0x20) == 0x00) && (uiCount--));

    ASSERT(uiCount != 0);
    //

    uiCount = 200;

    //
    do
    {
        MS_SLEEP(10);
        ucSFLSTAT = dvC789_Read(B1_SFLSTAT);
    }
    while (((ucSFLSTAT & 0x02) != 0x00) && (uiCount--));

    ASSERT(uiCount != 0);
    //
//A70LV_Larry_0336 ]]
#if 0
    dvC789_BurstWrite_FixedAdd(B1_SFLDT, uiSize, pucData);  //data shift issue
#else
    for(uiCount = 0; uiCount<uiSize; uiCount++)
    {
        ucData = pucData[uiCount];
//        dvC789_Write(B1_SFLDT, ucData);
        dvC789_WriteToBuffer(B1_SFLDT, ucData); //G100_Owen_0026
    }
    dvC789_Buffer_Flush();  //G100_Owen_0026
#endif


}

void dvC789_FlashRead(UINT32 ulAddress, UINT16 uiSize, UINT8 *pucData) //A70LV_Larry_0174
{
    UINT8   ucData = 0;
    UINT8   ucSFLDTCTL = 0xff;
    UINT16  uiCount = 0;



    dvC789_Write(B1_SFLCT, 0x00);
    dvC789_Write(B1_SFLMODE,0x80);
    dvC789_Write(B1_SFLMODE,0x15);
    dvC789_Write(B1_SFLCMDRD,0x0c);
    dvC789_Write(B1_SFLSZSEL,0x01);
    dvC789_Write(B1_SFLAD,ulAddress);
    dvC789_Write(B1_SFLDTCTL,0x01);

//A70LV_Larry_0336 [[
    uiCount = 200;
    //
    do
    {
        MS_SLEEP(10);
        ucSFLDTCTL = dvC789_Read(B1_SFLDTCTL);
    } while (((ucSFLDTCTL & 0x80)) && (uiCount--));

    ASSERT(uiCount != 0);

    //
//A70LV_Larry_0336 ]]
#if 1
    dvC789_BurstRead_FixedAdd(B1_SFLDT, uiSize, pucData);
#else
    for(uiCount = 0; uiCount<uiSize; uiCount++)
    {
        ucData = dvC789_Read(B1_SFLDT);
        pucData[uiCount] = ucData;
    }
#endif

}

void dvC789_FrmaeDelaySet(UINT8 ucValue)       //A70LV_Doulas_0154
{
	#ifdef Low_Latency_All
	if(m_sDrvC789Info.sOutputTimingInfo.ucLow_Latency)
	{
	}
	else
	{
        if(m_ucFrmaeDelay > ucValue)
        {
            UINT8 ucOSYCT3 = 0;
            UINT8 ucRTCT = 0;

            ucRTCT = dvC789_Read(BN_RTCT);
            ucOSYCT3 = dvC789_Read(B2_OSYCT3);

            if(ucOSYCT3 & 0x80)
            {
                dvC789_Write(BN_RTCT, 0x03);
                dvC789_Write(B2_OSYCT3, 0x00);
                MS_SLEEP(45);
            }

            dvC789_Write(B2_SYRDLY, ucValue);

            if(ucOSYCT3 & 0x80)
            {
                dvC789_Write(B2_OSYCT3, ucOSYCT3);
                MS_SLEEP(45);
                dvC789_Write(BN_RTCT, ucRTCT);
            }
        }
        else
        {
            dvC789_Write(B2_SYRDLY, ucValue);
        }

        m_ucFrmaeDelay = ucValue;
	}
	#else
		dvC789_Write(B2_SYRDLY, ucValue);
	#endif	/*Low_Latency_All*/
}

UINT16 dvC789_Output_V_Total_Get(void)   //A70LV_Doulas_0154
{
    return m_sDrvC789Info.sOutputTimingInfo.uiVTotal;
}

void dvC789_RegDump(void)
{
#if 1	//G100_Doulas_0027 test
	UINT16 wCount = 0;
	UINT32 dwData = 0;

	for(wCount = 0; wCount < C789REG_NUMBER;wCount++)
		{
			dwData = dvC789_Read(m_sICHIPC789_DUMP[wCount].ulAddress);
			switch(m_sICHIPC789_DUMP[wCount].ulAddress & 0xFF)
			{
				case 1:
					LOG_MSG(db_ALWAYS, "C789%s %02xh\r\n",	m_sICHIPC789_DUMP[wCount].cReg, dwData);
					break;
				case 2:
					LOG_MSG(db_ALWAYS, "C789%s %04xh\r\n",	m_sICHIPC789_DUMP[wCount].cReg, dwData);
					break;
				case 3:
					LOG_MSG(db_ALWAYS, "C789%s %06xh\r\n",	m_sICHIPC789_DUMP[wCount].cReg, dwData);
					break;
				case 4:
					LOG_MSG(db_ALWAYS, "C789%s %08xh\r\n",	m_sICHIPC789_DUMP[wCount].cReg, dwData);
					break;
				default:
					break;
			}
			MS_SLEEP(3);
		}


#else
    //UINT16 wCount = 0;
    UINT32 dwData = 0;

    sICHIP_DUMP* m_pc789Reg;

    m_pc789Reg = (sICHIP_DUMP*)ICHIPC789_DUMP_ADDR;

    if(m_pc789Reg->ulAddress == 0x12345678)
    {
        LOG_MSG(db_ALWAYS, "C789 %s\r\n",  m_pc789Reg->cReg);

        m_pc789Reg++;

        while(1)
        {
            if(m_pc789Reg->ulAddress == 0xFFFFFFFF)
            {
                LOG_MSG(db_ALWAYS, "C789 %s\r\n",  m_pc789Reg->cReg);
                return;
            }
            else
            {
                dwData = dvC789_Read(m_pc789Reg->ulAddress);

                switch(m_pc789Reg->ulAddress & 0xFF)
                {
                    case 1:
                        LOG_MSG(db_ALWAYS, "C789%s %02xh\r\n",  m_pc789Reg->cReg, dwData);
                        break;
                    case 2:
                        LOG_MSG(db_ALWAYS, "C789%s %04xh\r\n",  m_pc789Reg->cReg, dwData);
                        break;
                    case 3:
                        LOG_MSG(db_ALWAYS, "C789%s %06xh\r\n",  m_pc789Reg->cReg, dwData);
                        break;
                    case 4:
                        LOG_MSG(db_ALWAYS, "C789%s %08xh\r\n",  m_pc789Reg->cReg, dwData);
                        break;
                    default:
                        break;
                 }

                 MS_SLEEP(10);
            }

            m_pc789Reg++;
        }

    }
    else
    {
        return;
    }
#endif
}

UINT8 dvC789_OutputChanged_Get(void)        //A70LV_Doulas_0307
{
    return m_ucOutputChanged;
}

void dvC789_OutputChanged_Set(UINT8 ucVal)        //A70LV_Doulas_0307
{
    m_ucOutputChanged = ucVal;
}

void dvC789_ColorUniformity_Control(UINT8 ucEnable, UINT8 ucCPUAccessEnable)
{
    int GV_RTCT = dvC789_Read(BN_RTCT);
    LOG_MSG(db_DV_GEO, "(func:%s, line:%d)(%d,%d)\n", __FUNCTION__, __LINE__, ucEnable, ucCPUAccessEnable);	//A65_OPTOMA_Doulas_0136

    if(ucEnable == 0)
    {
        if (ucCPUAccessEnable == 0)
        {
            GV_RTCT = (GV_RTCT & 0xf8) | 0x05;
            dvC789_Write( BN_RTCT, GV_RTCT);
            dvC789_Write( B5_UCCT, 0x0104);  //A65_OPTOMA_CL_0005 , 30bit -> 28bit table
        }
        else
        {
            GV_RTCT = GV_RTCT & 0xf8;
            dvC789_Write( BN_RTCT, GV_RTCT);
            dvC789_Write( B5_UCCT, 0x0106);  //A65_OPTOMA_CL_0005 , 30bit -> 28bit table
        }
        dvC789_Write( B5_ERDFCT, 0x60);  //A65_OPTOMA_CL_0002 for contouring  //A35G2_CDS_CL_0001
        dvC789_Write( B5_ERDFCT2, 0x18);
    }
    else
    {
        if(ucCPUAccessEnable == 0)
        {
            GV_RTCT = (GV_RTCT & 0xf8) | 0x05;
            dvC789_Write( BN_RTCT, GV_RTCT);
            dvC789_Write( B5_UCCT, 0x0105);  //A65_OPTOMA_CL_0005 , 30bit -> 28bit table
        }
        else
        {
            GV_RTCT = GV_RTCT & 0xf8;
            dvC789_Write( BN_RTCT, GV_RTCT);
            dvC789_Write( B5_UCCT, 0x0107);  //A65_OPTOMA_CL_0005 , 30bit -> 28bit table
        }
        dvC789_Write( B5_ERDFCT, 0x60);  //A65_OPTOMA_CL_0002 for contouring  //A35G2_CDS_CL_0001
        dvC789_Write( B5_ERDFCT2, 0x1c);
    }


    dvC789_ColorUniformityEnable_Set(ucEnable);
    MS_SLEEP(20);

}

void dvC789_ColorUniformityEnable_Set(UINT8 ucVal)
{
    ucColorUniformityEnable = ucVal;
}

UINT8 dvC789_ColorUniformityEnable_Get(void)    //A65_OPTOMA_CL_0001  //A35G2_CDS_CL_0001
{
    return ucColorUniformityEnable;
}

void dvC789_COMS_Output_Set(UINT8 ucEnable)     //A70LV_Doulas_0361
{
    if(ucEnable)
    {
        dvC789_Write(B0_CMOSOUTSETUP, 0x50);
        dvC789_Write(B0_POCLKCT,  0x01);          //A70LV_Doulas_0363
    }
    else
    {
        dvC789_Write(B0_CMOSOUTSETUP, 0x00);
        dvC789_Write(B0_POCLKCT,  0x00);          //A70LV_Doulas_0363
    }
}

UINT8 dvC789_OutputV_FreqGet(void)     //A70LV_Doulas_0367
{
    UINT8 ucV_Freq = 60;//1;

#ifdef C789_2D_NO_FRAME_LOCK
    if((m_sDrvC789Info.ePanelTimingId == ePANEL_ID_1080P_120HZ) ||
       (m_sDrvC789Info.ePanelTimingId == ePANEL_ID_WUXGA_120HZ))    //A70LV_Doulas_0377 Modify
    {
        if(m_uiH_Total*2 > (m_sDrvC789Info.sOutputTimingInfo.uiHTotal + 50))
        {
            ucV_Freq = 100;
        }
        else
        {
            ucV_Freq = 120;
        }
    }
    else
    {
        if(m_ucForcedSyncReset)
        {
            if(m_uiH_Total > (m_sDrvC789Info.sOutputTimingInfo.uiHTotal + 50))
            {
                ucV_Freq = 50;
            }
            else
            {
                ucV_Freq = 60;
            }
        }
        else
        {
            ucV_Freq = 60;
        }
    }
#endif

    return ucV_Freq;
}
#ifdef Low_Latency_All
void dvC789_Low_Latency_Set(UINT8 ucEnable)
{
    m_sDrvC789Info.sOutputTimingInfo.ucLow_Latency = ucEnable;
	if(!ucEnable)
	{
		m_ucForcedSyncReset = FALSE;
		m_uiH_Total = m_sDrvC789Info.sOutputTimingInfo.uiHTotal;
		m_uiV_Total = m_sDrvC789Info.sOutputTimingInfo.uiVTotal;
	}
	dvC789_ConfigMemAD();
	dvC789_ConfigOutput();
}
BOOL dvC789_Low_Latency_Get(void)     	//ZU860_Doulas_00152
{
    return (BOOL)m_sDrvC789Info.sOutputTimingInfo.ucLow_Latency;
}
#endif	/*Low_Latency_All*/
void dvC789_Test(UINT16 *puiTtlCnt, UINT16 *puiErrCnt)  //G100_Owen_0085
{
    uOPD_DATA uOPDData = {0};
    UINT8 ucIdx, ucData, ucErrIdx = 0xFF;
    UINT16 uiTotal = 0, uiErrCnt = 0;

    sprintf(uOPDData.sINTERFACE.cOPD_Type, "SPI");
    sprintf(uOPDData.sINTERFACE.cIC_Name, "C789");
    sprintf(uOPDData.sINTERFACE.cResult, "PASS");

//    GPIO_Write(32, 0);
    for(ucIdx=0xFF; ucIdx>0; ucIdx--)   //G100_Owen_0137
    {
        dvC789_Write(B1_TESTG, ucIdx);
        MS_SLEEP(5);
        ucData = (dvC789_Read(B1_TESTG) & 0xFF);
        uiTotal++;
        if(ucData != ucIdx)
        {
//            GPIO_Write(32, 1);
            uiErrCnt++;
            if(uiErrCnt == 1)
            {
                sprintf(uOPDData.sINTERFACE.cResult, "FAIL");
                ucErrIdx = ucIdx;
            }
            LOG_MSG(db_DV_GEO, "C789 W[%X] R[%X]\r\n", ucIdx, ucData);
//            break;
        }
    }
    dvC789_Write(B1_TESTG, 0);  //G100_Owen_0137

    uOPDData.sINTERFACE.ulError = uiErrCnt;
    uOPDData.sINTERFACE.ucErrorRate = uiErrCnt*100/uiTotal;
    snprintf(uOPDData.sINTERFACE.cString, 255, "%s,%s,%d,%d,%d,%d,[%d]",
            uOPDData.sINTERFACE.cIC_Name,
            uOPDData.sINTERFACE.cResult,
            uOPDData.sINTERFACE.ulRetry,
            uOPDData.sINTERFACE.ulError,
            uOPDData.sINTERFACE.ucI2C_St,
            uOPDData.sINTERFACE.ucErrorRate,
            ucErrIdx);
    utilOPD_EventSet((UINT8)eOPD_INTERFACE_LOG, &uOPDData);
    *puiTtlCnt = uiTotal;
    *puiErrCnt = uiErrCnt;

    LOG_MSG(db_DV_GEO, "C789 Total[%d], Error[%d]\r\n", uiTotal, uiErrCnt);
}



//SetEgbBiasCursorEn
void dvC789_SetEdgebldBiasCursorEnable(UINT8 ucEnable)
{
	if ( ucEnable == 0 )
	{
		dvC789_Write( B8_CURCT, 0x0016 );
	}
	else
	{
		dvC789_Write( B8_CURCT, 0x0017 );
	}
}

//SetEgbBiasCurPos
void dvC789_SetEdgebldBiasCursorPos( int selx, int sely, int dx, int dy )
{
	PM_EBIAS_CUR[selx][sely].x = PM_EBIAS_CUR[selx][sely].x + dx;
	PM_EBIAS_CUR[selx][sely].x = ( PM_EBIAS_CUR[selx][sely].x < 0 ) ? 0 : ( PM_EBIAS_CUR[selx][sely].x >= PS_WP_HW ) ? PS_WP_HW - 1 : PM_EBIAS_CUR[selx][sely].x;

	PM_EBIAS_CUR[selx][sely].y = PM_EBIAS_CUR[selx][sely].y + dy;
	PM_EBIAS_CUR[selx][sely].y = ( PM_EBIAS_CUR[selx][sely].y < 0 ) ? 0 : ( PM_EBIAS_CUR[selx][sely].y >= PS_WP_VW ) ? PS_WP_VW - 1 : PM_EBIAS_CUR[selx][sely].y;

	if ( selx == 0 )
	{
		if ( sely == 0 )  //TL
		{
			dvC789_Write( B8_CURTLX, PM_EBIAS_CUR[selx][sely].x );
			dvC789_Write( B8_CURTLY, PM_EBIAS_CUR[selx][sely].y );
		}

		if ( sely == 1 )  //BL
		{
			dvC789_Write( B8_CURBLX, PM_EBIAS_CUR[selx][sely].x );
			dvC789_Write( B8_CURBLY, PM_EBIAS_CUR[selx][sely].y - (Def_4CURSOR_Size - 1) );
		}
	}

	if ( selx == 1 )
	{
		if ( sely == 0 )  //TR
		{
			dvC789_Write( B8_CURTRX, PM_EBIAS_CUR[selx][sely].x - (Def_4CURSOR_Size - 1) );
			dvC789_Write( B8_CURTRY, PM_EBIAS_CUR[selx][sely].y );
		}

		if ( sely == 1 )  //BR
		{
			dvC789_Write( B8_CURBRX, PM_EBIAS_CUR[selx][sely].x - (Def_4CURSOR_Size - 1) );
			dvC789_Write( B8_CURBRY, PM_EBIAS_CUR[selx][sely].y - (Def_4CURSOR_Size - 1) );
		}
	}
}


//EgbBiasEn
void dvC789_EdgebldBiasEnable(INT8 cEnable)
{
	if ( cEnable == FALSE )
	{
		dvC789_Write( B7_EGBIASCT, 0x005C );
	}
	else
	{
		dvC789_Write( B7_EGBIASCT, 0x005D );
	}
}

//SetEgbBiasPlt
void dvC789_SetEdgebldBiasPalette( int plt, int r, int g, int b )
{
	r = ( r < 0 ) ? 0 : (( r > 0xFF ) ? 0xFF : r);
	g = ( g < 0 ) ? 0 : (( g > 0xFF ) ? 0xFF : g);
	b = ( b < 0 ) ? 0 : (( b > 0xFF ) ? 0xFF : b);

	PM_EBIAS[plt][0] = r;
	PM_EBIAS[plt][1] = g;
	PM_EBIAS[plt][2] = b;
}


//////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////

void dvC789_BlackLevel_AreaWrite( int plt_sel, coord_t P1, coord_t P2, coord_t P3, coord_t P4 , int iDataWrite)
{
	coord_t P[4] = { P1, P2, P3, P4 };
	line_coef_ab_t coef[6];
	pos_t line[6];
	pos_t px, py;
	int y;
	///// bias area file /////
	FILE *fpr, *fpw;
	int fpr_open, fpw_open;
	int ferr;
	char fn_r[128];
	char fn_tmp[] = BLACKLEVEL_HANDLING_FILE /*"BlackLevelTmp.bak"*/;
	char buf[Def_HW_Max+2];
	int len;
	int x;
	int plt_om, plt_hend;
	int x_plt_om;
	//////////////////////////

	///// bias area file /////
	fpr_open = 0;
	fpw_open = 0;
	ferr = 0;

    // check file is exist or not
	if ( (fpr = fopen( fn_tmp, "r" )) != NULL )
	{
		fclose( fpr );
		remove( fn_tmp );
	}

	//fn_r = FileNameEBAreaBak( PM_BOARD );
	sprintf( fn_r, BLACKLEVEL_TMP_FILE /*"ebarea0.bak"*/ );

    if(access(BLENDING_FILE_PATH, R_OK) < 0)  //folder is not exist
	{
        LOG_MSG(db_DV_GEO, "create folder : %s\n", BLENDING_FILE_PATH );

	    mkdir(BLENDING_FILE_PATH, 0777);   //create folder
	}

    if(access(BLACKLEVEL_TMP_FILE, R_OK) < 0)  // BLACKLEVEL_TMP_FILE is not exist
    {
        LOG_MSG(db_DV_GEO, "create temp file : %s\n", fn_r );

	    //create temp file
		if ( (fpw = fopen( fn_r, "w" )) == NULL )
		{
			LOG_MSG(db_DV_GEO, "\n" );
			LOG_MSG(db_DV_GEO, "create temp file error : %s\n", fn_r );
			fpw_open = 0;
		}
		else
		{
            for(int i=0 ; i<PS_WP_VW ; i++)
            {
                buf[0] = '0';
                buf[1] = '\n';
                buf[2] = '\0';
                fputs( buf, fpw );
            }

            fclose(fpw);
        }
    }

	if ( rename( fn_r, fn_tmp ) != 0 )  //filename fn_r (ebarea0.bak) change to fn_tmp (BlackLevelTmp.bak)
	{
		LOG_MSG(db_DV_GEO, "\n" );
		LOG_MSG(db_DV_GEO, "File error : %s, %s\n", fn_r, fn_tmp );
		ferr = 1;
	}

	if ( ferr == 0 )
	{
		if ( (fpr = fopen( fn_tmp, "r" )) == NULL )
		{
			LOG_MSG(db_DV_GEO, "\n" );
			LOG_MSG(db_DV_GEO, "File open error : %s\n", fn_tmp );
			fpr_open = 0;
		}
		else
		{
			fpr_open = 1;
		}

		if ( (fpw = fopen( fn_r, "w" )) == NULL )
		{
			LOG_MSG(db_DV_GEO, "\n" );
			LOG_MSG(db_DV_GEO, "File open error : %s\n", fn_r );
			fpw_open = 0;
		}
		else
		{
			fpw_open = 1;
		}
	}
	//////////////////////////

	coef[0] = CalcCoef( &P[0], &P[1] );
	coef[1] = CalcCoef( &P[0], &P[2] );
	coef[2] = CalcCoef( &P[0], &P[3] );
	coef[3] = CalcCoef( &P[1], &P[2] );
	coef[4] = CalcCoef( &P[1], &P[3] );
	coef[5] = CalcCoef( &P[2], &P[3] );

	py = YMinMax( P, 4 );
	///// bias area file /////
	for ( y = 0; y < py.st; y++ )
	{
		if ( fpr_open == 1 )
		{
			if ( fgets( buf, sizeof( buf ), fpr ) == NULL )
			{
//				printf( "\n" );
//				printf( "Close %s\n", fn_tmp );
				fclose( fpr );
				remove( fn_tmp );
				fpr_open = 0;
				buf[0] = '0';
				buf[1] = '\n';
				buf[2] = '\0';
			}
		}
		else
		{
			buf[0] = '0';
			buf[1] = '\n';
			buf[2] = '\0';
		}
		len = strlen( buf ) - 1;
		if ( len < 1 )
		{
			buf[0] = '0';
			buf[1] = '\n';
			buf[2] = '\0';
			len = strlen( buf ) - 1;
		}

		if ( fpw_open == 1 )
		{
			fputs( buf, fpw );
		}
	}
	//////////////////////////

	for ( y = py.st; y <= py.end; y++ )
	{
		line[0] = CalcLineX( &y, &P[0], &P[1], &coef[0] );
		line[1] = CalcLineX( &y, &P[0], &P[2], &coef[1] );
		line[2] = CalcLineX( &y, &P[0], &P[3], &coef[2] );
		line[3] = CalcLineX( &y, &P[1], &P[2], &coef[3] );
		line[4] = CalcLineX( &y, &P[1], &P[3], &coef[4] );
		line[5] = CalcLineX( &y, &P[2], &P[3], &coef[5] );
		px = XMinMax( line, 6 );
		if ( (px.st < PS_WP_HW) && (px.end >= 0) && (px.end >= px.st) )
		{
			px.st = ( px.st < 0 ) ? 0 : px.st;
			px.end = ( px.end >= PS_WP_HW ) ? PS_WP_HW - 1 : px.end;

			if(iDataWrite)
			{
    			WPLT2( plt_sel, px, y );// bias area file
            }

			///// bias area file /////
			if ( fpr_open == 1 )
			{
				if ( fgets( buf, sizeof( buf ), fpr ) == NULL )
				{
//					printf( "\n" );
//					printf( "Close %s\n", fn_tmp );
					fclose( fpr );
					remove( fn_tmp );
					fpr_open = 0;
					buf[0] = '0';
					buf[1] = '\n';
					buf[2] = '\0';
				}
			}
			else
			{
				buf[0] = '0';
				buf[1] = '\n';
				buf[2] = '\0';
			}
			len = strlen( buf ) - 1;
			if ( len < 1 )
			{
				buf[0] = '0';
				buf[1] = '\n';
				buf[2] = '\0';
				len = strlen( buf ) - 1;
			}

			plt_om = 0;
			x_plt_om = 0;
			plt_hend = buf[len-1];
			for ( x = 0; x < PS_WP_HW; x++ )
			{
				if ( (x >= px.st) && (x <= px.end) )
				{
					buf[x] = HexToChar( plt_sel & 0x0f );
				}
				else
				{
					if ( x >= len )
					    { buf[x] = plt_hend; }
				}

				if ( CharToHex(buf[x]) != plt_om )
				{
					plt_om = CharToHex( buf[x] );
					x_plt_om = x;
				}
			}
			buf[x_plt_om+1] = '\n';
			buf[x_plt_om+2] = '\0';

			if ( fpw_open == 1 )
			{
				fputs( buf, fpw );
			}
			//////////////////////////
		}
	}
	dvC789_Write( B0_CPUWAD, 0 );// dummy

	///// bias area file /////
	for ( y = (py.end + 1); y < PS_WP_VW; y++ )
	{
		if ( fpr_open == 1 )
		{
			if ( fgets( buf, sizeof( buf ), fpr ) == NULL )
			{
//				printf( "\n" );
//				printf( "Close %s\n", fn_tmp );
				fclose( fpr );
				remove( fn_tmp );
				fpr_open = 0;
				buf[0] = '0';
				buf[1] = '\n';
				buf[2] = '\0';
			}
		}
		else
		{
			buf[0] = '0';
			buf[1] = '\n';
			buf[2] = '\0';
		}
		len = strlen( buf ) - 1;
		if ( len < 1 )
		{
			buf[0] = '0';
			buf[1] = '\n';
			buf[2] = '\0';
			len = strlen( buf ) - 1;
		}

		if ( fpw_open == 1 )
		{
			fputs( buf, fpw );
		}
	}

	if ( fpw_open == 1 )
	{
		fclose( fpw );
	}

	if ( fpr_open == 1 )
	{
		fclose( fpr );
		remove( fn_tmp );
	}
	//////////////////////////
}


void WPLT2( int plt_sel, pos_t x, int y )   // bias area file
{
	int ad;
	int bbacthw;
	int fill;
//	int rdt;
	///// bias area file /////
	//int biasdt = (plt_sel<<4) + plt_sel;
	//////////////////////////

	if ( (x.st >= PS_WP_HW) || (x.end < 0) || (x.st > x.end) )
	{
	    return;
	}

	x.st = ( x.st < 0 ) ? 0 : x.st;
	x.end = ( x.end >= PS_WP_HW ) ? PS_WP_HW - 1 : x.end;

	if ( (x.st & 0x01) == 1 )
	{
		ad = DEF_EBIASSAD + ( x.st >> 1 ) + y * DEF_EBIASMWI * 256;
		dvC789_WriteToBuffer( B0_CPURAD, ad );
		dvC789_WriteToBuffer( B0_CPUDTCTL, 0x01 );
		dvC789_Buffer_Flush();
		fill = dvC789_Read( B0_CPUDT ) & 0x0f;
		fill = fill + ( plt_sel << 4 );
		dvC789_WriteToBuffer( B0_CPUWAD, ad );
		dvC789_WriteToBuffer( B0_CPUDT, fill );//Data write
		dvC789_Buffer_Flush();
		x.st = x.st + 1;
	}

	if ( (x.st <= x.end) && ((x.end & 0x01) == 0) )
	{
		ad = DEF_EBIASSAD + ( x.end >> 1 ) + y * DEF_EBIASMWI * 256;
		dvC789_WriteToBuffer( B0_CPURAD, ad );
		dvC789_WriteToBuffer( B0_CPUDTCTL, 0x01 );
		dvC789_Buffer_Flush();
		fill = dvC789_Read( B0_CPUDT ) & 0xf0;
		fill = fill + plt_sel;
		dvC789_WriteToBuffer( B0_CPUWAD, ad );
		dvC789_WriteToBuffer( B0_CPUDT, fill );//Data write
		dvC789_Buffer_Flush();
		x.end = x.end - 1;
	}

	if ( x.st <= x.end )
	{
		ad = DEF_EBIASSAD + ( x.st >> 1 ) + y * DEF_EBIASMWI * 256;
		bbacthw = (x.end >> 1) - (x.st >> 1);
		fill = (plt_sel<<4) + plt_sel;
		dvC789_WriteToBuffer( B0_CPUWAD, ad );
		if ( bbacthw == 0 )
		{
			dvC789_WriteToBuffer( B0_CPUDT, fill );//Data write
		}
		else
		{
			dvC789_WriteToBuffer( B0_OSDFILL, fill );
			dvC789_WriteToBuffer( B0_BBACTHW, bbacthw );
			dvC789_WriteToBuffer( B0_BBACTVW, 0x00 );
			dvC789_WriteToBuffer( B0_OSDCT, 0x01 );//FILL write
//			do { rdt = dvC789_Read( B0_BOSTAT, 0 ) & 0x01; } while ( rdt != 0x00 );
			dvC789_WriteToBuffer( B0_OSDCT, 0x00 );
		}

		dvC789_Buffer_Flush();
	}
//	dvC789_Write( B0_CPUWAD, 0, 0 );//dummy
}


//calculation
line_coef_ab_t CalcCoef( const coord_t *p0, const coord_t *p1 )
{
	line_coef_ab_t ans;

	ans.a = ( (*p0).x == (*p1).x ) ? (float)( (*p0).y - (*p1).y ) * Def_DIV0 : (float)( (*p0).y - (*p1).y ) / (float)( (*p0).x - (*p1).x );
	ans.b = (float)(*p0).y - ans.a * (float)(*p0).x;
	return ans;
}

pos_t CalcLineX( const int *y_crt, const coord_t *p_b, const coord_t *p_t, const line_coef_ab_t *coef )
{
	pos_t ans;
	coord_t y_min, y_max;
	int x_crt, x_nxt;
	int x_center, y_center;
	float d_temp;

	if( (*p_b).y > (*p_t).y )
	{
		y_min = (*p_t);
		y_max = (*p_b);
	}
	else
	{
		y_min = (*p_b);
		y_max = (*p_t);
	}

	if( (*y_crt < y_min.y) || (*y_crt > y_max.y) )
	{
		ans.st = -1;
		ans.end = -1;
	}
	else if( (*p_b).y == (*p_t).y )
	{
		ans.st  = ( (*p_b).x > (*p_t).x ) ? (*p_t).x : (*p_b).x;
//		ans.end = ( (*p_b).x > (*p_t).x ) ? (*p_b).x + 1 : (*p_t).x + 1;
		ans.end = ( (*p_b).x > (*p_t).x ) ? (*p_b).x : (*p_t).x;
	}
	else if( (*p_b).x == (*p_t).x )
	{
		ans.st = (*p_b).x;
//		ans.end = (*p_b).x + 1;
		ans.end = (*p_b).x;
	}
	else
	{
		d_temp = ( (*coef).a == 0 ) ? Def_DIV0 : ( (*coef).a < 0 ) ? -1 / (*coef).a : 1 / (*coef).a;
		x_center = (int)( d_temp / 2 );
		y_center = ( (*coef).a < 0 ) ? (int)( -(*coef).a / 2 ) : (int)( (*coef).a / 2 );

		//current
		d_temp = ( (*coef).a == 0 ) ? ( (*y_crt - y_center) - (*coef).b ) * Def_DIV0 : ( (*y_crt - y_center) - (*coef).b ) / (*coef).a;
		x_crt = (int)( d_temp + 0.5 );
		//next
		d_temp = ( (*coef).a == 0 ) ? ( ((*y_crt + 1) - y_center) - (*coef).b ) * Def_DIV0 : ( ((*y_crt + 1) - y_center) - (*coef).b ) / (*coef).a;
		x_nxt = (int)( d_temp + 0.5 );

		if( x_crt < x_nxt )
		{
			ans.st = ( *y_crt == y_min.y ) ? y_min.x : x_crt - x_center;
//			ans.end = ( *y_crt == y_max.y ) ? y_max.x + 1 : x_nxt - x_center;
			ans.end = ( *y_crt == y_max.y ) ? y_max.x : x_nxt - x_center;
		}
		else if( x_crt > x_nxt )
		{
			ans.st = ( *y_crt == y_max.y ) ? y_max.x : (x_nxt + 1) + x_center;
//			ans.end = ( *y_crt == y_min.y ) ? y_min.x + 1 : (x_crt + 1) + x_center;
			ans.end = ( *y_crt == y_min.y ) ? y_min.x : (x_crt + 1) + x_center;
		}
		else
		{
			ans.st = x_crt;
//			ans.end = x_crt + 1;
			ans.end = x_crt;
		}
	}

	return ans;
}

pos_t XMinMax( const pos_t x[], int num )
{
	pos_t ans = { PS_WP_HW, -1 };

	while ( num )
	{
		num--;
		ans.st = ( (x[num].st != -1) && ( (ans.st == PS_WP_HW) || (ans.st > x[num].st) ) ) ? x[num].st : ans.st;
		ans.end = ( ans.end < x[num].end ) ? x[num].end : ans.end;
	}

	return ans;
}

//return min/max Y coordinate of 4-corner coordinate
// ans.st  == min Y coordinate
// ans.end == max Y coordinate
pos_t YMinMax( const coord_t p[], int num )
{
	pos_t ans = { PS_WP_VW, -1 };

	while ( num )
	{
		num--;
		ans.st = ( ans.st > p[num].y ) ? p[num].y : ans.st;
		ans.end = ( ans.end < p[num].y ) ? p[num].y : ans.end;
	}
//	ans.end += 1;

	return ans;
}


// file_io
#if 0
char *FileNameEBAreaBak( int num )
{
	char fn[Def_BUF_MAX];

	sprintf( fn, "ebarea%d.bak", num );

	return fn;
}
#endif

//LoadEgbBiasArea
int dvC789_LoadEgbBiasArea( char fn[] )
{
	FILE *fpr;
	int fpr_open;
	char buf[Def_HW_Max+2];

	int ad;
	int len = 0;  //A35G2_Simon_0070 for cppcheck
	int x, y, wbyte_rest;
	int dt_old, dt;
//	int rdt;

    LOG_MSG(db_DV_GEO, "Load file %s\n", fn);

	if ( (fpr = fopen( fn, "r" )) == NULL )
	{
		LOG_MSG(db_DV_GEO, "\n" );
		LOG_MSG(db_DV_GEO, "File open error : %s\n", fn );
		fpr_open = 0;
	}
	else
	{
		LOG_MSG(db_DV_GEO, "File load ok\n");
		fpr_open = 1;
	}

	dvC789_Write( B0_BBACTVW, 0x00 );

	for ( y = 0; y < PS_WP_VW; y++ )
	{
	    //printf( "\n(%d)\n" , y);
		if ( fpr_open == 1 )
		{
			if ( fgets( buf, sizeof( buf ), fpr ) == NULL )
			{
//				printf( "\n" );
//				printf( "Close %s\n", fn );
				fclose( fpr );
				fpr_open = 0;
				buf[0] = '0';
				buf[1] = '\n';
				buf[2] = '\0';
			}
			else
			{
			    //printf( "%s\n" , buf);
			}
		}
		else
		{
			buf[0] = '0';
			buf[1] = '\n';
			buf[2] = '\0';
		}
		len = strlen( buf ) - 1;
		if ( len < 1 )
		{
			buf[0] = '0';
			buf[1] = '\n';
			buf[2] = '\0';
			len = strlen( buf ) - 1;
		}

		dt_old = ( len == 1 ) ? CharToHex( buf[0] ) + ( CharToHex( buf[0] ) << 4 ) : CharToHex( buf[0] ) + ( CharToHex( buf[1] ) << 4 );
		ad = DEF_EBIASSAD + y * DEF_EBIASMWI * 256;
		wbyte_rest = 1;
		for ( x = 2; x < PS_WP_HW; x = x + 2 )
		{
			if ( x < len )
			{
				dt = ( len == (x + 1) ) ? CharToHex( buf[x] ) + ( CharToHex( buf[x] ) << 4 ) : CharToHex( buf[x] ) + ( CharToHex( buf[x+1] ) << 4 );
			}
			else
			{
				dt = CharToHex( buf[len-1] ) + ( CharToHex( buf[len-1] ) << 4 );
			}

			if ( dt != dt_old )
			{
				dvC789_Write( B0_CPUWAD, ad );
				if ( wbyte_rest == 1 )
				{
					dvC789_Write( B0_CPUDT, dt_old );
				}
				else
				{
					dvC789_Write( B0_OSDFILL, dt_old );
					dvC789_Write( B0_BBACTHW, (wbyte_rest - 1) );
//					dvC789_Write( B0_BBACTVW, 0x00, 0 );
					dvC789_Write( B0_OSDCT, 0x01 );//FILL write
//					do { rdt = dvC789_Read( B0_BOSTAT, 0 ) & 0x01; } while ( rdt != 0x00 );
					dvC789_Write( B0_OSDCT, 0x00 );
				}
				dt_old = dt;
				ad = DEF_EBIASSAD + (x >> 1) + y * DEF_EBIASMWI * 256;
				wbyte_rest = 1;
			}
			else
			{
				wbyte_rest = wbyte_rest + 1;
			}
		}

		dvC789_Write( B0_CPUWAD, ad);
		if ( wbyte_rest == 1 )
		{
			dvC789_Write( B0_CPUDT, dt_old );
		}
		else
		{
			dvC789_Write( B0_OSDFILL, dt_old );
			dvC789_Write( B0_BBACTHW, (wbyte_rest - 1) );
//			dvC789_Write( B0_BBACTVW, 0x00, 0 );
			dvC789_Write( B0_OSDCT, 0x01 );//FILL write
//			do { rdt = dvC789_Read( B0_BOSTAT, 0 ) & 0x01; } while ( rdt != 0x00 );
			dvC789_Write( B0_OSDCT, 0x00 );
		}

//		dvC789_Write( B0_CPUWAD, 0, 0 );//dummy
	}

	if ( fpr_open == 1 )
	{
		fclose( fpr );
	}

    LOG_MSG(db_DV_GEO, "write End\n");

	return len;
}


static float ebgmLevel[16] = { 0, 4, 8, 12, 16, 24, 32, 48, 64, 96, 128, 192, 256, 384, 512, 1024 };
static int ebTestPlt[16][3] = {
	{ 0x00, 0x00, 0x00 },// plt0
	{ 0xFF, 0x00, 0x00 },// plt1
	{ 0x00, 0xFF, 0x00 },// plt2
	{ 0x00, 0x00, 0xFF },// plt3
	{ 0xFF, 0xFF, 0x00 },// plt4
	{ 0xFF, 0x00, 0xFF },// plt5
	{ 0x00, 0xFF, 0xFF },// plt6
	{ 0xFF, 0x40, 0x80 },// plt7
	{ 0x80, 0xFF, 0x40 },// plt8
	{ 0x40, 0x80, 0xFF },// plt9
	{ 0xFF, 0x80, 0x40 },// plt10
	{ 0x40, 0xFF, 0x80 },// plt11
	{ 0x80, 0x40, 0xFF },// plt12
	{ 0x80, 0x80, 0x40 },// plt13
	{ 0x80, 0x40, 0x80 },// plt14
	{ 0x40, 0x80, 0x80 }// plt15
};

//CalcEgbBiasGammaTable
void dvC789_CalcEdgebldBiasGammaTable( int color, int plt, int allplt )
{
	int lv;
	int col;
	float coef;
	float bias;
	float gm;

	if ( allplt == 0 )
	{
		for ( col = 0; col < 3; col++ )
		{
			if ( (col == color) || (color == 3) )
			{
				coef = ( PM_EBIAS_AREA_TEST == 0 ) ? PM_EBIAS_GAMMA[col] : 1;
				bias = ( PM_EBIAS_AREA_TEST == 0 ) ? (float)PM_EBIAS[plt][col] : (float)ebTestPlt[plt][col];
				for ( lv = 0; lv < 16; lv++ )
				{
					gm = pow( (pow( ebgmLevel[lv], coef ) + pow( bias, coef )), 1/coef ) - ebgmLevel[lv];
					PM_EBIAS_GMDT2[plt][col][lv] = ( gm < 0 ) ? 0 : ( gm > 255 ) ? 255 : (int)gm;
				}
			}
		}
	}
	else
	{
		for ( plt = 0; plt < 16; plt++ )
		{
			for ( col = 0; col < 3; col++ )
			{
				coef = ( PM_EBIAS_AREA_TEST == 0 ) ? PM_EBIAS_GAMMA[col] : 1;
				bias = ( PM_EBIAS_AREA_TEST == 0 ) ? (float)PM_EBIAS[plt][col] : (float)ebTestPlt[plt][col];
				for ( lv = 0; lv < 16; lv++ )
				{
					gm = pow( (pow( ebgmLevel[lv], coef ) + pow( bias, coef )), 1/coef ) - ebgmLevel[lv];
					PM_EBIAS_GMDT2[plt][col][lv] = ( gm < 0 ) ? 0 : ( gm > 255 ) ? 255 : (int)gm;
				}
			}
		}
	}
}


//EgbBiasGammaTable
void dvC789_EdgebldBiasGammaTable( int plt )
{
	int lv;
	int ad;
	int acct;
	int rtct_org = (int)dvC789_Read(BN_RTCT);

	dvC789_Write( BN_RTCT, 0 );

	dvC789_Write( B7_EGBIASAD, 0x00 );

    for( int color = 0 ; color < 3 ; color++)  //rgb
    {
        acct = ( color == 0 ) ? 0x1 : ( color == 1 ) ? 0x3 : ( color == 2 ) ? 0x5 : 0x7;
        dvC789_Write( B7_EGBIASCT, 0x0050 + (acct << 1));

		//for ( plt = 0; plt < 16; plt++ )
		{
    		ad = ((plt < 16) && (plt >= 0)) ? (plt << 4) : 0;
    		dvC789_Write( B7_EGBIASAD, ad & 0xff );

    		dvC789_BurstWrite_FixedAdd( B7_EGBIASDT, 16 , PM_EBIAS_GMDT2[plt][color] );

			//for ( lv = 0; lv < 16; lv++ )
			//{
			//	dvC789_Write( B7_EGBIASDT, PM_EBIAS_GMDT2[plt][color][lv] & 0xff );
			//}
		}
    }

	//dvC789_EdgebldBiasEnable(TRUE);

	dvC789_Write( BN_RTCT, 0x05 );

}

void dvC789_EdgebldBiasGammaTableClear( void )
{
	int lv;
	int ad;
	int acct;
	int rtct_org = (int)dvC789_Read(BN_RTCT);

	memset(PM_EBIAS_GMDT2, 0, 16*3*16);

	dvC789_Write( BN_RTCT, 0 );

	dvC789_Write( B7_EGBIASAD, 0x00 );

    for( int color = 0 ; color < 3 ; color++)  //rgb
    {
        acct = ( color == 0 ) ? 0x1 : ( color == 1 ) ? 0x3 : ( color == 2 ) ? 0x5 : 0x7;
        dvC789_Write( B7_EGBIASCT, 0x0050 + (acct << 1));

		for ( int plt = 0; plt < 16; plt++ )
		{
    		ad = ((plt < 16) && (plt >= 0)) ? (plt << 4) : 0;
    		dvC789_Write( B7_EGBIASAD, ad & 0xff );

    		dvC789_BurstWrite_FixedAdd( B7_EGBIASDT, 16 , PM_EBIAS_GMDT2[plt][color] );
		}
    }

	//dvC789_EdgebldBiasEnable(TRUE);

	dvC789_Write( BN_RTCT, 0x05 );

}


#if 0
//EgbBiasGammaTable
void dvC789_EdgebldBiasGammaTable( int color, int plt, int allplt )
{
	int lv;
	int ad;
	int acct;
	int rtct_org = (int)dvC789_Read(BN_RTCT);

	dvC789_Write( BN_RTCT, 0 );

	acct = ( color == 0 ) ? 0x1 : ( color == 1 ) ? 0x3 : ( color == 2 ) ? 0x5 : 0x7;
	dvC789_Write( B7_EGBIASCT, 0x0050 + (acct << 1));

	if ( allplt == 0 )
	{
		ad = ((plt < 16) && (plt >= 0)) ? (plt << 4) : 0;

		dvC789_Write( B7_EGBIASAD, ad & 0xff );

		if ( (color >= 0) && (color < 3) )
		{
			for ( lv = 0; lv < 16; lv++ )
			{
				dvC789_Write( B7_EGBIASDT, PM_EBIAS_GMDT2[plt][color][lv] & 0xff );
			}
		}
		else
		{
			for ( lv = 0; lv < 16; lv++ )
			{
				dvC789_Write( B7_EGBIASDT, PM_EBIAS_GMDT2[plt][0][lv] & 0xff );
			}
		}
	}
	else
	{
		dvC789_Write( B7_EGBIASAD, 0x00 );

		for ( plt = 0; plt < 16; plt++ )
		{
			if ( (color >= 0) && (color < 3) )
			{
				for ( lv = 0; lv < 16; lv++ )
				{
					dvC789_Write( B7_EGBIASDT, PM_EBIAS_GMDT2[plt][color][lv] & 0xff );
				}
			}
			else
			{
				for ( lv = 0; lv < 16; lv++ )
				{
					dvC789_Write( B7_EGBIASDT, PM_EBIAS_GMDT2[plt][0][lv] & 0xff );
				}
			}
		}
	}

	dvC789_EdgebldBiasEnable(TRUE);

	dvC789_Write( BN_RTCT, 0x105 );

}
#endif


void dvC789_Blacklevel_Palette_Set(INT8 cPaletteIdx, UINT8 cRed, UINT8 cGreen, UINT8 cBlue)
{
    dvC789_Write( B0_CPUWAD, 0 );

    dvC789_SetEdgebldBiasPalette(cPaletteIdx, (INT32)cRed, (INT32)cGreen, (INT32)cBlue);

    dvC789_CalcEdgebldBiasGammaTable(3, cPaletteIdx, 0);

    dvC789_EdgebldBiasGammaTable(cPaletteIdx);
}

void dvC789_RTCT_Set(int data)
{
    dvC789_Write( BN_RTCT, data );
}

char HexToChar( int val )
{
	char ch;

	switch ( val ) {
		case 0  : ch = '0'; break;
		case 1  : ch = '1'; break;
		case 2  : ch = '2'; break;
		case 3  : ch = '3'; break;
		case 4  : ch = '4'; break;
		case 5  : ch = '5'; break;
		case 6  : ch = '6'; break;
		case 7  : ch = '7'; break;
		case 8  : ch = '8'; break;
		case 9  : ch = '9'; break;
		case 10 : ch = 'A'; break;
		case 11 : ch = 'B'; break;
		case 12 : ch = 'C'; break;
		case 13 : ch = 'D'; break;
		case 14 : ch = 'E'; break;
		case 15 : ch = 'F'; break;
		default : ch = -1;  break;
	}

	return ch;
}

int CharToHex( char ch )
{
	char val;

	switch( ch ) {
		case '0': val = 0; break;
		case '1': val = 1; break;
		case '2': val = 2; break;
		case '3': val = 3; break;
		case '4': val = 4; break;
		case '5': val = 5; break;
		case '6': val = 6; break;
		case '7': val = 7; break;
		case '8': val = 8; break;
		case '9': val = 9; break;
		case 'a':
		case 'A': val = 10; break;
		case 'b':
		case 'B': val = 11; break;
		case 'c':
		case 'C': val = 12; break;
		case 'd':
		case 'D': val = 13; break;
		case 'e':
		case 'E': val = 14; break;
		case 'f':
		case 'F': val = 15; break;
//		default : val = -1; break;
		default : val = 0; break;
	}

	return val;
}



void dvC789_Dummy(void)
{
	//Cindy 20190604-161804
	//Fix width wrong issue
	//Add this to avoid blend/osd drawing not completed

	// dummy start
	//dvC789_WriteBuffer(B0_CPUWAD, DEF_BLEND_ADDRESS + m_nVertResolution * DEF_OSDMWI *128, 0);
    dvC789_Write(B0_CPUWAD, DEF_BLEND_ADDRESS + PS_WP_VW * DEF_OSDMWI *128);
	for(int i=0; i<8; i++)
		dvC789_Write(B0_CPUDT, 0xFF);
	// dummy end
}

//A35G2_CDS_Simon_0044
void dvC789_SetOSDMode(bool bFront)
{
	dvC789_OSDACTStartConfig(bFront);

	if(bFront)
	{
    	dvC789_Write(B1_OSDMODE, 0x03);
    }
    else
    {
    	dvC789_Write(B1_OSDMODE, 0x07);
    }

}

void dvC789_OSDACTStartConfig(bool bFront)    //G100_Simon_0007
{
	int acthst, actvst;

	if (bFront)
	{
		if (PS_WP_VW == 1080)
		{
			acthst = 0x0021; //A35G2_CDS_Simon_0044
			actvst = 0x0029; //A35G2_CDS_Simon_0044
		}
		else
		{
			acthst = 0x0029;  //A35G2_CDS_Simon_0044
			actvst = 0x0014 - 1; //A35G2_CDS_Simon_0044
		}
	}
	else
	{
		if (PS_WP_VW == 1080)
		{
			acthst = 0x0021 ;
			actvst = 0x0027 + 2;   //A35G2_CDS_Simon_0023
		}
		else
		{
			acthst = 0x0029;
			actvst = 0x0015 - 2;   //A35G2_CDS_Simon_0023
		}
	}

	dvC789_Write(B1_OSDACTHST, acthst);
	dvC789_Write(B1_OSDACTVST, actvst);
}

UINT32 dvC789_V_Start_Get(void)    //G100_Owen_0078
{
    return dvC789_Read(B10_IFACTVST);
}

void dvC789_ISYCT_Set(UINT8 uiRegValue)    //A35G2_CDS_Simon_0003
{
    m_sDrvC789Info.ucRegISYCT = uiRegValue;
    dvC789_Write(B3_ISYCT, (UINT32)m_sDrvC789Info.ucRegISYCT);
}

UINT8 dvC789_ISYCT_Get(void)    //A35G2_CDS_Simon_0003
{
    return m_sDrvC789Info.ucRegISYCT ;
}

void dvC789_AdaptiveScaleFilterLutInit(void) //A65_OPTOMA_CL_0018  //A35G2_Simon_0115
{
	INT32 dir;// 0 : Horizontal, 1 : Vertical
    INT32 GV_RTCT = dvC789_Read(BN_RTCT);
//        GV_RTCT = GV_RTCT & 0xfc;// [1:0] RTG0 = Through

	dvC789_Write(BN_RTCT, 0);

	for (dir = 0; dir < 2; dir++)
	{
		if (dir == 0) // Horizontal
		{
			dvC789_WriteToBuffer(B6_SCLADPCT, 0x04);
		}
		else // Vertical
		{
			dvC789_WriteToBuffer(B6_SCLADPCT, 0x08);
		}
		dvC789_WriteToBuffer(B6_SCLADPAD, 0x00);

		// cutoff=1.000, window=1.000
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x01);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x01);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x01);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x01);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xff);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfd);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfb);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xf9);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xf8);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xf9);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfc);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x06);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0f);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x1a);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x26);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x32);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x3a);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x3e);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x40);
		// cutoff=0.988, window=0.988
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x01);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x01);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x01);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x01);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xff);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfc);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfa);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xf9);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xf8);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfa);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfc);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x07);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x10);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x1a);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x26);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x32);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x39);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x3e);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x40);
		// cutoff=0.975, window=0.975
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x01);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x01);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x01);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x01);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfe);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfc);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfa);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xf9);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xf9);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfa);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfd);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x01);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x07);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x10);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x1b);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x26);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x30);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x39);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x3e);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x3e);
		// cutoff=0.962, window=0.962
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x01);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x01);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x01);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x01);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfe);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfc);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfa);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xf9);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xf9);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfa);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfd);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x01);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x08);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x11);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x1b);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x26);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x30);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x38);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x3d);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x3e);
		// cutoff=0.950, window=0.950
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x01);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x01);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x01);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x01);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xff);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfe);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfc);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfa);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xf9);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xf9);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfb);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfe);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x02);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x08);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x11);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x1b);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x26);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x30);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x37);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x3c);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x3e);
		// cutoff=0.938, window=0.938
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x01);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x01);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x01);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x01);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xff);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfd);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfb);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfa);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xf9);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xf9);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfb);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfe);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x02);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x09);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x12);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x1b);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x26);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x30);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x37);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x3c);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x3e);
		// cutoff=0.925, window=0.925
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x01);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x01);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x01);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x01);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xff);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfd);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfb);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfa);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xf9);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xf9);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfb);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xff);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x03);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0a);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x12);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x1c);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x26);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x2f);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x37);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x3a);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x3c);
		// cutoff=0.913, window=0.913
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x01);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x01);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x01);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x01);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfe);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfd);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfb);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfa);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xf9);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfa);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfc);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xff);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x03);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0a);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x12);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x1c);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x26);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x2e);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x36);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x3a);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x3e);
		// cutoff=0.900, window=0.900
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x01);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x01);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x01);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x01);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfe);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfc);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfb);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xf9);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xf9);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfa);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfc);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x04);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0b);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x13);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x1c);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x26);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x2f);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x35);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x39);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x3c);
		// cutoff=0.875, window=0.875
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x01);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x01);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x01);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x01);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xff);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfd);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfc);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfa);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xf9);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xf9);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfb);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfd);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x05);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0c);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x14);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x1c);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x26);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x2f);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x34);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x38);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x3c);
		// cutoff=0.850, window=0.850
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x01);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x01);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x01);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x01);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfe);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfd);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfb);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfa);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfa);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfa);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfb);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfe);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x01);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x07);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0d);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x15);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x1d);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x25);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x2d);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x32);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x38);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x38);
		// cutoff=0.825, window=0.825
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x01);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x01);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x01);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xff);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfe);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfc);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfb);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfa);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfa);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfb);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfc);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xff);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x02);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x08);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0e);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x15);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x1d);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x25);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x2c);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x32);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x36);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x38);
		// cutoff=0.800, window=0.800
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x01);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x01);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xff);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfe);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfd);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfc);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfb);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfa);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfa);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfb);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfd);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x04);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x09);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0f);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x16);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x1e);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x25);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x2c);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x31);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x34);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x36);
		// cutoff=0.775, window=0.775
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x01);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xff);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfd);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfc);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfb);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfb);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfa);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfb);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfc);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfe);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x01);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x05);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0a);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x10);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x17);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x1e);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x24);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x2a);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x31);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x33);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x36);
		// cutoff=0.750, window=0.750
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x01);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xff);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfe);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfd);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfc);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfb);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfb);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfa);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfb);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfd);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xff);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x02);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x06);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0b);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x11);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x17);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x1e);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x24);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x2a);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x30);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x31);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x34);
		// cutoff=0.725, window=0.725
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfe);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfd);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfc);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfb);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfb);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfb);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfb);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfc);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfe);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x03);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x07);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0c);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x12);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x18);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x1e);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x24);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x29);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x2e);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x31);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x32);
		// cutoff=0.700, window=0.700
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xff);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfe);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfd);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfc);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfb);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfb);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfb);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfc);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfd);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xff);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x01);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x04);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x09);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0d);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x12);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x18);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x1e);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x23);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x28);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x2c);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x2f);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x30);
		// cutoff=0.675, window=0.675
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xff);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfe);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfd);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfc);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfc);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfb);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfb);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfb);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfc);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfe);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x02);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x06);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0a);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0e);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x13);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x19);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x1e);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x23);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x28);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x2a);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x2d);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x2e);
		// cutoff=0.650, window=0.650
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xff);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfe);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfd);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfd);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfc);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfb);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfb);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfc);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfc);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfd);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xff);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x03);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x07);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0b);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0e);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x14);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x19);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x1e);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x23);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x27);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x2a);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x2b);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x2c);
		// cutoff=0.625, window=0.625
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xff);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfe);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfd);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfc);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfc);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfb);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfc);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfc);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfd);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfe);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x02);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x05);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x08);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0c);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0f);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x14);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x19);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x1e);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x22);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x24);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x28);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x28);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x2a);
		// cutoff=0.600, window=0.600
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfe);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfd);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfc);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfc);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfc);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfc);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfc);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfd);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfe);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xff);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x03);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x06);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x09);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0c);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0f);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x14);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x19);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x1d);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x21);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x25);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x26);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x28);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x28);
		// cutoff=0.575, window=0.575
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfd);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfd);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfc);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfc);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfc);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfc);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfd);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfe);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xff);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x01);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x04);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x07);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0a);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0d);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x10);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x14);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x18);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x1c);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x20);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x24);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x25);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x26);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x24);
		// cutoff=0.550, window=0.550
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfd);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfd);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfc);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfc);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfc);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfd);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfd);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xff);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x01);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x03);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x05);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x08);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0b);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0e);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x10);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x14);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x18);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x1b);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x1f);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x22);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x22);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x24);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x22);
		// cutoff=0.525, window=0.525
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfd);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfc);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfc);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfc);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfd);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfd);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfe);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xff);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x02);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x04);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x06);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x09);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0b);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0e);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x10);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x14);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x17);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x1a);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x1e);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x20);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x23);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x23);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x22);
		// cutoff=0.500, window=0.500
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfd);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfd);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfd);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfd);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfd);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfe);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xff);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x01);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x03);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x05);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x07);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0a);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0c);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0e);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x10);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x14);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x17);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x19);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x1c);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x1e);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x1f);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x21);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x20);
		// cutoff=0.485, window=0.485
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfd);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfd);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfd);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfd);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfe);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xff);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x02);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x03);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x05);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x08);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0a);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0c);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0f);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x10);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x13);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x16);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x18);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x1b);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x1e);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x1f);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x1f);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x20);
		// cutoff=0.471, window=0.471
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfd);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfd);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfd);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfe);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfe);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xff);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x01);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x02);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x04);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x06);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x08);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0a);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0c);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0f);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x10);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x13);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x16);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x18);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x1a);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x1d);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x1e);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x1f);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x1e);
		// cutoff=0.457, window=0.457
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfd);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfd);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfe);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfe);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xff);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x01);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x03);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x05);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x06);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x08);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0a);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0d);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0f);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x10);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x13);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x15);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x17);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x1a);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x1c);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x1c);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x1e);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x1e);
		// cutoff=0.444, window=0.444
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfd);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfe);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfe);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xff);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x01);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x02);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x03);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x05);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x07);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x09);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0b);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0d);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0f);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x10);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x13);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x15);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x16);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x18);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x1a);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x1b);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x1d);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x1c);
		// cutoff=0.432, window=0.432
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfe);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfe);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xff);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xff);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x01);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x02);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x04);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x06);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x07);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x09);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0b);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0d);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0f);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x10);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x12);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x14);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x16);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x18);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x19);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x1b);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x1c);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x1c);
		// cutoff=0.421, window=0.421
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfe);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfe);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xff);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x01);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x02);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x03);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x04);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x06);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x08);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x09);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0b);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0d);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0f);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x10);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x12);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x14);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x15);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x17);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x19);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x1a);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x1b);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x1a);
		// cutoff=0.410, window=0.410
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xfe);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xff);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xff);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x01);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x02);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x03);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x05);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x06);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x08);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x09);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0b);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0d);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0e);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0f);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x12);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x13);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x15);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x17);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x19);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x1a);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x1b);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x1c);
		// cutoff=0.400, window=0.400
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xff);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xff);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x01);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x03);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x04);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x05);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x07);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x08);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0a);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0b);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0d);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0e);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0f);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x11);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x13);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x14);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x16);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x19);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x19);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x1a);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x1a);
		// cutoff=0.390, window=0.390
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xff);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xff);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x01);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x02);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x03);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x04);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x05);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x07);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x08);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0a);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0b);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0d);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0e);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0f);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x11);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x12);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x14);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x16);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x18);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x19);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x1a);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x1a);
		// cutoff=0.381, window=0.381
		dvC789_WriteToBuffer(B6_SCLADPDT, 0xff);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x01);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x02);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x03);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x04);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x06);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x07);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x08);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0a);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0b);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0d);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0e);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0f);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x11);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x12);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x13);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x16);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x19);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x18);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x19);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x1a);
		// cutoff=0.372, window=0.372
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x01);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x02);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x03);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x04);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x05);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x06);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x07);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x09);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0a);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0b);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0d);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0e);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0f);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x11);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x12);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x13);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x15);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x17);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x17);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x17);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x18);
		// cutoff=0.364, window=0.364
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x01);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x02);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x03);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x04);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x05);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x06);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x08);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x09);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0a);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0b);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0d);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0e);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0f);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x10);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x11);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x12);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x15);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x18);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x17);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x18);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x18);
		// cutoff=0.356, window=0.356
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x01);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x01);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x02);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x03);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x04);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x05);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x07);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x08);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x09);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0a);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0b);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0d);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0e);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0f);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x10);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x11);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x12);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x15);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x17);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x17);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x17);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x18);
		// cutoff=0.348, window=0.348
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x01);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x02);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x03);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x04);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x05);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x06);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x07);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x08);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x09);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0a);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0b);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0d);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0e);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0f);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x10);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x11);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x12);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x14);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x16);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x16);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x16);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x16);
		// cutoff=0.340, window=0.340
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x01);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x02);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x03);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x04);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x05);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x06);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x07);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x08);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x09);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0a);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0b);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0d);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0e);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0f);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x10);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x11);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x11);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x14);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x17);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x16);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x16);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x16);
		// cutoff=0.333, window=0.333
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x01);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x02);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x02);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x03);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x04);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x05);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x06);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x07);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x08);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x09);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0a);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0b);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0c);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0e);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0f);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x10);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x10);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x11);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x14);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x16);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x17);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x16);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x16);
		// cutoff=0.327, window=0.327
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x00);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x01);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x02);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x03);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x03);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x04);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x05);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x06);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x07);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x08);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x09);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0a);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0b);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0c);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0d);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0f);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0f);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x10);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x11);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x13);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x16);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x17);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x18);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x16);
		// cutoff=0.320, window=0.320
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x01);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x01);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x02);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x03);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x04);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x05);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x06);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x07);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x07);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x08);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x09);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0a);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0b);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0c);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0d);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0e);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0f);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x10);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x11);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x13);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x15);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x16);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x16);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x16);
		// cutoff=0.314, window=0.314
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x01);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x02);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x02);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x03);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x04);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x05);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x06);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x07);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x08);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x09);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x09);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0a);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0b);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0c);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0d);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0e);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0f);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x10);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x10);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x13);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x16);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x14);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x15);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x16);
		// cutoff=0.308, window=0.308
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x01);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x02);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x03);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x03);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x04);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x05);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x06);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x07);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x08);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x09);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0a);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0a);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0b);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0c);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0d);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0e);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0f);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0f);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x10);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x13);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x14);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x15);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x15);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x16);
		// cutoff=0.302, window=0.302
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x01);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x02);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x03);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x04);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x04);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x05);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x06);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x07);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x08);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x09);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0a);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0a);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0b);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0c);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0d);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0e);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0f);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0f);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x10);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x12);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x14);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x15);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x15);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x16);
		// cutoff=0.296, window=0.296
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x02);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x02);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x03);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x04);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x05);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x05);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x06);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x07);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x08);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x09);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0a);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0a);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0b);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0c);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0d);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0e);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0e);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0f);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x10);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x12);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x13);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x15);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x15);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x16);
		// cutoff=0.291, window=0.291
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x02);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x03);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x03);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x04);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x05);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x06);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x06);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x07);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x08);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x09);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0a);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0a);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0b);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0c);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0d);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0e);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0e);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0f);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0f);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x12);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x14);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x13);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x15);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x16);
		// cutoff=0.286, window=0.286
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x02);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x03);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x03);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x04);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x05);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x06);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x07);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x08);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x08);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x09);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0a);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0a);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0b);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0c);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0d);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0e);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0e);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0f);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0f);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x12);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x14);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x13);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x14);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x14);
		// cutoff=0.281, window=0.281
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x02);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x03);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x04);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x04);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x05);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x06);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x07);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x08);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x08);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x09);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0a);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0b);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0b);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0c);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0d);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0e);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0e);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0f);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0f);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x11);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x13);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x13);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x14);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x14);
		// cutoff=0.276, window=0.276
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x03);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x03);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x04);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x05);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x05);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x06);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x07);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x08);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x08);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x09);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0a);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0b);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0b);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0c);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0d);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0e);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0e);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0e);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0f);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x10);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x13);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x14);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x13);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x14);
		// cutoff=0.271, window=0.271
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x03);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x03);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x04);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x05);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x05);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x06);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x07);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x08);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x08);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x09);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0a);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0b);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0b);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0c);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0d);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0e);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0e);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0e);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0f);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x10);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x13);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x14);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x13);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x14);
		// cutoff=0.267, window=0.267
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x03);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x04);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x04);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x05);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x06);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x06);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x07);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x08);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x08);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x09);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0a);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0b);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0b);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0c);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0d);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0e);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0e);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0e);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0f);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x10);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x12);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x13);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x13);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x14);
		// cutoff=0.262, window=0.262
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x03);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x04);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x04);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x05);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x06);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x06);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x07);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x08);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x09);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x09);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0a);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0b);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0b);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0c);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0d);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0e);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0e);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0e);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0f);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x10);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x12);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x13);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x12);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x14);
		// cutoff=0.258, window=0.258
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x03);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x04);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x05);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x05);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x06);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x07);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x07);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x08);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x09);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x09);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0a);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0b);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0b);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0c);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0c);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0e);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0e);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0e);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0e);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x10);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x12);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x12);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x13);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x14);
		// cutoff=0.254, window=0.254
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x04);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x04);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x05);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x05);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x06);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x07);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x07);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x09);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x09);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x09);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0a);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0b);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0b);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0c);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0c);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0e);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0d);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0e);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0e);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x10);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x12);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x12);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x13);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x12);
		// cutoff=0.250, window=0.250
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x04);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x04);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x05);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x05);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x06);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x07);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x07);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x09);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x09);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x09);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0a);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0b);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0b);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0c);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0c);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0e);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0d);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0e);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0e);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x10);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x12);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x12);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x13);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x12);
		// cutoff=0.246, window=0.246
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x04);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x04);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x05);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x06);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x06);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x07);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x08);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x09);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x09);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x09);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0a);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0b);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0b);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0c);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0c);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0e);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0d);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0e);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0e);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0f);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x12);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x12);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x12);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x12);
		// cutoff=0.242, window=0.242
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x04);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x05);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x05);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x06);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x06);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x07);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x08);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x09);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x09);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x09);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0a);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0b);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0b);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0c);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0c);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0e);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0d);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0e);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0e);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0f);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x12);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x11);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x12);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x12);
		// cutoff=0.239, window=0.239
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x04);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x05);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x05);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x06);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x06);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x07);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x08);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x09);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x09);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x09);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0a);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0b);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0b);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0c);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0c);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0e);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0d);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0d);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0e);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0f);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x12);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x12);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x12);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x12);
		// cutoff=0.235, window=0.235
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x04);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x05);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x05);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x06);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x07);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x07);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x08);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x09);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x09);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x09);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0a);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0b);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0b);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0c);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0c);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0e);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0d);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0d);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0e);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0f);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x11);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x12);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x12);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x12);
		// cutoff=0.232, window=0.232
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x04);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x05);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x06);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x06);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x07);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x07);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x08);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x09);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x09);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x09);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0a);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0b);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0b);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0c);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0c);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0e);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0d);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0d);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0e);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0f);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x10);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x12);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x12);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x12);
		// cutoff=0.229, window=0.229
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x05);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x05);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x06);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x06);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x07);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x07);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x08);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x09);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x09);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x09);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0a);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0b);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0b);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0c);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0c);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0e);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0d);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0d);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0e);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0f);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x10);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x12);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x11);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x12);
		// cutoff=0.225, window=0.225
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x05);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x05);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x06);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x06);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x07);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x07);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x08);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x09);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x09);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0a);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0a);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0b);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0b);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0b);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0c);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0d);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0d);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0d);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0d);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x0f);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x11);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x12);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x11);
		dvC789_WriteToBuffer(B6_SCLADPDT, 0x14);
	}

	dvC789_WriteToBuffer(B6_SCLADPCT,0x00);

    dvC789_Buffer_Flush();

//	GV_RTCT = (GV_RTCT & 0xfc) | 0x02;// [1:0] RTG0 = POVS
	dvC789_Write(BN_RTCT, 0x02);
	dvC789_Write(BN_RTCT, GV_RTCT);
}


void dvC789_AdaptiveScaleFilterLutEnable(UINT8 ucEnable)  //A35G2_Simon_0115
{
    if(ucEnable)
    {
        dvC789_Write(B6_SCLADPCT, 0x03);
        LOG_MSG(db_HAL_WARPING, "Adaptive Warp Filter Lut On\n" );
    }
    else
    {
        dvC789_Write(B6_SCLADPCT, 0x00);
        LOG_MSG(db_HAL_WARPING, "Adaptive Warp Filter Lut Off\n" );
    }
}



