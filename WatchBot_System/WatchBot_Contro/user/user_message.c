/**
  ******************************************************************************
  * @file    user_message.c
  * @author  铁头山羊
  * @version V 1.0.0
  * @date    2023年4月24日
  * @brief   用户消息处理
  ******************************************************************************
  * @attention
  * Copyright (c) 2022 - 2023 东莞市明玉科技有限公司. 保留所有权力.
  ******************************************************************************
*/
#include "user_message.h"
#include "pal_message_codec.h"
#include "pal_pid.h"
#include "stm32f10x_pal_w25q64.h"
#include "user_persistance.h"

#define PID_BALANCE 0x00 // 平衡环
#define PID_SPEED   0x01 // 速度环

#define USER_MESSAGE_ID_UPMAP       0x00
#define USER_MESSAGE_ID_DOWNMAP     0x01 // 下载参数
#define USER_MESSAGE_ID_UPMAP_REPLY 0x02 
#define USER_MESSAGE_ID_PID_BURN    0x03 // 烧录
#define USER_MESSAGE_ID_SET_MECHANICAL_ANGLE 0x04
#define USER_MESSAGE_ID_READ_MECHANICAL_ANGLE 0x05
#define USER_MESSAGE_ID_READ_MECHANICAL_ANGLE_REPLY 0x06

extern PalMessageCodec_HandleTypeDef hMessageCodec;
extern PalPID_HandleTypeDef pid_balance;
extern PalPID_HandleTypeDef pid_speed;

static void User_Message_ReadMechanicalAngleHandler(uint8_t *arg, uint8_t arglen);
static void User_Message_SetMechanicalAngleHandler(uint8_t *arg, uint8_t arglen);
static void User_Message_UpmapHandler(uint8_t *arg, uint8_t arglen);
static void User_Message_DownmapHandler(uint8_t *arg, uint8_t arglen);
static void User_Message_BurnHandler(uint8_t *arg, uint8_t arglen);


static uint16_t SerializeUInt8(uint8_t Uint8Value, uint8_t *pBuffer, uint16_t Offset);
static uint16_t SerializeFloat(float FloatValue, uint8_t *pBuffer, uint16_t Offset);
static uint16_t DeserializeUInt8(uint8_t *pBuffer, uint16_t Offset, uint8_t *pUInt8ValueOut);
static uint16_t DeserializeFloat(uint8_t *pBuffer, uint16_t Offset, float *pFloatValueOut);

void User_Message_Process(void)
{
	uint16_t id;
	uint8_t arg[64];
	uint8_t arglen;
	
	if(PAL_MessageCodec_ReceiveMessage(&hMessageCodec, &id, arg, &arglen, 0) == SET)
	{
		switch(id)
		{
			case USER_MESSAGE_ID_UPMAP:
			{
				User_Message_UpmapHandler(arg, arglen);
				break;
			}
			case USER_MESSAGE_ID_DOWNMAP:
			{
				User_Message_DownmapHandler(arg, arglen);
				break;
			}
			case USER_MESSAGE_ID_PID_BURN:
			{
				User_Message_BurnHandler(arg, arglen);
				break;
			}
			case USER_MESSAGE_ID_SET_MECHANICAL_ANGLE:
			{
				User_Message_SetMechanicalAngleHandler(arg, arglen);
				break;
			}
			case USER_MESSAGE_ID_READ_MECHANICAL_ANGLE:
			{
				User_Message_ReadMechanicalAngleHandler(arg, arglen);
				break;
			}
			default:break;
		}
	}
}

extern float robot_mechanical_angle;

static void User_Message_ReadMechanicalAngleHandler(uint8_t *arg, uint8_t arglen)
{
	// 创建一个 read mechanical angle reply 消息，并发送出去
	uint8_t buffer[18]; 
	
	uint16_t offset = 0;
	
	offset += SerializeFloat(robot_mechanical_angle, buffer, offset);
	
	PAL_MessageCodec_SendMessage(&hMessageCodec, USER_MESSAGE_ID_READ_MECHANICAL_ANGLE_REPLY, buffer, offset);
}

static void User_Message_SetMechanicalAngleHandler(uint8_t *arg, uint8_t arglen)
{
	// 解析消息
	float newAngle;
	uint16_t offset = 0;
	
	offset += DeserializeFloat(arg, offset, &newAngle);
	
	// 设置新值
	__disable_irq();
	robot_mechanical_angle = newAngle;
	__enable_irq();
}

