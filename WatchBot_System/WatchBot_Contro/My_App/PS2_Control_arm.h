#ifndef _PS2_CONTROL_ARM_H_
#define	_PS2_CONTROL_ARM_H_

#include "stm32f10x.h"

#define JOY_DIR_COUNT 4 //摇杆轴的数量 //双摇杆就4个轴

/*
两个摇杆分别控制哪个模块
JOY1_X == 0
JOY1_Y == 1
JOY2_X == 2
JOY2_Y == 3
*/
#define JOY_FAN   0   //直流电机   JOY1_X == 0
#define JOY_ROLL   1   //大臂		   JOY1_Y == 1
#define JOY_PITCH  2	 //小臂			 JOY2_X == 2
#define JOY_BASE    3   //基座		 JOY2_Y == 3

//摇杆方向是否翻转
#define JOY1_X_DIR 1
#define JOY1_Y_DIR 1
#define JOY2_X_DIR 1
#define JOY2_Y_DIR -1

//摇杆中心值
#define JOY1_X_CENTRE 2048
#define JOY1_Y_CENTRE 1980
#define JOY2_X_CENTRE 2048
#define JOY2_Y_CENTRE 2048

//摇杆死区 可选0-4095  推荐30-60 
#define JOY_DEAD 30
//角度死区  //单位度可选0-90 推荐1-3
#define ANGLE_DEAD 2

typedef struct
{
	float Base;     	//摇杆映射到基座旋转角的值
  float Roll; 			//摇杆映射到大臂翻滚角的值
  float Pitch;			//摇杆映射到末端俯仰角的值
	float FanSpeed;   //摇杆映射到直流电机转速的值
}JOY_Angle;  //摇杆映射到的角读

typedef struct 
{
	uint16_t last_ADC[JOY_DIR_COUNT];   	//上次ADC的值
	uint16_t current_ADC[JOY_DIR_COUNT];	//当前ADC的值	
	
  JOY_Angle last_Angle;			//上次线性映射的角度
	JOY_Angle current_Angle;  //当前线性映射的角度
}JOY_State;

//摇杆初始化
void JOY_Control_Init(void);

//将摇杆过滤后的值写入伺服舵机系统
void PS2_Uptada(void);

//只读当前摇杆角度
const JOY_Angle *Joy_GetCurrent_Angle(void);

#endif
