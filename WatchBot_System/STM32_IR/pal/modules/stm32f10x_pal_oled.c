/**
  ******************************************************************************
  * @file    stm32f10x_pal_oled.c
  * @author  铁头山羊stm32工作组
  * @version V1.0.0
  * @date    2023年2月24日
  * @brief   oled驱动程序
  ******************************************************************************
  * @attention
  * Copyright (c) 2022 - 2023 东莞市明玉科技有限公司. 保留所有权力.
  ******************************************************************************
*/
	
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "stm32f10x_pal_oled.h"
#include "default_font.h"
#include <math.h>

// 定义屏幕尺寸
#define OLED_SCREEN_COLS 128 
#define OLED_SCREEN_ROWS  64  
#define OLED_SCREEN_PAGES 8 

// SSD1306命令
#define SSD1306_CTRL_COMMAND           0x80  // Continuation bit=1, D/C=0; 1000 0000
#define SSD1306_CTRL_COMMAND_STREAM    0x00  // Continuation bit=0, D/C=0; 0000 0000
#define SSD1306_CTRL_DATA              0xc0  // Continuation bit=1, D/C=1; 1100 0000
#define SSD1306_CTRL_DATA_STREAM       0x40  // Continuation bit=0, D/C=1; 0100 0000

typedef struct
{
	int16_t X;
	int16_t Y;
	uint16_t Width;
	uint16_t Height;
}Rect; /* 矩形 */

static ErrorStatus OLED_SendCommand(PalOled_HandleTypeDef *Handle, const uint8_t Cmd, const uint8_t *Arg, uint16_t Size);
static ErrorStatus OLED_SendData(PalOled_HandleTypeDef *Handle, const uint8_t *pData, uint16_t Size);
static void DrawCircleFrame(PalOled_HandleTypeDef *Handle, int16_t X, int16_t Y, uint16_t Radius);
static void FillCircle(PalOled_HandleTypeDef *Handle, int16_t X, int16_t Y, uint16_t Radius);
static void DrawRectFrame(PalOled_HandleTypeDef *Handle, int16_t X, int16_t Y, uint16_t Width, uint16_t Height);
static void FillRect(PalOled_HandleTypeDef *Handle, int16_t X, int16_t Y, uint16_t Width, uint16_t Height);
static int16_t unicode_2_glyph_idx(PalOled_HandleTypeDef *Handle, uint32_t Unicode);
static void DrawCharator(PalOled_HandleTypeDef *Handle, uint32_t Unicode);
static void BrushDot(PalOled_HandleTypeDef *Handle, int16_t x, int16_t y);
static void PenDot(PalOled_HandleTypeDef *Handle, int16_t x, int16_t y);
static void DrawBitmapEx(PalOled_HandleTypeDef *Handle,int16_t X, int16_t Y, uint16_t Width, uint16_t Height, const uint8_t *pBitmap);
static uint16_t GetGlyphWidth(PalOled_HandleTypeDef *Handle, uint32_t Unicode);

// 
// @简介：OLED初始化
// @参数：Handle - OLED显示器的句柄
// @返回值：SUCCESS - 成功，ERROR - 失败
//
ErrorStatus PAL_OLED_Init(PalOled_HandleTypeDef *Handle)
{
	// 计算从机地址
	// SA0 = L -> 0x78
	// SA0 = H -> 0x7a
	if(Handle->Init.SA0 == RESET)
	{
		Handle->SlaveAddress = 0x78;
	}
	else
	{
		Handle->SlaveAddress = 0x7a;
	}
	
	// 给缓冲区A分配空间
	Handle->pBuffer = (uint8_t *)malloc(OLED_SCREEN_COLS * OLED_SCREEN_PAGES * sizeof(uint8_t));
	if(Handle->pBuffer == 0)
	{
		return ERROR;
	}
	
	memset(Handle->pBuffer, 0, OLED_SCREEN_COLS * OLED_SCREEN_PAGES * sizeof(uint8_t));
	
	uint8_t arg;
	
	CHECK(OLED_SendCommand(Handle, 0xae, 0, 0)) /* display off*/
	arg = 0x80;
	CHECK(OLED_SendCommand(Handle, 0xd5, &arg, 1)) /* clock divide ratio (0x00=1) and oscillator frequency (0x8) */
	arg = 0x3f;
	CHECK(OLED_SendCommand(Handle, 0xa8, &arg, 1)) /* multiplex ratio */
	arg = 0x00;
	CHECK(OLED_SendCommand(Handle, 0xd3, &arg, 1)) /* vertical shift */
	
	CHECK(OLED_SendCommand(Handle, 0x40, 0, 0)) /* set display start line to 0 */
	arg = 0x14;
	CHECK(OLED_SendCommand(Handle, 0x8d, &arg, 1)) /* [2] charge pump setting (p62): 0x014 enable, 0x010 disable, SSD1306 only, should be removed for SH1106 */
	arg = 0x00;
	CHECK(OLED_SendCommand(Handle, 0x20, &arg, 1)) /* horizontal addressing mode */
	
	CHECK(OLED_SendCommand(Handle, 0xa1, 0, 0)) /* segment remap a0/a1 */
	CHECK(OLED_SendCommand(Handle, 0xc8, 0, 0)) /* c0: scan dir normal, c8: reverse */
	
	arg = 0x12;
	CHECK(OLED_SendCommand(Handle, 0xda, &arg, 1)) /* com pin HW config, sequential com pin config (bit 4), disable left/right remap (bit 5) */
	arg = 0xcf;
	CHECK(OLED_SendCommand(Handle, 0x81, &arg, 1)) /* [2] set contrast control */
	arg = 0xf1;
	CHECK(OLED_SendCommand(Handle, 0xd9, &arg, 1)) /* [2] pre-charge period 0x022/f1*/
	arg = 0x20;
	CHECK(OLED_SendCommand(Handle, 0xdb, &arg, 1)) /* vcomh deselect level */
	
	// if vcomh is 0, then this will give the biggest range for contrast control issue #98
  // restored the old values for the noname constructor, because vcomh=0 will not work for all OLEDs, #116
	CHECK(OLED_SendCommand(Handle, 0x2e, 0, 0)) /* Deactivate scroll */
	CHECK(OLED_SendCommand(Handle, 0xa4, 0, 0)) /* output ram to display */
	CHECK(OLED_SendCommand(Handle, 0xa6, 0, 0)) /* none inverted normal display mode */
	
	CHECK(OLED_SendCommand(Handle, 0xaf, 0, 0)) /* display on*/
	
	Handle->PenWidth = 1; // 默认线宽1
	Handle->PenColor = PEN_COLOR_WHITE; // 默认白色画笔
	Handle->Brush = BRUSH_TRANSPARENT; // 默认白色画刷
	
	Handle->CursorX = 0;
	Handle->CursorY = 0; // 光标默认在屏幕的左上角
	
	Handle->RefreshProgress = 0;
	
	Handle->TextRegionX = 0; // 默认关闭文本框功能
	Handle->TextRegionY = 0;
	Handle->TextRegionWidth = 0;
	Handle->TextRegionHeight = 0;
	
	Handle->Font = &default_font; // 使用默认字体，8*8点阵字体
	
	return SUCCESS;
}

