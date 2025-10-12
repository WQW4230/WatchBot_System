#include "stm32f10x.h"      
#include "serov_driver.h"   
#include "arm_control.h"     
#include "OLED.h"
#include "remote_nec_driver.h"
#include "ps2_ADC_DMA.h"
#include "menu.h"
#include "Ps2_key.h"


uint8_t Arm_Flag = 0;

//三路舵机一路直流电机的控制句柄
ServoAngles_t ServoAngles = {0, 0, 0, 0};

static uint8_t Arm_Cursor = 0;



//比较寄存器值500—2500为0°-180°
//基座角度控制
void Arm_SetBase(float base)
{
	if(base < 0.0)
	{
		ServoAngles.base = base = 0.0;
	}
	if(base > 185)
	{
		ServoAngles.base = base = 185; //限位185°		
	}
	PWM_SetCompare1((base /180 *2000 + 500));
}

//大臂角度控制
void Arm_SetShoulder(float shoulder)
{
	if(shoulder < 0.0)
	{
		ServoAngles.shoulder = shoulder = 0.0;
	}
	if(shoulder > 185)
	{
		ServoAngles.shoulder = shoulder = 185; //限位185°	
	}
	PWM_SetCompare2((shoulder /180 *2000 + 500));
}

//末端角度控制
void Arm_SetElbow(float elbow)
{
	if(elbow < 0.0)
	{
		ServoAngles.elbow = elbow = 0.0;
	}
	if(elbow > 185)
	{
		ServoAngles.elbow = elbow = 185; //限位185°
	}		
	PWM_SetCompare3((elbow /180 *2000 + 500));
}

//风扇转速控制
void Arm_SetFanSpeed(int16_t FanSpeed)
{
	if(FanSpeed >= 100)
	{
		FanSpeed = 100;
	}
	if(FanSpeed <= -100)
	{
		FanSpeed = -100;
	}
	if(FanSpeed >= 0)
	{
		GPIO_SetBits(GPIOB, GPIO_Pin_12);	//PA4置高电平
		GPIO_ResetBits(GPIOB, GPIO_Pin_13);	//PA5置低电平，设置方向为正转
		ServoAngles.FanSpeed = FanSpeed;
		PWM_SetCompare4(FanSpeed * 10);
	}
	else
	{
		GPIO_ResetBits(GPIOB, GPIO_Pin_12);	//PA4置低电平
		GPIO_SetBits(GPIOB, GPIO_Pin_13);	//PA5置高电平，设置方向为正转
		ServoAngles.FanSpeed = FanSpeed; //限位最高转速100
		PWM_SetCompare4( -FanSpeed * 10);
	}		

}

//机械臂角度控制
void Arm_MoveJoints(float base, float shoulder, float elbow, int16_t FanSpeed)
{
	Arm_SetBase(base);
	Arm_SetShoulder(shoulder);
	Arm_SetElbow(elbow);
	Arm_SetFanSpeed(FanSpeed);
	
}

//实时角度更新
void Arm_Update(void)
{
	Arm_SetBase(ServoAngles.base);
	Arm_SetShoulder(ServoAngles.shoulder);
	Arm_SetElbow(ServoAngles.elbow);
	Arm_SetFanSpeed(ServoAngles.FanSpeed);
}

//光标对应的电机
static void Arm_CursorSwitch(Key_Enum key)
{
	if(key == Key_A)
	{
		switch(Arm_Cursor)
		{
			case 0:ServoAngles.base -= ARM_STEP ; 		break;
			case 1:ServoAngles.shoulder -= ARM_STEP;  break;
			case 2:ServoAngles.elbow -= ARM_STEP; 		break;
			case 3:
				ServoAngles.FanSpeed -= ARM_STEP; 			
				if(ServoAngles.FanSpeed < FAN_MIN)
				{
					ServoAngles.FanSpeed = FAN_MIN;
				}			
				break;
		}
	}
	if(key == Key_D)
	{
		switch(Arm_Cursor)
		{
			case 0:ServoAngles.base += ARM_STEP; 		break;
			case 1:ServoAngles.shoulder += ARM_STEP; break;
			case 2:ServoAngles.elbow += ARM_STEP; 		break;
			case 3:
				ServoAngles.FanSpeed += ARM_STEP; 			
				if(ServoAngles.FanSpeed > FAN_MAX)
				{
					ServoAngles.FanSpeed = FAN_MAX;
				}
				break;
		}
	}
	//调用将值写入舵机
	Arm_Update();
}

//绘制函数
static void Arm_Show_Status(void)
{
	OLED_Clear();
	OLED_Printf(0, 0, OLED_8X16, "  Base   %f", ServoAngles.base);
	OLED_Printf(0, 16, OLED_8X16, "Shoulder %f", ServoAngles.shoulder);
	OLED_Printf(0, 32, OLED_8X16, " Elbow   %f", ServoAngles.elbow);
	OLED_Printf(0, 48, OLED_8X16, "FanSpeed %d", ServoAngles.FanSpeed);
	OLED_ReverseArea(0, (Arm_Cursor * 16), 128, 16);
	OLED_Update();
}

//机械臂复位
void Arm_Reset(void)
{
	ServoAngles.base = 0;
	ServoAngles.shoulder = 0;
	ServoAngles.elbow = 0;
	ServoAngles.FanSpeed = 0;
	Arm_Update();
}

//进程函数
void Arm_Proc(void)
{
	//摇杆抢夺控制权
	if(PS2_KeyFlag == 1)
	{
		ServoAngles.base = 185 - (PS2_AD[0] * 185 / 4095); 	
		ServoAngles.shoulder = PS2_AD[1] * 185 / 4095;
		ServoAngles.elbow = 185 - (PS2_AD[2] * 185 / 4095); 	
		ServoAngles.FanSpeed = (PS2_AD[3] - 2048) * 100 / 2048;
		Arm_Update();
	}
	if(Arm_Flag == 0) return;
	
	if(IR_Flag == 1)
	{
		IR_Flag = 0;
		switch(IR_GetKey())
		{
			case Key_A:
			case Key_D:
				Arm_CursorSwitch(IR_GetKey());
				break;
			
			case Key_W:
			{
				if(Arm_Cursor > 0){Arm_Cursor--;}						
				break;
			}	
			case Key_S:
			{
				if(Arm_Cursor < 3){Arm_Cursor++;}
				break;
			}	
			case Key_XingHao:
			{
				Arm_Flag = 0;
				Menu_Flag = 1;
				OLED_Clear();
				break;
			}
				
			case Key_JingHao:
			{
				
				break;
			}
		}
					
	}
		
	Arm_Show_Status();
	
	
}
