/**
  ******************************************************************************
  * @file    user_persistance.c
  * @author  铁头山羊
  * @version V 1.0.0
  * @date    2023年4月24日
  * @brief   参数持久化
  ******************************************************************************
  * @attention
  * Copyright (c) 2022 - 2023 东莞市明玉科技有限公司. 保留所有权力.
  ******************************************************************************
*/
#include "user_persistance.h"
#include "stm32f10x_pal_w25q64.h"

#define APP_PERSIST_ADDRESS (uint32_t)0x00000000

extern PalW25Q64_HandleTypeDef hw25q64;

void User_Persist_ReadFromFlash(AppPersist_TypeDef *pStruct)
{
	while(PAL_W25Q64_GetBusyFlag(&hw25q64) == SET);
	
	PAL_W25Q64_Read(&hw25q64, APP_PERSIST_ADDRESS, (uint8_t *)pStruct, sizeof(AppPersist_TypeDef));
}

void User_Persist_WriteToFlash(AppPersist_TypeDef *pStruct)
{
	pStruct->InitializeToken = PREDEFINED_INITIALIZE_TOKEN;
	
	while(PAL_W25Q64_GetBusyFlag(&hw25q64) == SET);
	
	PAL_W25Q64_WriteEnable(&hw25q64);
	
	PAL_W25Q64_SectorErase(&hw25q64, APP_PERSIST_ADDRESS);
	
	while(PAL_W25Q64_GetBusyFlag(&hw25q64) == SET);
	
	PAL_W25Q64_WriteEnable(&hw25q64);
	
	PAL_W25Q64_PageProgram(&hw25q64, APP_PERSIST_ADDRESS, (uint8_t *)pStruct, sizeof(AppPersist_TypeDef));
	
	while(PAL_W25Q64_GetBusyFlag(&hw25q64) == SET);
}
