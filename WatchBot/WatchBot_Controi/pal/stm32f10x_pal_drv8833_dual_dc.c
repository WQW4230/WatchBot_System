/**
  ******************************************************************************
  * @file    stm32f10x_pal_drv8833_dual_dc.c
  * @author  铁头山羊stm32工作组
  * @version V1.0.0
  * @date    2023年2月24日
  * @brief   DRV8833（双直流电机）驱动程序
  ******************************************************************************
  * @attention
  * Copyright (c) 2022 - 2023 东莞市明玉科技有限公司. 保留所有权力.
  ******************************************************************************
*/

#include "stm32f10x_pal_drv8833_dual_dc.h"
#include "stm32f10x_pal.h"
//#include "stm32f10x_pal_uart.h"

//extern PalUART_HandleTypeDef huart2;

static void TIMxForPWM_Init(TIM_TypeDef *TIMx, uint32_t Remap);
static void TIMxForEncoder_Init(TIM_TypeDef *TIMx, uint32_t Remap);

// 6阶iir滤波器，采样率200Hz，截止频率20Hz
static const float encoder_iir_filter_b[] = 
{
	0.00034053765272012769893886230399004944, 
	0.002043225916320766302053391072490740044,
  0.00510806479080191597197391217832773691  ,
	0.006810753054402553761936811582700102008 ,
	0.00510806479080191597197391217832773691  ,
	0.002043225916320766302053391072490740044, 
	0.00034053765272012769893886230399004944  ,
};

static const float encoder_iir_filter_a[] = 
{
	1,
	-3.579434798331192268960876390337944030762,
	 5.658667165933626286289381823735311627388,
	-4.965415228778572220846854179399088025093,
	2.529494905841449092775974349933676421642,
	-0.705274114509901295377858332358300685883,
	 0.083756479618679019472082813990709837526,
};

void PAL_Drv8833DualDC_Init(PalDRV8833DualDC_HandleTypeDef *Handle)
{
	GPIO_InitTypeDef GPIOInitStruct;
	
	/* 1. 初始化standby引脚 */
	if(!Handle->Init.Advanced.DisableStandbyPin)
	{
		RCC_GPIOx_ClkCmd(Handle->Init.PWM.StandbyGPIOPort, ENABLE);
	
		GPIO_WriteBit(Handle->Init.PWM.StandbyGPIOPort, Handle->Init.PWM.StandbyGPIOPin, Bit_RESET);// 进入sleep模式
		
		GPIOInitStruct.GPIO_Pin = Handle->Init.PWM.StandbyGPIOPin;
		GPIOInitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIOInitStruct.GPIO_Speed = GPIO_Speed_2MHz;
		GPIO_Init(Handle->Init.PWM.StandbyGPIOPort, &GPIOInitStruct);
	}
	
	/* 2. 为电机的PWM控制信号初始化定时器 */
	TIMxForPWM_Init(Handle->Init.PWM.TIMx, Handle->Init.Advanced.MotorTIMRemap);
	
	/* 3. 为电机1的编码器初始化定时器 */
	TIMxForEncoder_Init(Handle->Init.Encoder.Encoder1TIMx, Handle->Init.Advanced.Encoder1TIMRemap);
	
	/* 4. 为电机2的编码器初始化定时器 */
	TIMxForEncoder_Init(Handle->Init.Encoder.Encoder2TIMx, Handle->Init.Advanced.Encoder2TIMRemap);
	
	// 初始化编码器1的低通滤波器
	PAL_IIRFilter_Init(&Handle->Encoder1_lpf, 6, encoder_iir_filter_a, encoder_iir_filter_b);
	// 初始化编码器2的低通滤波器
	PAL_IIRFilter_Init(&Handle->Encoder2_lpf, 6, encoder_iir_filter_a, encoder_iir_filter_b);
	
	Handle->NextSampleTime = PAL_GetTick();
	Handle->Encoder1TIMxLastCnt = 0;
	Handle->Encoder2TIMxLastCnt = 0;
	Handle->k = 1.0 / (Handle->Init.Encoder.SampleInterval * 0.001) * 60 / Handle->Init.Encoder.TickPerTurn;
}

