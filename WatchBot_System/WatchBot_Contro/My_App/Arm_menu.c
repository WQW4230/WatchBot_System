#include "Arm_menu.h"
#include "Scheduler.h"
#include "OLED.h"
#include "NEC_driver.h"
#include "arm_control.h"
#include "PS2_Control_arm.h"
#include "Arm_Action_Data.h"

extern void Menu_Proc(void);
extern void USART_FrameHandler_Task(void);//串口控制舵机臂

static ArmMenu_Status_t arm_menu;//全局控制句柄

const char *Mode_Name[] =
{
	"遥控控制",
	"摇杆控制",
	"自动控制",
	"动作选择",
};

void ArmMenu_Init(void)
{
	//舵臂—摇杆初始化
	Arm_Init();
	JOY_Control_Init();
	
	//Arm_deom();//测试用
	
	Scheduler_AddTask(ArmMode_Task, 10, 4, 1000);

	Arm_MoveTo(-90, -60, -90, 0);
	
	arm_menu.mode = ARM_MODE_REMOTE;
	arm_menu.show = ARM_MENU_HOME;
	arm_menu.Action_Status = ARM_STATUS_STOP;
	arm_menu.mode_cursor = 0;
	arm_menu.remote_cursor = 0;
	arm_menu.action_cursor = 0;
	
}

//自动控制菜单显示
static void ArmAuto_Show(ArmMenu_Status_t *statu)
{
	const Arm_Angle_t * Current = Arm_GetCurrent_Angle();
	const Arm_Angle_t * Target  = Arm_GetTarget_Angle();
	
	OLED_Clear();
	
	OLED_Printf(0, 	0, OLED_8X16, "Pan ");
	OLED_ShowFloatNum(40, 0, Current->Pan_Angle, 2, 3, OLED_8X16);

	OLED_Printf(0, 16, OLED_8X16, "Roll ");
	OLED_ShowFloatNum(40, 16, Current->Roll_Angle, 2, 3, OLED_8X16);
	
	OLED_Printf(0, 32, OLED_8X16, "Tilt");
	OLED_ShowFloatNum(40, 32, Current->Tilt_Angle, 2, 3, OLED_8X16);
	
	OLED_Printf(0, 48, OLED_8X16, "Fan  ");
	OLED_ShowFloatNum(40, 48, Current->Fan_Speed, 2, 3, OLED_8X16);
	
}

//遥控菜单显示
static void ArmRemote_Show(ArmMenu_Status_t *statu)
{
	const Arm_Angle_t * Current = Arm_GetCurrent_Angle();
	const Arm_Angle_t * Target  = Arm_GetTarget_Angle();
	
	OLED_Clear();
	OLED_Printf(0, 	0, OLED_8X16, "Pan ");
	OLED_ShowFloatNum(40, 0, Current->Pan_Angle, 2, 3, OLED_6X8);
	OLED_ShowFloatNum(40, 8, Target->Pan_Angle, 2, 3, OLED_6X8);
	OLED_ShowFloatNum(85, 0, statu->remote_angle.Pan_Angle, 2, 1, OLED_8X16);
	
	OLED_Printf(0, 16, OLED_8X16, "Roll ");
	OLED_ShowFloatNum(40, 16, Current->Roll_Angle, 2, 3, OLED_6X8);
	OLED_ShowFloatNum(40, 24, Target->Roll_Angle, 2, 3, OLED_6X8);
	OLED_ShowFloatNum(85, 16, statu->remote_angle.Roll_Angle, 2, 1, OLED_8X16);
	
	OLED_Printf(0, 32, OLED_8X16, "Tilt");
	OLED_ShowFloatNum(40, 32, Current->Tilt_Angle, 2, 3, OLED_6X8);
	OLED_ShowFloatNum(40, 40, Target->Tilt_Angle, 2, 3, OLED_6X8);
	OLED_ShowFloatNum(85, 32, statu->remote_angle.Tilt_Angle, 2, 1, OLED_8X16);
	
	OLED_Printf(0, 48, OLED_8X16, "Fan  ");
	OLED_ShowFloatNum(40, 48, Current->Fan_Speed, 2, 3, OLED_6X8);
	OLED_ShowFloatNum(40, 56, Target->Fan_Speed, 2, 3, OLED_6X8);
	OLED_ShowFloatNum(85, 48, statu->remote_angle.Fan_Speed, 2, 1, OLED_8X16);
	
	OLED_ReverseArea(85, 16 * statu->remote_cursor, 128, 16);
}


