#include "LED_Frequency.h"
#include "LED_Blink.h"
#include "Scheduler.h"

#include "OLED.h"
#include "NEC_driver.h"

KeyOperation_t menu[6]=
{
	//第一层
	{0, 2, 1, 3, Led_BlinkInit},
	{1, 0, 2, 4, Led_BlinkInit},
	{2, 1, 0, 5, Led_BlinkInit},
	//第二层
	{3, 5, 4, 3, Led_BlinkInit},
	{4, 3, 5, 4, Led_BlinkInit},
	{5, 4, 3, 5, Led_BlinkInit},
};

//LED控制句柄
LED_t LED_PC13_Struct;

void Led_BlinkInit(void)
{
	LED_PC13_Struct.GPIOX = GPIOC;
	LED_PC13_Struct.GPIO_PIN = GPIO_Pin_13;
	LED_PC13_Struct.Lofic = LED_ACTIVE_LOW; //输出开漏接法低电平点亮
	LED_Init(&LED_PC13_Struct);
	
	LED_SetMode(&LED_PC13_Struct, LED_BLINK, 1000, 500);
	Scheduler_AddTask(LED_Proc_test, 100, 0, 1000);
}

void LED_Proc_test(void)
{
	LED_Proc();
}


//显示函数
void LED_Shwo(uint16_t Flicker)
{
	OLED_Clear();
	OLED_Printf(64, 0, OLED_8X16, "%d", Flicker);
	OLED_UpdateArea(64, 0, 32, 16);
}

//获取数字函数
void Led_GetKey_Num(uint8_t IR_Num, uint16_t *Flicker)
{
	if((IR_Num <=9))
	{
		*Flicker = *Flicker * 10 + IR_Num;
	}

}

void Led_Blink_Proc(void)
{
	if(NEC_RxFlag) return;
	
	//获取按键值
	uint8_t key = IR_GetKey();
	static uint16_t Flicker = 0;
	
	switch(key)
	{
		case Key_XingHao:
			break;
		case Key_JingHao:
			break;
		case Key_W:
			break;
		case Key_S:
			break;
		case Key_A:
			break;
		case Key_D:
			break;			
	}
	Led_GetKey_Num(key, &Flicker);
		
	LED_Shwo(Flicker);
	
}

