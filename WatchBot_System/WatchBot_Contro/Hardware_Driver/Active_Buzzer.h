#ifndef _ACTIVE_BUZZER_H
#define _ACTIVE_BUZZER_H

#include "stm32f10x.h"                

//该模块为有源蜂鸣器驱动模块
//支持多个蜂鸣器
//该模块强烈依赖系统时钟

//系统基准滴答定时器!!该模块强烈依赖系统时钟
//源文件内为 uint64_t system_tick

//高电平还是低电平蜂鸣
typedef enum
{
  BUZZER_ACTIVE_LOW  = 0,
	BUZZER_ACTIVE_HIGH = 1
}BuzzerLogic_e;

//蜂鸣器状态
typedef enum
{
	Buzzer_OFF  = 0,
	Buzzer_ON   = 1,
	Buzzer_Beep 
}BuzzerMode_e;

//GPIOX   :引脚总线位置
//GPIO_PIN:引脚位置
//Lofic   :高低电平触发

typedef struct
{
	GPIO_TypeDef *GPIOX;
	uint16_t GPIO_PIN;
	BuzzerLogic_e Lofic;
	uint32_t OnTime; 			//亮的时间
	uint32_t OffTime; 			 //暗的时间
	uint64_t Last_time;  			//上次切换的时间
	BuzzerMode_e Buzzer_Mode;   //蜂鸣器当前模式 
	uint8_t Buzzer_State;   			//蜂鸣器当前状态
}Buzzer_t;

//蜂鸣器初始化----需手动分配总线对应的引脚
void Buzzer_Init(Buzzer_t *buzzer);

//蜂鸣器设置状态
void Buzzer_SetState(Buzzer_t *buzzer, uint8_t state);

//蜂鸣器设置模式
void Buzzer_SetMode(Buzzer_t *buzzer, BuzzerMode_e Mode, uint32_t OnTime, uint32_t OffTime);

//进程函数放在主循环里
void Buzzer_Proc(void);

#endif
