#include "dvProAV_Base.h"
#include "dvProAV_HdmiRx.h"

//-------------------------------------------------------------------------------------------------
// HDMI Rx function
//-------------------------------------------------------------------------------------------------

/**
 * @brief 設HDMI RX IP 重置啟動/或除能
 * @param [in] bEnable
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_HdmiRxIpReset(bool bEnable)
{
    int status = rcSUCCESS;
    status &= dvProAV_AccessWrite(eBiuHdmiRx0Rst, (uint32)bEnable); // Reset HDMI IP Rx0
    status &= dvProAV_AccessWrite(eBiuHdmiRx1Rst, (uint32)bEnable); // Reset HDMI IP Rx1
    status &= dvProAV_AccessWrite(eBiuHdmiRx2Rst, (uint32)bEnable); // Reset HDMI IP Rx2
    status &= dvProAV_AccessWrite(eBiuHdmiRx3Rst, (uint32)bEnable); // Reset HDMI IP Rx3
    return status;
}

/**
 * @brief 傳回所有HDMI RX IP 重置是否完成
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
bool dvProAV_HdmiRxIsIpResetReady(void)
{
    int status = rcSUCCESS;
    uint32 stable[eRxPort_Max];
    bool ready = false;

    status &=  dvProAV_AccessRead(eRx0InfoChActive, &stable[0]);
    status &=  dvProAV_AccessRead(eRx1InfoChActive, &stable[1]);
    status &=  dvProAV_AccessRead(eRx2InfoChActive, &stable[2]);
    status &=  dvProAV_AccessRead(eRx3InfoChActive, &stable[3]);

    if (status == rcSUCCESS && stable[0] == 1 && stable[1] == 1  && stable[2] == 1  && stable[3] == 1)
        ready  = true;
    else
        ready  = false;

    return ready;
}

/**
 * @brief 取得輸入埠的VSI資訊
 * @param [in] sRxPort : 輸入埠選擇
 * @param [out] data : VSI Infoframe
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_HdmiRxVsiGet(RxPort sRxPort, VSI_INFOFRAME *data)
{
    int status = rcSUCCESS;
    if(sRxPort >= eRxPort_Max)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }
    uint32 ulValid = 0, ulRxStable = 0, ulShift;

    ulShift = sRxPort * (eRx1InfoAviInfoFrame - eRx0InfoAviInfoFrame);

    while(!ulValid)
    {
        status &= dvProAV_AccessRead(eRx0InfoChActive + ulShift, &ulRxStable);
        if(!ulRxStable)
            return rcERROR;
        status &= dvProAV_AccessRead(eRx0InfoVSIValid + ulShift, &ulValid);
    }

    status &= dvProAV_AccessBurstRead(eRx0InfoVsiInfoFrame + ulShift, (uint08 *)data, sizeof(VSI_INFOFRAME), BURST_INC_ADDR);

    return status;
}

/**
 * @brief 取得輸入埠的AVI資訊
 * @param [in] sRxPort : 輸入埠選擇
 * @param [out] data : AVI Infoframe
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_HdmiRxAviGet(RxPort sRxPort, AVI_INFOFRAME *data)
{
    int status = rcSUCCESS;
    if(sRxPort >= eRxPort_Max)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }
    uint32 ulValid = 0, ulRxStable = 0;
    uint32 RxActive[eRxPort_Max] = {eRx0InfoChActive, eRx1InfoChActive, eRx2InfoChActive, eRx3InfoChActive};
    uint32 RxInfo[eRxPort_Max] = {eRx0InfoAviInfoFrame, eRx1InfoAviInfoFrame, eRx2InfoAviInfoFrame, eRx3InfoAviInfoFrame};
    uint32 RxValid[eRxPort_Max] = {eRx0InfoAVIValid, eRx1InfoAVIValid, eRx2InfoAVIValid, eRx3InfoAVIValid};

    //ulShift = sRxPort * (eRx1InfoAviInfoFrame - eRx0InfoAviInfoFrame);

    while(!ulValid)
    {
        status &= dvProAV_AccessRead(RxActive[sRxPort], &ulRxStable);
        if(!ulRxStable)
        {
            return rcERROR;
        }
        status &= dvProAV_AccessRead(RxValid[sRxPort], &ulValid);
    }

    status &= dvProAV_AccessBurstRead(RxInfo[sRxPort], (uint08 *)data, sizeof(AVI_INFOFRAME), BURST_INC_ADDR);

    return status;
}

#ifdef QT_CPP
/**
 * @brief 取得輸入埠的HDR資訊
 * @param [in] sRxPort : 輸入埠選擇
 * @param [out] data : HDR Infoframe
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_HdmiRxHdrGet(RxPort sRxPort, HDR_INFOFRAME *data)
{
    int status = rcSUCCESS;

    if (sRxPort == eRxPort_0 )
        status &= dvProAV_AccessBurstRead(eRx0InfoHDRInfoFrame, (uint08 *)data, sizeof(HDR_INFOFRAME), BURST_INC_ADDR);
    else if (sRxPort == eRxPort_1 )
        status &= dvProAV_AccessBurstRead(eRx1InfoHDRInfoFrame, (uint08 *)data, sizeof(HDR_INFOFRAME), BURST_INC_ADDR);
    else if (sRxPort == eRxPort_2 )
        status &= dvProAV_AccessBurstRead(eRx2InfoHDRInfoFrame, (uint08 *)data, sizeof(HDR_INFOFRAME), BURST_INC_ADDR);
    else if (sRxPort == eRxPort_3 )
        status &= dvProAV_AccessBurstRead(eRx3InfoHDRInfoFrame, (uint08 *)data, sizeof(HDR_INFOFRAME), BURST_INC_ADDR);
    else
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }

    return status;
}
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
int dvProAV_HdmiRxDeScrambleSet(RxPort sRxPort, bool bDeScrambleEn)
{
    int status = rcSUCCESS;
    if(sRxPort >= eRxPort_Max)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }
    uint32 ulReg = 0;

    ulReg = (sRxPort == eRxPort_0)? eRx0InfoScrambleEn : (sRxPort == eRxPort_1)? eRx1InfoScrambleEn :
                 (sRxPort == eRxPort_2)? eRx2InfoScrambleEn : eRx3InfoScrambleEn;

    status &= dvProAV_AccessWrite(ulReg, bDeScrambleEn);

    return status;
}

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
int dvProAV_HdmiRxMode2p0Set(RxPort sRxPort, bool bHDMI2p0En)
{
    int status = rcSUCCESS;
    if(sRxPort >= eRxPort_Max)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }
    uint32 ulReg = 0;

    ulReg = (sRxPort == eRxPort_0)? eRx0InfoMode2p0Control : (sRxPort == eRxPort_1)? eRx1InfoMode2p0Control :
                 (sRxPort == eRxPort_2)? eRx2InfoMode2p0Control : eRx3InfoMode2p0Control;

    status &= dvProAV_AccessWrite(ulReg, (bHDMI2p0En)? 0x03: 0x01);

    return status;
}

/**
 * @brief 讀取HDMI mode 1.4 / 2.0
 * @param [in] sRxPort : Rx Port
 *              -eRxPort_0
 *              -eRxPort_1
 *              -eRxPort_2
 *              -eRxPort_3
 * @return bHDMI2p0 :
 *              -false: HDMI1.4
 *              -true: HDMI2.0
 */
