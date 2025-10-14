/**
  ******************************************************************************
  * @file    stm32f10x_pal_ec600.c
  * @author  铁头山羊stm32工作组
  * @version V1.0.0
  * @date    2023年2月24日
  * @brief   ec600驱动
  ******************************************************************************
  * @attention
  * Copyright (c) 2022 - 2023 东莞市明玉科技有限公司. 保留所有权力.
  ******************************************************************************
*/

#include "stm32f10x_pal_ec600.h"
#include "stm32f10x_pal.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

void PAL_EC600_Init(PalEC600_HandleTypeDef *Handle)
{
	// 初始化RST引脚
	RCC_GPIOx_ClkCmd(Handle->Init.RST_GPIOPort, ENABLE);
	
	GPIO_WriteBit(Handle->Init.RST_GPIOPort, Handle->Init.RST_GPIOPin, Bit_RESET); // deselect rst pin
	
	GPIO_InitTypeDef GPIOInitStruct;
	GPIOInitStruct.GPIO_Pin = Handle->Init.RST_GPIOPin;
	GPIOInitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIOInitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(Handle->Init.RST_GPIOPort, &GPIOInitStruct);
	
	// 初始化uart接口
	Handle->hUART.Init.USARTx = Handle->Init.UARTx;
	Handle->hUART.Init.Basic.BaudRate = 115200;
	Handle->hUART.Init.Basic.USART_Parity = USART_Parity_No;
	Handle->hUART.Init.Basic.USART_StopBits = USART_StopBits_1;
	Handle->hUART.Init.Basic.USART_WordLength = USART_WordLength_8b;
	Handle->hUART.Init.Tx.Enable = SET;
	Handle->hUART.Init.Tx.DMA.Enable = SET;
	Handle->hUART.Init.Tx.DMA.QueueSize = 256;
	Handle->hUART.Init.Rx.Enable = SET;
	Handle->hUART.Init.Rx.LineSeparator = LineSeparator_CRLF;
	Handle->hUART.Init.Rx.QueueSize = 256;
	Handle->hUART.Init.Rx.USART_IRQ_PreemptionPriority = 0;
	Handle->hUART.Init.Rx.USART_IRQ_SubPriority = 0;
	Handle->hUART.Init.Advanced.Remap = Handle->Init.Advanced.UARTRemap;
	PAL_UART_Init(&Handle->hUART);
}

void PAL_EC600_UART_IRQHandler(PalEC600_HandleTypeDef *Handle)
{
	PAL_UART_UART_IRQHandler(&Handle->hUART);
}

ErrorStatus PAL_EC600_Reset(PalEC600_HandleTypeDef *Handle)
{
	char buffer[64];
	ErrorStatus ret = ERROR;
	// 拉高RST
	GPIO_WriteBit(Handle->Init.RST_GPIOPort, Handle->Init.RST_GPIOPin, Bit_SET); 
	// 等待300ms
	PAL_Delay(500);
	// 拉低RST
	GPIO_WriteBit(Handle->Init.RST_GPIOPort, Handle->Init.RST_GPIOPin, Bit_RESET); 
	
	PAL_UART_DiscardInBuffer(&Handle->hUART);
	
	uint64_t start = PAL_GetTick();
	
	while(start + 20000 > PAL_GetTick())
	{
		if(PAL_UART_ReadLine(&Handle->hUART, buffer, 64, 0))
		{
			if(!strcmp(buffer, "RDY\r\n"))
			{
				ret = SUCCESS;
				break;
			}
		}
	}
	return ret;
}

ErrorStatus PAL_EC600_AT(PalEC600_HandleTypeDef *Handle)
{
	ErrorStatus ret = ERROR;
	char buffer[64];
	
	PAL_UART_DiscardInBuffer(&Handle->hUART);
	
	PAL_UART_Printf(&Handle->hUART, "AT\r");
	
	uint64_t start = PAL_GetTick();
	
	while(start + 200 > PAL_GetTick())
	{
		if(PAL_UART_ReadLine(&Handle->hUART, buffer, 64, 0))
		{
			if(!strcmp(buffer, "OK\r\n"))
			{
				ret = SUCCESS;
				break;
			}
		}
	}
	
	return ret;
}

