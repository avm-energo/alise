#include "gpiohelper.h"

#include <errno.h>
#include <fcntl.h>
#include <linux/gpio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/reboot.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

// thanx to https://blog.lxsang.me/post/id/33

void gpio_list(const char *dev_name)
{
    struct gpiochip_info info;
    struct gpioline_info line_info;

    int fd, ret;

    fd = open(dev_name, O_RDONLY);

    if (fd < 0)
    {
        printf("Unabled to open %s: %s", dev_name, strerror(errno));
        return;
    }

    ret = ioctl(fd, GPIO_GET_CHIPINFO_IOCTL, &info);

    if (ret == -1)
    {
        printf("Unable to get chip info from ioctl: %s", strerror(errno));
        close(fd);
        return;
    }

    printf("Chip name: %s\n", info.name);
    printf("Chip label: %s\n", info.label);
    printf("Number of lines: %d\n", info.lines);

    for (int i = 0; i < info.lines; i++)
    {
        line_info.line_offset = i;

        ret = ioctl(fd, GPIO_GET_LINEINFO_IOCTL, &line_info);

        if (ret == -1)
        {
            printf("Unable to get line info from offset %d: %s", i, strerror(errno));
        }
        else
        {
            printf("offset: %d, name: %s, consumer: %s. Flags:\t[%s]\t[%s]\t[%s]\t[%s]\t[%s]\n", i,
                line_info.name,                                                               //
                line_info.consumer,                                                           //
                (line_info.flags & GPIOLINE_FLAG_IS_OUT) ? "OUTPUT" : "INPUT",                //
                (line_info.flags & GPIOLINE_FLAG_ACTIVE_LOW) ? "ACTIVE_LOW" : "ACTIVE_HIGHT", //
                (line_info.flags & GPIOLINE_FLAG_OPEN_DRAIN) ? "OPEN_DRAIN" : "...",          //
                (line_info.flags & GPIOLINE_FLAG_OPEN_SOURCE) ? "OPENSOURCE" : "...",         //
                (line_info.flags & GPIOLINE_FLAG_KERNEL) ? "KERNEL" : "");                    //
        }
    }

    close(fd);
}

uint8_t gpio_read_detail(const char *dev_name, int offset)
{
    struct gpiohandle_request rq;
    struct gpiohandle_data data;

    int fd, ret;

    fd = open(dev_name, O_RDONLY);

    if (fd < 0)
    {
#ifndef DEBUG
        printf("Unabled to open %s: %s", dev_name, strerror(errno));
#endif
        return 0;
    }

    rq.lineoffsets[0] = offset;
    rq.flags = GPIOHANDLE_REQUEST_INPUT;
    rq.lines = 1;

    ret = ioctl(fd, GPIO_GET_LINEHANDLE_IOCTL, &rq);

    close(fd);

    if (ret == -1)
    {
#ifndef DEBUG
        printf("Unable to get line handle from ioctl : %s", strerror(errno));
#endif
        return 0;
    }

    ret = ioctl(rq.fd, GPIOHANDLE_GET_LINE_VALUES_IOCTL, &data);
    close(rq.fd);
    if (ret == -1)
    {
#ifndef DEBUG
        printf("Unable to get line value using ioctl : %s", strerror(errno));
#endif
        return 0;
    }
#ifndef DEBUG
    printf("Value of GPIO at offset %d (INPUT mode) on chip %s: %d\n", offset, dev_name, data.values[0]);
#endif
    return data.values[0];
}

void gpio_write_detail(const char *dev_name, int offset, uint8_t value)

{
    struct gpiohandle_request rq;
    struct gpiohandle_data data;

    int fd, ret;
#ifndef DEBUG
    printf("Write value %d to GPIO at offset %d (OUTPUT mode) on chip %s\n", value, offset, dev_name);
#endif
    fd = open(dev_name, O_RDONLY);

    if (fd < 0)
    {
#ifndef DEBUG
        printf("Unabled to open %s: %s", dev_name, strerror(errno));
#endif
        return;
    }

    rq.lineoffsets[0] = offset;
    rq.flags = GPIOHANDLE_REQUEST_OUTPUT;
    rq.lines = 1;

    ret = ioctl(fd, GPIO_GET_LINEHANDLE_IOCTL, &rq);

    close(fd);

    if (ret == -1)
    {
#ifndef DEBUG
        printf("Unable to line handle from ioctl : %s", strerror(errno));
#endif
        return;
    }

    data.values[0] = value;
    ret = ioctl(rq.fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &data);

    if (ret == -1)
    {
#ifndef DEBUG
        printf("Unable to set line value using ioctl : %s", strerror(errno));
#endif
    }

    close(rq.fd);
}

uint8_t gpio_read(uint8_t dev_name, int offset)
{
    char buffer[15];
    char str[2];
    memcpy(&buffer, gpio_device_prefix, 14);
    sprintf(str, "%d", dev_name);
    buffer[13] = str[0];
    buffer[14] = '\0';
    return gpio_read_detail(buffer, offset);
}

void gpio_write(uint8_t dev_name, int offset, uint8_t value)
{
    char buffer[15];
    char str[2];
    memcpy(&buffer, gpio_device_prefix, 14);
    sprintf(str, "%d", dev_name);
    buffer[13] = str[0];
    buffer[14] = '\0';
    gpio_write_detail(buffer, offset, value);
}
