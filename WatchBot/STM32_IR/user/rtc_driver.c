#include "rtc_driver.h"
#include <time.h>

RTC_Time Rtctime;

void Rtc_Init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_BKP, ENABLE);
	
	PWR_BackupAccessCmd(ENABLE);
	
	if(BKP_ReadBackupRegister(BKP_DR1) != 0x01)
	{
		RCC_LSEConfig(RCC_LSE_ON);
		while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) != SET);
		
		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
		RCC_RTCCLKCmd(ENABLE);
		
		RTC_WaitForSynchro();
		RTC_WaitForLastTask();
		
		RTC_SetPrescaler(32768 - 1);
		RTC_WaitForLastTask();
		
		RTC_SetCounter(1756976256);
		RTC_WaitForLastTask();
		
		BKP_WriteBackupRegister(BKP_DR1, 0x01);
	}
	else
	{
		RTC_WaitForSynchro();
		RTC_WaitForLastTask();
	}
		
}

void RTC_SetTime(RTC_Time Rtctime)
{
	time_t time_Cnt;
	struct tm time_date;
	time_date.tm_year = Rtctime.year - 1900;
	time_date.tm_mon  = Rtctime.mon - 1;
	time_date.tm_hour = Rtctime.hour;
	time_date.tm_mday = Rtctime.day;
	time_date.tm_min  = Rtctime.min;
	time_date.tm_sec  = Rtctime.sec;
	
	time_Cnt = mktime(&time_date);
	
	RTC_SetCounter(time_Cnt);
	RTC_WaitForLastTask();
}

void RTC_ReadTime(void)
{
	time_t Time_Cnt;
	
	struct tm time_date;
	
	Time_Cnt = RTC_GetCounter() + 8 * 60 * 60;//东八区加8小时 	
	
	time_date = *localtime(&Time_Cnt);
	
	Rtctime.year = time_date.tm_year + 1900;
	Rtctime.mon  = time_date.tm_mon + 1;
	Rtctime.day  = time_date.tm_mday;
  Rtctime.hour = time_date.tm_hour;
  Rtctime.min  = time_date.tm_min;
  Rtctime.sec  = time_date.tm_sec;
}
