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


extern uint8_t GuessMine_Flag;

void Mine_CursorShow(void);//棋盘边框打印 调用一次即可
void GuessMine_Proc(void);//非阻塞进程函数
void MineBoard_Init(void);//布置雷初始化函数

#endif
