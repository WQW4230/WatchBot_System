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
	PersonInfo SQDataType[MAX_CONTACT];//联系人最大数量
	uint8_t size;//有效个数
}SLT;


extern uint8_t Contact_Flag;

void Contact_Proc(void);
void Contact_Init(void);//顺序表初始化

#endif
