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

void Menu_Proc(void);

#endif
