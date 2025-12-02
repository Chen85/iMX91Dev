/**
  * @file dvProAV_HdmiTx.c
  *
  *
  */

#include "dvProAV_HdmiTx.h"
#include "dvProAV_MI2c.h"
#include "dvProAV_IndirectAccess.h"

//#define IN_FILE_DEBUG
#ifndef IN_FILE_DEBUG
    #undef DBMSG
    #define DBMSG(...) do{}while(0)
#endif


#define TABLE_UPDATE_DELAY 1  // 1 msec
#define COMMAND_WAIT_DELAY 1  // 1 msec
#define GBX_TX_RETRY_COUNT 20 //  the successor is 5
#define IOPLL_RETRY_COUNT  10 //  the successor is 5
#define FPLL_RETRY_COUNT   10 //  the successor is 5
#define SCDC_RETRY_COUNT   5 //  the successor is 5

// PLL
typedef struct iopll{
    uint32 Freq;
    uint32 M;
    uint32 N;
    uint32 C0;
    uint32 C1;
    uint32 C2;
    uint32 CP;
    uint32 LF;
}IoPll;

typedef struct fpll{
    uint32 Freq;
    uint32 R12B;
    uint32 R12C;
    uint32 R133;
    uint32 R134;
    uint32 R135;
    uint32 R136;
}FPll;

static const IoPll IOPLL_TABLE_V11[] ={ // only for FB Mode   for new IoPLL
// 參考頻率 為148.5M，用來給IOPLL/FPLL參考用
//   FREQ,           M(0x90),        N(0xA0),    C0(0x100),    C1(0x101),    C2(0x102), CP(0x20), LF/BW(0x40)
    {148500000,      0x05,           0x01,        0x0A,         0x0A,        0x00,       0x00000B, 0x000080>>6},
    {185625000,      0x05,           0x01,        0x0A,         0x08,        0x00,       0x00000B, 0x000080>>6},
    {297000000,      0x05,           0x01,        0x05,         0x05,        0x00,       0x00000B, 0x000080>>6},
    {371250000,      0x05,           0x01,        0x05,         0x04,        0x00,       0x00000B, 0x000080>>6},
    {594000000,      0x06,           0x01,        0x03,         0x03,        0x00,       0x000010, 0x0000C0>>6},
    // 100MHz ref clk
    {600000000,      0x06,           0x01,        0x02,         0x02,        0x00,       0x000010, 0x0000C0>>6}
};

static const IoPll IOPLL_TABLE_V12[] ={ // only for FB Mode
// 參考頻率 為270M，用來給IOPLL/FPLL參考用
//   FREQ,           M(0x90),        N(0xA0),    C0(0x100),    C1(0x101),    C2(0x102), CP(0x20), LF/BW(0x40)
    {148500000,         0x0B,           0x04,         0x05,         0x0A,         0x0A,     0x10,   0x0C0>>6},
    {185625000,         0x0B,           0x04,         0x04,         0x08,         0x0A,     0x10,   0x0C0>>6},
    {222750000,         0x21,           0x0A,         0x04,         0x08,         0x0C,     0x10,   0x100>>6},
    {297000000,         0x0B,           0x02,         0x05,         0x0A,         0x0A,     0x10,   0x0C0>>6},
    {371250000,         0x0B,           0x04,         0x08,         0x04,         0x05,     0x10,   0x0C0>>6},
    {445500000,         0x21,           0x0A,         0x08,         0x04,         0x06,     0x10,   0x100>>6},
    {594000000,         0x16,           0x05,         0x08,         0x04,         0x04,     0x0B,   0x0C0>>6}
};

