#ifndef _MY_SD_DRIVER_H_
#define _MY_SD_DRIVER_H_

#include <esp_camera.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SD_PIN_CMD 38
#define SD_PIN_CLK 39
#define SD_PIN_D0  40

#define MOUNT_POINT        "/sdcard"
#define LMAGE_PATH   "/lmage" //图片路径
#define VIDEO_PATH   "/video" //视频路径

#define EXAMPLE_MAX_CHAR_SIZE    512

void My_SD_Init(void);
void bap_camera_capture_init(void);
void camera_capture_to_sd(camera_fb_t *fb);

#ifdef __cplusplus
}
#endif

#endif
