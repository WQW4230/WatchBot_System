#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "app_camera_lcd.h"
#include "my_lcd_driver.h"
#include "my_sd_driver.h"
#include "app_led_control.h"
#include "yingwu.h"
#include "logo_en_240x240_lcd.h"

void app_main(void)
{
    My_SD_Init();
    vTaskDelay(pdMS_TO_TICKS(10000));
    bap_camera_capture_init();
    camera_capture_to_sd();
    //app_camera_lcd_init();
    
    //lcd_draw_pictrue(0, 0, 240, 240, logo_en_240x240_lcd); // 显示乐鑫logo图片
    // lcd_draw_pictrue(0, 0, 320, 240, gImage_yingwu); // 显示3只鹦鹉图片
    // vTaskDelay(3000 / portTICK_PERIOD_MS);  // 延时3000毫秒

    // app_camera_lcd();
    app_led_init();

}







