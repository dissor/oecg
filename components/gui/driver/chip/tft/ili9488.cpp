/**
 * @file ili9488.c
 */

/*********************
 *      INCLUDES
 *********************/
#include <unistd.h>
#include <assert.h>

#include "ili9488.h"

#include "drv_spi/disp_spi.h"
#include "drv_gpio/disp_gpio.h"

#include <iostream>
using namespace std;

/*********************
 *      DEFINES
 *********************/
#define TAG "ILI9488"

/**********************
 *      TYPEDEFS
 **********************/

/*The LCD needs a bunch of command/argument values to be initialized. They are stored in this struct. */
typedef struct
{
	uint8_t cmd;
	uint8_t data[16];
	uint8_t databytes; //No of data in data; bit 7 = delay after set; 0xFF = end of cmds.
} lcd_init_cmd_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void ili9488_set_orientation(uint8_t orientation);

static void ili9488_send_cmd(uint8_t cmd);
static void ili9488_send_data(void *data, uint16_t length);
static void ili9488_send_color(void *data, uint32_t length);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
// From github.com/jeremyjh/ESP32_TFT_library
// From github.com/mvturnho/ILI9488-lvgl-ESP32-WROVER-B
void ili9488_init(void)
{
	lcd_init_cmd_t ili_init_cmds[] = {
		{ILI9488_CMD_SLEEP_OUT, {0x00}, 0x80},
		{ILI9488_CMD_POSITIVE_GAMMA_CORRECTION, {0x00, 0x03, 0x09, 0x08, 0x16, 0x0A, 0x3F, 0x78, 0x4C, 0x09, 0x0A, 0x08, 0x16, 0x1A, 0x0F}, 15},
		{ILI9488_CMD_NEGATIVE_GAMMA_CORRECTION, {0x00, 0x16, 0x19, 0x03, 0x0F, 0x05, 0x32, 0x45, 0x46, 0x04, 0x0E, 0x0D, 0x35, 0x37, 0x0F}, 15},
		{ILI9488_CMD_POWER_CONTROL_1, {0x17, 0x15}, 2},
		{ILI9488_CMD_POWER_CONTROL_2, {0x41}, 1},
		{ILI9488_CMD_VCOM_CONTROL_1, {0x00, 0x12, 0x80}, 3},
		{ILI9488_CMD_MEMORY_ACCESS_CONTROL, {(0x20 | 0x08)}, 1},
		{ILI9488_CMD_COLMOD_PIXEL_FORMAT_SET, {0x66}, 1},
		{ILI9488_CMD_INTERFACE_MODE_CONTROL, {0x00}, 1},
		{ILI9488_CMD_FRAME_RATE_CONTROL_NORMAL, {0xA0}, 1},
		{ILI9488_CMD_DISPLAY_INVERSION_CONTROL, {0x02}, 1},
		{ILI9488_CMD_DISPLAY_FUNCTION_CONTROL, {0x02, 0x02}, 2},
		{ILI9488_CMD_SET_IMAGE_FUNCTION, {0x00}, 1},
		{ILI9488_CMD_WRITE_CTRL_DISPLAY, {0x28}, 1},
		{ILI9488_CMD_WRITE_DISPLAY_BRIGHTNESS, {0x7F}, 1},
		{ILI9488_CMD_ADJUST_CONTROL_3, {0xA9, 0x51, 0x2C, 0x02}, 4},
		{ILI9488_CMD_DISPLAY_ON, {0x00}, 0x80},
		{0, {0}, 0xff},
	};

	//Initialize non-SPI GPIOs
	gpio_pad_select_gpio(ILI9488_DC);
	gpio_set_direction(ILI9488_DC, GPIO_MODE_OUTPUT);

#if ILI9488_USE_RST
	gpio_pad_select_gpio(ILI9488_RST);
	gpio_set_direction(ILI9488_RST, GPIO_MODE_OUTPUT);
#endif

#if ILI9488_ENABLE_BACKLIGHT_CONTROL
	gpio_pad_select_gpio(ILI9488_BCKL);
	gpio_set_direction(ILI9488_BCKL, GPIO_MODE_OUTPUT);
#endif

#if ILI9488_USE_RST
	//Reset the display
	gpio_set_level(ILI9488_RST, 0);
	usleep(100 * 1000);
	gpio_set_level(ILI9488_RST, 1);
	usleep(100 * 1000);
#endif

	cout << "ILI9488 initialization." << endl;

	// Exit sleep
	ili9488_send_cmd(0x01); /* Software reset */
	usleep(100 * 1000);

	//Send all the commands
	uint16_t cmd = 0;
	while (ili_init_cmds[cmd].databytes != 0xff)
	{
		ili9488_send_cmd(ili_init_cmds[cmd].cmd);
		ili9488_send_data(ili_init_cmds[cmd].data, ili_init_cmds[cmd].databytes & 0x1F);
		if (ili_init_cmds[cmd].databytes & 0x80)
		{
			usleep(100 * 1000);
		}
		cmd++;
	}

	ili9488_enable_backlight(true);

	ili9488_set_orientation(CONFIG_LV_DISPLAY_ORIENTATION);
}

