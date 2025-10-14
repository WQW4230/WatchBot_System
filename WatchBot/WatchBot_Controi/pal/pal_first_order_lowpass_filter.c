#include "pal_first_order_lowpass_filter.h"

void PAL_FirstOrderLowpassFiler_Init(PalFirstOrderLowpassFilter_HandleTypeDef *Handle)
{
	Handle->n = 0;
	Handle->q = Handle->Init.Cutoff * 2 * 3.1415927 * Handle->Init.SampleInterval;
}

float PAL_FirstOrderLowpassFiler_Calc(PalFirstOrderLowpassFilter_HandleTypeDef *Handle, float Input)
{
	float result;
	if(Handle->n == 0) // 输入的第一个数据
	{
		result = Input;
		Handle->n++;
	}
	else
	{
		result = Handle->q * Input + (1-Handle->q)*Handle->lastOutput;
	}
	Handle->lastOutput = result;
	return result;
}
