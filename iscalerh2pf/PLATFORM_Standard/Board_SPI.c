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
#include <sys/mman.h>
#include <linux/types.h>
#include "gpio.h"
#include "Board_SPI.h"
#include "utilDbgMsg.h"
#include "dvInterfaceDiag.h"

//========================================Semaphore===========================================
#define SPI_SEMPHORE_WAIT_DELAY (10000) //A70LV_Larry_0052
//#define SHM_FLOCK_PROTECT   //maybe deadlock, don`t open

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

    if(ucBus == eBOARD_SSP_CS_C821)
    {
        psProcessMutex->Scaler_Mutex       = axSPI_Mutex[eBOARD_SSP_CS_C821];
        psProcessMutex->Scaler_CurrentBank = 0xFF;
    }
    else if(ucBus == eBOARD_SSP_CS_C789)
    {
        psProcessMutex->Warping_Mutex       = axSPI_Mutex[eBOARD_SSP_CS_C789];
        psProcessMutex->Warping_CurrentBank = 0xFF;
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
        case eBOARD_SSP_CS_C821:
            return &psProcessMutex->Scaler_Mutex;

        case eBOARD_SSP_CS_C789:
            return &psProcessMutex->Warping_Mutex;

        default:
            LOG_MSG(db_ALWAYS, "unknown SPI Mutex %d\n", ucBus);
            return &psProcessMutex->Scaler_Mutex;
    }
}

uint8 SpiMutexTake(uint8 ucBus)
{
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

    return SPI_OK;  //pass
}

uint8 SpiMutexGive(uint8 ucBus)
{
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

    return SPI_OK;  //pass
}

#define SPI_DEV_C789        "/dev/spidev2.0"
#define SPI_DEV_C821        "/dev/spidev0.0"

#define SPI_SPEED           (10 * 1000 * 1000) // 10M
#define SPI_BURST_SPEED     (20 * 1000 * 1000) // 20M
#define SPI_BITS    (8)//(16)

#define SPI_WRITE   1
#define SPI_READ    0
#define SPI_RXE_MODE_PIN    GPIO2_IO01_NANDF_D1 //G100_Owen_0080
#define SPI_RXE_ENABLE      TRUE                //G100_Owen_0080 : work with Y00.31 up

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

static struct spi_ioc_transfer m_sIocMsgTx[eBOARD_SSP_CS_NUMBERS];
static struct spi_ioc_transfer m_sIocMsgRx[eBOARD_SSP_CS_NUMBERS];
static UINT32 dwDevFdMem;
static UINT32 *gpioBase;
static UINT8 m_ucMsgIdx = 0;
//static UINT8 *m_paucTxBuf[SPI_MSG_SIZE];

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
    UINT8 *dev[eBOARD_SSP_CS_NUMBERS] = {SPI_DEV_C789, SPI_DEV_C821};
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
uint8 SPI_SendWriteCmd(eBOARD_SSP_CS eCS, uint8 *txData, uint32 txLen, uint8 fifo)
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

    m_sIocMsgTx[eCS].rx_buf = 0;
    m_sIocMsgTx[eCS].tx_buf = (unsigned long)txBuf;
    m_sIocMsgTx[eCS].len = txLen;
    m_sIocMsgTx[eCS].speed_hz = gSpiConfig[eCS].Speed;
    m_sIocMsgTx[eCS].bits_per_word = gSpiConfig[eCS].Bits;
    m_sIocMsgTx[eCS].delay_usecs = gSpiConfig[eCS].Delay;
    m_sIocMsgTx[eCS].cs_change = gSpiConfig[eCS].CS;

    ucMsgIdx = gSpiConfig[eCS].ucMsgIdx;

    {
        if(ioctl(gSpiConfig[eCS].DevFd, SPI_IOC_MESSAGE(1), &m_sIocMsgTx[eCS]) < 1)
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
    if((eCS == eBOARD_SSP_CS_C789) && SPI_RXE_ENABLE)
    {
        SPI_RxEnhanced_Enable(TRUE);
    }

    m_sIocMsgRx[eCS].rx_buf = (unsigned long)rxData;
    m_sIocMsgRx[eCS].tx_buf = (unsigned long)txData;
    m_sIocMsgRx[eCS].len = rxLen;//txLen + rxLen;
    m_sIocMsgRx[eCS].speed_hz = gSpiConfig[eCS].Speed;
    m_sIocMsgRx[eCS].bits_per_word = gSpiConfig[eCS].Bits;
    m_sIocMsgRx[eCS].delay_usecs = gSpiConfig[eCS].Delay;
    m_sIocMsgRx[eCS].cs_change = gSpiConfig[eCS].CS;

    ucMsgIdx = gSpiConfig[eCS].ucMsgIdx;

#ifdef SPI_GPIOCS
    if(GPIO_Write(gSpiConfig[eCS].GpioCS, GPIO_LOW) != GPIO_OK)
    {
        printf("SPI #%d: chip select write failed\r\n", eCS);
    }
#endif

    if(ioctl(gSpiConfig[eCS].DevFd, SPI_IOC_MESSAGE(1), &m_sIocMsgRx[eCS]) < 1)
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

    if((eCS == eBOARD_SSP_CS_C789) && SPI_RXE_ENABLE)
    {
        SPI_RxEnhanced_Enable(FALSE);
    }
#endif /* NO_INTERFACE */

	dvSPI_Dev_RunTimeCount_Set(eCS, result);

    return result;
}

