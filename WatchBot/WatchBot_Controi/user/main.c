#include "stm32f10x.h"
#include "stm32f10x_pal.h"

#include "App_Timer.h"
#include "LED_Blink.h"
#include "motor_driver.h"

int main(void)
{
	PAL_Init();
	
	App_Timer_Init();
	motor_Init();
	
	
	LED_t LED_Conflg;
	LED_Conflg.GPIOX = GPIOC;
	LED_Conflg.GPIO_PIN = GPIO_Pin_13;
	LED_Conflg.Lofic = LED_ACTIVE_LOW;
	LED_Init(&LED_Conflg);
	
	LED_SetState(&LED_Conflg, LED_ON);
	LED_SetState(&LED_Conflg, LED_OFF);
	
	PWM_SetCompare1(100);
	//PWM_SetCompare2(1000);
	PWM_SetCompare3(1000);
	PWM_SetCompare4(1000);
	
	Moto_SetState(MOTOR_FORWARD);
	
	
	while(1)
	{
		LED_Beep(&LED_Conflg, 1000);
		App_Timer_Delay_ms(1000);
		LED_Proc();
		App_Timer_Delay_ms(1000);
		
	}
}
