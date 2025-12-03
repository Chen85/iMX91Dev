// ===============================================================================
// FILE NAME: dvC341_Geo.c
// DESCRIPTION:
//
//
// Modification History
// --------------------
// 2017/09/13, Larry Create
// --------------------
// ===============================================================================

#include "Ichipc341_Reg.h"
#include "dvC341_Geo.h"
#include "Board_SPI.h"
#include "utilDbgMsg.h"
#include "utilCounterAPI.h"
#include "opdCtrlAPI.h"
//#include "MemMap.h"

float PM_EBIAS_GAMMA[3];
int PM_EBIAS_GAMMA_COLOR;
char PM_EBIAS_GMDT2[16][3][16];

int                         PM_EBIAS_EN;
int                         PM_EBIAS_SEL_CUR_X;
int                         PM_EBIAS_SEL_CUR_Y;
coord_t                     PM_EBIAS_CUR[2][2];
int                         PM_EBIAS_PLT;
int                         PM_EBIAS[16][3];
int                         PM_EBIAS_AREA_TEST;
int                         PM_EBIAS_CUR_MV;
int                         PM_EBIAS_COLOR;
int                         PM_BOARD;

//===== parameters 1 =====//
int                         PS_WP_HW;
int                         PS_WP_VW;

static UINT8 m_ucCurrentBank = 0;
static UINT32 ulV_START;    //A70LV_Doulas_0042

//#define BURST_MODE_FIXED_ADDRESS    (0x01 << 6)  //1 Serial burst mode: fixed address
//#define BURST_MODE_AUTO_INC_ADDRESS (0x01 << 7)  //2 Serial burst mode: address automatic increment

#define SPI_BUFFER_SIZE             (4 * 1024)
#define CONTROL_341

static UINT8 aucWriteBuffer[SPI_BUFFER_SIZE + 0x100] = {0};
static UINT16 uiWriteIdx = 0;

#ifdef C341GEO_2D_NO_FRAME_LOCK   //A70LV_Doulas_0307
static UINT8  m_ucForcedSyncReset = TRUE;
static UINT16 m_uiH_Total = 2200;
static UINT16 m_uiV_Total = 1125;
#endif
static UINT8  m_ucOutputChanged = FALSE;    //A70LV_Doulas_0307
static UINT8  m_ucFrmaeDelay = 0xff;

//UINT32 PS_WPOSFLD[4] = {0x2c40,
//                        0x1e00,
//                        0x22c0,
//                        0x2780};    //{0x12d4, 0x178c, 0x1c44, 0x20fc};

//0x16200000
//0x0F000000
//0x11600000
//0x13C00000


#if 0 //A70LV_Larry_0332 mask
static TickType_t ulC341StartTicks;

static void dvC341Geo_TimeInState(void)
{
    ulC341StartTicks = xTaskGetTickCount();
    return;
}

UINT32 dvC341Geo_TimeElapsedInState(void)
{
    TickType_t ulTicks;

    ulTicks = xTaskGetTickCount();

    if(ulC341StartTicks > ulTicks)
    {
        return (0xFFFFFFFF - ulC341StartTicks + 1 + ulTicks) / portTICK_RATE_MS;
    }
    else
    {
        return (ulTicks - ulC341StartTicks) / portTICK_RATE_MS;
    }
}
#endif /* 0 */


#if 0
UINT32 dvC341Geo_Read(const UINT32 ulAddr, const UINT8 ucBankOffset)
{
    return dvC341_Read(ulAddr, ucBankOffset);
}

void dvC341Geo_Write(const UINT32 ulAddr, UINT32 ulData, const UINT8 ucBankOffset)
{
    dvC341_Write(ulAddr, ulData, ucBankOffset);
}

void dvC341Geo_Buffer_Flush(void)
{
    dvC341_Buffer_Flush();
}

void dvC341Geo_WriteToBuffer(const UINT32 ulAddr, UINT32 ulData, const UINT8 ucBankOffset)
{
    dvC341_WriteToBuffer(ulAddr, ulData, ucBankOffset);
}

void dvC341Geo_BurstRead_FixedAdd(const UINT32 ulAddr, UINT16 uiSize, UINT8 *pucData)
{
    dvC341_BurstRead_FixedAdd(ulAddr, uiSize, pucData);
}

void dvC341Geo_BurstWrite_FixedAdd(const UINT32 ulAddr, UINT16 uiSize, UINT8 *pucData)
{
    dvC341_BurstWrite_FixedAdd(ulAddr, uiSize, pucData);
}

void dvC341Geo_BurstRead_AddInc(const UINT32 ulAddr, UINT16 uiSize, UINT8 *pucData)
{
    dvC341_BurstRead_AddInc(ulAddr, uiSize, pucData);
}

void dvC341Geo_BurstWrite_AddInc(const UINT32 ulAddr, UINT16 uiSize, UINT8 *pucData)
{
    dvC341_BurstWrite_AddInc(ulAddr, uiSize, pucData);
}
#endif /* 0 */


#if 0
#define dvC341Geo_Write(a,b,c)
#define dvC341Geo_Read(a,b) 0
#define dvC341Geo_Buffer_Flush()
#define dvC341Geo_WriteToBuffer(a,b,c)
#define dvC341Geo_BurstRead_FixedAdd(a,b,c)
#define dvC341Geo_BurstWrite_FixedAdd(a,b,c)
#define dvC341Geo_BurstRead_AddInc(a,b,c)
#define dvC341Geo_BurstWrite_AddInc(a,b,c)
#endif

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
            20,//32,		                    //VStart        //A70LV_Doulas_0170 TEST
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
            41,                         //VStart
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

    //XXXXX
    {
        ePANEL_ID_4096x2160_50HZ,           //Timing Id
        {
            0,			                //FrameRate
            0,                      //PixelClock
            0,	                    //HStart
            0,                       //HActive
            0,                         //VStart
            0,                       //VActive
            0,                       //HTotal
            0,                       //VTotal
            0,                         //H SyncWidth
            0,                         //V SyncWidth
            0,                          //POREFDIV
            0,                          //POFBDIV
        },
    },

    {
        ePANEL_ID_3840x2160_60HZ,//Timing Id
        {
            60,			                //FrameRate
            60000,                      //PixelClock
            384,	                    //HStart
            3840,                       //HActive
            20,                         //VStart
            2160,                       //VActive
            4560,                       //HTotal
            2194,                       //VTotal
            96,                         //H SyncWidth
            5,                          //V SyncWidth
            2,                          //POREFDIV
            20,                         //POFBDIV
        },
    },

    {
        ePANEL_ID_3840x2160_30HZ,//Timing Id
        {
            0,			                //FrameRate
            0,                      //PixelClock
            0,	                    //HStart
            0,                       //HActive
            0,                         //VStart
            0,                       //VActive
            0,                       //HTotal
            0,                       //VTotal
            0,                         //H SyncWidth
            0,                          //V SyncWidth
            0,                          //POREFDIV
            0,                         //POFBDIV
        },
    },

    {
        ePANEL_ID_3840x2160_60HZ_594MHz,//Timing Id
        {
            0,			                //FrameRate
            0,                      //PixelClock
            0,	                    //HStart
            0,                       //HActive
            0,                         //VStart
            0,                       //VActive
            0,                       //HTotal
            0,                       //VTotal
            0,                         //H SyncWidth
            0,                          //V SyncWidth
            0,                          //POREFDIV
            0,                         //POFBDIV
        },
    },

    {
        ePANEL_ID_3840x2400_60HZ,//Timing Id
        {
            60,			                //FrameRate
            60000,                      //PixelClock
            80,	                        //HStart
            3840,                       //HActive
            20,                         //VStart
            2400,                       //VActive
            4064,                       //HTotal
            2460,                       //VTotal
            64,                         //H SyncWidth
            10,                         //V SyncWidth
            2,                          //POREFDIV
            20,                         //POFBDIV
        },
    },


    {
        ePANEL_ID_3840x2400_50HZ,//Timing Id
        {
            0,			                //FrameRate
            0,                      //PixelClock
            0,	                    //HStart
            0,                       //HActive
            0,                         //VStart
            0,                       //VActive
            0,                       //HTotal
            0,                       //VTotal
            0,                         //H SyncWidth
            0,                          //V SyncWidth
            0,                          //POREFDIV
            0,                         //POFBDIV
        },
    },


    {
        ePANEL_ID_3840x2400_30HZ,//Timing Id
        {
            0,			                //FrameRate
            0,                      //PixelClock
            0,	                    //HStart
            0,                       //HActive
            0,                         //VStart
            0,                       //VActive
            0,                       //HTotal
            0,                       //VTotal
            0,                         //H SyncWidth
            0,                          //V SyncWidth
            0,                          //POREFDIV
            0,                         //POFBDIV
        },
    },



    {
        ePANEL_ID_4096x2160_60HZ,//Timing Id
        {
            0,			                //FrameRate
            0,                      //PixelClock
            0,	                    //HStart
            0,                       //HActive
            0,                         //VStart
            0,                       //VActive
            0,                       //HTotal
            0,                       //VTotal
            0,                         //H SyncWidth
            0,                          //V SyncWidth
            0,                          //POREFDIV
            0,                         //POFBDIV
        },
    },

    {
        ePANEL_ID_960x2160_50HZ,        //Timing Id
        {
            0,			                //FrameRate
            0,                      //PixelClock
            0,	                    //HStart
            0,                       //HActive
            0,                         //VStart
            0,                       //VActive
            0,                       //HTotal
            0,                       //VTotal
            0,                         //H SyncWidth
            0,                          //V SyncWidth
            0,                          //POREFDIV
            0,                         //POFBDIV
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

    {
        ePANEL_ID_1280x2160_50HZ,        //Timing Id
        {
            0,			                //FrameRate
            0,                      //PixelClock
            0,	                    //HStart
            0,                       //HActive
            0,                         //VStart
            0,                       //VActive
            0,                       //HTotal
            0,                       //VTotal
            0,                         //H SyncWidth
            0,                          //V SyncWidth
            0,                          //POREFDIV
            0,                         //POFBDIV
        },
    },

    {
        ePANEL_ID_1280x2160_60HZ,        //Timing Id
        {
            0,			                //FrameRate
            0,                      //PixelClock
            0,	                    //HStart
            0,                       //HActive
            0,                         //VStart
            0,                       //VActive
            0,                       //HTotal
            0,                       //VTotal
            0,                         //H SyncWidth
            0,                          //V SyncWidth
            0,                          //POREFDIV
            0,                         //POFBDIV
        },
    },

    {
        ePANEL_ID_2560x1440_60HZ,        //Timing Id
        {
            0,			                //FrameRate
            0,                      //PixelClock
            0,	                    //HStart
            0,                       //HActive
            0,                         //VStart
            0,                       //VActive
            0,                       //HTotal
            0,                       //VTotal
            0,                         //H SyncWidth
            0,                          //V SyncWidth
            0,                          //POREFDIV
            0,                         //POFBDIV
        },
    },

    {
        ePANEL_ID_2560x1600_60HZ,        //Timing Id
        {
            0,			                //FrameRate
            0,                      //PixelClock
            0,	                    //HStart
            0,                       //HActive
            0,                         //VStart
            0,                       //VActive
            0,                       //HTotal
            0,                       //VTotal
            0,                         //H SyncWidth
            0,                          //V SyncWidth
            0,                          //POREFDIV
            0,                         //POFBDIV
        },
    },


    // 3D  ///////////////////////////////////////////////////////////////
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
            13500,//27000,//29700,                      //PixelClock
            24,//41,//192,                        //HStart
            960,//1920,                       //HActive
            41,                         //VStart
            1080,                       //VActive
            1000,//2000,//2200,                       //HTotal
            1125,                       //VTotal
            21,                         //H SyncWidth
            4,                          //V SyncWidth
            //TODO
            2,                          //POREFDIV
            10,//20,//22,                         //POFBDIV
        },
    },

    {
        ePANEL_ID_1080P_240HZ,   //Timing Id
        {
            240,                        //FrameRate
            60000,                      //PixelClock
            192,                        //HStart
            1920,                       //HActive
            10,                         //VStart
            1080,                       //VActive
            2248,                       //HTotal
            1112,                       //VTotal
            92,                         //H SyncWidth
            4,                          //V SyncWidth
            2,                          //POREFDIV
            20,                         //POFBDIV
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
#if 1   //A70LV_Doulas_0377 Modify
        {
            120,                        //FrameRate
            6000,//30825,               //PixelClock    154128000x2
            80,//112,                   //HStart
            1920,//1920,                //HActive
            20,//32,                    //VStart
            1200,                       //VActive
            3064,//2004,//2080,         //HTotal
            1634,                       //VTotal
            64,                         //H SyncWidth
            10,                         //V SyncWidth
            2,                          //POREFDIV
            20,                         //POFBDIV
        },
#else
        {
            120,                        //FrameRate
            30825,                      //PixelClock    154128000x2
            112,                        //HStart
            1920,                       //HActive
            32,                         //VStart
            1200,                       //VActive
            2080,                       //HTotal
            1235,                       //VTotal
            15,                         //H SyncWidth
            5,                          //V SyncWidth
            2,                          //POREFDIV
            20,                         //POFBDIV
        },
#endif
    },

    {
        ePANEL_ID_WUXGA_240HZ,           //Timing Id
        {
            240,                        //FrameRate
            60000,                      //PixelClock
            40,                         //HStart
            1920,                       //HActive
            10,                         //VStart
            1200,                       //VActive
            2032,                       //HTotal
            1230,                       //VTotal
            32,                         //H SyncWidth
            5,                          //V SyncWidth
            2,                          //POREFDIV
            20,                         //POFBDIV
        },
    },
};


static const UINT8 m_ucPanelTblCount = sizeof(m_sPanelTable) / sizeof(sPANEL_INFO);

static sDRV_C341GEO_INFO m_sDrvC341Info;
static UINT8 ucC341_OutputEnable = TRUE;    //A70LV_Doulas_0079
static UINT8 ucColorUniformityEnable = 0;

static char cLastTakeC341Semaphore[128] = {'\0'};  //A35G2_Simon_0112


void dvC341Geo_OPDMutexLockEvent(UINT16 uiEvent, const char *pcFunc)  //A35G2_Simon_0112
{
    uOPD_DATA uOPDData ;

    snprintf(uOPDData.cString, 128,  "%s,%s", pcFunc, cLastTakeC341Semaphore);

    utilOPD_EventSet(uiEvent, &uOPDData);
}

BOOL dvC341Geo_SemaphoreTake(BOOL cEnable, const char *pcSemaphore)
{
    BOOL bResult = FALSE;

    if(cEnable)
    {
        bResult = Board_SSP_SemaphoreTake(eBOARD_SSP_CS_C341);

        if(bResult == FALSE)
        {
            dvC341Geo_OPDMutexLockEvent(eOPD_MUTEX_LOCKED_LOG, pcSemaphore);   //A35G2_Simon_0112
            LOG_MSG(db_DV_SEMAPHORE, "(func:%s) is Fail, last take semaphore func %s\r\n", pcSemaphore, cLastTakeC341Semaphore);
        }
        else
        {
            sprintf(cLastTakeC341Semaphore, "%s", pcSemaphore);
        }
    }
    else
    {
        bResult = Board_SSP_SemaphoreGive(eBOARD_SSP_CS_C341);

        sprintf(cLastTakeC341Semaphore, "");
    }

    return bResult;
}

static eC341GEO_EXEC_CODE dvC341Geo_GetPanelIndex(ePANEL_ID ePanelId, PUINT8 pucIndex)
{
    UINT8 ucIndex = 0;

    if(ePanelId < ePANEL_ID_LAST)
    {
        for(ucIndex = 0; ucIndex < m_ucPanelTblCount; ucIndex++)
        {
            if(m_sPanelTable[ucIndex].ePanelId == ePanelId)
            {
                *pucIndex = ucIndex;
                return eC341GEO_EXEC_CODE_PASS;
            }
        }
    }
    else
    {
        return eC341GEO_EXEC_CODE_PANEL_ID_NOT_FOUND;
    }

    return eC341GEO_EXEC_CODE_PANEL_TABEL_INDEX_NOT_FOUND;
}

static void dvC341Geo_GetOutputTiming(const UINT8 ucPanelIndex)
{
    LOG_MSG(db_DV_GEO, "(func:%s, line:%d)Output Res[%d x %d %dHz]\n", __FUNCTION__, __LINE__,
                    m_sPanelTable[ucPanelIndex].sOutputTimingInfo.uiHActive,
                    m_sPanelTable[ucPanelIndex].sOutputTimingInfo.uiVActive,
                    m_sPanelTable[ucPanelIndex].sOutputTimingInfo.ucFrameRate);

    m_sDrvC341Info.sOutputTimingInfo.ucFrameRate = m_sPanelTable[ucPanelIndex].sOutputTimingInfo.ucFrameRate;
    m_sDrvC341Info.sOutputTimingInfo.uiPixClk = m_sPanelTable[ucPanelIndex].sOutputTimingInfo.uiPixClk;
    m_sDrvC341Info.sOutputTimingInfo.uiHStart = m_sPanelTable[ucPanelIndex].sOutputTimingInfo.uiHStart;
    m_sDrvC341Info.sOutputTimingInfo.uiHActive = m_sPanelTable[ucPanelIndex].sOutputTimingInfo.uiHActive;
    m_sDrvC341Info.sOutputTimingInfo.uiVStart = m_sPanelTable[ucPanelIndex].sOutputTimingInfo.uiVStart;
    m_sDrvC341Info.sOutputTimingInfo.uiVActive = m_sPanelTable[ucPanelIndex].sOutputTimingInfo.uiVActive;
    m_sDrvC341Info.sOutputTimingInfo.uiHTotal = m_sPanelTable[ucPanelIndex].sOutputTimingInfo.uiHTotal;
    m_sDrvC341Info.sOutputTimingInfo.uiVTotal = m_sPanelTable[ucPanelIndex].sOutputTimingInfo.uiVTotal;
    m_sDrvC341Info.sOutputTimingInfo.ucHSyncWidth = m_sPanelTable[ucPanelIndex].sOutputTimingInfo.ucHSyncWidth;
    m_sDrvC341Info.sOutputTimingInfo.ucVSyncWidth = m_sPanelTable[ucPanelIndex].sOutputTimingInfo.ucVSyncWidth;

    ulV_START = m_sDrvC341Info.sOutputTimingInfo.uiVStart;    //A70LV_Doulas_0042
}

static void dvC341Geo_ConfigInput(void)        //A70LV_Doulas_0036
{
    LOG_MSG(db_DV_GEO , "(func:%s, line:%d)\n", __FUNCTION__, __LINE__);

    #if 0

    m_sDrvC341Info.ucRegISYCT = 0x31;    //A35G2_CDS_Simon_0003

//    dvC341Geo_Write(B0_IP2SCT,     0x00, 0);

//    dvC341Geo_Write(BN_MCT,        0x51, 0);    //Enable Image Output

    dvC341Geo_Write(B3_ISYCT,      m_sDrvC341Info.ucRegISYCT, 0);  //0x31    //A35G2_CDS_Simon_0003
    dvC341Geo_Write(B3_AUTOFLD,    0x80, 0);   //Field auto recognition
    dvC341Geo_Write(B3_DIFCT,      0x83, 0);   //Enable DE

    dvC341Geo_Write(B3_IACTHST,    m_sDrvC341Info.sOutputTimingInfo.uiHStart-7, 0);//-8);  //A70LV_Doulas_0045 modify
    dvC341Geo_Write(B3_IACTHW,     m_sDrvC341Info.sOutputTimingInfo.uiHActive, 0);
    dvC341Geo_Write(B3_IACTVST,    m_sDrvC341Info.sOutputTimingInfo.uiVStart, 0);
    dvC341Geo_Write(B3_IACTVW,     m_sDrvC341Info.sOutputTimingInfo.uiVActive, 0);

    dvC341Geo_Write(B3_ISWP,	0x00, 0);

    //Must reset Input Block
//    dvC341Geo_Write(BN_RSTCT,      0x0C, 0);
//    dvC341Geo_Write(B0_IP2SCT,     0x02, 0);
    #endif
}