bool dvProAV_SclHDMI2p0Get(RxPort sRxPort)
{
    int status = rcSUCCESS;
    if(sRxPort >= eRxPort_Max)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
    }
    uint32 bHDMI2p0 = 0, iWritePort = 0;

    iWritePort = (sRxPort == eRxPort_0)? eRx0InfoMode2p0Control : (sRxPort == eRxPort_1)? eRx1InfoMode2p0Control :
                 (sRxPort == eRxPort_2)? eRx2InfoMode2p0Control : eRx3InfoMode2p0Control;

    status &= dvProAV_AccessRead(iWritePort, &bHDMI2p0);

    return (bool)bHDMI2p0;
}

/**
 * @brief 讀取Rx over sampling
 * @param [in] sRxPort : Rx Port
 *              -eRxPort_0
 *              -eRxPort_1
 *              -eRxPort_2
 *              -eRxPort_3
 * @return bHDMI2p0 :
 *              -false: non-Over Sampling
 *              -true: Over Sampling
 */
bool dvProAV_SclOverSampleGet(RxPort sRxPort)
{
    int status = rcSUCCESS;
    if(sRxPort >= eRxPort_Max)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
    }
    uint32 OverSamplEn = 0, ulReg = 0;

    ulReg = (sRxPort == eRxPort_0)? eRx0InfoOverSampling : (sRxPort == eRxPort_1)? eRx1InfoOverSampling :
                 (sRxPort == eRxPort_2)? eRx2InfoOverSampling : eRx3InfoOverSampling;

    status &= dvProAV_AccessRead(ulReg, &OverSamplEn);

    return (bool)OverSamplEn;
}

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
int dvProAV_HdmiRxClkSet(RxPort sRxPort, uint16 uiClk)
{
    int status = rcSUCCESS;
    if(sRxPort >= eRxPort_Max)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }
    uint32 ulReg = 0;

    ulReg = (sRxPort == eRxPort_0)? eRx0InfoRegCfgFref : (sRxPort == eRxPort_1)? eRx1InfoRegCfgFref :
                 (sRxPort == eRxPort_2)? eRx2InfoRegCfgFref : eRx3InfoRegCfgFref;

    status &= dvProAV_AccessWrite(ulReg, (uiClk > 34000)? (uiClk >> 2) & 0x3FFF : uiClk);

    return status;
}

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
int dvProAV_HdmiRxConfigEn(RxPort sRxPort, bool bRxConfigEn)
{
    int status = rcSUCCESS;
    if(sRxPort >= eRxPort_Max)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }
    uint32 ulReg = 0;

    ulReg = (sRxPort == eRxPort_0)? eRx0InfoRcfgCtrl : (sRxPort == eRxPort_1)? eRx1InfoRcfgCtrl :
                 (sRxPort == eRxPort_2)? eRx2InfoRcfgCtrl : eRx3InfoRcfgCtrl;

    status &= dvProAV_AccessWrite(ulReg, bRxConfigEn);

    return status;
}

