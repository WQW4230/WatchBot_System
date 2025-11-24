#include "who_human_face_detection.hpp"
#include "esp_log.h"
#include "esp_camera.h"
#include "dl_image.hpp"
#include "human_face_detect_msr01.hpp"
#include "human_face_detect_mnp01.hpp"
#include "who_ai_utils.hpp"
#include "app_camera_lcd.h"
#include "my_lcd_driver.h"
#include "my_sd_driver.h"
#include "camera_driver.h" 
#include "app_led_control.h"
#include "face_calculation.h"

static const char *TAG = "human_face_detection";

SemaphoreHandle_t cmaera_sem;

static QueueHandle_t xQueueLCDFrame = NULL; 
static QueueHandle_t xQueueAIFrame = NULL;
extern QueueHandle_t xQueueFaceInfo; //通知人脸姿态解算
QueueHandle_t xQueuePatrllin = NULL; //无人脸信息10s去巡逻

QueueHandle_t xQueueCaptureNotify = NULL;// 拍照通知队列，只存储一个简单信号


static bool gEvent = true;
static bool gReturnFB = true;

#define LCD_W 320
#define LCD_H 240

//图片大小裁剪
void crop_and_resize(uint16_t *src, int src_w, int src_h,
                     int crop_x, int crop_y, int crop_w, int crop_h,
                     uint16_t *dst, int dst_w, int dst_h)
{
    for(int y=0; y<dst_h; y++){
        for(int x=0; x<dst_w; x++){
            int src_x = crop_x + x * crop_w / dst_w;
            int src_y = crop_y + y * crop_h / dst_h;
            dst[y*dst_w + x] = src[src_y*src_w + src_x];
        }
    }
}

// 等待队列清空
static void wait_queue_empty(QueueHandle_t q)
{
    camera_fb_t *frame = NULL;
    while(uxQueueMessagesWaiting(q) > 0) {
        if(xQueueReceive(q, &frame, pdMS_TO_TICKS(100))) {
            if(frame) esp_camera_fb_return(frame);
        }
    }
}

/*
    将当前人脸坐标发送至队列
*/
void send_first_face_info(std::list<dl::detect::result_t> &results)
{
    if (results.empty()) return;

    face_info_t face_info;

    auto &first_face = results.front();

    face_info.x1 = first_face.box[0];
    face_info.y1 = first_face.box[1];
    face_info.x2 = first_face.box[2];
    face_info.y2 = first_face.box[3];

    if (first_face.keypoint.size() == 10)
    {
        face_info.left_eye_x  = first_face.keypoint[0];
        face_info.left_eye_y  = first_face.keypoint[1];
        face_info.left_mouth_x  = first_face.keypoint[2];
        face_info.left_mouth_y  = first_face.keypoint[3];
        face_info.nose_x      = first_face.keypoint[4];
        face_info.nose_y      = first_face.keypoint[5];
        face_info.right_eye_x = first_face.keypoint[6];
        face_info.right_eye_y = first_face.keypoint[7];
        face_info.right_mouth_x = first_face.keypoint[8];
        face_info.right_mouth_y = first_face.keypoint[9];
    }

    if(xQueueFaceInfo)
    {
        xQueueSend(xQueueFaceInfo, &face_info, portMAX_DELAY);
    }
}

// AI处理任务
static void task_process_ai(void *arg)
{
    camera_fb_t *frame = NULL;
    HumanFaceDetectMSR01 detector(0.3F, 0.3F, 10, 0.3F);
    HumanFaceDetectMNP01 detector2(0.4F, 0.3F, 10);

    while (true)
    {
        if (gEvent)
        {
            if (xQueueReceive(xQueueAIFrame, &frame, portMAX_DELAY))//等待10,000ms
            {
                std::list<dl::detect::result_t> &detect_candidates = detector.infer((uint16_t *)frame->buf, {(int)frame->height, (int)frame->width, 3});
                std::list<dl::detect::result_t> &detect_results = detector2.infer((uint16_t *)frame->buf, {(int)frame->height, (int)frame->width, 3}, detect_candidates);

                //检测到人脸时
                if (detect_results.size() > 0)
                {
                    draw_detection_result((uint16_t *)frame->buf, frame->height, frame->width, detect_results);
                    print_detection_result(detect_results);
                    send_first_face_info(detect_results);

                    uint8_t temp;
                    xQueueOverwrite(xQueuePatrllin, &temp);
                }

            }

            if (xQueueLCDFrame)
            {
                xQueueSend(xQueueLCDFrame, &frame, portMAX_DELAY);
            }
            else if (gReturnFB)
            {
                esp_camera_fb_return(frame);
            }
            else
            {
                free(frame);
            }
        }
    }
}


