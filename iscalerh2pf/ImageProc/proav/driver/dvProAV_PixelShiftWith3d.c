#include "dvProAV_PixelShiftWith3d.h"

//define IN_FILE_DEBUG
#ifndef IN_FILE_DEBUG
    #undef DBMSG
    #define DBMSG(...) do{}while(0)
#endif


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
int dvProAV_SclEx3dSyncInFormatSet(bool bFormat)
{
    return dvProAV_AccessWrite(eVopEsType, bFormat);
}

/**
 * @brief External 3D Sync Mode Set
 * @param [in] eMode : mode select
 *              -eSyncMode_Off
 *              -eSyncMode_HSyncFreeRun
 *              -eSyncMode_VSyncReset
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclEx3dSyncModeSet(Ex3dSyncMode eMode)
{
    if(eMode >= eSyncMode_Max)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }
    return dvProAV_AccessWrite(eVopEsMd, eMode);
}

/**
 * @brief Dual pipe 3D mode Set
 * @param [in] bMode : pipe mode
 *              -false : off
 *              -true  : on
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclDualPipe3dModeSet(bool bMode)
{
    return dvProAV_AccessWrite(eVopDualPipe, bMode);
}

/**
 * @brief Frame Sequential 3D mode Set
 * @param [in] bMode : mode select
 *              -false : off
 *              -true  : on
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclFrameSeq3dModeSet(bool bMode)
{
    return dvProAV_AccessWrite(eVopFrSeq, bMode);
}

/**
 * @brief Double frame rate 3D sync in signal enable
 * @param [in] bDouble
 *              -false : 3D sync
 *              -true  : double 3D sync
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclFrameSeq3dsyncDubSet(bool bDouble)
{
    return dvProAV_AccessWrite(eVopSync3DDubEn, bDouble);
}

/**
 * @brief LR Sync Free Run Mode Select
 * @param [in] bLrSyncMod
 *              -false : LR sync will reset with input vsync, can't be use for 24 -> 120
 *              -true  : force LR sync to free run
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_Scl3D_LrFreeRun(bool bLrSyncMod)
{
    return dvProAV_AccessWrite(eVopLrFreeRun, bLrSyncMod);
}

/**
 * @brief Frame Sequential Input  Stack Frame Set
 * @param [in] eEntity : channel selection
 *              -eSclEntity_Main
 *              -eSclEntity_Sub
 * @param [in] ucFrmStackQty : frame sequential input stack frame quantity
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
int dvProAV_SclFrameSeqInStkFrmSet(SclEntity eEntity, uint08 ucFrmStackQty, bool bStkFrmOder, bool bEnable)
{
    int status = rcSUCCESS;
    if((eEntity >= eSclEntity_Max))
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }
    if(ucFrmStackQty > 3)
        ucFrmStackQty = 3;

    status &= dvProAV_AccessWrite((eEntity == eSclEntity_Main)? eIpmStkFmSet : eIpsStkFmSet, ucFrmStackQty);
    status &= dvProAV_AccessWrite((eEntity == eSclEntity_Main)? eIpmStkFmFsOdr : eIpsStkFmFsOdr, bStkFrmOder);
    status &= dvProAV_AccessWrite((eEntity == eSclEntity_Main)? eIpmStkFmEn : eIpsStkFmEn, bEnable);

    return status;
}

/**
 * @brief 3D Sync input switch mode set
 * @param [in] eMode : mode select
 *              -e3dSyncInSwMode_InternalSync
 *              -e3dSyncInSwMode_ExternalSync
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_Scl3dSyncInSwModeSet(Scl3dSyncInSwMode eMode)
{
    if(eMode >= e3dSyncInSwMode_Max)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }
    return dvProAV_AccessWrite(eVopSync3DInSw, eMode);
}

/**
 * @brief 3D Sync output switch mode set
 * @param [in] eMode : mode select
 *              -e3dSyncOutSwMode_ExternalSync
 *              -e3dSyncOutSwMode_DdpSyncIn
 *              -e3dSyncOutSwMode_InternalSync
 *              -e3dSyncOutSwMode_3dSyncInDelay
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_Scl3dSyncOutSwModeSet(Scl3dSyncOutSwMode eMode)
{
    if(eMode >= e3dSyncOutSwMode_Max)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }
    return dvProAV_AccessWrite(eVopSync3DOutSw, eMode);
}

/**
 * @brief Internal sync polarity control
 * @param [in] bMode : mode select
 *              -false : same
 *              -true : invert
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclIntSyncPolSet(bool bMode)
{
    return dvProAV_AccessWrite(eVopOpm3DIgPol, bMode);
}

/**
 * @brief DDP sync polarity control
 * @param [in] bMode : mode select
 *              -false : same
 *              -true : invert
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclDdpSyncPolSet(bool bMode)
{
    return dvProAV_AccessWrite(eVopOpm3DDpPol, bMode);
}

/**
 * @brief External sync polarity control
 * @param [in] bMode : mode select
 *              -false : same
 *              -true : invert
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclExtSyncPolSet(bool bMode)
{
	//For H60K polarity is invert
    bool bNewMode = false;

    if(bMode)
    {
        bNewMode = false;
    }
    else
    {
        bNewMode = true;
    }

    return dvProAV_AccessWrite(eVopOpm3DEsPol, bNewMode);
}

/**
 * @brief 3D external L/R sync polarity control
 * @param [in] bMode : mode select
 *              -false : same
 *              -true : invert
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_Scl3dExLRSyncInPolSet(bool bMode)
{
    return dvProAV_AccessWrite(eVopEsPol, bMode);
}

/**
 * @brief 2D sync Enable / Disable
 * @param [in] bMode : mode select
 *              -false : Disable
 *              -true : Enable
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_Scl2dSyncEn(bool bMode)
{
    return dvProAV_AccessWrite(eVopSyncO2D, bMode);
}

/**
 * @brief External 3D Sync mode delay set
 * @param [in] ucDelay : 0~255, Delay ucDelay/256 output frame
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_Scl3dExtSyncDelaySet(uint8 ucDelay)
{
    return dvProAV_AccessWrite(eVopEsDlyMd, ucDelay);
}

/**
 * @brief 3D Sync LR Delay Set
 * @param ucDelay : 0~255
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_Scl3dLRDelaySet(uint8 ucDelay)
{
    return dvProAV_AccessWrite(eVopLRDlyMd, ucDelay);
}

/**
 * @brief 3D Sync LR Delay Get
 * @return ulDelay : 0~255
 */
