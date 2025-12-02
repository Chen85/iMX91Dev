#ifndef DV_PROAV_HDR_GAMMA_TABLE_H
#define DV_PROAV_HDR_GAMMA_TABLE_H

#include "dvProAV_Platform.h"

typedef enum{
    eHdrGamma1,
    eHdrGamma3,
    eHdrGamma4,
    eHdrGamma4_1 = eHdrGamma4,
    eHdrGamma4_2,
    eHdrGamma4_3,
    eHdrGamma4_4,
    eHdrGamma4_Auto,
    eHdrGammaTypeMax
}HDR_GAMMA_TYPE;

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief Get ProAV HDR Gamma table
 * @param [in] eHdrGammaType : HDR Gamma type
 * @return ProAV HDR Gamma Table Pointer
 */
const uint08 *dvProAV_HdrGammaTableGet(HDR_GAMMA_TYPE eHdrGammaType);


#ifdef __cplusplus
}
#endif

#endif // DV_PROAV_HDR_GAMMA_TABLE_H
