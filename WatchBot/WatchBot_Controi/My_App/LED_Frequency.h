#ifndef _LED_FREQUENCY_H
#define _LED_FREQUENCY_H

#include "stm32f10x.h" 

typedef struct
{
	uint8_t current;							//当前指向的索引
	uint8_t up;										//向上翻索引号
	uint8_t down;									//向下翻索引号
	uint8_t enter;								//确认索引号
	void (*current_operation)(void);  //任务函数
} KeyOperation_t;


void Led_BlinkInit(void);

void LED_Proc_test(void);

extern uint8_t LED_Blink_Flag;

void Led_Blink_Proc(void);

#endif