// Flush function based on mvturnho repo
void ili9488_flush(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map)
{
	uint32_t size = lv_area_get_width(area) * lv_area_get_height(area);

	cout << "ili9488_flush size: "<<size<<endl;

	lv_color16_t *buffer_16bit = (lv_color16_t *)color_map;
	uint8_t *mybuf;
	do
	{
		mybuf = (uint8_t *)malloc(3 * size * sizeof(uint8_t));
		if (mybuf == NULL)
			cout<< "Could not allocate enough memory!" <<endl;
	} while (mybuf == NULL);

	uint32_t LD = 0;
	uint32_t j = 0;

	for (uint32_t i = 0; i < size; i++)
	{
		LD = buffer_16bit[i].full;
		mybuf[j] = (uint8_t)(((LD & 0xF800) >> 8) | ((LD & 0x8000) >> 13));
		j++;
		mybuf[j] = (uint8_t)((LD & 0x07E0) >> 3);
		j++;
		mybuf[j] = (uint8_t)(((LD & 0x001F) << 3) | ((LD & 0x0010) >> 2));
		j++;
	}

	/* Column addresses  */
	uint8_t xb[] = {
		(uint8_t)(area->x1 >> 8) & 0xFF,
		(uint8_t)(area->x1) & 0xFF,
		(uint8_t)(area->x2 >> 8) & 0xFF,
		(uint8_t)(area->x2) & 0xFF,
	};

	/* Page addresses  */
	uint8_t yb[] = {
		(uint8_t)(area->y1 >> 8) & 0xFF,
		(uint8_t)(area->y1) & 0xFF,
		(uint8_t)(area->y2 >> 8) & 0xFF,
		(uint8_t)(area->y2) & 0xFF,
	};

	/*Column addresses*/
	ili9488_send_cmd(ILI9488_CMD_COLUMN_ADDRESS_SET);
	ili9488_send_data(xb, 4);

	/*Page addresses*/
	ili9488_send_cmd(ILI9488_CMD_PAGE_ADDRESS_SET);
	ili9488_send_data(yb, 4);

	/*Memory write*/
	ili9488_send_cmd(ILI9488_CMD_MEMORY_WRITE);

	cout << "ili9488_send_color" << endl;
	ili9488_send_color((void *)mybuf, size * 3);
	free(mybuf);
}

