/**
  ******************************************************************************
  * @file    stm32f10x_pal_drv8833_dual_dc.h
  * @author  铁头山羊stm32工作组
  * @version V1.0.0
  * @date    2023年2月24日
  * @brief   DRV8833（双直流电机）驱动程序
  ******************************************************************************
  * @attention
  * Copyright (c) 2022 - 2023 东莞市明玉科技有限公司. 保留所有权力.
  ******************************************************************************
*/

#ifndef _STM32F10x_PAL_DRV8833_DUAL_DC_H_
#define _STM32F10x_PAL_DRV8833_DUAL_DC_H_
#include "stm32f10x.h"
#include "pal_iir_filter.h"

typedef struct
{
	FlagStatus DisableStandbyPin;
	uint32_t MotorTIMRemap; // 00 - no remap 01, 02 ,03 - (partital) remap
	uint32_t Encoder1TIMRemap; // 00 - no remap 01, 02 ,03 - (partital) remap
	uint32_t Encoder2TIMRemap; // 00 - no remap 01, 02 ,03 - (partital) remap
} PalDRV8833DualDC_AdvancedInitTypeDef;

typedef struct
{
	TIM_TypeDef *TIMx; // 使用定时器的通道1、通道2、通道3、通道4
	FlagStatus Motor1Reverse;
	FlagStatus Motor2Reverse;
	GPIO_TypeDef *StandbyGPIOPort;
	uint16_t StandbyGPIOPin;
}PalDRV8833DualDC_PWMInitTypeDef;

typedef struct{
	TIM_TypeDef *Encoder1TIMx; // 使用定时器的通道1和通道2
	TIM_TypeDef *Encoder2TIMx; // 使用定时器的通道1和通道2
	FlagStatus Encoder1Reverse; // 编码器1反向
	FlagStatus Encoder2Reverse; // 编码器2反向
	uint32_t SampleInterval; // 采样间隔(ms)
	uint32_t TickPerTurn; // 每转1圈编码器变化的数值
}PalDRV8833DualDC_EncoderInitTypeDef;

typedef struct{
	PalDRV8833DualDC_PWMInitTypeDef PWM;
	PalDRV8833DualDC_EncoderInitTypeDef Encoder;
	PalDRV8833DualDC_AdvancedInitTypeDef Advanced;
}PalDRV8833DualDC_InitTypeDef;

typedef struct
{
	PalDRV8833DualDC_InitTypeDef Init;
	PalIIRFilter_HandleTypeDef Encoder1_lpf;
	PalIIRFilter_HandleTypeDef Encoder2_lpf;
	uint64_t NextSampleTime;
	uint16_t Encoder1TIMxLastCnt;
	uint16_t Encoder2TIMxLastCnt;
	float speed1;
	float speed2;
	float k;
}PalDRV8833DualDC_HandleTypeDef;

void PAL_Drv8833DualDC_Init(PalDRV8833DualDC_HandleTypeDef *Handle);
void PAL_Drv8833DualDC_SleepModeConfig(PalDRV8833DualDC_HandleTypeDef *Handle, FunctionalState NewState);
void PAL_Drv8833DualDC_SetMotor1PWMDuty(PalDRV8833DualDC_HandleTypeDef *Handle, float Speed);
void PAL_Drv8833DualDC_SetMotor2PWMDuty(PalDRV8833DualDC_HandleTypeDef *Handle, float Speed);
void PAL_Drv8833DualDC_SetMotorPWMDuty(PalDRV8833DualDC_HandleTypeDef *Handle, float Motor1Speed, float Motor2Speed);
float PAL_Drv8833DualDC_GetMotor1Speed(PalDRV8833DualDC_HandleTypeDef *Handle);
float PAL_Drv8833DualDC_GetMotor2Speed(PalDRV8833DualDC_HandleTypeDef *Handle);
void PAL_Drv8833DualDC_SpeedTestProc(PalDRV8833DualDC_HandleTypeDef *Handle);

#endif
