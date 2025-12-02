#ifndef DV_PROAV_HDMI_RX_H
#define DV_PROAV_HDMI_RX_H

#include "dvProAV_Base.h"
#include "dvProAV_HdmiStandard.h"

#ifdef __cplusplus
extern "C" {
#endif


#define RX_COLORSPACE_RGB       0x00
#define RX_COLORSPACE_REC601    0x01
#define RX_COLORSPACE_REC709    0x02
#define RX_COLORSPACE_REC2020   0x03
#define RX_COLORSPACE_UNKNOW    0x04

//-------------------------------------------------------------------------------------------------
// HDMI Rx function
//-------------------------------------------------------------------------------------------------

/**
 * @brief 設HDMI RX IP 重置啟動/或除能
 * @param [in] bEnable
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_HdmiRxIpReset(bool bEnable);

/**
 * @brief 傳回所有HDMI RX IP 重置是否完成
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
bool dvProAV_HdmiRxIsIpResetReady(void);

/**
 * @brief 取得輸入埠的VSI資訊
 * @param [in] sRxPort : 輸入埠選擇
 * @param [out] data : VSI Infoframe
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_HdmiRxVsiGet(RxPort sRxPort, VSI_INFOFRAME *data);

/**
 * @brief 取得輸入埠的AVI資訊
 * @param [in] sRxPort : 輸入埠選擇
 * @param [out] data : AVI Infoframe
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_HdmiRxAviGet(RxPort sRxPort, AVI_INFOFRAME *data);


/**
 * @brief 取得輸入埠的HDR資訊
 * @param [in] sRxPort : 輸入埠選擇
 * @param [out] data : HDR Infoframe
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
#ifdef QT_CPP
int dvProAV_HdmiRxHdrGet(RxPort sRxPort, HDR_INFOFRAME *data);
#endif
/**
 * @brief 設定HDMI DeScramble
 * @param [in] sRxPort : Rx Port
 *              -eRxPort_0
 *              -eRxPort_1
 *              -eRxPort_2
 *              -eRxPort_3
 * @param [in] bHDMI2p0 :
                -false: DeScramble disable
                -true: DeScramble enable
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_HdmiRxDeScrambleSet(RxPort sRxPort, bool bDeScrambleEn);

/**
 * @brief 設定HDMI mode 1.4 / 2.0
 * @param [in] sRxPort : Rx Port
 *              -eRxPort_0
 *              -eRxPort_1
 *              -eRxPort_2
 *              -eRxPort_3
 * @param [in] bHDMI2p0 :
                -false: HDMI1.4
                -true: HDMI2.0
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_HdmiRxMode2p0Set(RxPort sRxPort, bool bHDMI2p0En);

/**
 * @brief 讀取HDMI mode 1.4 / 2.0
 * @param [in] sRxport : Rx Port
 *              -eRxPort_0
 *              -eRxPort_1
 *              -eRxPort_2
 *              -eRxPort_3
 * @return bHDMI2p0 :
 *              -false: HDMI1.4
 *              -true: HDMI2.0
 */
bool dvProAV_SclHDMI2p0Get(RxPort sRxPort);

/**
 * @brief 讀取Rx over sampling
 * @param [in] sRxport : Rx Port
 *              -eRxPort_0
 *              -eRxPort_1
 *              -eRxPort_2
 *              -eRxPort_3
 * @return bHDMI2p0 :
 *              -false: non-Over Sampling
 *              -true: Over Sampling
 */
bool dvProAV_SclOverSampleGet(RxPort sRxPort);

/**
 * @brief 設定HDMI Rx Pixel clock
 * @param [in] sRxPort : Rx Port
 *              -eRxPort_0
 *              -eRxPort_1
 *              -eRxPort_2
 *              -eRxPort_3
 * @param [in] uiClk : pixel clock
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_HdmiRxClkSet(RxPort sRxPort, uint16 uiClk);

/**
 * @brief 設定HDMI Rx F/W config enable / disable
 * @param [in] sRxPort : Rx Port
 *              -eRxPort_0
 *              -eRxPort_1
 *              -eRxPort_2
 *              -eRxPort_3
 * @param [in] bRxConfigEn :
 *              -false: Rx F/W config disable
 *              -true : Rx F/W config enable
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_HdmiRxConfigEn(RxPort sRxPort, bool bRxConfigEn);

/**
 * @brief 讀取Hdmi Rx Color Depth
 * @param [in] sRxPort : Rx Port
 *              -eRxPort_0
 *              -eRxPort_1
 *              -eRxPort_2
 *              -eRxPort_3
 * @return Hdmi Rx Color Depth : 8/10/12/16
 */
int dvProAV_HdmiRxColorDepthGet(RxPort sRxPort);

/**
 * @brief Pixel Repet times get
 * @param [in] sRxPort : Rx Port
 *              -eRxPort_0
 *              -eRxPort_1
 *              -eRxPort_2
 *              -eRxPort_3
 * @return Pixel Repet times value
 */
int dvProAV_HdmiRxPixelRepetGet(RxPort sRxPort);

/**
 * @brief 讀取HDMI RX Bit Clock Ratio
 * @param [in] sRxPort : Rx Port
 *              -eRxPort_0
 *              -eRxPort_1
 *              -eRxPort_2
 *              -eRxPort_3
 * @return Bcr : eBitClkRatio_HdmiV1p4 / eBitClkRatio_HdmiV2p0
 */
bool dvProAV_HdmiRxBCRGet(RxPort sRxPort);

/**
 * @brief 讀取HDMI RX TMDS clock
 * @param [in] port : Rx Port
 *              -eRxPort_0
 *              -eRxPort_1
 *              -eRxPort_2
 *              -eRxPort_3
 * @return Rx TMDS Clock
 */
uint32 dvProAV_HdmiRxTmdsClkGet(RxPort sRxPort);

//-------------------------------------------------------------------------------------------------
// Info frame Convert
//-------------------------------------------------------------------------------------------------

/**
 * @brief Convert the Color Info to Pixel Mode
 * @param [in] colorInfo : Rx Color Info
 * @return pixelmode :
 *              - 0x00, 0x01: 444
 *              - 0x02: 422
 *              - 0x03: 420
 */
uint08 dvProAV_HdmiRxColorInfo2PixelMode(uint08 colorInfo);

/**
 * @brief Convert the Color Info to RGB Mode
 * @param [in] colorInfo : Rx Color Info
 * @return RGBMode :
 *              - 0x00: RGB Limit
 *              - 0x01: RGB Full
 */
uint08 dvProAV_HdmiRxColorInfo2RGBMode(uint08 ucColorimetry, uint08 ucRGBMode);

/**
 * @brief Convert the Color Info  to Color Space
 * @param [in] colorInfo : Rx Color Info
 * @return colorspace :
 *              - 0x00: RGB
 *              - 0x01: Rec. 601
 *              - 0x02: Rec. 709
 *              - 0x03: Rec. 2020
 *              - 0x04: Invalid
 */
uint08 dvProAV_HdmiRxColorInfo2ColorSpace(AVI_INFOFRAME *sAVIInfo);

//-------------------------------------------------------------------------------------------------
// HDMI Rx Initial
//-------------------------------------------------------------------------------------------------

/**
 * @brief 初始化HDMI Rx
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_HdmiRxInit(void);

//-------------------------------------------------------------------------------------------------
// 待修正
//-------------------------------------------------------------------------------------------------

/**
 * @brief Convert the VSI Info to HDMI Video Format
 * @param [in] VSIInfo
 * @return HDMIVideoFormat :
 *              - 0: No additional HDMI video format is presented in this packet.
 *              - 1: Extended resolution format (e.g. used for 4K x 2K video) present.
 *              - 2: 3D format indication present.
 */
uint08 dvProAV_SclVSIInfo2HDMIVideoFormat(uint08 *VSIInfo);

/**
 * @brief Convert the VSI Info to 3D mode
 * @param [in] VSIInfo
 * @return uc3Dmode : 3D mode
 *              -eScl3DMode_Off
 *              -eScl3DMode_FramePacking
 *              -eScl3DMode_SideBySide
 *              -eScl3DMode_TopAndBottom
 */
Scl3DMode dvProAV_SclVSIInfo23DMode(uint08 *VSIInfo);

/**
 * @brief Åª¨úHDMI Vendor Specific InfoFrame
 * @param [in] entity : ³q¹D¿ï¾Ü
 *              -etMain
 *              -etSub
 * @param [out] sVSI_Info : VSI info frame
 * @return ¦¨¥\(rcSUCCESS)©Î¥¢±Ñ(rcERROR)
 */
int dvProAV_SrcInputVSIInfoGet(RxPort sRxPort, uint08 *VSI_Info);

/**
 * @brief 讀取HDMI HDR InfoFrame
 * @param [in] sRxPort : 通道選擇
 *              -eRxPort_0
 *              -eRxPort_1
 *              -eRxPort_2
 *              -eRxPort_3
 * @param [out] sHdrInfo HDR資訊
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclInputHDRInfoGet(RxPort sRxPort, HDR_INFOFRAME_PROAV *sHdrInfo);

#ifdef __cplusplus
}

#endif



#endif
