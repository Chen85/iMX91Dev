#ifndef DV_PROAV_PIXELSHIFTWITH3D_H
#define DV_PROAV_PIXELSHIFTWITH3D_H

#include "dvProAV_Platform.h"
#include "dvProAV_Access.h"
#include "dvProAV_RegTable.h"
#include "dvProAV_Base.h"
#ifndef QT_CPP
    #include "Common.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum{
    eSyncMode_Off,
    eSyncMode_HSyncFreeRun,
    eSyncMode_VSyncReset,
    eSyncMode_Max
}Ex3dSyncMode;

typedef enum{
    e3dSyncOutSwMode_ExternalSync,
    e3dSyncOutSwMode_DdpSyncIn,
    e3dSyncOutSwMode_InternalSync,
    e3dSyncOutSwMode_3dSyncInDelay,
    e3dSyncOutSwMode_Max
}Scl3dSyncOutSwMode;

typedef enum{
    e3dSyncInSwMode_InternalSync,
    e3dSyncInSwMode_ExternalSync,
    e3dSyncInSwMode_Max
}Scl3dSyncInSwMode;

typedef enum{
    e3dSaveMd_LT,
    e3dSaveMd_RT,
    e3dSaveMd_LB,
    e3dSaveMd_RB,

    e3dMd_Max
}Scl3dSaveMd;

typedef enum{
    ePixSftMd_2D = 0x00,
    ePixSftMd_2W4K3D = 0x01,
    ePixSftMd_4W4K3D = 0x02,
    ePixSftMd_4W4K3DPlus = 0x04,
}PixSftMd;

//-------------------------------------------------------------------------------------------------
// 3D Function
//-------------------------------------------------------------------------------------------------

/**
 * @brief External 3D Sync input format Set
 * @param [in] bFormat : format select
 *              -false : 3D L/R format
 *              -true  : Vsync format
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclEx3dSyncInFormatSet(bool bFormat);

/**
 * @brief External 3D Sync Mode Set
 * @param [in] eMode : mode select
 *              -eSyncMode_Off
 *              -eSyncMode_HSyncFreeRun
 *              -eSyncMode_VSyncReset
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclEx3dSyncModeSet(Ex3dSyncMode eMode);

/**
 * @brief Dual pipe 3D mode Set
 * @param [in] bMode : pipe mode
 *              -false : off
 *              -true  : on
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclDualPipe3dModeSet(bool bMode);

/**
 * @brief Frame Sequential 3D mode Set
 * @param [in] bMode : mode select
 *              -false : off
 *              -true  : on
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclFrameSeq3dModeSet(bool bMode);

/**
 * @brief Double frame rate 3D sync in signal enable
 * @param [in] bDouble
 *              -false : 3D sync
 *              -true  : double 3D sync
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclFrameSeq3dsyncDubSet(bool bDouble);

/**
 * @brief LR Sync Free Run Mode Select
 * @param [in] bLrSyncMod
 *              -false : LR sync will reset with input vsync, can't be use for 24 -> 120
 *              -true  : force LR sync to free run
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_Scl3D_LrFreeRun(bool bLrSyncMod);

/**
 * @brief Frame Sequential Input Stack Frame Set
 * @param [in] eEntity : channel selection
 *              -eSclEntity_Main
 *              -eSclEntity_Sub
 * @param [in] ucFrmStackQty : frame sequential input frame stack quantity
 *              -0~3 : x1~x4
 * @param [in] bStkFrmOder : force display order with input stack order,
 *                           Only for even multiple frame rate + frame sync mode,
 *                           for EX: input frame rate 30Hz/60Hz -> output frame rate 120Hz.
 *                           When input stack order is {L0, R0}, set this bit to force output order to be {L0, R0, L0, R0}.
 *                           If this bit is 0, the output order may be {R0, L0, R0, L0}.
 *                           For non-even multiple frame rate, for EX: input frame rate 24Hz/50Hz -> output frame rate 120Hz. This bit must be set to 0.
 *              -false : free run
 *              -true  : force first order with input
 * @param [in] bEnable : mode select
 *              -false : disable
 *              -true  : enable
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclFrameSeqInStkFrmSet(SclEntity eEntity, uint08 ucFrmStackQty, bool bStkFrmOder, bool bEnable);

/**
 * @brief 3D Sync input switch mode set
 * @param [in] eMode : mode select
 *              -e3dSyncInSwMode_InternalSync
 *              -e3dSyncInSwMode_ExternalSync
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_Scl3dSyncInSwModeSet(Scl3dSyncInSwMode eMode);

/**
 * @brief 3D Sync output switch mode set
 * @param [in] eMode : mode select
 *              -e3dSyncOutSwMode_ExternalSync
 *              -e3dSyncOutSwMode_DdpSyncIn
 *              -e3dSyncOutSwMode_InternalSync
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_Scl3dSyncOutSwModeSet(Scl3dSyncOutSwMode eMode);

/**
 * @brief Internal sync polarity control
 * @param [in] bMode : mode select
 *              -false : same
 *              -true : invert
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclIntSyncPolSet(bool bMode);

/**
 * @brief DDP sync polarity control
 * @param [in] bMode : mode select
 *              -false : same
 *              -true : invert
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclDdpSyncPolSet(bool bMode);

/**
 * @brief External sync polarity control
 * @param [in] bMode : mode select
 *              -false : same
 *              -true : invert
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclExtSyncPolSet(bool bMode);

/**
 * @brief 3D external L/R sync input polarity control
 * @param [in] bMode : mode select
 *              -false : same
 *              -true : invert
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_Scl3dExLRSyncInPolSet(bool ucMode);

/**
 * @brief 2D sync Enable / Disable
 * @param [in] bMode : mode select
 *              -false : Disable
 *              -true : Enable
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_Scl2dSyncEn(bool bMode);

/**
 * @brief External 3D Sync mode delay set
 * @param [in] ucDelay : 0~255, Delay ucDelay/256 output frame
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_Scl3dExtSyncDelaySet(uint8 ucDelay);

/**
 * @brief 3D Sync LR Delay Set
 * @param ucDelay : 0~255
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_Scl3dLRDelaySet(uint8 ucDelay);

/**
 * @brief 3D Sync LR Delay Get
 * @return ulDelay : 0~255
 */
