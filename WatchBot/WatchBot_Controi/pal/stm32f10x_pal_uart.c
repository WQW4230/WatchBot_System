/**
  ******************************************************************************
  * @file    stm32f10x_pal_uart.c
  * @author  铁头山羊stm32工作组
  * @version V1.0.0
  * @date    2023年1月7日
  * @brief   串口驱动
  ******************************************************************************
  * @attention
  * Copyright (c) 2022 - 2023 东莞市明玉科技有限公司. 保留所有权力.
  ******************************************************************************
*/

#include "stm32f10x_pal.h"
#include "stm32f10x_pal_uart.h"
#include "stdio.h"
#include "string.h"
#include "stdarg.h"

static void RCC_USARTClockCmd(USART_TypeDef *USARTx, FunctionalState NewState);

// @简介：UART初始化
// @参数：Handle - UART的句柄指针
// @返回值：初始化是否成功，SUCCESS表示成功，其它表示失败
ErrorStatus PAL_UART_Init(PalUART_HandleTypeDef *Handle)
{
	USART_InitTypeDef USART_InitStruct;
	GPIO_InitTypeDef GPIO_InitStruct;
	
	#if (defined STM32F10X_LD) || (defined STM32F10X_LD_VL)
	if(Handle->Init.USARTx == USART3) // 低密度器件无法使用USART3
	{
		return ERROR;
	}
	#endif
	
	// 不支持UART4和UART5
	if(Handle->Init.USARTx == UART4 || Handle->Init.USARTx == UART5)
	{
		return ERROR;
	}
	// 使能USART的时钟
	RCC_USARTClockCmd(Handle->Init.USARTx, ENABLE);
	
	// 复位USART
	USART_DeInit(Handle->Init.USARTx);
	
	if(Handle->Init.Tx.Enable == SET)// 发送
	{
		if(Handle->Init.Tx.DMA.Enable == SET) // DMA
		{
			// 初始化发送队列
			if(PAL_ByteQueue_Init(&Handle->hTxQueue, Handle->Init.Tx.DMA.QueueSize) != SUCCESS)
			{
				return ERROR;
			}
			
			// 初始化DMA
			if(Handle->Init.USARTx == USART1)
			{
				Handle->DMATxChannel = DMA1_Channel4;
			}
			else if(Handle->Init.USARTx == USART2)
			{
				Handle->DMATxChannel = DMA1_Channel7;
			}
			else if(Handle->Init.USARTx == USART3)
			{
				Handle->DMATxChannel = DMA1_Channel2;
			}
			
			RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
			
			DMA_SetCurrDataCounter(Handle->DMATxChannel, 0);
			
			USART_DMACmd(Handle->Init.USARTx, USART_DMAReq_Tx, ENABLE);
		}
		// 初始化Tx引脚
		if(Handle->Init.USARTx == USART1)
		{
			if(Handle->Init.Advanced.Remap == 0) // 无重映射
			{
				// PA9 -> UART_Tx 复用推挽
				RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
				GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;
				GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
				GPIO_InitStruct.GPIO_Speed = GPIO_Speed_10MHz;
				GPIO_Init(GPIOA, &GPIO_InitStruct);
			}
			else if(Handle->Init.Advanced.Remap == 1) // 完全重映射
			{
				// PB6 -> UART_Tx 复用推挽
				RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
				GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6;
				GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
				GPIO_InitStruct.GPIO_Speed = GPIO_Speed_10MHz;
				GPIO_Init(GPIOB, &GPIO_InitStruct);
			}
			else
			{
				return ERROR; // 参数有错
			}
		}
		else if(Handle->Init.USARTx == USART2)
		{
			if(Handle->Init.Advanced.Remap == 0)
			{
				// PA2 -> UART_Tx 复用推挽
				RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
				GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2;
				GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
				GPIO_InitStruct.GPIO_Speed = GPIO_Speed_10MHz;
				GPIO_Init(GPIOA, &GPIO_InitStruct);
			}
			else if(Handle->Init.Advanced.Remap == 1)
			{
				// PD5 -> UART_Tx 复用推挽
				RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
				GPIO_InitStruct.GPIO_Pin = GPIO_Pin_5;
				GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
				GPIO_InitStruct.GPIO_Speed = GPIO_Speed_10MHz;
				GPIO_Init(GPIOD, &GPIO_InitStruct);
			}
			else
			{
				return ERROR;
			}
		}
		else if(Handle->Init.USARTx == USART3)
		{
			if(Handle->Init.Advanced.Remap == 0)
			{
				// PB10 -> UART_Tx 复用推挽
				RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
				GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;
				GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
				GPIO_InitStruct.GPIO_Speed = GPIO_Speed_10MHz;
				GPIO_Init(GPIOB, &GPIO_InitStruct);
			}
			else if(Handle->Init.Advanced.Remap == 1)
			{
					// PC10 -> UART_Tx 复用推挽
					RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
					GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;
					GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
					GPIO_InitStruct.GPIO_Speed = GPIO_Speed_10MHz;
					GPIO_Init(GPIOC, &GPIO_InitStruct);
			}
			else if(Handle->Init.Advanced.Remap == 3)
			{
				// PD8 -> UART_Tx 复用推挽
				RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
				GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8;
				GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
				GPIO_InitStruct.GPIO_Speed = GPIO_Speed_10MHz;
				GPIO_Init(GPIOD, &GPIO_InitStruct);
			}
			else
			{
				return ERROR;
			}
		}
	}
	
	if(Handle->Init.Rx.Enable == SET)
	{
		Handle->LastestCh = '\0';
		Handle->LineSeparatorCounter1 = 0;
		Handle->LineSeparatorCounter2 = 0;
		// 初始化接收缓冲区
		if(PAL_ByteQueue_Init(&Handle->hRxQueue, Handle->Init.Rx.QueueSize) != SUCCESS)
		{
			return ERROR;
		}
		// 初始化Rx引脚
		if(Handle->Init.USARTx == USART1)
		{
			if(Handle->Init.Advanced.Remap == 0)
			{
				// PA10 -> UART_Rx 输入浮空
				RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
				GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;
				GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
				GPIO_Init(GPIOA, &GPIO_InitStruct);
			}
			else if(Handle->Init.Advanced.Remap == 1)
			{
				// PB7 -> UART_Rx 输入浮空
				RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
				GPIO_InitStruct.GPIO_Pin = GPIO_Pin_7;
				GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
				GPIO_Init(GPIOB, &GPIO_InitStruct);
			}
			else
			{
				return ERROR;
			}
		}
		else if(Handle->Init.USARTx == USART2)
		{
			if(Handle->Init.Advanced.Remap == 0)
			{
				// PA3 -> UART_Rx 输入浮空
				RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
				GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3;
				GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
				GPIO_Init(GPIOA, &GPIO_InitStruct);
			}
			else if(Handle->Init.Advanced.Remap == 1)
			{
				// PD6 -> UART_Rx 输入浮空
				RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
				GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6;
				GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
				GPIO_Init(GPIOD, &GPIO_InitStruct);
			}
			else
			{
				return ERROR;
			}
		}
		else if(Handle->Init.USARTx == USART3)
		{
			if(Handle->Init.Advanced.Remap == 0)
			{
				// PB11 -> UART_Rx 输入浮空
				RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
				GPIO_InitStruct.GPIO_Pin = GPIO_Pin_11;
				GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
				GPIO_Init(GPIOB, &GPIO_InitStruct);
			}
			else if(Handle->Init.Advanced.Remap == 1)
			{
				// PC11 -> UART_Rx 输入浮空
				RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
				GPIO_InitStruct.GPIO_Pin = GPIO_Pin_11;
				GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
				GPIO_Init(GPIOC, &GPIO_InitStruct);
			}
			else if(Handle->Init.Advanced.Remap == 3)
			{
				// PD9 -> UART_Rx 输入浮空
				RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
				GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;
				GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
				GPIO_Init(GPIOD, &GPIO_InitStruct);
			}
			else
			{
				return ERROR;
			}
		}		
	}
	
	// 初始化USART
	USART_InitStruct.USART_BaudRate = Handle->Init.Basic.BaudRate;
	USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // 该模式下默认关闭流控
	USART_InitStruct.USART_Mode = 0;
	if(Handle->Init.Tx.Enable == SET)
	{
		USART_InitStruct.USART_Mode |= USART_Mode_Tx;
	}
	if(Handle->Init.Rx.Enable == SET)
	{
		USART_InitStruct.USART_Mode |= USART_Mode_Rx;
	}
	USART_InitStruct.USART_Parity = Handle->Init.Basic.USART_Parity;
	USART_InitStruct.USART_StopBits = Handle->Init.Basic.USART_StopBits;
	USART_InitStruct.USART_WordLength = Handle->Init.Basic.USART_WordLength;
	USART_Init(Handle->Init.USARTx, &USART_InitStruct);
	
	// 配置RxNE中断
	if(Handle->Init.Rx.Enable == SET)
	{
		USART_ITConfig(Handle->Init.USARTx, USART_IT_RXNE, ENABLE);
		
		NVIC_InitTypeDef NVIC_InitStruct;
		if(Handle->Init.USARTx == USART1)
		{
			NVIC_InitStruct.NVIC_IRQChannel = USART1_IRQn;
		}
		else if(Handle->Init.USARTx == USART2)
		{
			NVIC_InitStruct.NVIC_IRQChannel = USART2_IRQn;
		}
#if (!defined STM32F10X_LD) || (!defined STM32F10X_LD_VL)
		else if(Handle->Init.USARTx == USART3)
		{
			NVIC_InitStruct.NVIC_IRQChannel = USART3_IRQn;
		}
#endif
		NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = Handle->Init.Rx.USART_IRQ_PreemptionPriority;
		NVIC_InitStruct.NVIC_IRQChannelSubPriority = Handle->Init.Rx.USART_IRQ_SubPriority;
		NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
		
		NVIC_Init(&NVIC_InitStruct);
	}
	
	USART_Cmd(Handle->Init.USARTx, ENABLE);
	
	return SUCCESS;
}