//主页菜单显示
static void ArmMenu_Show(ArmMenu_Status_t *statu)
{
	OLED_Clear();
	for(uint8_t i=0; i<ARM_OPTIONS_COUNT; i++)
	{
		OLED_Printf(0, i * 16, OLED_8X16, "%s", Mode_Name[i]);
		if(i == statu->mode_cursor)
		{
			OLED_ReverseArea(0, i * 16, 128, 16);
		}
	}
}

//红外输入判断设定角度是整数还是负数
float Arm_IR_InputJudgment(float Angle, float num)
{
	if(Angle >= 0) 
		return num;
	else
		return -num;
}

//红外任务进程 控制设置
static void Arm_Remote_Proc(ArmMenu_Status_t *statu)
{
	uint8_t key;	//存按键值
	if(IR_GetKey(&key) == -1) return;//无按键或错误
	
	switch(key)
	{
		case Key_W:
			if(statu->remote_cursor > 0)
				statu->remote_cursor--;
			else
				statu->remote_cursor = 3;
		break;
			
		case Key_S:
			if(statu->remote_cursor < 3)
				statu->remote_cursor++;
			else
				statu->remote_cursor = 0;
		break;
		
		case Key_A:
			switch(statu->remote_cursor)
			{
				case 0: statu->remote_angle.Pan_Angle 	-=ARM_IR_ANGLE_STEP; break;
				case 1:	statu->remote_angle.Roll_Angle	-=ARM_IR_ANGLE_STEP; break;
				case 2:	statu->remote_angle.Tilt_Angle -=ARM_IR_ANGLE_STEP; break;
				case 3:	statu->remote_angle.Fan_Speed 	-=ARM_IR_ANGLE_STEP; break;
			}
		break;
		
		case Key_D:
			switch(statu->remote_cursor)
			{
				case 0: statu->remote_angle.Pan_Angle 	+=ARM_IR_ANGLE_STEP; break;
				case 1:	statu->remote_angle.Roll_Angle 	+=ARM_IR_ANGLE_STEP; break;
				case 2:	statu->remote_angle.Tilt_Angle +=ARM_IR_ANGLE_STEP; break;
				case 3:	statu->remote_angle.Fan_Speed 	+=ARM_IR_ANGLE_STEP; break;
			}
		break;
		
		case Key_OK:
			Arm_MoveTo(statu->remote_angle.Pan_Angle , statu->remote_angle.Roll_Angle,
								 statu->remote_angle.Tilt_Angle, statu->remote_angle.Fan_Speed  );
		break;
		
		case Key_XingHao:
			statu->show = ARM_MENU_HOME;
		break;
		
		case Key_JingHao:
			switch(statu->remote_cursor)
			{
				case 0: statu->remote_angle.Pan_Angle 	= 0; break;
				case 1:	statu->remote_angle.Roll_Angle 	= 0; break;
				case 2:	statu->remote_angle.Tilt_Angle = 0; break;
				case 3:	statu->remote_angle.Fan_Speed 	= 0; break;
			}
		break;
			
		default:
			if(key > 9) break;//不是数字键
			
			switch(statu->remote_cursor)
			{
				case 0: statu->remote_angle.Pan_Angle  = 10 * statu->remote_angle.Pan_Angle + Arm_IR_InputJudgment(statu->remote_angle.Pan_Angle, key); break;
				case 1:	statu->remote_angle.Roll_Angle 	= 10 * statu->remote_angle.Roll_Angle + Arm_IR_InputJudgment(statu->remote_angle.Roll_Angle, key); break;
				case 2:	statu->remote_angle.Tilt_Angle = 10 * statu->remote_angle.Tilt_Angle + Arm_IR_InputJudgment(statu->remote_angle.Tilt_Angle, key); break;
				case 3:	statu->remote_angle.Fan_Speed 	= 10 * statu->remote_angle.Fan_Speed + Arm_IR_InputJudgment(statu->remote_angle.Fan_Speed, key); break;
			}
		break;
	}
}	

