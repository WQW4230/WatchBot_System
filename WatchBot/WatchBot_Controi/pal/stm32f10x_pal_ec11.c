/**
  ******************************************************************************
  * @file    stm32f10x_pal_ec11.c
  * @author  铁头山羊stm32工作组
  * @version V1.0.0
  * @date    2022年12月18日
  * @brief   ec11编码器驱动
  ******************************************************************************
  * @attention
  * Copyright (c) 2022 东莞市明玉科技有限公司. 保留所有权力.
  ******************************************************************************
	
  ==============================================================================
	                                  主要功能
  ==============================================================================
	EC11模块内部维护一个int32_t计数器，顺时针旋转时计数器的值增加，逆时针旋转时计数
	器的值减小
	1. 支持使用任意两个IO引脚驱动EC11，两个引脚分别用于连接EC11的A相和B相
	2. 支持EC11消抖功能
	3. 内部维护一个有符号32位计数器，用来记录编码器的当前值
	4. 允许设置一个回调函数，当计数器的值发生变化时触发该回调函数
	
  ==============================================================================
	                                  使用方法
  ==============================================================================
	1. 初始化EC11驱动
	  1.1. 声明一个句柄，例如：EC11_HandleTypeDef hEC11;
		1.2. 选择用于驱动CLK的引脚，使用hEC11.CLK_GPIO_Port和hEC11.CLK_GPIO_Pin
		1.3. 选择用于驱动DT的引脚，使用hEC11.DT_GPIO_Port和hEC11.DT_GPIO_Pin
		1.4. 由于CLK引脚需要使用外部中断EXTI，所以使用hEC11.EXTI_PreemptionPriority和 
		     hEC11.EXTI_SubPriority设置外部中断的中断优先级
		1.5. 设置回调函数hEC11.ValueChangedCallback，当旋转编码器的值发生改变的时候该
		     回调函数被调用
		1.6. 使用hEC11.CounterMin和hEC11.CounterMax设置计数器的范围。比如，计数器的范
		     围是0~100，那么可以这样设置：
				     hEC11.CounterMin = 0;
						 hEC11.CounterMax = 100; 
		1.7. 调用PAL_EC11_Init进行初始化，例如：PAL_EC11_Init(&hEC11);
				 
	2. 需要在对应的中断响应函数里调用PAL_EC11_EXTI_IRQHandler。
	   引脚编号和中断响应函数的对应关系：
		   CLK(A)使用引脚0 -> void EXTI0_IRQHandler(void)
			 CLK(A)使用引脚1 -> void EXTI1_IRQHandler(void)
			 CLK(A)使用引脚2 -> void EXTI2_IRQHandler(void)
			 CLK(A)使用引脚3 -> void EXTI3_IRQHandler(void)
			 CLK(A)使用引脚4 -> void EXTI4_IRQHandler(void)
			 
			 CLK(A)使用引脚5~引脚9 -> void EXTI9_5_IRQHandler(void)
			 CLK(A)使用引脚15~引脚10 -> void EXTI15_10_IRQHandler(void)
		
		 以使用PA0驱动CLK(A)为例，代码写在stm32f10x_it.c中，代码如下：
		   ...
			 #include "stm32f10x_pal.h"

       extern EC11_HandleTypeDef hEC11;

       void EXTI0_IRQHandler(void)
       {
	       PAL_EC11_EXTI_IRQHandler(&hEC11);
       }
			 
	3. 在主程序的while循环中调用任务进程，PAL_EC11_Proc。比如：
	   while(1)
		 {
		   ...
		   PAL_EC11_Proc(&hEC11);
			 ...
		 }
		 
	4. 可以使用以下方法对EC11的计数器进行操作：
	   PAL_EC11_GetCurrentValue - 获取计数器的当前值
		 PAL_EC11_Reset - 对计数器清零
		 
	5. 如果要使用EC11的按钮功能，请使用"stm32f10x_pal_button.h"按钮驱动
*/
#include "stm32f10x_pal.h"
#include "stm32f10x_pal_ec11.h"

