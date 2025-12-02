// ==============================================================================
// FILE NAME: BOARD_UART.C
// DESCRIPTION:
//
//
// modification history
// --------------------
// 09/01/2013, Leohong Create
// --------------------
// ==============================================================================

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <string.h>

//#include "uart.h"

#include "utilQueueAPI.h"
#include "Board_Uart.h"

#define RX0_BUFFER_SIZE  1024
#define RX1_BUFFER_SIZE  1024
#define RX2_BUFFER_SIZE  1024 //A70LV_Larry_0382 modify
#define RX3_BUFFER_SIZE  1024
#define RX4_BUFFER_SIZE  1024

static UINT8  m_aucRx0Buffer[RX0_BUFFER_SIZE];
static UINT8  m_aucRx1Buffer[RX1_BUFFER_SIZE];
static UINT8  m_aucRx2Buffer[RX2_BUFFER_SIZE];
static UINT8  m_aucRx3Buffer[RX3_BUFFER_SIZE];
static UINT8  m_aucRx4Buffer[RX4_BUFFER_SIZE];
static sQUEUE m_asUartRxRingBuffer[eBOARD_UART_NUMBERS];


#if (SYSTEM_OS_TYPE == POSIX_COMPLIANT)

pthread_mutex_t axUart_Mutex[eBOARD_UART_NUMBERS] ;
#define UART_SEMPHORE_WAIT_DELAY    (5000)
#define UART_MutexCreate(x) pthread_mutex_init(&axUart_Mutex[x], NULL)

uint8 UART_MutexTake(uint8 ucBus)
{
    struct timespec s_timeout;
    clock_gettime(CLOCK_REALTIME, &s_timeout);
    util_TimespecAddms(&s_timeout, UART_SEMPHORE_WAIT_DELAY);

    if(pthread_mutex_timedlock(&axUart_Mutex[ucBus] , &s_timeout) != 0)
    {
        //LOG_MSG(db_ALWAYS, "013 %s pthread_mutex_timedlock fail [%s]\n", __FUNCTION__ , strerror(errno));
        return UART_ERROR;
    }

    return UART_OK;  //pass
}

uint8 UART_MutexGive(uint8 ucBus)
{
    if(pthread_mutex_unlock(&axUart_Mutex[ucBus]) != 0)
    {
        //LOG_MSG(db_ALWAYS, "%s pthread_mutex_unlock fail [%s]\n", __FUNCTION__ , strerror(errno));
        return UART_ERROR;
    }

    return UART_OK;  //pass
}

#else
SemaphoreHandle_t axUARTSemaphore[eBOARD_UART_NUMBERS] = {NULL};

#define UART_SEMPHORE_WAIT_DELAY (5000) //A70LV_Larry_0375

#define UART_MutexCreate(x)   axUARTSemaphore[x] = xSemaphoreCreateMutex();
#define UART_MutexTake(x)     xSemaphoreTake(axUARTSemaphore[x], UART_SEMPHORE_WAIT_DELAY/portTICK_RATE_MS); //A70LV_Larry_0375
#define UART_MutexGive(x)     xSemaphoreGive(axUARTSemaphore[x]);
#endif