void test_set_px(uint16_t x1, uint16_t x2, uint16_t y1, uint16_t y2, uint16_t color){
	/* Column addresses  */
	uint8_t xb[] = {
		(uint8_t)(x1 >> 8) & 0xFF,
		(uint8_t)(x1) & 0xFF,
		(uint8_t)(x2 >> 8) & 0xFF,
		(uint8_t)(x2) & 0xFF,
	};

	/* Page addresses  */
	uint8_t yb[] = {
		(uint8_t)(y1 >> 8) & 0xFF,
		(uint8_t)(y1) & 0xFF,
		(uint8_t)(y2 >> 8) & 0xFF,
		(uint8_t)(y2) & 0xFF,
	};

	/*Column addresses*/
	ili9488_send_cmd(ILI9488_CMD_COLUMN_ADDRESS_SET);
	ili9488_send_data(xb, 4);

	/*Page addresses*/
	ili9488_send_cmd(ILI9488_CMD_PAGE_ADDRESS_SET);
	ili9488_send_data(yb, 4);

	/*Memory write*/
	ili9488_send_cmd(ILI9488_CMD_MEMORY_WRITE);

	uint8_t mybuf[3] = {0};
	int j = 0;
	uint32_t size = (x2-x1)*(y2-y1);
	cout << "size: " << size << endl;

	for (uint32_t i = 0; i < size; i++)
	{
		mybuf[j] = (uint8_t)(((color & 0xF800) >> 8) | ((color & 0x8000) >> 13));
		j++;
		mybuf[j] = (uint8_t)((color & 0x07E0) >> 3);
		j++;
		mybuf[j] = (uint8_t)(((color & 0x001F) << 3) | ((color & 0x0010) >> 2));
		j++;
	}

	for(int i=0; i<size; i++){
		ili9488_send_color((void *)mybuf, 3);
	}
	
}

void ili9488_enable_backlight(bool backlight)
{
#if ILI9488_ENABLE_BACKLIGHT_CONTROL
	// ESP_LOGI(TAG, "%s backlight.", backlight ? "Enabling" : "Disabling");
	uint32_t tmp = 0;

#if (ILI9488_BCKL_ACTIVE_LVL == 1)
	tmp = backlight ? 1 : 0;
#else
	tmp = backlight ? 0 : 1;
#endif

	gpio_set_level(ILI9488_BCKL, tmp);
#endif
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
void disp_wait_for_pending_transactions(void)
{
	// spi_transaction_t *presult;

	// while(uxQueueMessagesWaiting(TransactionPool) < SPI_TRANSACTION_POOL_SIZE) {	/* service until the transaction reuse pool is full again */
	//     if (spi_device_get_trans_result(spi, &presult, 1) == ESP_OK) {
	// 		xQueueSend(TransactionPool, &presult, portMAX_DELAY);
	//     }
	// }
}

static void ili9488_send_cmd(uint8_t cmd)
{
	disp_wait_for_pending_transactions();
	gpio_set_level(ILI9488_DC, 0); /*Command mode*/
	disp_spi_send_data(&cmd, 1);
}

static void ili9488_send_data(void *data, uint16_t length)
{
	disp_wait_for_pending_transactions();
	gpio_set_level(ILI9488_DC, 1); /*Data mode*/
	disp_spi_send_data((uint8_t *)data, length);
}

static void ili9488_send_color(void *data, uint32_t length)
{
	disp_wait_for_pending_transactions();
	gpio_set_level(ILI9488_DC, 1); /*Data mode*/
	disp_spi_send_colors((uint8_t *)data, length);
}

static void ili9488_set_orientation(uint8_t orientation)
{
	assert(orientation < 4);

	const char *orientation_str[] = {
		"PORTRAIT", "PORTRAIT_INVERTED", "LANDSCAPE", "LANDSCAPE_INVERTED"};

	cout << "Display orientation: " << orientation_str[orientation] << endl;

	uint8_t data[] = {0x48, 0x88, 0x28, 0xE8};

	cout << "0x36 command value: " << data[orientation] << endl;

	ili9488_send_cmd(0x36);
	ili9488_send_data((void *)&data[orientation], 1);
}