static const FPll FPLL_TABLE_V11[] = {
//   FREQ       0x12B, 0x12C, 0x133, 0x134, 0x135, 0x136
    //  0x12B; mask = 0xFF; // [7:0] cmu_fpll_pll_m_counter
    //  0x12C; mask = 0xFF; // [7:3] cmu_fpll_pll_n_counter, [2:1] cmu_fpll_pll_l_counter, [0] cmu_fpll_pll_m_counter
    //  0x133; mask = 0x0C; // [3:2] cmu_fpll_pll_lf_resistance
    //  0x134; mask = 0x70; // [6:4] cmu_fpll_pll_cp_current_setting, [3:2]cmu_pll_cp_lf_3rd_pole_freq
    //  0x135; mask = 0x07; // [2:0] cmu_fpll_pll_cp_current_setting
    //  0x136; mask = 0x04; // [2] cmu_fpll_pll_vco_ph2_en
    {148500000,  0x28,  0x0E,  0x46,  0x30,  0x03,  0x21 },
    {185625000,  0x19,  0x0C,  0x42,  0x30,  0x03,  0x21 },
    {297000000,  0x28,  0x0C,  0x46,  0x30,  0x03,  0x21 },
    {371250000,  0x19,  0x0A,  0x42,  0x30,  0x03,  0x21 },
    {594000000,  0x28,  0x0A,  0x46,  0x30,  0x03,  0x21 },
    // 100MHz ref clk
    {600000000,  0x3C,  0x0A,  0x46,  0x30,  0x03,  0x21 }
};
static const FPll FPLL_TABLE_V12[] = {
//   FREQ       0x12B, 0x12C, 0x133, 0x134, 0x135, 0x136
    //  0x12B; mask = 0xFF; // [7:0] cmu_fpll_pll_m_counter
    //  0x12C; mask = 0xFF; // [7:3] cmu_fpll_pll_n_counter, [2:1] cmu_fpll_pll_l_counter, [0] cmu_fpll_pll_m_counter
    //  0x133; mask = 0x0C; // [3:2] cmu_fpll_pll_lf_resistance
    //  0x134; mask = 0x70; // [6:4] cmu_fpll_pll_cp_current_setting, [3:2]cmu_pll_cp_lf_3rd_pole_freq
    //  0x135; mask = 0x07; // [2:0] cmu_fpll_pll_cp_current_setting
    //  0x136; mask = 0x04; // [2] cmu_fpll_pll_vco_ph2_en
    {148500000,  0x28,  0x0E,  0x46,  0x30,  0x03,  0x21 },
    {185625000,  0x14,  0x0C,  0x4A,  0x50,  0x03,  0x21 },
    {222750000,  0x14,  0x0C,  0x4A,  0x50,  0x03,  0x21 },
    {297000000,  0x14,  0x0C,  0x4A,  0x50,  0x03,  0x21 },
    {371250000,  0x28,  0x0A,  0x46,  0x30,  0x03,  0x21 },
    {445500000,  0x28,  0x0A,  0x46,  0x30,  0x03,  0x21 },
    {594000000,  0x14,  0x08,  0x4A,  0x50,  0x03,  0x25 }
};

#define OUTPUT_PLL_TABLE_SIZE (sizeof(OUTPUT_PLL_TABLE)/sizeof(OutputPll))
#define FPLL_TABLE_SIZE (sizeof(FPLL_TABLE)/sizeof(FPll))

//--------------------------------------------------------------------------------------
// TX PLL
//--------------------------------------------------------------------------------------
/**
 * @brief 數值轉換為pll的資料格式
 * @param [in] value : 傳入數值
 * @return plldata : 回傳轉換成寫入Pll的資料格式
 * @note
 * Data[15..8] = high_div
 * total_div = high_div + low_div
 * Data[16] = bypass enable
 *   — When Data[16] = 1, bypass is enabled. The selected counter is bypassed with counter division value=1.
 * Data[17] = odd division
 *   — When Data[17] = 0, odd division is disabled. The selected counter duty cycle = high_div/total_div.
 *   — When Data[17] = 1, odd division is enabled. The selected counter duty cycle = (high_div - 0.5)/total_div.
*/
static uint32 value2pll(uint08 u8Value)
{
    uint32 plldata;

    if (u8Value != 1)
    {
        uint08 div2 = u8Value / 2;
        plldata = div2;
        if(u8Value % 2 == 1)
        {
            plldata += 1;
            plldata |= 0x0200;
        }
        plldata = plldata << 8 | div2;
    }
    else
        plldata = 0x10000;

    return plldata;
}

