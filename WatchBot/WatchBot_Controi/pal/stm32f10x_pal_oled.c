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

// 命令字节
#define SSD1306_CTRL_COMMAND           0x80  // Continuation bit=1, D/C=0; 1000 0000
#define SSD1306_CTRL_COMMAND_STREAM    0x00  // Continuation bit=0, D/C=0; 0000 0000
#define SSD1306_CTRL_DATA              0xc0  // Continuation bit=1, D/C=1; 1100 0000
#define SSD1306_CTRL_DATA_STREAM       0x40  // Continuation bit=0, D/C=1; 0100 0000

static ErrorStatus OLED_SendCommand(PalOled_HandleTypeDef *Handle, const uint8_t Cmd, const uint8_t *Arg, uint16_t Size);
static ErrorStatus OLED_SendData(PalOled_HandleTypeDef *Handle, const uint8_t *pData, uint16_t Size);
static void DrawCircleFrame(PalOled_HandleTypeDef *Handle, int16_t X, int16_t Y, uint16_t Radius);
static void FillCircle(PalOled_HandleTypeDef *Handle, int16_t X, int16_t Y, uint16_t Radius);
static void DrawRectFrame(PalOled_HandleTypeDef *Handle, int16_t X, int16_t Y, uint16_t Width, uint16_t Height);
static void FillRect(PalOled_HandleTypeDef *Handle, int16_t X, int16_t Y, uint16_t Width, uint16_t Height);

ErrorStatus PAL_OLED_Init(PalOled_HandleTypeDef *Handle)
{
	// 计算从机地址
	// SA0 = L -> 0x78
	// SA0 = H -> 0x7a
	if(Handle->Init.Advanced.SA0 == RESET)
	{
		Handle->SlaveAddress = 0x78;
	}
	else
	{
		Handle->SlaveAddress = 0x7a;
	}
	
	// 给缓冲区A分配空间
	Handle->pBufferA = (uint8_t *)malloc(OLED_SCREEN_COLS * OLED_SCREEN_PAGES * sizeof(uint8_t));
	
	Handle->UseBufferA = 1; // 先使用BufferA
	
	if(!Handle->pBufferA)
	{
		return ERROR;
	}
	
	memset(Handle->pBufferA, 0, OLED_SCREEN_COLS * OLED_SCREEN_PAGES * sizeof(uint8_t));
	
	// 如果需要使用DMA
	if(Handle->Init.Advanced.UseDMA == SET) 
	{
		// DMA发送需要使用双缓冲区
		// 分配缓冲区B
		Handle->pBufferB = (uint8_t *)malloc(OLED_SCREEN_COLS * OLED_SCREEN_PAGES * sizeof(uint8_t));
		
		if(!Handle->pBufferB)
		{
			return ERROR;
		}
		
		memset(Handle->pBufferB, 0, OLED_SCREEN_COLS * OLED_SCREEN_PAGES * sizeof(uint8_t));
	}
	
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
	
	Handle->PenColor = OLED_COLOR_WHITE; // 默认白色画笔
	Handle->PenWidth = 1; // 默认线宽为1
	Handle->Brush = OLED_COLOR_WHITE; // 默认白色画刷
	
	Handle->CursorX = 0;
	Handle->CursorY = 0; // 光标默认在屏幕的左上角
	
	return SUCCESS;
}

void PAL_OLED_Clear(PalOled_HandleTypeDef *Handle)
{
	uint8_t *pBuffer = Handle->UseBufferA ? Handle->pBufferA : Handle->pBufferB;
	
	memset(pBuffer, 0, OLED_SCREEN_COLS * OLED_SCREEN_PAGES);
}

void PAL_OLED_SetFont(PalOled_HandleTypeDef *Handle, const uint8_t *Font)
{
	Handle->Font = Font;
}

void PAL_OLED_SetPen(PalOled_HandleTypeDef *Handle, uint8_t Pen_Color, uint8_t Width)
{
	Handle->PenColor = Pen_Color;
	Handle->PenWidth = Width;
}

void PAL_OLED_SetBrush(PalOled_HandleTypeDef *Handle, uint8_t Brush_Color)
{
	Handle->Brush = Brush_Color;
}

void PAL_OLED_MoveCursor(PalOled_HandleTypeDef *Handle, int16_t X, int16_t Y)
{
	Handle->CursorX = X;
	Handle->CursorY = Y;
}

