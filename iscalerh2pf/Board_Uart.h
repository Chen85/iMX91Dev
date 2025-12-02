#ifndef BOARD_UART_H
#define BOARD_UART_H
// ==============================================================================
// FILE NAME: BOARD_UART.H
// DESCRIPTION:
//
//
// modification history
// --------------------
// 09/01/2013, Leohong written
// --------------------
// ==============================================================================

#include "Common.h"

#define uaPCUART    eBOARD_UART_UART1//eBOARD_UART_UART0
//#define uaNXPUART   eBOARD_UART_UART1
#define uaNXPLAN    eBOARD_UART_UART2
#define uaMCU       eBOARD_UART_UART0
#define uaMotor     eBOARD_UART_UART3


//==============================================================================
#define UART_OK         0
#define UART_ERROR      (-1)
#define UART_ERR_OPEN   (-2)
#define UART_ERR_SET    (-3)
//==============================================================================
//#define UART_SCALER_DEV         "/dev/ttymxc1"
//#define UART_FRONTEND_DEV       "/dev/ttymxc2"
//==============================================================================

typedef enum
{
    eBOARD_UART_UART0,  //STB MCU
    eBOARD_UART_UART1,  //imx6 port
    eBOARD_UART_UART2,  //PC port
    eBOARD_UART_UART3,
    eBOARD_UART_UART4,  //Motor

    eBOARD_UART_NUMBERS,
} eBOARD_UART;

//==============================================================================


INT8 Board_Uart_Init(eBOARD_UART cPort);
INT8 Board_Uart_DeInit(eBOARD_UART ePort);
INT8 Board_Uart_Baudrate_Change(eBOARD_UART ePort, UINT32 ulBaudrate);
eRESULT Board_Uart_Write(eBOARD_UART ePort, UINT16 uiSize, UINT8 *pucOutBuffer);
UINT16 Board_Uart_Read(eBOARD_UART ePort, UINT16 uiSize, UINT8 *pucInBuffer);
void Board_Uart_Rx_FifoFlush(eBOARD_UART ePort);
void Board_HW_Read_UART0(void);
void Board_HW_Read_UART1(void);
void Board_HW_Read_UART2(void);
void Board_HW_Read_UART3(void);
void Board_HW_Read_PCUART(void);

#endif // BOARD_UART_H

