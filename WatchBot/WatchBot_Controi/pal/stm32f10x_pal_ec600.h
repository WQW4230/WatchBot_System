/**
  ******************************************************************************
  * @file    stm32f10x_pal_ec600.h
  * @author  铁头山羊stm32工作组
  * @version V1.0.0
  * @date    2023年2月24日
  * @brief   ec600驱动
  ******************************************************************************
  * @attention
  * Copyright (c) 2022 - 2023 东莞市明玉科技有限公司. 保留所有权力.
  ******************************************************************************
*/

#ifndef _STM32F10x_PAL_EC600_H_
#define _STM32F10x_PAL_EC600_H_

#include "stm32f10x.h"
#include "stm32f10x_pal_uart.h"

#define EC600_CONTEXTTYPE_IPv4 1
#define EC600_CONTEXTTYPE_IPv6 2
#define EC600_CONTEXTTYPE_IPv4v6 3

#define EC600_ACCESSMODE_CACHE ((uint8_t)0U)
#define EC600_ACCESSMODE_DIRECT ((uint8_t)1U)
#define EC600_ACCESSMODE_TRANSPARENT ((uint8_t)2U)

typedef struct
{
	uint32_t UARTRemap;
} PalEC600_AdvancedInitTypeDef;

typedef struct
{
	USART_TypeDef *UARTx;
	GPIO_TypeDef *RST_GPIOPort;
	uint16_t RST_GPIOPin;
	PalEC600_AdvancedInitTypeDef Advanced;
} PalEC600_InitTypeDef;

typedef struct
{
	PalEC600_InitTypeDef Init;
	PalUART_HandleTypeDef hUART;
} PalEC600_HandleTypeDef;

typedef struct
{
	uint8_t Bytes[16];
} PalIPv6_TypeDef;

typedef struct
{
	uint8_t Bytes[4];
} PalIPv4_TypeDef;

typedef struct
{
	uint8_t Id;
	FlagStatus Active; 
	uint8_t EC600ContextType; 
	FlagStatus HasIPv4;
	PalIPv4_TypeDef IPv4 ;
	FlagStatus HasIPv6;
	PalIPv6_TypeDef IPv6;
} PalEC600_PdpParametersTypeDef;

void PAL_IPv6ToStr(PalIPv6_TypeDef IPv6, char *pBuf);
ErrorStatus PAL_StrToIPv6(const char *str, PalIPv6_TypeDef *pIPv6);
void PAL_IPv4ToStr(PalIPv4_TypeDef IPv4, char *pBuf);
ErrorStatus PAL_StrToIPv4(const char *str, PalIPv4_TypeDef *pIPv4);

void PAL_EC600_Init(PalEC600_HandleTypeDef *Handle);
ErrorStatus PAL_EC600_Reset(PalEC600_HandleTypeDef *Handle);
ErrorStatus PAL_EC600_AT(PalEC600_HandleTypeDef *Handle);
ErrorStatus PAL_EC600_AT_CPIN_Q(PalEC600_HandleTypeDef *Handle);
ErrorStatus PAL_EC600_AT_QICSGP(PalEC600_HandleTypeDef *Handle, uint8_t ContextId, uint8_t ContextType, const char* APN, const char* UserName, const char* Pswd);
ErrorStatus PAL_EC600_AT_QIACT(PalEC600_HandleTypeDef *Handle, uint8_t ContextId);
ErrorStatus PAL_EC600_AT_QICLOSE(PalEC600_HandleTypeDef *Handle, uint8_t ContextId);
ErrorStatus PAL_EC600_PlusPlusPlus(PalEC600_HandleTypeDef *Handle);
ErrorStatus PAL_EC600_AT_QIOEPN(PalEC600_HandleTypeDef *Handle, uint8_t ContextId, uint8_t ConnectId, const char *ServiceType, const char* IP, uint16_t RemotePort, uint16_t LocalPort, uint8_t AccsessMode);
ErrorStatus PAL_EC600_AT_QIACT_Q(PalEC600_HandleTypeDef *Handle, PalEC600_PdpParametersTypeDef *pPdpParams, uint8_t *Size);
ErrorStatus PAL_EC600_AT_QISEND(PalEC600_HandleTypeDef *Handle, uint8_t ConnectionId, uint8_t *pData, uint32_t Size);
ErrorStatus PAL_EC600_AT_QIRD_GetLength(PalEC600_HandleTypeDef *Handle, uint8_t ConnectionId, uint32_t *pTotalRcvd, uint32_t *pHaveRead, uint32_t *pUnread);
ErrorStatus PAL_EC600_AT_QICFG_TransWtTm(PalEC600_HandleTypeDef *Handle, uint8_t n100mS);
ErrorStatus PAL_EC600_AT_QICFG_TransPktSize(PalEC600_HandleTypeDef *Handle, uint16_t Size);
ErrorStatus PAL_EC600_AT_IPR(PalEC600_HandleTypeDef *Handle, uint32_t Baudrate);
       void PAL_EC600_SendDataTransparent_DMA(PalEC600_HandleTypeDef *Handle, const uint8_t *pData, uint16_t Size);
       void PAL_EC600_FlushDMA(PalEC600_HandleTypeDef *Handle);

void PAL_EC600_UART_IRQHandler(PalEC600_HandleTypeDef *Handle);

#endif
