/**
  ******************************************************************************
  * @file    stm32f10x_pal_w25q64.c
  * @author  铁头山羊stm32工作组
  * @version V1.0.0
  * @date    2023年2月24日
  * @brief   w25q64 flash存储器驱动
  ******************************************************************************
  * @attention
  * Copyright (c) 2022 - 2023 东莞市明玉科技有限公司. 保留所有权力.
  ******************************************************************************
*/

#include "stm32f10x_pal_w25q64.h"
#include "stm32f10x_pal.h"

/*
 * @brief 命令列表
 */
#define W25Q64XX_CMD_WRITE_ENABLE    ((uint8_t)0x06) /* Write enable */
#define W25Q64XX_CMD_WRITE_DISABLE   ((uint8_t)0x04) /* Write disable */
#define W25Q64XX_CMD_READ_SR1        ((uint8_t)0x05) /* Read status register 1 */
#define W25Q64XX_CMD_READ_SR2        ((uint8_t)0x35) /* Read status register 2 */
#define W25Q64XX_CMD_WRITE_SR        ((uint8_t)0x01) /* Write status registers */
#define W25Q64XX_CMD_PAGE_PRG        ((uint8_t)0x02) /* Page program */
#define W25Q64XX_CMD_SEC_ERASE       ((uint8_t)0x20) /* Sector erase */
#define W25Q64XX_CMD_BLK_ERASE_32    ((uint8_t)0x52) /* Block erase 32k */
#define W25Q64XX_CMD_BLK_ERASE_64    ((uint8_t)0xd8) /* Block erase 64k */
#define W25Q64XX_CMD_CHIP_ERASE_A    ((uint8_t)0xc7) /* Chip erase */
#define W25Q64XX_CMD_CHIP_ERASE_B    ((uint8_t)0x60) /* Chip erase */
#define W25Q64XX_CMD_ERASE_SUS       ((uint8_t)0x75) /* Erase suspend */
#define W25Q64XX_CMD_ERASE_RSM       ((uint8_t)0x7a) /* Erase resume */
#define W25Q64XX_CMD_POWER_DOWN      ((uint8_t)0xb9) /* Power down */
#define W25Q64XX_CMD_CONT_READ_MODE_RST      ((uint8_t)0xff) /* Continuous read mode reset */

#define W25Q64XX_CMD_READ_DATA       ((uint8_t)0x03) /* Read data */
#define W25Q64XX_CMD_FAST_READ       ((uint8_t)0x0b) /* Fast read */
#define W25Q64XX_CMD_RELEASE_PD      ((uint8_t)0xab) /* Release power down */
#define W25Q64XX_CMD_DEVICE_ID       W25Q16XX_CMD_RELEASE_PD /* Read 8 LSBs of device ID */
#define W25Q64XX_CMD_MANUFACTURER_DEVICE_ID   ((uint8_t)0x90) /* Read manufacturer and 8 LSBs of device ID */
#define W25Q64XX_CMD_JEDEC_ID        ((uint8_t)0x9f) /* Read JEDEC ID (MF[7..0] ID[15..8] ID[7..0])*/
#define W25Q64XX_CMD_UNIQUE_ID       ((uint8_t)0x4b) /* Read unique ID */

/*
 * @brief 状态寄存器1
 */
