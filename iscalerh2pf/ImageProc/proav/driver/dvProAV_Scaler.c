/**
  * @file dvProAV.c
  *
  *
  */

#include "dvProAV_Scaler.h"
#include "dvProAV_MI2c.h"
#include "dvProAV_SclGammaTable.h"
#include "dvProAV_SerialFlash.h"
#include <math.h>

//#define IN_FILE_DEBUG
#ifndef IN_FILE_DEBUG
#undef DBMSG
    #define DBMSG(...) do{}while(0)
    #endif

#define TABLE_UPDATE_DELAY 1  // 1 msec


#ifdef QT_CPP
static const ModeTiming TIMING_MODE_TABLE[]={
    // id, HTotal, VTotal, HSize, VSize, HStart, VStart, HSync, VSync, HPol, VPol, HFreq, VFreq, Interlace
    //                                                                             (0.01) (0.01)
    {   0,   4400,   2250,  3840,  2160,    384,     82,    88,    10,    1,    1, 13500,  6000,        0},
    {   1,   2200,   1125,  1920,  1080,    192,     41,    44,     5,    1,    1,  6750,  6000,        0}
};

static const PanelTiming PANEL_TIMING_TABLE[]={
    // id, HTotal, VTotal, HSize, VSize, HStart, VStart, HSync, VSync, HPol, VPol, FrameRate, PixelClock
    //{   0,   4560,   2194,  3840,  2160,    384,     20,    96,     5,     1,   1,      6000,  600000000},
    {   0,   4400,   2250,  3840,  2160,    384,     82,    88,    10,     1,   1,      6000,  594000000},
    {   1,   4400,   2250,  3840,  2160,    384,     82,    88,    10,     1,   1,      3000,  297000000},
    {   2,   2200,   1125,  1920,  1080,    192,     41,    44,     5,     1,   1,     12000,  297000000},
    {   3,   4400,   2250,  3840,  2160,    384,     82,    88,    10,     1,   1,      1500,  148500000},
    {   4,   2200,   1125,  1920,  1080,    192,     41,    44,     5,     1,   1,      6000,  148500000},
    {   5,   4132,   2420,  4096,  2400,     16,     10,     8,     5,     1,   1,      6000,  600000000},
    {   6,   2072,   1212,  2048,  1200,     24,     10,     4,     2,     1,   1,     24000,  600000000},
};

#if 1
// rule :
//  - H direction parameter have to multiples of "8"
//  - VStart minimum value "10", Dual-pipe 4K 3D VStart minimum value "20"
//  - HTotal > Hsize "28"
static const PanelTiming VX1_PANEL_TIMING_TABLE[]={ // for 600MHz
    // id, HTotal, VTotal, HSize, VSize, HStart, VStart, HSync, VSync, HPol, VPol, FrameRate, PixelClock
    {   0,   4560,   2194,  3840,  2160,    384,     20,    96,     5,     1,   1,      6000,  600000000},
    {   1,   4000,   2500,  2712,  1528,    752,     38,   280,     6,     1,   1,      6000,  600000000},
    {   2,   4000,   2500,  2560,  1600,    752,     38,   280,     6,     1,   1,      6000,  600000000},
    //{   3,   3232,   3094,  1920,  1080,    180,     68,    32,    32,     1,   1,      6000,  600000000},
    {   3,   2200,   2250,  1920,  1080,    152,     36,    32,     5,     1,   1,      6000,  594000000},
    {   4,   3200,   1563,  2712,  1528,    200,     20,    32,     5,     1,   1,     12000,  600000000},
    {   5,   3088,   1620,  2560,  1600,    200,     19,   100,     4,     1,   1,     12000,  600000000},
    {   6,   2500,   2000,  1920,  1080,    180,     68,    32,    32,     1,   1,     12000,  600000000},
    {   7,   3232,   1856,  2688,  1472,    224,    111,   100,     6,     1,   1,     10000,  600000000},
    {   8,   3080,   1624,  2688,  1472,    224,    106,    32,     5,     1,   1,     12000,  600000000},
    {   9,   3080,   1624,  2688,  1472,    224,    106,    32,     5,     1,   1,      6000,  600000000},
    {  10,   2032,   1230,  1920,  1200,     40,     10,    32,     5,     1,   1,     24000,  600000000},
    {  11,   4064,   2460,  3840,  2400,     80,     20,    64,    10,     1,   1,      6000,  600000000},
    {  12,   2280,   1097,  1920,  1080,    136,     10,    32,     4,     1,   1,     24000,  600000000},
    {  13,   4136,   2420,  4096,  2400,     24,     10,     8,     5,     1,   1,      6000,  600000000},
    //{  13,   4132,   2420,  4096,  2400,     16,     10,     8,     5,     1,   1,      6000,  600000000},
    {  14,   2072,   1212,  2048,  1200,     24,     10,     4,     2,     1,   1,     24000,  600000000},
    {  15,   2280,   2194,  1920,  2160,    192,     20,    48,     5,     1,   1,     12000,  600000000},
};
#else
static const PanelTiming VX1_PANEL_TIMING_TABLE[]={ // for 594MHz
    // id, HTotal, VTotal, HSize, VSize, HStart, VStart, HSync, VSync, HPol, VPol, FrameRate, PixelClock
    {   0,   4520,   2190,  3840,  2160,    192,      8,    88,    10,     1,   1,      6000,  594000000},
    {   1,   4768,   2078,  1920,  1080,    144,     34,    32,    36,     1,   1,      6000,  594400000},
    {   2,   2384,   2078,  1920,  1080,    144,     34,    32,    36,     1,   1,     12000,  594400000},
    {   3,   2200,   1125,  1920,  1080,     80,     32,    32,     5,     1,   1,     24000,  594000000},
};
#endif

static const PanelTiming XPR_PANEL_TIMING_TABLE[]={
    // id, HTotal, VTotal, HSize, VSize, HStart, VStart, HSync, VSync, HPol, VPol, FrameRate, PixelClock
    {   0,   2640,   1125,  1920,  1080,    192,     41,    44,     5,     1,   1,      5000,  148500000},
    {   1,   2200,   1125,  1920,  1080,    192,     41,    44,     5,     1,   1,      6000,  148500000},
    {   2,   2400,   1125,  1920,  1080,    192,     41,    44,     5,     1,   1,     10000,  270000000},
    {   3,   2004,   1123,  1920,  1080,     16,     10,    12,     2,     1,   1,     11997,  270000000},
    {   4,   2280,   1097,  1920,  1080,    140,     10,    48,     2,     1,   1,     23989,  600000000},
    {   5,   2404,   1235,  1920,  1200,     36,     10,    32,     2,     1,   1,      5002,  148500000},
    {   6,   2004,   1235,  1920,  1200,     18,     10,    16,     2,     1,   1,      6000,  148500000},
    {   7,   2404,   1235,  1920,  1200,     36,     10,    32,     2,     1,   1,     10004,  297000000},
    {   8,   2004,   1235,  1920,  1200,     20,     10,    16,     2,     1,   1,     12000,  297000000},
    {   9,   4560,   2194,  3840,  2160,    280,     16,    32,     5,     1,   1,      5997,  600000000},
};

#define MODE_TABLE_SIZE (sizeof(TIMING_MODE_TABLE)/sizeof(ModeTiming))
#define PANEL_TABLE_SIZE (sizeof(PANEL_TIMING_TABLE)/sizeof(PanelTiming))
#define VX1_PANEL_TABLE_SIZE (sizeof(VX1_PANEL_TIMING_TABLE)/sizeof(PanelTiming))
#define XPR_PANEL_TABLE_SIZE (sizeof(XPR_PANEL_TIMING_TABLE)/sizeof(PanelTiming))
#endif /* QT_CPP */

static bool s_acIpmFrmbufferEn[eSclEntity_Max] = {false, false};

static const SharpnessParam  SharpnessParamTable[]={
// ScalingRatio, Level, Upx2, SRatio(SharpnessRatio)
    {      2,        1,    0,   0xE1      },    // MINIMUM
    {      2,        2,    0,   0xE5      },
    {      2,        3,    0,   0xE9      },
    {      2,        4,    0,   0xED      },
    {      2,        5,    0,   0xF1      },    // LOW
    {      2,        6,    0,   0x00      },
    {      2,        7,    0,   0x10      },
    {      2,        8,    0,   0x20      },    // NORMAL
    {      2,        9,    0,   0x31      },
    {      2,        10,   0,   0x42      },
    {      2,        11,   0,   0x53      },
    {      2,        12,   0,   0x64      },    // HIGH
    {      2,        13,   0,   0x6D      },
    {      2,        14,   0,   0x76      },
    {      2,        15,   0,   0x7F      },    // MAXIMUM

    {      3,        1,    1,   0xE1      },    // MINIMUM
    {      3,        2,    1,   0xE5      },
    {      3,        3,    1,   0xE9      },
    {      3,        4,    1,   0xED      },
    {      3,        5,    1,   0xF1      },    // LOW
    {      3,        6,    1,   0x00      },
    {      3,        7,    1,   0x10      },
    {      3,        8,    1,   0x20      },    // NORMAL
    {      3,        9,    1,   0x28      },
    {      3,        10,   1,   0x30      },
    {      3,        11,   1,   0x38      },
    {      3,        12,   1,   0x40      },    // HIGH
    {      3,        13,   1,   0x55      },
    {      3,        14,   1,   0x6A      },
    {      3,        15,   1,   0x7F      },    // MAXIMUM

    {      4,        1,    2,   0xE1      },    // MINIMUM
    {      4,        2,    2,   0xE5      },
    {      4,        3,    2,   0xE9      },
    {      4,        4,    2,   0xED      },
    {      4,        5,    2,   0xF1      },    // LOW
    {      4,        6,    2,   0x00      },
    {      4,        7,    2,   0x10      },
    {      4,        8,    2,   0x20      },    // NORMAL
    {      4,        9,    2,   0x28      },
    {      4,        10,   2,   0x30      },
    {      4,        11,   2,   0x38      },
    {      4,        12,   2,   0x40      },    // HIGH
    {      4,        13,   2,   0x4C      },
    {      4,        14,   2,   0x58      },
    {      4,        15,   2,   0x64      }     // MAXIMUM
};

#define SHARPNESS_PARAM_SIZE (sizeof(SharpnessParamTable)/sizeof(SharpnessParam))

typedef struct{
    TpColorBar HColorBar;
    TpColorBar VColorBar;
    TpFill Fill;
    TpRamp HRamp;
    TpRamp VRamp;
    TpChecker Checker;
    TpRnB RnB;
    TpWindow Window;
    TpCrossHatch CrossHatch;
    TpCrossHatch CrossHatch2;
    TpCrossHatch CrossHatch3;
    TpSlideBar SliderBar;
    TpFrame Frame;
    TpRgbT RgbT32Dot;
    TpRgbT TpRgbT64Dot5x5;
    TpRgbT TpRgbT64Dot3x3;
    TpStrip HStrip;
    TpStrip VStrip;
}TpData;


static const TpData m_defaultTpData = {
        { 240 },                               // H Color Bar: hLimitMax
        { 135 },                               // V Color Bar: vLimitMax
        { 1023, 1023, 1023 },                  // Fill: ColorR, ColorG, ColorB
        { 0, 1, 0, 1023},                      // Horzontal Ramp: rampCtrl = 0, hRampCnt, hLimitMin, hLimitMax
        { 1, 1, 0, 1023},                      // Vorzontal Ramp: rampCtrl = 1, vRampCnt, vLimitMin, vLimitMax
        { 1023, 1023, 1023, 480, 270},         // Checker: ColorR, ColorG, ColorB, hLimitMax, vLimitMax
        { 1023, 1023, 960},                    // RnB: ColorR, ColorB, hLimitMax
        { 1023, 1023, 1023, 128, 96},          // Window: ColorR, ColorG, ColorB, hLimitMax, vLimitMax
        {    0, 1023, 1023, 1023, 128, 96, 1}, // CrossHatch(Normal): focusControl =  0,  ColorR, ColorG, ColorB, hLimitMax, vLimitMax, lineWidth
        {    7, 1023, 1023, 1023,   0,  0, 1}, // CrossHatch2(White): focusControl =  7,  ColorR, ColorG, ColorB, (hLimitMax, vLimitMax), lineWidth
        {   11, 1023, 1023, 1023,   0,  0, 1}, // CrossHatch3(RGB): focusControl = 11,  ColorR, ColorG, ColorB, (hLimitMax, vLimitMax), lineWidth
        { 1023, 1023, 1023, 4, 1920, 255},     // eSclTpSlideBar: ColorR, ColorG, ColorB, hLimitMax, vLimitMax, hLimitMin
        { 1023, 1023, 1023, 1},                // eSclTpFrame: ColorR, ColorG, ColorB, lineWidth
        { 1023, 1023, 1023, 3},                // eSclTpRgbT32Dot: focusControl, ColorR, ColorG, ColorB, lineWidth
        { 1023, 1023, 1023, 1},                // eSclTpRgbT64Dot5x5: ColorR, ColorG, ColorB, lineWidth
        { 1023, 1023, 1023, 1},                // eSclTpRgbT64Dot3x3: ColorR, ColorG, ColorB, lineWidth
        { 1023, 1023, 1023, 2},                // HStrip: ColorR, ColorG, ColorB, hLimitMax
        { 1023, 1023, 1023, 2},                // VStrip: ColorR, ColorG, ColorB, vLimitMax
};

static uint32 m_version = 0;

//-------------------------------------------------------------------------------------------------
// PanelTiming & ModeTiming
//-------------------------------------------------------------------------------------------------
#ifdef QT_CPP
/**
 * @brief 讀取ID的mode table value
 * @param [in] id : Timing Mode ID
 * @return TIMING_MODE_TABLE value
 */
const ModeTiming *dvProAV_TimingDescGet(uint16 id)
{
    for (uint16 i = 0; i < MODE_TABLE_SIZE; i++)
    {
        if (id == TIMING_MODE_TABLE[i].id)
        {
           return &TIMING_MODE_TABLE[i];
        }
    }

    return 0;
}

/**
 * @brief Panel Timing value read
 * @param [in] hsize : Panel Horizontal Size
 * @param [in] vsize : Panel Vertical size
 * @param [in] farmerate : Panel farmerate
 * @return PANEL_TIMING_TABLE value
 */
const PanelTiming *dvProAV_PanelTimingDescGet(uint16 hsize, uint16 vsize, uint16 farmerate)
{
    for (uint16 i = 0; i < PANEL_TABLE_SIZE; i++)
    {
        if ( hsize == PANEL_TIMING_TABLE[i].HSize && \
             vsize == PANEL_TIMING_TABLE[i].VSize && \
             farmerate == PANEL_TIMING_TABLE[i].FrameRate)
        {
           return &PANEL_TIMING_TABLE[i];
        }
    }
    return 0;
}

/**
 * @brief Vx1 Panel Timing value get
 * @param [in] hsize : Panel Horizontal Size
 * @param [in] vsize : Panel Vertical size
 * @param [in] farmerate : Panel farmerate
 * @return VX1_PANEL_TIMING_TABLE value
 */
const PanelTiming *dvProAV_Vx1PanelTimingDescGet(uint16 hsize, uint16 vsize, uint16 farmerate)
{
    for (uint16 i = 0; i < VX1_PANEL_TABLE_SIZE; i++)
    {
        if ( hsize == VX1_PANEL_TIMING_TABLE[i].HSize && \
             vsize == VX1_PANEL_TIMING_TABLE[i].VSize && \
             farmerate == VX1_PANEL_TIMING_TABLE[i].FrameRate)
        {
           return &VX1_PANEL_TIMING_TABLE[i];
        }
    }
    return 0;
}

/**
 * @brief XPR Panel Timing value get
 * @param [in] hsize : Panel Horizontal Size
 * @param [in] vsize : Panel Vertical size
 * @param [in] farmerate : Panel farmerate
 * @return XPR_PANEL_TIMING_TABLE value
 */
const PanelTiming *dvProAV_XprPanelTimingDescGet(uint16 hsize, uint16 vsize, uint16 farmerate)
{
    for (uint16 i = 0; i < XPR_PANEL_TABLE_SIZE; i++)
    {
        if ( hsize == XPR_PANEL_TIMING_TABLE[i].HSize && \
             vsize == XPR_PANEL_TIMING_TABLE[i].VSize && \
             farmerate == XPR_PANEL_TIMING_TABLE[i].FrameRate)
        {
           return &XPR_PANEL_TIMING_TABLE[i];
        }
    }
    return 0;
}
#endif /* QT_CPP */

//-------------------------------------------------------------------------------------------------
// Input Bridge
//-------------------------------------------------------------------------------------------------
/**
 * @brief Input bridge Timing get
 * @param [in] port : Rx Port
 *              -eRxPort_0
 *              -eRxPort_1
 *              -eRxPort_2
 *              -eRxPort_3
 * @param [out] timing : Rx Port Timing information
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclIpbTmgGet(RxPort sRxPort, DetTmg *timing)
{
    int status = rcSUCCESS;
    uint8 cRetry = 0;
    uint32 data, ulStable = 0, ulRxStable, ulreg, ulreg1, ulShift;

    if(sRxPort >= eRxPort_Max)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }

    ulShift = (sRxPort * (eInBrgDetHTot1 - eInBrgDetHTot0));

    ulreg = (sRxPort == eRxPort_0)? eRx0InfoChActive : (sRxPort == eRxPort_1)? eRx1InfoChActive :
          (sRxPort == eRxPort_2)? eRx2InfoChActive : eRx3InfoChActive;

    ulreg1 = (sRxPort == eRxPort_0)? eInBrgDetStable0 : (sRxPort == eRxPort_1)? eInBrgDetStable1 :
           (sRxPort == eRxPort_2)? eInBrgDetStable2 : eInBrgDetStable3;

    while(!ulStable)
    {
        status &= dvProAV_AccessRead(ulreg, &ulRxStable);
        cRetry++;
        if(!ulRxStable || cRetry > 5)
        {
            timing->Hs.Total = 0;
            timing->Hs.Start = 0;
            timing->Hs.Size = 0;
            timing->Vs.Total = 0;
            timing->Vs.Start = 0;
            timing->Vs.Size = 0;
            timing->Interlace = 0;
            return rcERROR;
        }
        status &= dvProAV_AccessRead(ulreg1, &ulStable);
    }

    status &= dvProAV_AccessRead((uint32)(eInBrgDetHTot0  + ulShift), &data);
    timing->Hs.Total = (uint16)data;
    status &= dvProAV_AccessRead((uint32)(eInBrgDetHSt0   + ulShift), &data);
    timing->Hs.Start = (uint16)data;
    status &= dvProAV_AccessRead((uint32)(eInBrgDetHSize0 + ulShift), &data);
    timing->Hs.Size = (uint16)data;
    status &= dvProAV_AccessRead((uint32)(eInBrgDetVTot0  + ulShift), &data);
    timing->Vs.Total = (uint16)data;
    status &= dvProAV_AccessRead((uint32)(eInBrgDetVSt0   + ulShift), &data);
    timing->Vs.Start = (uint16)data;
    status &= dvProAV_AccessRead((uint32)(eInBrgDetVSize0 + ulShift), &data);
    timing->Vs.Size = (uint16)data;
    status &= dvProAV_AccessRead((uint32)(eInBrgDetIntlc0 + ulShift), &data);
    timing->Interlace = (bool)data;

    return status;
}

//-------------------------------------------------------------------------------------------------
// Input Bridge Source
//-------------------------------------------------------------------------------------------------
/**
 * @brief Input Bridge source Timing set
 * @param [in] port : Rx Port
 *              -eRxPort_0
 *              -eRxPort_1
 *              -eRxPort_2
 *              -eRxPort_3
 * @param [in] timing : Rx Port Timing information
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SrcTimingSet(RxPort sRxPort, DetTmg *timing)
{
    int status = rcSUCCESS;
    uint32 offset = sRxPort * (eIbSrcP1HSmpSt - eIbSrcP0HSmpSt);
    uint32 offset1 = sRxPort * (eIbSrcP1IntlcMode - eIbSrcP0IntlcMode);

    if(sRxPort >= eRxPort_Max)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }

    // Ipu parameter
    status &= dvProAV_AccessWrite(eIbSrcP0HSmpSz + offset, timing->Hs.Size);
    status &= dvProAV_AccessWrite(eIbSrcP0HSmpSt + offset, timing->Hs.Start);
    status &= dvProAV_AccessWrite(eIbSrcP0HWrSz + offset, 0); // auto
    status &= dvProAV_AccessWrite(eIbSrcP0VSmpSz + offset, timing->Vs.Size);
    status &= dvProAV_AccessWrite(eIbSrcP0VSmpSt + offset, timing->Vs.Start);
    status &= dvProAV_AccessWrite(eIbSrcP0VWrSz + offset, 0); // auto
    status &= dvProAV_AccessWrite(eIbSrcP0IntlcMode + offset1, timing->Interlace);

#if 0
    // for backup switch
    if(dvProAV_SclDatapathGet() == eSclPath_BackupSw)
    {
        UINT8 cCurrentSource = 0;
        status &= (UINT8)dvProAV_SclBackupSourceGet(&cCurrentSource);

        if(cCurrentSource)
        {
            offset = (sRxPort == eRxPort_2) ? eRxPort_0 * (eIbSrcP1HSmpSt - eIbSrcP0HSmpSt) : eRxPort_2 * (eIbSrcP1HSmpSt - eIbSrcP0HSmpSt);
            offset1 = (sRxPort == eRxPort_2) ? eRxPort_0 * (eIbSrcP1IntlcMode - eIbSrcP0IntlcMode) : eRxPort_2 * (eIbSrcP1IntlcMode - eIbSrcP0IntlcMode);

            status &= dvProAV_AccessWrite(eIbSrcP0Upd + offset1, true);

            // Ipu parameter
            status &= dvProAV_AccessWrite(eIbSrcP0HSmpSz + offset, timing->Hs.Size);
            status &= dvProAV_AccessWrite(eIbSrcP0HSmpSt + offset, timing->Hs.Start);
            status &= dvProAV_AccessWrite(eIbSrcP0HWrSz + offset, 0); // auto
            status &= dvProAV_AccessWrite(eIbSrcP0VSmpSz + offset, timing->Vs.Size);
            status &= dvProAV_AccessWrite(eIbSrcP0VSmpSt + offset, timing->Vs.Start);
            status &= dvProAV_AccessWrite(eIbSrcP0VWrSz + offset, 0); // auto
            status &= dvProAV_AccessWrite(eIbSrcP0IntlcMode + offset1, timing->Interlace);

            status &= dvProAV_AccessWrite(eIbSrcP0Upd + offset1, false);
        }
    }
#endif /* 0 */
    return status;
}

