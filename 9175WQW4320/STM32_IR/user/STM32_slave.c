#include "STM32_slave.h"
#include "usart_frame_handler.h"

void esp_command(void)
{
	if(ESP32_Rx_Flag == 0) return;
	
	//到这表示一帧数据已经收到，将标志位置位
	ESP32_Rx_Flag = 0;
	
	
}