void dvC341Geo_ConfigOutput(void)
{
    UINT16 uiOHScycl;
    UINT16 uiOVScycl;
    //UINT16 uiOVScyclmin;  //A70LV_Doulas_0072 modify
    LOG_MSG(db_DV_GEO , "(func:%s, line:%d) LLM %d\n", __FUNCTION__, __LINE__, m_sDrvC341Info.sOutputTimingInfo.eLow_Latency);

    UINT32 Wpisfld[4];
    Wpisfld[0] = DEF_WPISFLD0CH1;
    Wpisfld[1] = DEF_WPISFLD1CH1;
    Wpisfld[2] = DEF_WPISFLD2CH1;
    Wpisfld[3] = DEF_WPISFLD3CH1;

    int oaoihst;
    int oaoivst;
    int syren;
    int ch, bank_ofst;
    UINT32 ulWhw = 0, ad_ofst = 0;
    UINT16 dev16 = 0, mod16 = 0;

    oaoihst = m_sDrvC341Info.sOutputTimingInfo.uiHStart / OUTPUT_CH_NUM + 4; //40   //dvC341_Read(B8_OAOI0HSTCH1, 0); // 40 //dvC341_Read(B144_WPOAOI0HSTCH1, 0);;  //196;   //dvC341_Read(B144_WPOAOI0HSTCH1, 0);
    oaoivst = m_sDrvC341Info.sOutputTimingInfo.uiVStart + 1;   //12;//dvC341_Read(B8_OAOI0VSTCH1, 0); //10;//dvC341_Read(B144_WPOAOI0VSTCH1, 0);;  //83;    //dvC341_Read(B144_WPOAOI0VSTCH1, 0);

    LOG_MSG(db_HAL_WARPING, "oaoihst oaoivst <%d %d>\r\n", oaoihst, oaoivst);
    //LOG_MSG(db_HAL_WARPING, "oaoihst oaoivst <%d %d>\r\n", dvC341_Read(B8_OAOI0HSTCH1, 0), dvC341_Read(B8_OAOI0VSTCH1, 0));  //40 17

    syren = ((dvC341_Read(B144_WPOSYCTCH1, 0) & 0x0002) == 0) ? 0 : 1;

    for (ch = 0; ch < 4; ch++)
    {
        if (ch < 2)
        {
            // CH1-2
            ///// Warping
            bank_ofst = (ch * 5);

            // Warping
            if (ch == 0)
            {
                // CH1
                dvC341_WriteToBuffer(B144_WPBLKCTCH1, 0x03, bank_ofst);
                dvC341_WriteToBuffer(B144_WPMCTCH1,   0x11, bank_ofst);
                dvC341_WriteToBuffer(B144_WPOSYCTCH1, (0x0001 + (syren << 1)), bank_ofst);
                dvC341_WriteToBuffer(B145_WPWTRCTCH1, 0x00, bank_ofst);
                dvC341_WriteToBuffer(B145_WPLBFCTCH1, 0x00, bank_ofst);
            }
            else
            {
                // CH2
                dvC341_WriteToBuffer(B144_WPBLKCTCH1, 0x03, bank_ofst);
                dvC341_WriteToBuffer(B144_WPMCTCH1,   0x11, bank_ofst);
                dvC341_WriteToBuffer(B144_WPOSYCTCH1, (0x1011 + (syren << 1)), bank_ofst);
                dvC341_WriteToBuffer(B145_WPWTRCTCH1, 0x00, bank_ofst);
                dvC341_WriteToBuffer(B145_WPLBFCTCH1, 0x00, bank_ofst);
            }

            /*
            ePANEL_ID_3840x2400_60HZ,//Timing Id
            {
                60,			                //FrameRate
                60000,                      //PixelClock
                80,	                        //HStart
                3840,                       //HActive
                20,                         //VStart
                2400,                       //VActive
                4064,                       //HTotal
                2460,                       //VTotal
                64,                         //H SyncWidth
                10,                         //V SyncWidth
                2,                          //POREFDIV
                20,                         //POFBDIV
            },
            */

            dvC341_WriteToBuffer(B144_WPOHCYCLCH1, m_sDrvC341Info.sOutputTimingInfo.uiHTotal/2 - 2, bank_ofst);   //4064 x 2460
            dvC341_WriteToBuffer(B144_WPOVCYCLCH1, m_sDrvC341Info.sOutputTimingInfo.uiVTotal   - 2, bank_ofst);

            dvC341_WriteToBuffer(B144_WPOAOI0HSTCH1,  oaoihst           , bank_ofst);
            dvC341_WriteToBuffer(B144_WPOAOI0HENDCH1, oaoihst + m_sDrvC341Info.sOutputTimingInfo.uiHActive/2    , bank_ofst);
            dvC341_WriteToBuffer(B144_WPOAOI0VSTCH1,  oaoivst           , bank_ofst);
            dvC341_WriteToBuffer(B144_WPOAOI0VENDCH1, oaoivst + m_sDrvC341Info.sOutputTimingInfo.uiVActive    , bank_ofst);

            //for output WUXGA 240Hz bandwidth issue
            if(m_sDrvC341Info.ePanelTimingId == ePANEL_ID_WUXGA_240HZ)   //H2PF_Simon_0107   //H2PF_Simon_133
            {
                //for output WUXGA 240Hz bandwidth issue
                if(ch == 0)
                {
                    dvC341_WriteToBuffer(B145_WPIACTHWCH1, 0x780, 0);
                    dvC341_WriteToBuffer(B144_WOR1ACCTCH1, 0, 0);
                    dvC341_WriteToBuffer(B144_WOR2ACCTCH1, 0, 0);
                    dvC341_WriteToBuffer(B144_WOR3ACCTCH1, 0, 0);
                    dvC341_WriteToBuffer(B144_WOR4ACCTCH1, 0, 0);
                    dvC341_WriteToBuffer(B144_WOR5ACCTCH1, 0, 0);
                    dvC341_WriteToBuffer(B144_WOR6ACCTCH1, 0, 0);
                }
                else if(ch == 1)
                {
                    dvC341_WriteToBuffer(B150_WPIACTHWCH2, 0, 0);
                    dvC341_WriteToBuffer(B149_WOR1ACCTCH2, 0x3F, 0);
                    dvC341_WriteToBuffer(B149_WOR2ACCTCH2, 0x3F, 0);
                    dvC341_WriteToBuffer(B149_WOR3ACCTCH2, 0x3F, 0);
                    dvC341_WriteToBuffer(B149_WOR4ACCTCH2, 0x3F, 0);
                    dvC341_WriteToBuffer(B149_WOR5ACCTCH2, 0x3F, 0);
                    dvC341_WriteToBuffer(B149_WOR6ACCTCH2, 0x3F, 0);
                }
            }
            else
            {
                dvC341_WriteToBuffer(B145_WPIACTHWCH1, m_sDrvC341Info.sOutputTimingInfo.uiHActive/2, bank_ofst);

                if(ch == 0)
                {
                    dvC341_WriteToBuffer(B144_WOR1ACCTCH1, 0x80, 0);
                    dvC341_WriteToBuffer(B144_WOR2ACCTCH1, 0, 0);
                    dvC341_WriteToBuffer(B144_WOR3ACCTCH1, 0, 0);
                    dvC341_WriteToBuffer(B144_WOR4ACCTCH1, 0, 0);
                    dvC341_WriteToBuffer(B144_WOR5ACCTCH1, 0, 0);
                    dvC341_WriteToBuffer(B144_WOR6ACCTCH1, 0x3F, 0);
                }
                else if(ch == 1)
                {
                    dvC341_WriteToBuffer(B149_WOR1ACCTCH2, 0x80, 0);
                    dvC341_WriteToBuffer(B149_WOR2ACCTCH2, 0, 0);
                    dvC341_WriteToBuffer(B149_WOR3ACCTCH2, 0, 0);
                    dvC341_WriteToBuffer(B149_WOR4ACCTCH2, 0, 0);
                    dvC341_WriteToBuffer(B149_WOR5ACCTCH2, 0, 0);
                    dvC341_WriteToBuffer(B149_WOR6ACCTCH2, 0x3F, 0);
                }
            }

            dvC341_WriteToBuffer(B145_WPIACTVWCH1, m_sDrvC341Info.sOutputTimingInfo.uiVActive, bank_ofst);
    //      write_C341(B145_WPIACTSELCH1, 0x03, bank_ofst);// sel AOI2// v02
            dvC341_WriteToBuffer(B145_WPIACTSELCH1, 0x01, bank_ofst);// sel AOI0// v02

            dvC341_WriteToBuffer(B148_WPOSFLD0CH1, (DEF_WPOSFLD0CH1 >> 15), bank_ofst);
            dvC341_WriteToBuffer(B148_WPOSFLD1CH1, (DEF_WPOSFLD1CH1 >> 15), bank_ofst);
            dvC341_WriteToBuffer(B148_WPOSFLD2CH1, (DEF_WPOSFLD2CH1 >> 15), bank_ofst);
            dvC341_WriteToBuffer(B148_WPOSFLD3CH1, (DEF_WPOSFLD3CH1 >> 15), bank_ofst);

            // ISFLD
            dev16 = ulWhw / 16;
            dev16 = dev16 * 16;
            mod16 = ulWhw - dev16;
            ad_ofst = dev16 * 16 + mod16 * 4;

            if( (m_sDrvC341Info.ePanelTimingId == PANEL_2D_OUTPUT && m_sDrvC341Info.ucIsDefaultWarpTable == TRUE) ||    //use 30 bit process      //H2PF_Simon_0131
                WARP_ON_USE_FRAME_MEMORY_YUV422 == TRUE)  //H2PF_Simon_0137
            {
                dvC341_WriteToBuffer(B144_WPOSCWCTCH1, 0, bank_ofst);
                dvC341_WriteToBuffer(B144_WPISCWCTCH1, 0, bank_ofst);
                dvC341_WriteToBuffer(B148_WPISFLD0CH1, (Wpisfld[0] + ad_ofst), bank_ofst);
                dvC341_WriteToBuffer(B148_WPISFLD1CH1, (Wpisfld[1] + ad_ofst), bank_ofst);
                dvC341_WriteToBuffer(B148_WPISFLD2CH1, (Wpisfld[2] + ad_ofst), bank_ofst);
                dvC341_WriteToBuffer(B148_WPISFLD3CH1, (Wpisfld[3] + ad_ofst), bank_ofst);
            }
            else   //use 24 bit process
            {
                dvC341_WriteToBuffer(B144_WPOSCWCTCH1, 0x80, bank_ofst);
                dvC341_WriteToBuffer(B144_WPISCWCTCH1, 0x80, bank_ofst);
                dvC341_WriteToBuffer(B148_WPISFLD0CH1, (Wpisfld[0] + (ad_ofst*24/30)), bank_ofst);
                dvC341_WriteToBuffer(B148_WPISFLD1CH1, (Wpisfld[1] + (ad_ofst*24/30)), bank_ofst);
                dvC341_WriteToBuffer(B148_WPISFLD2CH1, (Wpisfld[2] + (ad_ofst*24/30)), bank_ofst);
                dvC341_WriteToBuffer(B148_WPISFLD3CH1, (Wpisfld[3] + (ad_ofst*24/30)), bank_ofst);
            }

            ulWhw = ulWhw + (m_sDrvC341Info.sOutputTimingInfo.uiHActive/2);


            ///// Output side
            // OSYNC
            dvC341_WriteToBuffer(B144_WPFLDDLYCH1, (oaoivst + m_sDrvC341Info.sOutputTimingInfo.uiVActive * 2 / 3), bank_ofst);
            // OACT
            dvC341_WriteToBuffer(B144_WPOACTHSTCH1, oaoihst, bank_ofst);
            dvC341_WriteToBuffer(B144_WPOACTHWCH1, m_sDrvC341Info.sOutputTimingInfo.uiHActive/2, bank_ofst);
            dvC341_WriteToBuffer(B144_WPOACTVSTCH1, oaoivst, bank_ofst);
            dvC341_WriteToBuffer(B144_WPOACTVWCH1, m_sDrvC341Info.sOutputTimingInfo.uiVActive, bank_ofst);
            dvC341_WriteToBuffer(B144_WPOAOI0HENDCH1, (oaoihst + (m_sDrvC341Info.sOutputTimingInfo.uiHActive/2)), bank_ofst);
            dvC341_WriteToBuffer(B144_WPOAOI0VENDCH1, (oaoivst + m_sDrvC341Info.sOutputTimingInfo.uiVActive), bank_ofst);
            ///// Input side
            bank_ofst = (ch * 16);
            // Measure Input
            /*
            write_C341(B16_PTHCH1, 0x80, bank_ofst);
            rdt = read_C341(B16_DIFCTCH1, bank_ofst);
            write_C341(B16_DIFCTCH1, (rdt & 0xfe), bank_ofst);
            rdt = read_C341(B16_PIH0CH1, bank_ofst);
            write_C341(B16_PIH0CH1, (rdt & 0xbfff), bank_ofst);
            */
            // Warping Cursor

        }
        else
        {// CH3-4
            ///// Warping
            bank_ofst = (ch * 5);

            // Warping
            dvC341_WriteToBuffer(B144_WPMCTCH1, 0x00, bank_ofst);
            dvC341_WriteToBuffer(B144_WPOSYCTCH1, 0x0000, bank_ofst);
            dvC341_WriteToBuffer(B145_WPDTCTCH1, 0x0000, bank_ofst);
            dvC341_WriteToBuffer(B145_WPWTRCTCH1, 0x00, bank_ofst);
            dvC341_WriteToBuffer(B145_WPLBFCTCH1, 0x00, bank_ofst);
            dvC341_WriteToBuffer(B145_WPIACTHWCH1, 0, bank_ofst);
            dvC341_WriteToBuffer(B145_WPIACTVWCH1, 0, bank_ofst);

            ///// Output side
            // OACT
            dvC341_WriteToBuffer(B144_WPOACTHWCH1, 0, bank_ofst);
            dvC341_WriteToBuffer(B144_WPOACTVWCH1, 0, bank_ofst);
        }
    }

    dvC341_WriteToBuffer(B144_WPSYRDLYCH1, 0x02, 0);   //H2PF_Simon_0150   //H2PF_Simon_0187
    dvC341_WriteToBuffer(B144_WPSYRDLYCH1, 0x02, CH_WPBANK_OFFSET);
    dvC341_WriteToBuffer(B144_WPFLDDLYCH1, (oaoivst + m_sDrvC341Info.sOutputTimingInfo.uiVActive * 2 / 3), 0);
    dvC341_WriteToBuffer(B144_WPFLDDLYCH1, (oaoivst + m_sDrvC341Info.sOutputTimingInfo.uiVActive * 2 / 3), CH_WPBANK_OFFSET);
    MS_SLEEP(100);

    if(m_sDrvC341Info.ePanelTimingId == PANEL_2D_OUTPUT) //H30K_Doulas_0010
    {
        if(m_sDrvC341Info.sOutputTimingInfo.eLow_Latency > eLLM_OFF && m_sDrvC341Info.sOutputTimingInfo.eLow_Latency < eLLM_INVALID)  //low latency on     //H2PF_Simon_0150
        {
            dvC341_WriteToBuffer(B144_WPOSYCTCH1, (0x0003 ), 0);
            dvC341_WriteToBuffer(B144_WPOSYCTCH1, (0x1013 ), 5);

            dvC341_WriteToBuffer(B144_WPOVCYCLCH1, m_sDrvC341Info.sOutputTimingInfo.uiVTotal * 1.1  - 2, 0); //H30K_Doulas_0012
            dvC341_WriteToBuffer(B144_WPOVCYCLCH1, m_sDrvC341Info.sOutputTimingInfo.uiVTotal * 1.1  - 2, 5); //H30K_Doulas_0012

            dvC341_WriteToBuffer(B144_WPOSYCT2CH1, 0x0, 0); //H30K_Doulas_0016
            dvC341_WriteToBuffer(B144_WPOSYCT2CH1, 0x0, 5);
            dvC341_WriteToBuffer(B144_WPOSYCT3CH1, 0x0, 0);
            dvC341_WriteToBuffer(B144_WPOSYCT3CH1, 0x0, 5);

            switch(m_sDrvC341Info.sOutputTimingInfo.eLow_Latency)   //H2PF_Simon_0150
            {
                case eLLM_LV1_FAST_WITH_FULL_WARPING:
                    dvC341_WriteToBuffer(B144_WPSYRDLYCH1, 0x02, 0);   //H2PF_Simon_0187
                    dvC341_WriteToBuffer(B144_WPSYRDLYCH1, 0x02, CH_WPBANK_OFFSET);
                    dvC341_WriteToBuffer(B144_WPFLDDLYCH1, (oaoivst + m_sDrvC341Info.sOutputTimingInfo.uiVActive * 2 / 3), 0);
                    dvC341_WriteToBuffer(B144_WPFLDDLYCH1, (oaoivst + m_sDrvC341Info.sOutputTimingInfo.uiVActive * 2 / 3), CH_WPBANK_OFFSET);
                    break;

                case eLLM_LV2_FAST_WITH_LIMITED_WARPING:
                default:
                    dvC341_WriteToBuffer(B144_WPSYRDLYCH1, SYRDLY_REG_FOR_LIMITED_WARP, 0);
                    dvC341_WriteToBuffer(B144_WPSYRDLYCH1, SYRDLY_REG_FOR_LIMITED_WARP, CH_WPBANK_OFFSET);
                    dvC341_WriteToBuffer(B144_WPFLDDLYCH1, FLDDLY_REG_FOR_LIMITED_WARP, 0);
                    dvC341_WriteToBuffer(B144_WPFLDDLYCH1, FLDDLY_REG_FOR_LIMITED_WARP, CH_WPBANK_OFFSET);
                    break;

                case eLLM_LV3_FAST_WITHOUT_WARPING:
                    break;

            }
        }
        else
        {
            dvC341_WriteToBuffer(B144_WPOSYCTCH1, (0x0001 ), 0);
            dvC341_WriteToBuffer(B144_WPOSYCTCH1, (0x1011 ), 5);
            dvC341_WriteToBuffer(B144_WPOSYCT2CH1, 0x0, 0);
            dvC341_WriteToBuffer(B144_WPOSYCT2CH1, 0x0, 5);
            dvC341_WriteToBuffer(B144_WPOSYCT3CH1, 0x0, 0);
            dvC341_WriteToBuffer(B144_WPOSYCT3CH1, 0x0, 5);
            dvC341_WriteToBuffer(B144_WPSYRDLYCH1, 0x0, 0);
            dvC341_WriteToBuffer(B144_WPSYRDLYCH1, 0x0, 5);
            dvC341_WriteToBuffer(B144_WPFLDDLYCH1, (oaoivst + m_sDrvC341Info.sOutputTimingInfo.uiVActive * 2 / 3), 0);   //H2PF_Simon_0150
            dvC341_WriteToBuffer(B144_WPFLDDLYCH1, (oaoivst + m_sDrvC341Info.sOutputTimingInfo.uiVActive * 2 / 3), CH_WPBANK_OFFSET);
        }

    }
    else if(m_sDrvC341Info.ePanelTimingId == PANEL_3D_OUTPUT) //H30K_Doulas_0016
    {
        if((dvC341_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_SIDEBYSIDE) ||
            (dvC341_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_FRAMEPACKING) ||
            (dvC341_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_TOPANDBOTTOM))
        {
            dvC341_WriteToBuffer(B144_WPOSYCTCH1, (0x8003 ), 0); //H30K_Doulas_0038
            dvC341_WriteToBuffer(B144_WPOSYCTCH1, (0x1013 ), 5);
        }
        else
        {
            if(dvC341_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_FRAME_SEQUENTIAL) //H30K_Doulas_0034
            {
                dvC341_WriteToBuffer(B144_WPOSYCTCH1, (0x0003 ), 0);
                dvC341_WriteToBuffer(B144_WPOSYCTCH1, (0x1013 ), 5);
            }
            else
            {
                dvC341_WriteToBuffer(B144_WPOSYCTCH1, (0x0005 ), 0);
                dvC341_WriteToBuffer(B144_WPOSYCTCH1, (0x1015 ), 5);
            }
        }
        dvC341_WriteToBuffer(B144_WPOVCYCLCH1, m_sDrvC341Info.sOutputTimingInfo.uiVTotal * 1.1  - 2, 0);
        dvC341_WriteToBuffer(B144_WPOVCYCLCH1, m_sDrvC341Info.sOutputTimingInfo.uiVTotal * 1.1  - 2, 5);

        dvC341_WriteToBuffer(B144_WPOSYCT2CH1, 0x0, 0);
        dvC341_WriteToBuffer(B144_WPOSYCT2CH1, 0x0, 5);
        dvC341_WriteToBuffer(B144_WPOSYCT3CH1, 0x80, 0);
        dvC341_WriteToBuffer(B144_WPOSYCT3CH1, 0x80, 5);
        dvC341_WriteToBuffer(B144_WPSYRDLYCH1, 0x02, 0);   //H2PF_Simon_0187
        dvC341_WriteToBuffer(B144_WPSYRDLYCH1, 0x02, 5);
        dvC341_WriteToBuffer(B144_WPFLDDLYCH1, (oaoivst + m_sDrvC341Info.sOutputTimingInfo.uiVActive * 2 / 3), 0);
        dvC341_WriteToBuffer(B144_WPFLDDLYCH1, (oaoivst + m_sDrvC341Info.sOutputTimingInfo.uiVActive * 2 / 3), CH_WPBANK_OFFSET);
    }
    else  //240Hz Panel
    {
        if(m_sDrvC341Info.sOutputTimingInfo.eLow_Latency > eLLM_OFF && m_sDrvC341Info.sOutputTimingInfo.eLow_Latency < eLLM_INVALID) //H30K_Doulas_0018     //H2PF_Simon_0150
        {
            dvC341_WriteToBuffer(B144_WPOSYCTCH1, (0x0003 ), 0);
            dvC341_WriteToBuffer(B144_WPOSYCTCH1, (0x1013 ), 5);
            dvC341_WriteToBuffer(B144_WPOVCYCLCH1, m_sDrvC341Info.sOutputTimingInfo.uiVTotal * 1.1  - 2, 0);
            dvC341_WriteToBuffer(B144_WPOVCYCLCH1, m_sDrvC341Info.sOutputTimingInfo.uiVTotal * 1.1  - 2, 5);

            dvC341_WriteToBuffer(B144_WPOSYCT2CH1, 0x0, 0);
            dvC341_WriteToBuffer(B144_WPOSYCT2CH1, 0x0, 5);
            dvC341_WriteToBuffer(B144_WPOSYCT3CH1, 0x80, 0);
            dvC341_WriteToBuffer(B144_WPOSYCT3CH1, 0x80, 5);

            switch(m_sDrvC341Info.sOutputTimingInfo.eLow_Latency)
            {
                case eLLM_LV1_FAST_WITH_FULL_WARPING:
                    dvC341_WriteToBuffer(B144_WPSYRDLYCH1, 0x02, 0);   //H2PF_Simon_0187
                    dvC341_WriteToBuffer(B144_WPSYRDLYCH1, 0x02, CH_WPBANK_OFFSET);
                    dvC341_WriteToBuffer(B144_WPFLDDLYCH1, (oaoivst + m_sDrvC341Info.sOutputTimingInfo.uiVActive * 2 / 3), 0);
                    dvC341_WriteToBuffer(B144_WPFLDDLYCH1, (oaoivst + m_sDrvC341Info.sOutputTimingInfo.uiVActive * 2 / 3), CH_WPBANK_OFFSET);
                    break;

                case eLLM_LV2_FAST_WITH_LIMITED_WARPING:
                default:
                    dvC341_WriteToBuffer(B144_WPSYRDLYCH1, SYRDLY_REG_FOR_LIMITED_WARP, 0);
                    dvC341_WriteToBuffer(B144_WPSYRDLYCH1, SYRDLY_REG_FOR_LIMITED_WARP, CH_WPBANK_OFFSET);
                    dvC341_WriteToBuffer(B144_WPFLDDLYCH1, FLDDLY_REG_FOR_LIMITED_WARP, 0);
                    dvC341_WriteToBuffer(B144_WPFLDDLYCH1, FLDDLY_REG_FOR_LIMITED_WARP, CH_WPBANK_OFFSET);
                    break;

                case eLLM_LV3_FAST_WITHOUT_WARPING:
                    break;

            }

        }
        else
        {
            dvC341_WriteToBuffer(B144_WPOSYCT2CH1, 0x0, 0);
            dvC341_WriteToBuffer(B144_WPOSYCT2CH1, 0x0, 5);
            dvC341_WriteToBuffer(B144_WPOSYCT3CH1, 0x0, 0);
            dvC341_WriteToBuffer(B144_WPOSYCT3CH1, 0x0, 5);
            dvC341_WriteToBuffer(B144_WPSYRDLYCH1, 0x0, 0); //H30K_Doulas_0018
            dvC341_WriteToBuffer(B144_WPSYRDLYCH1, 0x0, 5); //H30K_Doulas_0018
			dvC341_WriteToBuffer(B144_WPFLDDLYCH1, (oaoivst + m_sDrvC341Info.sOutputTimingInfo.uiVActive * 2 / 3), 0);
            dvC341_WriteToBuffer(B144_WPFLDDLYCH1, (oaoivst + m_sDrvC341Info.sOutputTimingInfo.uiVActive * 2 / 3), CH_WPBANK_OFFSET);

        }
    }

    #ifdef WARP_THROUGH_MODE
    dvC341_WriteToBuffer(B145_WPDTCTCH1, 0x2008, 0);  //through mode
    dvC341_WriteToBuffer(B145_WPDTCTCH1, 0x2008, CH_WPBANK_OFFSET);  //through mode
    #endif

    if(m_sDrvC341Info.sOutputTimingInfo.eLow_Latency == eLLM_LV3_FAST_WITHOUT_WARPING)   //H2PF_Simon_0150
    {
        dvc341Geo_BypassModeSet(TRUE);
    }

    dvC341_Buffer_Flush();

#if 0

#ifdef C341GEO_2D_NO_FRAME_LOCK   //A70LV_Doulas_0307 modify
	#ifdef Low_Latency_All
	if(!dvC341Geo_Low_Latency_Get())	//G100_Clare_0008
	#endif	/*Low_Latency_All*/
	{
    	dvC341Geo_Write(B2_OSYCT, 0x0311, 0);
	}
	#ifdef Low_Latency_All
	else
	{
		dvC341Geo_Write(B2_OSYCT, 0x0313, 0);  //enable force sync reset
	}
	#endif	/*Low_Latency_All*/
#else
    dvC341Geo_Write(B2_OSYCT, 0x0313, 0);//0x0311);//A70LV_Doulas_0045 modify Add VS forced sync reset //Select AOI0 as active output
#endif

    uiOHScycl =  m_sDrvC341Info.sOutputTimingInfo.uiHTotal;
    uiOVScycl =  m_sDrvC341Info.sOutputTimingInfo.uiVTotal;
//    uiOVScyclmin = uiOVScycl - 10;

    dvC341Geo_Write(B2_OHCYCL, (uiOHScycl - 2), 0);

    //forced sync reset on
	#ifdef Low_Latency_All
	if(dvC341Geo_Low_Latency_Get())	//G100_Clare_0008
	{
		dvC341Geo_Write(B2_SYRDLY, 0x08, 0);
	}
	else
	{
		dvC341Geo_Write(B2_SYRDLY, 0x02, 0);
	}
	#else
	dvC341Geo_Write(B2_SYRDLY, 0x02);
	#endif	/*Low_Latency_All*/
    dvC341Geo_Write(B2_FLDDLY, 0x01, 0);
#ifdef C341GEO_2D_NO_FRAME_LOCK   //A70LV_Doulas_0307 modify

	#ifdef Low_Latency_All
	if(!dvC341Geo_Low_Latency_Get())	//G100_Clare_0008
	#endif	/*Low_Latency_All*/
    {
    	dvC341Geo_Write(B2_OVCYCL, (uiOVScycl - 2), 0);   //forced sync reset off
	}
	#ifdef Low_Latency_All
	else
    {
    	dvC341Geo_Write(B2_OVCYCL, (uiOVScycl*1.1 - 2), 0);
	}
	#endif	/*Low_Latency_All*/
#else
    dvC341Geo_Write(B2_OVCYCL, (uiOVScycl*1.1 - 2), 0);
#endif


    dvC341Geo_Write(B2_OSYCT3, 0x80, 0);//Trigger forced sync reset in unit of HS.

    dvC341Geo_Write(B2_OHSCT, m_sDrvC341Info.sOutputTimingInfo.ucHSyncWidth/2 -1, 0);
    dvC341Geo_Write(B2_OVSCT, m_sDrvC341Info.sOutputTimingInfo.ucVSyncWidth-1, 0);

    #if 1//0//A70LV_Doulas_0098     //A70LV_Doulas_0079
    //dvC341Geo_Write(B2_SYRDLY, 0x02);    //A70LV_Doulas_0083 modify
    //dvC341Geo_Write(B2_FLDDLY, 0x01);
    //dvC341Geo_Write(B2_OVCYCL, (uiOVScycl*1.1 - 2));
    #else
    //forced sync reset off
    dvC341Geo_Write(B2_SYRDLY, 0x00);
    //dvC341Geo_Write(B2_OVCYCLMIN, (uiOVScyclmin - 1), 0);
    dvC341Geo_Write(B2_FLDDLY, 0x20);
    #endif

    dvC341Geo_Write(B2_OAOI0HST, m_sDrvC341Info.sOutputTimingInfo.uiHStart-7, 0);  //A70LV_Doulas_0045 modify
    dvC341Geo_Write(B2_OAOI0HEND, (m_sDrvC341Info.sOutputTimingInfo.uiHStart -7+    //A70LV_Doulas_0045 modify
                                   m_sDrvC341Info.sOutputTimingInfo.uiHActive), 0);
    dvC341Geo_Write(B2_OAOI0VST, m_sDrvC341Info.sOutputTimingInfo.uiVStart, 0);
    dvC341Geo_Write(B2_OAOI0VEND, (m_sDrvC341Info.sOutputTimingInfo.uiVStart +
                                   m_sDrvC341Info.sOutputTimingInfo.uiVActive), 0);

    dvC341Geo_Write(B2_OACTHST, m_sDrvC341Info.sOutputTimingInfo.uiHStart-7, 0);   //A70LV_Doulas_0045 modify
    dvC341Geo_Write(B2_OACTHW, m_sDrvC341Info.sOutputTimingInfo.uiHActive, 0);
    dvC341Geo_Write(B2_OACTVST, m_sDrvC341Info.sOutputTimingInfo.uiVStart+1, 0);
    dvC341Geo_Write(B2_OACTVW, m_sDrvC341Info.sOutputTimingInfo.uiVActive, 0);
#endif
}