/**
 * @brief Input Bridge Source Updating Set
 * @param [in] port : Rx Port
 *              -eRxPort_0
 *              -eRxPort_1
 *              -eRxPort_2
 *              -eRxPort_3
 * @param [in] bUpdatingEn : Input Bridge source updating flag
 *              -false : 參數設定完成, 同步參數至Input Bridge Source
 *              -true : 參數設定中, 先不同步參數至Input Bridge Source
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SrcUpdatSet(RxPort sRxPort, bool bUpdatingEn)
{
    uint32 offset = sRxPort * (eIbSrcP1IntlcMode - eIbSrcP0IntlcMode);

    if(sRxPort >= eRxPort_Max)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }
    return dvProAV_AccessWrite(eIbSrcP0Upd + offset, bUpdatingEn);
}

/**
 * @brief Input Bridge Source pixel mode set
 * @param [in] port : Rx Port
 *              -eRxPort_0
 *              -eRxPort_1
 *              -eRxPort_2
 *              -eRxPort_3
 * @param [in] mode 輸入埠的 pixel mode
 *             - 0x00, 0x01: 444
 *             - 0x02: 422
 *             - 0x03: 420
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SrcPixelModeSet(RxPort sRxPort, uint08 mode)
{
    if((mode > 3) || (sRxPort >= eRxPort_Max))
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }

    uint32 index = (sRxPort == eRxPort_0)? eIbSrcP0PixMd : (sRxPort == eRxPort_1)? eIbSrcP1PixMd :
            (sRxPort == eRxPort_2)? eIbSrcP2PixMd : eIbSrcP3PixMd;

    return dvProAV_AccessWrite(index, (uint32)mode);
}

/**
 * @brief Input Bridge Source pixel mode get
 * @param [in] port : Rx Port
 *              -eRxPort_0
 *              -eRxPort_1
 *              -eRxPort_2
 *              -eRxPort_3
 * @return mode :
 *             - 0x00, 0x01: 444
 *             - 0x02: 422
 *             - 0x03: 420
 */
int dvProAV_SrcPixelModeGet(RxPort sRxPort)
{
    if(sRxPort >= eRxPort_Max)
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
    uint32 index, mode;
    index = (sRxPort == eRxPort_0)? eIbSrcP0PixMd : (sRxPort == eRxPort_1)? eIbSrcP1PixMd :
            (sRxPort == eRxPort_2)? eIbSrcP2PixMd : eIbSrcP3PixMd;

    dvProAV_AccessRead(index, &mode);

    return (int)mode;
}

/**
 * @brief Input Bridge Source Color Space set
 * @param [in] sRxPort : Rx port
 *              -eRxPort_0
 *              -eRxPort_1
 *              -eRxPort_2
 *              -eRxPort_3
 * @param [in] mode  color space mode
 *             - 0: RGB
 *             - 1: YCbCr 601
 *             - 2: YCbCr 709
 *             - 3: YCbCr 2020
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SrcColorSpaceSet(RxPort sRxPort, uint08 mode)
{
    if((mode > 3) || (sRxPort >= eRxPort_Max))
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }

    uint32 ulIndex = (sRxPort == eRxPort_0)? eIbSrcP0ColIn : (sRxPort == eRxPort_1)? eIbSrcP1ColIn :
              (sRxPort == eRxPort_2)? eIbSrcP2ColIn : eIbSrcP3ColIn;

    return dvProAV_AccessWrite(ulIndex, (uint32)mode);
}

/**
 * @brief Input Bridge Source 3D mode set
 * @param [in] sRxPort : Rx port
 *              -eRxPort_0
 *              -eRxPort_1
 *              -eRxPort_2
 *              -eRxPort_3
 * @param [in] mode : 3D mode
 *              -eScl3DMode_Off
 *              -eScl3DMode_FramePacking
 *              -eScl3DMode_SideBySide
 *              -eScl3DMode_TopAndBottom
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_Src3DModeSet(RxPort sRxPort, Scl3DMode mode)
{
    if((mode > eScl3DMode_TopAndBottom) || (sRxPort >= eRxPort_Max))
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }
    uint32 index = (sRxPort == eRxPort_0)? eIbSrcP03D : (sRxPort == eRxPort_1)? eIbSrcP13D :
            (sRxPort == eRxPort_2)? eIbSrcP23D : eIbSrcP33D;

    return dvProAV_AccessWrite(index, mode);
}

/**
 * @brief Input Bridge Source 3D mode get
 * @param [in] sRxPort : Rx port
 *              -eRxPort_0
 *              -eRxPort_1
 *              -eRxPort_2
 *              -eRxPort_3
 * @param [out] mode : 3D mode
 *              -e3D_Off
 *              -e3D_FramePacking,
 *              -e3D_SideBySide,
 *              -e3D_TopAndBottom
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_Src3DModeGet(RxPort sRxPort, Scl3DMode *mode)
{
    int status = rcSUCCESS;
    uint32 index, Mode = 0;

    if (sRxPort >= eRxPort_Max)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }

    index = (sRxPort == eRxPort_0)? eIbSrcP03D : (sRxPort == eRxPort_1)? eIbSrcP13D :
            (sRxPort == eRxPort_2)? eIbSrcP23D : eIbSrcP33D;

    status &= dvProAV_AccessRead(index, &Mode);

    *mode = (Scl3DMode)Mode;

    return status;
}

/**
 * @brief Input Bridge Source Scaler RGB Range set
 * @param [in] sRxPort :
 *              -eRxPort_0 : Rx0
 *              -eRxPort_1 : Rx1
 *              -eRxPort_2 : Rx2
 *              -eRxPort_3 : Rx3
 * @param [in] bLim2FullEn :
 *              -false = limit to full Disable
 *              -true  = limit to full Enable
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SrcLim2FulSet(RxPort sRxPort, bool bLim2FullEn)
{
    uint32 ulShift = sRxPort * (eIbSrcP1Lim2ful - eIbSrcP0Lim2ful);
    if(sRxPort >= eRxPort_Max)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }

    return dvProAV_AccessWrite(eIbSrcP0Lim2ful + ulShift, (uint32)bLim2FullEn);
}

/**
 * @brief Input Bridge Source Output/Input Frame Rate Ratio set
 * @param [in] sRxPort :
 *              -eRxPort_0 : Rx0
 *              -eRxPort_1 : Rx1
 *              -eRxPort_2 : Rx2
 *              -eRxPort_3 : Rx3
 * @param [in] dIptFrameRate : Input frame rate
 * @param [in] dOptFrameRate : Output frame rate
 * @param [in] uiIptVtotal : Input Vtotal
 * @param [in] uiOptVtotal : Output Vtotal
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SrcFrameRateRatioSet(RxPort sRxPort, DOUBLE dIptFrameRate, DOUBLE dOptFrameRate, uint16 uiIptVtotal, uint16 uiOptVtotal)
{
    if(sRxPort >= eRxPort_Max)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }

    DOUBLE dFrmRateRatio = 0; //IptFrameRate = 0, dOptFrameRate = 0,
    uint32 ulFrmRateRatio = 0;
    uint32 ulReg = (sRxPort == eRxPort_0) ? eIbSrcP0FrmRateRatio : (sRxPort == eRxPort_1) ? eIbSrcP1FrmRateRatio :
                   (sRxPort == eRxPort_2) ? eIbSrcP2FrmRateRatio : eIbSrcP3FrmRateRatio;

    //dFrmRateRatio = (dOptFrameRate * (DOUBLE)((DOUBLE)uiOptVtotal / (DOUBLE)uiIptVtotal) * (DOUBLE)uiOptVsize) / (dIptFrameRate * (DOUBLE)uiIptVsize);

    dFrmRateRatio = (dOptFrameRate * (DOUBLE)((DOUBLE)uiOptVtotal / (DOUBLE)uiIptVtotal)) / (dIptFrameRate);

    //printf("%f %f %d %d %d %f %d \n", dFrmRateRatio, dOptFrameRate, uiOptVtotal, uiIptVtotal, uiOptVsize, dIptFrameRate, uiIptVsize);

    ulFrmRateRatio = (uint32)(dFrmRateRatio * 1024);

    return dvProAV_AccessWrite(ulReg, (uint32)ulFrmRateRatio);
}

/*
 * @brief Input Bridge Source Limit to Full Dither set
 * @param [in] sRxPort :
 *              -eRxPort_0 : Rx0
 *              -eRxPort_1 : Rx1
 *              -eRxPort_2 : Rx2
 *              -eRxPort_3 : Rx3
 * @param [in] bLim2FullEn :
 *              -false = limit to full Dither Disable
 *              -true  = limit to full Dither Enable
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SrcL2fDitherSet(RxPort sRxPort, bool bEnable)
{
    if(sRxPort >= eRxPort_Max)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }

    uint32 ulReg = (sRxPort == eRxPort_0) ? eIbSrcP0L2FDith : (sRxPort == eRxPort_1) ? eIbSrcP1L2FDith :
                   (sRxPort == eRxPort_2) ? eIbSrcP2L2FDith : eIbSrcP3L2FDith;

    return dvProAV_AccessWrite(ulReg, (uint32)bEnable);
}

//-------------------------------------------------------------------------------------------------
// Scaler Data Path
//-------------------------------------------------------------------------------------------------
/**
 * @brief 設定 ProAV active / Passthrough FIFO active
 * @param [in] Enable : Disable = 0 / Enable = 1
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclByPassEnableSet(bool Enable)
{
    return dvProAV_AccessWrite(eBiuPassThrough, (uint32)Enable);
}

/**
 * @brief 設定輸入訊號路徑(Datapath)
 * @param [in] VidPath
 *              -eSclPath_SingleFBL     // Ipm -> Opm, Ops open [fb-less]
 *              -eSclPath_Signle        // Ipm -> Miu -> Opm, Ops open [fb]
 *              -eSclPath_Pip           // Ipm & Ips -> Miu -> Opm, Ops open [PIP]
 *              -eSclPath_BackupSw      // Ipm or Ips -> Opm, Ops open [Backup Switch], only support in frame buffer mode
 *              -eSclPath_PopFBL        // Ipm -> Opm, Ips -> Miu -> Ops
 *              -eSclPath_Pop           // Ipm -> Miu -> Opm, Ips -> Miu -> Ops
 *              -eSclPath_SeamlessSw    // Ipm -> Miu -> Opm, Ips -> Miu -> Ops, [Seamless switch]
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclDatapathSet(SclDataPath dataPath)
{
    int status = rcSUCCESS;
    if(dataPath >= eSclPath_Max)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }

    status &= dvProAV_AccessWrite(eIpmFrmbufferEn, (uint32)false);
    status &= dvProAV_AccessWrite(eIpsFrmbufferEn, (uint32)false);

    status &= dvProAV_AccessWrite(eDcuDataPathCfg, dataPath);

    status &= dvProAV_AccessWrite(eIpmFrmbufferEn, (uint32)s_acIpmFrmbufferEn[0]);
    status &= dvProAV_AccessWrite(eIpsFrmbufferEn, (uint32)s_acIpmFrmbufferEn[1]);

    if (dataPath == eSclPath_SingleFBL || dataPath == eSclPath_PopFBL)
        status &= dvProAV_AccessWrite(eBiuTxRefClkSel, (uint32)true); // 1: Frame buffer less mode
    else
        status &= dvProAV_AccessWrite(eBiuTxRefClkSel, (uint32)false); // 0: Frame buffer mode

    return status;
}

/**
 * @brief 讀取輸入訊號路徑(Datapath)
 * @return VidPath :
 *              -eSclPath_SingleFBL     // Ipm -> Opm, Ops open [fb-less]
 *              -eSclPath_Signle        // Ipm -> Miu -> Opm, Ops open [fb]
 *              -eSclPath_Pip           // Ipm & Ips -> Miu -> Opm, Ops open [PIP]
 *              -eSclPath_BackupSw      // Ipm or Ips -> Opm, Ops open [Backup Switch], only support in frame buffer mode
 *              -eSclPath_PopFBL        // Ipm -> Opm, Ips -> Miu -> Ops
 *              -eSclPath_Pop           // Ipm -> Miu -> Opm, Ips -> Miu -> Ops
 *              -eSclPath_SeamlessSw    // Ipm -> Miu -> Opm, Ips -> Miu -> Ops, [Seamless switch]
 */
SclDataPath dvProAV_SclDatapathGet(void)
{
    uint32 ulDataPath;

    dvProAV_AccessRead(eDcuDataPathCfg, &ulDataPath);

    return (SclDataPath)ulDataPath;
}

/**
 * @brief input port set
 * @param [in] eEntity : 通道選擇
 *              -eSclEntity_Main
 *              -eSclEntity_Sub
 * @param [in] Num : Rx port Number
 *              -eRxPort_0
 *              -eRxPort_1
 *              -eRxPort_2
 *              -eRxPort_3
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclVipSourceSet(SclEntity eEntity, RxPort sRxPort)
{
    uint32 index;

    if((eEntity >= eSclEntity_Max) || (sRxPort >= eRxPort_Max))
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }

    index = (eEntity == eSclEntity_Main) ? eIpmSourceSel : eIpsSourceSel;

    return dvProAV_AccessWrite(index, (uint32)sRxPort);
}

/**
 * @brief 讀取目前輸入port
 * @param [in] eEntity : 通道選擇
 *              -eSclEntity_Main
 *              -eSclEntity_Sub
 * @param [out] Num : port Number
 *              -eRxPort_0
 *              -eRxPort_1
 *              -eRxPort_2
 *              -eRxPort_3
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclVipSourceGet(SclEntity eEntity, RxPort *sRxPort)
{
    int status = rcSUCCESS;
    uint32 index, data;

    if(eEntity >= eSclEntity_Max)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }

    index = (eEntity == eSclEntity_Main) ? eIpmSourceSel : eIpsSourceSel;
    status = dvProAV_AccessRead(index, &data);
    *sRxPort = (RxPort)data;

    return status;
}

/**
 * @brief Get Backup current port
 * @return eEntity : 當前選擇通道
 *          - 0 : use main path
 *          - 1 : use backup path
 */
int dvProAV_SclBackupSourceGet(uint08 *pcEntity)
{
    int status = rcSUCCESS;
    uint32 data;

    status = dvProAV_AccessRead(eDcuBackupState, &data);

    *pcEntity = (uint08)data;

    return status;
}

