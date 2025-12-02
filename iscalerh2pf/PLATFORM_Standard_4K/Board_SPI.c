#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <string.h>
#include <linux/stat.h>

#include <errno.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include "spi.h"
#include "gpio.h"
#include "Board_SPI.h"
#include "dvInterfaceDiag.h"
#include "utilDbgMsg.h"



//==============================================================================================
//#define SPI_DEV     "/dev/spidev2.0"
#define SPI_SEMPHORE_WAIT_DELAY (40000) //A70LV_Larry_0052   //H2PF_Simon_0038
#define SPI_CS_GPIO GPIO4_IO24_DISP0_DAT3
//#define SPI_DEV  "/dev/spidev0.0"
//#define SPI_CS_GPIO (105)

#define SPI_DEV_C789        "/dev/spidev2.0"
#define SPI_DEV_C821        "/dev/spidev0.0"


#ifdef SCALER_FPGA_F34
#define SPI_DEV             "/dev/spidev2.0"
//spi 傳送低於64byte，可以直接傳送
//若高於64 byte，需要是128 byte的倍數
#define SPI_GPIOCS
#define SPI_READ_SPEED      (30 * 1000 * 1000) // 30M
#define SPI_WRITE_SPEED     (30 * 1000 * 1000) // 30M
#define SPI_SPEED           (10 * 1000 * 1000) // 10M
#else
#define SPI_GPIOCS
#define SPI_DEV             "/dev/spidev0.0"
#define SPI_SPEED           (10 * 1000 * 1000) // 10M
#define SPI_BURST_SPEED     (20 * 1000 * 1000) // 20M
#endif


#define SPI_BITS    (0)

#define SPI_WRITE   1
#define SPI_READ    0
#define SPI_RXE_MODE_PIN    GPIO2_IO01_NANDF_D1 //G100_Owen_0080
#ifdef SCALER_FPGA_F34
#define SPI_RXE_ENABLE      FALSE               //G100_Owen_0080 : work with Y00.31 up
#else
#define SPI_RXE_ENABLE      TRUE                //G100_Owen_0080 : work with Y00.31 up
#endif /* SCALER_FPGA_F34 */

#define SPI_DATA_SIZE sizeof(sSPI_DATA)/sizeof(uint8)

static struct spi_ioc_transfer m_txbuffer[eBOARD_SSP_CS_NUMBERS];
static struct spi_ioc_transfer m_rxbuffer[eBOARD_SSP_CS_NUMBERS];

sPROCESS_MUTEX_DATA         *psProcessMutex;
pthread_mutex_t             axSPI_Mutex[eBOARD_SSP_CS_NUMBERS];
pthread_mutexattr_t         axSPI_MutexAttr[eBOARD_SSP_CS_NUMBERS];

uint8 SpiMutexCreate(uint8 ucBus)
{
    psProcessMutex = (sPROCESS_MUTEX_DATA *)SharedMem_GetMapPtr(eSB_PROCESS_MUTEX_DATA);

    pthread_mutexattr_init(&axSPI_MutexAttr[ucBus]);
    pthread_mutexattr_setpshared(&axSPI_MutexAttr[ucBus], PTHREAD_PROCESS_SHARED);  //Mutex for Process
    pthread_mutex_init(&axSPI_Mutex[ucBus], &axSPI_MutexAttr[ucBus]);

#ifdef SHM_FLOCK_PROTECT
    flock(SharedMem_GetFd(eSB_PROCESS_MUTEX_DATA), LOCK_EX);
#endif

    if(ucBus == eBOARD_SSP_CS_F34 || ucBus == eBOARD_SSP_CS_C341)
    {
        psProcessMutex->Scaler_Mutex       = axSPI_Mutex[eBOARD_SSP_CS_F34];
        psProcessMutex->Scaler_CurrentBank = 0xF0;   //H2PF_Simon_0093
    }
    else
    {
        LOG_MSG(db_ASSERT, "%s error bus %d\r\n", __FUNCTION__ , ucBus);
    }

#ifdef SHM_FLOCK_PROTECT
    msync((char *)SharedMem_GetMapPtr(eSB_PROCESS_MUTEX_DATA), SharedMem_GetSize(eSB_PROCESS_MUTEX_DATA), MS_SYNC);
    flock(SharedMem_GetFd(eSB_PROCESS_MUTEX_DATA), LOCK_UN);
#endif

}

