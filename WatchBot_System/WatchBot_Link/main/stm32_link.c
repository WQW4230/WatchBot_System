#include "stm32_link.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include <stdio.h>
#include "driver/uart.h"
#include "esp_log.h"
#include "USART_FrameHandler.h"

static const char *TAG = "stm32_link"; 
static uint8_t uart_buffer[1024];
static QueueHandle_t uart_queue;
QueueHandle_t cmd_execution_handle;//串口命令执行队列

Frame_t USART_Frame = {STATE_WAIT_EADER, {0}, 0, 0, 0};


/*
	收到的字节存储并且判断
	USART_Frame 数据帧结构体变量
	byte 一帧数据
	数据可用时会将ESP32_Tx_Flag置位
*/
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
						xQueueSend(cmd_execution_handle, USART_Frame->ESP_Data, portMAX_DELAY);
                        USART_Frame->State = STATE_WAIT_EADER;
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
						xQueueSend(cmd_execution_handle, USART_Frame->ESP_Data, portMAX_DELAY);
                        USART_Frame->State = STATE_WAIT_EADER;
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
    发送一帧数据
*/
esp_err_t stm32_send_frame(const uint8_t *data)
{
    uint8_t data_length = data[FRAME_IDX_LEN];
    uart_write_bytes(UART_NUM_1, data, data_length);
	return ESP_OK;
}

/*
	读取一帧数据
*/
esp_err_t stm32_read_frame(uint8_t *data)
{
	if(USART_Frame.ESP32_Tx_Flag == 0) return ESP_FAIL; 
	
	USART_Frame.ESP32_Tx_Flag = 0; //有数据帧标志位清除
	for(uint8_t i=0; i<USART_Frame.Data_Len; i++)
	{
		data[i] = USART_Frame.ESP_Data[i];
	}
	return ESP_OK;
}

static void uart_event_task(void *pvParameters)
{
    uart_event_t event;
    while (1)
    {
        //Waiting for UART event.
        if (xQueueReceive(uart_queue, (void *)&event, (TickType_t)portMAX_DELAY)) 
        {
            ESP_LOGI(TAG, "uart[%d] event:", USER_UART_NUM);
            switch (event.type) {
            case UART_DATA:
                ESP_LOGI(TAG, "[UART DATA]: %d", event.size);
                uart_read_bytes(USER_UART_NUM, uart_buffer, event.size, portMAX_DELAY);
				for(uint32_t i=0; i<event.size; i++)
				{
					USART_ProcessByte(&USART_Frame, uart_buffer[i]);
				}
                ESP_LOGI(TAG, "[DATA EVT]:");
                // uart_write_bytes(USER_UART_NUM, uart_buffer, event.size); //调试用
                break;
            //Event of HW FIFO overflow detected
            case UART_FIFO_OVF:
                ESP_LOGI(TAG, "hw fifo overflow");
                uart_flush_input(USER_UART_NUM);
                xQueueReset(uart_queue);
                break;
            //Event of UART ring buffer full
            case UART_BUFFER_FULL:
                ESP_LOGI(TAG, "ring buffer full");
                // If buffer full happened, you should consider increasing your buffer size
                // As an example, we directly flush the rx buffer here in order to read more data.
                uart_flush_input(USER_UART_NUM);
                xQueueReset(uart_queue);
                break;
            //Event of UART RX break detected
            case UART_BREAK:
                ESP_LOGI(TAG, "uart rx break");
                break;
            //Event of UART parity check error
            case UART_PARITY_ERR:
                ESP_LOGI(TAG, "uart parity error");
                break;
            //Event of UART frame error
            case UART_FRAME_ERR:
                ESP_LOGI(TAG, "uart frame error");
                break;
            default:
                ESP_LOGI(TAG, "uart event type: %d", event.type);
                break;
            }
        }
    }
    vTaskDelete(NULL);
}

/*
		
*/
void stm32_receive_task(void *pvParameters)
{
    uint8_t data[128];
	uint16_t length = 0;
    while(1)
    {      
        length = uart_read_bytes(UART_NUM_1, data, 128, portMAX_DELAY);
		if(length > 0)
		{
			for(uint16_t i=0; i<length; i++)
        	{
            	USART_ProcessByte(&USART_Frame, data[i]);
        	}
		} 
    }
}

esp_err_t stm32_link_init(void)
{
    ESP_LOGI(TAG, "初始化 STM32 UART 链接初始化...");
    uart_config_t uart_cfg={
        .baud_rate=115200,                          //波特率115200
        .data_bits=UART_DATA_8_BITS,                //8位数据位
        .flow_ctrl=UART_HW_FLOWCTRL_DISABLE,        //无硬件流控制
        .parity=UART_PARITY_DISABLE,                //无校验位
        .stop_bits=UART_STOP_BITS_1                 //1位停止位
    };
    uart_driver_install(USER_UART_NUM,1024,1024,20,&uart_queue,0);     //安装串口驱动
    uart_param_config(USER_UART_NUM,&uart_cfg);
    uart_set_pin(USER_UART_NUM, UART_TX_IO, UART_RX_IO, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE); 

	if (xTaskCreatePinnedToCore(uart_event_task,"uart_event_task",4096,NULL,9,NULL,1) != pdPASS) 
    {
        ESP_LOGE(TAG, "UART 接收任务创建失败");
        return ESP_FAIL;
    }

	cmd_execution_handle = xQueueCreate(30, DATA_MAX_LEN * sizeof(uint8_t));
	
	return ESP_OK;
}
