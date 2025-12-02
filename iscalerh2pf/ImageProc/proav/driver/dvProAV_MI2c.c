/*
 *  i2c.c
 *
 *  This file contains the i2c API routines.
 *
 *  All rights reserved. Property of Bitec Ltd, UK.
 *  Restricted rights to use, duplicate or disclose this code are
 *  granted through contract.
 *
 *  Copyright Bitec Ltd 2006
 *
 */

#include "dvProAV_MI2c.h"
#include "dvProAV_Access.h"
#include <time.h>

//#define IN_FILE_DEBUG
#ifndef IN_FILE_DEBUG
    #undef DBMSG
    #define DBMSG(...) do{}while(0)
#endif

// Overall status and control
#define I2C_CR_STA 0x80
#define I2C_CR_STO 0x40
#define I2C_CR_RD 0x20
#define I2C_CR_WR 0x10
#define I2C_CR_ACK 0x08
#define I2C_CR_IACK 0x01

#define I2C_SR_TIP 0x2

// Register index
#define I2C_REG_PRL_RW 0x10  // register for read and write
#define I2C_REG_PRH_RW 0x11  // register for read and write
#define I2C_REG_CTR_RW 0x12  // register for read and write

#define I2C_REG_RXR_R  0x13  // register for read
#define I2C_REG_STA_R  0x14  // register for read
#define I2C_REG_TXR_W  0x13  // register for write
#define I2C_REG_CMD_W  0x14  // register for write

#ifndef SCALER_LPC4337
    static uint32 ulStartTimeMsec[1];
#else
    static TickType_t ulStartTimeMsec[1];
#endif

static void _ResetTime(uint08 id)
{
#ifndef SCALER_LPC4337
    ulStartTimeMsec[id] = (uint32)xGetMSCount();
#else
    ulStartTimeMsec[id] = xTaskGetTickCount();
#endif
}

static uint32 _TimeElapsed(uint08 u8Id)
{
    uint32 ulTimeMsec;

#ifndef SCALER_LPC4337
    ulTimeMsec = xGetMSCount();
#else
    ulTimeMsec = xTaskGetTickCount();
#endif
    if(ulStartTimeMsec[u8Id] > ulTimeMsec)
    {
        return (0xFFFFFFFF - ulStartTimeMsec[u8Id] + 1 + ulTimeMsec);
    }
    else
    {
        return (ulTimeMsec - ulStartTimeMsec[u8Id]);
    }
}

static int _WaitTip(uint08 u8Port)
{
    int status = rcSUCCESS;
    uint08 data = 0xff; // 修改初值避免跳過while loop

    _ResetTime(0);
#ifndef NO_INTERFACE
    while((data & I2C_SR_TIP) > 0)
    {
        status &= dvProAV_AccessRawDataReadByte(u8Port, I2C_REG_STA_R, &data);
        if (status!= rcSUCCESS)
            break;
        if (_TimeElapsed(0) > PROAV_MASTER_I2C_RETRT_TIMEOUT)
            break;
    }
#endif /* NO_INTERFACE */

    return status;
}

int dvProAV_MI2CInit(uint08 u8Port, uint32 u32I2cFreq)
{
  // Setup prescaler for i2cFreqKHz with sysclk of REFERENCE_CLOCK
  int prescale = PROAV_MASTER_I2C_REFERENCE_CLOCK / (5 * u32I2cFreq);
  int status = rcSUCCESS;

  status &= dvProAV_AccessRawDataWriteByte(u8Port, I2C_REG_PRL_RW, prescale & 0xff);
  status &= dvProAV_AccessRawDataWriteByte(u8Port, I2C_REG_PRH_RW, (prescale & 0xff00)>>8);

  // Enable core
  status &= dvProAV_AccessRawDataWriteByte(u8Port, I2C_REG_CTR_RW,  0x80);
  return status;
}

int dvProAV_MI2CWrite(uint08 u8Port, uint08 u32Address, uint08 u8Reg, uint08 u8Data)
{
    int status = rcSUCCESS;

    status &= _WaitTip(u8Port);

    // write address

    status &= dvProAV_AccessRawDataWriteByte(u8Port, I2C_REG_TXR_W, u32Address);
    status &= dvProAV_AccessRawDataWriteByte(u8Port, I2C_REG_CMD_W, I2C_CR_STA | I2C_CR_WR | I2C_CR_ACK);
    status &= _WaitTip(u8Port);

    // write register address
    status &= dvProAV_AccessRawDataWriteByte(u8Port, I2C_REG_TXR_W, u8Reg);
    status &= dvProAV_AccessRawDataWriteByte(u8Port, I2C_REG_CMD_W, I2C_CR_WR | I2C_CR_ACK);
    status &= _WaitTip(u8Port);

    // write data
    status &= dvProAV_AccessRawDataWriteByte(u8Port, I2C_REG_TXR_W, u8Data);
    status &= dvProAV_AccessRawDataWriteByte(u8Port, I2C_REG_CMD_W, I2C_CR_WR | I2C_CR_STO | I2C_CR_IACK);
    status &= _WaitTip(u8Port);

    return status;
}


int dvProAV_MI2CRead(uint08 u8Port, uint08 u32Address, uint08 u8Reg, uint08 *u8Data)
{
    int status = rcSUCCESS;

    status &= _WaitTip(u8Port);

    // write address
    status &= dvProAV_AccessRawDataWriteByte(u8Port, I2C_REG_TXR_W, u32Address);
    status &= dvProAV_AccessRawDataWriteByte(u8Port, I2C_REG_CMD_W, I2C_CR_STA | I2C_CR_WR);
    status &= _WaitTip(u8Port);

    // write register address
    status &= dvProAV_AccessRawDataWriteByte(u8Port, I2C_REG_TXR_W, u8Reg);
    status &= dvProAV_AccessRawDataWriteByte(u8Port, I2C_REG_CMD_W, I2C_CR_WR);
    status &= _WaitTip(u8Port);

    // write address for reading
    status &= dvProAV_AccessRawDataWriteByte(u8Port, I2C_REG_TXR_W, u32Address | 1);
    status &= dvProAV_AccessRawDataWriteByte(u8Port, I2C_REG_CMD_W, I2C_CR_STA | I2C_CR_WR);
    status &= _WaitTip(u8Port);

    // read data
    status &= dvProAV_AccessRawDataWriteByte(u8Port, I2C_REG_CMD_W, I2C_CR_RD | I2C_CR_ACK | I2C_CR_STO);
    status &= _WaitTip(u8Port);
    status &= dvProAV_AccessRawDataReadByte(u8Port, I2C_REG_RXR_R, u8Data);

    return status;
}
