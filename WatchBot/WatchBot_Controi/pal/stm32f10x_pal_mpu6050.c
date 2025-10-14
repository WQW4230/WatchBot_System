/**
  ******************************************************************************
  * @file    stm32f10x_pal_mpu6050.c
  * @author  铁头山羊stm32工作组
  * @version V1.0.0
  * @date    2023年2月24日
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

#define MPU6050_ADDR_AD0_LOW  0xd0
#define MPU6050_ADDR_AD0_HIGH 0xd2
#define MPU6050_ADDR_DEFAULT  MPU6050_ADDR_AD0_LOW


/* mpu6050寄存器地址列表 */
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

#define MPU6050_WRITE_DELAY 30

#define MPU6050_SensorWrite(MPU, MemAddr, pBuf, Size) do{\
	MPU->Init.hI2c->Init.Advanced.FrameInterval = MPU6050_WRITE_DELAY;\
	PAL_I2C_MemWrite((MPU)->Init.hI2c, MPU6050_ADDR_DEFAULT, (MemAddr), I2C_MEMSIZE_8BIT, (pBuf), (Size));\
	MPU->Init.hI2c->Init.Advanced.FrameInterval = 0;\
} while(0)

#define MPU6050_SensorRead(MPU, MemAddr, pBuf, Size) PAL_I2C_MemRead((MPU)->Init.hI2c, MPU6050_ADDR_DEFAULT, (MemAddr), I2C_MEMSIZE_8BIT, (pBuf), (Size))

static signed char gyro_orientation[9] = {1, 0, 0,
                                          0, 1, 0,
                                          0, 0, 1};

static inline unsigned short inv_orientation_matrix_to_scalar(const signed char *mtx);
static inline unsigned short inv_row_2_scale(const signed char *row);
																					
static void PAL_MPU6050_SetSampleRate(PalMPU6050_HandleTypeDef *Handle, uint16_t SampleRate, uint16_t FilterType);
static void PAL_MPU6050_SetGyroScaleRange(PalMPU6050_HandleTypeDef *Handle, uint16_t Range);
static void PAL_MPU6050_SetAccelScaleRange(PalMPU6050_HandleTypeDef *Handle, uint16_t Range);
																					 
void PAL_MPU6050_Init(PalMPU6050_HandleTypeDef *Handle)
{
	mpu_init_t mpu_init_param;
	
	mpu_init_param.sample_rate = Handle->Init.SampleRate;
	
	switch(Handle->Init.MPU6050_AccelFsr)
	{
		case MPU6050_ACCELFSR_2g:
			mpu_init_param.accel_fsr = 2;
			break;
		case MPU6050_ACCELFSR_4g:
			mpu_init_param.accel_fsr = 4;
			break;
		case MPU6050_ACCELFSR_8g:
			mpu_init_param.accel_fsr = 8;
			break;
		case MPU6050_ACCELFSR_16g:
			mpu_init_param.accel_fsr = 16;
			break;
	}
	
	switch(Handle->Init.MPU6050_GyroFsr)
	{
		case MPU6050_GYROFSR_250:
			mpu_init_param.gyro_fsr = 250;
			break;
		case MPU6050_GYROFSR_500:
			mpu_init_param.gyro_fsr = 500;
			break;
		case MPU6050_GYROFSR_1000:
			mpu_init_param.gyro_fsr = 1000;
			break;
		case MPU6050_GYROFSR_2000:
			mpu_init_param.gyro_fsr = 2000;
			break;
	}
	
	mpu_init(Handle, &mpu_init_param);
	
	uint16_t dlfpType = MPU6050_DLFPTYPE_260Hz_256Hz;
	
	if(Handle->Init.SampleRate > 188)
	{
		dlfpType = MPU6050_DLFPTYPE_260Hz_256Hz;
	}
	else if(Handle->Init.SampleRate > 98)
	{
		dlfpType = MPU6050_DLFPTYPE_184Hz_188Hz;
	}
	else if(Handle->Init.SampleRate > 44)
	{
		dlfpType = MPU6050_DLFPTYPE_94Hz_98Hz;
	}
	else if(Handle->Init.SampleRate > 21)
	{
		dlfpType = MPU6050_DLFPTYPE_44Hz_42Hz;
	}
	else if(Handle->Init.SampleRate > 10)
	{
		dlfpType = MPU6050_DLFPTYPE_21Hz_20Hz;
	}
	else if(Handle->Init.SampleRate > 5)
	{
		dlfpType = MPU6050_DLFPTYPE_10Hz_10Hz;
	}
	else if(Handle->Init.SampleRate > 5)
	{
		dlfpType = MPU6050_DLFPTYPE_10Hz_10Hz;
	}
	else
	{
		dlfpType = MPU6050_DLFPTYPE_5Hz_5Hz;
	}
	
	/* Set sample rate and digital low pass filter type */
	PAL_MPU6050_SetSampleRate(Handle, Handle->Init.SampleRate, dlfpType);
	
	PAL_MPU6050_SetAccelScaleRange(Handle, Handle->Init.MPU6050_AccelFsr);
	PAL_MPU6050_SetGyroScaleRange(Handle, Handle->Init.MPU6050_GyroFsr);
	
	/* Wake up all sensors. */
	mpu_set_sensors(Handle, INV_XYZ_GYRO | INV_XYZ_ACCEL);
	/* Push both gyro and accel data into the FIFO. */
	mpu_configure_fifo(Handle, INV_XYZ_GYRO | INV_XYZ_ACCEL); 
	
	dmp_load_motion_driver_firmware(Handle);
	dmp_set_orientation(Handle, inv_orientation_matrix_to_scalar(gyro_orientation));
  dmp_enable_feature(Handle, DMP_FEATURE_6X_LP_QUAT|DMP_FEATURE_TAP|	//设置dmp功能
		    DMP_FEATURE_ANDROID_ORIENT|DMP_FEATURE_SEND_RAW_ACCEL|DMP_FEATURE_SEND_CAL_GYRO|
		    DMP_FEATURE_GYRO_CAL);
	dmp_set_fifo_rate(Handle, Handle->Init.SampleRate);
	mpu_set_dmp_state(Handle, 1);
	
	Handle->hexti.Init.GPIOx = Handle->Init.Int_GPIOPort;
	Handle->hexti.Init.GPIO_Pin = Handle->Init.Int_GPIOPin;
	Handle->hexti.Init.EXTI_Mode = EXTI_Mode_Interrupt;
	Handle->hexti.Init.EXTI_Trigger = EXTI_Trigger_Rising;
	Handle->hexti.Init.PreemptionPriority = Handle->Init.Int_PremptionPriority;
	Handle->hexti.Init.SubPriority = Handle->Init.Int_SubPriority;
	
	PAL_EXTI_Init(&Handle->hexti);
}

