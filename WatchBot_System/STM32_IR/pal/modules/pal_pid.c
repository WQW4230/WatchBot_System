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

#include "stm32f10x_pal.h"
#include "pal_pid.h"

//
// @简介：PID算法初始化
// @参数：Handle - PID算法句柄
// @返回：空
//
void PAL_PID_Init(PalPID_HandleTypeDef *Handle)
{
	Handle->Setpoint = Handle->Init.Setpoint;
	Handle->Kp = Handle->Init.Kp;
	Handle->Ki = Handle->Init.Ki;
	Handle->Kd = Handle->Init.Kd;
	Handle->OutputLowerLimit = Handle->Init.OutputLowerLimit;
	Handle->OutputUpperLimit = Handle->Init.OutputUpperLimit;
	Handle->ITerm = Handle->Init.DefaultOutput;
	Handle->LastTime = PAL_INVALID_TICK;
	Handle->LastInput = 0;
	Handle->ManualOutput = Handle->Init.DefaultOutput;
	Handle->Manual = SET;
}

//
// @简介：复位PID算法
// @参数：Handle - PID算法句柄
// @返回：空
// @注意：其实现原理是将积分项清零
//
void PAL_PID_Reset(PalPID_HandleTypeDef *Handle)
{
	Handle->LastTime = PAL_INVALID_TICK;
	Handle->ITerm = Handle->Init.DefaultOutput;
}

//
// @简介：执行一次PID运算并得出控制器当前的输出值
// @参数：Handle - PID算法句柄
// @参数：Input  - 传感器的输入值
// @返回：控制器当前的输出值
//
float PAL_PID_Compute1(PalPID_HandleTypeDef *Handle, float Input)
{
	float Output;
	float timeChange;
	float error;
	
	uint64_t now = PAL_GetUs();
	
	error	= Handle->Setpoint - Input;
	
	Output = Handle->Kp * error;
	
	if(Handle->LastTime != PAL_INVALID_TICK && (Handle->Ki != 0 || Handle->Kd != 0)) // 第一次初始化，或无I和D环节
	{
		timeChange = (float)(now - Handle->LastTime) * 1.0e-6;
		
		if(Handle->Kd != 0)
		{
			Output += -Handle->Kd * (Input - Handle->LastInput) / timeChange;
		}
		
		if(Handle->Ki != 0)
		{
			Handle->ITerm += Handle->Ki * (error + Handle->LastError) * 0.5 * timeChange;
			
			if(Handle->ITerm > Handle->OutputUpperLimit)
			{
				Handle->ITerm = Handle->OutputUpperLimit;
			}
			else if(Handle->ITerm < Handle->OutputLowerLimit)
			{
				Handle->ITerm = Handle->OutputLowerLimit;
			}
			
			Output += Handle->ITerm;
		}
	}
	
	if(Output > Handle->OutputUpperLimit)
	{
		Output = Handle->OutputUpperLimit;
	}
	else if(Output < Handle->OutputLowerLimit)
	{
		Output = Handle->OutputLowerLimit;
	}
	
	Handle->LastInput = Input;
	Handle->LastTime = now;
	Handle->LastError = error;
	
	if(Handle->Manual == SET) 
	{
		Output = Handle->ManualOutput;
	}
	
	return Output;
}