/**
 * @brief Pixel Repet times get
 * @param [in] sRxPort : Rx Port
 *              -eRxPort_0
 *              -eRxPort_1
 *              -eRxPort_2
 *              -eRxPort_3
 * @return Hdmi Rx Color Depth : 8/10/12/16
 */
int dvProAV_HdmiRxColorDepthGet(RxPort sRxPort)
{
    if(sRxPort >= eRxPort_Max)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
    }
    uint32 ulColorDepth = 8, ulReg = 0;

    ulReg = (sRxPort == eRxPort_0)? eRx0InfoGcpColorDepth : (sRxPort == eRxPort_1)? eRx1InfoGcpColorDepth :
             (sRxPort == eRxPort_2)? eRx2InfoGcpColorDepth : eRx3InfoGcpColorDepth;

    dvProAV_AccessRead(ulReg, &ulColorDepth);

    switch(ulColorDepth)
    {
        case eColorDepth_bpp24:
            return 8;
        case eColorDepth_bpp30:
            return 10;
        case eColorDepth_bpp36:
            return 12;
        case eColorDepth_bpp48:
            return 16;
        default:
            return 8;
    }
}

/**
 * @brief 讀取 HDMI Pixel Repet
 * @param [in] sRxPort : Rx Port
 *              -eRxPort_0
 *              -eRxPort_1
 *              -eRxPort_2
 *              -eRxPort_3
 * @return Pixel Repet times value
 */
