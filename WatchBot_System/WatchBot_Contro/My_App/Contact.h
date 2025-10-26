#ifndef _CONTACT_H
#define _CONTACT_H

#include "stm32f10x.h"       

#define MAX_CONTACT 10 //联系人总数
#define Contact_PageSize 3 //每页三个软件数量

typedef struct PersonInfo
{
	char name[12];
	char QQ[12];
	char WX[12];
}PersonInfo;

typedef struct SeqList
{
	uint8_t top_index; 			 // 当前页面顶部索引（0~6）
  uint8_t cursor;     		 // 当前光标所在行（0~2）
	uint8_t select_Contact;  // 选择的联系人
	PersonInfo SQDataType[MAX_CONTACT];//联系人最大数量
	uint8_t size;//有效个数
}SLT;

void Contact_Init(void);//通讯录顺序表初始化

void Contact_Proc(void);//任务进程

#endif
