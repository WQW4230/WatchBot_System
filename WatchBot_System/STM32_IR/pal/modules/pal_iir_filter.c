#include <stdlib.h>
#include <string.h>
#include "pal_iir_filter.h"

void PAL_IIRFilter_Init(PalIIRFilter_HandleTypeDef *Handle, uint16_t Order, const float *a, const float *b)
{
	Handle->Order = Order;
	Handle->a = (float *)malloc(sizeof(float) * (Handle->Order + 1));
	Handle->b = (float *)malloc(sizeof(float) * (Handle->Order + 1));
	memcpy(Handle->a, a, (Handle->Order + 1) * sizeof(float));
	memcpy(Handle->b, b, (Handle->Order + 1) * sizeof(float));
	Handle->x = (float *)malloc(sizeof(float) * (Handle->Order + 1));
	Handle->y = (float *)malloc(sizeof(float) * (Handle->Order + 1));
	Handle->n = 0;
	Handle->Cursor = 0;
}

float PAL_IIRFilter_Calc(PalIIRFilter_HandleTypeDef *Handle, float Input)
{
	float result;
	uint32_t i;
	
	Handle->x[Handle->Cursor] = Input;
	
	if(Handle->n < Handle->Order+1)
	{
		Handle->n++;
		result = Input;
	}
	else
	{
		result = Handle->b[0] * Handle->x[Handle->Cursor];
		for(i=1;i<Handle->Order+1;i++)
		{
			result += Handle->b[i] * Handle->x[(Handle->Cursor + Handle->Order + 1 - i) % (Handle->Order + 1)];
			result -= Handle->a[i] * Handle->y[(Handle->Cursor + Handle->Order + 1 - i) % (Handle->Order + 1)];
		}
	}
	
	Handle->y[Handle->Cursor] = result;
	Handle->Cursor = (Handle->Cursor + 1) % (Handle->Order + 1);
	return result;
}
