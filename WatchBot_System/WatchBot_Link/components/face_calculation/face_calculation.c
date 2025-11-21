#include <stdio.h>
#include "face_calculation.h"
#include <math.h>

//计算偏航角
//人脸框中心点位距离画面中心
float calc_pan(face_info_t *info)
{
    //人像框的中心的X轴坐标 
    int frame_centre_x = (info->x1 + info->x2) / 2;

    //水平距离中心像素距离
    int excursion_x = frame_centre_x - CAMERA_WIDTH;   // 正：偏右，负：偏左

    // 水平1°角度对应多少像素
    float level = CAMERA_WIDTH / FOV_X_DEG;

    //需要转动多少度
    float pan_angle  = excursion_x / level;

    //死区判定
    if (pan_angle > FACE_PAN_DEADZONE_ANGLE || pan_angle < -FACE_PAN_DEADZONE_ANGLE)//偏航角死区
    {
        return pan_angle;
    }
    else
    {
        return 0;
    }
}

//计算俯仰角
//人脸框中心点位距离画面中心的垂直角度
float calc_tilt(face_info_t *info)
{
    //人像框的中心坐标的y轴坐标
    int frame_centre_y = (info->y1 + info->y2) / 2;

    //垂直距离中心像素距离
    int excursion_y = frame_centre_y - CAMERA_CENTRE_Y;   // 正：偏下，负：偏上

    // 垂直1°角度对应多少像素
    float vertical = CAMERA_HEIGHT / FOV_Y_DEG;

    //需要转动多少度
    float tilt_angle = excursion_y / vertical;

    if (tilt_angle > FACE_TILT_DEADZONE_ANGLE || tilt_angle < -FACE_TILT_DEADZONE_ANGLE)//俯仰角死区
    {
        offset_tilt_angle(tilt_angle);
    }
}

//计算翻滚角度
//鼻子和眼镜的倾斜角
float calc_eye_mouth_roll(face_info_t *info)
{
    //计算眼睛向量
    float eye_vector_x = info->right_eye_x - info->left_eye_x; 
    float eye_vector_y = info->right_eye_y - info->left_eye_y; 
    //计算嘴巴向量
    float mouth_vector_x = info->right_mouth_x - info->left_mouth_x;
    float mouth_vector_y = info->right_mouth_y - info->left_mouth_y;

    //计算x轴之前的角度值
    float eye_tilt_angle   = atan2f(eye_vector_y, eye_vector_x);
    float mouth_tilt_angle = atan2f(mouth_vector_y, mouth_vector_x);

    //权重
    float eye_mouth_roll_angle = (eye_tilt_angle * KEYPOINT_EYES_WEIGHT) + (mouth_tilt_angle * KEYPOINT_MOUTH_WEIGHT);
    
    //死区校验
    if(eye_mouth_roll_angle > FACE_ROLL_DEADZONE_ANGLE || eye_mouth_roll_angle < -FACE_ROLL_DEADZONE_ANGLE)
    {
        return eye_mouth_roll_angle;
    }
    else
    {
        return 0;
    }
    
}