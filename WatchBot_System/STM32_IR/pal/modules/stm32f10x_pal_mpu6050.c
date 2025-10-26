/**
  ******************************************************************************
  * @file    stm32f10x_pal_mpu6050.c
  * @author  铁头山羊stm32工作组
  * @version V3.0.0
  * @date    2024年1月24日
  * @brief   mpu6050驱动
  ******************************************************************************
  * @attention
  * Copyright (c) 2022 - 2023 东莞市明玉科技有限公司. 保留所有权力.
  ******************************************************************************
*/

#include "stm32f10x_pal.h"
#include "stm32f10x_pal_mpu6050.h"
#include "stm32f10x_pal_exti.h"
#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h"
#include "math.h"
#include "string.h"

#define MPU6050_ADDR_AD0_LOW  0x68
#define MPU6050_ADDR_AD0_HIGH 0x69
#define MPU6050_ADDR_DEFAULT  MPU6050_ADDR_AD0_LOW

#define q30  9.313225746154785e-10
#define deg2rad 57.295779513082320876798154814105 // 180/PI


/* 
 * mpu6050寄存器地址列表 *
 */
#define MPU6050_REG_PRODUCT_ID 0x0c
#define MPU6050_REG_SMPRT_DIV 0x19
#define MPU6050_REG_CONFIG 0x1a
#define MPU6050_REG_GYRO_CONFIG 0x1b
#define MPU6050_REG_ACCEL_CONFIG 0x1c
#define MPU6050_REG_FIFO_EN 0x23
#define MPU6050_REG_ACCEL_XOUT_H 0x3b
#define MPU6050_REG_ACCEL_XOUT_L 0x3c
#define MPU6050_REG_ACCEL_YOUT_H 0x3d
#define MPU6050_REG_ACCEL_YOUT_L 0x3e
#define MPU6050_REG_ACCEL_ZOUT_H 0x3f
#define MPU6050_REG_ACCEL_ZOUT_L 0x40
#define MPU6050_REG_TEMP_OUT_H 0x41
#define MPU6050_REG_TEMP_OUT_L 0x42
#define MPU6050_REG_GYRO_XOUT_H 0x43
#define MPU6050_REG_GYRO_XOUT_L 0x44
#define MPU6050_REG_GYRO_YOUT_H 0x45
#define MPU6050_REG_GYRO_YOUT_L 0x46
#define MPU6050_REG_GYRO_ZOUT_H 0x47
#define MPU6050_REG_GYRO_ZOUT_L 0x48
#define MPU6060_REG_USER_CTRL 0x6a
#define MPU6050_REG_PWR_MGMT_1 0x6b
#define MPU6050_REG_BANK_SEL 0x6d
#define MPU6050_REG_MEM_START_ADDR 0x6e
#define MPU6050_REG_MEM_R_W 0x6f
#define MPU6050_REG_START_H 0x70
#define MPU6050_REG_WHO_AM_I 0x75

#define MPU6050_DLFPTYPE_260Hz_256Hz 0x00
#define MPU6050_DLFPTYPE_184Hz_188Hz 0x01
#define MPU6050_DLFPTYPE_94Hz_98Hz   0x02
#define MPU6050_DLFPTYPE_44Hz_42Hz   0x03
#define MPU6050_DLFPTYPE_21Hz_20Hz   0x04
#define MPU6050_DLFPTYPE_10Hz_10Hz   0x05
#define MPU6050_DLFPTYPE_5Hz_5Hz     0x06
#define MPU6050_DLFPTYPE_No          0x07

static signed char gyro_orientation[9] = {1, 0, 0, 0, 1, 0, 0, 0, 1}; // 方向矩阵（I）
static inline unsigned short inv_orientation_matrix_to_scalar(const signed char *mtx);
static inline unsigned short inv_row_2_scale(const signed char *row);
unsigned char i2c_write(void *context, unsigned char slave_addr, unsigned char reg_addr, unsigned char length, unsigned char const *data);
unsigned char i2c_read(void *context, unsigned char slave_addr, unsigned char reg_addr, unsigned char length, unsigned char *data);
void delay_ms(unsigned long num_ms);
void get_ms(unsigned long *count);


//
// @简介：将句柄的初始化参数设置为默认值
//        AccelBias               = (0,0,0)， 默认情况下不对陀螺仪进行补偿
//        GyroBias                = (0,0,0)， 默认情况下不对加速度传感器进行补偿
// 
// @注意：仍需要手动填写的项目
//        I2Cx                    - 所使用的I2C接口的句柄
//
void PAL_MPU6050_InitHandle(PalMPU6050_HandleTypeDef *Handle)
{
	Handle->Init.AccelBias.x = 0;
	Handle->Init.AccelBias.y = 0;
	Handle->Init.AccelBias.z = 0;
	Handle->Init.GyroBias.x = 0;
	Handle->Init.GyroBias.y = 0;
	Handle->Init.GyroBias.z = 0;
}

