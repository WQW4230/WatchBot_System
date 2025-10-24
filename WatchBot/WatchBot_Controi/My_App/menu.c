#include "menu.h"
#include "stm32f10x.h"
#include <stddef.h>
#include <stdlib.h>
#include "Scheduler.h"

#include "OLED.h"
#include "Scheduler.h"
#include "NEC_driver.h"

#include "LED_Frequency.h"

#include "GuessNum.h"
#include "GuessMine.h"
#include "Contact.h"
#include "Greedysnake.h"
#include "arm_control.h" 
#include "rtc_driver.h"
#include "Alarm_Clock.h"
#include "LED_Frequency.h"

Menu_t Menu_State = {0, 0, 0};//主页控制句柄

void Menu_Proc(void); //主页进程函数


menu_items app_items[] =  		//软件名称及其软件函数指针
{
	{"0通讯录", &LedSet_Menu_Proc},
	{"1贪吃蛇", &LedSet_Menu_Proc},
	{"2扫雷",   &LedSet_Menu_Proc},	
	{"3猜数字", &LedSet_Menu_Proc},	
	{"4LED调速",&LedSet_Menu_Proc},
	{"5定时器", &LedSet_Menu_Proc},
	{"6机械臂", &LedSet_Menu_Proc},	
};


void Menu_HomeShow(void)
{
	OLED_Clear();
	for(uint8_t i=0; i<APP_PAGE_SIZE; i++)
	{
		uint8_t app_index = (Menu_State.top_index + i) % APP_COUNT; //计算当前需要打印软件名称的数组位置
		OLED_Printf(0, i*20, OLED_8X16, app_items[app_index].app_name);
		
		//当前选中位置颜色取反
		if(i == Menu_State.cursor)
		{
			OLED_ReverseArea(0, i*20, 70, 20);
		}
	}
	
	//读当前时间已经移动到Alarm的时间比较函数下调用一次即可
	//RTC_ReadTime();
	OLED_Printf(70, 0, OLED_6X8, "%d-%d-%d", Rtctime.year, Rtctime.mon, Rtctime.day);
	OLED_Printf(76, 8, OLED_6X8, "%d:%d:%d", Rtctime.hour, Rtctime.min, Rtctime.sec);
}

uint8_t Menu_Flag = 1;//主页状态标志位;

//菜单选项向上
static void Option_up(void)
{
	if(Menu_State.cursor > 0)
		Menu_State.cursor--;
	else
		Menu_State.top_index = (Menu_State.top_index +APP_COUNT - 1) % APP_COUNT;	
}

//菜单选项向下
static void Option_down(void)
{
	if(Menu_State.cursor < APP_PAGE_SIZE - 1)
		Menu_State.cursor++;
	else
		Menu_State.top_index = (Menu_State.top_index + APP_COUNT + 1) % APP_COUNT;
}

//获取当前光标指向哪个软件
static void Current_option(void)
{
	Menu_State.select_app = (Menu_State.cursor + Menu_State.top_index) % APP_COUNT;
}

static void Task_creation(void)
{
	Current_option();//先更新当前光标指向的是哪个软件
	void (*fun)(void) = app_items[Menu_State.select_app].fun; //获得App函数指针
	
	vTask_Delete(Menu_Proc); //删除主页任务进程
	Scheduler_AddTask(fun, 50, 5, 1000);
}

//功能主页任务进程
void Menu_Proc(void)
{
	static uint8_t first_show = 1; //第一次进入菜单的时候显示
	
	if(first_show)
	{
		Menu_HomeShow();
	}	
	
	if(NEC_RxFlag == 0) return; //没有红外按键按下退出;
	uint8_t key = IR_GetKey();	//获取按键值
	
	switch(key)
	{
		// W键按下 菜单选项向上
		case Key_W:
			Option_up();
			break;
		
		// S键按下 菜单选项向下
		case Key_S:
			Option_down();
			break;
		// OK键按下 进入当前软件
		case Key_OK:
			Task_creation();
			break;
	}
	
	Menu_HomeShow();
}

//Random_Max：输入需要范围的随机数，返回值是0 — (Max-1)
//获取随机数
uint32_t GetRandom(uint32_t Random_Max)
{
	static uint8_t RandomInit_Flag = 0;
	if(!RandomInit_Flag)
	{
		RandomInit_Flag = 1;
		srand(TIM_GetCounter(TIM1));
	}
	return rand()% Random_Max;
}

