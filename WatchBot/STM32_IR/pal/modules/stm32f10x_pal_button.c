/**
  ******************************************************************************
  * @file    stm32f10x_pal_button.c
  * @author  铁头山羊stm32工作组
  * @version V1.0.0
  * @date    2022年11月25日
  * @brief   按钮驱动
  ******************************************************************************
  * @attention
  * Copyright (c) 2022 东莞市明玉科技有限公司. 保留所有权力.
  ******************************************************************************
*/

#include "stm32f10x_pal.h"
#include "stm32f10x_pal_button.h"

#define BUTTON_SETTLING_TIME            10   // 按钮消抖延迟
#define BUTTON_MULTI_CLICK_MAX_INTERVAL 300  // 按钮多击时每次点击的时间最大时间间隔
#define BUTTON_LONG_PRESS_THRESHOLD     2000 // 按钮长按最小时间

static void OnButtonPressed(PalButton_HandleTypeDef *Handle);
static void OnButtonReleased(PalButton_HandleTypeDef *Handle);

//
// @简介：将句柄的初始化参数设置为默认值
//        Button_Mode = Button_Mode_IPU，默认使用内部上拉模式
// 
// @注意：仍需要手动填写的项目
//        GPIOx                  - 所选IO引脚的端口编号
//        GPIO_Pin               - 所选IO引脚的引脚编号
//        ButtonPressedCallback  - 按钮按下时的回调函数（选填）
//        ButtonReleasedCallback - 按钮释放时的回调函数（选填）
//
void PAL_Button_InitHandle(PalButton_HandleTypeDef *Handle)
{
	Handle->Init.Button_Mode = Button_Mode_IPU;
	Handle->Init.ButtonPressedCallback = 0;
	Handle->Init.ButtonReleasedCallback = 0;
}

// @简介：用于初始化按钮的驱动
// @参数：Handle - 按钮句柄
// @返回值：无
void PAL_Button_Init(PalButton_HandleTypeDef *Handle)
{
	GPIO_InitTypeDef gpio_init_struct;
	
	RCC_GPIOx_ClkCmd(Handle->Init.GPIOx, ENABLE);
	
	gpio_init_struct.GPIO_Pin = Handle->Init.GPIO_Pin;
	if(Handle->Init.Button_Mode == Button_Mode_EPU)
	{
		gpio_init_struct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	}
	else if(Handle->Init.Button_Mode == Button_Mode_IPU)
	{
		gpio_init_struct.GPIO_Mode = GPIO_Mode_IPU;
	}
	
	GPIO_Init(Handle->Init.GPIOx, &gpio_init_struct);
	
	Handle->PendingTime = 0;
	Handle->ChangePending = 0;
	Handle->LastState = Bit_SET;
	
	if(Handle->Init.Advanced.LongPressThreshold == 0)
	{
		Handle->Init.Advanced.LongPressThreshold = BUTTON_LONG_PRESS_THRESHOLD;
	}
	
	Handle->LastPressTime = 0;
	Handle->LastReleaseTime = 0;
}

// @简介：按钮的进程函数
// @参数：Handle - 按钮的句柄
// @注意：该方法需要在main函数的while循环中调用
void PAL_Button_Proc(PalButton_HandleTypeDef *Handle)
{
	uint8_t currentState;
	uint64_t currentTime = PAL_GetTick();
	
	if(Handle->ChangePending)
	{
		if (currentTime >= Handle->PendingTime + BUTTON_SETTLING_TIME)
		{
			currentState = GPIO_ReadInputDataBit(Handle->Init.GPIOx, Handle->Init.GPIO_Pin);
			
			if(currentState != Handle->LastState)
			{
				if(currentState == Bit_RESET) 
					OnButtonPressed(Handle);
				else 
					OnButtonReleased(Handle);
			}
			Handle->LastState = currentState;
			Handle->ChangePending = 0;
		}
	}
	else
	{
		currentState = GPIO_ReadInputDataBit(Handle->Init.GPIOx, Handle->Init.GPIO_Pin);
		
		if(currentState != Handle->LastState)
		{
			Handle->PendingTime = currentTime;
			Handle->ChangePending = 1;
		}
	}
	
	if(currentState == Bit_RESET 
		&& Handle->LongPressTriggered == 0
		&& currentTime >= Handle->LastPressTime + Handle->Init.Advanced.LongPressThreshold)
	{
		Handle->LongPressTriggered = 1;
		
		if(Handle->Init.Advanced.ButtonLongPressCallback)
			Handle->Init.Advanced.ButtonLongPressCallback(); // 按钮长按
	}
	
	// 多击
	if( Handle->ClickCnt > 0 // 按钮点击过
	&& currentState == Bit_SET // 当前按钮松开
	&& currentTime - Handle->LastReleaseTime > BUTTON_MULTI_CLICK_MAX_INTERVAL) // 松开的时间超过了阈值
	{
		if(Handle->Init.Advanced.ButtonClickCallback)
			Handle->Init.Advanced.ButtonClickCallback(Handle->ClickCnt); // 按钮长按
		
		Handle->ClickCnt = 0;
	}
}

static void OnButtonPressed(PalButton_HandleTypeDef *Handle)
{
	// 记录按钮按下的时间
	Handle->LastPressTime = PAL_GetTick(); 
	
	// 清除长按标志位
	Handle->LongPressTriggered = 0;
	
	// 调用按钮按下的回调函数
	if(Handle->Init.ButtonPressedCallback != 0)
	{
		Handle->Init.ButtonPressedCallback();
	}
}

static void OnButtonReleased(PalButton_HandleTypeDef *Handle)
{
	// 记录按钮松开的时间
	Handle->LastReleaseTime = PAL_GetTick(); 
	
	// 记录一次按钮点击
	Handle->ClickCnt++; 
	
	// 调用按钮松开的回调函数
	if(Handle->Init.ButtonReleasedCallback != 0)
	{
		Handle->Init.ButtonReleasedCallback();
	}
}
