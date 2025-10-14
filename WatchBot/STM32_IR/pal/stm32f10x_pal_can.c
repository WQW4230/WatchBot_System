#include "stm32f10x_pal.h"
#include "stm32f10x_pal_can.h"

#define DEFAULT_TX_QUEUE_SIZE 8
#define DEFAULT_RX_QUEUE_SIZE 8

static uint32_t FilterIdTo32BitRegisterValue(PalCan_32BitFilterIdTypeDef *FilterId);
static uint16_t FilterIdTo16BitRegisterValue(PalCan_16BitFilterIdTypeDef *FilterId);
static uint8_t PAL_Can_GetTxFreeLevel(PalCan_HandleTypeDef *Handle);

ErrorStatus PAL_Can_Init(PalCan_HandleTypeDef *Handle)
{
	uint32_t i;
	GPIO_InitTypeDef GPIOInitStruct;
	CAN_FilterInitTypeDef CanFilterInitStruct;
	
	// 初始化缓冲队列
	if(Handle->Init.Advanced.TxQueueSize == 0)
	{
		Handle->Init.Advanced.TxQueueSize = DEFAULT_TX_QUEUE_SIZE; 
	}
	
	PAL_ObjectQueue_Init(&Handle->hTxQueue, sizeof(CanTxMsg), Handle->Init.Advanced.TxQueueSize);
	
	if(Handle->Init.Advanced.RxQueueSize == 0)
	{
		Handle->Init.Advanced.RxQueueSize = DEFAULT_RX_QUEUE_SIZE;
	}
	
	PAL_ObjectQueue_Init(&Handle->hRxQueue, sizeof(CanRxMsg), Handle->Init.Advanced.RxQueueSize);
	
	// 配置IO引脚
	if(Handle->Init.Advanced.Remap == 0) // 无重映射 CAN_Rx,CAN_Tx -> PA11, PA12
	{
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
		
		// CAN_Rx
		GPIOInitStruct.GPIO_Pin = GPIO_Pin_11;
		GPIOInitStruct.GPIO_Mode = GPIO_Mode_IPU;
		GPIO_Init(GPIOA, &GPIOInitStruct);
		
		// CAN_Tx
		GPIOInitStruct.GPIO_Pin = GPIO_Pin_12;
		GPIOInitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
		GPIOInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOA, &GPIOInitStruct);
	}
	else if(Handle->Init.Advanced.Remap == 2) // 重映射 CAN_Rx,CAN_Tx -> PB8, PB9
	{
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
		
		// CAN_Rx
		GPIOInitStruct.GPIO_Pin = GPIO_Pin_8;
		GPIOInitStruct.GPIO_Mode = GPIO_Mode_IPU;
		GPIO_Init(GPIOB, &GPIOInitStruct);
		
		// CAN_Tx
		GPIOInitStruct.GPIO_Pin = GPIO_Pin_9;
		GPIOInitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
		GPIOInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOB, &GPIOInitStruct);
	}
	else if(Handle->Init.Advanced.Remap == 3) // 重映射 CAN_Rx,CAN_Tx -> PD0, PD1
	{
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
		
		// CAN_Rx
		GPIOInitStruct.GPIO_Pin = GPIO_Pin_0;
		GPIOInitStruct.GPIO_Mode = GPIO_Mode_IPU;
		GPIO_Init(GPIOD, &GPIOInitStruct);
		
		// CAN_Tx
		GPIOInitStruct.GPIO_Pin = GPIO_Pin_1;
		GPIOInitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
		GPIOInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOD, &GPIOInitStruct);
	}
	
	// 为bxCAN开启时钟
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_CAN1, ENABLE);
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_CAN1, DISABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);
	
	// 配置滤波器
	uint32_t reg_val;
	
	CAN_SlaveStartBank(27); // 滤波器全部分配给CAN1
	
	for(i=0;i<MAX_FILTER_BANKS;i++)
	{
		if(Handle->Init.Filters[i].Cmd == ENABLE)
		{
			CanFilterInitStruct.CAN_FilterNumber = i;
			CanFilterInitStruct.CAN_FilterActivation = ENABLE;
			CanFilterInitStruct.CAN_FilterFIFOAssignment = CAN_FilterFIFO0;
			CanFilterInitStruct.CAN_FilterMode = Handle->Init.Filters[i].CAN_FilterMode;
			CanFilterInitStruct.CAN_FilterScale = Handle->Init.Filters[i].CAN_FilterScale;
			
			if(Handle->Init.Filters[i].CAN_FilterMode == CAN_FilterMode_IdList
				&& Handle->Init.Filters[i].CAN_FilterScale == CAN_FilterScale_32bit) // 32位地址列表
			{
				reg_val = FilterIdTo32BitRegisterValue(&Handle->Init.Filters[i]._32BitList.ID1);
				
				CanFilterInitStruct.CAN_FilterIdLow  = (uint16_t)(reg_val & 0xffff); // 低16位
				CanFilterInitStruct.CAN_FilterIdHigh = (uint16_t)((reg_val >> 16) & 0xffff); // 高16位
				
				reg_val = FilterIdTo32BitRegisterValue(&Handle->Init.Filters[i]._32BitList.ID2);
				CanFilterInitStruct.CAN_FilterMaskIdLow  = (uint16_t)(reg_val & 0xffff); // 低16位
				CanFilterInitStruct.CAN_FilterMaskIdHigh = (uint16_t)((reg_val >> 16) & 0xffff); // 高16位
			}
			else if(Handle->Init.Filters[i].CAN_FilterMode == CAN_FilterMode_IdMask
				&& Handle->Init.Filters[i].CAN_FilterScale == CAN_FilterScale_32bit) // 32位地址屏蔽
			{
				reg_val = FilterIdTo32BitRegisterValue(&Handle->Init.Filters[i]._32BitMask.ID);
				
				CanFilterInitStruct.CAN_FilterIdLow  = (uint16_t)(reg_val & 0xffff); // 低16位
				CanFilterInitStruct.CAN_FilterIdHigh = (uint16_t)((reg_val >> 16) & 0xffff); // 高16位
				
				reg_val = FilterIdTo32BitRegisterValue(&Handle->Init.Filters[i]._32BitMask.Mask);
				CanFilterInitStruct.CAN_FilterMaskIdLow  = (uint16_t)(reg_val & 0xffff); // 低16位
				CanFilterInitStruct.CAN_FilterMaskIdHigh = (uint16_t)((reg_val >> 16) & 0xffff); // 高16位
			}
			else if(Handle->Init.Filters[i].CAN_FilterMode == CAN_FilterMode_IdList
				&& Handle->Init.Filters[i].CAN_FilterScale == CAN_FilterScale_16bit) // 16位地址列表
			{
				CanFilterInitStruct.CAN_FilterIdLow  = FilterIdTo16BitRegisterValue(&Handle->Init.Filters[i]._16BitList.ID1);
				CanFilterInitStruct.CAN_FilterIdHigh  = FilterIdTo16BitRegisterValue(&Handle->Init.Filters[i]._16BitList.ID2);
				CanFilterInitStruct.CAN_FilterIdLow  = FilterIdTo16BitRegisterValue(&Handle->Init.Filters[i]._16BitList.ID3);
				CanFilterInitStruct.CAN_FilterMaskIdHigh  = FilterIdTo16BitRegisterValue(&Handle->Init.Filters[i]._16BitList.ID4);
			}
			else // 16位地址屏蔽
			{
				CanFilterInitStruct.CAN_FilterIdLow  = FilterIdTo16BitRegisterValue(&Handle->Init.Filters[i]._16BitMask.ID1);
				CanFilterInitStruct.CAN_FilterIdHigh  = FilterIdTo16BitRegisterValue(&Handle->Init.Filters[i]._16BitMask.Mask1);
				CanFilterInitStruct.CAN_FilterIdLow  = FilterIdTo16BitRegisterValue(&Handle->Init.Filters[i]._16BitMask.ID2);
				CanFilterInitStruct.CAN_FilterMaskIdHigh  = FilterIdTo16BitRegisterValue(&Handle->Init.Filters[i]._16BitMask.Mask2);
			}
		}
	}
	
	CAN_FilterInit(&CanFilterInitStruct);
	
	// 初始化
	CAN_InitTypeDef CANInitStruct;
	CANInitStruct.CAN_Prescaler = Handle->Init.BitTiming.Prescaler;
	CANInitStruct.CAN_SJW = Handle->Init.BitTiming.CAN_SJW;
	CANInitStruct.CAN_BS1 = Handle->Init.BitTiming.CAN_BS1;
	CANInitStruct.CAN_BS2 = Handle->Init.BitTiming.CAN_BS2;
	CANInitStruct.CAN_TTCM = DISABLE;
	CANInitStruct.CAN_ABOM = ENABLE;
	CANInitStruct.CAN_AWUM = DISABLE;
	CANInitStruct.CAN_NART = DISABLE;
	CANInitStruct.CAN_RFLM = DISABLE;
	CANInitStruct.CAN_TXFP = ENABLE;
	if(CAN_Init(CAN1, &CANInitStruct) != CAN_InitStatus_Success)
	{
		return ERROR;
	}
	
	// 开启接收中断
	CAN_ITConfig(CAN1, CAN_IT_FMP0, ENABLE); // 接受到can消息后触发中断
	
	NVIC_InitTypeDef NVICInitStruct;
	NVICInitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVICInitStruct.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
	NVICInitStruct.NVIC_IRQChannelPreemptionPriority = Handle->Init.CAN1_Rx0_IRQ_PreemptionPriority;
	NVICInitStruct.NVIC_IRQChannelSubPriority = Handle->Init.CAN1_Rx0_IRQ_SubPriority;
	NVIC_Init(&NVICInitStruct);
	
	return SUCCESS;
}