uint8 dvProAV_Scl3dLRDelayGet();

/**
 * @brief 3D Sync Internal Generated Sync selection
 * @param [in] bVsMode : mode select
 *              - 0: Align with wrap VS
 *              - 1: Align with Scaler VS
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_Scl3dIntVsModeSel(bool bVsMode);

/**
 * @brief External Sync Detection Count Set
 * @param [in] ucFrame : 0~255 frame, If no external sync signal time exceeds "ucFrame", eVopNoExtSyncIn = 1
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_Scl3dExtSyncDetCountSet(uint8 ucFrame);

/**
 * @brief No 3D External Sync Status
 * @return iNoExtSync
 *          0 : External Sync Input
 *          1 : No External Sync Input
 */
bool dvProAV_Scl3dNoExtSyncStatus(void);

/**
 * @brief 3D L/R Sync output in double mode set
 * @param [in] bEnable :
 *          0 : disable
 *          1 : enable
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_Scl3dLrSyncDouble(bool bEnable);

/**
 * @brief 3D L/R sync output delay mode set
 * @param [in] bDlyOneFrame :
 *          0 : align with frame sync
 *          1 : delay one frame
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_Scl3dLrSyncDlyMod(bool bDlyOneFrame);

/**
 * @brief 4K 3D resolution LR Control signal
 * @param [in] cLrMode :
 *          00: disable
 *          01: 2x
 *          10: 4x
 *          11: 8x
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_Scl_4K3dLrCtl(uint8 cLrMode);

/**
 * @brief 3D glass mode(LLRR) active
 * @param [in] bGlsEn :
 *          0 : disable
 *          1 : enable
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_Scl3dGlsModSet(bool bGlsEn);

/**
 * @brief 3D glass mode active, LR toggle without reset by frame lock
 * @param [in] bGlsWoRstEn :
 *          0 : disable
 *          1 : enable
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_Scl3dGlsModWoRstSet(bool bGlsWoRstEn);

int dvProAV_Scl3dSaveSet(Scl3dSaveMd e3dSaveMd, bool b3dSave);

bool dvProAV_Scl3dSaveGet(Scl3dSaveMd e3dSaveMd);

//-------------------------------------------------------------------------------------------------
// ProAV WCU Pixel shift
//-------------------------------------------------------------------------------------------------
/**
 * @brief Get Sub Frame Mode State
 * @return sub frame mode state
 */
