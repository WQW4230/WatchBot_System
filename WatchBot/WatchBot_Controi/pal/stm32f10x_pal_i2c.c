/**
  ******************************************************************************
  * @file    stm32f10x_pal_i2c.h
  * @author  铁头山羊stm32工作组
  * @version V1.0.0
  * @date    2023年2月24日
  * @brief   i2c总线驱动程序
  ******************************************************************************
  * @attention
  * Copyright (c) 2022 - 2023 东莞市明玉科技有限公司. 保留所有权力.
  ******************************************************************************
*/

#include "stm32f10x_pal_i2c.h"
#include "stm32f10x_pal.h"

ErrorStatus PAL_I2C_Init(PalI2C_HandleTypeDef *Handle)
{
	GPIO_InitTypeDef GPIOInitStruct;
	
	Handle->Init.Advanced.SendPrefix = RESET;
	
	// 1. 配置IO引脚
	if(Handle->Init.I2Cx == I2C1) // I2C1
	{
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
		if(Handle->Init.Advanced.Remap == 0)
		{
			GPIOInitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
		}
		else if(Handle->Init.Advanced.Remap == 1)
		{
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
			GPIO_PinRemapConfig(GPIO_Remap_I2C1, ENABLE);
			GPIOInitStruct.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
		}
		GPIOInitStruct.GPIO_Mode = GPIO_Mode_AF_OD;
		GPIOInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
		
		GPIO_Init(GPIOB, &GPIOInitStruct);
	}
	else // I2C2
	{
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
		
		GPIOInitStruct.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
		GPIOInitStruct.GPIO_Mode = GPIO_Mode_AF_OD;
		GPIOInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
		
		GPIO_Init(GPIOB, &GPIOInitStruct);
	}
	
	// 2. 开启I2Cx的时钟
	if(Handle->Init.I2Cx == I2C1) // I2C1
	{
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
		RCC_APB2PeriphResetCmd(RCC_APB1Periph_I2C1, ENABLE);
		RCC_APB2PeriphResetCmd(RCC_APB1Periph_I2C1, DISABLE);
	}
	else // I2C2
	{
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);
		RCC_APB2PeriphResetCmd(RCC_APB1Periph_I2C2, ENABLE);
		RCC_APB2PeriphResetCmd(RCC_APB1Periph_I2C2, DISABLE);
	}
	
	I2C_Cmd(Handle->Init.I2Cx, DISABLE);
	I2C_SoftwareResetCmd(Handle->Init.I2Cx, ENABLE);
	I2C_SoftwareResetCmd(Handle->Init.I2Cx, DISABLE);
	
	// 3. 初始化I2C
	I2C_InitTypeDef I2CInitStruct;
	I2CInitStruct.I2C_Mode = I2C_Mode_I2C;
	I2CInitStruct.I2C_ClockSpeed = Handle->Init.I2C_ClockSpeed;
	I2CInitStruct.I2C_DutyCycle = Handle->Init.Advanced.Use16_9_DutyCycle == SET ? I2C_DutyCycle_16_9 : I2C_DutyCycle_2;
	I2CInitStruct.I2C_OwnAddress1 = 0;
	I2CInitStruct.I2C_Ack = I2C_Ack_Disable;
	I2C_Init(Handle->Init.I2Cx, &I2CInitStruct);
	
	I2C_Cmd(Handle->Init.I2Cx, ENABLE);
	
	return SUCCESS;
}

