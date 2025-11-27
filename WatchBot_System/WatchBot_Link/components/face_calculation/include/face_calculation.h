#ifndef _FACE_CALCULATION_H_
#define _FACE_CALCULATION_H_

#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif

//摄像头分辨率
#define CAMERA_WIDTH  640
#define CAMERA_HEIGHT 480 

#define FOV_X_DEG     64.0f  //水平视场
#define FOV_Y_DEG     48.0f  //垂直视场

//鼻子眼睛的权重比
#define KEYPOINT_EYES_WEIGHT       0.85f
#define KEYPOINT_MOUTH_WEIGHT     (1.0f - KEYPOINT_EYES_WEIGHT)

#define FACE_PAN_DEADZONE_ANGLE        5.0f     // 偏航角死区 单位°度
#define FACE_ROLL_DEADZONE_ANGLE       3.5f     // 翻滚角死区
#define FACE_TILT_DEADZONE_ANGLE       3.5f     // 俯仰角死区

//角度增益 当前角 * 系数
#define ARM_PAN_GAIN                  0.3f
#define ARM_ROLL_GAIN                 0.4f
#define ARM_TILT_GAIN                 0.4f

//角度翻转
#define ARM_PAN_INVERT                -1    
#define ARM_ROLL_INVERT               -1    
#define ARM_TILT_INVERT                1

#define CAMERA_CENTRE_X (CAMERA_WIDTH  / 2)  //中心坐标
#define CAMERA_CENTRE_Y (CAMERA_HEIGHT / 2)

typedef struct 
{
    int x1, y1, x2, y2;               //人脸框 x1y1为人脸框左上坐标 x2y2为人脸框右下坐标
    int left_eye_x , left_eye_y;      //左眼坐标
    int right_eye_x, right_eye_y;     //右眼坐标
    int nose_x, nose_y;               //鼻子坐标
    int left_mouth_x, left_mouth_y;   //嘴巴左侧坐标
    int right_mouth_x, right_mouth_y; //嘴巴右侧坐标
}face_info_t;

float calc_pan(face_info_t *info);
float calc_tilt(face_info_t *info);
float calc_eye_mouth_roll(face_info_t *info);

#ifdef __cplusplus
}
#endif

#endif // _APP_CAMERA_LCD_H_