#define MIN_TRIGGER_INTERVAL 100

#define ReadA() (BitAction)GPIO_ReadInputDataBit(Handle->Init.CLK_GPIO_Port, Handle->Init.CLK_GPIO_Pin)
#define ReadB() (BitAction)GPIO_ReadInputDataBit(Handle->Init.DT_GPIO_Port, Handle->Init.DT_GPIO_Pin)

static uint8_t GPIOPort_2_GPIOPortSource(GPIO_TypeDef *GPIOx);
static uint8_t GPIOPin_2_GPIOPinSource(uint16_t GPIO_Pin);
static IRQn_Type GPIOPin_2_EXTIIRQn(uint16_t GPIO_Pin);
static uint32_t GPIOPin_2_EXTILine(uint16_t GPIO_Pin);

/*
 * @简介：用于EC11模块的初始化
 * @参数：
 *       Handle - EC11模块句柄，具体介绍请参照该结构体的定义
 * @返回值：
 *       SUCCESS - 成功
 *       ERROR - 发生错误
 */
ErrorStatus PAL_EC11_Init(PalEC11_HandleTypeDef *Handle)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	NVIC_InitTypeDef NVIC_InitStruct;
	EXTI_InitTypeDef EXTI_InitStruct;
	
	// 配置A引脚 
	// IO模式 = 输入浮空
	RCC_GPIOx_ClkCmd(Handle->Init.CLK_GPIO_Port, ENABLE);
	GPIO_InitStruct.GPIO_Pin = Handle->Init.CLK_GPIO_Pin;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(Handle->Init.CLK_GPIO_Port, &GPIO_InitStruct);
	
	// 配置B引脚
	// IO模式 = 输入浮空
	RCC_GPIOx_ClkCmd(Handle->Init.DT_GPIO_Port, ENABLE);
	GPIO_InitStruct.GPIO_Pin = Handle->Init.DT_GPIO_Pin;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(Handle->Init.DT_GPIO_Port, &GPIO_InitStruct);
	
	// 配置A引脚为外部中断，双边沿触发
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE); // 使能AFIO的时钟
	GPIO_EXTILineConfig(GPIOPort_2_GPIOPortSource(Handle->Init.CLK_GPIO_Port),  GPIOPin_2_GPIOPinSource(Handle->Init.CLK_GPIO_Pin));
	
	EXTI_InitStruct.EXTI_Line = GPIOPin_2_EXTILine(Handle->Init.CLK_GPIO_Pin);
	EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	EXTI_InitStruct.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStruct);
	
	EXTI_ClearFlag(GPIOPin_2_EXTILine(Handle->Init.CLK_GPIO_Pin));
	
	NVIC_InitStruct.NVIC_IRQChannel = GPIOPin_2_EXTIIRQn(Handle->Init.CLK_GPIO_Pin);
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = Handle->Init.EXTI_PreemptionPriority;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = Handle->Init.EXTI_SubPriority;
	NVIC_Init(&NVIC_InitStruct);
	
	// 复位当前计数值
	Handle->PreviousValue = 0;
	Handle->CurrentValue = 0;
	Handle->PreviousCLKBitVal = ReadA();
	Handle->PreiousChangeTime = 0;
	
	return SUCCESS;
}

/*
 * @简介：EC11进程
 * @参数：
 *       Handle - EC11模块句柄，具体介绍请参照该结构体的定义
 * @注意：该方法一般在main函数的while循环中调用
 */
