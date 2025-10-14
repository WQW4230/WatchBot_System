/**
  ******************************************************************************
  * @file    stm32f10x_pal_w25q64.h
  * @author  铁头山羊stm32工作组
  * @version V1.0.0
  * @date    2023年2月24日
  * @brief   w25q64 flash存储器驱动
  ******************************************************************************
  * @attention
  * Copyright (c) 2022 - 2023 东莞市明玉科技有限公司. 保留所有权力.
  ******************************************************************************
*/

#ifndef _STM32F10x_PAL_W25Q64_H_
#define _STM32F10x_PAL_W25Q64_H_

#include "stm32f10x.h"
#include "stm32f10x_pal_spi.h"

typedef struct
{
	PalSPI_HandleTypeDef *hspi;
	GPIO_TypeDef *SS_GPIO_Port;
	uint16_t SS_GPIO_Pin;
}PalW25Q64_InitTypeDef;

typedef struct
{
	PalW25Q64_InitTypeDef Init;
}PalW25Q64_HandleTypeDef;

void PAL_W25Q64_Init(PalW25Q64_HandleTypeDef *Handle);
void PAL_W25Q64_WriteEnable(PalW25Q64_HandleTypeDef *Handle);
void PAL_W25Q64_WriteDisable(PalW25Q64_HandleTypeDef *Handle);
void PAL_W25Q64_PageProgram(PalW25Q64_HandleTypeDef *Handle, uint32_t Addr, const uint8_t *pData, uint16_t Size);
void PAL_W25Q64_SectorErase(PalW25Q64_HandleTypeDef *Handle, uint32_t Addr);
void PAL_W25Q64_BlockErase_32k(PalW25Q64_HandleTypeDef *Handle, uint32_t Addr);
void PAL_W25Q64_BlockErase_64k(PalW25Q64_HandleTypeDef *Handle, uint32_t Addr);
void PAL_W25Q64_Read(PalW25Q64_HandleTypeDef *Handle, uint32_t Addr, uint8_t *pDataOut, uint16_t Size);
FlagStatus PAL_W25Q64_GetBusyFlag(PalW25Q64_HandleTypeDef *Handle);

#endif
