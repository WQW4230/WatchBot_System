#include "App_Timer.h"

static volatile uint64_t currentTick = 0;

void App_Timer_Init(void)
{
	//复位该时钟配置
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_TIM3, ENABLE);
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_TIM3, DISABLE);
	
	//使能TIM3时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	
	//使能ARR寄存器预加载特性
	TIM_ARRPreloadConfig(TIM3, ENABLE);
		
	//通用定时器只需设置这些
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
	TIM_TimeBaseInitStruct.TIM_Prescaler = 72 - 1;//分频器
	TIM_TimeBaseInitStruct.TIM_Period = 1000 - 1; //自动重装寄存器
	TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStruct);
	
	//PWM电机
	TIM_OCInitTypeDef TIM_OCInitStructure;
	TIM_OCStructInit(&TIM_OCInitStructure);
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 0;		//CCR
	TIM_OC3Init(TIM3, &TIM_OCInitStructure);
	
	TIM_GenerateEvent(TIM3, TIM_EventSource_Update);//产生事件把影子寄存器值更新到寄存器中
	
	//使能Update中断
	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
	
	NVIC_InitTypeDef NVIC_InitStruct;
	NVIC_InitStruct.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;//抢占
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);
	
	//使能定时器
	TIM_Cmd(TIM3, ENABLE);	
}

//中断函数
void TIM3_IRQHandler(void)
{
	if(TIM_GetFlagStatus(TIM3, TIM_FLAG_Update))
	{
		TIM_ClearFlag(TIM3, TIM_FLAG_Update);
		currentTick++;
	}
}

//返回当前时刻 单位毫秒
uint64_t App_Timer_GetTick(void)
{
	return currentTick;
}

void App_Timer_Delay_ms(uint32_t ms)
{
	uint64_t expireTime = App_Timer_GetTick() + ms;
	while(App_Timer_GetTick() < expireTime);
}

//返回当前时刻 单位微秒
uint64_t App_Timer_GetUs(void)
{
	return currentTick * 1000 + TIM_GetCounter(TIM3);
}
void App_Timer_Delay_us(uint32_t us)
{
	uint64_t expireTime = App_Timer_GetUs() + us;
	while(App_Timer_GetUs() < expireTime);
}