uint8 dvProAV_Scl3dLRDelayGet()
{
    uint32 ulDelay = 0;
    dvProAV_AccessRead(eVopLRDlyMd, &ulDelay);
    return (uint8)(ulDelay & 0xff);
}

/**
 * @brief 3D Sync Internal Generated Sync selection
 * @param [in] bVsMode : mode select
 *              - 0: Align with wrap VS
 *              - 1: Align with Scaler VS
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_Scl3dIntVsModeSel(bool bVsMode)
{
    return dvProAV_AccessWrite(eVopLRSw, bVsMode);
}

/**
 * @brief External Sync Detection Count Set
 * @param [in] ucFrame : 0~255 frame, If no external sync signal time exceeds "ucFrame", eVopNoExtSyncIn = 1
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_Scl3dExtSyncDetCountSet(uint8 ucFrame)
{
    return dvProAV_AccessWrite(eVopSyncDetCnt, ucFrame);
}

/**
 * @brief No 3D External Sync Status
 * @return iNoExtSync
 *          0 : External Sync Input
 *          1 : No External Sync Input
 */
bool dvProAV_Scl3dNoExtSyncStatus(void)
{
    uint32 iNoExtSync = 0;
    dvProAV_AccessRead(eVopNoExtSyncIn, &iNoExtSync);
    return (bool)(iNoExtSync & 0x01);
}

/**
 * @brief 3D L/R Sync output in double mode set
 * @param [in] bEnable :
 *          0 : disable
 *          1 : enable
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 * 3D sync out跟著output走還是input, ex 1080p60 in, true RRRRLLLL, false RL
 */
