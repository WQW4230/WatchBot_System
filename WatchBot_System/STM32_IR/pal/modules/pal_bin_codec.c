/**
  ******************************************************************************
  * @file    pal_bin_codec.c
  * @author  铁头山羊stm32工作组
  * @version V1.0.0
  * @date    2023年2月24日
  * @brief   二进制数据包编解码驱动
  ******************************************************************************
  * @attention
  * Copyright (c) 2022 - 2023 东莞市明玉科技有限公司. 保留所有权力.
  ******************************************************************************
*/

#include "pal_bin_codec.h"
#include "pal_crc.h"
#include "stm32f10x_pal.h"
#include <string.h>

#define BIN_MESSAGE_PKT_SYNC ((uint8_t)0x5a)

#define DECODER_STAGE_BEFORE_SYNC 0x00
#define DECODER_STAGE_ID_L        0x01 // ID低字节
#define DECODER_STAGE_ID_H        0x02 // ID高字节
#define DECODER_STAGE_DLC         0x03 // 数据长度
#define DECODER_STAGE_HEADER_CRC  0x04 // 包头校验
#define DECODER_STAGE_DATA        0x05 // 数据
#define DECODER_STAGE_DATA_CRC    0x06 // 数据校验

static void Decoder_Init(PalBinDecoder_TypeDef *Decoder);
static void Encoder_Init(PalBinEncoder_TypeDef *Encoder);
static ErrorStatus PAL_BinCodec_PutByteToDecoder(PalBinCodec_HandleTypeDef *Handle, uint8_t InputByte);

//
// @简介：初始化二进制消息编解码器
// @参数：Handle - 编解码器的句柄
// 
void PAL_BinCodec_Init(PalBinCodec_HandleTypeDef *Handle)
{
	Encoder_Init(&Handle->Encoder);
	Decoder_Init(&Handle->Decoder);
}

//
// @简介：开始发送数据包
// @参数：Handle - 编解码器句柄
// @参数：ID - 要发送的消息的ID
// 
void PAL_BinCodec_StartSend(PalBinCodec_HandleTypeDef *Handle, uint16_t ID)
{
	// 复位Encoder参数
	Handle->Encoder.DataLength = 0;
	
	// 设置消息ID
	Handle->Encoder.ID = ID;
}

//
// @简介：向参数中增加1个字节
// @参数：Handle - 编解码器的句柄
// @参数：Data - 要写入的字节
// 
void PAL_BinCodec_WriteByte(PalBinCodec_HandleTypeDef *Handle, uint8_t Data)
{
	if(Handle->Encoder.DataLength < PAL_BIN_CODEC_MAX_ARG_LEN) // 防止数组越界
	{
		Handle->Encoder.Data[Handle->Encoder.DataLength++] = Data;
	}
}

//
// @简介：向参数中增加多个字节
// @参数：Handle - 编解码器的句柄
// @参数：pData  - 要写入的字节数组
// @参数：Size   - 字节数组的长度
// 
void PAL_BinCodec_WriteBytes(PalBinCodec_HandleTypeDef *Handle, uint8_t *pData, uint16_t Size)
{
	uint16_t i;
	for(i=0;i<Size;i++)
	{
		if(Handle->Encoder.DataLength < PAL_BIN_CODEC_MAX_ARG_LEN)
		{
			Handle->Encoder.Data[Handle->Encoder.DataLength++] = pData[i];
		}
		else
		{
			break; // 防止数组越界
		}
	}
}

// 
// @简介：向Encoder写入int8数据
// @参数：Handle - 编解码器句柄
// @参数：Data   - 要写入的数据
//
void PAL_BinCodec_WriteInt8(PalBinCodec_HandleTypeDef *Handle, int8_t Data)
{
	if(Handle->Encoder.DataLength < PAL_BIN_CODEC_MAX_ARG_LEN) // 防止数组越界
	{
		Handle->Encoder.Data[Handle->Encoder.DataLength++] = *((uint8_t *)&Data);
	}
}

