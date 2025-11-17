#include "interface.h"
extern LCD_CONFIG lcdconfig;	//LCD_CONFIG

void CS_SET(void){	
if(lcdconfig.lcd_interface==SPI4Line_CS_D8)
	GPIOB->BSRR=1<<8;
else
	GPIOC->BSRR=1<<3;
}
void CS_CLR(void){
if(lcdconfig.lcd_interface==SPI4Line_CS_D8)
	GPIOB->BRR=1<<8;
else
	GPIOC->BRR=1<<3;
}
#ifdef LCD_SPI3Line 
void  SendDataSPI(unsigned char dat)
{  
   unsigned char i;
   for(i=0; i<8; i++)			
   {  
      if( (dat&0x80)==0x80 )
				LCD_SPI_SDA_SET;
      else
				LCD_SPI_SDA_CLR;
      dat <<= 1;
			LCD_SPI_SCL_CLR;
      LCD_SPI_SCL_SET;			
   }
}
void WriteComm(u16 data)
{ 
	LCD_CS_CLR;
	
	LCD_SPI_SDA_CLR;
	LCD_SPI_SCL_CLR;
	LCD_SPI_SCL_SET;		
	
	SendDataSPI(data);
	
	LCD_CS_SET;
}
void WriteData(u16 data)
{
	LCD_CS_CLR;
	LCD_SPI_SDA_SET;
	LCD_SPI_SCL_CLR;
	LCD_SPI_SCL_SET;		
	SendDataSPI(data);
	LCD_CS_SET;
}

void SendData(unsigned int color)
{
	WriteData(color>>8);
	WriteData(color);
}
#endif
#ifdef LCD_SPI3Line2	
void  SendDataSPI(unsigned char dat)
{  
   unsigned char i;
   
   for(i=0; i<8; i++)			
   {  
      if( (dat&0x80)==0x80 )
				LCD_SPI_SDA_SET;
      else
				LCD_SPI_SDA_CLR;

      dat <<= 1;

			LCD_SPI_SCL_CLR;
      LCD_SPI_SCL_SET;			
   }
}
void WriteComm(u16 data)
{ 
	LCD_CS_CLR;
	
	LCD_SPI_SDA_CLR;
	LCD_SPI_SCL_CLR;
	LCD_SPI_SCL_SET;		
	
	SendDataSPI(data);
	
	LCD_CS_SET;
}
void WriteData(u16 data)
{
	LCD_CS_CLR;
	
	LCD_SPI_SDA_SET;
	LCD_SPI_SCL_CLR;
	LCD_SPI_SCL_SET;		
	
	SendDataSPI(data);
	
	LCD_CS_SET;
}
void SendData(unsigned int color)
{
	unsigned char i;
	u8 dat1,dat2;
	dat1=color>>8;
	dat2=color;
	LCD_SPI_SDA_SET;
	LCD_SPI_SDA2_SET;
	LCD_SPI_SCL_CLR;
	LCD_SPI_SCL_SET;		
   for(i=0; i<8; i++)			
   {  
		if((dat1&0x80)==0x80)
			LCD_SPI_SDA_SET;
		else
			LCD_SPI_SDA_CLR;
		 if((dat2&0x80)==0x80)
			LCD_SPI_SDA2_SET;
		else
			LCD_SPI_SDA2_CLR;
	 dat1 <<= 1;
	 dat2 <<= 1;
		LCD_SPI_SCL_CLR;
		LCD_SPI_SCL_SET;			
   }
}
#endif
#ifdef LCD_SPI4Line	 
void  SendDataSPI(unsigned char dat)
{  
   unsigned char i;
   for(i=0; i<8; i++)			
   {  
      if( (dat&0x80)==0x80 )
				LCD_SPI_SDA_SET;
      else
				LCD_SPI_SDA_CLR;
      dat <<= 1;
			LCD_SPI_SCL_CLR;
      LCD_SPI_SCL_SET;			
   }
}
void WriteComm(u16 data)
{ 
	LCD_CS_CLR;
	LCD_RS_CLR;
	
	#ifdef commdata16
		SendDataSPI(data>>8);
	#endif	
	SendDataSPI(data);
	
	LCD_CS_SET;
}
void WriteData(u16 data)
{
	LCD_CS_CLR;
	LCD_RS_SET;
	
	#ifdef commdata16
		SendDataSPI(data>>8);
	#endif	
	SendDataSPI(data);
	LCD_CS_SET;
}
void SendData(unsigned int color)
{
	SendDataSPI(color>>8);
	SendDataSPI(color);
}
#endif
#ifdef LCD_MCU8
void WriteComm(u16 data)
{ 
	LCD_CS_CLR;
	LCD_RD_SET;
	LCD_RS_CLR;
	
#ifdef commdata16
	DATAOUT(data>>8);
	LCD_WR_CLR;
	LCD_WR_SET;
#endif
	
	DATAOUT(data);
	LCD_WR_CLR;
	LCD_WR_SET;
	
	LCD_CS_SET;
}
void WriteData(u16 data)
{
	LCD_CS_CLR;
	LCD_RD_SET;
	LCD_RS_SET;
	
	#ifdef commdata16
		DATAOUT(data>>8);
		LCD_WR_CLR;
		LCD_WR_SET;
	#endif
	
	DATAOUT(data);
	LCD_WR_CLR;
	LCD_WR_SET;
	
	LCD_CS_SET;
}
void SendData(unsigned int color)
{
	DATAOUT(color>>8);
	LCD_WR_CLR;
	LCD_WR_SET;
	
	DATAOUT(color);
	LCD_WR_CLR;
	LCD_WR_SET;
}
#endif
#ifdef LCD_MCUH8
void WriteComm(u16 data)
{ 
	LCD_CS_CLR;
	LCD_RD_SET;
	LCD_RS_CLR;
	
	DATAOUT(data<<8);
	
	LCD_WR_CLR;
	LCD_WR_SET;
	
	LCD_CS_SET;
}
void WriteData(u16 data)
{
	LCD_CS_CLR;
	LCD_RD_SET;
	LCD_RS_SET;
	
	DATAOUT(data<<8);
	LCD_WR_CLR;
	LCD_WR_SET;
	
	LCD_CS_SET;
}
void SendData(unsigned int color)
{
	DATAOUT(color);
	LCD_WR_CLR;
	LCD_WR_SET;
	
	DATAOUT(color<<8);
	LCD_WR_CLR;
	LCD_WR_SET;
}
#endif
#ifdef LCD_MCU16
void WriteComm(u16 data)
{ 
	LCD_CS_CLR;
	LCD_RD_SET;
	LCD_RS_CLR;
	
	DATAOUT(data);
	
	LCD_WR_CLR;
	LCD_WR_SET;
	
	LCD_CS_SET;
}
void WriteData(u16 data)
{
	LCD_CS_CLR;
	LCD_RD_SET;
	LCD_RS_SET;
	
	DATAOUT(data);
	LCD_WR_CLR;
	LCD_WR_SET;
	
	LCD_CS_SET;
}
void SendData(unsigned int color)
{
	DATAOUT(color);
	LCD_WR_CLR;
	LCD_WR_SET;
}
#endif
void WriteCommData(u16 com,u16 data)
{
  WriteComm(com);
  WriteData(data);
}  


