/**
  ******************************************************************************
  * @file    stm32f10x_pal_adc.c
  * @author  铁头山羊
  * @version V 1.0.0
  * @date    2023年4月24日
  * @brief   adc驱动程序
  ******************************************************************************
  * @attention
  * Copyright (c) 2022 - 2023 东莞市明玉科技有限公司. 保留所有权力.
  ******************************************************************************
*/
#include "stm32f10x_pal_adc.h"

#define PAL_ADC_RANK_1  0
#define PAL_ADC_RANK_2  1
#define PAL_ADC_RANK_3  2
#define PAL_ADC_RANK_4  3
#define PAL_ADC_RANK_5  4
#define PAL_ADC_RANK_6  5
#define PAL_ADC_RANK_7  6
#define PAL_ADC_RANK_8  7
#define PAL_ADC_RANK_9  7
#define PAL_ADC_RANK_10 7
#define PAL_ADC_RANK_11 7
#define PAL_ADC_RANK_12 7
#define PAL_ADC_RANK_13 7
#define PAL_ADC_RANK_14 7
#define PAL_ADC_RANK_15 7
#define PAL_ADC_RANK_16 7

static void PAL_ADC_Cmd(PalADC_HandleTypeDef *Handle, FunctionalState NewState);
static void PAL_ADC_SetRegChannelRank(PalADC_HandleTypeDef *Handle, uint8_t ADC_Channel, uint8_t ADC_Rank);
static void PAL_ADC_SetRegSeqenceLength(PalADC_HandleTypeDef *Handle, uint8_t L);

void PAL_ADC_Init(PalADC_HandleTypeDef *Handle)
{
	// 设置分频系数
	RCC_ADCCLKConfig(Handle->Init.ADC_ClockDiv);
	
	// 启动ADC的时钟
	if(Handle->Init.ADCx == ADC1)
	{
		// 复位
		RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC1, ENABLE);
		RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC1, DISABLE);
		// 开启时钟
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	}
	else if(Handle->Init.ADCx == ADC2)
	{
		// 复位
		RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC2, ENABLE);
		RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC2, DISABLE);
		// 开启时钟
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC2, ENABLE);
	}
	#if defined(STM32F10X_HD) || defined(STM32F10X_XL) // 只有高密度或者超大密度的器件才有ADC3
	else if(Handle->Init.ADCx == ADC3)
	{
		RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC3, ENABLE);
		RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC3, DISABLE);
		// 开启时钟
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC3, ENABLE);
	}
	#endif
	
	Handle->Mode = PAL_ADC_MODE_LOWPOWER; // 初始状态下ADC处于低功耗模式
	
	// 设置采样时间
	uint32_t smpr1 = 0, smpr2 = 0;
	smpr2 |= (Handle->Init.ChannelSampleTime[0]  & 0x07) << (0 * 3);       // 通道0
	smpr2 |= (Handle->Init.ChannelSampleTime[1]  & 0x07) << (1 * 3);       // 通道1
	smpr2 |= (Handle->Init.ChannelSampleTime[2]  & 0x07) << (2 * 3);       // 通道2
	smpr2 |= (Handle->Init.ChannelSampleTime[3]  & 0x07) << (3 * 3);       // 通道3
	smpr2 |= (Handle->Init.ChannelSampleTime[4]  & 0x07) << (4 * 3);       // 通道4
	smpr2 |= (Handle->Init.ChannelSampleTime[5]  & 0x07) << (5 * 3);       // 通道5
	smpr2 |= (Handle->Init.ChannelSampleTime[6]  & 0x07) << (6 * 3);       // 通道6
	smpr2 |= (Handle->Init.ChannelSampleTime[7]  & 0x07) << (7 * 3);       // 通道7
	smpr2 |= (Handle->Init.ChannelSampleTime[8]  & 0x07) << (8 * 3);       // 通道8
	smpr2 |= (Handle->Init.ChannelSampleTime[9]  & 0x07) << (9 * 3);       // 通道9
	smpr1 |= (Handle->Init.ChannelSampleTime[10] & 0x07) << ((10-10) * 3); // 通道10
	smpr1 |= (Handle->Init.ChannelSampleTime[11] & 0x07) << ((11-10) * 3); // 通道11
	smpr1 |= (Handle->Init.ChannelSampleTime[12] & 0x07) << ((12-10) * 3); // 通道12
	smpr1 |= (Handle->Init.ChannelSampleTime[13] & 0x07) << ((13-10) * 3); // 通道13
	smpr1 |= (Handle->Init.ChannelSampleTime[14] & 0x07) << ((14-10) * 3); // 通道14
	smpr1 |= (Handle->Init.ChannelSampleTime[15] & 0x07) << ((15-10) * 3); // 通道15
	smpr1 |= (Handle->Init.ChannelSampleTime[16] & 0x07) << ((16-10) * 3); // 通道16
	smpr1 |= (Handle->Init.ChannelSampleTime[17] & 0x07) << ((17-10) * 3); // 通道17
	Handle->Init.ADCx->SMPR1 = smpr1;
	Handle->Init.ADCx->SMPR2 = smpr2;
}

