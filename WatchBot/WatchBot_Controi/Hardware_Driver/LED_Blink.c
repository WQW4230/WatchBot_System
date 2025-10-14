#include "LED_Blink.h"
#include "stm32f10x.h"

#define MAX_LED 4

static struct
{
	LED_t *LED;
	uint16_t remain_ms;
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
	GPIO_WriteBit(LED->GPIOX, GPIO_Pin_13, Bit_RESET);
	
	//初始化默认关闭
	LED_SetState(LED, LED_OFF);
	
	//注册进系统
	for(uint8_t i=0; i<MAX_LED; i++)
	{
		if(led_list[i].LED == 0)
		{
			led_list[i].LED = LED;
			led_list[i].remain_ms = 0;
		}
	}
}

//LED状态设置
void LED_SetState(LED_t *LED, LedState_e state)
{
	switch(LED->Lofic)
	{
		
		case LED_ACTIVE_HIGH:
			if(state == LED_ON)
			{
				GPIO_WriteBit(LED->GPIOX, LED->GPIO_PIN, Bit_SET);
			}
			else
			{
				GPIO_WriteBit(LED->GPIOX, LED->GPIO_PIN, Bit_RESET);
			}
			break;
		
		case LED_ACTIVE_LOW:
			if(state == LED_ON)
			{
				GPIO_WriteBit(LED->GPIOX, LED->GPIO_PIN, Bit_RESET);
			}
			else
			{
				GPIO_WriteBit(LED->GPIOX, LED->GPIO_PIN, Bit_SET);
			}		
	}
}

//LED亮多久
void LED_Beep(LED_t *LED, uint64_t duration)
{
	for(uint8_t i=0; i<MAX_LED; i++)
	{
		if(led_list[i].LED)
		{
			led_list[i].remain_ms += duration;
			LED_SetState(LED, LED_ON);
		}
	}
}

//进程函数放在主循环里
void LED_Proc(void)
{
	for(uint8_t i=0; i<MAX_LED; i++)
	{
		if(led_list[i].LED && led_list[i].remain_ms > 0)
		{
			if(led_list[i].remain_ms <= 1000)
			{
				LED_SetState(led_list[i].LED, LED_OFF);
				led_list[i].remain_ms = 0;
			}
			else
			{
				led_list[i].remain_ms -= 1000;
			}
		}
	}
}
