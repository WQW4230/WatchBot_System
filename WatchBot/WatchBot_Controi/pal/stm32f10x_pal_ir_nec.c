/**
  ******************************************************************************
  * @file    stm32f10x_pal_ir_nec.c
  * @author  铁头山羊stm32工作组
  * @version V1.0.0
  * @date    2023年2月24日
  * @brief   红外遥控驱动
  ******************************************************************************
  * @attention
  * Copyright (c) 2022 - 2023 东莞市明玉科技有限公司. 保留所有权力.
  ******************************************************************************
*/

#include "stm32f10x_pal_ir_nec.h"
#include "stm32f10x_pal.h"

#define CAPTURE_EDGE_RISING 0x00
#define CAPTURE_EDGE_FALLING 0x01

#define PULSE_POLARITY_POSITIVE 0x00
#define PULSE_POLARITY_NEGITIVE 0x01
#define PULSE_POLARITY_INVALID 0xff

#define DECODER_STAGE_IDLE 0x00
#define DECODER_STAGE_START_1 0x01 // 9ms正脉冲上升沿
#define DECODER_STAGE_START_2 0x02 // 9ms正脉冲下降沿
#define DECODER_STAGE_ADDR 0x03 // 正在接收地址
#define DECODER_STAGE_ADDR_INVERSE 0x04 // 反地址
#define DECODER_STAGE_COMMAND 0x05 // 命令
#define DECODER_STAGE_COMMAND_INVERSE 0x06 // 反命令
#define DECODER_STAGE_FINAL_PULSE 0x07 // 帧结束脉冲

#define DECODER_BITSTAGE_POSITIVE_PULSE 0x00
#define DECODER_BITSTAGE_NEGITIVE_PULSE 0x01

static void OnPulseComplete(PalIRNEC_HandleTypeDef *Handle, float duration, uint16_t Polarity);

