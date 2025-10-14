#ifndef _PAL_FIRST_ORDER_LOWPASS_FILTER_H_
#define _PAL_FIRST_ORDER_LOWPASS_FILTER_H_

#include "stm32f10x.h"

typedef struct 
{
	float Cutoff;
	float SampleInterval;
}PalFirstOrderLowpassFilter_InitTypeDef;

typedef struct
{
	PalFirstOrderLowpassFilter_InitTypeDef Init;
	float q; // 时间常数
	uint32_t n; // 标志当前是第几个数据
	float lastOutput;
}PalFirstOrderLowpassFilter_HandleTypeDef;

void PAL_FirstOrderLowpassFiler_Init(PalFirstOrderLowpassFilter_HandleTypeDef *Handle);
float PAL_FirstOrderLowpassFiler_Calc(PalFirstOrderLowpassFilter_HandleTypeDef *Handle, float Input);

#endif