static void TIMxForPWM_Init(TIM_TypeDef *TIMx, uint32_t Remap)
{
	GPIO_InitTypeDef GPIOInitStruct;
	
	/* 开启定时器的时钟 */
	if(TIMx == TIM1)
	{
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
	}
	else if(TIMx == TIM2)
	{
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	}
	else if(TIMx == TIM3)
	{
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	}
	else if(TIMx == TIM4)
	{
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
	}
	
	TIM_ARRPreloadConfig(TIMx, ENABLE);
	
	/* 初始化时基单元 */
	RCC_ClocksTypeDef RCC_Clocks;
	RCC_GetClocksFreq(&RCC_Clocks);
	
	uint32_t tim_internal_clk;
	if(TIMx == TIM1 || TIMx == TIM8 || TIMx == TIM9 || TIMx == TIM10 || TIMx == TIM11)
	{
		tim_internal_clk = RCC_Clocks.PCLK2_Frequency;
		if(RCC_Clocks.HCLK_Frequency /  RCC_Clocks.PCLK2_Frequency > 1)
		{
			tim_internal_clk *= 2;
		}
	}
	else
	{
		tim_internal_clk = RCC_Clocks.PCLK1_Frequency;
		if(RCC_Clocks.HCLK_Frequency /  RCC_Clocks.PCLK1_Frequency > 1)
		{
			tim_internal_clk *= 2;
		}
	}
	
	TIM_TimeBaseInitTypeDef TimeBaseInitStruct;
	TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up; 
	TimeBaseInitStruct.TIM_Period = 99; // 周期0.1ms
	TimeBaseInitStruct.TIM_Prescaler = tim_internal_clk / 1000000 - 1; // 分辨率1us
	TimeBaseInitStruct.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIMx, &TimeBaseInitStruct);
	
	/* 配置通道1~4 */
	// 为通道1~4初始化IO引脚
	if(TIMx == TIM1)
	{
		if(Remap == 0 || Remap == 1) // 无IO重映射或部分重映射
		{ 
			// TIM1_CH1, CH2, CH3, CH4 -> PA8, PA9, PA10, PA11
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
			GPIOInitStruct.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11;
			GPIOInitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
			GPIOInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_Init(GPIOA, &GPIOInitStruct);
		}
		else if(Remap == 3) // 完全重映射
		{
			// TIM1_CH1, CH2, CH3, CH4 -> PE9, PE11, PE13, PE14
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);
			GPIOInitStruct.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_11 | GPIO_Pin_13 | GPIO_Pin_14;
			GPIOInitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
			GPIOInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_Init(GPIOE, &GPIOInitStruct);
		}
	}
	else if(TIMx == TIM2)
	{
		if(Remap == 0) // 无重映射
		{
			// TIM2_CH1, CH2, CH3, CH4 -> PA0, PA1, PA2, PA3
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
			GPIOInitStruct.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;
			GPIOInitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
			GPIOInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_Init(GPIOA, &GPIOInitStruct);
		}
		else if(Remap == 1) // 部分重映射01
		{
			// 配置AFIO
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
			GPIO_PinRemapConfig(GPIO_PartialRemap1_TIM2, ENABLE);
			// TIM2_CH1, CH2, CH3, CH4 -> PA15, PB3, PA2, PA3
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
			GPIOInitStruct.GPIO_Pin = GPIO_Pin_15 | GPIO_Pin_2 | GPIO_Pin_3;
			GPIOInitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
			GPIOInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_Init(GPIOA, &GPIOInitStruct);
			
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
			GPIOInitStruct.GPIO_Pin = GPIO_Pin_3;
			GPIOInitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
			GPIOInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_Init(GPIOB, &GPIOInitStruct);
		}
		else if(Remap == 2) // 部分重映射10
		{
			// 配置AFIO
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
			GPIO_PinRemapConfig(GPIO_PartialRemap2_TIM2, ENABLE);
			// TIM2_CH1, CH2, CH3, CH4 -> PA0, PA1, PB10, PB11
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
			GPIOInitStruct.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
			GPIOInitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
			GPIOInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_Init(GPIOA, &GPIOInitStruct);
			
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
			GPIOInitStruct.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
			GPIOInitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
			GPIOInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_Init(GPIOB, &GPIOInitStruct);
		}
		else if(Remap == 3) // 完全重映射
		{
			// 配置AFIO
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
			GPIO_PinRemapConfig(GPIO_FullRemap_TIM2, ENABLE);
			// TIM2_CH1, CH2, CH3, CH4 -> PA15, PB3, PB10, PB11
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
			GPIOInitStruct.GPIO_Pin = GPIO_Pin_15;
			GPIOInitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
			GPIOInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_Init(GPIOA, &GPIOInitStruct);
			
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
			GPIOInitStruct.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_10 | GPIO_Pin_11;
			GPIOInitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
			GPIOInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_Init(GPIOB, &GPIOInitStruct);
		}
	}
	else if(TIMx == TIM3)
	{
		if(Remap == 0) // 无重映射
		{
			// TIM3_CH1, CH2, CH3, CH4 -> PA6, PA7, PB0, PB1
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
			GPIOInitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
			GPIOInitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
			GPIOInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_Init(GPIOA, &GPIOInitStruct);
			
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
			GPIOInitStruct.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
			GPIOInitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
			GPIOInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_Init(GPIOB, &GPIOInitStruct);
		}
		else if(Remap == 2) // 部分重映射02
		{
			// 配置AFIO
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
			GPIO_PinRemapConfig(GPIO_PartialRemap_TIM3, ENABLE);
			// TIM2_CH1, CH2, CH3, CH4 -> PB4, PB5, PB0, PA1
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
			GPIOInitStruct.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_0 | GPIO_Pin_1;
			GPIOInitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
			GPIOInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_Init(GPIOB, &GPIOInitStruct);
		}
		else if(Remap == 3) // 全部重映射03，未实现
		{
			// 配置AFIO
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
			GPIO_PinRemapConfig(GPIO_FullRemap_TIM3, ENABLE);
			// TIM2_CH1, CH2, CH3, CH4 -> PC6, PC7, PC8, PC9
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
			GPIOInitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9;
			GPIOInitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
			GPIOInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_Init(GPIOC, &GPIOInitStruct);
		}
	}
	else if(TIMx == TIM4)
	{
		// TIM4_CH1, CH2, CH3, CH4 -> PB6, PB7, PB8, PB9
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
		GPIOInitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9;
		GPIOInitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
		GPIOInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOB, &GPIOInitStruct);
	}
	
	TIM_OCInitTypeDef OCInitStruct;
	
	// 通道1
	OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
	OCInitStruct.TIM_OutputState = TIM_OutputState_Enable; // 使能该通道的输出
	OCInitStruct.TIM_OutputNState = TIM_OutputNState_Disable; // 禁止互补通道的输出
	OCInitStruct.TIM_Pulse = 0x00; // 速度为0
	OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_Low;
	OCInitStruct.TIM_OCIdleState = TIM_OCIdleState_Reset;
	
	TIM_OC1Init(TIMx, &OCInitStruct);
	TIM_OC1PreloadConfig(TIMx, TIM_OCPreload_Enable);
	
	// 通道2
	OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
	OCInitStruct.TIM_OutputState = TIM_OutputState_Enable; // 使能该通道的输出
	OCInitStruct.TIM_OutputNState = TIM_OutputNState_Disable; // 禁止互补通道的输出
	OCInitStruct.TIM_Pulse = 0x00; // 速度为0
	OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_Low;
	OCInitStruct.TIM_OCIdleState = TIM_OCIdleState_Reset;
	
	TIM_OC2Init(TIMx, &OCInitStruct);
	TIM_OC2PreloadConfig(TIMx, TIM_OCPreload_Enable);
	
	// 通道3
	OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
	OCInitStruct.TIM_OutputState = TIM_OutputState_Enable; // 使能该通道的输出
	OCInitStruct.TIM_OutputNState = TIM_OutputNState_Disable; // 禁止互补通道的输出
	OCInitStruct.TIM_Pulse = 0x00; // 速度为0
	OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_Low;
	OCInitStruct.TIM_OCIdleState = TIM_OCIdleState_Reset;
	
	TIM_OC3Init(TIMx, &OCInitStruct);
	TIM_OC3PreloadConfig(TIMx, TIM_OCPreload_Enable);
	
	// 通道4
	OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
	OCInitStruct.TIM_OutputState = TIM_OutputState_Enable; // 使能该通道的输出
	OCInitStruct.TIM_OutputNState = TIM_OutputNState_Disable; // 禁止互补通道的输出
	OCInitStruct.TIM_Pulse = 0x00; // 速度为0
	OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_Low;
	OCInitStruct.TIM_OCIdleState = TIM_OCIdleState_Reset;
	
	TIM_OC4Init(TIMx, &OCInitStruct);
	TIM_OC4PreloadConfig(TIMx, TIM_OCPreload_Enable);
	
	TIM_CtrlPWMOutputs(TIMx, ENABLE);	 // MOE=1
	
	TIM_GenerateEvent(TIMx, TIM_EventSource_Update);
	
	/* 开启定时器 */
	TIM_Cmd(TIMx, ENABLE);
}