// 
// @简介：向Encoder写入int16数据
// @参数：Handle - 编解码器句柄
// @参数：Data   - 要写入的数据
//
void PAL_BinCodec_WriteInt16(PalBinCodec_HandleTypeDef *Handle, int16_t Data)
{
	uint16_t i;
	for(i=0;i<sizeof(int16_t);i++)
	{
		if(Handle->Encoder.DataLength < PAL_BIN_CODEC_MAX_ARG_LEN)
		{
			Handle->Encoder.Data[Handle->Encoder.DataLength++] = ((uint8_t*)&Data)[i];
		}
		else
		{
			break; // 防止数组越界
		}
	}
}

// 
// @简介：向Encoder写入int32数据
// @参数：Handle - 编解码器句柄
// @参数：Data   - 要写入的数据
//
void PAL_BinCodec_WriteInt32(PalBinCodec_HandleTypeDef *Handle, int32_t Data)
{
	uint16_t i;
	for(i=0;i<sizeof(int32_t);i++)
	{
		if(Handle->Encoder.DataLength < PAL_BIN_CODEC_MAX_ARG_LEN)
		{
			Handle->Encoder.Data[Handle->Encoder.DataLength++] = ((uint8_t*)&Data)[i];
		}
		else
		{
			break; // 防止数组越界
		}
	}
}

// 
// @简介：向Encoder写入int64数据
// @参数：Handle - 编解码器句柄
// @参数：Data   - 要写入的数据
//
void PAL_BinCodec_WriteInt64(PalBinCodec_HandleTypeDef *Handle, int64_t Data)
{
	uint16_t i;
	for(i=0;i<sizeof(int64_t);i++)
	{
		if(Handle->Encoder.DataLength < PAL_BIN_CODEC_MAX_ARG_LEN)
		{
			Handle->Encoder.Data[Handle->Encoder.DataLength++] = ((uint8_t*)&Data)[i];
		}
		else
		{
			break; // 防止数组越界
		}
	}
}

// 
// @简介：向Encoder写入uint8数据
// @参数：Handle - 编解码器句柄
// @参数：Data   - 要写入的数据
//
void PAL_BinCodec_WriteUInt8(PalBinCodec_HandleTypeDef *Handle, uint8_t Data)
{
	if(Handle->Encoder.DataLength < PAL_BIN_CODEC_MAX_ARG_LEN) // 防止数组越界
	{
		Handle->Encoder.Data[Handle->Encoder.DataLength++] = Data;
	}
}

// 
// @简介：向Encoder写入uint16数据
// @参数：Handle - 编解码器句柄
// @参数：Data   - 要写入的数据
//
void PAL_BinCodec_WriteUInt16(PalBinCodec_HandleTypeDef *Handle, uint16_t Data)
{
	uint16_t i;
	for(i=0;i<sizeof(uint16_t);i++)
	{
		if(Handle->Encoder.DataLength < PAL_BIN_CODEC_MAX_ARG_LEN)
		{
			Handle->Encoder.Data[Handle->Encoder.DataLength++] = ((uint8_t*)&Data)[i];
		}
		else
		{
			break; // 防止数组越界
		}
	}
}

// 
// @简介：向Encoder写入uint32数据
// @参数：Handle - 编解码器句柄
// @参数：Data   - 要写入的数据
//
void PAL_BinCodec_WriteUInt32(PalBinCodec_HandleTypeDef *Handle, uint32_t Data)
{
	uint16_t i;
	for(i=0;i<sizeof(uint32_t);i++)
	{
		if(Handle->Encoder.DataLength < PAL_BIN_CODEC_MAX_ARG_LEN)
		{
			Handle->Encoder.Data[Handle->Encoder.DataLength++] = ((uint8_t*)&Data)[i];
		}
		else
		{
			break; // 防止数组越界
		}
	}
}

