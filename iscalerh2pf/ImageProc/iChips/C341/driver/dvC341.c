#include <math.h>

#include "dvC341.h"
#include "utilDbgMsg.h"
#include "utilCounterAPI.h"
//#include "utilMathAPI.h"
#include "Ichipc341_Reg.c"
#include "opdCtrlAPI.h"

//#define ENABLE_IT6805_YUV422_YUV420_CONVERT_TO_YUV444

#define SPI_BUFFER_SIZE             (4 * 1024)

static UINT8 aucWriteBuffer[SPI_BUFFER_SIZE + 0x100] = {0};
static UINT16 uiWriteIdx = 0;


#define GlobalVar
GlobalVar int PS_VBO_2LANE[4]; // VBO input mode for 2Lane
GlobalVar int PS_VBO_4LANE[4]; // VBO input mode for 4Lane
GlobalVar int PS_VBO_8LANE[4]; // VBO input mode for 8Lane

//General constants
#define MAX_BUF_LEN 	256
#define SEP				", \r\n\t"
#define MAX_VBO_INF		24
#define MAX_IMODE_TBL	50			//input mode table constants
#define FRANGE_DIV		50000000.0F

//#define FRAME_SEQUENTAIL_SELF_TEST_120HZ         //H30K_Doulas_0005
#define FRAME_SEQUENTAIL_SELF_TEST_60HZ          //H30K_Doulas_0005

UINT8 HDR_DEMO_SPLIT = 0;    //H2PF_Simon_0193

GlobalVar VBO_Info g_vboinfo[MAX_VBO_INF];  //init by g_vboinfo_inittable
GlobalVar int g_vbo_recno[4];
//GlobalVar int g_pi_lanenum[4];
GlobalVar int g_port[2];  //simon: main window , sub window (g_port[ch_v] for input port)
                          //(main window==VBO0 => g_port[0]==2)
                          //(sub  window==VBO1 => g_port[1]==3)
GlobalVar int g_scan_mode[4];
GlobalVar int g_pict_mode[4];
GlobalVar int g_chinfo_chg[2];

//GlobalVar PORT_Info PI_PORT_INFO[4];

//GlobalVar IMODE_TBLdef g_imode[MAX_IMODE_TBL];
GlobalVar int g_imode_p;

//GlobalVar int g_checkpi_cnt[4];
//GlobalVar int g_pi_kind[4];

GlobalVar int g_oscl_use[2][2];

//GlobalVar int g_oimgct[2][2];


GlobalVar int g_freeze_isel[2][5];
GlobalVar int g_freeze_stat[2];
GlobalVar int g_rtct;
GlobalVar int g_vshrnk_flg[2];
GlobalVar int g_vzoom_flg[2];

GlobalVar int g_B16_IACTVST[2]; //H30K_Doulas_0020
GlobalVar int g_B16_IACTVW[2];  //H30K_Doulas_0020
GlobalVar int g_B8_OACTVST[2];  //H30K_Doulas_0020
GlobalVar int g_B8_OACTVW[2];   //H30K_Doulas_0020


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
#if 1   //H30K_Doulas_0001//A70LV_Doulas_0377 Modify
        {
            120,                        //FrameRate
            60000,//30825,              //PixelClock    154128000x2
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

//V by one table
VBO_Info g_vboinfo_inittable[MAX_VBO_INF] =
{
	//VICT   VIDSKW  IDUAL SEL1  SEL2  SEL3  SEL4 LANE CH_LANE CH        recno
	//for 2k
	{0x0000, 0x0000, 0x00,   -1,   -1,   -1,   -1,   1,      0, 0, 0}, // 0  dummy
	{0x0000, 0x0000, 0x00, 0x08,   -1,   -1,   -1,   1,      1, 1, 0}, // 1  straight
	{0x0001, 0x0010, 0x00, 0x08,   -1,   -1,   -1,   2,      2, 1, 0}, // 2  2parax1
	{0x0004, 0x0004, 0x00, 0x08,   -1,   -1,   -1,   4,      4, 1, 0}, // 3  4parax1 2k120
	{0x0404, 0x0002, 0x02, 0x08, 0x0a,   -1,   -1,   8,      4, 2, 0}, // 4  8parax1 2k240
	{0x0000, 0x0000, 0x00,   -1,   -1,   -1,   -1,   1,      0, 0, 0}, // 5  reserved
	{0x0000, 0x0000, 0x00,   -1,   -1,   -1,   -1,   1,      0, 0, 0}, // 6  reserved
	{0x0000, 0x0000, 0x00,   -1,   -1,   -1,   -1,   1,      0, 0, 0}, // 7  reserved
	{0x0000, 0x0000, 0x00,   -1,   -1,   -1,   -1,   1,      0, 0, 0}, // 8  reserved
	{0x0000, 0x0000, 0x00,   -1,   -1,   -1,   -1,   1,      0, 0, 0}, // 9  reserved

	//for 4k
	{0x0404, 0x0002, 0x02, 0x08, 0x0a,   -1,   -1,   8,      4, 2, 0}, // 10 8parax1
	{0x0404, 0x0002, 0x03, 0x08, 0x0a,   -1,   -1,   8,      4, 2, 0}, // 11 4parax2
	{0x0404, 0x0002, 0x08, 0x08, 0x0a,   -1,   -1,   8,      4, 2, 0}, // 12 2parax4 strpe
	{0x1111, 0x0002, 0x09, 0x08, 0x09, 0x0a, 0x0b,   8,      2, 4, 1}, // 13 2parax4 quad
	{0x0011, 0x0004, 0x01, 0x08, 0x09,   -1,   -1,   4,      2, 2, 0}, // 14 4parax1
	{0x0011, 0x0004, 0x03, 0x08, 0x09,   -1,   -1,   4,      2, 2, 0}, // 15 2parax2
	{0x0404, 0x0001, 0x0a, 0x08, 0x0a, 0x0c, 0x0e,  16,      4, 4, 0}, // 16 4parax4 4k120
	{0x0000, 0x0000, 0x00,   -1,   -1,   -1,   -1,   1,      0, 0, 0}, // 17 reserved
	{0x0000, 0x0000, 0x00,   -1,   -1,   -1,   -1,   1,      0, 0, 0}, // 18 reserved
	{0x0000, 0x0000, 0x00,   -1,   -1,   -1,   -1,   1,      0, 0, 0}, // 19 reserved
	{0x0000, 0x0000, 0x00,   -1,   -1,   -1,   -1,   1,      0, 0, 0}, // 20 reserved
	{0x0000, 0x0000, 0x00,   -1,   -1,   -1,   -1,   1,      0, 0, 0}, // 21 reserved
	{0x0000, 0x0000, 0x00,   -1,   -1,   -1,   -1,   1,      0, 0, 0}, // 22 reserved
	{0x0000, 0x0000, 0x00,   -1,   -1,   -1,   -1,   1,      0, 0, 0}, // 23 reserved
};

static const sMODE_TABLE m_sModetableTableWide[] =   //A70LV_Doulas_0123 //A70LV_Doulas_0112
{
    {0    ,{26968 , 60.0  ,  2750,   1125,   1920,   1080,    185,     42,  0}},
    {1    ,{26997 , 60.0  ,  2750,   1125,   1920,   1080,    185,     42,  0}},
    {2    ,{28121 , 60.0  ,  2640,   1125,   1920,   1080,    185,     42,  0}},
    {3    ,{33738 , 60.0  ,  2200,   1125,   1920,   1080,    185,     42,  0}},
    {4    ,{26975 , 60.0  ,  2750,   1125,   1920,   1080,    185,     42,  0}},
    {5    ,{27005 , 60.0  ,  2750,   1125,   1920,   1080,    185,     42,  0}},
    {6    ,{28121 , 60.0  ,  2640,    562,   1920,    540,    189,     21,  0}},        //A70LV_Doulas_0244 modify
    {7    ,{33738 , 60.0  ,  2200,    562,   1920,    540,    189,     21,  0}},        //A70LV_Doulas_0244 modify
    {8    ,{56242 , 60.0  ,  2640,   1125,   1920,   1080,    184,     42,  0}},
    {9    ,{67530 , 60.0  ,  2200,   1125,   1920,   1080,    185,     42,  0}},
    {10   ,{67430 , 60.0  ,  2200,   1123,   1920,   1080,    190,     41,  0}},
    {11   ,{67069 , 60.0  ,  2368,   1125,   1920,   1080,    412,     42,  0}},
    {12   ,{67888 , 60.0  ,  1552,    902,   1152,    864,    323,     37,  0}},
        //{13   ,{47824 , 60.0  ,  1664,    798,   1280,    768,    321,     28,  0}},
    {13   ,{47824 , 60.0  ,  1328,    798,   1024,    768,    247,     28,  0}},    //CVT1060 881
    {14   ,{47393 , 60.0  ,  1440,    790,   1280,    768,    105,     20,  0}},    //CVR1260E 881
    {15   ,{60132 , 60.0  ,  1712,    802,   1280,    768,    353,     33,  0}},
    {16   ,{60240 , 60.0  ,  1696,    805,   1280,    768,    334,     34,  0}},
    {17   ,{68587 , 60.0  ,  1728,    807,   1280,    768,    360,     38,  0}},
    {18   ,{68634 , 85.0  ,  1712,    809,   1280,    768,    343,     39,  0}},
    {19   ,{59594 , 60.0  ,  1712,    994,   1280,    960,    347,     33,  0}},
    {20   ,{75528 , 60.0  ,  1728,   1002,   1280,    960,    355,     41,  0}},
    {21   ,{47740 , 60.0  ,  1792,    795,   1360,    768,    362,     25,  0}},                //A70LV_Doulas_0257
    //{22   ,{47709 , 60.0  ,  1776,    798,   1360,    768,    335,     28,  0}},
    {22   ,{47740 , 60.0  ,  1792,    798,   1366,    768,    350,     28,  0}},    //T1376V60  //A70LV_Doulas_0257
    {23   ,{67069 , 60.0  ,  2576,   1118,   1921,   1080,    190,     32,  0}},
    {24   ,{67521 , 60.0  ,  2200,   1115,   1920,   1080,    184,     32,  0}},
    {25   ,{15730 , 60.0  ,   858,    525,    720,    480,    125,     38,  0}},
    //{26   ,{31466 , 60.0  ,   800,    525,    640,    400,    136,     76,  0}},    //SMT0660D 881
    {26   ,{31466 , 60.0  ,   800,    525,    640,    480,    136,     36,  0}},    //SMT0660  881
    {27   ,{15628 , 50.0  ,   864,    312,    720,    288,    128,     23,  0}},    //576i      //A70LV_Doulas_0244 modify
    {28   ,{90009 , 60.0  ,  1650,    750,   1280,    720,    252,     26,  0}},
    {29   ,{37495 , 50.0  ,  1980,    750,   1280,    720,    253,     26,  0}},
    {30   ,{44964 , 60.0  ,  1650,    750,   1280,    720,    253,     26,  0}},
    {31   ,{45004 , 60.0  ,  1650,    750,   1280,    720,    253,     26,  0}},
    {32   ,{44464 , 60.0  ,  1664,    748,   1280,    720,    318,     25,  0}},
    {33   ,{55865 , 60.0  ,  1696,    755,   1280,    720,    334,     32,  0}},
    {34   ,{49726 , 75.0  ,  1152,    667,    832,    624,    280,     43,  0}},    //802BT 8362A75
    {35   ,{31026 , 60.0  ,  1088,    517,    848,    480,    217,     32,  0}},
    {36   ,{29629 , 60.0  ,   928,    495,    768,    480,     95,     10,  0}},
    {37   ,{36968 , 60.0  ,   960,    618,    800,    600,    105,     16,  0}},    //CVR0860 881
    {38   ,{29515 , 60.0  ,  1008,    494,    848,    480,     95,     10,  0}},    //CVR0860H 881
    {39   ,{37050 , 60.0  ,  1120,    618,    960,    600,     95,     10,  0}},
            //{40   ,{47303 , 60.0  ,  1120,    790,    800,    600,     95,     10,  0}},
    {40   ,{47303 , 60.0  ,  1184,    790,   1024,    768,    105,     20,  0}},    //CVR1060 881
    {41   ,{36968 , 60.0  ,  1224,    618,   1064,    600,     95,     10,  0}},
    {42   ,{53163 , 60.0  ,  1312,    889,   1152,    864,    105,     23,  0}},    //CVR1160 881
    //{43   ,{44404 , 60.0  ,  1470,    741,   1152,    720,     95,     10,  0}},
    {43   ,{44404 , 60.0  ,  1440,    741,   1280,    720,    105,     19,  0}},    //CVR1260H 881
    {44   ,{59241 , 60.0  ,  1440,    988,   1280,    960,    105,     26,  0}},    //CVR1260 881
    {45   ,{47326 , 60.0  ,  1470,    790,   1280,    768,     95,     10,  0}},
    {46   ,{39635 , 50.0  ,  1648,    793,   1280,    768,    303,     23,  0}},    //CVT1250E 881
    {47   ,{39588 , 60.0  ,  1680,    792,   1280,    768,    334,     10,  0}},
    {48   ,{75187 , 60.0  ,  2064,   1005,   1536,    960,     95,     10,  0}},
    {49   ,{49504 , 60.0  ,  2232,    991,   1704,    960,     95,     10,  0}},
    {50   ,{67114 , 60.0  ,  2320,   1120,   1728,   1080,     95,     10,  0}},
    {51   ,{59737 , 60.0  ,  2264,    997,   1704,    960,     95,     10,  0}},
    {52   ,{24479 , 60.0  ,   640,    407,    512,    384,    112,     22,  0}},
    {53   ,{34989 , 60.0  ,   864,    525,    640,    480,    174,     42,  0}},
    {54   ,{68870 , 60.0  ,   832,    918,    640,    870,    160,     45,  0}},
    {55   ,{49115 , 60.0  ,  1120,    654,    832,    624,    248,     28,  0}},        //APP0875
    {56   ,{48192 , 60.0  ,  1328,    813,   1024,    768,    224,     34,  0}},
    {57   ,{60240 , 60.0  ,  1328,    804,   1024,    768,    272,     33,  0}},
    {58   ,{68681 , 60.0  ,  1456,    915,   1152,    870,    272,     42,  0}},
    {59   ,{31250 , 50.0  ,   864,    625,    720,    576,    124,     45,  0}},
    {60   ,{64599 , 60.0  ,  1688,   1078,   1280,   1024,    300,     14,  0}},
    {61   ,{70671 , 60.0  ,  1696,   1063,   1280,   1024,    369,     38,  0}},
    {62   ,{63734 , 60.0  ,  1696,   1062,   1280,   1024,    300,     35,  0}},
    {63   ,{78125 , 60.0  ,  1728,   1091,   1280,   1024,    334,     36,  0}},
    {64   ,{47393 , 60.0  ,  1438,    790,   1280,    768,    110,     19,  0}},
    {65   ,{64641 , 60.0  ,  1520,   1080,   1400,   1050,    110,     27,  0}},
    {66   ,{38669 , 60.0  ,  1344,    806,   1024,    768,    295,     35,  0}},
    {67   ,{51150 , 60.0  ,  1696,   1066,   1280,   1024,    367,     41,  0}},
    {68   ,{63291 , 60.0  ,  1440,   1054,   1280,   1024,    105,     28,  0}},    //CVR1260G
    {69   ,{75930 , 60.0  ,  1696,   1056,   1280,   1024,    367,     31,  0}},
    {70   ,{35997 , 60.0  ,  1980,    750,   1280,    720,    259,     25,  0}},
    {71   ,{38387 , 60.0  ,  1544,    800,   1280,    768,    223,     29,  0}},
    {72   ,{40000 , 60.0  ,  1544,    800,   1280,    768,    219,     31,  0}},
    {73   ,{47961 , 60.0  ,  1540,    800,   1280,    768,    219,     29,  0}},
    {74   ,{50000 , 60.0  ,  1800,   1000,   1280,    960,    423,     39,  0}},
    {75   ,{40000 , 60.0  ,  1620,    800,   1366,    768,    219,     31,  0}},
    {76   ,{47961 , 60.0  ,  1620,    800,   1366,    768,    219,     29,  0}},
    {77   ,{51948 , 60.0  ,  1832,   1082,   1400,   1050,    359,     29,  0}},
    {78   ,{54495 , 60.0  ,  1736,   1090,   1400,   1050,    299,     37,  0}},
    {79   ,{65445 , 60.0  ,  1736,   1092,   1400,   1050,    299,     39,  0}},
    {80   ,{65659 , 60.0  ,  1856,   1094,   1400,   1050,    397,     43,  0}},
    {81   ,{80971 , 60.0  ,  1750,   1080,   1400,   1050,    313,     29,  0}},
    {82   ,{59952 , 60.0  ,  2160,   1250,   1600,   1200,    495,     49,  0}},
    {83   ,{62500 , 60.0  ,  2160,   1250,   1600,   1200,    495,     49,  0}},
    {84   ,{53995 , 60.0  ,  2640,   1125,   1920,   1080,    191,     41,  0}},
    {85   ,{33568 , 60.0  ,  2328,    560,   2048,    512,    192,     44,  0}},
    {86   ,{37439 , 60.0  ,   832,    520,    640,    480,    175,     39,  0}},
    {87   ,{31250 , 60.0  ,  1024,    625,    800,    600,    199,     24,  0}},
    {88   ,{37636 , 60.0  ,  1056,    628,    800,    600,    215,     27,  0}},
    {89   ,{52493 , 60.0  ,  1320,    875,   1079,    809,    205,     63,  0}},
    {90   ,{15750 , 60.0  ,   858,    525,    720,    480,    117,     35,  0}},
    {91   ,{29832 , 60.0  ,  1056,    497,    848,    480,    208,     16,  0}},
    {92   ,{24715 , 60.0  ,  1040,    495,    848,    480,    192,     14,  0}},
    {93   ,{57339 , 60.0  ,  2608,   1147,   1920,   1080,    159,     63,  0}},
    {94   ,{48709 , 60.0  ,  1338,    806,   1024,    768,    294,     38,  0}},
    {95   ,{66577 , 60.0  ,  2080,   1111,   1920,   1080,    105,     29,  0}},    //CVR1960H
    {96   ,{61425 , 60.0  ,  2080,   1229,   1920,   1200,    109,     26,  0}},
    {97   ,{60975 , 60.0  ,  2080,   1229,   1920,   1200,    109,     26,  0}},
    {98   ,{61881 , 60.0  ,  2080,   1229,   1920,   1200,    109,     26,  0}},
    {99   ,{37565 , 60.0  ,   832,    520,    640,    480,    157,     22,  0}},
    {100  ,{48007 , 60.0  ,  1500,    800,   1366,    768,    113,     32,  0}},    //DMR1360H 881
    {101  ,{62500 , 60.0  ,  2160,   1301,   1600,   1200,    512,    100,  0}},
    {102  ,{81234 , 60.0  ,  2160,   1250,   1600,   1200,    496,     49,  0}},
    {103  ,{87489 , 60.0  ,  2160,   1250,   1600,   1200,    496,     49,  0}},
    {104  ,{93720 , 60.0  ,  2160,   1250,   1600,   1200,    496,     49,  0}},
    {105  ,{100000, 60.0  ,  2160,   1250,   1600,   1200,    496,     49,  0}},
    {106  ,{106269, 60.0  ,  2160,   1250,   1600,   1200,    496,     49,  0}},
    {107  ,{83682 , 60.0  ,  2448,   1394,   1792,   1344,    528,     49,  0}},
    {108  ,{106269, 60.0  ,  2456,   1417,   1792,   1344,    568,     72,  0}},
    {109  ,{86355 , 60.0  ,  2528,   1439,   1856,   1392,    576,     46,  0}},
    {110  ,{90009 , 60.0  ,  2600,   1500,   1920,   1440,    552,     59,  0}},
    {111  ,{112485, 60.0  ,  2640,   1500,   1920,   1440,    576,     59,  0}},
    {112  ,{95785 , 60.0  ,  2504,   1597,   2048,   1536,    408,     60,  0}},
    {113  ,{120481, 60.0  ,  2656,   1606,   2048,   1536,    544,     69,  0}},
    {114  ,{66979 , 60.0  ,  2200,   1125,   1920,   1080,    190,     41,  0}},
    {115  ,{15733 , 60.0  ,   780,    525,    640,    480,    122,     41,  0}},
    {116  ,{62972 , 60.0  ,  1344,    840,   1024,    768,    256,     68,  0}},
    {117  ,{60240 , 60.0  ,  1328,    803,   1024,    768,    288,     29,  0}},
    {118  ,{78125 , 60.0  ,  1728,   1085,   1280,   1024,    382,     57,  0}},
    {119  ,{30293 , 60.0  ,   826,    508,    640,    480,    183,     24,  0}},
    {120  ,{39385 , 60.0  ,   800,    525,    640,    480,    144,     34,  0}},
    {121  ,{31466 , 70.0  ,   900,    449,    720,    400,    154,     38,  0}},    //7240A70
    {122  ,{35423 , 60.0  ,  1286,    815,   1053,    754,    219,     61,  0}},
    {123  ,{35612 , 60.0  ,  1280,    819,   1056,    768,    217,     51,  0}},
    {124  ,{56274 , 60.0  ,  1368,    804,   1024,    768,    326,     36,  0}},
    {125  ,{61087 , 60.0  ,  1408,    806,   1024,    768,    376,     38,  0}},
    {126  ,{70771 , 60.0  ,  1696,   1056,   1280,   1024,    384,     29,  0}},
    {127  ,{56497 , 60.0  ,  1824,   1097,   1360,   1024,    440,     73,  0}},
    {128  ,{37355 , 60.0  ,  1024,    624,    800,    600,    184,     22,  0}},    //CVT0860 881
    {129  ,{74019 , 60.0  ,  2080,   1235,   1920,   1200,    105,     33,  0}},    //CVR1960D 881
    {130  ,{31250 , 60.0  ,   800,    449,    640,    400,    142,     32,  0}},
    {131  ,{29949 , 60.0  ,   960,    500,    768,    480,    160,     18,  0}},    //CVT0760D 881
    {132  ,{37650 , 60.0  ,   976,    504,    768,    480,    168,     22,  0}},    //CVT0775D 881
    {133  ,{42903 , 85.0  ,   816,    507,    640,    480,    144,     25,  0}},    //CVT0685 881
    {134  ,{30998 , 50.0  ,   992,    621,    800,    600,    160,     19,  0}},    //CVT0850 881
    {135  ,{24691 , 50.0  ,   800,    497,    640,    480,    136,     15,  0}},    //CVT0650 881
    {136  ,{29824 , 60.0  ,  1056,    500,    848,    480,    176,     18,  0}},    //CVT0860H 881
    {137  ,{47103 , 60.0  ,  1040,    629,    800,    600,    192,     27,  0}},    //CVT0875 881
    {138  ,{37692 , 75.0  ,   816,    504,    640,    480,    144,     22,  0}},    //CVT0675 881
    {139  ,{53763 , 60.0  ,  1056,    633,    800,    600,    199,     31,  0}},    //CVT0885 881
    {140  ,{42973 , 60.0  ,  1088,    507,    848,    480,    198,     24,  0}},
    {141  ,{30835 , 60.0  ,  1200,    621,    960,    600,    214,     18,  0}},
    {142  ,{37216 , 60.0  ,  1216,    624,    960,    600,    216,     22,  0}},    //CVT0960D 881
    {143  ,{36656 , 60.0  ,  1296,    599,   1024,    576,    230,     20,  0}},
    {144  ,{53792 , 60.0  ,  1520,    897,   1152,    864,    295,     31,  0}},
    {145  ,{49407 , 50.0  ,  1680,    991,   1280,    960,    319,     29,  0}},    //CVT1250 881
    {146  ,{41254 , 60.0  ,  1648,    826,   1280,    800,    303,     24,  0}},    //CVT1250_ 881
    {147  ,{52687 , 60.0  ,  1680,   1057,   1280,   1024,    318,     31,  0}},    //CVT1250G 881
    {148  ,{59701 , 60.0  ,  1696,    996,   1280,    960,    326,     34,  0}},    //CVT1260 881
    {149  ,{47778 , 60.0  ,  1664,    798,   1280,    768,    311,     28,  0}},
    {150  ,{63653 , 60.0  ,  1712,   1063,   1280,   1024,    342,     37,  0}},    //CVT1260G 881
    {151  ,{44762 , 60.0  ,  1664,    748,   1280,    720,    311,     26,  0}},
    {152  ,{75244 , 60.0  ,  1728,   1005,   1280,    960,    349,     43,  0}},    //CVT1275G 881
    {153  ,{62774 , 75.0  ,  1696,    838,   1280,    800,    327,     36,  0}},
    {154  ,{60277 , 75.0  ,  1696,    805,   1280,    768,    327,     35,  0}},
    {155  ,{80321 , 60.0  ,  1728,   1072,   1280,   1024,    349,     46,  0}},    //CVT1275G 881
    {156  ,{56490 , 75.0  ,  1696,    755,   1280,    720,    406,     33,  0}},    //CVT1275H 802   //A70LV_Doulas_0232 modify
    {157  ,{85836 , 60.0  ,  1728,   1011,   1280,    960,    349,     49,  0}},    //CVT1285G 881
    {158  ,{71530 , 85.0  ,  1712,    843,   1280,    800,    343,     41,  0}},
    {159  ,{91491 , 85.0  ,  1744,   1078,   1280,   1024,    356,     52,  0}},    //CVT1285G 881
    {160  ,{64391 , 85.0  ,  1712,    759,   1280,    720,    343,     37,  0}},
    {161  ,{39556 , 60.0  ,  1744,    793,   1360,    768,    319,     23,  0}},
    {162  ,{54112 , 50.0  ,  1848,   1083,   1400,   1050,    357,     31,  0}},    //CVT1450 881    //A70LV_Doulas_0257
    {163  ,{25000 , 60.0  ,  1864,   1089,   1400,   1050,    369,     36,  0}},
    {164  ,{55928 , 60.0  ,  1904,    934,   1440,    900,    382,     31,  0}},
    {165  ,{82328 , 75.0  ,  1896,   1099,   1400,   1050,    382,     47,  0}},
    {166  ,{70621 , 75.0  ,  1936,    942,   1440,    900,    389,     40,  0}},    //CVT1475D 881
    {167  ,{49480 , 60.0  ,  2016,    991,   1536,    960,    398,     28,  0}},
    {168  ,{59665 , 60.0  ,  2032,    996,   1536,    960,    406,     33,  0}},
    {169  ,{74515 , 60.0  ,  2160,   1245,   1600,   1200,    436,     43,  0}},    //CVT1660
    //{170  ,{65324 , 60.0  ,  2240,   1089,   1680,   1050,    446,     37,  0}},        //CVT1660D 802BT
    {170  ,{65324 , 60.0  ,  1864,   1089,   1400,   1050,    367,     37,  0}},      //CVT1460  802BT
    {171  ,{61804 , 60.0  ,  2560,   1238,   1920,   1200,    508,     36,  0}},    //CVT1950D
    {172  ,{55617 , 50.0  ,  2544,   1114,   1920,   1080,    501,     32,  0}},    //CVT1950H
    {173  ,{49309 , 60.0  ,  1440,    823,   1280,    800,    105,     21,  0}},
    //{174  ,{31496 , 60.0  ,   858,    525,    720,    480,    114,     37,  0}},    //480P60
    {174  ,{31496 , 60.0  ,   800,    525,    640,    480,    137,     36,  0}},    //dmt0660 802
    {175  ,{37864 , 72.0  ,   832,    520,    640,    480,    160,     32,  0}},
    {176  ,{37495 , 75.0  ,   840,    500,    640,    480,    176,     20,  0}},
    {177  ,{43271 , 85.0  ,   832,    509,    640,    480,    128,     29,  0}},
    //{178  ,{37864 , 85.0  ,   832,    445,    640,    350,    153,     64,  0}},    //DMT0685F 881
    {178  ,{37864 , 85.0  ,   832,    445,    640,    400,    120,     45,  0}},    //DMT0685D 881
    {179  ,{37864 , 60.0  ,   832,    445,    640,    350,    158,     63,  0}},
    {180  ,{37936 , 85.0  ,   936,    446,    720,    400,    172,     46,  0}},
    {181  ,{35161 , 56.0  ,  1024,    625,    800,    600,    193,     25,  0}},
    {182  ,{37878 , 60.0  ,  1056,    628,    800,    600,    209,     28,  0}},
    {183  ,{48076 , 72.0  ,  1040,    666,    800,    600,    177,     30,  0}},
    {184  ,{46882 , 75.0  ,  1056,    625,    800,    600,    233,     25,  0}},
    {185  ,{53676 , 85.0  ,  1048,    631,    800,    600,    209,     31,  0}},
    {186  ,{35523 , 60.0  ,  1264,    817,   1024,    768,    230,     48,  0}},
    {187  ,{48379 , 60.0  ,  1344,    806,   1024,    768,    288,     36,  0}},
    {188  ,{56465 , 70.0  ,  1328,    806,   1024,    768,    271,     36,  0}},
    {189  ,{60024 , 75.0  ,  1312,    800,   1024,    768,    265,     32,  0}},
    {190  ,{68681 , 85.0  ,  1376,    808,   1024,    768,    298,     40,  0}},
    {191  ,{63856 , 70.0  ,  1480,    912,   1152,    864,    290,     48,  0}},
    {192  ,{67521 , 75.0  ,  1600,    900,   1152,    864,    378,     36,  0}},
    {193  ,{77101 , 85.0  ,  1576,    907,   1152,    864,    353,     43,  0}},
    {194  ,{46446 , 60.0  ,  1696,   1069,   1280,   1024,    350,     42,  0}},
    //{195  ,{60024 , 60.0  ,  1800,   1000,   1600,    900,    168,    100,  0}},    //DMR1660H 881
    {195  ,{60024 , 60.0  ,  1800,   1000,   1280,    960,    418,     40,  0}},
    {196  ,{49701 , 60.0  ,  1680,    831,   1280,    800,    319,     29,  0}},   //DMT1260D 881
    {197  ,{63979 , 60.0  ,  1688,   1066,   1280,   1024,    354,     42,  0}},
    {198  ,{75018 , 75.0  ,  1680,   1000,   1280,    960,    361,     40,  0}},
    {199  ,{79936 , 75.0  ,  1688,   1066,   1280,   1024,    385,     42,  0}},
    {200  ,{85984 , 85.0  ,  1728,   1011,   1280,    960,    377,     51,  0}},
    {201  ,{91157 , 85.0  ,  1728,   1072,   1280,   1024,    377,     48,  0}},
    {202  ,{75028 , 60.0  ,  2160,   1250,   1600,   1200,    489,     50,  0}},
    {203  ,{50100 , 60.0  ,  1680,    831,   1280,    800,    326,     28,  0}},
    {204  ,{15625 , 60.0  ,   944,    625,    768,    574,    154,     48,  0}},
    {205  ,{15625 , 60.0  ,  1136,    625,    920,    574,    188,     48,  0}},
    {206  ,{26574 , 60.0  ,  1568,    885,   1024,    768,      3,      2,  0}},
    {207  ,{49358 , 60.0  ,  1680,    828,   1280,    800,    334,     27,  0}},
    {208  ,{64766 , 60.0  ,  1864,   1087,   1400,   1050,    376,     36,  0}},
    {209  ,{48332 , 60.0  ,  1344,    806,   1024,    768,    294,     35,  0}},
    {210  ,{59382 , 60.0  ,  1312,    800,   1024,    768,    270,     31,  0}},
    {211  ,{68306 , 60.0  ,  1376,    808,   1024,    768,    302,     39,  0}},
    {212  ,{62460 , 60.0  ,  1696,    838,   1280,    800,    332,     35,  0}},
    {213  ,{37735 , 60.0  ,  1056,    628,    800,    600,    214,     23,  0}},
    {214  ,{46339 , 60.0  ,  1056,    625,    800,    600,    238,     24,  0}},
    {215  ,{53333 , 60.0  ,  1056,    631,    800,    600,    214,     30,  0}},
    {216  ,{49358 , 60.0  ,  1152,    667,    832,    624,    293,     35,  0}},
    {217  ,{24820 , 60.0  ,   848,    440,    640,    400,    144,     33,  0}},
    {218  ,{32862 , 60.0  ,  1456,    821,   1120,    750,    224,     63,  0}},
    {219  ,{73583 , 60.0  ,  2080,   1235,   1920,   1200,    111,     32,  0}},
    {220  ,{15979 , 60.0  ,   896,    265,    640,    200,    200,     48,  0}},
    {221  ,{31496 , 60.0  ,   935,    530,    720,    480,    161,     43,  0}},
    {222  ,{45004 , 60.0  ,   944,    600,    720,    576,    160,     23,  0}},
    {223  ,{63613 , 60.0  ,  1072,    636,    800,    600,    224,     35,  0}},
    {224  ,{70422 , 60.0  ,  1072,    640,    800,    600,    224,     39,  0}},
    {225  ,{77160 , 60.0  ,  1088,    643,    800,    600,    232,     42,  0}},
    {226  ,{31725 , 60.0  ,  1072,    529,    852,    480,    196,     37,  0}},
    {227  ,{45085 , 60.0  ,  1040,    626,    800,    600,    200,     25,  0}},
    {228  ,{50327 , 60.0  ,  1056,    629,    800,    600,    216,     28,  0}},
    {229  ,{56882 , 60.0  ,  1056,    632,    800,    600,    216,     31,  0}},
    {230  ,{44762 , 60.0  ,  1664,    746,   1280,    720,    324,     25,  0}},
    {231  ,{55959 , 60.0  ,  1248,    746,    960,    720,    232,     25,  0}},
    {232  ,{56625 , 60.0  ,  1696,    752,   1280,    720,    342,     30,  0}},
    {233  ,{81433 , 60.0  ,  1392,    814,   1024,    768,    296,     45,  0}},
    {234  ,{90090 , 60.0  ,  1392,    819,   1024,    768,    296,     50,  0}},
    {235  ,{57703 , 60.0  ,  1360,    801,   1024,    768,    280,     32,  0}},
    {236  ,{64350 , 60.0  ,  1376,    804,   1024,    768,    288,     35,  0}},
    {237  ,{72833 , 60.0  ,  1376,    809,   1024,    768,    288,     40,  0}},
    {238  ,{108459, 60.0  ,  1760,   1085,   1280,   1024,    384,     60,  0}},
    {239  ,{47824 , 60.0  ,  1674,    797,   1280,    768,    350,     28,  0}},
    {240  ,{85763 , 60.0  ,  1744,   1072,   1280,   1024,    368,     47,  0}},
    {241  ,{97087 , 60.0  ,  1744,   1078,   1280,   1024,    368,     53,  0}},
    {242  ,{63613 , 60.0  ,  2144,   1060,   1600,   1024,    432,     35,  0}},
    {243  ,{55897 , 60.0  ,  2128,    932,   1600,    900,    386,     28,  0}},
    {244  ,{90090 , 60.0  ,  2176,   1251,   1600,   1200,    464,     50,  0}},
    {245  ,{79491 , 60.0  ,  2144,   1060,   1600,   1024,    432,     35,  0}},
    {246  ,{100502, 60.0  ,  2192,   1256,   1600,   1200,    472,     55,  0}},
    {247  ,{90090 , 60.0  ,  2144,   1060,   1600,   1024,    432,     35,  0}},
    {248  ,{113765, 60.0  ,  2208,   1264,   1600,   1200,    480,     63,  0}},
    {249  ,{66225 , 60.0  ,  2112,   1205,   1920,   1200,    152,      4,  0}},
    {250  ,{72254 , 60.0  ,  2112,   1205,   1920,   1200,    152,      4,  0}},
    {251  ,{84602 , 60.0  ,  2608,   1128,   1920,   1080,    552,     47,  0}},
    {252  ,{96432 , 60.0  ,  2624,   1134,   1920,   1080,    560,     53,  0}},
    {253  ,{61614 , 60.0  ,  2240,   1540,   2048,   1536,    168,      3,  0}},
    {254  ,{70871 , 60.0  ,  2240,   1540,   2048,   1536,    168,      3,  0}},
    {255  ,{76863 , 72.0  ,  1688,   1067,   1280,   1024,    385,     42,  0}},
    {256  ,{37078 , 50.0  ,  1632,    744,   1280,    720,    295,     22,  0}},    //CVT1250H 881
    {257  ,{62656 , 60.0  ,  1712,    835,   1280,    800,    350,     34,  0}},
    {258  ,{47505 , 60.0  ,  1776,    792,   1360,    765,    350,     26,  0}},
    {259  ,{65359 , 60.0  ,  1896,   1099,   1400,   1050,    334,     36,  0}},
    {260  ,{55948 , 60.0  ,  1904,    934,   1440,    900,    287,     32,  0}},
    {261  ,{57870 , 60.0  ,  1344,    806,   1024,    768,    295,     35,  0}},
    {262  ,{78864 , 60.0  ,  1712,   1064,   1280,   1024,    400,     37,  0}},
    {263  ,{26246 , 60.0  ,  1524,    875,   1244,    842,    224,     17,  0}},
    {264  ,{62073 , 60.0  ,  1360,    805,   1024,    768,    304,     35,  0}},
    {265  ,{71736 , 60.0  ,  1472,    943,   1152,    900,    304,     41,  0}},
    {266  ,{71684 , 60.0  ,  1648,   1075,   1280,   1024,    344,     49,  0}},
    {267  ,{81168 , 60.0  ,  1664,   1066,   1280,   1024,    352,     40,  0}},
    {268  ,{89285 , 60.0  ,  2240,   1334,   1600,   1280,    640,     54,  0}},
    {269  ,{37495 , 60.0  ,  1344,    625,   1024,    600,    156,     23,  0}},
    {270  ,{49627 , 60.0  ,  1680,    828,   1280,    800,    328,     28,  0}},
    {271  ,{31162 , 60.0  ,   832,    445,    640,    350,    169,     59,  0}},
    {272  ,{35523 , 60.0  ,  1264,    817,   1024,    768,    232,     24,  0}},
    {273  ,{31026 , 60.0  ,  1088,    517,    848,    480,    200,     27,  0}},
    {274  ,{63897 , 60.0  ,  1688,   1065,   1280,   1024,    360,     40,  0}},
    {275  ,{93984 , 60.0  ,  2560,   1253,   1868,   1200,    556,     52,  0}},
    {276  ,{96618 , 60.0  ,  2624,   1288,   1920,   1234,    560,     53,  0}},
    {277  ,{107066, 60.0  ,  2640,   1260,   1920,   1200,    568,     59,  0}},
    {278  ,{95328 , 60.0  ,  2800,   1589,   2048,   1536,    600,     52,  0}},
    {279  ,{39447 , 60.0  ,   900,    449,    720,    400,    162,     28,  0}},
    {280  ,{25006 , 60.0  ,   800,    417,    640,    400,    137,     15,  0}},    //CVT0660D 802BT
    {281  ,{29694 , 60.0  ,   800,    500,    640,    480,    136,     18,  0}},    //CVT0650 881
    {282  ,{53614 , 85.0  ,  1264,    633,    960,    600,    239,     31,  0}},    //CVT0985D 881
    {283  ,{76329 ,120.0  ,   960,    636,    800,    600,    105,     34,  0}},    //CVR0812 881
    {284  ,{97585 ,120.0  ,  1184,    813,   1024,    768,    104,     43,  0}},    //CVR1012 881
    {285  ,{29381 , 60.0  ,   800,    494,    640,    480,    137,     36,  0}},
    {286  ,{74129 , 60.0  ,  2080,   1235,   1920,   1200,    104,     33,  0}},    //A70LV_Doulas_0121
    {287  ,{74296 , 60.0  ,  2080,   1235,   1920,   1200,    104,     33,  0}},    //A70LV_Doulas_0142
    //{287  ,{62638 , 75.0  ,  1696,    835,   1280,    800,    327,     33,  0}},    //A70LV_Doulas_0124 //A70LV_Doulas_0121
    {288  ,{66150 , 58.80 ,  2200,   1125,   1920,   1080,    184,     42,  0}},    //1080P -10%            //A70LV_Doulas_0234
    {289  ,{68850 , 61.20 ,  2200,   1125,   1920,   1080,    184,     42,  0}},    //1080P +10%            //A70LV_Doulas_0234
    {290  ,{72557 , 58.75 ,  2080,   1235,   1920,   1200,    104,     33,  0}},    //CVR1960D 881 -10%     //A70LV_Doulas_0234
    {291  ,{75519 , 61.15 ,  2080,   1235,   1920,   1200,    104,     33,  0}},    //CVR1960D 881 +10%     //A70LV_Doulas_0234
    {292  ,{15734 , 60    ,   858,    262,    720,    240,    115,     19,  1}},   //480i       //A70LV_Doulas_0244
};

static const sMODE_TABLE m_sModetableTableNormal[] =     //A70LV_Doulas_0123
{
    {0    ,{26968 , 60.0  ,  2750,   1125,   1920,   1080,    185,     42,  0}},
    {1    ,{26997 , 60.0  ,  2750,   1125,   1920,   1080,    185,     42,  0}},
    {2    ,{28121 , 60.0  ,  2640,   1125,   1920,   1080,    185,     42,  0}},
    {3    ,{33738 , 60.0  ,  2200,   1125,   1920,   1080,    185,     42,  0}},
    {4    ,{26975 , 60.0  ,  2750,   1125,   1920,   1080,    185,     42,  0}},
    {5    ,{27005 , 60.0  ,  2750,   1125,   1920,   1080,    185,     42,  0}},
    {6    ,{28121 , 60.0  ,  2640,    562,   1920,    540,    189,     21,  0}},        //A70LV_Doulas_0244 modify
    {7    ,{33738 , 60.0  ,  2200,    562,   1920,    540,    189,     21,  0}},        //A70LV_Doulas_0244 modify
    {8    ,{56242 , 60.0  ,  2640,   1125,   1920,   1080,    184,     42,  0}},
    {9    ,{67530 , 60.0  ,  2200,   1125,   1920,   1080,    185,     42,  0}},
    {10   ,{67430 , 60.0  ,  2200,   1123,   1920,   1080,    190,     41,  0}},
    {11   ,{67069 , 60.0  ,  2368,   1125,   1920,   1080,    412,     42,  0}},
    {12   ,{67888 , 60.0  ,  1552,    902,   1152,    864,    323,     37,  0}},
        //{13   ,{47824 , 60.0  ,  1664,    798,   1280,    768,    321,     28,  0}},
    {13   ,{47824 , 60.0  ,  1328,    798,   1024,    768,    247,     28,  0}},    //CVT1060 881
    {14   ,{47393 , 60.0  ,  1440,    790,   1280,    768,    105,     20,  0}},    //CVR1260E 881
    {15   ,{60132 , 60.0  ,  1712,    802,   1280,    768,    353,     33,  0}},
    {16   ,{60240 , 60.0  ,  1696,    805,   1280,    768,    334,     34,  0}},
    {17   ,{68587 , 60.0  ,  1728,    807,   1280,    768,    360,     38,  0}},
    {18   ,{68634 , 85.0  ,  1712,    809,   1280,    768,    343,     39,  0}},
    {19   ,{59594 , 60.0  ,  1712,    994,   1280,    960,    347,     33,  0}},
    {20   ,{75528 , 60.0  ,  1728,   1002,   1280,    960,    355,     41,  0}},
    {21   ,{47740 , 60.0  ,  1792,    795,   1360,    768,    362,     25,  0}},                    //A70LV_Doulas_0257
    {22   ,{47740 , 60.0  ,  1776,    798,   1360,    768,    333,     28,  0}},      //CVT1360H 881   //A70LV_Doulas_0257
    //{22   ,{47709 , 60.0  ,  1792,    798,   1366,    768,    350,     28,  0}},    //T1376V60
    {23   ,{67069 , 60.0  ,  2576,   1118,   1921,   1080,    190,     32,  0}},
    {24   ,{67521 , 60.0  ,  2200,   1115,   1920,   1080,    184,     32,  0}},
    {25   ,{15730 , 60.0  ,   858,    525,    720,    480,    125,     38,  0}},
   // {26   ,{31466 , 60.0  ,   800,    525,    640,    400,    136,     76,  0}},    //SMT0660D 881
    {26   ,{31466,  60.0  ,   858,    525,    720,    480,    115,     37,  0}},      //480P       //A70LV_Doulas_0244 modify
    //{26   ,{31466 , 60.0  ,   800,    525,    640,    480,    136,     36,  0}},    //SMT0660  881
    {27   ,{15628 , 50.0  ,   864,    312,    720,    288,    128,     23,  0}},    //576i      //A70LV_Doulas_0244 modify
    {28   ,{90009 , 60.0  ,  1650,    750,   1280,    720,    252,     26,  0}},
    {29   ,{37495 , 50.0  ,  1980,    750,   1280,    720,    253,     26,  0}},
    {30   ,{44964 , 60.0  ,  1650,    750,   1280,    720,    253,     26,  0}},
    {31   ,{45004 , 60.0  ,  1650,    750,   1280,    720,    253,     26,  0}},
    {32   ,{44464 , 60.0  ,  1664,    748,   1280,    720,    318,     25,  0}},
    {33   ,{55865 , 60.0  ,  1696,    755,   1280,    720,    334,     32,  0}},
    {34   ,{49726 , 75.0  ,  1152,    667,    832,    624,    280,     43,  0}},    //802BT 8362A75
    {35   ,{31026 , 60.0  ,  1088,    517,    848,    480,    217,     32,  0}},
    {36   ,{29629 , 60.0  ,   928,    495,    768,    480,     95,     10,  0}},
    {37   ,{36968 , 60.0  ,   960,    618,    800,    600,    105,     16,  0}},    //CVR0860 881
    {38   ,{29515 , 60.0  ,  1008,    494,    848,    480,     95,     10,  0}},    //CVR0860H 881
    {39   ,{37050 , 60.0  ,  1120,    618,    960,    600,     95,     10,  0}},
            //{40   ,{47303 , 60.0  ,  1120,    790,    800,    600,     95,     10,  0}},
    {40   ,{47303 , 60.0  ,  1184,    790,   1024,    768,    105,     20,  0}},    //CVR1060 881
    {41   ,{36968 , 60.0  ,  1224,    618,   1064,    600,     95,     10,  0}},
    {42   ,{53163 , 60.0  ,  1312,    889,   1152,    864,    105,     23,  0}},    //CVR1160 881
    {43   ,{44404 , 60.0  ,  1470,    741,   1152,    720,     95,     10,  0}},
    //{43   ,{44404 , 60.0  ,  1440,    741,   1280,    720,    105,     19,  0}},    //CVR1260H 881
    {44   ,{59241 , 60.0  ,  1440,    988,   1280,    960,    105,     26,  0}},    //CVR1260 881
    {45   ,{47326 , 60.0  ,  1470,    790,   1280,    768,     95,     10,  0}},
    {46   ,{39635 , 50.0  ,  1648,    793,   1280,    768,    303,     23,  0}},    //CVT1250E 881
    {47   ,{39588 , 60.0  ,  1680,    792,   1280,    768,    334,     10,  0}},
    {48   ,{75187 , 60.0  ,  2064,   1005,   1536,    960,     95,     10,  0}},
    {49   ,{49504 , 60.0  ,  2232,    991,   1704,    960,     95,     10,  0}},
    {50   ,{67114 , 60.0  ,  2320,   1120,   1728,   1080,     95,     10,  0}},
    {51   ,{59737 , 60.0  ,  2264,    997,   1704,    960,     95,     10,  0}},
    {52   ,{24479 , 60.0  ,   640,    407,    512,    384,    112,     22,  0}},
    {53   ,{34989 , 60.0  ,   864,    525,    640,    480,    174,     42,  0}},
    {54   ,{68870 , 60.0  ,   832,    918,    640,    870,    160,     45,  0}},
    {55   ,{49115 , 60.0  ,  1120,    654,    832,    624,    248,     28,  0}},        //APP0875
    {56   ,{48192 , 60.0  ,  1328,    813,   1024,    768,    224,     34,  0}},
    {57   ,{60240 , 60.0  ,  1328,    804,   1024,    768,    272,     33,  0}},
    {58   ,{68681 , 60.0  ,  1456,    915,   1152,    870,    272,     42,  0}},
    {59   ,{31250 , 50.0  ,   864,    625,    720,    576,    124,     45,  0}},
    {60   ,{64599 , 60.0  ,  1688,   1078,   1280,   1024,    300,     14,  0}},
    {61   ,{70671 , 60.0  ,  1696,   1063,   1280,   1024,    369,     38,  0}},
    {62   ,{63734 , 60.0  ,  1696,   1062,   1280,   1024,    300,     35,  0}},
    {63   ,{78125 , 60.0  ,  1728,   1091,   1280,   1024,    334,     36,  0}},
    {64   ,{47393 , 60.0  ,  1438,    790,   1280,    768,    110,     19,  0}},
    {65   ,{64641 , 60.0  ,  1520,   1080,   1400,   1050,    110,     27,  0}},
    {66   ,{38669 , 60.0  ,  1344,    806,   1024,    768,    295,     35,  0}},
    {67   ,{51150 , 60.0  ,  1696,   1066,   1280,   1024,    367,     41,  0}},
    {68   ,{63291 , 60.0  ,  1440,   1054,   1280,   1024,    105,     28,  0}},    //CVR1260G
    {69   ,{75930 , 60.0  ,  1696,   1056,   1280,   1024,    367,     31,  0}},
    {70   ,{35997 , 60.0  ,  1980,    750,   1280,    720,    259,     25,  0}},
    {71   ,{38387 , 60.0  ,  1544,    800,   1280,    768,    223,     29,  0}},
    {72   ,{40000 , 60.0  ,  1544,    800,   1280,    768,    219,     31,  0}},
    {73   ,{47961 , 60.0  ,  1540,    800,   1280,    768,    219,     29,  0}},
    {74   ,{50000 , 60.0  ,  1800,   1000,   1280,    960,    423,     39,  0}},
    {75   ,{40000 , 60.0  ,  1620,    800,   1366,    768,    219,     31,  0}},
    {76   ,{47961 , 60.0  ,  1620,    800,   1366,    768,    219,     29,  0}},
    {77   ,{51948 , 60.0  ,  1832,   1082,   1400,   1050,    359,     29,  0}},
    {78   ,{54495 , 60.0  ,  1736,   1090,   1400,   1050,    299,     37,  0}},
    {79   ,{65445 , 60.0  ,  1736,   1092,   1400,   1050,    299,     39,  0}},
    {80   ,{65659 , 60.0  ,  1856,   1094,   1400,   1050,    397,     43,  0}},
    {81   ,{80971 , 60.0  ,  1750,   1080,   1400,   1050,    313,     29,  0}},
    {82   ,{59952 , 60.0  ,  2160,   1250,   1600,   1200,    495,     49,  0}},
    {83   ,{62500 , 60.0  ,  2160,   1250,   1600,   1200,    495,     49,  0}},
    {84   ,{53995 , 60.0  ,  2640,   1125,   1920,   1080,    191,     41,  0}},
    {85   ,{33568 , 60.0  ,  2328,    560,   2048,    512,    192,     44,  0}},
    {86   ,{37439 , 60.0  ,   832,    520,    640,    480,    175,     39,  0}},
    {87   ,{31250 , 60.0  ,  1024,    625,    800,    600,    199,     24,  0}},
    {88   ,{37636 , 60.0  ,  1056,    628,    800,    600,    215,     27,  0}},
    {89   ,{52493 , 60.0  ,  1320,    875,   1079,    809,    205,     63,  0}},
    {90   ,{15750 , 60.0  ,   858,    525,    720,    480,    117,     35,  0}},
    {91   ,{29832 , 60.0  ,  1056,    497,    848,    480,    208,     16,  0}},
    {92   ,{24715 , 60.0  ,  1040,    495,    848,    480,    192,     14,  0}},
    {93   ,{57339 , 60.0  ,  2608,   1147,   1920,   1080,    159,     63,  0}},
    {94   ,{48709 , 60.0  ,  1338,    806,   1024,    768,    294,     38,  0}},
    {95   ,{66577 , 60.0  ,  2080,   1111,   1920,   1080,    105,     29,  0}},    //CVR1960H
    {96   ,{61425 , 60.0  ,  2080,   1229,   1920,   1200,    109,     26,  0}},
    {97   ,{60975 , 60.0  ,  2080,   1229,   1920,   1200,    109,     26,  0}},
    {98   ,{61881 , 60.0  ,  2080,   1229,   1920,   1200,    109,     26,  0}},
    {99   ,{37565 , 60.0  ,   832,    520,    640,    480,    157,     22,  0}},
    {100  ,{48007 , 60.0  ,  1500,    800,   1366,    768,    113,     32,  0}},    //DMR1360H 881
    {101  ,{62500 , 60.0  ,  2160,   1301,   1600,   1200,    512,    100,  0}},
    {102  ,{81234 , 60.0  ,  2160,   1250,   1600,   1200,    496,     49,  0}},
    {103  ,{87489 , 60.0  ,  2160,   1250,   1600,   1200,    496,     49,  0}},
    {104  ,{93720 , 60.0  ,  2160,   1250,   1600,   1200,    496,     49,  0}},
    {105  ,{100000, 60.0  ,  2160,   1250,   1600,   1200,    496,     49,  0}},
    {106  ,{106269, 60.0  ,  2160,   1250,   1600,   1200,    496,     49,  0}},
    {107  ,{83682 , 60.0  ,  2448,   1394,   1792,   1344,    528,     49,  0}},
    {108  ,{106269, 60.0  ,  2456,   1417,   1792,   1344,    568,     72,  0}},
    {109  ,{86355 , 60.0  ,  2528,   1439,   1856,   1392,    576,     46,  0}},
    {110  ,{90009 , 60.0  ,  2600,   1500,   1920,   1440,    552,     59,  0}},
    {111  ,{112485, 60.0  ,  2640,   1500,   1920,   1440,    576,     59,  0}},
    {112  ,{95785 , 60.0  ,  2504,   1597,   2048,   1536,    408,     60,  0}},
    {113  ,{120481, 60.0  ,  2656,   1606,   2048,   1536,    544,     69,  0}},
    {114  ,{66979 , 60.0  ,  2200,   1125,   1920,   1080,    190,     41,  0}},
    {115  ,{15733 , 60.0  ,   780,    525,    640,    480,    122,     41,  0}},
    {116  ,{62972 , 60.0  ,  1344,    840,   1024,    768,    256,     68,  0}},
    {117  ,{60240 , 60.0  ,  1328,    803,   1024,    768,    288,     29,  0}},
    {118  ,{78125 , 60.0  ,  1728,   1085,   1280,   1024,    382,     57,  0}},
    {119  ,{30293 , 60.0  ,   826,    508,    640,    480,    183,     24,  0}},
    {120  ,{39385 , 60.0  ,   800,    525,    640,    480,    144,     34,  0}},
    {121  ,{31466 , 70.0  ,   900,    449,    720,    400,    154,     38,  0}},    //7240A70
    {122  ,{35423 , 60.0  ,  1286,    815,   1053,    754,    219,     61,  0}},
    {123  ,{35612 , 60.0  ,  1280,    819,   1056,    768,    217,     51,  0}},
    {124  ,{56274 , 60.0  ,  1368,    804,   1024,    768,    326,     36,  0}},
    {125  ,{61087 , 60.0  ,  1408,    806,   1024,    768,    376,     38,  0}},
    {126  ,{70771 , 60.0  ,  1696,   1056,   1280,   1024,    384,     29,  0}},
    {127  ,{56497 , 60.0  ,  1824,   1097,   1360,   1024,    440,     73,  0}},
    {128  ,{37355 , 60.0  ,  1024,    624,    800,    600,    184,     22,  0}},    //CVT0860 881
    {129  ,{74019 , 60.0  ,  2080,   1235,   1920,   1200,    105,     33,  0}},    //CVR1960D 881
    {130  ,{31250 , 60.0  ,   800,    449,    640,    400,    142,     32,  0}},
    {131  ,{29949 , 60.0  ,   960,    500,    768,    480,    160,     18,  0}},    //CVT0760D 881
    {132  ,{37650 , 60.0  ,   976,    504,    768,    480,    168,     22,  0}},    //CVT0775D 881
    {133  ,{42903 , 85.0  ,   816,    507,    640,    480,    144,     25,  0}},    //CVT0685 881
    {134  ,{30998 , 50.0  ,   992,    621,    800,    600,    160,     19,  0}},    //CVT0850 881
    {135  ,{24691 , 50.0  ,   800,    497,    640,    480,    136,     15,  0}},    //CVT0650 881
    {136  ,{29824 , 60.0  ,  1056,    500,    848,    480,    176,     18,  0}},    //CVT0860H 881
    {137  ,{47103 , 60.0  ,  1040,    629,    800,    600,    192,     27,  0}},    //CVT0875 881
    {138  ,{37692 , 75.0  ,   816,    504,    640,    480,    144,     22,  0}},    //CVT0675 881
    {139  ,{53763 , 60.0  ,  1056,    633,    800,    600,    199,     31,  0}},    //CVT0885 881
    {140  ,{42973 , 60.0  ,  1088,    507,    848,    480,    198,     24,  0}},
    {141  ,{30835 , 60.0  ,  1200,    621,    960,    600,    214,     18,  0}},
    {142  ,{37216 , 60.0  ,  1216,    624,    960,    600,    216,     22,  0}},    //CVT0960D 881
    {143  ,{36656 , 60.0  ,  1296,    599,   1024,    576,    230,     20,  0}},
    {144  ,{53792 , 60.0  ,  1520,    897,   1152,    864,    295,     31,  0}},
    {145  ,{49407 , 50.0  ,  1680,    991,   1280,    960,    319,     29,  0}},    //CVT1250 881
    {146  ,{41254 , 60.0  ,  1648,    826,   1280,    800,    303,     24,  0}},    //CVT1250_ 881
    {147  ,{52687 , 60.0  ,  1680,   1057,   1280,   1024,    318,     31,  0}},    //CVT1250G 881
    {148  ,{59701 , 60.0  ,  1696,    996,   1280,    960,    326,     34,  0}},    //CVT1260 881
    {149  ,{47778 , 60.0  ,  1664,    798,   1280,    768,    311,     28,  0}},
    {150  ,{63653 , 60.0  ,  1712,   1063,   1280,   1024,    342,     37,  0}},    //CVT1260G 881
    {151  ,{44762 , 60.0  ,  1664,    748,   1280,    720,    311,     26,  0}},
    {152  ,{75244 , 60.0  ,  1728,   1005,   1280,    960,    349,     43,  0}},    //CVT1275G 881
    {153  ,{62774 , 75.0  ,  1696,    838,   1280,    800,    327,     36,  0}},
    {154  ,{60277 , 75.0  ,  1696,    805,   1280,    768,    327,     35,  0}},
    {155  ,{80321 , 60.0  ,  1728,   1072,   1280,   1024,    349,     46,  0}},    //CVT1275G 881
    {156  ,{56490 , 75.0  ,  1696,    755,   1280,    720,    326,     33,  0}},    //CVT1275H 881  //A70LV_Doulas_0232 modify
    {157  ,{85836 , 60.0  ,  1728,   1011,   1280,    960,    349,     49,  0}},    //CVT1285G 881
    {158  ,{71530 , 85.0  ,  1712,    843,   1280,    800,    343,     41,  0}},
    {159  ,{91491 , 85.0  ,  1744,   1078,   1280,   1024,    356,     52,  0}},    //CVT1285G 881
    {160  ,{64391 , 85.0  ,  1712,    759,   1280,    720,    343,     37,  0}},
    {161  ,{39556 , 60.0  ,  1744,    793,   1360,    768,    319,     23,  0}},
    {162  ,{54112 , 60.0  ,  2208,   1083,   1680,   1050,    430,     31,  0}},    //CVT1650D
    {163  ,{25000 , 60.0  ,  1864,   1089,   1400,   1050,    369,     36,  0}},
    {164  ,{55928 , 60.0  ,  1904,    934,   1440,    900,    382,     31,  0}},
    {165  ,{82328 , 75.0  ,  1896,   1099,   1400,   1050,    382,     47,  0}},
    {166  ,{70621 , 75.0  ,  1936,    942,   1440,    900,    389,     40,  0}},    //CVT1475D 881
    {167  ,{49480 , 60.0  ,  2016,    991,   1536,    960,    398,     28,  0}},
    {168  ,{59665 , 60.0  ,  2032,    996,   1536,    960,    406,     33,  0}},
    {169  ,{74515 , 60.0  ,  2160,   1245,   1600,   1200,    436,     43,  0}},    //CVT1660
    {170  ,{65324 , 60.0  ,  2240,   1089,   1680,   1050,    446,     37,  0}},        //CVT1660D 802BT
    //{170  ,{65324 , 60.0  ,  1864,   1089,   1400,   1050,    367,     37,  0}},      //CVT1460  802BT
    {171  ,{61804 , 60.0  ,  2560,   1238,   1920,   1200,    508,     36,  0}},    //CVT1950D
    {172  ,{55617 , 50.0  ,  2544,   1114,   1920,   1080,    501,     32,  0}},    //CVT1950H
    {173  ,{49309 , 60.0  ,  1440,    823,   1280,    800,    105,     21,  0}},
    {174  ,{31496 , 60.0  ,   858,    525,    720,    480,    114,     37,  0}},    //480P60
    //{174  ,{31496 , 60.0  ,   800,    525,    640,    480,    137,     36,  0}},    //dmt0660 802
    {175  ,{37864 , 72.0  ,   832,    520,    640,    480,    160,     32,  0}},
    {176  ,{37495 , 75.0  ,   840,    500,    640,    480,    176,     20,  0}},
    {177  ,{43271 , 85.0  ,   832,    509,    640,    480,    128,     29,  0}},
    {178  ,{37864 , 85.0  ,   832,    445,    640,    350,    153,     64,  0}},    //DMT0685F 881
    //{178  ,{37864 , 85.0  ,   832,    445,    640,    400,    120,     45,  0}},    //DMT0685D 881
    {179  ,{37864 , 60.0  ,   832,    445,    640,    350,    158,     63,  0}},
    {180  ,{37936 , 85.0  ,   936,    446,    720,    400,    172,     46,  0}},
    {181  ,{35161 , 56.0  ,  1024,    625,    800,    600,    193,     25,  0}},
    {182  ,{37878 , 60.0  ,  1056,    628,    800,    600,    209,     28,  0}},
    {183  ,{48076 , 72.0  ,  1040,    666,    800,    600,    177,     30,  0}},
    {184  ,{46882 , 75.0  ,  1056,    625,    800,    600,    233,     25,  0}},
    {185  ,{53676 , 85.0  ,  1048,    631,    800,    600,    209,     31,  0}},
    {186  ,{35523 , 60.0  ,  1264,    817,   1024,    768,    230,     48,  0}},
    {187  ,{48379 , 60.0  ,  1344,    806,   1024,    768,    288,     36,  0}},
    {188  ,{56465 , 70.0  ,  1328,    806,   1024,    768,    271,     36,  0}},
    {189  ,{60024 , 75.0  ,  1312,    800,   1024,    768,    265,     32,  0}},
    {190  ,{68681 , 85.0  ,  1376,    808,   1024,    768,    298,     40,  0}},
    {191  ,{63856 , 70.0  ,  1480,    912,   1152,    864,    290,     48,  0}},
    {192  ,{67521 , 75.0  ,  1600,    900,   1152,    864,    378,     36,  0}},
    {193  ,{77101 , 85.0  ,  1576,    907,   1152,    864,    353,     43,  0}},
    {194  ,{46446 , 60.0  ,  1696,   1069,   1280,   1024,    350,     42,  0}},
    {195  ,{60024 , 60.0  ,  1800,   1000,   1600,    900,    168,    100,  0}},    //DMR1660H 881
    //{195  ,{60024 , 60.0  ,  1800,   1000,   1280,    960,    418,     40,  0}},
    {196  ,{49701 , 60.0  ,  1680,    831,   1280,    800,    319,     29,  0}},   //DMT1260D 881
    {197  ,{63979 , 60.0  ,  1688,   1066,   1280,   1024,    354,     42,  0}},
    {198  ,{75018 , 75.0  ,  1680,   1000,   1280,    960,    361,     40,  0}},
    {199  ,{79936 , 75.0  ,  1688,   1066,   1280,   1024,    385,     42,  0}},
    {200  ,{85984 , 85.0  ,  1728,   1011,   1280,    960,    377,     51,  0}},
    {201  ,{91157 , 85.0  ,  1728,   1072,   1280,   1024,    377,     48,  0}},
    {202  ,{75028 , 60.0  ,  2160,   1250,   1600,   1200,    489,     50,  0}},
    {203  ,{50100 , 60.0  ,  1680,    831,   1280,    800,    326,     28,  0}},
    {204  ,{15625 , 60.0  ,   944,    625,    768,    574,    154,     48,  0}},
    {205  ,{15625 , 60.0  ,  1136,    625,    920,    574,    188,     48,  0}},
    {206  ,{26574 , 60.0  ,  1568,    885,   1024,    768,      3,      2,  0}},
    {207  ,{49358 , 60.0  ,  1680,    828,   1280,    800,    334,     27,  0}},
    {208  ,{64766 , 60.0  ,  1864,   1087,   1400,   1050,    376,     36,  0}},
    {209  ,{48332 , 60.0  ,  1344,    806,   1024,    768,    294,     35,  0}},
    {210  ,{59382 , 60.0  ,  1312,    800,   1024,    768,    270,     31,  0}},
    {211  ,{68306 , 60.0  ,  1376,    808,   1024,    768,    302,     39,  0}},
    {212  ,{62460 , 60.0  ,  1696,    838,   1280,    800,    332,     35,  0}},
    {213  ,{37735 , 60.0  ,  1056,    628,    800,    600,    214,     23,  0}},
    {214  ,{46339 , 60.0  ,  1056,    625,    800,    600,    238,     24,  0}},
    {215  ,{53333 , 60.0  ,  1056,    631,    800,    600,    214,     30,  0}},
    {216  ,{49358 , 60.0  ,  1152,    667,    832,    624,    293,     35,  0}},
    {217  ,{24820 , 60.0  ,   848,    440,    640,    400,    144,     33,  0}},
    {218  ,{32862 , 60.0  ,  1456,    821,   1120,    750,    224,     63,  0}},
    {219  ,{73583 , 60.0  ,  2080,   1235,   1920,   1200,    111,     32,  0}},
    {220  ,{15979 , 60.0  ,   896,    265,    640,    200,    200,     48,  0}},
    {221  ,{31496 , 60.0  ,   935,    530,    720,    480,    161,     43,  0}},
    {222  ,{45004 , 60.0  ,   944,    600,    720,    576,    160,     23,  0}},
    {223  ,{63613 , 60.0  ,  1072,    636,    800,    600,    224,     35,  0}},
    {224  ,{70422 , 60.0  ,  1072,    640,    800,    600,    224,     39,  0}},
    {225  ,{77160 , 60.0  ,  1088,    643,    800,    600,    232,     42,  0}},
    {226  ,{31725 , 60.0  ,  1072,    529,    852,    480,    196,     37,  0}},
    {227  ,{45085 , 60.0  ,  1040,    626,    800,    600,    200,     25,  0}},
    {228  ,{50327 , 60.0  ,  1056,    629,    800,    600,    216,     28,  0}},
    {229  ,{56882 , 60.0  ,  1056,    632,    800,    600,    216,     31,  0}},
    {230  ,{44762 , 60.0  ,  1664,    746,   1280,    720,    324,     25,  0}},
    {231  ,{55959 , 60.0  ,  1248,    746,    960,    720,    232,     25,  0}},
    {232  ,{56625 , 60.0  ,  1696,    752,   1280,    720,    342,     30,  0}},
    {233  ,{81433 , 60.0  ,  1392,    814,   1024,    768,    296,     45,  0}},
    {234  ,{90090 , 60.0  ,  1392,    819,   1024,    768,    296,     50,  0}},
    {235  ,{57703 , 60.0  ,  1360,    801,   1024,    768,    280,     32,  0}},
    {236  ,{64350 , 60.0  ,  1376,    804,   1024,    768,    288,     35,  0}},
    {237  ,{72833 , 60.0  ,  1376,    809,   1024,    768,    288,     40,  0}},
    {238  ,{108459, 60.0  ,  1760,   1085,   1280,   1024,    384,     60,  0}},
    {239  ,{47824 , 60.0  ,  1674,    797,   1280,    768,    350,     28,  0}},
    {240  ,{85763 , 60.0  ,  1744,   1072,   1280,   1024,    368,     47,  0}},
    {241  ,{97087 , 60.0  ,  1744,   1078,   1280,   1024,    368,     53,  0}},
    {242  ,{63613 , 60.0  ,  2144,   1060,   1600,   1024,    432,     35,  0}},
    {243  ,{55897 , 60.0  ,  2128,    932,   1600,    900,    386,     28,  0}},
    {244  ,{90090 , 60.0  ,  2176,   1251,   1600,   1200,    464,     50,  0}},
    {245  ,{79491 , 60.0  ,  2144,   1060,   1600,   1024,    432,     35,  0}},
    {246  ,{100502, 60.0  ,  2192,   1256,   1600,   1200,    472,     55,  0}},
    {247  ,{90090 , 60.0  ,  2144,   1060,   1600,   1024,    432,     35,  0}},
    {248  ,{113765, 60.0  ,  2208,   1264,   1600,   1200,    480,     63,  0}},
    {249  ,{66225 , 60.0  ,  2112,   1205,   1920,   1200,    152,      4,  0}},
    {250  ,{72254 , 60.0  ,  2112,   1205,   1920,   1200,    152,      4,  0}},
    {251  ,{84602 , 60.0  ,  2608,   1128,   1920,   1080,    552,     47,  0}},
    {252  ,{96432 , 60.0  ,  2624,   1134,   1920,   1080,    560,     53,  0}},
    {253  ,{61614 , 60.0  ,  2240,   1540,   2048,   1536,    168,      3,  0}},
    {254  ,{70871 , 60.0  ,  2240,   1540,   2048,   1536,    168,      3,  0}},
    {255  ,{76863 , 72.0  ,  1688,   1067,   1280,   1024,    385,     42,  0}},
    {256  ,{37078 , 50.0  ,  1632,    744,   1280,    720,    295,     22,  0}},    //CVT1250H 881
    {257  ,{62656 , 60.0  ,  1712,    835,   1280,    800,    350,     34,  0}},
    {258  ,{47505 , 60.0  ,  1776,    792,   1360,    765,    350,     26,  0}},
    {259  ,{65359 , 60.0  ,  1896,   1099,   1400,   1050,    334,     36,  0}},
    {260  ,{55948 , 60.0  ,  1904,    934,   1440,    900,    287,     32,  0}},
    {261  ,{57870 , 60.0  ,  1344,    806,   1024,    768,    295,     35,  0}},
    {262  ,{78864 , 60.0  ,  1712,   1064,   1280,   1024,    400,     37,  0}},
    {263  ,{26246 , 60.0  ,  1524,    875,   1244,    842,    224,     17,  0}},
    {264  ,{62073 , 60.0  ,  1360,    805,   1024,    768,    304,     35,  0}},
    {265  ,{71736 , 60.0  ,  1472,    943,   1152,    900,    304,     41,  0}},
    {266  ,{71684 , 60.0  ,  1648,   1075,   1280,   1024,    344,     49,  0}},
    {267  ,{81168 , 60.0  ,  1664,   1066,   1280,   1024,    352,     40,  0}},
    {268  ,{89285 , 60.0  ,  2240,   1334,   1600,   1280,    640,     54,  0}},
    {269  ,{37495 , 60.0  ,  1344,    625,   1024,    600,    156,     23,  0}},
    {270  ,{49627 , 60.0  ,  1680,    828,   1280,    800,    328,     28,  0}},
    {271  ,{31162 , 60.0  ,   832,    445,    640,    350,    169,     59,  0}},
    {272  ,{35523 , 60.0  ,  1264,    817,   1024,    768,    232,     24,  0}},
    {273  ,{31026 , 60.0  ,  1088,    517,    848,    480,    200,     27,  0}},
    {274  ,{63897 , 60.0  ,  1688,   1065,   1280,   1024,    360,     40,  0}},
    {275  ,{93984 , 60.0  ,  2560,   1253,   1868,   1200,    556,     52,  0}},
    {276  ,{96618 , 60.0  ,  2624,   1288,   1920,   1234,    560,     53,  0}},
    {277  ,{107066, 60.0  ,  2640,   1260,   1920,   1200,    568,     59,  0}},
    {278  ,{95328 , 60.0  ,  2800,   1589,   2048,   1536,    600,     52,  0}},
    {279  ,{39447 , 60.0  ,   900,    449,    720,    400,    162,     28,  0}},
    {280  ,{25006 , 60.0  ,   800,    417,    640,    400,    137,     15,  0}},    //CVT0660D 802BT
    {281  ,{29694 , 60.0  ,   800,    500,    640,    480,    136,     18,  0}},    //CVT0650 881
    {282  ,{53614 , 85.0  ,  1264,    633,    960,    600,    239,     31,  0}},    //CVT0985D 881
    {283  ,{76329 ,120.0  ,   960,    636,    800,    600,    105,     34,  0}},    //CVR0812 881
    {284  ,{97585 ,120.0  ,  1184,    813,   1024,    768,    104,     43,  0}},    //CVR1012 881
    {285  ,{29381 , 60.0  ,   800,    494,    640,    480,    137,     36,  0}},
    {286  ,{74129 , 60.0  ,  2080,   1235,   1920,   1200,    104,     33,  0}},    //A70LV_Doulas_0121
    {287  ,{74296 , 60.0  ,  2080,   1235,   1920,   1200,    104,     33,  0}},    //A70LV_Doulas_0142
    //{287  ,{62638 , 75.0  ,  1696,    835,   1280,    800,    327,     33,  0}},    //A70LV_Doulas_0124  //A70LV_Doulas_0121
    {288  ,{66150 , 58.80 ,  2200,   1125,   1920,   1080,    184,     42,  0}},    //1080P -10%            //A70LV_Doulas_0234
    {289  ,{68850 , 61.20 ,  2200,   1125,   1920,   1080,    184,     42,  0}},    //1080P +10%            //A70LV_Doulas_0234
    {290  ,{72557 , 58.75 ,  2080,   1235,   1920,   1200,    104,     33,  0}},    //CVR1960D 881 -10%     //A70LV_Doulas_0234
    {291  ,{75519 , 61.15 ,  2080,   1235,   1920,   1200,    104,     33,  0}},    //CVR1960D 881 +10%     //A70LV_Doulas_0234
    {292  ,{15734 , 60    ,   858,    262,    720,    240,    115,     19,  1}},   //480i       //A70LV_Doulas_0244
};

static const sMODE_TABLE m_sModetableTableSOG[] =
{
//ModeTableNum  HFreq FrameRate  HTotal VTotal HActive VActive HStart VStart ScanMode
  {   0      ,{ 15734,   60    ,  858 ,  262 ,   720 ,   240 ,  112 ,  19 ,    1}},   //480i
  {   1      ,{ 15734,   60    , 1716 ,  262 ,  1440 ,   240 ,   20 ,  20 ,    1}},   //480i(x2)
  {   2      ,{ 15625,   50    ,  864 ,  312 ,   720 ,   288 ,  126 ,  23 ,    1}},   //576i
  {   3      ,{ 15625,   50    , 1728 ,  312 ,  1440 ,   288 ,  232 ,  40 ,    1}},   //576i(x2)
  {   4      ,{ 28125,   50    , 2640 ,  562 ,  1920 ,   540 ,  180 ,  21 ,    1}},   //1080i@50  4,38      //A70LV_Doulas_0257 modify for Christie (185)
  {   5      ,{ 33750,   60    , 2200 ,  562 ,  1920 ,   540 ,  180 ,  21 ,    1}},   //1080i@60  22,20     //A70LV_Doulas_0257 modify for Christie (185)
  {   6      ,{ 31500,   60    ,  858 ,  525 ,   720 ,   480 ,  115 ,  37 ,    0}},   //480P
  {   7      ,{ 31250,   50    ,  864 ,  625 ,   720 ,   576 ,  125 ,  45 ,    0}},   //576P
  {   8      ,{ 37500,   50    , 1980 ,  750 ,  1280 ,   720 ,  253 ,  26 ,    0}},   //720P@50
  {   9      ,{ 45000,   60    , 1650 ,  750 ,  1280 ,   720 ,  253 ,  26 ,    0}},   //720P@60
  {  10      ,{ 26973,   23.976, 2750 , 1125 ,  1920 ,  1080 ,  184 ,  42 ,    0}},   //1080P@23
  {  11      ,{ 27000,   24    , 2750 , 1125 ,  1920 ,  1080 ,  184 ,  42 ,    0}},   //1080P@24
  {  12      ,{ 28125,   25    , 2640 , 1125 ,  1920 ,  1080 ,  184 ,  42 ,    0}},   //1080P@25
  {  13      ,{ 33716,   29.97 , 2200 , 1125 ,  1920 ,  1080 ,  184 ,  42 ,    0}},   //1080P@29
  {  14      ,{ 33750,   30    , 2200 , 1125 ,  1920 ,  1080 ,  184 ,  42 ,    0}},   //1080P@30
  {  15      ,{ 56250,   50    , 2640 , 1125 ,  1920 ,  1080 ,  183 ,  42 ,    0}},   //1080P@50
  {  16      ,{ 67433,   59.940, 2200 , 1125 ,  1920 ,  1080 ,  183 ,  42 ,    0}},   //1080P@59
  {  17      ,{ 67500,   60    , 2200 , 1125 ,  1920 ,  1080 ,  183 ,  42 ,    0}},   //1080P@60
//  {  18      ,{ 90000,  120    , 1650 ,  750 ,  1280 ,   720 ,   20 ,  20 ,    0}},   //720P@120
//  {  19      ,{135000,  120    , 2200 , 1125 ,  1920 ,  1080 ,   20 ,  20 ,    0}},   //1080P@120
//  {  20      ,{ 74100,   60    , 2080 , 1235 ,  1920 ,  1200 ,   20 ,  20 ,    0}},   //1920x1200@60
};

static const UINT16 m_ucModeTblWideCount = sizeof(m_sModetableTableWide) / sizeof(sMODE_TABLE);  //A70LV_Doulas_0123 modify   //A70LV_Doulas_0005
static const UINT16 m_ucModeTblNormalCount = sizeof(m_sModetableTableNormal) / sizeof(sMODE_TABLE);    //A70LV_Doulas_0123
static const UINT16 m_ucModeTbSOGCount = sizeof(m_sModetableTableSOG) / sizeof(sMODE_TABLE);    //A70LV_Doulas_0112
static const UINT8 m_ucPanelTblCount = sizeof(m_sPanelTable) / sizeof(sPANEL_INFO);
static UINT8 m_ucCurrentBank;
static TickType_t m_ulStartTicks;

static sCHANNEL_INFO m_sChannelInfo[eC341_CH_VNUMBER];  //
#ifdef C341_WARPING_ENABLE      //A70LV_Doulas_0100 M0dify
static UINT8 ucWarpInit = 0x03;    //A70LV_Doulas_0079
#endif
static BOOL  bForcedSyncResetEnable = FALSE;    //A70LV_Doulas_0079
static UINT16 uiForcedSyncResetH_Total;     //A70LV_Doulas_0079
static UINT16 uiForcedSyncResetV_Total;     //A70LV_Doulas_0079
static sVIDEO_TIMING2 m_FrontEnd_Timing[eC341_CH_VNUMBER]; //H30K_Doulas_0011


//A70LV_Doulas_2000 start
//UINT8  ucPM_WARPING = 0;
UINT8  ucPM_WARPING;  //wait review
UINT8  ucPM_MVFLT[eC341_CH_VNUMBER];
UINT8  ucPM_DIAG[eC341_CH_VNUMBER];
UINT8  ucPM_COMB[eC341_CH_VNUMBER];
UINT8  ucPM_TNR[eC341_CH_VNUMBER];
UINT8  ucPM_HNR[eC341_CH_VNUMBER];
UINT8  ucPM_VNR[eC341_CH_VNUMBER];
UINT8  ucPM_MNR[eC341_CH_VNUMBER];
UINT8  ucPM_BNR[eC341_CH_VNUMBER];
//A70LV_Doulas_2000 end
INT8 cPM_HSHARP[eC341_CH_VNUMBER];   //A70LV_Doulas_0003
INT8 cPM_VSHARP[eC341_CH_VNUMBER];   //A70LV_Doulas_0003
UINT32 IMFH_MCLK_COUNT[eC341_CH_VNUMBER];     //A70LV_Doulas_0004
UINT32 IMFV_MCLK_COUNT[eC341_CH_VNUMBER];     //A70LV_Doulas_0004

//UINT8  ucNo_Signal_Config[eC341_CH_VNUMBER] = {0,0};//A70LV_Doulas_0004
UINT8  ucNo_Signal_Config[eC341_CH_VNUMBER] ;//A70LV_Doulas_0004  //wait review

//A70LV_Doulas_0003 Add
#define PBP_SMALL_MAIN_FACTOR  3
#define PBP_SMALL_SUB_FACTOR   1
#define PBP_MEDIUM_MAIN_FACTOR 3
#define PBP_MEDIUM_SUB_FACTOR  2
#define PBP_LARGE_MAIN_FACTOR  1
#define PBP_LARGE_SUB_FACTOR   1

#define PBP_LARG_BIG_SIZE       2
#define PBP_LARG_SMALL_SIZE     1
#define PBP_MEDIUM_BIG_SIZE     5
#define PBP_MEDIUM_SMALL_SIZE   3
#define PBP_SMALL_BIG_SIZE      3
#define PBP_SMALL_SMALL_SIZE    2

//#define FRAME_MEMORY_FORMAT     24	//FrameMemory format 0=YUV444 30b,1=YUV444_24bit,2=YUV422_20b,3=YUV422 16b
UINT8 FRAME_MEMORY_FORMAT = 30;   //H2PF_Simon_0131

void dvC341_Set_Color_Correction(eC341_CH_V ch_v, INT32 blgain, INT32 grgain, INT32 flgain, UINT8 mode);

static sCHANNEL_SETTING             m_sChannelSetting[eC341_CH_VNUMBER];   //A70LV_Doulas_0013
static UINT8 m_AutoImage[eCM_SOURCE_NUMBER] = {eAUTO_IMAGE_WIDE}; //A70LV_Doulas_0112
static UINT8 WorkingHorzPosition[eC341_CH_VNUMBER];        //A70LV_Doulas_0113
static UINT8 WorkingVertPosition[eC341_CH_VNUMBER];        //A70LV_Doulas_0113
static UINT8 ucInput3D_Format = eINPUT_3D_TYPE_OFF;//A70LV_Doulas_0154
static UINT8  m_ucModeTableID[2] = {0xFF,0xFF};     //A70LV_Doulas_0195
static UINT16 m_BestModeTableNumber[2] = {0,0};     //A70LV_Doulas_0195
static UINT16 uiModeAdjusmenttHorzStart = 0;        //A70LV_Doulas_0195
static UINT16 uiModeAdjusmenttVertStart = 0;        //A70LV_Doulas_0195
static BOOL   bModeAdjustmentEnable = FALSE;        //A70LV_Doulas_0195
static BOOL   bModeAdjustmentTableUse = FALSE;      //A70LV_Doulas_0195
static sTIMING_TABLE m_sModeAdjustmentTimingTable;               //A70LV_Doulas_0195
static BOOL bCapImgOutput = FALSE;                     //G100_Owen_0073
static BOOL bUpgradeEnable = FALSE;                    //A65_OPTOMA_Doulas_0126
static BOOL bPowerNormalDone = FALSE;               //R70G2_Donview_Sammy_0040

static char cLastTakeC341Semaphore[128] = {'\0'};   //A35G2_Simon_0112

static sSCALER_PANEL_INFO m_sC341PanelInfo[eC341_CH_VNUMBER] = {0}; //A35G2_CDS_Larry_0008

//C341 DDR test----------------------------------------------------
static INT32 bk_ddr3gds,bk_ddr3msdly;
static UINT32 PS_MEMCHKSTAD = 0x7FF00000;	//0x1FF00000;
static UINT32 PS_MEMCHKENDAD = 0x7FFFFFFF;	//0x1FFFFFFF;

static INT32 bk_ddr3wrlvlphct1,bk_ddr3wrlvlphct2,bk_ddr3wrlvlphct3,bk_ddr3wrlvlphct4;
static INT32 bk_ddr3wrlvlphct5,bk_ddr3wrlvlphct6,bk_ddr3wrlvlphct7,bk_ddr3wrlvlphct8;
static INT32 bk_ddr3wrtmgct;
static INT32 bk_ddr3rdtmgct1,bk_ddr3rdtmgct2,bk_ddr3rdtmgct3,bk_ddr3rdtmgct4;
static INT32 bk_ddr3rdtmgct5,bk_ddr3rdtmgct6,bk_ddr3rdtmgct7,bk_ddr3rdtmgct8;
//C341 DDR test----------------------------------------------------


void dvC341_OPDMutexLockEvent(UINT16 uiEvent, const char *pcFunc)  //A35G2_Simon_0112
{
    uOPD_DATA uOPDData ;

    snprintf(uOPDData.cString, 128,  "%s,%s", pcFunc, cLastTakeC341Semaphore);

    utilOPD_EventSet(uiEvent, &uOPDData);
}

BOOL dvC341_SemaphoreTake(BOOL cEnable, const char *pcSemaphore)
{
    BOOL bResult = FALSE;

    if(cEnable)
    {
        bResult = Board_SSP_SemaphoreTake(eBOARD_SSP_CS_C341);

        if(bResult == FALSE)
        {
            dvC341_OPDMutexLockEvent(eOPD_MUTEX_LOCKED_LOG, pcSemaphore);   //A35G2_Simon_0112
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

void dbmsg_ScalerDriver(const char *FunctionName, INT32 LineNum, char *str,  ...)
{
    va_list args;
    va_start(args, str);

    UINT8 strOffset = 0 ;

    char String[256] = {'\0'};
    strOffset = snprintf(String, sizeof(String), "(%s@%d)", FunctionName, LineNum);
    vsnprintf(String+strOffset, sizeof(String)-strOffset, str, args);
    LOG_MSG(db_DV_SCALER, String);
    va_end(args);
}

void dvC341_UpgradeAccess_Set(BOOL bEnable)	//A65_OPTOMA_Doulas_0126
{
    bUpgradeEnable = bEnable;
}

BOOL dvC341_UpgradeAccess_Get(void)			//A65_OPTOMA_Doulas_0126
{
    return bUpgradeEnable;
}

static void dvC341_ResetTimeInState(void)
{
    m_ulStartTicks = xTaskGetTickCount();
}

static UINT32 dvC341_TimeElapsedInState(void)
{
    TickType_t ulTicks;

    ulTicks = xTaskGetTickCount();

    if(m_ulStartTicks > ulTicks)
    {
        return (0xFFFFFFFF - m_ulStartTicks + 1 + ulTicks) / portTICK_RATE_MS;
    }
    else
    {
        return (ulTicks - m_ulStartTicks) / portTICK_RATE_MS;
    }
}

void dvC341_RegAccessMode(eREG_ACCMODE eRegAccess)
{
    dvC341_Write(BN_ACCMODE, eRegAccess, 0);
}

void dvC341_Buffer_Flush(void)
{
    ASSERT(uiWriteIdx <= SPI_BUFFER_SIZE); //A35G2_CDS_Larry_0030

    if(uiWriteIdx)
    {
        SPI_SendWriteCmd(eBOARD_SSP_CS_C341, aucWriteBuffer, uiWriteIdx);
        uiWriteIdx = 0;
    }
}

void dvC341_Buffer_Flush_Burst(void)
{
    ASSERT(uiWriteIdx <= SPI_BUFFER_SIZE);

    if(uiWriteIdx)
    {
        SPI_SendBurstWriteCmd(eBOARD_SSP_CS_C341, aucWriteBuffer, uiWriteIdx, 1);
        uiWriteIdx = 0;
    }
}

void dvC341_WriteToBuffer(const UINT32 ulAddr, UINT32 ulData, const UINT8 ucBankOffset)
{
    UINT8 ucBank, ucReg, ucSize;
    //-------------------------------------------------------------
    ucBank = (ulAddr >> 16) & 0xFF;
    ucReg = (ulAddr >> 8) & 0xFF;
    ucSize = ulAddr & 0xFF;

    ucBank += ucBankOffset;

    m_ucCurrentBank = dvC341_CurrentBankGet();

    //-------------------------------------------------------------
    if(ucReg == 0x00)
    {
        m_ucCurrentBank = ulData & 0xFF;
        dvC341_CurrentBankSet(m_ucCurrentBank);
    }
    else if((ucBank != m_ucCurrentBank) && (ucReg >= 0x08))//Change to appropriate bank
    {
        m_ucCurrentBank = ucBank;
        dvC341_CurrentBankSet(m_ucCurrentBank);

        aucWriteBuffer[uiWriteIdx++] = 0x80;
        aucWriteBuffer[uiWriteIdx++] = ucBank;

        ASSERT(uiWriteIdx <= SPI_BUFFER_SIZE); //A35G2_CDS_Larry_0030

        dvC341_Buffer_Flush();
    }

    //-------------------------------------------------------------
    while(ucSize > 0)
    {
        aucWriteBuffer[uiWriteIdx++] = ucReg | 0x80;
        aucWriteBuffer[uiWriteIdx++] = (UINT8)(ulData & 0xFF);

        if(uiWriteIdx >= SPI_BUFFER_SIZE)
        {
            dvC341_Buffer_Flush();
        }

        ucReg++;
        ulData >>= 0x08;
        ucSize--;
    }
}

void dvC341_Write(const UINT32 ulAddr, UINT32 ulData, const UINT8 ucBankOffset)
{
    UINT8 ucBank, ucReg, ucSize, ucIdx;
    UINT8 ucWBuffer[16];
    //-------------------------------------------------------------
    ucBank = (ulAddr >> 16) & 0xFF;
    ucReg = (ulAddr >> 8) & 0xFF;
    ucSize = ulAddr & 0xFF;

    ucBank += ucBankOffset;

    dvC341_Buffer_Flush(); //A35G2_CDS_Larry_0030

    m_ucCurrentBank = dvC341_CurrentBankGet();

    //-------------------------------------------------------------
    if(ucReg == 0x00)
    {
        m_ucCurrentBank = ulData & 0xFF;
        dvC341_CurrentBankSet(m_ucCurrentBank);
    }
    else if((ucBank != m_ucCurrentBank) && (ucReg >= 0x08))//Change to appropriate bank
    {
        m_ucCurrentBank = ucBank;
        dvC341_CurrentBankSet(m_ucCurrentBank);

        ucWBuffer[0] = 0x80;
        ucWBuffer[1] = ucBank;

        SPI_SendWriteCmd(eBOARD_SSP_CS_C341, ucWBuffer, 2);
    }

    //-------------------------------------------------------------
    ucIdx = 0;
    while(ucSize > 0)
    {
        ucWBuffer[ucIdx++] = ucReg | 0x80;
        ucWBuffer[ucIdx++] = (UINT8)(ulData & 0xFF);

        ucReg++;
        ulData >>= 0x08;
        ucSize--;
    }

    SPI_SendWriteCmd(eBOARD_SSP_CS_C341, ucWBuffer, ucIdx);

    return;
}


void dvC341_Write_ch_v(eC341_CH_V ch_v, const UINT32 ulAddr, UINT32 ulData, const UINT8 ucBankOffset)
{
    if(ucBankOffset == 0)
    {
        dbmsg("warning : ucBankOffset == 0 (write ulAddr %d)\r\n", ulAddr);
    }

    dvC341_Write(ulAddr, ulData, ((ch_v * 2) + 0) * ucBankOffset);
    dvC341_Write(ulAddr, ulData, ((ch_v * 2) + 1) * ucBankOffset);
}


void dvC341_BurstRead_FixedAdd(const UINT32 ulAddr, UINT16 uiSize, UINT8 *pucData) //A70LV_Larry_0139
{
    UINT8 ucBank, ucReg, ucWBuffer[2];
    UINT8 *pcBuffer = malloc(uiSize + 4);

    m_ucCurrentBank = dvC341_CurrentBankGet();

    dvC341_Buffer_Flush();

    if(pcBuffer == NULL)
    {
        LOG_MSG(db_ALWAYS,"\n%s(%d) malloc fail\n", __FUNCTION__ , __LINE__);
        return;
    }

    dvC341_RegAccessMode(eRA_BURST_MODE_FIXED_ADDRESS);  //enable burst mode

    //-------------------------------------------------------------
    ucBank = (ulAddr >> 16) & 0xFF;
    ucReg = (ulAddr >> 8) & 0xFF;

    //b7-b6
    //0 Normal mode
    //1 Serial burst mode: address fixed
    //2 Serial burst mode: address auto increment
    //3 T.B.D
    //-------------------------------------------------------------
    if(ucReg == 0x00)
    {
        m_ucCurrentBank = *pucData & 0xFF;
        dvC341_CurrentBankSet(m_ucCurrentBank);
    }
    else if((ucBank != m_ucCurrentBank) && (ucReg >= 0x08))//Change to appropriate bank
    {
        m_ucCurrentBank = ucBank;
        dvC341_CurrentBankSet(m_ucCurrentBank);

        ucWBuffer[0] = 0x80;
        ucWBuffer[1] = ucBank;

        SPI_SendWriteCmd(eBOARD_SSP_CS_C341, ucWBuffer, 2);
    }

    //-------------------------------------------------------------
    ucWBuffer[0] = ucReg;

    SPI_SendReadCmd(eBOARD_SSP_CS_C341, ucWBuffer, pcBuffer, 2, (uiSize+1));

    memcpy(pucData, &pcBuffer[1], uiSize); //0 is dummy
    free(pcBuffer);

    dvC341_RegAccessMode(eRA_NORMAL);  //disable burst mode
}

void dvC341_BurstWrite_FixedAdd(const UINT32 ulAddr, UINT32 uiSize, UINT8 *pucData) //A70LV_Larry_0061
{
    UINT8 ucBank, ucReg;
    UINT8 *pcBuffer = malloc(uiSize + 4);

    m_ucCurrentBank = dvC341_CurrentBankGet();

    dvC341_Buffer_Flush();

    if(pcBuffer == NULL)
    {
        LOG_MSG(db_ALWAYS,"\n%s(%d) malloc fail\n", __FUNCTION__ , __LINE__);
        return;
    }


    dvC341_RegAccessMode(eRA_BURST_MODE_FIXED_ADDRESS);  //enable burst mode

    //-------------------------------------------------------------
    ucBank = (ulAddr >> 16) & 0xFF;
    ucReg = (ulAddr >> 8) & 0xFF;

    //b7-b6
    //0 Normal mode
    //1 Serial burst mode: address fixed
    //2 Serial burst mode: address auto increment
    //3 T.B.D
    //-------------------------------------------------------------
    if(ucReg == 0x00)
    {
        m_ucCurrentBank = *pucData & 0xFF;
        dvC341_CurrentBankSet(m_ucCurrentBank);
    }
    else if((ucBank != m_ucCurrentBank) && (ucReg >= 0x08))//Change to appropriate bank
    {
        m_ucCurrentBank = ucBank;
        dvC341_CurrentBankSet(m_ucCurrentBank);

        pcBuffer[0] = 0x80;
        pcBuffer[1] = ucBank;

        SPI_SendWriteCmd(eBOARD_SSP_CS_C341, pcBuffer, 2);
    }

    //-------------------------------------------------------------
    pcBuffer[0] = ucReg | 0x80;
    memcpy(&pcBuffer[1], pucData, uiSize);

    SPI_SendBurstWriteCmd(eBOARD_SSP_CS_C341, pcBuffer, (uiSize+1), 1);

    free(pcBuffer);

    dvC341_RegAccessMode(eRA_NORMAL);  //disable burst mode

}

void dvC341_BurstRead_AddInc(const UINT32 ulAddr, UINT16 uiSize, UINT8 *pucData) //A70LV_Larry_0139
{
    UINT8 ucBank, ucReg, ucWBuffer[2];
    UINT8 *pcBuffer = malloc(uiSize + 4);

    m_ucCurrentBank = dvC341_CurrentBankGet();

    dvC341_Buffer_Flush();

    if(pcBuffer == NULL)
    {
        LOG_MSG(db_ALWAYS,"\n%s(%d) malloc fail\n", __FUNCTION__ , __LINE__);
        return;
    }

    dvC341_RegAccessMode(eRA_BURST_MODE_AUTO_INC_ADDRESS);

    //-------------------------------------------------------------
    ucBank = (ulAddr >> 16) & 0xFF;
    ucReg = (ulAddr >> 8) & 0xFF;

    //BN_ACCMODE b7-b6
    //0 Normal mode
    //1 Serial burst mode: address fixed
    //2 Serial burst mode: address auto increment
    //3 T.B.D
    //-------------------------------------------------------------
    if(ucReg == 0x00)
    {
        m_ucCurrentBank = *pucData & 0xFF;
        dvC341_CurrentBankSet(m_ucCurrentBank);
    }
    else if((ucBank != m_ucCurrentBank) && (ucReg >= 0x08))//Change to appropriate bank
    {
        m_ucCurrentBank = ucBank;
        dvC341_CurrentBankSet(m_ucCurrentBank);

        ucWBuffer[0] = 0x80;
        ucWBuffer[1] = ucBank;

        SPI_SendWriteCmd(eBOARD_SSP_CS_C341, ucWBuffer, 2);
    }

    //-------------------------------------------------------------
    ucWBuffer[0] = ucReg;
    SPI_SendReadCmd(eBOARD_SSP_CS_C341, ucWBuffer, pcBuffer, 2, (uiSize+1));
    memcpy(pucData, &pcBuffer[1], uiSize); //0 id dummy
    free(pcBuffer);

    dvC341_RegAccessMode(eRA_NORMAL);
}


void dvC341_BurstWrite_AddInc(const UINT32 ulAddr, UINT32 uiSize, UINT8 *pucData)
{
    UINT8 ucBank, ucReg;
    UINT8 *pcBuffer = malloc(uiSize + 4);

    m_ucCurrentBank = dvC341_CurrentBankGet();

    dvC341_Buffer_Flush();

    if(pcBuffer == NULL)
    {
        LOG_MSG(db_ALWAYS,"\n%s(%d) malloc fail\n", __FUNCTION__ , __LINE__);
        return;
    }

    dvC341_RegAccessMode(eRA_BURST_MODE_AUTO_INC_ADDRESS); //enable burst mode

    //-------------------------------------------------------------
    ucBank = (ulAddr >> 16) & 0xFF;
    ucReg = (ulAddr >> 8) & 0xFF;

    //BN_ACCMODE b7-b6
    //0 Normal mode
    //1 Serial burst mode: address fixed
    //2 Serial burst mode: address auto increment
    //3 T.B.D

    //-------------------------------------------------------------
    if(ucReg == 0x00)
    {
        m_ucCurrentBank = *pucData & 0xFF;
        dvC341_CurrentBankSet(m_ucCurrentBank);
    }
    else if((ucBank != m_ucCurrentBank) && (ucReg >= 0x08))//Change to appropriate bank
    {
        m_ucCurrentBank = ucBank;
        dvC341_CurrentBankSet(m_ucCurrentBank);

        pcBuffer[0] = 0x80;
        pcBuffer[1] = ucBank;

        SPI_SendWriteCmd(eBOARD_SSP_CS_C341, pcBuffer, 2);
    }

    //-------------------------------------------------------------
    pcBuffer[0] = ucReg | 0x80;
    memcpy(&pcBuffer[1], pucData, uiSize);

    SPI_SendBurstWriteCmd(eBOARD_SSP_CS_C341, pcBuffer, (uiSize+1), 1);
    free(pcBuffer);

    dvC341_RegAccessMode(eRA_NORMAL);
}

UINT32 dvC341_Read(const UINT32 ulAddr, const UINT8 ucBankOffset)
{
    UINT8 ucBank, ucReg, ucBytes, ucWBuffer[2], ucRBuffer[2];
    UINT32 ulData = 0;

    m_ucCurrentBank = dvC341_CurrentBankGet();

    dvC341_Buffer_Flush(); //A35G2_CDS_Larry_0030

    ucBank = (ulAddr >> 16) & 0xFF;
    ucReg = (ulAddr >> 8) & 0xFF;
    ucBytes = ulAddr & 0xFF;

    ucBank += ucBankOffset;

    if((ucBank != m_ucCurrentBank) && (ucReg >= 0x08))
    {
        m_ucCurrentBank = ucBank;
        dvC341_CurrentBankSet(m_ucCurrentBank);

        ucWBuffer[0] = 0x80;
        ucWBuffer[1] = ucBank;

        SPI_SendWriteCmd(eBOARD_SSP_CS_C341, ucWBuffer, 2);
    }

    while(ucBytes > 0)
    {
        ucBytes--;
        ulData = (ulData << 8);
        ucWBuffer[0] = (ucReg + ucBytes);// & ~(0x80);

        SPI_SendReadCmd(eBOARD_SSP_CS_C341, ucWBuffer, ucRBuffer, 2, 2);
        ulData += (ucRBuffer[1] & 0xFF);//cRBuffer[0] is dummy data
    }

    return ulData;
}

//wait review  //simon check
void dvC341_reg_set(int ADDR,int val)  //A70LV_Doulas_0002
{
	dvC341_Write_ch_v(eC341_CH_V0, ADDR, val, CH_BANK_OFFSET);						// ch_v 0 side Write

    if(m_sChannelSetting[eC341_CH_V0].cPIP_PBP_ON == FALSE)
    {
		dvC341_Write_ch_v(eC341_CH_V1, ADDR, 0x00, CH_BANK_OFFSET);  //clear ch_v 1
	}
	else
    {                   // !Async
		dvC341_Write_ch_v(eC341_CH_V1, ADDR, val, CH_BANK_OFFSET);  //write ch_v 1
	}
}

void dvC341_SetBrightness(const eC341_CH_V ch_v) //A70LV_Doulas_0022 //A70LV_Doulas_0011 modify
{
    UINT8 ucRegOffset = 0;
    UINT8 Offset = 16;

    if(ch_v >= eC341_CH_VNUMBER)
    {
        dbmsg("ch_v %d error\r\n", ch_v);
        return;
    }

    if(m_sChannelSetting[eC341_CH_V0].cPIP_PBP_ON == FALSE)
    {
        if(ch_v == eC341_CH_V1)
        {
            return ;
        }
    }

    //dvC341_Write(B0_RTCT, RTCT_POVSSTOP, 0);   //0x4321FFFF
    dvC341_set_rtct_stop();

    for(UINT8 ch=0; ch<OUTPUT4K_CH_NUM; ch++)
    {
        ucRegOffset = ((ch_v * 2) + ch) * Offset;
        dvC341_Write(B12_BGCTCH1,		0x01,ucRegOffset) ;
        dvC341_Write(B12_BIASRCH1,0,ucRegOffset);	//Output bias correction
        dvC341_Write(B12_BIASGCH1,(m_sChannelSetting[eC341_CH_V0].iBrightness)&0x0fff,ucRegOffset);
        dvC341_Write(B12_BIASBCH1,0,ucRegOffset);
    }

    //dvC341_Write(B0_RTCT, RTCT_NORMAL, 0);   //0x4321CBA9
    dvC341_set_rtct_normal();
}

//A70LV_Doulas_2000 start
void dvC341_Set_flddly(const eC341_CH_V ch_v)
{
	double	Input_frame_rate,Input_V_total,IACTVST,IACTVW,IPSYRDLY;
	double	Output_V_Total,Output_frame_rate,OACTVST,OACTVW,FLDDLY_UNINT;    //A70LV_Doulas_0008 modify
	double	UNITDLY_FLDDLY,UNITDLY_IPSYRDLY,TIST1,TIEND1,TOST,TOEND;
	double	MINDLY,MIN_REG_FLDDLY,TYP_REG_FLDDLY,LOW_LIMIT_REG_FLDDLY,HI_LIMIT_REG_FLDDLY,REG_FLDDLY;

	int		val;

	//int po_refdiv,po_fbdiv,po_odiv,po_clk;

    if(ch_v >= eC341_CH_VNUMBER)
    {
        return;
    }

//ICH selection ---------------------------------------------------
	Input_frame_rate = (double)m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.dFrameRate;
	Input_V_total    = (double)m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.uiVTotal;

	if(m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.eScanMode == eSCAN_MODE_INTERLACE)
    {
		val = (m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiVStart -1)* 2 ;
	}
	else
    {
		val = m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiVStart;
     //	val= ((PM_SCAN==1)&(PM_IWIN_VW[ch]>=1200)) ? val+60 : val ;			//WUXGA 1:1
	}
	IACTVST          = (double)val;

	if(m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.eScanMode == eSCAN_MODE_INTERLACE)
    {
		val = (m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive * 2);
	}
	else
    {
		val = m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive;
	}
	IACTVW   = val;

	IPSYRDLY = (double)(m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.eScanMode == eSCAN_MODE_INTERLACE) ? 1 : 0;

//OCH selection ---------------------------------------------------
/*	po_refdiv    =  PS_REG_POREFDIV + 1;
	po_fbdiv     =  PS_REG_POFBDIV  + 1;
	po_odiv = (PS_REG_POPLLCT & 0x30) >> 4 == 0 ? 1
			: (PS_REG_POPLLCT & 0x30) >> 4 == 1 ? 2
			: (PS_REG_POPLLCT & 0x30) >> 4 == 2 ? 4 : 8;

	po_clk = PEVB_POCLK[0] / po_refdiv * po_fbdiv / 2 / po_odiv;
*/
	Output_V_Total    = m_sChannelInfo[ch_v].sOutputTimingInfo.uiVTotal;
	//Output_H_Total    = m_sChannelInfo[ch_v].sOutputTimingInfo.uiHTotal;   //A70LV_Doulas_0008 remove
	Output_frame_rate = m_sChannelInfo[ch_v].sOutputTimingInfo.ucFrameRate;

	OACTVST           = m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiVStart;
	OACTVW            = m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiVActive;

	FLDDLY_UNINT      = 1.0;//8.0; //H30K_Doulas_0010 debug

    if(Output_V_Total == 0 ||   //H2PF_Simon_0196
       Output_frame_rate == 0 ||
       Input_V_total == 0 ||
       Input_frame_rate == 0)
    {
        LOG_MSG(db_DV_SCALER, "Set Flddly Error (%f %f %f %f)\n", Output_V_Total, Output_frame_rate, Input_V_total, Input_frame_rate);
        return;
    }

//-----------------------------------------------------------------
	UNITDLY_FLDDLY    = FLDDLY_UNINT / Output_V_Total / Output_frame_rate;
	UNITDLY_IPSYRDLY  = 4.0          / Input_V_total  / Input_frame_rate;

	TIST1  = IACTVST / Input_V_total / Input_frame_rate;
	TIEND1 = (IACTVST + IACTVW) / Input_V_total / Input_frame_rate;

	TOST   = OACTVST / Output_V_Total / Output_frame_rate;
	TOEND  = (OACTVST + OACTVW) / Output_V_Total / Output_frame_rate;

	MINDLY = (MIN( (TOST - TIST1) , (TOEND - TIEND1) )) * (-1);

	MIN_REG_FLDDLY = (MINDLY + UNITDLY_IPSYRDLY * IPSYRDLY) / UNITDLY_FLDDLY;

	TYP_REG_FLDDLY = MIN_REG_FLDDLY + Output_V_Total * 0.1 / FLDDLY_UNINT;

	LOW_LIMIT_REG_FLDDLY = 2.0;

	HI_LIMIT_REG_FLDDLY = (Output_V_Total - 20.0) / FLDDLY_UNINT ;

	REG_FLDDLY = (TYP_REG_FLDDLY < LOW_LIMIT_REG_FLDDLY ) ? LOW_LIMIT_REG_FLDDLY :
	             (TYP_REG_FLDDLY > HI_LIMIT_REG_FLDDLY  ) ? HI_LIMIT_REG_FLDDLY  : TYP_REG_FLDDLY;

	if(REG_FLDDLY < 0.0){						// round
		val = (int)(REG_FLDDLY - 0.5);
	}
	else{
		val = (int)(REG_FLDDLY + 0.5);
	}

    LOG_MSG(db_DV_SCALER,"Flddly: (%f,%f)(%d)\n",
		Input_frame_rate,Output_frame_rate ,val&0xffff);      //A70LV_Doulas_0003

	dvC341_Write_ch_v(ch_v, B8_FLDDLYCH1, val, CH_BANK_OFFSET);	//Field delay

	return;
}

void dvC341_Set_Flip(const eC341_CH_V ch_v)    //A70LV_Doulas_0002
{
    if((m_sChannelInfo[eC341_CH_V0].ePanelTimingId != ePANEL_ID_1080P_120HZ) &&
       (m_sChannelInfo[eC341_CH_V0].ePanelTimingId != ePANEL_ID_WUXGA_120HZ)) //H30K_Doulas_0001
    {
        return;   //simon check ... not implement
    }

	UINT32 wdt,val2;
	UINT32 val1;    //H30K_Doulas_0002
//	UINT32	FLG_VSHRNK=0;
//	int mlessvflip=0;
	//UINT32 WOSFLD0,WOSFLD1,WOSFLD2,WOSFLD3;     //A70LV_Doulas_0154
	//UINT32 WISFLD0,WISFLD1,WISFLD2,WISFLD3;     //A70LV_Doulas_0154
//    UINT32 sPS_MEM_SC[2][4];  A70LV_Doulas_0098
    UINT8 ucRegOffset = 0;

    if(ch_v >= eC341_CH_VNUMBER)
    {
        return;
    }

    val2 = DEF_MWI_2K_3D; //H30K_Doulas_0001

    if(ch_v == eC341_CH_V0)    //A70LV_Doulas_0098 modify
    {
        for(UINT8 ch=0; ch<OUTPUT4K_CH_NUM; ch++)
        {
            UINT8 BankOffset = CH_BANK_OFFSET;
            ucRegOffset = ((ch_v * 2) + ch) * BankOffset;

            switch(dvC341_Input_3D_Format_Config_Get())        //A70LV_Doulas_0154 modify    //simon check
            {
                #if 1 //H30K_Doulas_0001
                case eINPUT_3D_TYPE_FRAMEPACKING:
                    if(ch == 0)
                    {
                        dvC341_WriteToBuffer(B22_OSFLD0CH1,0x00000000,ucRegOffset);
                        dvC341_WriteToBuffer(B22_OSFLD1CH1,0x00a00000,ucRegOffset);
    	                dvC341_WriteToBuffer(B22_OSFLD2CH1,0x01400000,ucRegOffset);
    	                dvC341_WriteToBuffer(B22_OSFLD3CH1,0x01e00000,ucRegOffset);
    	            }
    	            else
    	            {
                        if(m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive > 1918 && m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive < 1922)
                        {
                            val1 = 0xf00 * FRAME_MEMORY_FORMAT / 30; //H30K_Doulas_0016
                            dvC341_WriteToBuffer(B22_OSFLD0CH1,0x00000000 + val1,ucRegOffset);
                            dvC341_WriteToBuffer(B22_OSFLD1CH1,0x00a00000 + val1,ucRegOffset);
                            dvC341_WriteToBuffer(B22_OSFLD2CH1,0x01400000 + val1,ucRegOffset);
                            dvC341_WriteToBuffer(B22_OSFLD3CH1,0x01e00000 + val1,ucRegOffset);
                            //dvC341_WriteToBuffer(B22_OSFLD0CH1,0x00000f00,ucRegOffset);
                            //dvC341_WriteToBuffer(B22_OSFLD1CH1,0x00a00f00,ucRegOffset);
                            //dvC341_WriteToBuffer(B22_OSFLD2CH1,0x01400f00,ucRegOffset);
                            //dvC341_WriteToBuffer(B22_OSFLD3CH1,0x01e00f00,ucRegOffset);
                        }
                        else
                        {
                            val1 = 0x9f0 * FRAME_MEMORY_FORMAT / 30; //H30K_Doulas_0016
                            dvC341_WriteToBuffer(B22_OSFLD0CH1,0x00000000 + val1,ucRegOffset);
                            dvC341_WriteToBuffer(B22_OSFLD1CH1,0x00a00000 + val1,ucRegOffset);
                            dvC341_WriteToBuffer(B22_OSFLD2CH1,0x01400000 + val1,ucRegOffset);
                            dvC341_WriteToBuffer(B22_OSFLD3CH1,0x01e00000 + val1,ucRegOffset);
     	                    //dvC341_WriteToBuffer(B22_OSFLD0CH1,0x000009f0,ucRegOffset);
                            //dvC341_WriteToBuffer(B22_OSFLD1CH1,0x00a009f0,ucRegOffset);
                            //dvC341_WriteToBuffer(B22_OSFLD2CH1,0x014009f0,ucRegOffset);
                            //dvC341_WriteToBuffer(B22_OSFLD3CH1,0x01e009f0,ucRegOffset);
    	                }
    	            }
                    break;

                case eINPUT_3D_TYPE_SIDEBYSIDE:
                    if(ch == 0)
                    {
     	                dvC341_WriteToBuffer(B22_OSFLD0CH1,0x00000000,ucRegOffset);
    	                dvC341_WriteToBuffer(B22_OSFLD1CH1,0x00a00000,ucRegOffset);
    	                dvC341_WriteToBuffer(B22_OSFLD2CH1,0x01400000,ucRegOffset);
    	                dvC341_WriteToBuffer(B22_OSFLD3CH1,0x01e00000,ucRegOffset);
    	            }
    	            else
    	            {
                        if(m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive > 1918 && m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive < 1922)
                        {
                            if(m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.eScanMode == eSCAN_MODE_INTERLACE)
                            {
                                val1 = 0x770 * FRAME_MEMORY_FORMAT / 30; //H30K_Doulas_0016
                                dvC341_WriteToBuffer(B22_OSFLD0CH1,0x00000000 + val1,ucRegOffset);
                                dvC341_WriteToBuffer(B22_OSFLD1CH1,0x00a00000 + val1,ucRegOffset);
                                dvC341_WriteToBuffer(B22_OSFLD2CH1,0x01400000 + val1,ucRegOffset);
                                dvC341_WriteToBuffer(B22_OSFLD3CH1,0x01e00000 + val1,ucRegOffset);
                                //dvC341_WriteToBuffer(B22_OSFLD0CH1,0x00000770,ucRegOffset); //H30K_Doulas_0005 Modify
                                //dvC341_WriteToBuffer(B22_OSFLD1CH1,0x00a00770,ucRegOffset);
                                //dvC341_WriteToBuffer(B22_OSFLD2CH1,0x01400770,ucRegOffset);
                                //dvC341_WriteToBuffer(B22_OSFLD3CH1,0x01e00770,ucRegOffset);
                            }
                            else
                            {
                                val1 = 0x770 * FRAME_MEMORY_FORMAT / 30; //H30K_Doulas_0016
                                dvC341_WriteToBuffer(B22_OSFLD0CH1,0x00000000 + val1,ucRegOffset);
                                dvC341_WriteToBuffer(B22_OSFLD1CH1,0x00a00000 + val1,ucRegOffset);
                                dvC341_WriteToBuffer(B22_OSFLD2CH1,0x01400000 + val1,ucRegOffset);
                                dvC341_WriteToBuffer(B22_OSFLD3CH1,0x01e00000 + val1,ucRegOffset);
                                //dvC341_WriteToBuffer(B22_OSFLD0CH1,0x00000770,ucRegOffset);
                                //dvC341_WriteToBuffer(B22_OSFLD1CH1,0x00a00770,ucRegOffset);
                                //dvC341_WriteToBuffer(B22_OSFLD2CH1,0x01400770,ucRegOffset);
                                //dvC341_WriteToBuffer(B22_OSFLD3CH1,0x01e00770,ucRegOffset);
                            }
                        }
                        else //720P to do
                        {
                            val1 = 0x4f4 * FRAME_MEMORY_FORMAT / 30; //H30K_Doulas_0016
                            dvC341_WriteToBuffer(B22_OSFLD0CH1,0x00000000 + val1,ucRegOffset);
                            dvC341_WriteToBuffer(B22_OSFLD1CH1,0x00a00000 + val1,ucRegOffset);
                            dvC341_WriteToBuffer(B22_OSFLD2CH1,0x01400000 + val1,ucRegOffset);
                            dvC341_WriteToBuffer(B22_OSFLD3CH1,0x01e00000 + val1,ucRegOffset);
                            //dvC341_WriteToBuffer(B22_OSFLD0CH1,0x000004f4,ucRegOffset);
                            //dvC341_WriteToBuffer(B22_OSFLD1CH1,0x00a004f4,ucRegOffset);
                            //dvC341_WriteToBuffer(B22_OSFLD2CH1,0x014004f4,ucRegOffset);
                            //dvC341_WriteToBuffer(B22_OSFLD3CH1,0x01e004f4,ucRegOffset);
                        }
    	            }
                    break;

                case eINPUT_3D_TYPE_TOPANDBOTTOM:
                    if(ch == 0)
                    {
     	                dvC341_WriteToBuffer(B22_OSFLD0CH1,0x00000000,ucRegOffset);
    	                dvC341_WriteToBuffer(B22_OSFLD1CH1,0x00a00000,ucRegOffset);
    	                dvC341_WriteToBuffer(B22_OSFLD2CH1,0x01400000,ucRegOffset);
    	                dvC341_WriteToBuffer(B22_OSFLD3CH1,0x01e00000,ucRegOffset);
    	            }
    	            else
    	            {
    	                if((m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive > 1918 && m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive < 1922) &&
                           (m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.eScanMode == eSCAN_MODE_PROGRESSIVE))
    	                {
    	                    val1 = 0xf00 * FRAME_MEMORY_FORMAT / 30; //H30K_Doulas_0016
                            dvC341_WriteToBuffer(B22_OSFLD0CH1,0x00000000 + val1,ucRegOffset);
                            dvC341_WriteToBuffer(B22_OSFLD1CH1,0x00a00000 + val1,ucRegOffset);
                            dvC341_WriteToBuffer(B22_OSFLD2CH1,0x01400000 + val1,ucRegOffset);
                            dvC341_WriteToBuffer(B22_OSFLD3CH1,0x01e00000 + val1,ucRegOffset);
    	                    //dvC341_WriteToBuffer(B22_OSFLD0CH1,0x00000f00,ucRegOffset);
                            //dvC341_WriteToBuffer(B22_OSFLD1CH1,0x00a00f00,ucRegOffset);
                            //dvC341_WriteToBuffer(B22_OSFLD2CH1,0x01400f00,ucRegOffset);
                            //dvC341_WriteToBuffer(B22_OSFLD3CH1,0x01e00f00,ucRegOffset);
    	                }
    	                else
    	                {
    	                    val1 = 0x9f0 * FRAME_MEMORY_FORMAT / 30; //H30K_Doulas_0016
                            dvC341_WriteToBuffer(B22_OSFLD0CH1,0x00000000 + val1,ucRegOffset);
                            dvC341_WriteToBuffer(B22_OSFLD1CH1,0x00a00000 + val1,ucRegOffset);
                            dvC341_WriteToBuffer(B22_OSFLD2CH1,0x01400000 + val1,ucRegOffset);
                            dvC341_WriteToBuffer(B22_OSFLD3CH1,0x01e00000 + val1,ucRegOffset);
                            //dvC341_WriteToBuffer(B22_OSFLD0CH1,0x000009f0,ucRegOffset);
                            //dvC341_WriteToBuffer(B22_OSFLD1CH1,0x00a009f0,ucRegOffset);
                            //dvC341_WriteToBuffer(B22_OSFLD2CH1,0x014009f0,ucRegOffset);
                            //dvC341_WriteToBuffer(B22_OSFLD3CH1,0x01e009f0,ucRegOffset);
                        }
    	            }
                    break;

                case eINPUT_3D_TYPE_FRAME_SEQUENTIAL:
                    if(ch == 0)
                    {
                        #ifdef FRAME_SEQUENTAIL_SELF_TEST_120HZ //H30K_Doulas_0005
                        dvC341_WriteToBuffer(B22_OSFLD0CH1,0x00000000,ucRegOffset);
                        dvC341_WriteToBuffer(B22_OSFLD1CH1,0x00000000,ucRegOffset);
    	                dvC341_WriteToBuffer(B22_OSFLD2CH1,0x00a00000,ucRegOffset);
    	                dvC341_WriteToBuffer(B22_OSFLD3CH1,0x00a00000,ucRegOffset);
    	                #else
    	                dvC341_WriteToBuffer(B22_OSFLD0CH1,0x00000000,ucRegOffset);
                        dvC341_WriteToBuffer(B22_OSFLD1CH1,0x00a00000,ucRegOffset);
    	                dvC341_WriteToBuffer(B22_OSFLD2CH1,0x00000000,ucRegOffset);
    	                dvC341_WriteToBuffer(B22_OSFLD3CH1,0x00a00000,ucRegOffset);
    	                #endif
    	            }
    	            else
    	            {
    	                if(g_vbo_recno[m_sChannelInfo[ch_v].ucInputPort] == 4) //H30K_Doulas_0029 8-lane
    	                {
                            if(m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive == 960)
                            {
                                val1 = m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive * 4;
                            }
                            else
                            {
                                val1 = (m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive * 4) - 16;
                            }
    	                }
    	                else if(m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive == 1920)
    	                {
    	                    val1 = m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive * 2;
    	                }
    	                else
    	                {
    	                    val1 = (m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive * 2) - 16;
    	                }

    	                val1 = val1 * FRAME_MEMORY_FORMAT / 30;  //H2PF_Simon_0097

    	                //val1 = m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive * 2;
    	                #ifdef FRAME_SEQUENTAIL_SELF_TEST_120HZ //H30K_Doulas_0005
    	                dvC341_WriteToBuffer(B22_OSFLD0CH1,0x00000000 + val1,ucRegOffset);
                        dvC341_WriteToBuffer(B22_OSFLD1CH1,0x00000000 + val1,ucRegOffset);
    	                dvC341_WriteToBuffer(B22_OSFLD2CH1,0x00a00000 + val1,ucRegOffset);
    	                dvC341_WriteToBuffer(B22_OSFLD3CH1,0x00a00000 + val1,ucRegOffset);
    	                #else
    	                dvC341_WriteToBuffer(B22_OSFLD0CH1,0x00000000 + val1,ucRegOffset);
                        dvC341_WriteToBuffer(B22_OSFLD1CH1,0x00a00000 + val1,ucRegOffset);
    	                dvC341_WriteToBuffer(B22_OSFLD2CH1,0x00000000 + val1,ucRegOffset);
    	                dvC341_WriteToBuffer(B22_OSFLD3CH1,0x00a00000 + val1,ucRegOffset);
    	                #endif
    	            }
    	            dvC341_WriteToBuffer(B22_OSFLD4CH1,0x00000000,ucRegOffset);
                    break;
                #endif

                default:
                if((m_sChannelInfo[eC341_CH_V0].ePanelTimingId == ePANEL_ID_1080P_120HZ) ||
                   (m_sChannelInfo[eC341_CH_V0].ePanelTimingId == ePANEL_ID_WUXGA_120HZ)) //H30K_Doulas_0002 Modify
                {
                    if(ch == 0)
                    {
                        #ifdef FRAME_SEQUENTAIL_SELF_TEST_60HZ //H30K_Doulas_0005
                        #if 1//H30K_Doulas_0034
                        dvC341_WriteToBuffer(B22_OSFLD0CH1,0x01400000,ucRegOffset);
                        dvC341_WriteToBuffer(B22_OSFLD1CH1,0x01e00000,ucRegOffset);
    	                dvC341_WriteToBuffer(B22_OSFLD2CH1,0x00000000,ucRegOffset);
    	                dvC341_WriteToBuffer(B22_OSFLD3CH1,0x00a00000,ucRegOffset);
                        #else
                        dvC341_WriteToBuffer(B22_OSFLD0CH1,0x00000000,ucRegOffset);
                        dvC341_WriteToBuffer(B22_OSFLD1CH1,0x00000000,ucRegOffset);
    	                dvC341_WriteToBuffer(B22_OSFLD2CH1,0x00a00000,ucRegOffset);
    	                dvC341_WriteToBuffer(B22_OSFLD3CH1,0x00a00000,ucRegOffset);
    	                #endif
    	                #else
    	                dvC341_WriteToBuffer(B22_OSFLD0CH1,0x00000000,ucRegOffset);
                        dvC341_WriteToBuffer(B22_OSFLD1CH1,0x00a00000,ucRegOffset);
    	                dvC341_WriteToBuffer(B22_OSFLD2CH1,0x00000000,ucRegOffset);
    	                dvC341_WriteToBuffer(B22_OSFLD3CH1,0x00a00000,ucRegOffset);
    	                #endif
    	            }
    	            else
    	            {
    	            #if 1//H30K_Doulas_0034
                        UINT16	uiHActive;

                        if((g_vbo_recno[m_sChannelInfo[ch_v].ucInputPort] == 14) && // 2-lane & 2CH
                            (m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive < 1920)) //not 4K
                        {
                            uiHActive = m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive * 2;
                        }
                        else if((g_vbo_recno[m_sChannelInfo[ch_v].ucInputPort] == 4)) //H30K_Doulas_0052 240hz
                        {
                            uiHActive = m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive * 2;
                        }
                        else
                        {
                            uiHActive = m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive;
                        }

                        if(uiHActive >= m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiHActive) //H30K_Doulas_0053 m_sChannelInfo[ch_v].sOutputTimingInfo.uiHActive
    	                {
    	                    uiHActive = m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiHActive; //H30K_Doulas_0053 m_sChannelInfo[ch_v].sOutputTimingInfo.uiHActive
    	                    val1 = uiHActive * 2;
    	                }
    	                else
    	                {
    	                    val1 = (uiHActive * 2) - 16;
    	                }

                        val1 = val1 * FRAME_MEMORY_FORMAT / 30;

                        dvC341_WriteToBuffer(B22_OSFLD0CH1,0x01400000 + val1,ucRegOffset);
                        dvC341_WriteToBuffer(B22_OSFLD1CH1,0x01e00000 + val1,ucRegOffset);
    	                dvC341_WriteToBuffer(B22_OSFLD2CH1,0x00000000 + val1,ucRegOffset);
    	                dvC341_WriteToBuffer(B22_OSFLD3CH1,0x00a00000 + val1,ucRegOffset);
                    #else
    	                if(m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive >= 1920)
    	                {
    	                    val1 = m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive * 2;
    	                }
    	                else
    	                {
    	                    val1 = (m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive * 2) - 16;
    	                }

    	                val1 = val1 * FRAME_MEMORY_FORMAT / 30;  //H2PF_Simon_0097

    	                //val1 = m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive * 2;
    	                #ifdef FRAME_SEQUENTAIL_SELF_TEST_60HZ //H30K_Doulas_0005
    	                dvC341_WriteToBuffer(B22_OSFLD0CH1,0x00000000 + val1,ucRegOffset);
                        dvC341_WriteToBuffer(B22_OSFLD1CH1,0x00000000 + val1,ucRegOffset);
    	                dvC341_WriteToBuffer(B22_OSFLD2CH1,0x00a00000 + val1,ucRegOffset);
    	                dvC341_WriteToBuffer(B22_OSFLD3CH1,0x00a00000 + val1,ucRegOffset);
    	                #else
    	                dvC341_WriteToBuffer(B22_OSFLD0CH1,0x00000000 + val1,ucRegOffset);
                        dvC341_WriteToBuffer(B22_OSFLD1CH1,0x00a00000 + val1,ucRegOffset);
    	                dvC341_WriteToBuffer(B22_OSFLD2CH1,0x00000000 + val1,ucRegOffset);
    	                dvC341_WriteToBuffer(B22_OSFLD3CH1,0x00a00000 + val1,ucRegOffset);
    	                #endif
    	            #endif
    	            }
    	            dvC341_WriteToBuffer(B22_OSFLD4CH1,0x00000000,ucRegOffset);
                }
                else
                {
    				#ifdef Low_Latency_All
    				if(m_sChannelSetting[ch_v].ucLow_Latency)
    				{
    					dvC341_WriteToBuffer(B22_OSFLD0CH1,DEF_ISFLD0CH1_4K,ucRegOffset);//if(ROT) PS_MEM_RT[ch][0] //Output field 0-3 memory read start address
    	                dvC341_WriteToBuffer(B22_OSFLD1CH1,DEF_ISFLD1CH1_4K,ucRegOffset);//if(ROT) PS_MEM_RT[ch][1]
    	                dvC341_WriteToBuffer(B22_OSFLD2CH1,DEF_ISFLD2CH1_4K,ucRegOffset);//if(ROT) PS_MEM_RT[ch][2]
    	                dvC341_WriteToBuffer(B22_OSFLD3CH1,DEF_ISFLD3CH1_4K,ucRegOffset);//if(ROT) PS_MEM_RT[ch][3]
    	                dvC341_WriteToBuffer(B22_OSFLD4CH1,DEF_ISFLD4CH1_4K,ucRegOffset);//if(ROT) PS_MEM_RT[ch][3]
    				}
    				else
    				{
    				    #ifdef MODIFY_FOR_C341
    		            dvC341_WriteToBuffer(B22_OSFLD0CH1,DEF_ISFLD0CH1_4K,ucRegOffset);//if(ROT) PS_MEM_RT[ch][0] //Output field 0-3 memory read start address
    		            dvC341_WriteToBuffer(B22_OSFLD1CH1,DEF_ISFLD1CH1_4K,ucRegOffset);//if(ROT) PS_MEM_RT[ch][1]
    		            dvC341_WriteToBuffer(B22_OSFLD2CH1,DEF_ISFLD2CH1_4K,ucRegOffset);//if(ROT) PS_MEM_RT[ch][2]
    		            dvC341_WriteToBuffer(B22_OSFLD3CH1,DEF_ISFLD3CH1_4K,ucRegOffset);//if(ROT) PS_MEM_RT[ch][3]
    		            dvC341_WriteToBuffer(B22_OSFLD4CH1,DEF_ISFLD4CH1_4K,ucRegOffset);//if(ROT) PS_MEM_RT[ch][3]
    		            #else
    		            dvC341_WriteToBuffer(B22_OSFLD0CH1,DEF_ISFLD4CH1_4K,ucRegOffset);//if(ROT) PS_MEM_RT[ch][0] //Output field 0-3 memory read start address
    		            dvC341_WriteToBuffer(B22_OSFLD1CH1,DEF_ISFLD0CH1_4K,ucRegOffset);//if(ROT) PS_MEM_RT[ch][1]
    		            dvC341_WriteToBuffer(B22_OSFLD2CH1,DEF_ISFLD1CH1_4K,ucRegOffset);//if(ROT) PS_MEM_RT[ch][2]
    		            dvC341_WriteToBuffer(B22_OSFLD3CH1,DEF_ISFLD2CH1_4K,ucRegOffset);//if(ROT) PS_MEM_RT[ch][3]
    		            dvC341_WriteToBuffer(B22_OSFLD4CH1,DEF_ISFLD3CH1_4K,ucRegOffset);//if(ROT) PS_MEM_RT[ch][3]
    		            #endif
    				}
    				#else
    					dvC341_WriteToBuffer(B22_OSFLD0CH1,DEF_ISFLD4CH1_4K,ucRegOffset);//if(ROT) PS_MEM_RT[ch][0] //Output field 0-3 memory read start address
    		            dvC341_WriteToBuffer(B22_OSFLD1CH1,DEF_ISFLD0CH1_4K,ucRegOffset);//if(ROT) PS_MEM_RT[ch][1]
    		            dvC341_WriteToBuffer(B22_OSFLD2CH1,DEF_ISFLD1CH1_4K,ucRegOffset);//if(ROT) PS_MEM_RT[ch][2]
    		            dvC341_WriteToBuffer(B22_OSFLD3CH1,DEF_ISFLD2CH1_4K,ucRegOffset);//if(ROT) PS_MEM_RT[ch][3]
    		            dvC341_WriteToBuffer(B22_OSFLD4CH1,DEF_ISFLD3CH1_4K,ucRegOffset);//if(ROT) PS_MEM_RT[ch][3]
    				#endif	/*Low_Latency_All*/
    				val2 = DEF_MWI_4K; //H30K_Doulas_0001
    			}
    	            break;
            }
        }
    }
    else
    {

        for(UINT8 ch=0; ch<OUTPUT4K_CH_NUM; ch++)
        {
            UINT8 BankOffset = CH_BANK_OFFSET;
            ucRegOffset = ((ch_v * 2) + ch) * BankOffset;

            switch(dvC341_Input_3D_Format_Config_Get())        //A70LV_Doulas_0154 modify
            {
                #if 1 //H30K_Doulas_0001
                case eINPUT_3D_TYPE_FRAMEPACKING:
                    if(ch == 0)
                    {
                        dvC341_WriteToBuffer(B22_OSFLD0CH1,0x00000000,ucRegOffset);
                        dvC341_WriteToBuffer(B22_OSFLD1CH1,0x00a00000,ucRegOffset);
    	                dvC341_WriteToBuffer(B22_OSFLD2CH1,0x01400000,ucRegOffset);
    	                dvC341_WriteToBuffer(B22_OSFLD3CH1,0x01e00000,ucRegOffset);
    	            }
    	            else
    	            {
                        if(m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive > 1918 && m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive < 1922)
                        {
                            dvC341_WriteToBuffer(B22_OSFLD0CH1,0x00000f00,ucRegOffset);
                            dvC341_WriteToBuffer(B22_OSFLD1CH1,0x00a00f00,ucRegOffset);
                            dvC341_WriteToBuffer(B22_OSFLD2CH1,0x01400f00,ucRegOffset);
                            dvC341_WriteToBuffer(B22_OSFLD3CH1,0x01e00f00,ucRegOffset);
                        }
                        else
                        {
     	                    dvC341_WriteToBuffer(B22_OSFLD0CH1,0x000009f0,ucRegOffset);
                            dvC341_WriteToBuffer(B22_OSFLD1CH1,0x00a009f0,ucRegOffset);
                            dvC341_WriteToBuffer(B22_OSFLD2CH1,0x014009f0,ucRegOffset);
                            dvC341_WriteToBuffer(B22_OSFLD3CH1,0x01e009f0,ucRegOffset);
    	                }
    	            }
                    break;

                case eINPUT_3D_TYPE_SIDEBYSIDE:
                    if(ch == 0)
                    {
     	                dvC341_WriteToBuffer(B22_OSFLD0CH1,0x00000000,ucRegOffset);
    	                dvC341_WriteToBuffer(B22_OSFLD1CH1,0x00a00000,ucRegOffset);
    	                dvC341_WriteToBuffer(B22_OSFLD2CH1,0x01400000,ucRegOffset);
    	                dvC341_WriteToBuffer(B22_OSFLD3CH1,0x01e00000,ucRegOffset);
    	            }
    	            else
    	            {
                        if(m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive > 1918 && m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive < 1922)
                        {
                            if(m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.eScanMode == eSCAN_MODE_INTERLACE)
                            {
                                dvC341_WriteToBuffer(B22_OSFLD0CH1,0x000004f5,ucRegOffset);
                                dvC341_WriteToBuffer(B22_OSFLD1CH1,0x00a004f5,ucRegOffset);
                                dvC341_WriteToBuffer(B22_OSFLD2CH1,0x014004f5,ucRegOffset);
                                dvC341_WriteToBuffer(B22_OSFLD3CH1,0x01e004f5,ucRegOffset);
                            //    dvC341_WriteToBuffer(B22_OSFLD0CH1,0x00000770,ucRegOffset);
                            //    dvC341_WriteToBuffer(B22_OSFLD1CH1,0x00a00770,ucRegOffset);
                            //    dvC341_WriteToBuffer(B22_OSFLD2CH1,0x01400770,ucRegOffset);
                            //    dvC341_WriteToBuffer(B22_OSFLD3CH1,0x01e00770,ucRegOffset);
                            }
                            else
                            {
                                dvC341_WriteToBuffer(B22_OSFLD0CH1,0x00000770,ucRegOffset);
                                dvC341_WriteToBuffer(B22_OSFLD1CH1,0x00a00770,ucRegOffset);
                                dvC341_WriteToBuffer(B22_OSFLD2CH1,0x01400770,ucRegOffset);
                                dvC341_WriteToBuffer(B22_OSFLD3CH1,0x01e00770,ucRegOffset);
                            }
                        }
                        else //720P to do
                        {
                            dvC341_WriteToBuffer(B22_OSFLD0CH1,0x000004f4,ucRegOffset);
                            dvC341_WriteToBuffer(B22_OSFLD1CH1,0x00a004f4,ucRegOffset);
                            dvC341_WriteToBuffer(B22_OSFLD2CH1,0x014004f4,ucRegOffset);
                            dvC341_WriteToBuffer(B22_OSFLD3CH1,0x01e004f4,ucRegOffset);
                        }
    	            }
                    break;

                case eINPUT_3D_TYPE_TOPANDBOTTOM:
                    if(ch == 0)
                    {
     	                dvC341_WriteToBuffer(B22_OSFLD0CH1,0x00000000,ucRegOffset);
    	                dvC341_WriteToBuffer(B22_OSFLD1CH1,0x00a00000,ucRegOffset);
    	                dvC341_WriteToBuffer(B22_OSFLD2CH1,0x01400000,ucRegOffset);
    	                dvC341_WriteToBuffer(B22_OSFLD3CH1,0x01e00000,ucRegOffset);
    	            }
    	            else
    	            {
    	                if((m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive > 1918 && m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive < 1922) &&
                           (m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.eScanMode == eSCAN_MODE_PROGRESSIVE))
    	                {
    	                    dvC341_WriteToBuffer(B22_OSFLD0CH1,0x00000f00,ucRegOffset);
                            dvC341_WriteToBuffer(B22_OSFLD1CH1,0x00a00f00,ucRegOffset);
                            dvC341_WriteToBuffer(B22_OSFLD2CH1,0x01400f00,ucRegOffset);
                            dvC341_WriteToBuffer(B22_OSFLD3CH1,0x01e00f00,ucRegOffset);
    	                }
    	                else
    	                {
                            dvC341_WriteToBuffer(B22_OSFLD0CH1,0x000009f0,ucRegOffset);
                            dvC341_WriteToBuffer(B22_OSFLD1CH1,0x00a009f0,ucRegOffset);
                            dvC341_WriteToBuffer(B22_OSFLD2CH1,0x014009f0,ucRegOffset);
                            dvC341_WriteToBuffer(B22_OSFLD3CH1,0x01e009f0,ucRegOffset);
                        }
    	            }
                    break;
                #endif

                default:
    				#ifdef Low_Latency_All
    				if(m_sChannelSetting[ch_v].ucLow_Latency)
    				{
    	                dvC341_WriteToBuffer(B22_OSFLD0CH1,DEF_ISFLD0CH2_4K,ucRegOffset);//if(ROT) PS_MEM_RT[ch][0] //Output field 0-3 memory read start address
    	                dvC341_WriteToBuffer(B22_OSFLD1CH1,DEF_ISFLD1CH2_4K,ucRegOffset);//if(ROT) PS_MEM_RT[ch][1]
    	                dvC341_WriteToBuffer(B22_OSFLD2CH1,DEF_ISFLD2CH2_4K,ucRegOffset);//if(ROT) PS_MEM_RT[ch][2]
    	                dvC341_WriteToBuffer(B22_OSFLD3CH1,DEF_ISFLD3CH2_4K,ucRegOffset);//if(ROT) PS_MEM_RT[ch][3]
    	                dvC341_WriteToBuffer(B22_OSFLD4CH1,DEF_ISFLD4CH2_4K,ucRegOffset);//if(ROT) PS_MEM_RT[ch][3]
    	            }
    				else
    	            {
    		            dvC341_WriteToBuffer(B22_OSFLD0CH1,DEF_ISFLD4CH2_4K,ucRegOffset);//if(ROT) PS_MEM_RT[ch][0] //Output field 0-3 memory read start address
    		            dvC341_WriteToBuffer(B22_OSFLD1CH1,DEF_ISFLD0CH2_4K,ucRegOffset);//if(ROT) PS_MEM_RT[ch][1]
    		            dvC341_WriteToBuffer(B22_OSFLD2CH1,DEF_ISFLD1CH2_4K,ucRegOffset);//if(ROT) PS_MEM_RT[ch][2]
    		            dvC341_WriteToBuffer(B22_OSFLD3CH1,DEF_ISFLD2CH2_4K,ucRegOffset);//if(ROT) PS_MEM_RT[ch][3]
    		            dvC341_WriteToBuffer(B22_OSFLD4CH1,DEF_ISFLD3CH2_4K,ucRegOffset);//if(ROT) PS_MEM_RT[ch][3]
    				}
    				#else
    				dvC341_WriteToBuffer(B22_OSFLD0CH1,DEF_ISFLD4CH2_4K,ucRegOffset);//if(ROT) PS_MEM_RT[ch][0] //Output field 0-3 memory read start address
    	            dvC341_WriteToBuffer(B22_OSFLD1CH1,DEF_ISFLD0CH2_4K,ucRegOffset);//if(ROT) PS_MEM_RT[ch][1]
    	            dvC341_WriteToBuffer(B22_OSFLD2CH1,DEF_ISFLD1CH2_4K,ucRegOffset);//if(ROT) PS_MEM_RT[ch][2]
    	            dvC341_WriteToBuffer(B22_OSFLD3CH1,DEF_ISFLD2CH2_4K,ucRegOffset);//if(ROT) PS_MEM_RT[ch][3]
    	            dvC341_WriteToBuffer(B22_OSFLD4CH1,DEF_ISFLD3CH2_4K,ucRegOffset);//if(ROT) PS_MEM_RT[ch][3]
    				#endif	/*Low_Latency_All*/
    				val2 = DEF_MWI_4K; //H30K_Doulas_0001
    	            break;
            }
        }
    }
	//dvC341_WriteToBuffer(B13_OSFLD0CH1,sPS_MEM_SC[ch_v][3]+val1,ucRegOffset);//if(ROT) PS_MEM_RT[ch][0] //Output field 0-3 memory read start address
	//dvC341_WriteToBuffer(B13_OSFLD1CH1,sPS_MEM_SC[ch_v][0]+val1,ucRegOffset);//if(ROT) PS_MEM_RT[ch][1]
	//dvC341_WriteToBuffer(B13_OSFLD2CH1,sPS_MEM_SC[ch_v][1]+val1,ucRegOffset);//if(ROT) PS_MEM_RT[ch][2]
	//dvC341_WriteToBuffer(B13_OSFLD3CH1,sPS_MEM_SC[ch_v][2]+val1,ucRegOffset);//if(ROT) PS_MEM_RT[ch][3]
	dvC341_WriteToBuffer(B22_OMWICH1 ,val2 ,ch_v* 2 * CH_V_BANK_OFFSET); //H30K_Doulas_0001//Output Memory Width
	dvC341_WriteToBuffer(B22_OMWICH1 ,val2 ,ch_v* 2 * CH_V_BANK_OFFSET + CH_BANK_OFFSET); //H30K_Doulas_0001
    dvC341_Buffer_Flush();

 	//halC341_Wait1_POVS();	// 2011/12/20 for rtct
 	dvC341_wait1_povs(ch_v);

//-----------------------------------------------------------------------------
// Flip Control
//-----------------------------------------------------------------------------
	wdt = 0;//PM_FLIP[ch]==1 ? 0x01:0x00;

	dvC341_Write_ch_v(ch_v, B8_OFLPCTCH1, wdt, CH_BANK_OFFSET);	//Horizontal flip control (Rear Projection左右相反)
}

static void dvC341_Init_Output_CSC1(const eC341_CH_V ch_v)
{
    if(ch_v >= eC341_CH_VNUMBER)
    {
        return;
    }

    for(UINT8 ch=0; ch<OUTPUT4K_CH_NUM; ch++)
    {
        UINT8 BankOffset = 16;
        UINT8 ucRegOffset = ((ch_v * 2) + ch) * BankOffset;
        dvC341_WriteToBuffer(B12_OCSC1CF00CH1,	0x3265, ucRegOffset);	//output color conversion coefficient	// Ref.  OCSC1CF/YUV-RGB_709CH1.txt
        dvC341_WriteToBuffer(B12_OCSC1CF01CH1,	0x2000, ucRegOffset);
        dvC341_WriteToBuffer(B12_OCSC1CF02CH1,	0x0000, ucRegOffset);
        dvC341_WriteToBuffer(B12_OCSC1CF10CH1,	0xf105, ucRegOffset);
        dvC341_WriteToBuffer(B12_OCSC1CF11CH1,	0x2000, ucRegOffset);
        dvC341_WriteToBuffer(B12_OCSC1CF12CH1,	0xfa02, ucRegOffset);
        dvC341_WriteToBuffer(B12_OCSC1CF20CH1,	0x0000, ucRegOffset);
        dvC341_WriteToBuffer(B12_OCSC1CF21CH1,	0x2000, ucRegOffset);
        dvC341_WriteToBuffer(B12_OCSC1CF22CH1,	0x3b61, ucRegOffset);
    }

    dvC341_Buffer_Flush();
}

#if 0 //A35G2_CDS_Larry_0030
void dvC341_Warp_LUT_Init(void)
{
    dvC341_Write(B32_HLUT0 ,0x00,0);	//Horizontal interpolation coefficient lookup table
    dvC341_Write(B32_HLUT1 ,0x00,0);
    dvC341_Write(B32_HLUT2,0x00,0);
    dvC341_Write(B32_HLUT3,0x01,0);
    dvC341_Write(B32_HLUT4,0x01,0);
    dvC341_Write(B32_HLUT5,0x01,0);
    dvC341_Write(B32_HLUT6,0x01,0);
    dvC341_Write(B32_HLUT7,0x00,0);
    dvC341_Write(B32_HLUT8,0xff,0);
    dvC341_Write(B32_HLUT9,0xfd,0);
    dvC341_Write(B32_HLUT10,0xfb,0);
    dvC341_Write(B32_HLUT11,0xf9,0);
    dvC341_Write(B32_HLUT12,0xf8,0);
    dvC341_Write(B32_HLUT13,0xf9,0);
    dvC341_Write(B32_HLUT14,0xfc,0);
    dvC341_Write(B32_HLUT15,0x00,0);
    dvC341_Write(B32_HLUT16,0x06,0);
    dvC341_Write(B32_HLUT17,0x0f,0);
    dvC341_Write(B32_HLUT18,0x1a,0);
    dvC341_Write(B32_HLUT19,0x26,0);
    dvC341_Write(B32_HLUT20,0x32,0);
    dvC341_Write(B32_HLUT21,0x3a,0);
    dvC341_Write(B32_HLUT22,0x3e,0);
    dvC341_Write(B32_HLUT23,0x40,0);

    dvC341_Write(B32_VLUT0 ,0x00,0);	//Vertical interpolation coefficient lookup table
    dvC341_Write(B32_VLUT1 ,0x00,0);
    dvC341_Write(B32_VLUT2,0x00,0);
    dvC341_Write(B32_VLUT3,0x01,0);
    dvC341_Write(B32_VLUT4,0x01,0);
    dvC341_Write(B32_VLUT5,0x01,0);
    dvC341_Write(B32_VLUT6,0x01,0);
    dvC341_Write(B32_VLUT7,0x00,0);
    dvC341_Write(B32_VLUT8,0xff,0);
    dvC341_Write(B32_VLUT9,0xfd,0);
    dvC341_Write(B32_VLUT10,0xfb,0);
    dvC341_Write(B32_VLUT11,0xf9,0);
    dvC341_Write(B32_VLUT12,0xf8,0);
    dvC341_Write(B32_VLUT13,0xf9,0);
    dvC341_Write(B32_VLUT14,0xfc,0);
    dvC341_Write(B32_VLUT15,0x00,0);
    dvC341_Write(B32_VLUT16,0x06,0);
    dvC341_Write(B32_VLUT17,0x0f,0);
    dvC341_Write(B32_VLUT18,0x1a,0);
    dvC341_Write(B32_VLUT19,0x26,0);
    dvC341_Write(B32_VLUT20,0x32,0);
    dvC341_Write(B32_VLUT21,0x3a,0);
    dvC341_Write(B32_VLUT22,0x3e,0);
    dvC341_Write(B32_VLUT23,0x40,0);

}
#endif /* 0 */

void dvC341_Set_Diagonal_Interpolation_Control(const eC341_CH_V ch_v)
{
    UINT8 ucRegOffset = 0;

    if(ch_v >= eC341_CH_VNUMBER)
    {
        return;
    }

    //ucPM_DIAG[ch_v] = 0;//default        //A70LV_Doulas_0145 remove

    for(UINT8 ch=0; ch<OUTPUT4K_CH_NUM; ch++)
    {
        UINT8 BankOffset = 16;
        ucRegOffset = ((ch_v * 2) + ch) * BankOffset;

    	if ( ucPM_DIAG[ch_v]==1)     //weak
        {
    		dvC341_WriteToBuffer(B20_DIAGCT0CH1,	0x09,	ucRegOffset);	//Diagonal interpolation control
    		dvC341_WriteToBuffer(B20_DIAGCT1CH1,	0xff,	ucRegOffset);	//Diagonal interpolation control 1
    		dvC341_WriteToBuffer(B20_DIAGCT2CH1,	0x77,	ucRegOffset);	//Diagonal interpolation control 2
    		dvC341_WriteToBuffer(B20_DIAGTH0CH1,	0x00,	ucRegOffset);
    		dvC341_WriteToBuffer(B20_DIAGTH1CH1,	0x04,	ucRegOffset);
    		dvC341_WriteToBuffer(B20_DIAGTH2CH1,	0x10,	ucRegOffset);
    		dvC341_WriteToBuffer(B20_DIAGTH3CH1,	0x03,	ucRegOffset);
    		dvC341_WriteToBuffer(B20_DIAGTH4CH1,	0x08,	ucRegOffset);
    		dvC341_WriteToBuffer(B20_DIAGTH5CH1,	0x0a,	ucRegOffset);
    		dvC341_WriteToBuffer(B20_DIAGTH6CH1,	0x14,	ucRegOffset);
    		dvC341_WriteToBuffer(B20_DIAGTH7CH1,	0x04,	ucRegOffset);
    		dvC341_WriteToBuffer(B20_DIAGTH8CH1,	0x04,	ucRegOffset);
    		dvC341_WriteToBuffer(B20_DIAGTH9CH1,	0x02,	ucRegOffset);
    		dvC341_WriteToBuffer(B20_DIAGTH10CH1,	0x02,	ucRegOffset);
    		dvC341_WriteToBuffer(B20_DIAGTH11CH1,	0x10,	ucRegOffset);
    		dvC341_WriteToBuffer(B20_DIAGTH12CH1,	0x32,	ucRegOffset);
    		dvC341_WriteToBuffer(B20_DIAGAD0CH1,	0x00,	ucRegOffset);
    		dvC341_WriteToBuffer(B20_DIAGAD1CH1,	0x00,	ucRegOffset);
    		dvC341_WriteToBuffer(B20_DIAGAD2CH1,	0x00,	ucRegOffset);
    		dvC341_WriteToBuffer(B20_DIAGAD3CH1,	0x00,	ucRegOffset);
    	}
        else if ( ucPM_DIAG[ch_v]==2)    //mid
        {
    		dvC341_WriteToBuffer(B20_DIAGCT0CH1,	0x0b,	ucRegOffset);
    		dvC341_WriteToBuffer(B20_DIAGCT1CH1,	0x7f,	ucRegOffset);
    		dvC341_WriteToBuffer(B20_DIAGCT2CH1,	0x77,	ucRegOffset);
    		dvC341_WriteToBuffer(B20_DIAGTH0CH1,	0x03,	ucRegOffset);
    		dvC341_WriteToBuffer(B20_DIAGTH1CH1,	0x04,	ucRegOffset);
    		dvC341_WriteToBuffer(B20_DIAGTH2CH1,	0x10,	ucRegOffset);
    		dvC341_WriteToBuffer(B20_DIAGTH3CH1,	0x01,	ucRegOffset);
    		dvC341_WriteToBuffer(B20_DIAGTH4CH1,	0x04,	ucRegOffset);
    		dvC341_WriteToBuffer(B20_DIAGTH5CH1,	0x0a,	ucRegOffset);
    		dvC341_WriteToBuffer(B20_DIAGTH6CH1,	0x14,	ucRegOffset);
    		dvC341_WriteToBuffer(B20_DIAGTH7CH1,	0x04,	ucRegOffset);
    		dvC341_WriteToBuffer(B20_DIAGTH8CH1,	0x04,	ucRegOffset);
    		dvC341_WriteToBuffer(B20_DIAGTH9CH1,	0x02,	ucRegOffset);
    		dvC341_WriteToBuffer(B20_DIAGTH10CH1,	0x02,	ucRegOffset);
    		dvC341_WriteToBuffer(B20_DIAGTH11CH1,	0x10,	ucRegOffset);
    		dvC341_WriteToBuffer(B20_DIAGTH12CH1,	0x32,	ucRegOffset);
    		dvC341_WriteToBuffer(B20_DIAGAD0CH1,	0x00,	ucRegOffset);
    		dvC341_WriteToBuffer(B20_DIAGAD1CH1,	0x00,	ucRegOffset);
    		dvC341_WriteToBuffer(B20_DIAGAD2CH1,	0x00,	ucRegOffset);
    		dvC341_WriteToBuffer(B20_DIAGAD3CH1,	0x00,	ucRegOffset);
    	}
        else if ( ucPM_DIAG[ch_v]==3)    //strong
        {
    		dvC341_WriteToBuffer(B20_DIAGCT0CH1,	0x0b,	ucRegOffset);
    		dvC341_WriteToBuffer(B20_DIAGCT1CH1,	0x7f,	ucRegOffset);
    		dvC341_WriteToBuffer(B20_DIAGCT2CH1,	0x77,	ucRegOffset);
    		dvC341_WriteToBuffer(B20_DIAGTH0CH1,	0x13,	ucRegOffset);
    		dvC341_WriteToBuffer(B20_DIAGTH1CH1,	0x09,	ucRegOffset);
    		dvC341_WriteToBuffer(B20_DIAGTH2CH1,	0x10,	ucRegOffset);
    		dvC341_WriteToBuffer(B20_DIAGTH3CH1,	0x03,	ucRegOffset);
    		dvC341_WriteToBuffer(B20_DIAGTH4CH1,	0x08,	ucRegOffset);
    		dvC341_WriteToBuffer(B20_DIAGTH5CH1,	0x0a,	ucRegOffset);
    		dvC341_WriteToBuffer(B20_DIAGTH6CH1,	0x14,	ucRegOffset);
    		dvC341_WriteToBuffer(B20_DIAGTH7CH1,	0x04,	ucRegOffset);
    		dvC341_WriteToBuffer(B20_DIAGTH8CH1,	0x04,	ucRegOffset);
    		dvC341_WriteToBuffer(B20_DIAGTH9CH1,	0x02,	ucRegOffset);
    		dvC341_WriteToBuffer(B20_DIAGTH10CH1,	0x02,	ucRegOffset);
    		dvC341_WriteToBuffer(B20_DIAGTH11CH1,	0x10,	ucRegOffset);
    		dvC341_WriteToBuffer(B20_DIAGTH12CH1,	0x31,	ucRegOffset);
    		dvC341_WriteToBuffer(B20_DIAGAD0CH1,	0x00,	ucRegOffset);
    		dvC341_WriteToBuffer(B20_DIAGAD1CH1,	0x00,	ucRegOffset);
    		dvC341_WriteToBuffer(B20_DIAGAD2CH1,	0x00,	ucRegOffset);
    		dvC341_WriteToBuffer(B20_DIAGAD3CH1,	0x00,	ucRegOffset);
    	}
        else
    	{
    		dvC341_WriteToBuffer(B20_DIAGCT0CH1,	0x00,	ucRegOffset);
    		dvC341_WriteToBuffer(B20_DIAGCT1CH1,	0x00,	ucRegOffset);
    		dvC341_WriteToBuffer(B20_DIAGCT2CH1,	0x00,	ucRegOffset);
    		dvC341_WriteToBuffer(B20_DIAGTH0CH1,	0x00,	ucRegOffset);
    		dvC341_WriteToBuffer(B20_DIAGTH1CH1,	0x00,	ucRegOffset);
    		dvC341_WriteToBuffer(B20_DIAGTH2CH1,	0x00,	ucRegOffset);
    		dvC341_WriteToBuffer(B20_DIAGTH3CH1,	0x00,	ucRegOffset);
    		dvC341_WriteToBuffer(B20_DIAGTH4CH1,	0x00,	ucRegOffset);
    		dvC341_WriteToBuffer(B20_DIAGTH5CH1,	0x00,	ucRegOffset);
    		dvC341_WriteToBuffer(B20_DIAGTH6CH1,	0x00,	ucRegOffset);
    		dvC341_WriteToBuffer(B20_DIAGTH7CH1,	0x00,	ucRegOffset);
    		dvC341_WriteToBuffer(B20_DIAGTH8CH1,	0x00,	ucRegOffset);
    		dvC341_WriteToBuffer(B20_DIAGTH9CH1,	0x00,	ucRegOffset);
    		dvC341_WriteToBuffer(B20_DIAGTH10CH1,	0x00,	ucRegOffset);
    		dvC341_WriteToBuffer(B20_DIAGTH11CH1,	0x00,	ucRegOffset);
    		dvC341_WriteToBuffer(B20_DIAGTH12CH1,	0x00,	ucRegOffset);
    	}
    }

    dvC341_Buffer_Flush();
}

static void dvC341_Sharpness_Init(const eC341_CH_V ch_v)
{
    UINT8 ucRegOffset = 0;

    if(ch_v >= eC341_CH_VNUMBER)
    {
        return;
    }

    for(UINT8 ch=0; ch<OUTPUT4K_CH_NUM; ch++)
    {
        UINT8 BankOffset = 16;
        ucRegOffset = ((ch_v * 2) + ch) * BankOffset;
        dvC341_WriteToBuffer(B11_HEGGAINCH1,    0x00, ucRegOffset);	//	Sharpness H
        dvC341_WriteToBuffer(B11_HEGCRLLCH1,    0xc5, ucRegOffset);
        dvC341_WriteToBuffer(B11_HEGCRULCH1,    0xc8, ucRegOffset);
        dvC341_WriteToBuffer(B11_HEGCOEF0CH1,   0x32, ucRegOffset);
        dvC341_WriteToBuffer(B11_HEGCOEF1CH1,   0x37, ucRegOffset);
        dvC341_WriteToBuffer(B11_HEGCOEF2CH1,   0x3a, ucRegOffset);
        dvC341_WriteToBuffer(B11_HEGCOEF3CH1,   0x3d, ucRegOffset);

		dvC341_WriteToBuffer(B11_VEGSELCH1,     0x01, ucRegOffset);
        dvC341_WriteToBuffer(B11_VEGGAINCH1,    0x00, ucRegOffset);	//	Sharpness V
        dvC341_WriteToBuffer(B11_VEGCRLLCH1,    0xc5, ucRegOffset);
        dvC341_WriteToBuffer(B11_VEGCRULCH1,    0xc8, ucRegOffset);
        dvC341_WriteToBuffer(B11_VEGCOEF0CH1,   0x32, ucRegOffset);
        dvC341_WriteToBuffer(B11_VEGCOEF1CH1,   0x37, ucRegOffset);
        dvC341_WriteToBuffer(B11_VEGCOEF2CH1,   0x3a, ucRegOffset);
        dvC341_WriteToBuffer(B11_VEGCOEF3CH1,   0x3d, ucRegOffset);
    }

    dvC341_Buffer_Flush();
}

static void dvC341_Init_Other_Registers(void)   //A70LV_Doulas_0003
{
    UINT8 ucRegOffset = 0;
    UINT8 BankOffset = 16;
    eC341_CH_V ch_v;

    for(UINT8 ch=0 ; ch < CHANNEL_MAX ; ch++)
    {
        dvC341_Write(B8_FLDDLYCH1, 0x0040, ch*BankOffset);   //simon check
    }

	dvC341_Write(B0_OCFMTCH1,		0x11,0);	//Output color format control  //simon check  //OCSC2 is disable   //C341 sample code 0xb1
	dvC341_Write(B0_OCFMTCH2,		0x11,0);
	dvC341_Write(B0_OCFMTCH3,		0x11,0);
	dvC341_Write(B0_OCFMTCH4,		0x11,0);

	dvC341_Write(B5_INTEN_0, 0x00, 0);			//Interrupt enable
	dvC341_Write(B5_INTEN_1, 0x00, 0);
	dvC341_Write(B5_INTEN_2, 0x00, 0);
	dvC341_Write(B5_INTEN_3, 0x00, 0);
	dvC341_Write(B5_INTEN_4, 0x00, 0);
	dvC341_Write(B5_INTEN_5, 0x00, 0);
	dvC341_Write(B5_INTEN_6, 0x00, 0);
	dvC341_Write(B5_INTEN_7, 0x00, 0);
	dvC341_Write(B5_INTEN_8, 0x00, 0);
	dvC341_Write(B5_INTEN_9, 0x00, 0);
	dvC341_Write(B5_INTEN_10, 0x00, 0);
	dvC341_Write(B5_INTEN_11, 0x00, 0);
	dvC341_Write(B5_INTEN_12, 0x00, 0);
	//dvC341_Write(B3_BLINKTIME1CH1,0x30,0);	//Blinking time //A70LV_Larry_0001
	//dvC341_Write(B3_BLINKCYCL1CH1,0x60,0);	//Blink cycle //A70LV_Larry_0001

    #if 0  //remove by simon
    for(UINT8 ch=0 ; ch < CHANNEL_MAX ; ch++)
    {
        dvC341_Write(B22_MRTOMWICH1,	0x0A, ch*BankOffset); //A70LV_Doulas_0009  //Image 90-degree rotation memory read linefeed width
        dvC341_Write(B22_MRTIMWICH1,	0x0A, ch*BankOffset); //A70LV_Doulas_0009  //Image 90-degree rotation memory write linefeed width

        dvC341_Write(B16_ACTCTCH1,	    0x80, ch*BankOffset); //A70LV_Doulas_0267 Horizontal filter select B
    }
    #endif

}

static void dvC341_Init_Default_Value(const eC341_CH_V ch_v)
{
    dvC341_Init_Output_CSC1(ch_v);
  //  dvC341_SetOutput_BiasGain1_Enable(eCH,1);     //A70LV_Doulas_0002 test
  //  dvC341_SetOutput_BiasGain2_Enable(eCH,0);     //A70LV_Doulas_0002 test
    dvC341_Sharpness_Init(ch_v);
    dvC341_Init_Other_Registers();  //A70LV_Doulas_0003
}

static void dvC341_ScalerFillWidth(UINT16 uiXO,UINT16 uiXI,const eC341_CH_V ch_v)
{
    UINT16 uiVW_OUT;
    UINT16 uiPixels;

    uiVW_OUT = (UINT16)((UINT32)m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiVActive *(UINT32)uiXO /(UINT32)uiXI);
    uiPixels = (UINT16)((UINT32)(uiVW_OUT - m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiVActive)*(UINT32)uiXI/(UINT32)uiXO);

    m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiVStart += uiPixels/2;
    m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiVActive -= uiPixels;
}

static void dvC341_ScalerFillHeight(UINT16 uiYO,UINT16 uiYI,const eC341_CH_V ch_v)
{
    UINT16 uiHW_OUT;
    UINT16 uiPixels;

    uiHW_OUT = (UINT16)((UINT32)m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiHActive * (UINT32)uiYO / (UINT32)uiYI);
    uiPixels = (UINT16) ((UINT32)(uiHW_OUT - m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiHActive)*(UINT32)uiYI /(UINT32)uiYO);

    m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiHStart += uiPixels/2;
    m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiHActive -= uiPixels;
}

static void dvC341_ScalerOneToOne(const eC341_CH_V ch_v)
{
    //Horizontal
    if(m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiHActive >= m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiHActive)
    {
        m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiHStart += (m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiHActive - m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiHActive)/2;
        m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiHActive = m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiHActive;
    }
    else
    {
        m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiHStart += (m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiHActive - m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiHActive )/2;
        m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiHActive = m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiHActive;
    }

    //Vert
    if(m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiVActive >= m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiVActive)
    {
        m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiVStart += (m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiVActive - m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiVActive)/2;
        m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiVActive = m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiVActive;
    }
    else
    {
        m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiVStart += (m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiVActive - m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiVActive )/2;
        m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiVActive = m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiVActive;
    }
}

static void dvC341_Calc_FillToThisAspect(UINT16 uiX,UINT16 uiY,const eC341_CH_V ch_v)
{
    //Aspect Ratio = uiX/uiY
    UINT16 uiVW,uiHW;

    if((m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiVActive == 0) || (uiY == 0) || (uiX == 0))
    {
        return;
    }

    if(((DOUBLE)uiX * 10000/(DOUBLE)uiY ) >
       ((DOUBLE)m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiHActive *10000/(DOUBLE)m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiVActive))
    {
        uiVW = (UINT16)((UINT32)m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiHActive * (UINT32)uiY / (UINT32)uiX);
        m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiVStart += (m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiVActive - uiVW )/2;
        m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiVActive = uiVW;

    }
    else
    {
        uiHW = (UINT16)((UINT32)m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiVActive *(UINT32)uiX /(UINT32)uiY);
        m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiHStart += (m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiHActive - uiHW )/2;
        m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiHActive = uiHW;

    }
}

static void dvC341_ClipPercentageSet(const eC341_CH_V ch_v,INT16 nHClipPercentage,INT16 nVClipPercentage)
{
    //nVOffsetPct 250 == 25%
    UINT16 uiOffset_X = 0;
    UINT16 uiOffset_Y = 0;

    // For negative modify the scaler output rectangle
    if(nHClipPercentage < 0)
    {
        nHClipPercentage = -nHClipPercentage;
        uiOffset_X = (UINT16)((UINT32)m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiHActive *
                        (UINT32)nHClipPercentage / (UINT32)1000 / 2);

        m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiHStart += uiOffset_X;
        m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiHActive -= (uiOffset_X*2);
    }
    else // otherwise modify the scaler input rectangle
    {
        uiOffset_X = (UINT16)((UINT32)m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiHActive *
                        (UINT32)nHClipPercentage / (UINT32)1000 / 2);

        m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiHStart += uiOffset_X;
        m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiHActive -= (uiOffset_X*2);
    }

    // For negative modify the scaler output rectangle
    if(nVClipPercentage < 0)
    {
        nVClipPercentage = -nVClipPercentage;
        uiOffset_Y = (UINT16)((UINT32)m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiVActive *
                        (UINT32)nVClipPercentage / (UINT32)1000 / 2);

        m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiVStart += uiOffset_Y;
        m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiVActive -= (uiOffset_Y*2);
    }
    else // otherwise modify the scaler input rectangle
    {
        uiOffset_Y = (UINT16)((UINT32)m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiVActive *
                        (UINT32)nVClipPercentage / (UINT32)1000 / 2);

        m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiVStart += uiOffset_Y;
        m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiVActive -= (uiOffset_Y*2);
    }
}

static void dvC341_Calc_Scaler_PIP_Window(const eC341_CH_V ch_v)   //A70LV_Doulas_0003
{
    UINT16 uiPIP_Offset = 5;
    UINT16 uiHActive = m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiHActive;
    UINT16 uiVActive = m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiVActive;
    UINT16 uiHStart  = m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiHStart;
    UINT16 uiVStart  = m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiVStart;

    if(ch_v == eC341_CH_V1) //only sub
    {
        //cal H/V size
        switch(m_sChannelSetting[eC341_CH_V0].cPIP_Size)
        {
		#ifdef CUSTOM_OPTOMA
			case eCM_PIP_SIZE_LARGE:
                uiHActive = uiHActive /3;
                uiVActive = uiVActive /3;
                break;

            case eCM_PIP_SIZE_MEDIUM:
                uiHActive = uiHActive /4;
                uiVActive = uiVActive /4;
                break;

            case eCM_PIP_SIZE_SMALL:
                uiHActive = uiHActive /5;
                uiVActive = uiVActive /5;
                break;
		#else
            case eCM_PIP_SIZE_LARGE:
                uiHActive = uiHActive /4;
                uiVActive = uiVActive /4;
                break;

            case eCM_PIP_SIZE_MEDIUM:
                uiHActive = uiHActive /5;
                uiVActive = uiVActive /5;
                break;

            case eCM_PIP_SIZE_SMALL:
                uiHActive = uiHActive /6;
                uiVActive = uiVActive /6;
                break;
		#endif
            default:
                break;
        }

        //cal H/V start
        switch(m_sChannelSetting[eC341_CH_V0].cMain_Layout)
        {
            case eCM_MAIN_LAYOUT_PIP_BOTTOM_RIGHT:
                uiHStart = uiHStart + m_sChannelInfo[ch_v].sOutputTimingInfo.uiHActive - uiHActive - uiPIP_Offset;
                uiVStart = uiVStart + m_sChannelInfo[ch_v].sOutputTimingInfo.uiVActive - uiVActive - uiPIP_Offset;
                break;

            case eCM_MAIN_LAYOUT_PIP_BOTTOM_LEFT:
                uiHStart = uiHStart + uiPIP_Offset;
                uiVStart = uiVStart + m_sChannelInfo[ch_v].sOutputTimingInfo.uiVActive - uiVActive - uiPIP_Offset;
                break;

            case eCM_MAIN_LAYOUT_PIP_TOP_LEFT:
                uiHStart = uiHStart + uiPIP_Offset;
                uiVStart = uiVStart + uiPIP_Offset;
                break;

            case eCM_MAIN_LAYOUT_PIP_TOP_RIGHT:
                uiHStart = uiHStart + m_sChannelInfo[ch_v].sOutputTimingInfo.uiHActive - uiHActive - uiPIP_Offset;
                uiVStart = uiVStart + uiPIP_Offset;
                break;

            default:
                break;
        }
        LOG_MSG(db_DV_SCALER, "(func:%s, line:%d) dvC341_Calc_Scaler_PIP_Size[%d]()(%dx%d)\r\n", __FUNCTION__, __LINE__,
            ch_v,uiHStart,uiVStart,uiHActive,uiVActive);

        m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiHActive = uiHActive;
        m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiVActive = uiVActive;
        m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiHStart  = uiHStart;
        m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiVStart  = uiVStart;
    }
    //ZU860_Clare_0111, modify, >>>
	else
	{
		if((m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.eScanMode == eSCAN_MODE_INTERLACE)&&
		   (m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive == 1440))	//A70LV_Doulas_0002
		{
			m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiHActive = m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiHActive/2;
			m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiHStart  = m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiHStart/2;
		}

		dvC341_Calc_FillToThisAspect(m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiHActive,
                                         m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiVActive,ch_v);
	    //back h-active and h-start
	    if((m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.eScanMode == eSCAN_MODE_INTERLACE)&&
	       (m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive == 1440))    //A70LV_Doulas_0002
	    {
	        m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiHActive = m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiHActive*2;
	        m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiHStart  = (m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiHStart*2)
	                                                                +(uiHStart%2);     //A70LV_Doulas_0005 modify
	    }
	}
	//ZU860_Clare_0111, modify, <<<
}

static void dvC341_Calc_Scaler_PBP_Window(const eC341_CH_V ch_v)   //A70LV_Doulas_0003
{
    UINT16 uiHActive = m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiHActive;
    UINT16 uiVActive = m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiVActive;
    UINT16 uiHStart  = m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiHStart;
    UINT16 uiVStart  = m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiVStart;
    UINT16 PLeft = 0, PRight = 0, PTop = 0, PDown = 0;
    UINT16 uiBig_Size = 0, uiSmall_Size = 0;

    if(ch_v == eC341_CH_V0)
    {
        switch(m_sChannelSetting[eC341_CH_V0].cPIP_Size)
        {
            case eCM_PIP_SIZE_LARGE:
                switch(m_sChannelSetting[eC341_CH_V0].cMain_Layout)
                {
                    case eCM_MAIN_LAYOUT_PBP_MAIN_LEFT:
                    PLeft = 0;
                    PTop = ((uiVActive / (PBP_LARGE_MAIN_FACTOR + PBP_LARGE_SUB_FACTOR)) * PBP_LARGE_SUB_FACTOR) / 2 ;
                    PRight = (uiHActive / (PBP_LARGE_MAIN_FACTOR + PBP_LARGE_SUB_FACTOR)) * PBP_LARGE_MAIN_FACTOR;
                    PDown = ((uiVActive / (PBP_LARGE_MAIN_FACTOR + PBP_LARGE_SUB_FACTOR)) * PBP_LARGE_SUB_FACTOR) / 2 + ((uiVActive / (PBP_LARGE_MAIN_FACTOR + PBP_LARGE_SUB_FACTOR)) * PBP_LARGE_MAIN_FACTOR);
                    break;

                    case eCM_MAIN_LAYOUT_PBP_MAIN_RIGHT:
                    PLeft = uiHActive / (PBP_LARGE_MAIN_FACTOR + PBP_LARGE_SUB_FACTOR) * PBP_LARGE_SUB_FACTOR;
                    PTop = ((uiVActive / (PBP_LARGE_MAIN_FACTOR + PBP_LARGE_SUB_FACTOR)) * PBP_LARGE_SUB_FACTOR) / 2 ;
                    PRight = (uiHActive);
                    PDown = ((uiVActive / (PBP_LARGE_MAIN_FACTOR + PBP_LARGE_SUB_FACTOR)) * PBP_LARGE_SUB_FACTOR) / 2 + ((uiVActive / (PBP_LARGE_MAIN_FACTOR + PBP_LARGE_SUB_FACTOR)) * PBP_LARGE_MAIN_FACTOR);
                    break;

                    case eCM_MAIN_LAYOUT_PBP_MAIN_TOP:  // 1/2
                    uiBig_Size = PBP_LARG_BIG_SIZE;
                    uiSmall_Size = PBP_LARG_SMALL_SIZE;
                    PLeft = ((uiHActive / uiBig_Size) * (uiBig_Size - uiSmall_Size)) / 2;
                    PTop = 0 ;
                    PRight = PLeft + ((uiHActive / uiBig_Size) * uiSmall_Size);
                    PDown = (uiVActive / uiBig_Size) * uiSmall_Size ;
                    break;

                    case eCM_MAIN_LAYOUT_PBP_MAIN_BOTTOM:   // 1/2
                    uiBig_Size = PBP_LARG_BIG_SIZE;
                    uiSmall_Size = PBP_LARG_SMALL_SIZE;
                    PLeft = ((uiHActive / uiBig_Size) * (uiBig_Size - uiSmall_Size)) / 2;
                    PRight = PLeft + ((uiHActive / uiBig_Size) * uiSmall_Size);
                    PTop = (uiVActive / uiBig_Size) * (uiBig_Size - uiSmall_Size);
                    PDown = (uiVActive);
                    break;
                }
                break;

            case eCM_PIP_SIZE_MEDIUM:
                switch(m_sChannelSetting[eC341_CH_V0].cMain_Layout)
                {
                    case eCM_MAIN_LAYOUT_PBP_MAIN_LEFT:
                    PLeft = 0;
                    PTop = ((uiVActive / (PBP_MEDIUM_MAIN_FACTOR + PBP_MEDIUM_SUB_FACTOR)) * PBP_MEDIUM_SUB_FACTOR) / 2 ;
                    PRight = (uiHActive / (PBP_MEDIUM_MAIN_FACTOR + PBP_MEDIUM_SUB_FACTOR)) * PBP_MEDIUM_MAIN_FACTOR;
                    PDown = ((uiVActive / (PBP_MEDIUM_MAIN_FACTOR + PBP_MEDIUM_SUB_FACTOR)) * PBP_MEDIUM_SUB_FACTOR) / 2 + (uiVActive / (PBP_MEDIUM_MAIN_FACTOR + PBP_MEDIUM_SUB_FACTOR)) * PBP_MEDIUM_MAIN_FACTOR;
                    break;

                    case eCM_MAIN_LAYOUT_PBP_MAIN_RIGHT:
                    PLeft = uiHActive / (PBP_MEDIUM_MAIN_FACTOR + PBP_MEDIUM_SUB_FACTOR) * PBP_MEDIUM_SUB_FACTOR;
                    PTop = ((uiVActive / (PBP_MEDIUM_MAIN_FACTOR + PBP_MEDIUM_SUB_FACTOR)) * PBP_MEDIUM_SUB_FACTOR) / 2 ;
                    PRight = (uiHActive);
                    PDown = ((uiVActive / (PBP_MEDIUM_MAIN_FACTOR + PBP_MEDIUM_SUB_FACTOR)) * PBP_MEDIUM_SUB_FACTOR) / 2 + (uiVActive / (PBP_MEDIUM_MAIN_FACTOR + PBP_MEDIUM_SUB_FACTOR)) * PBP_MEDIUM_MAIN_FACTOR;
                    break;

                    case eCM_MAIN_LAYOUT_PBP_MAIN_TOP:  // 3/5
                    uiBig_Size = PBP_MEDIUM_BIG_SIZE;
                    uiSmall_Size = PBP_MEDIUM_SMALL_SIZE;
                    PLeft = ((uiHActive / uiBig_Size) * (uiBig_Size - uiSmall_Size)) / 2;
                    PTop = 0 ;
                    PRight = PLeft + ((uiHActive / uiBig_Size) * uiSmall_Size);
                    PDown = (uiVActive / uiBig_Size) * uiSmall_Size ;
                    break;

                    case eCM_MAIN_LAYOUT_PBP_MAIN_BOTTOM:   // 3/5
                    uiBig_Size = PBP_MEDIUM_BIG_SIZE;
                    uiSmall_Size = PBP_MEDIUM_SMALL_SIZE;
                    PLeft = ((uiHActive / uiBig_Size) * (uiBig_Size - uiSmall_Size)) / 2;
                    PRight = PLeft + ((uiHActive / uiBig_Size) * uiSmall_Size);
                    PTop = (uiVActive / uiBig_Size) * (uiBig_Size - uiSmall_Size);
                    PDown = (uiVActive);
                    break;
                }
                break;

            case eCM_PIP_SIZE_SMALL:
                switch(m_sChannelSetting[eC341_CH_V0].cMain_Layout)
                {
                    case eCM_MAIN_LAYOUT_PBP_MAIN_LEFT:
                    PLeft = 0;
                    PTop = ((uiVActive / (PBP_SMALL_MAIN_FACTOR + PBP_SMALL_SUB_FACTOR)) * PBP_SMALL_SUB_FACTOR) / 2 ;
                    PRight = (uiHActive / (PBP_SMALL_MAIN_FACTOR + PBP_SMALL_SUB_FACTOR)) * PBP_SMALL_MAIN_FACTOR;
                    PDown = ((uiVActive / (PBP_SMALL_MAIN_FACTOR + PBP_SMALL_SUB_FACTOR)) * PBP_SMALL_SUB_FACTOR) / 2 + (uiVActive / (PBP_SMALL_MAIN_FACTOR + PBP_SMALL_SUB_FACTOR)) * PBP_SMALL_MAIN_FACTOR;
                    break;

                    case eCM_MAIN_LAYOUT_PBP_MAIN_RIGHT:
                    PLeft = uiHActive / (PBP_SMALL_MAIN_FACTOR + PBP_SMALL_SUB_FACTOR) * PBP_SMALL_SUB_FACTOR;
                    PTop = ((uiVActive / (PBP_SMALL_MAIN_FACTOR + PBP_SMALL_SUB_FACTOR)) * PBP_SMALL_SUB_FACTOR) / 2 ;
                    PRight = (uiHActive);
                    PDown = ((uiVActive / (PBP_SMALL_MAIN_FACTOR + PBP_SMALL_SUB_FACTOR)) * PBP_SMALL_SUB_FACTOR) / 2 + (uiVActive / (PBP_SMALL_MAIN_FACTOR + PBP_SMALL_SUB_FACTOR)) * PBP_SMALL_MAIN_FACTOR;
                    break;

                    case eCM_MAIN_LAYOUT_PBP_MAIN_TOP:  // 2/3
                    uiBig_Size = PBP_SMALL_BIG_SIZE;
                    uiSmall_Size = PBP_SMALL_SMALL_SIZE;
                    PLeft = ((uiHActive / uiBig_Size) * (uiBig_Size - uiSmall_Size)) / 2;
                    PTop = 0 ;
                    PRight = PLeft + ((uiHActive / uiBig_Size) * uiSmall_Size);
                    PDown = (uiVActive / uiBig_Size) * uiSmall_Size ;
                    break;

                    case eCM_MAIN_LAYOUT_PBP_MAIN_BOTTOM:   // 2/3
                    uiBig_Size = PBP_SMALL_BIG_SIZE;
                    uiSmall_Size = PBP_SMALL_SMALL_SIZE;
                    PLeft = ((uiHActive / uiBig_Size) * (uiBig_Size - uiSmall_Size)) / 2;
                    PRight = PLeft + ((uiHActive / uiBig_Size) * uiSmall_Size);
                    PTop = (uiVActive / uiBig_Size) * (uiBig_Size - uiSmall_Size);
                    PDown = (uiVActive);
                    break;
                }
                break;

            default:
                break;
        }
    }
    else
    {
        switch(m_sChannelSetting[eC341_CH_V0].cPIP_Size)
        {
            case eCM_PIP_SIZE_LARGE:
                switch(m_sChannelSetting[eC341_CH_V0].cMain_Layout)
                {
                    case eCM_MAIN_LAYOUT_PBP_MAIN_LEFT:
                    PLeft = (uiHActive / (PBP_LARGE_MAIN_FACTOR + PBP_LARGE_SUB_FACTOR)) * PBP_LARGE_MAIN_FACTOR;
                    PTop = ((uiVActive / (PBP_LARGE_MAIN_FACTOR + PBP_LARGE_SUB_FACTOR)) * PBP_LARGE_MAIN_FACTOR) / 2 ;
                    PRight =  uiHActive;
                    PDown = ((uiVActive / (PBP_LARGE_MAIN_FACTOR + PBP_LARGE_SUB_FACTOR)) * PBP_LARGE_MAIN_FACTOR) / 2 + (uiVActive / (PBP_LARGE_MAIN_FACTOR + PBP_LARGE_SUB_FACTOR) * PBP_LARGE_SUB_FACTOR);
                    break;

                    case eCM_MAIN_LAYOUT_PBP_MAIN_RIGHT:
                    PLeft = 0;
                    PTop = ((uiVActive / (PBP_LARGE_MAIN_FACTOR + PBP_LARGE_SUB_FACTOR)) * PBP_LARGE_MAIN_FACTOR) / 2 ;
                    PRight =  uiHActive / (PBP_LARGE_MAIN_FACTOR + PBP_LARGE_SUB_FACTOR) * PBP_LARGE_SUB_FACTOR;
                    PDown = ((uiVActive / (PBP_LARGE_MAIN_FACTOR + PBP_LARGE_SUB_FACTOR)) * PBP_LARGE_MAIN_FACTOR) / 2 + (uiVActive / (PBP_LARGE_MAIN_FACTOR + PBP_LARGE_SUB_FACTOR) * PBP_LARGE_SUB_FACTOR);
                    break;

                    case eCM_MAIN_LAYOUT_PBP_MAIN_TOP:  // 1/2
                    uiBig_Size = PBP_LARG_BIG_SIZE;
                    uiSmall_Size = PBP_LARG_SMALL_SIZE;
                    PLeft = ((uiHActive / uiBig_Size) * uiSmall_Size) / 2;
                    PRight = PLeft + (uiHActive / uiBig_Size) * (uiBig_Size - uiSmall_Size);
                    PTop = ((uiVActive / uiBig_Size) * uiSmall_Size) ;
                    PDown = (uiVActive) ;
                    break;

                    case eCM_MAIN_LAYOUT_PBP_MAIN_BOTTOM:   // 1/2
                    uiBig_Size = PBP_LARG_BIG_SIZE;
                    uiSmall_Size = PBP_LARG_SMALL_SIZE;
                    PLeft = ((uiHActive / uiBig_Size) * uiSmall_Size) / 2;
                    PRight = PLeft + (uiHActive / uiBig_Size) * (uiBig_Size - uiSmall_Size);
                    PTop = 0;
                    PDown = (uiVActive / uiBig_Size) * (uiBig_Size - uiSmall_Size);
                    break;
                }
                break;

            case eCM_PIP_SIZE_MEDIUM:
                switch(m_sChannelSetting[eC341_CH_V0].cMain_Layout)
                {
                    case eCM_MAIN_LAYOUT_PBP_MAIN_LEFT:
                    PLeft = (uiHActive / (PBP_MEDIUM_MAIN_FACTOR + PBP_MEDIUM_SUB_FACTOR)) * PBP_MEDIUM_MAIN_FACTOR;
                    PTop = ((uiVActive / (PBP_MEDIUM_MAIN_FACTOR + PBP_MEDIUM_SUB_FACTOR)) * PBP_MEDIUM_MAIN_FACTOR) / 2 ;
                    PRight =  uiHActive;
                    PDown = ((uiVActive / (PBP_MEDIUM_MAIN_FACTOR + PBP_MEDIUM_SUB_FACTOR)) * PBP_MEDIUM_MAIN_FACTOR) / 2 + (uiVActive / (PBP_MEDIUM_MAIN_FACTOR + PBP_MEDIUM_SUB_FACTOR) * PBP_MEDIUM_SUB_FACTOR) ;
                    break;

                    case eCM_MAIN_LAYOUT_PBP_MAIN_RIGHT:
                    PLeft = 0;
                    PTop = ((uiVActive / (PBP_MEDIUM_MAIN_FACTOR + PBP_MEDIUM_SUB_FACTOR)) * PBP_MEDIUM_MAIN_FACTOR) / 2 ;
                    PRight =  uiHActive / (PBP_MEDIUM_MAIN_FACTOR + PBP_MEDIUM_SUB_FACTOR) * PBP_MEDIUM_SUB_FACTOR;
                    PDown = ((uiVActive / (PBP_MEDIUM_MAIN_FACTOR + PBP_MEDIUM_SUB_FACTOR)) * PBP_MEDIUM_MAIN_FACTOR) / 2 + (uiVActive / (PBP_MEDIUM_MAIN_FACTOR + PBP_MEDIUM_SUB_FACTOR) * PBP_MEDIUM_SUB_FACTOR);
                    break;

                    case eCM_MAIN_LAYOUT_PBP_MAIN_TOP:  // 2/5
                    uiBig_Size = PBP_MEDIUM_BIG_SIZE;
                    uiSmall_Size = PBP_MEDIUM_SMALL_SIZE;
                    PLeft = ((uiHActive / uiBig_Size) * uiSmall_Size) / 2;
                    PRight = PLeft + (uiHActive / uiBig_Size) * (uiBig_Size - uiSmall_Size);
                    PTop = ((uiVActive / uiBig_Size) * uiSmall_Size) ;
                    PDown = (uiVActive) ;
                    break;

                    case eCM_MAIN_LAYOUT_PBP_MAIN_BOTTOM:   // 2/5
                    uiBig_Size = PBP_MEDIUM_BIG_SIZE;
                    uiSmall_Size = PBP_MEDIUM_SMALL_SIZE;
                    PLeft = ((uiHActive / uiBig_Size) * uiSmall_Size) / 2;
                    PRight = PLeft + (uiHActive / uiBig_Size) * (uiBig_Size - uiSmall_Size);
                    PTop = 0;
                    PDown = (uiVActive / uiBig_Size) * (uiBig_Size - uiSmall_Size);
                    break;
                }
                break;

            case eCM_PIP_SIZE_SMALL:
                switch(m_sChannelSetting[eC341_CH_V0].cMain_Layout)
                {
                    case eCM_MAIN_LAYOUT_PBP_MAIN_LEFT:
                    PLeft = (uiHActive / (PBP_SMALL_MAIN_FACTOR + PBP_SMALL_SUB_FACTOR)) * PBP_SMALL_MAIN_FACTOR;
                    PTop = ((uiVActive / (PBP_SMALL_MAIN_FACTOR + PBP_SMALL_SUB_FACTOR)) * PBP_SMALL_MAIN_FACTOR) / 2 ;
                    PRight =  uiHActive;
                    PDown = ((uiVActive / (PBP_SMALL_MAIN_FACTOR + PBP_SMALL_SUB_FACTOR)) * PBP_SMALL_MAIN_FACTOR) / 2 + (uiVActive / (PBP_SMALL_MAIN_FACTOR + PBP_SMALL_SUB_FACTOR) * PBP_SMALL_SUB_FACTOR) ;
                    break;

                    case eCM_MAIN_LAYOUT_PBP_MAIN_RIGHT:
                    PLeft = 0;
                    PTop = ((uiVActive / (PBP_SMALL_MAIN_FACTOR + PBP_SMALL_SUB_FACTOR)) * PBP_SMALL_MAIN_FACTOR) / 2 ;
                    PRight =  uiHActive / (PBP_SMALL_MAIN_FACTOR + PBP_SMALL_SUB_FACTOR) * PBP_SMALL_SUB_FACTOR;
                    PDown = ((uiVActive / (PBP_SMALL_MAIN_FACTOR + PBP_SMALL_SUB_FACTOR)) * PBP_SMALL_MAIN_FACTOR) / 2 + (uiVActive / (PBP_SMALL_MAIN_FACTOR + PBP_SMALL_SUB_FACTOR) * PBP_SMALL_SUB_FACTOR) ;
                    break;

                    case eCM_MAIN_LAYOUT_PBP_MAIN_TOP:  // 1/3
                    uiBig_Size = PBP_SMALL_BIG_SIZE;
                    uiSmall_Size = PBP_SMALL_SMALL_SIZE;
                    PLeft = ((uiHActive / uiBig_Size) * uiSmall_Size) / 2;
                    PRight = PLeft + (uiHActive / uiBig_Size) * (uiBig_Size - uiSmall_Size);
                    PTop = ((uiVActive / uiBig_Size) * uiSmall_Size) ;
                    PDown = (uiVActive) ;
                    break;

                    case eCM_MAIN_LAYOUT_PBP_MAIN_BOTTOM:   // 1/3
                    uiBig_Size = PBP_SMALL_BIG_SIZE;
                    uiSmall_Size = PBP_SMALL_SMALL_SIZE;
                    PLeft = ((uiHActive / uiBig_Size) * uiSmall_Size) / 2;
                    PRight = PLeft + (uiHActive / uiBig_Size) * (uiBig_Size - uiSmall_Size);
                    PTop = 0;
                    PDown = (uiVActive / uiBig_Size) * (uiBig_Size - uiSmall_Size);
                    break;
                }
                break;

            default:
                break;
        }
    }
    LOG_MSG(db_DV_SCALER, "(func:%s, line:%d) dvC341_Calc_Scaler_PBP_Size[%d]()(%dx%d)\r\n", __FUNCTION__, __LINE__,
            ch_v,uiHStart + PLeft,uiVStart + PTop,PRight - PLeft,PDown - PTop);

    m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiHActive = PRight - PLeft;
    m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiVActive = PDown - PTop;
    m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiHStart  = uiHStart + PLeft;
    m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiVStart  = uiVStart + PTop;
}

static void dvC341_Calc_Scaler_PIP_PBP_Window_4K(const eC341_CH_V ch_v)    //A70LV_Doulas_0003
{
    switch(m_sChannelSetting[eC341_CH_V0].cMain_Layout)
    {
        case eCM_MAIN_LAYOUT_PBP_MAIN_LEFT:
        case eCM_MAIN_LAYOUT_PBP_MAIN_TOP:
        case eCM_MAIN_LAYOUT_PBP_MAIN_RIGHT:
        case eCM_MAIN_LAYOUT_PBP_MAIN_BOTTOM:
            dvC341_Calc_Scaler_PBP_Window(ch_v);
            break;

        case eCM_MAIN_LAYOUT_PIP_BOTTOM_RIGHT:
        case eCM_MAIN_LAYOUT_PIP_BOTTOM_LEFT:
        case eCM_MAIN_LAYOUT_PIP_TOP_LEFT:
        case eCM_MAIN_LAYOUT_PIP_TOP_RIGHT:
            dvC341_Calc_Scaler_PIP_Window(ch_v);
            break;

        default:
            break;
    }
}



// Horizontal resolution of the input image written to memory.
GlobalVar int G_INPUT_IMAGE_HW[eC341_CH_VNUMBER];
// Vertical resolution of the input image written to memory.
GlobalVar int G_INPUT_IMAGE_VW[eC341_CH_VNUMBER];

static void dvC341_Calc_Scaler_InputAndOutput_Window_4K(const eC341_CH_V ch_v)
{
    UINT16 uiHStart,uiVStart;        //A70LV_Doulas_0005
    eCM_SCALING_MODE_ID eAspectRatio;     //A70LV_Doulas_0098
    UINT16 uiV_Total = 0; //H30K_Doulas_0001
    if(ch_v >= eC341_CH_VNUMBER)
    {
        return;
    }

    uiHStart = m_sChannelInfo[ch_v].sInputTimingInfo.uiHStart;
    uiVStart = m_sChannelInfo[ch_v].sInputTimingInfo.uiVStart;
    LOG_MSG(db_DV_SCALER, "Start22 (%d,%d) eAspectRatio %d  \r\n", uiHStart,uiVStart,m_sChannelSetting[ch_v].eScalingMode);       //A70LV_Doulas_0175 debug


    if (m_sChannelInfo[ch_v].sInputTimingInfo.lSTATE == eINPUT_PORT_INFO_STATE_NA) // No signal
	{
		G_INPUT_IMAGE_HW[ch_v] = 0;
		G_INPUT_IMAGE_VW[ch_v] = 0;
		uiV_Total = 0; //H30K_Doulas_0001
	}
	else if (m_sChannelInfo[ch_v].sInputTimingInfo.lSTATE < eINPUT_PORT_INFO_STATE_1CH) // 4k input
	{
		G_INPUT_IMAGE_HW[ch_v] = m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive * 2;
		G_INPUT_IMAGE_VW[ch_v] = m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive;
		uiV_Total = m_sChannelInfo[ch_v].sInputTimingInfo.uiVTotal; //H30K_Doulas_0001
	}
	else if (m_sChannelInfo[ch_v].sInputTimingInfo.eScanMode == eSCAN_MODE_INTERLACE)
	{
		G_INPUT_IMAGE_HW[ch_v] = m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive;
		if (ch_v < 2)
		{
			G_INPUT_IMAGE_VW[ch_v] = m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive * 2;
			uiV_Total = m_sChannelInfo[ch_v].sInputTimingInfo.uiVTotal * 2; //H30K_Doulas_0001
		}
		else
		{
			G_INPUT_IMAGE_VW[ch_v] = m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive;
			uiV_Total = m_sChannelInfo[ch_v].sInputTimingInfo.uiVTotal; //H30K_Doulas_0001
		}
	}
	else
	{
		G_INPUT_IMAGE_HW[ch_v] = m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive;
		G_INPUT_IMAGE_VW[ch_v] = m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive;
		uiV_Total = m_sChannelInfo[ch_v].sInputTimingInfo.uiVTotal; //H30K_Doulas_0001
	}



    //Input
    m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiHStart  = 0;
    m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiHActive = G_INPUT_IMAGE_HW[ch_v];
    m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiVStart  = 0;
    m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiVActive = G_INPUT_IMAGE_VW[ch_v];

    //m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiHTotal = 0;  //unused
    //m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiVTotal = 0;  //unused

    m_sChannelInfo[ch_v].sScaler_InputTimingInfo.dFrameRate = m_sChannelInfo[ch_v].sInputTimingInfo.dFrameRate;
    m_sChannelInfo[ch_v].sScaler_InputTimingInfo.eScanMode = m_sChannelInfo[ch_v].sInputTimingInfo.eScanMode;

    //Output
    m_sChannelInfo[ch_v].sScalerOutputTimingInfo.ucFrameRate  = m_sChannelInfo[ch_v].sOutputTimingInfo.ucFrameRate;
    m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiPixClk     = m_sChannelInfo[ch_v].sOutputTimingInfo.uiPixClk;
    m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiHStart     = m_sChannelInfo[ch_v].sOutputTimingInfo.uiHStart;
    m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiHActive    = m_sChannelInfo[ch_v].sOutputTimingInfo.uiHActive;
    m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiVStart     = m_sChannelInfo[ch_v].sOutputTimingInfo.uiVStart;
    m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiVActive    = m_sChannelInfo[ch_v].sOutputTimingInfo.uiVActive;
    m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiHTotal     = m_sChannelInfo[ch_v].sOutputTimingInfo.uiHTotal;   //unused
    m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiVTotal     = m_sChannelInfo[ch_v].sOutputTimingInfo.uiVTotal;   //unused
    m_sChannelInfo[ch_v].sScalerOutputTimingInfo.ucHSyncWidth = m_sChannelInfo[ch_v].sOutputTimingInfo.ucHSyncWidth;
    m_sChannelInfo[ch_v].sScalerOutputTimingInfo.ucVSyncWidth = m_sChannelInfo[ch_v].sOutputTimingInfo.ucVSyncWidth;

    m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiHActive = m_sChannelInfo[ch_v].sOutputTimingInfo.uiHActive;
    m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiVActive = m_sChannelInfo[ch_v].sOutputTimingInfo.uiVActive;
    m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiHStart = 0;
    m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiVStart = 0;

    #if 1//H30K_Doulas_0001 //simon check .. no need?
    switch(dvC341_Input_3D_Format_Config_Get())        //A70LV_Doulas_0154
    {
        case eINPUT_3D_TYPE_FRAMEPACKING:
            if(ch_v == eC341_CH_V0)
            {
                if(uiV_Total > 2240)
                {
                    m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiVActive = 1080;
                }
                else
                {
                    m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiVActive = 720;
                }
				LOG_MSG(db_DV_SCALER, "Vtotal(0) %d\r\n", uiV_Total);	//A70Gen2_Doulas_0007
            }
            else
            {
                if(uiV_Total > 2240)
                {
                    m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiVActive = 1080;
                //    m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiVStart += (m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiVTotal/2);
                }
                else
                {
                    m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiVActive = 720;
                //    m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiVStart += (m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiVTotal/2);
                }
				LOG_MSG(db_DV_SCALER, "Vtotal(1) (%d,%d)\r\n", uiV_Total,
														       uiV_Total);		//A70Gen2_Doulas_0007
            }
            break;

        case eINPUT_3D_TYPE_SIDEBYSIDE:
            if(ch_v == eC341_CH_V0)
            {
                m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiHActive /= 2;
            }
            else
            {
                m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiHActive /= 2;
            //    m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiHStart += m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiHActive;
            }
            break;

        case eINPUT_3D_TYPE_TOPANDBOTTOM:
            if(ch_v == eC341_CH_V0)
            {
                m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiVActive = m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiVActive/2;
            }
            else
            {
                m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiVActive = m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiVActive/2;
            //    m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiVStart += m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiVActive;
            }
            break;

        case eINPUT_3D_TYPE_FRAME_SEQUENTIAL:
            break;

        default:
            break;
    }
    #endif

    if(m_sChannelSetting[eC341_CH_V0].cPIP_PBP_ON == TRUE)     //A70LV_Doulas_0003
    {
        dvC341_Calc_Scaler_PIP_PBP_Window_4K(ch_v);
        LOG_MSG(db_DV_SCALER, "(func:%s, line:%d):(%d) I(%d,%d)(%d,%d)O(%d,%d)(%d,%d)\r\n", __FUNCTION__, __LINE__, m_sChannelSetting[ch_v].eScalingMode,
        m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiHStart,
        m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiVStart,
        m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiHActive,
        m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiVActive,
        m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiHStart,
        m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiVStart,
        m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiHActive,
        m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiVActive );    //A70LV_Doulas_0042
        return;
    }


    if((m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.eScanMode == eSCAN_MODE_INTERLACE)&&
       (m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive == 1440))    //A70LV_Doulas_0002
    {
        m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiHActive = m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiHActive/2;
        m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiHStart  = m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiHStart/2;
    }
    //p_sChannelInfo = &m_sChannelInfo[ch_v];

    eAspectRatio = m_sChannelSetting[ch_v].eScalingMode;     //A70LV_Doulas_0098
    if((m_sChannelInfo[eC341_CH_V0].ePanelTimingId == ePANEL_ID_1080P_120HZ) ||
        (m_sChannelInfo[eC341_CH_V0].ePanelTimingId == ePANEL_ID_WUXGA_120HZ) ||   //A70LV_Doulas_0377 Add
        (m_sChannelInfo[eC341_CH_V0].ePanelTimingId == ePANEL_ID_1080P_240HZ) || //H30K_Doulas_0009
        (m_sChannelInfo[eC341_CH_V0].ePanelTimingId == ePANEL_ID_WUXGA_240HZ) || //H30K_Doulas_0009
       (dvC341_AspectRatioIs3DModeGet() == TRUE))     //A70LV_Doulas_0154 modify //A70LV_Doulas_0098
    {
        eAspectRatio = eCM_SCALING_MODE_3D;
    }

    //Aspect Ratio
    switch(eAspectRatio) //A70LV_Doulas_0098 modify//A70LV_Doulas_0013
    {
        default:
        case eCM_SCALING_MODE_AUTO:
            dvC341_Calc_FillToThisAspect(m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiHActive,
                                         m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiVActive,ch_v);
            break;

        case eCM_SCALING_MODE_CUSTOM:
        case eCM_SCALING_MODE_NATIVE:
            dvC341_ScalerOneToOne(ch_v);
            break;

        case eCM_SCALING_MODE_LETTER_BOX:
            dvC341_ClipPercentageSet(ch_v,
                                     0,     // No horizontal clip
                                     250);  // Clip 25% vertically (12.5% top and // bottom)
            break;

        case eCM_SCALING_MODE_FULL_SIZE:
        //case eSCALING_MODE_3D:    //ZU860_Doulas_0119 remove
            //do nothing...
            break;

        case eCM_SCALING_MODE_3D:      //A70LV_Doulas_0336 Modify
#ifdef AUTO_ASPECT_RATIO_WITH_3D    //H30K_Doulas_0053 //A70LV_Doulas_0377 Modify
            switch(dvC341_Input_3D_Format_Config_Get())
            {
                case eINPUT_3D_TYPE_FRAMEPACKING:
                    break;

                case eINPUT_3D_TYPE_SIDEBYSIDE:
                    m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiHActive = m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiHActive * 2;
                    break;

                case eINPUT_3D_TYPE_TOPANDBOTTOM:
                    m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiVActive = m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiVActive * 2;
                    break;

                case eINPUT_3D_TYPE_FRAME_SEQUENTIAL:
                    break;

                default:
                    break;
            }

            dvC341_Calc_FillToThisAspect(m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiHActive,
                                         m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiVActive,ch_v);

            switch(dvC341_Input_3D_Format_Config_Get())
            {
                case eINPUT_3D_TYPE_FRAMEPACKING:
                    break;

                case eINPUT_3D_TYPE_SIDEBYSIDE:
                    m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiHActive /= 2;
                    break;

                case eINPUT_3D_TYPE_TOPANDBOTTOM:
                    m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiVActive = m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiVActive/2;
                    break;

                case eINPUT_3D_TYPE_FRAME_SEQUENTIAL:
                    break;

                default:
                    break;
            }
#endif
            break;

        case eCM_SCALING_MODE_FULL_WIDTH:
            {
                UINT32 ulny;

                if(m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiHActive == 0)
                {
                    return;
                }

                ulny = (UINT32)m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiVActive *
                       (UINT32)m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiHActive /
                       (UINT32)m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiHActive;

                if(ulny  > (UINT32)m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiVActive)
                {
                    dvC341_ScalerFillWidth(m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiHActive,m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiHActive,ch_v);
                }
                else
                {
                    dvC341_Calc_FillToThisAspect(m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiHActive,
                                            m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiVActive,ch_v);
                }
            }
            break;

        case eCM_SCALING_MODE_FULL_HEIGHT:
            {
                UINT32 ulnx;

                ulnx = (UINT32)m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiHActive *
                       (UINT32)m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiVActive /
                       (UINT32)m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiVActive;

                if(ulnx > (UINT32)m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiHActive)
                {
                    dvC341_ScalerFillHeight(m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiVActive,m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiVActive,ch_v);
                }
                else
                {
                    dvC341_Calc_FillToThisAspect(m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiHActive,
                                            m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiVActive,ch_v);
                }
            }
            break;

        case eCM_SCALING_MODE_4_3:
            dvC341_Calc_FillToThisAspect(400,300,ch_v);
            break;

        case eCM_SCALING_MODE_16_9:
            dvC341_Calc_FillToThisAspect(160,90,ch_v);
            break;

        case eCM_SCALING_MODE_16_10:
            dvC341_Calc_FillToThisAspect(160,100,ch_v);
            break;

        case eCM_SCALING_MODE_21_9:
            dvC341_Calc_FillToThisAspect(210,90,ch_v); //HICC2_AC_0012
            break;

    }

    LOG_MSG(db_DV_SCALER, "[1] (func:%s, line:%d):(%d) I(%d,%d)(%dx%d)O(%d,%d)(%dx%d)\r\n", __FUNCTION__, __LINE__, m_sChannelSetting[ch_v].eScalingMode,
        m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiHStart,
        m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiVStart,
        m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiHActive,
        m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiVActive,
        m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiHStart,
        m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiVStart,
        m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiHActive,
        m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiVActive );

    if(dvC341_AspectRatioIs3DModeGet() == eINPUT_3D_TYPE_OFF)        //A70LV_Doulas_0154
    {
        if((m_sChannelSetting[ch_v].uiDigitalHorzZoom == DIGITAL_HORZ_ZOOM_DEFAULT) &&
           (m_sChannelSetting[ch_v].uiDigitalVertZoom == DIGITAL_VERT_ZOOM_DEFAULT))      //A70LV_Doulas_0009 Add Over Scan and Digital Zoom
        {
            //Over Scan
            dvC341_Scale_Overscan(m_sChannelSetting[ch_v].eOverScan,
                                 &m_sChannelInfo[ch_v].sScaler_InputTimingInfo,
                                 &m_sChannelInfo[ch_v].sScalerOutputTimingInfo);

            //Edge Mask
            dvC341_Scale_EdgeMask(m_sChannelSetting[ch_v].cEdgeMask,
                                 &m_sChannelInfo[ch_v].sScaler_InputTimingInfo,
                                 &m_sChannelInfo[ch_v].sScalerOutputTimingInfo);      //ZU860_Doulas_0004
        }
        else
        {
            //Digital Zoom
            if(m_sChannelSetting[ch_v].eScalingMode == eCM_SCALING_MODE_CUSTOM)    //A70LV_Doulas_0013 modify
            {
                dvC341_Scale_Digital_Zoom_Custom(&m_sChannelInfo[ch_v].sScaler_InputTimingInfo,
                                                 &m_sChannelInfo[ch_v].sScalerOutputTimingInfo);
            }
            else
            {
                dvC341_Scale_Digital_Zoom(&m_sChannelInfo[ch_v].sScaler_InputTimingInfo,
                                          &m_sChannelInfo[ch_v].sScalerOutputTimingInfo);
            }
        }

        LOG_MSG(db_DV_SCALER, "[2] (func:%s, line:%d):(%d) I(%d,%d)(%dx%d)O(%d,%d)(%dx%d)\r\n", __FUNCTION__, __LINE__, m_sChannelSetting[ch_v].eScalingMode,
            m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiHStart,
            m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiVStart,
            m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiHActive,
            m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiVActive,
            m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiHStart,
            m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiVStart,
            m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiHActive,
            m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiVActive );

        //Calc Start Position
        //dvC341_Start_Position_Set(ch_v,
        //                      &m_sChannelInfo[ch_v].sScaler_InputTimingInfo,
        //                      &m_sChannelInfo[ch_v].sScalerOutputTimingInfo);    //H30K_Doulas_0041//A70LV_Doulas_0013 //A70LV_Doulas_0010
    }       //A70LV_Doulas_0155 modify


    LOG_MSG(db_DV_SCALER, "[3] (func:%s, line:%d):(%d) I(%d,%d)(%dx%d)O(%d,%d)(%dx%d)\r\n", __FUNCTION__, __LINE__, m_sChannelSetting[ch_v].eScalingMode,
        m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiHStart,
        m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiVStart,
        m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiHActive,
        m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiVActive,
        m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiHStart,
        m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiVStart,
        m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiHActive,
        m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiVActive );

    //if(m_sChannelSetting[ch_v].eInputSource == eCM_SOURCE_VGA) //H30K_Doulas_0041
    {
        dvC341_Scale_Setting_Check(ch_v,&m_sChannelInfo[ch_v].sScaler_InputTimingInfo,
                                        &m_sChannelInfo[ch_v].sScalerOutputTimingInfo);   //A70LV_Doulas_0010 Add
    }

    LOG_MSG(db_DV_SCALER, "[4] (func:%s, line:%d):(%d) I(%d,%d)(%dx%d)O(%d,%d)(%dx%d)\r\n", __FUNCTION__, __LINE__, m_sChannelSetting[ch_v].eScalingMode,
        m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiHStart,
        m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiVStart,
        m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiHActive,
        m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiVActive,
        m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiHStart,
        m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiVStart,
        m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiHActive,
        m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiVActive );    //A70LV_Doulas_0002 debu

    //back h-active and h-start
    if((m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.eScanMode == eSCAN_MODE_INTERLACE)&&
       (m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive == 1440))    //A70LV_Doulas_0002
    {
        m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiHActive = m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiHActive*2;
        m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiHStart  = (m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiHStart*2)
                                                                +(uiHStart%2);     //A70LV_Doulas_0005 modify
    }
}


#define HZMD 0
#define VZMD 0
static void dvC341_Set_ZoomShrink_4K(const eC341_CH_V ch_v)
{
	int IWIN_SHRNK_HW;
	int IWIN_SHRNK_VW;

	UINT16 PL_IWIN_HST = m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiHStart;
    UINT16 PL_IWIN_HW  = m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiHActive;
    UINT16 PL_IWIN_VST = m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiVStart;
    UINT16 PL_IWIN_VW  = m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiVActive;
	UINT16 PL_OWIN_HST = m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiHStart;
	UINT16 PL_OWIN_HW  = m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiHActive;
	UINT16 PL_OWIN_VST = m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiVStart;
	UINT16 PL_OWIN_VW  = m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiVActive;
	sIMAGE_WINDOW PL_IWIN = {PL_IWIN_HST, PL_IWIN_HW, PL_IWIN_VST, PL_IWIN_VW};
	sIMAGE_WINDOW PL_OWIN = {PL_OWIN_HST, PL_OWIN_HW, PL_OWIN_VST, PL_OWIN_VW};

    dbmsg("\r\n\r\nch_v = %d",ch_v);
	dbmsg("PL_IWIN (%d %d) (%dx%d)\r\n", PL_IWIN_HST, PL_IWIN_VST, PL_IWIN_HW, PL_IWIN_VW);
	dbmsg("PL_OWIN (%d %d) (%dx%d)\r\n", PL_OWIN_HST, PL_OWIN_VST, PL_OWIN_HW, PL_OWIN_VW);
	dbmsg("PL_IWIN (0x%02X 0x%02X) (0x%02X x 0x%02X)\r\n", PL_IWIN_HST, PL_IWIN_VST, PL_IWIN_HW, PL_IWIN_VW);
	dbmsg("PL_OWIN (0x%02X 0x%02X) (0x%02X x 0x%02X)\r\n", PL_OWIN_HST, PL_OWIN_VST, PL_OWIN_HW, PL_OWIN_VW);

	if(PL_IWIN_HW == 0 || PL_IWIN_VW == 0 || PL_OWIN_HW == 0 || PL_OWIN_VW == 0)   //H2PF_Simon_0131
	{
	    return ;
	}

	UINT16 OutputPanel_HW       = m_sChannelInfo[eC341_CH_V0].sOutputTimingInfo.uiHActive;
	UINT16 OutputPanel_HW_1CH   = m_sChannelInfo[eC341_CH_V0].sOutputTimingInfo.uiHActive / OUTPUT4K_CH_NUM;
	UINT16 OutputPanel_HST      = m_sChannelInfo[eC341_CH_V0].sOutputTimingInfo.uiHStart;
	UINT16 OutputPanel_HST_1CH  = m_sChannelInfo[eC341_CH_V0].sOutputTimingInfo.uiHStart / OUTPUT4K_CH_NUM;

	UINT16 OutputPanel_VW       = m_sChannelInfo[eC341_CH_V0].sOutputTimingInfo.uiVActive;
	UINT16 OutputPanel_VST      = m_sChannelInfo[eC341_CH_V0].sOutputTimingInfo.uiVStart - 2 ;

	int REG_HSSCL, REG_VSSCL;
	int REG_HZSCL, REG_VZSCL;
	int REG_HZINIT[2];
	int REG_OACTHST[2], REG_OACTHW[2];
	int REG_OACTVST, REG_OACTVW;
	int REG_IACTHST[2], REG_IACTHW[2];
	int REG_IACTVST[2], REG_IACTVW[2];    //H30K_Doulas_0001
	int REG_HSHRNKCT, REG_VSHRNKCT, REG_HZOOMCT, REG_VZOOMCT;
	int REG_VZMADJCT;
	int REG_OAOI1HST[2], REG_OAOI1HEND[2], REG_OAOI1VST, REG_OAOI1VEND;
	unsigned int rdata;

	double scale;
	int val;
	int hsscl, vsscl;
	int stspch2;
	int edspch2;
	int stspch2mod;
	int hsinit0ch2;
	int hsinit1ch2;
	int hsinitch2;
	int hfstvldch2;
	int stmhstch2;
	int stmhstch2_tmp;
	int adoffsetch1, adoffsetch2;
	int iacthwch1_tmp;
	int iacthwch2_offset;

	int stsp_v;
	int endpmod_v;
	int hsinitch1;
	int hfstvldch1;
	int osfld_ofst_ihst;
	int vsinit;
	int vfstvld;
	int osfld_ofst_ivst;

	int	isfldofst_20bitfmt;
	int	m20bitfmt_hstofst;

	int hzscl, hbofst_edge, hbofst_zoom, ohdst, ohdw;
	int hdw[2], ohst[2], hstbofst_e[2], ohst_eofst[2], ihst_eofst[2];
	int hstbofst_z[2], ihst_ofst[2], hzstofst[2], ihstsub_ofst[2];
	int hzinit[2];
	int ohend[2], hendbofst_e[2], ohend_eofst[2], ihend_eofst[2];
	int ihend_ofst[2], ohend_ofst[2], hzendofst[2], ihw[2];
	int vzscl, vbofst_edge, vbofst_zoom;
	int vdw1, ovst1, vbofst1, ovst1_ofst, ivst1, ivstsub1, vstofst1, ivst1_ofst, vzstofst1;
	int ivstsub1_ofst, vzinit1, ovend1, ovend1_ofst, ovw1;
	int osfld_ofst[2];
	int mhst_ofst[2];
	int	stmhaatch2mod5, stmhaatch2mod3, stmhaatch2rem3;
	int mhst1_ofst_div5[2],mhst1div3[2],mhst1rem3[2];

	int vzoom_ofst_vst;
	int vzoom_ofst_vw;

	int ibank_offset, obank_offset;

	int wait_loop;
	//int ipen = ((check_nr(ch_v) != 0) | (PI_SCAN_MODE[ch_v] != 0)) ? 1 : 0;   //simon check
	int ipen = (m_sChannelSetting[ch_v].eScanMode != eSCAN_MODE_PROGRESSIVE) ? 1 : 0;

	int	yuvrestart = 0;
	int	Input_HW[2];
	int	Input_HST[2];

	ibank_offset = 16 * 2 * ch_v;
	obank_offset = 16 * 2 * ch_v;

	// System param
	hbofst_edge = 4;// Horizontal edge enhancer = 9symbol// PS_HBOFST;
	hbofst_zoom = (HZMD == 0) ? 3 : 1;
	vbofst_edge = 2;// Vertical edge enhancer = 5symbol// PS_HBOFST;
	vbofst_zoom = (VZMD == 0) ? 3 : 1;
	vzoom_ofst_vst = (VZMD == 0) ? 4 : 2;
	vzoom_ofst_vw = (VZMD == 0) ? -3 : -1;
	//PM_SCFMFMTENB[ch_v] = 0;
	//PS_SCFMFMT[ch_v] = 0 ;

    UINT32 PS_MEM_SC[5] = {0};

    if(ch_v == eC341_CH_V0)
    {
        PS_MEM_SC[0] = DEF_ISFLD0CH1_4K;
        PS_MEM_SC[1] = DEF_ISFLD1CH1_4K;
        PS_MEM_SC[2] = DEF_ISFLD2CH1_4K;
        PS_MEM_SC[3] = DEF_ISFLD3CH1_4K;
        PS_MEM_SC[4] = DEF_ISFLD4CH1_4K;
    }
    else
    {
        PS_MEM_SC[0] = DEF_ISFLD0CH2_4K;
        PS_MEM_SC[1] = DEF_ISFLD1CH2_4K;
        PS_MEM_SC[2] = DEF_ISFLD2CH2_4K;
        PS_MEM_SC[3] = DEF_ISFLD3CH2_4K;
        PS_MEM_SC[4] = DEF_ISFLD4CH2_4K;
    }

	m_sChannelInfo[ch_v].ucSCFMFMTENB = 0;
	m_sChannelInfo[ch_v].ucSCFMFMT = 0;

	// Shrink
	hsscl = ((PL_IWIN.HW <= PL_OWIN.HW) | (PL_OWIN.HW < 1) | (PL_IWIN.HW < 2)) ? 65536
		: (65536 * (PL_OWIN.HW - 1)) / (PL_IWIN.HW - 1) + 1;
	vsscl = ((PL_IWIN.VW <= PL_OWIN.VW) | (PL_OWIN.VW < 1) | (PL_IWIN.VW < 2)) ? 65536
		: (65536 * (PL_OWIN.VW - 1)) / (PL_IWIN.VW - 1) + 1;


	if (m_sChannelInfo[ch_v].sInputTimingInfo.lSTATE <= eINPUT_PORT_INFO_STATE_4CH) // 4k input
	{
		Input_HW[0] = m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive;  // IFACTHWCH1
		Input_HW[1] = m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive * 2 - Input_HW[0];

		Input_HST[0] = m_sChannelInfo[ch_v].sInputTimingInfo.uiHStart + (m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive - Input_HW[0]) - 7;													// B172_IFACTHSTCH1
		Input_HST[1] = m_sChannelInfo[ch_v].sInputTimingInfo.uiHStart - 7;
	}
	else
	{
		Input_HW[0]  = m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive;
		Input_HW[1]  = 0;
		Input_HST[0] = m_sChannelInfo[ch_v].sInputTimingInfo.uiHStart - 7;
		Input_HST[1] = 0;
	}

	// IACT
	REG_IACTVST[0]    = m_sChannelInfo[ch_v].sInputTimingInfo.uiVStart + 1;
	REG_IACTVW[0]     = m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive;

	REG_IACTVST[1]    = m_sChannelInfo[ch_v].sInputTimingInfo.uiVStart + 1; //H30K_Doulas_0001
	REG_IACTVW[1]     = m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive;    //H30K_Doulas_0001

	REG_IACTHST[0] = Input_HST[0];
	REG_IACTHW[0]  = Input_HW[0];

	REG_IACTHST[1] = Input_HST[1];
	REG_IACTHW[1]  = Input_HW[1];

	switch(dvC341_Input_3D_Format_Config_Get()) //H30K_Doulas_0001
	{
        case eINPUT_3D_TYPE_FRAMEPACKING:
            //if(m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiVTotal > 2240)
            {
                REG_IACTVW[0] = m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiVActive;
                REG_IACTVW[1] = m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiVActive;
                REG_IACTVST[1] = REG_IACTVST[0] + m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive - REG_IACTVW[0];

                REG_IACTHST[1] = Input_HST[0];
	            REG_IACTHW[1]  = Input_HW[0];
            }
            break;

        case eINPUT_3D_TYPE_TOPANDBOTTOM:
            REG_IACTVW[0] = REG_IACTVW[0]/2;
            REG_IACTVW[1] = REG_IACTVW[1]/2;
            REG_IACTVST[1] = REG_IACTVST[0] + REG_IACTVW[0];
            REG_IACTHST[1] = Input_HST[0];
	        REG_IACTHW[1]  = Input_HW[0];
            break;

        case eINPUT_3D_TYPE_SIDEBYSIDE:
            //if(m_sChannelSetting[ch_v].eScanMode == eSCAN_MODE_INTERLACE)
            {
                REG_IACTHW[0]  = Input_HW[0]/2;
                REG_IACTHST[1] = REG_IACTHST[0]+REG_IACTHW[0];
                REG_IACTHW[1]  = Input_HW[0]/2;
            }
            break;

        case eINPUT_3D_TYPE_FRAME_SEQUENTIAL:
            if((m_sChannelInfo[eC341_CH_V0].ePanelTimingId == ePANEL_ID_1080P_120HZ) ||
               (m_sChannelInfo[eC341_CH_V0].ePanelTimingId == ePANEL_ID_WUXGA_120HZ)) //H30K_Doulas_0009
            {
                REG_IACTHST[1] = Input_HST[0];
	            REG_IACTHW[1]  = Input_HW[0];
	        }
            break;

        default:
            if((m_sChannelInfo[eC341_CH_V0].ePanelTimingId == ePANEL_ID_1080P_120HZ) ||
               (m_sChannelInfo[eC341_CH_V0].ePanelTimingId == ePANEL_ID_WUXGA_120HZ)) //H30K_Doulas_0002
            {
                REG_IACTHST[1] = Input_HST[0];
	            REG_IACTHW[1]  = Input_HW[0];
            }
            else if((ch_v == eC341_CH_V0) &&
                    (m_sChannelSetting[eC341_CH_V0].cPIP_PBP_ON == FALSE) && //H30K_Doulas_0044
                    (m_sChannelInfo[eC341_CH_V0].sInterlace_InputTimingInfo.eScanMode != eSCAN_MODE_INTERLACE) && //H30K_Doulas_0044 interlaced not suport image shift
                    (m_sChannelInfo[eC341_CH_V0].ePanelTimingId == PANEL_2D_OUTPUT)) //H30K_Doulas_0041 image shift
            {
                if (m_sChannelInfo[ch_v].sInputTimingInfo.lSTATE < eINPUT_PORT_INFO_STATE_1CH) // 4k input
                {
                    //H30K_Doulas_0046 Modify
                    Input_HST[0] += HORZ_POSITION_DEFAULT - WorkingHorzPosition[ch_v];
                    Input_HW[0]  -= HORZ_POSITION_DEFAULT - WorkingHorzPosition[ch_v];
                    Input_HW[1]  += HORZ_POSITION_DEFAULT - WorkingHorzPosition[ch_v];

                    REG_IACTHST[0] = Input_HST[0];
	                REG_IACTHW[0]  = Input_HW[0];

	                REG_IACTHST[1] = Input_HST[1];
	                REG_IACTHW[1]  = Input_HW[1];
	                //H30K_Doulas_0046 end
                }
                else
                {
                    if((WorkingHorzPosition[ch_v] > HORZ_POSITION_DEFAULT) && (PL_IWIN.HST > 0))
                    {
                        if(PL_IWIN.HST >= (WorkingHorzPosition[ch_v] - HORZ_POSITION_DEFAULT))
                        {
                            PL_IWIN.HST -= (WorkingHorzPosition[ch_v] - HORZ_POSITION_DEFAULT);
                        }
                        else
                        {
                            REG_IACTHST[0] += PL_IWIN.HST + HORZ_POSITION_DEFAULT - WorkingHorzPosition[ch_v];
                            REG_IACTHST[1] += PL_IWIN.HST + HORZ_POSITION_DEFAULT - WorkingHorzPosition[ch_v];
                            PL_IWIN.HST = 0;
                        }
                    }
                    else
                    {
                        REG_IACTHST[0] += HORZ_POSITION_DEFAULT - WorkingHorzPosition[ch_v];
                        REG_IACTHST[1] += HORZ_POSITION_DEFAULT - WorkingHorzPosition[ch_v];
                    }
                }


                //dbmsg("## (0x%02X 0x%02X) (0x%02X x 0x%02X)\r\n", WorkingVertPosition[ch_v], PL_IWIN.VST, REG_IACTVST[0], REG_IACTVST[1]);
                if((WorkingVertPosition[ch_v] < VERT_POSITION_DEFAULT) && (PL_IWIN.VST > 0))
                {
                    if(PL_IWIN.VST >= (VERT_POSITION_DEFAULT - WorkingVertPosition[ch_v]))
                    {
                        PL_IWIN.VST -= (VERT_POSITION_DEFAULT - WorkingVertPosition[ch_v]);
                    }
                    else
                    {
                        REG_IACTVST[0] += PL_IWIN.VST + WorkingVertPosition[ch_v] - VERT_POSITION_DEFAULT;
                        REG_IACTVST[1] += PL_IWIN.VST + WorkingVertPosition[ch_v] - VERT_POSITION_DEFAULT;
                        PL_IWIN.VST = 0;
                    }
                }
                else
                {
                    REG_IACTVST[0] += WorkingVertPosition[ch_v] - VERT_POSITION_DEFAULT;
                    REG_IACTVST[1] += WorkingVertPosition[ch_v] - VERT_POSITION_DEFAULT;
                }
            }
            break;
	}

// *************** HShrink ***************
	if (hsscl != 65536) {
		if ((PL_IWIN.HST <= 0) | (PL_IWIN.HW < 1)) {
			hsinitch1 = 0;
			hfstvldch1 = 0;
			osfld_ofst_ihst = 0;
		}
		else {
			stsp_v = (PL_IWIN.HST * hsscl) & 0xffff;
			endpmod_v = ((PL_IWIN.HW - 1) * hsscl) & 0xffff;
			hsinitch1 = 256 - (stsp_v >> 8);

			if (hsinitch1 == 256) {
				hsinitch1 = 0;
				hfstvldch1 = 0;
			}
			else {
				hfstvldch1 = 1;
			}

			osfld_ofst_ihst = (((hsinitch1 << 8) + (PL_IWIN.HST * hsscl)) >> 16) - hfstvldch1;
		}

		if (m_sChannelInfo[ch_v].sInputTimingInfo.lSTATE <= eINPUT_PORT_INFO_STATE_4CH) // 4k input
        {
			stmhstch2_tmp = (65536 * (1 - hfstvldch1) + hsscl * (Input_HW[0] - 1) + hsinitch1 * 256) >> 16;		// preCalc for 16/20/24bit
			stmhstch2     = stmhstch2_tmp + (stmhstch2_tmp % 2);

			iacthwch1_tmp = ((int)(ceil((stmhstch2-1)*65536.0 / hsscl))) + 1; //H30K_Doulas_0048
			REG_IACTHW[0] = (iacthwch1_tmp < Input_HW[0]) ? Input_HW[0] : iacthwch1_tmp;
			iacthwch2_offset = REG_IACTHW[0] - Input_HW[0];
			REG_IACTHW[1] = Input_HW[0] - iacthwch2_offset;

			stspch2       = (hsscl * REG_IACTHW[0] + hsinitch1 * 256) & 0xffff;
			edspch2       = (hsscl * ((REG_IACTHW[0] + REG_IACTHW[1]) - 1) + hsinitch1 * 256) & 0xffff;

			stspch2mod    =  stspch2 & 0xff;

			hsinit0ch2    = (stspch2 >> 8) & 0xff;
			hsinit1ch2    = hsinit0ch2 + (((stspch2mod >= 128) | (edspch2 < stspch2mod)) ? 1 : 0);

			hsinitch2     =   hsinit1ch2 & 0xff;
			hfstvldch2    = ((hsinit1ch2 == 256) | (stspch2 < hsscl)) ? 0 : 1;

			REG_IACTHST[1] = REG_IACTHST[1] + iacthwch2_offset;

            if(FRAME_MEMORY_FORMAT != 30) //1=YUV444_24bit,2=YUV422_20b,3=YUV422 16b
            {
                stmhaatch2mod5 = stmhstch2 % 5;
                stmhaatch2mod3 = stmhstch2 % 3;
                stmhaatch2rem3 = (3 - stmhaatch2mod3) % 3;

                if(FRAME_MEMORY_FORMAT == 24){           // 24bit
                    adoffsetch1 = 0;
                    adoffsetch2 = (stmhstch2 - stmhaatch2mod5) * 16 / 5 + stmhaatch2mod5 *3;
                }
                else if(FRAME_MEMORY_FORMAT == 20){      // 20bit
                    adoffsetch1 = stmhaatch2rem3 * 2;
                    adoffsetch2 = (stmhstch2 + stmhaatch2rem3) * 8 / 3;
                }
                else if(FRAME_MEMORY_FORMAT == 16){      // 16bit
                    adoffsetch1 = 0;
                    adoffsetch2 = stmhstch2 * 2;
                }
            }
            else
			{
				REG_IACTHW[0]  = Input_HW[0];																	// Calc for 30bit
				REG_IACTHW[1]  = Input_HW[1];
				REG_IACTHST[1] = Input_HST[1];

				stspch2       = (hsscl * Input_HW[0] + hsinitch1 * 256) & 0xffff;
				edspch2       = (hsscl * ((Input_HW[0] + Input_HW[1]) - 1) + hsinitch1 * 256) & 0xffff;

				stspch2mod    =  stspch2 & 0xff;

				hsinit0ch2    = (stspch2 >> 8) & 0xff;
				hsinit1ch2    = hsinit0ch2 + (((stspch2mod >= 128) | (edspch2 < stspch2mod)) ? 1 : 0);

				hsinitch2     =   hsinit1ch2 & 0xff;
				hfstvldch2    = ((hsinit1ch2 == 256) | (stspch2 < hsscl)) ? 0 : 1;

				stmhstch2     = (65536 * (1 - hfstvldch1) + hsscl * (Input_HW[0] - 1) + hsinitch1 * 256) >> 16;

				adoffsetch1 = 0;
				adoffsetch2 = stmhstch2 * 4;
			}
			REG_IACTHW[1]  = Input_HW[1]; //H30K_Doulas_0046
		}
		else
		{
			adoffsetch1 = 0;
			adoffsetch2 = 0;

			hsinitch2 = 0;
			hfstvldch2 = 0;
		}

		IWIN_SHRNK_HW = PL_OWIN.HW;
	}
	else
	{
		stmhaatch2mod5 = Input_HW[0] % 5;
		stmhaatch2mod3 = Input_HW[0] % 3;
		stmhaatch2rem3 = (3 - stmhaatch2mod3) % 3;

		if(FRAME_MEMORY_FORMAT == 24){			// 24bit
			adoffsetch1 = 0;
			adoffsetch2 = (Input_HW[0] - stmhaatch2mod5) * 16 / 5 + stmhaatch2mod5	*3;
		}
		else if(FRAME_MEMORY_FORMAT == 20){		// 20bit
			adoffsetch1 = stmhaatch2rem3 * 2;
			adoffsetch2 = (Input_HW[0] + stmhaatch2rem3) * 8 / 3;
		}
		else if(FRAME_MEMORY_FORMAT == 16){		// 16bit
			adoffsetch1 = 0;
			adoffsetch2 = Input_HW[0] * 2;
		}
		else {									// 30bit
			adoffsetch1 = 0;
			adoffsetch2 = Input_HW[0] * 4;
		}

		hfstvldch1 = 0;
		hsinitch1 = 0;
		hsinitch2 = 0;
		hfstvldch2 = 0;
		osfld_ofst_ihst = PL_IWIN.HST;

		IWIN_SHRNK_HW = PL_IWIN.HW;
	}

	if(FRAME_MEMORY_FORMAT == 20)					// 0=YUV444 30b,1=YUV444_24bit,2=YUV422_20b,3=YUV422 16b
    {
		isfldofst_20bitfmt = adoffsetch1 % 16;
	}
	else
    {
		isfldofst_20bitfmt = 0;
	}

	m20bitfmt_hstofst = (isfldofst_20bitfmt >= 12) ? 5 :
						(isfldofst_20bitfmt >= 10) ? 4 :
						(isfldofst_20bitfmt >=  8) ? 3 :
						(isfldofst_20bitfmt >=  4) ? 2 :
						(isfldofst_20bitfmt >=  2) ? 1 :
						0 ;

// *************** VShrink ***************
	if (vsscl != 65536)
	{
		if ((PL_IWIN.VST <= 0) | (PL_IWIN.VW < 1)) {
			vsinit = 0;
			vfstvld = 0;
			osfld_ofst_ivst = 0;
		}
		else {
			stsp_v = (PL_IWIN.VST * vsscl) & 0xffff;
			endpmod_v = ((PL_IWIN.VW - 1) * vsscl) & 0xffff;
			vsinit = 256 - (stsp_v >> 8);

			if (vsinit == 256) {
				vsinit = 0;
				vfstvld = 0;
			}
			else {
				vfstvld = 1;
			}

			osfld_ofst_ivst = (((vsinit << 8) + (PL_IWIN.VST * vsscl)) >> 16) - 1;
		}

		IWIN_SHRNK_VW = PL_OWIN.VW;
	}
	else
	{
		vsinit = 0;
		vfstvld = 0;
		osfld_ofst_ivst = PL_IWIN.VST;

		IWIN_SHRNK_VW = PL_IWIN.VW;
	}

	REG_HSSCL = (hsscl == 65536) ? 0 : hsscl;
	//REG_HSHRNKCT = (hsscl == 65536) ? 0 : 0x47;
	REG_HSHRNKCT = (hsscl == 65536) ? 0 : 0x07;
	REG_VSSCL = (vsscl == 65536) ? 0 : vsscl;
	//REG_VSHRNKCT = (vsscl == 65536) ? 0 : 0x47;
	if (g_vzoom_flg[ch_v]) REG_VSHRNKCT = (vsscl == 65536) ? 0 : 0x03;
	else REG_VSHRNKCT = (vsscl == 65536) ? 0 : 0x07;

// *************** HZoom ***************
	ohdst = (PL_OWIN.HST < 0) ? 0 : PL_OWIN.HST;
	ohdw = (ohdst > OutputPanel_HW) ? 0
		: ((PL_OWIN.HST + PL_OWIN.HW) > OutputPanel_HW) ? OutputPanel_HW - ohdst
		: (PL_OWIN.HST + PL_OWIN.HW) - ohdst;
	hdw[0] = (ohdst > OutputPanel_HW_1CH) ? 0
		: ((ohdst + ohdw) > OutputPanel_HW_1CH) ? OutputPanel_HW_1CH - ohdst
		: ohdw;
	hdw[1] = ohdw - hdw[0];
	hzscl = ((PL_OWIN.HW <= IWIN_SHRNK_HW) | (IWIN_SHRNK_HW < 1) | (PL_OWIN.HW < 2)) ? 65536
		: (65536 * (IWIN_SHRNK_HW - 1)) / (PL_OWIN.HW - 1) + 1;

	ohst[0]			= (PL_OWIN.HST < 0) ? -PL_OWIN.HST : 0;
	hstbofst_e[0]	= (hdw[0] == 0) ? 0
					: (ohst[0] < hbofst_edge) ? ohst[0] //rule 1
					: hbofst_edge;

	#if 1 //H30K_Doulas_0080
	ohst_eofst[0]	= ohst[0] - hstbofst_e[0];
	#else
	ohst_eofst[0]	= ((float)(PL_OWIN.VW *100 / PL_OWIN.HW) < 75) ? ohst[0] - hstbofst_e[0] //non 4:3
	                : (hstbofst_e[1] == 0) ? ohst[0] - hstbofst_e[0]
	                : (PL_IWIN_HW == OutputPanel_HW) ? ohst[0] - hstbofst_e[0]
	                : ((float)(OutputPanel_HW_1CH * 100 / ohdst) < 600) ? ohst[0] - (hstbofst_e[0] - 1) // edge mask > 0
	                : ohst[0] - hstbofst_e[0];
	#endif

	if(hzscl < 65536) {
		ihst_eofst[0]	= (ohst_eofst[0] * hzscl) >> 16;
		hstbofst_z[0]	= (hdw[0] == 0) ? 0
						: (ihst_eofst[0] >= (PL_OWIN.HW - 1)) ? 0
						: (ihst_eofst[0] >= hbofst_zoom) ? hbofst_zoom
						: ihst_eofst[0];
		ihst_ofst[0]	= ihst_eofst[0] - hstbofst_z[0];
		hzstofst[0]		= ((hstbofst_e[0] + hstbofst_z[0]) < 1) ? 0 : ohst[0] - ((ihst_ofst[0] * 65536 + 65535) / hzscl);

		ihstsub_ofst[0]	= ((ohst[0] - hzstofst[0]) * hzscl) & 0xffff;
		hzinit[0]		= (ihstsub_ofst[0] >> 8) & 0xff;
	}
	else{
		ihst_ofst[0]	= ohst_eofst[0];
		hzstofst[0]		= hstbofst_e[0];
		hzinit[0]		= 0;
	}

	ohend[0]		= (hdw[0] == 0) ? ohst[0]
					: ((ohst[0] + hdw[0] - 1) > (PL_OWIN.HW - 1)) ? PL_OWIN.HW - 1
					: ohst[0] + hdw[0] - 1;

    #if 1 //H30K_Doulas_0080
    hendbofst_e[0]	= (hdw[0] == 0) ? 0
					: ((ohend[0] + hbofst_edge) > (PL_OWIN.HW - 1)) ? (PL_OWIN.HW - 1) - ohend[0]
					: hbofst_edge;
    #else
	hendbofst_e[0]	= (hdw[0] == 0) ? 0
					//: ((ohend[0] + hbofst_edge) > (PL_OWIN.HW - 1)) ? (PL_OWIN.HW - 1) - ohend[0] // rule 2 del
					: (ohst[0] < hbofst_edge) ? ohst[0] //rule 1 add
					: hbofst_edge;
	#endif
	ohend_eofst[0]	= ohend[0] + hendbofst_e[0];

	if(hzscl < 65536) {
		ihend_eofst[0]	= (ohend_eofst[0] * hzscl) >> 16;
		ihend_ofst[0]	= ((ihend_eofst[0] + hbofst_zoom) > (IWIN_SHRNK_HW - 1)) ? IWIN_SHRNK_HW - 1 : ihend_eofst[0] + hbofst_zoom;
		ohend_ofst[0]	= ((ihend_ofst[0] << 16) + hzscl - 1) / hzscl;
		hzendofst[0]	= ohend_ofst[0] - ohend[0];
	}
	else{
		ihend_ofst[0]	= ohend_eofst[0];
		hzendofst[0]	= hendbofst_e[0];
	}

	ihw[0]			= (hdw[0] == 0) ? 0
					: ihend_ofst[0] - ihst_ofst[0] + 1;
	ohst[1]			= (hdw[0] == 0) ? ohst[0]
					: ((ohend[0] + 1) > (PL_OWIN.HW - 1)) ? PL_OWIN.HW - 1
					: ohend[0] + 1;
	hstbofst_e[1]	= (hdw[1] == 0) ? 0
					: (ohst[1] < hbofst_edge) ? ohst[1]  // rule 1
					: hbofst_edge;

	#if 1//H30K_Doulas_0080
	ohst_eofst[1]	= ohst[1] - hstbofst_e[1];
	#else
	ohst_eofst[1]	= ((float)(PL_OWIN.VW *100 / PL_OWIN.HW) < 75) ? ohst[1] - hstbofst_e[1] //non 4:3
	                : (hstbofst_e[1] == 0) ? ohst[1] - hstbofst_e[1]
	                : (PL_IWIN_HW == OutputPanel_HW) ? ohst[1] - hstbofst_e[1]
	                : ((float)(OutputPanel_HW_1CH * 100 / ohdst) < 600) ?  ohst[1] - (hstbofst_e[1] - 1) // edge mask > 0
	                : ohst[1] - hstbofst_e[1];
    #endif
	if(hzscl < 65536) {
		ihst_eofst[1]	= (ohst_eofst[1] * hzscl) >> 16;
		hstbofst_z[1]	= (hdw[1] == 0) ? 0
						: (ihst_eofst[1] >= (PL_OWIN.HW - 1)) ? 0
						: (ihst_eofst[1] >= hbofst_zoom) ? hbofst_zoom
						: ihst_eofst[1];
		ihst_ofst[1]	= ihst_eofst[1] - hstbofst_z[1];
		hzstofst[1]		= ((hstbofst_e[1] + hstbofst_z[1]) < 1) ? 0 : ohst[1] - ((ihst_ofst[1] * 65536 + 65535) / hzscl);
		ihstsub_ofst[1]	= ((ohst[1] - hzstofst[1]) * hzscl) & 0xffff;
		hzinit[1]		= (ihstsub_ofst[1] >> 8) & 0xff;
	}
	else{
		ihst_ofst[1]	= ohst_eofst[1];
		hzstofst[1]		= hstbofst_e[1];
		hzinit[1]		= 0;
	}

	ohend[1]		= (hdw[1] == 0) ? ohst[1]
					: ((ohst[1] + hdw[1] - 1) > (PL_OWIN.HW - 1)) ? PL_OWIN.HW - 1
					: ohst[1] + hdw[1] - 1;

	#if 1//H30K_Doulas_0080
	hendbofst_e[1]	= (hdw[1] == 0) ? 0
					: ((ohend[1] + hbofst_edge) > (PL_OWIN.HW - 1)) ? (PL_OWIN.HW - 1) - ohend[1]
					: hbofst_edge;
	#else
	hendbofst_e[1]	= (hdw[1] == 0) ? 0
					//: ((ohend[1] + hbofst_edge) > (PL_OWIN.HW - 1)) ? (PL_OWIN.HW - 1) - ohend[1] // rule 2 del
					: (ohst[1] < hbofst_edge) ? ohst[1]  // rule 1 add
					: hbofst_edge;
	#endif
	ohend_eofst[1]	= ohend[1] + hendbofst_e[1];

	if(hzscl < 65536) {
		ihend_eofst[1]	= (ohend_eofst[1] * hzscl) >> 16;
		ihend_ofst[1]	= ((ihend_eofst[1] + hbofst_zoom) > (IWIN_SHRNK_HW - 1)) ? IWIN_SHRNK_HW - 1 : ihend_eofst[1] + hbofst_zoom;
		ohend_ofst[1]	= ((ihend_ofst[1] << 16) + hzscl - 1) / hzscl;
		hzendofst[1]	= ohend_ofst[1] - ohend[1];
		REG_HZSCL		= hzscl;
		REG_HZOOMCT		= (HZMD==0) ? 0xc7 : 0xc3;
	}
	else {
		ihend_ofst[1]	= ohend_eofst[1];
		hzendofst[1]	= hendbofst_e[1];
		ihw[1]			= (hdw[1] == 0) ? 0
						: ihend_ofst[1] - ihst_ofst[1] + 1;
		REG_HZSCL		= 0;
		REG_HZOOMCT		= 0;
	}
	ihw[1]			= (hdw[1] == 0) ? 0 : ihend_ofst[1] - ihst_ofst[1] + 1;
	//LOG_MSG(db_ALWAYS,"Hx(%d,%d)\r\n",hzscl,hbofst_zoom); //H30K_Doulas_0080
	//LOG_MSG(db_ALWAYS,"H0(%d,%d)(%d,%d)(%d,%d)(%d,%d)\r\n",ihend_ofst[0],ihst_ofst[0],ohend_eofst[0],ohst_eofst[0],ohend[0],hendbofst_e[0],ohst[0],hstbofst_e[0]); //H30K_Doulas_0080
    //LOG_MSG(db_ALWAYS,"H1(%d,%d)(%d,%d)(%d,%d)(%d,%d)\r\n",ihend_ofst[1],ihst_ofst[1],ohend_eofst[1],ohst_eofst[1],ohend[1],hendbofst_e[1],ohst[1],hstbofst_e[1]); //H30K_Doulas_0080

	REG_OACTHST[0]	= (hdw[0] == 0) ? OutputPanel_HST_1CH
					: OutputPanel_HST_1CH + ohdst - hzstofst[0];
	REG_OACTHW[0]	= ihw[0];
	REG_HZINIT[0]	= hzinit[0];

	REG_OACTHST[1]	= (hdw[1] == 0) ? OutputPanel_HST_1CH
					: (ohdst > OutputPanel_HW_1CH) ? OutputPanel_HST_1CH + (ohdst - OutputPanel_HW_1CH) - hzstofst[1]
					: OutputPanel_HST_1CH - hzstofst[1];
	REG_OACTHW[1]	= ihw[1];
	REG_HZINIT[1]	= hzinit[1];

	if ((REG_OACTHW[0] < 16) && (REG_OACTHW[0] > 0)) REG_OACTHW[0] = 16;
	if ((REG_OACTHW[1] < 16) && (REG_OACTHW[1] > 0)) REG_OACTHW[1] = 16;

	g_oscl_use[ch_v][0] = (REG_OACTHW[0] != 0);
	g_oscl_use[ch_v][1] = (REG_OACTHW[1] != 0);

// *************** VZoom ***************
	vzscl			= ((PL_OWIN.VW <= IWIN_SHRNK_VW) | (IWIN_SHRNK_VW < 1) | (PL_OWIN.VW < 2)) ? 65536
					: (65536 * (IWIN_SHRNK_VW - 1)) / (PL_OWIN.VW - 1) + 1;
	vdw1			= (PL_OWIN.VST >= OutputPanel_VW) ? 0
					: (PL_OWIN.VST > 0) ? OutputPanel_VW - PL_OWIN.VST
					: OutputPanel_VW;
	ovst1			= (PL_OWIN.VST < 0) ? -PL_OWIN.VST : 0;
	vbofst1			= (ovst1 < vbofst_edge) ? ovst1
					: vbofst_edge;
	ovst1_ofst		= ovst1 - vbofst1;
	ivst1			= ovst1_ofst * vzscl >> 16;
	ivstsub1		= (ovst1_ofst * vzscl) & 0xffff;
	vstofst1		= ((PL_OWIN.VW - ivst1 - 1) < 1) ? 0
					: (ivst1 > vbofst1) ? vbofst1
					: ivst1;
	ivst1_ofst		= ivst1 - vstofst1;
	vzstofst1		= ((vbofst1 + vstofst1) < 1) ? 0
					: ovst1 - ((ivst1_ofst * 65536 + 65535) / vzscl);
	ivstsub1_ofst	= ((ovst1 - vzstofst1) * vzscl) & 0xffff;
	vzinit1			= (ivstsub1_ofst >> 8) & 0xff;
	ovend1			= (vdw1 == 0) ? ovst1
					: ((ovst1 + vdw1 - 1) > (PL_OWIN.VW - 1)) ? PL_OWIN.VW - 1
					: ovst1 + vdw1 - 1;
	ovend1_ofst		= (vdw1 == 0) ? ovend1
					: ((ovend1 + vbofst_edge) > (PL_OWIN.VW - 1)) ? PL_OWIN.VW - 1
					: ovend1 + vbofst_edge;
	ovw1			= ((PL_OWIN.VW < 1) | (vdw1 < 1)) ? 0
					: (ovend1_ofst - ovst1 + 1) + vzstofst1;

	if (vzscl != 65536) {
		REG_OACTVST		= (PL_OWIN.VST > 0) ? OutputPanel_VST + PL_OWIN.VST - (vzoom_ofst_vst - 2)    //simon check -2
						: OutputPanel_VST - vzstofst1 - (vzoom_ofst_vst - 2);
		REG_OACTVW		= ovw1 - vzoom_ofst_vw;
		REG_VZSCL		= vzscl;
		REG_VZOOMCT		= (VZMD==0) ? 0x47 : 0x43;
		REG_VZMADJCT	= (ovend1_ofst >= (PL_OWIN.VW - 1)) ? 1 : 0;
	}
	else{
		REG_OACTVST		= (PL_OWIN.VST > 0) ? OutputPanel_VST + PL_OWIN.VST
						: OutputPanel_VST - vzstofst1 ;
		REG_OACTVW		= ovw1 + 1;
		REG_VZSCL		= 0;
		REG_VZOOMCT		= 0;
		REG_VZMADJCT	= 0;
	}

	// calculate OAOI1
	REG_OAOI1VST = OutputPanel_VST + PL_OWIN.VST;
	REG_OAOI1VEND = REG_OAOI1VST + PL_OWIN.VW;

	if (PL_OWIN.HST < OutputPanel_HW_1CH)
	{
		if ((PL_OWIN.HST + PL_OWIN.HW) < OutputPanel_HW_1CH)
		{
			REG_OAOI1HST[0]  = OutputPanel_HST_1CH + PL_OWIN.HST;
			REG_OAOI1HEND[0] = REG_OAOI1HST[0] + PL_OWIN.HW;
			REG_OAOI1HST[1]  = OutputPanel_HST_1CH;
			REG_OAOI1HEND[1] = REG_OAOI1HST[1];
		}
		else
		{
			REG_OAOI1HST[0]  = OutputPanel_HST_1CH + PL_OWIN.HST;
			REG_OAOI1HEND[0] = OutputPanel_HST_1CH + OutputPanel_HW_1CH;
			REG_OAOI1HST[1]  = OutputPanel_HST_1CH;
			REG_OAOI1HEND[1] = REG_OAOI1HST[1] + (PL_OWIN.HW - (OutputPanel_HW_1CH - PL_OWIN.HST));
		}
	}
	else
	{
		REG_OAOI1HST[0]  = OutputPanel_HST_1CH;
		REG_OAOI1HEND[0] = REG_OAOI1HST[0];
		REG_OAOI1HST[1]  = OutputPanel_HST_1CH + (PL_OWIN.HST - OutputPanel_HW_1CH);
		REG_OAOI1HEND[1] = REG_OAOI1HST[1] + PL_OWIN.HW;
	}

	if (REG_OAOI1HST[1] < 0) REG_OAOI1HST[1] = REG_OAOI1HEND[0] = OutputPanel_HST_1CH;
	if (REG_OAOI1HEND[1] < 0) REG_OAOI1HEND[1] = REG_OAOI1HST[1];

	// memad
	#if 0  //simon check
	ivst1_ofst += (PM_UPSIDEDOWN[ch_v] == 0) ? 0
		: IWIN_SHRNK_VW - 1;
	#endif

	ivst1_ofst += 0;


	#if 0  //simon check
	if (PM_FLIP[ch_v] == 1) {
		mhst_ofst[0] = (IWIN_SHRNK_HW - (ihend_ofst[0] + 1)) ;
		mhst_ofst[1] = (IWIN_SHRNK_HW - (ihend_ofst[1] + 1)) ;
	}
	else
	#endif
	{
		mhst_ofst[0] = ihst_ofst[0] ;
		mhst_ofst[1] = ihst_ofst[1] ;
	}

    if (FRAME_MEMORY_FORMAT == 24)                         // 24bit
    {
        //=MHST1_OFST*3+MHST1_OFST_DIV5+IVST1_OFST*OMWIBYTE;
        mhst1_ofst_div5[0] = (mhst_ofst[0] + osfld_ofst_ihst) / 5;
        osfld_ofst[0] = (mhst_ofst[0] + osfld_ofst_ihst) * 3 + mhst1_ofst_div5[0] + (ivst1_ofst + osfld_ofst_ivst) * DEF_SCMWICH1_4K * 1024;

        mhst1_ofst_div5[1] = (mhst_ofst[1] + osfld_ofst_ihst) / 5;
        osfld_ofst[1] = (mhst_ofst[1] + osfld_ofst_ihst) * 3 + mhst1_ofst_div5[1] + (ivst1_ofst + osfld_ofst_ivst) * DEF_SCMWICH1_4K * 1024;
    }
    else if (FRAME_MEMORY_FORMAT == 20)                    // 20bit
    {
        //=MHST1DIV3*8+MHST1REM3*2+IVST1_OFST*OMWIBYTE;
        mhst1div3[0] = (mhst_ofst[0] + m20bitfmt_hstofst + osfld_ofst_ihst) / 3;
        mhst1rem3[0] = (mhst_ofst[0] + m20bitfmt_hstofst + osfld_ofst_ihst) % 3;
        osfld_ofst[0] = mhst1div3[0]*8 + mhst1rem3[0]*2 + (ivst1_ofst + osfld_ofst_ivst) * DEF_SCMWICH1_4K * 1024;

        mhst1div3[1] = (mhst_ofst[1] + m20bitfmt_hstofst + osfld_ofst_ihst) / 3;
        mhst1rem3[1] = (mhst_ofst[1] + m20bitfmt_hstofst + osfld_ofst_ihst) % 3;
        osfld_ofst[1] = mhst1div3[1]*8 + mhst1rem3[1]*2 + (ivst1_ofst + osfld_ofst_ivst) * DEF_SCMWICH1_4K * 1024;
    }
    else if (FRAME_MEMORY_FORMAT == 16)                     // 16bit
    {
        //=MHST1_OFST*2+IVST1_OFST*OMWIBYTE; L
        osfld_ofst[0] = (mhst_ofst[0] + osfld_ofst_ihst) *  2 + (ivst1_ofst + osfld_ofst_ivst) * DEF_SCMWICH1_4K * 1024;
        osfld_ofst[1] = (mhst_ofst[1] + osfld_ofst_ihst) *  2 + (ivst1_ofst + osfld_ofst_ivst) * DEF_SCMWICH1_4K * 1024;
    }
    else                                                   // 30bit
    {
        //=MHST1_OFST*4+IVST1_OFST*OMWIBYTE; L
        osfld_ofst[0] = (mhst_ofst[0] + osfld_ofst_ihst) *  4 + (ivst1_ofst + osfld_ofst_ivst) * DEF_SCMWICH1_4K * 1024;
        osfld_ofst[1] = (mhst_ofst[1] + osfld_ofst_ihst) *  4 + (ivst1_ofst + osfld_ofst_ivst) * DEF_SCMWICH1_4K * 1024;
    }

    #if 1 //H30K_Doulas_0077
    if(!g_rtct)
	{
		dvC341_set_rtct_normal();
    //    dvC341_wait1_povs(ch_v);
		dvC341_Write(B0_FLDRTCTCH12 + 0x200*ch_v, 0x85+0x20*ch_v, 0);
		//dvC341_set_rtct_stop(); //H30K_Doulas_0079//H30K_Doulas_0077
	}
    #endif
    #if 0
	if(!g_rtct)
	{
        //input port pass through
        dvC341_Write(B0_RHOLD, 0x01, 0);        // RTCT0-7 Hold
        dvC341_Write(B0_RTCT0, 0x00, 0);            // [7:4]RTG 1(OUTCH3/4)=F / [3:0]RTG 0(OUTCH1/2)=F
        dvC341_Write(B0_RTCT2, 0x00, 0);            // [7:4]RTG 5(IN CH2)=F   / [3:0]RTG 4(IN CH1)=F
        dvC341_Write(B0_RTCT3, 0x00, 0);            // [7:4]RTG 7(IN CH4)=F   / [3:0]RTG 6(IN CH3)=F
        dvC341_Write(B0_RHOLD, 0x00, 0);        // RTCT0-7 Normal
		//dvC341_set_rtct_normal();
        dvC341_wait1_povs(ch_v);
		dvC341_Write(B0_FLDRTCTCH12 + 0x200*ch_v, 0x85+0x20*ch_v, 0);
	}
	#endif
    //dvC341_Write_ch_v(ch_v, B17_ILSCCTCH1, 0x10, CH_BANK_OFFSET);

	dvC341_Write(B16_IACTHSTCH1, 	REG_IACTHST[0], ibank_offset);
	dvC341_Write(B16_IACTHWCH1, 	REG_IACTHW[0], ibank_offset);
	dvC341_Write(B16_IACTVSTCH1, 	REG_IACTVST[0], ibank_offset); //H30K_Doulas_0001
	dvC341_Write(B16_IACTVWCH1, 	REG_IACTVW[0], ibank_offset);  //H30K_Doulas_0001

    if(m_sChannelSetting[eC341_CH_V0].cPIP_PBP_ON == FALSE)  //H2PF_Simon_0171
    {
        dvC341_Write(B48_IACTHSTCH3,    0, 0);
        dvC341_Write(B48_IACTHWCH3,     0, 0);
        dvC341_Write(B48_IACTVSTCH3,    0, 0);
        dvC341_Write(B48_IACTVWCH3,     0, 0);
    }

#if 1  //H30K_Doulas_0077
    dvC341_Write(B17_SBOUNDCH1, hfstvldch1 * 128 + vfstvld * 8, ibank_offset);
    dvC341_Write(B17_HSHRNKCTCH1,   REG_HSHRNKCT | 0x80, ibank_offset);
    dvC341_Write(B17_HSSCLCH1, REG_HSSCL, ibank_offset);
    dvC341_Write(B17_VSHRNKCTCH1,   REG_VSHRNKCT | 0x80, ibank_offset);
    dvC341_Write(B17_VSSCLCH1, REG_VSSCL, ibank_offset);
    dvC341_Write(B17_SHRNKADJCH1, 0x02, ibank_offset);
    dvC341_Write(B17_HSINITEVCH1, hsinitch1, ibank_offset);
    dvC341_Write(B17_HSINITODCH1, hsinitch1, ibank_offset);
    dvC341_Write(B17_VSINITEVCH1, vsinit, ibank_offset);
    dvC341_Write(B17_VSINITODCH1, vsinit, ibank_offset);

    dvC341_Write(B32_IACTHSTCH2, REG_IACTHST[1], ibank_offset);
    dvC341_Write(B32_IACTHWCH2, REG_IACTHW[1], ibank_offset);
    dvC341_Write(B32_IACTVSTCH2, REG_IACTVST[1], ibank_offset); //H30K_Doulas_0001
    dvC341_Write(B32_IACTVWCH2, REG_IACTVW[1], ibank_offset);   //H30K_Doulas_0001

    if(m_sChannelSetting[eC341_CH_V0].cPIP_PBP_ON == FALSE)  //H2PF_Simon_0171
    {
        dvC341_Write(B64_IACTHSTCH4,    0, 0);
        dvC341_Write(B64_IACTHWCH4,     0, 0);
        dvC341_Write(B64_IACTVSTCH4,    0, 0);
        dvC341_Write(B64_IACTVWCH4,     0, 0);
    }

    dvC341_Write(B33_SBOUNDCH2, hfstvldch2 * 128 + vfstvld * 8, ibank_offset);
    dvC341_Write(B33_HSHRNKCTCH2, REG_HSHRNKCT | 0x80, ibank_offset);
    dvC341_Write(B33_HSSCLCH2, REG_HSSCL, ibank_offset);
    dvC341_Write(B33_VSHRNKCTCH2, REG_VSHRNKCT | 0x80, ibank_offset);
    dvC341_Write(B33_VSSCLCH2, REG_VSSCL, ibank_offset);
    dvC341_Write(B33_SHRNKADJCH2, 0x02, ibank_offset);
    dvC341_Write(B33_HSINITEVCH2,   hsinitch2, ibank_offset);
    dvC341_Write(B33_HSINITODCH2,   hsinitch2, ibank_offset);
    dvC341_Write(B33_VSINITEVCH2, vsinit, ibank_offset);
    dvC341_Write(B33_VSINITODCH2, vsinit, ibank_offset);
#else
	//dvC341_Write(B17_SBOUNDCH1, hfstvldch1 * 128 + vfstvld * 8, ibank_offset);
	dvC341_Write(B17_HSHRNKCTCH1,	REG_HSHRNKCT | 0x80, ibank_offset);
	dvC341_Write(B17_HSSCLCH1, REG_HSSCL, ibank_offset);
	dvC341_Write(B17_VSHRNKCTCH1,	REG_VSHRNKCT | 0x80, ibank_offset);
	dvC341_Write(B17_VSSCLCH1, REG_VSSCL, ibank_offset);
	//dvC341_Write(B17_SHRNKADJCH1, 0x02, ibank_offset);
	//dvC341_Write(B17_HSINITEVCH1, hsinitch1, ibank_offset);
	//dvC341_Write(B17_HSINITODCH1, hsinitch1, ibank_offset);
	//dvC341_Write(B17_VSINITEVCH1, vsinit, ibank_offset);
	//dvC341_Write(B17_VSINITODCH1, vsinit, ibank_offset);

	dvC341_Write(B32_IACTHSTCH2, REG_IACTHST[1], ibank_offset);
	dvC341_Write(B32_IACTHWCH2, REG_IACTHW[1], ibank_offset);
	dvC341_Write(B32_IACTVSTCH2, REG_IACTVST[1], ibank_offset); //H30K_Doulas_0001
	dvC341_Write(B32_IACTVWCH2, REG_IACTVW[1], ibank_offset);   //H30K_Doulas_0001

	//dvC341_Write(B33_SBOUNDCH2, hfstvldch2 * 128 + vfstvld * 8, ibank_offset);
	dvC341_Write(B33_HSHRNKCTCH2, REG_HSHRNKCT | 0x80, ibank_offset);
	dvC341_Write(B33_HSSCLCH2, REG_HSSCL, ibank_offset);
	dvC341_Write(B33_VSHRNKCTCH2, REG_VSHRNKCT | 0x80, ibank_offset);
	dvC341_Write(B33_VSSCLCH2, REG_VSSCL, ibank_offset);
	//dvC341_Write(B33_SHRNKADJCH2, 0x02, ibank_offset);
	//dvC341_Write(B33_HSINITEVCH2,	hsinitch2, ibank_offset);
	//dvC341_Write(B33_HSINITODCH2,	hsinitch2, ibank_offset);
	//dvC341_Write(B33_VSINITEVCH2, vsinit, ibank_offset);
	//dvC341_Write(B33_VSINITODCH2, vsinit, ibank_offset);
#endif
	g_B16_IACTVST[0] = REG_IACTVST[0];  //H30K_Doulas_0020
	g_B16_IACTVW[0]  = REG_IACTVW[0];   //H30K_Doulas_0020
	g_B16_IACTVST[1] = REG_IACTVST[1];  //H30K_Doulas_0020
	g_B16_IACTVW[1]  = REG_IACTVW[1];   //H30K_Doulas_0020

    if((m_sChannelInfo[eC341_CH_V0].ePanelTimingId != ePANEL_ID_1080P_120HZ) &&
       (m_sChannelInfo[eC341_CH_V0].ePanelTimingId != ePANEL_ID_WUXGA_120HZ)) //H30K_Doulas_0001
    {
        if(ch_v == eC341_CH_V0)
        {
        	dvC341_Write(B22_ISFLD0CH1, PS_MEM_SC[0] + adoffsetch1, 0);
        	dvC341_Write(B22_ISFLD1CH1, PS_MEM_SC[1] + adoffsetch1, 0);
        	dvC341_Write(B22_ISFLD2CH1, PS_MEM_SC[2] + adoffsetch1, 0);
        	dvC341_Write(B22_ISFLD3CH1, PS_MEM_SC[3] + adoffsetch1, 0);
        	dvC341_Write(B22_ISFLD4CH1, PS_MEM_SC[4] + adoffsetch1, 0);

        	dvC341_Write(B38_ISFLD0CH2, PS_MEM_SC[0] + adoffsetch2, 0);
        	dvC341_Write(B38_ISFLD1CH2, PS_MEM_SC[1] + adoffsetch2, 0);
        	dvC341_Write(B38_ISFLD2CH2, PS_MEM_SC[2] + adoffsetch2, 0);
        	dvC341_Write(B38_ISFLD3CH2, PS_MEM_SC[3] + adoffsetch2, 0);
        	dvC341_Write(B38_ISFLD4CH2, PS_MEM_SC[4] + adoffsetch2, 0);
        }
        else if(ch_v == eC341_CH_V1)
        {
        	dvC341_Write(B54_ISFLD0CH3, PS_MEM_SC[0] + adoffsetch1, 0);
        	dvC341_Write(B54_ISFLD1CH3, PS_MEM_SC[1] + adoffsetch1, 0);
        	dvC341_Write(B54_ISFLD2CH3, PS_MEM_SC[2] + adoffsetch1, 0);
        	dvC341_Write(B54_ISFLD3CH3, PS_MEM_SC[3] + adoffsetch1, 0);
        	dvC341_Write(B54_ISFLD4CH3, PS_MEM_SC[4] + adoffsetch1, 0);

        	dvC341_Write(B70_ISFLD0CH4, PS_MEM_SC[0] + adoffsetch2, 0);
        	dvC341_Write(B70_ISFLD1CH4, PS_MEM_SC[1] + adoffsetch2, 0);
        	dvC341_Write(B70_ISFLD2CH4, PS_MEM_SC[2] + adoffsetch2, 0);
        	dvC341_Write(B70_ISFLD3CH4, PS_MEM_SC[3] + adoffsetch2, 0);
        	dvC341_Write(B70_ISFLD4CH4, PS_MEM_SC[4] + adoffsetch2, 0);
    	}
    }

	dvC341_set_scfmfmt(ch_v, FRAME_MEMORY_FORMAT);   //H2PF_Simon_0131
	if (REG_HSSCL != 0)
	{
		dvC341_set_lut_hshrnk(REG_HSSCL, ch_v);
	}

	if (REG_VSSCL != 0) {
		dvC341_set_lut_vshrnk(REG_VSSCL, ch_v);
	}

	if ((OUTPUT_CH_NUM == 2) &&
	    ((m_sChannelInfo[eC341_CH_V0].ePanelTimingId != ePANEL_ID_1080P_120HZ) && (m_sChannelInfo[eC341_CH_V0].ePanelTimingId != ePANEL_ID_WUXGA_120HZ))) //H30K_Doulas_0002
	{
		REG_OACTVST--;
		REG_OAOI1VST--;
		REG_OAOI1VEND--;
	}

	if( (FRAME_MEMORY_FORMAT == 20) && (((PS_MEM_SC[0] + m20bitfmt_hstofst) % 2) == 0) && ((((PS_MEM_SC[0] + osfld_ofst[0]) % 16) == 2) || (((PS_MEM_SC[0] + osfld_ofst[0]) % 16) == 8) || (((PS_MEM_SC[0] + osfld_ofst[0]) % 16) == 12)) ){
		m_sChannelInfo[ch_v].ucOIMGCT = m_sChannelInfo[ch_v].ucOIMGCT | 0x20;
	}
	else if( (FRAME_MEMORY_FORMAT == 20) && (((PS_MEM_SC[0] + m20bitfmt_hstofst) % 2) == 1) && ((((PS_MEM_SC[0] + osfld_ofst[0]) % 16) == 0) || (((PS_MEM_SC[0] + osfld_ofst[0]) % 16) == 4) || (((PS_MEM_SC[0] + osfld_ofst[0]) % 16) == 10)) ){
		m_sChannelInfo[ch_v].ucOIMGCT = m_sChannelInfo[ch_v].ucOIMGCT | 0x20;
	}
	else if( (FRAME_MEMORY_FORMAT == 16) && (((PS_MEM_SC[0] + osfld_ofst[0]) % 4) == 2) ){
		m_sChannelInfo[ch_v].ucOIMGCT = m_sChannelInfo[ch_v].ucOIMGCT | 0x20;
	}
	else
	{
		m_sChannelInfo[ch_v].ucOIMGCT = m_sChannelInfo[ch_v].ucOIMGCT & 0xDF;
	}

	switch(dvC341_Input_3D_Format_Config_Get()) //H30K_Doulas_0001
	{
        case eINPUT_3D_TYPE_FRAMEPACKING:
            if((m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive == 1920) &&
                (m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiVActive == 1080))
            {
                REG_OACTHW[0] = m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive/2;
                REG_OACTHW[1] = m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive/2;
                REG_OACTHST[1] = REG_OACTHST[0];
            }
            else
            {
                REG_OACTHST[1] = REG_OACTHST[0] - 5;
            }
            REG_OACTVW = m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiVActive + 3;
            REG_VZMADJCT = 2;
            break;

        case eINPUT_3D_TYPE_TOPANDBOTTOM:
            if((m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive == 1920) &&
                (m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive == 1080))
            {
                REG_OACTHW[0] = m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive/2;
                REG_OACTHW[1] = m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive/2;
                REG_OACTHST[1] = REG_OACTHST[0];
            }
            else
            {
                REG_OACTHST[1] = REG_OACTHST[0] - 5;
            }
            REG_OACTVW = m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiVActive + 3;

            REG_VZMADJCT = 2;
            break;

        case eINPUT_3D_TYPE_SIDEBYSIDE:
            REG_OACTHST[1] = REG_OACTHST[0] - 7;
            REG_OACTHW[0] = m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive/2/2 + 4;
            REG_OACTHW[1] = m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive/2/2 + 4;
            REG_OACTVW = m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiVActive + 3;

            REG_VZMADJCT = 2;
            break;

        case eINPUT_3D_TYPE_FRAME_SEQUENTIAL:
            if((m_sChannelInfo[eC341_CH_V0].ePanelTimingId == ePANEL_ID_1080P_120HZ) ||
               (m_sChannelInfo[eC341_CH_V0].ePanelTimingId == ePANEL_ID_WUXGA_120HZ)) //H30K_Doulas_0009
            {
                REG_VZMADJCT = 2;
                if(g_vbo_recno[m_sChannelInfo[ch_v].ucInputPort] == 4) //H30K_Doulas_0029 8-lane
                {
                    if(m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive == 960)
                    {
                        REG_OACTHST[1] = REG_OACTHST[0];
                        REG_OACTHW[0] = m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive;
                        REG_OACTHW[1] = m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive;
                    }
                    else
                    {
                        REG_OACTHST[1] = REG_OACTHST[0] - 5;
                        REG_OACTHW[0] = m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive + 4;
                        REG_OACTHW[1] = m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive + 4;
                    }
                }
                else if((m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive == 1920))
                {
                    REG_OACTHST[1] = REG_OACTHST[0]; //H30K_Doulas_0002
                    REG_OACTHW[0] = m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive/2;
                    REG_OACTHW[1] = m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive/2;
                }
                else
                {
                #ifdef AUTO_ASPECT_RATIO_WITH_3D    //H30K_Doulas_0053
                    if(m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive == 720)
                        REG_OACTHST[1] = REG_OACTHST[0] - 8;
                    else if(m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive == 800)
                        REG_OACTHST[1] = REG_OACTHST[0] - 6;
                    else if((m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive == 1280) && (m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive == 1024))
                        REG_OACTHST[1] = REG_OACTHST[0] - 4;
                    else if((m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive == 1280) && (m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive == 960))
                        REG_OACTHST[1] = REG_OACTHST[0] - 4;
                    else if((m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive == 1360) && (m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive == 768))
                        REG_OACTHST[1] = REG_OACTHST[0] - 4;
                    else if((m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive == 1440) && (m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive == 900))
                        REG_OACTHST[1] = REG_OACTHST[0] - 4;
                    else if((m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive == 1440) && (m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive == 1050))
                        REG_OACTHST[1] = REG_OACTHST[0] - 4;
                    else if(m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive == 1600)
                        REG_OACTHST[1] = REG_OACTHST[0] - 4;
                    else if(m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive == 1680)
                        REG_OACTHST[1] = REG_OACTHST[0] - 4;
                    else
                        REG_OACTHST[1] = REG_OACTHST[0] - 5;
                #else
                    if(m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive == 720)
                        REG_OACTHST[1] = REG_OACTHST[0] - 8;
                    else if(m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive == 800)
                        REG_OACTHST[1] = REG_OACTHST[0] - 7;
                    else if((m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive == 1360) && (m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive == 768))
                        REG_OACTHST[1] = REG_OACTHST[0] - 4;
                    else if((m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive == 1440) && (m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive == 900))
                        REG_OACTHST[1] = REG_OACTHST[0] - 4;
                    else if((m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive == 1440) && (m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive == 1050))
                        REG_OACTHST[1] = REG_OACTHST[0] - 4;
                    else if(m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive == 1600)
                        REG_OACTHST[1] = REG_OACTHST[0] - 4;
                    else if(m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive == 1680)
                        REG_OACTHST[1] = REG_OACTHST[0] - 4;
                    else
                        REG_OACTHST[1] = REG_OACTHST[0] - 5;
                    //REG_OACTHST[1] = REG_OACTHST[0] - 5; //H30K_Doulas_0002
                #endif
                    REG_OACTHW[0] = m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive/2 + 4;
                    REG_OACTHW[1] = m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive/2 + 4;
                }
                #ifdef AUTO_ASPECT_RATIO_WITH_3D    //H30K_Doulas_0053
                REG_OACTHST[1] -= PL_OWIN.HST;
                #endif
                REG_OACTVW = m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiVActive + 3;

            }
            else if((m_sChannelInfo[eC341_CH_V0].ePanelTimingId == ePANEL_ID_WUXGA_240HZ) &&
                    (m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive == 1920 && m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive == 1080) &&
                    (m_sChannelInfo[ch_v].sInputTimingInfo.dFrameRate > 119.50 && m_sChannelInfo[ch_v].sInputTimingInfo.dFrameRate < 120.3)) //H30K_Doulas_0047 Modify//H2PF_Simon_0107
            {
                REG_OACTHST[1] += 4;    //H30K_Doulas_0054
                REG_OACTHW[0] = m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive/2;
                REG_OACTHW[1] = m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive/2;
            }
            break;

        default:
            if((m_sChannelInfo[eC341_CH_V0].ePanelTimingId == ePANEL_ID_1080P_120HZ) ||
               (m_sChannelInfo[eC341_CH_V0].ePanelTimingId == ePANEL_ID_WUXGA_120HZ)) //H30K_Doulas_0002
            {
                REG_VZMADJCT = 2;
                #if 1 //H30K_Doulas_0034
                UINT16	uiHActive;

                if((g_vbo_recno[m_sChannelInfo[ch_v].ucInputPort] == 14) && // 2-lane & 2CH
                   (m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive < 1920)) //not 4K
                    uiHActive = m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive * 2;
                else if((g_vbo_recno[m_sChannelInfo[ch_v].ucInputPort] == 4)) //H30K_Doulas_0052 240hz
                    uiHActive = m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive * 2;
                else
                    uiHActive = m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive;

                if(uiHActive >= m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiHActive) //H30K_Doulas_0053 m_sChannelInfo[ch_v].sOutputTimingInfo.uiHActive
                {
                    REG_OACTHST[1] = REG_OACTHST[0];
                    uiHActive = m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiHActive; //H30K_Doulas_0053 m_sChannelInfo[ch_v].sOutputTimingInfo.uiHActive
                    REG_OACTHW[0] = uiHActive/2;
                    REG_OACTHW[1] = uiHActive/2;
                }
                else
                {
                #ifdef AUTO_ASPECT_RATIO_WITH_3D    //H30K_Doulas_0053
                    if(uiHActive == 640)
                        REG_OACTHST[1] = REG_OACTHST[0] - 9;
                    else if((uiHActive == 720) && (m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive == 400))
                        REG_OACTHST[1] = REG_OACTHST[0] - 8;
                    else if((uiHActive == 720) && (m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive == 480))
                        REG_OACTHST[1] = REG_OACTHST[0] - 8;
                    else if((uiHActive == 720) && (m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive == 576))
                        REG_OACTHST[1] = REG_OACTHST[0] - 6;
                    else if(uiHActive == 800)
                        REG_OACTHST[1] = REG_OACTHST[0] - 7;
                    else if(uiHActive == 848)
                        REG_OACTHST[1] = REG_OACTHST[0] - 7;
                    else if((uiHActive == 1280) && (m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive == 1024))
                        REG_OACTHST[1] = REG_OACTHST[0] - 4;
                    else if((uiHActive == 1280) && (m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive == 960))
                        REG_OACTHST[1] = REG_OACTHST[0] - 4;
                    else if(uiHActive > 1280)
                        REG_OACTHST[1] = REG_OACTHST[0] - 4;
                    else
                        REG_OACTHST[1] = REG_OACTHST[0] - 5;
                #else
                    if(uiHActive == 640)
                        REG_OACTHST[1] = REG_OACTHST[0] - 9;
                    else if(uiHActive == 720)
                        REG_OACTHST[1] = REG_OACTHST[0] - 8;
                    else if(uiHActive == 800)
                        REG_OACTHST[1] = REG_OACTHST[0] - 7;
                    else if(uiHActive == 848)
                        REG_OACTHST[1] = REG_OACTHST[0] - 7;
                    else if(uiHActive == 1152)
                        REG_OACTHST[1] = REG_OACTHST[0] - 6;
                    else if(uiHActive == 1360)
                        REG_OACTHST[1] = REG_OACTHST[0] - 4;
                    else if(uiHActive == 1366)
                        REG_OACTHST[1] = REG_OACTHST[0] - 4;
                    else if(uiHActive >= 1440)
                        REG_OACTHST[1] = REG_OACTHST[0] - 4;
                    else
                        REG_OACTHST[1] = REG_OACTHST[0] - 5;
                #endif
                    REG_OACTHW[0] = uiHActive/2+4;
                    REG_OACTHW[1] = uiHActive/2+4;
                }
                #ifdef AUTO_ASPECT_RATIO_WITH_3D    //H30K_Doulas_0053
                REG_OACTHST[1] -= PL_OWIN.HST;
                #endif

                if(m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiVActive >= m_sChannelInfo[ch_v].sOutputTimingInfo.uiVActive)  //H30K_Doulas_0034
                    REG_OACTVW = m_sChannelInfo[ch_v].sOutputTimingInfo.uiVActive + 1;
                else
                    REG_OACTVW = m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiVActive + 3;
                //REG_OACTVW = m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiVActive + 3; //H30K_Doulas_0034
                #else
                if(m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive >= 1920)
                {
                    REG_OACTHST[1] = REG_OACTHST[0];
                    REG_OACTHW[0] = m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive/2;
                    REG_OACTHW[1] = m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive/2;
                }
                else
                {
                    if(m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive == 640)
                        REG_OACTHST[1] = REG_OACTHST[0] - 9;
                    else if(m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive == 720)
                        REG_OACTHST[1] = REG_OACTHST[0] - 8;
                    else
                        REG_OACTHST[1] = REG_OACTHST[0] - 5;
                    REG_OACTHW[0] = m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive/2+4;
                    REG_OACTHW[1] = m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive/2+4;
                }

                REG_OACTVW = m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiVActive + 3;
                #endif
            }
            else if((m_sChannelInfo[eC341_CH_V0].ePanelTimingId == ePANEL_ID_WUXGA_240HZ) &&
                    (m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive == 1920 && m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive == 1080) &&
                    (m_sChannelInfo[ch_v].sInputTimingInfo.dFrameRate > 119.50 && m_sChannelInfo[ch_v].sInputTimingInfo.dFrameRate < 120.3)) //H30K_Doulas_0047 Modify//H2PF_Simon_0107
            {
                REG_OACTHW[0] = m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive/2;
                REG_OACTHW[1] = m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive/2;
            }
            break;
	}

    if(dvC341_Read(B8_OIMGCTCH1, obank_offset) != m_sChannelInfo[ch_v].ucOIMGCT)
    	dvC341_Write(B8_OIMGCTCH1,  m_sChannelInfo[ch_v].ucOIMGCT , obank_offset);

	dvC341_Write(B8_OACTHSTCH1,	REG_OACTHST[0], obank_offset);
	dvC341_Write(B8_OACTHWCH1,	REG_OACTHW[0], obank_offset);
	dvC341_Write(B8_OACTVSTCH1,	REG_OACTVST, obank_offset);
	dvC341_Write(B8_OACTVWCH1,	REG_OACTVW, obank_offset);

	g_B8_OACTVST[0] = REG_OACTVST;  //H30K_Doulas_0020
    g_B8_OACTVW[0]  = REG_OACTVW;   //H30K_Doulas_0020

	dvC341_Write(B8_OAOI1HSTCH1,	REG_OAOI1HST[0], obank_offset);
	dvC341_Write(B8_OAOI1HENDCH1,	REG_OAOI1HEND[0], obank_offset);
	dvC341_Write(B8_OAOI1VSTCH1,	REG_OAOI1VST, obank_offset);
	dvC341_Write(B8_OAOI1VENDCH1,	REG_OAOI1VEND, obank_offset);

    dbmsg("B8_OACTHSTCH1 = REG_OACTHST[0] = %d\n",      REG_OACTHST[0]);
    dbmsg("B8_OACTHWCH1  = REG_OACTHW[0]  = %d\n",       REG_OACTHW[0]);
    dbmsg("B8_OACTVSTCH1 = REG_OACTVST    = %d\n",         REG_OACTVST);
    dbmsg("B8_OACTVWCH1  = REG_OACTVW     = %d\n",          REG_OACTVW);

    dbmsg("B8_OAOI1HSTCH1  = REG_OAOI1HST[0]  = %d\n",     REG_OAOI1HST[0]);
    dbmsg("B8_OAOI1HENDCH1 = REG_OAOI1HEND[0] = %d\n",    REG_OAOI1HEND[0]);
    dbmsg("B8_OAOI1VSTCH1  = REG_OAOI1VST     = %d\n",        REG_OAOI1VST);
    dbmsg("B8_OAOI1VENDCH1 = REG_OAOI1VEND    = %d\n",       REG_OAOI1VEND);

    #if 0
	if (PM_FLIP[ch_v] == 1)
	{
		dvC341_Write(B8_OFLPCTCH1, 0x01, obank_offset);
	}
	else
	#endif
	{
		dvC341_Write(B8_OFLPCTCH1, 0x00, obank_offset);
	}

	dvC341_Write(B10_VZMADJCTCH1, REG_VZMADJCT, obank_offset);
	dvC341_Write(B10_HZOOMCTCH1, REG_HZOOMCT, obank_offset);
	dvC341_Write(B10_VZOOMCTCH1, REG_VZOOMCT, obank_offset);
	dvC341_Write(B10_HZSCLCH1, REG_HZSCL, obank_offset);
	dvC341_Write(B10_VZSCLCH1, REG_VZSCL, obank_offset);
	dvC341_Write(B10_HZINITEVCH1, REG_HZINIT[0], obank_offset);
	dvC341_Write(B10_HZINITODCH1, REG_HZINIT[0], obank_offset);

    if((m_sChannelInfo[eC341_CH_V0].ePanelTimingId == ePANEL_ID_WUXGA_240HZ) &&
       (m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive == 1920 && m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive == 1080) &&
       (m_sChannelInfo[ch_v].sInputTimingInfo.dFrameRate > 119.50 && m_sChannelInfo[ch_v].sInputTimingInfo.dFrameRate < 120.3)) //H30K_Doulas_0047 Modify   //H2PF_Simon_0107
	{
        if(ch_v == eC341_CH_V0)
        {
        	dvC341_Write(B22_OSFLD0CH1, PS_MEM_SC[0] + 0, 0);
        	dvC341_Write(B22_OSFLD1CH1, PS_MEM_SC[1] + 0, 0);
        	dvC341_Write(B22_OSFLD2CH1, PS_MEM_SC[2] + 0, 0);
        	dvC341_Write(B22_OSFLD3CH1, PS_MEM_SC[3] + 0, 0);
        	dvC341_Write(B22_OSFLD4CH1, PS_MEM_SC[4] + 0, 0);
        }

	    dvC341_Write(B22_OMWICH1, DEF_SCMWICH1_4K, obank_offset);
    }
    else if((m_sChannelInfo[eC341_CH_V0].ePanelTimingId != ePANEL_ID_1080P_120HZ) &&
            (m_sChannelInfo[eC341_CH_V0].ePanelTimingId != ePANEL_ID_WUXGA_120HZ)  //H30K_Doulas_0001
    )
    {
        if(ch_v == eC341_CH_V0)
        {
        	dvC341_Write(B22_OSFLD0CH1, PS_MEM_SC[0] + osfld_ofst[0], 0);
        	dvC341_Write(B22_OSFLD1CH1, PS_MEM_SC[1] + osfld_ofst[0], 0);
        	dvC341_Write(B22_OSFLD2CH1, PS_MEM_SC[2] + osfld_ofst[0], 0);
        	dvC341_Write(B22_OSFLD3CH1, PS_MEM_SC[3] + osfld_ofst[0], 0);
        	dvC341_Write(B22_OSFLD4CH1, PS_MEM_SC[4] + osfld_ofst[0], 0);
        }
        else if(ch_v == eC341_CH_V1)
        {
        	dvC341_Write(B54_OSFLD0CH3, PS_MEM_SC[0] + osfld_ofst[0], 0);
        	dvC341_Write(B54_OSFLD1CH3, PS_MEM_SC[1] + osfld_ofst[0], 0);
        	dvC341_Write(B54_OSFLD2CH3, PS_MEM_SC[2] + osfld_ofst[0], 0);
        	dvC341_Write(B54_OSFLD3CH3, PS_MEM_SC[3] + osfld_ofst[0], 0);
        	dvC341_Write(B54_OSFLD4CH3, PS_MEM_SC[4] + osfld_ofst[0], 0);
        }

	    dvC341_Write(B22_OMWICH1, DEF_SCMWICH1_4K, obank_offset);
	}

    #if 0
	if ((PM_SCFMFMTENB[ch_v] == 2) && (((PS_MEM_SC[ch_v][0] + m20bitfmt_hstofst) % 2) == 0) && ((((PS_MEM_SC[ch_v][0] + osfld_ofst[1]) % 16) == 2) || (((PS_MEM_SC[ch_v][0] + osfld_ofst[1]) % 16) == 8) || (((PS_MEM_SC[ch_v][0] + osfld_ofst[1]) % 16) == 12))) {
		g_oimgct[ch_v][1] = g_oimgct[ch_v][1] | 0x20;
	}
	else if( (PM_SCFMFMTENB[ch_v] == 2) && (((PS_MEM_SC[ch_v][0] + m20bitfmt_hstofst) % 2) == 1) && ((((PS_MEM_SC[ch_v][0] + osfld_ofst[1]) % 16) == 0) || (((PS_MEM_SC[ch_v][0] + osfld_ofst[1]) % 16) == 4) || (((PS_MEM_SC[ch_v][0] + osfld_ofst[1]) % 16) == 10)) ){
		g_oimgct[ch_v][1] = g_oimgct[ch_v][1] | 0x20;
	}
	else if( (PM_SCFMFMTENB[ch_v] == 3) && (((PS_MEM_SC[ch_v][0] + osfld_ofst[1]) % 4) == 2) ){
		g_oimgct[ch_v][1] = g_oimgct[ch_v][1] | 0x20;
	}
	else
	#endif
	{
		m_sChannelInfo[ch_v].ucOIMGCT = m_sChannelInfo[ch_v].ucOIMGCT & 0xDF;
	}

    if(dvC341_Read(B24_OIMGCTCH2, obank_offset) != m_sChannelInfo[ch_v].ucOIMGCT)
	    dvC341_Write(B24_OIMGCTCH2,  m_sChannelInfo[ch_v].ucOIMGCT , obank_offset);

	dvC341_Write(B24_OACTHSTCH2, REG_OACTHST[1], obank_offset);
	dvC341_Write(B24_OACTHWCH2, REG_OACTHW[1], obank_offset);
	dvC341_Write(B24_OACTVSTCH2, REG_OACTVST, obank_offset);
	dvC341_Write(B24_OACTVWCH2, REG_OACTVW, obank_offset);

	g_B8_OACTVST[1] = REG_OACTVST;  //H30K_Doulas_0020
    g_B8_OACTVW[1]  = REG_OACTVW;   //H30K_Doulas_0020

	dvC341_Write(B24_OAOI1HSTCH2,	REG_OAOI1HST[1], obank_offset);
	dvC341_Write(B24_OAOI1HENDCH2,	REG_OAOI1HEND[1], obank_offset);
	dvC341_Write(B24_OAOI1VSTCH2,	REG_OAOI1VST, obank_offset);
	dvC341_Write(B24_OAOI1VENDCH2,	REG_OAOI1VEND, obank_offset);

    dbmsg("B24_OACTHSTCH2 = REG_OACTHST[1] = %d\n",      REG_OACTHST[1]);
    dbmsg("B24_OACTHWCH2  = REG_OACTHW[1]  = %d\n",       REG_OACTHW[1]);
    dbmsg("B24_OACTVSTCH2 = REG_OACTVST    = %d\n",         REG_OACTVST);
    dbmsg("B24_OACTVWCH2  = REG_OACTVW     = %d\n",          REG_OACTVW);

    dbmsg("B24_OAOI1HSTCH2  = REG_OAOI1HST[1]  = %d\n",     REG_OAOI1HST[1]);
    dbmsg("B24_OAOI1HENDCH2 = REG_OAOI1HEND[1] = %d\n",    REG_OAOI1HEND[1]);
    dbmsg("B24_OAOI1VSTCH2  = REG_OAOI1VST     = %d\n",        REG_OAOI1VST);
    dbmsg("B24_OAOI1VENDCH2 = REG_OAOI1VEND    = %d\n",       REG_OAOI1VEND);

    #if 0
	if (PM_FLIP[ch_v] == 1)
	{
		dvC341_Write(B24_OFLPCTCH2, 0x01, obank_offset);
	}
	else
	#endif
	{
		dvC341_Write(B24_OFLPCTCH2, 0x00, obank_offset);
	}

	dvC341_Write(B26_VZMADJCTCH2, REG_VZMADJCT, obank_offset);
	dvC341_Write(B26_HZOOMCTCH2, REG_HZOOMCT, obank_offset);
	dvC341_Write(B26_VZOOMCTCH2, REG_VZOOMCT, obank_offset);
	dvC341_Write(B26_HZSCLCH2, REG_HZSCL, obank_offset);
	dvC341_Write(B26_VZSCLCH2, REG_VZSCL, obank_offset);
	dvC341_Write(B26_HZINITEVCH2, REG_HZINIT[1], obank_offset);
	dvC341_Write(B26_HZINITODCH2, REG_HZINIT[1], obank_offset);

    if((m_sChannelInfo[eC341_CH_V0].ePanelTimingId == ePANEL_ID_WUXGA_240HZ) &&
       (m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive == 1920 && m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive == 1080) &&
       (m_sChannelInfo[ch_v].sInputTimingInfo.dFrameRate > 119.50 && m_sChannelInfo[ch_v].sInputTimingInfo.dFrameRate < 120.3)) //H30K_Doulas_0047 Modify //align with 1024 (寫ram一次的長度為1024), 可節省頻寬   //H2PF_Simon_0107
	{
	    if(ch_v == eC341_CH_V0)
        {
            if(FRAME_MEMORY_FORMAT == 20) //H2PF_Simon_0137
            {
            	dvC341_Write(B38_OSFLD0CH2, PS_MEM_SC[0] + 0xA00, 0);
            	dvC341_Write(B38_OSFLD1CH2, PS_MEM_SC[1] + 0xA00, 0);
            	dvC341_Write(B38_OSFLD2CH2, PS_MEM_SC[2] + 0xA00, 0);
            	dvC341_Write(B38_OSFLD3CH2, PS_MEM_SC[3] + 0xA00, 0);
            	dvC341_Write(B38_OSFLD4CH2, PS_MEM_SC[4] + 0xA00, 0);
        	}
        	else
        	{
            	dvC341_Write(B38_OSFLD0CH2, PS_MEM_SC[0] + 0xC00, 0);  //align with 1024 (寫ram一次的長度為1024), 可節省頻寬
            	dvC341_Write(B38_OSFLD1CH2, PS_MEM_SC[1] + 0xC00, 0);
            	dvC341_Write(B38_OSFLD2CH2, PS_MEM_SC[2] + 0xC00, 0);
            	dvC341_Write(B38_OSFLD3CH2, PS_MEM_SC[3] + 0xC00, 0);
            	dvC341_Write(B38_OSFLD4CH2, PS_MEM_SC[4] + 0xC00, 0);
        	}
        }

    	dvC341_Write(B38_OMWICH2, DEF_SCMWICH1_4K, obank_offset);
	}
    else if((m_sChannelInfo[eC341_CH_V0].ePanelTimingId != ePANEL_ID_1080P_120HZ) &&
            (m_sChannelInfo[eC341_CH_V0].ePanelTimingId != ePANEL_ID_WUXGA_120HZ)//H30K_Doulas_0001
    )
    {
        if(ch_v == eC341_CH_V0)
        {
        	dvC341_Write(B38_OSFLD0CH2, PS_MEM_SC[0] + osfld_ofst[1], 0);
        	dvC341_Write(B38_OSFLD1CH2, PS_MEM_SC[1] + osfld_ofst[1], 0);
        	dvC341_Write(B38_OSFLD2CH2, PS_MEM_SC[2] + osfld_ofst[1], 0);
        	dvC341_Write(B38_OSFLD3CH2, PS_MEM_SC[3] + osfld_ofst[1], 0);
        	dvC341_Write(B38_OSFLD4CH2, PS_MEM_SC[4] + osfld_ofst[1], 0);
        }
        else if(ch_v == eC341_CH_V1)
        {
        	dvC341_Write(B70_OSFLD0CH4, PS_MEM_SC[0] + osfld_ofst[1], 0);
        	dvC341_Write(B70_OSFLD1CH4, PS_MEM_SC[1] + osfld_ofst[1], 0);
        	dvC341_Write(B70_OSFLD2CH4, PS_MEM_SC[2] + osfld_ofst[1], 0);
        	dvC341_Write(B70_OSFLD3CH4, PS_MEM_SC[3] + osfld_ofst[1], 0);
        	dvC341_Write(B70_OSFLD4CH4, PS_MEM_SC[4] + osfld_ofst[1], 0);
        }

    	dvC341_Write(B38_OMWICH2, DEF_SCMWICH1_4K, obank_offset);
	}

	if (REG_HZSCL != 0) {
		dvC341_Write(B0_SELCOEFSET, 0x2e, 0);
		dvC341_Write(B0_SELLUT, 0x81 + 0x08*ch_v, 0);
		dvC341_Write(B0_SELLUT, 0x85 + 0x08*ch_v, 0);
	}

	if (REG_VZSCL != 0) {
		dvC341_Write(B0_SELCOEFSET, 0x2e, 0);
		dvC341_Write(B0_SELLUT, 0x80 + 0x08*ch_v, 0);
		dvC341_Write(B0_SELLUT, 0x84 + 0x08*ch_v, 0);
	}

    //dvC341_Write_ch_v(ch_v, B17_ILSCCTCH1, 0x00, CH_BANK_OFFSET);
	if(m_sChannelSetting[ch_v].cFREEZE == TRUE)
	{
	    dvC341_Freeze_Set(ch_v, FALSE);
    }

	if (ipen == 0) {
		dvC341_set_mct(ch_v, 0);
	}
	dvC341_Set_IP(ch_v);

    #if 1 //H30K_Doulas_0077
    //dvC341_set_rtct_normal(); //H30K_Doulas_0079//H30K_Doulas_0077
	if(!g_rtct)
	{
		if (m_sChannelInfo[ch_v].sInputTimingInfo.lSTATE > 0) {
			dvC341_Write(B0_FLDRTCT2CH12 + 0x200*ch_v, 0x01, 0);

			wait_loop = 0;
			rdata = dvC341_Read(B0_FLDRTCT2CH12 + 0x200*ch_v, 0);
			while (rdata & 0x70) {
				MS_SLEEP(3);
				wait_loop++;
				if (wait_loop > 20) break;
				rdata = dvC341_Read(B0_FLDRTCT2CH12 + 0x200*ch_v, 0);
			};

			dvC341_Write(B0_FLDRTCTCH12 + 0x200*ch_v, 0x00, 0);
			if (wait_loop > 20) {
				dvC341_wait1_povs(ch_v);
			}
		}
		else {
			dvC341_Write(B0_FLDRTCTCH12 + 0x200*ch_v, 0x00, 0);
			dvC341_wait1_povs(ch_v);
		}

		if ((REG_VSSCL != 0) && g_vzoom_flg[ch_v] == 1)
		{
			dvC341_Write(B17_VSHRNKCTCH1, 0x87, ibank_offset);
			dvC341_Write(B33_VSHRNKCTCH2, 0x87, ibank_offset);
		}
	}
	#endif

	if (ipen != 0) {
		dvC341_set_mct(ch_v, 0);
	}

#if 1  //H30K_Doulas_0077
#else
	dvC341_Write(B17_SBOUNDCH1, hfstvldch1 * 128 + vfstvld * 8, ibank_offset);
	//dvC341_Write(B17_HSHRNKCTCH1,	REG_HSHRNKCT | 0x80, ibank_offset);
	//dvC341_Write(B17_HSSCLCH1, REG_HSSCL, ibank_offset);
	//dvC341_Write(B17_VSHRNKCTCH1,	REG_VSHRNKCT | 0x80, ibank_offset);
	//dvC341_Write(B17_VSSCLCH1, REG_VSSCL, ibank_offset);
	dvC341_Write(B17_SHRNKADJCH1, 0x02, ibank_offset);
	dvC341_Write(B17_HSINITEVCH1, hsinitch1, ibank_offset);
	dvC341_Write(B17_HSINITODCH1, hsinitch1, ibank_offset);
	dvC341_Write(B17_VSINITEVCH1, vsinit, ibank_offset);
	dvC341_Write(B17_VSINITODCH1, vsinit, ibank_offset);

	dvC341_Write(B33_SBOUNDCH2, hfstvldch2 * 128 + vfstvld * 8, ibank_offset);
	//dvC341_Write(B33_HSHRNKCTCH2, REG_HSHRNKCT | 0x80, ibank_offset);
	//dvC341_Write(B33_HSSCLCH2, REG_HSSCL, ibank_offset);
	//dvC341_Write(B33_VSHRNKCTCH2, REG_VSHRNKCT | 0x80, ibank_offset);
	//dvC341_Write(B33_VSSCLCH2, REG_VSSCL, ibank_offset);
	dvC341_Write(B33_SHRNKADJCH2, 0x02, ibank_offset);
	dvC341_Write(B33_HSINITEVCH2,	hsinitch2, ibank_offset);
	dvC341_Write(B33_HSINITODCH2,	hsinitch2, ibank_offset);
	dvC341_Write(B33_VSINITEVCH2, vsinit, ibank_offset);
	dvC341_Write(B33_VSINITODCH2, vsinit, ibank_offset);
#endif
	if (REG_VSSCL != 0) g_vshrnk_flg[ch_v] = 1;
	else g_vshrnk_flg[ch_v] = 0;

	if (REG_VZSCL != 0) g_vzoom_flg[ch_v] = 1;
	else g_vzoom_flg[ch_v] = 0;
}

void dvC341_Set_Movement_NR(const eC341_CH_V ch_v)
{
	UINT32 ulval;
    UINT8  ucRegOffset = 0;

    if(ch_v >= eC341_CH_VNUMBER)
    {
        return;
    }

    //ucPM_MVFLT[ch_v] = 3;//default       //A70LV_Doulas_0145 remove
	ulval = ( ucPM_MVFLT[ch_v]==0) ? 0x00 : 0x1f;
    dvC341_Write_ch_v(ch_v, B20_MVNRCTCH1 ,ulval, 16);		//Movement NR control

	if((ch_v == eC341_CH_V1)&(m_sChannelSetting[eC341_CH_V0].cPIP_PBP_ON == FALSE))     //CH1 port 1(0,1,2) and PIP/PBP off
		dvC341_Write_ch_v(eC341_CH_V0, B20_MVNRCTCH1 ,0x00, 16);     //Movement NR control
//	else if((( PI_ACT_VW[0]==540 )&(PM_WARPING!=0))|(( PI_ACT_VW[0]==540 )&( PI_ACT_VW[1]==540)))
//		dvC341_Write(B11_MVNRCTCH1 ,0x03,0);     //Movement NR control
	else
		dvC341_Write_ch_v(eC341_CH_V0, B20_MVNRCTCH1 ,ulval, 16);      //Movement NR control


	if((ch_v == eC341_CH_V0)&(m_sChannelSetting[eC341_CH_V0].cPIP_PBP_ON == FALSE))     //CH1 port 0(0,1,2) and PIP/PBP off
        dvC341_Write_ch_v(eC341_CH_V1, B20_MVNRCTCH1 ,0x00, 16);   //Movement NR control
	else if (dvC341_PIP_Enable())                    //PIP on
        dvC341_Write_ch_v(eC341_CH_V1, B20_MVNRCTCH1 ,0x03, 16);
	else
        dvC341_Write_ch_v(eC341_CH_V1, B20_MVNRCTCH1 ,ulval, 16);      //PBP on

    if((dvC341_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_FRAMEPACKING) ||
       (dvC341_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_SIDEBYSIDE) ||
       (dvC341_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_TOPANDBOTTOM))     //A70LV_Doulas_0154
    {
        dvC341_Write_ch_v(ch_v, B20_MVNRCTCH1 ,ulval, 16);
    }


	//printf("PM_SEL_INPUT_MAIN=%d\n",PM_SEL_INPUT_MAIN);
	//printf("PM_DISPLAY_MODE=%d\n",PM_DISPLAY_MODE);

	for(UINT8 ch=0; ch<OUTPUT4K_CH_NUM; ch++)
    {
        UINT8 BankOffset = 16;
        ucRegOffset = ((ch_v * 2) + ch) * BankOffset;

    	if ( ucPM_MVFLT[ch_v]==0 )
    	{
    		dvC341_WriteToBuffer(B20_VFILCFACH1,0x00,ucRegOffset);
    		dvC341_WriteToBuffer(B20_HFILCFACH1,0x00,ucRegOffset); //Vertical & Horizontal movement gain
    		dvC341_WriteToBuffer(B20_VFILCFBCH1,0x00,ucRegOffset);
    		dvC341_WriteToBuffer(B20_HFILCFBCH1,0x00,ucRegOffset);
    		dvC341_WriteToBuffer(B20_VFILCFCCH1,0x00,ucRegOffset);
    		dvC341_WriteToBuffer(B20_HFILCFCCH1,0x00,ucRegOffset);
    		dvC341_WriteToBuffer(B20_VFILCFDCH1,0x00,ucRegOffset);
    		dvC341_WriteToBuffer(B20_HFILCFDCH1,0x00,ucRegOffset);
    	}
    	else if (  ucPM_MVFLT[ch_v]==1 )
    	{
    		dvC341_WriteToBuffer(B20_VFILCFACH1,0x04,ucRegOffset);
    		dvC341_WriteToBuffer(B20_HFILCFACH1,0x04,ucRegOffset);
    		dvC341_WriteToBuffer(B20_VFILCFBCH1,0x04,ucRegOffset);
    		dvC341_WriteToBuffer(B20_HFILCFBCH1,0x04,ucRegOffset);
    		dvC341_WriteToBuffer(B20_VFILCFCCH1,0x08,ucRegOffset);
    		dvC341_WriteToBuffer(B20_HFILCFCCH1,0x08,ucRegOffset);
    		dvC341_WriteToBuffer(B20_VFILCFDCH1,0x08,ucRegOffset);
    		dvC341_WriteToBuffer(B20_HFILCFDCH1,0x08,ucRegOffset);
    	}
    	else if (  ucPM_MVFLT[ch_v]==2 )
    	{
    		dvC341_WriteToBuffer(B20_VFILCFACH1,0x08,ucRegOffset);
    		dvC341_WriteToBuffer(B20_HFILCFACH1,0x08,ucRegOffset);
    		dvC341_WriteToBuffer(B20_VFILCFBCH1,0x08,ucRegOffset);
    		dvC341_WriteToBuffer(B20_HFILCFBCH1,0x08,ucRegOffset);
    		dvC341_WriteToBuffer(B20_VFILCFCCH1,0x08,ucRegOffset);
    		dvC341_WriteToBuffer(B20_HFILCFCCH1,0x08,ucRegOffset);
    		dvC341_WriteToBuffer(B20_VFILCFDCH1,0x10,ucRegOffset);
    		dvC341_WriteToBuffer(B20_HFILCFDCH1,0x10,ucRegOffset);
    	}
    	else if (  ucPM_MVFLT[ch_v]==3 )
    	{
    		dvC341_WriteToBuffer(B20_VFILCFACH1,0x08,ucRegOffset);
    		dvC341_WriteToBuffer(B20_HFILCFACH1,0x08,ucRegOffset);
    		dvC341_WriteToBuffer(B20_VFILCFBCH1,0x08,ucRegOffset);
    		dvC341_WriteToBuffer(B20_HFILCFBCH1,0x08,ucRegOffset);
    		dvC341_WriteToBuffer(B20_VFILCFCCH1,0x10,ucRegOffset);
    		dvC341_WriteToBuffer(B20_HFILCFCCH1,0x10,ucRegOffset);
    		dvC341_WriteToBuffer(B20_VFILCFDCH1,0x18,ucRegOffset);
    		dvC341_WriteToBuffer(B20_HFILCFDCH1,0x18,ucRegOffset);
    	}
    	else if (  ucPM_MVFLT[ch_v]==4 )
    	{
    		dvC341_WriteToBuffer(B20_VFILCFACH1,0x10,ucRegOffset);
    		dvC341_WriteToBuffer(B20_HFILCFACH1,0x10,ucRegOffset);
    		dvC341_WriteToBuffer(B20_VFILCFBCH1,0x10,ucRegOffset);
    		dvC341_WriteToBuffer(B20_HFILCFBCH1,0x10,ucRegOffset);
    		dvC341_WriteToBuffer(B20_VFILCFCCH1,0x18,ucRegOffset);
    		dvC341_WriteToBuffer(B20_HFILCFCCH1,0x18,ucRegOffset);
    		dvC341_WriteToBuffer(B20_VFILCFDCH1,0x1f,ucRegOffset);
    		dvC341_WriteToBuffer(B20_HFILCFDCH1,0x1f,ucRegOffset);
    	}
    }

    dvC341_Buffer_Flush();

}

static void dvC341_Set_Interlace_Regs(const eC341_CH_V ch_v)
{
	UINT32	ulpd_ctrl1 = 0;
	UINT32	ulval;
    UINT8   ucRegOffset = 0;
    UINT32  ulPM_22PDSQTH = 0,ulPM_23PDSQTH = 1;
    UINT32  ulpd_ctrl2 = 0x09;     //A70LV_Doulas_0145 Add


    if(ch_v >= eC341_CH_VNUMBER)
    {
        return;
    }


    if((m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.uiVActive == 480) &&    // 480i
       (m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.eScanMode == eSCAN_MODE_INTERLACE))
    {
        ulPM_22PDSQTH = 1;
        ulpd_ctrl1 = 0x32;
        #if 1    //A70LV_Doulas_0183 modify i-chip recommendation
        ulpd_ctrl2 = 0x09;
        #else
        if(m_sChannelInfo[ch_v].ucISYCT & 0x20)  //A70LV_Doulas_0145 fixed film mode
            ulpd_ctrl2 = 0x39;
        else
            ulpd_ctrl2 = 0x19;
        #endif
        LOG_MSG(db_DV_SCALER,"#480i\r\n");
        for(UINT8 ch=0; ch<OUTPUT4K_CH_NUM; ch++)
        {
            UINT8 BankOffset = 16;
            ucRegOffset = ((ch_v * 2) + ch) * BankOffset;
            dvC341_WriteToBuffer(B20_PDMV22THCH1		,0x0d, ucRegOffset);
            dvC341_WriteToBuffer(B20_PDMV23THCH1		,0x03, ucRegOffset);
            dvC341_WriteToBuffer(B20_PDSQTHCH1		    ,0x11, ucRegOffset);
            dvC341_WriteToBuffer(B20_MVACCSELCH1		,0x35, ucRegOffset);
            dvC341_WriteToBuffer(B20_PDACTSELCH1		,0x02, ucRegOffset);
            dvC341_WriteToBuffer(B20_PD22RSTCTCH1		,0x80, ucRegOffset);
            dvC341_WriteToBuffer(B20_PD23RSTTHCH1		,0x28, ucRegOffset);
            dvC341_WriteToBuffer(B20_PD2332CTRLCH1	    ,0xc4, ucRegOffset);
            dvC341_WriteToBuffer(B20_PD23322CTRLCH1	    ,0xc4, ucRegOffset);
            dvC341_WriteToBuffer(B20_PD23232CTRLCH1	    ,0xc4, ucRegOffset);
            dvC341_WriteToBuffer(B20_PD33CTRLCH1		,0xc4, ucRegOffset);
            dvC341_WriteToBuffer(B20_PD55CTRLCH1		,0xc2, ucRegOffset);
            dvC341_WriteToBuffer(B20_PD87CTRLCH1		,0xc2, ucRegOffset);

            dvC341_WriteToBuffer(B20_PD22ADJCH1		    ,0x46, ucRegOffset);
            dvC341_WriteToBuffer(B20_PDTESTCH1		    ,0x08, ucRegOffset);
            dvC341_WriteToBuffer(B20_PDNRCTCH1		    ,0x03, ucRegOffset);
            dvC341_WriteToBuffer(B20_PDMVNRCFACH1		,0x00, ucRegOffset);
            dvC341_WriteToBuffer(B20_PDMVNRCFBCH1		,0x00, ucRegOffset);
            dvC341_WriteToBuffer(B20_PDMVNRCFCCH1		,0x00, ucRegOffset);
            dvC341_WriteToBuffer(B20_PDMVNRCFDCH1		,0x09, ucRegOffset);
            dvC341_WriteToBuffer(B20_PDMV22NEWTHCH1		,0x00, ucRegOffset);
            dvC341_WriteToBuffer(B20_PDMV23NEWTHCH1		,0x00, ucRegOffset);
        }
    }
    else if((m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.uiVActive == 576) &&      // 576i
            (m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.eScanMode == eSCAN_MODE_INTERLACE))
    {
        ulPM_22PDSQTH = 1;
        ulpd_ctrl1 = 0x32;
        #if 1    //A70LV_Doulas_0183 modify i-chip recommendation
        ulpd_ctrl2 = 0x09;
        #else
        if(m_sChannelInfo[ch_v].ucISYCT & 0x20)  //A70LV_Doulas_0145 fixed film mode
            ulpd_ctrl2 = 0x39;
        else
            ulpd_ctrl2 = 0x19;
        #endif
        LOG_MSG(db_DV_SCALER,"#576i\r\n");
        for(UINT8 ch=0; ch<OUTPUT4K_CH_NUM; ch++)
        {
            UINT8 BankOffset = 16;
            ucRegOffset = ((ch_v * 2) + ch) * BankOffset;
            dvC341_WriteToBuffer(B20_PDMV22THCH1		,0x0d, ucRegOffset);
            dvC341_WriteToBuffer(B20_PDMV23THCH1		,0x03, ucRegOffset);
            dvC341_WriteToBuffer(B20_PDSQTHCH1		    ,0x11, ucRegOffset);
            dvC341_WriteToBuffer(B20_MVACCSELCH1		,0x45, ucRegOffset);
            dvC341_WriteToBuffer(B20_PDACTSELCH1		,0x02, ucRegOffset);
            dvC341_WriteToBuffer(B20_PD22RSTCTCH1		,0x80, ucRegOffset);
            dvC341_WriteToBuffer(B20_PD23RSTTHCH1		,0x28, ucRegOffset);
            dvC341_WriteToBuffer(B20_PD2332CTRLCH1	    ,0x00, ucRegOffset);
            dvC341_WriteToBuffer(B20_PD23322CTRLCH1	    ,0x00, ucRegOffset);
            dvC341_WriteToBuffer(B20_PD23232CTRLCH1	    ,0x00, ucRegOffset);
            dvC341_WriteToBuffer(B20_PD33CTRLCH1		,0x00, ucRegOffset);
            dvC341_WriteToBuffer(B20_PD55CTRLCH1		,0x00, ucRegOffset);
            dvC341_WriteToBuffer(B20_PD87CTRLCH1		,0x00, ucRegOffset);

            dvC341_WriteToBuffer(B20_PD22ADJCH1		    ,0x46, ucRegOffset);
            dvC341_WriteToBuffer(B20_PDTESTCH1		    ,0x08, ucRegOffset);
            dvC341_WriteToBuffer(B20_PDNRCTCH1		    ,0x03, ucRegOffset);
            dvC341_WriteToBuffer(B20_PDMVNRCFACH1		,0x00, ucRegOffset);
            dvC341_WriteToBuffer(B20_PDMVNRCFBCH1		,0x00, ucRegOffset);
            dvC341_WriteToBuffer(B20_PDMVNRCFCCH1		,0x00, ucRegOffset);
            dvC341_WriteToBuffer(B20_PDMVNRCFDCH1		,0x09, ucRegOffset);
            dvC341_WriteToBuffer(B20_PDMV22NEWTHCH1		,0x00, ucRegOffset);
            dvC341_WriteToBuffer(B20_PDMV23NEWTHCH1		,0x00, ucRegOffset);
        }
    }
    else if((m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.uiVActive == 1080) &&
            (m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.eScanMode == eSCAN_MODE_INTERLACE) &&
            (m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.dFrameRate > 59.80 && m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.dFrameRate < 60.2)) //H30K_Doulas_0001
         //   (m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.dFrameRate == 60))       // 1080i 60Hz
    {
        ulpd_ctrl1 = 0x33;
        #if 1    //A70LV_Doulas_0183 modify i-chip recommendation
        ulpd_ctrl2 = 0x09;
        #else
        if(m_sChannelInfo[ch_v].ucISYCT & 0x20)  //A70LV_Doulas_0145 fixed film mode
            ulpd_ctrl2 = 0x39;
        else
            ulpd_ctrl2 = 0x19;
        #endif
        LOG_MSG(db_DV_SCALER,"#1080i60\r\n");
        for(UINT8 ch=0; ch<OUTPUT4K_CH_NUM; ch++)
        {
            UINT8 BankOffset = 16;
            ucRegOffset = ((ch_v * 2) + ch) * BankOffset;
            dvC341_WriteToBuffer(B20_PDMV22THCH1		,0x1a, ucRegOffset);
            dvC341_WriteToBuffer(B20_PDMV23THCH1		,0x10, ucRegOffset);
            dvC341_WriteToBuffer(B20_PDSQTHCH1		    ,0x11, ucRegOffset);
            dvC341_WriteToBuffer(B20_MVACCSELCH1		,0x8e, ucRegOffset);
            dvC341_WriteToBuffer(B20_PDACTSELCH1		,0x06, ucRegOffset);
            dvC341_WriteToBuffer(B20_PD22RSTCTCH1		,0x80, ucRegOffset);
            dvC341_WriteToBuffer(B20_PD23RSTTHCH1		,0xc0, ucRegOffset);
            dvC341_WriteToBuffer(B20_PD2332CTRLCH1	    ,0xc1, ucRegOffset);
            dvC341_WriteToBuffer(B20_PD23322CTRLCH1	    ,0xc1, ucRegOffset);
            dvC341_WriteToBuffer(B20_PD23232CTRLCH1	    ,0xc1, ucRegOffset);
            dvC341_WriteToBuffer(B20_PD33CTRLCH1		,0x00, ucRegOffset);
            dvC341_WriteToBuffer(B20_PD55CTRLCH1		,0xc1, ucRegOffset);
            dvC341_WriteToBuffer(B20_PD87CTRLCH1		,0xc1, ucRegOffset);

            dvC341_WriteToBuffer(B20_PD22ADJCH1		    ,0x44, ucRegOffset);
            dvC341_WriteToBuffer(B20_PDTESTCH1		    ,0x08, ucRegOffset);
            dvC341_WriteToBuffer(B20_PDNRCTCH1		    ,0x03, ucRegOffset);
            dvC341_WriteToBuffer(B20_PDMVNRCFACH1		,0x00, ucRegOffset);
            dvC341_WriteToBuffer(B20_PDMVNRCFBCH1		,0x00, ucRegOffset);
            dvC341_WriteToBuffer(B20_PDMVNRCFCCH1		,0x02, ucRegOffset);
            dvC341_WriteToBuffer(B20_PDMVNRCFDCH1		,0x0f, ucRegOffset);
            dvC341_WriteToBuffer(B20_PDMV22NEWTHCH1		,0x00, ucRegOffset);
            dvC341_WriteToBuffer(B20_PDMV23NEWTHCH1		,0x00, ucRegOffset);
        }
    }
    else if((m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.uiVActive == 1080) &&
            (m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.eScanMode == eSCAN_MODE_INTERLACE) &&
            (m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.dFrameRate > 49.80 && m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.dFrameRate < 50.2)) //H30K_Doulas_0001
            //(m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.dFrameRate == 50))       // 1080i 50Hz
    {
        ulpd_ctrl1 = 0x33;
        #if 1    //A70LV_Doulas_0183 modify i-chip recommendation
        ulpd_ctrl2 = 0x09;
        #else
        if(m_sChannelInfo[ch_v].ucISYCT & 0x20)  //A70LV_Doulas_0145 fixed film mode
            ulpd_ctrl2 = 0x39;
        else
            ulpd_ctrl2 = 0x19;
        #endif
        LOG_MSG(db_DV_SCALER,"#1080i50\r\n");
        for(UINT8 ch=0; ch<OUTPUT4K_CH_NUM; ch++)
        {
            UINT8 BankOffset = 16;
            ucRegOffset = ((ch_v * 2) + ch) * BankOffset;
            dvC341_WriteToBuffer(B20_PDMV22THCH1		,0x1a, ucRegOffset);
            dvC341_WriteToBuffer(B20_PDMV23THCH1		,0x10, ucRegOffset);
            dvC341_WriteToBuffer(B20_PDSQTHCH1		    ,0x11, ucRegOffset);
            dvC341_WriteToBuffer(B20_MVACCSELCH1		,0x8e, ucRegOffset);
            dvC341_WriteToBuffer(B20_PDACTSELCH1		,0x06, ucRegOffset);
            dvC341_WriteToBuffer(B20_PD22RSTCTCH1		,0x80, ucRegOffset);
            dvC341_WriteToBuffer(B20_PD23RSTTHCH1		,0xc0, ucRegOffset);
            dvC341_WriteToBuffer(B20_PD2332CTRLCH1	    ,0x00, ucRegOffset);
            dvC341_WriteToBuffer(B20_PD23322CTRLCH1	    ,0x00, ucRegOffset);
            dvC341_WriteToBuffer(B20_PD23232CTRLCH1	    ,0x00, ucRegOffset);
            dvC341_WriteToBuffer(B20_PD33CTRLCH1		,0x00, ucRegOffset);
            dvC341_WriteToBuffer(B20_PD55CTRLCH1		,0x00, ucRegOffset);
            dvC341_WriteToBuffer(B20_PD87CTRLCH1		,0x00, ucRegOffset);

            dvC341_WriteToBuffer(B20_PD22ADJCH1		    ,0x44, ucRegOffset);
            dvC341_WriteToBuffer(B20_PDTESTCH1		    ,0x08, ucRegOffset);
            dvC341_WriteToBuffer(B20_PDNRCTCH1		    ,0x03, ucRegOffset);
            dvC341_WriteToBuffer(B20_PDMVNRCFACH1		,0x00, ucRegOffset);
            dvC341_WriteToBuffer(B20_PDMVNRCFBCH1		,0x00, ucRegOffset);
            dvC341_WriteToBuffer(B20_PDMVNRCFCCH1		,0x02, ucRegOffset);
            dvC341_WriteToBuffer(B20_PDMVNRCFDCH1		,0x0f, ucRegOffset);
            dvC341_WriteToBuffer(B20_PDMV22NEWTHCH1		,0x00, ucRegOffset);
            dvC341_WriteToBuffer(B20_PDMV23NEWTHCH1		,0x00, ucRegOffset);
        }
    }
    else //H30K_Doulas_0001
    {
        ulval = (ulPM_22PDSQTH<<4)|(ulPM_23PDSQTH);
	    dvC341_Write_ch_v(ch_v, B20_PDSQTHCH1  ,ulval, 16);       //Pulldown sequence threshold
    }

	//ulpd_ctrl1 = PM_22PD[ch] ? ulpd_ctrl1|0x40 : ulpd_ctrl1;
	//ulpd_ctrl1 = PM_23PD[ch] ? ulpd_ctrl1|0x80 : ulpd_ctrl1;
    if(m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.eScanMode == eSCAN_MODE_INTERLACE)
        ulpd_ctrl1 = ulpd_ctrl1 | 0x40 | 0x80;
	dvC341_Write_ch_v(ch_v, B20_PDCTRL1CH1 ,ulpd_ctrl1, 16);   //Pulldown control 1

    //ulval = 0x09;   //A70LV_Doulas_0145 remove
	//ulval = ( PM_24SF[ch]==0 ) ? ulval : (ulval|0x10);
	dvC341_Write_ch_v(ch_v, B20_PDCTRL2CH1 ,ulpd_ctrl2, 16);   //Pulldown control 2 //A70LV_Doulas_0145

	//ulval = (ulPM_22PDSQTH<<4)|(ulPM_23PDSQTH);
	//dvC341_Write_ch_v(ch_v, B20_PDSQTHCH1  ,ulval, 16);       //H30K_Doulas_0001 move//Pulldown sequence threshold

    dvC341_Buffer_Flush();
}

BOOL dvC341_PIP_Enable(void)
{
    if((m_sChannelSetting[eC341_CH_V0].cPIP_PBP_ON == TRUE) &&
       (m_sChannelSetting[eC341_CH_V0].cMain_Layout >= eCM_MAIN_LAYOUT_PIP_BOTTOM_RIGHT &&
        m_sChannelSetting[eC341_CH_V0].cMain_Layout <= eCM_MAIN_LAYOUT_PIP_TOP_RIGHT))
       return TRUE;
    else
       return FALSE;
}

BOOL dvC341_PBP_Enable(void)
{
    if((m_sChannelSetting[eC341_CH_V0].cPIP_PBP_ON == TRUE) &&
       (m_sChannelSetting[eC341_CH_V0].cMain_Layout <= eCM_MAIN_LAYOUT_PBP_MAIN_BOTTOM))
       return TRUE;
    else
       return FALSE;
}

BOOL dvC341_PIP_PBP_Enable(void)        //A70LV_Doulas_0226
{
    if(m_sChannelSetting[eC341_CH_V0].cPIP_PBP_ON == TRUE)
       return TRUE;
    else
       return FALSE;
}

static void dvC341_Set_IPMemory_2K(const eC341_CH_V ch_v)
{
    if(ch_v >= eC341_CH_VNUMBER)
    {
        return;
    }

    UINT8 ucRegOffset = 0;
    UINT8 BankOffset = 16;
    ucRegOffset = (ch_v == eC341_CH_V0) ? 0 : (BankOffset*OUTPUT4K_CH_NUM);

	UINT32 mwi = DEF_IPMWICH1_4K & 0x1f;
	UINT32 val = mwi * 1024;

    if(m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.eScanMode == eSCAN_MODE_PROGRESSIVE)
    {
        switch(dvC341_Input_3D_Format_Config_Get())        //A70LV_Doulas_0154 modify
        {
            case eINPUT_3D_TYPE_FRAMEPACKING:
            case eINPUT_3D_TYPE_TOPANDBOTTOM:
                if(ch_v == eC341_CH_V0)
                {
                    dvC341_WriteToBuffer(B23_IPOSFLD0CH1, DEF_IP_I_ISFLD0CH1_4K+val,   ucRegOffset);
                    dvC341_WriteToBuffer(B23_IPOSFLD1CH1, DEF_IP_I_ISFLD0CH1_4K,       ucRegOffset);
                    dvC341_WriteToBuffer(B23_IPOSFLD2CH1, DEF_IP_I_ISFLD1CH1_4K+val,   ucRegOffset); //simon check   //A70LV_Doulas_0009 modify image issue
                    dvC341_WriteToBuffer(B23_IPOSFLD3CH1, DEF_IP_I_ISFLD1CH1_4K,       ucRegOffset); //simon check   //A70LV_Doulas_0009 modify image issue
                    dvC341_WriteToBuffer(B23_IPISFLD0CH1, DEF_IP_I_ISFLD0CH1_4K+val,   ucRegOffset);
                    dvC341_WriteToBuffer(B23_IPISFLD1CH1, DEF_IP_I_ISFLD0CH1_4K,       ucRegOffset);
                    dvC341_WriteToBuffer(B23_IPISFLD2CH1, DEF_IP_I_ISFLD1CH1_4K+val,   ucRegOffset); //simon check   //A70LV_Doulas_0009 modify image issue
                    dvC341_WriteToBuffer(B23_IPISFLD3CH1, DEF_IP_I_ISFLD1CH1_4K,       ucRegOffset); //simon check   //A70LV_Doulas_0009 modify image issue

                    dvC341_WriteToBuffer(B23_IPOSFLD0CH1, DEF_IP_I_ISFLD0CH1_4K+val,   ucRegOffset + BankOffset);
                    dvC341_WriteToBuffer(B23_IPOSFLD1CH1, DEF_IP_I_ISFLD0CH1_4K,       ucRegOffset + BankOffset);
                    dvC341_WriteToBuffer(B23_IPOSFLD2CH1, DEF_IP_I_ISFLD1CH1_4K+val,   ucRegOffset + BankOffset); //simon check   //A70LV_Doulas_0009 modify image issue
                    dvC341_WriteToBuffer(B23_IPOSFLD3CH1, DEF_IP_I_ISFLD1CH1_4K,       ucRegOffset + BankOffset); //simon check   //A70LV_Doulas_0009 modify image issue
                    dvC341_WriteToBuffer(B23_IPISFLD0CH1, DEF_IP_I_ISFLD0CH1_4K+val,   ucRegOffset + BankOffset);
                    dvC341_WriteToBuffer(B23_IPISFLD1CH1, DEF_IP_I_ISFLD0CH1_4K,       ucRegOffset + BankOffset);
                    dvC341_WriteToBuffer(B23_IPISFLD2CH1, DEF_IP_I_ISFLD1CH1_4K+val,   ucRegOffset + BankOffset); //simon check   //A70LV_Doulas_0009 modify image issue
                    dvC341_WriteToBuffer(B23_IPISFLD3CH1, DEF_IP_I_ISFLD1CH1_4K,       ucRegOffset + BankOffset); //simon check   //A70LV_Doulas_0009 modify image issue
                }
                else if(ch_v == eC341_CH_V1)
                {
                    dvC341_WriteToBuffer(B23_IPOSFLD0CH1, DEF_IP_I_ISFLD0CH2_4K+val,   ucRegOffset);
                    dvC341_WriteToBuffer(B23_IPOSFLD1CH1, DEF_IP_I_ISFLD0CH2_4K,       ucRegOffset);
                    dvC341_WriteToBuffer(B23_IPOSFLD2CH1, DEF_IP_I_ISFLD1CH2_4K+val,   ucRegOffset); //simon check   //A70LV_Doulas_0009 modify image issue
                    dvC341_WriteToBuffer(B23_IPOSFLD3CH1, DEF_IP_I_ISFLD1CH2_4K,       ucRegOffset); //simon check   //A70LV_Doulas_0009 modify image issue
                    dvC341_WriteToBuffer(B23_IPISFLD0CH1, DEF_IP_I_ISFLD0CH2_4K+val,   ucRegOffset);
                    dvC341_WriteToBuffer(B23_IPISFLD1CH1, DEF_IP_I_ISFLD0CH2_4K,       ucRegOffset);
                    dvC341_WriteToBuffer(B23_IPISFLD2CH1, DEF_IP_I_ISFLD1CH2_4K+val,   ucRegOffset); //simon check   //A70LV_Doulas_0009 modify image issue
                    dvC341_WriteToBuffer(B23_IPISFLD3CH1, DEF_IP_I_ISFLD1CH2_4K,       ucRegOffset); //simon check   //A70LV_Doulas_0009 modify image issue

                    dvC341_WriteToBuffer(B23_IPOSFLD0CH1, DEF_IP_I_ISFLD0CH2_4K+val,   ucRegOffset + BankOffset);
                    dvC341_WriteToBuffer(B23_IPOSFLD1CH1, DEF_IP_I_ISFLD0CH2_4K,       ucRegOffset + BankOffset);
                    dvC341_WriteToBuffer(B23_IPOSFLD2CH1, DEF_IP_I_ISFLD1CH2_4K+val,   ucRegOffset + BankOffset); //simon check   //A70LV_Doulas_0009 modify image issue
                    dvC341_WriteToBuffer(B23_IPOSFLD3CH1, DEF_IP_I_ISFLD1CH2_4K,       ucRegOffset + BankOffset); //simon check   //A70LV_Doulas_0009 modify image issue
                    dvC341_WriteToBuffer(B23_IPISFLD0CH1, DEF_IP_I_ISFLD0CH2_4K+val,   ucRegOffset + BankOffset);
                    dvC341_WriteToBuffer(B23_IPISFLD1CH1, DEF_IP_I_ISFLD0CH2_4K,       ucRegOffset + BankOffset);
                    dvC341_WriteToBuffer(B23_IPISFLD2CH1, DEF_IP_I_ISFLD1CH2_4K+val,   ucRegOffset + BankOffset); //simon check   //A70LV_Doulas_0009 modify image issue
                    dvC341_WriteToBuffer(B23_IPISFLD3CH1, DEF_IP_I_ISFLD1CH2_4K,       ucRegOffset + BankOffset); //simon check   //A70LV_Doulas_0009 modify image issue
                }
                dvC341_WriteToBuffer(B23_IPYOFFCH1,  DEF_IPYOFFCH1_4K   , ucRegOffset);		//IP conversion luminance address offset
                dvC341_WriteToBuffer(B22_TMSASELCH1,    0x01            , ucRegOffset);		//Temporal NR memory read start address select

                dvC341_WriteToBuffer(B23_IPYOFFCH1,  DEF_IPYOFFCH1_4K   , ucRegOffset + BankOffset);		//IP conversion luminance address offset
                dvC341_WriteToBuffer(B22_TMSASELCH1,    0x01            , ucRegOffset + BankOffset);		//Temporal NR memory read start address select
            break;

            default:
                if(ch_v == eC341_CH_V0)
                {
                    //Setup De-interlacer
                    dvC341_WriteToBuffer(B23_IPOSFLD0CH1, DEF_IP_P_ISFLD0CH1_4K,    ucRegOffset);
                    dvC341_WriteToBuffer(B23_IPOSFLD1CH1, DEF_IP_P_ISFLD1CH1_4K,    ucRegOffset);
                    dvC341_WriteToBuffer(B23_IPOSFLD2CH1, DEF_IP_P_ISFLD0CH1_4K,    ucRegOffset);
                    dvC341_WriteToBuffer(B23_IPOSFLD3CH1, DEF_IP_P_ISFLD1CH1_4K,    ucRegOffset);
                    dvC341_WriteToBuffer(B23_IPISFLD0CH1, DEF_IP_P_ISFLD0CH1_4K,    ucRegOffset);
                    dvC341_WriteToBuffer(B23_IPISFLD1CH1, DEF_IP_P_ISFLD1CH1_4K,    ucRegOffset);
                    dvC341_WriteToBuffer(B23_IPISFLD2CH1, DEF_IP_P_ISFLD0CH1_4K,    ucRegOffset);
                    dvC341_WriteToBuffer(B23_IPISFLD3CH1, DEF_IP_P_ISFLD1CH1_4K,    ucRegOffset);

                    dvC341_WriteToBuffer(B23_IPOSFLD0CH1, DEF_IP_P_ISFLD0CH1_4K,    ucRegOffset + BankOffset);
                    dvC341_WriteToBuffer(B23_IPOSFLD1CH1, DEF_IP_P_ISFLD1CH1_4K,    ucRegOffset + BankOffset);
                    dvC341_WriteToBuffer(B23_IPOSFLD2CH1, DEF_IP_P_ISFLD0CH1_4K,    ucRegOffset + BankOffset);
                    dvC341_WriteToBuffer(B23_IPOSFLD3CH1, DEF_IP_P_ISFLD1CH1_4K,    ucRegOffset + BankOffset);
                    dvC341_WriteToBuffer(B23_IPISFLD0CH1, DEF_IP_P_ISFLD0CH1_4K,    ucRegOffset + BankOffset);
                    dvC341_WriteToBuffer(B23_IPISFLD1CH1, DEF_IP_P_ISFLD1CH1_4K,    ucRegOffset + BankOffset);
                    dvC341_WriteToBuffer(B23_IPISFLD2CH1, DEF_IP_P_ISFLD0CH1_4K,    ucRegOffset + BankOffset);
                    dvC341_WriteToBuffer(B23_IPISFLD3CH1, DEF_IP_P_ISFLD1CH1_4K,    ucRegOffset + BankOffset);
                }
                else if(ch_v == eC341_CH_V1)
                {
                    dvC341_WriteToBuffer(B23_IPOSFLD0CH1, DEF_IP_P_ISFLD0CH2_4K, ucRegOffset);
                    dvC341_WriteToBuffer(B23_IPOSFLD1CH1, DEF_IP_P_ISFLD1CH2_4K, ucRegOffset);
                    dvC341_WriteToBuffer(B23_IPOSFLD2CH1, DEF_IP_P_ISFLD0CH2_4K, ucRegOffset);
                    dvC341_WriteToBuffer(B23_IPOSFLD3CH1, DEF_IP_P_ISFLD1CH2_4K, ucRegOffset);
                    dvC341_WriteToBuffer(B23_IPISFLD0CH1, DEF_IP_P_ISFLD0CH2_4K, ucRegOffset);
                    dvC341_WriteToBuffer(B23_IPISFLD1CH1, DEF_IP_P_ISFLD1CH2_4K, ucRegOffset);
                    dvC341_WriteToBuffer(B23_IPISFLD2CH1, DEF_IP_P_ISFLD0CH2_4K, ucRegOffset);
                    dvC341_WriteToBuffer(B23_IPISFLD3CH1, DEF_IP_P_ISFLD1CH2_4K, ucRegOffset);

                    dvC341_WriteToBuffer(B23_IPOSFLD0CH1, DEF_IP_P_ISFLD0CH2_4K, ucRegOffset + BankOffset);
                    dvC341_WriteToBuffer(B23_IPOSFLD1CH1, DEF_IP_P_ISFLD1CH2_4K, ucRegOffset + BankOffset);
                    dvC341_WriteToBuffer(B23_IPOSFLD2CH1, DEF_IP_P_ISFLD0CH2_4K, ucRegOffset + BankOffset);
                    dvC341_WriteToBuffer(B23_IPOSFLD3CH1, DEF_IP_P_ISFLD1CH2_4K, ucRegOffset + BankOffset);
                    dvC341_WriteToBuffer(B23_IPISFLD0CH1, DEF_IP_P_ISFLD0CH2_4K, ucRegOffset + BankOffset);
                    dvC341_WriteToBuffer(B23_IPISFLD1CH1, DEF_IP_P_ISFLD1CH2_4K, ucRegOffset + BankOffset);
                    dvC341_WriteToBuffer(B23_IPISFLD2CH1, DEF_IP_P_ISFLD0CH2_4K, ucRegOffset + BankOffset);
                    dvC341_WriteToBuffer(B23_IPISFLD3CH1, DEF_IP_P_ISFLD1CH2_4K, ucRegOffset + BankOffset);

                }
                dvC341_WriteToBuffer(B23_IPYOFFCH1,     0x000   , ucRegOffset);		//IP conversion luminance address offset
                dvC341_WriteToBuffer(B22_TMSASELCH1,    0x00    , ucRegOffset);		//Temporal NR memory read start address select

                dvC341_WriteToBuffer(B23_IPYOFFCH1,     0x000   , ucRegOffset + BankOffset);		//IP conversion luminance address offset
                dvC341_WriteToBuffer(B22_TMSASELCH1,    0x00    , ucRegOffset + BankOffset);		//Temporal NR memory read start address select

            break;
        }
    }
    else
    {
        switch(dvC341_Input_3D_Format_Config_Get())        //A70LV_Doulas_0154 modify
        {
            case eINPUT_3D_TYPE_SIDEBYSIDE: //H30K_Doulas_0001
                if(ch_v == eC341_CH_V0)
                {
                    //Setup De-interlacer
                    #if 0
                    dvC341_WriteToBuffer(B23_IPOSFLD0CH1, 0x29000000 + val,     ucRegOffset);
                    dvC341_WriteToBuffer(B23_IPOSFLD1CH1, 0x29000000,           ucRegOffset);
                    dvC341_WriteToBuffer(B23_IPOSFLD2CH1, 0x29339000 + val,     ucRegOffset);
                    dvC341_WriteToBuffer(B23_IPOSFLD3CH1, 0x29339000,           ucRegOffset);
                    dvC341_WriteToBuffer(B23_IPISFLD0CH1, 0x29000000 + val,     ucRegOffset);
                    dvC341_WriteToBuffer(B23_IPISFLD1CH1, 0x29000000,           ucRegOffset);
                    dvC341_WriteToBuffer(B23_IPISFLD2CH1, 0x29339000 + val,     ucRegOffset);
                    dvC341_WriteToBuffer(B23_IPISFLD3CH1, 0x29339000,           ucRegOffset);

                    dvC341_WriteToBuffer(B23_IPOSFLD0CH1, 0x2b000000 + val,     ucRegOffset + BankOffset);
                    dvC341_WriteToBuffer(B23_IPOSFLD1CH1, 0x2b000000,           ucRegOffset + BankOffset);
                    dvC341_WriteToBuffer(B23_IPOSFLD2CH1, 0x2b339000 + val,     ucRegOffset + BankOffset);
                    dvC341_WriteToBuffer(B23_IPOSFLD3CH1, 0x2b339000,           ucRegOffset + BankOffset);
                    dvC341_WriteToBuffer(B23_IPISFLD0CH1, 0x2b000000 + val,     ucRegOffset + BankOffset);
                    dvC341_WriteToBuffer(B23_IPISFLD1CH1, 0x2b000000,           ucRegOffset + BankOffset);
                    dvC341_WriteToBuffer(B23_IPISFLD2CH1, 0x2b339000 + val,     ucRegOffset + BankOffset);
                    dvC341_WriteToBuffer(B23_IPISFLD3CH1, 0x2b339000,           ucRegOffset + BankOffset);
                    #else
                    dvC341_WriteToBuffer(B23_IPOSFLD0CH1, DEF_IPOSFLD0CH1_2K_3D + val,     ucRegOffset); //H30K_Doulas_0002
                    dvC341_WriteToBuffer(B23_IPOSFLD1CH1, DEF_IPOSFLD0CH1_2K_3D,           ucRegOffset);
                    dvC341_WriteToBuffer(B23_IPOSFLD2CH1, DEF_IPOSFLD1CH1_2K_3D + val,     ucRegOffset);
                    dvC341_WriteToBuffer(B23_IPOSFLD3CH1, DEF_IPOSFLD1CH1_2K_3D,           ucRegOffset);
                    dvC341_WriteToBuffer(B23_IPISFLD0CH1, DEF_IPISFLD0CH1_2K_3D + val,     ucRegOffset);
                    dvC341_WriteToBuffer(B23_IPISFLD1CH1, DEF_IPISFLD0CH1_2K_3D,           ucRegOffset);
                    dvC341_WriteToBuffer(B23_IPISFLD2CH1, DEF_IPISFLD1CH1_2K_3D + val,     ucRegOffset);
                    dvC341_WriteToBuffer(B23_IPISFLD3CH1, DEF_IPISFLD1CH1_2K_3D,           ucRegOffset);

                    dvC341_WriteToBuffer(B23_IPOSFLD0CH1, DEF_IPOSFLD0CH2_2K_3D + val,     ucRegOffset + BankOffset);
                    dvC341_WriteToBuffer(B23_IPOSFLD1CH1, DEF_IPOSFLD0CH2_2K_3D,           ucRegOffset + BankOffset);
                    dvC341_WriteToBuffer(B23_IPOSFLD2CH1, DEF_IPOSFLD1CH2_2K_3D + val,     ucRegOffset + BankOffset);
                    dvC341_WriteToBuffer(B23_IPOSFLD3CH1, DEF_IPOSFLD1CH2_2K_3D,           ucRegOffset + BankOffset);
                    dvC341_WriteToBuffer(B23_IPISFLD0CH1, DEF_IPISFLD0CH2_2K_3D + val,     ucRegOffset + BankOffset);
                    dvC341_WriteToBuffer(B23_IPISFLD1CH1, DEF_IPISFLD0CH2_2K_3D,           ucRegOffset + BankOffset);
                    dvC341_WriteToBuffer(B23_IPISFLD2CH1, DEF_IPISFLD1CH2_2K_3D + val,     ucRegOffset + BankOffset);
                    dvC341_WriteToBuffer(B23_IPISFLD3CH1, DEF_IPISFLD1CH2_2K_3D,           ucRegOffset + BankOffset);
                    #endif

                    dvC341_WriteToBuffer(B23_MVFLDCH1,    DEF_MVFLDCH1_2K_3D_1, ucRegOffset); //H30K_Doulas_0001
                    dvC341_WriteToBuffer(B23_MVFLDCH1,    DEF_MVFLDCH1_2K_3D_2, ucRegOffset + BankOffset); //H30K_Doulas_0001
                }
                else if(ch_v == eC341_CH_V1)
                {
                    #if 0
                    dvC341_WriteToBuffer(B23_IPOSFLD0CH1, 0x29000000 + val,     ucRegOffset);
                    dvC341_WriteToBuffer(B23_IPOSFLD1CH1, 0x29000000,           ucRegOffset);
                    dvC341_WriteToBuffer(B23_IPOSFLD2CH1, 0x29339000 + val,     ucRegOffset);
                    dvC341_WriteToBuffer(B23_IPOSFLD3CH1, 0x29339000,           ucRegOffset);
                    dvC341_WriteToBuffer(B23_IPISFLD0CH1, 0x29000000 + val,     ucRegOffset);
                    dvC341_WriteToBuffer(B23_IPISFLD1CH1, 0x29000000,           ucRegOffset);
                    dvC341_WriteToBuffer(B23_IPISFLD2CH1, 0x29339000 + val,     ucRegOffset);
                    dvC341_WriteToBuffer(B23_IPISFLD3CH1, 0x29339000,           ucRegOffset);

                    dvC341_WriteToBuffer(B23_IPOSFLD0CH1, 0x2b000000 + val,     ucRegOffset + BankOffset);
                    dvC341_WriteToBuffer(B23_IPOSFLD1CH1, 0x2b000000,           ucRegOffset + BankOffset);
                    dvC341_WriteToBuffer(B23_IPOSFLD2CH1, 0x2b339000 + val,     ucRegOffset + BankOffset);
                    dvC341_WriteToBuffer(B23_IPOSFLD3CH1, 0x2b339000,           ucRegOffset + BankOffset);
                    dvC341_WriteToBuffer(B23_IPISFLD0CH1, 0x2b000000 + val,     ucRegOffset + BankOffset);
                    dvC341_WriteToBuffer(B23_IPISFLD1CH1, 0x2b000000,           ucRegOffset + BankOffset);
                    dvC341_WriteToBuffer(B23_IPISFLD2CH1, 0x2b339000 + val,     ucRegOffset + BankOffset);
                    dvC341_WriteToBuffer(B23_IPISFLD3CH1, 0x2b339000,           ucRegOffset + BankOffset);
                    #else
                    dvC341_WriteToBuffer(B23_IPOSFLD0CH1, DEF_IPOSFLD0CH1_2K_3D + val,     ucRegOffset);
                    dvC341_WriteToBuffer(B23_IPOSFLD1CH1, DEF_IPOSFLD0CH1_2K_3D,           ucRegOffset);
                    dvC341_WriteToBuffer(B23_IPOSFLD2CH1, DEF_IPOSFLD1CH1_2K_3D + val,     ucRegOffset);
                    dvC341_WriteToBuffer(B23_IPOSFLD3CH1, DEF_IPOSFLD1CH1_2K_3D,           ucRegOffset);
                    dvC341_WriteToBuffer(B23_IPISFLD0CH1, DEF_IPISFLD0CH1_2K_3D + val,     ucRegOffset);
                    dvC341_WriteToBuffer(B23_IPISFLD1CH1, DEF_IPISFLD0CH1_2K_3D,           ucRegOffset);
                    dvC341_WriteToBuffer(B23_IPISFLD2CH1, DEF_IPISFLD1CH1_2K_3D + val,     ucRegOffset);
                    dvC341_WriteToBuffer(B23_IPISFLD3CH1, DEF_IPISFLD1CH1_2K_3D,           ucRegOffset);

                    dvC341_WriteToBuffer(B23_IPOSFLD0CH1, DEF_IPOSFLD0CH2_2K_3D + val,     ucRegOffset + BankOffset);
                    dvC341_WriteToBuffer(B23_IPOSFLD1CH1, DEF_IPOSFLD0CH2_2K_3D,           ucRegOffset + BankOffset);
                    dvC341_WriteToBuffer(B23_IPOSFLD2CH1, DEF_IPOSFLD1CH2_2K_3D + val,     ucRegOffset + BankOffset);
                    dvC341_WriteToBuffer(B23_IPOSFLD3CH1, DEF_IPOSFLD1CH2_2K_3D,           ucRegOffset + BankOffset);
                    dvC341_WriteToBuffer(B23_IPISFLD0CH1, DEF_IPISFLD0CH2_2K_3D + val,     ucRegOffset + BankOffset);
                    dvC341_WriteToBuffer(B23_IPISFLD1CH1, DEF_IPISFLD0CH2_2K_3D,           ucRegOffset + BankOffset);
                    dvC341_WriteToBuffer(B23_IPISFLD2CH1, DEF_IPISFLD1CH2_2K_3D + val,     ucRegOffset + BankOffset);
                    dvC341_WriteToBuffer(B23_IPISFLD3CH1, DEF_IPISFLD1CH2_2K_3D,           ucRegOffset + BankOffset);
                    #endif

                    dvC341_WriteToBuffer(B23_MVFLDCH1,    DEF_MVFLDCH2_2K_3D_1, ucRegOffset); //H30K_Doulas_0001
                    dvC341_WriteToBuffer(B23_MVFLDCH1,    DEF_MVFLDCH2_2K_3D_2, ucRegOffset + BankOffset); //H30K_Doulas_0001
                }
            break;

            case eINPUT_3D_TYPE_FRAMEPACKING:       //A70LV_Doulas_0159
            case eINPUT_3D_TYPE_TOPANDBOTTOM:
                if(ch_v == eC341_CH_V0)
                {
                    dvC341_WriteToBuffer(B23_IPOSFLD0CH1, DEF_IP_I_ISFLD0CH1_4K+val,    ucRegOffset);
                    dvC341_WriteToBuffer(B23_IPOSFLD1CH1, DEF_IP_I_ISFLD0CH1_4K,        ucRegOffset);
                    dvC341_WriteToBuffer(B23_IPOSFLD2CH1, DEF_IP_I_ISFLD1CH1_4K+val,    ucRegOffset);
                    dvC341_WriteToBuffer(B23_IPOSFLD3CH1, DEF_IP_I_ISFLD1CH1_4K,        ucRegOffset);
                    dvC341_WriteToBuffer(B23_IPISFLD0CH1, DEF_IP_I_ISFLD0CH1_4K+val,    ucRegOffset);
                    dvC341_WriteToBuffer(B23_IPISFLD1CH1, DEF_IP_I_ISFLD0CH1_4K,        ucRegOffset);
                    dvC341_WriteToBuffer(B23_IPISFLD2CH1, DEF_IP_I_ISFLD1CH1_4K+val,    ucRegOffset);
                    dvC341_WriteToBuffer(B23_IPISFLD3CH1, DEF_IP_I_ISFLD1CH1_4K,        ucRegOffset);

                    dvC341_WriteToBuffer(B23_IPOSFLD0CH1, DEF_IP_I_ISFLD0CH1_4K+val,    ucRegOffset + BankOffset);
                    dvC341_WriteToBuffer(B23_IPOSFLD1CH1, DEF_IP_I_ISFLD0CH1_4K,        ucRegOffset + BankOffset);
                    dvC341_WriteToBuffer(B23_IPOSFLD2CH1, DEF_IP_I_ISFLD1CH1_4K+val,    ucRegOffset + BankOffset);
                    dvC341_WriteToBuffer(B23_IPOSFLD3CH1, DEF_IP_I_ISFLD1CH1_4K,        ucRegOffset + BankOffset);
                    dvC341_WriteToBuffer(B23_IPISFLD0CH1, DEF_IP_I_ISFLD0CH1_4K+val,    ucRegOffset + BankOffset);
                    dvC341_WriteToBuffer(B23_IPISFLD1CH1, DEF_IP_I_ISFLD0CH1_4K,        ucRegOffset + BankOffset);
                    dvC341_WriteToBuffer(B23_IPISFLD2CH1, DEF_IP_I_ISFLD1CH1_4K+val,    ucRegOffset + BankOffset);
                    dvC341_WriteToBuffer(B23_IPISFLD3CH1, DEF_IP_I_ISFLD1CH1_4K,        ucRegOffset + BankOffset);
                    dvC341_WriteToBuffer(B23_MVFLDCH1,    DEF_MVFLDCH1_4K,              ucRegOffset);
                    dvC341_WriteToBuffer(B23_MVFLDCH1,    DEF_MVFLDCH1_4K,              ucRegOffset + BankOffset);
                }
                else if(ch_v == eC341_CH_V1)
                {
                    dvC341_WriteToBuffer(B23_IPOSFLD0CH1, DEF_IP_I_ISFLD0CH2_4K+val,    ucRegOffset);
                    dvC341_WriteToBuffer(B23_IPOSFLD1CH1, DEF_IP_I_ISFLD0CH2_4K,        ucRegOffset);
                    dvC341_WriteToBuffer(B23_IPOSFLD2CH1, DEF_IP_I_ISFLD1CH2_4K+val,    ucRegOffset);
                    dvC341_WriteToBuffer(B23_IPOSFLD3CH1, DEF_IP_I_ISFLD1CH2_4K,        ucRegOffset);
                    dvC341_WriteToBuffer(B23_IPISFLD0CH1, DEF_IP_I_ISFLD0CH2_4K+val,    ucRegOffset);
                    dvC341_WriteToBuffer(B23_IPISFLD1CH1, DEF_IP_I_ISFLD0CH2_4K,        ucRegOffset);
                    dvC341_WriteToBuffer(B23_IPISFLD2CH1, DEF_IP_I_ISFLD1CH2_4K+val,    ucRegOffset);
                    dvC341_WriteToBuffer(B23_IPISFLD3CH1, DEF_IP_I_ISFLD1CH2_4K,        ucRegOffset);

                    dvC341_WriteToBuffer(B23_IPOSFLD0CH1, DEF_IP_I_ISFLD0CH2_4K+val,    ucRegOffset + BankOffset);
                    dvC341_WriteToBuffer(B23_IPOSFLD1CH1, DEF_IP_I_ISFLD0CH2_4K,        ucRegOffset + BankOffset);
                    dvC341_WriteToBuffer(B23_IPOSFLD2CH1, DEF_IP_I_ISFLD1CH2_4K+val,    ucRegOffset + BankOffset);
                    dvC341_WriteToBuffer(B23_IPOSFLD3CH1, DEF_IP_I_ISFLD1CH2_4K,        ucRegOffset + BankOffset);
                    dvC341_WriteToBuffer(B23_IPISFLD0CH1, DEF_IP_I_ISFLD0CH2_4K+val,    ucRegOffset + BankOffset);
                    dvC341_WriteToBuffer(B23_IPISFLD1CH1, DEF_IP_I_ISFLD0CH2_4K,        ucRegOffset + BankOffset);
                    dvC341_WriteToBuffer(B23_IPISFLD2CH1, DEF_IP_I_ISFLD1CH2_4K+val,    ucRegOffset + BankOffset);
                    dvC341_WriteToBuffer(B23_IPISFLD3CH1, DEF_IP_I_ISFLD1CH2_4K,        ucRegOffset + BankOffset);
                    dvC341_WriteToBuffer(B23_MVFLDCH1,    DEF_MVFLDCH2_4K,              ucRegOffset);
                    dvC341_WriteToBuffer(B23_MVFLDCH1,    DEF_MVFLDCH2_4K,              ucRegOffset + BankOffset);
                }
            break;

            default:
                if((m_sChannelInfo[eC341_CH_V0].ePanelTimingId == ePANEL_ID_1080P_120HZ) ||
                    (m_sChannelInfo[eC341_CH_V0].ePanelTimingId == ePANEL_ID_WUXGA_120HZ)) //H30K_Doulas_0005
                {
                    if(ch_v == eC341_CH_V0)
                    {
                        //Setup De-interlacer
                        dvC341_WriteToBuffer(B23_IPOSFLD0CH1, DEF_IPOSFLD0CH1_2K_3D + val,     ucRegOffset);
                        dvC341_WriteToBuffer(B23_IPOSFLD1CH1, DEF_IPOSFLD0CH1_2K_3D,           ucRegOffset);
                        dvC341_WriteToBuffer(B23_IPOSFLD2CH1, DEF_IPOSFLD1CH1_2K_3D + val,     ucRegOffset);
                        dvC341_WriteToBuffer(B23_IPOSFLD3CH1, DEF_IPOSFLD1CH1_2K_3D,           ucRegOffset);
                        dvC341_WriteToBuffer(B23_IPISFLD0CH1, DEF_IPISFLD0CH1_2K_3D + val,     ucRegOffset);
                        dvC341_WriteToBuffer(B23_IPISFLD1CH1, DEF_IPISFLD0CH1_2K_3D,           ucRegOffset);
                        dvC341_WriteToBuffer(B23_IPISFLD2CH1, DEF_IPISFLD1CH1_2K_3D + val,     ucRegOffset);
                        dvC341_WriteToBuffer(B23_IPISFLD3CH1, DEF_IPISFLD1CH1_2K_3D,           ucRegOffset);

                        dvC341_WriteToBuffer(B23_IPOSFLD0CH1, DEF_IPOSFLD0CH2_2K_3D + val,     ucRegOffset + BankOffset);
                        dvC341_WriteToBuffer(B23_IPOSFLD1CH1, DEF_IPOSFLD0CH2_2K_3D,           ucRegOffset + BankOffset);
                        dvC341_WriteToBuffer(B23_IPOSFLD2CH1, DEF_IPOSFLD1CH2_2K_3D + val,     ucRegOffset + BankOffset);
                        dvC341_WriteToBuffer(B23_IPOSFLD3CH1, DEF_IPOSFLD1CH2_2K_3D,           ucRegOffset + BankOffset);
                        dvC341_WriteToBuffer(B23_IPISFLD0CH1, DEF_IPISFLD0CH2_2K_3D + val,     ucRegOffset + BankOffset);
                        dvC341_WriteToBuffer(B23_IPISFLD1CH1, DEF_IPISFLD0CH2_2K_3D,           ucRegOffset + BankOffset);
                        dvC341_WriteToBuffer(B23_IPISFLD2CH1, DEF_IPISFLD1CH2_2K_3D + val,     ucRegOffset + BankOffset);
                        dvC341_WriteToBuffer(B23_IPISFLD3CH1, DEF_IPISFLD1CH2_2K_3D,           ucRegOffset + BankOffset);

                        dvC341_WriteToBuffer(B23_MVFLDCH1,    DEF_MVFLDCH1_2K_3D_1, ucRegOffset);
                        dvC341_WriteToBuffer(B23_MVFLDCH1,    DEF_MVFLDCH1_2K_3D_2, ucRegOffset + BankOffset);
                    }
                    else if(ch_v == eC341_CH_V1)
                    {
                        dvC341_WriteToBuffer(B23_IPOSFLD0CH1, DEF_IPOSFLD0CH1_2K_3D + val,     ucRegOffset);
                        dvC341_WriteToBuffer(B23_IPOSFLD1CH1, DEF_IPOSFLD0CH1_2K_3D,           ucRegOffset);
                        dvC341_WriteToBuffer(B23_IPOSFLD2CH1, DEF_IPOSFLD1CH1_2K_3D + val,     ucRegOffset);
                        dvC341_WriteToBuffer(B23_IPOSFLD3CH1, DEF_IPOSFLD1CH1_2K_3D,           ucRegOffset);
                        dvC341_WriteToBuffer(B23_IPISFLD0CH1, DEF_IPISFLD0CH1_2K_3D + val,     ucRegOffset);
                        dvC341_WriteToBuffer(B23_IPISFLD1CH1, DEF_IPISFLD0CH1_2K_3D,           ucRegOffset);
                        dvC341_WriteToBuffer(B23_IPISFLD2CH1, DEF_IPISFLD1CH1_2K_3D + val,     ucRegOffset);
                        dvC341_WriteToBuffer(B23_IPISFLD3CH1, DEF_IPISFLD1CH1_2K_3D,           ucRegOffset);

                        dvC341_WriteToBuffer(B23_IPOSFLD0CH1, DEF_IPOSFLD0CH2_2K_3D + val,     ucRegOffset + BankOffset);
                        dvC341_WriteToBuffer(B23_IPOSFLD1CH1, DEF_IPOSFLD0CH2_2K_3D,           ucRegOffset + BankOffset);
                        dvC341_WriteToBuffer(B23_IPOSFLD2CH1, DEF_IPOSFLD1CH2_2K_3D + val,     ucRegOffset + BankOffset);
                        dvC341_WriteToBuffer(B23_IPOSFLD3CH1, DEF_IPOSFLD1CH2_2K_3D,           ucRegOffset + BankOffset);
                        dvC341_WriteToBuffer(B23_IPISFLD0CH1, DEF_IPISFLD0CH2_2K_3D + val,     ucRegOffset + BankOffset);
                        dvC341_WriteToBuffer(B23_IPISFLD1CH1, DEF_IPISFLD0CH2_2K_3D,           ucRegOffset + BankOffset);
                        dvC341_WriteToBuffer(B23_IPISFLD2CH1, DEF_IPISFLD1CH2_2K_3D + val,     ucRegOffset + BankOffset);
                        dvC341_WriteToBuffer(B23_IPISFLD3CH1, DEF_IPISFLD1CH2_2K_3D,           ucRegOffset + BankOffset);

                        dvC341_WriteToBuffer(B23_MVFLDCH1,    DEF_MVFLDCH2_2K_3D_1, ucRegOffset);
                        dvC341_WriteToBuffer(B23_MVFLDCH1,    DEF_MVFLDCH2_2K_3D_2, ucRegOffset + BankOffset);
                    }
                }
                else
                {
                    if(ch_v == eC341_CH_V0)
                    {
                        //Setup De-interlacer
                        dvC341_WriteToBuffer(B23_IPOSFLD0CH1, DEF_IP_I_ISFLD0CH1_4K+val,    ucRegOffset);
                        dvC341_WriteToBuffer(B23_IPOSFLD1CH1, DEF_IP_I_ISFLD0CH1_4K,        ucRegOffset);
                        dvC341_WriteToBuffer(B23_IPOSFLD2CH1, DEF_IP_I_ISFLD1CH1_4K+val,    ucRegOffset);//A70LV_Doulas_0009 modify image issue
                        dvC341_WriteToBuffer(B23_IPOSFLD3CH1, DEF_IP_I_ISFLD1CH1_4K,        ucRegOffset);//A70LV_Doulas_0009 modify image issue
                        dvC341_WriteToBuffer(B23_IPISFLD0CH1, DEF_IP_I_ISFLD0CH1_4K+val,    ucRegOffset);
                        dvC341_WriteToBuffer(B23_IPISFLD1CH1, DEF_IP_I_ISFLD0CH1_4K,        ucRegOffset);
                        dvC341_WriteToBuffer(B23_IPISFLD2CH1, DEF_IP_I_ISFLD1CH1_4K+val,    ucRegOffset);//A70LV_Doulas_0009 modify image issue
                        dvC341_WriteToBuffer(B23_IPISFLD3CH1, DEF_IP_I_ISFLD1CH1_4K,        ucRegOffset);//A70LV_Doulas_0009 modify image issue

                        dvC341_WriteToBuffer(B23_IPOSFLD0CH1, DEF_IP_I_ISFLD0CH1_4K+val,    ucRegOffset + BankOffset);
                        dvC341_WriteToBuffer(B23_IPOSFLD1CH1, DEF_IP_I_ISFLD0CH1_4K,        ucRegOffset + BankOffset);
                        dvC341_WriteToBuffer(B23_IPOSFLD2CH1, DEF_IP_I_ISFLD1CH1_4K+val,    ucRegOffset + BankOffset);//A70LV_Doulas_0009 modify image issue
                        dvC341_WriteToBuffer(B23_IPOSFLD3CH1, DEF_IP_I_ISFLD1CH1_4K,        ucRegOffset + BankOffset);//A70LV_Doulas_0009 modify image issue
                        dvC341_WriteToBuffer(B23_IPISFLD0CH1, DEF_IP_I_ISFLD0CH1_4K+val,    ucRegOffset + BankOffset);
                        dvC341_WriteToBuffer(B23_IPISFLD1CH1, DEF_IP_I_ISFLD0CH1_4K,        ucRegOffset + BankOffset);
                        dvC341_WriteToBuffer(B23_IPISFLD2CH1, DEF_IP_I_ISFLD1CH1_4K+val,    ucRegOffset + BankOffset);//A70LV_Doulas_0009 modify image issue
                        dvC341_WriteToBuffer(B23_IPISFLD3CH1, DEF_IP_I_ISFLD1CH1_4K,        ucRegOffset + BankOffset);//A70LV_Doulas_0009 modify image issue
                        dvC341_WriteToBuffer(B23_MVFLDCH1,    DEF_MVFLDCH1_4K,              ucRegOffset);
                        dvC341_WriteToBuffer(B23_MVFLDCH1,    DEF_MVFLDCH1_4K,              ucRegOffset + BankOffset);
                    }
                    else if(ch_v == eC341_CH_V1)
                    {
                        dvC341_WriteToBuffer(B23_IPOSFLD0CH1, DEF_IP_I_ISFLD0CH2_4K+val,    ucRegOffset);
                        dvC341_WriteToBuffer(B23_IPOSFLD1CH1, DEF_IP_I_ISFLD0CH2_4K,        ucRegOffset);
                        dvC341_WriteToBuffer(B23_IPOSFLD2CH1, DEF_IP_I_ISFLD1CH2_4K+val,    ucRegOffset);//A70LV_Doulas_0009 modify image issue
                        dvC341_WriteToBuffer(B23_IPOSFLD3CH1, DEF_IP_I_ISFLD1CH2_4K,        ucRegOffset);//A70LV_Doulas_0009 modify image issue
                        dvC341_WriteToBuffer(B23_IPISFLD0CH1, DEF_IP_I_ISFLD0CH2_4K+val,    ucRegOffset);
                        dvC341_WriteToBuffer(B23_IPISFLD1CH1, DEF_IP_I_ISFLD0CH2_4K,        ucRegOffset);
                        dvC341_WriteToBuffer(B23_IPISFLD2CH1, DEF_IP_I_ISFLD1CH2_4K+val,    ucRegOffset);//A70LV_Doulas_0009 modify image issue
                        dvC341_WriteToBuffer(B23_IPISFLD3CH1, DEF_IP_I_ISFLD1CH2_4K,        ucRegOffset);//A70LV_Doulas_0009 modify image issue

                        dvC341_WriteToBuffer(B23_IPOSFLD0CH1, DEF_IP_I_ISFLD0CH2_4K+val,    ucRegOffset + BankOffset);
                        dvC341_WriteToBuffer(B23_IPOSFLD1CH1, DEF_IP_I_ISFLD0CH2_4K,        ucRegOffset + BankOffset);
                        dvC341_WriteToBuffer(B23_IPOSFLD2CH1, DEF_IP_I_ISFLD1CH2_4K+val,    ucRegOffset + BankOffset);//A70LV_Doulas_0009 modify image issue
                        dvC341_WriteToBuffer(B23_IPOSFLD3CH1, DEF_IP_I_ISFLD1CH2_4K,        ucRegOffset + BankOffset);//A70LV_Doulas_0009 modify image issue
                        dvC341_WriteToBuffer(B23_IPISFLD0CH1, DEF_IP_I_ISFLD0CH2_4K+val,    ucRegOffset + BankOffset);
                        dvC341_WriteToBuffer(B23_IPISFLD1CH1, DEF_IP_I_ISFLD0CH2_4K,        ucRegOffset + BankOffset);
                        dvC341_WriteToBuffer(B23_IPISFLD2CH1, DEF_IP_I_ISFLD1CH2_4K+val,    ucRegOffset + BankOffset);//A70LV_Doulas_0009 modify image issue
                        dvC341_WriteToBuffer(B23_IPISFLD3CH1, DEF_IP_I_ISFLD1CH2_4K,        ucRegOffset + BankOffset);//A70LV_Doulas_0009 modify image issue
                        dvC341_WriteToBuffer(B23_MVFLDCH1,    DEF_MVFLDCH2_4K,              ucRegOffset);
                        dvC341_WriteToBuffer(B23_MVFLDCH1,    DEF_MVFLDCH2_4K,              ucRegOffset + BankOffset);
                    }
                }
            break;
        }
        dvC341_WriteToBuffer(B23_IPYOFFCH1,  DEF_IPYOFFCH1_4K   , ucRegOffset);		//IP conversion luminance address offset
        dvC341_WriteToBuffer(B22_TMSASELCH1,    0x01            , ucRegOffset);		//Temporal NR memory read start address select

        dvC341_WriteToBuffer(B23_IPYOFFCH1,  DEF_IPYOFFCH1_4K   , ucRegOffset + BankOffset);		//IP conversion luminance address offset
        dvC341_WriteToBuffer(B22_TMSASELCH1,    0x01            , ucRegOffset + BankOffset);		//Temporal NR memory read start address select
    }

    dvC341_Buffer_Flush();
}

static void dvC341_Set_NT_TPX(const eC341_CH_V ch_v)
{
    if(ch_v >= eC341_CH_VNUMBER)
    {
        return;
    }

    if(m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.eScanMode == eSCAN_MODE_INTERLACE)    //A70LV_Doulas_0085 modify 1080P60 frame tear//A70LV_Doulas_0051 //A70LV_Doulas_0029 modify
        ucPM_TNR[ch_v] = m_sChannelSetting[ch_v].cTemporal_NR;
    else
        ucPM_TNR[ch_v] = eTEMPORAL_NR_OFF;

    for(UINT8 ch=0; ch<OUTPUT4K_CH_NUM; ch++)
    {
        UINT8 BankOffset = 16;
        UINT8 ucRegOffset = ((ch_v * 2) + ch) * BankOffset;

    	if ( ucPM_TNR[ch_v]==1)
        {//weak
    		dvC341_WriteToBuffer(B18_TMPPXNRCTCH1,  0x03, ucRegOffset);		//Temporal NR control
    		dvC341_WriteToBuffer(B18_TPXNRTH1CH1,	0x31, ucRegOffset);
    		dvC341_WriteToBuffer(B18_TPXNRTH2CH1,	0x75, ucRegOffset);
    		dvC341_WriteToBuffer(B18_TPXNRTH3CH1,	0xb9, ucRegOffset);
    		dvC341_WriteToBuffer(B18_TPXNRTH4CH1,	0xfd, ucRegOffset);
    		dvC341_WriteToBuffer(B18_TPXNRCF1CH1,	0x12, ucRegOffset);
    		dvC341_WriteToBuffer(B18_TPXNRCF2CH1,	0x14, ucRegOffset);
    		dvC341_WriteToBuffer(B18_TPXNRCF3CH1,	0x18, ucRegOffset);
    		dvC341_WriteToBuffer(B18_TPXNRCF4CH1,	0x1b, ucRegOffset);
    		dvC341_WriteToBuffer(B18_TPXNRCF5CH1,	0x1c, ucRegOffset);
    		dvC341_WriteToBuffer(B18_TPXNRCF6CH1,	0x1d, ucRegOffset);
    		dvC341_WriteToBuffer(B18_TPXNRCF7CH1,	0x1e, ucRegOffset);
    		dvC341_WriteToBuffer(B18_TPXNRCF8CH1,	0x1f, ucRegOffset);
    	}
        else if ( ucPM_TNR[ch_v]==2)
        {//mid
    		dvC341_WriteToBuffer(B18_TMPPXNRCTCH1,  0x03, ucRegOffset);
    		dvC341_WriteToBuffer(B18_TPXNRTH1CH1,	0x31, ucRegOffset);
    		dvC341_WriteToBuffer(B18_TPXNRTH2CH1,	0x75, ucRegOffset);
    		dvC341_WriteToBuffer(B18_TPXNRTH3CH1,	0xb9, ucRegOffset);
    		dvC341_WriteToBuffer(B18_TPXNRTH4CH1,	0xfd, ucRegOffset);
    		dvC341_WriteToBuffer(B18_TPXNRCF1CH1,	0x11, ucRegOffset);
    		dvC341_WriteToBuffer(B18_TPXNRCF2CH1,	0x12, ucRegOffset);
    		dvC341_WriteToBuffer(B18_TPXNRCF3CH1,	0x14, ucRegOffset);
    		dvC341_WriteToBuffer(B18_TPXNRCF4CH1,	0x16, ucRegOffset);
    		dvC341_WriteToBuffer(B18_TPXNRCF5CH1,	0x18, ucRegOffset);
    		dvC341_WriteToBuffer(B18_TPXNRCF6CH1,	0x1a, ucRegOffset);
    		dvC341_WriteToBuffer(B18_TPXNRCF7CH1,	0x1c, ucRegOffset);
    		dvC341_WriteToBuffer(B18_TPXNRCF8CH1,	0x1f, ucRegOffset);
    	}
        else if ( ucPM_TNR[ch_v]==3)
        {//strong
    		dvC341_WriteToBuffer(B18_TMPPXNRCTCH1,  0x03, ucRegOffset);
    		dvC341_WriteToBuffer(B18_TPXNRTH1CH1,	0x31, ucRegOffset);
    		dvC341_WriteToBuffer(B18_TPXNRTH2CH1,	0x75, ucRegOffset);
    		dvC341_WriteToBuffer(B18_TPXNRTH3CH1,	0xb9, ucRegOffset);
    		dvC341_WriteToBuffer(B18_TPXNRTH4CH1,	0xfd, ucRegOffset);
    		dvC341_WriteToBuffer(B18_TPXNRCF1CH1,	0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B18_TPXNRCF2CH1,	0x08, ucRegOffset);
    		dvC341_WriteToBuffer(B18_TPXNRCF3CH1,	0x0c, ucRegOffset);
    		dvC341_WriteToBuffer(B18_TPXNRCF4CH1,	0x10, ucRegOffset);
    		dvC341_WriteToBuffer(B18_TPXNRCF5CH1,	0x14, ucRegOffset);
    		dvC341_WriteToBuffer(B18_TPXNRCF6CH1,	0x18, ucRegOffset);
    		dvC341_WriteToBuffer(B18_TPXNRCF7CH1,	0x1c, ucRegOffset);
    		dvC341_WriteToBuffer(B18_TPXNRCF8CH1,	0x1f, ucRegOffset);
    	}
        else
    	{
    		dvC341_WriteToBuffer(B18_TMPPXNRCTCH1,0x00, ucRegOffset);
    	}
    }

    dvC341_Buffer_Flush();
}

static void dvC341_Set_NR_HVPX(const eC341_CH_V ch_v)
{
	UINT32 ulHPXNR,ulVPXNR;
    if(ch_v >= eC341_CH_VNUMBER)
    {
        return;
    }

    //LOG_MSG(db_ALWAYS, "dvC341_Set_NR_HVPX %d,%d \r\n",ucPM_HNR[ch_v],ucPM_VNR[ch_v]);
    //ucPM_HNR[ch_v] = 0;      //A70LV_Doulas_0145 remove
    //ucPM_VNR[ch_v] = 0;      //A70LV_Doulas_0145 remove

	ulHPXNR=(ucPM_HNR[ch_v]!=0) ? 0x01 : 0;
	ulVPXNR=(ucPM_VNR[ch_v]!=0) ? 0x10 : 0;

	for(UINT8 ch=0; ch<OUTPUT4K_CH_NUM; ch++)
    {
        UINT8 BankOffset = 16;
        UINT8 ucRegOffset = ((ch_v * 2) + ch) * BankOffset;

    	dvC341_WriteToBuffer(B21_HVPXNRCTCH1,(ulVPXNR|ulHPXNR), ucRegOffset);	//Horizontal/vertical NR control

    	if ( ucPM_HNR[ch_v]==1 )
        {           //weak
    		dvC341_WriteToBuffer(B21_HPXNRCF1CH1,0x42, ucRegOffset);	//Horizontal NR coefficient 1
    		dvC341_WriteToBuffer(B21_HPXNRCF2CH1,0x96, ucRegOffset);	//Horizontal NR coefficient 2
    	}
        else 	if ( ucPM_HNR[ch_v]==2 )
        {   //medium
    		dvC341_WriteToBuffer(B21_HPXNRCF1CH1,0x75, ucRegOffset);
    		dvC341_WriteToBuffer(B21_HPXNRCF2CH1,0xc9, ucRegOffset);
    	}
        else 	if ( ucPM_HNR[ch_v]==3 )
    	{	//strong
    		dvC341_WriteToBuffer(B21_HPXNRCF1CH1,0xa8, ucRegOffset);
    		dvC341_WriteToBuffer(B21_HPXNRCF2CH1,0xfc, ucRegOffset);
    	}
        else
    	{
    		dvC341_WriteToBuffer(B21_HPXNRCF1CH1,0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_HPXNRCF2CH1,0x00, ucRegOffset);
    	}

    	if ( ucPM_VNR[ch_v]==1 )
        {           //weak
    		dvC341_WriteToBuffer(B21_VPXNRCF1CH1,0x42, ucRegOffset);	//Vertical NR coefficient 1
    		dvC341_WriteToBuffer(B21_VPXNRCF2CH1,0x96, ucRegOffset);	//Vertical NR coefficient 2
    	}
        else 	if ( ucPM_VNR[ch_v]==2 )
    	{	//medium
    		dvC341_WriteToBuffer(B21_VPXNRCF1CH1,0x75, ucRegOffset);
    		dvC341_WriteToBuffer(B21_VPXNRCF2CH1,0xc9, ucRegOffset);
    	}
        else 	if ( ucPM_VNR[ch_v]==3 )
    	{	//strong
    		dvC341_WriteToBuffer(B21_VPXNRCF1CH1,0xa8, ucRegOffset);
    		dvC341_WriteToBuffer(B21_VPXNRCF2CH1,0xfc, ucRegOffset);
    	}
        else
    	{
    		dvC341_WriteToBuffer(B21_VPXNRCF1CH1,0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_VPXNRCF2CH1,0x00, ucRegOffset);
    	}
    }

    dvC341_Buffer_Flush();
}

static void dvC341_Set_NR_MOSQ(const eC341_CH_V ch_v)
{
    if(ch_v >= eC341_CH_VNUMBER)
    {
        return;
    }

    if(m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.eScanMode == eSCAN_MODE_INTERLACE)  //A70LV_Doulas_0092 modify//A70LV_Doulas_0051  //A70LV_Doulas_0029 modify
        ucPM_MNR[ch_v] = m_sChannelSetting[ch_v].cMPEG_NR;
    else
        ucPM_MNR[ch_v] = eMPEG_NR_OFF;

	for(UINT8 ch=0; ch<OUTPUT4K_CH_NUM; ch++)
    {
        UINT8 BankOffset = 16;
        UINT8 ucRegOffset = ((ch_v * 2) + ch) * BankOffset;

    	if ( ucPM_MNR[ch_v]==1 )
        {//weak
    		dvC341_WriteToBuffer(B21_MSQENCH1     ,0x01, ucRegOffset);		//Mosquito NR control
    		dvC341_WriteToBuffer(B21_MSQLPF5H0CH1 ,0x80, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQLPF5H1CH1 ,0x38, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQLPF5H2CH1 ,0x08, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQLPF5V0CH1 ,0x80, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQLPF5V1CH1 ,0x38, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQLPF5V2CH1 ,0x08, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQVEGLPF0CH1,0x3f, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQVEGLPF1CH1,0x3f, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQVEGLPF2CH1,0x7f, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQVEGLPF3CH1,0x7f, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQVEGLPF4CH1,0x7f, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQVEGLPF5CH1,0x7f, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQHEGLPF0CH1,0x3f, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQHEGLPF1CH1,0x3f, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQHEGLPF2CH1,0x7f, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQHEGLPF3CH1,0x7f, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQHEGLPF4CH1,0x7f, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQHEGLPF5CH1,0x7f, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BLENDBIASCH1 ,0xfc, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BLENDGAINCH1 ,0x20, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQMAXBIASCH1,0xd5, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQSUBGAINCH1,0x7f, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQYTH0CH1   ,0x18, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQYTH1CH1   ,0x24, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQYTH2CH1   ,0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQYTH3CH1   ,0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQYTH4CH1   ,0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQYTH5CH1   ,0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQYTH6CH1   ,0x00, ucRegOffset);
    		//dvC341_WriteToBuffer(B21_MSQYTH7CH1   ,0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQYSUB0CH1  ,0x40, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQYSUB1CH1  ,0x51, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQYSUB2CH1  ,0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQYSUB3CH1  ,0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQYSUB4CH1  ,0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQYSUB5CH1  ,0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQYSUB6CH1  ,0x00, ucRegOffset);
    		//dvC341_WriteToBuffer(B21_MSQYSUB7CH1  ,0x00, ucRegOffset);
    	}
        else if ( ucPM_MNR[ch_v]==2 )
    	{//middle
    		dvC341_WriteToBuffer(B21_MSQENCH1     ,0x01, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQLPF5H0CH1 ,0x80, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQLPF5H1CH1 ,0x38, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQLPF5H2CH1 ,0x08, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQLPF5V0CH1 ,0x80, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQLPF5V1CH1 ,0x38, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQLPF5V2CH1 ,0x08, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQVEGLPF0CH1,0x7f, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQVEGLPF1CH1,0x7f, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQVEGLPF2CH1,0x7f, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQVEGLPF3CH1,0x7f, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQVEGLPF4CH1,0x7f, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQVEGLPF5CH1,0x7f, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQHEGLPF0CH1,0x7f, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQHEGLPF1CH1,0x7f, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQHEGLPF2CH1,0x7f, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQHEGLPF3CH1,0x7f, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQHEGLPF4CH1,0x7f, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQHEGLPF5CH1,0x7f, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BLENDBIASCH1 ,0xfc, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BLENDGAINCH1 ,0x3f, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQMAXBIASCH1,0xd9, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQSUBGAINCH1,0xff, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQYTH0CH1   ,0x18, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQYTH1CH1   ,0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQYTH2CH1   ,0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQYTH3CH1   ,0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQYTH4CH1   ,0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQYTH5CH1   ,0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQYTH6CH1   ,0x00, ucRegOffset);
    		//dvC341_WriteToBuffer(B21_MSQYTH7CH1   ,0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQYSUB0CH1  ,0x40, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQYSUB1CH1  ,0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQYSUB2CH1  ,0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQYSUB3CH1  ,0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQYSUB4CH1  ,0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQYSUB5CH1  ,0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQYSUB6CH1  ,0x00, ucRegOffset);
    		//dvC341_WriteToBuffer(B21_MSQYSUB7CH1  ,0x00, ucRegOffset);
    	}
        else if ( ucPM_MNR[ch_v]==3 )
    	{//strong
    		dvC341_WriteToBuffer(B21_MSQENCH1     ,0x01, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQLPF5H0CH1 ,0x80, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQLPF5H1CH1 ,0x38, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQLPF5H2CH1 ,0x08, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQLPF5V0CH1 ,0x80, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQLPF5V1CH1 ,0x38, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQLPF5V2CH1 ,0x08, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQVEGLPF0CH1,0x7f, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQVEGLPF1CH1,0x7f, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQVEGLPF2CH1,0x7f, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQVEGLPF3CH1,0x7f, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQVEGLPF4CH1,0x7f, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQVEGLPF5CH1,0x7f, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQHEGLPF0CH1,0x7f, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQHEGLPF1CH1,0x7f, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQHEGLPF2CH1,0x7f, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQHEGLPF3CH1,0x7f, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQHEGLPF4CH1,0x7f, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQHEGLPF5CH1,0x7f, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BLENDBIASCH1 ,0xfc, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BLENDGAINCH1 ,0xff, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQMAXBIASCH1,0xd9, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQSUBGAINCH1,0x6f, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQYTH0CH1   ,0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQYTH1CH1   ,0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQYTH2CH1   ,0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQYTH3CH1   ,0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQYTH4CH1   ,0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQYTH5CH1   ,0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQYTH6CH1   ,0x00, ucRegOffset);
    		//dvC341_WriteToBuffer(B21_MSQYTH7CH1   ,0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQYSUB0CH1  ,0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQYSUB1CH1  ,0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQYSUB2CH1  ,0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQYSUB3CH1  ,0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQYSUB4CH1  ,0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQYSUB5CH1  ,0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_MSQYSUB6CH1  ,0x00, ucRegOffset);
    		//dvC341_WriteToBuffer(B21_MSQYSUB7CH1  ,0x00, ucRegOffset);
    	}
        else
        {
    		dvC341_WriteToBuffer(B21_MSQENCH1     ,0x00, ucRegOffset);
    	}
    }

    dvC341_Buffer_Flush();
}

static void dvC341_Set_BNR_480i(const eC341_CH_V ch_v)
{
    if(ch_v >= eC341_CH_VNUMBER)
    {
        return;
    }

    for(UINT8 ch=0; ch<OUTPUT4K_CH_NUM; ch++)
    {
        UINT8 ucRegOffset = ((ch_v * 2) + ch) * CH_BANK_OFFSET;

    	if ( ucPM_BNR[ch_v]==1 )
        {
    		dvC341_WriteToBuffer(B21_BNRCTCH1,		    0x2297, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNTESTCH1,		    0x90, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNEGTH0CH1,		0x40, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNEGTH1CH1,		0x50, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNEGTH2CH1,		0x60, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNEGTH3CH1,		0x80, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNRABLCF0CH1,	    0x20, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNRABLCF1CH1,	    0x84, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNLPCF0CH1,		0x41, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNLPCF1CH1,		0x06, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNAMVTHCH1,		0x10, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNAEGTHCH1,		0x40, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHPOSCH1,		    0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVPOSCH1,		    0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNDETCTCH1,		0x06, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNLFCYCLCH1,		0x04, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHMTHHCCH1,		0x40, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHMTHLCCH1,		0x12, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHMTHSCH1,		0x10, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHMCNTTHCH1,	    0x0040, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHMCNTOFFCH1,	    0x0010, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVMTHHCCH1,		0x40, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVMTHLCCH1,		0x12, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVMTHSCH1,		0x10, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVMCNTTHCH1,	    0x0040, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVMCNTOFFCH1,	    0x0010, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNPJCTCH1,		    0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNPJCFCH1,		    0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNPJRADCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNPJRDTCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHINIPOSCH1,	    0x0000, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHSCLCH1,		    0x0000, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVINIPOSCH1,	    0x0000, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVSCLCH1,		    0x0000, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNRCT2CH1,		    0x12, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNTEST2CH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHPJTHCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVPJTHCH1,		0x0000, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNMVHGVTHCH1,	    0x08, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNMVHGCTHCH1,	    0x08, ucRegOffset);
    	}
        else if ( ucPM_BNR[ch_v]==2 )
    	{
    		dvC341_WriteToBuffer(B21_BNRCTCH1,		    0x2297, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNTESTCH1,		    0xa0, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNEGTH0CH1,		0x40, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNEGTH1CH1,		0x50, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNEGTH2CH1,		0x60, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNEGTH3CH1,		0x80, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNRABLCF0CH1,	    0x10, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNRABLCF1CH1,	    0x42, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNLPCF0CH1,		0x33, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNLPCF1CH1,		0x04, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNAMVTHCH1,		0x10, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNAEGTHCH1,		0x40, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHPOSCH1,		    0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVPOSCH1,		    0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNDETCTCH1,		0x06, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNLFCYCLCH1,		0x04, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHMTHHCCH1,		0x40, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHMTHLCCH1,		0x12, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHMTHSCH1,		0x10, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHMCNTTHCH1,	    0x0040, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHMCNTOFFCH1,	    0x0010, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVMTHHCCH1,		0x40, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVMTHLCCH1,		0x12, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVMTHSCH1,		0x10, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVMCNTTHCH1,	    0x0040, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVMCNTOFFCH1,	    0x0010, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNPJCTCH1,		    0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNPJCFCH1,		    0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNPJRADCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNPJRDTCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHINIPOSCH1,	    0x0000, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHSCLCH1,		    0x0000, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVINIPOSCH1,	    0x0000, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVSCLCH1,		    0x0000, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNRCT2CH1,		    0x12, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNTEST2CH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHPJTHCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVPJTHCH1,		0x0000, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNMVHGVTHCH1,	    0x08, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNMVHGCTHCH1,	    0x08, ucRegOffset);
    	}
        else if ( ucPM_BNR[ch_v]==3 )
    	{
    		dvC341_WriteToBuffer(B21_BNRCTCH1,		    0x0c97, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNTESTCH1,		    0xb0, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNEGTH0CH1,		0x40, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNEGTH1CH1,		0x50, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNEGTH2CH1,		0x60, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNEGTH3CH1,		0x80, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNRABLCF0CH1,	    0x10, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNRABLCF1CH1,	    0x42, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNLPCF0CH1,		0x33, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNLPCF1CH1,		0x04, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNAMVTHCH1,		0x06, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNAEGTHCH1,		0x40, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHPOSCH1,		    0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVPOSCH1,		    0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNDETCTCH1,		0x06, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNLFCYCLCH1,		0x04, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHMTHHCCH1,		0x40, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHMTHLCCH1,		0x12, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHMTHSCH1,		0x10, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHMCNTTHCH1,	    0x0040, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHMCNTOFFCH1,	    0x0010, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVMTHHCCH1,		0x40, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVMTHLCCH1,		0x12, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVMTHSCH1,		0x10, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVMCNTTHCH1,	    0x0040, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVMCNTOFFCH1,	    0x0010, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNPJCTCH1,		    0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNPJCFCH1,		    0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNPJRADCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNPJRDTCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHINIPOSCH1,	    0x0000, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHSCLCH1,		    0x0000, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVINIPOSCH1,	    0x0000, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVSCLCH1,		    0x0000, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNRCT2CH1,		    0x12, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNTEST2CH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHPJTHCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVPJTHCH1,		0x0000, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNMVHGVTHCH1,	    0x08, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNMVHGCTHCH1,	    0x08, ucRegOffset);
    	}
    }

    dvC341_Buffer_Flush();
}


static void dvC341_Set_BNR_480P(const eC341_CH_V ch_v)
{
    if(ch_v >= eC341_CH_VNUMBER)
    {
        return;
    }

    for(UINT8 ch=0; ch<OUTPUT4K_CH_NUM; ch++)
    {
        UINT8 ucRegOffset = ((ch_v * 2) + ch) * CH_BANK_OFFSET;

    	if ( ucPM_BNR[ch_v]==1 )
        {
    		dvC341_WriteToBuffer(B21_BNRCTCH1,		0xa297, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNTESTCH1,		0x90, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNEGTH0CH1,		0x40, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNEGTH1CH1,		0x50, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNEGTH2CH1,		0x60, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNEGTH3CH1,		0x80, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNRABLCF0CH1,	0x20, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNRABLCF1CH1,	0x84, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNLPCF0CH1,		0x41, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNLPCF1CH1,		0x06, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNAMVTHCH1,		0x10, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNAEGTHCH1,		0x40, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHPOSCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVPOSCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNDETCTCH1,		0x06, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNLFCYCLCH1,		0x04, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHMTHHCCH1,		0x40, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHMTHLCCH1,		0x12, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHMTHSCH1,		0x10, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHMCNTTHCH1,	0x0040, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHMCNTOFFCH1,	0x0010, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVMTHHCCH1,		0x40, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVMTHLCCH1,		0x12, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVMTHSCH1,		0x10, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVMCNTTHCH1,	0x0080, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVMCNTOFFCH1,	0x0020, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNPJCTCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNPJCFCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNPJRADCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNPJRDTCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHINIPOSCH1,	0x0000, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHSCLCH1,		0x0000, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVINIPOSCH1,	0x0000, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVSCLCH1,		0x0000, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNRCT2CH1,		0x12, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNTEST2CH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHPJTHCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVPJTHCH1,		0x0000, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNMVHGVTHCH1,	0x08, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNMVHGCTHCH1,	0x06, ucRegOffset);
    	}
        else if ( ucPM_BNR[ch_v]==2 )
    	{
    		dvC341_WriteToBuffer(B21_BNRCTCH1,		0xa297, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNTESTCH1,		0xa0, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNEGTH0CH1,		0x40, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNEGTH1CH1,		0x50, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNEGTH2CH1,		0x60, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNEGTH3CH1,		0x80, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNRABLCF0CH1,	0x10, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNRABLCF1CH1,	0x42, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNLPCF0CH1,		0x33, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNLPCF1CH1,		0x04, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNAMVTHCH1,		0x10, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNAEGTHCH1,		0x40, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHPOSCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVPOSCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNDETCTCH1,		0x06, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNLFCYCLCH1,		0x04, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHMTHHCCH1,		0x40, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHMTHLCCH1,		0x12, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHMTHSCH1,		0x10, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHMCNTTHCH1,	0x0040, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHMCNTOFFCH1,	0x0010, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVMTHHCCH1,		0x40, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVMTHLCCH1,		0x12, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVMTHSCH1,		0x10, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVMCNTTHCH1,	0x0080, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVMCNTOFFCH1,	0x0020, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNPJCTCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNPJCFCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNPJRADCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNPJRDTCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHINIPOSCH1,	0x0000, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHSCLCH1,		0x0000, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVINIPOSCH1,	0x0000, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVSCLCH1,		0x0000, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNRCT2CH1,		0x12, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNTEST2CH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHPJTHCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVPJTHCH1,		0x0000, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNMVHGVTHCH1,	0x08, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNMVHGCTHCH1,	0x06, ucRegOffset);
    	}
        else if ( ucPM_BNR[ch_v]==3 )
    	{
    		dvC341_WriteToBuffer(B21_BNRCTCH1,		0x8c97, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNTESTCH1,		0xb0, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNEGTH0CH1,		0x40, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNEGTH1CH1,		0x50, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNEGTH2CH1,		0x60, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNEGTH3CH1,		0x80, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNRABLCF0CH1,	0x10, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNRABLCF1CH1,	0x42, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNLPCF0CH1,		0x33, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNLPCF1CH1,		0x04, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNAMVTHCH1,		0x06, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNAEGTHCH1,		0x40, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHPOSCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVPOSCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNDETCTCH1,		0x06, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNLFCYCLCH1,		0x04, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHMTHHCCH1,		0x40, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHMTHLCCH1,		0x12, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHMTHSCH1,		0x10, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHMCNTTHCH1,	0x0040, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHMCNTOFFCH1,	0x0010, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVMTHHCCH1,		0x40, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVMTHLCCH1,		0x12, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVMTHSCH1,		0x10, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVMCNTTHCH1,	0x0080, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVMCNTOFFCH1,	0x0020, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNPJCTCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNPJCFCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNPJRADCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNPJRDTCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHINIPOSCH1,	0x0000, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHSCLCH1,		0x0000, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVINIPOSCH1,	0x0000, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVSCLCH1,		0x0000, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNRCT2CH1,		0x12, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNTEST2CH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHPJTHCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVPJTHCH1,		0x0000, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNMVHGVTHCH1,	0x08, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNMVHGCTHCH1,	0x06, ucRegOffset);
    	}
    }

    dvC341_Buffer_Flush();
}

static void dvC341_Set_BNR_1080i(const eC341_CH_V ch_v)
{
    if(ch_v >= eC341_CH_VNUMBER)
    {
        return;
    }

	for(UINT8 ch=0; ch<OUTPUT4K_CH_NUM; ch++)
    {
        UINT8 BankOffset = 16;
        UINT8 ucRegOffset = ((ch_v * 2) + ch) * BankOffset;

    	if ( ucPM_BNR[ch_v]==1 )
        {
    		dvC341_WriteToBuffer(B21_BNRCTCH1,		0x2297, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNTESTCH1,		0x90, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNEGTH0CH1,		0x40, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNEGTH1CH1,		0x50, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNEGTH2CH1,		0x60, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNEGTH3CH1,		0x80, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNRABLCF0CH1,	0x20, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNRABLCF1CH1,	0x84, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNLPCF0CH1,		0x41, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNLPCF1CH1,		0x06, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNAMVTHCH1,		0x10, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNAEGTHCH1,		0x40, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHPOSCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVPOSCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNDETCTCH1,		0x06, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNLFCYCLCH1,		0x04, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHMTHHCCH1,		0x40, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHMTHLCCH1,		0x12, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHMTHSCH1,		0x10, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHMCNTTHCH1,	0x0180, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHMCNTOFFCH1,	0x0060, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVMTHHCCH1,		0x40, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVMTHLCCH1,		0x12, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVMTHSCH1,		0x10, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVMCNTTHCH1,	0x0180, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVMCNTOFFCH1,	0x0060, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNPJCTCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNPJCFCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNPJRADCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNPJRDTCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHINIPOSCH1,	0x0000, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHSCLCH1,		0x0000, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVINIPOSCH1,	0x0000, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVSCLCH1,		0x0000, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNRCT2CH1,		0x12, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNTEST2CH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHPJTHCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVPJTHCH1,		0x0000, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNMVHGVTHCH1,	0x08, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNMVHGCTHCH1,	0x30, ucRegOffset);
    	}
        else if ( ucPM_BNR[ch_v]==2 )
    	{
    		dvC341_WriteToBuffer(B21_BNRCTCH1,		0x2297, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNTESTCH1,		0xa0, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNEGTH0CH1,		0x40, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNEGTH1CH1,		0x50, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNEGTH2CH1,		0x60, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNEGTH3CH1,		0x80, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNRABLCF0CH1,	0x10, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNRABLCF1CH1,	0x42, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNLPCF0CH1,		0x33, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNLPCF1CH1,		0x04, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNAMVTHCH1,		0x10, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNAEGTHCH1,		0x40, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHPOSCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVPOSCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNDETCTCH1,		0x06, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNLFCYCLCH1,		0x04, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHMTHHCCH1,		0x40, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHMTHLCCH1,		0x12, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHMTHSCH1,		0x10, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHMCNTTHCH1,	0x0180, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHMCNTOFFCH1,	0x0060, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVMTHHCCH1,		0x40, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVMTHLCCH1,		0x12, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVMTHSCH1,		0x10, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVMCNTTHCH1,	0x0300, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVMCNTOFFCH1,	0x00c0, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNPJCTCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNPJCFCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNPJRADCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNPJRDTCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHINIPOSCH1,	0x0000, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHSCLCH1,		0x0000, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVINIPOSCH1,	0x0000, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVSCLCH1,		0x0000, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNRCT2CH1,		0x12, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNTEST2CH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHPJTHCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVPJTHCH1,		0x0000, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNMVHGVTHCH1,	0x08, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNMVHGCTHCH1,	0x24, ucRegOffset);
    	}
        else if ( ucPM_BNR[ch_v]==3 )
    	{
    		dvC341_WriteToBuffer(B21_BNRCTCH1,		0x0c97, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNTESTCH1,		0xb0, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNEGTH0CH1,		0x40, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNEGTH1CH1,		0x50, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNEGTH2CH1,		0x60, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNEGTH3CH1,		0x80, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNRABLCF0CH1,	0x10, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNRABLCF1CH1,	0x42, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNLPCF0CH1,		0x33, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNLPCF1CH1,		0x04, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNAMVTHCH1,		0x06, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNAEGTHCH1,		0x40, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHPOSCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVPOSCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNDETCTCH1,		0x06, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNLFCYCLCH1,		0x04, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHMTHHCCH1,		0x40, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHMTHLCCH1,		0x12, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHMTHSCH1,		0x10, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHMCNTTHCH1,	0x0180, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHMCNTOFFCH1,	0x0060, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVMTHHCCH1,		0x40, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVMTHLCCH1,		0x12, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVMTHSCH1,		0x10, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVMCNTTHCH1,	0x0300, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVMCNTOFFCH1,	0x00c0, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNPJCTCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNPJCFCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNPJRADCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNPJRDTCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHINIPOSCH1,	0x0000, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHSCLCH1,		0x0000, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVINIPOSCH1,	0x0000, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVSCLCH1,		0x0000, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNRCT2CH1,		0x12, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNTEST2CH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHPJTHCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVPJTHCH1,		0x0000, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNMVHGVTHCH1,	0x08, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNMVHGCTHCH1,	0x24, ucRegOffset);
    	}
    }

    dvC341_Buffer_Flush();
}

static void dvC341_Set_BNR_720P(const eC341_CH_V ch_v)
{
    if(ch_v >= eC341_CH_VNUMBER)
    {
        return;
    }

	for(UINT8 ch=0; ch<OUTPUT4K_CH_NUM; ch++)
    {
        UINT8 BankOffset = 16;
        UINT8 ucRegOffset = ((ch_v * 2) + ch) * BankOffset;

    	if ( ucPM_BNR[ch_v]==1 )
        {
    		dvC341_WriteToBuffer(B21_BNRCTCH1,		0xa297, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNTESTCH1,		0x90, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNEGTH0CH1,		0x40, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNEGTH1CH1,		0x50, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNEGTH2CH1,		0x60, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNEGTH3CH1,		0x80, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNRABLCF0CH1,	0x20, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNRABLCF1CH1,	0x84, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNLPCF0CH1,		0x41, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNLPCF1CH1,		0x06, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNAMVTHCH1,		0x10, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNAEGTHCH1,		0x40, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHPOSCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVPOSCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNDETCTCH1,		0x06, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNLFCYCLCH1,		0x04, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHMTHHCCH1,		0x40, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHMTHLCCH1,		0x12, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHMTHSCH1,		0x10, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHMCNTTHCH1,	0x00ab, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHMCNTOFFCH1,	0x002b, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVMTHHCCH1,		0x40, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVMTHLCCH1,		0x12, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVMTHSCH1,		0x10, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVMCNTTHCH1,	0x0156, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVMCNTOFFCH1,	0x0056, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNPJCTCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNPJCFCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNPJRADCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNPJRDTCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHINIPOSCH1,	0x0000, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHSCLCH1,		0x0000, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVINIPOSCH1,	0x0000, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVSCLCH1,		0x0000, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNRCT2CH1,		0x12, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNTEST2CH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHPJTHCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVPJTHCH1,		0x0000, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNMVHGVTHCH1,	0x08, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNMVHGCTHCH1,	0x10, ucRegOffset);
    	}
        else if ( ucPM_BNR[ch_v]==2 )
    	{
    		dvC341_WriteToBuffer(B21_BNRCTCH1,		0xa297, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNTESTCH1,		0xa0, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNEGTH0CH1,		0x40, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNEGTH1CH1,		0x50, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNEGTH2CH1,		0x60, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNEGTH3CH1,		0x80, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNRABLCF0CH1,	0x10, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNRABLCF1CH1,	0x42, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNLPCF0CH1,		0x33, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNLPCF1CH1,		0x04, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNAMVTHCH1,		0x10, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNAEGTHCH1,		0x40, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHPOSCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVPOSCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNDETCTCH1,		0x06, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNLFCYCLCH1,		0x04, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHMTHHCCH1,		0x40, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHMTHLCCH1,		0x12, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHMTHSCH1,		0x10, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHMCNTTHCH1,	0x00ab, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHMCNTOFFCH1,	0x002b, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVMTHHCCH1,		0x40, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVMTHLCCH1,		0x12, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVMTHSCH1,		0x10, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVMCNTTHCH1,	0x0156, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVMCNTOFFCH1,	0x0056, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNPJCTCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNPJCFCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNPJRADCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNPJRDTCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHINIPOSCH1,	0x0000, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHSCLCH1,		0x0000, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVINIPOSCH1,	0x0000, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVSCLCH1,		0x0000, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNRCT2CH1,		0x12, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNTEST2CH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHPJTHCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVPJTHCH1,		0x0000, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNMVHGVTHCH1,	0x08, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNMVHGCTHCH1,	0x10, ucRegOffset);
    	}
        else if ( ucPM_BNR[ch_v]==3 )
    	{
    		dvC341_WriteToBuffer(B21_BNRCTCH1,		0x8c97, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNTESTCH1,		0xb0, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNEGTH0CH1,		0x40, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNEGTH1CH1,		0x50, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNEGTH2CH1,		0x60, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNEGTH3CH1,		0x80, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNRABLCF0CH1,	0x10, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNRABLCF1CH1,	0x42, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNLPCF0CH1,		0x33, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNLPCF1CH1,		0x04, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNAMVTHCH1,		0x06, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNAEGTHCH1,		0x40, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHPOSCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVPOSCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNDETCTCH1,		0x06, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNLFCYCLCH1,		0x04, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHMTHHCCH1,		0x40, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHMTHLCCH1,		0x12, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHMTHSCH1,		0x10, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHMCNTTHCH1,	0x00ab, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHMCNTOFFCH1,	0x002b, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVMTHHCCH1,		0x40, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVMTHLCCH1,		0x12, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVMTHSCH1,		0x10, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVMCNTTHCH1,	0x0156, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVMCNTOFFCH1,	0x0056, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNPJCTCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNPJCFCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNPJRADCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNPJRDTCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHINIPOSCH1,	0x0000, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHSCLCH1,		0x0000, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVINIPOSCH1,	0x0000, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVSCLCH1,		0x0000, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNRCT2CH1,		0x12, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNTEST2CH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHPJTHCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVPJTHCH1,		0x0000, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNMVHGVTHCH1,	0x08, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNMVHGCTHCH1,	0x10, ucRegOffset);
    	}
    }

    dvC341_Buffer_Flush();
}

static void dvC341_Set_BNR_1080P(const eC341_CH_V ch_v)
{
    if(ch_v >= eC341_CH_VNUMBER)
    {
        return;
    }

	for(UINT8 ch=0; ch<OUTPUT4K_CH_NUM; ch++)
    {
        UINT8 BankOffset = 16;
        UINT8 ucRegOffset = ((ch_v * 2) + ch) * BankOffset;

    	if ( ucPM_BNR[ch_v]==1 )
        {
    		dvC341_WriteToBuffer(B21_BNRCTCH1,		0xa297, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNTESTCH1,		0x90, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNEGTH0CH1,		0x40, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNEGTH1CH1,		0x50, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNEGTH2CH1,		0x60, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNEGTH3CH1,		0x80, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNRABLCF0CH1,	0x20, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNRABLCF1CH1,	0x84, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNLPCF0CH1,		0x41, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNLPCF1CH1,		0x06, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNAMVTHCH1,		0x10, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNAEGTHCH1,		0x40, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHPOSCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVPOSCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNDETCTCH1,		0x06, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNLFCYCLCH1,		0x04, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHMTHHCCH1,		0x40, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHMTHLCCH1,		0x12, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHMTHSCH1,		0x10, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHMCNTTHCH1,	0x0180, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHMCNTOFFCH1,	0x0060, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVMTHHCCH1,		0x40, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVMTHLCCH1,		0x12, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVMTHSCH1,		0x10, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVMCNTTHCH1,	0x0300, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVMCNTOFFCH1,	0x00c0, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNPJCTCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNPJCFCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNPJRADCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNPJRDTCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHINIPOSCH1,	0x0000, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHSCLCH1,		0x0000, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVINIPOSCH1,	0x0000, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVSCLCH1,		0x0000, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNRCT2CH1,		0x12, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNTEST2CH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHPJTHCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVPJTHCH1,		0x0000, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNMVHGVTHCH1,	0x08, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNMVHGCTHCH1,	0x24, ucRegOffset);
    	}
        else if ( ucPM_BNR[ch_v]==2 )
    	{
    		dvC341_WriteToBuffer(B21_BNRCTCH1,		0xa297, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNTESTCH1,		0xa0, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNEGTH0CH1,		0x40, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNEGTH1CH1,		0x50, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNEGTH2CH1,		0x60, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNEGTH3CH1,		0x80, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNRABLCF0CH1,	0x10, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNRABLCF1CH1,	0x42, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNLPCF0CH1,		0x33, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNLPCF1CH1,		0x04, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNAMVTHCH1,		0x10, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNAEGTHCH1,		0x40, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHPOSCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVPOSCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNDETCTCH1,		0x06, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNLFCYCLCH1,		0x04, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHMTHHCCH1,		0x40, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHMTHLCCH1,		0x12, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHMTHSCH1,		0x10, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHMCNTTHCH1,	0x0180, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHMCNTOFFCH1,	0x0060, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVMTHHCCH1,		0x40, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVMTHLCCH1,		0x12, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVMTHSCH1,		0x10, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVMCNTTHCH1,	0x0300, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVMCNTOFFCH1,	0x00c0, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNPJCTCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNPJCFCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNPJRADCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNPJRDTCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHINIPOSCH1,	0x0000, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHSCLCH1,		0x0000, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVINIPOSCH1,	0x0000, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVSCLCH1,		0x0000, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNRCT2CH1,		0x12, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNTEST2CH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHPJTHCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVPJTHCH1,		0x0000, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNMVHGVTHCH1,	0x08, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNMVHGCTHCH1,	0x24, ucRegOffset);
    	}
        else if ( ucPM_BNR[ch_v]==3 )
    	{
    		dvC341_WriteToBuffer(B21_BNRCTCH1,		0x8c97, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNTESTCH1,		0xb0, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNEGTH0CH1,		0x40, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNEGTH1CH1,		0x50, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNEGTH2CH1,		0x60, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNEGTH3CH1,		0x80, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNRABLCF0CH1,	0x10, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNRABLCF1CH1,	0x42, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNLPCF0CH1,		0x33, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNLPCF1CH1,		0x04, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNAMVTHCH1,		0x06, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNAEGTHCH1,		0x40, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHPOSCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVPOSCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNDETCTCH1,		0x06, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNLFCYCLCH1,		0x04, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHMTHHCCH1,		0x40, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHMTHLCCH1,		0x12, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHMTHSCH1,		0x10, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHMCNTTHCH1,	0x0180, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHMCNTOFFCH1,	0x0060, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVMTHHCCH1,		0x40, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVMTHLCCH1,		0x12, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVMTHSCH1,		0x10, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVMCNTTHCH1,	0x0300, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVMCNTOFFCH1,	0x00c0, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNPJCTCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNPJCFCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNPJRADCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNPJRDTCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHINIPOSCH1,	0x0000, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHSCLCH1,		0x0000, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVINIPOSCH1,	0x0000, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVSCLCH1,		0x0000, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNRCT2CH1,		0x12, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNTEST2CH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNHPJTHCH1,		0x00, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNVPJTHCH1,		0x0000, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNMVHGVTHCH1,	0x08, ucRegOffset);
    		dvC341_WriteToBuffer(B21_BNMVHGCTHCH1,	0x24, ucRegOffset);
    	}
    }

    dvC341_Buffer_Flush();
}

static void dvC341_Set_NR_Block(const eC341_CH_V ch_v)
{
    if(ch_v >= eC341_CH_VNUMBER)
    {
        return;
    }

    if(m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.eScanMode == eSCAN_MODE_INTERLACE)    //A70LV_Doulas_0092 modify//A70LV_Doulas_0051 //A70LV_Doulas_0029 modify
        ucPM_BNR[ch_v] = m_sChannelSetting[ch_v].cMPEG_NR;
    else
        ucPM_BNR[ch_v] = eMPEG_NR_OFF;

	if ( ucPM_BNR[ch_v]!= eMPEG_NR_OFF ) //A70LV_Doulas_0029
    {
		if ( m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive==240 ) {
			dvC341_Set_BNR_480i(ch_v);
		}
		else if ( m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive== 288 ) {   //A70LV_Doulas_0145 Add 576i Add
			dvC341_Set_BNR_480i(ch_v);
		}
        else if ( m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive==480 )
		{
			dvC341_Set_BNR_480P(ch_v);
		}
        else if ( m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive==540 )
		{
			dvC341_Set_BNR_1080i(ch_v);
		}
        else if ( m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive==720 )
		{
			dvC341_Set_BNR_720P(ch_v);
		}
        else if ( m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive==1080 )
		{
			dvC341_Set_BNR_1080P(ch_v);
		}
		else
        {                                   // Dont'match
			if(m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.eScanMode == eSCAN_MODE_INTERLACE)      // Interlace Input
            {
				dvC341_Write_ch_v(ch_v, B21_BNRCTCH1,0x0000, 16);	// Disable Setting(Inter.)
			}
			else
            {
				dvC341_Write_ch_v(ch_v, B21_BNRCTCH1,0x0000, 16);	// Disable Setting(Prog.)
			}
		}
	}
	else
    {
		if(m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.eScanMode == eSCAN_MODE_INTERLACE)    // Interlace Input
        {
			dvC341_Write_ch_v(ch_v, B21_BNRCTCH1,0x0000, 16);	// Disable Setting(Inter.)
		}
		else
        {
			dvC341_Write_ch_v(ch_v, B21_BNRCTCH1,0x0000, 16);	// Disable Setting(Prog.)
		}
	}
}


static void dvC341_Set_NR(const eC341_CH_V ch_v)
{
	dvC341_Set_NR_Block(ch_v);
	dvC341_Set_NT_TPX(ch_v);
	dvC341_Set_NR_HVPX(ch_v);
	dvC341_Set_NR_MOSQ(ch_v);

	return;
}

void dvC341_Set_Output_Fill_Color(BOOL bEnable,UINT8 ucValue)       //A70LV_Doulas_0035 modify
{
    UINT32 ulColor;

    switch(ucValue)
    {
        case eFILL_COLOR_RED:
            ulColor = 0x0000FF;
            break;

        case eFILL_COLOR_GREEN:
            ulColor = 0x00FF00;
            break;

        case eFILL_COLOR_BLUE:
            ulColor = 0xFF0000;
            break;

        case eFILL_COLOR_YELLOW:
            ulColor = 0x00FFFF;
            break;

        case eFILL_COLOR_WHITE:
            ulColor = 0xFFFFFF;
            break;

        case eFILL_COLOR_CYAN:
            ulColor = 0xFFFF00;
            break;

        case eFILL_COLOR_MAGENTA:
            ulColor = 0xFF00FF;
            break;

        case eFILL_COLOR_BLACK:
        default:
            ulColor = 0x000000;
            break;

    }

    if(bEnable)
    {
        dvC341_Write_ch_v(eC341_CH_V0, B8_PGCTRLCH1,0, CH_BANK_OFFSET); //disable test pattern

        dvC341_Write_ch_v(eC341_CH_V0, B8_OFILLCH1,ulColor, CH_BANK_OFFSET);
        dvC341_Write_ch_v(eC341_CH_V0, B8_OBKGD0CH1,ulColor, CH_BANK_OFFSET);
        dvC341_Write_ch_v(eC341_CH_V0, B8_OBKGD1CH1,ulColor, CH_BANK_OFFSET);
        dvC341_Write_ch_v(eC341_CH_V0, B8_OBKGD2CH1,ulColor, CH_BANK_OFFSET);

        dvC341_Write_ch_v(eC341_CH_V0, B8_OACTHSTCH1,m_sChannelInfo[0].sOutputTimingInfo.uiHStart, CH_BANK_OFFSET);
        dvC341_Write_ch_v(eC341_CH_V0, B8_OACTHWCH1,m_sChannelInfo[0].sOutputTimingInfo.uiHActive, CH_BANK_OFFSET);
        dvC341_Write_ch_v(eC341_CH_V0, B8_OACTVSTCH1,m_sChannelInfo[0].sOutputTimingInfo.uiVStart-5, CH_BANK_OFFSET);
        dvC341_Write_ch_v(eC341_CH_V0, B8_OACTVWCH1,m_sChannelInfo[0].sOutputTimingInfo.uiVActive+1, CH_BANK_OFFSET);

        dvC341_Write_ch_v(eC341_CH_V0, B10_LSCCTCH1,	0x00, CH_BANK_OFFSET);
        dvC341_Write_ch_v(eC341_CH_V0, B144_WPBLKCTCH1,	0x00, CH_WPBANK_OFFSET);   //disable wap
        m_sChannelInfo[0].ucOIMGCT |= 0x01;
    }
    else
    {
        dvC341_SetInputPort_2K(eC341_CH_V0);
        #ifdef C341_WARPING_ENABLE      //A70LV_Doulas_0100 M0dify
        dvC341_Write_ch_v(eC341_CH_V0, B144_WPBLKCTCH1, ucWarpInit, CH_WPBANK_OFFSET);  //A70LV_Doulas_0079
        #endif

        dvC341_Write_ch_v(eC341_CH_V0, B8_OFILLCH1, ulColor, CH_BANK_OFFSET);
        dvC341_Write_ch_v(eC341_CH_V0, B8_OBKGD0CH1,ulColor, CH_BANK_OFFSET);
        dvC341_Write_ch_v(eC341_CH_V0, B8_OBKGD1CH1,ulColor, CH_BANK_OFFSET);
        dvC341_Write_ch_v(eC341_CH_V0, B8_OBKGD2CH1,ulColor, CH_BANK_OFFSET);
        m_sChannelInfo[0].ucOIMGCT &= 0xFE;
    }

    if(dvC341_Read(B8_OIMGCTCH1, (eC341_CH_V0 * 2) * CH_BANK_OFFSET) != m_sChannelInfo[eC341_CH_V0].ucOIMGCT)
        dvC341_Write_ch_v(eC341_CH_V0, B8_OIMGCTCH1,m_sChannelInfo[eC341_CH_V0].ucOIMGCT, CH_BANK_OFFSET);

}

void dvC341_Set_Output_Test_Pattern(BOOL bEnable,UINT32 cTpye)      //A70LV_Doulas_0035 modify
{
    UINT32 data=0;

    if(bEnable)
    {
        dvC341_Set_Output_Fill_Color(FALSE,eFILL_COLOR_BLACK);  //A70LV_Doulas_0049 modify//disable background color

        dvC341_Write_ch_v(eC341_CH_V0, B8_PGCTRLCH1,cTpye,CH_BANK_OFFSET);
        switch(cTpye)
        {
            case 1: //Horizon Color bar
                dvC341_Write_ch_v(eC341_CH_V0, B8_HLIMITMAXCH1,(UINT32)(m_sChannelInfo[0].sOutputTimingInfo.uiHActive/8), CH_BANK_OFFSET);
                break;

            case 2: //Vertical Color bar
                dvC341_Write_ch_v(eC341_CH_V0, B8_VLIMITMAXCH1,(UINT32)(m_sChannelInfo[0].sOutputTimingInfo.uiVActive/8), CH_BANK_OFFSET);
                break;

            case 3: //Horizon RAMP
                dvC341_Write_ch_v(eC341_CH_V0, B8_PGPTCTRLCH1,0x00, CH_BANK_OFFSET);
                dvC341_Write_ch_v(eC341_CH_V0, B8_HLIMITMAXCH1,0x3ff, CH_BANK_OFFSET);     //A70LV_Doulas_0040 modify
                dvC341_Write_ch_v(eC341_CH_V0, B8_HRAMPCNTCH1,0x01, CH_BANK_OFFSET);
                break;

            case 4: //FILL
                dvC341_Write_ch_v(eC341_CH_V0, B8_PGCOLRCH1,0x00, CH_BANK_OFFSET);
                dvC341_Write_ch_v(eC341_CH_V0, B8_PGCOLGCH1,0xff, CH_BANK_OFFSET);
                dvC341_Write_ch_v(eC341_CH_V0, B8_PGCOLBCH1,0x00, CH_BANK_OFFSET);

                dvC341_Write_ch_v(eC341_CH_V0, B8_PGCOLLSRCH1,0x0, CH_BANK_OFFSET);
                dvC341_Write_ch_v(eC341_CH_V0, B8_PGCOLLSGCH1,0x03, CH_BANK_OFFSET);
                dvC341_Write_ch_v(eC341_CH_V0, B8_PGCOLLSBCH1,0x0, CH_BANK_OFFSET);
                break;

            case 5: //Checker
                dvC341_Write_ch_v(eC341_CH_V0, B8_HLIMITMAXCH1,(UINT32)(m_sChannelInfo[0].sOutputTimingInfo.uiHActive/4), CH_BANK_OFFSET);
                dvC341_Write_ch_v(eC341_CH_V0, B8_VLIMITMAXCH1,(UINT32)(m_sChannelInfo[0].sOutputTimingInfo.uiVActive/4), CH_BANK_OFFSET);

                dvC341_Write_ch_v(eC341_CH_V0, B8_PGCOLRCH1,0xff, CH_BANK_OFFSET);
                dvC341_Write_ch_v(eC341_CH_V0, B8_PGCOLGCH1,0xff, CH_BANK_OFFSET);
                dvC341_Write_ch_v(eC341_CH_V0, B8_PGCOLBCH1,0xff, CH_BANK_OFFSET);

                dvC341_Write_ch_v(eC341_CH_V0, B8_PGCOLLSRCH1,0x03, CH_BANK_OFFSET);
                dvC341_Write_ch_v(eC341_CH_V0, B8_PGCOLLSGCH1,0x03, CH_BANK_OFFSET);
                dvC341_Write_ch_v(eC341_CH_V0, B8_PGCOLLSBCH1,0x03, CH_BANK_OFFSET);
                break;

            case 6: //Red and Blue
                dvC341_Write_ch_v(eC341_CH_V0, B8_HLIMITMAXCH1,0x03C0, CH_BANK_OFFSET);

                dvC341_Write_ch_v(eC341_CH_V0, B8_PGCOLRCH1,0xff, CH_BANK_OFFSET);
                dvC341_Write_ch_v(eC341_CH_V0, B8_PGCOLGCH1,0xff, CH_BANK_OFFSET);
                dvC341_Write_ch_v(eC341_CH_V0, B8_PGCOLBCH1,0xff, CH_BANK_OFFSET);

                dvC341_Write_ch_v(eC341_CH_V0, B8_PGCOLLSRCH1,0x03, CH_BANK_OFFSET);
                dvC341_Write_ch_v(eC341_CH_V0, B8_PGCOLLSGCH1,0x03, CH_BANK_OFFSET);
                dvC341_Write_ch_v(eC341_CH_V0, B8_PGCOLLSBCH1,0x03, CH_BANK_OFFSET);
                break;

            case 7: //Display window
                dvC341_Write_ch_v(eC341_CH_V0, B8_HLIMITMAXCH1,0x0080, CH_BANK_OFFSET);
                dvC341_Write_ch_v(eC341_CH_V0, B8_VLIMITMAXCH1,0x0060, CH_BANK_OFFSET);

                dvC341_Write_ch_v(eC341_CH_V0, B8_PGCOLRCH1,0xff, CH_BANK_OFFSET);
                dvC341_Write_ch_v(eC341_CH_V0, B8_PGCOLGCH1,0xff, CH_BANK_OFFSET);
                dvC341_Write_ch_v(eC341_CH_V0, B8_PGCOLBCH1,0xff, CH_BANK_OFFSET);

                dvC341_Write_ch_v(eC341_CH_V0, B8_PGCOLLSRCH1,0x03, CH_BANK_OFFSET);
                dvC341_Write_ch_v(eC341_CH_V0, B8_PGCOLLSGCH1,0x03, CH_BANK_OFFSET);
                dvC341_Write_ch_v(eC341_CH_V0, B8_PGCOLLSBCH1,0x03, CH_BANK_OFFSET);
                break;

            case 8: //Display window
                dvC341_Write_ch_v(eC341_CH_V0, B8_PGPTCTRLCH1,0x30, CH_BANK_OFFSET);

                dvC341_Write_ch_v(eC341_CH_V0, B8_HLIMITMAXCH1, 24, CH_BANK_OFFSET);
                dvC341_Write_ch_v(eC341_CH_V0, B8_VLIMITMAXCH1, 24, CH_BANK_OFFSET);

                dvC341_Write_ch_v(eC341_CH_V0, B8_PGCOLRCH1,0xff, CH_BANK_OFFSET);
                dvC341_Write_ch_v(eC341_CH_V0, B8_PGCOLGCH1,0xff, CH_BANK_OFFSET);
                dvC341_Write_ch_v(eC341_CH_V0, B8_PGCOLBCH1,0xff, CH_BANK_OFFSET);

                dvC341_Write_ch_v(eC341_CH_V0, B8_PGCOLLSRCH1,0x03, CH_BANK_OFFSET);
                dvC341_Write_ch_v(eC341_CH_V0, B8_PGCOLLSGCH1,0x03, CH_BANK_OFFSET);
                dvC341_Write_ch_v(eC341_CH_V0, B8_PGCOLLSBCH1,0x03, CH_BANK_OFFSET);
                break;

            case 11: //RGB "T"character 32dot
                dvC341_Write_ch_v(eC341_CH_V0, B8_PGPTCTRLCH1,0x30, CH_BANK_OFFSET);

                dvC341_Write_ch_v(eC341_CH_V0, B8_PGCOLRCH1,0xff, CH_BANK_OFFSET);
                dvC341_Write_ch_v(eC341_CH_V0, B8_PGCOLGCH1,0xff, CH_BANK_OFFSET);
                dvC341_Write_ch_v(eC341_CH_V0, B8_PGCOLBCH1,0xff, CH_BANK_OFFSET);

                dvC341_Write_ch_v(eC341_CH_V0, B8_PGCOLLSRCH1,0x03, CH_BANK_OFFSET);
                dvC341_Write_ch_v(eC341_CH_V0, B8_PGCOLLSGCH1,0x03, CH_BANK_OFFSET);
                dvC341_Write_ch_v(eC341_CH_V0, B8_PGCOLLSBCH1,0x03, CH_BANK_OFFSET);
                break;

            case 14: //Stripe Horizon
                dvC341_Write_ch_v(eC341_CH_V0, B8_HLIMITMAXCH1,0x0001, CH_BANK_OFFSET);

                dvC341_Write_ch_v(eC341_CH_V0, B8_PGCOLRCH1,0xff, CH_BANK_OFFSET);
                dvC341_Write_ch_v(eC341_CH_V0, B8_PGCOLGCH1,0xff, CH_BANK_OFFSET);
                dvC341_Write_ch_v(eC341_CH_V0, B8_PGCOLBCH1,0xff, CH_BANK_OFFSET);

                dvC341_Write_ch_v(eC341_CH_V0, B8_PGCOLLSRCH1,0x03, CH_BANK_OFFSET);
                dvC341_Write_ch_v(eC341_CH_V0, B8_PGCOLLSGCH1,0x03, CH_BANK_OFFSET);
                dvC341_Write_ch_v(eC341_CH_V0, B8_PGCOLLSBCH1,0x03, CH_BANK_OFFSET);
                break;

            case 15: //Stripe Vertical
                dvC341_Write_ch_v(eC341_CH_V0, B8_VLIMITMAXCH1,0x0001, CH_BANK_OFFSET);

                dvC341_Write_ch_v(eC341_CH_V0, B8_PGCOLRCH1,0xff, CH_BANK_OFFSET);
                dvC341_Write_ch_v(eC341_CH_V0, B8_PGCOLGCH1,0xff, CH_BANK_OFFSET);
                dvC341_Write_ch_v(eC341_CH_V0, B8_PGCOLBCH1,0xff, CH_BANK_OFFSET);

                dvC341_Write_ch_v(eC341_CH_V0, B8_PGCOLLSRCH1,0x03, CH_BANK_OFFSET);
                dvC341_Write_ch_v(eC341_CH_V0, B8_PGCOLLSGCH1,0x03, CH_BANK_OFFSET);
                dvC341_Write_ch_v(eC341_CH_V0, B8_PGCOLLSBCH1,0x03, CH_BANK_OFFSET);
                break;


            default:
                dvC341_Write_ch_v(eC341_CH_V0, B8_PGCTRLCH1,0, CH_BANK_OFFSET);
                break;

        }

        dvC341_Write_ch_v(eC341_CH_V0, B8_OACTHSTCH1,m_sChannelInfo[0].sOutputTimingInfo.uiHStart, CH_BANK_OFFSET);
        dvC341_Write_ch_v(eC341_CH_V0, B8_OACTHWCH1,m_sChannelInfo[0].sOutputTimingInfo.uiHActive, CH_BANK_OFFSET);
        dvC341_Write_ch_v(eC341_CH_V0, B8_OACTVSTCH1,m_sChannelInfo[0].sOutputTimingInfo.uiVStart-5, CH_BANK_OFFSET);
        dvC341_Write_ch_v(eC341_CH_V0, B8_OACTVWCH1,m_sChannelInfo[0].sOutputTimingInfo.uiVActive+1, CH_BANK_OFFSET);

        dvC341_Write_ch_v(eC341_CH_V0, B10_HZOOMCTCH1,0x00, CH_BANK_OFFSET);     //A70LV_Doulas_0081 modify TP bug
        dvC341_Write_ch_v(eC341_CH_V0, B17_HSHRNKCTCH1,0x00, CH_BANK_OFFSET);
        dvC341_Write_ch_v(eC341_CH_V0, B10_VZOOMCTCH1,0x00, CH_BANK_OFFSET);
        dvC341_Write_ch_v(eC341_CH_V0, B17_VSHRNKCTCH1,0x00, CH_BANK_OFFSET);

        data = dvC341_Read(B0_MCT1,0);
        dvC341_Write(B0_MCT1, 0x0D | data,0) ;
        dvC341_Write(B0_MCT2, 0x0D | data,0) ;
        dvC341_Write_ch_v(eC341_CH_V0, B10_LSCCTCH1,	0x00, CH_BANK_OFFSET);
        dvC341_Write_ch_v(eC341_CH_V0, B144_WPBLKCTCH1,	0x00, CH_WPBANK_OFFSET);

        dvC341_Channel1_Auto_Fill_Screen_Set(FALSE);
        dvC341_Channel2_Auto_Fill_Screen_Set(FALSE);
        dvC341_Warp_Auto_Fill_Screen_Set(FALSE);
    }
    else
    {
        dvC341_Write_ch_v(eC341_CH_V0, B8_PGCTRLCH1, 0, CH_BANK_OFFSET);
        #ifdef C341_WARPING_ENABLE      //A70LV_Doulas_0100 M0dify
        dvC341_Write_ch_v(eC341_CH_V0, B144_WPBLKCTCH1, ucWarpInit,CH_WPBANK_OFFSET);  //A70LV_Doulas_0079
        #endif
        dvC341_SetInputPort_2K(eC341_CH_V0);
        dvC341_Auto_Fill_Screen_Init();
    }
}

static void dvC341_Set_Combing(const eC341_CH_V ch_v)
{
    if(ch_v >= eC341_CH_VNUMBER)
    {
        return;
    }

    UINT8 ucRegOffset = (ch_v * CH_V_BANK_OFFSET);

    ucPM_COMB[ch_v] = 0; //default

    if(m_sChannelInfo[ch_v].sInputTimingInfo.lSTATE <= eINPUT_PORT_INFO_STATE_4CH)
    {
        dvC341_WriteToBuffer(B20_COMBCTCH1,	    0x00, ucRegOffset);
    }
    else
    {
	    if ( ucPM_COMB[ch_v]==1)           //weak
        {
    		dvC341_WriteToBuffer(B20_COMBCTCH1,	    0x01, ucRegOffset);		//Combing process control
    		dvC341_WriteToBuffer(B20_COMBLTHCH1,	0x10, ucRegOffset);		//Combing process low level threshold
    		dvC341_WriteToBuffer(B20_COMBHTHCH1,	0x80, ucRegOffset);		//Combing process high level threshold
    	}
        else if ( ucPM_COMB[ch_v]==2)      //mid
    	{
    		dvC341_WriteToBuffer(B20_COMBCTCH1,	    0x01, ucRegOffset);
    		dvC341_WriteToBuffer(B20_COMBLTHCH1,	0x14, ucRegOffset);
    		dvC341_WriteToBuffer(B20_COMBHTHCH1,	0x80, ucRegOffset);
    	}
        else if ( ucPM_COMB[ch_v]==3)      //strong
        {
    		dvC341_WriteToBuffer(B20_COMBCTCH1,	    0x01, ucRegOffset);
    		dvC341_WriteToBuffer(B20_COMBLTHCH1,	0x70, ucRegOffset);
    		dvC341_WriteToBuffer(B20_COMBHTHCH1,	0xa0, ucRegOffset);
    	}
        else
    	{
    		dvC341_WriteToBuffer(B20_COMBCTCH1,	    0x00, ucRegOffset);
    	}
    }

    dvC341_Buffer_Flush();
}



void dvC341_Set_IP(const eC341_CH_V ch_v) //de-interlace block
{
	UINT8 ucNren[eC341_CH_VNUMBER];
    UINT32 ulval;
    UINT8 ucFLG_HSHRNK;
//	double tmpd;
//    int iphsyrdly;
//    int val4,val5;
    UINT8 ucRegOffset = 0;
    UINT8 uc3D_Format;  //A70LV_Doulas_0185
    INT16 input_VST_Shift = 0; //H30K_Doulas_0041

    if(ch_v >= eC341_CH_VNUMBER)
    {
        return;
    }

    #if 0   //A70LV_Doulas_0003
	ucNren[0] =(ucPM_HNR[0]!=0) | (ucPM_VNR[0]!=0) |
			 (ucPM_TNR[0]!=0) | (ucPM_BNR[0]!=0) | (ucPM_MNR[0]!=0);
	ucNren[1] =(ucPM_HNR[1]!=0) | (ucPM_VNR[1]!=0) |
			 (ucPM_TNR[1]!=0) | (ucPM_BNR[1]!=0) | (ucPM_MNR[1]!=0);
    #else
    ucNren[0] = 0;
    ucNren[1] = 0;
    #endif

	ucFLG_HSHRNK = (m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiHActive > m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiHActive ) ? 1 : 0;

    switch(dvC341_Input_3D_Format_Config_Get())    //A70LV_Doulas_0154
    {
        case eINPUT_3D_TYPE_FRAMEPACKING:
        case eINPUT_3D_TYPE_SIDEBYSIDE:
        case eINPUT_3D_TYPE_TOPANDBOTTOM:
            //ucNren[ch_v] = 1; //H30K_Doulas_0001 remove
			if((dvC341_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_SIDEBYSIDE) &&
               (m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.eScanMode == eSCAN_MODE_PROGRESSIVE) &&
               ((m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive > 1918 && m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive < 1922) && (m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive == 1080)) &&
               (m_sChannelInfo[ch_v].sInputTimingInfo.dFrameRate > 49.0 && m_sChannelInfo[ch_v].sInputTimingInfo.dFrameRate < 61.0))  //G100_Doulas_0064
            {
                ucNren[ch_v] = 0;
            }
            break;

        default:
            break;
    }

    if(m_sChannelInfo[ch_v].sInputTimingInfo.lSTATE <= eINPUT_PORT_INFO_STATE_4CH) // 4k input//H30K_Doulas_0058
        dvC341_Write_ch_v(ch_v, B16_DIFCTCH1, 0x80, CH_BANK_OFFSET);  //blanking不用黑取代
    else
        dvC341_Write_ch_v(ch_v, B16_DIFCTCH1, 0x81, CH_BANK_OFFSET);  //取消左邊線偏綠(blanking用黑取代),but 4K input中間有條線的副作用
//-----------------------------------------------------------------------------
	if ((m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.eScanMode == eSCAN_MODE_INTERLACE) | ucNren[ch_v]) {//enable IP block
//IPsync
        #if 0
		val = halC341_calc_ipohcyc(ch);						// IPCLK = 148.5MHz Base
		tmpd = (PS_IPCLK / 148500000.0) * val;
		val = (int)(tmpd)  + 0x20                  ;
		dvC341_Write(B11_IPOHCYCLCH1,val, ch);	//IP conversion output port horizontal sync signal cycle

		iphsyrdly = val * 3 / 4 + 1;
		dvC341_Write(B11_IPHSYRDLYCH1,((iphsyrdly & 0x3fff) | 0x8000),ch);	//IP conversion horizontal synchronization signal delay
        #endif
        dvC341_SetIpConv(ch_v,TRUE);
//		val = ip_mode[ch] ? 0x01   : 0x01          ;


        ulval = 0x01;
		dvC341_Write_ch_v(ch_v, B20_IPSYRDLYCH1,ulval, 16);	//IP conversion forced sync reset delay

		//ulval = (ucFLG_HSHRNK) ? m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiHActive : m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiHActive ; //H30K_Doulas_0049
		if(dvC341_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_SIDEBYSIDE) //H30K_Doulas_0049
		    ulval = m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.uiHActive / 2 ;
		else
		    ulval = m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.uiHActive ;

		dvC341_Write_ch_v(ch_v, B20_IPOACTHWCH1,ulval, 16);	//IP conversion output port active area horizontal direction width

		ulval = m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.uiVActive; //H30K_Doulas_0049 Modify
		dvC341_Write_ch_v(ch_v, B20_IPOACTVWCH1,ulval, 16);			//IP conversion output port active area vertical direction width

        if((ch_v == eC341_CH_V0) &&
            (m_sChannelSetting[eC341_CH_V0].cPIP_PBP_ON == FALSE) &&
            (m_sChannelInfo[eC341_CH_V0].ePanelTimingId == PANEL_2D_OUTPUT)) //H30K_Doulas_0044//H30K_Doulas_0041
        {
            #if 1 //interlaced not suport image shift
            input_VST_Shift = 0;
            #else
            input_VST_Shift = WorkingVertPosition[ch_v] - VERT_POSITION_DEFAULT;
            if((WorkingVertPosition[ch_v] < VERT_POSITION_DEFAULT) && (m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiVStart > 0))
            {
                if(m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiVStart >= (VERT_POSITION_DEFAULT - WorkingVertPosition[ch_v]))
                {
                    input_VST_Shift = 0;
                }
                else
                {
                    input_VST_Shift = (INT16)m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiVStart + WorkingVertPosition[ch_v] - VERT_POSITION_DEFAULT;
                }
            }
            else
            {
                input_VST_Shift = WorkingVertPosition[ch_v] - VERT_POSITION_DEFAULT;
            }
            #endif
        }
		//val =( (PI_ACT_VST[ch] -1) * 2 ) + 0x8000 ;
		if(m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.eScanMode == eSCAN_MODE_INTERLACE) //H30K_Doulas_0001
		    ulval =((((m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.uiVStart + input_VST_Shift + 1) - 1) * 2 - (1-1)) + 0x8000) & 0xFFFE ; //((IACTVSTCH1 - 1) * 2 - (IPSYRDLYCH1 - 1))
		else
		    ulval =(((m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.uiVStart + 1 - 1) * 1 - (1-1)) + 0x8000) & 0xFFFE ;   //A70LV_Doulas_0009 modify jittered image   //A70LV_Doulas_0003
		dvC341_Write_ch_v(ch_v, B20_IPOACTVSTCH1,ulval, 16); //32768=0x8000  //IP conversion output port active area vertical direction start point

        if(m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.eScanMode == eSCAN_MODE_INTERLACE) //H30K_Doulas_0001
            ulval = m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.uiVTotal * 2 +30;
        else
            ulval = m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.uiVTotal+30;
        dvC341_Write_ch_v(ch_v, B20_IPOVCYCLCH1, ulval, CH_BANK_OFFSET);  //H30K_Doulas_0001

//PM_MVFLT
		dvC341_Set_Movement_NR(ch_v);
//PM_DIAG
		dvC341_Set_Diagonal_Interpolation_Control(ch_v);
//set IP registers
		dvC341_Set_Interlace_Regs(ch_v);
//PM_COMB
		dvC341_Set_Combing(ch_v);
//PM_*NR
		dvC341_Set_NR(ch_v);
//PM_IP_DELAY
		/*
		if ( PM_IP_DELAY[ch]==1 ) {
			set_ip_lut_L7_EP23(ch);		//set_ip_lut_L7(ch);
			val=0x00;
		} else {
			set_ip_lut_L7_1F_EP23(ch);	//set_ip_lut_L7_1F(ch);
			val=0x02;
		}
		*/
//		val = (ip_mode[ch]==0 && PI_SCAN_MODE[ch]==1) ? val|0xc0 : val;
//		halC341_Write(B11_IPADJCTCH1,val,ch);
        ucNren[ch_v] = 0;    //A70LV_Doulas_0154
	}

	else
    {
		dvC341_Write_ch_v(ch_v, B21_UVNRCTCH1, 0, 16);	//Chroma error correction control
        dvC341_SetIpConv(ch_v,FALSE);
		dvC341_Set_NR(ch_v);
	}
    dvC341_set_scfmfmt(ch_v, FRAME_MEMORY_FORMAT);   //H2PF_Simon_0131

//-----------------------------------------------------------------------------
	ulval= (m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.eScanMode == eSCAN_MODE_PROGRESSIVE) ? 0x00: 0x80;	//Field auto recognition enable
	dvC341_Write_ch_v(ch_v, B16_AUTOFLDCH1, ulval, CH_BANK_OFFSET);		//Input field recognition control
//-----------------------------------------------------------------------------
	ulval= (m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.eScanMode == eSCAN_MODE_PROGRESSIVE) & ucNren[ch_v] 	? 0x01://progressive IP   //A70LV_Doulas_0003
		// (PM_IPEN[ch]==0)				? 0x01://interlace field merge
										  0x00;//interlace moving adaptive

	dvC341_Write_ch_v(ch_v, B20_IPTESTCH1 ,ulval, CH_BANK_OFFSET);		//IP test

    //dvC341_WriteToBuffer(B11_IPOACTHSTCH1, 0x0040, ucRegOffset);    //A70LV_Doulas_0154
    uc3D_Format = dvC341_Input_3D_Format_Config_Get();    //A70LV_Doulas_0185
    switch(uc3D_Format)    //A70LV_Doulas_0185 modify //A70LV_Doulas_0154
    {
        case eINPUT_3D_TYPE_SIDEBYSIDE:
        case eINPUT_3D_TYPE_FRAMEPACKING:
        case eINPUT_3D_TYPE_TOPANDBOTTOM:
			if((uc3D_Format == eINPUT_3D_TYPE_SIDEBYSIDE) &&
               (m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.eScanMode == eSCAN_MODE_PROGRESSIVE) &&
               ((m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive > 1918 && m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive < 1922) && (m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive == 1080)) &&
               (m_sChannelInfo[ch_v].sInputTimingInfo.dFrameRate > 49.0 && m_sChannelInfo[ch_v].sInputTimingInfo.dFrameRate < 61.0))  //G100_Doulas_0064
            {
                break;
            }

            if((m_sChannelInfo[ch_v].sInputTimingInfo.dFrameRate > 49.0 && m_sChannelInfo[ch_v].sInputTimingInfo.dFrameRate < 51.0) &&
               (uc3D_Format == eINPUT_3D_TYPE_SIDEBYSIDE))
            {
                dvC341_Write_ch_v(ch_v, B20_IPOHCYCLCH1, 0x16a0, CH_BANK_OFFSET); //H30K_Doulas_0001
            }
            else
            {
                dvC341_Write_ch_v(ch_v, B20_IPOHCYCLCH1, 0x12e0, CH_BANK_OFFSET); //H30K_Doulas_0001
            }
            dvC341_Write_ch_v(ch_v, B20_IPOVCYCLCH1, 0x486, CH_BANK_OFFSET); //H30K_Doulas_0001
            dvC341_Write_ch_v(ch_v, B20_IPSYRDLYCH1, 8, CH_BANK_OFFSET);     //H30K_Doulas_0001

            if(m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.eScanMode == eSCAN_MODE_INTERLACE) //H30K_Doulas_0001
                ulval =((((m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.uiVStart + 1) - 1) * 2 - (8-1)) + 0x8000) & 0xFFFE ; //((IACTVSTCH1 - 1) * 2 - (IPSYRDLYCH1 - 1))
            else
                ulval =(((m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.uiVStart + 1 - 1) * 1 - (8-1)) + 0x8000) & 0xFFFE ;
		    dvC341_Write_ch_v(ch_v, B20_IPOACTVSTCH1,ulval, 16);

            dvC341_Write_ch_v(ch_v, B20_IPOACTVWCH1, 1080, CH_BANK_OFFSET);

            dvC341_Write_ch_v(ch_v, B16_AUTOFLDCH1, 0x80, CH_BANK_OFFSET);

            //dvC341_WriteToBuffer(B11_IPHSYRDLYCH1, 0x86f0, ucRegOffset);
            dvC341_Write_ch_v(ch_v, B20_IPHSYRDLYCH1, (0x0e28 & 0x3FFF) | 0x8000, CH_BANK_OFFSET); //H30K_Doulas_0001
            break;

        default:
            if((m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.eScanMode == eSCAN_MODE_INTERLACE) &&
               (dvC341_IS_3D_Enable() == ets_ON))//H30K_Doulas_0034 frame sequential & interlaced
            {
                UINT16 uiIpHTotal = 1;

                if(m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive > 1918 && m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive < 1922)
                {
                    if(m_sChannelInfo[ch_v].sInputTimingInfo.dFrameRate > 49.0 && m_sChannelInfo[ch_v].sInputTimingInfo.dFrameRate < 51.0)
                        uiIpHTotal = 0x16a0;
                    else
                        uiIpHTotal = 0x12e0;
                }
                else
                {
                    DOUBLE dPiclk = (DOUBLE)(m_sChannelInfo[ch_v].sInputTimingInfo.fPICLK/1000000) ;// MHz
                    DOUBLE dPoclk = (DOUBLE)m_sChannelInfo[ch_v].sOutputTimingInfo.uiPixClk/100;// MHz
                    uiIpHTotal = (UINT16)(((DOUBLE)m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.uiHTotal) * dPoclk / (dPiclk * 4) * 1.1 + 1);
                }
                dvC341_Write_ch_v(ch_v, B20_IPOHCYCLCH1, uiIpHTotal, CH_BANK_OFFSET);
                uiIpHTotal = uiIpHTotal * 3 / 4 + 1;
                dvC341_Write_ch_v(ch_v, B20_IPHSYRDLYCH1, (uiIpHTotal & 0x3FFF) | 0x8000, CH_BANK_OFFSET);
            }
            break;
    }
//-----------------------------------------------------------------------------
    dvC341_Set_IPMemory_2K(ch_v);
//-----------------------------------------------------------------------------

    dvC341_Buffer_Flush();
	return;
}

void dvC341_Channel1_Auto_Fill_Screen_Set(BOOL bEnable) //A70LV_Doulas_0002
{
    if(bEnable) //no signal auto fill
    {
	    dvC341_Write(B8_AUTOFILLCTCH1,  0x00c5,0);	//Auto fill control   //simon check
	    dvC341_Write(B24_AUTOFILLCTCH2, 0x00c5,0);	//Auto fill control
    }
	else
    {
	    dvC341_Write(B8_AUTOFILLCTCH1,  0x0045,0);	//Auto fill control
	    dvC341_Write(B24_AUTOFILLCTCH2, 0x0045,0);	//Auto fill control
    }
}

void dvC341_Channel2_Auto_Fill_Screen_Set(BOOL bEnable) //A70LV_Doulas_0002
{
    if(bEnable) //no signal auto fill
    {
	    dvC341_Write(B40_AUTOFILLCTCH3, 0x02c5,0);	//Auto fill control   //simon check
	    dvC341_Write(B56_AUTOFILLCTCH4, 0x02c5,0);	//Auto fill control
    }
	else
    {
	    dvC341_Write(B40_AUTOFILLCTCH3, 0x0245,0);	//Auto fill control
	    dvC341_Write(B56_AUTOFILLCTCH4, 0x02c5,0);	//Auto fill control
    }
}

void dvC341_Warp_Auto_Fill_Screen_Set(BOOL bEnable) //A70LV_Doulas_0002
{
    #if 0   //simon check
    if(bEnable) //no signal auto fill
    {
	    dvC341_Write(B30_AUTOFILLCT,0xc5,0);		//Automatic fill control
    }
	else
    {
	    dvC341_Write(B30_AUTOFILLCT,0x45,0);		//Automatic fill control
    }
    #endif
}

void dvC341_Auto_Fill_Screen_Init(void) //A70LV_Doulas_0002
{
	dvC341_Channel1_Auto_Fill_Screen_Set(TRUE);
    dvC341_Channel2_Auto_Fill_Screen_Set(TRUE);
    //dvC341_Warp_Auto_Fill_Screen_Set(TRUE); //A70LV_Larry_0060
}

void dvC341_Auto_Fill_Screen_Setting(BOOL bSetting) //G100_Steven_0050
{
	dvC341_Channel1_Auto_Fill_Screen_Set(bSetting);
    dvC341_Channel2_Auto_Fill_Screen_Set(bSetting);
    //dvC341_Warp_Auto_Fill_Screen_Set(TRUE);//G100_Steven_0050
}

void dvC341_Field_Interlock_Transfer_Control_Start(const eC341_CH_V ch_v)    //A70LV_Doulas_0002
{
    dvC341_Write(B0_FLDRTCTCH12 + 0x200*ch_v, 0x85+0x20*ch_v, 0);		//Field interlock transfer control
}

void dvC341_Field_Interlock_Transfer_Control_End(const eC341_CH_V ch_v)  //A70LV_Doulas_0002
{
    if(m_sChannelInfo[ch_v].sInputTimingInfo.eScanMode != eSCAN_MODE_NO_SIGNAL)
    {
    	dvC341_Write(B0_FLDRTCT2CH12 + 0x200*ch_v, 0x01, 0);		//Field interlock transfer control 2

		UINT8 wait_loop = 0;
		UINT32 rdata = dvC341_Read(B0_FLDRTCT2CH12 + 0x200*ch_v, 0);
		while (rdata & 0x70)
		{
			MS_SLEEP(3);

			wait_loop++;
			if (wait_loop > 20)
			    break;

			rdata = dvC341_Read(B0_FLDRTCT2CH12 + 0x200*ch_v, 0);
		};

    	dvC341_Write(B0_FLDRTCTCH12 + 0x200*ch_v, 0x00, 0);

		if (wait_loop > 20)
		{
			dvC341_wait1_povs(ch_v);
		}
    }
    else
    {
        dvC341_Write(B0_FLDRTCTCH12 + 0x200*ch_v, 0x00, 0);		//Field interlock transfer control
		dvC341_wait1_povs(ch_v);
    }
}

void dvC341_Set_Color_Matrix(const eC341_CH_V ch_v,
                                     INT32 CMATRIX_SEL,//input color space.        0:BT-601, 1:BT-709
                                     INT32 ITEMP_SEL,  //input color temperature.  0:6500K , 1:9300K
                                     INT32 OTEMP_SEL,  //output color temperature. 0:6500K , 1:9300K
                                     INT32 HUE,
                                     DOUBLE dSaturation)    //A70LV_Doulas_0003  //A70LV_Doulas_0002
{
    //  dSaturation(0-3.99) for Saturation;
    //  HUE(-180-180) for Hue
    DOUBLE a,b,c,p,q,r,s,t,u;
    //DOUBLE yuvrgb[3][3];
    INT32 val;

    DOUBLE dYuvRgb1[3][3];
 //   DOUBLE dYuvRgb2[3][3];    //A70LV_Doulas_0008 modify
    DOUBLE dMatrix1[3][3];
    DOUBLE dMatrix2[3][3];
 //   DOUBLE dMatrix3[3][3];      //A70LV_Doulas_0008 modify
    UINT8 ucIndex1, ucIndex2;

    if(ch_v >= eC341_CH_VNUMBER)
    {
        return;
    }


    if(m_sChannelSetting[ch_v].eColorFormat == eCOLOR_FORMAT_RGB || m_sChannelSetting[ch_v].eColorFormat == eCOLOR_FORMAT_RGB_LIMIT)      //A70LV_Doulas_0080
    {
        CMATRIX_SEL = 1;    //REC-709, because input RGB to REC709,so output REC709 to RGB
    }

	val = CMATRIX_SEL*4 + ITEMP_SEL*2 + OTEMP_SEL;
    //Matrix range Y(0~255),Pb/Pr(-127.5~127.5) to RGB(0~255)
	a = (val==1)|(val==5) ? 0.8457
	  : (val==2)|(val==6) ? 1.1824
	  :                     1.0000;
	b = (val==1)|(val==5) ? 1.0112
	  : (val==2)|(val==6) ? 0.9889
	  :                     1.0000;
	c = (val==1)|(val==5) ? 1.3437
	  : (val==2)|(val==6) ? 0.7442
	  :                     1.0000;
	p=0;
	q = (val==0)|(val==3) ? -0.3441
	  : (val==1)		  ? -0.3479
	  : (val==2)		  ? -0.3403
	  : (val==4)|(val==7) ? -0.1873
	  : (val==5)		  ? -0.1894
	  :						-0.1852;

	r =	(val==0)|(val==3) ?  1.7720
	  : (val==1)		  ?  2.3811
	  : (val==2)		  ?  1.3187
	  :	(val==4)|(val==7) ?  1.8556
	  : (val==5)		  ?  2.4934
	  :						 1.3809;

	s = (val==0)|(val==3) ?  1.4020
	  : (val==1)		  ?  1.1857
	  : (val==2)		  ?  1.6578
	  :	(val==4)|(val==7) ?  1.5748
	  : (val==5)		  ?  1.3318
	  :						 1.8621;

	t = (val==0)|(val==3) ? -0.7141
	  : (val==1)		  ? -0.7221
	  : (val==2)		  ? -0.7062
	  : (val==4)|(val==7) ? -0.4681
	  : (val==5)		  ? -0.4733
	  :						-0.4629;
	u=0;

    #if 0
    dMatrix1[0][0] = s;
    dMatrix1[1][0] = t;
    dMatrix1[2][0] = u;
    dMatrix1[0][1] = a;
    dMatrix1[1][1] = b;
    dMatrix1[2][1] = c;
    dMatrix1[0][2] = p;
    dMatrix1[1][2] = q;
    dMatrix1[2][2] = r;

    // Hue
    dMatrix2[0][0] =  cos(HUE * 3.14159 / 180);
    dMatrix2[1][0] =  0;
    dMatrix2[2][0] =  sin(HUE * 3.14159 / 180);
    dMatrix2[0][1] =  0;
    dMatrix2[1][1] =  1;
    dMatrix2[2][1] =  0;
    dMatrix2[0][2] = -sin(HUE * 3.14159 / 180);
    dMatrix2[1][2] =  0;
    dMatrix2[2][2] =  cos(HUE * 3.14159 / 180);

    // Saturation Matrix
    dMatrix3[0][0] = dSaturation;
    dMatrix3[1][0] = 0;
    dMatrix3[2][0] = 0;
    dMatrix3[0][1] = 0;
    dMatrix3[1][1] = dSaturation;
    dMatrix3[2][1] = 0;
    dMatrix3[0][2] = 0;
    dMatrix3[1][2] = 0;
    dMatrix3[2][2] = dSaturation;

    dYuvRgb1[0][0] = dMatrix1[0][0] * dMatrix2[0][0] + dMatrix1[0][1] * dMatrix2[1][0] + dMatrix1[0][2] * dMatrix2[2][0];
    dYuvRgb1[0][1] = dMatrix1[0][0] * dMatrix2[0][1] + dMatrix1[0][1] * dMatrix2[1][1] + dMatrix1[0][2] * dMatrix2[2][1];
    dYuvRgb1[0][2] = dMatrix1[0][0] * dMatrix2[0][2] + dMatrix1[0][1] * dMatrix2[1][2] + dMatrix1[0][2] * dMatrix2[2][2];
    dYuvRgb1[1][0] = dMatrix1[1][0] * dMatrix2[0][0] + dMatrix1[1][1] * dMatrix2[1][0] + dMatrix1[1][2] * dMatrix2[2][0];
    dYuvRgb1[1][1] = dMatrix1[1][0] * dMatrix2[0][1] + dMatrix1[1][1] * dMatrix2[1][1] + dMatrix1[1][2] * dMatrix2[2][1];
    dYuvRgb1[1][2] = dMatrix1[1][0] * dMatrix2[0][2] + dMatrix1[1][1] * dMatrix2[1][2] + dMatrix1[1][2] * dMatrix2[2][2];
    dYuvRgb1[2][0] = dMatrix1[2][0] * dMatrix2[0][0] + dMatrix1[2][1] * dMatrix2[1][0] + dMatrix1[2][2] * dMatrix2[2][0];
    dYuvRgb1[2][1] = dMatrix1[2][0] * dMatrix2[0][1] + dMatrix1[2][1] * dMatrix2[1][1] + dMatrix1[2][2] * dMatrix2[2][1];
    dYuvRgb1[2][2] = dMatrix1[2][0] * dMatrix2[0][2] + dMatrix1[2][1] * dMatrix2[1][2] + dMatrix1[2][2] * dMatrix2[2][2];

    dYuvRgb2[0][0] = dMatrix3[0][0] * dYuvRgb1[0][0] + dMatrix3[0][1] * dYuvRgb1[1][0] + dMatrix3[0][2] * dYuvRgb1[2][0];
    dYuvRgb2[0][1] = dMatrix3[0][0] * dYuvRgb1[0][1] + dMatrix3[0][1] * dYuvRgb1[1][1] + dMatrix3[0][2] * dYuvRgb1[2][1];
    dYuvRgb2[0][2] = dMatrix3[0][0] * dYuvRgb1[0][2] + dMatrix3[0][1] * dYuvRgb1[1][2] + dMatrix3[0][2] * dYuvRgb1[2][2];
    dYuvRgb2[1][0] = dMatrix3[1][0] * dYuvRgb1[0][0] + dMatrix3[1][1] * dYuvRgb1[1][0] + dMatrix3[1][2] * dYuvRgb1[2][0];
    dYuvRgb2[1][1] = dMatrix3[1][0] * dYuvRgb1[0][1] + dMatrix3[1][1] * dYuvRgb1[1][1] + dMatrix3[1][2] * dYuvRgb1[2][1];
    dYuvRgb2[1][2] = dMatrix3[1][0] * dYuvRgb1[0][2] + dMatrix3[1][1] * dYuvRgb1[1][2] + dMatrix3[1][2] * dYuvRgb1[2][2];
    dYuvRgb2[2][0] = dMatrix3[2][0] * dYuvRgb1[0][0] + dMatrix3[2][1] * dYuvRgb1[1][0] + dMatrix3[2][2] * dYuvRgb1[2][0];
    dYuvRgb2[2][1] = dMatrix3[2][0] * dYuvRgb1[0][1] + dMatrix3[2][1] * dYuvRgb1[1][1] + dMatrix3[2][2] * dYuvRgb1[2][1];
    dYuvRgb2[2][2] = dMatrix3[2][0] * dYuvRgb1[0][2] + dMatrix3[2][1] * dYuvRgb1[1][2] + dMatrix3[2][2] * dYuvRgb1[2][2];
/*
	yuvrgb[0][0]=(p*sin(HUE*3.14159/180)+s*cos(HUE*3.14159/180));
	yuvrgb[1][0]=(q*sin(HUE*3.14159/180)+t*cos(HUE*3.14159/180));
	yuvrgb[2][0]=(r*sin(HUE*3.14159/180)+u*cos(HUE*3.14159/180));
	yuvrgb[0][1]=a;
	yuvrgb[1][1]=b;
	yuvrgb[2][1]=c;
	yuvrgb[0][2]=(p*cos(HUE*3.14159/180)-s*sin(HUE*3.14159/180));
	yuvrgb[1][2]=(q*cos(HUE*3.14159/180)-t*sin(HUE*3.14159/180));
	yuvrgb[2][2]=(r*cos(HUE*3.14159/180)-u*sin(HUE*3.14159/180));
*/
	val=(INT32)(dYuvRgb2[0][0]*8192+0.5)&0xffff; dvC341_Write(B6_OCSC1CF00CH1,val,ucRegOffset);	//Output color conversion 1 coefficient
	val=(INT32)(dYuvRgb2[0][1]*8192+0.5)&0xffff; dvC341_Write(B6_OCSC1CF01CH1,val,ucRegOffset);
	val=(INT32)(dYuvRgb2[0][2]*8192+0.5)&0xffff; dvC341_Write(B6_OCSC1CF02CH1,val,ucRegOffset);
	val=(INT32)(dYuvRgb2[1][0]*8192+0.5)&0xffff; dvC341_Write(B6_OCSC1CF10CH1,val,ucRegOffset);
	val=(INT32)(dYuvRgb2[1][1]*8192+0.5)&0xffff; dvC341_Write(B6_OCSC1CF11CH1,val,ucRegOffset);
	val=(INT32)(dYuvRgb2[1][2]*8192+0.5)&0xffff; dvC341_Write(B6_OCSC1CF12CH1,val,ucRegOffset);
	val=(INT32)(dYuvRgb2[2][0]*8192+0.5)&0xffff; dvC341_Write(B6_OCSC1CF20CH1,val,ucRegOffset);
	val=(INT32)(dYuvRgb2[2][1]*8192+0.5)&0xffff; dvC341_Write(B6_OCSC1CF21CH1,val,ucRegOffset);
	val=(INT32)(dYuvRgb2[2][2]*8192+0.5)&0xffff; dvC341_Write(B6_OCSC1CF22CH1,val,ucRegOffset);

    #else
#if 0 //A70LV_Doulas_0202 modify saturation algorithm
	dMatrix1[0][0] = (p*sin(HUE*3.14159/180)+s*cos(HUE*3.14159/180));
	dMatrix1[1][0] = (q*sin(HUE*3.14159/180)+t*cos(HUE*3.14159/180));
	dMatrix1[2][0] = (r*sin(HUE*3.14159/180)+u*cos(HUE*3.14159/180));
	dMatrix1[0][1] = a;
	dMatrix1[1][1] = b;
	dMatrix1[2][1] = c;
	dMatrix1[0][2] = (p*cos(HUE*3.14159/180)-s*sin(HUE*3.14159/180));
	dMatrix1[1][2] = (q*cos(HUE*3.14159/180)-t*sin(HUE*3.14159/180));
	dMatrix1[2][2] = (r*cos(HUE*3.14159/180)-u*sin(HUE*3.14159/180));

    // Saturation Matrix
    dMatrix2[0][0] = dSaturation;
    dMatrix2[1][0] = 0;
    dMatrix2[2][0] = 0;
    dMatrix2[0][1] = 0;
    dMatrix2[1][1] = dSaturation;
    dMatrix2[2][1] = 0;
    dMatrix2[0][2] = 0;
    dMatrix2[1][2] = 0;
    dMatrix2[2][2] = dSaturation;
#else
	dMatrix1[0][0] = (p*sin(HUE*3.14159/180)+s*cos(HUE*3.14159/180))*dSaturation;
	dMatrix1[1][0] = (q*sin(HUE*3.14159/180)+t*cos(HUE*3.14159/180))*dSaturation;
	dMatrix1[2][0] = (r*sin(HUE*3.14159/180)+u*cos(HUE*3.14159/180))*dSaturation;
	dMatrix1[0][1] = a;
	dMatrix1[1][1] = b;
	dMatrix1[2][1] = c;
	dMatrix1[0][2] = (p*cos(HUE*3.14159/180)-s*sin(HUE*3.14159/180))*dSaturation;
	dMatrix1[1][2] = (q*cos(HUE*3.14159/180)-t*sin(HUE*3.14159/180))*dSaturation;
	dMatrix1[2][2] = (r*cos(HUE*3.14159/180)-u*sin(HUE*3.14159/180))*dSaturation;

    // Saturation Matrix
    dMatrix2[0][0] = 1;
    dMatrix2[1][0] = 0;
    dMatrix2[2][0] = 0;
    dMatrix2[0][1] = 0;
    dMatrix2[1][1] = 1;
    dMatrix2[2][1] = 0;
    dMatrix2[0][2] = 0;
    dMatrix2[1][2] = 0;
    dMatrix2[2][2] = 1;
#endif
    dYuvRgb1[0][0] = dMatrix2[0][0] * dMatrix1[0][0] + dMatrix2[0][1] * dMatrix1[1][0] + dMatrix2[0][2] * dMatrix1[2][0];
    dYuvRgb1[0][1] = dMatrix2[0][0] * dMatrix1[0][1] + dMatrix2[0][1] * dMatrix1[1][1] + dMatrix2[0][2] * dMatrix1[2][1];
    dYuvRgb1[0][2] = dMatrix2[0][0] * dMatrix1[0][2] + dMatrix2[0][1] * dMatrix1[1][2] + dMatrix2[0][2] * dMatrix1[2][2];
    dYuvRgb1[1][0] = dMatrix2[1][0] * dMatrix1[0][0] + dMatrix2[1][1] * dMatrix1[1][0] + dMatrix2[1][2] * dMatrix1[2][0];
    dYuvRgb1[1][1] = dMatrix2[1][0] * dMatrix1[0][1] + dMatrix2[1][1] * dMatrix1[1][1] + dMatrix2[1][2] * dMatrix1[2][1];
    dYuvRgb1[1][2] = dMatrix2[1][0] * dMatrix1[0][2] + dMatrix2[1][1] * dMatrix1[1][2] + dMatrix2[1][2] * dMatrix1[2][2];
    dYuvRgb1[2][0] = dMatrix2[2][0] * dMatrix1[0][0] + dMatrix2[2][1] * dMatrix1[1][0] + dMatrix2[2][2] * dMatrix1[2][0];
    dYuvRgb1[2][1] = dMatrix2[2][0] * dMatrix1[0][1] + dMatrix2[2][1] * dMatrix1[1][1] + dMatrix2[2][2] * dMatrix1[2][1];
    dYuvRgb1[2][2] = dMatrix2[2][0] * dMatrix1[0][2] + dMatrix2[2][1] * dMatrix1[1][2] + dMatrix2[2][2] * dMatrix1[2][2];

    dYuvRgb1[0][0] = dYuvRgb1[0][0] * 8192;// pow((double)2.0,(double)13.0) = 8192
    dYuvRgb1[1][0] = dYuvRgb1[1][0] * 8192;
    dYuvRgb1[2][0] = dYuvRgb1[2][0] * 8192;
    dYuvRgb1[0][1] = dYuvRgb1[0][1] * 8192;
    dYuvRgb1[1][1] = dYuvRgb1[1][1] * 8192;
    dYuvRgb1[2][1] = dYuvRgb1[2][1] * 8192;
    dYuvRgb1[0][2] = dYuvRgb1[0][2] * 8192;
    dYuvRgb1[1][2] = dYuvRgb1[1][2] * 8192;
    dYuvRgb1[2][2] = dYuvRgb1[2][2] * 8192;

    for(ucIndex2 = 0; ucIndex2 < 3; ucIndex2++)
    {
        for(ucIndex1 = 0; ucIndex1 < 3; ucIndex1++)
        {
            if(dYuvRgb1[ucIndex1][ucIndex2] >= 0)
            {
                if(dYuvRgb1[ucIndex1][ucIndex2] >= 32767)
                {
                    dYuvRgb1[ucIndex1][ucIndex2] = 0x7FFF;
                }
                else
                {
                    dYuvRgb1[ucIndex1][ucIndex2] = 0x0000FFFF & (INT32)(dYuvRgb1[ucIndex1][ucIndex2] + 0.5);
                }
            }
            else
            {
                if(dYuvRgb1[ucIndex1][ucIndex2] <= -32768)
                {
                    dYuvRgb1[ucIndex1][ucIndex2] = 0x8000;
                }
                else
                {
                    dYuvRgb1[ucIndex1][ucIndex2] = 0x0000FFFF & (INT32)(floor(dYuvRgb1[ucIndex1][ucIndex2] - 0.5));
                }
            }
        }
    }

    for(UINT8 ch=0; ch<OUTPUT4K_CH_NUM; ch++)
    {
        UINT8 BankOffset = 16;
        UINT8 ucRegOffset = ((ch_v * 2) + ch) * BankOffset;

    	val=(INT32)(dYuvRgb1[0][0])&0xffff; dvC341_WriteToBuffer(B12_OCSC1CF00CH1,val,ucRegOffset);	//Output color conversion 1 coefficient
    	val=(INT32)(dYuvRgb1[0][1])&0xffff; dvC341_WriteToBuffer(B12_OCSC1CF01CH1,val,ucRegOffset);
    	val=(INT32)(dYuvRgb1[0][2])&0xffff; dvC341_WriteToBuffer(B12_OCSC1CF02CH1,val,ucRegOffset);
    	val=(INT32)(dYuvRgb1[1][0])&0xffff; dvC341_WriteToBuffer(B12_OCSC1CF10CH1,val,ucRegOffset);
    	val=(INT32)(dYuvRgb1[1][1])&0xffff; dvC341_WriteToBuffer(B12_OCSC1CF11CH1,val,ucRegOffset);
    	val=(INT32)(dYuvRgb1[1][2])&0xffff; dvC341_WriteToBuffer(B12_OCSC1CF12CH1,val,ucRegOffset);
    	val=(INT32)(dYuvRgb1[2][0])&0xffff; dvC341_WriteToBuffer(B12_OCSC1CF20CH1,val,ucRegOffset);
    	val=(INT32)(dYuvRgb1[2][1])&0xffff; dvC341_WriteToBuffer(B12_OCSC1CF21CH1,val,ucRegOffset);
    	val=(INT32)(dYuvRgb1[2][2])&0xffff; dvC341_WriteToBuffer(B12_OCSC1CF22CH1,val,ucRegOffset);
        dvC341_Buffer_Flush();
    }
/*
	val=(INT32)(dYuvRgb1[0][0]*8192+0.5)&0xffff; dvC341_Write(B6_OCSC1CF00CH1,val,ucRegOffset);	//Output color conversion 1 coefficient
	val=(INT32)(dYuvRgb1[0][1]*8192+0.5)&0xffff; dvC341_Write(B6_OCSC1CF01CH1,val,ucRegOffset);
	val=(INT32)(dYuvRgb1[0][2]*8192+0.5)&0xffff; dvC341_Write(B6_OCSC1CF02CH1,val,ucRegOffset);
	val=(INT32)(dYuvRgb1[1][0]*8192+0.5)&0xffff; dvC341_Write(B6_OCSC1CF10CH1,val,ucRegOffset);
	val=(INT32)(dYuvRgb1[1][1]*8192+0.5)&0xffff; dvC341_Write(B6_OCSC1CF11CH1,val,ucRegOffset);
	val=(INT32)(dYuvRgb1[1][2]*8192+0.5)&0xffff; dvC341_Write(B6_OCSC1CF12CH1,val,ucRegOffset);
	val=(INT32)(dYuvRgb1[2][0]*8192+0.5)&0xffff; dvC341_Write(B6_OCSC1CF20CH1,val,ucRegOffset);
	val=(INT32)(dYuvRgb1[2][1]*8192+0.5)&0xffff; dvC341_Write(B6_OCSC1CF21CH1,val,ucRegOffset);
	val=(INT32)(dYuvRgb1[2][2]*8192+0.5)&0xffff; dvC341_Write(B6_OCSC1CF22CH1,val,ucRegOffset);*/
    #endif
}

void dvC341_WAIP_init() //A70LV_Larry_0051 modify
{
#if 0      //A70LV_Doulas_0099 Add
    UINT8 ucTblIndex = m_sChannelInfo[eC341_CH_V0].ucPanelIndex;

    dvC341_Write(B30_WPBLKCT, 0x03,0);//Warping ON
	dvC341_Write(B30_WPMCT,	  0x11,0); //Warping main control

	dvC341_Write(B30_FLDDLY,    0x50,0);	//Field propagation delay
	dvC341_Write(B30_CMNSYADJ,  0xA6,0);	//synchronization adjustment

	dvC341_Write(B7_KCTRLCH1,   0x0008,0);	//Keystone correction control
	dvC341_Write(B7_KDLYCH1,    0x0049,0);	//Keystone Delay Control

    dvC341_Write(B30_OHCYCL,  m_sPanelTable[ucTblIndex].sOutputTimingInfo.uiHTotal, 0);
    dvC341_Write(B30_OVCYCL,  m_sPanelTable[ucTblIndex].sOutputTimingInfo.uiVTotal, 0);
    dvC341_Write(B30_OACTHST, m_sPanelTable[ucTblIndex].sOutputTimingInfo.uiHStart, 0);
    dvC341_Write(B30_OACTHW,  m_sPanelTable[ucTblIndex].sOutputTimingInfo.uiHActive, 0);
    dvC341_Write(B30_OACTVST, m_sPanelTable[ucTblIndex].sOutputTimingInfo.uiVStart, 0);
    dvC341_Write(B30_OACTVW,  m_sPanelTable[ucTblIndex].sOutputTimingInfo.uiVActive, 0);

    dvC341_Write(B31_IACTSEL, 0x01,0);
    dvC341_Write(B31_IACTHW, m_sPanelTable[ucTblIndex].sOutputTimingInfo.uiHActive, 0);
    dvC341_Write(B31_IACTVW, m_sPanelTable[ucTblIndex].sOutputTimingInfo.uiVActive, 0);

    dvC341_WarpLight_PanelConfig();

    LOG_MSG(db_DV_SCALER, "dvC341_WAIP_init\r\n");
#else

#endif

}

void dvC341_Set_Color(const eC341_CH_V ch_v)   //A70LV_Doulas_0002
{
	UINT32 val,icolor_space,nren;
    UINT8 ucRegOffset = 0;
    UINT32 ulRedGain;//A70LV_Doulas_0011
    UINT32 ulGreenGain;//A70LV_Doulas_0011
    UINT32 ulBlueGain;//A70LV_Doulas_0011

    if(ch_v >= eC341_CH_VNUMBER)
    {
        return;
    }

    if(m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.eScanMode == eSCAN_MODE_INTERLACE)    //A70LV_Doulas_0003
    {
        icolor_space = (m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.uiHActive > 1600) ? 1 : 0;	//0:BT-601, 1:BT-709
    }
    else
    {
	    icolor_space = (m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.uiHActive > 1024) ? 1 : 0;	//0:BT-601, 1:BT-709
    }

    if(m_sChannelSetting[ch_v].eVideoYUV != eVIDEO_YUV_UNKNOW)   //A70LV_Doulas_0109
    {
        icolor_space = m_sChannelSetting[ch_v].eVideoYUV;
    }
    LOG_MSG(db_DV_SCALER, "(func:%s, line:%d)(%d)Color(%d,%d)\r\n", __FUNCTION__, __LINE__,ch_v,icolor_space,m_sChannelSetting[ch_v].eColorFormat);

	if( m_sChannelSetting[ch_v].eColorFormat == eCOLOR_FORMAT_RGB_LIMIT )
    {
        for(UINT8 ch=0; ch<OUTPUT4K_CH_NUM; ch++)
        {
            UINT8 BankOffset = 16;
            ucRegOffset = ((ch_v * 2) + ch) * BankOffset;
    	    dvC341_WriteToBuffer(B16_IBIASRCH1,0x03c0, ucRegOffset);
    	    dvC341_WriteToBuffer(B16_IBIASGCH1,0x03c0, ucRegOffset);
    	    dvC341_WriteToBuffer(B16_IBIASBCH1,0x03c0, ucRegOffset);
        }
	}
	else if(m_sChannelSetting[ch_v].eColorFormat == eCOLOR_FORMAT_RGB)   //A70LV_Doulas_0080 modify
    {
        for(UINT8 ch=0; ch<OUTPUT4K_CH_NUM; ch++)
        {
            UINT8 BankOffset = 16;
            ucRegOffset = ((ch_v * 2) + ch) * BankOffset;
    	    dvC341_WriteToBuffer(B16_IBIASRCH1,0x0000,ucRegOffset);
    		//dvC341_WriteToBuffer(B8_IBIASGCH1,0x03c0,ucRegOffset);
            dvC341_WriteToBuffer(B16_IBIASGCH1,0x0000,ucRegOffset);   //A70LV_Doulas_0041 modify
            dvC341_WriteToBuffer(B16_IBIASBCH1,0x0000,ucRegOffset);
        }
	}
    else    // YUV/YPbPr
    {
        for(UINT8 ch=0; ch<OUTPUT4K_CH_NUM; ch++)
        {
            UINT8 BankOffset = 16;
            ucRegOffset = ((ch_v * 2) + ch) * BankOffset;
    	    dvC341_WriteToBuffer(B16_IBIASRCH1,0x0000,ucRegOffset);
    	    dvC341_WriteToBuffer(B16_IBIASGCH1,0x03c0,ucRegOffset);      //Y offset 16 ,let YUV to full range
    	    dvC341_WriteToBuffer(B16_IBIASBCH1,0x0000,ucRegOffset);
    	}
    }


///////////////////////////////////////
//Back Ground
///////////////////////////////////////
    val =  0x000000;        //A70LV_Doulas_0004
	dvC341_Write_ch_v(ch_v, B8_OFILLCH1  , val, 16);		    //Output fill
	dvC341_Write_ch_v(ch_v, B8_OBKGD1CH1 , val, 16);		//Output background color 1
	if(m_sChannelInfo[ch_v].sInputTimingInfo.eScanMode == eSCAN_MODE_NO_SIGNAL)	// b0:OFILLEN
        m_sChannelInfo[ch_v].ucOIMGCT |= 0x01;
    else
        m_sChannelInfo[ch_v].ucOIMGCT &= 0xFE;
  //  m_sChannelInfo[ch_v].ucOIMGCT |= 0x80;   //A70LV_Doulas_0004
    if(dvC341_Read(B8_OIMGCTCH1, (ch_v * 2) * CH_BANK_OFFSET) != m_sChannelInfo[ch_v].ucOIMGCT)
	    dvC341_Write_ch_v(ch_v, B8_OIMGCTCH1, m_sChannelInfo[ch_v].ucOIMGCT, CH_BANK_OFFSET);		//Output image control


///////////////////////////////////////
//Chromaticity
///////////////////////////////////////
	//val= ( m_sChannelSetting[ch_v].eColorFormat == eCOLOR_FORMAT_RGB_LIMIT ) ?
    //            ((int)( 598*((double)(100)/100.0))) :
    //            ((int)( 512*((double)(100)/100.0)));  //A70LV_Doulas_0003
	val= ( m_sChannelSetting[ch_v].eColorFormat == eCOLOR_FORMAT_RGB ) ?
                ((int)( 512*((double)(100)/100.0))) :
                ((int)( 597*((double)(100)/100.0)));        //A70LV_Doulas_0080 modify,let YUV to full range

	if(val > 0x07FF){		// Overflow limit
		val = 0x07FF;
	}
	val = val & 0x07FF;
    #if 0
    if(m_sChannelSetting[ch_v].eColorFormat == eCOLOR_FORMAT_RGB || m_sChannelSetting[ch_v].eColorFormat == eCOLOR_FORMAT_RGB_LIMIT)    //A70LV_Doulas_0080 test
    {
	    dvC341_WriteToBuffer(B8_IGAINRCH1,val   ,ucRegOffset);	//Input Red gain correction
        dvC341_WriteToBuffer(B8_IGAINGCH1,val   ,ucRegOffset);  //Input Green gain correction
	    dvC341_WriteToBuffer(B8_IGAINBCH1,val   ,ucRegOffset);  //Input Blue gain correction
    }
    else
    {
	    dvC341_WriteToBuffer(B8_IGAINRCH1,512   ,ucRegOffset);	//Input Red gain correction
        dvC341_WriteToBuffer(B8_IGAINGCH1,val   ,ucRegOffset);  //Input Green gain correction
	    dvC341_WriteToBuffer(B8_IGAINBCH1,512   ,ucRegOffset);  //Input Blue gain correction
    }
    #else

    for(UINT8 ch=0; ch<OUTPUT4K_CH_NUM; ch++)
    {
        UINT8 BankOffset = 16;
        ucRegOffset = ((ch_v * 2) + ch) * BankOffset;

        dvC341_WriteToBuffer(B16_IGAINRCH1,val   ,ucRegOffset);	//Input Red gain correction
        //if ( m_sChannelSetting[ch_v].eColorFormat!=4 )
        //    dvC341_WriteToBuffer(B8_IGAINGCH1,0x0255,ucRegOffset);     //Input Green gain correction
        //else
        //    dvC341_WriteToBuffer(B8_IGAINGCH1,0x0200,ucRegOffset);
        dvC341_WriteToBuffer(B16_IGAINGCH1,val   ,ucRegOffset);      //A70LV_Doulas_0003
        dvC341_WriteToBuffer(B16_IGAINBCH1,val   ,ucRegOffset);		//Input Blue gain correction
    }
	#endif

///////////////////////////////////////
//Brightness                                //A70LV_Doulas_0011 modify
///////////////////////////////////////
    for(UINT8 ch=0; ch<OUTPUT4K_CH_NUM; ch++)
    {
        UINT8 BankOffset = 16;
        ucRegOffset = ((ch_v * 2) + ch) * BankOffset;

        dvC341_WriteToBuffer(B12_BGCTCH1 ,0x01,ucRegOffset);
    	dvC341_WriteToBuffer(B12_BIASRCH1,0,ucRegOffset);	//Output bias correction
    	dvC341_WriteToBuffer(B12_BIASGCH1,(m_sChannelSetting[ch_v].iBrightness)&0x0fff,ucRegOffset);
    	dvC341_WriteToBuffer(B12_BIASBCH1,0,ucRegOffset);
    }

///////////////////////////////////////
//Contrast                                  //A70LV_Doulas_0011 modify
///////////////////////////////////////
	val = ((int)( 512*(m_sChannelSetting[ch_v].dContrast)));        //A70LV_Doulas_0003
	if (val > 0x07FF)
    {       // Overflow limit
		val = 0x07FF;
	}
	val = val & 0x07FF;

    for(UINT8 ch=0; ch<OUTPUT4K_CH_NUM; ch++)
    {
        UINT8 BankOffset = 16;
        ucRegOffset = ((ch_v * 2) + ch) * BankOffset;

    	dvC341_WriteToBuffer(B12_OGAIN1RCH1,val  ,ucRegOffset);				//Output gain correction
    	dvC341_WriteToBuffer(B12_OGAIN1GCH1,val  ,ucRegOffset);
    	dvC341_WriteToBuffer(B12_OGAIN1BCH1,val  ,ucRegOffset);
    }
///////////////////////////////////////
//RGB Gain                                  //A70LV_Doulas_0011 Add
///////////////////////////////////////
	ulRedGain = ((int)( 512*(m_sChannelSetting[ch_v].dRedGain)));
	if (ulRedGain > 0x07FF)
    {       // Overflow limit
		ulRedGain = 0x07FF;
	}
	ulRedGain = ulRedGain & 0x07FF;

	ulGreenGain = ((int)( 512*(m_sChannelSetting[ch_v].dGreenGain)));
	if (ulGreenGain > 0x07FF)
    {       // Overflow limit
		ulGreenGain = 0x07FF;
	}
	ulGreenGain = ulGreenGain & 0x07FF;

	ulBlueGain = ((int)( 512*(m_sChannelSetting[ch_v].dBlueGain)));
	if (ulBlueGain > 0x07FF)
    {       // Overflow limit
		ulBlueGain = 0x07FF;
	}
	ulBlueGain = ulBlueGain & 0x07FF;

    for(UINT8 ch=0; ch<OUTPUT4K_CH_NUM; ch++)
    {
        UINT8 BankOffset = 16;
        ucRegOffset = ((ch_v * 2) + ch) * BankOffset;

        dvC341_WriteToBuffer(B12_BGCT2CH1,		0x01    ,ucRegOffset) ;
        dvC341_WriteToBuffer(B12_OGAIN2RCH1,ulRedGain   ,ucRegOffset);				//Output gain correction
        dvC341_WriteToBuffer(B12_OGAIN2GCH1,ulGreenGain ,ucRegOffset);
        dvC341_WriteToBuffer(B12_OGAIN2BCH1,ulBlueGain  ,ucRegOffset);
    }
///////////////////////////////////////
//RGB Offset                                  //A70LV_Doulas_0011 Add
///////////////////////////////////////

    for(UINT8 ch=0; ch<OUTPUT4K_CH_NUM; ch++)
    {
        UINT8 BankOffset = 16;
        ucRegOffset = ((ch_v * 2) + ch) * BankOffset;

        dvC341_WriteToBuffer(B12_OBIAS2RCH1,(m_sChannelSetting[ch_v].iRedOffset)&0x0fff,ucRegOffset);	//Output bias correction
        dvC341_WriteToBuffer(B12_OBIAS2GCH1,(m_sChannelSetting[ch_v].iGreenOffset)&0x0fff,ucRegOffset);
        dvC341_WriteToBuffer(B12_OBIAS2BCH1,(m_sChannelSetting[ch_v].iBlueOffset)&0x0fff,ucRegOffset);
    }
///////////////////////////////////////
//RGB->YUV control, ceof defined in set_fix_reg()
///////////////////////////////////////
//	printf("set_color:PM_IFORMAT[%d]=%d isel=%d\n",ch,PM_IFORMAT[ch],isel);

	if((m_sChannelSetting[ch_v].eColorFormat==eCOLOR_FORMAT_RGB_LIMIT )|(m_sChannelSetting[ch_v].eColorFormat==eCOLOR_FORMAT_RGB))
	    val=(BIT4)|(BIT3);							// RGB (IMOD=0 & RGB->YUV enb)
	#ifndef ENABLE_IT6805_YUV422_YUV420_CONVERT_TO_YUV444
	else if (m_sChannelSetting[ch_v].eColorFormat==eCOLOR_FORMAT_422)
	    val=(BIT3)|(BIT0);							//YUV422 (IMOD=1)
	else if (m_sChannelSetting[ch_v].eColorFormat==eCOLOR_FORMAT_422_UVDLY)
	    val=(BIT3)|(BIT0);   //A70LV_Doulas_0003
	#else

	#if 0  //don`t care 3G-SDI
	else if((m_sChannelSetting[ch_v].eInputSource == eCM_SOURCE_3GSDI)  &&
			(m_sChannelInfo[eC341_CH_V0].ePanelTimingId != ePANEL_ID_1080P_120HZ) &&
			(m_sChannelInfo[eC341_CH_V0].ePanelTimingId != ePANEL_ID_WUXGA_120HZ))		//A70Gen2_Doulas_0030 Modify
	{
		val=(BIT3)|(BIT0);  //A70Gen2_John_0002 fix 3GSDI in YUV444 issue (only support YUV422)  //Simon check
	}
	#endif
	else if (m_sChannelSetting[ch_v].eColorFormat==eCOLOR_FORMAT_422)
	    val=(BIT3)|(BIT1);							//YUV422 (IMOD=1)  //Simon C341 YUV444
	else if (m_sChannelSetting[ch_v].eColorFormat==eCOLOR_FORMAT_422_UVDLY)
	    val=(BIT3)|(BIT1);   //A70LV_Doulas_0003    //Simon C341 YUV444
	#endif
	else if (m_sChannelSetting[ch_v].eColorFormat==eCOLOR_FORMAT_444)
	    val=(BIT3)|(BIT1);		                    //YUV444 (IMOD=2)  //Simon C341 YUV444

    m_sChannelInfo[ch_v].ucICFMT = val;  //A70LV_Doulas_0076 Add
	if(ch_v == 0)
    {
		dvC341_WriteToBuffer(B0_ICFMTCH1,val,0);				//Input color format control
		dvC341_WriteToBuffer(B0_ICFMTCH2,val,0);				//Input color format control
	}
	else
    {
		dvC341_WriteToBuffer(B0_ICFMTCH3,val,0);
		dvC341_WriteToBuffer(B0_ICFMTCH4,val,0);
	}
//	printf("set_color:B0_ICFMT[%d]=%02x\n",ch,(0x34|val));

    #if 0   //A70LV_Doulas_0003
	nren =(ucPM_HNR[ch_v]!=0) | (ucPM_VNR[ch_v]!=0) |
		  (ucPM_TNR[ch_v]!=0) | (ucPM_BNR[ch_v]!=0) | (ucPM_MNR[ch_v]!=0);
    #else
    nren = 0;
    #endif

	//val =  (  m_sChannelSetting[ch_v].eColorFormat==eCOLOR_FORMAT_RGB_LIMIT ) ? 0xc8 : 0x48;// IGAINEN(0x08)//LPFEN
	val =  (  m_sChannelSetting[ch_v].eColorFormat==eCOLOR_FORMAT_RGB_LIMIT ) ? 0x88 : 0x08;     //A70LV_Doulas_0080 modify,data not ok

#ifndef ENABLE_IT6805_YUV422_YUV420_CONVERT_TO_YUV444
	if ( m_sChannelSetting[ch_v].eColorFormat == eCOLOR_FORMAT_422 )
	{                                       //YUV422
	    val = val|0x03;													// UVBND[1]=ENB / UVDLY[0]=ENB
	    if((m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.eScanMode == eSCAN_MODE_INTERLACE)&&
	            (m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.uiVActive == 480 ||
	                    m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.uiVActive == 576))   //A70LV_Doulas_0076 modify //A70LV_Doulas_0003
	    {                                   //YUV422 & (IMOD == 480i)
	        val = val&0xFE;												// UVDLY[0]=DIS
	    }
	}
#else  //A70Gen2_John_0002 fix 3GSDI in YUV444 issue (only support YUV422)
    #if 0  //don`t care 3G-SDI
	if((m_sChannelSetting[ch_v].eInputSource == eCM_SOURCE_3GSDI) &&
		(m_sChannelInfo[eC341_CH_V0].ePanelTimingId != ePANEL_ID_1080P_120HZ) &&
		(m_sChannelInfo[eC341_CH_V0].ePanelTimingId != ePANEL_ID_WUXGA_120HZ))		//A70Gen2_Doulas_0030 Modify
	{
        val = val|0x03;                                                 // UVBND[1]=ENB / UVDLY[0]=ENB
        if((m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.eScanMode == eSCAN_MODE_INTERLACE)&&
                (m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.uiVActive == 480 ||
                        m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.uiVActive == 576))   //A70LV_Doulas_0076 modify //A70LV_Doulas_0003
        {                                   //YUV422 & (IMOD == 480i)
            val = val&0xFE;                                             // UVDLY[0]=DIS
        }
	}
	#endif
#endif

	//if (( nren == 1 || PI_SCAN_MODE[ch]==1) && (PM_IFORMAT[ch] != 1)){	// (Deinter or NR=EN) && IFMT != YUV422
	//	val = val | 0x40;													// UVLPF[6]=ENB
	//}
	#ifndef ENABLE_IT6805_YUV422_YUV420_CONVERT_TO_YUV444
	if(((nren == 1)||(m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.eScanMode == eSCAN_MODE_INTERLACE)) &&
       (m_sChannelSetting[ch_v].eColorFormat != eCOLOR_FORMAT_422 ))    //A70LV_Doulas_0003
	#else
	if(((nren == 1)||(m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.eScanMode == eSCAN_MODE_INTERLACE)) &&
	   (//(m_sChannelSetting[ch_v].eInputSource != eCM_SOURCE_3GSDI) ||   //don`t care 3G-SDI
	    (m_sChannelInfo[eC341_CH_V0].ePanelTimingId == ePANEL_ID_1080P_120HZ) ||
	    (m_sChannelInfo[eC341_CH_V0].ePanelTimingId == ePANEL_ID_WUXGA_120HZ)))  //A70Gen2_Doulas_0030 Modify//A70Gen2_John_0002 fix 3GSDI in YUV444 issue (only support YUV422)
	#endif
    {

        val = val | (BIT6);   //enable low pass filter
    }


	dvC341_Write_ch_v(ch_v, B16_ICOLORCTCH1, val, CH_BANK_OFFSET);		//Input image control

///////////////////////////////////////
//HUE
///////////////////////////////////////
//	printf("set_color:ICOLORCT[%d]=%02x\n",ch,val);

#if 1   //Doulas_3000
	dvC341_Set_Color_Matrix(ch_v,icolor_space,0,0,m_sChannelSetting[ch_v].iHue,m_sChannelSetting[ch_v].dColor);    //A70LV_Doulas_0003 modify
#endif

///////////////////////////////////////
// Skin color
///////////////////////////////////////
    dvC341_Set_Color_Correction(ch_v,0,0,m_sChannelSetting[ch_v].ucSkinColor,2);      //EK816U_626U_Doulas_0004

///////////////////////////////////////
// GAMMMA
///////////////////////////////////////
//gamma 1 or 2
//	val = PM_GAMMA[ch];
//	dvC341_Write(B6_GMBNKSEL11CH1,val,ch);	//Gamma 1 table select
//	dvC341_Write(B6_GMBNKSEL21CH1,0,ch);		//Gamma table 2 control

	return;
}

BOOL dvC341Interlace_measure(UINT16 H_ACTIVE,UINT16 V_ACTIVE)   //A70LV_Doulas_0002
{
    BOOL bInterlace = FALSE;
    if((H_ACTIVE == 1440) &&(V_ACTIVE == 288))
        bInterlace = TRUE;
    else if((H_ACTIVE == 720) &&(V_ACTIVE == 288))
         bInterlace = TRUE;
    else if((H_ACTIVE == 1440) &&(V_ACTIVE == 240))
         bInterlace = TRUE;
    else if((H_ACTIVE == 720) &&(V_ACTIVE == 240))
         bInterlace = TRUE;
    else if((H_ACTIVE == 1920) &&(V_ACTIVE == 540))
         bInterlace = TRUE;

    return bInterlace;
}

void dvC341_calc_clrm(
	double utmp, double vtmp,
	double UC, double VC,
	double UP, double VP,
	double C, double D, double DOM, double GAIN,int coveron,
	double *u, double *v)       //A70LV_Doulas_0003
{
	double distance, Dr, sinA, cosA, sinB, cosB, A, B;
	double COVER, utmpnew, vtmpnew, MARUME=0;

	distance = sqrt(pow((utmp-UC),2)+pow((vtmp-VC),2));

	if(distance <= DOM/4.0 )
    {
		Dr = pow(distance,2)/(pow(DOM/4.0,2)/GAIN);
	}
    else if(distance <= DOM/2.0)
	{
		Dr = pow((distance-(DOM/2.0)),2)/(pow(DOM/4.0,2)/GAIN);
	}
    else
    {
		Dr = 0;
	}

	sinA = ((utmp==UC)&(vtmp==VC)) ? 0.0
		 : (utmp-UC)/sqrt(pow((utmp-UC),2)+pow((vtmp-VC),2));

	cosA = ((utmp==UC)&(vtmp==VC)) ? 0.0
		 : (vtmp-VC)/sqrt(pow((utmp-UC),2)+pow((vtmp-VC),2));

	sinB = ((UP==UC)&(VP==VC))? 0.0
		 : (UP-UC)/sqrt(pow((UP-UC),2)+pow((VP-VC),2));							//UP:peak

	cosB = ((UP==UC)&(VP==VC)) ? 0.0
		 : (VP-VC)/sqrt(pow((UP-UC),2)+pow((VP-VC),2));							//VP:peak

	A = (sinA>=0) ? acos(cosA):((3.14159*2)-acos(cosA));

	B = (sinB>=0) ? acos(cosB):((3.14159*2)-acos(cosB));

	if((0 <= B) && (B < (3.14159/C)))       //calc cover
    {
	    if((0 <= A) && (A < B))
			COVER = (1+cos((B-A)*C))/2;											//(1+cos((B-A)*C))/2
	    else if((B <= A) && (A < (B+(3.14159/D))))
			COVER = (1+cos((B-A)*D))/2;											//(1+cos((B-A)*D))/2
        else if(((B+((3.14159*2)-(3.14159/C))) <= A) && (A < (3.14159*2)))
			COVER = (1+cos((B-A)*C))/2;											//(1+cos((B-A)*C))/2
	    else
			COVER = 0;
	}
    else if(((3.14159/C) <= B) && (B < ((3.14159*2)-(3.14159/D))))
	{
		if(((B-(3.14159/C)) <= A) && (A < B))
			COVER = (1+cos((B-A)*C))/2;											//(1+cos((B-A)*C))/2
		else if((B <= A) && (A < ((B+(3.14159/D)))))
			COVER = (1+cos((B-A)*D))/2;											//(1+cos((B-A)*D))/2
		else
			COVER = 0;
	}
    else if((((3.14159*2)-(3.14159/D)) <= B) && (B < (3.14159*2)))
    {
		if((0 <= A) && (A < (B-((3.14159*2)-(3.14159/D)))))
			COVER = (1+cos((B-A)*D))/2;											//(1+cos((B-A)*D))/2
		else if(((B-(3.14159/C)) <= A) && (A < B))
			COVER = (1+cos((B-A)*C))/2;											//(1+cos((B-A)*C))/2
		else if((B <= A) && (A < (3.14159*2)))
			COVER = (1+cos((B-A)*D))/2;											//(1+cos((B-A)*D))/2
		else
			COVER =0;
	}
    else
    {
		COVER = 0;
	}

	utmpnew = utmp - (Dr*sinA)*COVER + MARUME;
	vtmpnew = vtmp - (Dr*cosA)*COVER + MARUME;

	if(utmpnew > 1023.0 )
		utmpnew = 1023.0;
	else if (utmp < 0.0)
		utmpnew = 0.0;
	else
		;  //utmpnew = utmpnew;   //G100_Simon_0060 avoid Cppcheck warning

	if(vtmpnew > 1023.0 )
		vtmpnew = 1023.0;
	else if (vtmpnew < 0.0)
		vtmpnew = 0.0;
	else
		;  //vtmpnew = vtmpnew;   //G100_Simon_0060 avoid Cppcheck warning

	*u = utmpnew;
	*v = vtmpnew;

	return;
}

void dvC341_Set_Color_Correction(eC341_CH_V ch_v,	 //A70LV_Doulas_0003
	INT32 blgain,		//blue gain
	INT32 grgain,		//green gain
	INT32 flgain,		//fleshness(skin) gain
	UINT8 mode		//0..blue/1..green/2..flesh/3..all
)
{
	INT32 val;
	double utmp,vtmp,utmpnew2,vtmpnew2;
	double uc[3],vc[3],gain[3],dom[3],c[3],d[3],up[3],vp[3];
	INT32 coveron[3],gainon[3];
	INT32 u_shift,v_shift,uv_threth;
	double uv_mlti;
	UINT8 clrm_flg;

	INT32 kmin,kmax,lmin,lmax;

    INT8 iPM_CNT_CLRM[eC341_CH_VNUMBER] = {0,0};

    UINT8 BankOffset = CH_BANK_OFFSET;

    if(ch_v >= eC341_CH_VNUMBER)
    {
        return;
    }

    UINT8 *pcBuffer = malloc((64*3) + 2);
    if(pcBuffer == NULL)
    {
        LOG_MSG(db_ALWAYS,"\n%s(%d) malloc fail\n", __FUNCTION__ , __LINE__);
        return;
    }

    UINT16 wCount = 0;

	if((blgain == 0)&&(grgain==0)&&(flgain == 0))
    {
        dvC341_Write_ch_v(ch_v, B12_FCGCTCH1, 0x02, BankOffset);		//Output full color gamma control register		// CPUACC=Enb/FCG=Dis

		clrm_flg =0;
	}
	else
    {
		clrm_flg =1;
	}

	if(flgain != 0)
    { /*Flesh init value*/
		dom[0] =87*4;
		uc[0] =114*4;
		vc[0] =172*4;
		c[0] =1;
		d[0] =1;
		up[0] =114*4;
		vp[0] =0;
		gain[0] =9*4+flgain*10;
		coveron[0] =1;
		gainon[0] =0;
	}
    else
	{
		dom[0] =0;
		uc[0] =0;
		vc[0] =0;
		c[0] =0;
		d[0] =0;
		up[0] =0;
		vp[0] =0;
		gain[0] =0;
		coveron[0] =0;
		gainon[0] =0;
	}

	if(blgain !=0)
    { /*Blue init value*/
		dom[1] =162*4;
		uc[1] =193*4;
		vc[1] =88*4;
		c[1] =9;
		d[1] =2;
		up[1] =0;
		vp[1] =88*4;
		gain[1] =16*4+blgain*20;
		coveron[1] =1;
		gainon[1] =0;
	}
    else
    {
		dom[1] =0;
		uc[1] =0;
		vc[1] =0;
		c[1] =0;
		d[1] =0;
		up[1] =0;
		vp[1] =0;
		gain[1] =0;
		coveron[1] =0;
		gainon[1] =0;
	}

	if(grgain != 0)
    { /*Green init value*/
		dom[2] =129*4;
		uc[2] =82*4;
		vc[2] =75*4;
		c[2] =4;
        d[2] =4;
		up[2] =255*4;
		vp[2] =255*4;
		gain[2] =16*4+grgain*20;
		coveron[2] =1;
		gainon[2] =0;
	}
    else
	{
		dom[2] =0;
		uc[2] =0;
		vc[2] =0;
		c[2] =0;
        d[2] =0;
		up[2] =0;
		vp[2] =0;
		gain[2] =0;
		coveron[2] =0;
		gainon[2] =0;
	}

	u_shift = 0x03f;
	v_shift = 0xfc0;
	uv_threth = 6;
	uv_mlti = 16.0;

	if(mode == 0)
    {           // blue
		kmin =29;
		kmax =49;
		lmin =18;
		lmax =43;
	}
	else if(mode == 1)
    {       // green
		kmin =21;
		kmax =37;
		lmin =19;
		lmax =35;
	}
	else if(mode == 2)
    {       // flesh
		kmin =18;
		kmax =40;
		lmin =33;
		lmax =54;
	}
	else
    {                   // init(all)
		kmin =0;
		kmax =64;
		lmin =0;
		lmax =64;
	}

	if(clrm_flg==0)
    {
		dvC341_Write_ch_v(ch_v, B12_FCGCTCH1, 0x06, BankOffset);// CPU Access Enable			// YCMN=Enb/CPUACC=Enb/FCG=Dis
	}
    else
	{
		dvC341_Write_ch_v(ch_v, B12_FCGCTCH1, 0x07, BankOffset);// CPU Access Enable			// YCMN=Enb/CPUACC=Enb/FCG=Enb
	}
    //gainon[0] = gainon[0];   //G100_Simon_0060 avoid Cppcheck warning

	if(iPM_CNT_CLRM[ch_v] == 0)
    {
		iPM_CNT_CLRM[ch_v]=lmax-lmin;
	}

	for(INT32 l=(lmax-iPM_CNT_CLRM[ch_v]); l<lmax ;l++)
    {
        wCount = 0;

		for(INT32 k=kmin;k<kmax;k++)
        {
			INT32		i;
			i =l*64 + k;
			if(k == kmin)
            {
                UINT8 cData[2] = {0};

                cData[0] = (i&0xFF);
                cData[1] = ((i >> 8)&0xFF);

                dvC341_BurstWrite_FixedAdd((B12_FCGADCH1 + (UINT32)((ch_v * BankOffset * 2)<<16)), 2, cData); //A35G2_CDS_Larry_0030
                dvC341_BurstWrite_FixedAdd((B28_FCGADCH2 + (UINT32)((ch_v * BankOffset * 2)<<16)), 2, cData);
				//dvC341_WriteToBuffer(B12_FCGADCH1,(i&0xFF)       ,ucRegOffset);  //B6_FCGAD[7:0]  //Output full color gamma address
				//dvC341_WriteToBuffer(B12_FCGADCH1,((i >> 8)&0xFF),ucRegOffset);  //B6_FCGAD[11:8]
			}

			utmp = (double)(i&u_shift);
			vtmp = (double)((i&v_shift)>>uv_threth);
			utmp = utmp * uv_mlti;
			vtmp = vtmp * uv_mlti;
			utmpnew2=utmp;
			vtmpnew2=vtmp;

			for(INT32 j=0;j<3;j++)
            {
				dvC341_calc_clrm( utmpnew2,vtmpnew2,uc[j],vc[j],up[j],vp[j],c[j],d[j],dom[j],gain[j],coveron[j],&utmp,&vtmp);
				utmpnew2 = utmp;
				vtmpnew2 = vtmp;
			}

#if 0
			val = (INT32)(((unsigned int)utmpnew2)&0x0ff);								//	U[7:0]
			dvC341_WriteToBuffer(B12_FCGDTCH1,val,ucRegOffset);							//Output full color gamma data
			val = ((((INT32)utmpnew2&0x300)>>8) + (((INT32)vtmpnew2&0x03f)<<2));	//	{V[5:0],U[9:8]}
			dvC341_WriteToBuffer(B12_FCGDTCH1,val,ucRegOffset);							//Output full color gamma data
			val = (((INT32)vtmpnew2&0x3c0)>>6);									//	{4'd0,V[9:6]}
			dvC341_WriteToBuffer(B12_FCGDTCH1,val,ucRegOffset);							//Output full color gamma data
#endif /* 0 */


            pcBuffer[wCount++] = (INT32)(((unsigned int)utmpnew2)&0x0ff);
            pcBuffer[wCount++] = ((((INT32)utmpnew2&0x300)>>8) + (((INT32)vtmpnew2&0x03f)<<2));    //  {V[5:0],U[9:8]}
            pcBuffer[wCount++] = (((INT32)vtmpnew2&0x3c0)>>6);
		}

        dvC341_BurstWrite_FixedAdd((B12_FCGDTCH1 + (UINT32)((ch_v * BankOffset * 2)<<16)), wCount, pcBuffer); //A35G2_CDS_Larry_0030
        dvC341_BurstWrite_FixedAdd((B28_FCGDTCH2 + (UINT32)((ch_v * BankOffset * 2)<<16)), wCount, pcBuffer); //A35G2_CDS_Larry_0030

		iPM_CNT_CLRM[ch_v]--;
		/*if(iPM_CNT_CLRM[ch_v] != 0 && mode != 3)   //A70LV_Doulas_0003 remove test
        {
			goto fin;
		}*/
	}

    free(pcBuffer);
//fin:  //A70LV_Doulas_0008 remove

	if(clrm_flg==0)
    {
		dvC341_Write_ch_v(ch_v, B12_FCGCTCH1, 0x04, BankOffset);// CPU Access Disable			// YCMN=Enb/CPUACC=Enb/FCG=Dis
	}
    else
	{
		dvC341_Write_ch_v(ch_v, B12_FCGCTCH1, 0x05, BankOffset);// CPU Access Disable			// YCMN=Enb/CPUACC=Enb/FCG=Enb
	}

    //dvC341_Buffer_Flush();
}

void dvC341_Set_Sharpness(const eC341_CH_V ch_v)  //A70LV_Doulas_0003
{
	INT8 val, i;
	double PL_SCALE = 1.0;
	UINT8 coefz[5];
	UINT8 gain;      //H30K_Doulas_0080
    INT16 wVal;      //ZU860_Doulas_0011
    UINT8 BankOffset = 16;

	// 9symbol(for HEDGE)
	UINT8 coef9[][6]= {	{0x20,0x06,0x0a,0x00,0x00,0x00},//-10(0)		// Soft
						{0x20,0x08,0x08,0x00,0x00,0x00},// -9
						{0x24,0x07,0x07,0x00,0x00,0x00},// -8
						{0x28,0x06,0x06,0x00,0x00,0x00},// -7
						{0x2c,0x05,0x05,0x00,0x00,0x00},// -6
						{0x30,0x04,0x04,0x00,0x00,0x00},// -5
						{0x34,0x03,0x03,0x00,0x00,0x00},// -4
						{0x38,0x03,0x01,0x00,0x00,0x00},// -3
						{0x3a,0x03,0x00,0x00,0x00,0x00},// -2
						{0x3c,0x02,0x00,0x00,0x00,0x00},// -1
						{0x00,0x00,0x00,0x00,0x00,0x00},//  0(10)
						{0x40,0x38,0x38,0x38,0x38,   4},//  1			// Noused
						{0x40,0x38,0x38,0x38,0x38,   8},//  2			// Noused
						{0x40,0x38,0x38,0x38,0x38,  12},//  3			// Noused
						{0x40,0x38,0x38,0x38,0x38,  20},//  4			// Noused
						{0x40,0x38,0x38,0x38,0x38,  28},//  5			// Noused
						{0x40,0x38,0x38,0x38,0x38,  38},//  6			// Noused
						{0x40,0x38,0x38,0x38,0x38,  48},//  7			// Noused
						{0x40,0x38,0x38,0x38,0x38,  60},//  8			// Noused
						{0x40,0x38,0x38,0x38,0x38,  72},//  9			// Noused
						{0x40,0x38,0x38,0x38,0x38,  92} // 10(20)		// Sharp
	};

	// 5symbol(for VEDGE)
	UINT8 coef5[][6]= {	{0x20,0x06,0x0a,0x00,0x00,0x00},//-10(0)		// Soft
						{0x20,0x08,0x08,0x00,0x00,0x00},// -9
						{0x24,0x07,0x07,0x00,0x00,0x00},// -8
						{0x28,0x06,0x06,0x00,0x00,0x00},// -7
						{0x2c,0x05,0x05,0x00,0x00,0x00},// -6
						{0x30,0x04,0x04,0x00,0x00,0x00},// -5
						{0x34,0x03,0x03,0x00,0x00,0x00},// -4
						{0x38,0x03,0x01,0x00,0x00,0x00},// -3
						{0x3a,0x03,0x00,0x00,0x00,0x00},// -2
						{0x3c,0x02,0x00,0x00,0x00,0x00},// -1
						{0x00,0x00,0x00,0x00,0x00,0x00},//  0(10)
						{0x40,0x20,0x00,0x00,0x00,   4},//  1			// Noused
						{0x40,0x20,0x00,0x00,0x00,   8},//  2			// Noused
						{0x40,0x20,0x00,0x00,0x00,  12},//  3			// Noused
						{0x40,0x20,0x00,0x00,0x00,  20},//  4			// Noused
						{0x40,0x20,0x00,0x00,0x00,  28},//  5			// Noused
						{0x40,0x20,0x00,0x00,0x00,  38},//  6			// Noused
						{0x40,0x20,0x00,0x00,0x00,  48},//  7			// Noused
						{0x40,0x20,0x00,0x00,0x00,  60},//  8			// Noused
						{0x40,0x20,0x00,0x00,0x00,  72},//  9			// Noused
						{0x40,0x20,0x00,0x00,0x00,  92} // 10(20)		// Sharp
	};
    UINT8   ucRegOffset = 0;

    if(ch_v >= eC341_CH_VNUMBER)
    {
        return;
    }


#if 1   //ZU860_Doulas_0011 modify
    if(m_sChannelSetting[ch_v].ucSharpness >= SHARPNESS_MIN_VALUE && m_sChannelSetting[ch_v].ucSharpness <= SHARPNESS_MAX_VALUE)
    {
        wVal = (INT16)(m_sChannelSetting[ch_v].ucSharpness - SHARPNESS_DEFAULT_VALUE)* 20 /(INT16)(SHARPNESS_MAX_VALUE - SHARPNESS_MIN_VALUE);
    }
    else
    {
        wVal = 0;
    }
    cPM_HSHARP[ch_v] = (INT8)wVal;
    cPM_VSHARP[ch_v] = (INT8)wVal;
#else
    switch(m_sChannelSetting[ch_v].ucSharpness)  //A70LV_Doulas_0029 modify
    {
        case e_SHARPNESS_MAXIMUN:
            cPM_HSHARP[ch_v] = 7;
            cPM_VSHARP[ch_v] = 7;
            break;

        case e_SHARPNESS_HIGH:
            cPM_HSHARP[ch_v] = 3;
            cPM_VSHARP[ch_v] = 3;
            break;

        default:
        case e_SHARPNESS_NORMAL:
            cPM_HSHARP[ch_v] = 0;
            cPM_VSHARP[ch_v] = 0;
            break;

        case e_SHARPNESS_LOW:
            cPM_HSHARP[ch_v] = -3;
            cPM_VSHARP[ch_v] = -3;
            break;

        case e_SHARPNESS_MINIMUM:
            cPM_HSHARP[ch_v] = -7;
            cPM_VSHARP[ch_v] = -7;
            break;
    }
#endif
	dvC341_set_rtct_stop();
/******* HEDGE *******/
	if ( cPM_HSHARP[ch_v]==0 )
    {
        ucRegOffset = (ch_v * 2) * BankOffset;
		dvC341_WriteToBuffer(B11_HEGCTCH1,0x00,ucRegOffset);
		dvC341_WriteToBuffer(B27_HEGCTCH2,0x00,ucRegOffset);
	}
    else if ( cPM_HSHARP[ch_v]<0 )
    {                                   //softness(LowPass mode)
        ucRegOffset = (ch_v * 2) * BankOffset;
		dvC341_WriteToBuffer(B11_HEGCTCH1,0x05,ucRegOffset); //H30K_Doulas_0080
		dvC341_WriteToBuffer(B27_HEGCTCH2,0x05,ucRegOffset); //H30K_Doulas_0080
	}
    else
    {                                                       // Edge Enh.(YUV)
        ucRegOffset = (ch_v * 2) * BankOffset;
		dvC341_WriteToBuffer(B11_HEGCTCH1,0x06,ucRegOffset); //H30K_Doulas_0080
		dvC341_WriteToBuffer(B27_HEGCTCH2,0x06,ucRegOffset); //H30K_Doulas_0080
	}

	val= (cPM_HSHARP[ch_v]>10) ? 19 : (cPM_HSHARP[ch_v]+10);

	PL_SCALE = (double)(((float)m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiHActive)/((float)m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiHActive));

	if( cPM_HSHARP[ch_v] <= 0)
    {                                       // Soft
		for(i=0;i<5;i++){
			coefz[i] = coef9[val][i];
		}
	}
	else if(PL_SCALE > 2.5)
    {
		coefz[0] = 0x24;
		coefz[1] = 0x2b;
		coefz[2] = 0x3a;
		coefz[3] = 0x04;
		coefz[4] = 0x05;
	}
	else if(PL_SCALE > 2.0)
    {
		coefz[0] = 0x1e;
		coefz[1] = 0x2a;
		coefz[2] = 0x00;
		coefz[3] = 0x07;
		coefz[4] = 0x00;
	}
	else if(PL_SCALE > 1.25)
    {
		coefz[0] = 0x1c;
		coefz[1] = 0x2e;
		coefz[2] = 0x05;
		coefz[3] = 0x03;
		coefz[4] = 0x3c;
	}
	else
    {
		coefz[0] = 0x2c;
		coefz[1] = 0x31;
		coefz[2] = 0x38;
		coefz[3] = 0x3f;
		coefz[4] = 0x02;
	}

	gain = 128;//48/10; //H30K_Doulas_0080
	if(cPM_HSHARP[ch_v] > 0)
    {
		gain = (UINT8)((INT16)gain * (INT16)cPM_HSHARP[ch_v] / (INT16)10); //H30K_Doulas_0080
	}
	else
    {
		gain = 0x00;
	}

    for(UINT8 ch=0; ch<OUTPUT4K_CH_NUM; ch++)
    {
        ucRegOffset = ((ch_v * 2) + ch) * BankOffset;
    	dvC341_WriteToBuffer(B11_HEGCOEF0CH1,coefz[0],ucRegOffset);
    	dvC341_WriteToBuffer(B11_HEGCOEF1CH1,coefz[1],ucRegOffset);
    	dvC341_WriteToBuffer(B11_HEGCOEF2CH1,coefz[2],ucRegOffset);
    	dvC341_WriteToBuffer(B11_HEGCOEF3CH1,coefz[3],ucRegOffset);
    	dvC341_WriteToBuffer(B11_HEGCOEF4CH1,coefz[4],ucRegOffset);
    	dvC341_WriteToBuffer(B11_HEGGAINCH1 ,(UINT32)gain    ,ucRegOffset);
    }


/******* VEDGE *******/
	if ( cPM_VSHARP[ch_v]==0 )
    {
        ucRegOffset = (ch_v * 2) * BankOffset;
		dvC341_WriteToBuffer(B11_VEGCTCH1,0x00,ucRegOffset);
		dvC341_WriteToBuffer(B27_VEGCTCH2,0x00,ucRegOffset);
	}
    else if ( cPM_VSHARP[ch_v]<0 )
    {                                   //softness(LowPass mode)
        ucRegOffset = (ch_v * 2) * BankOffset;
		dvC341_WriteToBuffer(B11_VEGCTCH1,0x05,ucRegOffset); //H30K_Doulas_0080
		dvC341_WriteToBuffer(B27_VEGCTCH2,0x05,ucRegOffset); //H30K_Doulas_0080
	}
    else
    {                                                       // Edge Enh.(YUV)
        ucRegOffset = (ch_v * 2) * BankOffset;
		dvC341_WriteToBuffer(B11_VEGCTCH1,0x06,ucRegOffset); //H30K_Doulas_0080
		dvC341_WriteToBuffer(B27_VEGCTCH2,0x06,ucRegOffset); //H30K_Doulas_0080
	}
	val= (cPM_VSHARP[ch_v]>10) ? 19 : cPM_VSHARP[ch_v]+10;

	PL_SCALE = (double)(((float)m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiVActive) / ((float)m_sChannelInfo[ch_v].sScaler_InputTimingInfo.uiVActive));
																	// PI_SCAN_MODE[ch]=0...Progressive 1...Interlace

	if( cPM_VSHARP[ch_v] <= 0)
    {                                       // Soft
		for(i=0;i<5;i++){
			coefz[i] = coef5[val][i];
		}
	}
	else if(PL_SCALE > 2.5)
    {
		coefz[0] = 0x2a;
		coefz[1] = 0x30;
		coefz[2] = 0x3b;
		coefz[3] = 0x00;
		coefz[4] = 0x00;
	}
	else if(PL_SCALE > 2.0)
    {
		coefz[0] = 0x22;
		coefz[1] = 0x2f;
		coefz[2] = 0x00;
		coefz[3] = 0x00;
		coefz[4] = 0x00;
	}
	else if(PL_SCALE > 1.25)
    {
		coefz[0] = 0x1c;
		coefz[1] = 0x2d;
		coefz[2] = 0x05;
		coefz[3] = 0x00;
		coefz[4] = 0x00;
	}
	else{
		coefz[0] = 0x2c;
		coefz[1] = 0x32;
		coefz[2] = 0x38;
		coefz[3] = 0x00;
		coefz[4] = 0x00;
	}
	gain = 128;//48/10; //H30K_Doulas_0080
	if(cPM_VSHARP[ch_v] > 0)
    {
		gain = (UINT8)((INT16)gain * (INT16)cPM_VSHARP[ch_v] / (INT16)10); //H30K_Doulas_0080
	}
	else
    {
		gain = 0x00;
	}

    for(UINT8 ch=0; ch<OUTPUT4K_CH_NUM; ch++)
    {
        ucRegOffset = ((ch_v * 2) + ch) * BankOffset;
		dvC341_WriteToBuffer(B11_VEGSELCH1,0x01, ucRegOffset);
    	dvC341_WriteToBuffer(B11_VEGCOEF0CH1,coefz[0],ucRegOffset);
    	dvC341_WriteToBuffer(B11_VEGCOEF1CH1,coefz[1],ucRegOffset);
    	dvC341_WriteToBuffer(B11_VEGCOEF2CH1,coefz[2],ucRegOffset);
    	dvC341_WriteToBuffer(B11_VEGCOEF3CH1,coefz[3],ucRegOffset);
    	dvC341_WriteToBuffer(B11_VEGCOEF4CH1,coefz[4],ucRegOffset);
    	dvC341_WriteToBuffer(B11_VEGGAINCH1 ,(UINT32)gain    ,ucRegOffset);
    }

    dvC341_Buffer_Flush();
	dvC341_set_rtct_normal();
}
//A70LV_Doulas_2000 end

void dvC341_Config_NoSignalOutput(const eC341_CH_V ch_v, UINT8 ucDisplayOutput)   //A70LV_Doulas_0142 //A70LV_Doulas_0004  //A35G2_CDS_Simon_0017
{
    if((m_sChannelSetting[eC341_CH_V0].cPIP_PBP_ON == TRUE) &&
    //   (ch_v == eMCT_CH2) &&   //A70LV_Doulas_0142 remove
       (ucNo_Signal_Config[ch_v] == 0))
    {
        if(ucDisplayOutput)    //A35G2_CDS_Simon_0017
        {
            m_sChannelInfo[ch_v].ucMCT |= 0x01;
            m_sChannelInfo[ch_v].ucOIMGCT |= 0x01;

            for(UINT8 ch=0; ch<OUTPUT4K_CH_NUM; ch++)
            {
                UINT16 uwRegOffset = ((ch_v * 2) + ch) * 0x100;
                UINT8 ucRegOffset = ((ch_v * 2) + ch) * CH_BANK_OFFSET;
                dvC341_WriteToBuffer(B10_HZOOMCTCH1, 0,     ucRegOffset);
                dvC341_WriteToBuffer(B10_HZINITEVCH1, 0,    ucRegOffset);
                dvC341_WriteToBuffer(B10_HZINITODCH1, 0,    ucRegOffset);
                dvC341_WriteToBuffer(B10_HZSCLCH1, 0,       ucRegOffset);
                dvC341_WriteToBuffer(B10_VZOOMCTCH1, 0,     ucRegOffset);
                dvC341_WriteToBuffer(B10_VZINITEVCH1, 0,    ucRegOffset);
                dvC341_WriteToBuffer(B10_VZINITODCH1, 0,    ucRegOffset);
                dvC341_WriteToBuffer(B10_VZSCLCH1, 0,       ucRegOffset);


                dvC341_WriteToBuffer(B8_OACTHSTCH1, (UINT32)m_sC341PanelInfo[ch_v].uiHStart, ucRegOffset);
                dvC341_WriteToBuffer(B8_OACTHWCH1,  (UINT32)m_sC341PanelInfo[ch_v].uiHActive, ucRegOffset);
                dvC341_WriteToBuffer(B8_OACTVSTCH1, (UINT32)m_sC341PanelInfo[ch_v].uiVStart, ucRegOffset);
                dvC341_WriteToBuffer(B8_OACTVWCH1,  (UINT32)m_sC341PanelInfo[ch_v].uiVActive, ucRegOffset);


                //blank screen
                if(dvC341_Read(B8_OIMGCTCH1, ucRegOffset) != m_sChannelInfo[ch_v].ucOIMGCT)
                    dvC341_WriteToBuffer(B8_OIMGCTCH1,  (UINT32)m_sChannelInfo[ch_v].ucOIMGCT, ucRegOffset);
                //ucRegOffset = ((ch_v * 2) + ch) * 0x100;
                dvC341_WriteToBuffer(B0_MCT1 + uwRegOffset,  (UINT32)m_sChannelInfo[ch_v].ucMCT, 0);
            }
            dvC341_Buffer_Flush();
        }
        else
        {
            dvC341_SetInputPort_Output_Off(ch_v);
        }

        ucNo_Signal_Config[ch_v] == 1;
    }
    return;
}

void dvC341_HV_Sync_MCLK_Count_Get(const eC341_CH_V ch_v)   //A70LV_Doulas_0004
{
    if(ch_v == eC341_CH_V0)
    {
        IMFH_MCLK_COUNT[ch_v] = dvC341_Read(B168_IMFHCH1, 0);
        dvC341_SpecialTiming_Check_1280_960_85(ch_v, &IMFH_MCLK_COUNT[ch_v]);
        IMFV_MCLK_COUNT[ch_v] = dvC341_Read(B168_IMFVCH1, 0);
    }
    else if(ch_v == eC341_CH_V1)
    {
        IMFH_MCLK_COUNT[ch_v] = dvC341_Read(B168_IMFHCH3, 0);
        dvC341_SpecialTiming_Check_1280_960_85(ch_v, &IMFH_MCLK_COUNT[ch_v]);
        IMFV_MCLK_COUNT[ch_v] = dvC341_Read(B168_IMFVCH3, 0);
    }
}

BOOL dvC341_SYNC_LOCK_Compare(const eC341_CH_V ch_v)   //A70LV_Doulas_0005 modify //A70LV_Doulas_0004
{
    UINT32 ucIMFH = 0,ucIMFV = 0;
    DOUBLE V_Total,Vsync_Freq;

    #if 0
    if(ch_v == eC341_CH_V0)
    {
        ucIMFH = dvC341_Read(B168_IMFHCH1, 0);
        dvC341_SpecialTiming_Check_1280_960_85(ch_v, &ucIMFH);
        ucIMFV = dvC341_Read(B168_IMFVCH1, 0);
    }
    else if(ch_v == eC341_CH_V1)
    {
        ucIMFH = dvC341_Read(B168_IMFHCH3, 0);
        dvC341_SpecialTiming_Check_1280_960_85(ch_v, &ucIMFH);
        ucIMFV = dvC341_Read(B168_IMFVCH3, 0);
    }
    #else
    UINT8 ucInputPort = m_sChannelInfo[ch_v].ucInputPort ;
    int port_offset = ucInputPort - 2;  //for VBO

    ucIMFH = dvC341_Read(B174_VI11FHM + 0x800*port_offset, 0);
    dvC341_SpecialTiming_Check_1280_960_85(ch_v, &ucIMFH);
    ucIMFV = dvC341_Read(B174_VI11FVM + 0xc00*port_offset, 0);
    #endif

    LOG_MSG(db_DV_SCALER, "--ch_v : %d-- H:(%x,%x), V:(%x,%x)\r\n",ch_v,IMFH_MCLK_COUNT[ch_v],ucIMFH,IMFV_MCLK_COUNT[ch_v],ucIMFV);
    //LOG_MSG(db_DV_SCALER, "### %d \r\n",dvC341_Read(B172_IFACTHSTCH1 + 0x1000*ch_v, 0) + 7); //H30K_Doulas_0002 debug
    if((ucIMFH == 0xFFFF) || (ucIMFH == 0xFFFFFF))  //no sync
    {
        return FALSE;
    }

    if((ucIMFH < IMFH_MCLK_COUNT[ch_v] - IMFH_MCLK_TOLERANCE) ||
       (ucIMFH > IMFH_MCLK_COUNT[ch_v] + IMFH_MCLK_TOLERANCE) ||
       (ucIMFV < IMFV_MCLK_COUNT[ch_v] - IMFV_MCLK_TOLERANCE) ||
       (ucIMFV > IMFV_MCLK_COUNT[ch_v] + IMFV_MCLK_TOLERANCE) )   //not lock  //A70LV_Doulas_0009 modify not lock checking
    {
        LOG_MSG(db_DV_SCALER, "##112##(%x,%x)(%x,%x)\r\n",IMFH_MCLK_COUNT[ch_v],ucIMFH,IMFV_MCLK_COUNT[ch_v],ucIMFV );   //A70LV_Doulas_0124
        return FALSE;
    }

    if((IMFH_MCLK_COUNT[ch_v] == 0) || (IMFV_MCLK_COUNT[ch_v] == 0))   //A70LV_Doulas_0005 invalid value
    {
        LOG_MSG(db_DV_SCALER, "(func:%s, line:%d)CH= %d(%08Xh,%08Xh)\r\n", __FUNCTION__, __LINE__,ch_v,IMFH_MCLK_COUNT[ch_v],IMFV_MCLK_COUNT[ch_v]);    //A70LV_Doulas_0124
        return FALSE;
    }

    Vsync_Freq = (DOUBLE)(m_sChannelInfo[ch_v].ulFMCLK) / (DOUBLE)IMFV_MCLK_COUNT[ch_v];
    V_Total = (DOUBLE)IMFV_MCLK_COUNT[ch_v] / (DOUBLE)IMFH_MCLK_COUNT[ch_v];
    if((Vsync_Freq < 20) || (Vsync_Freq > 245) ||       //H30K_Doulas_0009 modify
       (V_Total < 100) || (V_Total > 3000))  //invalid timing
    {
        LOG_MSG(db_DV_SCALER, "(func:%s, line:%d)CH= %d(%08Xh,%08Xh)(%f,%f)\r\n", __FUNCTION__, __LINE__,ch_v,IMFH_MCLK_COUNT[ch_v],IMFV_MCLK_COUNT[ch_v],Vsync_Freq,V_Total);    //A70LV_Doulas_0124
        return FALSE;
    }

    return TRUE;
}

BOOL dvC341_HV_Sync_Measure(const eC341_CH_V ch_v)   //A70LV_Doulas_0005
{
    DOUBLE V_Total,Vsync_Freq;

    dvC341_HV_Sync_MCLK_Count_Get(ch_v);     //A70LV_Doulas_0007
    if((IMFH_MCLK_COUNT[ch_v] == 0xFFFF) || (IMFV_MCLK_COUNT[ch_v] == 0xFFFFFF))  //no sync
    {
        LOG_MSG(db_DV_SCALER, "(func:%s, line:%d)CH= %d(%08Xh,%08Xh)\r\n", __FUNCTION__, __LINE__,ch_v,IMFH_MCLK_COUNT[ch_v],IMFV_MCLK_COUNT[ch_v]);    //A70LV_Doulas_0112
        return FALSE;
    }

    if((IMFH_MCLK_COUNT[ch_v] == 0) || (IMFV_MCLK_COUNT[ch_v] == 0))  //invalid
    {
        LOG_MSG(db_DV_SCALER, "(func:%s, line:%d)CH= %d(%08Xh,%08Xh)\r\n", __FUNCTION__, __LINE__,ch_v,IMFH_MCLK_COUNT[ch_v],IMFV_MCLK_COUNT[ch_v]);    //A70LV_Doulas_0112
        return FALSE;
    }

    Vsync_Freq = (DOUBLE)(m_sChannelInfo[ch_v].ulFMCLK) / (DOUBLE)IMFV_MCLK_COUNT[ch_v];
    V_Total = (DOUBLE)IMFV_MCLK_COUNT[ch_v] / (DOUBLE)IMFH_MCLK_COUNT[ch_v];
    if((Vsync_Freq < 20) || (Vsync_Freq > 130) ||
       (V_Total < 100) || (V_Total > 3000))  //invalid timing
    {
        LOG_MSG(db_DV_SCALER, "(func:%s, line:%d)CH= %d(%08Xh,%08Xh)(%f,%f)\r\n", __FUNCTION__, __LINE__,ch_v,IMFH_MCLK_COUNT[ch_v],IMFV_MCLK_COUNT[ch_v],Vsync_Freq,V_Total);    //A70LV_Doulas_0112
        return FALSE;
    }

    return TRUE;
}

//A35G2_CDS_Simon_0041
BOOL dvC341_InputPort_Sync_MCLK_Count_Get(const eC341_CH_V ch_v, INT32 *lIMFH , INT32 *lIMFV)
{
    if((m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive == 0) || (m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive == 0))
    {
        dvC341_CheckInput_4K(ch_v);
    }

    if(ch_v == eC341_CH_V0)
    {
        *lIMFH = dvC341_Read(B168_IMFHCH1, 0);
        dvC341_SpecialTiming_Check_1280_960_85(ch_v, lIMFH);
        *lIMFV = dvC341_Read(B168_IMFVCH1, 0);
    }
    else if(ch_v == eC341_CH_V1)
    {
        *lIMFH = dvC341_Read(B168_IMFHCH3, 0);
        dvC341_SpecialTiming_Check_1280_960_85(ch_v, lIMFH);
        *lIMFV = dvC341_Read(B168_IMFVCH3, 0);
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}

static void dvC341_SetMemAD_2K(eC341_CH_V ch_v, PsACTIVE_AREA psOact)
{
    UINT32 ulOSFLD[5];//4 Bank
    UINT32 ulISFLD[5];//4 Bank
    UINT8  ucRegOffset = 0;
    UINT8  ucIPRegOffset = 0;
    //UINT8 ucOIMGCT2 = 0x01;

    if(ch_v >= eC341_CH_VNUMBER)
    {
        return;
    }

    {
        dvC341_set_scfmfmt(ch_v, FRAME_MEMORY_FORMAT);   //H2PF_Simon_0131
#if 0
        //dvC341_Write(B1_SCWCTCH1, 0x00, ucRegOffset);
        if(ch_v == eC341_CH_V0)
        {
            dvC341_Write(B0_OSCWCTCH1, 0x00, 0);
            dvC341_Write(B0_ISCWCTCH1, 0x00, 0);
            dvC341_Write(B0_OSCWCTCH2, 0x00, 0);
            dvC341_Write(B0_ISCWCTCH2, 0x00, 0);
        }
        else
        {
            dvC341_Write(B0_OSCWCTCH3, 0x00, 0);
            dvC341_Write(B0_ISCWCTCH3, 0x00, 0);
            dvC341_Write(B0_OSCWCTCH4, 0x00, 0);
            dvC341_Write(B0_ISCWCTCH4, 0x00, 0);
        }
#endif /* 0 */

        //dvC341_Write(B2_OIMGCT2CH1, ucOIMGCT2, ucRegOffset);
        //dvC341_Write(B2_OIMGCT2CH1, ucOIMGCT2, ucRegOffset);
        dvC341_Write_ch_v(ch_v, B20_ACTMSKCTCH1, 0x00, CH_BANK_OFFSET);
    }

    if(ch_v == eC341_CH_V0)
    {
        ulISFLD[0] = DEF_ISFLD0CH1_4K;
        ulISFLD[1] = DEF_ISFLD1CH1_4K;
        ulISFLD[2] = DEF_ISFLD2CH1_4K;
        ulISFLD[3] = DEF_ISFLD3CH1_4K;
        ulISFLD[4] = DEF_ISFLD4CH1_4K;

        ulOSFLD[0] = DEF_ISFLD4CH1_4K;// + (psOact->iHOffset * 4) + (psOact->iVOffset * DEF_MWI_2K * 4 * 128);  //A70LV_Doulas_0108
        ulOSFLD[1] = DEF_ISFLD0CH1_4K;// + (psOact->iHOffset * 4) + (psOact->iVOffset * DEF_MWI_2K * 4 * 128);
        ulOSFLD[2] = DEF_ISFLD1CH1_4K;// + (psOact->iHOffset * 4) + (psOact->iVOffset * DEF_MWI_2K * 4 * 128);
        ulOSFLD[3] = DEF_ISFLD2CH1_4K;// + (psOact->iHOffset * 4) + (psOact->iVOffset * DEF_MWI_2K * 4 * 128);
        ulOSFLD[4] = DEF_ISFLD3CH1_4K;// + (psOact->iHOffset * 4) + (psOact->iVOffset * DEF_MWI_2K * 4 * 128);
    }
    else if(ch_v == eC341_CH_V1)
    {
        ulISFLD[0] = DEF_ISFLD0CH2_4K;
        ulISFLD[1] = DEF_ISFLD1CH2_4K;
        ulISFLD[2] = DEF_ISFLD2CH2_4K;
        ulISFLD[3] = DEF_ISFLD3CH2_4K;
        ulISFLD[4] = DEF_ISFLD4CH2_4K;

        ulOSFLD[0] = DEF_ISFLD4CH2_4K;// + (psOact->iHOffset * 4) + (psOact->iVOffset * DEF_MWI_2K * 128);      //A70LV_Doulas_0108
        ulOSFLD[1] = DEF_ISFLD0CH2_4K;// + (psOact->iHOffset * 4) + (psOact->iVOffset * DEF_MWI_2K * 128);
        ulOSFLD[2] = DEF_ISFLD1CH2_4K;// + (psOact->iHOffset * 4) + (psOact->iVOffset * DEF_MWI_2K * 128);
        ulOSFLD[3] = DEF_ISFLD2CH2_4K;// + (psOact->iHOffset * 4) + (psOact->iVOffset * DEF_MWI_2K * 128);
        ulOSFLD[4] = DEF_ISFLD3CH2_4K;// + (psOact->iHOffset * 4) + (psOact->iVOffset * DEF_MWI_2K * 128);
    }

    for(UINT8 ch=0; ch<OUTPUT4K_CH_NUM; ch++)
    {
        UINT8 BankOffset = CH_BANK_OFFSET;
        ucRegOffset = ((ch_v * 2) + ch) * BankOffset;

        dvC341_Write(B22_ISFLD0CH1, ulISFLD[0], ucRegOffset);
        dvC341_Write(B22_ISFLD1CH1, ulISFLD[1], ucRegOffset);
        dvC341_Write(B22_ISFLD2CH1, ulISFLD[2], ucRegOffset);
        dvC341_Write(B22_ISFLD3CH1, ulISFLD[3], ucRegOffset);
        dvC341_Write(B22_ISFLD4CH1, ulISFLD[4], ucRegOffset);

        dvC341_Write(B22_OSFLD0CH1, ulOSFLD[0], ucRegOffset);
        dvC341_Write(B22_OSFLD1CH1, ulOSFLD[1], ucRegOffset);
        dvC341_Write(B22_OSFLD2CH1, ulOSFLD[2], ucRegOffset);
        dvC341_Write(B22_OSFLD3CH1, ulOSFLD[3], ucRegOffset);
        dvC341_Write(B22_OSFLD4CH1, ulOSFLD[4], ucRegOffset);
    }

    dvC341_Write_ch_v(ch_v, B22_MRTCTCH1, 0x00, CH_BANK_OFFSET);
}

#if 0
static void dvC341_SetShrink(const eC341_CH_V eCH, const UINT16 uiHsscl, const UINT16 uiVsscl)
{
    DOUBLE dScl;
    UINT8 ucSelCoef = 0;
    UINT8 ucSelLut = 0;
    UINT8 ucRegOffset = 0;

    if(eCH >= eC341_CH_VNUMBER)
    {
        return;
    }


    ucRegOffset = (eCH == eMCT_CH1) ? 0 : 13;

    // hshrink
    if(uiHsscl == 0x0000)
    {
        dvC341_Write(B9_HSHRNKCTCH1, 0x00, ucRegOffset);
        dvC341_Write(B9_HSSCLCH1, uiHsscl, ucRegOffset);
    }
    else
    {
        dvC341_Write(B9_HSHRNKCTCH1, 0x47, ucRegOffset);
        dvC341_Write(B9_HSSCLCH1, uiHsscl, ucRegOffset);

        dScl = (DOUBLE)uiHsscl / 65536;
        ucSelCoef =  (dScl<0.3) ? 0
                    :(dScl<0.4) ? 2
                    :(dScl<0.5) ? 4
                    :(dScl<0.6) ? 6
                    :(dScl<0.7) ? 8
                    :(dScl<0.8) ? 12
                    :(dScl<0.9) ? 22
                    :(dScl<1.0) ? 33
                    :46;

        ucSelLut = (eCH == eMCT_CH1) ? 0x83
                   : 0x87;

        dvC341_Write(B0_SELCOEFSET, ucSelCoef, 0);
        dvC341_Write(B0_SELLUT, ucSelLut, 0);
    }

    // vshrink
    if(uiVsscl == 0x0000)
    {
        dvC341_Write(B9_VSHRNKCTCH1, 0x00, ucRegOffset);
        dvC341_Write(B9_VSSCLCH1, uiVsscl, ucRegOffset);
    }
    else
    {
        dvC341_Write(B9_VSHRNKCTCH1, 0x47, ucRegOffset);
        dvC341_Write(B9_VSSCLCH1, uiVsscl, ucRegOffset);

        dScl = (DOUBLE)uiVsscl / 65536;
        ucSelCoef =  (dScl<0.3) ? 0
                    :(dScl<0.4) ? 2
                    :(dScl<0.5) ? 4
                    :(dScl<0.6) ? 6
                    :(dScl<0.7) ? 8
                    :(dScl<0.8) ? 12
                    :(dScl<0.9) ? 22
                    :(dScl<1.0) ? 33
                    :46;

        ucSelLut = (eCH == eMCT_CH1) ? 0x82
                   : 0x86;

        dvC341_Write(B0_SELCOEFSET, ucSelCoef, 0);
        dvC341_Write(B0_SELLUT, ucSelLut, 0);
    }
}
#endif

void dvC341_SetIpConv(const eC341_CH_V ch_v, const BOOL bEnable)
{
    UINT8 ucRegOffset = 0;
  //  UINT8 ucRegIpOffset = 0;
    DOUBLE dMclk = 0;// MHz
    DOUBLE dPiclk = 0;// MHz
    DOUBLE dPoclk = 0;// MHz
    BOOL   bIsSD[eC341_CH_VNUMBER] = {FALSE,FALSE};
    UINT16 uiIpHTotal = 0;
 //   UINT8 ucIpOactVw_ofst = 0;  //A70LV_Doulas_0008 remove
    UINT8 ucNren[eC341_CH_VNUMBER];    //A70LV_Doulas_0003

    if(ch_v >= eC341_CH_VNUMBER)
    {
        return;
    }

    if(bEnable == FALSE)
    {
        m_sChannelInfo[ch_v].ucMCT &= 0xF1;
        m_sChannelInfo[ch_v].ucICFMT &= 0x7F;

        //LOG_MSG(db_APP_DATAPATH, "<2-%d> %d\n", ch_v, m_sChannelInfo[ch_v].ucMCT);

        switch(ch_v)
        {
            case eC341_CH_V0:
                if(dvC341_Read(B0_MCT1,0) != m_sChannelInfo[ch_v].ucMCT)
                    dvC341_WriteToBuffer(B0_MCT1, m_sChannelInfo[ch_v].ucMCT, 0);
                if(dvC341_Read(B0_MCT2,0) != m_sChannelInfo[ch_v].ucMCT)
                    dvC341_WriteToBuffer(B0_MCT2, m_sChannelInfo[ch_v].ucMCT, 0);
                dvC341_WriteToBuffer(B0_ICFMTCH1, m_sChannelInfo[ch_v].ucICFMT, 0);
                dvC341_WriteToBuffer(B0_ICFMTCH2, m_sChannelInfo[ch_v].ucICFMT, 0);
                break;

            case eC341_CH_V1:
                if(dvC341_Read(B0_MCT3,0) != m_sChannelInfo[ch_v].ucMCT)
                    dvC341_WriteToBuffer(B0_MCT3, m_sChannelInfo[ch_v].ucMCT, 0);
                if(dvC341_Read(B0_MCT4,0) != m_sChannelInfo[ch_v].ucMCT)
                    dvC341_WriteToBuffer(B0_MCT4, m_sChannelInfo[ch_v].ucMCT, 0);
                dvC341_WriteToBuffer(B0_ICFMTCH3, m_sChannelInfo[ch_v].ucICFMT, 0);
                dvC341_WriteToBuffer(B0_ICFMTCH4, m_sChannelInfo[ch_v].ucICFMT, 0);
                break;

            default:
                break;
        }
       // dvC341_Write(B11_MVNRCTCH1, 0x00, ucRegOffset);   //A70LV_Doulas_0002

        dvC341_Buffer_Flush();
        return;
    }

    #if 0   //A70LV_Doulas_0003
	ucNren[0] =(ucPM_HNR[0]!=0) | (ucPM_VNR[0]!=0) |
			 (ucPM_TNR[0]!=0) | (ucPM_BNR[0]!=0) | (ucPM_MNR[0]!=0);
	ucNren[1] =(ucPM_HNR[1]!=0) | (ucPM_VNR[1]!=0) |
			 (ucPM_TNR[1]!=0) | (ucPM_BNR[1]!=0) | (ucPM_MNR[1]!=0);
    #else
	ucNren[0] = 0;
	ucNren[1] = 0;
    #endif


#if (CURRENT_MAIN_BOARD == CORE_BOARD)  //A70LV_Doulas_0002
    //CH1
    dPiclk = (DOUBLE)m_sChannelInfo[eC341_CH_V0].sInputTimingInfo.uiHTotal *
             (DOUBLE)m_sChannelInfo[eC341_CH_V0].sInputTimingInfo.uiVTotal *
             (DOUBLE)m_sChannelInfo[eC341_CH_V0].sInputTimingInfo.dFrameRate / 1000000 ;
   // if(m_sChannelInfo[eMCT_CH1].sInputTimingInfo.eScanMode == eSCAN_MODE_INTERLACE)  //需要在確認
   //     dPiclk /= 2;      //A70LV_Doulas_0002
    bIsSD[0] = (dPiclk < 20) ? TRUE : FALSE;
    dMclk = dPiclk;

    //CH2
    dPiclk = (DOUBLE)m_sChannelInfo[eC341_CH_V1].sInputTimingInfo.uiHTotal *
             (DOUBLE)m_sChannelInfo[eC341_CH_V1].sInputTimingInfo.uiVTotal *
             (DOUBLE)m_sChannelInfo[eC341_CH_V1].sInputTimingInfo.dFrameRate / 1000000 ;
   // if(m_sChannelInfo[eMCT_CH2].sInputTimingInfo.eScanMode == eSCAN_MODE_INTERLACE) //需要在確認
   //     dPiclk /= 2;      //A70LV_Doulas_0002
    bIsSD[1] = (dPiclk < 20) ? TRUE : FALSE;
    if(ch_v == eC341_CH_V0)
        dPiclk = dMclk;

    dPoclk = (DOUBLE)m_sChannelInfo[ch_v].sOutputTimingInfo.uiPixClk/100;

#else
    dMclk = m_sChannelInfo[ch_v].ulMclko_Freq;//330000000;
    dMclk = (dMclk/1000000)/2; //DDR

 //   dPiclk = (DOUBLE)m_sChannelInfo[ch_v].sInputTimingInfo.uiHTotal/
 //               (DOUBLE)m_sChannelInfo[ch_v].sInputTimingInfo.uiHTotal_MCLK * dMclk; //A70LV_Doulas_0005 remove

    dPoclk = (DOUBLE)m_sChannelInfo[ch_v].sOutputTimingInfo.uiPixClk/100;

    bIsSD[ch_v] = (dPiclk < 20) ? TRUE : FALSE;
#endif

    //畫面錯亂時,參數需要調整
    //dvC341_Write(B0_IPREFDIV, 0x04, 0);
    //dvC341_Write(B0_IPFBDIV, 0x88, 0);

    if((m_sChannelInfo[0].sInputTimingInfo.eScanMode == eSCAN_MODE_INTERLACE) ||
       (m_sChannelInfo[1].sInputTimingInfo.eScanMode == eSCAN_MODE_INTERLACE) ||
       ucNren[0] || ucNren[1])      //A70LV_Doulas_0003
        m_sChannelInfo[0].ucIPPLLCT = 0x09;
    else
        m_sChannelInfo[0].ucIPPLLCT = 0x00;

    if(bIsSD[0])
        m_sChannelInfo[0].ucIPPLLCT |= 0x10;
    if(bIsSD[1])
        m_sChannelInfo[0].ucIPPLLCT |= 0x20;

    //dvC341_WriteToBuffer(B0_IPPLLCT, m_sChannelInfo[0].ucIPPLLCT, 0);   //畫面錯亂時,參數需要調整    //simon check

    //ucIpOactVw_ofst = 0;//4;  //需要確認 //A70LV_Doulas_0008 remove

    //dvC341_Write(B11_IPOACTHWCH1, m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.uiHActive, ucRegOffset);     //A70LV_Doulas_0003 remove
    //dvC341_Write(B11_IPOACTVSTCH1, (0x8000 + (m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.uiVStart - 1) * 2), ucRegOffset);    //A70LV_Doulas_0003 remove
    //dvC341_Write(B11_IPOACTVWCH1, (m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.uiVActive + ucIpOactVw_ofst), ucRegOffset);     //A70LV_Doulas_0003 remove

    if(bIsSD[ch_v])
    {
        dPiclk = dPiclk * 2;

        if(dPoclk > 148) // 1080P Output
        {
            uiIpHTotal = (UINT16)(((DOUBLE)m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.uiHTotal) * dPoclk / (dPiclk * 4) * 1.21 + 1); //H30K_Doulas_0034
        }
        else
        {
            uiIpHTotal = (UINT16)(((DOUBLE)m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.uiHTotal) * dPoclk / (dPiclk) * 1.1 + 1);
        }
    }
    else
    {
        if(dPoclk > 148) // 1080P Output
        {
            uiIpHTotal = (UINT16)(((DOUBLE)m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.uiHTotal) * dPoclk / (dPiclk * 4) * 1.21 + 1); //H30K_Doulas_0034
        }
        else
        {
            uiIpHTotal = (UINT16)(((DOUBLE)m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.uiHTotal) * dPoclk / (dPiclk) * 1.1 + 1);
        }
    }
    LOG_MSG(db_DV_SCALER, "dPoclk =%f,dPiclk=%f(%f,%f)(%d,%d,%f)\r\n", dPoclk, dPiclk,
                (DOUBLE)m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.uiHTotal,(DOUBLE)uiIpHTotal,
                m_sChannelInfo[ch_v].sInputTimingInfo.uiHTotal,
                m_sChannelInfo[ch_v].sInputTimingInfo.uiVTotal,
                m_sChannelInfo[ch_v].sInputTimingInfo.dFrameRate);   //A70LV_Doulas_0002


    //uiIpHTotal = (dvC341_Read(B168_IFHCH1 + 0xc00*ch_v, 0) + 1+1)/2 +32 ; //H30K_Doulas_0034//H30K_Doulas_0001 new rule
    dvC341_Write_ch_v(ch_v, B20_IPOHCYCLCH1, (uiIpHTotal & 0x3FFF), CH_BANK_OFFSET);
    uiIpHTotal = uiIpHTotal * 3 / 4 + 1;
    dvC341_Write_ch_v(ch_v, B20_IPHSYRDLYCH1, (uiIpHTotal & 0x3FFF) | 0x8000, CH_BANK_OFFSET);

    //LOG_MSG(db_APP_DATAPATH, "<3>%d\n", m_sChannelInfo[ch_v].ucMCT);

    switch(ch_v)
    {
        case eC341_CH_V0:
            m_sChannelInfo[ch_v].ucMCT |= 0x0E;
            if(dvC341_Read(B0_MCT1,0) != m_sChannelInfo[ch_v].ucMCT)
                dvC341_WriteToBuffer(B0_MCT1, m_sChannelInfo[ch_v].ucMCT, 0);
            if(dvC341_Read(B0_MCT2,0) != m_sChannelInfo[ch_v].ucMCT)
                dvC341_WriteToBuffer(B0_MCT2, m_sChannelInfo[ch_v].ucMCT, 0);
            m_sChannelInfo[ch_v].ucICFMT |= 0x80;
            dvC341_WriteToBuffer(B0_ICFMTCH1, m_sChannelInfo[ch_v].ucICFMT, 0);
            dvC341_WriteToBuffer(B0_ICFMTCH2, m_sChannelInfo[ch_v].ucICFMT, 0);
            break;

        case eC341_CH_V1:
            m_sChannelInfo[ch_v].ucMCT |= 0x0E;
            if(dvC341_Read(B0_MCT3,0) != m_sChannelInfo[ch_v].ucMCT)
                dvC341_WriteToBuffer(B0_MCT3, m_sChannelInfo[ch_v].ucMCT, 0);
            if(dvC341_Read(B0_MCT4,0) != m_sChannelInfo[ch_v].ucMCT)
                dvC341_WriteToBuffer(B0_MCT4, m_sChannelInfo[ch_v].ucMCT, 0);
            m_sChannelInfo[ch_v].ucICFMT |= 0x80;
            dvC341_WriteToBuffer(B0_ICFMTCH3, m_sChannelInfo[ch_v].ucICFMT, 0);
            dvC341_WriteToBuffer(B0_ICFMTCH4, m_sChannelInfo[ch_v].ucICFMT, 0);
            break;

        default:
            break;
    }

    //dvC341_Set_Interlace_Regs(ch_v);   //A70LV_Doulas_0003 remove
    //dvC341_Set_Movement_NR(ch_v);      //A70LV_Doulas_0003 remove

    dvC341_Write_ch_v(ch_v, B20_PDSCLMDCH1,  0x00, CH_BANK_OFFSET);
    dvC341_Write_ch_v(ch_v, B20_ACTMSKCTCH1, 0x00, CH_BANK_OFFSET);

    dvC341_Buffer_Flush();
}


///////////////////////////////////////////////////////////////////////////////////
///@brief dvC341_SetInputPort_2K : set input port
///@param UINT8 ucCH - scaler channel. Don not care in the 4K Panel
///@return void -
///@author Roger Chen
///////////////////////////////////////////////////////////////////////////////////
void dvC341_SetInputPort_2K(eC341_CH_V ch_v)
{
    if(ch_v >= eC341_CH_VNUMBER)
    {
        return;
    }

    if(!m_sChannelInfo[ch_v].bInit)
    {
        return;
    }

    INT32 nren = 0;

    m_sChannelInfo[ch_v].ucMCT = (m_sChannelInfo[ch_v].sInputTimingInfo.eScanMode == eSCAN_MODE_INTERLACE)|nren ? 0x1F : 0x11;
    m_sChannelInfo[ch_v].ucMCT = (m_sChannelSetting[ch_v].cFREEZE == TRUE) ? m_sChannelInfo[ch_v].ucMCT&0xFD : m_sChannelInfo[ch_v].ucMCT;     //A70LV_Doulas_0030 //A70LV_Doulas_0004 Add  //simon C341 : wait review

    //LOG_MSG(db_APP_DATAPATH, "<4-%d> %d\n", eCH, m_sChannelInfo[eCH].ucMCT);

    switch(ch_v)
    {
        case eC341_CH_V0:
            if(dvC341_Read(B0_MCT1,0) != m_sChannelInfo[ch_v].ucMCT)
                dvC341_Write(B0_MCT1,  m_sChannelInfo[ch_v].ucMCT, 0);
            if((m_sChannelInfo[eC341_CH_V0].ePanelTimingId == ePANEL_ID_1080P_120HZ) ||
               (m_sChannelInfo[eC341_CH_V0].ePanelTimingId == ePANEL_ID_WUXGA_120HZ))  //H30K_Doulas_0002 //H30K_Doulas_0001
            {
                UINT16 uiH_Active = m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive;
                UINT16 uiV_Active = m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive;
                uiH_Active *= g_vboinfo[g_vbo_recno[m_sChannelInfo[ch_v].ucInputPort]].CH;
                if(dvC341_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_FRAME_SEQUENTIAL) // 100/120hz
                {
                    if(g_vbo_recno[m_sChannelInfo[ch_v].ucInputPort] == 4) //H30K_Doulas_0029 8-lane
                    {
                        dvC341_Write(B0_MCT2,  m_sChannelInfo[ch_v].ucMCT, 0);
                    }
                    else
                    {
                        dvC341_Write(B0_MCT2,  m_sChannelInfo[ch_v].ucMCT & 0xEF, 0);
                    }
                }
                else if((dvC341_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_OFF) &&
                        (uiV_Active <= 1200))
                {
                    if(m_sChannelInfo[ch_v].sInputTimingInfo.eScanMode == eSCAN_MODE_INTERLACE) //H30K_Doulas_0005
                    {
                        dvC341_Write(B0_MCT2,  0x01, 0); //H30K_Doulas_0034 Modify
                    }
                    else if(g_vbo_recno[m_sChannelInfo[ch_v].ucInputPort] == 4) //H30K_Doulas_0052 8-lane(240hz)
                    {
                        dvC341_Write(B0_MCT2,  m_sChannelInfo[ch_v].ucMCT, 0);
                    }
                    else
                    {
                        dvC341_Write(B0_MCT2,  m_sChannelInfo[ch_v].ucMCT & 0xEF, 0);
                    }
                }
                else
                    dvC341_Write(B0_MCT2,  m_sChannelInfo[ch_v].ucMCT, 0);
            }
            else
            {
                if(dvC341_Read(B0_MCT2,0) != m_sChannelInfo[ch_v].ucMCT)
                    dvC341_Write(B0_MCT2,  m_sChannelInfo[ch_v].ucMCT, 0);
			}
            break;

        case eC341_CH_V1:
            if(dvC341_Read(B0_MCT3,0) != m_sChannelInfo[ch_v].ucMCT)
                dvC341_Write(B0_MCT3,  m_sChannelInfo[ch_v].ucMCT, 0);
            if(dvC341_Read(B0_MCT4,0) != m_sChannelInfo[ch_v].ucMCT)
                dvC341_Write(B0_MCT4,  m_sChannelInfo[ch_v].ucMCT, 0); //H30K_Doulas_0002
            break;
    }

    if (g_port[ch_v] != m_sChannelInfo[ch_v].ucInputPort)
    {
        LOG_MSG(db_DV_SCALER, "dvC341_SetInputPort_2K ch_v=%d port=%d\n g_port Update\n", ch_v, m_sChannelInfo[ch_v].ucInputPort);
        g_port[ch_v] = m_sChannelInfo[ch_v].ucInputPort;
    }

    dvC341_set_VBOInput(ch_v, m_sChannelInfo[ch_v].ucInputPort);
}

///////////////////////////////////////////////////////////////////////////////////
///@brief dvC341_SetInput_2K : set C341 input register, like IACT
///@return void -
///@author Roger Chen
///////////////////////////////////////////////////////////////////////////////////
//(V Total) x (V Freq) = (H Freq)
//PCLK = (V Total) x (H Total) x (V Freq) = (H Freq) x (H Total)

static BOOL dvC341_MeasureInput_4K(eC341_CH_V ch_v)
{
	int OLD_ACTHST, OLD_ACTHW, OLD_ACTVST, OLD_ACTVW;
	int OLD_HTOTAL, OLD_VTOTAL;
	int OLD_FHM, OLD_FVM;
	int ph0;
	int ifh, ifv;
	int nosigal;
	int vbo_in = TRUE;
	float lanefpiclk;
	int lanect;

	int port_offset;
	int port_info_chg;
	int i;
	int	flg_4k;

	INT32 lScanMode = 0;  //PI_PORT_INFO[ucInputPort].STATE
	BOOL blSTATE_Changed = FALSE; //H30K_Doulas_0070

    UINT16 uiPh0 = 0;
    UINT16 uiPh1 = 0;
    UINT16 uiPv0 = 0;
    UINT16 uiPv1 = 0;
    UINT32 ulImfv = 0;  //A70LV_Doulas_0115
    UINT32 ulImfh = 0;
 //   UINT8  ucFldhist = 0; //A70LV_Doulas_0008 remove
    DOUBLE dTmp_frate = 0;

 //   UINT32 ulReg_ifh = 0; //A70LV_Doulas_0008 remove
    //UINT32 ulReg_ihw;
    //    UINT32 ulReg_ivw;
    UINT32 ulReg_ph0 = 0;
    UINT32 ulReg_ph1 = 0;
    UINT32 ulReg_pv0 = 0;
    UINT32 ulReg_pv1 = 0;
  //  UINT32 ulReg_fldhist = 0; //A70LV_Doulas_0008 remove
    UINT32 ulReg_imfv = 0;
    UINT32 ulReg_imfh = 0;  //A70LV_Doulas_0115
    UINT8  ucRegOffset = 0;

 //   UINT8  ucISYCT = 0;   //A70LV_Doulas_0008 remove
    UINT32 ulErr = 0;
    UINT32 ulCt = 0;
    BOOL   bWaitPIVS = FALSE;
    UINT32 ulRTCT = 0;
    INT32 rdt[4],val_pos[2],val_neg[2],val,sync_pol;

    UINT8 ucInputPort; //H30K_Wesley_0001

    dvC341_InputPort_Set(); //H30K_Wesley_0001
    ucInputPort = m_sChannelInfo[ch_v].ucInputPort;

    LOG_MSG(db_DV_SCALER, ">>>(func:%s, line:%d) Start[%d][%d]\n", __FUNCTION__, __LINE__, ch_v, ucInputPort);

    if(ch_v >= eC341_CH_VNUMBER)
    {
        LOG_MSG(db_DV_SCALER, "!!!(func:%s, line:%d) ch_v %d error\n", __FUNCTION__, __LINE__, ch_v);
        return FALSE;
    }

    //only support VBO
    if(ucInputPort != eC341_INPUT_PORT2 &&
       ucInputPort != eC341_INPUT_PORT3)
    {
        LOG_MSG(db_DV_SCALER, "!!!(func:%s, line:%d) InputPort %d not support\n", __FUNCTION__, __LINE__, m_sChannelInfo[ch_v].ucInputPort);
        return FALSE;
    }

	//MS_SLEEP(100);

    //VBO input set by dvC341_SetInputPort_2K
    dvC341_set_VBOInput(ch_v, m_sChannelInfo[ch_v].ucInputPort); //H30K_Wesley_0001

	OLD_HTOTAL = m_sChannelInfo[ch_v].sPortInfo.uiHTotal;
	OLD_VTOTAL = m_sChannelInfo[ch_v].sPortInfo.uiVTotal;
	OLD_FHM    = m_sChannelInfo[ch_v].sPortInfo.ulHFreqMclkCount;
	OLD_FVM    = m_sChannelInfo[ch_v].sPortInfo.ulVFreqMclkCount;
	OLD_ACTHW  = m_sChannelInfo[ch_v].sPortInfo.uiHActive;

    port_offset = ucInputPort - 2;  //for VBO

	dvC341_Write(B0_RHOLD, 0x80, 0);
	if (vbo_in == 0)
	{
		m_sChannelInfo[ch_v].sPortInfo.uiHTotal          = dvC341_Read(B174_PI1FH + 0x400*port_offset, 0) + 1;
		m_sChannelInfo[ch_v].sPortInfo.uiVTotal          = dvC341_Read(B174_PI1FV + 0x400*port_offset, 0);
		m_sChannelInfo[ch_v].sPortInfo.ulHFreqMclkCount  = dvC341_Read(B174_PI1FHM + 0x400*port_offset, 0) + 1;
		m_sChannelInfo[ch_v].sPortInfo.ulVFreqMclkCount  = dvC341_Read(B174_PI1FVM + 0x600*port_offset, 0) + 1;
		m_sChannelInfo[ch_v].sPortInfo.uiHActive         = dvC341_Read(B175_PI10ACTHW + 0x400*port_offset, 0);
	}
	else
	{
		m_sChannelInfo[ch_v].sPortInfo.uiHTotal          = dvC341_Read(B174_VI11FH + 0x800*port_offset, 0) + 1;
		m_sChannelInfo[ch_v].sPortInfo.uiVTotal          = dvC341_Read(B174_VI11FV + 0x800*port_offset, 0);
		m_sChannelInfo[ch_v].sPortInfo.ulHFreqMclkCount  = dvC341_Read(B174_VI11FHM + 0x800*port_offset, 0) + 1;
		m_sChannelInfo[ch_v].sPortInfo.ulVFreqMclkCount  = dvC341_Read(B174_VI11FVM + 0xc00*port_offset, 0) + 1;
		m_sChannelInfo[ch_v].sPortInfo.uiHActive         = dvC341_Read(B175_VI11ACTHW + 0x800*port_offset, 0);
        dbmsg("m_sChannelInfo[%d].sPortInfo.uiHTotal %d\n", ch_v, m_sChannelInfo[ch_v].sPortInfo.uiHTotal);
        dbmsg("m_sChannelInfo[%d].sPortInfo.uiVTotal %d\n", ch_v, m_sChannelInfo[ch_v].sPortInfo.uiVTotal);
        dbmsg("m_sChannelInfo[%d].sPortInfo.ulHFreqMclkCount %d\n", ch_v, m_sChannelInfo[ch_v].sPortInfo.ulHFreqMclkCount);
        dbmsg("m_sChannelInfo[%d].sPortInfo.ulVFreqMclkCount %d\n", ch_v, m_sChannelInfo[ch_v].sPortInfo.ulVFreqMclkCount);
        dbmsg("m_sChannelInfo[%d].sPortInfo.uiHActive %d\n", ch_v, m_sChannelInfo[ch_v].sPortInfo.uiHActive);
	}



	dvC341_Write(B0_RHOLD, 0x00, 0);

	// check signal
	nosigal = (m_sChannelInfo[ch_v].sPortInfo.uiHTotal           >= 0xffff  ) ? 1
			: (m_sChannelInfo[ch_v].sPortInfo.uiHTotal           == 0x0000  ) ? 1
			: (m_sChannelInfo[ch_v].sPortInfo.ulVFreqMclkCount   >= 0xffffff) ? 1
			: (m_sChannelInfo[ch_v].sPortInfo.ulVFreqMclkCount   == 0x000000) ? 1
			: (m_sChannelInfo[ch_v].sPortInfo.ulHFreqMclkCount   >= 0xffff  ) ? 1
			: (m_sChannelInfo[ch_v].sPortInfo.ulHFreqMclkCount   == 0x0000  ) ? 1
			: 0;

    if (nosigal == 1)
	{
        //clear info
        memset(&m_sChannelInfo[ch_v].sPortInfo,          0, sizeof(m_sChannelInfo[ch_v].sPortInfo));
        memset(&m_sChannelInfo[ch_v].sInputTimingInfo,   0, sizeof(m_sChannelInfo[ch_v].sPortInfo));
        m_sChannelInfo[ch_v].sPortInfo.eScanMode        = eSCAN_MODE_NO_SIGNAL;
        m_sChannelInfo[ch_v].sInputTimingInfo.eScanMode = eSCAN_MODE_NO_SIGNAL;

	    dvC341_set_mct(ch_v, 1);
	    dvC341_reset_vborx(port_offset, 0x80, 0.0F, 1);

	    return FALSE;  //no signal
	}


	if ((OLD_HTOTAL < (m_sChannelInfo[ch_v].sPortInfo.uiHTotal - MEASURE_PORT_HTOTAL_MARGIN)) |
	    (OLD_HTOTAL > (m_sChannelInfo[ch_v].sPortInfo.uiHTotal + MEASURE_PORT_HTOTAL_MARGIN)))
	{
		port_info_chg |= 1;
	}
	else if ((OLD_VTOTAL < (m_sChannelInfo[ch_v].sPortInfo.uiVTotal - MEASURE_PORT_VTOTAL_MARGIN)) |
	         (OLD_VTOTAL > (m_sChannelInfo[ch_v].sPortInfo.uiVTotal + MEASURE_PORT_VTOTAL_MARGIN)))
	{
		port_info_chg |= 1;
	}
	else if ((OLD_FHM < (m_sChannelInfo[ch_v].sPortInfo.ulHFreqMclkCount - MEASURE_PORT_FREQ_H_MCLK_COUNT_MARGIN)) |
		     (OLD_FHM > (m_sChannelInfo[ch_v].sPortInfo.ulHFreqMclkCount + MEASURE_PORT_FREQ_H_MCLK_COUNT_MARGIN)))
	{
		port_info_chg |= 1;
	}
	else if ((OLD_FVM < (m_sChannelInfo[ch_v].sPortInfo.ulVFreqMclkCount - MEASURE_PORT_FREQ_V_MCLK_COUNT_MARGIN)) |
		     (OLD_FVM > (m_sChannelInfo[ch_v].sPortInfo.ulVFreqMclkCount + MEASURE_PORT_FREQ_V_MCLK_COUNT_MARGIN)))
	{
		port_info_chg |= 1;
	}
	else if (OLD_ACTHW != m_sChannelInfo[ch_v].sPortInfo.uiHActive)
	{
		port_info_chg |= 1;
	}

    if (port_info_chg != 0)
	{
		m_sChannelInfo[ch_v].sPortInfo.ulHFreqMclkCount = (m_sChannelInfo[ch_v].sPortInfo.ulHFreqMclkCount == 0) ? 1 : m_sChannelInfo[ch_v].sPortInfo.ulHFreqMclkCount;
		m_sChannelInfo[ch_v].sPortInfo.ulVFreqMclkCount = (m_sChannelInfo[ch_v].sPortInfo.ulVFreqMclkCount == 0) ? 1 : m_sChannelInfo[ch_v].sPortInfo.ulVFreqMclkCount;

		m_sChannelInfo[ch_v].sPortInfo.dHFreq     = (float)(m_sChannelInfo[ch_v].ulFMCLK) / (float)(m_sChannelInfo[ch_v].sPortInfo.ulHFreqMclkCount);  //how many H sync in one second  (H Freq)
		m_sChannelInfo[ch_v].sPortInfo.dFrameRate = (float)(m_sChannelInfo[ch_v].ulFMCLK) / (float)(m_sChannelInfo[ch_v].sPortInfo.ulVFreqMclkCount);  //how many V sync in one second  (V Freq)

		lScanMode = (m_sChannelInfo[ch_v].sPortInfo.uiVTotal >= 560) & (m_sChannelInfo[ch_v].sPortInfo.uiVTotal <= 565) & (m_sChannelInfo[ch_v].sPortInfo.dFrameRate >= 55.0F) ? 1 // 1080i60
  				  : (m_sChannelInfo[ch_v].sPortInfo.uiVTotal >= 560) & (m_sChannelInfo[ch_v].sPortInfo.uiVTotal <= 565) & (m_sChannelInfo[ch_v].sPortInfo.dFrameRate  < 55.0F) ? 2 // 1080i50
  				  : (m_sChannelInfo[ch_v].sPortInfo.uiVTotal >= 266) & (m_sChannelInfo[ch_v].sPortInfo.uiVTotal <= 350) ? 4  // 576i
  				  : (m_sChannelInfo[ch_v].sPortInfo.uiVTotal >= 260) & (m_sChannelInfo[ch_v].sPortInfo.uiVTotal <= 265) ? 3  // 480i
  				  : 0;


        //search Vx1 lut (g_vboinfo_inittable)
		if(vbo_in == TRUE)
		{
			g_vbo_recno[ucInputPort] = dvC341_Get_VBOInfo(ucInputPort, m_sChannelInfo[ch_v].sPortInfo.uiVTotal, m_sChannelInfo[ch_v].sPortInfo.uiHTotal, ch_v); //H30K_Doulas_0011

            LOG_MSG(db_DV_SCALER, "!!!(func:%s, line:%d) g_vbo_recno[%d] = %d\n", __FUNCTION__, __LINE__, ucInputPort, g_vbo_recno[ucInputPort]);

			m_sChannelInfo[ch_v].sPortInfo.ucVBOLaneNum = g_vboinfo[g_vbo_recno[ucInputPort]].CH_LANE_NUM;

			m_sChannelInfo[ch_v].sPortInfo.fPICLK = m_sChannelInfo[ch_v].sPortInfo.dHFreq * m_sChannelInfo[ch_v].sPortInfo.uiHTotal;

			lanefpiclk = 0.0F;
			if (m_sChannelInfo[ch_v].sPortInfo.ucVBOLaneNum != 0)
			{
				lanefpiclk = m_sChannelInfo[ch_v].sPortInfo.fPICLK;
				m_sChannelInfo[ch_v].sPortInfo.fPICLK = m_sChannelInfo[ch_v].sPortInfo.fPICLK * m_sChannelInfo[ch_v].sPortInfo.ucVBOLaneNum;
			}

			m_sChannelInfo[ch_v].sPortInfo.lSTATE =  (lScanMode == 1)                              ? eINPUT_PORT_INFO_STATE_1080I60 // 1080i60
                                                   : (lScanMode == 2)                              ? eINPUT_PORT_INFO_STATE_1080I50 // 1080i50
                                                   : (lScanMode == 3)                              ? eINPUT_PORT_INFO_STATE_480I // 480i
                                                   : (lScanMode == 4)                              ? eINPUT_PORT_INFO_STATE_576I // 576i
                                                   : (g_vboinfo[g_vbo_recno[ucInputPort]].CH == 2) ? eINPUT_PORT_INFO_STATE_2CH // 2ch
                                                   : (g_vboinfo[g_vbo_recno[ucInputPort]].CH == 4) ? eINPUT_PORT_INFO_STATE_4CH // 4ch
                                                   : (g_vboinfo[g_vbo_recno[ucInputPort]].CH == 1) ? eINPUT_PORT_INFO_STATE_1CH // 1ch
                                                   : eINPUT_PORT_INFO_STATE_NA;

			lanect = (g_vboinfo[g_vbo_recno[ucInputPort]].LANE_NUM == 16) ? 0xff
				   : (g_vboinfo[g_vbo_recno[ucInputPort]].LANE_NUM == 8)  ? 0xff
				   : (g_vboinfo[g_vbo_recno[ucInputPort]].LANE_NUM == 4)  ? 0xf0
				   : (g_vboinfo[g_vbo_recno[ucInputPort]].LANE_NUM == 2)  ? 0xc0
				   : 0x80;
			//write_C341(B2_VBORX1_LANECT + 0x1c00*port_offset, lanect, 0);
			//wait1_pivs(ch_v);
			//wait1_pivs(ch_v);

			dvC341_reset_vborx(port_offset, lanect, lanefpiclk, 0);

			dvC341_set_sypol(1, port_offset);

			//set_VBORXfrange(port_offset, 0, lanefpiclk);

			dvC341_set_VBOInput(ch_v, ucInputPort);  //H2PF_Simon_0195 , 沒有此行的話，下方 B172 相關register會讀錯
		}

		for (i = 0; i < 2; i++)
		{
			if (g_port[i] == ucInputPort)
			{
				g_chinfo_chg[i] |= 1;
			}
		}
	}

    if(m_sChannelInfo[ch_v].sInputTimingInfo.lSTATE != m_sChannelInfo[ch_v].sPortInfo.lSTATE) //H30K_Doulas_0070
    {
    //    LOG_MSG(db_ALWAYS,"lSTATE change (%d) \r\n",ch_v);
        blSTATE_Changed = TRUE;
    }
    m_sChannelInfo[ch_v].sInputTimingInfo.lSTATE = m_sChannelInfo[ch_v].sPortInfo.lSTATE;

    if (port_info_chg == 1)
    {
		LOG_MSG(db_DV_SCALER, "PortInfo(%d) HTOTAL=%d  VTOTAL=%d  ACTHW=%d  FV=%.4f  FPICLK=%.4f Update\n",
			m_sChannelInfo[ch_v].ucInputPort,
			m_sChannelInfo[ch_v].sPortInfo.uiHTotal,
			m_sChannelInfo[ch_v].sPortInfo.uiVTotal,
			m_sChannelInfo[ch_v].sPortInfo.uiHActive,
			m_sChannelInfo[ch_v].sPortInfo.dFrameRate,
			m_sChannelInfo[ch_v].sPortInfo.fPICLK);  //ucInputPort
	}

    // for ch_v
	OLD_ACTVW  = m_sChannelInfo[ch_v].sIdual_Act_Ofst.uiVw;
	OLD_ACTVST = m_sChannelInfo[ch_v].sIdual_Act_Ofst.uiVst;
	OLD_ACTHST = m_sChannelInfo[ch_v].sIdual_Act_Ofst.uiHst;

	// get port info
	dvC341_Write(B0_RHOLD, 0x80, 0);
	m_sChannelInfo[ch_v].sIdual_Act_Ofst.uiVw  = dvC341_Read(B172_IFACTVWCH1  + 0x1000*ch_v, 0);
	m_sChannelInfo[ch_v].sIdual_Act_Ofst.uiVst = dvC341_Read(B172_IFACTVSTCH1 + 0x1000*ch_v, 0) - 1;
	m_sChannelInfo[ch_v].sIdual_Act_Ofst.uiHst = dvC341_Read(B172_IFACTHSTCH1 + 0x1000*ch_v, 0) + 7;
	dvC341_Write(B0_RHOLD, 0x00, 0);

	if (OLD_ACTVW != m_sChannelInfo[ch_v].sIdual_Act_Ofst.uiVw)
	{
		m_sChannelInfo[ch_v].ucCH_INFO_CHG |= 1;
	}
	else if (OLD_ACTVST != m_sChannelInfo[ch_v].sIdual_Act_Ofst.uiVst)
	{
		m_sChannelInfo[ch_v].ucCH_INFO_CHG |= 1;
	}
	else if (OLD_ACTHST != m_sChannelInfo[ch_v].sIdual_Act_Ofst.uiHst)
	{
		m_sChannelInfo[ch_v].ucCH_INFO_CHG |= 1;
	}
	else if(OLD_VTOTAL != m_sChannelInfo[ch_v].sPortInfo.uiVTotal) //H30K_Doulas_0001
	{
		m_sChannelInfo[ch_v].ucCH_INFO_CHG |= 1;
	}
	else if(blSTATE_Changed) //H30K_Doulas_0070
	{
        m_sChannelInfo[ch_v].ucCH_INFO_CHG |= 1;
	}


	if (m_sChannelInfo[ch_v].ucCH_INFO_CHG != 0)
	{
		// De-interlacer -> off
		dvC341_set_mct(ch_v, 1);

		//set_idual(ch_v, port, 1);

		dvC341_Write(B0_RHOLD, 0x80, 0);
		ifh = dvC341_Read(B168_IFHCH1 + 0xc00*ch_v, 0) + 1;
		ifv = dvC341_Read(B168_IFVCH1 + 0xc00*ch_v, 0);
		dvC341_Write(B0_RHOLD, 0x00, 0);

		dvC341_set_idual(ch_v, ucInputPort, 1);

		if (ifh < 0x4000)
		{
			m_sChannelInfo[ch_v].sIdual_Act_Ofst.uiHst = dvC341_Read(B172_IFACTHSTCH1 + 0x1000*ch_v, 0) + 7;
			m_sChannelInfo[ch_v].sInputTimingInfo.uiHStart = m_sChannelInfo[ch_v].sIdual_Act_Ofst.uiHst;
		}
		else
		{
			dvC341_Write(B16_PMODECH1, 0xa1, 32*ch_v);
			dvC341_wait1_pivs(ch_v);
			m_sChannelInfo[ch_v].sIdual_Act_Ofst.uiHst = dvC341_Read(B172_IFACTHSTCH1 + 0x1000*ch_v, 0) + 7;
			dvC341_wait1_pivs(ch_v);
			ph0 = dvC341_Read(B170_PH0CH1 + 0x1600*ch_v, 0);
			m_sChannelInfo[ch_v].sInputTimingInfo.uiHStart = ph0 + 7;
		}
        //LOG_MSG(db_DV_SCALER, "C341 ###  ifhL=0x%xh uiHst=(%d,%d) \n",ifh
        //   ,m_sChannelInfo[ch_v].sIdual_Act_Ofst.uiHst,m_sChannelInfo[ch_v].sInputTimingInfo.uiHStart); //H30K_Doulas_0002 debug
		m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive  = m_sChannelInfo[ch_v].sPortInfo.uiHActive;
		m_sChannelInfo[ch_v].sInputTimingInfo.uiVStart   = m_sChannelInfo[ch_v].sIdual_Act_Ofst.uiVst;
		m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive  = m_sChannelInfo[ch_v].sIdual_Act_Ofst.uiVw;
		m_sChannelInfo[ch_v].sInputTimingInfo.dHFreq     = ifh;
		m_sChannelInfo[ch_v].sInputTimingInfo.dFrameRate = ifv;

		m_sChannelInfo[ch_v].sInputTimingInfo.uiHTotal   = m_sChannelInfo[ch_v].sPortInfo.uiHTotal;
		m_sChannelInfo[ch_v].sInputTimingInfo.uiVTotal   = m_sChannelInfo[ch_v].sPortInfo.uiVTotal;
		m_sChannelInfo[ch_v].sInputTimingInfo.dHFreq     = m_sChannelInfo[ch_v].sPortInfo.dHFreq;
		m_sChannelInfo[ch_v].sInputTimingInfo.dFrameRate = m_sChannelInfo[ch_v].sPortInfo.dFrameRate;
		m_sChannelInfo[ch_v].sInputTimingInfo.fPICLK     = m_sChannelInfo[ch_v].sPortInfo.fPICLK;

		if (vbo_in == 1)
		{
			m_sChannelInfo[ch_v].sInputTimingInfo.uiHTotal  *= m_sChannelInfo[ch_v].sPortInfo.ucVBOLaneNum;
			m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive *= m_sChannelInfo[ch_v].sPortInfo.ucVBOLaneNum;
		}

		dvC341_set_idual(ch_v, ucInputPort, 0);

//		PI_ACT[ch_v].HW = PI_4K[ch_v] ? PI_ACT[ch_v].HW*2 : PI_ACT[ch_v].HW;

		//PI_SIGMODE[ch_v] = get_SigID(ch_v);
		//PI_SCAN_MODE[ch_v] = (PI_SIGMODE[ch_v] < 0) ? 0
		//				   : g_imode[PI_SIGMODE[ch_v]].SCAN_MODE;
        if(m_sChannelInfo[ch_v].sInputTimingInfo.lSTATE == eINPUT_PORT_INFO_STATE_NA ||
           m_sChannelInfo[ch_v].sInputTimingInfo.lSTATE >= eINPUT_PORT_INFO_STATE_NUMBER )
        {
            m_sChannelInfo[ch_v].sInputTimingInfo.eScanMode = eSCAN_MODE_NO_SIGNAL;
        }
        else if(m_sChannelInfo[ch_v].sInputTimingInfo.lSTATE == eINPUT_PORT_INFO_STATE_1080I60 ||
                m_sChannelInfo[ch_v].sInputTimingInfo.lSTATE == eINPUT_PORT_INFO_STATE_1080I50 ||
                m_sChannelInfo[ch_v].sInputTimingInfo.lSTATE == eINPUT_PORT_INFO_STATE_480I    ||
                m_sChannelInfo[ch_v].sInputTimingInfo.lSTATE == eINPUT_PORT_INFO_STATE_576I )
        {
            m_sChannelInfo[ch_v].sInputTimingInfo.eScanMode = eSCAN_MODE_INTERLACE;
        }
        else
        {
            m_sChannelInfo[ch_v].sInputTimingInfo.eScanMode = eSCAN_MODE_PROGRESSIVE;
        }

		m_sChannelInfo[ch_v].ucInputUpdate = 1;
		m_sChannelInfo[ch_v].ucCH_INFO_CHG = 0;
	}

	if (m_sChannelInfo[ch_v].ucInputUpdate == 1)
	{
		LOG_MSG(db_DV_SCALER, "C341 CHInfo  HTOTAL=%d  VTOTAL=%d  ACTHW=%d  FV=%.4f  FPICLK=%.4f  ACTVW=%d\n",
                               m_sChannelInfo[ch_v].sInputTimingInfo.uiHTotal,
                               m_sChannelInfo[ch_v].sInputTimingInfo.uiVTotal,
                               m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive,
                               m_sChannelInfo[ch_v].sInputTimingInfo.dFrameRate,
                               m_sChannelInfo[ch_v].sInputTimingInfo.fPICLK,
                               m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive);

		LOG_MSG(db_DV_SCALER, "PI_STATE=%d  PI_SCAN_MODE=%d  PI_UPDATE=%d ch_v=%d port=%d  ACTHST=%d  ACTVST=%d\n\n",
                                m_sChannelInfo[ch_v].sInputTimingInfo.lSTATE,
                                m_sChannelInfo[ch_v].sInputTimingInfo.eScanMode,
                                m_sChannelInfo[ch_v].ucInputUpdate,
                                ch_v,
                                ucInputPort,
                                m_sChannelInfo[ch_v].sInputTimingInfo.uiHStart,
                                m_sChannelInfo[ch_v].sInputTimingInfo.uiVStart); //H30K_Doulas_0044
	}

    if(m_sChannelInfo[ch_v].sInputTimingInfo.eScanMode == eSCAN_MODE_NO_SIGNAL)
    {
        m_sChannelInfo[ch_v].ucCH_INFO_CHG = 1;
        return FALSE;
    }

    if(dvC341_Timing_Detect_Fail_By_Front_End(ch_v)) //H30K_Doulas_0050
        return FALSE;

    return TRUE;
}

BOOL dvC341_CheckInput_4K(eC341_CH ch_v)
{
    sINPUT_TIMING_INFO sInput_Old[eC341_CH_VNUMBER];
    UINT32 ulDiff_act_hst;
    UINT32 ulDiff_act_hw;
    UINT32 ulDiff_act_vst;
    UINT32 ulDiff_act_vw;
    UINT32 ulDiff_ihcycl;
    UINT32 ulDiff_ivcycl;
    BOOL bSignalReady = FALSE;
    DOUBLE dDiff_V_Freq;//A70LV_Doulas_0078


    if(ch_v >= eC341_CH_VNUMBER)
    {
        return FALSE;
    }

    if(!m_sChannelInfo[ch_v].bInit)
    {
        return FALSE;
    }

    if(m_sChannelSetting[ch_v].cFREEZE == TRUE)     //A70LV_Doulas_0030 //A70LV_Doulas_0004
    {
        return FALSE;
    }

    sInput_Old[ch_v].dFrameRate       = m_sChannelInfo[ch_v].sInputTimingInfo.dFrameRate;
    sInput_Old[ch_v].uiHTotal         = m_sChannelInfo[ch_v].sInputTimingInfo.uiHTotal;
    sInput_Old[ch_v].uiVTotal         = m_sChannelInfo[ch_v].sInputTimingInfo.uiVTotal;
    sInput_Old[ch_v].uiHStart         = m_sChannelInfo[ch_v].sInputTimingInfo.uiHStart;
    sInput_Old[ch_v].uiHActive        = m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive;
    sInput_Old[ch_v].uiVStart         = m_sChannelInfo[ch_v].sInputTimingInfo.uiVStart;
    sInput_Old[ch_v].uiVActive        = m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive;
    sInput_Old[ch_v].eScanMode        = m_sChannelInfo[ch_v].sInputTimingInfo.eScanMode;
    sInput_Old[ch_v].ulHFreqMclkCount = m_sChannelInfo[ch_v].sInputTimingInfo.ulHFreqMclkCount;
    sInput_Old[ch_v].ulVFreqMclkCount = m_sChannelInfo[ch_v].sInputTimingInfo.ulVFreqMclkCount;

    bSignalReady = dvC341_MeasureInput_4K(ch_v);

    if(bSignalReady)
    {
        ulDiff_act_hst = abs(m_sChannelInfo[ch_v].sInputTimingInfo.uiHStart  - sInput_Old[ch_v].uiHStart);
        ulDiff_act_hw  = abs(m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive - sInput_Old[ch_v].uiHActive);
        ulDiff_act_vst = abs(m_sChannelInfo[ch_v].sInputTimingInfo.uiVStart  - sInput_Old[ch_v].uiVStart);
        ulDiff_act_vw  = abs(m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive - sInput_Old[ch_v].uiVActive);
        ulDiff_ihcycl  = abs(m_sChannelInfo[ch_v].sInputTimingInfo.uiHTotal  - sInput_Old[ch_v].uiHTotal);
        ulDiff_ivcycl  = abs(m_sChannelInfo[ch_v].sInputTimingInfo.uiVTotal  - sInput_Old[ch_v].uiVTotal);
        if(m_sChannelInfo[ch_v].sInputTimingInfo.dFrameRate > sInput_Old[ch_v].dFrameRate)        //A70LV_Doulas_0078
        {
            dDiff_V_Freq  = m_sChannelInfo[ch_v].sInputTimingInfo.dFrameRate - sInput_Old[ch_v].dFrameRate;
        }
        else
        {
            dDiff_V_Freq  = sInput_Old[ch_v].dFrameRate - m_sChannelInfo[ch_v].sInputTimingInfo.dFrameRate;
        }

        LOG_MSG(db_DV_SCALER, "(func:%s, line:%d) C341[%d] VF= %f \n", __FUNCTION__, __LINE__,
                ch_v,
                dDiff_V_Freq);

        if((ulDiff_act_hst > DEF_PI_ACTHST_ERR_RANGE) || (ulDiff_act_hw > DEF_PI_ACTHW_ERR_RANGE)
            || (ulDiff_act_vst > DEF_PI_ACTVST_ERR_RANGE) || (ulDiff_act_vw > DEF_PI_ACTVW_ERR_RANGE)
            || (ulDiff_ihcycl > DEF_PI_HCYCL_ERR_RANGE) || (ulDiff_ivcycl > DEF_PI_VCYCL_ERR_RANGE)
            || (m_sChannelInfo[ch_v].sInputTimingInfo.eScanMode != sInput_Old[ch_v].eScanMode)
           // || (m_sChannelInfo[ch_v].sInputTimingInfo.dFrameRate != sInput_Old[ch_v].dFrameRate))
            || (dDiff_V_Freq > DEF_PI_VYNC_FREQ_ERR_RANGE))     //A70LV_Doulas_0078 modify
        {
            bSignalReady = FALSE;
            m_sChannelInfo[ch_v].bAutoPosition = FALSE;  //A70LV_Doulas_0210
            #if 0
            LOG_MSG(db_DV_SCALER, "@Start@ (%d,%d)[%d,%d][%d,%d][%d,%d](%d,%d)  \r\n",
                                m_sChannelInfo[ch_v].sInputTimingInfo.uiVStart,
                                sInput_Old[ch_v].uiVStart,
                                ulDiff_act_hst,
                                ulDiff_act_vst,
                                ulDiff_act_hw,
                                ulDiff_act_vw,
                                ulDiff_ihcycl,
                                ulDiff_ivcycl,
                                m_sChannelInfo[ch_v].sInputTimingInfo.eScanMode,
                                sInput_Old[ch_v].eScanMode);       //A70LV_Doulas_0175 debug
            #endif
        }
        else
        {
            #if 0  //H2PF_Simon_0195 remove (keep new value)
           /* if(m_sChannelInfo[ch_v].bAutoPosition == TRUE)       //A70LV_Doulas_0210 remove  //A70LV_Doulas_0118
            {
                //position use new
                m_sChannelInfo[ch_v].bAutoPosition = FALSE;
            }
            else*/
            {
                m_sChannelInfo[ch_v].sInputTimingInfo.uiHStart = sInput_Old[ch_v].uiHStart;
                m_sChannelInfo[ch_v].sInputTimingInfo.uiVStart = sInput_Old[ch_v].uiVStart;
            }
            m_sChannelInfo[ch_v].sInputTimingInfo.dFrameRate = sInput_Old[ch_v].dFrameRate;
            m_sChannelInfo[ch_v].sInputTimingInfo.uiHTotal = sInput_Old[ch_v].uiHTotal;
            m_sChannelInfo[ch_v].sInputTimingInfo.uiVTotal = sInput_Old[ch_v].uiVTotal;
            m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive = sInput_Old[ch_v].uiHActive;
            m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive = sInput_Old[ch_v].uiVActive;
            m_sChannelInfo[ch_v].sInputTimingInfo.eScanMode = sInput_Old[ch_v].eScanMode;
            #endif

            bSignalReady = TRUE;
            dvC341_HV_Sync_MCLK_Count_Get(ch_v); //A70LV_Doulas_0004
        }
        ucNo_Signal_Config[ch_v] = 0;    //A70LV_Doulas_0004
    }
    else
    {
        bSignalReady = FALSE;
        dvC341_Config_NoSignalOutput(ch_v, FALSE);  //A70LV_Doulas_0142 //A70LV_Doulas_0004  //A35G2_CDS_Simon_0017

    }


    m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.uiHStart = m_sChannelInfo[ch_v].sInputTimingInfo.uiHStart;
    m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.uiHActive = m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive;

    m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.uiVStart = m_sChannelInfo[ch_v].sInputTimingInfo.uiVStart;

    if(m_sChannelInfo[ch_v].sInputTimingInfo.eScanMode == eSCAN_MODE_INTERLACE)
    {
        m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.uiVActive = m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive * 2;
    }
    else
    {
        m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.uiVActive = m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive;
    }

    m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.uiHTotal = m_sChannelInfo[ch_v].sInputTimingInfo.uiHTotal;
    m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.uiVTotal = m_sChannelInfo[ch_v].sInputTimingInfo.uiVTotal;
    m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.dFrameRate = m_sChannelInfo[ch_v].sInputTimingInfo.dFrameRate;
    m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.eScanMode = m_sChannelInfo[ch_v].sInputTimingInfo.eScanMode;

    return bSignalReady;
}

///////////////////////////////////////////////////////////////////////////////////
///@brief dvC341_SetAutoFill_2K : Set output solid color
///@param ucFillColor eFillColor - select enum color
///@param BOOL bEnable - solid output on or off
///@return void -
///@author Roger Chen
///////////////////////////////////////////////////////////////////////////////////
void dvC341_SetAutoFill_2K(const eC341_CH_V ch_v, const UINT8 ucFillColor, const BOOL bEnable)
{
    UINT32 ulRtct = 0;
    UINT8 ucRegOffset = ch_v * CH_V_BANK_OFFSET;
    UINT32 ulColor = 0;

	UINT16 OutputPanel_HW       = m_sChannelInfo[eC341_CH_V0].sOutputTimingInfo.uiHActive;
	UINT16 OutputPanel_HW_1CH   = m_sChannelInfo[eC341_CH_V0].sOutputTimingInfo.uiHActive / OUTPUT4K_CH_NUM;
	UINT16 OutputPanel_HST      = m_sChannelInfo[eC341_CH_V0].sOutputTimingInfo.uiHStart;
	UINT16 OutputPanel_HST_1CH  = m_sChannelInfo[eC341_CH_V0].sOutputTimingInfo.uiHStart / OUTPUT4K_CH_NUM;

	UINT16 OutputPanel_VW       = m_sChannelInfo[eC341_CH_V0].sOutputTimingInfo.uiVActive;
	UINT16 OutputPanel_VST      = m_sChannelInfo[eC341_CH_V0].sOutputTimingInfo.uiVStart - 2 ;

    if(ch_v >= eC341_CH_VNUMBER)
    {
        return;
    }

    switch(ucFillColor)
    {
        case eFILL_COLOR_RED:
            ulColor = 0x0000FF;
            break;

        case eFILL_COLOR_GREEN:
            ulColor = 0x00FF00;
            break;

        case eFILL_COLOR_BLUE:
            ulColor = 0xFF0000;
            break;

        case eFILL_COLOR_YELLOW:
            ulColor = 0x00FFFF;
            break;

        case eFILL_COLOR_WHITE:
            ulColor = 0xFFFFFF;
            break;

        case eFILL_COLOR_BLACK:
        default:
            ulColor = 0x000000;
            break;

    }

    //ulRtct = dvC341_Read(B0_RTCT, 0);
    //ulRtct |= 0x000F00FF;
    //dvC341_Write(B0_RTCT, ulRtct, 0);   //simon check

    if(bEnable)
    {
        //simon check
        dvC341_Write_ch_v(ch_v, B8_OAOI1VSTCH1, m_sChannelInfo[ch_v].sOutputTimingInfo.uiVStart - 2/*PS_PANEL_VST*/, CH_BANK_OFFSET);//42 + 1080  //simon check (ori -5 ?!)
        dvC341_Write_ch_v(ch_v, B8_OAOI1VENDCH1, (m_sChannelInfo[ch_v].sOutputTimingInfo.uiVStart - 2 +
                                                  m_sChannelInfo[ch_v].sOutputTimingInfo.uiVActive/*PS_PANEL_VST + PS_PANEL_VW*/), CH_BANK_OFFSET);
        dvC341_Write_ch_v(ch_v, B8_OBKGD0CH1, ulColor, CH_BANK_OFFSET);
        dvC341_Write_ch_v(ch_v, B8_OBKGD1CH1, ulColor, CH_BANK_OFFSET);
        m_sChannelInfo[ch_v].ucOIMGCT |= 0x02;   //set background color
    }
    else
    {
        dvC341_Write_ch_v(ch_v, B8_OBKGD0CH1, 0x000000, CH_BANK_OFFSET);
        dvC341_Write_ch_v(ch_v, B8_OBKGD1CH1, 0x000000, CH_BANK_OFFSET);
        m_sChannelInfo[ch_v].ucOIMGCT &= 0xFD;
    }

    dvC341_Buffer_Flush();

    if(dvC341_Read(B8_OIMGCTCH1, (ch_v * 2) * CH_BANK_OFFSET) != m_sChannelInfo[ch_v].ucOIMGCT)
        dvC341_Write_ch_v(ch_v, B8_OIMGCTCH1, m_sChannelInfo[ch_v].ucOIMGCT, CH_BANK_OFFSET);
    //dvC341_Write(B0_RTCT, RTCT_NORMAL, 0);   //0x4321CBA9  //simon check
    //dvC341_set_rtct_normal();
}

static void dvC341_ConfigGamma2_2K(const eC341_CH_V ch_v)//dvC341_SetupGamma2
{
    DOUBLE dCoef;
    UINT32 ulGmdt[2048];
    DOUBLE dGmdt;
    UINT16 uiIndex1, uiIndex2;
    UINT8 ucRegOffset = 0;  //A35G2_Simon_0070 for cppcheck
    UINT8 *pcBuffer = NULL; //A35G2_CDS_Larry_0030

    if(ch_v >= eC341_CH_VNUMBER)
    {
        return;
    }

    if(m_sChannelSetting[ch_v].cGamma == 0)   //A35G2_CDS_Simon_0034
    {
        dvC341_Write(B13_GMBNKSEL21CH1, 0x00, ucRegOffset);
        dvC341_Write(B13_GMCT2CH1, 0x36, ucRegOffset);
        return;
    }

    for(uiIndex2 = 0; uiIndex2 < 8; uiIndex2++)
    {
        dCoef = (uiIndex2 == 0) ? 1.8 / 2.2
                : (uiIndex2 == 1) ? 2.0 / 2.2
                : (uiIndex2 == 2) ? 1.8 / 2.0
                : (uiIndex2 == 3) ? 2.2 / 2.0
                : (uiIndex2 == 4) ? 2.0 / 1.8
                : (uiIndex2 == 5) ? 2.2 / 1.8
                : (uiIndex2 == 6) ? 1.0 / 2.2
                : 2.2;

        for(uiIndex1 = 0; uiIndex1 < 256; uiIndex1++)
        {
            dGmdt = pow((DOUBLE)(uiIndex1 * 4) / 1023, dCoef) * 4095;
            ulGmdt[(uiIndex2 << 8) + uiIndex1] = (dGmdt > 4095) ? 4095 : (dGmdt < 0) ? 0 : (UINT32)dGmdt;
        }
    }

    dvC341_Write_ch_v(ch_v, B13_GMCT2CH1, 0xf0, CH_BANK_OFFSET);  // access R,G,B of A,B

    dvC341_Write_ch_v(ch_v, B13_GMAD2CH1, 0x00, CH_BANK_OFFSET);
    dvC341_Write_ch_v(ch_v, B13_GMAD2CH1, 0x00, CH_BANK_OFFSET);

    pcBuffer = malloc(2048*2);
    if(pcBuffer == NULL)
    {
        LOG_MSG(db_ALWAYS,"\n%s(%d) malloc fail\n", __FUNCTION__ , __LINE__);
        return;
    }

    for(uiIndex1 = 0; uiIndex1 < 2048; uiIndex1++)
    {
        pcBuffer[(uiIndex1*2)]      = ulGmdt[uiIndex1] & 0xFF;
        pcBuffer[(uiIndex1*2)+1]    = (ulGmdt[uiIndex1] >> 8) & 0xFF;
        //dvC341_Write(B13_GMDT2CH1, ulGmdt[uiIndex1] & 0xFF, ucRegOffset);
        //dvC341_Write(B13_GMDT2CH1, (ulGmdt[uiIndex1] >> 8) & 0xFF, ucRegOffset);
    }

    if(ch_v == eC341_CH_V0)
    {
        dvC341_BurstWrite_FixedAdd(B13_GMDT2CH1, 4096, pcBuffer); //A35G2_CDS_Larry_0030
        dvC341_BurstWrite_FixedAdd(B29_GMDT2CH2, 4096, pcBuffer); //A35G2_CDS_Larry_0030
    }
    else
    {
        dvC341_BurstWrite_FixedAdd(B45_GMDT2CH3, 4096, pcBuffer); //A35G2_CDS_Larry_0030
        dvC341_BurstWrite_FixedAdd(B61_GMDT2CH4, 4096, pcBuffer); //A35G2_CDS_Larry_0030
    }

    free(pcBuffer);

    if(m_sChannelSetting[ch_v].cGamma > 0)
    {
        dvC341_Write_ch_v(ch_v, B13_GMBNKSEL21CH1, (m_sChannelSetting[ch_v].cGamma - 1) & 0x1F, CH_BANK_OFFSET);
        dvC341_Write_ch_v(ch_v, B13_GMCT2CH1, 0x37, CH_BANK_OFFSET);
    }
    else
    {
        dvC341_Write_ch_v(ch_v, B13_GMBNKSEL21CH1, 0x00, CH_BANK_OFFSET);
        dvC341_Write_ch_v(ch_v, B13_GMCT2CH1, 0x36, CH_BANK_OFFSET);
    }
}

static void dvC341_ConfigZoomLut(const eC341_CH_V ch_v) //dvC341_SetupZoomLut
{
    // HZoom
    dvC341_WriteToBuffer(B0_SELCOEFSET, 0x2D, 0);   //simon check   //demo code use 0x2e
    dvC341_WriteToBuffer(B0_SELLUT, 0x81 + 0x08*ch_v, 0);
    dvC341_WriteToBuffer(B0_SELLUT, 0x85 + 0x08*ch_v, 0);

    // VZoom
    dvC341_WriteToBuffer(B0_SELCOEFSET, 0x2D, 0);
    dvC341_WriteToBuffer(B0_SELLUT, 0x80 + 0x08*ch_v, 0);
    dvC341_WriteToBuffer(B0_SELLUT, 0x84 + 0x08*ch_v, 0);

    dvC341_Buffer_Flush();
}


///////////////////////////////////////////////////////////////////////////////////
///@brief dvC341_GetChannelTiming : get each scaler panel timing from m_sPanelTable
///@param UINT8 ucPanelIndex - panel index
///@return void -
///@author Roger Chen
///////////////////////////////////////////////////////////////////////////////////

static void dvC341_GetCHOutputTiming_2K(const eC341_CH_V ch_v, const UINT8 ucPanelIndex)
{
    if(ch_v >= eC341_CH_VNUMBER)
    {
        return;
    }

    LOG_MSG(db_DV_SCALER, "(func:%s, line:%d)Output Res[%d x %d %dHz]\n", __FUNCTION__, __LINE__,
            m_sPanelTable[ucPanelIndex].sOutputTimingInfo.uiHActive,
            m_sPanelTable[ucPanelIndex].sOutputTimingInfo.uiVActive,
            m_sPanelTable[ucPanelIndex].sOutputTimingInfo.ucFrameRate);
    m_sChannelInfo[ch_v].sOutputTimingInfo.ucFrameRate = m_sPanelTable[ucPanelIndex].sOutputTimingInfo.ucFrameRate;
    m_sChannelInfo[ch_v].sOutputTimingInfo.uiPixClk = m_sPanelTable[ucPanelIndex].sOutputTimingInfo.uiPixClk;
    m_sChannelInfo[ch_v].sOutputTimingInfo.uiHStart = m_sPanelTable[ucPanelIndex].sOutputTimingInfo.uiHStart;
    m_sChannelInfo[ch_v].sOutputTimingInfo.uiHActive = m_sPanelTable[ucPanelIndex].sOutputTimingInfo.uiHActive;
    m_sChannelInfo[ch_v].sOutputTimingInfo.uiVStart = m_sPanelTable[ucPanelIndex].sOutputTimingInfo.uiVStart;
    m_sChannelInfo[ch_v].sOutputTimingInfo.uiVActive = m_sPanelTable[ucPanelIndex].sOutputTimingInfo.uiVActive;
    m_sChannelInfo[ch_v].sOutputTimingInfo.uiHTotal = m_sPanelTable[ucPanelIndex].sOutputTimingInfo.uiHTotal;
    m_sChannelInfo[ch_v].sOutputTimingInfo.uiVTotal = m_sPanelTable[ucPanelIndex].sOutputTimingInfo.uiVTotal;
    m_sChannelInfo[ch_v].sOutputTimingInfo.ucHSyncWidth = m_sPanelTable[ucPanelIndex].sOutputTimingInfo.ucHSyncWidth;
    m_sChannelInfo[ch_v].sOutputTimingInfo.ucVSyncWidth = m_sPanelTable[ucPanelIndex].sOutputTimingInfo.ucVSyncWidth;

}

void dvC341_SetOverlay_4K(const eC341_CH_V ch_v, UINT8 ucPIPEnable)   //ZU860_Doulas_0017
{
    if(ch_v >= eC341_CH_VNUMBER)
    {
        return;
    }

    UINT8 ucRegOffset = 0;
    UINT8 BankOffset = 16;

	int		TX1_L,TX1_R;
	int		i,cur_ch;
	int		OVLAREA = 0x0000;  // All Layer=OACT
	int		chgflg;
	int 	ovlct;
	int		ovlchsel[2] = {0};
	int		cnt;

    m_sChannelSetting[eC341_CH_V0].cPIP_PBP_ON = ucPIPEnable;    //A70LV_Doulas_0003 Add

    if(m_sChannelSetting[eC341_CH_V0].cPIP_PBP_ON == FALSE)
    {
        m_sChannelInfo[eC341_CH_V0].ucPRIORITY = 0x00000000;	// TX1=CH_V1/CH_V2
    }
    else
    {
		m_sChannelInfo[eC341_CH_V0].ucPRIORITY = 0x00000020;	// TX1=CH_V2/CH_V1
    }

	// TX2=Mirror
//		TX1_L = 0x000000a0;											// TX1=CH_V2/CH_V1
//		TX1_R = 0x000000b1;

	TX1_L = m_sChannelInfo[eC341_CH_V0].ucPRIORITY + 0x00000080;
	TX1_R = m_sChannelInfo[eC341_CH_V0].ucPRIORITY + 0x00000080 + 0x00000011;

	//PIP off
	//TX1_L = 0x80 (CH1)
	//TX1_R = 0x91 (CH2)

	//PIP on
	//TX1_L = 0xA0 (CH3 Ch1)
	//TX1_R = 0xB1 (CH4 CH2)

	for (i = 0; i < 2; i++)
	{
		cur_ch = ((TX1_L >> (i * 4)) & 0x00000007) / 2;

		if (ch_v == 999)
		{
			chgflg = 0;
		}
		else if ((ch_v == cur_ch && m_sChannelInfo[ch_v].ucLAYOUT_CHG == 1))
		{
			chgflg = 1;
		}
		else
		{
			chgflg = 0;
		}

		if (chgflg == 1) 	// LayoutCHG
		{
			OVLAREA = OVLAREA | (0x00000001 << (i * 2));
		}

	}

	if ((OVLAREA & 0x005) == 0x005)
	{
		ovlct = 0x02;  //alpha blend
	}
	else
	{
		ovlct = 0x01;  //overlay
		ovlchsel[0] = 0;
		ovlchsel[1] = 0;
		i = 0;
		for (cnt = 0; cnt < 2; cnt++)
		{
			if ((OVLAREA & (0x03 << cnt*2)) == 0)
			{
				ovlchsel[0] |= (((TX1_L >> cnt*4) & 0x0f) << i*4);
				ovlchsel[1] |= (((TX1_R >> cnt*4) & 0x0f) << i*4);
				i++;
			}
		}
	}

	dvC341_WriteToBuffer(B14_OVLCTCH1, ovlct, 0);
	dvC341_WriteToBuffer(B14_OVLCHSELCH1, ovlchsel[0], 0);
	dvC341_WriteToBuffer(B30_OVLCTCH2, ovlct, 0);
	dvC341_WriteToBuffer(B30_OVLCHSELCH2, ovlchsel[1], 0);



    #if 0   //simon check
    if(ucPIPEnable)
    {
        //set ch_v == 0 (ch1 & ch2)
        for(UINT8 ch=0; ch<OUTPUT4K_CH_NUM ;ch++)
        {
            ucRegOffset = ch * BankOffset;
            dvC341_WriteToBuffer(B14_OVLCTCH1,  0x01+(0x10*ch), ucRegOffset);
            dvC341_WriteToBuffer(B11_VEGSELCH1, 0x01,           ucRegOffset);  //VEG 5symbol  //Vertical edge control
        }
    }
    else
    {
        //set ch_v == 0 (ch1 & ch2)
        for(UINT8 ch=0; ch<OUTPUT4K_CH_NUM ;ch++)
        {
            ucRegOffset = ch * BankOffset;
            dvC341_WriteToBuffer(B14_OVLCTCH1,  0x00+(0x10*ch), ucRegOffset);
            dvC341_WriteToBuffer(B11_VEGSELCH1, 0x01,           ucRegOffset);  //VEG 5symbol  //A65_OPTOMA_Doulas_0060 Modify//Vertical edge control
        }
    }
    #endif

    #if 0  //simon check ... no need ?
    if((m_sChannelInfo[eC341_CH_V0].ePanelTimingId == ePANEL_ID_1080P_120HZ) ||
       (m_sChannelInfo[eC341_CH_V0].ePanelTimingId == ePANEL_ID_WUXGA_120HZ))   //A70LV_Doulas_0377 Modify//A70LV_Doulas_0155 //A70LV_Doulas_0154 modify //A70LV_Doulas_0098
    {
        switch(dvC341_Input_3D_Format_Config_Get())    //A70LV_Doulas_0154 modify
        {
            case eINPUT_3D_TYPE_FRAME_SEQUENTIAL:
                {
                    //set ch_v == 1 (ch3 & ch4)
                    for(UINT8 ch=2; ch<2+OUTPUT4K_CH_NUM ;ch++)
                    {
                        ucRegOffset = ch * BankOffset;
                        dvC341_WriteToBuffer(B14_OVLCTCH1,  0x00+(0x10*ch), ucRegOffset);
                    }

                    //set ch_v == 0 (ch1 & ch2)
                    for(UINT8 ch=0; ch<OUTPUT4K_CH_NUM ;ch++)
                    {
                        ucRegOffset = ch * BankOffset;
                        dvC341_WriteToBuffer(B11_VEGSELCH1,  0x01, ucRegOffset);
                    }
                }
                break;

            case eINPUT_3D_TYPE_FRAMEPACKING:
            case eINPUT_3D_TYPE_SIDEBYSIDE:
            case eINPUT_3D_TYPE_TOPANDBOTTOM:
                for(UINT8 ch=2; ch<2+OUTPUT4K_CH_NUM ;ch++)
                {
                    ucRegOffset = ch * BankOffset;
                    dvC341_WriteToBuffer(B14_OVLCTCH1,  0x00+(0x10*ch), ucRegOffset);
                }

				if((dvC341_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_TOPANDBOTTOM) &&
                   (m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.eScanMode == eSCAN_MODE_PROGRESSIVE) &&
                   ((m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive > 1918 && m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive < 1922) && (m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive == 1080)) &&
                   (m_sChannelInfo[ch_v].sInputTimingInfo.dFrameRate > 49.0 && m_sChannelInfo[ch_v].sInputTimingInfo.dFrameRate < 61.0))     //G100_Doulas_0064
                {
                    for(UINT8 ch=2; ch<2+OUTPUT4K_CH_NUM ;ch++)
                    {
                        ucRegOffset = ch * BankOffset;
                        dvC341_WriteToBuffer(B11_VEGSELCH1,  0x01, ucRegOffset);  //simon check (C821 only set ch1)
                    }
                }
                else if((dvC341_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_SIDEBYSIDE) &&
                   (m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.eScanMode == eSCAN_MODE_PROGRESSIVE) &&
                   ((m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive > 1918 && m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive < 1922) && (m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive == 1080)) &&
                   (m_sChannelInfo[ch_v].sInputTimingInfo.dFrameRate > 49.0 && m_sChannelInfo[ch_v].sInputTimingInfo.dFrameRate < 61.0))     //G100_Doulas_0064
                {
                    for(UINT8 ch=2; ch<2+OUTPUT4K_CH_NUM ;ch++)
                    {
                        ucRegOffset = ch * BankOffset;
                        dvC341_WriteToBuffer(B11_VEGSELCH1,  0x01, ucRegOffset);  //simon check (C821 only set ch1)
                    }
                }
                break;

            default:
                {
                    //set ch_v == 1 (ch3 & ch4)
                    for(UINT8 ch=2; ch<2+OUTPUT4K_CH_NUM ;ch++)
                    {
                        ucRegOffset = ch * BankOffset;
                        dvC341_WriteToBuffer(B14_OVLCTCH1,  0x00+(0x10*ch), ucRegOffset);
                    }
                }
                break;
        }
    }
    else
    {
        //set ch_v == 1 (ch3 & ch4)
        for(UINT8 ch=2; ch<2+OUTPUT4K_CH_NUM ;ch++)
        {
            ucRegOffset = ch * BankOffset;
            dvC341_WriteToBuffer(B14_OVLCTCH1,  0x00+(0x10*ch), ucRegOffset);
        }
    }
    #endif

    dvC341_Buffer_Flush();
}


//simon check ...
static void dvC341_ConfigOutput_2K_3D(const eC341_CH_V ch_v)       //A70LV_Doulas_0098
{
#if 1 //H30K_Doulas_0001
    UINT8 ucRegOffset = CH_BANK_OFFSET;
    //UINT32 ulData = 0 ;
    UINT16  ucSYRDLY,ucFLDDLY;   //A70LV_Doulas_0079  //H2PF_Simon_0196

    if(ch_v >= eC341_CH_VNUMBER)
    {
        return;
    }

    dvC341_WriteToBuffer(B14_OVLCTCH1, 0x00, (ch_v * 2) * CH_BANK_OFFSET);      //H30K_Doulas_0001
    dvC341_WriteToBuffer(B14_OVLCTCH1, 0x10, (ch_v * 2 + 1) * CH_BANK_OFFSET);  //H30K_Doulas_0001
    dvC341_Write_ch_v(ch_v, B16_AUTOFLDCH1, 0x80, CH_BANK_OFFSET);              //H30K_Doulas_0001
    if(dvC341_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_FRAME_SEQUENTIAL) //A70LV_Doulas_0154
    {
        dvC341_WriteToBuffer(B3_OIDUALCTCH12, 0x05, 0);//0x01, 0);     //ZU860_Doulas_0138 Modify    //PS_OIDUALCTCH12
        dvC341_WriteToBuffer(B3_ODACTHWCH12, (m_sChannelInfo[eC341_CH_V0].sOutputTimingInfo.uiHActive/OUTPUT4K_CH_NUM), 0);        //960
        dvC341_WriteToBuffer(B5_TESTG, 0x00, 0);
        dvC341_WriteToBuffer(B8_SYVRSTCTCH1, 0x00, 0);
        dvC341_Write_ch_v(ch_v, B8_OSYCT2CH1, 0x00, CH_BANK_OFFSET);
    }
    else
    {
        dvC341_WriteToBuffer(B3_OIDUALCTCH12, 0x05, 0);     //PS_OIDUALCTCH12
        dvC341_WriteToBuffer(B3_ODACTHWCH12, (m_sChannelInfo[eC341_CH_V0].sOutputTimingInfo.uiHActive/OUTPUT4K_CH_NUM), 0);        //960
        dvC341_WriteToBuffer(B5_TESTG, 0x00, 0);  //

        //dvC341_Write(B3_IDACTHSTCH12 + 0x900*ch_v, m_sChannelInfo[ch_v].sInputTimingInfo.uiHStart     , 0); //H30K_Doulas_0001 test
		//dvC341_Write(B3_IDACTHWCH12  + 0x900*ch_v, m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive * 2, 0); //H30K_Doulas_0001 test

        #if 1//H30K_Doulas_0034
        if(m_sChannelInfo[ch_v].sInputTimingInfo.eScanMode == eSCAN_MODE_INTERLACE)
        {
            dvC341_WriteToBuffer(B8_OSYCT2CH1, 0x10, ((ch_v * 2) + 0) * CH_BANK_OFFSET);
        }
        else
        {
            dvC341_Write_ch_v(ch_v, B8_OSYCT2CH1, 0x00, CH_BANK_OFFSET);
        }
        #else
        if(((dvC341_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_SIDEBYSIDE) ||
            (dvC341_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_FRAMEPACKING) ||
            (dvC341_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_TOPANDBOTTOM)) &&
           (m_sChannelInfo[ch_v].sInputTimingInfo.eScanMode == eSCAN_MODE_INTERLACE))    //A70LV_Doulas_0159 modify
        {
            dvC341_WriteToBuffer(B8_OSYCT2CH1, 0x90, ((ch_v * 2) + 0) * CH_BANK_OFFSET);
            dvC341_WriteToBuffer(B8_OSYCT2CH1, 0x80, ((ch_v * 2) + 1) * CH_BANK_OFFSET);
        }
        else
        {
            dvC341_Write_ch_v(ch_v, B8_OSYCT2CH1, 0x80, CH_BANK_OFFSET);
        }
        #endif

        if(m_sChannelInfo[ch_v].sInputTimingInfo.eScanMode == eSCAN_MODE_NO_SIGNAL)
        {
            dvC341_WriteToBuffer(B8_SYVRSTCTCH1, 0x00, (ch_v * 2) * CH_BANK_OFFSET);
        }
        else if(m_sChannelInfo[ch_v].sInputTimingInfo.eScanMode == eSCAN_MODE_INTERLACE)
        {
            if((dvC341_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_SIDEBYSIDE) ||
               (dvC341_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_FRAMEPACKING) ||
               (dvC341_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_TOPANDBOTTOM))       //H30K_Doulas_0005
            {
                dvC341_WriteToBuffer(B8_SYVRSTCTCH1, 0xF4, (ch_v * 2) * CH_BANK_OFFSET);
            }
            else
            {
                dvC341_WriteToBuffer(B8_SYVRSTCTCH1, 0xF2, (ch_v * 2) * CH_BANK_OFFSET);
            }
        }
        else
        {
            dvC341_WriteToBuffer(B8_SYVRSTCTCH1, 0xF2, (ch_v * 2) * CH_BANK_OFFSET);
        }
    }
    //dvC341_Write(B1_ODACTHWCH12, m_sChannelInfo[eC341_CH_V0].sOutputTimingInfo.uiHActive, 0);        //960
    #if 0
    dvC341_Write_ch_v(ch_v, B144_WPBLKCTCH1, 0x00, CH_WPBANK_OFFSET);//Warping Off
    dvC341_Write_ch_v(ch_v, B144_WPOSYCTCH1, 0x00, CH_WPBANK_OFFSET);
    #endif
    dvC341_Buffer_Flush();

   // dvC341_Write(B1_IDUALCTCH12, 0x21,0);
   // dvC341_Write(B1_IDACTHWCH12, 1280 ,0);
   // dvC341_Write(B1_IDSYNCDLYCH12, 1280 + 4,0);

    #if 0 //H30K_Doulas_0001
    if(m_sChannelInfo[ch_v].bForceSyncReset == FALSE)
    {
        if(ch_v == eC341_CH_V0)
        {
            if(dvC341_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_FRAME_SEQUENTIAL)     //A70LV_Doulas_0154
            {
                //dvC341_Write_ch_v(ch_v, B8_OSYCTCH1, 0x0311, CH_BANK_OFFSET);//Select AOI0 as active output
                dvC341_Write(B8_OSYCTCH1, 0xc003, 0);
                dvC341_Write(B8_OSYCTCH1, 0x1011, CH_BANK_OFFSET);
            }
            else
            {
                //dvC341_Write_ch_v(ch_v, B8_OSYCTCH1, 0x6311, CH_BANK_OFFSET);
                dvC341_Write(B8_OSYCTCH1, 0xc003, 0);
                dvC341_Write(B8_OSYCTCH1, 0x1011, CH_BANK_OFFSET);
            }
        }
        else
        {
            //0x1xxx = Set out sync synchnorized with channel1
            if(dvC341_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_FRAME_SEQUENTIAL)     //A70LV_Doulas_0154
            {
                //dvC341_Write_ch_v(ch_v, B8_OSYCTCH1, 0x1811, CH_BANK_OFFSET);//Select AOI0 as active output
                dvC341_Write(B40_OSYCTCH3, 0xc003 + (ch_v << 9), 0);
                dvC341_Write(B40_OSYCTCH3, 0x1011 + (ch_v << 9), CH_BANK_OFFSET);
            }
            else
            {
                //dvC341_Write_ch_v(ch_v, B8_OSYCTCH1, 0x1811, CH_BANK_OFFSET);
                dvC341_Write(B40_OSYCTCH3, 0xc003 + (ch_v << 9), 0);
                dvC341_Write(B40_OSYCTCH3, 0x1011 + (ch_v << 9), CH_BANK_OFFSET);
            }
        }
        //A70LV_Doulas_0079

        //dvC341_reg_set(B8_OHCYCLCH1, ((m_sChannelInfo[ch_v].sOutputTimingInfo.uiHTotal/OUTPUT4K_CH_NUM) - 2));
        //dvC341_reg_set(B8_OVCYCLCH1, (m_sChannelInfo[ch_v].sOutputTimingInfo.uiVTotal - 2));
        dvC341_Write_ch_v(ch_v, B8_OHCYCLCH1, ((m_sChannelInfo[ch_v].sOutputTimingInfo.uiHTotal/OUTPUT4K_CH_NUM) - 2), CH_BANK_OFFSET);
        dvC341_Write_ch_v(ch_v, B8_OVCYCLCH1, (m_sChannelInfo[ch_v].sOutputTimingInfo.uiVTotal - 2), CH_BANK_OFFSET);

        dvC341_Write_ch_v(ch_v, B8_SYRDLYCH1, 0x00, ucRegOffset);//Forced sync reset delay
        dvC341_Write_ch_v(ch_v, B8_OSYCT3CH1, 0x00, ucRegOffset);//Trigger forced sync reset in unit of HS.
        //dvC341_Buffer_Flush();
    }
    else
    #endif
    {
        if(ch_v == eC341_CH_V0)
        {
            if(dvC341_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_FRAME_SEQUENTIAL) //H30K_Doulas_0005//H30K_Doulas_0002//A70LV_Doulas_0154
            {
                #ifdef FRAME_SEQUENTAIL_SELF_TEST_120HZ //H30K_Doulas_0005
                dvC341_Write(B8_OSYCTCH1, 0xc003, 0);
                #else
                dvC341_Write(B8_OSYCTCH1, 0x0003, 0);
                #endif
                dvC341_Write(B8_OSYCTCH1, 0x1011, ucRegOffset);//Select AOI0 as active output
            }
            else if((dvC341_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_OFF) && (dvC341_IS_3D_Enable() == ets_ON)) //H30K_Doulas_0005
            {
                #ifdef FRAME_SEQUENTAIL_SELF_TEST_60HZ //H30K_Doulas_0005
                dvC341_Write(B8_OSYCTCH1, 0x0083, 0); //H30K_Doulas_0034 0xc003
                #else
                dvC341_Write(B8_OSYCTCH1, 0x0003, 0);
                #endif
                dvC341_Write(B8_OSYCTCH1, 0x1011, ucRegOffset);//Select AOI0 as active output
            }
            else
            {
                dvC341_Write(B8_OSYCTCH1, 0xc003, 0);
                dvC341_Write(B8_OSYCTCH1, 0x1011, ucRegOffset);
            }
        }
        else
        {
            if(dvC341_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_FRAME_SEQUENTIAL) //H30K_Doulas_0005//H30K_Doulas_0002//A70LV_Doulas_0154
            {
                #ifdef FRAME_SEQUENTAIL_SELF_TEST_120HZ //H30K_Doulas_0005
                dvC341_Write(B40_OSYCTCH3, 0xc003 + (ch_v << 9), 0);
                #else
                dvC341_Write(B40_OSYCTCH3, 0x0003 + (ch_v << 9), 0);
                #endif
                dvC341_Write(B40_OSYCTCH3, 0x1011 + (ch_v << 9), ucRegOffset);//Select AOI0 as active output
            }
            else if((dvC341_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_OFF) && (dvC341_IS_3D_Enable() == ets_ON)) //H30K_Doulas_0005
            {
                #ifdef FRAME_SEQUENTAIL_SELF_TEST_60HZ //H30K_Doulas_0005
                dvC341_Write(B40_OSYCTCH3, 0xc003 + (ch_v << 9), 0);
                #else
                dvC341_Write(B40_OSYCTCH3, 0x0003 + (ch_v << 9), 0);
                #endif
                dvC341_Write(B40_OSYCTCH3, 0x1011 + (ch_v << 9), ucRegOffset);//Select AOI0 as active output
            }
            else
            {
                dvC341_Write(B40_OSYCTCH3, 0xc003 + (ch_v << 9), 0);
                dvC341_Write(B40_OSYCTCH3, 0x1011 + (ch_v << 9), ucRegOffset);
            }
        }
        //A70LV_Doulas_0079
        uiForcedSyncResetH_Total = m_sChannelInfo[ch_v].sOutputTimingInfo.uiHTotal;
        uiForcedSyncResetV_Total = m_sChannelInfo[ch_v].sOutputTimingInfo.uiVTotal;
        ucFLDDLY = dvC341_Read(B8_FLDDLYCH1, ucRegOffset);  //Field Delay
        if((ucFLDDLY + 5)  > uiForcedSyncResetV_Total) //avoid " ucSYRDLY > 1 frame "    //H2PF_Simon_0196
        {
            ucFLDDLY = (uiForcedSyncResetV_Total) - 5;
            ucSYRDLY = ucFLDDLY+1;
        }
        else
        {
            ucSYRDLY = ucFLDDLY+1;
        }

       // if(ch_v == eC341_CH_V0)
        {
            if(m_sChannelInfo[ch_v].sInputTimingInfo.eScanMode == eSCAN_MODE_NO_SIGNAL)
            {
                dvC341_Write_ch_v(ch_v, B8_OHCYCLCH1, (uiForcedSyncResetH_Total/OUTPUT4K_CH_NUM - 2), ucRegOffset);
                dvC341_Write_ch_v(ch_v, B8_OVCYCLCH1, (uiForcedSyncResetV_Total - 2), ucRegOffset);

                dvC341_Write_ch_v(ch_v, B8_SYRDLYCH1, 0x00, ucRegOffset);//Forced sync reset delay
                dvC341_Write_ch_v(ch_v, B8_OSYCT3CH1, 0x00, ucRegOffset);//Trigger forced sync reset in unit of HS.

                //dvC341_Write(B8_FLDDLYCH1, 0x00, ucRegOffset);
                bForcedSyncResetEnable = FALSE;

                LOG_MSG(db_DV_SCALER, "dvC341_ConfigOutput_2K1 Total[%d x %d]\n",uiForcedSyncResetH_Total,uiForcedSyncResetV_Total);
            }
            else if(m_sChannelInfo[ch_v].sInputTimingInfo.dFrameRate > 119.0 && m_sChannelInfo[ch_v].sInputTimingInfo.dFrameRate < 120.40)     //A70LV_Doulas_0237 modify
            {
                dvC341_Write_ch_v(ch_v, B8_OHCYCLCH1, (uiForcedSyncResetH_Total/OUTPUT4K_CH_NUM - 2), ucRegOffset);
                dvC341_Write_ch_v(ch_v, B8_OVCYCLCH1, ((uiForcedSyncResetV_Total * 1.1) - 2), ucRegOffset);
                dvC341_Write_ch_v(ch_v, B8_FLDDLYCH1, ucFLDDLY, ucRegOffset);  //Field Delay

                switch(dvC341_Input_3D_Format_Config_Get())        //A70LV_Doulas_0154
                {
                    case eINPUT_3D_TYPE_FRAMEPACKING:
                    case eINPUT_3D_TYPE_SIDEBYSIDE:
                    case eINPUT_3D_TYPE_TOPANDBOTTOM:
                    if(ch_v == eC341_CH_V0)
                    {
                        dvC341_Write(B8_OSYCT3CH1, 0x80, 0);//Trigger forced sync reset in unit of HS.
                        dvC341_Write(B8_OSYCT3CH1, 0x00, ucRegOffset);//Trigger forced sync reset in unit of HS.
                        dvC341_Write_ch_v(ch_v, B8_SYRDLYCH1, ucSYRDLY, ucRegOffset);  //Forced sync reset delay
                    }
                    else
                    {
                        dvC341_Write_ch_v(ch_v, B8_OSYCT3CH1, 0x00, ucRegOffset); //H30K_Doulas_0034//Trigger forced sync reset in unit of HS.
                        dvC341_Write_ch_v(ch_v, B8_SYRDLYCH1, 0x00, ucRegOffset); //H30K_Doulas_0034//Forced sync reset delay
                    }
                    break;

                    case eINPUT_3D_TYPE_FRAME_SEQUENTIAL:
                    default:
                        dvC341_Write_ch_v(ch_v, B8_OSYCT3CH1, 0x80, ucRegOffset);//Trigger forced sync reset in unit of HS.
                        dvC341_Write_ch_v(ch_v, B8_SYRDLYCH1, ucSYRDLY, ucRegOffset);  //Forced sync reset delay
                        break;
                }

                dvC341_Buffer_Flush();
                bForcedSyncResetEnable = TRUE;

                LOG_MSG(db_DV_SCALER, "dvC341_ConfigOutput_2K2 Total[%d x %d]\n",uiForcedSyncResetH_Total,uiForcedSyncResetV_Total);
            }
            else if(m_sChannelInfo[ch_v].sInputTimingInfo.dFrameRate > 99.0 && m_sChannelInfo[ch_v].sInputTimingInfo.dFrameRate < 100.40)  //A70LV_Doulas_0237 modify
            {
                UINT16 uiValue = 0;
                uiForcedSyncResetH_Total = (UINT16)(((UINT32)uiForcedSyncResetH_Total*120)/100);//2400  //A70LV_Doulas_0309 Modify
                uiValue = (uiForcedSyncResetH_Total/OUTPUT4K_CH_NUM)%8;                 //H30K_Doulas_0002
                uiForcedSyncResetH_Total = uiForcedSyncResetH_Total + (8-uiValue)*2;    //H30K_Doulas_0002 get ok h-total

                dvC341_Write_ch_v(ch_v, B8_OHCYCLCH1, (uiForcedSyncResetH_Total/OUTPUT4K_CH_NUM - 2), ucRegOffset);
                dvC341_Write_ch_v(ch_v, B8_OVCYCLCH1, ((uiForcedSyncResetV_Total * 1.1) - 2), ucRegOffset);
                dvC341_Write_ch_v(ch_v, B8_FLDDLYCH1, ucFLDDLY, ucRegOffset);  //Field Delay

                switch(dvC341_Input_3D_Format_Config_Get())        //A70LV_Doulas_0154
                {
                    case eINPUT_3D_TYPE_FRAMEPACKING:
                    case eINPUT_3D_TYPE_SIDEBYSIDE:
                    case eINPUT_3D_TYPE_TOPANDBOTTOM:
                    if(ch_v == eC341_CH_V0)
                    {
                        dvC341_Write(B8_OSYCT3CH1, 0x80, 0);//Trigger forced sync reset in unit of HS.
                        dvC341_Write(B8_OSYCT3CH1, 0x00, ucRegOffset);//Trigger forced sync reset in unit of HS.
                        dvC341_Write_ch_v(ch_v, B8_SYRDLYCH1, ucSYRDLY, ucRegOffset);  //Forced sync reset delay
                    }
                    else
                    {
                        dvC341_Write_ch_v(ch_v, B8_OSYCT3CH1, 0x00, ucRegOffset);//Trigger forced sync reset in unit of HS.
                        dvC341_Write_ch_v(ch_v, B8_SYRDLYCH1, 0x00, ucRegOffset);  //Forced sync reset delay
                    }
                    break;

                    case eINPUT_3D_TYPE_FRAME_SEQUENTIAL:
                    default:
                        dvC341_Write_ch_v(ch_v, B8_OSYCT3CH1, 0x80, ucRegOffset);//Trigger forced sync reset in unit of HS.
                        dvC341_Write_ch_v(ch_v, B8_SYRDLYCH1, ucSYRDLY, ucRegOffset);  //Forced sync reset delay
                        break;
                }
                dvC341_Buffer_Flush();
                bForcedSyncResetEnable = TRUE;

                LOG_MSG(db_DV_SCALER, "dvC341_ConfigOutput_2K3 Total[%d x %d]\n",uiForcedSyncResetH_Total,uiForcedSyncResetV_Total);
            }
            else if(m_sChannelInfo[ch_v].sInputTimingInfo.dFrameRate > 59.50 && m_sChannelInfo[ch_v].sInputTimingInfo.dFrameRate < 60.3)       //A70LV_Doulas_0237 modify//A70LV_Doulas_0154
            {

                dvC341_Write_ch_v(ch_v, B8_OHCYCLCH1, (uiForcedSyncResetH_Total/OUTPUT4K_CH_NUM - 2), ucRegOffset);
                dvC341_Write_ch_v(ch_v, B8_OVCYCLCH1, ((uiForcedSyncResetV_Total * 1.1) - 2), ucRegOffset);
                dvC341_Write_ch_v(ch_v, B8_FLDDLYCH1, ucFLDDLY, ucRegOffset);  //Field Delay

                switch(dvC341_Input_3D_Format_Config_Get())        //A70LV_Doulas_0154
                {
                    case eINPUT_3D_TYPE_FRAMEPACKING:
                    case eINPUT_3D_TYPE_SIDEBYSIDE:
                    case eINPUT_3D_TYPE_TOPANDBOTTOM:
                    if(ch_v == eC341_CH_V0)
                    {
                        dvC341_Write(B8_OSYCT3CH1, 0x80, 0);//Trigger forced sync reset in unit of HS.
                        dvC341_Write(B8_OSYCT3CH1, 0x00, ucRegOffset);//Trigger forced sync reset in unit of HS.
                        dvC341_Write_ch_v(ch_v, B8_SYRDLYCH1, ucSYRDLY, ucRegOffset);  //Forced sync reset delay
                    }
                    else
                    {
                        dvC341_Write_ch_v(ch_v, B8_OSYCT3CH1, 0x00, ucRegOffset);//Trigger forced sync reset in unit of HS.
                        dvC341_Write_ch_v(ch_v, B8_SYRDLYCH1, 0x00, ucRegOffset);  //Forced sync reset delay
                    }
                    break;

                    case eINPUT_3D_TYPE_FRAME_SEQUENTIAL:
                    default:
                        //dvC341_Write_ch_v(ch_v, B8_OSYCT3CH1, 0x00, ucRegOffset); //H30K_Doulas_0034//Trigger forced sync reset in unit of HS.
                        if(m_sChannelInfo[ch_v].sInputTimingInfo.eScanMode == eSCAN_MODE_INTERLACE) //H30K_Doulas_0034
                        {
                            dvC341_WriteToBuffer(B8_SYVRSTCTCH1, 0xF4, (ch_v * 2) * CH_BANK_OFFSET);
                        }
                        dvC341_Write(B8_OSYCT3CH1, 0x80, 0); //H30K_Doulas_0034
                        dvC341_Write_ch_v(ch_v, B8_SYRDLYCH1, ucSYRDLY, ucRegOffset);  //Forced sync reset delay
                        break;
                }
                dvC341_Buffer_Flush();
                bForcedSyncResetEnable = TRUE;

                LOG_MSG(db_DV_SCALER, "dvC341_ConfigOutput_2K2 Total[%d x %d]\n",uiForcedSyncResetH_Total,uiForcedSyncResetV_Total);
            }
            /*else if(m_sChannelInfo[ch_v].sInputTimingInfo.dFrameRate >= 60.3 && m_sChannelInfo[ch_v].sInputTimingInfo.dFrameRate < 60.4)  //H30K_Doulas_0002 remove//A70LV_Doulas_0237
            {
                if(m_sChannelInfo[eC341_CH_V0].ucPanelIndex == ePANEL_ID_WUXGA_120HZ)
                {
                    uiForcedSyncResetH_Total = (uiForcedSyncResetH_Total*120)/120.7;
                }
                else if(m_sChannelInfo[eC341_CH_V0].ucPanelIndex == ePANEL_ID_1080P_120HZ)
                {
                    uiForcedSyncResetH_Total = (uiForcedSyncResetH_Total*120)/120.7;
                }

                dvC341_Write_ch_v(ch_v, B8_OHCYCLCH1, (uiForcedSyncResetH_Total/OUTPUT4K_CH_NUM - 2), ucRegOffset);
                dvC341_Write_ch_v(ch_v, B8_OVCYCLCH1, ((uiForcedSyncResetV_Total * 1.1) - 2), ucRegOffset);
                dvC341_Write_ch_v(ch_v, B8_FLDDLYCH1, ucFLDDLY, ucRegOffset);  //Field Delay

                switch(dvC341_Input_3D_Format_Config_Get())        //A70LV_Doulas_0154
                {
                    case eINPUT_3D_TYPE_FRAMEPACKING:
                    case eINPUT_3D_TYPE_SIDEBYSIDE:
                    case eINPUT_3D_TYPE_TOPANDBOTTOM:
                    if(ch_v == eC341_CH_V0)
                    {
                        dvC341_Write(B8_OSYCT3CH1, 0x80, 0);//Trigger forced sync reset in unit of HS.
                        dvC341_Write(B8_OSYCT3CH1, 0x00, ucRegOffset);//Trigger forced sync reset in unit of HS.
                        dvC341_Write_ch_v(ch_v, B8_SYRDLYCH1, ucSYRDLY, ucRegOffset);  //Forced sync reset delay
                    }
                    else
                    {
                        dvC341_Write_ch_v(ch_v, B8_OSYCT3CH1, 0x00, ucRegOffset);//Trigger forced sync reset in unit of HS.
                        dvC341_Write_ch_v(ch_v, B8_SYRDLYCH1, 0x00, ucRegOffset);  //Forced sync reset delay
                    }
                    break;

                    case eINPUT_3D_TYPE_FRAME_SEQUENTIAL:
                    default:
                        dvC341_Write_ch_v(ch_v, B8_OSYCT3CH1, 0x00, ucRegOffset);//Trigger forced sync reset in unit of HS.
                        dvC341_Write_ch_v(ch_v, B8_SYRDLYCH1, ucSYRDLY, ucRegOffset);  //Forced sync reset delay
                        break;
                }
                dvC341_Buffer_Flush();
                bForcedSyncResetEnable = TRUE;

                LOG_MSG(db_DV_SCALER, "dvC341_ConfigOutput_2K2 Total[%d x %d]\n",uiForcedSyncResetH_Total,uiForcedSyncResetV_Total);
            }*/
            else if(m_sChannelInfo[ch_v].sInputTimingInfo.dFrameRate > 49.50 && m_sChannelInfo[ch_v].sInputTimingInfo.dFrameRate < 50.30)       //A70LV_Doulas_0237 modify//A70LV_Doulas_0154
            {
            #if 0  //test
                dvC341_WriteToBuffer(B8_OSYCTCH1, 0x1011 + (ch_v << 9), ch_v * 2 * CH_BANK_OFFSET);
                dvC341_WriteToBuffer(B8_SYVRSTCTCH1, 0x00, (ch_v * 2) * CH_BANK_OFFSET);


                dvC341_Write_ch_v(ch_v, B8_OHCYCLCH1, (uiForcedSyncResetH_Total/OUTPUT4K_CH_NUM - 2), ucRegOffset);
                dvC341_Write_ch_v(ch_v, B8_OVCYCLCH1, ((uiForcedSyncResetV_Total) - 2), ucRegOffset);
                dvC341_Write_ch_v(ch_v, B8_FLDDLYCH1, ucFLDDLY, ucRegOffset);
            #else
                UINT16 uiValue = 0;
                uiForcedSyncResetH_Total = (UINT16)(((UINT32)uiForcedSyncResetH_Total * 120)/100);

                uiValue = (uiForcedSyncResetH_Total/OUTPUT4K_CH_NUM)%8;
                uiForcedSyncResetH_Total = uiForcedSyncResetH_Total + (8-uiValue)*2;

                dvC341_Write_ch_v(ch_v, B8_OHCYCLCH1, (uiForcedSyncResetH_Total/OUTPUT4K_CH_NUM - 2), ucRegOffset);
                dvC341_Write_ch_v(ch_v, B8_OVCYCLCH1, ((uiForcedSyncResetV_Total * 1.1) - 2), ucRegOffset);
                dvC341_Write_ch_v(ch_v, B8_FLDDLYCH1, ucFLDDLY, ucRegOffset);  //Field Delay
            #endif
                switch(dvC341_Input_3D_Format_Config_Get())        //A70LV_Doulas_0154
                {
                    case eINPUT_3D_TYPE_FRAMEPACKING:
                    case eINPUT_3D_TYPE_SIDEBYSIDE:
                    case eINPUT_3D_TYPE_TOPANDBOTTOM:
                    if(ch_v == eC341_CH_V0)
                    {
                        dvC341_Write(B8_OSYCT3CH1, 0x80, 0);//Trigger forced sync reset in unit of HS.
                        dvC341_Write(B8_OSYCT3CH1, 0x00, ucRegOffset);//Trigger forced sync reset in unit of HS.
                        dvC341_Write_ch_v(ch_v, B8_SYRDLYCH1, ucSYRDLY, ucRegOffset);  //Forced sync reset delay
                    }
                    else
                    {
                        dvC341_Write_ch_v(ch_v, B8_OSYCT3CH1, 0x00, ucRegOffset);//Trigger forced sync reset in unit of HS.
                        dvC341_Write_ch_v(ch_v, B8_SYRDLYCH1, 0x00, ucRegOffset);  //Forced sync reset delay
                    }
                    break;

                    case eINPUT_3D_TYPE_FRAME_SEQUENTIAL:
                    default:
                        //dvC341_Write_ch_v(ch_v, B8_OSYCT3CH1, 0x00, ucRegOffset); //H30K_Doulas_0034//Trigger forced sync reset in unit of HS.
                        if(m_sChannelInfo[ch_v].sInputTimingInfo.eScanMode == eSCAN_MODE_INTERLACE) //H30K_Doulas_0034
                        {
                            dvC341_WriteToBuffer(B8_SYVRSTCTCH1, 0xF4, (ch_v * 2) * CH_BANK_OFFSET);
                        }
                        dvC341_Write(B8_OSYCT3CH1, 0x80, 0); //H30K_Doulas_0034
                        dvC341_Write_ch_v(ch_v, B8_SYRDLYCH1, ucSYRDLY, ucRegOffset);  //Forced sync reset delay
                        break;
                }
                dvC341_Buffer_Flush();
                bForcedSyncResetEnable = TRUE;

                LOG_MSG(db_DV_SCALER, "dvC341_ConfigOutput_2K3 Total[%d x %d]\n",uiForcedSyncResetH_Total,uiForcedSyncResetV_Total);
            }
            else if(m_sChannelInfo[ch_v].sInputTimingInfo.dFrameRate > 23.50 && m_sChannelInfo[ch_v].sInputTimingInfo.dFrameRate < 24.30)       //A70LV_Doulas_0237 modify//A70LV_Doulas_0154
            {
                //uiForcedSyncResetH_Total = 1250;    //A70LV_Doulas_0265 remove

                dvC341_Write_ch_v(ch_v, B8_OHCYCLCH1, (uiForcedSyncResetH_Total/OUTPUT4K_CH_NUM - 2), ucRegOffset);
                dvC341_Write_ch_v(ch_v, B8_OVCYCLCH1, ((uiForcedSyncResetV_Total * 1.1) - 2), ucRegOffset);
                dvC341_Write_ch_v(ch_v, B8_FLDDLYCH1, ucFLDDLY, ucRegOffset);  //Field Delay

                switch(dvC341_Input_3D_Format_Config_Get())        //A70LV_Doulas_0154
                {
                    case eINPUT_3D_TYPE_FRAMEPACKING:
                    case eINPUT_3D_TYPE_SIDEBYSIDE:
                    case eINPUT_3D_TYPE_TOPANDBOTTOM:
                    if(ch_v == eC341_CH_V0)
                    {
                        dvC341_Write(B8_OSYCT3CH1, 0x80, 0);//Trigger forced sync reset in unit of HS.
                        dvC341_Write(B8_OSYCT3CH1, 0x00, ucRegOffset);//Trigger forced sync reset in unit of HS.
                        dvC341_Write_ch_v(ch_v, B8_SYRDLYCH1, ucSYRDLY, ucRegOffset);  //Forced sync reset delay
                    }
                    else
                    {
                        dvC341_Write_ch_v(ch_v, B8_OSYCT3CH1, 0x00, ucRegOffset);//Trigger forced sync reset in unit of HS.
                        dvC341_Write_ch_v(ch_v, B8_SYRDLYCH1, 0x00, ucRegOffset);  //Forced sync reset delay
                    }
                    break;

                    case eINPUT_3D_TYPE_FRAME_SEQUENTIAL:
                    default:
                        //dvC341_Write_ch_v(ch_v, B8_OSYCT3CH1, 0x00, ucRegOffset); //H30K_Doulas_0034//Trigger forced sync reset in unit of HS.
                        dvC341_Write(B8_OSYCT3CH1, 0x80, 0); //H30K_Doulas_0034
                        dvC341_Write_ch_v(ch_v, B8_SYRDLYCH1, ucSYRDLY, ucRegOffset);  //Forced sync reset delay
                        break;
                }
                //dvC341_Write(B2_SYVRSTCTCH1, 0xF4, 0);    //A70LV_Doulas_0265 remove
                dvC341_WriteToBuffer(B8_SYVRSTCTCH1, 0xF5, (ch_v * 2) * CH_BANK_OFFSET);
                dvC341_Buffer_Flush();

                bForcedSyncResetEnable = TRUE;

                LOG_MSG(db_DV_SCALER, "dvC341_ConfigOutput_2K4 Total[%d x %d]\n",uiForcedSyncResetH_Total,uiForcedSyncResetV_Total);
            }
            else //no frame lock
            {
                //dvC789_ForcedSyncResetValueSet(1,uiForcedSyncResetH_Total,uiForcedSyncResetV_Total);
                dvC341_WriteToBuffer(B8_SYVRSTCTCH1, 0x00, (ch_v * 2) * CH_BANK_OFFSET);    //H30K_Doulas_0002
                if(ch_v == eC341_CH_V0)         //H30K_Doulas_0002
                {
                    dvC341_Write(B8_OSYCTCH1, 0xc001, 0);
                    dvC341_Write(B8_OSYCTCH1, 0x1011, ucRegOffset);
                }
                else
                {
                    dvC341_Write(B40_OSYCTCH3, 0xc001 + (ch_v << 9), 0);
                    dvC341_Write(B40_OSYCTCH3, 0x1011 + (ch_v << 9), ucRegOffset);
                }

                dvC341_Write_ch_v(ch_v, B8_OHCYCLCH1, (uiForcedSyncResetH_Total/OUTPUT4K_CH_NUM - 2), ucRegOffset); //H30K_Doulas_0002
                dvC341_Write_ch_v(ch_v, B8_OVCYCLCH1, (uiForcedSyncResetV_Total - 2), ucRegOffset);

                dvC341_Write_ch_v(ch_v, B8_SYRDLYCH1, 0x00, ucRegOffset);//Forced sync reset delay
                dvC341_Write_ch_v(ch_v, B8_OSYCT3CH1, 0x00, ucRegOffset);//Trigger forced sync reset in unit of HS.
                dvC341_Write_ch_v(ch_v, B8_OSYCT2CH1, 0x00, CH_BANK_OFFSET);   //H30K_Doulas_0002
                //dvC341_Write(B8_FLDDLYCH1, 0x00, ucRegOffset);
                dvC341_Buffer_Flush();
                bForcedSyncResetEnable = FALSE;

                LOG_MSG(db_DV_SCALER, "dvC341_ConfigOutput_2K5 Total[%d x %d]\n",uiForcedSyncResetH_Total,uiForcedSyncResetV_Total);
            }
        }

    }

    dvC341_Write(B3_OSYCTTX1, 0x09, 0);
    dvC341_Write(B3_OHSCTTX1, m_sChannelInfo[ch_v].sOutputTimingInfo.ucHSyncWidth/OUTPUT_CH_NUM/2-1, 0);//21
    dvC341_Write(B3_OVSCTTX1, m_sChannelInfo[ch_v].sOutputTimingInfo.ucVSyncWidth -1, 0);//4
    dvC341_Write(B3_OVDLYTX1, 0x01, 0);
    dvC341_Write(B3_VBOCHSEL, 0x01000100, 0);  //PS_VBOCHSEL
    //dvC341_Write(B3_LANESELTX1, 0x73625140, 0); //H30K_Doulas_0001
    //dvC341_Write(B3_LANESELTX2, 0x73625140, 0);
    dvC341_Write(B3_LANESELTX1, 0x04152637, 0);
    dvC341_Write(B3_LANESELTX2, 0x04152637, 0);

    if(ch_v == eC341_CH_V0) //H30K_Doulas_0023
    {  //H30K_Doulas_0001
    #if 0//H30K_Doulas_0034 remove
        dvC341_Write(B0_PIDSKW, 0x00, 0);
#ifdef SWAP_VBO_RX0_LANE
        dvC341_Write(B0_VI1CT, 0x0404 | 0xC8C8, 0);
#else
        dvC341_Write(B0_VI1CT, 0x0404, 0);
#endif
        dvC341_Write(B0_VI2CT, 0x0011, 0);
        dvC341_Write(B0_VIDSKW, 0x0402, 0);
    #endif
        if((dvC341_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_FRAMEPACKING) ||
           (dvC341_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_TOPANDBOTTOM) ||
           (dvC341_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_SIDEBYSIDE))     //H30K_Doulas_0033 fixed image fail
        {
            if(m_sChannelInfo[ch_v].ucInputPort == eC341_INPUT_PORT3) //H30K_Doulas_0076 Modify
                dvC341_Write(B0_ICHSEL2, 0x0C, 0);
            else
                dvC341_Write(B0_ICHSEL2, 0x08, 0);
        }
    }
    //dvC341_Write(B8_FLDDLYCH1, 0x30, ucRegOffset);      //A70LV_Doulas_0003 remove

    //dvC341_Write(B3_OSYCTTX2, 0x19, 0);
    //dvC341_Write(B3_OHSCTTX2, ((m_sChannelInfo[ch_v].sOutputTimingInfo.ucHSyncWidth / OUTPUT_CH_NUM) / 2) - 1, 0);
    //dvC341_Write(B3_OVSCTTX2, m_sChannelInfo[ch_v].sOutputTimingInfo.ucVSyncWidth - 1, 0);
    //dvC341_Write(B3_VBOCHSEL, 0x01000100, 0);  //PS_VBOCHSEL
    //dvC341_Write(B3_LANESELTX2, 0x73625140, 0);  //PS_LANESELTX2
    //dvC341_Write(B3_LANESELTX2, 0x04152637, 0);

    dvC341_Write_ch_v(ch_v, B8_OAOI0HSTCH1, m_sChannelInfo[ch_v].sOutputTimingInfo.uiHStart / OUTPUT_CH_NUM, ucRegOffset); //192 + 480
    dvC341_Write_ch_v(ch_v, B8_OAOI0HENDCH1, (m_sChannelInfo[ch_v].sOutputTimingInfo.uiHStart / OUTPUT_CH_NUM +
                                   m_sChannelInfo[ch_v].sOutputTimingInfo.uiHActive / OUTPUT_CH_NUM), ucRegOffset);
    dvC341_Write_ch_v(ch_v, B8_OAOI0VSTCH1, m_sChannelInfo[ch_v].sOutputTimingInfo.uiVStart - 2, ucRegOffset);  //42 + 1080
    dvC341_Write_ch_v(ch_v, B8_OAOI0VENDCH1, (m_sChannelInfo[ch_v].sOutputTimingInfo.uiVStart - 2 +
                                   m_sChannelInfo[ch_v].sOutputTimingInfo.uiVActive), ucRegOffset);

    //dvC341_Write(B8_OAOI1HSTCH1, m_sChannelInfo[ch_v].sOutputTimingInfo.uiHStart/*PS_PANEL_HST*/, ucRegOffset);//192 + 480
    //dvC341_Write(B8_OAOI1HENDCH1, (m_sChannelInfo[ch_v].sOutputTimingInfo.uiHStart +
    //                               m_sChannelInfo[ch_v].sOutputTimingInfo.uiHActive /*PS_PANEL_HST + PS_PANEL_1CH_HW*/), ucRegOffset);
    //dvC341_Write(B8_OAOI1VSTCH1, m_sChannelInfo[ch_v].sOutputTimingInfo.uiVStart - 5/*PS_PANEL_VST*/, ucRegOffset);//42 + 1080
    //dvC341_Write(B8_OAOI1VENDCH1, (m_sChannelInfo[ch_v].sOutputTimingInfo.uiVStart - 5 +
    //                               m_sChannelInfo[ch_v].sOutputTimingInfo.uiVActive/*PS_PANEL_VST + PS_PANEL_VW*/), ucRegOffset);

    //dvC341_Write(B0_CMOSOUTSETUP, 0x40, ucRegOffset);   //A70LV_Doulas_0079 //POHS, POVS, POACT, POFLD output enable ,CH1 output
    dvC341_Write(B0_TTLPOSYCT, 0x0040, 0);  //POHS, POVS, POACT, POFLD output enable ,CH1 output

	if(m_sChannelInfo[ch_v].sInputTimingInfo.eScanMode == eSCAN_MODE_NO_SIGNAL)	//A70LV_Doulas_0004 modify
        m_sChannelInfo[ch_v].ucOIMGCT |= 0x01;
    else
        m_sChannelInfo[ch_v].ucOIMGCT &= 0xFE;

    if(dvC341_Read(B8_OIMGCTCH1, (ch_v * 2) * CH_BANK_OFFSET) != m_sChannelInfo[ch_v].ucOIMGCT)
        dvC341_Write_ch_v(ch_v, B8_OIMGCTCH1, m_sChannelInfo[ch_v].ucOIMGCT, CH_BANK_OFFSET);
    dvC341_Buffer_Flush();
#endif
}

static void dvC341_ConfigOutput_4K(const eC341_CH_V ch_v)//dvC341_SetupOutput
{
    if(ch_v >= eC341_CH_VNUMBER)
    {
        return;
    }

    UINT8 ucRegOffset = 0;
    UINT16 ucSYRDLY,ucFLDDLY;   //H30K_Doulas_0010//A70LV_Doulas_0079
    UINT8 ch_v_RegOffset = (ch_v * CH_V_BANK_OFFSET);
    UINT32 ForcedSyncRstRegBIT = (BIT1);

    if((m_sChannelInfo[eC341_CH_V0].ePanelTimingId == ePANEL_ID_1080P_120HZ) ||
       (m_sChannelInfo[eC341_CH_V0].ePanelTimingId == ePANEL_ID_WUXGA_120HZ))   //A70LV_Doulas_0377 Modify//A70LV_Doulas_0155 //A70LV_Doulas_0154 modify //A70LV_Doulas_0098
    {
        dvC341_ConfigOutput_2K_3D(ch_v);   //simon check 3D
        return;
    }
    else
    {
        dvC341_WriteToBuffer(B3_OIDUALCTCH12, 0x05, 0);    //PS_OIDUALCTCH12
        dvC341_WriteToBuffer(B3_OIDUALCTCH34, 0x05, 0);    //PS_OIDUALCTCH34
        dvC341_WriteToBuffer(B3_ODACTHWCH12, (m_sChannelInfo[ch_v].sOutputTimingInfo.uiHActive / OUTPUT4K_CH_NUM), 0);
        dvC341_WriteToBuffer(B3_ODACTHWCH34, (m_sChannelInfo[ch_v].sOutputTimingInfo.uiHActive / OUTPUT4K_CH_NUM), 0);
        dvC341_WriteToBuffer(B8_SYVRSTCTCH1, 0x00, 0);     //H30K_Doulas_0001
        dvC341_Write_ch_v(ch_v, B16_AUTOFLDCH1, 0x80, CH_BANK_OFFSET); //H30K_Doulas_0001
        //dvC341_WriteToBuffer(B1_TESTG, 0x00, 0);            //A70LV_Doulas_0154
        //dvC341_WriteToBuffer(B2_SYVRSTCTCH1, 0x00, 0);      //A70LV_Doulas_0154
        //dvC341_Write(B30_WPBLKCT, 0x03,0);//Warping Off
    }

    /*if(m_sChannelInfo[ch_v].sInputTimingInfo.eScanMode == eSCAN_MODE_INTERLACE) //H30K_Doulas_0001
    {
        dvC341_WriteToBuffer(B8_OSYCT2CH1, 0x10, ((ch_v * 2) + 0) * CH_BANK_OFFSET);
        dvC341_WriteToBuffer(B8_OSYCT2CH1, 0x00, ((ch_v * 2) + 1) * CH_BANK_OFFSET);
    }
    else*/
    {
        dvC341_Write_ch_v(ch_v, B8_OSYCT2CH1, 0x00, CH_BANK_OFFSET);  //H30K_Doulas_0001
    }
    dvC341_Buffer_Flush();

    if(m_sChannelInfo[ch_v].bForceSyncReset == FALSE)
    {
        uiForcedSyncResetH_Total = m_sChannelInfo[ch_v].sOutputTimingInfo.uiHTotal; //H30K_Doulas_0009
        uiForcedSyncResetV_Total = m_sChannelInfo[ch_v].sOutputTimingInfo.uiVTotal; //H30K_Doulas_0009
        bForcedSyncResetEnable = FALSE;                                             //H30K_Doulas_0010//H30K_Doulas_0009

        if(ch_v == eC341_CH_V0)
        {
            //dvC341_Write(B8_OSYCTCH1,  0x0001, ch_v_RegOffset);//Select AOI0 as active output   //simon check
            //dvC341_Write(B24_OSYCTCH2, 0x1011, ch_v_RegOffset);                                 //simon check
        }
        else
        {
            //0x1xxx = Set out sync synchnorized with channel1
            dvC341_Write(B8_OSYCTCH1,  0x0011 + (ch_v << 9), ch_v_RegOffset);    //simon check
            dvC341_Write(B24_OSYCTCH2, 0x1011 + (ch_v << 9), ch_v_RegOffset);    //simon check
            dvC341_Write_ch_v(ch_v, B8_OHCYCLCH1, (uiForcedSyncResetH_Total/OUTPUT4K_CH_NUM - 2) , CH_BANK_OFFSET);
            dvC341_Write_ch_v(ch_v, B8_OVCYCLCH1, (uiForcedSyncResetV_Total - 2), CH_BANK_OFFSET);
            dvC341_Write_ch_v(ch_v, B8_SYRDLYCH1, 0x00, CH_BANK_OFFSET);//Forced sync reset delay
            dvC341_Write_ch_v(ch_v, B8_OSYCT3CH1, VAL_B8_OSYCT3CH1, CH_BANK_OFFSET);//Trigger forced sync reset in unit of HS.
        }

        ucFLDDLY = dvC341_Read(B8_FLDDLYCH1, ucRegOffset);  //H30K_Doulas_0010//Field Delay
        ucSYRDLY = ucFLDDLY+1;                              //H30K_Doulas_0010
        LOG_MSG(db_DV_SCALER,"#Low_Latency[%d] %d,Delay %d\r\n",ch_v,m_sChannelSetting[ch_v].ucLow_Latency,ucSYRDLY); //H30K_Doulas_0010

        if((m_sChannelInfo[eC341_CH_V0].ePanelTimingId == ePANEL_ID_1080P_240HZ) ||
            (m_sChannelInfo[eC341_CH_V0].ePanelTimingId == ePANEL_ID_WUXGA_240HZ)) //H30K_Doulas_0009
        {
            #if 0     //H2PF_Simon_0107 enable warping block for WUXGA240Hz panel
            dvC341_Write_ch_v(ch_v, B144_WPBLKCTCH1, 0x00, CH_WPBANK_OFFSET); //Warping Off
            dvC341_Write_ch_v(ch_v, B144_WPOSYCTCH1, 0x00, CH_WPBANK_OFFSET);
            #endif

            if(ch_v == eC341_CH_V0)
            {
                dvC341_Write_ch_v(ch_v, B22_ISLFRZCTCH1, 0x01, CH_BANK_OFFSET);  //H30K_Doulas_0010
                if((m_sChannelInfo[ch_v].sInputTimingInfo.eScanMode == eSCAN_MODE_NO_SIGNAL) ||
                   (m_sChannelSetting[eC341_CH_V0].cPIP_PBP_ON))    //H30K_Doulas_0010
                {
                    dvC341_Write(B8_OSYCTCH1,  0x0001, ch_v_RegOffset);//Select AOI0 as active output   //simon check
                    dvC341_Write(B24_OSYCTCH2, 0x1011, ch_v_RegOffset);                                 //simon check
                    dvC341_Write_ch_v(ch_v, B8_OHCYCLCH1, (uiForcedSyncResetH_Total/OUTPUT4K_CH_NUM - 2) , CH_BANK_OFFSET);
                    dvC341_Write_ch_v(ch_v, B8_OVCYCLCH1, (uiForcedSyncResetV_Total - 2), CH_BANK_OFFSET);

                    dvC341_Write_ch_v(ch_v, B8_SYRDLYCH1, 0, CH_BANK_OFFSET);  //Forced sync reset delay
                    dvC341_Write_ch_v(ch_v, B8_FLDDLYCH1, 0, CH_BANK_OFFSET);  //Field Delay
                    dvC341_Write_ch_v(ch_v, B8_OSYCT2CH1, 0, CH_BANK_OFFSET);
                    dvC341_Write_ch_v(ch_v, B8_OSYCT3CH1, VAL_B8_OSYCT3CH1    , CH_BANK_OFFSET);//Trigger forced sync reset in unit of HS.
                    LOG_MSG(db_DV_SCALER, "dvC341_ConfigOutput_2K240-0 no signal or PIP on\r\n");
                }
                else if((m_sChannelInfo[ch_v].sInputTimingInfo.dFrameRate > 239.50 && m_sChannelInfo[ch_v].sInputTimingInfo.dFrameRate < 240.3) &&
                        (m_sChannelSetting[ch_v].ucLow_Latency) &&
                        (m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.eScanMode == eSCAN_MODE_PROGRESSIVE))       //H30K_Doulas_0010
                {
                    dvC341_Write(B8_OSYCTCH1,  0x0003, ch_v_RegOffset);//Select AOI0 as active output
                    dvC341_Write(B24_OSYCTCH2, 0x1013, ch_v_RegOffset);
                    dvC341_reg_set(B8_OHCYCLCH1, (uiForcedSyncResetH_Total/OUTPUT_CH_NUM - 2));
                    dvC341_reg_set(B8_OVCYCLCH1, ((uiForcedSyncResetV_Total * 1.1) - 2));

                    dvC341_Write_ch_v(ch_v, B8_SYRDLYCH1, ucSYRDLY, CH_BANK_OFFSET);  //Forced sync reset delay
                    dvC341_Write_ch_v(ch_v, B8_FLDDLYCH1, ucFLDDLY, CH_BANK_OFFSET);  //Field Delay
                    dvC341_Write_ch_v(ch_v, B8_OSYCT3CH1, 0x80    , CH_BANK_OFFSET);//Trigger forced sync reset in unit of HS.
                    bForcedSyncResetEnable = TRUE;

                    dvC341_Write_ch_v(ch_v, B22_ISLFRZCTCH1, 0x00, CH_BANK_OFFSET); //H30K_Doulas_0010
                    LOG_MSG(db_DV_SCALER, "dvC341_ConfigOutput_2K240-1  Total[%d x %d]\n",uiForcedSyncResetH_Total,uiForcedSyncResetV_Total);
                }
                else if((m_sChannelInfo[ch_v].sInputTimingInfo.dFrameRate > 119.50 && m_sChannelInfo[ch_v].sInputTimingInfo.dFrameRate < 120.3) &&
                        (m_sChannelSetting[ch_v].ucLow_Latency) &&
                        (m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.eScanMode == eSCAN_MODE_PROGRESSIVE))  //H30K_Doulas_0010
                {
                    dvC341_Write(B8_OSYCTCH1,  0x0003, ch_v_RegOffset);//Select AOI0 as active output
                    dvC341_Write(B24_OSYCTCH2, 0x1013, ch_v_RegOffset);
                    dvC341_reg_set(B8_OHCYCLCH1, (uiForcedSyncResetH_Total/OUTPUT_CH_NUM - 2));
                    dvC341_reg_set(B8_OVCYCLCH1, ((uiForcedSyncResetV_Total * 1.1) - 2));
                    dvC341_WriteToBuffer(B8_SYVRSTCTCH1, 0xF2, (ch_v * 2) * CH_BANK_OFFSET);

                    dvC341_In120_Out240_CalFunction(ch_v,&ucSYRDLY,&ucFLDDLY); //H30K_Doulas_0012

                    //dvC341_Write_ch_v(ch_v, B8_SYRDLYCH1, 0x20    , CH_BANK_OFFSET);  //H30K_Doulas_0012 //Forced sync reset delay
                    dvC341_Write_ch_v(ch_v, B8_SYRDLYCH1, ucSYRDLY, CH_BANK_OFFSET);  //Forced sync reset delay
                    dvC341_Write_ch_v(ch_v, B8_FLDDLYCH1, ucFLDDLY, CH_BANK_OFFSET);  //Field Delay
                    dvC341_Write_ch_v(ch_v, B8_OSYCT3CH1, 0x80    , CH_BANK_OFFSET);//Trigger forced sync reset in unit of HS.
                    bForcedSyncResetEnable = TRUE;

                    dvC341_Write_ch_v(ch_v, B22_ISLFRZCTCH1, 0x00, CH_BANK_OFFSET); //H30K_Doulas_0010
                    LOG_MSG(db_DV_SCALER, "dvC341_ConfigOutput_2K240-2  Total[%d x %d]\n",uiForcedSyncResetH_Total,uiForcedSyncResetV_Total);
                }
                else
                {
                    dvC341_Write(B8_OSYCTCH1,  0x0001, ch_v_RegOffset);//Select AOI0 as active output   //simon check
                    dvC341_Write(B24_OSYCTCH2, 0x1011, ch_v_RegOffset);                                 //simon check
                    dvC341_Write_ch_v(ch_v, B8_OHCYCLCH1, (uiForcedSyncResetH_Total/OUTPUT4K_CH_NUM - 2) , CH_BANK_OFFSET);
                    dvC341_Write_ch_v(ch_v, B8_OVCYCLCH1, (uiForcedSyncResetV_Total - 2), CH_BANK_OFFSET);

                    dvC341_Write_ch_v(ch_v, B8_SYRDLYCH1, 0, CH_BANK_OFFSET);  //Forced sync reset delay
                    //dvC341_Write_ch_v(ch_v, B8_FLDDLYCH1, 0, CH_BANK_OFFSET);  //Field Delay
                    dvC341_Write_ch_v(ch_v, B8_OSYCT2CH1, 0, CH_BANK_OFFSET);
                    dvC341_Write_ch_v(ch_v, B8_OSYCT3CH1, VAL_B8_OSYCT3CH1    , CH_BANK_OFFSET);//Trigger forced sync reset in unit of HS.
                    bForcedSyncResetEnable = FALSE;

                    LOG_MSG(db_DV_SCALER, "dvC341_ConfigOutput_2K240-3  Total[%d x %d]\n",uiForcedSyncResetH_Total,uiForcedSyncResetV_Total);
                }
            }
        }
        else
        {
            if(ch_v == eC341_CH_V0) //H30K_Doulas_0010
            {
                dvC341_Write_ch_v(ch_v, B22_ISLFRZCTCH1, 0x01, CH_BANK_OFFSET);
                if((m_sChannelInfo[ch_v].sInputTimingInfo.eScanMode == eSCAN_MODE_NO_SIGNAL) ||
                   (m_sChannelSetting[eC341_CH_V0].cPIP_PBP_ON))
                {
                    dvC341_Write(B8_OSYCTCH1,  0x0001, ch_v_RegOffset);//Select AOI0 as active output   //simon check
                    dvC341_Write(B24_OSYCTCH2, 0x1011, ch_v_RegOffset);                                 //simon check
                    dvC341_Write_ch_v(ch_v, B8_OHCYCLCH1, (uiForcedSyncResetH_Total/OUTPUT4K_CH_NUM - 2) , CH_BANK_OFFSET);
                    dvC341_Write_ch_v(ch_v, B8_OVCYCLCH1, (uiForcedSyncResetV_Total - 2), CH_BANK_OFFSET);

                    dvC341_Write_ch_v(ch_v, B8_SYRDLYCH1, 0, CH_BANK_OFFSET);  //Forced sync reset delay
                    dvC341_Write_ch_v(ch_v, B8_FLDDLYCH1, 0, CH_BANK_OFFSET);  //Field Delay
                    dvC341_Write_ch_v(ch_v, B8_OSYCT2CH1, 0, CH_BANK_OFFSET);
                    dvC341_Write_ch_v(ch_v, B8_OSYCT3CH1, VAL_B8_OSYCT3CH1    , CH_BANK_OFFSET);//Trigger forced sync reset in unit of HS.
                    LOG_MSG(db_DV_SCALER, "dvC341_ConfigOutput_4K60-0 no signal or PIP on\r\n");
                }
                else if((m_sChannelInfo[ch_v].sInputTimingInfo.dFrameRate > 59.50 && m_sChannelInfo[ch_v].sInputTimingInfo.dFrameRate < 60.3) &&
                        (m_sChannelSetting[ch_v].ucLow_Latency) &&
                        (m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.eScanMode == eSCAN_MODE_PROGRESSIVE))
                {
                    dvC341_Write(B8_OSYCTCH1,  0x0003, ch_v_RegOffset);//Select AOI0 as active output
                    dvC341_Write(B24_OSYCTCH2, 0x1013, ch_v_RegOffset);
                    dvC341_reg_set(B8_OHCYCLCH1, (uiForcedSyncResetH_Total/OUTPUT_CH_NUM - 2));
                    dvC341_reg_set(B8_OVCYCLCH1, ((uiForcedSyncResetV_Total * 1.1) - 2)); //H30K_Doulas_0012

                    dvC341_Write_ch_v(ch_v, B8_SYRDLYCH1, ucSYRDLY, CH_BANK_OFFSET);  //Forced sync reset delay
                    dvC341_Write_ch_v(ch_v, B8_FLDDLYCH1, ucFLDDLY, CH_BANK_OFFSET);  //Field Delay
                    //dvC341_Write_ch_v(ch_v, B8_OSYCT3CH1, 0x80    , CH_BANK_OFFSET);//Trigger forced sync reset in unit of HS.
                //    dvC341_Write_ch_v(ch_v, B8_OSYCT2CH1, 0x80    , CH_BANK_OFFSET); //H30K_Doulas_0012
                    bForcedSyncResetEnable = TRUE;

                    dvC341_Write_ch_v(ch_v, B22_ISLFRZCTCH1, 0x00, CH_BANK_OFFSET);
                    LOG_MSG(db_DV_SCALER, "dvC341_ConfigOutput_4K60-1  Total[%d x %d]\n",uiForcedSyncResetH_Total,uiForcedSyncResetV_Total);
                }
                else
                {
                    dvC341_Write(B8_OSYCTCH1,  0x0001, ch_v_RegOffset);//Select AOI0 as active output   //simon check
                    dvC341_Write(B24_OSYCTCH2, 0x1011, ch_v_RegOffset);                                 //simon check
                    dvC341_Write_ch_v(ch_v, B8_OHCYCLCH1, (uiForcedSyncResetH_Total/OUTPUT4K_CH_NUM - 2) , CH_BANK_OFFSET);
                    dvC341_Write_ch_v(ch_v, B8_OVCYCLCH1, (uiForcedSyncResetV_Total - 2), CH_BANK_OFFSET);

                    dvC341_Write_ch_v(ch_v, B8_SYRDLYCH1, 0, CH_BANK_OFFSET);  //Forced sync reset delay
                    //dvC341_Write_ch_v(ch_v, B8_FLDDLYCH1, 0, CH_BANK_OFFSET);  //Field Delay
                    dvC341_Write_ch_v(ch_v, B8_OSYCT2CH1, 0, CH_BANK_OFFSET);
                    dvC341_Write_ch_v(ch_v, B8_OSYCT3CH1, VAL_B8_OSYCT3CH1    , CH_BANK_OFFSET);//Trigger forced sync reset in unit of HS.
                    LOG_MSG(db_DV_SCALER, "dvC341_ConfigOutput_4K60-5  Total[%d x %d]\n",uiForcedSyncResetH_Total,uiForcedSyncResetV_Total);
                }
            }
        }
    }
    else
    {
        if(ch_v == eC341_CH_V0)
        {
            dvC341_Write(B8_OSYCTCH1,  0x0003, ch_v_RegOffset);//Select AOI0 as active output   //simon check
            dvC341_Write(B24_OSYCTCH2, 0x1013, ch_v_RegOffset);                                 //simon check
        }
        else
        {
            //0x1xxx = Set out sync synchnorized with channel1
            dvC341_Write(B8_OSYCTCH1,  (0x0013 + (ch_v << 9)) | ForcedSyncRstRegBIT, ch_v_RegOffset);    //simon check
            dvC341_Write(B24_OSYCTCH2, (0x1013 + (ch_v << 9)) | ForcedSyncRstRegBIT, ch_v_RegOffset);    //simon check
        }

        //A70LV_Doulas_0079
        uiForcedSyncResetH_Total = m_sChannelInfo[ch_v].sOutputTimingInfo.uiHTotal;
        uiForcedSyncResetV_Total = m_sChannelInfo[ch_v].sOutputTimingInfo.uiVTotal;
        ucFLDDLY = dvC341_Read(B8_FLDDLYCH1, ucRegOffset);  //Field Delay
        if((ucFLDDLY + 5)  > uiForcedSyncResetV_Total) //avoid " ucSYRDLY > 1 frame "      //H2PF_Simon_0196
        {
            ucFLDDLY = (uiForcedSyncResetV_Total) - 5;
            ucSYRDLY = ucFLDDLY+1;
        }
        else
        {
            ucSYRDLY = ucFLDDLY+1;
        }

        if(ch_v == eC341_CH_V0)
        {
            if((m_sChannelInfo[ch_v].sInputTimingInfo.eScanMode == eSCAN_MODE_NO_SIGNAL) ||
               (m_sChannelSetting[eC341_CH_V0].cPIP_PBP_ON))       //A70LV_Doulas_0168 modify PIP on not frame lock
            {
                //dvC789_ForcedSyncResetValueSet(1,uiForcedSyncResetH_Total,uiForcedSyncResetV_Total);
                dvC341_reg_set(B8_OHCYCLCH1, (uiForcedSyncResetH_Total/OUTPUT_CH_NUM - 2));
                dvC341_reg_set(B8_OVCYCLCH1, (uiForcedSyncResetV_Total - 2));

                dvC341_Write_ch_v(ch_v, B8_SYRDLYCH1, 0x00, CH_BANK_OFFSET);//Forced sync reset delay
                dvC341_Write_ch_v(ch_v, B8_OSYCT3CH1, VAL_B8_OSYCT3CH1, CH_BANK_OFFSET);//Trigger forced sync reset in unit of HS.
                //dvC341_Write(B8_FLDDLYCH1, 0x00, ucRegOffset);
                bForcedSyncResetEnable = FALSE;

                LOG_MSG(db_DV_SCALER, "dvC341_ConfigOutput_2K1 Total[%d x %d]\n",uiForcedSyncResetH_Total,uiForcedSyncResetV_Total);
            }
            else if(m_sChannelInfo[ch_v].sInputTimingInfo.dFrameRate > 59.50 && m_sChannelInfo[ch_v].sInputTimingInfo.dFrameRate < 60.3)   //A70LV_Doulas_0237 modify
            {
              /*  if((m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.uiVActive == 480) &&      // 480i
                   (m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.eScanMode == eSCAN_MODE_INTERLACE))      //A70LV_Doulas_0154 modify
                {
                    dvC341_reg_set(B2_OHCYCLCH1, (uiForcedSyncResetH_Total - 2));
                    dvC341_reg_set(B2_OVCYCLCH1, (uiForcedSyncResetV_Total - 2));
                    dvC341_Write(B2_SYRDLYCH1, 0, ucRegOffset);  //Forced sync reset delay
                    dvC341_Write(B2_OSYCT3CH1, 0x80, ucRegOffset);//Trigger forced sync reset in unit of HS.
                    bForcedSyncResetEnable = FALSE;
                }
                else */   //A70LV_Doulas_0170 remove ,fixed 480i frame lock
                {
                    dvC341_reg_set(B8_OHCYCLCH1, (uiForcedSyncResetH_Total/OUTPUT_CH_NUM - 2));
                    dvC341_reg_set(B8_OVCYCLCH1, ((uiForcedSyncResetV_Total * 1.1) - 2));

                    dvC341_Write_ch_v(ch_v, B8_SYRDLYCH1, ucSYRDLY, CH_BANK_OFFSET);  //Forced sync reset delay
                    dvC341_Write_ch_v(ch_v, B8_FLDDLYCH1, ucFLDDLY, CH_BANK_OFFSET);  //Field Delay
                    dvC341_Write_ch_v(ch_v, B8_OSYCT3CH1, VAL_B8_OSYCT3CH1    , CH_BANK_OFFSET);//Trigger forced sync reset in unit of HS.
                    bForcedSyncResetEnable = TRUE;
                }

                LOG_MSG(db_DV_SCALER, "dvC341_ConfigOutput_2K2 Total[%d x %d]\n",uiForcedSyncResetH_Total,uiForcedSyncResetV_Total);
            }
            else if(m_sChannelInfo[ch_v].sInputTimingInfo.dFrameRate >= 60.3 && m_sChannelInfo[ch_v].sInputTimingInfo.dFrameRate < 60.4)  //A70LV_Doulas_0237 modify//A70LV_Doulas_0169 Add
            {
                if(m_sChannelInfo[eC341_CH_V0].ucPanelIndex == ePANEL_ID_WUXGA_60HZ)
                {
                    uiForcedSyncResetH_Total = 1988;
                }
                else if(m_sChannelInfo[eC341_CH_V0].ucPanelIndex == ePANEL_ID_1080P_60HZ)
                {
                    uiForcedSyncResetH_Total = 2182;
                }

                dvC341_reg_set(B8_OHCYCLCH1, (uiForcedSyncResetH_Total/OUTPUT_CH_NUM - 2));
                dvC341_reg_set(B8_OVCYCLCH1, ((uiForcedSyncResetV_Total * 1.1) - 2));

                dvC341_Write_ch_v(ch_v, B8_SYRDLYCH1, ucSYRDLY, CH_BANK_OFFSET);  //Forced sync reset delay
                dvC341_Write_ch_v(ch_v, B8_FLDDLYCH1, ucFLDDLY, CH_BANK_OFFSET);  //Field Delay
                dvC341_Write_ch_v(ch_v, B8_OSYCT3CH1, VAL_B8_OSYCT3CH1,     CH_BANK_OFFSET);  //Trigger forced sync reset in unit of HS.
                bForcedSyncResetEnable = TRUE;

                LOG_MSG(db_DV_SCALER, "dvC341_ConfigOutput_2K5 Total[%d x %d]\n",uiForcedSyncResetH_Total,uiForcedSyncResetV_Total);
            }
            else if(m_sChannelInfo[ch_v].sInputTimingInfo.dFrameRate > 49.50 && m_sChannelInfo[ch_v].sInputTimingInfo.dFrameRate < 50.30)   //A70LV_Doulas_0237 modify
            {
                if(m_sChannelInfo[eC341_CH_V0].ucPanelIndex == ePANEL_ID_WUXGA_60HZ)
                {
                    uiForcedSyncResetH_Total = 2404;//2496;    //A70LV_Doulas_0145 modify
                }
                else if(m_sChannelInfo[eC341_CH_V0].ucPanelIndex == ePANEL_ID_1080P_60HZ)
                {
                    uiForcedSyncResetH_Total = 2640;
                }

                //dvC789_ForcedSyncResetValueSet(1,uiForcedSyncResetH_Total,uiForcedSyncResetV_Total);
                //dvC341_reg_set(B2_OHCYCLCH1, (uiForcedSyncResetH_Total - 2));         //A70LV_Doulas_0154 remove
                if((m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.uiVActive == 576) &&      // 576i
                   (m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.eScanMode == eSCAN_MODE_INTERLACE))      //A70LV_Doulas_0174 Fixed 576i frame lock
                {
                 /*   //not forced sync reset
                    uiForcedSyncResetH_Total = m_sChannelInfo[ch_v].sOutputTimingInfo.uiHTotal;  //A70LV_Doulas_0154
                    dvC341_reg_set(B2_OHCYCLCH1, (uiForcedSyncResetH_Total - 2));               //A70LV_Doulas_0154
                    dvC341_reg_set(B2_OVCYCLCH1, (uiForcedSyncResetV_Total - 2));
                    dvC341_Write(B2_SYRDLYCH1, 0, ucRegOffset);  //Forced sync reset delay
                    dvC341_Write(B2_OSYCT3CH1, 0x80, ucRegOffset);//Trigger forced sync reset in unit of HS.
                    bForcedSyncResetEnable = FALSE;
                    */
                    dvC341_reg_set(B8_OHCYCLCH1, (uiForcedSyncResetH_Total/OUTPUT_CH_NUM - 2));
                    dvC341_reg_set(B8_OVCYCLCH1, ((uiForcedSyncResetV_Total * 1.1) - 2));

                    dvC341_Write_ch_v(ch_v, B8_SYRDLYCH1, ucSYRDLY, CH_BANK_OFFSET);  //Forced sync reset delay
                    dvC341_Write_ch_v(ch_v, B8_FLDDLYCH1, ucFLDDLY, CH_BANK_OFFSET);  //Field Delay
                    dvC341_Write_ch_v(ch_v, B8_OSYCT3CH1, 0x90,     CH_BANK_OFFSET);      //Trigger forced sync reset in unit of HS.

                    if(m_sChannelSetting[ch_v].eInputSource == eCM_SOURCE_VGA)        //A70LV_Doulas_0175 modify
                    {
                        dvC341_Write_ch_v(ch_v, B20_IPOACTVSTCH1,(m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.uiVStart/2)*2 -1, CH_BANK_OFFSET); //H30K_Doulas_0001
                    }
                    else
                    {
                        dvC341_Write_ch_v(ch_v, B20_IPOACTVSTCH1,(m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.uiVStart/2)*2 -1 + 0x8000, CH_BANK_OFFSET); //H30K_Doulas_0001//A70LV_Doulas_0203 Fixed image jitter
                    }
                    bForcedSyncResetEnable = TRUE;
                }
                else
                {
                    dvC341_reg_set(B8_OHCYCLCH1, (uiForcedSyncResetH_Total/OUTPUT_CH_NUM - 2));   //A70LV_Doulas_0154
                    dvC341_reg_set(B8_OVCYCLCH1, ((uiForcedSyncResetV_Total * 1.1) - 2));

                    dvC341_Write_ch_v(ch_v, B8_SYRDLYCH1, ucSYRDLY, CH_BANK_OFFSET);  //Forced sync reset delay
                    dvC341_Write_ch_v(ch_v, B8_FLDDLYCH1, ucFLDDLY, CH_BANK_OFFSET);  //Field Delay
                    dvC341_Write_ch_v(ch_v, B8_OSYCT3CH1, VAL_B8_OSYCT3CH1,     CH_BANK_OFFSET);//Trigger forced sync reset in unit of HS.
                    bForcedSyncResetEnable = TRUE;
                }

                LOG_MSG(db_DV_SCALER, "dvC341_ConfigOutput_2K3 Total[%d x %d]\n",uiForcedSyncResetH_Total,uiForcedSyncResetV_Total);
            }
            else if((m_sChannelInfo[ch_v].sInputTimingInfo.dFrameRate > 239.50 && m_sChannelInfo[ch_v].sInputTimingInfo.dFrameRate < 240.3) &&
                    ((m_sChannelInfo[eC341_CH_V0].ePanelTimingId == ePANEL_ID_1080P_240HZ) || (m_sChannelInfo[eC341_CH_V0].ePanelTimingId == ePANEL_ID_WUXGA_240HZ)))  //H30K_Doulas_0009
            {
                {
                    dvC341_reg_set(B8_OHCYCLCH1, (uiForcedSyncResetH_Total/OUTPUT_CH_NUM - 2));
                    dvC341_reg_set(B8_OVCYCLCH1, ((uiForcedSyncResetV_Total * 1.1) - 2));

                    dvC341_Write_ch_v(ch_v, B8_SYRDLYCH1, ucSYRDLY, CH_BANK_OFFSET);  //Forced sync reset delay
                    dvC341_Write_ch_v(ch_v, B8_FLDDLYCH1, ucFLDDLY, CH_BANK_OFFSET);  //Field Delay
                    dvC341_Write_ch_v(ch_v, B8_OSYCT3CH1, 0x80    , CH_BANK_OFFSET);//Trigger forced sync reset in unit of HS.
                    bForcedSyncResetEnable = TRUE;
                }

                LOG_MSG(db_DV_SCALER, "dvC341_ConfigOutput_2K7 Total[%d x %d]\n",uiForcedSyncResetH_Total,uiForcedSyncResetV_Total);
            }
            else
            {
                //H30K_Doulas_0009 Add no frame lock
                {
                    dvC341_Write(B8_OSYCTCH1,  0x0001, ch_v_RegOffset);//Select AOI0 as active output   //simon check
                    dvC341_Write(B24_OSYCTCH2, 0x1011, ch_v_RegOffset);                                 //simon check
                }

                //dvC789_ForcedSyncResetValueSet(1,uiForcedSyncResetH_Total,uiForcedSyncResetV_Total);
                dvC341_reg_set(B8_OHCYCLCH1, (uiForcedSyncResetH_Total/OUTPUT_CH_NUM - 2));
                dvC341_reg_set(B8_OVCYCLCH1, (uiForcedSyncResetV_Total - 2));

                dvC341_Write_ch_v(ch_v, B8_SYRDLYCH1, 0x00, CH_BANK_OFFSET);//Forced sync reset delay
                dvC341_Write_ch_v(ch_v, B8_OSYCT3CH1, VAL_B8_OSYCT3CH1, CH_BANK_OFFSET);//Trigger forced sync reset in unit of HS.
                //dvC341_Write(B8_FLDDLYCH1, 0x00, ucRegOffset);
                bForcedSyncResetEnable = FALSE;

                LOG_MSG(db_DV_SCALER, "dvC341_ConfigOutput_2K4 Total[%d x %d]\n",uiForcedSyncResetH_Total,uiForcedSyncResetV_Total);
            }
        }
        else
        {
            dvC341_Write_ch_v(ch_v, B8_SYRDLYCH1, 0x00, CH_BANK_OFFSET);//Forced sync reset delay
            dvC341_Write_ch_v(ch_v, B8_OSYCT3CH1, VAL_B8_OSYCT3CH1, CH_BANK_OFFSET);//Trigger forced sync reset in unit of HS.
            //dvC341_Write(B8_FLDDLYCH1, 0x00, ucRegOffset);

            if(m_sChannelSetting[eC341_CH_V0].cPIP_PBP_ON)    //A70LV_Doulas_0349
            {
                dvC341_reg_set(B8_OHCYCLCH1, (m_sChannelInfo[ch_v].sOutputTimingInfo.uiHTotal/OUTPUT_CH_NUM - 2));
                dvC341_reg_set(B8_OVCYCLCH1, (m_sChannelInfo[ch_v].sOutputTimingInfo.uiVTotal - 2));
            }
        }
    }


    dvC341_Write(B3_OSYCTTX1, 0x19, 0);
    dvC341_Write(B3_OHSCTTX1, ((m_sChannelInfo[ch_v].sOutputTimingInfo.ucHSyncWidth / OUTPUT_CH_NUM) / 2) - 1, 0);
    dvC341_Write(B3_OVSCTTX1, m_sChannelInfo[ch_v].sOutputTimingInfo.ucVSyncWidth - 1, 0);
    //dvC341_Write(B3_LANESELTX1, 0x73625140, 0);  //PS_LANESELTX1
    dvC341_Write(B3_LANESELTX1, 0x04152637, 0);
    dvC341_Write(B3_OSYCTTX2, 0x19, 0);
    dvC341_Write(B3_OHSCTTX2, ((m_sChannelInfo[ch_v].sOutputTimingInfo.ucHSyncWidth / OUTPUT_CH_NUM) / 2) - 1, 0);
    dvC341_Write(B3_OVSCTTX2, m_sChannelInfo[ch_v].sOutputTimingInfo.ucVSyncWidth - 1, 0);

    dvC341_Write(B3_VBOCHSEL, 0x01000100, 0);  //PS_VBOCHSEL
    //dvC341_Write(B3_LANESELTX2, 0x73625140, 0);  //PS_LANESELTX2
    dvC341_Write(B3_LANESELTX2, 0x04152637, 0);

    dvC341_Write(B3_OVDLYTX1, 0x00, 0); //H30K_Doulas_0001
    dvC341_Write(B0_TTLPOSYCT, 0x0000, 0);  //H30K_Doulas_0001 //POHS, POVS, POACT, POFLD output disable ,CH1 output
    //dvC341_Write(B6_PISYPOLCTRL, 0x77777777, 0); //H30K_Doulas_0001
    //dvC341_reg_set(B2_OHSCTCH1, m_sChannelInfo[ch_v].sOutputTimingInfo.ucHSyncWidth/2-1);//21
    //dvC341_reg_set(B2_OVSCTCH1, m_sChannelInfo[ch_v].sOutputTimingInfo.ucVSyncWidth -1);//4
    //dvC341_Write(B8_FLDDLYCH1, 0x30, ucRegOffset);      //A70LV_Doulas_0003 remove

    dbmsg("m_sChannelInfo[ch_v].sOutputTimingInfo.uiVStart = %d\n\n", m_sChannelInfo[ch_v].sOutputTimingInfo.uiVStart);

    UINT16 REG_ACTVST = m_sChannelInfo[ch_v].sOutputTimingInfo.uiVStart;

    if(OUTPUT_CH_NUM == OUTPUT4K_CH_NUM)
    {
        REG_ACTVST--;
    }

    dvC341_reg_set(B8_OAOI0HSTCH1,  m_sChannelInfo[ch_v].sOutputTimingInfo.uiHStart / OUTPUT_CH_NUM /*PS_PANEL_HST*/);//192 + 480
    dvC341_reg_set(B8_OAOI0HENDCH1, (m_sChannelInfo[ch_v].sOutputTimingInfo.uiHStart / OUTPUT_CH_NUM +
                                     m_sChannelInfo[ch_v].sOutputTimingInfo.uiHActive / OUTPUT_CH_NUM  /*PS_PANEL_HST + PS_PANEL_1CH_HW*/));
    dvC341_reg_set(B8_OAOI0VSTCH1,  REG_ACTVST - 2);//42 + 1080  //simon check (ori -5 ?!)
    dvC341_reg_set(B8_OAOI0VENDCH1, (REG_ACTVST + m_sChannelInfo[ch_v].sOutputTimingInfo.uiVActive - 2 /*PS_PANEL_VST + PS_PANEL_VW*/));

    //dvC341_WriteToBuffer(B0_CMOSOUTSETUP, 0x40, ucRegOffset);   //A70LV_Doulas_0079 //POHS, POVS, POACT, POFLD output enable ,CH1 output    //simon check
    dvC341_Buffer_Flush();

	if(m_sChannelInfo[ch_v].sInputTimingInfo.eScanMode == eSCAN_MODE_NO_SIGNAL)	//A70LV_Doulas_0004 modify
        m_sChannelInfo[ch_v].ucOIMGCT |= 0x01;
    else
        m_sChannelInfo[ch_v].ucOIMGCT &= 0xFE;

    if(dvC341_Read(B8_OIMGCTCH1, (ch_v * 2) * CH_BANK_OFFSET) != m_sChannelInfo[ch_v].ucOIMGCT)
        dvC341_Write_ch_v(ch_v, B8_OIMGCTCH1, m_sChannelInfo[ch_v].ucOIMGCT, CH_BANK_OFFSET);
    dvC341_Buffer_Flush();
}

static void dvC341_ConfigIPInput_2K(const eC341_CH_V ch_v)//dvC341_SetupInput
{
    if(ch_v >= eC341_CH_VNUMBER)
    {
        return;
    }

    for(UINT8 ch=0; ch<OUTPUT4K_CH_NUM; ch++)
    {
        UINT8 ucRegOffset = ((ch_v * 2) + ch) * CH_BANK_OFFSET;

        //TODO : Need to set correct color conversion table, REC709...
        #if 1   //A70LV_Doulas_0080 modify
        dvC341_WriteToBuffer(B16_ICSC1CF00CH1, 0x4000, ucRegOffset);
        dvC341_WriteToBuffer(B16_ICSC1CF01CH1, 0xc5dd, ucRegOffset);
        dvC341_WriteToBuffer(B16_ICSC1CF02CH1, 0xfa23, ucRegOffset);
        dvC341_WriteToBuffer(B16_ICSC1CF10CH1, 0x1b36, ucRegOffset);
        dvC341_WriteToBuffer(B16_ICSC1CF11CH1, 0x5b8c, ucRegOffset);
        dvC341_WriteToBuffer(B16_ICSC1CF12CH1, 0x093e, ucRegOffset);
        dvC341_WriteToBuffer(B16_ICSC1CF20CH1, 0xf155, ucRegOffset);
        dvC341_WriteToBuffer(B16_ICSC1CF21CH1, 0xceab, ucRegOffset);
        dvC341_WriteToBuffer(B16_ICSC1CF22CH1, 0x4000, ucRegOffset);
        #else //REC601
        dvC341_WriteToBuffer(B16_ICSC1CF00CH1, 0x4000, ucRegOffset);
        dvC341_WriteToBuffer(B16_ICSC1CF01CH1, 0xca68, ucRegOffset);
        dvC341_WriteToBuffer(B16_ICSC1CF02CH1, 0xf598, ucRegOffset);
        dvC341_WriteToBuffer(B16_ICSC1CF10CH1, 0x2646, ucRegOffset);
        dvC341_WriteToBuffer(B16_ICSC1CF11CH1, 0x4b23, ucRegOffset);
        dvC341_WriteToBuffer(B16_ICSC1CF12CH1, 0x0e98, ucRegOffset);
        dvC341_WriteToBuffer(B16_ICSC1CF20CH1, 0xea68, ucRegOffset);
        dvC341_WriteToBuffer(B16_ICSC1CF21CH1, 0xd598, ucRegOffset);
        dvC341_WriteToBuffer(B16_ICSC1CF22CH1, 0x4000, ucRegOffset);
        #endif

        #ifdef MODIFY_FOR_C341
        dvC341_WriteToBuffer(B20_IPOACTHSTCH1, 0x0010, ucRegOffset);

        dvC341_WriteToBuffer(B20_MVNRCTCH1, 0x1F, ucRegOffset);//Movement NR control
        dvC341_WriteToBuffer(B20_HMVNRCFACH1, 0x01, ucRegOffset);//Horizontal movement NR coefficient
        dvC341_WriteToBuffer(B20_HMVNRCFBCH1, 0x01, ucRegOffset);
        dvC341_WriteToBuffer(B20_HMVNRCFCCH1, 0x02, ucRegOffset);
        dvC341_WriteToBuffer(B20_HMVNRCFDCH1, 0x08, ucRegOffset);

        dvC341_WriteToBuffer(B20_VFILCFACH1, 0x08, ucRegOffset);	//Vertical movement gain
        dvC341_WriteToBuffer(B20_VFILCFBCH1, 0x08, ucRegOffset);
        dvC341_WriteToBuffer(B20_VFILCFCCH1, 0x10, ucRegOffset);
        dvC341_WriteToBuffer(B20_VFILCFDCH1, 0x18, ucRegOffset);

        dvC341_WriteToBuffer(B20_HFILCFACH1, 0x08, ucRegOffset);//Horizontal movement gain
        dvC341_WriteToBuffer(B20_HFILCFBCH1, 0x08, ucRegOffset);
        dvC341_WriteToBuffer(B20_HFILCFCCH1, 0x10, ucRegOffset);
        dvC341_WriteToBuffer(B20_HFILCFDCH1, 0x18, ucRegOffset);

        dvC341_WriteToBuffer(B20_XVFILMDCH1, 0x00, ucRegOffset);	//Vertical MAX filter
        dvC341_WriteToBuffer(B20_XNRCUTCH1, 0x11, ucRegOffset);//Movement coefficient noise cut
    	dvC341_WriteToBuffer(B20_UVMVFIL0CH1, 0x08, ucRegOffset);
    	dvC341_WriteToBuffer(B20_UVMVFIL1CH1, 0x04, ucRegOffset);
        #else
        dvC341_WriteToBuffer(B20_IPOVCYCLCH1, 0x0FFF, ucRegOffset);
        dvC341_WriteToBuffer(B20_IPSYRDLYCH1, 0x01, ucRegOffset);
        dvC341_WriteToBuffer(B20_IPOACTHSTCH1, 0x0040, ucRegOffset);

        dvC341_WriteToBuffer(B20_MVNRCTCH1, 0x1F, ucRegOffset);//Movement NR control
        dvC341_WriteToBuffer(B20_HMVNRCFACH1, 0x01, ucRegOffset);//Horizontal movement NR coefficient
        dvC341_WriteToBuffer(B20_HMVNRCFBCH1, 0x01, ucRegOffset);
        dvC341_WriteToBuffer(B20_HMVNRCFCCH1, 0x02, ucRegOffset);
        dvC341_WriteToBuffer(B20_HMVNRCFDCH1, 0x08, ucRegOffset);

        dvC341_WriteToBuffer(B20_VFILCFACH1, 0x08, ucRegOffset);	//Vertical movement gain
        dvC341_WriteToBuffer(B20_VFILCFBCH1, 0x08, ucRegOffset);
        dvC341_WriteToBuffer(B20_VFILCFCCH1, 0x10, ucRegOffset);
        dvC341_WriteToBuffer(B20_VFILCFDCH1, 0x18, ucRegOffset);

        dvC341_WriteToBuffer(B20_HFILCFACH1, 0x08, ucRegOffset);//Horizontal movement gain
        dvC341_WriteToBuffer(B20_HFILCFBCH1, 0x08, ucRegOffset);
        dvC341_WriteToBuffer(B20_HFILCFCCH1, 0x10, ucRegOffset);
        dvC341_WriteToBuffer(B20_HFILCFDCH1, 0x18, ucRegOffset);

        dvC341_WriteToBuffer(B20_XVFILMDCH1, 0x00, ucRegOffset);	//Vertical MAX filter
        dvC341_WriteToBuffer(B20_XNRCUTCH1, 0x01, ucRegOffset);//Movement coefficient noise cut
        dvC341_WriteToBuffer(B20_IPTESTCH1, 0x08, ucRegOffset);   //IP test, bit3: EPADJ: Line adjustment
        dvC341_WriteToBuffer(B20_IPHSYRDLYCH1,	0x86f0, ucRegOffset);	//IP conversion horizontal synchronization signal delay
        dvC341_WriteToBuffer(B23_IPYOFFCH1,	0x04ae, ucRegOffset);		//IP conversion luminance address offset

        dvC341_WriteToBuffer(B20_DTNRFIL0CH1,	0x00, ucRegOffset);//0x01, ucRegOffset);	//A70LV_Doulas_0179 modify default//Dot disturbance filter
        dvC341_WriteToBuffer(B20_DTNRFIL1CH1,	0x00, ucRegOffset);//0x01, ucRegOffset);    //A70LV_Doulas_0179 modify default
        dvC341_WriteToBuffer(B20_UVMVFIL0CH1,	0x01, ucRegOffset);	//Color difference filter
        dvC341_WriteToBuffer(B20_UVMVFIL1CH1,	0x01, ucRegOffset);
        #endif

        #ifndef ONLY_TEST_FOR_C341
        dvC341_WriteToBuffer(B20_DIAGCT0CH1, 0x0B, ucRegOffset);//Diagonal interpolation control
        dvC341_WriteToBuffer(B20_DIAGCT1CH1, 0x7F, ucRegOffset);
        dvC341_WriteToBuffer(B20_DIAGCT2CH1, 0x77, ucRegOffset);
        dvC341_WriteToBuffer(B20_DIAGTH0CH1, 0x13, ucRegOffset);
        dvC341_WriteToBuffer(B20_DIAGTH1CH1, 0x09, ucRegOffset);
        dvC341_WriteToBuffer(B20_DIAGTH2CH1, 0x10, ucRegOffset);
        dvC341_WriteToBuffer(B20_DIAGTH3CH1, 0x03, ucRegOffset);
        dvC341_WriteToBuffer(B20_DIAGTH4CH1, 0x08, ucRegOffset);
        dvC341_WriteToBuffer(B20_DIAGTH5CH1, 0x0A, ucRegOffset);
        dvC341_WriteToBuffer(B20_DIAGTH6CH1, 0x14, ucRegOffset);
        dvC341_WriteToBuffer(B20_DIAGTH7CH1, 0x04, ucRegOffset);
        dvC341_WriteToBuffer(B20_DIAGTH8CH1, 0x04, ucRegOffset);
        dvC341_WriteToBuffer(B20_DIAGTH9CH1, 0x02, ucRegOffset);
        dvC341_WriteToBuffer(B20_DIAGTH10CH1, 0x02, ucRegOffset);
        dvC341_WriteToBuffer(B20_DIAGTH11CH1, 0x10, ucRegOffset);
        dvC341_WriteToBuffer(B20_DIAGTH12CH1, 0x31, ucRegOffset);
        dvC341_WriteToBuffer(B20_DIAGAD0CH1, 0x00, ucRegOffset);//Diagonal interpolation addition
        dvC341_WriteToBuffer(B20_DIAGAD1CH1, 0x00, ucRegOffset);
        dvC341_WriteToBuffer(B20_DIAGAD2CH1, 0x00, ucRegOffset);
        dvC341_WriteToBuffer(B20_DIAGAD3CH1, 0x00, ucRegOffset);

        dvC341_WriteToBuffer(B20_IPHMONCH1, 0x01, ucRegOffset);   //De-interlacer monitor
        #endif

#if 0 //test on   //simon check
        // IPLUT
        // VP syoga p2,3 = 0.5 p0,4 = 0.0
        dvC341_WriteToBuffer(B20_LUTCFCH1, 0x00004000, ucRegOffset);//De-interlacer LUT
        dvC341_WriteToBuffer(B20_LUTWADCH1, 0x00, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWRCH1, 0xFF, ucRegOffset);

        // VP syoga p2,3 = 0.4375 p0,4 = 0.0625
        dvC341_WriteToBuffer(B20_LUTCFCH1, 0x00103801, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWADCH1, 0x01, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWRCH1, 0xFF, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWADCH1, 0x02, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWRCH1, 0xFF, ucRegOffset);

        // VP syoga p2,3 = 0.375 p0,4 = 0.125
        dvC341_WriteToBuffer(B20_LUTCFCH1, 0x00203002, ucRegOffset);

        dvC341_WriteToBuffer(B20_LUTWADCH1, 0x03, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWRCH1, 0xFF, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWADCH1, 0x04, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWRCH1, 0xFF, ucRegOffset);

        // VP syoga p2,3 = 0.3125 p0,4 = 0.1875
        dvC341_WriteToBuffer(B20_LUTCFCH1, 0x00302803, ucRegOffset);

        dvC341_WriteToBuffer(B20_LUTWADCH1, 0x05, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWRCH1, 0xFF, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWADCH1, 0x06, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWRCH1, 0xFF, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWADCH1, 0x07, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWRCH1, 0xFF, ucRegOffset);

        // VP syoga p2,3 = 0.25 p0,4 = 0.25
        dvC341_WriteToBuffer(B20_LUTCFCH1, 0x00402004, ucRegOffset);

        dvC341_WriteToBuffer(B20_LUTWADCH1, 0x08, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWRCH1, 0xFF, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWADCH1, 0x09, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWRCH1, 0xFF, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWADCH1, 0x0A, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWRCH1, 0xFF, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWADCH1, 0x0B, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWRCH1, 0xFF, ucRegOffset);

        // VP syoga p2,3 = 0.1875 p0,4 = 0.3125
        dvC341_WriteToBuffer(B20_LUTCFCH1, 0x00501805, ucRegOffset);

        dvC341_WriteToBuffer(B20_LUTWADCH1, 0x0C, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWRCH1, 0xFF, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWADCH1, 0x0D, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWRCH1, 0xFF, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWADCH1, 0x0E, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWRCH1, 0xFF, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWADCH1, 0x0F, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWRCH1, 0xFF, ucRegOffset);

        // VP syoga p2,3 = 0.125 p0,4 = 0.375
        dvC341_WriteToBuffer(B20_LUTCFCH1, 0x00601006, ucRegOffset);

        dvC341_WriteToBuffer(B20_LUTWADCH1, 0x10, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWRCH1, 0xFF, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWADCH1, 0x11, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWRCH1, 0xFF, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWADCH1, 0x12, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWRCH1, 0xFF, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWADCH1, 0x13, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWRCH1, 0xFF, ucRegOffset);

        // VP syoga p2,3 = 0.0625 p0,4 = 0.4375
        dvC341_WriteToBuffer(B20_LUTCFCH1, 0x00700807, ucRegOffset);

        dvC341_WriteToBuffer(B20_LUTWADCH1, 0x14, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWRCH1, 0xFF, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWADCH1, 0x15, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWRCH1, 0xFF, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWADCH1, 0x16, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWRCH1, 0xFF, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWADCH1, 0x17, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWRCH1, 0xFF, ucRegOffset);

        // VP line double p2,3 = 0.0 p0,4 = 0.5
        dvC341_WriteToBuffer(B20_LUTCFCH1, 0x00800008, ucRegOffset);

        dvC341_WriteToBuffer(B20_LUTWADCH1, 0x18, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWRCH1, 0xFF, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWADCH1, 0x19, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWRCH1, 0xFF, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWADCH1, 0x1A, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWRCH1, 0xFF, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWADCH1, 0x1B, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWRCH1, 0xFF, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWADCH1, 0x1C, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWRCH1, 0xFF, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWADCH1, 0x1D, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWRCH1, 0xFF, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWADCH1, 0x1E, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWRCH1, 0xFF, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWADCH1, 0x1F, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWRCH1, 0xFF, ucRegOffset);

        // EP p2 = 1.0
        dvC341_WriteToBuffer(B20_LUTCFCH1, 0x00004000, ucRegOffset);

        dvC341_WriteToBuffer(B20_LUTWADCH1, 0x20, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWRCH1, 0xFF, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWADCH1, 0x21, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWRCH1, 0xFF, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWADCH1, 0x22, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWRCH1, 0xFF, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWADCH1, 0x23, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWRCH1, 0xFF, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWADCH1, 0x24, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWRCH1, 0xFF, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWADCH1, 0x25, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWRCH1, 0xFF, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWADCH1, 0x26, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWRCH1, 0xFF, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWADCH1, 0x27, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWRCH1, 0xFF, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWADCH1, 0x28, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWRCH1, 0xFF, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWADCH1, 0x29, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWRCH1, 0xFF, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWADCH1, 0x2A, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWRCH1, 0xFF, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWADCH1, 0x2B, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWRCH1, 0xFF, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWADCH1, 0x2C, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWRCH1, 0xFF, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWADCH1, 0x2D, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWRCH1, 0xFF, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWADCH1, 0x2E, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWRCH1, 0xFF, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWADCH1, 0x2F, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWRCH1, 0xFF, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWADCH1, 0x30, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWRCH1, 0xFF, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWADCH1, 0x31, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWRCH1, 0xFF, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWADCH1, 0x32, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWRCH1, 0xFF, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWADCH1, 0x33, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWRCH1, 0xFF, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWADCH1, 0x34, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWRCH1, 0xFF, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWADCH1, 0x35, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWRCH1, 0xFF, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWADCH1, 0x36, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWRCH1, 0xFF, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWADCH1, 0x37, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWRCH1, 0xFF, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWADCH1, 0x38, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWRCH1, 0xFF, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWADCH1, 0x39, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWRCH1, 0xFF, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWADCH1, 0x3A, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWRCH1, 0xFF, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWADCH1, 0x3B, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWRCH1, 0xFF, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWADCH1, 0x3C, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWRCH1, 0xFF, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWADCH1, 0x3D, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWRCH1, 0xFF, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWADCH1, 0x3E, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWRCH1, 0xFF, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWADCH1, 0x3F, ucRegOffset);
        dvC341_WriteToBuffer(B20_LUTWRCH1, 0xFF, ucRegOffset);
#endif

    }

    dvC341_Buffer_Flush();
}

static void dvC341_ConfigInput_2K(const eC341_CH_V ch_v)//dvC341_SetupInput
{
    if(ch_v >= eC341_CH_VNUMBER)
    {
        return;
    }

    UINT8 ucRegOffset;

    m_sChannelInfo[ch_v].ucMCT = 0x01;

    //LOG_MSG(db_APP_DATAPATH, "<5>%d\n", m_sChannelInfo[ch_v].ucMCT);

    if(ch_v == eC341_CH_V0)
    {
        if(dvC341_Read(B0_MCT1,0) != m_sChannelInfo[ch_v].ucMCT)
            dvC341_Write(B0_MCT1, m_sChannelInfo[ch_v].ucMCT, 0);    //Enable Image Output
        if(dvC341_Read(B0_MCT2,0) != m_sChannelInfo[ch_v].ucMCT)
            dvC341_Write(B0_MCT2, m_sChannelInfo[ch_v].ucMCT, 0);
    }
    else if(ch_v == eC341_CH_V1)
    {
        if(dvC341_Read(B0_MCT3,0) != m_sChannelInfo[ch_v].ucMCT)
            dvC341_Write(B0_MCT3, m_sChannelInfo[ch_v].ucMCT, 0);    //Enable Image Output
        if(dvC341_Read(B0_MCT4,0) != m_sChannelInfo[ch_v].ucMCT)
            dvC341_Write(B0_MCT4, m_sChannelInfo[ch_v].ucMCT, 0);
    }

  //  dvC341_Write(B1_INTEN, m_sChannelInfo[ch_v].ulInterruptEnable, 0); //A70LV_Doulas_0002


    m_sChannelInfo[ch_v].ucISYCT = 0x00;
    dvC341_Write_ch_v(ch_v, B16_ISYCTCH1, m_sChannelInfo[ch_v].ucISYCT, CH_BANK_OFFSET);
    dvC341_Write_ch_v(ch_v, B16_DIFCTCH1, 0x81, CH_BANK_OFFSET);       //H30K_Doulas_0049 左邊線偏綠//simon check ori 0x83
    dvC341_Write_ch_v(ch_v, B16_ICOLORCTCH1, 0x08, CH_BANK_OFFSET);   //A70LV_Doulas_0080 modify//Enable YUV422 to YUV444 conversion in PIACT area    //simon check //demo board 0x8a
    //dvC341_Write(B16_AUTOFLDCH1, 0x80, ucRegOffset);    //Field auto recognition enable
    dvC341_Write_ch_v(ch_v, B16_AUTOFLDCH1, 0x80, CH_BANK_OFFSET);    //Field auto recognition enable   //simon check ori 0x00
    dvC341_Write_ch_v(ch_v, B17_SHRNKADJCH1, 0x02, CH_BANK_OFFSET);   //Shrink compensation enable

    //dvC341_WriteToBuffer(B16_ISWPCH1,  	0x00, ucRegOffset);	    //Input Data Swap   //A70LV_Doulas_0002   //simon check

    dvC341_Write_ch_v(ch_v, B16_UVLPCOEF0CH1,	0x00, CH_BANK_OFFSET); //Input UV low pass filter coefficient	//	YUV444->YUV422 filter
    dvC341_Write_ch_v(ch_v, B16_UVLPCOEF1CH1,	0x04, CH_BANK_OFFSET);
    dvC341_Write_ch_v(ch_v, B16_UVLPCOEF2CH1,	0x08, CH_BANK_OFFSET);

    //		dvC341_Write( B17_PTHCH1, 0x80, ich_ofst );
    //TODO
    dvC341_Write_ch_v(ch_v, B16_PTHCH1, 0x00 /*0xF0*/, CH_BANK_OFFSET);        //Characteristics measurement threshold   //simon check, A70G2 use 0xF0
    //dvC341_Write(B8_PTHCH1, 0x04, ucRegOffset);
    dvC341_Buffer_Flush();
}

static void dvC341_ConfigIPMemory_2K(const eC341_CH_V ch_v)
{
    UINT32 ulData = 0;
    if(ch_v >= eC341_CH_VNUMBER)
    {
        return;
    }

	UINT32 mwi = DEF_IPMWICH1_4K & 0x1f;
	UINT32 val = mwi * 1024;

    if(ch_v == eC341_CH_V0)
    {
        for(UINT8 ch=0; ch<OUTPUT4K_CH_NUM; ch++)
        {
            UINT8 BankOffset = CH_BANK_OFFSET;
            UINT8 ucRegOffset = ((ch_v * 2) + ch) * BankOffset;

            dvC341_WriteToBuffer(B22_OMWICH1, DEF_MWI_4K, ucRegOffset);
            dvC341_WriteToBuffer(B22_IMWICH1, DEF_MWI_4K, ucRegOffset);

            //Setup De-interlacer
            dvC341_WriteToBuffer(B23_IPOSFLD0CH1, DEF_IP_I_ISFLD0CH1_4K+val,   ucRegOffset);
            dvC341_WriteToBuffer(B23_IPOSFLD1CH1, DEF_IP_I_ISFLD0CH1_4K,       ucRegOffset);
            dvC341_WriteToBuffer(B23_IPOSFLD2CH1, DEF_IP_I_ISFLD1CH1_4K+val,   ucRegOffset);
            dvC341_WriteToBuffer(B23_IPOSFLD3CH1, DEF_IP_I_ISFLD1CH1_4K,       ucRegOffset);
            dvC341_WriteToBuffer(B23_IPISFLD0CH1, DEF_IP_I_ISFLD0CH1_4K+val,   ucRegOffset);
            dvC341_WriteToBuffer(B23_IPISFLD1CH1, DEF_IP_I_ISFLD0CH1_4K,       ucRegOffset);
            dvC341_WriteToBuffer(B23_IPISFLD2CH1, DEF_IP_I_ISFLD1CH1_4K+val,   ucRegOffset);
            dvC341_WriteToBuffer(B23_IPISFLD3CH1, DEF_IP_I_ISFLD1CH1_4K,       ucRegOffset);

        //TODO
        //ulData = ((0x07&DEF_MV_MWICH1_2K)<<5)|(DEF_IPMWICH1_2K&0x1f);
            dvC341_WriteToBuffer(B23_IPMWICH1,  DEF_IPMWICH1_4K , ucRegOffset);//0x45, 1920 x (4/3)/1024x2 = 5  //simon check
            dvC341_WriteToBuffer(B23_IPYOFFCH1, DEF_IPYOFFCH1_4K, ucRegOffset);
            dvC341_WriteToBuffer(B23_MVFLDCH1,  DEF_MVFLDCH1_4K,  ucRegOffset);
        }
    }
    else if(ch_v == eC341_CH_V1)
    {
        for(UINT8 ch=0; ch<OUTPUT4K_CH_NUM; ch++)
        {
            UINT8 BankOffset = CH_BANK_OFFSET;
            UINT8 ucRegOffset = ((ch_v * 2) + ch) * BankOffset;

            dvC341_WriteToBuffer(B22_OMWICH1, DEF_MWI_4K, ucRegOffset);
            dvC341_WriteToBuffer(B22_IMWICH1, DEF_MWI_4K, ucRegOffset);

            dvC341_WriteToBuffer(B23_IPOSFLD0CH1, DEF_IP_I_ISFLD0CH2_4K+val,     ucRegOffset);
            dvC341_WriteToBuffer(B23_IPOSFLD1CH1, DEF_IP_I_ISFLD0CH2_4K,         ucRegOffset);
            dvC341_WriteToBuffer(B23_IPOSFLD2CH1, DEF_IP_I_ISFLD1CH2_4K+val,     ucRegOffset);
            dvC341_WriteToBuffer(B23_IPOSFLD3CH1, DEF_IP_I_ISFLD1CH2_4K,         ucRegOffset);
            dvC341_WriteToBuffer(B23_IPISFLD0CH1, DEF_IP_I_ISFLD0CH2_4K+val,     ucRegOffset);
            dvC341_WriteToBuffer(B23_IPISFLD1CH1, DEF_IP_I_ISFLD0CH2_4K,         ucRegOffset);
            dvC341_WriteToBuffer(B23_IPISFLD2CH1, DEF_IP_I_ISFLD1CH2_4K+val,     ucRegOffset);
            dvC341_WriteToBuffer(B23_IPISFLD3CH1, DEF_IP_I_ISFLD1CH2_4K,         ucRegOffset);

            //ulData = ((0x07&DEF_MV_MWICH2_4K)<<5)|(DEF_IPMWICH2_4K&0x1f);
            dvC341_WriteToBuffer(B23_IPMWICH1,  DEF_IPMWICH2_4K,  ucRegOffset);
            dvC341_WriteToBuffer(B23_IPYOFFCH1, DEF_IPYOFFCH2_4K, ucRegOffset);
            dvC341_WriteToBuffer(B23_MVFLDCH1,  DEF_MVFLDCH2_4K,  ucRegOffset);
        }
    }

    dvC341_Buffer_Flush();
}

static void dvC341_ConfigBasicMemory_2K(void)//dvC341_SetupMemory
{
    ///////////////////////////////////////////////////////////
	//  DDR
	///////////////////////////////////////////////////////////
    // ; C341BN registers
    dvC341_Write(BN_DDR3CT, 0x00, 0);
    dvC341_Write(BN_MEMCT, 0x00, 0);
    dvC341_Write(B1_DDR3DLLCT, 0x00, 0);
    MS_SLEEP(1);

    dvC341_Write(B1_DDR3CT2, 0x30, 0);        // Fixed value, [5]OCDMD=1(auto) / [4]OCDEN=1(OCD-cal EN)
    dvC341_Write(B1_DDR3GDS, 0x33333333, 0);    // [31:0]GDS8-1(MDQS[7:0] round trip timing-1)
    dvC341_Write(B1_DDR3MSDLY, 0x66554444, 0);    // [31:0]MSDLY8-1(MDQS[7:0] round trip timing-2)
    dvC341_Write(B1_DDR3PHYODTMD, 0x22222222, 0);    // [31:0]ODTMD8-1(ASIC ODT=60ohm)
    dvC341_Write(B1_DDR3RONMD_0, 0x77, 0);        // [7:0]RONMDD2-1(ASIC DIC(Data)=34ohm)
    dvC341_Write(B1_DDR3RONMD_1, 0x77, 0);        // [7:0]RONMDD4-3(ASIC DIC(Data)=34ohm)
    dvC341_Write(B1_DDR3RONMD_2, 0x77, 0);        // [7:0]RONMDD6-5(ASIC DIC(Data)=34ohm)
    dvC341_Write(B1_DDR3RONMD_3, 0x77, 0);        // [7:0]RONMDD8-7(ASIC DIC(Data)=34ohm)
    dvC341_Write(B1_DDR3RONMD_4, 0x07, 0);        // [2:0]RONMDA(ASIC DIC(CLK/Cont/Addr)=34ohm)
    dvC341_Write(B1_DDR3OCDDIP, 0x3f, 0);        // Fixed value
    dvC341_Write(B1_DDR3OCDDIN, 0x3f, 0);        // Fixed value
    dvC341_Write(B1_DDR3LVLCT, 0x00, 0);        // Fixed value
    if((m_sChannelInfo[eC341_CH_V0].ePanelTimingId == ePANEL_ID_1080P_120HZ) ||
       (m_sChannelInfo[eC341_CH_V0].ePanelTimingId == ePANEL_ID_WUXGA_120HZ)) //H30K_Doulas_0001
    {
        dvC341_Write(B1_DDR3WRLVLPHCT1, 0x26, 0);        // [6:0]PDLSET1(MDQS[0] output phase control)
        dvC341_Write(B1_DDR3WRLVLPHCT2, 0x26, 0);        // [6:0]PDLSET2(MDQS[1] output phase control)
        dvC341_Write(B1_DDR3WRLVLPHCT3, 0x24, 0);        // [6:0]PDLSET3(MDQS[2] output phase control)
        dvC341_Write(B1_DDR3WRLVLPHCT4, 0x24, 0);        // [6:0]PDLSET4(MDQS[3] output phase control)
        dvC341_Write(B1_DDR3WRLVLPHCT5, 0x45, 0);        // [6:0]PDLSET5(MDQS[4] output phase control)
        dvC341_Write(B1_DDR3WRLVLPHCT6, 0x45, 0);        // [6:0]PDLSET6(MDQS[5] output phase control)
        dvC341_Write(B1_DDR3WRLVLPHCT7, 0x62, 0);        // [6:0]PDLSET7(MDQS[6] output phase control)
        dvC341_Write(B1_DDR3WRLVLPHCT8, 0x62, 0);        // [6:0]PDLSET8(MDQS[7] output phase control)
    }
    else
    {
        dvC341_Write(B1_DDR3WRLVLPHCT1, 0x41, 0);        // [6:0]PDLSET1(MDQS[0] output phase control)
        dvC341_Write(B1_DDR3WRLVLPHCT2, 0x40, 0);        // [6:0]PDLSET2(MDQS[1] output phase control)
        dvC341_Write(B1_DDR3WRLVLPHCT3, 0x25, 0);        // [6:0]PDLSET3(MDQS[2] output phase control)
        dvC341_Write(B1_DDR3WRLVLPHCT4, 0x25, 0);        // [6:0]PDLSET4(MDQS[3] output phase control)
        dvC341_Write(B1_DDR3WRLVLPHCT5, 0x60, 0);        // [6:0]PDLSET5(MDQS[4] output phase control)
        dvC341_Write(B1_DDR3WRLVLPHCT6, 0x60, 0);        // [6:0]PDLSET6(MDQS[5] output phase control)
        dvC341_Write(B1_DDR3WRLVLPHCT7, 0x63, 0);        // [6:0]PDLSET7(MDQS[6] output phase control)
        dvC341_Write(B1_DDR3WRLVLPHCT8, 0x63, 0);        // [6:0]PDLSET8(MDQS[7] output phase control)
    }
    dvC341_Write(B1_DDR3VREFCT, 0x09, 0);        // Fixed value, [3:1]VREFSEL=100(0.5*VCC15O_DDR) / [0]SELFBIASEN=1(use internal VREF)
    dvC341_Write(B1_DDR3DLLCT, 0x0f, 0);        // Fixed value, [3:1]DLLFRANGE=111(for DDR3-1600) / [0]DLLPDN=1(PWR ON)
    dvC341_Write(B1_DDR3ODTCT, 0x11, 0);        // [5:4]ODTWR=01(DDR3 Rtt_WR=RZQ/4) / [2:0]ODTNOM=001(DDR3 Rtt_Nom=RZQ/4)
    dvC341_Write(B1_DDR3ZQCT, 0x01, 0);        // [7:4]ZQCNT=0000(DDR3 ZQ-cal interval=80us) / [0]ZQEN=1(DDR3 ZQ-cal EN)
    dvC341_Write(B1_DDR3WRTMGCT, 0x88888888, 0);    // Fixed value
    dvC341_Write(B1_DDR3RDTMGCT1, 0x77, 0);        // Fixed value
    dvC341_Write(B1_DDR3RDTMGCT2, 0x77, 0);        // Fixed value
    dvC341_Write(B1_DDR3RDTMGCT3, 0x77, 0);        // Fixed value
    dvC341_Write(B1_DDR3RDTMGCT4, 0x77, 0);        // Fixed value
    dvC341_Write(B1_DDR3RDTMGCT5, 0x77, 0);        // Fixed value
    dvC341_Write(B1_DDR3RDTMGCT6, 0x77, 0);        // Fixed value
    dvC341_Write(B1_DDR3RDTMGCT7, 0x77, 0);        // Fixed value
    dvC341_Write(B1_DDR3RDTMGCT8, 0x77, 0);        // Fixed value
    dvC341_Write(B1_DDR3CMDTMGCT, 0x07, 0);        // Fixed value
    dvC341_Write(B1_DDR3CKTMGCT1, 0x67, 0);        // Fixed value
    dvC341_Write(B1_DDR3CKTMGCT2, 0x65, 0);        // Fixed value
    dvC341_Write(B1_DDR3CKTMGCT3, 0x62, 0);        // Fixed value
    dvC341_Write(B1_DDR3CKTMGCT4, 0x60, 0);        // Fixed value
    dvC341_Write(B1_DDR3CKTMGCT5, 0x6a, 0);        // Fixed value
    dvC341_Write(B1_DDR3CKTMGCT6, 0x67, 0);        // Fixed value
    dvC341_Write(B1_DDR3CKTMGCT7, 0x64, 0);        // Fixed value
    dvC341_Write(B1_DDR3CKTMGCT8, 0x61, 0);        // Fixed value
    dvC341_Write(B1_DDR3CKTMGCTA, 0x6c, 0);        // Fixed value
    dvC341_Write(B1_DDR3CKDUTYSEL, 0x08, 0);        // Fixed value
    dvC341_Write(B1_DDR3DQRDDLSEL1, 0x924924, 0);    // Fixed value
    dvC341_Write(B1_DDR3DQRDDLSEL2, 0x924924, 0);    // Fixed value
    dvC341_Write(B1_DDR3DQRDDLSEL3, 0x924924, 0);    // Fixed value
    dvC341_Write(B1_DDR3DQRDDLSEL4, 0x924924, 0);    // Fixed value
    dvC341_Write(B1_DDR3DQRDDLSEL5, 0x924924, 0);    // Fixed value
    dvC341_Write(B1_DDR3DQRDDLSEL6, 0x924924, 0);    // Fixed value
    dvC341_Write(B1_DDR3DQRDDLSEL7, 0x924924, 0);    // Fixed value
    dvC341_Write(B1_DDR3DQRDDLSEL8, 0x924924, 0);    // Fixed value
    dvC341_Write(B1_DDR3DQWRDLSEL1, 0x6db6db, 0);    // Fixed value
    dvC341_Write(B1_DDR3DQWRDLSEL2, 0x6db6db, 0);    // Fixed value
    dvC341_Write(B1_DDR3DQWRDLSEL3, 0x6db6db, 0);    // Fixed value
    dvC341_Write(B1_DDR3DQWRDLSEL4, 0x6db6db, 0);    // Fixed value
    dvC341_Write(B1_DDR3DQWRDLSEL5, 0x6db6db, 0);    // Fixed value
    dvC341_Write(B1_DDR3DQWRDLSEL6, 0x6db6db, 0);    // Fixed value
    dvC341_Write(B1_DDR3DQWRDLSEL7, 0x6db6db, 0);    // Fixed value
    dvC341_Write(B1_DDR3DQWRDLSEL8, 0x6db6db, 0);    // Fixed value
    dvC341_Write(B1_DDR3DQMDLSEL, 0x33333333, 0);    // Fixed value
    MS_SLEEP(1);                    // wait 500us
    dvC341_Write(BN_DDR3CT, 0x88, 0);        // [7]RSTB=1(Rst ENB.) / [3:2]RFACSEL=10(use 4Gbit DDR3)
    MS_SLEEP(1);                    // wait 10us
    dvC341_Write(BN_DDR3CT, 0xc8, 0);        // [6]MRESETB=1(DDR3 RESET# L->H)
    MS_SLEEP(1);                    // wait 1ms
    dvC341_Write(BN_MEMCT, 0x61, 0);        // [7:6]DS=01(DDR3 DIC=RZQ/7) / [5:4]RFCNT=10(for DDR3-1600) / [0]SDACT=1(DDR3 initialize EN)
}



static void dvC341_ConfigClock(const UINT8 ucPoRefDiv, const UINT8 ucPoFbDiv, const UINT8 ucIPPLLCT) //dvC341_SetupClock
{
/*
;register settings
00         ; PS_PO1CLKCT
89         ; PS_PO1PLLCT
00         ; PS_PO1REFSEL
02         ; PS_PO1REFDIV
16         ; PS_PO1FBDIV
89         ; PS_PIPLLCT
00         ; PS_PIREFSEL
02         ; PS_PIREFDIV
18         ; PS_PIFBDIV
*/

    dvC341_Write(B0_PO1CLKCT,   0x00, 0);
    dvC341_Write(B0_PO1PLLCT,   0x89, 0);
    dvC341_Write(B0_PO1REFSEL,  0x00, 0);
    dvC341_Write(B0_PO1REFDIV,  ucPoRefDiv, 0);
    dvC341_Write(B0_PO1FBDIV,   ucPoFbDiv,  0);

    dvC341_Write(B0_PO2CLKCT,   0x00, 0);
    dvC341_Write(B0_PO2PLLCT,   0x89, 0);
    dvC341_Write(B0_PO2REFSEL,  0x00, 0);
    dvC341_Write(B0_PO2REFDIV,  ucPoRefDiv, 0);
    dvC341_Write(B0_PO2FBDIV,   ucPoFbDiv,  0);

    if((m_sChannelInfo[eC341_CH_V0].ePanelTimingId == ePANEL_ID_1080P_120HZ) ||
       (m_sChannelInfo[eC341_CH_V0].ePanelTimingId == ePANEL_ID_WUXGA_120HZ)) //H30K_Doulas_0001
    {
        dvC341_Write(B0_PIPLLCT,    0x89, 0);
        dvC341_Write(B0_PIREFSEL,   0x01, 0);
        //dvC341_Write(B0_PIREFSEL,   0x00, 0);
        dvC341_Write(B0_PIREFDIV,   0x02, 0);
        dvC341_Write(B0_PIFBDIV,    0x18, 0);
    }
    else
    {
        dvC341_Write(B0_PIPLLCT,    0x89, 0);
        dvC341_Write(B0_PIREFSEL,   0x00, 0);
        dvC341_Write(B0_PIREFDIV,   0x02, 0);
        dvC341_Write(B0_PIFBDIV,    0x18, 0);
    }


#if 0
    //TODO

    //Becaure channel 2~4 OSYCTCH will set to synchnorized with channel1 in the dvC341_SetOcycle()
    //Therefore, we only set enable pll in the ch1~ch4
    dvC341_Write(B0_POREFDIV, ucPoRefDiv, 0);
    dvC341_Write(B0_POFBDIV,  ucPoFbDiv, 0);     //75.0MHz/4*8=150MHz    //74.25MHz/1*2=148.5MHz
    if(m_sChannelInfo[eMCT_CH1].ePanelTimingId == ePANEL_ID_1080P_120HZ)     //A70LV_Doulas_0154 modify //A70LV_Doulas_0094
    {
        dvC341_Write(B0_POPLLCT,  0x08, 0);//0x0C, 0);     //A70LV_Doulas_0098
        dvC341_Write(B0_POPLLCT,  0x09, 0);//0x0D, 0);     //A70LV_Doulas_0098 //Enable PLL Control    //PLL Frquency Range From Higher than 160 MHz

        dvC341_Write(B0_POCLKCT,  0x01, 0);//0x00, 0);     //A70LV_Doulas_0098
    }
    else if(m_sChannelInfo[eMCT_CH1].ePanelTimingId == ePANEL_ID_WUXGA_120HZ)   //A70LV_Doulas_0377
    {
        dvC341_Write(B0_POPLLCT,  0x08, 0);
        dvC341_Write(B0_POPLLCT,  0x09, 0);      //Enable PLL Control    //PLL Frquency Range From Higher than 160 MHz

        dvC341_Write(B0_POCLKCT,  0x00, 0);
    }
    else
    {
        dvC341_Write(B0_POPLLCT,  0x08, 0);
        dvC341_Write(B0_POPLLCT,  0x09, 0);          //Enable PLL Control    //PLL Frquency Range From 80~160 MHz

        dvC341_Write(B0_POCLKCT,  0x00, 0);     //A70LV_Doulas_0002
    }
    dvC341_Write(B0_IPPLLCT,     ucIPPLLCT, 0);
    MS_SLEEP(10);//10Ms
    dvC341_Write(B0_IPREFDIV, ucPoRefDiv, 0);
    dvC341_Write(B0_IPFBDIV,  ucPoFbDiv, 0);

	//MS_SLEEP(10);//wait for 10ms
	//dvC341_Write(B0_IPREFDIV   ,ucPoRefDiv,0);
	//dvC341_Write(B0_IPFBDIV    ,ucPoFbDiv,0);
#endif
}

///////////////////////////////////////////////////////////////////////////////////
///@brief dvC341_GetPanelIndex : get panel table index from panel id
///@param ePANEL_ID ePanelId - panel id
///@param PUINT8 pucIndex - panel index pointer
///@return INT32 - return error code
///@author Roger Chen
///////////////////////////////////////////////////////////////////////////////////

static eSCALER_EXEC_CODE dvC341_GetPanelIndex(ePANEL_ID ePanelId, PUINT8 pucIndex)
{
    UINT8 ucIndex = 0;

    if(ePanelId < ePANEL_ID_LAST)
    {
        for(ucIndex = 0; ucIndex < m_ucPanelTblCount; ucIndex++)
        {
            if(m_sPanelTable[ucIndex].ePanelId == ePanelId)
            {
                *pucIndex = ucIndex;
                return eSCALER_EXEC_CODE_PASS;
            }
        }
    }
    else
    {
        return eSCALER_EXEC_CODE_PANEL_ID_NOT_FOUND;
    }

    return eSCALER_EXEC_CODE_PANEL_TABEL_INDEX_NOT_FOUND;
}

void dvC341_SetIdual_2K(const eC341_CH_V ch_v)
{
//    dvC341_Write(B0_RTCT, RTCT_PIVSTHRU, 0);   //0x0000CBA9 //A70LV_Doulas_0002

    m_sChannelInfo[ch_v].sIdual_Act_Ofst.uiHst = 0;
    m_sChannelInfo[ch_v].sIdual_Act_Ofst.uiHw = 0;
    m_sChannelInfo[ch_v].sIdual_Act_Ofst.uiVst = 0;
    m_sChannelInfo[ch_v].sIdual_Act_Ofst.uiVw = 0;
}


void dvC341_BuildPicture_4K(const eC341_CH_V ch_v)
{
    if(!m_sChannelInfo[ch_v].bInit)
    {
        dbmsg("Not init %d", m_sChannelInfo[ch_v].bInit);
        return;
    }

    if(ch_v == eC341_CH_V0)
    {
        dvC341_SetInputPort_Output_Off(ch_v);  //A35G2_CDS_Simon_0035
    }

    //LOG_MSG(db_DV_SCALER, "B30_WPBLKCT %x\n",(UINT8)dvC341_Read(B30_WPBLKCT  ,0));

    dvC341_Calc_Scaler_InputAndOutput_Window_4K(ch_v);

    dvC341_Set_Sharpness(ch_v);  //A70LV_Doulas_0003


	//if(!palDataPath_BKInput_Status())    //G100_Steven_0050     //H2 wait review
		dvC341_Auto_Fill_Screen_Init();

    dvC341_SetOverlay_4K(ch_v, m_sChannelSetting[eC341_CH_V0].cPIP_PBP_ON);   //ZU860_Doulas_0017//A70LV_Doulas_0108
    //dvC341_SetInput_2K(ch_v);

    dvC341_WriteToBuffer(B16_IIMGCTCH1,0x00,0);
    dvC341_WriteToBuffer(B32_IIMGCTCH2,0x00,0);
    dvC341_WriteToBuffer(B48_IIMGCTCH3,0x00,0);
    dvC341_WriteToBuffer(B64_IIMGCTCH4,0x00,0);
    dvC341_Buffer_Flush();

	//dvC341_Set_Display_Window(ch,
	//	PM_IWIN_HST[ch],PM_IWIN_HW[ch],PM_IWIN_VST[ch],PM_IWIN_VW[ch],
	//	PM_OWIN_HST[ch],PM_OWIN_HW[ch],PM_OWIN_VST[ch],PM_OWIN_VW[ch]);
	dvC341_Set_ZoomShrink_4K(ch_v);
    dvC341_set_rtct_stop();  //simon check
    //dvC341_Field_Interlock_Transfer_Control_Start(ch_v);

    //dvC341_SetInputColor(eCH);      //A70LV_Doulas_0076 remove
    dvC341_Set_Color(ch_v);
	//dvC341_set_datapath(ch);
	dvC341_Set_IP(ch_v);

    dvC341_Set_flddly(ch_v);
    dvC341_ConfigOutput_4K(ch_v);

    if((m_sChannelInfo[eC341_CH_V0].ePanelTimingId == ePANEL_ID_1080P_120HZ) ||
       (m_sChannelInfo[eC341_CH_V0].ePanelTimingId == ePANEL_ID_WUXGA_120HZ))  //H30K_Doulas_0053 //H30K_Doulas_0022 fixed 4k frame sequential image fail
    {
        dvC341_SetInputMemoryBuffer_4K(ch_v);      //A70LV_Doulas_0154 Add
    }
	dvC341_Set_Flip(ch_v);

    //dvC341_SetInputPort_2K(ch_v);     //A35G2_CDS_Simon_0005  move to bottom
    if(ch_v == eC341_CH_V1)    //A70LV_Doulas_0108 fixed CH1 抖動
    {
        dvC341_Set_Flip(eC341_CH_V0);
    }

    //dvC341_Write(B0_RTCT, RTCT_NORMAL, 0);       //0x4321CBA9
    dvC341_set_rtct_normal();

    //dvC341_WaitIntct(ch_v, 0xF0);// wait POVS
    //dvC341_WaitIntct(ch_v, C341_INT_PI1VS);// wait POVS  //check output ports //A70LV_Larry_0001  //A35G2_CDS_Simon_0035
    dvC341_wait1_pivs(ch_v);

	//dvC341_Field_Interlock_Transfer_Control_End(ch_v);

    if(m_sChannelSetting[eC341_CH_V0].cPIP_PBP_ON)  //A35G2_CDS_Simon_0035
    {
        dvC341_wait1_pivs(ch_v);
        dvC341_wait1_pivs(ch_v);
        dvC341_wait1_pivs(ch_v);
        dvC341_wait1_pivs(ch_v);
    }

    dvC341_SetInputPort_2K(ch_v);  //A35G2_CDS_Simon_0005

    //LOG_MSG(db_ALWAYS, "(func:%s, line:%d)CH= %d ,%08Xh\r\n", __FUNCTION__, __LINE__,eCH,dvC341_Read(B13_OSFLD1CH1,0)); //A70LV_Doulas_0108
    //halC341_InputSourceSetting(0);

}

void dvC341_BuildPicture_4K_ChangePIPLayout(eC341_CH_V ch_v)    //A35G2_CDS_Simon_0001
{
    if(!m_sChannelInfo[ch_v].bInit)
    {
        return;
    }
    dvC341_Calc_Scaler_InputAndOutput_Window_4K(ch_v);
    //dvC341_Write(B0_RTCT, RTCT_STOP, 0);

	//if(!palDataPath_BKInput_Status())    //G100_Steven_0050      //H2 wait review
		dvC341_Auto_Fill_Screen_Init();

    dvC341_SetOverlay_4K(ch_v, m_sChannelSetting[eC341_CH_V0].cPIP_PBP_ON);   //ZU860_Doulas_0017//A70LV_Doulas_0108
    dvC341_Set_ZoomShrink_4K(ch_v);
    dvC341_set_rtct_stop();
    //dvC341_Field_Interlock_Transfer_Control_Start(ch_v);

    dvC341_Set_IP(ch_v);

    dvC341_Set_Flip(ch_v); //A35G2_CDS_Larry_0015

    if(ch_v == eC341_CH_V1)    // fixed CH1 抖動 //A35G2_CDS_Larry_0015
    {
        dvC341_Set_Flip(eC341_CH_V0);
    }

    //dvC341_Write(B0_RTCT, RTCT_NORMAL, 0);       //0x4321CBA9
    dvC341_set_rtct_normal();

    //dvC341_WaitIntct(ch_v, C341_INT_PO1VS);// wait POVS  //check output ports //A70LV_Larry_0001
    dvC341_wait1_povs(ch_v);

	//dvC341_Field_Interlock_Transfer_Control_End(ch_v);

    if(ch_v == eC341_CH_V0)
    {
        dvC341_wait1_povs(ch_v);
        dvC341_wait1_povs(ch_v);
        dvC341_wait1_povs(ch_v);
        dvC341_wait1_povs(ch_v);
    }

    dvC341_SetInputPort_2K(ch_v);

}


//simon check
eSCALER_EXEC_CODE dvC341_PSQ_EXE(ePSQ_STATE eState)
{
    static UINT8 ucState = 0x00;

    if(eState == ePSQ_STATE_RUN)
    {
        ucState |= 0x01;    //ENABLE
    }
    else if(eState == ePSQ_STATE_PAUSE)
    {
        ucState |= 0x08;    //PAUSE
    }
    else if(eState == ePSQ_STATE_RESTART)
    {
        ucState &= 0xF7;    //RESTART
    }
    else if(eState == ePSQ_STATE_STOP)
    {
        ucState = 0x00;    //STOP
    }

    dvC341_Write(BN_PSQCT , ucState, 0);

    return eSCALER_EXEC_CODE_PASS;

}

//C341 Brightness data range : -511 ~ 511 (defalt:0)
INT8 dvC341_Brightness_Value_Set (UINT8 ucCH,INT16 iBrightness)    //A70LV_Doulas_0022
{
    if ( iBrightness>C341_BRIGHTNESS_MAX || iBrightness< C341_BRIGHTNESS_MIN )  //A70LV_Doulas_0035
    {
        return ERROR_C341_DATA_OUT_OF_RANGE;
    }

    LOG_MSG(db_DV_SCALER, "(func:%s, line:%d): Brightness=%d\r\n", __FUNCTION__, __LINE__, iBrightness);

    m_sChannelSetting[ucCH].iBrightness = iBrightness;

    return C341_PASS;
}


INT16 dvC341_Brightness_Value_Get(void)
{
    return m_sChannelSetting[eC341_CH_V0].iBrightness;
}


//C341 Contrast data range : 0 ~ 3.99 (default:1)
INT8 dvC341_Contrast_Value_Set (UINT8 ucCH,DOUBLE dContrast)   //A70LV_Doulas_0022
{
    if ( dContrast>3.99 || dContrast<0 )
    {
        return ERROR_C341_DATA_OUT_OF_RANGE;
    }

    LOG_MSG(db_DV_SCALER, "(func:%s, line:%d): Contrast=%f\r\n", __FUNCTION__, __LINE__, dContrast);

    m_sChannelSetting[ucCH].dContrast = dContrast;

    return C341_PASS;
}


DOUBLE dvC341_Contrast_Value_Get ()
{
    return m_sChannelSetting[eC341_CH_V0].dContrast;
}


eSCALER_EXEC_CODE dvC341_Init_4K(const eC341_CH_V ch_v, const ePANEL_ID ePanelId)
{
    UINT8 ucTblIndex = 0;
    eSCALER_EXEC_CODE eExecCode = eSCALER_EXEC_CODE_PASS;
    sACTIVE_AREA sOact; //A70LV_Doulas_0002

    LOG_MSG(db_DV_SCALER, "(func:%s, line:%d)CH= %d\r\n", __FUNCTION__, __LINE__,ch_v);  //A70LV_Doulas_0003

    if(ch_v >= eC341_CH_VNUMBER)
    {
        return eSCALER_EXEC_CODE_CH_OVER_RANGE;
    }

    //To make sure channel 1 will be inited before other channels.
    //Because no matter 4K or 2K are all refer to channel PLL
    if(ch_v > eC341_CH_V0)
    {
        if(m_sChannelInfo[eC341_CH_V0].bInit == FALSE)
        {
            return eSCALER_EXEC_CODE_CH1_NOT_INITED;
        }
    }


    if((eExecCode = dvC341_GetPanelIndex(ePanelId, &ucTblIndex)) != eSCALER_EXEC_CODE_PASS)
    {
        LOG_MSG(db_DV_SCALER, "(func:%s, line:%d) C341 Get Panel Timing Index Fail = %d\n", __FUNCTION__, __LINE__, eExecCode);
        return eExecCode;
    }

    LOG_MSG(db_DV_SCALER, "\nC341 Get Panel Index = %d\n", ucTblIndex);

    if(ch_v == eC341_CH_V0)   //H2PF_Simon_0093 Start
    {
        dvC341_Write(BN_RGBNK, 0x00, 0);
        dvC341_Write(BN_RSTCT, 0x01, 0);  //Soft Reset
        MS_SLEEP(10);
    }   //H2PF_Simon_0093 End

    //halC341Init();
    //TODO 20160705
    dvC341_GetCHOutputTiming_2K(ch_v, ucTblIndex);


    m_sChannelInfo[ch_v].bInit = TRUE;
    m_sChannelInfo[ch_v].ePanelTimingId = ePanelId;

    m_sChannelInfo[ch_v].ucIPPLLCT = 0x00;//0x01;        //A70LV_Doulas_0002
    m_sChannelInfo[ch_v].ulInterruptEnable = 0x00000000;//0x00000F00; //A70LV_Doulas_0002 modify
    m_sChannelInfo[ch_v].bForceSyncReset = FALSE; //TRUE; //A70LV_Doulas_0083 //A70LV_Doulas_0002     A70LV_Doulas_0154
    m_sChannelInfo[ch_v].bOutputChSwap = FALSE;

    m_sChannelInfo[ch_v].sInputTimingInfo.dHFreq = 0;    //A70LV_Doulas_0005 Add
    m_sChannelInfo[ch_v].sInputTimingInfo.dFrameRate = 0;
    m_sChannelInfo[ch_v].sInputTimingInfo.uiHStart = 0;
    m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive = 0;
    m_sChannelInfo[ch_v].sInputTimingInfo.uiVStart = 0;
    m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive = 0;
    m_sChannelInfo[ch_v].sInputTimingInfo.uiHTotal = 0;
  //  m_sChannelInfo[ch_v].sInputTimingInfo.uiHTotal_MCLK = 0;  //A70LV_Doulas_0005 remove
    m_sChannelInfo[ch_v].sInputTimingInfo.uiVTotal = 0;
    m_sChannelInfo[ch_v].sInputTimingInfo.eScanMode = eSCAN_MODE_NO_SIGNAL;

    m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.dHFreq = 0;     //A70LV_Doulas_0005 Add
    m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.dFrameRate = 0;
    m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.uiHStart = 0;
    m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.uiHActive = 0;
    m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.uiVStart = 0;
    m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.uiVActive = 0;
    m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.uiHTotal = 0;
 //   m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.uiHTotal_MCLK = 0; //A70LV_Doulas_0005 remove
    m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.uiVTotal = 0;
    m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.eScanMode = eSCAN_MODE_NO_SIGNAL;

	g_freeze_isel[ch_v][0] = 0;
	g_freeze_isel[ch_v][1] = 1;
	g_freeze_isel[ch_v][2] = 2;
	g_freeze_isel[ch_v][3] = 3;
	g_freeze_isel[ch_v][4] = 4;

    dvC341_InputPort_Set();

    m_sChannelInfo[ch_v].ucMCT = 0;
    m_sChannelInfo[ch_v].ucISYCT = 0;
    m_sChannelInfo[ch_v].ucOIMGCT = 0;
    m_sChannelInfo[ch_v].ucICFMT = 0;

	m_sChannelInfo[ch_v].ucSCFMFMTENB = 0;
	m_sChannelInfo[ch_v].ucSCFMFMT = 0;
	m_sChannelInfo[ch_v].ucLAYOUT_CHG = 0;


    m_sChannelInfo[ch_v].ucPanelIndex = ucTblIndex;
    m_sChannelInfo[ch_v].eInputSyncType = eINPUT_PC_SYNC;   //A70LV_Doulas_0005 Add
    m_sChannelInfo[ch_v].bAutoPosition = FALSE;         //A70LV_Doulas_0118

#if (CURRENT_MAIN_BOARD == ICHIP_BOARD)
    //Should be the same with memory clock 12x66 = 792M (792/2)
    m_sChannelInfo[ch_v].ulMclko_Freq = 800000000;//350000000;
#else
    //Should be the same with memory clock
    m_sChannelInfo[ch_v].ulMclko_Freq = 800000000;//330000000;
#endif

    m_sChannelInfo[ch_v].ulFMCLK = m_sChannelInfo[ch_v].ulMclko_Freq / 4;

    ucPM_HNR[ch_v] = 0;              //A70LV_Doulas_0145
    ucPM_VNR[ch_v] = 0;
    ucPM_MVFLT[ch_v] = 3;//default
    ucPM_DIAG[ch_v] = 0;//default

    if(ch_v == eC341_CH_V0)
    {
        memcpy(g_vboinfo, g_vboinfo_inittable ,sizeof(g_vboinfo_inittable));
    }

#if 0
    halC341Init();
	halC341_InputSourceSetting(0);
    halC341_InputSourceSetting(1);
#else

	for(UINT32 i=0; i<4; i++)
	{
		dvC341_Write((B0_MCT1 + (i<<8)), 0x11, 0);
	}

    dvC341_Write(B0_PICT, 0x1111, 0);//ch1to4 DDRin

    LOG_MSG(db_DV_SCALER, "===> (func:%s, line:%d) B0_PICT = 0x%X\n", __FUNCTION__, __LINE__, dvC341_Read(B0_PICT, 0));

    dvC341_VBO_Init();
    dvC341_RegInit();

    for(UINT32 i=0; i<4; i++)
    {
	    dvC341_Write((B0_PI1SWP + (i << 8)), 0x04, 0);
	}

    dvC341_Write(B0_PIDSKW, 0x0c, 0);


#ifdef SWAP_VBO_RX0_LANE
    dvC341_Write(B0_VI1CT, 0x0404 | 0xC8C8, 0);
#else
    dvC341_Write(B0_VI1CT, 0x0404, 0);
#endif

    dvC341_Write(B0_VI2CT, 0x0404, 0);
    dvC341_Write(B0_VIDSKW, 0x0202, 0);


#ifdef SWAP_VBO_RX0_LANE
    dvC341_Write(B0_ICHSEL1, 0x0A, 0);
    dvC341_Write(B0_ICHSEL2, 0x08, 0);
    dvC341_Write(B0_ICHSEL3, 0x0C, 0);
    dvC341_Write(B0_ICHSEL4, 0x0E, 0);
#else
    dvC341_Write(B0_ICHSEL1, 0x08, 0);
    dvC341_Write(B0_ICHSEL2, 0x0A, 0);
    dvC341_Write(B0_ICHSEL3, 0x0C, 0);
    dvC341_Write(B0_ICHSEL4, 0x0E, 0);
#endif

    //Only need to do once in channel 1
    if(ch_v == eC341_CH_V0)
    {
        dvC341_ConfigClock(m_sPanelTable[ucTblIndex].sOutputTimingInfo.ucPoRefDiv,
                           m_sPanelTable[ucTblIndex].sOutputTimingInfo.ucPoFbDiv,
                           m_sChannelInfo[ch_v].ucIPPLLCT);

        //halC341_set_fix_reg();
        dvC341_ConfigBasicMemory_2K();
    }

    sOact.uiHst =0; //A70LV_Doulas_0002
    sOact.uiHw =0;
    sOact.uiVst =0;
    sOact.uiVw =0;
    sOact.iHOffset =0;
    sOact.iVOffset =0;
    dvC341_SetMemAD_2K(ch_v,&sOact);        //A70LV_Doulas_0002

    dvC341_ConfigIPMemory_2K(ch_v);

    if(ch_v == eC341_CH_V0) //A35G2_CDS_Larry_0030
    {
        //dvC341_ConfigLVDS_2K(ePanelId);
    }

    dvC341_ConfigInput_2K(ch_v);
    dvC341_ConfigIPInput_2K(ch_v);//De-Interlacer
    dvC341_ConfigOutput_4K(ch_v);

    dvC341_Init_Default_Value(ch_v); //A70LV_Doulas_0002

    //Only need to do once in channel 1
    if(ch_v == eC341_CH_V0)
    {
        dvC341_ConfigZoomLut(ch_v);
        dvC341_SetIdual_2K(eC341_CH_V0);
    }

    dvC341_ConfigGamma2_2K(ch_v);
    dvC341_Set_Color_Correction(ch_v,0,0,m_sChannelSetting[ch_v].ucSkinColor,3);  //A70LV_Doulas_0029 modify  //A70LV_Doulas_0003
#endif

    return eExecCode;
}


//Vx1 initial
void dvC341_VBO_Init(void)
{
    ///////////////////////////////////////////////////////////
	//  VbO
	///////////////////////////////////////////////////////////
                                        // VbyOne-RX CH1 setup Seq.(4byteMode/RGB30)
    dvC341_Write(B2_VBOCT, 0x0e, 0);        // [  0] Reset Enable  V-by-One RX 1
    dvC341_Write(B2_VBOCT, 0x0f, 0);        // [  0] Reset Disable V-by-One RX 1
    dvC341_Write(B2_VBORX1_FMT, 0x05, 0);        // [1:0] Byte mode    = 4-byte
                                            //dvC341_Write(B2_VBORX1_FMT,    0x08, 0);        // [1:0] Byte mode    = 3-byte
                                            //                        [5:2] Color Format = RGB/YUV444 30bit
    dvC341_Write(B2_VBORX1_LANECT, 0xC0, 0);        // [7:0] Lane1 Enable
    dvC341_Write(B2_VBORX1_OPT1, 0xa3, 0);        // [  3] Data Rate = 2.0-4.0 Gbps

                                                // VbyOne-RX CH2 setup Seq.(4byteMode/RGB30)
    dvC341_Write(B2_VBOCT, 0x0d, 0);        // [  1] Reset Enable  V-by-One RX 2
    dvC341_Write(B2_VBOCT, 0x0f, 0);        // [  1] Reset Disable V-by-One RX 2
    dvC341_Write(B2_VBORX2_FMT, 0x05, 0);        // [1:0] Byte mode    = 4-byte
                                            //dvC341_Write(B2_VBORX2_FMT,    0x08, 0);        // [1:0] Byte mode    = 3-byte
                                            //                        [5:2] Color Format = RGB/YUV444 30bit
    dvC341_Write(B2_VBORX2_LANECT, 0x80, 0);        // [7:0] Lane1 Enable
    dvC341_Write(B2_VBORX2_OPT1, 0xa3, 0);        // [  3] Data Rate = 2.0-4.0 Gbps

                                                // VbyOne-TX CH1 setup Seq.(4byteMode/RGB30)

    dvC341_Write(B2_VBOCT, 0x0b, 0);        // [2]TX1RSTB=0(Rst ENB.)
    dvC341_Write(B2_VBOCT, 0x0f, 0);        // [2]TX1RSTB=1(Rst DIS.)
    dvC341_Write(B2_VBOTX1_PWR, 0x00, 0);        // [0]TX1PWR=0(PWR OFF)
    dvC341_Write(B2_VBOTX1MD, 0x0011, 0);    // [15]STRM_DATALOCK=0(Not Locked) / [7:4]Color=0001(RGB/444 30bit) / [1:0]ByteMode=01(4Byte)

    if (OUTPUT_CH_NUM == 2)			// QFHD
    {
	    dvC341_Write(B2_VBOTX1_LANECTRL, 0xff, 0);
	}
	else
	{
	    dvC341_Write(B2_VBOTX1_LANECTRL, 0x03, 0);
	}
    dvC341_Write(B2_VBOTX1_FMT, 0x11, 0);        // [5:4]BMOD=01(4Byte) / [3:0]CFMT=0001(RGB/444 30bit)
    dvC341_Write(B2_VBOTX1_CTR1, (0x4c), 0);        // [7:6]RS=01 / [3:2]FRG=11(40MHz <= FREF <= 133.33MHz)

    dvC341_Write(B2_VBOTX1_PWR, 0x01, 0);        // [0]TX1PWR=1(PWR ON)
    if (OUTPUT_CH_NUM == 2)					// QFHD
    {
	    dvC341_Write(B2_VBOTX1CTL, 0x09, 0);
	}
	else
	{
	    dvC341_Write(B2_VBOTX1CTL, 0x05, 0);
	}
    dvC341_Write(B2_VBOTX1MD, 0x8011, 0);    // [15]STRM_DATALOCK=1(Locked) / [7:4]Color=0001(RGB/444 30bit) / [1:0]ByteMode=01(4Byte)

                                        // VbyOne-TX CH2 setup Seq.(4byteMode/RGB30)

    dvC341_Write(B2_VBOCT, 0x07, 0);        // [3]TX2RSTB=0(Rst ENB.)
    dvC341_Write(B2_VBOCT, 0x0f, 0);        // [3]TX2RSTB=1(Rst DIS.)
    dvC341_Write(B2_VBOTX2_PWR, 0x00, 0);        // [0]TX2PWR=0(PWR OFF)
    dvC341_Write(B2_VBOTX2MD, 0x0011, 0);    // [15]STRM_DATALOCK=0(Not Locked) / [7:4]Color=0001(RGB/444 30bit) / [1:0]ByteMode=01(4Byte)

    if (OUTPUT_CH_NUM == 2)				// QFHD
    {
	    dvC341_Write(B2_VBOTX2_LANECTRL, 0xff, 0);
	}
	else
	{
	    dvC341_Write(B2_VBOTX2_LANECTRL, 0x03, 0);
	}
    dvC341_Write(B2_VBOTX2_FMT, 0x11, 0);        // [5:4]BMOD=01(4Byte) / [3:0]CFMT=0001(RGB/444 30bit)
    dvC341_Write(B2_VBOTX2_CTR1, 0x4c, 0);        // [7:6]RS=01 / [3:2]FRG=11(40MHz <= FREF <= 133.33MHz)

    dvC341_Write(B2_VBOTX2_PWR, 0x01, 0);        // [0]TX2PWR=1(PWR ON)
    if (OUTPUT_CH_NUM == 2)				// QFHD
    {
	    dvC341_Write(B2_VBOTX2CTL, 0x09, 0);        // [3:2]PARAMD=01(2para) / [0]VBOEN=1(ENB)
	}
	else
	{
	    dvC341_Write(B2_VBOTX2CTL, 0x05, 0);        // [3:2]PARAMD=01(2para) / [0]VBOEN=1(ENB)
	}
    dvC341_Write(B2_VBOTX2MD, 0x8011, 0);    // [15]STRM_DATALOCK=1(Locked) / [7:4]Color=0001(RGB/444 30bit) / [1:0]ByteMode=01(4Byte)

}

void dvC341_VBO_Init_3D_Panel(void) //H30K_Doulas_0001
{
    ///////////////////////////////////////////////////////////
	//  VbO
	///////////////////////////////////////////////////////////
                                                // VbyOne-TX CH1 setup Seq.(4byteMode/RGB30)

    dvC341_Write(B2_VBOCT, 0x0b, 0);        // [2]TX1RSTB=0(Rst ENB.)
    dvC341_Write(B2_VBOCT, 0x0f, 0);        // [2]TX1RSTB=1(Rst DIS.)
    dvC341_Write(B2_VBOTX1_PWR, 0x00, 0);        // [0]TX1PWR=0(PWR OFF)
    dvC341_Write(B2_VBOTX1MD, 0x0011, 0);    // [15]STRM_DATALOCK=0(Not Locked) / [7:4]Color=0001(RGB/444 30bit) / [1:0]ByteMode=01(4Byte)

    if (OUTPUT_CH_NUM == 2)			// QFHD
    {
	    dvC341_Write(B2_VBOTX1_LANECTRL, 0xff, 0);
	}
	else
	{
	    dvC341_Write(B2_VBOTX1_LANECTRL, 0x03, 0);
	}
    dvC341_Write(B2_VBOTX1_FMT, 0x11, 0);        // [5:4]BMOD=01(4Byte) / [3:0]CFMT=0001(RGB/444 30bit)
    dvC341_Write(B2_VBOTX1_CTR1, (0x4c), 0);        // [7:6]RS=01 / [3:2]FRG=11(40MHz <= FREF <= 133.33MHz)

    dvC341_Write(B2_VBOTX1_PWR, 0x01, 0);        // [0]TX1PWR=1(PWR ON)
    if (OUTPUT_CH_NUM == 2)					// QFHD
    {
	    dvC341_Write(B2_VBOTX1CTL, 0x09, 0);
	}
	else
	{
	    dvC341_Write(B2_VBOTX1CTL, 0x05, 0);
	}
    dvC341_Write(B2_VBOTX1MD, 0x8011, 0);    // [15]STRM_DATALOCK=1(Locked) / [7:4]Color=0001(RGB/444 30bit) / [1:0]ByteMode=01(4Byte)


}


void dvC341_RegInit(void)
{
    if((m_sChannelInfo[eC341_CH_V0].ePanelTimingId == ePANEL_ID_1080P_120HZ) ||
       (m_sChannelInfo[eC341_CH_V0].ePanelTimingId == ePANEL_ID_WUXGA_120HZ)) //H30K_Doulas_0001
    {
        dvC341_Write(B0_PIDSKW, 0x00, 0);

#ifdef SWAP_VBO_RX0_LANE
        dvC341_Write(B0_VI1CT, 0x0404 | 0xC8C8, 0);
#else
        dvC341_Write(B0_VI1CT, 0x0404, 0);
#endif

        dvC341_Write(B0_VI2CT, 0x0011, 0);
        dvC341_Write(B0_VIDSKW, 0x0402, 0);

        // ; C341B6 registers
        dvC341_Write(B6_PISYPOLCTRL, 0x00000007, 0);
        dvC341_Write(B6_VISYPOLCTRL, 0x33333333, 0);

        // ; C341B7 registers
	    //dvC341_Write(B14_OVLCHSELCH1, 0x0000eca0 , 0);				// CH1(Left  Side) [15: 8] = 1110_1100_1010_0000     LYR3=CH7 LYR2=CH5 LYR1=CH3 LYR0=CH1
	    //dvC341_Write(B30_OVLCHSELCH2, 0x0000fdb1 , 0);
        dvC341_SetOverlay_4K(eC341_CH_V0, m_sChannelSetting[eC341_CH_V0].cPIP_PBP_ON);

	    // initialize regiters of freeze function
	    for (UINT8 i = 0; i < 4; i++)
	    {
		    dvC341_Write(B22_ISLFRZCTCH1, 0x00, 16*i);
		    dvC341_Write(B22_ISLFRZFSELCH1, 0x043210, 16*i);
		    dvC341_Write(B22_OSLFRZCTCH1, 0x00, 16*i);
		    dvC341_Write(B22_OSLFRZFSELCH1, 0x2103, 16*i);
	    }

	    for(UINT8 i = 0; i <= 4; i++) //H30K_Doulas_0072
	    {
	        g_freeze_isel[0][i] = i;
		    g_freeze_isel[1][i] = i;
        }

	    #ifdef SWAP_VBO_RX0_LANE
        dvC341_Write(B0_ICHSEL1, 0x0A, 0);
        dvC341_Write(B0_ICHSEL2, 0x0A, 0);
        dvC341_Write(B0_ICHSEL3, 0x0C, 0);
        dvC341_Write(B0_ICHSEL4, 0x0E, 0);
        #else
        dvC341_Write(B0_ICHSEL1, 0x08, 0);
        dvC341_Write(B0_ICHSEL2, 0x08, 0);
        dvC341_Write(B0_ICHSEL3, 0x0C, 0);
        dvC341_Write(B0_ICHSEL4, 0x0E, 0);
        #endif
    }
    else //2D
    {
        dvC341_Write(B0_PIDSKW, 0x0c, 0);

#ifdef SWAP_VBO_RX0_LANE
        dvC341_Write(B0_VI1CT, 0x0404 | 0xC8C8, 0);
#else
        dvC341_Write(B0_VI1CT, 0x0404, 0);
#endif

        dvC341_Write(B0_VI2CT, 0x0404, 0);
        dvC341_Write(B0_VIDSKW, 0x0202, 0);

        // ; C341B6 registers
        dvC341_Write(B6_PISYPOLCTRL, 0x77777777, 0);
        dvC341_Write(B6_VISYPOLCTRL, 0x33333333, 0);

        // ; C341B7 registers
	    //dvC341_Write(B14_OVLCHSELCH1, 0x0000eca0 , 0);				// CH1(Left  Side) [15: 8] = 1110_1100_1010_0000     LYR3=CH7 LYR2=CH5 LYR1=CH3 LYR0=CH1
	    //dvC341_Write(B30_OVLCHSELCH2, 0x0000fdb1 , 0);
        dvC341_SetOverlay_4K(eC341_CH_V0, m_sChannelSetting[eC341_CH_V0].cPIP_PBP_ON);

	    // initialize regiters of freeze function
	    for (UINT8 i = 0; i < 4; i++)
	    {
		    dvC341_Write(B22_ISLFRZCTCH1, 0x01, 16*i);
		    dvC341_Write(B22_ISLFRZFSELCH1, 0x043210, 16*i);
		    dvC341_Write(B22_OSLFRZCTCH1, 0x00, 16*i);
		    dvC341_Write(B22_OSLFRZFSELCH1, 0x2103, 16*i);
		}
		for(UINT8 i = 0; i <= 4; i++) //H30K_Doulas_0072
	    {
	        g_freeze_isel[0][i] = i;
		    g_freeze_isel[1][i] = i;
        }
		#ifdef SWAP_VBO_RX0_LANE
        dvC341_Write(B0_ICHSEL1, 0x0A, 0);
        dvC341_Write(B0_ICHSEL2, 0x08, 0);
        dvC341_Write(B0_ICHSEL3, 0x0C, 0);
        dvC341_Write(B0_ICHSEL4, 0x0E, 0);
        #else
        dvC341_Write(B0_ICHSEL1, 0x08, 0);
        dvC341_Write(B0_ICHSEL2, 0x0A, 0);
        dvC341_Write(B0_ICHSEL3, 0x0C, 0);
        dvC341_Write(B0_ICHSEL4, 0x0E, 0);
        #endif
	}

    // ; C341B14 registers
    dvC341_Write(B14_OVLCTCH1,      0x01,   0);
    dvC341_Write(B14_OVLAREACH1,    0x005,  0);
    dvC341_Write(B30_OVLCTCH2,      0x11,   0);
    dvC341_Write(B30_OVLAREACH2,    0x005,  0);
    dvC341_Write(B46_OVLCTCH3,      0x21,   0);
    dvC341_Write(B46_OVLAREACH3,    0x005,  0);
    dvC341_Write(B62_OVLCTCH4,      0x31,   0);
    dvC341_Write(B62_OVLAREACH4,    0x005,  0);

    dvC341_Write(B64_DIFCTCH4, 0x80, 0);


}


void C341Test(void)
{
#if 0   //simon check ... not implement

#if 0
    extern unsigned int POSD_SAD;
    extern unsigned int POSD_SAD0;
    extern unsigned int POSD_SAD1;
#endif
    UINT32 ucData = 0 ;
    UINT8 ucIdx;
    {
#if 0
        ucData = dvC341_Read(B0_RTCT, 0);
        printf("\r\nC341 read B0_RTCT = %d\r\n" , ucData) ;

        ucData = dvC341_Read(B0_BOWIHN , 0) ;
        printf("\r\nC341 read B0_BOWIHN = %d\r\n" , ucData) ;

        ucData = dvC341_Read(B17_ISYCTCH1, 0);
        printf("\r\nC341 read B17_ISYCTCH1 = %d\r\n" , ucData) ;
#endif


#if 1
//        ucData = dvC341_Read(B0_OSDCT, 0);
//        printf("\r\nC341 read B0_OSDCT = %d\r\n" , ucData) ;

        for(ucIdx=1; ucIdx<0xFF; ucIdx++)
        {
            dvC341_Write(B0_OSDCT , ucIdx , 0) ;
            ucData = (dvC341_Read(B0_OSDCT , 0) & 0xFF) ;
            if(ucIdx != ucData)
            {
                LOG_MSG(db_ALWAYS, "ucIdx[%d] != Data[%d]\r\n", ucIdx, ucData) ;
            }
        }

//        POSD_SAD0 = DEF_OSDSAD_LAYER0_4K & 0x3fffffff;
//        POSD_SAD1 = DEF_OSDSAD_LAYER1_4K & 0x3fffffff;
//        POSD_SAD = ( POSD_SAD != POSD_SAD0 ) ? POSD_SAD0 : POSD_SAD1;
//
//        printf("write POSD_SAD %X\r\n",POSD_SAD);
//        dvC341_Write(B3_OSDSAD1CH1, POSD_SAD/*POSD_SAD_CH1*/ & 0xffffffff, 0);
//
//        printf("read POSD_SAD\r\n");
//        for(ucIdx=0; ucIdx<4; ucIdx++)
//        {
//            ucData = dvC341_Read(0x031A01+0x100*ucIdx , 0);
//            printf("Reg %X, = %d\r\n", 0x031A01+0x100*ucIdx, ucData);
//        }
#endif
//        dvC341_Write(B12_FCGDTCH1 , 0x3e , 0) ;
//        dvC341_Write( B3_PLTDT1CH1, 0xea, 0 );
//        dvC341_Write(B1_PSQIMDT, 0x68, 0);
//        ucData = dvC341_Read(B12_FCGDTCH1 , 0) ;
//        printf("Data = %d\r\n" , ucData) ;
//
//        ucData = dvC341_Read(B3_OSDSAD1CH1 , 0);
//        printf("B3_OSDSAD1CH1 Data = %X\r\n" , ucData);
    }
#endif
}

void dvC341_SetOutputTestPattern(void)
{
    dvC341_Set_Output_Test_Pattern(TRUE,1);
}


//#define C341_REG_ONLY_DUMP_TO_FILE

void dvC341_RegDump(void)   //Owen_G100_0060
{
    UINT32 dwData = 0;
    UINT32 dwCount = 0;

    sICHIP_DUMP* m_pC341Reg = (sICHIP_DUMP*)malloc(sizeof(m_sICHIPC341_DUMP));

    if(m_pC341Reg == NULL)
    {
        LOG_MSG(db_ALWAYS,"\n%s(%d) malloc fail\n", __FUNCTION__ , __LINE__);
        return;
    }

    memcpy(m_pC341Reg, m_sICHIPC341_DUMP, sizeof(m_sICHIPC341_DUMP));

#if 0
    FILE *pRegFile = NULL;
    pRegFile = fopen("/usr/configs/scaler/system/C341_Reg", "rb");
    if(pRegFile != NULL)
    {
        if(fread(m_C341RegTbl, sizeof(m_C341RegTbl), 1, pRegFile) != 1)
        {
            LOG_MSG(db_ALWAYS, "Open C341 reg file failed !\r\n");
            fclose(pRegFile);
            return;
        }
        fclose(pRegFile);
    }
    else
    {
        LOG_MSG(db_ALWAYS, "Open C341 reg file failed !\r\n");
        return;
    }
#endif /* 0 */

    if(m_pC341Reg[dwCount].ulAddress == 0x12345678)
    {
        FILE *pDumpFile = NULL;
        char pString[128]={'\0'};
        LOG_MSG(db_ALWAYS, "C341 %s\r\n",  m_pC341Reg[dwCount].cReg);
        //pDumpFile = fopen("/usr/configs/scaler/system/C341_dump", "wb+");
        pDumpFile = fopen("/home/root/C341RegDump.txt", "wb+");
        if(pDumpFile == NULL)
        {
            LOG_MSG(db_ALWAYS, "Open C341 dump file failed !\r\n");
            free(m_pC341Reg);
            return;
        }

        dwCount++;

        while(1)
        {
            if(m_pC341Reg[dwCount].ulAddress == 0xFFFFFFFF)
            {
                LOG_MSG(db_ALWAYS, "C341 %s\r\n",  m_pC341Reg->cReg);
                fclose(pDumpFile);
                free(m_pC341Reg);

                SYSTEM_CALL("cp -R %s %s", "/home/root/C341RegDump.txt", UPGRADE_LOG_PATH); //H30K_Doulas_0067 dump register to OPD
                return;
            }
            else
            {
                dwData = dvC341_Read(m_pC341Reg[dwCount].ulAddress, 0);

                memset(pString,'\0',128);

                switch(m_pC341Reg[dwCount].ulAddress & 0xFF)
                {
                    case 1:
                        #ifndef C341_REG_ONLY_DUMP_TO_FILE
                        LOG_MSG(db_ALWAYS, "%s %02xh\r\n",  m_pC341Reg[dwCount].cReg, dwData);
                        #endif
                        sprintf(pString,"%s %02xh\r\n",  m_pC341Reg[dwCount].cReg, dwData);
                        fwrite(pString, sizeof(char), strlen(pString), pDumpFile);
                        break;
                    case 2:
                        #ifndef C341_REG_ONLY_DUMP_TO_FILE
                        LOG_MSG(db_ALWAYS, "%s %04xh\r\n",  m_pC341Reg[dwCount].cReg, dwData);
                        #endif
                        sprintf(pString,"%s %04xh\r\n",  m_pC341Reg[dwCount].cReg, dwData);
                        fwrite(pString, sizeof(char), strlen(pString), pDumpFile);
                        break;
                    case 3:
                        #ifndef C341_REG_ONLY_DUMP_TO_FILE
                        LOG_MSG(db_ALWAYS, "%s %06xh\r\n",  m_pC341Reg[dwCount].cReg, dwData);
                        #endif
                        sprintf(pString,"%s %06xh\r\n",  m_pC341Reg[dwCount].cReg, dwData);
                        fwrite(pString, sizeof(char), strlen(pString), pDumpFile);
                        break;
                    case 4:
                        #ifndef C341_REG_ONLY_DUMP_TO_FILE
                        LOG_MSG(db_ALWAYS, "%s %08xh\r\n",  m_pC341Reg[dwCount].cReg, dwData);
                        #endif
                        sprintf(pString,"%s %08xh\r\n",  m_pC341Reg[dwCount].cReg, dwData);
                        fwrite(pString, sizeof(char), strlen(pString), pDumpFile);
                        break;
                    default:
                        break;
                 }

                 #ifdef C341_REG_ONLY_DUMP_TO_FILE
                 MS_SLEEP(0);
                 #else
                 MS_SLEEP(5);
                 #endif
            }

            dwCount++;
        }

    }
    else
    {
        LOG_MSG(db_ALWAYS, "Read C341 reg file failed !\r\n");

        free(m_pC341Reg);
        return;
    }
}

void dvC341_Bypass(void)    //A70LV_Doulas_0002
{
#if 0   //simon check ... not implement
    dvC341_Write(B0_RTCT,0,0);
#if 0
    halC341_Write(B0_CMOSOUTSETUP, 0x02,0);
    halC341_Write(B0_POPLLCT,0x80,0);

    halC341_Write(B0_LVDSOSETUP, 0x25,0);

    halC341_Write(B0_LVDSO1CT,0x02,0);
    halC341_Write(B0_LVDSTXCT1,0x00,0);

    utilDelayMs(100);

    halC341_Write(B0_LVDSTXCT1,0x09,0);
#endif /* 0 */

#if 1
    dvC341_Write(BN_RSTCT,0x01,0);
    dvC341_Write(B0_LVDSTXCT1   , 0x00 ,0);// for 120MHz-166MHz
    dvC341_Write(B0_LVDSTXCT2   , 0x00,0);
    MS_SLEEP(800);   //A70LV_Doulas_0008 modify

    dvC341_Write(BN_RSTCT,0x00,0);
    dvC341_Write(BN_MCT1,0x00,0);

    dvC341_Write(B0_POPLLCT, 0xa9,0);    // CH1 for 160MHz-166MHz
    dvC341_Write(B0_POREFDIV, 0x0a,0);
    dvC341_Write(B0_POFBDIV, 0x0a,0);

    dvC341_Write(B0_LVDSOCLKCT  , 0x01,0);
    dvC341_Write(B0_LVDSOSETUP  , 0x25,0);
    dvC341_Write(B0_LVDSO1CT    , 0x02,0);
    dvC341_Write(B0_LVDSO2CT    , 0x02,0);
    dvC341_Write(B0_LVDSTXCT1   , 0x09 ,0);// for 120MHz-166MHz
    dvC341_Write(B0_LVDSTXCT2   , 0x09,0);
#endif /* 0 */

    //halC341_Write(B8_AUTOFLDCH1,0x80,0);

    LOG_MSG(db_DV_SCALER, "dvC341_Bypass\n");
#endif
}

void dvC341_Test(void)  //A70LV_Doulas_0002
{
#if 0   //simon check ... not implement
   // dvC341_Write(B30_WPBLKCT  , 0x00,0);
   #if 0
   LOG_MSG(db_DV_SCALER, "B35_FLDHISTCH1 =0x%x\n",dvC341_Read(B35_FLDHISTCH1, 0));
   LOG_MSG(db_DV_SCALER, "B37_MSTS =0x%x\n",dvC341_Read(B37_MSTS, 0));
   LOG_MSG(db_DV_SCALER, "B29_MIMSK =0x%x\n",dvC341_Read(B29_MIMSK, 0));
   LOG_MSG(db_DV_SCALER, "B29_MIRQ =0x%x\n",dvC341_Read(B29_MIRQ, 0));
   LOG_MSG(db_DV_SCALER, "B8_FLDDLYCH1 =%d\r\n",dvC341_Read(B8_FLDDLYCH1, 0));
   LOG_MSG(db_DV_SCALER, "BN_MCT1 =0x%x\r\n",dvC341_Read(BN_MCT1, 0));
   LOG_MSG(db_DV_SCALER, "BN_MCT2 =0x%x\r\n",dvC341_Read(BN_MCT2, 0));

   LOG_MSG(db_DV_SCALER, "B14_OVLCTCH1 =0x%x\r\n",dvC341_Read(B14_OVLCTCH1, 0));
   LOG_MSG(db_DV_SCALER, "B11_VEGSELCH1 =0x%x\r\n",dvC341_Read(B11_VEGSELCH1, 0));
   LOG_MSG(db_DV_SCALER, "B35_IMFHCH1 =0x%x,0x%x\r\n",dvC341_Read(B35_IMFHCH1, 0),dvC341_Read(B35_IMFVCH1, 0));
   LOG_MSG(db_DV_SCALER, "B35_IMFHCH2 =0x%x,0x%x\r\n",dvC341_Read(B35_IMFHCH2, 0),dvC341_Read(B35_IMFVCH2, 0));
   LOG_MSG(db_DV_SCALER, "B35_PSTATCH1 =0x%x\r\n",dvC341_Read(B35_PSTATCH1, 0));
   LOG_MSG(db_DV_SCALER, "B35_PSTATCH2 =0x%x\r\n",dvC341_Read(B35_PSTATCH2, 0));
   LOG_MSG(db_DV_SCALER, "B35_STATECH1 =0x%x\r\n",dvC341_Read(B35_STATECH1, 0));
   LOG_MSG(db_DV_SCALER, "B35_STATECH2 =0x%x\r\n",dvC341_Read(B35_STATECH2, 0));

   LOG_MSG(db_DV_SCALER, "B0_IPPLLCT =0x%x\r\n",dvC341_Read(B0_IPPLLCT, 0));
   LOG_MSG(db_DV_SCALER, "B35_PH0CH1 =0x%x\r\n",dvC341_Read(B35_PH0CH1, 0));
   LOG_MSG(db_DV_SCALER, "B35_PV0CH1 =0x%x\r\n",dvC341_Read(B35_PV0CH1, 0));
   #endif
 //  dvC341_Write(B8_ACTCTCH1,0x40,0);    //A70LV_Doulas_0007 test


   LOG_MSG(db_DV_SCALER, "B35_IAPL0CH1 =0x%x\r\n",dvC341_Read(B35_IAPL0CH1, 0));
   LOG_MSG(db_DV_SCALER, "B35_IAPL0CH2 =0x%x\r\n",dvC341_Read(B35_IAPL0CH2, 0));
   LOG_MSG(db_DV_SCALER, "B8_ISYCTCH1 = 0x%x \r\n",dvC341_Read(B8_ISYCTCH1,0));
   LOG_MSG(db_DV_SCALER, "B35_PSTATCH1 =0x%x\r\n",dvC341_Read(B35_PSTATCH1, 0));
   LOG_MSG(db_DV_SCALER, "B35_STATECH1 =0x%x\r\n",dvC341_Read(B35_STATECH1, 0));
   LOG_MSG(db_DV_SCALER, "B35_IFACTHSTCH1 =0x%x\r\n",dvC341_Read(B35_IFACTHSTCH1, 0));
   //LOG_MSG(db_DV_SCALER, "B35_IMAXFILTRCH1 =0x%x\r\n",dvC341_Read(B35_IMAXFILTRCH1, 0));
   //LOG_MSG(db_DV_SCALER, "B35_IMINFILTRCH1 =0x%x\r\n",dvC341_Read(B35_IMINFILTRCH1, 0));
   LOG_MSG(db_ALWAYS, "B35_IFHCH1 =0x%x\r\n",dvC341_Read(B35_IFHCH1, 0));
   LOG_MSG(db_ALWAYS, "B35_IFVCH1 =0x%x\r\n",dvC341_Read(B35_IFVCH1, 0));
   LOG_MSG(db_ALWAYS, "B35_IFHCH2 =0x%x\r\n",dvC341_Read(B35_IFHCH2, 0));
   LOG_MSG(db_ALWAYS, "B35_IFVCH2 =0x%x\r\n",dvC341_Read(B35_IFVCH2, 0));
   LOG_MSG(db_ALWAYS, "B35_IMFHCH1 =0x%x\r\n",dvC341_Read(B35_IMFHCH1, 0));
   LOG_MSG(db_ALWAYS, "B35_IMFVCH1 =0x%x\r\n",dvC341_Read(B35_IMFVCH1, 0));
   LOG_MSG(db_ALWAYS, "B35_IMFHCH2 =0x%x\r\n",dvC341_Read(B35_IMFHCH2, 0));
   LOG_MSG(db_ALWAYS, "B35_IMFVCH2 =0x%x\r\n",dvC341_Read(B35_IMFVCH2, 0));

   LOG_MSG(db_ALWAYS, "B8_ISYCTCH1 =0x%x\r\n",dvC341_Read(B8_ISYCTCH1, 0));
   LOG_MSG(db_ALWAYS, "B15_ISYCTCH2 =0x%x\r\n",dvC341_Read(B8_ISYCTCH1, 13));

#endif
}

void dvC341_Set_HUE(eC341_CH_V ch_v)  //A70LV_Doulas_0022
{
    if(m_sChannelSetting[eC341_CH_V0].cPIP_PBP_ON == FALSE)        //A70LV_Doulas_0155
    {
        #if 0  //simon check
        if((m_sChannelInfo[eC341_CH_V0].ePanelTimingId == ePANEL_ID_1080P_120HZ) ||
           (m_sChannelInfo[eC341_CH_V0].ePanelTimingId == ePANEL_ID_WUXGA_120HZ))   //A70LV_Doulas_0377 Modify
        {

        }
        else
        #endif
        if(ch_v == eC341_CH_V1)   //simon check ???
        {
            return ;
        }
    }
    dvC341_Set_Color(ch_v);//A70LV_Doulas_0012 modify
}

INT16 dvC341_Get_HUE(eC341_CH_V ch_v)  //A70LV_Doulas_0003
{
    return m_sChannelSetting[ch_v].iHue;
}

void dvC341_Set_Saturation(eC341_CH_V ch_v)  //A70LV_Doulas_0022
{
    if(m_sChannelSetting[eC341_CH_V0].cPIP_PBP_ON == FALSE)        //A70LV_Doulas_0155
    {
        #if 0  //simon check
        if((m_sChannelInfo[eC341_CH_V0].ePanelTimingId == ePANEL_ID_1080P_120HZ) ||
           (m_sChannelInfo[eC341_CH_V0].ePanelTimingId == ePANEL_ID_WUXGA_120HZ))   //A70LV_Doulas_0377 Modify
        {

        }
        else
        #endif
        if(ch_v == eC341_CH_V1)   //simon check ???
        {
            return ;
        }
    }
    dvC341_Set_Color(ch_v);//A70LV_Doulas_0012 modify
}

DOUBLE dvC341_Get_Saturation(eC341_CH_V ch_v)  //A70LV_Doulas_0003
{
    return m_sChannelSetting[ch_v].dColor;
}

void dvC341_Set_PIP_PBP_Layout(UINT8 uiLayout)  //A70LV_Doulas_0003
{
    m_sChannelSetting[eC341_CH_V0].cMain_Layout = uiLayout;     //A70LV_Doulas_0008 modify
}

void dvC341_Set_PIP_PBP_Size(UINT8 uiSize)  //A70LV_Doulas_0003
{
    LOG_MSG(db_DV_SCALER, "##Set_PIP_PBP_Size%d##\n",uiSize);
    m_sChannelSetting[eC341_CH_V0].cPIP_Size = uiSize;     //A70LV_Doulas_0008 modify
}

UINT8 dvC341_Get_PIP_PBP_Layout(void)  //A70LV_Doulas_0003
{
    return m_sChannelSetting[eC341_CH_V0].cMain_Layout;
}

UINT8 dvC341_Get_PIP_PBP_Size(void)  //A70LV_Doulas_0003
{
    return m_sChannelSetting[eC341_CH_V0].cPIP_Size;
}

void dvC341_Resync_Init(eC341_CH_V ch_v)   //A70LV_Doulas_0004
{
    ucNo_Signal_Config[ch_v] = 0;
    m_sChannelInfo[ch_v].sInputTimingInfo.eScanMode = eSCAN_MODE_NO_SIGNAL;  //A70LV_Doulas_0005
}

void dvC341_Freeze_Set(eC341_CH_V ch_v, BOOL bFreezeEn)     //A70LV_Doulas_0004
{
    if(ch_v >= eC341_CH_VNUMBER)
    {
        return;
    }

    UINT8 ucRegOffset = ch_v * (CH_BANK_OFFSET*2);

    UINT32 rdata;
	UINT32 ifld;
	UINT32 ifld2;
	UINT32 sel_tmp;
	UINT32 reg_sel;

    m_sChannelSetting[ch_v].cFREEZE = (UINT8)bFreezeEn;      //A70LV_Doulas_0030

    if(m_sChannelSetting[ch_v].cFREEZE == TRUE)  //freeze on
    {
        dvC341_set_rtct_thru();
        //LOG_MSG(db_ALWAYS,"##1(%d)(%d%d%d%d%d)\r\n",ch_v,g_freeze_isel[ch_v][0],g_freeze_isel[ch_v][1],g_freeze_isel[ch_v][2],g_freeze_isel[ch_v][3],g_freeze_isel[ch_v][4]);

        m_sChannelInfo[ch_v].ucISYCT &= 0xE0;
        m_sChannelInfo[ch_v].ucISYCT |= 0x02;  //Keep field with current value
        dvC341_Write(B16_ISYCTCH1, m_sChannelInfo[ch_v].ucISYCT, ucRegOffset);

		dvC341_Write(B0_MCT1 + 0x200*ch_v, 0x01, 0);
		dvC341_Write(B0_MCT2 + 0x200*ch_v, 0x01, 0);
        dvC341_wait1_povs(0);

		rdata = dvC341_Read(B171_STATECH1 + 0x200*ch_v, 0);
		ifld = (rdata >> 2) & 0x03;  //ifld:   IFLD[1:0] CH1: Outputs the internal write field (the field signal of memory address).

		ifld2 = (ifld + 3) & 0x03;
		sel_tmp = g_freeze_isel[ch_v][ifld2];
		g_freeze_isel[ch_v][ifld2] = g_freeze_isel[ch_v][4];
		g_freeze_isel[ch_v][4] = sel_tmp;
        //LOG_MSG(db_ALWAYS,"##2(%d)\r\n",sel_tmp);

		reg_sel = ( g_freeze_isel[ch_v][0]        & 0x000007)
				+ ((g_freeze_isel[ch_v][1] <<  4) & 0x000070)
				+ ((g_freeze_isel[ch_v][2] <<  8) & 0x000700)
				+ ((g_freeze_isel[ch_v][3] << 12) & 0x007000)
				+ ((g_freeze_isel[ch_v][4] << 16) & 0x070000);

		dvC341_Write(B22_ISLFRZFSELCH1, reg_sel, ucRegOffset);
		dvC341_Write(B38_ISLFRZFSELCH2, reg_sel, ucRegOffset);

		reg_sel = ( g_freeze_isel[ch_v][4]        & 0x000007)
				+ ((g_freeze_isel[ch_v][4] <<  4) & 0x000070)
				+ ((g_freeze_isel[ch_v][4] <<  8) & 0x000700)
				+ ((g_freeze_isel[ch_v][4] << 12) & 0x007000);

		dvC341_Write(B22_OSLFRZFSELCH1, reg_sel, ucRegOffset);
		dvC341_Write(B38_OSLFRZFSELCH2, reg_sel, ucRegOffset);

        if(m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.eScanMode == eSCAN_MODE_INTERLACE)
        {
            m_sChannelInfo[ch_v].ucISYCT &= 0xE0;
        }
        else
        {
            m_sChannelInfo[ch_v].ucISYCT &= 0xE0;
            m_sChannelInfo[ch_v].ucISYCT |= 0x01;
        }
		dvC341_Write(B16_ISYCTCH1, m_sChannelInfo[ch_v].ucISYCT, ucRegOffset);

		dvC341_set_rtct_normal();

    }
    else  //freeze off
    {
		reg_sel = ( g_freeze_isel[ch_v][3]        & 0x000007)
				+ ((g_freeze_isel[ch_v][0] <<  4) & 0x000070)
				+ ((g_freeze_isel[ch_v][1] <<  8) & 0x000700)
				+ ((g_freeze_isel[ch_v][2] << 12) & 0x007000);

		dvC341_Write(B22_OSLFRZFSELCH1, reg_sel, ucRegOffset);
		dvC341_Write(B38_OSLFRZFSELCH2, reg_sel, ucRegOffset);
    }

    LOG_MSG(db_DV_SCALER,"ucNo_Signal_Config[%d] = %d\r\n",ch_v,ucNo_Signal_Config[ch_v]); //H30K_Doulas_0036
    if(ucNo_Signal_Config[ch_v] == FALSE)  //A35G2_CDS_Simon_0017
    {
        if(m_sChannelInfo[eC341_CH_V0].ePanelTimingId == PANEL_3D_OUTPUT) //H30K_Doulas_0036
        {
            dvC341_set_mct(ch_v,0);
        }
        else
        {
            dvC341_SetInputPort_2K(ch_v);
        }
    }

    if(m_sChannelSetting[ch_v].cFREEZE == TRUE)  //A70LV_Doulas_0030
    {   //not auto fill back ground
	    dvC341_Channel1_Auto_Fill_Screen_Set(FALSE);
        dvC341_Channel2_Auto_Fill_Screen_Set(FALSE);
        dvC341_Warp_Auto_Fill_Screen_Set(FALSE);
    }
    else if(!bCapImgOutput)
    {
        dvC341_Auto_Fill_Screen_Init();
    }
}

BOOL dvC341_Freeze_Get(eC341_CH_V ch_v)        //A70LV_Doulas_0004
{
    return m_sChannelSetting[ch_v].cFREEZE;      //A70LV_Doulas_0030
}

#if 0  //unused
BOOL dvC341_Panel_Change_LVDS_Stop(void) //A70LV_Doulas_0005
{
    UINT32 g_RTCT;
    g_RTCT = RTCT_STOP;

    dvC341_Write(B0_RTCT,g_RTCT,0);

	dvC341_LVDS_Stop();
    MS_SLEEP(10);
 #endif

BOOL dvC341_Panel_Change_Setting(eC341_CH_V ch_v,const ePANEL_ID ePanelId) //A70LV_Doulas_0005
{
    UINT32 g_RTCT;
    UINT8 ucTblIndex = 0;
    eSCALER_EXEC_CODE eExecCode = eSCALER_EXEC_CODE_PASS;


    if((eExecCode = dvC341_GetPanelIndex(ePanelId, &ucTblIndex)) != eSCALER_EXEC_CODE_PASS)
    {
        LOG_MSG(db_DV_SCALER, "(func:%s, line:%d) C341 Get Panel Timing Index Fail = %d\n", __FUNCTION__, __LINE__, eExecCode);
        return eExecCode;
    }
    dvC341_GetCHOutputTiming_2K(eC341_CH_V0, ucTblIndex);
    dvC341_GetCHOutputTiming_2K(eC341_CH_V1, ucTblIndex);

    m_sChannelInfo[eC341_CH_V0].ePanelTimingId = ePanelId;
    m_sChannelInfo[eC341_CH_V0].sInputTimingInfo.eScanMode = eSCAN_MODE_NO_SIGNAL;
    m_sChannelInfo[eC341_CH_V0].sInterlace_InputTimingInfo.eScanMode = eSCAN_MODE_NO_SIGNAL;
    m_sChannelInfo[eC341_CH_V0].ucPanelIndex = ucTblIndex;
    m_sChannelInfo[eC341_CH_V1].ePanelTimingId = ePanelId;
    m_sChannelInfo[eC341_CH_V1].sInputTimingInfo.eScanMode = eSCAN_MODE_NO_SIGNAL;
    m_sChannelInfo[eC341_CH_V1].sInterlace_InputTimingInfo.eScanMode = eSCAN_MODE_NO_SIGNAL;
    m_sChannelInfo[eC341_CH_V1].ucPanelIndex = ucTblIndex;

    dvC341_InputPort_Set();

    dvC341_ConfigClock(m_sPanelTable[ucTblIndex].sOutputTimingInfo.ucPoRefDiv,
                           m_sPanelTable[ucTblIndex].sOutputTimingInfo.ucPoFbDiv,
                           m_sChannelInfo[ch_v].ucIPPLLCT);
    dvC341_RegInit(); //H30K_Doulas_0001
    dvC341_ConfigBasicMemory_2K();
   // dvC341_SetMemAD_2K(ch_v,&sOact);
   // dvC341_ConfigIPMemory_2K(ch_v);
    //dvC341_ConfigLVDS_2K(ePanelId);
    if((m_sChannelInfo[eC341_CH_V0].ePanelTimingId == ePANEL_ID_1080P_120HZ) ||
       (m_sChannelInfo[eC341_CH_V0].ePanelTimingId == ePANEL_ID_WUXGA_120HZ)) //H30K_Doulas_0001
        dvC341_VBO_Init_3D_Panel();
    else
        dvC341_VBO_Init();  //H30K_Doulas_0001

    dvC341_SetInputMemoryBuffer_4K(eC341_CH_V0);   //A70LV_Doulas_0154
    dvC341_SetInputMemoryBuffer_4K(eC341_CH_V1);   //A70LV_Doulas_0154
    dvC341_Set_flddly(ch_v);  //H2PF_Simon_0196
    dvC341_ConfigOutput_4K(eC341_CH_V0);
    dvC341_ConfigOutput_4K(eC341_CH_V1);
    dvC341_Set_Flip(eC341_CH_V1);          //A70LV_Doulas_0154
    dvC341_Set_Flip(eC341_CH_V0);          //A70LV_Doulas_0154
    dvC341_SetInputPort_2K(eC341_CH_V1);   //A70LV_Doulas_0154


    //dvC341_WAIP_init();
    dvC341_Write(B48_IACTHSTCH3, 	0, 0); //H30K_Doulas_0032 init sub source
	dvC341_Write(B48_IACTHWCH3, 	0, 0);
	dvC341_Write(B48_IACTVSTCH3, 	0, 0);
	dvC341_Write(B48_IACTVWCH3, 	0, 0);
	dvC341_Write(B64_IACTHSTCH4, 	0, 0);
	dvC341_Write(B64_IACTHWCH4, 	0, 0);
	dvC341_Write(B64_IACTVSTCH4, 	0, 0);
	dvC341_Write(B64_IACTVWCH4, 	0, 0);


    //g_RTCT = RTCT_NORMAL;
    //dvC341_Write(B0_RTCT, g_RTCT , 0);
    dvC341_set_rtct_normal();

    return eExecCode;
}

UINT32 dvC341_GetAPL(const eC341_CH_V ch_v,UINT8 *ucMax,UINT8 *ucMin)   //A70LV_Doulas_0215 modify//A70LV_Doulas_0007
{
    UINT32 ucAPL = 0;
    if(ch_v >= eC341_CH_VNUMBER)
    {
        return FALSE;
    }

#if 0   //simon check ...  not implement
    if(ch_v == eC341_CH_V0)
    {
        ucAPL = dvC341_Read(B171_IAPL0CH1, 0);
        ucAPL = dvC341_Read(B171_IAPL0CH2, 0);
        *ucMax = dvC341_Read(B169_IMAXFILTRCH1, 0);
        *ucMin = dvC341_Read(B169_IMINFILTRCH1, 0);
        *ucMax = dvC341_Read(B169_IMAXFILTRCH2, 0);
        *ucMin = dvC341_Read(B169_IMINFILTRCH2, 0);
        LOG_MSG(db_DV_SCALER, "(func:%s, line:%d) Phase(%02d)=(%x)(%d,%d)\r\n", __FUNCTION__, __LINE__,m_sChannelInfo[eCH].sAutoPhase.uiCurrentPhase,ucAPL,*ucMax,*ucMin);  //A70LV_Doulas_0116
    }
    else
    {
        ucAPL = dvC341_Read(B171_IAPL0CH3, 0);
        ucAPL = dvC341_Read(B171_IAPL0CH4, 0);
        *ucMax = dvC341_Read(B169_IMAXFILTRCH3, 0);
        *ucMin = dvC341_Read(B169_IMINFILTRCH3, 0);
        *ucMax = dvC341_Read(B169_IMAXFILTRCH4, 0);
        *ucMin = dvC341_Read(B169_IMINFILTRCH4, 0);
        LOG_MSG(db_DV_SCALER, "(func:%s, line:%d) Phase(%02d)=(%x)(%d,%d)\r\n", __FUNCTION__, __LINE__,m_sChannelInfo[eCH].sAutoPhase.uiCurrentPhase,ucAPL,*ucMax,*ucMin);  //A70LV_Doulas_0116
    }
#endif

    return ucAPL;
}

void dvC341_InputSourceSet(const eC341_CH_V ch_v,UINT8 ucInputSource)   //A70LV_Doulas_0007
{
    m_sChannelSetting[ch_v].eInputSource = (eCM_SOURCE_ID)ucInputSource;    //A70LV_Doulas_0013 modify
}

void dvC341_SetContrast(const eC341_CH_V ch_v)
{
    UINT8 ucRegOffset = 0;
    UINT32 ulVal;

    if(ch_v >= eC341_CH_VNUMBER)
    {
        return;
    }

    if(m_sChannelSetting[ch_v].cPIP_PBP_ON == FALSE)
    {
        if(ch_v == eC341_CH_V1)
        {
            return ;
        }
    }

	ulVal = ((int)( 512*(m_sChannelSetting[ch_v].dContrast)));
	if (ulVal > 0x07FF)
    {       // Overflow limit
		ulVal = 0x07FF;
	}
	ulVal = ulVal & 0x07FF;


    //dvC341_Write(B0_RTCT, RTCT_POVSSTOP, 0);   //0x4321FFFF
    dvC341_set_rtct_stop();

    for(UINT8 ch=0; ch<OUTPUT4K_CH_NUM; ch++)
    {
        ucRegOffset = ((ch_v * 2) + ch) * CH_BANK_OFFSET;
        dvC341_Write(B12_BGCTCH1,		0x01,ucRegOffset) ;
    	dvC341_Write(B12_OGAIN1RCH1,ulVal  ,ucRegOffset);	//Output gain correction
    	dvC341_Write(B12_OGAIN1GCH1,ulVal  ,ucRegOffset);
    	dvC341_Write(B12_OGAIN1BCH1,ulVal  ,ucRegOffset);
    }

    //dvC341_Write(B0_RTCT, RTCT_NORMAL, 0);   //0x4321CBA9
    dvC341_set_rtct_normal();
}

void dvC341_Scale_Overscan(eOVER_SCAN_TYPE ucOverScan,PsINPUT_TIMING_INFO psScalerInput,PsOUTPUT_TIMING_INFO psScalerOutput) //A70LV_Doulas_0009
{
    UINT16 wOverscanPercentage, wPixels;

    LOG_MSG(db_DV_SCALER, "OverScan = %d\r\n", ucOverScan);

    switch(ucOverScan)
    {
        case eOVER_SCAN_CROP: //overscan - crop 3%
            //caculate input
            wOverscanPercentage = 3;
            wPixels = (UINT16)(((wOverscanPercentage) * psScalerInput->uiHActive) / 100);
            psScalerInput->uiHStart += wPixels;
            psScalerInput->uiHActive -=  (2 * wPixels);

            wPixels = (UINT16)(((wOverscanPercentage) * psScalerInput->uiVActive) / 100);
            psScalerInput->uiVStart += wPixels;
            psScalerInput->uiVActive -=  (2 * wPixels);

            //caculate output
            wOverscanPercentage = 3;
            wPixels = (UINT16)(((wOverscanPercentage) * psScalerOutput->uiHActive) / 100);
            psScalerOutput->uiHStart += wPixels;
            psScalerOutput->uiHActive -=  (2 * wPixels);

            wPixels = (UINT16)(((wOverscanPercentage) * psScalerOutput->uiVActive) / 100);
            psScalerOutput->uiVStart += wPixels;
            psScalerOutput->uiVActive -=  (2 * wPixels);
            break;

        case eOVER_SCAN_ZOOM://overscan - zoom
            //caculate input
            wOverscanPercentage = 3;
            wPixels = (UINT16)(((wOverscanPercentage) * psScalerInput->uiHActive) / 100);
            psScalerInput->uiHStart += wPixels;
            psScalerInput->uiHActive -=  (2 * wPixels);

            wPixels = (UINT16)(((wOverscanPercentage) * psScalerInput->uiVActive) / 100);
            psScalerInput->uiVStart += wPixels;
            psScalerInput->uiVActive -=  (2 * wPixels);

            break;

        case eOVER_SCAN_OFF:
        default:

            break;
    }

}

eSCALER_EXEC_CODE dvC341_Overscan_Set(const eC341_CH_V ch_v,eOVER_SCAN_TYPE ucOverScan)    //A70LV_Doulas_0009
{
    eSCALER_EXEC_CODE eResult = eSCALER_EXEC_CODE_PASS;
    m_sChannelSetting[ch_v].eOverScan = ucOverScan;
    return eResult;
}

eSCALER_EXEC_CODE dvC341_Digital_Horz_Zoom_Set(const eC341_CH_V ch_v,UINT16 uiDigital_Horz_Zoom)    //A70LV_Doulas_0009
{
    eSCALER_EXEC_CODE eResult = eSCALER_EXEC_CODE_PASS;
    m_sChannelSetting[ch_v].uiDigitalHorzZoom = uiDigital_Horz_Zoom;

    return eResult;
}

eSCALER_EXEC_CODE dvC341_Digital_Vert_Zoom_Set(const eC341_CH_V ch_v,UINT16 uiDigital_Vert_Zoom)    //A70LV_Doulas_0009
{
    eSCALER_EXEC_CODE eResult = eSCALER_EXEC_CODE_PASS;
    m_sChannelSetting[ch_v].uiDigitalVertZoom = uiDigital_Vert_Zoom;

    return eResult;
}

eSCALER_EXEC_CODE dvC341_Digital_Horz_Shift_Set(const eC341_CH_V ch_v,UINT16 uiDigital_Horz_Shift)    //A70LV_Doulas_0009
{
    eSCALER_EXEC_CODE eResult = eSCALER_EXEC_CODE_PASS;
    m_sChannelSetting[ch_v].uiDigitalHorzShift = uiDigital_Horz_Shift;

    return eResult;
}

eSCALER_EXEC_CODE dvC341_Digital_Vert_Shift_Set(const eC341_CH_V ch_v,UINT16 uiDigital_Vert_Shift)    //A70LV_Doulas_0009
{
    eSCALER_EXEC_CODE eResult = eSCALER_EXEC_CODE_PASS;
    m_sChannelSetting[ch_v].uiDigitalVertShift = uiDigital_Vert_Shift;

    return eResult;
}

eSCALER_EXEC_CODE dvC341_Digital_Horz_Zoom_Get(const eC341_CH_V ch_v,UINT16 *uiDigital_Horz_Zoom)    //A70LV_Doulas_0009
{
    eSCALER_EXEC_CODE eResult = eSCALER_EXEC_CODE_PASS;
    *uiDigital_Horz_Zoom = m_sChannelSetting[ch_v].uiDigitalHorzZoom;

    return eResult;
}

eSCALER_EXEC_CODE dvC341_Digital_Vert_Zoom_Get(const eC341_CH_V ch_v,UINT16 *uiDigital_Vert_Zoom)    //A70LV_Doulas_0009
{
    eSCALER_EXEC_CODE eResult = eSCALER_EXEC_CODE_PASS;
    *uiDigital_Vert_Zoom = m_sChannelSetting[ch_v].uiDigitalVertZoom;

    return eResult;
}

eSCALER_EXEC_CODE dvC341_Digital_Horz_Shift_Get(const eC341_CH_V ch_v,UINT16 *uiDigital_Horz_Shift)    //A70LV_Doulas_0009
{
    eSCALER_EXEC_CODE eResult = eSCALER_EXEC_CODE_PASS;
    *uiDigital_Horz_Shift = m_sChannelSetting[ch_v].uiDigitalHorzShift;

    return eResult;
}

eSCALER_EXEC_CODE dvC341_Digital_Vert_Shift_Get(const eC341_CH_V ch_v,UINT16 *uiDigital_Vert_Shift)    //A70LV_Doulas_0009
{
    eSCALER_EXEC_CODE eResult = eSCALER_EXEC_CODE_PASS;
    *uiDigital_Vert_Shift = m_sChannelSetting[ch_v].uiDigitalVertShift;

    return eResult;
}

void dvC341_Scale_Digital_Zoom(PsINPUT_TIMING_INFO psScalerInput,PsOUTPUT_TIMING_INFO psScalerOutput)   //A70LV_Doulas_0009
{
    UINT32 udwMagnifyPlusShiftX = 0;
    UINT32 udwMagnifyPlusShiftY = 0;
    UINT16 udwPixels;

    //caculate scaler input
    if((m_sChannelSetting[eC341_CH_V0].uiDigitalHorzZoom > DIGITAL_HORZ_ZOOM_DEFAULT) ||
       (m_sChannelSetting[eC341_CH_V0].uiDigitalVertZoom > DIGITAL_VERT_ZOOM_DEFAULT))
    {
        if(m_sChannelSetting[eC341_CH_V0].uiDigitalHorzZoom > DIGITAL_HORZ_ZOOM_DEFAULT)  //Horz
        {
            udwPixels = (UINT16)(((UINT32)psScalerInput->uiHActive * 100) / (UINT32)m_sChannelSetting[eC341_CH_V0].uiDigitalHorzZoom) & 0xFFFE;

            udwMagnifyPlusShiftX = (UINT32)(psScalerInput->uiHActive - udwPixels);
            udwMagnifyPlusShiftX = (udwMagnifyPlusShiftX * (UINT32)(m_sChannelSetting[eC341_CH_V0].uiDigitalHorzShift - DIGITAL_HORZ_SHIFT_MIN))/(DIGITAL_HORZ_SHIFT_MAX - DIGITAL_HORZ_SHIFT_MIN);

            psScalerInput->uiHStart += (UINT16)udwMagnifyPlusShiftX;
            psScalerInput->uiHActive = udwPixels;
        }

        if(m_sChannelSetting[eC341_CH_V0].uiDigitalVertZoom > DIGITAL_VERT_ZOOM_DEFAULT)  //Vert
        {
            udwPixels = (UINT16)(((UINT32)psScalerInput->uiVActive * 100) / (UINT32)m_sChannelSetting[eC341_CH_V0].uiDigitalVertZoom) & 0xFFFE;

            udwMagnifyPlusShiftY = (UINT32)(psScalerInput->uiVActive - udwPixels);
            udwMagnifyPlusShiftY = (udwMagnifyPlusShiftY * (UINT32)(DIGITAL_VERT_SHIFT_MAX - m_sChannelSetting[eC341_CH_V0].uiDigitalVertShift))/(DIGITAL_VERT_SHIFT_MAX - DIGITAL_VERT_SHIFT_MIN);  //A70LV_Doulas_0010 modify

            psScalerInput->uiVStart += (UINT16)udwMagnifyPlusShiftY;
            psScalerInput->uiVActive = udwPixels;
        }
    }

    //caculate scaler output
    if((m_sChannelSetting[eC341_CH_V0].uiDigitalHorzZoom < DIGITAL_HORZ_ZOOM_DEFAULT) ||
       (m_sChannelSetting[eC341_CH_V0].uiDigitalVertZoom < DIGITAL_VERT_ZOOM_DEFAULT))
    {
        if(m_sChannelSetting[eC341_CH_V0].uiDigitalHorzZoom < DIGITAL_HORZ_ZOOM_DEFAULT) //Horz
        {
            udwPixels = (UINT16)(((UINT32)psScalerOutput->uiHActive * (UINT32)m_sChannelSetting[eC341_CH_V0].uiDigitalHorzZoom)/100);

            udwMagnifyPlusShiftX = (UINT32)(psScalerOutput->uiHActive - udwPixels);
            udwMagnifyPlusShiftX = (udwMagnifyPlusShiftX * (UINT32)(m_sChannelSetting[eC341_CH_V0].uiDigitalHorzShift - DIGITAL_HORZ_SHIFT_MIN))/(DIGITAL_HORZ_SHIFT_MAX - DIGITAL_HORZ_SHIFT_MIN);

            psScalerOutput->uiHStart += (UINT16)udwMagnifyPlusShiftX;
            psScalerOutput->uiHActive = udwPixels;
        }

        if(m_sChannelSetting[eC341_CH_V0].uiDigitalVertZoom < DIGITAL_VERT_ZOOM_DEFAULT) //Vert
        {
            udwPixels = (UINT16)(((UINT32)psScalerOutput->uiVActive * (UINT32)m_sChannelSetting[eC341_CH_V0].uiDigitalVertZoom)/100);

            udwMagnifyPlusShiftY = (UINT32)(psScalerOutput->uiVActive - udwPixels);
            udwMagnifyPlusShiftY = (udwMagnifyPlusShiftY * (UINT32)(DIGITAL_VERT_SHIFT_MAX - m_sChannelSetting[eC341_CH_V0].uiDigitalVertShift))/(DIGITAL_VERT_SHIFT_MAX - DIGITAL_VERT_SHIFT_MIN);  //A70LV_Doulas_0010 modify

            psScalerOutput->uiVStart += (UINT16)udwMagnifyPlusShiftY;
            psScalerOutput->uiVActive = udwPixels;
        }
    }
}

void dvC341_Scale_Digital_Zoom_Custom(PsINPUT_TIMING_INFO psScalerInput,PsOUTPUT_TIMING_INFO psScalerOutput)   //A70LV_Doulas_0010
{
    UINT32 udwMagnifyPlusShiftX = 0;
    UINT32 udwMagnifyPlusShiftY = 0;
    UINT16 udwOutputWidth;
    UINT16 udwOutputHeight;

    udwOutputWidth = (UINT16)(((UINT32)psScalerOutput->uiHActive * (UINT32)m_sChannelSetting[eC341_CH_V0].uiDigitalHorzZoom)/DIGITAL_HORZ_ZOOM_DEFAULT);
    udwOutputHeight = (UINT16)(((UINT32)psScalerOutput->uiVActive * (UINT32)m_sChannelSetting[eC341_CH_V0].uiDigitalVertZoom)/DIGITAL_VERT_ZOOM_DEFAULT);

    //caculate scaler output
    if(udwOutputWidth > m_sChannelInfo[eC341_CH_V0].sOutputTimingInfo.uiHActive)
    {
        psScalerOutput->uiHStart  = m_sChannelInfo[eC341_CH_V0].sOutputTimingInfo.uiHStart;
        psScalerOutput->uiHActive = m_sChannelInfo[eC341_CH_V0].sOutputTimingInfo.uiHActive;
    }
    else
    {
        udwMagnifyPlusShiftX = (UINT32)(m_sChannelInfo[eC341_CH_V0].sOutputTimingInfo.uiHActive - udwOutputWidth);

        if(m_sChannelSetting[eC341_CH_V0].uiDigitalHorzZoom != DIGITAL_HORZ_ZOOM_DEFAULT) //A35G2_CDS_Larry_0014
        {
            udwMagnifyPlusShiftX = (udwMagnifyPlusShiftX * (UINT32)(m_sChannelSetting[eC341_CH_V0].uiDigitalHorzShift - DIGITAL_HORZ_SHIFT_MIN))/(DIGITAL_HORZ_SHIFT_MAX - DIGITAL_HORZ_SHIFT_MIN);
        }
        else
        {
            udwMagnifyPlusShiftX = (udwMagnifyPlusShiftX * (UINT32)(DIGITAL_HORZ_SHIFT_DEFAULT - DIGITAL_HORZ_SHIFT_MIN))/(DIGITAL_HORZ_SHIFT_MAX - DIGITAL_HORZ_SHIFT_MIN);
        }

        psScalerOutput->uiHStart = (UINT16)udwMagnifyPlusShiftX + m_sChannelInfo[eC341_CH_V0].sOutputTimingInfo.uiHStart;
        psScalerOutput->uiHActive = udwOutputWidth;
    }

    if(udwOutputHeight > m_sChannelInfo[eC341_CH_V0].sOutputTimingInfo.uiVActive)
    {
        psScalerOutput->uiVStart  = m_sChannelInfo[eC341_CH_V0].sOutputTimingInfo.uiVStart;
        psScalerOutput->uiVActive = m_sChannelInfo[eC341_CH_V0].sOutputTimingInfo.uiVActive;
    }
    else
    {
        udwMagnifyPlusShiftY = (UINT32)(m_sChannelInfo[eC341_CH_V0].sOutputTimingInfo.uiVActive - udwOutputHeight);

        if(m_sChannelSetting[eC341_CH_V0].uiDigitalVertZoom != DIGITAL_VERT_ZOOM_DEFAULT) //A35G2_CDS_Larry_0014
        {
            udwMagnifyPlusShiftY = (udwMagnifyPlusShiftY * (UINT32)(DIGITAL_VERT_SHIFT_MAX - m_sChannelSetting[eC341_CH_V0].uiDigitalVertShift))/(DIGITAL_VERT_SHIFT_MAX - DIGITAL_VERT_SHIFT_MIN);
        }
        else
        {
            udwMagnifyPlusShiftY = (udwMagnifyPlusShiftY * (UINT32)(DIGITAL_VERT_SHIFT_MAX - DIGITAL_VERT_SHIFT_DEFAULT))/(DIGITAL_VERT_SHIFT_MAX - DIGITAL_VERT_SHIFT_MIN);
        }

        psScalerOutput->uiVStart = (UINT16)udwMagnifyPlusShiftY + m_sChannelInfo[eC341_CH_V0].sOutputTimingInfo.uiVStart;
        psScalerOutput->uiVActive = udwOutputHeight;
    }
}

void dvC341_Scale_Setting_Check(const eC341_CH_V ch_v,PsINPUT_TIMING_INFO psScalerInput,PsOUTPUT_TIMING_INFO psScalerOutput) //H30K_Doulas_0041//A70LV_Doulas_0010
{
    INT16 Input_HST = 0;
    INT16 Input_VST = 0;

    if(m_sChannelInfo[ch_v].sInputTimingInfo.lSTATE < eINPUT_PORT_INFO_STATE_1CH) //H30K_Doulas_0046// 4k input
        Input_HST = (INT16)(m_sChannelInfo[ch_v].sInputTimingInfo.uiHStart - 7) + (INT16)(HORZ_POSITION_DEFAULT - m_sChannelSetting[ch_v].ucHoriPosition);
    else
        Input_HST = (INT16)psScalerInput->uiHStart + (INT16)(m_sChannelInfo[ch_v].sInputTimingInfo.uiHStart - 7) + (INT16)(HORZ_POSITION_DEFAULT - m_sChannelSetting[ch_v].ucHoriPosition);
	Input_VST = (INT16)psScalerInput->uiVStart + (INT16)(m_sChannelInfo[ch_v].sInputTimingInfo.uiVStart + 1) + (INT16)(m_sChannelSetting[ch_v].ucVertPosition - VERT_POSITION_DEFAULT);


    if( Input_HST < HORZ_POSITION_LIMIT_MIN) //image error
    {
        INT16 uiVal = HORZ_POSITION_LIMIT_MIN - Input_HST;
        WorkingHorzPosition[ch_v] = m_sChannelSetting[ch_v].ucHoriPosition - (UINT8)uiVal;
    }
    else
    {
        WorkingHorzPosition[ch_v] = m_sChannelSetting[ch_v].ucHoriPosition;
    }

    if(m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.eScanMode == eSCAN_MODE_INTERLACE)    //A70LV_Doulas_0113 modify position work//A70LV_Doulas_0066 modify image error
    {
        //Check V-start Min
        if( Input_VST < VERT_POSITION_LIMIT_MIN_INT)
        {
            INT16 uiVal = VERT_POSITION_LIMIT_MIN_INT - Input_VST;
            WorkingVertPosition[ch_v] = m_sChannelSetting[ch_v].ucVertPosition + (UINT8)uiVal;
        //    psScalerInput->uiVStart = VERT_POSITION_LIMIT_MIN_INT;
        }
        else
        {
            WorkingVertPosition[ch_v] = m_sChannelSetting[ch_v].ucVertPosition;
        }

        //Check V-start Max       //A70LV_Doulas_0144 avoid jitter image
        if((m_sChannelInfo[ch_v].sInputTimingInfo.uiVTotal > 310) &&
           (m_sChannelInfo[ch_v].sInputTimingInfo.uiVTotal <= 313))   //total 625 (576i)
        {
            if((Input_VST + psScalerInput->uiVActive)/2 > 312)
            {
                INT16 iVal = (INT16)(Input_VST + psScalerInput->uiVActive) - (INT16)(312 * 2) - 1;
                WorkingVertPosition[ch_v] = m_sChannelSetting[ch_v].ucVertPosition - (UINT8)iVal;
            //    psScalerInput->uiVStart = psScalerInput->uiVStart - (UINT16)iVal;
            }
        }
        else if((m_sChannelInfo[ch_v].sInputTimingInfo.uiVTotal > 253) &&
           (m_sChannelInfo[ch_v].sInputTimingInfo.uiVTotal <= 263))   //total 525 (480i)
        {
            if((Input_VST + psScalerInput->uiVActive)/2 > 262)
            {
                INT16 iVal = (INT16)(Input_VST + psScalerInput->uiVActive) - (INT16)(262 * 2) - 1;
                WorkingVertPosition[ch_v] = m_sChannelSetting[ch_v].ucVertPosition - (UINT8)iVal;
            //    psScalerInput->uiVStart = psScalerInput->uiVStart - (UINT16)iVal;
            }
        }
        else if((m_sChannelInfo[ch_v].sInputTimingInfo.uiVTotal > 550) &&
           (m_sChannelInfo[ch_v].sInputTimingInfo.uiVTotal <= 563))   //total 1125 (1080i)
        {
            if((Input_VST + psScalerInput->uiVActive/2) > 562)
            {
                INT16 iVal = (INT16)(Input_VST + psScalerInput->uiVActive/2) - (INT16)(562 ) ;
                WorkingVertPosition[ch_v] = m_sChannelSetting[ch_v].ucVertPosition - (UINT8)iVal;
            //    psScalerInput->uiVStart = psScalerInput->uiVStart - (UINT16)iVal;
            }
        }
    }
    else
    {
        if( Input_VST < VERT_POSITION_LIMIT_MIN_PRO)
        {
            INT16 uiVal = VERT_POSITION_LIMIT_MIN_PRO - Input_VST;
            WorkingVertPosition[ch_v] = m_sChannelSetting[ch_v].ucVertPosition + (UINT8)uiVal;
        }
        else
        {
            WorkingVertPosition[ch_v] = m_sChannelSetting[ch_v].ucVertPosition;
        }
    }
}

void dvC341_Start_Position_Set(const eC341_CH_V ch_v,PsINPUT_TIMING_INFO psScalerInput,PsOUTPUT_TIMING_INFO psScalerOutput)   //A70LV_Doulas_0010
{
    psScalerInput->uiHStart += HORZ_POSITION_DEFAULT - m_sChannelSetting[ch_v].ucHoriPosition;
    psScalerInput->uiVStart += m_sChannelSetting[ch_v].ucVertPosition - VERT_POSITION_DEFAULT;
}

eSCALER_EXEC_CODE dvC341_Horz_Start_Position_Set(const eC341_CH_V ch_v,UINT8 ucPosition)  //A70LV_Doulas_0010
{
    eSCALER_EXEC_CODE eResult = eSCALER_EXEC_CODE_PASS;
    m_sChannelSetting[ch_v].ucHoriPosition = ucPosition;

    return eResult;
}

eSCALER_EXEC_CODE dvC341_Vert_Start_Position_Set(const eC341_CH_V ch_v,UINT8 ucPosition)  //A70LV_Doulas_0010
{
    eSCALER_EXEC_CODE eResult = eSCALER_EXEC_CODE_PASS;
    m_sChannelSetting[ch_v].ucVertPosition = ucPosition;

    return eResult;
}

UINT8 dvC341_Horz_Start_Position_Get(const eC341_CH_V ch_v) //G100_Julie_0017
{
    return m_sChannelSetting[ch_v].ucHoriPosition;
}

UINT8 dvC341_Vert_Start_Position_Get(const eC341_CH_V ch_v) //G100_Julie_0017
{
    return m_sChannelSetting[ch_v].ucVertPosition;
}

void dvC341_Set_RGB_Gain(const eC341_CH_V ch_v)    //A70LV_Doulas_0011
{
    UINT8 ucRegOffset = 0;
    UINT32 ulRedGain;
    UINT32 ulGreenGain;
    UINT32 ulBlueGain;

    if(ch_v >= eC341_CH_VNUMBER)
    {
        return;
    }

    if(m_sChannelSetting[eC341_CH_V0].cPIP_PBP_ON == FALSE)
    {
        if(ch_v == eC341_CH_V1)
        {
            return ;
        }
    }

	ulRedGain = ((int)( 512*(m_sChannelSetting[ch_v].dRedGain)));
	if (ulRedGain > 0x07FF)
    {       // Overflow limit
		ulRedGain = 0x07FF;
	}
	ulRedGain = ulRedGain & 0x07FF;


	ulGreenGain = ((int)( 512*(m_sChannelSetting[ch_v].dGreenGain)));
	if (ulGreenGain > 0x07FF)
    {       // Overflow limit
		ulGreenGain = 0x07FF;
	}
	ulGreenGain = ulGreenGain & 0x07FF;


	ulBlueGain = ((int)( 512*(m_sChannelSetting[ch_v].dBlueGain)));
	if (ulBlueGain > 0x07FF)
    {       // Overflow limit
		ulBlueGain = 0x07FF;
	}
	ulBlueGain = ulBlueGain & 0x07FF;

    //dvC341_Write(B0_RTCT, RTCT_POVSSTOP, 0);   //0x4321FFFF
    dvC341_set_rtct_stop();

    for(UINT8 ch=0; ch<OUTPUT4K_CH_NUM; ch++)
    {
        ucRegOffset = ((ch_v * 2) + ch) * CH_BANK_OFFSET;
        dvC341_Write(B12_BGCT2CH1,		0x01,ucRegOffset) ;
    	dvC341_Write(B12_OGAIN2RCH1,ulRedGain  ,ucRegOffset);	//Output gain correction
    	dvC341_Write(B12_OGAIN2GCH1,ulGreenGain  ,ucRegOffset);
    	dvC341_Write(B12_OGAIN2BCH1,ulBlueGain  ,ucRegOffset);
    }

    //dvC341_Write(B0_RTCT, RTCT_NORMAL, 0);   //0x4321CBA9
    dvC341_set_rtct_normal();

}

void dvC341_Set_RGB_Offset(const eC341_CH_V ch_v)    //A70LV_Doulas_00011
{
    UINT8 ucRegOffset = 0;

    if(ch_v >= eC341_CH_VNUMBER)
    {
        return;
    }

    if(m_sChannelSetting[eC341_CH_V0].cPIP_PBP_ON == FALSE)
    {
        if(ch_v == eC341_CH_V1)
        {
            return ;
        }
    }

    dvC341_set_rtct_stop();

    for(UINT8 ch=0; ch<OUTPUT4K_CH_NUM; ch++)
    {
        ucRegOffset = ((ch_v * 2) + ch) * CH_BANK_OFFSET;
        dvC341_Write(B12_BGCT2CH1,		0x01,ucRegOffset) ;
     	dvC341_Write(B12_OBIAS2RCH1,(m_sChannelSetting[ch_v].iRedOffset)&0x0fff,ucRegOffset);	//Output bias correction
     	dvC341_Write(B12_OBIAS2GCH1,(m_sChannelSetting[ch_v].iGreenOffset)&0x0fff,ucRegOffset);
     	dvC341_Write(B12_OBIAS2BCH1,(m_sChannelSetting[ch_v].iBlueOffset)&0x0fff,ucRegOffset);
    }

    //dvC341_Write(B0_RTCT, RTCT_NORMAL, 0);   //0x4321CBA9
    dvC341_set_rtct_normal();
}

INT8 dvC341_RedOffset_Value_Set (UINT8 ch_v,INT16 iRedOffset) //A70LV_Doulas_0022    //A70LV_Doulas_0011
{
    LOG_MSG(db_DV_SCALER, "(func:%s, line:%d): RedOffset=%d\r\n", __FUNCTION__, __LINE__, iRedOffset);

    m_sChannelSetting[ch_v].iRedOffset = iRedOffset;

    return C341_PASS;
}

INT8 dvC341_GreenOffset_Value_Set (UINT8 ch_v,INT16 iGreenOffset) //A70LV_Doulas_0022    //A70LV_Doulas_0011
{
    LOG_MSG(db_DV_SCALER, "(func:%s, line:%d): GreenOffset=%d\r\n", __FUNCTION__, __LINE__, iGreenOffset);

    m_sChannelSetting[ch_v].iGreenOffset = iGreenOffset;

    return C341_PASS;
}

INT8 dvC341_BlueOffset_Value_Set (UINT8 ch_v,INT16 iBlueOffset)  //A70LV_Doulas_0022   //A70LV_Doulas_0011
{
    LOG_MSG(db_DV_SCALER, "(func:%s, line:%d): BlueOffset=%d\r\n", __FUNCTION__, __LINE__, iBlueOffset);

    m_sChannelSetting[ch_v].iBlueOffset = iBlueOffset;

    return C341_PASS;
}

INT8 dvC341_RedGain_Value_Set (UINT8 ch_v,DOUBLE dRedGain)    //A70LV_Doulas_0022 //A70LV_Doulas_0011
{
    LOG_MSG(db_DV_SCALER, "(func:%s, line:%d): RedGain=%f\r\n", __FUNCTION__, __LINE__, dRedGain);

    m_sChannelSetting[ch_v].dRedGain = dRedGain;

    return C341_PASS;
}

INT8 dvC341_GreenGain_Value_Set (UINT8 ch_v,DOUBLE dGreenGain) //A70LV_Doulas_0022    //A70LV_Doulas_0011
{
    LOG_MSG(db_DV_SCALER, "(func:%s, line:%d): GreenGain=%f\r\n", __FUNCTION__, __LINE__, dGreenGain);

    m_sChannelSetting[ch_v].dGreenGain = dGreenGain;

    return C341_PASS;
}

INT8 dvC341_BlueGain_Value_Set (UINT8 ch_v,DOUBLE dBlueGain) //A70LV_Doulas_0022      //A70LV_Doulas_0011
{
    LOG_MSG(db_DV_SCALER, "(func:%s, line:%d): BlueGain=%f\r\n", __FUNCTION__, __LINE__, dBlueGain);

    m_sChannelSetting[ch_v].dBlueGain = dBlueGain;

    return C341_PASS;
}

INT8 dvC341_Set_HUE_Value(eC341_CH_V ch_v,INT16 iValue)  //A70LV_Doulas_0022
{
    //HUE(-180 ~ 180)
    m_sChannelSetting[ch_v].iHue = iValue;
    return C341_PASS;
}

INT8 dvC341_Set_Saturation_Value(eC341_CH_V ch_v,DOUBLE dSaturation)  //A70LV_Doulas_0022
{
    //color(0-3.99)
    m_sChannelSetting[ch_v].dColor = dSaturation;
    return C341_PASS;
}

eSCALER_EXEC_CODE dvC341_Set_SizePresets_Value(eC341_CH_V ch_v,UINT8 ucSizePresets)  //A70LV_Doulas_0020
{
    eSCALER_EXEC_CODE eResult = eSCALER_EXEC_CODE_PASS;

    m_sChannelSetting[ch_v].eScalingMode = (eCM_SCALING_MODE_ID)ucSizePresets;
    return eResult;
}

void dvC341_Sharpness_Value_Set(const eC341_CH_V ch_v,UINT8 ucVal)     //A70LV_Doulas_0029
{
    m_sChannelSetting[ch_v].ucSharpness = ucVal;
}

void dvC341_SkinColor_Value_Set(const eC341_CH_V ch_v,UINT8 ucVal)
{
    m_sChannelSetting[ch_v].ucSkinColor = ucVal;
}

void dvC341_SkinColor_Set(const eC341_CH_V ch_v)   //A70LV_Doulas_0155 modify
{
    dvC341_Set_Color_Correction(eC341_CH_V0,0,0,m_sChannelSetting[eC341_CH_V0].ucSkinColor,2);
}

void dvC341_Temporal_NR_Value_Set(const eC341_CH_V ch_v,UINT8 ucVal)
{
    m_sChannelSetting[ch_v].cTemporal_NR = ucVal;
}

void dvC341_MPEG_NR_Value_Set(const eC341_CH_V ch_v,UINT8 ucVal)
{
    m_sChannelSetting[ch_v].cMPEG_NR = ucVal;
}

void dvC341_Detect_Film_Value_Set(const eC341_CH_V ch_v,UINT8 ucVal)
{
    m_sChannelSetting[ch_v].cDetectFilm = ucVal;
}

void dvC341_IintChannelSetting(const eC341_CH_V ch_v,sCHANNEL_SETTING m_sCH_Setting)     //A70LV_Doulas_0030
{
    memcpy(&m_sChannelSetting[ch_v], &m_sCH_Setting, sizeof(sCHANNEL_SETTING));
    m_sChannelInfo[ch_v].eInputSyncType = (eINPUT_SYNC_TYPE)m_sChannelSetting[ch_v].eVGA_SYNC_TYPE;     //A70LV_Doulas_0112

    //printf("dvC341_IintChannelSetting << %d %d >>\r\n" ,ch_v,  m_sChannelSetting[ch_v].eInputSource);

    dvC341_InputPort_Set();
}

void dvC341_TestPattern_Set(const eC341_CH_V ch_v,UINT8 ucVal)     //A70LV_Doulas_0035
{
    m_sChannelSetting[ch_v].cTestPattern = ucVal;
    switch(ucVal)
    {
        default:
        case eCM_TEST_PATTERN_OFF:
            dvC341_Set_Output_Fill_Color(FALSE,eFILL_COLOR_BLACK);
            dvC341_Set_Output_Test_Pattern(FALSE,0);
            break;

        case eCM_TEST_PATTERN_GRID:
            dvC341_Set_Output_Test_Pattern(TRUE,8);
            break;

        case eCM_TEST_PATTERN_WHITE:
            dvC341_Set_Output_Fill_Color(TRUE,eFILL_COLOR_WHITE);
            break;

        case eCM_TEST_PATTERN_BLACK:
            dvC341_Set_Output_Fill_Color(TRUE,eFILL_COLOR_BLACK);
            break;

        case eCM_TEST_PATTERN_CHECKERBOARD:
            dvC341_Set_Output_Test_Pattern(TRUE,5);
            break;

        case eCM_TEST_PATTERN_COLORBAR:
            dvC341_Set_Output_Test_Pattern(TRUE,1);
            break;

        case eCM_TEST_PATTERN_RED:
            dvC341_Set_Output_Fill_Color(TRUE,eFILL_COLOR_RED);
            break;

        case eCM_TEST_PATTERN_GREEN:
            dvC341_Set_Output_Fill_Color(TRUE,eFILL_COLOR_GREEN);
            break;

        case eCM_TEST_PATTERN_BLUE:
            dvC341_Set_Output_Fill_Color(TRUE,eFILL_COLOR_BLUE);
            break;

        case eCM_TEST_PATTERN_YELLOW:
            dvC341_Set_Output_Fill_Color(TRUE,eFILL_COLOR_YELLOW);
            break;

        case eCM_TEST_PATTERN_MAGENTA:
            dvC341_Set_Output_Fill_Color(TRUE,eFILL_COLOR_MAGENTA);
            break;

        case eCM_TEST_PATTERN_CYAN:
            dvC341_Set_Output_Fill_Color(TRUE,eFILL_COLOR_CYAN);
            break;

        case eCM_TEST_PATTERN_BORESIGHT:
            dvC341_Set_Output_Test_Pattern(TRUE,14);
            break;

        case eCM_TEST_PATTERN_FULL_SCREEN:
            dvC341_Set_Output_Test_Pattern(TRUE,3);     //A70LV_Doulas_0040 modify
            break;
    }
}

void dvC341_InputPixelClock_Get(const eC341_CH_V ch_v,UINT8 *ucValue)   //A70LV_Doulas_0056
{
    char aucString[VERSION_STRING_MAX_LENGTH];
    DOUBLE dbVal;

    dbVal = m_sChannelInfo[ch_v].sInputTimingInfo.dFrameRate *m_sChannelInfo[ch_v].sInputTimingInfo.uiHTotal
           *g_vboinfo[g_vbo_recno[m_sChannelInfo[ch_v].ucInputPort]].CH    //H30K_Doulas_0051
           *m_sChannelInfo[ch_v].sInputTimingInfo.uiVTotal / (DOUBLE)1000 ;

    switch(dvC341_Input_3D_Format_Get())  //A70LV_Doulas_0162 modify
    {
        case eINPUT_3D_TYPE_FRAME_SEQUENTIAL:
            dbVal = dbVal; //H30K_Doulas_0051 120hz error
            sprintf(aucString, "%d.%03dMHz",(UINT32)dbVal/1000,(UINT32)dbVal%1000);
            break;

        default:
            sprintf(aucString, "%d.%03dMHz",(UINT32)dbVal/1000,(UINT32)dbVal%1000);
            break;
    }
    memcpy(ucValue, aucString, strlen(aucString)+1);
}

void dvC341_InputResolution_Get(const eC341_CH_V ch_v,UINT8 *ucValue)
{
    char aucString[VERSION_STRING_MAX_LENGTH];
    UINT16 uiH_Active;
    UINT16 uiV_Active;

    if(m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.eScanMode == eSCAN_MODE_INTERLACE)  //simon check
    {
        if(m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive == 1440)
            uiH_Active = m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive/2;
        else
            uiH_Active = m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive;
        uiV_Active = m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive * 2;
    }
    else
    {
        switch(dvC341_Input_3D_Format_Get())  //A70LV_Doulas_0162 modify
        {
            case eINPUT_3D_TYPE_FRAMEPACKING: //H30K_Doulas_0026
                uiH_Active = m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive;
                if(m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive == 2205)
                    uiV_Active = 1080;
                else if(m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive == 1470)
                    uiV_Active = 720;
                else
                {
                    uiH_Active = m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive;
                    uiV_Active = m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive;
                }
                break;


            default:
                uiH_Active = m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive;
                uiV_Active = m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive;
                break;
        }
        //uiH_Active = m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive;
        //uiV_Active = m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive;
    }
    uiH_Active *= g_vboinfo[g_vbo_recno[m_sChannelInfo[ch_v].ucInputPort]].CH;

    sprintf(aucString, "%d x %d",uiH_Active ,uiV_Active);
    memcpy(ucValue, aucString, strlen(aucString)+1);
}

void dvC341_InputHorzRefresh_Get(const eC341_CH_V ch_v,UINT8 *ucValue)
{
    char aucString[VERSION_STRING_MAX_LENGTH];
    DOUBLE dbVal;

    dbVal = m_sChannelInfo[ch_v].sInputTimingInfo.dFrameRate * (DOUBLE)m_sChannelInfo[ch_v].sInputTimingInfo.uiVTotal;

    sprintf(aucString, "%d.%03dkHz",(UINT32)dbVal/1000,(UINT32)dbVal%1000);
    memcpy(ucValue, aucString, strlen(aucString)+1);
}

void dvC341_InputVertRefresh_Get(const eC341_CH_V ch_v,UINT8 *ucValue)
{
    char aucString[VERSION_STRING_MAX_LENGTH];
    UINT16 uiFraneRate;

#if 1  //G100_Steven_0068 fix mantis #15554, 後續 SWQE 同意延用 A70LV 設計
    if(m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.eScanMode == eSCAN_MODE_INTERLACE)    //A70LV_Doulas_0258 modify
    {
        uiFraneRate = (UINT16)(m_sChannelInfo[ch_v].sInputTimingInfo.dFrameRate * 100.0 / 2.0);
    }
    else
#endif
    {
        uiFraneRate = (UINT16)(m_sChannelInfo[ch_v].sInputTimingInfo.dFrameRate * 100.0);
    }
    sprintf(aucString, "%d.%02dHz",uiFraneRate/100,uiFraneRate%100);
    memcpy(ucValue, aucString, strlen(aucString)+1);
}

void dvC341_InputVertRefresh2_Get(const eC341_CH_V ch_v,UINT16 *uciValue)      //A70LV_Doulas_0200
{
    *uciValue = (UINT16)(m_sChannelInfo[ch_v].sInputTimingInfo.dFrameRate * 100.0);
}

void dvC341_InputVertRefresh2_Get_FromMCLKcount(const eC341_CH_V ch_v,UINT16 *uciValue)     //A35G2_CDS_Simon_0052
{
    if(ch_v == eC341_CH_V0)
    {
        *uciValue = (UINT16)((DOUBLE)(m_sChannelInfo[eC341_CH_V0].ulFMCLK) * 100.0 / dvC341_Read(B168_IMFVCH1, 0) ) ;  //simon check
    }
    else
    {
        *uciValue = (UINT16)((DOUBLE)(m_sChannelInfo[eC341_CH_V1].ulFMCLK) * 100.0 / dvC341_Read(B168_IMFVCH3, 0) ) ;
    }

}

void dvC341_InputSignalFormat_Get(const eC341_CH_V ch_v,UINT8 *ucValue)
{
    char aucString[VERSION_STRING_MAX_LENGTH];
    //UINT16 uiV_Active;

    if(m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.eScanMode == eSCAN_MODE_INTERLACE)
    {
        if((m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive == 1920) &&
           (m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive == 540))
        {
            sprintf(aucString, "1080i");
        }
        else if((m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive == 1440) &&
           (m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive == 288))
        {
            sprintf(aucString, "576i");
        }
        else if((m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive == 720) &&
           (m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive == 288))
        {
            sprintf(aucString, "576i");
        }
        else if((m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive == 1440) &&
           (m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive == 240))
        {
            sprintf(aucString, "480i");
        }
        else if((m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive == 720) &&
           (m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive == 240))
        {
            sprintf(aucString, "480i");
        }
        else
        {
            if(m_sChannelSetting[ch_v].eInputSource == eCM_SOURCE_VGA)
                sprintf(aucString, "Analog");
            else
                sprintf(aucString, "Digital");
        }
    }
    else
    {
        if((m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive == 1920) &&
           (m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive == 1080))
        {
            sprintf(aucString, "1080P");
        }
        else if((m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive == 1280) &&
           (m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive == 720))
        {
            sprintf(aucString, "720P");
        }
        else if((m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive == 720) &&
           (m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive == 576))
        {
            sprintf(aucString, "576P");
        }
        else if((m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive == 720) &&
           (m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive == 480))
        {
            sprintf(aucString, "480P");
        }
        else if((dvC341_Input_3D_Format_Get() == eINPUT_3D_TYPE_FRAME_SEQUENTIAL) &&
                (m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive == 960) &&
                (m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive == 1080))   //A70LV_Doulas_0162
        {
            sprintf(aucString, "1080P");
        }
        else if((dvC341_Input_3D_Format_Get() == eINPUT_3D_TYPE_FRAME_SEQUENTIAL) &&
                (m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive == 640) &&
                (m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive == 720))    //A70LV_Doulas_0162
        {
            sprintf(aucString, "720P");
        }
        else if(dvC341_Input_3D_Format_Get() == eINPUT_3D_TYPE_FRAMEPACKING)    //A70LV_Doulas_0162
        {
                if(m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive > 2150)
                    sprintf(aucString, "1080P");
                else
                    sprintf(aucString, "720P");
        }
        else
        {
            if(m_sChannelSetting[ch_v].eInputSource == eCM_SOURCE_VGA)
                sprintf(aucString, "Analog");
            else
                sprintf(aucString, "Digital");
        }
    }
    memcpy(ucValue, aucString, strlen(aucString)+1);
}

void dvC341_InputAspectRatio_Get(const eC341_CH_V ch_v,UINT8 *ucValue)
{
    char aucString[VERSION_STRING_MAX_LENGTH];
    //UINT16 uiV_Active;

    if(m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.eScanMode == eSCAN_MODE_INTERLACE)
    {
        if((m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive == 1920) &&
           (m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive == 540))
        {
            sprintf(aucString, "16:9");
        }
        else if((m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive == 1440) &&
           (m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive == 288))
        {
            sprintf(aucString, "5:4");
        }
        else if((m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive == 720) &&
           (m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive == 288))
        {
            sprintf(aucString, "5:4");
        }
        else if((m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive == 1440) &&
           (m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive == 240))
        {
            sprintf(aucString, "3:2");
        }
        else if((m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive == 720) &&
           (m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive == 240))
        {
            sprintf(aucString, "3:2");
        }
        else
        {
            sprintf(aucString, "16:9");
        }
    }
    else
    {
        if((m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive * 2 / 3) == m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive)
        {
            sprintf(aucString, "3:2");
        }
        else if((m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive * 3 / 4) == m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive)
        {
            sprintf(aucString, "4:3");
        }
        else if((m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive * 4 / 5) == m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive)
        {
            sprintf(aucString, "5:4");
        }
        else if((m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive * 9 / 16) == m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive)
        {
            sprintf(aucString, "16:9");
        }
        else if((m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive * 10 / 16) == m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive)
        {
            sprintf(aucString, "16:10");
        }
        else if((m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive * 9 / 15) == m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive)
        {
            sprintf(aucString, "15:9");
        }
        else if((m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive * 3 / 5) == m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive)
        {
            sprintf(aucString, "5:3");
        }
        else if((m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive * 5 / 6) == m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive)
        {
            sprintf(aucString, "6:5");
        }
        else
        {
            sprintf(aucString, "16:9");
        }
    }
    memcpy(ucValue, aucString, strlen(aucString)+1);
}

void dvC341_ColorSpace_Value_Set(const eC341_CH_V ch_v,UINT8 ucVal)     //A70LV_Doulas_0069
{
    m_sChannelSetting[ch_v].eColorFormat = (eCOLOR_FORMAT)ucVal;
    LOG_MSG(db_DV_SCALER, "dvC341_ColorSpace_Value_Set[%d] = %d\n",ch_v,ucVal);
}

void dvC341_FrontEndScanMode_Set(const eC341_CH_V ch_v,UINT8 ucVal)     //A70LV_Doulas_0076
{
    m_sChannelSetting[ch_v].eScanMode = ucVal;
    LOG_MSG(db_DV_SCALER, "dvC341_FrontEndScanMode_Set[%d] = %d\n",ch_v,ucVal);    //A70LV_Doulas_0079
}

void dvC341_ForcedSyncResetDisable_Set(void)     //A70LV_Doulas_0079
{
    dvC341_reg_set(B8_OHCYCLCH1, (m_sChannelInfo[eC341_CH_V0].sOutputTimingInfo.uiHTotal/OUTPUT_CH_NUM - 2));
    dvC341_reg_set(B8_OVCYCLCH1, (m_sChannelInfo[eC341_CH_V0].sOutputTimingInfo.uiVTotal - 2));

    dvC341_Write_ch_v(eC341_CH_V0, B8_SYRDLYCH1, 0x00, CH_BANK_OFFSET);//Forced sync reset delay
    dvC341_Write_ch_v(eC341_CH_V0, B8_OSYCT3CH1, 0x00, CH_BANK_OFFSET);     //A70LV_Doulas_0098 //Trigger forced sync reset in unit of HS.
    dvC341_Write_ch_v(eC341_CH_V0, B8_FLDDLYCH1, 0x00, CH_BANK_OFFSET);

    if(m_sChannelSetting[eC341_CH_V0].cPIP_PBP_ON == FALSE)
    {
        dvC341_Write_ch_v(eC341_CH_V1, B8_SYRDLYCH1, 0x00, CH_BANK_OFFSET);    //Forced sync reset delay
        dvC341_Write_ch_v(eC341_CH_V1, B8_OSYCT3CH1, 0x00, CH_BANK_OFFSET);    //Trigger forced sync reset in unit of HS.
        dvC341_Write_ch_v(eC341_CH_V1, B8_FLDDLYCH1, 0x00, CH_BANK_OFFSET);
    }

    #ifdef C341_WARPING_ENABLE  //A70LV_Doulas_0099 modify
    #if 0
    dvC341_Write(B30_OHCYCL,  m_sChannelInfo[eC341_CH_V0].sOutputTimingInfo.uiHTotal - 2, 0);
    dvC341_Write(B30_OVCYCL,  m_sChannelInfo[eC341_CH_V0].sOutputTimingInfo.uiVTotal - 2, 0);
    dvC341_Write(B30_SYRDLY, 0x00, 0);
    dvC341_Write(B30_FLDDLY, 0x50, 0);
    #endif
    #endif

    uiForcedSyncResetH_Total = m_sChannelInfo[eC341_CH_V0].sOutputTimingInfo.uiHTotal;
    uiForcedSyncResetV_Total = m_sChannelInfo[eC341_CH_V0].sOutputTimingInfo.uiVTotal;

    bForcedSyncResetEnable = FALSE;
}

void dvC341_ForcedSyncResetDisable_Set2(UINT16 uiH_Toral,UINT16 uiV_Toral)     //ZU860_Doulas_0138
{
    dvC341_reg_set(B8_OHCYCLCH1, (uiH_Toral/OUTPUT_CH_NUM - 2));
    dvC341_reg_set(B8_OVCYCLCH1, (uiV_Toral - 2));

    dvC341_Write_ch_v(eC341_CH_V0, B8_SYRDLYCH1, 0x00, CH_BANK_OFFSET);    //Forced sync reset delay
    dvC341_Write_ch_v(eC341_CH_V0, B8_OSYCT3CH1, 0x00, CH_BANK_OFFSET);    //Trigger forced sync reset in unit of HS.
    dvC341_Write_ch_v(eC341_CH_V0, B8_FLDDLYCH1, 0x00, CH_BANK_OFFSET);

    if(m_sChannelSetting[eC341_CH_V0].cPIP_PBP_ON == FALSE)
    {
        dvC341_Write_ch_v(eC341_CH_V1, B8_SYRDLYCH1, 0x00, CH_BANK_OFFSET);    //Forced sync reset delay
        dvC341_Write_ch_v(eC341_CH_V1, B8_OSYCT3CH1, 0x00, CH_BANK_OFFSET);    //Trigger forced sync reset in unit of HS.
        dvC341_Write_ch_v(eC341_CH_V1, B8_FLDDLYCH1, 0x00, CH_BANK_OFFSET);
    }

    #ifdef C341_WARPING_ENABLE
    #if 0
    dvC341_Write(B30_OHCYCL,  uiH_Toral - 2, 0);
    dvC341_Write(B30_OVCYCL,  uiV_Toral - 2, 0);
    dvC341_Write(B30_SYRDLY, 0x00, 0);
    dvC341_Write(B30_FLDDLY, 0x50, 0);
    #endif
    #endif
    uiForcedSyncResetH_Total = uiH_Toral;
    uiForcedSyncResetV_Total = uiV_Toral;

    bForcedSyncResetEnable = FALSE;
}

BOOL dvC341_ForcedSyncResetType_Get(void)
{
    return bForcedSyncResetEnable;
}

void dvC341_ForceSyncReset_Get(BOOL *bForceSyncReseVal,UINT16 *uiH_Toral,UINT16 *uiV_Toral)
{
    *bForceSyncReseVal = bForcedSyncResetEnable;//m_sChannelInfo[eC341_CH_V0].bForceSyncReset; //H30K_Doulas_0016
    *uiH_Toral = uiForcedSyncResetH_Total;
    *uiV_Toral = uiForcedSyncResetV_Total;
}

BOOL dvC341_IsInterlaced_Get(const eC341_CH_V ch_v)       //A70LV_Doulas_0092
{
    if(m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.eScanMode == eSCAN_MODE_INTERLACE)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

ePANEL_ID dvC341_PanelGet(void)       //A70LV_Doulas_0098
{
     return m_sChannelInfo[eC341_CH_V0].ePanelTimingId;;
}

void dvC341_VideoYUV_Set(const eC341_CH_V ch_v,UINT8 ucVal)     //A70LV_Doulas_0109
{
    m_sChannelSetting[ch_v].eVideoYUV = ucVal;
    LOG_MSG(db_DV_SCALER, "dvC341_VideoYUV_Set[%d] = %d\n",ch_v,ucVal);
}

UINT8 dvC341_VideoYUV_Get(const eC341_CH_V ch_v)   //A70LV_Doulas_0109
{
	UINT8 icolor_space;


    if(m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.eScanMode == eSCAN_MODE_INTERLACE)
    {
        icolor_space = (m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.uiHActive > 1600) ? 1 : 0;	//0:BT-601, 1:BT-709
    }
    else
    {
	    icolor_space = (m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.uiHActive > 1024) ? 1 : 0;	//0:BT-601, 1:BT-709
    }

    return icolor_space;
}

void dvC341_AutoImage_Set(UINT8 ucSource,UINT8 ucVal)     //A70LV_Doulas_0112
{
    if(ucSource < eCM_SOURCE_NUMBER)
    {
        m_AutoImage[ucSource] = ucVal;
        LOG_MSG(db_DV_SCALER, "dvC341_AutoImage_Set[%d] = %d\n",ucSource,ucVal);
    }
}

UINT8 dvC341_AutoImage_Get(void)     //A70LV_Doulas_0112
{
    return m_AutoImage[m_sChannelSetting[eC341_CH_V0].eInputSource];   //only get main
}

DOUBLE dvC341_uiHFreq_Get(const eC341_CH_V ch_v)     //A70LV_Doulas_0115
{
    return m_sChannelInfo[ch_v].sInputTimingInfo.dHFreq;
}

void dvC341_Position_Run(eC341_CH_V ch_v)  //A70LV_Doulas_0113
{
    if(!m_sChannelInfo[ch_v].bInit)
    {
        return;
    }
    dvC341_Calc_Scaler_InputAndOutput_Window_4K(ch_v);
	dvC341_Set_ZoomShrink_4K(ch_v);
    dvC341_Set_Color(ch_v);
	dvC341_Set_IP(ch_v);
    dvC341_Set_flddly(ch_v);
    dvC341_Set_Flip(ch_v);   //G100_Simon_0076
   // dvC341_ConfigOutput_2K(eCH);
}

BOOL dvC341_CheckingHorzPositionSettingWork(eC341_CH_V ch_v)
{
    if(WorkingHorzPosition[ch_v] == m_sChannelSetting[ch_v].ucHoriPosition)
        return TRUE;
    else
        return FALSE;
}

BOOL dvC341_CheckingVertPositionSettingWork(eC341_CH_V ch_v)
{
    if(WorkingVertPosition[ch_v] == m_sChannelSetting[ch_v].ucVertPosition)
        return TRUE;
    else
        return FALSE;
}

UINT8 dvC341_HorzPositionWorkValueGet(eC341_CH_V ch_v)
{
    return WorkingHorzPosition[ch_v];
}

UINT8 dvC341_VertPositionWorkValueGet(eC341_CH_V ch_v)
{
    return WorkingVertPosition[ch_v];
}

void dvC341_AutoPsitionStart(eC341_CH_V ch_v,BOOL bAutoEnable)     //A70LV_Doulas_0118
{
    m_sChannelInfo[ch_v].bAutoPosition = bAutoEnable;
}

void dvC341_FlashWrite(UINT32 ulAddress, UINT16 uiSize, UINT8 *pucData)
{
    UINT8   ucSFLDTCTL = 0xff;
    UINT8   ucSFLSTAT = 0xff;
    UINT8   ucRDTCHK = 0xff;

    dvC341_Write(B5_GIOS, 0x000055, 0);//2bits for each GIO[X]
	dvC341_Write(B5_GIOPDCT,0x03FF, 0);//Enable pulldown for all GIO
	dvC341_Write(B5_GIOC, 0x000300, 0);//Drive   GIO[9:8](WP#,HLD#)
	dvC341_Write(B5_GIOO, 0x000300, 0);//High to GIO[9:8](WP#,HLD#)
	dvC341_Write(B4_OSDCT, 0x00, 0);

    dvC341_Write(B4_SFLCT, 0x00, 0);
    dvC341_Write(B4_SFLMODE, 0x80, 0);
    dvC341_Write(B4_SFLMODE, 0x33, 0);

    dvC341_Write(B4_SFLCMDWR, 0x12, 0);
    dvC341_Write(B4_SFLCMDWREN, 0x06, 0);
    dvC341_Write(B4_SFLCMDRDSR, 0x05, 0);
    dvC341_Write(B4_SFLSZSEL, 0x01, 0);
    dvC341_Write(B4_SFLAD, ulAddress, 0);

    dvC341_ResetTimeInState();
    do
    {
        MS_SLEEP(10);
        ucSFLDTCTL = dvC341_Read(B4_SFLDTCTL, 0);

        if(dvC341_TimeElapsedInState() >= 1000)
        {
            ASSERT_ALWAYS();
            break;
        }
    }
    while((ucSFLDTCTL & 0x20) == 0x00);
    LOG_MSG(db_DV_SCALER, "(func:%s, line:%d) time (%d)\n", __FUNCTION__, __LINE__, dvC341_TimeElapsedInState());

    dvC341_ResetTimeInState();
    do
    {
        MS_SLEEP(10);
        ucSFLSTAT = dvC341_Read(B4_SFLSTAT, 0);

        if(dvC341_TimeElapsedInState() >= 1000)
        {
            ASSERT_ALWAYS();
            break;
        }
    }
    while((ucSFLSTAT & 0x02) != 0x00);
    LOG_MSG(db_DV_SCALER, "(func:%s, line:%d) time (%d)\n", __FUNCTION__, __LINE__, dvC341_TimeElapsedInState());

    dvC341_BurstWrite_FixedAdd(B4_SFLDT, uiSize, pucData);

}

void dvC341_FlashRead(UINT32 ulAddress, UINT16 uiSize, UINT8 *pucData)
{
    UINT8   ucSFLDTCTL = 0xff;
    UINT8   ucSFLSTAT = 0xff;
    UINT8   ucRDTCHK = 0xff;

    dvC341_Write(B5_GIOS, 0x000055, 0);//2bits for each GIO[X]
	dvC341_Write(B5_GIOPDCT,0x03FF, 0);//Enable pulldown for all GIO
	dvC341_Write(B5_GIOC, 0x000300, 0);//Drive   GIO[9:8](WP#,HLD#)
	dvC341_Write(B5_GIOO, 0x000300, 0);//High to GIO[9:8](WP#,HLD#)
	dvC341_Write(B4_OSDCT, 0x00, 0);

    dvC341_Write(B4_SFLCT, 0x00, 0);
    dvC341_Write(B4_SFLMODE, 0x80, 0);
    dvC341_Write(B4_SFLMODE, 0x33, 0);

    dvC341_Write(B4_SFLCMDRD, 0x0c, 0);
    dvC341_Write(B4_SFLSZSEL, 0x01, 0);
    dvC341_Write(B4_SFLAD, ulAddress, 0);
    dvC341_Write(B4_SFLDTCTL, 0x01, 0);

    dvC341_ResetTimeInState();
    do
    {
        MS_SLEEP(10);
        ucSFLDTCTL = dvC341_Read(B4_SFLDTCTL, 0);

        if(dvC341_TimeElapsedInState() >= 1000)
        {
            ASSERT_ALWAYS();
            break;
        }
    }
    while((ucSFLDTCTL & 0x20) == 0x00);
    LOG_MSG(db_DV_SCALER, "(func:%s, line:%d) time (%d)\n", __FUNCTION__, __LINE__, dvC341_TimeElapsedInState());

    dvC341_BurstRead_FixedAdd(B4_SFLDT, uiSize, pucData);
    dvC341_Write(B4_SFLAD, 0, 0);

}

void dvC341_FlashEraseSector(UINT32 ulSflAd, UINT32 ulDataSize) //A70LV_Larry_0139
{
    UINT32 ulEraseAddrStart = ulSflAd;
    UINT32 ulEraseAddrEnd   = ulSflAd + ulDataSize;
    UINT8  ucRDTCHK = 0;

    if((ulEraseAddrStart & 0x0000FFFF) || (ulEraseAddrEnd & 0x0000FFFF))
    {
        LOG_MSG(db_DV_SCALER_OSD, "Not matching 64k\n");
        return;
    }

    for(ulEraseAddrStart = ulSflAd; ulEraseAddrStart < ulEraseAddrEnd; ulEraseAddrStart += 0x00010000) //64k
    {
        dvC341_Write(B4_SFLSZSEL, 0x01, 0);
        dvC341_Write(B4_SFLCT, 0x00, 0);
        dvC341_Write(B4_SFLMODE, 0x80, 0);
        dvC341_Write(B4_SFLMODE, 0x23, 0); //A35G2_CDS_Larry_0032 0x33 -> 0x23
        dvC341_Write(B4_SFLCT, 0x00, 0);
        dvC341_Write(B4_SFLCMDWR, 0x12, 0);  //Set the page write command to the serial Flash connected
        dvC341_Write(B4_SFLCMDRD, 0x0C, 0);  //Set a read or fast read command to be executed for the serial Flash connected
        dvC341_Write(B4_SFLCMDER, 0xDC, 0);  //Set a selector erase or bulk (chip) erase command to be executed for the serial Flash connected
        dvC341_Write(B4_SFLCMDWREN, 0x06, 0);  //Set the value of WREN command of the serial Flash connected
        dvC341_Write(B4_SFLCMDRDSR, 0x05, 0);  //Set the value of RDSR command of the serial Flash connected
        dvC341_Write(B4_SFLAD, ulEraseAddrStart, 0);  //Specify the serial Flash address. After accessing, auto increment starts
        dvC341_Write(B4_SFLCT, 0x04, 0);

        dvC341_ResetTimeInState();

        do
        {
            ucRDTCHK = dvC341_Read(B4_SFLSTAT, 0); //A70LV_Larry_0054
            MS_SLEEP(100);

            if(dvC341_TimeElapsedInState() >= 5000)
            {
                ASSERT_ALWAYS();
                return;
            }

        } while ((ucRDTCHK & 0x02)==0x02);

        dvC341_ResetTimeInState();
        do
        {
            ucRDTCHK = dvC341_Read(B4_SFLRDSR, 0); //A70LV_Larry_0054
            MS_SLEEP(100);

            if(dvC341_TimeElapsedInState() >= 5000)
            {
                ASSERT_ALWAYS();
                return;
            }
        } while ((ucRDTCHK & 0x03)!=0x00);

    }


    return;
}

void dvC341_DMA_Transfer(UINT32 ulAddress, UINT32 ulSize, UINT8 ucRegsel, UINT32 ulDmaCnt)
{
    UINT32 ucSFLSTAT = 0;

    dvC341_Write(B4_SFLCT,0x00,0);
    dvC341_Write(B4_SFLMODE,0x80,0);
    dvC341_Write(B4_SFLMODE,0x33,0);
    //dvC341_Write(B4_SFLCMDWR,0x12,0);
    dvC341_Write(B4_SFLCMDRD,0x0c,0);
    dvC341_Write(B4_SFLSZSEL,0x01,0);
    //dvC341_Write(B4_SFLCMDER,0xC7,0);
    //dvC341_Write(B4_SFLCMDWREN,0x06,0);
    //dvC341_Write(B4_SFLCMDRDSR,0x05,0);
    dvC341_Write(B4_SFLAD,ulAddress,0);
    dvC341_Write(B4_SFLCNT,ulSize,0);
    dvC341_Write(B4_SFLREGSEL,ucRegsel,0);
    dvC341_Write(B4_SFLCT,ulDmaCnt,0);

    dvC341_ResetTimeInState();

    do
    {
        ucSFLSTAT = dvC341_Read( B4_SFLSTAT, 0 );
        MS_SLEEP(10);

        if(dvC341_TimeElapsedInState() >= 1000)
        {
            ASSERT_ALWAYS();
            return;
        }

    }while((ucSFLSTAT & 0x01) != 0) ; // busy waiting
    LOG_MSG(db_DV_SCALER, "(func:%s, line:%d) time (%d)\n", __FUNCTION__, __LINE__, dvC341_TimeElapsedInState());
}

void dvC341_DDR_InputDataGet(eC341_CH_V ch_v,UINT16 uiPixelHPosition,UINT16 uiPixelVPosition)      //A70LV_Doulas_0124
{
#if 0  //simon check .. not implement
    UINT8  ucRegOffset = 0;
    UINT32 ulLineAddr;
    UINT32 ulPixelAddr;

    UINT8  ucR_h8bit;
    UINT8  ucG_h8bit;
    UINT8  ucB_h8bit;
    UINT8  ucRGB_LSB;

    UINT16 uiRed;
    UINT16 uiGreen;
    UINT16 uiBlue;
    UINT8  ucVAl;

    if(ch_v >= eC341_CH_VNUMBER)
    {
        return;
    }


    switch(ch_v)
    {
        case eC341_CH_V0:
            dvC341_Write(B0_MCT1,  m_sChannelInfo[ch_v].ucMCT & 0xEF, 0);    //set image input idle
            dvC341_Write(B0_MCT2,  m_sChannelInfo[ch_v].ucMCT & 0xEF, 0);    //set image input idle
            break;

        case eC341_CH_V1:
            dvC341_Write(B0_MCT3,  m_sChannelInfo[ch_v].ucMCT & 0xEF, 0);    //set image input idle
            dvC341_Write(B0_MCT4,  m_sChannelInfo[ch_v].ucMCT & 0xEF, 0);    //set image input idle
            break;
    }
    //M0
    ulLineAddr = DEF_ISFLD0CH1_4K + (UINT32)DEF_MWI_4K *64*4* (UINT32)uiPixelVPosition;
    ulPixelAddr = ulLineAddr + (UINT32)uiPixelHPosition * 4;    //one pixel 4BYTE

    dvC341_Write(B4_CPURAD, ulPixelAddr ,ucRegOffset);
    dvC341_Write(B4_CPUDTCTL, 0x01 ,ucRegOffset);


    dvC341_ResetTimeInState();

    ucVAl = 0x08;
    while( (ucVAl & 0x08) != 0)
    {
        ucVAl = (UINT8)dvC341_Read(B4_CPUDTCTL,ucRegOffset);

        if(dvC341_TimeElapsedInState() >= 1000)
        {
            ASSERT_ALWAYS();
            return;
        }
    }

    ucR_h8bit = dvC341_Read(B4_CPUDT,ucRegOffset);
    ucG_h8bit = dvC341_Read(B4_CPUDT,ucRegOffset);
    ucB_h8bit = dvC341_Read(B4_CPUDT,ucRegOffset);
    ucRGB_LSB = dvC341_Read(B4_CPUDT,ucRegOffset);

    uiRed   = (UINT16)(ucR_h8bit << 2) + (UINT16)(ucRGB_LSB & 0x03);
    uiGreen = (UINT16)(ucG_h8bit << 2) + (UINT16)((ucRGB_LSB>>2) & 0x03);
    uiBlue  = (UINT16)(ucB_h8bit << 2) + (UINT16)((ucRGB_LSB>>4) & 0x03);
    LOG_MSG(db_DV_SCALER, "\r\n DEF_ISFLD0 %x (%d,%d,%d) \r\n",ulPixelAddr,uiRed,uiGreen,uiBlue);

    //M1
    ulLineAddr = DEF_ISFLD1CH1_4K + (UINT32)DEF_MWI_4K *64*4* (UINT32)uiPixelVPosition;
    ulPixelAddr = ulLineAddr + (UINT32)uiPixelHPosition * 4;    //one pixel 4BYTE

    dvC341_Write(B4_CPURAD, ulPixelAddr ,ucRegOffset);
    dvC341_Write(B4_CPUDTCTL, 0x01 ,ucRegOffset);

    dvC341_ResetTimeInState();

    ucVAl = 0x08;
    while( (ucVAl & 0x08) != 0)
    {
        ucVAl = (UINT8)dvC341_Read(B4_CPUDTCTL,ucRegOffset);

        if(dvC341_TimeElapsedInState() >= 1000)
        {
            ASSERT_ALWAYS();
            return;
        }
    }

    ucR_h8bit = dvC341_Read(B4_CPUDT,ucRegOffset);
    ucG_h8bit = dvC341_Read(B4_CPUDT,ucRegOffset);
    ucB_h8bit = dvC341_Read(B4_CPUDT,ucRegOffset);
    ucRGB_LSB = dvC341_Read(B4_CPUDT,ucRegOffset);

    uiRed   = (UINT16)(ucR_h8bit << 2) + (UINT16)(ucRGB_LSB & 0x03);
    uiGreen = (UINT16)(ucG_h8bit << 2) + (UINT16)((ucRGB_LSB>>2) & 0x03);
    uiBlue  = (UINT16)(ucB_h8bit << 2) + (UINT16)((ucRGB_LSB>>4) & 0x03);
    LOG_MSG(db_DV_SCALER, " DEF_ISFLD1 %x (%d,%d,%d) \r\n",ulPixelAddr,uiRed,uiGreen,uiBlue);

    //M2
    ulLineAddr = DEF_ISFLD2CH1_4K + (UINT32)DEF_MWI_4K *64*4* (UINT32)uiPixelVPosition;
    ulPixelAddr = ulLineAddr + (UINT32)uiPixelHPosition * 4;    //one pixel 4BYTE

    dvC341_Write(B4_CPURAD, ulPixelAddr ,ucRegOffset);
    dvC341_Write(B4_CPUDTCTL, 0x01 ,ucRegOffset);

    dvC341_ResetTimeInState();

    ucVAl = 0x08;
    while( (ucVAl & 0x08) != 0)
    {
        ucVAl = (UINT8)dvC341_Read(B4_CPUDTCTL,ucRegOffset);

        if(dvC341_TimeElapsedInState() >= 1000)
        {
            ASSERT_ALWAYS();
            return;
        }
    }

    ucR_h8bit = dvC341_Read(B4_CPUDT,ucRegOffset);
    ucG_h8bit = dvC341_Read(B4_CPUDT,ucRegOffset);
    ucB_h8bit = dvC341_Read(B4_CPUDT,ucRegOffset);
    ucRGB_LSB = dvC341_Read(B4_CPUDT,ucRegOffset);

    uiRed   = (UINT16)(ucR_h8bit << 2) + (UINT16)(ucRGB_LSB & 0x03);
    uiGreen = (UINT16)(ucG_h8bit << 2) + (UINT16)((ucRGB_LSB>>2) & 0x03);
    uiBlue  = (UINT16)(ucB_h8bit << 2) + (UINT16)((ucRGB_LSB>>4) & 0x03);
    LOG_MSG(db_DV_SCALER, " DEF_ISFLD2 %x (%d,%d,%d) \r\n",ulPixelAddr,uiRed,uiGreen,uiBlue);

    //M3
    ulLineAddr = DEF_ISFLD3CH1_4K + (UINT32)DEF_MWI_4K *64*4* (UINT32)uiPixelVPosition;
    ulPixelAddr = ulLineAddr + (UINT32)uiPixelHPosition * 4;    //one pixel 4BYTE

    dvC341_Write(B4_CPURAD, ulPixelAddr ,ucRegOffset);
    dvC341_Write(B4_CPUDTCTL, 0x01 ,ucRegOffset);

    dvC341_ResetTimeInState();

    ucVAl = 0x08;
    while( (ucVAl & 0x08) != 0)
    {
        ucVAl = (UINT8)dvC341_Read(B4_CPUDTCTL,ucRegOffset);

        if(dvC341_TimeElapsedInState() >= 1000)
        {
            ASSERT_ALWAYS();
            return;
        }
    }

    ucR_h8bit = dvC341_Read(B4_CPUDT,ucRegOffset);
    ucG_h8bit = dvC341_Read(B4_CPUDT,ucRegOffset);
    ucB_h8bit = dvC341_Read(B4_CPUDT,ucRegOffset);
    ucRGB_LSB = dvC341_Read(B4_CPUDT,ucRegOffset);

    uiRed   = (UINT16)(ucR_h8bit << 2) + (UINT16)(ucRGB_LSB & 0x03);
    uiGreen = (UINT16)(ucG_h8bit << 2) + (UINT16)((ucRGB_LSB>>2) & 0x03);
    uiBlue  = (UINT16)(ucB_h8bit << 2) + (UINT16)((ucRGB_LSB>>4) & 0x03);
    LOG_MSG(db_DV_SCALER, " DEF_ISFLD3 %x (%d,%d,%d) \r\n",ulPixelAddr,uiRed,uiGreen,uiBlue);

    switch(ch_v)
    {
        case eC341_CH_V0:
            dvC341_Write(B0_MCT1,  m_sChannelInfo[ch_v].ucMCT, 0);
            dvC341_Write(B0_MCT2,  m_sChannelInfo[ch_v].ucMCT, 0);
            break;

        case eC341_CH_V1:
            dvC341_Write(B0_MCT3,  m_sChannelInfo[ch_v].ucMCT, 0);
            dvC341_Write(B0_MCT4,  m_sChannelInfo[ch_v].ucMCT, 0);
            break;
    }
#endif
}

void dvC341_DDR_InputDataAreaTotalGet(eC341_CH_V eCH,RECT DataArea,UINT32 *ulTotalRed,UINT32 *ulTotalGreen,UINT32 *ulTotalBlue)
{
#if 0  //simon check .. not implement
    UINT8  ucRegOffset = 0;
    UINT32 ulLineAddr;
    UINT32 ulPixelAddr;

    UINT8  ucR_h8bit;
    UINT8  ucG_h8bit;
    UINT8  ucB_h8bit;
    UINT8  ucRGB_LSB;

    UINT16 uiRed;
    UINT16 uiGreen;
    UINT16 uiBlue;
    UINT8  ucVAl;
    UINT32 ulTotal[3];
    UINT16 uiPixelHPosition;
    UINT16 uiPixelVPosition;
    UINT32 ulAreaCount = 0;

    if(eCH >= eC341_CH_VNUMBER)
    {
        return;
    }

    ucRegOffset = (eCH == eMCT_CH1) ? 0 : 13;

    switch(eCH)
    {
        case eMCT_CH1:
            dvC341_Write(BN_MCT1,  m_sChannelInfo[eCH].ucMCT & 0xEF, 0);    //set image input idle
            break;

        case eMCT_CH2:
            dvC341_Write(BN_MCT2,  m_sChannelInfo[eCH].ucMCT & 0xEF, 0);    //set image input idle
            break;
    }
    ulTotal[0] = 0;
    ulTotal[1] = 0;
    ulTotal[2] = 0;

    //M0
    uiPixelHPosition = DataArea.wLeft;
    uiPixelVPosition = DataArea.wTop;

    for(;uiPixelVPosition <= DataArea.wBottom ; uiPixelVPosition++)
    {
        ulLineAddr = DEF_ISFLD0CH1_2K + (UINT32)DEF_MWI_2K *64*4* (UINT32)uiPixelVPosition;
        ulPixelAddr = ulLineAddr + (UINT32)uiPixelHPosition * 4;    //one pixel 4BYTE

        dvC341_Write(B0_CPURAD, ulPixelAddr ,ucRegOffset);
        dvC341_Write(B0_CPUDTCTL, 0x01 ,ucRegOffset);

        dvC341_ResetTimeInState();

        ucVAl = 0x08;
        while( (ucVAl & 0x08) != 0)
        {
            ucVAl = (UINT8)dvC341_Read(B0_CPUDTCTL,ucRegOffset);

            if(dvC341_TimeElapsedInState() >= 1000)
            {
                ASSERT_ALWAYS();
                return;
            }
        }

        for(; uiPixelHPosition <= DataArea.wRight; uiPixelHPosition++)
        {
            ucR_h8bit = dvC341_Read(B0_CPUDT,ucRegOffset);
            ucG_h8bit = dvC341_Read(B0_CPUDT,ucRegOffset);
            ucB_h8bit = dvC341_Read(B0_CPUDT,ucRegOffset);
            ucRGB_LSB = dvC341_Read(B0_CPUDT,ucRegOffset);

            uiRed   = (UINT16)(ucR_h8bit << 2) + (UINT16)(ucRGB_LSB & 0x03);
            uiGreen = (UINT16)(ucG_h8bit << 2) + (UINT16)((ucRGB_LSB>>2) & 0x03);
            uiBlue  = (UINT16)(ucB_h8bit << 2) + (UINT16)((ucRGB_LSB>>4) & 0x03);
            ulTotal[0] += (UINT32)uiRed;
            ulTotal[1] += (UINT32)uiGreen;
            ulTotal[2] += (UINT32)uiBlue;
        }
        uiPixelHPosition = DataArea.wLeft;
    }
    //LOG_MSG(db_ALWAYS, "\r\n DEF_ISFLD0 %x (%d,%d,%d) \r\n",ulPixelAddr,uiRed,uiGreen,uiBlue);

    //M1
    uiPixelHPosition = DataArea.wLeft;
    uiPixelVPosition = DataArea.wTop;

    for(;uiPixelVPosition <= DataArea.wBottom ; uiPixelVPosition++)
    {
        ulLineAddr = DEF_ISFLD1CH1_2K + (UINT32)DEF_MWI_2K *64*4* (UINT32)uiPixelVPosition;
        ulPixelAddr = ulLineAddr + (UINT32)uiPixelHPosition * 4;    //one pixel 4BYTE

        dvC341_Write(B0_CPURAD, ulPixelAddr ,ucRegOffset);
        dvC341_Write(B0_CPUDTCTL, 0x01 ,ucRegOffset);

        dvC341_ResetTimeInState();

        ucVAl = 0x08;
        while( (ucVAl & 0x08) != 0)
        {
            ucVAl = (UINT8)dvC341_Read(B0_CPUDTCTL,ucRegOffset);

            if(dvC341_TimeElapsedInState() >= 1000)
            {
                ASSERT_ALWAYS();
                return;
            }
        }

        for(; uiPixelHPosition <= DataArea.wRight; uiPixelHPosition++)
        {
            ucR_h8bit = dvC341_Read(B0_CPUDT,ucRegOffset);
            ucG_h8bit = dvC341_Read(B0_CPUDT,ucRegOffset);
            ucB_h8bit = dvC341_Read(B0_CPUDT,ucRegOffset);
            ucRGB_LSB = dvC341_Read(B0_CPUDT,ucRegOffset);

            uiRed   = (UINT16)(ucR_h8bit << 2) + (UINT16)(ucRGB_LSB & 0x03);
            uiGreen = (UINT16)(ucG_h8bit << 2) + (UINT16)((ucRGB_LSB>>2) & 0x03);
            uiBlue  = (UINT16)(ucB_h8bit << 2) + (UINT16)((ucRGB_LSB>>4) & 0x03);
            ulTotal[0] += (UINT32)uiRed;
            ulTotal[1] += (UINT32)uiGreen;
            ulTotal[2] += (UINT32)uiBlue;
        }
        uiPixelHPosition = DataArea.wLeft;
    }
    //LOG_MSG(db_ALWAYS, " DEF_ISFLD1 %x (%d,%d,%d) \r\n",ulPixelAddr,uiRed,uiGreen,uiBlue);

    //M2
    uiPixelHPosition = DataArea.wLeft;
    uiPixelVPosition = DataArea.wTop;

    for(;uiPixelVPosition <= DataArea.wBottom ; uiPixelVPosition++)
    {
        ulLineAddr = DEF_ISFLD2CH1_2K + (UINT32)DEF_MWI_2K *64*4* (UINT32)uiPixelVPosition;
        ulPixelAddr = ulLineAddr + (UINT32)uiPixelHPosition * 4;    //one pixel 4BYTE

        dvC341_Write(B0_CPURAD, ulPixelAddr ,ucRegOffset);
        dvC341_Write(B0_CPUDTCTL, 0x01 ,ucRegOffset);

        dvC341_ResetTimeInState();

        ucVAl = 0x08;
        while( (ucVAl & 0x08) != 0)
        {
            ucVAl = (UINT8)dvC341_Read(B0_CPUDTCTL,ucRegOffset);
            if(dvC341_TimeElapsedInState() >= 1000)
            {
                ASSERT_ALWAYS();
                return;
            }
        }

        for(; uiPixelHPosition <= DataArea.wRight; uiPixelHPosition++)
        {
            ucR_h8bit = dvC341_Read(B0_CPUDT,ucRegOffset);
            ucG_h8bit = dvC341_Read(B0_CPUDT,ucRegOffset);
            ucB_h8bit = dvC341_Read(B0_CPUDT,ucRegOffset);
            ucRGB_LSB = dvC341_Read(B0_CPUDT,ucRegOffset);

            uiRed   = (UINT16)(ucR_h8bit << 2) + (UINT16)(ucRGB_LSB & 0x03);
            uiGreen = (UINT16)(ucG_h8bit << 2) + (UINT16)((ucRGB_LSB>>2) & 0x03);
            uiBlue  = (UINT16)(ucB_h8bit << 2) + (UINT16)((ucRGB_LSB>>4) & 0x03);
            ulTotal[0] += (UINT32)uiRed;
            ulTotal[1] += (UINT32)uiGreen;
            ulTotal[2] += (UINT32)uiBlue;
        }
        uiPixelHPosition = DataArea.wLeft;
    }
    //LOG_MSG(db_ALWAYS, " DEF_ISFLD2 %x (%d,%d,%d) \r\n",ulPixelAddr,uiRed,uiGreen,uiBlue);

    //M3
    uiPixelHPosition = DataArea.wLeft;
    uiPixelVPosition = DataArea.wTop;

    for(;uiPixelVPosition <= DataArea.wBottom ; uiPixelVPosition++)
    {
        ulLineAddr = DEF_ISFLD3CH1_2K + (UINT32)DEF_MWI_2K *64*4* (UINT32)uiPixelVPosition;
        ulPixelAddr = ulLineAddr + (UINT32)uiPixelHPosition * 4;    //one pixel 4BYTE

        dvC341_Write(B0_CPURAD, ulPixelAddr ,ucRegOffset);
        dvC341_Write(B0_CPUDTCTL, 0x01 ,ucRegOffset);

        dvC341_ResetTimeInState();

        ucVAl = 0x08;
        while( (ucVAl & 0x08) != 0)
        {
            ucVAl = (UINT8)dvC341_Read(B0_CPUDTCTL,ucRegOffset);
            if(dvC341_TimeElapsedInState() >= 1000)
            {
                ASSERT_ALWAYS();
                return;
            }
        }

        for(; uiPixelHPosition <= DataArea.wRight; uiPixelHPosition++)
        {
            ucR_h8bit = dvC341_Read(B0_CPUDT,ucRegOffset);
            ucG_h8bit = dvC341_Read(B0_CPUDT,ucRegOffset);
            ucB_h8bit = dvC341_Read(B0_CPUDT,ucRegOffset);
            ucRGB_LSB = dvC341_Read(B0_CPUDT,ucRegOffset);

            uiRed   = (UINT16)(ucR_h8bit << 2) + (UINT16)(ucRGB_LSB & 0x03);
            uiGreen = (UINT16)(ucG_h8bit << 2) + (UINT16)((ucRGB_LSB>>2) & 0x03);
            uiBlue  = (UINT16)(ucB_h8bit << 2) + (UINT16)((ucRGB_LSB>>4) & 0x03);
            ulTotal[0] += (UINT32)uiRed;
            ulTotal[1] += (UINT32)uiGreen;
            ulTotal[2] += (UINT32)uiBlue;
        }
        uiPixelHPosition = DataArea.wLeft;
    }
    //LOG_MSG(db_ALWAYS, " DEF_ISFLD3 %x (%d,%d,%d) \r\n",ulPixelAddr,uiRed,uiGreen,uiBlue);

    *ulTotalRed   = ulTotal[0]/4;
    *ulTotalGreen = ulTotal[1]/4;
    *ulTotalBlue  = ulTotal[2]/4;
    ulAreaCount = (UINT32)(DataArea.wRight - DataArea.wLeft +1) * (UINT32)(DataArea.wBottom - DataArea.wTop +1);
    LOG_MSG(db_DV_SCALER, "[DDR(%d,%d,%d)] \r\n",(*ulTotalRed)/ulAreaCount, (*ulTotalGreen)/ulAreaCount, (*ulTotalBlue)/ulAreaCount);


    switch(eCH)
    {
        case eMCT_CH1:
            dvC341_Write(BN_MCT1,  m_sChannelInfo[eCH].ucMCT, 0);
            break;

        case eMCT_CH2:
            dvC341_Write(BN_MCT2,  m_sChannelInfo[eCH].ucMCT, 0);
            break;
    }
#endif
}

void dvC341_DDR_InputDataSet(eC341_CH_V eCH,UINT16 uiPixelHPosition,UINT16 uiPixelVPosition,UINT16 uiRed,UINT16 uiGreen,UINT16 uiBlue)
{
#if 0  //simon check .. not implement
    UINT8  ucRegOffset = 0;
    UINT32 ulLineAddr;
    UINT32 ulPixelAddr;

    UINT8  ucR_h8bit;
    UINT8  ucG_h8bit;
    UINT8  ucB_h8bit;
    UINT8  ucRGB_LSB;

    //UINT16 uiRed;
    //UINT16 uiGreen;
    //UINT16 uiBlue;
    UINT16  i,j;


    if(eCH >= eC341_CH_VNUMBER)
    {
        return;
    }

    ucRegOffset = (eCH == eMCT_CH1) ? 0 : 13;

    switch(eCH)
    {
        case eMCT_CH1:
            dvC341_Write(BN_MCT1,  m_sChannelInfo[eCH].ucMCT & 0xEF, 0);    //set image input idle
            break;

        case eMCT_CH2:
            dvC341_Write(BN_MCT2,  m_sChannelInfo[eCH].ucMCT & 0xEF, 0);    //set image input idle
            break;
    }

    //M0
    ucR_h8bit = (UINT8)((uiRed >> 2) & 0xFF);
    ucG_h8bit = (UINT8)((uiGreen >> 2) & 0xFF);
    ucB_h8bit = (UINT8)((uiBlue >> 2) & 0xFF);
    ucRGB_LSB = (UINT8)(uiRed & 0x03) + (UINT8)((uiGreen & 0x03)<<2) + (UINT8)((uiRed & 0x03)<<4);

    i = uiPixelVPosition;
    j = uiPixelHPosition;
    for( ; i< uiPixelVPosition +3 ; i++)
    {
        ulLineAddr = DEF_ISFLD0CH1_2K + (UINT32)DEF_MWI_2K *64*4* (UINT32)i;
        ulPixelAddr = ulLineAddr + (UINT32)j * 4;    //one pixel 4BYTE
        dvC341_Write(B0_CPURAD, ulPixelAddr ,ucRegOffset);
        LOG_MSG(db_DV_SCALER, " Set DEF_ISFLD0 %x (%d,%d,%d) \r\n",ulPixelAddr,uiRed,uiGreen,uiBlue);
        for( ; j< uiPixelHPosition +3 ; j++)
        {
            dvC341_Write(B0_CPUDT, ucR_h8bit, ucRegOffset);
            dvC341_Write(B0_CPUDT, ucG_h8bit, ucRegOffset);
            dvC341_Write(B0_CPUDT, ucB_h8bit, ucRegOffset);
            dvC341_Write(B0_CPUDT, ucRGB_LSB, ucRegOffset);
        }
        j = uiPixelHPosition;
    }
    //LOG_MSG(db_DV_SCALER, " Set DEF_ISFLD0 %x (%d,%d,%d) \r\n",ulPixelAddr,uiRed,uiGreen,uiBlue);

    //M1
    ucR_h8bit = (UINT8)((uiRed >> 2) & 0xFF);
    ucG_h8bit = (UINT8)((uiGreen >> 2) & 0xFF);
    ucB_h8bit = (UINT8)((uiBlue >> 2) & 0xFF);
    ucRGB_LSB = (UINT8)(uiRed & 0x03) + (UINT8)((uiGreen & 0x03)<<2) + (UINT8)((uiRed & 0x03)<<4);

    i = uiPixelVPosition;
    j = uiPixelHPosition;
    for( ; i< uiPixelVPosition +3 ; i++)
    {
        ulLineAddr = DEF_ISFLD1CH1_2K + (UINT32)DEF_MWI_2K *64*4* (UINT32)i;
        ulPixelAddr = ulLineAddr + (UINT32)j * 4;    //one pixel 4BYTE
        dvC341_Write(B0_CPURAD, ulPixelAddr ,ucRegOffset);
        LOG_MSG(db_DV_SCALER, " Set DEF_ISFLD1 %x (%d,%d,%d) \r\n",ulPixelAddr,uiRed,uiGreen,uiBlue);

        for( ; j< uiPixelHPosition +3 ; j++)
        {
            dvC341_Write(B0_CPUDT, ucR_h8bit, ucRegOffset);
            dvC341_Write(B0_CPUDT, ucG_h8bit, ucRegOffset);
            dvC341_Write(B0_CPUDT, ucB_h8bit, ucRegOffset);
            dvC341_Write(B0_CPUDT, ucRGB_LSB, ucRegOffset);
        }
        j = uiPixelHPosition;
    }
    //LOG_MSG(db_DV_SCALER, " Set DEF_ISFLD1 %x (%d,%d,%d) \r\n",ulPixelAddr,uiRed,uiGreen,uiBlue);

    //M2
    ucR_h8bit = (UINT8)((uiRed >> 2) & 0xFF);
    ucG_h8bit = (UINT8)((uiGreen >> 2) & 0xFF);
    ucB_h8bit = (UINT8)((uiBlue >> 2) & 0xFF);
    ucRGB_LSB = (UINT8)(uiRed & 0x03) + (UINT8)((uiGreen & 0x03)<<2) + (UINT8)((uiRed & 0x03)<<4);

    i = uiPixelVPosition;
    j = uiPixelHPosition;
    for( ; i< uiPixelVPosition +3 ; i++)
    {
        ulLineAddr = DEF_ISFLD2CH1_2K + (UINT32)DEF_MWI_2K *64*4* (UINT32)i;
        ulPixelAddr = ulLineAddr + (UINT32)j * 4;    //one pixel 4BYTE
        dvC341_Write(B0_CPURAD, ulPixelAddr ,ucRegOffset);
        LOG_MSG(db_DV_SCALER, " Set DEF_ISFLD2 %x (%d,%d,%d) \r\n",ulPixelAddr,uiRed,uiGreen,uiBlue);

        for( ; j< uiPixelHPosition +3 ; j++)
        {
            dvC341_Write(B0_CPUDT, ucR_h8bit, ucRegOffset);
            dvC341_Write(B0_CPUDT, ucG_h8bit, ucRegOffset);
            dvC341_Write(B0_CPUDT, ucB_h8bit, ucRegOffset);
            dvC341_Write(B0_CPUDT, ucRGB_LSB, ucRegOffset);
        }
        j = uiPixelHPosition;
    }
    //LOG_MSG(db_DV_SCALER, " Set DEF_ISFLD2 %x (%d,%d,%d) \r\n",ulPixelAddr,uiRed,uiGreen,uiBlue);

    //M3
    ucR_h8bit = (UINT8)((uiRed >> 2) & 0xFF);
    ucG_h8bit = (UINT8)((uiGreen >> 2) & 0xFF);
    ucB_h8bit = (UINT8)((uiBlue >> 2) & 0xFF);
    ucRGB_LSB = (UINT8)(uiRed & 0x03) + (UINT8)((uiGreen & 0x03)<<2) + (UINT8)((uiRed & 0x03)<<4);

    i = uiPixelVPosition;
    j = uiPixelHPosition;
    for( ; i< uiPixelVPosition +3 ; i++)
    {
        ulLineAddr = DEF_ISFLD3CH1_2K + (UINT32)DEF_MWI_2K *64*4* (UINT32)i;
        ulPixelAddr = ulLineAddr + (UINT32)j * 4;    //one pixel 4BYTE
        dvC341_Write(B0_CPURAD, ulPixelAddr ,ucRegOffset);
        LOG_MSG(db_DV_SCALER, " Set DEF_ISFLD3 %x (%d,%d,%d) \r\n",ulPixelAddr,uiRed,uiGreen,uiBlue);

        for( ; j< uiPixelHPosition +3 ; j++)
        {
            dvC341_Write(B0_CPUDT, ucR_h8bit, ucRegOffset);
            dvC341_Write(B0_CPUDT, ucG_h8bit, ucRegOffset);
            dvC341_Write(B0_CPUDT, ucB_h8bit, ucRegOffset);
            dvC341_Write(B0_CPUDT, ucRGB_LSB, ucRegOffset);
        }
        j = uiPixelHPosition;
    }

#endif

}

void dvC341_DDR_InputDataSet2(eC341_CH_V eCH,UINT16 uiPixelHPosition,UINT16 uiPixelVPosition,UINT16 uiRed,UINT16 uiGreen,UINT16 uiBlue)
{
#if 0  //simon check .. not implement
    UINT8  ucRegOffset = 0;
    UINT32 ulLineAddr;
    UINT32 ulPixelAddr;

    UINT8  ucR_h8bit;
    UINT8  ucG_h8bit;
    UINT8  ucB_h8bit;
    UINT8  ucRGB_LSB;

    UINT16  i,j;
    UINT8  ucVAl;


    if(eCH >= eC341_CH_VNUMBER)
    {
        return;
    }

    ucRegOffset = (eCH == eMCT_CH1) ? 0 : 13;

    switch(eCH)
    {
        case eMCT_CH1:
            dvC341_Write(BN_MCT1,  m_sChannelInfo[eCH].ucMCT & 0xEF, 0);    //set image input idle
            break;

        case eMCT_CH2:
            dvC341_Write(BN_MCT2,  m_sChannelInfo[eCH].ucMCT & 0xEF, 0);    //set image input idle
            break;
    }

    //M0
    ucR_h8bit = (UINT8)((uiRed >> 2) & 0xFF);
    ucG_h8bit = (UINT8)((uiGreen >> 2) & 0xFF);
    ucB_h8bit = (UINT8)((uiBlue >> 2) & 0xFF);
    ucRGB_LSB = (UINT8)(uiRed & 0x03) + (UINT8)((uiGreen & 0x03)<<2) + (UINT8)((uiRed & 0x03)<<4);

    i = uiPixelVPosition;
    j = uiPixelHPosition;
    for( ; i< uiPixelVPosition +1 ; i++)
    {
        ulLineAddr = DEF_ISFLD0CH1_2K + (UINT32)DEF_MWI_2K *64*4* (UINT32)i;
        ulPixelAddr = ulLineAddr + (UINT32)j * 4;    //one pixel 4BYTE
        dvC341_Write(B0_CPURAD, ulPixelAddr ,ucRegOffset);
        LOG_MSG(db_DV_SCALER, " Set DEF_ISFLD0 %x (%d,%d,%d) \r\n",ulPixelAddr,uiRed,uiGreen,uiBlue);
        for( ; j< uiPixelHPosition +1 ; j++)
        {
            dvC341_Write(B0_CPUDT, (UINT32)ucR_h8bit, ucRegOffset);
            dvC341_Write(B0_CPUDT, (UINT32)ucG_h8bit, ucRegOffset);
            dvC341_Write(B0_CPUDT, (UINT32)ucB_h8bit, ucRegOffset);
            dvC341_Write(B0_CPUDT, (UINT32)ucRGB_LSB, ucRegOffset);
        }
        j = uiPixelHPosition;
    }
    //LOG_MSG(db_DV_SCALER, " Set DEF_ISFLD0 %x (%d,%d,%d) \r\n",ulPixelAddr,uiRed,uiGreen,uiBlue);


    ulLineAddr = DEF_ISFLD0CH1_2K + (UINT32)DEF_MWI_2K *64*4* (UINT32)uiPixelVPosition;
    ulPixelAddr = ulLineAddr + (UINT32)uiPixelHPosition * 4;    //one pixel 4BYTE

    dvC341_Write(B0_CPURAD, ulPixelAddr ,ucRegOffset);
    dvC341_Write(B0_CPUDTCTL, 0x01 ,ucRegOffset);

    dvC341_ResetTimeInState();

    ucVAl = 0x08;
    while( (ucVAl & 0x08) != 0)
    {
        ucVAl = (UINT8)dvC341_Read(B0_CPUDTCTL,ucRegOffset);

        if(dvC341_TimeElapsedInState() >= 1000)
        {
            ASSERT_ALWAYS();
            return;
        }
    }

    ucR_h8bit = dvC341_Read(B0_CPUDT,ucRegOffset);
    ucG_h8bit = dvC341_Read(B0_CPUDT,ucRegOffset);
    ucB_h8bit = dvC341_Read(B0_CPUDT,ucRegOffset);
    ucRGB_LSB = dvC341_Read(B0_CPUDT,ucRegOffset);

    uiRed   = (UINT16)(ucR_h8bit << 2) + (UINT16)(ucRGB_LSB & 0x03);
    uiGreen = (UINT16)(ucG_h8bit << 2) + (UINT16)((ucRGB_LSB>>2) & 0x03);
    uiBlue  = (UINT16)(ucB_h8bit << 2) + (UINT16)((ucRGB_LSB>>4) & 0x03);

    LOG_MSG(db_DV_SCALER, " DEF_ISFLD0 %x (%d,%d,%d) \r\n",ulPixelAddr,uiRed,uiGreen,uiBlue);


    #if 0
    switch(eCH)
    {
        case eMCT_CH1:
            dvC341_Write(BN_MCT1,  m_sChannelInfo[eCH].ucMCT, 0);
            break;

        case eMCT_CH2:
            dvC341_Write(BN_MCT2,  m_sChannelInfo[eCH].ucMCT, 0);
            break;
    }
    #endif
    //dvC341_DDR_InputDataGet(eCH,uiPixelHPosition,uiPixelVPosition);
#endif
}

void dvC341_InputADC_Cali_CalibrationEnableSet(eC341_CH_V eCH,UINT8 ucADC_CAl_Enable)
{
#if 0   //unused
    if(ucADC_CAl_Enable)
    {
        UINT8 ucRegOffset = 0;
        UINT32 val = 0;	//G100_Doulas_0077 Modify

        ucRegOffset = (eCH == eMCT_CH1) ? 0 : 13;

        dvC341_Write(B0_ICFMTCH1,0x00 ,ucRegOffset);

	    if((m_sChannelSetting[eCH].eColorFormat==eCOLOR_FORMAT_RGB_LIMIT )|(m_sChannelSetting[eCH].eColorFormat==eCOLOR_FORMAT_RGB))	val=0x00;							// RGB (IMOD=0 & RGB->YUV enb)
		#ifndef ENABLE_IT6805_YUV422_YUV420_CONVERT_TO_YUV444
	    else if (m_sChannelSetting[eCH].eColorFormat==eCOLOR_FORMAT_422)							val=0x05;							//YUV422 (IMOD=1)
	    else if (m_sChannelSetting[eCH].eColorFormat==eCOLOR_FORMAT_422_UVDLY)					    val=0x05;
		#else
	    else if((m_sChannelSetting[eCH].eInputSource == eCM_SOURCE_3GSDI) &&
			    (m_sChannelInfo[eMCT_CH1].ePanelTimingId != ePANEL_ID_1080P_120HZ) &&
			    (m_sChannelInfo[eMCT_CH1].ePanelTimingId != ePANEL_ID_WUXGA_120HZ))		//A70Gen2_Doulas_0030 Modify
	    {
			val=0x05;      //A70Gen2_John_0002 fix 3GSDI in YUV444 issue (only support YUV422)                      //YUV422 (IMOD=1)
	   	}
	    else if (m_sChannelSetting[eCH].eColorFormat==eCOLOR_FORMAT_422)							val=0x06;							//YUV422 (IMOD=1)
	    else if (m_sChannelSetting[eCH].eColorFormat==eCOLOR_FORMAT_422_UVDLY)					    val=0x06;
	    #endif
	    else if (m_sChannelSetting[eCH].eColorFormat==eCOLOR_FORMAT_444)							val=0x06;		                    //YUV444 (IMOD=2)

        m_sChannelInfo[eCH].ucICFMT = val;  //A70LV_Doulas_0076 Add
	    if(eCH == 0)
        {
		    dvC341_Write(B0_ICFMTCH1,val,0);				//Input color format control
	    }
	    else
        {
		    dvC341_Write(B0_ICFMTCH2,val,0);
	    }

		dvC341_Write(B8_IBIASRCH1,0x0000,ucRegOffset);
        dvC341_Write(B8_IBIASGCH1,0x0000,ucRegOffset);
		dvC341_Write(B8_IBIASBCH1,0x0000,ucRegOffset);

	    dvC341_Write(B8_IGAINRCH1,512   ,ucRegOffset);
        dvC341_Write(B8_IGAINGCH1,512   ,ucRegOffset);
	    dvC341_Write(B8_IGAINBCH1,512   ,ucRegOffset);
    }
    else
    {
        dvC341_Set_Color(eCH);
    }
#endif
}

UINT16 dvC341_Input_H_Active_Get(const eC341_CH_V ch_v) //H30K_Doulas_0068
{
    UINT16 uiH_Active;

    if(m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.eScanMode == eSCAN_MODE_INTERLACE)
    {
        if(m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive == 1440)
            uiH_Active = m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive/2;
        else
            uiH_Active = m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive;
    }
    else
        uiH_Active = m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive;

    return uiH_Active * g_vboinfo[g_vbo_recno[m_sChannelInfo[ch_v].ucInputPort]].CH;
}

UINT16 dvC341_Input_V_Active_Get(const eC341_CH_V ch_v) //H30K_Doulas_0068
{
    UINT16 uiV_Active;
    if(m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.eScanMode == eSCAN_MODE_INTERLACE)
        uiV_Active = m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive * 2;
    else
        uiV_Active = m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive;
    return uiV_Active;
}

void dvC341_Set_Movement_NR_Flag(const eC341_CH_V ch_v,UINT8 ucVal)     //A70LV_Doulas_0145
{
    if(ucVal <= 4)
    {
        ucPM_MVFLT[ch_v] = ucVal;
    }
}

void dvC341_Set_Horizontal_NR_Flag(const eC341_CH_V ch_v,UINT8 ucVal)     //A70LV_Doulas_0145
{
    if(ucVal <= 3)
    {
        ucPM_HNR[ch_v] = ucVal;
    }
}

void dvC341_Set_Vertical_NR_Flag(const eC341_CH_V ch_v,UINT8 ucVal)     //A70LV_Doulas_0145
{
    if(ucVal <= 3)
    {
        ucPM_VNR[ch_v] = ucVal;
    }
}

void dvC341_Set_Diagonal_Interpolation_Flag(const eC341_CH_V ch_v,UINT8 ucVal)  //A70LV_Doulas_0145
{
    if(ucVal <= 3)
    {
        ucPM_DIAG[ch_v] = ucVal;
    }
}

void dvC341_Input_3D_Format_Set(UINT8 ucVal)  //A70LV_Doulas_0154
{
    ucInput3D_Format = ucVal;
}

UINT8 dvC341_Input_3D_Format_Get(void)      //A70LV_Doulas_0154
{
    return ucInput3D_Format;
}

UINT8 dvC341_Input_3D_Format_Config_Get(void)      //A70LV_Doulas_0159
{
    UINT8 uc3D_Format;
    UINT8 uc3D_Enable;
    //LOG_MSG(db_ALWAYS, "dvC341_OSD_3D_Enable  = (%d,%d) \r\n",m_sChannelSetting[eC341_CH_V0].eOSD_3D_Enable,ucInput3D_Format);  //A70LV_Doulas_0160

    //if(m_sChannelSetting[eC341_CH_V0].eOSD_3D_Mode)    //ZU860_Doulas_0049
    if(m_sChannelSetting[eC341_CH_V0].eOSD_3D_Mode != eCM_3D_MODE_OFF) //H30K_Doulas_0001
    {
        uc3D_Enable =  m_sChannelSetting[eC341_CH_V0].eOSD_3D_Enable;
    }
    else
    {
        uc3D_Enable =  eCM_3D_FORMAT_OFF;
    }

    if(m_sChannelSetting[eC341_CH_V0].cPIP_PBP_ON)     //ZU860_Doulas_0104
    {
        uc3D_Enable =  eCM_3D_FORMAT_OFF;
    }

    switch(uc3D_Enable)  //ZU860_Doulas_0049 modify
    {
        case eCM_3D_FORMAT_AUTO:
            uc3D_Format = ucInput3D_Format;
            break;

        case eCM_3D_FORMAT_FRAME_PACKING:
            if(ucInput3D_Format == eINPUT_3D_TYPE_FRAME_SEQUENTIAL)
                uc3D_Format = ucInput3D_Format;
            else
                uc3D_Format = eINPUT_3D_TYPE_FRAMEPACKING;
            break;

        case eCM_3D_FORMAT_SIDE_BY_SIDE:
            if(ucInput3D_Format == eINPUT_3D_TYPE_FRAME_SEQUENTIAL)
                uc3D_Format = ucInput3D_Format;
            else
                uc3D_Format = eINPUT_3D_TYPE_SIDEBYSIDE;
            break;

        case eCM_3D_FORMAT_TOP_AND_BOTTOM:
            if(ucInput3D_Format == eINPUT_3D_TYPE_FRAME_SEQUENTIAL)
                uc3D_Format = ucInput3D_Format;
            else
                uc3D_Format = eINPUT_3D_TYPE_TOPANDBOTTOM;
            break;

        case eCM_3D_FORMAT_FRAME_SEQUENTIAL:
            if(ucInput3D_Format == eINPUT_3D_TYPE_FRAME_SEQUENTIAL)
                uc3D_Format = ucInput3D_Format;
            else
                uc3D_Format = eINPUT_3D_TYPE_OFF;
            break;

        default:
            if(ucInput3D_Format == eINPUT_3D_TYPE_FRAME_SEQUENTIAL)
                uc3D_Format = ucInput3D_Format;
            else
                uc3D_Format = eINPUT_3D_TYPE_OFF;
            break;
    }
    return uc3D_Format;
}

//C341 wait review
void dvC341_SetInputMemoryBuffer_4K(eC341_CH_V ch_v)
{
    UINT32 ulISFLD[5];//4 Bank

    if(ch_v >= eC341_CH_VNUMBER)
    {
        return;
    }

    UINT8 ucRegOffset = 0;
    UINT8 BankOffset = CH_BANK_OFFSET;
    UINT32 val2 = DEF_MWI_2K_3D;  //H30K_Doulas_0001
    UINT32 val1 = 0; //H30K_Doulas_0002
    //ucRegOffset = (ch_v == eC341_CH_V0) ? 0 : (BankOffset * OUTPUT4K_CH_NUM); //H30K_Doulas_0001

    if(ch_v == eC341_CH_V0)
    {

        for(UINT8 ch=0 ; ch<OUTPUT4K_CH_NUM ; ch++) //H30K_Doulas_0001
        {
            ucRegOffset = ((ch_v * 2) + ch) * BankOffset;
            switch(dvC341_Input_3D_Format_Config_Get())  //simon check
            {
                case eINPUT_3D_TYPE_FRAMEPACKING:
                    if(ch == 0)
                    {
                        ulISFLD[0] = 0x00a00000;
                        ulISFLD[1] = 0x00000000;
                        ulISFLD[2] = 0x00a00000;
                        ulISFLD[3] = 0x00000000;
                        ulISFLD[4] = DEF_ISFLD0CH1_4K;
                    }
                    else
                    {
                     /*   val1 = 0x00a00000 * FRAME_MEMORY_FORMAT / 30
                        ulISFLD[0] = 0x01400000 + val1;
                        ulISFLD[1] = 0x01400000;
                        ulISFLD[2] = 0x01400000 + val1;
                        ulISFLD[3] = 0x01400000;
                    */
                        ulISFLD[0] = 0x01e00000;
                        ulISFLD[1] = 0x01400000;
                        ulISFLD[2] = 0x01e00000;
                        ulISFLD[3] = 0x01400000;
                        ulISFLD[4] = DEF_ISFLD0CH1_4K;
                    }
                    break;

                case eINPUT_3D_TYPE_SIDEBYSIDE:
                    if(ch == 0)
                    {
                        ulISFLD[0] = 0x00a00000;
                        ulISFLD[1] = 0x00000000;
                        ulISFLD[2] = 0x00a00000;
                        ulISFLD[3] = 0x00000000;
                        ulISFLD[4] = DEF_ISFLD0CH1_4K;
                    }
                    else
                    {
                        ulISFLD[0] = 0x01e00000;
                        ulISFLD[1] = 0x01400000;
                        ulISFLD[2] = 0x01e00000;
                        ulISFLD[3] = 0x01400000;
                        ulISFLD[4] = DEF_ISFLD0CH1_4K;
                    }
                    break;

                case eINPUT_3D_TYPE_TOPANDBOTTOM:
                    if(ch == 0)
                    {
                        ulISFLD[0] = 0x00a00000;
                        ulISFLD[1] = 0x00000000;
                        ulISFLD[2] = 0x00a00000;
                        ulISFLD[3] = 0x00000000;
                        ulISFLD[4] = DEF_ISFLD0CH1_4K;
                    }
                    else
                    {
                        ulISFLD[0] = 0x01e00000;
                        ulISFLD[1] = 0x01400000;
                        ulISFLD[2] = 0x01e00000;
                        ulISFLD[3] = 0x01400000;
                        ulISFLD[4] = DEF_ISFLD0CH1_4K;
                    }
                    break;

                case eINPUT_3D_TYPE_FRAME_SEQUENTIAL:
                    if((m_sChannelInfo[eC341_CH_V0].ePanelTimingId == ePANEL_ID_1080P_120HZ) ||
                       (m_sChannelInfo[eC341_CH_V0].ePanelTimingId == ePANEL_ID_WUXGA_120HZ)) //H30K_Doulas_0009
                    {
                        if(ch == 0)
                        {
                            ulISFLD[0] = 0x00000000;
                            ulISFLD[1] = 0x00a00000;
                            ulISFLD[2] = 0x00000000;
                            ulISFLD[3] = 0x00a00000;
                            ulISFLD[4] = 0;
                        }
                        else
                        {
                            if(g_vbo_recno[m_sChannelInfo[ch_v].ucInputPort] == 4) //H30K_Doulas_0029 8-lane
    	                    {
                                if(m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive == 960)
                                {
                                    val1 = m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive * 4;
                                }
                                else
                                {
                                    val1 = (m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive * 4) - 16;
                                }
    	                    }
                            else if(m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive == 1920)
                            {
                                val1 = m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive * 2;
                            }
                            else
                            {
                                val1 = (m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive * 2) - 16;
                            }
                            //val1 = m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive * 2;

    	                    val1 = val1 * FRAME_MEMORY_FORMAT / 30;  //H2PF_Simon_0097

                            #ifdef FRAME_SEQUENTAIL_SELF_TEST_120HZ //H30K_Doulas_0005
                            ulISFLD[0] = 0x00000000 + val1;
                            ulISFLD[1] = 0x00a00000 + val1;
                            ulISFLD[2] = 0x00000000 + val1;
                            ulISFLD[3] = 0x00a00000 + val1;
                            ulISFLD[4] = 0;
                            #else
                            if(g_vbo_recno[m_sChannelInfo[ch_v].ucInputPort] == 4) //H30K_Doulas_0029 8-lane
                            {
                                ulISFLD[0] = 0x00000000 + val1;
                                ulISFLD[1] = 0x00a00000 + val1;
                                ulISFLD[2] = 0x00000000 + val1;
                                ulISFLD[3] = 0x00a00000 + val1;
                                ulISFLD[4] = 0;
                            }
                            else
                            {
                                ulISFLD[0] = 0x3f000000 + val1;
                                ulISFLD[1] = 0x3f000000 + val1;
                                ulISFLD[2] = 0x3f000000 + val1;
                                ulISFLD[3] = 0x3f000000 + val1;
                                ulISFLD[4] = 0;
                            }
                            #endif
                        }
                        break;
                    }

                default:
                    if((m_sChannelInfo[eC341_CH_V0].ePanelTimingId == ePANEL_ID_1080P_120HZ) ||
                       (m_sChannelInfo[eC341_CH_V0].ePanelTimingId == ePANEL_ID_WUXGA_120HZ)) //H30K_Doulas_0002
                    {
                        if(ch == 0)
                        {
                        #if 1//H30K_Doulas_0034
                            ulISFLD[0] = 0x00000000;
                            ulISFLD[1] = 0x00a00000;
                            ulISFLD[2] = 0x01400000;
                            ulISFLD[3] = 0x01e00000;
                        #else
                            ulISFLD[0] = 0x00000000;
                            ulISFLD[1] = 0x00a00000;
                            ulISFLD[2] = 0x00000000;
                            ulISFLD[3] = 0x00a00000;
                        #endif
                            ulISFLD[4] = 0;
                        }
                        else
                        {
                        #if 1//H30K_Doulas_0034
                            UINT16	uiHActive;

                            if((g_vbo_recno[m_sChannelInfo[ch_v].ucInputPort] == 14) && // 2-lane & 2CH
                                (m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive < 1920)) //not 4K
                            {
                                uiHActive = m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive * 2;
                            }
                            else if((g_vbo_recno[m_sChannelInfo[ch_v].ucInputPort] == 4)) //H30K_Doulas_0052 240hz
                            {
                                uiHActive = m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive * 2;
                            }
                            else
                            {
                                uiHActive = m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive;
                            }

                            if(uiHActive >= m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiHActive) //H30K_Doulas_0053 m_sChannelInfo[ch_v].sOutputTimingInfo.uiHActive
                            {
                                uiHActive = m_sChannelInfo[ch_v].sScalerOutputTimingInfo.uiHActive; //H30K_Doulas_0053 m_sChannelInfo[ch_v].sOutputTimingInfo.uiHActive
                                val1 = uiHActive * 2;
                            }
                            else
                            {
                                val1 = (uiHActive * 2) ;
                            }

    	                    val1 = val1 * FRAME_MEMORY_FORMAT / 30;

                            ulISFLD[0] = 0x00000000 + val1;
                            ulISFLD[1] = 0x00a00000 + val1;
                            ulISFLD[2] = 0x01400000 + val1;
                            ulISFLD[3] = 0x01e00000 + val1;
                            ulISFLD[4] = 0;
                        #else
                            if(m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive >= 1920)
                            {
                                val1 = m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive * 2;
                            }
                            else
                            {
                                val1 = (m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive * 2) - 16;
                            }

    	                    val1 = val1 * FRAME_MEMORY_FORMAT / 30;  //H2PF_Simon_0097

                            //val1 = m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive * 2;
                            #ifdef FRAME_SEQUENTAIL_SELF_TEST_60HZ //H30K_Doulas_0005
                            ulISFLD[0] = 0x00000000 + val1;
                            ulISFLD[1] = 0x00a00000 + val1;
                            ulISFLD[2] = 0x00000000 + val1;
                            ulISFLD[3] = 0x00a00000 + val1;
                            ulISFLD[4] = 0;
                            #else
                            ulISFLD[0] = 0x3f000000 + val1;
                            ulISFLD[1] = 0x3f000000 + val1;
                            ulISFLD[2] = 0x3f000000 + val1;
                            ulISFLD[3] = 0x3f000000 + val1;
                            ulISFLD[4] = 0;
                            #endif
                        #endif
                        }
                    }
                    else
                    {
                        ulISFLD[0] = DEF_ISFLD0CH1_4K;
                        ulISFLD[1] = DEF_ISFLD1CH1_4K;
                        ulISFLD[2] = DEF_ISFLD2CH1_4K;
                        ulISFLD[3] = DEF_ISFLD3CH1_4K;
                        ulISFLD[4] = DEF_ISFLD4CH1_4K;
                        val2 = DEF_MWI_4K;
                    }
                    break;
            }
            dvC341_WriteToBuffer(B22_ISFLD0CH1, ulISFLD[0], ucRegOffset);
            dvC341_WriteToBuffer(B22_ISFLD1CH1, ulISFLD[1], ucRegOffset);
            dvC341_WriteToBuffer(B22_ISFLD2CH1, ulISFLD[2], ucRegOffset);
            dvC341_WriteToBuffer(B22_ISFLD3CH1, ulISFLD[3], ucRegOffset);
            dvC341_WriteToBuffer(B22_ISFLD4CH1, ulISFLD[4], ucRegOffset);
            dvC341_WriteToBuffer(B22_IMWICH1  , val2      , ucRegOffset);
            dvC341_Buffer_Flush();
        }
    }
    else if(ch_v == eC341_CH_V1)
    {
        for(UINT8 ch=0 ; ch<OUTPUT4K_CH_NUM ; ch++) //H30K_Doulas_0001
        {
            ucRegOffset = ((ch_v * 2) + ch) * BankOffset;
            switch(dvC341_Input_3D_Format_Config_Get())
            {
                case eINPUT_3D_TYPE_FRAMEPACKING:
                    if(ch == 0)
                    {
                        ulISFLD[0] = 0x00a00000;
                        ulISFLD[1] = 0x00000000;
                        ulISFLD[2] = 0x00a00000;
                        ulISFLD[3] = 0x00000000;
                        ulISFLD[4] = DEF_ISFLD0CH1_4K;
                    }
                    else
                    {
                        ulISFLD[0] = 0x01e00000;
                        ulISFLD[1] = 0x01400000;
                        ulISFLD[2] = 0x01e00000;
                        ulISFLD[3] = 0x01400000;
                        ulISFLD[4] = DEF_ISFLD0CH1_4K;
                    }
                    break;

                case eINPUT_3D_TYPE_SIDEBYSIDE:
                    if(ch == 0)
                    {
                        ulISFLD[0] = 0x00a00000;
                        ulISFLD[1] = 0x00000000;
                        ulISFLD[2] = 0x00a00000;
                        ulISFLD[3] = 0x00000000;
                        ulISFLD[4] = DEF_ISFLD0CH1_4K;
                    }
                    else
                    {
                        ulISFLD[0] = 0x01e00000;
                        ulISFLD[1] = 0x01400000;
                        ulISFLD[2] = 0x01e00000;
                        ulISFLD[3] = 0x01400000;
                        ulISFLD[4] = DEF_ISFLD0CH1_4K;
                    }
                    break;

                case eINPUT_3D_TYPE_TOPANDBOTTOM:
                    if(ch == 0)
                    {
                        ulISFLD[0] = 0x00a00000;
                        ulISFLD[1] = 0x00000000;
                        ulISFLD[2] = 0x00a00000;
                        ulISFLD[3] = 0x00000000;
                        ulISFLD[4] = DEF_ISFLD0CH1_4K;
                    }
                    else
                    {
                        ulISFLD[0] = 0x01e00000;
                        ulISFLD[1] = 0x01400000;
                        ulISFLD[2] = 0x01e00000;
                        ulISFLD[3] = 0x01400000;
                        ulISFLD[4] = DEF_ISFLD0CH1_4K;
                    }
                    break;

                default:
                    /*if((m_sChannelInfo[eC341_CH_V0].ePanelTimingId == ePANEL_ID_1080P_120HZ) ||
                       (m_sChannelInfo[eC341_CH_V0].ePanelTimingId == ePANEL_ID_WUXGA_120HZ)) //H30K_Doulas_0005//H30K_Doulas_0002
                    {
                        if(ch == 0)
                        {
                            ulISFLD[0] = 0x00000000;
                            ulISFLD[1] = 0x00a00000;
                            ulISFLD[2] = 0x00000000;
                            ulISFLD[3] = 0x00a00000;
                            ulISFLD[4] = 0;
                        }
                        else
                        {
                            if(m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive >= 1920)
                            {
                                val1 = m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive * 2;
                            }
                            else
                            {
                                val1 = (m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive * 2) - 16;
                            }
                            //val1 = m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive * 2;

    	                    val1 = val1 * FRAME_MEMORY_FORMAT / 30;  //H2PF_Simon_0097

                            ulISFLD[0] = 0x00000000 + val1;
                            ulISFLD[1] = 0x00a00000 + val1;
                            ulISFLD[2] = 0x00000000 + val1;
                            ulISFLD[3] = 0x00a00000 + val1;
                            ulISFLD[4] = 0;
                        }
                    }
                    else*/
                    {
                        ulISFLD[0] = DEF_ISFLD0CH2_4K;
                        ulISFLD[1] = DEF_ISFLD1CH2_4K;
                        ulISFLD[2] = DEF_ISFLD2CH2_4K;
                        ulISFLD[3] = DEF_ISFLD3CH2_4K;
                        ulISFLD[4] = DEF_ISFLD4CH2_4K;
                        val2 = DEF_MWI_4K;
                    }
                    break;
            }
            dvC341_WriteToBuffer(B22_ISFLD0CH1, ulISFLD[0], ucRegOffset);
            dvC341_WriteToBuffer(B22_ISFLD1CH1, ulISFLD[1], ucRegOffset);
            dvC341_WriteToBuffer(B22_ISFLD2CH1, ulISFLD[2], ucRegOffset);
            dvC341_WriteToBuffer(B22_ISFLD3CH1, ulISFLD[3], ucRegOffset);
            dvC341_WriteToBuffer(B22_ISFLD4CH1, ulISFLD[4], ucRegOffset);
            dvC341_WriteToBuffer(B22_IMWICH1  , val2      , ucRegOffset);
            dvC341_Buffer_Flush();
        }
    }

    /*for(UINT8 ch=0 ; ch<OUTPUT4K_CH_NUM ; ch++) //H30K_Doulas_0001 remove
    {
        dvC341_WriteToBuffer(B22_ISFLD0CH1, ulISFLD[0], ucRegOffset + (ch*BankOffset));
        dvC341_WriteToBuffer(B22_ISFLD1CH1, ulISFLD[1], ucRegOffset + (ch*BankOffset));
        dvC341_WriteToBuffer(B22_ISFLD2CH1, ulISFLD[2], ucRegOffset + (ch*BankOffset));
        dvC341_WriteToBuffer(B22_ISFLD3CH1, ulISFLD[3], ucRegOffset + (ch*BankOffset));
        dvC341_WriteToBuffer(B22_ISFLD4CH1, ulISFLD[4], ucRegOffset + (ch*BankOffset));
        dvC341_Buffer_Flush();
    }*/
}

void dvC341_OSD_3D_Enable_Set(const eC341_CH_V ch_v,UINT8 ucVal)     //A70LV_Doulas_0154
{
    m_sChannelSetting[ch_v].eOSD_3D_Enable = ucVal;
    LOG_MSG(db_DV_SCALER, "dvC341_OSD_3D_Enable_Set[%d] = %d\n",ch_v,ucVal);
}

BOOL dvC341_AspectRatioIs3DModeGet(void)            //A70LV_Doulas_0154
{
    BOOL bIs3DMode = FALSE;
    UINT8 uc3D_Enable;

    //if(m_sChannelSetting[eC341_CH_V0].eOSD_3D_Mode)    //ZU860_Doulas_0049
    if(m_sChannelSetting[eC341_CH_V0].eOSD_3D_Mode != eCM_3D_MODE_OFF) //H30K_Doulas_0001
    {
        uc3D_Enable =  m_sChannelSetting[eC341_CH_V0].eOSD_3D_Enable;
    }
    else
    {
        uc3D_Enable =  eCM_3D_FORMAT_OFF;
    }

    switch(uc3D_Enable)  //ZU860_Doulas_0049 modify
    {
        case eCM_3D_FORMAT_AUTO:
            switch(dvC341_Input_3D_Format_Get())  //A70LV_Doulas_0162 //A70LV_Doulas_0159 modify
            {
                case eINPUT_3D_TYPE_FRAMEPACKING:
                case eINPUT_3D_TYPE_SIDEBYSIDE:
                case eINPUT_3D_TYPE_TOPANDBOTTOM:
                case eINPUT_3D_TYPE_FRAME_SEQUENTIAL:
                    bIs3DMode = TRUE;
                    break;

                default:
                    break;
            }
            break;

        case eCM_3D_FORMAT_FRAME_PACKING:
        case eCM_3D_FORMAT_SIDE_BY_SIDE:
        case eCM_3D_FORMAT_TOP_AND_BOTTOM:
        case eCM_3D_FORMAT_FRAME_SEQUENTIAL:
            bIs3DMode = TRUE;
            break;

        default:
            if(dvC341_Input_3D_Format_Get() == eINPUT_3D_TYPE_FRAME_SEQUENTIAL)
            {
                bIs3DMode = TRUE;
            }
            break;
    }
    if((m_sChannelInfo[eC341_CH_V0].ePanelTimingId == ePANEL_ID_1080P_240HZ) ||
       (m_sChannelInfo[eC341_CH_V0].ePanelTimingId == ePANEL_ID_WUXGA_240HZ)) //H30K_Doulas_0009
    {
        bIs3DMode = TRUE;
    }
    return bIs3DMode;
}

void dvC341_3D_InputPort_Set(void)     //A70LV_Doulas_0154
{
#if 0   //simon check (no need?)
    switch(dvC341_Input_3D_Format_Config_Get())
    {
        case eINPUT_3D_TYPE_FRAMEPACKING:
        case eINPUT_3D_TYPE_SIDEBYSIDE:
        case eINPUT_3D_TYPE_TOPANDBOTTOM:
            m_sChannelInfo[eC341_CH_V1].ucInputPort = eC341_CH_V0;
            break;

        default:
            m_sChannelInfo[eC341_CH_V1].ucInputPort = eC341_CH_V1;
            break;
    }
    dvC341_SetInputPort_2K(eC341_CH_V1);
#endif
}

UINT32 dvC341_InputHorzPeriod_Get(const eC341_CH_V ch_v)     //A70LV_Doulas_0195
{
    return (UINT32)(m_sChannelInfo[ch_v].sInputTimingInfo.dFrameRate * (DOUBLE)m_sChannelInfo[ch_v].sInputTimingInfo.uiVTotal);
}

UINT16 dvC341_VGA_V_Total_Get(const eC341_CH_V ch_v)   //A70LV_Doulas_0195
{
    return m_sChannelInfo[ch_v].sInputTimingInfo.uiVTotal;
}

UINT8 dvC341_VGA_ModeTable_Get(const eC341_CH_V ch_v)   //A70LV_Doulas_0195
{
    return m_ucModeTableID[ch_v];
}

UINT16 dvC341_VGA_ModeTableNumber_Get(const eC341_CH_V ch_v)   //A70LV_Doulas_0195
{
    return m_BestModeTableNumber[ch_v];
}

UINT16 dvC341_ModeAdjusmenttHorzStart_Get(void)   //A70LV_Doulas_0195
{
    return uiModeAdjusmenttHorzStart;
}

UINT16 dvC341_ModeAdjusmenttVertStart_Get(void)   //A70LV_Doulas_0195
{
    return uiModeAdjusmenttVertStart;
}

void dvC341_ModeAdjusmenttHorzStart_Set(UINT16 uiHStart)   //A70LV_Doulas_0195
{
    uiModeAdjusmenttHorzStart = uiHStart;
}

void dvC341_ModeAdjusmenttVertStart_Set(UINT16 uiVStart)   //A70LV_Doulas_0195
{
    uiModeAdjusmenttVertStart = uiVStart;
}

void dvC341_ModeAdjusmenttEnableSetting(void)   //A70LV_Doulas_0195
{
#if 0
    UINT16 uiHStart,uiVStart;
    //LOG_MSG(db_DV_SCALER, "Start00 (%d,%d)  \r\n", uiHStart,uiVStart);
    if((m_sChannelSetting[eMCT_CH1].eInputSource == eCM_SOURCE_VGA) &&
       (bModeAdjustmentEnable == FALSE))
    {
        bModeAdjustmentEnable = TRUE;
        if(dvC341_AutoPositionSetting(eMCT_CH1) == TRUE)
        {
            //A70LV_Doulas_0210 modify
            if((m_sChannelInfo[eMCT_CH1].sInputTimingInfo.uiHActive == m_sChannelInfo[eMCT_CH1].sInputAutoMeasure.uiHw) ||
               ((m_sChannelInfo[eMCT_CH1].sInputTimingInfo.uiHActive+1) == m_sChannelInfo[eMCT_CH1].sInputAutoMeasure.uiHw))
            {
                uiHStart = m_sChannelInfo[eMCT_CH1].sInputAutoMeasure.uiHst;

            }
            else
            {
                uiHStart = m_sChannelInfo[eMCT_CH1].sInputTimingInfo.uiHStart;
            }

            if(m_sChannelInfo[eMCT_CH1].sInputTimingInfo.uiVActive == m_sChannelInfo[eMCT_CH1].sInputAutoMeasure.uiVw)
            {
                uiVStart = m_sChannelInfo[eMCT_CH1].sInputAutoMeasure.uiVst;
            }
            else
            {
                uiVStart = m_sChannelInfo[eMCT_CH1].sInputTimingInfo.uiVStart;
            }
            //uiHStart = m_sChannelInfo[eMCT_CH1].sInputAutoMeasure.uiHst;
            //uiVStart = m_sChannelInfo[eMCT_CH1].sInputAutoMeasure.uiVst;
            LOG_MSG(db_DV_SCALER, "Start33 (%d,%d)  \r\n", uiHStart,uiVStart);
        }
        else
        {
            uiHStart = m_sChannelInfo[eMCT_CH1].sInputTimingInfo.uiHStart;
            uiVStart = m_sChannelInfo[eMCT_CH1].sInputTimingInfo.uiVStart;
            LOG_MSG(db_DV_SCALER, "Start36 (%d,%d)  \r\n", uiHStart,uiVStart);
        }
        uiModeAdjusmenttHorzStart = uiHStart;
        uiModeAdjusmenttVertStart = uiVStart;
    }
#endif   //no need
}

void dvC341_ModeAdjusmenttDisableSetting(void)   //A70LV_Doulas_0195
{
    bModeAdjustmentEnable = FALSE;
}

void dvC341_Init_Mode_Adjustment_EEPROM_Setting(sTIMING_TABLE sTiming_Table)  //A70LV_Doulas_0195
{
    memcpy(&m_sModeAdjustmentTimingTable, &sTiming_Table, sizeof(sTIMING_TABLE));
    /*LOG_MSG(db_ALWAYS, "##(%d,%d)(%d,%d)(%d)(%d,%d)(%d,%d)(%d,%d)", m_sModeAdjustmentTimingTable.ucNewMA_Mode_Flag[0]
                                                                  , m_sModeAdjustmentTimingTable.ucNewMA_Source_ID[0]
                                                                  , m_sModeAdjustmentTimingTable.ucNewMA_Table_ID[0]
                                                                  , m_sModeAdjustmentTimingTable.uiNewMA_Mode_ID[0]
                                                                  , m_sModeAdjustmentTimingTable.uiNewMA_HPeriod[0]
                                                                  , m_sModeAdjustmentTimingTable.uiNewMA_HTotal[0]
                                                                  , m_sModeAdjustmentTimingTable.uiNewMA_VTotal[0]
                                                                  , m_sModeAdjustmentTimingTable.uiNewMA_HActive[0]
                                                                  , m_sModeAdjustmentTimingTable.uiNewMA_VActive[0]
                                                                  , m_sModeAdjustmentTimingTable.uiNewMA_HStart[0]
                                                                  , m_sModeAdjustmentTimingTable.uiNewMA_VStart[0]);*/
}

UINT8 dvC341_Input3D_Timing_Get(const eC341_CH_V ch_v)   //A70LV_Doulas_0196
{
    UINT8  uc3DTiming = e3D_Customer_Timing;

    switch(dvC341_Input_3D_Format_Config_Get())
    {
        case eINPUT_3D_TYPE_FRAMEPACKING:
            uc3DTiming = e3DFP;
            break;

        case eINPUT_3D_TYPE_SIDEBYSIDE:
            uc3DTiming = e3DSbS;
            break;

        case eINPUT_3D_TYPE_TOPANDBOTTOM:
            uc3DTiming = e3DTB;
            break;

        case eINPUT_3D_TYPE_FRAME_SEQUENTIAL:
            if(((m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive) == 1920) &&   //simon check (ori *2)
               (m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive == 1080))
            {
                uc3DTiming = e3DFS_1080P_120;
            }
            else
            {
                uc3DTiming = e3DFS_Other_120;
            }
            break;

        default:
            if((m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive == 1920) &&
                (m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive == 1080) &&
                (m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.eScanMode == eSCAN_MODE_PROGRESSIVE))
            {
                if((m_sChannelInfo[ch_v].sInputTimingInfo.dFrameRate > 49 ) &&
                   (m_sChannelInfo[ch_v].sInputTimingInfo.dFrameRate < 51 ))
                {
                    uc3DTiming = e3DFS_1080P_50;
                }
                else if((m_sChannelInfo[ch_v].sInputTimingInfo.dFrameRate > 59 ) &&
                        (m_sChannelInfo[ch_v].sInputTimingInfo.dFrameRate < 61 ))
                {
                    uc3DTiming = e3DFS_1080P_60;
                }
                else
                {
                    uc3DTiming = e3D_Customer_Timing;
                }
            }
            else if ((m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive == 1920) &&
                     (m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive == 1200) &&
                     (m_sChannelInfo[ch_v].sInterlace_InputTimingInfo.eScanMode == eSCAN_MODE_PROGRESSIVE))
            {
                if((m_sChannelInfo[ch_v].sInputTimingInfo.dFrameRate > 59 ) &&
                   (m_sChannelInfo[ch_v].sInputTimingInfo.dFrameRate < 61 ))
                {
                    uc3DTiming = e3DFS_WUXGA_60;
                }
                else
                {
                    uc3DTiming = e3D_Customer_Timing;
                }
            }
            else
            {
                uc3DTiming = e3D_Customer_Timing;
            }
            break;
    }

    return uc3DTiming;
}

void dvC341_BuildPosition_2K(eC341_CH_V ch_v)      //A70LV_Doulas_0238
{
    if(!m_sChannelInfo[ch_v].bInit)
    {
        return;
    }

    dvC341_Calc_Scaler_InputAndOutput_Window_4K(ch_v);
	dvC341_Set_ZoomShrink_4K(ch_v);
	dvC341_Set_IP(ch_v);
    dvC341_Set_flddly(ch_v);  //H2PF_Simon_0196
    dvC341_ConfigOutput_4K(ch_v);
	dvC341_Set_Flip(ch_v);   //G100_Simon_0076
}

void dvC341_BuildScaler_2K(eC341_CH_V ch_v)   //A70LV_Doulas_0284
{
    if(!m_sChannelInfo[ch_v].bInit)
    {
        return;
    }

    dvC341_Calc_Scaler_InputAndOutput_Window_4K(ch_v);
    dvC341_Set_Sharpness(ch_v);  //A70LV_Doulas_0003
    dvC341_Auto_Fill_Screen_Init();
    dvC341_SetOverlay_4K(ch_v, m_sChannelSetting[eC341_CH_V0].cPIP_PBP_ON);   //ZU860_Doulas_0017
    dvC341_Write_ch_v(ch_v, B16_IIMGCTCH1, 0x00, CH_BANK_OFFSET);
	dvC341_Set_ZoomShrink_4K(ch_v);

    //input port pass through
	dvC341_Write(B0_RHOLD, 0x01, 0);		// RTCT0-7 Hold
	dvC341_Write(B0_RTCT0, 0xFF, 0);			// [7:4]RTG 1(OUTCH3/4)=F / [3:0]RTG 0(OUTCH1/2)=F
	dvC341_Write(B0_RTCT2, 0x00, 0);			// [7:4]RTG 5(IN CH2)=F   / [3:0]RTG 4(IN CH1)=F
	dvC341_Write(B0_RTCT3, 0x00, 0);			// [7:4]RTG 7(IN CH4)=F   / [3:0]RTG 6(IN CH3)=F
	dvC341_Write(B0_RHOLD, 0x00, 0);		// RTCT0-7 Normal
    g_rtct = TRUE;
    //dvC341_set_rtct_stop();
    //dvC341_Field_Interlock_Transfer_Control_Start(ch_v);

    dvC341_Set_flddly(ch_v);  //H30K_Doulas_0031  //H2PF_Simon_0196
    dvC341_ConfigOutput_4K(ch_v);

    dvC341_set_rtct_normal();
    dvC341_wait1_povs(ch_v);
	//dvC341_Field_Interlock_Transfer_Control_End(ch_v);

    //MS_SLEEP(20);
	dvC341_Set_IP(ch_v);
    dvC341_SetInputPort_2K(ch_v);
	dvC341_Set_Flip(ch_v);  //G100_Simon_0076
    if(ch_v == eC341_CH_V1)    // fixed CH1 抖動
    {
        dvC341_Set_Flip(eC341_CH_V0);
    }
}

void dvC341_Scale_EdgeMask(UINT8 ucEdgeMask,PsINPUT_TIMING_INFO psScalerInput,PsOUTPUT_TIMING_INFO psScalerOutput)      //ZU860_Doulas_0004
{
    UINT16 wOverscanPercentage, wPixels;

    LOG_MSG(db_DV_SCALER, "EdgeMask = %d\r\n", ucEdgeMask);

    //caculate input
    wOverscanPercentage = ucEdgeMask;
    wPixels = (UINT16)(((wOverscanPercentage) * psScalerInput->uiHActive) / 100);
    psScalerInput->uiHStart += wPixels;
    psScalerInput->uiHActive -=  (2 * wPixels);

    wPixels = (UINT16)(((wOverscanPercentage) * psScalerInput->uiVActive) / 100);
    psScalerInput->uiVStart += wPixels;
    psScalerInput->uiVActive -=  (2 * wPixels);

    //caculate output
    wOverscanPercentage = ucEdgeMask;
    wPixels = (UINT16)(((wOverscanPercentage) * psScalerOutput->uiHActive) / 100);
    psScalerOutput->uiHStart += wPixels;
    psScalerOutput->uiHActive -=  (2 * wPixels);

    wPixels = (UINT16)(((wOverscanPercentage) * psScalerOutput->uiVActive) / 100);
    psScalerOutput->uiVStart += wPixels;
    psScalerOutput->uiVActive -=  (2 * wPixels);

}

eSCALER_EXEC_CODE dvC341_EdgeMask_Set(const eC341_CH_V ch_v,UINT8 ucEdgeMask)    //ZU860_Doulas_0004
{
    eSCALER_EXEC_CODE eResult = eSCALER_EXEC_CODE_PASS;

    m_sChannelSetting[ch_v].cEdgeMask = ucEdgeMask;
    return eResult;
}

void dvC341_OSD_3D_Mode_Set(const eC341_CH_V ch_v,UINT8 ucVal)     //ZU860_Doulas_0049
{
    m_sChannelSetting[ch_v].eOSD_3D_Mode = ucVal;
    //LOG_MSG(db_DV_SCALER, "dvC341_OSD_3D_Mode_Set[%d] = %d\n",eCH,ucVal);
}

BOOL dvC341_IsVsync120or100Hz_Get(const eC341_CH_V ch_v)   //ZU860_Doulas_0104
{
    UINT32 ucIMFV = 0;
    DOUBLE Vsync_Freq;

    if(ch_v == eC341_CH_V0)
    {
        ucIMFV = dvC341_Read(B168_IMFVCH1, 0);
    }
    else if(ch_v == eC341_CH_V1)
    {
        ucIMFV = dvC341_Read(B168_IMFVCH3, 0);
    }

    Vsync_Freq = (DOUBLE)(m_sChannelInfo[ch_v].ulFMCLK) / (DOUBLE)ucIMFV;

    if(((Vsync_Freq < 120.5) && (Vsync_Freq > 119.5)) ||
       ((Vsync_Freq < 100.5) && (Vsync_Freq > 99.5)) ||
       ((Vsync_Freq > 121))) //G100_Coda_0096
    {

        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

BOOL dvC341_IsAspectRation16_9_Get(void)  //ZU860_Doulas_0119
{
    UINT16 uiH_Active;
    UINT16 uiV_Active;

    uiH_Active = m_sChannelInfo[0].sInputTimingInfo.uiHActive * 2;   //simon check
    uiV_Active = m_sChannelInfo[0].sInputTimingInfo.uiVActive;

    if( ((float)uiH_Active /(float)uiV_Active) < (float)1.7)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

BOOL dvC341_IsAspectRation16_10_Get(void)    //ZU860_Doulas_0120
{
    UINT16 uiH_Active;
    UINT16 uiV_Active;

    uiH_Active = m_sChannelInfo[0].sInputTimingInfo.uiHActive * 2;   //simon check
    uiV_Active = m_sChannelInfo[0].sInputTimingInfo.uiVActive;

    if( (((float)uiH_Active /(float)uiV_Active) < (float)1.7) &&
        (((float)uiH_Active /(float)uiV_Active) > (float)1.55) )
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

void dvC341_SetInputPort_Output_Off(const eC341_CH_V ch_v)     //ZU860_Doulas_0124
{
    if(ch_v >= eC341_CH_VNUMBER)
    {
        dbmsg("ch_v %d error\r\n", ch_v);
        return;
    }

    if(!m_sChannelInfo[ch_v].bInit)
    {
        return;
    }

    m_sChannelInfo[ch_v].ucMCT &= 0xFE ;    //A35G2_CDS_Simon_0005

    //LOG_MSG(db_APP_DATAPATH, "<10-%d> %d\n", eCH, m_sChannelInfo[eCH].ucMCT );

    switch(ch_v)
    {
        case eC341_CH_V0:
            if(dvC341_Read(B0_MCT1,0) != m_sChannelInfo[ch_v].ucMCT)
                dvC341_Write(B0_MCT1,  (m_sChannelInfo[ch_v].ucMCT), 0);
            if(dvC341_Read(B0_MCT2,0) != m_sChannelInfo[ch_v].ucMCT)
                dvC341_Write(B0_MCT2,  (m_sChannelInfo[ch_v].ucMCT), 0);
            break;

        case eC341_CH_V1:
            if(dvC341_Read(B0_MCT3,0) != m_sChannelInfo[ch_v].ucMCT)
                dvC341_Write(B0_MCT3,  (m_sChannelInfo[ch_v].ucMCT), 0);
            if(dvC341_Read(B0_MCT4,0) != m_sChannelInfo[ch_v].ucMCT)
                dvC341_Write(B0_MCT4,  (m_sChannelInfo[ch_v].ucMCT), 0);
            break;

        default:
            break;
    }
}

void dvC341_SetInputPort_Output_On(const eC341_CH_V ch_v)       //A35G2_CDS_Simon_0005
{
    if(ch_v >= eC341_CH_VNUMBER)
    {
        return;
    }

    if(!m_sChannelInfo[ch_v].bInit)
    {
        return;
    }

    //LOG_MSG(db_APP_DATAPATH, "<12>%d\n", m_sChannelInfo[ch_v].ucMCT | 0x01);

    switch(ch_v)
    {
        case eC341_CH_V0:
            dvC341_Write(B0_MCT1,  (m_sChannelInfo[ch_v].ucMCT | 0x01), 0);  //simon check
            dvC341_Write(B0_MCT2,  (m_sChannelInfo[ch_v].ucMCT | 0x01), 0);
            break;

        case eC341_CH_V1:
            dvC341_Write(B0_MCT3,  (m_sChannelInfo[ch_v].ucMCT | 0x01), 0);
            dvC341_Write(B0_MCT4,  (m_sChannelInfo[ch_v].ucMCT | 0x01), 0);
            break;
    }
}

UINT8 dvC341_OutputVsyncFreqGetForTwistOn(void)   //ZU860_Doulas_0138
{
    UINT32 ucIMFV;
    DOUBLE Vsync_Freq;
    UINT8  ucOutputVsyncFrweq = 1;
    UINT8  uc3D_Format = eINPUT_3D_TYPE_OFF;

    ucIMFV = dvC341_Read(B168_IMFVCH1, 0);
    Vsync_Freq = (DOUBLE)(m_sChannelInfo[eC341_CH_V0].ulFMCLK) / (DOUBLE)ucIMFV;

    uc3D_Format = dvC341_Input_3D_Format_Config_Get();

    switch(uc3D_Format)
    {
        case eINPUT_3D_TYPE_FRAME_SEQUENTIAL:
            if((Vsync_Freq < 100.5) && (Vsync_Freq > 99.5))
            {
                ucOutputVsyncFrweq = 100;
            }
            else
            {
                ucOutputVsyncFrweq = 120;
            }
            break;

        case eINPUT_3D_TYPE_FRAMEPACKING:
        case eINPUT_3D_TYPE_TOPANDBOTTOM:
        case eINPUT_3D_TYPE_SIDEBYSIDE:
            if((Vsync_Freq < 50.5) && (Vsync_Freq > 49.5))
            {
                ucOutputVsyncFrweq = 100;
            }
            else
            {
                ucOutputVsyncFrweq = 120;
            }
            break;

        case eINPUT_3D_TYPE_OFF:
            if((m_sChannelSetting[eC341_CH_V0].eOSD_3D_Enable == eCM_3D_FORMAT_FRAME_SEQUENTIAL) &&
               (m_sChannelSetting[eC341_CH_V0].eOSD_3D_Mode != eCM_3D_MODE_OFF))  //H30K_Doulas_0001 Modify
            {
                if((Vsync_Freq < 50.5) && (Vsync_Freq > 49.5))
                {
                    ucOutputVsyncFrweq = 50;
                }
                else
                {
                    ucOutputVsyncFrweq = 60;
                }
            }
            else
            {
                ucOutputVsyncFrweq = 60;
            }
            break;

        default:
            ucOutputVsyncFrweq = 60;
            break;
    }

    return ucOutputVsyncFrweq;
}

UINT32 dvC341_Reg0_Read_Get(void) //G100_Julie_0004
{
    UINT32 ulvalue;
    UINT32 ulCount;
    UINT32 ulError = 0;

    for(ulCount = 0; ulCount < 10; ulCount++)
    {
        dvC341_Write(BN_RGBNK, ulCount, 0);

        ulvalue = dvC341_Read(BN_RGBNK, 0);

        if(ulvalue != ulCount)
        {
            ulError++;
        }
    }
    return ulError;
}

void dvC341_Cmd_Test(UINT16 *puiTtlCnt, UINT16 *puiErrCnt)  //G100_Owen_0085
{
    uOPD_DATA uOPDData = {0};
    UINT8 ucIdx, ucData, ucErrIdx = 0xFF;
    UINT16 uiTotal = 0, uiErrCnt = 0;

    sprintf(uOPDData.sINTERFACE.cOPD_Type, "SPI");
    sprintf(uOPDData.sINTERFACE.cIC_Name, "C341");
    sprintf(uOPDData.sINTERFACE.cResult, "PASS");

    for(ucIdx=0xFF; ucIdx>0; ucIdx--)   //G100_Owen_0137
    {
        dvC341_Write(B5_TESTG, ucIdx, 0);
        MS_SLEEP(5);
        ucData = (dvC341_Read(B5_TESTG, 0) & 0xFF);
        uiTotal++;
        if(ucIdx != ucData)
        {
            uiErrCnt++;
            if(uiErrCnt == 1)
            {
                sprintf(uOPDData.sINTERFACE.cResult, "FAIL");
                ucErrIdx = ucIdx;
            }
            LOG_MSG(db_DV_SCALER, "C341 W[%X] != R[%X]\r\n", ucIdx, ucData);
        }
    }
    dvC341_Write(B5_TESTG, 0, 0);   //G100_Owen_0137

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

    LOG_MSG(db_DV_SCALER, "C341 Total[%d], Error[%d]\r\n", uiTotal, uiErrCnt);
}

sCHANNEL_INFO* dvC341_Copy_Timing(eC341_CH_V ch_v)	//R70G2_Sammy_0014
{
	return  &m_sChannelInfo[ch_v];
}

void dvC341_Get_OutputHV(uint16 *pHStart, uint16 *pHActive, uint16 *pVStart, uint16 *pVActive) //R70G2_Sammy_0014
{
	ePANEL_ID ePanelId = dvC341_PanelGet();

	*pHStart = m_sPanelTable[ePanelId].sOutputTimingInfo.uiHStart;
	*pHActive = m_sPanelTable[ePanelId].sOutputTimingInfo.uiHActive;
	*pVStart = m_sPanelTable[ePanelId].sOutputTimingInfo.uiVStart;
	*pVActive = m_sPanelTable[ePanelId].sOutputTimingInfo.uiVActive;
}

#ifdef Low_Latency_All
void dvC341_Low_Latency_Set(const eC341_CH_V ch_v,UINT8 ucVal)
{
    m_sChannelSetting[ch_v].ucLow_Latency = ucVal;
    LOG_MSG(db_DV_SCALER,"#2Low_Latency[%d] %d\r\n",ch_v,m_sChannelSetting[ch_v].ucLow_Latency); //H30K_Doulas_0010
}
#endif	/*Low_Latency_All*/

void dvC341_InputImage_Idle_Set(eC341_CH_V ch_v, BOOL bEnable) //G100_Owen_0048
{
    UINT8  ucRegOffset = 0;

    if(ch_v >= eC341_CH_VNUMBER)
    {
        return;
    }

    //LOG_MSG(db_APP_DATAPATH, "<13>%d\n", m_sChannelInfo[eCH].ucMCT & 0xEF);

    if(bEnable == TRUE)
    {
        switch(ch_v)
        {
            case eC341_CH_V0:
                dvC341_Write(B0_MCT1,  m_sChannelInfo[ch_v].ucMCT & 0xEF, 0);    //set image input idle
                dvC341_Write(B0_MCT2,  m_sChannelInfo[ch_v].ucMCT & 0xEF, 0);    //set image input idle
                break;

            case eC341_CH_V1:
                dvC341_Write(B0_MCT3,  m_sChannelInfo[ch_v].ucMCT & 0xEF, 0);    //set image input idle
                dvC341_Write(B0_MCT4,  m_sChannelInfo[ch_v].ucMCT & 0xEF, 0);    //set image input idle
                break;
        }
    }
    else
    {
        switch(ch_v)
        {
            case eC341_CH_V0:
                if(dvC341_Read(B0_MCT1,0) != m_sChannelInfo[ch_v].ucMCT)
                    dvC341_Write(B0_MCT1,  m_sChannelInfo[ch_v].ucMCT, 0);
                if(dvC341_Read(B0_MCT2,0) != m_sChannelInfo[ch_v].ucMCT)
                    dvC341_Write(B0_MCT2,  m_sChannelInfo[ch_v].ucMCT, 0);
                break;

            case eC341_CH_V1:
                if(dvC341_Read(B0_MCT3,0) != m_sChannelInfo[ch_v].ucMCT)
                    dvC341_Write(B0_MCT3,  m_sChannelInfo[ch_v].ucMCT, 0);
                if(dvC341_Read(B0_MCT4,0) != m_sChannelInfo[ch_v].ucMCT)
                    dvC341_Write(B0_MCT4,  m_sChannelInfo[ch_v].ucMCT, 0);
                break;
        }
    }
}

void dvC341_RegTbl_to_Flash(UINT32 ulSFLAddr, UINT32 *pulRegTbl, UINT16 uiTblSize)   //G100_Owen_0060
{
    UINT16 uiTblIdx = 0;
    UINT8 aucData[0x100] = {0xFF};
    UINT8 ucBank, ucReg, ucSize, ucIdx;
    UINT8 ucBankChg = 0xFF;

    for(ucIdx = 0; ucIdx < uiTblSize; ucIdx++)
    {
        ucBank = (pulRegTbl[ucIdx] >> 16) & 0x3F;
        ucReg = (pulRegTbl[ucIdx] >> 8) & 0xFF;
        ucSize = pulRegTbl[ucIdx] & 0xFF;

        if(ucBankChg != ucBank)
        {
            aucData[uiTblIdx++] = 0x00;
            aucData[uiTblIdx++] = ucBank;
            ucBankChg = ucBank;
        }

        while(ucSize > 0)
        {
            aucData[uiTblIdx++] = ucReg;
            aucData[uiTblIdx++] = (UINT8)dvC341_Read((ucBank<<16 | ucReg<<8 | 1), 0);

            ucReg++;
            ucSize--;
        }
    }
    //LOG_MSG(db_DV_SCALER, "(func:%s, line:%d) uiTblIdx (%d)\r\n", __FUNCTION__, __LINE__, uiTblIdx);

    dvC341_FlashWrite(ulSFLAddr, 0x100, aucData);
}

void dvC341_CapturedIMG_Output_Set(BOOL bEnable)    //G100_Owen_0048 //A35G2_BRC_Casper_0036
{
#if 0  //simon check ... not implement
    static UINT32 sulOSFLD0_Addr = 0;   //G100_Owen_0099

    if(bEnable)
    {
        sulOSFLD0_Addr = dvC341_Read(B13_OSFLD0CH1, 0); //G100_Owen_0099
        dvC341_Write(B13_OSFLD0CH1, DEF_ISFLD0CH3_2K, 0);
        dvC341_Write(B2_OSYCTCH1, 0x0813, 0);
        dvC341_Write(BN_MCT1, 0x1, 0);  //G100_Owen_0056
    }
    else
    {
        if(sulOSFLD0_Addr != 0) //G100_Owen_0099
        {
            dvC341_Write(B13_OSFLD0CH1, sulOSFLD0_Addr, 0);
        }
        dvC341_Write(B2_OSYCTCH1, 0x0313, 0);
        dvC341_Write(B2_OIMGCTCH1, 0x01, 0);
        dvC341_Write(B2_OIMGCTCH1, 0x01, 13);   //G100_Owen_0062
    }

    bCapImgOutput = bEnable;

//    LOG_MSG(db_DV_SCALER, "B2_OSYCTCH1=0x%X\r\n", dvC341_Read(B2_OSYCTCH1,0));
//    LOG_MSG(db_DV_SCALER, "B2_OACTHSTCH1=0x%X\r\n", dvC341_Read(B2_OACTHSTCH1,0));
//    LOG_MSG(db_DV_SCALER, "B2_OACTHWCH1=0x%X\r\n", dvC341_Read(B2_OACTHWCH1,0));
//    LOG_MSG(db_DV_SCALER, "B2_OACTVSTCH1=0x%X\r\n", dvC341_Read(B2_OACTVSTCH1,0));
//    LOG_MSG(db_DV_SCALER, "B2_OACTVWCH1=0x%X\r\n", dvC341_Read(B2_OACTVWCH1,0));
//    LOG_MSG(db_DV_SCALER, "B2_OIMGCTCH1=0x%X\r\n", dvC341_Read(B2_OIMGCTCH1,0));
//    LOG_MSG(db_DV_SCALER, "B4_HZOOMCTCH1=0x%X\r\n", dvC341_Read(B4_HZOOMCTCH1,0));
//    LOG_MSG(db_DV_SCALER, "B9_HSHRNKCTCH1=0x%X\r\n", dvC341_Read(B9_HSHRNKCTCH1,0));
//    LOG_MSG(db_DV_SCALER, "B4_VZOOMCTCH1=0x%X\r\n", dvC341_Read(B4_VZOOMCTCH1,0));
//    LOG_MSG(db_DV_SCALER, "B9_VSHRNKCTCH1=0x%X\r\n", dvC341_Read(B9_VSHRNKCTCH1,0));

#endif

    LOG_MSG(db_DV_SCALER, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
}

void dvC341_CapturedIMG_Reset(UINT32 ulSFLAddr, UINT32 *pulRegTbl, UINT16 uiTblSize)    //G100_Owen_0060 //G100_Owen_0059
{
#if 0  //simon check ... not implement
    UINT16 uiTblIdx = 0;
    UINT8 aucData[0x100] = {0xFF};
    UINT8 ucBank, ucReg, ucSize, ucIdx;
    UINT8 ucBankChg = 0xFF; //G100_Owen_0065

    for(ucIdx = 0; ucIdx < uiTblSize; ucIdx++)
    {
        ucBank = (pulRegTbl[ucIdx] >> 16) & 0x3F;
        ucReg = (pulRegTbl[ucIdx] >> 8) & 0xFF;
        ucSize = pulRegTbl[ucIdx] & 0xFF;

        if(ucBankChg != ucBank)
        {
            aucData[uiTblIdx++] = 0x00;
            aucData[uiTblIdx++] = ucBank;
            ucBankChg = ucBank;
        }

        while(ucSize > 0)
        {
            aucData[uiTblIdx++] = ucReg;
            if(ucReg == ((B0_OCFMTCH1>> 8) & 0xFF))
            {
                aucData[uiTblIdx++] = 0x11;
            }
            else if(ucReg == ((B2_OIMGCTCH1>> 8) & 0xFF))
            {
                aucData[uiTblIdx++] = 1;
            }
            else
            {
                aucData[uiTblIdx++] = 0;
            }

            ucReg++;
            ucSize--;
        }
    }
    dvC341_FlashWrite(ulSFLAddr, 0x100, aucData);
#endif

    LOG_MSG(db_DV_SCALER, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
}

void dvC341_PIPPBPPanelInfo_Setting(const eC341_CH_V ch_v, sSCALER_PANEL_INFO *psPanelInfo) //A35G2_CDS_Larry_0008
{
    if(ch_v < eC341_CH_VNUMBER)
    {
        m_sC341PanelInfo[ch_v].uiHActive = psPanelInfo->uiHActive;
        m_sC341PanelInfo[ch_v].uiVActive = psPanelInfo->uiVActive;
        m_sC341PanelInfo[ch_v].uiHStart = psPanelInfo->uiHStart + m_sPanelTable[PANEL_2D_OUTPUT].sOutputTimingInfo.uiHStart;  //A35G2_CDS_Simon_0017
        m_sC341PanelInfo[ch_v].uiVStart = psPanelInfo->uiVStart + m_sPanelTable[PANEL_2D_OUTPUT].sOutputTimingInfo.uiVStart;
    }
}

void dvC341_PIPPBPPanelInfo_Get(const eC341_CH_V ch_v, sSCALER_PANEL_INFO *psPanelInfo)
{
    if(ch_v < eC341_CH_VNUMBER)
    {
        psPanelInfo->uiHActive = m_sC341PanelInfo[ch_v].uiHActive;
        psPanelInfo->uiVActive = m_sC341PanelInfo[ch_v].uiVActive;
        psPanelInfo->uiHStart  = m_sC341PanelInfo[ch_v].uiHStart;
        psPanelInfo->uiVStart  = m_sC341PanelInfo[ch_v].uiVStart;
    }
}

UINT32 dvC341_Mclko_Freq_Get(void)
{
    return m_sChannelInfo[eC341_CH_V0].ulMclko_Freq;
}

BOOL dvC341_SpecialTiming_Check_1280_960_85(const eC341_CH_V ch_v, UINT32 *ulValue)
{
	if((m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive==1280) &&   //simon check
		(m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive == 960) &&
		(m_sChannelInfo[ch_v].sInputTimingInfo.dFrameRate >= 82.5 && m_sChannelInfo[ch_v].sInputTimingInfo.dFrameRate <= 87.5))
	{
         *ulValue = (((*ulValue + (2 * IMFH_MCLK_TOLERANCE)) & 0x09FF) | 0x0900) - (2 * IMFH_MCLK_TOLERANCE);
         //LOG_MSG(db_DV_SCALER, "(dvC341_SpecialTiming_Check_1280_960_85) : %x\r\n",*ulValue);
		 return TRUE;

	}
	else
	{
		 return FALSE;
	}
}

void dvC341_WaitOneVsyncTime(eC341_CH_V ch_v) //A35G2_Simon_0113 //A35G2_BRC_Casper_0146
{
    dvC341_wait1_pivs(ch_v);
}


void dvC341_InputPort_Set()
{
    if((m_sChannelSetting[eC341_CH_V1].eInputSource == eCM_SOURCE_HDBASET)
    || ((m_sChannelSetting[eC341_CH_V0].eInputSource == eCM_SOURCE_HDMI1) && (m_sChannelSetting[eC341_CH_V1].eInputSource == eCM_SOURCE_HDMI2))
    || ((m_sChannelSetting[eC341_CH_V0].eInputSource == eCM_SOURCE_HDMI1) && (m_sChannelSetting[eC341_CH_V1].eInputSource == eCM_SOURCE_DISPLAYPORT))
    || ((m_sChannelSetting[eC341_CH_V0].eInputSource == eCM_SOURCE_HDMI2) && (m_sChannelSetting[eC341_CH_V1].eInputSource == eCM_SOURCE_DISPLAYPORT))
    || (m_sChannelSetting[eC341_CH_V0].eInputSource == eCM_SOURCE_12GSDI))
    {
        m_sChannelInfo[eC341_CH_V0].ucInputPort = eC341_INPUT_PORT3;
        m_sChannelInfo[eC341_CH_V1].ucInputPort = eC341_INPUT_PORT2;
    }
    else
    {
        m_sChannelInfo[eC341_CH_V0].ucInputPort = eC341_INPUT_PORT2;
        m_sChannelInfo[eC341_CH_V1].ucInputPort = eC341_INPUT_PORT3;
    }
    LOG_MSG(db_DV_SCALER, "##999##(%x,%x)(%x,%x)\r\n", m_sChannelSetting[eC341_CH_V0].eInputSource, m_sChannelInfo[eC341_CH_V0].ucInputPort, m_sChannelSetting[eC341_CH_V1].eInputSource, m_sChannelInfo[eC341_CH_V1].ucInputPort);
    return;
}

void dvC341_InputPort_Show(void)     //R70G2_Doulas_0002
{
	LOG_MSG(db_ALWAYS, "Main = (%d,%d)\r\n",m_sChannelSetting[eC341_CH_V0].eInputSource,m_sChannelInfo[eC341_CH_V0].ucInputPort);
	LOG_MSG(db_ALWAYS, "Sub  = (%d,%d)\r\n",m_sChannelSetting[eC341_CH_V1].eInputSource,m_sChannelInfo[eC341_CH_V1].ucInputPort);
}



void dvC341_PowerNormalDone(BOOL bDone) //R70G2_Donview_Sammy_0040
{
    bPowerNormalDone = bDone;
}

UINT32 dvC341_CurrentBankGet(void)
{
    sPROCESS_MUTEX_DATA *sProcessMutexData = (sPROCESS_MUTEX_DATA *)SharedMem_GetMapPtr(eSB_PROCESS_MUTEX_DATA);

    return sProcessMutexData->Scaler_CurrentBank;
}

void dvC341_CurrentBankSet(UINT32 CurrentBank)
{
    sPROCESS_MUTEX_DATA *sProcessMutexData = (sPROCESS_MUTEX_DATA *)SharedMem_GetMapPtr(eSB_PROCESS_MUTEX_DATA);

    sProcessMutexData->Scaler_CurrentBank = (UINT32)CurrentBank;
}


////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////

//get_VBOInfo
INT32 dvC341_Get_VBOInfo(INT32 port, INT32 vtotal, INT32 htotal, eC341_CH_V ch_v) //H30K_Doulas_0011
{
    int lanenum;
    int htotalmin;
    int vtotalnew;
    float magic_num;
    float hratio;
    int recno;

    magic_num = 1.38F;

    // interlace
    if (((vtotal >= 560) & (vtotal <= 565)) |
        ((vtotal >= 266) & (vtotal <= 350)) |
        ((vtotal >= 260) & (vtotal <= 265)))
    {
        vtotalnew = vtotal * 2;
    }
    else
    {
        vtotalnew = vtotal;
    }

    //if(ucInput3D_Format == eINPUT_3D_TYPE_FRAMEPACKING) //H30K_Doulas_0001 for frame packing measure
    //{
    //    vtotalnew = vtotal/2;   //H30K_Doulas_0011
    //}

    htotalmin = vtotalnew * magic_num;

    hratio = (float)htotalmin / htotal;

    lanenum = (hratio < 1) ? 1
            : (hratio < 2) ? 2
            : (hratio < 4) ? 4
            : (hratio < 8) ? 8
            : 16;

    recno = 0;
    if (lanenum == 8)
    {
        recno = (vtotal < 1350)           ? 4  // 8parax1 2K240
                          : (PS_VBO_8LANE[port] == 0) ? 10 // 8parax1
                          : (PS_VBO_8LANE[port] == 1) ? 11 // 4parax2 LR
                          : (PS_VBO_8LANE[port] == 2) ? 12 // 2parax4 strpe
                          : 10; // 8parax1 default
    }
    else if (lanenum == 4)
    {
        recno = (vtotal < 1350)           ? 3  // 4parax1 2k120
                          : (PS_VBO_4LANE[port] == 0) ? 14 // 4parax1 4k30
                          : (PS_VBO_4LANE[port] == 1) ? 15 // 2parax2 4k30
                          : 14; // 4parax1 4k30 default
    }
    else if (lanenum == 2)
    {
        recno = (PS_VBO_2LANE[port] == 0) ? 2  // 2parax1
                          : (PS_VBO_2LANE[port] == 1) ? 13 // 2parax4 quad
                          : 2; // 2parax1 default
    }
    else if (lanenum == 1)
    {
        recno = 1; // straight
    }
    else if (lanenum == 16)
    {
        recno = 16; // 4parax4 4k120
    }

    if((m_FrontEnd_Timing[ch_v].u16VideoVRate > 95 && m_FrontEnd_Timing[ch_v].u16VideoVRate < 125) &&
       (m_FrontEnd_Timing[ch_v].u32VideoPCLK > 160000 && m_FrontEnd_Timing[ch_v].u32VideoPCLK <= 320000) &&
       (m_FrontEnd_Timing[ch_v].u16VideoHActive < 2000 ) &&
       (m_FrontEnd_Timing[ch_v].u16VideoVActive < 1300 ) )  //H30K_Doulas_0011  //2k120hz
    {
        recno = 3;
    }
    else if((m_FrontEnd_Timing[ch_v].u16VideoVRate > 230 && m_FrontEnd_Timing[ch_v].u16VideoVRate < 250) &&    //H2PF_Simon_0140
            (m_FrontEnd_Timing[ch_v].u32VideoPCLK > 320000 ) &&
            (m_FrontEnd_Timing[ch_v].u16VideoHActive < 2000 ) &&
            (m_FrontEnd_Timing[ch_v].u16VideoVActive < 1300 ) )  //2k240hz
    {
        recno = 4;
    }
    else if((m_FrontEnd_Timing[ch_v].u16VideoHActive == 1920 ) &&
            (m_FrontEnd_Timing[ch_v].u16VideoVActive == 2205 ) &&
            (m_FrontEnd_Timing[ch_v].u32VideoPCLK < 160000) &&
            (m_FrontEnd_Timing[ch_v].u16VideoVRate > 21 && m_FrontEnd_Timing[ch_v].u16VideoVRate <= 25)) //1080P24 FP
    {
        recno = 2;
    }
    else if((m_FrontEnd_Timing[ch_v].u16VideoHActive == 1280 ) &&
            (m_FrontEnd_Timing[ch_v].u16VideoVActive == 1470 ) &&
            (m_FrontEnd_Timing[ch_v].u32VideoPCLK < 160000) &&
            (m_FrontEnd_Timing[ch_v].u16VideoVRate > 57 && m_FrontEnd_Timing[ch_v].u16VideoVRate < 63)) //720P60 FP
    {
        recno = 2;
    }
    else if((m_FrontEnd_Timing[ch_v].u16VideoHActive == 1280 ) &&
            (m_FrontEnd_Timing[ch_v].u16VideoVActive == 1470 ) &&
            (m_FrontEnd_Timing[ch_v].u32VideoPCLK < 160000) &&
            (m_FrontEnd_Timing[ch_v].u16VideoVRate > 47 && m_FrontEnd_Timing[ch_v].u16VideoVRate < 53)) //720P50 FP
    {
        recno = 2;
    }
    else if((m_FrontEnd_Timing[ch_v].u16VideoHActive == 2560 ) &&
            (m_FrontEnd_Timing[ch_v].u16VideoVActive == 1080 ) &&
            (m_FrontEnd_Timing[ch_v].u32VideoPCLK > 160000) &&
            (m_FrontEnd_Timing[ch_v].u16VideoVRate > 47 && m_FrontEnd_Timing[ch_v].u16VideoVRate < 63)) //2560x1080P 50/60 //H30K_Doulas_0050
    {
        recno = 3;
    }
    else if((m_FrontEnd_Timing[ch_v].u16VideoHActive == 2560 ) &&
            (m_FrontEnd_Timing[ch_v].u16VideoVActive == 1080 ) &&
            (m_FrontEnd_Timing[ch_v].u32VideoPCLK < 160000) &&
            (m_FrontEnd_Timing[ch_v].u16VideoVRate > 21 && m_FrontEnd_Timing[ch_v].u16VideoVRate < 33)) //2560x1080P 24/25/30 //H30K_Doulas_0050
    {
        recno = 2;
    }

    return(recno);
}


void dvC341_set_VBOInput(eC341_CH_V ch_v, INT32 port)
{
    unsigned int reg;
    int port_offset;

    if (port < 2)
        return;

    port_offset = port - 2;

//#ifdef ONLY_TEST_FOR_C341
//g_vbo_recno[port] = 2;
//#endif

    LOG_MSG(db_DV_SCALER, "\n\ndvC341_set_VBOInput\n");
    LOG_MSG(db_DV_SCALER, "ch_v = %d\n", ch_v);
    LOG_MSG(db_DV_SCALER, "port = %d\n", port);
    LOG_MSG(db_DV_SCALER, "g_vbo_recno[%d] = %d\n", port, g_vbo_recno[port]);
    LOG_MSG(db_DV_SCALER, "g_vboinfo[g_vbo_recno[port]].CH = %d\n", g_vboinfo[g_vbo_recno[port]].CH);

	//g_vbo_recno[ucInputPort] = dvC341_Get_VBOInfo(ucInputPort, m_sChannelInfo[ch_v].sPortInfo.uiVTotal, m_sChannelInfo[ch_v].sPortInfo.uiHTotal);

    if (g_vboinfo[g_vbo_recno[port]].CH < 3)
    {
        if (g_vboinfo[g_vbo_recno[port]].ICHSEL1 != -1)
        {
            LOG_MSG(db_DV_SCALER, "g_vboinfo[g_vbo_recno[port]].ICHSEL1 = %d\n", g_vboinfo[g_vbo_recno[port]].ICHSEL1);
            reg = g_vboinfo[g_vbo_recno[port]].ICHSEL1 + 0x04*port_offset;
            dvC341_Write(B0_ICHSEL1 + 0x200*ch_v, reg, 0);
        }

        if (g_vboinfo[g_vbo_recno[port]].ICHSEL2 != -1)
        {
            LOG_MSG(db_DV_SCALER, "g_vboinfo[g_vbo_recno[port]].ICHSEL2 = %d\n", g_vboinfo[g_vbo_recno[port]].ICHSEL2);
            reg = g_vboinfo[g_vbo_recno[port]].ICHSEL2 + 0x04*port_offset;
            dvC341_Write(B0_ICHSEL2 + 0x200*ch_v, reg, 0);
        }
    }
    else
    {
        if (port_offset == 0)
        {
            if (g_vboinfo[g_vbo_recno[port]].ICHSEL1 != -1)
                dvC341_Write(B0_ICHSEL1 + 0x200*ch_v, g_vboinfo[g_vbo_recno[port]].ICHSEL1, 0);

            if (g_vboinfo[g_vbo_recno[port]].ICHSEL2 != -1)
                dvC341_Write(B0_ICHSEL2 + 0x200*ch_v, g_vboinfo[g_vbo_recno[port]].ICHSEL2, 0);
        }
        else
        {
            if (g_vboinfo[g_vbo_recno[port]].ICHSEL3 != -1)
                dvC341_Write(B0_ICHSEL1 + 0x200*ch_v, g_vboinfo[g_vbo_recno[port]].ICHSEL3, 0);

            if (g_vboinfo[g_vbo_recno[port]].ICHSEL4 != -1)
                dvC341_Write(B0_ICHSEL2 + 0x200*ch_v, g_vboinfo[g_vbo_recno[port]].ICHSEL4, 0);
        }
    }

#ifdef SWAP_VBO_RX0_LANE
    if(port_offset == 0)
    {
        dvC341_Write(B0_VI1CT + 0x200*port_offset, g_vboinfo[g_vbo_recno[port]].VICT | 0xC8C8, 0);  //swap Vx1 lane
    }
    else
    {
        dvC341_Write(B0_VI1CT + 0x200*port_offset, g_vboinfo[g_vbo_recno[port]].VICT , 0);
    }
#else

    dvC341_Write(B0_VI1CT + 0x200*port_offset, g_vboinfo[g_vbo_recno[port]].VICT , 0);

#endif

    reg = dvC341_Read(B0_VIDSKW, 0);
    reg = reg & (0xff00 >> 8*port_offset) | (g_vboinfo[g_vbo_recno[port]].VIDSKW << 8*port_offset);
    dvC341_Write(B0_VIDSKW, reg, 0);

    LOG_MSG(db_DV_SCALER, "B0_ICHSEL1 = 0x%X\n", dvC341_Read(B0_ICHSEL1,0));
    LOG_MSG(db_DV_SCALER, "B0_ICHSEL2 = 0x%X\n", dvC341_Read(B0_ICHSEL2,0));
    LOG_MSG(db_DV_SCALER, "B0_ICHSEL3 = 0x%X\n", dvC341_Read(B0_ICHSEL3,0));
    LOG_MSG(db_DV_SCALER, "B0_ICHSEL4 = 0x%X\n", dvC341_Read(B0_ICHSEL4,0));

    dvC341_wait1_pivs(ch_v);
    dvC341_wait1_pivs(ch_v);
}



INT8 dvC341_wait1_pivs(eC341_CH_V ch_v)
{
	int		int_stat;
	int		imfh_msb;
	int		int_bit;
	unsigned int imfh_ad;
	INT8 cRet = TRUE;

	if(ch_v == eC341_CH_V0)
	{
		int_bit = 0x01;// PIVSCH1
		imfh_ad = B168_IMFHCH1;
	}
	else if(ch_v == eC341_CH_V1)
	{
		int_bit = 0x04;// PIVSCH3
		imfh_ad = B168_IMFHCH3;
	}
	else
	{
	    cRet = FALSE;
		return cRet;
	}

	imfh_ad = imfh_ad & 0xffff00;
	imfh_ad = imfh_ad | 0x000001;// 1byte
	imfh_ad = imfh_ad + 0x000100;// IMFH[15:8]

	dvC341_Write(B5_INTEN_0, (int_bit), 0);

    dvC341_ResetTimeInState();

	int_stat  = dvC341_Read(B5_INTSTAT_0, 0) & int_bit;
	imfh_msb = dvC341_Read(imfh_ad, 0) & 0xff;
	while((int_stat != int_bit) & (imfh_msb != 0xff))
	{
	    MS_SLEEP(1);
		int_stat  = dvC341_Read(B5_INTSTAT_0, 0) & int_bit;
		imfh_msb = dvC341_Read(imfh_ad, 0) & 0xff;

		if(dvC341_TimeElapsedInState() >= DEF_WAIT_TIMEOUT)
        {
            LOG_MSG(db_DV_SCALER, "---(func:%s, line:%d) timeout, int_stat[0x%X] imfh_msb[0x%X]\n", __FUNCTION__, __LINE__, int_stat, imfh_msb);
            cRet = FALSE;
            break;
        }
	};

	dvC341_Write(B5_INTEN_0, 0x00, 0);

	if (imfh_msb == 0xff)
	{
	    cRet = FALSE;
		LOG_MSG(db_DV_SCALER, "(func:%s, line:%d) CH%d : No input\n", __FUNCTION__, __LINE__, ch_v);
	}

    return cRet;
}

INT8 dvC341_wait1_povs(eC341_CH_V ch_v)
{
	int		int_stat;
	int		int_bit;
	INT8 cRet = TRUE;

	if (ch_v == eC341_CH_V0)
	{
		int_bit = 0x01;// POVSCH1
	}
	else if (ch_v == eC341_CH_V1)
	{
		int_bit = 0x04;// POVSCH3
	}
	else
	{
		return FALSE;
	}

	dvC341_Write(B5_INTEN_1, int_bit, 0);

    dvC341_ResetTimeInState();

	int_stat = dvC341_Read(B5_INTSTAT_1, 0) & int_bit;
	while (int_stat != int_bit)
	{
	    MS_SLEEP(3);
		int_stat = dvC341_Read(B5_INTSTAT_1, 0) & int_bit;

		if(dvC341_TimeElapsedInState() >= DEF_WAIT_TIMEOUT)
        {
            LOG_MSG(db_DV_SCALER, "---(func:%s, line:%d) timeout, int_stat[0x%X] int_bit[0x%X]\n", __FUNCTION__, __LINE__, int_stat, int_bit);
            cRet = FALSE;
            break;
        }
	}

	dvC341_Write(B5_INTEN_1, 0x00, 0);

	return cRet;
}

INT8 dvC341_wait1_povs_allch(void)
{
	int		int_stat;
	int		int_bit;
	INT8 cRet = TRUE;

	int_bit = 0x0f;

	dvC341_Write(B5_INTEN_1, int_bit, 0);

    dvC341_ResetTimeInState();

	int_stat = dvC341_Read(B5_INTSTAT_1, 0) & int_bit;
	while (int_stat != int_bit)
	{
	    MS_SLEEP(3);
		int_stat = dvC341_Read(B5_INTSTAT_1, 0) & int_bit;

		if(dvC341_TimeElapsedInState() >= DEF_WAIT_TIMEOUT)
        {
            LOG_MSG(db_DV_SCALER, "---(func:%s, line:%d) timeout, int_stat[0x%X] int_bit[0x%X]\n", __FUNCTION__, __LINE__, int_stat, int_bit);
            cRet = FALSE;
            break;
        }
	}

	dvC341_Write(B5_INTEN_1, 0x00, 0);

	return cRet;
}


static void reset_sub(int rx_num, int reg_lane, int reg_frange)
{
	if (rx_num == 0)
	{
		dvC341_Write(B2_VBOCT, 0x0e, 0);
		dvC341_Write(B2_VBOCT, 0x0f, 0);
		dvC341_Write(B2_VBORX1_FMT, 0x05, 0);
		dvC341_Write(B2_VBORX1_LANECT, reg_lane & 0xff, 0);
		dvC341_Write(B2_VBORX1_OPT1, reg_frange & 0xff, 0);
	}
	else
	{
		dvC341_Write(B2_VBOCT, 0x0d, 0);
		dvC341_Write(B2_VBOCT, 0x0f, 0);
		dvC341_Write(B2_VBORX2_FMT, 0x05, 0);
		dvC341_Write(B2_VBORX2_LANECT, reg_lane & 0xff, 0);
		dvC341_Write(B2_VBORX2_OPT1, reg_frange & 0xff, 0);
	}
}


void dvC341_reset_vborx(int rx_num, int reg_lane, float fpclk, int reset)
{
	int vborx_opt1, vbystate, lock;
    int check_cnt[2] = {0, 0};

	if (reset == 1)
	{
		if (check_cnt[rx_num] <= 0)
		{
			vbystate = dvC341_Read(B178_VBYSTATE, 0);
			lock = vbystate & (0x20 << 2*rx_num);
			vborx_opt1 = dvC341_Read(B2_VBORX1_OPT1 + 0x1c00*rx_num, 0);

			if (lock > 0)
			{
				vborx_opt1 = ((vborx_opt1 & 0x08) > 0) ? vborx_opt1 & 0xf7
						   : vborx_opt1 | 0x08;
			}
//printf(">>>>>reset_vborx  rx=%d  reg_lane=%02x  vborx_opt1=%02x  reset=%d\n", rx_num, reg_lane, vborx_opt1, reset);
			reset_sub(rx_num, reg_lane, vborx_opt1);

			check_cnt[rx_num] = 10;
			//return;
		}
		else
		{
			check_cnt[rx_num]--;
			return;
		}
	}
	else
	{
		vborx_opt1 = dvC341_Read(B2_VBORX1_OPT1 + 0x1c00*rx_num, 0);
		vborx_opt1 = (fpclk < FRANGE_DIV) ? vborx_opt1 | 0x08
				   : vborx_opt1 & 0xf7;
//printf("<<<<< reset_vborxrx=%d  reg_lane=%02x  vborx_opt1=%02x  reset=%d\n", rx_num, reg_lane, vborx_opt1, reset);
		reset_sub(rx_num, reg_lane, vborx_opt1);
	}

	MS_SLEEP(90);
}


void dvC341_set_sypol(int kind, int port)
{
    unsigned int sypol, sypol1, sypol2;

    if (kind == 0) // CMOS
    {
        dvC341_Write(B0_RHOLD, 0x80, 0);
        sypol1 = dvC341_Read(B174_PI1SYPOL + 0x200*port, 0);
        sypol2 = dvC341_Read(B174_PI2SYPOL + 0x200*port, 0);
        sypol  = dvC341_Read(B6_PISYPOLCTRL, 0);
        dvC341_Write(B0_RHOLD, 0x00, 0);

        sypol = (port == 0) ? sypol & 0xffff0000 | sypol1 | (sypol2 << 8)
              : sypol & 0x0000ffff | (sypol1 << 16) | (sypol2 << 24);
        dvC341_Write(B6_PISYPOLCTRL, sypol, 0);

    }
    else // VBO
    {
        dvC341_Write(B0_RHOLD, 0x80, 0);
        sypol1 = dvC341_Read(B174_VI113SYPOL + 0x200*port, 0);
        sypol2 = dvC341_Read(B174_VI157SYPOL + 0x200*port, 0);
        sypol  = dvC341_Read(B6_VISYPOLCTRL, 0);
        dvC341_Write(B0_RHOLD, 0x00, 0);

        sypol = (port == 0) ? sypol & 0xffff0000 | sypol1 | (sypol2 << 8)
              : sypol & 0x0000ffff | (sypol1 << 16) | (sypol2 << 24);
        dvC341_Write(B6_VISYPOLCTRL, sypol, 0);
    }

}


void dvC341_set_idual(int ch_v, int port, int reset)
{
	if (reset == 1) // 2k1ch
	{
		dvC341_Write(B3_IDUALCTCH12 + 0x900*ch_v, 0x00, 0);

		dvC341_Write(B16_IVLDHSDLYCH1, 0x00, ch_v*CH_V_BANK_OFFSET);  //H2PF_Simon_0140
		dvC341_Write(B16_IVLDHSDLYCH1, 0x00, ch_v*CH_V_BANK_OFFSET + CH_BANK_OFFSET);

		dvC341_wait1_pivs(ch_v);
		dvC341_wait1_pivs(ch_v);
	}
	else if (m_sChannelInfo[ch_v].sInputTimingInfo.lSTATE == 1) // 4k2ch
	{
		if ((port == 0) | (port == 1)) // CMOS
		{
			dvC341_Write(B3_IDUALCTCH12 + 0x900*ch_v, 0x03, 0);
		}
		else  // VBO
		{
			dvC341_Write(B3_IDUALCTCH12 + 0x900*ch_v, g_vboinfo[g_vbo_recno[port]].IDUAL, 0);

            #if (SCALER_HDR_ENABLE == TRUE)    //H2PF_Simon_0193
            if(HDR_Demo_SplitScreenMode_Get() == TRUE)
            {
			    dvC341_Write(B3_IDUALCTCH12 + 0x900*ch_v, 0x03, 0);
            }
			#endif

            if(g_vboinfo[g_vbo_recno[port]].IDUAL == 0)
            {
        		dvC341_Write(B16_IVLDHSDLYCH1, 0x00, ch_v*CH_V_BANK_OFFSET);  //H2PF_Simon_0140
        		dvC341_Write(B16_IVLDHSDLYCH1, 0x00, ch_v*CH_V_BANK_OFFSET + CH_BANK_OFFSET);
            }
            else
            {
        		dvC341_Write(B16_IVLDHSDLYCH1, 0x01, ch_v*CH_V_BANK_OFFSET);  //H2PF_Simon_0140
        		dvC341_Write(B16_IVLDHSDLYCH1, 0x01, ch_v*CH_V_BANK_OFFSET + CH_BANK_OFFSET);
            }
		}

		dvC341_Write(B3_IDACTHSTCH12 + 0x900*ch_v, m_sChannelInfo[ch_v].sInputTimingInfo.uiHStart     , 0);
		dvC341_Write(B3_IDACTHWCH12  + 0x900*ch_v, m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive * 2, 0);

	}
	else if (m_sChannelInfo[ch_v].sInputTimingInfo.lSTATE == 2) // 4k4ch
	{
		if ((port == 0) | (port == 1)) // CMOS
		{
			dvC341_Write(B3_IDUALCTCH12 + 0x900*ch_v, 0x0a, 0);
		}
		else  // VBO
		{
			dvC341_Write(B3_IDUALCTCH12 + 0x900*ch_v, g_vboinfo[g_vbo_recno[port]].IDUAL, 0);

            #if (SCALER_HDR_ENABLE == TRUE)    //H2PF_Simon_0193
            if(HDR_Demo_SplitScreenMode_Get() == TRUE)
            {
			    dvC341_Write(B3_IDUALCTCH12 + 0x900*ch_v, 0x03, 0);
            }
			#endif

            if(g_vboinfo[g_vbo_recno[port]].IDUAL == 0)
            {
        		dvC341_Write(B16_IVLDHSDLYCH1, 0x00, ch_v*CH_V_BANK_OFFSET);  //H2PF_Simon_0140
        		dvC341_Write(B16_IVLDHSDLYCH1, 0x00, ch_v*CH_V_BANK_OFFSET + CH_BANK_OFFSET);
            }
            else
            {
        		dvC341_Write(B16_IVLDHSDLYCH1, 0x01, ch_v*CH_V_BANK_OFFSET);  //H2PF_Simon_0140
        		dvC341_Write(B16_IVLDHSDLYCH1, 0x01, ch_v*CH_V_BANK_OFFSET + CH_BANK_OFFSET);
            }
		}

		dvC341_Write(B3_IDACTHSTCH12 + 0x900*ch_v, m_sChannelInfo[ch_v].sInputTimingInfo.uiHStart     , 0);
		dvC341_Write(B3_IDACTHWCH12  + 0x900*ch_v, m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive * 4, 0);

	}
    //LOG_MSG(db_ALWAYS,"ch (%d,%d)(%d)(%d,%d) \r\n",ch_v,port,reset,
    //m_sChannelInfo[ch_v].sInputTimingInfo.lSTATE, g_vboinfo[g_vbo_recno[port]].IDUAL);

	//wait1_pivs(ch_v);
	//wait1_pivs(ch_v);

	return;
}


void dvC341_set_mct(int ch_v, int ipoff)
{
	int isyct, ifldc, mct;
	int nren = 0, val0, val1, val2;
	eSCAN_MODE scan_mode;
	int ibank_offset;

	static int nren_old[2] = { -1, -1 };

	ibank_offset = 16 * 2 * ch_v;

	scan_mode = (ipoff!=0) ? 0 : m_sChannelInfo[ch_v].sInputTimingInfo.eScanMode;
	isyct = 0;  // = (PI_SYNC_POL[ch_v] == 0) ? 0x00 : 0x30;
	ifldc = (m_sChannelSetting[ch_v].cFREEZE != 0) ? 0x02 : (scan_mode == 0) ? 0x01 : 0x00;
	dvC341_Write(B16_ISYCTCH1, (isyct | ifldc), ibank_offset);
	dvC341_Write(B32_ISYCTCH2, (isyct | (0x08 + (ch_v << 1))), ibank_offset);

	if (m_sChannelInfo[ch_v].sInputTimingInfo.lSTATE < 4)  //4k
	{
		if (m_sChannelSetting[ch_v].cFREEZE != 0)
		{
            if(dvC341_Read(B0_MCT1 + (ch_v << 9),0) != 0x01)
			    dvC341_Write(B0_MCT1 + (ch_v << 9), 0x01, 0);
            if(dvC341_Read(B0_MCT2 + (ch_v << 9),0) != 0x01)
			    dvC341_Write(B0_MCT2 + (ch_v << 9), 0x01, 0);
		}
		else
		{
            if(dvC341_Read(B0_MCT1 + (ch_v << 9),0) != 0x11)
                dvC341_Write(B0_MCT1 + (ch_v << 9), 0x11, 0);
            if(dvC341_Read(B0_MCT2 + (ch_v << 9),0) != 0x11)
    		    dvC341_Write(B0_MCT2 + (ch_v << 9), 0x11, 0);
		}
	}
	else
	{
		if ((ipoff==0) & (ch_v < 2))
		{
			scan_mode = m_sChannelInfo[ch_v].sInputTimingInfo.eScanMode;
			//nren = (PM_BNR[ch_v] != 0) ? 2 : (PM_HNR[ch_v] != 0) | (PM_VNR[ch_v] != 0) | (PM_TNR[ch_v] != 0) | (PM_MNR[ch_v] != 0) ? 1 : 0;
		}
		else
		{
			scan_mode = eSCAN_MODE_PROGRESSIVE;
			nren = 0;
		}

		val0 = 0x01;															// OCMD
		val1 = 0x10;															// ICMD
		val2 = (scan_mode | nren) ? 0x0E : 0;									// IPEN/IPICMD/IPOCMD

		mct = val2 | val1 | val0;

		if (m_sChannelSetting[ch_v].cFREEZE != 0)
		{
			mct = mct & 0x0d;
		}

		if (nren != nren_old[ch_v])
		{
			nren_old[ch_v] = nren;
			if (scan_mode == 0) // Progressive
			{
				if (nren == 0)
				{
					dvC341_Write(B21_NRCONFCH1, 0x00, ibank_offset);
				}
				else // NR Enable
				{
					if (nren == 1) // (PM_BNR == 0)
					{
						dvC341_Write(B21_NRCONFCH1, 0x01, ibank_offset);
					}
					else
					{
						dvC341_Write(B21_NRCONFCH1, 0x03, ibank_offset);
					}
				}
			}
		}

		dvC341_Write(B0_MCT1 + (ch_v << 9), mct, 0);
		if((m_sChannelInfo[eC341_CH_V0].ePanelTimingId == PANEL_3D_OUTPUT) &&  (ch_v == eC341_CH_V0)) //H30K_Doulas_0036 Modify
		{
            UINT16 uiH_Active = m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive;
            UINT16 uiV_Active = m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive;
            uiH_Active *= g_vboinfo[g_vbo_recno[m_sChannelInfo[ch_v].ucInputPort]].CH;

            m_sChannelInfo[ch_v].ucMCT = mct;
            if(dvC341_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_FRAME_SEQUENTIAL)
            {
                if(g_vbo_recno[m_sChannelInfo[ch_v].ucInputPort] == 4) //H30K_Doulas_0029 8-lane
                {
                    dvC341_Write(B0_MCT2,  m_sChannelInfo[ch_v].ucMCT, 0);
                }
                else
                {
                    dvC341_Write(B0_MCT2,  m_sChannelInfo[ch_v].ucMCT & 0xEF, 0);
                }
            }
            else if((dvC341_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_OFF) &&
                    (uiV_Active <= 1200))
            {
                if(m_sChannelInfo[ch_v].sInputTimingInfo.eScanMode == eSCAN_MODE_INTERLACE) //H30K_Doulas_0005
                {
                    dvC341_Write(B0_MCT2,  0x01, 0); //H30K_Doulas_0034 Modify
                }
                else
                {
                    dvC341_Write(B0_MCT2,  m_sChannelInfo[ch_v].ucMCT & 0xEF, 0);
                }
            }
            else
                dvC341_Write(B0_MCT2,  m_sChannelInfo[ch_v].ucMCT, 0);
        }
		else
		    dvC341_Write(B0_MCT2 + (ch_v << 9), 0x01, 0);
	}
}


void dvC341_set_rtct_thru(void)
{
	dvC341_Write(B0_RHOLD, 0x01, 0);		// RTCT0-7 Hold

	dvC341_Write(B0_RTCT0, 0x00, 0);			// [7:4]RTG 1(OUTCH3/4)=0 / [3:0]RTG 0(OUTCH1/2)=0
	dvC341_Write(B0_RTCT2, 0x00, 0);			// [7:4]RTG 5(IN CH2)=0   / [3:0]RTG 4(IN CH1)=0
	dvC341_Write(B0_RTCT3, 0x00, 0);			// [7:4]RTG 7(IN CH4)=0   / [3:0]RTG 6(IN CH3)=0

	dvC341_Write(B0_RHOLD, 0x00, 0);		// RTCT0-7 Normal

	return;
}

void dvC341_set_rtct_stop(void)
{
	dvC341_Write(B0_RHOLD, 0x01, 0);		// RTCT0-7 Hold

	dvC341_Write(B0_RTCT0, 0xFF, 0);			// [7:4]RTG 1(OUTCH3/4)=F / [3:0]RTG 0(OUTCH1/2)=F
	dvC341_Write(B0_RTCT2, 0xFF, 0);			// [7:4]RTG 5(IN CH2)=F   / [3:0]RTG 4(IN CH1)=F
	dvC341_Write(B0_RTCT3, 0xFF, 0);			// [7:4]RTG 7(IN CH4)=F   / [3:0]RTG 6(IN CH3)=F

	dvC341_Write(B0_RHOLD, 0x00, 0);		// RTCT0-7 Normal
    g_rtct = TRUE;

	return;
}

void dvC341_set_rtct_normal(void)
{
	dvC341_Write(B0_RHOLD, 0x01, 0);		// RTCT0-7 Hold

	dvC341_Write(B0_RTCT0, 0xa9, 0);			// [7:4]RTG 1(OUTCH3/4)=a(POVSCH3/4) / [3:0]RTG 0(OUTCH1/2)=9(POVSCH1/2)
											// 4K@120Hz as x99
	// ch_v = 0
	if (m_sChannelInfo[eC341_CH_V0].sInputTimingInfo.lSTATE == 0) {					// No input
		dvC341_Write(B0_RTCT2, 0x00, 0);		// [7:4]RTG 5(IN CH2)=0 / [3:0]RTG 4(IN CH1)=0
	}
	else if (m_sChannelInfo[eC341_CH_V0].sInputTimingInfo.lSTATE < 4) {				// 4k input
		dvC341_Write(B0_RTCT2, 0x11, 0);		// [7:4]RTG 5(IN CH2)=1 / [3:0]RTG 4(IN CH1)=1(PIVS1)
	}
	else {									// 2k input
		dvC341_Write(B0_RTCT2, 0x01, 0);		// [7:4]RTG 5(IN CH2)=0 / [3:0]RTG 4(IN CH1)=1(PIVS1)
	}

	// ch_v = 1
	if (m_sChannelInfo[eC341_CH_V1].sInputTimingInfo.lSTATE == 0) {					// No input
		dvC341_Write(B0_RTCT3, 0x00, 0);		// [7:4]RTG 7(IN CH4)=0 / [3:0]RTG 6(IN CH3)=0
	}
	else if (m_sChannelInfo[eC341_CH_V1].sInputTimingInfo.lSTATE < 4) {				// 4k input
		dvC341_Write(B0_RTCT3, 0x33, 0);		// [7:4]RTG 7(IN CH4)=3 / [3:0]RTG 6(IN CH3)=3(PIVS3)
	}
	else {									// 2k input
		dvC341_Write(B0_RTCT3, 0x03, 0);		// [7:4]RTG 7(IN CH4)=0 / [3:0]RTG 6(IN CH3)=3(PIVS3)
	}

	dvC341_Write(B0_RHOLD, 0x00, 0);		// RTCT0-7 Normal
    g_rtct = FALSE;
	return;
}


//Output scaling format
void dvC341_set_scfmfmt(eC341_CH_V ch_v, UINT8 FrameMemoryFormat)   //H2PF_Simon_0131
{
	int	write_val = 0x00;

	FRAME_MEMORY_FORMAT = FrameMemoryFormat;   //H2PF_Simon_0131

	if(FRAME_MEMORY_FORMAT == 24){						// 24bit(YUV444)
		write_val = 0x80;
	}
	else if(FRAME_MEMORY_FORMAT == 20){					// 20bit (YUV422)
		write_val = 0x01;
	}
	else if(FRAME_MEMORY_FORMAT == 16){					// 16bit (YUV422)
		write_val = 0x11;
	}
	else {												// 30bit(YUV444) (0)
		write_val = 0x00;
	}

	if(ch_v == 0)
	{
		dvC341_Write(B0_OSCWCTCH1, write_val, 0);
		dvC341_Write(B0_OSCWCTCH2, write_val, 0);
		dvC341_Write(B0_ISCWCTCH1, write_val, 0);
		dvC341_Write(B0_ISCWCTCH2, write_val, 0);
	}
	else
	{
		dvC341_Write(B0_OSCWCTCH3, write_val, 0);
		dvC341_Write(B0_OSCWCTCH4, write_val, 0);
		dvC341_Write(B0_ISCWCTCH3, write_val, 0);
		dvC341_Write(B0_ISCWCTCH4, write_val, 0);
	}
}



void dvC341_set_lut_hshrnk(INT32 hsscl, eC341_CH_V ch_v)
{
	int recno;
	int bank_offset;
	int i;

    static int Hshrink_lut_6s[20][24] =
    {
    	{0x02, 0x04, 0x06, 0x07, 0x07, 0x05, 0x03, 0x00, 0xfd, 0xf9, 0xf6, 0xf4, 0xf4, 0xf6, 0xfa, 0x00, 0x08, 0x11, 0x1b, 0x25, 0x2e, 0x37, 0x3c, 0x40},
    	{0x05, 0x06, 0x07, 0x06, 0x05, 0x03, 0x00, 0xfd, 0xfa, 0xf7, 0xf5, 0xf4, 0xf5, 0xf8, 0xfd, 0x03, 0x0b, 0x14, 0x1c, 0x26, 0x2e, 0x34, 0x39, 0x40},
    	{0x06, 0x07, 0x06, 0x05, 0x03, 0x00, 0xfe, 0xfa, 0xf8, 0xf6, 0xf5, 0xf5, 0xf7, 0xfb, 0x00, 0x06, 0x0e, 0x16, 0x1e, 0x26, 0x2d, 0x32, 0x36, 0x40},
    	{0x07, 0x06, 0x05, 0x03, 0x00, 0xfe, 0xfb, 0xf7, 0xf6, 0xf5, 0xf5, 0xf6, 0xf9, 0xfd, 0x02, 0x0a, 0x10, 0x18, 0x20, 0x27, 0x2d, 0x32, 0x36, 0x3e},
    	{0x05, 0x04, 0x02, 0x00, 0xfe, 0xfb, 0xf8, 0xf5, 0xf5, 0xf5, 0xf6, 0xf8, 0xfc, 0x00, 0x05, 0x0d, 0x13, 0x1a, 0x21, 0x28, 0x2d, 0x32, 0x36, 0x3c},
    	{0x03, 0x01, 0x00, 0xfd, 0xfb, 0xf8, 0xf7, 0xf5, 0xf5, 0xf6, 0xf8, 0xfb, 0xff, 0x03, 0x09, 0x10, 0x15, 0x1c, 0x23, 0x28, 0x2b, 0x32, 0x33, 0x36},
    	{0x00, 0xff, 0xfc, 0xfa, 0xf8, 0xf7, 0xf6, 0xf6, 0xf7, 0xf8, 0xfb, 0xfe, 0x01, 0x06, 0x0c, 0x12, 0x17, 0x1d, 0x23, 0x28, 0x2d, 0x2f, 0x30, 0x30},
    	{0xfd, 0xfb, 0xfa, 0xf8, 0xf7, 0xf7, 0xf7, 0xf8, 0xf9, 0xfb, 0xfe, 0x01, 0x05, 0x0a, 0x0e, 0x12, 0x18, 0x1e, 0x23, 0x27, 0x29, 0x2b, 0x2d, 0x2c},
    	{0xfb, 0xf9, 0xf8, 0xf8, 0xf7, 0xf8, 0xf9, 0xfb, 0xfc, 0xff, 0x01, 0x04, 0x08, 0x0c, 0x10, 0x13, 0x18, 0x1d, 0x21, 0x24, 0x27, 0x27, 0x28, 0x24},
    	{0xf9, 0xf9, 0xf9, 0xf9, 0xf9, 0xfa, 0xfc, 0xfe, 0xff, 0x01, 0x04, 0x07, 0x0a, 0x0e, 0x11, 0x12, 0x17, 0x1b, 0x1e, 0x20, 0x22, 0x23, 0x24, 0x20},
    	{0xfa, 0xfa, 0xfa, 0xfb, 0xfc, 0xfd, 0xff, 0x00, 0x02, 0x04, 0x06, 0x09, 0x0c, 0x0e, 0x11, 0x11, 0x16, 0x18, 0x1a, 0x1c, 0x1e, 0x1f, 0x1e, 0x1e},
    	{0xfb, 0xfc, 0xfd, 0xfe, 0xff, 0x00, 0x01, 0x02, 0x04, 0x06, 0x08, 0x0a, 0x0c, 0x0e, 0x10, 0x11, 0x14, 0x15, 0x17, 0x18, 0x19, 0x1b, 0x1c, 0x1a},
    	{0xfe, 0xff, 0xff, 0x00, 0x01, 0x02, 0x03, 0x05, 0x06, 0x08, 0x09, 0x0b, 0x0c, 0x0e, 0x0f, 0x10, 0x12, 0x13, 0x14, 0x15, 0x17, 0x16, 0x18, 0x16},
    	{0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x10, 0x11, 0x13, 0x15, 0x16, 0x15, 0x14},
    	{0x02, 0x03, 0x04, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x09, 0x0a, 0x0b, 0x0c, 0x0c, 0x0d, 0x0e, 0x0e, 0x0f, 0x0f, 0x11, 0x12, 0x13, 0x13, 0x14},
    	{0x04, 0x05, 0x06, 0x06, 0x07, 0x08, 0x08, 0x0a, 0x09, 0x0a, 0x0a, 0x0b, 0x0b, 0x0c, 0x0c, 0x0e, 0x0d, 0x0d, 0x0d, 0x0f, 0x11, 0x10, 0x12, 0x10},
    	{0x07, 0x07, 0x07, 0x08, 0x08, 0x09, 0x09, 0x0b, 0x0a, 0x0a, 0x0a, 0x0b, 0x0b, 0x0b, 0x0b, 0x0d, 0x0c, 0x0c, 0x0c, 0x0d, 0x10, 0x0f, 0x0f, 0x10},
    	{0x08, 0x08, 0x09, 0x09, 0x09, 0x09, 0x0a, 0x0b, 0x0a, 0x0a, 0x0a, 0x0a, 0x0b, 0x0b, 0x0b, 0x0d, 0x0b, 0x0b, 0x0b, 0x0d, 0x0e, 0x0f, 0x0e, 0x10},
    	{0x09, 0x09, 0x09, 0x0a, 0x0a, 0x0a, 0x0a, 0x0c, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0d, 0x0b, 0x0b, 0x0b, 0x0c, 0x0e, 0x0e, 0x0e, 0x0e},
    	{0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0c, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0c, 0x0a, 0x0a, 0x0a, 0x0c, 0x0e, 0x0e, 0x0e, 0x10},
    };


	for (i = 0; i < 2; i++)
	{
								   // cutoff
		recno = hsscl > 65000 ?  0 // 1.00
			  : hsscl > 62259 ?  1 // 0.95
			  : hsscl > 58982 ?  2 // 0.90
			  : hsscl > 55705 ?  3 // 0.85
			  : hsscl > 52428 ?  4 // 0.80
			  : hsscl > 49152 ?  5 // 0.75
			  : hsscl > 45875 ?  6 // 0.70
			  : hsscl > 42598 ?  7 // 0.65
			  : hsscl > 39321 ?  8 // 0.60
			  : hsscl > 36044 ?  9 // 0.55
			  : hsscl > 32768 ? 10 // 0.50
			  : hsscl > 29491 ? 11 // 0.45
			  : hsscl > 26214 ? 12 // 0.40
			  : hsscl > 22937 ? 13 // 0.35
			  : hsscl > 19660 ? 14 // 0.30
			  : hsscl > 16384 ? 15 // 0.25
			  : hsscl > 13107 ? 16 // 0.20
			  : hsscl >  9830 ? 17 // 0.15
			  : hsscl >  6553 ? 18 // 0.10
			  : 19;				 // 0.05

		bank_offset = 32 * ch_v + 16 * i;

		dvC341_WriteToBuffer(B17_HSLUT0CH1, Hshrink_lut_6s[recno][0], bank_offset);
		dvC341_WriteToBuffer(B17_HSLUT1CH1, Hshrink_lut_6s[recno][1], bank_offset);
		dvC341_WriteToBuffer(B17_HSLUT2CH1, Hshrink_lut_6s[recno][2], bank_offset);
		dvC341_WriteToBuffer(B17_HSLUT3CH1, Hshrink_lut_6s[recno][3], bank_offset);
		dvC341_WriteToBuffer(B17_HSLUT4CH1, Hshrink_lut_6s[recno][4], bank_offset);
		dvC341_WriteToBuffer(B17_HSLUT5CH1, Hshrink_lut_6s[recno][5], bank_offset);
		dvC341_WriteToBuffer(B17_HSLUT6CH1, Hshrink_lut_6s[recno][6], bank_offset);
		dvC341_WriteToBuffer(B17_HSLUT7CH1, Hshrink_lut_6s[recno][7], bank_offset);
		dvC341_WriteToBuffer(B17_HSLUT8CH1, Hshrink_lut_6s[recno][8], bank_offset);
		dvC341_WriteToBuffer(B17_HSLUT9CH1, Hshrink_lut_6s[recno][9], bank_offset);
		dvC341_WriteToBuffer(B17_HSLUT10CH1, Hshrink_lut_6s[recno][10], bank_offset);
		dvC341_WriteToBuffer(B17_HSLUT11CH1, Hshrink_lut_6s[recno][11], bank_offset);
		dvC341_WriteToBuffer(B17_HSLUT12CH1, Hshrink_lut_6s[recno][12], bank_offset);
		dvC341_WriteToBuffer(B17_HSLUT13CH1, Hshrink_lut_6s[recno][13], bank_offset);
		dvC341_WriteToBuffer(B17_HSLUT14CH1, Hshrink_lut_6s[recno][14], bank_offset);
		dvC341_WriteToBuffer(B17_HSLUT15CH1, Hshrink_lut_6s[recno][15], bank_offset);
		dvC341_WriteToBuffer(B17_HSLUT16CH1, Hshrink_lut_6s[recno][16], bank_offset);
		dvC341_WriteToBuffer(B17_HSLUT17CH1, Hshrink_lut_6s[recno][17], bank_offset);
		dvC341_WriteToBuffer(B17_HSLUT18CH1, Hshrink_lut_6s[recno][18], bank_offset);
		dvC341_WriteToBuffer(B17_HSLUT19CH1, Hshrink_lut_6s[recno][19], bank_offset);
		dvC341_WriteToBuffer(B17_HSLUT20CH1, Hshrink_lut_6s[recno][20], bank_offset);
		dvC341_WriteToBuffer(B17_HSLUT21CH1, Hshrink_lut_6s[recno][21], bank_offset);
		dvC341_WriteToBuffer(B17_HSLUT22CH1, Hshrink_lut_6s[recno][22], bank_offset);
		dvC341_WriteToBuffer(B17_HSLUT23CH1, Hshrink_lut_6s[recno][23], bank_offset);
	}

	dvC341_Buffer_Flush();
}


void dvC341_set_lut_vshrnk(INT32 vsscl, eC341_CH_V ch_v)
{
	int recno;
	int bank_offset;
	int i;

    static int Vshrink_lut_6s[20][24] =
    {
    	{0x02, 0x04, 0x06, 0x07, 0x07, 0x05, 0x03, 0x00, 0xfd, 0xf9, 0xf6, 0xf4, 0xf4, 0xf6, 0xfa, 0x00, 0x08, 0x11, 0x1b, 0x25, 0x2e, 0x37, 0x3c, 0x40},
    	{0x05, 0x06, 0x07, 0x06, 0x05, 0x03, 0x00, 0xfd, 0xfa, 0xf7, 0xf5, 0xf4, 0xf5, 0xf8, 0xfd, 0x03, 0x0b, 0x14, 0x1c, 0x26, 0x2e, 0x34, 0x39, 0x40},
    	{0x06, 0x07, 0x06, 0x05, 0x03, 0x00, 0xfe, 0xfa, 0xf8, 0xf6, 0xf5, 0xf5, 0xf7, 0xfb, 0x00, 0x06, 0x0e, 0x16, 0x1e, 0x26, 0x2d, 0x32, 0x36, 0x40},
    	{0x07, 0x06, 0x05, 0x03, 0x00, 0xfe, 0xfb, 0xf7, 0xf6, 0xf5, 0xf5, 0xf6, 0xf9, 0xfd, 0x02, 0x0a, 0x10, 0x18, 0x20, 0x27, 0x2d, 0x32, 0x36, 0x3e},
    	{0x05, 0x04, 0x02, 0x00, 0xfe, 0xfb, 0xf8, 0xf5, 0xf5, 0xf5, 0xf6, 0xf8, 0xfc, 0x00, 0x05, 0x0d, 0x13, 0x1a, 0x21, 0x28, 0x2d, 0x32, 0x36, 0x3c},
    	{0x03, 0x01, 0x00, 0xfd, 0xfb, 0xf8, 0xf7, 0xf5, 0xf5, 0xf6, 0xf8, 0xfb, 0xff, 0x03, 0x09, 0x10, 0x15, 0x1c, 0x23, 0x28, 0x2b, 0x32, 0x33, 0x36},
    	{0x00, 0xff, 0xfc, 0xfa, 0xf8, 0xf7, 0xf6, 0xf6, 0xf7, 0xf8, 0xfb, 0xfe, 0x01, 0x06, 0x0c, 0x12, 0x17, 0x1d, 0x23, 0x28, 0x2d, 0x2f, 0x30, 0x30},
    	{0xfd, 0xfb, 0xfa, 0xf8, 0xf7, 0xf7, 0xf7, 0xf8, 0xf9, 0xfb, 0xfe, 0x01, 0x05, 0x0a, 0x0e, 0x12, 0x18, 0x1e, 0x23, 0x27, 0x29, 0x2b, 0x2d, 0x2c},
    	{0xfb, 0xf9, 0xf8, 0xf8, 0xf7, 0xf8, 0xf9, 0xfb, 0xfc, 0xff, 0x01, 0x04, 0x08, 0x0c, 0x10, 0x13, 0x18, 0x1d, 0x21, 0x24, 0x27, 0x27, 0x28, 0x24},
    	{0xf9, 0xf9, 0xf9, 0xf9, 0xf9, 0xfa, 0xfc, 0xfe, 0xff, 0x01, 0x04, 0x07, 0x0a, 0x0e, 0x11, 0x12, 0x17, 0x1b, 0x1e, 0x20, 0x22, 0x23, 0x24, 0x20},
    	{0xfa, 0xfa, 0xfa, 0xfb, 0xfc, 0xfd, 0xff, 0x00, 0x02, 0x04, 0x06, 0x09, 0x0c, 0x0e, 0x11, 0x11, 0x16, 0x18, 0x1a, 0x1c, 0x1e, 0x1f, 0x1e, 0x1e},
    	{0xfb, 0xfc, 0xfd, 0xfe, 0xff, 0x00, 0x01, 0x02, 0x04, 0x06, 0x08, 0x0a, 0x0c, 0x0e, 0x10, 0x11, 0x14, 0x15, 0x17, 0x18, 0x19, 0x1b, 0x1c, 0x1a},
    	{0xfe, 0xff, 0xff, 0x00, 0x01, 0x02, 0x03, 0x05, 0x06, 0x08, 0x09, 0x0b, 0x0c, 0x0e, 0x0f, 0x10, 0x12, 0x13, 0x14, 0x15, 0x17, 0x16, 0x18, 0x16},
    	{0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x10, 0x11, 0x13, 0x15, 0x16, 0x15, 0x14},
    	{0x02, 0x03, 0x04, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x09, 0x0a, 0x0b, 0x0c, 0x0c, 0x0d, 0x0e, 0x0e, 0x0f, 0x0f, 0x11, 0x12, 0x13, 0x13, 0x14},
    	{0x04, 0x05, 0x06, 0x06, 0x07, 0x08, 0x08, 0x0a, 0x09, 0x0a, 0x0a, 0x0b, 0x0b, 0x0c, 0x0c, 0x0e, 0x0d, 0x0d, 0x0d, 0x0f, 0x11, 0x10, 0x12, 0x10},
    	{0x07, 0x07, 0x07, 0x08, 0x08, 0x09, 0x09, 0x0b, 0x0a, 0x0a, 0x0a, 0x0b, 0x0b, 0x0b, 0x0b, 0x0d, 0x0c, 0x0c, 0x0c, 0x0d, 0x10, 0x0f, 0x0f, 0x10},
    	{0x08, 0x08, 0x09, 0x09, 0x09, 0x09, 0x0a, 0x0b, 0x0a, 0x0a, 0x0a, 0x0a, 0x0b, 0x0b, 0x0b, 0x0d, 0x0b, 0x0b, 0x0b, 0x0d, 0x0e, 0x0f, 0x0e, 0x10},
    	{0x09, 0x09, 0x09, 0x0a, 0x0a, 0x0a, 0x0a, 0x0c, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0d, 0x0b, 0x0b, 0x0b, 0x0c, 0x0e, 0x0e, 0x0e, 0x0e},
    	{0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0c, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0c, 0x0a, 0x0a, 0x0a, 0x0c, 0x0e, 0x0e, 0x0e, 0x10},
    };

	for (i = 0; i < 2; i++)
	{
								   // cutoff
		recno = vsscl > 65000 ?  0 // 1.00
			  : vsscl > 62259 ?  1 // 0.95
			  : vsscl > 58982 ?  2 // 0.90
			  : vsscl > 55705 ?  3 // 0.85
			  : vsscl > 52428 ?  4 // 0.80
			  : vsscl > 49152 ?  5 // 0.75
			  : vsscl > 45875 ?  6 // 0.70
			  : vsscl > 42598 ?  7 // 0.65
			  : vsscl > 39321 ?  8 // 0.60
			  : vsscl > 36044 ?  9 // 0.55
			  : vsscl > 32768 ? 10 // 0.50
			  : vsscl > 29491 ? 11 // 0.45
			  : vsscl > 26214 ? 12 // 0.40
			  : vsscl > 22937 ? 13 // 0.35
			  : vsscl > 19660 ? 14 // 0.30
			  : vsscl > 16384 ? 15 // 0.25
			  : vsscl > 13107 ? 16 // 0.20
			  : vsscl >  9830 ? 17 // 0.15
			  : vsscl >  6553 ? 18 // 0.10
			  : 19;				   // 0.05

		bank_offset = 32 * ch_v + 16 * i;

		dvC341_WriteToBuffer(B17_VSLUT0CH1, Vshrink_lut_6s[recno][0], bank_offset);
		dvC341_WriteToBuffer(B17_VSLUT1CH1, Vshrink_lut_6s[recno][1], bank_offset);
		dvC341_WriteToBuffer(B17_VSLUT2CH1, Vshrink_lut_6s[recno][2], bank_offset);
		dvC341_WriteToBuffer(B17_VSLUT3CH1, Vshrink_lut_6s[recno][3], bank_offset);
		dvC341_WriteToBuffer(B17_VSLUT4CH1, Vshrink_lut_6s[recno][4], bank_offset);
		dvC341_WriteToBuffer(B17_VSLUT5CH1, Vshrink_lut_6s[recno][5], bank_offset);
		dvC341_WriteToBuffer(B17_VSLUT6CH1, Vshrink_lut_6s[recno][6], bank_offset);
		dvC341_WriteToBuffer(B17_VSLUT7CH1, Vshrink_lut_6s[recno][7], bank_offset);
		dvC341_WriteToBuffer(B17_VSLUT8CH1, Vshrink_lut_6s[recno][8], bank_offset);
		dvC341_WriteToBuffer(B17_VSLUT9CH1, Vshrink_lut_6s[recno][9], bank_offset);
		dvC341_WriteToBuffer(B17_VSLUT10CH1, Vshrink_lut_6s[recno][10], bank_offset);
		dvC341_WriteToBuffer(B17_VSLUT11CH1, Vshrink_lut_6s[recno][11], bank_offset);
		dvC341_WriteToBuffer(B17_VSLUT12CH1, Vshrink_lut_6s[recno][12], bank_offset);
		dvC341_WriteToBuffer(B17_VSLUT13CH1, Vshrink_lut_6s[recno][13], bank_offset);
		dvC341_WriteToBuffer(B17_VSLUT14CH1, Vshrink_lut_6s[recno][14], bank_offset);
		dvC341_WriteToBuffer(B17_VSLUT15CH1, Vshrink_lut_6s[recno][15], bank_offset);
		dvC341_WriteToBuffer(B17_VSLUT16CH1, Vshrink_lut_6s[recno][16], bank_offset);
		dvC341_WriteToBuffer(B17_VSLUT17CH1, Vshrink_lut_6s[recno][17], bank_offset);
		dvC341_WriteToBuffer(B17_VSLUT18CH1, Vshrink_lut_6s[recno][18], bank_offset);
		dvC341_WriteToBuffer(B17_VSLUT19CH1, Vshrink_lut_6s[recno][19], bank_offset);
		dvC341_WriteToBuffer(B17_VSLUT20CH1, Vshrink_lut_6s[recno][20], bank_offset);
		dvC341_WriteToBuffer(B17_VSLUT21CH1, Vshrink_lut_6s[recno][21], bank_offset);
		dvC341_WriteToBuffer(B17_VSLUT22CH1, Vshrink_lut_6s[recno][22], bank_offset);
		dvC341_WriteToBuffer(B17_VSLUT23CH1, Vshrink_lut_6s[recno][23], bank_offset);
	}

	dvC341_Buffer_Flush();
}

void dvC341Test(void)
{
#if 0
    extern unsigned int POSD_SAD;
    extern unsigned int POSD_SAD0;
    extern unsigned int POSD_SAD1;
#endif
    UINT32 ucData = 0 ;
    static UINT8 ucIdx = 0;
    {
#if 0
        ucData = dvC821_Read(B0_RTCT, 0);
        printf("\r\nC821 read B0_RTCT = %d\r\n" , ucData) ;

        ucData = dvC821_Read(B0_BOWIHN , 0) ;
        printf("\r\nC821 read B0_BOWIHN = %d\r\n" , ucData) ;

        ucData = dvC821_Read(B17_ISYCTCH1, 0);
        printf("\r\nC821 read B17_ISYCTCH1 = %d\r\n" , ucData) ;
#endif


#if 1
//        ucData = dvC821_Read(B0_OSDCT, 0);
//        printf("\r\nC821 read B0_OSDCT = %d\r\n" , ucData) ;

        //for(ucIdx=1; ucIdx<0xFF; ucIdx++)
        {
            dvC341_Write(B4_OSDCT , ucIdx , 0) ;
            ucData = (dvC341_Read(B4_OSDCT , 0) & 0xFF) ;
            //if(ucIdx != ucData)
            {
                printf("ucIdx[%d] != Data[%d]\r\n", ucIdx, ucData) ;
            }
            ucIdx++;
        }

//        POSD_SAD0 = DEF_OSDSAD_LAYER0_4K & 0x3fffffff;
//        POSD_SAD1 = DEF_OSDSAD_LAYER1_4K & 0x3fffffff;
//        POSD_SAD = ( POSD_SAD != POSD_SAD0 ) ? POSD_SAD0 : POSD_SAD1;
//
//        printf("write POSD_SAD %X\r\n",POSD_SAD);
//        dvC821_Write(B3_OSDSAD1CH1, POSD_SAD/*POSD_SAD_CH1*/ & 0xffffffff, 0);
//
//        printf("read POSD_SAD\r\n");
//        for(ucIdx=0; ucIdx<4; ucIdx++)
//        {
//            ucData = dvC821_Read(0x031A01+0x100*ucIdx , 0);
//            printf("Reg %X, = %d\r\n", 0x031A01+0x100*ucIdx, ucData);
//        }
#endif
//        dvC821_Write(B6_FCGDTCH1 , 0x3e , 0) ;
//        dvC821_Write( B3_PLTDT1CH1, 0xea, 0 );
//        dvC821_Write(B1_PSQIMDT, 0x68, 0);
//        ucData = dvC821_Read(B6_FCGDTCH1 , 0) ;
//        printf("Data = %d\r\n" , ucData) ;
//
//        ucData = dvC821_Read(B3_OSDSAD1CH1 , 0);
//        printf("B3_OSDSAD1CH1 Data = %X\r\n" , ucData);
    }
}


void bkup_reg(void)
{
	bk_ddr3gds			= dvC341_Read(B1_DDR3GDS,0);
	bk_ddr3msdly		= dvC341_Read(B1_DDR3MSDLY,0);
	bk_ddr3wrlvlphct1	= dvC341_Read(B1_DDR3WRLVLPHCT1,0);
	bk_ddr3wrlvlphct2	= dvC341_Read(B1_DDR3WRLVLPHCT2,0);
	bk_ddr3wrlvlphct3	= dvC341_Read(B1_DDR3WRLVLPHCT3,0);
	bk_ddr3wrlvlphct4	= dvC341_Read(B1_DDR3WRLVLPHCT4,0);
	bk_ddr3wrlvlphct5	= dvC341_Read(B1_DDR3WRLVLPHCT5,0);
	bk_ddr3wrlvlphct6	= dvC341_Read(B1_DDR3WRLVLPHCT6,0);
	bk_ddr3wrlvlphct7	= dvC341_Read(B1_DDR3WRLVLPHCT7,0);
	bk_ddr3wrlvlphct8	= dvC341_Read(B1_DDR3WRLVLPHCT8,0);
	bk_ddr3wrtmgct		= dvC341_Read(B1_DDR3WRTMGCT,0);
	bk_ddr3rdtmgct1		= dvC341_Read(B1_DDR3RDTMGCT1,0);
	bk_ddr3rdtmgct2		= dvC341_Read(B1_DDR3RDTMGCT2,0);
	bk_ddr3rdtmgct3		= dvC341_Read(B1_DDR3RDTMGCT3,0);
	bk_ddr3rdtmgct4		= dvC341_Read(B1_DDR3RDTMGCT4,0);
	bk_ddr3rdtmgct5		= dvC341_Read(B1_DDR3RDTMGCT5,0);
	bk_ddr3rdtmgct6		= dvC341_Read(B1_DDR3RDTMGCT6,0);
	bk_ddr3rdtmgct7		= dvC341_Read(B1_DDR3RDTMGCT7,0);
	bk_ddr3rdtmgct8		= dvC341_Read(B1_DDR3RDTMGCT8,0);

	return;
}
void check_wrlvlphct(void)
{
	int b,p,d;
	int wdat,rdat,res;
	FILE *fpw;

	printf("check WRLVLPHCT ...\n");
	fpw = fopen("/mnt/configs/scaler/C341_DDR_test/check_WRLVLPHCT.txt","w");

	char data[32] = ""; time_t t = time(NULL); strftime(data, sizeof(data), "%Y/%m/%d %a %H:%M:%S", localtime(&t));
	printf("%s\n", data);
	fprintf(fpw, "%s\n", data); //write Time-stamp

	int devicecode;
	dvC341_Write(BN_RGBNK, 0xb2, 0);//bank 178
	devicecode = dvC341_Read(B178_DEVICECODE, 0);
	if (devicecode == 0x28) {
		fprintf(fpw, "----------------------------------------------------\n");
		fprintf(fpw, "C341 SPI access is available	(O v O)\n");
		fprintf(fpw, "----------------------------------------------------\n");
	}
	else {
		fprintf(fpw, "----------------------------------------------------\n");
		fprintf(fpw, "C341 SPI access is not available !!! \n");
		fprintf(fpw, "----------------------------------------------------\n");
	}

	for(b=1; b<=8; b++){	// bytelane 1-8
		printf("  check bytelane%d ...\n",b);
		fprintf(fpw,"For bytelane-%d\n",b);
		fprintf(fpw,"B1_DDR3WRLVLPHCT%d_PDLSET%d[6:5](Dec),B1_DDR3WRLVLPHCT%d_PDLSET%d[4:0](Dec),B1_DDR3WRLVLPHCT%d_PDLSET%d[6:0](Hex),Result\n",b,b,b,b,b,b);

		for(p=0; p<=3; p++){	// B1_DDR3WRLVLPHCT_PDLSET[6:5] 0-3
			for(d=0; d<=31; d++){	// B1_DDR3WRLVLPHCT_PDLSET[4:0] 0-31
				wdat = ((p<<5)&0x60)|(d&0x1f);
				dvC341_Write(B1_DDR3WRLVLPHCT1+((b-1)<<8),wdat,0);
				dvC341_Write(B1_DDR3LVLCT 	,0x80,0);
				dvC341_Write(B1_DDR3LVLCT 	,0x00,0);

				dvC341_Write(B1_MEMCHKCT		,0x00,0);
				dvC341_Write(B1_MEMCHKST		,PS_MEMCHKSTAD,0);
				dvC341_Write(B1_MEMCHKEND 	,PS_MEMCHKENDAD,0);
				dvC341_Write(B1_MEMCHKCT		,0x13,0);
				do{
					rdat = (dvC341_Read(B1_MEMCHKSTAT,0))&0x01;
				}while(rdat==0x01);
				rdat = (dvC341_Read(B1_MEMCHKSTAT,0))&0x02;
				res  = (rdat>>1);

				fprintf(fpw,"%d,%d,%X,",p,d,wdat);
				if ((b == 1) && (p == (bk_ddr3wrlvlphct1 & 0x60) >> 5) && (d == (bk_ddr3wrlvlphct1 & 0x1f))) {
					if (res == 0) fprintf(fpw, "PASS");
					else fprintf(fpw, "FAIL");
				}
				else if ((b == 2) && (p == (bk_ddr3wrlvlphct2 & 0x60) >> 5) && (d == (bk_ddr3wrlvlphct2 & 0x1f))) {
					if (res == 0) fprintf(fpw, "PASS");
					else fprintf(fpw, "FAIL");
				}
				else if ((b == 3) && (p == (bk_ddr3wrlvlphct3 & 0x60) >> 5) && (d == (bk_ddr3wrlvlphct3 & 0x1f))) {
					if (res == 0) fprintf(fpw, "PASS");
					else fprintf(fpw, "FAIL");
				}
				else if ((b == 4) && (p == (bk_ddr3wrlvlphct4 & 0x60) >> 5) && (d == (bk_ddr3wrlvlphct4 & 0x1f))) {
					if (res == 0) fprintf(fpw, "PASS");
					else fprintf(fpw, "FAIL");
				}
				else if ((b == 5) && (p == (bk_ddr3wrlvlphct5 & 0x60) >> 5) && (d == (bk_ddr3wrlvlphct5 & 0x1f))) {
					if (res == 0) fprintf(fpw, "PASS");
					else fprintf(fpw, "FAIL");
				}
				else if ((b == 6) && (p == (bk_ddr3wrlvlphct6 & 0x60) >> 5) && (d == (bk_ddr3wrlvlphct6 & 0x1f))) {
					if (res == 0) fprintf(fpw, "PASS");
					else fprintf(fpw, "FAIL");
				}
				else if ((b == 7) && (p == (bk_ddr3wrlvlphct7 & 0x60) >> 5) && (d == (bk_ddr3wrlvlphct7 & 0x1f))) {
					if (res == 0) fprintf(fpw, "PASS");
					else fprintf(fpw, "FAIL");
				}
				else if ((b == 8) && (p == (bk_ddr3wrlvlphct8 & 0x60) >> 5) && (d == (bk_ddr3wrlvlphct8 & 0x1f))) {
					if (res == 0) fprintf(fpw, "PASS");
					else fprintf(fpw, "FAIL");
				}
				else {
					if (res == 0) fprintf(fpw, "PASS\n");
					else fprintf(fpw, "FAIL\n");
				}

				if ((b == 1) && (p == (bk_ddr3wrlvlphct1 & 0x60) >> 5) && (d == (bk_ddr3wrlvlphct1 & 0x1f))) fprintf(fpw, "-----------Current setting calculated by C341_DDR3_GDS_MSDLY_PDLSET_selection_V1.0a - byte%d.xls\n", b);
				if ((b == 2) && (p == (bk_ddr3wrlvlphct2 & 0x60) >> 5) && (d == (bk_ddr3wrlvlphct2 & 0x1f))) fprintf(fpw, "-----------Current setting calculated by C341_DDR3_GDS_MSDLY_PDLSET_selection_V1.0a - byte%d.xls\n", b);
				if ((b == 3) && (p == (bk_ddr3wrlvlphct3 & 0x60) >> 5) && (d == (bk_ddr3wrlvlphct3 & 0x1f))) fprintf(fpw, "-----------Current setting calculated by C341_DDR3_GDS_MSDLY_PDLSET_selection_V1.0a - byte%d.xls\n", b);
				if ((b == 4) && (p == (bk_ddr3wrlvlphct4 & 0x60) >> 5) && (d == (bk_ddr3wrlvlphct4 & 0x1f))) fprintf(fpw, "-----------Current setting calculated by C341_DDR3_GDS_MSDLY_PDLSET_selection_V1.0a - byte%d.xls\n", b);
				if ((b == 5) && (p == (bk_ddr3wrlvlphct5 & 0x60) >> 5) && (d == (bk_ddr3wrlvlphct5 & 0x1f))) fprintf(fpw, "-----------Current setting calculated by C341_DDR3_GDS_MSDLY_PDLSET_selection_V1.0a - byte%d.xls\n", b);
				if ((b == 6) && (p == (bk_ddr3wrlvlphct6 & 0x60) >> 5) && (d == (bk_ddr3wrlvlphct6 & 0x1f))) fprintf(fpw, "-----------Current setting calculated by C341_DDR3_GDS_MSDLY_PDLSET_selection_V1.0a - byte%d.xls\n", b);
				if ((b == 7) && (p == (bk_ddr3wrlvlphct7 & 0x60) >> 5) && (d == (bk_ddr3wrlvlphct7 & 0x1f))) fprintf(fpw, "-----------Current setting calculated by C341_DDR3_GDS_MSDLY_PDLSET_selection_V1.0a - byte%d.xls\n", b);
				if ((b == 8) && (p == (bk_ddr3wrlvlphct8 & 0x60) >> 5) && (d == (bk_ddr3wrlvlphct8 & 0x1f))) fprintf(fpw, "-----------Current setting calculated by C341_DDR3_GDS_MSDLY_PDLSET_selection_V1.0a - byte%d.xls\n", b);

			}
		}

		dvC341_Write(B1_MEMCHKCT		,0x00,0);
		dvC341_Write(B1_DDR3WRLVLPHCT1,bk_ddr3wrlvlphct1,0);
		dvC341_Write(B1_DDR3WRLVLPHCT2,bk_ddr3wrlvlphct2,0);
		dvC341_Write(B1_DDR3WRLVLPHCT3,bk_ddr3wrlvlphct3,0);
		dvC341_Write(B1_DDR3WRLVLPHCT4,bk_ddr3wrlvlphct4,0);
		dvC341_Write(B1_DDR3WRLVLPHCT5,bk_ddr3wrlvlphct5,0);
		dvC341_Write(B1_DDR3WRLVLPHCT6,bk_ddr3wrlvlphct6,0);
		dvC341_Write(B1_DDR3WRLVLPHCT7,bk_ddr3wrlvlphct7,0);
		dvC341_Write(B1_DDR3WRLVLPHCT8,bk_ddr3wrlvlphct8,0);
		dvC341_Write(B1_DDR3LVLCT 	,0x80,0);
		dvC341_Write(B1_DDR3LVLCT 	,0x00,0);

		fprintf(fpw,"\n");
	}

	fclose(fpw);

	return;
}


void check_gdsmsdly(void)
{
	int b,g,m;
	int gds1,gds2,gds3,gds4,gds5,gds6,gds7,gds8;
	int msdly1,msdly2,msdly3,msdly4,msdly5,msdly6,msdly7,msdly8;
	int	wdat,rdat,res;
	FILE *fpw;

	printf("check GDS_MSDLY ...\n");
	fpw = fopen("/mnt/configs/scaler/C341_DDR_test/check_GDS_MSDLY.txt","w");

	char data[32] = ""; time_t t = time(NULL); strftime(data, sizeof(data), "%Y/%m/%d %a %H:%M:%S", localtime(&t));
	printf("%s\n", data);
	fprintf(fpw, "%s\n", data); //write Time-stamp

	int devicecode;
	dvC341_Write(BN_RGBNK, 0xb2, 0);//bank 178
	devicecode = dvC341_Read(B178_DEVICECODE, 0);
	if (devicecode == 0x28) {
		fprintf(fpw, "----------------------------------------------------\n");
		fprintf(fpw, "C341 SPI access is available  (O v O)\n");
		fprintf(fpw, "----------------------------------------------------\n");
	}
	else {
		fprintf(fpw, "----------------------------------------------------\n");
		fprintf(fpw, "C341 SPI access is not available !!! \n");
		fprintf(fpw, "----------------------------------------------------\n");
	}

	for(b=1; b<=8; b++){	// bytelane 1-8
		printf("  check bytelane%d ...\n",b);
		fprintf(fpw,"For bytelane-%d\n",b);
		fprintf(fpw,"B1_DDR3GDS_GDS%d[2:0](Dec),B1_DDR3MSDLY_MSDLY%d[3:0](Dec),Result\n",b,b);

		for(g=0; g<=7; g++){	// B1_DDR3GDS_GDS1-8[2:0] 0-7
			for(m=0; m<=15; m++){	// B1_DDR3MSDLY_MSDLY1-8[3:0] 0-15
				gds1 = (b==1) ? g : (bk_ddr3gds    )&0x7;
				gds2 = (b==2) ? g : (bk_ddr3gds>> 4)&0x7;
				gds3 = (b==3) ? g : (bk_ddr3gds>> 8)&0x7;
				gds4 = (b==4) ? g : (bk_ddr3gds>>12)&0x7;
				gds5 = (b==5) ? g : (bk_ddr3gds>>16)&0x7;
				gds6 = (b==6) ? g : (bk_ddr3gds>>20)&0x7;
				gds7 = (b==7) ? g : (bk_ddr3gds>>24)&0x7;
				gds8 = (b==8) ? g : (bk_ddr3gds>>28)&0x7;
				wdat = (gds8<<28)|(gds7<<24)|(gds6<<20)|(gds5<<16)
				      |(gds4<<12)|(gds3<< 8)|(gds2<< 4)|(gds1);
				dvC341_Write(B1_DDR3GDS		,wdat,0);

				msdly1 = (b==1) ? m : (bk_ddr3msdly    )&0xf;
				msdly2 = (b==2) ? m : (bk_ddr3msdly>> 4)&0xf;
				msdly3 = (b==3) ? m : (bk_ddr3msdly>> 8)&0xf;
				msdly4 = (b==4) ? m : (bk_ddr3msdly>>12)&0xf;
				msdly5 = (b==5) ? m : (bk_ddr3msdly>>16)&0xf;
				msdly6 = (b==6) ? m : (bk_ddr3msdly>>20)&0xf;
				msdly7 = (b==7) ? m : (bk_ddr3msdly>>24)&0xf;
				msdly8 = (b==8) ? m : (bk_ddr3msdly>>28)&0xf;
				wdat   = (msdly8<<28)|(msdly7<<24)|(msdly6<<20)|(msdly5<<16)
				        |(msdly4<<12)|(msdly3<< 8)|(msdly2<< 4)|(msdly1);
				dvC341_Write(B1_DDR3MSDLY		,wdat,0);

				dvC341_Write(B1_DDR3LVLCT		,0x80,0);
				dvC341_Write(B1_DDR3LVLCT		,0x00,0);

				dvC341_Write(B1_MEMCHKCT		,0x00,0);
				dvC341_Write(B1_MEMCHKST		,PS_MEMCHKSTAD,0);
				dvC341_Write(B1_MEMCHKEND		,PS_MEMCHKENDAD,0);
				dvC341_Write(B1_MEMCHKCT		,0x13,0);
				do{
					rdat = (dvC341_Read(B1_MEMCHKSTAT,0))&0x01;
				}while(rdat==0x01);
				rdat = (dvC341_Read(B1_MEMCHKSTAT,0))&0x02;
				res  = (rdat>>1);

				fprintf(fpw,"%d,%d,",g,m);
				if ((b == 1) && (g == (bk_ddr3gds & 0x00000007)) && (m == (bk_ddr3msdly & 0x0000000f))) {
					if (res == 0) fprintf(fpw, "PASS");
					else fprintf(fpw, "FAIL");
				}
				else if ((b == 2) && (g == (bk_ddr3gds & 0x00000070) >> 4) && (m == (bk_ddr3msdly & 0x000000f0) >> 4)) {
					if (res == 0) fprintf(fpw, "PASS");
					else fprintf(fpw, "FAIL");
				}
				else if ((b == 3) && (g == (bk_ddr3gds & 0x00000700) >> 8) && (m == (bk_ddr3msdly & 0x00000f00) >> 8)) {
					if (res == 0) fprintf(fpw, "PASS");
					else fprintf(fpw, "FAIL");
				}
				else if ((b == 4) && (g == (bk_ddr3gds & 0x00007000) >> 12) && (m == (bk_ddr3msdly & 0x0000f000) >> 12)) {
					if (res == 0) fprintf(fpw, "PASS");
					else fprintf(fpw, "FAIL");
				}
				else if ((b == 5) && (g == (bk_ddr3gds & 0x00070000) >> 16) && (m == (bk_ddr3msdly & 0x000f0000) >> 16)) {
					if (res == 0) fprintf(fpw, "PASS");
					else fprintf(fpw, "FAIL");
				}
				else if ((b == 6) && (g == (bk_ddr3gds & 0x00700000) >> 20) && (m == (bk_ddr3msdly & 0x00f00000) >> 20)) {
					if (res == 0) fprintf(fpw, "PASS");
					else fprintf(fpw, "FAIL");
				}
				else if ((b == 7) && (g == (bk_ddr3gds & 0x07000000) >> 24) && (m == (bk_ddr3msdly & 0x0f000000) >> 24)) {
					if (res == 0) fprintf(fpw, "PASS");
					else fprintf(fpw, "FAIL");
				}
				else if ((b == 8) && (g == (bk_ddr3gds & 0x70000000) >> 28) && (m == (bk_ddr3msdly & 0xf0000000) >> 28)) {
					if (res == 0) fprintf(fpw, "PASS");
					else fprintf(fpw, "FAIL");
				}
				else {
					if (res == 0) fprintf(fpw, "PASS\n");
					else fprintf(fpw, "FAIL\n");
				}
				if ((b == 1) && (g == (bk_ddr3gds & 0x00000007)) && (m == (bk_ddr3msdly & 0x0000000f))) fprintf(fpw, "-----------Current setting calculated by C341_DDR3_GDS_MSDLY_PDLSET_selection_V1.0a - byte%d.xls\n", b);
				if ((b == 2) && (g == (bk_ddr3gds & 0x00000070) >> 4) && (m == (bk_ddr3msdly & 0x000000f0) >> 4)) fprintf(fpw, "-----------Current setting calculated by C341_DDR3_GDS_MSDLY_PDLSET_selection_V1.0a - byte%d.xls\n", b);
				if ((b == 3) && (g == (bk_ddr3gds & 0x00000700) >> 8) && (m == (bk_ddr3msdly & 0x00000f00) >> 8)) fprintf(fpw, "-----------Current setting calculated by C341_DDR3_GDS_MSDLY_PDLSET_selection_V1.0a - byte%d.xls\n", b);
				if ((b == 4) && (g == (bk_ddr3gds & 0x00007000) >> 12) && (m == (bk_ddr3msdly & 0x0000f000) >> 12)) fprintf(fpw, "-----------Current setting calculated by C341_DDR3_GDS_MSDLY_PDLSET_selection_V1.0a - byte%d.xls\n", b);
				if ((b == 5) && (g == (bk_ddr3gds & 0x00070000) >> 16) && (m == (bk_ddr3msdly & 0x000f0000) >> 16)) fprintf(fpw, "-----------Current setting calculated by C341_DDR3_GDS_MSDLY_PDLSET_selection_V1.0a - byte%d.xls\n", b);
				if ((b == 6) && (g == (bk_ddr3gds & 0x00700000) >> 20) && (m == (bk_ddr3msdly & 0x00f00000) >> 20)) fprintf(fpw, "-----------Current setting calculated by C341_DDR3_GDS_MSDLY_PDLSET_selection_V1.0a - byte%d.xls\n", b);
				if ((b == 7) && (g == (bk_ddr3gds & 0x07000000) >> 24) && (m == (bk_ddr3msdly & 0x0f000000) >> 24)) fprintf(fpw, "-----------Current setting calculated by C341_DDR3_GDS_MSDLY_PDLSET_selection_V1.0a - byte%d.xls\n", b);
				if ((b == 8) && (g == (bk_ddr3gds & 0x70000000) >> 28) && (m == (bk_ddr3msdly & 0xf0000000) >> 28)) fprintf(fpw, "-----------Current setting calculated by C341_DDR3_GDS_MSDLY_PDLSET_selection_V1.0a - byte%d.xls\n", b);
			}
		}

		dvC341_Write(B1_MEMCHKCT		,0x00,0);
		dvC341_Write(B1_DDR3GDS		,bk_ddr3gds,0);
		dvC341_Write(B1_DDR3MSDLY		,bk_ddr3msdly,0);
		dvC341_Write(B1_DDR3LVLCT		,0x80,0);
		dvC341_Write(B1_DDR3LVLCT		,0x00,0);

		fprintf(fpw,"\n");
	}

	fclose(fpw);

	return;
}
void check_DDR_Stress(void)
{
	int b;
	int wdat,rdat,res;
	PS_MEMCHKSTAD = 0x0;

	//while(1)
	{
		dvC341_Write(B1_DDR3LVLCT 	,0x80,0);
		dvC341_Write(B1_DDR3LVLCT 	,0x00,0);

		dvC341_Write(B1_MEMCHKCT		,0x00,0);
		dvC341_Write(B1_MEMCHKST		,PS_MEMCHKSTAD,0);
		dvC341_Write(B1_MEMCHKEND 	,PS_MEMCHKENDAD,0);
		dvC341_Write(B1_MEMCHKCT		,0x13,0);
		do{
			rdat = (dvC341_Read(B1_MEMCHKSTAT,0))&0x01;
		}while(rdat==0x01);
		rdat = (dvC341_Read(B1_MEMCHKSTAT,0))&0x02;
		res  = (rdat>>1);

        #if 1
		{
			if (res == 0)
			{
				//LOG_MSG(db_ALWAYS, "DDR Pass\r\n");
			}
			else  LOG_MSG(db_ALWAYS, "DDR Fail!!!\r\n");
		}
		#endif

		dvC341_Write(B1_MEMCHKCT		,0x00,0);
		dvC341_Write(B1_DDR3LVLCT 	,0x80,0);
		dvC341_Write(B1_DDR3LVLCT 	,0x00,0);

		MS_SLEEP(100);
	}
	return;
}

UINT8 dvC341_IS_3D_Enable(void)         //H30K_Doulas_0002
{
    UINT8  ucValue = 0;
    UINT8  uc3D_Enable = ets_OFF;

    ucValue = m_sChannelSetting[eC341_CH_V0].eOSD_3D_Enable;
    if(m_sChannelSetting[eC341_CH_V0].eOSD_3D_Mode == eCM_3D_MODE_OFF)
    {
        ucValue = eCM_3D_FORMAT_OFF;
    }
    else if(m_sChannelSetting[eC341_CH_V0].cPIP_PBP_ON)
    {
        ucValue = eCM_3D_FORMAT_OFF;
    }

    switch(ucValue)
    {
        case eCM_3D_FORMAT_AUTO:
            //get frontend 3D infomation to check 3D on/off
            switch(dvC341_Input_3D_Format_Get())    //A70LV_Doulas_0154
            {
                case eINPUT_3D_TYPE_FRAMEPACKING:
                case eINPUT_3D_TYPE_SIDEBYSIDE:
                case eINPUT_3D_TYPE_TOPANDBOTTOM:
                //case eINPUT_3D_TYPE_FRAME_SEQUENTIAL:   //ZU860_Doulas_0049 remove
                    uc3D_Enable = ets_ON;
                    break;

                default:
                    uc3D_Enable = ets_OFF;
                    break;
            }
            break;

        case eCM_3D_FORMAT_FRAME_PACKING:
        case eCM_3D_FORMAT_SIDE_BY_SIDE:
        case eCM_3D_FORMAT_TOP_AND_BOTTOM:
        case eCM_3D_FORMAT_FRAME_SEQUENTIAL:
        case eCM_3D_FORMAT_DUALPIPE:
        case eCM_3D_FORMAT_DUALPIPE_3D:
        case eCM_3D_FORMAT_4K3D:
        case eCM_3D_FORMAT_4K3D_DUALPIPE:
            uc3D_Enable = ets_ON;
            break;

        case eCM_3D_FORMAT_OFF:
            uc3D_Enable = ets_OFF;
            break;

        default:
            break;
    }

    return uc3D_Enable;
}

UINT16 dvC341_OutputActiveHW_1CH(void)
{
    return m_sChannelInfo[eC341_CH_V0].sOutputTimingInfo.uiHActive / OUTPUT_CH_NUM;
}


void dvC341_FrameDelaySet(UINT16 uiValue)        //H30K_Doulas_0010//H30K_Doulas_0003
{
    UINT8 ucRegOffset = 0;
	#ifdef Low_Latency_All
	if(dvC341_IS_3D_Enable())   //H30K_Doulas_0010
    {
        if(uiValue > 0)
        {
            dvC341_Write(B8_FLDDLYCH1, uiValue-1, ucRegOffset);  //Field Delay
            dvC341_Write(B8_SYRDLYCH1, uiValue,   ucRegOffset);  //Forced sync reset delay
        }
        else
        {
            dvC341_Write(B8_FLDDLYCH1, 0, ucRegOffset);         //Field Delay
            dvC341_Write(B8_SYRDLYCH1, 1,   ucRegOffset);       //Forced sync reset delay
        }
    }
	#else
	if(uiValue > 0)
    {
        dvC341_Write(B8_FLDDLYCH1, uiValue-1, ucRegOffset);  //Field Delay
        dvC341_Write(B8_SYRDLYCH1, uiValue,   ucRegOffset);  //Forced sync reset delay
    }
    else
    {
        dvC341_Write(B8_FLDDLYCH1, 0, ucRegOffset);         //Field Delay
        dvC341_Write(B8_SYRDLYCH1, 1,   ucRegOffset);       //Forced sync reset delay
    }
	#endif	/*Low_Latency_All*/
}

void dvC341_In120_Out240_CalFunction(UINT8 ch_v,UINT16 *uiSYRDLY,UINT16 *uiFLDDLY) //H30K_Doulas_0012
{
    UINT16 IFR = 120;
    UINT16 SYVRSTCT = 2;
    UINT16 HSYRMOD = 1;
    UINT32 TOCLK = 300000000l;
    UINT16 OHCYCL = 1016;
    UINT16 OACTVST = 5;
    UINT16 OACTVW = 1200;
    UINT16 Margin = 16;
    UINT16 IV_TOATL,IACTVST,IACTVW;
    UINT16 OFR = 240;
    DOUBLE OHSCY,TIST,TIED,TOST,TOED;
    DOUBLE TIST_TOST,TIED_TOED,MINDLY;
    UINT16 MINDLY_OHS;
    DOUBLE OFV,IFV_OHS;

    UINT16 TMP1,TMP2;
    UINT16 SYRDLY_MIN,SYRDLY_TMP;


    IV_TOATL  = m_sChannelInfo[ch_v].sInputTimingInfo.uiVTotal;
    IACTVST   = g_B16_IACTVST[ch_v];    //H30K_Doulas_0020//dvC341_Read(B16_IACTVSTCH1, ch_v * CH_V_BANK_OFFSET); //m_sChannelInfo[ch_v].sInputTimingInfo.uiVStart;
    IACTVW    = g_B16_IACTVW[ch_v];     //H30K_Doulas_0020//dvC341_Read(B16_IACTVWCH1, ch_v * CH_V_BANK_OFFSET); //m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive;

    OHCYCL    = m_sChannelInfo[ch_v].sOutputTimingInfo.uiHTotal/OUTPUT4K_CH_NUM;
    OACTVST   = g_B8_OACTVST[ch_v];     //H30K_Doulas_0020//dvC341_Read(B8_OACTVSTCH1, ch_v * CH_V_BANK_OFFSET); //m_sChannelInfo[ch_v].sOutputTimingInfo.uiVStart;
    OACTVW    = g_B8_OACTVW[ch_v];      //H30K_Doulas_0020//dvC341_Read(B8_OACTVWCH1, ch_v * CH_V_BANK_OFFSET); //m_sChannelInfo[ch_v].sOutputTimingInfo.uiVActive;


    OFR = IFR * SYVRSTCT;
    OHSCY = (DOUBLE)OHCYCL / (DOUBLE)TOCLK;

    TIST = (DOUBLE)IACTVST/(DOUBLE)IV_TOATL/(DOUBLE)IFR;
    TIED = (DOUBLE)(IACTVST + IACTVW)/(DOUBLE)IV_TOATL/(DOUBLE)IFR;
    TOST = (DOUBLE)(OHSCY * OACTVST);
    TOED = (DOUBLE)(OHSCY * (OACTVST + OACTVW));

    TIST_TOST = TIST - TOST;
    TIED_TOED = TIED - TOED;
    if(TIST_TOST > TIED_TOED)
        MINDLY = TIST_TOST;
    else
        MINDLY = TIED_TOED;

    MINDLY_OHS = (UINT16)(MINDLY/OHSCY) + 1;
    OFV = 1.0/(DOUBLE)(OHSCY * OFR);
    IFV_OHS = 1.0/(DOUBLE)(OHSCY * IFR);

    TMP1 = (UINT16)((MINDLY_OHS + Margin * 1)/OFV);
    TMP2 = (UINT16)((MINDLY_OHS + Margin * 2)/OFV);
    if(HSYRMOD == 0)
        SYRDLY_MIN = 2;
    else
        SYRDLY_MIN = 1;
    SYRDLY_TMP = (UINT16)(MINDLY_OHS + Margin - TMP2 * OFV) + 1;
    if(TMP1 != TMP2)
    {
        *uiSYRDLY = SYRDLY_MIN;
    }
    else
    {
        *uiSYRDLY = SYRDLY_TMP;
    }

    *uiFLDDLY = (UINT16)((*uiSYRDLY + TMP2 * OFV)/2);
    LOG_MSG(db_DV_SCALER, "Total = (%d,%d) \r\n",OHCYCL,IV_TOATL);  //H30K_Doulas_0020
    LOG_MSG(db_DV_SCALER, "Vxx = (%d,%d,%d,%d) \r\n",IACTVST,IACTVW,OACTVST,OACTVW);


    LOG_MSG(db_DV_SCALER, "TIST = %f \r\n",TIST);
    LOG_MSG(db_DV_SCALER, "TIED = %f \r\n",TIED);
    LOG_MSG(db_DV_SCALER, "TOST = %f \r\n",TOST);
    LOG_MSG(db_DV_SCALER, "TOED = %f \r\n",TOED);

    LOG_MSG(db_DV_SCALER, "TIST_TOST = %f \r\n",TIST_TOST);
    LOG_MSG(db_DV_SCALER, "TIED_TOED = %f \r\n",TIED_TOED);

    LOG_MSG(db_DV_SCALER, "MINDLY_OHS = %d \r\n",MINDLY_OHS);
    LOG_MSG(db_DV_SCALER, "OFV = %f \r\n",OFV);
    LOG_MSG(db_DV_SCALER, "IFV_OHS = %f \r\n",IFV_OHS);

    LOG_MSG(db_DV_SCALER, "Temp = (%d,%d) \r\n",TMP1 ,TMP2);
    LOG_MSG(db_DV_SCALER, "uiSYRDLY = %d \r\n",*uiSYRDLY);
    LOG_MSG(db_DV_SCALER, "uiFLDDLY = %d \r\n",*uiFLDDLY);
}

void dvC341_FrontEnd_Timing_Set(UINT8 ucCH ,sVIDEO_TIMING2 sFrontEnd_Timing) //H30K_Doulas_0011
{
    memcpy(&m_FrontEnd_Timing[ucCH], &sFrontEnd_Timing, sizeof(sVIDEO_TIMING2));

    LOG_MSG(db_DV_SCALER, "CH      =0x%x \r\n",ucCH);
    LOG_MSG(db_DV_SCALER, "PCLK    =0x%x \r\n",m_FrontEnd_Timing[ucCH].u32VideoPCLK);
    LOG_MSG(db_DV_SCALER, "H-total =%d \r\n"  ,m_FrontEnd_Timing[ucCH].u16VideoHTotal);
    LOG_MSG(db_DV_SCALER, "V-total =%d \r\n"  ,m_FrontEnd_Timing[ucCH].u16VideoVTotal);
    LOG_MSG(db_DV_SCALER, "H-Activ =%d \r\n"  ,m_FrontEnd_Timing[ucCH].u16VideoHActive);
    LOG_MSG(db_DV_SCALER, "V-Activ =%d \r\n"  ,m_FrontEnd_Timing[ucCH].u16VideoVActive);
    LOG_MSG(db_DV_SCALER, "V-Freq  =%d \r\n"  ,m_FrontEnd_Timing[ucCH].u16VideoVRate);
}

void wait1_povs(void) //H30K_Doulas_0035//A35G2_Simon_0121
{
	int		w1osts;
	dvC341_Write(B5_INTEN_1 ,0x01,0);

	do{
		w1osts = dvC341_Read(B5_INTSTAT_1,0)&0x01;
	} while(w1osts!=0x01);

	dvC341_Write(B5_INTEN_1 ,0x00,0);
}

void wait1_pivs(int ch)  //H30K_Doulas_0035//A35G2_Simon_0121
{
	int		w1osts;
 	int		int_bit = (0x01<<(ch*2));

	dvC341_Write(B5_INTEN_0 ,(int_bit),0);
    dvC341_ResetTimeInState(); //H30K_Doulas_0039

	do{
		w1osts = dvC341_Read(B5_INTSTAT_0,0)&int_bit;
		if(dvC341_TimeElapsedInState() >= DEF_WAIT_TIMEOUT) //H30K_Doulas_0039
        {
            LOG_MSG(db_DV_SCALER, "---(func:%s, line:%d) timeout, int_stat[0x%X,0x%X] \n", __FUNCTION__, __LINE__, w1osts, int_bit);
            break;
        }
	} while(w1osts!=int_bit);

	dvC341_Write(B5_INTEN_0 ,0x00,0);
}

#define READ_POFLD_RECHECK_NUMBER (2)
UINT8 dvC341_3DPOFLD_State_Get(void) //H30K_Doulas_0035  //A35G2_Simon_0121
{
    UINT8 Retry = 50;
    UINT32 OFLD = 0;
    UINT32 OFLDLast = OFLD;
    UINT8 CheckPass = 0;

    wait1_pivs(eC341_CH_V0); //H30K_Doulas_0039
    OFLD = (dvC341_Read(B171_STATECH1, 0) >> 1) & 0x01;  //read OFLD[1]
    OFLDLast = OFLD;

    LOG_MSG(db_DV_SCALER, "Read OFLD (%d)\r\n", OFLD);

    while(Retry--)
    {
        LOG_MSG(db_DV_SCALER, "OFLD (%d) OFLDLast (%d)\r\n", OFLD, OFLDLast);

        wait1_pivs(eC341_CH_V0); //H30K_Doulas_0039
        OFLD = (dvC341_Read(B171_STATECH1, 0) >> 1) & 0x01;  //read OFLD[1]

        LOG_MSG(db_DV_SCALER, "Read OFLD (%d)\r\n", OFLD);

        if(OFLD == OFLDLast)
        {
            CheckPass++;
        }
        else
        {
            CheckPass = 0;
        }

        OFLDLast = OFLD;

        if(CheckPass >= READ_POFLD_RECHECK_NUMBER)
        {
            break;
        }
    }

    return OFLD;
}

eSCALER_EXEC_CODE dvC341_3DPOFLD_Check(UINT8 *ucIsReconfig)  //H30K_Doulas_0035//A35G2_Simon_0121
{
    UINT8 Retry = 5;
    UINT8 OFLD = dvC341_3DPOFLD_State_Get();

    *ucIsReconfig = FALSE;

    while(OFLD == LOW && Retry--)   //wrong, need to reconfig
    {
        LOG_MSG(db_DV_SCALER, "\r\n 3DPOFLD wrong detected, start recover !\r\n", *ucIsReconfig);

        dvC341_Write(B8_OSYCTCH1, 0x0a03, 0); //H30K_Doulas_0038 reset OFLD to 0
        wait1_pivs(eC341_CH_V0); //H30K_Doulas_0039
        dvC341_wait1_povs(eC341_CH_V0);
        dvC341_Write(B8_OSYCTCH1, 0xc003, 0); // enable 3D mode

        MS_SLEEP(100);

        *ucIsReconfig = TRUE;

        OFLD = dvC341_3DPOFLD_State_Get();

        if(OFLD == HIGH)  //Correct
        {
            break;
        }

    }

    return eSCALER_EXEC_CODE_PASS;
}


UINT16 dvC341_Get_Panel_H_Active(void)   //H2PF_Simon_0107
{
    return m_sChannelInfo[0].sOutputTimingInfo.uiHActive;
}

BOOL dvC341_Timing_Detect_Fail_By_Front_End(eC341_CH_V ch_v) //H30K_Doulas_0050
{
    BOOL bTiming_Detect_Fail = FALSE;

    if(((m_sChannelInfo[ch_v].sInputTimingInfo.uiHActive * g_vboinfo[g_vbo_recno[m_sChannelInfo[ch_v].ucInputPort]].CH) == m_FrontEnd_Timing[ch_v].u16VideoHActive) &&
         (m_sChannelInfo[ch_v].sInputTimingInfo.uiVActive == m_FrontEnd_Timing[ch_v].u16VideoVActive))
    {
        bTiming_Detect_Fail = FALSE;
    }
    else
    {
        bTiming_Detect_Fail = TRUE;
    }

    if(bTiming_Detect_Fail)
    {
        LOG_MSG(db_DV_SCALER, "++++===== Timing_Detect_Fail =====+++\r\n");
    }
    else
    {
     //   LOG_MSG(db_DV_SCALER, " 4K/2K OK\r\n");
    }

    return bTiming_Detect_Fail;
}

eSCAN_MODE dvC341_ScanMode_Get(int ch_v)   //H2PF_Simon_0131
{
    return m_sChannelInfo[ch_v].sInputTimingInfo.eScanMode;
}

void dvC341_ScalerFrameMemoryProcSet(UINT8 Enable30bit)   //H2PF_Simon_0131
{
    if(Enable30bit)
    {
        dvC341_set_scfmfmt(0, 30);
        dvC341_Set_ZoomShrink_4K(0);
    }
    else
    {
        if(WARP_ON_USE_FRAME_MEMORY_YUV422 == TRUE)  //H2PF_Simon_0137
        {
            dvC341_set_scfmfmt(0, 20);
        }
        else
        {
            dvC341_set_scfmfmt(0, 24);
        }

        dvC341_Set_ZoomShrink_4K(0);
    }
}

UINT8 dvC341_ScalerFrameMemoeyFormat_Get(void)   //H2PF_Simon_0131
{
    return FRAME_MEMORY_FORMAT;
}

//H2PF_Simon_0193 Start
UINT8 HDR_Demo_SplitScreenMode_Get(void)
{
    return HDR_DEMO_SPLIT;
}

void HDR_Demo_SplitScreenMode_Set(UINT8 value)
{
    HDR_DEMO_SPLIT = value;
}


//hlg_max : monitor 亮度
//sdr_max : 影片亮度
void dvC341_degamma_HLG(int sdr_max, int hlg_max)
{
	int i;
	double B, C, D, E;
	unsigned int x1, x2, x3, x4, y, y2;
	double a, b, c;
	double ratio;

	ratio = (double)hlg_max / sdr_max;

    dbmsg("[%s] ratio=%.2f\n", __func__, ratio);

	dvC341_Write(B0_RTCT0, 0x00, 0);
	dvC341_Write(B0_RTCT2, 0x00, 0);
	dvC341_Write(B0_RTCT3, 0x00, 0);

	//dvC341_Write(B0_OCFMTCH1, 0x91, 0); // CSC1RNGCH1[1:0] = 10b
	//dvC341_Write(B0_OCFMTCH2, 0x91, 0); // CSC1RNGCH1[1:0] = 10b

    if(HDR_Demo_SplitScreenMode_Get() == FALSE)
    {
	    dvC341_Write(BN_ACCMODE, 0x02, 0); // write CH1 and CH2
    }
    else
    {
	    dvC341_Write(BN_ACCMODE, 0x00, 0); // write CH1
    }


	dvC341_WriteToBuffer(B13_GMCTCH1, 0x00, 0);
	dvC341_WriteToBuffer(B13_GMBNKSEL11CH1, 0x00, 0);
	dvC341_WriteToBuffer(B13_GMCTCH1, 0x72, 0); // GMCSEL[1:0] = 11b (R=G=B), CPUACEN[1:0] = 01b (access bank B)
	dvC341_WriteToBuffer(B13_GMADCH1, 0x00, 0);
	dvC341_WriteToBuffer(B13_GMADCH1, 0x00, 0);

	a = 0.17883277;
	b = 0.28466892;
	c = 0.55991073;

	for (i = 0; i < 1024; i++)
	{
		B = (double)i / 1023;
		C = (B > 1.0) ? 1.0 : B;
		D = (C <= 0.5) ? pow(C, 2) / 3.0
					   : (exp((C - c) / a) + b) / 12.0;
		E = (double)D * ratio;
		y = (unsigned int)(E * 134217727 + 0.5);
		y2 = (y > 134217727) ? 134217727 : y;
		x1 = y2         & 0xff; // LSB
		x2 = (y2 >>  8) & 0xff;
		x3 = (y2 >> 16) & 0xff;
		x4 = (y2 >> 24) & 0xff; // MSB

		// write even address
		dvC341_WriteToBuffer(B13_GMDTCH1, x1, 0);
		dvC341_WriteToBuffer(B13_GMDTCH1, x2, 0);
		dvC341_WriteToBuffer(B13_GMDTCH1, x3, 0);
		dvC341_WriteToBuffer(B13_GMDTCH1, x4, 0);

		// write odd address (not used but need address increment)
		dvC341_WriteToBuffer(B13_GMDTCH1, x1, 0);
		dvC341_WriteToBuffer(B13_GMDTCH1, x2, 0);
		dvC341_WriteToBuffer(B13_GMDTCH1, x3, 0);
		dvC341_WriteToBuffer(B13_GMDTCH1, x4, 0);
	}

	dvC341_WriteToBuffer(B13_GMCTCH1, 0x03, 0); // enable bank B LUT

	dvC341_Write(BN_ACCMODE, 0x00, 0);

	return;
}

//sdr_max 為影片metadata 紀錄的最大亮度 (MaxCCL or Mastering display luminance)
void dvC341_HDR_degamma_PQ(int sdr_max)
{
	int i;
	double B, C, D, E;
	unsigned int x1, x2, x3, x4, y, y2;
	double m1, m2, c1, c2, c3;
	double pq_max;
	double ratio;

	pq_max = 10000;
	ratio = (double)pq_max / sdr_max;

    dbmsg("[%s] ratio=%.2f\r\n", __func__, ratio);

	dvC341_Write(B0_RTCT0, 0x00, 0);
	dvC341_Write(B0_RTCT2, 0x00, 0);
	dvC341_Write(B0_RTCT3, 0x00, 0);

	//write_C341(B0_OCFMTCH1, 0x91, 0); // CSC1RNGCH1[1:0] = 10b
	//write_C341(B0_OCFMTCH2, 0x91, 0); // CSC1RNGCH1[1:0] = 10b

    if(HDR_Demo_SplitScreenMode_Get() == FALSE)
    {
	    dvC341_Write(BN_ACCMODE, 0x02, 0); // write CH1 and CH2
    }
    else
    {
	    dvC341_Write(BN_ACCMODE, 0x00, 0); // write CH1
    }


	dvC341_WriteToBuffer(B13_GMCTCH1, 0x00, 0);
	dvC341_WriteToBuffer(B13_GMBNKSEL11CH1, 0x00, 0);
	dvC341_WriteToBuffer(B13_GMCTCH1, 0xb4, 0); // GMCSEL[1:0] = 11b (R=G=B), CPUACEN[1:0] = 10b (access bank A)
	dvC341_WriteToBuffer(B13_GMADCH1, 0x00, 0);
	dvC341_WriteToBuffer(B13_GMADCH1, 0x00, 0);

	m1 = 2610.0 / 4096.0 / 4.0;
	m2 = 2523.0 / 4096.0 * 128.0;
	c1 = 3424.0 / 4096.0;
	c2 = 2413.0 / 4096.0 * 32.0;
	c3 = 2392.0 / 4096.0 * 32.0;

	for (i = 0; i < 1024; i++)
	{
		B = (double)i / 1023;
		C = (B > 1.0) ? 1.0 : B;
		D = ((pow(C, 1/m2) - c1) < 0) ? pow(0 / (c2 - c3 * pow(C, 1/m2)), 1/m1)
									  : pow((pow(C, 1/m2) - c1) / (c2 - c3 * pow(C, 1/m2)), 1/m1);
		E = (double)D * ratio;  //i==769 => E == 1 (if sdr_max == 1000)
		y = (unsigned int)(E * 134217727 + 0.5);
		y2 = (y > 134217727) ? 134217727 : y;
		x1 = y2         & 0xff; // LSB
		x2 = (y2 >>  8) & 0xff;
		x3 = (y2 >> 16) & 0xff;
		x4 = (y2 >> 24) & 0xff; // MSB

		// write even address
		dvC341_WriteToBuffer(B13_GMDTCH1, x1, 0);
		dvC341_WriteToBuffer(B13_GMDTCH1, x2, 0);
		dvC341_WriteToBuffer(B13_GMDTCH1, x3, 0);
		dvC341_WriteToBuffer(B13_GMDTCH1, x4, 0);

		// write odd address (not used but need address increment)
		dvC341_WriteToBuffer(B13_GMDTCH1, x1, 0);
		dvC341_WriteToBuffer(B13_GMDTCH1, x2, 0);
		dvC341_WriteToBuffer(B13_GMDTCH1, x3, 0);
		dvC341_WriteToBuffer(B13_GMDTCH1, x4, 0);
	}

	dvC341_Write(B13_GMCTCH1, 0x05, 0); // enable bank A LUT

	dvC341_Write(BN_ACCMODE, 0x00, 0);

	return;
}


void degamma_PQ_strength(int sdr_max, double tone_mapping_strength)
{
	int i;
	double B, C, D, E;
	unsigned int x1, x2, x3, x4, y, y2;
	double m1, m2, c1, c2, c3;
	double pq_max;
	double ratio;

    double Mapped;

    if(tone_mapping_strength >= 1.0)
        tone_mapping_strength = 1.0;

	pq_max = 10000;
	ratio = (double)pq_max / sdr_max;
//printf("[%s]ratio=%.2f, tone_mapping_strength %f\n", __func__, ratio, tone_mapping_strength);

	dvC341_Write(B0_RTCT0, 0x00, 0);
	dvC341_Write(B0_RTCT2, 0x00, 0);
	dvC341_Write(B0_RTCT3, 0x00, 0);

	//write_C341(B0_OCFMTCH1, 0x91, 0); // CSC1RNGCH1[1:0] = 10b
	//write_C341(B0_OCFMTCH2, 0x91, 0); // CSC1RNGCH1[1:0] = 10b

    if(HDR_Demo_SplitScreenMode_Get() == FALSE)
    {
	    dvC341_Write(BN_ACCMODE, 0x02, 0); // write CH1 and CH2
    }
    else
    {
	    dvC341_Write(BN_ACCMODE, 0x00, 0); // write CH1
    }


	dvC341_WriteToBuffer(B13_GMCTCH1, 0x00, 0);
	dvC341_WriteToBuffer(B13_GMBNKSEL11CH1, 0x00, 0);
	dvC341_WriteToBuffer(B13_GMCTCH1, 0xb4, 0); // GMCSEL[1:0] = 11b (R=G=B), CPUACEN[1:0] = 10b (access bank A)
	dvC341_WriteToBuffer(B13_GMADCH1, 0x00, 0);
	dvC341_WriteToBuffer(B13_GMADCH1, 0x00, 0);

	m1 = 2610.0 / 4096.0 / 4.0;
	m2 = 2523.0 / 4096.0 * 128.0;
	c1 = 3424.0 / 4096.0;
	c2 = 2413.0 / 4096.0 * 32.0;
	c3 = 2392.0 / 4096.0 * 32.0;

	for (i = 0; i < 1024; i++)
	{
		B = (double)i / 1023;
		C = (B > 1.0) ? 1.0 : B;
		D = ((pow(C, 1/m2) - c1) < 0) ? pow(0 / (c2 - c3 * pow(C, 1/m2)), 1/m1)
									  : pow((pow(C, 1/m2) - c1) / (c2 - c3 * pow(C, 1/m2)), 1/m1);
		E = (double)D * ratio;  //i==769 => E == 1 (if sdr_max == 1000)

        Mapped = D * (1.0 - tone_mapping_strength) + E * tone_mapping_strength;

		y = (unsigned int)(Mapped * 134217727 + 0.5);
		y2 = (y > 134217727) ? 134217727 : y;
		x1 = y2         & 0xff; // LSB
		x2 = (y2 >>  8) & 0xff;
		x3 = (y2 >> 16) & 0xff;
		x4 = (y2 >> 24) & 0xff; // MSB

		// write even address
		dvC341_WriteToBuffer(B13_GMDTCH1, x1, 0);
		dvC341_WriteToBuffer(B13_GMDTCH1, x2, 0);
		dvC341_WriteToBuffer(B13_GMDTCH1, x3, 0);
		dvC341_WriteToBuffer(B13_GMDTCH1, x4, 0);

		// write odd address (not used but need address increment)
		dvC341_WriteToBuffer(B13_GMDTCH1, x1, 0);
		dvC341_WriteToBuffer(B13_GMDTCH1, x2, 0);
		dvC341_WriteToBuffer(B13_GMDTCH1, x3, 0);
		dvC341_WriteToBuffer(B13_GMDTCH1, x4, 0);
	}

	dvC341_Write(B13_GMCTCH1, 0x05, 0); // enable bank A LUT

	dvC341_Write(BN_ACCMODE, 0x00, 0);

	return;
}


void dvC341_degamma_SDREnhance(int sdr_max)
{
	int i;
	double B, C, D, E;
	unsigned int x1, x2, x3, x4, y, y2;
	double m1, m2, c1, c2, c3;
	double pq_max;
	double ratio;

	pq_max = 10000;
	ratio = (double)pq_max / sdr_max;
//printf("[%s]ratio=%.2f\n", __func__, ratio);

	dvC341_Write(B0_RTCT0, 0x00, 0);
	dvC341_Write(B0_RTCT2, 0x00, 0);
	dvC341_Write(B0_RTCT3, 0x00, 0);

	//write_C341(B0_OCFMTCH1, 0x91, 0); // CSC1RNGCH1[1:0] = 10b
	//write_C341(B0_OCFMTCH2, 0x91, 0); // CSC1RNGCH1[1:0] = 10b

    if(HDR_Demo_SplitScreenMode_Get() == FALSE)
    {
	    dvC341_Write(BN_ACCMODE, 0x02, 0); // write CH1 and CH2
    }
    else
    {
	    dvC341_Write(BN_ACCMODE, 0x00, 0); // write CH1
    }

	dvC341_WriteToBuffer(B13_GMCTCH1, 0x00, 0);
	dvC341_WriteToBuffer(B13_GMBNKSEL11CH1, 0x00, 0);
	dvC341_WriteToBuffer(B13_GMCTCH1, 0xb4, 0); // GMCSEL[1:0] = 11b (R=G=B), CPUACEN[1:0] = 10b (access bank A)
	dvC341_WriteToBuffer(B13_GMADCH1, 0x00, 0);
	dvC341_WriteToBuffer(B13_GMADCH1, 0x00, 0);

	m1 = 0.3;  //2610.0 / 4096.0 / 4.0;
	m2 = 70.0; //2523.0 / 4096.0 * 128.0;
	c1 = 3424.0 / 4096.0;
	c2 = 2413.0 / 4096.0 * 32.0;
	c3 = 2392.0 / 4096.0 * 32.0;

	for (i = 0; i < 1024; i++)
	{
		B = (double)i / 1023;
		C = (B > 1.0) ? 1.0 : B;
		D = ((pow(C, 1/m2) - c1) < 0) ? pow(0 / (c2 - c3 * pow(C, 1/m2)), 1/m1)
									  : pow((pow(C, 1/m2) - c1) / (c2 - c3 * pow(C, 1/m2)), 1/m1);
		E = (double)D * ratio;
		y = (unsigned int)(E * 134217727 + 0.5);
		y2 = (y > 134217727) ? 134217727 : y;
		x1 = y2         & 0xff; // LSB
		x2 = (y2 >>  8) & 0xff;
		x3 = (y2 >> 16) & 0xff;
		x4 = (y2 >> 24) & 0xff; // MSB

		// write even address
		dvC341_WriteToBuffer(B13_GMDTCH1, x1, 0);
		dvC341_WriteToBuffer(B13_GMDTCH1, x2, 0);
		dvC341_WriteToBuffer(B13_GMDTCH1, x3, 0);
		dvC341_WriteToBuffer(B13_GMDTCH1, x4, 0);

		// write odd address (not used but need address increment)
		dvC341_WriteToBuffer(B13_GMDTCH1, x1, 0);
		dvC341_WriteToBuffer(B13_GMDTCH1, x2, 0);
		dvC341_WriteToBuffer(B13_GMDTCH1, x3, 0);
		dvC341_WriteToBuffer(B13_GMDTCH1, x4, 0);
	}

	dvC341_Write(B13_GMCTCH1, 0x05, 0); // enable bank A LUT

	dvC341_Write(BN_ACCMODE, 0x00, 0);

	return;
}


//hlg_max 為螢幕亮度
void dvC341_HLG_y_lut(int hlg_max)
{
	int i, B;
	double C, D;
	unsigned int x1, x2, y, y2;

	dvC341_Write(B0_RTCT0, 0x00, 0);
	dvC341_Write(B0_RTCT2, 0x00, 0);
	dvC341_Write(B0_RTCT3, 0x00, 0);

	//write_C341(B0_OCFMTCH1, 0x91, 0); // CSC1RNGCH1[1:0] = 10b
	//write_C341(B0_OCFMTCH2, 0x91, 0); // CSC1RNGCH1[1:0] = 10b

    if(HDR_Demo_SplitScreenMode_Get() == FALSE)
    {
	    dvC341_Write(BN_ACCMODE, 0x02, 0); // write CH1 and CH2
    }
    else
    {
	    dvC341_Write(BN_ACCMODE, 0x00, 0); // write CH1
    }

	dvC341_WriteToBuffer(B13_OHLGYLUTCTCH1, 0x00, 0);
	dvC341_WriteToBuffer(B13_OHLGYLUTBKSELCH1, 0x00, 0);
	dvC341_WriteToBuffer(B13_OHLGYLUTCTCH1, 0xc0, 0); // CPUACEN[1:0] = 11b (access bank A&B)
	dvC341_WriteToBuffer(B13_OHLGYLUTADCH1, 0x00, 0);
	dvC341_WriteToBuffer(B13_OHLGYLUTADCH1, 0x00, 0);

	for (i = 0 ; i < 4096; i++)
	{
		B = (i < 1024) ? i
		  : (i < 2048) ? ((i - 1024) * 32 + 1024)
		  : (i < 3072) ? ((i - 2048) * 2048 + 32768)
		  : (((i - 3072) * 131072 + 2097152) > 134217727) ? 134217727 : ((i - 3072) * 131072 + 2097152);
		C = (double)B / 134217727;
		D = pow(C, (1.2 + 0.42 * log10(hlg_max / 1000) - 1.0));
		y = (unsigned int)(D * 4095 + 0.5);
		y2 = (y > 4095) ? 4095 : y;
		x1 = y2        & 0xff; // LSB
		x2 = (y2 >> 8) & 0xff; // MSB

		dvC341_WriteToBuffer(B13_OHLGYLUTDTCH1, x1, 0);
		dvC341_WriteToBuffer(B13_OHLGYLUTDTCH1, x2, 0);
	}

	dvC341_WriteToBuffer(B13_OHLGYLUTCTCH1, 0x0f, 0); // enable Y LUT with table shift interpolation mode
	dvC341_WriteToBuffer(B13_OHLGYLUTBKSELCH1, 0x40, 0); // 1-table mode

	dvC341_Write(BN_ACCMODE, 0x00, 0);

	return;
}



void dvC341_Panelgamma_BT709_Gamma22(void)
{
	int i, B;
	double C, D;
	unsigned int x1, x2, y, y2;

//printf("[%s]\n", __func__);

	dvC341_Write(B0_RTCT0, 0x00, 0);
	dvC341_Write(B0_RTCT2, 0x00, 0);
	dvC341_Write(B0_RTCT3, 0x00, 0);

    if(HDR_Demo_SplitScreenMode_Get() == FALSE)
    {
        dvC341_Write(B0_OCFMTCH1, 0x91, 0); // CSC1RNGCH1[1:0] = 10b
        dvC341_Write(B0_OCFMTCH2, 0x91, 0); // CSC1RNGCH1[1:0] = 10b
	    dvC341_Write(BN_ACCMODE, 0x02, 0); // write CH1 and CH2
    }
    else
    {
        dvC341_Write(B0_OCFMTCH1, 0x91, 0); // CSC1RNGCH1[1:0] = 10b
	    dvC341_Write(BN_ACCMODE, 0x00, 0); // write CH1
    }


	dvC341_WriteToBuffer(B13_GMCT2CH1, 0x00, 0);
	dvC341_WriteToBuffer(B13_GMBNKSEL21CH1, 0x00, 0);
	dvC341_WriteToBuffer(B13_GMCT2CH1, 0xf0, 0); // GMCSEL[1:0] = 11b (R=G=B), CPUACEN[1:0] = 11b (access bank A&B)
	dvC341_WriteToBuffer(B13_GMAD2CH1, 0x00, 0);
	dvC341_WriteToBuffer(B13_GMAD2CH1, 0x00, 0);

	for (i = 0 ; i < 4096; i++)
	{
        #if 1
		B = (i < 1024) ? i
		  : (i < 2048) ? ((i - 1024) * 32 + 1024)
		  : (i < 3072) ? ((i - 2048) * 2048 + 32768)
		  : (((i - 3072) * 131072 + 2097152) > 134217727) ? 134217727 : ((i - 3072) * 131072 + 2097152);
		C = (double)B / 134217727;
		//D = (C < 0.018) ? 4.5 * C : (1.099) * pow((double)C, 0.45) - 0.099; // refer BT.709-6. same as gamma=2.2
		D = pow(C, 1.0 / 2.2); // normal 2.2
        #else

        C = (double)i/4095;
        D = pow(C, 1.0 / 2.2); // normal 2.2

        #endif

		y = (unsigned int)(D * 4095 + 0.5);
		y2 = (y > 4095) ? 4095 : y;
		x1 = y2        & 0xff; // LSB
		x2 = (y2 >> 8) & 0xff; // MSB

		dvC341_WriteToBuffer(B13_GMDT2CH1, x1, 0);
		dvC341_WriteToBuffer(B13_GMDT2CH1, x2, 0);
	}

	dvC341_WriteToBuffer(B13_GMCT2CH1, 0x0f, 0); // enable panel-gamma with table shift interpolation mode
	dvC341_WriteToBuffer(B13_GMBNKSEL21CH1, 0x40, 0); // 1-table mode

	dvC341_Write(BN_ACCMODE, 0x00, 0);

	return;
}



void dvC341_Panelgamma_BT709_Gamma22_OnlyCH1(void)
{
	int i, B;
	double C, D;
	unsigned int x1, x2, y, y2;

//printf("[%s]\n", __func__);

	dvC341_Write(B0_RTCT0, 0x00, 0);
	dvC341_Write(B0_RTCT2, 0x00, 0);
	dvC341_Write(B0_RTCT3, 0x00, 0);

	dvC341_Write(B0_OCFMTCH1, 0x91, 0); // CSC1RNGCH1[1:0] = 10b
	//dvC341_Write(B0_OCFMTCH2, 0x91, 0); // CSC1RNGCH1[1:0] = 10b

	//dvC341_Write(BN_ACCMODE, 0x02, 0); // write CH1 and CH2

	dvC341_WriteToBuffer(B13_GMCT2CH1, 0x00, 0);
	dvC341_WriteToBuffer(B13_GMBNKSEL21CH1, 0x00, 0);
	dvC341_WriteToBuffer(B13_GMCT2CH1, 0xf0, 0); // GMCSEL[1:0] = 11b (R=G=B), CPUACEN[1:0] = 11b (access bank A&B)
	dvC341_WriteToBuffer(B13_GMAD2CH1, 0x00, 0);
	dvC341_WriteToBuffer(B13_GMAD2CH1, 0x00, 0);

	for (i = 0 ; i < 4096; i++)
	{
        #if 1
		B = (i < 1024) ? i
		  : (i < 2048) ? ((i - 1024) * 32 + 1024)
		  : (i < 3072) ? ((i - 2048) * 2048 + 32768)
		  : (((i - 3072) * 131072 + 2097152) > 134217727) ? 134217727 : ((i - 3072) * 131072 + 2097152);
		C = (double)B / 134217727;
		//D = (C < 0.018) ? 4.5 * C : (1.099) * pow((double)C, 0.45) - 0.099; // refer BT.709-6. same as gamma=2.2
		D = pow(C, 1.0 / 2.2); // normal 2.2
        #else

        C = (double)i/4095;
        D = pow(C, 1.0 / 2.2); // normal 2.2

        #endif

		y = (unsigned int)(D * 4095 + 0.5);
		y2 = (y > 4095) ? 4095 : y;
		x1 = y2        & 0xff; // LSB
		x2 = (y2 >> 8) & 0xff; // MSB

		dvC341_WriteToBuffer(B13_GMDT2CH1, x1, 0);
		dvC341_WriteToBuffer(B13_GMDT2CH1, x2, 0);
	}

	dvC341_WriteToBuffer(B13_GMCT2CH1, 0x0f, 0); // enable panel-gamma with table shift interpolation mode
	dvC341_WriteToBuffer(B13_GMBNKSEL21CH1, 0x40, 0); // 1-table mode

	dvC341_Write(BN_ACCMODE, 0x00, 0);

	return;
}



void panelgamma_BT709(void)
{
	int i, B;
	double C, D;
	unsigned int x1, x2, y, y2;

//printf("[%s]\n", __func__);

	dvC341_Write(B0_RTCT0, 0x00, 0);
	dvC341_Write(B0_RTCT2, 0x00, 0);
	dvC341_Write(B0_RTCT3, 0x00, 0);

	dvC341_Write(B0_OCFMTCH1, 0x91, 0); // CSC1RNGCH1[1:0] = 10b
	dvC341_Write(B0_OCFMTCH2, 0x91, 0); // CSC1RNGCH1[1:0] = 10b

	dvC341_Write(BN_ACCMODE, 0x02, 0); // write CH1 and CH2

	dvC341_WriteToBuffer(B13_GMCT2CH1, 0x00, 0);
	dvC341_WriteToBuffer(B13_GMBNKSEL21CH1, 0x00, 0);
	dvC341_WriteToBuffer(B13_GMCT2CH1, 0xf0, 0); // GMCSEL[1:0] = 11b (R=G=B), CPUACEN[1:0] = 11b (access bank A&B)
	dvC341_WriteToBuffer(B13_GMAD2CH1, 0x00, 0);
	dvC341_WriteToBuffer(B13_GMAD2CH1, 0x00, 0);

	for (i = 0 ; i < 4096; i++)
	{
		B = (i < 1024) ? i
		  : (i < 2048) ? ((i - 1024) * 32 + 1024)
		  : (i < 3072) ? ((i - 2048) * 2048 + 32768)
		  : (((i - 3072) * 131072 + 2097152) > 134217727) ? 134217727 : ((i - 3072) * 131072 + 2097152);
		C = (double)B / 134217727;
		D = (C < 0.018) ? 4.5 * C : (1.099) * pow((double)C, 0.45) - 0.099; // refer BT.709-6. same as gamma=2.2
		//D = pow(C, 1.0 / 2.22); // normal 2.2

		y = (unsigned int)(D * 4095 + 0.5);
		y2 = (y > 4095) ? 4095 : y;
		x1 = y2        & 0xff; // LSB
		x2 = (y2 >> 8) & 0xff; // MSB

		dvC341_WriteToBuffer(B13_GMDT2CH1, x1, 0);
		dvC341_WriteToBuffer(B13_GMDT2CH1, x2, 0);
	}

	dvC341_WriteToBuffer(B13_GMCT2CH1, 0x0f, 0); // enable panel-gamma with table shift interpolation mode
	dvC341_WriteToBuffer(B13_GMBNKSEL21CH1, 0x40, 0); // 1-table mode

	dvC341_Write(BN_ACCMODE, 0x00, 0);

	return;
}


void panelgamma_BT709_NA(void)
{
	int i, B;
	double C, D;
	unsigned int x1, x2, y, y2;

//printf("[%s]\n", __func__);

	dvC341_Write(B0_RTCT0, 0x00, 0);
	dvC341_Write(B0_RTCT2, 0x00, 0);
	dvC341_Write(B0_RTCT3, 0x00, 0);

	dvC341_Write(B0_OCFMTCH1, 0x91, 0); // CSC1RNGCH1[1:0] = 10b
	dvC341_Write(B0_OCFMTCH2, 0x91, 0); // CSC1RNGCH1[1:0] = 10b

	dvC341_Write(BN_ACCMODE, 0x02, 0); // write CH1 and CH2

	dvC341_WriteToBuffer(B13_GMCT2CH1, 0x00, 0);
	dvC341_WriteToBuffer(B13_GMBNKSEL21CH1, 0x00, 0);
	dvC341_WriteToBuffer(B13_GMCT2CH1, 0xf0, 0); // GMCSEL[1:0] = 11b (R=G=B), CPUACEN[1:0] = 11b (access bank A&B)
	dvC341_WriteToBuffer(B13_GMAD2CH1, 0x00, 0);
	dvC341_WriteToBuffer(B13_GMAD2CH1, 0x00, 0);

	for (i = 0 ; i < 4096; i++)
	{
		B = (i < 1024) ? i
		  : (i < 2048) ? ((i - 1024) * 32 + 1024)
		  : (i < 3072) ? ((i - 2048) * 2048 + 32768)
		  : (((i - 3072) * 131072 + 2097152) > 134217727) ? 134217727 : ((i - 3072) * 131072 + 2097152);
		C = (double)B / 134217727;
		//D = (C < 0.018) ? 4.5 * C : (1.099) * pow((double)C, 0.45) - 0.099; // refer BT.709-6. same as gamma=2.2
		D = C; //pow(C, 1.0 / 2.2); // normal 2.2

		y = (unsigned int)(D * 4095 + 0.5);
		y2 = (y > 4095) ? 4095 : y;
		x1 = y2        & 0xff; // LSB
		x2 = (y2 >> 8) & 0xff; // MSB

		dvC341_WriteToBuffer(B13_GMDT2CH1, x1, 0);
		dvC341_WriteToBuffer(B13_GMDT2CH1, x2, 0);
	}

	dvC341_WriteToBuffer(B13_GMCT2CH1, 0x0f, 0); // enable panel-gamma with table shift interpolation mode
	dvC341_WriteToBuffer(B13_GMBNKSEL21CH1, 0x40, 0); // 1-table mode

	dvC341_Write(BN_ACCMODE, 0x00, 0);

	return;
}



void dvC341_GamutBT2020ToREC709(void)
{
	int i, B;
	double C, D;
	unsigned int x1, x2, y, y2;

//printf("[%s]\n", __func__);

    if(HDR_Demo_SplitScreenMode_Get() == FALSE)
    {
        dvC341_WriteToBuffer(B0_OCFMTCH1, 0xB1, 0); // CSC1RNGCH1[1:0] = 10b
        dvC341_WriteToBuffer(B0_OCFMTCH2, 0xB1, 0); // CSC1RNGCH1[1:0] = 10b
        dvC341_WriteToBuffer(B0_ICFMTCH1, 0x09, 0);
        dvC341_WriteToBuffer(B0_ICFMTCH2, 0x09, 0);
	    dvC341_Write(BN_ACCMODE, 0x02, 0); // write CH1 and CH2
    }
    else
    {
        dvC341_WriteToBuffer(B0_OCFMTCH1, 0xB1, 0); // CSC1RNGCH1[1:0] = 10b
        dvC341_WriteToBuffer(B0_ICFMTCH1, 0x09, 0);
	    dvC341_Write(BN_ACCMODE, 0x00, 0); // write CH1
    }


	dvC341_WriteToBuffer(B12_OCSC2CF00CH1, 0x3523, 0);
	dvC341_WriteToBuffer(B12_OCSC2CF01CH1, 0xED32, 0);
	dvC341_WriteToBuffer(B12_OCSC2CF02CH1, 0xFDAB, 0);
	dvC341_WriteToBuffer(B12_OCSC2CF10CH1, 0xFC04, 0);
	dvC341_WriteToBuffer(B12_OCSC2CF11CH1, 0x2441, 0);
	dvC341_WriteToBuffer(B12_OCSC2CF12CH1, 0xFFBC, 0);
	dvC341_WriteToBuffer(B12_OCSC2CF20CH1, 0xFF6B, 0);
	dvC341_WriteToBuffer(B12_OCSC2CF21CH1, 0xFCC8, 0);
	dvC341_WriteToBuffer(B12_OCSC2CF22CH1, 0x23CD, 0);

	dvC341_Write(BN_ACCMODE, 0x00, 0);

	return;
}



void GamutRGBBT2020ToREC709(void)
{
	int i, B;
	double C, D;
	unsigned int x1, x2, y, y2;

//printf("[%s]\n", __func__);

	dvC341_Write(B0_OCFMTCH1, 0xB1, 0); // CSC1RNGCH1[1:0] = 10b
	dvC341_Write(B0_OCFMTCH2, 0xB1, 0); // CSC1RNGCH1[1:0] = 10b

	dvC341_WriteToBuffer(B0_ICFMTCH1, 0x18, 0);
	dvC341_WriteToBuffer(B0_ICFMTCH2, 0x18, 0);

	dvC341_Write(BN_ACCMODE, 0x02, 0); // write CH1 and CH2

	dvC341_WriteToBuffer(B12_OCSC2CF00CH1, 0x4000, 0);
	dvC341_WriteToBuffer(B12_OCSC2CF01CH1, 0xC525, 0);
	dvC341_WriteToBuffer(B12_OCSC2CF02CH1, 0xFADB, 0);
	dvC341_WriteToBuffer(B12_OCSC2CF10CH1, 0x21A0, 0);
	dvC341_WriteToBuffer(B12_OCSC2CF11CH1, 0x56C9, 0);
	dvC341_WriteToBuffer(B12_OCSC2CF12CH1, 0x0797, 0);
	dvC341_WriteToBuffer(B12_OCSC2CF20CH1, 0xEE22, 0);
	dvC341_WriteToBuffer(B12_OCSC2CF21CH1, 0xD1D1, 0);
	dvC341_WriteToBuffer(B12_OCSC2CF22CH1, 0x4000, 0);


	dvC341_Write(BN_ACCMODE, 0x00, 0);

	return;
}

void dvC341_GamutBT2020ToREC709_OnlyCH1(void)
{
	int i, B;
	double C, D;
	unsigned int x1, x2, y, y2;

//printf("[%s]\n", __func__);

	dvC341_Write(B0_OCFMTCH1, 0xB1, 0); // CSC1RNGCH1[1:0] = 10b
	//dvC341_Write(B0_OCFMTCH2, 0xB1, 0); // CSC1RNGCH1[1:0] = 10b

	dvC341_WriteToBuffer(B0_ICFMTCH1, 0x09, 0);
	//dvC341_WriteToBuffer(B0_ICFMTCH2, 0x09, 0);

	//dvC341_Write(BN_ACCMODE, 0x02, 0); // write CH1 and CH2

	dvC341_WriteToBuffer(B12_OCSC2CF00CH1, 0x3523, 0);
	dvC341_WriteToBuffer(B12_OCSC2CF01CH1, 0xED32, 0);
	dvC341_WriteToBuffer(B12_OCSC2CF02CH1, 0xFDAB, 0);
	dvC341_WriteToBuffer(B12_OCSC2CF10CH1, 0xFC04, 0);
	dvC341_WriteToBuffer(B12_OCSC2CF11CH1, 0x2441, 0);
	dvC341_WriteToBuffer(B12_OCSC2CF12CH1, 0xFFBC, 0);
	dvC341_WriteToBuffer(B12_OCSC2CF20CH1, 0xFF6B, 0);
	dvC341_WriteToBuffer(B12_OCSC2CF21CH1, 0xFCC8, 0);
	dvC341_WriteToBuffer(B12_OCSC2CF22CH1, 0x23CD, 0);

	dvC341_Write(BN_ACCMODE, 0x00, 0);

	return;
}



void GamutDCIP3ToREC709(void)
{
    int i, B;
    double C, D;
    unsigned int x1, x2, y, y2;

//printf("[%s]\n", __func__);

    dvC341_Write(B0_OCFMTCH1, 0xB1, 0); // CSC1RNGCH1[1:0] = 10b
    dvC341_Write(B0_OCFMTCH2, 0xB1, 0); // CSC1RNGCH1[1:0] = 10b

    dvC341_WriteToBuffer(B0_ICFMTCH1, 0x09, 0);
    dvC341_WriteToBuffer(B0_ICFMTCH2, 0x09, 0);

    dvC341_Write(BN_ACCMODE, 0x02, 0); // write CH1 and CH2

    // DCI-P3 to Rec.709 color space conversion matrix
    // Matrix coefficients converted to fixed-point format
    dvC341_WriteToBuffer(B12_OCSC2CF00CH1, 0x3A83, 0); // 1.22491 * 2^14
    dvC341_WriteToBuffer(B12_OCSC2CF01CH1, 0xE671, 0); // -0.224915 * 2^14
    dvC341_WriteToBuffer(B12_OCSC2CF02CH1, 0x0000, 0); // 0.0 * 2^14
    dvC341_WriteToBuffer(B12_OCSC2CF10CH1, 0xFD54, 0); // -0.0420532 * 2^14
    dvC341_WriteToBuffer(B12_OCSC2CF11CH1, 0x4295, 0); // 1.04205 * 2^14
    dvC341_WriteToBuffer(B12_OCSC2CF12CH1, 0x0000, 0); // 0.0 * 2^14
    dvC341_WriteToBuffer(B12_OCSC2CF20CH1, 0xFCE0, 0); // -0.0195923 * 2^14
    dvC341_WriteToBuffer(B12_OCSC2CF21CH1, 0xEBF5, 0); // -0.0786133 * 2^14
    dvC341_WriteToBuffer(B12_OCSC2CF22CH1, 0x4638, 0); // 1.09827 * 2^14

    dvC341_Write(BN_ACCMODE, 0x00, 0);

    return;
}


void dvC341_HLG_BT2020(void)
{

    //dvC341_WriteToBuffer(B0_ICFMTCH2,  0x0a, 0);

    if(HDR_Demo_SplitScreenMode_Get() == FALSE)
    {
        dvC341_WriteToBuffer(B0_ICFMTCH1, 0x09, 0);
        dvC341_WriteToBuffer(B0_ICFMTCH2, 0x09, 0);
        dvC341_WriteToBuffer(B0_OCFMTCH1, 0xB1, 0);
        dvC341_WriteToBuffer(B0_OCFMTCH2, 0xB1, 0);
	    dvC341_Write(BN_ACCMODE, 0x02, 0); // write CH1 and CH2

    }
    else
    {
        dvC341_WriteToBuffer(B0_ICFMTCH1, 0x09, 0);
        dvC341_WriteToBuffer(B0_OCFMTCH1, 0xB1, 0);
	    dvC341_Write(BN_ACCMODE, 0x00, 0); // write CH1
    }


    dvC341_WriteToBuffer(B13_OHLGYCCCTCH1,  0x01, 0);
    dvC341_WriteToBuffer(B13_OHLGYCCFT0CH1, 0x0868, 0);
    dvC341_WriteToBuffer(B13_OHLGYCCFT1CH1, 0x15b2, 0);
    dvC341_WriteToBuffer(B13_OHLGYCCFT2CH1, 0x01e6, 0);
    dvC341_WriteToBuffer(B13_OBGCT3CH1,     0x03, 0);
    dvC341_Write(BN_ACCMODE, 0x00, 0);


    //dvC341_WriteToBuffer(B12_OCSC1CF00CH1  ,  0x2f30, 0);
    //dvC341_WriteToBuffer(B12_OCSC1CF01CH1  ,  0x2000, 0);


    return;
}


void dvC341_HLG_BT2020_OnlyCH1(void)
{
    dvC341_WriteToBuffer(B0_ICFMTCH1,  0x09, 0);

    //dvC341_WriteToBuffer(B0_ICFMTCH2,  0x0a, 0);

    dvC341_WriteToBuffer(B13_OHLGYCCCTCH1,  0x01, 0);
    dvC341_WriteToBuffer(B13_OHLGYCCFT0CH1, 0x0868, 0);
    dvC341_WriteToBuffer(B13_OHLGYCCFT1CH1, 0x15b2, 0);
    dvC341_WriteToBuffer(B13_OHLGYCCFT2CH1, 0x01e6, 0);
    dvC341_WriteToBuffer(B13_OBGCT3CH1,     0x03, 0);

    dvC341_WriteToBuffer(B0_OCFMTCH1, 0xB1, 0);

    //dvC341_WriteToBuffer(B12_OCSC1CF00CH1  ,  0x2f30, 0);
    //dvC341_WriteToBuffer(B12_OCSC1CF01CH1  ,  0x2000, 0);

    return;
}



typedef struct
{
    UINT8 ucHDRDetect;  //Auto or Disable
    eHDR_EOTF_TYPE eEOTF_Type;
    eCM_HDR_GAMMA_FUNCTION eHDRGamma;  //eCM_HDR_GAMMA_FUNCTION
    eCM_HDR_MODE eHDRMode;  //auto (using mastering max nit) or manual
    UINT8 ucSDREnhance;
    UINT16 uiHDRContentMaxNit; //mastering max nit , or MaxCLL
    UINT16 uiMonitorMaxNit;
}sHDR_Config;


sHDR_Config sHDRConfig;


void dvC341_HDR_Init(void)
{
    sHDRConfig.ucHDRDetect = TRUE; //Auto
    sHDRConfig.eEOTF_Type = eHDR_EOTF_SDR;
    sHDRConfig.eHDRGamma = eCM_SDR_CUSTOM;
    sHDRConfig.eHDRMode = eCM_HDR_AUTO;
    sHDRConfig.ucSDREnhance = FALSE;
    sHDRConfig.uiHDRContentMaxNit = 1000;
    sHDRConfig.uiMonitorMaxNit = 1000;

    return;
}


void dvC341_HDR_Config(void)
{
    if(sHDRConfig.ucHDRDetect)
    {
        switch(sHDRConfig.eEOTF_Type)
        {
             case eHDR_EOTF_SDR:
                dvC341_HDR_Disable();
                break;

             case eHDR_EOTF_ST2084:   //HDR10
                dvC341_HDR_degamma_PQ(sHDRConfig.uiHDRContentMaxNit);
                dvC341_Panelgamma_BT709_Gamma22();
                dvC341_GamutBT2020ToREC709();
                break;

             case eHDR_EOTF_HLG:
                dvC341_degamma_HLG(1000, 1000);
                dvC341_HLG_y_lut(sHDRConfig.uiMonitorMaxNit);
                dvC341_HLG_BT2020();
                dvC341_Panelgamma_BT709_Gamma22();
                dvC341_GamutBT2020ToREC709();
                break;

             default:
                LOG_MSG(db_DV_SCALER, "(%s@%d) sHDRConfig.eEOTF_Type %d unknown", __func__, __LINE__, sHDRConfig.eEOTF_Type);
                break;
        }
    }
    else
    {
        //disable detect HDR, always SDR
        dvC341_HDR_Disable();
    }

    return;
}

void dvC341_HDR_Disable(void)
{
    dvC341_Write(BN_ACCMODE, 0x02, 0); // write CH1 and CH2

    //disable gamma
    dvC341_WriteToBuffer(B13_GMCTCH1,  0x00, 0);
    dvC341_WriteToBuffer(B13_GMCT2CH1, 0x00, 0);

    //disable hlg
    dvC341_WriteToBuffer(B13_OHLGYCCCTCH1,  0x00, 0);  //disable HLG Y gen
    dvC341_WriteToBuffer(B13_OHLGYLUTCTCH1, 0x00, 0);  //disable Y Lut (OOTF)
    dvC341_WriteToBuffer(B13_OBGCT3CH1,     0x00, 0);

    dvC341_Write(BN_ACCMODE, 0x00, 0);

    dvC341_WriteToBuffer(B0_OCFMTCH1,  0x11, 0);
    dvC341_WriteToBuffer(B0_OCFMTCH2,  0x11, 0);
    dvC341_WriteToBuffer(B0_ICFMTCH1,  0x18, 0);
    dvC341_WriteToBuffer(B0_ICFMTCH2,  0x18, 0);
    dvC341_Buffer_Flush();
}
//H2PF_Simon_0193 End

