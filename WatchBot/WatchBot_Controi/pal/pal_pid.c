/**
  ******************************************************************************
  * @file    stm32f10x_pid.c
  * @author  铁头山羊stm32工作组
  * @version V1.0.0
  * @date    2023年2月24日
  * @brief   pid算法库
  ******************************************************************************
  * @attention
  * Copyright (c) 2022 - 2023 东莞市明玉科技有限公司. 保留所有权力.
  ******************************************************************************
*/

#include "pal_pid.h"

#define INVALID_TIME_STAMP 0xffffffffffffffff

void PAL_PID_Init(PalPID_HandleTypeDef *Handle)
{
	Handle->Setpoint = Handle->Init.Setpoint;
	Handle->Kp = Handle->Init.Kp;
	Handle->Ki = Handle->Init.Ki;
	Handle->Kd = Handle->Init.Kd;
	Handle->OutputLowerLimit = Handle->Init.OutputLowerLimit;
	Handle->OutputUpperLimit = Handle->Init.OutputUpperLimit;
	Handle->ITerm = Handle->Init.DefaultOutput;
	Handle->LastTime = INVALID_TIME_STAMP;
	Handle->ManualOutput = Handle->Init.DefaultOutput;
	Handle->Manual = SET; // 初始状态下关闭PID
}

void PAL_PID_Reset(PalPID_HandleTypeDef *Handle)
{
	Handle->LastTime = INVALID_TIME_STAMP;
	Handle->ITerm = Handle->Init.DefaultOutput;
}

float PAL_PID_Compute1(PalPID_HandleTypeDef *Handle, float Input, float dInput, uint32_t TimeStamp)
{
	float Output;
	float timeChange;
	float error;
	
	if(Handle->Manual == SET) 
	{
		Output = Handle->ManualOutput;
	}
	else
	{
		error	= Handle->Setpoint - Input;
		if(Handle->LastTime == INVALID_TIME_STAMP) // 第一次初始化
		{
			Output = Handle->Kp * error + Handle->ITerm;
		}
		else
		{
			timeChange = (float)(TimeStamp - Handle->LastTime) / 1000.0;
			Handle->ITerm += Handle->Ki * error * timeChange;
			Output = Handle->Kp * error + Handle->ITerm  - Handle->Kd * dInput;
			if(Handle->Ki != 0)
			{
				if(Output > Handle->OutputUpperLimit)
				{
					Handle->ITerm -= Output - Handle->OutputUpperLimit;
					Output = Handle->OutputUpperLimit;
				}
				else if(Output < Handle->OutputLowerLimit)
				{
					Handle->ITerm += Handle->OutputLowerLimit - Output;
					Output = Handle->OutputLowerLimit;
				}
			}
		}
	}
	
	Handle->LastTime = TimeStamp;
	return Output;
}

float PAL_PID_Compute(PalPID_HandleTypeDef *Handle, float Input, uint64_t TimeStamp)
{
	float Output;
	float dInput;
	float timeChange;
	float error;
	
	if(Handle->Manual == SET) 
	{
		Output = Handle->ManualOutput;
	}
	else
	{
		error	= Handle->Setpoint - Input;
		if(Handle->LastTime == INVALID_TIME_STAMP) // 第一次初始化
		{
			Output = Handle->Kp * error + Handle->ITerm;
		}
		else
		{
			timeChange = (float)(TimeStamp - Handle->LastTime) / 1000.0;
			Handle->ITerm += Handle->Ki * error * timeChange;
			dInput = (Input - Handle->LastInput) / timeChange;
			Output = Handle->Kp * error + Handle->ITerm  - Handle->Kd * dInput;
			if(Handle->Ki != 0)
			{
				if(Output > Handle->OutputUpperLimit)
				{
					Handle->ITerm -= Output - Handle->OutputUpperLimit;
					Output = Handle->OutputUpperLimit;
				}
				else if(Output < Handle->OutputLowerLimit)
				{
					Handle->ITerm += Handle->OutputLowerLimit - Output;
					Output = Handle->OutputLowerLimit;
				}
			}
			else
			{
				if(Output > Handle->OutputUpperLimit)
				{
					Output = Handle->OutputUpperLimit;
				}
				else if(Output < Handle->OutputLowerLimit)
				{
					Output = Handle->OutputLowerLimit;
				}
			}
		}
	}
	
	Handle->LastInput = Input;
	Handle->LastTime = TimeStamp;
	return Output;
}

void PAL_PID_ChangeTunings(PalPID_HandleTypeDef *Handle, float NewKp, float NewKi, float NewKd)
{
	Handle->Kp = NewKp;
	Handle->Ki = NewKi;
	Handle->Kd = NewKd;
}

void PAL_PID_ChangeSetpoint(PalPID_HandleTypeDef *Handle, float NewSetpoint)
{
	Handle->Setpoint = NewSetpoint;
}

float PAL_PID_GetKp(PalPID_HandleTypeDef *Handle)
{
	return Handle->Kp;
}

float PAL_PID_GetKi(PalPID_HandleTypeDef *Handle)
{
	return Handle->Ki;
}

float PAL_PID_GetKd(PalPID_HandleTypeDef *Handle)
{
	return Handle->Kd;
}

void PAL_PID_Cmd(PalPID_HandleTypeDef *Handle, FunctionalState NewState)
{
	FlagStatus newManual = NewState == ENABLE ? RESET : SET;
	
	if(Handle->Manual == SET && newManual == RESET)
	{
		Handle->ITerm = Handle->ManualOutput;
		if(Handle->ITerm > Handle->OutputUpperLimit)
		{
			Handle->ITerm = Handle->OutputUpperLimit;
		}
		
		if(Handle->ITerm < Handle->OutputLowerLimit)
		{
			Handle->ITerm = Handle->OutputLowerLimit;
		}
	}
	
	Handle->Manual = newManual;
}

void PAL_PID_ChangeManualOutput(PalPID_HandleTypeDef *Handle, float NewValue)
{
	Handle->ManualOutput = NewValue;
}