// 
// @简介：向Encoder写入uint64数据
// @参数：Handle - 编解码器句柄
// @参数：Data   - 要写入的数据
//
void PAL_BinCodec_WriteUInt64(PalBinCodec_HandleTypeDef *Handle, uint64_t Data)
{
	uint16_t i;
	for(i=0;i<sizeof(uint64_t);i++)
	{
		if(Handle->Encoder.DataLength < PAL_BIN_CODEC_MAX_ARG_LEN)
		{
			Handle->Encoder.Data[Handle->Encoder.DataLength++] = ((uint8_t*)&Data)[i];
		}
		else
		{
			break; // 防止数组越界
		}
	}
}

// 
// @简介：向Encoder写入float数据
// @参数：Handle - 编解码器句柄
// @参数：Data   - 要写入的数据
//
void PAL_BinCodec_WriteFloat(PalBinCodec_HandleTypeDef *Handle, float Data)
{
	uint16_t i;
	for(i=0;i<sizeof(float);i++)
	{
		if(Handle->Encoder.DataLength < PAL_BIN_CODEC_MAX_ARG_LEN)
		{
			Handle->Encoder.Data[Handle->Encoder.DataLength++] = ((uint8_t*)&Data)[i];
		}
		else
		{
			break; // 防止数组越界
		}
	}
}

// 
// @简介：向Encoder写入double数据
// @参数：Handle - 编解码器句柄
// @参数：Data   - 要写入的数据
//
void PAL_BinCodec_WriteDouble(PalBinCodec_HandleTypeDef *Handle, double Data)
{
	uint16_t i;
	for(i=0;i<sizeof(double);i++)
	{
		if(Handle->Encoder.DataLength < PAL_BIN_CODEC_MAX_ARG_LEN)
		{
			Handle->Encoder.Data[Handle->Encoder.DataLength++] = ((uint8_t*)&Data)[i];
		}
		else
		{
			break; // 防止数组越界
		}
	}
}

// 
// @简介：向Encoder写入字符串
// @参数：Handle - 编解码器句柄
// @参数：Str    - 字符串
//
void PAL_BinCodec_WriteString(PalBinCodec_HandleTypeDef *Handle, const char *Str)
{
	uint16_t i;
	for(i=0;i<strlen(Str);i++)
	{
		if(Handle->Encoder.DataLength < PAL_BIN_CODEC_MAX_ARG_LEN - 1)
		{
			Handle->Encoder.Data[Handle->Encoder.DataLength++] = Str[i];
		}
		else
		{
			break; // 防止数组越界
		}
	}
	// 增加字符串结束符 ( Terminate string with '\0' )
	if(Handle->Encoder.DataLength < PAL_BIN_CODEC_MAX_ARG_LEN)
	{
		Handle->Encoder.Data[Handle->Encoder.DataLength++] = '\0';
	}
}

