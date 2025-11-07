#include "USART_FrameHandler.h"
#include "usart_rx_driver.h"
#include "arm_control.h"
#include "Alarm_Clock.h"
#include "Active_Buzzer.h"
#include <stm32f10x.h>

extern Buzzer_t PB14_Buzzer; //蜂鸣器控制句柄

//初始化串口接收
void USART_FrameHandler_Init(void)
{
	usart_slave_Init(); 
}

//设置机械臂角度
//数据位180为90° 90为0° 0为-90°

//机械臂直流电机参数
static void Cmd_ArmControl(uint8_t *Angle)
{
	//if()自动模式开启
	Arm_MoveTo(Angle[3] - 90, Angle[4] - 90, Angle[5] - 90, Angle[6] - 90);
}

//蜂鸣器关闭
static void Cmd_BuzzerOFF(uint8_t *status)
{
	Buzzer_SetMode(&PB14_Buzzer, Buzzer_OFF, status[3], status[4]);
}

//设置蜂鸣器
static void Cmd_BuzzerControl(uint8_t *status)
{
	//小端 发送端将16位拆 2个8位发送
	uint16_t ON = ((uint16_t)(status[3] << 8)) | status[4];
	uint16_t OFF  = ((uint16_t)(status[5] << 8)) | status[6];
	Buzzer_SetMode(&PB14_Buzzer, Buzzer_Beep, ON, OFF);
}

//串口控制硬件 自动模式下启用 
void USART_FrameHandler_Task(void)
{
	uint8_t Frame_buf[DATA_MAX_LEN];
	if(USART_ReadFrame(Frame_buf) != 1) return;
	
	uint8_t cmd = Frame_buf[1];//命令位
	
	switch(cmd)
	{
		case CMD_ARM_CONTROL:
			Cmd_ArmControl(Frame_buf);
			break;
		
		case CMD_BUZZER_CTRL:
			Cmd_BuzzerControl(Frame_buf);
			break;
		
		case CMD_BUZZER_OFF:
			Cmd_BuzzerOFF(Frame_buf);
			break;
	}
}

/*
	发送指令集
	CMD： 命令位
	ON ： 开启时间
	OFF： 关闭时间
*/
void USART_SenFrame(uint8_t CMD, uint16_t ON, uint16_t OFF)
{
	uint8_t On_High, On_Low, Off_High, Off_Low;
  On_High = (uint8_t)(ON >> 8); // 右移 8 位，获取高字节
  On_Low  = (uint8_t)(ON & 0xFF); // 低字节取 ON 的低 8 位
	
  Off_High = (uint8_t)(OFF >> 8); // 右移 8 位，获取高字节
  Off_Low  = (uint8_t)(OFF & 0xFF); // 低字节取 OFF 的低 8 位
	uint8_t arr[8] = {FRAME_HEADER, CMD, 0x08, On_High, On_Low, Off_High, Off_Low, FRAME_END};
	
	USART3_SendString(arr);
}