int dvProAV_Scl3dLrSyncDouble(bool bEnable)
{
    return dvProAV_AccessWrite(eVopFrSeqDb, bEnable);
}

/**
 * @brief 3D L/R sync output delay mode set
 * @param [in] bDlyOneFrame :
 *          0 : align with frame sync
 *          1 : delay one frame
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_Scl3dLrSyncDlyMod(bool bDlyOneFrame)
{
    return dvProAV_AccessWrite(eVopFrSeqDbDly, bDlyOneFrame);
}

/**
 * @brief 4K 3D resolution LR Control signal
 * @param [in] cLrMode :
 *          00: disable
 *          01: 2x
 *          10: 4x
 *          11: 8x
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_Scl_4K3dLrCtl(uint8 cLrMode)
{
    if(cLrMode > 3)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }
    return dvProAV_AccessWrite(eVopr4k_3d, cLrMode);
}

/**
 * @brief 3D glass mode(LLRR) active
 * @param [in] bGlsEn :
 *          0 : disable
 *          1 : enable
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_Scl3dGlsModSet(bool bGlsEn)
{
    return dvProAV_AccessWrite(eVopact_glass_0, bGlsEn);
}

/**
 * @brief 3D glass mode active, LR toggle without reset by frame lock
 * @param [in] bGlsWoRstEn :
 *          0 : disable
 *          1 : enable
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_Scl3dGlsModWoRstSet(bool bGlsWoRstEn)
{
    return dvProAV_AccessWrite(eVopact_glass_1, bGlsWoRstEn);
}

//-------------------------------------------------------------------------------------------------
// ProAV WCU Pixel shift
//-------------------------------------------------------------------------------------------------
/**
 * @brief Get Sub Frame Mode State
 * @return sub frame mode state
 */
bool dvProAV_PixSftSubFrameModGet(void)
{
    return (dvProAV_PixSftWcuSubFrameModGet() || dvProAV_PixSftModGet());
}

/**
 * @brief WCU Pixel Shift For 4-way XPR Enable
 * @param [in] bPixSftEn : WCU pixel shift enable
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_PixSftWcuEn(bool bPixSftEn)
{
    return dvProAV_AccessWrite(eWcuPixShft, bPixSftEn);
}

/**
 * @brief Get WCU Sub Frame Mode State
 * @return WCU sub frame mode state
 */
bool dvProAV_PixSftWcuSubFrameModGet(void)
{
    uint32 u32SclSubFrmMod, u32WarpSubFrmMod;
    dvProAV_AccessRead(eWcuSbfSpt, &u32WarpSubFrmMod);
    return (u32WarpSubFrmMod & 0x01);
}

