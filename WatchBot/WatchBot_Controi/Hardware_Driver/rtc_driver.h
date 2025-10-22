#ifndef _RTC_DRIVER_H
#define	_RTC_DRIVER_H

#include "stm32f10x.h"

typedef struct RTC_Time
{
	uint16_t year;
	uint8_t mon;
	uint8_t day;
	uint8_t hour;
	uint8_t min;
	uint8_t sec;
}RTC_Time;

//时间错操作句柄,读写
extern RTC_Time Rtctime;
//初始化
void Rtc_Init(void);
//设置时间戳
void RTC_SetTime(RTC_Time Rtctime);
//读当前时间戳到Rtctime里
void RTC_ReadTime(void);

#endif