//
// @简介：设置剪切区域
// @参数：Handle - OLED显示器的句柄
// @参数：X - 剪切区域左上角的横坐标
// @参数：Y - 剪切区域左上角的纵坐标
// @参数：Wdith - 剪切区域宽度
// @参数：Height - 剪切区域高度
//
void PAL_OLED_StartClipRegion(PalOled_HandleTypeDef *Handle, int16_t X, int16_t Y, uint16_t Width, uint16_t Height)
{
	Handle->ClipRegionX = X;
	Handle->ClipRegionY = Y;
	Handle->ClipRegionWidth = Width;
	Handle->ClipRegionHeight = Height;
}

//
// @简介：关闭剪切区域
// @参数：Handle - OLED显示器的句柄
//
void PAL_OLED_StopClipRegion(PalOled_HandleTypeDef *Handle)
{
	Handle->ClipRegionX = 0;
	Handle->ClipRegionY = 0;
	Handle->ClipRegionWidth = 0;
	Handle->ClipRegionHeight = 0;
}

//
// @简介：获取屏幕的宽度
// @返回值：屏幕宽度，单位：像素
//
uint16_t PAL_OLED_GetScreenWidth(PalOled_HandleTypeDef *Handle)
{
	return OLED_SCREEN_COLS;
}

//
// @简介：获取屏幕高度
// @返回值：屏幕高度，单位：像素
//
uint16_t PAL_OLED_GetScreenHeight(PalOled_HandleTypeDef *Handle)
{
	return OLED_SCREEN_ROWS;
}

//
// @简介：清空缓冲区内容
// @参数：Handle - 显示器的句柄
//
void PAL_OLED_Clear(PalOled_HandleTypeDef *Handle)
{
	memset(Handle->pBuffer, 0, OLED_SCREEN_COLS * OLED_SCREEN_PAGES);
}

// 
// @简介：设置字体
// @参数：Handle - 显示器的句柄
// @参数：Font - 字体，如果要使用默认字体，可填写&default_font
//
void PAL_OLED_SetFont(PalOled_HandleTypeDef *Handle, const PalFont_TypeDef *Font)
{
	Handle->Font = Font;
}

//
// @简介：设置画笔的颜色和宽度
// @参数：Handle - 显示器的句柄
// @参数：Pen_Color - 画笔颜色
//             PEN_COLOR_TRANSPARENT - 透明
//             PEN_COLOR_WHITE - 白色
//             PEN_COLOR_BLACK - 黑色
// @参数：Width - 画笔宽度
// 
void PAL_OLED_SetPen(PalOled_HandleTypeDef *Handle, uint8_t Pen_Color, uint8_t Width)
{
	Handle->PenColor = Pen_Color;
	Handle->PenWidth = Width;
}

//
// @简介：设置画刷
// @参数：Handle - 显示器的句柄
// @参数：Brush_Color - 画刷颜色
//             BRUSH_TRANSPARENT - 透明
//             BRUSH_WHITE - 白色
//             BRUSH_BLACK - 黑色
// 
void PAL_OLED_SetBrush(PalOled_HandleTypeDef *Handle, uint8_t Brush_Color)
{
	Handle->Brush = Brush_Color;
}

//
// @简介：将光标设置到坐标点（X，Y）处
// @参数：Handle - 显示器的句柄
// @参数：X - 光标横坐标
// @参数：Y - 光标纵坐标
// 
void PAL_OLED_SetCursor(PalOled_HandleTypeDef *Handle, int16_t X, int16_t Y)
{
	Handle->CursorX = X;
	Handle->CursorY = Y;
}

//
// @简介：将光标的横坐标设置到X处
// @参数：Handle - 显示器的句柄
// @参数：X - 光标横坐标
// 
void PAL_OLED_SetCursorX(PalOled_HandleTypeDef *Handle, int16_t X)
{
	Handle->CursorX = X;
}

//
// @简介：将光标的纵坐标设置到Y处
// @参数：Handle - 显示器的句柄
// @参数：Y - 光标纵坐标
// 
void PAL_OLED_SetCursorY(PalOled_HandleTypeDef *Handle, int16_t Y)
{
	Handle->CursorY = Y;
}

