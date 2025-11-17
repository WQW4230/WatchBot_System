#ifndef __LED_H
#define __LED_H

#include "stm32f10x.h"

#define       LED0_ON   GPIO_SetBits(GPIOC, GPIO_Pin_13)
#define       LED0_OFF  GPIO_ResetBits(GPIOC, GPIO_Pin_13)
#define       LED1_ON   GPIO_SetBits(GPIOC, GPIO_Pin_0)
#define       LED1_OFF  GPIO_ResetBits(GPIOC, GPIO_Pin_0)
#define       LED2_ON   GPIO_SetBits(GPIOC, GPIO_Pin_1)
#define       LED2_OFF  GPIO_ResetBits(GPIOC, GPIO_Pin_1)
#define       LED3_ON   GPIO_SetBits(GPIOC, GPIO_Pin_2)
#define       LED3_OFF  GPIO_ResetBits(GPIOC, GPIO_Pin_2)


void LED_GPIO_Config(void);	

#endif
