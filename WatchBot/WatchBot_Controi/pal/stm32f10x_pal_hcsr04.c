/**
  ******************************************************************************
  * @file    stm32f10x_pal_hcsr04.c
  * @author  铁头山羊stm32工作组
  * @version V1.0.0
  * @date    2023年2月24日
  * @brief   hc-sr04驱动
  ******************************************************************************
  * @attention
  * Copyright (c) 2022 - 2023 东莞市明玉科技有限公司. 保留所有权力.
  ******************************************************************************
*/

#include "stm32f10x_pal_hcsr04.h"
#include "stm32f10x_pal.h"

void PAL_HCSR04_Init(PalHCSR04_HandleTypeDef *Handle)
{
	/* 配置Trig引脚 */
	RCC_GPIOx_ClkCmd(Handle->Init.TrigGPIOPort, ENABLE);
	
	GPIO_WriteBit(Handle->Init.TrigGPIOPort, Handle->Init.TrigGPIOPin, Bit_RESET);
	
	GPIO_InitTypeDef GPIOInitStruct;
	GPIOInitStruct.GPIO_Pin = Handle->Init.TrigGPIOPin;
	GPIOInitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIOInitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(Handle->Init.TrigGPIOPort, &GPIOInitStruct);
	
	
	/* 开启定时器的时钟 */
	if(Handle->Init.TIMx == TIM1)
	{
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
	}
	else if(Handle->Init.TIMx == TIM2)
	{
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	}
	else if(Handle->Init.TIMx == TIM3)
	{
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	}
	else if(Handle->Init.TIMx == TIM4)
	{
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
	}
	
	/* 配置时基单元 */
	uint32_t internal_clk;
	
	RCC_ClocksTypeDef RCC_Clocks;
	RCC_GetClocksFreq(&RCC_Clocks); // 获取时钟配置情况
	
	if(Handle->Init.TIMx == TIM1 
	|| Handle->Init.TIMx == TIM8 || Handle->Init.TIMx == TIM9 
	|| Handle->Init.TIMx == TIM10 || Handle->Init.TIMx == TIM11)
	{
		internal_clk = RCC_Clocks.PCLK2_Frequency;
		if(RCC_Clocks.HCLK_Frequency / RCC_Clocks.PCLK2_Frequency > 1)
		{
			internal_clk *= 2;
		}
	}
	else if(Handle->Init.TIMx == TIM2 || Handle->Init.TIMx == TIM3 || Handle->Init.TIMx == TIM4)
	{
		internal_clk = RCC_Clocks.PCLK1_Frequency;
		if(RCC_Clocks.HCLK_Frequency / RCC_Clocks.PCLK1_Frequency > 1)
		{
			internal_clk *= 2;
		}
	}
	
	TIM_TimeBaseInitTypeDef TimeBaseInitStruct;
	TimeBaseInitStruct.TIM_Prescaler = internal_clk / 1000000 - 1; // 分辨率1us
	TimeBaseInitStruct.TIM_Period = 0xffff; // 设置为最大值
	TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up; 
	TimeBaseInitStruct.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(Handle->Init.TIMx, &TimeBaseInitStruct);
	
	TIM_GenerateEvent(Handle->Init.TIMx, TIM_EventSource_Update); // 产生一个Update事件
	
	/* 配置通道1 */
	/* 将通道1的IO配置为输入下拉模式 */
	if(Handle->Init.TIMx == TIM1) 
	{
		if(Handle->Init.Advanced.Remap == 0 || Handle->Init.Advanced.Remap == 1) // 无重映射 TIM1_CH1 -> PA8
		{
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
			GPIOInitStruct.GPIO_Pin = GPIO_Pin_8;
			GPIOInitStruct.GPIO_Mode = GPIO_Mode_IPD;
			GPIO_Init(GPIOA, &GPIOInitStruct);
		}
		else if(Handle->Init.Advanced.Remap == 3)// 全部映射 TIM1_CH1 -> PE9
		{
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);
			GPIOInitStruct.GPIO_Pin = GPIO_Pin_9;
			GPIOInitStruct.GPIO_Mode = GPIO_Mode_IPD;
			GPIO_Init(GPIOE, &GPIOInitStruct);
		}
	}
	else if(Handle->Init.TIMx == TIM2) 
	{
		if(Handle->Init.Advanced.Remap == 0 || Handle->Init.Advanced.Remap == 2) // TIM2_CH1 -> PA0
		{
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
			GPIOInitStruct.GPIO_Pin = GPIO_Pin_0;
			GPIOInitStruct.GPIO_Mode = GPIO_Mode_IPD;
			GPIO_Init(GPIOA, &GPIOInitStruct);
		}
		else if(Handle->Init.Advanced.Remap == 1 || Handle->Init.Advanced.Remap == 3) // TIM2_CH1 -> PA15
		{
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
			GPIOInitStruct.GPIO_Pin = GPIO_Pin_15;
			GPIOInitStruct.GPIO_Mode = GPIO_Mode_IPD;
			GPIO_Init(GPIOA, &GPIOInitStruct);
		}
	}
	else if(Handle->Init.TIMx == TIM3) 
	{
		if(Handle->Init.Advanced.Remap == 0) // TIM3 -> PA6
		{
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
			GPIOInitStruct.GPIO_Pin = GPIO_Pin_6;
			GPIOInitStruct.GPIO_Mode = GPIO_Mode_IPD;
			GPIO_Init(GPIOA, &GPIOInitStruct);
		}
		else if(Handle->Init.Advanced.Remap == 2) // TIM3 -> PB4
		{
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
			GPIOInitStruct.GPIO_Pin = GPIO_Pin_4;
			GPIOInitStruct.GPIO_Mode = GPIO_Mode_IPD;
			GPIO_Init(GPIOB, &GPIOInitStruct);
		}
		else if(Handle->Init.Advanced.Remap == 3) // TIM3 -> PC6
		{
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
			GPIOInitStruct.GPIO_Pin = GPIO_Pin_6;
			GPIOInitStruct.GPIO_Mode = GPIO_Mode_IPD;
			GPIO_Init(GPIOC, &GPIOInitStruct);
		}
	}
	
	TIM_ICInitTypeDef TIM_ICInitStruct;
	TIM_ICInitStruct.TIM_Channel = TIM_Channel_1;
	TIM_ICInitStruct.TIM_ICFilter = 0; // 关闭滤波器
	TIM_ICInitStruct.TIM_ICPolarity = TIM_ICPolarity_Rising; // 上升沿
	TIM_ICInitStruct.TIM_ICPrescaler = TIM_ICPSC_DIV1;
	TIM_ICInitStruct.TIM_ICSelection = TIM_ICSelection_DirectTI;
	TIM_ICInit(Handle->Init.TIMx, &TIM_ICInitStruct);
	
	/* 配置通道2 */
	TIM_ICInitStruct.TIM_Channel = TIM_Channel_2;
	TIM_ICInitStruct.TIM_ICFilter = 0; // 关闭滤波器
	TIM_ICInitStruct.TIM_ICPolarity = TIM_ICPolarity_Falling; // 下降沿
	TIM_ICInitStruct.TIM_ICPrescaler = TIM_ICPSC_DIV1;
	TIM_ICInitStruct.TIM_ICSelection = TIM_ICSelection_IndirectTI;
	TIM_ICInit(Handle->Init.TIMx, &TIM_ICInitStruct);
	
	/* 配置触发 */
	
	TIM_SelectInputTrigger(Handle->Init.TIMx, TIM_TS_TI1FP1);// Tirgger = TI1FP1
	TIM_SelectSlaveMode(Handle->Init.TIMx, TIM_SlaveMode_Reset); 
	
	Handle->LastTriggerTime = PAL_INVALID_TICK; // 无效时间
}