//-------------------------------------------------------------------------------------------------
// Scaler Input Port Information
//-------------------------------------------------------------------------------------------------
/**
 * @brief Input Port Info Get
 * @param [in] eEntity : 通道選擇
 *              -eSclEntity_Main
 *              -eSclEntity_Sub
 * @param [out] portstatus
 *        - 0: unstable
 *        - 1: stable
 *        - 2: changed
 *        - 3: update failed
 * @param [out] currentTiming 目前的Timing
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclInputPortInfoGet(SclEntity eEntity, uint08 *portstatus, DetTmg *currentTiming)
{
    int status = rcSUCCESS;
    if(eEntity >= eSclEntity_Max)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }
    uint32 rxsrc;
    uint32 changed = 0;
    uint32 stable = 0;
    uint32 index;

    *portstatus = 0;

    index = (eEntity == eSclEntity_Main) ? eIpmSourceSel : eIpsSourceSel;

    status &= dvProAV_AccessRead(index, &rxsrc);

    if (rxsrc == (uint32)eRxPort_0)
    {
        status &=  dvProAV_AccessRead(eRx0InfoChActive, &stable);
        status &=  dvProAV_AccessRead(eBiuRx0TmgChg, &changed);
    }
    else if (rxsrc == (uint32)eRxPort_1)
    {
        status &=  dvProAV_AccessRead(eRx1InfoChActive, &stable);
        status &=  dvProAV_AccessRead(eBiuRx1TmgChg, &changed);
    }
    else if (rxsrc == (uint32)eRxPort_2)
    {
        status &=  dvProAV_AccessRead(eRx2InfoChActive, &stable);
        status &=  dvProAV_AccessRead(eBiuRx2TmgChg, &changed);
    }
    else if (rxsrc == (uint32)eRxPort_3)
    {
        status &=  dvProAV_AccessRead(eRx3InfoChActive, &stable);
        status &=  dvProAV_AccessRead(eBiuRx3TmgChg, &changed);
    }
    else
    {
        status = rcERROR;
    }

    if (status != rcSUCCESS)
        return status;

    if (stable)
        *portstatus = (changed == 1) ? eInpStatusChanged : eInpStatusStable;
    else
       *portstatus = eInpStatusUnstable; // unstable

    // Update the currentTiming
    status &=  dvProAV_SclIpbTmgGet(rxsrc, currentTiming);  // dvProAV_SclVipTmgGet(eEntity, currentTiming);

    if (status != rcSUCCESS)
        *portstatus = eInpStatusUpdateFailed;

    return status;
}

/**
 * @brief 讀取Vip Timing
 * @param [in] eEntity : 通道選擇
 *              -eSclEntity_Main
 *              -eSclEntity_Sub
 * @param [out] timing : Vip Timing information
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclVipTmgGet(SclEntity eEntity, DetTmg *timing)
{
    int status = rcSUCCESS;
    if(eEntity >= eSclEntity_Max)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }
    uint32 data, ulValid = 0, ulRxStable = 0, ulShift, ulReg;
    RxPort sRxPort = eRxPort_0;

    ulShift = (eEntity == eSclEntity_Main) ? 0 : (eIpsDetHTot - eIpmDetHTot);

    status &= dvProAV_SclVipSourceGet(eEntity, &sRxPort);
    ulReg = (sRxPort == eRxPort_0) ? eRx0InfoChActive : (sRxPort == eRxPort_1) ? eRx1InfoChActive :
            (sRxPort == eRxPort_2) ? eRx2InfoChActive : (sRxPort == eRxPort_3) ? eRx3InfoChActive : eRx0InfoChActive;

    while(!ulValid)
    {
        status &= dvProAV_AccessRead(ulReg, &ulRxStable);
        if(!ulRxStable)
            return rcERROR;
        status &= dvProAV_AccessRead(eIpmInputStbl + ulShift, &ulValid);
    }

    status &= dvProAV_AccessRead(eIpmDetHTot + ulShift, &data);
    timing->Hs.Total = (uint16)data;
    status &= dvProAV_AccessRead(eIpmDetHSt + ulShift, &data);
    timing->Hs.Start = (uint16)data;
    status &= dvProAV_AccessRead(eIpmDetHSize + ulShift, &data);
    timing->Hs.Size = (uint16)data;
    status &= dvProAV_AccessRead(eIpmDetVTot + ulShift, &data);
    timing->Vs.Total = (uint16)data;
    status &= dvProAV_AccessRead(eIpmDetVSt + ulShift, &data);
    timing->Vs.Start = (uint16)data;
    status &= dvProAV_AccessRead(eIpmDetVSize + ulShift, &data);
    timing->Vs.Size = (uint16)data;
    status &= dvProAV_AccessRead(eIpmIntlcIn + ulShift, &data);
    timing->Interlace = (bool)data;

    return status;
}

/**
 * @brief 讀取Scaler input port timing detect 是否Stable
 * @param [in] eEntity : 通道選擇
 *              -eSclEntity_Main
 *              -eSclEntity_Sub
 * @param [out] stable
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclInputStableGet(SclEntity eEntity, bool *stable)
{
    int status = rcSUCCESS;
    if(eEntity >= eSclEntity_Max)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }
    uint32 index, value;

    index = (eEntity == eSclEntity_Main) ? eIpmInputStbl : eIpsInputStbl;

    status = dvProAV_AccessRead(index, &value);

    *stable = (status == rcSUCCESS) ? (value != 0) : false;

    return status;
}

/**
 * @brief Input Port Info clear
 * @param [in] eEntity : 通道選擇
 *              -eSclEntity_Main
 *              -eSclEntity_Sub
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclInputPortInfoClear(SclEntity eEntity)
{
    int status = rcSUCCESS;
    if(eEntity >= eSclEntity_Max)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }
    uint32 rxsrc, index;

    index = (eEntity == eSclEntity_Main) ? eIpmSourceSel : eIpsSourceSel;

    status &= dvProAV_AccessRead(index, &rxsrc);

    if (rxsrc == (uint32)eRxPort_0)
    {
        status &= dvProAV_AccessWrite(eBiuRx0TmgChg, (uint32)1); // clear flag
    }
    else if (rxsrc == (uint32)eRxPort_1)
    {
        status &= dvProAV_AccessWrite(eBiuRx1TmgChg, (uint32)1); // clear flag
    }
    else if (rxsrc == (uint32)eRxPort_2)
    {
        status &= dvProAV_AccessWrite(eBiuRx2TmgChg, (uint32)1); // clear flag
    }
    else if (rxsrc == (uint32)eRxPort_3)
    {
        status &= dvProAV_AccessWrite(eBiuRx3TmgChg, (uint32)1); // clear flag
    }
    else
    {
        status = rcERROR;
    }

    return status;
}

/**
 * @brief 設定IPU相關參數
 * @param [in] eEntity : 通道選擇
 *              -eSclEntity_Main
 *              -eSclEntity_Sub
 * @param [in] dim : Ipu相關參數
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclIpuDimSet(SclEntity eEntity, SclIpuDim dim)
{
    int status = rcSUCCESS;
    if(eEntity >= eSclEntity_Max)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }

    status &= dvProAV_AccessWrite((eEntity == eSclEntity_Main) ? eIpmHSmpSt : eIpsHSmpSt, (uint32)dim.HSmpSt);
    status &= dvProAV_AccessWrite((eEntity == eSclEntity_Main) ? eIpmHSmpSz : eIpsHSmpSz, (uint32)dim.HSmpSize);
    status &= dvProAV_AccessWrite((eEntity == eSclEntity_Main) ? eIpmHWrSz : eIpsHWrSz, (uint32)dim.HWrSize);
    status &= dvProAV_AccessWrite((eEntity == eSclEntity_Main) ? eIpmVSmpSt : eIpsVSmpSt, (uint32)dim.VSmpSt);
    status &= dvProAV_AccessWrite((eEntity == eSclEntity_Main) ? eIpmVSmpSz : eIpsVSmpSz, (uint32)dim.VSmpSize);
    status &= dvProAV_AccessWrite((eEntity == eSclEntity_Main) ? eIpmVWrSz : eIpsVWrSz, (uint32)dim.VWrSize);

    // for backup switch
    if((dvProAV_SclDatapathGet() == eSclPath_BackupSw) && (eEntity == eSclEntity_Main))
    {
        //UINT8 cCurrentSource = 0;
        //status &= (UINT8)dvProAV_SclBackupSourceGet(&cCurrentSource);

        //if(cCurrentSource)
        {
            status &= dvProAV_AccessWrite((eEntity == eSclEntity_Main) ? eIpsHSmpSt : eIpmHSmpSt, (uint32)dim.HSmpSt);
            status &= dvProAV_AccessWrite((eEntity == eSclEntity_Main) ? eIpsHSmpSz : eIpmHSmpSz, (uint32)dim.HSmpSize);
            status &= dvProAV_AccessWrite((eEntity == eSclEntity_Main) ? eIpsHWrSz : eIpmHWrSz, (uint32)dim.HWrSize);
            status &= dvProAV_AccessWrite((eEntity == eSclEntity_Main) ? eIpsVSmpSt : eIpmVSmpSt, (uint32)dim.VSmpSt);
            status &= dvProAV_AccessWrite((eEntity == eSclEntity_Main) ? eIpsVSmpSz : eIpmVSmpSz, (uint32)dim.VSmpSize);
            status &= dvProAV_AccessWrite((eEntity == eSclEntity_Main) ? eIpsVWrSz : eIpmVWrSz, (uint32)dim.VWrSize);
        }
    }
    return status;
}

/**
 * @brief 讀取IPU相關參數
 * @param [in] eEntity : 通道選擇
 *              -eSclEntity_Main
 *              -eSclEntity_Sub
 * @param [out] dim : Ipu相關參數
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclIpuDimGet(SclEntity eEntity, SclIpuDim *dim)
{
    int status = rcSUCCESS;
    if(eEntity >= eSclEntity_Max)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }
    uint32 data;

    status &= dvProAV_AccessRead((eEntity == eSclEntity_Main) ? eIpmHSmpSt : eIpsHSmpSt, &data);
    dim->HSmpSt = (uint16)data;
    status &= dvProAV_AccessRead((eEntity == eSclEntity_Main) ? eIpmHSmpSz : eIpsHSmpSz, &data);
    dim->HSmpSize = (uint16)data;
    status &= dvProAV_AccessRead((eEntity == eSclEntity_Main) ? eIpmHWrSz : eIpsHWrSz, &data);
    dim->HWrSize = (uint16)data;
    status &= dvProAV_AccessRead((eEntity == eSclEntity_Main) ? eIpmVSmpSt : eIpsVSmpSt, &data);
    dim->VSmpSt = (uint16)data;
    status &= dvProAV_AccessRead((eEntity == eSclEntity_Main) ? eIpmVSmpSz : eIpsVSmpSz, &data);
    dim->VSmpSize = (uint16)data;
    status &= dvProAV_AccessRead((eEntity == eSclEntity_Main) ? eIpmVWrSz : eIpsVWrSz, &data);
    dim->VWrSize = (uint16)data;

    return status;
}

int dvProAV_SclIpuDimDetGet(SclEntity eEntity, SclIpuDim *dim)
{
    int status = rcSUCCESS;
    if(eEntity >= eSclEntity_Max)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }
    uint32 data;

    status &= dvProAV_AccessRead((eEntity == eSclEntity_Main) ? eIpmDetHSt : eIpsDetHSt, &data);
    dim->HSmpSt = (uint16)data;
    status &= dvProAV_AccessRead((eEntity == eSclEntity_Main) ? eIpmDetHSize : eIpsDetHSize, &data);
    dim->HSmpSize = (uint16)data;
    status &= dvProAV_AccessRead((eEntity == eSclEntity_Main) ? eIpmDetHTot : eIpsDetHTot, &data);
    dim->HWrSize = (uint16)data;
    status &= dvProAV_AccessRead((eEntity == eSclEntity_Main) ? eIpmDetVSt : eIpsDetVSt, &data);
    dim->VSmpSt = (uint16)data;
    status &= dvProAV_AccessRead((eEntity == eSclEntity_Main) ? eIpmDetVSize : eIpsDetVSize, &data);
    dim->VSmpSize = (uint16)data;
    status &= dvProAV_AccessRead((eEntity == eSclEntity_Main) ? eIpmDetVTot : eIpsDetVTot, &data);
    dim->VWrSize = (uint16)data;

    return status;
}

/**
 * @brief 設定IPU/OPU參數同步更新
 * @param [in] eEntity : 通道選擇
 *              -eSclEntity_Main
 *              -eSclEntity_Sub
 * @param [in] bUpdatingEn : 設定參數
 *              -false : 參數設定完成, 同步參數至IPU
 *              -true : 參數設定中, 先不同步參數至IPU
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclUpdatingSet(SclEntity eEntity, bool bUpdatingEn)
{
    if(eEntity >= eSclEntity_Max)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }

    // for backup switch
    if((dvProAV_SclDatapathGet() == eSclPath_BackupSw) && (eEntity == eSclEntity_Main))
    {
        dvProAV_AccessWrite((eEntity == eSclEntity_Main) ? eIpsUpdating : eIpmUpdating, bUpdatingEn);
    }
    return dvProAV_AccessWrite((eEntity == eSclEntity_Main) ? eIpmUpdating : eIpsUpdating, bUpdatingEn);
}

/**
 * @brief 設定OPU相關參數
 * @param [in] eEntity : 通道選擇
 *              -eSclEntity_Main
 *              -eSclEntity_Sub
 * @param [in] dim : Opu相關參數
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclOpuDimSet(SclEntity eEntity, SclOpuDim dim)
{
    int status = rcSUCCESS;
    if(eEntity >= eSclEntity_Max)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }

    dim.HDspSt += (dim.HDspSt % 2)? 1:0;
    dim.HSize -= (dim.HSize % 2)? 1:0;

    status &= dvProAV_AccessWrite((eEntity == eSclEntity_Main) ? eOpmHDspSt : eOpsHDspSt, (uint32)dim.HDspSt);
    status &= dvProAV_AccessWrite((eEntity == eSclEntity_Main) ? eOpmHRdSz : eOpsHRdSz, (uint32)dim.HRdSize);
    status &= dvProAV_AccessWrite((eEntity == eSclEntity_Main) ? eOpmHDspSize : eOpsHDspSize, (uint32)dim.HSize);
    status &= dvProAV_AccessWrite((eEntity == eSclEntity_Main) ? eOpmVDspSt : eOpsVDspSt, (uint32)dim.VDspSt);
    status &= dvProAV_AccessWrite((eEntity == eSclEntity_Main) ? eOpmVDspSize : eOpsVDspSize, (uint32)dim.VSize);
    status &= dvProAV_AccessWrite((eEntity == eSclEntity_Main) ? eOpmVRdSz : eOpsVRdSz, (uint32)dim.VRdSize);

    // for backup switch
    if((dvProAV_SclDatapathGet() == eSclPath_BackupSw) && (eEntity == eSclEntity_Main))
    {
        //UINT8 cCurrentSource = 0;
        //status &= (UINT8)dvProAV_SclBackupSourceGet(&cCurrentSource);

        //if(cCurrentSource)
        {
            status &= dvProAV_AccessWrite((eEntity == eSclEntity_Main) ? eOpsHDspSt : eOpmHDspSt, (uint32)dim.HDspSt);
            status &= dvProAV_AccessWrite((eEntity == eSclEntity_Main) ? eOpsHRdSz : eOpmHRdSz , (uint32)dim.HRdSize);
            status &= dvProAV_AccessWrite((eEntity == eSclEntity_Main) ? eOpsHDspSize : eOpmHDspSize , (uint32)dim.HSize);
            status &= dvProAV_AccessWrite((eEntity == eSclEntity_Main) ? eOpsVDspSt : eOpmVDspSt , (uint32)dim.VDspSt);
            status &= dvProAV_AccessWrite((eEntity == eSclEntity_Main) ? eOpsVDspSize : eOpmVDspSize , (uint32)dim.VSize);
            status &= dvProAV_AccessWrite((eEntity == eSclEntity_Main) ? eOpsVRdSz : eOpmVRdSz , (uint32)dim.VRdSize);
        }
    }
    return status;
}

/**
 * @brief 讀取OPU相關參數
 * @param [in] eEntity : 通道選擇
 *              -eSclEntity_Main
 *              -eSclEntity_Sub
 * @param [out] dim : Opu相關參數
 *              -HDspSt     //Opu horizontal display start
 *              -HRdCnt     //Opu horizontal read dot count
 *              -VDspSt     //Opu vertical display start
 *              -VRdCnt     //Opu vertical read line count
 *              -RdBAdr     //Opu DRAM base address
 *              -HSize      //Opu horizontal display size
 *              -VSize      //Opu vertical display size
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclOpuDimGet(SclEntity eEntity, SclOpuDim *dim)
{
    int status = rcSUCCESS;
    if(eEntity >= eSclEntity_Max)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }
    uint32 data;

    status &= dvProAV_AccessRead((eEntity == eSclEntity_Main) ?eOpmHDspSt : eOpsHDspSt, &data);
    dim->HDspSt = (uint16)data;
    status &= dvProAV_AccessRead((eEntity == eSclEntity_Main) ?eOpmHRdSz : eOpsHRdSz, &data);
    dim->HRdSize = (uint16)data;
    status &= dvProAV_AccessRead((eEntity == eSclEntity_Main) ?eOpmHDspSize : eOpsHDspSize, &data);
    dim->HSize = (uint16)data;
    status &= dvProAV_AccessRead((eEntity == eSclEntity_Main) ?eOpmVDspSt : eOpsVDspSt, &data);
    dim->VDspSt = (uint16)data;
    status &= dvProAV_AccessRead((eEntity == eSclEntity_Main) ?eOpmVDspSize : eOpsVDspSize, &data);
    dim->VSize = (uint16)data;
    status &= dvProAV_AccessRead((eEntity == eSclEntity_Main) ?eOpmVRdSz : eOpsVRdSz, &data);
    dim->VRdSize = (uint16)data;

    return status;
}

/**
 * @brief 設定Vop Timing
 * @param [in] Vop : Vop Timing相關參數
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclVopTmgSet(SclVop Vop)
{
    int status = rcSUCCESS;

    status &= dvProAV_AccessWrite(eVopHTot,    Vop.Hs.Total);
    status &= dvProAV_AccessWrite(eVopHsWidth, Vop.Hs.Sync);
    status &= dvProAV_AccessWrite(eVopHDspSt,  Vop.Hs.Start);
    status &= dvProAV_AccessWrite(eVopHSize,   Vop.Hs.Size);
    status &= dvProAV_AccessWrite(eVopVTot,    Vop.Vs.Total);
    status &= dvProAV_AccessWrite(eVopVsWidth, Vop.Vs.Sync);
    status &= dvProAV_AccessWrite(eVopVDspSt,  Vop.Vs.Start);
    status &= dvProAV_AccessWrite(eVopVSize,   Vop.Vs.Size);

    return status;
}

/**
 * @brief 讀取Vop Timing
 * @param [out] Vop : Vop Timing相關參數
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclVopTmgGet(SclVop *Vop)
{
    int status = rcSUCCESS;
    uint32 data;

    status &= dvProAV_AccessRead(eVopHTot, &data);
    Vop->Hs.Total = (uint16)data;
    status &= dvProAV_AccessRead(eVopHsWidth, &data);
    Vop->Hs.Sync = (uint16)data;
    status &= dvProAV_AccessRead(eVopHDspSt, &data);
    Vop->Hs.Start = (uint16)data;
    status &= dvProAV_AccessRead(eVopHSize, &data);
    Vop->Hs.Size = (uint16)data;

    status &= dvProAV_AccessRead(eVopVTot, &data);
    Vop->Vs.Total = (uint16)data;
    status &= dvProAV_AccessRead(eVopVsWidth, &data);
    Vop->Vs.Sync = (uint16)data;
    status &= dvProAV_AccessRead(eVopVDspSt, &data);
    Vop->Vs.Start = (uint16)data;
    status &= dvProAV_AccessRead(eVopVSize, &data);
    Vop->Vs.Size = (uint16)data;

    return status;
}

/**
 * @brief 設定Opu display start
 * @param [in] eEntity : 通道選擇
 *              -eSclEntity_Main
 *              -eSclEntity_Sub
 * @param [in] HDspSt : VOP horizontal display start
 * @param [in] VDspSt : VOP vertical display start
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclOpuDspStSet(SclEntity eEntity, uint16 HDspSt, uint16 VDspSt)
{
    int status = rcSUCCESS;
    if(eEntity >= eSclEntity_Max)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }
    uint32 hStart = (eEntity == eSclEntity_Main) ? eOpmHDspSt : eOpsHDspSt;
    uint32 vStart = (eEntity == eSclEntity_Main) ? eOpmVDspSt : eOpsVDspSt;

    status &= dvProAV_AccessWrite(hStart, (uint32)HDspSt);
    status &= dvProAV_AccessWrite(vStart, (uint32)VDspSt);

    return status;
}

/**
 * @brief 設定PIP sub window起始位置
 * @param [in] uiHstart : 設定PIP sub window Horizontal起始位置
 * @param [in] uiVstart : 設定PIP sub window Vertical起始位置
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclPipStartPosition(uint16 uiHstart, uint16 uiVstart)
{
    int status = rcSUCCESS;

    status &= dvProAV_AccessWrite(eDcuPipHs, (uint32)uiHstart);
    status &= dvProAV_AccessWrite(eDcuPipVs, (uint32)uiVstart);

    return status;
}

/**
 * @brief Ipu Color Space set
 * @param [in] eEntity :
 *             - eSclEntity_Main
 *             - eSclEntity_Sub
 * @param [in] ucMode  color space mode
 *             - 0: Automatic (by eDcuIpmColMd or eDcuIpsColMd)
 *             - 2: Force Off
 *             - 3: Force On (Rec. 709)
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclIpuColorSpaceSet(SclEntity eEntity, uint08 ucMode)
{
    if((ucMode > 3) || (eEntity >= eSclEntity_Max))
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }

    return dvProAV_AccessWrite((eEntity == eSclEntity_Main) ? eIpmCscMode : eIpsCscMode, (uint32)ucMode);
}

/**
 * @brief Opu Color Space set
 * @param [in] eEntity :
 *             - eSclEntity_Main
 *             - eSclEntity_Sub
 * @param [in] ucMode  color space mode
 *             - 0: Automatic (by eDcuIpmColMd or eDcuIpsColMd)
 *             - 4: Force Off
 *             - 5: Rec. 601
 *             - 6: 0x06: Rec. 709
 *             - 7: Rec. 2020
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclOpuColorSpaceSet(SclEntity eEntity, uint08 ucMode)
{
    if((eEntity >= eSclEntity_Max) || ((ucMode != 0) && (ucMode != 4) && (ucMode != 5) && (ucMode != 7)))
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }

    return dvProAV_AccessWrite((eEntity == eSclEntity_Main) ? eOpmCscMode : eOpsCscMode, (uint32)ucMode);
}

/**
 * @brief 設定deinterlace mode
 * @param [in] eEntity : 通道選擇
 *              -eSclEntity_Main
 *              -eSclEntity_Sub
 * @param [in] DeIntMode : 設定deinterlace mode
 *              -diBob      //0: Bob mode
 *              -diEven     //1: Even field only
 *              -diOdd      //2: Odd field only
 *              -diWave     //3: Weave mode
 *              -diMotion   //4: Motion adaptive
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclDeIntModeSet(SclEntity eEntity, SclDeIntMode DeIntMode)
{
    uint32 index;
    if((DeIntMode > eSclDeIntMode_Motion) || (eEntity >= eSclEntity_Max))
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }

    index = (eEntity == eSclEntity_Main) ? eIpmDeIntMode : eIpsDeIntMode;

    return dvProAV_AccessWrite(index, (uint32)DeIntMode);
}

/**
 * @brief 讀取目前deinterlace mode
 * @param [in] eEntity : 通道選擇
 *              -eSclEntity_Main
 *              -eSclEntity_Sub
 * @param [out] DeIntMode : 回傳目前的deinterlace mode
 *              -diBob      //0: Bob mode
 *              -diEven     //1: Even field only
 *              -diOdd      //2: Odd field only
 *              -diWave     //3: Weave mode
 *              -diMotion   //4: Motion adaptive
 * @return  status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclDeIntModeGet(SclEntity eEntity, SclDeIntMode *DeIntMode)
{
    int status = rcSUCCESS;
    if(eEntity >= eSclEntity_Max)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }
    uint32 index, data;

    index = (eEntity == eSclEntity_Main) ? eIpmDeIntMode : eIpsDeIntMode;
    status = dvProAV_AccessRead(index, &data);
    *DeIntMode = (SclDeIntMode)data;

    return status;
}

/**
 * @brief 設定MADI threshold
 * @param [in] eEntity : 通道選擇
 *              -eSclEntity_Main
 *              -eSclEntity_Sub
 * @param [in] Threshold??
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclMADiThresholdSet(SclEntity eEntity, uint08 *Threshold)
{
    int status = rcSUCCESS;
    if(eEntity >= eSclEntity_Max)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }
    uint32 lower = (eEntity == eSclEntity_Main) ? eOpmMADiThLower : eOpsMADiThLower;
    uint32 upper = (eEntity == eSclEntity_Main) ? eOpmMADiThUpper : eOpsMADiThUpper;

    status &= dvProAV_AccessWrite(lower, (uint32)Threshold[0]);
    status &= dvProAV_AccessWrite(upper, (uint32)Threshold[1]);

    return status;
}

/**
 * @brief Input port freeze Enable / Disable
 * @param [in] eEntity : 通道選擇
 *              -eSclSrcIn_Main
 *              -eSclSrcIn_Sub
 *              -eSclSrcIn_MainSub
 * @param [in] bEnable
 *             -true : 凍結
 *             -false : 取消凍結
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclInputFreezeSet(SclSrcIn eEntity, bool bEnable)
{
    int status = rcSUCCESS;

    if(eEntity >= eSclSrcIn_Max)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }
    else
    {
        if(eEntity == eSclSrcIn_MainSub)
        {
            if(bEnable)
            {
                status &= dvProAV_AccessWrite(eIpmFrmbufferEn, (uint32)false);
                status &= dvProAV_AccessWrite(eIpsFrmbufferEn, (uint32)false);
                MS_SLEEP(100);    //R70K_Sammy_0002 modify for 3D freeze
            }
            else
            {
                status &= dvProAV_AccessWrite(eIpmFrmbufferEn, (uint32)s_acIpmFrmbufferEn[0]); //A70LK_Larry_0173
                status &= dvProAV_AccessWrite(eIpsFrmbufferEn, (uint32)s_acIpmFrmbufferEn[1]); //A70LK_Larry_0173
            }

            status &= dvProAV_AccessWrite(eIpmShutUp, (uint32)bEnable);
            status &= dvProAV_AccessWrite(eIpsShutUp, (uint32)bEnable);
        }
        else
        {
            uint32 index = (eEntity == eSclSrcIn_Main) ? eIpmShutUp : eIpsShutUp;
            if(bEnable)
            {
                status &= dvProAV_AccessWrite((eEntity == eSclEntity_Main)? eIpmFrmbufferEn : eIpsFrmbufferEn, (uint32)false);
                MS_SLEEP(100);    //R70K_Sammy_0002 modify for 3D freeze
            }
            else
            {
                status &= dvProAV_AccessWrite((eEntity == eSclEntity_Main)? eIpmFrmbufferEn : eIpsFrmbufferEn, (uint32)s_acIpmFrmbufferEn[eEntity]);
            }

            status &= dvProAV_AccessWrite(index, (uint32)bEnable);
        }
    }

    return status;
}

/**
 * @brief Input port freeze mode get
 * @param [in] eEntity : channel selection
 *              -eSclEntity_Main
 *              -eSclEntity_Sub
 * @return mode
 *             -true : freeze
 *             -false : unfreeze
 */
