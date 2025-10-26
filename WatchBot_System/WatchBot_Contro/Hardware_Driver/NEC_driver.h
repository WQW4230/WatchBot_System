#ifndef _NEC_DRIVER_H
#define _NEC_DRIVER_H

#include "stm32f10x.h" 

#define ADDRES 0x00 //遥控器用户码

//extern IR_NEC_HandleTypedef IR_NEC;

typedef enum
{
    NEC_STATE_IDLE = 0,      // 空闲状态（未接收到起始信号）
    NEC_STATE_RECEIVING,     // 正在接收数据位
    NEC_STATE_REPEAT,        // 连发码状态（长按）
    NEC_STATE_DONE           // 解码完成（32位数据完整）
} NEC_StateEnum;

typedef struct
{
	uint32_t FullCode; //完整数据
	uint8_t Addres;    //地址位
	uint8_t Data;      //数据位
	
	uint8_t  Flag;          // 有效标志（1=解码完成）
	NEC_StateEnum  State;   // 状态（空闲/接收中/连发）
	uint8_t  PulseCount;    // 捕获次数
	uint16_t PulseWidth;    // 脉宽
	uint16_t ThisCapture;     // 当前捕获值
	uint16_t LastCapture;   // 上次捕获值
	uint16_t RepeatCount;   // 连发计数

	uint8_t  KeyUpFlag;     // 按钮抬起
}IR_NEC_HandleTypedef;

typedef enum
{
	Key_Zero 		=  0,
	Key_One 		=  1,
	Key_Two			=  2,
	Key_three		=  3,
	Key_Four		=  4, 
	Key_Five		=  5, 
	Key_Six			=  6,
	Key_Seven		=  7,
	Key_Eight		=  8,
	Key_Nine    =  9,
	Key_ERROR 	= 10,//无效按键
	Key_XingHao = 11,
	Key_JingHao = 12,
	Key_W 			= 13,
	Key_S 			= 14,
	Key_A 			= 15,
	Key_D 			= 16,
	Key_OK 			= 17,	
}Key_Enum;

void NEC_Init(void);//初始化红外遥控所有通道及其中断初始化

int IR_GetKey(uint8_t *key);//获取当前按钮值 

#endif
