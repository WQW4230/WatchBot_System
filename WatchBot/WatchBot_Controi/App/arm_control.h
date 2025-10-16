#ifndef _SEROV_DRIVER_H
#define	_SEROV_DRIVER_H

//TB6612  电机驱动
//MG90舵机 驱动程序
#include "stm32f10x.h"

typedef struct
{
	float Base_Angle;   //基座平移角
	float Roll_Angle; 	//大臂翻滚角
	float Pitch_Angle;	//末端俯仰角
}Arm_Angle_t;

//初始化函数
void Arm_Init(void);

//直接设置机械臂角度，不平滑移动
void Servo_SetAngle(float Base, float Roll, float Pitch);

//平滑移动
void Arm_MoveTo(float Base, float Roll, float Pitch);

//更新函数
void Arm_Update(void);

#endif
