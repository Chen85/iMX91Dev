#ifndef DV_PROAV_SCALER_H
#define DV_PROAV_SCALER_H

#include "dvProAV_Base.h"
#include "dvProAV_Platform.h"
#include "dvProAV_Access.h"
#include "dvProAV_SclCoeffTable.h"
#include "dvProAV_RegTable.h"
#include "dvProAV_HdmiStandard.h"
#include "dvProAV_SclGammaTable.h"
#include "dvProAV_HdrGammaTable.h"
#ifndef QT_CPP
    #include "dvProAV.h"
#endif

typedef enum{
    eSclBuf_Write,
    eSclBuf_Read,
} SclBufMode;

typedef enum{
    eSclPath_SingleFBL = 0, // Ipm -> Opm, Ops open [fb-less]
    eSclPath_Signle,        // Ipm -> Miu -> Opm, Ops open [fb]
    eSclPath_Pip,           // Ipm & Ips -> Miu -> Opm, Ops open [PIP]
    eSclPath_BackupSw,      // Ipm or Ips -> Opm, Ops open [Backup Switch], only support in frame buffer mode
    eSclPath_PopFBL,        // Ipm -> Opm, Ips -> Miu -> Ops
    eSclPath_Pop,           // Ipm -> Miu -> Opm, Ips -> Miu -> Ops
    eSclPath_SeamlessSw,    // Ipm -> Miu -> Opm, Ips -> Miu -> Ops, [Seamless switch]
    eSclPath_Max
} SclDataPath;

typedef enum{
    eSclColorR = 0,
    eSclColorG,
    eSclColorB,
    eSclColorMax
} SclColor;

typedef enum{
    eFblMode_0 = 0,   //framebuffer-less: VOP hsync follow LbSel to avoid full/empty, vsync align to hsync
    eFblMode_1,       //framebuffer-less: VOP hsync free run with HTotal, vsync align to hsync ( good )
    eFblMode_2,       //framebuffer-less: VIP vsync reset all
    eFblMode_Reserved //framebuffer-less: Reserved
} FblMode;

typedef enum{
    eSclDeIntMode_Bob = 0,   //0: Bob mode
    eSclDeIntMode_Even,      //1: Even field only
    eSclDeIntMode_Odd,       //2: Odd field only
    eSclDeIntMode_Weave,     //3: Weave mode
    eSclDeIntMode_Motion,    //4: Motion adaptive
} SclDeIntMode;

typedef enum{
    eSclAlg_Lanczos = 0,
    eSclAlg_Bilinear,
} SclAlg;

typedef enum{
    eSclPeakingMode_Bypass = 0,
    eSclPeakingMode_Low,
    eSclPeakingMode_Medium,
    eSclPeakingMode_High,
    eSclPeakingMode_Max
} SclPeakingMode;


typedef enum{
    eScalingTap_BlurA,             // L2_tap6_phase64  Blur
    eScalingTap_BlurB,             // L2_tap8_phase64, Blur
    eScalingTap_SharpZero,         // L2_tap4_phase64, Sharp, via zero point
    eScalingTap_BlurZero,          // L3_tap8_phase64, Blur, via zero point
    eScalingTap_Max
} ScalingTap;

typedef enum{
    eSclDlSel_Off      = 0x00, // 0000, S-B-V-H
    eSclDlSel_HSharp   = 0x09, // 1001, S-B-V-H, Download to table Horz. Sharp (for scaling up)
    eSclDlSel_VSharp   = 0x0A, // 1010, S-B-V-H, Download to table Vert. Sharp (for scaling up)
    eSclDlSel_HVSharp  = 0x0B, // 1011, S-B-V-H, Download to table Horz. & Vert. Sharp (for scaling up)
    eSclDlSel_HBlur    = 0x05, // 0101, S-B-V-H, Download to table Horz. Blur (for 1/2 scaling down)
    eSclDlSel_VBlur    = 0x06, // 0110, S-B-V-H, Download to table Vert. Blur (for 1/2 scaling down)
    eSclDlSel_HVBlur   = 0x07, // 0111, S-B-V-H, Download to table Horz. & Vert Blur (for 1/2 scaling down), vop not support "H scaling down <= 1/2" ...
} SclDlSel;

typedef struct
{
    uint16 HSmpSt;    //Vip horizontal sample start
    uint16 HSmpSize;  //Vip horizontal sample dot count
    uint16 VSmpSt;    //Vip vertical sample start
    uint16 VSmpSize;  //Vip vertical sample line count
    uint16 HWrSize;   //Vip horizontal write dot count
    uint16 VWrSize;   //Vip vertical write line count
} SclIpuDim;

typedef struct
{
    uint16 HDspSt;    //Opu horizontal display start
    uint16 HRdSize;   //Opu horizontal read size
    uint16 VDspSt;    //Opu vertical display start
    uint16 VRdSize;   //Opu vertical read size
    uint16 HSize;     //Opu horizontal display size
    uint16 VSize;     //Opu vertical display size
} SclOpuDim;

typedef struct
{
    SclIpuDim Ipu;
    SclOpuDim Opu;
} SclDim;

typedef struct
{
    SyncTmg  Hs;
    SyncTmg  Vs;
    ColorDepth  CD;
} SclVop;

typedef struct
{
    SyncTmg Hs;
    SyncTmg Vs;
} SclTpTmg;

typedef struct
{
    uint16  id;
    uint16  HTotal;      //H Total
    uint16  VTotal;      //V Total
    uint16  HSize;       //H Active size
    uint16  VSize;       //V Active size
    uint16  HStart;      //H Start
    uint16  VStart;      //V Start
    uint16  HSync;       //H Sync Width
    uint16  VSync;       //V Sync Width
    uint16  HPol;        //H Polarity(not used)
    uint16  VPol;        //V Active size
    uint16  FrameRate;   //Frame rate(0.01Hz)
    uint32  PixelClock;  //Pixel clock
} PanelTiming;

typedef struct
{
    uint16 id;
    uint16 HTotal;
    uint16 VTotal;
    uint16 HSize;
    uint16 VSize;
    uint16 HStart;
    uint16 VStart;
    uint16 HSync;        // H Sync Width
    uint16 VSync;        // V Sync Width
    uint16 HPol;
    uint16 VPol;
    uint16 HFreq;        // 0.01kHz
    uint16 VFreq;        // 0.01Hz, frame rate
    uint08 interlace;
} ModeTiming;

