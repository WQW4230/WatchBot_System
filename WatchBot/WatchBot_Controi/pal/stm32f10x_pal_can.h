#ifndef _STM32F10x_PAL_CAN_H_
#define _STM32F10x_PAL_CAN_H_

#include "stm32f10x.h"
#include "stm32f10x_can.h"
#include "pal_object_queue.h"

#ifdef STM32F10X_CL
#define MAX_FILTER_BANKS 28
#else
#define MAX_FILTER_BANKS 14
#endif

typedef struct
{
	uint16_t StdID_10_0; // 11bits
	uint32_t ExtID_17_0; // 18bits
	FlagStatus ExtendedId;
	FlagStatus RemoteRequest;
} PalCan_32BitFilterIdTypeDef;

typedef struct
{
	uint16_t StdID_10_0; // 11bits
	FlagStatus ExtendedId;
	FlagStatus RemoteRequest;
	uint32_t ExtID_17_15;
} PalCan_16BitFilterIdTypeDef;

typedef struct
{
	PalCan_32BitFilterIdTypeDef ID;
	PalCan_32BitFilterIdTypeDef Mask;
} PalCan_Filter_32BitMaskTypeDef;

typedef struct
{
	PalCan_32BitFilterIdTypeDef ID1;
	PalCan_32BitFilterIdTypeDef ID2;
} PalCan_Filter_32BitListTypeDef;

typedef struct
{
	PalCan_16BitFilterIdTypeDef ID1;
	PalCan_16BitFilterIdTypeDef Mask1;
	PalCan_16BitFilterIdTypeDef ID2;
	PalCan_16BitFilterIdTypeDef Mask2;
} PalCan_Filter_16BitMaskTypeDef;

typedef struct
{
	PalCan_16BitFilterIdTypeDef ID1;
	PalCan_16BitFilterIdTypeDef ID2;
	PalCan_16BitFilterIdTypeDef ID3;
	PalCan_16BitFilterIdTypeDef ID4;
} PalCan_Filter_16BitListTypeDef;

typedef struct
{
	FunctionalState Cmd;
	uint16_t CAN_FilterMode;
	uint16_t CAN_FilterScale;
	PalCan_Filter_32BitMaskTypeDef _32BitMask;
	PalCan_Filter_32BitListTypeDef _32BitList;
	PalCan_Filter_16BitMaskTypeDef _16BitMask;
	PalCan_Filter_16BitListTypeDef _16BitList;
} PalCan_FilterBankInitTypeDef;

typedef struct
{
	uint32_t Remap;
	uint16_t TxQueueSize;
	uint16_t RxQueueSize;
} PalCan_AdvancedInitTypeDef;

typedef struct
{
	uint16_t Prescaler;
	uint8_t  CAN_SJW;
	uint8_t  CAN_BS1;
	uint8_t  CAN_BS2;
} PalCan_BitTimingInitTypeDef;

typedef struct
{
	PalCan_BitTimingInitTypeDef BitTiming;
	PalCan_FilterBankInitTypeDef Filters[MAX_FILTER_BANKS];
	PalCan_AdvancedInitTypeDef Advanced;
	uint8_t CAN1_Rx0_IRQ_PreemptionPriority;
	uint8_t CAN1_Rx0_IRQ_SubPriority;
} PalCan_InitTypeDef;

typedef struct
{
	PalCan_InitTypeDef Init;
	PalObjectQueue_HandleTypeDef hTxQueue;
	PalObjectQueue_HandleTypeDef hRxQueue;
} PalCan_HandleTypeDef;

ErrorStatus PAL_Can_Init(PalCan_HandleTypeDef *Handle);
void        PAL_Can_Rx0_IRQHandler(PalCan_HandleTypeDef *Handle);
// 轮询方式发送
uint8_t     PAL_Can_GetTxFreeLevel(PalCan_HandleTypeDef *Handle);
uint8_t     PAL_Can_Transmit(PalCan_HandleTypeDef *Handle, CanTxMsg *Msg);
void        PAL_Can_CancelTransmit(PalCan_HandleTypeDef *Handle, uint8_t Mailbox);
// 缓冲方式发送
ErrorStatus PAL_Can_SendToOutputBuffer(PalCan_HandleTypeDef *Handle, CanTxMsg *Msg);
uint16_t    PAL_Can_FlushOutputBuffer(PalCan_HandleTypeDef *Handle, uint32_t Timeout);
void        PAL_Can_DiscardOutputBuffer(PalCan_HandleTypeDef *Handle);
// 缓冲方式接收
uint16_t    PAL_Can_NumberOfMessagesInInputBuffer(PalCan_HandleTypeDef *Handle);
ErrorStatus PAL_Can_ReceiveFromInputBuffer(PalCan_HandleTypeDef *Handle, CanRxMsg *pRxMessage);
void        PAL_Can_DiscardInputBbuffer(PalCan_HandleTypeDef *Handle);

#endif