bool dvProAV_SclInputFreezeGet(SclEntity eEntity)
{
    int status = rcSUCCESS;
    uint32 ulFreeze;
    uint32 index = (eEntity == eSclEntity_Main) ? eIpmShutUp: eIpsShutUp ;

    status &= dvProAV_AccessRead(index, &ulFreeze);
    if(status != rcSUCCESS)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
    }

    return (bool)ulFreeze;
}

/**
 * @brief Rx channel select at Passthrough/fb-less mode??
 * @param [in] sRxPort : Rx Port
 *              -eRxPort_0
 *              -eRxPort_1
 *              -eRxPort_2
 *              -eRxPort_3
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclFBLClockSourceSet(RxPort sRxPort)
{
    if(sRxPort >= eRxPort_Max)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }

    return dvProAV_AccessWrite(eBiuRxClkSel, (uint32)sRxPort);
}

/**
 * @brief AB frame buffer mode set
 * @param [in] eEntity : channel selection
 *              -eSclEntity_Main
 *              -eSclEntity_Sub
 * @param [in] bEnable
 *              - false : Disable AB frame buffer
 *              - true : Enable AB frame buffer
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclAbBufModeSet(SclEntity eEntity, bool bEnable)
{
    if(eEntity >= eSclEntity_Max)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }

    s_acIpmFrmbufferEn[eEntity] = bEnable;

    return dvProAV_AccessWrite((eEntity == eSclEntity_Main)? eIpmFrmbufferEn : eIpsFrmbufferEn, (uint32)bEnable);
}

//-------------------------------------------------------------------------------------------------
// Gamma
//-------------------------------------------------------------------------------------------------
/**
 * @brief Download the Gamma Table  to Chip
 * @note The gamma table setting is used the Vop Clock, so the setting should after the Vop Clock setting
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclVopGammaSet(bool bSameTable)
{
    int status = rcSUCCESS;
    uint08 ColorCount = (bSameTable)? 3:1;
    uint16 GammaSteps = dvProAV_SclGammaItemCountGet();
    uint16 length = GammaSteps * 2 * ColorCount;
    uint08 *buf = (uint08 *)malloc(length);

    if (buf == 0)
    {
        DBMSG("dvProAV_SclVopGammaSet Memory Size insufficient");
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcERROR;
    }

    status &= dvProAV_UmbRgbTableTypeSet(bSameTable);   // set RGB use the same/different table

    for(uint08 c=0; c<ColorCount; c++)
    {
        uint16 *table = dvProAV_SclGammaTableGet(c);
        for(uint16 i=0; i<GammaSteps; i++)
        {
            buf[(c * GammaSteps) + i * 2 + 0] = table[i] & 0xff;
            buf[(c * GammaSteps) + i * 2 + 1] = table[i] >> 8;
        }
    }
    status &= dvProAV_UmbGammaWrite(eUmbFunc_SpiToGamma, 0, eUmbGammaType_VopGamma, length, buf);

    free(buf);

    return status;
}

/**
 * @brief Download the dram data to gamma buffer(sram)
 * @param dramAddr DRAM address of gamma data
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclVopGammaFromDram(uint32 dramAddr)
{
    int status = rcSUCCESS;

    status &= dvProAV_AccessWrite(eVopGamaAdr, dramAddr >> 1);
    status &= dvProAV_AccessWrite(eVopDram2Gama, 1); // 1-> 0, trigger dram data to gamma buffer(sram)
    status &= dvProAV_AccessWrite(eVopDram2Gama, 0);

    return rcSUCCESS;
}

/**
 * @brief Download the dram data to gamma buffer(sram)
 * @param dramAddr DRAM address of gamma data
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
/*int dvProAV_SclVopGammaFlash2Dram(void)
{
    int status = rcSUCCESS;
    uint32 gammaByteCount = dvProAV_SclGammaItemCountGet() * 2; // use 2 byte for each Gamma Item
    status = dvProAV_SfiDram2Flash(DRAM_GAMMA_ADDR * PROAV_DRAM_LINE_BYTES, FLASH_VOPGMA0_ADDR, gammaByteCount, false);
    return status;
}*/

//-------------------------------------------------------------------------------------------------
// Bias / Gain
//-------------------------------------------------------------------------------------------------
/**
 * @brief Scaler Vip Bias set
 * @param [in] eEntity : 通道選擇
 *              -eSclEntity_Main
 *              -eSclEntity_Sub
 * @param [in] eColor : 顏色選擇
 *              -eSclColorR
 *              -eSclColorG
 *              -eSclColorB
 * @param [in] cBias : -128 ~ 127
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclVipBiasSet(SclEntity eEntity, SclColor eColor, int08 cBias)
{
    if((eEntity >= eSclEntity_Max) || (eColor >= eSclColorMax))
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }
    uint32 ulIndex = (eEntity == eSclEntity_Main) ? eIpmBiasR : eIpsBiasR;
    uint32 ulOffSet = eColor * (eIpmBiasG - eIpmBiasR);

    return dvProAV_AccessWrite((uint32)(ulIndex + ulOffSet), (uint32)cBias);
}

/**
 * @brief Scaler Vip Gain set
 * @param [in] eEntity : 通道選擇
 *              -eSclEntity_Main
 *              -eSclEntity_Sub
 * @param [in] Color : 顏色選擇
 *              -eSclColorR
 *              -eSclColorG
 *              -eSclColorB
 * @param [in] Gain : 0 ~ 255
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclVipGainSet(SclEntity eEntity, SclColor eColor, uint08 ucGain)
{
    if((eEntity >= eSclEntity_Max) || (eColor >= eSclColorMax))
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }
    uint32 ulIndex = (eEntity == eSclEntity_Main) ? eIpmGainR : eIpsGainR;
    uint32 ulOffSet = eColor * (eIpmGainG - eIpmGainR);

    return dvProAV_AccessWrite((uint32)(ulIndex + ulOffSet), (uint32)ucGain);
}

/**
 * @brief Scaler Vop Bias set
 * @param [in] eEntity : 通道選擇
 *              -eSclEntity_Main
 *              -eSclEntity_Sub
 * @param [in] Color : 顏色選擇
 *              -eSclColorR
 *              -eSclColorG
 *              -eSclColorB
 * @param [in] Bias : -128 ~ 127
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclVopBiasSet(SclEntity eEntity, SclColor eColor, int08 cBias)
{
    if((eEntity >= eSclEntity_Max) || (eColor >= eSclColorMax))
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }
    uint32 ulIndex = (eEntity == eSclEntity_Main) ? eVopmBkeBiasR : eVopsBkeBiasR;
    uint32 ulOffSet = eColor * (eVopmBkeBiasG - eVopmBkeBiasR);

    return dvProAV_AccessWrite((uint32)(ulIndex + ulOffSet), (uint32)cBias);
}

/**
 * @brief Scaler Vop Gain set
 * @param [in] eEntity : 通道選擇
 *              -eSclEntity_Main
 *              -eSclEntity_Sub
 * @param [in] Color : 顏色選擇
 *              -eSclColorR
 *              -eSclColorG
 *              -eSclColorB
 * @param [in] Gain : 0 ~ 255
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclVopGainSet(SclEntity eEntity, SclColor eColor, uint08 ucGain)
{
    if((eEntity >= eSclEntity_Max) || (eColor >= eSclColorMax))
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }
    uint32 ulIndex = (eEntity == eSclEntity_Main) ? eVopmBkeGainR : eVopsBkeGainR;
    uint32 ulOffSet = eColor * (eVopmBkeGainG - eVopmBkeGainR);

    return dvProAV_AccessWrite((uint32)(ulIndex + ulOffSet), (uint32)ucGain);
}

/**
 * @brief Disable VOP Backgain Sub Window
 * @param [in] bEnable : false / true = enable sub backgain / disable sub backgain
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclVopGainBiasSubOffSet(bool bEnable)
{
    return dvProAV_AccessWrite(eVopmBkesub_off_E4k, bEnable);
}

/**
 * @brief VOP Backgain Contrast Set
 * @param [in] eEntity : 通道選擇
 *              -eSclEntity_Main
 *              -eSclEntity_Sub
 * @param [in] uiContrast : 0~1023: (0 ~ 1023) / 512
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclVopBkeContrastSet(SclEntity eEntity, uint16 uiContrast)
{
    if((eEntity >= eSclEntity_Max) || (uiContrast > 1023))
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }

    return dvProAV_AccessWrite((eEntity == eSclEntity_Main) ? eVopmBkeContrast : eVopsBkeContrast, (uint32)uiContrast);
}

/**
 * @brief VOP Backgain Brightness Set
 * @param [in] eEntity : 通道選擇
 *              -eSclEntity_Main
 *              -eSclEntity_Sub
 * @param [in] uiBrightness 0~1023: -512 ~ 511
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclVopBkeBrightnessSet(SclEntity eEntity, uint16 uiBrightness)
{
    if((eEntity >= eSclEntity_Max) || (uiBrightness > 1023))
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }

    return dvProAV_AccessWrite((eEntity == eSclEntity_Main) ? eVopmBkeBrightness : eVopsBkeBrightness, (uint32)uiBrightness);
}

//-------------------------------------------------------------------------------------------------
// Scaling mode - Lanczos / Bilinear
//-------------------------------------------------------------------------------------------------
/**
 * @brief Download Vip Coeff Table to chip
 * @param [in] eEntity : 通道選擇
 *              -eSclEntity_Main
 *              -eSclEntity_Sub
 * @param [in] Sel :
 *              -DlTblOff   = 0x00, // 0000, S-B-V-H
 *              -DlTblH_S   = 0x09, // 1001, S-B-V-H, Download to table Horz. Sharp (for scaling up)
 *              -DlTblV_S   = 0x0A, // 1010, S-B-V-H, Download to table Vert. Sharp (for scaling up)
 *              -DlTblHnV_S = 0x0B, // 1011, S-B-V-H, Download to table Horz. & Vert. Sharp (for scaling up)
 *              -DlTblH_B   = 0x05, // 0101, S-B-V-H, Download to table Horz. Blur (for 1/2 scaling down)
 *              -DlTblV_B   = 0x06, // 0110, S-B-V-H, Download to table Vert. Blur (for 1/2 scaling down)
 *              -DlTblHnV_B = 0x07, // 0111, S-B-V-H, Download to table Horz. & Vert Blur (for 1/2 scaling down), vop not support "H scaling down <= 1/2" ...
 * @param [in] Coeff : Vip Coeff Table
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
static int dvProAV_SclVipCoeffSet(SclEntity eEntity, SclDlSel Sel, const uint16 *Coeff)
{
    int status = rcSUCCESS;
    if((eEntity >= eSclEntity_Max) || ((Sel != eSclDlSel_Off) && (Sel != eSclDlSel_HSharp) && (Sel != eSclDlSel_VSharp) && (Sel != eSclDlSel_HVSharp) &&
                                       (Sel != eSclDlSel_HBlur) && (Sel != eSclDlSel_VBlur) && (Sel != eSclDlSel_HVBlur)))
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }
    uint16 CoeffBlockSize = dvProAV_SclCoeffTableBlockSizeGet();
    uint16 length = CoeffBlockSize * 2;
    uint08 *buf; // uint08[length];
    uint32 ScBufWr = (eEntity == eSclEntity_Main) ? eIpmScBufWr : eIpsScBufWr;
    uint32 BufPort = (eEntity == eSclEntity_Main) ? eIpmScBufPort : eIpsScBufPort;
    uint32 DlTbl = (eEntity == eSclEntity_Main) ? eIpmDlTbl : eIpsDlTbl;

    buf = (uint08 *)malloc(length);

    if (buf == 0)
    {
        DBMSG("dvProAV_SclVipCoeffSet Memory Size insufficient");
        status = rcERROR;
    }

    if (status != rcSUCCESS)
    {
        free(buf);
        return status;
    }
    // VIP scaling buffer(Lanczos) config Write the value into buffer enable??
    status &= dvProAV_AccessWrite(ScBufWr, (uint32)true);
    // Write Coeff Table to VIP scaling buffer data port
    for(uint16 i=0; i<CoeffBlockSize; i++)
    {
        buf[i*2 + 0] = Coeff[i] & 0xff;
        buf[i*2 + 1] = Coeff[i] >> 8;
    }
#ifdef QT_CPP
    status &= dvProAV_AccessBurstWrite(BufPort, buf, length, BURST_FIX_ADDR); // Fixed Address if last parameter is false
#else
    for(uint16 uiCount = 0; uiCount < length; uiCount++)
    {
        status &= dvProAV_AccessWrite(BufPort, buf[uiCount]);
    }
#endif

    free(buf);

    // VIP scaling buffer(Lanczos) config Write the value into buffer disable??
    status &= dvProAV_AccessWrite(ScBufWr, (uint32)false); // set to read state to avoid the others' write
    // 設定將VIP scaling buffer data port中資料寫入的位置
    status &= dvProAV_AccessWrite(DlTbl, (uint32)Sel);
    // 更新完, Coeff Table write disable
    DelayMSec(TABLE_UPDATE_DELAY);
    status &= dvProAV_AccessWrite(DlTbl, (uint32)eSclDlSel_Off);

    return status;
}

/**
 * @brief Download Vop Coeff Table to chip
 * @param [in] eEntity : 通道選擇
 *          -eSclEntity_Main
 *          -eSclEntity_Sub
 * @param [in] Sel :
 *          -eSclDlSel_Off      = 0x00, // 0000, S-B-V-H
 *          -eSclDlSel_HSharp   = 0x09, // 1001, S-B-V-H, Download to table Horz. Sharp (for scaling up)
 *          -eSclDlSel_VSharp   = 0x0A, // 1010, S-B-V-H, Download to table Vert. Sharp (for scaling up)
 *          -eSclDlSel_HVSharp  = 0x0B, // 1011, S-B-V-H, Download to table Horz. & Vert. Sharp (for scaling up)
 *          -eSclDlSel_HBlur    = 0x05, // 0101, S-B-V-H, Download to table Horz. Blur (for 1/2 scaling down)
 *          -eSclDlSel_VBlur    = 0x06, // 0110, S-B-V-H, Download to table Vert. Blur (for 1/2 scaling down)
 *          -eSclDlSel_HVBlur   = 0x07, // 0111, S-B-V-H, Download to table Horz. & Vert Blur (for 1/2 scaling down), vop not support "H scaling down <= 1/2" ...??
 * @param [in] Coeff : Vop Coeff Table
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
static int dvProAV_SclVopCoeffSet(SclEntity eEntity, SclDlSel Sel, const uint16 *Coeff)
{
    int status = rcSUCCESS;
    if((eEntity >= eSclEntity_Max) || ((Sel != eSclDlSel_Off) && (Sel != eSclDlSel_HSharp) && (Sel != eSclDlSel_VSharp) && (Sel != eSclDlSel_HVSharp) &&
                                       (Sel != eSclDlSel_HBlur) && (Sel != eSclDlSel_VBlur) && (Sel != eSclDlSel_HVBlur)))
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }
    uint16 CoeffBlockSize = dvProAV_SclCoeffTableBlockSizeGet();
    uint16 length = CoeffBlockSize * 2;
    uint08 *buf;
    uint32 ScBufWr = (eEntity == eSclEntity_Main) ? eOpmScBufWr : eOpsScBufWr;
    uint32 BufPort = (eEntity == eSclEntity_Main) ? eOpmScBufPort : eOpsScBufPort;
    uint32 DlTbl = (eEntity == eSclEntity_Main) ? eOpmDlTbl : eOpsDlTbl;

    buf = (uint08 *)malloc(length);

    if (buf == 0)
    {
        DBMSG("dvProAV_SclVopCoeffSet Memory Size insufficient");
        status = rcERROR;
    }

    if (status != rcSUCCESS)
    {
        free(buf);
        return status;
    }

    // VOP scaling buffer(Lanczos) config Write the value into buffer enable
    status &= dvProAV_AccessWrite(ScBufWr, (uint32)true);

    // Write Coeff Table to VOP scaling buffer data port
    for(uint16 i=0; i < CoeffBlockSize; i++)
    {
        buf[i*2 + 0] = Coeff[i] & 0xff;
        buf[i*2 + 1] = Coeff[i] >> 8;
    }

#ifdef QT_CPP
    status &= dvProAV_AccessBurstWrite(BufPort, buf, length, BURST_FIX_ADDR); // Fixed Address if last parameter is false
#else
    for(uint16 uiCount = 0; uiCount < length; uiCount++)
    {
        status &= dvProAV_AccessWrite(BufPort, buf[uiCount]);
    }
#endif

    free(buf);

    // VOP scaling buffer(Lanczos) config Write the value into buffer disable
    status &= dvProAV_AccessWrite(ScBufWr, (uint32)false);
    // 設定將VOP scaling buffer data port中資料寫入的位置
    status &= dvProAV_AccessWrite(DlTbl, (uint32)Sel);
    // 更新完, Coeff Table write disable
    DelayMSec(TABLE_UPDATE_DELAY);
    status &= dvProAV_AccessWrite(DlTbl , (uint32)eSclDlSel_Off);

    return status;
}

/**
 * @brief 下載(dvProAV_SclCoeffTable.c)表列的Scaling 係數到晶片上
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclCoeffTableDl2Chip(void)
{
    int status = rcSUCCESS;
    uint16 CoeffBlockSize = dvProAV_SclCoeffTableBlockSizeGet();
    const uint16 *coeff = dvProAV_SclCoeffTableGet();

    for (int i = 0; i < (int)eSclEntity_Max; i++)
    {
        status &= dvProAV_SclVipCoeffSet((SclEntity)i, eSclDlSel_HVSharp, &coeff[CoeffBlockSize * eScalingTap_BlurA]);
        status &= dvProAV_SclVipCoeffSet((SclEntity)i, eSclDlSel_HVBlur, &coeff[CoeffBlockSize * eScalingTap_BlurB]);
        status &= dvProAV_SclVopCoeffSet((SclEntity)i, eSclDlSel_HVSharp, &coeff[CoeffBlockSize * eScalingTap_SharpZero]);
        status &= dvProAV_SclVopCoeffSet((SclEntity)i, eSclDlSel_VBlur, &coeff[CoeffBlockSize * eScalingTap_BlurZero]);  // vop not support "H scaling down <= 1/2" ...
    }

    return status;
}

/**
 * @brief 設定VIP horizontal scaling mode
 * @param [in] eEntity : 通道選擇
 *              -eSclEntity_Main
 *              -eSclEntity_Sub
 * @param [in] Alg : 設定horizontal scaling mode
 *              -saLanczos
 *              -saBilinear
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclVipHsAlgSet(SclEntity eEntity, SclAlg Alg)
{
    uint32 index;
    if((eEntity >= eSclEntity_Max) || (Alg > eSclAlg_Bilinear))
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }

    index = (eEntity == eSclEntity_Main) ? eIpmHScMode : eIpsHScMode;

    return dvProAV_AccessWrite(index, (uint32)Alg);
}

/**
 * @brief 讀取VIP horizontal scaling mode
 * @param [in] eEntity : 通道選擇
 *              -eSclEntity_Main
 *              -eSclEntity_Sub
 * @param [out] Alg : 回傳horizontal scaling mode
 *              -saLanczos
 *              -saBilinear
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclVipHsAlgGet(SclEntity eEntity, SclAlg *Alg)
{
    int status = rcSUCCESS;
    if (eEntity >= eSclEntity_Max)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }
    uint32 index = (eEntity == eSclEntity_Main) ? eIpmHScMode : eIpsHScMode;
    uint32 data;

    status = dvProAV_AccessRead(index, &data);
    *Alg = (SclAlg)data;

    return status;
}

/**
 * @brief 設定VIP vertical scaling mode
 * @param [in] eEntity : 通道選擇
 *              -eSclEntity_Main
 *              -eSclEntity_Sub
 * @param [in] Alg : 設定vertical scaling mode
 *              -saLanczos
 *              -saBilinear
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclVipVsAlgSet(SclEntity eEntity, SclAlg Alg)
{
    if((eEntity >= eSclEntity_Max) || (Alg > eSclAlg_Bilinear))
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }
    uint32 index = (eEntity == eSclEntity_Main) ? eIpmVScMode : eIpsVScMode;

    return dvProAV_AccessWrite(index, (uint32)Alg);
}

/**
 * @brief 讀取VIP vertical scaling mode
 * @param [in] eEntity : 通道選擇
 *              -eSclEntity_Main
 *              -eSclEntity_Sub
 * @param [out] Alg : 讀取vertical scaling mode
 *              -saLanczos
 *              -saBilinear
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclVipVsAlgGet(SclEntity eEntity, SclAlg *Alg)
{
    int status = rcSUCCESS;
    if (eEntity >= eSclEntity_Max)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }
    uint32 index = (eEntity == eSclEntity_Main) ? eIpmVScMode : eIpsVScMode;
    uint32 data;

    status = dvProAV_AccessRead(index, &data);
    *Alg = (SclAlg)data;

    return status;
}

/**
 * @brief 設定Opu horizontal scaling mode
 * @param [in] eEntity : 通道選擇
 *              -eSclEntity_Main
 *              -eSclEntity_Sub
 * @param [in] Alg : 設定Opu horizontal scaling mode
 *              -saLanczos
 *              -saBilinear
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclOpuHsAlgSet(SclEntity eEntity, SclAlg Alg)
{
    if((eEntity >= eSclEntity_Max) || (Alg > eSclAlg_Bilinear))
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }

    uint32 index = (eEntity == eSclEntity_Main) ? eOpmHScMode : eOpsHScMode;

    return dvProAV_AccessWrite(index, (uint32)Alg);
}

/**
 * @brief 讀取Opu horizontal scaling mode
 * @param [in] eEntity : 通道選擇
 *              -eSclEntity_Main
 *              -eSclEntity_Sub
 * @param [out] Alg : 讀取Opu horizontal scaling mode
 *              -saLanczos
 *              -saBilinear
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclOpuHsAlgGet(SclEntity eEntity, SclAlg *Alg)
{
    int status = rcSUCCESS;
    if (eEntity >= eSclEntity_Max)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }
    uint32 index = (eEntity == eSclEntity_Main) ? eOpmHScMode : eOpsHScMode;
    uint32 data;

    status = dvProAV_AccessRead(index, &data);
    *Alg = (SclAlg)data;

    return status;
}

/**
 * @brief 讀取Opu vertical scaling mode
 * @param [in] eEntity : 通道選擇
 *              -eSclEntity_Main
 *              -eSclEntity_Sub
 * @param [in] Alg : 設定Opu vertical scaling mode
 *              -saLanczos
 *              -saBilinear
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclOpuVsAlgSet(SclEntity eEntity, SclAlg Alg)
{
    if((eEntity >= eSclEntity_Max) || (Alg > eSclAlg_Bilinear))
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }

    uint32 index = (eEntity == eSclEntity_Main) ? eOpmVScMode : eOpsVScMode;
    return dvProAV_AccessWrite(index, (uint32)Alg);
}

/**
 * @brief 讀取Opu vertical scaling mode
 * @param [in] eEntity : 通道選擇
 *          -eSclEntity_Main
 *          -eSclEntity_Sub
 * @param [out] Alg : 讀取Opu vertical scaling mode
 *          -saLanczos
 *          -saBilinear
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclOpuVsAlgGet(SclEntity eEntity, SclAlg *Alg)
{
    int status = rcSUCCESS;
    if(eEntity >= eSclEntity_Max)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }
    uint32 index = (eEntity == eSclEntity_Main) ? eOpmVScMode : eOpsVScMode;
    uint32 data;

    status = dvProAV_AccessRead(index, &data);
    *Alg = (SclAlg)data;

    return status;
}

//-------------------------------------------------------------------------------------------------
// Frame Sync Function
//-------------------------------------------------------------------------------------------------
/**
 * @brief 設定Frame Sync mode
 * @param [in] eEntity : Frame sync source select
 *              -eSclEntity_Main : Ipm
 *              -SclEntity_CH2 : Ips
 * @param [in] sRxPort : PLL source select
 *              -eRxPort_0 : Rx0
 *              -eRxPort_1 : Rx1
 *              -eRxPort_2 : Rx2
 *              -eRxPort_3 : Rx3
 * @param [in] eMode : Frame Sync mode
 *              -eSyncMode_Off
 *              -eSyncMode_HSyncFreeRun
 *              -eSyncMode_VSyncReset
 * @param [in] ucTimes : Times of VSync frequency
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclFrameSyncModeSet(SclEntity eEntity, RxPort sRxPort, SclFrameSyncMode eMode, uint8 ucTimes)
{
    int status = rcSUCCESS;
    if((eEntity >= eSclEntity_Max) || (sRxPort >= eRxPort_Max) || (eMode >= eSclSyncMode_Max))
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }

    if((eMode == eSclSyncMode_Off || ucTimes != 0) && (ucTimes != 0xFF))
    {
        //60hz out, 30hz in
        //120hz out, 60/30hz in
        status &= dvProAV_SclAbBufModeSet(eSclEntity_Main, true);
        status &= dvProAV_SclAbBufModeSet(eSclEntity_Sub, true);
    }
    else
    {
        //60hz out, 60hz in
        //120hz out, 120hz in
        status &= dvProAV_SclAbBufModeSet(eSclEntity_Main, false);
        status &= dvProAV_SclAbBufModeSet(eSclEntity_Sub, false);
    }

    // set PLL source clock form Rx
    status &= dvProAV_SclExtPllSourceSel(sRxPort);
    // Set Frame Sync mode
    status &= dvProAV_AccessWrite(eVopFSSel, eEntity);
    status &= dvProAV_AccessWrite(eVopFSMd, eMode);

    //check VCXO
    if((status == rcSUCCESS) && (eMode != eSclSyncMode_Off))
    {
        status = rcERROR;
        uint8 retry = 5;

        do
        {
            uint32 vcxo_clk = 0;

            MS_SLEEP(50);

            if(rcSUCCESS == dvProAV_AccessRead(eClkFreqVcxo0, &vcxo_clk))
            {
                if(vcxo_clk >= (HDMI_REF_CLK_6000 - 100))
                {
                    status = rcSUCCESS;
                }
            }
            //printf("vcxo_clk = %d\n", vcxo_clk);
        }while((status == rcERROR) && (retry--));
    }

    return status;
}

int dvProAV_SclFrameSyncModeGet(uint08* cMode)
{
    int status = rcSUCCESS;
    uint32 data = 0;

    status = dvProAV_AccessRead(eVopFSMd, &data);
    *cMode = (uint08)data;

    return status;
}

/**
 * @brief Frame Sync mode delay set
 * @param [in] ucDelay :
 *              - 0: Delay 12 input lines
 *              - 1-255: Delay 1/256 ~ 255/256 input frame
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclFrameSyncDelaySet(uint8 ucDelay)
{
    return dvProAV_AccessWrite(eInBrgFsDlyMd, ucDelay);
}

/**
 * @brief External PLL clock source selection
 * @param [in] ucSource :
 *              -eRxPort_0 : Rx0
 *              -eRxPort_1 : Rx1
 *              -eRxPort_2 : Rx2
 *              -eRxPort_3 : Rx3
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclExtPllSourceSel(RxPort sRxPort)
{
    if(sRxPort >= eRxPort_Max)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }
    return dvProAV_AccessWrite(eBiuHdClkSel, sRxPort);
}

/**
 * @brief External PLL clock source get
 * @param [0ut] ucSource :
 *              -eRxPort_0 : Rx0
 *              -eRxPort_1 : Rx1
 *              -eRxPort_2 : Rx2
 *              -eRxPort_3 : Rx3
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclExtPllSourceGet(RxPort *ucSource)
{
    int status = rcSUCCESS;
    uint32 data;

    status = dvProAV_AccessRead(eBiuHdClkSel, &data);
    *ucSource = (RxPort)data;

    return status;
}

/**
 * @brief dvProAV_SclFrameSyncUpdating
 * @param [in] bUpdating :
 *          - false : Updating done
 *          - true : Updating start
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclFrameSyncUpdating(bool bUpdating)
{
    int status = rcSUCCESS;

    if(bUpdating)
    {
        status &= dvProAV_AccessWrite(eBiuVidoMask, true);
        status &= dvProAV_AccessWrite(eVopFsSwIng, true);
    }
    else
    {
        status &= dvProAV_AccessWrite(eBiuVidoMask, false);
        status &= dvProAV_AccessWrite(eVopFsSwIng, false);
    }
    return status;
}

//-------------------------------------------------------------------------------------------------
// MEMC
//-------------------------------------------------------------------------------------------------
/**
 * @brief Memc Bypass Set, for External MEMC mode selection
 * @param bEnable :
 *          - false : Enable MEMC
 *          - true : Bypass MEMC
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclMemcBypassSet(bool bEnable)
{
#ifdef QT_CPP
    return dvProAV_AccessWrite(eBiuPassMemc, bEnable);
#endif /* 0 */
}

