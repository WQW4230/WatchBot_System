#include "USART_FrameHandler.h"
#include "stm32_link.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "app_led_control.h"
#include "esp_log.h"
#include "arm_control.h"

extern QueueHandle_t cmd_execution_handle;//串口命令执行队列
static QueueHandle_t cmd_picture_handle = NULL;//拍照命令执行队列
static SemaphoreHandle_t uart_sem = NULL; //互斥锁


static const char *TAG = "USART_FrameHandler"; 

//////////////////////////////////////////////发送模块///////////////////////////////////////////////

/*
    设置蜂鸣器
    ON  蜂鸣时间
    OFF 关闭时间    
*/
void BuzzerControl(uint16_t ON, uint16_t OFF)
{
    if(xSemaphoreTake(uart_sem, pdMS_TO_TICKS(portMAX_DELAY)) == pdTRUE)//上锁
    {
        uint8_t On_High, On_Low, Off_High, Off_Low;
        On_High = (uint8_t)(ON >> 8); // 右移 8 位，获取高字节
        On_Low  = (uint8_t)(ON & 0xFF); // 低字节取 ON 的低 8 位
            
        Off_High = (uint8_t)(OFF >> 8); // 右移 8 位，获取高字节
        Off_Low  = (uint8_t)(OFF & 0xFF); // 低字节取 OFF 的低 8 位
        uint8_t data[8] = {FRAME_HEADER, CMD_BUZZER_CTRL, DATA_MAX_LEN, On_High, On_Low, Off_High, Off_Low, FRAME_END};
        stm32_send_frame(data);

        xSemaphoreGive(uart_sem);//放锁
    }

	
}

/*
    关闭蜂鸣器
*/
void Buzzer_off(void)
{
    if(xSemaphoreTake(uart_sem, pdMS_TO_TICKS(portMAX_DELAY)) == pdTRUE)//上锁
    {
        uint8_t data[5] = {FRAME_HEADER, CMD_BUZZER_CTRL, DATA_MIN_LEN, NO_PARAM_8, FRAME_END};
        stm32_send_frame(data);

        xSemaphoreGive(uart_sem);//放锁
    }
}

/*
    控制机械臂
    pan:偏航角
    roll:翻滚角
    tilt:俯仰角
    fan:风扇转速
*/
void tx_arm_control(float pan, float roll, float tilt, float fan)
{
    if(xSemaphoreTake(uart_sem, pdMS_TO_TICKS(portMAX_DELAY)) == pdTRUE)//上锁
    {
        //传输一字节八位，不能表示负数和浮点数，+90 *100转换成无符号16位 拿高低位传输
        //传输到了再转成浮点数并且-90转换负数 公式 angle = (x / 100.0f) - 90
        uint16_t pan_u16  = (uint16_t)((pan  + 90) * 100 + 0.5);
        uint16_t roll_u16  = (uint16_t)((roll  + 90) * 100 + 0.5);
        uint16_t tilt_u16 = (uint16_t)((tilt + 90) * 100 + 0.5);
        //uint16_t fan_u16   = (uint16_t)((fan   + 90) * 100 + 0.5);

        //、pan —— roll ---tilt
        uint8_t pan_u8_high, pan_u8_low, roll_u8_high, roll_u8_low, tilt_u8_high, tilt_u8_low;
        //各个角度拿到高低位
        pan_u8_high = (uint8_t)(pan_u16 >> 8);
        pan_u8_low  = (uint8_t)(pan_u16);
        roll_u8_high = (uint8_t)(roll_u16 >> 8);
        roll_u8_low  = (uint8_t)(roll_u16);
        tilt_u8_high = (uint8_t)(tilt_u16 >> 8);
        tilt_u8_low  = (uint8_t)(tilt_u16);
        uint8_t pan_roll_data[10] = {FRAME_HEADER, CMD_ARM_CONTROL, DATA_ARM_LEN,
                                    pan_u8_high, pan_u8_low, 
                                    roll_u8_high, roll_u8_low, 
                                    tilt_u8_high, tilt_u8_low, 
                                    FRAME_END};
        stm32_send_frame(pan_roll_data);

        //第二帧 tilt —— fan
        // uint8_t fan_u8_high, fan_u8_low;
        // fan_u8_high   = (uint8_t)(fan_u16 >> 8);
        // fan_u8_low    = (uint8_t)(fan_u16);
        // uint8_t tilt_fan_data[8] = {FRAME_HEADER, CMD_ARM_TILT_FAN, DATA_MAX_LEN, tilt_u8_high, tilt_u8_low, fan_u8_high, fan_u8_low, FRAME_END};
        
        // stm32_send_frame(tilt_fan_data);      
        xSemaphoreGive(uart_sem);//放锁  
    }

}




