/*
 * gpio.c
 *
 *  Created on: Apr 16, 2018
 *      Author: bennytseng
 */
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <string.h>
#include <linux/types.h>
#include <linux/stat.h>
#include "gpio.h"

//==============================================================================================
_GpioErr GPIO_ExportPin(uint32 pin)
{
    int fd = -1;
    uint8 str[10];

    fd = open("/sys/class/gpio/export", O_WRONLY);
    if(fd < 0)
    {
        DEBUGP("GPIO %d: can't open export for writing\n", pin);
        return GPIO_ERROR;
    }

    snprintf(str, sizeof(str), "%d", pin);
    write(fd, str, sizeof(str));
    close(fd);
    return GPIO_OK;
}
//==============================================================================================
_GpioErr GPIO_SetDirection(uint32 pin, uint32 dir)
{
    int fd = -1;
    uint8 dir_str[] = "in\0out";
    uint8 pin_str[256];
    uint32 ret;

    snprintf(pin_str, sizeof(pin_str), "/sys/class/gpio/gpio%d/direction", pin);
    fd = open(pin_str, O_WRONLY);
    if(fd < 0)
    {
        DEBUGP("GPIO %d: can't open direction for writing\n", pin);
        return GPIO_ERROR;
    }

    if(write(fd, &dir_str[GPIO_IN == dir ? 0 : 3], GPIO_IN == dir ? 2 : 3) < 0)
    {
        close(fd);
        DEBUGP("GPIO %d: set gpio direction failed\n", pin);
        return GPIO_ERROR;
    }

    close(fd);
    return GPIO_OK;
}
//==============================================================================================
uint32 GPIO_Read(uint32 pin, uint8 *value)
{
#ifndef SIMULATOR_ISCALER
    int fd = -1;
    uint8 pin_str[256];

    snprintf(pin_str, sizeof(pin_str), "/sys/class/gpio/gpio%d/value", pin);
    fd = open(pin_str, O_RDONLY);
    if(fd < 0)
    {
        DEBUGP("GPIO %d: can't open value for reading\n", pin);
        return GPIO_ERROR;
    }

    if(read(fd, value, 1) < 0)
    {
        close(fd);
        DEBUGP("GPIO %d: can't read gpio value failed\n", pin);
        return GPIO_ERROR;
    }
    close(fd);
#endif /* SIMULATOR_ISCALER */
    return GPIO_OK;
}
//==============================================================================================
_GpioErr GPIO_Write(uint32 pin, uint8 value)
{
#ifndef SIMULATOR_ISCALER
    int fd = -1;
    uint8 pin_str[256];
    uint8 val_str[] = "01";

    snprintf(pin_str, sizeof(pin_str), "/sys/class/gpio/gpio%d/value", pin);
    fd = open(pin_str, O_WRONLY);
    if(fd < 0)
    {
        DEBUGP("GPIO %d: can't open value for writing\n", pin);
        return GPIO_ERROR;
    }

    if(write(fd, &val_str[GPIO_LOW == value ? 0 : 1], 1) < 0)
    {
        close(fd);
        DEBUGP("GPIO %d: can't set gpio value failed\n", pin);
        return GPIO_ERROR;
    }
    close(fd);
#endif /* SIMULATOR_ISCALER */
    return GPIO_OK;
}
//==============================================================================================
_GpioErr GPIO_Open(uint32 pin, uint32 dir)
{
#ifndef SIMULATOR_ISCALER
    if(GPIO_ExportPin(pin) != GPIO_OK)
        return GPIO_ERROR;

    if(GPIO_SetDirection(pin, dir) != GPIO_OK)
        return GPIO_ERROR;
#endif /* SIMULATOR_ISCALER */

    return GPIO_OK;
}
//==============================================================================================