//自动&摇杆任务进程
static void ArmAuro_Proc(ArmMenu_Status_t *statu)
{
	uint8_t key;	//存按键值
	if(IR_GetKey(&key) == -1) return;//无按键或错误
	
	switch(key)
	{
		case Key_XingHao:
			statu->show = ARM_MENU_HOME;
		break;
	}
}

//动作运行任务进程 确认动作后在调度器不断轮询
static void ActionRum(void)
{	
	const Arm_ActionaGroup *p1 = &Actiona_Table[arm_menu.action_cursor]; //当前选中动作表简化名字
	
	arm_menu.Action_Status = ARM_STATUS_RUN; //正在表演
	
	uint16_t index = arm_menu.Action_Index;//动作索引
	
	//表演结束
	if(index == p1->Actions_ChangesNum)
	{
		arm_menu.Action_Index = 0;
		vTask_Delete(ActionRum);
		arm_menu.Action_Status = ARM_STATUS_STOP;
		return;
	}
	
	float Pan  = p1->ActionList[index].Pan_Angle;
	float roll  = p1->ActionList[index].Roll_Angle;
	float Tilt = p1->ActionList[index].Tilt_Angle; 
	float fan   =  p1->ActionList[index].Fan_Speed;
	uint16_t delay = p1->ActionList[index].Duration;
	
	Arm_MoveTo(Pan, roll, Tilt, fan);
	arm_menu.Action_Index++;
	vTask_Delay(delay);
}

//创建后台动作任务
static void ActionStart(void)
{
	if(arm_menu.Action_Status == ARM_STATUS_STOP)
	{
		arm_menu.Action_Status = ARM_STATUS_START;
	}
}

//检查动作是否需要删除 中途退出或者更改动作
static void ActionStop(void)
{
	if(arm_menu.Action_Status == ARM_STATUS_STOP)
		return;
	arm_menu.Action_Index = 0;
	arm_menu.Action_Status = ARM_STATUS_STOP;
	vTask_Delete(ActionRum);
}

//切换动作或者退出动作模式检查当前状态
static void Change_Action()
{
	 switch(arm_menu.Action_Status)
	 {
		 case ARM_STATUS_START:  
			 Scheduler_AddTask(ActionRum, 200, 2, 1000);
			 arm_menu.Action_Status = ARM_STATUS_RUN;
			 break;
		 case ARM_STATUS_RUN:
			 break;
		 
		 case ARM_STATUS_STOP:
			 //等待下一次任务
			 break;
		 
		 case ARM_STATUS_IDLE:
			 break;
	 }
}

//动作二级菜单任务进程
static void Arm_ActionSecondary(ArmMenu_Status_t *status)
{
	uint8_t key;	//存按键值
	if(IR_GetKey(&key) == -1) return;//无按键或错误
	
	switch(key)
	{
		case Key_XingHao:
			ActionStop();
			break;
	}
}

//动作选择一级菜单任务进程
static void Arm_Action(ArmMenu_Status_t *statu)
{
	uint8_t key;	//存按键值
	if(IR_GetKey(&key) == -1) return;//无按键或错误
	
	switch(key)
	{
		case Key_W:
			if(statu->action_cursor > 0)
				statu->action_cursor--;
			else
				statu->action_cursor = 3;
			break;
		
		case Key_S:
			if(statu->action_cursor < 3)
				statu->action_cursor++;
			else
				statu->action_cursor = 0;
			break;
		
		case Key_OK:
			ActionStart();
//			statu->Action_Status = ARM_STATUS_IDLE;
			break;
		
		case Key_XingHao:
			statu->show = ARM_MENU_HOME;
			break;
		
	}
}

