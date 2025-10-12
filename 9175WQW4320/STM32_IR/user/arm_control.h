#ifndef _ARM_CONTROL_H
#define	_ARM_CONTROL_H

#include "stm32f10x.h"  


#define ARM_STEP 1 //按键按下时角度变化值

#define FAN_MAX 100
#define FAN_MIN -100

typedef struct 
{
    float base;
    float shoulder;
    float elbow;
    int16_t FanSpeed;
}ServoAngles_t;

//初始化
void Arm_Init(void);

//进程函数
void Arm_Proc(void);

//机械臂复位
void Arm_Reset(void);

extern uint8_t Arm_Flag;
#endif