/**
 * @brief Memc Status Get
 * @return status : 0=MEMC unstable, 1=MEMC stable
 */
bool dvProAV_SclMemcStatusGet(void)
{
    int status = rcSUCCESS;
    uint32 ulMemcMode =0;
    uint8 ucStatus1 = 0, ucStatus2 = 0;

    status &= dvProAV_AccessRead(eBiuPassMemc, &ulMemcMode);
    status &= dvProAV_AccessRawDataReadByte(0x00, 0x19, &ucStatus1);
    status &= ((ucStatus1 & 0x13) == 0x13)? rcSUCCESS : rcERROR;
    if((ulMemcMode & 0x01) == true) // if memc mode
    {
        status &= dvProAV_AccessRawDataReadByte(0x00, 0x18, &ucStatus2);
        status &= ((ucStatus2 & 0xfe) == 0xfe)? rcSUCCESS : rcERROR;
    }
    return (bool)status;
}

//-------------------------------------------------------------------------------------------------
// Anti-Smear
//-------------------------------------------------------------------------------------------------
/**
 * @brief Anti-smear blank Enable
 * @param [in] bEnable :
 *              - false: Disable
 *              - true: Enable
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclAtsBlankEn(bool bEnable)
{
    return dvProAV_AccessWrite(eVopBlkEn, bEnable);
}

/**
 * @brief Anti-smear blank index polarity control
 * @param [in] bHighActive :
 *              - false: low active
 *              - true: high active
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclAtsBlankPolSet(bool bHighActive)
{
    return dvProAV_AccessWrite(eVopBlkPol, bHighActive);
}

/**
 * @brief Anti-smear blank mode selection
 * @param [in] ucMode :
 *              -8 : 4'b1000: 1x frame rate, 1/2 blank
 *              -0 : 4'b0000: 2x frame rate, 1/2 blank
 *              -1 : 4'b0001: 8x frame rate, 1/8 blank
 *              -2 : 4'b0010: 8x frame rate, 2/8 blank
 *              -3 : 4'b0011: 8x frame rate, 3/8 blank
 *              -4 : 4'b0100: 8x frame rate, 4/8 blank
 *              -5 : 4'b0101: 8x frame rate, 5/8 blank
 *              -6 : 4'b0110: 8x frame rate, 6/8 blank
 *              -7 : 4'b0111: 8x frame rate, 7/8 blank
 *              -10 : 4'b1010: 4x frame rate, 1/4 blank
 *              -12 : 4'b1100: 4x frame rate, 2/4 blank
 *              -14 : 4'b1110: 4x frame rate, 3/4 blank
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclAtsBlankmode(uint8 ucMode)
{
    if(((ucMode > 8) && (ucMode % 2 != 0)) || (ucMode > 14))
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }

    return dvProAV_AccessWrite(eVopBlkMd, ucMode);
}

/**
 * @brief Anti-smear Blank Index delay mode
 * @param [in] bDelayH :
 *                  - 0 : Delay 0 output frame
 *                  - 1 : Delay 1 output frame
 * @param [in] ucDelayL : ucDelayL / 256 output frame, total delay = bDelayH + ucDelayL
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclAtsBlankDelaySet(bool bDelayH, uint8 ucDelayL)
{
    int status = rcSUCCESS;

    status &= dvProAV_AccessWrite(eVopBkDlyMdL, ucDelayL);
    status &= dvProAV_AccessWrite(eVopBkDlyMdH, bDelayH);

    return status;
}

//-------------------------------------------------------------------------------------------------
// Screen Off
//-------------------------------------------------------------------------------------------------
/**
 * @brief 設定Opu Screen Off Enable / Disable
 * @param [in] eEntity : 通道選擇
 *              -eSclEntity_Main
 *              -eSclEntity_Sub
 * @param [in] enable
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclOpuScreenOffSet(SclEntity eEntity, bool enable)
{
    if(eEntity >= eSclEntity_Max)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }
    uint32 index = (eEntity == eSclEntity_Main) ? eOpmScrnOff : eOpsScrnOff;

    return dvProAV_AccessWrite(index, enable ? true : false);
}

/**
 * @brief 設定Opu Screen Off 的背景顏色
 * @param [in] eEntity : 通道選擇
 *              -eSclEntity_Main
 *              -eSclEntity_Sub
 * @param [in] bgColor
 *              -[2:0] R:G:B
 *               000b: Black, 001b: Blue, 010b: Green, 011b: Cyan(G + B)
 *               100b: Red, 101b: Megenta(R + B), 110b: Yellow(R + G), 111b: White(R + G + B)
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclOpuScreenOffBgColorSet(SclEntity eEntity, uint08 bgColor)
{
    if(eEntity >= eSclEntity_Max)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }
    uint32 index = (eEntity == eSclEntity_Main) ? eOpmBgColr : eOpsBgColr;

    return  dvProAV_AccessWrite(index, bgColor);
}

/**
 * @brief 設定Vop Screen Off Enable / Disable
 * @param [in] eEntity : 通道選擇
 *              -eSclEntity_Main
 *              -eSclEntity_Sub
 * @param [in] enable
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclVopScreenOffSet(bool enable)
{
    return dvProAV_AccessWrite(eVopScrnOff, enable ? true : false);
}

/**
 * @brief 設定Vop Screen Off 的背景顏色
 * @param [in] color : 設定背景顏色
 *              -[2:0] R:G:B
 *               000b: Black, 001b: Blue, 010b: Green, 011b: Cyan(G + B)
 *               100b: Red, 101b: Megenta(R + B), 110b: Yellow(R + G), 111b: White(R + G + B)
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclVopScreenOffBgColorSet(uint08 bgColor)
{
    return dvProAV_AccessWrite(eVopBgColr, bgColor);
}

/**
 * @brief vop window off set
 * @param [in] eEntity : 通道選擇
 *              -eSclEntity_Main
 *              -eSclEntity_Sub
 * @param [in] bOff :
 *              -false : enable window
 *              -true  : disable window
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclVopWindowOffSet(SclEntity eEntity, bool bOff)
{
    if(eEntity >= eSclEntity_Max)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }

    return dvProAV_AccessWrite((eEntity == eSclEntity_Main)? eVopMainOff : eVopSubOff, bOff);
}

/**
 * @brief vop window off get
 * @param [in] eEntity : 通道選擇
 *              -eSclEntity_Main
 *              -eSclEntity_Sub
 * @return ulOffStatus : false = enable window / true = disable window
 */
bool dvProAV_SclVopWindowOffGet(SclEntity eEntity)
{
    if(eEntity >= eSclEntity_Max)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }
    uint32 ulOffStatus = 0;

    dvProAV_AccessRead((eEntity == eSclEntity_Main)? eVopMainOff : eVopSubOff, &ulOffStatus);

    return (bool)ulOffStatus;
}

//-------------------------------------------------------------------------------------------------
// Sharpness
//-------------------------------------------------------------------------------------------------
/**
 * @brief 設定Sharpness初始參數
 * @param [in] eEntity : 通道選擇
 *              -eSclEntity_Main
 *              -eSclEntity_Sub
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclSharpnessInit(SclEntity eEntity)
{
    int status = rcSUCCESS;
    if(eEntity >= eSclEntity_Max)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }

    status &= dvProAV_AccessWrite((eEntity == eSclEntity_Main) ? eShrpmClip : eShrpsClip, 0x40);
    status &= dvProAV_AccessWrite((eEntity == eSclEntity_Main) ? eShrpmGain : eShrpsGain, 0x01);
    status &= dvProAV_AccessWrite((eEntity == eSclEntity_Main) ? eShrpmClip3 : eShrpsClip3, 0x20);
    status &= dvProAV_AccessWrite((eEntity == eSclEntity_Main) ? eShrpmGain3 : eShrpsGain3, 0x20);
    status &= dvProAV_AccessWrite((eEntity == eSclEntity_Main) ? eShrpmClip5 : eShrpsClip5, 0x20);
    status &= dvProAV_AccessWrite((eEntity == eSclEntity_Main) ? eShrpmGain5 : eShrpsGain5, 0x20);
    status &= dvProAV_AccessWrite((eEntity == eSclEntity_Main) ? eShrpmPeaking : eShrpsPeaking, 0x04);
    status &= dvProAV_AccessWrite((eEntity == eSclEntity_Main) ? eShrpmSPmax : eShrpsSPmax, 0x0020);
    status &= dvProAV_AccessWrite((eEntity == eSclEntity_Main) ? eShrpmShpnsS8 : eShrpsShpnsS8, 0x08);

    return status;
}

/**
 * @brief 設定Sharpness Enable / Disable
 * @param [in] eEntity : 通道選擇
 *              -eSclEntity_Main
 *              -eSclEntity_Sub
 * @param [in] enable : disable = 0 / enable = 1
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclSharpnessEnableSet(SclEntity eEntity, bool enable)
{
    if(eEntity >= eSclEntity_Max)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }
    uint32 index = (eEntity == eSclEntity_Main) ? eShrpmEnable : eShrpsEnable;

    return dvProAV_AccessWrite(index, (uint32)enable);
}

/**
 * @brief Sharpness parameter set
 * @param [in] eEntity : 通道選擇
 *              -eSclEntity_Main
 *              -eSclEntity_Sub
 * @param [in] level : Sharpness level
 * @param [in] scalingratio : scaling ratio
 * @param [in] rgbfull : limit range = 0 / full range = 1
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclSharpnessSet(SclEntity eEntity, uint08 level, uint08 scalingratio, bool rgbfull)
{
    int status = rcSUCCESS;
    if(eEntity >= eSclEntity_Max)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }
    uint32 i;
    uint32 Upx2 = (eEntity == eSclEntity_Main) ? eShrpmUpx2 : eShrpsUpx2;
    uint32 SRatio = (eEntity == eSclEntity_Main) ? eShrpmSRatio : eShrpsSRatio;
    uint32 RgbFull = (eEntity == eSclEntity_Main) ? eShrpmRgbFull : eShrpsRgbFull;
    uint32 ulBkePeaking = (eEntity == eSclEntity_Main) ? eVopmBkePeaking : eVopsBkePeaking;

    uint08 ucScalingLimit = scalingratio;

//    if (scalingratio == 1)
//        scalingratio = 2; //2: scaling up 1~2

    if(ucScalingLimit < 2)  //A70LK_Jacky_0037
    {
        scalingratio = 2;
    }
    else if(ucScalingLimit > 3)
    {
        scalingratio = 4;
    }
    else
    {
        scalingratio = 3;
    }


    for (i = 0; i < SHARPNESS_PARAM_SIZE; i++)
    {
        if (SharpnessParamTable[i].ScalingRatio == scalingratio && SharpnessParamTable[i].Level == level)
            break;
    }

    if (i == SHARPNESS_PARAM_SIZE)
    {
        // not found match table
        return rcSUCCESS;
    }

    if(level == 8)
    {
        dvProAV_SclSharpnessEnableSet(eEntity, FALSE); //A70LK_Jacky_0037
    }
    else
    {
        dvProAV_SclSharpnessEnableSet(eEntity, TRUE);
    }


    status &= dvProAV_AccessWrite(ulBkePeaking, 0x00); // Disbale peaking mode
    status &= dvProAV_AccessWrite(Upx2, SharpnessParamTable[i].Upx2);// Upx, 0: scaling up 1~2,   1: scaling up 3,   2: scaling up 4
    status &= dvProAV_AccessWrite(SRatio, (uint32)SharpnessParamTable[i].SRatio);
    status &= dvProAV_AccessWrite(RgbFull, (uint32)rgbfull); // Input RGB range, 0: limit, 1: full

    return status;
}

/**
 * @brief 設定Sharpness 比重參數
 * @param [in] eEntity : 通道選擇
 *              -eSclEntity_Main
 *              -eSclEntity_Sub
 * @param [in] weight : 比重參數
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclSharpnessWeightSet(SclEntity eEntity, uint08 *weight)
{
    if(eEntity >= eSclEntity_Max)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }
    uint32 index = (eEntity == eSclEntity_Main) ? eShrpmWeight : eShrpsWeight;

    return dvProAV_AccessBurstWrite(index, weight, 32, BURST_INC_ADDR); // Sharpness weight table size is 32 bytes
}

//-------------------------------------------------------------------------------------------------
// Peaking Mode
//-------------------------------------------------------------------------------------------------
/**
 * @brief Peaking Mode set
 * @param [in] eEntity : 通道選擇
 *              -eSclEntity_Main
 *              -eSclEntity_Sub
 * @param [in] mode : Peaking Mode
 *              -spmBypass : bypass
 *              -spmLow    : (1 -4 22 -4 1)/16
 *              -spmMedium : (-1 10 -1)/8
 *              -spmHigh   : (-1 6 -1)/4
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 * @note
 *      功能與Sharpness相近,但Sharpness效果更好,不建議使用PeakingMode
 */
int dvProAV_SclPeakingModeSet(SclEntity eEntity, SclPeakingMode mode)
{
    if((eEntity >= eSclEntity_Max) || (mode >= eSclPeakingMode_Max))
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }
    uint32 ulbkePeaking = (eEntity == eSclEntity_Main) ? eVopmBkePeaking : eVopsBkePeaking;

    return dvProAV_AccessWrite(ulbkePeaking, (uint32)mode);
}

/**
 * @brief Peaking Mode get
 * @param [in] eEntity : 通道選擇
 *              -eSclEntity_Main
 *              -eSclEntity_Sub
 * @param [out] mode : Peaking Mode
 *              -spmBypass : bypass
 *              -spmLow    : (1 -4 22 -4 1)/16
 *              -spmMedium : (-1 10 -1)/8
 *              -spmHigh   : (-1 6 -1)/4
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 * @note
 *      功能與Sharpness相近,但Sharpness效果更好,不建議使用PeakingMode
 */