void PAL_Can_Rx0_IRQHandler(PalCan_HandleTypeDef *Handle)
{
	CanRxMsg rxmsg;
	if(CAN_GetITStatus(CAN1, CAN_IT_FMP0) == SET) // 收到CAN消息
	{
		CAN_ClearITPendingBit(CAN1, CAN_IT_FMP0);
		// 将CAN消息保存到队列中
		while(CAN_MessagePending(CAN1, CAN_FIFO0) > 0)
		{
			CAN_Receive(CAN1, CAN_FIFO0, &rxmsg);
			
			PAL_ObjectQueue_EnqueueEx(&Handle->hRxQueue, &rxmsg);
		}
	}
}

void PAL_Can_DiscardOutputBuffer(PalCan_HandleTypeDef *Handle)
{
	PAL_ObjectQueue_Clear(&Handle->hTxQueue);
}

uint16_t PAL_Can_NumberOfMessagesInInputBuffer(PalCan_HandleTypeDef *Handle)
{
	return PAL_ObjectQueue_GetLength(&Handle->hRxQueue);
}

ErrorStatus PAL_Can_ReceiveFromInputBuffer(PalCan_HandleTypeDef *Handle, CanRxMsg *pRxMessage)
{
	return PAL_ObjectQueue_Dequeue(&Handle->hRxQueue, pRxMessage);
}

