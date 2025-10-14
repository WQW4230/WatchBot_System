/**
  ******************************************************************************
  * @file    stm32f10x_pal_spi.h
  * @author  铁头山羊stm32工作组
  * @version V1.0.0
  * @date    2023年2月24日
  * @brief   spi总线驱动程序
  ******************************************************************************
  * @attention
  * Copyright (c) 2022 - 2023 东莞市明玉科技有限公司. 保留所有权力.
  ******************************************************************************
*/

#ifndef _STM32F10x_PAL_SPI_H_
#define _STM32F10x_PAL_SPI_H_

#include "stm32f10x.h"

typedef struct
{
	uint32_t Remap; /* GPIO重映射。
	                 * SPI1 Remap=0 Remap=1
	                 * NSS    PA4     PA15
	                 * SCK    PA5     PB3
	                 * MISO   PA6     PB4
	                 * MOSI   PA7     PB5
	                 * 
	                 * SPI2 不具有IO重映射功能
	                 */
	const uint8_t *pPrefix; 
	uint16_t PrefixSize;
}PalSPI_AdvancedInitTypeDef;

typedef struct
{
	SPI_TypeDef *SPIx; /* 使用哪个SPI外设，可选择以下值：
	                    * SPI1 - 使用SPI1
	                    * SPI2 - 使用SPI2
	                    */
	uint16_t SPI_CPOL; /* 时钟的极性，可选以下值
	                    * SPI_CPOL_LOW - 空闲时SCK呈现低电平
	                    * SPI_CPOL_HIGH - 空闲时SCK呈现高电平
	                    */
	uint16_t SPI_CPHA; /* 时钟的相位，可选以下值
	                    * SPI_CPHA_1Edge - SCK的第1个边沿采集数据
	                    * SPI_CPHA_2Edge - SCK的第2个边沿采集数据
	                    */
	uint16_t SPI_BaudRatePrescaler; /* 波特率分频系数。波特率 = PCLKx / 波特率分频系数。
	                                 * 可选择以下值：
	                                 * SPI_BaudRatePrescaler_2   -   2分频
	                                 * SPI_BaudRatePrescaler_4   -   4分频
	                                 * SPI_BaudRatePrescaler_8   -   8分频
	                                 * SPI_BaudRatePrescaler_16  -  16分频
	                                 * SPI_BaudRatePrescaler_32  -  32分频
	                                 * SPI_BaudRatePrescaler_64  -  64分频
	                                 * SPI_BaudRatePrescaler_128 - 128分频
	                                 * SPI_BaudRatePrescaler_256 - 256分频
	                                 */
  uint16_t SPI_FirstBit; /* SPI数据的传输顺序，可取以下值：
	                        * SPI_FirstBit_MSB - 先传输最高位
	                        * SPI_FirstBit_LSB - 先传输最低位
	                        */
  PalSPI_AdvancedInitTypeDef Advanced;
}PalSPI_InitTypeDef;

typedef struct
{
	PalSPI_InitTypeDef Init;
}PalSPI_HandleTypeDef;

void PAL_SPI_Init(PalSPI_HandleTypeDef *Handle);
void PAL_SPI_MasterTransmit(PalSPI_HandleTypeDef *Handle, const uint8_t *pData, uint16_t Size);
void PAL_SPI_MasterReceive(PalSPI_HandleTypeDef *Handle, uint8_t *pBuf, uint16_t Size);
void PAL_SPI_MasterTransmitReceive(PalSPI_HandleTypeDef *Handle, const uint8_t *pTxData, uint8_t *pRxData, uint16_t Size);

#endif
