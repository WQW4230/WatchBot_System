#ifndef _CAMERRA_DRIVER_H__
#define _CAMERRA_DRIVER_H__
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_camera.h>

#define CAM_PIN_PWDN -1
#define CAM_PIN_RESET   -1
#define CAM_PIN_D0  11
#define CAM_PIN_D1  9
#define CAM_PIN_D2 8
#define CAM_PIN_D3 10
#define CAM_PIN_D4 12
#define CAM_PIN_D5 18
#define CAM_PIN_D6 17
#define CAM_PIN_D7 16
#define CAM_PIN_SIOD 4
#define CAM_PIN_SIOC 5
#define CAM_PIN_VSYNC 6
#define CAM_PIN_HREF 7
#define CAM_PIN_XCLK 15
#define CAM_PIN_PCLK 13

void bsp_camera_init();
void camera_capture();

#endif