void PAL_ADC_Calibrate(PalADC_HandleTypeDef *Handle)
{
	volatile uint8_t tmp;
	RCC_ClocksTypeDef RCCClockStruct;
	
	PAL_ADC_Cmd(Handle, DISABLE); // 关闭ADC
	
	// 等待2个ADC周期后开始校准
	RCC_GetClocksFreq(&RCCClockStruct);
	tmp = (RCCClockStruct.ADCCLK_Frequency / RCCClockStruct.PCLK2_Frequency)  * 2; 
	
	while(tmp != 0)
	{
		tmp--;
	}
	
	PAL_ADC_Cmd(Handle, ENABLE); // 使能ADC
	
	// 复位ADC校准寄存器
	Handle->Init.ADCx->CR2 |= ADC_CR2_RSTCAL;
	
	while(Handle->Init.ADCx->CR2 & ADC_CR2_RSTCAL); // 等待复位完成
	
	// 开始校准
	Handle->Init.ADCx->CR2 |= ADC_CR2_CAL;
	
	while(Handle->Init.ADCx->CR2 & ADC_CR2_CAL); // 等待校准完成
}

void PAL_ADC_LowPowerMode_Enter(PalADC_HandleTypeDef *Handle)
{
	NVIC_InitTypeDef NVICInitStruct;
	
	PAL_ADC_Cmd(Handle, DISABLE); // 关闭ADC
	
	// 释放资源
	switch(Handle->Mode) // 待补充
	{
		case PAL_ADC_MODE_SINGLE:
		{
			break;
		}
		case PAL_ADC_MODE_SINGLE_TRI:
		case PAL_ADC_MODE_SINGLE_CONT:
		{
			// 关闭中断
			ADC_ITConfig(Handle->Init.ADCx, ADC_IT_EOC, DISABLE); 
			
			NVICInitStruct.NVIC_IRQChannel = ADC1_2_IRQn;
			NVICInitStruct.NVIC_IRQChannelPreemptionPriority = Handle->Init.Advanced.ADCIRQ_PreemptionPriority;
			NVICInitStruct.NVIC_IRQChannelSubPriority = Handle->Init.Advanced.ADCIRQ_SubPriority;
			NVICInitStruct.NVIC_IRQChannelCmd = DISABLE;
			
			NVIC_Init(&NVICInitStruct);
		}
		default:break;
	}
}

void PAL_ADC_SingleMode_Enter(PalADC_HandleTypeDef *Handle, uint8_t ADC_Channel)
{
	// 如果之前处于别的模式，先进入低功耗模式
	if(Handle->Mode != PAL_ADC_MODE_LOWPOWER && Handle->Mode != PAL_ADC_MODE_SINGLE)
	{
		PAL_ADC_LowPowerMode_Enter(Handle); 
	}
	
	Handle->Mode = PAL_ADC_MODE_SINGLE;
	
	ADC_InitTypeDef ADCInitStruct;
	
	ADCInitStruct.ADC_Mode = ADC_Mode_Independent; // 禁止双ADC模式
	ADCInitStruct.ADC_ScanConvMode = DISABLE; // 禁止扫描模式
	ADCInitStruct.ADC_ContinuousConvMode = DISABLE; // 禁止连续模式
	ADCInitStruct.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; // 软件触发
	ADCInitStruct.ADC_DataAlign = ADC_DataAlign_Right; // 右对齐
	ADCInitStruct.ADC_NbrOfChannel = 1; // 单通道
	
	ADC_ExternalTrigConvCmd(Handle->Init.ADCx, ENABLE);
	
	ADC_Init(Handle->Init.ADCx, &ADCInitStruct);
	
		GPIO_InitTypeDef GPIOInitStruct;
	
	// 配置对应的GPIO为模拟模式
	if(ADC_Channel <= ADC_Channel_7) // PA0 ~ PA7
	{
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
		GPIOInitStruct.GPIO_Pin = 0x01 << ADC_Channel;
		GPIOInitStruct.GPIO_Mode = GPIO_Mode_AIN;
		GPIO_Init(GPIOA, &GPIOInitStruct);
	}
	else if(ADC_Channel <= ADC_Channel_9) // PB0 PB1
	{
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
		GPIOInitStruct.GPIO_Pin = 0x01 << (ADC_Channel - 8);
		GPIOInitStruct.GPIO_Mode = GPIO_Mode_AIN;
		GPIO_Init(GPIOB, &GPIOInitStruct);
	}
	else if(ADC_Channel <= ADC_Channel_15) // PC0 ~ PC5
	{
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
		GPIOInitStruct.GPIO_Pin = 0x01 << (ADC_Channel - 10);
		GPIOInitStruct.GPIO_Mode = GPIO_Mode_AIN;
		GPIO_Init(GPIOC, &GPIOInitStruct);
	}
	else // 通道16 -> 温度 通道17 ->参考电压
	{
		ADC_TempSensorVrefintCmd(ENABLE);
	}
	
	// 将序列的第一个通道设置为该通道
	PAL_ADC_SetRegSeqenceLength(Handle, 1);
	PAL_ADC_SetRegChannelRank(Handle, ADC_Channel, PAL_ADC_RANK_1); 
}

