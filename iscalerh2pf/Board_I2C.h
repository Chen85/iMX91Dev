#ifndef BOARD_I2C_H
#define BOARD_I2C_H
// ==============================================================================
// FILE NAME: BOARD_I2C.H
// DESCRIPTION:
//
//
// modification history
// --------------------
// 03/01/2013, Leohong written
// --------------------
// ==============================================================================

#include "Common.h"

/*
 * i2c.h
 *
 *  Created on: May 7, 2018
 *      Author: bennytseng
 */

#ifndef I2C_H_
#define I2C_H_
//==============================================================================
#define I2C_DEV_1     "/dev/i2c-0"
#define I2C_DEV_2     "/dev/i2c-1"
#define I2C_DEV_3     "/dev/i2c-2"
//==============================================================================

#define I2C_DEFAULT     (0x01<<0)
#define I2C_REG_NEED    (0x01<<1)
#define I2C_WORD_REG    (0x01<<2)
#define I2C_IGNORE_ACK  (0x01<<3)
#define I2C_ADD_TRAN    (0x01<<4)

#define I2C_OK 1
#define I2C_ERROR 0

typedef enum
{
    eBOARD_I2C_BUS0,
    eBOARD_I2C_BUS1,
    eBOARD_I2C_BUS2,

    eBOARD_I2C_NUMBERS,
} eBOARD_I2C;


typedef enum
{
    eI2CDEV_SYSTEM,   //standby MCU
    eI2CDEV_FRONTEND,
    eI2CDEV_MOTOR,
    eI2CDEV_DDP,
    eI2CDEV_LDDRV,
    eI2CDEV_FPGA0_VIDEOSWITCH,
    eI2CDEV_FPGA1_SYSTEM,
    eI2CDEV_FPGA2_LD,
    eI2CDEV_XILINX_FPGA,
    eI2CDEV_VCXO_0,
    eI2CDEV_VCXO_1,
    eI2CDEV_VCXO_2,

    eI2CDEV_NUMBER,
}eI2C_DEVICE;


typedef struct
{
    char DeviceName[24];
    int I2C_Bus;
    int I2C_Addr;
    int Retry;
    int WriteFlag;
    int ReadFlag;
}sI2C_INFO;


//==============================================================================
uint8 Board_I2C_Master_Write(uint8 cBus, uint8 addr, uint16 reg, uint16 len, void *data, uint8 ucFlag);
uint8 Board_I2C_Master_Read(uint8 cBus, uint8 addr, uint16 reg, uint16 len, void *data, uint8 ucFlag);
void Board_I2C_Init(void);
void I2C_Config(void);
sI2C_INFO Board_I2C_DevInfoGet(eI2C_DEVICE eDev);
BOOL Board_I2C_MCUEnable(void);
//==============================================================================
#endif /* I2C_H_ */


#endif // BOARD_I2C_H