int dvProAV_HdmiRxPixelRepetGet(RxPort sRxPort)
{
    uint08 ucPixelRepet = 0;

    if(sRxPort == eRxPort_0)
        dvProAV_AccessRawDataReadByte(0x81, 0x22, &ucPixelRepet);
    else if(sRxPort == eRxPort_1)
        dvProAV_AccessRawDataReadByte(0x82, 0x22, &ucPixelRepet);
    else if(sRxPort == eRxPort_2)
        dvProAV_AccessRawDataReadByte(0x85, 0x22, &ucPixelRepet);
    else if(sRxPort == eRxPort_3)
        dvProAV_AccessRawDataReadByte(0x86, 0x22, &ucPixelRepet);
    else
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
    }

    return ((ucPixelRepet & 0x0f) + 1);
}

/**
 * @brief 讀取HDMI RX Bit Clock Ratio
 * @param [in] port : Rx Port
 *              -eRxPort_0
 *              -eRxPort_1
 *              -eRxPort_2
 *              -eRxPort_3
 * @return bBcr : 0:eBitClkRatio_HdmiV1p4 / 1:eBitClkRatio_HdmiV2p0
 */
bool dvProAV_HdmiRxBCRGet(RxPort sRxPort)
{
    if(sRxPort >= eRxPort_Max)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
    }
    uint32 ulReg = 0, bBcr = 0;

    ulReg = (sRxPort == eRxPort_0)? eRx0InfoBitClockRatio : (sRxPort == eRxPort_1)? eRx1InfoBitClockRatio :
             (sRxPort == eRxPort_2)? eRx2InfoBitClockRatio : eRx3InfoBitClockRatio;
    dvProAV_AccessRead(ulReg, &bBcr);

    return (bool)bBcr;
}

/**
 * @brief 讀取HDMI RX TMDS clock (unit : 100kHz)
 * @param [in] port : Rx Port
 *              -eRxPort_0
 *              -eRxPort_1
 *              -eRxPort_2
 *              -eRxPort_3
 * @return Rx TMDS Clock (unit : 100kHz)
 */
uint32 dvProAV_HdmiRxTmdsClkGet(RxPort sRxPort)
{
    if(sRxPort >= eRxPort_Max)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
    }
    uint32 ulTMDSCLK, ulReg;
    ulReg = (sRxPort == eRxPort_0)? eRx0InfoTmdsClkFreq : (sRxPort == eRxPort_1)? eRx1InfoTmdsClkFreq :
            (sRxPort == eRxPort_2)? eRx2InfoTmdsClkFreq : eRx3InfoTmdsClkFreq;
    dvProAV_AccessRead(ulReg, &ulTMDSCLK);
    return ulTMDSCLK;
}

//-------------------------------------------------------------------------------------------------
// Info frame Convert
//-------------------------------------------------------------------------------------------------
/**
 * @brief Convert the Color Info to Pixel Mode
 * @param [in] colorInfo : Rx Color Info
 * @return pixelmode :
 *              - 0x00, 0x01 : 444
 *              - 0x02: 422
 *              - 0x03: 420
 */
uint08 dvProAV_HdmiRxColorInfo2PixelMode(uint08 colorInfo)
{
    uint08 PixelMode = colorInfo;
    if (PixelMode == eAVI_PIXEL_MODE_RGB)       // RGB444
        return 0x00;
    if (PixelMode == eAVI_PIXEL_MODE_YCbCr422)  // YCbCr422
        return 0x02;
    if (PixelMode == eAVI_PIXEL_MODE_YCbCr444)  // YCbCr444
        return 0x01;
    if (PixelMode == eAVI_PIXEL_MODE_YCbCr420)  // YCbCr420
        return 0x03;

    return PixelMode;
}

