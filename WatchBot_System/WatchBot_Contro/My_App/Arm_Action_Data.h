#ifndef _ARM_ACTION_DATA_H_
#define _ARM_ACTION_DATA_H_
#include "stm32f10x.h" 

#define ARM_ACTIONS_NUM 4 //一共四种动作演示

#define QuickTravel_Num (sizeof(QuickTravel_Action) / sizeof(QuickTravel_Action[0]))
#define Bow_Num         (sizeof(Bow_Action) / sizeof(Bow_Action[0]))
#define GoOnPatrol_Num      (sizeof(GoOnPatrol_Action) / sizeof(GoOnPatrol_Action[0]))
	
typedef struct
{
	float Pan_Angle;   //基座旋转角
	float Roll_Angle; 	//大臂翻滚角
	float Tilt_Angle;	//末端俯仰角
	float Fan_Speed;    //风扇转速
	uint16_t Duration;  //动作持续时间
}Arm_ActionAngle_t;

typedef struct
{
	const char *Action_Name;			//动作名
	const Arm_ActionAngle_t *ActionList;//动作表
	const uint16_t Actions_ChangesNum;  //动作变化数量
}Arm_ActionaGroup;

extern const Arm_ActionaGroup Actiona_Table[ARM_ACTIONS_NUM];

#endif

