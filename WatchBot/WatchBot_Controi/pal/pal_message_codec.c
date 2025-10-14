/**
  ******************************************************************************
  * @file    pal_message_codec.c
  * @author  铁头山羊stm32工作组
  * @version V1.0.0
  * @date    2023年2月24日
  * @brief   二进制数据包编解码驱动
  ******************************************************************************
  * @attention
  * Copyright (c) 2022 - 2023 东莞市明玉科技有限公司. 保留所有权力.
  ******************************************************************************
*/

#include "pal_message_codec.h"
#include "pal_crc.h"
#include "stm32f10x_pal.h"

#define BINARY_PKT_SYNC ((uint8_t)0x53)

#define DECODER_STAGE_BEFORE_SYNC 0x00
#define DECODER_STAGE_ID_L        0x01 // ID低字节
#define DECODER_STAGE_ID_H        0x02 // ID高字节
#define DECODER_STAGE_DLC         0x03
#define DECODER_STAGE_HEADER_CRC  0x04
#define DECODER_STAGE_DATA        0x05
#define DECODER_STAGE_DATA_CRC    0x06

typedef struct{
	uint16_t ID;
	uint8_t DataLength; // 数据长度在0到64之间
	uint8_t Data[64];
	FlagStatus ValidFlag;
}PalMessage_TypeDef;

static void Decoder_Init(PalMessageDecoder_TypeDef *Decoder);
static void Encoder_Init(PalMessageEncoder_TypeDef *Encoder);
static FlagStatus PAL_MessageCodec_PutByteToDecoder(PalMessageCodec_HandleTypeDef *Handle, PalMessage_TypeDef *pRcvdPkt, uint8_t InputByte);

void PAL_MessageCodec_Init(PalMessageCodec_HandleTypeDef *Handle)
{
	Encoder_Init(&Handle->Encoder);
	Decoder_Init(&Handle->Decoder);
}

static void Encoder_Init(PalMessageEncoder_TypeDef *Encoder)
{
	Encoder->Crc8.Init.Initial = 0x00;
	Encoder->Crc8.Init.Polygon = 0x07;
	Encoder->Crc8.Init.ResultXOR = 0x00;
	Encoder->Crc8.Init.ReverseInput = DISABLE;
	Encoder->Crc8.Init.ReverseOutput = DISABLE;
	
	PAL_CRC8_Init(&Encoder->Crc8);
}

static void Decoder_Init(PalMessageDecoder_TypeDef *Decoder)
{
	Decoder->Stage = DECODER_STAGE_BEFORE_SYNC;
	
	Decoder->Crc8.Init.Initial = 0x00;
	Decoder->Crc8.Init.Polygon = 0x07;
	Decoder->Crc8.Init.ResultXOR = 0x00;
	Decoder->Crc8.Init.ReverseInput = DISABLE;
	Decoder->Crc8.Init.ReverseOutput = DISABLE;
	
	PAL_CRC8_Init(&Decoder->Crc8);
}

void PAL_MessageCodec_SendMessage(PalMessageCodec_HandleTypeDef *Handle, uint8_t ID, uint8_t *pArg, uint8_t ArgLength)
{
	uint8_t buffer[69]; // 最大长度64字节
	
	PAL_CRC8_Reset(&Handle->Encoder.Crc8);
	
	uint16_t pktSize = 0;
	uint8_t tmp;
	
	// SYNC 同步字节
	tmp = BINARY_PKT_SYNC;
	buffer[pktSize] = tmp; 
	PAL_CRC8_Input(&Handle->Encoder.Crc8, tmp);
	pktSize++;
	
	// ID字节1
	tmp = (uint8_t)(ID & 0xff); 
	buffer[pktSize] = tmp; 
	PAL_CRC8_Input(&Handle->Encoder.Crc8, tmp);
	pktSize++;
	
	// ID字节2
	tmp = (uint8_t)((ID>>8) & 0xff); 
	buffer[pktSize] = tmp; 
	PAL_CRC8_Input(&Handle->Encoder.Crc8, tmp);
	pktSize++;
	
	// DLC
	tmp = ArgLength;
	buffer[pktSize] = tmp; 
	PAL_CRC8_Input(&Handle->Encoder.Crc8, tmp);
	pktSize++;
	
	// Header CRC
	tmp = PAL_CRC8_GetResult(&Handle->Encoder.Crc8);
	buffer[pktSize] = tmp; 
	pktSize++;
	
	PAL_CRC8_Reset(&Handle->Encoder.Crc8);
	
	// 数据字节
	uint8_t i;
	
	for(i=0; i<ArgLength; i++)
	{
		tmp = pArg[i];
		buffer[pktSize] = tmp;
		PAL_CRC8_Input(&Handle->Encoder.Crc8, tmp);
		pktSize++;
	}
	
	// 数据字节的CRC
	tmp = PAL_CRC8_GetResult(&Handle->Encoder.Crc8); 
	buffer[pktSize] = tmp;
	pktSize++;
	
	// 使用串口发送数据
	if(Handle->Init.hUART->Init.Tx.Enable == SET)
	{
		if(Handle->Init.hUART->Init.Tx.DMA.Enable == SET)
		{
			PAL_UART_SendBytes_DMA(Handle->Init.hUART, buffer, pktSize);
		}
		else
		{
			PAL_UART_SendBytes(Handle->Init.hUART, buffer, pktSize);
		}
	}
}

