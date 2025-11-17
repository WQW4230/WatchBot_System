#ifndef __MAIN_H
#define __MAIN_H	
#include "sys.h"	
//==================================================================
///*LCD 分辨率*/
////==================================================================
#define COL  240			//显示列数
#define ROW  320			//显示行数
#define COL_Pre  0			//空列数
#define ROW_Pre  0			//空行数
#define Delay_Time 500
////==================================================================
///*LCD 接口选择 interface select(SPI3,SPI4,MCU8,MCUH8,MCU16)*/
//==================================================================
//#define LCD_SPI3Line				//3线SPI
//#define LCD_SPI4Line				//4线SPI
//#define LCD_SPI4Line_CS_D8	//4线SPI CS设为DB8
//#define LCD_MCU8						//MCU低8位
//#define LCD_MCUH8						//MCU高8位
#define LCD_MCU16						//MCU16位
//==================================================================
//#define commdata16						//16位指令
//==================================================================
///*测试画面*/ 
#define TEST_Stand		  //正常测试画面开启
//#define TEST_STAND_H9B		//H9Bit测试
//#define String_Enable		//显示字符功能
//==================================================================
#endif
