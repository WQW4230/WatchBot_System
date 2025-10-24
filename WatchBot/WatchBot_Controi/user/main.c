#include "stm32f10x.h"
#include "OLED.h"
#include "Scheduler.h"

#include "arm_control.h"
#include "PS2_Control_arm.h"

#include "LED_Frequency.h"
#include "Alarm_Clock.h"
#include "NEC_driver.h"

#include "menu.h"

uint64_t Now = 0;
uint64_t Last = 0;
uint64_t Time = 0;

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
	
	Scheduler_AddTask(PS2_Uptada, 50, 4, 1000);
	Scheduler_AddTask(Arm_Update, 50, 3, 1000);
	//Scheduler_AddTask(Menu_Proc, 100, 5, 1000);
	Scheduler_AddTask(OLED_UpdateStep, 60, 5, 1000);
		
	Scheduler_Run();
	
}