static void dvC341Geo_ConfigClock(const UINT8 ucPoRefDiv,const UINT8 ucPoFbDiv)
{
    LOG_MSG(db_DV_GEO , "(func:%s, line:%d)\n", __FUNCTION__, __LINE__);
}

#if 0
void dvC341Geo_ConfigMemAD(void)
{
    LOG_MSG(db_DV_GEO , "(func:%s, line:%d)\n", __FUNCTION__, __LINE__);

    //UINT32 Wpisfld[4];
    //Wpisfld[0] = PS_WPOSFLD[0] << 15;
    //Wpisfld[1] = PS_WPOSFLD[1] << 15;
    //Wpisfld[2] = PS_WPOSFLD[2] << 15;
    //Wpisfld[3] = PS_WPOSFLD[3] << 15;

    dvC341Geo_ConfigOutput();

    #if 0

	#ifdef Low_Latency_All
	if(m_sDrvC341Info.sOutputTimingInfo.eLow_Latency)
	{
		dvC341Geo_Write(B9_OSFLD0, DEF_OSFLD0, 0);
		dvC341Geo_Write(B9_OSFLD1, DEF_OSFLD1, 0);
		dvC341Geo_Write(B9_OSFLD2, DEF_OSFLD2, 0);
		dvC341Geo_Write(B9_OSFLD3, DEF_OSFLD3, 0);
	}
	else
	{
		dvC341Geo_Write(B9_OSFLD0, DEF_OSFLD3, 0);
		dvC341Geo_Write(B9_OSFLD1, DEF_OSFLD0, 0);
		dvC341Geo_Write(B9_OSFLD2, DEF_OSFLD1, 0);
		dvC341Geo_Write(B9_OSFLD3, DEF_OSFLD2, 0);
	}
	#else
	dvC341Geo_Write(B9_OSFLD0, DEF_OSFLD3, 0);
    dvC341Geo_Write(B9_OSFLD1, DEF_OSFLD0, 0);
    dvC341Geo_Write(B9_OSFLD2, DEF_OSFLD1, 0);
    dvC341Geo_Write(B9_OSFLD3, DEF_OSFLD2, 0);
	#endif	/*Low_Latency_All*/
    dvC341Geo_Write(B9_ISFLD0, DEF_ISFLD0, 0);
    dvC341Geo_Write(B9_ISFLD1, DEF_ISFLD1, 0);
    dvC341Geo_Write(B9_ISFLD2, DEF_ISFLD2, 0);
    dvC341Geo_Write(B9_ISFLD3, DEF_ISFLD3, 0);

    dvC341Geo_Write(B9_IMWI, 0x00, 0);

    #endif

}
#endif

static void dvC341Geo_ConfigBasicMemory(void)
{
    LOG_MSG(db_DV_GEO , "(func:%s, line:%d)\n", __FUNCTION__, __LINE__);
}

static void dvC341GeoCtrl_Init_Default_Register(void)
{
    UINT16 uiData = 0;
    UINT8  ucData = 0;

    LOG_MSG(db_DV_GEO , "(func:%s, line:%d)\n", __FUNCTION__, __LINE__);

    //for OSD 閃動問題  //H2PF_Simon_0098     //H2PF_Simon_0107 remove
    #if 0
    dvC341Geo_WriteToBuffer(B144_WOR1ACCTCH1,	0x80, eC341_CH1);
    dvC341Geo_WriteToBuffer(B144_WOR6ACCTCH1,	0x3F, eC341_CH1);
    dvC341Geo_WriteToBuffer(B144_WOR1ACCTCH1,	0x80, eC341_CH2);
    dvC341Geo_WriteToBuffer(B144_WOR6ACCTCH1,	0x3F, eC341_CH2);
    #endif

    #if 0   //C341 wait review first

    dvC341Geo_WriteToBuffer(B0_CPUDT,	0x40, 0);
    dvC341Geo_WriteToBuffer(B0_CPUDTCTL,	0x60, 0);

    dvC341Geo_WriteToBuffer(B1_PGCTRL,	0x00, 0);

    dvC341Geo_WriteToBuffer(B1_SFLDTCTL,	0xa0, 0);
    dvC341Geo_WriteToBuffer(B1_SFLSTAT,	0x22, 0);

    dvC341Geo_WriteToBuffer(B1_GIOI,	0x32, 0);
    dvC341Geo_WriteToBuffer(B1_GIOS,	0x0F, 0); //Use Serial Flash mode //A70LV_Larry_0139

    dvC341Geo_WriteToBuffer(B2_OVPHRDT,	0x0a7fff, 0);
    dvC341Geo_Buffer_Flush();
    //if((m_sDrvC341Info.ePanelTimingId == ePANEL_ID_1080P_120HZ) ||
       //(m_sDrvC341Info.ePanelTimingId == ePANEL_ID_WUXGA_120HZ))     //A70LV_Doulas_0098
    //{

    uiData = (UINT16)dvC341Geo_Read(B5_DTCT, 0);   //A70LV_Larry_0177
    dvC341Geo_Write(B5_DTCT,	uiData | 0x0C00, 0); //A70LV_Larry_0177

    ucData = (UINT8)dvC341Geo_Read(B5_WPACCT, 0); //A70LV_Larry_0177
    dvC341Geo_Write(B5_WPACCT,	ucData | 0x30, 0); //A70LV_Larry_0177

    //}
    //else
    //{
        //dvC341Geo_Write(B5_DTCT,	0x0008);
        //dvC341Geo_Write(B5_WPACCT,	0x00);
    //}

#if 1
    dvC341Geo_WriteToBuffer(B6_HLUT0,	0x02, 0);
    dvC341Geo_WriteToBuffer(B6_HLUT1,	0x04, 0);
    dvC341Geo_WriteToBuffer(B6_HLUT2,	0x06, 0);
    dvC341Geo_WriteToBuffer(B6_HLUT3,	0x07, 0);
    dvC341Geo_WriteToBuffer(B6_HLUT4,	0x07, 0);
    dvC341Geo_WriteToBuffer(B6_HLUT5,	0x05, 0);
    dvC341Geo_WriteToBuffer(B6_HLUT6,	0x03, 0);
    dvC341Geo_WriteToBuffer(B6_HLUT7,	0x00, 0);
    dvC341Geo_WriteToBuffer(B6_HLUT8,	0xfd, 0);
    dvC341Geo_WriteToBuffer(B6_HLUT9,	0xf9, 0);
    dvC341Geo_WriteToBuffer(B6_HLUT10,	0xf6, 0);
    dvC341Geo_WriteToBuffer(B6_HLUT11,	0xf4, 0);
    dvC341Geo_WriteToBuffer(B6_HLUT12,	0xf4, 0);
    dvC341Geo_WriteToBuffer(B6_HLUT13,	0xf6, 0);
    dvC341Geo_WriteToBuffer(B6_HLUT14,	0xfa, 0);
    dvC341Geo_WriteToBuffer(B6_HLUT15,	0x00, 0);
    dvC341Geo_WriteToBuffer(B6_HLUT16,	0x08, 0);
    dvC341Geo_WriteToBuffer(B6_HLUT17,	0x11, 0);
    dvC341Geo_WriteToBuffer(B6_HLUT18,	0x1b, 0);
    dvC341Geo_WriteToBuffer(B6_HLUT19,	0x25, 0);
    dvC341Geo_WriteToBuffer(B6_HLUT20,	0x2e, 0);
    dvC341Geo_WriteToBuffer(B6_HLUT21,	0x37, 0);
    dvC341Geo_WriteToBuffer(B6_HLUT22,	0x3c, 0);
    dvC341Geo_WriteToBuffer(B6_HLUT23,	0x40, 0);
    dvC341Geo_WriteToBuffer(B6_VLUT0,	0x02, 0);
    dvC341Geo_WriteToBuffer(B6_VLUT1,	0x04, 0);
    dvC341Geo_WriteToBuffer(B6_VLUT2,	0x06, 0);
    dvC341Geo_WriteToBuffer(B6_VLUT3,	0x07, 0);
    dvC341Geo_WriteToBuffer(B6_VLUT4,	0x07, 0);
    dvC341Geo_WriteToBuffer(B6_VLUT5,	0x05, 0);
    dvC341Geo_WriteToBuffer(B6_VLUT6,	0x03, 0);
    dvC341Geo_WriteToBuffer(B6_VLUT7,	0x00, 0);
    dvC341Geo_WriteToBuffer(B6_VLUT8,	0xfd, 0);
    dvC341Geo_WriteToBuffer(B6_VLUT9,	0xf9, 0);
    dvC341Geo_WriteToBuffer(B6_VLUT10,	0xf6, 0);
    dvC341Geo_WriteToBuffer(B6_VLUT11,	0xf4, 0);
    dvC341Geo_WriteToBuffer(B6_VLUT12,	0xf4, 0);
    dvC341Geo_WriteToBuffer(B6_VLUT13,	0xf6, 0);
    dvC341Geo_WriteToBuffer(B6_VLUT14,	0xfa, 0);
    dvC341Geo_WriteToBuffer(B6_VLUT15,	0x00, 0);
    dvC341Geo_WriteToBuffer(B6_VLUT16,	0x08, 0);
    dvC341Geo_WriteToBuffer(B6_VLUT17,	0x11, 0);
    dvC341Geo_WriteToBuffer(B6_VLUT18,	0x1b, 0);
    dvC341Geo_WriteToBuffer(B6_VLUT19,	0x25, 0);
    dvC341Geo_WriteToBuffer(B6_VLUT20,	0x2e, 0);
    dvC341Geo_WriteToBuffer(B6_VLUT21,	0x37, 0);
    dvC341Geo_WriteToBuffer(B6_VLUT22,	0x3c, 0);
    dvC341Geo_WriteToBuffer(B6_VLUT23,	0x40, 0);
#endif /* 0 */

    dvC341Geo_WriteToBuffer(B7_EGBCT, 0x0080, 0); //A70LV_Larry_0318

/*
    dvC341Geo_WriteToBuffer(B9_OSFLD1,	0x0980, 0);
    dvC341Geo_WriteToBuffer(B9_OSFLD2,	0x1300, 0);
    dvC341Geo_WriteToBuffer(B9_OSFLD3,	0x1c80, 0);
    dvC341Geo_WriteToBuffer(B9_ISFLD0,	0x00000000, 0);
    dvC341Geo_WriteToBuffer(B9_ISFLD1,	0x01300000, 0);
    dvC341Geo_WriteToBuffer(B9_ISFLD2,	0x02600000, 0);
    dvC341Geo_WriteToBuffer(B9_ISFLD3,	0x03900000, 0);
*/
    dvC341Geo_WriteToBuffer(B9_IPOSFLD0,	0x04c00b00, 0);
    dvC341Geo_WriteToBuffer(B9_IPOSFLD1,	0x04c00000, 0);
    dvC341Geo_WriteToBuffer(B9_IPOSFLD2,	0x04eee100, 0);
    dvC341Geo_WriteToBuffer(B9_IPOSFLD3,	0x04eed600, 0);
    dvC341Geo_WriteToBuffer(B9_IPISFLD0,	0x04c00b00, 0);
    dvC341Geo_WriteToBuffer(B9_IPISFLD1,	0x04c00000, 0);
    dvC341Geo_WriteToBuffer(B9_IPISFLD2,	0x04eee100, 0);
    dvC341Geo_WriteToBuffer(B9_IPISFLD3,	0x04eed600, 0);
    dvC341Geo_WriteToBuffer(B9_IPMWI,	0x8b, 0);
    dvC341Geo_WriteToBuffer(B9_IPYOFF,	0x02ee, 0);
    dvC341Geo_WriteToBuffer(B9_MVFLD,	0x057b8000, 0);
    dvC341Geo_Buffer_Flush();

    #endif
}