ErrorStatus PAL_I2C_MasterTransmit_DMA(PalI2C_HandleTypeDef *Handle, uint16_t DevAddr, const uint8_t *Buffer, uint16_t Size)
{
	uint32_t sr1cpy; // SR1寄存器的拷贝值
	DMA_InitTypeDef DMAInitStruct;
	
	// 发送start和地址
	// 等待总线空闲
	while(I2C_GetFlagStatus(Handle->Init.I2Cx, I2C_FLAG_BUSY) != RESET);
	
	// 发送Start
	I2C_GenerateSTART(Handle->Init.I2Cx, ENABLE);
	
	// 等待SB置位
	while(I2C_GetFlagStatus(Handle->Init.I2Cx, I2C_FLAG_SB) != SET);
	
	// 发送地址
	if(Handle->Init.Advanced.Use10BitAddress != SET)
	{
		// 向DR写入slave地址
		I2C_SendData(Handle->Init.I2Cx, DevAddr & 0xfe);
		
		// 等待ADDR
		while(1)
		{
			sr1cpy = Handle->Init.I2Cx->SR1;
			// 先检查AF
			if((sr1cpy & I2C_SR1_AF) != 0) // NACK，寻址失败
      {
			  I2C_GenerateSTOP(Handle->Init.I2Cx, ENABLE);
				// 清除ACK
				I2C_ClearFlag(Handle->Init.I2Cx, I2C_FLAG_AF);
				
				return ERROR; // 寻址失败
			}
			// 等待ADDR标志位置位
			// ADDR标志位置位代表地址发送完成
			if((sr1cpy & I2C_SR1_ADDR) != 0) // ADDR置位
			{
				// 清除ADDR标志位
				// 先读sr1再读sr2
				sr1cpy = Handle->Init.I2Cx->SR1;
				sr1cpy = Handle->Init.I2Cx->SR2;
				break;
			}
		}
	}
	else
	{
		// 首先发送header
		// 11110xx0，其中xx代表地址的高2位
		I2C_SendData(Handle->Init.I2Cx, (uint8_t)(((DevAddr & 0x0300) >> 7) | 0xf0));
		
		// 等待ADDR10
		while(1)
		{
			sr1cpy = Handle->Init.I2Cx->SR1;
			// 先检查AF
			if((sr1cpy & I2C_SR1_AF) != 0) // NACK，寻址失败
      {
			  I2C_GenerateSTOP(Handle->Init.I2Cx, ENABLE);
				// 清除ACK
				I2C_ClearFlag(Handle->Init.I2Cx, I2C_FLAG_AF);
				
				return ERROR; // 寻址失败
			}
			// 等待ADD10标志位
			if((sr1cpy & I2C_SR1_ADD10) != 0) // ADDR置位
			{
				// 清除ADD1标志位
				// 先读SR1（已读取）再写DR
				break;
			}
		}
		
		// 发送地址的低位
		I2C_SendData(Handle->Init.I2Cx, (uint8_t)(DevAddr & 0xfe));
		
		// 等待ADDR
		while(1)
		{
			sr1cpy = Handle->Init.I2Cx->SR1;
			// 先检查AF
			if((sr1cpy & I2C_SR1_AF) != 0) // NACK，寻址失败
      {
			  I2C_GenerateSTOP(Handle->Init.I2Cx, ENABLE);
				// 清除ACK
				I2C_ClearFlag(Handle->Init.I2Cx, I2C_FLAG_AF);
				
				return ERROR; // 寻址失败
			}
			// 等待ADDR标志位
			if((sr1cpy & I2C_SR1_ADDR) != 0) // ADDR置位
			{
				// 清除ADDR标志位
				// 先读sr1再读sr2
				sr1cpy = Handle->Init.I2Cx->SR1;
				sr1cpy = Handle->Init.I2Cx->SR2;
				break;
			}
		}
	}
	
	// 发送前缀
	if(Handle->Init.Advanced.SendPrefix == SET)
	{
		const uint8_t *pPrefix = Handle->Init.Advanced.pPrefix;
		uint16_t prefixCnt = Handle->Init.Advanced.PrefixSize;
		// 等待TXE
		while(prefixCnt > 0)
		{
			// 等待TXE
			while(1)
			{
				sr1cpy = Handle->Init.I2Cx->SR1;
				
				if(sr1cpy & I2C_SR1_AF) // AF
				{
					I2C_GenerateSTOP(Handle->Init.I2Cx, ENABLE);
					return ERROR;
				}
				
				if(sr1cpy & I2C_SR1_TXE)
				{
					break;
				}
			}
			// 发送数据
			I2C_SendData(Handle->Init.I2Cx, *pPrefix++);
			prefixCnt--;
			if(Handle->Init.Advanced.FrameInterval != 0)
			{
				PAL_DelayUs(Handle->Init.Advanced.FrameInterval);
			}
		}
	}
	
	// 使用DMA发送剩余的数据
	// 初始化DMA
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE); // 开启DMA1的时钟
	DMAInitStruct.DMA_PeripheralBaseAddr  = (uint32_t)&Handle->Init.I2Cx->DR;
	DMAInitStruct.DMA_MemoryBaseAddr = (uint32_t)Buffer;
	DMAInitStruct.DMA_DIR = DMA_DIR_PeripheralDST;
	DMAInitStruct.DMA_BufferSize = Size;
	DMAInitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMAInitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMAInitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
	DMAInitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMAInitStruct.DMA_Mode = DMA_Mode_Normal;
	DMAInitStruct.DMA_Priority = DMA_Priority_Medium;
	DMAInitStruct.DMA_M2M = DMA_M2M_Disable;
	
	DMA_Channel_TypeDef *dmax;
	
	if(Handle->Init.I2Cx == I2C1) // I2C1_Tx -> DMA1_Ch6
	{
		dmax = DMA1_Channel6;
	}
	else if(Handle->Init.I2Cx == I2C2) // I2C1_Tx -> DMA1_Ch4
	{
		dmax = DMA1_Channel4;
	}
	
	DMA_Init(dmax, &DMAInitStruct);
	
	// 使能DMA中断
	// 只使能“发送完成中断”和“发送出错中断”
	DMA_ITConfig(dmax, DMA_IT_TC | DMA_IT_TE, ENABLE);
	
	NVIC_InitTypeDef NVICInitStruct;
	if(Handle->Init.I2Cx == I2C1)
	{
		NVICInitStruct.NVIC_IRQChannel = DMA1_Channel6_IRQn;
	}
	else if(Handle->Init.I2Cx == I2C2)
	{
		NVICInitStruct.NVIC_IRQChannel = DMA1_Channel4_IRQn;
	}
	NVICInitStruct.NVIC_IRQChannelPreemptionPriority = 0;
	NVICInitStruct.NVIC_IRQChannelSubPriority = 0;
	NVICInitStruct.NVIC_IRQChannelCmd = ENABLE;
	
	NVIC_Init(&NVICInitStruct);
	
	DMA_Cmd(dmax, ENABLE); // 开启DMA通道
	
	I2C_DMACmd(Handle->Init.I2Cx, ENABLE); // 使能I2C的DMA
	
	// 此时数据已经开始发送
	return SUCCESS;
}

FlagStatus PAL_I2C_IsTxDMAInProgress(PalI2C_HandleTypeDef *Handle)
{
	if((RCC->AHBENR & RCC_AHBPeriph_DMA1) == 0)
	{
		return RESET;
	}
	
	DMA_Channel_TypeDef *dmax;
	
	if(Handle->Init.I2Cx == I2C1) // I2C1_Tx -> DMA1_Ch6
	{
		dmax = DMA1_Channel6;
	}
	else if(Handle->Init.I2Cx == I2C2)
	{
		dmax = DMA1_Channel4;
	}
	
	if((dmax->CCR & DMA_CCR1_EN) == 0)
	{
		return RESET;
	}
	
	if(DMA_GetCurrDataCounter(dmax) == 0)
	{
		return RESET;
	}
	
	return SET;
}

