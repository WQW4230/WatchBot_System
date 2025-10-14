/**
  ******************************************************************************
  * @file    stm32f10x_pal_adc.h
  * @author  铁头山羊
  * @version V 1.0.0
  * @date    2023年4月20日
  * @brief   外设抽象层ADC驱动程序头文件
  ******************************************************************************
  */
	
#ifndef _STM32F10x_PAL_ADC_H_
#define _STM32F10x_PAL_ADC_H_

#include "stm32f10x.h"
#include "stm32f10x_pal.h"

#define PAL_ADC_MODE_LOWPOWER    0x00 // 低功耗（休眠）
#define PAL_ADC_MODE_SINGLE      0x01 // 单通道、软件触发
#define PAL_ADC_MODE_SINGLE_TRI  0x02 // 单通道、外部信号触发
#define PAL_ADC_MODE_SINGLE_CONT 0x03 // 单通道、连续采样
#define PAL_ADC_MODE_SCAN        0x04 // 多通道扫描、软件触发
#define PAL_ADC_MODE_SCAN_TRI    0x05 // 多通道扫描、外部信号触发
#define PAL_ADC_MODE_SCAN_CONT   0x06 // 多通道扫描、连续采样

#define ADC_CLOCK_DIV_2 RCC_PCLK2_Div2 // ADC时钟=PCLK2/2
#define ADC_CLOCK_DIV_4 RCC_PCLK2_Div4 // ADC时钟=PCLK2/4
#define ADC_CLOCK_DIV_6 RCC_PCLK2_Div6 // ADC时钟=PCLK2/6
#define ADC_CLOCK_DIV_8 RCC_PCLK2_Div8 // ADC时钟=PCLK2/8

typedef struct
{
	uint32_t ADCIRQ_PreemptionPriority;
	uint32_t ADCIRQ_SubPriority;
} PalADC_AdvancedInitTypeDef;

typedef struct
{
	ADC_TypeDef *ADCx;
	uint32_t ADC_ClockDiv;
	uint8_t ChannelSampleTime[18];
	PalADC_AdvancedInitTypeDef Advanced;
} PalADC_InitTypeDef;

typedef struct
{
	PalADC_InitTypeDef Init;
	uint8_t Mode;
	void (*SingleConvertCb)(float);
} PalADC_HandleTypeDef;

void PAL_ADC_Init(PalADC_HandleTypeDef *Handle);

void PAL_ADC_Calibrate(PalADC_HandleTypeDef *Handle);

void PAL_ADC_LowPowerMode_Enter(PalADC_HandleTypeDef *Handle);

 void PAL_ADC_SingleMode_Enter(PalADC_HandleTypeDef *Handle, uint8_t ADC_Channel);
float PAL_ADC_SingleMode_Convert(PalADC_HandleTypeDef *Handle);

void PAL_ADC_SingleTriggerMode_Enter(PalADC_HandleTypeDef *Handle, uint8_t ADC_Channel, uint32_t ADC_TriggerSource, void (*Cb)(float));
void PAL_ADC_SingleTriggerMode_Start(PalADC_HandleTypeDef *Handle);
void PAL_ADC_SingleTriggerMode_SoftwareTrigger(PalADC_HandleTypeDef *Handle);

void PAL_ADC_SingleContinuousMode_Enter(PalADC_HandleTypeDef *Handle, uint8_t ADC_Channel, void (*Cb)(float));
void PAL_ADC_SingleContinuousMode_Start(PalADC_HandleTypeDef *Handle);

void PAL_ADC_ScanMode_Enter(PalADC_HandleTypeDef *Handle, uint8_t *pChannels);
void PAL_ADC_ScanMode_Convert(PalADC_HandleTypeDef *Handle, float *pResultOut);

void PAL_ADC_ScanTriggerMode_Enter(PalADC_HandleTypeDef *Handle, uint8_t *pChannels, uint32_t ADC_TriggerSource, void (*Cb)(float *));
void PAL_ADC_ScanTriggerMode_Start(PalADC_HandleTypeDef *Handle);
void PAL_ADC_ScanTriggerMode_SoftwareTrigger(PalADC_HandleTypeDef *Handle);

void PAL_ADC_ScanContinuousMode_Enter(PalADC_HandleTypeDef *Handle, uint8_t *pChannels, void (*Cb)(float *));
void PAL_ADC_ScanContinuousMode_Start(PalADC_HandleTypeDef *Handle);

void PAL_ADC_ADCIRQHandler(PalADC_HandleTypeDef *Handle);
void PAL_ADC_DMAIRQHandler(PalADC_HandleTypeDef *Handle);

#endif
