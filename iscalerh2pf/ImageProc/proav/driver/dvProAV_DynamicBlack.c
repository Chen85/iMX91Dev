#include "dvProAV_DynamicBlack.h"

//define IN_FILE_DEBUG
#ifndef IN_FILE_DEBUG
    #undef DBMSG
    #define DBMSG(...) do{}while(0)
#endif

//-------------------------------------------------------------------------------------------------
// Basic Configuration
//-------------------------------------------------------------------------------------------------
/**
 * @brief Dynamic Black Bypass
 * @param [in] bDbByps : false:normal/true:bypass
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_DblkByps(bool bDbByps)
{
    int status = rcSUCCESS;

    status &= dvProAV_AccessWrite(eDBlackDBlackByps, bDbByps);

    return status;
}

/**
 * @brief Dynamic Black Manual Enable
 * @param [in] bDbManuEn : false:manu disable/true:manu enable
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_DblkManuEn(bool bDbManuEn)
{
    int status = rcSUCCESS;

    status &= dvProAV_AccessWrite(eDBlackDBlackManual, bDbManuEn);

    return status;
}

/**
 * @brief Dynamic Black Enable
 * @param [in] bDbManuEn : false:disable/true:enable
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_DblkEnable(bool bDbEn)
{
    int status = rcSUCCESS;

    status &= dvProAV_AccessWrite(eDBlackDBlackEnable, bDbEn);

    return status;
}

/**
 * @brief Dynamic Black CDF Threshold Set
 * @param [in] ulThreshold : CDF threshold (unit: pixels)
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_DblkCdfThresholdSet(uint32 ulThreshold)
{
    int status = rcSUCCESS;
    if(ulThreshold > 0xffffff)
        ulThreshold = 0xffffff;

    status &= dvProAV_AccessWrite(eDBlackCdfThr, ulThreshold);

    return status;
}

/**
 * @brief Dynamic Black CDF Threshold Gat
 * @param [out] ulThreshold : CDF threshold (unit: pixels) data point
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_DblkCdfThresholdGet(uint32 *ulThreshold)
{
    int status = rcSUCCESS;

    status &= dvProAV_AccessRead(eDBlackCdfThr, ulThreshold);

    return status;
}

/**
 * @brief Dynamic Black Border Configuration Left Set
 * @param [in] uiTBorderConfig : Border Configuration Left
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_DblkBorderConfigSet_L(uint16 uiTBorderConfig)
{
    int status = rcSUCCESS;

    status &= dvProAV_AccessWrite(eDBlackBdrHL, uiTBorderConfig);

    return status;
}

/**
 * @brief Dynamic Black Border Configuration Left Get
 * @param [Out] uiTBorderConfig : Border Configuration Left
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_DblkBorderConfigGet_L(uint16 *uiTBorderConfig)
{
    int status = rcSUCCESS;
    uint32 ulRData = 0;

    status &= dvProAV_AccessRead(eDBlackBdrHL, &ulRData);
    *uiTBorderConfig = (ulRData & 0xffff);

    return status;
}

/**
 * @brief Dynamic Black Border Configuration Right Set
 * @param [in] uiTBorderConfig : Border Configuration Right
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_DblkBorderConfigSet_R(uint16 uiTBorderConfig)
{
    int status = rcSUCCESS;

    status &= dvProAV_AccessWrite(eDBlackBdrHR, uiTBorderConfig);

    return status;
}

/**
 * @brief Dynamic Black Border Configuration Right Get
 * @param [Out] uiTBorderConfigR : Border Configuration Right
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_DblkBorderConfigGet_R(uint16 *uiTBorderConfig)
{
    int status = rcSUCCESS;
    uint32 ulRData = 0;

    status &= dvProAV_AccessRead(eDBlackBdrHR, &ulRData);
    *uiTBorderConfig = (ulRData & 0xffff);

    return status;
}

/**
 * @brief Dynamic Black Border Configuration Top Set
 * @param [in] uiTBorderConfigT : Border Configuration Top
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_DblkBorderConfigSet_T(uint16 uiTBorderConfigT)
{
    int status = rcSUCCESS;

    status &= dvProAV_AccessWrite(eDBlackBdrVT, uiTBorderConfigT);

    return status;
}

/**
 * @brief Dynamic Black Border Configuration Top Get
 * @param [Out] uiTBorderConfigT : Border Configuration Top
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_DblkBorderConfigGet_T(uint16 *uiTBorderConfig)
{
    int status = rcSUCCESS;
    uint32 ulRData = 0;

    status &= dvProAV_AccessRead(eDBlackBdrVT, &ulRData);
    *uiTBorderConfig = (ulRData & 0xffff);

    return status;
}

/**
 * @brief Dynamic Black Border Configuration Bottom Set
 * @param [in] uiTBorderConfigB : Border Configuration Bottom
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_DblkBorderConfigSet_B(uint16 uiTBorderConfigB)
{
    int status = rcSUCCESS;

    status &= dvProAV_AccessWrite(eDBlackBdrVB, uiTBorderConfigB);

    return status;
}

/**
 * @brief Dynamic Black Border Configuration Bottom Get
 * @param [Out] uiTBorderConfigR : Border Configuration Bottom
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_DblkBorderConfigGet_B(uint16 *uiTBorderConfig)
{
    int status = rcSUCCESS;
    uint32 ulRData = 0;

    status &= dvProAV_AccessRead(eDBlackBdrVB, &ulRData);
    *uiTBorderConfig = (ulRData & 0xffff);

    return status;
}

/**
 * @brief Dynamic Black Fdesat Set
 * @param [in] ucFdesat : Adjustable parameter that varies the saturation of the desired hue (u1.7)
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_DblkFdesatSet(uint8 ucFdesat)
{
    int status = rcSUCCESS;

    status &= dvProAV_AccessWrite(eDBlackFdesat, ucFdesat);

    return status;
}

/**
 * @brief Dynamic Black Fdesat Get
 * @param [out] ucFdesat : adjustable parameter that varies the saturation of the desired hue (u1.7)
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_DblkFdesatGet(uint8 *ucFdesat)
{
    int status = rcSUCCESS;
    uint32 ulRData = 0;

    status &= dvProAV_AccessRead(eDBlackFdesat, &ulRData);
    *ucFdesat = (ulRData & 0xff);

    return status;
}

/**
 * @brief Dynamic Black Pixel Gain in manual mode Set
 * @param [in] uiPixGain : pixel gain
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_DblkPixGainSet(uint16 uiPixGain)
{
    int status = rcSUCCESS;

    status &= dvProAV_AccessWrite(eDBlackPixGain, uiPixGain);

    return status;
}

/**
 * @brief Dynamic Black Pixel Gain in manual mode Get
 * @param [Out] uiPixGain : pixel gain
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_DblkPixGainGet(uint16 *uiPixGain)
{
    int status = rcSUCCESS;
    uint32 ulRData = 0;

    status &= dvProAV_AccessRead(eDBlackPixGain, &ulRData);
    *uiPixGain = (ulRData & 0xffff);

    return status;
}

/**
 * @brief Dynamic Black LD strength in manual mode Set
 * @param [in] ucLdStrng : LD strength in manual mode
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_DblkLdStrngSet(uint8 ucLdStrng)
{
    int status = rcSUCCESS;

    status &= dvProAV_AccessWrite(eDBlackLdStrng, ucLdStrng);

    return status;
}

/**
 * @brief Dynamic Black LD strength in manual mode Set
 * @param [out] ucLdStrng : LD strength in manual mode
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_DblkLdStrngGet(uint8 *ucLdStrng)
{
    int status = rcSUCCESS;
    uint32 ulRData = 0;

    status &= dvProAV_AccessRead(eDBlackLdStrng, &ulRData);
    *ucLdStrng = (ulRData & 0xff);

    return status;
}

/**
 * @brief Dynamic Black PWM Cycle Set
 * @param [in] uiTBorderConfigB : PWM cycles, FPGA counter clk = 100 MHz. If PWM frequency = 1 kHz, PWM cycles = 100 MHz / 1 kHz = 100k
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_DblkPwmCycSet(uint32 ulPwmCyc)
{
    int status = rcSUCCESS;
    if(ulPwmCyc > 0x7ffffff)
        ulPwmCyc = 0x7ffffff;

    status &= dvProAV_AccessWrite(eDBlackPwmCyc, ulPwmCyc);

    return status;
}

/**
 * @brief Dynamic Black PWM Cycle Get
 * @param [out] uiTBorderConfigB : PWM cycles, FPGA counter clk = 100 MHz. If PWM frequency = 1 kHz, PWM cycles = 100 MHz / 1 kHz = 100k
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_DblkPwmCycGet(uint32 *ulPwmCyc)
{
    int status = rcSUCCESS;

    status &= dvProAV_AccessRead(eDBlackPwmCyc, ulPwmCyc);

    return status;
}

//-------------------------------------------------------------------------------------------------
// Table Access
//-------------------------------------------------------------------------------------------------
/**
 * @brief Dynamic Black Table Config Set
 * @param [in] eTbCfg :
 *                  - eDBlkTbCfg_Off = 0x00
 *                  - eDBlkTbCfg_GainTbWr = 0x01
 *                  - eDBlkTbCfg_GainTbRd = 0x02
 *                  - eDBlkTbCfg_LdTbWr = 0x04
 *                  - eDBlkTbCfg_LdTbRd = 0x08
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_DblkTbCfgSel(eDBlkTbCfg eTbCfg)
{
    int status = rcSUCCESS;
    if((eTbCfg != eDBlkTbCfg_GainTbWr) && (eTbCfg != eDBlkTbCfg_GainTbRd) && (eTbCfg != eDBlkTbCfg_LdTbWr) && (eTbCfg != eDBlkTbCfg_LdTbRd))
        eTbCfg = eDBlkTbCfg_Off;

    status &= dvProAV_AccessWrite(REG_DBlackTbCfg, eTbCfg);

    return status;
}

/**
 * @brief Dynamic Black Table Data Write
 * @param [in] uiWLen : data length
 * @param [in] ucData : write data point
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_DblkTbDataWrite(uint16 uiWLen, uint8 *ucData)
{
    int status = rcSUCCESS;

    status &= dvProAV_AccessBurstWrite(eDBlackTbData, ucData, uiWLen, BURST_FIX_ADDR);

    return status;
}

/**
 * @brief Dynamic Black Table Data Read
 * @param [in] uiRLen : data length
 * @param [out] ucData : read data point
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_DblkTbDataRead(uint16 uiRLen, uint8 *ucData)
{
    int status = rcSUCCESS;

    status &= dvProAV_AccessBurstRead(eDBlackTbData, ucData, uiRLen, BURST_FIX_ADDR);

    return status;
}

//-------------------------------------------------------------------------------------------------
// Histogram
//-------------------------------------------------------------------------------------------------
/**
 * @brief Dynamic Black Number Of Histogram Levels Get
 * @param [out] ucHgByt : number of histogram levels data point
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_DblkHgLvGet(uint8 *ucHgLv)
{
    int status = rcSUCCESS;
    uint32 ulRData = 0;

    status &= dvProAV_AccessRead(eDBlackHgLv, &ulRData);
    *ucHgLv = (ulRData & 0xff);

    return status;
}

/**
 * @brief Dynamic Black Byte Number Of Each Histogram Value Get
 * @param [out] uiHgByt : byte number of each histogram value data point
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_DblkHgBytGet(uint8 *ucHgByt)
{
    int status = rcSUCCESS;
    uint32 ulRData = 0;

    status &= dvProAV_AccessRead(eDBlackHgByt, &ulRData);
    *ucHgByt = (ulRData & 0xff);

    return status;
}

/**
 * @brief Dynamic Black Histogram Read Enable
 * @param [in] bHgrRdEn : false:disable/true:enable
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_DblkHgrRdEn(bool bHgrRdEn)
{
    int status = rcSUCCESS;

    status &= dvProAV_AccessWrite(eDBlackHgrRd, bHgrRdEn);

    return status;
}

/**
 * @brief Dynamic Black Histogram Data Read
 * @param [in] uiRLen : data length
 * @param [out] ucRData : read histogram data point
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_DblkHgrDataRead(uint16 uiRLen, uint8 *ucRData)
{
    int status = rcSUCCESS;

    status &= dvProAV_AccessBurstRead(eDBlackHgrData, ucRData, uiRLen, BURST_FIX_ADDR);

    return status;
}

//-------------------------------------------------------------------------------------------------
// Debug Signel
//-------------------------------------------------------------------------------------------------
/**
 * @brief Dynamic Black Debug Signel Select
 * @param [in] eDbgSel :
 *                  - eDBlkDbg_PixelGainB0 = 0
 *                  - eDBlkDbg_PixelGainB1 = 1
 *                  - eDBlkDbg_LdStrength = 2
 *                  - eDBlkDbg_CdTthreshold = 3
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_DblkDbgSel(eDBlkDbg eDbgSel)
{
    int status = rcSUCCESS;
    if(eDbgSel >= eDBlkDbg_Max)
        return rcERROR;

    status &= dvProAV_AccessWrite(REG_DBlackDbgSel, eDbgSel);

    return status;
}

/**
 * @brief Dynamic Black Pixel Gain Get
 * @param [out] uiPixelGain : pixel gain data point
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_DblkDbgPixelGainGet(uint16 *uiPixelGain)
{
    int status = rcSUCCESS;
    uint32 ulRData = 0;

    status &= dvProAV_DblkDbgSel(eDBlkDbg_PixelGainB1);
    status &= dvProAV_AccessRead(REG_DBlackDbgSig, &ulRData);
    *uiPixelGain = (ulRData << 8) & 0xff00;

    status &= dvProAV_DblkDbgSel(eDBlkDbg_PixelGainB0);
    status &= dvProAV_AccessRead(REG_DBlackDbgSig, &ulRData);
    *uiPixelGain |= (ulRData & 0xff);

    return status;
}

/**
 * @brief Dynamic Black LD Strength Get
 * @param [out] uiPixelGain : LD Strength data point
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_DblkDbgLdStrengthGet(uint8 *ucLdStrength)
{
    int status = rcSUCCESS;
    uint32 ulRData = 0;

    status &= dvProAV_DblkDbgSel(eDBlkDbg_LdStrength);
    status &= dvProAV_AccessRead(REG_DBlackDbgSig, &ulRData);
    *ucLdStrength = (ulRData & 0xff);

    return status;
}

/**
 * @brief Dynamic Black CDF Tthreshold Get
 * @param [out] uiPixelGain : CDF Tthreshold data point
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_DblkDbgCdfTthresholdGet(uint8 *ucCdfTthreshold)
{
    int status = rcSUCCESS;
    uint32 ulRData = 0;

    status &= dvProAV_DblkDbgSel(eDBlkDbg_CdfTthreshold);
    status &= dvProAV_AccessRead(REG_DBlackDbgSig, &ulRData);
    *ucCdfTthreshold = (ulRData & 0xff);

    return status;
}