float PAL_ADC_SingleMode_Convert(PalADC_HandleTypeDef *Handle)
{	
	// 使能ADC
	PAL_ADC_Cmd(Handle, ENABLE);
	
	// 先清除EOC标志位
	ADC_ClearFlag(Handle->Init.ADCx, ADC_FLAG_EOC);
	
	ADC_SoftwareStartConvCmd(Handle->Init.ADCx, ENABLE);
	
	while(ADC_GetFlagStatus(Handle->Init.ADCx, ADC_FLAG_EOC) != SET); // 等待转换完成
	
	return (float)(ADC_GetConversionValue(Handle->Init.ADCx) & 0xffff) / 4096;
}

void PAL_ADC_SingleTriggerMode_Enter(PalADC_HandleTypeDef *Handle, uint8_t ADC_Channel, uint32_t ADC_TriggerSource, void (*Cb)(float))
{
	GPIO_InitTypeDef GPIOInitStruct;
	
	// 如果之前处于别的模式，先进入低功耗模式
	if(Handle->Mode != PAL_ADC_MODE_LOWPOWER && Handle->Mode != PAL_ADC_MODE_SINGLE)
	{
		PAL_ADC_LowPowerMode_Enter(Handle); 
	}
	
	Handle->Mode = PAL_ADC_MODE_SINGLE_TRI;
	
	Handle->SingleConvertCb = Cb;
	
	// 配置对应的GPIO为模拟模式
	if(ADC_Channel <= ADC_Channel_7) // PA0 ~ PA7
	{
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
		GPIOInitStruct.GPIO_Pin = 0x01 << ADC_Channel;
		GPIOInitStruct.GPIO_Mode = GPIO_Mode_AIN;
		GPIO_Init(GPIOA, &GPIOInitStruct);
	}
	else if(ADC_Channel <= ADC_Channel_9) // PB0 PB1
	{
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
		GPIOInitStruct.GPIO_Pin = 0x01 << (ADC_Channel - 8);
		GPIOInitStruct.GPIO_Mode = GPIO_Mode_AIN;
		GPIO_Init(GPIOB, &GPIOInitStruct);
	}
	else if(ADC_Channel <= ADC_Channel_15) // PC0 ~ PC5
	{
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
		GPIOInitStruct.GPIO_Pin = 0x01 << (ADC_Channel - 10);
		GPIOInitStruct.GPIO_Mode = GPIO_Mode_AIN;
		GPIO_Init(GPIOC, &GPIOInitStruct);
	}
	else // 通道16 -> 温度 通道17 ->参考电压
	{
		ADC_TempSensorVrefintCmd(ENABLE);
	}
	
	// 将序列的第一个通道设置为该通道
	PAL_ADC_SetRegSeqenceLength(Handle, 1);
	PAL_ADC_SetRegChannelRank(Handle, ADC_Channel, PAL_ADC_RANK_1); 
	
	ADC_InitTypeDef ADCInitStruct;
	
	ADCInitStruct.ADC_Mode = ADC_Mode_Independent; // 禁止双ADC模式
	ADCInitStruct.ADC_ScanConvMode = DISABLE; // 禁止扫描模式
	ADCInitStruct.ADC_ContinuousConvMode = DISABLE; // 禁止连续模式
	ADCInitStruct.ADC_ExternalTrigConv = ADC_TriggerSource; // 选择触发模式
	ADCInitStruct.ADC_DataAlign = ADC_DataAlign_Right; // 右对齐
	ADCInitStruct.ADC_NbrOfChannel = 1; // 单通道
	
	ADC_ExternalTrigConvCmd(Handle->Init.ADCx, ENABLE);
	
	ADC_Init(Handle->Init.ADCx, &ADCInitStruct);
	
	// 配置中断
	ADC_ITConfig(Handle->Init.ADCx, ADC_IT_EOC, ENABLE); // 置位EOCIE
	
	NVIC_InitTypeDef NVICInitStruct;
	NVICInitStruct.NVIC_IRQChannel = ADC1_2_IRQn;
	NVICInitStruct.NVIC_IRQChannelPreemptionPriority = Handle->Init.Advanced.ADCIRQ_PreemptionPriority;
	NVICInitStruct.NVIC_IRQChannelSubPriority = Handle->Init.Advanced.ADCIRQ_SubPriority;
	NVICInitStruct.NVIC_IRQChannelCmd = ENABLE;
	
	NVIC_Init(&NVICInitStruct);
}
void PAL_ADC_SingleTriggerMode_Start(PalADC_HandleTypeDef *Handle)
{
	// 先清除EOC标志位
	ADC_ClearFlag(Handle->Init.ADCx, ADC_FLAG_EOC);
	// 使能ADC
	PAL_ADC_Cmd(Handle, ENABLE);
}