void PAL_MPU6050_IntrruptCmd(PalMPU6050_HandleTypeDef *Handle, FunctionalState NewState)
{
	PAL_EXTI_InterruptCmd(&Handle->hexti, NewState);
}

void PAL_MPU6050_Int_IRQHandler(PalMPU6050_HandleTypeDef *Handle)
{
	PAL_EXTI_IRQHandler(&Handle->hexti);
	Handle->Init.Int_Callback();
}

#define q30  1073741824.0f

void PAL_MPU6050_Update(PalMPU6050_HandleTypeDef *Handle)
{
	long quat[4]; 
	unsigned char more;
	unsigned long sensor_timestamp;
	short gyro[3], accel[3], sensors;
	float q0, q1, q2, q3;
	
	do
	{
		dmp_read_fifo(Handle, gyro, accel, quat, &sensor_timestamp, &sensors, &more);
	}
	while(more);
	
	q0 = quat[0] / q30;	//q30格式转换为浮点数
	q1 = quat[1] / q30;
	q2 = quat[2] / q30;
	q3 = quat[3] / q30;
	
	Handle->Result.Pitch = asin(-2 * q1 * q3 + 2 * q0* q2)* 57.3;	// pitch
	Handle->Result.Roll  = atan2(2 * q2 * q3 + 2 * q0 * q1, -2 * q1 * q1 - 2 * q2* q2 + 1)* 57.3;	// roll
	Handle->Result.Yaw   = atan2(2*(q1*q2 + q0*q3),q0*q0+q1*q1-q2*q2-q3*q3) * 57.3;	//yaw
	
	Handle->Result.Quat_w = q0;
	Handle->Result.Quat_x = q1;
	Handle->Result.Quat_y = q2;
	Handle->Result.Quat_z = q3;
	
	float gyro_sense;
	unsigned short accel_sense;
	
	mpu_get_gyro_sens(Handle, &gyro_sense);
	Handle->Result.Gyro_x = (float)gyro[0] / gyro_sense;
	Handle->Result.Gyro_y = (float)gyro[1] / gyro_sense;
	Handle->Result.Gyro_z = (float)gyro[2] / gyro_sense;
	
	mpu_get_accel_sens(Handle, &accel_sense);
	Handle->Result.Accel_x = (float)accel[0] / accel_sense;
	Handle->Result.Accel_y = (float)accel[1] / accel_sense;
	Handle->Result.Accel_z = (float)accel[2] / accel_sense;
	
	uint8_t buffer[2], tmp;
	
	MPU6050_SensorRead(Handle, MPU6050_REG_GYRO_YOUT_H, buffer, 2);
	
	tmp = buffer[0];
	buffer[0] = buffer[1];
	buffer[1] = tmp;
}

