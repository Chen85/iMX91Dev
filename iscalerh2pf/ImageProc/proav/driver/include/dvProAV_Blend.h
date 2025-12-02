#ifndef DV_PROAV_BLEND_H
#define DV_PROAV_BLEND_H

#include "dvProAV_Platform.h"

#ifdef __cplusplus
extern "C"
{
#endif

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
int dvProAV_BldUpdatingEnable(bool bEnable);

/**
 * @brief set blending enable
 * @param [in] u8Mode :
 *        - bit 0: enable bottom blending
 *        - bit 1: enable top blending
 *        - bit 2: enable right blending
 *        - bit 3: enable left blending
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_BldEnableSet(uint08 u8Mode);

/**
 * @brief Get Blending Enable Status
 * @return u32BlendEn : false=disable / true=enable
 *        - bit 0: enable bottom blending
 *        - bit 1: enable top blending
 *        - bit 2: enable right blending
 *        - bit 3: enable left blending
 */
uint08 dvProAV_BldEnableGet(void);

/**
 * @brief set blending start and width of top
 * @param [in] u16Start : edge blending vertical top start position
 * @param [in] u16Width : edge blending vertical top width
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_BldTopSet(uint16 u16Start, uint16 u16Width);

/**
 * @brief set blending start and width of bottom
 * @param [in] u16Start : edge blending vertical bottom start position
 * @param [in] width : edge blending vertical bottom width
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_BldBottomSet(uint16 u16Start, uint16 u16Width);

/**
 * @brief set blending start and width of left
 * @param [in] u16Start : edge blending horizontal left start position
 * @param [in] u16Width : edge blending horizontal left width
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_BldLeftSet(uint16 u16Start, uint16 u16Width);

/**
 * @brief set blending start and width of right
 * @param [in] u16Start : edge blending horizontal right start position
 * @param [in] u16Width : edge blending horizontal right width
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_BldRightSet(uint16 u16Start, uint16 u16Width);

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
int dvProAV_BldBiasAccessSet(uint8 u8Channel, bool bWrite);

/**
 * @brief set the edge-blending black level correction function enable
 * @param [in] bEnable : enable/disbale edge-blending black level correction function
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_BldBiasEnableSet(bool bEnable);

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
int dvProAV_BldBiasBufAddrSet(uint08 u8Table, uint08 u8Level);

/**
 * @brief set the data to write to the bias table.
 * @param [in] u8Data : data of bias table
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_BldBiasBufDataSet(uint08 u8Data);

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
int dvProAV_BldBiasDl2Chip(uint08 u8Location, uint08 *u8BiasTable, uint08 u8ColorMode);

/**
 * @brief set Edge blending gamma function enable
 * @param [in] u8GammaType
 *        - 0 : front-gamma
 *        - 1 : back-gamma
 * @param [in] enable: enable/disbale front-gamma
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_BldGammaEnableSet(uint08 u8GammaType, bool bEnable);

/**
 * @brief get Edge blending gamma function enable
 * @param [in] u8GammaType
 *        - 0 : front-gamma
 *        - 1 : back-gamma
 * @return False : front-gamma / True : back-gamma
 */
int dvProAV_BldGammaEnableGet(uint08 u8GammaType);

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
int dvProAV_BldGammaDitherEnableSet(uint08 u8GammaType, uint08 u8Mode, uint08 u8Value);

/**
 * @brief Download the Edge blending back-gamma Table  to Chip
 * @param [in] gammaType
 *        - 0 : front-gamma
 *        - 1 : back-gamma
 * @param [in] ucGammaStepBit 10 / 12 bit
 * @note The gamma table setting is used the Vop Clock, so the setting should after the Vop Clock setting
 * @return status : success or fail
 */
int dvProAV_BldGammaDl2Chip(uint08 u8GammaType, uint08 u8GammaStepBit);

/**
 * @brief Back Gamma Write Select
 * @param  bSel
 *          false : select low gamma to write
 *          true : select high gamma to write
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_BldBackGammaWriteSelect(bool bSel);

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
int dvProAV_BldGammaPowerSet(uint08 u8GammaType, double dPower);

//-------------------------------------------------------------------------------------------------
// Dot-By-Dot Blend
//-------------------------------------------------------------------------------------------------
/**
 * @brief Set DotByDot Blending Dram Address by memory id
 * @param [in] bMemId : memory ID
 * @param [in] u32Addr : DRAM Address
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_BldDBDDramAddrSet(bool bMemId, uint32 u32Addr);

/**
 * @brief Set DotByDot Blending Gain(weighting) Base Address
 * @param [in] u32Addr : Gain Base Address
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_BldDBDGainBaseAddrSet(uint32 u32Addr);

/**
 * @brief Set DotByDot Blending Bias(blacklevel) Base Address
 * @param [in] u32Addr : Bias Base Address
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_BldDBDBiasBaseAddrSet(uint32 u32Addr);

/**
 * @brief Set DotByDot Blending Block Address
 * @param [in] u32Addr : Block Address
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_BldDBDBlockAddrSet(uint32 u32Addr);

/**
 * @brief Set DotByDot Blending Block Size
 * @param [in] u16Width : Block Width
 * @param [in] u16Height : height Block Height
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_BldDBDBlockSizeSet(uint16 u16Width, uint16 u16Height);

/**
 * @brief Set DotByDot Blending Read Size
 * @param [in] u16Hsize : Horizontal Block Read Size
 * @param [in] u16Vsize : Vertical Block Read Size
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_BldDBDReadSizeSet(uint16 u16Hsize, uint16 u16Vsize);

/**
 * @brief Set DotByDot Blending Data Mode
 * @param [in] u8Mode
 *        - 0 : 8 bits weighting (64 in 64 bytes)
 *        - 1 : 10 bits weighting (48 in 64 bytes)
 *        - 2 : 4 bits bias (128 in 64 bytes)
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_BldDbdDataModeSet(uint08 u8Mode);

/**
 * @brief Set DotByDot Blending Data Mode
 * @param mode
 *        - 0 : 8 bits weighting (64 in 64 bytes)
 *        - 1 : 10 bits weighting (48 in 64 bytes)
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_BldDbdDataModeSet(uint08 mode);

/**
 * @brief Get DotByDot Blending Data Mode
 * @return
 *        - 0 : 8 bits weighting
 *        - 1 : 10 bits weighting
 *        - 2 : 4 bits bias
 */
uint08 dvProAV_BldDbdDataModeGet(void);

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
int dvProAV_BldDBDDataSet(bool bMemId, uint08 u8DataMode, uint08 *u8Data, uint32 u32DataLen);

/**
 * @brief Set DotByDot Blending Bias Enable or Disable
 * @param [in] bEnable : Enable the Blending Bias Function
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_BldDBDBiasEnableSet(bool bEnable);

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
int dvProAV_BldDBDBiasDl2Chip(uint08 u8Location, uint08 *u8BiasTable, uint08 u8ColorMode);

/**
 * @brief Initialize the DotByDot Blending Bias Function
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_BldDBDInit(void);

/**
 * @brief Reset the DotByDot Blending Function
 * @param [in] bMemBase : Memory ID
 * @param [in] u8DataMode: Data Mode
 * @param [in] u16ResH : Horizontal Resolution
 * @param [in] u16ResV : Vertial Resolution
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_BldDBDReset(bool bMemBase, uint08 u8DataMode, uint16 u16ResH, uint16 u16ResV);

/**
 * @brief Set the DotByDot Blending Function to Enable or Disable
 * @param [in] bEnable : Enable/Disbale the DotByDot Blending Function
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_BldDBDEnableSet(bool bEnable);

/**
 * @brief Get the DotByDot Blending Function to Enable or Disable
 * @return
 *      false : DBD Disable
 *      true : DBD Enable
 */
bool dvProAV_BldDBDEnableGet(void);

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
int dvProAV_BldDBDDataSet_4k3d(uint08 u8DataMode, uint08 *u8Data, uint32 u32DataLen, uint16 u16Hsize, uint16 u16Vsize);

/**
 * @brief Set DotByDot Blending Gain Data Block
 * @param [in] u32AddrPich :
 *              - when 4way XPR disable : invalid
 *              - when 4way XPR enable : base address shift
 */
int dvProAV_BldDBDGainDataBlk(uint32 u32AddrPich);

/**
 * @brief Set DotByDot Blending Bias Data Block
 * @param [in] u32AddrPich :
 *              - when 4way XPR disable : invalid
 *              - when 4way XPR enable : base address shift
 */
int dvProAV_BldDBDBiasDataBlk(uint32 u32AddrPich);

//-------------------------------------------------------------------------------------------------
// IR Dot-By-Dot Blend
//-------------------------------------------------------------------------------------------------
/**
 * @brief Initialize the DotByDot Blending Bias Function For Ir
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_BldDBDIrInit(void);

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
int dvProAV_BldDBDDataSet_Ir(bool bMemId, uint08 u8DataMode, uint08 *u8Data, uint32 u32DataLen);

/**
 * @brief Inverse define of OpmREye signal
 * @param [in] bInv : false:same / true:inverse
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_BldDBDIrInvSet(bool bInv);

/**
 * @brief Enable to detect OpmREye signal and switch base address of gain
 * @param [in] bEnable : Enable/Disbale DotByDot blending IR mode
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_BldDBDIrSet(bool bEnable);

#ifdef __cplusplus
}
#endif

#endif // DV_PROAV_BLEND_H
