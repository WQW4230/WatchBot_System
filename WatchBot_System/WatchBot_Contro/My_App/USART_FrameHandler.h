#ifndef _USART_FRAMEHANDLER_H
#define _USART_FRAMEHANDLER_H

#include "stm32f10x.h" 

typedef enum
{
	CMD_ARM_CONTROL = 0x00,   //控制机械臂
	CMD_BUZZER_CTRL = 0x01,		//控制指示灯
	CMD_BUZZER_OFF  = 0x02
}USART_Command_e;

void USART_FrameHandler_Init(void);

void USART_FrameHandler_Task(void);

#endif