//
// @简介：结束发送，调用该编程接口后消息立即被发送出去
// @参数：Handle - 编解码器的句柄
//
void PAL_BinCodec_EndSend(PalBinCodec_HandleTypeDef *Handle)
{
	uint8_t tmp;
	
	PAL_CRC8_Reset(&Handle->Encoder.Crc8); // 复位CRC8校验，用于产生数据头的CRC码
	
	// 1. 发送SYNC（1字节）
	tmp = BIN_MESSAGE_PKT_SYNC;
	PAL_CRC8_Input(&Handle->Encoder.Crc8, tmp);
	PAL_USART_SendByte(Handle->Init.hUSART, tmp);
	
	// 2. 发送ID（2字节）
	tmp = (uint8_t)(Handle->Encoder.ID & 0xff); // 低字节
	PAL_CRC8_Input(&Handle->Encoder.Crc8, tmp);
	PAL_USART_SendByte(Handle->Init.hUSART, tmp);
	
	tmp = (uint8_t)((Handle->Encoder.ID >> 8) & 0xff); // 高字节
	PAL_CRC8_Input(&Handle->Encoder.Crc8, tmp);
	PAL_USART_SendByte(Handle->Init.hUSART, tmp);
	
	// 3. 参数数组长度 - DLC（1字节）
	tmp = Handle->Encoder.DataLength; // 低字节
	PAL_CRC8_Input(&Handle->Encoder.Crc8, tmp);
	PAL_USART_SendByte(Handle->Init.hUSART, tmp);
	
	// 4. 数据头的CRC8校验码（1字节）
	tmp = PAL_CRC8_GetResult(&Handle->Encoder.Crc8);
	PAL_USART_SendByte(Handle->Init.hUSART, tmp);
	
	PAL_CRC8_Reset(&Handle->Encoder.Crc8); // 复位CRC8校验，用于产生参数部分的CRC码
	
	// 5. 发送参数（n个字节）
	uint16_t i;
	for(i=0; i<Handle->Encoder.DataLength; i++)
	{
		tmp = Handle->Encoder.Data[i];
		PAL_CRC8_Input(&Handle->Encoder.Crc8, tmp);
		PAL_USART_SendByte(Handle->Init.hUSART, tmp);
	}
	
	// 6. 发送参数部分的CRC8校验码
	tmp = PAL_CRC8_GetResult(&Handle->Encoder.Crc8); 
	PAL_USART_SendByte(Handle->Init.hUSART, tmp);
}

//
// @简介：接收二进制消息
// @参数：Handle - 编解码器的句柄
// @参数：Timeout - 超时时间，即未通过串口收到有效消息时最长的等待时间，单位毫秒：
//                  0表示不等待；PAL_MAX_DELAY表示超时时间为无限长；其它自然数表示有限的等待时间
// @返回值：表示超时时间结束前是否接收到了有效的消息：ERROR - 接收失败，SUCCESS - 接收成功
// 
ErrorStatus PAL_BinCodec_Receive(PalBinCodec_HandleTypeDef *Handle, uint32_t Timeout)
{
	ErrorStatus ret = ERROR;
	uint64_t expiredTime;
	
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
	int16_t rx;
	do
	{
		while((rx = PAL_USART_ReceiveByte(Handle->Init.hUSART, 0)) >= 0)
		{			
			// 已接收到消息
			if(PAL_BinCodec_PutByteToDecoder(Handle, (uint8_t)rx) == SUCCESS)
			{
				ret = SUCCESS; // 表示已经收到消息
				break;
			}
		}
		if(ret == SUCCESS)
		{
			break;
		}
	}while(PAL_GetTick() < expiredTime);
	
	return ret;
}

//
// @简介：读取消息ID
// @参数：Handle - 编解码器的句柄
// @注意：在调用此编程接口前需要先成功调用PAL_BinCodec_Receive接收到消息
//
uint16_t PAL_BinCodec_GetID(PalBinCodec_HandleTypeDef *Handle)
{
	return Handle->Decoder.ID;
}

// 
// @简介：获取消息中的数据部分的长度（以字节为单位）
// @参数：Handle - 编解码器的句柄
// @返回值：数据部分的长度（以字节为单位）
// @注意：在调用此编程接口前需要先成功调用PAL_BinCodec_Receive接收到消息
// 
uint8_t PAL_BinCodec_GetDataLength(PalBinCodec_HandleTypeDef *Handle)
{
	return Handle->Decoder.DataLength;
}

// 
// @简介：查询当前消息的数据部分的CRC校验码是否正确
// @参数：Handle - 编解码器的句柄
// @返回值：校验码是否正确，SET - 校验码正确，数据部分传输无错误；RESET - 校验码不正确
// @注意：在调用此编程接口前需要先成功调用PAL_BinCodec_Receive接收到消息
// 
FlagStatus PAL_BinCodec_GetDataCRC(PalBinCodec_HandleTypeDef *Handle)
{
	return Handle->Decoder.DataCRCValid;
}

