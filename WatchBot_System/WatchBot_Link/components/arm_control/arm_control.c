#include <stdio.h>
#include "arm_control.h"
#include "USART_FrameHandler.h"


arm_control_t arm_control = {0, 0, 0, 0};

// 设置偏航角（Pan）
void set_pan_angle(float angle)
{
    if (angle > ARM_PAN_ANGLE_MAX)
    {
        angle = ARM_PAN_ANGLE_MAX;
    }
    else if (angle < ARM_PAN_ANGLE_MIN)
    {
        angle = ARM_PAN_ANGLE_MIN;
    }
    arm_control.pan_angle = angle;
}

// 设置翻滚角（Roll）
void set_roll_angle(float angle)
{
    if (angle > ARM_ROLL_ANGLE_MAX)
    {
        angle = ARM_ROLL_ANGLE_MAX;
    }
    else if (angle < ARM_ROLL_ANGLE_MIN)
    {
        angle = ARM_ROLL_ANGLE_MIN;
    }
    arm_control.roll_angle = angle;
}

// 设置俯仰角（Tilt）
void set_tilt_angle(float angle)
{
    if (angle > ARM_TILT_ANGLE_MAX)
    {
        angle = ARM_TILT_ANGLE_MAX;
    }
    else if (angle < ARM_TILT_ANGLE_MIN)
    {
        angle = ARM_TILT_ANGLE_MIN;
    }
    arm_control.tilt_angle = angle;
}
//设置风扇转速
void set_fan_speed(float speed)
{
    if (speed > ARM_FAN_SPEED_MAX)
    {
        speed = ARM_FAN_SPEED_MAX;
    }
    else if (speed < ARM_FAN_SPEED_MIN)
    {
        speed = ARM_FAN_SPEED_MIN;
    }
    arm_control.fan_speed = speed;
}

//一次设置所有角度
void set_arm_angle(float pan_angle, float roll_angle, float tilt_angle, float speed)
{
    set_pan_angle(pan_angle);
    set_roll_angle(roll_angle);
    set_tilt_angle(tilt_angle);
    set_fan_speed(speed);
}

//增加偏航角度
void offset_pan_angle(float offset)
{
    offset += arm_control.pan_angle;
    if (offset > ARM_PAN_ANGLE_MAX)
    {
        offset = ARM_PAN_ANGLE_MAX;
    }
    else if (offset < ARM_PAN_ANGLE_MIN)
    {
        offset = ARM_PAN_ANGLE_MIN;
    }
    arm_control.pan_angle = offset;
}
//增加翻滚角度
void offset_roll_angle(float offset)
{
    offset += arm_control.roll_angle;
    if (offset > ARM_ROLL_ANGLE_MAX)
    {
        offset = ARM_ROLL_ANGLE_MAX;
    }
    else if (offset < ARM_ROLL_ANGLE_MIN)
    {
        offset = ARM_ROLL_ANGLE_MIN;
    }
    arm_control.roll_angle = offset;
}
//增加俯仰角度
void offset_tilt_angle(float offset)
{
    offset += arm_control.tilt_angle;
    if (offset > ARM_TILT_ANGLE_MAX)
    {
        offset = ARM_TILT_ANGLE_MAX;
    }
    else if (offset < ARM_TILT_ANGLE_MIN)
    {
        offset = ARM_TILT_ANGLE_MIN;
    }
    arm_control.tilt_angle = offset;
}
//增加风扇转速
void offset_fan_speed(float offset)
{
    offset += arm_control.fan_speed;
    if (offset > ARM_FAN_SPEED_MAX)
    {
        offset = ARM_FAN_SPEED_MAX;
    }
    else if (offset < ARM_FAN_SPEED_MIN)
    {
        offset = ARM_FAN_SPEED_MIN;
    }
    arm_control.fan_speed = offset;
}
//一次增加机械臂所有参数角度
void offset_arm_angle(float pan_angle, float roll_angle, float tilt_angle, float speed)
{
    offset_pan_angle(pan_angle);
    offset_roll_angle(roll_angle);
    offset_tilt_angle(tilt_angle);
    offset_fan_speed(speed);
}
/*
    获得当前舵机偏航角
    参数：angle 将当前偏航角值给angle
*/
void arm_get_pan_angle(float angle)
{

}
void arm_get_roll_angle(float angle);
void arm_get_tilt_angle(float angle);
void arm_get_fan_speed(float angle);
/*
    获得当前舵机所有角度
    参数：parameter一个float类型的数组，最少4个元素
*/
void arm_get_angle(float *parameter)
{
    parameter[0]
}