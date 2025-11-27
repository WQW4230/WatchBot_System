#ifndef _USART_FRAMEHANDLER_H_
#define _USART_FRAMEHANDLER_H_

#include <stdint.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _USART_FRAMEHANDLER_H
#define _USART_FRAMEHANDLER_H     

//拍照时间间隔 不能太短否则队列满重启
#define PHOTO_INTERVAL_TIME 7000

#define CMD_TABLE_SIZE (sizeof(cmd_table)/sizeof(cmd_table[0]))//指令个数
#define NO_PARAM_16  0x0000   // 占位参数，表示无参数
#define NO_PARAM_8   0x00     

//发送指令
typedef enum
{
	CMD_ARM_CONTROL = 	 	 0x10,   //控制偏航角、翻滚角和俯仰角
	CMD_BUZZER_CTRL =   	 0x12,	 //控制STM板载蜂鸣器
	CMD_BUZZER_OFF  =        0x13,   //关闭蜂鸣器
}UART_TxCommand_e;

//接收指令
typedef enum 
{	
	CMD_ESP32_ON_CONTROL  =  0x01,   //接收到取消接管指令
	CMD_ESP32_CONTROL_ARM =  0x02,   //接收到发送接管控制指令
	CMD_ESP32_LED   = 	   	 0x03,   //ESP板载LED
	CMD_ESPCAM_OFF_LDE =     0x04,   //关闭ESP闪光灯
	CMD_ESP32CAM_WHITE_LED = 0x05,   //闪光灯白色
	CMD_ESP32CAM_BLUE_LED =  0x06,	 //闪关灯蓝色
	CMD_ESP32CAM_RED_LED  =  0x07,	 //闪光灯红色
	CMD_ESP32CAM_ALARM_LED = 0x08,   //闪光灯红蓝爆闪 警报模式
	CMD_ESP32_PICTURE      = 0x09,   //拍照
	CMD_ESP32_PATROL       = 0x0A,   //闪光灯红蓝交替 巡逻模式
}UART_RxCommand_e;

void USART_FrameHandler_Init(void);
void tx_arm_control(float pan, float roll, float tilt, float fan);
void BuzzerControl(uint16_t ON, uint16_t OFF);
void Buzzer_off(void);
void cmd_app_capture(void *parameter);
/////////////////////////
//测试用
//void cmd_camera_flash_set(uint8_t *data);

#endif

#ifdef __cplusplus
}
#endif

#endif