// @简介：USART中断处理代码
// @参数：Handle - UART句柄的指针
// @返回值：空
// @注意：此方法应当在对应的UART的中断响应函数中被调用
void PAL_UART_UART_IRQHandler(PalUART_HandleTypeDef *Handle)
{
	uint8_t byte_received;
	// 接收到数据后将数据存放在缓冲区中
	if(Handle->Init.Rx.Enable == SET)
	{
		while(USART_GetITStatus(Handle->Init.USARTx, USART_IT_RXNE) == SET)
		{
			byte_received = (uint8_t)USART_ReceiveData(Handle->Init.USARTx);
			
			if(Handle->Init.Rx.LineSeparator != LineSeparator_Disable)
			{
				// 判断是否接收到了新行
				switch(Handle->Init.Rx.LineSeparator)
				{
					case LineSeparator_CR:
					{
						if(byte_received == '\r' && Handle->hTxQueue.innerObjectQueue.Capcity > 1 )
						{
							Handle->LineSeparatorCounter1 = Handle->hRxQueue.innerObjectQueue.Capcity - 1;
							Handle->LineSeparatorCounter2 = PAL_ByteQueue_GetLength(&Handle->hRxQueue) + 1;
							if(Handle->LineSeparatorCounter2 == Handle->hRxQueue.innerObjectQueue.Capcity)
							{
								Handle->LineSeparatorCounter2--;
							}
						}
						else
						{
							if(Handle->LineSeparatorCounter1 != 0)
							{
								Handle->LineSeparatorCounter1--;
							}
						}
						break;
					}
					case LineSeparator_LF:
					{
						if(byte_received == '\n' && Handle->hTxQueue.innerObjectQueue.Capcity > 1)
						{
							Handle->LineSeparatorCounter1 = Handle->hRxQueue.innerObjectQueue.Capcity - 1;
							Handle->LineSeparatorCounter2 = PAL_ByteQueue_GetLength(&Handle->hRxQueue) + 1;
							if(Handle->LineSeparatorCounter2 == Handle->hRxQueue.innerObjectQueue.Capcity)
							{
								Handle->LineSeparatorCounter2--;
							}
						}
						else
						{
							if(Handle->LineSeparatorCounter1 != 0)
							{
								Handle->LineSeparatorCounter1--;
							}
						}
						break;
					}
					case LineSeparator_CRLF:
					{
						if(byte_received == '\n' && Handle->LastestCh == '\r' && Handle->hTxQueue.innerObjectQueue.Capcity > 2)
						{
							Handle->LineSeparatorCounter1 = Handle->hRxQueue.innerObjectQueue.Capcity - 2;
							Handle->LineSeparatorCounter2 = PAL_ByteQueue_GetLength(&Handle->hRxQueue) + 1;
							if(Handle->LineSeparatorCounter2 == Handle->hRxQueue.innerObjectQueue.Capcity)
							{
								Handle->LineSeparatorCounter2--;
							}
						}
						else
						{
							if(Handle->LineSeparatorCounter1 != 0)
							{
								Handle->LineSeparatorCounter1--;
							}
						}
						break;
					}
				}
			}
			// 如果环形队列满，则自动删除掉最早的元素
			PAL_ByteQueue_EnqueueEx(&Handle->hRxQueue, byte_received);
			
			Handle->LastestCh = byte_received;
			
			if(Handle->LineSeparatorCounter1 == 0)
			{
				Handle->LineSeparatorCounter2 = 0;
			}
		}
	}
}