/**
 * @brief dvProAV_PixSft3dMdSel
 * @param [in] ePixSftMd
 *              - ePixSftMd_2D = 0x00,
 *              - ePixSftMd_2W4K3D = 0x01,
 *              - ePixSftMd_4W4K3D = 0x02,
 *              - ePixSftMd_4W4K3DPlus = 0x04,
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_PixSft3dMdSel(PixSftMd ePixSftMd)
{
    int status = rcSUCCESS;
    if((ePixSftMd != ePixSftMd_2W4K3D) && (ePixSftMd != ePixSftMd_4W4K3D) && (ePixSftMd != ePixSftMd_4W4K3DPlus))
        ePixSftMd = ePixSftMd_2D;

    status &= dvProAV_PixSftBplusMdEn((ePixSftMd & ePixSftMd_4W4K3DPlus) >> 2);
    status &= dvProAV_PixSft4W3DModEn((ePixSftMd & ePixSftMd_4W4K3D) >> 1);
    status &= dvProAV_PixSft2W3DModEn(ePixSftMd & ePixSftMd_2W4K3D);
    return status;
}

/**
 * @brief WCU 4-way 4K 3D Plus Mode Enable (B+ mode)
 * @param [in] bPixSftEn : WCU 4-way 4K 3D plus mode enable
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_PixSftBplusMdEn(bool bBplusMdEn)
{
    return dvProAV_AccessWrite(eWcuLr4wPs3D, bBplusMdEn);
}

/**
 * @brief WCU 4-way 4K 3D mode Enable
 * @param [in] b4W3DEn : WCU 4-way 4K 3D mode enable
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_PixSft4W3DModEn(bool b4W3DEn)
{
    return dvProAV_AccessWrite(eWcuLr4w3D, b4W3DEn);
}


/**
 * @brief WCU 2-way 4K 3D mode Enable
 * @param [in] b2W3DEn : WCU 2-way 4K 3D mode enable
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_PixSft2W3DModEn(bool b2W3DEn)
{
    return dvProAV_AccessWrite(eWcuLr2w3D, b2W3DEn);
}

/**
 * @brief WCU Pixel Shift Calibration Pattern Enable
 * @param [in] bPixSftCalPatnEn : WCU pixel shift Calibration Pattern enable
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_PixSftCalPatnEn(bool bPixSftCalPatnEn)
{
    return dvProAV_AccessWrite(eWcuPxSftCalEn, bPixSftCalPatnEn);
}

/**
 * @brief WCU pixel shift sub-frame Set
 * @param [in] ucFrm1 : 0~3
 * @param [in] ucFrm2 : 0~3
 * @param [in] ucFrm3 : 0~3
 * @param [in] ucFrm4 : 0~3
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_PixSftSubFrameSet(uint8 ucFrm1, uint8 ucFrm2, uint8 ucFrm3, uint8 ucFrm4)
{
    int status = rcSUCCESS;
    if((ucFrm1 > 3) || (ucFrm2 > 3) || (ucFrm3 > 3) || (ucFrm4 > 3))
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }

    status &= dvProAV_AccessWrite(REG_WcuWcuPxSftFrmOdrL, ((ucFrm2 << 4) & 0x30) | ucFrm1);
    status &= dvProAV_AccessWrite(REG_WcuWcuPxSftFrmOdrH, ((ucFrm4 << 4) & 0x30) | ucFrm3);
    return status;
}

/**
 * @brief dvProAV_PixSft3dSaveSet
 * @param [in] e3dSaveMd :
 * @param [in] b3dSave :
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_PixSft3dSaveSet(Scl3dSaveMd e3dSaveMd, bool b3dSave)
{
    int status = rcSUCCESS;
    if(e3dSaveMd >= e3dMd_Max)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }
    uint32 uReg = (e3dSaveMd == e3dSaveMd_LT)? eWcuLr2w3DSaveLT : (e3dSaveMd == e3dSaveMd_RT)? eWcuLr2w3DSaveRT :
                  (e3dSaveMd == e3dSaveMd_LB)? eWcuLr2w3DSaveLB : eWcuLr2w3DSaveRB;

    status &= dvProAV_AccessWrite(uReg, b3dSave);
    return status;
}

/**
 * @brief dvProAV_PixSft3dSaveGet
 * @param [in] e3dSaveMd :
 * @return b3dSave data
 */
bool dvProAV_PixSft3dSaveGet(Scl3dSaveMd e3dSaveMd)
{
    int status = rcSUCCESS;
    if(e3dSaveMd >= e3dMd_Max)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }
    uint32 uReg = (e3dSaveMd == e3dSaveMd_LT)? eWcuLr2w3DSaveLT : (e3dSaveMd == e3dSaveMd_RT)? eWcuLr2w3DSaveRT :
                  (e3dSaveMd == e3dSaveMd_LB)? eWcuLr2w3DSaveLB : eWcuLr2w3DSaveRB;
    uint32 uRData = 0;

    status &= dvProAV_AccessRead(uReg, &uRData);
    return (uRData & 0x01);
}