void PAL_IRNEC_Init(PalIRNEC_HandleTypeDef *Handle)
{
	/*1. 初始化解码器的状态变量 */
	Handle->Decoder.Carrier = RESET;
	Handle->Decoder.Stage = 0x00;
	Handle->Decoder.BitPos = 0x00;
	Handle->Decoder.BitStage = 0x00;
	
	Handle->Decoder.Addr = 0x00;
	Handle->Decoder.LastCapture = 0x00;
	Handle->Decoder.LastCaptureEdge = CAPTURE_EDGE_FALLING;
	
	Handle->Decoder.RxNE = RESET;
	
	/* 2. 初始化IO引脚 */ 
	GPIO_InitTypeDef GPIOInitStruct;
	
	if(Handle->Init.TIMx == TIM1) // TIM1_CH1 -> PA8
	{
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
		GPIOInitStruct.GPIO_Pin = GPIO_Pin_8;
		GPIOInitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		GPIO_Init(GPIOA, &GPIOInitStruct);
	}
	else if(Handle->Init.TIMx == TIM2) // TIM2_CH1 -> PA0
	{
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
		
		GPIOInitStruct.GPIO_Pin = GPIO_Pin_0;
		GPIOInitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		GPIO_Init(GPIOA, &GPIOInitStruct);
	}
	else if(Handle->Init.TIMx == TIM3) // TIM3_CH1 -> PA6
	{
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
		
		GPIOInitStruct.GPIO_Pin = GPIO_Pin_6;
		GPIOInitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		GPIO_Init(GPIOA, &GPIOInitStruct);		
	}
	
	/* 3. 开启定时器的时钟 */ 
	if(Handle->Init.TIMx == TIM1)
	{
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
	}
	else if(Handle->Init.TIMx == TIM2)
	{
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	}
	else if(Handle->Init.TIMx == TIM3)
	{
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	}
	
	/* 4. 初始化时基单元 */
	
	uint32_t tim_input_clk ; // 定时器的输入时钟
	RCC_ClocksTypeDef RCC_Clocks;
	
	RCC_GetClocksFreq(&RCC_Clocks);
	
	if(Handle->Init.TIMx == TIM1)
	{
		tim_input_clk = RCC_Clocks.PCLK2_Frequency;
		if(RCC_Clocks.HCLK_Frequency / RCC_Clocks.PCLK2_Frequency != 1)
		{
			tim_input_clk *= 2;
		}
	}
	else if(Handle->Init.TIMx == TIM2 || Handle->Init.TIMx == TIM3)
	{
		tim_input_clk = RCC_Clocks.PCLK1_Frequency;
		if(RCC_Clocks.HCLK_Frequency / RCC_Clocks.PCLK1_Frequency != 1)
		{
			tim_input_clk *= 2;
		}
	}
	
	TIM_TimeBaseInitTypeDef TimeBaseInitStruct;
	TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV4;
	TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
	TimeBaseInitStruct.TIM_Prescaler = 56.25 / 1000000.0 * tim_input_clk - 1; // 56.25us / tick
	TimeBaseInitStruct.TIM_Period = 0xffff; // 周期 3.6864s
	TimeBaseInitStruct.TIM_RepetitionCounter = 0;
	
	TIM_TimeBaseInit(Handle->Init.TIMx, &TimeBaseInitStruct);
	
	Handle->Decoder.TickPeriod = 1.0 / tim_input_clk * 1000000 * (TimeBaseInitStruct.TIM_Prescaler + 1);
	
	/* 5. 配置通道1 */
	
	TIM_ICInitTypeDef ICInitStruct;
	ICInitStruct.TIM_Channel = TIM_Channel_1;
	ICInitStruct.TIM_ICFilter = 0xff;
	ICInitStruct.TIM_ICPolarity = TIM_ICPolarity_Falling;
	ICInitStruct.TIM_ICPrescaler = 0;
	ICInitStruct.TIM_ICSelection = TIM_ICSelection_DirectTI;
	TIM_ICInit(Handle->Init.TIMx, &ICInitStruct);
	
	/* 6. 配置通道2 */
	
	ICInitStruct.TIM_Channel = TIM_Channel_2;
	ICInitStruct.TIM_ICFilter = 0xff;
	ICInitStruct.TIM_ICPolarity = TIM_ICPolarity_Rising;
	ICInitStruct.TIM_ICPrescaler = 0;
	ICInitStruct.TIM_ICSelection = TIM_ICSelection_IndirectTI;
	TIM_ICInit(Handle->Init.TIMx, &ICInitStruct);
	
	/* 7. 使能中断: Update, CC1, CC2 */
	
	TIM_ITConfig(Handle->Init.TIMx, TIM_IT_CC1, ENABLE);
	TIM_ITConfig(Handle->Init.TIMx, TIM_IT_CC2, ENABLE);
	TIM_ITConfig(Handle->Init.TIMx, TIM_IT_Update, ENABLE);
	
	NVIC_InitTypeDef NVICInitStruct;
	
	if(Handle->Init.TIMx == TIM1)
	{
		// Update中断
		NVICInitStruct.NVIC_IRQChannel = TIM1_UP_IRQn; 
		NVICInitStruct.NVIC_IRQChannelCmd = ENABLE;
		NVICInitStruct.NVIC_IRQChannelPreemptionPriority = 0;
		NVICInitStruct.NVIC_IRQChannelSubPriority = 0;
		NVIC_Init(&NVICInitStruct);
		// 通道捕获中断
		NVICInitStruct.NVIC_IRQChannel = TIM1_CC_IRQn; 
		NVICInitStruct.NVIC_IRQChannelCmd = ENABLE;
		NVICInitStruct.NVIC_IRQChannelPreemptionPriority = 0;
		NVICInitStruct.NVIC_IRQChannelSubPriority = 0;
		NVIC_Init(&NVICInitStruct);
	}
	else if(Handle->Init.TIMx == TIM2)
	{
		NVICInitStruct.NVIC_IRQChannel = TIM2_IRQn; 
		NVICInitStruct.NVIC_IRQChannelCmd = ENABLE;
		NVICInitStruct.NVIC_IRQChannelPreemptionPriority = 0;
		NVICInitStruct.NVIC_IRQChannelSubPriority = 0;
		NVIC_Init(&NVICInitStruct);
	}
	else if(Handle->Init.TIMx == TIM3)
	{
		NVICInitStruct.NVIC_IRQChannel = TIM3_IRQn; 
		NVICInitStruct.NVIC_IRQChannelCmd = ENABLE;
		NVICInitStruct.NVIC_IRQChannelPreemptionPriority = 0;
		NVICInitStruct.NVIC_IRQChannelSubPriority = 0;
		NVIC_Init(&NVICInitStruct);
	}
	/* 8. 开启定时器 */
	TIM_Cmd(Handle->Init.TIMx, ENABLE);
}

