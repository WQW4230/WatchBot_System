#include "arm_control.h"
#include "motor_driver.h"
#include <math.h>

static Arm_Angle_t Current_Angle; //当前角度
static Arm_Angle_t Target_Angle;  //目标角度

/*
	角度线性映射函数
	输入的角度映射为定时器对应的占空比
	参数max：最大角度对应的占空比
	参数min：最小角度对应的占空比
*/
static uint16_t Angle_Map(float Angle, float max, float min)
{
	float range = max - min;
	//线性映射公式
	float PWM = (max - ((Angle + 90.0f) * range / 180.0f ));
	return (uint16_t)(PWM + 0.5f); //四舍五入
}

/*
	转速线性映射函数
	如果为负数在此更改H桥方向
	输入的转速映射为定时器对应的占空比
	参数max：最大速度对应的占空比
	参数min：最小速度对应的占空比
*/
static uint16_t FanSpeed_Map(float Speed, float max, float min)
{
	float abs_speed = fabs(Speed);

	//线性映射公式
	float PWM = min + (max - min) * abs_speed / 100.0f;
	return (uint16_t)(PWM + 0.5f); //四舍五入
}

/*
	电机状态设置
	输入值为正数 正转
	输入值为负数 反转
	输入值为 0	 刹车
	死区 5
*/
static void Moto_correction()
{
	float Speed = Current_Angle.Fan_Speed;
	if(Speed > 5)
		Moto_SetState(MOTOR_FORWARD);//正转
	else if(Speed < -5)
		Moto_SetState(MOTOR_BACKWARD);//反转
	else
		Moto_SetState(MOTOR_STOP); //刹车
}

/*
		直接设置舵机角度 并且记录当前角度
		Pan:  基座旋转角设置
		Roll:	 大臂翻滚角设置
		Tilt: 末端俯仰角设置
		输入值不能超过 @ANGLE_MAX @ANGLE_MIN 如若超过则为 @ANGLE_MAX @ANGLE_MIN
*/
void Servo_SetAngle(float Pan, float Roll, float Tilt, float speed)
{
	//角度进行校验
	CLAMP(Pan  , ANGLE_MIN, ANGLE_MAX);
	CLAMP(Roll , ANGLE_MIN, ANGLE_MAX);
	CLAMP(Tilt , ANGLE_MIN, ANGLE_MAX);
	CLAMP_FAN(speed, SPEED_MIN, SPEED_MAX);
	
	//放入线性映射函数得到PWM具体占空比
	uint16_t Pan_Temp   = Angle_Map(Pan , PAN_ANGLE_MAX , PAN_ANGLE_MIN );
	uint16_t Roll_Temp  = Angle_Map(Roll, ROLL_ANGLE_MAX, ROLL_ANGLE_MIN);
	uint16_t Tilt_Temp  = Angle_Map(Tilt, TILT_ANGLE_MAX, TILT_ANGLE_MIN);
	uint16_t Speed_Temp = FanSpeed_Map(speed, FAN_SPEED_MAX, FAN_SPEED_MIN);
	
	//设置占空比
	PWM_SetCompare1(Pan_Temp);
	PWM_SetCompare2(Roll_Temp);
	PWM_SetCompare3(Tilt_Temp);
	PWM_SetCompare4(Speed_Temp);
	
	//记录当前角度 /速度
	Current_Angle.Pan_Angle  = Pan;
	Current_Angle.Roll_Angle = Roll;
	Current_Angle.Tilt_Angle = Tilt;
	Current_Angle.Fan_Speed  = speed;
}

/*
	调用该函数设置目标值，会平滑缓慢向目标值逼近
	每次调用Arm_Update会逼近一次
*/
void Arm_MoveTo(float Pan, float Roll, float Tilt, float Speed)
{
	//角度进行校验后放入目标角度值
	Target_Angle.Pan_Angle  = CLAMP(Pan , ANGLE_MIN, ANGLE_MAX);
	Target_Angle.Roll_Angle = CLAMP(Roll, ANGLE_MIN, ANGLE_MAX);
	Target_Angle.Tilt_Angle = CLAMP(Tilt, ANGLE_MIN, ANGLE_MAX);
	Target_Angle.Fan_Speed  = CLAMP(Speed, SPEED_MIN, SPEED_MAX);
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
	Current_Angle.Pan_Angle  = (Current_Angle.Pan_Angle  * (1 - ANGLE_PAN_PID )) + (Target_Angle.Pan_Angle  * ANGLE_PAN_PID );
	Current_Angle.Roll_Angle = (Current_Angle.Roll_Angle * (1 - ANGLE_ROLL_PID)) + (Target_Angle.Roll_Angle * ANGLE_ROLL_PID);
	Current_Angle.Tilt_Angle = (Current_Angle.Tilt_Angle * (1 - ANGLE_TILT_PID)) + (Target_Angle.Tilt_Angle * ANGLE_TILT_PID);
	Current_Angle.Fan_Speed  = (Current_Angle.Fan_Speed  * (1 - FAN_SPEED_PID )) + (Target_Angle.Fan_Speed  * FAN_SPEED_PID );
	
	//设置直流电机状态
	Moto_correction();
	
	//调整当前角度
	Servo_SetAngle(Current_Angle.Pan_Angle, Current_Angle.Roll_Angle, Current_Angle.Tilt_Angle, Current_Angle.Fan_Speed);
}

//只读 获得舵机臂当前角度句柄
const Arm_Angle_t *Arm_GetCurrent_Angle(void)
{
	return &Current_Angle;
}

//只读 获得舵机臂目标角度句柄
const Arm_Angle_t *Arm_GetTarget_Angle(void)
{
	return &Target_Angle;
}

//初始化函数
void Arm_Init(void)
{
	 motor_Init();
}

