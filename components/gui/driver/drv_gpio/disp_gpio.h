/**
 * @file disp_gpio.h
 *
 */

#ifndef DISP_GPIO_H
#define DISP_GPIO_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include <stdint.h>

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
enum gpio_mode_t
{
    GPIO_MODE_INTPUT = 0,
    GPIO_MODE_OUTPUT
};

// enum gpio_value_t
// {
//     GPIO_LOW = 0,
//     GPIO_HIGH
// };


/**********************
 * GLOBAL PROTOTYPES
 **********************/
int gpio_pad_select_gpio(uint32_t gpio_num);
int gpio_set_direction(uint32_t gpio_num, gpio_mode_t mode);
int gpio_set_level(uint32_t gpio_num, uint32_t level);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*DISP_GPIO_H*/
