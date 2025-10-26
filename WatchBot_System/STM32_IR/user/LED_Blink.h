#ifndef _LED_BLINK_H
#define _LED_BLINK_H

#include "stm32f10x.h" 

void Led_BlinkInit(void);

extern uint8_t LED_Blink_Flag;

void Led_Blink_Proc(void);

#endif
