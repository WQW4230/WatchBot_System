#include "stm32_usart_slave.h"
#include "stm32f10x.h" 

uint8_t stm32_usart_RxData[20]; // 数据帧
uint8_t stm32_RxData_len = 0;       //数据帧长度
uint8_t USART_Rx_Flag = 0;      //置位表示命令收到

//初始化配置gpio和usart时钟参数
void usart_slave_Init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	GPIO_InitTypeDef GPIO_StructInit;
	GPIO_StructInit.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_StructInit.GPIO_Speed =GPIO_Speed_50MHz;
	GPIO_StructInit.GPIO_Pin = GPIO_Pin_10;
	GPIO_Init(GPIOB,&GPIO_StructInit);
	
	GPIO_StructInit.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_StructInit.GPIO_Pin = GPIO_Pin_11;
	GPIO_Init(GPIOB,&GPIO_StructInit);
	
	USART_InitTypeDef USART_InitStruct;
	USART_InitStruct.USART_BaudRate = 115200;
	USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_InitStruct.USART_WordLength = USART_WordLength_8b;
  USART_InitStruct.USART_StopBits = USART_StopBits_1;
  USART_InitStruct.USART_Parity = USART_Parity_No ;
	USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	
  USART_Init(USART3, &USART_InitStruct);
	
	//中断配置
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
	
	NVIC_InitTypeDef NVIC_InitStruct;
	NVIC_InitStruct.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0; 
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);
	
	USART_Cmd(USART3, ENABLE);
	
}


//中断响应函数
void USART3_IRQHandler(void)
{
	if(USART_GetITStatus(USART3, USART_IT_RXNE) == SET)
	{
		stm32_usart_RxData[stm32_RxData_len++] = USART_ReceiveData(USART3);
		
		//帧头不对
		if(stm32_usart_RxData[0] != 0x88)
		{
			stm32_RxData_len = 0;
			return;
		}
		
		//如果帧尾为0x66表示一帧数据已经发送完成
		//一帧最少5个字节
		if((stm32_RxData_len >= 5) &&(stm32_usart_RxData[stm32_RxData_len - 1] == 0x66))
		{
			//置位可用
			USART_Rx_Flag= 1;
			stm32_RxData_len = 0;
		}
	}
}