typedef enum{
    eInpStatusUnstable     = 0,
    eInpStatusStable       = 1,
    eInpStatusChanged      = 2,
    eInpStatusUpdateFailed = 3,
    eInpStatusMax
} InputPortStatus;

typedef enum{
    plPopML = 0 ,
    plPopMT,
    plPopMR,
    plPopMB,
    plPipBR,
    plPipBL,
    plPipTR,
    plPipTL,
    plSingle,
    plMax
}PanelLayout;

typedef enum{
    GammaMod_Normal,    // main and sub used same gamma
    GammaMod_Single,    // main or sub used gamma
    GammaMod_Mpcd,      // main and sub used different gamma

    GammaModMax
}GammaMod;

typedef struct{
    uint08 ScalingRatio;
    uint08 Level;
    uint08 Upx2;
    uint08 SRatio;
}SharpnessParam;

typedef enum{
    eSclTpDisable      = 0,
    eSclTpHColorBar    = 1,
    eSclTpVColorBar    = 2,
    eSclTpRamp         = 3,
    eSclTpFill         = 4,
    eSclTpChecker      = 5,
    eSclTpRnB          = 6,
    eSclTpWindow       = 7,
    eSclTpCrossHatch   = 8,
    eSclTpSlideBar     = 9,
    eSclTpFrame        = 10,
    eSclTpRgbT32Dot    = 11,
    eSclTpRgbT64Dot5x5 = 12,
    eSclTpRgbT64Dot3x3 = 13,
    eSclTpHStrip       = 14,
    eSclTpVStrip       = 15,
    eSclTpMax,
    // For Default Pattern Set
    eSclTpVRamp        = 16,
    eSclTpCrossHatch2  = 17,
    eSclTpCrossHatch3  = 18,
    eSclTpHRamp        = eSclTpRamp
}TpControl;

typedef enum{
    eSclSyncMode_Off,
    eSclSyncMode_HSyncFreeRun,
    eSclSyncMode_VSyncReset,
    eSclSyncMode_Max
}SclFrameSyncMode;


typedef struct{
    uint08 rampCtrl;
    uint16 rampCnt;
    uint16 limitMin;
    uint16 limitMax;
}TpRamp;

typedef struct{
    uint16 limitMax;
}TpColorBar;

typedef struct {
    uint16 colorR;
    uint16 colorG;
    uint16 colorB;
}TpFill;

typedef struct {
    uint16 colorR;
    uint16 colorG;
    uint16 colorB;
    uint16 hLimitMax;
    uint16 vLimitMax;
}TpChecker;

typedef struct {
    uint16 colorR;
    uint16 colorB;
    uint16 hLimitMax;
}TpRnB;

typedef struct {
    uint16 colorR;
    uint16 colorG;
    uint16 colorB;
    uint16 hLimitMax;
    uint16 vLimitMax;
}TpWindow;

typedef struct {
    uint08 focusCtrl;
    uint16 colorR;
    uint16 colorG;
    uint16 colorB;
    uint16 hLimitMax;
    uint16 vLimitMax;
    uint16 lineWidth;
}TpCrossHatch;

typedef struct {
    uint08 focusCtrl;
    uint16 colorR;
    uint16 colorG;
    uint16 colorB;
    uint16 lineWidth;
}TpCrossHatch2;

typedef struct {
    uint16 colorR;
    uint16 colorG;
    uint16 colorB;
    uint16 hLimitMin;
    uint16 hLimitMax;
    uint16 vLimitMax;
}TpSlideBar;

typedef struct {
    uint16 colorR;
    uint16 colorG;
    uint16 colorB;
    uint08 lineWidth;
}TpFrame;

typedef struct {
    uint16 colorR;
    uint16 colorG;
    uint16 colorB;
    uint08 lineWidth;
}TpRgbT;

typedef struct {
    uint16 colorR;
    uint16 colorG;
    uint16 colorB;
    uint16 limitMax;
}TpStrip;

typedef union{
    TpColorBar ColorBar;
    TpRamp Ramp;
    TpFill Fill;
    TpChecker Checker;
    TpRnB RnB;
    TpWindow Window;
    TpCrossHatch CrossHatch;
    TpSlideBar SlideBar;
    TpFrame Frame;
    TpRgbT RgbT;
    TpStrip Strip;
}TpParam;

typedef struct {
    uint16 LvR;
    uint16 LvG;
    uint16 LvB;
}DetRgbLv;

typedef struct {
    DetRgbLv Max;
    DetRgbLv Min;
}DetLv;


