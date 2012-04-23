#include "system_a4.h"
#include "mouse_memory.h"
#include <unistd.h>
#include <stdio.h>

#define USB_VENDOR_ID_A4          0x09DA
#define USB_PRODUCT_ID_A4_DONGLE  0x054F

#define OSCAR_SUCCESS             0xFB

a4_device *a4_open_device()
{
    a4_device *dv = new a4_device;

    dv->ctx = NULL;
    dv->dev = NULL;

    int r = libusb_init(&dv->ctx);

    if(r < 0)
    {
        delete dv;
        return NULL;
    }

    dv->dev = libusb_open_device_with_vid_pid(dv->ctx,USB_VENDOR_ID_A4,USB_PRODUCT_ID_A4_DONGLE);

    if (!dv->dev)
    {
        libusb_exit(dv->ctx);
        delete dv;
        return NULL;
    }

    return dv;
}

void a4_close_device(a4_device *dev)
{
    if (dev)
    {
        if (dev->dev)
            libusb_close(dev->dev);

        if (dev->ctx)
            libusb_exit(dev->ctx);

        delete dev;
    }
}

int a4_dongle_read(a4_device *dev, unsigned short address, unsigned short pin, void *buffer, int bf_size)
{
    if (dev)
        if (dev->dev)
        {
            int res = libusb_control_transfer(dev->dev, LIBUSB_ENDPOINT_IN, LIBUSB_REQUEST_GET_DESCRIPTOR,
                                              address , pin, (unsigned char *)buffer, bf_size, 1000);

            usleep(10000);

            if (res < 0)
            {
                res = libusb_control_transfer(dev->dev, LIBUSB_ENDPOINT_IN, LIBUSB_REQUEST_GET_DESCRIPTOR,
                                              address , pin, (unsigned char *)buffer, bf_size, 1000);

                usleep(10000);
            }

            if (res > 0)
                return res;
        }

    return A4_ERROR;
}

int a4_dongle_write(a4_device *dev, unsigned short addr_pin, unsigned short word)
{
    if (dev)
        if (dev->dev)
        {
            unsigned char ret[8];

            int res = libusb_control_transfer(dev->dev, LIBUSB_ENDPOINT_IN, LIBUSB_REQUEST_GET_DESCRIPTOR,
                                              addr_pin , word, ret, 1, 1000);

            usleep(10000);

            if (res < 0)
            {
                res = libusb_control_transfer(dev->dev, LIBUSB_ENDPOINT_IN, LIBUSB_REQUEST_GET_DESCRIPTOR,
                                              addr_pin , word, ret, 1, 1000);

                usleep(10000);
            }

            if (res == 1)
                if (ret[0] == OSCAR_SUCCESS)
                    return A4_SUCCESS;
        }

    return A4_ERROR;
}

int a4_dump(a4_device *dev, const char *file)
{
    if (!dev)
        return A4_ERROR;

    FILE *f = fopen(file, "wb");
    if (!f)
        return A4_ERROR;

    unsigned short *buf[0x4000];

    a4_mem_read_block(dev,0,0x4000,buf,NULL);

    fwrite(buf,0x4000,2,f);

    fclose(f);
}
