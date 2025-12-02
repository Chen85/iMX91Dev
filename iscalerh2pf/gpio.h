/*
 * gpio.h
 *
 *  Created on: Apr 16, 2018
 *      Author: bennytseng
 */

#ifndef GPIO_H_
#define GPIO_H_

#include "utilCommon.h"
#include "IMX6_GPIO_Table.h"

//==============================================================================================
#define GPIO_IN     0
#define GPIO_OUT    1
#define GPIO_HIGH   1
#define GPIO_LOW    0
//==============================================================================================
typedef enum
{
    GPIO_OK = 0,
    GPIO_ERROR
}_GpioErr;
//==============================================================================================
uint32 GPIO_Read(uint32 pin, uint8 *value);
_GpioErr GPIO_Write(uint32 pin, uint8 value);
_GpioErr GPIO_Open(uint32 pin, uint32 dir);
//==============================================================================================
#endif /* GPIO_H_ */
