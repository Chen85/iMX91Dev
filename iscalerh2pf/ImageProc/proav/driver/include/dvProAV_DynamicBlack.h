#ifndef DV_PROAV_DYNAMIC_BLACK_H
#define DV_PROAV_DYNAMIC_BLACK_H

#include "dvProAV_Platform.h"
#include "dvProAV_Access.h"
#include "dvProAV_RegTable.h"
#ifndef QT_CPP
    #include "Common.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum{
    eDBlkTbCfg_Off = 0x00,
    eDBlkTbCfg_GainTbWr = 0x01,
    eDBlkTbCfg_GainTbRd = 0x02,
    eDBlkTbCfg_LdTbWr = 0x04,
    eDBlkTbCfg_LdTbRd = 0x08,
} eDBlkTbCfg;

typedef enum{
    eDBlkDbg_PixelGainB0,
    eDBlkDbg_PixelGainB1,
    eDBlkDbg_LdStrength,
    eDBlkDbg_CdfTthreshold,

    eDBlkDbg_Max
} eDBlkDbg;

//-------------------------------------------------------------------------------------------------
// Basic Configuration
//-------------------------------------------------------------------------------------------------
/**
 * @brief Dynamic Black Bypass
 * @param [in] bDbByps : false:normal/true:bypass
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_DblkByps(bool bDbByps);

/**
 * @brief Dynamic Black Manual Enable
 * @param [in] bDbManuEn : false:manu disable/true:manu enable
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_DblkManuEn(bool bDbManuEn);

/**
 * @brief Dynamic Black Enable
 * @param [in] bDbManuEn : false:disable/true:enable
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_DblkEnable(bool bDbEn);

/**
 * @brief Dynamic Black CDF Threshold Set
 * @param [in] ulThreshold : CDF threshold (unit: pixels)
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_DblkCdfThresholdSet(uint32 ulThreshold);

/**
 * @brief Dynamic Black CDF Threshold Gat
 * @param [out] ulThreshold : CDF threshold (unit: pixels) data point
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_DblkCdfThresholdGet(uint32 *ulThreshold);

/**
 * @brief Dynamic Black Border Configuration Left Set
 * @param [in] uiTBorderConfig : Border Configuration Left
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_DblkBorderConfigSet_L(uint16 uiTBorderConfig);

/**
 * @brief Dynamic Black Border Configuration Left Get
 * @param [Out] uiTBorderConfig : Border Configuration Left
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_DblkBorderConfigGet_L(uint16 *uiTBorderConfig);

/**
 * @brief Dynamic Black Border Configuration Right Set
 * @param [in] uiTBorderConfig : Border Configuration Right
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_DblkBorderConfigSet_R(uint16 uiTBorderConfig);

/**
 * @brief Dynamic Black Border Configuration Right Get
 * @param [Out] uiTBorderConfigR : Border Configuration Right
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_DblkBorderConfigGet_R(uint16 *uiTBorderConfig);

/**
 * @brief Dynamic Black Border Configuration Top Set
 * @param [in] uiTBorderConfigT : Border Configuration Top
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_DblkBorderConfigSet_T(uint16 uiTBorderConfigT);

/**
 * @brief Dynamic Black Border Configuration Top Get
 * @param [Out] uiTBorderConfigT : Border Configuration Top
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_DblkBorderConfigGet_T(uint16 *uiTBorderConfig);

/**
 * @brief Dynamic Black Border Configuration Bottom Set
 * @param [in] uiTBorderConfigB : Border Configuration Bottom
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_DblkBorderConfigSet_B(uint16 uiTBorderConfigB);

/**
 * @brief Dynamic Black Border Configuration Bottom Get
 * @param [Out] uiTBorderConfigR : Border Configuration Bottom
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_DblkBorderConfigGet_B(uint16 *uiTBorderConfig);

/**
 * @brief Dynamic Black Fdesat Set
 * @param [in] ucFdesat : Adjustable parameter that varies the saturation of the desired hue (u1.7)
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_DblkFdesatSet(uint8 ucFdesat);

/**
 * @brief Dynamic Black Fdesat Get
 * @param [out] ucFdesat : adjustable parameter that varies the saturation of the desired hue (u1.7)
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_DblkFdesatGet(uint8 *ucFdesat);

/**
 * @brief Dynamic Black Pixel Gain in manual mode Set
 * @param [in] uiPixGain : pixel gain
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_DblkPixGainSet(uint16 uiPixGain);

/**
 * @brief Dynamic Black Pixel Gain in manual mode Get
 * @param [Out] uiPixGain : pixel gain
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_DblkPixGainGet(uint16 *uiPixGain);

/**
 * @brief Dynamic Black LD strength in manual mode Set
 * @param [in] ucLdStrng : LD strength in manual mode
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_DblkLdStrngSet(uint8 ucLdStrng);

/**
 * @brief Dynamic Black LD strength in manual mode Set
 * @param [out] ucLdStrng : LD strength in manual mode
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_DblkLdStrngGet(uint8 *ucLdStrng);

/**
 * @brief Dynamic Black PWM Cycle Set
 * @param [in] uiTBorderConfigB : PWM cycles, FPGA counter clk = 100 MHz. If PWM frequency = 1 kHz, PWM cycles = 100 MHz / 1 kHz = 100k
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_DblkPwmCycSet(uint32 ulPwmCyc);

/**
 * @brief Dynamic Black PWM Cycle Get
 * @param [out] uiTBorderConfigB : PWM cycles, FPGA counter clk = 100 MHz. If PWM frequency = 1 kHz, PWM cycles = 100 MHz / 1 kHz = 100k
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_DblkPwmCycGet(uint32 *ulPwmCyc);

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
int dvProAV_DblkTbCfgSel(eDBlkTbCfg eTbCfg);

/**
 * @brief Dynamic Black Table Data Write
 * @param [in] uiWLen : data length
 * @param [in] ucData : write data point
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_DblkTbDataWrite(uint16 uiWLen, uint8 *ucData);

/**
 * @brief Dynamic Black Table Data Read
 * @param [in] uiRLen : data length
 * @param [out] ucData : read data point
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_DblkTbDataRead(uint16 uiRLen, uint8 *ucData);

//-------------------------------------------------------------------------------------------------
// Histogram
//-------------------------------------------------------------------------------------------------
/**
 * @brief Dynamic Black Number Of Histogram Levels Get
 * @param [out] ucHgByt : number of histogram levels data point
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_DblkHgLvGet(uint8 *ucHgLv);

/**
 * @brief Dynamic Black Byte Number Of Each Histogram Value Get
 * @param [out] ucHgByt : byte number of each histogram value data point
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_DblkHgBytGet(uint8 *ucHgByt);

/**
 * @brief Dynamic Black Histogram Read Enable
 * @param [in] bHgrRdEn : false:disable/true:enable
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_DblkHgrRdEn(bool bHgrRdEn);

/**
 * @brief Dynamic Black Histogram Data Read
 * @param [in] uiRLen : data length
 * @param [out] ucRData : read histogram data point
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_DblkHgrDataRead(uint16 uiRLen, uint8 *ucRData);

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
int dvProAV_DblkDbgSel(eDBlkDbg eDbgSel);

/**
 * @brief Dynamic Black Pixel Gain Get
 * @param [out] uiPixelGain : pixel gain data point
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_DblkDbgPixelGainGet(uint16 *uiPixelGain);

/**
 * @brief Dynamic Black LD Strength Get
 * @param [out] uiPixelGain : LD Strength data point
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_DblkDbgLdStrengthGet(uint8 *ucLdStrength);

/**
 * @brief Dynamic Black CDF Tthreshold Get
 * @param [out] uiPixelGain : CDF Tthreshold data point
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_DblkDbgCdfTthresholdGet(uint8 *ucCdfTthreshold);

#ifdef __cplusplus
}
#endif

#endif // DV_PROAV_DYNAMIC_BLACK_H