pthread_mutex_t* SpiMutexGet(uint8 ucBus)
{
    switch(ucBus)
    {
#ifdef SCALER_C821_C789

        case eBOARD_SSP_CS_C821:
            return &psProcessMutex->Scaler_Mutex;

        case eBOARD_SSP_CS_C789:
            return &psProcessMutex->Warping_Mutex;

#elif defined(SCALER_C341)

        case eBOARD_SSP_CS_C341:
            return &psProcessMutex->Scaler_Mutex;

#elif defined(SCALER_FPGA_F34)

        case eBOARD_SSP_CS_F34:
            return &psProcessMutex->Scaler_Mutex;

#else

        #error "!!! undefine scaler"

#endif

        default:
            //LOG_MSG(db_ALWAYS, "unknown SPI Mutex %d\n", ucBus);
            return &psProcessMutex->Scaler_Mutex;
    }
}

uint8 SpiMutexTake(uint8 ucBus)
{
#ifndef SIMULATOR_ISCALER
    int err;

#ifdef SHM_FLOCK_PROTECT
    flock(SharedMem_GetFd(eSB_PROCESS_MUTEX_DATA), LOCK_EX);
#endif

    pthread_mutex_t *Mutex = SpiMutexGet(ucBus);
    struct timespec s_timeout;
    clock_gettime(CLOCK_REALTIME, &s_timeout);
    util_TimespecAddms(&s_timeout, SPI_SEMPHORE_WAIT_DELAY);
    err = pthread_mutex_timedlock(Mutex , &s_timeout);

#ifdef SHM_FLOCK_PROTECT
    msync((char *)SharedMem_GetMapPtr(eSB_PROCESS_MUTEX_DATA), SharedMem_GetSize(eSB_PROCESS_MUTEX_DATA), MS_SYNC);
    flock(SharedMem_GetFd(eSB_PROCESS_MUTEX_DATA), LOCK_UN);
#endif

    if(err != 0)
    {
        LOG_MSG(db_ALWAYS, "012 %s Bus %d pthread_mutex_timedlock fail [%s]\n", __FUNCTION__, ucBus, strerror(err));
        return SPI_ERROR;
    }
#endif /* SIMULATOR_ISCALER */

    return SPI_OK;  //pass
}

uint8 SpiMutexGive(uint8 ucBus)
{
#ifndef SIMULATOR_ISCALER
    int err;

#ifdef SHM_FLOCK_PROTECT
    flock(SharedMem_GetFd(eSB_PROCESS_MUTEX_DATA), LOCK_EX);
#endif

    pthread_mutex_t *Mutex = SpiMutexGet(ucBus);
    err = pthread_mutex_unlock(Mutex);

#ifdef SHM_FLOCK_PROTECT
    msync((char *)SharedMem_GetMapPtr(eSB_PROCESS_MUTEX_DATA), SharedMem_GetSize(eSB_PROCESS_MUTEX_DATA), MS_SYNC);
    flock(SharedMem_GetFd(eSB_PROCESS_MUTEX_DATA), LOCK_UN);
#endif

    if(err != 0)
    {
        LOG_MSG(db_ALWAYS, "%s pthread_mutex_unlock fail [%s]\n", __FUNCTION__, strerror(err));
        return SPI_ERROR;
    }
#endif /* SIMULATOR_ISCALER */

    return SPI_OK;  //pass
}


