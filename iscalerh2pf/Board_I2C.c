// ==============================================================================
// FILE NAME: BOARD_I2C.C
// DESCRIPTION:
//
//
// modification history
// --------------------
// 03/01/2013, Leohong written
// --------------------
// ==============================================================================


/*
 * i2c.c
 *
 *  Created on: May 7, 2018
 *      Author: bennytseng
 */

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/fs.h>
#include <errno.h>
#include <string.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include "Board_I2C.h"
#include "utilDbgMsg.h"
#include "SystemCfgAccess.h"

//#include "i2c.h"

//==============================================================================
static int m_gI2cFd[eBOARD_I2C_NUMBERS] = {-1, -1, -1};
//==============================================================================


#define I2C_SLAVE_ADDR      0x66
#define I2C_RX_BUFFER_SIZE  2048 //A70LH_Larry_0002
#define I2C_TX_BUFFER_SIZE  2048

static INT32 m_iMCUEnableChecksum = 0;

sI2C_INFO sI2C_DevInfo[eI2CDEV_NUMBER] =
{
    {"System",              eBOARD_I2C_BUS2, 0x78, 10, I2C_DEFAULT|I2C_WORD_REG|I2C_REG_NEED  , I2C_DEFAULT|I2C_WORD_REG|I2C_REG_NEED},
    {"Frontend",            eBOARD_I2C_BUS0, 0x72, 10, I2C_DEFAULT|I2C_REG_NEED               , I2C_DEFAULT|I2C_REG_NEED},
    {"Motor",               eBOARD_I2C_BUS0, 0x74, 10, I2C_DEFAULT|I2C_WORD_REG|I2C_REG_NEED  , I2C_DEFAULT|I2C_WORD_REG|I2C_REG_NEED},
    {"DDP",                 eBOARD_I2C_BUS0, 0x34, 10, I2C_DEFAULT|I2C_REG_NEED               , I2C_DEFAULT},
    {"LDDrv",               eBOARD_I2C_BUS0, 0x74, 10, I2C_DEFAULT|I2C_WORD_REG|I2C_REG_NEED  , I2C_DEFAULT|I2C_WORD_REG|I2C_REG_NEED},
    {"FPFA0_VideoSwitch",   eBOARD_I2C_BUS0, 0x34, 10, I2C_DEFAULT|I2C_REG_NEED               , I2C_DEFAULT|I2C_REG_NEED},
    {"FPFA1_System",        eBOARD_I2C_BUS0, 0x32, 10, I2C_DEFAULT|I2C_REG_NEED               , I2C_DEFAULT|I2C_REG_NEED},
    {"FPFA2_LD",            eBOARD_I2C_BUS0, 0x36, 10, I2C_DEFAULT|I2C_REG_NEED               , I2C_DEFAULT|I2C_REG_NEED},
    {"XilinxFPGA",          eBOARD_I2C_BUS2, 0x90, 10, I2C_DEFAULT|I2C_REG_NEED               , I2C_DEFAULT|I2C_REG_NEED},
    {"VCXO_0",              eBOARD_I2C_BUS0, 0x00, 10, I2C_DEFAULT|I2C_REG_NEED               , I2C_DEFAULT|I2C_REG_NEED},
    {"VCXO_1",              eBOARD_I2C_BUS0, 0x00, 10, I2C_DEFAULT|I2C_REG_NEED               , I2C_DEFAULT|I2C_REG_NEED},
    {"VCXO_2",              eBOARD_I2C_BUS0, 0x00, 10, I2C_DEFAULT|I2C_REG_NEED               , I2C_DEFAULT|I2C_REG_NEED}
};

