#include "Alarm_Clock.h"
#include "Scheduler.h"
#include "Active_Buzzer.h"

#include "OLED.h"
#include "rtc_driver.h"
#include <string.h>
#include "menu.h"
#include "NEC_driver.h"

Buzzer_t PB14_Buzzer;

RTC_Time Alarm;

static uint8_t Alarm_Cursor = 0;

Alarm_Time Set_AlarmTime = {0, 0, 0};

uint16_t Alarm_count = 3; //警报声句柄，响几次

uint8_t Alarm_Flag = 0;//闹钟标志位

static uint8_t	Set_Alarm_Flag = 0;//设定时间界面标志位
static uint8_t Set_RTC_Flag_Flag = 0;//设置时间标志位

static uint8_t Alarm_switch = 0;//蜂鸣器标志位
//static uint8_t Alarm_Trigger_Flag = 0; //设定时间已到

//引脚初始化
void Alarm_Init(void)
{
	PB14_Buzzer.GPIOX = GPIOB;
	PB14_Buzzer.GPIO_PIN = GPIO_Pin_14;
	PB14_Buzzer.Lofic =BUZZER_ACTIVE_LOW;
	
	Buzzer_Init(&PB14_Buzzer);
	
	//Buzzer_SetMode(&PB14_Buzzer, Buzzer_Beep, 1000, 500);
	
	Scheduler_AddTask(Buzzer_Proc, 10, 1, 1000);
}



//闹钟设置时间
void Set_Alarm(void)
{	
	if(Set_Alarm_Flag == 1)
	{
		static uint8_t index = 0;//结构体索引
		
		if(NEC_RxFlag == 1 && (IR_GetKey() <= 9))
		{
			NEC_RxFlag = 0;			
			uint8_t Key = IR_GetKey();
			switch(index)
			{
				case 0:Set_AlarmTime.day  = Set_AlarmTime.day * 10 + Key; break;
				case 1:Set_AlarmTime.hour = Set_AlarmTime.hour * 10 + Key; break;
				case 2:Set_AlarmTime.min  = Set_AlarmTime.min * 10 + Key; break;			
			}
			
		}
		
		if(NEC_RxFlag == 1 && (IR_GetKey() == Key_OK))
		{
			NEC_RxFlag = 0;
			index++;
			if(index == 4)
			{
				Set_Alarm_Flag = 0;
				Alarm_Flag = 1;
				index = 0;
			}
		}
		
		if(NEC_RxFlag == 1 && (IR_GetKey() == Key_XingHao))
		{
			NEC_RxFlag = 0;
			Set_Alarm_Flag = 0;
			Alarm_Flag = 1;
			index = 0;
		}
		
		if(NEC_RxFlag == 1 && (IR_GetKey() == Key_JingHao))
		{
			NEC_RxFlag = 0;
			switch(index)
			{
				case 0:Set_AlarmTime.day  = 0; break;
				case 1:Set_AlarmTime.hour = 0; break;
				case 2:Set_AlarmTime.min  = 0; break;			
			}
		}
		
		//显示模块
		OLED_Clear();
		OLED_Printf(0, 0, OLED_8X16, "%d %d : %d", Set_AlarmTime.day, Set_AlarmTime.hour, Set_AlarmTime.min);
		switch(index)
			{
				case 0:OLED_Printf(0, 16, OLED_8X16, "%d", Set_AlarmTime.day); break;
				case 1:OLED_Printf(0, 16, OLED_8X16, "%d", Set_AlarmTime.hour); break;
				case 2:OLED_Printf(0, 16, OLED_8X16, "%d", Set_AlarmTime.min); break;	
			}
		OLED_Update();
		
	}
	
}


