#ifndef _GREEDY_SNAKE_H
#define _GREEDY_SNAKE_H

#include "stm32f10x.h"            


#define MAX_LEN 210 + 1 		//蛇身最大长度
#define SNAKE_WIN_LENGTH 10 //胜利条件的蛇身长度
#define SNAKE_ROWS_MAX 10 	// 0 - 9 	Y方向
#define SNAKE_COLS_MAX 21		// 0 - 20 X方向

//游戏状态
enum GAME_STATUS
{
	SNAKE_OK,
	SNAKE_KILL_WALL,
	SNAKE_KILL_SELF,
	SNAKE_SB,
	SNAKE_NB,
	SNAKE_ON
};

//当前按键
enum DIRECTION
{
	W = 1,
	S,
	A,
	D
};

//食物坐标
typedef struct SnakeFood
{
	uint16_t Food_x;
	uint16_t Food_y;
}SnakeFood;

//蛇身节点
typedef struct SnakeNode
{
	uint16_t x;
	uint16_t y;
}SnakeNode;

typedef struct Snake
{
	SnakeNode Snake_Body[MAX_LEN];	//存储蛇身
	uint16_t Snake_Head;						//头在数组中的位置
	uint16_t Snake_Tail;						//尾在数组中的位置
	uint16_t Snake_Size;						//当前长度
	SnakeFood Snake_Food[1];					//食物位置
	uint32_t Snake_SleepTimel;			//走一步休眠时间
	enum GAME_STATUS Snake_Status;	//游戏状态
	enum DIRECTION Direction;				//游戏当前方向
	
}Snake;

extern uint8_t SnakeState; 
extern uint8_t SnakeState_Flag;

//贪吃蛇初始化
extern void SnakeGame_Init(void);

extern void GreedySnake_Proc(void);

#endif
