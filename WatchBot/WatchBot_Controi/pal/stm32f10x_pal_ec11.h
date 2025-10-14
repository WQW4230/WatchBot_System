/**
  ******************************************************************************
  * @file    stm32f10x_pal_ec11.h
  * @author  铁头山羊stm32工作组
  * @version V1.0.0
  * @date    2022年12月18日
  * @brief   EC11编码器驱动
  ******************************************************************************
  * @attention
  * Copyright (c) 2022 东莞市明玉科技有限公司. 保留所有权力.
  ******************************************************************************
	*/

#ifndef _STM32F10x_PAL_EC11_H_
#define _STM32F10x_PAL_EC11_H_

#include "stm32f10x.h"

typedef struct
{
	GPIO_TypeDef *CLK_GPIO_Port; /* CLK引脚（模块的CLK引脚对应EC11的引脚A）的GPIO端口。可设置为以下值之一：
	                                  GPIOA, GPIOB, GPIOC, .., GPIOG */
	uint16_t CLK_GPIO_Pin;       /* CLK引脚（模块的CLK引脚对应EC11的引脚A）的GPIO引脚编号。可设置为以下值之一：
	                                  GPIO_Pin_0, GPIO_Pin_1,.. , GPIO_Pin_15 */
	GPIO_TypeDef *DT_GPIO_Port;  /* DT引脚（模块的DT引脚对应EC11的引脚B）的GPIO端口。可设置为以下值之一：
	                                  GPIOA, GPIOB, GPIOC, .., GPIOG */
	uint16_t DT_GPIO_Pin;        /* DT引脚（模块的CLK引脚对应EC11的引脚B）的GPIO引脚编号。可设置为以下值之一：
	                                  GPIO_Pin_0, GPIO_Pin_1,.. , GPIO_Pin_15 */
	uint8_t EXTI_PreemptionPriority;  /* CLK引脚（模块的CLK引脚对应EC11的引脚A）需要使用外部中断，该值用于设置外部中断的抢占优先级 */
	uint8_t EXTI_SubPriority;         /* CLK引脚（模块的CLK引脚对应EC11的引脚A）需要使用外部中断，该值用于设置外部中断的子优先级 */
	int32_t CounterMin;            /* 用于设置内部计数器的最小值 */
	int32_t CounterMax;            /* 用于设置内部计数器的最大值 */
	void (*ValueChangedCallback)(int32_t Value, int32_t PreviousValue); /* 回调函数，当内部计数器的值发生改变时触发该回调函数 
	                                                                       参数含义如下：
	                                                                         - Value 变化之后的值
	                                                                         - PreviousValue 变化之前的值*/
} PalEC11_InitTypeDef;

typedef struct
{
	PalEC11_InitTypeDef Init;
	/*Private*/
	BitAction PreviousCLKBitVal;
	int32_t PreviousValue;
	int32_t CurrentValue;
	uint64_t PreiousChangeTime;
}PalEC11_HandleTypeDef;

ErrorStatus PAL_EC11_Init(PalEC11_HandleTypeDef *Handle);
       void PAL_EC11_Proc(PalEC11_HandleTypeDef *Handle);
       void PAL_EC11_EXTI_IRQHandler(PalEC11_HandleTypeDef *Handle);
    int32_t PAL_EC11_GetCurrentValue(PalEC11_HandleTypeDef *Handle);
       void PAL_EC11_Reset(PalEC11_HandleTypeDef *Handle);

#endif