/**
 * @brief Convert the Color Info to RGB Mode
 * @param [in] colorInfo : Rx Color Info
 * @return RGBMode :
 *              - 0x00: RGB Limit
 *              - 0x01: RGB Full
 */
uint08 dvProAV_HdmiRxColorInfo2RGBMode(uint08 ucColorimetry, uint08 ucRGBMode)
{
    //ucColorimetry is vic

    uint08 RGBMode = 0;

    if(ucRGBMode == eAVI_RGB_RANGE_LIMITED)
    {
        RGBMode = 0x00; //RGB Limit
    }
    else if(ucRGBMode == eAVI_RGB_RANGE_FULL)
    {
        RGBMode = 0x01; //RGB Full
    }
    else
    {
        //eAVI_RGB_RANGE_DEFAULT //VIC by CEA-861
        if((ucColorimetry == 0) || (ucColorimetry == 1) ||
           ((ucColorimetry > 127) && (ucColorimetry < 193))||
           ((ucColorimetry > 219)))
        {
            RGBMode = 0x01; // RGB Full
        }
        else
        {
            RGBMode = 0x00; //RGB Limit
        }
    }

    return RGBMode;
}

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
uint08 dvProAV_HdmiRxColorInfo2ColorSpace(AVI_INFOFRAME *sAVIInfo)
{
    uint08 cscmode = 0;
    uint08 Colorimetry = sAVIInfo->byte2.C;
    uint08 ExtendedColorimetry = sAVIInfo->byte3.EC;
    uint08 PixelMode = sAVIInfo->byte1.Y;

    if (PixelMode == eAVI_PIXEL_MODE_RGB) // RGB
    {
        cscmode = RX_COLORSPACE_RGB; // RGB

        //if (ExtendedColorimetry !== 0x04) // AdobeRGB
        //else if (ExtendedColorimetry == 0x06) // ITU-R BT.2020
    }
    else if (PixelMode == eAVI_PIXEL_MODE_YCbCr422 ||  // YCbCr422
             PixelMode == eAVI_PIXEL_MODE_YCbCr444 ||  // YCbCr444
             PixelMode == eAVI_PIXEL_MODE_YCbCr420)    // YCbCr420
    {
        if (Colorimetry == eAVI_COLORIMETRY_EXCOLORIMETRY) // Extended ColorimetryInformation Valid
        {
            if ( ExtendedColorimetry == eAVI_EXCOLORIMETRY_xvYCC601 ||     // xvYCC601
                 ExtendedColorimetry == eAVI_EXCOLORIMETRY_sYCC601 ||     // sYCC601
                 ExtendedColorimetry == eAVI_EXCOLORIMETRY_AdobeYCC601  )     // AdobeYCC601
            {
                cscmode = RX_COLORSPACE_REC601; // Set space to Rec. 601
            }
            else if (ExtendedColorimetry == eAVI_EXCOLORIMETRY_xvYCC709)   // xvYCC709
            {
                cscmode = RX_COLORSPACE_REC709; // Set space to Rec. 709
            }
            else if (ExtendedColorimetry == eAVI_EXCOLORIMETRY_ITUR_BT2020_YCBCR || // ITU-R BT.2020 Y'CC'BCC'RC
                     ExtendedColorimetry == eAVI_EXCOLORIMETRY_ITUR_BT2020_RGB_OR_YCBCR )  // ITU-R BT.2020 R??G??B?? or Y'C'BC'R
            {
                cscmode = RX_COLORSPACE_REC2020; // Set space to Rec. 2020
            }
            else // if (ExtendedColorimetry == eAVI_EXCOLORIMETRY_Reserved) // Reserved
            {
                //cscmode = RX_COLORSPACE_UNKNOW; // Set space to Invalid
                cscmode = RX_COLORSPACE_REC709;
             }
        }
        else if (Colorimetry == eAVI_COLORIMETRY_ITUR_BT709/*0x02*/) // SMTPE170M(aka BT.601)
        {
            cscmode = RX_COLORSPACE_REC709; // Set space to Rec. 601
        }
        else if (Colorimetry == eAVI_COLORIMETRY_SMTPE_170M/*0x01*/) // ITU-R BT.709
        {
            cscmode = RX_COLORSPACE_REC601; // Set space to Rec. 709
        }
        else // if (Colorimetry == eAVI_COLORIMETRY_NODATA) // No Data
        {
            //cscmode = RX_COLORSPACE_UNKNOW; // Set space to Invalid
            cscmode = RX_COLORSPACE_REC709;
        }
    }
    else
    {
        // Reserved or IDO-Defined
        cscmode = RX_COLORSPACE_UNKNOW; // Invalid
    }
    return cscmode;
}

