//=============================================================================
//文件名称:led.h
//文件概要:LED初始化
//库版本：V3.5.0
//版权所有:源地工作室 http://www.vcc-gnd.com/  网店 http://vcc-gnd.taobao.com/
//版本更新:2013-10-09 V1.0
//=============================================================================


//头文件
#include "led.h"


//=============================================================================
//函数名称: LED_GPIO_Config(void)
//功能概要:LED灯引脚配置
//参数名称:无
//函数返回:无
//=============================================================================
void LED_GPIO_Config(void)
{	
	//定义一个GPIO_InitTypeDef 类型的结构体，名字叫GPIO_InitStructure 
	GPIO_InitTypeDef  GPIO_InitStructure;
	//使能GPIOC的外设时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
	//选择要用的GPIO引脚		
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_13;
	///设置引脚模式为推免输出模式			 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 
	//设置引脚速度为50MHZ
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	//调用库函数，初始化GPIO
	GPIO_Init(GPIOC, &GPIO_InitStructure);
}

