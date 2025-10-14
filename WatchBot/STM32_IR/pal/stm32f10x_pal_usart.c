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
#include "stm32f10x_pal_usart.h"
#include "stdio.h"
#include "string.h"
#include "stdarg.h"

static void PAL_USART_ClockCmd(USART_TypeDef *USARTx, FunctionalState NewState);
static ErrorStatus PAL_USART_TxPinInit(PalUSART_HandleTypeDef *Handle);
static ErrorStatus PAL_USART_RxPinInit(PalUSART_HandleTypeDef *Handle);
static uint8_t PAL_USART_GetIRQChannel(PalUSART_HandleTypeDef *Handle);
static void PAL_USART_OnRXNE(PalUSART_HandleTypeDef *Handle);
static void PAL_USART_OnTXE(PalUSART_HandleTypeDef *Handle);


//
// @简介：将句柄的初始化参数设置为默认值
//        USART_WordLength             = 8，     默认8位数据位
//        USART_StopBits               = 1，     默认1位停止位
//        USART_Parity                 = None，  默认不启用校验
//        USART_Mode                   = Tx|Rx， 默认收发双向
//        USART_IRQ_PreemptionPriority = 0，     默认抢占优先级=0
//        USART_IRQ_SubPriority        = 0，     默认子优先级=0
//        RxBufferSize                 = 128，   默认接收队列长度128
//        TxBufferSize                 = 128，   默认发送队列长度128
// @注意：仍需要手动填写的项目
//        USARTx                       - 所使用的USART接口的名称
//        BaudRate                     - 波特率
//
void PAL_USART_InitHandle(PalUSART_HandleTypeDef *Handle)
{
	Handle->Init.USART_WordLength = USART_WordLength_8b;
	Handle->Init.USART_StopBits = USART_StopBits_1;
	Handle->Init.USART_Parity = USART_Parity_No;
	Handle->Init.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	Handle->Init.USART_IRQ_PreemptionPriority = 0;
	Handle->Init.USART_IRQ_SubPriority = 0;
	Handle->Init.RxBufferSize = 128;
	Handle->Init.TxBufferSize = 128;
}

// @简介：USART初始化
// @参数：Handle - USART的句柄指针
// @返回值：初始化是否成功，SUCCESS表示成功，其它表示失败
ErrorStatus PAL_USART_Init(PalUSART_HandleTypeDef *Handle)
{
	USART_InitTypeDef USART_InitStruct;
	NVIC_InitTypeDef NVIC_InitStruct;
	
	Handle->LastestCh = '\0';
	Handle->LineSeparatorCounter1 = 0;
	Handle->LineSeparatorCounter2 = 0;
	
	Handle->TxCmd = (Handle->Init.USART_Mode & USART_Mode_Tx) != 0 ? ENABLE : DISABLE;
	Handle->RxCmd = (Handle->Init.USART_Mode & USART_Mode_Rx) != 0 ? ENABLE : DISABLE;
	
	// 1. 初始化IO引脚
	if(Handle->TxCmd == ENABLE) { PAL_USART_TxPinInit(Handle); } // 初始化Tx引脚
	if(Handle->RxCmd == ENABLE) { PAL_USART_RxPinInit(Handle); } // 初始化Rx引脚
	
	// 2. 使能USART的时钟
	USART_DeInit(Handle->Init.USARTx); // 复位
	PAL_USART_ClockCmd(Handle->Init.USARTx, ENABLE);
	
	// 3. 配置USART的参数
	USART_InitStruct.USART_BaudRate = Handle->Init.BaudRate;
	USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // 该模式下默认关闭流控
	USART_InitStruct.USART_Mode = 0;
	USART_InitStruct.USART_Mode = Handle->Init.USART_Mode;
	USART_InitStruct.USART_Parity = Handle->Init.USART_Parity;
	USART_InitStruct.USART_StopBits = Handle->Init.USART_StopBits;
	USART_InitStruct.USART_WordLength = Handle->Init.USART_WordLength;
	USART_Init(Handle->Init.USARTx, &USART_InitStruct);
	
	// 4. 配置中断
	// 配置NVIC
	NVIC_InitStruct.NVIC_IRQChannel = PAL_USART_GetIRQChannel(Handle);
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = Handle->Init.USART_IRQ_PreemptionPriority;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = Handle->Init.USART_IRQ_SubPriority;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);
	// 开启TXE中断
	if(Handle->TxCmd == ENABLE)
	{
		// 无需开启TXE中断
		// USART_ITConfig(Handle->Init.USARTx, USART_IT_TXE, ENABLE);
	}
	// 开启RXNE中断
	if(Handle->RxCmd == ENABLE)
	{
		USART_ITConfig(Handle->Init.USARTx, USART_IT_RXNE, ENABLE);
	}
	
	// 5. 初始化缓冲区
	// 发送缓冲区
	if(Handle->TxCmd == ENABLE && PAL_ByteQueue_Init(&Handle->hTxQueue, Handle->Init.TxBufferSize) != SUCCESS)
	{
		return ERROR;
	}
	// 接收缓冲区
	if(Handle->RxCmd == ENABLE && PAL_ByteQueue_Init(&Handle->hRxQueue, Handle->Init.RxBufferSize) != SUCCESS)
	{
		return ERROR;
	}
	
	// 6. 闭合USART总开关
	USART_Cmd(Handle->Init.USARTx, ENABLE);
	
	return SUCCESS;
}

