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
#include "stm32f10x_pal.h"

#define I2C_MEMSIZE_8BIT  ((uint32_t)0x00)
#define I2C_MEMSIZE_16BIT ((uint32_t)0x01)

typedef struct
{
	uint32_t Remap;         /* 用于指定复用功能重映射的值 
	                           该参数的取值请查阅参考手册 */
	
} PalI2C_AdvancedInitParam;

typedef struct
{
	I2C_TypeDef *I2Cx;              /* 该参数用于填写所使用的I2C的名称。
	                                   根据使用情况可以填I2C1或者I2C2 */
	
	uint32_t I2C_ClockSpeed;        /* 该参数用于设置I2C总线的波特率。
	                                   最大可设置为400kbps(400000) */
	
	uint16_t I2C_DutyCycle;         /* 该参数用于设置SCL的占空比。
	                                   @I2C_DutyCycle_2 - Tl:Th = 2:1
	                                   @I2C_DutyCycle_16_9 - Tl:Th=16:9 */
	
	PalI2C_AdvancedInitParam Advanced;  /* 高级参数 */
}PalI2C_InitTypeDef;

typedef struct
{
	PalI2C_InitTypeDef Init;
	uint32_t ByteInterval;
}PalI2C_HandleTypeDef;

/* --------------------------------- 初始化 -------------------------------------------------- */
       void PAL_I2C_InitHandle(PalI2C_HandleTypeDef *Handle);
ErrorStatus PAL_I2C_Init(PalI2C_HandleTypeDef *Handle);

/* --------------------------------- 数据收发（7位从机地址） --------------------------------- */
ErrorStatus PAL_I2C_MasterTransmit(PalI2C_HandleTypeDef *Handle, uint16_t SlaveAddr, const uint8_t *pData, uint16_t Size);
ErrorStatus PAL_I2C_MasterReceive(PalI2C_HandleTypeDef *Handle, uint16_t SlaveAddr, uint8_t *pBufferOut, uint16_t Size);
ErrorStatus PAL_I2C_RegWrite(PalI2C_HandleTypeDef *Handle, uint16_t SlaveAddr, uint16_t RegAddr, const uint8_t Data);
ErrorStatus PAL_I2C_RegRead(PalI2C_HandleTypeDef *Handle, uint16_t SlaveAddr, uint16_t RegAddr, uint8_t *pDataOut);
ErrorStatus PAL_I2C_RegWriteBytes(PalI2C_HandleTypeDef *Handle, uint16_t SlaveAddr, uint16_t RegAddr, const uint8_t *pData, uint16_t Size);
ErrorStatus PAL_I2C_RegReadBytes(PalI2C_HandleTypeDef *Handle, uint16_t SlaveAddr, uint16_t RegAddr, uint8_t *pBufferOut, uint16_t Size);

/* --------------------------------- 数据收发（10位从机地址） --------------------------------- */
ErrorStatus PAL_I2C_RegWrite10(PalI2C_HandleTypeDef *Handle, uint16_t SlaveAddr, uint16_t RegAddr, const uint8_t Data);
ErrorStatus PAL_I2C_RegRead10(PalI2C_HandleTypeDef *Handle, uint16_t SlaveAddr, uint16_t RegAddr, uint8_t *pDataOut);
ErrorStatus PAL_I2C_RegWriteBytes10(PalI2C_HandleTypeDef *Handle, uint16_t SlaveAddr, uint16_t RegAddr, const uint8_t *pData, uint16_t Size);
ErrorStatus PAL_I2C_RegReadBytes10(PalI2C_HandleTypeDef *Handle, uint16_t SlaveAddr, uint16_t RegAddr, uint8_t *pBufferOut, uint16_t Size);
ErrorStatus PAL_I2C_MasterTransmit10(PalI2C_HandleTypeDef *Handle, uint16_t SlaveAddr, const uint8_t *pData, uint16_t Size);
ErrorStatus PAL_I2C_MasterReceive10(PalI2C_HandleTypeDef *Handle, uint16_t SlaveAddr, uint8_t *pBufferOut, uint16_t Size);

       void PAL_I2C_SetByteInterval(PalI2C_HandleTypeDef *Handle, uint32_t Interval);

/* ----------------------------------分段数据发送---------------------------------------------- */
ErrorStatus PAL_I2C_StartTx(PalI2C_HandleTypeDef *Handle, uint16_t SlaveAddr);
ErrorStatus PAL_I2C_StartTx10(PalI2C_HandleTypeDef *Handle, uint16_t SlaveAddr);
ErrorStatus PAL_I2C_SendByte(PalI2C_HandleTypeDef *Handle, uint8_t Byte);
ErrorStatus PAL_I2C_SendBytes(PalI2C_HandleTypeDef *Handle, const uint8_t *pData, uint16_t Size);
       void PAL_I2C_StopTx(PalI2C_HandleTypeDef *Handle);

#endif
