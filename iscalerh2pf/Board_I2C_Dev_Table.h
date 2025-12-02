#ifndef BOARD_I2C_DEV_TABLE_H
#define BOARD_I2C_DEV_TABLE_H
// ===============================================================================
// FILE NAME: Board_I2C_Dev_Table.h
// DESCRIPTION:
//
//
// Modification History
// --------------------
// 2014/07/14, Leo Create
// --------------------
// ===============================================================================

#include "Board_I2C.h"
#include "utilCommon.h"
#include "dvInterfaceDiag.h"

#define LPC_54605_I2C_BUS               Board_I2C_DevInfoGet(eI2CDEV_SYSTEM).I2C_Bus
#define LPC_54605_I2C_ADDRESS           Board_I2C_DevInfoGet(eI2CDEV_SYSTEM).I2C_Addr
#define LPC_54605_I2C_RETRY             Board_I2C_DevInfoGet(eI2CDEV_SYSTEM).Retry
#define LPC_54605_I2C_FLAG              Board_I2C_DevInfoGet(eI2CDEV_SYSTEM).WriteFlag

#define LDDRV_I2C_BUS                   Board_I2C_DevInfoGet(eI2CDEV_LDDRV).I2C_Bus
#define LDDRV_I2C_ADDRESS               Board_I2C_DevInfoGet(eI2CDEV_LDDRV).I2C_Addr
#define LDDRV_I2C_RETRY                 Board_I2C_DevInfoGet(eI2CDEV_LDDRV).Retry
#define LDDRV_I2C_FLAG                  Board_I2C_DevInfoGet(eI2CDEV_LDDRV).WriteFlag

#define FRONT_END_I2C_BUS               Board_I2C_DevInfoGet(eI2CDEV_FRONTEND).I2C_Bus
#define FRONT_END_I2C_ADDRESS           Board_I2C_DevInfoGet(eI2CDEV_FRONTEND).I2C_Addr
#define FRONT_END_I2C_RETRY             Board_I2C_DevInfoGet(eI2CDEV_FRONTEND).Retry
#define FRONT_END_I2C_FLAG              Board_I2C_DevInfoGet(eI2CDEV_FRONTEND).WriteFlag
#define FRONT_END_I2C_FLAG_2ByteReg     ((Board_I2C_DevInfoGet(eI2CDEV_FRONTEND).WriteFlag)|I2C_WORD_REG)

#define MOTOR_I2C_BUS                   Board_I2C_DevInfoGet(eI2CDEV_MOTOR).I2C_Bus
#define MOTOR_I2C_ADDRESS               Board_I2C_DevInfoGet(eI2CDEV_MOTOR).I2C_Addr
#define MOTOR_I2C_RETRY                 Board_I2C_DevInfoGet(eI2CDEV_MOTOR).Retry
#define MOTOR_I2C_FLAG                  Board_I2C_DevInfoGet(eI2CDEV_MOTOR).WriteFlag

#define I2C_DDP_BUS                     Board_I2C_DevInfoGet(eI2CDEV_DDP).I2C_Bus
#define I2C_DDP_ADDRESS                 Board_I2C_DevInfoGet(eI2CDEV_DDP).I2C_Addr
#define I2C_DDP_RETRY                   Board_I2C_DevInfoGet(eI2CDEV_DDP).Retry
#define I2C_DDP_WRITE_FLAG              Board_I2C_DevInfoGet(eI2CDEV_DDP).WriteFlag
#define I2C_DDP_READ_FLAG               Board_I2C_DevInfoGet(eI2CDEV_DDP).ReadFlag

#define I2C_FPGA10M04_BUS_0             Board_I2C_DevInfoGet(eI2CDEV_FPGA0_VIDEOSWITCH).I2C_Bus
#define I2C_FPGA10M04_BUS_1             Board_I2C_DevInfoGet(eI2CDEV_FPGA1_SYSTEM).I2C_Bus
#define I2C_FPGA10M04_BUS_2             Board_I2C_DevInfoGet(eI2CDEV_FPGA2_LD).I2C_Bus
#define I2C_FPGA10M04_ADDR_0            Board_I2C_DevInfoGet(eI2CDEV_FPGA0_VIDEOSWITCH).I2C_Addr //video
#define I2C_FPGA10M04_ADDR_1            Board_I2C_DevInfoGet(eI2CDEV_FPGA1_SYSTEM).I2C_Addr //power seq
#define I2C_FPGA10M04_ADDR_2            Board_I2C_DevInfoGet(eI2CDEV_FPGA2_LD).I2C_Addr //LD
#define I2C_FPGA10M04_RETRY             Board_I2C_DevInfoGet(eI2CDEV_FPGA0_VIDEOSWITCH).Retry
#define I2C_FPGA10M04_FLAG              Board_I2C_DevInfoGet(eI2CDEV_FPGA0_VIDEOSWITCH).WriteFlag

#define XILLINX_FPGA_I2C_BUS            Board_I2C_DevInfoGet(eI2CDEV_XILINX_FPGA).I2C_Bus
#define XILLINX_FPGA_I2C_ADDRESS        Board_I2C_DevInfoGet(eI2CDEV_XILINX_FPGA).I2C_Addr
#define XILLINX_FPGA_I2C_RETRY          Board_I2C_DevInfoGet(eI2CDEV_XILINX_FPGA).Retry
#define XILLINX_FPGA_I2C_FLAG           Board_I2C_DevInfoGet(eI2CDEV_XILINX_FPGA).WriteFlag

#define VCXO_0_I2C_BUS                  Board_I2C_DevInfoGet(eI2CDEV_VCXO_0).I2C_Bus
#define VCXO_0_I2C_ADDR                 Board_I2C_DevInfoGet(eI2CDEV_VCXO_0).I2C_Addr
#define VCXO_1_I2C_BUS                  Board_I2C_DevInfoGet(eI2CDEV_VCXO_1).I2C_Bus
#define VCXO_1_I2C_ADDR                 Board_I2C_DevInfoGet(eI2CDEV_VCXO_1).I2C_Addr
#define VCXO_2_I2C_BUS                  Board_I2C_DevInfoGet(eI2CDEV_VCXO_2).I2C_Bus
#define VCXO_2_I2C_ADDR                 Board_I2C_DevInfoGet(eI2CDEV_VCXO_2).I2C_Addr
#define VCXO_I2C_RETRY                  Board_I2C_DevInfoGet(eI2CDEV_VCXO_0).Retry
#define VCXO_I2C_FLAG                   Board_I2C_DevInfoGet(eI2CDEV_VCXO_0).WriteFlag

#if 0
#define PROAV_I2C_BUSID         eBOARD_I2C_BUS0
#define PROAV_I2C_ADDRESS       0xAA
#define PROAV_I2C_RETRY         10
#define PROAV_I2C_FLAGS         I2C_DEFAULT | I2C_REG_NEED
#endif


#endif /* BOARD_I2C_DEV_TABLE_H */

