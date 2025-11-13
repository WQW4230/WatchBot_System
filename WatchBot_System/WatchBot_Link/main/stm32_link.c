#include <stdio.h>
#include "stm32_link.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/uart.h"
#include "esp_log.h"

static QueueHandle_t uart_queue;
static const char *TAG = "stm32_link"; 

void stm32_link_init(void)
{
    ESP_LOGI(TAG, "初始化 STM32 UART 链接...");
    
    uart_config_t uart_config;
    uart_config.baud_rate  = 115200;
    uart_config.data_bits  = UART_DATA_8_BITS;
    uart_config.parity     = UART_PARITY_DISABLE;
    uart_config.source_clk = UART_SCLK_DEFAULT;
    uart_config.stop_bits  = UART_STOP_BITS_1;

    if (uart_param_config(UART_NUM_1, &uart_config) != ESP_OK) 
    {
        ESP_LOGE(TAG, "UART 参数配置失败！");
        return;
    }

    if (uart_set_pin(UART_NUM_1, UART_TX_PIN, UART_RX_PIN, UART_PIN_NO_CHANGE,UART_PIN_NO_CHANGE) != ESP_OK) 
    {
        ESP_LOGE(TAG, "UART 引脚配置失败！");
        return;
    }
    
    if (uart_driver_install(UART_NUM_1, 2048, 2048, 20, &uart_queue, 0) != ESP_OK) 
    {
        ESP_LOGE(TAG, "UART 队列创建失败！");
        return;
    }

}

void stm32_send_frame(void)
{

}

void stm32_receive_task(void)
{
    uint8_t buf[128];
    uart_event_t event;//队列状态

    while(1)
    {
        if(xQueueReceive(uart_queue, &event, portMAX_DELAY))
        {
            //数据解析。。。。。。
        }
    }
}