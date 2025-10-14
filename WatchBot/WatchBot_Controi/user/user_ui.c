/**
  ******************************************************************************
  * @file    user_ui.c
  * @author  铁头山羊
  * @version V 1.0.0
  * @date    2023年4月24日
  * @brief   用户图形界面显示
  ******************************************************************************
  * @attention
  * Copyright (c) 2022 - 2023 东莞市明玉科技有限公司. 保留所有权力.
  ******************************************************************************
*/
#include "user_ui.h"
#include "stm32f10x_pal_oled.h"
#include "stm32f10x_pal_adc.h"
#include "pal_pid.h"
#include <stdio.h>

extern PalADC_HandleTypeDef hadc;
extern PalOled_HandleTypeDef holed;
extern PalPID_HandleTypeDef pid_balance;
extern PalPID_HandleTypeDef pid_speed;
extern float hcsr04Value;// = 3.402823E38;

static void FormatFloatNumber(float Number, char *pBuf);

void User_UI_Update(void)
{
	float battary = PAL_ADC_SingleMode_Convert(&hadc) * 3.3 / 2 * 3;
	
	PAL_OLED_Clear(&holed);
	
	PAL_OLED_SetFont(&holed, font_5x8_matrix);
	
	PAL_OLED_SetPen(&holed, PEN_COLOR_WHITE, 1);
	
	PAL_OLED_MoveCursor(&holed, 0, 0);
	
	PAL_OLED_Printf(&holed, "Battary: %.2f", battary);
	
	PAL_OLED_MoveCursorX(&holed, 0);
	PAL_OLED_OffsetCursor(&holed, 0, 16);
	PAL_OLED_DrawString(&holed, "Balance PID");
	
	PAL_OLED_MoveCursorX(&holed, 0);
	PAL_OLED_OffsetCursor(&holed, 0, 8);
	
	char buf[8];
	FormatFloatNumber(pid_balance.Kp, buf);
	PAL_OLED_DrawString(&holed, buf);
	PAL_OLED_OffsetCursor(&holed, 5, 0);
	FormatFloatNumber(pid_balance.Ki, buf);
	PAL_OLED_DrawString(&holed, buf);
	PAL_OLED_OffsetCursor(&holed, 5, 0);
	FormatFloatNumber(pid_balance.Kd, buf);
	PAL_OLED_DrawString(&holed, buf);
	
	PAL_OLED_MoveCursorX(&holed, 0);
	PAL_OLED_OffsetCursor(&holed, 0, 8);
	PAL_OLED_DrawString(&holed, "Speed PID");
	
	PAL_OLED_MoveCursorX(&holed, 0);
	PAL_OLED_OffsetCursor(&holed, 0, 8);
	
	FormatFloatNumber(pid_speed.Kp, buf);
	PAL_OLED_DrawString(&holed, buf);
	PAL_OLED_OffsetCursor(&holed, 5, 0);
	FormatFloatNumber(pid_speed.Ki, buf);
	PAL_OLED_DrawString(&holed, buf);
	PAL_OLED_OffsetCursor(&holed, 5, 0);
	FormatFloatNumber(pid_speed.Kd, buf);
	PAL_OLED_DrawString(&holed, buf);
	
	PAL_OLED_MoveCursorX(&holed, 0);
	PAL_OLED_OffsetCursor(&holed, 0, 8);
	PAL_OLED_DrawString(&holed, "hc-sr04 ");
	if(hcsr04Value != 3.402823E38)
	{
		FormatFloatNumber(hcsr04Value, buf);
	}
	else
	{
		sprintf(buf, "NA");
	}
	PAL_OLED_DrawString(&holed, buf);
	
	PAL_OLED_SendBuffer(&holed);
}

static void FormatFloatNumber(float Number, char *pBuf)
{
	int e;
	
	if(Number >= 0 && Number < 1.00e-9)
	{
		sprintf(pBuf, "+0.00+0");
	} 
	else if(Number < 0 && Number > -1.00e9)
	{
		sprintf(pBuf, "-0.00+0");
	}
	else
	{
		// 符号位
		pBuf[0] = Number>=0?'+':'-';
		
		if(Number < 0) Number = -Number;
		
		// 计算指数
		if(Number < 1)
		{
			e = 0;
			while(Number < 1)
			{
				Number = Number * 10;
				e--;
			}
		}
		else if(Number > 1)
		{
			e = 0;
			while(Number > 10)
			{
				Number = Number / 10;
				e++;
			}
		}
		// 有效位
		sprintf(pBuf+1, "%.2f", Number);
		
		// 指数符号
		pBuf[5] = e >= 0 ? '+':'-';
		
		// 符号位
		if(e<0) e=-e;
		sprintf(pBuf+6, "%d", e);
	}	
	
	pBuf[7] = '\0';
}