ErrorStatus PAL_EC600_AT_CPIN_Q(PalEC600_HandleTypeDef *Handle)
{
	ErrorStatus ret = ERROR;
	char buffer[64];
	
	PAL_UART_DiscardInBuffer(&Handle->hUART);
	
	PAL_UART_Printf(&Handle->hUART, "AT+CPIN?\r");
	
	uint64_t start = PAL_GetTick();
	
	while(start + 200 > PAL_GetTick())
	{
		if(PAL_UART_ReadLine(&Handle->hUART, buffer, 64, 0))
		{
			if(!strcmp(buffer, "OK\r\n"))
			{
				ret = SUCCESS;
				break;
			}
			else if(strstr(buffer, "+CME ERROR:"))
			{
				ret = ERROR;
				break;
			}
		}
	}
	
	return ret;
}

#define EC600_CONTEXTTYPE_IPv4 1
#define EC600_CONTEXTTYPE_IPv6 2
#define EC600_CONTEXTTYPE_IPv4v6 3

/*
 * @brief 用于配置TCP/IP场景参数
 * @param ContextId   - 场景ID，范围1~15
 *        ContextType - 协议类型，可取以下值之一：
 *                      EC600_CONTEXTTYPE_IPv4 - 只使用IPv4
 *                      EC600_CONTEXTTYPE_IPv6 - 只使用IPv6
 *                      EC600_CONTEXTTYPE_IPv4v6 - 使用IPv4和IPv6的混合模式
 *        APN         - 接入点名称
 *        UserName    - 卡用户名，无密码时为空字符串
 *        UserPsw     - 卡密码，无密码时为空字符串
 */
ErrorStatus PAL_EC600_AT_QICSGP(PalEC600_HandleTypeDef *Handle, uint8_t ContextId, uint8_t ContextType, const char* APN, const char* UserName, const char* Pswd)
{
	ErrorStatus ret = ERROR;
	char buffer[64];
	
	PAL_UART_DiscardInBuffer(&Handle->hUART);
	
	PAL_UART_Printf(&Handle->hUART, "AT+QICSGP=%d,%d,\"%s\",\"%s\",\"%s\"\r", ContextId, ContextType, APN, UserName, Pswd);
	
	uint64_t start = PAL_GetTick();
	
	while(start + 1000 > PAL_GetTick())
	{
		if(PAL_UART_ReadLine(&Handle->hUART, buffer, 64, 0))
		{
			if(!strcmp(buffer, "OK\r\n"))
			{
				ret = SUCCESS;
				break;
			}
			else if(!strcmp(buffer, "ERROR\r\n"))
			{
				ret = ERROR;
				break;
			}
		}
	}
	
	return ret;
}

ErrorStatus PAL_EC600_AT_QIACT(PalEC600_HandleTypeDef *Handle, uint8_t ContextId)
{
	ErrorStatus ret = ERROR;
	char buffer[64];
	
	PAL_UART_DiscardInBuffer(&Handle->hUART);
	
	PAL_UART_Printf(&Handle->hUART, "AT+QIACT=%d\r", ContextId);
	
	uint64_t start = PAL_GetTick();
	
	while(start + 20000 > PAL_GetTick())
	{
		if(PAL_UART_ReadLine(&Handle->hUART, buffer, 64, 0))
		{
			if(!strcmp(buffer, "OK\r\n"))
			{
				ret = SUCCESS;
				break;
			}
			else if(!strcmp(buffer, "ERROR\r\n"))
			{
				ret = ERROR;
				break;
			}
		}
	}
	
	return ret;
}

ErrorStatus PAL_EC600_AT_QICLOSE(PalEC600_HandleTypeDef *Handle, uint8_t ContextId)
{
	ErrorStatus ret = ERROR;
	char buffer[64];
	
	PAL_UART_DiscardInBuffer(&Handle->hUART);
	
	PAL_UART_Printf(&Handle->hUART, "AT+QICLOSE=%d\r", ContextId);
	
	uint64_t start = PAL_GetTick();
	
	while(start + 1000 > PAL_GetTick())
	{
		if(PAL_UART_ReadLine(&Handle->hUART, buffer, 64, 0))
		{
			if(!strcmp(buffer, "OK\r\n"))
			{
				ret = SUCCESS;
				break;
			}
			else if(!strcmp(buffer, "ERROR\r\n"))
			{
				ret = ERROR;
				break;
			}
		}
	}
	
	return ret;
}

