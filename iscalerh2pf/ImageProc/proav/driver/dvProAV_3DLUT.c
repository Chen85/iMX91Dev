#include "dvProAV_3DLUT.h"
#include "dvProAV_Base.h"

//define IN_FILE_DEBUG
#ifndef IN_FILE_DEBUG
    #undef DBMSG
    #define DBMSG(...) do{}while(0)
    #define DBMSG_ASSERT() ASSERT_ALWAYS()
#endif

//-------------------------------------------------------------------------------------------------
// 3D LUT
//-------------------------------------------------------------------------------------------------
/**
 * @brief 3D LUT Table Spi Write
 * @param [in] u16Size : 3DLUT data byte
 * @param [in] u8LutTb : 3DLUT data point
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_3dLutTbSpiWr(uint32 u32Size, uint8 *u8LutTb)
{
    int status = rcSUCCESS;
    const uint16 u16WSize = 32700;
    uint8 u8WriteCycle = (u32Size / u16WSize);
    uint16 u8WriteRemainByte = (u32Size % u16WSize);

    status &= dvProAV_AccessWrite(ePsclPortEn, true);
    for(uint8 u8Cycle=0; u8Cycle<u8WriteCycle; u8Cycle++)
        status &= dvProAV_AccessBurstWrite(ePsclLUT3DPort, &u8LutTb[u8Cycle * u16WSize], u16WSize, BURST_FIX_ADDR);
    if(u8WriteRemainByte > 0)
        status &= dvProAV_AccessBurstWrite(ePsclLUT3DPort, &u8LutTb[u8WriteCycle * u16WSize], u8WriteRemainByte, BURST_FIX_ADDR);
    status &= dvProAV_AccessWrite(ePsclPortEn, false);

    return status;
}

/**
 * @brief 3DLUT Enable Set
 * @param [in] bLutEn : 3DLUT enable
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_3dLutEn(bool bLutEn)
{
    return dvProAV_AccessWrite(ePsclLUT3DEn, bLutEn);
}

/**
 * @brief 3DLUT Config
 * @param [in] eMode : 3DLUT mode
 *              - eLut3DModeStandard : Standard mode
 *              - eLut3DModeVivid : Vivid mode
 *              - eLut3DModeCinema : Cineam mode
 *              - else : disable 3DLUT
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_3dLutConfig(eLut3DMode eMode)
{
    int status = rcSUCCESS;

    status &= dvProAV_3dLutEn(false);
    if(eMode < eLut3DModeMax)
    {
        status &= dvProAV_3dLutTbSpiWr((uint32)LUT_3D_SIZE, (uint8 *)(&u8Lut3DTb[eMode][0]));
        status &= dvProAV_3dLutEn(true);
    }
    return status;
}
