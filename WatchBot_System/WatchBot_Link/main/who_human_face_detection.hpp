#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

void app_camera_ai_lcd(void);

#ifdef __cplusplus
extern "C" {
#endif

extern QueueHandle_t xQueueCaptureNotify;

#ifdef __cplusplus
}
#endif

