/**
 * @file disp_spi.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <string.h>
#include "disp_spi.h"
#include "spidev.h"


/*********************
 *      DEFINES
 *********************/
#define TAG "disp_spi"
#define SPI_DEVICE "/dev/spidev0.0"

/**********************
 *      TYPEDEFS
 **********************/
typedef struct spi
{
    uint8_t mode;
    uint8_t bits;
    uint32_t speed;
    uint16_t delay;
} spi_dev_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/
int spidev_init(spi_dev_t *spi_dev);
void drv_spi_send(char *txbuff, size_t len);

/**********************
 *  STATIC VARIABLES
 **********************/
static spi_dev_t spi_dev = {
        .mode = SPI_MODE_0,
        .bits = 8,
        .speed = 18*1000*1000,
        .delay = 0};

static int spi_fd = -1;

/**********************
 *      MACROS
 **********************/


/**********************
 *   GLOBAL FUNCTIONS
 **********************/
int disp_spi_init(void)
{
    int ret = 0;
    spi_fd = spidev_init(&spi_dev);
    if (spi_fd < 0)
    {
        printf("can't open %s \n", SPI_DEVICE);
        return -1;
    }

    printf("spi has open\n");
}

// int main(int argc, char *argv[])
// {
//     int ret = 0;
//     spi_fd = spidev_init(&spi_dev);
//     if (spi_fd < 0)
//     {
//         printf("can't open %s \n", SPI_DEVICE);
//         return -1;
//     }

//     printf("spi has open\n");

//     while (1)
//     {
//         drv_spi_send(SPI_DEVICE, 15);
//         usleep(1000);
//     }

//     close(spi_fd);

//     return ret;
// }

int spidev_init(spi_dev_t *spi_dev)
{
    int ret = 0;

    spi_fd = open(SPI_DEVICE, O_RDWR); /* 打开SPI总线的设备文件            */
    if (spi_fd < 0)
    {
        printf("can't open %s \n", SPI_DEVICE);
        return -1;
    }

    ret = ioctl(spi_fd, SPI_IOC_WR_MODE, &spi_dev->mode);
    if (ret == -1)
    {
        printf("can't set wr spi mode\n");
        return -1;
    }

    ret = ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &spi_dev->bits); /* 设置SPI的数据位            */
    if (ret == -1)
    {
        printf("can't set bits per word\n");
        return -1;
    }

    ret = ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &spi_dev->speed);
    if (ret == -1)
    {
        printf("can't set max speed hz");
        return -1;
    }

    return spi_fd;
}

void disp_spi_transaction(uint8_t *data, size_t length)
{
    int ret;

    struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)data,
        .rx_buf = (unsigned long)NULL,
        .len = length,
        .speed_hz = spi_dev.speed,
        .delay_usecs = spi_dev.delay,
        .bits_per_word = spi_dev.bits,
    };

    ioctl(spi_fd, SPI_IOC_MESSAGE(1), &tr);
}

void disp_spi_send_data(uint8_t *data, size_t length)
{
    //todo: add queue 1
    disp_spi_transaction(data, length);
}

void disp_spi_send_colors(uint8_t *data, size_t length)
{
    //todo: add queue 2
    disp_spi_transaction(data, length);
}
/**********************
 *   STATIC FUNCTIONS
 **********************/