bool dvProAV_PixSftSubFrameModGet(void);

/**
 * @brief WCU Pixel Shift Enable
 * @param [in] bPixSftEn : WCU pixel shift enable
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_PixSftWcuEn(bool bPixSftEn);

/**
 * @brief Get WCU Sub Frame Mode State
 * @return WCU sub frame mode state
 */
bool dvProAV_PixSftWcuSubFrameModGet(void);

/**
 * @brief dvProAV_PixSft3dMdSel
 * @param [in] ePixSftMd
 *              - ePixSftMd_2D = 0x00,
 *              - ePixSftMd_2W4K3D = 0x01,
 *              - ePixSftMd_4W4K3D = 0x02,
 *              - ePixSftMd_4W4K3DPlus = 0x04,
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_PixSft3dMdSel(PixSftMd ePixSftMd);

/**
 * @brief WCU 4-way 4K 3D plus mode (B+ mode)
 * @param [in] bPixSftEn
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_PixSftBplusMdEn(bool bBplusMdEn);

/**
 * @brief WCU 4-way 4K 3D mode Enable
 * @param [in] b4W3DEn : WCU 4-way 4K 3D mode enable
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_PixSft4W3DModEn(bool b4W3DEn);


/**
 * @brief WCU 2-way 4K 3D mode Enable
 * @param [in] b2W3DEn : WCU 2-way 4K 3D mode enable
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_PixSft2W3DModEn(bool b2W3DEn);

/**
 * @brief WCU Pixel Shift Calibration Pattern Enable
 * @param [in] bPixSftCalPatnEn : WCU pixel shift Calibration Pattern enable
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_PixSftCalPatnEn(bool bPixSftCalPatnEn);

/**
 * @brief WCU pixel shift sub-frame Set
 * @param [in] ucFrm1 : 0~3
 * @param [in] ucFrm2 : 0~3
 * @param [in] ucFrm3 : 0~3
 * @param [in] ucFrm4 : 0~3
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_PixSftSubFrameSet(uint8 ucFrm1, uint8 ucFrm2, uint8 ucFrm3, uint8 ucFrm4);

/**
 * @brief dvProAV_PixSft3dSaveSet
 * @param [in] e3dSaveMd :
 * @param [in] b3dSave :
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_PixSft3dSaveSet(Scl3dSaveMd e3dSaveMd, bool b3dSave);

/**
 * @brief dvProAV_PixSft3dSaveGet
 * @param [in] e3dSaveMd :
 * @return b3dSave data
 */
bool dvProAV_PixSft3dSaveGet(Scl3dSaveMd e3dSaveMd);