void PAL_Can_DiscardInputBbuffer(PalCan_HandleTypeDef *Handle)
{
	PAL_ObjectQueue_Clear(&Handle->hRxQueue);
}

ErrorStatus PAL_Can_SendToOutputBuffer(PalCan_HandleTypeDef *Handle, CanTxMsg *Msg)
{
	return PAL_ObjectQueue_Enqueue(&Handle->hTxQueue, Msg);
}

uint16_t PAL_Can_FlushOutputBuffer(PalCan_HandleTypeDef *Handle, uint32_t Timeout)
{
	uint16_t number_of_msg_flushed = 0;
	uint64_t expiredTick;
	CanTxMsg txmsg;
	
	if(Timeout == PAL_MAX_DELAY)
	{
		expiredTick = PAL_INVALID_TICK;
	}
	else
	{
		expiredTick = PAL_GetTick() + Timeout;
	}
	
	do
	{
		if(PAL_Can_GetTxFreeLevel(Handle) != 0 && PAL_ObjectQueue_GetLength(&Handle->hTxQueue) != 0)
		{
			PAL_ObjectQueue_Dequeue(&Handle->hTxQueue,&txmsg); 
			
			PAL_Can_Transmit(Handle, &txmsg);
			
			number_of_msg_flushed++;
		}
	}
	while(PAL_GetTick() <= expiredTick);
	
	return number_of_msg_flushed;
}

static uint32_t FilterIdTo32BitRegisterValue(PalCan_32BitFilterIdTypeDef *FilterId)
{
	uint32_t ret = 0;
	
	if(FilterId->RemoteRequest == SET)
	{
		ret |= 0x02;
	}
	if(FilterId->ExtendedId == SET)
	{
		ret |= 0x04;
	}
	ret |= (FilterId->ExtID_17_0 & 0x3ffff) << 3;
	ret |= (FilterId->StdID_10_0 & 0x07ff) << 21;
	
	return ret;
}

static uint16_t FilterIdTo16BitRegisterValue(PalCan_16BitFilterIdTypeDef *FilterId)
{
	uint16_t ret = 0;
	ret |= FilterId->ExtID_17_15 & 0x07;
	
	if(FilterId->ExtendedId == SET)
	{
		ret |= 0x01 << 4;
	}
	if(FilterId->RemoteRequest == SET)
	{
		ret |= 0x01 << 5;
	}
	
	ret |= (FilterId->StdID_10_0 & 0x07ff) << 6;
	
	return ret;
}

uint8_t PAL_Can_Transmit(PalCan_HandleTypeDef *Handle, CanTxMsg *Msg)
{
	return CAN_Transmit(CAN1, Msg);
}

void PAL_Can_CancelTransmit(PalCan_HandleTypeDef *Handle, uint8_t Mailbox)
{
	CAN_CancelTransmit(CAN1, Mailbox);
}

uint8_t PAL_Can_GetTxFreeLevel(PalCan_HandleTypeDef *Handle)
{
	uint8_t freelevel = 0;
	
	if((CAN1->TSR & CAN_TSR_TME0) != 0)
	{
		freelevel++;
	}
	if((CAN1->TSR & CAN_TSR_TME1) != 0)
	{
		freelevel++;
	}
	if((CAN1->TSR & CAN_TSR_TME2) != 0)
	{
		freelevel++;
	}
	
	return freelevel;
}
