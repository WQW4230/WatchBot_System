#ifndef _APP_TIMER_H
#define _APP_TIMER_H

#include "stm32f10x.h"

void App_Timer_Init(void);

//系统毫秒时间及其毫秒延迟
uint64_t App_Timer_GetTick(void);
void App_TImer_Delay(uint32_t ms);

//系统微秒时间及其微秒延迟
uint64_t App_Timer_GetUs(void);
void App_Timer_Delay_us(uint32_t us);
	


#endif