void I2C_Config(void)
{
    sI2C_DevInfo[eI2CDEV_SYSTEM].I2C_Bus                = Syscfg_Value_Get_Typeint(eSystem_I2C_BUS);
    sI2C_DevInfo[eI2CDEV_SYSTEM].I2C_Addr               = Syscfg_Value_Get_Typeint(eSystem_I2C_ADDR);
    sI2C_DevInfo[eI2CDEV_FRONTEND].I2C_Bus              = Syscfg_Value_Get_Typeint(eFrontend_I2C_BUS);
    sI2C_DevInfo[eI2CDEV_FRONTEND].I2C_Addr             = Syscfg_Value_Get_Typeint(eFrontend_I2C_ADDR);
    sI2C_DevInfo[eI2CDEV_MOTOR].I2C_Bus                 = Syscfg_Value_Get_Typeint(eMotor_I2C_BUS);
    sI2C_DevInfo[eI2CDEV_MOTOR].I2C_Addr                = Syscfg_Value_Get_Typeint(eMotor_I2C_ADDR);
    sI2C_DevInfo[eI2CDEV_DDP].I2C_Bus                   = Syscfg_Value_Get_Typeint(eDDP_I2C_BUS);
    sI2C_DevInfo[eI2CDEV_DDP].I2C_Addr                  = Syscfg_Value_Get_Typeint(eDDP_I2C_ADDR);
    sI2C_DevInfo[eI2CDEV_LDDRV].I2C_Bus                 = Syscfg_Value_Get_Typeint(eLDDRV_I2C_BUS);
    sI2C_DevInfo[eI2CDEV_LDDRV].I2C_Addr                = Syscfg_Value_Get_Typeint(eLDDRV_I2C_ADDR);
    sI2C_DevInfo[eI2CDEV_FPGA0_VIDEOSWITCH].I2C_Bus     = Syscfg_Value_Get_Typeint(eFPGA0_I2C_BUS);
    sI2C_DevInfo[eI2CDEV_FPGA0_VIDEOSWITCH].I2C_Addr    = Syscfg_Value_Get_Typeint(eFPGA0_I2C_ADDR);
    sI2C_DevInfo[eI2CDEV_FPGA1_SYSTEM].I2C_Bus          = Syscfg_Value_Get_Typeint(eFPGA1_I2C_BUS);
    sI2C_DevInfo[eI2CDEV_FPGA1_SYSTEM].I2C_Addr         = Syscfg_Value_Get_Typeint(eFPGA1_I2C_ADDR);
    sI2C_DevInfo[eI2CDEV_FPGA2_LD].I2C_Bus              = Syscfg_Value_Get_Typeint(eFPGA2_I2C_BUS);
    sI2C_DevInfo[eI2CDEV_FPGA2_LD].I2C_Addr             = Syscfg_Value_Get_Typeint(eFPGA2_I2C_ADDR);
    sI2C_DevInfo[eI2CDEV_XILINX_FPGA].I2C_Bus           = Syscfg_Value_Get_Typeint(eXilinx_FPGA_I2C_BUS);
    sI2C_DevInfo[eI2CDEV_XILINX_FPGA].I2C_Addr          = Syscfg_Value_Get_Typeint(eXilinx_FPGA_I2C_ADDR);
    sI2C_DevInfo[eI2CDEV_VCXO_0].I2C_Bus                = Syscfg_Value_Get_Typeint(eCDEC913_0_I2C_BUS);
    sI2C_DevInfo[eI2CDEV_VCXO_0].I2C_Addr               = Syscfg_Value_Get_Typeint(eCDEC913_0_I2C_ADDR);
    sI2C_DevInfo[eI2CDEV_VCXO_1].I2C_Bus                = Syscfg_Value_Get_Typeint(eCDEC913_1_I2C_BUS);
    sI2C_DevInfo[eI2CDEV_VCXO_1].I2C_Addr               = Syscfg_Value_Get_Typeint(eCDEC913_1_I2C_ADDR);
    sI2C_DevInfo[eI2CDEV_VCXO_2].I2C_Bus                = Syscfg_Value_Get_Typeint(eCDEC913_2_I2C_BUS);
    sI2C_DevInfo[eI2CDEV_VCXO_2].I2C_Addr               = Syscfg_Value_Get_Typeint(eCDEC913_2_I2C_ADDR);

    //dvMCUDriverI2CEnableChecksum(Syscfg_Value_Get_Typeint(eSystem_I2C_EnableChecksum));
    m_iMCUEnableChecksum = Syscfg_Value_Get_Typeint(eSystem_I2C_EnableChecksum);

    sI2C_DevInfo[eI2CDEV_MOTOR].WriteFlag = Syscfg_Value_Get_Typeint(eMotorDriverWR_WORD_REG) ? (I2C_DEFAULT|I2C_WORD_REG|I2C_REG_NEED) : (I2C_DEFAULT|I2C_REG_NEED);
    sI2C_DevInfo[eI2CDEV_MOTOR].ReadFlag  = Syscfg_Value_Get_Typeint(eMotorDriverWR_WORD_REG) ? (I2C_DEFAULT|I2C_WORD_REG|I2C_REG_NEED) : (I2C_DEFAULT|I2C_REG_NEED);

}

