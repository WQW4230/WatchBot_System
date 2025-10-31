#include "stm32f10x.h"
#include "OLED.h"
#include "Scheduler.h"

#include "Arm_menu.h"

#include "LED_Frequency.h"
#include "Alarm_Clock.h"
#include "NEC_driver.h"

#include "menu.h"

#include "rtc_driver.h" //红外
#include "GreedySnake.h"
#include "Contact.h"   //通讯录
#include "GuessMine.h" //扫雷
#include "USART_FrameHandler.h"//串口通信

int main(void)
{
	Scheduler_Init();
	OLED_Init();
	
	Led_BlinkInit();
	
	Alarm_Init();
	NEC_Init();
	SnakeGame_Init();//贪吃蛇
	Contact_Init();//通讯录顺序表初始化
	MineBoard_Init(); //扫雷初始化
	USART_FrameHandler_Init();//串口通信初始化
	
	ArmMenu_Init();//机械臂控制菜单
	
	Scheduler_AddTask(USART_FrameHandler_Task, 100, 7, 1000);
	Scheduler_AddTask(Menu_Proc, 100, 5, 1000);
	Scheduler_AddTask(OLED_UpdateStep, 7, 70, 1000);
	
		
	Scheduler_Run();
	
}