//时间设置
void Set_RTC_Time(void)
{
	if(Set_RTC_Flag_Flag == 1)
	{
		static uint32_t Tmp[6];//存放年月日
		static uint8_t Tmp_Cnt;//Tmp下标
		if(NEC_RxFlag == 1 && (IR_GetKey() <= 9))
		{
			NEC_RxFlag = 0;
			
			uint8_t Key = IR_GetKey();
			Tmp[Tmp_Cnt] = Key + Tmp[Tmp_Cnt] * 10;
			
		}
		
		if(NEC_RxFlag == 1 && (IR_GetKey() == Key_OK))
		{
			NEC_RxFlag = 0;
			Tmp_Cnt++;
			
			if(Tmp_Cnt == 6)
			{
				Alarm.year = Tmp[0];
				Alarm.mon  = Tmp[1];
				Alarm.day  = Tmp[2];
				Alarm.hour = Tmp[3] - 8;//东八区换算
				Alarm.min  = Tmp[4];
				Alarm.sec  = Tmp[5];
				RTC_SetTime(Alarm);
				
				memset(Tmp, 0, sizeof(Tmp));
				NEC_RxFlag = 0;
				Tmp_Cnt = 0;
				Set_RTC_Flag_Flag = 0;
				Alarm_Flag = 1;
			}
		}
		
		if(IR_GetKey() == Key_XingHao)
		{
			memset(Tmp, 0, sizeof(Tmp));
			Tmp_Cnt = 0;
			NEC_RxFlag = 0;
			Set_RTC_Flag_Flag = 0;
			Alarm_Flag = 1;
		}
		
		OLED_Clear();
		OLED_Printf(0, 0, OLED_6X8, "%d-%d-%d-%d-%d-%d", Tmp[0]
		, Tmp[1], Tmp[2], Tmp[3], Tmp[4], Tmp[5]);
		OLED_Printf(0, 8, OLED_8X16, "%d", Tmp[Tmp_Cnt]);
		OLED_UpdateArea(0, 0, 128, 40);
	}
}

//选择状态机
void Alarm_Switch(void)
{
	Alarm_Flag = 0;
	
	switch(Alarm_Cursor)
	{
		case 0:
			Set_RTC_Flag_Flag = 1;
			break;
		case 1:		
			Set_Alarm_Flag = 1;
			break;
		case 2:
			Alarm_Flag = 1;
			Alarm_switch = !Alarm_switch;
			break;
	}
}


//时间比较函数
void Alarm_Check(void)
{
	//读当前时间
	RTC_ReadTime();
	if(Set_AlarmTime.day  == Rtctime.day   &&
	   Set_AlarmTime.hour == Rtctime.hour  &&
	   Set_AlarmTime.min  == Rtctime.min   &&
			      Rtctime.sec == 0               )     
	{
		//Alarm_Trigger_Flag = 1;
	}
}


//显示函数
void Alarm_Show(void)
{	
	//时间显示
	OLED_Clear();
	RTC_ReadTime();
	OLED_Printf(0, 2, OLED_6X8, "%d-%d-%d-%d-%d-%d", 
	Rtctime.year, Rtctime.mon, Rtctime.day,
	Rtctime.hour, Rtctime.min, Rtctime.sec);
	
	//闹钟显示
	OLED_Printf(0, 10, OLED_8X16, "%d - %d : %d", Set_AlarmTime.day, Set_AlarmTime.hour, Set_AlarmTime.min);
	
	//开关显示
	if(Alarm_switch == 1)
		OLED_Printf(0, 26, OLED_8X16, "ON");
	else
		OLED_Printf(0, 26, OLED_8X16, "OFF");
	
	if(Alarm_Cursor == 0){OLED_ReverseArea(0, 1, 128, 9); };
	if(Alarm_Cursor == 1){OLED_ReverseArea(0, 10, 128, 16); };
	if(Alarm_Cursor == 2){OLED_ReverseArea(0, 26, 128, 16); };
	
	OLED_Update();
}


//进程
void Alarm_Proc(void)
{
	if(Alarm_Flag == 1)
	{
		Alarm_Show();
		
		if(NEC_RxFlag == 1)
		{	
			NEC_RxFlag = 0;
			switch(IR_GetKey())
			{
				case Key_W:
				{
					if(Alarm_Cursor != 0)
					{
						Alarm_Cursor--;
					}
					break;
				}
				case Key_S:
				{
					if(Alarm_Cursor != 2)
					{
						Alarm_Cursor++;
					}
					break;
				}
				case Key_OK:
				{
					Alarm_Switch();
					break;
				}
				case Key_XingHao:
				{
					Menu_Flag = 1;
					Alarm_Flag = 0;
					OLED_Clear();
					break;
				}
			}
			
		}		
		
	}
	Set_RTC_Time();//设置时间进程函数
	Set_Alarm();//设置闹钟进程函数
	Alarm_Check();//时间比较函数

}