//
// @简介：移动光标
// @参数：Handle - 显示器的句柄
// @参数：dX - 横向移动的距离
// @参数：dY - 纵向移动的距离
// 
void PAL_OLED_MoveCursor(PalOled_HandleTypeDef *Handle, int16_t dX, int16_t dY)
{
	Handle->CursorX += dX;
	Handle->CursorY += dY;
}

//
// @简介：横向移动光标
// @参数：Handle - 显示器的句柄
// @参数：dX - 横向移动的距离
// 
void PAL_OLED_MoveCursorX(PalOled_HandleTypeDef *Handle, int16_t dX)
{
	Handle->CursorX += dX;
}

//
// @简介：纵向移动光标
// @参数：Handle - 显示器的句柄
// @参数：dY - 纵向移动的距离
// 
void PAL_OLED_MoveCursorY(PalOled_HandleTypeDef *Handle, int16_t dY)
{
	Handle->CursorY += dY;
}

//
// 简介：获取光标的当前位置
// @参数：Handle - 显示器的句柄
// @参数：pXOut - 输出参数，用于接收光标的横坐标
// @参数：pYOut - 输出参数，用于接收光标的纵坐标
// 
void PAL_OLED_GetCursor(PalOled_HandleTypeDef *Handle, int16_t *pXOut, int16_t *pYOut)
{
	*pXOut = Handle->CursorX;
	*pYOut = Handle->CursorY;
}

//
// 简介：获取光标的横坐标
// @参数：Handle - 显示器的句柄
// @返回值：光标的横坐标值
// 
int16_t PAL_OLED_GetCursorX(PalOled_HandleTypeDef *Handle)
{
	return Handle->CursorX;
}

//
// 简介：获取光标的纵坐标
// @参数：Handle - 显示器的句柄
// @返回值：光标的纵坐标值
// 
int16_t PAL_OLED_GetCursorY(PalOled_HandleTypeDef *Handle)
{
	return Handle->CursorY;
}

#define min(x1,x2) x1>x2?x2:x1
#define max(x1,x2) x1>x2?x1:x2

static Rect GetOverlappedRect(Rect rect1, Rect rect2)
{
	Rect ret = {0,0,0,0};
	
	int16_t xl = max(rect1.X,rect2.X);
	int16_t xr = min(rect1.X + rect1.Width, rect2.X + rect2.Width);
	
	int16_t yt = max(rect1.Y,rect2.Y);
	int16_t yb = min(rect1.Y + rect1.Height, rect2.Y + rect2.Height);
	
	if(xl<xr&&yt<yb)
	{
		ret.X = xl;
		ret.Y = yt;
		ret.Width = xr - xl;
		ret.Height = yb - yt;
	}
	
	return ret;
}

static uint16_t GetGlyphWidth(PalOled_HandleTypeDef *Handle, uint32_t Unicode)
{
	if(Handle->Font == NULL) return 0; // 未设置字体
	
	int16_t idx = unicode_2_glyph_idx(Handle, Unicode);
	
	if(idx < 0) return 0; // 未找到对应的字形
	
	return Handle->Font->Glyphs[idx].Dwx0;
}

static void DrawCharator(PalOled_HandleTypeDef *Handle, uint32_t Unicode)
{
	if(Handle->Font == NULL) return; // 未设置字体
	
	int16_t idx = unicode_2_glyph_idx(Handle, Unicode);
	
	const PalGlyph_TypeDef *pGlyph = 0;
	
	if(idx >=0)
	{
		pGlyph = &Handle->Font->Glyphs[idx];
	}
	
	int16_t clipRegionXCpy, clipRegionYCpy, clipRegionWidthCpy, clipRegionHeightCpy;
	
	// 如果启用了文本框区域
	if(Handle->TextRegionWidth!=0 && Handle->TextRegionHeight!=0)
	{
		// 备份剪切区域
		clipRegionXCpy = Handle->ClipRegionX;
		clipRegionYCpy = Handle->ClipRegionY;
		clipRegionWidthCpy = Handle->ClipRegionWidth;
		clipRegionHeightCpy = Handle->ClipRegionHeight;
		
		// 将剪切区域设置到与文本框区域相交处
		
		if(Handle->ClipRegionWidth != 0 && Handle->ClipRegionHeight != 0)
		{
			Rect rect1 = {Handle->TextRegionX, Handle->TextRegionY, Handle->TextRegionWidth, Handle->TextRegionHeight};
			Rect rect2 = {Handle->ClipRegionX, Handle->ClipRegionY, Handle->ClipRegionWidth, Handle->ClipRegionHeight};
			Rect overlapped = GetOverlappedRect(rect1, rect2);
			
			Handle->ClipRegionX = overlapped.X;
			Handle->ClipRegionY = overlapped.Y;
			Handle->ClipRegionWidth = overlapped.Width;
			Handle->ClipRegionHeight = overlapped.Height;
		}
		else
		{
			Handle->ClipRegionX = Handle->TextRegionX;
			Handle->ClipRegionY = Handle->TextRegionY;
			Handle->ClipRegionWidth = Handle->TextRegionWidth;
			Handle->ClipRegionHeight = Handle->TextRegionHeight;
		}
		
		// 如果光标在文本框之外
		if(Handle->CursorX<Handle->TextRegionX || Handle->CursorX>=Handle->TextRegionX+Handle->TextRegionWidth)
		{
			Handle->CursorX = Handle->TextRegionX; // 让光标回到起始点
		}
		
		if(pGlyph != 0)
		{
			// 如果下一个字符的宽度超过了文本框
			if(Handle->CursorX+pGlyph->Dwx0 >= Handle->TextRegionX+Handle->TextRegionWidth)
			{
				Handle->CursorX = Handle->TextRegionX; // 让光标回到起始点
				Handle->CursorY = Handle->CursorY + Handle->Font->FBBy + Handle->Font->FBBYoff;
			}
		}
		
		// 如果为\r（回车）
		if(Unicode == '\r')
		{
			Handle->CursorX = Handle->TextRegionX;
		}
		else if(Unicode == '\n')
		{
			Handle->CursorY = Handle->CursorY + Handle->Font->FBBy + Handle->Font->FBBYoff;
		}
	}
	
	if(pGlyph != 0)
	{
		// 绘制背景
		FillRect(Handle, Handle->CursorX, Handle->CursorY - Handle->Font->FBBYoff - Handle->Font->FBBy, pGlyph->Dwx0, Handle->Font->FBBy);
		
		// 绘制字形
		DrawBitmapEx(Handle, Handle->CursorX + pGlyph->BBxoff0x, Handle->CursorY - pGlyph->BByoff0y - pGlyph->BBh, pGlyph->BBw, pGlyph->BBh, pGlyph->Bitmap);
	}
	
	// 如果启用了文本框区域
	if(Handle->TextRegionWidth!=0 && Handle->TextRegionHeight!=0)
	{
		// 恢复剪切区域
		Handle->ClipRegionX = clipRegionXCpy;
		Handle->ClipRegionY = clipRegionYCpy;
		Handle->ClipRegionWidth = clipRegionWidthCpy;
		Handle->ClipRegionHeight = clipRegionHeightCpy;
	}
	if(pGlyph != 0){
		Handle->CursorX += pGlyph->Dwx0;
	}
}

