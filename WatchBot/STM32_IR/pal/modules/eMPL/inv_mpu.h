/*
 $License:
    Copyright (C) 2011-2012 InvenSense Corporation, All Rights Reserved.
    See included License.txt for License information.
 $
 */
/**
 *  @addtogroup  DRIVERS Sensor Driver Layer
 *  @brief       Hardware drivers to communicate with sensors via I2C.
 *
 *  @{
 *      @file       inv_mpu.h
 *      @brief      An I2C-based driver for Invensense gyroscopes.
 *      @details    This driver currently works for the following devices:
 *                  MPU6050
 *                  MPU6500
 *                  MPU9150 (or MPU6050 w/ AK8975 on the auxiliary bus)
 *                  MPU9250 (or MPU6500 w/ AK8963 on the auxiliary bus)
 */

#ifndef _INV_MPU_H_
#define _INV_MPU_H_

#define INV_X_GYRO      (0x40)
#define INV_Y_GYRO      (0x20)
#define INV_Z_GYRO      (0x10)
#define INV_XYZ_GYRO    (INV_X_GYRO | INV_Y_GYRO | INV_Z_GYRO)
#define INV_XYZ_ACCEL   (0x08)
#define INV_XYZ_COMPASS (0x01)

#define INV_CLOCK_INTERNAL  (0x00)
#define INV_CLOCK_GYROX     (0x01)
#define INV_CLOCK_GYROY     (0x02)
#define INV_CLOCK_GYROZ     (0x03)
#define INV_CLOCK_EXT_32768 (0x04)
#define INV_CLOCK_EXT_19_2  (0x05)

#define MPU_INT_STATUS_DATA_READY       (0x0001)
#define MPU_INT_STATUS_DMP              (0x0002)
#define MPU_INT_STATUS_PLL_READY        (0x0004)
#define MPU_INT_STATUS_I2C_MST          (0x0008)
#define MPU_INT_STATUS_FIFO_OVERFLOW    (0x0010)
#define MPU_INT_STATUS_ZMOT             (0x0020)
#define MPU_INT_STATUS_MOT              (0x0040)
#define MPU_INT_STATUS_FREE_FALL        (0x0080)
#define MPU_INT_STATUS_DMP_0            (0x0100)
#define MPU_INT_STATUS_DMP_1            (0x0200)
#define MPU_INT_STATUS_DMP_2            (0x0400)
#define MPU_INT_STATUS_DMP_3            (0x0800)
#define MPU_INT_STATUS_DMP_4            (0x1000)
#define MPU_INT_STATUS_DMP_5            (0x2000)

typedef struct
{
	unsigned short gyro_fsr;
	unsigned short accel_fsr;
	unsigned short sample_rate;
	unsigned short dlfp;
} mpu_init_t;

/* Set up APIs */
int mpu_init(void *context, mpu_init_t *init_param);
int mpu_set_clock_source(void *context, unsigned char source);
int mpu_init_slave(void *context);
int mpu_set_bypass(void *context, unsigned char bypass_on);

/* Configuration APIs */
int mpu_lp_accel_mode(void *context, unsigned char rate);
int mpu_lp_motion_interrupt(void *context, unsigned short thresh, unsigned char time,
    unsigned char lpa_freq);
int mpu_set_int_level(void *context, unsigned char active_low);
int mpu_set_int_latched(void *context, unsigned char enable);

int mpu_set_dmp_state(void *context, unsigned char enable);
int mpu_get_dmp_state(void *context, unsigned char *enabled);

int mpu_get_lpf(void *context, unsigned short *lpf);
int mpu_set_lpf(void *context, unsigned short lpf);

int mpu_get_gyro_fsr(void *context, unsigned short *fsr);
int mpu_set_gyro_fsr(void *context, unsigned short fsr);

int mpu_get_accel_fsr(void *context, unsigned char *fsr);
int mpu_set_accel_fsr(void *context, unsigned char fsr);

int mpu_get_compass_fsr(void *context, unsigned short *fsr);

int mpu_get_gyro_sens(void *context, float *sens);
int mpu_get_accel_sens(void *context, unsigned short *sens);

int mpu_get_sample_rate(void *context, unsigned short *rate);
int mpu_set_sample_rate(void *context, unsigned short rate);
int mpu_get_compass_sample_rate(void *context, unsigned short *rate);
int mpu_set_compass_sample_rate(void *context, unsigned short rate);

int mpu_get_fifo_config(void *context, unsigned char *sensors);
int mpu_configure_fifo(void *context, unsigned char sensors);

int mpu_get_power_state(void *context, unsigned char *power_on);
int mpu_set_sensors(void *context, unsigned char sensors);

int mpu_set_AccelBias(void *context, const long *AccelBias);

/* Data getter/setter APIs */
int mpu_get_gyro_reg(void *context, short *data, unsigned long *timestamp);
int mpu_get_accel_reg(void *context, short *data, unsigned long *timestamp);
int mpu_get_compass_reg(void *context, short *data, unsigned long *timestamp);
int mpu_get_temperature(void *context, long *data, unsigned long *timestamp);

int mpu_get_int_status(void *context, short *status);
int mpu_read_fifo(void *context, short *gyro, short *accel, unsigned long *timestamp,
    unsigned char *sensors, unsigned char *more);
int mpu_read_fifo_stream(void *context, unsigned short length, unsigned char *data,
    unsigned char *more);
int mpu_reset_fifo(void *context);

int mpu_write_mem(void *context, unsigned short mem_addr, unsigned short length,
    unsigned char *data);
int mpu_read_mem(void *context, unsigned short mem_addr, unsigned short length,
    unsigned char *data);
int mpu_load_firmware(void *context, unsigned short length, const unsigned char *firmware,
    unsigned short start_addr, unsigned short sample_rate);

int mpu_reg_dump(void *context);
int mpu_read_reg(void *context, unsigned char reg, unsigned char *data);
int mpu_run_self_test(void *context, long *gyro, long *accel);
int mpu_register_tap_cb(void *context, void (*func)(unsigned char, unsigned char));

inline unsigned char i2c_write(void *context, unsigned char slave_addr, unsigned char reg_addr, unsigned char length, unsigned char const *data);
inline unsigned char i2c_read(void *context, unsigned char slave_addr, unsigned char reg_addr, unsigned char length, unsigned char *data);
inline void delay_ms(unsigned long num_ms);
inline void get_ms(unsigned long *count);

#endif  /* #ifndef _INV_MPU_H_ */