void PAL_I2C_TxDMAAbort(PalI2C_HandleTypeDef *Handle)
{
	__disable_irq();
	
	DMA_Channel_TypeDef *dmax;
	
	if(Handle->Init.I2Cx == I2C1) // I2C1_Tx -> DMA1_Ch6
	{
		dmax = DMA1_Channel6;
	}
	else if(Handle->Init.I2Cx == I2C2)
	{
		dmax = DMA1_Channel4;
	}
	
	DMA_Cmd(dmax, DISABLE);
	
	if(Handle->Init.I2Cx == I2C1)
	{
		//关闭中断
		DMA_ITConfig(DMA1_Channel6, DMA_IT_TC, DISABLE);
		DMA_ITConfig(DMA1_Channel6, DMA_IT_TE, DISABLE);
		// 清除中断标志位
		DMA_ClearITPendingBit(DMA1_IT_TC6);
		DMA_ClearITPendingBit(DMA1_IT_TE6); 
		// 关I2C DMAEN
		I2C_DMACmd(Handle->Init.I2Cx, DISABLE);
		// 关中断
		NVIC_InitTypeDef NVICInitStruct;
		NVICInitStruct.NVIC_IRQChannel = DMA1_Channel6_IRQn;
		NVICInitStruct.NVIC_IRQChannelCmd = DISABLE;
		NVIC_Init(&NVICInitStruct);
	}
	else if(Handle->Init.I2Cx == I2C2)
	{
		//关闭中断
		DMA_ITConfig(DMA1_Channel4, DMA_IT_TC, DISABLE);
		DMA_ITConfig(DMA1_Channel4, DMA_IT_TE, DISABLE);
		// 清除中断标志位
		DMA_ClearITPendingBit(DMA1_IT_TC4);
		DMA_ClearITPendingBit(DMA1_IT_TE4); 
		// 关I2C DMAEN
		I2C_DMACmd(Handle->Init.I2Cx, DISABLE);
		// 关中断
		NVIC_InitTypeDef NVICInitStruct;
		NVICInitStruct.NVIC_IRQChannel = DMA1_Channel6_IRQn;
		NVICInitStruct.NVIC_IRQChannelCmd = DISABLE;
		NVIC_Init(&NVICInitStruct);
	}
	
	__enable_irq();
	
	// 无论如何发送一个STOP
	I2C_GenerateSTOP(Handle->Init.I2Cx, ENABLE);
}

void PAL_I2C_DMATxIRQHandler(PalI2C_HandleTypeDef *Handle)
{
	DMA_Channel_TypeDef *dmax;
	
	if(Handle->Init.I2Cx == I2C1) // I2C1_Tx -> DMA1_Ch6
	{
		dmax = DMA1_Channel6;
		
		// 发送完成或发送出错
		if(DMA_GetITStatus(DMA1_IT_TC6) == SET || DMA_GetITStatus(DMA1_IT_TE6) == SET)
		{
			//关闭中断
			DMA_ITConfig(DMA1_Channel6, DMA_IT_TC, DISABLE);
			DMA_ITConfig(DMA1_Channel6, DMA_IT_TE, DISABLE);
			// 清除中断标志位
			DMA_ClearITPendingBit(DMA1_IT_TC6);
			DMA_ClearITPendingBit(DMA1_IT_TE6);
			// 关DMA
			DMA_Cmd(dmax, DISABLE);
			// 关I2C DMAEN
			I2C_DMACmd(Handle->Init.I2Cx, DISABLE);
			// 关中断
			NVIC_InitTypeDef NVICInitStruct;
			NVICInitStruct.NVIC_IRQChannel = DMA1_Channel6_IRQn;
			NVICInitStruct.NVIC_IRQChannelCmd = DISABLE;
			NVIC_Init(&NVICInitStruct);
			
			// 发送STOP
			I2C_GenerateSTOP(Handle->Init.I2Cx, ENABLE);
		}
	}
	else if(Handle->Init.I2Cx == I2C2) // I2C1_Tx -> DMA1_Ch4
	{
		dmax = DMA1_Channel4;
		
		// 发送完成或发送出错
		if(DMA_GetITStatus(DMA1_IT_TC4) == SET || DMA_GetITStatus(DMA1_IT_TE4) == SET)
		{
			//关闭中断
			DMA_ITConfig(DMA1_Channel4, DMA_IT_TC, DISABLE);
			DMA_ITConfig(DMA1_Channel4, DMA_IT_TE, DISABLE);
			// 清除中断标志位
			DMA_ClearITPendingBit(DMA1_IT_TC4);
			DMA_ClearITPendingBit(DMA1_IT_TE4); 
			DMA_Cmd(dmax, DISABLE);
			// 关I2C DMAEN
			I2C_DMACmd(Handle->Init.I2Cx, DISABLE);
			// 关中断
			NVIC_InitTypeDef NVICInitStruct;
			NVICInitStruct.NVIC_IRQChannel = DMA1_Channel4_IRQn;
			NVICInitStruct.NVIC_IRQChannelCmd = DISABLE;
			NVIC_Init(&NVICInitStruct);
			
			// 发送STOP
			I2C_GenerateSTOP(Handle->Init.I2Cx, ENABLE);
		}
	}
	else
	{
		return;
	}
}