//
// 简介：从光标处开始绘制字符串
// @参数：Handle - 显示器的句柄
// @参数：Str - 要绘制的字符串
// 
void PAL_OLED_DrawString(PalOled_HandleTypeDef *Handle, const char *Str)
{
	// 注意这里使用的是UTF-8编码，因此先将其解析成Unicode
	uint16_t i;
	uint32_t unicode;
	uint8_t first, second, third, forth;
	
	i=0;
	for(;;)
	{
		first = Str[i++];
		if(first == '\0') break; 
		
		if ((first & (1 << 7)) == 0) // 1字节
		{
			unicode = first;
			DrawCharator(Handle, unicode);
		}
		else if ((first & (1 << 7 | 1 << 6 | 1 << 5)) == (1 << 7 | 1 << 6)) // 2字
		{
			first = first & 0x1f;
			
			second = Str[i++];
			if(second == '\0' || ((second & (1 << 7 | 1 << 6)) != 0x80)) break;
			second = second & 0x3f;
			
			unicode = ((uint32_t)first << 6) | second;
			DrawCharator(Handle, unicode);
		}
		else if ((first & (1 << 7 | 1 << 6 | 1 << 5 | 1 << 4)) == (1 << 7 | 1 << 6 | 1 << 5)) // 3字节
		{
			first = first & 0x0f;
			
			second = Str[i++];
			if(second == '\0' || ((second & (1 << 7 | 1 << 6)) != 0x80)) break;
			second = second & 0x3f;
			
			third = Str[i++];
			if(third == '\0' || ((third & (1 << 7 | 1 << 6)) != 0x80)) break;
			third = third & 0x3f;
			
			unicode = ((uint32_t)first << 12) | ((uint32_t)second << 6) | third;
			DrawCharator(Handle, unicode);
		}
		else if ((first & (1 << 7 | 1 << 6 | 1 << 5 | 1 << 4 | 1 << 3)) == (1 << 7 | 1 << 6 | 1 << 5 | 1 << 4)) // 4字节
		{
			first = first & 0x07;
			
			second = Str[i++];
			if(second == '\0' || ((second & (1 << 7 | 1 << 6)) != 0x80)) break;
			second = second & 0x3f;
			
			third = Str[i++];
			if(third == '\0' || ((third & (1 << 7 | 1 << 6)) != 0x80)) break;
			third = third & 0x3f;
			
			forth = Str[i++];
			if(forth == '\0' || ((forth & (1 << 7 | 1 << 6)) != 0x80)) break;
			forth = forth & 0x3f;
			
			unicode = ((uint32_t)first << 18) | ((uint32_t)second << 12) | ((uint32_t)second << 6) | forth;
			
			DrawCharator(Handle, unicode);
		}
	}
}


//
// @简介：设置文本框区域，同时将光标移动到文本框的第一个字符处
// @参数：Handle - 显示器的句柄
// @参数：X - 文本框左上角的横坐标
// @参数：Y - 文本框左上角的纵坐标
// @参数：Width - 文本框的宽度
// @参数：Height - 文本框的高度
//
void PAL_OLED_StartTextRegion(PalOled_HandleTypeDef *Handle, int16_t X, int16_t Y, uint16_t Width, uint16_t Height)
{
	Handle->TextRegionX = X;
	Handle->TextRegionY = Y;
	Handle->TextRegionWidth = Width;
	Handle->TextRegionHeight = Height;
	
	Handle->CursorX = X;
	Handle->CursorY = Y + PAL_OLED_GetFontHeight(Handle);
}

//
// @简介：取消文本框
// @参数：Handle - 显示器的句柄
//
void PAL_OLED_StopTextRegion(PalOled_HandleTypeDef *Handle)
{
	Handle->TextRegionX = 0;
	Handle->TextRegionY = 0;
	Handle->TextRegionWidth = 0;
	Handle->TextRegionHeight = 0;
}