volatile struct
{
    INT32   UartFd;
    UINT8    *cDev;
    INT32   ulBaudrate;      // UART Baudrate
    UINT8   ucDatabit;
    UINT8   ucStopBit;
    char    ucPriority;

    sQUEUE      *sRxRingBuffer;
    UINT8       *aucRxBuffer;
    UINT32      ulRxBufferSize;
} m_asBoard_Uart_Cfg[eBOARD_UART_NUMBERS] =
{
    /**************
    /dev/ttymxc0 -> debug
    /dev/ttymxc1 -> MCU
    /dev/ttymxc2 ->
    /dev/ttymxc3 ->
    /dev/ttymxc4 ->
    ****************/

    // UART Port #0
    {
        -1,
        "/dev/ttymxc1",
        115200,    // UART Baudrate
        8,          //ucDatabit
        1,          //ucStopBit
        'N',        //ucPriority
        &m_asUartRxRingBuffer[eBOARD_UART_UART0],
        m_aucRx0Buffer,
        RX0_BUFFER_SIZE,
    },
    // UART Port #1
    {
        -1,
        "/dev/ttymxc2",
        115200,    // UART Baudrate
        8,          //ucDatabit
        1,          //ucStopBit
        'N',        //ucPriority
        &m_asUartRxRingBuffer[eBOARD_UART_UART1],
        m_aucRx1Buffer,
        RX1_BUFFER_SIZE,
    },
    // UART Port #2
    {
        -1,
        "/dev/ttymxc3",
        115200,    // UART Baudrate
        8,          //ucDatabit
        1,          //ucStopBit
        'N',        //ucPriority
        &m_asUartRxRingBuffer[eBOARD_UART_UART2],
        m_aucRx2Buffer,
        RX2_BUFFER_SIZE,
    },
    // UART Port #3
    {
        -1,
        "/dev/ttymxc4",
        115200,    // UART Baudrate
        8,          //ucDatabit
        1,          //ucStopBit
        'N',        //ucPriority
        &m_asUartRxRingBuffer[eBOARD_UART_UART3],
        m_aucRx3Buffer,
        RX3_BUFFER_SIZE,
    },
    // UART Port #4
    {
        -1,
        "/dev/ttymxc0",
        115200,    // UART Baudrate
        8,          //ucDatabit
        1,          //ucStopBit
        'N',        //ucPriority
        &m_asUartRxRingBuffer[eBOARD_UART_UART4],
        m_aucRx4Buffer,
        RX4_BUFFER_SIZE,
    },
};

static int gSpeedArray[] =
{
    921600, B921600,
    460800, B460800,
    230400, B230400,
    115200, B115200,
     57600, B57600,
     38400, B38400,
     19200, B19200,
      9600, B9600,
      4800, B4800,
      2400, B2400,
      1200, B1200,
};

INT32 UART_SetParity(INT32 fd, UINT32 speed, UINT8 databits, UINT8 stopbits, char parity)
{
    UINT32   i;
    UINT32   status;
    struct termios options;

    if( tcgetattr(fd, &options)  !=  0)
    {
        DEBUGP("tcgetattr\n");
        return UART_ERROR;
    }

    for (i = 0;  i < ARRAY_SIZE(gSpeedArray);  i += 2)
    {
        if(speed == gSpeedArray[i])
        {
            tcflush(fd, TCIOFLUSH);
            cfsetispeed(&options, gSpeedArray[i + 1]);
            cfsetospeed(&options, gSpeedArray[i + 1]);
            status = tcsetattr(fd, TCSANOW, &options);
            if(status != 0)
            {
                DEBUGP("tcsetattr\n");
                return UART_ERROR;
            }
            tcflush(fd,TCIOFLUSH);
        }
    }

    options.c_cflag &= ~CSIZE;
    switch (databits)
    {
        case 7:
            options.c_cflag |= CS7;
            break;
        case 8:
            options.c_cflag |= CS8;
            break;
        default:
            DEBUGP("Unsupported data size\n");
            return UART_ERROR;
    }

    switch (parity)
    {
        case 'n':
        case 'N':
            options.c_cflag &= ~PARENB;   /* Clear parity enable */
            options.c_iflag &= ~INPCK;     /* Enable parity checking */
            break;
        case 'o':
        case 'O':
            options.c_cflag |= (PARODD | PARENB); /* */
            options.c_iflag |= INPCK;             /* Disnable parity checking */
            break;
        case 'e':
        case 'E':
            options.c_cflag |= PARENB;     /* Enable parity */
            options.c_cflag &= ~PARODD;   /* */
            options.c_iflag |= INPCK;       /* Disnable parity checking */
            break;
        case 'S':
        case 's':  /*as no parity*/
            options.c_cflag &= ~PARENB;
            options.c_cflag &= ~CSTOPB;
            break;
        default:
            DEBUGP("Unsupported parity\n");
            return UART_ERROR;
    }

    // Flow control none
    options.c_cflag &= ~CRTSCTS;
    options.c_iflag &= ~(ICRNL | IXON | IXOFF | IXANY);

    switch (stopbits)
    {
        case 1:
            options.c_cflag &= ~CSTOPB;
            break;
        case 2:
            options.c_cflag |= CSTOPB;
           break;
        default:
             fprintf(stderr,"Unsupported stop bits\n");
             return UART_ERROR;
    }

    options.c_oflag = 0;
    options.c_lflag = 0;

    options.c_cc[VTIME] = 15; /* 1.5 seconds*/
    options.c_cc[VMIN] = 0; /* Update the options and do it NOW */
    if (tcsetattr(fd, TCSANOW, &options) != 0)
    {
        DEBUGP("tcsetattr\n");
        return UART_ERROR;
    }

    tcflush(fd,TCIFLUSH);

    return UART_OK;
}

