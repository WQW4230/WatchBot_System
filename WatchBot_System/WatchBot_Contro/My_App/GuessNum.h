#ifndef _GUESSNUM_H
#define _GUESSNUM_H

#include "stm32f10x.h"            

#define NUM_MAX 1000

typedef struct
{
	uint32_t Guess_Answer;//答案
	uint32_t GuessNum;    //猜
	uint8_t  first_enter; //是不是第一次进入 0代表第一次。 1代表进入过
}Guess_State_t;

void GuessNum_Proc(void);

#endif
