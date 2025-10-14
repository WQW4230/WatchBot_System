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
#ifndef _STM32F10x_PAL_USART_H_
#define _STM32F10x_PAL_USART_H_

#include "stm32f10x.h"
#include "stm32f10x_pal.h"
#include "pal_byte_queue.h"

#define LineSeparator_Disable 0x00 // 禁止行划分
#define LineSeparator_CR 0x01 // 回车 Carriage Return \r
#define LineSeparator_LF 0x02 // 换行 Line Feed \n
#define LineSeparator_CRLF 0x03 // 换行回车 \r\n

#define WarningLevel_Info 0x00
#define WarningLevel_Warning 0x01
#define WarningLevel_Error 0x02

/* USART的高级设置 */
typedef struct
{
	uint32_t Remap; // 复用功能重映射的值，取值范围见参考手册
	uint16_t LineSeparator; // 行分隔符
	                        // 可选以下几个值之一：
	                        //	@LineSeparator_CR - 回车\r
                          //  @LineSeparator_LF 0x01 - 换行\n
                          //  @LineSeparator_CRLF 0x02 - 回车+换行\r\n 
}PalUSART_AdvancedInitTypeDef;

/* USART初始化参数 */
typedef struct
{
	USART_TypeDef* USARTx; /* 使用的USART接口，目前支持USART1, USART2和USART3 */
	uint32_t BaudRate; // 波特率
	uint16_t USART_WordLength; // 数据位长度
	                           // 可设置为以下值之一：
	                           //  @USART_WordLength_8b - 8位数据位
	                           //  @USART_WordLength_9b - 9位数据位*/
	uint16_t USART_Parity; // 奇偶校验
	                       // 可设置为以下值之一：
	                       //   @USART_Parity_No - 无奇偶校验
	                       //   @USART_Parity_Even - 偶校验
	                       //   @USART_Parity_Odd - 奇校验
	uint16_t USART_StopBits; // 停止位
	                         // 可设置为以下值之一：
	                         //   @USART_StopBits_1 - 1位停止位
	                         //   @USART_StopBits_0_5 - 0.5位停止位
	                         //   @USART_StopBits_2 - 2位停止位
	                         //   @USART_StopBits_1_5 - 1.5位停止位
	uint16_t USART_Mode; // 模式（通信方向）
	                     // 可以设置为以下值之一
	                     //   @USART_Mode_Tx - 只发送
	                     //   @USART_Mode_Rx - 只接收
											 //   @USART_Mode_Tx | USART_Mode_Rx - 收发双向
	uint8_t USART_IRQ_PreemptionPriority; // USART中断的抢占优先级
	uint8_t USART_IRQ_SubPriority; // USART中断的子优先级
	uint16_t TxBufferSize; // 发送缓冲区大小
	uint16_t RxBufferSize; // 接收缓冲区大小
	PalUSART_AdvancedInitTypeDef Advanced; // 高级设置
}PalUSART_InitTypeDef;

/* 句柄数据类型定义 */
typedef struct
{
	PalUSART_InitTypeDef Init;
	FunctionalState TxCmd;
	FunctionalState RxCmd;
	PalByteQueue_HandleTypeDef hTxQueue; // 发送缓冲区
	PalByteQueue_HandleTypeDef hRxQueue; // 接收缓冲区
	uint16_t LineSeparatorCounter1; /* 当LineSeparatorCounter1=0时分隔符会因缓冲区满而被（部分）丢弃
	                                   当LineSeparatorCounter1=1时，缓冲区仅剩分隔符（完整）*/
	uint16_t LineSeparatorCounter2; /* 当LineSeparatorCounter2=0时标志着最后一行被完全读出 */
	uint8_t LastestCh;
}PalUSART_HandleTypeDef;

       void PAL_USART_InitHandle(PalUSART_HandleTypeDef *Handle);
ErrorStatus PAL_USART_Init(PalUSART_HandleTypeDef *Handle);
       void PAL_USART_IRQHandler(PalUSART_HandleTypeDef *Handle);
       void PAL_USART_ChangeBaudrate(PalUSART_HandleTypeDef *Handle, uint32_t NewBaudrate);
   uint16_t PAL_USART_GetTxBufferOccupancy(PalUSART_HandleTypeDef *Handle);
   uint16_t PAL_USART_GetRxBufferOccupancy(PalUSART_HandleTypeDef *Handle);
       void PAL_USART_Log(PalUSART_HandleTypeDef *Handle, uint16_t WarningLevel, const char *Format, ...);

       void PAL_USART_DiscardOutBuffer(PalUSART_HandleTypeDef *Handle);
       void PAL_USART_SendByte(PalUSART_HandleTypeDef *Handle, uint8_t Data);
       void PAL_USART_SendBytes(PalUSART_HandleTypeDef *Handle, const uint8_t* pData, uint16_t Size);
       void PAL_USART_PutChar(PalUSART_HandleTypeDef *Handle, char c);
       void PAL_USART_SendString(PalUSART_HandleTypeDef *Handle, const char *Str);
       void PAL_USART_Printf(PalUSART_HandleTypeDef *Handle, const char* format, ...);

       void PAL_USART_DiscardInBuffer(PalUSART_HandleTypeDef *Handle);
    int16_t PAL_USART_ReceiveByte(PalUSART_HandleTypeDef *Handle, uint32_t Timeout);
   uint16_t PAL_USART_ReceiveBytes(PalUSART_HandleTypeDef *Handle, uint8_t *pData, uint16_t Size, uint32_t Timeout);
   uint16_t PAL_USART_NumberOfBytesToRead(PalUSART_HandleTypeDef *Handle);
   uint16_t PAL_USART_ReadLine(PalUSART_HandleTypeDef *Handle, char *pBuffer, uint16_t Size, uint32_t Timeout);

#endif
