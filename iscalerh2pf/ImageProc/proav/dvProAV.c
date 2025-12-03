#include <math.h>

//#include "dvC821.h"
//#include "dvPro_WarpLight.h"

#include "utilDbgMsg.h"
#include "dvProAV.h"
#include "dvProAV_OSD.h"
#include "dvProAV_Scaler.h"
#include "dvProAV_Interface.h"
#include "dvProAV_HdmiTx.h"
#include "dvProAV_HdmiRx.h"
#include "dvProAV_Warp.h"
#include "dvProAV_PixelShiftWith3d.h"


//#include "halC821CtrlAPI.h" //A70LV_Doulas_0002 test

#define BURST_MODE_FIXED_ADDRESS    (0x01 << 6)  //1 Serial burst mode: fixed address
#define BURST_MODE_AUTO_INC_ADDRESS (0x01 << 7)  //2 Serial burst mode: address automatic increment

#define SPI_BURST_MAX 0x204 //A70LV_Larry_0061
static UINT8 m_aucSPI_BurstBuffer[SPI_BURST_MAX] = {0}; //A70LV_Larry_0061

#ifdef Tx_Vx1_148M
static const PanelTiming m_sPanelTable[] =
{
    //Timing Id                 HTotal, VTotal, HSize, VSize, HStart, VStart, HSync, VSync, HPol, VPol, FrameRate, PixelClock
    {ePANEL_ID_3840x2160_60HZ,  4400,   2250,  3840,  2160,    384,     82,    88,    10,    1,   1,      6000,  594000000},
    //{ePANEL_ID_2688x1472_120HZ, 3080,   1624,  2688,  1472,    224,     106,   32,    5,     1,   1,     12000,  600000000}, // ProAV_Rex_0056
#ifdef CONFIG_4K_DISPLAY
    {ePANEL_ID_1080P_240HZ,     2200,   1125,  1920,  1080,    112,     37,    32,    5,     1,   1,     24000,  594000000},
    {ePANEL_ID_1080P_120HZ,     2640,   1875,  1920,  1080,    180,     68,    32,    5,     1,   1,     12000,  594000000},

//{ePANEL_ID_1080P_120HZ,     2640,   1875,  1920,  1080,    180,     68,    32,    32,    1,   1,     12000,  594400000},
    {ePANEL_ID_1080P_60HZ,      4768,   2078,  1920,  1080,    144,     34,    32,    36,    1,   1,      6000,  594400000},
#else
    {ePANEL_ID_3840x2160_30HZ,  4400,   2250,  3840,  2160,    384,     82,    88,    10,    1,   1,      3000,  297000000},
    {ePANEL_ID_1080P_120HZ,     2200,   1125,  1920,  1080,    192,     41,    21,    4,     1,   1,     12000,  297000000},
    {ePANEL_ID_1080P_240HZ,     2200,   1125,  1920,  1080,    192,     41,    21,    4,     1,   1,     24000,  594000000},
    {ePANEL_ID_1080P_60HZ,      2200,   1125,  1920,  1080,    192,     41,    21,    4,     1,   1,      6000,  148500000},
#endif /* CONFIG_4K_DISPLAY */
    //{ePANEL_ID_2560x1440_60HZ,  3232,   3094,  1920,  1080,    180,     68,    32,    32,    1,   1,      6000,  600000000},},
};
#else
//AB buffer mode, V start要大於9
//H Start, H Toatl要是8的倍數
static const PanelTiming m_sPanelTable[] =
{
    //Timing Id                 HTotal, VTotal, HSize, VSize, HStart, VStart, HSync, VSync, HPol, VPol, FrameRate, PixelClock

#ifdef MEMC_SINGLE_BOARD
    {ePANEL_ID_3840x2160_60HZ,  4472,   2240,  3840,  2160,    376,     64,    88,    10,    1,   1,      6000,  600000000},
#else
    {ePANEL_ID_3840x2400_60HZ,  4064,   2460,  3840,  2400,    80,     20,    64,    10,     1,   1,      6000,  600000000},
    {ePANEL_ID_3840x2160_60HZ,  4560,   2194,  3840,  2160,    384,    20,    96,    5,      1,   1,      6000,  600000000},
#endif /* MEMC_SINGLE_BOARD */

    {ePANEL_ID_1080P_120HZ,     3064,   1634,  1920,  1080,    224,     488,   32,    5,    1,   1,      12000,  600000000},
    {ePANEL_ID_1080P_240HZ,     2248,   1112,  1920,  1080,    192,     10,    92,    4,     1,   1,     24000,  600000000},
    //{ePANEL_ID_1080P_240HZ,     2280,   1096,  1920,  1080,    184,     5,     92,    4,     1,   1,     24000,  600000000},

    {ePANEL_ID_WUXGA_120HZ, 	3064,	1634,  1920,  1200,    224, 	368,   32,	  5,	1,	 1, 	 12000,  600000000}, //H60_Doulas_0003
	{ePANEL_ID_WUXGA_240HZ, 	2032,	1230,  1920,  1200,     40, 	10,    32,	  5,	1,   1,	 	 24000,  600000000}, //H60_Doulas_0003

    {ePANEL_ID_1080P_60HZ,      3232,   3094,  1920,  1080,    180,     68,    32,    32,    1,   1,      6000,  600000000},
    {ePANEL_ID_2688x1472_120HZ, 3080,   1624,  2688,  1472,    224,     106,   32,    5,     1,   1,     12000,  600000000}, // ProAV_Rex_0056
    {ePANEL_ID_1920x2400_60HZ,  4064,   2460,  1920,  2400,    80,      20,    32,    10,     1,   1,     6000,  600000000},


    //{ePANEL_ID_2560x1440_60HZ,  {3232,   3094,  1920,  1080,    180,     68,    32,    32,    1,   1,      6000,  600000000},},
};

static const PanelTiming m_sSinglePanelTable[] =
{
    //Timing Id                 HTotal, VTotal, HSize, VSize, HStart, VStart, HSync, VSync, HPol, VPol, FrameRate, PixelClock
    {ePANEL_ID_3840x2400_60HZ,  4064,   2460,  3840,  2400,    80,     20,    64,    10,     1,   1,      6000,  600000000},
    {ePANEL_ID_3840x2160_60HZ,  4560,   2194,  3840,  2160,    384,     20,    88,    5,     1,   1,      6000,  600000000},
    {ePANEL_ID_1080P_120HZ,     2200,   1125,  1920,  1080,    192,     41,    21,    4,     1,   1,     12000,  297000000}, //for debug
    {ePANEL_ID_1080P_240HZ,     2200,   1125,  1920,  1080,    192,     41,    21,    4,     1,   1,     24000,  594000000}, //for debug
    {ePANEL_ID_WUXGA_120HZ, 	3064,	1634,  1920,  1200,    224, 	368,   32,	  5,	1,	 1, 	 12000,  600000000}, //H60_Doulas_0003
	{ePANEL_ID_WUXGA_240HZ, 	2032,	1230,  1920,  1200,     40, 	10,    32,	  5,	1,   1,	 	 24000,  600000000}, //H60_Doulas_0003
};

#endif /* 0 */

#define PANEL_TABLE_NUMBER          sizeof(m_sPanelTable) / sizeof(PanelTiming)
#define SINGLE_PANEL_TABLE_NUMBER   sizeof(m_sSinglePanelTable) / sizeof(PanelTiming)

static UINT8 m_ucCurrentBank;
static TickType_t m_ulStartTicks;

static sCHANNEL_INFO m_sChannelInfo[eMCT_LAST];
static BOOL  bForcedSyncResetEnable = FALSE;    //A70LV_Doulas_0079
static UINT16 uiForcedSyncResetH_Total;     //A70LV_Doulas_0079
static UINT16 uiForcedSyncResetV_Total;     //A70LV_Doulas_0079

UINT32 IMFH_MCLK_COUNT[eMCT_LAST];     //A70LV_Doulas_0004
UINT32 IMFV_MCLK_COUNT[eMCT_LAST];     //A70LV_Doulas_0004

UINT8  ucNo_Signal_Config[eMCT_LAST] = {0,0};//A70LV_Doulas_0004

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

/*
//TODO
static INT32 PM_FSYNRST;
static INT32 PM_PANEL;
static INT32 PM_GAMMA[4];

static INT32 PS_REG_OHSCT;
static INT32 PS_REG_OVSCT;

static INT32 PS_PANEL_HST;
static INT32 PS_PANEL_HW;
static INT32 PS_PANEL_VST;
static INT32 PS_PANEL_VW;
static INT32 PS_PANEL_1CH_HW;

static INT32 PS_OCH_SWP;
static INT32 PS_OIDUALCTCH12;
static INT32 PS_OIDUALCTCH34;
*/

static sCHANNEL_SETTING             m_sChannelSetting[eMCT_LAST];   //A70LV_Doulas_0013
static UINT8 m_AutoImage[eCM_SOURCE_NUMBER] = {eAUTO_IMAGE_WIDE}; //A70LV_Doulas_0112
static UINT8 WorkingHorzPosition[eMCT_LAST];        //A70LV_Doulas_0113
static UINT8 WorkingVertPosition[eMCT_LAST];        //A70LV_Doulas_0113
static UINT8 ucInput3D_Format = eINPUT_3D_TYPE_OFF;//A70LV_Doulas_0154
static UINT8  m_ucModeTableID[2] = {0xFF,0xFF};     //A70LV_Doulas_0195
static UINT16 m_BestModeTableNumber[2] = {0,0};     //A70LV_Doulas_0195
static UINT16 uiModeAdjusmenttHorzStart = 0;        //A70LV_Doulas_0195
static UINT16 uiModeAdjusmenttVertStart = 0;        //A70LV_Doulas_0195
static BOOL   bModeAdjustmentEnable = FALSE;        //A70LV_Doulas_0195
static BOOL   bModeAdjustmentTableUse = FALSE;      //A70LV_Doulas_0195
static sTIMING_TABLE m_sModeAdjustmentTimingTable;               //A70LV_Doulas_0195
static char cSemaphore[256] = {'\0'};

BOOL dvProAV_SemaphoreTake(BOOL cEnable, const char *pcFunc) //A70LV_Larry_0052
{
    BOOL bResult = FALSE;

    if(cEnable)
    {
        bResult = dvProAV_InterfaceMutexTake();
        if(bResult == FALSE)
        {
            LOG_MSG(db_ALWAYS, "(func:%s) Take is Fail, (func:%s) is locked\r\n", pcFunc, cSemaphore);
        }
        else
        {
            sprintf(cSemaphore, "%s", pcFunc);
        }
    }
    else
    {
        bResult = dvProAV_InterfaceMutexGive();

        sprintf(cSemaphore, "");
    }

    return bResult;
}

static void dvPro_ResetTimeInState(void)
{
    m_ulStartTicks = xTaskGetTickCount();
}

static UINT32 dvPro_TimeElapsedInState(void)
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


int dvProAV_AccessRaw_Read(UINT8 bank, UINT8 offset, UINT16 len, UINT8 *data)
{
    int result = 0;

    if(dvProAV_SemaphoreTake(TRUE, __FUNCTION__))
    {
        result = dvProAV_AccessRawDataRead(bank, offset, len, data);
        dvProAV_SemaphoreTake(FALSE, __FUNCTION__);
    }

    return result;

}

int dvProAV_AccessRaw_Write(UINT8 bank, UINT8 offset, UINT16 len, UINT8 *data)
{
    int result = 0;

    if(dvProAV_SemaphoreTake(TRUE, __FUNCTION__))
    {
        result = dvProAV_AccessRawDataWrite(bank, offset, len, data);
        dvProAV_SemaphoreTake(FALSE, __FUNCTION__);
    }

    return result;

}

int dvProAV_Access_Write(UINT32 regEnum, UINT32 data)
{
    int result = 0;

    if(dvProAV_SemaphoreTake(TRUE, __FUNCTION__))
    {
        result = dvProAV_AccessWrite(regEnum, data);
        dvProAV_SemaphoreTake(FALSE, __FUNCTION__);
    }

    return result;

}

int dvProAV_Access_Read(UINT32 regEnum, UINT32 *data)
{
    int result = 0;

    if(dvProAV_SemaphoreTake(TRUE, __FUNCTION__))
    {
        result = dvProAV_AccessRead(regEnum, (uint32 *)data);
        dvProAV_SemaphoreTake(FALSE, __FUNCTION__);
    }

    return result;

}

void dvPro_reg_set(int ADDR,int val)  //A70LV_Doulas_0002
{
    if(m_sChannelInfo[eMCT_CH1].ePanelTimingId == PANEL_3D_OUTPUT)     //A70LV_Doulas_0154 modify //A70LV_Doulas_0098
    {
        //dvPro_Write(ADDR,val,13);
    }
	else if(m_sChannelSetting[eMCT_CH1].cPIP_PBP_ON == FALSE)
    {
		//dvPro_Write(ADDR,0x00,13);
	}
	else
    {                   // !Async
		//dvPro_Write(ADDR,val,13);
	}

	return;
}

static UINT32 dvPro_WaitPstat(eMCT eCH)
{
    UINT32 ulErr = 0;

    if(eCH >= eMCT_LAST)
    {
        return 1;
    }

    dvPro_ResetTimeInState();

    switch(eCH)
    {
        case eMCT_CH1:

#if 0
            while(dvPro_Read(B35_PSTATCH1, 0) != 0x02)
            {
                if(dvPro_TimeElapsedInState() >= DEF_WAIT_TIMEOUT)
                {
                    //					printf("Time out %f sec: wait_pstat CH%d\n", ((double)(tm_now - tm_start) / CLOCKS_PER_SEC), (ch + 1));
                    ulErr = 1;// timeout
                    break;
                }
            }
#endif /* 0 */

            break;

        case eMCT_CH2:

#if 0
            while(dvPro_Read(B35_PSTATCH2, 0) != 0x02)
            {
                if(dvPro_TimeElapsedInState() >= DEF_WAIT_TIMEOUT)
                {
                    //					printf("Time out %f sec: wait_pstat CH%d\n", ((double)(tm_now - tm_start) / CLOCKS_PER_SEC), (ch + 1));
                    ulErr = 1;// timeout
                    break;
                }
            }
#endif /* 0 */

            break;

        default:
            break;
    }

    return ulErr;
}



///////////////////////////////////////////////////////////////////////////////////
///@brief dvPro_WaitIntct : Wait PIVS
///@param ulCt - Interrupt Enable
///@return UINT32 - Return 1 = Timeout, 0 = Interrupt
///@author Roger Chen
///////////////////////////////////////////////////////////////////////////////////

UINT32 dvPro_WaitIntct(const eMCT eCH, const UINT32 ulCt)
{
    UINT32 ulRdt;
    UINT32 ulErr = 0;

    if(eCH >= eMCT_LAST)
    {
        return 1;
    }

    m_sChannelInfo[eCH].ulInterruptEnable |= ulCt;

    //dvPro_Write(B1_INTEN, (m_sChannelInfo[eCH].ulInterruptEnable), 0);
    //dvPro_Write(B1_INTRST, ulCt, 0); //A70LV_Doulas_0002 test
#if 0
    dvPro_ResetTimeInState();

    ulRdt = dvPro_Read(B1_INTSTAT, 0) & ulCt;

    while(ulRdt != ulCt)
    {
        if(dvPro_TimeElapsedInState() >= DEF_WAIT_TIMEOUT)
        {
            //			printf("Time out %f sec: wait_intct %d\n", ((double)(tm_now - tm_start) / CLOCKS_PER_SEC), (~rdt) & ct);
            ulErr = (~ulRdt) & ulCt;// timeout
            break;
        }

        ulRdt = dvPro_Read(B1_INTSTAT, 0) & ulCt;
    }
#endif /* 0 */

    m_sChannelInfo[eCH].ulInterruptEnable &= (~ulCt);
    //dvPro_Write(B1_INTEN, (m_sChannelInfo[eCH].ulInterruptEnable), 0);

    return ulErr;
}

void dvPro_SetBrightness(const eMCT eCH) //A70LV_Doulas_0022 //A70LV_Doulas_0011 modify
{
    UINT16 uiBrightness = 0;

    if(eCH >= eMCT_LAST)
    {
        return;
    }

    if(m_sChannelSetting[eMCT_CH1].cPIP_PBP_ON == FALSE)
    {
        if(m_sChannelInfo[eMCT_CH1].ePanelTimingId == PANEL_3D_OUTPUT)    //A70LV_Doulas_0155 modify
        {

        }
        else if(eCH == eMCT_CH2)
        {
            return ;
        }
    }


    uiBrightness = (UINT16)(m_sChannelSetting[eCH].iBrightness);

    dvProAV_SclVopBkeBrightnessSet(eCH, uiBrightness);

}

void dvPro_SetMatrix(eMCT eCH, eCOLOR_FORMAT eOutformat)
{
    DOUBLE dColor;
    DOUBLE dGain_R;
    DOUBLE dGain_G;
    DOUBLE dGain_B;
    UINT16 uiHue;

    UINT8 ucIndex1, ucIndex2;
    UINT8 ucRegOffset = 0;

    DOUBLE dYuvRgb1[3][3];
    DOUBLE dYuvRgb2[3][3];
    DOUBLE dYuvRgb3[3][3];
    DOUBLE dYuvRgb4[3][3];
    DOUBLE dMatrix1[3][3];
    DOUBLE dMatrix2[3][3];
    DOUBLE dMatrix3[3][3];
    DOUBLE dMatrix4[3][3];
    DOUBLE dMatrix5[3][3];

    UINT16 uiCoef_RR;
    UINT16 uiCoef_RG;
    UINT16 uiCoef_RB;
    UINT16 uiCoef_GR;
    UINT16 uiCoef_GG;
    UINT16 uiCoef_GB;
    UINT16 uiCoef_BR;
    UINT16 uiCoef_BG;
    UINT16 uiCoef_BB;

    if(eCH >= eMCT_LAST)
    {
        return;
    }


    //  color(0-3.99) for Saturation;  gain(0-3.99) for Contrast
    //  hue(-180-180) for Hue
    //  ch=0 Channel1, ch=1 Channel2, ch=2 Channel3,ch=3 Channel4
    //  iformat=0, RGB input;  iformat=1, YUV input;

    dGain_R = m_sChannelSetting[eCH].dContrast;
    dGain_G = m_sChannelSetting[eCH].dContrast;
    dGain_B = m_sChannelSetting[eCH].dContrast;
    dColor = m_sChannelSetting[eCH].dColor;
    uiHue = m_sChannelSetting[eCH].iHue;


    // 709 Matrix
    if(eOutformat == eCOLOR_FORMAT_RGB)     // RGB input
    {
        dMatrix1[0][0] =  0.2126;
        dMatrix1[1][0] = -0.1146;
        dMatrix1[2][0] =  0.5;
        dMatrix1[0][1] =  0.7152;
        dMatrix1[1][1] = -0.3854;
        dMatrix1[2][1] = -0.4542;
        dMatrix1[0][2] =  0.0722;
        dMatrix1[1][2] =  0.5;
        dMatrix1[2][2] = -0.0458;
    }
    else    // YUV input
    {
        dMatrix1[0][0] = 1.0;
        dMatrix1[1][0] = 0;
        dMatrix1[2][0] = 0;
        dMatrix1[0][1] = 0;
        dMatrix1[1][1] = 1.0;
        dMatrix1[2][1] = 0;
        dMatrix1[0][2] = 0;
        dMatrix1[1][2] = 0;
        dMatrix1[2][2] = 1.0;
    }

    // Saturation Matrix
    dMatrix2[0][0] = 1.0;
    dMatrix2[1][0] = 0;
    dMatrix2[2][0] = 0;
    dMatrix2[0][1] = 0;
    dMatrix2[1][1] = dColor;
    dMatrix2[2][1] = 0;
    dMatrix2[0][2] = 0;
    dMatrix2[1][2] = 0;
    dMatrix2[2][2] = dColor;

    // YUV->RGB
    if(eOutformat == eCOLOR_FORMAT_RGB)     // RGB input
    {
        dMatrix3[0][0] =  1.0;
        dMatrix3[1][0] =  1.0;
        dMatrix3[2][0] =  1.0;
        dMatrix3[0][1] =  0.0;
        dMatrix3[1][1] = -0.187255859;
        dMatrix3[2][1] =  1.85550982;
        dMatrix3[0][2] =  1.574829102;
        dMatrix3[1][2] = -0.468139648;
        dMatrix3[2][2] =  0.0;
    }
    else
    {
        dMatrix3[0][0] =  1.167808219;
        dMatrix3[1][0] =  1.167808219;
        dMatrix3[2][0] =  1.167808219;
        dMatrix3[0][1] =  0.0;
        dMatrix3[1][1] = -0.187255859;
        dMatrix3[2][1] =  1.85550982;
        dMatrix3[0][2] =  1.574829102;
        dMatrix3[1][2] = -0.468139648;
        dMatrix3[2][2] =  0.0;
    }

    // Contrast Matrix
    dMatrix4[0][0] = dGain_R;
    dMatrix4[1][0] = 0;
    dMatrix4[2][0] = 0;
    dMatrix4[0][1] = 0;
    dMatrix4[1][1] = dGain_G;
    dMatrix4[2][1] = 0;
    dMatrix4[0][2] = 0;
    dMatrix4[1][2] = 0;
    dMatrix4[2][2] = dGain_B;


    // Hue
    dMatrix5[0][0] =  1.0;
    dMatrix5[1][0] =  0;
    dMatrix5[2][0] =  0;
    dMatrix5[0][1] =  0;
    dMatrix5[1][1] =  cos(uiHue * 3.14159 / 180);
    dMatrix5[2][1] = -sin(uiHue * 3.14159 / 180);
    dMatrix5[0][2] =  0;
    dMatrix5[1][2] =  sin(uiHue * 3.14159 / 180);
    dMatrix5[2][2] =  cos(uiHue * 3.14159 / 180);

    dYuvRgb1[0][0] = dMatrix2[0][0] * dMatrix1[0][0] + dMatrix2[0][1] * dMatrix1[1][0] + dMatrix2[0][2] * dMatrix1[2][0];
    dYuvRgb1[0][1] = dMatrix2[0][0] * dMatrix1[0][1] + dMatrix2[0][1] * dMatrix1[1][1] + dMatrix2[0][2] * dMatrix1[2][1];
    dYuvRgb1[0][2] = dMatrix2[0][0] * dMatrix1[0][2] + dMatrix2[0][1] * dMatrix1[1][2] + dMatrix2[0][2] * dMatrix1[2][2];
    dYuvRgb1[1][0] = dMatrix2[1][0] * dMatrix1[0][0] + dMatrix2[1][1] * dMatrix1[1][0] + dMatrix2[1][2] * dMatrix1[2][0];
    dYuvRgb1[1][1] = dMatrix2[1][0] * dMatrix1[0][1] + dMatrix2[1][1] * dMatrix1[1][1] + dMatrix2[1][2] * dMatrix1[2][1];
    dYuvRgb1[1][2] = dMatrix2[1][0] * dMatrix1[0][2] + dMatrix2[1][1] * dMatrix1[1][2] + dMatrix2[1][2] * dMatrix1[2][2];
    dYuvRgb1[2][0] = dMatrix2[2][0] * dMatrix1[0][0] + dMatrix2[2][1] * dMatrix1[1][0] + dMatrix2[2][2] * dMatrix1[2][0];
    dYuvRgb1[2][1] = dMatrix2[2][0] * dMatrix1[0][1] + dMatrix2[2][1] * dMatrix1[1][1] + dMatrix2[2][2] * dMatrix1[2][1];
    dYuvRgb1[2][2] = dMatrix2[2][0] * dMatrix1[0][2] + dMatrix2[2][1] * dMatrix1[1][2] + dMatrix2[2][2] * dMatrix1[2][2];

    dYuvRgb2[0][0] = dMatrix5[0][0] * dYuvRgb1[0][0] + dMatrix5[0][1] * dYuvRgb1[1][0] + dMatrix5[0][2] * dYuvRgb1[2][0];
    dYuvRgb2[0][1] = dMatrix5[0][0] * dYuvRgb1[0][1] + dMatrix5[0][1] * dYuvRgb1[1][1] + dMatrix5[0][2] * dYuvRgb1[2][1];
    dYuvRgb2[0][2] = dMatrix5[0][0] * dYuvRgb1[0][2] + dMatrix5[0][1] * dYuvRgb1[1][2] + dMatrix5[0][2] * dYuvRgb1[2][2];
    dYuvRgb2[1][0] = dMatrix5[1][0] * dYuvRgb1[0][0] + dMatrix5[1][1] * dYuvRgb1[1][0] + dMatrix5[1][2] * dYuvRgb1[2][0];
    dYuvRgb2[1][1] = dMatrix5[1][0] * dYuvRgb1[0][1] + dMatrix5[1][1] * dYuvRgb1[1][1] + dMatrix5[1][2] * dYuvRgb1[2][1];
    dYuvRgb2[1][2] = dMatrix5[1][0] * dYuvRgb1[0][2] + dMatrix5[1][1] * dYuvRgb1[1][2] + dMatrix5[1][2] * dYuvRgb1[2][2];
    dYuvRgb2[2][0] = dMatrix5[2][0] * dYuvRgb1[0][0] + dMatrix5[2][1] * dYuvRgb1[1][0] + dMatrix5[2][2] * dYuvRgb1[2][0];
    dYuvRgb2[2][1] = dMatrix5[2][0] * dYuvRgb1[0][1] + dMatrix5[2][1] * dYuvRgb1[1][1] + dMatrix5[2][2] * dYuvRgb1[2][1];
    dYuvRgb2[2][2] = dMatrix5[2][0] * dYuvRgb1[0][2] + dMatrix5[2][1] * dYuvRgb1[1][2] + dMatrix5[2][2] * dYuvRgb1[2][2];

    dYuvRgb3[0][0] = dMatrix3[0][0] * dYuvRgb2[0][0] + dMatrix3[0][1] * dYuvRgb2[1][0] + dMatrix3[0][2] * dYuvRgb2[2][0];
    dYuvRgb3[0][1] = dMatrix3[0][0] * dYuvRgb2[0][1] + dMatrix3[0][1] * dYuvRgb2[1][1] + dMatrix3[0][2] * dYuvRgb2[2][1];
    dYuvRgb3[0][2] = dMatrix3[0][0] * dYuvRgb2[0][2] + dMatrix3[0][1] * dYuvRgb2[1][2] + dMatrix3[0][2] * dYuvRgb2[2][2];
    dYuvRgb3[1][0] = dMatrix3[1][0] * dYuvRgb2[0][0] + dMatrix3[1][1] * dYuvRgb2[1][0] + dMatrix3[1][2] * dYuvRgb2[2][0];
    dYuvRgb3[1][1] = dMatrix3[1][0] * dYuvRgb2[0][1] + dMatrix3[1][1] * dYuvRgb2[1][1] + dMatrix3[1][2] * dYuvRgb2[2][1];
    dYuvRgb3[1][2] = dMatrix3[1][0] * dYuvRgb2[0][2] + dMatrix3[1][1] * dYuvRgb2[1][2] + dMatrix3[1][2] * dYuvRgb2[2][2];
    dYuvRgb3[2][0] = dMatrix3[2][0] * dYuvRgb2[0][0] + dMatrix3[2][1] * dYuvRgb2[1][0] + dMatrix3[2][2] * dYuvRgb2[2][0];
    dYuvRgb3[2][1] = dMatrix3[2][0] * dYuvRgb2[0][1] + dMatrix3[2][1] * dYuvRgb2[1][1] + dMatrix3[2][2] * dYuvRgb2[2][1];
    dYuvRgb3[2][2] = dMatrix3[2][0] * dYuvRgb2[0][2] + dMatrix3[2][1] * dYuvRgb2[1][2] + dMatrix3[2][2] * dYuvRgb2[2][2];

    dYuvRgb4[0][0] = dMatrix4[0][0] * dYuvRgb3[0][0] + dMatrix4[0][1] * dYuvRgb3[1][0] + dMatrix4[0][2] * dYuvRgb3[2][0];
    dYuvRgb4[0][1] = dMatrix4[0][0] * dYuvRgb3[0][1] + dMatrix4[0][1] * dYuvRgb3[1][1] + dMatrix4[0][2] * dYuvRgb3[2][1];
    dYuvRgb4[0][2] = dMatrix4[0][0] * dYuvRgb3[0][2] + dMatrix4[0][1] * dYuvRgb3[1][2] + dMatrix4[0][2] * dYuvRgb3[2][2];
    dYuvRgb4[1][0] = dMatrix4[1][0] * dYuvRgb3[0][0] + dMatrix4[1][1] * dYuvRgb3[1][0] + dMatrix4[1][2] * dYuvRgb3[2][0];
    dYuvRgb4[1][1] = dMatrix4[1][0] * dYuvRgb3[0][1] + dMatrix4[1][1] * dYuvRgb3[1][1] + dMatrix4[1][2] * dYuvRgb3[2][1];
    dYuvRgb4[1][2] = dMatrix4[1][0] * dYuvRgb3[0][2] + dMatrix4[1][1] * dYuvRgb3[1][2] + dMatrix4[1][2] * dYuvRgb3[2][2];
    dYuvRgb4[2][0] = dMatrix4[2][0] * dYuvRgb3[0][0] + dMatrix4[2][1] * dYuvRgb3[1][0] + dMatrix4[2][2] * dYuvRgb3[2][0];
    dYuvRgb4[2][1] = dMatrix4[2][0] * dYuvRgb3[0][1] + dMatrix4[2][1] * dYuvRgb3[1][1] + dMatrix4[2][2] * dYuvRgb3[2][1];
    dYuvRgb4[2][2] = dMatrix4[2][0] * dYuvRgb3[0][2] + dMatrix4[2][1] * dYuvRgb3[1][2] + dMatrix4[2][2] * dYuvRgb3[2][2];

    dYuvRgb4[0][0] = dYuvRgb4[0][0] * 8192;// pow((double)2.0,(double)13.0) = 8192
    dYuvRgb4[1][0] = dYuvRgb4[1][0] * 8192;
    dYuvRgb4[2][0] = dYuvRgb4[2][0] * 8192;
    dYuvRgb4[0][1] = dYuvRgb4[0][1] * 8192;
    dYuvRgb4[1][1] = dYuvRgb4[1][1] * 8192;
    dYuvRgb4[2][1] = dYuvRgb4[2][1] * 8192;
    dYuvRgb4[0][2] = dYuvRgb4[0][2] * 8192;
    dYuvRgb4[1][2] = dYuvRgb4[1][2] * 8192;
    dYuvRgb4[2][2] = dYuvRgb4[2][2] * 8192;

    for(ucIndex2 = 0; ucIndex2 < 3; ucIndex2++)
    {
        for(ucIndex1 = 0; ucIndex1 < 3; ucIndex1++)
        {
            if(dYuvRgb4[ucIndex1][ucIndex2] >= 0)
            {
                if(dYuvRgb4[ucIndex1][ucIndex2] >= 32767)
                {
                    dYuvRgb4[ucIndex1][ucIndex2] = 0x7FFF;
                }
                else
                {
                    dYuvRgb4[ucIndex1][ucIndex2] = 0x0000FFFF & (INT32)(dYuvRgb4[ucIndex1][ucIndex2] + 0.5);
                }
            }
            else
            {
                if(dYuvRgb4[ucIndex1][ucIndex2] <= -32768)
                {
                    dYuvRgb4[ucIndex1][ucIndex2] = 0x8000;
                }
                else
                {
                    dYuvRgb4[ucIndex1][ucIndex2] = 0x0000FFFF & (INT32)(floor(dYuvRgb4[ucIndex1][ucIndex2] - 0.5));
                }
            }
        }
    }

    if(eOutformat == eCOLOR_FORMAT_RGB)     // RGB input
    {
        uiCoef_RR = (int)(dYuvRgb4[0][0]) & 0xFFFF;
        uiCoef_GR = (int)(dYuvRgb4[1][0]) & 0xFFFF;
        uiCoef_BR = (int)(dYuvRgb4[2][0]) & 0xFFFF;
        uiCoef_RG = (int)(dYuvRgb4[0][1]) & 0xFFFF;
        uiCoef_GG = (int)(dYuvRgb4[1][1]) & 0xFFFF;
        uiCoef_BG = (int)(dYuvRgb4[2][1]) & 0xFFFF;
        uiCoef_RB = (int)(dYuvRgb4[0][2]) & 0xFFFF;
        uiCoef_GB = (int)(dYuvRgb4[1][2]) & 0xFFFF;
        uiCoef_BB = (int)(dYuvRgb4[2][2]) & 0xFFFF;
    }
    else    // YUV input
    {
        uiCoef_RG = (int)(dYuvRgb4[0][0]) & 0xFFFF;
        uiCoef_GG = (int)(dYuvRgb4[1][0]) & 0xFFFF;
        uiCoef_BG = (int)(dYuvRgb4[2][0]) & 0xFFFF;
        uiCoef_RB = (int)(dYuvRgb4[0][1]) & 0xFFFF;
        uiCoef_GB = (int)(dYuvRgb4[1][1]) & 0xFFFF;
        uiCoef_BB = (int)(dYuvRgb4[2][1]) & 0xFFFF;
        uiCoef_RR = (int)(dYuvRgb4[0][2]) & 0xFFFF;
        uiCoef_GR = (int)(dYuvRgb4[1][2]) & 0xFFFF;
        uiCoef_BR = (int)(dYuvRgb4[2][2]) & 0xFFFF;
    }

    ucRegOffset = (eCH == eMCT_CH1) ? 0 : 13;


    //dvPro_Write(B0_RTCT, RTCT_POVSSTOP, 0);   //0x4321FFFF

    //dvPro_Write(B6_OCSC1CF00CH1, uiCoef_RR, ucRegOffset);    //COEFrr
    //dvPro_Write(B6_OCSC1CF22CH1, uiCoef_BB, ucRegOffset);    //COEFbb
    //dvPro_Write(B6_OCSC1CF10CH1, uiCoef_GR, ucRegOffset);    //COEFgr
    //dvPro_Write(B6_OCSC1CF12CH1, uiCoef_GB, ucRegOffset);    //COEFgb
    //dvPro_Write(B6_OCSC1CF02CH1, uiCoef_RB, ucRegOffset);    //COEFrb
    //dvPro_Write(B6_OCSC1CF20CH1, uiCoef_BR, ucRegOffset);    //COEFbr
    //dvPro_Write(B6_OCSC1CF01CH1, uiCoef_RG, ucRegOffset);    //COEFrg
    //dvPro_Write(B6_OCSC1CF21CH1, uiCoef_BG, ucRegOffset);    //COEFbg
    //dvPro_Write(B6_OCSC1CF11CH1, uiCoef_GG, ucRegOffset);    //COEFgg

    if(eOutformat == eCOLOR_FORMAT_RGB)
    {
        if(eCH == eMCT_CH1)
        {
            //dvPro_Write(B0_OCFMTCH1, 0x10, 0);
        }
        else if(eCH == eMCT_CH2)
        {
            //dvPro_Write(B0_OCFMTCH2, 0x10, 0);
        }
    }
    else
    {
        if(eCH == eMCT_CH1)
        {
            //dvPro_Write(B0_OCFMTCH1, 0x11, 0);
        }
        else if(eCH == eMCT_CH2)
        {
            //dvPro_Write(B0_OCFMTCH2, 0x11, 0);
        }
    }

    //dvPro_Write(B0_RTCT, RTCT_NORMAL, 0);   //0x4321CBA9

}