//==============================================================================================
static sBOARD_SPI_CFG gSpiConfig[eBOARD_SSP_CS_NUMBERS] =
{
    {
        .Speed  = SPI_SPEED,
        .Mode   = SPI_MODE_3,
        .Bits   = SPI_BITS,
        .Delay  = 0,
        .CS     = 0,
        .GpioCS = GPIO4_IO24_DISP0_DAT3,      //(GPIO4_IO24)
        .ucMsgIdx = 0
    },
    {
        .Speed  = SPI_SPEED,
        .Mode   = SPI_MODE_3,
        .Bits   = SPI_BITS,
        .Delay  = 0,
        .CS     = 0,
        .GpioCS = GPIO4_IO09_KEY_ROW1, //(GPIO4_IO09)
        .ucMsgIdx = 0
    }
};


uint8 SPI_SetChipSet(uint32 gpio, uint8 value)
{
    if(gpio != 0)
    {
        if(GPIO_Write(gpio, value) != GPIO_OK)
        {
            printf("SPI: can't set CS gpio\n");
            return SPI_ERROR;
        }
        return SPI_OK;
    }
    else
    {
        return SPI_OK;
    }
}

//==============================================================================================

static inline void SPI_RxEnhanced_Enable(BOOL bEnable)  //G100_Owen_0080
{
    UINT8 ucData = 0;

    if(bEnable)
    {
        ucData = 1;
    }

    GPIO_Write(SPI_RXE_MODE_PIN, ucData);
}


static uint8 SPI_InitDev(eBOARD_SSP_CS eCS)
{
    UINT8 *dev[eBOARD_SSP_CS_NUMBERS] = {SPI_DEV, SPI_DEV};
    UINT8 ucIdx;

    if(eCS >= eBOARD_SSP_CS_NUMBERS)
        return SPI_ERROR;

    gSpiConfig[eCS].DevFd = open(dev[eCS], O_WRONLY);
    if(gSpiConfig[eCS].DevFd < 0)
    {
        printf("SPI: can't open %s\n", dev[eCS]);
        return SPI_ERROR;
    }

    usleep(100 * 1000); //100 ms for spi device ready

    // spi mode
    if(ioctl(gSpiConfig[eCS].DevFd, SPI_IOC_WR_MODE, &gSpiConfig[eCS].Mode) < 0)
    {
        printf("SPI: can't set spi mode");
        return SPI_ERROR;
    }

    if(ioctl(gSpiConfig[eCS].DevFd, SPI_IOC_RD_MODE, &gSpiConfig[eCS].Mode) < 0)
    {
        printf("SPI: can't get spi mode");
        return SPI_ERROR;
    }
//    gSpiConfig.Mode = mode;

    // spi bits
    if(ioctl(gSpiConfig[eCS].DevFd, SPI_IOC_WR_BITS_PER_WORD, &gSpiConfig[eCS].Bits) < 0)
    {
        printf("SPI: can't set spi bits");
        return SPI_ERROR;
    }

    if(ioctl(gSpiConfig[eCS].DevFd, SPI_IOC_RD_BITS_PER_WORD, &gSpiConfig[eCS].Bits) < 0)
    {
        printf("SPI: can't get spi bits");
        return SPI_ERROR;
    }

    // spi speed
    if(ioctl(gSpiConfig[eCS].DevFd, SPI_IOC_WR_MAX_SPEED_HZ, &gSpiConfig[eCS].Speed) < 0)
    {
        printf("SPI: can't set spi speed");
        return SPI_ERROR;
    }

    if(ioctl(gSpiConfig[eCS].DevFd, SPI_IOC_RD_MAX_SPEED_HZ, &gSpiConfig[eCS].Speed) < 0)
    {
        printf("SPI: can't get spi speed");
        return SPI_ERROR;
    }

    for(ucIdx = 0; ucIdx < SPI_MSG_SIZE; ucIdx++)
    {
        gSpiConfig[eCS].paucTxBuf[ucIdx] = 0;
    }

#ifdef SPI_GPIOCS
    if(GPIO_Open(gSpiConfig[eCS].GpioCS, GPIO_OUT) != GPIO_OK)
    {
        printf("SPI #%d: chip select open failed\r\n", eCS);
    }

    if(GPIO_Write(gSpiConfig[eCS].GpioCS, GPIO_HIGH) != GPIO_OK)
    {
        printf("SPI #%d: chip select write failed\r\n", eCS);
    }
#endif

    printf("SPI Info:\n");
    printf("    Device  :     %s\n", dev[eCS]);
    printf("    Speed   :     %d\n", gSpiConfig[eCS].Speed);
    printf("    Mode    :     %d\n", gSpiConfig[eCS].Mode);
    printf("    Bits    :     %d\n", gSpiConfig[eCS].Bits);
    printf("    Deleay  :     %d\n", gSpiConfig[eCS].Delay);
    printf("    CS      :     %d\n", gSpiConfig[eCS].CS);

    //usleep(100 * 1000); //100ms for spi device ready

//    GPIO_Write(SPI_CS_GPIO, GPIO_LOW);
//    GPIO_Read(SPI_CS_GPIO, &ret);
//    printf("read gpio %d = %d\r\n", SPI_CS_GPIO, ret);
//    m_cEnDev = (uint8)eCS;

    return SPI_OK;
}


