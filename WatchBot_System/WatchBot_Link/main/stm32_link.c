#include "stm32_link.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include <stdio.h>
#include "driver/uart.h"
#include "esp_log.h"

static const char *TAG = "stm32_link"; 
static QueueHandle_t uart_queue = NULL; //uart接收队列
QueueHandle_t executeFrame_queue = NULL; //执行指令队列

Frame_t USART_Frame = {STATE_WAIT_EADER, {0}, 0, 0, 0};

void USART_ProcessByte(Frame_t *USART_Frame, uint8_t byte)
{
	switch(USART_Frame->State)
	{
		//等待帧头
		case STATE_WAIT_EADER:
		{
			if(byte == FRAME_HEADER)
			{
				USART_Frame->Data_index = 0;
				USART_Frame->ESP_Data[USART_Frame->Data_index] = byte;
				USART_Frame->Data_index += 1 ;
				USART_Frame->State = STATE_RECV_DATA;				
			}
			else
			{
				USART_Frame->Data_index = 0 ;
			}		
		}
		break;
		
		//接收数据
		case STATE_RECV_DATA:
		{
			USART_Frame->ESP_Data[USART_Frame->Data_index] = byte;
			USART_Frame->Data_index += 1 ;
			
			switch(USART_Frame->Data_index)
			{
				//数据帧长度为5时 校验数据位长度并校验帧尾是否正确
				case DATA_MIN_LEN:
				{
					//接收成功
					if(USART_Frame->ESP_Data[FRAME_IDX_LEN] == DATA_MIN_LEN && USART_Frame->ESP_Data[FRAME_IDX_END_MIN] == FRAME_END)
					{
						USART_Frame->ESP32_Tx_Flag = 1;
						USART_Frame->Data_Len = USART_Frame->Data_index;
                        xQueueSend(executeFrame_queue, &USART_Frame, portMAX_DELAY);//发送队列通知
                        USART_Frame->State = STATE_WAIT_EADER;
                        USART_Frame->ESP32_Tx_Flag = 0;
						return;
					}
					else if(USART_Frame->ESP_Data[FRAME_IDX_LEN] == DATA_MIN_LEN&& USART_Frame->Data_index == DATA_MIN_LEN) 
					{
						//如果是5字节的并且索引已经超过5字节则是乱码
						USART_Frame->State = STATE_WAIT_EADER;
					}
				}		
				break;
				//数据帧长度为8时 校验数据位长度并校验帧尾是否正确
				case DATA_MAX_LEN:
				{
					//接收成功
					if(USART_Frame->ESP_Data[FRAME_IDX_LEN] == DATA_MAX_LEN && USART_Frame->ESP_Data[FRAME_IDX_END_MAX] == FRAME_END)
					{
						USART_Frame->ESP32_Tx_Flag = 1;
						USART_Frame->Data_Len = USART_Frame->Data_index;
                        xQueueSend(executeFrame_queue, &USART_Frame, portMAX_DELAY);//发送队列通知
                        USART_Frame->State = STATE_WAIT_EADER;
                        USART_Frame->ESP32_Tx_Flag = 0;
						return;
					}
					else if(USART_Frame->ESP_Data[FRAME_IDX_LEN] == DATA_MAX_LEN&& USART_Frame->Data_index == DATA_MAX_LEN) 
					{
						//如果是8字节的并且索引已经超过8字节则是乱码
						USART_Frame->State = STATE_WAIT_EADER;
					}
				}	
				break;
				
			}	
		}
		break;	

	}
}

/*
    发送一帧命令
*/
void stm32_send_frame(const uint8_t *data)
{
    uint8_t data_length = data[FRAME_IDX_LEN];
    uart_write_bytes(UART_NUM_1, data, data_length);
}

void stm32_receive_task(void)
{
    uint8_t data[128];
    uart_event_t event;//队列状态

    while(1)
    {
        if(xQueueReceive(uart_queue, &event, portMAX_DELAY))
        {
            uint16_t length = 0;
            length = uart_read_bytes(UART_NUM_1, data, event.size, portMAX_DELAY);
            for(uint16_t i=0; i<length; i++)
            {
                USART_ProcessByte(&USART_Frame, data[i]);
            }
        }
    }
}

void stm32_link_init(void)
{
    ESP_LOGI(TAG, "初始化 STM32 UART 链接...");
    
    uart_config_t uart_config;
    uart_config.baud_rate  = 115200;
    uart_config.data_bits  = UART_DATA_8_BITS;
    uart_config.parity     = UART_PARITY_DISABLE;
    uart_config.source_clk = UART_SCLK_DEFAULT;
    uart_config.stop_bits  = UART_STOP_BITS_1;
	uart_config.flow_ctrl  = UART_HW_FLOWCTRL_DISABLE;

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
	executeFrame_queue = xQueueCreate(10, sizeof(Frame_t));
	if (executeFrame_queue == NULL)
	{
		ESP_LOGE(TAG, "UART 队列创建失败");
		return;
	}

	if (uart_driver_install(UART_NUM_1, 2048, 2048, 10, &uart_queue, 0) != ESP_OK)
	{
		ESP_LOGE(TAG, "UART 队列创建失败");
		return;
	}
}