static void TIMxForEncoder_Init(TIM_TypeDef *TIMx, uint32_t Remap)
{
	GPIO_InitTypeDef GPIOInitStruct;
	
	if(TIMx == TIM1)
	{
		RCC_APB2PeriphResetCmd(RCC_APB2Periph_TIM1, ENABLE);
		RCC_APB2PeriphResetCmd(RCC_APB2Periph_TIM1, DISABLE);
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
		
		if(Remap == 0 || Remap== 1) // 无IO重映射或部分重映射
		{ 
			// TIM1_CH1, CH2 -> PA8, PA9
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
			GPIOInitStruct.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
			GPIOInitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
			GPIOInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_Init(GPIOA, &GPIOInitStruct);
		}
		else if(Remap == 3)
		{
			// TIM1_CH1, CH2 -> PE9, PE11
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);
			GPIOInitStruct.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_11;
			GPIOInitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
			GPIOInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_Init(GPIOE, &GPIOInitStruct);
		}
	}
	else if(TIMx == TIM2)
	{
		RCC_APB1PeriphResetCmd(RCC_APB1Periph_TIM2, ENABLE);
		RCC_APB1PeriphResetCmd(RCC_APB1Periph_TIM2, DISABLE);
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
		if(Remap == 0) // 无重映射
		{
			// TIM2_CH1, CH2, CH3, CH4 -> PA0, PA1, PA2, PA3
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
			GPIOInitStruct.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
			GPIOInitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
			GPIOInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_Init(GPIOA, &GPIOInitStruct);
		}
		else if(Remap == 1) // 部分重映射01
		{
			// 配置AFIO
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
			GPIO_PinRemapConfig(GPIO_PartialRemap1_TIM2, ENABLE);
			// TIM2_CH1, CH2, CH3, CH4 -> PA15, PB3, PA2, PA3
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
			GPIOInitStruct.GPIO_Pin = GPIO_Pin_15;
			GPIOInitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
			GPIOInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_Init(GPIOA, &GPIOInitStruct);
			
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
			GPIOInitStruct.GPIO_Pin = GPIO_Pin_3;
			GPIOInitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
			GPIOInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_Init(GPIOB, &GPIOInitStruct);
		}
		else if(Remap == 2) // 部分重映射10
		{
			// 配置AFIO
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
			GPIO_PinRemapConfig(GPIO_PartialRemap2_TIM2, ENABLE);
			// TIM2_CH1, CH2, CH3, CH4 -> PA0, PA1, PB10, PB11
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
			GPIOInitStruct.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
			GPIOInitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
			GPIOInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_Init(GPIOA, &GPIOInitStruct);
		}
		else if(Remap == 3) // 完全重映射
		{
			// 配置AFIO
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
			GPIO_PinRemapConfig(GPIO_FullRemap_TIM2, ENABLE);
			// TIM2_CH1, CH2, CH3, CH4 -> PA15, PB3, PB10, PB11
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
			GPIOInitStruct.GPIO_Pin = GPIO_Pin_15;
			GPIOInitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
			GPIOInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_Init(GPIOA, &GPIOInitStruct);
			
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
			GPIOInitStruct.GPIO_Pin = GPIO_Pin_3;
			GPIOInitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
			GPIOInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_Init(GPIOB, &GPIOInitStruct);
		}
	}
	else if(TIMx == TIM3)
	{
		RCC_APB1PeriphResetCmd(RCC_APB1Periph_TIM3, ENABLE);
		RCC_APB1PeriphResetCmd(RCC_APB1Periph_TIM3, DISABLE);
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
		if(Remap == 0) // 无重映射
		{
			// TIM3_CH1, CH2, CH3, CH4 -> PA6, PA7, PB0, PB1
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
			GPIOInitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
			GPIOInitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
			GPIOInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_Init(GPIOA, &GPIOInitStruct);
		}
		else if(Remap == 2) // 部分重映射02
		{
			// 配置AFIO
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
			GPIO_PinRemapConfig(GPIO_PartialRemap_TIM3, ENABLE);
			// TIM2_CH1, CH2, CH3, CH4 -> PB4, PB5, PB0, PA1
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
			GPIOInitStruct.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;
			GPIOInitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
			GPIOInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_Init(GPIOB, &GPIOInitStruct);
		}
		else if(Remap == 3) // 全部重映射03，未实现
		{
		}
	}
	else if(TIMx == TIM4)
	{
		RCC_APB1PeriphResetCmd(RCC_APB1Periph_TIM4, ENABLE);
		RCC_APB1PeriphResetCmd(RCC_APB1Periph_TIM4, DISABLE);
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
		// TIM4_CH1, CH2, CH3, CH4 -> PB6, PB7, PB8, PB9
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
		GPIOInitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
		GPIOInitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		GPIOInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOB, &GPIOInitStruct);
	}
	
	// 初始化时基单元
	TIM_TimeBaseInitTypeDef TimeBaseInitStruct;
	TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
	TimeBaseInitStruct.TIM_Period = 0xffff;
	TimeBaseInitStruct.TIM_Prescaler = 0;
	TimeBaseInitStruct.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIMx, &TimeBaseInitStruct);
	
	// 选择从模式为编码器模式1
	TIM_SelectSlaveMode(TIMx, TIM_SlaveMode_EncoderMode3);
	
	// 配置通道1
	// 正极性
	// 配置滤波器，ICF=1111 -> 4us滤波周期
	TIM_ICInitTypeDef ICInitStruct;
	
	ICInitStruct.TIM_Channel = TIM_Channel_1;
	ICInitStruct.TIM_ICFilter = 0xf;
	ICInitStruct.TIM_ICPolarity = TIM_ICPolarity_Rising;
	ICInitStruct.TIM_ICPrescaler = 0;
	ICInitStruct.TIM_ICSelection = TIM_ICSelection_DirectTI;
	TIM_ICInit(TIMx, &ICInitStruct);
	
	// 配置通道2
	// 负极性
	// 配置滤波器，ICF=1111 -> 4us滤波周期
	ICInitStruct.TIM_Channel = TIM_Channel_2;
	ICInitStruct.TIM_ICFilter = 0xf;
	ICInitStruct.TIM_ICPolarity = TIM_ICPolarity_Falling;
	ICInitStruct.TIM_ICPrescaler = 0;
	ICInitStruct.TIM_ICSelection = TIM_ICSelection_DirectTI;
	TIM_ICInit(TIMx, &ICInitStruct);
	
	TIM_Cmd(TIMx, ENABLE);
}