// 
// @简介：从数据包的数据部分读取一个字节的数据
// @参数：Handle - 编解码器的句柄
// @返回值：数据
// @注意：在调用此编程接口前需要先成功调用PAL_BinCodec_Receive接收到消息
// 
uint8_t PAL_BinCodec_ReadByte(PalBinCodec_HandleTypeDef *Handle)
{
	uint8_t ret = 0;
	if(Handle->Decoder.Cursor < Handle->Decoder.DataLength)
	{
		ret = Handle->Decoder.Data[Handle->Decoder.Cursor++];
	}
	return ret;
}

// 
// @简介：从数据包的数据部分读取多个字节的数据
// @参数：Handle - 编解码器的句柄
// @参数：pDataOut - 输出参数，用于接收读取到的字节数组
// @参数：Size - 需要读取的数据的长度（以字节为单位）
// @注意：在调用此编程接口前需要先成功调用PAL_BinCodec_Receive接收到消息
// 
void PAL_BinCodec_ReadBytes(PalBinCodec_HandleTypeDef *Handle, uint8_t *pDataOut, uint16_t Size)
{
	uint16_t i;
	for(i=0;i<Size;i++)
	{
		if(Handle->Decoder.Cursor < Handle->Decoder.DataLength) // 防止越界读取
		{
			pDataOut[i] = Handle->Decoder.Data[Handle->Decoder.Cursor++];
		}
		else
		{
			break;
		}
	}
}

// 
// @简介：从数据包的数据部分读取一个带符号的8位整数
// @参数：Handle - 编解码器的句柄
// @返回值：数据
// @注意：在调用此编程接口前需要先成功调用PAL_BinCodec_Receive接收到消息
// 
int8_t PAL_BinCodec_ReadInt8(PalBinCodec_HandleTypeDef *Handle)
{
	int8_t ret = 0;
	if(Handle->Decoder.Cursor < Handle->Decoder.DataLength)
	{
		ret = *((int8_t *)&Handle->Decoder.Data[Handle->Decoder.Cursor++]);
	}
	return ret;
}


// 
// @简介：从数据包的数据部分读取一个带符号的16位整数
// @参数：Handle - 编解码器的句柄
// @返回值：数据
// @注意：在调用此编程接口前需要先成功调用PAL_BinCodec_Receive接收到消息
// 
int16_t PAL_BinCodec_ReadInt16(PalBinCodec_HandleTypeDef *Handle)
{
	int16_t ret = 0;
	
	if(Handle->Decoder.Cursor < Handle->Decoder.DataLength + 1 - sizeof(int16_t)) // 防止越界读取
	{
		ret = *((int16_t *)&Handle->Decoder.Data[Handle->Decoder.Cursor]);
		Handle->Decoder.Cursor += sizeof(int16_t);
	}
	else // 消化完未读取的数据
	{
		while(Handle->Decoder.Cursor < Handle->Decoder.DataLength)
		{
			Handle->Decoder.Cursor++;
		}
	}
	return ret;
}


// 
// @简介：从数据包的数据部分读取一个带符号的32位整数
// @参数：Handle - 编解码器的句柄
// @返回值：数据
// @注意：在调用此编程接口前需要先成功调用PAL_BinCodec_Receive接收到消息
// 
int32_t PAL_BinCodec_ReadInt32(PalBinCodec_HandleTypeDef *Handle)
{
	int32_t ret = 0;
	
	if(Handle->Decoder.Cursor < Handle->Decoder.DataLength + 1 - sizeof(int32_t)) // 防止越界读取
	{
		ret = *((int32_t *)&Handle->Decoder.Data[Handle->Decoder.Cursor]);
		Handle->Decoder.Cursor += sizeof(int32_t);
	}
	else // 消化完未读取的数据
	{
		while(Handle->Decoder.Cursor < Handle->Decoder.DataLength)
		{
			Handle->Decoder.Cursor++;
		}
	}
	return ret;
}

