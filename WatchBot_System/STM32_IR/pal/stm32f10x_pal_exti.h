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
#include "stm32f10x_pal.h"
#include "stm32f10x_exti.h"
#include "stm32f10x_gpio.h"

typedef struct
{
	GPIO_TypeDef *GPIOx; // 所选引脚的端口号，填写GPIOx，x可以取A..G
	uint16_t GPIO_Pin;   // 所选引脚的引脚编号，填写GPIO_Pin_y，y可以取0..15
	GPIOMode_TypeDef GPIO_Mode;  // 引脚模式
	                             //  @GPIO_Mode_IN_FLOATING - 输入浮空
	                             //  @GPIO_Mode_IPD         - 输入下拉
	                             //  @GPIO_Mode_IPU         - 输入上拉
	                             //  @GPIO_Mode_Out_OD      - 输出开漏
	                             //  @GPIO_Mode_Out_PP      - 输出推挽
	                             //  @GPIO_Mode_AF_OD       - 复用开漏
	                             //  @GPIO_Mode_AF_PP       - 复用推挽
	GPIOSpeed_TypeDef GPIO_Speed; // 引脚的最大输出速度（仅输出模式有效）
	                              //  @GPIO_Speed_2MHz       - 2M
	                              //  @GPIO_Speed_10MHz      - 10M
	                              //  @GPIO_Speed_50MHz      - 50M
} PalEXTI_GPIOInitTypeDef;

typedef struct
{
	EXTIMode_TypeDef EXTI_Mode;       // EXTI模式，可选以下值之一：
	                                  //         @EXTI_Mode_Interrupt - 中断模式
	                                  //         @EXTI_Mode_Event - 事件模式
	EXTITrigger_TypeDef EXTI_Trigger; // EXTI触发方式，可选以下值之一：
	                                  //   @EXTI_Trigger_Rising - 上升沿触发
	                                  //   @EXTI_Trigger_Falling - 下降沿触发
	                                  //   @EXTI_Trigger_Rising_Falling - 双边沿触发
} PalEXTI_EXTIInitTypeDef;

typedef struct
{
	uint8_t PreemptionPriority;       // 中断的抢占优先级
	uint8_t SubPriority;              // 中断的子优先级
} PalEXTI_ITInitTypeDef;

typedef struct
{
	PalEXTI_GPIOInitTypeDef GPIO; // GPIO的相关设置
	PalEXTI_EXTIInitTypeDef Exti; // EXTI的相关设置
	PalEXTI_ITInitTypeDef Interrupt; // 中断相关设置
	void (*CallbackFn)(void);        // 回调函数，中断触发时被调用
}PalEXTI_InitTypeDef;

typedef struct
{
	PalEXTI_InitTypeDef Init;
}PalEXTI_HandleTypeDef;

void PAL_EXTI_Init(PalEXTI_HandleTypeDef *Handle);
void PAL_EXTI_LineCmd(PalEXTI_HandleTypeDef *Handle, FunctionalState NewState);
void PAL_EXTI_IRQHandler(PalEXTI_HandleTypeDef *Handle);

#endif
