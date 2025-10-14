/**
  ******************************************************************************
  * @file    stm32f10x_pal_oled.h
  * @author  铁头山羊stm32工作组
  * @version V1.0.0
  * @date    2023年2月24日
  * @brief   oled驱动程序
  ******************************************************************************
  * @attention
  * Copyright (c) 2022 - 2023 东莞市明玉科技有限公司. 保留所有权力.
  ******************************************************************************
*/

#ifndef _STM32F10X_PAL_OLED_I2C_H_
#define _STM32F10X_PAL_OLED_I2C_H_

#include "stm32f10x.h"
#include "stm32f10x_pal.h"
#include "stm32f10x_pal_i2c.h"
#include "stm32f10x_pal_oled_font.h"

// 定义屏幕尺寸
#define OLED_SCREEN_COLS 128 
#define OLED_SCREEN_ROWS  64  
#define OLED_SCREEN_PAGES 8   

#define OLED_COLOR_TRANSPARENT 0x00 // 透明画笔
#define OLED_COLOR_WHITE       0x01 // 白色画笔
#define OLED_COLOR_BLACK       0x02 // 黑色画笔

#define PEN_COLOR_TRANSPARENT OLED_COLOR_TRANSPARENT // 透明画笔
#define PEN_COLOR_WHITE OLED_COLOR_WHITE // 白色画笔
#define PEN_COLOR_BLACK OLED_COLOR_BLACK // 黑色画笔

#define BRUSH_COLOR_TRANSPARENT OLED_COLOR_TRANSPARENT // 透明画刷
#define BRUSH_COLOR_WHITE OLED_COLOR_WHITE // 白色画刷
#define BRUSH_COLOR_BLACK OLED_COLOR_BLACK // 黑色画刷

typedef struct
{
	FlagStatus UseDMA;
	FlagStatus SA0; /* 地址线SA0，
	                 * SA0 = L, Slave Address = 0x78
	                 * SA0 = H, Slave Address = 0x7a
	                 */
}PalOled_AdvancedInitTypeDef;

typedef struct
{
	PalI2C_HandleTypeDef *hi2c;
	PalOled_AdvancedInitTypeDef Advanced;
}PalOled_InitTypeDef;

typedef struct 
{
	PalOled_InitTypeDef Init;
	uint8_t UseBufferA;
	uint8_t *pBufferA;
	uint8_t *pBufferB;
	uint16_t SlaveAddress;
	const uint8_t *Font;
	uint8_t PenColor;
	uint8_t PenWidth;
	uint8_t Brush;
	int16_t CursorX;
	int16_t CursorY;
}PalOled_HandleTypeDef;

ErrorStatus PAL_OLED_Init(PalOled_HandleTypeDef *Handle);
       void PAL_OLED_Clear(PalOled_HandleTypeDef *Handle);

       void PAL_OLED_SetFont(PalOled_HandleTypeDef *Handle, const uint8_t *Font);
       void PAL_OLED_SetPen(PalOled_HandleTypeDef *Handle, uint8_t Pen_Color, uint8_t Width);
       void PAL_OLED_SetBrush(PalOled_HandleTypeDef *Handle, uint8_t Brush_Color);

       void PAL_OLED_MoveCursor(PalOled_HandleTypeDef *Handle, int16_t X, int16_t Y);
       void PAL_OLED_MoveCursorX(PalOled_HandleTypeDef *Handle, int16_t X);
       void PAL_OLED_MoveCursorY(PalOled_HandleTypeDef *Handle, int16_t Y);
       void PAL_OLED_OffsetCursor(PalOled_HandleTypeDef *Handle, int16_t OffsetX, int16_t OffsetY);
       void PAL_OLED_GetCursor(PalOled_HandleTypeDef *Handle, int16_t *X, int16_t *Y);
    int16_t PAL_OLED_GetCursorX(PalOled_HandleTypeDef *Handle);
    int16_t PAL_OLED_GetCursorY(PalOled_HandleTypeDef *Handle);

       void PAL_OLED_DrawCharator(PalOled_HandleTypeDef *Handle, char Ch);
       void PAL_OLED_DrawString(PalOled_HandleTypeDef *Handle, const char *Str);
       void PAL_OLED_Printf(PalOled_HandleTypeDef *Handle, const char *Format, ...);
   uint16_t PAL_OLED_GetFontHeight(const uint8_t *Font);
   uint16_t PAL_OLED_GetFontMaxWidth(const uint8_t *Font);
   uint16_t PAL_OLED_GetChWidth(PalOled_HandleTypeDef *Handle, char Ch);
   uint16_t PAL_OLED_GetStrWidth(PalOled_HandleTypeDef *Handle, const char *Str);
   uint16_t PAL_OLED_GetStrHeight(PalOled_HandleTypeDef *Handle, const char *Str);

       void PAL_OLED_DrawLine(PalOled_HandleTypeDef *Handle, int16_t Start_x, int16_t Start_y, int16_t End_x, int16_t End_y); 
       void PAL_OLED_DrawCircle(PalOled_HandleTypeDef *Handle, int16_t X, int16_t Y, uint16_t Radius);
       void PAL_OLED_DrawRect(PalOled_HandleTypeDef *Handle, int16_t X, int16_t Y, uint16_t Width, uint16_t Height);

       void PAL_OLED_DrawBitmap(PalOled_HandleTypeDef *Handle, int16_t X, int16_t Y, uint16_t Width, uint16_t Height, const uint8_t *pBitmap);

       void PAL_OLED_Clear(PalOled_HandleTypeDef *Handle);
ErrorStatus PAL_OLED_SendBuffer(PalOled_HandleTypeDef *Handle);

#endif
