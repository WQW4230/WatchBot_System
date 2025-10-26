/**
  ******************************************************************************
  * @file    pal_object_queue.h
  * @author  铁头山羊stm32工作组
  * @version V1.0.0
  * @date    2023年1月7日
  * @brief   环形缓冲队列
  ******************************************************************************
  * @attention
  * Copyright (c) 2022 - 2023 东莞市明玉科技有限公司. 保留所有权力.
  ******************************************************************************
	*/

#ifndef __PAL_OBJECT_QUEUE_H_
#define __PAL_OBJECT_QUEUE_H_

#include "stm32f10x.h"
#include "stm32f10x_pal.h"

/* 队列句柄结构体定义 */
typedef struct
{
	volatile uint16_t Head; // 队头
	volatile uint16_t Tail; // 队尾
	uint16_t ElementSize; // 元素大小
	uint16_t Capcity; // 容量（缓冲区大小，实际容量是 Capicity - 1）
	volatile uint16_t MaxLength; // 最大长度，用于队列占用率统计
	void *pData; // 缓冲区，用于存放队列的数据
} PalObjectQueue_HandleTypeDef; /* 队列句柄 */

ErrorStatus PAL_ObjectQueue_Init(PalObjectQueue_HandleTypeDef *hqueue, uint16_t ElementSize, uint16_t Size);
ErrorStatus PAL_ObjectQueue_Enqueue(PalObjectQueue_HandleTypeDef *hQueue, void *pElement);
void        PAL_ObjectQueue_EnqueueEx(PalObjectQueue_HandleTypeDef *hQueue, void *pElement);
ErrorStatus PAL_ObjectQueue_Dequeue(PalObjectQueue_HandleTypeDef *hQueue, void *pElement);
uint16_t    PAL_ObjectQueue_GetLength(PalObjectQueue_HandleTypeDef *hQueue);
ErrorStatus PAL_ObjectQueue_DeInit(PalObjectQueue_HandleTypeDef *hQueue);
ErrorStatus PAL_ObjectQueue_EnqueueBatch(PalObjectQueue_HandleTypeDef *hQueue, const void *pData, uint16_t Size);
void        PAL_ObjectQueue_EnqueueBatchEx(PalObjectQueue_HandleTypeDef *hQueue, const void *pData, uint16_t Size);
uint16_t    PAL_ObjectQueue_DequeueBatch(PalObjectQueue_HandleTypeDef *hQueue, void *pData, uint16_t Size);
void        PAL_ObjectQueue_Clear(PalObjectQueue_HandleTypeDef *hQueue);
uint16_t    PAL_ObjectQueue_GetOccupancy(PalObjectQueue_HandleTypeDef *hQueue);

#endif
