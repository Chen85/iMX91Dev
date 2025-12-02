//#include "Board_I2C.h"
//#include "Board_I2C_Dev_Table.h"
#include "utilDbgMsg.h"
#include "dvProAV_Interface.h"
#include "dvProAV_Access.h"
#include "dvProAV_Platform.h"
#include "Board_SPI.h"
#include "Board_I2C.h"
#include "Board_Uart.h"
#include "utilMathAPI.h"

#define ALIGNMENT_BYTES 4
#define ADDRESS_BYTES 1
#define CHECKSUM_BYTES 1
#define LENGTH_BYTES 2
#define READ_TX_LEN (ADDRESS_BYTES+LENGTH_BYTES)
#define KERNEL_ALIGNMENT 256

static UINT8 spiWrite(UINT32 address, UINT32 length, const UINT8 *data)
{
    UINT8 eResult = rcSUCCESS;
    UINT8 checksum;
    UINT8 *Txpbuf = NULL;
    UINT16 tranLen;
    UINT16 count = 0;
    UINT32 datacount = 0;
    UINT32 datalen = 0;

    tranLen = length + ADDRESS_BYTES + LENGTH_BYTES + CHECKSUM_BYTES;

    Txpbuf = (uint8 *)malloc(tranLen);
    memset(Txpbuf, 0, tranLen);

    count = 0;
    datacount = 0;
    checksum = 0;

    Txpbuf[count++] = (UINT8)(address & 0x7F);
    Txpbuf[count++] = (UINT8)(length & 0xFF);

    if(LENGTH_BYTES > 1)
    {
        Txpbuf[count++] = (length >> 8) & 0xFF;
    }

    do
    {
        checksum += data[datacount];
        Txpbuf[count++] = data[datacount++];

    }while(datacount < length);

    Txpbuf[count++] = checksum;

    if(SPI_SendWriteCmd(Txpbuf, tranLen) == SPI_ERROR)
    {
        free(Txpbuf);
        return rcERROR;
    }

    free(Txpbuf);

    return eResult;
}


static UINT8 spiRead(UINT32 address, UINT32 length, UINT8 *data)
{
    UINT8 eResult = rcSUCCESS;
    UINT8 checksum;
    UINT8 *pRxbuf = NULL;
    UINT8 Txbuf[32] = {0};
    UINT16 tranLen;
    UINT16 count = 0;

    tranLen = length + ADDRESS_BYTES + LENGTH_BYTES + CHECKSUM_BYTES;
    pRxbuf = (uint8 *)malloc(tranLen);
    memset(pRxbuf, 0, tranLen);

    //tranLen = tranLen + ALIGNMENT_BYTES;

    Txbuf[0] = (UINT8)(address | 0x80);  // Modify for ProAV scaler SPI read bit7 must be 1
    Txbuf[1] = (UINT8)(length & 0xFF);

    if(LENGTH_BYTES > 1)
    {
        Txbuf[2] = (length >> 8) & 0xFF;
    }

    if(SPI_SendReadCmd(Txbuf, pRxbuf, READ_TX_LEN, tranLen ) == SPI_ERROR)
    {
        free(pRxbuf);
        return rcERROR;
    }

    memcpy(&data[0], &pRxbuf[READ_TX_LEN], length);

    checksum = 0;

    for(count = 0; count < length ; count++)
    {
       checksum += pRxbuf[READ_TX_LEN + count];
    }

    if(checksum != pRxbuf[READ_TX_LEN + length])
    {
        //utilDbgMsg_Set(db_DV_VCXO, 1);
        LOG_MSG(db_ALWAYS, "(%s, %d)checksum error reg = 0x%02x, len = %d, checksum[0x%02x], readchecksum[0x%02x]\r\n", __FILE__, __LINE__, address, length, checksum, pRxbuf[READ_TX_LEN + length]);

        free(pRxbuf);
        return rcERROR;
    }

    free(pRxbuf);

    return eResult;
}