static void dvC341GeoCtrl_Change_Panel_Init(void) //A70LV_Larry_0177
{
    //UINT16 uiData = 0;
    UINT8  ucData = 0;
#if 0
    dvC341Geo_Write(B0_CPUDT,	0x40, 0);
    dvC341Geo_Write(B0_CPUDTCTL,	0x60, 0);

    dvC341Geo_Write(B1_PGCTRL,	0x00, 0);

    dvC341Geo_Write(B1_SFLDTCTL,	0xa0, 0);
    dvC341Geo_Write(B1_SFLSTAT,	0x22, 0);

    dvC341Geo_Write(B1_GIOI,	0x32, 0);
    dvC341Geo_Write(B1_GIOS,	0x0F, 0); //Use Serial Flash mode //A70LV_Larry_0139

    dvC341Geo_Write(B2_OVPHRDT,	0x0a7fff, 0);
    //if((m_sDrvC341Info.ePanelTimingId == ePANEL_ID_1080P_120HZ) ||
       //(m_sDrvC341Info.ePanelTimingId == ePANEL_ID_WUXGA_120HZ))     //A70LV_Doulas_0098
    //{

    //uiData = (UINT16)dvC341Geo_Read(B5_DTCT, 0);
    //dvC341Geo_Write(B5_DTCT,	uiData | 0x0C00, 0);

    ucData = (UINT8)dvC341Geo_Read(B5_WPACCT, 0);
    dvC341Geo_Write(B5_WPACCT,	ucData | 0x30, 0);

    //}
    //else
    //{
        //dvC341Geo_Write(B5_DTCT,	0x0008), 0;
        //dvC341Geo_Write(B5_WPACCT,	0x00, 0);
    //}

#if 0
    dvC341Geo_Write(B6_HLUT0,	0x02);
    dvC341Geo_Write(B6_HLUT1,	0x04);
    dvC341Geo_Write(B6_HLUT2,	0x06);
    dvC341Geo_Write(B6_HLUT3,	0x07);
    dvC341Geo_Write(B6_HLUT4,	0x07);
    dvC341Geo_Write(B6_HLUT5,	0x05);
    dvC341Geo_Write(B6_HLUT6,	0x03);
    dvC341Geo_Write(B6_HLUT7,	0x00);
    dvC341Geo_Write(B6_HLUT8,	0xfd);
    dvC341Geo_Write(B6_HLUT9,	0xf9);
    dvC341Geo_Write(B6_HLUT10,	0xf6);
    dvC341Geo_Write(B6_HLUT11,	0xf4);
    dvC341Geo_Write(B6_HLUT12,	0xf4);
    dvC341Geo_Write(B6_HLUT13,	0xf6);
    dvC341Geo_Write(B6_HLUT14,	0xfa);
    dvC341Geo_Write(B6_HLUT15,	0x00);
    dvC341Geo_Write(B6_HLUT16,	0x08);
    dvC341Geo_Write(B6_HLUT17,	0x11);
    dvC341Geo_Write(B6_HLUT18,	0x1b);
    dvC341Geo_Write(B6_HLUT19,	0x25);
    dvC341Geo_Write(B6_HLUT20,	0x2e);
    dvC341Geo_Write(B6_HLUT21,	0x37);
    dvC341Geo_Write(B6_HLUT22,	0x3c);
    dvC341Geo_Write(B6_HLUT23,	0x40);
    dvC341Geo_Write(B6_VLUT0,	0x02);
    dvC341Geo_Write(B6_VLUT1,	0x04);
    dvC341Geo_Write(B6_VLUT2,	0x06);
    dvC341Geo_Write(B6_VLUT3,	0x07);
    dvC341Geo_Write(B6_VLUT4,	0x07);
    dvC341Geo_Write(B6_VLUT5,	0x05);
    dvC341Geo_Write(B6_VLUT6,	0x03);
    dvC341Geo_Write(B6_VLUT7,	0x00);
    dvC341Geo_Write(B6_VLUT8,	0xfd);
    dvC341Geo_Write(B6_VLUT9,	0xf9);
    dvC341Geo_Write(B6_VLUT10,	0xf6);
    dvC341Geo_Write(B6_VLUT11,	0xf4);
    dvC341Geo_Write(B6_VLUT12,	0xf4);
    dvC341Geo_Write(B6_VLUT13,	0xf6);
    dvC341Geo_Write(B6_VLUT14,	0xfa);
    dvC341Geo_Write(B6_VLUT15,	0x00);
    dvC341Geo_Write(B6_VLUT16,	0x08);
    dvC341Geo_Write(B6_VLUT17,	0x11);
    dvC341Geo_Write(B6_VLUT18,	0x1b);
    dvC341Geo_Write(B6_VLUT19,	0x25);
    dvC341Geo_Write(B6_VLUT20,	0x2e);
    dvC341Geo_Write(B6_VLUT21,	0x37);
    dvC341Geo_Write(B6_VLUT22,	0x3c);
    dvC341Geo_Write(B6_VLUT23,	0x40);
#endif /* 0 */

/*
    dvC341Geo_Write(B9_OSFLD1,	0x0980, 0);
    dvC341Geo_Write(B9_OSFLD2,	0x1300, 0);
    dvC341Geo_Write(B9_OSFLD3,	0x1c80, 0);
    dvC341Geo_Write(B9_ISFLD0,	0x00000000, 0);
    dvC341Geo_Write(B9_ISFLD1,	0x01300000, 0);
    dvC341Geo_Write(B9_ISFLD2,	0x02600000, 0);
    dvC341Geo_Write(B9_ISFLD3,	0x03900000, 0);
*/
    dvC341Geo_Write(B9_IPOSFLD0,	0x04c00b00, 0);
    dvC341Geo_Write(B9_IPOSFLD1,	0x04c00000, 0);
    dvC341Geo_Write(B9_IPOSFLD2,	0x04eee100, 0);
    dvC341Geo_Write(B9_IPOSFLD3,	0x04eed600, 0);
    dvC341Geo_Write(B9_IPISFLD0,	0x04c00b00, 0);
    dvC341Geo_Write(B9_IPISFLD1,	0x04c00000, 0);
    dvC341Geo_Write(B9_IPISFLD2,	0x04eee100, 0);
    dvC341Geo_Write(B9_IPISFLD3,	0x04eed600, 0);
    dvC341Geo_Write(B9_IPMWI,	0x8b, 0);
    dvC341Geo_Write(B9_IPYOFF,	0x02ee, 0);
    dvC341Geo_Write(B9_MVFLD,	0x057b8000, 0);

#if 0
//; C341B10 registers
    dvC341Geo_Write(B10_IFH,	0x0897);
    dvC341Geo_Write(B10_IFV,	0x0465);
    dvC341Geo_Write(B10_IHW,	0x2c);
    dvC341Geo_Write(B10_IVW,	0x05);
    dvC341Geo_Write(B10_IMFH,	0x0b92);
    dvC341Geo_Write(B10_IMFV,	0x32dcb8);
//C341B10	IHVPHDF0	0000h
//C341B10	IHVPHDF1	0000h
    dvC341Geo_Write(B10_FLDHIST,	0xff);
    dvC341Geo_Write(B10_PSTAT,	0x02);
//C341B10	PVAL0	00h
//C341B10	PVAL1	00h
    dvC341Geo_Write(B10_PH0,	0x3fff);
    dvC341Geo_Write(B10_PV0,	0x3fff);
//C341B10	PH1	0000h
//C341B10	PV1	0000h
//C341B10	IPIXR	0000h
//C341B10	IPIXG	0000h
//C341B10	IPIXB	0000h
    dvC341Geo_Write(B10_IPTCNT,	0x166f65);
    dvC341Geo_Write(B10_IAPL0,	0x06645d8d);
    dvC341Geo_Write(B10_IMAXFILTR,	0xfe);
//C341B10	IMINFILTR	00h
    dvC341Geo_Write(B10_STATE,	0x29);
//C341B10	IPFLDMON	00h
    dvC341Geo_Write(B10_IFACTHST,	0x00b9);
    dvC341Geo_Write(B10_IFACTHW,	0x0780);
    dvC341Geo_Write(B10_IFACTVST,	0x002a);
    dvC341Geo_Write(B10_IFACTVW,	0x0438);
//    dvC341Geo_Write(B10_OHISTGRAM,	0x1107085c);
    dvC341Geo_Write(B10_OHISTGRAM_0,	0x5c);
    dvC341Geo_Write(B10_OHISTGRAM_1,	0x08);
    dvC341Geo_Write(B10_OHISTGRAM_2,	0x07);
    dvC341Geo_Write(B10_OHISTGRAM_3,	0x11);
//C341B10	OHISTGRAM_4	00h
//C341B10	OHISTGRAM_5	00h
//C341B10	OHISTGRAM_6	00h
//C341B10	OHISTGRAM_7	00h
    dvC341Geo_Write(B10_OAPL0,	0x06645d8d);
    dvC341Geo_Write(B10_MAXFILTR,	0xfe);
//C341B10	MINFILTR	00h
    dvC341Geo_Write(B10_OPTCNT,	0x166f60);
    dvC341Geo_Write(B10_OFV,	0x0464);
//C341B10	IPOFV	0000h
    dvC341Geo_Write(B10_OLNUM,	0x0101);
    dvC341Geo_Write(B10_ILNUM,	0x0171);
//C341B10	LVDSRXMON	00h
    dvC341Geo_Write(B10_DEVICECODE,	0x20);
#endif

#endif
}

eC341GEO_EXEC_CODE dvC341Geo_Init(const ePANEL_ID ePanelId, eLOW_LATENCY_MODE bLow_Latency)   //H2PF_Simon_0187
{
    UINT8 ucTblIndex = 0;
    eC341GEO_EXEC_CODE eExecCode = eC341GEO_EXEC_CODE_PASS;

    if((eExecCode = dvC341Geo_GetPanelIndex(ePanelId, &ucTblIndex)) != eC341GEO_EXEC_CODE_PASS)
    {
        LOG_MSG(db_DV_GEO , "(func:%s, line:%d) C341 Get Panel Timing Index Fail = %d\n", __FUNCTION__, __LINE__, eExecCode);
        return eExecCode;
    }

    LOG_MSG(db_DV_GEO , "(func:%s, line:%d) Get Panel Index %d\n", __FUNCTION__, __LINE__, ucTblIndex);


#if 1
	#ifdef Low_Latency_All
	dvC341Geo_Low_Latency_Set(bLow_Latency);
	#endif	/*Low_Latency_All*/
#endif

#ifdef C341GEO_2D_NO_FRAME_LOCK   //A70LV_Doulas_0307
	#ifdef Low_Latency_All	//G100_Clare_0009
	if(!dvC341Geo_Low_Latency_Get())	//G100_Clare_0008
	#endif	/*Low_Latency_All*/
	{
	    m_ucForcedSyncReset = FALSE;
	    m_uiH_Total = m_sDrvC341Info.sOutputTimingInfo.uiHTotal;
	    m_uiV_Total = m_sDrvC341Info.sOutputTimingInfo.uiVTotal;
	}
#endif

    m_sDrvC341Info.bInit = TRUE;
    m_sDrvC341Info.ePanelTimingId = ePanelId;
    m_sDrvC341Info.ulMclko_Freq = 800000000; //800MHz
    m_sDrvC341Info.ucIsDefaultWarpTable = TRUE;   //H2PF_Simon_0131
    //m_sDrvC341Info.ucRegISYCT = 0x31;    //A35G2_CDS_Simon_0003

    dvC341Geo_GetOutputTiming(ucTblIndex);
    dvC341GeoCtrl_Init_Default_Register();
    dvC341Geo_ConfigOutput();

    PS_WP_HW = 3840 ;
    PS_WP_VW = 2400 ;
    PM_BOARD = 0;
    PM_EBIAS_AREA_TEST = 0 ;
	PM_EBIAS_GAMMA[0] = 2.2;
	PM_EBIAS_GAMMA[1] = 2.2;
	PM_EBIAS_GAMMA[2] = 2.2;

    dvC341_WriteToBuffer( B148_WPEBIASSADCH1, DEF_WPEBIASSAD0 , 0);

    //Specify the memory read linefeed width for edge blend bias data in units of EBIASMWI [7:0]  1024 bytes.
    //EBIASMWI = Result of rounding off of the No. of horizontal effective pixels/2048 at certain decimal place.
    dvC341_WriteToBuffer( B148_WPEBIASMWICH1, DEF_EBIASMWI&0xff , 0);

	//address of dot by dot blending area
	dvC341_WriteToBuffer( B148_WPEGBDBDSADCH1, OSD_DBD_ADDR, 0);
	dvC341_WriteToBuffer( B148_WPEGBDBDMWICH1, OSD_DBD_8BIT_MWI, 0);

    //CH2
    dvC341_WriteToBuffer( B148_WPEBIASSADCH1, DEF_WPEBIASSAD1 , CH_WPBANK_OFFSET);
    dvC341_WriteToBuffer( B148_WPEBIASMWICH1, DEF_EBIASMWI&0xff , CH_WPBANK_OFFSET);
	dvC341_WriteToBuffer( B148_WPEGBDBDSADCH1, OSD_DBD_ADDR, CH_WPBANK_OFFSET);
	dvC341_WriteToBuffer( B148_WPEGBDBDMWICH1, OSD_DBD_8BIT_MWI, CH_WPBANK_OFFSET);

    //OSD Init
    dvC341Geo_EnableMagnifyOSD2X(TRUE);
    dvC341_WriteToBuffer( B4_BBWMWI, DEF_OSDMWI&0xff, 0);

    dvC341_WriteToBuffer( B9_OSDSAD1CH1, DEF_OSDSAD2, 0);   //H2PF_Simon_0107
    dvC341_WriteToBuffer( B9_OSDMWI1CH1, DEF_OSDMWI&0xff, 0);
    dvC341_WriteToBuffer( B9_OSDACTHW1CH1, PS_WP_HW/OUTPUT_CH_NUM, 0);
    dvC341_WriteToBuffer( B9_OSDACTVW1CH1, PS_WP_VW, 0);
    dvC341_WriteToBuffer( B9_OSDMODE1CH1, (BIT14|BIT12|BIT11|BIT2), 0);

    dvC341_WriteToBuffer( B9_OSDSAD1CH1, DEF_OSDSAD2, CH_BANK_OFFSET);   //H2PF_Simon_0107
    dvC341_WriteToBuffer( B9_OSDMWI1CH1, DEF_OSDMWI&0xff, CH_BANK_OFFSET);
    dvC341_WriteToBuffer( B9_OSDACTHW1CH1, PS_WP_HW/OUTPUT_CH_NUM, CH_BANK_OFFSET);
    dvC341_WriteToBuffer( B9_OSDACTVW1CH1, PS_WP_VW, CH_BANK_OFFSET);
    dvC341_WriteToBuffer( B9_OSDMODE1CH1, (BIT14|BIT12|BIT11|BIT2), CH_BANK_OFFSET);

    dvC341Geo_OSDACTStartConfig(1);

    return eExecCode;        //A70LV_Doulas_0072
}

void dvC341Geo_InputSwapSet(UINT8 ucVal)
{
//   dvC341Geo_Write(B3_ISWP,     ucVal, 0);
}

void dvC341Geo_TestPatternSet(UINT8 ucVal)
{
#if 0
    switch(ucVal)
    {
        default:
        case 0:
            dvC341Geo_Write(B1_PGCTRL,	0x00, 0);
            break;

        case 1:     //Horizon Stripe
            dvC341Geo_Write(B1_PGCTRL,	0x0e, 0);

            dvC341Geo_Write(B1_HLIMITMAX,	0x0001, 0);

            dvC341Geo_Write(B1_PGCOLR,	0xFF, 0);
            dvC341Geo_Write(B1_PGCOLG,	0xFF, 0);
            dvC341Geo_Write(B1_PGCOLB,	0xFF, 0);
            dvC341Geo_Write(B1_PGCOLLSR,	0x03, 0);
            dvC341Geo_Write(B1_PGCOLLSG,	0x03, 0);
            dvC341Geo_Write(B1_PGCOLLSB,	0x03, 0);
            break;

        case 2:     //Vertical Stripe
            dvC341Geo_Write(B1_PGCTRL,	0x0f, 0);

            dvC341Geo_Write(B1_VLIMITMAX,	0x0001, 0);

            dvC341Geo_Write(B1_PGCOLR,	0xFF, 0);
            dvC341Geo_Write(B1_PGCOLG,	0xFF, 0);
            dvC341Geo_Write(B1_PGCOLB,	0xFF, 0);
            dvC341Geo_Write(B1_PGCOLLSR,	0x03, 0);
            dvC341Geo_Write(B1_PGCOLLSG,	0x03, 0);
            dvC341Geo_Write(B1_PGCOLLSB,	0x03, 0);
            break;

        case 3: //Horizon Color bar
            dvC341Geo_Write(B1_PGCTRL,	0x01, 0);
            dvC341Geo_Write(B1_HLIMITMAX,(UINT32)(m_sDrvC341Info.sOutputTimingInfo.uiHActive/8), 0);
            break;

        case 4: //Vertical Color bar
            dvC341Geo_Write(B1_PGCTRL,	0x02, 0);
            dvC341Geo_Write(B1_VLIMITMAX,(UINT32)(m_sDrvC341Info.sOutputTimingInfo.uiVActive/8), 0);
            break;

        case 5: //Horizon RAMP
            dvC341Geo_Write(B1_PGCTRL,	0x03, 0);
            dvC341Geo_Write(B1_PGPTCTRL,0x00, 0);
            dvC341Geo_Write(B1_HLIMITMAX,0x3ff, 0);  //A70LV_Doulas_0040 modify
            dvC341Geo_Write(B1_HRAMPCNT,0x01, 0);
            break;

        case 6: //Checker
            dvC341Geo_Write(B1_PGCTRL,	0x05, 0);
            dvC341Geo_Write(B1_HLIMITMAX,(UINT32)(m_sDrvC341Info.sOutputTimingInfo.uiHActive/4), 0);
            dvC341Geo_Write(B1_VLIMITMAX,(UINT32)(m_sDrvC341Info.sOutputTimingInfo.uiVActive/4), 0);

            dvC341Geo_Write(B1_PGCOLR,0xff, 0);
            dvC341Geo_Write(B1_PGCOLG,0xff, 0);
            dvC341Geo_Write(B1_PGCOLB,0xff, 0);

            dvC341Geo_Write(B1_PGCOLLSR,0x03, 0);
            dvC341Geo_Write(B1_PGCOLLSG,0x03, 0);
            dvC341Geo_Write(B1_PGCOLLSB,0x03, 0);
            break;
    }
#endif
}

#if 0
void dvC341Geo_TestMode(void)
{
    UINT32 ulIFH,ulIFV;
    UINT32 ulIMFH,ulIMFV;
    UINT32 ulPH0,ulPV0;
    UINT32 ulVal;
    UINT32 ulHW,ulVW;

    ulIFH = dvC341Geo_Read(B10_IFH, 0);
    ulIFV = dvC341Geo_Read(B10_IFV, 0);
    ulIMFH = dvC341Geo_Read(B10_IMFH, 0);
    ulIMFV = dvC341Geo_Read(B10_IMFV, 0);


   // dvC341Geo_Write(B3_PTH,0x04);    //A70LV_Doulas_0042
    dvC341Geo_Write(B3_PMODE,0x81);//0x81);
  //  dvC341Geo_Write(B3_PMODE,0xA6);
    ulVal = 0xA1;
 //   MS_SLEEP(120);
    //while(ulVal!=0x81)
    {
        ulVal = (INT32)dvC341Geo_Read(B3_PMODE, 0);   //A70LV_Doulas_0072 modify
    }


    ulPH0 = dvC341Geo_Read(B10_IFACTHST, 0);
    ulPV0 = dvC341Geo_Read(B10_IFACTVST, 0);

    ulHW = dvC341Geo_Read(B10_IFACTHW, 0);
    ulVW = dvC341Geo_Read(B10_IFACTVW, 0);

    //LOG_MSG(db_DV_GEO , "(func:%s, line:%d) C341 Get (%d,%d)(0x%x,0x%x)P(0x%x,0x%x)(0x%x)(0x%x)(0x%x,0x%x)\n", __FUNCTION__, __LINE__, ulIFH,ulIFV,ulIMFH,ulIMFV
            //,ulPH0,ulPV0,dvC341Geo_Read(B2_OAOI0VST, 0),ulVal,ulHW,ulVW);     //A70LV_Doulas_0157 debug
}
#endif

void dvC341Geo_V_Start_Checking(void)  //A70LV_Doulas_0042
{
#if 0
    UINT32 ulVal = 0;
    UINT32 ulVal2 = 0;
    UINT32 ulVal3 = 0;
    UINT8  ucCount = 0;
    UINT8 ucRTCT = 0;

//    dvC341Geo_Write(B3_PMODE,0x81);
    for(;ucCount < 3; ucCount++)
    {
        ulVal = dvC341Geo_Read(B10_IFACTVST, 0);
        ulVal2 = dvC341Geo_Read(B10_IFACTVST, 0);
        ulVal3 = dvC341Geo_Read(B10_IFACTVST, 0);
        if((ulVal == ulVal2) && (ulVal == ulVal3) && (ulVal > 3) &&
		   ((ulVal + m_sDrvC341Info.sOutputTimingInfo.uiVActive) < m_sDrvC341Info.sOutputTimingInfo.uiVTotal))	//G100_Doulas_0084 Modify
        {
			ucCount = 4;
        }
        else
        {
            LOG_MSG(db_DV_GEO, "dvC341Geo_V_Start_Checking Vstart Get (0x%x)(0x%x)(0x%x)\n",ulVal,ulVal2,ulVal3);
			ulVal = m_sDrvC341Info.sOutputTimingInfo.uiVStart;		//G100_Doulas_0084 Add
        }
		MS_SLEEP(1);	//G100_Doulas_0084 Add
    }
    if(ulVal != ulV_START)
    {
        ucRTCT = dvC341Geo_Read(BN_RTCT, 0);      //A70LV_Doulas_0120
        dvC341Geo_WriteToBuffer(BN_RTCT, 0, 0);           //A70LV_Doulas_0120
        dvC341Geo_WriteToBuffer(B3_IACTVST,    ulVal, 0);
        dvC341Geo_WriteToBuffer(B2_OAOI0VST, ulVal, 0);
        dvC341Geo_WriteToBuffer(B2_OAOI0VEND, (ulVal + m_sDrvC341Info.sOutputTimingInfo.uiVActive), 0);
        dvC341Geo_WriteToBuffer(B2_OACTVST, ulVal+1, 0);
        dvC341Geo_WriteToBuffer(BN_RTCT, ucRTCT, 0);      //A70LV_Doulas_0120
        dvC341Geo_Buffer_Flush();
        ulV_START = ulVal;
    }

    LOG_MSG(db_DV_GEO, "(func:%s, line:%d) C341 Vstart Get (0x%x)\n", __FUNCTION__, __LINE__, ulVal);
#endif
}

