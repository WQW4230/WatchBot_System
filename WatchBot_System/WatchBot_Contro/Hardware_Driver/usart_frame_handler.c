#include "usart_frame_handler.h"
#include "stm32f10x.h" 

uint8_t ESP32_Rx_Flag = 0;

#define FRAME_HEADER 0x88 //帧头
#define FRAME_END    0x66 //帧尾
#define DATA_MIN_LEN 5		 //一帧最少几位
#define DATA_MAX_LEN 8    //最多8个

#define FRAME_IDX_HEADER   0   								// 帧头在第0位
#define FRAME_IDX_CMD      1  							  // 命令在第1位
#define FRAME_IDX_LEN      2   								// 数据长度在第2位
#define FRAME_IDX_DATA     3   								// 数据从第3位开始
#define FRAME_IDX_END_MIN	 DATA_MIN_LEN	- 1		// 一帧8字节时帧尾位置
#define FRAME_IDX_END_MAX  DATA_MAX_LEN	- 1		// 一帧5字节时帧尾位置

/*
一帧的数据结构如下

帧头     0x88
命令     
一帧长度

数据
...
数据

帧尾     0x66

*/
typedef enum
{
	STATE_WAIT_EADER,   //等待帧头
	STATE_RECV_DATA,    //接收数据
	
}USATR_State;

typedef struct Frame_t
{
	USATR_State State;
	uint8_t ESP_Data[DATA_MAX_LEN]; // 数据帧
	uint8_t Data_index;       //数据帧当前长度
}Frame_t;

static Frame_t USART_Frame = {STATE_WAIT_EADER, {0}, 0};


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
/*
	中断处理函数
*/
void USART_ProcessByte(uint8_t byte)
{
	switch(USART_Frame.State)
	{
		//等待帧头
		case STATE_WAIT_EADER:
		{
			if(byte == FRAME_HEADER)
			{
				USART_Frame.Data_index = 0;
				USART_Frame.ESP_Data[USART_Frame.Data_index] = byte;
				USART_Frame.Data_index += 1 ;
				USART_Frame.State = STATE_RECV_DATA;				
			}
			else
			{
				USART_Frame.Data_index = 0 ;
			}		
		}
		break;
		
		//接收数据
		case STATE_RECV_DATA:
		{
			USART_Frame.ESP_Data[USART_Frame.Data_index] = byte;
			USART_Frame.Data_index += 1 ;
			
			switch(USART_Frame.Data_index)
			{
				//数据帧长度为5时 校验数据位长度并校验帧尾是否正确
				case DATA_MIN_LEN:
				{
					//接收成功
					if(USART_Frame.ESP_Data[FRAME_IDX_LEN] == DATA_MIN_LEN && USART_Frame.ESP_Data[FRAME_IDX_END_MIN] == FRAME_END)
					{
						ESP32_Rx_Flag = 1;
						USART_Frame.State = STATE_WAIT_EADER;
					}
					else if(USART_Frame.ESP_Data[FRAME_IDX_LEN] == DATA_MIN_LEN&& USART_Frame.Data_index == DATA_MIN_LEN) 
					{
						//如果是5字节的并且索引已经超过5字节则是乱码
						USART_Frame.State = STATE_WAIT_EADER;
					}
				}		
				break;
				//数据帧长度为8时 校验数据位长度并校验帧尾是否正确
				case DATA_MAX_LEN:
				{
					//接收成功
					if(USART_Frame.ESP_Data[FRAME_IDX_LEN] == DATA_MAX_LEN && USART_Frame.ESP_Data[FRAME_IDX_END_MAX] == FRAME_END)
					{
						ESP32_Rx_Flag = 1;
						USART_Frame.State = STATE_WAIT_EADER;
					}
					else if(USART_Frame.ESP_Data[FRAME_IDX_LEN] == DATA_MAX_LEN&& USART_Frame.Data_index == DATA_MAX_LEN) 
					{
						//如果是8字节的并且索引已经超过8字节则是乱码
						USART_Frame.State = STATE_WAIT_EADER;
					}
				}	
				break;
				
			}	
		}
		break;	

	}
}

//中断响应函数
void USART3_IRQHandler(void)
{
	if(USART_GetITStatus(USART3, USART_IT_RXNE) == SET)
	{
		uint8_t byte = USART_ReceiveData(USART3);
		USART_ProcessByte(byte);
	}
}
