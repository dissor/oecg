/**
 * @file disp_gpio.cpp
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <string.h>

#include "disp_gpio.h"

#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

/*********************
 *      DEFINES
 *********************/
#define DEV_PATH "/sys/class/gpio/gpio68/value"        // 输入输出电平值设备
#define EXPORT_PATH "/sys/class/gpio/export"           // GPIO设备导出设备
#define DIRECT_PATH "/sys/class/gpio/gpio68/direction" // GPIO输入输出控制设备
#define OUT "out"
#define IN "in"
#define GPIO "68" // GPIO3_4
#define HIGH_LEVEL "1"
#define LOW_LEVEL "0"

/**********************
 *      TYPEDEFS
 **********************/
class disp_gpio
{
public:
    int fd;
    uint32_t num;

public:
    disp_gpio(uint32_t num, int fd);
    ~disp_gpio();
    bool operator==(uint32_t num);
};

disp_gpio::disp_gpio(uint32_t num, int fd = -1)
{
    this->fd = fd;
    this->num = num;
}

disp_gpio::~disp_gpio()
{
}

bool disp_gpio::operator==(uint32_t num)
{
    if (this->num == num)
    {
        return true;
    }
    return false;
}

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/
vector<disp_gpio> gv_gpio; //无参构造

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void printVector(vector<disp_gpio> &v)
{
    for (vector<disp_gpio>::iterator it = v.begin(); it != v.end(); it++)
    {
        cout << "fd: " << (*it).fd << "\t"
             << "num: " << (*it).num << endl;
    }
    cout << endl;
}

int gpio_pad_select_gpio(uint32_t gpio_num)
{
    char buf[10] = {0};
    int ret = 0;
    disp_gpio gpio(gpio_num); //fd 缺省 -1

    cout << "open gpio" << endl;
    gpio.fd = open(EXPORT_PATH, O_WRONLY); // 打开GPIO设备导出设备
    if (gpio.fd < 0)
    {
        perror("open export:");
        return -1;
    }

    memset(buf, 0, sizeof(buf));
    ret = sprintf(buf, "%d", gpio.num);
    if (ret < 1)
    {
        cout << "buf: " << buf << "\t"
             << "ret: " << ret << endl;
    }

    write(gpio.fd, buf, ret); // 打开GPIO设备导出设备

    close(gpio.fd);

    gv_gpio.push_back(gpio);
    // printVector(gv_gpio);

    return 0;
}

int gpio_set_direction(uint32_t gpio_num, gpio_mode_t mode)
{
    char buf[128] = {0};
    int ret = 0;
    vector<disp_gpio>::iterator it = find(gv_gpio.begin(), gv_gpio.end(), gpio_num);
    if (it == gv_gpio.end())
    {
        cout << "not found!" << endl;
        return -1;
    }

    memset(buf, 0, sizeof(buf));
    ret = sprintf(buf, "/sys/class/gpio/gpio%d/direction", it->num);
    if (ret < 1)
    {
        cout << "buf: " << buf << "\t"
             << "ret: " << ret << endl;
        return -1;
    }

    it->fd = open(buf, O_RDWR); // 打开GPIO输入输出控制设备
    if (it->fd < 0)
    {
        perror("open direction:");
        return -1;
    }
    cout << "set gpio " << buf;

    memset(buf, 0, sizeof(buf));
    if (mode == GPIO_MODE_INTPUT)
    {
        ret = sprintf(buf, "%s", IN);
    }
    else
    {
        ret = sprintf(buf, "%s", OUT);
    }

    if (ret < 1)
    {
        cout << "buf: " << buf << "\t"
             << "ret: " << ret << endl;
        return -1;
    }

    cout << " " << buf << endl;

    ret = write(it->fd, buf, ret);
    if (ret < 0)
    {
        perror("write direction:");
        close(it->fd);
        return -1;
    }

    close(it->fd);
    return 0;
}

int gpio_set_level(uint32_t gpio_num, uint32_t level)
{
    char buf[128] = {0};
    int ret = 0;
    vector<disp_gpio>::iterator it = find(gv_gpio.begin(), gv_gpio.end(), gpio_num);
    if (it == gv_gpio.end())
    {
        cout << "not found!" << endl;
        return -1;
    }
    else

        memset(buf, 0, sizeof(buf));
    ret = sprintf(buf, "/sys/class/gpio/gpio%d/value", it->num);
    if (ret < 1)
    {
        return -1;
    }

    cout << "set gpio " << it->num;
    it->fd = open(buf, O_RDWR); // 打开GPIO输入输出控制设备
    if (it->fd < 0)
    {
        perror("open value:");
        return -1;
    }

    memset(buf, 0, sizeof(buf));
    if (level == 0)
    {
        ret = sprintf(buf, "%s", LOW_LEVEL);
    }
    else
    {
        ret = sprintf(buf, "%s", HIGH_LEVEL);
    }
    cout << " " << buf << endl;

    ret = write(it->fd, buf, strlen(IN));
    if (ret < 0)
    {
        perror("write value:");
        close(it->fd);
        return -1;
    }

    close(it->fd);
    return 0;
}

// int main(int argc, char **argv)
// {
//     int cnt = 0;
//     gpio_pad_select_gpio(4);
//     gpio_set_direction(4, GPIO_MODE_OUTPUT);
//     while (1)
//     {
//         gpio_set_level(4, (gpio_value_t)(cnt++ % 2));
//         sleep(1);
//     }

//     return 0;
// }
