/**
  ******************************************************************************
  * @file    pal_crc.h
  * @author  铁头山羊stm32工作组
  * @version V1.0.0
  * @date    2023年2月25日
  * @brief   循环冗余校验
  ******************************************************************************
  * @attention
  * Copyright (c) 2022 - 2023 东莞市明玉科技有限公司. 保留所有权力.
  ******************************************************************************
	*/
#ifndef _PAL_CRC_H_
#define _PAL_CRC_H_
#include "stm32f10x.h"

typedef struct
{
	uint8_t Polygon; // 生成多项式，忽略最高位的1，
	                 // 例如当生成多项式为x8+x2+x+1时，该字段应填写0x07
	uint8_t Initial; // 初始值
	uint8_t ResultXOR; // 结果异或值
	FunctionalState ReverseInput; // 输入反转
	FunctionalState ReverseOutput; // 输出反转
}PalCRC8_CalcInitTypeDef;

typedef struct
{
	PalCRC8_CalcInitTypeDef Init;
	uint8_t crc;
	uint8_t lookup_table[256];
}PalCRC8_HandleTypeDef;

void PAL_CRC8_Init(PalCRC8_HandleTypeDef *Handle);
void PAL_CRC8_Reset(PalCRC8_HandleTypeDef *Handle);
void PAL_CRC8_Input(PalCRC8_HandleTypeDef *Handle, uint8_t value);
uint8_t PAL_CRC8_GetResult(PalCRC8_HandleTypeDef *Handle);

#endif
