/*
 *  dvProAV_MI2C.h
 *
 *  This file contains the ProAV master i2c API routines.
 *
 */

#ifndef DV_PROAV_MI2C_H
#define DV_PROAV_MI2C_H

#include "dvProAV_Platform.h"

#ifdef __cplusplus
extern "C" {
#endif
#define PROAV_MASTER_I2C_RETRT_TIMEOUT 1000 // 1000 msec

#define PROAV_MASTER_I2C_REFERENCE_CLOCK 50000000 // 50MHz

int dvProAV_MI2CInit(uint08 u8Port, uint32 u32I2cFreq);
int dvProAV_MI2CWrite(uint08 u8Port, uint08 u32Address, uint08 u8Reg, uint08 u8Data);
int dvProAV_MI2CRead(uint08 u8Port, uint08 u32Address, uint08 u8Reg, uint08 *u8Data);

#ifdef __cplusplus
}
#endif


#endif // DV_PROAV_MI2C_H_
