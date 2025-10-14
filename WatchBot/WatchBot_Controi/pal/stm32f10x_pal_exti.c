/**
  ******************************************************************************
  * @file    stm32f10x_pal_exti.c
  * @author  铁头山羊
  * @version V 1.0.0
  * @date    2023年4月24日
  * @brief   外部中断驱动程序
  ******************************************************************************
  * @attention
  * Copyright (c) 2022 - 2023 东莞市明玉科技有限公司. 保留所有权力.
  ******************************************************************************
*/
#include "stm32f10x_pal.h"
#include "stm32f10x_pal_exti.h"

// @简介：初始化EXTI的单个通道
// @参数：Handle - EXTI单个通道的句柄的指针
// @返回值：无
void PAL_EXTI_Init(PalEXTI_HandleTypeDef *Handle)
{
	GPIO_InitTypeDef GPIOInitStruct;
	uint8_t GPIO_PortSourceGPIOx, GPIO_PinSource;
	
	
	// IO引脚设置为输入模式
	RCC_GPIOx_ClkCmd(Handle->Init.GPIOx, ENABLE);
	
	GPIOInitStruct.GPIO_Pin = Handle->Init.GPIO_Pin;
	GPIOInitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING; // 未定
	GPIO_Init(Handle->Init.GPIOx, &GPIOInitStruct);
	
	// 开启AFIO的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	
	// 配置AFIO
	GPIO_PortSourceGPIOx = ((uint32_t)Handle->Init.GPIOx - (uint32_t)GPIOA) / 0x400;
	
	for(GPIO_PinSource = 0; GPIO_PinSource <= 15; GPIO_PinSource++)
	{
		if((0x01 << GPIO_PinSource) & Handle->Init.GPIO_Pin)
		{
			break;
		}
	}
	
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOx, GPIO_PinSource);
	
	// 配置EXTI
	EXTI_InitTypeDef ExtiInitStruct;
	ExtiInitStruct.EXTI_Line = Handle->Init.GPIO_Pin;
	ExtiInitStruct.EXTI_Mode = Handle->Init.EXTI_Mode;
	ExtiInitStruct.EXTI_Trigger = Handle->Init.EXTI_Trigger;
	ExtiInitStruct.EXTI_LineCmd = ENABLE;
	EXTI_Init(&ExtiInitStruct);
}

// @简介：用于使能或禁止EXTI该通道的中断
// @参数：Handle - EXTI单个通道的句柄的指针
// @返回值：无
void PAL_EXTI_InterruptCmd(PalEXTI_HandleTypeDef *Handle, FunctionalState NewState)
{
		// 配置中断
	NVIC_InitTypeDef NVIC_InitStruct;
	if(Handle->Init.GPIO_Pin == GPIO_Pin_0)
	{
		NVIC_InitStruct.NVIC_IRQChannel = EXTI0_IRQn;
	}
	else if(Handle->Init.GPIO_Pin == GPIO_Pin_1)
	{
		NVIC_InitStruct.NVIC_IRQChannel = EXTI1_IRQn;
	}
	else if(Handle->Init.GPIO_Pin == GPIO_Pin_2)
	{
		NVIC_InitStruct.NVIC_IRQChannel = EXTI2_IRQn;
	}
	else if(Handle->Init.GPIO_Pin == GPIO_Pin_3)
	{
		NVIC_InitStruct.NVIC_IRQChannel = EXTI3_IRQn;
	}
	else if(Handle->Init.GPIO_Pin == GPIO_Pin_4)
	{
		NVIC_InitStruct.NVIC_IRQChannel = EXTI4_IRQn;
	}
	else if(Handle->Init.GPIO_Pin >= GPIO_Pin_5 && Handle->Init.GPIO_Pin <= GPIO_Pin_9)
	{
		NVIC_InitStruct.NVIC_IRQChannel = EXTI9_5_IRQn;
	}
	else if(Handle->Init.GPIO_Pin >= GPIO_Pin_10 && Handle->Init.GPIO_Pin <= GPIO_Pin_15)
	{
		NVIC_InitStruct.NVIC_IRQChannel = EXTI15_10_IRQn;
	}
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = Handle->Init.PreemptionPriority;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = Handle->Init.SubPriority;
	NVIC_InitStruct.NVIC_IRQChannelCmd = NewState;
	
	NVIC_Init(&NVIC_InitStruct);
}

// @简介：EXTI中断的处理函数
// @参数：Handle - EXTI单个通道的句柄的指针
// @返回值：无
// @注意：此函数应当在对应的中断响应函数中被调用
void PAL_EXTI_IRQHandler(PalEXTI_HandleTypeDef *Handle)
{
	EXTI_ClearITPendingBit(Handle->Init.GPIO_Pin);
	
	if(Handle->Init.CallbackFn)
	{
		Handle->Init.CallbackFn();
	}
}