FlagStatus PAL_I2C_TestConnection(PalI2C_HandleTypeDef *Handle, uint16_t DevAddr)
{
	uint32_t sr1cpy; // SR1寄存器的拷贝值
	
	// 如果DMA正在传送数据
	// 先终止掉之前的传输过程
	if(PAL_I2C_IsTxDMAInProgress(Handle))
	{
		PAL_I2C_TxDMAAbort(Handle);
	}
	
	// 等待总线空闲
	while(I2C_GetFlagStatus(Handle->Init.I2Cx, I2C_FLAG_BUSY) != RESET);
	// 发送start
	I2C_GenerateSTART(Handle->Init.I2Cx, ENABLE);
	// 等待SB置位
	while(I2C_GetFlagStatus(Handle->Init.I2Cx, I2C_FLAG_SB) != SET);	
	// 发送地址
	if(Handle->Init.Advanced.Use10BitAddress != SET)
	{
		// 向DR写入slave地址
		I2C_SendData(Handle->Init.I2Cx, DevAddr & 0xfe);
		
		// 等待ADDR
		while(1)
		{
			sr1cpy = Handle->Init.I2Cx->SR1;
			// 先检查AF
			if((sr1cpy & I2C_SR1_AF) != 0) // NACK，寻址失败
      {
			  I2C_GenerateSTOP(Handle->Init.I2Cx, ENABLE);
				// 清除ACK
				I2C_ClearFlag(Handle->Init.I2Cx, I2C_FLAG_AF);
				
				return RESET; // 寻址失败
			}
			// 等待ADDR标志位置位
			// ADDR标志位置位代表地址发送完成
			if((sr1cpy & I2C_SR1_ADDR) != 0) // ADDR置位
			{
				// 清除ADDR标志位
				// 先读sr1再读sr2
				sr1cpy = Handle->Init.I2Cx->SR1;
				sr1cpy = Handle->Init.I2Cx->SR2;
				break;
			}
		}
	}
	else
	{
		// 首先发送header
		// 11110xx0，其中xx代表地址的高2位
		I2C_SendData(Handle->Init.I2Cx, (uint8_t)(((DevAddr & 0x0300) >> 7) | 0xf0));
		
		// 等待ADDR10
		while(1)
		{
			sr1cpy = Handle->Init.I2Cx->SR1;
			// 先检查AF
			if((sr1cpy & I2C_SR1_AF) != 0) // NACK，寻址失败
      {
			  I2C_GenerateSTOP(Handle->Init.I2Cx, ENABLE);
				// 清除ACK
				I2C_ClearFlag(Handle->Init.I2Cx, I2C_FLAG_AF);
				
				return RESET; // 寻址失败
			}
			// 等待ADD10标志位
			if((sr1cpy & I2C_SR1_ADD10) != 0) // ADDR置位
			{
				// 清除ADD1标志位
				// 先读SR1（已读取）再写DR
				break;
			}
		}
		
		// 发送地址的低位
		I2C_SendData(Handle->Init.I2Cx, (DevAddr & 0xff) & 0xfe);
		
		// 等待ADDR
		while(1)
		{
			sr1cpy = Handle->Init.I2Cx->SR1;
			// 先检查AF
			if((sr1cpy & I2C_SR1_AF) != 0) // NACK，寻址失败
      {
			  I2C_GenerateSTOP(Handle->Init.I2Cx, ENABLE);
				// 清除ACK
				I2C_ClearFlag(Handle->Init.I2Cx, I2C_FLAG_AF);
				
				return RESET; // 寻址失败
			}
			// 等待ADDR标志位
			if((sr1cpy & I2C_SR1_ADDR) != 0) // ADDR置位
			{
				// 清除ADDR标志位
				// 先读sr1再读sr2
				sr1cpy = Handle->Init.I2Cx->SR1;
				sr1cpy = Handle->Init.I2Cx->SR2;
				break;
			}
		}
	}
	I2C_GenerateSTOP(Handle->Init.I2Cx, ENABLE);
	return SET;
}

ErrorStatus PAL_I2C_MasterTransmit(PalI2C_HandleTypeDef *Handle, uint16_t DevAddr, const uint8_t *pData, uint16_t Size)
{
	uint32_t sr1cpy; // SR1寄存器的拷贝值
	
	// 等待总线空闲
	while(I2C_GetFlagStatus(Handle->Init.I2Cx, I2C_FLAG_BUSY) != RESET);
	
	// 发送Start
	I2C_GenerateSTART(Handle->Init.I2Cx, ENABLE);
	
	// 等待SB置位
	while(I2C_GetFlagStatus(Handle->Init.I2Cx, I2C_FLAG_SB) != SET);
	
	// 发送地址
	if(Handle->Init.Advanced.Use10BitAddress != SET)
	{
		// 向DR写入slave地址
		I2C_SendData(Handle->Init.I2Cx, DevAddr & 0xfe);
		
		// 等待ADDR
		while(1)
		{
			sr1cpy = Handle->Init.I2Cx->SR1;
			// 先检查AF
			if((sr1cpy & I2C_SR1_AF) != 0) // NACK，寻址失败
      {
			  I2C_GenerateSTOP(Handle->Init.I2Cx, ENABLE);
				// 清除ACK
				I2C_ClearFlag(Handle->Init.I2Cx, I2C_FLAG_AF);
				
				return ERROR; // 寻址失败
			}
			// 等待ADDR标志位置位
			// ADDR标志位置位代表地址发送完成
			if((sr1cpy & I2C_SR1_ADDR) != 0) // ADDR置位
			{
				// 清除ADDR标志位
				// 先读sr1再读sr2
				sr1cpy = Handle->Init.I2Cx->SR1;
				sr1cpy = Handle->Init.I2Cx->SR2;
				break;
			}
		}
	}
	else
	{
		// 首先发送header
		// 11110xx0，其中xx代表地址的高2位
		I2C_SendData(Handle->Init.I2Cx, (uint8_t)(((DevAddr & 0x0300) >> 7) | 0xf0));
		
		// 等待ADDR10
		while(1)
		{
			sr1cpy = Handle->Init.I2Cx->SR1;
			// 先检查AF
			if((sr1cpy & I2C_SR1_AF) != 0) // NACK，寻址失败
      {
			  I2C_GenerateSTOP(Handle->Init.I2Cx, ENABLE);
				// 清除ACK
				I2C_ClearFlag(Handle->Init.I2Cx, I2C_FLAG_AF);
				
				return ERROR; // 寻址失败
			}
			// 等待ADD10标志位
			if((sr1cpy & I2C_SR1_ADD10) != 0) // ADDR置位
			{
				// 清除ADD1标志位
				// 先读SR1（已读取）再写DR
				break;
			}
		}
		
		// 发送地址的低位
		I2C_SendData(Handle->Init.I2Cx, (uint8_t)(DevAddr & 0xfe));
		
		// 等待ADDR
		while(1)
		{
			sr1cpy = Handle->Init.I2Cx->SR1;
			// 先检查AF
			if((sr1cpy & I2C_SR1_AF) != 0) // NACK，寻址失败
      {
			  I2C_GenerateSTOP(Handle->Init.I2Cx, ENABLE);
				// 清除ACK
				I2C_ClearFlag(Handle->Init.I2Cx, I2C_FLAG_AF);
				
				return ERROR; // 寻址失败
			}
			// 等待ADDR标志位
			if((sr1cpy & I2C_SR1_ADDR) != 0) // ADDR置位
			{
				// 清除ADDR标志位
				// 先读sr1再读sr2
				sr1cpy = Handle->Init.I2Cx->SR1;
				sr1cpy = Handle->Init.I2Cx->SR2;
				break;
			}
		}
	}
	
	// 发送前缀
	if(Handle->Init.Advanced.SendPrefix == SET)
	{
		const uint8_t *pPrefix = Handle->Init.Advanced.pPrefix;
		uint16_t prefixCnt = Handle->Init.Advanced.PrefixSize;
		// 等待TXE
		while(prefixCnt > 0)
		{
			// 等待TXE
			while(1)
			{
				sr1cpy = Handle->Init.I2Cx->SR1;
				
				if(sr1cpy & I2C_SR1_AF) // AF
				{
					I2C_GenerateSTOP(Handle->Init.I2Cx, ENABLE);
					return ERROR;
				}
				
				if(sr1cpy & I2C_SR1_TXE)
				{
					break;
				}
			}
			// 发送数据
			I2C_SendData(Handle->Init.I2Cx, *pPrefix++);
			prefixCnt--;
			if(Handle->Init.Advanced.FrameInterval != 0)
			{
				PAL_DelayUs(Handle->Init.Advanced.FrameInterval);
			}
		}
	}
	
	// 发送数据
	while(Size > 0)
	{
		// 等待TXE
		while(1)
		{
			sr1cpy = Handle->Init.I2Cx->SR1;
			
			if(sr1cpy & I2C_SR1_AF) // AF
			{
				I2C_GenerateSTOP(Handle->Init.I2Cx, ENABLE);
				return ERROR;
			}
			
			if(sr1cpy & I2C_SR1_TXE)
			{
				break;
			}
		}
		// 发送数据
		I2C_SendData(Handle->Init.I2Cx, *pData);
		
		pData++;
		Size--;
		if(Size > 0 &&Handle->Init.Advanced.FrameInterval != 0)
		{
			PAL_DelayUs(Handle->Init.Advanced.FrameInterval);
		}
	}
	
	// 停止发送
	// 等待BTF
	while(1)
	{
		sr1cpy = Handle->Init.I2Cx->SR1;
		
		if(sr1cpy & I2C_SR1_AF) // AF
		{
			I2C_GenerateSTOP(Handle->Init.I2Cx, ENABLE);
			return ERROR;
		}
		
		if(sr1cpy & I2C_SR1_BTF)
		{
			break;
		}
	}
	
	// 发送停止条件
	I2C_GenerateSTOP(Handle->Init.I2Cx, ENABLE);
	
	return SUCCESS;
}

