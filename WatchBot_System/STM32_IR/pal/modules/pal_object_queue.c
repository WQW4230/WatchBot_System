/**
  ******************************************************************************
  * @file    pal_object_queue.c
  * @author  铁头山羊stm32工作组
  * @version V1.0.0
  * @date    2023年1月7日
  * @brief   环形缓冲队列
  ******************************************************************************
  * @attention
  * Copyright (c) 2022 - 2023 东莞市明玉科技有限公司. 保留所有权力.
  ******************************************************************************
	*/

#include "pal_object_queue.h"
#include "stdlib.h"
#include "string.h"

#define ElementCpy(Ptr1, i1, Ptr2, i2) memcpy((uint8_t *)(Ptr1) + (i1) * hQueue->ElementSize, (uint8_t *)(Ptr2) + (i2) * hQueue->ElementSize, hQueue->ElementSize)

ErrorStatus PAL_ObjectQueue_Init(PalObjectQueue_HandleTypeDef *hQueue, uint16_t ElementSize, uint16_t Size)
{
	hQueue->ElementSize = ElementSize;
	hQueue->pData = (uint8_t*)malloc(hQueue->ElementSize * Size); // 分配存储空间
	if(hQueue->pData == 0) 
	{
		return ERROR;// 如果分配存储空间失败，则返回ERROR
	}
	hQueue->Capcity = Size;
	hQueue->Head = 0;
	hQueue->Tail = 0;
	hQueue->MaxLength = 0;
	return SUCCESS;
}

ErrorStatus PAL_ObjectQueue_Enqueue(PalObjectQueue_HandleTypeDef *hQueue, void *pElement)
{
	if(PAL_ObjectQueue_GetLength(hQueue) == hQueue->Capcity - 1) {
		return ERROR; // 如果队列剩余空间不足，则返回ERROR
	}

	ElementCpy(hQueue->pData, hQueue->Tail, pElement, 0); // 新元素插入到队尾
	hQueue->Tail = (hQueue->Tail + 1) % hQueue->Capcity; // 队位指针向后移动

	uint16_t length = PAL_ObjectQueue_GetLength(hQueue);

	// 附加功能：统计队列的占用率
	if(length > hQueue->MaxLength)
	{
		hQueue->MaxLength = length; 
	}

	return SUCCESS;
}

void PAL_ObjectQueue_EnqueueEx(PalObjectQueue_HandleTypeDef *hQueue, void *pElement)
{
	if(PAL_ObjectQueue_GetLength(hQueue) == hQueue->Capcity - 1)
	{
		hQueue->Head = (hQueue->Head + 1) % hQueue->Capcity; // 如果队列满，先删除掉队头的一个元素
	}
	ElementCpy(hQueue->pData, hQueue->Tail, pElement, 0); // 新元素插入到队尾
	hQueue->Tail = (hQueue->Tail + 1) % hQueue->Capcity; // 队位指针向后移动
	uint16_t length = PAL_ObjectQueue_GetLength(hQueue);

	// 附加功能：统计队列的占用率
	if(length > hQueue->MaxLength)
	{
		hQueue->MaxLength = length; 
	}
}

ErrorStatus PAL_ObjectQueue_Dequeue(PalObjectQueue_HandleTypeDef *hQueue, void *pElement)
{
	if(PAL_ObjectQueue_GetLength(hQueue) == 0)
	{
		return ERROR;
	}
	ElementCpy(pElement, 0, hQueue->pData, hQueue->Head); 
	hQueue->Head = (hQueue->Head + 1) % (hQueue->Capcity);
	return SUCCESS;
}

uint16_t PAL_ObjectQueue_GetLength(PalObjectQueue_HandleTypeDef *hQueue)
{
	return (hQueue->Tail + hQueue->Capcity - hQueue->Head) % hQueue->Capcity;
}

ErrorStatus PAL_ObjectQueue_DeInit(PalObjectQueue_HandleTypeDef *hQueue)
{
	if(hQueue->pData != 0)
	{
		free(hQueue->pData);
		hQueue->pData = 0;
	}
	hQueue->Capcity = 0;
	hQueue->Head = 0;
	hQueue->Tail = 0;
	hQueue->MaxLength = 0;
	return SUCCESS;
}

