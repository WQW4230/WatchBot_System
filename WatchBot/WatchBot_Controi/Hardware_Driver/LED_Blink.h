#ifndef _ACTIVE_BUZZER_H
#define _ACTIVE_BUZZER_H

#include "stm32f10x.h"                

//该模块为LED驱动模块
//该模块强烈依赖系统时钟
//支持多个LED

//系统基准滴答定时器!!该模块强烈依赖系统时钟
//源文件内为 uint64_t system_tick

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
	LED_ON  = 1,
	LED_BLINK,
}LedMode_e;

//GPIOX   :引脚总线位置
//GPIO_PIN:引脚位置
//Lofic   :高低电平触发

typedef struct
{
	GPIO_TypeDef *GPIOX; //总线位置
	uint16_t GPIO_PIN;	 //引脚位置
	LedLogic_e Lofic;		 //高低电平点亮
	uint32_t Brigh_time; //亮的时间
	uint32_t Dark_time;  //暗的时间
	uint64_t Last_time;  //上次切换的时间
	LedMode_e LED_Mode;   //LED当前模式 
	uint8_t LED_State;   //LED当前亮灭状态
}LED_t;

//LED初始化----需手动分配总线对应的引脚
void LED_Init(LED_t *LED);

//LED状态设置
void LED_SetState(LED_t *LED, uint8_t state);

//LED设置模式
void LED_SetMode(LED_t *LED, LedMode_e Mode, uint32_t Brigh_time, uint32_t Dark_time);

//进程函数放在主循环里
void LED_Proc(void);

#endif