#define W25Q64XX_SR1_BUSY_Pos 0               /* Busy flag. Read only.  */
#define W25Q64XX_SR1_BUSY_Msk ((uint8_t)0x01)
#define W25Q64XX_SR1_WEL_Pos  1               /* Write enable latch flag. Read only.  */
#define W25Q64XX_SR1_WEL_Msk  ((uint8_t)0x02)
#define W25Q64XX_SR1_BP_Pos   2               /* Block protection bits.  W/R. Default [BP2,BP1,BP0] = 000b */
#define W25Q64XX_SR1_BP_Msk   ((uint8_t)0x1c)
#define W25Q64XX_SR1_BP0_Pos  2
#define W25Q64XX_SR1_BP0_Msk  ((uint8_t)0x04)
#define W25Q64XX_SR1_BP1_Pos  3
#define W25Q64XX_SR1_BP1_Msk  ((uint8_t)0x08)
#define W25Q64XX_SR1_BP2_Pos  4
#define W25Q64XX_SR1_BP2_Msk  ((uint8_t)0x10)
#define W25Q64XX_SR1_TB_Pos   5               /* Top/Bottom. TB=0, protect from top; TB=1, protect from bottom. Default 0 */
#define W25Q64XX_SR1_TB_Msk   ((uint8_t)0x20)
#define W25Q64XX_SR1_SEC_Pos  6               /* Sector/Block protect. SEC=0, protect 64k blocks; SEC=1, protect 4k sectors. Default 0 */
#define W25Q64XX_SR1_SEC_Msk  ((uint8_t)0x40)
#define W25Q64XX_SR1_SRP0_Pos 7
#define W25Q64XX_SR1_SRP0_Msk ((uint8_t)0x80)
/*
 * @brief 状态寄存器2
 */
#define W25Q64XX_SR2_SRP1_Pos 0
#define W25Q64XX_SR2_SRP1_Msk ((uint8_t)0x01)
#define W25Q64XX_SR2_QE_Pos   1               /* Quad Enable.  */
#define W25Q64XX_SR2_QE_Msk   ((uint8_t)0x02)
#define W25Q64XX_SR2_SUS_Pos  7               /* Erase Suspend Status. Read only.  */
#define W25Q64XX_SR2_SUS_Msk  ((uint8_t)0x80)

uint8_t PAL_W25Q64_ReadStatusRegister1(PalW25Q64_HandleTypeDef *Handle);
//static uint8_t PAL_W25Q64_ReadStatusRegister2(PalW25Q64_HandleTypeDef *Handle);
//static void PAL_W25Q64_WriteStatusRegister(PalW25Q64_HandleTypeDef *Handle, uint16_t NewValue);
static inline void SelectChip(PalW25Q64_HandleTypeDef *Handle);
static inline void DeselectChip(PalW25Q64_HandleTypeDef *Handle);

void PAL_W25Q64_Init(PalW25Q64_HandleTypeDef *Handle)
{
	GPIO_InitTypeDef GPIOInitStruct;
	
	RCC_GPIOx_ClkCmd(Handle->Init.SS_GPIO_Port, ENABLE);
	GPIOInitStruct.GPIO_Pin = Handle->Init.SS_GPIO_Pin;
	GPIOInitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIOInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_WriteBit(Handle->Init.SS_GPIO_Port, Handle->Init.SS_GPIO_Pin, Bit_SET);
	GPIO_Init(Handle->Init.SS_GPIO_Port, &GPIOInitStruct);
}

FlagStatus PAL_W25Q64_GetBusyFlag(PalW25Q64_HandleTypeDef *Handle)
{
	if(PAL_W25Q64_ReadStatusRegister1(Handle) & W25Q64XX_SR1_BUSY_Msk)
	{
		return SET;
	}
	else
	{
		return RESET;
	}
}

void PAL_W25Q64_WriteEnable(PalW25Q64_HandleTypeDef *Handle)
{
	uint8_t tmp = W25Q64XX_CMD_WRITE_ENABLE;
	SelectChip(Handle);
	PAL_SPI_MasterTransmit(Handle->Init.hspi, &tmp, 1);
	DeselectChip(Handle);
}

void PAL_W25Q64_WriteDisable(PalW25Q64_HandleTypeDef *Handle)
{
	uint8_t tmp = W25Q64XX_CMD_WRITE_DISABLE;
	SelectChip(Handle);
	PAL_SPI_MasterTransmit(Handle->Init.hspi, &tmp, 1);
}

