#include "USART_queue.h"

//队列初始化
void Queue_Init(Queue_HandleTypeDef *hQueue)
{
	hQueue->Tail = 0;
}

//元素入队
//队列句柄
//需要入队的元素
void Queue_Enqueue(Queue_HandleTypeDef *hQueue, uint8_t Element)
{
	hQueue->Data[hQueue->Tail ++] = Element;
}

//元素出队 调用一次出队一次
ErrorStatus Queue_Dequeue(Queue_HandleTypeDef *hQueue, uint8_t *pElement)
{
	uint32_t i;
	if(hQueue->Tail == 0) return ERROR; // 队列空，操作无效
	*pElement = hQueue->Data[0];
	for(i=0;i<hQueue->Tail-1;i++)
	{
		hQueue->Data[i] = hQueue->Data[i+1];
	}
	hQueue->Tail--;
	return SUCCESS;
}