static void dvPro_SetGamma2(eMCT eCH)
{
    UINT8 ucRegOffset = 0;

    if(eCH >= eMCT_LAST)
    {
        return;
    }

    ucRegOffset = (eCH == eMCT_CH1) ? 0 : 13;

    //dvPro_Write(B0_RTCT, RTCT_POVSSTOP, 0);   //0x4321FFFF

    if(m_sChannelSetting[eCH].cGamma > 0)
    {
        //dvPro_Write(B6_GMBNKSEL21CH1, (m_sChannelSetting[eCH].cGamma - 1) & 0x1F, ucRegOffset);
        //dvPro_Write(B6_GMCT2CH1, 0x37, ucRegOffset);
    }
    else
    {
        //dvPro_Write(B6_GMBNKSEL21CH1, 0x00, ucRegOffset);
        //dvPro_Write(B6_GMCT2CH1, 0x36, ucRegOffset);
    }

    //dvPro_Write(B0_RTCT, RTCT_NORMAL, 0);   //0x4321CBA9

}


void dvPro_SetOutputColor_2K(const eMCT eCH, const eCOLOR_FORMAT eColorFmt)
{
    if(eColorFmt == eCOLOR_FORMAT_RGB)
    {
        //dvPro_SetBrightness(eCH, eCOLOR_FORMAT_RGB);   //A70LV_Doulas_0022
        dvPro_SetMatrix(eCH, eCOLOR_FORMAT_RGB);
    }
    else
    {
        //dvPro_SetBrightness(eCH, eCOLOR_FORMAT_422);     //A70LV_Doulas_0022
        dvPro_SetMatrix(eCH, eCOLOR_FORMAT_422);
    }

    dvPro_SetGamma2(eCH);
}

void dvPro_Set_Flip(const eMCT eCH)    //A70LV_Doulas_0002
{
	UINT32 wdt,val2;
//	UINT32	FLG_VSHRNK=0;
//	int mlessvflip=0;
	//UINT32 WOSFLD0,WOSFLD1,WOSFLD2,WOSFLD3;     //A70LV_Doulas_0154
	//UINT32 WISFLD0,WISFLD1,WISFLD2,WISFLD3;     //A70LV_Doulas_0154
//    UINT32 sPS_MEM_SC[2][4];  A70LV_Doulas_0098
    UINT8 ucRegOffset = 0;

    if(eCH >= eMCT_LAST)
    {
        return;
    }

    ucRegOffset = (eCH == eMCT_CH1) ? 0 : 13;
#if 0       //A70LV_Doulas_0154
//	FLG_VSHRNK = ( (PM_IWIN_VW[ch]*(PI_SCAN_MODE[ch]+1)) > PM_OWIN_VW[ch]) ? 1 : 0;
//-----------------------------------------------------------------------------
// Upsidedown Control
//-----------------------------------------------------------------------------
	//dvPro_Write(B34_OSFLD0,		    0x3998,0);	//Output field 0-3 memory read start address
	//dvPro_Write(B34_OSFLD1,		    0x3e58,0);
	//dvPro_Write(B34_OSFLD2,		    0x4318,0);
	//dvPro_Write(B34_OSFLD3,		    0x47d8,0);


	WOSFLD0= 0x3998; //1080p out
	WOSFLD1= 0x3e58;
	WOSFLD2= 0x4318;
	WOSFLD3= 0x47d8;
	WISFLD0=0x07330000;
    WISFLD1=0x07cb0000;
    WISFLD2=0x08630000;
    WISFLD3=0x08fb0000;
 //   mlessvflip = ((((PM_OWIN_VW[0]-1)/4)*32768)+(((PM_OWIN_VW[0]-1)%4)*16));
	                                 //  2^15 =  32768                 2^4=16
 //   printf("(PM_OWIN_VW[0]-1)/4= %d\n",(PM_OWIN_VW[0]-1)/4);
//	printf("(PM_OWIN_VW[0]-1)%4= %d\n",(PM_OWIN_VW[0]-1)%4);
 //  mlessvflip = 8814640;
 //  printf("mlessvflip= %x\n",mlessvflip);

    //dvPro_Write(B34_OSFLD0,WOSFLD0,0);
    //dvPro_Write(B34_OSFLD1,WOSFLD1,0);
    //dvPro_Write(B34_OSFLD2,WOSFLD2,0);
    //dvPro_Write(B34_OSFLD3,WOSFLD3,0);

    //dvPro_Write(B34_ISFLD0,WISFLD0,0);
    //dvPro_Write(B34_ISFLD1,WISFLD1,0);
    //dvPro_Write(B34_ISFLD2,WISFLD2,0);
    //dvPro_Write(B34_ISFLD3,WISFLD3,0);
    //dvPro_Write(B34_MWI,0x03,0);

#endif
//	if ( PM_UPSIDEDOWN[ch]==0 )
    {       //Ceiling Mount
//		val1 = 0;
		val2 = 0x20;//PS_MWI_SC[ch];
	}
/*    else
	{
		if( FLG_VSHRNK == 0)
        {
			val1 = PS_MWI_SC[ch]*256*(PM_IWIN_VW[ch]*(PI_SCAN_MODE[ch]+1)-1);
		}
		else
        {
			val1 = PS_MWI_SC[ch]*256*(PM_OWIN_VW[ch]*(1)-1);
		}
		val2 = (0x100 - PS_MWI_SC[ch])&0xff;
	}*/
/*	sPS_MEM_SC[0][0]= DEF_ISFLD0CH1_2K;
    sPS_MEM_SC[0][1]= DEF_ISFLD1CH1_2K;
    sPS_MEM_SC[0][2]= DEF_ISFLD2CH1_2K;
    sPS_MEM_SC[0][3]= DEF_ISFLD3CH1_2K;

    sPS_MEM_SC[1][0]= DEF_ISFLD0CH2_2K;
    sPS_MEM_SC[1][1]= DEF_ISFLD1CH2_2K;
    sPS_MEM_SC[1][2]= DEF_ISFLD2CH2_2K;
    sPS_MEM_SC[1][3]= DEF_ISFLD3CH2_2K;
 */

    if(eCH == eMCT_CH1)    //A70LV_Doulas_0098 modify
    {
        switch(dvPro_Input_3D_Format_Config_Get())        //A70LV_Doulas_0154 modify
        {
            case eINPUT_3D_TYPE_FRAMEPACKING:
 	            //dvPro_Write(B13_OSFLD0CH1,0x974000,ucRegOffset);
	            //dvPro_Write(B13_OSFLD1CH1,0x00,ucRegOffset);
	            //dvPro_Write(B13_OSFLD2CH1,0x430bc00,ucRegOffset);
	            //dvPro_Write(B13_OSFLD3CH1,0x3997c00,ucRegOffset);
                break;

            case eINPUT_3D_TYPE_SIDEBYSIDE:
	         //   //dvPro_Write(B13_OSFLD0CH1,0x974000,ucRegOffset);
	         //   //dvPro_Write(B13_OSFLD1CH1,0x000000,ucRegOffset);
	         //   //dvPro_Write(B13_OSFLD2CH1,0x974f00,ucRegOffset);
	         //   //dvPro_Write(B13_OSFLD3CH1,0x000f00,ucRegOffset);
                //A70LV_Doulas_0185 Modify
                //dvPro_Write(B13_OSFLD0CH1,0x974000,ucRegOffset);
	            //dvPro_Write(B13_OSFLD1CH1,0x000000,ucRegOffset);
	            //dvPro_Write(B13_OSFLD2CH1,0x0430bc00,ucRegOffset);
	            //dvPro_Write(B13_OSFLD3CH1,0x03997c00,ucRegOffset);
                break;

            case eINPUT_3D_TYPE_TOPANDBOTTOM:
	        //    //dvPro_Write(B13_OSFLD0CH1,0x974000,ucRegOffset);
	        //    //dvPro_Write(B13_OSFLD1CH1,0x000000,ucRegOffset);
	        //    //dvPro_Write(B13_OSFLD2CH1,0xc44000,ucRegOffset);
	        //    //dvPro_Write(B13_OSFLD3CH1,0x2d0000,ucRegOffset);
                //A70LV_Doulas_0185 modify
                //dvPro_Write(B13_OSFLD0CH1,0x00974000,ucRegOffset);
	            //dvPro_Write(B13_OSFLD1CH1,0x000000,ucRegOffset);
	            //dvPro_Write(B13_OSFLD2CH1,0x0430bc00,ucRegOffset);
	            //dvPro_Write(B13_OSFLD3CH1,0x03997c00,ucRegOffset);
                break;

            default:
	            //dvPro_Write(B13_OSFLD0CH1,DEF_ISFLD3CH1_2K,ucRegOffset);//if(ROT) PS_MEM_RT[ch][0] //Output field 0-3 memory read start address
	            //dvPro_Write(B13_OSFLD1CH1,DEF_ISFLD0CH1_2K,ucRegOffset);//if(ROT) PS_MEM_RT[ch][1]
	            //dvPro_Write(B13_OSFLD2CH1,DEF_ISFLD1CH1_2K,ucRegOffset);//if(ROT) PS_MEM_RT[ch][2]
	            //dvPro_Write(B13_OSFLD3CH1,DEF_ISFLD2CH1_2K,ucRegOffset);//if(ROT) PS_MEM_RT[ch][3]
	            break;
        }
    }
    else
    {
        switch(dvPro_Input_3D_Format_Config_Get())        //A70LV_Doulas_0154 modify
        {
            case eINPUT_3D_TYPE_FRAMEPACKING:
                //A70LV_Doulas_0185 modify
                if(m_sChannelInfo[eCH].sInputTimingInfo.dFrameRate > 23.0 && m_sChannelInfo[eCH].sInputTimingInfo.dFrameRate < 25.0)
                {
 	                //dvPro_Write(B13_OSFLD0CH1,0x974f00,ucRegOffset);
	                //dvPro_Write(B13_OSFLD1CH1,0x000f00,ucRegOffset);
	                //dvPro_Write(B13_OSFLD2CH1,0x430cb00,ucRegOffset);
	                //dvPro_Write(B13_OSFLD3CH1,0x3998b00,ucRegOffset);
                }
                else
                {
                    //A70LV_Doulas_0189 modify
 	                //dvPro_Write(B13_OSFLD0CH1,0x009749f0,ucRegOffset);
	                //dvPro_Write(B13_OSFLD1CH1,0x000009f0,ucRegOffset);
	                //dvPro_Write(B13_OSFLD2CH1,0x0430c5f0,ucRegOffset);
	                //dvPro_Write(B13_OSFLD3CH1,0x039985f0,ucRegOffset);
 	        //        //dvPro_Write(B13_OSFLD0CH1,0x00974a00,ucRegOffset);
	        //        //dvPro_Write(B13_OSFLD1CH1,0x00000a00,ucRegOffset);
	        //        //dvPro_Write(B13_OSFLD2CH1,0x0430c600,ucRegOffset);
	        //        //dvPro_Write(B13_OSFLD3CH1,0x03998600,ucRegOffset);
                }
                break;

            case eINPUT_3D_TYPE_SIDEBYSIDE:
                #if 0
 	            //dvPro_Write(B13_OSFLD0CH1,0x974770,ucRegOffset);
	            //dvPro_Write(B13_OSFLD1CH1,0x000770,ucRegOffset);
	            //dvPro_Write(B13_OSFLD2CH1,0x975670,ucRegOffset);
	            //dvPro_Write(B13_OSFLD3CH1,0x001670,ucRegOffset);
                #endif
                #if 1
 	            ////dvPro_Write(B13_OSFLD0CH1,0x9744fa,ucRegOffset); //self test
	            ////dvPro_Write(B13_OSFLD1CH1,0x0004fa,ucRegOffset);
	            ////dvPro_Write(B13_OSFLD2CH1,0x9753fa,ucRegOffset);
	            ////dvPro_Write(B13_OSFLD3CH1,0x0013fa,ucRegOffset);
 	       //     //dvPro_Write(B13_OSFLD0CH1,0x9744f4,ucRegOffset);
	       //     //dvPro_Write(B13_OSFLD1CH1,0x0004f4,ucRegOffset);
	       //     //dvPro_Write(B13_OSFLD2CH1,0x9753f4,ucRegOffset);
	       //     //dvPro_Write(B13_OSFLD3CH1,0x0013f4,ucRegOffset);
                //A70LV_Doulas_0185 modify
             //   //dvPro_Write(B13_OSFLD0CH1,0x00974770,ucRegOffset);
	         //   //dvPro_Write(B13_OSFLD1CH1,0x00000770,ucRegOffset);
	         //   //dvPro_Write(B13_OSFLD2CH1,0x0430c370,ucRegOffset);
	         //   //dvPro_Write(B13_OSFLD3CH1,0x03998370,ucRegOffset);
	            //A70LV_Doulas_0189 modify
           //     //dvPro_Write(B13_OSFLD0CH1,0x00974500,ucRegOffset);
	       //     //dvPro_Write(B13_OSFLD1CH1,0x00000500,ucRegOffset);
	       //     //dvPro_Write(B13_OSFLD2CH1,0x0430c100,ucRegOffset);
	       //     //dvPro_Write(B13_OSFLD3CH1,0x03998100,ucRegOffset);
                //dvPro_Write(B13_OSFLD0CH1,0x009744f4,ucRegOffset);
	            //dvPro_Write(B13_OSFLD1CH1,0x000004f4,ucRegOffset);
	            //dvPro_Write(B13_OSFLD2CH1,0x0430c0f4,ucRegOffset);
	            //dvPro_Write(B13_OSFLD3CH1,0x039980f4,ucRegOffset);
                #endif
                break;

            case eINPUT_3D_TYPE_TOPANDBOTTOM:
 	         //   //dvPro_Write(B13_OSFLD0CH1,0x9749f0,ucRegOffset);
	         //   //dvPro_Write(B13_OSFLD1CH1,0x0009f0,ucRegOffset);
	         //   //dvPro_Write(B13_OSFLD2CH1,0xc449f0,ucRegOffset);
	         //   //dvPro_Write(B13_OSFLD3CH1,0x2d09f0,ucRegOffset);
                //A70LV_Doulas_0185 modify
                if(m_sChannelInfo[eCH].sInputTimingInfo.dFrameRate > 23.0 && m_sChannelInfo[eCH].sInputTimingInfo.dFrameRate < 25.0)
                {
 	                //dvPro_Write(B13_OSFLD0CH1,0x00974f00,ucRegOffset);
	                //dvPro_Write(B13_OSFLD1CH1,0x00000f00,ucRegOffset);
	                //dvPro_Write(B13_OSFLD2CH1,0x0430cb00,ucRegOffset);
	                //dvPro_Write(B13_OSFLD3CH1,0x03998b00,ucRegOffset);
                }
                else
                {
                    //A70LV_Doulas_0189 modify
 	                //dvPro_Write(B13_OSFLD0CH1,0x009749f0,ucRegOffset);
	                //dvPro_Write(B13_OSFLD1CH1,0x000009f0,ucRegOffset);
	                //dvPro_Write(B13_OSFLD2CH1,0x0430c5f0,ucRegOffset);
	                //dvPro_Write(B13_OSFLD3CH1,0x039985f0,ucRegOffset);
 	         //       //dvPro_Write(B13_OSFLD0CH1,0x00974a00,ucRegOffset);
	         //       //dvPro_Write(B13_OSFLD1CH1,0x00000a00,ucRegOffset);
	         //       //dvPro_Write(B13_OSFLD2CH1,0x0430c600,ucRegOffset);
	         //       //dvPro_Write(B13_OSFLD3CH1,0x03998600,ucRegOffset);
                }
                break;

            default:
	            //dvPro_Write(B13_OSFLD0CH1,DEF_ISFLD3CH2_2K,ucRegOffset);//if(ROT) PS_MEM_RT[ch][0] //Output field 0-3 memory read start address
	            //dvPro_Write(B13_OSFLD1CH1,DEF_ISFLD0CH2_2K,ucRegOffset);//if(ROT) PS_MEM_RT[ch][1]
	            //dvPro_Write(B13_OSFLD2CH1,DEF_ISFLD1CH2_2K,ucRegOffset);//if(ROT) PS_MEM_RT[ch][2]
	            //dvPro_Write(B13_OSFLD3CH1,DEF_ISFLD2CH2_2K,ucRegOffset);//if(ROT) PS_MEM_RT[ch][3]
	            break;
        }
    }
	////dvPro_Write(B13_OSFLD0CH1,sPS_MEM_SC[eCH][3]+val1,ucRegOffset);//if(ROT) PS_MEM_RT[ch][0] //Output field 0-3 memory read start address
	////dvPro_Write(B13_OSFLD1CH1,sPS_MEM_SC[eCH][0]+val1,ucRegOffset);//if(ROT) PS_MEM_RT[ch][1]
	////dvPro_Write(B13_OSFLD2CH1,sPS_MEM_SC[eCH][1]+val1,ucRegOffset);//if(ROT) PS_MEM_RT[ch][2]
	////dvPro_Write(B13_OSFLD3CH1,sPS_MEM_SC[eCH][2]+val1,ucRegOffset);//if(ROT) PS_MEM_RT[ch][3]
	//dvPro_Write(B13_OMWICH1  ,val2   ,ucRegOffset);											//Output Memory Width

//-----------------------------------------------------------------------------
// Flip Control
//-----------------------------------------------------------------------------
	wdt = 0;//PM_FLIP[ch]==1 ? 0x01:0x00;


	//dvPro_Write(B2_OFLPCTCH1,wdt,ucRegOffset);	//Horizontal flip control

}

static void dvPro_ScalerFillWidth(UINT16 uiXO,UINT16 uiXI,const eMCT eCH)
{
    UINT16 uiVW_OUT;
    UINT16 uiPixels;

    uiVW_OUT = (UINT16)((UINT32)m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiVActive *(UINT32)uiXO /(UINT32)uiXI);
    uiPixels = (UINT16)((UINT32)(uiVW_OUT - m_sChannelInfo[eCH].sScalerOutputTimingInfo.uiVActive)*(UINT32)uiXI/(UINT32)uiXO);

    m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiVStart += uiPixels/2;
    m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiVActive -= uiPixels;
}

static void dvPro_ScalerFillHeight(UINT16 uiYO,UINT16 uiYI,const eMCT eCH)
{
    UINT16 uiHW_OUT;
    UINT16 uiPixels;

    uiHW_OUT = (UINT16)((UINT32)m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiHActive * (UINT32)uiYO / (UINT32)uiYI);
    uiPixels = (UINT16) ((UINT32)(uiHW_OUT - m_sChannelInfo[eCH].sScalerOutputTimingInfo.uiHActive)*(UINT32)uiYI /(UINT32)uiYO);

    m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiHStart += uiPixels/2;
    m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiHActive -= uiPixels;
}

static void dvPro_ScalerOneToOne(const eMCT eCH)
{
    //Horizontal
    if(m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiHActive >= m_sChannelInfo[eCH].sScalerOutputTimingInfo.uiHActive)
    {
        m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiHStart += (m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiHActive - m_sChannelInfo[eCH].sScalerOutputTimingInfo.uiHActive)/2;
        m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiHActive = m_sChannelInfo[eCH].sScalerOutputTimingInfo.uiHActive;
    }
    else
    {
        m_sChannelInfo[eCH].sScalerOutputTimingInfo.uiHStart += (m_sChannelInfo[eCH].sScalerOutputTimingInfo.uiHActive - m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiHActive )/2;
        m_sChannelInfo[eCH].sScalerOutputTimingInfo.uiHActive = m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiHActive;
    }

    //Vert
    if(m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiVActive >= m_sChannelInfo[eCH].sScalerOutputTimingInfo.uiVActive)
    {
        m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiVStart += (m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiVActive - m_sChannelInfo[eCH].sScalerOutputTimingInfo.uiVActive)/2;
        m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiVActive = m_sChannelInfo[eCH].sScalerOutputTimingInfo.uiVActive;
    }
    else
    {
        m_sChannelInfo[eCH].sScalerOutputTimingInfo.uiVStart += (m_sChannelInfo[eCH].sScalerOutputTimingInfo.uiVActive - m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiVActive )/2;
        m_sChannelInfo[eCH].sScalerOutputTimingInfo.uiVActive = m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiVActive;
    }
}

static void dvPro_Calc_FillToThisAspect(UINT16 uiX,UINT16 uiY,const eMCT eCH)
{
    //Aspect Ratio = uiX/uiY
    UINT16 uiVW,uiHW;

    if(uiX == 0 || uiY == 0)
    {
        LOG_MSG(db_HAL_PROAV, "dvPro_Calc_FillToThisAspect size Zero\n");
        return;
    }

    if(((DOUBLE)uiX * 10000/(DOUBLE)uiY ) >
       ((DOUBLE)m_sChannelInfo[eCH].sScalerOutputTimingInfo.uiHActive *10000/(DOUBLE)m_sChannelInfo[eCH].sScalerOutputTimingInfo.uiVActive))
    {
        uiVW = (UINT16)((UINT32)m_sChannelInfo[eCH].sScalerOutputTimingInfo.uiHActive * (UINT32)uiY / (UINT32)uiX);
        m_sChannelInfo[eCH].sScalerOutputTimingInfo.uiVStart += (m_sChannelInfo[eCH].sScalerOutputTimingInfo.uiVActive - uiVW )/2;
        m_sChannelInfo[eCH].sScalerOutputTimingInfo.uiVActive = uiVW;

    }
    else
    {
        uiHW = (UINT16)((UINT32)m_sChannelInfo[eCH].sScalerOutputTimingInfo.uiVActive *(UINT32)uiX /(UINT32)uiY);
        m_sChannelInfo[eCH].sScalerOutputTimingInfo.uiHStart += (m_sChannelInfo[eCH].sScalerOutputTimingInfo.uiHActive - uiHW )/2;
        m_sChannelInfo[eCH].sScalerOutputTimingInfo.uiHActive = uiHW;

    }
}

static void dvPro_ClipPercentageSet(const eMCT eCH,INT16 nHClipPercentage,INT16 nVClipPercentage)
{
    //nVOffsetPct 250 == 25%
    UINT16 uiOffset_X = 0;
    UINT16 uiOffset_Y = 0;

    // For negative modify the scaler output rectangle
    if(nHClipPercentage < 0)
    {
        nHClipPercentage = -nHClipPercentage;
        uiOffset_X = (UINT16)((UINT32)m_sChannelInfo[eCH].sScalerOutputTimingInfo.uiHActive *
                        (UINT32)nHClipPercentage / (UINT32)1000 / 2);

        m_sChannelInfo[eCH].sScalerOutputTimingInfo.uiHStart += uiOffset_X;
        m_sChannelInfo[eCH].sScalerOutputTimingInfo.uiHActive -= (uiOffset_X*2);
    }
    else // otherwise modify the scaler input rectangle
    {
        uiOffset_X = (UINT16)((UINT32)m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiHActive *
                        (UINT32)nHClipPercentage / (UINT32)1000 / 2);

        m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiHStart += uiOffset_X;
        m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiHActive -= (uiOffset_X*2);
    }

    // For negative modify the scaler output rectangle
    if(nVClipPercentage < 0)
    {
        nVClipPercentage = -nVClipPercentage;
        uiOffset_Y = (UINT16)((UINT32)m_sChannelInfo[eCH].sScalerOutputTimingInfo.uiVActive *
                        (UINT32)nVClipPercentage / (UINT32)1000 / 2);

        m_sChannelInfo[eCH].sScalerOutputTimingInfo.uiVStart += uiOffset_Y;
        m_sChannelInfo[eCH].sScalerOutputTimingInfo.uiVActive -= (uiOffset_Y*2);
    }
    else // otherwise modify the scaler input rectangle
    {
        uiOffset_Y = (UINT16)((UINT32)m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiVActive *
                        (UINT32)nVClipPercentage / (UINT32)1000 / 2);

        if(m_sChannelInfo[eCH].sScaler_InputTimingInfo.eScanMode == eSCAN_MODE_INTERLACE)
        {
            m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiVStart += (uiOffset_Y /2);
        }
        else
        {
            m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiVStart += uiOffset_Y;
        }
        m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiVActive -= (uiOffset_Y*2);
    }
}

static void dvPro_Calc_Scaler_PIP_Window(const eMCT eCH)   //A70LV_Doulas_0003
{
    UINT16 uiPIP_Offset = 5;
    UINT16 uiHActive = m_sChannelInfo[eCH].sScalerOutputTimingInfo.uiHActive;
    UINT16 uiVActive = m_sChannelInfo[eCH].sScalerOutputTimingInfo.uiVActive;
    UINT16 uiHStart  = m_sChannelInfo[eCH].sScalerOutputTimingInfo.uiHStart;
    UINT16 uiVStart  = m_sChannelInfo[eCH].sScalerOutputTimingInfo.uiVStart;

    if(uiHActive > 1920 || uiVActive > 1200) //OSD用double size,所以offset要更著調整
    {
        uiPIP_Offset = uiPIP_Offset*2;
    }

    if(eCH == eMCT_CH2) //only sub
    {
        //cal H/V size
        switch(m_sChannelSetting[eMCT_CH1].cPIP_Size)
        {
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

            default:
                break;
        }

        //cal H/V start
        switch(m_sChannelSetting[eMCT_CH1].cMain_Layout)
        {
            case eCM_MAIN_LAYOUT_PIP_BOTTOM_RIGHT:
                uiHStart = uiHStart + m_sChannelInfo[eCH].sOutputTimingInfo.uiHActive - uiHActive - uiPIP_Offset;
                uiVStart = uiVStart + m_sChannelInfo[eCH].sOutputTimingInfo.uiVActive - uiVActive - uiPIP_Offset;
                break;

            case eCM_MAIN_LAYOUT_PIP_BOTTOM_LEFT:
                uiHStart = uiHStart + uiPIP_Offset;
                uiVStart = uiVStart + m_sChannelInfo[eCH].sOutputTimingInfo.uiVActive - uiVActive - uiPIP_Offset;
                break;

            case eCM_MAIN_LAYOUT_PIP_TOP_LEFT:
                uiHStart = uiHStart + uiPIP_Offset;
                uiVStart = uiVStart + uiPIP_Offset;
                break;

            case eCM_MAIN_LAYOUT_PIP_TOP_RIGHT:
                uiHStart = uiHStart + m_sChannelInfo[eCH].sOutputTimingInfo.uiHActive - uiHActive - uiPIP_Offset;
                uiVStart = uiVStart + uiPIP_Offset;
                break;

            default:
                break;
        }
        LOG_MSG(db_DV_SCALER, "dvPro_Calc_Scaler_PIP_Size[%d]()(%dx%d)\r\n",
            eCH,uiHStart,uiVStart,uiHActive,uiVActive);

        m_sChannelInfo[eCH].sScalerOutputTimingInfo.uiHActive = uiHActive;
        m_sChannelInfo[eCH].sScalerOutputTimingInfo.uiVActive = uiVActive;
        m_sChannelInfo[eCH].sScalerOutputTimingInfo.uiHStart  = uiHStart;
        m_sChannelInfo[eCH].sScalerOutputTimingInfo.uiVStart  = uiVStart;
    }
}

