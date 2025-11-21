#include "my_sd_driver.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/unistd.h>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/sdmmc_host.h"
#include "camera_driver.h"   
#include <esp_psram.h>

static const char *TAG = "My_SD_Driver";
extern void off_camera_ai_lcd(void); //停止摄像头采集任务
extern void on_camera_ai_lcd(void);  //开始摄像头采集任务

// 写文件内容 path是路径 data是内容
static esp_err_t s_example_write_file(const char *path, char *data)
{
    ESP_LOGI(TAG, "Opening file %s", path);
    FILE *f = fopen(path, "w");   // 以只写方式打开路径中文件
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for writing"); 
        return ESP_FAIL;
    }
    fprintf(f, data); // 写入内容
    fclose(f);  // 关闭文件
    ESP_LOGI(TAG, "File written");

    return ESP_OK;
}

// 读文件内容 path是路径
static esp_err_t s_example_read_file(const char *path)
{
    ESP_LOGI(TAG, "Reading file %s", path);
    FILE *f = fopen(path, "r");  // 以只读方式打开文件
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return ESP_FAIL;
    }
    char line[EXAMPLE_MAX_CHAR_SIZE];  // 定义一个字符串数组
    fgets(line, sizeof(line), f); // 获取文件中的内容到字符串数组
    fclose(f); // 关闭文件

    // strip newline
    char *pos = strchr(line, '\n'); // 查找字符串中的“\n”并返回其位置
    if (pos) {
        *pos = '\0'; // 把\n替换成\0
    }
    ESP_LOGI(TAG, "Read from file: '%s'", line); // 把数组内容输出到终端

    return ESP_OK;
}

void My_SD_Init(void)
{
    esp_err_t ret;

    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = true,   // 如果挂载不成功是否需要格式化SD卡
        .max_files = 5, // 允许打开的最大文件数
        .allocation_unit_size = 128 * 1024  // 分配单元大小
    };
    
    sdmmc_card_t *card;
    const char mount_point[] = MOUNT_POINT;
    ESP_LOGI(TAG, "Initializing SD card");
    ESP_LOGI(TAG, "Using SDMMC peripheral");

    sdmmc_host_t host = SDMMC_HOST_DEFAULT(); // SDMMC主机接口配置
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT(); // SDMMC插槽配置
    slot_config.width = 1;  // 设置为1线SD模式
    slot_config.clk = SD_PIN_CLK; 
    slot_config.cmd = SD_PIN_CMD;
    slot_config.d0 = SD_PIN_D0;
    slot_config.flags |= SDMMC_SLOT_FLAG_INTERNAL_PULLUP; // 打开内部上拉电阻

    ESP_LOGI(TAG, "Mounting filesystem");
    ret = esp_vfs_fat_sdmmc_mount(mount_point, &host, &slot_config, &mount_config, &card); // 挂载SD卡

    if (ret != ESP_OK) {  // 如果没有挂载成功
        if (ret == ESP_FAIL) { // 如果挂载失败
            ESP_LOGE(TAG, "Failed to mount filesystem. ");
        } else { // 如果是其它错误 打印错误名称
            ESP_LOGE(TAG, "Failed to initialize the card (%s). ", esp_err_to_name(ret));
        }
        return;
    }
    ESP_LOGI(TAG, "Filesystem mounted"); // 提示挂载成功
    sdmmc_card_print_info(stdout, card); // 终端打印SD卡的一些信息

    //新建一个txt文件 并且给文件中写入几个字符
    const char *file_hello = MOUNT_POINT"/hello.txt";
    char data[EXAMPLE_MAX_CHAR_SIZE];
    snprintf(data, EXAMPLE_MAX_CHAR_SIZE, "%s %s!\n", "hellowrold", card->cid.name);
    ret = s_example_write_file(file_hello, data);
    if (ret != ESP_OK) {
        return;
    }

    // 打开txt文件，并读出文件中的内容
    ret = s_example_read_file(file_hello);
    if (ret != ESP_OK) {
        return;
    }

    //卸载SD卡
    // esp_vfs_fat_sdcard_unmount(mount_point, card);
    // ESP_LOGI(TAG, "Card unmounted");
}

/*
    jpeg格式1600*1200相机初始化
*/
void bap_camera_capture_init(void)
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

        .xclk_freq_hz = 20000000,       // XCLK = 20MHz
        .ledc_timer = LEDC_TIMER_1,
        .ledc_channel = LEDC_CHANNEL_1,
        .pixel_format = PIXFORMAT_JPEG, // JPEG格式保存
        .frame_size = FRAMESIZE_UXGA,     // 1600x1200
        .jpeg_quality = 10,             // 0-63 越小质量越好
        .fb_count = 2,
        .fb_location = CAMERA_FB_IN_PSRAM,
        .grab_mode = CAMERA_GRAB_WHEN_EMPTY,
        .sccb_i2c_port = 1,
    };

     // 先释放之前的摄像头
    esp_camera_deinit();  

    // 修改配置
    cam_config.pixel_format = PIXFORMAT_JPEG;
    cam_config.frame_size = FRAMESIZE_UXGA;

    // 再初始化
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
        s->set_hmirror(s, 1);  //摄像头镜像 写1镜像 写0不镜像
    }

    

    //camera_capture();
}

//拍一帧图片存到sd卡 
//将摄像头设置成最大分辨率及其jpeg格式
//参数fb 获取图片的句柄
//写入失败会释放图片帧
void camera_capture_to_sd(camera_fb_t *fb)
{
    // sensor_t *s = esp_camera_sensor_get();
    // s->set_pixformat(s, PIXFORMAT_JPEG); //JPEG
    // s->set_framesize(s, FRAMESIZE_UXGA); //1600x1200

    if (!fb) {
        ESP_LOGE(TAG, "Camera capture failed");
        return;
    }

    // 创建路径
    mkdir(MOUNT_POINT LMAGE_PATH, 0775);

    static uint32_t file_number = 0;
    char filename[64];

    // 避免文件名冲突
    do {
        snprintf(filename, sizeof(filename),
                 MOUNT_POINT LMAGE_PATH "/PHOTO%03lu.JPG", file_number++);
    } while (access(filename, F_OK) == 0);

    // 写入文件
    FILE *f = fopen(filename, "wb");
    if (!f) {
        ESP_LOGE(TAG, "Failed to open file: %s", filename);
        esp_camera_fb_return(fb);
        return;
    }

    size_t written = fwrite(fb->buf, 1, fb->len, f);
    fclose(f);

    if (written == fb->len) {
        ESP_LOGI(TAG, "Saved: %s (%zu bytes)", filename, fb->len);
    } else {
        ESP_LOGE(TAG, "Write failed: wrote %zu/%zu bytes", written, fb->len);
    }

    esp_camera_fb_return(fb);
    //拍完照回复成lcd显示的格式
    // s->set_pixformat(s, PIXFORMAT_RGB565); //RGB565
    // s->set_framesize(s, FRAMESIZE_VGA);    //640x480
}