//-------------------------------------------------------------------------------------------------
// HDMI Rx Initial
//-------------------------------------------------------------------------------------------------

/**
 * @brief 初始化HDMI Rx
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_HdmiRxInit(void)
{
    int status = rcSUCCESS;
    bool bRxConfigEn = false;

    // FPGA Rx FW config Enable
#ifdef CONFIG_RX_TIMING
    bRxConfigEn = true;
#endif

    status &= dvProAV_HdmiRxConfigEn(eRxPort_0, bRxConfigEn);
    status &= dvProAV_HdmiRxConfigEn(eRxPort_1, bRxConfigEn);
    status &= dvProAV_HdmiRxConfigEn(eRxPort_2, bRxConfigEn);
    status &= dvProAV_HdmiRxConfigEn(eRxPort_3, bRxConfigEn);

    return status;
}

//-------------------------------------------------------------------------------------------------
// 待修正
//-------------------------------------------------------------------------------------------------

/**
 * @brief Convert the VSI Info to HDMI Video Format
 * @param [in] VSIInfo
 * @return HDMIVideoFormat :
 *              - 0x0: No additional HDMI video format is presented in this packet.
 *              - 0x1: Extended resolution format (e.g. used for 4K x 2K video) present.
 *              - 0x2: 3D format indication present.
 */
uint08 dvProAV_SclVSIInfo2HDMIVideoFormat(uint08 *VSIInfo)
{
    return ((VSIInfo[5] >> 2) & 0x03);
}

/**
 * @brief Convert the VSI Info to 3D mode
 * @param [in] VSIInfo
 * @return uc3Dmode : 3D mode
 *              -eScl3DMode_Off
 *              -eScl3DMode_FramePacking
 *              -eScl3DMode_SideBySide
 *              -eScl3DMode_TopAndBottom
 */
Scl3DMode dvProAV_SclVSIInfo23DMode(uint08 *VSIInfo)
{
    uint08 uc3Dmode = eScl3DMode_Off;

    if((VSIInfo[5] >> 1) == 0x04) //3D format
    {
        uc3Dmode = VSIInfo[6];

        uc3Dmode = (((VSIInfo[6] >> 1) & 0x0f) == 0)? eScl3DMode_FramePacking :
                   (((VSIInfo[6] >> 1) & 0x0f) == 6)? eScl3DMode_TopAndBottom :
                   (((VSIInfo[6] >> 1) & 0x0f) == 3)? eScl3DMode_SideBySide : //full
                   (((VSIInfo[6] >> 1) & 0x0f) == 8)? eScl3DMode_SideBySide : eScl3DMode_Off;
    }

    return (Scl3DMode)uc3Dmode;
}

/**
 * @brief Åª¨úHDMI Vendor Specific InfoFrame
 * @param [in] entity : ³q¹D¿ï¾Ü
 *              -etMain
 *              -etSub
 * @param [out] sVSI_Info : VSI info frame
 * @return ¦¨¥\(rcSUCCESS)©Î¥¢±Ñ(rcERROR)
 */
