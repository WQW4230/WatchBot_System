#include "app_led_control.h"
#include "driver/gpio.h"
#include "led_ws2812_driver.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

led_param esp32_led = {0, 0, COLOR_BLACK, LED_MODE_BLACK, 0}; //esp32控制句柄
led_param camera_flash = {0, 0, COLOR_BLACK, LED_MODE_ALARM, 0};//ws2812控制句柄
ws2812_strip_handle_t ws2812_handle = NULL; //ws句柄

const rgb_color_t color_table[COLOR_NUM] = {
    [COLOR_BLACK]   = { 0, 0, 0 },
    [COLOR_RED]     = { 255, 0, 0 },
    [COLOR_GREEN]   = { 0, 255, 0 },
    [COLOR_BLUE]    = { 0, 0, 255 },
    [COLOR_YELLOW]  = { 255, 255, 0 },
    [COLOR_WHITE]   = { 255, 255, 255 },
};

/*
    led驱动初始化
*/
void Led_Blink_Init(void)
{
    gpio_config_t gpio_configStruct;
    gpio_configStruct.mode = GPIO_MODE_OUTPUT;
    gpio_configStruct.pull_down_en = GPIO_PULLDOWN_DISABLE;
    gpio_configStruct.pull_up_en = GPIO_PULLDOWN_DISABLE;
    gpio_configStruct.pin_bit_mask = 1ULL << LED_BLINK_PIN;
    gpio_configStruct.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&gpio_configStruct);
}

void led_Blink_on(void)
{
    gpio_set_level(LED_BLINK_PIN, 1);
}

void led_Blink_off(void)
{
    gpio_set_level(LED_BLINK_PIN, 0);
}

void camera_flash_task(void *Param)
{
    while (1)
    {
        led_param *cfg = (led_param *)Param;
        uint8_t index = 0; //灯珠位置
        rgb_color_t black = color_table[COLOR_BLACK];  //黑/关闭
        rgb_color_t Red = color_table[COLOR_RED];    //红
        rgb_color_t Blue  = color_table[COLOR_BLUE];  //蓝
        rgb_color_t White  = color_table[COLOR_WHITE];  //白
        rgb_color_t colro = color_table[cfg->led_color]; //当前选择的颜色
        switch (cfg->led_mode)
        {
            case LED_MODE_BLACK: //关
                ws2812_write(ws2812_handle, index, black.r, black.g, black.b);
                break;
            case LED_MODE_LIMITEF://常亮
                ws2812_write(ws2812_handle, index, colro.r, colro.g, colro.b);
                break;
            case LED_MODE_LIMITED://亮几次
                cfg->blink_count--;
                ws2812_write(ws2812_handle, index, colro.r, colro.g, colro.b);
                vTaskDelay(pdMS_TO_TICKS(cfg->led_on));
                ws2812_write(ws2812_handle, index, black.r, black.g, black.b);
                vTaskDelay(pdMS_TO_TICKS(cfg->led_off));
                if(cfg->blink_count == 0)
                {
                    cfg->led_mode = LED_MODE_BLACK;
                }
                break;
            case LED_MODE_PATROL://红蓝白巡逻色
                
                // 短闪
                ws2812_write(ws2812_handle, index, Red.r, Red.g, Red.b);
                vTaskDelay(pdMS_TO_TICKS(100));
                ws2812_write(ws2812_handle, index, black.r, black.g, black.b);
                vTaskDelay(pdMS_TO_TICKS(50));

                ws2812_write(ws2812_handle, index, Blue.r, Blue.g, Blue.b);
                vTaskDelay(pdMS_TO_TICKS(100));
                ws2812_write(ws2812_handle, index, black.r, black.g, black.b);
                vTaskDelay(pdMS_TO_TICKS(50));

                ws2812_write(ws2812_handle, index, White.r, White.g, White.b);
                vTaskDelay(pdMS_TO_TICKS(100));
                ws2812_write(ws2812_handle, index, black.r, black.g, black.b);
                vTaskDelay(pdMS_TO_TICKS(200));

                // 中闪
                ws2812_write(ws2812_handle, index, Red.r, Red.g, Red.b);
                vTaskDelay(pdMS_TO_TICKS(150));
                ws2812_write(ws2812_handle, index, black.r, black.g, black.b);
                vTaskDelay(pdMS_TO_TICKS(100));

                ws2812_write(ws2812_handle, index, Blue.r, Blue.g, Blue.b);
                vTaskDelay(pdMS_TO_TICKS(150));
                ws2812_write(ws2812_handle, index, black.r, black.g, black.b);
                vTaskDelay(pdMS_TO_TICKS(100));

                ws2812_write(ws2812_handle, index, White.r, White.g, White.b);
                vTaskDelay(pdMS_TO_TICKS(150));
                ws2812_write(ws2812_handle, index, black.r, black.g, black.b);
                vTaskDelay(pdMS_TO_TICKS(250));

                // 长停
                ws2812_write(ws2812_handle, index, black.r, black.g, black.b);
                vTaskDelay(pdMS_TO_TICKS(500));
                break;

            case LED_MODE_ALARM: //巡逻模式，红蓝交替
                // 短闪
                ws2812_write(ws2812_handle, index, Red.r, Red.g, Red.b);
                vTaskDelay(pdMS_TO_TICKS(150));
                ws2812_write(ws2812_handle, index, Blue.r, Blue.g, Blue.b);
                vTaskDelay(pdMS_TO_TICKS(150));
                break;
            case LED_MODE_BLINK://闪烁
                ws2812_write(ws2812_handle, index, colro.r, colro.g, colro.b);
                vTaskDelay(pdMS_TO_TICKS(cfg->led_on));
                ws2812_write(ws2812_handle, index, black.r, black.g, black.b);
                vTaskDelay(pdMS_TO_TICKS(cfg->led_off));
                break;
            default:
                break;
        }
    }  
}

