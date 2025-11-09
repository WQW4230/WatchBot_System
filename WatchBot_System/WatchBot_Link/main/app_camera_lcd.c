#include "app_camera_lcd.h"
#include "camera_driver.h"   
#include "my_lcd_driver.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include <esp_camera.h>
#include "esp_log.h"

static QueueHandle_t xQueueLCDFrame = NULL;

void app_camera_lcd_init(void)
{
    // bap_camera_capture_init();
    // camera_capture_to_sd();
    bsp_camera_init();
    bsp_lcd_init();
}

// lcd处理任务
static void task_process_lcd(void *arg)
{
    camera_fb_t *frame = NULL;

    while (true)
    {
        if (xQueueReceive(xQueueLCDFrame, &frame, portMAX_DELAY))
        {
            esp_lcd_panel_draw_bitmap(panel_handle, 0, 0, frame->width, frame->height, (uint16_t *)frame->buf);
            esp_camera_fb_return(frame);
        }
    }
}

// 摄像头处理任务
static void task_process_camera(void *arg)
{
    while (true)
    {
        camera_fb_t *frame = esp_camera_fb_get();
        if (frame)
            xQueueSend(xQueueLCDFrame, &frame, portMAX_DELAY);
    }
}

// 让摄像头显示到LCD
void app_camera_lcd(void)
{
    xQueueLCDFrame = xQueueCreate(2, sizeof(camera_fb_t *));
    xTaskCreatePinnedToCore(task_process_camera, "task_process_camera", 3 * 1024, NULL, 15, NULL, 1);
    xTaskCreatePinnedToCore(task_process_lcd, "task_process_lcd", 4 * 1024, NULL, 15, NULL, 0);
}