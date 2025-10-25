#include "Alarm.h"
#include "App_Timer.h"
#include "OLED.h"
#include "rtc_driver.h"
#include "remote_nec_driver.h"
#include "menu.h"
#include <string.h>

RTC_Time Alarm;

static uint8_t Alarm_Cursor = 0;

Alarm_Time Set_AlarmTime = {0, 0, 0};

uint16_t Alarm_count = 3; //警报声句柄，响几次

uint8_t Alarm_Flag = 0;//闹钟标志位

static uint8_t	Set_Alarm_Flag = 0;//设定时间界面标志位
static uint8_t Set_RTC_Flag_Flag = 0;//设置时间标志位

static uint8_t Alarm_switch = 0;//蜂鸣器标志位
static uint8_t Alarm_Trigger_Flag = 0; //设定时间已到

//引脚初始化
void Alarm_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_14;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	GPIO_WriteBit(GPIOB, GPIO_Pin_14, Bit_SET);
}

//声音间隔
void Alarm_Delay(uint16_t Delay_ms)
{
	static uint8_t Alarm_Delay_Flag = 1;
	
	static uint64_t Last_Time = 0;
	uint64_t Current_Time = App_Timer_GetTick();
	
	if(Alarm_Delay_Flag == 1)
	{
		if(Current_Time + Last_Time >= Delay_ms)
		{
			Last_Time = App_Timer_GetTick();
			GPIO_WriteBit(GPIOB, GPIO_Pin_14, Bit_RESET);
			Alarm_Delay_Flag = 0;
			if(Alarm_count != 0)
			{
				Alarm_count--;//计数响了几次 
			}				
		}
		
	}
	
	if(Alarm_Delay_Flag == 0)
	{
		if(Current_Time - Last_Time >= Delay_ms)
		{
			Last_Time = App_Timer_GetTick();
			GPIO_WriteBit(GPIOB, GPIO_Pin_14, Bit_SET);
			Alarm_Delay_Flag = 1;
		}
	}
	
}


//闹钟设置时间
void Set_Alarm(void)
{	
	if(Set_Alarm_Flag == 1)
	{
		static uint8_t index = 0;//结构体索引
		
		if(IR_Flag == 1 && (IR_GetKey() <= 9))
		{
			IR_Flag = 0;			
			uint8_t Key = IR_GetKey();
			switch(index)
			{
				case 0:Set_AlarmTime.day  = Set_AlarmTime.day * 10 + Key; break;
				case 1:Set_AlarmTime.hour = Set_AlarmTime.hour * 10 + Key; break;
				case 2:Set_AlarmTime.min  = Set_AlarmTime.min * 10 + Key; break;			
			}
			
		}
		
		if(IR_Flag == 1 && (IR_GetKey() == Key_OK))
		{
			IR_Flag = 0;
			index++;
			if(index == 4)
			{
				Set_Alarm_Flag = 0;
				Alarm_Flag = 1;
				index = 0;
			}
		}
		
		if(IR_Flag == 1 && (IR_GetKey() == Key_XingHao))
		{
			IR_Flag = 0;
			Set_Alarm_Flag = 0;
			Alarm_Flag = 1;
			index = 0;
		}
		
		if(IR_Flag == 1 && (IR_GetKey() == Key_JingHao))
		{
			IR_Flag = 0;
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

//持续响铃函数
void Alarm_Ring(void)
{
	if((Alarm_Trigger_Flag == 1) && (Alarm_switch == 1))
	{
		if(IR_GetKey() == Key_JingHao)
		{
			IR_Flag = 0;
			Alarm_Trigger_Flag = 0;
			Alarm_switch = 0;
			GPIO_WriteBit(GPIOB, GPIO_Pin_14, Bit_SET);
		}
		else
		{
			Alarm_Delay(500);
		}
		
	}
}

//时间设置
void Set_RTC_Time(void)
{
	if(Set_RTC_Flag_Flag == 1)
	{
		static uint32_t Tmp[6];//存放年月日
		static uint8_t Tmp_Cnt;//Tmp下标
		if(IR_Flag == 1 && (IR_GetKey() <= 9))
		{
			IR_Flag = 0;
			
			uint8_t Key = IR_GetKey();
			Tmp[Tmp_Cnt] = Key + Tmp[Tmp_Cnt] * 10;
			
		}
		
		if(IR_Flag == 1 && (IR_GetKey() == Key_OK))
		{
			IR_Flag = 0;
			Tmp_Cnt++;
			
			if(Tmp_Cnt == 6)
			{
				Alarm.year = Tmp[0];
				Alarm.mon  = Tmp[1];
				Alarm.day  = Tmp[2];
				Alarm.hour = Tmp[3];
				Alarm.min  = Tmp[4];
				Alarm.sec  = Tmp[5];
				RTC_SetTime(Alarm);
				
				memset(Tmp, 0, sizeof(Tmp));
				IR_Flag = 0;
				Tmp_Cnt = 0;
				Set_RTC_Flag_Flag = 0;
				Alarm_Flag = 1;
			}
		}
		
		if(IR_GetKey() == Key_XingHao)
		{
			memset(Tmp, 0, sizeof(Tmp));
			Tmp_Cnt = 0;
			IR_Flag = 0;
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
		Alarm_Trigger_Flag = 1;
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

//警报声
void Alarm_count_Proc(uint16_t *count)
{
	if((*count) == 0)
	{
		GPIO_WriteBit(GPIOB, GPIO_Pin_14, Bit_SET);
		return;
	}
	Alarm_Delay(100);
}

//进程
void Alarm_Proc(void)
{
	if(Alarm_Flag == 1)
	{
		Alarm_Show();
		
		if(IR_Flag == 1)
		{	
			IR_Flag = 0;
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
	Alarm_Ring();//响铃函数

	
	
	
}
