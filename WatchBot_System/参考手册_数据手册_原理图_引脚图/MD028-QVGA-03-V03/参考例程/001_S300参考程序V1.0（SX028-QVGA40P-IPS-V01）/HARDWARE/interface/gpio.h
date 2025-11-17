#ifndef __LCD_H2
#define __LCD_H2	
#include "sys.h"	
//#include "stdlib.h"
//#include "usart.h"
//#include "delay.h"

#ifdef SPI4Line_CS_D8		//4线SPI CS设为DB8
	#define	LCD_CS_SET  	GPIOB->BSRR=1<<8    //片选端口  	PB8
	#define	LCD_CS_CLR  	GPIOB->BRR=1<<8    //片选端口  	PB8
#else
	#define	LCD_CS_SET  	GPIOC->BSRR=1<<3    //片选端口  	PC3
	#define	LCD_CS_CLR  	GPIOC->BRR=1<<3     //片选端口  	PC3
#endif

#define	LCD_RS_SET		GPIOC->BSRR=1<<4    //数据/命令   	PC4	   
#define	LCD_WR_SET		GPIOC->BSRR=1<<5    //写数据		PC5
#define	LCD_RD_SET		GPIOC->BSRR=1<<6    //读数据		PC6
#define	LCD_RST_SET		GPIOC->BSRR=1<<7   //复位			PC7
							    
#define	LCD_RS_CLR		GPIOC->BRR=1<<4     //数据/命令  	PC4	   
#define	LCD_WR_CLR		GPIOC->BRR=1<<5     //写数据		PC5
#define	LCD_RD_CLR		GPIOC->BRR=1<<6     //读数据		PC6
#define	LCD_RST_CLR		GPIOC->BRR=1<<7    //复位			PC7	

//IO SPI
#define	LCD_SPI_SCL_SET		GPIOC->BSRR=1<<5    //SPI_SCL
#define	LCD_SPI_SDA_SET		GPIOC->BSRR=1<<6    //SPI_SDA
#define	LCD_SPI_SDA2_SET		GPIOB->BSRR=1<<0    //SPI_SDA2
 
#define	LCD_SPI_SCL_CLR		GPIOC->BRR=1<<5     //SPI_SCL
#define	LCD_SPI_SDA_CLR		GPIOC->BRR=1<<6     //SPI_SDA
#define	LCD_SPI_SDA2_CLR		GPIOB->BRR=1<<0    //SPI_SDA2

#define DATAOUT(x) 		GPIOB->ODR=x; 		//数据输出  B
#define DATAIN   			GPIOB->IDR;   		//数据输入

#define	LCD_PD_SET  	GPIOD->BSRR=1<<2    //  	PD2
#define	LCD_PD_CLR       GPIOD->BRR=1<<2 

//#define DATAOUT(x) 		GPIOD->ODR=2; 		//数据输出  LED PD2

#define	TP_CS_CLR       GPIOA->BRR=1<<15 
#define	TP_PENIRQ_CLR   GPIOA->BRR=1<<8 
#define	TP_DOUT_CLR     GPIOC->BRR=1<<11 
#define	TP_DIN_CLR      GPIOC->BRR=1<<10 
//#define	TP_DCLK_CLR     GPIOA->BRR=1<<14 
#define	TP_DCLK_CLR     GPIOA->BRR=1<<2 

#define	TP_CS_SET        GPIOA->BSRR=1<<15 
#define	TP_PENIRQ_SET    GPIOA->BSRR=1<<8 
#define	TP_DOUT_SET      GPIOC->BSRR=1<<11 
#define	TP_DIN_SET       GPIOC->BSRR=1<<10 
//#define	TP_DCLK_SET      GPIOA->BSRR=1<<14 
#define	TP_DCLK_SET      GPIOA->BSRR=1<<2 
#define PEN_IRQ  GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_8)
#define DOUT     GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_11) 
#define LCD_SPI_SDO    GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_9) 
//==================================================================
///*按键定义*/
//==================================================================
#define ENTER    GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_2) //ENTER按键
#define KEY_STEP  GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_1)	//STEP按键
#define KEY_UP  GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_0)	//UP按键
//==================================================================
///*画笔颜色*/
//==================================================================
#define WHITE       0xFFFF
#define BLACK      	0x0000	  
#define BLUE       	0x001F 
#define BRED        0XF81F
#define GRED 			 	0XFFE0
#define GBLUE			 	0X07FF
#define RED         0xF800
#define MAGENTA     0xF81F
#define GREEN       0x07E0
#define CYAN        0x7FFF
#define YELLOW      0xFFE0
#define BROWN 			0XBC40 //棕色
#define BRRED 			0XFC07 //棕红色
#define GRAY  			0X8430 //灰色
#define GRAY25      0xADB5
//GUI颜色
#define DARKBLUE      	 0X01CF	//深蓝色
#define LIGHTBLUE      	 0X7D7C	//浅蓝色  
#define GRAYBLUE       	 0X5458 //灰蓝色
//以上三色为PANEL的颜色 
 
#define LIGHTGREEN     	0X841F //浅绿色
//#define LIGHTGRAY     0XEF5B //浅灰色(PANNEL)
#define LGRAY 			 		0XC618 //浅灰色(PANNEL),窗体背景色

#define LGRAYBLUE      	0XA651 //浅灰蓝色(中间层颜色)
#define LBBLUE          0X2B12 //浅棕蓝色(选择条目的反色)

#define X_min 0x0043		 //TP测试范围常量定义
#define X_max 0x07AE
#define Y_min 0x00A1
#define Y_max 0x0759

#endif
