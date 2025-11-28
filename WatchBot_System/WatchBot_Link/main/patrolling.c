#include "patrolling.h"
#include "USART_FrameHandler.h"
#include "arm_control.h"
#include "USART_FrameHandler.h"
#include "app_led_control.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

extern QueueHandle_t xQueuePatrllin; //检测到人脸时候会通知该队列

static TaskHandle_t patrolling_task_handle = NULL; //巡逻步进角度任务句柄
static TaskHandle_t buzzer_task_handle = NULL;            //蜂鸣器发送指令任务句柄
static TaskHandle_t yes_face_watchdog_handle = NULL;      //看门狗巡逻任务句柄 由xQueuePatrllin唤醒、超时唤醒
QueueHandle_t xQueueBuzzer; //蜂鸣器队列
extern void cmd_app_capture(void *parameter); //拍照指令

//蜂鸣器任务进程
static void buzzer_task(void *param)
{
    uint8_t now_status;
    static uint8_t last_status = 0;
    while(1)
    {
        if(xQueueReceive(xQueueBuzzer, &now_status, portMAX_DELAY))
        {
            if(now_status)
            { //发现人脸开启蜂鸣器
                patrolling_off();                     //关闭并且挂起一段时间等待蜂鸣器指令发送
                vTaskDelay(pdMS_TO_TICKS(250)); 
                if(last_status == 0)
                {
                    BuzzerControl(500, 500);//蜂鸣
                    last_status = now_status;
                }
                vTaskDelay(pdMS_TO_TICKS(250)); 
            }          
            else
            {
                //无人脸关闭蜂鸣器 进入巡逻状态
                patrolling_off();
                vTaskDelay(pdMS_TO_TICKS(250));     //关闭并且挂起一段时间等待蜂鸣器指令发送
                if(last_status == 1)
                {
                    Buzzer_off();//关闭蜂鸣
                    last_status = now_status;
                }
                vTaskDelay(pdMS_TO_TICKS(250)); 
            }
                
        }
    }
}


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
            if(Buzzer_status == 0)
            {
                camera_flash_set(LED_MODE_ALARM, COLOR_WHITE, 0, 0, 0);//报警灯
                void *parameter = NULL; //一个空值填充参数
                cmd_app_capture(parameter); //拍照，拍照间隔在 USART_FrameHandler.h中设置
                Buzzer_status = 1;
                xQueueSend(xQueueBuzzer, &Buzzer_status, portMAX_DELAY);
            }
            patrolling_off();//关闭巡航
        }
        else
        {
            if(Buzzer_status == 1)
            {
                camera_flash_set(LED_MODE_PATROL, COLOR_WHITE, 0, 0, 0);//巡逻灯颜色
                Buzzer_status = 0;
                xQueueSend(xQueueBuzzer, &Buzzer_status, portMAX_DELAY);
            }
            patrolling_on();//开启巡航
        }
    }
    
} 

/*
    开启自动巡逻模式
*/
void patrolling_on(void)
{
    if(patrolling_task_handle)
    {
        vTaskResume(patrolling_task_handle);
    }
}

/*
    关闭自动巡逻模式
*/
void patrolling_off(void)
{
    if(patrolling_task_handle)
    {
        vTaskSuspend(patrolling_task_handle);
    }
    
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
    挂起所有巡逻任务进程
*/
void patrolling_off_all(void)
{
    if(yes_face_watchdog_handle)
    {
        vTaskSuspend(yes_face_watchdog_handle);
    }
    if(buzzer_task_handle)
    {
        vTaskSuspend(buzzer_task_handle);
    }
    patrolling_off();
}

/*
    开启所有巡逻任务进程
*/
void patrolling_on_all(void)
{
    if(yes_face_watchdog_handle)
    {
        vTaskResume(yes_face_watchdog_handle);
    }
    if(buzzer_task_handle)
    {
        vTaskResume(buzzer_task_handle);
    }
    
    patrolling_on();

}

/*
    巡逻模式初始化
*/
void patrolling_init(void)
{
    xQueueBuzzer = xQueueCreate(5, sizeof(uint8_t));
    xTaskCreatePinnedToCore(buzzer_task, "buzzer_task", 1 * 1024, NULL, 5, &buzzer_task_handle, 1);
    xTaskCreatePinnedToCore(arm_patrolling_task, "arm_patrolling_task", 3 * 1024, NULL, 3, &patrolling_task_handle, 1);
    xTaskCreatePinnedToCore(yes_face_watchdog, "yes_face_watchdog", 3 * 1024, NULL, 3, &yes_face_watchdog_handle, 1);
    patrolling_off_all();//默认上电关闭
}
