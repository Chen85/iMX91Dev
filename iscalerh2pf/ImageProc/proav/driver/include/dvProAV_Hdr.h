#ifndef DV_PROAV_HDR_GAMMA_CAL_H
#define DV_PROAV_HDR_GAMMA_CAL_H

#include <stdint.h>
#include "dvProAV_Platform.h"
#include "dvProAV_Base.h"
#include "dvProAV_HdrGammaTable.h"
#ifdef __cplusplus
extern "C"
{
#endif

#define GAMMA_TBL_NUM 1024
#define FRACTION_BITS 16

typedef struct {
    double signalMax;  // LW_in
    double signalMin;  // LB_in
    double displayMax; // Lmax_in
    double displayMin; // Lmin_in

    bool clipEnabled;
    double clipPoint;

    int transMethod;
    double transHighPivotY;
    double transGamma;
    double transLowPivotX;
    double transLowPivotY;

    uint16 u16MaxLevel;
}PQ_HDR_PARAM;

typedef enum
{
    eHDRLevel_1,
    eHDRLevel_2,
    eHDRLevel_3,
    eHDRLevel_4,

    eHDRLevel_MAX,
} eHDR_LEVEL;

typedef enum
{
    eHDRType_Off,
    eHDRType_10,
    eHDRType_10p,
    eHDRType_Sdr10p,

    eHDRType_lMAX,
} eHDR_TYPE;

typedef enum
{
    eHDR10pMode_ForceReg,               // luminance & max level use register value
    eHDR10pMode_InfoLum,                // get infoframe luminance & max level use register value
    eHDR10pMode_InfoLumWithLevelDetect, // get infoframe luminance & max level detect
    eHDR10pMode_RegLumWithLevelDetect,  // luminance use register value & max level detect

    eHDR10pMode_MAX,
} eHDR10pMode;

typedef struct HDR10P{
    eHDR10pMode eMode;
    uint16 u16SglMaxLum;
    float fSglMinxLum;
    uint16 u16DisMaxLum;
    float fDisMinLum;
    float fHdrGamma;
    uint16 u16MaxLevel;
    uint16 u16MaxLevelMin;
    uint8 u8FrameAdaptUp;
    uint8 u8FrameAdaptDown;
}HDR10P_PARAM;

static const HDR10P_PARAM sHdrCfgParam[3][4] =
//                                 Mode,SglMax,SglMin,DisMax,DisMin,Gamma,MaxLevel,MaxLevelMin,AdaptUp,AdaptDown
{
    {{             eHDR10pMode_ForceReg, 10000,     0,  1000,     0,  1.2,    1023,       1023,      0,        0}, // HDR
     {             eHDR10pMode_ForceReg, 10000,     0,  1000,     0,  1.5,    1023,       1023,      0,        0},
     {             eHDR10pMode_ForceReg, 15000,     0,  1000,     0,  1.6,    1023,       1023,      0,        0},
     {             eHDR10pMode_ForceReg, 16383,     0,  1000,     0,  1.7,    1023,       1023,      0,        0}},
    {{eHDR10pMode_RegLumWithLevelDetect, 10000,     0,  1000,     0,  1.0,     768,        768,      4,        4}, // HDR 10+
     {eHDR10pMode_RegLumWithLevelDetect, 10000,     0,  1000,     0,  1.1,     768,        768,      4,        4},
     {eHDR10pMode_RegLumWithLevelDetect, 10000,     0,  1000,     0,  1.2,     768,        768,      4,        4},
     {eHDR10pMode_RegLumWithLevelDetect, 10000,     0,  1000,     0,  1.5,     768,        768,      4,        4}},
    {{eHDR10pMode_RegLumWithLevelDetect,  2000,     0,  1000,     0,  1.1,     922,        922,      4,        4}, // SDR with HDR 10+
     {eHDR10pMode_RegLumWithLevelDetect,  1000,     0,  1000,     0,  1.1,     922,        922,      4,        4},
     {eHDR10pMode_RegLumWithLevelDetect,  2000,     0,  1000,     0,  1.1,     922,        922,      4,        4},
     {eHDR10pMode_RegLumWithLevelDetect,  1000,     0,  1000,     0,  1.1,     922,        922,      4,        4}}
};

//-------------------------------------------------------------------------------------------------
// Hdr Tone Mapping (Gamma4)
//-------------------------------------------------------------------------------------------------
uint08 *dvProAV_HdrPq2SdrGammaCalculate(PQ_HDR_PARAM hdrParam);

//-------------------------------------------------------------------------------------------------
// HDR
//-------------------------------------------------------------------------------------------------
/**
 * @brief Download the HDR CSC Table to Chip
 * @param [in] eEntity : channel select
 *          -eSclEntity_Main
 *          -eSclEntity_Sub
 * @param [in] cscTable: HDR CSC Table (3 bytes * 9)
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 * @note
 * if(input>0)
 *    output =DEC2HEX(FLOOR(N32*POWER(2,16),1),6) ;
 * else
 *    output =DEC2HEX(FLOOR(ABS(N32)*POWER(2,16),1)+POWER(2,17),6)) ;
 */
int dvProAV_SclVopHdrCscSet(SclEntity eEntity, uint08 *cscTable);

/**
 * @brief dvProAV_SclVopHdrCscEn
 * @param [in] eEntity : channel select
 *          -eSclEntity_Main
 *          -eSclEntity_Sub
 * @param [in] bHdrCscEn : HDR CSC enable
 *          -false : HDR CSC disable
 *          -true : HDR CSC enable
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclVopHdrCscEn(SclEntity eEntity, bool bHdrCscEn);

/**
 * @brief Download the HDR Gamma Table to Chip
 * @param [in] eEntity : channel select
 *          -eSclEntity_Main
 *          -eSclEntity_Sub
 * @param [in] gammaType: HDR gamma type
 *                 - eHdrGamma1
 *                 - eHdrGamma3
 *                 - eHdrGamma4
 * @param [in] gamma: gamma data
 *
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclVopHdrGammaDl2Chip(SclEntity eEntity, uint08 gammaType, const uint08 *gamma);

/**
 * @brief Set the HDR csc by index
 * @param [in] eEntity : channel select
 *          -eSclEntity_Main
 *          -eSclEntity_Sub
 * @param [in] cscType: HDR csc type index
 *          -eHdrCsc2020to709
 *          -eHdrCsc2020toAdobe
 *          -eHdrCsc2020toP3
 *          -eHdrCscP3to709
 *          -eHdrCscAdobeto709
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclVopHdrCscByIndexSet(SclEntity eEntity, HdrCscType eCscType);

/**
 * @brief Set the HDR Gamma Enabled by Type
 * @param [in] eEntity : channel select
 *          -eSclEntity_Main
 *          -eSclEntity_Sub
 * @param [in] gammaType: HDR gamma type
 *                 - eHdrGamma1
 *                 - eHdrGamma3
 *                 - eHdrGamma4
 *                 - eHdrGamma3_7t
 * @param [in] enable: enable the selected gamma type
 *
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclVopHdrGammaEnableSet(SclEntity eEntity, uint08 gammaType, bool enable);

/**
 * @brief Color Gamut Matrix Transform
 * @param [in] sSrcClrGamutCoor : source color gamut coordinate
 * @param [in] sTgClrGamutCoor : target color gamut coordinate
 * @param [out] darMtx : color gamut transform matrix point
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_HdrCscTransMtx(CIE_Coor sSrcClrGamutCoor, CIE_Coor sTgClrGamutCoor, double *darMtx);

/**
 * @brief Color Gamut Coordinate Transform And Set
 * @param [in] eEntity :  channel select
 *          -eSclEntity_Main
 *          -eSclEntity_Sub
 * @param [in] sSrcClrGamutCoor : source color gamut coordinate
 * @param [in] sTgClrGamutCoor : target color gamut coordinate
 * @param [in] bHdrCscEn
 *          -false : disable HDR CSC transform
 *          -true  : enable HDR CSC transform
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_HdrCscCoorTransSet(SclEntity eEntity, CIE_Coor sSrcClrGamutCoor, CIE_Coor sTgClrGamutCoor, bool bHdrCscEn);

/**
 * @brief Enable The HDR CSC Transform
 * @param [in] eEntity :  channel select
 *          -eSclEntity_Main
 *          -eSclEntity_Sub
 * @param [in] eCscType: HDR csc type index
 *          -eHdrCsc2020to709
 *          -eHdrCsc2020toAdobe
 *          -eHdrCsc2020toP3
 *          -eHdrCscP3to709
 *          -eHdrCscP3Adobe709
 * @param [in] bHdrCscEn
 *          -false : disable HDR CSC transform
 *          -true  : enable HDR CSC transform
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_HdrCscTransEn(SclEntity eEntity, HdrCscType eCscType, bool bHdrCscEn);

/**
 * @brief Enable The HDR Tone Mapping Gamma Curve
 * @param [in] eEntity : channel select
 *          -eSclEntity_Main
 *          -eSclEntity_Sub
 * @param [in] eHdr4GammaType
 *          -eHdrGamma4_1
 *          -eHdrGamma4_2
 *          -eHdrGamma4_3
 *          -eHdrGamma4_4
 *          -eHdrGamma4_Auto
 * @param [in] sHdrParam : when eHdr4GammaType=eHdrGamma4_Auto, calculation parameters that need to be used
 * @param [in] bToneMappingEn
 *          -false : disable HDR Tone Mapping Gamma Curve
 *          -true  : enable HDR Tone Mapping Gamma Curve
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_HdrToneMappingEn(SclEntity eEntity, HDR_GAMMA_TYPE eHdr4GammaType, PQ_HDR_PARAM sHdrParam, bool bToneMappingEn);

//-------------------------------------------------------------------------------------------------
// HDR 10+
//-------------------------------------------------------------------------------------------------
/**
 * @brief Hdr10+ Tone Mapping Enable
 * @param [in] eEntity
 * @param [in] bEnable
 *          -false : disable HDR Tone Mapping Gamma Curve
 *          -true  : enable HDR Tone Mapping Gamma Curve
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_Hdr10pEn(SclEntity eEntity, bool bEnable);

/**
 * @brief Hdr10+ Mode select
 * @param [in] eEntity : channel select
 * @param [in] eMode : mode select
 *          -eHDR10pMode_ForceReg,               // luminance & max level use register value
 *          -eHDR10pMode_InfoLum,                // get infoframe luminance & max level use register value
 *          -eHDR10pMode_InfoLumWithLevelDetect, // get infoframe luminance & max level detect
 *          -eHDR10pMode_RegLumWithLevelDetect,  // luminance use register value & max level detect
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_Hdr10pMode(SclEntity eEntity, eHDR10pMode eMode);

/**
 * @brief Hdr10+ Signel Maximal Luminance Set
 * @param [in] eEntity : channel select
 * @param [in] u16SglMaxLum : DisMaxLum~16383
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_Hdr10pSglMaxLum(SclEntity eEntity, uint16 u16SglMaxLum);

/**
 * @brief Hdr10+ Signel Minimal Luminance Set
 * @param [in] eEntity : channel select
 * @param [in] fSglMinLum : 0 ~ 0.9999
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_Hdr10pSglMinLum(SclEntity eEntity, float fSglMinLum);

/**
 * @brief Hdr10+ Display Maximal Luminance Set
 * @param [in] eEntity : channel select
 * @param [in] u16DisMaxLum : 1000 ~ 10000
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_Hdr10pDisMaxLum(SclEntity eEntity, uint16 u16DisMaxLum);

/**
 * @brief Hdr10+ Display Minimal Luminance Set
 * @param [in] eEntity : channel select
 * @param [in] fDisMinLum : 0 ~ 0.9999
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_Hdr10pDisMinLum(SclEntity eEntity, float fDisMinLum);

/**
 * @brief Hdr10+ HDR Gamma Set
 * @param [in] eEntity : channel select
 * @param [in] fGamma : 0.357 ~ 2.8
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_Hdr10pHdrGamma(SclEntity eEntity, float fGamma);

/**
 * @brief Hdr10+ Frame Max Level Set
 * @param [in] eEntity : channel select
 * @param [in] u16MaxLevel : 256 ~ 1023
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_Hdr10pMaxLevelSet(SclEntity eEntity, uint16 u16MaxLevel);

/**
 * @brief Hdr10+ Frame Max Level Min Set
 * @param [in] eEntity : channel select
 * @param [in] u16MaxLevelMin : 256 ~ 1023
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_Hdr10pMaxLevelMinSet(SclEntity eEntity, uint16 u16MaxLevelMin);

/**
 * @brief Hdr10+ Frame Adapt Level Set
 * @param [in] eEntity : channel select
 * @param [in] u8Inc : 0~255
 * @param [in] u8Dec : 0~255
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_Hdr10pFrameAdaptSet(SclEntity eEntity, uint8 u8Inc, uint8 u8Dec);

/**
 * @brief Hdr10+ Frame Detect Area Set
 * @param [in] u16StLine : start line
 * @param [in] u16EndLine : end line
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_Hdr10pFrameDecAreaSet(uint16 u16StLine, uint16 u16EndLine);

/**
 * @brief HDR Tone Mapping Config Set
 * @param [in] eEntity :  channel select
 *          -eSclEntity_Main
 *          -eSclEntity_Sub
 * @param [in] eHdrType
 *          -eHDRType_Off
 *          -eHDRType_10
 *          -eHDRType_10p
 *          -eHDRType_Sdr10p
 * @param [in] eLevel :
 *          -eHDRLevel_1
 *          -eHDRLevel_2
 *          -eHDRLevel_3
 *          -eHDRLevel_4
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_Hdr10pToneMappingSet(SclEntity eEntity, eHDR_TYPE eHdrType, eHDR_LEVEL eHdrLevel);

#ifdef __cplusplus
}
#endif

#endif // DV_PROAV_HDR_GAMMA_TABLE_H
