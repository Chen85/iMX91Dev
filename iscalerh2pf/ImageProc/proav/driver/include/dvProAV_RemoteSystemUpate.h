#ifndef DV_PROAV_RSU
#define DV_PROAV_RSU

#include "dvProAV_Platform.h"

#ifdef __cplusplus
extern "C"
{
#endif

int dvProAV_RsuInit(void);
int dvProAV_RsuFlashEraseAll(void);
int dvProAV_RsuFlashEraseSector(uint32 address, uint32 length);
int dvProAV_RsuFlashAccess(uint32 flashAddress, uint08 *data, uint32 length, bool write);
int dvProAV_RsuReadImageStatus(uint32 *errStatus);
int dvProAV_RsuReConfig( uint32 customAddr, uint32 watchdogTime);
int dvProAV_RsuFlashReadDlySet(uint32 ulDly);
int dvProAV_RsuFlashClkRateSet(uint32 ulSpiClockRate);

#ifdef __cplusplus
}
#endif

#endif //DV_PROAV_RSU
