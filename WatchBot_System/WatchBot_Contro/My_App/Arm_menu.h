#ifndef _ARM_MENU_H_
#define _ARM_MENU_H_

#include "stm32f10x.h"            

#define ARM_OPTIONS_COUNT 4 //四个选项

#define ARM_IR_ANGLE_STEP 0.1f //遥控器设置角度步幅

typedef enum
{
	ARM_MENU_HOME,    //主页
	ARM_MENU_REMOTE,  //遥控
	ARM_MENU_JOYSTICK,//摇杆
	ARM_MENU_AUTO,		//自动
	ARM_MENU_ACTION		//预设动作
}ArmMenu_Show_e;

typedef enum
{
	ARM_MODE_REMOTE,  //遥控
	ARM_MODE_JOYSTICK,//摇杆
	ARM_MODE_AUTO,		//自动 	
	ARM_MODE_ACTION		//预设动作
}ArmControl_Mode_e;

typedef enum
{
	ARM_STATUS_STOP = 0,   // 不执行任何动作
  ARM_STATUS_START,      // 新动作启动（初始化）
  ARM_STATUS_RUN,        // 正在执行动作
	ARM_STATUS_IDLE,       // 空闲状态
}Arm_Action_Status_e;

typedef struct
{
	float Pan_Angle;    //基座旋转角
	float Roll_Angle;   //大臂翻滚角
	float Tilt_Angle;	  //末端俯仰角
	float Fan_Speed;    //风扇转速
}Remote_SetAngle_t;

typedef struct
{
	ArmMenu_Show_e show;      					//当前显示的界面
	ArmControl_Mode_e mode; 					  //选择的模式
	Arm_Action_Status_e Action_Status; 	//动作演示状态
	Remote_SetAngle_t remote_angle; 		//红外遥控目标值
	uint16_t Action_Index;          	  //当前执行到第几个动作
	uint8_t mode_cursor;						 		//选择模式光标
  uint8_t remote_cursor;  	 					//远程遥控光标
	uint8_t action_cursor;  	 				  //动作选择的光标
		
}ArmMenu_Status_t;

void ArmMenu_Init(void);

void ArmMode_Task(void);//后台调用轮询舵机臂模式
void ArmMenu_Proc(void);

#endif
