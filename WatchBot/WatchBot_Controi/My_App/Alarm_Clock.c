#include "Alarm_Clock.h"
#include "Scheduler.h"
#include "Active_Buzzer.h"
#include "OLED.h"
#include "rtc_driver.h"
#include <string.h>
#include "NEC_driver.h"

Buzzer_t PB14_Buzzer; //蜂鸣器初始化句柄
RTC_Time Alarm; //当前时间戳句柄

Alarm_menu_state Alarm_setHandle; //闹钟菜单控制句柄

extern void Menu_Proc(void);//一级主页菜单

static void Time_Set(void);  			//设置时间
static void Alarm_Set(void);			//设置闹钟
static void Buzzer_SetParam(void);//设置蜂鸣器参数

//引脚初始化
void Alarm_Init(void)
{
	Rtc_Init();
	
	//初始化蜂鸣器驱动
	PB14_Buzzer.GPIOX = GPIOB;
	PB14_Buzzer.GPIO_PIN = GPIO_Pin_14;
	PB14_Buzzer.Lofic =BUZZER_ACTIVE_LOW;//低电平触发
	Buzzer_Init(&PB14_Buzzer); //蜂鸣器PB14引脚
	
	RTC_ReadTime(); //读当前时间
	Alarm = Rtctime;//当前时间赋值给设置时间
	
	Alarm_setHandle.Change_Time[0] = Alarm.year;
	Alarm_setHandle.Change_Time[1] = Alarm.mon;
	Alarm_setHandle.Change_Time[2] = Alarm.day;
	Alarm_setHandle.Change_Time[3] = Alarm.hour;
	Alarm_setHandle.Change_Time[4] = Alarm.min;
	Alarm_setHandle.Change_Time[5] = Alarm.sec;
}


/*
	菜单选项确认后进入功能设置界面
	参数 index：选择的是哪个功能
*/
static void menu_selector(uint8_t index)
{
	//删除一级菜单任务
	vTask_Delete(Alarm_Clock_Proc);
	switch(index)
	{
		//选择0为时间设置界面
		case 0:
			Scheduler_AddTask(Time_Set, 50, 5, 1000);
			break;
		//选择1为闹钟设置界面
		case 1:
			Scheduler_AddTask(Alarm_Set, 50, 5, 1000);
			break;
		//选择2为蜂鸣器参数设置界面
		case 2:
			Scheduler_AddTask(Buzzer_SetParam, 50, 5, 1000);
			break;
	}
}

//设置时间戳
static void Time_Set(void)
{
	static uint8_t time_index = 0; //选中的时间索引，0-5表示年月日 
	uint16_t *SetTime_arr = Alarm_setHandle.Change_Time; //拿到设置时间句柄
	
	uint8_t key;	//存按键值
	if(IR_GetKey(&key) == -1) return;//无按键或错误

	switch(key)
	{
		//左键索引--
		case Key_A:
			if(time_index == 0) 
				time_index = 5;
			else
				time_index--;
			break;
		//右键索引++
		case Key_D:
			if(time_index == 5) 
				time_index = 0;
			else
				time_index++;
			break;
		//*号键退出当前菜单
		case Key_XingHao:
			vTask_Delete(Time_Set);
			Scheduler_AddTask(Alarm_Clock_Proc, 50, 5, 1000);
			break;
		//#号键清空当前数组
		case Key_JingHao:
			SetTime_arr[time_index] = 0;
			break;
		//OK键确认更改
		case Key_OK:
			Alarm.year = SetTime_arr[0];
			Alarm.mon  = SetTime_arr[1];
			Alarm.day  = SetTime_arr[2];
			Alarm.hour = SetTime_arr[3];
			Alarm.min  = SetTime_arr[4];
			Alarm.sec  = SetTime_arr[5];
			RTC_SetTime(Alarm);
			break;
		//数字键更改当前数组
		default:
			if(key > 9) break;
			SetTime_arr[time_index] = (10 * SetTime_arr[time_index]) + key;
	}
	OLED_Clear();
	OLED_Printf(0, 0, OLED_8X16, "更改完再OK");
	OLED_Printf(0, 16, OLED_8X16, "左右键选择");
	OLED_Printf(0, 32, OLED_6X8, "%d-%d-%d-%d-%d-%d", 
	SetTime_arr[0], SetTime_arr[1], SetTime_arr[2], SetTime_arr[3], SetTime_arr[4], SetTime_arr[5]);
	OLED_Printf(0, 48, OLED_8X16, "%d", SetTime_arr[time_index]);
	
}

/*
	检测设置的闹钟时间是否与当前时间相等任务进程
*/
void Alarm_Check(void)
{
	uint8_t key;	//存按键值		
	//无按键或错误					      按键为＃键						           2代表闹钟正在触发
	if(IR_GetKey(&key) == 1 && key == Key_JingHao && Alarm_setHandle.Change_Time[3] == 2)
	{
		Buzzer_SetMode(&PB14_Buzzer, Buzzer_OFF, 500, 500);
		vTask_Delete(Buzzer_Proc);//删除闹钟蜂鸣任务进程
		vTask_Delete(Alarm_Check);
		Alarm_setHandle.Change_Time[3] = 0;
	}
	
	uint16_t *SetAlarm_arr = Alarm_setHandle.alarm_clock; //拿到设置闹钟时间句柄
	RTC_ReadTime();//更新当前时间
	if(Rtctime.day == SetAlarm_arr[0] && Rtctime.hour == SetAlarm_arr[1] && Rtctime.min == SetAlarm_arr[2])
	{
		if(Alarm_setHandle.Change_Time[3] == 0) return; //当前闹钟已关闭
		Buzzer_SetMode(&PB14_Buzzer, Buzzer_Beep, 500, 500);//设置蜂鸣器状态，500ms频率蜂鸣
		Scheduler_AddTask(Buzzer_Proc, 200, 6, 1000);//创建蜂鸣器任务
	}
}