// @简介：USART中断处理代码
// @参数：Handle - USART句柄的指针
// @返回值：空
// @注意：此方法应当在对应的USART的中断响应函数中被调用
void PAL_USART_IRQHandler(PalUSART_HandleTypeDef *Handle)
{
	// 处理RXNE中断
	if(USART_GetITStatus(Handle->Init.USARTx, USART_IT_RXNE) == SET)
	{
		PAL_USART_OnRXNE(Handle);
	}
	if(USART_GetITStatus(Handle->Init.USARTx, USART_IT_TXE) == SET)
	{
		PAL_USART_OnTXE(Handle);
	}
}

// @简介：发送一个字节
// @参数：Handle - USART句柄的指针
// @参数：Data - 要发送的字节
void PAL_USART_SendByte(PalUSART_HandleTypeDef *Handle, uint8_t Data)
{
	if(Handle->TxCmd == ENABLE)
	{
		USART_ITConfig(Handle->Init.USARTx, USART_IT_TXE, DISABLE);
		PAL_ByteQueue_EnqueueEx(&Handle->hTxQueue, Data); 
		USART_ITConfig(Handle->Init.USARTx, USART_IT_TXE, ENABLE);
	}
}

// @简介：发送一个字节
// @参数：Handle - USART句柄的指针
// @参数：c - 要发送的字符
void PAL_USART_PutChar(PalUSART_HandleTypeDef *Handle, char c)
{
	if(Handle->TxCmd == ENABLE)
	{
		USART_ITConfig(Handle->Init.USARTx, USART_IT_TXE, DISABLE);
		PAL_ByteQueue_EnqueueEx(&Handle->hTxQueue, c); 
		USART_ITConfig(Handle->Init.USARTx, USART_IT_TXE, ENABLE);
	}
}

// @简介：发送多个字节
// @参数：Handle - USART句柄的指针
// @参数：pData - 要发送的字节数组的指针
// @参数：Size - 要发送的数据个数
void PAL_USART_SendBytes(PalUSART_HandleTypeDef *Handle, const uint8_t* pData, uint16_t Size)
{
	if(Handle->TxCmd == ENABLE)
	{
		USART_ITConfig(Handle->Init.USARTx, USART_IT_TXE, DISABLE);
		PAL_ByteQueue_EnqueueBatchEx(&Handle->hTxQueue, pData, Size); 
		USART_ITConfig(Handle->Init.USARTx, USART_IT_TXE, ENABLE);
	}
}

// @简介：接收单个字节
// @参数：Handle - USART句柄的指针
// @参数：Timeout - 超时值，如果此参数填写PAL_MAX_DELAY则永不超时
// @返回值：接收到的字节，如果返回值小于0表示接收失败（超时）
int16_t PAL_USART_ReceiveByte(PalUSART_HandleTypeDef *Handle, uint32_t Timeout)
{
	if(Handle->RxCmd != ENABLE) return 0;
	
	int16_t ret = -1;
	uint8_t tmp;
	uint64_t expiredTime = PAL_GetTick() + Timeout;
	
	do
	{
		if(PAL_ByteQueue_GetLength(&Handle->hRxQueue) != 0)
		{
			USART_ITConfig(Handle->Init.USARTx, USART_IT_RXNE, DISABLE);
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
			USART_ITConfig(Handle->Init.USARTx, USART_IT_RXNE, ENABLE);
			break;
		}
	}
	while(Timeout == PAL_MAX_DELAY || PAL_GetTick() < expiredTime);
	
	return ret;
}