static UINT8 i2cWrite(UINT32 address, UINT32 length, const UINT8 *data)
{
    UINT8 eResult = rcERROR;

#if 1
    (void)address;
    (void)length;
    (void)data;
#else
    UINT8	ucRetry = 0;

    do
    {
        eResult = Board_I2C_Master_Write(PROAV_I2C_BUSID, PROAV_I2C_ADDRESS, (UINT16)registerAddress, (UINT16)length, data, PROAV_I2C_FLAGS);
    }
    while((ucRetry++ < PROAV_I2C_RETRY) && (eResult != rcSUCCESS));
#endif /* 0 */

    return eResult;

}

static UINT8 i2cRead(UINT32 address, UINT32 length, UINT8 *data)
{
    UINT8 eResult = rcERROR;
#if 1
    (void)address;
    (void)length;
    (void)data;
#else
    UINT8	ucRetry = 0;

    do
    {
        eResult = Board_I2C_Master_Read(PROAV_I2C_BUSID, PROAV_I2C_ADDRESS, (UINT16)address, length, data, PROAV_I2C_FLAGS);
    }
    while((ucRetry++ < PROAV_I2C_RETRY) && (eResult != rcSUCCESS));
#endif /* 0 */

    return eResult;
}

static UINT8 NA_BufferWrite(void)
{
    return rcSUCCESS;
}

static UINT8 NA_Write(UINT32 address, UINT32 length, const UINT8 *data)
{
    (void)address;
    (void)length;
    (void)data;

    return rcSUCCESS;
}

static UINT8 NA_Read(UINT32 address, UINT32 length, UINT8 *data)
{
    (void)address;
    (void)length;
    (void)data;

    return rcSUCCESS;
}

int dvProAV_InterfaceInit(ePROAV_IF interface)
{
    int status;

    if (interface == eProAVIF_I2C)
    {
        status = dvProAV_AccessInit(eAccessIfMcuSpi, i2cRead, i2cWrite, NA_BufferWrite);
    }
    else if(interface == eProAVIF_SPI)
    {
        Board_I2C_Init();
        Board_SPI_Init();

        status = dvProAV_AccessInit(eAccessIfMcuSpi, spiRead, spiWrite, NA_BufferWrite);
    }
    else
    {

#ifndef SIMULATOR_ISCALER
        Board_I2C_Init();
#endif /* SIMULATOR_ISCALER */

        status = dvProAV_AccessInit(eAccessIfMcuSpi, NA_Read, NA_Write, NA_BufferWrite);
    }

#if 0
#if (SYSTEM_OS_TYPE == POSIX_COMPLIANT)
    if(InterfaceMutexCreate() == -1)
    {
        LOG_MSG(db_ALWAYS, "%s pthread_mutex_init fail [%s]\n", __FUNCTION__ , strerror(errno));
    }
#else
    InterfaceMutexCreate();
#endif
#endif /* 0 */

    LOG_MSG(db_HAL_PROAV,"InterfaceInit %d\n", interface);

    return status;
}

BOOL dvProAV_InterfaceMutexGive(void)
{
    return Board_SSP_SemaphoreGive(eBOARD_SSP_CS_F34);
}

BOOL dvProAV_InterfaceMutexTake(void)
{
    return Board_SSP_SemaphoreTake(eBOARD_SSP_CS_F34);
}

#if 0
    pthread_mutex_init(&(cnt.lock),NULL);

    LOG_MSG(db_ALWAYS,"InterfaceInit %d\n", interface);

    return eResult;
}

BOOL dvProAV_InterfaceMutexGive(void)
{

    return !pthread_mutex_unlock(&(cnt.lock));

}

BOOL dvProAV_InterfaceMutexTake(void)
{
    if(pthread_mutex_lock(&(cnt.lock)) != 0)
    {
        ASSERT_ALWAYS();
        return FALSE;
    }

    return TRUE;
}
#endif /* 0 */