void PAL_OLED_MoveCursorX(PalOled_HandleTypeDef *Handle, int16_t X)
{
	Handle->CursorX = X;
}

void PAL_OLED_MoveCursorY(PalOled_HandleTypeDef *Handle, int16_t Y)
{
	Handle->CursorY = Y;
}

void PAL_OLED_OffsetCursor(PalOled_HandleTypeDef *Handle, int16_t OffsetX, int16_t OffsetY)
{
	Handle->CursorX += OffsetX;
	Handle->CursorY += OffsetY;
}

void PAL_OLED_GetCursor(PalOled_HandleTypeDef *Handle, int16_t *X, int16_t *Y)
{
	*X = Handle->CursorX;
	*Y = Handle->CursorY;
}

int16_t PAL_OLED_GetCursorX(PalOled_HandleTypeDef *Handle)
{
	return Handle->CursorX;
}

int16_t PAL_OLED_GetCursorY(PalOled_HandleTypeDef *Handle)
{
	return Handle->CursorY;
}

void PAL_OLED_DrawCharator(PalOled_HandleTypeDef *Handle, char Ch)
{
	int16_t col;
	uint8_t *pBuffer = Handle->UseBufferA ? Handle->pBufferA : Handle->pBufferB;
	
	if(Handle->Font == NULL) return;
	
	if(Handle->Font == font_5x8_matrix)
	{
		// 字符超出边界
		if(Handle->PenColor == OLED_COLOR_TRANSPARENT || Handle->CursorX < -5 || Handle->CursorX > 127 || Handle->CursorY < -8 || Handle->CursorY > 63)
		{
		}
		else
		{
			if(Ch<0x20||Ch >0x7f)
			{
				Ch = 0x20;
			}
			
			const uint8_t *ch_data = &font_5x8_matrix[(Ch - 0x20) * 5];
			
			for(col = Handle->CursorX; col < Handle->CursorX + 5; col++)
			{
				if(col < 0 || col >= OLED_SCREEN_COLS) continue;
				if(Handle->CursorY % 8 == 0) // 正好在页边界上
				{
					pBuffer[col + Handle->CursorY/8*OLED_SCREEN_COLS] |= ch_data[col - Handle->CursorX];
				}
				else
				{
					if(Handle->CursorY > 0)
					{
						if(Handle->PenColor == OLED_COLOR_WHITE)
						{
							pBuffer[col + Handle->CursorY/8*OLED_SCREEN_COLS] |= ch_data[col - Handle->CursorX] << (Handle->CursorY % 8);
						}
						else if(Handle->PenColor == OLED_COLOR_BLACK)
						{
							pBuffer[col + Handle->CursorY/8*OLED_SCREEN_COLS] &= ~(ch_data[col - Handle->CursorX] << (Handle->CursorY % 8));
						}
					}
					if(Handle->CursorY < (OLED_SCREEN_PAGES - 1) * 8)
					{
						if(Handle->PenColor == OLED_COLOR_WHITE)
						{
							pBuffer[col + (Handle->CursorY/8 + 1)*OLED_SCREEN_COLS] |= ch_data[col - Handle->CursorX] >> (8 - (Handle->CursorY % 8));
						}
						else if(Handle->PenColor == OLED_COLOR_BLACK)
						{
							pBuffer[col + (Handle->CursorY/8 + 1)*OLED_SCREEN_COLS] &= ~(ch_data[col - Handle->CursorX] >> (8 - (Handle->CursorY % 8)));
						}
					}
				}
			}
		}
		Handle->CursorX += 5;
	}
}

void PAL_OLED_DrawString(PalOled_HandleTypeDef *Handle, const char *Str)
{
	uint16_t i;
	
	for(i=0;i<strlen(Str);i++)
	{
		PAL_OLED_DrawCharator(Handle, Str[i]);
	}
}

void PAL_OLED_Printf(PalOled_HandleTypeDef *Handle, const char *Format, ...)
{
	char format_buffer[128];
	
	va_list argptr;
	__va_start(argptr, Format);
	vsprintf(format_buffer, Format, argptr);
	__va_end(argptr);
	PAL_OLED_DrawString(Handle, format_buffer);
}

uint16_t PAL_OLED_GetFontHeight(const uint8_t *Font)
{
	return 8;
}

uint16_t PAL_OLED_GetFontMaxWidth(const uint8_t *Font)
{
	return 5;
}

uint16_t PAL_OLED_GetChWidth(PalOled_HandleTypeDef *Handle, char Ch)
{
	return 5;
}

