#include "menu.h"
#include "remote_nec_driver.h"
#include <stdlib.h>
#include "OLED.h"
#include "GuessNum.h"
#include "GuessMine.h"
#include "Contact.h"
#include "Greedysnake.h"
#include "arm_control.h" 
#include "rtc_driver.h"
#include "Alarm.h"
#include "LED_Blink.h"


uint8_t Menu_Flag = 1;//主页状态标志位;
static uint8_t Menu_PageStart = 0;//页面起始 0/0-2， 1/1-3， 2/2-4， 3/3-5
static uint8_t Menu_Cursor = 0;//当前光标指向的位置

static char* menu_items[] = 
{
    "0通讯录",
    "1贪吃蛇",
    "2扫雷",
    "3猜数字",
		"4LED调速",
    "5定时器",
		"6机械臂",
};

//Random_Max：输入需要范围的随机数，返回值是0 — (Max-1)
//获取随机数
uint32_t GetRandom(uint32_t Random_Max)
{
	static uint8_t RandomInit_Flag = 0;
	if(!RandomInit_Flag)
	{
		RandomInit_Flag = 1;
		srand(TIM_GetCounter(TIM1));
	}
	return rand()% Random_Max;
}


//翻页显示
static void Menu_HomeShow(void)
{
    OLED_Clear();
	
    for(uint8_t i = 0; i < APP_PageSize; i++)
    {
        uint8_t index = (Menu_PageStart + i); // 当前要显示的第几个应用
        if(index < APP_Top)
				{
					OLED_Printf(0, (20*i), OLED_8X16, menu_items[index]);
					if(index == Menu_Cursor)
					{
						OLED_ReverseArea(0, (20*i), 70, 20);
					}
				}
    }
		//读当前时间已经移动到Alarm的时间比较函数下调用一次即可
		//RTC_ReadTime();
		OLED_Printf(70, 0, OLED_6X8, "%d-%d-%d", Rtctime.year, Rtctime.mon, Rtctime.day);
		OLED_Printf(76, 8, OLED_6X8, "%d:%d:%d", Rtctime.hour, Rtctime.min, Rtctime.sec);
    OLED_Update();
}
void Menu_Proc(void)
{
	if(Menu_Flag == 1)
	{
		Menu_HomeShow();
	}	
	if((IR_Flag == 1) && (Menu_Flag == 1))
	{
		IR_Flag = 0;
		switch(IR_GetKey())
		{
			case Key_W:
			{			
				if(Menu_Cursor == 0)
					Menu_Cursor = APP_Top - 1;
				else
					Menu_Cursor--;
					//光标超过当前页面的第一行
				if(Menu_Cursor < Menu_PageStart)
				{
					Menu_PageStart = Menu_Cursor;
				}
				
				break;
			}
			case Key_S:
			{
				Menu_Cursor++;
				if(Menu_Cursor >= APP_Top)
					Menu_Cursor = 0;
				
				//光标超过当前页面的最后一行
				if(Menu_Cursor >= Menu_PageStart + APP_PageSize)
				{
					 Menu_PageStart = Menu_Cursor - (APP_PageSize - 1);
				}
				break;
			}
			
			case Key_OK:
			{
				switch(Menu_Cursor)
				{
					case 0:
						Menu_Flag = 0;
						Contact_Flag = 1;
						OLED_Clear();
						break;
					case 1:
						Menu_Flag = 0;					
						SnakeState_Flag = 1;
						OLED_Clear();
						break;
					case 2:
						Menu_Flag = 0;
						GuessMine_Flag = 1;
						OLED_Clear();
						Mine_CursorShow();
						break;
					case 3:
						Menu_Flag = 0;
						GuessNum_Flag = 1;
						OLED_Clear();
						
						break;
					case 4:
						Menu_Flag = 0;
						LED_Blink_Flag = 1;
						OLED_Clear();
						OLED_Printf(0, 0, OLED_8X16, "Flicker:");
						OLED_Update();
					
						break;
					case 5:
						Menu_Flag = 0;					
						Alarm_Flag = 1;
						OLED_Clear();
						break;
					
					case 6:
						Menu_Flag = 0;
						Arm_Flag = 1;
						OLED_Clear();
						break;
				}
			}		
		}	
	}
}