/*
	设置闹钟任务进程
*/
static void Alarm_Set(void)
{
	static uint8_t Alarm_index = 0; //选中的时间索引，0-2表示日时分 3表示开关
	uint16_t *SetAlarm_arr = Alarm_setHandle.alarm_clock; //拿到设置闹钟时间句柄
	
	OLED_Clear();
	OLED_Printf(0, 0, OLED_8X16, "day:%d", SetAlarm_arr[0]);
	OLED_Printf(0, 16, OLED_8X16, "hour:%d", SetAlarm_arr[1]);
	OLED_Printf(0, 32, OLED_8X16, "min:%d", SetAlarm_arr[2]);
	if(SetAlarm_arr[3] == 0) OLED_Printf(0, 48, OLED_8X16, "Alarm OFF");
	if(SetAlarm_arr[3] == 1) OLED_Printf(0, 48, OLED_8X16, "Alarm ON");
	OLED_ReverseArea(0, Alarm_index * 16, 128, 16);
	
	uint8_t key;	//存按键值
	if(IR_GetKey(&key) == -1) return;//无按键或错误
	switch(key)
	{
		//左键索引--
		case Key_W:
			if(Alarm_index == 0) 
				Alarm_index = 3;
			else
				Alarm_index--;
			break;
		//右键索引++
		case Key_S:
			if(Alarm_index == 3) 
				Alarm_index = 0;
			else
				Alarm_index++;
			break;
		
		case Key_XingHao:
			vTask_Delete(Alarm_Set);
			Scheduler_AddTask(Alarm_Clock_Proc, 50, 5, 1000);
			break;
		
		case Key_JingHao:
			SetAlarm_arr[Alarm_index] = 0;
			break;
		
		case Key_OK:
			if(Alarm_index != 3) break; //不是开关直接退出
			if(SetAlarm_arr[Alarm_index] == 0) 
			{
				SetAlarm_arr[Alarm_index] = 1; //如果是闹钟关的，打开
				Scheduler_AddTask(Alarm_Check, 500, 6, 500); //创建时间检测任务
			}
			else if(SetAlarm_arr[Alarm_index] == 1)
			{
				Buzzer_SetMode(&PB14_Buzzer, Buzzer_OFF, 500, 500);
				vTask_Delete(Buzzer_Proc);//删除闹钟蜂鸣任务进程
				vTask_Delete(Alarm_Check);
				SetAlarm_arr[Alarm_index] = 0; //如果是闹钟开的，关闭 
			}
			

			break;
		
		default:
			if(key > 9 && Alarm_index != 3) break;//不是数字键 且不能是开关的下标
			SetAlarm_arr[Alarm_index] = 	SetAlarm_arr[Alarm_index] * 10 + key;
			break;
	}
}
static void Buzzer_SetParam(void)
{
	uint8_t key;	//存按键值
	if(IR_GetKey(&key) == -1) return;//无按键或错误
	switch(key)
	{
		case Key_W:
			
			break;
		
		case Key_S:
			
			break;
		
		case Key_XingHao:
			vTask_Delete(Buzzer_SetParam);
			Scheduler_AddTask(Alarm_Clock_Proc, 50, 5, 1000);
			break;
		
		case Key_JingHao:
			
			break;
		
		case Key_OK:
			
			break;
	}
}


//显示函数
static void Alarm_Show(void)
{	
	//时间显示
	OLED_Clear();
	RTC_ReadTime();
	OLED_Printf(0,  0, OLED_6X8, "%d-%d-%d-%d-%d-%d", Rtctime.year, Rtctime.mon, Rtctime.day,Rtctime.hour, Rtctime.min, Rtctime.sec);
	OLED_Printf(0, 16, OLED_8X16, "Set Time");
	OLED_Printf(0, 32, OLED_8X16, "Set Alarm");
	OLED_Printf(0, 48, OLED_8X16, "Buzzing");
	
	OLED_ReverseArea(0, 16 * Alarm_setHandle.cursor + 16, 128, 16);
}


//进程
void Alarm_Clock_Proc(void)
{
	Alarm_Show();
	uint8_t key;	//存按键值
	if(IR_GetKey(&key) == -1) return;//无按键或错误
	
	switch(key)
	{
		case Key_W:
			if(Alarm_setHandle.cursor == 0)//菜单首行
				Alarm_setHandle.cursor = 2;
			else
				Alarm_setHandle.cursor--;
			break;
		
		case Key_S:
			if(Alarm_setHandle.cursor == 2)//菜单尾部
				Alarm_setHandle.cursor = 0;
			else
				Alarm_setHandle.cursor++;
			break;
		
			//星号键退出菜单销毁当前任务
		case Key_XingHao:
			vTask_Delete(Alarm_Clock_Proc);
			Scheduler_AddTask(Menu_Proc, 50, 5, 1000);//创建一级主页菜单任务;
			break;

		case Key_OK:
			menu_selector(Alarm_setHandle.cursor);
			break;
	}
	
}