sI2C_INFO Board_I2C_DevInfoGet(eI2C_DEVICE eDev)
{
    return sI2C_DevInfo[eDev];
}

////////////Semaphore////////////
#if (SYSTEM_OS_TYPE == POSIX_COMPLIANT)

pthread_mutex_t axI2C_Mutex[eBOARD_I2C_NUMBERS] ;
#define I2C_SEMPHORE_WAIT_DELAY         (5000)
#define I2CMutexCreate(x)               pthread_mutex_init(&axI2C_Mutex[x], NULL)

uint8 I2CMutexTake(uint8 ucBus)
{
    struct timespec s_timeout;
    clock_gettime(CLOCK_REALTIME, &s_timeout);
    util_TimespecAddms(&s_timeout, I2C_SEMPHORE_WAIT_DELAY);

    if(pthread_mutex_timedlock(&axI2C_Mutex[ucBus] , &s_timeout) != 0)
    {
        LOG_MSG(db_ALWAYS, "011 %s pthread_mutex_timedlock fail [%s]\n", __FUNCTION__ , strerror(errno));
        return I2C_ERROR;
    }

    return I2C_OK;  //pass
}

uint8 I2CMutexGive(uint8 ucBus)
{
    if(pthread_mutex_unlock(&axI2C_Mutex[ucBus]) != 0)
    {
        LOG_MSG(db_ALWAYS, "%s pthread_mutex_unlock fail [%s]\n", __FUNCTION__ , strerror(errno));
        return I2C_ERROR;
    }

    return I2C_OK;  //pass
}


#else
SemaphoreHandle_t axI2CSemaphore[eBOARD_I2C_NUMBERS] = {NULL};

#define I2C_SEMPHORE_WAIT_DELAY (5000) //A70LV_Larry_0375

#define I2CMutexCreate(x)   axI2CSemaphore[x] = xSemaphoreCreateMutex();
#define I2CMutexTake(x)     xSemaphoreTake(axI2CSemaphore[x], I2C_SEMPHORE_WAIT_DELAY/portTICK_RATE_MS); //A70LV_Larry_0375
#define I2CMutexGive(x)     xSemaphoreGive(axI2CSemaphore[x]);
#endif
///////////Semaphore////////////