ErrorStatus PAL_EC600_PlusPlusPlus(PalEC600_HandleTypeDef *Handle)
{
	ErrorStatus ret = ERROR;
	char buffer[64];
	
	PAL_Delay(1000);
	
	PAL_UART_DiscardInBuffer(&Handle->hUART);
	
	PAL_UART_Printf(&Handle->hUART, "+++");
	
	uint64_t start = PAL_GetTick();
	
	while(start + 1000 > PAL_GetTick())
	{
		if(PAL_UART_ReadLine(&Handle->hUART, buffer, 64, 0))
		{
			if(!strcmp(buffer, "OK\r\n"))
			{
				ret = SUCCESS;
				break;
			}
		}
	}
	
	return ret;
}

/*
* @brief 打开socket服务器
* @param ContextId   - 场景ID，范围1~15
*        ConnectId   - 连接ID，范围0~11
*        ServiceType - Socket服务类型，可选择以下值之一
*                      "TCP" - 客户端建立TCP连接
*                      "UDP" - 客户端建立UDP连接
*                      "TCP LISTENER" - 建立TCP服务监听TCP连接
*                      "UDP LISTENER" - 建立UDP服务
*        IP          - IP地址。
*                      如果SericeType是"TCP"或者"UDP"，则为远程服务器的IP地址， 例如220.180.239.212。
*                      如果SericeType是"TCP LISTENER"或者"UDP SERVICE"，请输入127.0.0.1。
*        RemotePort  - 远程服务器端口。范围：0~65535。仅当<service_type> 是"TCP"或者"UDP"时才有效。
*        LocalPort   - 本地端口。范围：0~65535。
*                      如果<service_type>是"TCP LISTENER"或者"UDPSERVICE"，该参数必须指定。
*                      如果<service_type>是"TCP"或者"UDP"，且<local_port>是0，那么将会自动分配本地端口；
*                      否则本地端口会被指定。
*        AccsessMode - Socket 服务的数据访问模式。
*                      EC600_ACCESSMODE_CACHE - 缓存模式
*                      EC600_ACCESSMODE_DIRECT - 直吐模式
*                      EC600_ACCESSMODE_TRANSPARENT - 透传模式
*/
ErrorStatus PAL_EC600_AT_QIOEPN(PalEC600_HandleTypeDef *Handle, uint8_t ContextId, uint8_t ConnectId, const char *ServiceType, const char* IP, uint16_t RemotePort, uint16_t LocalPort, uint8_t AccsessMode)
{
	ErrorStatus ret = ERROR;
	char buffer[64];
	
	PAL_UART_DiscardInBuffer(&Handle->hUART);
	
	PAL_UART_Printf(&Handle->hUART, "AT+QIOPEN=%d,%d,\"%s\",\"%s\",%d,%d,%d\r", ContextId, ConnectId, ServiceType, IP, RemotePort, LocalPort, AccsessMode);
	
	uint64_t start = PAL_GetTick();
	
	if(AccsessMode == EC600_ACCESSMODE_TRANSPARENT)
	{
		while(start + 20000 > PAL_GetTick())
		{
			if(PAL_UART_ReadLine(&Handle->hUART, buffer, 64, 0))
			{
				if(!strcmp(buffer, "CONNECT\r\n"))
				{
					ret = SUCCESS;
					break;
				}
				else if(!strcmp(buffer, "ERROR\r\n"))
				{
					ret = ERROR;
					break;
				}
			}
		}
	}	
	else
	{
		while(start + 20000 > PAL_GetTick())
		{
			if(PAL_UART_ReadLine(&Handle->hUART, buffer, 64, 0))
			{
				char *p = strstr(buffer, "+QIOPEN: ");
				if(p)
				{
					char *p1, *p2;
					p1 = strstr((char *)buffer, "\r\n+QIOPEN: ") + strlen("\r\n+QIOPEN: ");
					p1 = strstr(p1, ",") + 1;
					p2 = strstr(p1,"\r\n");
					*p2 = '\0';
					int errorCode = atoi(p1);
					if(errorCode != 0)
					{
						ret = ERROR;
					}
					else
					{
						ret = SUCCESS;
					}
					break;
				}
			}
		}
	}
	return ret;
}

