#ifndef _PS2_ADC_DMA_H
#define	_PS2_ADC_DMA_H

#include "stm32f10x.h"

//PS2摇杆驱动
//需要配置引脚口，ADC-DMA
//PS2_AD的四个值分别为两个摇杆的XY轴
//GPIOA	  3 4 5 6 引脚
//对应数组		0 1 2 3 

//ADC_DMA初始化
void PS2_ADCInit(void);

extern uint16_t PS2_AD[4];

#endif
