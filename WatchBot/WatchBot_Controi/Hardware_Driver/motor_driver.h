#ifndef _SEROV_DRIVER_H
#define	_SEROV_DRIVER_H

//TB6612  电机驱动
//MG90- MG996R舵机 驱动程序
#include "stm32f10x.h"

typedef enum {
    MOTOR_STOP,       // 空挡/停止
    MOTOR_FORWARD,    // 正转
    MOTOR_BACKWARD,   // 反转
    MOTOR_BRAKE       // 刹车
} Motor_State_t;

//PWM初始化
void motor_Init(void);

//控制CCR寄存器值 占空比设置 比理论控制的略宽
//三路MG90舵机 取值 400-2600 角度0°-180°

// 600顺时针90°  中心1550   2500逆时针90°
void PWM_SetCompare1(uint16_t Compare);


void PWM_SetCompare2(uint16_t Compare);
void PWM_SetCompare3(uint16_t Compare);

//控制直流电机 取值0-1000 转速0%-100%
void PWM_SetCompare4(uint16_t Compare); //直流电机

//电机状态设置
//state : 取值在@state 里
void Moto_SetState(Motor_State_t state);

#endif
