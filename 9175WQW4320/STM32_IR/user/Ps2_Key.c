#include "Ps2_key.h"
#include "App_Timer.h"
#include "Alarm.h"

//按键消抖时间
#define DEBOUNCE_MS 1000

uint8_t PS2_KeyFlag = 0;

void Ps2_KeyInit(void)
{
	//GPIO初始化
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10 |GPIO_Pin_11;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	//AFIO
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource10);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource11);
	
	//EXTI 10
	EXTI_InitTypeDef EXTI_InitStruct;
	EXTI_InitStruct.EXTI_Line = EXTI_Line10;
	EXTI_InitStruct.EXTI_LineCmd = ENABLE;
	EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_Init(&EXTI_InitStruct);

	//EXTI 11	
	EXTI_InitStruct.EXTI_Line = EXTI_Line11;
	EXTI_InitStruct.EXTI_LineCmd = ENABLE;
	EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_Init(&EXTI_InitStruct);
	
	//NVIC
	NVIC_InitTypeDef NVIC_InitStruct;
	NVIC_InitStruct.NVIC_IRQChannel = EXTI15_10_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);
	
}

//中断响应函数
void EXTI15_10_IRQHandler(void)
{ 
	uint64_t now_time = App_Timer_GetTick();//当前时间
	static uint64_t lase_time_Line10;
	static uint64_t lase_time_Line11;
	
	if(EXTI_GetITStatus(EXTI_Line10) == SET)
	{
		EXTI_ClearITPendingBit(EXTI_Line10);
		if(now_time - lase_time_Line10 > DEBOUNCE_MS)
		{
			lase_time_Line10 = App_Timer_GetTick();
			PS2_KeyFlag = 1;
			Alarm_count++;//蜂鸣器句柄，响一次
		}
	}
	
	if(EXTI_GetITStatus(EXTI_Line11) == SET)
	{
		EXTI_ClearITPendingBit(EXTI_Line11);
		if(now_time - lase_time_Line11 > DEBOUNCE_MS)
		{
			lase_time_Line11 = App_Timer_GetTick();
			PS2_KeyFlag = 0;
			Alarm_count++;//蜂鸣器句柄，响一次
			
		}
	}
}





