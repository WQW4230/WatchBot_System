#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"


int main(void)
{
	OLED_Init();
	OLED_Printf(0, 0, OLED_8X16, "通讯录");	
	OLED_Printf(0, 20, OLED_8X16, "扫雷");	
	OLED_Printf(0, 40, OLED_8X16, "贪吃蛇");	
	OLED_Update();
	OLED_ReverseArea(0, 0, 64, 16);
	OLED_Update();
	
	while (1)
	{
		
	}
}