/**
 * @brief DDP Delay Set
 * @param [in] ucDlyFrameNub : 0~3 (unit : number of sub-frame)
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_PixSftDdpDlySet(uint8 ucDlyFrameNub)
{
    return dvProAV_AccessWrite(eWcuDdpDly, ucDlyFrameNub);
}

/**
 * @brief WCU Pixel shift warp to DRAM pixel location map Set
 * @param [in] ucElep : 0~3
 * @param [in] ucElop : 0~3
 * @param [in] ucOlep : 0~3
 * @param [in] ucOlop : 0~3
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_PixSftLocSet(uint8 ucElep, uint8 ucElop, uint8 ucOlep, uint8 ucOlop)
{
    int status = rcSUCCESS;
    if((ucElop > 3) || (ucElep > 3) || (ucOlop > 3) || (ucOlep > 3))
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }

    status &= dvProAV_AccessWrite(REG_WcuPxSftWfoPmapL, ((ucElop << 4) & 0x30) | ucElep);
    status &= dvProAV_AccessWrite(REG_WcuPxSftWfoPmapH, ((ucOlop << 4) & 0x30) | ucOlep);
    return status;
}

/**
 * @brief WCU Pixel shift Actuator position sub-frame Set
 * @param [in] ucAps1 : 0~3
 * @param [in] ucAps2 : 0~3
 * @param [in] ucAps3 : 0~3
 * @param [in] ucAps4 : 0~3
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_PixSftApsSet(uint8 ucAps1, uint8 ucAps2, uint8 ucAps3, uint8 ucAps4)
{
    int status = rcSUCCESS;
    if((ucAps1 > 3) || (ucAps2 > 3) || (ucAps3 > 3) || (ucAps4 > 3))
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }

    status &= dvProAV_AccessWrite(REG_WcuPxSftApsOdrL, ((ucAps2 << 4) & 0x30) | ucAps1);
    status &= dvProAV_AccessWrite(REG_WcuPxSftApsOdrH, ((ucAps4 << 4) & 0x30) | ucAps3);
    return status;
}

/**
 * @brief even-line even-pix Test Pattern Set
 * @param [in] bLFrame : 0: RFrame, 1: LFrame
 * @param [in] bWhiteTp : 0: black, 1: white
 * @param [in] bElEpEn : ElEp test pattern enable
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_PixSft4k3DTpfElEpSet(bool bLFrame, bool bWhiteTp, bool bElEpEn)
{
    int status = rcSUCCESS;
    status &= dvProAV_AccessWrite((bLFrame)? eWcuLr4k3DTpLfElEpPat : eWcuLr4k3DTpRfElEpPat, bWhiteTp);
    status &= dvProAV_AccessWrite((bLFrame)? eWcuLr4k3DTpLfElEpEn : eWcuLr4k3DTpRfElEpPat, bElEpEn);
    return status;
}

/**
 * @brief even-line odd-pix Test Pattern Set
 * @param [in] bLFrame : 0: RFrame, 1: LFrame
 * @param [in] bWhiteTp : 0: black, 1: white
 * @param [in] bElOpEn : ElOp test pattern enable
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_PixSft4k3DTpfElOpSet(bool bLFrame, bool bWhiteTp, bool bElOpEn)
{
    int status = rcSUCCESS;
    status &= dvProAV_AccessWrite((bLFrame)? eWcuLr4k3DTpLfElOpPat : eWcuLr4k3DTpRfElOpPat, bWhiteTp);
    status &= dvProAV_AccessWrite((bLFrame)? eWcuLr4k3DTpLfElOpEn : eWcuLr4k3DTpRfElOpPat, bElOpEn);
    return status;
}

/**
 * @brief odd-line even-pix Test Pattern Set
 * @param [in] bLFrame : 0: RFrame, 1: LFrame
 * @param [in] bWhiteTp : 0: black, 1: white
 * @param [in] bOlEpEn : OlEp test pattern enable
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_PixSft4k3DTpfOlEpSet(bool bLFrame, bool bWhiteTp, bool bOlEpEn)
{
    int status = rcSUCCESS;
    status &= dvProAV_AccessWrite((bLFrame)? eWcuLr4k3DTpLfOlEpPat : eWcuLr4k3DTpRfOlEpPat, bWhiteTp);
    status &= dvProAV_AccessWrite((bLFrame)? eWcuLr4k3DTpLfOlEpEn : eWcuLr4k3DTpRfOlEpPat, bOlEpEn);
    return status;
}

/**
 * @brief odd-line odd-pix Test Pattern Set
 * @param [in] bLFrame : 0: RFrame, 1: LFrame
 * @param [in] bWhiteTp : 0: black, 1: white
 * @param [in] bOlOpEn : OlOp test pattern enable
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_PixSft4k3DTpfOlOpSet(bool bLFrame, bool bWhiteTp, bool bOlOpEn)
{
    int status = rcSUCCESS;
    status &= dvProAV_AccessWrite((bLFrame)? eWcuLr4k3DTpLfOlOpPat : eWcuLr4k3DTpRfOlOpPat, bWhiteTp);
    status &= dvProAV_AccessWrite((bLFrame)? eWcuLr4k3DTpLfOlOpEn : eWcuLr4k3DTpRfOlOpPat, bOlOpEn);
    return status;
}

/**
 * @brief 3rd and 4th loop config enable
 * @param [in] bLp3_4En
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_PixSft2w3dLp3_4En(bool bLp3_4En)
{
    return dvProAV_AccessWrite(eWcuLr4k3dLp3_4En, bLp3_4En);
}

/**
 * @brief 2nd loop config enable
 * @param [in] bLr2ndEn
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_PixSft4k3dLp2ndEn(bool bLr2ndEn)
{
    return dvProAV_AccessWrite(eWcuLr4k3dLp2ndEn, bLr2ndEn);
}

/**
 * @brief 2nd loop reverted config enable
 * @param [in] bLr2ndReverEn
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_PixSft4k3dLp2ndReverEn(bool bLr2ndReverEn)
{
    return dvProAV_AccessWrite(eWcuLr4k3dLp2ndRv, bLr2ndReverEn);
}

/**
 * @brief R first for wfo(warp out -> DRAM) set
 * @param [in] bMode 0: LRLR, 1: RLRL
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_PixSft4k3dR1stWfoSet(bool bMode)
{
    return dvProAV_AccessWrite(eWcuLr4k3dWfoR1st, bMode);
}

/**
 * @brief Start index of 2nd loop
 * @param [in] u8StrLndex : 0 ~ 3
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_PixSftStrLndex2ndSet(uint8 u8StrLndex)
{
    return dvProAV_AccessWrite(eWcuLr4k3dLp2ndStr, u8StrLndex);
}

/**
* @brief Start index of 4th loop
* @param [in] u8StrLndex : 0 ~ 3
* @return status : rcSUCCESS / rcERROR / rcINVALID
*/
int dvProAV_PixSftStrLndex4thSet(uint8 u8StrLndex)
{
    return dvProAV_AccessWrite(eWcuLr4k3dLp4thStr, u8StrLndex);
}