// @简介：接收多个字节
// @参数：Handle - USART句柄的指针
// @参数：pData - 缓冲区指针，用户需为接收此数据提供一个缓冲区，该参数为此缓冲区的指针
// @参数：Size  - 该缓冲区（pData）的最大长度
// @参数：Timeout - 超时值，单位毫秒。当超过此时间仍未接收到足够数量的数据时该方法提前返回
//                  当Timeout等于0时，无论是否接收到足够的数据，该方法立即返回；
//                  当Timeout等于PAL_MAX_Delay时，该方法永不超时
// @返回值：实际接收到的字节数量
uint16_t PAL_USART_ReceiveBytes(PalUSART_HandleTypeDef *Handle, uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
	uint16_t num_of_bytes_2_read;
	uint16_t queue_length;
	uint64_t expiredTime;
	
	if(Handle->RxCmd != ENABLE) return 0;
	
	if(Timeout != 0)
	{
		expiredTime = PAL_GetTick() + Timeout;
		
		while(( Timeout == PAL_MAX_DELAY || PAL_GetTick() < expiredTime) && PAL_ByteQueue_GetLength(&Handle->hRxQueue) < Size);
	}
	
	num_of_bytes_2_read = Size;
	
	USART_ITConfig(Handle->Init.USARTx, USART_IT_RXNE, DISABLE);
	
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
	
	USART_ITConfig(Handle->Init.USARTx, USART_IT_RXNE, ENABLE);
	
	return num_of_bytes_2_read;
}

// @简介：接收一行字符串
// @参数：Handle - USART句柄的指针
// @参数：pBuffer - 缓冲区指针，用户需为接收此数据提供一个缓冲区，该参数为此缓冲区的指针
// @参数：Size  - 该缓冲区（pData）的最大长度
// @参数：Timeout - 超时值，单位毫秒。当超过此时间仍未接收到足够数量的数据时该方法提前返回
//                  当Timeout等于0时，无论是否接收到足够的数据，该方法立即返回；
//                  当Timeout等于PAL_MAX_Delay时，该方法永不超时
// @返回值：实际接收到的字节数量
// @注意：有三种行分隔符可选，分别是LF(\n)，CR(\r)和CR+LF(\r\n)，可以在初始化时进行设置
uint16_t PAL_USART_ReadLine(PalUSART_HandleTypeDef *Handle, char *pBuffer, uint16_t Size, uint32_t Timeout)
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
			USART_ITConfig(Handle->Init.USARTx, USART_IT_RXNE, DISABLE);
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
				
				if((Handle->Init.Advanced.LineSeparator == LineSeparator_CRLF 
					&& previousByte == '\r' && currentByte == '\n')
				||(Handle->Init.Advanced.LineSeparator == LineSeparator_CR
					&& currentByte == '\r')
				||(Handle->Init.Advanced.LineSeparator == LineSeparator_LF
					&& currentByte == '\n'))
				{
					break;
				}
				
				previousByte = currentByte;
			}
			
			pBuffer[strlen] = '\0';
			USART_ITConfig(Handle->Init.USARTx, USART_IT_RXNE, ENABLE);
			
			break;
		}
	}
	while((Timeout == PAL_MAX_DELAY)||(PAL_GetTick() < expiredTime));
	
	return strlen;
}

// @简介：修改串口的波特率
// @参数：Handle - USART句柄的指针
// @参数：NewBaudrate - 新的波特率值
void PAL_USART_ChangeBaudrate(PalUSART_HandleTypeDef *Handle, uint32_t NewBaudrate)
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
	
	Handle->Init.BaudRate = NewBaudrate;
}

// @简介：返回当前接收缓冲区内待读取的数据量
// @参数：Handle - USART句柄的指针
// @返回值：待读取的数据量，以字节为单位
uint16_t PAL_USART_NumberOfBytesToRead(PalUSART_HandleTypeDef *Handle)
{
	uint16_t ret = 0;
	
	if(Handle->RxCmd != ENABLE) return 0;
	
	USART_ITConfig(Handle->Init.USARTx, USART_IT_RXNE, DISABLE);
	
	ret = PAL_ByteQueue_GetLength(&Handle->hRxQueue);
	
	USART_ITConfig(Handle->Init.USARTx, USART_IT_RXNE, ENABLE);
	
	return ret;
}

