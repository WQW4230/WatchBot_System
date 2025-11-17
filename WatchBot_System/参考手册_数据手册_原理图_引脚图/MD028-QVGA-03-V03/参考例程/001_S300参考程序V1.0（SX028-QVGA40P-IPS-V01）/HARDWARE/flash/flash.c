#include <stdio.h>
#include <string.h>
#include "sys.h"
#include "delay.h"   
#include "spi.h"
#include "usart.h"
#include "flash.h" 
#include "main.h"
#include "lcd.h"
#include "interface.h"

u8 is_Fread;
//u8 is_Erase_Chip_end=0; 
extern LCD_CONFIG lcdconfig;	
u32 pic_start_addr=4096<<2;
PIC_INFO pic_info;	
u8 start_x=2;	//X起始位置

void SPI_Flash_Init(void)	//flash初始化
{
	RCC->APB2ENR|=1<<2;       //PORTA时钟使能 	    
	GPIOA->CRL&=0XFFF000FF; 
	GPIOA->CRL|=0X00033300;//PA2.3.4 推挽 	    
	GPIOA->ODR|=0X7<<2;    //PA2.3.4上拉
	SPIx_Init();
}  
//==============================================
//==============================================
//读取SPI_FLASH的状态寄存器
//BIT7  6   5   4   3   2   1   0
//SPR   RV  TB BP2 BP1 BP0 WEL BUSY
//SPR:默认0,状态寄存器保护位,配合WP使用
//TB,BP2,BP1,BP0:FLASH区域写保护设置
//WEL:写使能锁定
//BUSY:忙标记位(1,忙;0,空闲)
//默认:0x00
u8 SPI_Flash_ReadSR(void)   
{  
	u8 byte=0;   
	SPI_FLASH_CS=0;                            //使能器件   
	SPIx_ReadWriteByte(W25X_ReadStatusReg);    //发送读取状态寄存器命令    
	byte=SPIx_ReadWriteByte(0Xff);             //读取一个字节  
	SPI_FLASH_CS=1;                            //取消片选     
	return byte;   
} 
//写SPI_FLASH状态寄存器
//只有SPR,TB,BP2,BP1,BP0(bit 7,5,4,3,2)可以写!!!
void SPI_FLASH_Write_SR(u8 sr)   
{   
	SPI_FLASH_CS=0;                            //使能器件   
	SPIx_ReadWriteByte(W25X_WriteStatusReg);   //发送写取状态寄存器命令    
	SPIx_ReadWriteByte(sr);               //写入一个字节  
	SPI_FLASH_CS=1;                            //取消片选     	      
}   
//SPI_FLASH写使能	
//将WEL置位   
void SPI_FLASH_Write_Enable(void)   
{
	SPI_FLASH_CS=0;                            //使能器件   
    SPIx_ReadWriteByte(W25X_WriteEnable);      //发送写使能  
	SPI_FLASH_CS=1;                            //取消片选     	      
} 
//SPI_FLASH写禁止	
//将WEL清零  
void SPI_FLASH_Write_Disable(void)   
{  
	SPI_FLASH_CS=0;                            //使能器件   
    SPIx_ReadWriteByte(W25X_WriteDisable);     //发送写禁止指令    
	SPI_FLASH_CS=1;                            //取消片选     	      
} 			    
//读取芯片ID W25X16的ID:0XEF14
u16 SPI_Flash_ReadID(void)
{
	u16 Temp = 0;	  
	SPI_FLASH_CS=0;				    
	SPIx_ReadWriteByte(0x90);//发送读取ID命令	    
	SPIx_ReadWriteByte(0x00); 	    
	SPIx_ReadWriteByte(0x00); 	    
	SPIx_ReadWriteByte(0x00); 	 			   
	Temp|=SPIx_ReadWriteByte(0xFF)<<8;  
	Temp|=SPIx_ReadWriteByte(0xFF);	 
	SPI_FLASH_CS=1;				    
	return Temp;
}   		    
//读取SPI FLASH  
//在指定地址开始读取指定长度的数据
//pBuffer:数据存储区
//ReadAddr:开始读取的地址(24bit)
//NumByteToRead:要读取的字节数(最大65535)
void SPI_Flash_Read(u8* pBuffer,u32 ReadAddr,u16 NumByteToRead)   
{ 
 	u16 i;    												    
	SPI_FLASH_CS=0;                            //使能器件   
    SPIx_ReadWriteByte(W25X_ReadData);         //发送读取命令   
    SPIx_ReadWriteByte((u8)((ReadAddr)>>16));  //发送24bit地址    
    SPIx_ReadWriteByte((u8)((ReadAddr)>>8));   
    SPIx_ReadWriteByte((u8)ReadAddr);   
    for(i=0;i<NumByteToRead;i++)
		{ 
        pBuffer[i]=SPIx_ReadWriteByte(0XFF);   //循环读数  
    }
	SPI_FLASH_CS=1;                            //取消片选     	      
}  
//SPI在一页(0~65535)内写入少于256个字节的数据
//在指定地址开始写入最大256字节的数据
//pBuffer:数据存储区
//WriteAddr:开始写入的地址(24bit)
//NumByteToWrite:要写入的字节数(最大256),该数不应该超过该页的剩余字节数!!!	 
void SPI_Flash_Write_Page(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite)
{
 	u16 i;  
    SPI_FLASH_Write_Enable();                  //SET WEL 
	SPI_FLASH_CS=0;                            //使能器件   
    SPIx_ReadWriteByte(W25X_PageProgram);      //发送写页命令   
    SPIx_ReadWriteByte((u8)((WriteAddr)>>16)); //发送24bit地址    
    SPIx_ReadWriteByte((u8)((WriteAddr)>>8));   
    SPIx_ReadWriteByte((u8)WriteAddr);   
    for(i=0;i<NumByteToWrite;i++)SPIx_ReadWriteByte(pBuffer[i]);//循环写数  
	SPI_FLASH_CS=1;                            //取消片选 
	SPI_Flash_Wait_Busy();					   //等待写入结束
} 
//无检验写SPI FLASH 
//必须确保所写的地址范围内的数据全部为0XFF,否则在非0XFF处写入的数据将失败!
//具有自动换页功能 
//在指定地址开始写入指定长度的数据,但是要确保地址不越界!
//pBuffer:数据存储区
//WriteAddr:开始写入的地址(24bit)
//NumByteToWrite:要写入的字节数(最大65535)
//CHECK OK
void SPI_Flash_Write_NoCheck(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite)  
{
	u16 pageremain;	   
	pageremain=256-WriteAddr%256; //单页剩余的字节数		 	    
	if(NumByteToWrite<=pageremain)pageremain=NumByteToWrite;//不大于256个字节
	while(1)
	{	   
		SPI_Flash_Write_Page(pBuffer,WriteAddr,pageremain);
		if(NumByteToWrite==pageremain)break;//写入结束了
	 	else //NumByteToWrite>pageremain
		{
			pBuffer+=pageremain;
			WriteAddr+=pageremain;	

			NumByteToWrite-=pageremain;			  //减去已经写入了的字节数
			if(NumByteToWrite>256)pageremain=256; //一次可以写入256个字节
			else pageremain=NumByteToWrite; 	  //不够256个字节了
		}
	};	    
} 
//写SPI FLASH  
//在指定地址开始写入指定长度的数据
//该函数带擦除操作!
//pBuffer:数据存储区
//WriteAddr:开始写入的地址(24bit)
//NumByteToWrite:要写入的字节数(最大65535)  		   
u8 SPI_FLASH_BUF[4096];
void SPI_Flash_Write(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite)   
{ 
	u32 secpos;
	u16 secoff;
	u16 secremain;	   
 	u16 i;    

	secpos=WriteAddr/4096;//扇区地址 0~511 for w25x16
	secoff=WriteAddr%4096;//在扇区内的偏移
	secremain=4096-secoff;//扇区剩余空间大小   

	if(NumByteToWrite<=secremain)secremain=NumByteToWrite;//不大于4096个字节
	while(1) 
	{	
		SPI_Flash_Read(SPI_FLASH_BUF,secpos*4096,4096);//读出整个扇区的内容
		for(i=0;i<secremain;i++)//校验数据
		{
			if(SPI_FLASH_BUF[secoff+i]!=0XFF)break;//需要擦除  	  
		}
		if(i<secremain)//需要擦除
		{
			SPI_Flash_Erase_Sector(secpos);//擦除这个扇区
			for(i=0;i<secremain;i++)	   //复制
			{
				SPI_FLASH_BUF[i+secoff]=pBuffer[i];	  
			}
			SPI_Flash_Write_NoCheck(SPI_FLASH_BUF,secpos*4096,4096);//写入整个扇区  

		}else SPI_Flash_Write_NoCheck(pBuffer,WriteAddr,secremain);//写已经擦除了的,直接写入扇区剩余区间. 				   
		if(NumByteToWrite==secremain)break;//写入结束了
		else//写入未结束
		{
			secpos++;//扇区地址增1
			secoff=0;//偏移位置为0 	 

		   	pBuffer+=secremain;  //指针偏移
			WriteAddr+=secremain;//写地址偏移	   
		   	NumByteToWrite-=secremain;				//字节数递减
			if(NumByteToWrite>4096)secremain=4096;	//下一个扇区还是写不完
			else secremain=NumByteToWrite;			//下一个扇区可以写完了
		}	 
	};	 	 
}
//==========（下载图片）=================	
//等待是否图片下载，是则擦除FLASH
void waiteflash(void)
{
		while(1);
}
//擦除整个芯片
//整片擦除时间:
//W25X16:25s 
//W25X32:40s 
//W25X64:40s 
//等待时间超长...
void SPI_Flash_Erase_Chip_ALL(void)   
{
    SPI_Flash_Wait_Busy();   
    SPI_FLASH_Write_Enable();                  //SET WEL 
    SPI_Flash_Wait_Busy();   
  	SPI_FLASH_CS=0;                            //使能器件   
   SPIx_ReadWriteByte(W25X_ChipErase);        //发送片擦除命令  
		SPI_FLASH_CS=1;                            //取消片选     	      
		SPI_Flash_Wait_Busy();   				   //等待芯片擦除结束
}   
//擦除一个扇区
//Dst_Addr:扇区地址 0~511 for w25x16
//擦除一个山区的最少时间:150ms
void SPI_Flash_Erase_Sector(u32 Dst_Addr)   
{   
//	Dst_Addr*=4096;
		Dst_Addr=Dst_Addr<<12;
    SPI_FLASH_Write_Enable();                  //SET WEL 	 
    SPI_Flash_Wait_Busy();   
  	SPI_FLASH_CS=0;                            //使能器件   
    SPIx_ReadWriteByte(W25X_SectorErase);      //发送扇区擦除指令 
    SPIx_ReadWriteByte((u8)((Dst_Addr)>>16));  //发送24bit地址    
    SPIx_ReadWriteByte((u8)((Dst_Addr)>>8));   
    SPIx_ReadWriteByte((u8)Dst_Addr);  
	SPI_FLASH_CS=1;                            //取消片选     	      
    SPI_Flash_Wait_Busy();   				   //等待擦除完成
}  
//等待空闲
void SPI_Flash_Wait_Busy(void)   
{ 
	while ((SPI_Flash_ReadSR()&0x01)==0x01);   // 等待BUSY位清空
}  
//进入掉电模式
void SPI_Flash_PowerDown(void)   
{ 
  	SPI_FLASH_CS=0;                            //使能器件   
    SPIx_ReadWriteByte(W25X_PowerDown);        //发送掉电命令  
	SPI_FLASH_CS=1;                            //取消片选     	      
    delay_us(3);                               //等待TPD  
}   
//唤醒
void SPI_Flash_WAKEUP(void)   
{  
  	SPI_FLASH_CS=0;                            //使能器件   
    SPIx_ReadWriteByte(W25X_ReleasePowerDown);   //  send W25X_PowerDown command 0xAB    
	SPI_FLASH_CS=1;                            //取消片选     	      
    delay_us(3);                               //等待TRES1
}   
unsigned char pic_data[2];
void DisPic_Windows_From_FLASH(u32 ReadAddr,u32 NumByteToRead) 
{
	u32 j;
	LCD_CS_CLR;
	LCD_RS_SET;
	LCD_RD_SET;
	  SPI_Flash_Init();
		 for(j=0;j<NumByteToRead;j++)
	{

				SPI_FLASH_CS=0;                            //使能器件 
				SPIx_ReadWriteByte(W25X_ReadData);         //发送读取命令 
			//	SPIx_ReadWriteByte(W25X_FastReadData); 	
				SPIx_ReadWriteByte((u8)((ReadAddr+(j))>>16));  //发送24bit地址    
				SPIx_ReadWriteByte((u8)((ReadAddr+(j))>>8));   
				SPIx_ReadWriteByte((u8)ReadAddr+(j));
		
				//pic_data[0]=SPIx_ReadWriteByte(0XFF);		//dummy
	
				pic_data[0]=SPIx_ReadWriteByte(0XFF);   //循环读数 
				pic_data[1]=SPIx_ReadWriteByte(0XFF);  
					SPI_FLASH_CS=0;   
		
					DATAOUT((pic_data[0]));
					LCD_WR_CLR;
					LCD_WR_SET;
		
					DATAOUT((pic_data[1]));
					LCD_WR_CLR;
					LCD_WR_SET;
	} 
		LCD_CS_SET;
		SPI_FLASH_CS=1;                                //取消片选 
}
void Disp_Flash_Pic_9b_Num(u8 num)	//H9BIT取图片
{
	Read_FlashPic_9b(lcdconfig.pic_bytes*num + pic_start_addr,lcdconfig.pic_bytes); 
}
void Read_FlashPic_9b(u32 ReadAddr,u32 NumByteToRead)  	//H9BIT取图片 
{ 
	u32 i;
	int data_r,data_g,data_b;
	int data_first_9bit,data_second_9bit;
	//	LCD_GPIOInit();
	//	LCD_WR_REG(0x36);
	//	LCD_WR_DATA(0x08);
	//  delay_ms(10);
	BlockWrite(lcdconfig.col_pre,lcdconfig.col+lcdconfig.col_pre-1,lcdconfig.row_pre,lcdconfig.row+lcdconfig.row_pre-1);
	 // SPI_Flash_Init();
		SPI_FLASH_CS=0;                              //使能器件 
		LCD_RD_SET;
		LCD_RS_SET;
		LCD_CS_CLR;
	
    SPIx_ReadWriteByte(W25X_ReadData);         //发送读取命令   
    SPIx_ReadWriteByte((u8)((ReadAddr)>>16));  //发送24bit地址    
    SPIx_ReadWriteByte((u8)((ReadAddr)>>8));   
    SPIx_ReadWriteByte((u8)ReadAddr);  	
	
		for(i=0;i<NumByteToRead/3;i++)
		{ 
			data_r=(SPIx_ReadWriteByte(0XFF));
			data_g=(SPIx_ReadWriteByte(0XFF));
			data_b=(SPIx_ReadWriteByte(0XFF));
			
//			if(i<16)
//			printf("%2x %2x %2x ",data_r,data_g,data_b);
			
//			data_first_9bit=(((data_r&0xfc)<<1)|(((data_g&0xfc)>>5)&0x07))<<7;
//			data_second_9bit=((((data_g&0x00fc)<<4)&0x1c0)|(((data_b&0x00fc)>>2)&0x3f))<<7;		   
			
			data_first_9bit=((data_r&0xfc)<<8)|(((data_g&0xe0)<<2));
			data_second_9bit=((data_g&0x1c)<<11)|((data_b&0x00fc)<<5);		   
			

			DATAOUT(data_first_9bit);
			LCD_WR_CLR;
			LCD_WR_SET;
			
			DATAOUT(data_second_9bit);
			LCD_WR_CLR;
			LCD_WR_SET;
		} 
		SPI_FLASH_CS=1;                              //取消片选 
		LCD_CS_SET;
	}