static void User_Message_UpmapHandler(uint8_t *arg, uint8_t arglen)
{
	// 解析消息
	uint8_t which = arg[0];
	PalPID_HandleTypeDef *pid;
	
	if(which == PID_BALANCE)
	{
		pid = &pid_balance;
	}
	else
	{
		pid = &pid_speed;
	}
	
	// 创建一个 upmap reply 消息，并发送出去
	uint8_t buffer[18]; 
	
	uint16_t offset = 0;
	
	// which
	offset += SerializeUInt8(which, buffer, offset);
	
	// sp
	offset += SerializeFloat(pid->Setpoint, buffer, offset);
	
	// kp
	offset += SerializeFloat(pid->Kp, buffer, offset);
	
	// ki
	offset += SerializeFloat(pid->Ki, buffer, offset);
	
	// kd
	offset += SerializeFloat(pid->Kd, buffer, offset);
	
	// manual
	offset += SerializeUInt8(pid->Manual, buffer, offset);
	
	// manual output
	offset += SerializeFloat(pid->ManualOutput, buffer, offset);
	
	PAL_MessageCodec_SendMessage(&hMessageCodec, USER_MESSAGE_ID_UPMAP_REPLY, buffer, offset);
}

extern AppPersist_TypeDef AppPersistance;

extern PalW25Q64_HandleTypeDef hw25q64;

extern float robot_mechanical_angle;

static void User_Message_BurnHandler(uint8_t *arg, uint8_t arglen)
{
	// 待补充
	AppPersistance.InitializeToken = PREDEFINED_INITIALIZE_TOKEN;
	
	AppPersistance.MechanicalAngle = robot_mechanical_angle;
	
	AppPersistance.BalancePID.Sp = pid_balance.Setpoint;
	AppPersistance.BalancePID.Kp = pid_balance.Kp;
	AppPersistance.BalancePID.Ki = pid_balance.Ki;
	AppPersistance.BalancePID.Kd = pid_balance.Kd;
	AppPersistance.BalancePID.Manual = pid_balance.Manual;
	AppPersistance.BalancePID.ManualOutput = pid_balance.ManualOutput;
	
	AppPersistance.SpeedPID.Sp = pid_speed.Setpoint;
	AppPersistance.SpeedPID.Kp = pid_speed.Kp;
	AppPersistance.SpeedPID.Ki = pid_speed.Ki;
	AppPersistance.SpeedPID.Kd = pid_speed.Kd;
	AppPersistance.SpeedPID.Manual = pid_speed.Manual;
	AppPersistance.SpeedPID.ManualOutput = pid_speed.ManualOutput;
	
	User_Persist_WriteToFlash(&AppPersistance);
}

static void User_Message_DownmapHandler(uint8_t *arg, uint8_t arglen)
{
	// 解析消息
	uint16_t offset = 0;
	uint8_t which, manual;
	float sp, kp, ki, kd, manualOutput;
	
	// which
	offset += DeserializeUInt8(arg, offset, &which);
	// sp
	offset += DeserializeFloat(arg, offset, &sp);
	// kp
	offset += DeserializeFloat(arg, offset, &kp);
	// ki
	offset += DeserializeFloat(arg, offset, &ki);
	// kd
	offset += DeserializeFloat(arg, offset, &kd);
	// manual
	offset += DeserializeUInt8(arg, offset, &manual);
	// manualOutput
	offset += DeserializeFloat(arg, offset, &manualOutput);
	
	// which pid
	PalPID_HandleTypeDef *pid;
	
	if(which == PID_BALANCE)
	{
		pid = &pid_balance;
	}
	else
	{
		pid = &pid_speed;
	}
	
	// 给pid设置新值
	__disable_irq();
	PAL_PID_ChangeTunings(pid, kp, ki, kd);
	PAL_PID_Cmd(pid, manual ? DISABLE : ENABLE);
	PAL_PID_ChangeManualOutput(pid, manualOutput);
	PAL_PID_ChangeSetpoint(pid, sp);
	__enable_irq();
}

static uint16_t SerializeUInt8(uint8_t Uint8Value, uint8_t *pBuffer, uint16_t Offset)
{
	*(pBuffer + Offset) = Uint8Value;
	return sizeof(uint8_t);
}

static uint16_t SerializeFloat(float FloatValue, uint8_t *pBuffer, uint16_t Offset)
{
	*((float *)(pBuffer + Offset)) = FloatValue;
	return sizeof(float);
}

static uint16_t DeserializeUInt8(uint8_t *pBuffer, uint16_t Offset, uint8_t *pUInt8ValueOut)
{
	*pUInt8ValueOut = *(pBuffer + Offset);
	return sizeof(uint8_t);
}

static uint16_t DeserializeFloat(uint8_t *pBuffer, uint16_t Offset, float *pFloatValueOut)
{
	*pFloatValueOut = *((float *)(pBuffer+Offset));
	return sizeof(float);
}
