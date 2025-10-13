#ifndef _SEROV_DRIVER_H
#define	_SEROV_DRIVER_H

#include "stm32f10x.h"

//PWM初始化
void Arm_Init(void);

//控制CCR寄存器值 占空比设置
void PWM_SetCompare1(uint16_t Compare);
void PWM_SetCompare2(uint16_t Compare);
void PWM_SetCompare3(uint16_t Compare);

void PWM_SetCompare4(uint16_t Compare); //直流电机

#endif
