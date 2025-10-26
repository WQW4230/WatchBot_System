#ifndef _MENU_H
#define _MENU_H

#include "stm32f10x.h" 



#define APP_PageSize 3 //每页三个软件数量
#define APP_Top  ((sizeof(menu_items)) / sizeof(menu_items[0]))   //软件总数

extern uint8_t IR_Flag;//红外状态标志位
extern uint8_t Menu_Flag;// 主页标志位
void Menu_Proc(void);

uint32_t GetRandom(uint32_t Random_Max);//获得一个伪随机数值为0~Max-1




#endif