// lcd处理任务
static void task_process_lcd(void *arg)
{
    camera_fb_t *frame = NULL;

    // 在PSRAM分配LCD显示buffer
    uint16_t *lcd_buf = (uint16_t *)heap_caps_malloc(LCD_W*LCD_H*2, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if(!lcd_buf) {
        ESP_LOGE(TAG, "LCD buffer malloc failed");
        vTaskDelete(NULL);
        return;
    }

    while (true)
    {
        if (xQueueReceive(xQueueLCDFrame, &frame, portMAX_DELAY))
        {
            // 裁剪中心区域并缩放
            int crop_w = frame->height * LCD_W / LCD_H;  // 保持比例
            int crop_x = (frame->width - crop_w) / 2;
            int crop_y = 0;

            crop_and_resize((uint16_t *)frame->buf, frame->width, frame->height,
                            crop_x, crop_y, crop_w, frame->height,
                            lcd_buf, LCD_W, LCD_H);

            // 显示到LCD
            esp_lcd_panel_draw_bitmap(panel_handle, 0, 0, LCD_W, LCD_H, lcd_buf);
            esp_camera_fb_return(frame);
        }
    }

    heap_caps_free(lcd_buf);
}

// 摄像头处理任务
static void task_process_camera(void *arg)
{
    while (true)
    {
        xSemaphoreTake(cmaera_sem, portMAX_DELAY);
        camera_fb_t *frame = esp_camera_fb_get();
        if (frame)
        {
            xQueueSend(xQueueAIFrame, &frame, portMAX_DELAY);
        }
        xSemaphoreGive(cmaera_sem);
        vTaskDelay(pdMS_TO_TICKS(50));  // 延时50ms 20帧
    }
}

/*
    拍照处理任务
    拍一张照片最快需要700ms
    停止摄像头任务初始化为1600*1200分辨率
    延迟<300左右延迟等ai和lcd处理完腾出psram空间和dma
    拍完照等待500ms等待dma写入sd卡再重启摄像头任务
*/
static void task_process_picture(void *arg)
{
    while (true)
    {
        uint8_t notify;
        if(xQueueReceive(xQueueCaptureNotify, &notify, portMAX_DELAY) == pdTRUE)
        {
            ESP_LOGI(TAG, "111111111111111111111.");
            if(pdTRUE == xSemaphoreTake(cmaera_sem, portMAX_DELAY))//上锁
            {   
                ESP_LOGI(TAG, "Waiting for capture notification22222222222...");
                //等待帧/队列释放清空
                wait_queue_empty(xQueueAIFrame);
                wait_queue_empty(xQueueLCDFrame);

                camera_flash_set(LED_MODE_LIMITED, COLOR_WHITE, 1, 1000, 300);

                bap_camera_capture_init();//切换jepg
                vTaskDelay(pdMS_TO_TICKS(200));  // 延时100毫秒
                camera_fb_t *frame = esp_camera_fb_get();
                camera_capture_to_sd(frame);
                bsp_camera_init();//切回 RGB565

                camera_flash_set(LED_MODE_LIMITED, COLOR_WHITE, 1, 100, 1);
                vTaskDelay(pdMS_TO_TICKS(500));  // 延时1000毫秒
                
                if (frame)
                {
                    xQueueSend(xQueueAIFrame, &frame, portMAX_DELAY);
                }
                xSemaphoreGive(cmaera_sem);
            }
        }
        
    }
    
}

//人脸检测并且输出到LCD
//创建摄像头任务、ai任务、lcd任务
void app_camera_ai_lcd(void)
{

    xQueueLCDFrame = xQueueCreate(2, sizeof(camera_fb_t *));
    xQueueAIFrame = xQueueCreate(2, sizeof(camera_fb_t *));
    xQueueCaptureNotify = xQueueCreate(1, sizeof(uint8_t));
    xQueuePatrllin = xQueueCreate(1, sizeof(uint8_t));

    if (!xQueueCaptureNotify) ESP_LOGE(TAG, "Capture queue creation failed!");
    cmaera_sem = xSemaphoreCreateMutex();

    xTaskCreatePinnedToCore(task_process_camera, "task_process_camera", 3 * 1024, NULL, 5, NULL, 1);
    xTaskCreatePinnedToCore(task_process_lcd, "task_process_lcd", 4 * 1024, NULL, 5, NULL, 0);
    xTaskCreatePinnedToCore(task_process_ai, "task_process_ai", 4 * 1024, NULL, 5, NULL, 0);
    xTaskCreatePinnedToCore(task_process_picture, "task_process_picture", 4 * 1024, NULL, 10, NULL, 0);
}
