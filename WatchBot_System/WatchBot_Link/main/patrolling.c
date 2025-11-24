#include "patrolling.h"
#include "USART_FrameHandler.h"
#include "arm_control.h"
#include "USART_FrameHandler.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

extern QueueHandle_t xQueuePatrllin;
static TaskHandle_t patrolling_task_handle = NULL;

/*
    有人脸时队列通知唤醒 巡逻角度控制任务进程
    超时则进入巡逻模式
*/
static void yes_face_watchdog(void *parameter)
{
    static uint8_t Buzzer_status = 0; //当前蜂鸣器状态为0是关闭 1打开
    while (true)
    {
        uint8_t yes_face_flag;
        
        if (xQueueReceive(xQueuePatrllin, &yes_face_flag, pdMS_TO_TICKS(3000)))
        {
            patrolling_off();//关闭巡航
            if(Buzzer_status == 0)
            {
                BuzzerControl(500, 500);//开启蜂鸣器
                Buzzer_status = 1;
            }

        }
        else
        {
            patrolling_on();//开启巡航
            if(Buzzer_status == 1)
            {
                Buzzer_off();//关闭蜂鸣器
                Buzzer_status = 0;
            }
        }

    }
    
} 

/*
    开启自动巡逻模式
*/
void patrolling_on(void)
{
    vTaskResume(patrolling_task_handle);
}

/*
    关闭自动巡逻模式
*/
void patrolling_off(void)
{
    vTaskSuspend(patrolling_task_handle);
}

/*
    巡逻角度控制任务进程
*/
static void arm_patrolling_task(void *parameter)
{
    static action_sequence_e  pan_status = positive;
    static action_sequence_e roll_status = positive;
    static action_sequence_e tilt_status = positive;
    static float  pan_angle = 0;
    static float roll_angle = 0;
    static float tilt_angle = 0;

    while (true)
    {
        //偏航角巡航
        pan_angle += (PATROL_STEP_PAN * pan_status);
        if(pan_status == positive && pan_angle >= PATROL_PAN_MAX)
        {
            pan_angle  = PATROL_PAN_MAX;
            pan_status = negative;
        }
        else if(pan_status == negative && pan_angle <= PATROL_PAN_MIN)
        {
            pan_angle  = PATROL_PAN_MIN;
            pan_status = positive;
        }
        //翻滚角巡航
        roll_angle += (PATROL_STEP_ROLL * roll_status);
        if(roll_status == positive && roll_angle >= PATROL_ROLL_MAX)
        {
            roll_angle  = PATROL_ROLL_MAX;
            roll_status = negative;
        }
        else if(roll_status == negative && roll_angle <= PATROL_ROLL_MIN)
        {
            roll_angle  = PATROL_ROLL_MIN;
            roll_status = positive;
        }
        //俯仰角巡航
        tilt_angle += (PATROL_STEP_TILT * tilt_status);
        if(tilt_status == positive && tilt_angle >= PATROL_TILT_MAX)
        {
            tilt_angle  = PATROL_TILT_MAX;
            tilt_status = negative;
        }
        else if(tilt_status == negative && tilt_angle <= PATROL_TILT_MIN)
        {
            tilt_angle  = PATROL_TILT_MIN;
            tilt_status = positive;
        }
        set_arm_angle(pan_angle, roll_angle, tilt_angle, 0);
        arm_update();
        vTaskDelay(pdMS_TO_TICKS(PATROL_STEP_TIME));
    }
}


/*
    巡逻模式初始化
*/
void patrolling_init(void)
{
    xTaskCreatePinnedToCore(arm_patrolling_task, "arm_patrolling_task", 1024, NULL, 3, &patrolling_task_handle, 1);
    xTaskCreatePinnedToCore(yes_face_watchdog, "no_face_watchdog", 1024, NULL, 3, NULL, 1);
}