//
// 简介：绘制格式化字符串
// @参数：Handle - 显示器的句柄
// @参数：Format - 格式
// @参数：... - 可变参数
// 
void PAL_OLED_Printf(PalOled_HandleTypeDef *Handle, const char *Format, ...)
{
	char format_buffer[128];
	
	va_list argptr;
	__va_start(argptr, Format);
	vsprintf(format_buffer, Format, argptr);
	__va_end(argptr);
	PAL_OLED_DrawString(Handle, format_buffer);
}

//
// @简介：获取当前字体下字符串所占的宽度
// @参数：Handle - 显示器的句柄
// @参数：Str    - 字符串
// @返回值：宽度，单位：像素
// 
uint16_t PAL_OLED_GetStrWidth(PalOled_HandleTypeDef *Handle, const char *Str)
{
		// 注意这里使用的是UTF-8编码，因此先将其解析成Unicode
	uint16_t i;
	uint32_t unicode;
	uint8_t first, second, third, forth;
	uint16_t ret = 0;
	
	i=0;
	for(;;)
	{
		first = Str[i++];
		if(first == '\0') break; 
		
		if ((first & (1 << 7)) == 0) // 1字节
		{
			unicode = first;
			ret += GetGlyphWidth(Handle, unicode);
		}
		else if ((first & (1 << 7 | 1 << 6 | 1 << 5)) == (1 << 7 | 1 << 6)) // 2字
		{
			first = first & 0x1f;
			
			second = Str[i++];
			if(second == '\0' || ((second & (1 << 7 | 1 << 6)) != 0x80)) break;
			second = second & 0x3f;
			
			unicode = ((uint32_t)first << 6) | second;
			ret += GetGlyphWidth(Handle, unicode);
		}
		else if ((first & (1 << 7 | 1 << 6 | 1 << 5 | 1 << 4)) == (1 << 7 | 1 << 6 | 1 << 5)) // 3字节
		{
			first = first & 0x0f;
			
			second = Str[i++];
			if(second == '\0' || ((second & (1 << 7 | 1 << 6)) != 0x80)) break;
			second = second & 0x3f;
			
			third = Str[i++];
			if(third == '\0' || ((third & (1 << 7 | 1 << 6)) != 0x80)) break;
			third = third & 0x3f;
			
			unicode = ((uint32_t)first << 12) | ((uint32_t)second << 6) | third;
			ret += GetGlyphWidth(Handle, unicode);
		}
		else if ((first & (1 << 7 | 1 << 6 | 1 << 5 | 1 << 4 | 1 << 3)) == (1 << 7 | 1 << 6 | 1 << 5 | 1 << 4)) // 4字节
		{
			first = first & 0x07;
			
			second = Str[i++];
			if(second == '\0' || ((second & (1 << 7 | 1 << 6)) != 0x80)) break;
			second = second & 0x3f;
			
			third = Str[i++];
			if(third == '\0' || ((third & (1 << 7 | 1 << 6)) != 0x80)) break;
			third = third & 0x3f;
			
			forth = Str[i++];
			if(forth == '\0' || ((forth & (1 << 7 | 1 << 6)) != 0x80)) break;
			forth = forth & 0x3f;
			
			unicode = ((uint32_t)first << 18) | ((uint32_t)second << 12) | ((uint32_t)second << 6) | forth;
			
			ret += GetGlyphWidth(Handle, unicode);
		}
	}
	
	return ret;
}

// 
// @简介：获取当前字体的最大高度
// @参数：Handle - 显示器的句柄
// @返回值：字体最大高度
// 
uint16_t PAL_OLED_GetFontHeight(PalOled_HandleTypeDef *Handle)
{
	return Handle->Font->FontSize;
}

#define swap(x,y) do{(x) = (x) + (y); (y) = (x) - (y); (x) = (x) - (y); }while(0)

//
// @简介：画点
// @参数：Handle - OLED显示器的句柄	
// @参数：X - 横坐标
// @参数：Y - 纵坐标
//
void PAL_OLED_DrawDot(PalOled_HandleTypeDef *Handle)
{
	PenDot(Handle, Handle->CursorX, Handle->CursorY);
}	

//
// @简介：以当前光标位置为起点绘制直线
// @参数：Handle - OLED显示器的句柄	
// @参数：X - 终止点横坐标
// @参数：Y - 终止点纵坐标
// 
void PAL_OLED_DrawLine(PalOled_HandleTypeDef *Handle, int16_t X, int16_t Y)
{
	int16_t x, y;
	int16_t X0 = Handle->CursorX;
	int16_t Y0 = Handle->CursorY;
	int16_t X1 = X;
	int16_t Y1 = Y;
	
	if(Handle->PenColor == PEN_COLOR_TRANSPARENT) return; // 透明画笔不需要绘图
	
	if(X0 != X1)
	{
		if(X0 > X1)
		{
			swap(X0, X1);
			swap(Y0, Y1);
		}
		for(x=X0; x < X1; x++)
		{
			if(x < 0 || x >= OLED_SCREEN_COLS) continue;
			y = (int16_t)round(1.0 * (Y1 - Y0) * (x - X0) / (X1 - X0) + Y0);
			if(y < 0 || y >= OLED_SCREEN_ROWS) continue;
			
			PenDot(Handle, x, y);
		}
	}
	if(Y0 != Y1)
	{
		if(Y0 > Y1)
		{
			swap(X0, X1);
			swap(Y0, Y1);
		}
		for(y=Y0; y < Y1; y++)
		{
			if(y < 0 || y >= OLED_SCREEN_ROWS) continue;
			x = (int16_t)round(1.0 * (y - Y0) * (X1 - X0) / (Y1 - Y0) + X0);
			if(x < 0 || x >= OLED_SCREEN_COLS) continue;
			
			PenDot(Handle, x, y);
		}
	}
}

