#ifndef _ALARM_H
#define	_ALARM_H

#include "stm32f10x.h"   

typedef struct Alarm_Time
{
	uint8_t day;
	uint8_t hour;
	uint8_t min;
}Alarm_Time;

extern uint8_t Alarm_Flag;

extern uint16_t Alarm_count;//警报声音需要响几次，该变量为句柄 直接+就好

void Alarm_Init(void);

void Alarm_Proc(void);


//蜂鸣器Delay_us：间隔时间，单位ms
void Alarm_Delay(uint16_t Delay_us);

//蜂鸣器蜂鸣几次 放在主程序
void Alarm_count_Proc(uint16_t *Alarm_count);
	

#endif