uint8 Board_I2C_Master_Write(uint8 cBus, uint8 addr, uint16 reg, uint16 len, void *data, uint8 ucFlag)
{
    struct i2c_rdwr_ioctl_data i2c_tfx;
    struct i2c_msg i2c_msg[2];
    uint8 *buf = NULL;

#ifdef NO_INTERFACE
    return I2C_OK;
#else

#if (SYSTEM_OS_TYPE == POSIX_COMPLIANT)
    if(I2CMutexTake(cBus) != I2C_OK)
    {
        return I2C_ERROR;
    }
#else

    I2CMutexTake(cBus);

#endif

    buf = (uint8 *)malloc(sizeof(uint8) * (len + 2));
    if(buf == NULL)
    {
        LOG_MSG(db_ALWAYS,"I2C:  I2C_Read malloc buf failed!\n");
        I2CMutexGive(cBus);
        return I2C_ERROR;
    }

    if(ucFlag & I2C_REG_NEED)
    {
        if(ucFlag & I2C_WORD_REG)
        {
            buf[0] = (reg >> 8) & 0xFF;
            buf[1] = reg & 0xFF;

            memcpy(&buf[2], (uint8 *)data, len);

            i2c_msg[0].addr = (addr >> 1);
            i2c_msg[0].flags = 0;
            i2c_msg[0].len = (len + 2);
            i2c_msg[0].buf = (uint8 *)buf;

            i2c_tfx.msgs = i2c_msg;
            i2c_tfx.nmsgs = 1;
        }
        else
        {
            buf[0] = reg & 0xFF;

            memcpy(&buf[1], (uint8 *)data, len);

            i2c_msg[0].addr = (addr >> 1);
            i2c_msg[0].flags = 0;
            i2c_msg[0].len = (len + 1);
            i2c_msg[0].buf = (uint8 *)buf;

            i2c_tfx.msgs = i2c_msg;
            i2c_tfx.nmsgs = 1;
        }
    }
    else
    {
        memcpy(&buf[0], (uint8 *)data, len);

        i2c_msg[0].addr = (addr >> 1);
        i2c_msg[0].flags = 0;
        i2c_msg[0].len = len;
        i2c_msg[0].buf = (uint8 *)buf;

        i2c_tfx.msgs = i2c_msg;
        i2c_tfx.nmsgs = 1;
    }

    if(ioctl(m_gI2cFd[cBus], I2C_RDWR, &i2c_tfx) < 0)
    {
        LOG_MSG(db_DV_I2C,"(%s, %d)I2C: can't send write i2c msg\n", __FUNCTION__, __LINE__);
        LOG_MSG(db_DV_I2C,"Bus 0x%02x, Add 0x%02x, Reg 0x%04x\n", cBus, addr, reg);
        free(buf);
        I2CMutexGive(cBus);
        return I2C_ERROR;
    }

    free(buf);

#if (SYSTEM_OS_TYPE == POSIX_COMPLIANT)
    if(I2CMutexGive(cBus) != I2C_OK)
    {
        //return I2C_ERROR;
    }
#else
    I2CMutexGive(cBus);
#endif

#endif

    return I2C_OK;

}

//UINT8 Board_I2C_Master_Read(UINT8 ucBus, UINT8 ucDeviceAddress, UINT16 uiRegister, UINT16 uiDataSz, UINT8 *pucData, UINT8 ucFlag)
uint8 Board_I2C_Master_Read(uint8 cBus, uint8 addr, uint16 reg, uint16 len, void *data, uint8 ucFlag)
{
    struct i2c_rdwr_ioctl_data i2c_rfx;
    struct i2c_msg i2c_msg[2];
    uint8 *buf = NULL;
    uint8 bufreg[2] = {0};
    int result = 0;

#ifdef NO_INTERFACE
    return I2C_OK;
#else

#if (SYSTEM_OS_TYPE == POSIX_COMPLIANT)
    if(I2CMutexTake(cBus) != I2C_OK)
    {
        return I2C_ERROR;
    }
#else

    I2CMutexTake(cBus);

#endif

    buf = (uint8 *)malloc(sizeof(uint8) * len);

    if(buf == NULL)
    {
        LOG_MSG(db_ALWAYS,"I2C:  I2C_Read malloc buf failed!\n");
        I2CMutexGive(cBus);
        return I2C_ERROR;
    }

    memset(buf, 0x0, len);

    if(ucFlag & I2C_REG_NEED)
    {
        if(ucFlag & I2C_WORD_REG)
        {
            bufreg[0] = (reg >> 8) & 0xFF;
            bufreg[1] = reg & 0xFF;

            i2c_msg[0].addr = (addr >> 1);
            i2c_msg[0].flags = 0;
            i2c_msg[0].len = 2;
            i2c_msg[0].buf = (uint8 *)bufreg;

        }
        else
        {
            bufreg[0] = (reg & 0xff);

            i2c_msg[0].addr = (addr >> 1);
            i2c_msg[0].flags = 0;
            i2c_msg[0].len = 1;
            i2c_msg[0].buf = (uint8 *)bufreg;
        }

        i2c_msg[1].addr = (addr >> 1);
        i2c_msg[1].flags = I2C_M_RD;
        i2c_msg[1].len = len;
        i2c_msg[1].buf = (uint8 *)buf;

        i2c_rfx.msgs = i2c_msg;
        i2c_rfx.nmsgs = 2;
    }
    else
    {
        i2c_msg[0].addr = (addr >> 1);
        i2c_msg[0].flags = 0;
        i2c_msg[0].len = 0;
        i2c_msg[0].buf = (uint8 *)bufreg;

        i2c_msg[1].addr = (addr >> 1);
        i2c_msg[1].flags = I2C_M_RD;
        i2c_msg[1].len = len;
        i2c_msg[1].buf = (uint8 *)buf;

        i2c_rfx.msgs = i2c_msg;
        i2c_rfx.nmsgs = 2;
    }

    result = ioctl(m_gI2cFd[cBus], I2C_RDWR, &i2c_rfx);

    if(result < 0)
    {
        LOG_MSG(db_DV_I2C,"(%s, %d)I2C: can't send read i2c msg [%s] \n" , __FUNCTION__, __LINE__, strerror(result));
        LOG_MSG(db_DV_I2C,"Bus 0x%02x, Add 0x%02x, Reg 0x%04x\n", cBus, addr, reg);
        free(buf);
        I2CMutexGive(cBus);
        return I2C_ERROR;
    }

    memcpy(data, buf, len);
    free(buf);

#if (SYSTEM_OS_TYPE == POSIX_COMPLIANT)
    if(I2CMutexGive(cBus) != I2C_OK)
    {
        //return I2C_ERROR;
    }
#else
    I2CMutexGive(cBus);
#endif

#endif

    return I2C_OK;

}