//
// @简介：以当前光标位置为起点绘制直线，绘制完成后光标移动到线段的终点
// @参数：Handle - OLED显示器的句柄	
// @参数：X - 终止点横坐标
// @参数：Y - 终止点纵坐标
// 
void PAL_OLED_LineTo(PalOled_HandleTypeDef *Handle, int16_t X, int16_t Y)
{
	PAL_OLED_DrawLine(Handle, X, Y);
	Handle->CursorX = X;
	Handle->CursorY = Y;
}

static void DrawCircleFrame(PalOled_HandleTypeDef *Handle, int16_t X, int16_t Y, uint16_t Radius)
{
	int16_t x, y, distance;
	
	if(Handle->PenColor == PEN_COLOR_TRANSPARENT) return; // 透明画笔不需要绘图

	if(X - Radius >= OLED_SCREEN_COLS || X + Radius < 0) return; // 绘图区域超出缓冲区范围
	if(Y - Radius >= OLED_SCREEN_ROWS || Y + Radius < 0) return; // 绘图区域超出缓冲区范围

	for(x=X-Radius; x<=X+Radius; x++)
	{
		if(x < 0 || x > OLED_SCREEN_COLS) continue; // x坐标超出缓冲区范围
		for(distance = 0; distance <= Radius; distance++)
		{
			if((x - X) * (x - X) + (distance + 1) * (distance + 1) > Radius * Radius) // x^2 + y ^ 2 > radius^2
			{
				if(Y+distance < OLED_SCREEN_ROWS && x >= 0 && x < OLED_SCREEN_COLS)
				{
					PenDot(Handle, x, Y+distance);
				}
				if(Y-distance > 0 && x >= 0 && x < OLED_SCREEN_COLS)
				{
					PenDot(Handle, x, Y-distance);
				}
				break;
			}
		}
	}

	for(y=Y-Radius; y<=Y+Radius; y++)
	{
		if(y < 0 || y > OLED_SCREEN_ROWS) continue;
		for(distance = 0; distance <= Radius; distance++)
		{
			if((y - Y) * (y - Y) + (distance + 1) * (distance + 1) > Radius * Radius)
			{
				if(X+distance < OLED_SCREEN_COLS && y >=0 && y < OLED_SCREEN_ROWS)
				{
					PenDot(Handle, X+distance, y);
				}
				if(X-distance > 0 && y >=0 && y < OLED_SCREEN_ROWS)
				{
					PenDot(Handle, X-distance, y);
				}
				break;
			}
		}
	}
}

static void FillCircle(PalOled_HandleTypeDef *Handle, int16_t X, int16_t Y, uint16_t Radius)
{
	int16_t x, distance;
	
	if(Handle->Brush == BRUSH_TRANSPARENT) return; // 透明画刷不需要绘图

	if(X - Radius >= OLED_SCREEN_COLS || X + Radius < 0) return;
	if(Y - Radius >= OLED_SCREEN_ROWS || Y + Radius < 0) return;

	for(x=X-Radius; x<=X+Radius; x++)
	{
		if(x < 0 || x > OLED_SCREEN_COLS) continue;
		for(distance = 0; distance <= Radius; distance++)
		{
			if((x - X) * (x - X) + distance*distance <= Radius * Radius)
			{
				if(Y+distance < OLED_SCREEN_ROWS)
				{
					BrushDot(Handle, x, Y+distance);
				}
				if(Y-distance > 0)
				{
					BrushDot(Handle, x, Y-distance);
				}
			}
			else
			{
				break;
			}
		}
	}
}

//
// @简介：以光标为圆心绘制圆形
// @参数：Handle - OLED显示器的句柄	
// @参数：X - 圆心横坐标
// @参数：Y - 圆心纵坐标
// @参数：Radius - 圆的半径
// 
void PAL_OLED_DrawCircle(PalOled_HandleTypeDef *Handle, uint16_t Radius)
{
	int16_t X = Handle->CursorX, Y = Handle->CursorY;
	
	if(Handle->PenColor != PEN_COLOR_TRANSPARENT)
	{
		DrawCircleFrame(Handle, X, Y, Radius);
	}
	
	if(Handle->Brush != BRUSH_TRANSPARENT)
	{
		FillCircle(Handle, X, Y, Radius);
	}
}

static void DrawRectFrame(PalOled_HandleTypeDef *Handle, int16_t X, int16_t Y, uint16_t Width, uint16_t Height)
{
	int16_t x,y;
	
	if(Handle->PenColor == PEN_COLOR_TRANSPARENT) return; // 透明画笔不需要绘图
	
	// 绘制左侧边
	x = X;
	if(x>=0 && x<OLED_SCREEN_COLS)
	{
		for(y=max(0,Y);y<Y+Height;y++)
		{
			PenDot(Handle, x, y);
		}
	}
	
	// 绘制右侧边
	x = X + Width -1;
	if(Width > 0 && x>=0 && x<OLED_SCREEN_COLS)
	{
		for(y=max(0,Y);y<Y+Height;y++)
		{
			PenDot(Handle, x, y);
		}
	}
	
	// 绘制上边
	y = Y;
	if(y>=0 && y<OLED_SCREEN_ROWS)
	{
		for(x=max(0,X);x<X+Width;x++)
		{
			PenDot(Handle, x, y);
		}
	}
	
	// 绘制下边
	y = Y + Height - 1;
	if(y>=0 && y<OLED_SCREEN_ROWS)
	{
		for(x=max(0,X);x<X+Width;x++)
		{
			PenDot(Handle, x, y);
		}
	}
}

