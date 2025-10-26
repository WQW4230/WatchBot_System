/**
  ******************************************************************************
  * @file    pal_bin_codec.h
  * @author  铁头山羊stm32工作组
  * @version V1.0.0
  * @date    2023年2月24日
  * @brief   二进制数据包编解码驱动
  ******************************************************************************
  * @attention
  * Copyright (c) 2022 - 2023 东莞市明玉科技有限公司. 保留所有权力.
  ******************************************************************************
*/

#ifndef _PAL_BIN_CODEC_H_
#define _PAL_BIN_CODEC_H_

#include "stm32f10x.h"
#include "pal_crc.h"
#include "stm32f10x_pal_usart.h"

#define PAL_BIN_CODEC_MAX_ARG_LEN 64 // 数据包中最大的参数长度

typedef struct{
	PalCRC8_HandleTypeDef Crc8;
	uint8_t Stage;
	uint16_t ID;
	uint8_t BytesToRecv;
	uint8_t Data[PAL_BIN_CODEC_MAX_ARG_LEN];
	uint8_t Cursor; // 用于标记数据读出的当前进度
	uint8_t DataLength;
	FlagStatus DataCRCValid;
}PalBinDecoder_TypeDef;

typedef struct
{
	PalCRC8_HandleTypeDef Crc8;
	uint16_t ID;
	uint8_t  Data[PAL_BIN_CODEC_MAX_ARG_LEN];
	uint8_t DataLength;
}PalBinEncoder_TypeDef;

typedef struct
{
	PalUSART_HandleTypeDef *hUSART; // 串口句柄
} PalBinCodec_InitTypeDef;

typedef struct
{
	PalBinCodec_InitTypeDef Init;
	PalBinEncoder_TypeDef Encoder; // 编码器
	PalBinDecoder_TypeDef Decoder; // 解码器
}PalBinCodec_HandleTypeDef;

       void PAL_BinCodec_Init(PalBinCodec_HandleTypeDef *Handle);

       void PAL_BinCodec_StartSend(PalBinCodec_HandleTypeDef *Handle, uint16_t ID);
       void PAL_BinCodec_WriteByte(PalBinCodec_HandleTypeDef *Handle, uint8_t Data);
       void PAL_BinCodec_WriteBytes(PalBinCodec_HandleTypeDef *Handle, uint8_t *pData, uint16_t Size);
       void PAL_BinCodec_WriteInt8(PalBinCodec_HandleTypeDef *Handle, int8_t Data);
       void PAL_BinCodec_WriteInt16(PalBinCodec_HandleTypeDef *Handle, int16_t Data);
       void PAL_BinCodec_WriteInt32(PalBinCodec_HandleTypeDef *Handle, int32_t Data);
       void PAL_BinCodec_WriteInt64(PalBinCodec_HandleTypeDef *Handle, int64_t Data);
       void PAL_BinCodec_WriteUInt8(PalBinCodec_HandleTypeDef *Handle, uint8_t Data);
       void PAL_BinCodec_WriteUInt16(PalBinCodec_HandleTypeDef *Handle, uint16_t Data);
       void PAL_BinCodec_WriteUInt32(PalBinCodec_HandleTypeDef *Handle, uint32_t Data);
       void PAL_BinCodec_WriteUInt64(PalBinCodec_HandleTypeDef *Handle, uint64_t Data);
       void PAL_BinCodec_WriteFloat(PalBinCodec_HandleTypeDef *Handle, float Data);
       void PAL_BinCodec_WriteDouble(PalBinCodec_HandleTypeDef *Handle, double Data);
       void PAL_BinCodec_WriteString(PalBinCodec_HandleTypeDef *Handle, const char *Str);
       void PAL_BinCodec_EndSend(PalBinCodec_HandleTypeDef *Handle);

ErrorStatus PAL_BinCodec_Receive(PalBinCodec_HandleTypeDef *Handle, uint32_t Timeout);
   uint16_t PAL_BinCodec_GetID(PalBinCodec_HandleTypeDef *Handle);
    uint8_t PAL_BinCodec_GetDataLength(PalBinCodec_HandleTypeDef *Handle);
 FlagStatus PAL_BinCodec_GetDataCRC(PalBinCodec_HandleTypeDef *Handle);
    uint8_t PAL_BinCodec_ReadByte(PalBinCodec_HandleTypeDef *Handle);
       void PAL_BinCodec_ReadBytes(PalBinCodec_HandleTypeDef *Handle, uint8_t *pDataOut, uint16_t Size);
     int8_t PAL_BinCodec_ReadInt8(PalBinCodec_HandleTypeDef *Handle);
    int16_t PAL_BinCodec_ReadInt16(PalBinCodec_HandleTypeDef *Handle);
    int32_t PAL_BinCodec_ReadInt32(PalBinCodec_HandleTypeDef *Handle);
    int64_t PAL_BinCodec_ReadInt64(PalBinCodec_HandleTypeDef *Handle);
    uint8_t PAL_BinCodec_ReadUInt8(PalBinCodec_HandleTypeDef *Handle);
   uint16_t PAL_BinCodec_ReadUInt16(PalBinCodec_HandleTypeDef *Handle);
   uint32_t PAL_BinCodec_ReadUInt32(PalBinCodec_HandleTypeDef *Handle);
   uint64_t PAL_BinCodec_ReadUInt64(PalBinCodec_HandleTypeDef *Handle);
       void PAL_BinCodec_ReadString(PalBinCodec_HandleTypeDef *Handle, char *pStrOut, uint16_t Size);
      float PAL_BinCodec_ReadFloat(PalBinCodec_HandleTypeDef *Handle);
     double PAL_BinCodec_ReadDouble(PalBinCodec_HandleTypeDef *Handle);

#endif