void PAL_EC11_Proc(PalEC11_HandleTypeDef *Handle)
{
	int32_t currentValCpy = Handle->CurrentValue;
	uint64_t currentTimeCpy = PAL_GetTick();
	if(currentValCpy != Handle->PreviousValue) // EC11的当前值发生改变
	{
		if(currentTimeCpy > Handle->PreiousChangeTime + MIN_TRIGGER_INTERVAL)
		{
			if(Handle->Init.ValueChangedCallback != 0)
			{
				Handle->Init.ValueChangedCallback(currentValCpy, Handle->PreviousValue);
			}
			
			Handle->PreviousValue = currentValCpy;
			Handle->PreiousChangeTime = currentTimeCpy;
		}
	}
}

/*
 * @简介：获取计数器的当前值
 * @参数：
 *       Handle - EC11模块句柄，具体介绍请参照该结构体的定义
 * @返回值：计数器的当前值
 */
int32_t PAL_EC11_GetCurrentValue(PalEC11_HandleTypeDef *Handle)
{
	return Handle->CurrentValue;
}

/*
 * @简介：对计数器进行复位，复位后计数器的值等于0
 * @参数：
 *       Handle - EC11模块句柄，具体介绍请参照该结构体的定义
 */
void PAL_EC11_Reset(PalEC11_HandleTypeDef *Handle)
{
	
	Handle->CurrentValue = 0;
	
	if(Handle->PreviousValue != Handle->CurrentValue)
	{
			if(Handle->Init.ValueChangedCallback != 0)
			{
				Handle->Init.ValueChangedCallback(0, Handle->PreviousValue);
			}
			Handle->PreviousValue = 0;
	}
}

/*
 * @简介：CLK引脚的外部中断处理函数
 * @参数：
 *       Handle - EC11模块句柄，具体介绍请参照该结构体的定义
 * @注意：该方法需要在中断响应函数(EXTIx_IRQHandler)里调用
 */
void PAL_EC11_EXTI_IRQHandler(PalEC11_HandleTypeDef *Handle)
{
	uint32_t EXTILine = GPIOPin_2_EXTILine(Handle->Init.CLK_GPIO_Pin);
	BitAction clk_bit_val, dt_bit_val;
	
	if(EXTI_GetITStatus(EXTILine) == SET)
	{
		EXTI_ClearITPendingBit(EXTILine); // 清除中断挂起标志位
		
		clk_bit_val = ReadA();
		
		if(Handle->PreviousCLKBitVal != clk_bit_val) // 如果状态发生了改变，非毛刺
		{
			Handle->PreviousCLKBitVal = clk_bit_val;
			
			dt_bit_val = ReadB();
			
			if(dt_bit_val != clk_bit_val) // 顺时针转
			{
				if(Handle->CurrentValue < Handle->Init.CounterMax)
				{
					Handle->CurrentValue++;
				}
			}
			else // 逆时针转
			{
				if(Handle->CurrentValue > Handle->Init.CounterMin)
				{
					Handle->CurrentValue--;
				}
			}
		}
	}
}

static uint8_t GPIOPort_2_GPIOPortSource(GPIO_TypeDef *GPIOx)
{
	uint8_t GPIO_PortSource = 0;
	if(GPIOx == GPIOA)
	{
		GPIO_PortSource = GPIO_PortSourceGPIOA;
	}
	else if(GPIOx == GPIOB)
	{
		GPIO_PortSource = GPIO_PortSourceGPIOB;
	}
	else if(GPIOx == GPIOC)
	{
		GPIO_PortSource = GPIO_PortSourceGPIOC;
	}
	else if(GPIOx == GPIOD)
	{
		GPIO_PortSource = GPIO_PortSourceGPIOD;
	}
	else if(GPIOx == GPIOE)
	{
		GPIO_PortSource = GPIO_PortSourceGPIOE;
	}
	else if(GPIOx == GPIOF)
	{
		GPIO_PortSource = GPIO_PortSourceGPIOF;
	}
	else if(GPIOx == GPIOG)
	{
		GPIO_PortSource = GPIO_PortSourceGPIOG;
	}
	
	return GPIO_PortSource;
}