ErrorStatus PAL_ObjectQueue_EnqueueBatch(PalObjectQueue_HandleTypeDef *hQueue, const void *pData, uint16_t Size)
{
	uint32_t tailcpy, i;

	if(PAL_ObjectQueue_GetLength(hQueue) + Size >= hQueue->Capcity - 1)
	{
		return ERROR; // 如果队列不足以容纳所有新元素，则入队失败
	}

	// 将所有新元素插入到队尾
	tailcpy = hQueue->Tail;

	for(i=0;i<Size;i++)
	{
		ElementCpy(&hQueue->pData, tailcpy, pData, i);
		tailcpy = (tailcpy + 1) % hQueue->Capcity;
	}
	hQueue->Tail = tailcpy;

	// 附加功能：统计队列的占用率
	uint16_t length = PAL_ObjectQueue_GetLength(hQueue);

	if(length > hQueue->MaxLength)
	{
		hQueue->MaxLength = length;
	}

	return SUCCESS;
}

/*
 * @简介：批量入队列，当队列满时自动删除最先入队的元素
 * @参数：
 *       hQueue - 字节队列的句柄指针
 *       pData - 需要入队的元素组成的数组
 *       Size - 需要入队的元素的个数
 */
void PAL_ObjectQueue_EnqueueBatchEx(PalObjectQueue_HandleTypeDef *hQueue, const void *pData, uint16_t Size)
{
	uint32_t tailcpy, i;
	uint32_t num_of_bytes_dequeue; // 需要丢弃的队列中元素数量
	uint32_t num_of_bytes_abondon = 0; // 需要丢弃的新元素数量
	
	// 如果插入新元素后的队列长度大于队列容量（队列剩余空间不足）
	if(PAL_ObjectQueue_GetLength(hQueue) + Size >= hQueue->Capcity - 1)
	{
		// 抛弃掉队头的元素
		// 计算需要出队的元素数量 = 入队后队列的长度 - 队列的容量
		num_of_bytes_dequeue = PAL_ObjectQueue_GetLength(hQueue) + Size - (hQueue->Capcity - 1);
		
		// 即使抛弃掉所有元素仍不能容纳新元素（新元素数量大于队列容量）
		// 考虑抛弃掉新元素中的一部分
		if(num_of_bytes_dequeue > PAL_ObjectQueue_GetLength(hQueue))
		{
			PAL_ObjectQueue_Clear(hQueue); // 清空队列
			
			// 计算需要抛弃的新元素的数量
			// 需要抛弃的新元素的数量 = 需要出队的数量（大于当前队列长度） - 当前队列长度
			num_of_bytes_abondon = num_of_bytes_dequeue - PAL_ObjectQueue_GetLength(hQueue);
		}
		else
		{
			// 如果队列剩余空间不足以容纳新元素，队头元素先出队
		  hQueue->Head = (hQueue->Head + num_of_bytes_dequeue) % hQueue->Capcity;
		}
	}

	// 将所有新元素插入到队尾
	tailcpy = hQueue->Tail;

	for(i=num_of_bytes_abondon;i<Size-num_of_bytes_abondon;i++)
	{
		ElementCpy(hQueue->pData, tailcpy, pData, i);
		tailcpy = (tailcpy + 1) % hQueue->Capcity;
	}
	hQueue->Tail = tailcpy;

	// 附加功能：统计队列的占用率
	uint16_t length = PAL_ObjectQueue_GetLength(hQueue);

	if(length > hQueue->MaxLength)
	{
		hQueue->MaxLength = length;
	}
}

/*
 * @简介：批量出队
 * @参数：
 *       hQueue - 字节队列的句柄指针
 *       pData - 数组，缓冲区，用于接收出队的元素
 *       Size - 需要出队的元素的个数
 * @返回值：
 *       实际出队的元素个数
 */
uint16_t PAL_ObjectQueue_DequeueBatch(PalObjectQueue_HandleTypeDef *hQueue, void *pData, uint16_t Size)
{
	uint32_t i;
	uint16_t nRead = PAL_ObjectQueue_GetLength(hQueue);

	if(nRead > Size)
	{
		nRead = Size;
	}


	for(i=0;i<nRead;i++)
	{
		ElementCpy(pData, i, hQueue->pData, (hQueue->Head + i) % hQueue->Capcity);
	}

	hQueue->Head = (hQueue->Head + nRead) % hQueue->Capcity;


	return nRead;
}

/*
 * @简介：清空队列
 * @参数：
 *       hQueue - 字节队列的句柄指针
 */
void PAL_ObjectQueue_Clear(PalObjectQueue_HandleTypeDef *hQueue)
{
	hQueue->Head = 0;
	hQueue->Tail = 0;
}

/*
 * @简介：获取队列的最大占用率
 * @参数：
 *       hQueue - 字节队列的句柄指针
 * @返回值：占用率 0~100
 */
uint16_t PAL_ObjectQueue_GetOccupancy(PalObjectQueue_HandleTypeDef *hQueue)
{
	return (uint16_t)(100.0 * hQueue->MaxLength / hQueue->Capcity);
}
