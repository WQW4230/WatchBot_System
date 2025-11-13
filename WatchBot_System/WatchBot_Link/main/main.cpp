#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "app_camera_lcd.h"
#include "my_lcd_driver.h"
#include "my_sd_driver.h"
#include "app_led_control.h"
#include "who_human_face_detection.hpp"
#include "yingwu.h"
#include "logo_en_240x240_lcd.h"
#include "stm32_link.h"

extern "C" void app_main(void)
{

    // My_SD_Init();
    // bap_camera_capture_init();
    // camera_capture_to_sd();
    app_camera_lcd_init();
    
    lcd_draw_pictrue(0, 0, 320, 240, gImage_yingwu); // 显示3只鹦鹉图片
    vTaskDelay(pdMS_TO_TICKS(3000));  // 延时3000毫秒
    //app_camera_lcd();
    app_camera_ai_lcd();
    app_led_init();
    stm32_link_init();
}








