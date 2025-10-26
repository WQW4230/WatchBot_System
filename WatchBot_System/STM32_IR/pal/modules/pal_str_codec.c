#include "stm32f10x.h"
#include "stm32f10x_pal.h"
#include "pal_str_codec.h"
#include <string.h>
#include <stdlib.h>
#include <errno.h>

static const char *PAL_StrCodec_ReadNextArg(PalStrCodec_HandleTypeDef *Handle, const char *PreArg);

//
// @简介：对该字符串命令编解码器进行初始化
// @参数：Handle - 编解码器的句柄
// 
void PAL_StrCodec_Init(PalStrCodec_HandleTypeDef *Handle)
{
	memset(Handle->rx_msg_buf, 0, sizeof(Handle->rx_msg_buf) / sizeof(char)); // 缓冲区清零
}

// 
// @简介：接收一条新的消息
// @参数：Handle - 编解码器的句柄
// @返回值：是否接收成功。SUCCESS - 成功，ERROR - 失败
// 
ErrorStatus PAL_StrCodec_Receive(PalStrCodec_HandleTypeDef *Handle)
{
	size_t cmdStrLen; // 命令字符串的原始长度
	
	if(Handle->Init.hUSART->Init.Advanced.LineSeparator != LineSeparator_CR
		&&Handle->Init.hUSART->Init.Advanced.LineSeparator != LineSeparator_LF
	  &&Handle->Init.hUSART->Init.Advanced.LineSeparator != LineSeparator_CRLF)
	{
		return ERROR; // 未设置有效的换行符
	}
	
	if(PAL_USART_ReadLine(Handle->Init.hUSART, Handle->rx_msg_buf, sizeof(Handle->rx_msg_buf) / sizeof(char), 0) == 0)
	{
		return ERROR; // 未从串口读到一个完整的行
	}
	
	cmdStrLen = strlen(Handle->rx_msg_buf);
	
	uint16_t i,j;
	
	// 删除行首的空格
	i = 0;
	while(Handle->rx_msg_buf[i] == ' ')
	{
		Handle->rx_msg_buf[i] = '\0';
		i++;
	}
	
	// 删除掉行尾的换行符
	switch(Handle->Init.hUSART->Init.Advanced.LineSeparator)
	{
		case LineSeparator_CR:
		case LineSeparator_LF:
			Handle->rx_msg_buf[cmdStrLen-1] = '\0';
		  break;
		case LineSeparator_CRLF:
			Handle->rx_msg_buf[cmdStrLen-2] = '\0'; // \r
		  Handle->rx_msg_buf[cmdStrLen-1] = '\0'; // \n
		  break;
		default: break;
	}
	
	FlagStatus doubleQuoted = RESET; // 是否在双引号内部
	
	// 对命令行进行分割
	for(;i<cmdStrLen;i++)
	{
		if(doubleQuoted == SET) // 处于双引号内部
		{
			// 再次遇到 "
			if(Handle->rx_msg_buf[i] == '\"')
			{
				if(Handle->rx_msg_buf[i+1] == ' ' || Handle->rx_msg_buf[i+1] == '\0')
				{
					Handle->rx_msg_buf[i] = '\0'; // "
					i++;
					// 消除连续的空格
					while(Handle->rx_msg_buf[i] == ' ')
					{
						Handle->rx_msg_buf[i] = '\0'; // 空格
						i++;
					}
					doubleQuoted = RESET;
				}
				else
				{
					return ERROR; // 双引号后面必须跟空格或者\0，否则视为格式错误
				}
			}
		}
		else // 不在双引号的内部
		{
			if(Handle->rx_msg_buf[i] == ' ') // 遇到空格
			{
				while(Handle->rx_msg_buf[i+1] == ' ') // 删除连续的空格
				{
					Handle->rx_msg_buf[i] = '\0';
					i++;
				}
				
				Handle->rx_msg_buf[i] = '\0'; // 标记此空格
				i++;
				
				if(Handle->rx_msg_buf[i] == '\"') // 遇到双引号
				{
					Handle->rx_msg_buf[i] = '\0';
					doubleQuoted = SET;
				}
			}
			else if(Handle->rx_msg_buf[i] == '\"') // 双引号，但前边没空格
			{
				return ERROR; // 视为格式错误
			}
		}
	}
	
	// 对消息重新拷贝
	i=0; // 源地址
	j=0; // 目标地址
	
	while(Handle->rx_msg_buf[i] == '\0') // 忽略开头的'\0'
	{
		i++;
	}
	
	for(;i<cmdStrLen;i++)
	{
		if(i+1 < cmdStrLen && Handle->rx_msg_buf[i] == '\0' && Handle->rx_msg_buf[i+1] == '\0')
		{
		}
		else
		{
			Handle->rx_msg_buf[j++] = Handle->rx_msg_buf[i];
		}
	}
	
	// 结尾全部赋值\0
	for(;j<PAL_STR_MESSAGE_MAX_LEN;j++)
	{
		Handle->rx_msg_buf[j] = '\0';
	}
	
	// 如果命令行为空行，则返回ERROR，表示没有接收到命令
	if(strlen(Handle->rx_msg_buf) == 0)
	{
		return ERROR;
	}
	
	return SUCCESS;
}