// @简介：将发送缓冲区的数据交给DMA进行发送
// @参数：Handle - UART句柄的指针
// @返回值：空
// @注意：此方法适用于DMA发送方式，当使用PAL_UART_xxx_DMA后应当调用此方法
void PAL_UART_FlushDMA(PalUART_HandleTypeDef *Handle)
{
	uint16_t txlen;
	DMA_InitTypeDef DMA_InitStruct;
	
	if(Handle->Init.Tx.Enable != SET || Handle->Init.Tx.DMA.Enable != SET) return;
	
	/* 1. 发送Tx队列中的数据 ********************************************/
	
  txlen	= PAL_ByteQueue_GetLength(&Handle->hTxQueue); 

	// 无数据需要发送，或者上次数据还未发完
	if(txlen == 0 || DMA_GetCurrDataCounter(Handle->DMATxChannel) != 0)
	{
		return;
	}
	
	// 每次最多发送512个数据
	if(txlen > 256)
	{
		txlen = 256;
	}
	
	// 将数据存放在缓冲区中
	PAL_ByteQueue_DequeueBatch(&Handle->hTxQueue, Handle->DMATxBuffer, txlen);
	
	// 设置DMA
	DMA_Cmd(Handle->DMATxChannel, DISABLE);
	
	DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t)(&(Handle->Init.USARTx->DR));
	DMA_InitStruct.DMA_MemoryBaseAddr = (uint32_t)Handle->DMATxBuffer;
	DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralDST;
	DMA_InitStruct.DMA_BufferSize = txlen;
	DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
	DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStruct.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStruct.DMA_Priority = DMA_Priority_Medium;
	DMA_InitStruct.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(Handle->DMATxChannel, &DMA_InitStruct);
	
	DMA_Cmd(Handle->DMATxChannel, ENABLE);
}