// ==============================================================================
// FUNCTION NAME: BOARD_UART_INIT
// DESCRIPTION:
//
//
// Params:
// eBOARD_UART cPort:
//
// Returns:
//
//
// modification history
// --------------------
// --------------------
// ==============================================================================
INT8 Board_Uart_Init(eBOARD_UART ePort)
{
    INT32 iResult = -1;

    if(ePort >= eBOARD_UART_NUMBERS)
    {
        return UART_ERROR;
    }

    m_asBoard_Uart_Cfg[ePort].UartFd = open(m_asBoard_Uart_Cfg[ePort].cDev, O_RDWR | O_NOCTTY | O_SYNC | O_NONBLOCK);

    DEBUGP("Open uart ePort %d %s %d %d\n", ePort, m_asBoard_Uart_Cfg[ePort].cDev, m_asBoard_Uart_Cfg[ePort].UartFd,  m_asBoard_Uart_Cfg[ePort].ulBaudrate);

    if(m_asBoard_Uart_Cfg[ePort].UartFd < 0)
    {
        DEBUGP("Open %s error\n", m_asBoard_Uart_Cfg[ePort].cDev);
        return UART_ERR_OPEN;
    }

    iResult = UART_SetParity(m_asBoard_Uart_Cfg[ePort].UartFd,
                            m_asBoard_Uart_Cfg[ePort].ulBaudrate,
                            m_asBoard_Uart_Cfg[ePort].ucDatabit,
                            m_asBoard_Uart_Cfg[ePort].ucStopBit,
                            m_asBoard_Uart_Cfg[ePort].ucPriority);

    if(iResult != UART_OK)
    {
        DEBUGP("UART_SetParity error: %s\n", m_asBoard_Uart_Cfg[ePort].cDev);
        return UART_ERR_SET;
    }

    utilQueueInitial(m_asBoard_Uart_Cfg[ePort].sRxRingBuffer,
                     m_asBoard_Uart_Cfg[ePort].ulRxBufferSize,
                     sizeof(UINT8),
                     (UINT8*)m_asBoard_Uart_Cfg[ePort].aucRxBuffer);

    UART_MutexCreate(ePort);

    return UART_OK;

}

// ==============================================================================
// FUNCTION NAME: BOARD_UART_DEINIT
// DESCRIPTION:
//
//
// Params:
// eBOARD_UART ePort:
//
// Returns:
//
//
// modification history
// --------------------
// 30/10/2013, Leo Create
// --------------------
// ==============================================================================
INT8 Board_Uart_DeInit(eBOARD_UART ePort)
{
//    struct termios options;
//    INT32 fd = 0;

    if(ePort >= eBOARD_UART_NUMBERS)
    {
        return UART_ERROR;
    }

    if(m_asBoard_Uart_Cfg[ePort].UartFd < 0)
    {
        DEBUGP("Open %s error\n", m_asBoard_Uart_Cfg[ePort].cDev);
        return UART_ERR_OPEN;
    }

    close(m_asBoard_Uart_Cfg[ePort].UartFd);
}

