#include "LED_Blink.h"
#include "App_Timer.h"
#include "stm32f10x.h"

#define MAX_LED 4

static struct
{
	LED_t *LED;
	
}led_list[MAX_LED] = {0};

//LED初始化----需手动分配总线对应的引脚
void LED_Init(LED_t *LED)
{
	//注意这里需要设置LED挂在那个总线上的引脚
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_InitStruct.GPIO_Pin = LED->GPIO_PIN;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(LED->GPIOX, &GPIO_InitStruct);
	
	LED->Brigh_time = 0;
	LED->Dark_time  = 0;
	LED->Last_time  = 0;
	
	//初始化默认关闭
	LED_SetState(LED, LED_OFF);
	
	//注册进系统
	for(uint8_t i=0; i<MAX_LED; i++)
	{
		if(led_list[i].LED == 0)
		{
			led_list[i].LED = LED;
			break;
		}
	}
}

//LED状态设置
void LED_SetState(LED_t *LED, uint8_t state)
{
	switch(LED->Lofic)
	{
		
		case LED_ACTIVE_HIGH:
			if(state == LED_ON)
			{
				GPIO_WriteBit(LED->GPIOX, LED->GPIO_PIN, Bit_SET);
				LED->LED_State = LED_ON;
			}
			else
			{
				GPIO_WriteBit(LED->GPIOX, LED->GPIO_PIN, Bit_RESET);
				LED->LED_State = LED_OFF;
			}
			break;
		
		case LED_ACTIVE_LOW:
			if(state == LED_ON)
			{
				GPIO_WriteBit(LED->GPIOX, LED->GPIO_PIN, Bit_RESET);
				LED->LED_State = LED_ON;
			}
			else
			{
				GPIO_WriteBit(LED->GPIOX, LED->GPIO_PIN, Bit_SET);
				LED->LED_State = LED_OFF;
			}		
	}
}

//LED模式设置
void LED_SetMode(LED_t *LED, LedMode_e Mode, uint32_t Brigh_time, uint32_t Dark_time)
{
	LED_t *t = LED;
	if(t == 0) return;
	
	switch(Mode)
	{
		case LED_OFF:
			t->LED_Mode = LED_OFF;
			LED_SetState(t, LED_OFF);
			break;	
		
		case LED_ON:
			t->LED_Mode = LED_ON;
			LED_SetState(t, LED_ON);
			break;
		
		case LED_BLINK:
			t->LED_Mode = LED_BLINK;
			t->Brigh_time = Brigh_time;
			t->Dark_time  = Dark_time ;
			break;
	}
}

//进程函数放在主循环里
void LED_Proc(void)
{
	for(uint8_t i=0; i<MAX_LED; i++)
	{
		LED_t *t = led_list[i].LED;
		
		//led句柄不为空且为闪烁模式
		if(t == 0) continue;
		if(t->LED_Mode != LED_BLINK) continue;
		
		switch(t->LED_State)
		{
			case LED_OFF:
				if(system_tick >= t->Last_time + t->Dark_time)
				{
					t->Last_time = system_tick;
					LED_SetState(t, LED_ON);
				}
				break;
			
			case LED_ON:
				if(system_tick >= t->Last_time + t->Brigh_time)
				{
					t->Last_time = system_tick;
					LED_SetState(t, LED_OFF);
				}
				break;
		
		}
	}
}