// @简介：使用DMA方式发送一个字节
// @参数：Handle - UART句柄的指针
// @参数：Data - 要发送的字节
// @注意：此方法的真正作用是将此字节添加到发送缓冲区中，并不会实际发送出去
//        该字节会在下一次PAL_UART_Flash_DMA方法调用时被发送
void PAL_UART_SendByte_DMA(PalUART_HandleTypeDef *Handle, uint8_t Data)
{
	if(Handle->Init.Tx.Enable == SET && Handle->Init.Tx.DMA.Enable == SET)
	{
		PAL_ByteQueue_EnqueueEx(&Handle->hTxQueue, Data);
	}
}

// @简介：使用常规方式发送一个字节
// @参数：Handle - UART句柄的指针
// @参数：Data - 要发送的字节
void PAL_UART_SendByte(PalUART_HandleTypeDef *Handle, uint8_t Data)
{
	if(Handle->Init.Tx.Enable == SET)
	{
		while(USART_GetFlagStatus(Handle->Init.USARTx, USART_FLAG_TXE) != SET); // 等待标志位TXE置位
		USART_SendData(Handle->Init.USARTx, Data);
		while(USART_GetFlagStatus(Handle->Init.USARTx, USART_FLAG_TC) != SET); // 等待标志位TC置位
	}
}

