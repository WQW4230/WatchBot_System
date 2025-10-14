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
		if(currentTime < Handle->PendingTime + BUTTON_SETTLING_TIME)
		{
			// Do nothing
		}
		else
		{
			currentState = GPIO_ReadInputDataBit(Handle->Init.GPIOx, Handle->Init.GPIO_Pin);
			if(currentState != Handle->LastState)
			{
				Handle->LastState = currentState;
				if(currentState == Bit_RESET)
				{
					if(Handle->Init.ButtonPressedCallback != 0)
					{
						Handle->Init.ButtonPressedCallback();
					}
				}
				else 
				{
					if(Handle->Init.ButtonReleasedCallback != 0)
					{
						Handle->Init.ButtonReleasedCallback();
					}
				}
			}
			else
			{
				// Do nothing
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
}