// @简介：通过串口发送字符换
// @参数：Handle - USART句柄的指针
// @参数：Str    - 要发送的字符串
void PAL_USART_SendString(PalUSART_HandleTypeDef *Handle, const char *Str)
{
	PAL_USART_SendBytes(Handle, (const uint8_t *)Str, strlen(Str));
}

// @简介：通过串口打印格式化字符串
// @参数：Handle - USART句柄的指针
// @参数：format - 格式字符串
// @参数：...    - 可变参数
// @注意：单次格式化的字符串长度不得超过256个
void PAL_USART_Printf(PalUSART_HandleTypeDef *Handle, const char* format, ...)
{
	uint8_t format_buffer[256];
	
	if(Handle->TxCmd != ENABLE) return;
	
	va_list argptr;
	__va_start(argptr, format);
	vsprintf((char* )format_buffer, format, argptr);
	__va_end(argptr);
	PAL_USART_SendBytes(Handle, format_buffer, strlen((char*)format_buffer));
}

// @简介：调用此方法可以清空接收缓冲区的内容
// @参数：Handle - USART句柄的指针
void PAL_USART_DiscardInBuffer(PalUSART_HandleTypeDef *Handle)
{
	if(Handle->RxCmd == ENABLE)
	{
		USART_ITConfig(Handle->Init.USARTx, USART_IT_RXNE, DISABLE);
		Handle->LastestCh = '\0';
		Handle->LineSeparatorCounter1 = 0;
		Handle->LineSeparatorCounter2 = 0;
		PAL_ByteQueue_Clear(&Handle->hRxQueue);
		USART_ITConfig(Handle->Init.USARTx, USART_IT_RXNE, ENABLE);
	}
}

// @简介：调用此方法可以清空发送缓冲区的内容
// @参数：Handle - USART句柄的指针
void PAL_USART_DiscardOutBuffer(PalUSART_HandleTypeDef *Handle)
{
	if(Handle->TxCmd == ENABLE)
	{
		USART_ITConfig(Handle->Init.USARTx, USART_IT_TXE, DISABLE);
		PAL_ByteQueue_Clear(&Handle->hTxQueue);
	}
}

// @简介：获取发送缓冲区的最高占用率
// @参数：Handle - USART的句柄指针
// @返回值：发送缓冲区的最高占用率，用百分数表示，范围0~100
uint16_t PAL_USART_GetTxBufferOccupancy(PalUSART_HandleTypeDef *Handle)
{
	uint16_t result = 0;
	
	USART_ITConfig(Handle->Init.USARTx, USART_IT_TXE, DISABLE);
	result = PAL_ByteQueue_GetOccupancy(&Handle->hTxQueue);
	USART_ITConfig(Handle->Init.USARTx, USART_IT_TXE, ENABLE);
	
	return result;
}

// @简介：获取接收缓冲区的最高占用率
// @参数：Handle - USART的句柄指针
// @返回值：接收缓冲区的最高占用率，用百分数表示，范围0~100
uint16_t PAL_USART_GetRxBufferOccupancy(PalUSART_HandleTypeDef *Handle)
{
	uint16_t result = 0;
	
	USART_ITConfig(Handle->Init.USARTx, USART_IT_RXNE, DISABLE);
	result = PAL_ByteQueue_GetOccupancy(&Handle->hRxQueue);
	USART_ITConfig(Handle->Init.USARTx, USART_IT_RXNE, ENABLE);
	
	return result;
}