// @简介：使用DMA方式发送多个字节
// @参数：Handle - UART句柄的指针
// @参数：pData - 要发送的字节数组的指针
// @参数：Size - 要发送的数据个数
// @注意：此方法的真正作用是将字节数组添加到发送缓冲区中，并不会实际发送出去
//        该字节数组会在下一次PAL_UART_Flash_DMA方法调用时被发送
// @注意：单次发送的字节数组的大小不得超过发送缓冲区的容量
void PAL_UART_SendBytes_DMA(PalUART_HandleTypeDef *Handle, const uint8_t* pData, uint16_t Size)
{
	if(Handle->Init.Tx.Enable == SET && Handle->Init.Tx.DMA.Enable == SET)
	{
		PAL_ByteQueue_EnqueueBatchEx(&Handle->hTxQueue, pData, Size);
	}
}

// @简介：使用常规方式发送多个字节
// @参数：Handle - UART句柄的指针
// @参数：pData - 要发送的字节数组的指针
// @参数：Size - 要发送的数据个数
void PAL_UART_SendBytes(PalUART_HandleTypeDef *Handle, const uint8_t* pData, uint16_t Size)
{
	uint16_t i;
	
	if(Handle->Init.Tx.Enable != SET) return;
	
	for(i=0;i<Size;i++)
	{
		while(USART_GetFlagStatus(Handle->Init.USARTx, USART_FLAG_TXE) != SET); // 等待标志位TXE置位
		USART_SendData(Handle->Init.USARTx, pData[i]);
	}
	while(USART_GetFlagStatus(Handle->Init.USARTx, USART_FLAG_TC) != SET); // 等待标志位TC置位
}

// @简介：接收单个字节
// @参数：Handle - UART句柄的指针
// @参数：Timeout - 超时值，如果此参数填写PAL_MAX_DELAY则永不超时
// @返回值：接收到的字节，如果返回值小于0表示接收失败（超时）
int16_t PAL_UART_RecieveByte(PalUART_HandleTypeDef *Handle, uint32_t Timeout)
{
	if(Handle->Init.Rx.Enable != SET) return 0;
	
	int16_t ret = -1;
	uint8_t tmp;
	uint64_t expiredTime = PAL_GetTick() + Timeout;
	
	do
	{
		if(PAL_ByteQueue_GetLength(&Handle->hRxQueue) != 0)
		{
			__disable_irq();
			if(PAL_ByteQueue_GetLength(&Handle->hRxQueue) != 0)
			{
				PAL_ByteQueue_Dequeue(&Handle->hRxQueue, &tmp);
				
				ret = tmp;
				
				if(Handle->LineSeparatorCounter2 != 0)
				{
					Handle->LineSeparatorCounter2--;
					if(Handle->LineSeparatorCounter2 == 0)
					{
						Handle->LineSeparatorCounter1 = 0;
					}
				}
			}
			__enable_irq();
			break;
		}
	}
	while(Timeout == PAL_MAX_DELAY || PAL_GetTick() < expiredTime);
	
	return ret;
}

