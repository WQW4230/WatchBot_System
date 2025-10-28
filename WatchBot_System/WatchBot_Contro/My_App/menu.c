#include "menu.h"
#include "stm32f10x.h"
#include "Scheduler.h"
#include "rtc_driver.h"

#include "OLED.h"
#include "Scheduler.h"
#include "NEC_driver.h"

static Menu_t Menu_State = {0, 0, 0};//主页控制句柄

extern void ArmMenu_Proc(void);
extern void Alarm_Clock_Proc(void);
extern void LedSet_Menu_Proc(void);
extern void GreedySnake_Proc(void);
extern void Contact_Proc(void);
extern void GuessMine_Proc(void);
extern void GuessNum_Proc(void);

menu_items app_items[] =  		//软件名称及其软件函数指针
{
	{"0机械臂",    &ArmMenu_Proc},
	{"1定时器",    &Alarm_Clock_Proc},
	{"2LED调速",   &LedSet_Menu_Proc},	
	{"3贪吃蛇",    &GreedySnake_Proc},	
	{"4通讯录",    &Contact_Proc},
	{"5扫雷",      &GuessMine_Proc},
	{"6猜数字",    &GuessNum_Proc},	
};

//主页屏幕刷新
static void Menu_HomeShow(void)
{
	OLED_Clear();
	for(uint8_t i=0; i<APP_PAGE_SIZE; i++)
	{
		uint8_t app_index = (Menu_State.top_index + i) % APP_COUNT; //计算当前需要打印软件名称的数组位置
		OLED_Printf(0, i*20, OLED_8X16, app_items[app_index].app_name);
		
		//当前选中位置颜色取反
		if(i == Menu_State.cursor)
		{
			OLED_ReverseArea(0, i*20, 63, 20);
		}
	}
	
	//时间显示
	//读当前时间已经移动到Alarm的时间比较函数下调用一次即可
	RTC_ReadTime();
	OLED_Printf(63, 0, OLED_8X16, "%d年", Rtctime.year);
	OLED_Printf(63, 16, OLED_8X16, "%d月%d日", Rtctime.mon, Rtctime.day);
	OLED_Printf(63, 32, OLED_8X16, "%d:%d:%d", Rtctime.hour, Rtctime.min, Rtctime.sec);
}

//菜单选项向上
static void Option_up(void)
{
	if(Menu_State.cursor > 0)
		Menu_State.cursor--;
	else
		Menu_State.top_index = (Menu_State.top_index + APP_COUNT - 1) % APP_COUNT;	
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

//菜单任务选中创建
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
	Menu_HomeShow();

	uint8_t key;	//存按键值
	if(IR_GetKey(&key) == -1) return;//无按键或错误
	
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

}



