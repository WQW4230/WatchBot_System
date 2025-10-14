/**
  ******************************************************************************
  * @file    stm32f10x_pal_i2c.c
  * @author  铁头山羊stm32工作组
  * @version V1.0.0
  * @date    2023年2月24日
  * @brief   i2c总线驱动程序
  ******************************************************************************
  * @attention
  * Copyright (c) 2022 - 2023 东莞市明玉科技有限公司. 保留所有权力.
  ******************************************************************************
*/

#ifndef _PAL_BUFFERED_I2C_H_
#define _PAL_BUFFERED_I2C_H_

#include "stm32f10x.h"

#define I2C_MEMSIZE_8BIT  ((uint32_t)0x00)
#define I2C_MEMSIZE_16BIT ((uint32_t)0x01)

typedef struct
{
	FlagStatus Use10BitAddress; // 使用10位地址
	FlagStatus Use16_9_DutyCycle; // 使用16：9的占空比（只能在快速模式下使用）
	uint16_t FrameInterval;          /* 连续向从机读写数据时每个数据帧之间的时间间隔 
	                                  * 单位：微秒(us = 1e-6s)
	                                  * 一些I2C从机设计存在缺陷。它们既不支持时钟拉
	                                  * 伸（Clock Stratch）也不能承受高速的数据写入
	                                  *（连续写入时NAK），此时在连续写入时应当在每个
	                                  * 数据帧之间增加延迟。可设置此参数以增加延迟。
	                                  */
	uint32_t Remap;      /* 是否使用重映射 */
	FlagStatus SendPrefix; // 是否发送前缀
	const uint8_t *pPrefix;
	uint16_t PrefixSize;
} PalI2C_AdvancedParam;

typedef struct
{
	I2C_TypeDef *I2Cx;
	uint32_t I2C_ClockSpeed;
	PalI2C_AdvancedParam Advanced; /* 高级参数 */
}PalI2C_InitTypeDef;

typedef struct
{
	PalI2C_InitTypeDef Init;
}PalI2C_HandleTypeDef;

ErrorStatus PAL_I2C_Init(PalI2C_HandleTypeDef *Handle);
FlagStatus  PAL_I2C_TestConnection(PalI2C_HandleTypeDef *Handle, uint16_t DevAddr); 
ErrorStatus PAL_I2C_MasterTransmit(PalI2C_HandleTypeDef *Handle, uint16_t DevAddr, const uint8_t *pData, uint16_t Size);
ErrorStatus PAL_I2C_MasterReceive(PalI2C_HandleTypeDef *Handle, uint16_t DevAddr, uint8_t *Buffer, uint16_t Size);
ErrorStatus PAL_I2C_MemWrite(PalI2C_HandleTypeDef *Handle, uint16_t DevAddr, uint16_t MemAddr, uint16_t MemAddrSize, const uint8_t *pData, uint16_t Size);
ErrorStatus PAL_I2C_MemRead(PalI2C_HandleTypeDef *Handle, uint16_t DevAddr, uint16_t MemAddr, uint16_t MemAddrSize, uint8_t *Buffer, uint16_t Size);

ErrorStatus PAL_I2C_MasterTransmit_DMA(PalI2C_HandleTypeDef *Handle, uint16_t DevAddr, const uint8_t *Buffer, uint16_t Size);
 FlagStatus PAL_I2C_IsTxDMAInProgress(PalI2C_HandleTypeDef *Handle);
       void PAL_I2C_TxDMAAbort(PalI2C_HandleTypeDef *Handle);
       void PAL_I2C_DMATxIRQHandler(PalI2C_HandleTypeDef *Handle);
#endif