static void FillRect(PalOled_HandleTypeDef *Handle, int16_t X, int16_t Y, uint16_t Width, uint16_t Height)
{
	if(Handle->Brush == BRUSH_TRANSPARENT) return; // 透明画刷不需要绘图
	
	int16_t x,y;
	
	for(x=X;x<X+Width;x++)
	{
		for(y=Y;y<Y+Height;y++)
		{
			BrushDot(Handle, x, y);
		}
	}
}

//
// @简介：以光标位置为左上角绘制矩形
// @参数：Handle - OLED显示器的句柄
// @参数：X - 矩形左上角的横坐标
// @参数：Y - 矩形左上角的纵坐标
// @参数：Width - 矩形宽度
// @参数：Height - 矩形高度
// 
void PAL_OLED_DrawRect(PalOled_HandleTypeDef *Handle, uint16_t Width, uint16_t Height)
{
	if(Handle->PenColor != PEN_COLOR_TRANSPARENT)
	{
		DrawRectFrame(Handle, Handle->CursorX, Handle->CursorY, Width, Height);
	}
	if(Handle->Brush != BRUSH_TRANSPARENT)
	{
		FillRect(Handle, Handle->CursorX, Handle->CursorY, Width, Height);
	}
}

//
// @简介：开始分段发送缓冲区内容到屏幕（每个8*8的区域为一个单元进行发送）
// @参数：Handle - OLED显示器的句柄
// @返回值：SUCCESS - 启动成功，ERROR - 启动失败
//
ErrorStatus PAL_OLED_StartSendBuffer(PalOled_HandleTypeDef *Handle)
{
	uint8_t arg[2];
	
	Handle->RefreshProgress = 0;
	
	// 设置寻址模式为横向寻址模式
	arg[0] = 0x00;
	CHECK(OLED_SendCommand(Handle, 0x20, arg, 1))

	// 设置列范围
	arg[0] = 0x00;
	arg[1] = 0x7f;
	CHECK(OLED_SendCommand(Handle, 0x21, arg, 2));

	// 设置页范围
	arg[0] = 0x00;
	arg[1] = 0x07;
	CHECK(OLED_SendCommand(Handle, 0x22, arg, 2));
	
	return SUCCESS;
}

//
// @简介：分段发送缓冲区内容到屏幕（每个8*8的区域为一个单元进行发送）
// @参数：Handle - OLED显示器的句柄
// @参数：pMoreOut - 输出参数，用于接收是否有后续数据需要发送
//                   0    - 当前分段为最后一个分段，至此所有分段均已发送结束
//                   非零 - 当前分段不是最后一个分段
// @返回值：SUCCESS - 成功，ERROR - 失败
//
ErrorStatus PAL_OLED_EndSendBuffer(PalOled_HandleTypeDef *Handle, uint8_t *pMoreOut)
{
	if(Handle->RefreshProgress >= 127)
	{
		*pMoreOut = 0;
	}
	else
	{
		*pMoreOut = 1;
	}
	
	if(Handle->RefreshProgress >= 128)
	{
		return ERROR;
	}
	
	// 更新显示数据
	CHECK(OLED_SendData(Handle, &Handle->pBuffer[Handle->RefreshProgress * 8], 8));
	
	Handle->RefreshProgress = (Handle->RefreshProgress + 1) % 128;
	
	return SUCCESS;
}

//
// @简介：将缓冲区数据一次性发送到屏幕
// @参数：Handle - OLED显示器的句柄
// @返回值：SUCCESS - 成功，ERROR - 失败
//
ErrorStatus PAL_OLED_SendBuffer(PalOled_HandleTypeDef *Handle)
{
	uint8_t arg[2];

	// 设置寻址模式为横向寻址模式
	arg[0] = 0x00;
	CHECK(OLED_SendCommand(Handle, 0x20, arg, 1))

	// 设置列范围
	arg[0] = 0x00;
	arg[1] = 0x7f;
	CHECK(OLED_SendCommand(Handle, 0x21, arg, 2));

	// 设置页范围
	arg[0] = 0x00;
	arg[1] = 0x07;
	CHECK(OLED_SendCommand(Handle, 0x22, arg, 2));
	
	// 更新显示数据
	CHECK(OLED_SendData(Handle, Handle->pBuffer, OLED_SCREEN_COLS * OLED_SCREEN_PAGES));
	
	return SUCCESS;
}

static ErrorStatus OLED_SendCommand(PalOled_HandleTypeDef *Handle, uint8_t Cmd, const uint8_t *Arg, uint16_t Size)
{
	uint8_t buf[8];
	uint8_t i;
	
	buf[0] = SSD1306_CTRL_COMMAND_STREAM;
	buf[1] = Cmd;
	
	for(i=0;i<Size;i++)
	{
		buf[i + 2] =  Arg[i];
	}
	
	CHECK(PAL_I2C_MasterTransmit(Handle->Init.hi2c, Handle->SlaveAddress, buf, i+2))
	
	return SUCCESS;
}

static ErrorStatus OLED_SendData(PalOled_HandleTypeDef *Handle, const uint8_t *pData, uint16_t Size)
{
	
	ErrorStatus ret = SUCCESS;
	
	if(PAL_I2C_StartTx(Handle->Init.hi2c, Handle->SlaveAddress) != SUCCESS)
	{
		ret = ERROR;
		goto TAG_SEND_STOP;
	}
	
	if(PAL_I2C_SendByte(Handle->Init.hi2c, SSD1306_CTRL_DATA_STREAM) != SUCCESS)
	{
		ret = ERROR;
		goto TAG_SEND_STOP;
	}
	
	if(PAL_I2C_SendBytes(Handle->Init.hi2c, pData, Size) != SUCCESS)
	{
		ret = ERROR;
		goto TAG_SEND_STOP;
	}
	
TAG_SEND_STOP:
	
	PAL_I2C_StopTx(Handle->Init.hi2c);
	
	return ret;
}