void PAL_Drv8833DualDC_SleepModeConfig(PalDRV8833DualDC_HandleTypeDef *Handle, FunctionalState NewState)
{
	if(!Handle->Init.Advanced.DisableStandbyPin)
	{
		GPIO_WriteBit(Handle->Init.PWM.StandbyGPIOPort, Handle->Init.PWM.StandbyGPIOPin, NewState == ENABLE ?  Bit_RESET : Bit_SET);
	}
}

void PAL_Drv8833DualDC_SetMotor1PWMDuty(PalDRV8833DualDC_HandleTypeDef *Handle, float Speed)
{
	if(Handle->Init.PWM.Motor1Reverse == SET)
	{
		Speed = -Speed;
	}
	
	if(Speed >= 100.0)
	{
		Speed = 100.0;
	}
	else if(Speed <= -100.0)
	{
		Speed = -100.0;
	}
	
	TIM_UpdateDisableConfig(Handle->Init.PWM.TIMx, ENABLE);
	if(Speed > 0)
	{
		TIM_SetCompare2(Handle->Init.PWM.TIMx, (uint16_t)(Speed / 100.0 * Handle->Init.PWM.TIMx->ARR));
		TIM_SetCompare1(Handle->Init.PWM.TIMx, 0);
	}
	else
	{
		TIM_SetCompare2(Handle->Init.PWM.TIMx, 0);
		TIM_SetCompare1(Handle->Init.PWM.TIMx, (uint16_t)(-Speed / 100.0 * Handle->Init.PWM.TIMx->ARR));
	}
	TIM_UpdateDisableConfig(Handle->Init.PWM.TIMx, DISABLE);
}