static uint8_t GPIOPin_2_GPIOPinSource(uint16_t GPIO_Pin)
{
	uint8_t GPIO_PinSource = 0;
	switch(GPIO_Pin)
	{
		case GPIO_Pin_0: GPIO_PinSource = GPIO_PinSource0; break;
		case GPIO_Pin_1: GPIO_PinSource = GPIO_PinSource1; break;
		case GPIO_Pin_2: GPIO_PinSource = GPIO_PinSource2; break;
		case GPIO_Pin_3: GPIO_PinSource = GPIO_PinSource3; break;
		case GPIO_Pin_4: GPIO_PinSource = GPIO_PinSource4; break;
		case GPIO_Pin_5: GPIO_PinSource = GPIO_PinSource5; break;
		case GPIO_Pin_6: GPIO_PinSource = GPIO_PinSource6; break;
		case GPIO_Pin_7: GPIO_PinSource = GPIO_PinSource7; break;
		case GPIO_Pin_8: GPIO_PinSource = GPIO_PinSource8; break;
		case GPIO_Pin_9: GPIO_PinSource = GPIO_PinSource9; break;
		case GPIO_Pin_10: GPIO_PinSource = GPIO_PinSource10; break;
		case GPIO_Pin_11: GPIO_PinSource = GPIO_PinSource11; break;
		case GPIO_Pin_12: GPIO_PinSource = GPIO_PinSource12; break;
		case GPIO_Pin_13: GPIO_PinSource = GPIO_PinSource13; break;
		case GPIO_Pin_14: GPIO_PinSource = GPIO_PinSource14; break;
		case GPIO_Pin_15: GPIO_PinSource = GPIO_PinSource15; break;
	}
	return GPIO_PinSource;
}

static IRQn_Type GPIOPin_2_EXTIIRQn(uint16_t GPIO_Pin)
{
	IRQn_Type result = EXTI0_IRQn;
	
	switch(GPIO_Pin)
	{
		case GPIO_Pin_0: result = EXTI0_IRQn; break;
		case GPIO_Pin_1: result = EXTI1_IRQn; break;
		case GPIO_Pin_2: result = EXTI2_IRQn; break;
		case GPIO_Pin_3: result = EXTI3_IRQn; break;
		case GPIO_Pin_4: result = EXTI4_IRQn; break;
		default:
			if(GPIO_Pin >= GPIO_Pin_5 && GPIO_Pin <= GPIO_Pin_9)
			{
				result = EXTI9_5_IRQn;
			}
			else
			{
				result = EXTI15_10_IRQn;
			}
			break;
	}
	
	return result;
}

static uint32_t GPIOPin_2_EXTILine(uint16_t GPIO_Pin)
{
	uint32_t result = EXTI_Line0;
	
	switch(GPIO_Pin)
	{
		case GPIO_Pin_0: result = EXTI_Line0; break;
		case GPIO_Pin_1: result = EXTI_Line1; break;
		case GPIO_Pin_2: result = EXTI_Line2; break;
		case GPIO_Pin_3: result = EXTI_Line3; break;
		case GPIO_Pin_4: result = EXTI_Line4; break;
		case GPIO_Pin_5: result = EXTI_Line5; break;
		case GPIO_Pin_6: result = EXTI_Line6; break;
		case GPIO_Pin_7: result = EXTI_Line7; break;
		case GPIO_Pin_8: result = EXTI_Line8; break;
		case GPIO_Pin_9: result = EXTI_Line9; break;
		case GPIO_Pin_10: result = EXTI_Line10; break;
		case GPIO_Pin_11: result = EXTI_Line11; break;
		case GPIO_Pin_12: result = EXTI_Line12; break;
		case GPIO_Pin_13: result = EXTI_Line13; break;
		case GPIO_Pin_14: result = EXTI_Line14; break;
		case GPIO_Pin_15: result = EXTI_Line15; break;
	}
	
	return result;
}