//==============================================================================================

#ifdef SCALER_FPGA_F34
uint8 SPI_SendWriteCmd(uint8 *txData, uint32 txLen)
{
    uint8 *txBuf = NULL;
    uint8  result = SPI_OK;

    txBuf = (uint8 *)malloc(txLen);

    memset(txBuf, 0, txLen);
    memcpy(txBuf, txData, txLen);

#ifdef SPI_GPIOCS
    if(SPI_SetChipSet(gSpiConfig[eBOARD_SSP_CS_F34].GpioCS, GPIO_LOW) == SPI_ERROR)
    {
        printf("SPI chip select write failed\r\n");
    }
#endif /* SPI_GPIOCS */

    m_txbuffer[eBOARD_SSP_CS_F34].rx_buf = 0;
    m_txbuffer[eBOARD_SSP_CS_F34].tx_buf = (unsigned long)txBuf;
    m_txbuffer[eBOARD_SSP_CS_F34].len = txLen;
    m_txbuffer[eBOARD_SSP_CS_F34].speed_hz = SPI_WRITE_SPEED;
    m_txbuffer[eBOARD_SSP_CS_F34].bits_per_word = 8;
    m_txbuffer[eBOARD_SSP_CS_F34].delay_usecs = gSpiConfig[eBOARD_SSP_CS_F34].Delay;
    m_txbuffer[eBOARD_SSP_CS_F34].cs_change = 0;

    if(ioctl(gSpiConfig[eBOARD_SSP_CS_F34].DevFd, SPI_IOC_MESSAGE(1), &m_txbuffer[eBOARD_SSP_CS_F34]) < 1)
    {
        printf("%s Len %d ioctl: %s\n", __FUNCTION__, txLen, strerror(errno));
        //DEBUGP("SPI_SendWriteCmd can't send spi write msg len [%02x][%02x][%02x][%02x][%d]\n", txData[0], txData[1], txData[2], txData[3], txLen);
        result = SPI_ERROR;
    }

#ifdef SPI_GPIOCS
    if(SPI_SetChipSet(gSpiConfig[eBOARD_SSP_CS_F34].GpioCS, GPIO_HIGH) == SPI_ERROR)
    {
        printf("SPI chip select write failed\r\n");
    }
#endif /* SPI_GPIOCS */

    free(txBuf);

	dvSPI_Dev_RunTimeCount_Set(0, result);

    return result;
}
//==============================================================================================
uint8 SPI_SendReadCmd(uint8 *txData, uint8 *rxData, uint32 txLen, uint32 rxLen)
{
    uint8 txBuf[8] = {0};
    uint8 *rxBuf = NULL;

    int i = 0;
    uint8  result = SPI_OK;

    rxBuf = (uint8 *)malloc(rxLen);

    memset(rxBuf, 0, rxLen);
    memcpy(txBuf, txData, txLen);

    //txBuf[0] = txData[0];

#ifdef SPI_GPIOCS
    if(SPI_SetChipSet(gSpiConfig[eBOARD_SSP_CS_F34].GpioCS, GPIO_LOW) == SPI_ERROR)
    {
        printf("SPI chip select write failed\r\n");
    }
#endif /* SPI_GPIOCS */

    m_rxbuffer[eBOARD_SSP_CS_F34].rx_buf = (unsigned long)rxBuf;
    m_rxbuffer[eBOARD_SSP_CS_F34].tx_buf = (unsigned long)txBuf;
    m_rxbuffer[eBOARD_SSP_CS_F34].len = rxLen;
    m_rxbuffer[eBOARD_SSP_CS_F34].speed_hz = SPI_READ_SPEED;
    m_rxbuffer[eBOARD_SSP_CS_F34].bits_per_word = 8;
    m_rxbuffer[eBOARD_SSP_CS_F34].delay_usecs = gSpiConfig[eBOARD_SSP_CS_F34].Delay;
    m_rxbuffer[eBOARD_SSP_CS_F34].cs_change = 0;

    if(ioctl(gSpiConfig[eBOARD_SSP_CS_F34].DevFd, SPI_IOC_MESSAGE(1), &m_rxbuffer[eBOARD_SSP_CS_F34]) < 1)
    {
        DEBUGP("SPI: can't send spi write msg\n");
        result = SPI_ERROR;
    }

#ifdef SPI_GPIOCS
    if(SPI_SetChipSet(gSpiConfig[eBOARD_SSP_CS_F34].GpioCS, GPIO_HIGH) == SPI_ERROR)
    {
        printf("SPI chip select write failed\r\n");
    }
#endif /* SPI_GPIOCS */

    if(result == SPI_OK)
    {
        memcpy(rxData, rxBuf, rxLen);
    }

    free(rxBuf);

	dvSPI_Dev_RunTimeCount_Set(0, result);

    return result;
}

