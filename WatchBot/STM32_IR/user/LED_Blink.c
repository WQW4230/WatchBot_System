#include "LED_Blink.h"
#include "remote_nec_driver.h"
#include "menu.h"
#include "OLED.h"
#include "App_Timer.h"

uint8_t LED_Blink_Flag = 0;

void Led_BlinkInit(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOC, &GPIO_InitStruct);
	GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_RESET);
}

//led闪烁状态机
void LED_Flicker_Proc(uint16_t Flicker)
{
	
	static uint8_t Blink_Flag = 0;
	static uint64_t Time = 0;
	
	if(Blink_Flag == 0)
	{
		if(App_Timer_GetTick() > Time + Flicker)
		{
			Blink_Flag = 1;
			Time = App_Timer_GetTick();
			GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_RESET);
		}
		
	}
	
	if(Blink_Flag == 1)
	{
		if(App_Timer_GetTick() > Time + Flicker)
		{
			Blink_Flag = 0;
			Time = App_Timer_GetTick();
			GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_SET);
		}
	}
	
	
}

//显示函数
void LED_Shwo(uint16_t Flicker)
{
	OLED_Printf(64, 0, OLED_8X16, "%d", Flicker);
	OLED_UpdateArea(64, 0, 32, 16);
	OLED_Clear();
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
	static uint16_t Flicker = 0;
	
	if(LED_Blink_Flag == 1)
	{
		static uint8_t Temp_Key = 0;
		if(IR_Flag == 1)
		{
			IR_Flag = 0;
			Temp_Key =  IR_GetKey();
			
			switch(Temp_Key)
			{			
				case Key_XingHao:
				{
					LED_Blink_Flag = 0;
					Menu_Flag = 1;
					OLED_Clear();
					
					break;
				}
				
				case Key_JingHao:
				{
					Flicker = 0;
				}
				
			}
			
			Led_GetKey_Num(Temp_Key, &Flicker);	
		}
		
		LED_Shwo(Flicker);
	}
	
	LED_Flicker_Proc(Flicker);
}

