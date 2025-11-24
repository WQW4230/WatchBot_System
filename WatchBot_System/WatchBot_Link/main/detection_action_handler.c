#include "detection_action_handler.h"
#include "arm_control.h"
#include "face_calculation.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

QueueHandle_t xQueueFaceInfo = NULL; 
static QueueHandle_t xQueueArmControl = NULL;

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
            if(xQueueArmControl)
            {
                xQueueSend(xQueueArmControl, &arm_angle, portMAX_DELAY);
            }
        }
    }
    
}

/*
    人脸姿态解算队列通知控制机械臂
*/
static void arm_control_task(void *parameter)
{
    arm_control_t arm_angle;
    while (true)
    {
        if (xQueueReceive(xQueueArmControl, &arm_angle, portMAX_DELAY))
        {
            offset_pan_angle(arm_angle.pan_angle);
            offset_roll_angle(arm_angle.roll_angle);
            offset_tilt_angle(arm_angle.tilt_angle);
            arm_update();
        }
    }
    
}

/*
    人脸姿态解算队列通知控制机械臂初始化
*/
void face_to_arm_init(void)
{
    xQueueFaceInfo   = xQueueCreate(3, sizeof(face_info_t));
    xQueueArmControl = xQueueCreate(3, sizeof(arm_control_t));

    xTaskCreatePinnedToCore(face_calc_task, "face_calc_task", 3 * 1024, NULL, 2, NULL, 1);
    xTaskCreatePinnedToCore(arm_control_task, "arm_control_task", 3 * 1024, NULL, 2, NULL, 1);

}