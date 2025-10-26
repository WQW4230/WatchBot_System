/**
  ******************************************************************************
  * @file    stm32f10x_pal_button.h
  * @author  铁头山羊stm32工作组
  * @version V1.0.0
  * @date    2022年11月25日
  * @brief   按钮驱动
  ******************************************************************************
  * @attention
  * Copyright (c) 2022 - 2023 东莞市明玉科技有限公司.
  * 保留所有权力.
  *
  ******************************************************************************
  */


#ifndef _STM32F10x_PAL_MODULE_BUTTON_H_
#define _STM32F10x_PAL_MODULE_BUTTON_H_

#include "stm32f10x.h"
#include "stm32f10x_gpio.h"

#define BUTTON_NOPULL            GPIO_Mode_IN_FLOATING
#define BUTTON_INTERNAL_PULLUP   GPIO_Mode_IPU
#define BUTTON_INTERNAL_PULLDOWN GPIO_Mode_IPD

#define Button_Mode_IPU ((uint32_t)0x00) // 模式1，接内部上拉电阻
#define Button_Mode_EPU ((uint32_t)0x01) // 模式2，接外部上拉电阻

typedef struct
{
	uint32_t LongPressThreshold; // 按钮长按的最短时间标准，单位ms（下限）
	
	void (*ButtonClickCallback)(uint8_t Clicks); // 按钮敲击(适用于多击)
	void (*ButtonLongPressCallback)(void);       // 按钮长按
} PalButton_AdvancedInitTypeDef;

typedef struct
{
	GPIO_TypeDef *GPIOx;   /* 按钮的端口号。可以取GPIOA..G中的一个*/
	uint16_t GPIO_Pin;     /* 按钮的引脚编号。可以取GPIO_Pin_0..15中的一个 */
	uint32_t Button_Mode;  /* 按钮模式：
	                            内部上拉模式 - Button_Mode_IPU
                            	外部上拉模式 - Button_Mode_EPU */
	void (*ButtonPressedCallback)(void);         // 按钮按下
	void (*ButtonReleasedCallback)(void);        // 按钮松开
	
	PalButton_AdvancedInitTypeDef Advanced;
} PalButton_InitTypeDef;

typedef struct 
{
	PalButton_InitTypeDef Init;
	uint64_t PendingTime;
	uint8_t LastState;
	uint8_t ChangePending;
	
	uint8_t LongPressTriggered;
	uint64_t LastPressTime;
	uint64_t LastReleaseTime;
	
	uint64_t ClickCnt;
	
} PalButton_HandleTypeDef;

void PAL_Button_InitHandle(PalButton_HandleTypeDef *Handle);
void PAL_Button_Init(PalButton_HandleTypeDef *Handle);
void PAL_Button_Proc(PalButton_HandleTypeDef *Handle);

#endif
