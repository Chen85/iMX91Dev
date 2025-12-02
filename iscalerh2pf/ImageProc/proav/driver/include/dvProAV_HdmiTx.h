#ifndef DV_PROAV_HDMI_TX_H
#define DV_PROAV_HDMI_TX_H

#include "dvProAV_Platform.h"
#include "dvProAV_Base.h"

typedef enum{
    eBitClkRatio_HdmiV1p4 = 0,
    eBitClkRatio_HdmiV2p0
} BitClkRatio;

typedef enum{
    eHdmiMode_DVI = 0,
    eHdmiMode_HDMI
} HdmiMode;

typedef enum{
    eHdmiAVMute_Disable = 0,
    eHdmiAVMute_Enable
} HdmiAVMute;

typedef enum{
    eHDMI_CalibreTx = 0,
    eHDMI_IntelTx,
    eVx1,
    eXpr,

    eTxTypeMax
} TxType;

typedef enum{
    eVx1_4k60_148p5M = 1,       // 3840x2160@60 Lanes:8 (ext_clk = 148.5), Ht:4400,Vt:2250
    eVx1_4k60_150M = 2,         // 3840x2160@60 Lanes:8 (ext_clk = 150), Ht:4560,Vt:2194
    eVx1_4k60_148p5M_Memc = 3,  // 3840x2160@60 Lanes:8 MEMC mode (ext_clk = 148.5), Ht:4400,Vt:2250
    eVx1_4k60_150M_Memc = 4,    // 3840x2160@60 Lanes:8 MEMC mode (ext_clk = 150), Ht:4560,Vt:2194
    eVx1_2k50_60_148p5M = 5,    // 1080p and 1920x1200 @50/60 Lanes:2 (ext_clk = 148.5), (1080p50/60) Ht:2640/2200,Vt:1125, (1920x1200 60/60) Ht:2404/2004, Vt:1235
    eVx1_2k100_120_148p5M = 6,  // 1920x1200 @100/120 Lanes:4 (ext_clk=148.5), Ht:2404/2004, Vt:1235
    eVx1_2k100_120_135M = 7,    // 1080p @100/120 Lanes:4 (ext_clk = 135), Ht:2400/2000, Vt:1125

    eVx1TypeMax
} Vx1Type;


#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief 設HDMI TX IP 重置啟動/或除能
 * @param [in] bEnable
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_HdmiTxIpReset(bool bEnable);

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
int dvProAV_HdmiTxConfigSet(uint32 u32Freq, ColorDepth eColorDepth);

/**
 * @brief dvProAV_HdmiOutputInit
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_HdmiTxDriverInit(void);

/**
 * @brief 切換Calibre Tx / Intel Tx / Vx1
 * @param ucTxType
 * @tonote for 過渡階段測試使用
 */
void dvProAV_TxTypeSet(TxType ucTxType);

/**
 * @brief 讀取Tx Port的Video Clock(Pixel clock/2)
 * @return Tx video Clock
 */
uint32 dvProAV_HdmiTxVideoClkGet(void);

/**
 * @brief 讀取Tx Port的Pixel Clock
 * @return Tx Pixel Clock
 */
uint32 dvProAV_SclTxPixelClkGet(void);

//-------------------------------------------------------------------------------------------------
// Vx1
//-------------------------------------------------------------------------------------------------
/**
 * @brief V-by-one Recalibration
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_Vx1ReCal(void);

/**
 * @brief V-by-one timing change configuration
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_Vx1ChgCfg(uint16 uiHsize, uint16 uiVsize, uint32 uiPxlClk);

#ifdef __cplusplus
}
#endif

#endif // DV_PROAV_HDMI_TX_H