/**
 * @brief 讀取output pll頻率的設定參數
 * @param [in] freq : output pll 頻率
 * @param [out] desc : 回傳output pll 設定參數
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
static int IoPllDescGet(uint32 u32Freq, IoPll *sDesc)
{
    const IoPll *ioplltable;
    int  iopllCount;

    VopRefClock voprefclock = dvProAV_VopRefClockGet();

    if (voprefclock == vrc148p5Mhz)
    {
        ioplltable = IOPLL_TABLE_V11;
        iopllCount = sizeof(IOPLL_TABLE_V11);
    }
    else
    {
        ioplltable = IOPLL_TABLE_V12;
        iopllCount = sizeof(IOPLL_TABLE_V12);
    }

    for (uint16 i = 0; i < iopllCount; i++)
    {
        if (ioplltable[i].Freq == u32Freq)
        {
            sDesc->M = value2pll((uint08)ioplltable[i].M);
            sDesc->N = value2pll((uint08)ioplltable[i].N);
            sDesc->C0 = value2pll((uint08)ioplltable[i].C0);
            sDesc->C1 = value2pll((uint08)ioplltable[i].C1);
            sDesc->C2 = value2pll((uint08)ioplltable[i].C2);

            sDesc->LF = ioplltable[i].LF;
            sDesc->CP = ioplltable[i].CP;
            sDesc->Freq = ioplltable[i].Freq;

            return rcSUCCESS;
        }
    }
    return rcERROR;
}

/**
 * @brief 讀取Fpll頻率的設定參數
 * @param [in] freq : Fpll freq
 * @param [out] desc : 回傳Fpll 設定參數
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
static int FPllDescGet(uint32 u32Freq, FPll *sDesc)
{
    const FPll *fplltable;
    int  fpllCount;

    VopRefClock voprefclock = dvProAV_VopRefClockGet();

    if (voprefclock == vrc148p5Mhz)
    {
        fplltable = FPLL_TABLE_V11;
        fpllCount = sizeof(FPLL_TABLE_V11);
    }
    else
    {
        fplltable = FPLL_TABLE_V12;
        fpllCount = sizeof(FPLL_TABLE_V12);
    }


    for (uint16 i = 0; i < fpllCount; i++)
    {
        if (fplltable[i].Freq == u32Freq)
        {
            sDesc->Freq = fplltable[i].Freq;
            sDesc->R12B = fplltable[i].R12B;
            sDesc->R12C = fplltable[i].R12C;
            sDesc->R133 = fplltable[i].R133;
            sDesc->R134 = fplltable[i].R134;
            sDesc->R135 = fplltable[i].R135;
            sDesc->R136 = fplltable[i].R136;

            return rcSUCCESS;
        }
    }
    return rcERROR;
}

/**
 * @brief 設定Output Pll freq
 * @param [in] freq : output pll 頻率
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
static int dvProAV_IoPllConfigSet(uint32 u32Freq)
{
    int status = rcSUCCESS;
    int retry;
    uint32 locked = 0;
    IoPll pll = {0, 0, 0, 0, 0, 0, 0, 0};

    // 讀取output pll頻率的設定參數
    status &= IoPllDescGet(u32Freq, &pll);
    //status &= OutputPllCDDescGet(freq, &pll, 0);

    status &= dvProAV_AccessWrite(eVopIoPllReset_RECFG, (uint32)true); // Reset
    DelayMSec(1);
    status &= dvProAV_AccessWrite(eVopIoPllReset_RECFG, (uint32)false); // Free Run

    // 寫入新的頻率
    status &= dvProAV_IndirectRegWriteData(eIOPLLPARAM_M, pll.M);
    status &= dvProAV_IndirectRegWriteData(eIOPLLPARAM_N, pll.N);
    status &= dvProAV_IndirectRegWriteData(eIOPLLPARAM_C_C0, pll.C0);
    status &= dvProAV_IndirectRegWriteData(eIOPLLPARAM_C_C1, pll.C1);
    status &= dvProAV_IndirectRegWriteData(eIOPLLPARAM_C_C2, pll.C2);
    status &= dvProAV_IndirectRegWriteData(eIOPLLPARAM_CP, pll.CP);
    status &= dvProAV_IndirectRegWriteData(eIOPLLPARAM_LF, pll.LF);
    status &= dvProAV_IndirectRegWriteData(eIOPLLPARAM_Recfg, 1); // Update IOPLL data, update max data count is 8

    status &= dvProAV_AccessWrite(eVopIoPllReset_PLL, (uint32)true); // Reset
    DelayMSec(1);
    status &= dvProAV_AccessWrite(eVopIoPllReset_PLL, (uint32)false); // Free Run

    // 判斷PLL是否鎖住
    retry = 0;
    while(retry < IOPLL_RETRY_COUNT)
    {
        status &= dvProAV_AccessRead(eVopIoPllLocked, &locked);
        if (locked == 1)
            break;
        retry++;
    }

    if (locked == 0 )
    {
        status = rcERROR;
    }


    return status;
}

/**
 * @brief FPll相關參數設定
 * @param [in] freq : pll 頻率
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
static int dvProAV_FPllConfigSet(uint32 u32Freq)
{
    int status = rcSUCCESS;
    uint32 locked = 0;
    uint08 retry;
    uint32 r133, r134, r135, r136;

    FPll pll = { 0, 0, 0, 0, 0, 0, 0};

    status &= FPllDescGet(u32Freq, &pll);
    r133 = (pll.R133 & 0x0C) >> 2;
    r134 = (pll.R134 & 0x70) >> 4;
    r135 = pll.R135 & 0x07;
    r136 = (pll.R136 & 0x04) >> 2;

    status &= dvProAV_AccessWrite(eVopGxbTxPMAChSel, (uint32)false);    // Reset fPLL & PMA
    status &= dvProAV_AccessWrite(eVopGxbTxPMAReconfEn, (uint32)false); // Reset fPLL & PMA
    status &= dvProAV_AccessWrite(eVopGxbTxReset, (uint32)true);       // Reset fPLL & PMA

    status &= dvProAV_IndirectRegWriteData(eFPLLPARAM_000, 0x02);
    DelayMSec(1);

    status &= dvProAV_IndirectRegWriteData(eFPLLPARAM_12B, pll.R12B);
    status &= dvProAV_IndirectRegWriteData(eFPLLPARAM_12C, pll.R12C);
    status &= dvProAV_IndirectRegWriteData(eFPLLPARAM_133, r133);
    status &= dvProAV_IndirectRegWriteData(eFPLLPARAM_134, r134);
    status &= dvProAV_IndirectRegWriteData(eFPLLPARAM_135, r135);
    status &= dvProAV_IndirectRegWriteData(eFPLLPARAM_136, r136);

    DelayMSec(1);
    status &= dvProAV_IndirectRegWriteData(eFPLLPARAM_100, 0x02);
    status &= dvProAV_IndirectRegWriteData(eFPLLPARAM_000, 0x01);

    retry = 0;
    while(retry < FPLL_RETRY_COUNT)
    {
        status &= dvProAV_AccessRead(eVopFPllLocked, &locked);
        if (locked == 1)
            break;
        retry++;
    }

    if (locked == 0 )
    {
        status = rcERROR;
    }

    return status;
}

/**
 * @brief GxBTx Pll相關參數設定
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
static int dvProAV_GxBTxPllConfigSet(void)
{
    int status = rcSUCCESS;
    uint32 busy;
    uint08 retry;

    // read avalon bus busy state
    status &= dvProAV_AccessRead(eVopGxbTxAvlBusy, &busy);

    // Enable CH0 ~ Ch3
    for (uint32 i = 0 ; i < 4 ; i++)
    {
        DBMSG(">Enable CH %d\n",i);
        status &= dvProAV_AccessWrite(eVopGxbTxPMAChSel, i );     // PMA Channel select
        status &= dvProAV_AccessWrite(eVopGxbTxPMAReconfEn,(uint32)true); // PMA reconfig enable

        retry = 0;
        while(retry < GBX_TX_RETRY_COUNT)
        {
            status &= dvProAV_AccessRead(eVopGxbTxAvlBusy, &busy);
            if (busy == 0)
                break;
            retry++;
        }

        if (busy == 1)
        {
            status = rcERROR;
        }

        DBMSG(">eGXBTXPARAM_000\n");
        status &= dvProAV_IndirectRegWriteData(eGXBTXPARAM_000, 0x02);
        status &= dvProAV_IndirectRegWriteData(eGXBTXPARAM_100, 0x20>>5);
        status &= dvProAV_IndirectRegWriteData(eGXBTXPARAM_000, 0x01);

        retry = 0;
        while(retry < GBX_TX_RETRY_COUNT)
        {
            status &= dvProAV_AccessRead(eVopGxbTxBusy, &busy);
            if (busy == 0)
                break;
            retry++;
        }

        if (busy == 1)
        {
            status = rcERROR;
        }
    }
    // End of Enable CH0~ CH3

    DBMSG(">Disable CH\n");
    status &= dvProAV_AccessWrite(eVopGxbTxPMAReconfEn,(uint32)false);// Disable CH
    status &= dvProAV_AccessWrite(eVopGxbTxReset,(uint32)true); // reset fPLL & PMA
    status &= dvProAV_AccessWrite(eVopGxbTxReset,(uint32)false); // freerun fPLL & PMA

    return status;
}

//-------------------------------------------------------------------------------------------------
// HDMI TX/RX
//-------------------------------------------------------------------------------------------------

/**
 * @brief 設定HDMI TX Color Depth
 * @param [in] Depth : Color bit數??
 *              -eColorDepth_bpp24 = 4, // "0100" = 8 bpc
 *              -eColorDepth_bpp30 = 5, // "0101" = 10 bpc
 *              -eColorDepth_bpp36 = 6, // "0110" = 12 bpc
 *              -eColorDepth_bpp48 = 7  // "0111" = 16 bpc
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
static int dvProAV_HdmiTxColorDepthSet(ColorDepth eDepth)
{
    return dvProAV_AccessWrite(eTxInfoGcpColorDepth, (uint32)eDepth);
}

/**
 * @brief 設定HDMI TX Bit Clock Ratio
 * @param [in] Bcr :
 *              -bcrHdmiV1p4
 *              -bcrHdmiV2p0
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
static int dvProAV_HdmiTxBCRSet(BitClkRatio eBcr)
{
    return dvProAV_AccessWrite(eTxInfoBitClockRatio, (uint32)eBcr);
}

/**
 * @brief 設定HDMI TX Mode
 * @param [in] Mode :
 *              -mDvi
 *              -mHdmi
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
static int dvProAV_HdmiTxModeSet(HdmiMode eMode)
{
    return dvProAV_AccessWrite(eTxInfoTxMode, (uint32)eMode);
}

/**
 * @brief 設定HDMI TX Mute for video and audio
 * @param [in] enable :
 *              -AvMute enable to turn off video and audio
 *              -AvMute disable to turn on video and audio
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_HdmiTxAvMuteSet(bool bEnable)
{
    int status;
    status  = rcSUCCESS;
    status &= dvProAV_AccessWrite(eTxInfoGcpSetAVMute, (uint32)(bEnable));
    status &= dvProAV_AccessWrite(eTxInfoGcpClearAVMute, (uint32)(!bEnable));
    return status;
}

/**
 * @brief 設定HDMI TX SCDC功能為啟用或除能
 * @param [in] enable
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
static int dvProAV_HdmiTxSCDCSet(bool bEnable)
{
    int status = rcSUCCESS;
    uint08 scdc_config;
    uint08 scdc_locked_status;

    uint08 retry = 0;

#if 0
    uint08 scdc_supported;
    // 目前MI2C Read 有問題
    // 判斷是否Support HDMI 2.0
    status &= dvProAV_MI2CRead(0x84, 0xA8, 0x01, &scdc_supported); // retry ~8

    if (status == rcERROR || scdc_supported!=1)
    {
        return rcSUCCESS; // Displayer not support HDMI 2.0
    }
#else
    if (!bEnable)
    {
        return rcSUCCESS; // HDMI 1.4 do nothing and return rcSUCCESS
    }
#endif


    while (retry++ < SCDC_RETRY_COUNT)
    {
        status &= dvProAV_MI2CWrite(0x84, 0xA8, 0x20, bEnable ? 0x03: 0x00); // 連下兩次SCDC成功率較高
        DelayMSec(1000); //delay 1000 ms

        status &= dvProAV_MI2CWrite(0x84, 0xA8, 0x20, bEnable ? 0x03: 0x00);
        DelayMSec(500); //delay 500 ms

        status &= dvProAV_MI2CRead(0x84, 0xA8, 0x40, &scdc_locked_status); // retry ~8
        DBMSG("scdc_locked_status = 0x%x\n",scdc_locked_status  & 0x0F );
        if (status != rcSUCCESS || (scdc_locked_status & 0xF) != 0x0F)
        {
            continue;
        }
        DelayMSec(500); //delay 500 ms
        status &= dvProAV_MI2CRead(0x84, 0xA8, 0x20, &scdc_config);
        if (status == rcSUCCESS && (scdc_config == (bEnable ? 0x03: 0x00)))
        {
            break;
        }
    }

    return status;
}

/**
 * @brief 設HDMI TX IP 重置啟動/或除能
 * @param [in] bEnable
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_HdmiTxIpReset(bool bEnable)
{
    return dvProAV_AccessWrite(eBiuHdmiTxRst, (uint32)bEnable);  // Reset HDMI IP Tx
}

/**
 * @brief Calibre HDMI Tx 參數設定
 * @param [in] bHdmi2p0En :
 *              -0 = Hdmi1.4
 *              -1 = Hdmi2.0
 * @param [in] eColorDepth : Color bit數
 *              -eColorDepth_bpp24 = 4, // "0100" = 8 bpc
 *              -eColorDepth_bpp30 = 5, // "0101" = 10 bpc
 *              -eColorDepth_bpp36 = 6, // "0110" = 12 bpc
 *              -eColorDepth_bpp48 = 7  // "0111" = 16 bpc
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
static int dvProAV_HdmiCalibreTxSet(bool bHdmi2p0En, ColorDepth eColorDepth)
{
    int status = rcSUCCESS;
    uint32 ulWord0 = 0, ulWord1 = 0;
                      //byte3 byte2 byte1 byte0
    uint8 ucWord0[4] = {0x08, 0x1F, 0x00, 0x40};
    uint8 ucWord1[4] = {0x03, 0xF0, 0x00, 0x00};

    ucWord0[3] = (uint8)(eColorDepth<<4) | (uint8)(bHdmi2p0En << 1);
    ucWord0[0] |= bHdmi2p0En << 2;
    ucWord1[0] = (uint8)(eColorDepth - 1);

    for(uint8 i=0; i<sizeof(ucWord0); i++)
    {
        ulWord0 |= ucWord0[i];
        ulWord1 |= ucWord1[i];
        if(i!=3)
        {
            ulWord0 <<= 8;
            ulWord1 <<= 8;
        }
    }

    // Write HDMI TX Protocol Word #0
    status &= dvProAV_AccessWrite(eCTxProtoAvlAddr, 0x00);      //addr[7:0] = 0x00//addr[15:8] = 0x00
    status &= dvProAV_AccessWrite(eCTxProtoAvlMask, 0x00);      //mask[7:0] = 0x00//mask[15:8] = 0x00//mask[23:16] = 0x00//mask[31:24] = 0x00
    status &= dvProAV_AccessWrite(eCTxProtoAvlVal, ulWord0);    //data[7:4] deep color 4'h4 = 24bpp (4'h5 = 30bpp, 4'h6 = 36bpp),[1] disable clk div by4
                                                                //data[15:8] = 0x00 //
                                                                //data[23:16] = 0x1f //[25:16] clock pattern = 10'b0000011111
                                                                //data[31:24] = 0x08 //[27] set HDMI (not DVI), [26] disable scrambling
    status &= dvProAV_AccessWrite(eCTxProtoAvlWr, true);        //OpCode = 0x02 = Write*/

    // Write HDMI TX Protocol Word #1
    status &= dvProAV_AccessWrite(eCTxProtoAvlAddr, 0x01);      //addr[7:0] = 0x00//addr[15:8] = 0x00
    status &= dvProAV_AccessWrite(eCTxProtoAvlMask, 0x00);      //mask[7:0] = 0x00//mask[15:8] = 0x00//mask[23:16] = 0x00//mask[31:24] = 0x00
    status &= dvProAV_AccessWrite(eCTxProtoAvlVal, ulWord1);    //data[7:0] = 0x00   // fifo in
                                                                //data[15:8] = 0x00
                                                                //data[23:16] = 0xf0 //valid pattern = "1111"
                                                                //data[31:24] = 0x03 //repeat the valid pattern every (3+1) cycles
    status &= dvProAV_AccessWrite(eCTxProtoAvlWr, true);        //OpCode = 0x02 = Write*/
    return status;
}