// 
// @简介：从数据包的数据部分读取一个带符号的64位整数
// @参数：Handle - 编解码器的句柄
// @返回值：数据
// @注意：在调用此编程接口前需要先成功调用PAL_BinCodec_Receive接收到消息
// 
int64_t PAL_BinCodec_ReadInt64(PalBinCodec_HandleTypeDef *Handle)
{
	int64_t ret = 0;
	
	if(Handle->Decoder.Cursor < Handle->Decoder.DataLength + 1 - sizeof(int64_t)) // 防止越界读取
	{
		ret = *((int64_t *)&Handle->Decoder.Data[Handle->Decoder.Cursor]);
		Handle->Decoder.Cursor += sizeof(int64_t);
	}
	else // 消化完未读取的数据
	{
		while(Handle->Decoder.Cursor < Handle->Decoder.DataLength)
		{
			Handle->Decoder.Cursor++;
		}
	}
	return ret;
}

// 
// @简介：从数据包的数据部分读取一个无符号的8位整数
// @参数：Handle - 编解码器的句柄
// @返回值：数据
// @注意：在调用此编程接口前需要先成功调用PAL_BinCodec_Receive接收到消息
// 
uint8_t PAL_BinCodec_ReadUInt8(PalBinCodec_HandleTypeDef *Handle)
{
	uint8_t ret = 0;
	if(Handle->Decoder.Cursor < Handle->Decoder.DataLength)
	{
		ret = Handle->Decoder.Data[Handle->Decoder.Cursor++];
	}
	return ret;
}

// 
// @简介：从数据包的数据部分读取一个无符号的16位整数
// @参数：Handle - 编解码器的句柄
// @返回值：数据
// @注意：在调用此编程接口前需要先成功调用PAL_BinCodec_Receive接收到消息
// 
uint16_t PAL_BinCodec_ReadUInt16(PalBinCodec_HandleTypeDef *Handle)
{
	uint16_t ret = 0;
	
	if(Handle->Decoder.Cursor < Handle->Decoder.DataLength + 1 - sizeof(uint16_t)) // 防止越界读取
	{
		ret = *((uint16_t *)&Handle->Decoder.Data[Handle->Decoder.Cursor]);
		Handle->Decoder.Cursor += sizeof(uint16_t);
	}
	else // 消化完未读取的数据
	{
		while(Handle->Decoder.Cursor < Handle->Decoder.DataLength)
		{
			Handle->Decoder.Cursor++;
		}
	}
	return ret;
}
// 
// @简介：从数据包的数据部分读取一个无符号的32位整数
// @参数：Handle - 编解码器的句柄
// @返回值：数据
// @注意：在调用此编程接口前需要先成功调用PAL_BinCodec_Receive接收到消息
// 
uint32_t PAL_BinCodec_ReadUInt32(PalBinCodec_HandleTypeDef *Handle)
{
	uint32_t ret = 0;
	
	if(Handle->Decoder.Cursor < Handle->Decoder.DataLength + 1 - sizeof(uint32_t)) // 防止越界读取
	{
		ret = *((uint32_t *)&Handle->Decoder.Data[Handle->Decoder.Cursor]);
		Handle->Decoder.Cursor += sizeof(uint32_t);
	}
	else // 消化完未读取的数据
	{
		while(Handle->Decoder.Cursor < Handle->Decoder.DataLength)
		{
			Handle->Decoder.Cursor++;
		}
	}
	return ret;
}
// 
// @简介：从数据包的数据部分读取一个无符号的64位整数
// @参数：Handle - 编解码器的句柄
// @返回值：数据
// @注意：在调用此编程接口前需要先成功调用PAL_BinCodec_Receive接收到消息
// 
uint64_t PAL_BinCodec_ReadUInt64(PalBinCodec_HandleTypeDef *Handle)
{
	uint64_t ret = 0;
	
	if(Handle->Decoder.Cursor < Handle->Decoder.DataLength + 1 - sizeof(uint64_t)) // 防止越界读取
	{
		ret = *((uint64_t *)&Handle->Decoder.Data[Handle->Decoder.Cursor]);
		Handle->Decoder.Cursor += sizeof(uint64_t);
	}
	else // 消化完未读取的数据
	{
		while(Handle->Decoder.Cursor < Handle->Decoder.DataLength)
		{
			Handle->Decoder.Cursor++;
		}
	}
	return ret;
}

