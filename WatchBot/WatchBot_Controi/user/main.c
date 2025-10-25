#include "stm32f10x.h"
#include "OLED.h"
#include "Scheduler.h"

#include "arm_control.h"
#include "PS2_Control_arm.h"

#include "LED_Frequency.h"
#include "Alarm_Clock.h"
#include "NEC_driver.h"

#include "menu.h"

#include "rtc_driver.h"

int main(void)
{
	Scheduler_Init();
	OLED_Init();
	
	Arm_Init();
	JOY_Control_Init();
	
	Led_BlinkInit();
	
	Alarm_Init();
	NEC_Init();
	//Arm_deom();
	
	Scheduler_AddTask(PS2_Uptada, 3, 4, 1000);
	Scheduler_AddTask(Arm_Update, 100, 5, 1000);
	Scheduler_AddTask(Menu_Proc, 100, 5, 1000);
	Scheduler_AddTask(OLED_UpdateStep, 7, 5, 1000);
		
	Scheduler_Run();
	
}
