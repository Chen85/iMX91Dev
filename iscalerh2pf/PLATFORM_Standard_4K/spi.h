/*
 * spi.h
 *
 *  Created on: Apr 12, 2018
 *      Author: bennytseng
 */

#ifndef SPI_H_
#define SPI_H_
//==============================================================================================

#include "utilCommon.h"


typedef struct
{
    uint32 DevFd;
    uint32 Speed;
    uint8 Mode;
    uint8 Bits;
    uint16 Delay;
    uint32 CS;
}_SpiConfig;
//==============================================================================================
#endif /* SPI_H_ */
