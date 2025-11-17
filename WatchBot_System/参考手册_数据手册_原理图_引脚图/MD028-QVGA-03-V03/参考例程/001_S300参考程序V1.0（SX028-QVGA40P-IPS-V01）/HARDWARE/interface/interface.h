#include "lcd.h" 
#ifndef __INTERFACE_H
#define __INTERFACE_H		

void CS_SET(void);
void CS_CLR(void);
void WriteComm(u16 data);		//写指令函数，指令参数为16位数据
void WriteData(u16 data);		//写指令参数函数，指令参数为16位数据
void SendData(unsigned int color);	//发送16位数据
void SendDataSPI(unsigned char dat);	//发送8位SPI数据

#endif
