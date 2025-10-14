#ifndef _REMOTE_NEC_DRIVER_H
#define _REMOTE_NEC_DRIVER_H

#include "stm32f10x.h" 

typedef enum
{
	Key_Zero = 0,
	Key_One = 1,
	Key_Two,
	Key_three,
	Key_Four, 
	Key_Five, 
	Key_Six,
	Key_Seven,
	Key_Eight,
	Key_Nine,
	Key_ON,//无效按键
	Key_XingHao = 11,
	Key_JingHao = 12,
	Key_W = 13,
	Key_S = 14,
	Key_A = 15,
	Key_D = 16,
	Key_OK = 17,	
}Key_Enum;

void Capture_Config(void);//初始化红外遥控所有通道及其中断初始化
Key_Enum IR_GetKey(void);//获取当前按钮值 返回按键映射表的枚举常量
extern uint8_t IR_Flag; // 按键标志位

#endif
