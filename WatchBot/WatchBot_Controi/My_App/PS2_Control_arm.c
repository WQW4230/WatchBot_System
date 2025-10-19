#include "PS2_Control_arm.h"
#include "ps2_ADC_DMA.h"
#include <stdlib.h>
#include <math.h>
#include "arm_control.h"

#include "OLED.h"//测试


//句柄只能在当前函数调用
static JOY_State state; 

void JOY_Control_Init(void)
{
	//硬件底层初始化
	PS2_ADCInit();
	for(uint16_t i=0; i<JOY_DIR_COUNT; i++)
	{
		state.last_ADC[i]    = PS2_AD[i];
		state.current_ADC[i] = PS2_AD[i];
	}
	state.last_Angle.Base = 0;
	state.last_Angle.Roll = 0;
	state.last_Angle.Pitch = 0;
	state.last_Angle.FanSpeed = 0;
}

//读ADC并进行死区判断
//读到的值会存入现在的ADC
static void Read_ADC(JOY_State *state)
{
	//死区判断
	for(uint16_t i=0; i<JOY_DIR_COUNT; i++)
	{
		uint16_t ADC = PS2_AD[i];
		//更新当前ADC的值
		state->current_ADC[i] = ADC;
		
		int32_t Temp = (int32_t)state->last_ADC[i] - (int32_t)ADC;
		//死区判断通过，ADC值会变化
		if(abs(Temp) > JOY_DEAD)
		{
			state->last_ADC[i] = ADC;
		}
	}
}

//将ADC值映射到角度加死区判定
//参数 AD：AD通道的值
//参数 Cnetre：AD通道的中心值
//返回值: 需要变化的风扇转速
static float ADC_MapFan(uint16_t ADC, uint16_t Cnetre)
{
	if(ADC > Cnetre)
	{
		float FanSpeed = ((float)ADC - (int32_t)Cnetre) / (4095 - (float)Cnetre) * 100;
		//死区设置
		if(FanSpeed >= 97)
		{
			FanSpeed = 100;
		}
		return FanSpeed;
	}
	else
	{
		float FanSpeed = ((float)ADC - (float)Cnetre) / (Cnetre - 0) * 100;
		//死区设置
		if(FanSpeed <= -97)
		{
			FanSpeed = -100;
		}
		return FanSpeed;
	}
}

//将ADC值映射到角度加死区判定
//参数 AD：AD通道的值
//参数 Cnetre：AD通道的中心值
//返回值: 需要变化的角度
static float ADC_MapAngle(uint16_t ADC, uint16_t Cnetre)
{
	if(ADC > Cnetre)
	{
		float Angle = ((float)ADC - (int32_t)Cnetre) / (4095 - (float)Cnetre) * 90;
		//死区设置
		if(Angle >= 87)
		{
			Angle = 90;
		}
		return Angle;
	}
	else
	{
		float Angle = ((float)ADC - (float)Cnetre) / (Cnetre - 0) * 90;
		//死区设置
		if(Angle <= -87)
		{
			Angle = -90;
		}
		return Angle;
	}
}

//ADC映射后的角度死区过滤，当大于1°才对舵机进行变化
static void Angle_Dead(JOY_State *state)
{
	
	JOY_Angle *current = &(state->current_Angle);
	JOY_Angle *Last = &(state->last_Angle);
	
	Read_ADC(state);
	//要改摇杆对应舵机和电机在这里
	current->Base	    	= JOY2_Y_DIR * ADC_MapAngle(state->current_ADC[JOY_BASE], JOY2_Y_CENTRE);
	current->Roll 		  = JOY1_Y_DIR * ADC_MapAngle(state->current_ADC[JOY_ROLL], JOY1_Y_CENTRE);
	current->Pitch		  = JOY2_X_DIR * ADC_MapAngle(state->current_ADC[JOY_PITCH], JOY2_X_CENTRE);
	current->FanSpeed 	= JOY1_X_DIR * ADC_MapFan(state->current_ADC[JOY_FAN], JOY1_X_CENTRE);
	
	if(fabs(current->Base - Last->Base) > ANGLE_DEAD)
	{
		Last->Base = current->Base;
	}
	
	if(fabs(current->Roll - Last->Roll) > ANGLE_DEAD)
	{
		Last->Roll = current->Roll;
	}
	
	if(fabs(current->Pitch - Last->Pitch) > ANGLE_DEAD)
	{
		Last->Pitch = current->Pitch;
	}
	
	if(fabs(current->FanSpeed - Last->FanSpeed) > ANGLE_DEAD)
	{
		Last->FanSpeed = current->FanSpeed;
	}
	
}

//放在任务里按时间调度即可
void PS2_Uptada(void)
{
	Angle_Dead(&state);
	
//	打印测试
	OLED_ShowFloatNum(0, 0, state.last_Angle.Base, 3, 4, OLED_8X16);
	OLED_ShowFloatNum(0, 16, state.last_Angle.Roll, 3, 4, OLED_8X16);
	OLED_ShowFloatNum(0, 32, state.last_Angle.Pitch, 3, 4, OLED_8X16);
	OLED_ShowFloatNum(0, 48, state.last_Angle.FanSpeed, 3, 4, OLED_8X16);
	
	Arm_MoveTo(state.last_Angle.Base, state.last_Angle.Roll, state.last_Angle.Pitch);
}