//
// @简介：执行一次PID运算并得出控制器当前的输出值
//        此方法与PAL_PID_Compute1相同，但需要用户提供dInputDt的值
// @参数：Handle - PID算法句柄
// @参数：Input  - 传感器的输入值
// @参数：dInputDt - 传感器输入的变化速度 dInput(k) = (Input(k) - Input(k-1)) / dt
// @返回：控制器当前的输出值
//
float PAL_PID_Compute2(PalPID_HandleTypeDef *Handle, float Input, float dInputDt)
{
	float Output;
	float timeChange;
	float error;
	
	uint64_t timeStamp = PAL_GetUs();
	
	error	= Handle->Setpoint - Input;
	
	if(Handle->LastTime == PAL_INVALID_TICK) // 第一次初始化
	{
		Output = Handle->Kp * error + Handle->ITerm;
	}
	else
	{
		timeChange = (float)(timeStamp - Handle->LastTime) * 1.0e-6;		
		
		Handle->ITerm += Handle->Ki * (error + Handle->LastError) * 0.5 * timeChange;
		
		if(Handle->Ki != 0)
		{
			if(Handle->ITerm > Handle->OutputUpperLimit)
			{
				Handle->ITerm = Handle->OutputUpperLimit;
			}
			if(Handle->ITerm < Handle->OutputLowerLimit)
			{
				Handle->ITerm = Handle->OutputLowerLimit;
			}
		}
		
		Output = Handle->Kp * error + Handle->ITerm  - Handle->Kd * dInputDt;
		
		if(Output > Handle->OutputUpperLimit)
		{
			Output = Handle->OutputUpperLimit;
		}
		else if(Output < Handle->OutputLowerLimit)
		{
			Output = Handle->OutputLowerLimit;
		}
	}
	
	Handle->LastInput = Input;
	Handle->LastTime = timeStamp;
	Handle->LastError = error;
	
	if(Handle->Manual == SET) 
	{
		Output = Handle->ManualOutput;
	}
	
	return Output;
}

//
// @简介：调节PID的参数（Kp, Ki和Kd）
// @参数：Handle - PID算法句柄
// @参数：NewKp  - 新的比例系数Kp
// @参数：NewKi  - 新的积分系数Ki
// @参数：NewKd  - 新的微分系数Kd
// @返回：空
//
void PAL_PID_ChangeTunings(PalPID_HandleTypeDef *Handle, float NewKp, float NewKi, float NewKd)
{
	Handle->Kp = NewKp;
	Handle->Ki = NewKi;
	Handle->Kd = NewKd;
}

//
// @简介：改变设定值SP
// @参数：Handle      - PID算法句柄
// @参数：NewSetpoint - 新的设定值
// @返回：空
//
void PAL_PID_ChangeSetpoint(PalPID_HandleTypeDef *Handle, float NewSetpoint)
{
	Handle->Setpoint = NewSetpoint;
}

//
// @简介：获取设定值SP
// @参数：Handle      - PID算法句柄
// @返回：SP
//
float PAL_PID_GetSetpoint(PalPID_HandleTypeDef *Handle)
{
	return Handle->Setpoint;
}

//
// @简介：获取当前的PID参数
// @参数：Handle - PID算法句柄
// @参数：pKpOut - 输出参数，用于输出Kp的值
// @参数：pKiOut - 输出参数，用于输出Ki的值
// @参数：pKdOut - 输出参数，用于输出Kd的值
// @返回：空
//
void PAL_PID_GetTunings(PalPID_HandleTypeDef *Handle, float *pKpOut, float *pKiOut, float *pKdOut)
{
	*pKpOut = Handle->Kp;
	*pKiOut = Handle->Ki;
	*pKdOut = Handle->Kd;
}

//
// @简介：使能/禁止PID
// @参数：Handle   - PID算法句柄
// @参数：NewState - ENABLE - 使能，DISABLE - 禁止
// @返回：空
// @注意：PID被禁止后会进入手动模式。手动模式下的输出由PAL_PID_ChangeManualOutput设置
//
void PAL_PID_Cmd(PalPID_HandleTypeDef *Handle, uint8_t NewState)
{
	FlagStatus newManual = NewState ? RESET : SET;
	
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

//
// @简介：修改手模式下的输出
// @参数：Handle   - PID算法句柄
// @参数：NewValue - 手动模式下的输出
// @返回：空
//
void PAL_PID_ChangeManualOutput(PalPID_HandleTypeDef *Handle, float NewValue)
{
	Handle->ManualOutput = NewValue;
}
