#include "camera_driver.h"   
#include "my_lcd_driver.h"
#include <esp_camera.h>
#include <esp_psram.h>
#include "esp_log.h"

static const char *TAG = "camera_driver";

void bsp_camera_init(void)
{
     // 检测 PSRAM 存在性和大小
    if(esp_psram_get_size() == 0) {
        ESP_LOGE("BOOT", "PSRAM NOT DETECTED! Check hardware connection.");
        vTaskDelay(pdMS_TO_TICKS(1000));
        esp_restart(); // 自动重启
    }
    
    ESP_LOGI("BOOT", "PSRAM Size: %d KB", esp_psram_get_size() / 1024);
    
    // 初始化 PSRAM 缓存
    //esp_psram_init();

    camera_config_t cam_config = {
        .pin_pwdn = CAM_PIN_PWDN,
        .pin_reset = CAM_PIN_RESET,
        .pin_xclk = CAM_PIN_XCLK,
        .pin_sccb_sda = CAM_PIN_SIOD,
        .pin_sccb_scl = CAM_PIN_SIOC,

        .pin_d7 = CAM_PIN_D7,
        .pin_d6 = CAM_PIN_D6,
        .pin_d5 = CAM_PIN_D5,
        .pin_d4 = CAM_PIN_D4,
        .pin_d3 = CAM_PIN_D3,
        .pin_d2 = CAM_PIN_D2,
        .pin_d1 = CAM_PIN_D1,
        .pin_d0 = CAM_PIN_D0,
        .pin_vsync = CAM_PIN_VSYNC,
        .pin_href = CAM_PIN_HREF,
        .pin_pclk = CAM_PIN_PCLK,

        .xclk_freq_hz = 15000000,       // XCLK = 10MHz
        .ledc_timer = LEDC_TIMER_1,
        .ledc_channel = LEDC_CHANNEL_1,
        .pixel_format = PIXFORMAT_RGB565, // RGB565 输出与lcd屏保持一致
        .frame_size = FRAMESIZE_QVGA,   // 320x240
        .jpeg_quality = 10,             // 0-63 越小质量越好
        .fb_count = 2,
        .fb_location = CAMERA_FB_IN_PSRAM,
        .grab_mode = CAMERA_GRAB_WHEN_EMPTY,
        .sccb_i2c_port = 1,
    };

    // 先释放之前的摄像头
    esp_camera_deinit();  

    // 修改配置
    cam_config.pixel_format = PIXFORMAT_RGB565;
    cam_config.frame_size = FRAMESIZE_QVGA;

    // 再初始化
    esp_err_t err = esp_camera_init(&cam_config);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Camera init failed with error 0x%x", err);
    }
    else
    {
        ESP_LOGI(TAG, "Camera init success!");
    }

     sensor_t *s = esp_camera_sensor_get(); // 获取摄像头型号

    if (s->id.PID == OV2640_PID) {
        s->set_hmirror(s, 1);  // 这里控制摄像头镜像 写1镜像 写0不镜像
    }



     //摄像头参数设置
    sensor_t *SB = esp_camera_sensor_get();
    if (SB) 
    {
        SB->set_brightness(SB, 0);     // -2 to 2
        SB->set_contrast(SB, 0);       // -2 to 2
        SB->set_saturation(SB, 0);     // -2 to 2
        SB->set_special_effect(SB, 0); // 0 to 6 (0 - No Effect, 1 - Negative, 2 - Grayscale, 3 - Red Tint, 4 - Green Tint, 5 - Blue Tint, 6 - Sepia)
        SB->set_whitebal(SB, 1);       // 0 = disable , 1 = enable
        SB->set_awb_gain(SB, 1);       // 0 = disable , 1 = enable
        SB->set_wb_mode(SB, 0);        // 0 to 4 - if awb_gain enabled (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)
        SB->set_exposure_ctrl(SB, 1);  // 0 = disable , 1 = enable
        SB->set_aec2(SB, 0);           // 0 = disable , 1 = enable
        SB->set_ae_level(SB, 0);       // -2 to 2
        SB->set_aec_value(SB, 300);    // 0 to 1200
        SB->set_gain_ctrl(SB, 1);      // 0 = disable , 1 = enable
        SB->set_agc_gain(SB, 0);       // 0 to 30
        SB->set_gainceiling(SB, (gainceiling_t)0);  // 0 to 6
        SB->set_bpc(SB, 0);            // 0 = disable , 1 = enable
        SB->set_wpc(SB, 1);            // 0 = disable , 1 = enable
        SB->set_raw_gma(SB, 1);        // 0 = disable , 1 = enable
        SB->set_lenc(SB, 1);           // 0 = disable , 1 = enable
        SB->set_hmirror(SB, 0);        // 0 = disable , 1 = enable
        SB->set_vflip(SB, 0);          // 0 = disable , 1 = enable
        SB->set_dcw(SB, 1);            // 0 = disable , 1 = enable
        SB->set_colorbar(SB, 0);       // 0 = disable , 1 = enable
    }
        
    camera_capture();
}

/**
 * @brief 拍照函数（只捕获帧，不保存）
 */
void camera_capture(void)
{
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb)
    {
        ESP_LOGE(TAG, "Camera capture failed");
        return;
    }

    ESP_LOGI(TAG, "Captured image: %d bytes", fb->len);
    // 这里可以将 fb->buf 发往 WiFi、串口、或者屏幕显示
    // 暂时只打印信息
    // 整帧打印，每行16字节
    // for (size_t i = 0; i < fb->len; i++) {
    //     printf("%02X ", fb->buf[i]);
    // }
    // if (fb->len % 16 != 0) {
    //     printf("\n");
    // }
    // 释放帧缓冲
    esp_camera_fb_return(fb);
}