//==============================================================================================
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
    m_sIocMsgTx[eCS].rx_buf = 0;
    m_sIocMsgTx[eCS].tx_buf = (unsigned long)txData;
    m_sIocMsgTx[eCS].len = txLen;
    m_sIocMsgTx[eCS].speed_hz = SPI_BURST_SPEED;
    m_sIocMsgTx[eCS].bits_per_word = gSpiConfig[eCS].Bits;//32;
    m_sIocMsgTx[eCS].delay_usecs = gSpiConfig[eCS].Delay;
    m_sIocMsgTx[eCS].cs_change = gSpiConfig[eCS].CS;

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
        if(ioctl(gSpiConfig[eCS].DevFd, SPI_IOC_MESSAGE(fifo), &m_sIocMsgTx[eCS]) < 1)
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

	dvSPI_Dev_RunTimeCount_Set(eCS, result);

    return result;
}

//==============================================================================================

BOOL Board_SSP_SemaphoreGive(eBOARD_SSP_CS eCS)
{
    if(eCS < eBOARD_SSP_CS_NUMBERS)
    {
        return SpiMutexGive(eCS);
    }
    else
    {
        return FALSE;
    }
}

BOOL Board_SSP_SemaphoreTake(eBOARD_SSP_CS eCS)
{
    if(eCS < eBOARD_SSP_CS_NUMBERS)
    {
        return SpiMutexTake(eCS);
    }
    else
    {
        return FALSE;
    }
}

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
            printf("%s SPI init failed !\n", (ucDev == eBOARD_SSP_CS_C821 ? "C821" : "C789"));
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

#if 0
static void reverse_array(UINT8 *parray, UINT32 uiSize)
{
    UINT32 uiIdx;
    UINT8 ucTmp = 0;

    // Swapping
    for(uiIdx = 0; uiIdx < uiSize/2; uiIdx++)
    {
        ucTmp = parray[uiIdx];
        parray[uiIdx] = parray[uiSize-uiIdx-1];
        parray[uiSize-uiIdx-1] = ucTmp;
    }
}
#endif /* 0 */

void SPI_Test(void)
{
    static UINT8 ucTgl;
    UINT16 ucIdx;
    uint8 aucTxData[12] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC};
    UINT8 ucTxBuffer[1024] = {0};
    UINT8 ucReg = 0xDD, ucCount=0;
#if 0
//    ucTgl ^= GPIO_HIGH;
//
//    GPIO_Write(SPI_CS_GPIO, ucTgl);
//    *pcData = ucTgl;
#else
#if 0
    for(ucIdx=0; ucIdx<256; ucIdx++)
    {
        ucTxBuffer[ucIdx] = ucIdx;//+1;
        SPI_SendWriteCmd(eBOARD_SSP_CS_C789, ucTxBuffer, ucIdx+1, 1);
        SPI_SendWriteCmd(eBOARD_SSP_CS_C821, ucTxBuffer, ucIdx+1, 1);
        usleep(10);
//        if((ucIdx % 2) == 0)
//        {
//            ucTxBuffer[ucCount++] = ucReg;
//        }
//        ucTxBuffer[ucCount++] = aucTxData[ucIdx];
    }
#endif
//    SPI_SendBurstWriteCmd(eBOARD_SSP_CS_C789, ucTxBuffer, 64, 1);
//    SPI_SendBurstWriteCmd(eBOARD_SSP_CS_C821, aucTxData, 12, 1);
    //usleep(10 * 1000);  //10 ms
//    SPI_SendWriteCmd(eBOARD_SSP_CS_C821, aucTxData, 12, 1);
    SPI_SendWriteCmd(eBOARD_SSP_CS_C789, aucTxData, 12, 1);

//    for(ucIdx=0; ucIdx<ucCount; ucIdx+=3)
//    {
//        reverse_array(&ucTxBuffer[ucIdx], 3);
//    }
//    SPI_SendBurstWriteCmd(eBOARD_SSP_CS_C789, aucTxData, 12, 1);
#endif
}