void Disp_Flash_Pic_Num(u8 num)   	//按序号从Flash取图片，第一张为0
{ 
		u32 i;
		u8 data_h,data_l; 
		u32 ReadAddr;
		u32 NumByteToRead;
	
		//地址为lcd一张图片字节总数x图片序号
		ReadAddr=lcdconfig.pic_bytes*num + pic_start_addr;	//跳过第1扇区
		NumByteToRead=lcdconfig.pic_bytes;
		BlockWrite(lcdconfig.col_pre,lcdconfig.col+lcdconfig.col_pre-1,lcdconfig.row_pre,lcdconfig.row+lcdconfig.row_pre-1);
		SPI_FLASH_CS=0;                              //使能器件 
		LCD_RD_SET;
		LCD_RS_SET;
		LCD_CS_CLR;
		is_Fread=1;
    SPIx_ReadWriteByte(W25X_ReadData);         //发送读取命令   
    SPIx_ReadWriteByte((u8)((ReadAddr)>>16));  //发送24bit地址    
    SPIx_ReadWriteByte((u8)((ReadAddr)>>8));   
    SPIx_ReadWriteByte((u8)ReadAddr);  	
		for(i=0;i<(NumByteToRead>>1);i++)
		{ 
			data_h=	(SPIx_ReadWriteByte(0XFF));
			data_l=(SPIx_ReadWriteByte(0XFF));
			SendData(((u16)(data_h)<<8)|(data_l));
		} 
		SPI_FLASH_CS=1;                              //取消片选 
		LCD_CS_SET;
		is_Fread=0;
}
void Read_FlashPic(u32 ReadAddr,u32 NumByteToRead)   
{ 
		u32 i;
		u8 data_h,data_l; 
	//	LCD_GPIOInit();
	//	LCD_WR_REG(0x36);
	//	LCD_WR_DATA(0x08);
	//  delay_ms(10);
		BlockWrite(lcdconfig.col_pre,lcdconfig.col+lcdconfig.col_pre-1,lcdconfig.row_pre,lcdconfig.row+lcdconfig.row_pre-1);
	 // SPI_Flash_Init();
		SPI_FLASH_CS=0;                              //使能器件 
		LCD_RD_SET;
		LCD_RS_SET;
		LCD_CS_CLR;
		is_Fread=1;
    SPIx_ReadWriteByte(W25X_ReadData);         //发送读取命令   
    SPIx_ReadWriteByte((u8)((ReadAddr)>>16));  //发送24bit地址    
    SPIx_ReadWriteByte((u8)((ReadAddr)>>8));   
    SPIx_ReadWriteByte((u8)ReadAddr);  	
	
		for(i=0;i<NumByteToRead>>1;i++)
		{ 
			data_h=	(SPIx_ReadWriteByte(0XFF));
			data_l=(SPIx_ReadWriteByte(0XFF));
			
		//	LCD_WR_DATA(((u16)(data_h)<<8)|(data_l));
//			LCD_CS_CLR;
			
			
//		DATAOUT(((u16)(data_h)<<8)|(data_l));
//			LCD_WR_CLR;
//			LCD_WR_SET;
			SendData(((u16)(data_h)<<8)|(data_l));
			
//		DATAOUT(data_l<<8);
//			LCD_WR_CLR;
//			LCD_WR_SET;
//		DATAOUT(data_h<<8);
//			LCD_WR_CLR;
//			LCD_WR_SET;

			
//		SendData(((u16)(data_h)<<8)|(data_l));
			
			
//			LCD_CS_SET;
	//		printf("%2x\r\n",(((u16)(data_h)<<8)|(data_l)));
//			DATAOUT((SPIx_ReadWriteByte(0XFF)));	
//			LCD_WR_CLR;
//			LCD_WR_SET;
//			DATAOUT((SPIx_ReadWriteByte(0XFF)));	
//			LCD_WR_CLR;
//			LCD_WR_SET;
		
		//	printf("%x %x ",pic_data[j],pic_data[j+1]);
		} 

		SPI_FLASH_CS=1;                              //取消片选 
		LCD_CS_SET;
}
 