//动作显示菜单
void ArmAction_Show(ArmMenu_Status_t *status, const Arm_ActionaGroup *Actiona_Table)
{
	OLED_Clear();
	for(uint8_t i=0; i<3; i++)
	{
		OLED_Printf(0, 16 * i, OLED_8X16, "%s", Actiona_Table[i].Action_Name);
		if(status->action_cursor == i)
			OLED_ReverseArea(0, 16 * i, 128, 16);
	}
}

//主菜单任务进程
void ArmMenu_Home(ArmMenu_Status_t *statu)
{
	uint8_t key;	//存按键值
	if(IR_GetKey(&key) == -1) return;//无按键或错误
	
	switch(key)
	{
		case Key_W:
			if(arm_menu.mode_cursor > 0)
			{
				arm_menu.mode_cursor--; //选择光标与模式对齐
				arm_menu.mode--;
			}
			else
			{
				arm_menu.mode_cursor = ARM_OPTIONS_COUNT - 1;;
				arm_menu.mode = ARM_MODE_ACTION;
			}
		break;
		
		case Key_S:
			if(arm_menu.mode_cursor < ARM_OPTIONS_COUNT - 1)
			{
				arm_menu.mode_cursor++;
				arm_menu.mode++;
			}
			else
			{
				arm_menu.mode_cursor = 0;
				arm_menu.mode = ARM_MODE_REMOTE;
			}
		break;
		
		case Key_OK:
			switch(arm_menu.mode)
			{
				case ARM_MODE_REMOTE:
					arm_menu.show = ARM_MENU_REMOTE;
				break;
				
				case ARM_MODE_JOYSTICK:
					arm_menu.show = ARM_MENU_JOYSTICK;
				break;
				
				case ARM_MODE_AUTO:
					arm_menu.show = ARM_MENU_AUTO;
				break;
				
				case ARM_MODE_ACTION:
					arm_menu.show = ARM_MENU_ACTION;
				break;
			}
			
		break;
		
		//*号退出
		case Key_XingHao:
			Scheduler_AddTask(Menu_Proc, 50, 5, 1000);
			vTask_Delete(ArmMenu_Proc);
		break;
	}
}


//显示选择
void ArmMode_Details(ArmMenu_Status_t *statu)
{
	switch(statu->show)
	{
		case ARM_MENU_HOME:
			ArmMenu_Home(statu);
			ArmMenu_Show(statu);
		break;
		
		case ARM_MENU_REMOTE:
			ArmRemote_Show(statu);
			Arm_Remote_Proc(statu);
		break;
		
		case ARM_MENU_JOYSTICK:
			ArmAuto_Show(statu);
			ArmAuro_Proc(statu);
		break;
		
		case ARM_MENU_AUTO:
			ArmAuto_Show(statu);
			ArmAuro_Proc(statu);
		break;
		
		case ARM_MENU_ACTION:
			if(statu->Action_Status == ARM_STATUS_STOP) //停止状态才能显示
			{
				ArmAction_Show(statu, Actiona_Table);
				Arm_Action(statu);
			}
			else
			{	
				Arm_ActionSecondary(statu);
				ArmAuto_Show(statu);
			}
		break;
	}
}

//后台调用轮询舵机臂模式
void ArmMode_Task(void)
{
	switch(arm_menu.mode)
	{
		case ARM_MODE_REMOTE://遥控控制
			Arm_Update();
			break;
		
		case ARM_MODE_JOYSTICK://摇杆控制
			Arm_Update();
			PS2_Uptada();
			break;
		
		case ARM_MODE_AUTO://自动控制
			Arm_Update();
			USART_FrameHandler_Task();
			break;
		
		case ARM_MODE_ACTION://选择动作
			Change_Action();
			Arm_Update();
			break;
	}
}

//菜单任务进程
void ArmMenu_Proc(void)
{
	ArmMode_Details(&arm_menu);
}


