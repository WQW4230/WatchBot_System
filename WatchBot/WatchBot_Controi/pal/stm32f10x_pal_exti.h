/**
  ******************************************************************************
  * @file    stm32f10x_pal_exti.h
  * @author  铁头山羊
  * @version V 1.0.0
  * @date    2023年4月24日
  * @brief   外部中断驱动程序
  ******************************************************************************
  * @attention
  * Copyright (c) 2022 - 2023 东莞市明玉科技有限公司. 保留所有权力.
  ******************************************************************************
*/
#ifndef _STM32F10x_PAL_EXTI_H_
#define _STM32F10x_PAL_EXTI_H_

#include "stm32f10x.h"
#include "stm32f10x_exti.h"

typedef struct
{
	GPIO_TypeDef *GPIOx; // 所选引脚的端口号，x可以取A..G
	uint16_t GPIO_Pin; // 所选引脚的引脚编号，可以是 @GPIO_Pin_0到 @GPIO_Pin_15中的一个
	EXTIMode_TypeDef EXTI_Mode; // EXTI模式，可选以下值之一：
	                            //         @EXTI_Mode_Interrupt - 中断模式
	                            //         @EXTI_Mode_Event - 事件模式
	EXTITrigger_TypeDef EXTI_Trigger; // EXTI触发方式，可选以下值之一：
	                                  //   @EXTI_Trigger_Rising - 上升沿触发
	                                  //   @EXTI_Trigger_Falling - 下降沿触发
	                                  //   @EXTI_Trigger_Rising_Falling - 双鞭岩触发
	uint8_t PreemptionPriority; // 所产生的中断源的抢占优先级
	uint8_t SubPriority; // 所产生的中断源的子优先级
	void (*CallbackFn)(void); // 回调函数，中断触发时被调用
}PalEXTI_InitTypeDef;

typedef struct
{
	PalEXTI_InitTypeDef Init;
}PalEXTI_HandleTypeDef;

void PAL_EXTI_Init(PalEXTI_HandleTypeDef *Handle);
void PAL_EXTI_InterruptCmd(PalEXTI_HandleTypeDef *Handle, FunctionalState NewState);
void PAL_EXTI_IRQHandler(PalEXTI_HandleTypeDef *Handle);

#endif
