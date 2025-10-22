#include "stm32f10x.h"
#include "OLED.h"
#include "Scheduler.h"

#include "arm_control.h"
#include "PS2_Control_arm.h"

#include "LED_Frequency.h"
#include "Alarm_Clock.h"
#include "NEC_driver.h"

#include "LED_Blink.h"

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
		
	Scheduler_Run();
	
}
