#ifndef _STM32_USART_SLAVE_H
#define	_STM32_USART_SLAVE_H

#include "stm32f10x.h"         

extern uint8_t ESP32_Rx_Flag;  //当前标志位挂起时表示从esp32接收到了一帧数据

void usart_slave_Init(void);   //初始化USART串口

void USART_ProcessByte(uint8_t byte); //中断函数内调用处理处理数据帧--非中断测试用
	

#endif