void dvC341Geo_ForcedSyncResetValueSet(UINT8 ucForcedSyncReset,UINT16 uiH_Total,UINT16 uiV_Total)     //A70LV_Doulas_0079
{
//#if 0   //H30K_Doulas_0016//C341 wait review first

#ifdef C341GEO_2D_NO_FRAME_LOCK   //A70LV_Doulas_0307 modify

    UINT32 oaoivst = m_sDrvC341Info.sOutputTimingInfo.uiVStart + 1;

    if(m_sDrvC341Info.sOutputTimingInfo.eLow_Latency == eLLM_LV3_FAST_WITHOUT_WARPING)   //H2PF_Simon_0150
    {
        dvc341Geo_BypassModeSet(TRUE);
        return;
    }

    if(m_ucForcedSyncReset == ucForcedSyncReset)
    {
        m_ucOutputChanged = FALSE;
    }
    else
    {
        m_ucOutputChanged = TRUE;
    }


    dvC341_WriteToBuffer(B144_WPSYRDLYCH1, 0x02, 0);   //H2PF_Simon_0187
    dvC341_WriteToBuffer(B144_WPSYRDLYCH1, 0x02, CH_WPBANK_OFFSET);
    dvC341_WriteToBuffer(B144_WPFLDDLYCH1, (oaoivst + m_sDrvC341Info.sOutputTimingInfo.uiVActive * 2 / 3), 0);
    dvC341_WriteToBuffer(B144_WPFLDDLYCH1, (oaoivst + m_sDrvC341Info.sOutputTimingInfo.uiVActive * 2 / 3), CH_WPBANK_OFFSET);   //H2PF_Simon_0150
    MS_SLEEP(100);

    if(m_sDrvC341Info.ePanelTimingId == PANEL_2D_OUTPUT)
    {
        if(m_sDrvC341Info.sOutputTimingInfo.eLow_Latency > eLLM_OFF && m_sDrvC341Info.sOutputTimingInfo.eLow_Latency < eLLM_INVALID)   //H2PF_Simon_0150
        {
            dvC341_WriteToBuffer(B144_WPOSYCTCH1, (0x0003 ), 0);
            dvC341_WriteToBuffer(B144_WPOSYCTCH1, (0x1013 ), 5);

            dvC341_WriteToBuffer(B144_WPOHCYCLCH1, m_sDrvC341Info.sOutputTimingInfo.uiHTotal/2 - 2, 0);
            dvC341_WriteToBuffer(B144_WPOHCYCLCH1, m_sDrvC341Info.sOutputTimingInfo.uiHTotal/2 - 2, 5);
            dvC341_WriteToBuffer(B144_WPOVCYCLCH1, m_sDrvC341Info.sOutputTimingInfo.uiVTotal * 1.1  - 2, 0);
            dvC341_WriteToBuffer(B144_WPOVCYCLCH1, m_sDrvC341Info.sOutputTimingInfo.uiVTotal * 1.1  - 2, 5);

            dvC341_WriteToBuffer(B144_WPOSYCT2CH1, 0x0, 0);
            dvC341_WriteToBuffer(B144_WPOSYCT2CH1, 0x0, 5);
            dvC341_WriteToBuffer(B144_WPOSYCT3CH1, 0x0, 0);
            dvC341_WriteToBuffer(B144_WPOSYCT3CH1, 0x0, 5);

            switch(m_sDrvC341Info.sOutputTimingInfo.eLow_Latency)   //H2PF_Simon_0150
            {
                case eLLM_LV1_FAST_WITH_FULL_WARPING:
                    dvC341_WriteToBuffer(B144_WPSYRDLYCH1, 0x02, 0);   //H2PF_Simon_0187
                    dvC341_WriteToBuffer(B144_WPSYRDLYCH1, 0x02, CH_WPBANK_OFFSET);
                    dvC341_WriteToBuffer(B144_WPFLDDLYCH1, (oaoivst + m_sDrvC341Info.sOutputTimingInfo.uiVActive * 2 / 3), 0);
                    dvC341_WriteToBuffer(B144_WPFLDDLYCH1, (oaoivst + m_sDrvC341Info.sOutputTimingInfo.uiVActive * 2 / 3), CH_WPBANK_OFFSET);
                    break;

                case eLLM_LV2_FAST_WITH_LIMITED_WARPING:
                default:
                    dvC341_WriteToBuffer(B144_WPSYRDLYCH1, SYRDLY_REG_FOR_LIMITED_WARP, 0);
                    dvC341_WriteToBuffer(B144_WPSYRDLYCH1, SYRDLY_REG_FOR_LIMITED_WARP, CH_WPBANK_OFFSET);
                    dvC341_WriteToBuffer(B144_WPFLDDLYCH1, FLDDLY_REG_FOR_LIMITED_WARP, 0);
                    dvC341_WriteToBuffer(B144_WPFLDDLYCH1, FLDDLY_REG_FOR_LIMITED_WARP, CH_WPBANK_OFFSET);
                    break;

                case eLLM_LV3_FAST_WITHOUT_WARPING:
                    break;

            }
        }
        else
        {
            dvC341_WriteToBuffer(B144_WPOSYCTCH1, (0x0001 ), 0);
            dvC341_WriteToBuffer(B144_WPOSYCTCH1, (0x1011 ), 5);

            dvC341_WriteToBuffer(B144_WPOHCYCLCH1, m_sDrvC341Info.sOutputTimingInfo.uiHTotal/2 - 2, 0);
            dvC341_WriteToBuffer(B144_WPOHCYCLCH1, m_sDrvC341Info.sOutputTimingInfo.uiHTotal/2 - 2, 5);
            dvC341_WriteToBuffer(B144_WPOVCYCLCH1, m_sDrvC341Info.sOutputTimingInfo.uiVTotal - 2, 0);
            dvC341_WriteToBuffer(B144_WPOVCYCLCH1, m_sDrvC341Info.sOutputTimingInfo.uiVTotal - 2, 5);

            dvC341_WriteToBuffer(B144_WPOSYCT2CH1, 0x0, 0);
            dvC341_WriteToBuffer(B144_WPOSYCT2CH1, 0x0, 5);
            dvC341_WriteToBuffer(B144_WPOSYCT3CH1, 0x0, 0);
            dvC341_WriteToBuffer(B144_WPOSYCT3CH1, 0x0, 5);
            dvC341_WriteToBuffer(B144_WPSYRDLYCH1, 0x0, 0);
            dvC341_WriteToBuffer(B144_WPSYRDLYCH1, 0x0, 5);
            dvC341_WriteToBuffer(B144_WPFLDDLYCH1, (oaoivst + m_sDrvC341Info.sOutputTimingInfo.uiVActive * 2 / 3), 0);   //H2PF_Simon_0150
            dvC341_WriteToBuffer(B144_WPFLDDLYCH1, (oaoivst + m_sDrvC341Info.sOutputTimingInfo.uiVActive * 2 / 3), CH_WPBANK_OFFSET);

        }
        //LOG_MSG(db_ALWAYS, "2Dx panel\r\n"); //H30K_Doulas_0016
    }
    else if(m_sDrvC341Info.ePanelTimingId == PANEL_3D_OUTPUT)
    {
        if((dvC341_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_SIDEBYSIDE) ||
            (dvC341_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_FRAMEPACKING) ||
            (dvC341_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_TOPANDBOTTOM))
        {
            //LOG_MSG(db_ALWAYS, "3Dx panel1 (%d)\r\n",ucForcedSyncReset);
            dvC341_WriteToBuffer(B144_WPOSYCTCH1, (0x8003 ), 0);  //H30K_Doulas_0038
            dvC341_WriteToBuffer(B144_WPOSYCTCH1, (0x1013 ), 5);
        }
        else if(dvC341_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_FRAME_SEQUENTIAL)
        {
            //LOG_MSG(db_ALWAYS, "3Dx panel2 (%d)\r\n",ucForcedSyncReset);
            dvC341_WriteToBuffer(B144_WPOSYCTCH1, (0x0003 ), 0);
            dvC341_WriteToBuffer(B144_WPOSYCTCH1, (0x1013 ), 5);
        }
        else
        {
            //LOG_MSG(db_ALWAYS, "3Dx panel3 (%d)\r\n",ucForcedSyncReset);
            dvC341_WriteToBuffer(B144_WPOSYCTCH1, (0x0005 ), 0); //H30K_Doulas_0034 0xc003
            dvC341_WriteToBuffer(B144_WPOSYCTCH1, (0x1015 ), 5); //H30K_Doulas_0034 0x1013
        }

        dvC341_WriteToBuffer(B144_WPOHCYCLCH1, uiH_Total/2 - 2, 0);
        dvC341_WriteToBuffer(B144_WPOHCYCLCH1, uiH_Total/2 - 2, 5);
        if(ucForcedSyncReset)
        {
            dvC341_WriteToBuffer(B144_WPOVCYCLCH1, uiV_Total * 1.1  - 2, 0);
            dvC341_WriteToBuffer(B144_WPOVCYCLCH1, uiV_Total * 1.1  - 2, 5);

            dvC341_WriteToBuffer(B144_WPSYRDLYCH1, 0x02, 0);   //H2PF_Simon_0187
            dvC341_WriteToBuffer(B144_WPSYRDLYCH1, 0x02, CH_WPBANK_OFFSET);
            dvC341_WriteToBuffer(B144_WPOSYCT3CH1, 0x80, 0);
            dvC341_WriteToBuffer(B144_WPOSYCT3CH1, 0x80, 5);
        }
        else
        {
            dvC341_WriteToBuffer(B144_WPOVCYCLCH1, uiV_Total - 2, 0);
            dvC341_WriteToBuffer(B144_WPOVCYCLCH1, uiV_Total - 2, 5);

            dvC341_WriteToBuffer(B144_WPSYRDLYCH1, 0x0, 0);
            dvC341_WriteToBuffer(B144_WPSYRDLYCH1, 0x0, 5);
            dvC341_WriteToBuffer(B144_WPOSYCT3CH1, 0x0, 0);
            dvC341_WriteToBuffer(B144_WPOSYCT3CH1, 0x0, 5);
        }
        dvC341_WriteToBuffer(B144_WPOSYCT2CH1, 0x0, 0);
        dvC341_WriteToBuffer(B144_WPOSYCT2CH1, 0x0, 5);
    }
    else  //240Hz Panel
    {
        //LOG_MSG(db_ALWAYS, "240x panel\r\n"); //H30K_Doulas_0016
        if(ucForcedSyncReset && m_sDrvC341Info.sOutputTimingInfo.eLow_Latency > eLLM_OFF && m_sDrvC341Info.sOutputTimingInfo.eLow_Latency < eLLM_INVALID) //H30K_Doulas_0018   //H2PF_Simon_0150
        {
            dvC341_WriteToBuffer(B144_WPOSYCTCH1, (0x0003 ), 0);
            dvC341_WriteToBuffer(B144_WPOSYCTCH1, (0x1013 ), 5);

            dvC341_WriteToBuffer(B144_WPOHCYCLCH1, uiH_Total/2 - 2, 0);
            dvC341_WriteToBuffer(B144_WPOHCYCLCH1, uiH_Total/2 - 2, 5);
            dvC341_WriteToBuffer(B144_WPOVCYCLCH1, uiV_Total * 1.1  - 2, 0);
            dvC341_WriteToBuffer(B144_WPOVCYCLCH1, uiV_Total * 1.1  - 2, 5);

            dvC341_WriteToBuffer(B144_WPOSYCT2CH1, 0x0, 0);
            dvC341_WriteToBuffer(B144_WPOSYCT2CH1, 0x0, 5);
            dvC341_WriteToBuffer(B144_WPOSYCT3CH1, 0x80, 0);
            dvC341_WriteToBuffer(B144_WPOSYCT3CH1, 0x80, 5);

            switch(m_sDrvC341Info.sOutputTimingInfo.eLow_Latency)   //H2PF_Simon_0150
            {
                case eLLM_LV1_FAST_WITH_FULL_WARPING:
                    dvC341_WriteToBuffer(B144_WPSYRDLYCH1, 0x02, 0);   //H2PF_Simon_0187
                    dvC341_WriteToBuffer(B144_WPSYRDLYCH1, 0x02, CH_WPBANK_OFFSET);
                    dvC341_WriteToBuffer(B144_WPFLDDLYCH1, (oaoivst + m_sDrvC341Info.sOutputTimingInfo.uiVActive * 2 / 3), 0);
                    dvC341_WriteToBuffer(B144_WPFLDDLYCH1, (oaoivst + m_sDrvC341Info.sOutputTimingInfo.uiVActive * 2 / 3), CH_WPBANK_OFFSET);
                    break;

                case eLLM_LV2_FAST_WITH_LIMITED_WARPING:
                default:
                    dvC341_WriteToBuffer(B144_WPSYRDLYCH1, SYRDLY_REG_FOR_LIMITED_WARP, 0);
                    dvC341_WriteToBuffer(B144_WPSYRDLYCH1, SYRDLY_REG_FOR_LIMITED_WARP, CH_WPBANK_OFFSET);
                    dvC341_WriteToBuffer(B144_WPFLDDLYCH1, FLDDLY_REG_FOR_LIMITED_WARP, 0);
                    dvC341_WriteToBuffer(B144_WPFLDDLYCH1, FLDDLY_REG_FOR_LIMITED_WARP, CH_WPBANK_OFFSET);
                    break;

                case eLLM_LV3_FAST_WITHOUT_WARPING:
                    break;

            }

        }
        else
        {
            dvC341_WriteToBuffer(B144_WPOSYCTCH1, (0x0001 ), 0);
            dvC341_WriteToBuffer(B144_WPOSYCTCH1, (0x1011 ), 5);
            dvC341_WriteToBuffer(B144_WPOHCYCLCH1, uiH_Total/2 - 2, 0);
            dvC341_WriteToBuffer(B144_WPOHCYCLCH1, uiH_Total/2 - 2, 5);
            dvC341_WriteToBuffer(B144_WPOVCYCLCH1, uiV_Total - 2, 0);
            dvC341_WriteToBuffer(B144_WPOVCYCLCH1, uiV_Total - 2, 5);

            dvC341_WriteToBuffer(B144_WPOSYCT2CH1, 0x0, 0);
            dvC341_WriteToBuffer(B144_WPOSYCT2CH1, 0x0, 5);
            dvC341_WriteToBuffer(B144_WPOSYCT3CH1, 0x0, 0);
            dvC341_WriteToBuffer(B144_WPOSYCT3CH1, 0x0, 5);
            dvC341_WriteToBuffer(B144_WPSYRDLYCH1, 0x0, 0); //H30K_Doulas_0018
            dvC341_WriteToBuffer(B144_WPSYRDLYCH1, 0x0, 5); //H30K_Doulas_0018
			dvC341_WriteToBuffer(B144_WPFLDDLYCH1, (oaoivst + m_sDrvC341Info.sOutputTimingInfo.uiVActive * 2 / 3), 0);
            dvC341_WriteToBuffer(B144_WPFLDDLYCH1, (oaoivst + m_sDrvC341Info.sOutputTimingInfo.uiVActive * 2 / 3), CH_WPBANK_OFFSET);

            ucForcedSyncReset = FALSE;
        }
    }

#else
        if(ucForcedSyncReset)
        {
            dvC341Geo_Write(B2_OSYCT, 0x0313, 0);
            if((m_sDrvC341Info.ePanelTimingId == ePANEL_ID_1080P_120HZ) ||
               (m_sDrvC341Info.ePanelTimingId == ePANEL_ID_WUXGA_120HZ))    //A70LV_Doulas_0377 Modify//A70LV_Doulas_0154
            {
                dvC341Geo_Write(B2_OHCYCL, (uiH_Total*2 - 2), 0);
            }
            else
            {
                dvC341Geo_Write(B2_OHCYCL, (uiH_Total - 2), 0);
            }
            dvC341Geo_Write(B2_OVCYCL, (uiV_Total*1.1 - 2), 0);

            //dvC341Geo_Write(B2_SYRDLY, 0x02, 0);        //A70LV_Doulas_0154 remove
            dvC341Geo_Write(B2_FLDDLY, 0x01, 0);
            LOG_MSG(db_DV_GEO , "(func:%s,%d) dvC341Geo_ForcedSyncResetValueSet (%d,%d)\n", __FUNCTION__, __LINE__, uiH_Total,uiV_Total);   //A70LV_Doulas_0154
        }
        else
        {
            dvC341Geo_Write(B2_OSYCT, 0x0313, 0);
            if((m_sDrvC341Info.ePanelTimingId == ePANEL_ID_1080P_120HZ) ||
               (m_sDrvC341Info.ePanelTimingId == ePANEL_ID_WUXGA_120HZ))    //A70LV_Doulas_0377 Modify//A70LV_Doulas_0154
            {
                dvC341Geo_Write(B2_OHCYCL, (uiH_Total*2 - 2), 0);
            }
            else
            {
                dvC341Geo_Write(B2_OHCYCL, (uiH_Total - 2), 0);
            }
            dvC341Geo_Write(B2_OVCYCL, (uiV_Total - 2), 0);
            dvC341Geo_Write(B2_SYRDLY, 0x00, 0);
        }
#endif

    m_ucForcedSyncReset = ucForcedSyncReset;   //A35G2_CDS_Simon_0033
    m_uiH_Total = uiH_Total;
    m_uiV_Total = uiV_Total;
//#endif

}

void dvC341Geo_OutputEnableSet(UINT8 ucEnable)
{
#if 0
    dvC341Geo_Write(B2_OFILL, 0x000000, 0);

    if(!ucEnable)
    {
        dvC341Geo_Write(B2_OIMGCT, 0x01, 0);
    }
    else
    {
        dvC341Geo_Write(B2_OIMGCT, 0x00, 0);
    }
#endif
    //dvC341Geo_Write(BN_MCT, 0x50+ucEnable, 0);      //A70LV_Doulas_0326 remove
    ucC341_OutputEnable = ucEnable;
}

UINT8 dvC341Geo_OutputEnableGet(void)
{
    return ucC341_OutputEnable;
}

eC341GEO_EXEC_CODE dvC341Geo_Change_Panel(const ePANEL_ID ePanelId)
{
    UINT8 ucTblIndex = 0;
    eC341GEO_EXEC_CODE eExecCode = eC341GEO_EXEC_CODE_PASS;

    m_sDrvC341Info.ePanelTimingId = ePanelId; //H30K_Doulas_0010

    LOG_MSG(db_DV_GEO, "(func:%s, line%d) Change Panel ID %d\n", __func__, __LINE__, ePanelId);

    if((eExecCode = dvC341Geo_GetPanelIndex(ePanelId, &ucTblIndex)) != eC341GEO_EXEC_CODE_PASS)
    {
        LOG_MSG(db_DV_GEO , "(func:%s, line:%d) C341 Get Panel Timing Index Fail = %d\n", __FUNCTION__, __LINE__, eExecCode);
        return eExecCode;
    }

    LOG_MSG(db_DV_GEO, "(func:%s, line%d) Get Panel Index %d\n", __func__, __LINE__, ucTblIndex);

    dvC341Geo_GetOutputTiming(ucTblIndex);
    dvC341Geo_ConfigOutput();

    if(DEF_OSD_PANEL_WIDTH < m_sDrvC341Info.sOutputTimingInfo.uiHActive)
    {
        dvC341Geo_EnableMagnifyOSD2X(TRUE);
    }
    else
    {
        dvC341Geo_EnableMagnifyOSD2X(FALSE);
    }


#if 0
    UINT8 aucLUT[48] = {0};
    UINT8 aucBlendingEdge[30] = {0};
    UINT8 ucRTCT = 0;
    UINT16 uiEGBCT  = 0;
    UINT16 uiDTCT  = 0; //A70LV_Larry_0308
    UINT16 uiUCCT  = 0;
//    UINT8 u8Idx;

    if((eExecCode = dvC341Geo_GetPanelIndex(ePanelId, &ucTblIndex)) != eC341GEO_EXEC_CODE_PASS)
    {
        LOG_MSG(db_DV_GEO , "(func:%s, line:%d) C341 Get Panel Timing Index Fail = %d\n", __FUNCTION__, __LINE__, eExecCode);
        return eExecCode;
    }



    uiDTCT = (UINT16)dvC341Geo_Read(B5_DTCT, 0); //A70LV_Larry_0308

    if(ucColorUniformityEnable)
    {
        uiUCCT = (UINT16)dvC341Geo_Read(B5_UCCT, 0);
    }

    dvC341Geo_BurstRead_AddInc(B6_HLUT0, 48, aucLUT); //A70LV_Larry_0177
//    for(u8Idx=0; u8Idx<48; u8Idx++)
//    {
//        aucLUT[u8Idx] = (UINT8)dvC341Geo_Read(B6_HLUT0+0x0100*u8Idx, 0);
//    }
    dvC341Geo_BurstRead_AddInc(B7_LEGBHST, 30, aucBlendingEdge); //A70LV_Larry_0177
//    for(u8Idx=0; u8Idx<30; u8Idx++)
//    {
//        aucBlendingEdge[u8Idx] = (UINT8)dvC341Geo_Read(B7_LEGBHST+0x0100*u8Idx, 0);
//    }

    uiEGBCT = (UINT16)dvC341Geo_Read(B7_EGBCT, 0); //A70LV_Larry_0177

    ucRTCT = (UINT8)dvC341Geo_Read(B0_RTCT6, 0); //A70LV_Larry_0177
    dvC341Geo_Write(BN_RTCT, 0x00, 0); //A70LV_Larry_0177

#if 1
    dvC341Geo_Write(BN_RSTCT, 0x01, 0);//Soft Reset
    MS_SLEEP(120);
    dvC341Geo_Write(BN_RSTCT, 0x00, 0);
#endif



    m_sDrvC341Info.bInit = TRUE;
    m_sDrvC341Info.ePanelTimingId = ePanelId;
    m_sDrvC341Info.ulMclko_Freq = 396000000;   //Should be the same with memory clock 12x66 = 792M (792/2)
    m_sDrvC341Info.ucRegISYCT = 0x31;    //A35G2_CDS_Simon_0003

    dvC341Geo_GetOutputTiming(ucTblIndex);
    dvC341Geo_ConfigClock(m_sPanelTable[ucTblIndex].sOutputTimingInfo.ucPoRefDiv,
                     m_sPanelTable[ucTblIndex].sOutputTimingInfo.ucPoFbDiv);

    PS_WP_HW = m_sPanelTable[ucTblIndex].sOutputTimingInfo.uiHActive ;  //A35G2_CDS_Simon_0023
    PS_WP_VW = m_sPanelTable[ucTblIndex].sOutputTimingInfo.uiVActive ;

    dvC341Geo_ConfigBasicMemory();

    dvC341GeoCtrl_Change_Panel_Init(); //A70LV_Larry_0177

    //dvC341Geo_BurstWrite_AddInc(B6_HLUT0, 48, aucLUT); //A70LV_Larry_0177
//    for(u8Idx=0; u8Idx<48; u8Idx++)
//    {
//        dvC341Geo_Write(B6_HLUT0+0x0100*u8Idx, aucLUT[u8Idx], 0);
//    }
    //dvC341Geo_BurstWrite_AddInc(B7_LEGBHST, 30, aucBlendingEdge); //A70LV_Larry_0177
//    for(u8Idx=0; u8Idx<30; u8Idx++)
//    {
//        dvC341Geo_Write(B7_LEGBHST+0x0100*u8Idx, aucBlendingEdge[u8Idx], 0);
//    }
    dvC341Geo_Write(B7_EGBCT, uiEGBCT, 0); //A70LV_Larry_0177

    dvC341Geo_ConfigMemAD();
    dvC341Geo_ConfigInput();
    dvC341Geo_ConfigOutput();

    dvC341Geo_Write(B145_WPDTCTCH1, uiDTCT, 0); //A70LV_Larry_0308
    dvC341Geo_Write(B148_WPEGBDBDSADCH1, TWIST_DEF_BLEND_ADDRESS, 0);  //A70LV_Larry_0456

    if(ucColorUniformityEnable)
    {
        dvC341Geo_Write(B13_UCCTCH1,   uiUCCT, 0);
    }

    dvC341Geo_Write(B0_RTCT6, ucRTCT, 0); //A70LV_Larry_0177

    dvC341Geo_OSDACTStartConfig(FALSE);  //A35G2_CDS_Simon_0023

    LOG_MSG(db_DV_GEO , "(func:%s, line:%d) C341 Panel Change = %d\n", __FUNCTION__, __LINE__, eExecCode);
    m_ucOutputChanged = TRUE;   //A70LV_Doulas_0307
#endif

    return eExecCode;
}

void dvC341Geo_FlashEraseSector(UINT32 ulSflAd, UINT32 ulDataSize) //A70LV_Larry_0139
{
#if 0
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
        dvC341Geo_Write(B1_SFLSZSEL, 0x01, 0);
        dvC341Geo_Write(B1_SFLCT, 0x00, 0);
        dvC341Geo_Write(B1_SFLMODE, 0x80, 0);
        dvC341Geo_Write(B1_SFLMODE, 0x13, 0);
        dvC341Geo_Write(B1_SFLCT, 0x00, 0);
        dvC341Geo_Write(B1_SFLCMDWR, 0x12, 0);  //Set the page write command to the serial Flash connected
        dvC341Geo_Write(B1_SFLCMDRD, 0x0C, 0);  //Set a read or fast read command to be executed for the serial Flash connected
        dvC341Geo_Write(B1_SFLCMDER, 0xDC, 0);  //Set a selector erase or bulk (chip) erase command to be executed for the serial Flash connected
        dvC341Geo_Write(B1_SFLCMDWREN, 0x06, 0);  //Set the value of WREN command of the serial Flash connected
        dvC341Geo_Write(B1_SFLCMDRDSR, 0x05, 0);  //Set the value of RDSR command of the serial Flash connected
        dvC341Geo_Write(B1_SFLAD, ulEraseAddrStart, 0);  //Specify the serial Flash address. After accessing, auto increment starts
        dvC341Geo_Write(B1_SFLCT, 0x04, 0);

//A70LV_Larry_0336 [[
        ucRetryCount = 200;
        //
        do
        {
            ucSFLSTAT = dvC341Geo_Read(B1_SFLSTAT, 0); //A70LV_Larry_0054
            MS_SLEEP(10);
        } while (((ucSFLSTAT & 0x02) != 0x00) && (ucRetryCount--));

        ASSERT(ucRetryCount != 0);

        //


        ucRetryCount = 200;
        //
        do
        {
            ucSFLRDSR = dvC341Geo_Read(B1_SFLRDSR, 0); //A70LV_Larry_0054
            MS_SLEEP(10);
        } while (((ucSFLRDSR & 0x03) != 0x00) && (ucRetryCount--));

        ASSERT(ucRetryCount != 0);

        //
//A70LV_Larry_0336 ]]
    }
