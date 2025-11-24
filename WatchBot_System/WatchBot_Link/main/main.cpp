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
#include "detection_action_handler.h"
#include "patrolling.h"

extern "C" void app_main(void)
{
    app_camera_lcd_init();
    app_led_init();
    USART_FrameHandler_Init();
    lcd_draw_pictrue(0, 0, 320, 240, gImage_yingwu); 
    vTaskDelay(pdMS_TO_TICKS(3000));  
    app_camera_ai_lcd();
    vTaskDelay(pdMS_TO_TICKS(3000)); 
    face_to_arm_init();
    patrolling_init();

    BuzzerControl(500, 500);//开启蜂鸣器
    vTaskDelay(pdMS_TO_TICKS(3000)); 
    Buzzer_off();//关闭蜂鸣器
}