/**
 * @brief DDP Delay Set
 * @param [in] ucDlyFrameNub : 0~3 (unit : number of sub-frame)
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_PixSftDdpDlySet(uint8 ucDlyFrameNub);

/**
 * @brief WCU Pixel shift warp to DRAM pixel location map Set
 * @param [in] ucElep : 0~3
 * @param [in] ucElop : 0~3
 * @param [in] ucOlep : 0~3
 * @param [in] ucOlop : 0~3
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_PixSftLocSet(uint8 ucElep, uint8 ucElop, uint8 ucOlep, uint8 ucOlop);

/**
 * @brief WCU Pixel shift Actuator position sub-frame Set
 * @param [in] ucAps1 : 0~3
 * @param [in] ucAps2 : 0~3
 * @param [in] ucAps3 : 0~3
 * @param [in] ucAps4 : 0~3
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_PixSftApsSet(uint8 ucAps1, uint8 ucAps2, uint8 ucAps3, uint8 ucAps4);

/**
 * @brief even-line even-pix Test Pattern Set
 * @param [in] bLFrame : 0: RFrame, 1: LFrame
 * @param [in] bWhiteTp : 0: black, 1: white
 * @param [in] bElEpEn : ElEp test pattern enable
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_PixSft4k3DTpfElEpSet(bool bLFrame, bool bWhiteTp, bool bElEpEn);

/**
 * @brief even-line odd-pix Test Pattern Set
 * @param [in] bLFrame : 0: RFrame, 1: LFrame
 * @param [in] bWhiteTp : 0: black, 1: white
 * @param [in] bElOpEn : ElOp test pattern enable
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_PixSft4k3DTpfElOpSet(bool bLFrame, bool bWhiteTp, bool bElOpEn);

/**
 * @brief odd-line even-pix Test Pattern Set
 * @param [in] bLFrame : 0: RFrame, 1: LFrame
 * @param [in] bWhiteTp : 0: black, 1: white
 * @param [in] bOlEpEn : OlEp test pattern enable
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_PixSft4k3DTpfOlEpSet(bool bLFrame, bool bWhiteTp, bool bOlEpEn);

/**
 * @brief odd-line odd-pix Test Pattern Set
 * @param [in] bLFrame : 0: RFrame, 1: LFrame
 * @param [in] bWhiteTp : 0: black, 1: white
 * @param [in] bOlOpEn : OlOp test pattern enable
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_PixSft4k3DTpfOlOpSet(bool bLFrame, bool bWhiteTp, bool bOlOpEn);

/**
 * @brief 3rd and 4th loop config enable
 * @param [in] bLr2ndEn
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_PixSft2w3dLp3_4En(bool bLp3_4En);

/**
 * @brief 2nd loop config enable
 * @param [in] bLr2ndEn
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_PixSft4k3dLp2ndEn(bool bLr2ndEn);

/**
 * @brief 2nd loop reverted config enable
 * @param [in] bLr2ndReverEn
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_PixSft4k3dLp2ndReverEn(bool bLr2ndReverEn);

/**
 * @brief R first for wfo(warp out -> DRAM) set
 * @param [in] bMode 0: LRLR, 1: RLRL
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_PixSft4k3dR1stWfoSet(bool bMode);

/**
 * @brief Start index of 2nd loop
 * @param [in] u8StrLndex : 0 ~ 3
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_PixSftStrLndex2ndSet(uint8 u8StrLndex);

/**
* @brief Start index of 4th loop
* @param [in] u8StrLndex : 0 ~ 3
* @return status : rcSUCCESS / rcERROR / rcINVALID
*/
int dvProAV_PixSftStrLndex4thSet(uint8 u8StrLndex);

/**
* @brief Start index of 3rd loop
* @param [in] u8StrLndex : 0 ~ 3
* @return status : rcSUCCESS / rcERROR / rcINVALID
*/
int dvProAV_PixSftStrLndex3rdSet(uint8 u8StrLndex);


int dvProAV_PixSft_3D_Setting(uint8 ucPixSften, uint8 uc4Way, uint8 uc2Way);

/**
 * @brief dvProAV_PixSftWcuLr4k3DCfgSet
 * @param [in] u8B4k3dLp2ndRv
 * @param [in] u8B4k3dLp3_4En
 * @param [in] u8B4k3dLp2ndEn
 * @param [in] u8B4k3dWfoR1st
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_PixSftWcuLr4k3DCfgSet(uint8 u8B4k3dLp2ndRv, uint8 u8B4k3dLp3_4En, uint8 u8B4k3dLp2ndEn, uint8 u8B4k3dWfoR1st);

//-------------------------------------------------------------------------------------------------
// Dual Pipe 4K 3D
//-------------------------------------------------------------------------------------------------
/**
 * @brief Actuator control By Scaler/XPR Select
 * @param [in] bMod :
 *              false : by scaler
 *              true : by TI XPR
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_PixSftModSel(bool bMod);

/**
 * @brief Scaler Output Sub-Frame Mode Get
 * @return ulMod :
 *              -false : scaler is complete frame
 *              -true : scaler is sub-frame split
 */