void PAL_IRNEC_Proc(PalIRNEC_HandleTypeDef *Handle)
{
	FlagStatus isPktPending = RESET;
	PalNEC_PacketTypeDef pkt;
	__disable_irq();
	if(Handle->Decoder.RxNE == SET)
	{
		isPktPending = SET;
		pkt = Handle->Decoder.RDR;
		Handle->Decoder.RxNE = RESET;
	}
	__enable_irq();
	
	
	if(isPktPending == SET)
	{
		Handle->Init.PktRcvdCallback(pkt);
	}
}

void PAL_IRNEC_TIMIRQHandler(PalIRNEC_HandleTypeDef *Handle)
{
	uint32_t ccx;
	float duration;
	if(TIM_GetITStatus(Handle->Init.TIMx, TIM_IT_CC1) == SET)// 上升沿
	{
		ccx = TIM_GetCapture1(Handle->Init.TIMx);
		
		if(Handle->Decoder.LastCaptureEdge == CAPTURE_EDGE_FALLING)
		{
			if(Handle->Decoder.Carrier == SET)
			{
				ccx += Handle->Init.TIMx->ARR;
				Handle->Decoder.Carrier = RESET;
			}
			
			duration = (ccx - Handle->Decoder.LastCapture) * Handle->Decoder.TickPeriod;
			
			OnPulseComplete(Handle, duration, PULSE_POLARITY_NEGITIVE);
		}
		else // 无效脉冲
		{
			OnPulseComplete(Handle, 0, PULSE_POLARITY_INVALID);
		}
		Handle->Decoder.LastCapture = ccx;
		Handle->Decoder.LastCaptureEdge = CAPTURE_EDGE_RISING;
	}
	else if(TIM_GetITStatus(Handle->Init.TIMx, TIM_IT_CC2) == SET) // 下降沿
	{
		ccx = TIM_GetCapture2(Handle->Init.TIMx);
		
		if(Handle->Decoder.LastCaptureEdge == CAPTURE_EDGE_RISING)
		{
			if(Handle->Decoder.Carrier == SET)
			{
				ccx += Handle->Init.TIMx->ARR;
				Handle->Decoder.Carrier = RESET;
			}
			
			duration = (ccx - Handle->Decoder.LastCapture) * Handle->Decoder.TickPeriod;
			
			OnPulseComplete(Handle, duration, PULSE_POLARITY_POSITIVE);
		}
		else // 无效脉冲
		{
			OnPulseComplete(Handle, 0, PULSE_POLARITY_INVALID);
		}
		Handle->Decoder.LastCapture = ccx;
		Handle->Decoder.LastCaptureEdge = CAPTURE_EDGE_FALLING;
	}
	else if(TIM_GetITStatus(Handle->Init.TIMx, TIM_IT_Update) == SET) // 下降沿
	{
		TIM_ClearFlag(Handle->Init.TIMx, TIM_IT_Update);
		Handle->Decoder.Carrier = SET;
	}
}

// 取绝对值
#define ABS(__v__) ((__v__) > 0 ? (__v__) : (-(__v__)))
// 与参考值相差x%以内
#define APPROX(__ref__, __v__, __pct__) (ABS(((__ref__) - (__v__))/(__ref__)) < (__pct__) * 0.01)
// 与参考值相差10%以内
#define APPROX10(__ref__, __v__) APPROX(__ref__, __v__, 30)

