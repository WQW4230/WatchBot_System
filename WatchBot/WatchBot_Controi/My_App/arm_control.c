#include "arm_control.h"
#include "motor_driver.h"
#include "Scheduler.h"

#include "OLED.h" //测试用


static Arm_Angle_t Current_Angle; //当前角度
static Arm_Angle_t Target_Angle;  //目标角度

/*
	角度线性映射函数
	输入的角度映射为定时器对应的占空比
*/
uint16_t Angle_Map(float Angle, float max, float min)
{
	float range = max - min;
	//线性映射公式
	float PWM = (max - ((Angle + 90.0f) * range / 180.0f ));
	return (uint16_t)(PWM + 0.5f); //四舍五入
}

/*
		直接设置舵机角度 并且记录当前角度
		Base:  基座旋转角设置
		Roll:	 大臂翻滚角设置
		Ritch: 末端俯仰角设置
		输入值不能超过 @ANGLE_MAX @ANGLE_MIN 如若超过则为 @ANGLE_MAX @ANGLE_MIN
*/
void Servo_SetAngle(float Base, float Roll, float Pitch)
{
	//角度进行校验
	CLAMP(Base , ANGLE_MIN, ANGLE_MAX);
	CLAMP(Roll , ANGLE_MIN, ANGLE_MAX);
	CLAMP(Pitch, ANGLE_MIN, ANGLE_MAX);
	
	//放入线性映射函数得到PWM具体占空比
	uint16_t Base_Temp  = Angle_Map(Base , BASE_ANGLE_MAX,  BASE_ANGLE_MIN);
	uint16_t Roll_Temp  = Angle_Map(Roll , ROLL_ANGLE_MAX,  ROLL_ANGLE_MIN);
	uint16_t Pitch_Temp = Angle_Map(Pitch, PITCH_ANGLE_MAX, PITCH_ANGLE_MIN);
	
	//设置占空比
	PWM_SetCompare1(Base_Temp);
	PWM_SetCompare2(Roll_Temp);
	PWM_SetCompare3(Pitch_Temp);
	
	//记录当前角度
	Current_Angle.Base_Angle  = Base;
	Current_Angle.Roll_Angle = Roll;
	Current_Angle.Pitch_Angle  = Pitch;
}

/*
	调用该函数设置目标值，会平滑缓慢向目标值逼近
	每次调用Arm_Update会逼近一次
*/
void Arm_MoveTo(float Base, float Roll, float Pitch)
{
	//角度进行校验后放入目标角度值
	Target_Angle.Base_Angle  = CLAMP(Base , ANGLE_MIN, ANGLE_MAX);
	Target_Angle.Roll_Angle  = CLAMP(Roll , ANGLE_MIN, ANGLE_MAX);
	Target_Angle.Pitch_Angle = CLAMP(Pitch, ANGLE_MIN, ANGLE_MAX);
}

