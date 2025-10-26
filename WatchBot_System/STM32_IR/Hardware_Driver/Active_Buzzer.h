#ifndef _ACTIVE_BUZZER_H
#define _ACTIVE_BUZZER_H

#include "stm32f10x.h"                

//该模块为有源蜂鸣器驱动模块
//支持多个蜂鸣器

//高电平还是低电平蜂鸣
typedef enum
{
  BUZZER_ACTIVE_LOW  = 0,
	BUZZER_ACTIVE_HIGH = 1
}BuzzerLogic_e;

//蜂鸣器状态
typedef enum
{
	Buzzer_OFF = 0,
	Buzzer_ON  = 1
}BuzzerState_e;

//GPIOX   :引脚总线位置
//GPIO_PIN:引脚位置
//Lofic   :高低电平触发

typedef struct
{
	GPIO_TypeDef *GPIOX;
	uint16_t GPIO_PIN;
	BuzzerLogic_e Lofic;
}Buzzer_t;

//蜂鸣器初始化----需手动分配总线对应的引脚
void Buzzer_Init(Buzzer_t *buzzer);

//蜂鸣器状态设置
void Buzzer_SetState(Buzzer_t *buzzer, BuzzerState_e state);

//蜂鸣器响多久
void Buzzer_Beep(Buzzer_t *buzzer, uint64_t duration);

//进程函数放在主循环里
void Buzzer_Proc(void);

#endif
