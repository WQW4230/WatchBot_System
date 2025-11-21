#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "app_camera_lcd.h"
#include "my_lcd_driver.h"
#include "app_led_control.h"
#include "who_human_face_detection.hpp"
#include "yingwu.h"
#include "stm32_link.h"
#include "USART_FrameHandler.h"

extern "C" void app_main(void)
{
    app_camera_lcd_init();
    app_led_init();
    // USART_FrameHandler_Init();
    lcd_draw_pictrue(0, 0, 320, 240, gImage_yingwu); // 显示3只鹦鹉图片
    vTaskDelay(pdMS_TO_TICKS(3000));  // 延时3000毫秒
    app_camera_ai_lcd();
    vTaskDelay(pdMS_TO_TICKS(3000));  // 延时3000毫秒
    arm_control(-90, -90, -90, 0);
}








