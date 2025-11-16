#ifndef _USART_FRAMEHANDLER_H_
#define _USART_FRAMEHANDLER_H_

#include <stdint.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _USART_FRAMEHANDLER_H
#define _USART_FRAMEHANDLER_H     


#define CMD_TABLE_SIZE (sizeof(cmd_table)/sizeof(cmd_table[0]))//指令个数
#define NO_PARAM_16  0x0000   // 占位参数，表示无参数
#define NO_PARAM_8   0x00     

typedef enum
{
	CMD_ARM_PAN_ROLL = 	 	 0x00,   //控制偏航角和翻滚角
	CMD_ARM_TILT_FAN =       0x01,   //控制俯仰角和风扇
	CMD_BUZZER_CTRL =   	 0x02,	 //控制STM板载蜂鸣器
	CMD_BUZZER_OFF  =        0x03,   //关闭STM板载蜂鸣器
}USART_TxCommand_e;

typedef enum
{	
	CMD_ESP32_LED   = 	     0x03,   //ESP板载LED
	CMD_ESPCAM_OFF_LDE =     0x04,   //关闭ESP闪光灯
	CMD_ESP32CAM_WHITE_LED = 0x05,   //闪光灯白色
	CMD_ESP32CAM_BLUE_LED =  0x06,	 //闪关灯蓝色
	CMD_ESP32CAM_RED_LED  =  0x07,	 //闪光灯红色
	CMD_ESP32CAM_ALARM_LED = 0x08,   //闪光灯红蓝爆闪
	CMD_ESP32_PICTURE      = 0x09,   //拍照
}USART_RxCommand_e;

void USART_FrameHandler_Init(void);
void arm_control(float pan, float roll, float tilt, float fan);
void BuzzerControl(uint16_t ON, uint16_t OFF);
void Buzzer_off(void);

/////////////////////////
//测试用
//void cmd_camera_flash_set(uint8_t *data);

#endif

#ifdef __cplusplus
}
#endif

#endif
