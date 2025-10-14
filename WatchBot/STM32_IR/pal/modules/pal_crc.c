/**
  ******************************************************************************
  * @file    pal_crc.c
  * @author  铁头山羊stm32工作组
  * @version V1.0.0
  * @date    2023年2月25日
  * @brief   循环冗余校验
  ******************************************************************************
  * @attention
  * Copyright (c) 2022 - 2023 东莞市明玉科技有限公司. 保留所有权力.
  ******************************************************************************
	*/
#include "pal_crc.h"

static void PopulateLookupTable(PalCRC8_HandleTypeDef *Handle);
static uint8_t ReverseBitsOfAByte(uint8_t Input);

void PAL_CRC8_Init(PalCRC8_HandleTypeDef *Handle)
{
	PopulateLookupTable(Handle);
	Handle->crc = Handle->Init.Initial;
}

void PAL_CRC8_Reset(PalCRC8_HandleTypeDef *Handle)
{
	Handle->crc = 0;
}

void PAL_CRC8_Input(PalCRC8_HandleTypeDef *Handle, uint8_t value)
{
	if(Handle->Init.ReverseInput == ENABLE)
	{
		value = ReverseBitsOfAByte(value);
	}
	Handle->crc = Handle->lookup_table[value ^ Handle->crc];
}

uint8_t PAL_CRC8_GetResult(PalCRC8_HandleTypeDef *Handle)
{
	uint8_t result = Handle->crc;
	
	if(Handle->Init.ReverseOutput == ENABLE)
	{
		result = ReverseBitsOfAByte(result);
	}
	
	return result ^ (Handle->Init.ResultXOR);
}

static void PopulateLookupTable(PalCRC8_HandleTypeDef *Handle)
{
	uint16_t i, tmp;
	uint8_t j;
	
	for(i=0;i<256;i++)
	{
		tmp = i;
		for(j=0;j<8;j++)
		{
			if((tmp & 0x80) != 0)
			{
				tmp <<= 1;
				tmp ^= Handle->Init.Polygon;
			}
			else
			{
				tmp <<= 1;
			}
		}
		Handle->lookup_table[i] = tmp;
	}
}

static uint8_t ReverseBitsOfAByte(uint8_t Input)
{
	uint8_t result = 0;
	
	result |= (Input & (0x80 >> 0)) >> 7;
	result |= (Input & (0x80 >> 1)) >> 5;
	result |= (Input & (0x80 >> 2)) >> 3;
	result |= (Input & (0x80 >> 3)) >> 1;
	
	result |= (Input & (0x80 >> 4)) << 1;
	result |= (Input & (0x80 >> 5)) << 3;
	result |= (Input & (0x80 >> 6)) << 5;
	result |= (Input & (0x80 >> 7)) << 7;
	
	return result;
}
