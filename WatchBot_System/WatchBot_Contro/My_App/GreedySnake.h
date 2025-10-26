#ifndef _GREEDY_SNAKE_H
#define _GREEDY_SNAKE_H

#include "stm32f10x.h"            

#define MAX_LEN 210 + 1 		//蛇身最大长度
#define SNAKE_WIN_LENGTH 10 //胜利条件的蛇身长度
#define SNAKE_ROWS_MAX 10 	// 0 - 9 	Y方向
#define SNAKE_COLS_MAX 21		// 0 - 20 X方向

#define SNAKE_SPEED 300 //贪吃蛇移动速度

//游戏状态
typedef enum 
{
	SNAKE_OK,       //活
	SNAKE_KILL_WALL,//撞墙
	SNAKE_KILL_SELF,//自杀
	SNAKE_SB,				//输
	SNAKE_NB,				//赢
	SNAKE_ON				//结束？
}GAME_STATUS;

//当前按键
typedef enum  
{
	W = 13, //对应红外遥控的方向键
	S = 14,
	A = 15,
	D = 16
}DIRECTION;

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
	SnakeFood Snake_Food[1];				//食物位置
	uint32_t Snake_SleepTimel;			//走一步休眠时间
	GAME_STATUS Snake_Status;				//游戏状态
	DIRECTION Direction;						//游戏当前方向
	
}Snake;

//贪吃蛇初始化
void SnakeGame_Init(void);

//贪吃蛇任务进程
void GreedySnake_Proc(void);

#endif
