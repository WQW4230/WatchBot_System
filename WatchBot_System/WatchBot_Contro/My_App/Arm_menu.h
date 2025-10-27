#ifndef _ARM_MENU_H_
#define _ARM_MENU_H_

#include "stm32f10x.h"            

typedef enum
{
	ARM_MODE_REMOTE,  //遥控
	ARM_MODE_JOYSTICK,//摇杆
	ARM_MODE_AUTO,		//自动
	ARM_MODE_ACTION		//预设动作
}ArmControl_Mode_e;

typedef struct
{
	ArmControl_Mode_e mode; //选择的模式
	uint8_t mode_cursor;		//选择模式光标
  uint8_t remote_cursor;  //远程遥控光标
	uint8_t action_cursor;  //动作选择的光标
	
}ArmMenu_Status_t;

void ArmMenu_Init(void);

void ArmMenu_Proc(void);

#endif