// 
// @简介：从数据包的数据部分读取一个浮点数
// @参数：Handle - 编解码器的句柄
// @返回值：数据
// @注意：在调用此编程接口前需要先成功调用PAL_BinCodec_Receive接收到消息
// 
float PAL_BinCodec_ReadFloat(PalBinCodec_HandleTypeDef *Handle)
{
	float ret = 0.0;
	
	if(Handle->Decoder.Cursor < Handle->Decoder.DataLength + 1 - sizeof(float)) // 防止越界读取
	{
		ret = *((float *)&Handle->Decoder.Data[Handle->Decoder.Cursor]);
		Handle->Decoder.Cursor += sizeof(float);
	}
	else // 消化完未读取的数据
	{
		while(Handle->Decoder.Cursor < Handle->Decoder.DataLength)
		{
			Handle->Decoder.Cursor++;
		}
	}
	return ret;
}

// 
// @简介：从数据包的数据部分读取一个双精度浮点数
// @参数：Handle - 编解码器的句柄
// @返回值：数据
// @注意：在调用此编程接口前需要先成功调用PAL_BinCodec_Receive接收到消息
// 
double PAL_BinCodec_ReadDouble(PalBinCodec_HandleTypeDef *Handle)
{
	double ret = 0.0;
	
	if(Handle->Decoder.Cursor < Handle->Decoder.DataLength + 1 - sizeof(double)) // 防止越界读取
	{
		ret = *((double *)&Handle->Decoder.Data[Handle->Decoder.Cursor]);
		Handle->Decoder.Cursor += sizeof(double);
	}
	else // 消化完未读取的数据
	{
		while(Handle->Decoder.Cursor < Handle->Decoder.DataLength)
		{
			Handle->Decoder.Cursor++;
		}
	}
	return ret;
}

// 
// @简介：从数据包的数据部分读取一行字符串（遇到\0停止读取）
// @参数：Handle - 编解码器的句柄
// @参数：pStrOut - 输出参数，用于接收读取到的字符串
// @参数：Size - 用于指定接收缓冲区pStrOut的长度
// @注意：在调用此编程接口前需要先成功调用PAL_BinCodec_Receive接收到消息
// @注意：读取的内容受限于参数Size的值，如果Size小于等于字符串长度，读取将提前终止
void PAL_BinCodec_ReadString(PalBinCodec_HandleTypeDef *Handle, char *pStrOut, uint16_t Size)
{
	uint16_t i;
	
	for(i=0;i<Size-1;i++) // 最后一个字符用于存储'\0'
	{
		if(Handle->Decoder.Cursor >= Handle->Decoder.DataLength) // 读取越界
		{
			break;
		}
		if(Handle->Decoder.Data[Handle->Decoder.Cursor] == '\0') // 读取到了一个完整的行
		{
			break;
		}
		pStrOut[i] = Handle->Decoder.Data[Handle->Decoder.Cursor++];
	}
	
	// 在字符串的末尾增加\0
	pStrOut[i] = '\0';
}

static void Encoder_Init(PalBinEncoder_TypeDef *Encoder)
{
	Encoder->Crc8.Init.Initial = 0x00;
	Encoder->Crc8.Init.Polygon = 0x07;
	Encoder->Crc8.Init.ResultXOR = 0x00;
	Encoder->Crc8.Init.ReverseInput = DISABLE;
	Encoder->Crc8.Init.ReverseOutput = DISABLE;
	
	PAL_CRC8_Init(&Encoder->Crc8);
}

