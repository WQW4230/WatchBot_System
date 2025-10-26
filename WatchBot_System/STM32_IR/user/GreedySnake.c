#include "GreedySnake.h"
#include "OLED.h"
#include "remote_nec_driver.h"
#include "menu.h"
#include "App_Timer.h"
#include <string.h>

//声明结构体
static Snake Snake_P1;

uint8_t SnakeState_Flag = 0;
//static uint16_t x = 0;
//static uint16_t y = 0;

//棋盘宽度
void CreateMap(void)
{
		OLED_DrawRectangle(0, 0, 128, 2, OLED_FILLED);
		OLED_DrawRectangle(0, 62, 128, 2, OLED_FILLED);
	
		OLED_DrawRectangle(0, 0, 1, 64, OLED_FILLED);
		OLED_DrawRectangle(127, 0, 1, 64, OLED_FILLED);

		OLED_Update();
}

//初始化
void SnakeInit(Snake* s, uint16_t StartX, uint16_t StartY)
{
	s->Snake_Head = 0;
	s->Snake_Tail = 0;
	s->Snake_Size = 1;
	s->Snake_Body[0].x = StartX;
	s->Snake_Body[0].y = StartY;
}

//入队
void Snake_PushHead(Snake *s, uint16_t StartX, uint16_t StartY)
{
	s->Snake_Head = (s->Snake_Head + 1) % MAX_LEN;
	s->Snake_Body[s->Snake_Head].x = StartX;
	s->Snake_Body[s->Snake_Head].y = StartY;
	s->Snake_Size++;
	
}

//出队
void Snake_DeleteTail(Snake *s)
{
	s->Snake_Tail = (s->Snake_Tail + 1) % MAX_LEN;
	s->Snake_Size--;
}
 
//返回值如果为1证明死亡，并置位状态标志位
//返回值如果为0证明活着，状态标志位为OK
//死亡判定函数
uint8_t Snake_IsDead(Snake *s)
{
	uint16_t Temp_X = s->Snake_Body[s->Snake_Head].x;
	uint16_t Temp_Y = s->Snake_Body[s->Snake_Head].y;
	//如果撞墙
	if(Temp_X >= SNAKE_COLS_MAX || Temp_Y >= SNAKE_ROWS_MAX)
	{
		s->Snake_Status = SNAKE_KILL_WALL;
		return 1;
	}
	//如果撞自己
	for(uint16_t i = s->Snake_Tail; i != s->Snake_Head; i = (i + 1) % MAX_LEN)
	{
		if( (s->Snake_Body[i].x == Temp_X) && (s->Snake_Body[i].y == Temp_Y))
		{
			s->Snake_Status = SNAKE_KILL_SELF;
			return 1;
		}
	}
	s->Snake_Status = SNAKE_OK;
	return 0;
}

//创建食物函数
void Snake_GenerateFood(Snake *s)
{
	uint8_t Food_Flag = 1;
	uint16_t Temp_X = 0;
	uint16_t Temp_Y = 0;
	while(Food_Flag)
	{
		Temp_X = GetRandom(SNAKE_COLS_MAX);
		Temp_Y  = GetRandom(SNAKE_ROWS_MAX);
		for(uint16_t i = s->Snake_Tail; i != (s->Snake_Head + 1) % MAX_LEN; i = (i + 1) % MAX_LEN)
		{
			if( (s->Snake_Body[i].x == Temp_X) && (s->Snake_Body[i].y == Temp_Y))
			{
				//食物与蛇身冲突重新生成
				Food_Flag = 1;
				break;
			}
			else
			{
				Food_Flag = 0;
				break;
			}
		}
		
	}
	s->Snake_Food[0].Food_x = Temp_X;
	s->Snake_Food[0].Food_y = Temp_Y;
}

//失败结算画面
void Snake_GameOver(Snake *s)
{
	OLED_ClearArea(1, 2, 126, 60);
	OLED_Printf(2, 30, OLED_8X16, "恭喜你你输了");
	s->Snake_Status = SNAKE_ON;//表示进入一个死区，switch什么都不执行
	OLED_Update();
}

//胜利结算画面
void Snake_GameVictory(Snake *s)
{
	OLED_ClearArea(1, 2, 126, 60);
	OLED_Printf(2, 30, OLED_8X16, "很遗憾你赢了");
	s->Snake_Status = SNAKE_ON;//表示进入一个死区，switch什么都不执行
	OLED_Update();
}