// ==============================================================================
// FUNCTION NAME: BOARD_UART_BAUDRATE_CHANGE
// DESCRIPTION:
//
//
// Params:
// DWORD dwBaudrate:
//
// Returns:
//
//
// modification history
// --------------------
// 2014/01/13, Leo Create
// --------------------
// ==============================================================================
INT8 Board_Uart_Baudrate_Change(eBOARD_UART ePort, UINT32 ulBaudrate)
{
    UINT32   i;
    UINT32   status;
    struct termios options;

    if(ePort >= eBOARD_UART_NUMBERS)
    {
        return UART_ERROR;
    }

    if(m_asBoard_Uart_Cfg[ePort].UartFd < 0)
    {
        DEBUGP("Baudrate_Change %s error\n", m_asBoard_Uart_Cfg[ePort].cDev);
        return UART_ERR_OPEN;
    }

    if(UART_MutexTake(ePort) == UART_OK)
    {
        if(tcgetattr(m_asBoard_Uart_Cfg[ePort].UartFd, &options)  !=  0)
        {
            DEBUGP("tcgetattr\n");
            UART_MutexGive(ePort);
            return UART_ERROR;
        }

        for (i = 0;  i < ARRAY_SIZE(gSpeedArray);  i += 2)
        {
            if(ulBaudrate == gSpeedArray[i])
            {
//                tcflush(m_asBoard_Uart_Cfg[ePort].UartFd, TCIOFLUSH); //G100_Owen_0012
                cfsetispeed(&options, gSpeedArray[i + 1]);
                cfsetospeed(&options, gSpeedArray[i + 1]);
                status = tcsetattr(m_asBoard_Uart_Cfg[ePort].UartFd, TCSANOW, &options);

                if(status != 0)
                {
                    DEBUGP("tcsetattr\n");
                    UART_MutexGive(ePort);
                    return UART_ERROR;
                }

//                tcflush(m_asBoard_Uart_Cfg[ePort].UartFd,TCIOFLUSH);  //G100_Owen_0012
                break;
            }
        }

        if(i == ARRAY_SIZE(gSpeedArray))
        {
            DEBUGP("uart speed not found\n");
            UART_MutexGive(ePort);
            return UART_ERROR;
        }

        UART_MutexGive(ePort);
    }

    return UART_OK;
}

// ==============================================================================
// FUNCTION NAME: BOARD_UART_WRITE
// DESCRIPTION:
//
//
// Params:
// eBOARD_UART ePort:
// WORD wSize:
// BYTE *pOutBuffer:
//
// Returns:
//
//
// modification history
// --------------------
// 30/10/2013, Leo Create
// --------------------
// ==============================================================================
eRESULT Board_Uart_Write(eBOARD_UART ePort, UINT16 uiSize, UINT8 *pucOutBuffer)
{
    INT32 iResult = -1;
    //printf("data = %02x %02x %02x %02x\n",pucOutBuffer[0], pucOutBuffer[1], pucOutBuffer[2], pucOutBuffer[3]);
    //printf("m_asBoard_Uart_Cfg[ePort].UartFd %d\n",m_asBoard_Uart_Cfg[ePort].UartFd);

    if(UART_MutexTake(ePort) == UART_OK)
    {
        iResult = write(m_asBoard_Uart_Cfg[ePort].UartFd, pucOutBuffer, uiSize);

        if(iResult < 0)
        {
            //DEBUGP("Uart: can't send msg[%d]\n", iResult);
            UART_MutexGive(ePort);
            return rcERROR;
        }

        UART_MutexGive(ePort);
    }

    return rcSUCCESS;
}

// ==============================================================================
// FUNCTION NAME: BOARD_UART_READ
// DESCRIPTION:
//
//
// Params:
// eBOARD_UART ePort:
// WORD wSize:
// BYTE *pucInBuffer:
//
// Returns:
//
//
// modification history
// --------------------
// 14/11/2013, Leo Create
// --------------------
// ==============================================================================
UINT16 Board_Uart_Read(eBOARD_UART ePort, UINT16 uiSize, UINT8 *pucInBuffer)
{
#if 0
    return Chip_UART_ReadRB(m_apsBoard_Uart_Reg[ePort],
                            m_asBoard_Uart_Cfg[ePort].sRxRingBuffer,
                            pucInBuffer,
                            uiSize);
#endif /* 0 */

    return utilQueueRead(m_asBoard_Uart_Cfg[ePort].sRxRingBuffer, pucInBuffer);

}