void Read_FlashPic_H(u32 ReadAddr,u32 NumByteToRead)   
{ 
		u32 i;
		u8 data_h,data_l;
		LCD_GPIOInit();
//		LCD_WR_REG(0x36);
//		LCD_WR_DATA(0x08);//
		delay_ms(10);
		BlockWrite(lcdconfig.col_pre,lcdconfig.col+lcdconfig.col_pre-1,lcdconfig.row_pre,lcdconfig.row+lcdconfig.row_pre-1);
	  SPI_Flash_Init();
		SPI_FLASH_CS=0;                              //使能器件 
		LCD_RD_SET;
		LCD_RS_SET;
		LCD_CS_CLR;
    SPIx_ReadWriteByte(W25X_ReadData);         //发送读取命令   
    SPIx_ReadWriteByte((u8)((ReadAddr)>>16));  //发送24bit地址    
    SPIx_ReadWriteByte((u8)((ReadAddr)>>8));   
    SPIx_ReadWriteByte((u8)ReadAddr);  	
	
		for(i=0;i<NumByteToRead/2;i++)
		{ 
			data_l=	(SPIx_ReadWriteByte(0XFF));
			data_h=(SPIx_ReadWriteByte(0XFF));	
			DATAOUT(((u16)(data_h)<<8)|(data_l));
			LCD_WR_CLR;
			LCD_WR_SET;
		} 
		SPI_FLASH_CS=0;      
		LCD_CS_SET;
	}

