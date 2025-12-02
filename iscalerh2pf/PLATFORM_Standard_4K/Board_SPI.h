#ifndef _BOARD_SPI_H_
#define _BOARD_SPI_H_

#define SPI_SEND_MAX_BUFFER (512)
#define SPI_SEND_MAX_MSG    (32)

#define SPI_MSG_SIZE    0xFF

#define SPI_OK      1
#define SPI_ERROR   0

#define BOARD_SSP_CS0 0
#define BOARD_SSP_CS1 1

typedef enum
{
    eBOARD_SSP_CS_C789 = BOARD_SSP_CS0,
    eBOARD_SSP_CS_C821 = BOARD_SSP_CS1,
    eBOARD_SSP_CS_C341 = BOARD_SSP_CS1,
    eBOARD_SSP_CS_F34  = BOARD_SSP_CS0,

    eBOARD_SSP_CS_NUMBERS = 2,
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

typedef struct
{
    UINT16  wSize;
    UINT8   acData[SPI_SEND_MAX_BUFFER];
} sSPI_DATA;

void Board_SPI_Init(void);
BOOL SPI_BufferIsFull(void);
uint8 SPI_SendWriteBuffer(void);
eRESULT SPI_WriteToBuffer(uint8 *txData, uint16 txLen);
#ifdef SCALER_FPGA_F34
uint8 SPI_SendWriteCmd(uint8 *txData, uint32 txLen);
uint8 SPI_SendReadCmd(uint8 *txData, uint8 *rxData, uint32 txLen, uint32 rxLen);
#else
uint8 SPI_SendReadCmd(eBOARD_SSP_CS eCS, uint8 *txData, uint8 *rxData, uint32 txLen, uint32 rxLen);
uint8 SPI_SendWriteCmd(eBOARD_SSP_CS eCS, uint8 *txData, uint32 txLen);
uint8 SPI_SendBurstWriteCmd(eBOARD_SSP_CS eCS, uint8 *txData, uint32 txLen, uint8 fifo);
#endif
BOOL Board_SSP_SemaphoreGive(eBOARD_SSP_CS eCS);
BOOL Board_SSP_SemaphoreTake(eBOARD_SSP_CS eCS);


#endif