uint16_t PAL_OLED_GetStrWidth(PalOled_HandleTypeDef *Handle, const char *Str)
{
	return strlen(Str) * 5;
}

uint16_t PAL_OLED_GetStrHeight(PalOled_HandleTypeDef *Handle, const char *Str)
{
	return 8;
}

#define swap(x,y) do{(x) = (x) + (y); (y) = (x) - (y); (x) = (x) - (y); }while(0)

void PAL_OLED_DrawLine(PalOled_HandleTypeDef *Handle, int16_t Start_x, int16_t Start_y, int16_t End_x, int16_t End_y)
{
	int16_t x, y;
	uint8_t *pBuffer = Handle->UseBufferA ? Handle->pBufferA : Handle->pBufferB;
	
	if(Start_x > End_x)
	{
		swap(Start_x, End_x);
	}
	
	if(Start_y > End_y)
	{
		swap(Start_y, End_y);
	}
	
	if(Start_x != End_x)
	{
		for(x=Start_x; x < End_x; x++)
		{
			if(x < 0 || x >= OLED_SCREEN_COLS) continue;
			y = (End_y - Start_y) * (x - Start_x) / (End_x - Start_x) + Start_y;
			if(y < 0 || y >= OLED_SCREEN_ROWS) continue;
			if(Handle->PenColor == OLED_COLOR_WHITE)
			{
				pBuffer[x + y/8*OLED_SCREEN_COLS] |= 0x01 << (y%8);
			}
			else if(Handle->PenColor == OLED_COLOR_BLACK)
			{
				pBuffer[x + y/8*OLED_SCREEN_COLS] &= ~(0x01 << (y%8));
			}
		}
	}
	if(Start_y != End_y)
	{
		for(y=Start_y; y < End_y; y++)
		{
			if(y < 0 || y >= OLED_SCREEN_ROWS) continue;
			x = (y - Start_y) * (End_x - Start_x) / (End_y - Start_y) + Start_x;
			if(x < 0 || x >= OLED_SCREEN_COLS) continue;
			if(Handle->PenColor == OLED_COLOR_WHITE)
			{
				pBuffer[x + y/8*OLED_SCREEN_COLS] |= 0x01 << (y%8);
			}
			else if(Handle->PenColor == OLED_COLOR_BLACK)
			{
				pBuffer[x + y/8*OLED_SCREEN_COLS] &= ~(0x01 << (y%8));
			}
		}
	}
}

static void DrawCircleFrame(PalOled_HandleTypeDef *Handle, int16_t X, int16_t Y, uint16_t Radius)
{
	int16_t x, y, distance;
	uint8_t *pBuffer = Handle->UseBufferA ? Handle->pBufferA : Handle->pBufferB;

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
					if(Handle->PenColor == OLED_COLOR_WHITE)
					{
						pBuffer[x + (Y+distance) / 8 * OLED_SCREEN_COLS] |= 0x01 << ((Y+distance) %8);
					}
					else if(Handle->PenColor == OLED_COLOR_BLACK)
					{
						pBuffer[x + (Y+distance) / 8 * OLED_SCREEN_COLS] &= ~(0x01 << ((Y+distance) %8));
					}
				}
				if(Y-distance > 0 && x >= 0 && x < OLED_SCREEN_COLS)
				{
					if(Handle->PenColor == OLED_COLOR_WHITE)
					{
						pBuffer[x + (Y-distance) / 8 * OLED_SCREEN_COLS] |= 0x01 << ((Y-distance) %8);
					}
					else if(Handle->PenColor == OLED_COLOR_BLACK)
					{
						pBuffer[x + (Y-distance) / 8 * OLED_SCREEN_COLS] &= ~(0x01 << ((Y-distance) %8));
					}
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
					if(Handle->PenColor == OLED_COLOR_WHITE)
					{
						pBuffer[(X + distance) + y / 8 * OLED_SCREEN_COLS] |= 0x01 << (y %8);
					}
					else if(Handle->PenColor == OLED_COLOR_BLACK)
					{
						pBuffer[(X + distance) + y / 8 * OLED_SCREEN_COLS] &= ~(0x01 << (y %8));
					}
				}
				if(X-distance > 0 && y >=0 && y < OLED_SCREEN_ROWS)
				{
					if(Handle->PenColor == OLED_COLOR_WHITE)
					{
						pBuffer[(X - distance) + y / 8 * OLED_SCREEN_COLS] |= 0x01 << (y %8);
					}
					else if(Handle->PenColor == OLED_COLOR_BLACK)
					{
						pBuffer[(X - distance) + y / 8 * OLED_SCREEN_COLS] &= ~(0x01 << (y %8));
					}
				}
				break;
			}
		}
	}
}