void PAL_HCSR04_Trigger(PalHCSR04_HandleTypeDef *Handle)
{
	// 清除通道2标志位
	TIM_ClearFlag(Handle->Init.TIMx, TIM_FLAG_CC2OF);
	TIM_ClearFlag(Handle->Init.TIMx, TIM_FLAG_CC2);
	
	// 向Trig引脚发送10us的脉冲
	GPIO_WriteBit(Handle->Init.TrigGPIOPort, Handle->Init.TrigGPIOPin, Bit_SET);
	TIM_GenerateEvent(Handle->Init.TIMx, TIM_EventSource_Update);
	TIM_Cmd(Handle->Init.TIMx, ENABLE); // 开启定时器，开始计时10us
	while(TIM_GetCounter(Handle->Init.TIMx) < 10);
	GPIO_WriteBit(Handle->Init.TrigGPIOPort, Handle->Init.TrigGPIOPin, Bit_RESET);
	
	Handle->LastTriggerTime = PAL_GetTick();
}

FlagStatus PAL_HCSR04_IsBusy(PalHCSR04_HandleTypeDef *Handle)
{
	if(Handle->LastTriggerTime == PAL_INVALID_TICK) // 第一次使用
	{
		return RESET;
	}
	
	if(TIM_GetFlagStatus(Handle->Init.TIMx, TIM_FLAG_CC2) == SET) // 数据采集完成
	{
		return RESET; 
	}
	
	if(Handle->LastTriggerTime + 38 < PAL_GetTick())
	{
		return RESET; // 已过最大采样时间，距离为无限远
	}
	
	return SET; // 采集进行中
}

float PAL_HCSR04_Read(PalHCSR04_HandleTypeDef *Handle)
{	
	float ret = 3.402823E38; // 无限远
	if(TIM_GetFlagStatus(Handle->Init.TIMx, TIM_FLAG_CC2) == SET)
	{
		uint16_t cc2reg = TIM_GetCapture2(Handle->Init.TIMx);// 读出CC2的值
		// 计算距离
		if(cc2reg < 38000)
		{
			ret = 340.0 / 2 / 1000 / 1000 * cc2reg * 100; // 单位：厘米
		}
	}
	return ret;
}
