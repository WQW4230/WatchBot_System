/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <stdint.h>
#include "driver/rmt_encoder.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef struct ws2812_strip_t *ws2812_strip_handle_t;

/** 初始化WS2812外设
 * @param gpio 控制WS2812的管脚
 * @param maxled 控制WS2812的个数
 * @param led_handle 返回的控制句柄
 * @return ESP_OK or ESP_FAIL
*/
esp_err_t ws2812_init(gpio_num_t gpio,int maxled,ws2812_strip_handle_t* led_handle);

/** 反初始化WS2812外设
 * @param handle 初始化的句柄
 * @return ESP_OK or ESP_FAIL
*/
esp_err_t ws2812_deinit(ws2812_strip_handle_t handle);

/** 向某个WS2812写入RGB数据
 * @param handle 句柄
 * @param index 第几个WS2812（0开始）
 * @param r,g,b RGB数据
 * @return ESP_OK or ESP_FAIL
*/
esp_err_t ws2812_write(ws2812_strip_handle_t handle,uint32_t index,uint32_t r,uint32_t g,uint32_t b);


#ifdef __cplusplus
}
#endif
