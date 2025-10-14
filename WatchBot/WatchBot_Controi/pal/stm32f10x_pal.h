/**
  ******************************************************************************
  * @file    stm32f10x_pal.h 
  * @author  铁头山羊
  * @version V 1.0.0
  * @date    2022年8月30日
  * @brief   外设抽象层头文件
  ******************************************************************************
  */

#ifndef _STM32F10X_PAL_H_
#define _STM32F10X_PAL_H_

#include "stm32f10x.h"

#define _STM32F10X_PAL_VERSION_MAJOR (uint32_t)2  // 主版本
#define _STM32F10x_PAL_VERSION_MINOR (uint32_t)0  // 副版本号
#define _STM32F10x_PAL_VERSION_BUILD (uint32_t)60 // 编译版本号

#define PAL_MAX_DELAY 0xffffffff
#define PAL_INVALID_TICK 0xffffffffffffffff

#define CHECK(v) if((v)!=SUCCESS) return ERROR;

#define RCC_GPIOx_ClkCmd(GPIOx, NewState) do{\
	if(GPIOx == GPIOA)\
	{\
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, NewState);\
	}\
	else if(GPIOx == GPIOB)\
	{\
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, NewState);\
	}\
	else if(GPIOx == GPIOC)\
	{\
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, NewState);\
	}\
	else if(GPIOx == GPIOD)\
	{\
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, NewState);\
	}\
	else if(GPIOx == GPIOE)\
	{\
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, NewState);\
	}\
	else if(GPIOx == GPIOF)\
	{\
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF, NewState);\
	}\
	else if(GPIOx == GPIOG)\
	{\
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOG, NewState);\
	}\
}while(0)

void PAL_Init(void);
void PAL_Delay(uint32_t Delay);
void PAL_DelayUntil(uint64_t WakeupTime);
uint64_t PAL_GetTick(void);
void PAL_DelayUs(uint32_t Us);
uint64_t PAL_GetUs(void);
void PAL_Systick_IRQHandler(void);

#endif