#else   //iChips

uint8 SPI_SendWriteCmd(eBOARD_SSP_CS eCS, uint8 *txData, uint32 txLen)
{
    uint8 *txBuf = NULL;
    uint8 ucMsgIdx, ucIdx;
    uint8 result = SPI_ERROR;

    if(eCS >= eBOARD_SSP_CS_NUMBERS)
        return result;

#ifdef NO_INTERFACE
    return SPI_OK;
#else
#ifdef SPI_GPIOCS
    if(GPIO_Write(gSpiConfig[eCS].GpioCS, GPIO_LOW) != GPIO_OK)
    {
        printf("SPI #%d: chip select write failed\r\n", eCS);
    }
#endif

    txBuf = (uint8 *)calloc(txLen, sizeof(uint8));
    memcpy(txBuf, txData, txLen);

    m_txbuffer[eCS].rx_buf = 0;
    m_txbuffer[eCS].tx_buf = (unsigned long)txBuf;
    m_txbuffer[eCS].len = txLen;
    m_txbuffer[eCS].speed_hz = gSpiConfig[eCS].Speed;
    m_txbuffer[eCS].bits_per_word = gSpiConfig[eCS].Bits;
    m_txbuffer[eCS].delay_usecs = gSpiConfig[eCS].Delay;
    m_txbuffer[eCS].cs_change = gSpiConfig[eCS].CS;

    ucMsgIdx = gSpiConfig[eCS].ucMsgIdx;

    {
        if(ioctl(gSpiConfig[eCS].DevFd, SPI_IOC_MESSAGE(1), &m_txbuffer[eCS]) < 1)
        {
            LOG_MSG(db_ALWAYS, "(func:%s, line:%d): SPI#%d: can't send spi write msg\r\n", __FUNCTION__, __LINE__, eCS);
        }
        else
        {
            result = SPI_OK;
        }
    }
    gSpiConfig[eCS].ucMsgIdx = ucMsgIdx;

    free(txBuf);
#ifdef SPI_GPIOCS
    if(GPIO_Write(gSpiConfig[eCS].GpioCS, GPIO_HIGH) != GPIO_OK)
    {
        printf("SPI #%d: chip select write failed\r\n", eCS);
    }
#endif
#endif /* NO_INTERFACE */
	dvSPI_Dev_RunTimeCount_Set(eCS, result);

    return result;
}

