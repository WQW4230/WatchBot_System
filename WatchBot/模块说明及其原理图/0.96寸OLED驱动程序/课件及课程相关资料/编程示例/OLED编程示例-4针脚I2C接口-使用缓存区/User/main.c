#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include <math.h>

int main(void)
{
	OLED_Init();
	
	OLED_Clear();
	
	for (uint16_t i = 0; i < 360; i += 20)
	{
		OLED_DrawLine(64, 32, 64 + 30 * cos(i / 360.0 * 2 * 3.14), 32 + 30 * sin(i / 360.0 * 2 * 3.14));
	}
	

	OLED_Update();
	
	
	while (1)
	{
		
	}
}
