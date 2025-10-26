#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"


int main(void)
{
	OLED_Init();
	OLED_Clear();
	
	OLED_ShowChinese(10, 2, "你好世界啊");
	
	while (1)
	{
		
	}
}
