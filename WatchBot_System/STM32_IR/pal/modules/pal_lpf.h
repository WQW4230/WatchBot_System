#ifndef _PAL_LPF_H_
#define _PAL_LPF_H_

#include <stdint.h>

typedef struct 
{
	float Tf; // Low pass filter time constant
	float dt; // Delta t in seconds
}PalLpf_InitTypeDef;

typedef struct
{
	PalLpf_InitTypeDef Init;
	float LastOutput;
	uint64_t LastTimeStamp;
	float alpha;
	float _1_minus_alpha;
	uint8_t FirstRun;
}PalLpf_HandleTypeDef;

 void PAL_Lpf_Init(PalLpf_HandleTypeDef *Handle);
float PAL_Lpf_Calc(PalLpf_HandleTypeDef *Handle, float Input);

#endif
