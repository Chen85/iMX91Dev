#ifndef DV_PROAV_SCLGAMMA_TABLE_H
#define DV_PROAV_SCLGAMMA_TABLE_H

#include "dvProAV_Platform.h"

typedef enum {
    eGamma_1p0,
    eGamma_1p8,
    eGammaMax
}eGammaTable;


#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief Load ProAV Scaler Gamma table
 * @param [in] u8ColorId: Scaler Color index
 * @param [out] data : ProAV Scaler Gamma Table Pointer
 */
void dvProAV_SclGammaTableSet(uint08 u8ColorId, uint16 *u16Data);

/**
 * @brief Caculate ProAV Scaler Gamma table by power
 * @param [in] dPower : gamma power
 * @param [in] u9ColorId : color, 0: R, 1:G, B:2
 * @return ProAV Scaler Table Pointer
 */
uint16 *dvProAV_SclGammaTableCaculate(double dPower, uint08 u9ColorId);

/**
 * @brief Caculate Dual Pipe ProAV Scaler Gamma table by power
 * @param [in] dMainPower : main gamma power
 * @param [in] dSubPower : sub gamma power
 * @param [in] u8ColorId : color, 0: R, 1:G, B:2
 * @return ProAV Scaler Table Pointer
 */
uint16 *dvProAV_SclDualGammaTableCaculate(double dMainPower, double dSubPower, uint08 u8ColorId);

/**
 * @brief Caculate ProAV Blend Gamma table by power
 * @param [in] ucInputBits: input gamma step bits
 * @param [in] ucOutputBits: output gamma step bits
 * @param [in] ucScaleBits: one step scale bits
 * @param [in] dPower: gamma power
 * @param [in] u8ColorId
 * @return ProAV Scaler Table Pointer
 */
uint16 *dvProAV_BldGammaTableCaculate(uint08 ucInputBits, uint08 ucOutputBits, uint08 ucScaleBits, double dPower, uint08 u8ColorId);

/**
 * @brief Get ProAV Scaler Gamma table
 * @param [in] colorId
 * @return ProAV Scaler Gamma Table Pointer
 */
uint16 *dvProAV_SclGammaTableGet(uint08 u8ColorId);


/**
 * @brief Get the Scaler Gamma Table Item Count
 * @return Byte Count of ProAV Scaler Gamma table
 */
uint16 dvProAV_SclGammaItemCountGet(void);

#ifdef __cplusplus
}
#endif

#endif // DV_PROAV_SCLGAMMA_TABLE_H
