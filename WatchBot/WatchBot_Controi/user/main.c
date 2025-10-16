#include "stm32f10x.h"
#include "stm32f10x_pal.h"

#include "App_Timer.h"
#include "LED_Blink.h"
#include "motor_driver.h"
#include "NEC_driver.h"

Key_Enum IR_testData;

int main(void)
{
	PAL_Init();
	
	App_Timer_Init();
	motor_Init();
	NEC_Init();
	
	LED_t LED_Conflg;
	LED_Conflg.GPIOX = GPIOC;
	LED_Conflg.GPIO_PIN = GPIO_Pin_13;
	LED_Conflg.Lofic = LED_ACTIVE_LOW;
	LED_Init(&LED_Conflg);
	
	LED_SetState(&LED_Conflg, LED_ON);
	LED_SetState(&LED_Conflg, LED_OFF);
	
	PWM_SetCompare1(400);
	PWM_SetCompare2(2000);
	PWM_SetCompare3(2000);
	PWM_SetCompare4(0);
	
	Moto_SetState(MOTOR_FORWARD);//正转
	
	
	
	while(1)
	{
		LED_Beep(&LED_Conflg, 1000);
		App_Timer_Delay_ms(1000);
		LED_Proc();
		App_Timer_Delay_ms(1000);
			
	}
}
