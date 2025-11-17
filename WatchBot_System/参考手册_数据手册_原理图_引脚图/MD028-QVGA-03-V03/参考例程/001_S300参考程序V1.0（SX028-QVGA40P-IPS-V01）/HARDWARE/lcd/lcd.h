#ifndef __LCD_H2
#define __LCD_H2	
#include "sys.h"	
#include "main.h"	
//==================================================================
enum LCD_INTERFACE{MCU16,SPI4Line,MCU8,MCUH8,SPI3Line,SPI3Line2,SPI4Line_CS_D8};
typedef struct _LCD_CONFIG{
	int col;
	int row;
	int col_pre;
	int row_pre;
	int delay_time;
	u8 is_commdata16;
	enum LCD_INTERFACE lcd_interface;
	u32 pic_bytes;
}LCD_CONFIG;
//==================================================================
///*LCD-MCU端口定义*/
//==================================================================
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

#define	LCD_CS_SET  	CS_SET()    //片选端口  	PB8
#define	LCD_CS_CLR  	CS_CLR()    //片选端口  	PB8

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
//==================================================================
extern u16 BACK_COLOR, POINT_COLOR ;  
void LCD_CONFIG_INIT(void);
void LCD_Init(void);
void LCD_IC_Init(void);
void LCD_WR_REG(u16 data);
void LCD_WR_DATA(u16 data);
void SendData(unsigned int color);
void DispColor(unsigned int color);

void BlockWrite(unsigned int Xstart,unsigned int Xend,unsigned int Ystart,unsigned int Yend);
void LCD_GPIOInit(void);

void WriteOneDot(unsigned int color);
void WriteDispData(unsigned int color1,unsigned int color2);
void DispFrame(void);
void DispBand(void);
void DispGrayHor16(void);
void DispGrayHor32(void);
extern void TP_DrawLine(void);
void DrawGird(unsigned int color);
void PutPixel(unsigned int x,unsigned int y,unsigned int color);
void DispStr(unsigned char *str,unsigned int Xstart,unsigned int Ystart,unsigned int TextColor,unsigned int BackColor);
void DispInt(unsigned int i,unsigned int Xstart,unsigned int Ystart,unsigned int TextColor,unsigned int BackColor);
void StopDelay(unsigned int sdly);
void LCD_SetCursor(unsigned short Xpos, unsigned short Ypos);
void LCD_Fast_DrawPoint(unsigned short x,unsigned short y,unsigned short color);
void  LCD_DispChars_24a(/*坐标X*/unsigned short Char_X,/*坐标Y*/unsigned short Char_Y,/*显示内容*/char hz[],/*显示颜色*/uint16_t Char_Color,/*显示样式*/u8 XY_Mod );
//void LCD_ShowChar(uint16_t x,uint16_t y,uint16_t fc, uint16_t bc, uint8_t num,uint8_t size,uint8_t mode);
void LCD_ShowString(u16 x,u16 y,u16 width,u16 height,u8 size,u8 *p);
void LCD_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2);
void LCD_DrawLine(u16 x1, u16 y1, u16 x2, u16 y2);
void LCD_DrawPoint(u16 x,u16 y);
//void LCD_ShowChar1(uint16_t x,uint16_t y,uint16_t fc, uint16_t bc, uint8_t num,uint8_t size,uint8_t mode);
void LCD_ShowCharaa(uint16_t x,uint16_t y,uint16_t fc, uint16_t bc, uint8_t num,uint8_t size,uint8_t mode);
void LCD_ShowCharqq(uint16_t x,uint16_t y,uint16_t fc, uint16_t bc, uint8_t num,uint8_t size,uint8_t mode);
void Show_Str(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, uint8_t *str,uint8_t size,uint8_t mode);
//void LCD_ShowCharaa(u16 x,u16 y,u8 num,u8 size,u8 mode);
void LCD_ShowChar(u16 x,u16 y,u8 num,u8 size,u8 mode);
void DispColor_9bit_window(unsigned char r,unsigned char g,unsigned char b);
void DispBand_9b(void);	 
void DispFrame_9b(void);
void DispGrayHor16_9b(void); 
void DispChess(u8 div);
void TEST_STAND_9b(void);
#endif
