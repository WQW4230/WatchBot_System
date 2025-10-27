#include "Arm_menu.h"
#include "Scheduler.h"
#include "OLED.h"
#include "arm_control.h"
#include "PS2_Control_arm.h"

static ArmMenu_Status_t arm_menu;//全局控制句柄

const char *Mode_Name[] =
{
	"遥控控制",
	"红外控制",
	"自动控制",
	"动作选择",
};

void ArmMenu_Init(void)
{
	//舵臂—摇杆初始化
	Arm_Init();
	JOY_Control_Init();
	
	arm_menu.mode = ARM_MODE_REMOTE;
	arm_menu.mode_cursor = 0;
	arm_menu.remote_cursor = 0;
	arm_menu.action_cursor = 0;
	
	Arm_deom();
	
	Scheduler_AddTask(Arm_Update, 50, 5, 1000);
}

void ArmMenu_Proc(void)
{
	
}





























