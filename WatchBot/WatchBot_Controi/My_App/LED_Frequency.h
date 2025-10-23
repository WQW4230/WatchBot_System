#ifndef _LED_FREQUENCY_H
#define _LED_FREQUENCY_H

#include "stm32f10x.h" 

typedef struct
{
	uint8_t current;							//当前指向的索引
	uint8_t up;										//向上翻索引号
	uint8_t down;									//向下翻索引号
	uint8_t enter;								//确认索引号
	void (*current_operation)(void);  //任务函数
} KeyOperation_t;

typedef struct
{
	uint16_t index; //当前位置索引
	
	//STM32板载单色LED
	uint16_t stm_LedDark_time;
	uint16_t stm_LedBrigh_time;
	
	//ESP32板载单色LED
	uint16_t esp_LedDark_time;
	uint16_t esp_LedBrigh_time;
	
	//ESP32摄像头照明灯
	uint16_t cam_LedBrigh_time;	//亮时间
	uint16_t cam_LedDark_time;	//暗时间
	uint8_t  cam_LedColour;   	//颜色
}LED_SetState_t;


void Led_BlinkInit(void);

void LED_Proc_test(void);

void LedSet_Proc(void);

#endif
