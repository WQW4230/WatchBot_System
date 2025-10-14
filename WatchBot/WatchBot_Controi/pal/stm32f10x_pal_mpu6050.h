/**
  ******************************************************************************
  * @file    stm32f10x_pal_mpu6050.h
  * @author  铁头山羊stm32工作组
  * @version V1.0.0
  * @date    2023年2月24日
  * @brief   mpu6050驱动
  ******************************************************************************
  * @attention
  * Copyright (c) 2022 - 2023 东莞市明玉科技有限公司. 保留所有权力.
  ******************************************************************************
*/


#ifndef _STM32F10x_PAL_MPU6050_H_
#define _STM32F10x_PAL_MPU6050_H_
#include "stm32f10x.h"
#include "stm32f10x_pal_i2c.h"
#include "stm32f10x_pal_exti.h"

#define MPU6050_GYROFSR_250 0x00 // 250°/秒
#define MPU6050_GYROFSR_500 0x01 // 500°/秒
#define MPU6050_GYROFSR_1000 0x02 // 1050°/秒
#define MPU6050_GYROFSR_2000 0x03 // 2000°/秒

#define MPU6050_ACCELFSR_2g 0x00 // 2g
#define MPU6050_ACCELFSR_4g 0x01 // 4g
#define MPU6050_ACCELFSR_8g 0x02 // 8g
#define MPU6050_ACCELFSR_16g 0x03 // 16g

typedef struct 
{
	float Yaw;
	float Pitch;
	float Roll;
	float Gyro_x;
	float Gyro_y;
	float Gyro_z;
	float Accel_x;
	float Accel_y;
	float Accel_z;
	float Quat_w;
	float Quat_x;
	float Quat_y;
	float Quat_z;
} PalMPU6050Result_TypeDef;

typedef struct
{
	PalI2C_HandleTypeDef *hI2c;
	uint16_t SampleRate;
//	uint16_t MPU6050_DLFPType;
	uint16_t MPU6050_AccelFsr;
	uint16_t MPU6050_GyroFsr;
	GPIO_TypeDef *Int_GPIOPort; // Int引脚所连接的GPIO端口
	uint16_t Int_GPIOPin; // Int引脚所连接的GPIO引脚
	uint8_t Int_PremptionPriority;
	uint8_t Int_SubPriority;
	void (*Int_Callback)(void); // Int引脚中断回调函数
} PalMPU6050_InitTypeDef;

typedef struct
{
	PalMPU6050_InitTypeDef Init;
	PalEXTI_HandleTypeDef hexti;
	PalMPU6050Result_TypeDef Result;
} PalMPU6050_HandleTypeDef;

void PAL_MPU6050_Init(PalMPU6050_HandleTypeDef *Handle);
void PAL_MPU6050_Int_IRQHandler(PalMPU6050_HandleTypeDef *Handle);
void PAL_MPU6050_IntrruptCmd(PalMPU6050_HandleTypeDef *Handle, FunctionalState NewState);
void PAL_MPU6050_Update(PalMPU6050_HandleTypeDef *Handle);
float PAL_MPU6050_GetYaw(PalMPU6050_HandleTypeDef *Handle);
float PAL_MPU6050_GetRoll(PalMPU6050_HandleTypeDef *Handle);
float PAL_MPU6050_GetPitch(PalMPU6050_HandleTypeDef *Handle);
float PAL_MPU6050_GetGyroX(PalMPU6050_HandleTypeDef *Handle);
float PAL_MPU6050_GetGyroY(PalMPU6050_HandleTypeDef *Handle);
float PAL_MPU6050_GetGyroZ(PalMPU6050_HandleTypeDef *Handle);
float PAL_MPU6050_GetAccelX(PalMPU6050_HandleTypeDef *Handle);
float PAL_MPU6050_GetAccelY(PalMPU6050_HandleTypeDef *Handle);
float PAL_MPU6050_GetAccelZ(PalMPU6050_HandleTypeDef *Handle);
void PAL_MPU6050_GetAllResult(PalMPU6050_HandleTypeDef *Handle, PalMPU6050Result_TypeDef *pResult);

#endif
