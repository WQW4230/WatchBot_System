#include "stm32f10x.h"
#include "OLED.h"
#include "Scheduler.h"

#include "Arm_menu.h"

#include "LED_Frequency.h"
#include "Alarm_Clock.h"
#include "NEC_driver.h"

#include "menu.h"

#include "rtc_driver.h"
#include "GreedySnake.h"
#include "Contact.h"   //通讯录
#include "GuessMine.h" //扫雷

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
	
	ArmMenu_Init();
	
	//Arm_MoveTo(90, -90, -90, 0);
	
	//Scheduler_AddTask(PS2_Uptada, 50, 4, 1000);
	Scheduler_AddTask(Menu_Proc, 100, 5, 1000);
	Scheduler_AddTask(OLED_UpdateStep, 7, 5, 1000);
	
		
	Scheduler_Run();
	
}
