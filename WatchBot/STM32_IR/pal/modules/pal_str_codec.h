#ifndef __PAL_STR_MESSAGE_CODEC_H__
#define __PAL_STR_MESSAGE_CODEC_H__

#include "stm32f10x.h"
#include "stm32f10x_pal_usart.h"

#define PAL_STR_MESSAGE_MAX_LEN 64

typedef struct
{
	PalUSART_HandleTypeDef *hUSART; // 串口句柄
}
PalStrCodec_InitTypeDef;

typedef struct
{
	PalStrCodec_InitTypeDef Init;
	char rx_msg_buf[PAL_STR_MESSAGE_MAX_LEN];
}PalStrCodec_HandleTypeDef;

       void PAL_StrCodec_Init(PalStrCodec_HandleTypeDef *Handle);
ErrorStatus PAL_StrCodec_Receive(PalStrCodec_HandleTypeDef *Handle);
   uint16_t PAL_StrCodec_GetNumberOfArgs(PalStrCodec_HandleTypeDef *Handle);
const char *PAL_StrCodec_GetName(PalStrCodec_HandleTypeDef *Handle);
const char *PAL_StrCodec_ReadArgStr(PalStrCodec_HandleTypeDef *Handle, uint16_t Index);
ErrorStatus PAL_StrCodec_ReadArgInt(PalStrCodec_HandleTypeDef *Handle, uint16_t Index, int32_t *pValOut);
ErrorStatus PAL_StrCodec_ReadArgFloat(PalStrCodec_HandleTypeDef *Handle, uint16_t Index, float *pValOut);

#endif