int dvProAV_SclPeakingModeGet(SclEntity eEntity, SclPeakingMode *pmode)
{
    if(eEntity >= eSclEntity_Max)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }
    uint32 mode, ulbkePeaking = (eEntity == eSclEntity_Main) ? eVopmBkePeaking : eVopsBkePeaking;

    int status = dvProAV_AccessRead(ulbkePeaking, &mode);
    *pmode = (SclPeakingMode)mode;

    return status;
}

//-------------------------------------------------------------------------------------------------
// Test Pattern
//-------------------------------------------------------------------------------------------------
/**
 * @brief 設置測試圖形的輸出時序
 * @param [in] bTpgLocaVop : 位置選擇
 *              -0 : vip
 *              -1 : vop
 * @param [in] SclTpTmg : 輸出時序
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclTpOutputTimingSet(bool bTpgLocaVop, SclTpTmg timing) // bTpgLocaVop Mode 0: Vip Mode 1:Vop
{
    int status = rcSUCCESS;

    if (bTpgLocaVop == 0)
    {
        status &= dvProAV_AccessWrite(eIpmSourceSel, 7);
        status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eIpmHSmpSt : eIpsHSmpSt, (uint32)timing.Hs.Start);
        status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eIpmHSmpSz : eIpsHSmpSz, (uint32)timing.Hs.Size);
        status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eIpmHWrSz : eIpsHWrSz, (uint32)0); // Auto
        status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eIpmVSmpSt : eIpsVSmpSt, (uint32)timing.Vs.Start);
        status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eIpmVSmpSz : eIpsVSmpSz, (uint32)timing.Vs.Size);
        status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eIpmVWrSz : eIpsVWrSz, (uint32)0); // Auto
        status &= dvProAV_AccessWrite(eTpgOpCtrl, 0); // disable the vop
    }

    status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eTpgIpHTotal : eTpgOpHTotal, (uint32)timing.Hs.Total);
    status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eTpgIpHDspSt : eTpgOpHDspSt, (uint32)timing.Hs.Start);
    status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eTpgIpHSize : eTpgOpHSize, (uint32)timing.Hs.Size);
    status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eTpgIpHSynW : eTpgOpHSynW, (uint32)timing.Hs.Sync);
    status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eTpgIpVTotal : eTpgOpVTotal, (uint32)timing.Vs.Total);
    status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eTpgIpVDspSt : eTpgOpVDspSt, (uint32)timing.Vs.Start);
    status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eTpgIpVSize : eTpgOpVSize, (uint32)timing.Vs.Size);
    status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eTpgIpVSynW : eTpgOpVSynW, (uint32)timing.Vs.Sync);
    return status;
}

/**
 * @brief 設置測試圖形的圖形格式
 * @param [in] bTpgLocaVop : 位置選擇
 *              -0 : vip
 *              -1 : vop
 * @param [in] control : 格式選擇
 * 0: Disable
 * 1: H Color Bar
 * 2: V Color Bar
 * 3: Ramp
 * 4: Fill
 * 5: Checker
 * 6: R&B
 * 7: Window
 * 8: Crosshatch
 * 9: Slide Bar
 * 10: Frame11: RGB "T" 32dot
 * 12: RGB "T" 64dot 5x5
 * 13: RGB "T" 64dot 3x3
 * 14: H Stripe
 * 15: V Stripe
 * @param [in] SclTpTmg : 輸出時序
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclTpControlSet(bool bTpgLocaVop, TpControl control)
{
    int status = rcSUCCESS;
    static uint32 lastVipSource = 0; // Save the last vip last input source

    if (control == eSclTpDisable)
    {
        if (bTpgLocaVop == 0)
        {
            status &= dvProAV_AccessWrite(eIpmSourceSel, lastVipSource);
        }
    }
    else
    {
        if (bTpgLocaVop == 0)
        {
            status &= dvProAV_AccessRead(eIpmSourceSel, &lastVipSource); // restore the last vip last input source
            status &= dvProAV_AccessWrite(eTpgOpCtrl, 0); // disable the vop
            status &= dvProAV_AccessWrite(eIpmSourceSel, 7);
        }
    }

    if (control < eSclTpMax)
        status &= dvProAV_AccessWrite((bTpgLocaVop == 0) ? eTpgIpCtrl : eTpgOpCtrl, (uint32)control);
    else
        status = rcERROR;

    return status;
}

/**
 * @brief 設置HColorBar的參數
 * @param [in] bTpgLocaVop : 位置選擇
 *              -0 : vip
 *              -1 : vop
 * @param [in] param : 測試圖形的參數
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclTpHColorBar(bool bTpgLocaVop, TpColorBar param)
{
    int status = rcSUCCESS;

    status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eTpgIpCtrl : eTpgOpCtrl, (uint32)eSclTpHColorBar);
    status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eTpgIpHLMax : eTpgOpCtrl, (uint32)param.limitMax);

    return status;
}

/**
 * @brief 設置VColorBar的參數
 * @param [in] bTpgLocaVop : 位置選擇
 *              -0 : vip
 *              -1 : vop
 * @param [in] param : 測試圖形的參數
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclTpVColorBar(bool bTpgLocaVop, TpColorBar param)
{
    int status = rcSUCCESS;

    status &= dvProAV_AccessWrite((bTpgLocaVop!=0) ? eTpgIpCtrl : eTpgOpCtrl, (uint32)eSclTpVColorBar);
    status &= dvProAV_AccessWrite((bTpgLocaVop!=0) ? eTpgIpVLMax : eTpgOpVLMax, (uint32)param.limitMax);

    return status;
}

/**
 * @brief 設置Ramp的參數
 * @param [in] bTpgLocaVop : 位置選擇
 *              -0 : vip
 *              -1 : vop
 * @param [in] param : 測試圖形的參數
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclTpRamp(bool bTpgLocaVop, TpRamp param)
{
    int status = rcSUCCESS;
    if (param.rampCtrl > 1) param.rampCtrl = 1;


    status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eTpgIpCtrl : eTpgOpCtrl, (uint32)eSclTpRamp);
    status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eTpgIpRampCtrl : eTpgOpRampCtrl, (uint32)param.rampCtrl);

    if (param.rampCtrl == 0)
    {
        status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eTpgIpHrampCnt : eTpgOpHrampCnt, (uint32)param.rampCnt);
        status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eTpgIpHLMin : eTpgOpHLMin, (uint32)param.limitMin);
        status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eTpgIpHLMax : eTpgOpHLMax, (uint32)param.limitMax);
    }
    else
    {
        status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eTpgIpVrampCnt : eTpgOpVrampCnt, (uint32)param.rampCnt);
        status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eTpgIpVLMin : eTpgOpVLMin, (uint32)param.limitMin);
        status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eTpgIpVLMax : eTpgOpVLMax, (uint32)param.limitMax);
    }

    return status;
}

/**
 * @brief 設置Fill的參數
 * @param [in] bTpgLocaVop : 位置選擇
 *              -0 : vip
 *              -1 : vop
 * @param [in] param : 測試圖形的參數
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclTpFill(bool bTpgLocaVop, TpFill param)
{
    int status = rcSUCCESS;

    status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eTpgIpCtrl : eTpgOpCtrl, (uint32)eSclTpFill);
    status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eTpgIpColR : eTpgOpColR, (uint32)param.colorR);
    status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eTpgIpColG : eTpgOpColG, (uint32)param.colorG);
    status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eTpgIpColB : eTpgOpColB, (uint32)param.colorB);

    return status;
}

/**
 * @brief 設置Checker的參數
 * @param [in] bTpgLocaVop : 位置選擇
 *              -0 : vip
 *              -1 : vop
 * @param [in] param : 測試圖形的參數
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclTpChecker(bool bTpgLocaVop, TpChecker param)
{
    int status = rcSUCCESS;

    status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eTpgIpCtrl : eTpgOpCtrl, (uint32)eSclTpChecker);
    status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eTpgIpColR : eTpgOpColR, (uint32)param.colorR);
    status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eTpgIpColG : eTpgOpColG, (uint32)param.colorG);
    status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eTpgIpColB : eTpgOpColB, (uint32)param.colorB);
    status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eTpgIpHLMax : eTpgOpHLMax, (uint32)param.hLimitMax);
    status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eTpgIpVLMax : eTpgOpVLMax, (uint32)param.vLimitMax);

    return status;
}

/**
 * @brief 設置RnB測試圖形的參數
 * @param [in] bTpgLocaVop : 位置選擇
 *              -0 : vip
 *              -1 : vop
 * @param [in] param : 測試圖形的參數
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclTpRnB(bool bTpgLocaVop, TpRnB param)
{
    int status = rcSUCCESS;

    status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eTpgIpCtrl : eTpgOpCtrl, (uint32)eSclTpRnB);

    status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eTpgIpColR : eTpgOpColR, (uint32)param.colorR);
    status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eTpgIpColB : eTpgOpColB, (uint32)param.colorB);

    status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eTpgIpHLMax : eTpgOpHLMax, (uint32)param.hLimitMax);

    return status;
}

/**
 * @brief 設置Window測試圖形的參數
 * @param [in] bTpgLocaVop : 位置選擇
 *              -0 : vip
 *              -1 : vop
 * @param [in] param : 測試圖形的參數
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclTpWindow(bool bTpgLocaVop, TpWindow param)
{
    int status = rcSUCCESS;

    status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eTpgIpCtrl : eTpgOpCtrl, (uint32)eSclTpWindow);
    status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eTpgIpColR : eTpgOpColR, (uint32)param.colorR);
    status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eTpgIpColG : eTpgOpColG, (uint32)param.colorG);
    status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eTpgIpColB : eTpgOpColB, (uint32)param.colorB);

    status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eTpgIpHLMax : eTpgOpHLMax, (uint32)param.hLimitMax);
    status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eTpgIpVLMax : eTpgOpVLMax, (uint32)param.vLimitMax);

    return status;
}

/**
 * @brief 設置CrossHatch測試圖形的參數
 * @param [in] bTpgLocaVop : 位置選擇
 *              -0 : vip
 *              -1 : vop
 * @param [in] param : 測試圖形的參數
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclTpCrossHatch(bool bTpgLocaVop, TpCrossHatch param)
{
    int status = rcSUCCESS;

    status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eTpgIpCtrl : eTpgOpCtrl, (uint32)eSclTpCrossHatch);
    status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eTpgIpFocusCtrl : eTpgOpFocusCtrl, (uint32)param.focusCtrl);
    status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eTpgIpColR : eTpgOpColR, (uint32)param.colorR);
    status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eTpgIpColG : eTpgOpColG, (uint32)param.colorG);
    status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eTpgIpColB : eTpgOpColB, (uint32)param.colorB);

    status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eTpgIpLineWidth : eTpgOpLineWidth, (uint32)param.lineWidth);
    status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eTpgIpHLMax : eTpgOpHLMax, (uint32)param.hLimitMax);
    status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eTpgIpVLMax : eTpgOpVLMax, (uint32)param.vLimitMax);

    return status;
}

/**
 * @brief 設置Silder Bar測試圖形的參數
 * @param [in] bTpgLocaVop : 位置選擇
 *              -0 : vip
 *              -1 : vop
 * @param [in] param : 測試圖形的參數
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclTpSlideBar(bool bTpgLocaVop, TpSlideBar param)
{
    int status = rcSUCCESS;

    status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eTpgIpCtrl : eTpgOpCtrl, (uint32)eSclTpSlideBar);
    status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eTpgIpColR : eTpgOpColR, (uint32)param.colorR);
    status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eTpgIpColG : eTpgOpColG, (uint32)param.colorG);
    status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eTpgIpColB : eTpgOpColB, (uint32)param.colorB);
    status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eTpgIpHLMin : eTpgOpHLMin, (uint32)param.hLimitMin);
    status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eTpgIpHLMax : eTpgOpHLMax, (uint32)param.hLimitMax);
    status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eTpgIpVLMax : eTpgOpVLMax, (uint32)param.vLimitMax);

    return status;
}

/**
 * @brief 設置Frame測試圖形的參數
 * @param [in] bTpgLocaVop : 位置選擇
 *              -0 : vip
 *              -1 : vop
 * @param [in] param : 測試圖形的參數
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclTpFrame(bool bTpgLocaVop, TpFrame param)
{
    int status = rcSUCCESS;

    status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eTpgIpCtrl : eTpgOpCtrl, (uint32)eSclTpFrame);
    status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eTpgIpColR : eTpgOpColR, (uint32)param.colorR);
    status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eTpgIpColG : eTpgOpColG, (uint32)param.colorG);
    status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eTpgIpColB : eTpgOpColB, (uint32)param.colorB);

    status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eTpgIpLineWidth : eTpgOpLineWidth, (uint32)param.lineWidth);

    return status;
}

/**
 * @brief 設置RgbT32Dot測試圖形的參數
 * @param [in] bTpgLocaVop : 位置選擇
 *              -0 : vip
 *              -1 : vop
 * @param [in] param : 測試圖形的參數
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclTpRgbT32Dot(bool bTpgLocaVop, TpRgbT param)
{
    int status = rcSUCCESS;

    status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eTpgIpCtrl : eTpgOpCtrl, (uint32)eSclTpRgbT32Dot);
    status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eTpgIpColR : eTpgOpColR, (uint32)param.colorR);
    status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eTpgIpColG : eTpgOpColG, (uint32)param.colorG);
    status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eTpgIpColB : eTpgOpColB, (uint32)param.colorB);

    status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eTpgIpLineWidth : eTpgOpLineWidth, (uint32)param.lineWidth);

    return status;
}

/**
 * @brief 設置RgbT64Dot5x5測試圖形的參數
 * @param [in] bTpgLocaVop : 位置選擇
 *              -0 : vip
 *              -1 : vop
 * @param [in] param : 測試圖形的參數
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclTpRgbT64Dot5x5(bool bTpgLocaVop, TpRgbT param)
{
    int status = rcSUCCESS;

    status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eTpgIpCtrl : eTpgOpCtrl, (uint32)eSclTpRgbT64Dot5x5);
    status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eTpgIpColR : eTpgOpColR, (uint32)param.colorR);
    status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eTpgIpColG : eTpgOpColG, (uint32)param.colorG);
    status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eTpgIpColB : eTpgOpColB, (uint32)param.colorB);
    status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eTpgIpLineWidth : eTpgOpLineWidth, (uint32)param.lineWidth);

    return status;
}

/**
 * @brief 設置RgbT64Dot3x3測試圖形的參數
 * @param [in] bTpgLocaVop : 位置選擇
 *              -0 : vip
 *              -1 : vop
 * @param [in] param : 測試圖形的參數
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclTpRgbT64Dot3x3(bool bTpgLocaVop, TpRgbT param)
{
    int status = rcSUCCESS;

    status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eTpgIpCtrl : eTpgOpCtrl, (uint32)eSclTpRgbT64Dot3x3);
    status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eTpgIpColR : eTpgOpColR, (uint32)param.colorR);
    status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eTpgIpColG : eTpgOpColG, (uint32)param.colorG);
    status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eTpgIpColB : eTpgOpColB, (uint32)param.colorB);
    status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eTpgIpLineWidth : eTpgOpLineWidth, (uint32)param.lineWidth);

    return status;
}

/**
 * @brief 設置HStrip測試圖形的參數
 * @param [in] bTpgLocaVop : 位置選擇
 *              -0 : vip
 *              -1 : vop
 * @param [in] param : 測試圖形的參數
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclTpHStrip(bool bTpgLocaVop, TpStrip param)
{
    int status = rcSUCCESS;

    status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eTpgIpCtrl : eTpgOpCtrl, (uint32)eSclTpHStrip);
    status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eTpgIpColR : eTpgOpColR, (uint32)param.colorR);
    status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eTpgIpColG : eTpgOpColG, (uint32)param.colorG);
    status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eTpgIpColB : eTpgOpColB, (uint32)param.colorB);
    status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eTpgIpHLMax : eTpgOpHLMax, (uint32)param.limitMax);

    return status;
}

/**
 * @brief 設置VStrip測試圖形的參數
 * @param [in] bTpgLocaVop : 位置選擇
 *              -0 : vip
 *              -1 : vop
 * @param [in] param : 測試圖形的參數
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclTpVStrip(bool bTpgLocaVop, TpStrip param)
{
    int status = rcSUCCESS;

    status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eTpgIpCtrl : eTpgIpCtrl, (uint32)eSclTpVStrip);
    status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eTpgIpColR : eTpgIpColR, (uint32)param.colorR);
    status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eTpgIpColG : eTpgIpColG, (uint32)param.colorG);
    status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eTpgIpColB : eTpgIpColB, (uint32)param.colorB);
    status &= dvProAV_AccessWrite((bTpgLocaVop==0) ? eTpgIpVLMax : eTpgIpVLMax, (uint32)param.limitMax);

    return status;
}

/**
 * @brief 顯示Default Test Pattern
 * @param [in] bTpgLocaVop : 位置選擇
 *              -0 : vip
 *              -1 : vop
 * @param [in] patternID : 測試圖形的編號
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
void dvProAV_SclDefaultTestPatternSet(bool bTpgLocaVop, uint08 patternID)
{
    SclTpTmg timing;
    SclVop vop;

    dvProAV_SclVopTmgGet(&vop);

    timing.Hs.Total = vop.Hs.Total;
    timing.Hs.Start = vop.Hs.Start;
    timing.Hs.Size = vop.Hs.Size;
    timing.Hs.Sync = vop.Hs.Sync;
    timing.Vs.Total = vop.Vs.Total;
    timing.Vs.Start = vop.Vs.Start;
    timing.Vs.Size = vop.Vs.Size;
    timing.Vs.Sync = vop.Vs.Sync;

    dvProAV_SclTpOutputTimingSet(bTpgLocaVop, timing);

    switch(patternID)
    {
    case eSclTpDisable:
        break;
    case eSclTpHColorBar:
        dvProAV_SclTpHColorBar(bTpgLocaVop, m_defaultTpData.HColorBar);
        break;
    case eSclTpVColorBar:
        dvProAV_SclTpVColorBar(bTpgLocaVop, m_defaultTpData.VColorBar);
        break;
    case eSclTpFill:
        dvProAV_SclTpFill(bTpgLocaVop, m_defaultTpData.Fill);
        break;
    case eSclTpHRamp:
        dvProAV_SclTpRamp(bTpgLocaVop, m_defaultTpData.HRamp);
        patternID = eSclTpRamp;
        break;
    case eSclTpVRamp:
        dvProAV_SclTpRamp(bTpgLocaVop, m_defaultTpData.VRamp);
        patternID = eSclTpRamp;
        break;
    case eSclTpChecker:
        dvProAV_SclTpChecker(bTpgLocaVop, m_defaultTpData.Checker);
        break;
    case eSclTpRnB:
        dvProAV_SclTpRnB(bTpgLocaVop, m_defaultTpData.RnB);
        break;
    case eSclTpWindow:
        dvProAV_SclTpWindow(bTpgLocaVop, m_defaultTpData.Window);
        break;
    case eSclTpCrossHatch:
        dvProAV_SclTpCrossHatch(bTpgLocaVop, m_defaultTpData.CrossHatch);
        break;
    case eSclTpCrossHatch2:
        dvProAV_SclTpCrossHatch(bTpgLocaVop, m_defaultTpData.CrossHatch2);
        patternID = eSclTpCrossHatch;
        break;
    case eSclTpCrossHatch3:
        dvProAV_SclTpCrossHatch(bTpgLocaVop, m_defaultTpData.CrossHatch3);
        patternID = eSclTpCrossHatch;
        break;
    case eSclTpSlideBar:
        dvProAV_SclTpSlideBar(bTpgLocaVop, m_defaultTpData.SliderBar);
        break;
    case eSclTpFrame:
        dvProAV_SclTpFrame(bTpgLocaVop, m_defaultTpData.Frame);
        break;
    case eSclTpRgbT32Dot:
        dvProAV_SclTpRgbT32Dot(bTpgLocaVop, m_defaultTpData.RgbT32Dot);
        break;
    case eSclTpRgbT64Dot5x5:
        dvProAV_SclTpRgbT64Dot5x5(bTpgLocaVop, m_defaultTpData.TpRgbT64Dot5x5);
        break;
    case eSclTpRgbT64Dot3x3:
        dvProAV_SclTpRgbT64Dot3x3(bTpgLocaVop, m_defaultTpData.TpRgbT64Dot3x3);
        break;
    case eSclTpHStrip:
        dvProAV_SclTpHStrip(bTpgLocaVop, m_defaultTpData.HStrip);
        break;
    case eSclTpVStrip:
        dvProAV_SclTpVStrip(bTpgLocaVop, m_defaultTpData.VStrip);
        break;
    }

    dvProAV_SclTpControlSet(bTpgLocaVop, (TpControl)patternID);
}

//-------------------------------------------------------------------------------------------------
// Logo Capture
//-------------------------------------------------------------------------------------------------
/**
 * @brief Logo Capture Enable
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclLogoCaptureEn(void)
{
    return dvProAV_AccessWrite(eLocCapLocCapEn, true);
}

/**
 * @brief Logo Capture Busy Status Get
 * @return ulBusyState : false = unbusy, true = busy
 */
bool dvProAV_SclLogoCaptureBusyStatusGet(void)
{
    uint32 ulBusyStatus = 0;
    dvProAV_AccessRead(eLocCapLocCapEn, &ulBusyStatus);
    return (bool)ulBusyStatus;
}