ErrorStatus PAL_I2C_MasterReceive(PalI2C_HandleTypeDef *Handle, uint16_t DevAddr, uint8_t *Buffer, uint16_t Size)
{
	uint32_t sr1cpy; // SR1寄存器的拷贝值
	
	// 等待总线空闲
	while(I2C_GetFlagStatus(Handle->Init.I2Cx, I2C_FLAG_BUSY) != RESET);
	
	// 发送Start
	I2C_GenerateSTART(Handle->Init.I2Cx, ENABLE);
	
	// 等待SB置位
	while(I2C_GetFlagStatus(Handle->Init.I2Cx, I2C_FLAG_SB) != SET);
	
	// 发送地址
	if(Handle->Init.Advanced.Use10BitAddress != SET)
	{
		// 向DR写入slave地址
		I2C_SendData(Handle->Init.I2Cx, DevAddr | 0x01);
		
		// 等待ADDR
		while(1)
		{
			sr1cpy = Handle->Init.I2Cx->SR1;
			// 先检查AF
			if((sr1cpy & I2C_SR1_AF) != 0) // NACK，寻址失败
			{
			  I2C_GenerateSTOP(Handle->Init.I2Cx, ENABLE);
				// 清除ACK
				I2C_ClearFlag(Handle->Init.I2Cx, I2C_FLAG_AF);
				
				return ERROR; // 寻址失败
			}
			// 等待ADDR标志位置位
			// ADDR标志位置位代表地址发送完成
			if((sr1cpy & I2C_SR1_ADDR) != 0) // ADDR置位
			{
				break;
			}
		}
	}
	else
	{
		// 首先发送header
		// 11110xx0，其中xx代表地址的高2位
		I2C_SendData(Handle->Init.I2Cx, (uint8_t)(((DevAddr & 0x0300) >> 7) | 0xf0));
		
		// 等待ADDR10
		while(1)
		{
			sr1cpy = Handle->Init.I2Cx->SR1;
			// 先检查AF
			if((sr1cpy & I2C_SR1_AF) != 0) // NACK，寻址失败
      {
			  I2C_GenerateSTOP(Handle->Init.I2Cx, ENABLE);
				// 清除ACK
				I2C_ClearFlag(Handle->Init.I2Cx, I2C_FLAG_AF);
				
				return ERROR; // 寻址失败
			}
			// 等待ADD10标志位
			if((sr1cpy & I2C_SR1_ADD10) != 0) // ADDR置位
			{
				// 清除ADD1标志位
				// 先读SR1（已读取）再写DR
				break;
			}
		}
		
		// 发送地址的低位
		I2C_SendData(Handle->Init.I2Cx, (DevAddr & 0xff) | 0x01);
		
		// 等待ADDR
		while(1)
		{
			sr1cpy = Handle->Init.I2Cx->SR1;
			// 先检查AF
			if((sr1cpy & I2C_SR1_AF) != 0) // NACK，寻址失败
      {
			  I2C_GenerateSTOP(Handle->Init.I2Cx, ENABLE);
				// 清除ACK
				I2C_ClearFlag(Handle->Init.I2Cx, I2C_FLAG_AF);
				
				return ERROR; // 寻址失败
			}
			// 等待ADDR标志位
			if((sr1cpy & I2C_SR1_ADDR) != 0) // ADDR置位
			{
				// 清除ADDR标志位
				// 先读sr1再读sr2
				sr1cpy = Handle->Init.I2Cx->SR1;
				sr1cpy = Handle->Init.I2Cx->SR2;
				break;
			}
		}
		
		// 发送ReStart
		I2C_GenerateSTART(Handle->Init.I2Cx, ENABLE);
		
		// 等待SB置位
		while(I2C_GetFlagStatus(Handle->Init.I2Cx, I2C_FLAG_SB) != SET);		
		
		// 发送header
		// 11110xx1，其中xx代表地址的高2位
		I2C_SendData(Handle->Init.I2Cx, (uint8_t)(((DevAddr & 0x0300) >> 7) | 0xf1));
		
		// 等待ADDR
		while(1)
		{
			sr1cpy = Handle->Init.I2Cx->SR1;
			// 先检查AF
			if((sr1cpy & I2C_SR1_AF) != 0) // NACK，寻址失败
      {
			  I2C_GenerateSTOP(Handle->Init.I2Cx, ENABLE);
				// 清除ACK
				I2C_ClearFlag(Handle->Init.I2Cx, I2C_FLAG_AF);
				
				return ERROR; // 寻址失败
			}
			// 等待ADDR标志位
			if((sr1cpy & I2C_SR1_ADDR) != 0) // ADDR置位
			{
				break;
			}
		}
	}
	// 开始接收数据
	if(Size == 0)
	{
		// 1. 清除ADDR标志位
		sr1cpy = Handle->Init.I2Cx->SR1;
		sr1cpy = Handle->Init.I2Cx->SR2;
		// 2. 发送停止位
		I2C_GenerateSTOP(Handle->Init.I2Cx, ENABLE);
	}
	else if(Size == 1)
	{
		// 1. NACK
		Handle->Init.I2Cx->CR1 &= ~I2C_CR1_POS;
		Handle->Init.I2Cx->CR1 &= ~I2C_CR1_ACK;
		// 2. 清除ADDR标志位
		// 先读sr1再读sr2
		sr1cpy = Handle->Init.I2Cx->SR1;
		sr1cpy = Handle->Init.I2Cx->SR2;
		// 3. 发送结束标志
		I2C_GenerateSTOP(Handle->Init.I2Cx, ENABLE);
		// 4. 等待RxNE，读取数据
		while(I2C_GetFlagStatus(Handle->Init.I2Cx, I2C_FLAG_RXNE) != SET);
		*Buffer = I2C_ReceiveData(Handle->Init.I2Cx);
	}
	else if(Size == 2)
	{
		// 1. ACK = POS = 1
		Handle->Init.I2Cx->CR1 |= I2C_CR1_ACK;
		Handle->Init.I2Cx->CR1 |= I2C_CR1_POS;
		// 2. 清除ADDR标志位
		// 先读sr1再读sr2
		sr1cpy = Handle->Init.I2Cx->SR1;
		sr1cpy = Handle->Init.I2Cx->SR2;
		// 3. 清除ACK
		Handle->Init.I2Cx->CR1 &= ~I2C_CR1_ACK;
		// 4. 等待BTF置位
		while(I2C_GetFlagStatus(Handle->Init.I2Cx, I2C_FLAG_BTF) != SET);
		// 5. 发送停止条件
		I2C_GenerateSTOP(Handle->Init.I2Cx, ENABLE);
		// 6. 连续两次读取数据
		*Buffer = I2C_ReceiveData(Handle->Init.I2Cx);
		
		if(Handle->Init.Advanced.FrameInterval != 0)
		{
			PAL_DelayUs(Handle->Init.Advanced.FrameInterval);
		}
		
		*(Buffer+1) = I2C_ReceiveData(Handle->Init.I2Cx);
	}
	else // N > 2
	{
		Handle->Init.I2Cx->CR1 |= I2C_CR1_ACK;
		while(Size > 2)
		{
			while(I2C_GetFlagStatus(Handle->Init.I2Cx, I2C_FLAG_RXNE) != SET); // 等待RxNE = 1
			*Buffer++ = I2C_ReceiveData(Handle->Init.I2Cx);
		}
		// DataN-2暂停接收，等待BTF
		while(I2C_GetFlagStatus(Handle->Init.I2Cx, I2C_FLAG_BTF) != SET);
		// 清除ACK
		Handle->Init.I2Cx->CR1 &= ~I2C_CR1_ACK;
		// 读取DataN-2
		*Buffer++ = I2C_ReceiveData(Handle->Init.I2Cx);
		// 发送STOP
		I2C_GenerateSTOP(Handle->Init.I2Cx, ENABLE);
		*Buffer++ = I2C_ReceiveData(Handle->Init.I2Cx);
		
		if(Handle->Init.Advanced.FrameInterval != 0)
		{
			PAL_DelayUs(Handle->Init.Advanced.FrameInterval);
		}
	}
	return SUCCESS;
}

