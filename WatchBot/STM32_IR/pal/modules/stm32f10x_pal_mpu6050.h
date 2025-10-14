/**
  ******************************************************************************
  * @file    stm32f10x_pal_mpu6050.h
  * @author  铁头山羊stm32工作组
  * @version V3.0.0
  * @date    2024年1月24日
  * @brief   mpu6050驱动
  ******************************************************************************
  * @attention
  * Copyright (c) 2022 - 2024 东莞市明玉科技有限公司. 保留所有权力.
  ******************************************************************************
*/


#ifndef _STM32F10x_PAL_MPU6050_H_
#define _STM32F10x_PAL_MPU6050_H_

#include "stm32f10x.h"
#include "stm32f10x_pal_i2c.h"

typedef struct 
{
	float gx;
	float gy;
	float gz;
	float ax;
	float temperature;
	float ay;
	float az;
	float yaw;
	float pitch;
	float roll;
} PalMPU6050_CacheTypeDef;

typedef struct
{
	float x;
	float y;
	float z;
} Vector3D_f;

typedef struct
{
	PalI2C_HandleTypeDef *I2Cx; /* 用于和mpu6050通信的I2C句柄，mpu6050支持快速模式 */
	Vector3D_f GyroBias;        /* 陀螺仪校准向量 */
	Vector3D_f AccelBias;       /* 加速度传感器校准向量 */
} PalMPU6050_InitTypeDef;

typedef struct
{
	PalMPU6050_InitTypeDef Init;
	PalMPU6050_CacheTypeDef Cache;
	float gyro_sense;
	float accel_sense;
} PalMPU6050_HandleTypeDef;

void PAL_MPU6050_InitHandle(PalMPU6050_HandleTypeDef *Handle);
ErrorStatus PAL_MPU6050_Init(PalMPU6050_HandleTypeDef *Handle);
ErrorStatus PAL_MPU6050_Proc(PalMPU6050_HandleTypeDef *Handle);
float PAL_MPU6050_GetAccelX(PalMPU6050_HandleTypeDef *Handle);
float PAL_MPU6050_GetAccelY(PalMPU6050_HandleTypeDef *Handle);
float PAL_MPU6050_GetAccelZ(PalMPU6050_HandleTypeDef *Handle);
float PAL_MPU6050_GetGyroX(PalMPU6050_HandleTypeDef *Handle);
float PAL_MPU6050_GetGyroY(PalMPU6050_HandleTypeDef *Handle);
float PAL_MPU6050_GetGyroZ(PalMPU6050_HandleTypeDef *Handle);
float PAL_MPU6050_GetYaw(PalMPU6050_HandleTypeDef *Handle);
float PAL_MPU6050_GetRoll(PalMPU6050_HandleTypeDef *Handle);
float PAL_MPU6050_GetPitch(PalMPU6050_HandleTypeDef *Handle);
float PAL_MPU6050_GetTemperature(PalMPU6050_HandleTypeDef *Handle);

#endif