// @简介：通过USART打印一行日志，日志采用以下格式
//        [hh:mm:ss.sss] 警告级别：详细信息 
// @参数：Handle - USART的句柄指针
// @参数：WarningLevel - 警告级别，可以取以下值之一：
//                          @WarningLevel_Info - 信息 
//                          @WarningLevel_Warning - 警告 
//                          @WarningLevel_Error - 错误
// @参数：Format       - 格式字符串，用于填写详细信息的格式
// @参数：...          - 可变参数
void PAL_USART_Log(PalUSART_HandleTypeDef *Handle, uint16_t WarningLevel, const char *Format, ...)
{
	uint64_t currentTick = PAL_GetTick();
	uint32_t h, m, s, ms;
	
	if(Handle->TxCmd != ENABLE) return;
	
	currentTick = PAL_GetTick();
	h = currentTick / 1000 / 60 /60;
	m = currentTick / 1000 / 60 % 60;
	s = currentTick / 1000 % 60;
	ms = currentTick % 1000;
	
	PAL_USART_SendString(Handle, "\r\n"); // 打印换行回车
	
	// 1. 显示时间
	PAL_USART_Printf(Handle, "[%02d:%02d:%02d.%03d]", h, m, s, ms); 
	PAL_USART_PutChar(Handle, ' '); // 打印空格
	
	// 2. 打印警告级别
	switch(WarningLevel)
  {
		case WarningLevel_Info:
			PAL_USART_SendString(Handle, "Info"); break;
		case WarningLevel_Warning:
			PAL_USART_SendString(Handle, "Warning");break;
		case WarningLevel_Error:
			PAL_USART_SendString(Handle, "Error"); break;
		default: break;
	}
	
	// 3. 打印详细信息
	uint8_t format_buffer[256];
	va_list argptr;
	
	PAL_USART_SendString(Handle, ": "); // 打印冒号和空格
	
	__va_start(argptr, Format);
	vsprintf((char* )format_buffer, Format, argptr);
	__va_end(argptr);
	PAL_USART_SendString(Handle, (const char *)format_buffer);
	
	PAL_USART_SendString(Handle, "\r\n"); // 打印换行回车
}

static void PAL_USART_ClockCmd(USART_TypeDef *USARTx, FunctionalState NewState)
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
	else if(USARTx == UART4)
	{
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);
	}
	else if(USARTx == UART5)
	{
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, ENABLE);
	}
}


static ErrorStatus PAL_USART_TxPinInit(PalUSART_HandleTypeDef *Handle)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_10MHz;
	
	if(Handle->Init.USARTx == USART1)
	{
		if(Handle->Init.Advanced.Remap == 0) // 无重映射
		{
			// PA9 -> USART_Tx
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
			GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;
			GPIO_Init(GPIOA, &GPIO_InitStruct);
		}
		else if(Handle->Init.Advanced.Remap == 1) // 完全重映射
		{
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
			GPIO_PinRemapConfig(GPIO_Remap_USART1, ENABLE);
			// PB6 -> USART_Tx
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
			GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6;
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
			// PA2 -> USART_Tx 复用推挽
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
			GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2;
			GPIO_Init(GPIOA, &GPIO_InitStruct);
		}
		else if(Handle->Init.Advanced.Remap == 1)
		{
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
			GPIO_PinRemapConfig(GPIO_Remap_USART2, ENABLE);
			// PD5 -> USART_Tx 复用推挽
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
			GPIO_InitStruct.GPIO_Pin = GPIO_Pin_5;
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
			// PB10 -> USART_Tx 复用推挽
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
			GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;
			GPIO_Init(GPIOB, &GPIO_InitStruct);
		}
		else if(Handle->Init.Advanced.Remap == 1)
		{
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
			GPIO_PinRemapConfig(GPIO_PartialRemap_USART3, ENABLE);
			// PC10 -> USART_Tx 复用推挽
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
			GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;
			GPIO_Init(GPIOC, &GPIO_InitStruct);
		}
		else if(Handle->Init.Advanced.Remap == 3)
		{
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
			GPIO_PinRemapConfig(GPIO_FullRemap_USART3, ENABLE);
			// PD8 -> USART_Tx 复用推挽
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
			GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8;
			GPIO_Init(GPIOD, &GPIO_InitStruct);
		}
		else
		{
			return ERROR;
		}
	}
	else if(Handle->Init.USARTx == UART4)
	{
		// PC10 -> USART_Tx 复用推挽
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
		GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;
		GPIO_Init(GPIOC, &GPIO_InitStruct);
	}
	else if(Handle->Init.USARTx == UART5)
	{
		// PC12 -> USART_Tx 复用推挽
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
		GPIO_InitStruct.GPIO_Pin = GPIO_Pin_12;
		GPIO_Init(GPIOC, &GPIO_InitStruct);
	}
	else
	{
		return ERROR;
	}
	return SUCCESS;
}

