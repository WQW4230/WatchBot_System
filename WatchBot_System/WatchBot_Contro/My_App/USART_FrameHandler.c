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

///////////传输一字节八位，不能表示负数和浮点数，+90 *100转换成无符号16位 拿高低位传输///////////////
///////////传输到了再转成浮点数并且-90转换负数 公式 angle = (x / 100.0f - 90)///////////////////////
/*
	串口控制偏航角 翻滚角
*/
static void arm_update_pan_roll_tilt(uint8_t *status)
{
	//获取当前舵机臂目标角度
	const Arm_Angle_t *Angle = Arm_GetTarget_Angle();
	
	uint16_t pan_u16   = ((uint16_t)(status[3] << 8)) | status[4];
	uint16_t roll_u16  = ((uint16_t)(status[5] << 8)) | status[6];
	uint16_t tilt_u16  = ((uint16_t)(status[7] << 8)) | status[8];
	
	float angle_pan  = (float)(pan_u16 / 100.0f - 90);
	float angle_roll = (float)(roll_u16 / 100.0f - 90);
	float angle_tilt = (float)(tilt_u16 / 100.0f - 90);
	Arm_MoveTo(angle_pan, angle_roll, angle_tilt, Angle->Fan_Speed);
}
/*
	串口控制俯仰角 电机转速
*/
//static void arm_update_tilt_fan(uint8_t *status)
//{
//	//获取当前舵机臂目标角度
//	const Arm_Angle_t *Angle = Arm_GetTarget_Angle();
//	
//	uint16_t tilt_u16 = ((uint16_t)(status[3] << 8)) | status[4];
//	uint16_t fan_u16  = ((uint16_t)(status[5] << 8)) | status[6];
//	
//	float angle_tilt  = (float)(tilt_u16 / 100.0f - 90);
//	float angle_fan   = (float)(fan_u16 / 100.0f - 90);
//	Arm_MoveTo(Angle->Pan_Angle, Angle->Roll_Angle, angle_tilt, angle_fan);

//}

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
	uint8_t Frame_buf[DATA_ARM_LEN];
	if(USART_ReadFrame(Frame_buf) != 1) return;
	
	uint8_t cmd = Frame_buf[FRAME_IDX_CMD];//命令位
	
	switch(cmd)
	{
		case CMD_ARM_CONTROL://控制机械臂
			arm_update_pan_roll_tilt(Frame_buf);
		break;	
		case CMD_BUZZER_CTRL: //控制蜂鸣器
			Cmd_BuzzerControl(Frame_buf);
		break;
		case CMD_BUZZER_OFF:  //关闭蜂鸣器
			Cmd_BuzzerOFF(Frame_buf);
		break;
	}
}

/*
	发送不带参数指令集
	CMD： 命令
*/
void UART_SenCmd(UART_TxCommand_e Cmd)
{
	uint8_t arr[5] = {FRAME_HEADER, Cmd, 0x05, NO_PARAM_8, FRAME_END};
	
	USART3_SendString(arr);
}

/*
	发送带参数指令集控制ESP32led状态
	CMD： 命令
	ON ： 开启时间
	OFF： 关闭时间
*/
void USART_SenFrame(UART_TxCommand_e Cmd, uint16_t ON, uint16_t OFF)
{
	uint8_t On_High, On_Low, Off_High, Off_Low;
  On_High = (uint8_t)(ON >> 8); // 右移 8 位，获取高字节
  On_Low  = (uint8_t)(ON & 0xFF); // 低字节取 ON 的低 8 位
	
  Off_High = (uint8_t)(OFF >> 8); // 右移 8 位，获取高字节
  Off_Low  = (uint8_t)(OFF & 0xFF); // 低字节取 OFF 的低 8 位
	uint8_t arr[8] = {FRAME_HEADER, Cmd, 0x08, On_High, On_Low, Off_High, Off_Low, FRAME_END};
	
	USART3_SendString(arr);
}
