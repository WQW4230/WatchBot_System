#pragma once

#include <string.h>
#include "math.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_check.h"
#include "driver/i2c.h"
#include "driver/spi_master.h"
#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_lcd_types.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"

/***********************************************************/
/****************    LCD显示屏 ↓   *************************/
#define BSP_LCD_PIXEL_CLOCK_HZ     (20 * 1000 * 1000)
#define BSP_LCD_SPI_NUM            (SPI2_HOST)
#define LCD_CMD_BITS               (8)
#define LCD_PARAM_BITS             (8)
#define BSP_LCD_BITS_PER_PIXEL     (16)
#define LCD_LEDC_CH          LEDC_CHANNEL_0

#define BSP_LCD_H_RES              (320)
#define BSP_LCD_V_RES              (240)

#define BSP_LCD_BACKLIGHT     (GPIO_NUM_41)
#define BSP_LCD_SPI_CLK       (GPIO_NUM_40)
#define BSP_LCD_SPI_MOSI      (GPIO_NUM_39)
#define BSP_LCD_DC            (GPIO_NUM_38)
#define BSP_LCD_RST           (GPIO_NUM_47)
#define BSP_LCD_SPI_CS        (GPIO_NUM_14)

esp_err_t bsp_lcd_init(void);
void lcd_set_color(uint16_t color);
void lcd_draw_pictrue(int x_start, int y_start, int x_end, int y_end, const unsigned char *gImage);
/***************    LCD显示屏 ↑   *************************/
/***********************************************************/

