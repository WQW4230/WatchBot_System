/**
  ******************************************************************************
  * @file    pal_message_codec.h
  * @author  铁头山羊stm32工作组
  * @version V1.0.0
  * @date    2023年2月24日
  * @brief   二进制数据包编解码驱动
  ******************************************************************************
  * @attention
  * Copyright (c) 2022 - 2023 东莞市明玉科技有限公司. 保留所有权力.
  ******************************************************************************
*/

#ifndef _PAL_MESSAGE_CODEC_H_
#define _PAL_MESSAGE_CODEC_H_

#include "stm32f10x.h"
#include "pal_crc.h"
#include "stm32f10x_pal_uart.h"

typedef struct{
	PalCRC8_HandleTypeDef Crc8;
	
	uint8_t Stage;
	uint16_t ID;
	uint8_t  DataLength;
	uint8_t BytesToRcv;
	uint8_t Data[64];
	FlagStatus DataCRCValid;
}PalMessageDecoder_TypeDef;

typedef struct
{
	PalCRC8_HandleTypeDef Crc8;
}PalMessageEncoder_TypeDef;

typedef struct
{
	PalUART_HandleTypeDef *hUART; // 串口句柄
} PalMessageCodec_InitTypeDef;

typedef struct
{
	PalMessageCodec_InitTypeDef Init;
	PalMessageEncoder_TypeDef Encoder; // 编码器
	PalMessageDecoder_TypeDef Decoder; // 解码器
}PalMessageCodec_HandleTypeDef;

      void PAL_MessageCodec_Init(PalMessageCodec_HandleTypeDef *Handle);
      void PAL_MessageCodec_SendMessage(PalMessageCodec_HandleTypeDef *Handle, uint8_t ID, uint8_t *pArg, uint8_t ArgLength);
      void PAL_MessageCodec_DiscardInputBuffer(PalMessageCodec_HandleTypeDef *Handle);
FlagStatus PAL_MessageCodec_ReceiveMessage(PalMessageCodec_HandleTypeDef *Handle, uint16_t *pIDOut, uint8_t *pArgOut, uint8_t *pArgLength, uint32_t Timeout);

#endif