/*
* @brief 查询context参数
*/
ErrorStatus PAL_EC600_AT_QIACT_Q(PalEC600_HandleTypeDef *Handle, PalEC600_PdpParametersTypeDef *pPdpParams, uint8_t *Size)
{
	ErrorStatus ret = ERROR;
	char buffer[64];
	
	PAL_UART_DiscardInBuffer(&Handle->hUART);
	
	PAL_UART_Printf(&Handle->hUART, "AT+QIACT?\r");
	
	PAL_Delay(10);
	
	uint64_t start = PAL_GetTick();
	
	while(start + 1000 > PAL_GetTick())
	{
		if(PAL_UART_ReadLine(&Handle->hUART, buffer, 64, 0))
		{
			if(!strcmp(buffer, "OK\r\n")) // 接收到了\r\nOK\r\n，表示接收结束
			{
				ret = SUCCESS;
				break;
			}
			else 
			{
				char *p1 = strstr(buffer, "+QIACT: ");
				char *p2;
				if(p1 == 0) continue; // 此行不含有效信息
				
				// Context ID
				p1 += strlen("+QIACT: ");
				p2 = strstr(p1, ",");
				*p2 = '\0';
				pPdpParams[*Size].Id = atoi(p1);
				p1 = p2+1;
				
				// Context State
				p2 = strstr(p1, ",");
				*p2 = '\0';
				pPdpParams[*Size].Active = atoi(p1) == 1 ? SET : RESET;
				p1 = p2+1;
				
				// Context Type
				p2 = strstr(p1+1, ",");
				*p2 = '\0';
				pPdpParams[*Size].EC600ContextType = atoi(p1);
				p1 = p2+1;
				
				// Ip address
				pPdpParams[*Size].HasIPv4 = RESET;
				pPdpParams[*Size].HasIPv6 = RESET;
				
				p1++;
				p2 = strstr(p1, "\"");
				*p2 = '\0';

				if(strstr(p1, ":") != NULL) // IPv6
				{
					pPdpParams[*Size].HasIPv6 = SET;
					PAL_StrToIPv6(p1, &pPdpParams[*Size].IPv6);
				}
				else // IPv4
				{
					pPdpParams[*Size].HasIPv4 = SET;
					PAL_StrToIPv4(p1, &pPdpParams[*Size].IPv4);
				}
				p1 = p2+1;

				if(*p1 == ',') // 双IP地址
				{
					p1 += 2;
					p2 = strstr(p1, "\"\r\n");
					*p2 = '\0';

					if(strstr(p1, ":") != NULL) // IPv6
					{
						pPdpParams[*Size].HasIPv6 = SET;
						PAL_StrToIPv6(p1, &pPdpParams[*Size].IPv6);
					}
					else // IPv4
					{
						pPdpParams[*Size].HasIPv4 = SET;
						PAL_StrToIPv4(p1, &pPdpParams[*Size].IPv4);
					}

					p1 = p2+1;
				}
				(*Size)++;
			}
		}
	}
	return ret;
}

