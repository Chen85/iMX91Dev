#include "dvProAV_Base.h"
#include "dvProAV_Blend.h"
#include "dvProAV_Access.h"
#include "dvProAV_RegTable.h"
#include "dvProAV_SclGammaTable.h"
#ifndef QT_CPP
    #include "utilDbgMsg.h"
#endif

//#define IN_FILE_DEBUG
#ifndef IN_FILE_DEBUG
    #undef DBMSG
    #define DBMSG(...) do{}while(0)
#endif

#define BLD_GAMMA_TABLE_UPDATE_DELAY 1 // 1 msec
#define GAMMA_TYPE_COUNT 2

static const uint32 m_cuEmuGammaEnable[GAMMA_TYPE_COUNT] = {eBldFrontGammaEn, eBldBackGammaEn};
static const uint32 m_cuEmuGammaBufWE[GAMMA_TYPE_COUNT] = {eBldFrontGmaBfWE, eBldBackGmaBfWE};
static const uint32 m_cuEmuGammaRgbSep[GAMMA_TYPE_COUNT] = {eBldFrontGmaRgbSep, eBldBackGmaRgbSep};
static const uint32 m_cuEmuGammaBufPort[GAMMA_TYPE_COUNT] = {eBldFrontGmaBfPort, eBldBackGmaBfPort};
static const uint32 m_cuEmuGammaBufDl[GAMMA_TYPE_COUNT] = {eBldFrontGmaBfDl, eBldBackGmaBfDl};
static const uint32 m_cuEmuGammaDithEn[GAMMA_TYPE_COUNT] = { eBldFrontGmaDithEn, eBldBackGmaDithEn};
static const uint32 m_cuEmuGammaDithValue[GAMMA_TYPE_COUNT] = { eBldFrontGmaDithAddValue, eBldBackGmaDithAddValue};

static uint08 m_cuUnitList[3] = { 64, 48, 128}; //0:Gain 8bits = 64, Gain 10bit = 48, 4bits bias = 128

static uint08 m_DbdDataMode = 0; // 0: 8bits weighting, 1: 10bits weighting, 2: 4bits bias
static uint16 m_PanelWidth = 1920;
static uint16 m_PanelHeight = 1080;
static uint32 m_BlockAddr = 0;


//-------------------------------------------------------------------------------------------------
// Edge Blend
//-------------------------------------------------------------------------------------------------
/**
 * @brief  entable blending updating
 * @param  [in] bEnable
 *         - true  : enable blending updating
 *         - false : disable blending updating
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_BldUpdatingEnable(bool bEnable)
{
    return dvProAV_AccessWrite(eEgblUpdating, bEnable ? false : true);
}

/**
 * @brief set blending enable
 * @param [in] u8Mode :
 *        - bit 0: enable bottom blending
 *        - bit 1: enable top blending
 *        - bit 2: enable right blending
 *        - bit 3: enable left blending
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_BldEnableSet(uint08 u8Mode)
{
    int status = rcSUCCESS;

    dvProAV_InterfaceWriteErrorCountGet(); // clear access write error count before any interface write

    status &= dvProAV_AccessWrite(eEgblEnableB, (uint32) (u8Mode & 0x01) == 0x01 ? true : false);
    status &= dvProAV_AccessWrite(eEgblEnableT, (uint32) (u8Mode & 0x02) == 0x02 ? true : false);
    status &= dvProAV_AccessWrite(eEgblEnableR, (uint32) (u8Mode & 0x04) == 0x04 ? true : false);
    status &= dvProAV_AccessWrite(eEgblEnableL, (uint32) (u8Mode & 0x08) == 0x08 ? true : false);

    status &= (dvProAV_InterfaceWriteErrorCountGet() > 0) ? rcERROR : rcSUCCESS; // check access write error count after all interface write
    return status;
}

/**
 * @brief Get Blending Enable Status
 * @return u32BlendEn : false=disable / true=enable
 *        - bit 0: enable bottom blending
 *        - bit 1: enable top blending
 *        - bit 2: enable right blending
 *        - bit 3: enable left blending
 */
uint08 dvProAV_BldEnableGet(void)
{
    uint32 u32BlendEn = 0;
    dvProAV_AccessRead(REG_EgblEn, &u32BlendEn);
    return (u32BlendEn & 0x0f);
}

