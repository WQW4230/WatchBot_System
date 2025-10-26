#include <GuessNum.h>
#include "Scheduler.h"
#include "OLED.h"
#include "NEC_driver.h"

extern void Menu_Proc(void);//退出后进入的任务进程
extern uint32_t GetRandom(uint32_t Random_Max);//依赖生成随机数

Guess_State_t Guess_State;//句柄

//输入大小判断
void Size_Judgment(uint32_t GuessNum) 
{	
	OLED_ClearArea(0, 16, 128, 16);
	if(GuessNum > Guess_State.Guess_Answer)
		OLED_Printf(0, 16, OLED_8X16, "大了");
	if(GuessNum < Guess_State.Guess_Answer)
		OLED_Printf(0, 16, OLED_8X16, "小了");
	if(GuessNum == Guess_State.Guess_Answer)
		OLED_Printf(0, 16, OLED_8X16, "对了");
}

//oled显示
void Guess_Show(void)
{
	OLED_ClearArea(0, 32, 128, 16);
	OLED_Printf(0, 0, OLED_8X16, "请猜数字0-%d", NUM_MAX);
	OLED_Printf(0, 32, OLED_8X16, "%d", Guess_State.GuessNum);
}

//是否是第一次进入
void GuessGame_FirstEntry(void)
{
	if(Guess_State.first_enter == 0)
	{
		OLED_Clear();
		Guess_State.first_enter = 1;
		Guess_State.Guess_Answer = GetRandom(NUM_MAX);
		Guess_Show();
	}
}


void GuessNum_Proc(void)
{		
	GuessGame_FirstEntry();//检测是否第一次进入
	
	uint8_t key;	//存按键值
	if(IR_GetKey(&key) == -1) return;//无按键或错误
	
	switch(key)
	{
		case Key_OK:
			Size_Judgment(Guess_State.GuessNum);
			break;
		
		//星号退出
		case Key_XingHao:
			vTask_Delete(GuessNum_Proc);
			Scheduler_AddTask(Menu_Proc, 100, 5, 1000);
			Guess_State.first_enter = 0;
			Guess_State.GuessNum = 0;
			break;
		
		//#号清空
		case Key_JingHao:
			Guess_State.GuessNum = 0; 
			break;
		
		default:
			if(key > 9) break;//不是数字键退出
			Guess_State.GuessNum = Guess_State.GuessNum * 10 + key;		
	}
	
	Guess_Show();//显示
}