//A70LV_Larry_0332 ]]

#endif
    return;
}

void dvC341Geo_DMA_Transfer(UINT32 ulAddress, UINT32 ulSize, UINT8 ucRegsel, UINT32 ulDmaCnt) //A70LV_Larry_0139
{
#if 0
    UINT8  ucSFLSTAT = 0;
    UINT8  ucREAD_SFLSTAT = 0;
    UINT16 ucRetryCount = 1000;

    if(ulDmaCnt == eC341GEO_SFLCT_DMA_REG2FLASH) //A70LV_Larry_0227
    {
        ucSFLSTAT = 0x03; //1:WBUSY 0;DMA BUSY
    }
    else
    {
        ucSFLSTAT = 0x01;
    }



    dvC341Geo_Write(B1_SFLMODE,0x80, 0);
    dvC341Geo_Write(B1_SFLMODE,0x15, 0);
    dvC341Geo_Write(B1_SFLCT,0x00, 0);
    dvC341Geo_Write(B1_SFLCMDWR,0x12, 0);
    dvC341Geo_Write(B1_SFLCMDRD,0x0C, 0);
    dvC341Geo_Write(B1_SFLCMDWREN,0x06, 0);
    dvC341Geo_Write(B1_SFLCMDRDSR,0x05, 0);
    dvC341Geo_Write(B1_SFLAD,ulAddress, 0);
    dvC341Geo_Write(B1_SFLCNT,ulSize, 0);

    if(ucRegsel != eC341GEO_SFLREGSEL_NA) //A70LV_Larry_0174
    {
        dvC341Geo_Write(B1_SFLREGSEL,ucRegsel, 0);
    }

    dvC341Geo_Write(B1_SFLSZSEL,0x01, 0);
    dvC341Geo_Write(B1_SFLCT,ulDmaCnt, 0);

//A70LV_Larry_0336 [[
    //if((ulDmaCnt == eC341GEO_SFLCT_DMA_REG2FLASH) && (ucRegsel == eC341_SFLREGSEL_DCL))
    //{
        //MS_SLEEP(1000);
    //}

    do
    {
		// NOP
        MS_SLEEP(10);
        ucREAD_SFLSTAT = dvC341Geo_Read(B1_SFLSTAT, 0);

	}while(((ucREAD_SFLSTAT & ucSFLSTAT) != 0) && (ucRetryCount--)) ; // busy waiting

    ASSERT(ucRetryCount != 0);

    dvC341Geo_Write(BN_RGBNK, 0x00, 0); //A70LV_Larry_0318
    dvC341Geo_Write(B1_SFLCT, 0x00, 0); //A70LV_Larry_0227

#endif
}

void dvC341Geo_FlashWrite(UINT32 ulAddress, UINT16 uiSize, UINT8 *pucData) //A70LV_Larry_0174
{
#if 0
    UINT8   ucData = 0;
    UINT16  uiCount = 0;
    UINT8   ucSFLDTCTL = 0xff;
    UINT8   ucSFLSTAT = 0xff;



    dvC341Geo_Write(B1_SFLCT, 0x00, 0);
    dvC341Geo_Write(B1_SFLMODE,0x80, 0);
    dvC341Geo_Write(B1_SFLMODE,0x15, 0);

    dvC341Geo_Write(B1_SFLCMDWR,0x12, 0);
    dvC341Geo_Write(B1_SFLCMDWREN,0x06, 0);
    dvC341Geo_Write(B1_SFLCMDRDSR,0x05, 0);
    dvC341Geo_Write(B1_SFLSZSEL,0x01, 0);
    dvC341Geo_Write(B1_SFLAD,ulAddress, 0);

//A70LV_Larry_0336 [[
    uiCount = 200;

    //
    do
    {
        MS_SLEEP(10);
        ucSFLDTCTL = dvC341Geo_Read(B1_SFLDTCTL, 0);
    }
    while(((ucSFLDTCTL & 0x20) == 0x00) && (uiCount--));

    ASSERT(uiCount != 0);
    //

    uiCount = 200;

    //
    do
    {
        MS_SLEEP(10);
        ucSFLSTAT = dvC341Geo_Read(B1_SFLSTAT, 0);
    }
    while (((ucSFLSTAT & 0x02) != 0x00) && (uiCount--));

    ASSERT(uiCount != 0);
    //
//A70LV_Larry_0336 ]]
#if 0
    dvC341Geo_BurstWrite_FixedAdd(B1_SFLDT, uiSize, pucData);  //data shift issue
#else
    for(uiCount = 0; uiCount<uiSize; uiCount++)
    {
        ucData = pucData[uiCount];
//        dvC341Geo_Write(B1_SFLDT, ucData);
        dvC341Geo_WriteToBuffer(B1_SFLDT, ucData, 0); //G100_Owen_0026
    }
    dvC341Geo_Buffer_Flush();  //G100_Owen_0026
#endif

#endif
}

void dvC341Geo_FlashRead(UINT32 ulAddress, UINT16 uiSize, UINT8 *pucData) //A70LV_Larry_0174
{
#if 0
    UINT8   ucData = 0;
    UINT8   ucSFLDTCTL = 0xff;
    UINT16  uiCount = 0;



    dvC341Geo_Write(B1_SFLCT, 0x00, 0);
    dvC341Geo_Write(B1_SFLMODE,0x80, 0);
    dvC341Geo_Write(B1_SFLMODE,0x15, 0);
    dvC341Geo_Write(B1_SFLCMDRD,0x0c, 0);
    dvC341Geo_Write(B1_SFLSZSEL,0x01, 0);
    dvC341Geo_Write(B1_SFLAD,ulAddress, 0);
    dvC341Geo_Write(B1_SFLDTCTL,0x01, 0);

//A70LV_Larry_0336 [[
    uiCount = 200;
    //
    do
    {
        MS_SLEEP(10);
        ucSFLDTCTL = dvC341Geo_Read(B1_SFLDTCTL, 0);
    } while (((ucSFLDTCTL & 0x80)) && (uiCount--));

    ASSERT(uiCount != 0);

    //
//A70LV_Larry_0336 ]]
#if 1
    dvC341Geo_BurstRead_FixedAdd(B1_SFLDT, uiSize, pucData);
#else
    for(uiCount = 0; uiCount<uiSize; uiCount++)
    {
        ucData = dvC341Geo_Read(B1_SFLDT, 0);
        pucData[uiCount] = ucData;
    }
#endif

#endif
}

void dvC341Geo_FrmaeDelaySet(UINT8 ucValue)       //A70LV_Doulas_0154
{
#if 0
	#ifdef Low_Latency_All
	if(m_sDrvC341Info.sOutputTimingInfo.eLow_Latency)
	{
	}
	else
	{
        if(m_ucFrmaeDelay > ucValue)
        {
            UINT8 ucOSYCT3 = 0;
            UINT8 ucRTCT = 0;

            ucRTCT = dvC341Geo_Read(BN_RTCT, 0);
            ucOSYCT3 = dvC341Geo_Read(B2_OSYCT3, 0);

            if(ucOSYCT3 & 0x80)
            {
                dvC341Geo_Write(BN_RTCT, 0x03, 0);
                dvC341Geo_Write(B2_OSYCT3, 0x00, 0);
                MS_SLEEP(45);
            }

            dvC341Geo_Write(B2_SYRDLY, ucValue, 0);

            if(ucOSYCT3 & 0x80)
            {
                dvC341Geo_Write(B2_OSYCT3, ucOSYCT3, 0);
                MS_SLEEP(45);
                dvC341Geo_Write(BN_RTCT, ucRTCT, 0);
            }
        }
        else
        {
            dvC341Geo_Write(B2_SYRDLY, ucValue, 0);
        }

        m_ucFrmaeDelay = ucValue;
	}
	#else
		dvC341Geo_Write(B2_SYRDLY, ucValue, 0);
	#endif	/*Low_Latency_All*/
#endif
}

UINT16 dvC341Geo_Output_V_Total_Get(void)   //A70LV_Doulas_0154
{
    return m_sDrvC341Info.sOutputTimingInfo.uiVTotal;
}


#if 0
void dvC341Geo_RegDump(void)
{
#if 1	//G100_Doulas_0027 test
	UINT16 wCount = 0;
	UINT32 dwData = 0;

	for(wCount = 0; wCount < C341REG_NUMBER; wCount++)
		{
			dwData = dvC341Geo_Read(m_sICHIPC341Geo_DUMP[wCount].ulAddress, 0);
			switch(m_sICHIPC341Geo_DUMP[wCount].ulAddress & 0xFF)
			{
				case 1:
					LOG_MSG(db_ALWAYS, "C341%s %02xh\r\n",	m_sICHIPC341Geo_DUMP[wCount].cReg, dwData);
					break;
				case 2:
					LOG_MSG(db_ALWAYS, "C341%s %04xh\r\n",	m_sICHIPC341Geo_DUMP[wCount].cReg, dwData);
					break;
				case 3:
					LOG_MSG(db_ALWAYS, "C341%s %06xh\r\n",	m_sICHIPC341Geo_DUMP[wCount].cReg, dwData);
					break;
				case 4:
					LOG_MSG(db_ALWAYS, "C341%s %08xh\r\n",	m_sICHIPC341Geo_DUMP[wCount].cReg, dwData);
					break;
				default:
					break;
			}
			MS_SLEEP(3);
		}


#else
    //UINT16 wCount = 0;
    UINT32 dwData = 0;

    sICHIP_DUMP* m_pC341Reg;

    m_pC341Reg = (sICHIP_DUMP*)ICHIPC341_DUMP_ADDR;

    if(m_pC341Reg->ulAddress == 0x12345678)
    {
        LOG_MSG(db_ALWAYS, "C341 %s\r\n",  m_pC341Reg->cReg);

        m_pC341Reg++;

        while(1)
        {
            if(m_pC341Reg->ulAddress == 0xFFFFFFFF)
            {
                LOG_MSG(db_ALWAYS, "C341 %s\r\n",  m_pC341Reg->cReg);
                return;
            }
            else
            {
                dwData = dvC341Geo_Read(m_pC341Reg->ulAddress, 0);

                switch(m_pC341Reg->ulAddress & 0xFF)
                {
                    case 1:
                        LOG_MSG(db_ALWAYS, "C341%s %02xh\r\n",  m_pC341Reg->cReg, dwData);
                        break;
                    case 2:
                        LOG_MSG(db_ALWAYS, "C341%s %04xh\r\n",  m_pC341Reg->cReg, dwData);
                        break;
                    case 3:
                        LOG_MSG(db_ALWAYS, "C341%s %06xh\r\n",  m_pC341Reg->cReg, dwData);
                        break;
                    case 4:
                        LOG_MSG(db_ALWAYS, "C341%s %08xh\r\n",  m_pC341Reg->cReg, dwData);
                        break;
                    default:
                        break;
                 }

                 MS_SLEEP(10);
            }

            m_pC341Reg++;
        }

    }
    else
    {
        return;
    }
#endif
}
#endif


UINT8 dvC341Geo_OutputChanged_Get(void)        //A70LV_Doulas_0307
{
    return m_ucOutputChanged;
}

void dvC341Geo_OutputChanged_Set(UINT8 ucVal)        //A70LV_Doulas_0307
{
    m_ucOutputChanged = ucVal;
}

void dvC341Geo_ColorUniformity_Control(UINT8 ucEnable, UINT8 ucCPUAccessEnable)
{
    int GV_RTCT = dvC341Geo_Read(B0_RTCT6, 0);
    LOG_MSG(db_DV_GEO, "(func:%s, line:%d)(%d,%d)\n", __FUNCTION__, __LINE__, ucEnable, ucCPUAccessEnable);	//A65_OPTOMA_Doulas_0136

    for(eC341_CH eCH = eC341_CH1 ; eCH < OUTPUT_CH_NUM ; eCH++)
    {
        UINT8 BankOffset = eCH * CH_BANK_OFFSET;

        if(ucEnable == 0)
        {
            if (ucCPUAccessEnable == 0)
            {
                dvC341_Write( B0_RTCT6, 0, 0);
                dvC341_Write( B13_UCCTCH1, 0x0100, BankOffset);
            }
            else
            {
                dvC341_Write( B0_RTCT6, 0, 0);
                dvC341_Write( B13_UCCTCH1, 0x0102, BankOffset);
            }
            //dvC341Geo_Write( B13_ERDFCTCH1, 0x60, BankOffset);  //A65_OPTOMA_CL_0002 for contouring  //A35G2_CDS_CL_0001
            //dvC341Geo_Write( B13_ERDFCT2CH1, 0x18, BankOffset);
        }
        else
        {
            if(ucCPUAccessEnable == 0)
            {
                dvC341_Write( B0_RTCT6, 0, 0);
                dvC341_Write( B13_UCCTCH1, 0x0101, BankOffset);
            }
            else
            {
                dvC341_Write( B0_RTCT6, 0, 0);
                dvC341_Write( B13_UCCTCH1, 0x0103, BankOffset);
            }
            //dvC341Geo_Write( B13_ERDFCTCH1, 0x60, BankOffset);  //A65_OPTOMA_CL_0002 for contouring  //A35G2_CDS_CL_0001
            //dvC341Geo_Write( B13_ERDFCT2CH1, 0x1c, BankOffset);
        }
    }

    dvC341_Write( B0_RTCT6, GV_RTCT, 0);

    dvC341Geo_ColorUniformityEnable_Set(ucEnable);
    MS_SLEEP(20);

}

void dvC341Geo_ColorUniformityEnable_Set(UINT8 ucVal)
{
    ucColorUniformityEnable = ucVal;
}

UINT8 dvC341Geo_ColorUniformityEnable_Get(void)    //A65_OPTOMA_CL_0001  //A35G2_CDS_CL_0001
{
    return ucColorUniformityEnable;
}

void dvC341Geo_COMS_Output_Set(UINT8 ucEnable)     //A70LV_Doulas_0361
{
#if 0
    if(ucEnable)
    {
        dvC341Geo_Write(B0_CMOSOUTSETUP, 0x50, 0);
        dvC341Geo_Write(B0_POCLKCT,  0x01, 0);          //A70LV_Doulas_0363
    }
    else
    {
        dvC341Geo_Write(B0_CMOSOUTSETUP, 0x00, 0);
        dvC341Geo_Write(B0_POCLKCT,  0x00, 0);          //A70LV_Doulas_0363
    }
#endif
}

UINT8 dvC341Geo_OutputV_FreqGet(void)     //A70LV_Doulas_0367
{
    UINT8 ucV_Freq = 60;//1;

#if 0
#ifdef C341GEO_2D_NO_FRAME_LOCK
    if((m_sDrvC341Info.ePanelTimingId == ePANEL_ID_1080P_120HZ) ||
       (m_sDrvC341Info.ePanelTimingId == ePANEL_ID_WUXGA_120HZ))    //A70LV_Doulas_0377 Modify
    {
        if(m_uiH_Total*2 > (m_sDrvC341Info.sOutputTimingInfo.uiHTotal + 50))
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
            if(m_uiH_Total > (m_sDrvC341Info.sOutputTimingInfo.uiHTotal + 50))
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
#endif

    return ucV_Freq;
}


#ifdef Low_Latency_All
void dvC341Geo_Low_Latency_Set(eLOW_LATENCY_MODE eLLMode)   //H2PF_Simon_0150
{
    m_sDrvC341Info.sOutputTimingInfo.eLow_Latency = eLLMode;

    LOG_MSG(db_DV_GEO, "(func:%s,line:%d) set %d\r\n", __FUNCTION__, __LINE__, m_sDrvC341Info.sOutputTimingInfo.eLow_Latency);

	//if(!ucEnable) //H30K_Doulas_0016
	//{
	//	m_ucForcedSyncReset = FALSE;
	//	m_uiH_Total = m_sDrvC341Info.sOutputTimingInfo.uiHTotal;
	//	m_uiV_Total = m_sDrvC341Info.sOutputTimingInfo.uiVTotal;
	//}
	//dvC341Geo_ConfigMemAD();
	//dvC341Geo_ConfigOutput(); //H30K_Doulas_0016
}
eLOW_LATENCY_MODE dvC341Geo_Low_Latency_Get(void)     	//ZU860_Doulas_00152     //H2PF_Simon_0150
{
    return m_sDrvC341Info.sOutputTimingInfo.eLow_Latency;
}
#endif	/*Low_Latency_All*/

void dvC341Geo_Test(UINT16 *puiTtlCnt, UINT16 *puiErrCnt)  //G100_Owen_0085
{
#if 0
    uOPD_DATA uOPDData = {0};
    UINT8 ucIdx, ucData, ucErrIdx = 0xFF;
    UINT16 uiTotal = 0, uiErrCnt = 0;

    sprintf(uOPDData.sINTERFACE.cOPD_Type, "SPI");
    sprintf(uOPDData.sINTERFACE.cIC_Name, "C341");
    sprintf(uOPDData.sINTERFACE.cResult, "PASS");

//    GPIO_Write(32, 0);
    for(ucIdx=0xFF; ucIdx>0; ucIdx--)   //G100_Owen_0137
    {
        dvC341Geo_Write(B1_TESTG, ucIdx, 0);
        MS_SLEEP(5);
        ucData = (dvC341Geo_Read(B1_TESTG, 0) & 0xFF);
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
            LOG_MSG(db_DV_GEO, "C341 W[%X] R[%X]\r\n", ucIdx, ucData);
//            break;
        }
    }
    dvC341Geo_Write(B1_TESTG, 0, 0);  //G100_Owen_0137

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

    LOG_MSG(db_DV_GEO, "C341 Total[%d], Error[%d]\r\n", uiTotal, uiErrCnt);
#endif
}



//SetEgbBiasCursorEn
void dvC341Geo_SetEdgebldBiasCursorEnable(UINT8 ucEnable)
{
#if 0
	if ( ucEnable == 0 )
	{
		dvC341Geo_Write( B8_CURCT, 0x0016 , 0);
	}
	else
	{
		dvC341Geo_Write( B8_CURCT, 0x0017 , 0);
	}
#endif
}

//SetEgbBiasCurPos
void dvC341Geo_SetEdgebldBiasCursorPos( int selx, int sely, int dx, int dy )
{
#if 0
	PM_EBIAS_CUR[selx][sely].x = PM_EBIAS_CUR[selx][sely].x + dx;
	PM_EBIAS_CUR[selx][sely].x = ( PM_EBIAS_CUR[selx][sely].x < 0 ) ? 0 : ( PM_EBIAS_CUR[selx][sely].x >= PS_WP_HW ) ? PS_WP_HW - 1 : PM_EBIAS_CUR[selx][sely].x;

	PM_EBIAS_CUR[selx][sely].y = PM_EBIAS_CUR[selx][sely].y + dy;
	PM_EBIAS_CUR[selx][sely].y = ( PM_EBIAS_CUR[selx][sely].y < 0 ) ? 0 : ( PM_EBIAS_CUR[selx][sely].y >= PS_WP_VW ) ? PS_WP_VW - 1 : PM_EBIAS_CUR[selx][sely].y;

	if ( selx == 0 )
	{
		if ( sely == 0 )  //TL
		{
			dvC341Geo_Write( B8_CURTLX, PM_EBIAS_CUR[selx][sely].x , 0);
			dvC341Geo_Write( B8_CURTLY, PM_EBIAS_CUR[selx][sely].y , 0);
		}

		if ( sely == 1 )  //BL
		{
			dvC341Geo_Write( B8_CURBLX, PM_EBIAS_CUR[selx][sely].x , 0);
			dvC341Geo_Write( B8_CURBLY, PM_EBIAS_CUR[selx][sely].y - (Def_4CURSOR_Size - 1) , 0);
		}
	}

	if ( selx == 1 )
	{
		if ( sely == 0 )  //TR
		{
			dvC341Geo_Write( B8_CURTRX, PM_EBIAS_CUR[selx][sely].x - (Def_4CURSOR_Size - 1) , 0);
			dvC341Geo_Write( B8_CURTRY, PM_EBIAS_CUR[selx][sely].y , 0);
		}

		if ( sely == 1 )  //BR
		{
			dvC341Geo_Write( B8_CURBRX, PM_EBIAS_CUR[selx][sely].x - (Def_4CURSOR_Size - 1) , 0);
			dvC341Geo_Write( B8_CURBRY, PM_EBIAS_CUR[selx][sely].y - (Def_4CURSOR_Size - 1) , 0);
		}
	}
#endif
}


//EgbBiasEn
void dvC341Geo_EdgebldBiasEnable(INT8 cEnable)
{
    if(ADVANCED_BLACK_LEVEL == FALSE)
    {
        cEnable = FALSE;
    }

	if ( cEnable == FALSE )
	{
		dvC341Geo_Write( B147_WPEGBIASCTCH1, 0x0050 , eC341_CH1);  //H2PF_Simon_0109
		dvC341Geo_Write( B147_WPEGBIASCTCH1, 0x0050 , eC341_CH2);
	}
	else
	{
		dvC341Geo_Write( B147_WPEGBIASCTCH1, 0x0051 , eC341_CH1);
		dvC341Geo_Write( B147_WPEGBIASCTCH1, 0x0051 , eC341_CH2);
	}
}

//SetEgbBiasPlt
void dvC341Geo_SetEdgebldBiasPalette( int plt, int r, int g, int b )
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

void dvC341Geo_BlackLevel_AreaWrite( int plt_sel, coord_t P1, coord_t P2, coord_t P3, coord_t P4 , int iDataWrite)
{
#if 0
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
    			dvC341Geo_WPLT2( plt_sel, px, y );// bias area file
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
	dvC341Geo_Write( B0_CPUWAD, 0 , 0);// dummy

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
#endif
}