void esp32_led_task(void *Param)
{
    while (1)
    {
        led_param *cfg = (led_param *)Param;
        switch (cfg->led_mode)
        {
            case LED_MODE_BLACK: //关
                led_Blink_off();
                break;
            case LED_MODE_LIMITEF://常亮
                led_Blink_on();
                break;
            case LED_MODE_LIMITED://亮几次
                cfg->blink_count--;
                led_Blink_on();
                vTaskDelay(pdMS_TO_TICKS(cfg->led_on));
                led_Blink_off();
                vTaskDelay(pdMS_TO_TICKS(cfg->led_off));
                if(cfg->blink_count == 0)
                {
                    cfg->led_mode = LED_MODE_BLACK;
                }
                break;
            case LED_MODE_BLINK://闪烁
                led_Blink_on();
                vTaskDelay(pdMS_TO_TICKS(cfg->led_on));
                led_Blink_off();
                vTaskDelay(pdMS_TO_TICKS(cfg->led_off));
                break;
            default:
                break;
        }
    }
    
}

void camera_flash_set(ws2821_mode_e led_mode, color_name_e colour, uint32_t blink_count, uint32_t bright_time, uint32_t darkness_time)
{
    switch (led_mode)
    {
    case LED_MODE_BLACK:
        camera_flash.led_mode = LED_MODE_BLACK;
        break;
    case LED_MODE_LIMITEF:
        camera_flash.led_mode = LED_MODE_LIMITEF;
         camera_flash.led_color = COLOR_WHITE;
        break;
    case LED_MODE_BLINK:
        camera_flash.led_on = bright_time;
        camera_flash.led_off = darkness_time;
        camera_flash.blink_count = 0;
        camera_flash.led_mode = LED_MODE_BLINK;
        camera_flash.led_color = colour;
        break;
    case LED_MODE_ALARM:
        camera_flash.led_mode = LED_MODE_ALARM;
        break;
    case LED_MODE_PATROL:
        camera_flash.led_mode = LED_MODE_PATROL;
        break;
    case LED_MODE_LIMITED:
        camera_flash.led_on = bright_time;
        camera_flash.led_off = darkness_time;
        camera_flash.blink_count = blink_count;
        camera_flash.led_mode = LED_MODE_LIMITED;
        camera_flash.led_color = colour;
        break;
    default:
        break;
    }
}
void esp32_led_set(ws2821_mode_e led_mode,  uint32_t blink_count, uint32_t bright_time, uint32_t darkness_time)
{
    switch (led_mode)
    {
    case LED_MODE_BLACK:
        esp32_led.led_mode = LED_MODE_BLACK;
        break;
    case LED_MODE_LIMITEF:
        esp32_led.led_mode = LED_MODE_LIMITEF;
        break;
    case LED_MODE_LIMITED:
        esp32_led.led_on = bright_time;
        esp32_led.led_off = darkness_time;
        esp32_led.blink_count = blink_count;
        esp32_led.led_mode = LED_MODE_LIMITED;
        esp32_led.led_color = COLOR_BLACK;
        break;
    case LED_MODE_BLINK:
        esp32_led.led_on = bright_time;
        esp32_led.led_off = darkness_time;
        esp32_led.blink_count = 0;
        esp32_led.led_mode = LED_MODE_BLINK;
        esp32_led.led_color = COLOR_BLACK;
        break;
    default:
        break;
    }
}

/*
    led灯珠控制初始化
*/
void app_led_init(void)
{
    Led_Blink_Init();
    ws2812_init(WS2812_GPIO_NUM,WS2812_LED_NUM,&ws2812_handle);
    //板载led控制句柄初始化
    esp32_led.led_on = 300;
    esp32_led.led_off = 300;
    esp32_led.blink_count = 0;
    esp32_led.led_mode = LED_MODE_BLINK;
    esp32_led.led_color = COLOR_BLACK;
    //ws2812控制句柄初始化
    camera_flash.led_on = 500;
    camera_flash.led_off = 0;
    camera_flash.blink_count = 1;
    camera_flash.led_mode = LED_MODE_PATROL;
    camera_flash.led_color = COLOR_WHITE;
    
    xTaskCreatePinnedToCore(camera_flash_task, "CameraFlash", 2048, &camera_flash, 0, NULL, 0);
    xTaskCreatePinnedToCore(esp32_led_task, "Esp32Led", 2048, &esp32_led, 0, NULL, 0);
}