// ==============================================================================
// FUNCTION NAME: BOARD_UART_RX_FIFOFLUSH
// DESCRIPTION:
//
//
// Params:
// eBOARD_UART ePort:
//
// Returns:
//
//
// modification history
// --------------------
// 14/11/2013, Leo Create
// --------------------
// ==============================================================================
void Board_Uart_Rx_FifoFlush(eBOARD_UART ePort)
{
    //m_apsBoard_Uart_Reg[ePort]->FCR |= UART_FCR_RX_RS;
    //RingBuffer_Flush(m_asBoard_Uart_Cfg[ePort].sRxRingBuffer);
}

void Board_HW_Read_UART0(void)
{
    INT16 iLen = 0;
    UINT8 acData[256] = {0};
    INT16 iCount = 0;

    do
    {
        iLen = read(m_asBoard_Uart_Cfg[eBOARD_UART_UART0].UartFd, acData, 255);

        if(iLen < 0)
        {
            return;
        }

        for(iCount = 0; iCount < iLen; iCount++)
        {
            utilQueueWrite(m_asBoard_Uart_Cfg[eBOARD_UART_UART0].sRxRingBuffer, &acData[iCount]);
        }

    }while(iLen > 0);
}

void Board_HW_Read_UART1(void)
{
    INT16 iLen = 0;
    UINT8 acData[256] = {0};
    INT16 iCount = 0;

    do
    {
        iLen = read(m_asBoard_Uart_Cfg[eBOARD_UART_UART1].UartFd, acData, 255);

        if(iLen < 0)
        {
            return;
        }

        for(iCount = 0; iCount < iLen; iCount++)
        {
            utilQueueWrite(m_asBoard_Uart_Cfg[eBOARD_UART_UART1].sRxRingBuffer, &acData[iCount]);
        }

    }while(iLen > 0);
}

void Board_HW_Read_UART2(void)
{
    INT16 iLen = 0;
    UINT8 acData[256] = {0};
    INT16 iCount = 0;

    do
    {
        iLen = read(m_asBoard_Uart_Cfg[eBOARD_UART_UART2].UartFd, acData, 255);

        if(iLen < 0)
        {
            return;
        }

        for(iCount = 0; iCount < iLen; iCount++)
        {
            utilQueueWrite(m_asBoard_Uart_Cfg[eBOARD_UART_UART2].sRxRingBuffer, &acData[iCount]);
        }

    }while(iLen > 0);
}

void Board_HW_Read_UART3(void)
{
    INT16 iLen = 0;
    UINT8 acData[256] = {0};
    INT16 iCount = 0;

    do
    {
        iLen = read(m_asBoard_Uart_Cfg[eBOARD_UART_UART3].UartFd, acData, 255);

        if(iLen < 0)
        {
            return;
        }

        for(iCount = 0; iCount < iLen; iCount++)
        {
            utilQueueWrite(m_asBoard_Uart_Cfg[eBOARD_UART_UART3].sRxRingBuffer, &acData[iCount]);
        }

    }while(iLen > 0);
}


void Board_HW_Read_UART4(void)
{
    INT16 iLen = 0;
    UINT8 acData[256] = {0};
    INT16 iCount = 0;

    do
    {
        iLen = read(m_asBoard_Uart_Cfg[eBOARD_UART_UART4].UartFd, acData, 255);

        if(iLen < 0)
        {
            return;
        }

        for(iCount = 0; iCount < iLen; iCount++)
        {
            utilQueueWrite(m_asBoard_Uart_Cfg[eBOARD_UART_UART4].sRxRingBuffer, &acData[iCount]);
        }

    }while(iLen > 0);
}

void Board_HW_Read_PCUART(void)
{
    switch(uaPCUART)
    {
        case eBOARD_UART_UART0:
            Board_HW_Read_UART0();
            break;
        case eBOARD_UART_UART1:
            Board_HW_Read_UART1();
            break;
        case eBOARD_UART_UART2:
            Board_HW_Read_UART2();
            break;
        case eBOARD_UART_UART3:
            Board_HW_Read_UART3();
            break;
        case eBOARD_UART_UART4:
            Board_HW_Read_UART4();
            break;
    }
}