//
// @简介：绘制位图
// @参数：Handle - OLED显示器的句柄
// @参数：X - 位图左上角的横坐标
// @参数：Y - 位图左上角的纵坐标
// @参数：Width - 位图宽度
// @参数：Height - 位图高度
// @参数：pBitmap - 位图数据
//
static void DrawBitmapEx(PalOled_HandleTypeDef *Handle,int16_t X, int16_t Y, uint16_t Width, uint16_t Height, const uint8_t *pBitmap)
{
	int16_t x,y;
	
	if(Handle->Brush == BRUSH_TRANSPARENT && Handle->PenColor == PEN_COLOR_TRANSPARENT) return; // 透明画刷不做任何操作
	
	uint16_t penWidthCpy = Handle->PenWidth;
	
	Handle->PenWidth = 1;
	
	uint16_t nBytesPerRow = (uint16_t)ceil(Width / 8.0);
	
	for(x=0;x<Width;x++)
	{
		for(y=0;y<Height;y++)
		{
			// b0 b1 .. b7 b0 b1 
			if((pBitmap[x/8+y*nBytesPerRow] & (0x80>>(x%8))) != 0)
			{
				PenDot(Handle, X+x, Y+y);
			}
			else
			{
				BrushDot(Handle, X+x, Y+y);
			}
		}
	}
	
	Handle->PenWidth = penWidthCpy;
}

//
// @简介：以光标位置为左上角绘制位图
// @参数：X - 位图左上角横坐标
// @参数：Y - 位图左上角纵坐标
// @参数：Width - 位图宽度
// @参数：Height - 位图高度
// @参数：pBitmap - 位图数据，格式：每个字节表示横向的8个像素点，行末尾不足8个像素的用0补齐
//
void PAL_OLED_DrawBitmap(PalOled_HandleTypeDef *Handle, uint16_t Width, uint16_t Height, const uint8_t *pBitmap)
{
	DrawBitmapEx(Handle, Handle->CursorX, Handle->CursorY, Width, Height, pBitmap);
}

// 
// @简介：根据字符的unicode码获取字形序号
// @参数：Handle - OLED显示器句柄
// @参数：Unicode - 字符的Unicode编码
// @返回值：如果查找成功则返回字符序号，否则返回-1
//
static int16_t unicode_2_glyph_idx(PalOled_HandleTypeDef *Handle, uint32_t Unicode)
{
	int16_t ret = -1;
	
	if(Handle->Font == 0)
	{
		return -1;
	}
	
	uint32_t i;
	
	for(i = 0; i < Handle->Font->nChars; i++)
	{
		if(Handle->Font->Map[i] == Unicode)
		{
			ret = i;
			break;
		}
	}
	
	return ret;
}

static void PenDot(PalOled_HandleTypeDef *Handle, int16_t x, int16_t y)
{
	// 判断是否是透明画笔
	if(Handle->PenColor == PEN_COLOR_TRANSPARENT)
	{
		return; // 透明画笔不需要绘图
	}
	
	if(Handle->PenWidth == 0)
	{
		return;
	}
	
	uint16_t borderLeft, borderRight, borderTop, borderBottom;
	
	if(Handle->PenWidth % 2)
	{
		borderLeft = borderRight = borderTop = borderBottom = Handle->PenWidth / 2;
	}
	else
	{
		borderLeft = borderRight = borderTop = borderBottom = Handle->PenWidth / 2;
		borderLeft--;
		borderTop--;
	}
	
	// 绘图
	// 备份画刷
	uint8_t brushCpy = Handle->Brush;
	
	Handle->Brush = Handle->PenColor;
	
	int16_t i,j;
	
	for(i=x-borderLeft;i<=x+borderRight;i++)
	{
		for(j=y-borderTop;j<=y+borderBottom;j++)
		{
			BrushDot(Handle, i, j);
		}
	}
	
	// 还原画刷
	Handle->Brush = brushCpy;
}

static void BrushDot(PalOled_HandleTypeDef *Handle, int16_t x, int16_t y)
{
	// 判断是否是透明画笔
	if(Handle->Brush == BRUSH_TRANSPARENT)
	{
		return; // 透明画笔不需要绘图
	}
	
	// 判断绘图点是否在屏幕外部
	if(x < 0 || x >= OLED_SCREEN_COLS)
	{
		return;
	}
	
	if(y<0 || y >= OLED_SCREEN_ROWS)
	{
		return;
	}
	
	// 判断是否在绘图区域的外部
	if(Handle->ClipRegionWidth != 0 && Handle->ClipRegionHeight != 0)
	{
		if(x<Handle->ClipRegionX || x>=Handle->ClipRegionX + Handle->ClipRegionWidth)
		{
			return;
		}
		if(y<Handle->ClipRegionY || y>=Handle->ClipRegionY + Handle->ClipRegionHeight)
		{
			return;
		}
	}
	
	// 绘图
	if(Handle->Brush == BRUSH_WHITE) // 点亮
	{
		Handle->pBuffer[x + y / 8 * OLED_SCREEN_COLS] |= 1 << (y % 8);
	}
	else // 熄灭
	{
		Handle->pBuffer[x + y / 8 * OLED_SCREEN_COLS] &= ~(1 << (y % 8));
	}
}
