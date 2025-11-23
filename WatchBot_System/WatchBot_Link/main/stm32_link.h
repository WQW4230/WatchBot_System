#ifndef _STM32_LINK_H_
#define _STM32_LINK_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "esp_err.h"

#define USER_UART_NUM UART_NUM_1
#define UART_TX_IO 19
#define UART_RX_IO 20
// #define UART_TX_IO 47
// #define UART_RX_IO 14
#define UART_BUFFER_SIZE        1024

#define FRAME_HEADER 0x88 //帧头
#define FRAME_END    0x66 //帧尾
#define DATA_MIN_LEN 5		 //一帧最少几字节
#define DATA_MAX_LEN 8    //最多8个
#define DATA_ARM_LEN 10   //控制机械臂的一帧为10字节

#define FRAME_IDX_HEADER   0   								// 帧头在第0位
#define FRAME_IDX_CMD      1  							  // 命令在第1位
#define FRAME_IDX_LEN      2   								// 数据长度在第2位
#define FRAME_IDX_DATA     3   								// 数据从第3位开始
#define FRAME_IDX_END_MIN	 DATA_MIN_LEN	- 1		// 一帧5字节时帧尾位置
#define FRAME_IDX_END_MAX  DATA_MAX_LEN	- 1		// 一帧8字节时帧尾位置
#define FRAME_IDX_END_ARM  DATA_ARM_LEN - 1   // 一帧10字节时候帧尾的位置

/*
一帧的数据结构如下

帧头     0x88
命令     0x**   
一帧长度 0x**

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
	uint8_t Data_Len;        //当前完整数据的长度
	uint32_t ESP32_Tx_Flag;  //当前标志位挂起时表示从stm32接收到了一帧完整的数据
}Frame_t;

esp_err_t stm32_link_init(void);
esp_err_t stm32_send_frame(const uint8_t *data);
esp_err_t stm32_read_frame(uint8_t *data);

////////////////////////测试用/////////////////////////////
void USART_ProcessByte(Frame_t *USART_Frame, uint8_t byte);

#ifdef __cplusplus
}
#endif

#endif