//==============================================================================================
uint8 SPI_SendReadCmd(eBOARD_SSP_CS eCS, uint8 *txData, uint8 *rxData, uint32 txLen, uint32 rxLen)
{
    uint8  ucMsgIdx, ucIdx;
    uint8  result = SPI_ERROR;

    if(eCS >= eBOARD_SSP_CS_NUMBERS)
        return result;

#ifdef NO_INTERFACE
    return SPI_OK;
#else
    if((eCS == BOARD_SSP_CS0) && SPI_RXE_ENABLE)
    {
        SPI_RxEnhanced_Enable(TRUE);
    }

    m_rxbuffer[eCS].rx_buf = (unsigned long)rxData;
    m_rxbuffer[eCS].tx_buf = (unsigned long)txData;
    m_rxbuffer[eCS].len = rxLen;//txLen + rxLen;
    m_rxbuffer[eCS].speed_hz = gSpiConfig[eCS].Speed;
    m_rxbuffer[eCS].bits_per_word = gSpiConfig[eCS].Bits;
    m_rxbuffer[eCS].delay_usecs = gSpiConfig[eCS].Delay;
    m_rxbuffer[eCS].cs_change = gSpiConfig[eCS].CS;

    ucMsgIdx = gSpiConfig[eCS].ucMsgIdx;

#ifdef SPI_GPIOCS
    if(GPIO_Write(gSpiConfig[eCS].GpioCS, GPIO_LOW) != GPIO_OK)
    {
        printf("SPI #%d: chip select write failed\r\n", eCS);
    }
#endif

    if(ioctl(gSpiConfig[eCS].DevFd, SPI_IOC_MESSAGE(1), &m_rxbuffer[eCS]) < 1)
    {
        DEBUGP("SPI: can't send spi write msg\n");
    }
    else
    {
        result = SPI_OK;
    }

#ifdef SPI_GPIOCS
    if(GPIO_Write(gSpiConfig[eCS].GpioCS, GPIO_HIGH) != GPIO_OK)
    {
        printf("SPI #%d: chip select write failed\r\n", eCS);
    }
#endif

    if((eCS == BOARD_SSP_CS0) && SPI_RXE_ENABLE)
    {
        SPI_RxEnhanced_Enable(FALSE);
    }
#endif /* NO_INTERFACE */

	dvSPI_Dev_RunTimeCount_Set(eCS, result);

    return result;
}