//
// @简介：对mpu6050传感器进行初始化
//        该版本的mpu6050驱动使用了官方的dmp进行解算(eMPL v5.1)
// @参数：Handle - mpu6050的句柄
// @参数：SUCCESS - 成功，ERROR - 失败
//
ErrorStatus PAL_MPU6050_Init(PalMPU6050_HandleTypeDef *Handle)
{
	
	/* 1. 对MPU6050进行初始化*/
	
	// 设置采样率、低通滤波器和量程等参数
	mpu_init_t mpu_init_param;
	
	mpu_init_param.sample_rate = 1000;
	mpu_init_param.accel_fsr = 2;
	mpu_init_param.dlfp =  188; // 188, 98, 42, 20, 10, 5.
	mpu_init_param.gyro_fsr = 2000;
	
	switch(mpu_init_param.accel_fsr)
	{
		case 2:
			Handle->accel_sense = 1.0 / 16384; break;
		case 4:
			Handle->accel_sense = 1.0 / 8092;  break;
		case 8:
			Handle->accel_sense = 1.0 / 4096;  break;
		case 16:
			Handle->accel_sense = 1.0 / 2048;  break;
		default:
			return ERROR;
	}
	
	switch(mpu_init_param.gyro_fsr)
	{
		case 250:
			Handle->gyro_sense = 1.0 / 131; break;
		case 500:
			Handle->gyro_sense = 1.0 / 65.5; break;
		case 1000:
			Handle->gyro_sense = 1.0 / 32.8; break;
		case 2000:
			Handle->gyro_sense = 1.0 / 16.4; break;
		default:
			return ERROR;
	}
	
	if(mpu_init(Handle, &mpu_init_param)) 
		return ERROR; // 初始化失败
	
	/* 2. 唤醒陀螺仪和加速度传感器 */
	if(mpu_set_sensors(Handle, INV_XYZ_GYRO | INV_XYZ_ACCEL)) 
		return ERROR;
	
	/* 3. 设置mpu6050的时钟（选择陀螺仪作为时钟更精确） */
	if(mpu_set_clock_source(Handle, INV_CLOCK_GYROX))
		return ERROR;
	
	/* 4. 将陀螺仪和加速度计的原始数据推送到FIFO */
	// 注意，此步骤不可缺少
	// 猜测该数据会在dmp解算时用到
	if(mpu_configure_fifo(Handle, INV_XYZ_GYRO | INV_XYZ_ACCEL))
		return ERROR;
	
	/* 5. 给dmp下载固件 */
	PAL_I2C_SetByteInterval(Handle->Init.I2Cx, 100);
	
	int ret = dmp_load_motion_driver_firmware(Handle);
	
	PAL_I2C_SetByteInterval(Handle->Init.I2Cx, 0);
	
	if(ret) return ERROR;
	
	PAL_Delay(1); // 下载完成后延迟1ms
	
	/* 6. 设置dmp的方向矩阵 */
	if(dmp_set_orientation(Handle, inv_orientation_matrix_to_scalar(gyro_orientation)))
		return ERROR;
	
	/* 7. 开启dmp的特性 */
	
	// 开启以下特性：
	//     四元数、轻拍检测、朝向检测、向FIFO推送加速度计原始数据、向FIFO推送陀螺仪校准后的数据
	//     *在校准模式下关闭“陀螺仪自动校准”特性
	
	// 判断是否处在校准模式
	uint8_t isInCalibration = 1;
	
	if(Handle->Init.AccelBias.x != 0 || Handle->Init.AccelBias.y != 0 || Handle->Init.AccelBias.z != 0)
	{
		isInCalibration = 0;
	}
	
	if(Handle->Init.GyroBias.x != 0 || Handle->Init.GyroBias.y != 0 || Handle->Init.GyroBias.z != 0)
	{
		isInCalibration = 0;
	}
	
	// 开启dmp的特性
	unsigned short dmp_features = DMP_FEATURE_6X_LP_QUAT|DMP_FEATURE_TAP| 
		    DMP_FEATURE_ANDROID_ORIENT|DMP_FEATURE_SEND_RAW_ACCEL|DMP_FEATURE_SEND_CAL_GYRO;
	
	if(!isInCalibration) // 如果是非校准模式则启用陀螺仪自动校准功能
	{
		dmp_features |= DMP_FEATURE_GYRO_CAL;
	}
	
  if(dmp_enable_feature(Handle, dmp_features))
		return ERROR;
	
	
	/* 8. 设置FIFO的推送速率，默认200sps */
	if(dmp_set_fifo_rate(Handle, 200))
		return ERROR;
	
	/* 9. 向dmp推送校准参数 */
	if(!isInCalibration)
	{
		if(dmp_set_AccelBias_f(Handle, Handle->Init.AccelBias.x, 
																		Handle->Init.AccelBias.y, 
																		Handle->Init.AccelBias.z))
			return ERROR;
		
		if(dmp_set_GyroBias_f(Handle, Handle->Init.GyroBias.x, 
																	 Handle->Init.GyroBias.y, 
																	 Handle->Init.GyroBias.z))
			return ERROR;
	}
	
	/*10. 使能dmp */
	if(mpu_set_dmp_state(Handle, 1))
		return ERROR;
	
	return SUCCESS;
}