#ifdef __cplusplus
extern "C"
{
#endif

//-------------------------------------------------------------------------------------------------
// PanelTiming & ModeTiming
//-------------------------------------------------------------------------------------------------

#ifdef QT_CPP
/**
 * @brief 讀取ID的mode table value
 * @param [in] id : 傳入Timing Mode ID
 * @return 回傳TIMING_MODE_TABLE value
 */
const ModeTiming *dvProAV_TimingDescGet(uint16 id);

/**
 * @brief 讀取Panel Timing value
 * @param [in] hsize : 傳入Panel Horizontal Size
 * @param [in] vsize : 傳入Panel Vertical size
 * @param [in] farmerate : 傳入Panel farmerate
 * @return 回傳PANEL_TIMING_TABLE value
 */
const PanelTiming *dvProAV_PanelTimingDescGet(uint16 hsize, uint16 vsize, uint16 farmerate);

/**
 * @brief 讀取Vx1 Panel Timing value
 * @param [in] hsize : 傳入Panel Horizontal Size
 * @param [in] vsize : 傳入Panel Vertical size
 * @param [in] farmerate : 傳入Panel farmerate
 * @return VX1_PANEL_TIMING_TABLE value
 */
const PanelTiming *dvProAV_Vx1PanelTimingDescGet(uint16 hsize, uint16 vsize, uint16 farmerate);

/**
 * @brief XPR Panel Timing value get
 * @param [in] hsize : Panel Horizontal Size
 * @param [in] vsize : Panel Vertical size
 * @param [in] farmerate : Panel farmerate
 * @return VX1_PANEL_TIMING_TABLE value
 */
const PanelTiming *dvProAV_XprPanelTimingDescGet(uint16 hsize, uint16 vsize, uint16 farmerate);
#endif /* QT_CPP */

//-------------------------------------------------------------------------------------------------
// Input Bridge
//-------------------------------------------------------------------------------------------------

/**
 * @brief Input bridge Timing get
 * @param [in] sRxPort : Rx Port
 *              -eRxPort_0
 *              -eRxPort_1
 *              -eRxPort_2
 *              -eRxPort_3
 * @param [out] timing : Rx Port Timing information
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclIpbTmgGet(RxPort sRxPort, DetTmg *timing);

/**
 * @brief Input port Timing set
 * @param [in] eEntity : 通道選擇
 *              -eSclEntity_Main
 *              -eSclEntity_Sub
 * @param [in] timing : Rx Port Timing information
 * @param [out] portstatus : eInpStatusStable / eInpStatusChanged / eInpStatusUpdateFailed
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclInBrgTmgSet(SclEntity eEntity, uint08 *portstatus, DetTmg timing);

//--------------------------------------------------------------------------------------
// Input Bridge Source
//--------------------------------------------------------------------------------------

/**
 * @brief Input bridge source Timing set
 * @param [in] sRxPort : Rx Port
 *              -eInputPort0
 *              -eInputPort1
 *              -eInputPort2
 *              -eInputPort3
 * @param [in] timing : Rx Port Timing information
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SrcTimingSet(RxPort sRxPort, DetTmg *timing);

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
int dvProAV_SrcUpdatSet(RxPort sRxPort, bool bUpdatingEn);

/**
 * @brief Input Bridge Source pixel mode set
 * @param [in] port : Rx Port
 *              -eInputPort0
 *              -eInputPort1
 *              -eInputPort2
 *              -eInputPort3
 * @param [in] mode 輸入埠的 pixel mode
 *             - 0x00, 0x01: 444
 *             - 0x02: 422
 *             - 0x03: 420
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SrcPixelModeSet(RxPort sRxPort, uint08 mode);

/**
 * @brief Input Bridge Source pixel mode get
 * @param [in] sRxPort : Rx Port
 *              -eInputPort0
 *              -eInputPort1
 *              -eInputPort2
 *              -eInputPort3
 * @return mode :
 *             - 0x00, 0x01: 444
 *             - 0x02: 422
 *             - 0x03: 420
 */
int dvProAV_SrcPixelModeGet(RxPort sRxPort);

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
int dvProAV_SrcColorSpaceSet(RxPort sRxPort, uint08 mode);

/**
 * @brief Input Bridge Source 3D mode set
 * @param [in] sRxPort : Rx port
 *              -eRxPort_0
 *              -eRxPort_1
 *              -eRxPort_2
 *              -eRxPort_3
 * @param [in] mode : 3D模式
 *              -e3D_Off
 *              -e3D_FramePacking,
 *              -e3D_SideBySide,
 *              -e3D_TopAndBottom
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_Src3DModeSet(RxPort sRxPort, Scl3DMode mode);

/**
 * @brief Input Bridge Source 3D mode get
 * @param [in] sRxPort : Rx port
 *              -eRxPort_0
 *              -eRxPort_1
 *              -eRxPort_2
 *              -eRxPort_3
 * @param [out] mode : 3D模式
 *              -e3D_Off
 *              -e3D_FramePacking,
 *              -e3D_SideBySide,
 *              -e3D_TopAndBottom
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_Src3DModeGet(RxPort sRxPort, Scl3DMode *mode);

/**
 * @brief Input Bridge Source Scaler RGB Range set
 * @param [in] sRxPort :
 *              -eRxPort_0 : Rx0
 *              -eRxPort_1 : Rx1
 *              -eRxPort_2 : Rx2
 *              -eRxPort_3 : Rx3
 * @param [in] bRGBfull : limit range = 0 / full range = 1
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SrcLim2FulSet(RxPort sRxPort, bool bLim2FullEn);

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
int dvProAV_SrcFrameRateRatioSet(RxPort sRxPort, DOUBLE dIptFrameRate, DOUBLE dOptFrameRate, uint16 uiIptVtotal, uint16 uiOptVtotal);

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
int dvProAV_SrcL2fDitherSet(RxPort sRxPort, bool bEnable);
//-------------------------------------------------------------------------------------------------
// Scaler Data Path
//-------------------------------------------------------------------------------------------------

/**
 * @brief 設定 ProAV active / Passthrough FIFO active
 * @param [in] Enable : Disable = 0 / Enable = 1
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclByPassEnableSet(bool Enable);

/**
 * @brief 設定輸入訊號路徑(Datapath)
 * @param [in] VidPath
 *              -ptSingleFBL    //Ipm -> Opm, Ops open
 *              -ptSignle       //Ipm -> Miu -> Opm, Ops open
 *              -ptPip          //Ipm&Ips -> Miu -> Opm, Ops open
 *              -ptPopFBL       //Ipm -> Ops, Ips -> Miu -> Ops
 *              -ptReserved     //Reserved
 *              -ptPop          //Ipm -> Miu -> Opm, Ips -> Miu -> Ops
* @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclDatapathSet(SclDataPath dataPath);

/**
 * @brief 讀取輸入訊號路徑(Datapath)
 * @return ulDatapath :
 *              -eSclPath_SingleFBL     // Ipm -> Opm, Ops open [fb-less]
 *              -eSclPath_Signle        // Ipm -> Miu -> Opm, Ops open [fb]
 *              -eSclPath_Pip           // Ipm & Ips -> Miu -> Opm, Ops open [PIP]
 *              -eSclPath_BackupSw      // Ipm or Ips -> Opm, Ops open [Backup Switch], only support in frame buffer mode
 *              -eSclPath_PopFBL        // Ipm -> Opm, Ips -> Miu -> Ops
 *              -eSclPath_Pop           // Ipm -> Miu -> Opm, Ips -> Miu -> Ops
 *              -eSclPath_SeamlessSw    // Ipm -> Miu -> Opm, Ips -> Miu -> Ops, [Seamless switch]
 */
SclDataPath dvProAV_SclDatapathGet(void);

/**
 * @brief 設定輸入port
 * @param [in] eEntity : 通道選擇
 *              -etMain
 *              -etSub
 * @param [in] Num : Rx port Number
 *              -eInputPort0
 *              -eInputPort1
 *              -eInputPort2
 *              -eInputPort3
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclVipSourceSet(SclEntity eEntity, RxPort Num);

/**
 * @brief 讀取目前輸入port
 * @param [in] eEntity : 通道選擇
 *              -etMain
 *              -etSub
 * @param [in] Num : 讀取port Number
 *              -srcP0
 *              -srcP1
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclVipSourceGet(SclEntity eEntity, RxPort *Num);

/**
 * @brief Get Backup current port
 * @return eEntity : 當前選擇通道
 *          - 0 : use main path
 *          - 1 : use backup path
 */
int dvProAV_SclBackupSourceGet(uint08 *pcEntity);

//-------------------------------------------------------------------------------------------------
// Scaler Input Port Information
//-------------------------------------------------------------------------------------------------

/**
 * @brief Input Port Info Get
 * @param [in] eEntity : 通道選擇
 *              -etMain
 *              -etSub
 * @param [out] portstatus
 *        - 0: unstable
 *        - 1: stable
 *        - 2: changed
 *        - 3: update failed
 * @param [out] currentTiming 目前的Timing
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclInputPortInfoGet(SclEntity eEntity, uint08 *portstatus, DetTmg *currentTiming);

/**
 * @brief 讀取Vip Timing
 * @param [in] eEntity : 通道選擇
 *              -etMain
 *              -etSub
 * @param [out] timing : Vip Timing information
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclVipTmgGet(SclEntity eEntity, DetTmg *timing);

/**
 * @brief Input Port Info clear
 * @param [in] eEntity : 通道選擇
 *              -etMain
 *              -etSub
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclInputPortInfoClear(SclEntity eEntity);

/**
 * @brief 讀取Scaler input port timing detect 是否Stable
 * @param [in] eEntity : 通道選擇
 *              -etMain
 *              -etSub
 * @param [out] stable
 *
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclInputStableGet(SclEntity eEntity, bool *stable);

/**
 * @brief 設定IPU相關參數
 * @param [in] eEntity : 通道選擇
 *              -etMain
 *              -etSub
 * @param [in] dim : Ipu相關參數
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclIpuDimSet(SclEntity eEntity, SclIpuDim dim);

/**
 * @brief 讀取IPU相關參數
 * @param [in] eEntity : 通道選擇
 *              -etMain
 *              -etSub
 * @param [out] dim : Ipu相關參數
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclIpuDimGet(SclEntity eEntity, SclIpuDim *dim);
int dvProAV_SclIpuDimDetGet(SclEntity eEntity, SclIpuDim *dim);

/**
 * @brief 設定IPU/OPU參數同步更新
 * @param [in] eEntity : 通道選擇
 *              -eSclEntity_Main
 *              -eSclEntity_Sub
 * @param [in] bUpdatingEn : 設定參數
 *              -false : 參數設定完成,同步參數至IPU
 *              -true : 參數設定中, 先不同步參數至IPU
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclUpdatingSet(SclEntity eEntity, bool bUpdatingEn);

/**
 * @brief 設定OPU相關參數
 * @param [in] eEntity : 通道選擇
 *              -etMain
 *              -etSub
 * @param [in] dim : Opu相關參數
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclOpuDimSet(SclEntity eEntity, SclOpuDim dim);

/**
 * @brief 讀取OPU相關參數
 * @param [in] eEntity : 通道選擇
 *              -etMain
 *              -etSub
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
int dvProAV_SclOpuDimGet(SclEntity eEntity, SclOpuDim *dim);

/**
 * @brief 設定Vop Timing
 * @param [in] Vop : Vop Timing相關參數
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclVopTmgSet(SclVop Vop);

/**
 * @brief 讀取Vop Timing
 * @param [out] Vop : Vop Timing相關參數
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclVopTmgGet(SclVop *Vop);

/**
 * @brief 設定Opu display start
 * @param [in] eEntity : 通道選擇
 *              -etMain
 *              -etSub
 * @param [in] HDspSt : VOP horizontal display start
 * @param [in] VDspSt : VOP vertical display start
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclOpuDspStSet(SclEntity eEntity, uint16 HDspSt, uint16 VDspSt);

/**
 * @brief vop window off
 * @param [in] eEntity : 通道選擇
 *              -eSclEntity_Main
 *              -eSclEntity_Sub
 * @param [in] bOff :
 *              -false enable window
 *              -true  disable window
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclVopWindowOffSet(SclEntity eEntity, bool bOff);

/**
 * @brief vop window off get
 * @param [in] eEntity : 通道選擇
 *              -eSclEntity_Main
 *              -eSclEntity_Sub
 * @return ulOffStatus : false = enable window / true = disable window
 */
bool dvProAV_SclVopWindowOffGet(SclEntity eEntity);

/**
 * @brief 設定PIP sub window起始位置
 * @param [in] uiHstart : 設定PIP sub window Horizontal起始位置
 * @param [in] uiVstart : 設定PIP sub window Vertical起始位置
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclPipStartPosition(uint16 uiHstart, uint16 uiVstart);

/**
 * @brief Ipu Color Space set
 * @param [in] eEntity :
 *             - etMain
 *             - etSub
 * @param [in] ucMode  color space mode
 *             - 0: Automatic (by eDcuIpmColMd or eDcuIpsColMd)
 *             - 1: Force Off
 *             - 2: Force On (Rec. 709)
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclIpuColorSpaceSet(SclEntity eEntity, uint08 ucMode);

/**
 * @brief Opu Color Space set
 * @param [in] eEntity :
 *             - etMain
 *             - etSub
 * @param [in] ucMode  color space mode
 *             - 0: Automatic (by eDcuIpmColMd or eDcuIpsColMd)
 *             - 4: Force Off
 *             - 5: Rec. 601
 *             - 6: 0x06: Rec. 709
 *             - 7: Rec. 2020
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclOpuColorSpaceSet(SclEntity eEntity, uint08 ucMode);

/**
 * @brief 設定deinterlace mode
 * @param [in] eEntity : 通道選擇
 *              -etMain
 *              -etSub
 * @param [in] DeIntMode : 設定deinterlace mode
 *              -diBob      //0: Bob mode
 *              -diEven     //1: Even field only
 *              -diOdd      //2: Odd field only
 *              -diWave     //3: Weave mode
 *              -diMotion   //4: Motion adaptive
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclDeIntModeSet(SclEntity eEntity, SclDeIntMode DeIntMode);

/**
 * @brief 讀取目前deinterlace mode
 * @param [in] eEntity : 通道選擇
 *              -etMain
 *              -etSub
 * @param [out] DeIntMode : 回傳目前的deinterlace mode
 *              -diBob      //0: Bob mode
 *              -diEven     //1: Even field only
 *              -diOdd      //2: Odd field only
 *              -diWave     //3: Weave mode
 *              -diMotion   //4: Motion adaptive
 * @return  status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclDeIntModeGet(SclEntity eEntity, SclDeIntMode *DeIntMode);

/**
 * @brief 設定MADI threshold
 * @param [in] eEntity : 通道選擇
 *              -etMain
 *              -etSub
 * @param [in] Threshold??
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclMADiThresholdSet(SclEntity eEntity, uint08 *Threshold);

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
int dvProAV_SclInputFreezeSet(SclSrcIn eEntity, bool bEnable);

/**
 * @brief Input port freeze mode get
 * @param [in] eEntity : channel selection
 *              -eSclEntity_Main
 *              -eSclEntity_Sub
 * @return mode
 *             -true : freeze
 *             -false : unfreeze
 */
bool dvProAV_SclInputFreezeGet(SclEntity eEntity);

/**
 * @brief Output/Input Frame Ratio set
 * @param [in] eEntity : 通道選擇
 *              -eSclEntity_Main
 *              -eSclEntity_Sub
 * @param [in] uiIptFrameRate : Input frame rate
 * @param [in] uiOptFrameRate : Output frame rate
 * @param [in] uiIptVsize : Input Vsize
 * @param [in] uiOptVsize : Output Vsize
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclFrameRatioSet(SclEntity eEntity, uint16 uiIptFrameRate, uint16 uiOptFrameRate, uint16 uiIptVsize, uint16 uiOptVsize);

/**
 * @brief Rx channel select at Passthrough/fb-less mode??
 * @param [in] sRxPort : Rx Port
 *              -eRxPort_0 : Rx0
 *              -eRxPort_1 : Rx1
 *              -eRxPort_2 : Rx2
 *              -eRxPort_3 : Rx3
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclFBLClockSourceSet(RxPort sRxPort);

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
int dvProAV_SclAbBufModeSet(SclEntity eEntity, bool bEnable);

//-------------------------------------------------------------------------------------------------
// Gamma
//-------------------------------------------------------------------------------------------------
/**
 * @brief Download the Gamma Table  to Chip
 * @note The gamma table setting is used the Vop Clock, so the setting should after the Vop Clock setting
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclVopGammaSet(bool bSameTable);

/**
 * @brief select gamma mode
 * @param [in] eGammaMod
 *              - GammaMod_Normal
 *              - GammaMod_Single
 *              - GammaMod_Mpcd
 * @return rcSUCCESS or rcERROR
 */
int dvProAV_SclVopGammaModSel(GammaMod eGammaMod);

/**
 * @brief set Single mode gamma Polarity
 * @param [in] eEntity : 通道選擇
 *              -eSclEntity_Main
 *              -eSclEntity_Sub
 * @return rcSUCCESS or rcERROR
 */
int dvProAV_SclVopGammaSinglePolSet(SclEntity eEntity);

/**
 * @brief set MPCD mode gamma Polarity
 * @param [in] bHigherIr : 0= table lower addressed are for IR, 1 = table higher addresses are for IR
 * @return rcSUCCESS or rcERROR
 */
int dvProAV_SclVopGammaMpcdPolSet(bool bHigherIr);

/**
 * @brief Download the dram data to gamma buffer(sram)
 * @param dramAddr DRAM address of gamma data
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclVopGammaFromDram(uint32 dramAddr);

/**
 * @brief Download the dram data to gamma buffer(sram)
 * @param dramAddr DRAM address of gamma data
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclVopGammaFlash2Dram(void);

//-------------------------------------------------------------------------------------------------
// Bias / Gain
//-------------------------------------------------------------------------------------------------
/**
 * @brief Scaler Vip Bias set
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
int dvProAV_SclVipBiasSet(SclEntity eEntity, SclColor eColor, int08 cBias);

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
int dvProAV_SclVipGainSet(SclEntity eEntity, SclColor eColor, uint08 ucGain);

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
int dvProAV_SclVopBiasSet(SclEntity eEntity, SclColor eColor, int08 cBias);

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
int dvProAV_SclVopGainSet(SclEntity eEntity, SclColor eColor, uint08 ucGain);

/**
 * @brief Disable VOP Backgain Sub Window
 * @param [in] bEnable : false / true = enable sub backgain / disable sub backgain
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclVopGainBiasSubOffSet(bool bEnable);

/**
 * @brief VOP Backgain Contrast Set
 * @param [in] eEntity : 通道選擇
 *              -eSclEntity_Main
 *              -eSclEntity_Sub
 * @param [in] uiContrast : 0~1023: (0 ~ 1023) / 512
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclVopBkeContrastSet(SclEntity eEntity, uint16 uiContrast);

/**
 * @brief VOP Backgain Brightness Set
 * @param [in] eEntity : 通道選擇
 *              -eSclEntity_Main
 *              -eSclEntity_Sub
 * @param [in] uiBrightness 0~1023: -512 ~ 511
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclVopBkeBrightnessSet(SclEntity eEntity, uint16 uiBrightness);

//-------------------------------------------------------------------------------------------------
// Scaling mode - Lanczos / Bilinear
//-------------------------------------------------------------------------------------------------
/**
 * @brief 下載(dvProAVSclCoeffTable.c)表列的Scaling 係數到晶片上
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclCoeffTableDl2Chip(void);

/**
 * @brief 設定VIP horizontal scaling mode
 * @param [in] eEntity : 通道選擇
 *              -etMain
 *              -etSub
 * @param [in] Alg : 設定horizontal scaling mode
 *              -saLanczos
 *              -saBilinear
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclVipHsAlgSet(SclEntity eEntity, SclAlg Alg);

/**
 * @brief 讀取VIP horizontal scaling mode
 * @param [in] eEntity : 通道選擇
 *              -etMain
 *              -etSub
 * @param [out] Alg : 回傳horizontal scaling mode
 *              -saLanczos
 *              -saBilinear
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclVipHsAlgGet(SclEntity eEntity, SclAlg *Alg);

/**
 * @brief 設定VIP vertical scaling mode
 * @param [in] eEntity : 通道選擇
 *              -etMain
 *              -etSub
 * @param [in] Alg : 設定vertical scaling mode
 *              -saLanczos
 *              -saBilinear
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclVipVsAlgSet(SclEntity eEntity, SclAlg Alg);

/**
 * @brief 讀取VIP vertical scaling mode
 * @param [in] eEntity : 通道選擇
 *              -etMain
 *              -etSub
 * @param [out] Alg : 讀取vertical scaling mode
 *              -saLanczos
 *              -saBilinear
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclVipVsAlgGet(SclEntity eEntity, SclAlg *Alg);


/**
 * @brief 設定Opu horizontal scaling mode
 * @param [in] eEntity : 通道選擇
 *              -etMain
 *              -etSub
 * @param [in] Alg : 設定Opu horizontal scaling mode
 *              -saLanczos
 *              -saBilinear
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclOpuHsAlgSet(SclEntity eEntity, SclAlg Alg);

/**
 * @brief 讀取Opu horizontal scaling mode
 * @param [in] eEntity : 通道選擇
 *              -etMain
 *              -etSub
 * @param [out] Alg : 讀取Opu horizontal scaling mode
 *              -saLanczos
 *              -saBilinear
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclOpuHsAlgGet(SclEntity eEntity, SclAlg *Alg);

/**
 * @brief 讀取Opu vertical scaling mode
 * @param [in] eEntity : 通道選擇
 *              -etMain
 *              -etSub
 * @param [in] Alg : 設定Opu vertical scaling mode
 *              -saLanczos
 *              -saBilinear
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclOpuVsAlgSet(SclEntity eEntity, SclAlg Alg);

/**
 * @brief 讀取Opu vertical scaling mode
 * @param [in] eEntity : 通道選擇
 *          -etMain
 *          -etSub
 * @param [out] Alg : 讀取Opu vertical scaling mode
 *          -saLanczos
 *          -saBilinear
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclOpuVsAlgGet(SclEntity eEntity, SclAlg *Alg);

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
int dvProAV_SclFrameSyncModeSet(SclEntity eEntity, RxPort sRxPort, SclFrameSyncMode eMode, uint8 ucTimes);
int dvProAV_SclFrameSyncModeGet(uint08* cMode);

/**
 * @brief Frame Sync mode delay set
 * @param [in] ucDelay :
 *              - 0: Delay 12 input lines
 *              - 1-255: Delay 1/256 ~ 255/256 input frame
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclFrameSyncDelaySet(uint8 ucDelay);

/**
 * @brief External PLL clock source selection
 * @param ucSource :
 *              -eRxPort_0 : Rx0
 *              -eRxPort_1 : Rx1
 *              -eRxPort_2 : Rx2
 *              -eRxPort_3 : Rx3
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclExtPllSourceSel(RxPort sRxPort);

/**
 * @brief External PLL clock source get
 * @param [0ut] ucSource :
 *              -eRxPort_0 : Rx0
 *              -eRxPort_1 : Rx1
 *              -eRxPort_2 : Rx2
 *              -eRxPort_3 : Rx3
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclExtPllSourceGet(RxPort *ucSource);

/**
 * @brief dvProAV_SclFrameSyncUpdating
 * @param [in] bUpdating :
 *          - false : Updating done
 *          - true : Updating start
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclFrameSyncUpdating(bool bUpdating);
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
int dvProAV_SclMemcBypassSet(bool bEnable);

/**
 * @brief Memc Status Get
 * @return status : 0=MEMC unstable, 1=MEMC stable
 */
bool dvProAV_SclMemcStatusGet(void);

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
int dvProAV_SclAtsBlankEn(bool bEnable);

/**
 * @brief Anti-smear blank index polarity control
 * @param [in] bHighActive :
 *              - false: low active
 *              - true: high active
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclAtsBlankPolSet(bool bHighActive);

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
int dvProAV_SclAtsBlankmode(uint8 ucMode);

/**
 * @brief Anti-smear Blank Index delay mode
 * @param [in] bDelayH :
 *                  - 0 : Delay 0 output frame
 *                  - 1 : Delay 1 output frame
 * @param [in] ucDelayL : ucDelayL / 256 output frame, total delay = bDelayH + ucDelayL
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclAtsBlankDelaySet(bool bDelayH, uint8 ucDelayL);

//-------------------------------------------------------------------------------------------------
// Screen Off
//-------------------------------------------------------------------------------------------------

/**
 * @brief 設定Opu Screen Off Enable / Disable
 * @param [in] eEntity : 通道選擇
 *              -etMain
 *              -etSub
 * @param [in] enable
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclOpuScreenOffSet(SclEntity eEntity, bool enable);

/**
 * @brief 設定Opu Screen Off 的背景顏色
 * @param [in] eEntity : 通道選擇
 *              -etMain
 *              -etSub
 * @param [in] bgColor
 *              -[2:0] R:G:B
 *               000b: Black, 001b: Blue, 010b: Green, 011b: Cyan(G + B)
 *               100b: Red, 101b: Megenta(R + B), 110b: Yellow(R + G), 111b: White(R + G + B)
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclOpuScreenOffBgColorSet(SclEntity eEntity, uint08 bgColor);

/**
 * @brief 設定Vop Screen Off Enable / Disable
 * @param [in] eEntity : 通道選擇
 *              -etMain
 *              -etSub
 * @param [in] enable
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclVopScreenOffSet(bool enable);

/**
 * @brief 設定Vop Screen Off 的背景顏色
 * @param [in] color : 設定背景顏色
 *              -[2:0] R:G:B
 *               000b: Black, 001b: Blue, 010b: Green, 011b: Cyan(G + B)
 *               100b: Red, 101b: Megenta(R + B), 110b: Yellow(R + G), 111b: White(R + G + B)
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclVopScreenOffBgColorSet(uint08 bgColor);
//-------------------------------------------------------------------------------------------------
// Sharpness
//-------------------------------------------------------------------------------------------------

/**
 * @brief 設定Sharpness初始參數
 * @param [in] eEntity : 通道選擇
 *              -etMain
 *              -etSub
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclSharpnessInit(SclEntity eEntity);

/**
 * @brief 設定Sharpness Enable / Disable
 * @param [in] eEntity : 通道選擇
 *              -etMain
 *              -etSub
 * @param [in] enable : disable = 0 / enable = 1
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclSharpnessEnableSet(SclEntity eEntity, bool enable);

/**
 * @brief Sharpness parameter set
 * @param [in] eEntity : 通道選擇
 *              -etMain
 *              -etSub
 * @param [in] level : Sharpness level
 * @param [in] scalingratio : scaling ratio
 * @param [in] rgbfull : limit range = 0 / full range = 1
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclSharpnessSet(SclEntity eEntity, uint08 level, uint08 scalingratio, bool rgbfull);

/**
 * @brief 設定Sharpness 比重參數
 * @param [in] eEntity : 通道選擇
 *              -etMain
 *              -etSub
 * @param [in] weight : 比重參數
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclSharpnessWeightSet(SclEntity eEntity, uint08 *weight);

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
 *      功能與Sharpness相近，但Sharpness效果更好，不建議使用PeakingMode，
 */
int dvProAV_SclPeakingModeSet(SclEntity eEntity, SclPeakingMode mode);

/**
 * @brief 讀取Peaking Mode
 * @param [in] eEntity : 通道選擇
 *              -eSclEntity_Main
 *              -eSclEntity_Sub
 * @param [out] mode : 讀取Peaking Mode
 *              -spmBypass : bypass
 *              -spmLow    : (1 -4 22 -4 1)/16
 *              -spmMedium : (-1 10 -1)/8
 *              -spmHigh   : (-1 6 -1)/4
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 * @note
 *      功能與Sharpness相近，但Sharpness效果更好，不建議使用PeakingMode
 */
int dvProAV_SclPeakingModeGet(SclEntity eEntity, SclPeakingMode *pmode);

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
int dvProAV_SclTpOutputTimingSet(bool bTpgLocaVop, SclTpTmg timing); // bTpgLocaVop Mode 0: Vip Mode 1:Vop

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
int dvProAV_SclTpControlSet(bool bTpgLocaVop, TpControl control);

/**
 * @brief 設置HColorBar的參數
 * @param [in] bTpgLocaVop : 位置選擇
 *              -0 : vip
 *              -1 : vop
 * @param [in] param : 測試圖形的參數
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclTpHColorBar(bool bTpgLocaVop, TpColorBar param);

/**
 * @brief 設置VColorBar的參數
 * @param [in] bTpgLocaVop : 位置選擇
 *              -0 : vip
 *              -1 : vop
 * @param [in] param : 測試圖形的參數
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclTpVColorBar(bool bTpgLocaVop, TpColorBar param);

/**
 * @brief 設置Ramp的參數
 * @param [in] bTpgLocaVop : 位置選擇
 *              -0 : vip
 *              -1 : vop
 * @param [in] param : 測試圖形的參數
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclTpRamp(bool bTpgLocaVop, TpRamp param);

/**
 * @brief 設置Fill的參數
 * @param [in] bTpgLocaVop : 位置選擇
 *              -0 : vip
 *              -1 : vop
 * @param [in] param : 測試圖形的參數
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclTpFill(bool bTpgLocaVop, TpFill param);

/**
 * @brief 設置Checker的參數
 * @param [in] bTpgLocaVop : 位置選擇
 *              -0 : vip
 *              -1 : vop
 * @param [in] param : 測試圖形的參數
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclTpChecker(bool bTpgLocaVop, TpChecker param);

/**
 * @brief 設置RnB測試圖形的參數
 * @param [in] bTpgLocaVop : 位置選擇
 *              -0 : vip
 *              -1 : vop
 * @param [in] param : 測試圖形的參數
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclTpRnB(bool bTpgLocaVop, TpRnB param);

/**
 * @brief 設置Window測試圖形的參數
 * @param [in] bTpgLocaVop : 位置選擇
 *              -0 : vip
 *              -1 : vop
 * @param [in] param : 測試圖形的參數
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclTpWindow(bool bTpgLocaVop, TpWindow param);

/**
 * @brief 設置CrossHatch測試圖形的參數
 * @param [in] bTpgLocaVop : 位置選擇
 *              -0 : vip
 *              -1 : vop
 * @param [in] param : 測試圖形的參數
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclTpCrossHatch(bool bTpgLocaVop, TpCrossHatch param);

/**
 * @brief 設置Silder Bar測試圖形的參數
 * @param [in] bTpgLocaVop : 位置選擇
 *              -0 : vip
 *              -1 : vop
 * @param [in] param : 測試圖形的參數
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclTpSlideBar(bool bTpgLocaVop, TpSlideBar param);

/**
 * @brief 設置Frame測試圖形的參數
 * @param [in] bTpgLocaVop : 位置選擇
 *              -0 : vip
 *              -1 : vop
 * @param [in] param : 測試圖形的參數
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclTpFrame(bool bTpgLocaVop, TpFrame param);

/**
 * @brief 設置RgbT32Dot測試圖形的參數
 * @param [in] bTpgLocaVop : 位置選擇
 *              -0 : vip
 *              -1 : vop
 * @param [in] param : 測試圖形的參數
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclTpRgbT32Dot(bool bTpgLocaVop, TpRgbT param);

/**
 * @brief 設置RgbT64Dot5x5測試圖形的參數
 * @param [in] bTpgLocaVop : 位置選擇
 *              -0 : vip
 *              -1 : vop
 * @param [in] param : 測試圖形的參數
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclTpRgbT64Dot5x5(bool bTpgLocaVop, TpRgbT param);

/**
 * @brief 設置RgbT64Dot3x3測試圖形的參數
 * @param [in] bTpgLocaVop : 位置選擇
 *              -0 : vip
 *              -1 : vop
 * @param [in] param : 測試圖形的參數
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclTpRgbT64Dot3x3(bool bTpgLocaVop, TpRgbT param);

/**
 * @brief 設置HStrip測試圖形的參數
 * @param [in] bTpgLocaVop : 位置選擇
 *              -0 : vip
 *              -1 : vop
 * @param [in] param : 測試圖形的參數
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclTpHStrip(bool bTpgLocaVop, TpStrip param);

/**
 * @brief 設置VStrip測試圖形的參數
 * @param [in] bTpgLocaVop : 位置選擇
 *              -0 : vip
 *              -1 : vop
 * @param [in] param : 測試圖形的參數
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclTpVStrip(bool bTpgLocaVop, TpStrip param);


/**
 * @brief 顯示Default Test Pattern
 * @param [in] bTpgLocaVop : 位置選擇
 *              -0 : vip
 *              -1 : vop
 * @param [in] patternID : 測試圖形的編號
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
void dvProAV_SclDefaultTestPatternSet(bool bTpgLocaVop, uint08 patternID);


//-------------------------------------------------------------------------------------------------
// Logo Capture
//-------------------------------------------------------------------------------------------------

/**
 * @brief Logo Capture Enable
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclLogoCaptureEn(void);

/**
 * @brief Logo Capture Busy Status Get
 * @return ulBusyState : false = unbusy, true = busy
 */
bool dvProAV_SclLogoCaptureBusyStatusGet(void);

/**
 * @brief Logo Display Enable
 * @param bEnable :
 *          - false : display back ground color
 *          - true : display logo
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclLogoDisplayEn(bool bEnable);

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
int dvProAV_SclFadeEn(bool bEnable);

/**
 * @brief Fade Interupt
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclFadeInt(void);

/**
 * @brief Fade Curve Set
 * @param [in] bEnable :
 *          - false : Linear Curve
 *          - true : Smooth Curve
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclFadeCurveSet(bool bCurve);

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
int dvProAV_SclFadeChCtrl(bool bSubEn, bool bMainEn);

/**
 * @brief Fade Rate Set (Fade Step)
 * @param [in] ucFadeRate :
 *          - 0 : x1
 *          - 1 : x2
 *          - 2 : x3
 *          - 3 : x4
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclFadeRateSet(uint08 ucFadeRate);

/**
 * @brief Fade Time Set, the duration of fade
 * @param [in] ucFadeTime : 0 ~ 15 = 256(frame) * ucTime
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclFadeTimeSet(uint08 ucFadeTime);

/**
 * @brief Get Fade busy state
 * @return ulBusyStatus : 0=unbusy, 1=busy
 */
bool dvProAV_SclFadeBusyGet(void);

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
int dvProAV_SclIrHidEn(bool bEnable);

/**
 * @brief dvProAV_SclIrHidFrameRateSet
 * @param [in] ucRate : 0~255, IR video = ucRatio : 1
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclIrHidFrameRateSet(uint08 ucRatio);

/**
 * @brief Ir Hidden Delay Set
 * @param uiDelay : 0~384, IR pulse delay is uiDelay/128 frame
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclIrHidDlySet(uint16 uiDelay);

/**
 * @brief Ir Hidden Pulse Width Set
 * @param ucPulseWidth : 0~8, IR Pulse Width ucPulseWidth/8 frame
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclIrHidPulseWidSet(uint08 ucPulseWidth);

//-------------------------------------------------------------------------------------------------
// Local Focus
//-------------------------------------------------------------------------------------------------
/**
 * @brief Local Focus LUT Write Enable
 * @param [in] bEnable :
 *              -false : write down
 *              -true : write enable
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclLofTableWrEn(bool bEnable);

/**
 * @brief Local Focus Enable
 * @param [in] bEnable :
 *              -false : use local focus sharpness
 *              -true : use local focus
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclLofLocalFocEn(bool bEnable);

/**
 * @brief Local Focus Shift Set
 * @param [in] ucShift
 *              -0 : no shift
 *              -1 : shift right 2 bits
 *              -2 : shift right 4 bits
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclLofShiftSet(uint08 ucShift);

/**
 * @brief Local Focus Block Set
 * @param [in] ucBlkCntH : 0 ~ 31
 * @param [in] ucBlkCntV : 0 ~ 31
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclLofBlockSet(uint08 ucBlkCntH, uint08 ucBlkCntV);

/**
 * @brief Local Focus LUT Start Address Count Set
 * @param [in] uiStAddrCnt : start address count
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclLofLutStAddrSet(uint16 uiStAddrCnt);

/**
 * @brief Local Focus LUT Data Set
 * @param [in] uiCount : data byte count
 * @param [in] ucData : data point,
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclLofLutDataSet(uint16 uiCount, uint08 *ucData);

//-------------------------------------------------------------------------------------------------
// Real Black
//-------------------------------------------------------------------------------------------------
/**
 * @brief Get RGB Max & Min Level for Real Black
 * @param [out] VopRgbLv : vop rgb max & min level
 * @return rcSUCCESS or rcERROR
 */
int dvProAV_SclRgbLvGet(DetLv *VopRgbLv);

/**
 * @brief Set Real Black Area
 * @param [in] u16RlbStarH : real black area H start
 * @param [in] u16RlbStarV : real black area V start
 * @param [in] u16RlbSizeH : real black area H size
 * @param [in] u16RlbSizeV : real black area V size
 * @return rcSUCCESS or rcERROR
 */
int dvProAV_SclRelBlkAreaSet(uint16 u16RlbStarH, uint16 u16RlbStarV, uint16 u16RlbSizeH, uint16 u16RlbSizeV);

//-------------------------------------------------------------------------------------------------
// Common
//-------------------------------------------------------------------------------------------------
/**
 * @brief FPGA temperature(Celsius) get
 * @return FPGA temperature(Celsius)
 */
UINT16 dvProAV_SclFpgaTempGet(void);
int dvProAV_SclFpgaTempADCRst(void);

int dvProAV_SclFpgaTempThreshold(uint16 uiFpgaTemp);

/**
 * @brief 取得Scaler版本號碼
 * @return Scaler版本號碼
 *         Byte 0 : sub version
 *         Byte 1 : main version
 *         Byte 2 : minor version
 *         Byte 3 : (reserved)
 */
uint32 dvProAV_SclVersionGet(void);

/**
 * @brief 初始化ProAV 的相關設定
 */
void dvProAV_ScalerDriverInit(void);


#ifdef __cplusplus
}
#endif

#endif // DV_PROAV_SCALER_H
