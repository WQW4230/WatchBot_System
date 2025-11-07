#ifndef _USART_FRAMEHANDLER_H
#define _USART_FRAMEHANDLER_H

#include "stm32f10x.h"         


typedef enum
{
	CMD_ARM_CONTROL = 			 0x00,   //控制机械臂
	CMD_BUZZER_CTRL =   	   0x01,		//控制STM板载指示灯
	CMD_BUZZER_OFF  =        0x02,   //关闭STM板载指示灯
	CMD_ESP32_LED   = 			 0x03,   //ESP板载LED
	CMD_ESP32CAM_WHITE_LED = 0x04,   //闪光灯白色
	CMD_ESP32CAM_BLUE_LED =  0x05,	 //闪关灯蓝色
	CMD_ESP32CAM_RED_LED  =  0x06		 //闪光灯红色
}USART_RxCommand_e;

void USART_FrameHandler_Init(void);
void USART_SenFrame(uint8_t CMD, uint16_t ON, uint16_t OFF);
void USART_FrameHandler_Task(void);

#endif
