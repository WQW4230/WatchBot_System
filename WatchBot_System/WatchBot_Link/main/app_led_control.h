#ifndef _APP_LED_CONTROL_H_
#define _APP_LED_CONTROL_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LED_BLINK_PIN 2 //板载LED引脚
#define WS2812_GPIO_NUM 48//灯珠引脚
#define WS2812_LED_NUM 1  //几个灯珠

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
    LED_MODE_BLACK,   //不亮
    LED_MODE_LIMITEF, //常亮
    LED_MODE_LIMITED, //有限
    LED_MODE_ALARM,   //报警
    LED_MODE_PATROL,  //巡逻
    LED_MODE_BLINK    //闪烁
}ws2821_mode_e;

extern const rgb_color_t color_table[COLOR_NUM];

typedef struct 
{
    uint32_t led_on; 
    uint32_t led_off;
    color_name_e led_color; //颜色
    ws2821_mode_e led_mode; //模式
    uint32_t blink_count;  //如果有限亮几次?
}led_param;

extern led_param camera_flash;

void app_led_init(void);
void camera_flash_set(ws2821_mode_e led_mode, color_name_e colour, uint32_t blink_count, uint32_t bright_time, uint32_t darkness_time);
void esp32_led_set(ws2821_mode_e led_mode,  uint32_t blink_count, uint32_t bright_time, uint32_t darkness_time);

#ifdef __cplusplus
}
#endif

#endif
