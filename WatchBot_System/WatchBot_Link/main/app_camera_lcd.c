#include "app_camera_lcd.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "camera_driver.h"   
#include "my_lcd_driver.h"
#include <esp_camera.h>
#include "my_sd_driver.h"
#include "esp_log.h"
#include "who_human_face_detection.hpp"
#include "app_led_control.h"

// static QueueHandle_t xQueueLCDFrame = NULL;

/*
    摄像头_lcd_sd初始化
*/
void app_camera_lcd_init(void)
{
    // My_SD_Init();
    bsp_camera_init();
    bsp_lcd_init();
}

/*
    拍一张1600x1200JPEG照片至存储卡
*/
void app_capture(void)
{
    camera_flash_set(LED_MODE_LIMITED, COLOR_WHITE, 1, 1000, 300);

    bap_camera_capture_init();//切换jepg
     vTaskDelay(pdMS_TO_TICKS(100));  // 延时100毫秒
    camera_fb_t *frame = esp_camera_fb_get();
    camera_capture_to_sd(frame);
    bsp_camera_init();//切回 RGB565

    vTaskDelay(pdMS_TO_TICKS(100));  // 延时200毫秒

    camera_flash_set(LED_MODE_LIMITED, COLOR_WHITE, 1, 100, 1);
}

// // lcd处理任务
// static void task_process_lcd(void *arg)
// {
//     camera_fb_t *frame = NULL;

//     while (true)
//     {
//         if (xQueueReceive(xQueueLCDFrame, &frame, portMAX_DELAY))
//         {
//             esp_lcd_panel_draw_bitmap(panel_handle, 0, 0, frame->width, frame->height, (uint16_t *)frame->buf);
//             esp_camera_fb_return(frame);
//         }
//     }
// }

// // 摄像头处理任务
// static void task_process_camera(void *arg)
// {
//     while (true)
//     {
//         camera_fb_t *frame = esp_camera_fb_get();
//         if (frame)
//             xQueueSend(xQueueLCDFrame, &frame, portMAX_DELAY);
//     }
// }

// //让摄像头显示到LCD
// void app_camera_lcd(void)
// {
//     xQueueLCDFrame = xQueueCreate(2, sizeof(camera_fb_t *));
//     xTaskCreatePinnedToCore(task_process_camera, "task_process_camera", 3 * 1024, NULL, 15, NULL, 1);
//     xTaskCreatePinnedToCore(task_process_lcd, "task_process_lcd", 4 * 1024, NULL, 15, NULL, 0);
// }

// 液晶屏显示内容 方便外面文件调用
void lcd_draw_bitmap(int x_start, int y_start, int x_end, int y_end, const void *color_data)
{
    esp_lcd_panel_draw_bitmap(panel_handle, x_start, y_start, x_end, y_end, color_data);
}