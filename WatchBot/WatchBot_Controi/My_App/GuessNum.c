#include <GuessNum.h>
#include <stdlib.h> 
#include "OLED.h"
#include "menu.h"
#include "NEC_driver.h"

uint8_t GuessNum_Flag = 0;

static uint8_t GuessNum_Show_Flag = 0;//游戏界面显示

static uint16_t r;//答案
static uint32_t GuessNum_Guess = 0;

//判断输入值是否正确
void HighLow(void)
{	
	OLED_ClearArea(0, 0, 32, 16);
	OLED_ClearArea(0, 40, 32, 16);
	//生成随机数
	if(r == 0)
	{
		r = (TIM_GetCounter(TIM1) % 1000) + 1;
	}
	if(GuessNum_Guess > r)
	{
		OLED_Printf(0, 40, OLED_8X16, "大了");		
	}
	else if(GuessNum_Guess < r)
	{
		OLED_Printf(0, 40, OLED_8X16, "小了");
	}
	else
	{
		OLED_Printf(0, 40, OLED_8X16, "对了");
		r = 0;
	}
	OLED_UpdateArea(0, 40, 32, 16);
	GuessNum_Guess = 0;
}

void Get_GuessNum(void)
{
	if(NEC_RxFlag == 1)
	{
		NEC_RxFlag = 0;
		uint8_t GuessNum_Key = IR_GetKey();
		//按键为0-9Guess才进行自增
		if(GuessNum_Key <= 9)
		{
			GuessNum_Guess = ((GuessNum_Guess * 10) + GuessNum_Key);
			OLED_Printf(0, 0, OLED_8X16, "%d", GuessNum_Guess);
			OLED_UpdateArea(0, 0, 64, 16);
		}
	
		switch(GuessNum_Key)
		{
			
			case Key_XingHao:
			{
				Menu_Flag = 1;
				GuessNum_Flag = 0;
				GuessNum_Guess = 0;
				GuessNum_Show_Flag = 0;
				r = 0;
				break;
			}		
			case Key_OK:
			{
				HighLow();
				break;
			}
			
		}
	}
}

//游戏显示函数，只显示一次
void GuessNum_Show()
{
	if(GuessNum_Show_Flag == 0)
	{
		OLED_Printf(0, 20, OLED_8X16, "请猜数字:0-1000");
		OLED_UpdateArea(0, 20, 128, 16);
		GuessNum_Show_Flag = 1;
	}
}

void GuessNum_Proc(void)
{
	if(	GuessNum_Flag == 1 )
	{		
		GuessNum_Show();
		Get_GuessNum();		
	}
	
}
