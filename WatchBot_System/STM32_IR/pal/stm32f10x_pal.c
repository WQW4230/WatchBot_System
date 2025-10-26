/**
  ******************************************************************************
  * @file    stm32f10x_pal.c 
  * @author  铁头山羊
  * @version V 1.0.0
  * @date    2022年8月30日
  * @brief   外设抽象层源文件
  ******************************************************************************
  */

#include "stm32f10x_pal.h"

static __IO uint64_t ulTicks;
static float us_per_mini_tick;
static void Systick_Init(void);

// @简介：初始化PAL库
// @参数：无
// @返回值：无
// @注意：请务必在使用PAL库之前调用此方法
void PAL_Init(void)
{
	Systick_Init(); // 初始化系统滴答定时器	
}


// @简介：毫秒级延迟
// @参数：Delay - 延迟时长，以毫秒为单位(千分之一秒)
// @返回值：无
// @注意：不允许在中断响应函数中调用此方法
void PAL_Delay(uint32_t Delay)
{
	uint64_t expiredTime = ulTicks + Delay;

	while(ulTicks <  expiredTime){}
}

// @简介：微秒级延迟
// @参数：Us - 延迟时长，单位微秒(百万分之一秒)
// @返回值：无
// @注意：不允许在中断响应函数中调用此方法
void PAL_DelayUs(uint32_t Us)
{
	uint64_t expiredTime = PAL_GetUs() + Us + 1;
	
	while(PAL_GetUs() < expiredTime);
}

// @简介：延迟直到某一时刻
// @参数：WakeupTime - 延迟停止的时刻，单位毫秒(千分之一秒)
// @返回值：无
// 注意：不允许在中断响应函数中调用此方法
void PAL_DelayUntil(uint64_t WakeupTime)
{
	while(ulTicks < WakeupTime);
}

// @简介：获取当前时间，以毫秒（千分之一秒）为单位
// @参数：无
// @返回值：当前时间，单位为毫秒（千分之一秒）
uint64_t PAL_GetTick(void)
{
	return ulTicks;
}


// @简介：获取当前时间，以微秒（百万分之一秒）为单位
// @参数：空
// @返回值：当前时间，单位为微秒（百万分之一秒）
uint64_t PAL_GetUs(void)
{
	uint64_t tick;
	uint32_t mini_tick;
	
	SysTick->CTRL &= ~SysTick_CTRL_COUNTFLAG; // 清除COUNTFLAG
	
	tick = ulTicks; // 读取毫秒值
	mini_tick = SysTick->VAL; // 读取SYSTICK的值
	
	// 直到无溢出标志
	// 读取COUNTERFLAG也会清除它的值
	while(SysTick->CTRL & SysTick_CTRL_COUNTFLAG) 
	{
		mini_tick = SysTick->VAL;
		tick = ulTicks;
	}
	
	// 换算成微秒
	tick *= 1000; // 毫秒部分乘以1000
	tick += (uint32_t)((SysTick->LOAD - mini_tick) * us_per_mini_tick); // 小数部分折算成微秒
	
	return tick;
}

// @简介：系统滴答定时器中断的处理代码
// @参数：无
// @返回值：无
// @注意：此方法应当在Systick_IRQHandler中被调用
void PAL_Systick_IRQHandler(void)
{
	ulTicks++;
}

static void Systick_Init(void)
{
	RCC_ClocksTypeDef clockinfo = {0};
	uint32_t tmp;
	
	SysTick->CTRL &= ~SysTick_CTRL_ENABLE; // 禁止SYSTICK

	ulTicks = 0;

	RCC_GetClocksFreq(&clockinfo);

	SysTick->CTRL |= SysTick_CTRL_TICKINT; // 开启中断

	// 设置中断优先级为0
	SCB->SHP[7] = 0;

	// 设置自动重装值以保证1ms的时钟
	tmp =  clockinfo.HCLK_Frequency / 1000;
	if(tmp > 0x00ffffff)
	{
		tmp = tmp / 8;
		SysTick->CTRL &= ~SysTick_CTRL_CLKSOURCE; 
	}
	else
	{
		SysTick->CTRL |= SysTick_CTRL_CLKSOURCE; 
	}
	SysTick->LOAD = tmp - 1;

	SysTick->CTRL |= SysTick_CTRL_ENABLE; 
	
	us_per_mini_tick = 1000.0 / ((SysTick->LOAD & 0x00ffffff) + 1);
}