ErrorStatus PAL_I2C_MemWrite(PalI2C_HandleTypeDef *Handle, uint16_t DevAddr, uint16_t MemAddr, uint16_t MemAddrSize, const uint8_t *pData, uint16_t Size)
{
	uint32_t sr1cpy; // SR1寄存器的拷贝值
	
	// 等待总线空闲
	while(I2C_GetFlagStatus(Handle->Init.I2Cx, I2C_FLAG_BUSY) != RESET);
	
	// 清除POS位
	Handle->Init.I2Cx->CR1 &= ~I2C_CR1_POS;
	
	// 发送Start
	I2C_GenerateSTART(Handle->Init.I2Cx, ENABLE);
	
	// 等待SB置位
	while(I2C_GetFlagStatus(Handle->Init.I2Cx, I2C_FLAG_SB) != SET);
	
	// 发送地址

	// 向DR写入slave地址
	I2C_SendData(Handle->Init.I2Cx, DevAddr & 0xfe);
	
	// 等待ADDR
	while(1)
	{
		sr1cpy = Handle->Init.I2Cx->SR1;
		// 先检查AF
		if((sr1cpy & I2C_SR1_AF) != 0) // NACK，寻址失败
		{
			I2C_GenerateSTOP(Handle->Init.I2Cx, ENABLE);
			// 清除ACK
			I2C_ClearFlag(Handle->Init.I2Cx, I2C_FLAG_AF);
			
			return ERROR; // 寻址失败
		}
		// 等待ADDR标志位置位
		// ADDR标志位置位代表地址发送完成
		if((sr1cpy & I2C_SR1_ADDR) != 0) // ADDR置位
		{
			// 清除ADDR标志位
			// 先读sr1再读sr2
			sr1cpy = Handle->Init.I2Cx->SR1;
			sr1cpy = Handle->Init.I2Cx->SR2;
			break;
		}
	}
	
	// 等待TXE置位
	while(I2C_GetFlagStatus(Handle->Init.I2Cx, I2C_FLAG_TXE) != SET);
	
	// 发送MemAddr(内存地址)
	if(MemAddrSize == I2C_MEMSIZE_8BIT) // 8位地址
	{
		I2C_SendData(Handle->Init.I2Cx, (uint8_t)(MemAddr & 0xff));
	}
	else // 16位地址
	{
		I2C_SendData(Handle->Init.I2Cx, (uint8_t)((MemAddr >> 8) & 0xff));
		while(I2C_GetFlagStatus(Handle->Init.I2Cx, I2C_FLAG_TXE) != SET)
		{
			if(I2C_GetFlagStatus(Handle->Init.I2Cx, I2C_FLAG_AF) == RESET)// NACKed
			{
				I2C_GenerateSTOP(Handle->Init.I2Cx, ENABLE);
				return ERROR;
			}
		}
		I2C_SendData(Handle->Init.I2Cx, (uint8_t)(MemAddr & 0xff));
	}
	
	// 发送前缀
	if(Handle->Init.Advanced.SendPrefix == SET)
	{
		const uint8_t *pPrefix = Handle->Init.Advanced.pPrefix;
		uint16_t prefixCnt = Handle->Init.Advanced.PrefixSize;
		// 等待TXE
		while(prefixCnt > 0)
		{
			// 等待TXE
			while(1)
			{
				sr1cpy = Handle->Init.I2Cx->SR1;
				
				if(sr1cpy & I2C_SR1_AF) // AF
				{
					I2C_GenerateSTOP(Handle->Init.I2Cx, ENABLE);
					return ERROR;
				}
				
				if(sr1cpy & I2C_SR1_TXE)
				{
					break;
				}
			}
			// 发送数据
			I2C_SendData(Handle->Init.I2Cx, *pPrefix++);
			prefixCnt--;
			if(Handle->Init.Advanced.FrameInterval != 0)
			{
				PAL_DelayUs(Handle->Init.Advanced.FrameInterval);
			}
		}
	}
	
	// 发送数据
	// 发送数据
	while(Size > 0)
	{
		// 等待TXE
		while(I2C_GetFlagStatus(Handle->Init.I2Cx, I2C_FLAG_TXE) != SET)
		{
			if(I2C_GetFlagStatus(Handle->Init.I2Cx, I2C_FLAG_AF) == RESET)// NACKed
			{
				I2C_GenerateSTOP(Handle->Init.I2Cx, ENABLE);
				return ERROR;
			}
		}
		// 发送数据
		I2C_SendData(Handle->Init.I2Cx, *pData);
		
		pData++;
		Size--;
		
//		if(I2C_GetFlagStatus(Handle->Init.I2Cx, I2C_FLAG_BTF) == SET && Size != 0)
//		{
//			if(Size > 0 && Handle->Init.Advanced.FrameInterval != 0)
//			{
//				PAL_DelayMicroSeconds(Handle->Init.Advanced.FrameInterval);
//			}
//			
//			I2C_SendData(Handle->Init.I2Cx, *pData);
//		
//			pData++;
//			Size--;
//		}
		
		if(Size > 0 && Handle->Init.Advanced.FrameInterval != 0)
		{
			PAL_DelayUs(Handle->Init.Advanced.FrameInterval);
		}
	}
	
	// 停止发送
	// 等待BTF
	while(I2C_GetFlagStatus(Handle->Init.I2Cx, I2C_FLAG_BTF) != SET)
	{
		if(I2C_GetFlagStatus(Handle->Init.I2Cx, I2C_FLAG_AF) == SET) // AF
		{
			I2C_GenerateSTOP(Handle->Init.I2Cx, ENABLE);
			return ERROR;
		}
	}
	
	// 发送停止条件
	I2C_GenerateSTOP(Handle->Init.I2Cx, ENABLE);
	
	return SUCCESS;	
}