static void Decoder_Init(PalBinDecoder_TypeDef *Decoder)
{
	Decoder->Stage = DECODER_STAGE_BEFORE_SYNC;
	Decoder->Cursor = 0;
	
	Decoder->Crc8.Init.Initial = 0x00;
	Decoder->Crc8.Init.Polygon = 0x07;
	Decoder->Crc8.Init.ResultXOR = 0x00;
	Decoder->Crc8.Init.ReverseInput = DISABLE;
	Decoder->Crc8.Init.ReverseOutput = DISABLE;
	
	PAL_CRC8_Init(&Decoder->Crc8);
}

static ErrorStatus PAL_BinCodec_PutByteToDecoder(PalBinCodec_HandleTypeDef *Handle, uint8_t InputByte)
{
	ErrorStatus ret = ERROR;
	
	switch(Handle->Decoder.Stage)
	{
		case DECODER_STAGE_BEFORE_SYNC: // 等待SYNC
		{
			if(InputByte == BIN_MESSAGE_PKT_SYNC)
			{
				PAL_CRC8_Reset(&Handle->Decoder.Crc8);
				PAL_CRC8_Input(&Handle->Decoder.Crc8, InputByte);
				Handle->Decoder.Stage = DECODER_STAGE_ID_L;
			}
			break;
		}
		case DECODER_STAGE_ID_L: // 接收消息ID的低字节
		{
			Handle->Decoder.ID = InputByte;
			
			PAL_CRC8_Input(&Handle->Decoder.Crc8, InputByte);
			
			Handle->Decoder.Stage = DECODER_STAGE_ID_H;
			break;
		}
		case DECODER_STAGE_ID_H: // 接收消息ID的高字节
		{
			Handle->Decoder.ID |= ((uint16_t)InputByte) << 8;
			
			PAL_CRC8_Input(&Handle->Decoder.Crc8, InputByte);
			
			Handle->Decoder.Stage = DECODER_STAGE_DLC;
			break;
		}
		case DECODER_STAGE_DLC: // 接收数据长度的低字节
		{
			Handle->Decoder.DataLength = InputByte;
			
			PAL_CRC8_Input(&Handle->Decoder.Crc8, InputByte);
			
			Handle->Decoder.Stage = DECODER_STAGE_HEADER_CRC;
			break;
		}
		case DECODER_STAGE_HEADER_CRC: // 比对包头CRC
		{
			if(InputByte == PAL_CRC8_GetResult(&Handle->Decoder.Crc8))
			{
				PAL_CRC8_Reset(&Handle->Decoder.Crc8); // 复位CRC校验，为校验数据部分做好准备
				if(Handle->Decoder.DataLength == 0) // 无数据，直接进入CRC阶段
				{
					Handle->Decoder.Stage = DECODER_STAGE_DATA_CRC;
				}
				else
				{
					Handle->Decoder.BytesToRecv = Handle->Decoder.DataLength;
					Handle->Decoder.Stage = DECODER_STAGE_DATA;
				}
			}
			else // 包头CRC校验失败
			{
				Handle->Decoder.Stage = DECODER_STAGE_BEFORE_SYNC;
			}
			break;
		}
		case DECODER_STAGE_DATA:
		{
			Handle->Decoder.Data[Handle->Decoder.DataLength - Handle->Decoder.BytesToRecv] = InputByte;
			PAL_CRC8_Input(&Handle->Decoder.Crc8, InputByte);
			Handle->Decoder.BytesToRecv--;
			if(Handle->Decoder.BytesToRecv == 0)
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
			Handle->Decoder.Cursor = 0; // 重置游标位置，以便后续读取
			ret = SUCCESS;
			
			Handle->Decoder.Stage = DECODER_STAGE_BEFORE_SYNC;
			break;
		}
		default:
			break;
	}
	
	return ret;
}