/**
* @brief Start index of 3rd loop
* @param [in] u8StrLndex : 0 ~ 3
* @return status : rcSUCCESS / rcERROR / rcINVALID
*/
int dvProAV_PixSftStrLndex3rdSet(uint8 u8StrLndex)
{
    return dvProAV_AccessWrite(eWcuLr4k3dLp3rdStr, u8StrLndex);
}

int dvProAV_PixSft_3D_Setting(uint8 ucPixSften, uint8 uc4Way, uint8 uc2Way)
{
    int status = rcSUCCESS;
    uint8 data = 0;

    if(uc4Way)
    {
        data = 0x05;
    }
    else if(uc2Way)
    {
        if(ucPixSften)
        {
            data = 0x83;
        }
        else
        {
            data = 0x03;
        }
    }
    else
    {
        if(ucPixSften)
        {
            data = 0x80;
        }
        else
        {
            data = 0x00;
        }
    }

    status &= dvProAV_AccessWrite(REG_WcuCfgA, data);

    return status;
}

/**
 * @brief dvProAV_PixSftWcuLr4k3DCfgSet
 * @param [in] u8B4k3dLp2ndRv
 * @param [in] u8B4k3dLp3_4En
 * @param [in] u8B4k3dLp2ndEn
 * @param [in] u8B4k3dWfoR1st
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_PixSftWcuLr4k3DCfgSet(uint8 u8B4k3dLp2ndRv, uint8 u8B4k3dLp3_4En, uint8 u8B4k3dLp2ndEn, uint8 u8B4k3dWfoR1st)
{
    uint32 data = 0;

    data = (uint32)(((u8B4k3dLp2ndRv << 3) & 0x08) | ((u8B4k3dLp3_4En << 2) & 0x04) | ((u8B4k3dLp2ndEn << 1) & 0x02) | (u8B4k3dWfoR1st & 0x01));

    return dvProAV_AccessWrite(REG_WcuLr4k3DCfg, data);
}

//-------------------------------------------------------------------------------------------------
// Dual Pipe 4K 3D
//-------------------------------------------------------------------------------------------------
/**
 * @brief Actuator control By Scaler/XPR Select
 * @param [in] bMod :
 *              -false : by scaler
 *              -true : by TI XPR
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_PixSftModSel(bool bMod)
{
    return dvProAV_AccessWrite(eDcuActrSw, bMod);
}

/**
 * @brief Scaler Output Sub-Frame Mode Get
 * @return ulMod :
 *              -false : scaler is complete frame
 *              -true : scaler is sub-frame split
 */