static void FillCircle(PalOled_HandleTypeDef *Handle, int16_t X, int16_t Y, uint16_t Radius)
{
	int16_t x, distance;
	uint8_t *pBuffer = Handle->UseBufferA ? Handle->pBufferA : Handle->pBufferB;

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
					if(Handle->Brush == OLED_COLOR_WHITE)
					{
						pBuffer[x + (Y+distance) / 8 * OLED_SCREEN_COLS] |= 0x01 << ((Y+distance) %8);
					}
					else if(Handle->Brush == OLED_COLOR_BLACK)
					{
						pBuffer[x + (Y+distance) / 8 * OLED_SCREEN_COLS] &= ~(0x01 << ((Y+distance) %8));
					}
				}
				if(Y-distance > 0)
				{
					if(Handle->Brush == OLED_COLOR_WHITE)
					{
						pBuffer[x + (Y-distance) / 8 * OLED_SCREEN_COLS] |= 0x01 << ((Y-distance) %8);
					}
					else if(Handle->Brush == OLED_COLOR_BLACK)
					{
						pBuffer[x + (Y-distance) / 8 * OLED_SCREEN_COLS] &= ~(0x01 << ((Y-distance) %8));
					}
				}
			}
			else
			{
				break;
			}
		}
	}
}

void PAL_OLED_DrawCircle(PalOled_HandleTypeDef *Handle, int16_t X, int16_t Y, uint16_t Radius)
{
	if(Handle->PenColor != OLED_COLOR_TRANSPARENT)
	{
		DrawCircleFrame(Handle, X, Y, Radius);
	}
	
	if(Handle->Brush != OLED_COLOR_TRANSPARENT)
	{
		FillCircle(Handle, X, Y, Radius);
	}
}

static void DrawRectFrame(PalOled_HandleTypeDef *Handle, int16_t X, int16_t Y, uint16_t Width, uint16_t Height)
{
	PAL_OLED_DrawLine(Handle, X, Y, X+Width, Y);

	PAL_OLED_DrawLine(Handle, X, Y, X, Y + Height);

	PAL_OLED_DrawLine(Handle, X + Width, Y, X+Width, Y + Height);

	PAL_OLED_DrawLine(Handle, X, Y + Height, X + Width, Y + Height);
}

static void FillRect(PalOled_HandleTypeDef *Handle, int16_t X, int16_t Y, uint16_t Width, uint16_t Height)
{
	int16_t page,column;
	uint8_t *pBuffer = Handle->UseBufferA ? Handle->pBufferA : Handle->pBufferB;

	page = Y/8;

	if(Y%8 != 0)
	{
		if(page < OLED_SCREEN_PAGES && page >= 0)
		{
			for(column = X; column < X + Width; column++)
			{
				if(column < 0 || column >= OLED_SCREEN_COLS) continue;
				if(Handle->Brush == OLED_COLOR_WHITE)
				{
					pBuffer[Y/8*OLED_SCREEN_COLS + column] |= 0xff << (Y%8);
				}
				else if(Handle->Brush == OLED_COLOR_BLACK)
				{
					pBuffer[Y/8*OLED_SCREEN_COLS + column] &= ~(0xff << (Y%8));
				}
			}
			page++;
		}
	}

	for(; page < (Y+Height)/8; page++)
	{
		if(page >= OLED_SCREEN_PAGES || page < 0) continue;
		for(column = X; column < X + Width; column++)
		{
			if(column < 0 || column >= OLED_SCREEN_COLS) continue;
			if(Handle->Brush == OLED_COLOR_WHITE)
			{
				pBuffer[page*128 + column] = 0xff;
			}
			else if(Handle->Brush == OLED_COLOR_BLACK)
			{
				pBuffer[page*128 + column] = 0x00;
			}
		}
	}

	if((Y+Height)%8 != 0 && Y+Height > 0 && Y+Height < OLED_SCREEN_ROWS)
	{
		for(column = X; column < X + Width; column++)
		{
			if(column < 0 || column >= OLED_SCREEN_COLS) continue;
			if(Handle->Brush == OLED_COLOR_WHITE)
			{
				pBuffer[(Y+Height)/8*OLED_SCREEN_COLS + column] |= 0xff >> (8-Y%8);
			}
			else if(Handle->Brush == OLED_COLOR_BLACK)
			{
				pBuffer[(Y+Height)/8*OLED_SCREEN_COLS + column] &= ~(0xff >> (8-Y%8));
			}
		}
	}
}