//
// @简介：  mpu6050传感器的进程函数
//          该函数需要在进程阶段的while循环中被调用
// @参数：  Handle - mpu6050的句柄
// @返回值：SUCCESS - 成功，ERROR - 失败
//
ErrorStatus PAL_MPU6050_Proc(PalMPU6050_HandleTypeDef *Handle)
{
	long quat[4]; 
	unsigned long sensor_timestamp;
	short gyro[3], accel[3], sensors;
	float q0, q1, q2, q3;
	unsigned char more = 1;
	
	/* 1. 从FIFO中读取dmp解算出的数据 */
	int ret = 0;
	
	while(more)
	{
		ret = dmp_read_fifo(Handle, gyro, accel, quat, &sensor_timestamp, &sensors, &more);
		
		if(ret == -1)
		{
			return ERROR; // 数据读取失败
		}
		else if(ret == -2)
		{
			return SUCCESS; // 没有足够的数据可以被读取
		}
	}
	
	/* 2. 对四元数进行格式转换 */
	q0 = quat[0] * q30;	//q30格式转换为浮点数
	q1 = quat[1] * q30;
	q2 = quat[2] * q30;
	q3 = quat[3] * q30;
	
	
	/* 3. 从四元数解算出欧拉角 */
	// 俯仰角 pitch
	Handle->Cache.pitch = asin(-2 * q1 * q3 + 2 * q0* q2)* deg2rad;	
	// 翻滚角 roll
	Handle->Cache.roll  = atan2(2 * q2 * q3 + 2 * q0 * q1, -2 * q1 * q1 - 2 * q2* q2 + 1)* deg2rad;	
	// 偏航角 yaw
	Handle->Cache.yaw   = atan2(2*(q1*q2 + q0*q3),q0*q0+q1*q1-q2*q2-q3*q3) * deg2rad;	
	
	
	/* 4. 读取陀螺仪和加速度计的原始数据 */
	//    因为有可能会用到这些原始数据，所以将它们读取出来
	Handle->Cache.gx = (float)gyro[0] * Handle->gyro_sense;
	Handle->Cache.gy = (float)gyro[1] * Handle->gyro_sense;
	Handle->Cache.gz = (float)gyro[2] * Handle->gyro_sense;
	
	Handle->Cache.ax = (float)accel[0] * Handle->accel_sense;
	Handle->Cache.ay = (float)accel[1] * Handle->accel_sense;
	Handle->Cache.az = (float)accel[2] * Handle->accel_sense;
	
	/* 5. 读取温度 */
	long tmp_in_q16;
	if(mpu_get_temperature(Handle, &tmp_in_q16, &sensor_timestamp))
	{
		return ERROR;
	}
	Handle->Cache.temperature = tmp_in_q16 * 0.0000152587890625; // q16 -> float
	
	return SUCCESS;
}

//
// @简介：  获取加速度传感器x轴方向的读数
// @参数：  Handle - MPU6050的句柄
// @返回值：x轴向加速度，单位g，其中g为重力加速度
//
float PAL_MPU6050_GetAccelX(PalMPU6050_HandleTypeDef *Handle)
{
	return Handle->Cache.ax;
}

//
// @简介：  获取加速度传感器y轴方向的读数
// @参数：  Handle - MPU6050的句柄
// @返回值：y轴向加速度，单位g，其中g为重力加速度
//
float PAL_MPU6050_GetAccelY(PalMPU6050_HandleTypeDef *Handle)
{
	return Handle->Cache.ay;
}

//
// @简介：  获取加速度传感器z轴方向的读数
// @参数：  Handle - MPU6050的句柄
// @返回值：z轴向加速度，单位g，其中g为重力加速度
//
float PAL_MPU6050_GetAccelZ(PalMPU6050_HandleTypeDef *Handle)
{
	return Handle->Cache.az;
}

