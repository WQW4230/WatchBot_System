#ifndef _ALARM_H
#define	_ALARM_H

#include "stm32f10x.h"   

typedef struct 
{
  uint16_t alarm_clock[4]; 				// 设定闹钟的时间 最后一个数组代表闹钟标志位 0代表闹钟关闭 1代表闹钟开启 2代表闹钟正在触发
	uint16_t Change_Time[6]; 				// 更改的时间
  uint8_t cursor;    			 				// 当前光标所在行（0~2）
	uint16_t buzzer_ontime;  				// 蜂鸣器蜂鸣时间
	uint16_t buzzer_offtime; 				// 蜂鸣器关闭时间
} Alarm_menu_state;

//RTC时钟_闹钟初始化
void Alarm_Init(void);

//任务进程函数
void Alarm_Clock_Proc(void);

	

#endif
