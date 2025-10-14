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
#include "pal_math.h"

#define sin(x)       pal_sin(x)
#define cos(x)       pal_cos(x)
#define tan(x)       pal_tan(x)
#define asin(x)      pal_asin(x)
#define acos(x)      pal_acos(x)
#define atan(x)      pal_atan(x)
#define atan2(y,x)   pal_atan2(y,x)

#define _PAL_VERSION_MAJOR (uint32_t)2  // 主版本
#define _PAL_VERSION_MINOR (uint32_t)0  // 副版本号
#define _PAL_VERSION_BUILD (uint32_t)3  // 编译版本号

#define PAL_MAX_DELAY 0xffffffff
#define PAL_INVALID_TICK 0xffffffffffffffff
#define PAL_FLOAT_INFINATE 3.402823E38

#define PAL_IS_FLOAT_INFINATE(v) ((v)==PAL_FLOAT_INFINATE)

#define CHECK(v) if((v)!=SUCCESS) return ERROR;

/* 周期性任务宏 */
// 该宏定义应当放在周期性任务进程函数的第一行
// 放置该宏后任务将以__Interval__为间隔执行
#define PAL_PERIODIC_PROC(__Interval__) \
static uint64_t __next_exec_time__=PAL_INVALID_TICK; \
if(__next_exec_time__==PAL_INVALID_TICK) __next_exec_time__ = PAL_GetTick(); \
if(PAL_GetTick() < __next_exec_time__) return; \
__next_exec_time__ += (__Interval__);

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
