#ifndef DV_PROAV_UNIFORMITY_H
#define DV_PROAV_UNIFORMITY_H

#include "dvProAV_Platform.h"
#include "dvProAV_Access.h"
#include "dvProAV_RegTable.h"
#include "dvProAV_Scaler.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief Set Uniformity Enable/Disbale
 * @param [in] bEnable
 *             -true: ColorUniformity enable
 *             -false: ColorUniformity disable
 * @return status : rcSUCCESS / rcERROR / rcINVALID / rcINVALID
 */
int dvProAV_UnifEnableSet(bool bEnable);

/**
 * @brief Get Uniformity Enable / Disbale
 * @return Uniformity Enable / Disbale
 *             -true: ColorUniformity enable
 *             -false: ColorUniformity disable
 */
bool dvProAV_UnifEnableGet(void);

/**
 * @brief Get Uniformity Enable/Disbale
 * @return Uniformity Enable/Disbale
 *             -true: ColorUniformity enable
 *             -false: ColorUniformity disable
 */
bool dvProAV_UnifEnableGet(void);

/**
 * @brief Set Uniformity Data
 * @param [in] u16Offset : 資料起始點
 * @param [in] u8HCount : 水平區塊數
 * @param [in] u8VCount : 垂直區塊數
 * @param [in] u8Data : 寫入的資料
 * @param [in] u16Len : 寫入的長度
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_UnifDataSet(uint16 u16Offset, uint08 u8HCount, uint08 u8VCount, uint08 *u8Data, uint16 u16Len);

/**
 * @brief Set LSB shift of Uniformity Data
 * @param [in] u8ShiftBits : bits of LSB shift
 *             -0: no shift    , (1<<0)
 *             -1: 2 bits shift, (1<<1)
 *             -2: 4 bits shift, (1<<2)
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_UnifDataShiftSet(uint08 u8ShiftBits);

#ifdef __cplusplus
}
#endif

#endif // DV_PROAV_UNIFORMITY_H
