#ifndef _ARM_CONTROL_H_
#define _ARM_CONTROL_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ARM_PAN_ANGLE_MIN   (-90.0f)
#define ARM_PAN_ANGLE_MAX   ( 90.0f)

#define ARM_ROLL_ANGLE_MIN  (-90.0f)
#define ARM_ROLL_ANGLE_MAX  ( 90.0f)

#define ARM_TILT_ANGLE_MIN  (-90.0f)
#define ARM_TILT_ANGLE_MAX  ( 90.0f)

#define ARM_FAN_SPEED_MIN  (-100.0f)
#define ARM_FAN_SPEED_MAX  ( 100.0f)

typedef struct 
{
    float pan_angle;
    float roll_angle;
    float tilt_angle;
    float fan_speed;
}arm_control_t;

void set_pan_angle(float angele);
void set_roll_angle(float angle);
void set_tilt_angle(float angle);
void set_fan_speed(float speed);
void set_arm_angle(float pan_angle, float roll_angle, float tilt_angle, float speed);

void offset_pan_angle(float offset);
void offset_roll_angle(float offset);
void offset_tilt_angle(float offset);
void offset_fan_speed(float offset);
void offset_arm_angle(float pan_angle, float roll_angle, float tilt_angle, float speed);

void arm_get_pan_angle(float angle);
void arm_get_roll_angle(float angle);
void arm_get_tilt_angle(float angle);
void arm_get_fan_speed(float angle);
void arm_get_angle(float *parameter);

#ifdef __cplusplus
}
#endif

#endif