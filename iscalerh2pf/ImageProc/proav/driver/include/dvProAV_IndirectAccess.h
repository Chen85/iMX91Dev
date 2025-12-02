#ifndef DV_PROAV_INDIRECT_ACCESS_H
#define DV_PROAV_INDIRECT_ACCESS_H

#include "dvProAV_Platform.h"
#include "dvProAV_IndirectRegTable.h"

#ifdef __cplusplus
extern "C"
{
#endif

int dvProAV_IndirectRawDataWrite32bits(uint08 u8Bank, uint32 u32Addr, uint32 u32Data);
int dvProAV_IndirectRawDataRead32bits(uint08 u8Bank, uint32 u32Addr, uint32 *u32Data);
int dvProAV_IndirectRawDataWrite(uint08 u8Bank, uint32 u32Addr, uint16 len, uint32 *u32Data);
int dvProAV_IndirectRawDataRead(uint08 u8Bank, uint32 u32Addr, uint16 len, uint32 *u32Data);
int dvProAV_IndirectRegWriteData(uint16 u16RegEnum, uint32 u32Data);
int dvProAV_IndirectRegReadData(uint16 u16RegEnum, uint32 *u32Data);
int dvProAV_IndirectRegWrite(uint16 u16RegEnum, uint32 *u32Data);
int dvProAV_IndirectRegRead(uint16 u16RegEnum, uint32 *u32Data);
int dvProAV_IndirectAccessInit(void); // Should Call the dvProvAVInit() first

#ifdef __cplusplus
}
#endif

#endif // DV_PROAV_INDIRECT_ACCESS_H