// 
// @简介：获取消息的名称
// @参数：Handle - 编解码器的句柄
// @返回值：消息名称
// 
const char *PAL_StrCodec_GetName(PalStrCodec_HandleTypeDef *Handle)
{
	return Handle->rx_msg_buf;
}

//
// @简介：获取参数的数量
// @参数：Handle - 编解码器的句柄
// @返回值：参数的数量
//
uint16_t PAL_StrCodec_GetNumberOfArgs(PalStrCodec_HandleTypeDef *Handle)
{
	uint16_t ret = 0;
	
	const char *ptr = PAL_StrCodec_GetName(Handle);
	
	while((ptr = PAL_StrCodec_ReadNextArg(Handle, ptr)) != 0)
	{
		ret++;
	}
	
	return ret;
}

//
// @简介：读取第n个参数
// @参数：Handle - 编解码器的句柄
// @参数：Index - 参数的序号，以0开始
// @返回值：返回参数字符串的指针，如果Index超出范围则返回0
//
const char *PAL_StrCodec_ReadArgStr(PalStrCodec_HandleTypeDef *Handle, uint16_t Index)
{
	uint16_t i;
	
	const char *ptr = PAL_StrCodec_GetName(Handle);
	
	for(i=0; i <= Index; i++)
	{
		ptr = PAL_StrCodec_ReadNextArg(Handle, ptr);
		
		if(ptr == 0) break;
	}
	
	return ptr;
}

// 
// @简介：将第Index个参数当作整数读取出来
// @参数：Handle - 编解码器的句柄
// @参数：Index - 参数的序号，以0开始
// @参数：pValOut - 输出参数，用于输出转化后的整数
// @返回值：如果转化成功则返回SUCCESS，否则返回ERROR
// @注意：参数以0x开头按十六进制解析，参数以0开头按八进制解析，否则按十进制解析
// 
ErrorStatus PAL_StrCodec_ReadArgInt(PalStrCodec_HandleTypeDef *Handle, uint16_t Index, int32_t *pValOut)
{
	const char *argStr;
	char *endptr;
	int base = 10; // 进制数，默认以十进制解析该整数
	
	argStr = PAL_StrCodec_ReadArgStr(Handle, Index);
	
	if(argStr == 0) 
	{
		return ERROR; // 索引超出范围
	}
	
	errno = 0;
	
	if(argStr[0] == '0' && argStr[1] == 'x') // 以0x开头的字符串，按十六进制解析
	{
		argStr = argStr + 2;
		base = 16;
	}
	else if(argStr[0] == '0') // 以0开头的字符串，按八进制解析
	{
		argStr = argStr + 1;
		base = 8;
	}
	
	*pValOut = strtol(argStr, &endptr, base);
	
	if(endptr == argStr) 
	{
		return ERROR; // 参数中不含有效的数字
	}
	
	if(*endptr != '\0') 
	{
		return ERROR; // 格式错误，比如输入了一个小数
	}
	
	if(errno == ERANGE)
	{
		return ERROR; // 整数的值超出了最大范围
	}
	
	return SUCCESS;
}

// 
// @简介：将第Index个参数当作单浮点数读取出来
// @参数：Handle - 编解码器的句柄
// @参数：Index - 参数的序号，以0开始
// @参数：pValOut - 输出参数，用于输出转化后的单浮点数
// @返回值：如果转化成功则返回SUCCESS，否则返回ERROR
// 
ErrorStatus PAL_StrCodec_ReadArgFloat(PalStrCodec_HandleTypeDef *Handle, uint16_t Index, float *pValOut)
{
	const char *argStr;
	char *endptr;
	
	argStr = PAL_StrCodec_ReadArgStr(Handle, Index);
	
	if(argStr == 0) return ERROR; // 索引超出范围
	
	errno = 0;
	
	*pValOut = strtof(argStr, &endptr);
	
	if(endptr == argStr) // 参数中不含有效的数字
	{
		return ERROR;
	}
	
	if(*endptr != '\0')
	{
		return ERROR; // 格式错误
	}
	
	if(errno == ERANGE)
	{
		return ERROR; // 整数的值超出了最大范围
	}
	
	return SUCCESS;
}

static const char *PAL_StrCodec_ReadNextArg(PalStrCodec_HandleTypeDef *Handle, const char *PreArg)
{
	const char *ptr;
	
	if(PreArg < Handle->rx_msg_buf || PreArg > &Handle->rx_msg_buf[PAL_STR_MESSAGE_MAX_LEN])
	{
		return NULL; // 不在范围内
	}
	
	if(strlen(PreArg) == 0)
	{
		return NULL; // PreArg无效
	}
	
	ptr = PreArg + strlen(PreArg) + 1;
	
	if(ptr < Handle->rx_msg_buf || ptr > &Handle->rx_msg_buf[PAL_STR_MESSAGE_MAX_LEN])
	{
		return NULL; // 不在范围内
	}
	
	if(strlen(ptr) == 0)
	{
		return NULL; // ptr无效
	}
	
	return ptr;
}
