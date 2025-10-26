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
 *      @file       inv_mpu_dmp_motion_driver.h
 *      @brief      DMP image and interface functions.
 *      @details    All functions are preceded by the dmp_ prefix to
 *                  differentiate among MPL and general driver function calls.
 */
#ifndef _INV_MPU_DMP_MOTION_DRIVER_H_
#define _INV_MPU_DMP_MOTION_DRIVER_H_

#define TAP_X               (0x01)
#define TAP_Y               (0x02)
#define TAP_Z               (0x04)
#define TAP_XYZ             (0x07)

#define TAP_X_UP            (0x01)
#define TAP_X_DOWN          (0x02)
#define TAP_Y_UP            (0x03)
#define TAP_Y_DOWN          (0x04)
#define TAP_Z_UP            (0x05)
#define TAP_Z_DOWN          (0x06)

#define ANDROID_ORIENT_PORTRAIT             (0x00)
#define ANDROID_ORIENT_LANDSCAPE            (0x01)
#define ANDROID_ORIENT_REVERSE_PORTRAIT     (0x02)
#define ANDROID_ORIENT_REVERSE_LANDSCAPE    (0x03)

#define DMP_INT_GESTURE     (0x01)
#define DMP_INT_CONTINUOUS  (0x02)

#define DMP_FEATURE_TAP             (0x001)
#define DMP_FEATURE_ANDROID_ORIENT  (0x002)
#define DMP_FEATURE_LP_QUAT         (0x004)
#define DMP_FEATURE_PEDOMETER       (0x008)
#define DMP_FEATURE_6X_LP_QUAT      (0x010)
#define DMP_FEATURE_GYRO_CAL        (0x020)
#define DMP_FEATURE_SEND_RAW_ACCEL  (0x040)
#define DMP_FEATURE_SEND_RAW_GYRO   (0x080)
#define DMP_FEATURE_SEND_CAL_GYRO   (0x100)

#define INV_WXYZ_QUAT       (0x100)

/* Set up functions. */
int dmp_load_motion_driver_firmware(void *context);
int dmp_set_fifo_rate(void *context, unsigned short rate);
int dmp_get_fifo_rate(void *context, unsigned short *rate);
int dmp_enable_feature(void *context, unsigned short mask);
int dmp_get_enabled_features(void *context, unsigned short *mask);
int dmp_set_interrupt_mode(void *context, unsigned char mode);
int dmp_set_orientation(void *context, unsigned short orient);
int dmp_set_GyroBias(void *context, long *bias);
int dmp_set_GyroBias_f(void *context, float x, float y, float z);
int dmp_set_AccelBias(void *context, long *bias);
int dmp_set_AccelBias_f(void *context, float x, float y, float z);

/* Tap functions. */
int dmp_register_tap_cb(void (*func)(unsigned char, unsigned char));
int dmp_set_tap_thresh(void *context, unsigned char axis, unsigned short thresh);
int dmp_set_tap_axes(void *context, unsigned char axis);
int dmp_set_tap_count(void *context, unsigned char min_taps);
int dmp_set_tap_time(void *context, unsigned short time);
int dmp_set_tap_time_multi(void *context, unsigned short time);
int dmp_set_shake_reject_thresh(void *context, long sf, unsigned short thresh);
int dmp_set_shake_reject_time(void *context, unsigned short time);
int dmp_set_shake_reject_timeout(void *context, unsigned short time);

/* Android orientation functions. */
int dmp_register_android_orient_cb(void (*func)(unsigned char));

/* LP quaternion functions. */
int dmp_enable_lp_quat(void *context, unsigned char enable);
int dmp_enable_6x_lp_quat(void *context, unsigned char enable);

/* Pedometer functions. */
int dmp_get_pedometer_step_count(void *context, unsigned long *count);
int dmp_set_pedometer_step_count(void *context, unsigned long count);
int dmp_get_pedometer_walk_time(void *context, unsigned long *time);
int dmp_set_pedometer_walk_time(void *context, unsigned long time);

/* DMP gyro calibration functions. */
int dmp_enable_gyro_cal(void *context, unsigned char enable);

/* Read function. This function should be called whenever the MPU interrupt is
 * detected.
 */
int dmp_read_fifo(void *context, short *gyro, short *accel, long *quat,
    unsigned long *timestamp, short *sensors, unsigned char *more);

#endif  /* #ifndef _INV_MPU_DMP_MOTION_DRIVER_H_ */

