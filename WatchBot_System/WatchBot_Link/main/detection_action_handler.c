#include "detection_action_handler.h"
#include "arm_control.h"
#include "face_calculation.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

QueueHandle_t xQueueFaceInfo = NULL; 
static TaskHandle_t face_calc_task_handle = NULL;  //人脸姿态解算任务句柄

uint8_t face_calc_task_flag = 0; //人脸姿态解算任务标志位

/*
    获取人脸解算后的姿态
*/
static void face_calc_task(void *parameter)
{
    face_info_t info;
    arm_control_t arm_angle;
    while (true)
    {
        if (xQueueReceive(xQueueFaceInfo, &info, portMAX_DELAY))
        {
            arm_angle.pan_angle  = calc_pan(&info);
            arm_angle.roll_angle = calc_eye_mouth_roll(&info);
            arm_angle.tilt_angle = calc_tilt(&info);
            offset_pan_angle(arm_angle.pan_angle);
            offset_roll_angle(arm_angle.roll_angle);
            offset_tilt_angle(arm_angle.tilt_angle);
            arm_update();
        }
    }
    
}

/*
    人脸姿态解算任务开启
*/
void face_to_arm_on(void)
{
    if(face_calc_task_handle)
    {
        face_calc_task_flag = 1;
        vTaskResume(face_calc_task_handle);
    }
}

/*
    人脸姿态解算任务关闭
*/
void face_to_arm_off(void)
{
    if(face_calc_task_handle)
    {
        face_calc_task_flag = 0;
        vTaskSuspend(face_calc_task_handle);
    }
}

/*
    人脸姿态解算队列通知控制机械臂初始化
*/
void face_to_arm_init(void)
{
    xQueueFaceInfo   = xQueueCreate(3, sizeof(face_info_t));

    xTaskCreatePinnedToCore(face_calc_task, "face_calc_task", 3 * 1024, NULL, 2, &face_calc_task_handle, 1);
    face_to_arm_off();//默认挂起任务
}