float PAL_MPU6050_GetYaw(PalMPU6050_HandleTypeDef *Handle)
{
	return Handle->Result.Yaw;
}

float PAL_MPU6050_GetRoll(PalMPU6050_HandleTypeDef *Handle)
{
	return Handle->Result.Roll;
}
float PAL_MPU6050_GetPitch(PalMPU6050_HandleTypeDef *Handle)
{
	return Handle->Result.Pitch;
}
float PAL_MPU6050_GetGyroX(PalMPU6050_HandleTypeDef *Handle)
{
	return Handle->Result.Gyro_x;
}
float PAL_MPU6050_GetGyroY(PalMPU6050_HandleTypeDef *Handle)
{
	return Handle->Result.Gyro_y;
}
float PAL_MPU6050_GetGyroZ(PalMPU6050_HandleTypeDef *Handle)
{
	return Handle->Result.Gyro_z;
}
float PAL_MPU6050_GetAccelX(PalMPU6050_HandleTypeDef *Handle)
{
	return Handle->Result.Accel_x;
}
float PAL_MPU6050_GetAccelY(PalMPU6050_HandleTypeDef *Handle)
{
	return Handle->Result.Accel_y;
}
float PAL_MPU6050_GetAccelZ(PalMPU6050_HandleTypeDef *Handle)
{
	return Handle->Result.Accel_z;
}
void PAL_MPU6050_GetAllResult(PalMPU6050_HandleTypeDef *Handle, PalMPU6050Result_TypeDef *pResult)
{
	memcpy(pResult, &Handle->Result, sizeof(PalMPU6050Result_TypeDef));
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
	
	hmpu->Init.hI2c->Init.Advanced.FrameInterval = MPU6050_WRITE_DELAY;
	if(PAL_I2C_MemWrite(hmpu->Init.hI2c, slave_addr << 1, reg_addr, I2C_MEMSIZE_8BIT, data, length) == SUCCESS)
	{
		ret = 0;
	}
	else
	{
		ret = 1;
	}
	hmpu->Init.hI2c->Init.Advanced.FrameInterval = 0;
	return ret;
}

unsigned char i2c_read(void *context, unsigned char slave_addr, unsigned char reg_addr, unsigned char length, unsigned char *data)
{
	PalMPU6050_HandleTypeDef *hmpu = (PalMPU6050_HandleTypeDef *)context;
	if(PAL_I2C_MemRead(hmpu->Init.hI2c, slave_addr << 1, reg_addr, I2C_MEMSIZE_8BIT, data, length) == SUCCESS)
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

static void PAL_MPU6050_SetSampleRate(PalMPU6050_HandleTypeDef *Handle, uint16_t SampleRate, uint16_t FilterType)
{
	uint16_t fs;
	uint8_t tmp[1];
	if(FilterType == MPU6050_DLFPTYPE_No || FilterType == MPU6050_DLFPTYPE_260Hz_256Hz)
	{
		fs = 8000;
	}
	else
	{
		fs = 1000;
	}
	tmp[0] = fs / SampleRate - 1;
	
	MPU6050_SensorWrite(Handle, MPU6050_REG_SMPRT_DIV, tmp, 1);
	
	tmp[0] = (uint8_t)(FilterType & 0x07);
	
	MPU6050_SensorWrite(Handle, MPU6050_REG_CONFIG, tmp, 1);
}

static void PAL_MPU6050_SetGyroScaleRange(PalMPU6050_HandleTypeDef *Handle, uint16_t Range)
{
	uint8_t tmp = (uint8_t)((Range << 3) & 0x18);
	MPU6050_SensorWrite(Handle, MPU6050_REG_GYRO_CONFIG, &tmp, 1);
}

static void PAL_MPU6050_SetAccelScaleRange(PalMPU6050_HandleTypeDef *Handle, uint16_t Range)
{
	uint8_t tmp = (uint8_t)((Range << 3) & 0x18);
	MPU6050_SensorWrite(Handle, MPU6050_REG_ACCEL_CONFIG, &tmp, 1);
}