uint16_t PAL_UART_RecieveBytes(PalUART_HandleTypeDef *Handle, uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
	uint16_t num_of_bytes_2_read;
	uint16_t queue_length;
	uint64_t expiredTime;
	
	if(Handle->Init.Rx.Enable != SET) return 0;
	
	if(Timeout != 0)
	{
		expiredTime = PAL_GetTick() + Timeout;
		
		while(( Timeout == PAL_MAX_DELAY || PAL_GetTick() < expiredTime) && PAL_ByteQueue_GetLength(&Handle->hRxQueue) < Size);
	}
	
	num_of_bytes_2_read = Size;
	
	__disable_irq();
	
	queue_length = PAL_ByteQueue_GetLength(&Handle->hRxQueue);
	
	if(queue_length < num_of_bytes_2_read)
	{
		num_of_bytes_2_read = queue_length;
	}
	
	PAL_ByteQueue_DequeueBatch(&Handle->hRxQueue, pData, num_of_bytes_2_read);
	
	if(Handle->LineSeparatorCounter2 != 0)
	{
		if(num_of_bytes_2_read > Handle->LineSeparatorCounter2)
		{
			Handle->LineSeparatorCounter2 = 0;
		}
		else
		{
			Handle->LineSeparatorCounter2 -= num_of_bytes_2_read;
		}
		if(Handle->LineSeparatorCounter2 == 0)
		{
			Handle->LineSeparatorCounter1 = 0;
		}
	}
	
	__enable_irq();
	
	return num_of_bytes_2_read;
}

uint16_t PAL_UART_ReadLine(PalUART_HandleTypeDef *Handle, char *pBuffer, uint16_t Size, uint32_t Timeout)
{
	uint64_t expiredTime;
	uint16_t strlen = 0;
	uint8_t currentByte, previousByte = 0;
	
	if(Timeout != 0)
	{
		expiredTime = PAL_GetTick() + Timeout;
	}
	
	do
	{
		if(Timeout == 0 || Handle->LineSeparatorCounter1 != 0)
		{
			__disable_irq();
			if(Timeout != 0 && Handle->LineSeparatorCounter1 == 0) // 在禁用中断之前行结束符被丢弃了
			{
				continue;
			}
			
			for(; Handle->LineSeparatorCounter1 > 0; )
			{
				PAL_ByteQueue_Dequeue(&Handle->hRxQueue, &currentByte);
				
				if(strlen < Size - 1)
				{
					pBuffer[strlen++] = currentByte;
				}
				
				Handle->LineSeparatorCounter2--;
				
				if(Handle->LineSeparatorCounter2 == 0)
				{
					Handle->LineSeparatorCounter1 = 0;
				}
				
				if((Handle->Init.Rx.LineSeparator == LineSeparator_CRLF 
					&& previousByte == '\r' && currentByte == '\n')
				||(Handle->Init.Rx.LineSeparator == LineSeparator_CR
					&& currentByte == '\r')
				||(Handle->Init.Rx.LineSeparator == LineSeparator_LF
					&& currentByte == '\n'))
				{
					break;
				}
				
				previousByte = currentByte;
			}
			
			pBuffer[strlen] = '\0';
			__enable_irq();
			
			break;
		}
	}
	while((Timeout == PAL_MAX_DELAY)||(PAL_GetTick() < expiredTime));
	
	return strlen;
}