/**
 * @brief 設定HDMI Output相關參數
 * @param [in] u32Freq : 設定頻率
 * @param [in] eColorDepth : Color bit數
 *              -eColorDepth_bpp24 = 4, // "0100" = 8 bpc
 *              -eColorDepth_bpp30 = 5, // "0101" = 10 bpc
 *              -eColorDepth_bpp36 = 6, // "0110" = 12 bpc
 *              -eColorDepth_bpp48 = 7  // "0111" = 16 bpc
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_HdmiTxConfigSet(uint32 u32Freq, ColorDepth eColorDepth)
{
    int status = rcSUCCESS;

    uint08 perColorBits;
    uint32 tmdsClockMhz;
    bool hdmi2p0Enabled = false;
    VopRefClock voprefclock = dvProAV_VopRefClockGet();

    // 設定HDMI TX Bit Clock Ratio
    if (eColorDepth == eColorDepth_bpp24)
        perColorBits = 8;
    else if (eColorDepth == eColorDepth_bpp30)
        perColorBits = 10;
    else if (eColorDepth == eColorDepth_bpp36)
        perColorBits = 12;
    else if (eColorDepth == eColorDepth_bpp48)
        perColorBits = 16;
    else
        perColorBits = 8;

    tmdsClockMhz = (u32Freq / 8) * perColorBits;

    hdmi2p0Enabled = (tmdsClockMhz >= 340000000) ?  true: false; // TMDS clock >= 340MHz for HDMI 2.0

    if (hdmi2p0Enabled)
        status &= dvProAV_HdmiTxBCRSet(eBitClkRatio_HdmiV2p0);
    else
        status &= dvProAV_HdmiTxBCRSet(eBitClkRatio_HdmiV1p4);
    // ProAV Soft state reset
    status &= dvProAV_StateMachineReset(); // Reset State Machine
    // 設定HDMI TX Mode
    status &= dvProAV_HdmiTxModeSet(eHdmiMode_HDMI); // set to HDMI mode
    // 設定HDMI TX Color Depth
    status &= dvProAV_HdmiTxColorDepthSet(eColorDepth);
    DBMSG(">dvProAV_TxColorDepthSet %d, status %d\n", colordepth, status);
    // 設定HDMI TX Mute for video and audio
    status &= dvProAV_HdmiTxAvMuteSet(true); //?
    DBMSG(">dvProAV_TxHdmiAvMuteSet true, status %d\n", status);
    // Cirlibre Tx Config Set
    status &= dvProAV_HdmiCalibreTxSet(hdmi2p0Enabled, eColorDepth);

    if (voprefclock == vrc148p5Mhz)
    {
        status &= dvProAV_AccessWrite(eVopGxbTxReset,(uint32)true);  // GXB_Tx reset

        status &= dvProAV_FPllConfigSet(tmdsClockMhz);
        DBMSG(">dvProAV_FPllConfigSet %uld, status %d\n", tmdsClockMhz, status);

        status &= dvProAV_GxBTxPllConfigSet();
        DBMSG(">dvProAV_GxBTxPllConfigSet, status %d\n", status);

        //RESET MCGB
        status &= dvProAV_AccessWrite(eVopFPllReset_MCGB, true);
        status &= dvProAV_AccessWrite(eVopFPllReset_MCGB, false);

        status &= dvProAV_IoPllConfigSet(tmdsClockMhz);
        DBMSG(">dvProAV_IoPllConfigSet %uld, status %d\n", tmdsClockMhz, status);
    }
    else
    {
        status &= dvProAV_AccessWrite(eVopGxbTxReset,(uint32)true);  // GXB_Tx reset

        status &= dvProAV_FPllConfigSet(tmdsClockMhz);
        DBMSG(">dvProAV_FPllConfigSet %uld, status %d\n", tmdsClockMhz, status);

        status &= dvProAV_GxBTxPllConfigSet();
        DBMSG(">dvProAV_GxBTxPllConfigSet, status %d\n", status);

        //RESET MCGB
        status &= dvProAV_AccessWrite(eVopFPllReset_MCGB, true);
        status &= dvProAV_AccessWrite(eVopFPllReset_MCGB, false);

        status &= dvProAV_IoPllConfigSet(tmdsClockMhz);
        DBMSG(">dvProAV_IoPllConfigSet %uld, status %d\n", tmdsClockMhz, status);
    }

    //-- Ten ms delay before releasing the Xcvr reset: HDMI 2.0 spec only calls
    //-- for one ms but provide some margin.
    DelayMSec(10);

   // status &= dvProAV_HdmiTxSCDCSet(hdmi2p0Enabled);
    DBMSG(">dvProAV_SCDCReconfigMI2CSet %d, status %d\n", hdmi2p0Enabled, status);

    status &= dvProAV_HdmiTxAvMuteSet(false);
    DBMSG(">dvProAV_TxHdmiAvMuteSet false= %d\n", status);

    return status;
}

/**
 * @brief dvProAV_HdmiOutputInit
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_HdmiTxDriverInit(void)
{
    // Initial Arrial10Access Driver
    if (dvProAV_IndirectAccessInit() != rcSUCCESS)
    {
        DBMSG("Arrial10Access Driver Init Fail\n");
        return rcERROR;
    }

    if (dvProAV_MI2CInit(0x84, 100000) != rcSUCCESS)
    {
        DBMSG("ProAVMI2C Driver Init Fail\n");
        return rcERROR;
    }

    return rcSUCCESS;
}

/**
 * @brief 讀取Tx Port的Video Clock(Pixel clock/2)
 * @return Tx video Clock
 */