ErrorStatus PAL_I2C_MemRead(PalI2C_HandleTypeDef *Handle, uint16_t DevAddr, uint16_t MemAddr, uint16_t MemAddrSize, uint8_t *Buffer, uint16_t Size)
{
	uint8_t sr1cpy;
	
	// 等待总线空闲
	while(I2C_GetFlagStatus(Handle->Init.I2Cx, I2C_FLAG_BUSY) != RESET);
	// ACK置位
	Handle->Init.I2Cx->CR1 &= ~I2C_CR1_POS;
	Handle->Init.I2Cx->CR1 |= I2C_CR1_ACK;
	// 发送start标志
	I2C_GenerateSTART(Handle->Init.I2Cx, ENABLE);
	// 等待SB置位
	while(I2C_GetFlagStatus(Handle->Init.I2Cx, I2C_FLAG_SB) != SET);
	
	// 发送地址
	// 向DR写入slave地址
	I2C_SendData(Handle->Init.I2Cx, DevAddr & 0xfe);
	
	// 等待ADDR
	while(1)
	{
		sr1cpy = Handle->Init.I2Cx->SR1;
		// 先检查AF
		if((sr1cpy & I2C_SR1_AF) != 0) // NACK，寻址失败
		{
			I2C_GenerateSTOP(Handle->Init.I2Cx, ENABLE);
			// 清除ACK
			I2C_ClearFlag(Handle->Init.I2Cx, I2C_FLAG_AF);
			
			return ERROR; // 寻址失败
		}
		// 等待ADDR标志位置位
		// ADDR标志位置位代表地址发送完成
		if((sr1cpy & I2C_SR1_ADDR) != 0) // ADDR置位
		{
			// 清除ADDR标志位
			// 先读sr1再读sr2
			sr1cpy = Handle->Init.I2Cx->SR1;
			sr1cpy = Handle->Init.I2Cx->SR2;
			break;
		}
	}
	
	// 等待TXE置位
	while(I2C_GetFlagStatus(Handle->Init.I2Cx, I2C_FLAG_TXE) != SET);
	
	// 发送MemAddr(内存地址)
	if(MemAddrSize == I2C_MEMSIZE_8BIT) // 8位地址
	{
		I2C_SendData(Handle->Init.I2Cx, (uint8_t)(MemAddr & 0xff));
	}
	else // 16位地址
	{
		I2C_SendData(Handle->Init.I2Cx, (uint8_t)((MemAddr >> 8) & 0xff));
		while(I2C_GetFlagStatus(Handle->Init.I2Cx, I2C_FLAG_TXE) != SET)
		{
			if(I2C_GetFlagStatus(Handle->Init.I2Cx, I2C_FLAG_AF) == RESET)// NACKed
			{
				I2C_GenerateSTOP(Handle->Init.I2Cx, ENABLE);
				return ERROR;
			}
		}
		I2C_SendData(Handle->Init.I2Cx, (uint8_t)(MemAddr & 0xff));
	}
	while(I2C_GetFlagStatus(Handle->Init.I2Cx, I2C_FLAG_TXE) != SET)
	{
		if(I2C_GetFlagStatus(Handle->Init.I2Cx, I2C_FLAG_AF) == RESET)// NACKed
		{
			I2C_GenerateSTOP(Handle->Init.I2Cx, ENABLE);
			return ERROR;
		}
	}
	// 发送Restart
	I2C_GenerateSTART(Handle->Init.I2Cx, ENABLE);
	// 等待SB置位
	while(I2C_GetFlagStatus(Handle->Init.I2Cx, I2C_FLAG_SB) != SET);
	// 发送设备地址（读）
	I2C_SendData(Handle->Init.I2Cx, DevAddr | 0x01);
	
	// 等待ADDR
	while(1)
	{
		sr1cpy = Handle->Init.I2Cx->SR1;
		// 先检查AF
		if((sr1cpy & I2C_SR1_AF) != 0) // NACK，寻址失败
		{
			I2C_GenerateSTOP(Handle->Init.I2Cx, ENABLE);
			// 清除ACK
			I2C_ClearFlag(Handle->Init.I2Cx, I2C_FLAG_AF);
			
			return ERROR; // 寻址失败
		}
		// 等待ADDR标志位置位
		// ADDR标志位置位代表地址发送完成
		if((sr1cpy & I2C_SR1_ADDR) != 0) // ADDR置位
		{
			break;
		}
	}
	
	// 开始接收数据
	if(Size == 0)
	{
		// 1. 清除ADDR标志位
		sr1cpy = Handle->Init.I2Cx->SR1;
		sr1cpy = Handle->Init.I2Cx->SR2;
		// 2. 发送停止位
		I2C_GenerateSTOP(Handle->Init.I2Cx, ENABLE);
	}
	else if(Size == 1)
	{
		// 1. NACK
		Handle->Init.I2Cx->CR1 &= ~I2C_CR1_POS;
		Handle->Init.I2Cx->CR1 &= ~I2C_CR1_ACK;
		__disable_irq();
		// 2. 清除ADDR标志位
		// 先读sr1再读sr2
		sr1cpy = Handle->Init.I2Cx->SR1;
		sr1cpy = Handle->Init.I2Cx->SR2;
		// 3. 发送结束标志
		I2C_GenerateSTOP(Handle->Init.I2Cx, ENABLE);
		__enable_irq();
		// 4. 等待RxNE，读取数据
		while(I2C_GetFlagStatus(Handle->Init.I2Cx, I2C_FLAG_RXNE) != SET);
		*Buffer = I2C_ReceiveData(Handle->Init.I2Cx);
	}
	else if(Size == 2)
	{
		// 1. ACK = POS = 1
		Handle->Init.I2Cx->CR1 |= I2C_CR1_ACK;
		Handle->Init.I2Cx->CR1 |= I2C_CR1_POS;
		
		__disable_irq();
		
		// 2. 清除ADDR标志位
		// 先读sr1再读sr2
		sr1cpy = Handle->Init.I2Cx->SR1;
		sr1cpy = Handle->Init.I2Cx->SR2;
		// 3. 清除ACK
		Handle->Init.I2Cx->CR1 &= ~I2C_CR1_ACK;
		__enable_irq();
		
		// 4. 等待BTF置位
		while(I2C_GetFlagStatus(Handle->Init.I2Cx, I2C_FLAG_BTF) != SET);
		// 5. 发送停止条件
		I2C_GenerateSTOP(Handle->Init.I2Cx, ENABLE);
		// 6. 连续两次读取数据
		*Buffer = I2C_ReceiveData(Handle->Init.I2Cx);
		if(Handle->Init.Advanced.FrameInterval != 0)
		{
			PAL_DelayUs(Handle->Init.Advanced.FrameInterval);
		}
		*(Buffer+1) = I2C_ReceiveData(Handle->Init.I2Cx);
	}
	else // N > 2
	{
		Handle->Init.I2Cx->CR1 |= I2C_CR1_ACK;
		
		// 清除ADDR标志位
		// 先读sr1再读sr2
		sr1cpy = Handle->Init.I2Cx->SR1;
		sr1cpy = Handle->Init.I2Cx->SR2;
		
		while(Size > 2)
		{
			while(I2C_GetFlagStatus(Handle->Init.I2Cx, I2C_FLAG_RXNE) != SET); // 等待RxNE = 1
			*Buffer++ = I2C_ReceiveData(Handle->Init.I2Cx);
			Size--;
			if(Handle->Init.Advanced.FrameInterval != 0)
			{
				PAL_DelayUs(Handle->Init.Advanced.FrameInterval);
			}
		}
		// DataN-2暂停接收，等待BTF
		while(I2C_GetFlagStatus(Handle->Init.I2Cx, I2C_FLAG_BTF) != SET);
		// 清除ACK
		Handle->Init.I2Cx->CR1 &= ~I2C_CR1_ACK;
		// 读取DataN-2
		*Buffer++ = I2C_ReceiveData(Handle->Init.I2Cx);
		// 发送STOP
		I2C_GenerateSTOP(Handle->Init.I2Cx, ENABLE);
		*Buffer++ = I2C_ReceiveData(Handle->Init.I2Cx);
	}
	return SUCCESS;
}