/**
 * @brief Logo Display Enable
 * @param bEnable :
 *          - false : display back ground color
 *          - true : display logo
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclLogoDisplayEn(bool bEnable)
{
    return dvProAV_AccessWrite(eLogoLogoEn, bEnable);
}

//-------------------------------------------------------------------------------------------------
// Fading
//-------------------------------------------------------------------------------------------------
/**
 * @brief Fade Enable
 * @param [in] bEnable :
 *          - false : disable fading
 *          - true : Enable fading
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclFadeEn(bool bEnable)
{
    return dvProAV_AccessWrite(eVopFadeEn, bEnable);
}

/**
 * @brief Fade Interupt
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclFadeInt(void)
{
    int status = rcSUCCESS;
    status &= dvProAV_AccessWrite(eVopFadeInt, true);
    status &= dvProAV_AccessWrite(eVopFadeInt, false);
    return status;
}

/**
 * @brief Fade Curve Set
 * @param [in] bEnable :
 *          - false : Linear Curve
 *          - true : Smooth Curve
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclFadeCurveSet(bool bCurve)
{
    return dvProAV_AccessWrite(eVopFadeAccGCEn, bCurve);
}

/**
 * @brief Fade Channel Ctrl
 * @param [in] bSubEn :
 *          - false : Sub window fade-out
 *          - true : Sub window fade-in
 * @param [in] bMainEn :
 *          - false : Main window fade-out
 *          - true : Main window fade-in
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclFadeChCtrl(bool bSubEn, bool bMainEn)
{
    uint8 ucCtrl = 0;
    ucCtrl = (((ucCtrl | bSubEn) << 1) | bMainEn) & 0x03;
    return dvProAV_AccessWrite(eVopFadeScrnOpt, ucCtrl);
}

/**
 * @brief Fade Rate Set (Fade Step)
 * @param [in] ucFadeRate :
 *          - 0 : x1
 *          - 1 : x2
 *          - 2 : x3
 *          - 3 : x4
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclFadeRateSet(uint08 ucFadeRate)
{
    if(ucFadeRate > 3)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }

    return dvProAV_AccessWrite(eVopFadeFFSel, ucFadeRate);
}

/**
 * @brief Fade Time Set, the duration of fade
 * @param [in] ucFadeTime : 0 ~ 15 = 256(frame) * ucTime
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclFadeTimeSet(uint08 ucFadeTime)
{
    if(ucFadeTime > 15)
        ucFadeTime = 15;

    return dvProAV_AccessWrite(eVopFadeTmSpn, ucFadeTime);
}

/**
 * @brief Get Fade busy state
 * @return ulBusyStatus : 0=unbusy, 1=busy
 */
bool dvProAV_SclFadeBusyGet(void)
{
    UINT32 ulBusyStatus = 0;
    dvProAV_AccessRead(eVopFadeBz, &ulBusyStatus);
    return (bool)ulBusyStatus;
}

//-------------------------------------------------------------------------------------------------
// IR Hidden
//-------------------------------------------------------------------------------------------------
/**
 * @brief IR Hidden Enable
 * @param [in] bEnable :
 *          - false : disable fading
 *          - true : Enable fading
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclIrHidEn(bool bEnable)
{
    return dvProAV_AccessWrite(eVopIRHidCalEn, bEnable);
}

/**
 * @brief Ir Hidden Frame Rate Set
 * @param [in] ucRatio : 0~255, IR video = ucRatio : 1
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclIrHidFrameRateSet(uint08 ucRatio)
{
    return dvProAV_AccessWrite(eVopIRFrmRatio, ucRatio);
}

/**
 * @brief Ir Hidden Delay Set
 * @param uiDelay : 0~384, IR pulse delay is uiDelay/128 frame
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclIrHidDlySet(uint16 uiDelay)
{
    if(uiDelay > 384)   // maximum Ir Hidden Delay is 3 frame
        uiDelay = 384;
    return dvProAV_AccessWrite(eVopIRDly, uiDelay);
}

/**
 * @brief Ir Hidden Pulse Width Set
 * @param ucPulseWidth : 0~8, IR pulse width is ucPulseWidth/8 frame
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclIrHidPulseWidSet(uint08 ucPulseWidth)
{
    if(ucPulseWidth > 8)    // maximum IR pulse width is 1 frame
        ucPulseWidth = 8;
    return dvProAV_AccessWrite(eVopIRPulseWid, ucPulseWidth);
}

//-------------------------------------------------------------------------------------------------
// Local Focus
//-------------------------------------------------------------------------------------------------
/**
 * @brief dvProAV_SclLofFrmatEn
 * @param bEnable
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclLofFrmatEn(bool bEnable)
{
    return dvProAV_AccessWrite(eLafocFrcPixFmt, bEnable);
}

int dvProAV_SclLofForceFrmatSet(bool bFormat)
{
    return dvProAV_AccessWrite(eLafocFrcYCbCr, bFormat);
}

int dvProAV_SclLofBypassEn(bool bEnable)
{
    return dvProAV_AccessWrite(eLafocBypass, bEnable);
}

int dvProAV_SclLofFrmatSet(bool bFormat)
{
    return dvProAV_AccessWrite(eLafocPixFmt, bFormat);
}

int dvProAV_SclLofRgbRangeSet(bool bFull)
{
    return dvProAV_AccessWrite(eLafocRgbFull, bFull);
}

int dvProAV_SclLofShpnsEn(bool bEnable)
{
    return dvProAV_AccessWrite(eLafocShpnsEnable, bEnable);
}

int dvProAV_SclLofSclUpRatioSet(uint08 ucSclUpRatio)
{
    return dvProAV_AccessWrite(eLafocUpx2, ucSclUpRatio);
}

int dvProAV_SclLofShpnsClipSet(uint08 ucPixel, uint08 ucClip)
{
    uint32 ulReg = (ucPixel == 1)? eLafocClip : (ucPixel == 3)? eLafocClip3 : (ucPixel == 5)? eLafocClip5 : 0;
    if(ulReg == 0)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }
    return dvProAV_AccessWrite(ulReg, ucClip);
}

int dvProAV_SclLofShpnsGainSet(uint08 ucPixel, uint08 ucGain)
{
    uint32 ulReg = (ucPixel == 1)? eLafocGain : (ucPixel == 3)? eLafocGain3 : (ucPixel == 5)? eLafocGain5 : 0;
    if(ulReg == 0)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }
    return dvProAV_AccessWrite(ulReg, ucGain);
}

int dvProAV_SclLofPeakingEn(bool bEnable)
{
    return dvProAV_AccessWrite(eLafocPeaking, bEnable);
}

int dvProAV_SclLofShpnsMaxSet(uint16 uiShpnsMax)
{
    return dvProAV_AccessWrite(eLafocSPmax, uiShpnsMax);
}

int dvProAV_SclLofShpnsS8Set(uint08 ucShpns)
{
    return dvProAV_AccessWrite(eLafocShpnsS8, ucShpns);
}

int dvProAV_SclLofShpnsRatioSet(uint08 ucRatio)
{
    return dvProAV_AccessWrite(eLafocSRatio, ucRatio);
}

int dvProAV_SclLofShpnsWeightSet(uint08 *ucRatio)
{
    return dvProAV_AccessBurstWrite(eLafocWeight, ucRatio, 32, BURST_INC_ADDR);
}

/**
 * @brief Local Focus LUT Write Enable
 * @param [in] bEnable :
 *              -false : write down
 *              -true : write enable
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclLofTableWrEn(bool bEnable)
{
    return dvProAV_AccessWrite(eLafocTableWrite, bEnable);
}

/**
 * @brief Local Focus Enable
 * @param [in] bEnable :
 *              -false : use local focus sharpness
 *              -true : use local focus
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclLofLocalFocEn(bool bEnable)
{
    return dvProAV_AccessWrite(eLafocGdEnable, bEnable);
}

/**
 * @brief Local Focus Shift Set
 * @param [in] ucShift
 *              -0 : no shift
 *              -1 : shift right 2 bits
 *              -2 : shift right 4 bits
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclLofShiftSet(uint08 ucShift)
{
    if(ucShift > 2)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }
    return dvProAV_AccessWrite(eLafocVGofHGGSft, (ucShift * 2));
}

/**
 * @brief Local Focus Block Set
 * @param [in] ucBlkCntH : 0 ~ 31
 * @param [in] ucBlkCntV : 0 ~ 31
 * @return 成功(rcSUCCESS)或失敗(rcERROR)
 */
int dvProAV_SclLofBlockSet(uint08 ucBlkCntH, uint08 ucBlkCntV)
{
    int status = rcSUCCESS;
    if(ucBlkCntH > 31)
        ucBlkCntH = 31;
    if(ucBlkCntV > 31)
        ucBlkCntV = 31;

    status &= dvProAV_AccessWrite(eLafocHtotBlock, ucBlkCntH);
    status &= dvProAV_AccessWrite(eLafocVtotBlock, ucBlkCntV);
    return status;
}

/**
 * @brief Local Focus LUT Start Address Count Set
 * @param [in] uiStAddrCnt : start address count
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclLofLutStAddrSet(uint16 uiStAddrCnt)
{
    int status = rcSUCCESS;
    uint08 ucStAddrCnt[2];
    ucStAddrCnt[0] = (uiStAddrCnt & 0xff);
    ucStAddrCnt[1] = ((uiStAddrCnt >> 8) & 0xff);

    status &= dvProAV_AccessBurstWrite(eLafocLutAdrPort, ucStAddrCnt, 2, BURST_FIX_ADDR);
    return status;
}

/**
 * @brief Local Focus LUT Data Set
 * @param [in] uiCount : data byte count
 * @param [in] ucData : data point,
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclLofLutDataSet(uint16 uiCount, uint08 *ucData)
{
    int status = rcSUCCESS;

    status &= dvProAV_AccessBurstWrite(eLafocLutDataPort, ucData, uiCount, BURST_FIX_ADDR);
    return status;
}

//-------------------------------------------------------------------------------------------------
// Real Black
//-------------------------------------------------------------------------------------------------
/**
 * @brief Get RGB Max & Min Level for Real Black
 * @param [out] VopRgbLv : vop rgb max & min level
 * @return rcSUCCESS or rcERROR
 */
int dvProAV_SclRgbLvGet(DetLv *VopRgbLv)
{
    return dvProAV_AccessBurstRead(eVopVopMaxRLvl_E4k, (uint8 *)VopRgbLv, 12, BURST_INC_ADDR);
}

/**
 * @brief Set Real Black Area
 * @param [in] u16RlbStarH : real black area H start
 * @param [in] u16RlbStarV : real black area V start
 * @param [in] u16RlbSizeH : real black area H size
 * @param [in] u16RlbSizeV : real black area V size
 * @return rcSUCCESS or rcERROR
 */
int dvProAV_SclRelBlkAreaSet(uint16 u16RlbStarH, uint16 u16RlbStarV, uint16 u16RlbSizeH, uint16 u16RlbSizeV)
{
    int status = rcSUCCESS;

    status &= dvProAV_AccessWrite(eVopGetHSt_E4k, u16RlbStarH);
    status &= dvProAV_AccessWrite(eVopGetVSt_E4k, u16RlbStarV);
    status &= dvProAV_AccessWrite(eVopGetWid_E4k, u16RlbSizeH);
    status &= dvProAV_AccessWrite(eVopGetHei_E4k, u16RlbSizeV);
    return status;
}

//-------------------------------------------------------------------------------------------------
// Common
//-------------------------------------------------------------------------------------------------
/**
 * @brief FPGA temperature(Celsius) get
 * @return FPGA temperature(Celsius)
 */
UINT16 dvProAV_SclFpgaTempGet(void)
{
    int status = rcSUCCESS;
    uint32 ulFpgaTempAdc = 0;
    uint16 uiFpgaTemp = 0;

    //status &= dvProAV_AccessWrite(eBiuTsdEn, true); //A70LK_Larry_0100
    //DelayMSec(2);    // need 10MHz 1024 clock cycles delay
    status &= dvProAV_AccessRead(eBiuTsdAdc, &ulFpgaTempAdc);   // temperature(Celsius) = ( 693 x BiuTsdAdc ) ÷ 1024 - 265
    //status &= dvProAV_AccessWrite(eBiuTsdEn, false); //A70LK_Larry_0100

    if((ulFpgaTempAdc == 0) || (status == rcERROR))
        return uiFpgaTemp = 0;

    if((693 * ulFpgaTempAdc) > 271360)
    {
        ulFpgaTempAdc = (693 * ulFpgaTempAdc) - 271360;
    }
    else
    {
        ulFpgaTempAdc  = 0;
    }
    uiFpgaTemp = (uint16)(ulFpgaTempAdc / 10);  // 取小數點下兩位

    return uiFpgaTemp;
}

int dvProAV_SclFpgaTempADCRst(void) //A70LK_Larry_0100
{
    int status = rcSUCCESS;

    // FPGA temperature ADC Reset
    status &= dvProAV_AccessWrite(eBiuTsdEn, false);
    status &= dvProAV_AccessWrite(eBiuTsdRst, true);
    status &= dvProAV_AccessWrite(eBiuTsdRst, false);
    status &= dvProAV_AccessWrite(eBiuTsdEn, true);

    return status;
}

int dvProAV_SclFpgaTempThreshold(uint16 uiFpgaTemp)
{
    //uiFpgaTemp = temperature*100, ex:30C = 3000
    uint32 ulFpgaTempAdc = 0;

    ulFpgaTempAdc = (uint32)uiFpgaTemp;
    ulFpgaTempAdc = ((ulFpgaTempAdc * 10) + 271360);
    ulFpgaTempAdc = ulFpgaTempAdc / 693;

    return dvProAV_AccessWrite(eBiuTsdHighThr, ulFpgaTempAdc&0xFFFF);
}

/**
 * @brief 取得Scaler版本號碼
 * @return Scaler版本號碼
 *         Byte 0 : sub version
 *         Byte 1 : main version
 *         Byte 2 : minor version
 *         Byte 3 : (reserved)
 */
UINT32 dvProAV_SclVersionGet(void)
{
    return m_version;
}

/**
 * @brief 初始化ProAV 的相關設定
 */
