#ifndef DV_PROAV_SCLCOEFF_TABLE_H_
#define DV_PROAV_SCLCOEFF_TABLE_H_

#include "dvProAV_Platform.h"


#define SclTapMax   8
#define SclPhMax    64

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief Get ProAV Scaling table pointer
 * @return ProAV Scaling Table Pointer
 */
const uint16 *dvProAV_SclCoeffTableGet(void);

/**
 * @brief Get block size of ProAV scaling table
 * @return block size of ProAV scaling table
 */
uint16 dvProAV_SclCoeffTableBlockSizeGet(void);

#ifdef __cplusplus
}
#endif

#endif // DV_PROAV_SCLCOEFF_TABLE_H
