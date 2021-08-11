/**
 * @file disp_spi.h
 *
 */

#ifndef DISP_SPI_H
#define DISP_SPI_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/


/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/



/**********************
 * GLOBAL PROTOTYPES
 **********************/
int disp_spi_init(void);
void disp_spi_send_data(uint8_t *data, size_t length);
void disp_spi_send_colors(uint8_t *data, size_t length);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*DISP_SPI_H*/