void dvC341Geo_WPLT2( int plt_sel, pos_t x, int y )   // bias area file
{
#if 0
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
		dvC341Geo_WriteToBuffer( B0_CPURAD, ad , 0);
		dvC341Geo_WriteToBuffer( B0_CPUDTCTL, 0x01 , 0);
		dvC341Geo_Buffer_Flush();
		fill = dvC341Geo_Read( B0_CPUDT, 0 ) & 0x0f;
		fill = fill + ( plt_sel << 4 );
		dvC341Geo_WriteToBuffer( B0_CPUWAD, ad , 0);
		dvC341Geo_WriteToBuffer( B0_CPUDT, fill , 0);//Data write
		dvC341Geo_Buffer_Flush();
		x.st = x.st + 1;
	}

	if ( (x.st <= x.end) && ((x.end & 0x01) == 0) )
	{
		ad = DEF_EBIASSAD + ( x.end >> 1 ) + y * DEF_EBIASMWI * 256;
		dvC341Geo_WriteToBuffer( B0_CPURAD, ad, 0 );
		dvC341Geo_WriteToBuffer( B0_CPUDTCTL, 0x01, 0 );
		dvC341Geo_Buffer_Flush();
		fill = dvC341Geo_Read( B0_CPUDT, 0 ) & 0xf0;
		fill = fill + plt_sel;
		dvC341Geo_WriteToBuffer( B0_CPUWAD, ad, 0 );
		dvC341Geo_WriteToBuffer( B0_CPUDT, fill, 0 );//Data write
		dvC341Geo_Buffer_Flush();
		x.end = x.end - 1;
	}

	if ( x.st <= x.end )
	{
		ad = DEF_EBIASSAD + ( x.st >> 1 ) + y * DEF_EBIASMWI * 256;
		bbacthw = (x.end >> 1) - (x.st >> 1);
		fill = (plt_sel<<4) + plt_sel;
		dvC341Geo_WriteToBuffer( B0_CPUWAD, ad, 0 );
		if ( bbacthw == 0 )
		{
			dvC341Geo_WriteToBuffer( B0_CPUDT, fill, 0 );//Data write
		}
		else
		{
			dvC341Geo_WriteToBuffer( B0_OSDFILL, fill, 0 );
			dvC341Geo_WriteToBuffer( B0_BBACTHW, bbacthw, 0 );
			dvC341Geo_WriteToBuffer( B0_BBACTVW, 0x00, 0 );
			dvC341Geo_WriteToBuffer( B0_OSDCT, 0x01, 0 );//FILL write
//			do { rdt = dvC341Geo_Read( B0_BOSTAT, 0 ) & 0x01; } while ( rdt != 0x00 );
			dvC341Geo_WriteToBuffer( B0_OSDCT, 0x00, 0 );
		}

		dvC341Geo_Buffer_Flush();
	}