void PAL_W25Q64_PageProgram(PalW25Q64_HandleTypeDef *Handle, uint32_t Addr, const uint8_t *pData, uint16_t Size)
{
	uint8_t tmp[] = {W25Q64XX_CMD_PAGE_PRG, (uint8_t)((Addr >> 16)&0xff), (uint8_t)((Addr >>  8)&0xff), (uint8_t)((Addr >>  0)&0xff)};
	SelectChip(Handle);
	PAL_SPI_MasterTransmit(Handle->Init.hspi, tmp, 4);
	PAL_SPI_MasterTransmit(Handle->Init.hspi, pData, Size);
	DeselectChip(Handle);
}

void PAL_W25Q64_SectorErase(PalW25Q64_HandleTypeDef *Handle, uint32_t Addr)
{
	uint8_t cmd[5];
	cmd[0] = W25Q64XX_CMD_SEC_ERASE;
	cmd[1] = (uint8_t)((Addr >> 16)&0xff);
	cmd[2] = (uint8_t)((Addr >>  8)&0xff);
	cmd[3] = (uint8_t)((Addr >>  0)&0xff);
	SelectChip(Handle);
	PAL_SPI_MasterTransmit(Handle->Init.hspi, cmd, 4);
	DeselectChip(Handle);
}

void PAL_W25Q64_BlockErase_32k(PalW25Q64_HandleTypeDef *Handle, uint32_t Addr)
{
		uint8_t cmd[5];
	cmd[0] = W25Q64XX_CMD_BLK_ERASE_32;
	cmd[1] = (uint8_t)((Addr >> 16)&0xff);
	cmd[2] = (uint8_t)((Addr >>  8)&0xff);
	cmd[3] = (uint8_t)((Addr >>  0)&0xff);
	SelectChip(Handle);
	PAL_SPI_MasterTransmit(Handle->Init.hspi, cmd, 4);
	DeselectChip(Handle);
}
void PAL_W25Q64_BlockErase_64k(PalW25Q64_HandleTypeDef *Handle, uint32_t Addr)
{
	uint8_t cmd[5];
	cmd[0] = W25Q64XX_CMD_BLK_ERASE_64;
	cmd[1] = (uint8_t)((Addr >> 16)&0xff);
	cmd[2] = (uint8_t)((Addr >>  8)&0xff);
	cmd[3] = (uint8_t)((Addr >>  0)&0xff);
	SelectChip(Handle);
	PAL_SPI_MasterTransmit(Handle->Init.hspi, cmd, 4);
	DeselectChip(Handle);
}

void PAL_W25Q64_Read(PalW25Q64_HandleTypeDef *Handle, uint32_t Addr, uint8_t *pDataOut, uint16_t Size)
{
	uint8_t cmd[4] = {W25Q64XX_CMD_READ_DATA, (uint8_t)((Addr >> 16)&0xff), (uint8_t)((Addr >> 8)&0xff), (uint8_t)((Addr >> 0)&0xff)};
	SelectChip(Handle);
	PAL_SPI_MasterTransmit(Handle->Init.hspi, cmd, 4);
	PAL_SPI_MasterReceive(Handle->Init.hspi, pDataOut, Size);
	DeselectChip(Handle);
}

uint8_t PAL_W25Q64_ReadStatusRegister1(PalW25Q64_HandleTypeDef *Handle)
{
	uint8_t cmd = W25Q64XX_CMD_READ_SR1;
	uint8_t ret = 0;
	SelectChip(Handle);
	PAL_SPI_MasterTransmit(Handle->Init.hspi, &cmd, 1);
	PAL_SPI_MasterReceive(Handle->Init.hspi, &ret, 1);
	DeselectChip(Handle);
	return ret;
}

static inline void SelectChip(PalW25Q64_HandleTypeDef *Handle)
{
	GPIO_WriteBit(Handle->Init.SS_GPIO_Port, Handle->Init.SS_GPIO_Pin, Bit_RESET);
}

static inline void DeselectChip(PalW25Q64_HandleTypeDef *Handle)
{
	GPIO_WriteBit(Handle->Init.SS_GPIO_Port, Handle->Init.SS_GPIO_Pin, Bit_SET);
}
