#include <stdio.h>
#include <string.h>
#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "flash.h"
#include "interface.h"
#include "main.h"
#include "lcd.h" 

#include "init/SX028_QVGA40P_IPS_init.h"
#include "BlockWrite/blockwrite_default.h"	
//==================================================================
 void TEST_STAND()	//测试画面  
 {
		DispFrame();							//显示框线画面
		StopDelay(Delay_Time);		//画面延时
	 
		DispGrayHor16();					//显示16阶灰度画面
		StopDelay(Delay_Time);		//画面延时
	 
		Disp_Flash_Pic_Num(0);  	//显示图片
		StopDelay(Delay_Time);		//画面延时
	 
		DispBand();								//显示彩条画面
		StopDelay(Delay_Time);		//画面延时
	 
		DispColor(RED);						//显示红色画面
		StopDelay(Delay_Time);		//画面延时
	 
		DispColor(GREEN);					//显示绿色画面
		StopDelay(Delay_Time);		//画面延时
	 
		DispColor(BLUE);					//显示蓝色画面
		StopDelay(Delay_Time);		//画面延时
	 
		DispColor(WHITE);					//显示白色画面
		StopDelay(Delay_Time);		//画面延时
	 
		DispColor(BLACK);					//显示黑色画面
		StopDelay(Delay_Time);		//画面延时
}
 //==========================================
extern u16 POINT_COLOR;
int main(void)
{	
	SystemInit();//初始化RCC 设置系统主频为72MHZ
	delay_init(72);	//延时初始化
	uart_init(256000);	//串口初始化
	LCD_Init();		
	SPI_Flash_Init(); 
  
  while(1) 
	{
		#ifdef TEST_Stand	//正常测试
			TEST_STAND();
		#endif
		#ifdef TEST_STAND_H9B	//H9Bit测试
			TEST_STAND_9b();
		#endif
	}
}