uint32 dvProAV_HdmiTxVideoClkGet(void)
{
    uint32 ulTxVideoCLK;
    dvProAV_AccessRead(eTxInfoPixClkFreqD2, &ulTxVideoCLK);
    return ulTxVideoCLK;
}

/**
 * @brief 讀取Tx Pixel Clock
 * @return Tx Pixel Clock
 */
uint32 dvProAV_SclTxPixelClkGet(void)
{
    uint32 ulTxVideoCLK;

    dvProAV_AccessRead(eTxInfoPixClkFreqD2, &ulTxVideoCLK);
    ulTxVideoCLK <<= 1;

    return ulTxVideoCLK;
}

//-------------------------------------------------------------------------------------------------
// Vx1
//-------------------------------------------------------------------------------------------------
/**
 * @brief V-by-one Recalibration
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_Vx1ReCal(void)
{
    int status = rcSUCCESS;
    int retry = 0;
    uint32 ucReadData = 0;

    //Recfg Tx-atxPLL
    status &= dvProAV_AccessWrite(eVopFPllReset_MCGB, true);        //RESET MCGB
    status &= dvProAV_AccessWrite(eVopFPllReset_MCGB, false);
    status &= dvProAV_AccessWrite(eVopGxbTxReset, (uint32)true);       // Reset fPLL & PMA

    status &= dvProAV_IndirectRegWriteData(eFPLLPARAM_000, 0x02);
    status &= dvProAV_IndirectRegWriteData(eFPLLPARAM_12B, 0x14);
    status &= dvProAV_IndirectRegWriteData(eFPLLPARAM_12C, 0x0A);
    status &= dvProAV_IndirectRegWriteData(eFPLLPARAM_133, 0x4A);
    status &= dvProAV_IndirectRegWriteData(eFPLLPARAM_134, 0x50);
    status &= dvProAV_IndirectRegWriteData(eFPLLPARAM_135, 0x03);
    status &= dvProAV_IndirectRegWriteData(eFPLLPARAM_136, 0x21);
    status &= dvProAV_IndirectRegWriteData(eFPLLPARAM_100, 0x01);
    status &= dvProAV_IndirectRegWriteData(eFPLLPARAM_000, 0x01);
    // wait for fPLL calibration
    while(retry < FPLL_RETRY_COUNT)
    {
        status &= dvProAV_IndirectRegRead(eFPLLPARAM_280, &ucReadData);
        if((ucReadData & 0x02) == 0x02)
            break;
        retry++;
    }

    //GXB-PMA
    status &= dvProAV_IndirectRegWriteData(eGXBTXPARAM_000, 0x02);
    status &= dvProAV_IndirectRegWriteData(eGXBTXPARAM_100, 0x20);
    status &= dvProAV_IndirectRegWriteData(eGXBTXPARAM_000, 0x01);
    DelayMSec(1);

    status &= dvProAV_AccessWrite(eVopGxbTxReset, (uint32)false);       // Release fPLL & PMA

    return status;
}

/**
 * @brief V-by-one timing change configuration
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_Vx1ChgCfg(uint16 uiHsize, uint16 uiVsize, uint32 uiPxlClk)
{
    int status = rcSUCCESS;
    Vx1Type eType = eVx1_4k60_148p5M;

    (void) uiVsize;
    if(uiHsize == 1920)
    {
        if(uiPxlClk == 148500000)
            eType = eVx1_2k50_60_148p5M;
        else if(uiPxlClk == 297000000)
            eType = eVx1_2k100_120_148p5M;
        else if(uiPxlClk == 270000000)
            eType = eVx1_2k100_120_135M;
    }

    status &= dvProAV_AccessWrite(eBiuVopVx1ChgCfg, eType);

    return status;
}
