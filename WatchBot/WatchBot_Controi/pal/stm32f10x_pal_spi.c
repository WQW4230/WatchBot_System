/**
  ******************************************************************************
  * @file    stm32f10x_pal_spi.c
  * @author  铁头山羊stm32工作组
  * @version V1.0.0
  * @date    2023年2月24日
  * @brief   spi总线驱动程序
  ******************************************************************************
  * @attention
  * Copyright (c) 2022 - 2023 东莞市明玉科技有限公司. 保留所有权力.
  ******************************************************************************
*/

#include "stm32f10x_pal_spi.h"

void PAL_SPI_Init(PalSPI_HandleTypeDef *Handle)
{
	GPIO_InitTypeDef GPIOInitStruct;
	
	// 1. 初始化GPIO
	if(Handle->Init.SPIx == SPI1)
	{
		if(Handle->Init.Advanced.Remap == 0) // NSS, SCK, MISO, MOSI PA4, PA5, PA6, PA7
		{
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
			// SCK -> PA5
			GPIOInitStruct.GPIO_Pin = GPIO_Pin_5;
			GPIOInitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
			GPIOInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_Init(GPIOA, &GPIOInitStruct);
			// MISO -> PA6
			GPIOInitStruct.GPIO_Pin = GPIO_Pin_6;
			GPIOInitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
			GPIO_Init(GPIOA, &GPIOInitStruct);
			// MOSI -> PA7
			GPIOInitStruct.GPIO_Pin = GPIO_Pin_7;
			GPIOInitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
			GPIOInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_Init(GPIOA, &GPIOInitStruct);
		}
		else // Remap = 1, NSS, SCK, MISO, MOSI PA15, PB3, PB4, PB5
		{
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
			// SCK -> PB3
			GPIOInitStruct.GPIO_Pin = GPIO_Pin_3;
			GPIOInitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
			GPIOInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_Init(GPIOB, &GPIOInitStruct);
			// MISO -> PB4
			GPIOInitStruct.GPIO_Pin = GPIO_Pin_4;
			GPIOInitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
			GPIO_Init(GPIOB, &GPIOInitStruct);
			// MOSI -> PB5
			GPIOInitStruct.GPIO_Pin = GPIO_Pin_5;
			GPIOInitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
			GPIOInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_Init(GPIOB, &GPIOInitStruct);
		}
	}
	else if(Handle->Init.SPIx == SPI2) // NSS, SCK, MISO, MOSI PB12 PB13 PB14 PB15
	{
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
		// SCK -> PB13
		GPIOInitStruct.GPIO_Pin = GPIO_Pin_13;
		GPIOInitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
		GPIOInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOB, &GPIOInitStruct);
		// MISO -> PB14
		GPIOInitStruct.GPIO_Pin = GPIO_Pin_14;
		GPIOInitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		GPIO_Init(GPIOB, &GPIOInitStruct);
		// MOSI -> PB15
		GPIOInitStruct.GPIO_Pin = GPIO_Pin_15;
		GPIOInitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
		GPIOInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOB, &GPIOInitStruct);
	}
	
	// 2. 使能SPIx的时钟
	if(Handle->Init.SPIx == SPI1)
	{
		// 复位
		RCC_APB2PeriphResetCmd(RCC_APB2Periph_SPI1, ENABLE);
		RCC_APB2PeriphResetCmd(RCC_APB2Periph_SPI1, DISABLE);
		// 开启时钟
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
	}
	else if(Handle->Init.SPIx == SPI2)
	{
		// 复位
		RCC_APB1PeriphResetCmd(RCC_APB1Periph_SPI2, ENABLE);
		RCC_APB1PeriphResetCmd(RCC_APB1Periph_SPI2, DISABLE);
		// 开启时钟
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
	}
	
	// 3. 初始化SPI
	SPI_NSSInternalSoftwareConfig(Handle->Init.SPIx, SPI_NSSInternalSoft_Set); // SSI=1
	SPI_InitTypeDef SPIInitStruct;
	SPIInitStruct.SPI_Mode = SPI_Mode_Master; // 只支持master模式
	SPIInitStruct.SPI_BaudRatePrescaler = Handle->Init.SPI_BaudRatePrescaler;
	SPIInitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPIInitStruct.SPI_DataSize = SPI_DataSize_8b; // 只支持8比特
	SPIInitStruct.SPI_FirstBit = Handle->Init.SPI_FirstBit;
	SPIInitStruct.SPI_CPOL = Handle->Init.SPI_CPOL;
	SPIInitStruct.SPI_CPHA = Handle->Init.SPI_CPHA;
	SPIInitStruct.SPI_NSS = SPI_NSS_Soft;
	SPI_Init(Handle->Init.SPIx, &SPIInitStruct);
}