bool dvProAV_PixSftModGet(void)
{
    UINT32 ulMod = 0;
    dvProAV_AccessRead(eDcuSclSbfSpt, &ulMod);
    return (bool)ulMod;
}

/**
 * @brief Dual-Pipe 4K3D B' Mode Control
 * @param [in] bDpBprnMd :
 *              -false : disable 4K3D B' mode
 *              -true : enable 4K3D B' mode
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_4K3dDpBprnMdCtl(bool bDpBprnMd)
{
    return dvProAV_AccessWrite(eDcuF3dDpBprnMd, bDpBprnMd);
}

/**
 * @brief Dual-Pipe 4K3D B' Mode: Test Mode Sub Black
 * @param [in] bDpBprnSubBlk :
 *              -false : disable 4K3D B' sub black
 *              -true : enable 4K3D B' sub black
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_4K3dDpBprnSubBlkTest(bool bDpBprnSubBlk)
{
    return dvProAV_AccessWrite(eDcuF3dDpBprnTsmds, bDpBprnSubBlk);
}

/**
 * @brief Dual-Pipe 4K3D B' Mode: Test Mode Main Black
 * @param [in] bDpBprnMainBlk :
 *              -false : disable 4K3D B' main black
 *              -true : enable 4K3D B' main black
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_4K3dDpBprnMainBlkTest(bool bDpBprnMainBlk)
{
    return dvProAV_AccessWrite(eDcuF3dDpBprnTsmdm, bDpBprnMainBlk);
}

/**
 * @brief Dual-Pipe 4K3D B' Mode: Main Select Odd Pixel at line 0
 * @param [in] bDpBprnMsoEn :
 *              -false : disable 4K3D B' sub black
 *              -true : enable 4K3D B' sub black
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_4K3dDpBprnMsoTest(bool bDpBprnMsoEn)
{
    return dvProAV_AccessWrite(eDcuF3dDpBprnMso, bDpBprnMsoEn);
}

/**
 * @brief XPR calibration pattern enable for scaler only (without warping)
 * @param [in] u8StrLndex
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_4K3dXprCalEn(bool bXprCalEn)
{
    return dvProAV_AccessWrite(eDcuXprCalEn, bXprCalEn);
}

/**
 * @brief 4K3D Sub-Frame Index Order Set
 * @param [in] u8Odr1 : 0 ~ 3, 4K3D sub-frame index order of 1st sub-frame
 * @param [in] u8Odr2 : 0 ~ 3, 4K3D sub-frame index order of 2nd sub-frame
 * @param [in] u8Odr3 : 0 ~ 3, 4K3D sub-frame index order of 3rd sub-frame
 * @param [in] u8Odr4 : 0 ~ 3, 4K3D sub-frame index order of 4th sub-frame
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_4K3dVopSubFrmIndOdrSet(uint8 u8Odr1, uint8 u8Odr2, uint8 u8Odr3, uint8 u8Odr4)
{
    uint8 u8OdrData[2] = {(((u8Odr2 << 4) & 0x30) | (u8Odr1 & 0x03)), (((u8Odr4 << 4) & 0x30) | (u8Odr3 & 0x03))};
    return dvProAV_AccessBurstWrite(REG_VopVopF3dSfiOdrL, u8OdrData, sizeof(u8OdrData), BURST_INC_ADDR);
}

/**
 * @brief 4K3D Actuator Position Order Set
 * @param [in] u8Odr1 : 0 ~ 3, 4K3D actuator position order of 1st sub-frame
 * @param [in] u8Odr2 : 0 ~ 3, 4K3D actuator position order of 2nd sub-frame
 * @param [in] u8Odr3 : 0 ~ 3, 4K3D actuator position order of 3rd sub-frame
 * @param [in] u8Odr4 : 0 ~ 3, 4K3D actuator position order of 4th sub-frame
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_4K3dVopActuatorPosOdrSet(uint8 u8Odr1, uint8 u8Odr2, uint8 u8Odr3, uint8 u8Odr4)
{
    uint8 u8OdrData[2] = {(((u8Odr2 << 4) & 0x30) | (u8Odr1 & 0x03)), (((u8Odr4 << 4) & 0x30) | (u8Odr3 & 0x03))};
    return dvProAV_AccessBurstWrite(REG_VopVopF3dApsOdrL, u8OdrData, sizeof(u8OdrData), BURST_INC_ADDR);
}

//-------------------------------------------------------------------------------------------------
// Debug func
//-------------------------------------------------------------------------------------------------
/**
 * @brief dvProAV_WcuDbgClr
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_WcuDbgClr(void)
{
    int status = rcSUCCESS;
    status &= dvProAV_AccessWrite(eWcuWopDbgClr, true);
    status &= dvProAV_AccessWrite(eWcuWopDbgClr, false);
    return status;
}

/**
 * @brief dvProAV_WcuDbgVsSel
 * @param [in] bWop : false / true = Wip / Wop
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_WcuDbgVsSel(bool bWop)
{
    return dvProAV_AccessWrite(eWcuWopDbgVsSrc, bWop);
}

/**
 * @brief dvProAV_WcuDbgVtotalMaxGet
 * @return u16Vtotal : Vtotal Max
 */