bool dvProAV_PixSftModGet(void);

/**
 * @brief Dual-Pipe 4K3D B' Mode Control
 * @param [in] cDpBprnMd :
 *              -false : disable 4K3D B' mode
 *              -true : enable 4K3D B' mode
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_4K3dDpBprnMdCtl(bool bDpBprnMd);

/**
 * @brief Dual-Pipe 4K3D B' Mode: Test Mode Sub Black
 * @param [in] bDpBprnSubBlk :
 *              -false : disable 4K3D B' sub black
 *              -true : enable 4K3D B' sub black
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_4K3dDpBprnSubBlkTest(bool bDpBprnSubBlk);

/**
 * @brief Dual-Pipe 4K3D B' Mode: Test Mode Main Black
 * @param [in] bDpBprnMainBlk :
 *              -false : disable 4K3D B' main black
 *              -true : enable 4K3D B' main black
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_4K3dDpBprnMainBlkTest(bool bDpBprnMainBlk);

/**
 * @brief Dual-Pipe 4K3D B' Mode: Main Select Odd Pixel at line 0
 * @param [in] bDpBprnMsoEn :
 *              -false : disable 4K3D B' sub black
 *              -true : enable 4K3D B' sub black
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_4K3dDpBprnMsoTest(bool bDpBprnMsoEn);

/**
 * @brief XPR calibration pattern enable for scaler only (without warping)
 * @param [in] u8StrLndex
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_4K3dXprCalEn(bool bXprCalEn);

/**
 * @brief 4K3D Sub-Frame Index Order Set
 * @param [in] u8Odr1 : 0 ~ 3, 4K3D sub-frame index order of 1st sub-frame
 * @param [in] u8Odr2 : 0 ~ 3, 4K3D sub-frame index order of 2nd sub-frame
 * @param [in] u8Odr3 : 0 ~ 3, 4K3D sub-frame index order of 3rd sub-frame
 * @param [in] u8Odr4 : 0 ~ 3, 4K3D sub-frame index order of 4th sub-frame
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_4K3dVopSubFrmIndOdrSet(uint8 u8Odr1, uint8 u8Odr2, uint8 u8Odr3, uint8 u8Odr4);

/**
 * @brief 4K3D Actuator Position Order Set
 * @param [in] u8Odr1 : 0 ~ 3, 4K3D actuator position order of 1st sub-frame
 * @param [in] u8Odr2 : 0 ~ 3, 4K3D actuator position order of 2nd sub-frame
 * @param [in] u8Odr3 : 0 ~ 3, 4K3D actuator position order of 3rd sub-frame
 * @param [in] u8Odr4 : 0 ~ 3, 4K3D actuator position order of 4th sub-frame
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_4K3dVopActuatorPosOdrSet(uint8 u8Odr1, uint8 u8Odr2, uint8 u8Odr3, uint8 u8Odr4);

//-------------------------------------------------------------------------------------------------
// Debug func
//-------------------------------------------------------------------------------------------------
/**
 * @brief dvProAV_WcuDbgClr
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_WcuDbgClr(void);

/**
 * @brief dvProAV_WcuDbgVsSel
 * @param [in] bWop : false / true = Wip / Wop
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_WcuDbgVsSel(bool bWop);

/**
 * @brief dvProAV_WcuDbgVtotalMaxGet
 * @return u16Vtotal : Vtotal Max
 */
uint16 dvProAV_WcuDbgVtotalMaxGet(void);

/**
 * @brief dvProAV_WcuDbgVtotalMinGet
 * @return u16Vtotal : Vtotal Min
 */
uint16 dvProAV_WcuDbgVtotalMinGet(void);

#ifdef __cplusplus
}
#endif

#endif // DV_PROAV_PIXELSHIFTWITH3D_H
