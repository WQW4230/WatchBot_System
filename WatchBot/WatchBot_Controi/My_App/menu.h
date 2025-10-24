#ifndef _MENU_H
#define _MENU_H

#include "stm32f10x.h" 

#define APP_COUNT ((sizeof(app_items)) / sizeof(app_items[0]))   //软件总数
#define APP_PAGE_SIZE 3 //每页显示几个app 不可更改

typedef struct
{
	char *app_name;  	 //软件的名字
	void (*fun)(void); //任务进程函数指针
}menu_items;

typedef struct 
{
  uint8_t top_index;  // 当前页面顶部索引（0~6）
  uint8_t cursor;     // 当前光标所在行（0~2）
	uint8_t select_app;  // 选择的app
} Menu_t;


#define APP_PageSize 3 //每页三个软件数量
#define APP_Top  ((sizeof(menu_items)) / sizeof(menu_items[0]))   //软件总数

extern uint8_t Menu_Flag;// 主页标志位
void Menu_Proc(void);

uint32_t GetRandom(uint32_t Random_Max);//获得一个伪随机数值为0~Max-1




#endif