//
// @简介：  获取陀螺仪传感器x轴方向的读数
// @参数：  Handle - MPU6050的句柄
// @返回值：x轴向角速度，单位°/s
//
float PAL_MPU6050_GetGyroX(PalMPU6050_HandleTypeDef *Handle)
{
	return Handle->Cache.gx;
}

//
// @简介：  获取陀螺仪传感器y轴方向的读数
// @参数：  Handle - MPU6050的句柄
// @返回值：y轴向角速度，单位°/s
//
float PAL_MPU6050_GetGyroY(PalMPU6050_HandleTypeDef *Handle)
{
	return Handle->Cache.gy;
}

//
// @简介：  获取陀螺仪传感器z轴方向的读数
// @参数：  Handle - MPU6050的句柄
// @返回值：z轴向角速度，单位°/s
//
float PAL_MPU6050_GetGyroZ(PalMPU6050_HandleTypeDef *Handle)
{
	return Handle->Cache.gz;
}

//
// @简介：  获取偏航角
// @参数：  Handle - MPU6050的句柄
// @返回值：偏航角，单位°，范围-180° ~ +180°
//
float PAL_MPU6050_GetYaw(PalMPU6050_HandleTypeDef *Handle)
{
	return Handle->Cache.yaw;
}

//
// @简介：  获取翻滚角
// @参数：  Handle - MPU6050的句柄
// @返回值：翻滚角，单位°，范围-180° ~ +180°
//
float PAL_MPU6050_GetRoll(PalMPU6050_HandleTypeDef *Handle)
{
	return Handle->Cache.roll;
}

//
// @简介：  获取俯仰角
// @参数：  Handle - MPU6050的句柄
// @返回值：俯仰角，单位°，范围-180° ~ +180°
//
float PAL_MPU6050_GetPitch(PalMPU6050_HandleTypeDef *Handle)
{
	return Handle->Cache.pitch;
}


//
// @简介：  获取温度
// @参数：  Handle - MPU6050的句柄
// @返回值：温度，单位℃
//
float PAL_MPU6050_GetTemperature(PalMPU6050_HandleTypeDef *Handle)
{
	return Handle->Cache.temperature;
}

static inline unsigned short inv_orientation_matrix_to_scalar(
    const signed char *mtx)
{
    unsigned short scalar;

    /*
       XYZ  010_001_000 Identity Matrix
       XZY  001_010_000
       YXZ  010_000_001
       YZX  000_010_001
       ZXY  001_000_010
       ZYX  000_001_010
     */

    scalar = inv_row_2_scale(mtx);
    scalar |= inv_row_2_scale(mtx + 3) << 3;
    scalar |= inv_row_2_scale(mtx + 6) << 6;


    return scalar;
}

/* These next two functions converts the orientation matrix (see
 * gyro_orientation) to a scalar representation for use by the DMP.
 * NOTE: These functions are borrowed from Invensense's MPL.
 */
static inline unsigned short inv_row_2_scale(const signed char *row)
{
    unsigned short b;

    if (row[0] > 0)
        b = 0;
    else if (row[0] < 0)
        b = 4;
    else if (row[1] > 0)
        b = 1;
    else if (row[1] < 0)
        b = 5;
    else if (row[2] > 0)
        b = 2;
    else if (row[2] < 0)
        b = 6;
    else
        b = 7;      // error
    return b;
}

unsigned char i2c_write(void *context, unsigned char slave_addr, unsigned char reg_addr, unsigned char length, unsigned char const *data)
{
	unsigned char ret = 0;
	PalMPU6050_HandleTypeDef *hmpu = (PalMPU6050_HandleTypeDef *)context;
	
	if(PAL_I2C_RegWriteBytes(hmpu->Init.I2Cx, slave_addr << 1, reg_addr, data, length) == SUCCESS)
	{
		ret = 0;
	}
	else
	{
		ret = 1;
	}
	
	return ret;
}

unsigned char i2c_read(void *context, unsigned char slave_addr, unsigned char reg_addr, unsigned char length, unsigned char *data)
{
	PalMPU6050_HandleTypeDef *hmpu = (PalMPU6050_HandleTypeDef *)context;
	if(PAL_I2C_RegReadBytes(hmpu->Init.I2Cx, slave_addr << 1, reg_addr, data, length) == SUCCESS)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

void delay_ms(unsigned long num_ms)
{
	PAL_Delay(num_ms);
}

void get_ms(unsigned long *count)
{
	*count = (unsigned long)PAL_GetTick();
}