static ErrorStatus PAL_USART_RxPinInit(PalUSART_HandleTypeDef *Handle)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
	// 初始化Rx引脚
	if(Handle->Init.USARTx == USART1)
	{
		if(Handle->Init.Advanced.Remap == 0)
		{
			// PA10 -> USART_Rx 输入浮空
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
			GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;
			GPIO_Init(GPIOA, &GPIO_InitStruct);
		}
		else if(Handle->Init.Advanced.Remap == 1)
		{
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
			GPIO_PinRemapConfig(GPIO_Remap_USART1, ENABLE);
			// PB7 -> USART_Rx 输入浮空
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
			GPIO_InitStruct.GPIO_Pin = GPIO_Pin_7;
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
			// PA3 -> USART_Rx 输入浮空
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
			GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3;
			GPIO_Init(GPIOA, &GPIO_InitStruct);
		}
		else if(Handle->Init.Advanced.Remap == 1)
		{
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
			GPIO_PinRemapConfig(GPIO_Remap_USART2, ENABLE);
			// PD6 -> USART_Rx 输入浮空
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
			GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6;
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
			// PB11 -> USART_Rx 输入浮空
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
			GPIO_InitStruct.GPIO_Pin = GPIO_Pin_11;
			GPIO_Init(GPIOB, &GPIO_InitStruct);
		}
		else if(Handle->Init.Advanced.Remap == 1)
		{
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
			GPIO_PinRemapConfig(GPIO_PartialRemap_USART3, ENABLE);
			// PC11 -> USART_Rx 输入浮空
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
			GPIO_InitStruct.GPIO_Pin = GPIO_Pin_11;
			GPIO_Init(GPIOC, &GPIO_InitStruct);
		}
		else if(Handle->Init.Advanced.Remap == 3)
		{
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
			GPIO_PinRemapConfig(GPIO_FullRemap_USART3, ENABLE);
			// PD9 -> USART_Rx 输入浮空
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
  else if(Handle->Init.USARTx == UART4) // PC11
	{
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
		GPIO_InitStruct.GPIO_Pin = GPIO_Pin_11;
		GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
		GPIO_Init(GPIOC, &GPIO_InitStruct);
	}	
	else if(Handle->Init.USARTx == UART5) // PD2
	{
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
		GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2;
		GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
		GPIO_Init(GPIOD, &GPIO_InitStruct);
	}
	else 
	{
		return ERROR;
	}
	
	return SUCCESS;
}

static uint8_t PAL_USART_GetIRQChannel(PalUSART_HandleTypeDef *Handle)
{
	uint8_t result = 0;
	
	if(Handle->Init.USARTx == USART1)
	{
		result = 37;
	}
	else if(Handle->Init.USARTx == USART2)
	{
		result = 38;
	}
	else if(Handle->Init.USARTx == USART3)
	{
		result = 39;
	}
	else if(Handle->Init.USARTx == UART4)
	{
		result = 52;
	}
	else if(Handle->Init.USARTx == UART5)
	{
		result = 53;
	}
	
	return result;
}

static void PAL_USART_OnRXNE(PalUSART_HandleTypeDef *Handle)
{
	uint8_t byte_received;
	
	// 1. 接收数据
	byte_received = (uint8_t)USART_ReceiveData(Handle->Init.USARTx);
	
	// 2. 处理数据
	// 2.1. 处理换行
	if(Handle->Init.Advanced.LineSeparator != LineSeparator_Disable)
	{
		// 判断是否接收到了新行
		switch(Handle->Init.Advanced.LineSeparator)
		{
			case LineSeparator_CR:
			{
				if(byte_received == '\r' && Handle->hRxQueue.innerObjectQueue.Capcity > 1 )
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
				if(byte_received == '\n' && Handle->hRxQueue.innerObjectQueue.Capcity > 1)
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
				if(byte_received == '\n' && Handle->LastestCh == '\r' && Handle->hRxQueue.innerObjectQueue.Capcity > 2)
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
	
	Handle->LastestCh = byte_received;
	
	if(Handle->LineSeparatorCounter1 == 0)
	{
		Handle->LineSeparatorCounter2 = 0;
	}
	
	// 2.2. 将数据存储到缓冲区
	// 如果环形队列满，则自动删除掉最早的元素
	PAL_ByteQueue_EnqueueEx(&Handle->hRxQueue, byte_received);
}

static void PAL_USART_OnTXE(PalUSART_HandleTypeDef *Handle)
{
	uint8_t byteToSend;
	// 如果数据发送完成
	// 关闭TXE中断
	if(PAL_ByteQueue_GetLength(&Handle->hTxQueue) == 0)
	{
		USART_ITConfig(Handle->Init.USARTx, USART_IT_TXE, DISABLE);
	}
	else
	{
		// 从队列取出一个元素发送
		PAL_ByteQueue_Dequeue(&Handle->hTxQueue, &byteToSend);
		USART_SendData(Handle->Init.USARTx, byteToSend);
	}
}
