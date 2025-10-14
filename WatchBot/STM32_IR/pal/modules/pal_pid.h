/**
  ******************************************************************************
  * @file    stm32f10x_pid.h
  * @author  铁头山羊stm32工作组
  * @version V1.0.0
  * @date    2023年2月24日
  * @brief   pid算法库
  ******************************************************************************
  * @attention
  * Copyright (c) 2022 - 2023 东莞市明玉科技有限公司. 保留所有权力.
  ******************************************************************************
*/

#ifndef _PAL_PID_H_
#define _PAL_PID_H_

#include <stdint.h>
#include "pal_lpf.h"

typedef struct{
	float Kp; 
	float Ki; 
	float Kd; 
	float Setpoint;
	float OutputUpperLimit;
	float OutputLowerLimit;
	float DefaultOutput;
}PalPID_InitTypeDef;

typedef struct{
	PalPID_InitTypeDef Init;
	uint64_t LastTime; // Last time run (us)
	float ITerm;
	float LastInput;
	float LastError;
	float Kp;
	float Ki; 
	float Kd; 
	float OutputUpperLimit;
	float OutputLowerLimit;
	float Setpoint;
	uint32_t Manual;
	float ManualOutput;
}PalPID_HandleTypeDef;

 void PAL_PID_Init(PalPID_HandleTypeDef *Handle);
 void PAL_PID_Cmd(PalPID_HandleTypeDef *Handle, uint8_t NewState);
 void PAL_PID_Reset(PalPID_HandleTypeDef *Handle);
float PAL_PID_Compute1(PalPID_HandleTypeDef *Handle, float Input);
float PAL_PID_Compute2(PalPID_HandleTypeDef *Handle, float Input, float dInput);
 void PAL_PID_ChangeTunings(PalPID_HandleTypeDef *Handle, float NewKp, float NewKi, float NewKd);
 void PAL_PID_GetTunings(PalPID_HandleTypeDef *Handle, float *pKpOut, float *pKiOut, float *pKdOut);
 void PAL_PID_ChangeSetpoint(PalPID_HandleTypeDef *Handle, float NewSetpoint);
float PAL_PID_GetSetpoint(PalPID_HandleTypeDef *Handle);
 void PAL_PID_ChangeManualOutput(PalPID_HandleTypeDef *Handle, float NewValue);

#endif
