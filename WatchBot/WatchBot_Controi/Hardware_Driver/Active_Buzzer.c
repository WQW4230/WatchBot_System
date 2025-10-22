#include "Active_Buzzer.h"
#include "App_Timer.h"
#include "stm32f10x.h"

//蜂鸣器最大数量
#define MAX_BUZZERS 4

static struct 
{
   Buzzer_t* buzzer;
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
	
	buzzer->OnTime = 0;
	buzzer->OffTime = 0;
	buzzer->Last_time = 0;
	//关闭
	Buzzer_SetState(buzzer, Buzzer_OFF);
	
	for(uint8_t i = 0; i < MAX_BUZZERS; i++)
	{
		if(buzzer_list[i].buzzer == 0)
		{
			buzzer_list[i].buzzer = buzzer;
			break;
		}
	}
	
}

/*
	控制蜂鸣器是否蜂鸣
	buzzer：蜂鸣器句柄
	state：蜂鸣器状态 选值在 @BuzzerMode_e
*/
//蜂鸣器状态设置
void Buzzer_SetState(Buzzer_t *buzzer, uint8_t state)
{
	switch(buzzer->Lofic)
	{
		case BUZZER_ACTIVE_HIGH:
			if(state == Buzzer_ON)
			{
				GPIO_WriteBit(buzzer->GPIOX, buzzer->GPIO_PIN, Bit_SET);
				buzzer->Buzzer_State = Buzzer_ON;
			}
			else
			{
				GPIO_WriteBit(buzzer->GPIOX, buzzer->GPIO_PIN, Bit_RESET);
				buzzer->Buzzer_State = Buzzer_OFF;
			}
			break;
		
		case BUZZER_ACTIVE_LOW:
			if(state == Buzzer_ON)
			{
				GPIO_WriteBit(buzzer->GPIOX, buzzer->GPIO_PIN, Bit_RESET);
				buzzer->Buzzer_State = Buzzer_ON;
			}
			else
			{
				GPIO_WriteBit(buzzer->GPIOX, buzzer->GPIO_PIN, Bit_SET);
				buzzer->Buzzer_State = Buzzer_OFF;
			}		
	}
			
}
/*
	蜂鸣器状态选择
	buzzer：蜂鸣器句柄
	Mode ： 模式选择 @BuzzerMode_e里
	OnTime：蜂鸣时间
	OffTime：停止蜂鸣时间
如果是一直关或者一直开，蜂鸣时间和停止蜂鸣时间取值值为0
*/
//蜂鸣器模式设置
void Buzzer_SetMode(Buzzer_t *buzzer, BuzzerMode_e Mode, uint32_t OnTime, uint32_t OffTime)
{
	Buzzer_t *t = buzzer;
	if(t == 0) return;
	
	switch(Mode)
	{
		case Buzzer_ON:
			t->Buzzer_Mode = Buzzer_ON;
			Buzzer_SetState(t, Buzzer_ON);
			break;
		
		case Buzzer_OFF:
			t->Buzzer_Mode = Buzzer_OFF;
			Buzzer_SetState(t, Buzzer_OFF);
			break;
		
		case Buzzer_Beep:
			t->Buzzer_Mode = Buzzer_Beep;
			t->OnTime = OnTime;
			t->OffTime = OffTime;
			break;
	}
}

//进程函数放在主循环里
void Buzzer_Proc(void)
{
	for(uint8_t i=0; i<MAX_BUZZERS; i++)
	{
		Buzzer_t *t = buzzer_list[i].buzzer;
		//结构体指针为空并且不为蜂鸣模式
		if(t == 0) continue;
		if(t->Buzzer_Mode != Buzzer_Beep) continue;
		
		switch(t->Buzzer_State)
		{
			case Buzzer_ON:
				if(system_tick > t->Last_time + t->OnTime)
				{
					t->Last_time = system_tick;
					Buzzer_SetState(t, Buzzer_OFF);
				}
				break;
			
			case Buzzer_OFF:
				if(system_tick > t->Last_time + t->OffTime)
				{
					t->Last_time = system_tick;
					Buzzer_SetState(t, Buzzer_ON);
				}
				break;
		}
	}
}

