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

extern RTC_Time Rtctime;

void Rtc_Init(void);
void RTC_SetTime(RTC_Time Rtctime);
void RTC_ReadTime(void);

#endif