int dvProAV_SrcInputVSIInfoGet(RxPort sRxPort, uint08 *VSI_Info)
{
    int status = rcSUCCESS;
    if(sRxPort >= eRxPort_Max)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }
    uint08 rxVSIByte[8] = {0,0,0,0,0,0,0,0};
    uint32 RxInfo[eRxPort_Max] = {eRx0InfoVsiInfoFrame, eRx1InfoVsiInfoFrame, eRx2InfoVsiInfoFrame, eRx3InfoVsiInfoFrame};

#if 1
    if(sRxPort >= eRxPort_Max)
    {
        return rcERROR;
    }

    dvProAV_AccessBurstRead(RxInfo[sRxPort], rxVSIByte, 8, BURST_INC_ADDR);

#else
    if (sRxPort == eRxPort_0 )
        status &= dvProAV_AccessBurstRead(eRx0InfoVsiInfoFrame, rxVSIByte, sizeof(rxVSIByte), BURST_INC_ADDR);
    else if (sRxPort == eRxPort_1 )
        status &= dvProAV_AccessBurstRead(eRx1InfoVsiInfoFrame, rxVSIByte, sizeof(rxVSIByte), BURST_INC_ADDR);
    else if (sRxPort == eRxPort_2 )
        status &= dvProAV_AccessBurstRead(eRx2InfoVsiInfoFrame, rxVSIByte, sizeof(rxVSIByte), BURST_INC_ADDR);
    else if (sRxPort == eRxPort_3 )
        status &= dvProAV_AccessBurstRead(eRx3InfoVsiInfoFrame, rxVSIByte, sizeof(rxVSIByte), BURST_INC_ADDR);
#endif /* 0 */

    memcpy((uint8*)VSI_Info, rxVSIByte, 8);
    return status;
}

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
int dvProAV_SclInputHDRInfoGet(RxPort sRxPort, HDR_INFOFRAME_PROAV *sHdrInfo)
{
    int status = rcSUCCESS;
    if(sRxPort >= eRxPort_Max)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }
    uint08 rxByte[HDR_INFOFRAME_PROAV_SIZE + 1];
    uint08 lo3Bits, hi5Bits;

    memset(rxByte, 0, sizeof(rxByte));

    // 讀取Ipu HDMI HDR InfoFrame (HDR)參數
    if (sRxPort == eRxPort_0 )
        status &= dvProAV_AccessBurstRead(eRx0InfoHDRInfoFrame, rxByte, sizeof(rxByte), BURST_INC_ADDR);
    else if (sRxPort == eRxPort_1 )
        status &= dvProAV_AccessBurstRead(eRx1InfoHDRInfoFrame, rxByte, sizeof(rxByte), BURST_INC_ADDR);
    else if (sRxPort == eRxPort_2 )
        status &= dvProAV_AccessBurstRead(eRx2InfoHDRInfoFrame, rxByte, sizeof(rxByte), BURST_INC_ADDR);
    else if (sRxPort == eRxPort_3 )
        status &= dvProAV_AccessBurstRead(eRx3InfoHDRInfoFrame, rxByte, sizeof(rxByte), BURST_INC_ADDR);

    uint08 u8HdrLen = rxByte[0] & 0x1F;
    if(u8HdrLen <= (HDR_INFOFRAME_PROAV_SIZE - 2))  // Reserved byte = 2
    {
        for(uint08 i=0; i<HDR_INFOFRAME_SIZE; i++)
        {
            lo3Bits = (uint08)((rxByte[i  ] & 0xE0) >> 5);
            hi5Bits = (uint08)((rxByte[i+1] & 0x1F) << 3);
            rxByte[i] = hi5Bits | lo3Bits;
        }
    }

    memcpy(sHdrInfo, rxByte, HDR_INFOFRAME_PROAV_SIZE);
    return status;
}