//==============================================================================
//uint8 I2C_OpenDev(uint8 *dev)
uint8 I2C_OpenDev(eBOARD_I2C eBus)
{
    if(eBus < eBOARD_I2C_NUMBERS)
    {
        I2CMutexCreate(eBus);

        switch(eBus)
        {
            case eBOARD_I2C_BUS0:
               {
                    m_gI2cFd[eBus] = open(I2C_DEV_1, O_RDWR);

                    if(m_gI2cFd[eBus] < 0)
                    {
                        LOG_MSG(db_ALWAYS,"I2C 0: can't OpenDev\n");
                        return I2C_ERROR;
                    }

                    ioctl(m_gI2cFd[eBus], I2C_TIMEOUT, 1);
                    ioctl(m_gI2cFd[eBus], I2C_RETRIES, 10);
                }
                break;

            case eBOARD_I2C_BUS1:
                {
                    m_gI2cFd[eBus] = open(I2C_DEV_2, O_RDWR);

                    if(m_gI2cFd[eBus] < 0)
                    {
                        LOG_MSG(db_ALWAYS,"I2C 1: can't OpenDev\n");
                        return I2C_ERROR;
                    }

                    ioctl(m_gI2cFd[eBus], I2C_TIMEOUT, 1);
                    ioctl(m_gI2cFd[eBus], I2C_RETRIES, 10);
                }
                break;

            case eBOARD_I2C_BUS2:
                {
                    m_gI2cFd[eBus] = open(I2C_DEV_3, O_RDWR);

                    if(m_gI2cFd[eBus] < 0)
                    {
                        LOG_MSG(db_ALWAYS,"I2C 2: can't OpenDev\n");
                        return I2C_ERROR;
                    }

                    ioctl(m_gI2cFd[eBus], I2C_TIMEOUT, 1);
                    ioctl(m_gI2cFd[eBus], I2C_RETRIES, 10);
                }
                break;

            default:
                break;
        }
    }
    else
    {
        LOG_MSG(db_ALWAYS,"I2C_OpenDev over flow\n");
    }



    return I2C_OK;
}

// ==============================================================================
// FUNCTION NAME: BOARD_I2C_INIT
// DESCRIPTION:
//
//
// Params:
// sBOARD_I2C_CFG *psI2CCgf:
//
// Returns:
//
//
// modification history
// --------------------
// 2014/05/03, Leo Create
// --------------------
// ==============================================================================
void Board_I2C_Init(void)
{
#ifdef NO_INTERFACE
    return;
#else
    I2C_OpenDev(eBOARD_I2C_BUS0);
    I2C_OpenDev(eBOARD_I2C_BUS2);
#endif
}

// ==============================================================================
// FUNCTION NAME: Board_I2C_MCUEnable
// DESCRIPTION:
//
//
// Params:
// void:
//
// Returns:
//
//
// Modification History
// --------------------
// 2024/07/10, Larry Create
// --------------------
// ==============================================================================
BOOL Board_I2C_MCUEnable(void)
{
    if(m_iMCUEnableChecksum == -1)
    {
        return FALSE;
    }
    else
    {
        if(m_iMCUEnableChecksum)
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
}