void PAL_ADC_SingleTriggerMode_SoftwareTrigger(PalADC_HandleTypeDef *Handle)
{
	ADC_SoftwareStartConvCmd(Handle->Init.ADCx, ENABLE);
}

void PAL_ADC_SingleContinuousMode_Enter(PalADC_HandleTypeDef *Handle, uint8_t ADC_Channel, void (*Cb)(float))
{
	GPIO_InitTypeDef GPIOInitStruct;
	
	// 如果之前处于别的模式，先进入低功耗模式
	if(Handle->Mode != PAL_ADC_MODE_LOWPOWER && Handle->Mode != PAL_ADC_MODE_SINGLE)
	{
		PAL_ADC_LowPowerMode_Enter(Handle); 
	}
	
	Handle->Mode = PAL_ADC_MODE_SINGLE_CONT;
	
	Handle->SingleConvertCb = Cb;
	
	// 配置对应的GPIO为模拟模式
	if(ADC_Channel <= ADC_Channel_7) // PA0 ~ PA7
	{
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
		GPIOInitStruct.GPIO_Pin = 0x01 << ADC_Channel;
		GPIOInitStruct.GPIO_Mode = GPIO_Mode_AIN;
		GPIO_Init(GPIOA, &GPIOInitStruct);
	}
	else if(ADC_Channel <= ADC_Channel_9) // PB0 PB1
	{
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
		GPIOInitStruct.GPIO_Pin = 0x01 << (ADC_Channel - 8);
		GPIOInitStruct.GPIO_Mode = GPIO_Mode_AIN;
		GPIO_Init(GPIOB, &GPIOInitStruct);
	}
	else if(ADC_Channel <= ADC_Channel_15) // PC0 ~ PC5
	{
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
		GPIOInitStruct.GPIO_Pin = 0x01 << (ADC_Channel - 10);
		GPIOInitStruct.GPIO_Mode = GPIO_Mode_AIN;
		GPIO_Init(GPIOC, &GPIOInitStruct);
	}
	else // 通道16 -> 温度 通道17 ->参考电压
	{
		ADC_TempSensorVrefintCmd(ENABLE);
	}
	
	// 将序列的第一个通道设置为该通道
	PAL_ADC_SetRegSeqenceLength(Handle, 1);
	PAL_ADC_SetRegChannelRank(Handle, ADC_Channel, PAL_ADC_RANK_1); 
	
	ADC_InitTypeDef ADCInitStruct;
	
	ADCInitStruct.ADC_Mode = ADC_Mode_Independent; // 禁止双ADC模式
	ADCInitStruct.ADC_ScanConvMode = DISABLE; // 禁止扫描模式
	ADCInitStruct.ADC_ContinuousConvMode = ENABLE; // 禁止连续模式
	ADCInitStruct.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; // 选择触发模式
	ADCInitStruct.ADC_DataAlign = ADC_DataAlign_Right; // 右对齐
	ADCInitStruct.ADC_NbrOfChannel = 1; // 单通道
	
	ADC_ExternalTrigConvCmd(Handle->Init.ADCx, ENABLE);
	
	ADC_Init(Handle->Init.ADCx, &ADCInitStruct);
	
	// 配置中断
	ADC_ITConfig(Handle->Init.ADCx, ADC_IT_EOC, ENABLE); // 置位EOCIE
	
	NVIC_InitTypeDef NVICInitStruct;
	NVICInitStruct.NVIC_IRQChannel = ADC1_2_IRQn;
	NVICInitStruct.NVIC_IRQChannelPreemptionPriority = Handle->Init.Advanced.ADCIRQ_PreemptionPriority;
	NVICInitStruct.NVIC_IRQChannelSubPriority = Handle->Init.Advanced.ADCIRQ_SubPriority;
	NVICInitStruct.NVIC_IRQChannelCmd = ENABLE;
	
	NVIC_Init(&NVICInitStruct);
}