//	dvC341Geo_Write( B0_CPUWAD, 0, 0 );//dummy
#endif
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
int dvC341Geo_LoadEgbBiasArea( char fn[] )
{
#if 0
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

	dvC341Geo_Write( B0_BBACTVW, 0x00, 0);

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
				dvC341Geo_Write( B0_CPUWAD, ad, 0 );
				if ( wbyte_rest == 1 )
				{
					dvC341Geo_Write( B0_CPUDT, dt_old, 0 );
				}
				else
				{
					dvC341Geo_Write( B0_OSDFILL, dt_old, 0 );
					dvC341Geo_Write( B0_BBACTHW, (wbyte_rest - 1), 0 );
//					dvC341Geo_Write( B0_BBACTVW, 0x00, 0 );
					dvC341Geo_Write( B0_OSDCT, 0x01, 0 );//FILL write
//					do { rdt = dvC341Geo_Read( B0_BOSTAT, 0 ) & 0x01; } while ( rdt != 0x00 );
					dvC341Geo_Write( B0_OSDCT, 0x00, 0 );
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

		dvC341Geo_Write( B0_CPUWAD, ad, 0);
		if ( wbyte_rest == 1 )
		{
			dvC341Geo_Write( B0_CPUDT, dt_old, 0 );
		}
		else
		{
			dvC341Geo_Write( B0_OSDFILL, dt_old, 0 );
			dvC341Geo_Write( B0_BBACTHW, (wbyte_rest - 1), 0 );
//			dvC341Geo_Write( B0_BBACTVW, 0x00, 0 );
			dvC341Geo_Write( B0_OSDCT, 0x01, 0 );//FILL write
//			do { rdt = dvC341Geo_Read( B0_BOSTAT, 0 ) & 0x01; } while ( rdt != 0x00 );
			dvC341Geo_Write( B0_OSDCT, 0x00, 0 );
		}

//		dvC341Geo_Write( B0_CPUWAD, 0, 0 );//dummy
	}

	if ( fpr_open == 1 )
	{
		fclose( fpr );
	}

    LOG_MSG(db_DV_GEO, "write End\n");

	return len;
#endif

    return 0;
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
void dvC341Geo_CalcEdgebldBiasGammaTable( int color, int plt, int allplt )
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
void dvC341Geo_EdgebldBiasGammaTable( int plt )
{
#if 0
	int lv;
	int ad;
	int acct;
	int rtct_org = (int)dvC341Geo_Read(BN_RTCT, 0);

	dvC341Geo_Write( BN_RTCT, 0, 0 );

	dvC341Geo_Write( B7_EGBIASAD, 0x00, 0 );

    for( int color = 0 ; color < 3 ; color++)  //rgb
    {
        acct = ( color == 0 ) ? 0x1 : ( color == 1 ) ? 0x3 : ( color == 2 ) ? 0x5 : 0x7;
        dvC341Geo_Write( B7_EGBIASCT, 0x0050 + (acct << 1), 0);

		//for ( plt = 0; plt < 16; plt++ )
		{
    		ad = ((plt < 16) && (plt >= 0)) ? (plt << 4) : 0;
    		dvC341Geo_Write( B7_EGBIASAD, ad & 0xff, 0 );

    		dvC341Geo_BurstWrite_FixedAdd( B7_EGBIASDT, 16 , PM_EBIAS_GMDT2[plt][color] );

			//for ( lv = 0; lv < 16; lv++ )
			//{
			//	dvC341Geo_Write( B7_EGBIASDT, PM_EBIAS_GMDT2[plt][color][lv] & 0xff, 0 );
			//}
		}
    }

	//dvC341Geo_EdgebldBiasEnable(TRUE);

	dvC341Geo_Write( BN_RTCT, 0x05, 0 );
#endif
}

void dvC341Geo_EdgebldBiasGammaTableClear( void )
{
#if 0
	int lv;
	int ad;
	int acct;
	int rtct_org = (int)dvC341Geo_Read(BN_RTCT, 0);

	memset(PM_EBIAS_GMDT2, 0, 16*3*16);

	dvC341Geo_Write( BN_RTCT, 0, 0 );

	dvC341Geo_Write( B7_EGBIASAD, 0x00, 0 );

    for( int color = 0 ; color < 3 ; color++)  //rgb
    {
        acct = ( color == 0 ) ? 0x1 : ( color == 1 ) ? 0x3 : ( color == 2 ) ? 0x5 : 0x7;
        dvC341Geo_Write( B7_EGBIASCT, 0x0050 + (acct << 1), 0);

		for ( int plt = 0; plt < 16; plt++ )
		{
    		ad = ((plt < 16) && (plt >= 0)) ? (plt << 4) : 0;
    		dvC341Geo_Write( B7_EGBIASAD, ad & 0xff, 0 );

    		dvC341Geo_BurstWrite_FixedAdd( B7_EGBIASDT, 16 , PM_EBIAS_GMDT2[plt][color] );
		}
    }

	//dvC341Geo_EdgebldBiasEnable(TRUE);

	dvC341Geo_Write( BN_RTCT, 0x05, 0 );
#endif
}


#if 0
//EgbBiasGammaTable
void dvC341Geo_EdgebldBiasGammaTable( int color, int plt, int allplt )
{
	int lv;
	int ad;
	int acct;
	int rtct_org = (int)dvC341Geo_Read(BN_RTCT, 0);

	dvC341Geo_Write( BN_RTCT, 0, 0 );

	acct = ( color == 0 ) ? 0x1 : ( color == 1 ) ? 0x3 : ( color == 2 ) ? 0x5 : 0x7;
	dvC341Geo_Write( B7_EGBIASCT, 0x0050 + (acct << 1), 0);

	if ( allplt == 0 )
	{
		ad = ((plt < 16) && (plt >= 0)) ? (plt << 4) : 0;

		dvC341Geo_Write( B7_EGBIASAD, ad & 0xff, 0 );

		if ( (color >= 0) && (color < 3) )
		{
			for ( lv = 0; lv < 16; lv++ )
			{
				dvC341Geo_Write( B7_EGBIASDT, PM_EBIAS_GMDT2[plt][color][lv] & 0xff, 0 );
			}
		}
		else
		{
			for ( lv = 0; lv < 16; lv++ )
			{
				dvC341Geo_Write( B7_EGBIASDT, PM_EBIAS_GMDT2[plt][0][lv] & 0xff, 0 );
			}
		}
	}
	else
	{
		dvC341Geo_Write( B7_EGBIASAD, 0x00, 0 );

		for ( plt = 0; plt < 16; plt++ )
		{
			if ( (color >= 0) && (color < 3) )
			{
				for ( lv = 0; lv < 16; lv++ )
				{
					dvC341Geo_Write( B7_EGBIASDT, PM_EBIAS_GMDT2[plt][color][lv] & 0xff, 0 );
				}
			}
			else
			{
				for ( lv = 0; lv < 16; lv++ )
				{
					dvC341Geo_Write( B7_EGBIASDT, PM_EBIAS_GMDT2[plt][0][lv] & 0xff, 0 );
				}
			}
		}
	}

	dvC341Geo_EdgebldBiasEnable(TRUE);

	dvC341Geo_Write( BN_RTCT, 0x105, 0 );

}
#endif


void dvC341Geo_Blacklevel_Palette_Set(INT8 cPaletteIdx, UINT8 cRed, UINT8 cGreen, UINT8 cBlue)
{
#if 0
    dvC341Geo_Write( B0_CPUWAD, 0, 0 );

    dvC341Geo_SetEdgebldBiasPalette(cPaletteIdx, (INT32)cRed, (INT32)cGreen, (INT32)cBlue);

    dvC341Geo_CalcEdgebldBiasGammaTable(3, cPaletteIdx, 0);

    dvC341Geo_EdgebldBiasGammaTable(cPaletteIdx);
#endif
}

void dvC341Geo_RTCT_Set(int data)
{
    dvC341Geo_Write( B0_RTCT6, data, 0 );
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



void dvC341Geo_Dummy(void)
{
#if 0
	//Cindy 20190604-161804
	//Fix width wrong issue
	//Add this to avoid blend/osd drawing not completed

	// dummy start
	//dvC341Geo_WriteBuffer(B0_CPUWAD, DEF_BLEND_ADDRESS + m_nVertResolution * DEF_OSDMWI *128, 0);
    dvC341Geo_Write(B0_CPUWAD, DEF_BLEND_ADDRESS + PS_WP_VW * DEF_OSDMWI *128, 0);
	for(int i=0; i<8; i++)
		dvC341Geo_Write(B0_CPUDT, 0xFF, 0);
	// dummy end
#endif
}

void dvC341Geo_SetOSDMode(bool bFront)
{
	dvC341Geo_OSDACTStartConfig(bFront);

	UINT16 OSDMODE = dvC341Geo_Read(B9_OSDMODE1CH1, 0);

	if(dvC341Geo_IsMagnifyOSD2X())
	{
	    if(!(OSDMODE & ((BIT14)|(BIT12)|(BIT11))))
        {
    	    OSDMODE |= (BIT14)|(BIT12)|(BIT11);
            dvC341_Write(B9_OSDMODE1CH1, OSDMODE, 0);
    	    dvC341_Write(B9_OSDMODE1CH1, OSDMODE, CH_BANK_OFFSET);
        }
	}
	else
	{
        if(OSDMODE & ((BIT12)|(BIT11)))
        {
            OSDMODE |= (BIT14)|(BIT1);
            OSDMODE &= ~(BIT12|BIT11);
            dvC341_Write(B9_OSDMODE1CH1, OSDMODE, 0);
            dvC341_Write(B9_OSDMODE1CH1, OSDMODE, CH_BANK_OFFSET);
        }
	}

#if 0
	if(bFront)
	{
	    OSDMODE &= ~(BIT3);
	    OSDMODE |= (BIT2);
    	dvC341_Write(B9_OSDMODE1CH1, OSDMODE, 0);
    	dvC341_Write(B9_OSDMODE1CH1, OSDMODE, CH_BANK_OFFSET);
    }
    else
    {
	    OSDMODE &= ~(BIT3|BIT2);
    	dvC341_Write(B9_OSDMODE1CH1, OSDMODE, 0);
    	dvC341_Write(B9_OSDMODE1CH1, OSDMODE, CH_BANK_OFFSET);
    }
#endif /* 0 */

}

void dvC341Geo_OSDACTStartConfig(bool bFront)
{
	int OSDACT_HST, OSDACT_VST;

	if (bFront)
	{
	    if(m_sDrvC341Info.ePanelTimingId == PANEL_3D_OUTPUT) //H30K_Doulas_0045
	    {
		    OSDACT_HST = (m_sDrvC341Info.sOutputTimingInfo.uiHStart / 2) - 4;
		    OSDACT_VST = m_sDrvC341Info.sOutputTimingInfo.uiVStart + 1;
        }
	    else
	    {
		    OSDACT_HST = (m_sDrvC341Info.sOutputTimingInfo.uiHStart / 2) - 4;
		    OSDACT_VST = m_sDrvC341Info.sOutputTimingInfo.uiVStart;
        }
	}
	else
	{
		if (PS_WP_VW == 3840)
		{
    		OSDACT_HST = (m_sDrvC341Info.sOutputTimingInfo.uiHStart / 2) - 4;
    		OSDACT_VST = m_sDrvC341Info.sOutputTimingInfo.uiVStart;
		}
		else
		{
    		OSDACT_HST = (m_sDrvC341Info.sOutputTimingInfo.uiHStart / 2) - 4;
    		OSDACT_VST = m_sDrvC341Info.sOutputTimingInfo.uiVStart + 1;
		}
	}

	dvC341_WriteToBuffer(B9_OSDACTHST1CH1, OSDACT_HST, 0);
	dvC341_WriteToBuffer(B9_OSDACTVST1CH1, OSDACT_VST, 0);
	dvC341_WriteToBuffer(B9_OSDACTHST1CH1, OSDACT_HST, CH_BANK_OFFSET);
	dvC341_WriteToBuffer(B9_OSDACTVST1CH1, OSDACT_VST, CH_BANK_OFFSET);
	dvC341Geo_Buffer_Flush();

}

UINT32 dvC341Geo_V_Start_Get(void)    //G100_Owen_0078
{
#if 0
    return dvC341Geo_Read(B10_IFACTVST, 0);
#endif
}

void dvC341Geo_ISYCT_Set(UINT8 uiRegValue)    //A35G2_CDS_Simon_0003
{
#if 0
    m_sDrvC341Info.ucRegISYCT = uiRegValue;
    dvC341Geo_Write(B3_ISYCT, (UINT32)m_sDrvC341Info.ucRegISYCT, 0);
#endif
}

UINT8 dvC341Geo_ISYCT_Get(void)    //A35G2_CDS_Simon_0003
{
    return m_sDrvC341Info.ucRegISYCT ;
}

static UINT8 m_acAdaptiveScaleFilterLut[] =
{
    0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0xff, 0xfd, 0xfb, 0xf9, 0xf8, 0xf9, 0xfc, 0x00, 0x06, 0x0f, 0x1a, 0x26, 0x32, 0x3a, 0x3e, 0x40, // cutoff=1.000, window=1.000
    0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0xfc, 0xfa, 0xf9, 0xf8, 0xfa, 0xfc, 0x00, 0x07, 0x10, 0x1a, 0x26, 0x32, 0x39, 0x3e, 0x40, // cutoff=0.988, window=0.988
    0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0xfe, 0xfc, 0xfa, 0xf9, 0xf9, 0xfa, 0xfd, 0x01, 0x07, 0x10, 0x1b, 0x26, 0x30, 0x39, 0x3e, 0x3e, // cutoff=0.975, window=0.975
    0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0xfe, 0xfc, 0xfa, 0xf9, 0xf9, 0xfa, 0xfd, 0x01, 0x08, 0x11, 0x1b, 0x26, 0x30, 0x38, 0x3d, 0x3e, // cutoff=0.962, window=0.962
    0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0xff, 0xfe, 0xfc, 0xfa, 0xf9, 0xf9, 0xfb, 0xfe, 0x02, 0x08, 0x11, 0x1b, 0x26, 0x30, 0x37, 0x3c, 0x3e, // cutoff=0.950, window=0.950
    0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0xfd, 0xfb, 0xfa, 0xf9, 0xf9, 0xfb, 0xfe, 0x02, 0x09, 0x12, 0x1b, 0x26, 0x30, 0x37, 0x3c, 0x3e, // cutoff=0.938, window=0.938
    0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0xfd, 0xfb, 0xfa, 0xf9, 0xf9, 0xfb, 0xff, 0x03, 0x0a, 0x12, 0x1c, 0x26, 0x2f, 0x37, 0x3a, 0x3c, // cutoff=0.925, window=0.925
    0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0xfe, 0xfd, 0xfb, 0xfa, 0xf9, 0xfa, 0xfc, 0xff, 0x03, 0x0a, 0x12, 0x1c, 0x26, 0x2e, 0x36, 0x3a, 0x3e, // cutoff=0.913, window=0.913
    0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0xfe, 0xfc, 0xfb, 0xf9, 0xf9, 0xfa, 0xfc, 0x00, 0x04, 0x0b, 0x13, 0x1c, 0x26, 0x2f, 0x35, 0x39, 0x3c, // cutoff=0.900, window=0.900
    0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0xfd, 0xfc, 0xfa, 0xf9, 0xf9, 0xfb, 0xfd, 0x00, 0x05, 0x0c, 0x14, 0x1c, 0x26, 0x2f, 0x34, 0x38, 0x3c, // cutoff=0.875, window=0.875
    0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0xfe, 0xfd, 0xfb, 0xfa, 0xfa, 0xfa, 0xfb, 0xfe, 0x01, 0x07, 0x0d, 0x15, 0x1d, 0x25, 0x2d, 0x32, 0x38, 0x38, // cutoff=0.850, window=0.850
    0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0xfe, 0xfc, 0xfb, 0xfa, 0xfa, 0xfb, 0xfc, 0xff, 0x02, 0x08, 0x0e, 0x15, 0x1d, 0x25, 0x2c, 0x32, 0x36, 0x38, // cutoff=0.825, window=0.825
    0x01, 0x01, 0x00, 0x00, 0xff, 0xfe, 0xfd, 0xfc, 0xfb, 0xfa, 0xfa, 0xfb, 0xfd, 0x00, 0x04, 0x09, 0x0f, 0x16, 0x1e, 0x25, 0x2c, 0x31, 0x34, 0x36, // cutoff=0.800, window=0.800
    0x01, 0x00, 0x00, 0x00, 0xff, 0xfd, 0xfc, 0xfb, 0xfb, 0xfa, 0xfb, 0xfc, 0xfe, 0x01, 0x05, 0x0a, 0x10, 0x17, 0x1e, 0x24, 0x2a, 0x31, 0x33, 0x36, // cutoff=0.775, window=0.775
    0x01, 0x00, 0x00, 0xff, 0xfe, 0xfd, 0xfc, 0xfb, 0xfb, 0xfa, 0xfb, 0xfd, 0xff, 0x02, 0x06, 0x0b, 0x11, 0x17, 0x1e, 0x24, 0x2a, 0x30, 0x31, 0x34, // cutoff=0.750, window=0.750
    0x00, 0x00, 0x00, 0xfe, 0xfd, 0xfc, 0xfb, 0xfb, 0xfb, 0xfb, 0xfc, 0xfe, 0x00, 0x03, 0x07, 0x0c, 0x12, 0x18, 0x1e, 0x24, 0x29, 0x2e, 0x31, 0x32, // cutoff=0.725, window=0.725
    0x00, 0x00, 0xff, 0xfe, 0xfd, 0xfc, 0xfb, 0xfb, 0xfb, 0xfc, 0xfd, 0xff, 0x01, 0x04, 0x09, 0x0d, 0x12, 0x18, 0x1e, 0x23, 0x28, 0x2c, 0x2f, 0x30, // cutoff=0.700, window=0.700
    0x00, 0xff, 0xfe, 0xfd, 0xfc, 0xfc, 0xfb, 0xfb, 0xfb, 0xfc, 0xfe, 0x00, 0x02, 0x06, 0x0a, 0x0e, 0x13, 0x19, 0x1e, 0x23, 0x28, 0x2a, 0x2d, 0x2e, // cutoff=0.675, window=0.675
    0xff, 0xfe, 0xfd, 0xfd, 0xfc, 0xfb, 0xfb, 0xfc, 0xfc, 0xfd, 0xff, 0x00, 0x03, 0x07, 0x0b, 0x0e, 0x14, 0x19, 0x1e, 0x23, 0x27, 0x2a, 0x2b, 0x2c, // cutoff=0.650, window=0.650
    0xff, 0xfe, 0xfd, 0xfc, 0xfc, 0xfb, 0xfc, 0xfc, 0xfd, 0xfe, 0x00, 0x02, 0x05, 0x08, 0x0c, 0x0f, 0x14, 0x19, 0x1e, 0x22, 0x24, 0x28, 0x28, 0x2a, // cutoff=0.625, window=0.625
    0xfe, 0xfd, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfd, 0xfe, 0xff, 0x00, 0x03, 0x06, 0x09, 0x0c, 0x0f, 0x14, 0x19, 0x1d, 0x21, 0x25, 0x26, 0x28, 0x28, // cutoff=0.600, window=0.600
    0xfd, 0xfd, 0xfc, 0xfc, 0xfc, 0xfc, 0xfd, 0xfe, 0xff, 0x00, 0x01, 0x04, 0x07, 0x0a, 0x0d, 0x10, 0x14, 0x18, 0x1c, 0x20, 0x24, 0x25, 0x26, 0x24, // cutoff=0.575, window=0.575
    0xfd, 0xfd, 0xfc, 0xfc, 0xfc, 0xfd, 0xfd, 0xff, 0x00, 0x01, 0x03, 0x05, 0x08, 0x0b, 0x0e, 0x10, 0x14, 0x18, 0x1b, 0x1f, 0x22, 0x22, 0x24, 0x22, // cutoff=0.550, window=0.550
    0xfd, 0xfc, 0xfc, 0xfc, 0xfd, 0xfd, 0xfe, 0xff, 0x00, 0x02, 0x04, 0x06, 0x09, 0x0b, 0x0e, 0x10, 0x14, 0x17, 0x1a, 0x1e, 0x20, 0x23, 0x23, 0x22, // cutoff=0.525, window=0.525
    0xfd, 0xfd, 0xfd, 0xfd, 0xfd, 0xfe, 0xff, 0x00, 0x01, 0x03, 0x05, 0x07, 0x0a, 0x0c, 0x0e, 0x10, 0x14, 0x17, 0x19, 0x1c, 0x1e, 0x1f, 0x21, 0x20, // cutoff=0.500, window=0.500
    0xfd, 0xfd, 0xfd, 0xfd, 0xfe, 0xff, 0x00, 0x00, 0x02, 0x03, 0x05, 0x08, 0x0a, 0x0c, 0x0f, 0x10, 0x13, 0x16, 0x18, 0x1b, 0x1e, 0x1f, 0x1f, 0x20, // cutoff=0.485, window=0.485
    0xfd, 0xfd, 0xfd, 0xfe, 0xfe, 0xff, 0x00, 0x01, 0x02, 0x04, 0x06, 0x08, 0x0a, 0x0c, 0x0f, 0x10, 0x13, 0x16, 0x18, 0x1a, 0x1d, 0x1e, 0x1f, 0x1e, // cutoff=0.471, window=0.471
    0xfd, 0xfd, 0xfe, 0xfe, 0xff, 0x00, 0x00, 0x01, 0x03, 0x05, 0x06, 0x08, 0x0a, 0x0d, 0x0f, 0x10, 0x13, 0x15, 0x17, 0x1a, 0x1c, 0x1c, 0x1e, 0x1e, // cutoff=0.457, window=0.457
    0xfd, 0xfe, 0xfe, 0xff, 0x00, 0x00, 0x01, 0x02, 0x03, 0x05, 0x07, 0x09, 0x0b, 0x0d, 0x0f, 0x10, 0x13, 0x15, 0x16, 0x18, 0x1a, 0x1b, 0x1d, 0x1c, // cutoff=0.444, window=0.444
    0xfe, 0xfe, 0xff, 0xff, 0x00, 0x00, 0x01, 0x02, 0x04, 0x06, 0x07, 0x09, 0x0b, 0x0d, 0x0f, 0x10, 0x12, 0x14, 0x16, 0x18, 0x19, 0x1b, 0x1c, 0x1c, // cutoff=0.432, window=0.432
    0xfe, 0xfe, 0xff, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x06, 0x08, 0x09, 0x0b, 0x0d, 0x0f, 0x10, 0x12, 0x14, 0x15, 0x17, 0x19, 0x1a, 0x1b, 0x1a, // cutoff=0.421, window=0.421
    0xfe, 0xff, 0xff, 0x00, 0x00, 0x01, 0x02, 0x03, 0x05, 0x06, 0x08, 0x09, 0x0b, 0x0d, 0x0e, 0x0f, 0x12, 0x13, 0x15, 0x17, 0x19, 0x1a, 0x1b, 0x1c, // cutoff=0.410, window=0.410
    0xff, 0xff, 0x00, 0x00, 0x00, 0x01, 0x03, 0x04, 0x05, 0x07, 0x08, 0x0a, 0x0b, 0x0d, 0x0e, 0x0f, 0x11, 0x13, 0x14, 0x16, 0x19, 0x19, 0x1a, 0x1a, // cutoff=0.400, window=0.400
    0xff, 0xff, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x07, 0x08, 0x0a, 0x0b, 0x0d, 0x0e, 0x0f, 0x11, 0x12, 0x14, 0x16, 0x18, 0x19, 0x1a, 0x1a, // cutoff=0.390, window=0.390
    0xff, 0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x06, 0x07, 0x08, 0x0a, 0x0b, 0x0d, 0x0e, 0x0f, 0x11, 0x12, 0x13, 0x16, 0x19, 0x18, 0x19, 0x1a, // cutoff=0.381, window=0.381
    0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x09, 0x0a, 0x0b, 0x0d, 0x0e, 0x0f, 0x11, 0x12, 0x13, 0x15, 0x17, 0x17, 0x17, 0x18, // cutoff=0.372, window=0.372
    0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x08, 0x09, 0x0a, 0x0b, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x15, 0x18, 0x17, 0x18, 0x18, // cutoff=0.364, window=0.364
    0x00, 0x00, 0x01, 0x01, 0x02, 0x03, 0x04, 0x05, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x15, 0x17, 0x17, 0x17, 0x18, // cutoff=0.356, window=0.356
    0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x14, 0x16, 0x16, 0x16, 0x16, // cutoff=0.348, window=0.348
    0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x11, 0x14, 0x17, 0x16, 0x16, 0x16, // cutoff=0.340, window=0.340
    0x00, 0x01, 0x02, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0e, 0x0f, 0x10, 0x10, 0x11, 0x14, 0x16, 0x17, 0x16, 0x16, // cutoff=0.333, window=0.333
    0x00, 0x01, 0x02, 0x03, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0f, 0x0f, 0x10, 0x11, 0x13, 0x16, 0x17, 0x18, 0x16, // cutoff=0.327, window=0.327
    0x01, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x13, 0x15, 0x16, 0x16, 0x16, // cutoff=0.320, window=0.320
    0x01, 0x02, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x10, 0x13, 0x16, 0x14, 0x15, 0x16, // cutoff=0.314, window=0.314
    0x01, 0x02, 0x03, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x0f, 0x10, 0x13, 0x14, 0x15, 0x15, 0x16, // cutoff=0.308, window=0.308
    0x01, 0x02, 0x03, 0x04, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x0f, 0x10, 0x12, 0x14, 0x15, 0x15, 0x16, // cutoff=0.302, window=0.302
    0x02, 0x02, 0x03, 0x04, 0x05, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0e, 0x0f, 0x10, 0x12, 0x13, 0x15, 0x15, 0x16, // cutoff=0.296, window=0.296
    0x02, 0x03, 0x03, 0x04, 0x05, 0x06, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0e, 0x0f, 0x0f, 0x12, 0x14, 0x13, 0x15, 0x16, // cutoff=0.291, window=0.291
    0x02, 0x03, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x08, 0x09, 0x0a, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0e, 0x0f, 0x0f, 0x12, 0x14, 0x13, 0x14, 0x14, // cutoff=0.286, window=0.286
    0x02, 0x03, 0x04, 0x04, 0x05, 0x06, 0x07, 0x08, 0x08, 0x09, 0x0a, 0x0b, 0x0b, 0x0c, 0x0d, 0x0e, 0x0e, 0x0f, 0x0f, 0x11, 0x13, 0x13, 0x14, 0x14, // cutoff=0.281, window=0.281
    0x03, 0x03, 0x04, 0x05, 0x05, 0x06, 0x07, 0x08, 0x08, 0x09, 0x0a, 0x0b, 0x0b, 0x0c, 0x0d, 0x0e, 0x0e, 0x0e, 0x0f, 0x10, 0x13, 0x14, 0x13, 0x14, // cutoff=0.276, window=0.276
    0x03, 0x03, 0x04, 0x05, 0x05, 0x06, 0x07, 0x08, 0x08, 0x09, 0x0a, 0x0b, 0x0b, 0x0c, 0x0d, 0x0e, 0x0e, 0x0e, 0x0f, 0x10, 0x13, 0x14, 0x13, 0x14, // cutoff=0.271, window=0.271
    0x03, 0x04, 0x04, 0x05, 0x06, 0x06, 0x07, 0x08, 0x08, 0x09, 0x0a, 0x0b, 0x0b, 0x0c, 0x0d, 0x0e, 0x0e, 0x0e, 0x0f, 0x10, 0x12, 0x13, 0x13, 0x14, // cutoff=0.267, window=0.267
    0x03, 0x04, 0x04, 0x05, 0x06, 0x06, 0x07, 0x08, 0x09, 0x09, 0x0a, 0x0b, 0x0b, 0x0c, 0x0d, 0x0e, 0x0e, 0x0e, 0x0f, 0x10, 0x12, 0x13, 0x12, 0x14, // cutoff=0.262, window=0.262
    0x03, 0x04, 0x05, 0x05, 0x06, 0x07, 0x07, 0x08, 0x09, 0x09, 0x0a, 0x0b, 0x0b, 0x0c, 0x0c, 0x0e, 0x0e, 0x0e, 0x0e, 0x10, 0x12, 0x12, 0x13, 0x14, // cutoff=0.258, window=0.258
    0x04, 0x04, 0x05, 0x05, 0x06, 0x07, 0x07, 0x09, 0x09, 0x09, 0x0a, 0x0b, 0x0b, 0x0c, 0x0c, 0x0e, 0x0d, 0x0e, 0x0e, 0x10, 0x12, 0x12, 0x13, 0x12, // cutoff=0.254, window=0.254
    0x04, 0x04, 0x05, 0x05, 0x06, 0x07, 0x07, 0x09, 0x09, 0x09, 0x0a, 0x0b, 0x0b, 0x0c, 0x0c, 0x0e, 0x0d, 0x0e, 0x0e, 0x10, 0x12, 0x12, 0x13, 0x12, // cutoff=0.250, window=0.250
    0x04, 0x04, 0x05, 0x06, 0x06, 0x07, 0x08, 0x09, 0x09, 0x09, 0x0a, 0x0b, 0x0b, 0x0c, 0x0c, 0x0e, 0x0d, 0x0e, 0x0e, 0x0f, 0x12, 0x12, 0x12, 0x12, // cutoff=0.246, window=0.246
    0x04, 0x05, 0x05, 0x06, 0x06, 0x07, 0x08, 0x09, 0x09, 0x09, 0x0a, 0x0b, 0x0b, 0x0c, 0x0c, 0x0e, 0x0d, 0x0e, 0x0e, 0x0f, 0x12, 0x11, 0x12, 0x12, // cutoff=0.242, window=0.242
    0x04, 0x05, 0x05, 0x06, 0x06, 0x07, 0x08, 0x09, 0x09, 0x09, 0x0a, 0x0b, 0x0b, 0x0c, 0x0c, 0x0e, 0x0d, 0x0d, 0x0e, 0x0f, 0x12, 0x12, 0x12, 0x12, // cutoff=0.239, window=0.239
    0x04, 0x05, 0x05, 0x06, 0x07, 0x07, 0x08, 0x09, 0x09, 0x09, 0x0a, 0x0b, 0x0b, 0x0c, 0x0c, 0x0e, 0x0d, 0x0d, 0x0e, 0x0f, 0x11, 0x12, 0x12, 0x12, // cutoff=0.235, window=0.235
    0x04, 0x05, 0x06, 0x06, 0x07, 0x07, 0x08, 0x09, 0x09, 0x09, 0x0a, 0x0b, 0x0b, 0x0c, 0x0c, 0x0e, 0x0d, 0x0d, 0x0e, 0x0f, 0x10, 0x12, 0x12, 0x12, // cutoff=0.232, window=0.232
    0x05, 0x05, 0x06, 0x06, 0x07, 0x07, 0x08, 0x09, 0x09, 0x09, 0x0a, 0x0b, 0x0b, 0x0c, 0x0c, 0x0e, 0x0d, 0x0d, 0x0e, 0x0f, 0x10, 0x12, 0x11, 0x12, // cutoff=0.229, window=0.229
    0x05, 0x05, 0x06, 0x06, 0x07, 0x07, 0x08, 0x09, 0x09, 0x0a, 0x0a, 0x0b, 0x0b, 0x0b, 0x0c, 0x0d, 0x0d, 0x0d, 0x0d, 0x0f, 0x11, 0x12, 0x11, 0x14, // cutoff=0.225, window=0.225
};

void dvC341Geo_AdaptiveScaleFilterLutInit(void) //A65_OPTOMA_CL_0018  //A35G2_Simon_0115
{
	INT32 dir;// 0 : Horizontal, 1 : Vertical
    INT32 GV_RTCT = dvC341Geo_Read(B0_RTCT6, 0);
//        GV_RTCT = GV_RTCT & 0xfc;// [1:0] RTG0 = Through

	dvC341Geo_Write(B0_RTCT6, 0, 0);

    for(eC341_CH eCH = eC341_CH1; eCH < OUTPUT_CH_NUM ; eCH++)
    {
    	for (dir = 0; dir < 2; dir++)
    	{
    		if (dir == 0) // Horizontal
    		{
    			dvC341Geo_Write(B146_WPSCLADPCTCH1, 0x04, eCH);
    		}
    		else // Vertical
    		{
    			dvC341Geo_Write(B146_WPSCLADPCTCH1, 0x08, eCH);
    		}
    		dvC341Geo_Write(B146_WPSCLADPADCH1, 0x00, eCH);

            if(eCH == eC341_CH1)
            {
                dvC341Geo_BurstWrite_FixedAdd(B146_WPSCLADPDTCH1, sizeof(m_acAdaptiveScaleFilterLut)/sizeof(UINT8), &m_acAdaptiveScaleFilterLut[0]);
            }
            else if(eCH == eC341_CH2)
            {
                dvC341Geo_BurstWrite_FixedAdd(B151_WPSCLADPDTCH2, sizeof(m_acAdaptiveScaleFilterLut)/sizeof(UINT8), &m_acAdaptiveScaleFilterLut[0]);
            }
    	}

        dvC341Geo_Write(B146_WPSCLADPCTCH1, 0x00, eCH);
    }

    dvC341Geo_Write(B0_RTCT6, GV_RTCT, 0);
}


void dvC341Geo_AdaptiveScaleFilterLutEnable(UINT8 ucEnable)  //A35G2_Simon_0115
{
    if(ucEnable)
    {
        for(eC341_CH eCH = eC341_CH1; eCH < OUTPUT_CH_NUM ; eCH++)
        {
            dvC341Geo_WriteToBuffer(B146_WPSCLADPCTCH1, 0x03, eCH);
        }

        LOG_MSG(db_HAL_WARPING, "Adaptive Warp Filter Lut On\n" );
    }
    else
    {
        for(eC341_CH eCH = eC341_CH1; eCH < OUTPUT_CH_NUM ; eCH++)
        {
            dvC341Geo_WriteToBuffer(B146_WPSCLADPCTCH1, 0x00, eCH);
        }
        LOG_MSG(db_HAL_WARPING, "Adaptive Warp Filter Lut Off\n" );
    }

    dvC341Geo_Buffer_Flush();
}


void dvC341Geo_Freeze(UINT8 ucEnable)
{
    if(ucEnable)
    {
        UINT32 WPISFLD = dvC341Geo_Read(B148_WPISFLD0CH1, 0);

        for(eC341_CH eCH = eC341_CH1; eCH < OUTPUT_CH_NUM ; eCH++)
        {
            dvC341Geo_WriteToBuffer(B145_WPIACTSELCH1, 0x03, eCH);   //H2PF_Simon_0096
            dvC341Geo_WriteToBuffer(B148_WPOSFLD0CH1, WPISFLD >> 15, eCH);
            dvC341Geo_WriteToBuffer(B148_WPOSFLD1CH1, WPISFLD >> 15, eCH);
            dvC341Geo_WriteToBuffer(B148_WPOSFLD2CH1, WPISFLD >> 15, eCH);
            dvC341Geo_WriteToBuffer(B148_WPOSFLD3CH1, WPISFLD >> 15, eCH);
        }

        dvC341Geo_Buffer_Flush();
    }
    else
    {
        for(eC341_CH eCH = eC341_CH1; eCH < OUTPUT_CH_NUM ; eCH++)   //H2PF_Simon_0131
        {
            dvC341Geo_WriteToBuffer(B145_WPIACTSELCH1, 0x01, eCH);   //H2PF_Simon_0096
            dvC341Geo_WriteToBuffer(B148_WPOSFLD0CH1, (DEF_WPOSFLD0CH1 >> 15), eCH);
            dvC341Geo_WriteToBuffer(B148_WPOSFLD1CH1, (DEF_WPOSFLD1CH1 >> 15), eCH);
            dvC341Geo_WriteToBuffer(B148_WPOSFLD2CH1, (DEF_WPOSFLD2CH1 >> 15), eCH);
            dvC341Geo_WriteToBuffer(B148_WPOSFLD3CH1, (DEF_WPOSFLD3CH1 >> 15), eCH);
        }

        dvC341Geo_Buffer_Flush();
    }
}

UINT32 dvC341Geo_FrameMemoryDrawStartAddrGet(void)
{
    UINT32 WPISFLD = dvC341Geo_Read(B148_WPISFLD0CH1, 0);
    return WPISFLD;
}

UINT8 dvC341Geo_IsMagnifyOSD2X(void)
{
    return m_sDrvC341Info.ucMagnifyOSD2X;
}

void dvC341Geo_EnableMagnifyOSD2X(UINT8 Enable2X)
{
    UINT16 OsdMode2 = dvC341_Read(B9_OSDMODE1CH1, 0);

    if(Enable2X)
    {
        m_sDrvC341Info.ucMagnifyOSD2X = TRUE;
        OsdMode2 |= (BIT11|BIT12);
    }
    else
    {
        m_sDrvC341Info.ucMagnifyOSD2X = FALSE;
        OsdMode2 &= ~(BIT11|BIT12);
    }

    //CH1
    dvC341_WriteToBuffer(B9_OSDMODE1CH1, OsdMode2, 0);
    dvC341_WriteToBuffer(B9_OSDACTHW1CH1, m_sDrvC341Info.sOutputTimingInfo.uiHActive/OUTPUT_CH_NUM, 0);
    dvC341_WriteToBuffer(B9_OSDACTVW1CH1, m_sDrvC341Info.sOutputTimingInfo.uiVActive, 0);
    //CH2
    dvC341_WriteToBuffer(B9_OSDMODE1CH1, OsdMode2, CH_BANK_OFFSET);
    dvC341_WriteToBuffer(B9_OSDACTHW1CH1, m_sDrvC341Info.sOutputTimingInfo.uiHActive/OUTPUT_CH_NUM, CH_BANK_OFFSET);
    dvC341_WriteToBuffer(B9_OSDACTVW1CH1, m_sDrvC341Info.sOutputTimingInfo.uiVActive, CH_BANK_OFFSET);

    dvC341Geo_Buffer_Flush();
}

//warping block frame memory
void dvc341Geo_FrameMemoryProc30Bits(UINT8 Enable30bit)   //H2PF_Simon_0131
{
    UINT32 ulWhw = 0, ad_ofst = 0;
    UINT16 dev16 = 0, mod16 = 0;

    UINT32 Wpisfld[4];
    Wpisfld[0] = DEF_WPISFLD0CH1;
    Wpisfld[1] = DEF_WPISFLD1CH1;
    Wpisfld[2] = DEF_WPISFLD2CH1;
    Wpisfld[3] = DEF_WPISFLD3CH1;

    for(eC341_CH eCH = eC341_CH1; eCH < OUTPUT_CH_NUM ; eCH++)
    {
        // ISFLD
        dev16 = ulWhw / 16;
        dev16 = dev16 * 16;
        mod16 = ulWhw - dev16;
        ad_ofst = dev16 * 16 + mod16 * 4;

        if(Enable30bit)
        {
            dvC341Geo_WriteToBuffer(B144_WPOSCWCTCH1, 0, eCH);
            dvC341Geo_WriteToBuffer(B144_WPISCWCTCH1, 0, eCH);
            dvC341Geo_WriteToBuffer(B148_WPISFLD0CH1, (Wpisfld[0] + ad_ofst), eCH);
            dvC341Geo_WriteToBuffer(B148_WPISFLD1CH1, (Wpisfld[1] + ad_ofst), eCH);
            dvC341Geo_WriteToBuffer(B148_WPISFLD2CH1, (Wpisfld[2] + ad_ofst), eCH);
            dvC341Geo_WriteToBuffer(B148_WPISFLD3CH1, (Wpisfld[3] + ad_ofst), eCH);
        }
        else
        {
            dvC341Geo_WriteToBuffer(B144_WPOSCWCTCH1, 0x80, eCH);
            dvC341Geo_WriteToBuffer(B144_WPISCWCTCH1, 0x80, eCH);
            dvC341Geo_WriteToBuffer(B148_WPISFLD0CH1, (Wpisfld[0] + (ad_ofst*24/30)), eCH);
            dvC341Geo_WriteToBuffer(B148_WPISFLD1CH1, (Wpisfld[1] + (ad_ofst*24/30)), eCH);
            dvC341Geo_WriteToBuffer(B148_WPISFLD2CH1, (Wpisfld[2] + (ad_ofst*24/30)), eCH);
            dvC341Geo_WriteToBuffer(B148_WPISFLD3CH1, (Wpisfld[3] + (ad_ofst*24/30)), eCH);
        }

        ulWhw = ulWhw + (m_sDrvC341Info.sOutputTimingInfo.uiHActive/2);
    }

    dvC341Geo_Buffer_Flush();
}


UINT8 dvc341Geo_IsFrameMemoryProc30Bits(void)   //H2PF_Simon_0131
{
    UINT8 WPBLKCT = (UINT8)dvC341Geo_Read(B144_WPBLKCTCH1, 0);
    if(WPBLKCT == 0x00)
    {
        return TRUE;
    }

    UINT8 WPOSCWCT = (UINT8)dvC341Geo_Read(B144_WPOSCWCTCH1, 0);
    return (WPOSCWCT == 0x00) ? TRUE : FALSE ;  //H2PF_Simon_0137
}

void dvc341Geo_IsDefaultWarpTable_Set(UINT8 IsDefault)   //H2PF_Simon_0131
{
    m_sDrvC341Info.ucIsDefaultWarpTable = IsDefault;
}


void dvc341Geo_BypassModeSet(UINT8 BypassWarpingBlock)   //H2PF_Simon_0150
{
    if(BypassWarpingBlock == TRUE)
    {
        dvC341Geo_Write(B144_WPBLKCTCH1, 0x00, 0);
        dvC341Geo_Write(B144_WPBLKCTCH1, 0x00, CH_WPBANK_OFFSET);
    }
    else
    {
        dvC341Geo_Write(B144_WPBLKCTCH1, 0x03, 0);
        dvC341Geo_Write(B144_WPBLKCTCH1, 0x03, CH_WPBANK_OFFSET);
    }
}



