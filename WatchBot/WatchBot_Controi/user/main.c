#include "stm32f10x.h"
#include "stm32f10x_pal.h"
#include "OLED.h"
#include "App_Timer.h"


#include "LED_Blink.h"
#include "NEC_driver.h"
#include "arm_control.h"

#include "PS2_Control_arm.h"

Key_Enum IR_testData;

int main(void)
{
	PAL_Init();
	OLED_Init();
	App_Timer_Init();
	
	NEC_Init();
	Arm_Init();
	JOY_Control_Init();
	
	LED_t LED_Conflg;
	LED_Conflg.GPIOX = GPIOC;
	LED_Conflg.GPIO_PIN = GPIO_Pin_13;
	LED_Conflg.Lofic = LED_ACTIVE_LOW;
	LED_Init(&LED_Conflg);
	
	LED_SetState(&LED_Conflg, LED_ON);
	LED_SetState(&LED_Conflg, LED_OFF);
	
	Arm_MoveTo(90, 90, 0);
	//Arm_MoveTo(-90, 90, 0);
	while(1)
	{
//		LED_Beep(&LED_Conflg, 1000);
//		App_Timer_Delay_ms(1000);
//		LED_Proc();
//		App_Timer_Delay_ms(1000);
		OLED_Clear();
		
		//PS2_Uptada();
		
		Arm_Update();
		
		OLED_Update();
		App_Timer_Delay_ms(20);
		
	}
}
