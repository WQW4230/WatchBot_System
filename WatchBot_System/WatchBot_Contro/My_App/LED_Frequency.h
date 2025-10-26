#ifndef _LED_FREQUENCY_H
#define _LED_FREQUENCY_H

#include "stm32f10x.h" 

typedef enum
{
	//RGB灯颜色
	WHITE = 0, //白色
	BLUE,	 //蓝色
	RED,	 //红色

}LedColour_e;

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
	LedColour_e  cam_LedColour; //颜色
}LED_SetState_t;

//LED闪烁初始化
void Led_BlinkInit(void);

//LED菜单任务进程
void LedSet_Menu_Proc(void);

#endif