void PAL_Drv8833DualDC_SetMotor2PWMDuty(PalDRV8833DualDC_HandleTypeDef *Handle, float Speed)
{
	if(Handle->Init.PWM.Motor2Reverse == SET)
	{
		Speed = -Speed;
	}
	
	if(Speed >= 100.0)
	{
		Speed = 100.0;
	}
	else if(Speed <= -100.0)
	{
		Speed = -100.0;
	}
	
	TIM_UpdateDisableConfig(Handle->Init.PWM.TIMx, ENABLE);
	if(Speed > 0)
	{
		TIM_SetCompare4(Handle->Init.PWM.TIMx, (uint16_t)(Speed / 100.0 * Handle->Init.PWM.TIMx->ARR));
		TIM_SetCompare3(Handle->Init.PWM.TIMx, 0);
	}
	else
	{
		TIM_SetCompare4(Handle->Init.PWM.TIMx, 0);
		TIM_SetCompare3(Handle->Init.PWM.TIMx, (uint16_t)(-Speed / 100.0 * Handle->Init.PWM.TIMx->ARR));
	}
	TIM_UpdateDisableConfig(Handle->Init.PWM.TIMx, DISABLE);
}

void PAL_Drv8833DualDC_SetMotorPWMDuty(PalDRV8833DualDC_HandleTypeDef *Handle, float Motor1Speed, float Motor2Speed)
{
	PAL_Drv8833DualDC_SetMotor1PWMDuty(Handle, Motor1Speed);
	PAL_Drv8833DualDC_SetMotor2PWMDuty(Handle, Motor2Speed);
}

