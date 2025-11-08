#include "my_sd_driver.h"
#include <stdio.h>
#include <string.h>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/sdmmc_host.h"

static const char *TAG = "My_SD_Driver";

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
        .allocation_unit_size = 16 * 1024  // 分配单元大小
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

    // 新建一个txt文件 并且给文件中写入几个字符
    const char *file_hello = MOUNT_POINT"/hello.txt";
    char data[EXAMPLE_MAX_CHAR_SIZE];
    snprintf(data, EXAMPLE_MAX_CHAR_SIZE, "%s %s!\n", "我好hello先生", card->cid.name);
    ret = s_example_write_file(file_hello, data);
    if (ret != ESP_OK) {
        return;
    }

    // 打开txt文件，并读出文件中的内容
    ret = s_example_read_file(file_hello);
    if (ret != ESP_OK) {
        return;
    }

    // 卸载SD卡
    esp_vfs_fat_sdcard_unmount(mount_point, card);
    ESP_LOGI(TAG, "Card unmounted");
}
