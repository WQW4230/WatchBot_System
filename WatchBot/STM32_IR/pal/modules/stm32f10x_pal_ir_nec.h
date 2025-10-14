/**
  ******************************************************************************
  * @file    stm32f10x_pal_ir_nec.h
  * @author  铁头山羊stm32工作组
  * @version V1.0.0
  * @date    2023年2月24日
  * @brief   红外遥控驱动
  ******************************************************************************
  * @attention
  * Copyright (c) 2022 - 2023 东莞市明玉科技有限公司. 保留所有权力.
  ******************************************************************************
*/

#ifndef _STM32F10x_PAL_IR_NEC_H_
#define _STM32F10x_PAL_IR_NEC_H_

#include "stm32f10x.h"

typedef struct
{
	uint8_t Addr;
	uint8_t Command;
	uint64_t TimeStamp;
	FlagStatus Valid;
} PalNEC_PacketTypeDef;

typedef struct
{
	TIM_TypeDef *TIMx;
	void (*PktRcvdCallback)(PalNEC_PacketTypeDef);
}PalIRNEC_InitTypeDef;

typedef struct
{
	uint16_t Stage;
	FlagStatus Carrier; // 定时器是否溢出
	uint16_t LastCapture; // 上次捕获值
	uint16_t LastCaptureEdge; // 上次捕获边沿
	float TickPeriod; // 定时器每个Tick对应的时间，单位us
	float TimeElapsedFromStart;
	uint16_t BitPos; // 当前解码的是第几个比特
	uint16_t BitStage; // 当前比特的解码阶段（阶段1-正脉冲；阶段2：负脉冲）
	uint8_t Addr;
	uint8_t AddrInverse;
	uint8_t Command;
	uint8_t CommandInverse;
	PalNEC_PacketTypeDef RDR; // 接收到的数据包
	FlagStatus RxNE; // 接收缓冲区非空
}PalIRNEC_DecoderTypeDef;

typedef struct
{
	PalIRNEC_InitTypeDef Init;
	PalIRNEC_DecoderTypeDef Decoder; // 解码器状态记录
}PalIRNEC_HandleTypeDef;

void PAL_IRNEC_Init(PalIRNEC_HandleTypeDef *Handle);
void PAL_IRNEC_TIMIRQHandler(PalIRNEC_HandleTypeDef *Handle);
void PAL_IRNEC_Proc(PalIRNEC_HandleTypeDef *Handle);

#endif