FlagStatus PAL_MessageCodec_ReceiveMessage(PalMessageCodec_HandleTypeDef *Handle, uint16_t *pIDOut, uint8_t *pArgOut, uint8_t *pArgLength, uint32_t Timeout)
{
	uint64_t expiredTime;
	
	// 如果串口未开启接收功能，直接返回
	if(Handle->Init.hUART->Init.Rx.Enable != SET)
	{
		return RESET; 
	}
	
	// 计算超时时间
	if(Timeout == PAL_MAX_DELAY)
	{
		expiredTime = 0xffffffffffffffff; // 永不超时
	}
	else
	{
		expiredTime = PAL_GetTick() + Timeout;
	}
	
	// 开始接收消息
	PalMessage_TypeDef msg;
	uint8_t rcvd_flag = 0;
	int16_t rx;
	
	do
	{
		while((rx = PAL_UART_RecieveByte(Handle->Init.hUART, 0)) >= 0)
		{			
			// 已接收到消息
			if(PAL_MessageCodec_PutByteToDecoder(Handle, &msg, (uint8_t)rx) == SET)
			{
				rcvd_flag = 1; // 表示已经收到消息
				break;
			}
		}
		if(rcvd_flag)
		{
			break;
		}
	}while(PAL_GetTick() < expiredTime);
	
	if(rcvd_flag)
	{
		*pIDOut = msg.ID;
		*pArgLength = msg.DataLength;
		uint16_t i;
		for(i=0;i<msg.DataLength;i++)
		{
			pArgOut[i] = msg.Data[i];
		}
		return SET;
	}
	else
	{
		return RESET;
	}
}

void PAL_MessageCodec_DiscardInputBuffer(PalMessageCodec_HandleTypeDef *Handle)
{
	PAL_UART_DiscardInBuffer(Handle->Init.hUART);
	Handle->Decoder.BytesToRcv = 0;
	PAL_CRC8_Reset(&Handle->Decoder.Crc8);
	Handle->Decoder.Stage = DECODER_STAGE_BEFORE_SYNC;
}

static FlagStatus PAL_MessageCodec_PutByteToDecoder(PalMessageCodec_HandleTypeDef *Handle, PalMessage_TypeDef *pRcvdPkt, uint8_t InputByte)
{
	FlagStatus ret = RESET;
	
	switch(Handle->Decoder.Stage)
	{
		case DECODER_STAGE_BEFORE_SYNC: // 等待SYNC
		{
			if(InputByte == BINARY_PKT_SYNC)
			{
				PAL_CRC8_Reset(&Handle->Decoder.Crc8);
				PAL_CRC8_Input(&Handle->Decoder.Crc8, InputByte);
				Handle->Decoder.Stage = DECODER_STAGE_ID_L;
			}
			break;
		}
		case DECODER_STAGE_ID_L:
		{
			Handle->Decoder.ID = InputByte;
			
			PAL_CRC8_Input(&Handle->Decoder.Crc8, InputByte);
			
			Handle->Decoder.Stage = DECODER_STAGE_ID_H;
			break;
		}
		case DECODER_STAGE_ID_H:
		{
			Handle->Decoder.ID |= ((uint16_t)InputByte) << 8;
			
			PAL_CRC8_Input(&Handle->Decoder.Crc8, InputByte);
			
			Handle->Decoder.Stage = DECODER_STAGE_DLC;
			break;
		}
		case DECODER_STAGE_DLC:
		{
			Handle->Decoder.DataLength = InputByte;
			
			PAL_CRC8_Input(&Handle->Decoder.Crc8, InputByte);
			
			Handle->Decoder.Stage = DECODER_STAGE_HEADER_CRC;
			break;
		}
		case DECODER_STAGE_HEADER_CRC:
		{
			if(InputByte == PAL_CRC8_GetResult(&Handle->Decoder.Crc8))
			{
				if(Handle->Decoder.DataLength == 0)
				{
					pRcvdPkt->DataLength = 0;
					pRcvdPkt->ID = Handle->Decoder.ID;
					pRcvdPkt->ValidFlag = SET;
					
					ret = SET;
					Handle->Decoder.Stage = DECODER_STAGE_BEFORE_SYNC;
				}
				else
				{
					Handle->Decoder.BytesToRcv = Handle->Decoder.DataLength;
					Handle->Decoder.Stage = DECODER_STAGE_DATA;
					PAL_CRC8_Reset(&Handle->Decoder.Crc8);
				}
			}
			else
			{
				ret = RESET; // 消息头无效（CRC校验出错）
				Handle->Decoder.Stage = DECODER_STAGE_BEFORE_SYNC;
			}
			break;
		}
		case DECODER_STAGE_DATA:
		{
			Handle->Decoder.Data[Handle->Decoder.DataLength - Handle->Decoder.BytesToRcv] = InputByte;
			PAL_CRC8_Input(&Handle->Decoder.Crc8, InputByte);
			Handle->Decoder.BytesToRcv--;
			if(Handle->Decoder.BytesToRcv == 0)
			{
				Handle->Decoder.Stage = DECODER_STAGE_DATA_CRC;
			}
			break;
		}
		case DECODER_STAGE_DATA_CRC:
		{
			if(PAL_CRC8_GetResult(&Handle->Decoder.Crc8) == InputByte)
			{
				Handle->Decoder.DataCRCValid = SET;
			}
			else
			{
				Handle->Decoder.DataCRCValid = RESET;
			}
			ret = SET;
			
			pRcvdPkt->ID = Handle->Decoder.ID;
			pRcvdPkt->DataLength = Handle->Decoder.DataLength;
			pRcvdPkt->ValidFlag = Handle->Decoder.DataCRCValid;
			
			uint8_t i;
			
			for(i=0;i<Handle->Decoder.DataLength;i++)
			{
				pRcvdPkt->Data[i] = Handle->Decoder.Data[i];
			}
			
			Handle->Decoder.Stage = DECODER_STAGE_BEFORE_SYNC;
			break;
		}
		default:
			break;
	}
	
	return ret;
}
