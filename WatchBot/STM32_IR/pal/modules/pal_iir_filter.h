#ifndef _PAL_IIR_FILTER_H_
#define _PAL_IIR_FILTER_H_

#include "stm32f10x.h"

typedef struct
{
	uint16_t Order; // 滤波器阶数
	float *a; // 分母多项式系数
	float *b; // 分子多项式系数
	float *x; 
	float *y;
	uint32_t n;
	uint32_t Cursor;
}PalIIRFilter_HandleTypeDef;

void PAL_IIRFilter_Init(PalIIRFilter_HandleTypeDef *Handle, uint16_t Order, const float *a, const float *b);
float PAL_IIRFilter_Calc(PalIIRFilter_HandleTypeDef *Handle, float Input);

#endif
