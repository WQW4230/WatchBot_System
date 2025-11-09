#ifndef _APP_LED_CONTROL_H_
#define _APP_LED_CONTROL_H_

#include <stdint.h>

#define WS2812_GPIO_NUM 48
#define WS2812_LED_NUM 1

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} rgb_color_t;

typedef enum {
    COLOR_BLACK = 0, //不亮
    COLOR_RED,
    COLOR_GREEN,
    COLOR_BLUE,
    COLOR_YELLOW,
    COLOR_WHITE,
    COLOR_NUM  // 颜色总数
} color_name_e;

typedef enum
{
    LED_MODE_BLACK,  //不亮
    LED_MODE_LIMITEF,//常亮
    LED_MODE_LIMITED, //有限
    LED_MODE_ALARM,   //报警
    LED_MODE_BLINK //闪烁
}ws2821_colour_e;

extern const rgb_color_t color_table[COLOR_NUM];

typedef struct 
{
    uint32_t led_on; 
    uint32_t led_off;
    color_name_e led_color; //颜色
    ws2821_colour_e led_mode; //模式
    uint32_t blink_count;  //如果有限亮几次?
}led_param;

void app_led_init(void);
void camera_flash_Alarm(void *Param);
void camera_flash_set(uint32_t degree, ws2821_colour_e colour, uint32_t bright_time, uint32_t darkness_time);
void esp32_led_set(uint32_t degree,  uint32_t bright_time, uint32_t darkness_time);

#endif
