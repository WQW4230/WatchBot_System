/**
  ******************************************************************************
  * @file    byte_queue.c
  * @author  铁头山羊stm32工作组
  * @version V1.0.0
  * @date    2023年1月7日
  * @brief   环形字节缓冲队列
  ******************************************************************************
  * @attention
  * Copyright (c) 2022 - 2023 东莞市明玉科技有限公司. 保留所有权力.
  ******************************************************************************
  ==============================================================================
	                                  使用方法
  ==============================================================================
  1. 声明一个环形队列的句柄。比如：
     ByteQueue_HandleTypeDef hQueue;
	2. 对环形队列进行初始化
	   调用ByteQueue_Init() 方法初始化环形队列，其中 Size 参数用于指定缓冲区的大小
		 注意：考虑到程序执行的效率，环形缓冲区大小在初始化时确定，一旦初始化就不能再
		       改变。因此编程时应当预估缓冲区的大小。比如Size=512。
	3. 对环形缓冲队列进行操作
	   ByteQueue_Enqueue - 单个元素入队
		 ByteQueue_EnqueueEx - 同ByteQueue_Enqueue，但队列满时会删除掉最早入队的元素
		 ByteQueue_Dequeue - 让队头的元素出队
		 ByteQueue_GetLength - 获取队列的当前长度
		 ByteQueue_DeInit - 反初始化，释放队列的内存
		 ByteQueue_EnqueueBatch - 多个元素入队
		 ByteQueue_EnqueueBatchEx - 同ByteQueue_EnqueueBatch，但队列满时会删除掉最早入
		                            队的元素
		 ByteQueue_DequeueBatch - 多个元素出队
		 ByteQueue_Clear - 清空队列
		 ByteQueue_GetOccupency - 获取队列的最大占用率，用于评估分配的缓冲区大小是否合理
	*/

#include "pal_byte_queue.h"
#include "stdlib.h"

/*
 * @简介：初始化字节队列。给字节队列分配存储空间
 * @参数：
 *       hQueue - 字节队列的句柄指针
 *       Size - 字节队列缓冲区大小，以字节为单位
 * @返回值：
 *       SUCCESS - 字节队列初始化成功
 *       ERROR - 字节队列初始化失败。可能的原因：单片机RAM空间不足，无法分配足够的内存
 */
ErrorStatus PAL_ByteQueue_Init(PalByteQueue_HandleTypeDef *hQueue, uint16_t Size)
{
	return PAL_ObjectQueue_Init(&hQueue->innerObjectQueue, 1, Size);
}

/*
 * @简介：入队列
 * @参数：
 *       hQueue - 字节队列的句柄指针
 *       Element - 需要入队的字节
 * @返回值：
 *       SUCCESS - 入队成功
 *       ERROR - 入队失败。可能的原因：队列剩余空间不足
 */
ErrorStatus PAL_ByteQueue_Enqueue(PalByteQueue_HandleTypeDef *hQueue, uint8_t Element)
{
	return PAL_ObjectQueue_Enqueue(&hQueue->innerObjectQueue, &Element);
}

/*
 * @简介：入队列，当队列满时自动删除最先入队的元素
 * @参数：
 *       hQueue - 字节队列的句柄指针
 *       Element - 需要入队的字节
 */
void PAL_ByteQueue_EnqueueEx(PalByteQueue_HandleTypeDef *hQueue, uint8_t Element)
{
	PAL_ObjectQueue_EnqueueEx(&hQueue->innerObjectQueue, &Element);
}

/*
 * @简介：出队列
 * @参数：
 *       hQueue - 字节队列的句柄指针
 *       pElement - 字节类型的指针，用于接收出队列的元素
 * @返回值：
 *       SUCCESS - 出队成功
 *       ERROR - 出队失败。可能的原因：队列中无元素，无法完成出队
 */
ErrorStatus PAL_ByteQueue_Dequeue(PalByteQueue_HandleTypeDef *hQueue, uint8_t *pElement)
{
	return PAL_ObjectQueue_Dequeue(&hQueue->innerObjectQueue, pElement);
}

/*
 * @简介：获取队列当前长度
 * @参数：
 *       hQueue - 字节队列的句柄指针
 * @返回值：队列现有的元素个数
 */
uint16_t PAL_ByteQueue_GetLength(PalByteQueue_HandleTypeDef *hQueue)
{
	return PAL_ObjectQueue_GetLength(&hQueue->innerObjectQueue);
}

/*
 * @简介：反初始化队列，释放占用的内存
 * @参数：
 *       hQueue - 字节队列的句柄指针
 * @返回值：SUCCESS - 反初始化成功
 */
ErrorStatus PAL_ByteQueue_DeInit(PalByteQueue_HandleTypeDef *hQueue)
{
	return PAL_ObjectQueue_DeInit(&hQueue->innerObjectQueue);
}

/*
 * @简介：批量入队列
 * @参数：
 *       hQueue - 字节队列的句柄指针
 *       pData - 需要入队的元素组成的数组
 *       Size - 需要入队的元素的个数
 * @返回值：
 *       SUCCESS - 所有的元素入队成功
 *       ERROR - 入队失败。可能的原因：队列剩余空间不足。
 */
ErrorStatus PAL_ByteQueue_EnqueueBatch(PalByteQueue_HandleTypeDef *hQueue, const uint8_t *pData, uint16_t Size)
{
	return PAL_ObjectQueue_EnqueueBatch(&hQueue->innerObjectQueue, pData, Size);
}

/*
 * @简介：批量入队列，当队列满时自动删除最先入队的元素
 * @参数：
 *       hQueue - 字节队列的句柄指针
 *       pData - 需要入队的元素组成的数组
 *       Size - 需要入队的元素的个数
 */
void PAL_ByteQueue_EnqueueBatchEx(PalByteQueue_HandleTypeDef *hQueue, const uint8_t *pData, uint16_t Size)
{
	PAL_ObjectQueue_EnqueueBatchEx(&hQueue->innerObjectQueue, pData, Size);
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
uint16_t PAL_ByteQueue_DequeueBatch(PalByteQueue_HandleTypeDef *hQueue, uint8_t *pData, uint16_t Size)
{
	return PAL_ObjectQueue_DequeueBatch(&hQueue->innerObjectQueue, pData, Size);
}

/*
 * @简介：清空队列
 * @参数：
 *       hQueue - 字节队列的句柄指针
 */
void PAL_ByteQueue_Clear(PalByteQueue_HandleTypeDef *hQueue)
{
	PAL_ObjectQueue_Clear(&hQueue->innerObjectQueue);
}

/*
 * @简介：获取队列的最大占用率
 * @参数：
 *       hQueue - 字节队列的句柄指针
 * @返回值：占用率 0~100
 */
uint16_t PAL_ByteQueue_GetOccupency(PalByteQueue_HandleTypeDef *hQueue)
{
	return PAL_ObjectQueue_GetOccupency(&hQueue->innerObjectQueue);
}
