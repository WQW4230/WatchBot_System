#include "serov_driver.h"

void Arm_Init(void)
{
	//初始化GPIO使能PA012  三路舵机
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 ;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	//初始化GPIO使能PB1 一路直流电机
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	//初始化GPIO电机驱动H桥高低引脚转换
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	
	//开启定时器2时钟 定时器3
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	
	TIM_InternalClockConfig(TIM2);
	TIM_InternalClockConfig(TIM3);
	
	//配置定时器2 三路PWM舵机
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
	TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStruct.TIM_Period = 20000 - 1;		//ARR
	TIM_TimeBaseInitStruct.TIM_Prescaler = 72 - 1;	//PSC
	TIM_TimeBaseInitStruct.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStruct);
	
	//配置定时器3 一路直流130电机
	TIM_TimeBaseInitStruct.TIM_Prescaler = 72 - 1;//分频器
	TIM_TimeBaseInitStruct.TIM_Period = 1000 - 1; //自动重装寄存器
	TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStruct);
	
	//更新事件
	TIM_GenerateEvent(TIM2, TIM_EventSource_Update);
	TIM_GenerateEvent(TIM3, TIM_EventSource_Update);
	
	//三路舵机输出比较通道配置
	TIM_OCInitTypeDef TIM_OCInitStruct;
	TIM_OCStructInit(&TIM_OCInitStruct); //赋初始化值给成员防止更换定时器报错
	TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStruct.TIM_Pulse = 0;   //CCR寄存器值
	
	//一路直流电机输出比较通道配置
	TIM_OCStructInit(&TIM_OCInitStruct);
	TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStruct.TIM_Pulse = 0;		//CCR
	
	
	TIM_OC1Init(TIM2, &TIM_OCInitStruct);//OC1通道配置
	TIM_OC2Init(TIM2, &TIM_OCInitStruct);//OC2通道配置
	TIM_OC3Init(TIM2, &TIM_OCInitStruct);//OC3通道配置
	TIM_OC3Init(TIM3, &TIM_OCInitStruct);//定时器3 直流电机通道
	
	TIM_Cmd(TIM2, ENABLE);
	TIM_Cmd(TIM3, ENABLE);
	
	//TIM3定时器，与微秒计时器共用————PWM直流舵机 频率1kHz
	//TIM_OC2Init(TIM3, &TIM_OCInitStruct);
}

void PWM_SetCompare1(uint16_t Compare)
{
	TIM_SetCompare1(TIM2, Compare);
}

void PWM_SetCompare2(uint16_t Compare)
{
	TIM_SetCompare2(TIM2, Compare);
}

void PWM_SetCompare3(uint16_t Compare)
{
	TIM_SetCompare3(TIM2, Compare);
}

void PWM_SetCompare4(uint16_t Compare)
{
	TIM_SetCompare3(TIM3, Compare);
}