float PAL_Drv8833DualDC_GetMotor1Speed(PalDRV8833DualDC_HandleTypeDef *Handle)
{
	float speed = Handle->speed1;
	if(Handle->Init.Encoder.Encoder1Reverse == SET)
	{
		speed*=-1;
	}
	return speed;
}

float PAL_Drv8833DualDC_GetMotor2Speed(PalDRV8833DualDC_HandleTypeDef *Handle)
{
	float speed = Handle->speed2;
	if(Handle->Init.Encoder.Encoder2Reverse == SET)
	{
		speed*=-1;
	}
	return speed;
}

void PAL_Drv8833DualDC_SpeedTestProc(PalDRV8833DualDC_HandleTypeDef *Handle)
{
	uint64_t currentTick = PAL_GetTick();
	
	if(currentTick > Handle->NextSampleTime)
	{
		// 更新编码器的值
		uint16_t encoder1 = TIM_GetCounter( Handle->Init.Encoder.Encoder1TIMx);
		uint16_t encoder2 = TIM_GetCounter( Handle->Init.Encoder.Encoder2TIMx);
		
		int encoder1Change, encoder2Change;
		
		if(encoder1 >= Handle->Encoder1TIMxLastCnt)
		{
			if(encoder1 - Handle->Encoder1TIMxLastCnt < 0x7fff) // 正转
			{
				encoder1Change = encoder1 - Handle->Encoder1TIMxLastCnt;
			}
			else // 反转&溢出
			{
				encoder1Change = -(Handle->Encoder1TIMxLastCnt + 0xffff - encoder1);
			}
		}
		else
		{
			if(Handle->Encoder1TIMxLastCnt - encoder1 < 0x7fff) // 反转
			{
				encoder1Change = -(Handle->Encoder1TIMxLastCnt - encoder1);
			}
			else // 正转&溢出
			{
				encoder1Change = encoder1 + 0xffff - Handle->Encoder1TIMxLastCnt;
			}
		}
		
		if(encoder2 > Handle->Encoder2TIMxLastCnt)
		{
			if(encoder2 - Handle->Encoder2TIMxLastCnt < 0x7fff) // 正转
			{
				encoder2Change = encoder2 - Handle->Encoder2TIMxLastCnt;
			}
			else // 反转&溢出
			{
				encoder2Change = -(Handle->Encoder2TIMxLastCnt + 0xffff - encoder2);
			}
		}
		else
		{
			if(Handle->Encoder2TIMxLastCnt - encoder2 < 0x7fff) // 反转
			{
				encoder2Change = -(Handle->Encoder2TIMxLastCnt - encoder2);
			}
			else // 正转&溢出
			{
				encoder2Change = encoder2 + 0xffff - Handle->Encoder2TIMxLastCnt;
			}
		}
		Handle->Encoder1TIMxLastCnt = encoder1;
		Handle->Encoder2TIMxLastCnt = encoder2;
		
		Handle->speed1 = PAL_IIRFilter_Calc(&Handle->Encoder1_lpf, encoder1Change) * Handle->k;
		Handle->speed2 = PAL_IIRFilter_Calc(&Handle->Encoder2_lpf, encoder2Change) * Handle->k;
		
//		PAL_UART_Printf_DMA(&huart2, "$%f %f;", Handle->speed1, encoder1Change * Handle->k);
//		PAL_UART_FlushDMA(&huart2);
		
		while(currentTick > Handle->NextSampleTime && currentTick - Handle->NextSampleTime >= Handle->Init.Encoder.SampleInterval)
		{
			Handle->NextSampleTime += Handle->Init.Encoder.SampleInterval;
		}
		
		Handle->NextSampleTime += Handle->Init.Encoder.SampleInterval;
	}
}
