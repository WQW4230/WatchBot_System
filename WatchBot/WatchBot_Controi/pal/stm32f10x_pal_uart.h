/**
  ******************************************************************************
  * @file    stm32f10x_pal_uart.h
  * @author  铁头山羊stm32工作组
  * @version V1.0.0
  * @date    2023年1月7日
  * @brief   串口驱动
  ******************************************************************************
  * @attention
  * Copyright (c) 2022 - 2023 东莞市明玉科技有限公司. 保留所有权力.
  ******************************************************************************
	*/
#ifndef _STM32F10x_PAL_UART_H_
#define _STM32F10x_PAL_UART_H_

#include "stm32f10x.h"
#include "pal_byte_queue.h"

#define LineSeparator_Disable 0x00 // 禁止行划分
#define LineSeparator_CR 0x01 // 回车 Carriage Return \r
#define LineSeparator_LF 0x02 // 换行 Line Feed \n
#define LineSeparator_CRLF 0x03 // 换行回车 \r\n

/* UART的基本初始化参数 */
typedef struct
{
	uint32_t BaudRate; // 波特率
	uint16_t USART_Parity; // 奇偶校验，可设置为以下值之一：
	                       //   @USART_Parity_No - 无奇偶校验
	                       //   @USART_Parity_Even - 偶校验
	                       //   @USART_Parity_Odd - 奇校验
	uint16_t USART_StopBits; // 停止位，可设置为以下值之一：
	                         //   @USART_StopBits_1 - 1位停止位
	                         //   @USART_StopBits_0_5 - 0.5位停止位
	                         //   @USART_StopBits_2 - 2位停止位
	                         //   @USART_StopBits_1_5 - 1.5位停止位
	uint16_t USART_WordLength; // 数据位长度，可设置为以下值之一：
	                           //  @USART_WordLength_8b - 8位数据位
	                           //  @USART_WordLength_9b - 9位数据位*/
}PalUART_BasicParaTypeDef;

/* 负责UART发送的DMA初始化参数 */
typedef struct
{
	FlagStatus Enable; // DMA开关，该DMA用于数据发送
	uint16_t QueueSize; // 发送缓冲区大小，一般设置256字节，可根据需求自行调整
}PalUART_DMATxParaTypeDef;

/* UART发送的初始化参数 */
typedef struct
{
	FlagStatus Enable; // 数据发送开关
	PalUART_DMATxParaTypeDef DMA; // DMA设置
}PalUART_TxParaTypeDef;

/* UART接收的初始化参数 */
typedef struct
{
	FlagStatus Enable; // 是否需要用UART接收数据
	uint16_t QueueSize; // 接收缓冲区大小，一般设置256字节，可根据需求自行调整
	uint8_t USART_IRQ_PreemptionPriority; // USART中断的抢占优先级，仅对数据接收有效
	uint8_t USART_IRQ_SubPriority; // USART中断的子优先级，仅对数据接收有效
	uint16_t LineSeparator; // 行分隔符，可选以下几个值之一
                          //	@LineSeparator_CR - 回车\r
                          //  @LineSeparator_LF 0x01 - 换行\n
                          //  @LineSeparator_CRLF 0x02 - 回车+换行\r\n 
}PalUART_RxParaTypeDef;

/* UART的高级设置 */
typedef struct
{
	uint32_t Remap; // IO重映射的值，取值范围见参考手册
}PalUART_AdvancedInitTypeDef;

/* UART初始化参数 */
typedef struct
{
	USART_TypeDef* USARTx; /* 使用的USART接口，目前支持USART1, USART2和USART3 */
	PalUART_BasicParaTypeDef Basic; // 基本参数设置
	PalUART_TxParaTypeDef Tx; // 发送参数设置
	PalUART_RxParaTypeDef Rx; // 接收参数设置
	PalUART_AdvancedInitTypeDef Advanced; // 高级设置
}PalUART_InitTypeDef;

/* 句柄数据类型定义 */
typedef struct
{
	PalUART_InitTypeDef Init;
	PalByteQueue_HandleTypeDef hTxQueue; // 发送缓冲区
	PalByteQueue_HandleTypeDef hRxQueue; // 接收缓冲区
	DMA_Channel_TypeDef* DMATxChannel;
	uint8_t DMATxBuffer[256];
	uint16_t LineSeparatorCounter1; /* 当LineSeparatorCounter1=0时分隔符会因缓冲区满而被（部分）丢弃
	                                   当LineSeparatorCounter1=1时，缓冲区仅剩分隔符（完整）*/
	uint16_t LineSeparatorCounter2; /* 当LineSeparatorCounter2=0时标志着最后一行被完全读出 */
	uint8_t LastestCh;
}PalUART_HandleTypeDef;

ErrorStatus PAL_UART_Init(PalUART_HandleTypeDef *Handle);
void PAL_UART_UART_IRQHandler(PalUART_HandleTypeDef *Handle);
void PAL_UART_SendByte(PalUART_HandleTypeDef *Handle, uint8_t Data);
void PAL_UART_SendBytes(PalUART_HandleTypeDef *Handle, const uint8_t* pData, uint16_t Size);
void PAL_UART_Printf(PalUART_HandleTypeDef *Handle, const char* format, ...);

void PAL_UART_SendByte_DMA(PalUART_HandleTypeDef *Handle, uint8_t Data);
void PAL_UART_SendBytes_DMA(PalUART_HandleTypeDef *Handle, const uint8_t* pData, uint16_t Size);
void PAL_UART_Printf_DMA(PalUART_HandleTypeDef *Handle, const char* format, ...);
void PAL_UART_FlushDMA(PalUART_HandleTypeDef *Handle);

void PAL_UART_DiscardInBuffer(PalUART_HandleTypeDef *Handle);
int16_t PAL_UART_RecieveByte(PalUART_HandleTypeDef *Handle, uint32_t Timeout);
uint16_t PAL_UART_RecieveBytes(PalUART_HandleTypeDef *Handle, uint8_t *pData, uint16_t Size, uint32_t Timeout);
uint16_t PAL_UART_NumberOfBytesToRead(PalUART_HandleTypeDef *Handle);
uint16_t PAL_UART_ReadLine(PalUART_HandleTypeDef *Handle, char *pBuffer, uint16_t Size, uint32_t Timeout);
void PAL_UART_ChangeBaudrate(PalUART_HandleTypeDef *Handle, uint32_t NewBaudrate);
#endif
