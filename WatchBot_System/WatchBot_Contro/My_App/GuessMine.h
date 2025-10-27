#ifndef _GUESSMINE_H
#define _GUESSMINE_H

#include "stm32f10x.h"           


//雷数量
#define MINE_COUNT 10

//旗子数量
#define MINE_FLAG_COUNT 10;

//光标尺寸
#define WIDTH  6
#define HEIGHT 8

//雷区长度
#define ROW 8
#define COL 21

//棋盘长度
#define ROWS 23
#define COLS 10

typedef struct
{
	uint8_t MineField[ROWS][COLS];//整个棋盘
	uint8_t MineField_Flag[ROWS][COLS];//为1表示格子已经翻开

	//光标映射在屏幕上的实际长宽
	uint8_t Mine_Cursor_X;
	uint8_t Mine_Cursor_Y;

	//光标映射在棋盘的实际位置
	uint8_t MineBoard_X;
	uint8_t MineBoard_Y;

 uint16_t Opened_Count;//打开格子数量
 uint16_t Opened_Flag;//插旗正确数量
 uint16_t Closed_Flag;//剩余旗子数量
	}GuessMine_Statu_t;

void MineBoard_Init(void);//布置雷初始化函数
void GuessMine_Proc(void);//非阻塞进程函数

#endif
