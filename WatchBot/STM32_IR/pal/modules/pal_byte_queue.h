/**
  ******************************************************************************
  * @file    byte_queue.h
  * @author  铁头山羊stm32工作组
  * @version V1.0.0
  * @date    2023年1月7日
  * @brief   环形字节缓冲队列
  ******************************************************************************
  * @attention
  * Copyright (c) 2022 - 2023 东莞市明玉科技有限公司. 保留所有权力.
  ******************************************************************************
	*/

#ifndef __BYTE_QUEUE_H_
#define __BYTE_QUEUE_H_

#include "stm32f10x.h"
#include "stm32f10x_pal.h"
#include "pal_object_queue.h"

/* 队列句柄结构体定义 */
typedef struct
{
	PalObjectQueue_HandleTypeDef innerObjectQueue;
} PalByteQueue_HandleTypeDef; /* 队列句柄 */

ErrorStatus PAL_ByteQueue_Init(PalByteQueue_HandleTypeDef *hqueue, uint16_t Size);
ErrorStatus PAL_ByteQueue_Enqueue(PalByteQueue_HandleTypeDef *hQueue, uint8_t Element);
void        PAL_ByteQueue_EnqueueEx(PalByteQueue_HandleTypeDef *hQueue, uint8_t Element);
ErrorStatus PAL_ByteQueue_Dequeue(PalByteQueue_HandleTypeDef *hQueue, uint8_t *pElement);
uint16_t    PAL_ByteQueue_GetLength(PalByteQueue_HandleTypeDef *hQueue);
ErrorStatus PAL_ByteQueue_DeInit(PalByteQueue_HandleTypeDef *hQueue);
ErrorStatus PAL_ByteQueue_EnqueueBatch(PalByteQueue_HandleTypeDef *hQueue, const uint8_t *pData, uint16_t Size);
void        PAL_ByteQueue_EnqueueBatchEx(PalByteQueue_HandleTypeDef *hQueue, const uint8_t *pData, uint16_t Size);
uint16_t    PAL_ByteQueue_DequeueBatch(PalByteQueue_HandleTypeDef *hQueue, uint8_t *pData, uint16_t Size);
void        PAL_ByteQueue_Clear(PalByteQueue_HandleTypeDef *hQueue);
uint16_t    PAL_ByteQueue_GetOccupancy(PalByteQueue_HandleTypeDef *hQueue);

#endif