void PAL_OLED_DrawRect(PalOled_HandleTypeDef *Handle, int16_t X, int16_t Y, uint16_t Width, uint16_t Height)
{
	if(Handle->PenColor != OLED_COLOR_TRANSPARENT)
	{
		DrawRectFrame(Handle, X, Y, Width, Height);
	}
	if(Handle->Brush != OLED_COLOR_TRANSPARENT)
	{
		FillRect(Handle, X, Y, Width, Height);
	}
}

ErrorStatus PAL_OLED_SendBuffer(PalOled_HandleTypeDef *Handle)
{
	uint8_t arg[2];
	const uint8_t *pBufferToSend = Handle->pBufferA; // 非DMA方式下只使用BufferA

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
	CHECK(OLED_SendData(Handle, pBufferToSend, OLED_SCREEN_COLS * OLED_SCREEN_PAGES));
	
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

static const uint8_t send_data_prifix = SSD1306_CTRL_DATA_STREAM;

static ErrorStatus OLED_SendData(PalOled_HandleTypeDef *Handle, const uint8_t *pData, uint16_t Size)
{
	Handle->Init.hi2c->Init.Advanced.SendPrefix = SET;
	Handle->Init.hi2c->Init.Advanced.pPrefix = &send_data_prifix;
	Handle->Init.hi2c->Init.Advanced.PrefixSize = 1;
	
	if(Handle->Init.Advanced.UseDMA == RESET || Size < 128)
	{
		CHECK(PAL_I2C_MasterTransmit(Handle->Init.hi2c, Handle->SlaveAddress, pData, Size))
	}
	else
	{
		uint8_t *pBuffer = Handle->UseBufferA ? Handle->pBufferA : Handle->pBufferB;
		Handle->UseBufferA = Handle->UseBufferA ? 0 : 1;
		CHECK(PAL_I2C_MasterTransmit_DMA(Handle->Init.hi2c, Handle->SlaveAddress, pBuffer, Size))
	}
	
	Handle->Init.hi2c->Init.Advanced.SendPrefix = RESET;
	
	return SUCCESS;
}

void PAL_OLED_DrawBitmap(PalOled_HandleTypeDef *Handle, int16_t X, int16_t Y, uint16_t Width, uint16_t Height, const uint8_t *pBitmap)
{
	int16_t i,j,x,y;
	uint8_t *pBuffer = Handle->UseBufferA ? Handle->pBufferA : Handle->pBufferB;
	
	if(Handle->Brush == OLED_COLOR_TRANSPARENT) return; // 透明画刷不做任何操作

	for(i=0;i<Width;i++)
	{
		x = X + i;
		if(x < 0 || x > OLED_SCREEN_COLS) continue;

		for(j=0;j<Height/8;j++)
		{
			y = Y + j * 8;
			if(y%8==0)
			{
				if(Handle->Brush == OLED_COLOR_WHITE){
					pBuffer[y/8*OLED_SCREEN_COLS + x] |= pBitmap[j*Width + i];
				}
				else{
					pBuffer[y/8*OLED_SCREEN_COLS + x] &= ~(pBitmap[j*Width + i]);
				}
			}
			else
			{
				if(y/8 > 0 && y/8 < OLED_SCREEN_PAGES)
				{
					if(Handle->Brush == OLED_COLOR_WHITE){
						pBuffer[y/8*OLED_SCREEN_COLS + x] |= pBitmap[j*Width + i] << (y%8);
					}
					else{
						pBuffer[y/8*OLED_SCREEN_COLS + x] &= ~(pBitmap[j*Width + i] << (y%8));
					}
				}
				if(y/8 + 1 > 0 && y/8 + 1 < OLED_SCREEN_PAGES)
				{
					if(Handle->Brush == OLED_COLOR_WHITE){
						pBuffer[(y/8+1)*OLED_SCREEN_COLS + x] |= pBitmap[j*Width + i] >> (8-y%8);
					}
					else{
						pBuffer[(y/8+1)*OLED_SCREEN_COLS + x] &= ~(pBitmap[j*Width + i] >> (8-y%8));
					}
				}
			}
		}
	}
}
