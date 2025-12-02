#ifndef _HALRSU_H_
#define _HALRSU_H_

#include "Common.h"

#define FACTORY_CODE_START_ADDRESS  0x00000020
#define FILE_START_ADDRESS          0x04000000
#define APP_CODE_BASED_ADDRESS      0x04000000
#define APP_CODE_START_ADDRESS      0x04000020

//#define FPGA_FW          	"/mnt/configs/scaler/FWPackge/FPGA_2656_150800a0_T50_1150_1.rpd"
//#define FILE_START_ADDRESS          0x00000000

eRESULT halRsu_Init(void);
UINT8 halRsu_Upgrade_Partial(char *filename, const UINT32 ulFlash_Addr, UINT8 bFroce);
UINT32 halRsu_FPGAVersionGet(void);

#endif /*_HALRSU_H_*/
