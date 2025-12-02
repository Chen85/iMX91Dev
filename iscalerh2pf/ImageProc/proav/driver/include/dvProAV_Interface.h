#ifndef _DV_PROAV_INTERFACE_H_
#define _DV_PROAV_INTERFACE_H_

typedef enum
{
    eProAVIF_I2C,
    eProAVIF_SPI,
    eProAVIF_NA, //Not doing
    eProAVIF_MAX
}ePROAV_IF;

BOOL dvProAV_InterfaceWriteBufferIsFull(void);

int dvProAV_InterfaceWriteToBuffer(UINT32 address, UINT32 length, const UINT8 *data);

int dvProAV_InterfaceInit(ePROAV_IF interface);

BOOL dvProAV_InterfaceMutexGive(void);

BOOL dvProAV_InterfaceMutexTake(void);


#endif // _DV_PROAV_INTERFACE_H_