/*
调用Arm_Update会直接读取当前ADC值并写入舵机寄存器
	会平滑缓慢向目标值逼近
	目标值在这个结构体变量里:Target_Angle
	每次逼近值在				 :ANGLE_STEP
					PID系数				ANGLE_PID
*/
void Arm_Update(void)
{
	//基座旋转角-如果目标值大于当前值
//	if(Target_Angle.Base_Angle > Current_Angle.Base_Angle)
//	{
//		Current_Angle.Base_Angle += ANGLE_STEP;
//	}
//	else if(Target_Angle.Base_Angle < Current_Angle.Base_Angle)
//	{
//		Current_Angle.Base_Angle -= ANGLE_STEP;
//	}
//	
//	//大臂翻滚角-如果目标值大于当前值
//	if(Target_Angle.Roll_Angle > Current_Angle.Roll_Angle)
//	{
//		Current_Angle.Roll_Angle += ANGLE_STEP;
//	}
//	else if(Target_Angle.Roll_Angle < Current_Angle.Roll_Angle)
//	{
//		Current_Angle.Roll_Angle -= ANGLE_STEP;
//	}
//	
//	//腕部俯仰角-如果目标值大于当前值
//	if(Target_Angle.Pitch_Angle > Current_Angle.Pitch_Angle)
//	{
//		Current_Angle.Pitch_Angle += ANGLE_STEP;
//	}
//	else if(Target_Angle.Pitch_Angle < Current_Angle.Pitch_Angle)
//	{
//		Current_Angle.Pitch_Angle -= ANGLE_STEP;
//	}
	
	Current_Angle.Base_Angle = (Current_Angle.Base_Angle * (1 - ANGLE_BASE_PID)) + (Target_Angle.Base_Angle * ANGLE_BASE_PID);
	Current_Angle.Roll_Angle = (Current_Angle.Roll_Angle * (1 - ANGLE_PID)) + (Target_Angle.Roll_Angle * ANGLE_PID);
	Current_Angle.Pitch_Angle = (Current_Angle.Pitch_Angle * (1 - ANGLE_PID)) + (Target_Angle.Pitch_Angle * ANGLE_PID);
	
	OLED_ShowFloatNum(0, 0, Current_Angle.Base_Angle, 3, 5, OLED_6X8);
	OLED_ShowFloatNum(0, 8, Current_Angle.Roll_Angle, 3, 5, OLED_6X8);
	OLED_ShowFloatNum(0, 16, Current_Angle.Pitch_Angle, 3, 5, OLED_6X8);
	
	//调整当前角度
	Servo_SetAngle(Current_Angle.Base_Angle, Current_Angle.Roll_Angle, Current_Angle.Pitch_Angle);
}

//初始化函数
void Arm_Init(void)
{
	 motor_Init();
}

/*
	测试用demo
*/
void Arm_deom(void)
{
	Arm_Init();
	
	uint32_t Last;
	
	Servo_SetAngle(90, -90, -90);
	App_Timer_Delay_ms(1500);
	Arm_MoveTo(-90, 90, -90);
	Last =  App_Timer_GetTick();
	while(1)
	{
		OLED_Clear();
		Arm_Update();
		OLED_Update();
		if(App_Timer_GetTick() > Last + 3000)
		{
			Last =  App_Timer_GetTick();
			break;
		}
	}
	
	Arm_MoveTo(90, -90, 90);
	Last =  App_Timer_GetTick();
	while(1)
	{
		OLED_Clear();
		Arm_Update();
		OLED_Update();
		if(App_Timer_GetTick() > Last + 3000)
		{
			Last =  App_Timer_GetTick();
			break;
		}
	}
	Arm_MoveTo(90, 90, -90);
	Last =  App_Timer_GetTick();
	while(1)
	{
		OLED_Clear();
		Arm_Update();
		OLED_Update();
		if(App_Timer_GetTick() > Last + 3000)
		{
			Last =  App_Timer_GetTick();
			break;
		}
	}
	
	Arm_MoveTo(-90, 90, -90);
	Last =  App_Timer_GetTick();
	while(1)
	{
		OLED_Clear();
		Arm_Update();
		OLED_Update();
		if(App_Timer_GetTick() > Last + 3000)
		{
			Last =  App_Timer_GetTick();
			break;
		}
	}
	Arm_MoveTo(-90, -90, 90);
	Last =  App_Timer_GetTick();
	while(1)
	{
		OLED_Clear();
		Arm_Update();
		OLED_Update();
		if(App_Timer_GetTick() > Last + 3000)
		{
			Last =  App_Timer_GetTick();
			break;
		}
	}
	
	Arm_MoveTo(0, 0, 0);
	Last =  App_Timer_GetTick();
	while(1)
	{
		OLED_Clear();
		Arm_Update();
		OLED_Update();
		if(App_Timer_GetTick() > Last + 3000)
		{
			Last =  App_Timer_GetTick();
			break;
		}
	}
	
	Arm_MoveTo(0, 90, 90);
	Last =  App_Timer_GetTick();
	while(1)
	{
		OLED_Clear();
		Arm_Update();
		OLED_Update();
		if(App_Timer_GetTick() > Last + 3000)
		{
			Last =  App_Timer_GetTick();
			break;
		}
	}
}