void PAL_SPI_MasterTransmit(PalSPI_HandleTypeDef *Handle, const uint8_t *pData, uint16_t Size)
{
	uint16_t i;
	
	SPI_Cmd(Handle->Init.SPIx, ENABLE);
	
	if(Handle->Init.Advanced.PrefixSize != 0 && Handle->Init.Advanced.pPrefix != 0)
	{
		for(i=0;i<Handle->Init.Advanced.PrefixSize;i++)
		{
			// 等待TXE置位
			while(SPI_I2S_GetFlagStatus(Handle->Init.SPIx, SPI_I2S_FLAG_TXE) == RESET);
			// 发送数据
			SPI_I2S_SendData(Handle->Init.SPIx, Handle->Init.Advanced.pPrefix[i]);
		}
	}
	
	for(i=0;i<Size;i++)
	{
		// 等待TXE置位
		while(SPI_I2S_GetFlagStatus(Handle->Init.SPIx, SPI_I2S_FLAG_TXE) == RESET);
		// 发送数据
		SPI_I2S_SendData(Handle->Init.SPIx, pData[i]);
	}
	
	// 等待发送完成
	while(SPI_I2S_GetFlagStatus(Handle->Init.SPIx, SPI_I2S_FLAG_TXE) == RESET);
	while(SPI_I2S_GetFlagStatus(Handle->Init.SPIx, SPI_I2S_FLAG_BSY) == SET);
	
	// 清除OVR
	if(SPI_I2S_GetFlagStatus(Handle->Init.SPIx, SPI_I2S_FLAG_OVR) == SET) 
	{
		SPI_I2S_ReceiveData(Handle->Init.SPIx);
	}
	// 清除RXNE
	if(SPI_I2S_GetFlagStatus(Handle->Init.SPIx, SPI_I2S_FLAG_RXNE) == SET) 
  {
		SPI_I2S_ReceiveData(Handle->Init.SPIx);
  }
	
	SPI_Cmd(Handle->Init.SPIx, DISABLE);
}

void PAL_SPI_MasterReceive(PalSPI_HandleTypeDef *Handle, uint8_t *pBuf, uint16_t Size)
{
	uint32_t i;
	
	if(Size == 0) return; // 需要接收的数据长度为0，无需任何操作
	
	SPI_Cmd(Handle->Init.SPIx, ENABLE);
	
	// 清除OVR
	if(SPI_I2S_GetFlagStatus(Handle->Init.SPIx, SPI_I2S_FLAG_OVR) == SET) 
	{
		SPI_I2S_ReceiveData(Handle->Init.SPIx);
	}
	// 清除RXNE
	if(SPI_I2S_GetFlagStatus(Handle->Init.SPIx, SPI_I2S_FLAG_RXNE) == SET) 
  {
		SPI_I2S_ReceiveData(Handle->Init.SPIx);
  }
	
	// 先发送第一个数据
	SPI_I2S_SendData(Handle->Init.SPIx, 0x00);
	
	for(i=0;i<Size-1;i++)
	{
		// 等待TxE置位
		while(SPI_I2S_GetFlagStatus(Handle->Init.SPIx, SPI_I2S_FLAG_TXE) == RESET);
		// 此时DR的数据刚刚被传递到移位寄存器
		SPI_I2S_SendData(Handle->Init.SPIx, 0x00);
		// 等待RxNE置位
		while(SPI_I2S_GetFlagStatus(Handle->Init.SPIx, SPI_I2S_FLAG_RXNE) == RESET);
		// 接收数据
		pBuf[i] = SPI_I2S_ReceiveData(Handle->Init.SPIx);
	}
	
	// 等待RxNE置位
	while(SPI_I2S_GetFlagStatus(Handle->Init.SPIx, SPI_I2S_FLAG_RXNE) == RESET);
	// 读出最后一个数据
	pBuf[Size-1] = SPI_I2S_ReceiveData(Handle->Init.SPIx);
	// 等待BSY复位
	while(SPI_I2S_GetFlagStatus(Handle->Init.SPIx, SPI_I2S_FLAG_BSY) == SET);
	
	SPI_Cmd(Handle->Init.SPIx, DISABLE);
}

void PAL_SPI_MasterTransmitReceive(PalSPI_HandleTypeDef *Handle, const uint8_t *pTxData, uint8_t *pRxData, uint16_t Size)
{
	uint32_t i;
	
	SPI_Cmd(Handle->Init.SPIx, ENABLE);
	
	// 向DR中写入数据
	SPI_I2S_SendData(Handle->Init.SPIx, pTxData[0]);
	
	for(i=0;i<Size-1;i++)
	{
		// 等待TxE置位
		while(SPI_I2S_GetFlagStatus(Handle->Init.SPIx, SPI_I2S_FLAG_TXE) == RESET);
		// 此时DR的数据刚刚被传递到移位寄存器
		SPI_I2S_SendData(Handle->Init.SPIx, pTxData[i+1]);
		// 等待RxNE置位
		while(SPI_I2S_GetFlagStatus(Handle->Init.SPIx, SPI_I2S_FLAG_RXNE) == RESET);
		// 接收数据
		pRxData[i] = SPI_I2S_ReceiveData(Handle->Init.SPIx);
	}
	
	// 等待RxNE置位
	while(SPI_I2S_GetFlagStatus(Handle->Init.SPIx, SPI_I2S_FLAG_RXNE) == RESET);
	// 读出最后一个数据
	pRxData[Size-1] = SPI_I2S_ReceiveData(Handle->Init.SPIx);
	// 等待BSY复位
	while(SPI_I2S_GetFlagStatus(Handle->Init.SPIx, SPI_I2S_FLAG_BSY) == SET);
	
	SPI_Cmd(Handle->Init.SPIx, DISABLE);
}
