#include "USART_FrameHandler.h"
#include "stm32_link.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "app_led_control.h"
#include "esp_log.h"

static const char *TAG = "USART_FrameHandler"; 

extern QueueHandle_t executeFrame_queue; //执行指令队列

/*
    设置蜂鸣器
    ON  蜂鸣时间
    OFF 关闭时间    
*/
void BuzzerControl(uint16_t ON, uint16_t OFF)
{
	uint8_t On_High, On_Low, Off_High, Off_Low;
    On_High = (uint8_t)(ON >> 8); // 右移 8 位，获取高字节
    On_Low  = (uint8_t)(ON & 0xFF); // 低字节取 ON 的低 8 位
        
    Off_High = (uint8_t)(OFF >> 8); // 右移 8 位，获取高字节
    Off_Low  = (uint8_t)(OFF & 0xFF); // 低字节取 OFF 的低 8 位
    uint8_t data[8] = {FRAME_HEADER, CMD_BUZZER_CTRL, DATA_MAX_LEN, On_High, On_Low, Off_High, Off_Low, FRAME_END};
    stm32_send_frame(data);
}

/*
    关闭蜂鸣器
*/
void Buzzer_off(void)
{
    uint8_t data[5] = {FRAME_HEADER, CMD_BUZZER_CTRL, DATA_MIN_LEN, NO_PARAM_8, FRAME_END};
    stm32_send_frame(data);
}

/*
    控制机械臂

*/
void arm_control(int32_t base, int32_t roll, int32_t pitch, uint32_t fan)
{
    //传输只有八位不能表示负数，传输到了再转换负数
    uint8_t base_u8  = (uint8_t)(base  + 90);
    uint8_t roll_u8  = (uint8_t)(roll  + 90);
    uint8_t pitch_u8 = (uint8_t)(pitch + 90);
    uint8_t fan_u8   = (uint8_t)(fan   + 90);

    uint8_t data[8] = {FRAME_HEADER, CMD_ARM_CONTROL, DATA_MAX_LEN, base_u8, roll_u8, pitch_u8, fan_u8, FRAME_END};
    stm32_send_frame(data);
}
//发送模块
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//接收模块
// 命令映射表
typedef void (*CmdHandler_t)(Frame_t *frame);

typedef struct 
{
    USART_RxCommand_e cmd;
    CmdHandler_t handler;
} CmdMap_t;

extern void cmd_esp32_led_set(Frame_t *frame);
extern void cmd_camera_flash_set(Frame_t *frame);

CmdMap_t cmd_table[] = 
{
    {CMD_ESP32_LED, cmd_esp32_led_set},  //ESP板载LED
	{CMD_ESPCAM_OFF_LDE, cmd_camera_flash_set},   //关闭ESP闪光灯
	{CMD_ESP32CAM_WHITE_LED, cmd_camera_flash_set},   //闪光灯白色
	{CMD_ESP32CAM_BLUE_LED, cmd_camera_flash_set} ,	 //闪关灯蓝色
	{CMD_ESP32CAM_RED_LED, cmd_camera_flash_set},	 //闪光灯红色
	{CMD_ESP32CAM_ALARM_LED, cmd_camera_flash_set},   //闪光灯红蓝爆闪
	//{CMD_ESP32_PICTURE},   //拍照
};

void cmd_esp32_led_set(Frame_t *frame)
{
    //小端传输
    uint16_t ON = ((uint16_t)(frame->ESP_Data[3] << 8)) | frame->ESP_Data[4];
	uint16_t OFF  = ((uint16_t)(frame->ESP_Data[5] << 8)) | frame->ESP_Data[6];
    esp32_led_set(LED_MODE_BLINK, 0, ON, OFF);
}

void cmd_camera_flash_set(Frame_t *frame)
{
    if(frame->Data_Len == DATA_MIN_LEN)//一帧五字节
    {
        USART_RxCommand_e cmd = frame->ESP_Data[FRAME_IDX_CMD];
        switch (cmd)
        {
        case CMD_ESPCAM_OFF_LDE:
            camera_flash_set(LED_MODE_BLACK, 0, 0, 0, 0);
            break;
        case CMD_ESP32CAM_ALARM_LED:
            camera_flash_set(LED_MODE_ALARM, COLOR_WHITE, 0, 0, 0);
            break;
        default:
            break;
        }

    }
    else if(frame->Data_Len == DATA_MAX_LEN)//一帧八字节
    {
        USART_RxCommand_e cmd = frame->ESP_Data[FRAME_IDX_CMD];
        uint16_t ON = ((uint16_t)(frame->ESP_Data[3] << 8)) | frame->ESP_Data[4];
        uint16_t OFF  = ((uint16_t)(frame->ESP_Data[5] << 8)) | frame->ESP_Data[6];

        switch (cmd)
        {
        case CMD_ESP32CAM_WHITE_LED:
            camera_flash_set(LED_MODE_BLINK, COLOR_WHITE, 0, ON, OFF);
            break;
        case CMD_ESP32CAM_BLUE_LED:
            camera_flash_set(LED_MODE_BLINK, COLOR_BLUE, 0, ON, OFF);
            break;
	    case CMD_ESP32CAM_RED_LED:
            camera_flash_set(LED_MODE_BLINK, COLOR_RED, 0, ON, OFF);
            break;
        default:
            break;
        }
    }
}

void execute_frame_task(void *pvParameters)
{
    Frame_t recv_frame;

    while (true)
    {
        if (xQueueReceive(executeFrame_queue, &recv_frame, portMAX_DELAY))
        {
            USART_RxCommand_e cmd = recv_frame.ESP_Data[FRAME_IDX_CMD];
            for(uint8_t i=0; i<CMD_TABLE_SIZE; i++)
            {
                if(cmd == cmd_table[i].cmd)
                {
                    cmd_table[i].handler(&recv_frame);
                    break;
                }
            }
        }
    }
    
}


//初始化串口接收
void USART_FrameHandler_Init(void)
{
    stm32_link_init();
    if (xTaskCreatePinnedToCore(execute_frame_task, "execute_frame", 3 * 1024, NULL, 14, NULL, 1) != pdPASS) 
    {
        ESP_LOGE(TAG, "UART 任务创建失败");
        return;
    }
    
}