uint8 SPI_SendBurstWriteCmd(eBOARD_SSP_CS eCS, uint8 *txData, uint32 txLen, uint8 fifo)
{
//    uint8 *txBuf = NULL;
    uint8  result = SPI_ERROR;
    uint8 ucMsgIdx, ucIdx;

    if(eCS >= eBOARD_SSP_CS_NUMBERS)
        return result;

//    txBuf = (uint8 *)malloc((txLen + 8) * sizeof(uint8));
//    memset(txBuf, 0, txLen);
//    memcpy(txBuf, txData, txLen);
    m_txbuffer[eCS].rx_buf = 0;
    m_txbuffer[eCS].tx_buf = (unsigned long)txData;
    m_txbuffer[eCS].len = txLen;
    m_txbuffer[eCS].speed_hz = SPI_BURST_SPEED;
    m_txbuffer[eCS].bits_per_word = gSpiConfig[eCS].Bits;//32;
    m_txbuffer[eCS].delay_usecs = gSpiConfig[eCS].Delay;
    m_txbuffer[eCS].cs_change = gSpiConfig[eCS].CS;

    ucMsgIdx = gSpiConfig[eCS].ucMsgIdx;

#ifdef NO_INTERFACE
    return SPI_OK;
#else
#ifdef SPI_GPIOCS
    if(GPIO_Write(gSpiConfig[eCS].GpioCS, GPIO_LOW) != GPIO_OK)
    {
        printf("SPI #%d: chip select write failed\r\n", eCS);
    }
    else
#endif
    {
        if(ioctl(gSpiConfig[eCS].DevFd, SPI_IOC_MESSAGE(fifo), &m_txbuffer[eCS]) < 1)
        {
            DEBUGP("SPI_SendBurstWriteCmd can't send spi write msg len [%02x][%02x][%02x][%02x][%d]\n", txData[0], txData[1], txData[2], txData[3], txLen);
        }
        else
        {
            result = SPI_OK;
        }
    }

//    free(txBuf);
#ifdef SPI_GPIOCS
    if(GPIO_Write(gSpiConfig[eCS].GpioCS, GPIO_HIGH) != GPIO_OK)
    {
        printf("SPI #%d: chip select write failed\r\n", eCS);
    }
#endif
#endif /* NO_INTERFACE */

	//dvSPI_Dev_RunTimeCount_Set(eCS, result);

    return result;
}


#endif
//==============================================================================================

BOOL Board_SSP_SemaphoreGive(eBOARD_SSP_CS eCS)
{
    if(eCS < eBOARD_SSP_CS_NUMBERS)
    {
        if(SPI_OK == SpiMutexGive(eCS))
        {
            return TRUE;
        }
        return FALSE;
    }
}

BOOL Board_SSP_SemaphoreTake(eBOARD_SSP_CS eCS)
{
    if(eCS < eBOARD_SSP_CS_NUMBERS)
    {
        if(SPI_OK == SpiMutexTake(eCS))
        {
            return TRUE;
        }
        return FALSE;
    }
}

#ifdef SCALER_FPGA_F34
void Board_SPI_Init(void)
{
    if(SPI_InitDev(eBOARD_SSP_CS_F34) == SPI_OK)
    {
        SpiMutexCreate(eBOARD_SSP_CS_F34);
        return;
    }
}
#else
void Board_SPI_Init(void)
{
    UINT8 ucDev;

    for(ucDev=0; ucDev<eBOARD_SSP_CS_NUMBERS; ucDev++)
    {
        if(SPI_InitDev(ucDev) == SPI_OK)
        {
            SpiMutexCreate(ucDev);
        }
        else
        {
            printf("%s SPI init failed !\n", (ucDev == BOARD_SSP_CS0 ? "SSP0" : "SSP1"));
        }

        usleep(100);
    }

    if(SPI_RXE_ENABLE)
    {
        if(GPIO_Open(SPI_RXE_MODE_PIN, GPIO_OUT) != GPIO_OK)
        {
            printf("control FPGA mode pin init failed !\r\n");
        }

        if(GPIO_Write(SPI_RXE_MODE_PIN, GPIO_LOW) != GPIO_OK)
        {
            printf("control FPGA mode pin write failed\r\n");
        }
#if 0   //EE debug
        if(GPIO_Open(GPIO2_IO00_NANDF_D0, GPIO_OUT) != GPIO_OK)
        {
            printf("control FPGA mode pin init failed !\r\n");
        }

        if(GPIO_Write(GPIO2_IO00_NANDF_D0, GPIO_LOW) != GPIO_OK)
        {
            printf("control FPGA mode pin write failed\r\n");
        }
#endif
    }
}
#endif /* SCALER_FPGA_F34 */