uint16 dvProAV_WcuDbgVtotalMaxGet(void)
{
    uint32 u32Data = 0;
    uint16 u16Vtotal = 0;

    dvProAV_AccessWrite(eWcuWopDbgSel, 3);
    dvProAV_AccessRead(eWcuWopDbgOut, &u32Data);
    u16Vtotal = (u32Data << 8) & 0xFF00;
    dvProAV_AccessWrite(eWcuWopDbgSel, 2);
    dvProAV_AccessRead(eWcuWopDbgOut, &u32Data);
    u16Vtotal |= u32Data & 0xFF;

    return u16Vtotal;
}

/**
 * @brief dvProAV_WcuDbgVtotalMinGet
 * @return u16Vtotal : Vtotal Min
 */
uint16 dvProAV_WcuDbgVtotalMinGet(void)
{
    uint32 u32Data = 0;
    uint16 u16Vtotal = 0;

    dvProAV_AccessWrite(eWcuWopDbgSel, 1);
    dvProAV_AccessRead(eWcuWopDbgOut, &u32Data);
    u16Vtotal = (u32Data << 8) & 0xFF00;
    dvProAV_AccessWrite(eWcuWopDbgSel, 0);
    dvProAV_AccessRead(eWcuWopDbgOut, &u32Data);
    u16Vtotal |= u32Data & 0xFF;

    return u16Vtotal;
}