/**
 * @brief set blending start and width of top
 * @param [in] u16Start : edge blending vertical top start position
 * @param [in] u16Width : edge blending vertical top width
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_BldTopSet(uint16 u16Start, uint16 u16Width)
{
    int status = rcSUCCESS;
    uint32 topEnable = 0; // false
    uint32 gradient;

    if(u16Width <=  1)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }
    else
    {
        gradient= 131072/(u16Width - 1); //131072 = 2^17
    }

    dvProAV_InterfaceWriteErrorCountGet(); // clear access write error count before any interface write

    status &= dvProAV_AccessWrite(eEgblVTGrad, gradient);
    status &= dvProAV_AccessWrite(eEgblVTSt, (uint32)u16Start);
    status &= dvProAV_AccessWrite(eEgblVTW, (uint32)u16Width);
    status &= dvProAV_AccessRead(eEgblEnableT, &topEnable);
    status &= dvProAV_AccessWrite(eEgblVinit, (topEnable == 1) ? (uint32)0x00:  (uint32)0x80);

    status &= (dvProAV_InterfaceWriteErrorCountGet() > 0) ? rcERROR : rcSUCCESS; // check access write error count after all interface write
    return status;
}

/**
 * @brief set blending start and width of bottom
 * @param [in] u16Start : edge blending vertical bottom start position
 * @param [in] width : edge blending vertical bottom width
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_BldBottomSet(uint16 u16Start, uint16 u16Width)
{
    int status = rcSUCCESS;
    uint32 topEnable = 0; // false
    uint32 gradient;

    if(u16Width <= 1)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }
    else
    {
        gradient= (uint16)(-131072/(u16Width - 1) + 1); //131072 = 2^17
    }

    dvProAV_InterfaceWriteErrorCountGet(); // clear access write error count before any interface write

    status &= dvProAV_AccessWrite(eEgblVBGrad, gradient);
    status &= dvProAV_AccessWrite(eEgblVBSt, u16Start);
    status &= dvProAV_AccessWrite(eEgblVBW, u16Width);
    status &= dvProAV_AccessRead(eEgblEnableT, &topEnable);
    status &= dvProAV_AccessWrite(eEgblVinit, (topEnable == 1) ? (uint32)0x00:  (uint32)0x80);

    status &= (dvProAV_InterfaceWriteErrorCountGet() > 0) ? rcERROR : rcSUCCESS; // check access write error count after all interface write
    return status;
}

/**
 * @brief set blending start and width of left
 * @param [in] u16Start : edge blending horizontal left start position
 * @param [in] u16Width : edge blending horizontal left width
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_BldLeftSet(uint16 u16Start, uint16 u16Width)
{
    int status = rcSUCCESS;
    uint32 leftEnable = 0; // false
    uint32 gradient;

    if (u16Width <= 1)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }
    else
    {
        gradient= 131072/(u16Width - 1); //131072 = 2^17
    }

    dvProAV_InterfaceWriteErrorCountGet(); // clear access write error count before any interface write

    status &= dvProAV_AccessWrite(eEgblHLGrad, gradient);
    status &= dvProAV_AccessWrite(eEgblHLSt, u16Start);
    status &= dvProAV_AccessWrite(eEgblHLW, u16Width);
    status &= dvProAV_AccessRead(eEgblEnableL, &leftEnable);
    status &= dvProAV_AccessWrite(eEgblHinit, (leftEnable == 1) ? (uint32)0x00:  (uint32)0x80);

    status &= (dvProAV_InterfaceWriteErrorCountGet() > 0) ? rcERROR : rcSUCCESS; // check access write error count after all interface write
    return status;
}

/**
 * @brief set blending start and width of right
 * @param [in] u16Start : edge blending horizontal right start position
 * @param [in] u16Width : edge blending horizontal right width
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_BldRightSet(uint16 u16Start, uint16 u16Width)
{
    int status = rcSUCCESS;
    uint32 leftEnable = 0; // false
    uint32 gradient;

    if(u16Width <= 1)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }
    else
    {
        gradient= (uint16)(-131072/(u16Width - 1) + 1); //131072 = 2^17
    }

    dvProAV_InterfaceWriteErrorCountGet(); // clear access write error count before any interface write

    status &= dvProAV_AccessWrite(eEgblHRGrad, gradient);
    status &= dvProAV_AccessWrite(eEgblHRSt, u16Start);
    status &= dvProAV_AccessWrite(eEgblHRW, u16Width);
    status &= dvProAV_AccessRead(eEgblEnableL, &leftEnable);
    status &= dvProAV_AccessWrite(eEgblHinit, (leftEnable == 1) ? (uint32)0x00:  (uint32)0x80);

    status &= (dvProAV_InterfaceWriteErrorCountGet() > 0) ? rcERROR : rcSUCCESS; // check access write error count after all interface write
    return status;
}

/**
 * @brief set reading/writing to bias data table enable by the channel
 * @param [in] u8Channel
 *        - 0 : Select Red
 *        - 1 : Select Green
 *        - 2 : Select Blue
 *        - 3 : Select all colors and read ExNOR of them.
 * @param [in] bWrite
 *        - false : read
 *        - true : write
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_BldBiasAccessSet(uint8 u8Channel, bool bWrite)
{
    int status = rcSUCCESS;
    if(u8Channel > 3)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }

    dvProAV_InterfaceWriteErrorCountGet(); // clear access write error count before any interface write

    status &= dvProAV_AccessWrite(eEgblBiasCh, u8Channel);
    status &= dvProAV_AccessWrite(eEgblBiasWr, (uint32)(bWrite ? true:false)); // 0: read, 1: write

    status &= (dvProAV_InterfaceWriteErrorCountGet() > 0) ? rcERROR : rcSUCCESS; // check access write error count after all interface write
    return status;
}

/**
 * @brief set the edge-blending black level correction function enable
 * @param [in] bEnable : enable/disbale edge-blending black level correction function
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_BldBiasEnableSet(bool bEnable)
{
    int status = rcSUCCESS;

    dvProAV_InterfaceWriteErrorCountGet(); // clear access write error count before any interface write

    status &= dvProAV_AccessWrite(eEgblBiasEn, (uint32)bEnable);

    status &= (dvProAV_InterfaceWriteErrorCountGet() > 0) ? rcERROR : rcSUCCESS; // check access write error count after all interface write
    return status;
}

/**
 * @brief set the address to write to the bias table
 * @param [in] u8Table : This is the table number, for 9 location.
 *        - bit 0: bottom
 *        - bit 1: top
 *        - bit 2: right
 *        - bit 3: left
 *
 * @param [in] u8Level : This is a 16 levels bias corresponding to the 16 levels of the pixel value..
 *                In read mode assign the start adress to write bias table.
 *                In write mode, add 1 to the address automatically.
 *
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_BldBiasBufAddrSet(uint08 u8Table, uint08 u8Level)
{
    int status = rcSUCCESS;

    dvProAV_InterfaceWriteErrorCountGet(); // clear access write error count before any interface write

    status &= dvProAV_AccessWrite(eEgblBiasBfTableAddr, u8Table);
    status &= dvProAV_AccessWrite(eEgblBiasBfLevelAddr, u8Level);

    status &= (dvProAV_InterfaceWriteErrorCountGet() > 0) ? rcERROR : rcSUCCESS; // check access write error count after all interface write
    return status;
}

/**
 * @brief set the data to write to the bias table.
 * @param [in] u8Data : data of bias table
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_BldBiasBufDataSet(uint08 u8Data)
{
    int status = rcSUCCESS;

    dvProAV_InterfaceWriteErrorCountGet(); // clear access write error count before any interface write

    status &= dvProAV_AccessWrite(eEgblCorrBfData, (uint32)u8Data);

    status &= (dvProAV_InterfaceWriteErrorCountGet() > 0) ? rcERROR : rcSUCCESS; // check access write error count after all interface write
    return status;
}

/**
 * @brief Download the Edge blending bias Table to Chip
 * @param [in] location:  location of blending
 *        - 0  : middle
 *        - 1  : bottom middle
 *        - 2  : top middle
 *        - 3  : (n/a)
 *        - 4  : right middle
 *        - 5  : right bottom
 *        - 6  : right top
 *        - 7  : (n/a)
 *        - 8  : left middle
 *        - 9  : left bottom
 *        - 10 : left top
 *        - 3, 7, 11~15 : n/a
 * @param [in] biasTable : pointer of bias table
 * @param [in] colorMode
 *        - 0 : R channel
 *        - 1 : G channel
 *        - 2 : B channel
 *        - 3 : RGB channel
 *
 * @note size of biasTable is 16*9 = 144 when color is RGB
 *       size of biasTable is 48*9 = 432 when color is RGB split
 *
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_BldBiasDl2Chip(uint08 u8Location, uint08 *u8BiasTable, uint08 u8ColorMode)
{
    int status = rcSUCCESS;
    if(u8ColorMode > 3)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }

    dvProAV_InterfaceWriteErrorCountGet(); // clear access write error count before any interface write

    // Write the value into the buffer enable
    status &= dvProAV_AccessWrite(eEgblBiasWr, (uint32)false);  //set buffer to read mode
    status &= dvProAV_AccessWrite(eEgblBiasBfTableAddr, u8Location);
    status &= dvProAV_AccessWrite(eEgblBiasBfLevelAddr, (uint32)false); // set the level address to 0, and incement address when wrote eEgblCorrBfData
    status &= dvProAV_AccessWrite(eEgblBiasWr, (uint32)true);  //set buffer to write mode

    status &= dvProAV_AccessWrite(eEgblBiasCh, (uint32)u8ColorMode);  // 0: R, 1: G, 2:B, 3: RGB
    status &= dvProAV_AccessBurstWrite(eEgblCorrBfData, u8BiasTable, 16, BURST_FIX_ADDR); // Fixed Address if last parameter is false

    status &= dvProAV_AccessWrite(eEgblBiasWr, (uint32)false); // set buffer to read mode

    status &= (dvProAV_InterfaceWriteErrorCountGet() > 0) ? rcERROR : rcSUCCESS; // check access write error count after all interface write
    return status;
}

/**
 * @brief set Edge blending gamma function enable
 * @param [in] gammaType
 *        - 0 : front-gamma
 *        - 1 : back-gamma
 * @param [in] enable: enable/disbale front-gamma
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_BldGammaEnableSet(uint08 u8GammaType, bool bEnable)
{
    int status = rcSUCCESS;
    if (u8GammaType != 0)
    {
        u8GammaType = 1;
    }
    dvProAV_InterfaceWriteErrorCountGet(); // clear access write error count before any interface write

    status &= dvProAV_AccessWrite(m_cuEmuGammaEnable[u8GammaType], (uint32)bEnable);

    status &= (dvProAV_InterfaceWriteErrorCountGet() > 0) ? rcERROR : rcSUCCESS; // check access write error count after all interface write
    return status;
}

/**
 * @brief get Edge blending gamma function enable
 * @param [in] u8GammaType
 *        - 0 : front-gamma
 *        - 1 : back-gamma
 * @return False : front-gamma / True : back-gamma
 */
