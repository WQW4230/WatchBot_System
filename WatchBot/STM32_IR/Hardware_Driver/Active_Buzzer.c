#include "Active_Buzzer.h"
#include "stm32f10x.h"

//蜂鸣器最大数量
#define MAX_BUZZERS 4

static struct 
	{
    Buzzer_t* buzzer;
    uint16_t remain_ms;
} buzzer_list[MAX_BUZZERS] = {0};

//蜂鸣器初始化
void Buzzer_Init(Buzzer_t *buzzer)
{
	
	//这里需手动分配那个总线上的引脚
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Pin = buzzer->GPIO_PIN;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(buzzer->GPIOX, &GPIO_InitStruct);
	
	//关闭
	Buzzer_SetState(buzzer, Buzzer_OFF);
	
	for(uint8_t i = 0; i < MAX_BUZZERS; i++)
	{
		if(buzzer_list[i].buzzer == 0)
		{
			buzzer_list[i].buzzer = buzzer;
			buzzer_list[i].remain_ms = 0;
		}
	}
	
}

//蜂鸣器状态设置
void Buzzer_SetState(Buzzer_t *buzzer, BuzzerState_e state)
{
	switch(buzzer->Lofic)
	{
		
		case BUZZER_ACTIVE_HIGH:
			if(state == Buzzer_ON)
			{
				GPIO_WriteBit(buzzer->GPIOX, buzzer->GPIO_PIN, Bit_SET);
			}
			else
			{
				GPIO_WriteBit(buzzer->GPIOX, buzzer->GPIO_PIN, Bit_RESET);
			}
			break;
		
		case BUZZER_ACTIVE_LOW:
			if(state == Buzzer_ON)
			{
				GPIO_WriteBit(buzzer->GPIOX, buzzer->GPIO_PIN, Bit_RESET);
			}
			else
			{
				GPIO_WriteBit(buzzer->GPIOX, buzzer->GPIO_PIN, Bit_SET);
			}		
	}
			
}

//蜂鸣器响多久
void Buzzer_Beep(Buzzer_t *buzzer, uint64_t duration)
{
	for(uint8_t i=0; i<MAX_BUZZERS; i++)
	{
		if(buzzer_list[i].buzzer == buzzer)
		{
			buzzer_list[i].remain_ms = duration;
			Buzzer_SetState(buzzer, Buzzer_ON);
		}
	}
}

//进程函数放在主循环里
void Buzzer_Proc(void)
{
	for(uint8_t i=0; i<MAX_BUZZERS; i++)
	{
		if(buzzer_list[i].buzzer && buzzer_list[i].remain_ms > 0)
		{
			if(buzzer_list[i].remain_ms <= 10)
			{
				buzzer_list[i].remain_ms = 0;
				Buzzer_SetState(buzzer_list[i].buzzer, Buzzer_OFF);
			}
			else
			{
				buzzer_list[i].remain_ms -= 10; //该值填每次多久执行一次这个函数
			}
			
		}
	}
}

