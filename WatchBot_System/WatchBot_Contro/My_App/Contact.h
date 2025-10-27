#ifndef _CONTACT_H
#define _CONTACT_H

#include "stm32f10x.h"       

#define CONTACH_MAX      6  //联系人总数
#define CONTACH_PAGESIZE 3   //每页三个软件数量

typedef struct PersonInfo
{
	char name[12];
	char 	 QQ[12];
	char   WX[12];
}PersonInfo;

typedef struct SeqList
{
	uint8_t top_index; 			 					 //当前页面顶部索引（0~6）
  uint8_t name_cursor;     					 //一级菜单名字在行（0~2）	
	uint8_t select_contact;  					 //一级菜单选择的联系人	
	PersonInfo SQDataType[CONTACH_MAX];//联系人最大数量
	uint8_t contach_size;							 //联系人有效个数
	
	uint8_t content_cursor;						 //二级菜单指向内容光标
	
	uint8_t modify_cursor;             //三级菜单 光标指向修改的内容
	char input_Data[12];  						 //即将存入顺序表的值
	uint8_t input_index;  						 //当前输入数组位置
	uint8_t key_last;      			 			 //上次按键
	uint8_t key_count;     			  		 //按键次数
	char key_mapping;                  //上次按下的按键

}SLT;

void Contact_Init(void);//通讯录顺序表初始化

void Contact_Proc(void);//任务进程

#endif