//绘制蛇身
void Snake_CreateTmp(Snake s)
{
	OLED_ClearArea(1, 2, 126, 60);
	
	for(uint16_t i = s.Snake_Tail; i != (s.Snake_Head + 1) % MAX_LEN; i = (i + 1) % MAX_LEN)
	{
		uint16_t Temp_X = s.Snake_Body[i].x;
		uint16_t Temp_y = s.Snake_Body[i].y;
		
		//转换成屏幕实际需要涂刷的位置
		OLED_DrawRectangle((Temp_X * 6 + 1),(Temp_y * 6 + 2) , 6, 6, OLED_FILLED);		
	}
	
	
	//打印食物
	uint16_t Temp_Food_X = s.Snake_Food[0].Food_x;
	uint16_t Temp_Food_y = s.Snake_Food[0].Food_y;	
	OLED_DrawRectangle((Temp_Food_X * 6 + 1),(Temp_Food_y * 6 + 2) , 6, 6, OLED_UNFILLED);
	
	OLED_Update();
}

//游戏初始化函数
void SnakeGame_Init(void)
{
	
	Snake_P1.Snake_SleepTimel = 500;
	Snake_P1.Snake_Status = SNAKE_OK;
	SnakeInit(&Snake_P1, 3, 3);
	
	//生成食物
	Snake_GenerateFood(&Snake_P1);
	
}

//销毁函数
void SnakeDeInit(Snake *s)
{
	memset(s, 0, sizeof(Snake));
}

//蛇移动函数
void Snake_Move(Snake *s)
{
	uint16_t Temp_X = s->Snake_Body[s->Snake_Head].x;
	uint16_t Temp_Y = s->Snake_Body[s->Snake_Head].y;
	
	switch(IR_GetKey())
	{
		case Key_W: Temp_Y--; break;
			
		case Key_S: Temp_Y++; break;
		
		case Key_A: Temp_X--; break;
			
		case Key_D: Temp_X++; break;
		
	}
	
	//入队
	Snake_PushHead(s, Temp_X, Temp_Y);
	
	//检查是否迟到食物
	if((Temp_X == s->Snake_Food[0].Food_x) && (Temp_Y == s->Snake_Food[0].Food_y))
	{
		//吃到食物 生成新食物
		Snake_GenerateFood(s);
	}
	else
	{
		//没吃到食物，出队
		Snake_DeleteTail(s);
	}	
	//死亡判定
	Snake_IsDead(s);
	//胜利判定
	if(s->Snake_Size == SNAKE_WIN_LENGTH )
	{
		s->Snake_Status = SNAKE_NB;
	}
	
}

//void testwads(void)
//{
//	OLED_DrawRectangle((x * 6 + 1),(y * 6 + 2) , 6, 6, OLED_FILLED);
//	if(IR_Flag == 1)
//	{
//		IR_Flag = 0;
//		OLED_Clear();
//		switch(IR_GetKey())
//		{
//			case Key_W:
//				y--;
//				break;
//			case Key_S:
//				y++;

//				break;
//			case Key_A:
//				x--;
//				
//				break;
//			case Key_D:
//				x++;

//				break;				
//		}
//		OLED_Update();
//	}

//}

//进程函数
void GreedySnake_Proc(void)
{	
	if(SnakeState_Flag == 1)
	{	
		static uint8_t Snake_DelayTime_Flag = 0;//延迟状态机标志位
		static uint8_t SnakeInit_Flag = 0; //Snake初始化状态标志位
		
		static uint64_t Snake_DelayTime = 0;
		//初始化状态机
		if(SnakeInit_Flag == 0)
		{
			//绘制游戏边框
			CreateMap();
			//初始化游戏
			SnakeGame_Init();
			SnakeInit_Flag = 1;
		}
		
		//延迟状态机
		if(Snake_DelayTime_Flag == 0)
		{
			Snake_DelayTime = App_Timer_GetTick() + Snake_P1.Snake_SleepTimel;
			Snake_DelayTime_Flag = 1;
		}
		if(App_Timer_GetTick() > Snake_DelayTime)
		{
			Snake_DelayTime_Flag = 0;
			
			if(Snake_P1.Snake_Status == SNAKE_OK)
			{
				Snake_Move(&Snake_P1);
			}
			
			switch(Snake_P1.Snake_Status)
			{
				case SNAKE_OK:
				{
					Snake_CreateTmp(Snake_P1);
					break;
				}
				case SNAKE_KILL_SELF:
				case SNAKE_KILL_WALL:
				{
					Snake_GameOver(&Snake_P1);
					break;
				}				
				case SNAKE_NB:
				{
					Snake_GameVictory(&Snake_P1);
					break;
				}
					
			}
		}
		//退出游戏
		if(IR_GetKey() == Key_XingHao)
		{
			SnakeDeInit(&Snake_P1);
			Menu_Flag = 1;		
			SnakeInit_Flag = 0;			
			SnakeState_Flag = 0;
			OLED_Clear();
		}
	}
}