static void dvPro_Calc_Scaler_PBP_Window(const eMCT eCH)   //A70LV_Doulas_0003
{
    UINT16 uiHActive = m_sChannelInfo[eCH].sScalerOutputTimingInfo.uiHActive;
    UINT16 uiVActive = m_sChannelInfo[eCH].sScalerOutputTimingInfo.uiVActive;
    UINT16 uiHStart  = m_sChannelInfo[eCH].sScalerOutputTimingInfo.uiHStart;
    UINT16 uiVStart  = m_sChannelInfo[eCH].sScalerOutputTimingInfo.uiVStart;
    UINT16 PLeft = 0, PRight = 0, PTop = 0, PDown = 0;
    UINT16 uiBig_Size = 0, uiSmall_Size = 0;

    if(eCH == eMCT_CH1)
    {
        switch(m_sChannelSetting[eMCT_CH1].cPIP_Size)
        {
            case eCM_PIP_SIZE_LARGE:
                switch(m_sChannelSetting[eMCT_CH1].cMain_Layout)
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
                switch(m_sChannelSetting[eMCT_CH1].cMain_Layout)
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
                switch(m_sChannelSetting[eMCT_CH1].cMain_Layout)
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
        switch(m_sChannelSetting[eMCT_CH1].cPIP_Size)
        {
            case eCM_PIP_SIZE_LARGE:
                switch(m_sChannelSetting[eMCT_CH1].cMain_Layout)
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
                switch(m_sChannelSetting[eMCT_CH1].cMain_Layout)
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
                switch(m_sChannelSetting[eMCT_CH1].cMain_Layout)
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
    LOG_MSG(db_DV_SCALER, "dvPro_Calc_Scaler_PBP_Size[%d]()(%dx%d)\r\n", eCH,uiHStart + PLeft,uiVStart + PTop,PRight - PLeft,PDown - PTop);

    m_sChannelInfo[eCH].sScalerOutputTimingInfo.uiHActive = PRight - PLeft;
    m_sChannelInfo[eCH].sScalerOutputTimingInfo.uiVActive = PDown - PTop;
    m_sChannelInfo[eCH].sScalerOutputTimingInfo.uiHStart  = uiHStart + PLeft;
    m_sChannelInfo[eCH].sScalerOutputTimingInfo.uiVStart  = uiVStart + PTop;
}

static void dvPro_Calc_Scaler_PIP_PBP_Window_2K(const eMCT eCH)    //A70LV_Doulas_0003
{
    switch(m_sChannelSetting[eMCT_CH1].cMain_Layout)
    {
        case eCM_MAIN_LAYOUT_PBP_MAIN_LEFT:
        case eCM_MAIN_LAYOUT_PBP_MAIN_TOP:
        case eCM_MAIN_LAYOUT_PBP_MAIN_RIGHT:
        case eCM_MAIN_LAYOUT_PBP_MAIN_BOTTOM:
            dvPro_Calc_Scaler_PBP_Window(eCH);
            break;

        case eCM_MAIN_LAYOUT_PIP_BOTTOM_RIGHT:
        case eCM_MAIN_LAYOUT_PIP_BOTTOM_LEFT:
        case eCM_MAIN_LAYOUT_PIP_TOP_LEFT:
        case eCM_MAIN_LAYOUT_PIP_TOP_RIGHT:
            dvPro_Calc_Scaler_PIP_Window(eCH);
            break;

        default:
            break;
    }
}

static void dvPro_Calc_Scaler_InputAndOutput_Window_2K(const eMCT eCH)
{
    UINT16 uiHStart,uiVStart;        //A70LV_Doulas_0005
    eCM_SCALING_MODE_ID eAspectRatio;     //A70LV_Doulas_0098
    if(eCH >= eMCT_LAST)
    {
        return;
    }

    //A70LV_Doulas_0005 start
    if(dvPro_AutoPositionSetting(eCH) == TRUE)
    {
        //A70LV_Doulas_0210 modify
        if((m_sChannelInfo[eCH].sInputTimingInfo.uiHActive == m_sChannelInfo[eCH].sInputAutoMeasure.uiHw) ||
           ((m_sChannelInfo[eCH].sInputTimingInfo.uiHActive+1) == m_sChannelInfo[eCH].sInputAutoMeasure.uiHw))
        {
            uiHStart = m_sChannelInfo[eCH].sInputAutoMeasure.uiHst;

        }
        else
        {
            uiHStart = m_sChannelInfo[eCH].sInputTimingInfo.uiHStart;
        }

        if(m_sChannelInfo[eCH].sInputTimingInfo.uiVActive == m_sChannelInfo[eCH].sInputAutoMeasure.uiVw)
        {
            uiVStart = m_sChannelInfo[eCH].sInputAutoMeasure.uiVst;
        }
        else
        {
            uiVStart = m_sChannelInfo[eCH].sInputTimingInfo.uiVStart;
        }
        //uiHStart = m_sChannelInfo[eCH].sInputAutoMeasure.uiHst;
        //uiVStart = m_sChannelInfo[eCH].sInputAutoMeasure.uiVst;
    }
    else
    {
        uiHStart = m_sChannelInfo[eCH].sInputTimingInfo.uiHStart;
        uiVStart = m_sChannelInfo[eCH].sInputTimingInfo.uiVStart;
    }
    //A70LV_Doulas_0005 end
    if(bModeAdjustmentEnable == TRUE)       //A70LV_Doulas_0195 Add
    {
        uiHStart = uiModeAdjusmenttHorzStart;
        uiVStart = uiModeAdjusmenttVertStart;
    }

    //Input
    m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiHStart = uiHStart;   //A70LV_Doulas_0005 modify
    m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiHActive = m_sChannelInfo[eCH].sInputTimingInfo.uiHActive;
    m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiHTotal = m_sChannelInfo[eCH].sInputTimingInfo.uiHTotal;

    m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiVStart = uiVStart;       //A70LV_Doulas_0005 modify
    m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiVActive = m_sChannelInfo[eCH].sInputTimingInfo.uiVActive;
    m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiVTotal = m_sChannelInfo[eCH].sInputTimingInfo.uiVTotal;

    m_sChannelInfo[eCH].sScaler_InputTimingInfo.dFrameRate = m_sChannelInfo[eCH].sInputTimingInfo.dFrameRate;
    m_sChannelInfo[eCH].sScaler_InputTimingInfo.eScanMode = m_sChannelInfo[eCH].sInputTimingInfo.eScanMode;

    //Output
    m_sChannelInfo[eCH].sScalerOutputTimingInfo.uiFrameRate  = m_sChannelInfo[eCH].sOutputTimingInfo.uiFrameRate;
    m_sChannelInfo[eCH].sScalerOutputTimingInfo.ulPixelClock = m_sChannelInfo[eCH].sOutputTimingInfo.ulPixelClock;
    m_sChannelInfo[eCH].sScalerOutputTimingInfo.uiHStart     = m_sChannelInfo[eCH].sOutputTimingInfo.uiHStart;
    m_sChannelInfo[eCH].sScalerOutputTimingInfo.uiHActive    = m_sChannelInfo[eCH].sOutputTimingInfo.uiHActive;
    m_sChannelInfo[eCH].sScalerOutputTimingInfo.uiVStart     = m_sChannelInfo[eCH].sOutputTimingInfo.uiVStart;
    m_sChannelInfo[eCH].sScalerOutputTimingInfo.uiVActive    = m_sChannelInfo[eCH].sOutputTimingInfo.uiVActive;
    m_sChannelInfo[eCH].sScalerOutputTimingInfo.uiHTotal     = m_sChannelInfo[eCH].sOutputTimingInfo.uiHTotal;
    m_sChannelInfo[eCH].sScalerOutputTimingInfo.uiVTotal     = m_sChannelInfo[eCH].sOutputTimingInfo.uiVTotal;
    m_sChannelInfo[eCH].sScalerOutputTimingInfo.uiHSyncWidth = m_sChannelInfo[eCH].sOutputTimingInfo.uiHSyncWidth;
    m_sChannelInfo[eCH].sScalerOutputTimingInfo.uiVSyncWidth = m_sChannelInfo[eCH].sOutputTimingInfo.uiVSyncWidth;

    switch(dvPro_Input_3D_Format_Config_Get())        //A70LV_Doulas_0154
    {
        case eINPUT_3D_TYPE_FRAMEPACKING:
            if(eCH == eMCT_CH1)
            {
                if(m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiVTotal > 2240)
                {
                    m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiVActive = 1080;
                }
                else
                {
                    m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiVActive = 720;
                }
            }
            else
            {
                if(m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiVTotal > 2240)
                {
                    m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiVActive = 1080;
                    m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiVStart += (m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiVTotal/2);
                }
                else
                {
                    m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiVActive = 720;
                    m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiVStart += (m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiVTotal/2);
                }
            }
            break;

        case eINPUT_3D_TYPE_SIDEBYSIDE:
            if(eCH == eMCT_CH1)
            {
                m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiHActive /= 2;
            }
            else
            {
                m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiHActive /= 2;
                m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiHStart += m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiHActive;
            }
            break;

        case eINPUT_3D_TYPE_TOPANDBOTTOM:
            if(eCH == eMCT_CH1)
            {
                m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiVActive = m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiVActive/2;
            }
            else
            {
                m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiVActive = m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiVActive/2;
                m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiVStart += m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiVActive;
            }
            break;

        case eINPUT_3D_TYPE_FRAME_SEQUENTIAL:
        case eINPUT_3D_TYPE_DAUL_PIPE:
            break;

        default:
            break;
    }

    if(m_sChannelSetting[eMCT_CH1].cPIP_PBP_ON == TRUE)     //A70LV_Doulas_0003
    {
        dvPro_Calc_Scaler_PIP_PBP_Window_2K(eCH);
        LOG_MSG(db_DV_SCALER, "InputAndOutput_Window_2K(%d) PIP:(%d) I(%d,%d)(%d,%d)O(%d,%d)(%d,%d)\r\n",
        eCH,
        //m_sChannelSetting[eCH].eScalingMode,
        m_sChannelSetting[eCH].cMain_Layout,
        m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiHStart,
        m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiVStart,
        m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiHActive,
        m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiVActive,
        m_sChannelInfo[eCH].sScalerOutputTimingInfo.uiHStart,
        m_sChannelInfo[eCH].sScalerOutputTimingInfo.uiVStart,
        m_sChannelInfo[eCH].sScalerOutputTimingInfo.uiHActive,
        m_sChannelInfo[eCH].sScalerOutputTimingInfo.uiVActive );    //A70LV_Doulas_0042
        return;
    }

    //p_sChannelInfo = &m_sChannelInfo[eCH];

    eAspectRatio = m_sChannelSetting[eCH].eScalingMode;     //A70LV_Doulas_0098
    if((m_sChannelInfo[eMCT_CH1].ePanelTimingId == PANEL_3D_OUTPUT) ||
       (dvPro_AspectRatioIs3DModeGet() == TRUE))     //A70LV_Doulas_0154 modify //A70LV_Doulas_0098
    {
        eAspectRatio = eCM_SCALING_MODE_3D;
    }
    else if(m_sChannelInfo[eMCT_CH1].ePanelTimingId == PANEL_2D_HIGHSPEED)
    {
        eAspectRatio = eCM_SCALING_MODE_AUTO;
    }

    //Aspect Ratio
    switch(eAspectRatio) //A70LV_Doulas_0098 modify//A70LV_Doulas_0013
    {
        default:
        case eCM_SCALING_MODE_AUTO:
            dvPro_Calc_FillToThisAspect(m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiHActive,
                                         m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiVActive,eCH);
            break;

        case eCM_SCALING_MODE_CUSTOM:
        case eCM_SCALING_MODE_NATIVE:
            dvPro_ScalerOneToOne(eCH);
            break;

        case eCM_SCALING_MODE_LETTER_BOX:
            dvPro_ClipPercentageSet(eCH,
                                     0,     // No horizontal clip
                                     250);  // Clip 25% vertically (12.5% top and // bottom)
            break;

        case eCM_SCALING_MODE_FULL_SIZE:
        //case eSCALING_MODE_3D:    //ZU860_Doulas_0119 remove
            //do nothing...
            break;

        case eCM_SCALING_MODE_3D:
            break;

        case eCM_SCALING_MODE_FULL_WIDTH:
            {
                UINT32 ulny;

                if(m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiHActive == 0)
                {
                    ASSERT_ALWAYS();
                    m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiHActive = 1;
                }

                ulny = (UINT32)m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiVActive *
                       (UINT32)m_sChannelInfo[eCH].sScalerOutputTimingInfo.uiHActive /
                       (UINT32)m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiHActive;

                if(ulny  > (UINT32)m_sChannelInfo[eCH].sScalerOutputTimingInfo.uiVActive)
                {
                    dvPro_ScalerFillWidth(m_sChannelInfo[eCH].sScalerOutputTimingInfo.uiHActive,m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiHActive,eCH);
                }
                else
                {
                    dvPro_Calc_FillToThisAspect(m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiHActive,
                                            m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiVActive,eCH);
                }
            }
            break;

        case eCM_SCALING_MODE_FULL_HEIGHT:
            {
                UINT32 ulnx;

                if(m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiVActive == 0)
                {
                    ASSERT_ALWAYS();
                    m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiVActive = 1;
                }

                ulnx = (UINT32)m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiHActive *
                       (UINT32)m_sChannelInfo[eCH].sScalerOutputTimingInfo.uiVActive /
                       (UINT32)m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiVActive;

                if(ulnx > (UINT32)m_sChannelInfo[eCH].sScalerOutputTimingInfo.uiHActive)
                {
                    dvPro_ScalerFillWidth(m_sChannelInfo[eCH].sScalerOutputTimingInfo.uiVActive,m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiVActive,eCH);
                }
                else
                {
                    dvPro_Calc_FillToThisAspect(m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiHActive,
                                            m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiVActive,eCH);
                }
            }
            break;

        case eCM_SCALING_MODE_4_3:
            dvPro_Calc_FillToThisAspect(400,300,eCH);
            break;

        case eCM_SCALING_MODE_16_9:
            dvPro_Calc_FillToThisAspect(160,90,eCH);
            break;

        case eCM_SCALING_MODE_16_10:
            dvPro_Calc_FillToThisAspect(160,100,eCH);
            break;

        case eCM_SCALING_MODE_21_9:// x35G2_Bruce_0001
            dvPro_Calc_FillToThisAspect(210,90,eCH);
            break;
    }

    if(dvPro_AspectRatioIs3DModeGet() == eINPUT_3D_TYPE_OFF)        //A70LV_Doulas_0154
    {
        if((m_sChannelSetting[eCH].uiDigitalHorzZoom == DIGITAL_HORZ_ZOOM_DEFAULT) &&
           (m_sChannelSetting[eCH].uiDigitalVertZoom == DIGITAL_VERT_ZOOM_DEFAULT))      //A70LV_Doulas_0009 Add Over Scan and Digital Zoom
        {
            //Over Scan
            dvPro_Scale_Overscan(m_sChannelSetting[eCH].eOverScan,
                                 &m_sChannelInfo[eCH].sScaler_InputTimingInfo,
                                 &m_sChannelInfo[eCH].sScalerOutputTimingInfo);
        }
        else
        {
            //Digital Zoom
            if(eAspectRatio == eCM_SCALING_MODE_CUSTOM)	//A70LK_Sammy_0002
            {
                dvPro_Scale_Digital_Zoom_Custom(&m_sChannelInfo[eCH].sScaler_InputTimingInfo,
                                          &m_sChannelInfo[eCH].sScalerOutputTimingInfo);
            }
            else
            {
                dvPro_Scale_Digital_Zoom(&m_sChannelInfo[eCH].sScaler_InputTimingInfo,
                                          &m_sChannelInfo[eCH].sScalerOutputTimingInfo);
            }
        }

        //Calc Start Position
        dvPro_Start_Position_Set(eCH,
                              &m_sChannelInfo[eCH].sScaler_InputTimingInfo,
                              &m_sChannelInfo[eCH].sScalerOutputTimingInfo);    //A70LV_Doulas_0013 //A70LV_Doulas_0010
    }       //A70LV_Doulas_0155 modify

    dvPro_Scale_Setting_Check(eCH,&m_sChannelInfo[eCH].sScaler_InputTimingInfo,
                                   &m_sChannelInfo[eCH].sScalerOutputTimingInfo);   //A70LV_Doulas_0010 Add


    LOG_MSG(db_DV_SCALER, "InputAndOutput_Window_2K :(%d) I(%d,%d)(%d,%d)O(%d,%d)(%d,%d)\r\n", m_sChannelSetting[eCH].eScalingMode,
        m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiHStart,
        m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiVStart,
        m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiHActive,
        m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiVActive,
        m_sChannelInfo[eCH].sScalerOutputTimingInfo.uiHStart,
        m_sChannelInfo[eCH].sScalerOutputTimingInfo.uiVStart,
        m_sChannelInfo[eCH].sScalerOutputTimingInfo.uiHActive,
        m_sChannelInfo[eCH].sScalerOutputTimingInfo.uiVActive );    //A70LV_Doulas_0002 debu
}

BOOL dvPro_PIP_Enable(void)
{
    if((m_sChannelSetting[eMCT_CH1].cPIP_PBP_ON == TRUE) &&
       (m_sChannelSetting[eMCT_CH1].cMain_Layout >= eCM_MAIN_LAYOUT_PIP_BOTTOM_RIGHT &&
        m_sChannelSetting[eMCT_CH1].cMain_Layout <= eCM_MAIN_LAYOUT_PIP_TOP_RIGHT))
       return TRUE;
    else
       return FALSE;
}

BOOL dvPro_PBP_Enable(void)
{
    if((m_sChannelSetting[eMCT_CH1].cPIP_PBP_ON == TRUE) &&
       (m_sChannelSetting[eMCT_CH1].cMain_Layout <= eCM_MAIN_LAYOUT_PBP_MAIN_BOTTOM))
       return TRUE;
    else
       return FALSE;
}

BOOL dvPro_PIP_PBP_Enable(void)        //A70LV_Doulas_0226
{
    if(m_sChannelSetting[eMCT_CH1].cPIP_PBP_ON == TRUE)
       return TRUE;
    else
       return FALSE;
}

void dvPro_Set_Output_Fill_Color(BOOL bEnable,UINT8 ucValue)       //A70LV_Doulas_0035 modify
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

        //dvPro_Write(B2_PGCTRLCH1,0,0); //disable test pattern

        //dvPro_Write(B2_OFILLCH1,ulColor,0);
        //dvPro_Write(B2_OBKGD0CH1,ulColor,0);
        //dvPro_Write(B2_OBKGD1CH1,ulColor,0);
        //dvPro_Write(B2_OBKGD2CH1,ulColor,0);

        //dvPro_Write(B2_OACTHSTCH1,m_sChannelInfo[0].sOutputTimingInfo.uiHStart,0);
        //dvPro_Write(B2_OACTHWCH1,m_sChannelInfo[0].sOutputTimingInfo.uiHActive,0);
        //dvPro_Write(B2_OACTVSTCH1,m_sChannelInfo[0].sOutputTimingInfo.uiVStart-5,0);
        //dvPro_Write(B2_OACTVWCH1,m_sChannelInfo[0].sOutputTimingInfo.uiVActive+1,0);

        //dvPro_Write(B0_LSCCT,	0x00,0) ;
        //dvPro_Write(B30_WPBLKCT,	0x00,0) ;   //disable wap
        m_sChannelInfo[0].ucOIMGCT |= 0x01;
    }
    else
    {
        dvPro_SetInputPort_2K(eMCT_CH1);
        #ifdef C821_WARPING_ENABLE      //A70LV_Doulas_0100 M0dify
        //dvPro_Write(B30_WPBLKCT, ucWarpInit,0);  //A70LV_Doulas_0079
        #endif

        //dvPro_Write(B2_OFILLCH1,ulColor,0);
        //dvPro_Write(B2_OBKGD0CH1,ulColor,0);
        //dvPro_Write(B2_OBKGD1CH1,ulColor,0);
        //dvPro_Write(B2_OBKGD2CH1,ulColor,0);
        m_sChannelInfo[0].ucOIMGCT &= 0xFE;
    }


    //dvPro_Write(B2_OIMGCTCH1,m_sChannelInfo[0].ucOIMGCT,0);

}

void dvPro_LVDS_Stop(void)
{

	//dvPro_Write(B0_LVDSOCLKCT,		0x00,0);	//Output side LVDS clock control
	//dvPro_Write(B0_LVDSOSETUP,		0x00,0);	//Output side LVDS setup
	//dvPro_Write(B0_LVDSO1CT,		0x00,0);	//Output side LVDS1 control
	//dvPro_Write(B0_LVDSO2CT,		0x00,0);
	//dvPro_Write(B0_LVDSTXCT1,		0x00,0);	//###Output side LVDS macro control
	//dvPro_Write(B0_LVDSTXCT2,		0x00,0);
    MS_SLEEP(10);//10Ms


}

void dvPro_Field_Interlock_Transfer_Control_Start(void)    //A70LV_Doulas_0002
{
	//dvPro_Write(B2_FLDRTCTCH1,0x85,0);		//Field interlock transfer control
	//dvPro_Write(B15_FLDRTCTCH2,0x85,0);		//Field interlock transfer control
	//dvPro_Write(B30_FLDRTCT,0x85,0);		//Field interlock transfer control
}

void dvPro_Field_Interlock_Transfer_Control_End(void)  //A70LV_Doulas_0002
{
	//dvPro_Write(B2_FLDRTCT2CH1,0x01,0);		//Field interlock transfer control 2
	//dvPro_Write(B15_FLDRTCT2CH2,0x01,0);	//Field interlock transfer control 2
	//dvPro_Write(B30_FLDRTCT2,0x01,0);		//Field interlock transfer control 2

    //dvPro_Write(B2_FLDRTCTCH1,0x00,0);		//Field interlock transfer control
	//dvPro_Write(B15_FLDRTCTCH2,0x00,0);		//Field interlock transfer control
	//dvPro_Write(B30_FLDRTCT,0x00,0);		//Field interlock transfer control
}

static const double m_arCSCREC709toRGB[]=
{
    1.164, 0, 1.793,
    1.164, -0.213, -0.534,
    1.164, 2.115, 0
};

static const int16 m_arCSCREC709toRGB_bias[]=
{
    -16, -128, -128
};

void dvPro_Set_Color_Matrix(const eMCT eCH, INT32 HUE, DOUBLE dSaturation)
{

    DOUBLE dYuvRgb1[9];
    DOUBLE dMatrix1[3][3];
    DOUBLE dMatrix2[3][3];
    UINT8  cCSC_TableBias[3] = {0};
    UINT8  ucCount = 0;
    UINT32 wTemp32 = 0;
    double dTemp = 0;
    UINT16 uiCSC_Table[9] = {0};

    UINT32 coefOpmAddr[9] = { //Main
       eOpmCoefR1Adr, eOpmCoefG1Adr, eOpmCoefB1Adr,
       eOpmCoefR2Adr, eOpmCoefG2Adr, eOpmCoefB2Adr,
       eOpmCoefR3Adr, eOpmCoefG3Adr, eOpmCoefB3Adr};

    UINT32 coefOpsAddr[9] = { //Sub
       eOpsCoefR1Adr, eOpsCoefG1Adr, eOpsCoefB1Adr,
       eOpsCoefR2Adr, eOpsCoefG2Adr, eOpsCoefB2Adr,
       eOpsCoefR3Adr, eOpsCoefG3Adr, eOpsCoefB3Adr};

    if(eCH >= eMCT_LAST)
    {
        return;
    }

    dMatrix1[0][0] = m_arCSCREC709toRGB[0];
    dMatrix1[0][1] = m_arCSCREC709toRGB[1];
    dMatrix1[0][2] = m_arCSCREC709toRGB[2];
    dMatrix1[1][0] = m_arCSCREC709toRGB[3];
    dMatrix1[1][1] = m_arCSCREC709toRGB[4];
    dMatrix1[1][2] = m_arCSCREC709toRGB[5];
    dMatrix1[2][0] = m_arCSCREC709toRGB[6];
    dMatrix1[2][1] = m_arCSCREC709toRGB[7];
    dMatrix1[2][2] = m_arCSCREC709toRGB[8];

    // Saturation Matrix
    dMatrix2[0][0] = 1;
    dMatrix2[0][1] = 0;
    dMatrix2[0][2] = 0;
    dMatrix2[1][0] = 0;
    dMatrix2[1][1] = cos(HUE*3.14159/180)*dSaturation;
    dMatrix2[1][2] = sin(HUE*3.14159/180)*dSaturation;
    dMatrix2[2][0] = 0;
    dMatrix2[2][1] = -sin(HUE*3.14159/180)*dSaturation;
    dMatrix2[2][2] = cos(HUE*3.14159/180)*dSaturation;

    dYuvRgb1[0] = dMatrix1[0][0] * dMatrix2[0][0] + dMatrix1[0][1] * dMatrix2[1][0] + dMatrix1[0][2] * dMatrix2[2][0];
    dYuvRgb1[1] = dMatrix1[0][0] * dMatrix2[0][1] + dMatrix1[0][1] * dMatrix2[1][1] + dMatrix1[0][2] * dMatrix2[2][1];
    dYuvRgb1[2] = dMatrix1[0][0] * dMatrix2[0][2] + dMatrix1[0][1] * dMatrix2[1][2] + dMatrix1[0][2] * dMatrix2[2][2];
    dYuvRgb1[3] = dMatrix1[1][0] * dMatrix2[0][0] + dMatrix1[1][1] * dMatrix2[1][0] + dMatrix1[1][2] * dMatrix2[2][0];
    dYuvRgb1[4] = dMatrix1[1][0] * dMatrix2[0][1] + dMatrix1[1][1] * dMatrix2[1][1] + dMatrix1[1][2] * dMatrix2[2][1];
    dYuvRgb1[5] = dMatrix1[1][0] * dMatrix2[0][2] + dMatrix1[1][1] * dMatrix2[1][2] + dMatrix1[1][2] * dMatrix2[2][2];
    dYuvRgb1[6] = dMatrix1[2][0] * dMatrix2[0][0] + dMatrix1[2][1] * dMatrix2[1][0] + dMatrix1[2][2] * dMatrix2[2][0];
    dYuvRgb1[7] = dMatrix1[2][0] * dMatrix2[0][1] + dMatrix1[2][1] * dMatrix2[1][1] + dMatrix1[2][2] * dMatrix2[2][1];
    dYuvRgb1[8] = dMatrix1[2][0] * dMatrix2[0][2] + dMatrix1[2][1] * dMatrix2[1][2] + dMatrix1[2][2] * dMatrix2[2][2];


    for(ucCount = 0; ucCount < 9; ucCount++)
    {
        if(dYuvRgb1[ucCount] >= 0)
        {
            dTemp = dYuvRgb1[ucCount];
            dTemp *= 8192;
            wTemp32 = (UINT32)dTemp;
            uiCSC_Table[ucCount] = (UINT16)(wTemp32 & 0xFFFF);
            uiCSC_Table[ucCount] = (uiCSC_Table[ucCount] >= 0x8000)?0x7FFF:uiCSC_Table[ucCount];
        }
        else
        {
            dTemp = -dYuvRgb1[ucCount];
            dTemp *= 8192;
            wTemp32 = (UINT32)dTemp;
            uiCSC_Table[ucCount] = (UINT16)(wTemp32 & 0xFFFF);
            uiCSC_Table[ucCount] = (uiCSC_Table[ucCount] >= 0x8000)?0x7FFF:uiCSC_Table[ucCount];
            uiCSC_Table[ucCount] |= 0x8000;
        }
    }

    //for(ucCount = 0; ucCount < 9; ucCount++)
    //{
        //LOG_MSG(db_ALWAYS,"%f 0x%04x \n", dYuvRgb1[ucCount], uiCSC_Table[ucCount]);
    //}
    //LOG_MSG(db_ALWAYS,"\n");

    if(eCH == eMCT_CH1)
    {
        for(ucCount = 0; ucCount < 9; ucCount++)
        {
            dvProAV_AccessWrite(coefOpmAddr[ucCount], (uint32)uiCSC_Table[ucCount]);
        }
    }
    else
    {
        for(ucCount = 0; ucCount < 9; ucCount++)
        {
            dvProAV_AccessWrite(coefOpsAddr[ucCount], (uint32)uiCSC_Table[ucCount]);
        }
    }

    for(ucCount = 0; ucCount < 3; ucCount++)
    {
        int16 iTemp = m_arCSCREC709toRGB_bias[ucCount];
        iTemp = iTemp*4 + 512;


        if(iTemp >= 0)
        {
            cCSC_TableBias[ucCount] = (UINT8)(iTemp/4);
        }
        else
        {

            cCSC_TableBias[ucCount] = (UINT8)(-iTemp/4);
            cCSC_TableBias[ucCount] |= 0x80;
        }
    }

    if(eCH == eMCT_CH1)
    {
        dvProAV_AccessWrite(eOpmBiasG, (uint32)cCSC_TableBias[0]);
        dvProAV_AccessWrite(eOpmBiasB, (uint32)cCSC_TableBias[1]);
        dvProAV_AccessWrite(eOpmBiasR, (uint32)cCSC_TableBias[2]);

    }
    else
    {
        dvProAV_AccessWrite(eOpsBiasG, (uint32)cCSC_TableBias[0]);
        dvProAV_AccessWrite(eOpsBiasB, (uint32)cCSC_TableBias[1]);
        dvProAV_AccessWrite(eOpsBiasR, (uint32)cCSC_TableBias[2]);
    }
}

void dvPro_Set_Color(const eMCT eCH)
{

	dvPro_Set_Color_Matrix(eCH, m_sChannelSetting[eCH].iHue, m_sChannelSetting[eCH].dColor);

	return;
}


void dvPro_calc_clrm(
	double utmp, double vtmp,
	double UC, double VC,
	double UP, double VP,
	double C, double D, double DOM, double GAIN,int coveron,
	double *u, double *v)       //A70LV_Doulas_0003
{
	double distance, Dr, sinA, cosA, sinB, cosB, A, B;
	double COVER, utmpnew, vtmpnew, MARUME=0;

	distance = sqrt(pow((utmp-UC),2)+pow((vtmp-VC),2));

    if(GAIN == 0)
    {
        ASSERT_ALWAYS();
        GAIN = 1;
    }

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
    {
		utmpnew = 0.0;
    }


	if(vtmpnew > 1023.0 )
		vtmpnew = 1023.0;
	else if (vtmpnew < 0.0)
    {
		vtmpnew = 0.0;
    }

	*u = utmpnew;
	*v = vtmpnew;

	return;
}

void dvPro_Set_Color_Correction(eMCT eCH,	 //A70LV_Doulas_0003
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
    UINT8   ucRegOffset = 0;
    INT8 iPM_CNT_CLRM[eMCT_LAST] = {0,0};


    if(eCH >= eMCT_LAST)
    {
        return;
    }

    ucRegOffset = (eCH == eMCT_CH1) ? 0 : 13;

	if((blgain == 0)&&(grgain==0)&&(flgain == 0))
    {
		//dvPro_Write_NoSemaphore(B6_FCGCTCH1,0x02,ucRegOffset);		//Output full color gamma control register		// CPUACC=Enb/FCG=Dis
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
		//dvPro_Write_NoSemaphore(B6_FCGCTCH1,0x06,ucRegOffset);// CPU Access Enable			// YCMN=Enb/CPUACC=Enb/FCG=Dis
	}
    else
	{
		//dvPro_Write_NoSemaphore(B6_FCGCTCH1,0x07,ucRegOffset);// CPU Access Enable			// YCMN=Enb/CPUACC=Enb/FCG=Enb
	}
    //gainon[0] = gainon[0];

	if(iPM_CNT_CLRM[eCH] == 0)
    {
		iPM_CNT_CLRM[eCH]=lmax-lmin;
	}

	for(INT32 l=(lmax-iPM_CNT_CLRM[eCH]); l<lmax ;l++)
    {
		for(INT32 k=kmin;k<kmax;k++)
        {
			INT32		i;
			i =l*64 + k;
			if(k == kmin)
            {
				//dvPro_Write_NoSemaphore(B6_FCGADCH1,(i&0xFF)       ,ucRegOffset);  //B6_FCGAD[7:0]  //Output full color gamma address
				//dvPro_Write_NoSemaphore(B6_FCGADCH1,((i >> 8)&0xFF),ucRegOffset);  //B6_FCGAD[11:8]
			}
			utmp = (double)(i&u_shift);
			vtmp = (double)((i&v_shift)>>uv_threth);
			utmp = utmp * uv_mlti;
			vtmp = vtmp * uv_mlti;
			utmpnew2=utmp;
			vtmpnew2=vtmp;

			for(INT32 j=0;j<3;j++)
            {
				dvPro_calc_clrm( utmpnew2,vtmpnew2,uc[j],vc[j],up[j],vp[j],c[j],d[j],dom[j],gain[j],coveron[j],&utmp,&vtmp);
				utmpnew2 = utmp;
				vtmpnew2 = vtmp;
			}

			val = (INT32)(((unsigned int)utmpnew2)&0x0ff);								//	U[7:0]
			//dvPro_Write_NoSemaphore(B6_FCGDTCH1,val,ucRegOffset);							//Output full color gamma data
			val = ((((INT32)utmpnew2&0x300)>>8) + (((INT32)vtmpnew2&0x03f)<<2));	//	{V[5:0],U[9:8]}
			//dvPro_Write_NoSemaphore(B6_FCGDTCH1,val,ucRegOffset);							//Output full color gamma data
			val = (((INT32)vtmpnew2&0x3c0)>>6);									//	{4'd0,V[9:6]}
			//dvPro_Write_NoSemaphore(B6_FCGDTCH1,val,ucRegOffset);							//Output full color gamma data
		}
		iPM_CNT_CLRM[eCH]--;
		/*if(iPM_CNT_CLRM[eCH] != 0 && mode != 3)   //A70LV_Doulas_0003 remove test
        {
			goto fin;
		}*/
	}

//fin:  //A70LV_Doulas_0008 remove
	//dvPro_Write_NoSemaphore(B6_FCGCTCH1,0x07,ucRegOffset);									// YCMN=Enb/CPUACC=Enb/FCG=Enb	buffer flush
	return;
}

UINT8 dvPro_Set_Sharpness(const eMCT eCH)
{
    UINT8 cResult;

    uint08 level = 0;
    uint08 scalingratio = 0;
    bool rgbfull= 0;

    if((m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiHActive < m_sChannelInfo[eCH].sScalerOutputTimingInfo.uiHActive) && (m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiHActive != 0))
    {
        scalingratio = (uint08)(m_sChannelInfo[eCH].sScalerOutputTimingInfo.uiHActive/m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiHActive);
    }
    else
    {
        scalingratio = 1;
    }

    if(scalingratio <= 0)
    {
        scalingratio = 1;
    }

    //dvProAV_SclSharpnessEnableSet(eCH, TRUE); //A70LK_Jacky_0037

    cResult = (UINT8)dvProAV_SclSharpnessSet(eCH, m_sChannelSetting[eCH].ucSharpness, scalingratio, TRUE);

    return cResult;
}

//A70LV_Doulas_2000 end

void dvPro_Config_NoSignalOutput(const eMCT eCH, UINT8 ucDisplayOutput)   //A70LV_Doulas_0142 //A70LV_Doulas_0004
{
    if((m_sChannelSetting[eMCT_CH1].cPIP_PBP_ON == TRUE) &&
    //   (eCH == eMCT_CH2) &&   //A70LV_Doulas_0142 remove
       (ucNo_Signal_Config[eCH] == 0))
    {
        m_sChannelInfo[eCH].sInputTimingInfo.uiHStart = 0;
        m_sChannelInfo[eCH].sInputTimingInfo.uiVStart = 0;
        m_sChannelInfo[eCH].sInputTimingInfo.uiHActive = 0;
        m_sChannelInfo[eCH].sInputTimingInfo.uiVActive = 0;
        //m_sChannelInfo[eCH].sInputTimingInfo.uiHTotal = 2200;
        //m_sChannelInfo[eCH].sInputTimingInfo.uiVTotal = 1125;
        m_sChannelInfo[eCH].sInputTimingInfo.eScanMode = eSCAN_MODE_NO_SIGNAL;
        m_sChannelInfo[eCH].sInputTimingInfo.dFrameRate = 0;
        dvProAV_SclVopGainBiasSubOffSet(true);

        if(ucDisplayOutput == 0)
        {
            dvProAV_SclVopWindowOffSet((SclEntity)eCH, TRUE);
        }
        else if(ucDisplayOutput == 1)
        {
             dvProAV_SclVopWindowOffSet((SclEntity)eCH, FALSE);
        }

        dvProAV_SclOpuScreenOffSet((SclEntity)eCH, TRUE);
        dvPro_BuildPicture_2K(eCH);
        ucNo_Signal_Config[eCH] = 1;
    }
}

BOOL dvPro_SYNC_LOCK_Compare(const eMCT eCH)   //A70LV_Doulas_0005 modify //A70LV_Doulas_0004
{
    int status;
    DetTmg currentTiming = {0};
    uint08 portstatus = 0;

    status = rcSUCCESS;
    status &= dvProAV_SclInputPortInfoGet(eCH, &portstatus, &currentTiming);

    if(status == rcSUCCESS)
    {
        if(portstatus != eInpStatusStable) //input lost
        {
            dvProAV_SclInputPortInfoClear(eCH);
            return FALSE;
        }

        return TRUE;
    }

    return FALSE;
}

///////////////////////////////////////////////////////////////////////////////////
///@brief dvPro_SetInputPort_2K : set input port
///@param UINT8 ucCH - scaler channel. Don not care in the 4K Panel
///@return void -
///@author Roger Chen
///////////////////////////////////////////////////////////////////////////////////
void dvPro_SetInputPort_2K(const eMCT eCH)
{
	UINT32 nren;    //A70LV_Doulas_0003
    if(eCH >= eMCT_LAST)
    {
        return;
    }

    if(!m_sChannelInfo[eCH].bInit)
    {
        return;
    }

    #if 0       //A70LV_Doulas_0003
	nren =(ucPM_HNR[eCH]!=0) | (ucPM_VNR[eCH]!=0) |
		  (ucPM_TNR[eCH]!=0) | (ucPM_BNR[eCH]!=0) | (ucPM_MNR[eCH]!=0);
    #else
    nren = 0;
    #endif

    m_sChannelInfo[eCH].ucMCT = (m_sChannelInfo[eCH].sInputTimingInfo.eScanMode == eSCAN_MODE_INTERLACE)|nren ? 0x1F : 0x11;    //A70LV_Doulas_0003 modify
    m_sChannelInfo[eCH].ucMCT = (m_sChannelInfo[eCH].ucMCT | ((m_sChannelInfo[eCH].ucInputPort&0x03) << 5));
    m_sChannelInfo[eCH].ucMCT = (m_sChannelSetting[eCH].cFREEZE == TRUE) ? m_sChannelInfo[eCH].ucMCT&0xFD : m_sChannelInfo[eCH].ucMCT;     //A70LV_Doulas_0030 //A70LV_Doulas_0004 Add
    //m_sChannelInfo[eCH].ucMCT |= (m_sChannelInfo[eCH].sInterlace_InputTimingInfo.eScanMode == eSCAN_MODE_PROGRESSIVE) & (ucPM_BNR[eCH]==0) & nren ? 0x80 : 0x00;    //A70LV_Doulas_0003

    switch(eCH)
    {
        case eMCT_CH1:
            //dvPro_Write(BN_MCT1,  m_sChannelInfo[eCH].ucMCT, 0);
            break;

        case eMCT_CH2:
            //dvPro_Write(BN_MCT2,  m_sChannelInfo[eCH].ucMCT, 0);
            break;
    }
}

///////////////////////////////////////////////////////////////////////////////////
///@brief dvPro_SetInput_2K : set C821 input register, like IACT
///@return void -
///@author Roger Chen
///////////////////////////////////////////////////////////////////////////////////

void dvPro_NoSignal(const eMCT eCH, BOOL bTrue)
{
    TpParam param;

	if(bTrue && m_sChannelInfo[eCH].sInputTimingInfo.eScanMode != eSCAN_MODE_NO_SIGNAL)
	{
		dvProAV_SclTpControlSet(1, eSclTpDisable);
		m_sChannelInfo[eCH].sInputTimingInfo.eScanMode = eSCAN_MODE_NO_SIGNAL;
        dvProAV_SclDeIntModeSet(eCH, m_sChannelInfo[eCH].sOutputTimingInfo.cDeinterlaceMode);
        dvProAV_SclSharpnessEnableSet(eCH, false);
	}
}

void dvPro_InputTimingSetting(const eMCT eCH, BOOL bSourceLock, PsINPUT_TIMING_INFO psInputTiming)
{
    if(bSourceLock == 1)
    {
        m_sChannelInfo[eCH].sInputTimingInfo.uiHTotal = psInputTiming->uiHTotal;
        m_sChannelInfo[eCH].sInputTimingInfo.uiVTotal = psInputTiming->uiVTotal;
        m_sChannelInfo[eCH].sInputTimingInfo.uiHActive = psInputTiming->uiHActive;
        m_sChannelInfo[eCH].sInputTimingInfo.uiVActive = psInputTiming->uiVActive;
        m_sChannelInfo[eCH].sInputTimingInfo.uiHStart = psInputTiming->uiHStart;
        m_sChannelInfo[eCH].sInputTimingInfo.uiVStart = psInputTiming->uiVStart;
        //m_sChannelInfo[eCH].sInputTimingInfo.uiHSyncWidth = psInputTiming->uiHTotal;
        //m_sChannelInfo[eCH].sInputTimingInfo.uiVSyncWidth = psInputTiming->uiv;
        m_sChannelInfo[eCH].sInputTimingInfo.dFrameRate = psInputTiming->dFrameRate;
        m_sChannelInfo[eCH].sInputTimingInfo.eScanMode = psInputTiming->eScanMode;
    }
    else
    {
        m_sChannelInfo[eCH].sInputTimingInfo.uiHStart = 0;
        m_sChannelInfo[eCH].sInputTimingInfo.uiVStart = 0;
        m_sChannelInfo[eCH].sInputTimingInfo.uiHActive = 1920;
        m_sChannelInfo[eCH].sInputTimingInfo.uiVActive = 1080;
        //m_sChannelInfo[eCH].sInputTimingInfo.uiHTotal = 2200;
        //m_sChannelInfo[eCH].sInputTimingInfo.uiVTotal = 1125;
        m_sChannelInfo[eCH].sInputTimingInfo.eScanMode = eSCAN_MODE_NO_SIGNAL;
        m_sChannelInfo[eCH].sInputTimingInfo.dFrameRate = 60;
    }
}

void dvPro_InputTimingGet(const eMCT eCH, sINPUT_TIMING_INFO *psInputTiming)    // ProAV_Rex_0033
{
        psInputTiming->uiHTotal = m_sChannelInfo[eCH].sInputTimingInfo.uiHTotal;
        psInputTiming->uiVTotal = m_sChannelInfo[eCH].sInputTimingInfo.uiVTotal;
        psInputTiming->uiHActive = m_sChannelInfo[eCH].sInputTimingInfo.uiHActive;
        psInputTiming->uiVActive = m_sChannelInfo[eCH].sInputTimingInfo.uiVActive;
        psInputTiming->uiHStart = m_sChannelInfo[eCH].sInputTimingInfo.uiHStart;
        psInputTiming->uiVStart = m_sChannelInfo[eCH].sInputTimingInfo.uiVStart;
        psInputTiming->dFrameRate = m_sChannelInfo[eCH].sInputTimingInfo.dFrameRate;
        psInputTiming->eScanMode = m_sChannelInfo[eCH].sInputTimingInfo.eScanMode;
}


static void dvPro_GetCHOutputTiming_2K(const eMCT eCH, const UINT8 ucPanelIndex)
{
    if(eCH >= eMCT_LAST)
    {
        return;
    }

    m_sChannelInfo[eCH].sOutputTimingInfo.uiHTotal      =   m_sPanelTable[ucPanelIndex].HTotal;     //H Total
    m_sChannelInfo[eCH].sOutputTimingInfo.uiVTotal      =   m_sPanelTable[ucPanelIndex].VTotal;     //V Total
    m_sChannelInfo[eCH].sOutputTimingInfo.uiHActive     =   m_sPanelTable[ucPanelIndex].HSize;      //H Active size
    m_sChannelInfo[eCH].sOutputTimingInfo.uiVActive     =   m_sPanelTable[ucPanelIndex].VSize;      //V Active size
    m_sChannelInfo[eCH].sOutputTimingInfo.uiHStart      =   m_sPanelTable[ucPanelIndex].HStart;     //H Start
    m_sChannelInfo[eCH].sOutputTimingInfo.uiVStart      =   m_sPanelTable[ucPanelIndex].VStart;     //V Start
    m_sChannelInfo[eCH].sOutputTimingInfo.uiHSyncWidth  =   m_sPanelTable[ucPanelIndex].HSync;      //H Sync Width
    m_sChannelInfo[eCH].sOutputTimingInfo.uiVSyncWidth  =   m_sPanelTable[ucPanelIndex].VSync;      //V Sync Width
    m_sChannelInfo[eCH].sOutputTimingInfo.uiHPol        =   m_sPanelTable[ucPanelIndex].HPol;       //H Polarity
    m_sChannelInfo[eCH].sOutputTimingInfo.uiVPol        =   m_sPanelTable[ucPanelIndex].VPol;       //V Active size
    m_sChannelInfo[eCH].sOutputTimingInfo.uiFrameRate   =   m_sPanelTable[ucPanelIndex].FrameRate;  //Frame rate
    m_sChannelInfo[eCH].sOutputTimingInfo.ulPixelClock  =   m_sPanelTable[ucPanelIndex].PixelClock; //Pixel clock

}

void dvPro_SetOverlay_2K(const eMCT eCH, BOOL bIsEnable)
{
    if(eCH >= eMCT_LAST)
    {
        return;
    }

    m_sChannelSetting[eMCT_CH1].cPIP_PBP_ON = (UINT8)bIsEnable;    //A70LV_Doulas_0003 Add

    if(eMCT_CH1 == eCH)
    {
        if(bIsEnable)
        {
            //dvPro_Write(B7_OVLCTCH1,  0x01, ucRegOffset);
            //dvPro_Write(B5_VZEGSELCH1,0x01, ucRegOffset);  //VEG 5symbol  //Vertical edge control

            dvProAV_SclDatapathSet(eSclPath_Pop);
            dvProAV_SclVopWindowOffSet(eSclEntity_Main, true);
            dvProAV_SclVopWindowOffSet(eSclEntity_Sub, true);
            dvProAV_SclOpuScreenOffSet(eSclEntity_Main, true);
            dvProAV_SclOpuScreenOffSet(eSclEntity_Sub, true);
            dvProAV_SclVopGainBiasSubOffSet(true);
        }
        else
        {
            //dvPro_Write(B7_OVLCTCH1,  0x00, ucRegOffset);
            //dvPro_Write(B5_VZEGSELCH1,0x00, ucRegOffset);  //VEG 5symbol  //Vertical edge control

            dvProAV_SclDatapathSet(eSclPath_SeamlessSw);
            dvProAV_SclVopWindowOffSet(eSclEntity_Main, false); // ProAV_Rex_0045
            dvProAV_SclVopWindowOffSet(eSclEntity_Sub, true);
            dvProAV_SclOpuScreenOffSet(eSclEntity_Main, true);
            dvProAV_SclOpuScreenOffSet(eSclEntity_Sub, true);
            dvProAV_SclVopGainBiasSubOffSet(false);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////
///@brief dvPro_GetPanelIndex : get panel table index from panel id
///@param ePANEL_ID ePanelId - panel id
///@param PUINT8 pucIndex - panel index pointer
///@return INT32 - return error code
///@author Roger Chen
///////////////////////////////////////////////////////////////////////////////////

static ePROAV_EXEC_CODE dvPro_GetPanelIndex(ePANEL_ID ePanelId, PUINT8 pucIndex)
{
    UINT8 ucIndex = 0;

    if(ePanelId < ePANEL_ID_LAST)
    {
        if(Board_SingleBoard_Get() == FALSE)
        {
            for(ucIndex = 0; ucIndex < PANEL_TABLE_NUMBER; ucIndex++)
            {
                if(m_sPanelTable[ucIndex].id == ePanelId)
                {
                    *pucIndex = ucIndex;
                    return ePROAV_EXEC_CODE_PASS;
                }
            }
        }
        else
        {
            for(ucIndex = 0; ucIndex < SINGLE_PANEL_TABLE_NUMBER; ucIndex++)
            {
                if(m_sSinglePanelTable[ucIndex].id == ePanelId)
                {
                    *pucIndex = ucIndex;
                    return ePROAV_EXEC_CODE_PASS;
                }
            }
        }
    }
    else
    {
        return ePROAV_EXEC_CODE_PANEL_ID_NOT_FOUND;
    }

    return ePROAV_EXEC_CODE_PANEL_TABEL_INDEX_NOT_FOUND;
}

UINT8 dvPro_GetPanelInfo(ePANEL_ID ePanelId, UINT8 *psPanel)
{
    UINT8 ucIndex = 0;

    PanelTiming *psPanelTable = (PanelTiming*)psPanel;


    if(ePanelId < ePANEL_ID_LAST)
    {
        if(Board_SingleBoard_Get() == FALSE)
        {
            for(ucIndex = 0; ucIndex < PANEL_TABLE_NUMBER; ucIndex++)
            {
                if(m_sPanelTable[ucIndex].id == ePanelId)
                {
                    psPanelTable->id         = m_sPanelTable[ucIndex].id;
                    psPanelTable->HTotal     = m_sPanelTable[ucIndex].HTotal;
                    psPanelTable->VTotal     = m_sPanelTable[ucIndex].VTotal;
                    psPanelTable->HSize      = m_sPanelTable[ucIndex].HSize;
                    psPanelTable->VSize      = m_sPanelTable[ucIndex].VSize;
                    psPanelTable->HStart     = m_sPanelTable[ucIndex].HStart;
                    psPanelTable->VStart     = m_sPanelTable[ucIndex].VStart;
                    psPanelTable->HSync      = m_sPanelTable[ucIndex].HSync;
                    psPanelTable->VSync      = m_sPanelTable[ucIndex].VSync;
                    psPanelTable->HPol       = m_sPanelTable[ucIndex].HPol;
                    psPanelTable->VPol       = m_sPanelTable[ucIndex].VPol;
                    psPanelTable->FrameRate  = m_sPanelTable[ucIndex].FrameRate;
                    psPanelTable->PixelClock = m_sPanelTable[ucIndex].PixelClock;

                    return ePROAV_EXEC_CODE_PASS;
                }
            }
        }
        else
        {
            for(ucIndex = 0; ucIndex < SINGLE_PANEL_TABLE_NUMBER; ucIndex++)
            {
                if(m_sSinglePanelTable[ucIndex].id == ePanelId)
                {
                    psPanelTable->id         = m_sSinglePanelTable[ucIndex].id;
                    psPanelTable->HTotal     = m_sSinglePanelTable[ucIndex].HTotal;
                    psPanelTable->VTotal     = m_sSinglePanelTable[ucIndex].VTotal;
                    psPanelTable->HSize      = m_sSinglePanelTable[ucIndex].HSize;
                    psPanelTable->VSize      = m_sSinglePanelTable[ucIndex].VSize;
                    psPanelTable->HStart     = m_sSinglePanelTable[ucIndex].HStart;
                    psPanelTable->VStart     = m_sSinglePanelTable[ucIndex].VStart;
                    psPanelTable->HSync      = m_sSinglePanelTable[ucIndex].HSync;
                    psPanelTable->VSync      = m_sSinglePanelTable[ucIndex].VSync;
                    psPanelTable->HPol       = m_sSinglePanelTable[ucIndex].HPol;
                    psPanelTable->VPol       = m_sSinglePanelTable[ucIndex].VPol;
                    psPanelTable->FrameRate  = m_sSinglePanelTable[ucIndex].FrameRate;
                    psPanelTable->PixelClock = m_sSinglePanelTable[ucIndex].PixelClock;

                    return ePROAV_EXEC_CODE_PASS;
                }
            }
        }

    }
    else
    {
        return ePROAV_EXEC_CODE_PANEL_ID_NOT_FOUND;
    }

    return ePROAV_EXEC_CODE_PANEL_TABEL_INDEX_NOT_FOUND;
}

void dvPro_SetIdual_2K(const eMCT eCH)
{
//    //dvPro_Write(B0_RTCT, RTCT_PIVSTHRU, 0);   //0x0000CBA9 //A70LV_Doulas_0002

    m_sChannelInfo[eCH].sIdual_Act_Ofst.uiHst = 0;
    m_sChannelInfo[eCH].sIdual_Act_Ofst.uiHw = 0;
    m_sChannelInfo[eCH].sIdual_Act_Ofst.uiVst = 0;
    m_sChannelInfo[eCH].sIdual_Act_Ofst.uiVw = 0;
}


int dvPro_BuildPicture_2K(const eMCT eCH)
{
    SclIpuDim ipuDim;
    SclOpuDim opuDim;
    SclIpuDim *ipu = &ipuDim;
    SclOpuDim *opu = &opuDim;
    int isIpuHalfVSzie = 0;
    int status = rcSUCCESS;
    Scl3DMode uc3DMode = eScl3DMode_Off;
    RxPort sRxPort;
    DetTmg timing = {0};
    //UINT8 cCurrentSource = 0;
    SclDataPath cDatapath = 0;

    //status &= (UINT8)dvProAV_SclBackupSourceGet(&cCurrentSource);
    cDatapath = dvProAV_SclDatapathGet();

    if (eCH >= eMCT_LAST)
        return rcERROR;

    dvPro_Calc_Scaler_InputAndOutput_Window_2K(eCH);

    ipu->HSmpSt = m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiHStart;//260;
    ipu->VSmpSt = m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiVStart;
    ipu->HSmpSize= m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiHActive;
    ipu->VSmpSize= m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiVActive;
    ipu->HWrSize = 0;//m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiHActive;
    ipu->VWrSize = 0;//m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiVActive;

    opu->HDspSt = m_sChannelInfo[eCH].sScalerOutputTimingInfo.uiHStart;
    opu->VDspSt = m_sChannelInfo[eCH].sScalerOutputTimingInfo.uiVStart;
    opu->HSize  = m_sChannelInfo[eCH].sScalerOutputTimingInfo.uiHActive;
    opu->VSize  = m_sChannelInfo[eCH].sScalerOutputTimingInfo.uiVActive;
    opu->HRdSize = 0;//m_sChannelInfo[eCH].sScalerOutputTimingInfo.uiHActive;
    opu->VRdSize = 0;//m_sChannelInfo[eCH].sScalerOutputTimingInfo.uiVActive;

#if 1
    if((eCH == eMCT_CH2) && (m_sChannelSetting[eMCT_CH1].cPIP_PBP_ON == 0))
    {
        ipu->HSmpSt = 0;
        ipu->VSmpSt = 0;
        //ipu->HSmpSize = 0;
        //ipu->VSmpSize = 0;
        ipu->HWrSize = 0;//m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiHActive;
        ipu->VWrSize = 0;//m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiVActive;
    }
#endif /* 0 */


    //uint16 arH = aspectratio >> 4;   // H factor
    //uint16 arV = aspectratio & 0x0F; // V factor

    /*if(m_sChannelInfo[eCH].sInputTimingInfo.eScanMode == eSCAN_MODE_INTERLACE)    // ProAV_Rex_0023
    {
        ipu->VSmpSt = m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiVStart/2;
        //ipu->VSmpSize = m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiVActive/2;
        //opu->VSize = m_sChannelInfo[eCH].sScalerOutputTimingInfo.uiVActive*2;
    }*/

    //if((m_sChannelInfo[eCH].sInputTimingInfo.eScanMode == eSCAN_MODE_INTERLACE) && (opu->VSize <= ipu->VSmpSize))
    if((m_sChannelInfo[eCH].sInputTimingInfo.eScanMode == eSCAN_MODE_INTERLACE))
    {
        LOG_MSG(db_DV_SCALER, "Interlace\n");

        if(opu->VSize == 0)
        {
            ASSERT_ALWAYS();
            opu->VSize = 1;
        }

        if(opu->HSize == 0)
        {
            ASSERT_ALWAYS();
            opu->HSize = 1;
        }

        if(((ipu->VSmpSize / opu->VSize) < 2) || ((ipu->HSmpSize / opu->HSize) < 2))
            status &= dvProAV_SclDeIntModeSet(eCH, eSclDeIntMode_Motion);    // ProAV_Rex_0001
        else
            status &= dvProAV_SclDeIntModeSet(eCH, eSclDeIntMode_Bob);       // ProAV_Rex_0027
    }
    else
    {
        LOG_MSG(db_DV_SCALER, "Progressive \n");
        status &= dvProAV_SclDeIntModeSet(eCH, m_sChannelInfo[eCH].sOutputTimingInfo.cDeinterlaceMode);
    }

    if(eMCT_CH1 == eCH)
    {
        if(m_sChannelSetting[eMCT_CH1].cPIP_PBP_ON == 0)
        {
            //switch(m_sChannelSetting[eMCT_CH1].eOSD_3D_Enable)
            switch(dvPro_Input_3D_Format_Config_Get())
            {
                case eINPUT_3D_TYPE_FRAMEPACKING:
                    uc3DMode = eScl3DMode_FramePacking;
                    break;

                case eINPUT_3D_TYPE_TOPANDBOTTOM:
                    uc3DMode = eScl3DMode_TopAndBottom;
                    break;

                case eINPUT_3D_TYPE_SIDEBYSIDE:
                    uc3DMode = eScl3DMode_SideBySide;
                    break;

                default:
                    uc3DMode = eScl3DMode_Off;
                    break;
            }
        }
        else
        {
            uc3DMode = eScl3DMode_Off;
        }
    }

    timing.Hs.Size = ipu->HSmpSize;
    timing.Hs.Start = ipu->HSmpSt;
    timing.Vs.Size = ipu->VSmpSize;
    timing.Vs.Start = ipu->VSmpSt;
    timing.Interlace = m_sChannelInfo[eCH].sInputTimingInfo.eScanMode;

    status &= dvProAV_SclVipSourceGet(eCH, &sRxPort);
    //ProAV
    status &= dvProAV_SclUpdatingSet(eCH, true);    // ProAV_Rex_0008
    status &= dvProAV_SrcUpdatSet(sRxPort, true);
    status &= dvProAV_SrcTimingSet(sRxPort, &timing);

    if(cDatapath == eSclPath_BackupSw)
    {
        status &= dvProAV_SrcUpdatSet(eRxPort_2, true);
        status &= dvProAV_SrcTimingSet(eRxPort_2, &timing);
    }

    status &= dvProAV_SclIpuDimSet(eCH, *ipu);
    status &= dvProAV_SclOpuDimSet(eCH, *opu);

    //printf("input hz = %f, output hz = %d\n", m_sChannelInfo[eCH].sInputTimingInfo.dFrameRate, m_sChannelInfo[eCH].sOutputTimingInfo.uiFrameRate);

    if(eMCT_CH1 == eCH)
    {
        status &= dvProAV_Src3DModeSet(sRxPort, uc3DMode);
        status &= dvPro_OSD_3D_Sync_Setting(uc3DMode);

        switch(uc3DMode)
        {
            case eScl3DMode_FramePacking:
            case eScl3DMode_SideBySide:
            case eScl3DMode_TopAndBottom:
                {
                    UINT16 uiOutputFrameRate = (UINT16)(m_sChannelInfo[eCH].sOutputTimingInfo.uiFrameRate/100);
                    UINT16 uiInputFrameRate = (UINT16)dvPro_InputRataIntegerGet(m_sChannelInfo[eCH].sInputTimingInfo.dFrameRate);
                    UINT8  ucRate = 1;

                    if(uiInputFrameRate)
                    {
                        ucRate = (UINT8)(uiOutputFrameRate/uiInputFrameRate);
                    }

                    dvProAV_WarpOSDSubFrameModFlagSet_E4K(false);

                    status &= dvProAV_Scl_4K3dLrCtl(0);
                    status &= dvProAV_4K3dDpBprnMdCtl(0);
                    status &= dvProAV_SclEx3dSyncModeSet(eSclSyncMode_Off);
                    status &= dvProAV_SclVopWindowOffSet(eSclEntity_Sub,1);
                    status &= dvProAV_SclOpuScreenOffSet(eSclEntity_Sub,1);
                    status &= dvProAV_SclFrameSeq3dModeSet(false);
                    status &= dvProAV_SclDualPipe3dModeSet(false);
                    //status &= dvProAV_Scl2dSyncEn(false);
                    status &= dvProAV_SclDatapathSet(eSclPath_Signle);
                    status &= dvProAV_SclFrameSeqInStkFrmSet((SclEntity)eCH, 0, false, false);
#ifndef VPD_XPR_ENABLE
                    if(Board_SingleBoard_Get() == FALSE)
                    {
                        status &= dvProAV_PixSft3dMdSel(ePixSftMd_2D);
                    }
#endif /* VPD_XPR_ENABLE */
                    status &= dvProAV_PixSftModSel(1);

                    if((ucRate != 1) && (ucRate % 2))
                    {
                        status &= dvProAV_Scl3D_LrFreeRun(1);
                    }
                }

                //LOG_MSG(db_ALWAYS, "e3dSyncOutSwMode_InternalSync\n");
                break;

            default:
                switch(dvPro_Input_3D_Format_Config_Get())
                {
                    case eINPUT_3D_TYPE_FRAME_SEQUENTIAL:
                        {
                            UINT16 uiOutputFrameRate = (UINT16)(m_sChannelInfo[eCH].sOutputTimingInfo.uiFrameRate/100);
                            UINT16 uiInputFrameRate = (UINT16)dvPro_InputRataIntegerGet(m_sChannelInfo[eCH].sInputTimingInfo.dFrameRate);
                            UINT8  ucMultiple = 0;
                            BOOL   bStkFrmOder = FALSE;

                            if(uiInputFrameRate)
                            {
                                //避免除0
                                ucMultiple = uiOutputFrameRate/uiInputFrameRate;

                                if(ucMultiple >= 1)
                                {
                                    if((ucMultiple % 2) ||
                                       ((uiOutputFrameRate * 10)/uiInputFrameRate) % 10)
                                    {
                                        // 1200/50 = 24
                                        ucMultiple = 0;
                                    }
                                    else
                                    {
                                        ucMultiple--;
                                    }
                                }

                                if(uiInputFrameRate == 60)
                                {
                                    bStkFrmOder = TRUE;
                                }
                            }

                            dvProAV_WarpOSDSubFrameModFlagSet_E4K(false);

                            status &= dvProAV_Scl3D_LrFreeRun(0);
                            status &= dvProAV_Scl_4K3dLrCtl(0);
                            status &= dvProAV_4K3dDpBprnMdCtl(0);
                            status &= dvProAV_SclVopWindowOffSet(eSclEntity_Sub,1);
                            status &= dvProAV_SclOpuScreenOffSet(eSclEntity_Sub,1);
                            status &= dvProAV_SclDualPipe3dModeSet(false);
                            status &= dvProAV_SclFrameSeq3dModeSet(true);
                            status &= dvProAV_SclEx3dSyncModeSet(eSclSyncMode_Off);
                            //status &= dvProAV_Scl2dSyncEn(false);
                            //status &= dvProAV_SclDatapathSet(eSclPath_Signle);
#ifndef VPD_XPR_ENABLE
                            if(Board_SingleBoard_Get() == FALSE)
                            {
                                status &= dvProAV_PixSft3dMdSel(ePixSftMd_2D);
                            }
#endif /* VPD_XPR_ENABLE */
                            status &= dvPro_PixelShift_Setting(ePIXEL_SHIFT_NORMAL);

                            if((ucMultiple) && (ucMultiple < 4) && (m_sChannelInfo[eCH].sInputTimingInfo.eScanMode == eSCAN_MODE_PROGRESSIVE))
                            {
                                status &= dvProAV_SclFrameSeqInStkFrmSet((SclEntity)eCH, ucMultiple, bStkFrmOder, true);
                            }
                            else
                            {
                                status &= dvProAV_SclFrameSeqInStkFrmSet((SclEntity)eCH, 0, false, false);
                            }
                            status &= dvProAV_PixSftModSel(1);

                            LOG_MSG(db_DV_SCALER, "FrameSeqInMult %d\n", ucMultiple);
                            LOG_MSG(db_DV_SCALER, "eINPUT_3D_TYPE_FRAME_SEQUENTIAL\n");
                        }
                        break;

                    case eINPUT_3D_TYPE_DAUL_PIPE:
                        {
                            RxPort sRxPortSub = 0;
                            SclIpuDim ipuDimSub;

                            memcpy(&ipuDimSub, &ipuDim, sizeof(SclIpuDim));

                            dvProAV_WarpOSDSubFrameModFlagSet_E4K(false);

                            status &= dvProAV_SclIpuDimDetGet(eSclEntity_Sub, &ipuDimSub);
                            status &= dvProAV_SclVipSourceGet(eSclEntity_Sub, &sRxPortSub);

                            //ipu->HSmpSt = m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiHStart;//260;
                            //ipu->VSmpSt = m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiVStart;
                            ipuDimSub.HSmpSize= m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiHActive;
                            ipuDimSub.VSmpSize= m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiVActive;
                            ipuDimSub.HWrSize = 0;//m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiHActive;
                            ipuDimSub.VWrSize = 0;//m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiVActive;

                            //Sub source
                            //Main is PROA_RX_3
                            status &= dvProAV_SclVipSourceSet(eSclEntity_Sub, PROA_RX_3);

                            status &= dvProAV_SclVopWindowOffSet(eSclEntity_Sub,0);
                            status &= dvProAV_SclOpuScreenOffSet(eSclEntity_Sub,1); //A70LK_Larry_0133

                            status &= dvProAV_SclUpdatingSet(eSclEntity_Sub, true);
                            status &= dvProAV_SrcUpdatSet(sRxPortSub, true);
                            status &= dvProAV_SrcTimingSet(sRxPortSub, &timing);
                            status &= dvProAV_SclIpuDimSet(eSclEntity_Sub, ipuDimSub);
                            status &= dvProAV_SclOpuDimSet(eSclEntity_Sub, *opu);
                            status &= dvProAV_SrcUpdatSet(sRxPortSub, false);
                            status &= dvProAV_SclUpdatingSet(eSclEntity_Sub, false);
                            status &= dvPro_Set_Sharpness(eSclEntity_Sub);

                            status &= dvProAV_SclDatapathSet(eSclPath_Pop);

                            status &= dvProAV_Scl3D_LrFreeRun(0);
                            status &= dvProAV_Scl_4K3dLrCtl(0);
                            status &= dvProAV_4K3dDpBprnMdCtl(0);
                            status &= dvProAV_SclEx3dSyncModeSet(eSclSyncMode_Off);
                            //status &= dvProAV_Scl2dSyncEn(false);
#ifndef VPD_XPR_ENABLE
                            if(Board_SingleBoard_Get() == FALSE)
                            {
                                status &= dvProAV_PixSft3dMdSel(ePixSftMd_2D);
                            }
#endif /* VPD_XPR_ENABLE */
                            status &= dvPro_PixelShift_Setting(ePIXEL_SHIFT_NORMAL);

                            status &= dvProAV_SclFrameSeq3dModeSet(false);
                            status &= dvProAV_SclDualPipe3dModeSet(true);
                            status &= dvProAV_SclFrameSeqInStkFrmSet((SclEntity)eCH, 0, false, false);
                            status &= dvProAV_PixSftModSel(1);

                            LOG_MSG(db_DV_SCALER, "eINPUT_3D_TYPE_DAUL_PIPE result:%d\n", status);
                        }
                        break;

                    case eINPUT_3D_TYPE_4K3D_SEQUENTIAL:
                        {
                            dvProAV_WarpOSDSubFrameModFlagSet_E4K(true);

                            status &= dvProAV_Scl3D_LrFreeRun(0);
                            status &= dvProAV_Scl3dLrSyncDouble(FALSE);
                            status &= dvProAV_Scl_4K3dLrCtl(2);
                            status &= dvProAV_4K3dDpBprnMdCtl(0);
                            //status &= dvProAV_Scl2dSyncEn(false);
                            //status &= dvProAV_Scl3dIntVsModeSel(0);
#ifndef VPD_XPR_ENABLE
                            if(Board_SingleBoard_Get() == FALSE)
                            {
                                status &= dvProAV_PixSft3dMdSel(ePixSftMd_4W4K3DPlus);
                            }
#endif /* VPD_XPR_ENABLE */

                            status &= dvProAV_WarpWipLineDlySet(0);//dvProAV_Warp_WopDly_2Way3D(true);
                            status &= dvPro_PixelShift_Setting(ePIXEL_SHIFT_FRAME_SEQ_PLUS);
                            status &= dvProAV_SclFrameSeqInStkFrmSet((SclEntity)eCH, 0, false, false);
                            status &= dvProAV_SclFrameSeq3dModeSet(true);
                            status &= dvProAV_SclDualPipe3dModeSet(false);
                            status &= dvProAV_PixSftModSel(0);

                            LOG_MSG(db_DV_SCALER, "eINPUT_3D_TYPE_4K3D_SEQUENTIAL result:%d\n", status);
                        }
                        break;

                    case eINPUT_3D_TYPE_4K3D_DAUL_PIPE:
                        {
                            RxPort sRxPortSub = 0;
                            SclIpuDim ipuDimSub;

                            memcpy(&ipuDimSub, &ipuDim, sizeof(SclIpuDim));

                            dvProAV_WarpOSDSubFrameModFlagSet_E4K(true);

                            status &= dvProAV_SclIpuDimDetGet(eSclEntity_Sub, &ipuDimSub);
                            status &= dvProAV_SclVipSourceGet(eSclEntity_Sub, &sRxPortSub);

                            //ipu->HSmpSt = m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiHStart;//260;
                            //ipu->VSmpSt = m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiVStart;
                            ipuDimSub.HSmpSize= m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiHActive;
                            ipuDimSub.VSmpSize= m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiVActive;
                            ipuDimSub.HWrSize = 0;//m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiHActive;
                            ipuDimSub.VWrSize = 0;//m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiVActive;

                            //Sub source
                            //Main is PROA_RX_3
                            status &= dvProAV_SclVipSourceSet(eSclEntity_Sub, PROA_RX_3);

                            status &= dvProAV_SclVopWindowOffSet(eSclEntity_Sub,0);
                            status &= dvProAV_SclOpuScreenOffSet(eSclEntity_Sub,1); //A70LK_Larry_0133

                            status &= dvProAV_SclUpdatingSet(eSclEntity_Sub, true);
                            status &= dvProAV_SrcUpdatSet(sRxPortSub, true);
                            status &= dvProAV_SrcTimingSet(sRxPortSub, &timing);
                            status &= dvProAV_SclIpuDimSet(eSclEntity_Sub, ipuDimSub);
                            status &= dvProAV_SclOpuDimSet(eSclEntity_Sub, *opu);
                            status &= dvProAV_SrcUpdatSet(sRxPortSub, false);
                            status &= dvProAV_SclUpdatingSet(eSclEntity_Sub, false);
                            status &= dvPro_Set_Sharpness(eSclEntity_Sub);

                            status &= dvProAV_SclDatapathSet(eSclPath_Pop);
                            status &= dvProAV_Scl3dLrSyncDouble(FALSE);
                            status &= dvProAV_Scl3D_LrFreeRun(0);
                            status &= dvProAV_Scl_4K3dLrCtl(1);
                            status &= dvProAV_4K3dDpBprnMdCtl(1);
                            //status &= dvProAV_Scl2dSyncEn(false);
                            //status &= dvProAV_Scl3dIntVsModeSel(0);
#ifndef VPD_XPR_ENABLE
                            if(Board_SingleBoard_Get() == FALSE)
                            {
                                status &= dvProAV_PixSft3dMdSel(ePixSftMd_2W4K3D);
                            }
#endif /* VPD_XPR_ENABLE */
                            status &= dvProAV_WarpWipLineDlySet(0);//dvProAV_Warp_WopDly_2Way3D(true);
                            status &= dvPro_PixelShift_Setting(ePIXEL_SHIFT_DUAL_PIPE_4K3D);
							//status &= dvProAV_4K3dVopSubFrmIndOdrSet(2,0,1,3);//(1,3,2,0);			//A70LK_Doulas_0017
                            status &= dvProAV_SclFrameSeqInStkFrmSet((SclEntity)eCH, 0, false, false);

                            status &= dvProAV_SclFrameSeq3dModeSet(false);
                            status &= dvProAV_SclDualPipe3dModeSet(false);
                            status &= dvProAV_PixSftModSel(0);

                            LOG_MSG(db_DV_SCALER, "eINPUT_3D_TYPE_4K3D_DAUL_PIPE result:%d\n", status);
                        }
                        break;

                    default:
                        if(m_sChannelSetting[eMCT_CH1].cPIP_PBP_ON == 0)
                        {
                            status &= dvProAV_SclVopWindowOffSet(eSclEntity_Sub,1);
                            status &= dvProAV_SclOpuScreenOffSet(eSclEntity_Sub,1);
                        }
                        dvProAV_WarpOSDSubFrameModFlagSet_E4K(false);
                        status &= dvProAV_Scl3D_LrFreeRun(0);
                        status &= dvProAV_Scl_4K3dLrCtl(0);
                        status &= dvProAV_4K3dDpBprnMdCtl(0);
                        status &= dvProAV_SclEx3dSyncModeSet(eSclSyncMode_Off);
                        status &= dvProAV_SclFrameSeq3dModeSet(false);
                        status &= dvProAV_SclDualPipe3dModeSet(false);
                        //status &= dvProAV_Scl2dSyncEn(true);
                        status &= dvProAV_SclFrameSeqInStkFrmSet((SclEntity)eCH, 0, false, false);
#ifndef VPD_XPR_ENABLE
                        if(Board_SingleBoard_Get() == FALSE)
                        {
                            status &= dvProAV_PixSft3dMdSel(ePixSftMd_2D);
                        }
#endif /* VPD_XPR_ENABLE */
                        status &= dvPro_PixelShift_Setting(ePIXEL_SHIFT_NORMAL);

                        status &= dvProAV_PixSftModSel(1);

                        break;
                }
                break;
        }
    }
    else
    {
        status &= dvProAV_SclEx3dSyncModeSet(eSclSyncMode_Off);
        status &= dvPro_OSD_3D_Sync_Setting(eScl3DMode_Off);

        if(m_sChannelInfo[eCH].sInputTimingInfo.eScanMode != eSCAN_MODE_NO_SIGNAL)
        {
            status &= dvProAV_SclVopWindowOffSet((SclEntity)eCH, false);
            status &= dvProAV_SclOpuScreenOffSet((SclEntity)eCH, false);
            status &= dvProAV_SclVopGainBiasSubOffSet(false);
        }
        else
        {
            status &= dvProAV_SclVopGainBiasSubOffSet(true);
        }

        status &= dvProAV_SclFrameSeq3dModeSet(false);
        status &= dvProAV_SclDualPipe3dModeSet(false);
    }

    status &= dvProAV_SrcUpdatSet(sRxPort, false);
    if(cDatapath == eSclPath_BackupSw)
    {
        status &= dvProAV_SrcUpdatSet(eRxPort_2, false);
    }
    status &= dvProAV_SclUpdatingSet(eCH, false);
    status &= dvPro_Set_Sharpness(eCH);

    if((eMCT_CH1 == eCH) && (cDatapath == eSclPath_BackupSw))
    {
        status &= dvPro_Set_Sharpness(eSclEntity_Sub);
    }


#if 0
    if((eMCT_CH1 == eCH) && (cCurrentSource)) //backup下要有Vsync參考才能變更size presets
    {
        status &= dvProAV_SclVipSourceSet(eCH, (sRxPort == eRxPort_0) ? eRxPort_2 : eRxPort_0);
        MS_SLEEP(200);
        status &= dvProAV_SclVipSourceSet(eCH, sRxPort);
    }
#endif /* 0 */

    LOG_MSG(db_DV_SCALER, "==============================\n");

    LOG_MSG(db_DV_SCALER, "Current Input ch: %d \n", eCH);

    LOG_MSG(db_DV_SCALER, "H Size:  %d\n", timing.Hs.Size);
    LOG_MSG(db_DV_SCALER, "H Start:  %d\n", timing.Hs.Start);
    LOG_MSG(db_DV_SCALER, "V Size: %d\n", timing.Vs.Size);
    LOG_MSG(db_DV_SCALER, "V Start: %d\n", timing.Vs.Start);
    LOG_MSG(db_DV_SCALER, "Interlace:  %d\n", timing.Interlace);

    LOG_MSG(db_DV_SCALER, "HSmpSt:  %d\n", ipu->HSmpSt);
    LOG_MSG(db_DV_SCALER, "VSmpSt:  %d\n", ipu->VSmpSt);
    LOG_MSG(db_DV_SCALER, "HSmpCnt: %d\n", ipu->HSmpSize);
    LOG_MSG(db_DV_SCALER, "VSmpCnt: %d\n", ipu->VSmpSize);
    LOG_MSG(db_DV_SCALER, "HWrCnt:  %d\n", ipu->HWrSize);
    LOG_MSG(db_DV_SCALER, "VWrCnt:  %d\n", ipu->VWrSize);

    LOG_MSG(db_DV_SCALER, "HDspSt: %d\n", opu->HDspSt);
    LOG_MSG(db_DV_SCALER, "VDspSt: %d\n", opu->VDspSt);
    LOG_MSG(db_DV_SCALER, "HSize:  %d\n", opu->HSize);
    LOG_MSG(db_DV_SCALER, "VSize:  %d\n", opu->VSize);
    LOG_MSG(db_DV_SCALER, "HRdCnt: %d\n", opu->HRdSize);
    LOG_MSG(db_DV_SCALER, "VRdCnt: %d\n", opu->VRdSize);
    LOG_MSG(db_DV_SCALER, "3DMode: %d %d %d\n", uc3DMode, ucInput3D_Format, m_sChannelSetting[eMCT_CH1].eOSD_3D_Enable);
    //LOG_MSG(db_DV_SCALER, "INTERLACE: %ld\n", m_sChannelSetting[eMCT_CH1].eScanMode);
    LOG_MSG(db_DV_SCALER, "Reseult %d\n", status);

    LOG_MSG(db_DV_SCALER, "==============================\n");

    return status;
}




//C821 Brightness data range : -511 ~ 511 (defalt:0)
INT8 dvPro_Brightness_Value_Set(UINT8 ucCH, INT16 iBrightness)    //A70LV_Doulas_0022
{
    LOG_MSG(db_DV_SCALER, "(func:%s, line:%d): Brightness=%d\r\n", __FUNCTION__, __LINE__, iBrightness);

    m_sChannelSetting[ucCH].iBrightness = iBrightness;

    return PROAV_PASS;
}


INT16 dvPro_Brightness_Value_Get ()
{
    return m_sChannelSetting[eMCT_CH1].iBrightness;
}


//C821 Contrast data range : 0 ~ 3.99 (default:1)
INT8 dvPro_Contrast_Value_Set(UINT8 ucCH, DOUBLE dContrast)   //A70LV_Doulas_0022
{

    LOG_MSG(db_DV_SCALER, "(func:%s, line:%d): Contrast=%f\r\n", __FUNCTION__, __LINE__, dContrast);

    m_sChannelSetting[ucCH].dContrast = dContrast;

    return PROAV_PASS;
}


DOUBLE dvPro_Contrast_Value_Get ()
{
    return m_sChannelSetting[eMCT_CH1].dContrast;
}


ePROAV_EXEC_CODE dvPro_Init_Panel(const eMCT eCH, const ePANEL_ID ePanelId)
{
    UINT8 ucTblIndex = 0;
    ePROAV_EXEC_CODE eExecCode = ePROAV_EXEC_CODE_PASS;
    sACTIVE_AREA sOact; //A70LV_Doulas_0002
    int status = rcSUCCESS;

    SclOpuDim opuDim;
    SclOpuDim *opu = &opuDim;
    //PanelTiming sTiming;

    LOG_MSG(db_DV_SCALER, "(func:%s, line:%d)CH= %d, PanelId = %d\r\n", __FUNCTION__, __LINE__,eCH, ePanelId);  //A70LV_Doulas_0003

    if(eCH >= eMCT_LAST)
    {
        return ePROAV_EXEC_CODE_CH_OVER_RANGE;
    }

    //To make sure channel 1 will be inited before other channels.
    //Because no matter 4K or 2K are all refer to channel PLL
    if(eCH > eMCT_CH1)
    {
        if(m_sChannelInfo[eMCT_CH1].bInit == FALSE)
        {
            return ePROAV_EXEC_CODE_CH1_NOT_INITED;
        }
    }

    if((eExecCode = dvPro_GetPanelIndex(ePanelId, &ucTblIndex)) != ePROAV_EXEC_CODE_PASS)
    {
        LOG_MSG(db_DV_SCALER, "(func:%s, line:%d) Get Panel Timing Index Fail = %d\n", __FUNCTION__, __LINE__, eExecCode);
        return eExecCode;
    }

    //TODO 20160705
    dvPro_GetCHOutputTiming_2K(eCH, ucTblIndex);

    m_sChannelInfo[eCH].bInit = TRUE;
    m_sChannelInfo[eCH].ePanelTimingId = ePanelId;

    m_sChannelInfo[eCH].ucIPPLLCT = 0x00;//0x01;        //A70LV_Doulas_0002
    m_sChannelInfo[eCH].ulInterruptEnable = 0x00000000;//0x00000F00; //A70LV_Doulas_0002 modify
    m_sChannelInfo[eCH].bForceSyncReset = TRUE;//FALSE;//TRUE; //A70LV_Doulas_0083 //A70LV_Doulas_0002     A70LV_Doulas_0154
    m_sChannelInfo[eCH].bOutputChSwap = FALSE;

    m_sChannelInfo[eCH].sInputTimingInfo.uiHFreq = 0;    //A70LV_Doulas_0005 Add
    m_sChannelInfo[eCH].sInputTimingInfo.dFrameRate = 0;
    m_sChannelInfo[eCH].sInputTimingInfo.uiHStart = 0;
    m_sChannelInfo[eCH].sInputTimingInfo.uiHActive = 0;
    m_sChannelInfo[eCH].sInputTimingInfo.uiVStart = 0;
    m_sChannelInfo[eCH].sInputTimingInfo.uiVActive = 0;
    m_sChannelInfo[eCH].sInputTimingInfo.uiHTotal = 0;
  //  m_sChannelInfo[eCH].sInputTimingInfo.uiHTotal_MCLK = 0;  //A70LV_Doulas_0005 remove
    m_sChannelInfo[eCH].sInputTimingInfo.uiVTotal = 0;
    m_sChannelInfo[eCH].sInputTimingInfo.eScanMode = eSCAN_MODE_NO_SIGNAL;

    m_sChannelInfo[eCH].ucInputPort = (UINT8)eCH;

    m_sChannelInfo[eCH].ucMCT = 0;
    m_sChannelInfo[eCH].ucISYCT = 0;
    m_sChannelInfo[eCH].ucOIMGCT = 0;
    m_sChannelInfo[eCH].ucICFMT = 0;

    m_sChannelInfo[eCH].ucPanelIndex = ucTblIndex;
    m_sChannelInfo[eCH].eInputSyncType = eINPUT_PC_SYNC;   //A70LV_Doulas_0005 Add
    m_sChannelInfo[eCH].bAutoPosition = FALSE;         //A70LV_Doulas_0118

#if (CURRENT_MAIN_BOARD == ICHIP_BOARD)
    //Should be the same with memory clock 12x66 = 792M (792/2)
    m_sChannelInfo[eCH].ulMclko_Freq = 396000000;//350000000;
#else
    //Should be the same with memory clock
    m_sChannelInfo[eCH].ulMclko_Freq = 396000000;//330000000;
#endif

//#ifndef NO_INTERFACE
    if(eCH == eMCT_CH1)
    {
        dvProAV_HdmiTxIpReset(false);

        status &= dvProAV_AccessWrite(eMiuWdtEn,    0x01);
        //status &= dvProAV_SpiMisoDelaySet(eScalerSPI_MCU1, 2); FPGA 900 NA
        // 0.
        status &= dvProAV_SclVopScreenOffBgColorSet(0);
        status &= dvProAV_SclOpuScreenOffBgColorSet((SclEntity)eCH, 0);

        // 1.
        status &= dvProAV_SclVopScreenOffSet(true);
        status &= dvProAV_SclOpuScreenOffSet((SclEntity)eCH, true);

        // 2.
        //status &= dvProAV_SclCoeffTableDl2Chip();
        status &= dvProAV_SclVipHsAlgSet((SclEntity)eCH, eSclAlg_Lanczos); // ProAV_Rex_0003
        status &= dvProAV_SclVipVsAlgSet((SclEntity)eCH, eSclAlg_Lanczos);
        status &= dvProAV_SclOpuHsAlgSet((SclEntity)eCH, eSclAlg_Lanczos);
        status &= dvProAV_SclOpuVsAlgSet((SclEntity)eCH, eSclAlg_Lanczos);

        // 3.
        status &= dvProAV_SclSharpnessInit((SclEntity)eCH);

        // 4.

        //if((eExecCode = dvPro_GetPanelIndex(ePanelId, &ucTblIndex)) != ePROAV_EXEC_CODE_PASS)
        //{
            //LOG_MSG(db_DV_SCALER, "(func:%s, line:%d) C821 Get Panel Timing Index Fail = %d\n", __FUNCTION__, __LINE__, eExecCode);
            //return eExecCode;
        //}

        //dvPro_GetCHOutputTiming_2K(eCH, ucTblIndex);


        // 4. 設定PanelID

        // 5. 將PanelId的Timing ( panelTiming )載入(panelOutputTiming)

#if 0
        sTiming.HTotal  = m_sChannelInfo[eCH].sOutputTimingInfo.uiHTotal;
        sTiming.VTotal  = m_sChannelInfo[eCH].sOutputTimingInfo.uiVTotal;
        sTiming.HSize   = m_sChannelInfo[eCH].sOutputTimingInfo.uiHActive;
        sTiming.VSize   = m_sChannelInfo[eCH].sOutputTimingInfo.uiVActive;
        sTiming.HStart  = m_sChannelInfo[eCH].sOutputTimingInfo.uiHStart;
        sTiming.VStart  = m_sChannelInfo[eCH].sOutputTimingInfo.uiVStart;
        sTiming.HSync   = m_sChannelInfo[eCH].sOutputTimingInfo.uiHSyncWidth;
        sTiming.VSync   = m_sChannelInfo[eCH].sOutputTimingInfo.uiVSyncWidth;
#endif /* 0 */

        // 6.
        status &= dvProAV_HdmiRxIpReset(true);
        status &= dvProAV_HdmiRxIpReset(false);
#if 0
#ifndef Tx_Vx1
        status &= dvProAV_HdmiTxConfigSet(m_sChannelInfo[eCH].sOutputTimingInfo.ulPixelClock , eColorDepth_bpp24);
#else
        status &= dvProAV_Vx1ReCal();
#endif /* Tx_Vx1 */
#else //FPGA 900 only v by one
        status &= dvProAV_Vx1ReCal();
#endif /* 0 */

        //status &=dvProAV_SclVopTmgSet(panelOutputTiming);

        status &= dvProAV_AccessWrite(eVopHTot,    m_sChannelInfo[eCH].sOutputTimingInfo.uiHTotal);
        status &= dvProAV_AccessWrite(eVopHsWidth, m_sChannelInfo[eCH].sOutputTimingInfo.uiHSyncWidth);
        status &= dvProAV_AccessWrite(eVopHDspSt,  m_sChannelInfo[eCH].sOutputTimingInfo.uiHStart);
        status &= dvProAV_AccessWrite(eVopHSize,   m_sChannelInfo[eCH].sOutputTimingInfo.uiHActive);
        status &= dvProAV_AccessWrite(eVopVTot,    m_sChannelInfo[eCH].sOutputTimingInfo.uiVTotal);
        status &= dvProAV_AccessWrite(eVopVsWidth, m_sChannelInfo[eCH].sOutputTimingInfo.uiVSyncWidth);
        status &= dvProAV_AccessWrite(eVopVDspSt,  m_sChannelInfo[eCH].sOutputTimingInfo.uiVStart);
        status &= dvProAV_AccessWrite(eVopVSize,   m_sChannelInfo[eCH].sOutputTimingInfo.uiVActive);

        LOG_MSG(db_DV_SCALER, "===============================\n");

        LOG_MSG(db_DV_SCALER, "Panel Output %d Timing:\n", eCH);
        LOG_MSG(db_DV_SCALER, "Set Pixel Colork =%ld, %s\n", m_sChannelInfo[eCH].sOutputTimingInfo.ulPixelClock , status == rcSUCCESS ? "PASS": "FAIL");
        LOG_MSG(db_DV_SCALER, " eVopHTot %d \n",    m_sChannelInfo[eCH].sOutputTimingInfo.uiHTotal);
        LOG_MSG(db_DV_SCALER, " eVopHsWidth %d \n",  m_sChannelInfo[eCH].sOutputTimingInfo.uiHSyncWidth);
        LOG_MSG(db_DV_SCALER, " eVopHDspSt %d \n",   m_sChannelInfo[eCH].sOutputTimingInfo.uiHStart);
        LOG_MSG(db_DV_SCALER, " eVopHSize %d \n",    m_sChannelInfo[eCH].sOutputTimingInfo.uiHActive);
        LOG_MSG(db_DV_SCALER, " eVopVTot %d \n",    m_sChannelInfo[eCH].sOutputTimingInfo.uiVTotal);
        LOG_MSG(db_DV_SCALER, " eVopVsWidth %d \n",  m_sChannelInfo[eCH].sOutputTimingInfo.uiVSyncWidth);
        LOG_MSG(db_DV_SCALER, " eVopVDspSt %d \n",   m_sChannelInfo[eCH].sOutputTimingInfo.uiVStart);
        LOG_MSG(db_DV_SCALER, " eVopVSize %d \n",    m_sChannelInfo[eCH].sOutputTimingInfo.uiVActive);
        LOG_MSG(db_DV_SCALER, "===============================\n");

        opu->HDspSt = m_sChannelInfo[eCH].sOutputTimingInfo.uiHStart;    //Opu horizontal display start
        opu->HRdSize = 0;   //Opu horizontal read size
        opu->VDspSt = m_sChannelInfo[eCH].sOutputTimingInfo.uiVStart;;    //Opu vertical display start
        opu->VRdSize = 0;   //Opu vertical read size
        opu->HSize = m_sChannelInfo[eCH].sOutputTimingInfo.uiHActive;     //Opu horizontal display size
        opu->VSize = m_sChannelInfo[eCH].sOutputTimingInfo.uiVActive;     //Opu vertical display size

        status &= dvProAV_SclOpuDimSet((SclEntity)eCH, *opu);

        status &= dvProAV_WarpImageSizeSet((uint16)ePanelId);   // set warping size // ProAV_Rex_0033

        // 7. Set Gamma Table to Vop, set after Vop Clock setting
        //DBMSG("Time After Reset =%ld, apptest line: %d\n", TimeElapsed(eTimerAfterReset), __LINE__);
        status &= dvProAV_SclVopGammaSet(false);
        //DBMSG("Time After Reset =%ld, apptest line: %d\n", TimeElapsed(eTimerAfterReset), __LINE__);

        // 8. set default layout to single
        status &= dvProAV_SclDatapathSet(eSclPath_Signle);
        status &= dvProAV_SclVopWindowOffSet(eSclEntity_Main, true);   // ProAV_Rex_0045
        status &= dvProAV_SclVopWindowOffSet(eSclEntity_Sub, true);
        // 9. set default input source from which channel
        status &= dvProAV_SclVipSourceSet((SclEntity)eCH, 0);//FPGA input channel
        //10. set Deinterlace Mode
        m_sChannelInfo[eCH].sOutputTimingInfo.cDeinterlaceMode = eSclDeIntMode_Motion;
        status &= dvProAV_SclDeIntModeSet((SclEntity)eCH, m_sChannelInfo[eCH].sOutputTimingInfo.cDeinterlaceMode);
        //DBMSG("Time After Reset =%ld, apptest line: %d\n", TimeElapsed(eTimerAfterReset), __LINE__);

        // 11. turn on the vop display
        status &= dvProAV_SclVopScreenOffSet(false);

        //set sharpness
        status &= dvProAV_SclSharpnessEnableSet((SclEntity)eCH, false);
        status &= dvProAV_SclSharpnessSet((SclEntity)eCH, 3, 2, TRUE);

        status &= dvProAV_SclIpuColorSpaceSet((SclEntity)eCH, 0);
        //status &= dvProAV_SclOpuColorSpaceSet((SclEntity)eCH, 6);

        status &= dvProAV_SclEx3dSyncModeSet(eSclSyncMode_Off);
        status &= dvProAV_SclFrameSeq3dModeSet(false);
        status &= dvProAV_SclDualPipe3dModeSet(false);
        status &= dvProAV_Scl_4K3dLrCtl(0);
        status &= dvProAV_4K3dDpBprnMdCtl(0);
        status &= dvProAV_Scl2dSyncEn(false);
        status &= dvProAV_SclFrameSeqInStkFrmSet((SclEntity)eCH, 0, false, false);
        status &= dvPro_RGBMask(0);  //A70LK_Simon_0017

#ifndef VPD_XPR_ENABLE
        if(Board_SingleBoard_Get() == FALSE)
        {
            status &= dvProAV_PixSft3dMdSel(ePixSftMd_2D);
        }
#endif /* VPD_XPR_ENABLE */
        //MEMC by pass
        status &= dvProAV_SclMemcBypassSet(true);

        //Enable sub gain/bias
        status &= dvProAV_SclVopGainBiasSubOffSet(false);

        status &= dvProAV_Scl3dIntVsModeSel(1);

#ifdef VPD_XPR_ENABLE
        //set pixshift
        status &= dvProAV_PixSftWcuEn(1);
#else
        //set pixshift
        status &= dvProAV_PixSftWcuEn(0);
#endif /* VPD_XPR_ENABLE */

        //set actuctor to TI XPR
        status &= dvProAV_PixSftModSel(1);
    }
    else
    {
        //dvProAV_HdmiTxIpReset(false);

        // 0.
        status &= dvProAV_SclOpuScreenOffBgColorSet((SclEntity)eCH, 0);

        // 1.
        status &= dvProAV_SclOpuScreenOffSet((SclEntity)eCH, true);

        //status &= dvProAV_SclCoeffTableDl2Chip();
        status &= dvProAV_SclVipHsAlgSet((SclEntity)eCH, eSclAlg_Lanczos);
        status &= dvProAV_SclVipVsAlgSet((SclEntity)eCH, eSclAlg_Lanczos);
        status &= dvProAV_SclOpuHsAlgSet((SclEntity)eCH, eSclAlg_Lanczos);
        status &= dvProAV_SclOpuVsAlgSet((SclEntity)eCH, eSclAlg_Lanczos);
        // 3.
        status &= dvProAV_SclSharpnessInit((SclEntity)eCH);

        // 4.
        //if((eExecCode = dvPro_GetPanelIndex(ePanelId, &ucTblIndex)) != ePROAV_EXEC_CODE_PASS)
        //{
            //LOG_MSG(db_DV_SCALER, "(func:%s, line:%d) C821 Get Panel Timing Index Fail = %d\n", __FUNCTION__, __LINE__, eExecCode);
            //return eExecCode;
        //}

        //dvPro_GetCHOutputTiming_2K(eCH, ucTblIndex);

        opu->HDspSt = m_sChannelInfo[eCH].sOutputTimingInfo.uiHStart;    //Opu horizontal display start
        opu->HRdSize = 0;   //Opu horizontal read size
        opu->VDspSt = m_sChannelInfo[eCH].sOutputTimingInfo.uiVStart;;    //Opu vertical display start
        opu->VRdSize = 0;   //Opu vertical read size
        opu->HSize = m_sChannelInfo[eCH].sOutputTimingInfo.uiHActive;     //Opu horizontal display size
        opu->VSize = m_sChannelInfo[eCH].sOutputTimingInfo.uiVActive;     //Opu vertical display size

        status &= dvProAV_SclOpuDimSet((SclEntity)eCH, *opu);

        // 9. set default input source from which channel
        status &= dvProAV_SclVipSourceSet((SclEntity)eCH, 1); //FPGA input channel
        //10. set Deinterlace Mode
        m_sChannelInfo[eCH].sOutputTimingInfo.cDeinterlaceMode = eSclDeIntMode_Motion;
        status &= dvProAV_SclDeIntModeSet((SclEntity)eCH, m_sChannelInfo[eCH].sOutputTimingInfo.cDeinterlaceMode);

        //set sharpness
        status &= dvProAV_SclSharpnessEnableSet((SclEntity)eCH, false);
        status &= dvProAV_SclSharpnessSet((SclEntity)eCH, 3, 2, TRUE);

        status &= dvProAV_SclIpuColorSpaceSet((SclEntity)eCH, 0);
        //status &= dvProAV_SclOpuColorSpaceSet((SclEntity)eCH, 6);
    }
//#endif /* NO_INTERFACE */



    if(status != rcSUCCESS)
    {
        LOG_MSG(db_DV_SCALER, "essInit Error CH = %d\n", eCH);
    }

    return eExecCode;
}

void dvPro_Set_HUE(eMCT eCH)  //A70LV_Doulas_0022
{
    if(m_sChannelSetting[eMCT_CH1].cPIP_PBP_ON == FALSE)        //A70LV_Doulas_0155
    {
        if(m_sChannelInfo[eMCT_CH1].ePanelTimingId == PANEL_3D_OUTPUT)
        {

        }
        else if(eCH == eMCT_CH2)
        {
            return ;
        }
    }
    dvPro_Set_Color(eCH);//A70LV_Doulas_0012 modify
}

INT16 dvPro_Get_HUE(eMCT eCH)  //A70LV_Doulas_0003
{
    return m_sChannelSetting[eCH].iHue;
}

void dvPro_Set_Saturation(eMCT eCH)  //A70LV_Doulas_0022
{
    if(m_sChannelSetting[eMCT_CH1].cPIP_PBP_ON == FALSE)        //A70LV_Doulas_0155
    {
        if(m_sChannelInfo[eMCT_CH1].ePanelTimingId == PANEL_3D_OUTPUT)
        {

        }
        else if(eCH == eMCT_CH2)
        {
            return ;
        }
    }
    dvPro_Set_Color(eCH);//A70LV_Doulas_0012 modify
}

DOUBLE dvPro_Get_Saturation(eMCT eCH)  //A70LV_Doulas_0003
{
    return m_sChannelSetting[eCH].dColor;
}

void dvPro_Set_PIP_PBP_Layout(UINT8 uiLayout)  //A70LV_Doulas_0003
{
    m_sChannelSetting[eMCT_CH1].cMain_Layout = uiLayout;     //A70LV_Doulas_0008 modify
}

void dvPro_Set_PIP_PBP_Size(UINT8 uiSize)  //A70LV_Doulas_0003
{
    LOG_MSG(db_DV_SCALER, "##Set_PIP_PBP_Size%d##\n",uiSize);
    m_sChannelSetting[eMCT_CH1].cPIP_Size = uiSize;     //A70LV_Doulas_0008 modify
}

UINT8 dvPro_Get_PIP_PBP_Layout(void)  //A70LV_Doulas_0003
{
    return m_sChannelSetting[eMCT_CH1].cMain_Layout;
}

UINT8 dvPro_Get_PIP_PBP_Size(void)  //A70LV_Doulas_0003
{
    return m_sChannelSetting[eMCT_CH1].cPIP_Size;
}

void dvPro_Resync_Init(eMCT eCH)   //A70LV_Doulas_0004
{
    ucNo_Signal_Config[eCH] = 0;
    m_sChannelInfo[eCH].sInputTimingInfo.eScanMode = eSCAN_MODE_NO_SIGNAL;  //A70LV_Doulas_0005
}

BOOL dvPro_Panel_Change_Setting(eMCT eCH,const ePANEL_ID ePanelId) //A70LV_Doulas_0005
{
    UINT32 g_RTCT;
    UINT8 ucTblIndex = 0;
    ePROAV_EXEC_CODE eExecCode = ePROAV_EXEC_CODE_PASS;
    int status = rcSUCCESS;
    PanelTiming sTiming;

    SclOpuDim opuDim;
    SclOpuDim *opu = &opuDim;

    if((eExecCode = dvPro_GetPanelIndex(ePanelId, &ucTblIndex)) != ePROAV_EXEC_CODE_PASS)
    {
        LOG_MSG(db_DV_SCALER, "(func:%s, line:%d) C821 Get Panel Timing Index Fail = %d\n", __FUNCTION__, __LINE__, eExecCode);
        return eExecCode;
    }
    dvPro_GetCHOutputTiming_2K(eMCT_CH1, ucTblIndex);
    dvPro_GetCHOutputTiming_2K(eMCT_CH2, ucTblIndex);

    m_sChannelInfo[eMCT_CH1].ePanelTimingId = ePanelId;
    m_sChannelInfo[eMCT_CH1].sInputTimingInfo.eScanMode = eSCAN_MODE_NO_SIGNAL;
    m_sChannelInfo[eMCT_CH1].ucPanelIndex = ucTblIndex;
    m_sChannelInfo[eMCT_CH2].ePanelTimingId = ePanelId;
    m_sChannelInfo[eMCT_CH2].sInputTimingInfo.eScanMode = eSCAN_MODE_NO_SIGNAL;
    m_sChannelInfo[eMCT_CH2].ucPanelIndex = ucTblIndex;

    switch(dvPro_Input_3D_Format_Config_Get())        //A70LV_Doulas_0154
    {
        case eINPUT_3D_TYPE_FRAMEPACKING:
        case eINPUT_3D_TYPE_SIDEBYSIDE:
        case eINPUT_3D_TYPE_TOPANDBOTTOM:
            m_sChannelInfo[eMCT_CH2].ucInputPort = eMCT_CH1;
            break;

        default:
            m_sChannelInfo[eMCT_CH2].ucInputPort = eMCT_CH2;
            break;
    }

    // 1.
    status &= dvProAV_SclVopScreenOffSet(true);
    status &= dvProAV_SclOpuScreenOffSet(eSclEntity_Main, true);
    status &= dvProAV_SclOpuScreenOffSet(eSclEntity_Sub, true);

    status &= dvProAV_SclVopWindowOffSet(eSclEntity_Main, true);
    status &= dvProAV_SclVopWindowOffSet(eSclEntity_Sub, true);

    // 6.
    status &= dvProAV_HdmiRxIpReset(true);
    status &= dvProAV_HdmiRxIpReset(false);

#if 0
#ifndef Tx_Vx1
    status &= dvProAV_HdmiTxConfigSet(m_sChannelInfo[eCH].sOutputTimingInfo.ulPixelClock , eColorDepth_bpp24);
#else
    status &= dvProAV_Vx1ReCal();
#endif /* Tx_Vx1 */
#else //FPGA 900 only v by one
    status &= dvProAV_Vx1ReCal();
#endif /* 0 */

    status &= dvProAV_SclEx3dSyncModeSet(eSclSyncMode_Off);
    MS_SLEEP(50);
    status &= dvProAV_WarpDownScalingIssueWorkAround();

    //status &=dvProAV_SclVopTmgSet(panelOutputTiming);

    status &= dvProAV_AccessWrite(eVopHTot,    m_sChannelInfo[eCH].sOutputTimingInfo.uiHTotal);
    status &= dvProAV_AccessWrite(eVopHsWidth, m_sChannelInfo[eCH].sOutputTimingInfo.uiHSyncWidth);
    status &= dvProAV_AccessWrite(eVopHDspSt,  m_sChannelInfo[eCH].sOutputTimingInfo.uiHStart);
    status &= dvProAV_AccessWrite(eVopHSize,   m_sChannelInfo[eCH].sOutputTimingInfo.uiHActive);
    status &= dvProAV_AccessWrite(eVopVTot,    m_sChannelInfo[eCH].sOutputTimingInfo.uiVTotal);
    status &= dvProAV_AccessWrite(eVopVsWidth, m_sChannelInfo[eCH].sOutputTimingInfo.uiVSyncWidth);
    status &= dvProAV_AccessWrite(eVopVDspSt,  m_sChannelInfo[eCH].sOutputTimingInfo.uiVStart);
    status &= dvProAV_AccessWrite(eVopVSize,   m_sChannelInfo[eCH].sOutputTimingInfo.uiVActive);

    LOG_MSG(db_DV_SCALER, "===============================\n");
    LOG_MSG(db_DV_SCALER, "===============================\n");
    LOG_MSG(db_DV_SCALER, "Panel Change Output %d Timing:\n", eCH);
    LOG_MSG(db_DV_SCALER, "Set Pixel Colork =%ld, %s\n", m_sChannelInfo[eCH].sOutputTimingInfo.ulPixelClock , status == rcSUCCESS ? "PASS": "FAIL");
    LOG_MSG(db_DV_SCALER, " eVopHTot %d \n",    m_sChannelInfo[eCH].sOutputTimingInfo.uiHTotal);
    LOG_MSG(db_DV_SCALER, " eVopHsWidth %d \n",  m_sChannelInfo[eCH].sOutputTimingInfo.uiHSyncWidth);
    LOG_MSG(db_DV_SCALER, " eVopHDspSt %d \n",   m_sChannelInfo[eCH].sOutputTimingInfo.uiHStart);
    LOG_MSG(db_DV_SCALER, " eVopHSize %d \n",    m_sChannelInfo[eCH].sOutputTimingInfo.uiHActive);
    LOG_MSG(db_DV_SCALER, " eVopVTot %d \n",    m_sChannelInfo[eCH].sOutputTimingInfo.uiVTotal);
    LOG_MSG(db_DV_SCALER, " eVopVsWidth %d \n",  m_sChannelInfo[eCH].sOutputTimingInfo.uiVSyncWidth);
    LOG_MSG(db_DV_SCALER, " eVopVDspSt %d \n",   m_sChannelInfo[eCH].sOutputTimingInfo.uiVStart);
    LOG_MSG(db_DV_SCALER, " eVopVSize %d \n",    m_sChannelInfo[eCH].sOutputTimingInfo.uiVActive);
    LOG_MSG(db_DV_SCALER, "===============================\n");
    LOG_MSG(db_DV_SCALER, "===============================\n");

    opu->HDspSt = m_sChannelInfo[eCH].sOutputTimingInfo.uiHStart;    //Opu horizontal display start
    opu->HRdSize = 0;   //Opu horizontal read size
    opu->VDspSt = m_sChannelInfo[eCH].sOutputTimingInfo.uiVStart;;    //Opu vertical display start
    opu->VRdSize = 0;   //Opu vertical read size
    opu->HSize = m_sChannelInfo[eCH].sOutputTimingInfo.uiHActive;     //Opu horizontal display size
    opu->VSize = m_sChannelInfo[eCH].sOutputTimingInfo.uiVActive;     //Opu vertical display size

    status &= dvProAV_SclOpuDimSet((SclEntity)eCH, *opu);

    //status &= dvProAV_WarpImageSizeSet((uint16)ePanelId);   // set warping size

    return eExecCode;
}

BOOL dvPro_AutoPositionSetting(const eMCT eCH)   //A70LV_Doulas_0005
{
    //BOOL bAutoPositionEnable = FALSE;
    if(eCH >= eMCT_LAST)
    {
        return FALSE;
    }

    if(bModeAdjustmentTableUse == TRUE)     //A70LV_Doulas_0195
    {
        return FALSE;
    }

    if(m_sChannelInfo[eMCT_CH1].ePanelTimingId == PANEL_3D_OUTPUT)        //A70LV_Doulas_0195
    {
        return FALSE;
    }

    if(m_sChannelInfo[eCH].bAutoPosition == TRUE)          //A70LV_Doulas_0210 modify
    {
       // if((m_sChannelInfo[eCH].sInputTimingInfo.uiVActive == 288) &&
       //    (m_sChannelInfo[eCH].sInputTimingInfo.uiHActive == 720 || m_sChannelInfo[eCH].sInputTimingInfo.uiHActive == 1440) &&
       //    (m_sChannelInfo[eCH].sInterlace_InputTimingInfo.eScanMode == eSCAN_MODE_INTERLACE))
        if(m_sChannelSetting[eCH].eVGA_SYNC_TYPE == eVGA_SYNC_TYPE_SOG)
        {
            return FALSE;
        }
        else
        {
            return TRUE;
        }
    }
    else
    {
        return FALSE;
    }
}

UINT32 dvPro_GetAPL(const eMCT eCH,UINT8 *ucMax,UINT8 *ucMin)   //A70LV_Doulas_0215 modify//A70LV_Doulas_0007
{
    UINT32 ucAPL = 0;
    if(eCH >= eMCT_LAST)
    {
        return FALSE;
    }



    if(eCH == eMCT_CH1)
    {
        //ucAPL = dvPro_Read(B35_IAPL0CH1, 0);
        //*ucMax = dvPro_Read(B35_IMAXFILTRCH1, 0);
        //*ucMin = dvPro_Read(B35_IMINFILTRCH1, 0);
        LOG_MSG(db_DV_SCALER, "(func:%s, line:%d) Phase(%02d)=(%x)(%d,%d)\r\n", __FUNCTION__, __LINE__,m_sChannelInfo[eCH].sAutoPhase.uiCurrentPhase,ucAPL,*ucMax,*ucMin);  //A70LV_Doulas_0116
    }
    else
    {
        //ucAPL = dvPro_Read(B35_IAPL0CH2, 0);
        //*ucMax = dvPro_Read(B35_IMAXFILTRCH2, 0);
        //*ucMin = dvPro_Read(B35_IMINFILTRCH2, 0);
        LOG_MSG(db_DV_SCALER, "(func:%s, line:%d) Phase(%02d)=(%x)(%d,%d)\r\n", __FUNCTION__, __LINE__,m_sChannelInfo[eCH].sAutoPhase.uiCurrentPhase,ucAPL,*ucMax,*ucMin);  //A70LV_Doulas_0116
    }



    return ucAPL;
}

ePROAV_EXEC_CODE dvPro_AutoPhaseStart(const eMCT eCH)   //A70LV_Doulas_0007
{
    ePROAV_EXEC_CODE eExecCode = ePROAV_EXEC_CODE_PASS;

    if(dvPro_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_OFF)    //A70LV_Doulas_0154 modify
    {
#if 0
        if(m_sChannelSetting[eCH].eInputSource == eINPUT_SOURCE_VGA)   //A70LV_Doulas_0013 modify
        {
            m_sChannelInfo[eCH].sAutoPhase.uiBestPAL = 0;
            m_sChannelInfo[eCH].sAutoPhase.uiBestPhase = 0;
            m_sChannelInfo[eCH].sAutoPhase.uiCurrentPhase = 0;
            m_sChannelInfo[eCH].sAutoPhase.ucBestMax = 0;       //A70LV_Doulas_0215
            m_sChannelInfo[eCH].sAutoPhase.ucBestMin = 0;       //A70LV_Doulas_0215
            dvPro_BuildPicture_2K(eCH);    //A70LV_Doulas_0116
        }
        else
#endif /* 0 */
            eExecCode = ePROAV_EXEC_CODE_FAIL;
    }
    else
    {
        eExecCode = ePROAV_EXEC_CODE_FAIL;
    }

    return eExecCode;
}

ePROAV_EXEC_CODE dvPro_AutoPhase(const eMCT eCH)    //A70LV_Doulas_0007
{
    ePROAV_EXEC_CODE eExecCode;
    UINT32 ucAPL = 0;
    UINT8  ucMax = 0;   //A70LV_Doulas_0215
    UINT8  ucMin = 0;   //A70LV_Doulas_0215

    if(m_sChannelInfo[eCH].sAutoPhase.uiCurrentPhase <= VGA_MAX_PHASE)
    {
        #if 1       //A70LV_Doulas_0215 modify auto phase algorithm
        ucAPL = dvPro_GetAPL(eCH,&ucMax,&ucMin);
        if(ucMax > m_sChannelInfo[eCH].sAutoPhase.ucBestMax)
        {
            m_sChannelInfo[eCH].sAutoPhase.ucBestMax = ucMax;
            m_sChannelInfo[eCH].sAutoPhase.ucBestMin = ucMin;
            m_sChannelInfo[eCH].sAutoPhase.uiBestPAL = ucAPL;
            m_sChannelInfo[eCH].sAutoPhase.uiBestPhase = m_sChannelInfo[eCH].sAutoPhase.uiCurrentPhase;
        }
        else if(ucMax == m_sChannelInfo[eCH].sAutoPhase.ucBestMax)
        {
            if(ucAPL > m_sChannelInfo[eCH].sAutoPhase.uiBestPAL )
            {
                m_sChannelInfo[eCH].sAutoPhase.ucBestMax = ucMax;
                m_sChannelInfo[eCH].sAutoPhase.ucBestMin = ucMin;
                m_sChannelInfo[eCH].sAutoPhase.uiBestPAL = ucAPL;
                m_sChannelInfo[eCH].sAutoPhase.uiBestPhase = m_sChannelInfo[eCH].sAutoPhase.uiCurrentPhase;
            }
        }
        #else
        ucAPL = dvPro_GetAPL(eCH,&ucMax,&ucMin);
        if(ucAPL > m_sChannelInfo[eCH].sAutoPhase.uiBestPAL )
        {
            m_sChannelInfo[eCH].sAutoPhase.uiBestPAL = ucAPL;
            m_sChannelInfo[eCH].sAutoPhase.uiBestPhase = m_sChannelInfo[eCH].sAutoPhase.uiCurrentPhase;
        }
        #endif
        m_sChannelInfo[eCH].sAutoPhase.uiCurrentPhase++;
        eExecCode = ePROAV_EXEC_CODE_CH_AUTO_PHASE;
    }
    else
    {
        eExecCode = ePROAV_EXEC_CODE_PASS;
    }

    return eExecCode;
}

UINT8 dvPro_CurrentPhaseGet(const eMCT eCH)   //A70LV_Doulas_0007
{
    return m_sChannelInfo[eCH].sAutoPhase.uiCurrentPhase;
}

UINT8 dvPro_BsetPhaseGet(const eMCT eCH)   //A70LV_Doulas_0007
{
    LOG_MSG(db_DV_SCALER, "(func:%s, line:%d) BestPhase=%d\r\n", __FUNCTION__, __LINE__,m_sChannelInfo[eCH].sAutoPhase.uiBestPhase);   //A70LV_Doulas_0211
    return m_sChannelInfo[eCH].sAutoPhase.uiBestPhase;
}

void dvPro_InputSourceSet(const eMCT eCH,UINT8 ucInputSource)   //A70LV_Doulas_0007
{
    m_sChannelSetting[eCH].eInputSource = (eCM_SOURCE_ID)ucInputSource;    //A70LV_Doulas_0013 modify
}

void dvPro_SetContrast(const eMCT eCH) //A70LV_Doulas_0011 modify
{
    UINT16 uiContrast= 0;

    if(eCH >= eMCT_LAST)
    {
        return;
    }

    if(m_sChannelSetting[eMCT_CH1].cPIP_PBP_ON == FALSE)
    {
        if(m_sChannelInfo[eMCT_CH1].ePanelTimingId == PANEL_3D_OUTPUT)    //A70LV_Doulas_0155 modify
        {

        }
        else if(eCH == eMCT_CH2)
        {
            return ;
        }
    }

    uiContrast = (UINT16)m_sChannelSetting[eCH].dContrast;

    dvProAV_SclVopBkeContrastSet(eCH, uiContrast);
}

void dvPro_Scale_Overscan(eOVER_SCAN_TYPE ucOverScan,PsINPUT_TIMING_INFO psScalerInput,PsOUTPUT_TIMING_INFO psScalerOutput) //A70LV_Doulas_0009
{
    UINT16 wOverscanPercentage, wPixels;

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

ePROAV_EXEC_CODE dvPro_Overscan_Set(const eMCT eCH,eOVER_SCAN_TYPE ucOverScan)    //A70LV_Doulas_0009
{
    ePROAV_EXEC_CODE eResult = ePROAV_EXEC_CODE_PASS;

    //if(m_sChannelSetting[eMCT_CH1].cPIP_PBP_ON == TRUE)     //A70LV_Doulas_0155 remove
    //    return ePROAV_EXEC_CODE_FAIL;

    m_sChannelSetting[eCH].eOverScan = ucOverScan;
    return eResult;
}

ePROAV_EXEC_CODE dvPro_Digital_Horz_Zoom_Set(const eMCT eCH,UINT16 uiDigital_Horz_Zoom)    //A70LV_Doulas_0009
{
    ePROAV_EXEC_CODE eResult = ePROAV_EXEC_CODE_PASS;

    //if(m_sChannelSetting[eMCT_CH1].cPIP_PBP_ON == TRUE)     //A70LV_Doulas_0155 remove
    //    return ePROAV_EXEC_CODE_FAIL;

    m_sChannelSetting[eCH].uiDigitalHorzZoom = uiDigital_Horz_Zoom;

    return eResult;
}

ePROAV_EXEC_CODE dvPro_Digital_Vert_Zoom_Set(const eMCT eCH,UINT16 uiDigital_Vert_Zoom)    //A70LV_Doulas_0009
{
    ePROAV_EXEC_CODE eResult = ePROAV_EXEC_CODE_PASS;

    //if(m_sChannelSetting[eMCT_CH1].cPIP_PBP_ON == TRUE)     //A70LV_Doulas_0155 remove
    //    return ePROAV_EXEC_CODE_FAIL;

    m_sChannelSetting[eCH].uiDigitalVertZoom = uiDigital_Vert_Zoom;

    return eResult;
}

ePROAV_EXEC_CODE dvPro_Digital_Horz_Shift_Set(const eMCT eCH,UINT16 uiDigital_Horz_Shift)    //A70LV_Doulas_0009
{
    ePROAV_EXEC_CODE eResult = ePROAV_EXEC_CODE_PASS;

    //if(m_sChannelSetting[eMCT_CH1].cPIP_PBP_ON == TRUE)     //A70LV_Doulas_0155 remove
    //    return ePROAV_EXEC_CODE_FAIL;

    m_sChannelSetting[eCH].uiDigitalHorzShift = uiDigital_Horz_Shift;

    return eResult;
}

ePROAV_EXEC_CODE dvPro_Digital_Vert_Shift_Set(const eMCT eCH,UINT16 uiDigital_Vert_Shift)    //A70LV_Doulas_0009
{
    ePROAV_EXEC_CODE eResult = ePROAV_EXEC_CODE_PASS;

    //if(m_sChannelSetting[eMCT_CH1].cPIP_PBP_ON == TRUE)     //A70LV_Doulas_0155 remove
    //    return ePROAV_EXEC_CODE_FAIL;

    m_sChannelSetting[eCH].uiDigitalVertShift = uiDigital_Vert_Shift;

    return eResult;
}

ePROAV_EXEC_CODE dvPro_Digital_Horz_Zoom_Get(const eMCT eCH,UINT16 *uiDigital_Horz_Zoom)    //A70LV_Doulas_0009
{
    ePROAV_EXEC_CODE eResult = ePROAV_EXEC_CODE_PASS;

    //if(m_sChannelSetting[eMCT_CH1].cPIP_PBP_ON == TRUE)     //A70LV_Doulas_0155 remove
    //    return ePROAV_EXEC_CODE_FAIL;

    *uiDigital_Horz_Zoom = m_sChannelSetting[eCH].uiDigitalHorzZoom;

    return eResult;
}

ePROAV_EXEC_CODE dvPro_Digital_Vert_Zoom_Get(const eMCT eCH,UINT16 *uiDigital_Vert_Zoom)    //A70LV_Doulas_0009
{
    ePROAV_EXEC_CODE eResult = ePROAV_EXEC_CODE_PASS;

    //if(m_sChannelSetting[eMCT_CH1].cPIP_PBP_ON == TRUE)     //A70LV_Doulas_0155 remove
    //    return ePROAV_EXEC_CODE_FAIL;

    *uiDigital_Vert_Zoom = m_sChannelSetting[eCH].uiDigitalVertZoom;

    return eResult;
}

ePROAV_EXEC_CODE dvPro_Digital_Horz_Shift_Get(const eMCT eCH,UINT16 *uiDigital_Horz_Shift)    //A70LV_Doulas_0009
{
    ePROAV_EXEC_CODE eResult = ePROAV_EXEC_CODE_PASS;

    //if(m_sChannelSetting[eMCT_CH1].cPIP_PBP_ON == TRUE)     //A70LV_Doulas_0155 remove
    //    return ePROAV_EXEC_CODE_FAIL;

    *uiDigital_Horz_Shift = m_sChannelSetting[eCH].uiDigitalHorzShift;

    return eResult;
}

ePROAV_EXEC_CODE dvPro_Digital_Vert_Shift_Get(const eMCT eCH,UINT16 *uiDigital_Vert_Shift)    //A70LV_Doulas_0009
{
    ePROAV_EXEC_CODE eResult = ePROAV_EXEC_CODE_PASS;

    //if(m_sChannelSetting[eMCT_CH1].cPIP_PBP_ON == TRUE)     //A70LV_Doulas_0155 remove
    //    return ePROAV_EXEC_CODE_FAIL;

    *uiDigital_Vert_Shift = m_sChannelSetting[eCH].uiDigitalVertShift;

    return eResult;
}

void dvPro_Scale_Digital_Zoom(PsINPUT_TIMING_INFO psScalerInput,PsOUTPUT_TIMING_INFO psScalerOutput)   //A70LV_Doulas_0009
{
    UINT32 udwMagnifyPlusShiftX = 0;
    UINT32 udwMagnifyPlusShiftY = 0;
    UINT16 udwPixels;

    //caculate scaler input
    if((m_sChannelSetting[eMCT_CH1].uiDigitalHorzZoom > DIGITAL_HORZ_ZOOM_DEFAULT) ||
       (m_sChannelSetting[eMCT_CH1].uiDigitalVertZoom > DIGITAL_VERT_ZOOM_DEFAULT))
    {
        if(m_sChannelSetting[eMCT_CH1].uiDigitalHorzZoom > DIGITAL_HORZ_ZOOM_DEFAULT)  //Horz
        {
            if(m_sChannelSetting[eMCT_CH1].uiDigitalHorzZoom == 0)
            {
                ASSERT_ALWAYS();
                m_sChannelSetting[eMCT_CH1].uiDigitalHorzZoom = 1;
            }

            udwPixels = (UINT16)(((UINT32)psScalerInput->uiHActive * 100) / (UINT32)m_sChannelSetting[eMCT_CH1].uiDigitalHorzZoom) & 0xFFFE;

            udwMagnifyPlusShiftX = (UINT32)(psScalerInput->uiHActive - udwPixels);
            udwMagnifyPlusShiftX = (udwMagnifyPlusShiftX * (UINT32)(m_sChannelSetting[eMCT_CH1].uiDigitalHorzShift - DIGITAL_HORZ_SHIFT_MIN))/(DIGITAL_HORZ_SHIFT_MAX - DIGITAL_HORZ_SHIFT_MIN);

            psScalerInput->uiHStart += (UINT16)udwMagnifyPlusShiftX;
            psScalerInput->uiHActive = udwPixels;
        }

        if(m_sChannelSetting[eMCT_CH1].uiDigitalVertZoom > DIGITAL_VERT_ZOOM_DEFAULT)  //Vert
        {
            if(m_sChannelSetting[eMCT_CH1].uiDigitalVertZoom == 0)
            {
                ASSERT_ALWAYS();
                m_sChannelSetting[eMCT_CH1].uiDigitalVertZoom = 1;
            }

            udwPixels = (UINT16)(((UINT32)psScalerInput->uiVActive * 100) / (UINT32)m_sChannelSetting[eMCT_CH1].uiDigitalVertZoom) & 0xFFFE;

            udwMagnifyPlusShiftY = (UINT32)(psScalerInput->uiVActive - udwPixels);
            udwMagnifyPlusShiftY = (udwMagnifyPlusShiftY * (UINT32)(DIGITAL_VERT_SHIFT_MAX - m_sChannelSetting[eMCT_CH1].uiDigitalVertShift))/(DIGITAL_VERT_SHIFT_MAX - DIGITAL_VERT_SHIFT_MIN);  //A70LV_Doulas_0010 modify


            if(psScalerInput->eScanMode == eSCAN_MODE_INTERLACE)
            {
                psScalerInput->uiVStart += (UINT16)(udwMagnifyPlusShiftY + 1)/2;
            }
            else
            {
                psScalerInput->uiVStart += (UINT16)udwMagnifyPlusShiftY;
            }
            psScalerInput->uiVActive = udwPixels;
        }
    }

    //caculate scaler output
    if((m_sChannelSetting[eMCT_CH1].uiDigitalHorzZoom < DIGITAL_HORZ_ZOOM_DEFAULT) ||
       (m_sChannelSetting[eMCT_CH1].uiDigitalVertZoom < DIGITAL_VERT_ZOOM_DEFAULT))
    {
        if(m_sChannelSetting[eMCT_CH1].uiDigitalHorzZoom < DIGITAL_HORZ_ZOOM_DEFAULT) //Horz
        {
            udwPixels = (UINT16)(((UINT32)psScalerOutput->uiHActive * (UINT32)m_sChannelSetting[eMCT_CH1].uiDigitalHorzZoom)/100);

            udwMagnifyPlusShiftX = (UINT32)(psScalerOutput->uiHActive - udwPixels);
            udwMagnifyPlusShiftX = (udwMagnifyPlusShiftX * (UINT32)(m_sChannelSetting[eMCT_CH1].uiDigitalHorzShift - DIGITAL_HORZ_SHIFT_MIN))/(DIGITAL_HORZ_SHIFT_MAX - DIGITAL_HORZ_SHIFT_MIN);

            psScalerOutput->uiHStart += (UINT16)udwMagnifyPlusShiftX;
            psScalerOutput->uiHActive = udwPixels;
        }

        if(m_sChannelSetting[eMCT_CH1].uiDigitalVertZoom < DIGITAL_VERT_ZOOM_DEFAULT) //Vert
        {
            udwPixels = (UINT16)(((UINT32)psScalerOutput->uiVActive * (UINT32)m_sChannelSetting[eMCT_CH1].uiDigitalVertZoom)/100);

            udwMagnifyPlusShiftY = (UINT32)(psScalerOutput->uiVActive - udwPixels);
            udwMagnifyPlusShiftY = (udwMagnifyPlusShiftY * (UINT32)(DIGITAL_VERT_SHIFT_MAX - m_sChannelSetting[eMCT_CH1].uiDigitalVertShift))/(DIGITAL_VERT_SHIFT_MAX - DIGITAL_VERT_SHIFT_MIN);  //A70LV_Doulas_0010 modify

            psScalerOutput->uiVStart += (UINT16)udwMagnifyPlusShiftY;
            psScalerOutput->uiVActive = udwPixels;
        }
    }
}

void dvPro_Scale_Digital_Zoom_Custom(PsINPUT_TIMING_INFO psScalerInput,PsOUTPUT_TIMING_INFO psScalerOutput)   //A70LV_Doulas_0010
{
    UINT32 udwMagnifyPlusShiftX = 0;
    UINT32 udwMagnifyPlusShiftY = 0;
    UINT16 udwOutputWidth;
    UINT16 udwOutputHeight;

    udwOutputWidth = (UINT16)(((UINT32)psScalerOutput->uiHActive * (UINT32)m_sChannelSetting[eMCT_CH1].uiDigitalHorzZoom)/DIGITAL_HORZ_ZOOM_DEFAULT);
    udwOutputHeight = (UINT16)(((UINT32)psScalerOutput->uiVActive * (UINT32)m_sChannelSetting[eMCT_CH1].uiDigitalVertZoom)/DIGITAL_VERT_ZOOM_DEFAULT);

    //caculate scaler output
    if(udwOutputWidth > m_sChannelInfo[eMCT_CH1].sOutputTimingInfo.uiHActive)
    {
        psScalerOutput->uiHStart  = m_sChannelInfo[eMCT_CH1].sOutputTimingInfo.uiHStart;
        psScalerOutput->uiHActive = m_sChannelInfo[eMCT_CH1].sOutputTimingInfo.uiHActive;
    }
    else
    {
        udwMagnifyPlusShiftX = (UINT32)(m_sChannelInfo[eMCT_CH1].sOutputTimingInfo.uiHActive - udwOutputWidth);
        udwMagnifyPlusShiftX = (udwMagnifyPlusShiftX * (UINT32)(m_sChannelSetting[eMCT_CH1].uiDigitalHorzShift - DIGITAL_HORZ_SHIFT_MIN))/(DIGITAL_HORZ_SHIFT_MAX - DIGITAL_HORZ_SHIFT_MIN);

        psScalerOutput->uiHStart = (UINT16)udwMagnifyPlusShiftX + m_sChannelInfo[eMCT_CH1].sOutputTimingInfo.uiHStart;
        psScalerOutput->uiHActive = udwOutputWidth;
    }

    if(udwOutputHeight > m_sChannelInfo[eMCT_CH1].sOutputTimingInfo.uiVActive)
    {
        psScalerOutput->uiVStart  = m_sChannelInfo[eMCT_CH1].sOutputTimingInfo.uiVStart;
        psScalerOutput->uiVActive = m_sChannelInfo[eMCT_CH1].sOutputTimingInfo.uiVActive;
    }
    else
    {
        udwMagnifyPlusShiftY = (UINT32)(m_sChannelInfo[eMCT_CH1].sOutputTimingInfo.uiVActive - udwOutputHeight);
        udwMagnifyPlusShiftY = (udwMagnifyPlusShiftY * (UINT32)(DIGITAL_VERT_SHIFT_MAX - m_sChannelSetting[eMCT_CH1].uiDigitalVertShift))/(DIGITAL_VERT_SHIFT_MAX - DIGITAL_VERT_SHIFT_MIN);

        psScalerOutput->uiVStart = (UINT16)udwMagnifyPlusShiftY + m_sChannelInfo[eMCT_CH1].sOutputTimingInfo.uiVStart;
        psScalerOutput->uiVActive = udwOutputHeight;
    }
}

void dvPro_Scale_Setting_Check(const eMCT eCH,PsINPUT_TIMING_INFO psScalerInput,PsOUTPUT_TIMING_INFO psScalerOutput) //A70LV_Doulas_0010
{
    if(psScalerOutput->uiHStart < m_sChannelInfo[eCH].sOutputTimingInfo.uiHStart)
        psScalerOutput->uiHStart = m_sChannelInfo[eCH].sOutputTimingInfo.uiHStart;

    if(psScalerOutput->uiVStart < m_sChannelInfo[eCH].sOutputTimingInfo.uiVStart)
        psScalerOutput->uiVStart = m_sChannelInfo[eCH].sOutputTimingInfo.uiVStart;

    if(psScalerOutput->uiHActive > m_sChannelInfo[eCH].sOutputTimingInfo.uiHActive)
        psScalerOutput->uiHActive = m_sChannelInfo[eCH].sOutputTimingInfo.uiHActive;

    if(psScalerOutput->uiVActive > m_sChannelInfo[eCH].sOutputTimingInfo.uiVActive)
        psScalerOutput->uiVActive = m_sChannelInfo[eCH].sOutputTimingInfo.uiVActive;

    if( (INT16)psScalerInput->uiHStart < HORZ_POSITION_LIMIT_MIN) //A70LV_Doulas_0113 modify position work//A70LV_Doulas_0066 modify image error
    {
        INT16 uiVal = HORZ_POSITION_LIMIT_MIN - (INT16)psScalerInput->uiHStart;
        WorkingHorzPosition[eCH] = m_sChannelSetting[eCH].ucHoriPosition - (UINT8)uiVal;
        psScalerInput->uiHStart = HORZ_POSITION_LIMIT_MIN;
    }
    else
    {
        WorkingHorzPosition[eCH] = m_sChannelSetting[eCH].ucHoriPosition;
    }

    if( (INT16)psScalerInput->uiVStart < VERT_POSITION_LIMIT_MIN_PRO)
    {
        INT16 uiVal = VERT_POSITION_LIMIT_MIN_PRO - (INT16)psScalerInput->uiVStart;
        WorkingVertPosition[eCH] = m_sChannelSetting[eCH].ucVertPosition + (UINT8)uiVal;
        psScalerInput->uiVStart = VERT_POSITION_LIMIT_MIN_PRO;
    }
    else
    {
        WorkingVertPosition[eCH] = m_sChannelSetting[eCH].ucVertPosition;
    }
}

void dvPro_Start_Position_Set(const eMCT eCH,PsINPUT_TIMING_INFO psScalerInput,PsOUTPUT_TIMING_INFO psScalerOutput)   //A70LV_Doulas_0010
{
    psScalerInput->uiHStart += HORZ_POSITION_DEFAULT - m_sChannelSetting[eCH].ucHoriPosition;
    psScalerInput->uiVStart += m_sChannelSetting[eCH].ucVertPosition - VERT_POSITION_DEFAULT;
}

ePROAV_EXEC_CODE dvPro_Horz_Start_Position_Set(const eMCT eCH,UINT8 ucPosition)  //A70LV_Doulas_0010
{
    ePROAV_EXEC_CODE eResult = ePROAV_EXEC_CODE_PASS;

    //if(m_sChannelSetting[eMCT_CH1].cPIP_PBP_ON == TRUE)     //A70LV_Doulas_0155 remove
    //    return ePROAV_EXEC_CODE_FAIL;

    m_sChannelSetting[eCH].ucHoriPosition = ucPosition;

    return eResult;
}

ePROAV_EXEC_CODE dvPro_Vert_Start_Position_Set(const eMCT eCH,UINT8 ucPosition)  //A70LV_Doulas_0010
{
    ePROAV_EXEC_CODE eResult = ePROAV_EXEC_CODE_PASS;

    //if(m_sChannelSetting[eMCT_CH1].cPIP_PBP_ON == TRUE)     //A70LV_Doulas_0155 remove
    //    return ePROAV_EXEC_CODE_FAIL;

    m_sChannelSetting[eCH].ucVertPosition = ucPosition;

    return eResult;
}

ePROAV_EXEC_CODE dvPro_Horz_Start_Position_Get(const eMCT eCH,UINT8 *ucPosition)  //A70LV_Doulas_0010
{
    ePROAV_EXEC_CODE eResult = ePROAV_EXEC_CODE_PASS;

    //if(m_sChannelSetting[eMCT_CH1].cPIP_PBP_ON == TRUE)     //A70LV_Doulas_0155 remove
    //    return ePROAV_EXEC_CODE_FAIL;

    *ucPosition = m_sChannelSetting[eCH].ucHoriPosition;

    return eResult;
}

ePROAV_EXEC_CODE dvPro_Vert_Start_Position_Get(const eMCT eCH,UINT8 *ucPosition)  //A70LV_Doulas_0010
{
    ePROAV_EXEC_CODE eResult = ePROAV_EXEC_CODE_PASS;

    //if(m_sChannelSetting[eMCT_CH1].cPIP_PBP_ON == TRUE)     //A70LV_Doulas_0155 remove
    //    return ePROAV_EXEC_CODE_FAIL;

    *ucPosition = m_sChannelSetting[eCH].ucVertPosition;

    return eResult;
}

void dvPro_Set_RGB_Gain(const eMCT eCH)    //A70LV_Doulas_0011
{
    (void)eCH;
    dvProAV_SclVopGainSet((SclEntity)eCH ,eSclColorR, m_sChannelSetting[eCH].iRedGain);
    dvProAV_SclVopGainSet((SclEntity)eCH ,eSclColorG, m_sChannelSetting[eCH].iGreenGain);
    dvProAV_SclVopGainSet((SclEntity)eCH ,eSclColorB, m_sChannelSetting[eCH].iBlueGain);
}

void dvPro_Set_RGB_Offset(const eMCT eCH)    //A70LV_Doulas_00011
{
    (void)eCH;
    dvProAV_SclVopBiasSet((SclEntity)eCH ,eSclColorR, m_sChannelSetting[eCH].iRedOffset);
    dvProAV_SclVopBiasSet((SclEntity)eCH ,eSclColorG, m_sChannelSetting[eCH].iGreenOffset);
    dvProAV_SclVopBiasSet((SclEntity)eCH ,eSclColorB, m_sChannelSetting[eCH].iBlueOffset);
}

INT8 dvPro_RedOffset_Value_Set (UINT8 ucCH, INT8 iRedOffset) //A70LV_Doulas_0022    //A70LV_Doulas_0011
{
    LOG_MSG(db_DV_SCALER, "(func:%s, line:%d): RedOffset=%d\r\n", __FUNCTION__, __LINE__, iRedOffset);

    m_sChannelSetting[ucCH].iRedOffset = iRedOffset;

    return PROAV_PASS;
}

INT8 dvPro_GreenOffset_Value_Set (UINT8 ucCH, INT8 iGreenOffset) //A70LV_Doulas_0022    //A70LV_Doulas_0011
{
    LOG_MSG(db_DV_SCALER, "(func:%s, line:%d): GreenOffset=%d\r\n", __FUNCTION__, __LINE__, iGreenOffset);

    m_sChannelSetting[ucCH].iGreenOffset = iGreenOffset;

    return PROAV_PASS;
}

INT8 dvPro_BlueOffset_Value_Set (UINT8 ucCH, INT8 iBlueOffset)  //A70LV_Doulas_0022   //A70LV_Doulas_0011
{
    LOG_MSG(db_DV_SCALER, "(func:%s, line:%d): BlueOffset=%d\r\n", __FUNCTION__, __LINE__, iBlueOffset);

    m_sChannelSetting[ucCH].iBlueOffset = iBlueOffset;

    return PROAV_PASS;
}

INT8 dvPro_RedGain_Value_Set (UINT8 ucCH, UINT8 iRedGain)    //A70LV_Doulas_0022 //A70LV_Doulas_0011
{
    LOG_MSG(db_DV_SCALER, "(func:%s, line:%d): RedGain=%d\r\n", __FUNCTION__, __LINE__, iRedGain);

    m_sChannelSetting[ucCH].iRedGain = iRedGain;

    return PROAV_PASS;
}

INT8 dvPro_GreenGain_Value_Set (UINT8 ucCH, UINT8 iGreenGain) //A70LV_Doulas_0022    //A70LV_Doulas_0011
{
    LOG_MSG(db_DV_SCALER, "(func:%s, line:%d): GreenGain=%d\r\n", __FUNCTION__, __LINE__, iGreenGain);

    m_sChannelSetting[ucCH].iGreenGain = iGreenGain;

    return PROAV_PASS;
}

INT8 dvPro_BlueGain_Value_Set (UINT8 ucCH, UINT8 iBlueGain) //A70LV_Doulas_0022      //A70LV_Doulas_0011
{
    LOG_MSG(db_DV_SCALER, "(func:%s, line:%d): BlueGain=%d\r\n", __FUNCTION__, __LINE__, iBlueGain);

    m_sChannelSetting[ucCH].iBlueGain = iBlueGain;

    return PROAV_PASS;
}

INT8 dvPro_Set_HUE_Value(eMCT eCH,INT16 iValue)  //A70LV_Doulas_0022
{
    //HUE(-180 ~ 180)
    m_sChannelSetting[eCH].iHue = iValue;
    return PROAV_PASS;
}

INT8 dvPro_Set_Saturation_Value(eMCT eCH,DOUBLE dSaturation)  //A70LV_Doulas_0022
{
    //color(0-3.99)
    m_sChannelSetting[eCH].dColor = dSaturation;
    return PROAV_PASS;
}

ePROAV_EXEC_CODE dvPro_Set_SizePresets_Value(eMCT eCH,UINT8 ucSizePresets)  //A70LV_Doulas_0020
{
    ePROAV_EXEC_CODE eResult = ePROAV_EXEC_CODE_PASS;

    //if(m_sChannelSetting[eMCT_CH1].cPIP_PBP_ON == TRUE)       //A70LV_Doulas_0155 remove
    //    return ePROAV_EXEC_CODE_FAIL;

    m_sChannelSetting[eCH].eScalingMode = (eCM_SCALING_MODE_ID)ucSizePresets;
    return eResult;
}

void dvPro_Sharpness_Value_Set(const eMCT eCH,UINT8 ucVal)     //A70LV_Doulas_0029
{
    m_sChannelSetting[eCH].ucSharpness = ucVal;
}

void dvPro_SkinColor_Value_Set(const eMCT eCH,UINT8 ucVal)
{
    m_sChannelSetting[eCH].ucSkinColor = ucVal;
}

void dvPro_SkinColor_Set(const eMCT eCH)   //A70LV_Doulas_0155 modify
{
    dvPro_Set_Color_Correction(eMCT_CH1,0,0,m_sChannelSetting[eMCT_CH1].ucSkinColor,2);
    if((m_sChannelSetting[eMCT_CH1].cPIP_PBP_ON == TRUE) ||
       (m_sChannelInfo[eMCT_CH1].ePanelTimingId == PANEL_3D_OUTPUT))
    {
        dvPro_Set_Color_Correction(eMCT_CH2,0,0,m_sChannelSetting[eMCT_CH2].ucSkinColor,2);
    }
}

void dvPro_Temporal_NR_Value_Set(const eMCT eCH,UINT8 ucVal)
{
    m_sChannelSetting[eCH].cTemporal_NR = ucVal;
}

void dvPro_MPEG_NR_Value_Set(const eMCT eCH,UINT8 ucVal)
{
    m_sChannelSetting[eCH].cMPEG_NR = ucVal;
}

void dvPro_Detect_Film_Value_Set(const eMCT eCH,UINT8 ucVal)
{
    m_sChannelSetting[eCH].cDetectFilm = ucVal;
}

void dvPro_IintChannelSetting(const eMCT eCH,sCHANNEL_SETTING m_sCH_Setting)     //A70LV_Doulas_0030
{
    memcpy(&m_sChannelSetting[eCH], &m_sCH_Setting, sizeof(sCHANNEL_SETTING));
    m_sChannelInfo[eCH].eInputSyncType = (eINPUT_SYNC_TYPE)m_sChannelSetting[eCH].eVGA_SYNC_TYPE;     //A70LV_Doulas_0112
}

void dvPro_InputFrameRate_Set(const eMCT eCH, DOUBLE dFrameRate)
{
    m_sChannelInfo[eCH].sInputTimingInfo.dFrameRate = dFrameRate;
}

DOUBLE dvPro_InputFrameRate_Get(const eMCT eCH) // ProAV_Rex_0016
{
    return m_sChannelInfo[eCH].sInputTimingInfo.dFrameRate;
}

void dvPro_InputPixelClock_Get(const eMCT eCH,UINT8 *ucValue)   //A70LV_Doulas_0056
{
    char aucString[VERSION_STRING_MAX_LENGTH];
    DOUBLE dbVal;

    dbVal = m_sChannelInfo[eCH].sInputTimingInfo.dFrameRate *m_sChannelInfo[eCH].sInputTimingInfo.uiHTotal
           *m_sChannelInfo[eCH].sInputTimingInfo.uiVTotal / (DOUBLE)1000 ;

#if 0
    switch(dvPro_Input_3D_Format_Get())  //A70LV_Doulas_0162 modify
    {
        case eINPUT_3D_TYPE_FRAME_SEQUENTIAL:
            dbVal = dbVal * 2;
            sprintf(aucString, "%d.%03dMHz",(UINT32)dbVal/1000,(UINT32)dbVal%1000);
            break;

        default:
            sprintf(aucString, "%d.%03dMHz",(UINT32)dbVal/1000,(UINT32)dbVal%1000);
            break;
    }
#endif /* 0 */

    snprintf(aucString, 31, "%d.%03dMHz",(UINT32)dbVal/1000,(UINT32)dbVal%1000);
    memcpy(ucValue, aucString, strlen(aucString)+1);
}

UINT16 dvPro_InputVSize_Get(const eMCT eCH)
{
    return m_sChannelInfo[eCH].sInputTimingInfo.uiHActive;
}

void dvPro_InputResolution_Get(const eMCT eCH,UINT8 *ucValue)
{
    char aucString[VERSION_STRING_MAX_LENGTH];
    UINT16 uiH_Active;
    UINT16 uiV_Active;

    switch(dvPro_Input_3D_Format_Get())  //A70LV_Doulas_0162 modify
    {
        case eINPUT_3D_TYPE_FRAMEPACKING:
            uiH_Active = m_sChannelInfo[eCH].sInputTimingInfo.uiHActive;
            if(m_sChannelInfo[eCH].sInputTimingInfo.uiVActive > 2150)
                uiV_Active = 1080;
            else
                uiV_Active = 720;
            break;

        default:
            uiH_Active = m_sChannelInfo[eCH].sInputTimingInfo.uiHActive;
            uiV_Active = m_sChannelInfo[eCH].sInputTimingInfo.uiVActive;
            break;
    }


    sprintf(aucString, "%d x %d",uiH_Active ,uiV_Active);
    memcpy(ucValue, aucString, strlen(aucString)+1);
}

void dvPro_InputHorzRefresh_Get(const eMCT eCH,UINT8 *ucValue)
{
    char aucString[VERSION_STRING_MAX_LENGTH];
    DOUBLE dbVal;

    dbVal = m_sChannelInfo[eCH].sInputTimingInfo.dFrameRate * (DOUBLE)m_sChannelInfo[eCH].sInputTimingInfo.uiVTotal;

    snprintf(aucString, 31, "%d.%03dkHz",(UINT32)dbVal/1000,(UINT32)dbVal%1000);
    memcpy(ucValue, aucString, strlen(aucString)+1);
}

void dvPro_InputVertRefresh_Get(const eMCT eCH,UINT8 *ucValue)
{
    char aucString[VERSION_STRING_MAX_LENGTH];
    UINT16 uiFraneRate;

    if(m_sChannelInfo[eCH].sInputTimingInfo.eScanMode == eSCAN_MODE_INTERLACE)    //A70LV_Doulas_0258 modify
    {
        uiFraneRate = (UINT16)(m_sChannelInfo[eCH].sInputTimingInfo.dFrameRate * 100.0);// * 2.0);
    }
    else
    {
        uiFraneRate = (UINT16)(m_sChannelInfo[eCH].sInputTimingInfo.dFrameRate * 100.0);
    }
    sprintf(aucString, "%d.%02dHz",uiFraneRate/100,uiFraneRate%100);
    memcpy(ucValue, aucString, strlen(aucString)+1);
}

void dvPro_InputVertRefresh2_Get(const eMCT eCH,UINT16 *uciValue)      //A70LV_Doulas_0200
{
    *uciValue = (UINT16)(m_sChannelInfo[eCH].sInputTimingInfo.dFrameRate * 100.0);
}

void dvPro_InputSignalFormat_Get(const eMCT eCH,UINT8 *ucValue)
{
    char aucString[VERSION_STRING_MAX_LENGTH];
    //UINT16 uiV_Active;

    if(m_sChannelInfo[eCH].sInputTimingInfo.eScanMode == eSCAN_MODE_INTERLACE)
    {
        if((m_sChannelInfo[eCH].sInputTimingInfo.uiHActive == 1920) &&
           (m_sChannelInfo[eCH].sInputTimingInfo.uiVActive == 1080))
        {
            sprintf(aucString, "1080i");
        }
        else if((m_sChannelInfo[eCH].sInputTimingInfo.uiHActive == 1440) &&
           (m_sChannelInfo[eCH].sInputTimingInfo.uiVActive == 576))
        {
            sprintf(aucString, "576i");
        }
        else if((m_sChannelInfo[eCH].sInputTimingInfo.uiHActive == 720) &&
           (m_sChannelInfo[eCH].sInputTimingInfo.uiVActive == 576))
        {
            sprintf(aucString, "576i");
        }
        else if((m_sChannelInfo[eCH].sInputTimingInfo.uiHActive == 1440) &&
           (m_sChannelInfo[eCH].sInputTimingInfo.uiVActive == 480))
        {
            sprintf(aucString, "480i");
        }
        else if((m_sChannelInfo[eCH].sInputTimingInfo.uiHActive == 720) &&
           (m_sChannelInfo[eCH].sInputTimingInfo.uiVActive == 480))
        {
            sprintf(aucString, "480i");
        }
        else
        {
            //if(m_sChannelSetting[eCH].eInputSource == eINPUT_SOURCE_VGA)
                //sprintf(aucString, "Analog");
            //else
                sprintf(aucString, "Digital");
        }
    }
    else
    {
        if((m_sChannelInfo[eCH].sInputTimingInfo.uiHActive == 1920) &&
           (m_sChannelInfo[eCH].sInputTimingInfo.uiVActive == 1080))
        {
            sprintf(aucString, "1080P");
        }
        else if((m_sChannelInfo[eCH].sInputTimingInfo.uiHActive == 1280) &&
           (m_sChannelInfo[eCH].sInputTimingInfo.uiVActive == 720))
        {
            sprintf(aucString, "720P");
        }
        else if((m_sChannelInfo[eCH].sInputTimingInfo.uiHActive == 720) &&
           (m_sChannelInfo[eCH].sInputTimingInfo.uiVActive == 576))
        {
            sprintf(aucString, "576P");
        }
        else if((m_sChannelInfo[eCH].sInputTimingInfo.uiHActive == 720) &&
           (m_sChannelInfo[eCH].sInputTimingInfo.uiVActive == 480))
        {
            sprintf(aucString, "480P");
        }
        else if((dvPro_Input_3D_Format_Get() == eINPUT_3D_TYPE_FRAME_SEQUENTIAL) &&
                (m_sChannelInfo[eCH].sInputTimingInfo.uiHActive == 960) &&
                (m_sChannelInfo[eCH].sInputTimingInfo.uiVActive == 1080))   //A70LV_Doulas_0162
        {
            sprintf(aucString, "1080P");
        }
        else if((dvPro_Input_3D_Format_Get() == eINPUT_3D_TYPE_FRAME_SEQUENTIAL) &&
                (m_sChannelInfo[eCH].sInputTimingInfo.uiHActive == 640) &&
                (m_sChannelInfo[eCH].sInputTimingInfo.uiVActive == 720))    //A70LV_Doulas_0162
        {
            sprintf(aucString, "720P");
        }
        else if(dvPro_Input_3D_Format_Get() == eINPUT_3D_TYPE_FRAMEPACKING)    //A70LV_Doulas_0162
        {
                if(m_sChannelInfo[eCH].sInputTimingInfo.uiVActive > 2150)
                    sprintf(aucString, "1080P");
                else
                    sprintf(aucString, "720P");
        }
        else
        {
            //if(m_sChannelSetting[eCH].eInputSource == eINPUT_SOURCE_VGA)
            //    sprintf(aucString, "Analog");
            //else
                sprintf(aucString, "Digital");
        }
    }
    memcpy(ucValue, aucString, strlen(aucString)+1);
}

void dvPro_InputAspectRatio_Get(const eMCT eCH,UINT8 *ucValue)
{
    char aucString[VERSION_STRING_MAX_LENGTH];
    //UINT16 uiV_Active;

    if((m_sChannelInfo[eCH].sInputTimingInfo.uiHActive * 2 / 3) == m_sChannelInfo[eCH].sInputTimingInfo.uiVActive)
    {
        sprintf(aucString, "3:2");
    }
    else if((m_sChannelInfo[eCH].sInputTimingInfo.uiHActive * 3 / 4) == m_sChannelInfo[eCH].sInputTimingInfo.uiVActive)
    {
        sprintf(aucString, "4:3");
    }
    else if((m_sChannelInfo[eCH].sInputTimingInfo.uiHActive * 4 / 5) == m_sChannelInfo[eCH].sInputTimingInfo.uiVActive)
    {
        sprintf(aucString, "5:4");
    }
    else if((m_sChannelInfo[eCH].sInputTimingInfo.uiHActive * 9 / 16) == m_sChannelInfo[eCH].sInputTimingInfo.uiVActive)
    {
        sprintf(aucString, "16:9");
    }
    else if((m_sChannelInfo[eCH].sInputTimingInfo.uiHActive * 10 / 16) == m_sChannelInfo[eCH].sInputTimingInfo.uiVActive)
    {
        sprintf(aucString, "16:10");
    }
    else if((m_sChannelInfo[eCH].sInputTimingInfo.uiHActive * 9 / 15) == m_sChannelInfo[eCH].sInputTimingInfo.uiVActive)
    {
        sprintf(aucString, "15:9");
    }
    else if((m_sChannelInfo[eCH].sInputTimingInfo.uiHActive * 3 / 5) == m_sChannelInfo[eCH].sInputTimingInfo.uiVActive)
    {
        sprintf(aucString, "5:3");
    }
    else if((m_sChannelInfo[eCH].sInputTimingInfo.uiHActive * 5 / 6) == m_sChannelInfo[eCH].sInputTimingInfo.uiVActive)
    {
        sprintf(aucString, "6:5");
    }
    else
    {
        sprintf(aucString, "16:9");
    }

    memcpy(ucValue, aucString, strlen(aucString)+1);
}

void dvPro_FrontEndScanMode_Set(const eMCT eCH,UINT8 ucVal)     //A70LV_Doulas_0076
{
    m_sChannelSetting[eCH].eScanMode = ucVal;
    LOG_MSG(db_DV_SCALER, "dvPro_FrontEndScanMode_Set[%d] = %d\n",eCH,ucVal);    //A70LV_Doulas_0079
}

void dvPro_ForcedSyncResetDisable_Set(void)     //A70LV_Doulas_0079
{
    uiForcedSyncResetH_Total = m_sChannelInfo[eMCT_CH1].sOutputTimingInfo.uiHTotal;
    uiForcedSyncResetV_Total = m_sChannelInfo[eMCT_CH1].sOutputTimingInfo.uiVTotal;

    bForcedSyncResetEnable = FALSE;
}

BOOL dvPro_ForcedSyncResetType_Get(void)
{
    return bForcedSyncResetEnable;
}

void dvPro_ForceSyncRese_Get(BOOL *bForceSyncReseVal,UINT16 *uiH_Toral,UINT16 *uiV_Toral)
{
    *bForceSyncReseVal = m_sChannelInfo[eMCT_CH1].bForceSyncReset;
    *uiH_Toral = uiForcedSyncResetH_Total;
    *uiV_Toral = uiForcedSyncResetV_Total;
}

BOOL dvPro_IsInterlaced_Get(const eMCT eCH)       //A70LV_Doulas_0092
{
    if(m_sChannelInfo[eCH].sInputTimingInfo.eScanMode == eSCAN_MODE_INTERLACE)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

UINT16 dvPro_PanelVSize_Get(void)
{
    return m_sChannelInfo[eMCT_CH1].sOutputTimingInfo.uiVActive;
}

DOUBLE dvPro_PanelFrameRateGet(void)
{
    //return (DOUBLE)(m_sPanelTable[eMCT_CH1].sOutputTimingInfo.uiFrameRate / 100);
    return (DOUBLE)( m_sChannelInfo[eMCT_CH1].sOutputTimingInfo.uiFrameRate/ 100);
}

UINT16 dvPro_PanelHTotalGet(void)
{
    return m_sChannelInfo[eMCT_CH1].sOutputTimingInfo.uiHTotal;
}

UINT16 dvPro_PanelVTotalGet(void)
{
    return m_sChannelInfo[eMCT_CH1].sOutputTimingInfo.uiVTotal;
}

UINT32 dvPro_PanelPixelClockGet(void)
{
    return m_sChannelInfo[eMCT_CH1].sOutputTimingInfo.ulPixelClock;
}

ePANEL_ID dvPro_PanelGet(void)       //A70LV_Doulas_0098
{
     return m_sChannelInfo[eMCT_CH1].ePanelTimingId;
}

void dvPro_VideoYUV_Set(const eMCT eCH,UINT8 ucVal)     //A70LV_Doulas_0109
{
    m_sChannelSetting[eCH].eVideoYUV = ucVal;
    LOG_MSG(db_DV_SCALER, "dvPro_VideoYUV_Set[%d] = %d\n",eCH,ucVal);
}

UINT8 dvPro_VideoYUV_Get(const eMCT eCH)   //A70LV_Doulas_0109
{
	UINT8 icolor_space;

	icolor_space = (m_sChannelInfo[eCH].sInputTimingInfo.uiHActive > 1024) ? 1 : 0;	//0:BT-601, 1:BT-709

    return icolor_space;
}

UINT16 dvPro_VGA_H_Total_Get(const eMCT eCH)   //A70LV_Doulas_0112
{
    return m_sChannelInfo[eCH].sInputTimingInfo.uiHTotal;
}

void dvPro_VGA_SYNC_TYPE_Set(const eMCT eCH,UINT8 ucVal)     //A70LV_Doulas_0112
{
    m_sChannelSetting[eCH].eVGA_SYNC_TYPE = ucVal;
    m_sChannelInfo[eCH].eInputSyncType = (eINPUT_SYNC_TYPE)ucVal;
    LOG_MSG(db_DV_SCALER, "dvPro_VGA_SYNC_TYPE_Set[%d] = %d\n",eCH,ucVal);
}

void dvPro_AutoImage_Set(UINT8 ucSource,UINT8 ucVal)     //A70LV_Doulas_0112
{
    if(ucSource < eCM_SOURCE_NUMBER)
    {
        m_AutoImage[ucSource] = ucVal;
        LOG_MSG(db_DV_SCALER, "dvPro_AutoImage_Set[%d] = %d\n",ucSource,ucVal);
    }
}

UINT8 dvPro_AutoImage_Get(void)     //A70LV_Doulas_0112
{
    return m_AutoImage[m_sChannelSetting[eMCT_CH1].eInputSource];   //only get main
}

UINT32 dvPro_uiHFreq_Get(const eMCT eCH)     //A70LV_Doulas_0115
{
    return m_sChannelInfo[eCH].sInputTimingInfo.uiHFreq;
}

BOOL dvPro_CheckingHorzPositionSettingWork(eMCT eCH)
{
    if(WorkingHorzPosition[eCH] == m_sChannelSetting[eCH].ucHoriPosition)
        return TRUE;
    else
        return FALSE;
}

BOOL dvPro_CheckingVertPositionSettingWork(eMCT eCH)
{
    if(WorkingVertPosition[eCH] == m_sChannelSetting[eCH].ucVertPosition)
        return TRUE;
    else
        return FALSE;
}

UINT8 dvPro_HorzPositionWorkValueGet(eMCT eCH)
{
    return WorkingHorzPosition[eCH];
}

UINT8 dvPro_VertPositionWorkValueGet(eMCT eCH)
{
    return WorkingVertPosition[eCH];
}

void dvPro_AutoPsitionStart(eMCT eCH,BOOL bAutoEnable)     //A70LV_Doulas_0118
{
    m_sChannelInfo[eCH].bAutoPosition = bAutoEnable;
}

UINT16 dvPro_Input_H_Active_Get(const eMCT eCH)
{
    return m_sChannelInfo[eCH].sInputTimingInfo.uiHActive;
}

UINT16 dvPro_Input_V_Active_Get(const eMCT eCH)
{
    return m_sChannelInfo[eCH].sInputTimingInfo.uiVActive;
}

	//Cassper_ProAV
UINT16 dvPro_Input_H_Total_Get(const eMCT eCH)
{
    return m_sChannelInfo[eCH].sInputTimingInfo.uiHTotal;
}

UINT16 dvPro_Input_V_Total_Get(const eMCT eCH)
{
    return m_sChannelInfo[eCH].sInputTimingInfo.uiVTotal;
}

UINT16 dvPro_Input_H_Start_Get(const eMCT eCH)
{
    return m_sChannelInfo[eCH].sInputTimingInfo.uiHStart;
}

UINT16 dvPro_Input_V_Start_Get(const eMCT eCH)
{
    return m_sChannelInfo[eCH].sInputTimingInfo.uiVStart;
}
	//Cassper_ProAV

void dvPro_Input_3D_Format_Set(UINT8 ucVal)  //A70LV_Doulas_0154
{
    ucInput3D_Format = ucVal;
}

UINT8 dvPro_Input_3D_Format_Get(void)      //A70LV_Doulas_0154
{
    return ucInput3D_Format;
}

UINT8 dvPro_Input_3D_Format_Config_Get(void)      //A70LV_Doulas_0159
{
    UINT8 uc3D_Format;
    //LOG_MSG(db_ALWAYS, "dvPro_OSD_3D_Enable  = (%d,%d) \r\n",m_sChannelSetting[eMCT_CH1].eOSD_3D_Enable,ucInput3D_Format);  //A70LV_Doulas_0160

    if(m_sChannelSetting[eMCT_CH1].cPIP_PBP_ON == TRUE)
    {
        return eINPUT_3D_TYPE_OFF;
    }

    switch(m_sChannelSetting[eMCT_CH1].eOSD_3D_Enable)
    {
        case eCM_3D_FORMAT_AUTO:
            uc3D_Format = ucInput3D_Format;
            break;

        case eCM_3D_FORMAT_FRAME_PACKING:
            uc3D_Format = eINPUT_3D_TYPE_FRAMEPACKING;
            break;

        case eCM_3D_FORMAT_SIDE_BY_SIDE:
            uc3D_Format = eINPUT_3D_TYPE_SIDEBYSIDE;
            break;

        case eCM_3D_FORMAT_TOP_AND_BOTTOM:
            uc3D_Format = eINPUT_3D_TYPE_TOPANDBOTTOM;
            break;

        case eCM_3D_FORMAT_FRAME_SEQUENTIAL:
            uc3D_Format = eINPUT_3D_TYPE_FRAME_SEQUENTIAL;
            break;

        case eCM_3D_FORMAT_DUALPIPE:
        case eCM_3D_FORMAT_DUALPIPE_3D:
            uc3D_Format = eINPUT_3D_TYPE_DAUL_PIPE;
            break;

        case eCM_3D_FORMAT_4K3D:
            uc3D_Format = eINPUT_3D_TYPE_4K3D_SEQUENTIAL;
            break;

        case eCM_3D_FORMAT_4K3D_DUALPIPE:
            uc3D_Format = eINPUT_3D_TYPE_4K3D_DAUL_PIPE;
            break;

        default:
            uc3D_Format = eINPUT_3D_TYPE_OFF;
            break;
    }
    return uc3D_Format;
}

void dvPro_OSD_3D_Enable_Set(const eMCT eCH,UINT8 ucVal)     //A70LV_Doulas_0154
{
    m_sChannelSetting[eCH].eOSD_3D_Enable = ucVal;
    LOG_MSG(db_DV_SCALER, "dvPro_OSD_3D_Enable_Set[%d] = %d\n",eCH,ucVal);
}

void dvPro_OSD_3D_Sync_OutSet(const eMCT eCH, UINT8 ucVal)
{
    m_sChannelSetting[eCH].eOSD_3D_SyncOut = ucVal;
}

UINT8 dvPro_OSD_3D_Sync_OutGet(const eMCT eCH)
{
    return m_sChannelSetting[eCH].eOSD_3D_SyncOut;
}

void dvPro_OSD_3D_Sync_InSet(const eMCT eCH, UINT8 ucVal)
{
    m_sChannelSetting[eCH].eOSD_3D_SyncIn = ucVal;
}

UINT8 dvPro_OSD_3D_Sync_InGet(const eMCT eCH)
{
    return m_sChannelSetting[eCH].eOSD_3D_SyncIn;
}

void dvPro_OSD_3D_InvertSet(const eMCT eCH, UINT8 ucVal)
{
    m_sChannelSetting[eCH].eOSD_3D_Invert = ucVal;
}

UINT8 dvPro_OSD_3D_InvertGet(const eMCT eCH)
{
    return m_sChannelSetting[eCH].eOSD_3D_Invert;
}

int dvPro_OSD_3D_Sync_Setting(UINT8 uc3DMode)
{
    int16 status = rcSUCCESS;

    status &= dvProAV_SclFrameSeq3dsyncDubSet(false);

    //4K3D預設下都要開warping，而且預設delay 1frame，所以sync delay剛好對齊影像
    //其他的3D格式需要調整sync delay去對齊影像，造成4K3D極性剛好相反
    //針對4K3D的eVopLRSw參考warping後，這裡預設scaler，BuildPicture針對4K3D設定
    //正確解應該看是否有開warping去調整eVopLRSw
    status &= dvProAV_Scl3dIntVsModeSel(1);

    switch(uc3DMode)
    {
        case eScl3DMode_FramePacking:
        case eScl3DMode_SideBySide:
        case eScl3DMode_TopAndBottom:
            if(m_sChannelSetting[eMCT_CH1].eOSD_3D_SyncOut == eCM_3D_SYNC_OUT_TO_EMITTER)
            {
                status &= dvProAV_SclIntSyncPolSet((bool)m_sChannelSetting[eMCT_CH1].eOSD_3D_Invert);
                status &= dvProAV_SclExtSyncPolSet((bool)m_sChannelSetting[eMCT_CH1].eOSD_3D_Invert);
#ifdef FRAME_SEQUENTIAL_3D_ULTRA
                status &= dvProAV_Scl3dSyncOutSwModeSet(e3dSyncOutSwMode_3dSyncInDelay);
#else
                status &= dvProAV_Scl3dSyncOutSwModeSet(e3dSyncOutSwMode_DdpSyncIn);
#endif /* FRAME_SEQUENTIAL_3D_ULTRA */
                LOG_MSG(db_DV_3D_CONFIG, "(%s, %d)1.4a 3D OutSw DdpSyncIn\n", __FUNCTION__, __LINE__);
            }
            else
            {
                status &= dvProAV_SclIntSyncPolSet(false);
                status &= dvProAV_SclExtSyncPolSet(false);
                status &= dvProAV_Scl3dSyncOutSwModeSet(e3dSyncOutSwMode_InternalSync);

                LOG_MSG(db_DV_3D_CONFIG, "(%s, %d)1.4a 3D OutSw InternalSync\n", __FUNCTION__, __LINE__);
            }

            //if(m_sChannelSetting[eMCT_CH1].eOSD_3D_SyncIn == eCM_3D_SYNC_TYPE_EXTERNAL)
            //{
                //status &= dvProAV_Scl3dSyncInSwModeSet(e3dSyncInSwMode_ExternalSync);
                //LOG_MSG(db_DV_3D_CONFIG, "1.4a 3D InSw ExternalSync\n");
            //}
            //else
            {
                status &= dvProAV_Scl3dSyncInSwModeSet(e3dSyncInSwMode_InternalSync);
                LOG_MSG(db_DV_3D_CONFIG, "(%s, %d)1.4a 3D InSw InternalSync\n", __FUNCTION__, __LINE__);
            }
            break;

        default:
            if(dvPro_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_FRAME_SEQUENTIAL ||
               dvPro_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_4K3D_SEQUENTIAL)
            {
                if(m_sChannelSetting[eMCT_CH1].eOSD_3D_SyncOut == eCM_3D_SYNC_OUT_TO_EMITTER)
                {
                    status &= dvProAV_SclIntSyncPolSet((bool)m_sChannelSetting[eMCT_CH1].eOSD_3D_Invert);
                    status &= dvProAV_SclExtSyncPolSet((bool)m_sChannelSetting[eMCT_CH1].eOSD_3D_Invert);

                    if(m_sChannelSetting[eMCT_CH1].eOSD_3D_SyncIn == eCM_3D_SYNC_TYPE_INTERNAL)
                    {
#ifdef FRAME_SEQUENTIAL_3D_ULTRA
                        status &= dvProAV_Scl3dSyncOutSwModeSet(e3dSyncOutSwMode_3dSyncInDelay);
                        LOG_MSG(db_DV_3D_CONFIG, "(%s, %d)Frame Seq. OutSw 3dSyncInDelay\n", __FUNCTION__, __LINE__);
#else
                        status &= dvProAV_Scl3dSyncOutSwModeSet(e3dSyncOutSwMode_DdpSyncIn);
                        LOG_MSG(db_DV_3D_CONFIG, "Frame Seq. OutSw DdpSyncIn\n");
#endif /* FRAME_SEQUENTIAL_3D_ULTRA */

                        status &= dvProAV_Scl3dSyncInSwModeSet(e3dSyncInSwMode_InternalSync);
                        LOG_MSG(db_DV_3D_CONFIG, "(%s, %d)Frame Seq. InSw InternalSync\n", __FUNCTION__, __LINE__);

                    }
                    else
                    {
#ifdef FRAME_SEQUENTIAL_3D_ULTRA
                        status &= dvProAV_Scl3dSyncOutSwModeSet(e3dSyncOutSwMode_3dSyncInDelay);
                        LOG_MSG(db_DV_3D_CONFIG, "(%s, %d)Frame Seq. OutSw 3dSyncInDelay\n", __FUNCTION__, __LINE__);
#else
                        status &= dvProAV_Scl3dSyncOutSwModeSet(e3dSyncOutSwMode_DdpSyncIn);
                        LOG_MSG(db_DV_3D_CONFIG, "Frame Seq. OutSw DdpSyncIn\n");
#endif /* FRAME_SEQUENTIAL_3D_ULTRA */

                        if(m_sChannelSetting[eMCT_CH1].eOSD_3D_SyncIn == eCM_3D_SYNC_TYPE_AUTO)
                        {
                            if(dvProAV_Scl3dNoExtSyncStatus()) //no 3D sync
                            {
                                status &= dvProAV_Scl3dSyncInSwModeSet(e3dSyncInSwMode_InternalSync);
                                LOG_MSG(db_DV_3D_CONFIG, "(%s, %d)Frame Seq. InSw InternalSync\n", __FUNCTION__, __LINE__);
                            }
                            else
                            {
                                status &= dvProAV_Scl3dSyncInSwModeSet(e3dSyncInSwMode_ExternalSync);

                                if(dvPro_InputRataIntegerGet(m_sChannelInfo[eMCT_CH1].sInputTimingInfo.dFrameRate) == 60)
                                {
                                    status &= dvProAV_SclFrameSeq3dsyncDubSet(true);

                                    if(dvPro_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_4K3D_SEQUENTIAL)
                                    {
                                        status &= dvProAV_Scl3dIntVsModeSel(0);
                                    }
                                }
                                LOG_MSG(db_DV_3D_CONFIG, "(%s, %d)Frame Seq. InSw ExternalSync\n", __FUNCTION__, __LINE__);
                            }
                        }
                        else
                        {
                            status &= dvProAV_Scl3dSyncInSwModeSet(e3dSyncInSwMode_ExternalSync);
                            if(dvPro_InputRataIntegerGet(m_sChannelInfo[eMCT_CH1].sInputTimingInfo.dFrameRate) == 60)
                            {
                                status &= dvProAV_SclFrameSeq3dsyncDubSet(true);
                            }

                            if(dvPro_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_4K3D_SEQUENTIAL)
                            {
                                status &= dvProAV_Scl3dIntVsModeSel(0);
                            }

                            LOG_MSG(db_DV_3D_CONFIG, "(%s, %d)Frame Seq. InSw ExternalSync\n", __FUNCTION__, __LINE__);
                        }
                    }
                }
                else //next projector
                {
                    status &= dvProAV_SclIntSyncPolSet(false);
                    status &= dvProAV_SclExtSyncPolSet(true);
                    status &= dvProAV_Scl3dSyncOutSwModeSet(e3dSyncOutSwMode_ExternalSync);
                    LOG_MSG(db_DV_3D_CONFIG, "(%s, %d)Frame Seq. OutSw ExternalSync\n", __FUNCTION__, __LINE__);

                    if(m_sChannelSetting[eMCT_CH1].eOSD_3D_SyncIn == eCM_3D_SYNC_TYPE_INTERNAL)
                    {
                        status &= dvProAV_Scl3dSyncOutSwModeSet(e3dSyncOutSwMode_InternalSync);
                        LOG_MSG(db_DV_3D_CONFIG, "(%s, %d)Frame Seq. InternalSync\n", __FUNCTION__, __LINE__);
                    }
                    else
                    {

                        if(m_sChannelSetting[eMCT_CH1].eOSD_3D_SyncIn == eCM_3D_SYNC_TYPE_AUTO)
                        {
                            if(dvProAV_Scl3dNoExtSyncStatus()) //no 3D sync
                            {
                                status &= dvProAV_Scl3dSyncInSwModeSet(e3dSyncInSwMode_InternalSync);
                                LOG_MSG(db_DV_3D_CONFIG, "(%s, %d)Frame Seq. InSw InternalSync\n", __FUNCTION__, __LINE__);
                            }
                            else
                            {
                                status &= dvProAV_Scl3dSyncInSwModeSet(e3dSyncInSwMode_ExternalSync);
                                LOG_MSG(db_DV_3D_CONFIG, "(%s, %d)Frame Seq. InSw ExternalSync\n", __FUNCTION__, __LINE__);
                            }
                        }
                        else
                        {
                            status &= dvProAV_Scl3dSyncInSwModeSet(e3dSyncInSwMode_ExternalSync);
                            LOG_MSG(db_DV_3D_CONFIG, "(%s, %d)Frame Seq. InSw ExternalSync\n", __FUNCTION__, __LINE__);
                        }
                    }
                }
            }
            else if(dvPro_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_DAUL_PIPE ||
                    dvPro_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_4K3D_DAUL_PIPE) //||
                    //dvPro_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_4K3D_SEQUENTIAL)
            {
                if(m_sChannelSetting[eMCT_CH1].eOSD_3D_SyncOut == eCM_3D_SYNC_OUT_TO_EMITTER)
                {
                    status &= dvProAV_SclIntSyncPolSet((bool)m_sChannelSetting[eMCT_CH1].eOSD_3D_Invert);
                    status &= dvProAV_SclExtSyncPolSet((bool)m_sChannelSetting[eMCT_CH1].eOSD_3D_Invert);
#ifdef FRAME_SEQUENTIAL_3D_ULTRA
                    status &= dvProAV_Scl3dSyncOutSwModeSet(e3dSyncOutSwMode_3dSyncInDelay);
#else
                    status &= dvProAV_Scl3dSyncOutSwModeSet(e3dSyncOutSwMode_DdpSyncIn);
#endif /* FRAME_SEQUENTIAL_3D_ULTRA */
                    LOG_MSG(db_DV_3D_CONFIG, "(%s, %d)Dual Pipe and 4K3D OutSw DdpSyncIn\n", __FUNCTION__, __LINE__);
                }
                else
                {
                    status &= dvProAV_SclIntSyncPolSet(false);
                    status &= dvProAV_SclExtSyncPolSet(true);
                    status &= dvProAV_Scl3dSyncOutSwModeSet(e3dSyncOutSwMode_InternalSync);
                    LOG_MSG(db_DV_3D_CONFIG, "(%s, %d)Dual Pipe and 4K3D OutSw InternalSync\n", __FUNCTION__, __LINE__);
                }

                //if(m_sChannelSetting[eMCT_CH1].eOSD_3D_SyncIn == eCM_3D_SYNC_TYPE_EXTERNAL)
                //{
                    //status &= dvProAV_Scl3dSyncInSwModeSet(e3dSyncInSwMode_ExternalSync);
                    //LOG_MSG(db_DV_3D_CONFIG, "Dual Pipe InSw ExternalSync\n");
                //}
                //else
                {
                    status &= dvProAV_Scl3dSyncInSwModeSet(e3dSyncInSwMode_InternalSync);
                    LOG_MSG(db_DV_3D_CONFIG, "(%s, %d)Dual Pipe InSw InternalSync\n", __FUNCTION__, __LINE__);
                }
            }
            else
            {
                if(m_sChannelSetting[eMCT_CH1].eOSD_3D_SyncOut == eCM_3D_SYNC_OUT_TO_EMITTER)
                {
                    status &= dvProAV_SclIntSyncPolSet((bool)m_sChannelSetting[eMCT_CH1].eOSD_3D_Invert);
                    status &= dvProAV_SclExtSyncPolSet((bool)m_sChannelSetting[eMCT_CH1].eOSD_3D_Invert);
#ifdef FRAME_SEQUENTIAL_3D_ULTRA
                    status &= dvProAV_Scl3dSyncOutSwModeSet(e3dSyncOutSwMode_3dSyncInDelay);
#else
                    status &= dvProAV_Scl3dSyncOutSwModeSet(e3dSyncOutSwMode_DdpSyncIn);
#endif /* FRAME_SEQUENTIAL_3D_ULTRA */
                    LOG_MSG(db_DV_3D_CONFIG, "(%s, %d)3D Off OutSw DdpSyncIn\n", __FUNCTION__, __LINE__);
                }
                else
                {
                    status &= dvProAV_SclIntSyncPolSet(false);
                    status &= dvProAV_SclExtSyncPolSet(true);
                    status &= dvProAV_Scl3dSyncOutSwModeSet(e3dSyncOutSwMode_InternalSync);
                    LOG_MSG(db_DV_3D_CONFIG, "(%s, %d)3D Off OutSw InternalSync\n", __FUNCTION__, __LINE__);
                }

                //if(m_sChannelSetting[eMCT_CH1].eOSD_3D_SyncIn == eCM_3D_SYNC_TYPE_EXTERNAL)
                //{
                    //status &= dvProAV_Scl3dSyncInSwModeSet(e3dSyncInSwMode_ExternalSync);
                    //LOG_MSG(db_DV_3D_CONFIG, "3D Off InSw ExternalSync\n");
                //}
                //else
                {
                    status &= dvProAV_Scl3dSyncInSwModeSet(e3dSyncInSwMode_InternalSync);
                    LOG_MSG(db_DV_3D_CONFIG, "(%s, %d)3D Off InSw InternalSync\n", __FUNCTION__, __LINE__);
                }
            }

            //LOG_MSG(db_ALWAYS, "e3dSyncOutSwMode_ExternalSync\n");
            break;
    }

    return status;
}


BOOL dvPro_AspectRatioIs3DModeGet(void)            //A70LV_Doulas_0154
{
    BOOL bIs3DMode = FALSE;
    //LOG_MSG(db_ALWAYS, "dvPro_AspectRatioIs3DModeGet %d %d\r\n", m_sChannelSetting[eMCT_CH1].eOSD_3D_Enable, dvPro_Input_3D_Format_Get());

    switch(m_sChannelSetting[eMCT_CH1].eOSD_3D_Enable)
    {
        case eCM_3D_FORMAT_AUTO:
            switch(dvPro_Input_3D_Format_Get())  //A70LV_Doulas_0162 //A70LV_Doulas_0159 modify
            {
                case eINPUT_3D_TYPE_FRAMEPACKING:
                case eINPUT_3D_TYPE_SIDEBYSIDE:
                case eINPUT_3D_TYPE_TOPANDBOTTOM:
                    bIs3DMode = TRUE;
                    break;

                default:
                    break;
            }
            break;

        case eCM_3D_FORMAT_OFF:
            bIs3DMode = FALSE;
            break;

        default:
            bIs3DMode = TRUE;
            break;
    }
    return bIs3DMode;
}

void dvPro_3D_InputPort_Set(void)     //A70LV_Doulas_0154
{
    switch(dvPro_Input_3D_Format_Config_Get())
    {
        case eINPUT_3D_TYPE_FRAMEPACKING:
        case eINPUT_3D_TYPE_SIDEBYSIDE:
        case eINPUT_3D_TYPE_TOPANDBOTTOM:
            m_sChannelInfo[eMCT_CH2].ucInputPort = eMCT_CH1;
            break;

        default:
            m_sChannelInfo[eMCT_CH2].ucInputPort = eMCT_CH2;
            break;
    }
    dvPro_SetInputPort_2K(eMCT_CH2);
}

UINT32 dvPro_InputHorzPeriod_Get(const eMCT eCH)     //A70LV_Doulas_0195
{
    return (UINT32)(m_sChannelInfo[eCH].sInputTimingInfo.dFrameRate * (DOUBLE)m_sChannelInfo[eCH].sInputTimingInfo.uiVTotal);
}

UINT16 dvPro_VGA_V_Total_Get(const eMCT eCH)   //A70LV_Doulas_0195
{
    return m_sChannelInfo[eCH].sInputTimingInfo.uiVTotal;
}

UINT8 dvPro_VGA_ModeTable_Get(const eMCT eCH)   //A70LV_Doulas_0195
{
    return m_ucModeTableID[eCH];
}

UINT16 dvPro_VGA_ModeTableNumber_Get(const eMCT eCH)   //A70LV_Doulas_0195
{
    return m_BestModeTableNumber[eCH];
}

UINT16 dvPro_ModeAdjusmenttHorzStart_Get(void)   //A70LV_Doulas_0195
{
    return uiModeAdjusmenttHorzStart;
}

UINT16 dvPro_ModeAdjusmenttVertStart_Get(void)   //A70LV_Doulas_0195
{
    return uiModeAdjusmenttVertStart;
}

void dvPro_ModeAdjusmenttHorzStart_Set(UINT16 uiHStart)   //A70LV_Doulas_0195
{
    uiModeAdjusmenttHorzStart = uiHStart;
}

void dvPro_ModeAdjusmenttVertStart_Set(UINT16 uiVStart)   //A70LV_Doulas_0195
{
    uiModeAdjusmenttVertStart = uiVStart;
}

void dvPro_ModeAdjusmenttDisableSetting(void)   //A70LV_Doulas_0195
{
    bModeAdjustmentEnable = FALSE;
}

void dvPro_Init_Mode_Adjustment_EEPROM_Setting(sTIMING_TABLE sTiming_Table)  //A70LV_Doulas_0195
{
    memcpy(&m_sModeAdjustmentTimingTable, &sTiming_Table, sizeof(sTIMING_TABLE));
}

int dvPro_PixelShift_Setting(ePIXEL_SHIFT ePixelShift)
{
    int16 status = rcSUCCESS;

    switch(ePixelShift)
    {
        case ePIXEL_SHIFT_NORMAL:
        case ePIXEL_SHIFT_FRAME_SEQ_RL:
        case ePIXEL_SHIFT_DUAL_PIPE_RL:
            //如果DDP開FRC by pass, SubFrameSet要改成2,0,1, 3
            status &= dvProAV_PixSftSubFrameSet(0, 1, 3, 2);
            status &= dvProAV_PixSftApsSet(1, 0, 2, 3);
            status &= dvProAV_PixSftWcuLr4k3DCfgSet(0, 0, 0, 0);
            status &= dvProAV_PixSftStrLndex2ndSet(1);
            break;

        case ePIXEL_SHIFT_FRAME_SEQ_PLUS:
            status &= dvProAV_PixSftSubFrameSet(2, 3, 0, 1);
            status &= dvProAV_PixSftApsSet(1, 0, 2, 3);
            status &= dvProAV_PixSftWcuLr4k3DCfgSet(1, 0, 1, 0);
            status &= dvProAV_PixSftStrLndex2ndSet(3);
            break;

        case ePIXEL_SHIFT_FRAME_SEQ_RRLL:
        case ePIXEL_SHIFT_DUAL_PIPE_RRLL:
            status &= dvProAV_PixSftSubFrameSet(0, 3, 1, 2);
            status &= dvProAV_PixSftApsSet(1, 0, 3, 2);
            status &= dvProAV_PixSftWcuLr4k3DCfgSet(0, 0, 0, 0);
            status &= dvProAV_PixSftStrLndex2ndSet(1);
            break;

        case ePIXEL_SHIFT_DUAL_PIPE_4K3D:
            status &= dvProAV_PixSftSubFrameSet(0, 2, 1, 3);
            status &= dvProAV_PixSftApsSet(1, 0, 2, 3);
            status &= dvProAV_PixSftWcuLr4k3DCfgSet(0, 0, 0, 0);
            status &= dvProAV_PixSftStrLndex2ndSet(1);
            break;

        case ePIXEL_SHIFT_OFF: //XPR OFF
            //status &= dvProAV_PixSftSubFrameSet(0, 1, 3, 2);
            status &= dvProAV_PixSftApsSet(0, 0, 0, 0);
            status &= dvProAV_PixSftWcuLr4k3DCfgSet(0, 0, 0, 0);
            status &= dvProAV_PixSftStrLndex2ndSet(0);
            break;
        default:
            break;
    }

    return status;
}

UINT8 dvPro_InputRataIntegerGet(DOUBLE dIptFrameRate)
{
    UINT8 ucValue = (UINT8)dIptFrameRate;

    if((dIptFrameRate >= 23.93) && (dIptFrameRate <= 24.07))
    {
        ucValue = 24;
    }

    if((dIptFrameRate >= 29.93) && (dIptFrameRate <= 30.07))
    {
        ucValue = 30;
    }

    if((dIptFrameRate >= 59.93) && (dIptFrameRate <= 60.07))
    {
        ucValue = 60;
    }

    if((dIptFrameRate >= 119.86) && (dIptFrameRate <= 120.14))
    {
        ucValue = 120;
    }

    if((dIptFrameRate >= 239.72) && (dIptFrameRate <= 240.28))
    {
        ucValue = 240;
    }

    return ucValue;
}

int dvPro_RGBMask(UINT8 ucMask)  //A70LK_Simon_0017
{
    int status = rcSUCCESS;

    status &= dvProAV_WcuRgbMaskSet(ucMask);

    return (status == rcSUCCESS);
}

int dvPro_RGBLSMdSet(bool bSwitch)  //RGB input conversion, default on (0) :  (RGB->Ycbcr) , off (1) //A70LK_Casper_0011
{
    int status = rcSUCCESS;

    status &= dvProAV_AccessWrite(eDcuRgbLslsMd, bSwitch);

    return (status == rcSUCCESS);
}

void dvPro_SetHDRDemo(void)
{
    SclOpuDim mainopu, subopu;
    SclIpuDim mainipu, subipu;
    RxPort sRxPort = 0;

    dvPro_GetCHOutputTiming_2K(eMCT_CH1, m_sChannelInfo[eMCT_CH1].ucPanelIndex);

    dvPro_Set_PIP_PBP_Size(eCM_PIP_SIZE_LARGE);
    dvPro_Set_PIP_PBP_Layout(eCM_MAIN_LAYOUT_PBP_MAIN_LEFT);

    dvPro_Calc_Scaler_PBP_Window(eMCT_CH1);
    dvPro_Calc_Scaler_PBP_Window(eMCT_CH2);

    mainipu.HSmpSt = m_sChannelInfo[eMCT_CH1].sScaler_InputTimingInfo.uiHStart;//260;
    mainipu.VSmpSt = m_sChannelInfo[eMCT_CH1].sScaler_InputTimingInfo.uiVStart;
    mainipu.HSmpSize= m_sChannelInfo[eMCT_CH1].sScaler_InputTimingInfo.uiHActive/2;
    mainipu.VSmpSize= m_sChannelInfo[eMCT_CH1].sScaler_InputTimingInfo.uiVActive;
    mainipu.HWrSize = 0;//m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiHActive;
    mainipu.VWrSize = 0;//m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiVActive;

    mainopu.HDspSt = m_sChannelInfo[eMCT_CH1].sScalerOutputTimingInfo.uiHStart;
    mainopu.VDspSt = m_sChannelInfo[eMCT_CH1].sOutputTimingInfo.uiVStart;
    mainopu.HSize  = m_sChannelInfo[eMCT_CH1].sOutputTimingInfo.uiHActive/2;
    mainopu.VSize  = m_sChannelInfo[eMCT_CH1].sOutputTimingInfo.uiVActive;
    mainopu.HRdSize = 0;//m_sChannelInfo[eCH].sScalerOutputTimingInfo.uiHActive;
    mainopu.VRdSize = 0;//m_sChannelInfo[eCH].sScalerOutputTimingInfo.uiVActive;


    subipu.HSmpSt = m_sChannelInfo[eMCT_CH1].sScaler_InputTimingInfo.uiHStart + m_sChannelInfo[eMCT_CH1].sScaler_InputTimingInfo.uiHActive/2;//260;
    subipu.VSmpSt = m_sChannelInfo[eMCT_CH1].sScaler_InputTimingInfo.uiVStart;
    subipu.HSmpSize= m_sChannelInfo[eMCT_CH1].sScaler_InputTimingInfo.uiHActive/2;
    subipu.VSmpSize= m_sChannelInfo[eMCT_CH1].sScaler_InputTimingInfo.uiVActive;
    subipu.HWrSize = 0;//m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiHActive;
    subipu.VWrSize = 0;//m_sChannelInfo[eCH].sScaler_InputTimingInfo.uiVActive;

    subopu.HDspSt = m_sChannelInfo[eMCT_CH1].sScalerOutputTimingInfo.uiHStart + m_sChannelInfo[eMCT_CH1].sOutputTimingInfo.uiHActive/2;
    subopu.VDspSt = m_sChannelInfo[eMCT_CH1].sOutputTimingInfo.uiVStart;
    subopu.HSize  = m_sChannelInfo[eMCT_CH1].sOutputTimingInfo.uiHActive/2;
    subopu.VSize  = m_sChannelInfo[eMCT_CH1].sOutputTimingInfo.uiVActive;
    subopu.HRdSize = 0;//m_sChannelInfo[eCH].sScalerOutputTimingInfo.uiHActive;
    subopu.VRdSize = 0;//m_sChannelInfo[eCH].sScalerOutputTimingInfo.uiVActive;

    dvProAV_SclVipSourceGet(eMCT_CH1, &sRxPort);
    dvProAV_SclVipSourceSet(eMCT_CH2, sRxPort);

    dvProAV_SclDatapathSet(eSclPath_Pop);
    dvProAV_SclVopWindowOffSet(eSclEntity_Main, false);
    dvProAV_SclVopWindowOffSet(eSclEntity_Sub, false);
    dvProAV_SclOpuScreenOffSet(eSclEntity_Main, false);
    dvProAV_SclOpuScreenOffSet(eSclEntity_Sub, false);

    dvProAV_SclUpdatingSet(eSclEntity_Main, true);
    dvProAV_SclIpuDimSet(eSclEntity_Main, mainipu);
    dvProAV_SclOpuDimSet(eSclEntity_Main, mainopu);
    dvProAV_SclUpdatingSet(eSclEntity_Main, false);

    dvProAV_SclUpdatingSet(eSclEntity_Sub, true);
    dvProAV_SclIpuDimSet(eSclEntity_Sub, subipu);
    dvProAV_SclOpuDimSet(eSclEntity_Sub, subopu);
    dvProAV_SclUpdatingSet(eSclEntity_Sub, false);
}