void dvProAV_ScalerDriverInit(void)
{
    int status = rcSUCCESS;
    uint32 mainVer;
    uint32 subVer;
    uint32 minorVer;


    // Read ProAV FPGA Version
    status &= dvProAV_AccessRead(eComPrjMainRev, &mainVer);
    status &= dvProAV_AccessRead(eComPrjSubRev, &subVer);
    status &= dvProAV_AccessRead(eComPrjMinorRev, &minorVer);

    m_version = (uint32) (minorVer << 16 | mainVer << 8 | subVer);

    // FPGA temperature ADC Reset
    status &= dvProAV_AccessWrite(eBiuTsdRst, true);
    status &= dvProAV_AccessWrite(eBiuTsdRst, false);

    // Set memory base address of scaling windows
    status &= dvProAV_AccessWrite(eIpmBaseAdr, (DRAM_WND1_MEM_ADDR / 2));
    status &= dvProAV_AccessWrite(eOpmBaseAdr, (DRAM_WND1_MEM_ADDR / 2));
    status &= dvProAV_AccessWrite(eIpmBaseAdr2, (DRAM_WND2_MEM_ADDR / 2));
    status &= dvProAV_AccessWrite(eOpmBaseAdr2, (DRAM_WND2_MEM_ADDR / 2));
    status &= dvProAV_AccessWrite(eIpmBaseAdr3, (DRAM_WND3_MEM_ADDR / 2));
    status &= dvProAV_AccessWrite(eOpmBaseAdr3, (DRAM_WND3_MEM_ADDR / 2));
    status &= dvProAV_AccessWrite(eIpmBaseAdr4, (DRAM_WND4_MEM_ADDR / 2));
    status &= dvProAV_AccessWrite(eOpmBaseAdr4, (DRAM_WND4_MEM_ADDR / 2));
    status &= dvProAV_AccessWrite(eIpmBaseAdr5, (DRAM_WND5_MEM_ADDR / 2));
    status &= dvProAV_AccessWrite(eOpmBaseAdr5, (DRAM_WND5_MEM_ADDR / 2));
    status &= dvProAV_AccessWrite(eIpmBaseAdr6, (DRAM_WND6_MEM_ADDR / 2));
    status &= dvProAV_AccessWrite(eOpmBaseAdr6, (DRAM_WND6_MEM_ADDR / 2));
    status &= dvProAV_AccessWrite(eOpmMfBsAdr, (DRAM_MTN_MEM_ADDR / 2));
    status &= dvProAV_AccessWrite(eIpsBaseAdr, (DRAM_SWND1_MEM_ADDR / 2));
    status &= dvProAV_AccessWrite(eOpsBaseAdr, (DRAM_SWND1_MEM_ADDR / 2));
    status &= dvProAV_AccessWrite(eIpsBaseAdr2, (DRAM_SWND2_MEM_ADDR / 2));
    status &= dvProAV_AccessWrite(eOpsBaseAdr2, (DRAM_SWND2_MEM_ADDR / 2));
    status &= dvProAV_AccessWrite(eIpsBaseAdr3, (DRAM_SWND3_MEM_ADDR / 2));
    status &= dvProAV_AccessWrite(eOpsBaseAdr3, (DRAM_SWND3_MEM_ADDR / 2));
    status &= dvProAV_AccessWrite(eIpsBaseAdr4, (DRAM_SWND4_MEM_ADDR / 2));
    status &= dvProAV_AccessWrite(eOpsBaseAdr4, (DRAM_SWND4_MEM_ADDR / 2));
    status &= dvProAV_AccessWrite(eIpsBaseAdr5, (DRAM_SWND5_MEM_ADDR / 2));
    status &= dvProAV_AccessWrite(eOpsBaseAdr5, (DRAM_SWND5_MEM_ADDR / 2));
    status &= dvProAV_AccessWrite(eIpsBaseAdr6, (DRAM_SWND6_MEM_ADDR / 2));
    status &= dvProAV_AccessWrite(eOpsBaseAdr6, (DRAM_SWND6_MEM_ADDR / 2));
    status &= dvProAV_AccessWrite(eOpsMfBsAdr, (DRAM_SMTN_MEM_ADDR / 2));
    status &= dvProAV_AccessWrite(eVopGamaAdr, (DRAM_GAMMA_ADDR / 2));
    status &= dvProAV_AccessWrite(eWcuWrpWopBasAdr1, (DRAM_WOP1_ADDR / 2));
    status &= dvProAV_AccessWrite(eWcuWrpWopBasAdr2, (DRAM_WOP2_ADDR / 2));
    status &= dvProAV_AccessWrite(eWcuWipWrpBasAdr, (DRAM_WIP_ADDR / 2));
    status &= dvProAV_AccessWrite(eLocCapLocCapBasAdr, (DRAM_LOGO_CAPTURE_ADDR / 2));
    status &= dvProAV_AccessWrite(eLogoLogoBasAdr, (DRAM_LOGO_DISPLAY1_2D_ADDR / 2));  //H2PF_Simon_0056
    LOG_MSG(db_DV_SCALER, "ProAV Scaler Driver, Version = %ld.%ld.%ld\n", (m_version >> 8) & 0xff, m_version & 0xff, minorVer);

    // Set Burst R/W without increase address
    status &= dvProAV_AccessBurstAddrIncEnable(false);

    status &= dvProAV_AccessBurstAddrIncEnable(true); // force the to enable automatic address indexing

#ifndef QT_CPP
    // Load Gamma Table
    uint32 GammaSplit = 0;

    status &= dvProAV_AccessWrite(eVopGmaRgbSep, (uint32)FALSE); // Set RGB Gamma have same curve
    dvProAV_SclGammaTableSet(0, dvProAV_SclGammaTableCaculate(1.8, 0)); // Load Gamma Power 1.8 Table to R Channel
    if (GammaSplit!=0)
    {
        dvProAV_SclGammaTableSet(1, dvProAV_SclGammaTableCaculate(1.8, 1)); // Load Gamma Power 1.8 Table to G Channel
        dvProAV_SclGammaTableSet(2, dvProAV_SclGammaTableCaculate(1.8, 2)); // Load Gamma Power 1.8 Table to B Channel
    }
#endif

#if 1 // TODO: Enable Watch Dog to Fix FPGA MIU Busy issue
        status &= dvProAV_AccessWrite(eMiuWdtEn, true);
        status &= dvProAV_AccessWrite(eMwiu2WdtEn, true);
#endif

#ifdef dvProAV_SCALER_DEBUG
    if (status != rcSUCCESS)
        DBMSG("ProAV Scaler Driver Init Fail\n");
    else
        DBMSG("ProAV Scaler Driver, Version = %ld.%ld\n", mainVer, subVer);
#endif

#ifndef QT_CPP
    LOG_MSG(db_DV_SCALER, "Flash Bus Width is 1 Byte for 1 address\n");DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "FLASH_DATINFO_ADDR      = 0x%08lX\n", FLASH_DATINFO_ADDR);DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "FLASH_OSDTEXT_ADDR      = 0x%08lX\n", FLASH_OSDTEXT_ADDR);DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "FLASH_OSDBMP0_ADDR      = 0x%08lX\n", FLASH_OSDBMP0_ADDR);DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "FLASH_OSDBMP1_ADDR      = 0x%08lX\n", FLASH_OSDBMP1_ADDR);DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "FLASH_OSDBMP2_ADDR      = 0x%08lX\n", FLASH_OSDBMP2_ADDR);DelayMSec(5);

    LOG_MSG(db_DV_SCALER, "FLASH_LOGO0_2D_ADDR   = 0x%08lX\n", FLASH_LOGO0_2D_ADDR);DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "FLASH_LOGO1_2D_ADDR   = 0x%08lX\n", FLASH_LOGO1_2D_ADDR);DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "FLASH_LOGO2_2D_ADDR   = 0x%08lX\n", FLASH_LOGO2_2D_ADDR);DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "FLASH_LOGO0_3D_ADDR   = 0x%08lX\n", FLASH_LOGO0_3D_ADDR);DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "FLASH_LOGO1_3D_ADDR   = 0x%08lX\n", FLASH_LOGO1_3D_ADDR);DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "FLASH_LOGO2_3D_ADDR   = 0x%08lX\n", FLASH_LOGO2_3D_ADDR);DelayMSec(5);

    LOG_MSG(db_DV_SCALER, "FLASH_WRPTBL0_ADDR      = 0x%08lX\n", FLASH_WRPTBL0_ADDR);DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "FLASH_WRPTBL1_ADDR      = 0x%08lX\n", FLASH_WRPTBL1_ADDR);DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "FLASH_WRPTBL2_ADDR      = 0x%08lX\n", FLASH_WRPTBL2_ADDR);DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "FLASH_WRPTBL3_ADDR      = 0x%08lX\n", FLASH_WRPTBL3_ADDR);DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "FLASH_WRPTBL4_ADDR      = 0x%08lX\n", FLASH_WRPTBL4_ADDR);DelayMSec(5);

    LOG_MSG(db_DV_SCALER, "FLASH_USED_SIZE         = 0x%08lX\n", FLASH_USED_SIZE);DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "FLASH_FREE_SIZE         = 0x%08lX\n", FLASH_MEM_SIZE - FLASH_USED_SIZE);DelayMSec(5);

    LOG_MSG(db_DV_SCALER, "Dram Bus Width is 64 Bytes(512 bits) for 1 address\n");DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "DRAM_WND1_MEM_ADDR      = 0x%08lX (0x%08lX)\n", DRAM_WND1_MEM_ADDR,DRAM_WND1_MEM_ADDR * PROAV_DRAM_LINE_BYTES);DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "DRAM_WND2_MEM_ADDR      = 0x%08lX (0x%08lX)\n", DRAM_WND2_MEM_ADDR,DRAM_WND2_MEM_ADDR * PROAV_DRAM_LINE_BYTES);DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "DRAM_WND3_MEM_ADDR      = 0x%08lX (0x%08lX)\n", DRAM_WND3_MEM_ADDR,DRAM_WND3_MEM_ADDR * PROAV_DRAM_LINE_BYTES);DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "DRAM_WND4_MEM_ADDR      = 0x%08lX (0x%08lX)\n", DRAM_WND4_MEM_ADDR,DRAM_WND4_MEM_ADDR * PROAV_DRAM_LINE_BYTES);DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "DRAM_WND5_MEM_ADDR      = 0x%08lX (0x%08lX)\n", DRAM_WND5_MEM_ADDR,DRAM_WND5_MEM_ADDR * PROAV_DRAM_LINE_BYTES);DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "DRAM_WND6_MEM_ADDR      = 0x%08lX (0x%08lX)\n", DRAM_WND6_MEM_ADDR,DRAM_WND6_MEM_ADDR * PROAV_DRAM_LINE_BYTES);DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "DRAM_SWND1_MEM_ADDR     = 0x%08lX (0x%08lX)\n", DRAM_SWND1_MEM_ADDR,DRAM_SWND1_MEM_ADDR * PROAV_DRAM_LINE_BYTES);DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "DRAM_SWND2_MEM_ADDR     = 0x%08lX (0x%08lX)\n", DRAM_SWND2_MEM_ADDR,DRAM_SWND2_MEM_ADDR * PROAV_DRAM_LINE_BYTES);DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "DRAM_SWND3_MEM_ADDR     = 0x%08lX (0x%08lX)\n", DRAM_SWND3_MEM_ADDR,DRAM_SWND3_MEM_ADDR * PROAV_DRAM_LINE_BYTES);DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "DRAM_SWND4_MEM_ADDR     = 0x%08lX (0x%08lX)\n", DRAM_SWND4_MEM_ADDR,DRAM_SWND4_MEM_ADDR * PROAV_DRAM_LINE_BYTES);DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "DRAM_SWND5_MEM_ADDR     = 0x%08lX (0x%08lX)\n", DRAM_SWND5_MEM_ADDR,DRAM_SWND5_MEM_ADDR * PROAV_DRAM_LINE_BYTES);DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "DRAM_SWND6_MEM_ADDR     = 0x%08lX (0x%08lX)\n", DRAM_SWND6_MEM_ADDR,DRAM_SWND6_MEM_ADDR * PROAV_DRAM_LINE_BYTES);DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "DRAM_MTN_MEM_ADDR       = 0x%08lX (0x%08lX)\n", DRAM_MTN_MEM_ADDR,DRAM_MTN_MEM_ADDR * PROAV_DRAM_LINE_BYTES);DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "DRAM_SMTN_MEM_ADDR      = 0x%08lX (0x%08lX)\n", DRAM_SMTN_MEM_ADDR,DRAM_SMTN_MEM_ADDR * PROAV_DRAM_LINE_BYTES);DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "DRAM_GAMMA_ADDR         = 0x%08lX (0x%08lX)\n", DRAM_GAMMA_ADDR, DRAM_GAMMA_ADDR * PROAV_DRAM_LINE_BYTES);DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "DRAM_OSD_MEM1_ADDR      = 0x%08lX (0x%08lX)\n", DRAM_OSD_MEM1_ADDR,DRAM_OSD_MEM1_ADDR * PROAV_DRAM_LINE_BYTES);DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "DRAM_OSD_MEM2_ADDR      = 0x%08lX (0x%08lX)\n", DRAM_OSD_MEM2_ADDR,DRAM_OSD_MEM2_ADDR * PROAV_DRAM_LINE_BYTES);DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "DRAM_OSD_MEM3_ADDR      = 0x%08lX (0x%08lX)\n", DRAM_OSD_MEM3_ADDR,DRAM_OSD_MEM3_ADDR * PROAV_DRAM_LINE_BYTES);DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "DRAM_OSD_TEXT_ADDR      = 0x%08lX (0x%08lX)\n", DRAM_OSD_TEXT_ADDR,DRAM_OSD_TEXT_ADDR * PROAV_DRAM_LINE_BYTES);DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "DRAM_OSD_BMP_ADDR       = 0x%08lX (0x%08lX)\n", DRAM_OSD_BMP_ADDR,DRAM_OSD_BMP_ADDR * PROAV_DRAM_LINE_BYTES);DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "DRAM_HDRPQ1_ADDR        = 0x%08lX (0x%08lX)\n", DRAM_HDRPQ1_ADDR,DRAM_HDRPQ1_ADDR * PROAV_DRAM_LINE_BYTES);DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "DRAM_HDRPQ2_ADDR        = 0x%08lX (0x%08lX)\n", DRAM_HDRPQ2_ADDR,DRAM_HDRPQ2_ADDR * PROAV_DRAM_LINE_BYTES);DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "DRAM_HDRPQ3_ADDR        = 0x%08lX (0x%08lX)\n", DRAM_HDRPQ3_ADDR,DRAM_HDRPQ3_ADDR * PROAV_DRAM_LINE_BYTES);DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "DRAM_HDRPQ4_ADDR        = 0x%08lX (0x%08lX)\n", DRAM_HDRPQ4_ADDR,DRAM_HDRPQ4_ADDR * PROAV_DRAM_LINE_BYTES);DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "DRAM_HDRHL1_ADDR        = 0x%08lX (0x%08lX)\n", DRAM_HDRHL1_ADDR,DRAM_HDRHL1_ADDR * PROAV_DRAM_LINE_BYTES);DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "DRAM_HDRHL2_ADDR        = 0x%08lX (0x%08lX)\n", DRAM_HDRHL2_ADDR,DRAM_HDRHL2_ADDR * PROAV_DRAM_LINE_BYTES);DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "DRAM_HDRHL3_ADDR        = 0x%08lX (0x%08lX)\n", DRAM_HDRHL3_ADDR,DRAM_HDRHL3_ADDR * PROAV_DRAM_LINE_BYTES);DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "DRAM_HDRHL4_ADDR        = 0x%08lX (0x%08lX)\n", DRAM_HDRHL4_ADDR,DRAM_HDRHL4_ADDR * PROAV_DRAM_LINE_BYTES);DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "DRAM_LOGO_CAPTURE_ADDR  = 0x%08lX (0x%08lX)\n", DRAM_LOGO_CAPTURE_ADDR,DRAM_LOGO_CAPTURE_ADDR * PROAV_DRAM_LINE_BYTES);DelayMSec(5);

    LOG_MSG(db_DV_SCALER, "DRAM_LOGO_DISPLAY0_2D_ADDR = 0x%08lX (0x%08lX)\n", DRAM_LOGO_DISPLAY0_2D_ADDR,DRAM_LOGO_DISPLAY0_2D_ADDR * PROAV_DRAM_LINE_BYTES);DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "DRAM_LOGO_DISPLAY1_2D_ADDR = 0x%08lX (0x%08lX)\n", DRAM_LOGO_DISPLAY1_2D_ADDR,DRAM_LOGO_DISPLAY1_2D_ADDR * PROAV_DRAM_LINE_BYTES);DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "DRAM_LOGO_DISPLAY2_2D_ADDR = 0x%08lX (0x%08lX)\n", DRAM_LOGO_DISPLAY2_2D_ADDR,DRAM_LOGO_DISPLAY2_2D_ADDR * PROAV_DRAM_LINE_BYTES);DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "DRAM_LOGO_DISPLAY0_3D_ADDR = 0x%08lX (0x%08lX)\n", DRAM_LOGO_DISPLAY0_3D_ADDR,DRAM_LOGO_DISPLAY0_3D_ADDR * PROAV_DRAM_LINE_BYTES);DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "DRAM_LOGO_DISPLAY1_3D_ADDR = 0x%08lX (0x%08lX)\n", DRAM_LOGO_DISPLAY1_3D_ADDR,DRAM_LOGO_DISPLAY1_3D_ADDR * PROAV_DRAM_LINE_BYTES);DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "DRAM_LOGO_DISPLAY2_3D_ADDR = 0x%08lX (0x%08lX)\n", DRAM_LOGO_DISPLAY2_3D_ADDR,DRAM_LOGO_DISPLAY2_3D_ADDR * PROAV_DRAM_LINE_BYTES);DelayMSec(5);

    LOG_MSG(db_DV_SCALER, "DRAM_WRPOSD_MEM1_ADDR   = 0x%08lX (0x%08lX)\n", DRAM_WRPOSD_MEM1_ADDR,DRAM_WRPOSD_MEM1_ADDR * PROAV_DRAM_LINE_BYTES);DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "DRAM_WRPOSD_MEM2_ADDR   = 0x%08lX (0x%08lX)\n", DRAM_WRPOSD_MEM2_ADDR,DRAM_WRPOSD_MEM2_ADDR * PROAV_DRAM_LINE_BYTES);DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "DRAM_WRPOSD_MEM3_ADDR   = 0x%08lX (0x%08lX)\n", DRAM_WRPOSD_MEM3_ADDR,DRAM_WRPOSD_MEM3_ADDR * PROAV_DRAM_LINE_BYTES);
    LOG_MSG(db_DV_SCALER, "DRAM_WRPOSD_MEM4_ADDR   = 0x%08lX (0x%08lX)\n", DRAM_WRPOSD_MEM4_ADDR,DRAM_WRPOSD_MEM4_ADDR * PROAV_DRAM_LINE_BYTES);
    LOG_MSG(db_DV_SCALER, "DRAM_WRPOSD_BMP0_ADDR   = 0x%08lX (0x%08lX)\n", DRAM_WRPOSD_BMP0_ADDR,DRAM_WRPOSD_BMP0_ADDR * PROAV_DRAM_LINE_BYTES);DelayMSec(5);

    LOG_MSG(db_DV_SCALER, "DRAM_WIP_ADDR           = 0x%08lX (0x%08lX)\n", DRAM_WIP_ADDR,DRAM_WIP_ADDR * PROAV_DRAM_LINE_BYTES);DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "DRAM_WOP1_ADDR          = 0x%08lX (0x%08lX)\n", DRAM_WOP1_ADDR,DRAM_WOP1_ADDR * PROAV_DRAM_LINE_BYTES);DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "DRAM_WOP2_ADDR          = 0x%08lX (0x%08lX)\n", DRAM_WOP2_ADDR,DRAM_WOP2_ADDR * PROAV_DRAM_LINE_BYTES);DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "DRAM_WRP_TABLE_ADDR     = 0x%08lX (0x%08lX)\n", DRAM_WRP_TABLE_ADDR,DRAM_WRP_TABLE_ADDR * PROAV_DRAM_LINE_BYTES);DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "DRAM_DBDBLD_BIAS_ADDR0  = 0x%08lX (0x%08lX)\n", DRAM_DBDBLD_BIAS_ADDR0,DRAM_DBDBLD_BIAS_ADDR0 * PROAV_DRAM_LINE_BYTES);DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "DRAM_DBDBLD_BIAS_ADDR1  = 0x%08lX (0x%08lX)\n", DRAM_DBDBLD_BIAS_ADDR1,DRAM_DBDBLD_BIAS_ADDR1 * PROAV_DRAM_LINE_BYTES);DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "DRAM_DBDBLD_BIAS_ADDR2  = 0x%08lX (0x%08lX)\n", DRAM_DBDBLD_BIAS_ADDR2,DRAM_DBDBLD_BIAS_ADDR2 * PROAV_DRAM_LINE_BYTES);DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "DRAM_DBDBLD_BIAS_ADDR3  = 0x%08lX (0x%08lX)\n", DRAM_DBDBLD_BIAS_ADDR3,DRAM_DBDBLD_BIAS_ADDR3 * PROAV_DRAM_LINE_BYTES);DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "DRAM_DBDBLD_GAIN_ADDR0  = 0x%08lX (0x%08lX)\n", DRAM_DBDBLD_GAIN_ADDR0,DRAM_DBDBLD_GAIN_ADDR0 * PROAV_DRAM_LINE_BYTES);DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "DRAM_DBDBLD_GAIN_ADDR1  = 0x%08lX (0x%08lX)\n", DRAM_DBDBLD_GAIN_ADDR1,DRAM_DBDBLD_GAIN_ADDR1 * PROAV_DRAM_LINE_BYTES);DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "DRAM_DBDBLD_GAIN_ADDR2  = 0x%08lX (0x%08lX)\n", DRAM_DBDBLD_GAIN_ADDR2,DRAM_DBDBLD_GAIN_ADDR2 * PROAV_DRAM_LINE_BYTES);DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "DRAM_DBDBLD_GAIN_ADDR3  = 0x%08lX (0x%08lX)\n", DRAM_DBDBLD_GAIN_ADDR3,DRAM_DBDBLD_GAIN_ADDR3 * PROAV_DRAM_LINE_BYTES);DelayMSec(5);

    LOG_MSG(db_DV_SCALER, "DRAM_BANK0_SIZE         = 0x%08lX (0x%08lX)\n", DRAM_BANK0_SIZE,DRAM_BANK0_SIZE * PROAV_DRAM_LINE_BYTES);DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "DRAM_WND1_MEM_SIZE      = 0x%08lX (0x%08lX)\n", MAIN_WND_SIZE,MAIN_WND_SIZE * PROAV_DRAM_LINE_BYTES);DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "DRAM_WND2_MEM_SIZE      = 0x%08lX (0x%08lX)\n", MAIN_WND_SIZE,MAIN_WND_SIZE * PROAV_DRAM_LINE_BYTES);DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "DRAM_WND3_MEM_SIZE      = 0x%08lX (0x%08lX)\n", MAIN_WND_SIZE,MAIN_WND_SIZE * PROAV_DRAM_LINE_BYTES);DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "DRAM_SWND1_MEM_SIZE     = 0x%08lX (0x%08lX)\n", SUB_WND_SIZE,SUB_WND_SIZE * PROAV_DRAM_LINE_BYTES);DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "DRAM_SWND2_MEM_SIZE     = 0x%08lX (0x%08lX)\n", SUB_WND_SIZE,SUB_WND_SIZE * PROAV_DRAM_LINE_BYTES);DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "DRAM_SWND3_MEM_SIZE     = 0x%08lX (0x%08lX)\n", SUB_WND_SIZE,SUB_WND_SIZE * PROAV_DRAM_LINE_BYTES);DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "DRAM_MTN_MEM_SIZE       = 0x%08lX (0x%08lX)\n", MTN_MEM_SIZE,MTN_MEM_SIZE * PROAV_DRAM_LINE_BYTES);DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "DRAM_SMTN_MEM_SIZE      = 0x%08lX (0x%08lX)\n", MTN_MEM_SIZE,MTN_MEM_SIZE * PROAV_DRAM_LINE_BYTES);DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "DRAM_OSD_MEM_SIZE       = 0x%08lX (0x%08lX)\n", OSD_MEM_SIZE,OSD_MEM_SIZE * PROAV_DRAM_LINE_BYTES);DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "DRAM_OSD_FNT_SIZE       = 0x%08lX (0x%08lX)\n", OSD_FNT_SIZE,OSD_FNT_SIZE * PROAV_DRAM_LINE_BYTES);DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "DRAM_OSD_BMP_SIZE       = 0x%08lX (0x%08lX)\n", OSD_BMP_SIZE,OSD_BMP_SIZE * PROAV_DRAM_LINE_BYTES);DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "DRAM_HDR_TABLE_SIZE     = 0x%08lX (0x%08lX)\n", DRAM_HDR_TABLE_SIZE,DRAM_HDR_TABLE_SIZE * PROAV_DRAM_LINE_BYTES);DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "LOGO_CAPTURE_2D_SIZE    = 0x%08lX (0x%08lX)\n", LOGO_CAPTURE_2D_SIZE,LOGO_CAPTURE_2D_SIZE * PROAV_DRAM_LINE_BYTES);DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "LOGO_CAPTURE_3D_SIZE    = 0x%08lX (0x%08lX)\n", LOGO_CAPTURE_3D_SIZE,LOGO_CAPTURE_3D_SIZE * PROAV_DRAM_LINE_BYTES);DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "LOGO_DISPLAY_SIZE       = 0x%08lX (0x%08lX)\n", DRAM_LOGO_CAPTURE_SIZE,LOGO_CAPTURE_2D_SIZE * PROAV_DRAM_LINE_BYTES);DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "DRAM_BANK0_USED_SIZE    = 0x%08lX (0x%08lX)\n", DRAM_BANK0_USED_SIZE,DRAM_BANK0_USED_SIZE * PROAV_DRAM_LINE_BYTES);DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "DRAM_BANK0_FREE_SIZE    = 0x%08lX (0x%08lX)\n", DRAM_BANK0_SIZE - DRAM_BANK0_USED_SIZE, (DRAM_BANK0_SIZE - DRAM_BANK0_USED_SIZE)* PROAV_DRAM_LINE_BYTES);DelayMSec(5);

    LOG_MSG(db_DV_SCALER, "DRAM_BANK1_SIZE         = 0x%08lX (0x%08lX)\n", DRAM_BANK1_USED_SIZE,DRAM_BANK1_USED_SIZE * PROAV_DRAM_LINE_BYTES);DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "DRAM_WIP_SIZE           = 0x%08lX (0x%08lX)\n", DRAM_WIP_SIZE,DRAM_WIP_SIZE * PROAV_DRAM_LINE_BYTES);DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "DRAM_WOP_SIZE           = 0x%08lX (0x%08lX)\n", DRAM_WOP_SIZE,DRAM_WOP_SIZE * PROAV_DRAM_LINE_BYTES);DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "DRAM_WRP_TABLE_SIZE     = 0x%08lX (0x%08lX)\n", DRAM_WRP_TABLE_SIZE,DRAM_WRP_TABLE_SIZE * PROAV_DRAM_LINE_BYTES);DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "DRAM_DBDBLD_MEM_SIZE    = 0x%08lX (0x%08lX)\n", DRAM_DBDBLD_GAIN_SIZE,DRAM_DBDBLD_GAIN_SIZE * PROAV_DRAM_LINE_BYTES);DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "DRAM_WPOSD_MEM1_SIZE    = 0x%08lX (0x%08lX)\n", WPOSD_MEM_SIZE,WPOSD_MEM_SIZE * PROAV_DRAM_LINE_BYTES);DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "DRAM_WPOSD_MEM2_SIZE    = 0x%08lX (0x%08lX)\n", WPOSD_MEM_SIZE,WPOSD_MEM_SIZE * PROAV_DRAM_LINE_BYTES);DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "DRAM_BANK1_USED_SIZE    = 0x%08lX (0x%08lX)\n", DRAM_BANK1_USED_SIZE,DRAM_BANK1_USED_SIZE * PROAV_DRAM_LINE_BYTES);DelayMSec(5);
    LOG_MSG(db_DV_SCALER, "DRAM_BANK1_FREE_SIZE    = 0x%08lX (0x%08lX)\n", DRAM_BANK1_SIZE - DRAM_BANK1_USED_SIZE, (DRAM_BANK1_SIZE - DRAM_BANK1_USED_SIZE)* PROAV_DRAM_LINE_BYTES);DelayMSec(5);
#endif
}