/////////////////////////////////接收模块/////////////////////////////////////////////////////////////////////////////
// 命令映射表
typedef void (*CmdHandler_t)(void *);

typedef struct 
{
    UART_RxCommand_e cmd;
    CmdHandler_t handler;
} CmdMap_t;

extern void app_capture(void);

static void cmd_esp32_led_set(void *parameter);
static void cmd_camera_flash_set(void *parameter);
static void cmd_app_capture(void *parameter);
static void cmd_arm_control(void *parameter);

extern QueueHandle_t xQueueCaptureNotify;

CmdMap_t cmd_table[] = 
{
    {CMD_ESP32_LED, cmd_esp32_led_set},              //ESP板载LED
	{CMD_ESPCAM_OFF_LDE, cmd_camera_flash_set},      //关闭ESP闪光灯
	{CMD_ESP32CAM_WHITE_LED, cmd_camera_flash_set},  //闪光灯白色
	{CMD_ESP32CAM_BLUE_LED, cmd_camera_flash_set} ,	 //闪关灯蓝色
	{CMD_ESP32CAM_RED_LED, cmd_camera_flash_set},	 //闪光灯红色
	{CMD_ESP32CAM_ALARM_LED, cmd_camera_flash_set},  //闪光灯红蓝爆闪
	{CMD_ESP32_PICTURE, cmd_app_capture},            //拍照
    {CMD_ESP32_CONTROL_ARM, cmd_arm_control},        //切换自动模式
};

/*
    接收到控制机械臂指令
*/
static void cmd_arm_control(void *parameter)
{
    //矫正机械臂角度记录机械臂当前值
    set_arm_angle(0, 0, 0, 0);
    arm_update();
}

//接收拍照指令
static void cmd_app_capture(void *parameter)
{
    static TickType_t last_time = 0;                                // 上次拍照时间
    TickType_t now_time = xTaskGetTickCount();                      // 当前时刻
    const TickType_t interval = pdMS_TO_TICKS(PHOTO_INTERVAL_TIME); // 等待时间

    if ((now_time - last_time) < interval) 
    {
        ESP_LOGW("CAPTURE", "拍照频繁");
        return;
    }

    last_time = now_time;  // 更新最后拍照时间
    uint8_t notify = 1;
    xQueueSend(xQueueCaptureNotify, &notify, 0);
}

//接收led控制指令
static void cmd_esp32_led_set(void *parameter)
{
    uint8_t *data = (uint8_t *)parameter;
    //小端传输
    uint16_t ON = ((uint16_t)(data[3] << 8)) | data[4];
	uint16_t OFF  = ((uint16_t)(data[5] << 8)) | data[6];
    esp32_led_set(LED_MODE_BLINK, 0, ON, OFF);
}

//接收闪光灯控制指令
static void cmd_camera_flash_set(void *parameter)
{
    uint8_t *data = (uint8_t *)parameter;
    if(data[FRAME_IDX_LEN] == DATA_MIN_LEN)//一帧五字节
    {
        UART_RxCommand_e cmd = data[FRAME_IDX_CMD];
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
    else if(data[FRAME_IDX_LEN] == DATA_MAX_LEN)//一帧八字节
    {
        UART_RxCommand_e cmd = data[FRAME_IDX_CMD];
        uint16_t ON = ((uint16_t)(data[3] << 8)) | data[4];
        uint16_t OFF  = ((uint16_t)(data[5] << 8)) | data[6];

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
    while (true)
    {
        uint8_t data[DATA_MAX_LEN];
        if(xQueueReceive(cmd_execution_handle, data, portMAX_DELAY))
        {
            UART_RxCommand_e cmd = data[FRAME_IDX_CMD];//拿命令位
            for(uint8_t i=0; i<CMD_TABLE_SIZE; i++)
            {
                if(cmd == cmd_table[i].cmd)
                {
                    cmd_table[i].handler(data);
                    break;
                }
            }
        }
        
    }
    
}

//初始化串口接收
void USART_FrameHandler_Init(void)
{
    uart_sem = xSemaphoreCreateMutex();//给多个发送指令的互斥锁
    stm32_link_init();
    cmd_picture_handle = xQueueCreate(30, 5 * sizeof(uint8_t));
    if(xTaskCreatePinnedToCore(execute_frame_task, "execute_frame", 3 * 1024, NULL, 10, NULL, 1) != pdPASS)
    {
        ESP_LOGE(TAG, "UART 执行框架任务创建失败");
    }
    
}
