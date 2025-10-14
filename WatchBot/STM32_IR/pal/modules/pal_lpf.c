#include "pal_lpf.h"
#include "stm32f10x_pal.h"

void PAL_Lpf_Init(PalLpf_HandleTypeDef *Handle)
{
	Handle->LastTimeStamp = PAL_INVALID_TICK;
	
	if(Handle->Init.dt != 0)
	{
		Handle->alpha = Handle->Init.Tf / (Handle->Init.Tf + Handle->Init.dt);
		Handle->_1_minus_alpha = 1-Handle->alpha;
	}
	
	Handle->FirstRun = 1;
}

float PAL_Lpf_Calc(PalLpf_HandleTypeDef *Handle, float Input)
{
	float output;
	
	if(Handle->alpha != 0)
	{
		if(Handle->FirstRun)
		{
			output = Input;
			Handle->FirstRun = 0;
		}
		else
		{
			output = Handle->alpha * Handle->LastOutput + Handle->_1_minus_alpha * Input;
		}
	}
	else
	{
		uint64_t now = PAL_GetUs();
		
		if(Handle->LastTimeStamp == PAL_INVALID_TICK)
		{
			output = Input;
		}
		else
		{
			float dt = (now - Handle->LastTimeStamp) * 1e-6;
			
			float alpha = Handle->Init.Tf / (Handle->Init.Tf + dt);

			output = alpha * Handle->LastOutput + (1-alpha) * Input;
		}
		
		Handle->LastTimeStamp = now;
	}
	
	Handle->LastOutput = output;
	
	return output;
}