int dvProAV_BldGammaEnableGet(uint08 u8GammaType)
{
    int status = rcSUCCESS;
    if (u8GammaType != 0)
    {
        u8GammaType = 1;
    }
    uint32 ucValue = 0;

    status &= dvProAV_AccessRead(m_cuEmuGammaEnable[u8GammaType], (uint32*)&ucValue);

    if(ucValue)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/**
 * @brief set Edge blending gamma dithering enable mode and add dithering value
 * @param [in] u8GammaType
 *        - 0 : front-gamma
 *        - 1 : back-gamma
 * @param [in] u8Mode :
 *         - bit 0 : temporal enable
 *         - bit 1 : spatial enable
 *         - bit 2~7: unused
 * @param [in] u8Value: value of dithering add (0~3)
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_BldGammaDitherEnableSet(uint08 u8GammaType, uint08 u8Mode, uint08 u8Value)
{
    int status = rcSUCCESS;
    if (u8GammaType != 0)
    {
        u8GammaType = 1;
    }

    dvProAV_InterfaceWriteErrorCountGet(); // clear access write error count before any interface write

    status &= dvProAV_AccessWrite(m_cuEmuGammaDithEn[u8GammaType], (uint32)u8Mode);
    status &= dvProAV_AccessWrite(m_cuEmuGammaDithValue[u8GammaType], (uint32)u8Value);

    status &= (dvProAV_InterfaceWriteErrorCountGet() > 0) ? rcERROR : rcSUCCESS; // check access write error count after all interface write
    return status;
}

/**
 * @brief Download the Edge blending back-gamma Table  to Chip
 * @param [in] gammaType
 *        - 0 : front-gamma
 *        - 1 : back-gamma
 * @param [in] ucGammaStepBit 10 / 12 bit
 * @note The gamma table setting is used the Vop Clock, so the setting should after the Vop Clock setting
 * @return status : success or fail
 */
int dvProAV_BldGammaDl2Chip(uint08 u8GammaType, uint08 u8GammaStepBit)
{
    int status;
    uint32 ColorSplited = 0;
    uint08 ColorCount;
    uint8 buf[8192] = {0};

    if (u8GammaType != 0)
    {
        u8GammaType = 1;
    }

    uint16 GammaSteps = (uint16)(1 << u8GammaStepBit); //uint16 GammaSteps = dvProAV_SclGammaItemCountGet();
    if(GammaSteps > (sizeof (buf) / 2))
        return rcERROR;

    status = rcSUCCESS;

    dvProAV_InterfaceWriteErrorCountGet(); // clear access write error count before any interface write

    // Write the value into the buffer enable
    status &= dvProAV_AccessWrite(m_cuEmuGammaBufWE[u8GammaType], (uint32)true);  //set buffer to write mode

    // get RGB use the same/different table
    status &= dvProAV_AccessRead(m_cuEmuGammaRgbSep[u8GammaType], &ColorSplited);
    if (status == rcSUCCESS && ColorSplited != 0)
        ColorCount = 3;
    else
        ColorCount = 1;


    for(uint08 c=0; c < ColorCount; c++)
    {
        uint16 *table = dvProAV_SclGammaTableGet(c);
        for(uint16 i=0; i<GammaSteps; i++)
        {
            buf[i*2 + 0] = table[i] & 0xff;
            buf[i*2 + 1] = table[i] >> 8;
        }

        status &= dvProAV_AccessBurstWrite(m_cuEmuGammaBufPort[u8GammaType], &buf[0], GammaSteps*2, BURST_FIX_ADDR);
    }

    status &= dvProAV_AccessWrite(m_cuEmuGammaBufWE[u8GammaType], (uint32)false); // set buffer to read mode
    // Download the value from buffer to Gamma table
    status &= dvProAV_AccessWrite(m_cuEmuGammaBufDl[u8GammaType], (uint32)true);
    // 更新完，關閉寫入Gamma Table
    MS_SLEEP(BLD_GAMMA_TABLE_UPDATE_DELAY);
    status &= dvProAV_AccessWrite(m_cuEmuGammaBufDl[u8GammaType], (uint32)false);

    status &= (dvProAV_InterfaceWriteErrorCountGet() > 0) ? rcERROR : rcSUCCESS; // check access write error count after all interface write
    return status;
}

/**
 * @brief Back Gamma Write Select
 * @param [in] bSel
 *          false : select low gamma to write
 *          true : select high gamma to write
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_BldBackGammaWriteSelect(bool bSel)
{
    int status = rcSUCCESS;
    status &= dvProAV_AccessWrite(eBldBackGmaSel, bSel);
    return status;
}

/**
 * @brief Set Gamma Table Index by gamma type
 * @param [in] u8GammaType
 *        - 0 : front-gamma
 *        - 1 : back-gamma
 * @param [in] dPower : gamma power
 * @note
 *        1. Do nothing when the current gamma index .equ. the new gamma index
 *        2. RGB Gamma is not splitted
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_BldGammaPowerSet(uint08 u8GammaType, double dPower)
{
    int status = rcSUCCESS;

    if (u8GammaType != 0)
    {
        u8GammaType = 1;
    }

    if (dPower == 1.0 || dPower <= 0.0)
    {
        dvProAV_InterfaceWriteErrorCountGet(); // clear access write error count before any interface write
        status &= dvProAV_AccessWrite(m_cuEmuGammaEnable[u8GammaType], (uint32)false);
    }
    else
    {
        //dvProAV_SclGammaTableCaculate(power, 0);
        if(u8GammaType) // back gamma
        {
            // back gamma High
            status &= dvProAV_BldBackGammaWriteSelect(true);
            dvProAV_BldGammaTableCaculate(12, 12, 12, dPower, 0);
            dvProAV_BldGammaDl2Chip(u8GammaType, 12);
            // back gamma Low
            status &= dvProAV_BldBackGammaWriteSelect(false);
            dvProAV_BldGammaTableCaculate(10, 12, 16, dPower, 0);
            dvProAV_BldGammaDl2Chip(u8GammaType, 10);
        }
        else // front gamma
        {
            dvProAV_BldGammaTableCaculate(10, 16, 10, dPower, 0);
            dvProAV_BldGammaDl2Chip(u8GammaType, 10);
        }
        dvProAV_InterfaceWriteErrorCountGet(); // clear access write error count before any interface write
        status &= dvProAV_AccessWrite(m_cuEmuGammaEnable[u8GammaType], (uint32)true);
    }

    status &= (dvProAV_InterfaceWriteErrorCountGet() > 0) ? rcERROR : rcSUCCESS; // check access write error count after all interface write
    return (status == rcSUCCESS);
}

//-------------------------------------------------------------------------------------------------
// Dot-By-Dot Blend
//-------------------------------------------------------------------------------------------------
/**
 * @brief Set DotByDot Blending Dram Address by memory id
 * @param [in] bMemId : memory ID
 * @param [in] u32Addr : DRAM Address
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_BldDBDDramAddrSet(bool bMemId, uint32 u32Addr)
{
    int status = rcSUCCESS;

    dvProAV_InterfaceWriteErrorCountGet(); // clear access write error count before any interface write

    if (bMemId)
        status &= dvProAV_AccessWrite(eDbdRealBaseAdr1, u32Addr);
    else
        status &= dvProAV_AccessWrite(eDbdRealBaseAdr0, u32Addr);

    status &= (dvProAV_InterfaceWriteErrorCountGet() > 0) ? rcERROR : rcSUCCESS; // check access write error count after all interface write
    return status;
}

/**
 * @brief Set DotByDot Blending Gain(weighting) Base Address
 * @param [in] u32Addr : Gain Base Address
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_BldDBDGainBaseAddrSet(uint32 u32Addr)
{
    int status = rcSUCCESS;

    dvProAV_InterfaceWriteErrorCountGet(); // clear access write error count before any interface write

    status &= dvProAV_AccessWrite(eDbdBaseAdr, u32Addr);

    status &= (dvProAV_InterfaceWriteErrorCountGet() > 0) ? rcERROR : rcSUCCESS; // check access write error count after all interface write
    return status;
}

/**
 * @brief Set DotByDot Blending Bias(blacklevel) Base Address
 * @param [in] u32Addr : Bias Base Address
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_BldDBDBiasBaseAddrSet(uint32 u32Addr)
{
    int status = rcSUCCESS;

    dvProAV_InterfaceWriteErrorCountGet(); // clear access write error count before any interface write

    status &= dvProAV_AccessWrite(eDbdBiasBaseAdr, u32Addr);

    status &= (dvProAV_InterfaceWriteErrorCountGet() > 0) ? rcERROR : rcSUCCESS; // check access write error count after all interface write
    return status;
}

/**
 * @brief Set DotByDot Blending Block Address
 * @param [in] u32Addr : Block Address
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_BldDBDBlockAddrSet(uint32 u32Addr)
{
    int status = rcSUCCESS;
    dvProAV_InterfaceWriteErrorCountGet(); // clear access write error count before any interface write

    status &= dvProAV_AccessWrite(eDbdBlockBaseAdr, u32Addr);
    m_BlockAddr = u32Addr;

    status &= (dvProAV_InterfaceWriteErrorCountGet() > 0) ? rcERROR : rcSUCCESS; // check access write error count after all interface write
    return status;
}

/**
 * @brief Set DotByDot Blending Block Size
 * @param [in] u16Width : Block Width
 * @param [in] u16Height : height Block Height
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_BldDBDBlockSizeSet(uint16 u16Width, uint16 u16Height)
{
    int status = rcSUCCESS;
    uint08 unit = m_cuUnitList[m_DbdDataMode % 3];

    u16Width = ((u16Width + unit -1)/ unit); //Roundup

    dvProAV_InterfaceWriteErrorCountGet(); // clear access write error count before any interface write

    status &= dvProAV_AccessWrite(eDbdBlockWidth, u16Width);
    status &= dvProAV_AccessWrite(eDbdBlockHeight, u16Height);

    status &= (dvProAV_InterfaceWriteErrorCountGet() > 0) ? rcERROR : rcSUCCESS; // check access write error count after all interface write
    return status;
}

/**
 * @brief Set DotByDot Blending Read Size
 * @param [in] u16Hsize : Horizontal Block Read Size
 * @param [in] u16Vsize : Vertical Block Read Size
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_BldDBDReadSizeSet(uint16 u16Hsize, uint16 u16Vsize)
{
    int status = rcSUCCESS;
    uint08 unit = m_cuUnitList[m_DbdDataMode % 3];
    u16Hsize = ((u16Hsize + unit -1) / unit); //Roundup

    dvProAV_InterfaceWriteErrorCountGet(); // clear access write error count before any interface write

    status &= dvProAV_AccessWrite(eDbdHReadSize, u16Hsize);
    status &= dvProAV_AccessWrite(eDbdVReadSize, u16Vsize);

    status &= (dvProAV_InterfaceWriteErrorCountGet() > 0) ? rcERROR : rcSUCCESS; // check access write error count after all interface write
    return status;
}
/**
 * @brief Set DotByDot Blending Read Size
 * @param [in] u8Mode
 *        - 0 : 8 bits weighting
 *        - 1 : 10 bits weighting
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_BldDBDGainModeSet(uint08 u8Mode)
{
    int status = rcSUCCESS;
    if(u8Mode > 1)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }

    dvProAV_InterfaceWriteErrorCountGet(); // clear access write error count before any interface write

    status &= dvProAV_AccessWrite(eDbdWeightMode, u8Mode);

    status &= (dvProAV_InterfaceWriteErrorCountGet() > 0) ? rcERROR : rcSUCCESS; // check access write error count after all interface write
    return status;
}

/**
 * @brief Set DotByDot Blending Data Mode
 * @param [in] u8Mode
 *        - 0 : 8 bits weighting (64 in 64 bytes)
 *        - 1 : 10 bits weighting (48 in 64 bytes)
 *        - 2 : 4 bits bias (128 in 64 bytes)
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_BldDbdDataModeSet(uint08 u8Mode)
{
    int status = rcSUCCESS;
    if(u8Mode > 2)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }

    m_DbdDataMode = u8Mode % 3; // % 3 to limit the value to 2

    if (m_DbdDataMode == 0 || m_DbdDataMode == 1)
    {
        // set Weighting Mode when data mode is 0 or 1
        status &= dvProAV_BldDBDGainModeSet(m_DbdDataMode);
    }

    return status;
}

/**
 * @brief Get DotByDot Blending Data Mode
 * @return
 *        - 0 : 8 bits weighting
 *        - 1 : 10 bits weighting
 *        - 2 : 4 bits bias
 */
uint08 dvProAV_BldDbdDataModeGet(void)
{
    return (m_DbdDataMode % 3); // % 3 to limit the value is 2
}

/**
 * @brief Set DotByDot Blending Data
 * @param [in] bMemId : memory ID
 * @param [in] u8DataMode : Data Mode
 *        - 0 : 8 bits weighting (64 pixel in 64 bytes)
 *        - 1 : 10 bits weighting (48 pixel in 64 bytes)
 *        - 2 : 4 bits bias (128 pixel in 64 bytes)
 * @param [in] u8Data : Point of Blending Data
 * @param [in] u32DataLen : Length of Blending Data
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_BldDBDDataSet(bool bMemId, uint08 u8DataMode, uint08 *u8Data, uint32 u32DataLen)
{
    int status = rcSUCCESS;
    if((u8DataMode > 2) || (u32DataLen == 0))
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }
    uint32 addr;
    status &= dvProAV_BldDbdDataModeSet(u8DataMode);

    dvProAV_InterfaceWriteErrorCountGet(); // clear access write error count before any interface write

    status &= dvProAV_AccessWrite(eDbdWriteRealBaseAdrSel, (uint32)bMemId);


    if (u8DataMode == 0 || u8DataMode == 1)
    {
        addr = (bMemId == 0)? DRAM_DBDBLD_GAIN_ADDR0 : DRAM_DBDBLD_GAIN_ADDR1;

        status &= dvProAV_AccessWrite(eDbdGainClear, false);   // Disable DBD clear
        status &= dvProAV_AccessWrite(eDbdGainWriteEn, false); // Disable DBD Write Enable
        status &= dvProAV_AccessWrite(eDbdEnableDBD, true);    // Enable DBD Gain
    }
    else
    {
        addr = (bMemId == 0)? DRAM_DBDBLD_BIAS_ADDR0 : DRAM_DBDBLD_BIAS_ADDR1;

        status &= dvProAV_AccessWrite(eDbdBiasClear, false); // Disable DBD clear
        status &= dvProAV_AccessWrite(eDbdBiasEn, true);     // Enable DBD Bias
    }
    status &= dvProAV_LineBufAccess(eDramBank1, addr, u8Data, u32DataLen, IO_WRITE);
    status &= (dvProAV_InterfaceWriteErrorCountGet() > 0) ? rcERROR : rcSUCCESS; // check access write error count after all interface write
    status &= dvProAV_AccessWrite(eDbdReadRealBaseAdrSel, (uint32)bMemId);

    return status;
}

/**
 * @brief Set DotByDot Blending Bias Enable or Disable
 * @param [in] bEnable : Enable the Blending Bias Function
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_BldDBDBiasEnableSet(bool bEnable)
{
    int status = rcSUCCESS;

    dvProAV_InterfaceWriteErrorCountGet(); // clear access write error count before any interface write

    status &= dvProAV_AccessWrite(eDbdBiasEn, (uint32)bEnable);

    status &= (dvProAV_InterfaceWriteErrorCountGet() > 0) ? rcERROR : rcSUCCESS; // check access write error count after all interface write
    return status;
}

/**
 * @brief Set DotByDot Blending Bias Table Data
 * @param [in] u8Location : Number of the table location
 * @param [in] u8BiasTable : Point of Bias Table Data
 * @param [in] u8ColorMode :
 *        - 0 : R channel
 *        - 1 : G channel
 *        - 2 : B channel
 *        - 3 : RGB channels
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_BldDBDBiasDl2Chip(uint08 u8Location, uint08 *u8BiasTable, uint08 u8ColorMode)
{
    int status = rcSUCCESS;

    if(u8ColorMode >= 4)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }

    dvProAV_InterfaceWriteErrorCountGet(); // clear access write error count before any interface write

    // Write the value into the buffer enable
    status &= dvProAV_AccessWrite(eDbdBiasEn, (uint32)false); // Disable bias function

    status &= dvProAV_AccessWrite(eDbdBiasBfTableAddr, (uint32)(u8Location & 0x0f));
    status &= dvProAV_AccessWrite(eDbdBiasBfLevelAddr, (uint32)false);

    status &= dvProAV_AccessWrite(eDbdBiasCh, (uint32)u8ColorMode);  // 0: R, 1: G, 2:B, 3: RGB
    status &= dvProAV_AccessWrite(eDbdBiasTableWrite, (uint32)true);  // set buffer to write mode

    status &= dvProAV_AccessBurstWrite(eDbdCorrBfData, u8BiasTable , 16, BURST_FIX_ADDR); // Fixed Address if last parameter is false

    status &= dvProAV_AccessWrite(eDbdBiasTableWrite, (uint32)false);  // set buffer to read mode
    status &= dvProAV_AccessWrite(eDbdBiasEn, (uint32)true); // Enable bias function

    status &= (dvProAV_InterfaceWriteErrorCountGet() > 0) ? rcERROR : rcSUCCESS; // check access write error count after all interface write
    return status;
}

/**
 * @brief Initialize the DotByDot Blending Bias Function
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_BldDBDInit(void)
{
    int status = rcSUCCESS;

    dvProAV_InterfaceWriteErrorCountGet(); // clear access write error count before any interface write

    DBMSG("set real DRAM adress of Gain table 0\n");
    status &= dvProAV_AccessWrite(eDbdRealBaseAdr0, (uint32)DRAM_DBDBLD_GAIN_ADDR0);
    //set real DRAM adress of Gain table 1
    status &= dvProAV_AccessWrite(eDbdRealBaseAdr1, (uint32)DRAM_DBDBLD_GAIN_ADDR1);

    //set real DRAM adress of BIAS table 0
    DBMSG("set real DRAM adress of BIAS table 0\n");
    status &= dvProAV_AccessWrite(eDbdBiasRealBaseAdr0, (uint32)DRAM_DBDBLD_BIAS_ADDR0);

    //set real DRAM adress of BIAS table 1
    DBMSG("set real DRAM adress of BIAS table 1\n");
    status &= dvProAV_AccessWrite(eDbdBiasRealBaseAdr1, (uint32)DRAM_DBDBLD_BIAS_ADDR1);

    status &= (dvProAV_InterfaceWriteErrorCountGet() > 0) ? rcERROR : rcSUCCESS; // check access write error count after all interface write
    return status;
}

/**
 * @brief Reset the DotByDot Blending Function
 * @param [in] bMemBase : Memory ID
 * @param [in] u8DataMode: Data Mode
 * @param [in] u16ResH : Horizontal Resolution
 * @param [in] u16ResV : Vertial Resolution
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_BldDBDReset(bool bMemBase, uint08 u8DataMode, uint16 u16ResH, uint16 u16ResV)
{
    int status = rcSUCCESS;

    m_PanelWidth = u16ResH;
    m_PanelHeight = u16ResV;

    dvProAV_InterfaceWriteErrorCountGet(); // clear access write error count before any interface write

    //set Read Real Base Address Selection
    DBMSG("set Read real Base Address Selection");
    status &= dvProAV_AccessWrite(eDbdReadRealBaseAdrSel, (uint32)bMemBase);

    //set Write Real Base Address Selection
    DBMSG("set Write real Base Address Selection");
    status &= dvProAV_AccessWrite(eDbdWriteRealBaseAdrSel, (uint32)bMemBase);

    status &= (dvProAV_InterfaceWriteErrorCountGet() > 0) ? rcERROR : rcSUCCESS; // check access write error count after all interface write

    //set port for weighting
    DBMSG("set port data mode =%d\n", dataMode);
    status &= dvProAV_BldDbdDataModeSet(u8DataMode);

    if (u8DataMode ==0 || u8DataMode == 1)
    {
        //set related addrees to 0
        DBMSG("set related addrees to 0");
        status &= dvProAV_BldDBDBlockAddrSet(0);

        //set block size
        DBMSG("set block size");
        status &= dvProAV_BldDBDBlockSizeSet(u16ResH, u16ResV);

        //set read size
        DBMSG("set read size");
        status &= dvProAV_BldDBDReadSizeSet(u16ResH, u16ResV);


        dvProAV_InterfaceWriteErrorCountGet(); // clear access write error count before any interface write

        //Clear Dot by Dot Gain DRAM
        DBMSG("Clear Dot by Dot Gain DRAM");
        status &= dvProAV_AccessWrite(eDbdGainClear, (uint32)true);
        //status &= dvProAV_ClearDram(eDramBank1, (memBase == 0)? DRAM_DBDBLD_GAIN_ADDR0 : DRAM_DBDBLD_GAIN_ADDR0, (hRes * vRes), 0xff);

        //Enable Dot by Dot
        //DBMSG("Enable Dot by Dot");
        //status &= dvProAV_AccessWrite(eDbdEnableDBD, (uint32)true);

        status &= (dvProAV_InterfaceWriteErrorCountGet() > 0) ? rcERROR : rcSUCCESS; // check access write error count after all interface write
    }
    else
    {
        uint08 biasData[16];

        // write 0 for clear data
        for (uint08 idx = 0; idx <16 ; idx++)
        {
            biasData[idx]= 0;
        }

        for (uint08 loc = 0; loc <16 ; loc++)
        {
            //clear R&G&B color, Bias download to Chip
            DBMSG("R&G&B color, Bias location %02d download to of Chip \n", loc);
            status &= dvProAV_BldDBDBiasDl2Chip(loc, (uint08 *)&biasData, 3); // 3: for R&G&B 3 channels of color
        }

        //set related addrees to 0
        DBMSG("set related addrees to 0");
        status &= dvProAV_BldDBDBlockAddrSet(0);

        //set block size
        DBMSG("set block size\n");
        status &= dvProAV_BldDBDBlockSizeSet(u16ResH, u16ResV);

        //set read size
        DBMSG("set read size\n");
        status &= dvProAV_BldDBDReadSizeSet(u16ResH, u16ResV);


        dvProAV_InterfaceWriteErrorCountGet(); // clear access write error count before any interface write

        //set related addrees to 0
        DBMSG("set related addrees to 0\n");
        status &= dvProAV_AccessWrite(eDbdBiasBaseAdr, 0);

        // Clear Dot by Dot Bias DRAM
        DBMSG("Clear Dot by Dot Bias DRAM\n");
        status &= dvProAV_AccessWrite(eDbdBiasClear, (uint32)true);
        //status &= dvProAV_ClearDram(eDramBank1, (memBase == 0)? DRAM_DBDBLD_BIAS_ADDR0 : DRAM_DBDBLD_BIAS_ADDR1, ((hRes * vRes) / 2), 0xff);

        //Enable Dot by Dot Bias
        DBMSG("set Write real Base Address Selection\n");
        status &= dvProAV_AccessWrite(eDbdBiasEn,(uint32)true);

        status &= (dvProAV_InterfaceWriteErrorCountGet() > 0) ? rcERROR : rcSUCCESS; // check access write error count after all interface write
    }
    DBMSG("Dot by Dot Reset done!\n");

    return status;
}

/**
 * @brief Set the DotByDot Blending Function to Enable or Disable
 * @param [in] bEnable : Enable/Disbale the DotByDot Blending Function
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_BldDBDEnableSet(bool bEnable)
{
    int status = rcSUCCESS;

    dvProAV_InterfaceWriteErrorCountGet(); // clear access write error count before any interface write

    status &= dvProAV_AccessWrite(eDbdEnableDBD, (uint32)bEnable); // Enable Dot by Dot

    status &= (dvProAV_InterfaceWriteErrorCountGet() > 0) ? rcERROR : rcSUCCESS; // check access write error count after all interface write
    return status;
}

/**
 * @brief Get the DotByDot Blending Function to Enable or Disable
 * @return
 *      false : DBD Disable
 *      true : DBD Enable
 */
bool dvProAV_BldDBDEnableGet(void)
{
    int status = rcSUCCESS;
    uint32 DBDEnable = 0;

    status &= dvProAV_AccessRead(eDbdEnableDBD, &DBDEnable);

    return (bool)DBDEnable;
}
//-------------------------------------------------------------------------------------------------
// 4K3D Blend
//-------------------------------------------------------------------------------------------------
/**
 * @brief DotByDot Blending Data Conver To 4K3D
 * @param [in] u8DataMode : Data Mode
 *        - 0 : 8 bits weighting (64 pixel in 64 bytes)
 *        - 1 : 10 bits weighting (48 pixel in 64 bytes)
 *        - 2 : 4 bits bias (128 pixel in 64 bytes)
 * @param [in] u8Data : Point of Blending Data
 * @param [in] u32DataLen : Length of Blending Data
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 * @todo add 10 & 4bit mode
 */
int dvProAV_BldDBDDataSet_4k3d(uint08 u8DataMode, uint08 *u8Data, uint32 u32DataLen, uint16 u16Hsize, uint16 u16Vsize)
{
    int status = rcSUCCESS;
    if((u8DataMode > 2) || (u32DataLen == 0))
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }

    const uint8 u8SubFmCnt = 2;
    uint8 memId = 0;
    uint32 addr;
    uint32 u32SubFrameSz = ((((u16Hsize * u16Vsize / u8SubFmCnt) - 1) / PROAV_DRAM_LINE_BYTES) + 1) * PROAV_DRAM_LINE_BYTES;
    uint8 **u8Array = (uint8**)malloc(u8SubFmCnt * sizeof(uint8*));
    uint8 *u8ArraySpace = (uint8*)malloc(u8SubFmCnt * u32SubFrameSz * sizeof(uint8*));
    for (uint8 i=0; i<u8SubFmCnt; i++)
        u8Array[i] = u8ArraySpace + i * u32SubFrameSz;

    uint32 u32DataOfst = 0, u32DataOfst0 = 0, u32DataOfst1 = 0;
    for(uint32 u32VCnt=0; u32VCnt<u16Vsize; u32VCnt++ )
    {
        for(uint32 u32HCnt=0; u32HCnt<(u16Hsize/2); u32HCnt++)
        {
            u32DataOfst = (2 * u32HCnt) + (u32VCnt * u16Hsize);
            if((u32VCnt) % 2 == 0)
            {
                u8Array[0][u32DataOfst0++] = u8Data[u32DataOfst];
                u8Array[1][u32DataOfst1++] = u8Data[u32DataOfst + 1];
            }
            else
            {
                u8Array[1][u32DataOfst1++] = u8Data[u32DataOfst];
                u8Array[0][u32DataOfst0++] = u8Data[u32DataOfst + 1];
            }
        }
    }

    status &= dvProAV_BldDbdDataModeSet(u8DataMode);

    dvProAV_InterfaceWriteErrorCountGet(); // clear access write error count before any interface write

    status &= dvProAV_AccessWrite(eDbdWriteRealBaseAdrSel, (uint32)memId);

    if (u8DataMode == 0 || u8DataMode == 1)
    {
        addr = (memId == 0)? DRAM_DBDBLD_GAIN_ADDR0 : DRAM_DBDBLD_GAIN_ADDR1;

        status &= dvProAV_AccessWrite(eDbdGainClear, false);   // Disable DBD clear
        status &= dvProAV_AccessWrite(eDbdGainWriteEn, false); // Disable DBD Write Enable
        status &= dvProAV_AccessWrite(eDbdEnableDBD, true);    // Enable DBD Gain
    }
    else
    {
        addr = (memId == 0)? DRAM_DBDBLD_BIAS_ADDR0 : DRAM_DBDBLD_BIAS_ADDR1;

        status &= dvProAV_AccessWrite(eDbdBiasClear, false); // Disable DBD clear
        status &= dvProAV_AccessWrite(eDbdBiasEn, true);     // Enable DBD Bias
    }
    status &= dvProAV_LineBufAccess(eDramBank1, addr, &u8Array[0][0], (u32DataOfst0 + u32DataOfst1), IO_WRITE);
    status &= (dvProAV_InterfaceWriteErrorCountGet() > 0) ? rcERROR : rcSUCCESS; // check access write error count after all interface write
    status &= dvProAV_AccessWrite(eDbdReadRealBaseAdrSel, (uint32)memId);

    free(u8ArraySpace);
    free(u8Array);

    return status;
}