static void OnPulseComplete(PalIRNEC_HandleTypeDef *Handle, float Duration, uint16_t Polarity)
{	
	Handle->Decoder.TimeElapsedFromStart += Duration;
	
	switch(Handle->Decoder.Stage)
	{
		case DECODER_STAGE_IDLE:
		{
			if(Polarity == PULSE_POLARITY_NEGITIVE && Duration > 108.0e3 * 0.8)
			{
				Handle->Decoder.Addr = 0x00;
				Handle->Decoder.AddrInverse = 0xff;
				Handle->Decoder.Command = 0x00;
				Handle->Decoder.CommandInverse = 0x00;
				Handle->Decoder.BitPos = 0x00;
				Handle->Decoder.BitStage = DECODER_BITSTAGE_POSITIVE_PULSE;
				Handle->Decoder.TimeElapsedFromStart = 0;
				Handle->Decoder.Stage = DECODER_STAGE_START_1;
			}
			else
			{
				__nop();
			}
			break;
		}
		case DECODER_STAGE_START_1:
		{
			if(Polarity == PULSE_POLARITY_POSITIVE && APPROX10(9e3, Duration))
			{
				Handle->Decoder.Stage = DECODER_STAGE_START_2;
			}
			else
			{
				Handle->Decoder.Stage = DECODER_STAGE_IDLE;
			}
			break;
		}
		case DECODER_STAGE_START_2:
		{
			if(Polarity == PULSE_POLARITY_NEGITIVE && APPROX10(4.5e3, Duration))
			{
				Handle->Decoder.BitPos = 0x00;
				Handle->Decoder.BitStage = DECODER_BITSTAGE_POSITIVE_PULSE;
				Handle->Decoder.Stage = DECODER_STAGE_ADDR;
			}
			else if(Polarity == PULSE_POLARITY_NEGITIVE && APPROX10(2.25e3, Duration))
			{
				Handle->Decoder.Addr = Handle->Decoder.RDR.Addr;
				Handle->Decoder.AddrInverse = ~Handle->Decoder.RDR.Addr;
				Handle->Decoder.Command = Handle->Decoder.RDR.Command;
				Handle->Decoder.CommandInverse = ~Handle->Decoder.RDR.Command;
				Handle->Decoder.Stage = DECODER_STAGE_FINAL_PULSE;
			}
			else
			{
				Handle->Decoder.Stage = DECODER_STAGE_IDLE;
			}
			break;
		}
		case DECODER_STAGE_ADDR:
		{
			if(Handle->Decoder.BitStage == DECODER_BITSTAGE_POSITIVE_PULSE 
				&& Polarity == PULSE_POLARITY_POSITIVE  // 收到正脉冲
			  && APPROX10(562.5, Duration)) // 562.5us的脉冲
			{
				Handle->Decoder.BitStage = DECODER_BITSTAGE_NEGITIVE_PULSE;
			}
			else if(Handle->Decoder.BitStage == DECODER_BITSTAGE_NEGITIVE_PULSE 
				&& Polarity == PULSE_POLARITY_NEGITIVE  // 收到正脉冲
			  && APPROX10(562.5, Duration)) // 562.5us的负脉冲，逻辑0
			{
				Handle->Decoder.Addr &= ~(0x01 << Handle->Decoder.BitPos);
				Handle->Decoder.BitPos++;
				Handle->Decoder.BitStage = DECODER_BITSTAGE_POSITIVE_PULSE;
				if(Handle->Decoder.BitPos == 8)
				{
					Handle->Decoder.Stage = DECODER_STAGE_ADDR_INVERSE;
					Handle->Decoder.BitPos = 0;
					Handle->Decoder.BitStage = DECODER_BITSTAGE_POSITIVE_PULSE;
				}
			}
			else if(Handle->Decoder.BitStage == DECODER_BITSTAGE_NEGITIVE_PULSE 
				&& Polarity == PULSE_POLARITY_NEGITIVE  // 收到正脉冲
			  && APPROX10(1687.5, Duration)) // 1687.5us的负脉冲，逻辑1
			{
				Handle->Decoder.Addr |= 0x01 << Handle->Decoder.BitPos;
				Handle->Decoder.BitPos++;
				Handle->Decoder.BitStage = DECODER_BITSTAGE_POSITIVE_PULSE;
				if(Handle->Decoder.BitPos == 8)
				{
					Handle->Decoder.Stage = DECODER_STAGE_ADDR_INVERSE;
					Handle->Decoder.BitPos = 0;
					Handle->Decoder.BitStage = DECODER_BITSTAGE_POSITIVE_PULSE;
				}
			}
			else
			{
				Handle->Decoder.Stage = DECODER_STAGE_IDLE;
			}
			break;
		}
		case DECODER_STAGE_ADDR_INVERSE:
		{
			if(Handle->Decoder.BitStage == DECODER_BITSTAGE_POSITIVE_PULSE 
				&& Polarity == PULSE_POLARITY_POSITIVE  // 收到正脉冲
			  && APPROX10(562.5, Duration)) // 562.5us的脉冲
			{
				Handle->Decoder.BitStage = DECODER_BITSTAGE_NEGITIVE_PULSE;
			}
			else if(Handle->Decoder.BitStage == DECODER_BITSTAGE_NEGITIVE_PULSE 
				&& Polarity == PULSE_POLARITY_NEGITIVE  // 收到正脉冲
			  && APPROX10(562.5, Duration)) // 562.5us的负脉冲，逻辑0
			{
				Handle->Decoder.AddrInverse &= ~(0x01 << Handle->Decoder.BitPos);
				Handle->Decoder.BitPos++;
				Handle->Decoder.BitStage = DECODER_BITSTAGE_POSITIVE_PULSE;
				if(Handle->Decoder.BitPos == 8)
				{
					Handle->Decoder.Stage = DECODER_STAGE_COMMAND;
					Handle->Decoder.BitPos = 0;
					Handle->Decoder.BitStage = DECODER_BITSTAGE_POSITIVE_PULSE;
				}
			}
			else if(Handle->Decoder.BitStage == DECODER_BITSTAGE_NEGITIVE_PULSE 
				&& Polarity == PULSE_POLARITY_NEGITIVE  // 收到正脉冲
			  && APPROX10(1687.5, Duration)) // 1687.5us的负脉冲，逻辑1
			{
				Handle->Decoder.AddrInverse |= 0x01 << Handle->Decoder.BitPos;
				Handle->Decoder.BitPos++;
				Handle->Decoder.BitStage = DECODER_BITSTAGE_POSITIVE_PULSE;
				if(Handle->Decoder.BitPos == 8)
				{
					Handle->Decoder.Stage = DECODER_STAGE_COMMAND;
					Handle->Decoder.BitPos = 0;
					Handle->Decoder.BitStage = DECODER_BITSTAGE_POSITIVE_PULSE;
				}
			}
			else
			{
				Handle->Decoder.Stage = DECODER_STAGE_IDLE;
			}
			break;
		}
		case DECODER_STAGE_COMMAND:
		{
			if(Handle->Decoder.BitStage == DECODER_BITSTAGE_POSITIVE_PULSE 
				&& Polarity == PULSE_POLARITY_POSITIVE  // 收到正脉冲
			  && APPROX10(562.5, Duration)) // 562.5us的脉冲
			{
				Handle->Decoder.BitStage = DECODER_BITSTAGE_NEGITIVE_PULSE;
			}
			else if(Handle->Decoder.BitStage == DECODER_BITSTAGE_NEGITIVE_PULSE 
				&& Polarity == PULSE_POLARITY_NEGITIVE  // 收到正脉冲
			  && APPROX10(562.5, Duration)) // 562.5us的负脉冲，逻辑0
			{
				Handle->Decoder.Command &= ~(0x01 << Handle->Decoder.BitPos);
				Handle->Decoder.BitPos++;
				Handle->Decoder.BitStage = DECODER_BITSTAGE_POSITIVE_PULSE;
				if(Handle->Decoder.BitPos == 8)
				{
					Handle->Decoder.BitPos = 0;
					Handle->Decoder.BitStage = DECODER_BITSTAGE_POSITIVE_PULSE;
					Handle->Decoder.Stage = DECODER_STAGE_COMMAND_INVERSE;
				}
			}
			else if(Handle->Decoder.BitStage == DECODER_BITSTAGE_NEGITIVE_PULSE 
				&& Polarity == PULSE_POLARITY_NEGITIVE  // 收到正脉冲
			  && APPROX10(1687.5, Duration)) // 1687.5us的负脉冲，逻辑1
			{
				Handle->Decoder.Command |= 0x01 << Handle->Decoder.BitPos;
				Handle->Decoder.BitPos++;
				Handle->Decoder.BitStage = DECODER_BITSTAGE_POSITIVE_PULSE;
				if(Handle->Decoder.BitPos == 8)
				{
					Handle->Decoder.BitPos = 0;
					Handle->Decoder.BitStage = DECODER_BITSTAGE_POSITIVE_PULSE;
					Handle->Decoder.Stage = DECODER_STAGE_COMMAND_INVERSE;
				}
			}
			else
			{
				Handle->Decoder.Stage = DECODER_STAGE_IDLE;
			}
			break;
		}
		case DECODER_STAGE_COMMAND_INVERSE:
		{
			if(Handle->Decoder.BitStage == DECODER_BITSTAGE_POSITIVE_PULSE 
				&& Polarity == PULSE_POLARITY_POSITIVE  // 收到正脉冲
			  && APPROX10(562.5, Duration)) // 562.5us的脉冲
			{
				Handle->Decoder.BitStage = DECODER_BITSTAGE_NEGITIVE_PULSE;
			}
			else if(Handle->Decoder.BitStage == DECODER_BITSTAGE_NEGITIVE_PULSE 
				&& Polarity == PULSE_POLARITY_NEGITIVE  // 收到正脉冲
			  && APPROX10(562.5, Duration)) // 562.5us的负脉冲，逻辑0
			{
				Handle->Decoder.CommandInverse &= ~(0x01 << Handle->Decoder.BitPos);
				Handle->Decoder.BitPos++;
				Handle->Decoder.BitStage = DECODER_BITSTAGE_POSITIVE_PULSE;
				if(Handle->Decoder.BitPos == 8)
				{
					Handle->Decoder.BitPos = 0;
					Handle->Decoder.BitStage = DECODER_BITSTAGE_POSITIVE_PULSE;
					Handle->Decoder.Stage = DECODER_STAGE_FINAL_PULSE;
				}
			}
			else if(Handle->Decoder.BitStage == DECODER_BITSTAGE_NEGITIVE_PULSE 
				&& Polarity == PULSE_POLARITY_NEGITIVE  // 收到正脉冲
			  && APPROX10(1687.5, Duration)) // 1687.5us的负脉冲，逻辑1
			{
				Handle->Decoder.CommandInverse |= 0x01 << Handle->Decoder.BitPos;
				Handle->Decoder.BitPos++;
				Handle->Decoder.BitStage = DECODER_BITSTAGE_POSITIVE_PULSE;
				if(Handle->Decoder.BitPos == 8)
				{
					Handle->Decoder.BitPos = 0;
					Handle->Decoder.BitStage = DECODER_BITSTAGE_POSITIVE_PULSE;
					Handle->Decoder.Stage = DECODER_STAGE_FINAL_PULSE;
				}
			}
			else
			{
				Handle->Decoder.Stage = DECODER_STAGE_IDLE;
			}
			break;
		}
		case DECODER_STAGE_FINAL_PULSE:
		{
			if(Polarity == PULSE_POLARITY_POSITIVE && APPROX10(562.5, Duration))
			{
				Handle->Decoder.RDR.Addr = Handle->Decoder.Addr;
				Handle->Decoder.RDR.Command = Handle->Decoder.Command;
				if(((Handle->Decoder.Addr ^ Handle->Decoder.AddrInverse) == 0xff) 
					&& ((Handle->Decoder.Command ^ Handle->Decoder.CommandInverse) == 0xff)
				)
				{
					Handle->Decoder.RDR.Valid = SET;
				}
				else
				{
					Handle->Decoder.RDR.Valid = RESET;
				}
				Handle->Decoder.RDR.TimeStamp = PAL_GetTick();
				Handle->Decoder.RxNE = SET;
				Handle->Decoder.Stage = DECODER_STAGE_IDLE;
			}
			else
			{
				Handle->Decoder.Stage = DECODER_STAGE_IDLE;
			}
			break;
		}
		default:
			break;
	}
}
