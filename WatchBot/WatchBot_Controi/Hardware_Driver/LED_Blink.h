#ifndef _ACTIVE_BUZZER_H
#define _ACTIVE_BUZZER_H

#include "stm32f10x.h"                

//该模块为LED驱动模块
//支持多个LED

//高电平还是低电平点亮还是高电平
typedef enum
{
  LED_ACTIVE_LOW  = 0,
	LED_ACTIVE_HIGH = 1
}LedLogic_e;

//LED状态
typedef enum
{
	LED_OFF = 0,
	LED_ON  = 1
}LedState_e;

//GPIOX   :引脚总线位置
//GPIO_PIN:引脚位置
//Lofic   :高低电平触发

typedef struct
{
	GPIO_TypeDef *GPIOX;
	uint16_t GPIO_PIN;
	LedLogic_e Lofic;
}LED_t;

//LED初始化----需手动分配总线对应的引脚
void LED_Init(LED_t *LED);

//LED状态设置
void LED_SetState(LED_t *LED, LedState_e state);

//LED亮多久
void LED_Beep(LED_t *LED, uint64_t duration);

//进程函数放在主循环里
void LED_Proc(void);

#endif