void PAL_UART_ChangeBaudrate(PalUART_HandleTypeDef *Handle, uint32_t NewBaudrate)
{
	uint32_t tmpreg = 0x00;
	uint32_t usartxbase = 0x00, apbclock = 0x00;
	RCC_ClocksTypeDef RCC_ClocksStatus;
	uint32_t integerdivider = 0x00;
	uint32_t fractionaldivider = 0x00;
	
  RCC_GetClocksFreq(&RCC_ClocksStatus);
  if (usartxbase == USART1_BASE)
  {
    apbclock = RCC_ClocksStatus.PCLK2_Frequency;
  }
  else
  {
    apbclock = RCC_ClocksStatus.PCLK1_Frequency;
  }
  
  /* Determine the integer part */
  if ((Handle->Init.USARTx->CR1 & 0x8000) != 0)
  {
    /* Integer part computing in case Oversampling mode is 8 Samples */
    integerdivider = ((25 * apbclock) / (2 * NewBaudrate));    
  }
  else /* if ((USARTx->CR1 & CR1_OVER8_Set) == 0) */
  {
    /* Integer part computing in case Oversampling mode is 16 Samples */
    integerdivider = ((25 * apbclock) / (4 * NewBaudrate));    
  }
  tmpreg = (integerdivider / 100) << 4;

  /* Determine the fractional part */
  fractionaldivider = integerdivider - (100 * (tmpreg >> 4));

  /* Implement the fractional part in the register */
  if ((Handle->Init.USARTx->CR1 & 0x8000) != 0)
  {
    tmpreg |= ((((fractionaldivider * 8) + 50) / 100)) & ((uint8_t)0x07);
  }
  else /* if ((USARTx->CR1 & CR1_OVER8_Set) == 0) */
  {
    tmpreg |= ((((fractionaldivider * 16) + 50) / 100)) & ((uint8_t)0x0F);
  }
  
	USART_Cmd(Handle->Init.USARTx, DISABLE);
  /* Write to USART BRR */
  Handle->Init.USARTx->BRR = (uint16_t)tmpreg;
	
	USART_Cmd(Handle->Init.USARTx, ENABLE);
	
	Handle->Init.Basic.BaudRate = NewBaudrate;
}

uint16_t PAL_UART_NumberOfBytesToRead(PalUART_HandleTypeDef *Handle)
{
	uint16_t ret = 0;
	
	if(Handle->Init.Rx.Enable != SET) return 0;
	
	__disable_irq();
	
	ret = PAL_ByteQueue_GetLength(&Handle->hRxQueue);
	
	__enable_irq();
	
	return ret;
}

void PAL_UART_Printf(PalUART_HandleTypeDef *Handle, const char* format, ...)
{
	uint8_t format_buffer[128];
	
	if(Handle->Init.Tx.Enable != SET) return;
	
	va_list argptr;
	__va_start(argptr, format);
	vsprintf((char* )format_buffer, format, argptr);
	__va_end(argptr);
	PAL_UART_SendBytes(Handle, format_buffer, strlen((char*)format_buffer));
}

void PAL_UART_Printf_DMA(PalUART_HandleTypeDef *Handle, const char* format, ...)
{
	uint8_t format_buffer[128];
	
	if(Handle->Init.Tx.Enable != SET || Handle->Init.Tx.DMA.Enable != SET) return;
	
	va_list argptr;
	
	__va_start(argptr, format);
	vsprintf((char* )format_buffer, format, argptr);
	__va_end(argptr);
	PAL_ByteQueue_EnqueueBatchEx(&Handle->hTxQueue, (uint8_t*)format_buffer, strlen((char *)format_buffer));
}

void PAL_UART_DiscardInBuffer(PalUART_HandleTypeDef *Handle)
{
	if(Handle->Init.Rx.Enable == SET)
	{
		__disable_irq();
		Handle->LastestCh = '\0';
		Handle->LineSeparatorCounter1 = 0;
		Handle->LineSeparatorCounter2 = 0;
		PAL_ByteQueue_Clear(&Handle->hRxQueue);
		__enable_irq();
	}
}

static void RCC_USARTClockCmd(USART_TypeDef *USARTx, FunctionalState NewState)
{
	if(USARTx == USART1)
	{
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	}
	else if(USARTx == USART2)
	{
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	}
	else if(USARTx == USART3)
	{
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
	}
}
