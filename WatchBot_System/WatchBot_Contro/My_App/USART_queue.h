#ifndef _USART_QUEUE_H
#define _USART_QUEUE_H

#include "stm32f10x.h" 

typedef struct 
{
	uint8_t Data[100];
	uint16_t Tail; // 队尾
} Queue_HandleTypeDef;

void Queue_Init(Queue_HandleTypeDef *hQueue);
void Queue_Enqueue(Queue_HandleTypeDef *hQueue, uint8_t Element);
ErrorStatus Queue_Dequeue(Queue_HandleTypeDef *hQueue, uint8_t *pElement);

#endif