/**
 * @brief Set DotByDot Blending Gain Data Block
 * @param [in] u32AddrPich :
 *              - when 4way XPR disable : invalid
 *              - when 4way XPR enable : base address shift
 */
int dvProAV_BldDBDGainDataBlk(uint32 u32AddrPich)
{
    return dvProAV_AccessWrite(eDbdGainXprAdrSft, u32AddrPich);
}

/**
 * @brief Set DotByDot Blending Bias Data Block
 * @param [in] u32AddrPich :
 *              - when 4way XPR disable : invalid
 *              - when 4way XPR enable : base address shift
 */
int dvProAV_BldDBDBiasDataBlk(uint32 u32AddrPich)
{
    return dvProAV_AccessWrite(eDbdBiasXprAdrSft, u32AddrPich);
}

//-------------------------------------------------------------------------------------------------
// IR Dot-By-Dot Blend
//-------------------------------------------------------------------------------------------------
/**
 * @brief Initialize the DotByDot Blending Bias Function For Ir
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_BldDBDIrInit(void)
{
    int status = rcSUCCESS;

    dvProAV_InterfaceWriteErrorCountGet(); // clear access write error count before any interface write

    DBMSG("set real DRAM adress of Gain table 2\n");
    status &= dvProAV_AccessWrite(eDbdOpmREyeBaseAdr0, (uint32)DRAM_DBDBLD_GAIN_ADDR2);
    //set real DRAM adress of Gain table 1
    status &= dvProAV_AccessWrite(eDbdOpmREyeBaseAdr1, (uint32)DRAM_DBDBLD_GAIN_ADDR3);

    status &= (dvProAV_InterfaceWriteErrorCountGet() > 0) ? rcERROR : rcSUCCESS; // check access write error count after all interface write
    return status;
}

/**
 * @brief Set DotByDot Blending Data For Ir
 * @param [in] bMemId : memory ID
 * @param [in] u8DataMode : Data Mode
 *        - 0 : 8 bits weighting (64 pixel in 64 bytes)
 *        - 1 : 10 bits weighting (48 pixel in 64 bytes)
 *        - 2 : 4 bits bias (128 pixel in 64 bytes)
 * @param [in] u8Data : Point of Blending Data
 * @param [in] u32DataLen : Length of Blending Data
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_BldDBDDataSet_Ir(bool bMemId, uint08 u8DataMode, uint08 *u8Data, uint32 u32DataLen)
{
    int status = rcSUCCESS;
    if((u8DataMode > 2) || (u32DataLen == 0))
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }
    uint32 u32Addr;
    status &= dvProAV_BldDbdDataModeSet(u8DataMode);

    dvProAV_InterfaceWriteErrorCountGet(); // clear access write error count before any interface write

    status &= dvProAV_AccessWrite(eDbdWriteRealBaseAdrSel, (uint32)bMemId);


    if (u8DataMode < 2)
    {
        u32Addr = (bMemId == 0)? DRAM_DBDBLD_GAIN_ADDR2 : DRAM_DBDBLD_GAIN_ADDR3;

        status &= dvProAV_AccessWrite(eDbdGainClear, false);   // Disable DBD clear
        status &= dvProAV_AccessWrite(eDbdGainWriteEn, false); // Disable DBD Write Enable
        status &= dvProAV_AccessWrite(eDbdEnableDBD, true);    // Enable DBD Gain
    }

    status &= dvProAV_LineBufAccess(eDramBank1, u32Addr, u8Data, u32DataLen, IO_WRITE);
    status &= (dvProAV_InterfaceWriteErrorCountGet() > 0) ? rcERROR : rcSUCCESS; // check access write error count after all interface write
    status &= dvProAV_AccessWrite(eDbdReadRealBaseAdrSel, (uint32)bMemId);

    return status;
}

/**
 * @brief Inverse define of OpmREye signal
 * @param [in] bInv : false:same / true:inverse
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_BldDBDIrInvSet(bool bInv)
{
    return dvProAV_AccessWrite(eDbdOpmREyeInv, bInv);
}

/**
 * @brief Enable to detect OpmREye signal and switch base address of gain
 * @param [in] bEnable : Enable/Disbale DotByDot blending IR mode
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_BldDBDIrSet(bool bEnable)
{
    return dvProAV_AccessWrite(eDbdOpmREyeEn, bEnable);
}