void PAL_ADC_SingleContinuousMode_Start(PalADC_HandleTypeDef *Handle)
{
	// 先清除EOC标志位
	ADC_ClearFlag(Handle->Init.ADCx, ADC_FLAG_EOC);
	// 使能ADC
	PAL_ADC_Cmd(Handle, ENABLE);
	// 开启转换
	Handle->Init.ADCx->CR2 |= ADC_CR2_ADON;
}

static void PAL_ADC_Cmd(PalADC_HandleTypeDef *Handle, FunctionalState NewState)
{
	if(NewState == ENABLE)
	{
		if(Handle->Init.ADCx->CR2 & ADC_CR2_ADON)
		{
			return; // 原本就处于开启状态
		}
		
		Handle->Init.ADCx->CR2 |= ADC_CR2_ADON; // 开启ADC
		
		while(!(Handle->Init.ADCx->CR2 & ADC_CR2_ADON)); // 等待ADC开启
	}
	else
	{
		if(!(Handle->Init.ADCx->CR2 & ADC_CR2_ADON))
		{
			return; // 原本就处于关闭状态
		}
		
		Handle->Init.ADCx->CR2 &= ~ADC_CR2_ADON; // 关闭ADC
		
		while(Handle->Init.ADCx->CR2 & ADC_CR2_ADON); // 等待ADC关闭
	}
}

static void PAL_ADC_SetRegChannelRank(PalADC_HandleTypeDef *Handle, uint8_t ADC_Channel, uint8_t ADC_Rank)
{
	uint32_t sqrcpy;
	if(ADC_Rank <= PAL_ADC_RANK_6) 
	{
		sqrcpy  = Handle->Init.ADCx->SQR3;
		sqrcpy &= ~(0x1f << (ADC_Rank * 5));
		sqrcpy |= ADC_Channel << (ADC_Rank * 5);
		Handle->Init.ADCx->SQR3 = sqrcpy;
	}
	else if(ADC_Rank <= PAL_ADC_RANK_12)
	{
		sqrcpy  = Handle->Init.ADCx->SQR2;
		sqrcpy &= ~(0x1f << ((ADC_Rank - PAL_ADC_RANK_7) * 5));
		sqrcpy |= ADC_Channel << ((ADC_Rank - PAL_ADC_RANK_7) * 5);
		Handle->Init.ADCx->SQR2 = sqrcpy;
	}
	else
	{
		sqrcpy  = Handle->Init.ADCx->SQR1;
		sqrcpy &= ~(0x1f << ((ADC_Rank - PAL_ADC_RANK_13) * 5));
		sqrcpy |= ADC_Channel << ((ADC_Rank - PAL_ADC_RANK_13) * 5);
		Handle->Init.ADCx->SQR1 = sqrcpy;
	}
}

static void PAL_ADC_SetRegSeqenceLength(PalADC_HandleTypeDef *Handle, uint8_t L)
{
	uint32_t sqr3cpy = Handle->Init.ADCx->SQR3;
	sqr3cpy &= ~(0x0f << 20);
	sqr3cpy |= (uint32_t)(L-1) << 20;
	Handle->Init.ADCx->SQR3 = sqr3cpy;
}

void PAL_ADC_ADCIRQHandler(PalADC_HandleTypeDef *Handle)
{
	if(Handle->Mode == PAL_ADC_MODE_SINGLE_TRI | Handle->Mode == PAL_ADC_MODE_SINGLE_CONT)
	{
		if(ADC_GetITStatus(Handle->Init.ADCx, ADC_IT_EOC) == SET)
		{
			Handle->SingleConvertCb((float)(ADC_GetConversionValue(Handle->Init.ADCx) & 0xffff) / 4096);
		}
	}
}
