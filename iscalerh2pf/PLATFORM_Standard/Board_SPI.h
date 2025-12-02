#ifndef _BOARD_SPI_H_
#define _BOARD_SPI_H_

#include "utilCommon.h"
#include <linux/spi/spidev.h>

//#define SPI_ASYNC
#define SPI_GPIOCS

#define SPI_OK      1
#define SPI_ERROR   0

#define SPI_MSG_SIZE    0xFF

#define ECSPI3_BASE_ADDR        0x02010000
#define ECSPI_CSPIRXDATA        0x00
#define ECSPI_CSPITXDATA        0x04
#define ECSPI_CSPICTRL          0x08
#define ECSPI_CSPICFG           0x0c
#define ECSPI_RESET             0x1c

#define GPIO4_BASE_ADDR         0x020A8000
#define GPIO_SIZE               32

typedef enum
{
    eBOARD_SSP_CS_C789,
    eBOARD_SSP_CS_C821,

    eBOARD_SSP_CS_NUMBERS
} eBOARD_SSP_CS;

typedef struct
{
    uint32 DevFd;
    uint32 Speed;
    uint8 Mode;
    uint8 Bits;
    uint16 Delay;
    uint32 CS;
    uint32 GpioCS;
//    struct spi_ioc_transfer message[SPI_MSG_SIZE];
    UINT8 *paucTxBuf[SPI_MSG_SIZE];
    UINT8 ucMsgIdx;
} sBOARD_SPI_CFG;

typedef struct
{
    char   cReg[32];
    UINT32 ulAddress;
}sICHIP_DUMP;

void Board_SPI_Init(void);
uint8 SPI_SendReadCmd(eBOARD_SSP_CS eCS, uint8 *txData, uint8 *rxData, uint32 txLen, uint32 rxLen);
uint8 SPI_SendWriteCmd(eBOARD_SSP_CS eCS, uint8 *txData, uint32 txLen, uint8 fifo);
uint8 SPI_SendBurstWriteCmd(eBOARD_SSP_CS eCS, uint8 *txData, uint32 txLen, uint8 fifo);
BOOL Board_SSP_SemaphoreGive(eBOARD_SSP_CS eCS);
BOOL Board_SSP_SemaphoreTake(eBOARD_SSP_CS eCS);
void SPI_Test(void);



#endif


