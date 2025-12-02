#ifndef DV_PROAV_WARP_TABLE_H
#define DV_PROAV_WARP_TABLE_H

#include "dvProAV_Platform.h"
#include "dvProAV_Warp.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief Get ProAV Warp table pointer
 * @param [in] eIndex :
 * @return ProAV Warp Table Pointer
 */
uint8 *dvProAV_WarpTableGet(WarpTableTypeIndex eIndex);

/**
 * @brief Get block size of ProAV Warp table
 * @param [in] eTableSize :
 * @return block size of ProAV Warp table
 */
uint32 dvProAV_WarpTableBlockSizeGet(WarpTableSize eTableSize);

#ifdef __cplusplus
}
#endif

#endif // DV_PROAV_WARP_TABLE_H
