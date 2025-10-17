#include "stm32f10x.h"
#include "stm32f10x_pal.h"

#include "App_Timer.h"
#include "LED_Blink.h"
#include "NEC_driver.h"
#include "arm_control.h"
#include "OLED.h"
#include "ps2_ADC_DMA.h"

Key_Enum IR_testData;

int main(void)
{
	PAL_Init();
	OLED_Init();
	App_Timer_Init();
	
	NEC_Init();
	Arm_Init();
	PS2_ADCInit();
	
	LED_t LED_Conflg;
	LED_Conflg.GPIOX = GPIOC;
	LED_Conflg.GPIO_PIN = GPIO_Pin_13;
	LED_Conflg.Lofic = LED_ACTIVE_LOW;
	LED_Init(&LED_Conflg);
	
	LED_SetState(&LED_Conflg, LED_ON);
	LED_SetState(&LED_Conflg, LED_OFF);
	
//	App_Timer_Delay_ms(4000);
	Servo_SetAngle(-90, -90, -90);
//	App_Timer_Delay_ms(4000);
//	
//	Arm_MoveTo(90, 90, 90);
	while(1)
	{
//		LED_Beep(&LED_Conflg, 1000);
//		App_Timer_Delay_ms(1000);
//		LED_Proc();
//		App_Timer_Delay_ms(1000);
		OLED_Clear();
		OLED_ShowNum(0, 0,  PS2_AD[0], 5, OLED_8X16);
		OLED_ShowNum(0, 16, PS2_AD[1], 5, OLED_8X16);
		OLED_ShowNum(0, 32, PS2_AD[2], 5, OLED_8X16);
		OLED_ShowNum(0, 48, PS2_AD[3], 5, OLED_8X16);
		OLED_Update();
		
	}
}