/*
* @brief 发送数据
* @param ConnectionId - 连接ID，范围：0~11
*        pData        - 要发送的数据
*        Size         - 要发送的数据的长度
*/
ErrorStatus PAL_EC600_AT_QISEND(PalEC600_HandleTypeDef *Handle, uint8_t ConnectionId, uint8_t *pData, uint32_t Size)
{
	ErrorStatus ret = ERROR;
	char buffer[64];
	
	PAL_UART_DiscardInBuffer(&Handle->hUART);
	
	PAL_UART_Printf(&Handle->hUART, "AT+QISEND=%d,%d\r", ConnectionId, (int)Size);
	
	uint64_t start = PAL_GetTick();
	
	// 等待/r/n>[空格]
	char tmp[] = {0, 0, 0, 0, 0};
	FlagStatus canWritePlayload = RESET;
	while(start + 200 > PAL_GetTick())
	{
		int16_t byteRcvd = PAL_UART_RecieveByte(&Handle->hUART, 0);
		if(byteRcvd > 0)
		{
			tmp[0] = tmp[1];
			tmp[1] = tmp[2];
			tmp[2] = tmp[3];
			tmp[4] = (uint8_t)byteRcvd;
		}
		if(strcmp(tmp, "/r/n> ") == 0)
		{
			canWritePlayload = SET;
			break;
		}
	}
	
	if(canWritePlayload == SET)
	{
		start = PAL_GetTick();
		
		PAL_UART_DiscardInBuffer(&Handle->hUART);
		
		PAL_UART_SendBytes(&Handle->hUART, pData, Size);
		
		
		while(1000 > PAL_GetTick() - start)
		{
			if(PAL_UART_ReadLine(&Handle->hUART, buffer, 64, 0))
			{
				if(strcmp(buffer, "SEND OK\r\n") == 0)
				{
					ret = SUCCESS;
					break;
				}
				else if(strcmp(buffer, "SEND FAIL\r\n") == 0 || strcmp(buffer, "ERROR\r\n") == 0)
				{
					ret = ERROR;
					break;
				}
			}
		}
	}
	
	return ret;
}

ErrorStatus PAL_EC600_AT_QIRD_GetLength(PalEC600_HandleTypeDef *Handle, uint8_t ConnectionId, uint32_t *pTotalRcvd, uint32_t *pHaveRead, uint32_t *pUnread)
{
	ErrorStatus ret = ERROR;
	char buffer[64];
	
	PAL_UART_DiscardInBuffer(&Handle->hUART);
	
	PAL_UART_Printf(&Handle->hUART, "AT+QIRD=%d,0\r", ConnectionId);
	
	uint64_t start = PAL_GetTick();
	
	while(start + 200 > PAL_GetTick())
	{
		if(PAL_UART_ReadLine(&Handle->hUART, buffer, 64, 0))
		{
			if(strcmp(buffer, "ERROR\r\n") == 0)
			{
				ret = ERROR;
				break;
			}
			else if(strstr(buffer, "+QIRD: ") == buffer)
			{
				char *p1, *p2;
				p1 = buffer + strlen("+QIRD: ");
				p2 = strstr(p1, ",");
				*p2 = '\0';
				if(pTotalRcvd != NULL)
				{
					*pTotalRcvd = atoi(p1);
				}
				*p2 = ',';
				
				p1 = p2 + 1;
				p2 = strstr(p1, ",");
				*p2 = '\0';
				if(pHaveRead != NULL)
				{
					*pHaveRead = atoi(p1);
				}
				*p2 = ',';

				p1 = p2 + 1;
				p2 = strstr(p1, "\r\n");
				*p2 = '\0';
				if(pUnread != NULL)
				{
					*pUnread = atoi(p1);
				}
				*p2 = '\r';
				
				ret = SUCCESS;
				break;
			}
		}
	}
	return ret;
}

ErrorStatus PAL_EC600_AT_QICFG_TransWtTm(PalEC600_HandleTypeDef *Handle, uint8_t n100mS)
{
	ErrorStatus ret = ERROR;
	char buffer[64];
	
	PAL_UART_DiscardInBuffer(&Handle->hUART);
	
	PAL_UART_Printf(&Handle->hUART, "AT+QICFG=\"transwaittm\",%d\r", n100mS);
	
	uint64_t start = PAL_GetTick();
	
	while(start + 200 > PAL_GetTick())
	{
		if(PAL_UART_ReadLine(&Handle->hUART, buffer, 64, 0))
		{
			if(!strcmp(buffer, "OK\r\n"))
			{
				ret = SUCCESS;
				break;
			}
			else if(!strcmp(buffer, "ERROR\r\n"))
			{
				ret = ERROR;
				break;
			}
		}
	}
	
	return ret;
}

