#ifndef _ARM_CONTROL_H_
#define	_ARM_CONTROL_H_

#include "stm32f10x.h"

//TB6612  H桥驱动
//MG90/MG996R 舵机 驱动程序

/*
该宏定义定义舵机旋转最大角度的占空比值
MAX对应的是逆时针 90° 
MIN对应的是顺时针-90°
*/
#define PAN_ANGLE_MAX   2500
#define	PAN_ANGLE_MIN   600
#define	ROLL_ANGLE_MAX  530
#define	ROLL_ANGLE_MIN  2450
#define	TILT_ANGLE_MAX  400
#define	TILT_ANGLE_MIN  2450
#define FAN_SPEED_MAX   1000
#define FAN_SPEED_MIN   300

//舵机角度每次变换，越小越平滑
#define ANGLE_STEP 1.2
//目标的PID系数 目标值系数比
#define ANGLE_PAN_PID   0.02
#define ANGLE_ROLL_PID  0.03
#define ANGLE_TILT_PID  0.03
#define FAN_SPEED_PID   0.2

//对输入的角度进行校验，不能超过限位
#define ANGLE_MAX  90
#define ANGLE_MIN -90

//对输入的转速进行校验，不能超过限位
#define SPEED_MAX  100
#define SPEED_MIN -100

//限位宏定义 输入任意角度如诺大于阈值则为阈值
#define CLAMP(x, min, max)  ((x) < (min) ? (min) : ((x) > (max) ? (max) : (x)))

//限位宏定义 输入任意速度如诺大于阈值则为阈值
#define CLAMP_FAN(x, min, max)  ((x) < (min) ? (min) : ((x) > (max) ? (max) : (x)))


typedef struct
{
	float Pan_Angle;    //基座旋转角
	float Roll_Angle; 	//大臂翻滚角
	float Tilt_Angle;	//末端俯仰角
	float Fan_Speed;    //风扇转速
}Arm_Angle_t;


void Arm_Init(void);
void Servo_SetAngle(float Pan, float Roll, float Tilt, float speed);
void Arm_MoveTo(float Pan, float Roll, float Tilt, float Speed);
void Arm_Update(void);
void Arm_deom(void);

//读当前机械臂角度参数
const Arm_Angle_t *Arm_GetCurrent_Angle(void);
const Arm_Angle_t *Arm_GetTarget_Angle(void);

#endif