ErrorStatus PAL_EC600_AT_QICFG_TransPktSize(PalEC600_HandleTypeDef *Handle, uint16_t Size)
{
	ErrorStatus ret = ERROR;
	char buffer[64];
	
	PAL_UART_DiscardInBuffer(&Handle->hUART);
	
	PAL_UART_Printf(&Handle->hUART, "AT+QICFG=\"transpktsize\",%d\r", Size);
	
	uint64_t start = PAL_GetTick();
	
	while(start + 200 > PAL_GetTick())
	{
		if(PAL_UART_ReadLine(&Handle->hUART, buffer, 64, 0))
		{
			if(!strcmp(buffer, "OK\r\n"))
			{
				ret = SUCCESS;
				break;
			}
			else if(!strcmp(buffer, "ERROR\r\n"))
			{
				ret = ERROR;
				break;
			}
		}
	}
	
	return ret;
}

ErrorStatus PAL_EC600_AT_IPR(PalEC600_HandleTypeDef *Handle, uint32_t Baudrate)
{
	ErrorStatus ret = ERROR;
	char buffer[64];
	
	PAL_UART_DiscardInBuffer(&Handle->hUART);
	
	PAL_UART_Printf(&Handle->hUART, "AT+IPR=%d\r", (int)Baudrate);
	
	uint64_t start = PAL_GetTick();
	
	while(start + 200 > PAL_GetTick())
	{
		if(PAL_UART_ReadLine(&Handle->hUART, buffer, 64, 0))
		{
			if(!strcmp(buffer, "OK\r\n"))
			{
				ret = SUCCESS;
				break;
			}
			else if(!strcmp(buffer, "ERROR\r\n"))
			{
				ret = ERROR;
				break;
			}
		}
	}
	
	if(ret == SUCCESS)
	{
		PAL_UART_ChangeBaudrate(&Handle->hUART, Baudrate);
	}
	
	return ret;
}

void PAL_EC600_SendDataTransparent_DMA(PalEC600_HandleTypeDef *Handle, const uint8_t *pData, uint16_t Size)
{
	PAL_UART_SendBytes_DMA(&Handle->hUART, pData, Size);
}

void PAL_EC600_FlushDMA(PalEC600_HandleTypeDef *Handle)
{
	PAL_UART_FlushDMA(&Handle->hUART);
}

/* 
* @brief 将IPv6转化为字符串
*/
ErrorStatus PAL_StrToIPv6(const char *str, PalIPv6_TypeDef *pIPv6)
{
	uint16_t *p = (uint16_t *)(pIPv6->Bytes);
	int ret = sscanf(str, "%hx:%hx:%hx:%hx:%hx:%hx:%hx:%hx", &p[0], &p[1], &p[2], &p[3], &p[4], &p[5], &p[6], &p[7]);
	return ret == 8 ? SUCCESS : ERROR;
}

/* 
* @brief 将IPv4转化为字符串
*/
void PAL_IPv4ToStr(PalIPv4_TypeDef IPv4, char *pBuf)
{
	sprintf(pBuf, "%d.%d.%d.%d", IPv4.Bytes[0], IPv4.Bytes[1], IPv4.Bytes[2], IPv4.Bytes[3]);
}

/* 
* @brief 通过IPv4字符串解析出IPv4数据
*/
ErrorStatus PAL_StrToIPv4(const char *str, PalIPv4_TypeDef *pIPv4)
{
	uint32_t tmp[4];
	
	int ret = sscanf(str, "%u.%u.%u.%u", tmp, tmp+1, tmp+2, tmp+3);
	
	pIPv4->Bytes[0] = tmp[0];
	pIPv4->Bytes[1] = tmp[1];
	pIPv4->Bytes[2] = tmp[2];
	pIPv4->Bytes[3] = tmp[3];
	
	return ret == 4 ? SUCCESS : ERROR;
}

/* 
* @brief 通过IPv6字符串解析出IPv6数据
*/
void PAL_IPv6ToStr(PalIPv6_TypeDef IPv6, char *pBuf)
{
	uint16_t *p = (uint16_t *)(IPv6.Bytes);
	sprintf(pBuf, "%x:%x:%x:%x:%x:%x:%x:%x", p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]);
}
