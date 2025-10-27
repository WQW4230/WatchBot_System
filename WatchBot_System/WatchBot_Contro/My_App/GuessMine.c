#include <GuessMine.h>
#include "Scheduler.h"
#include <string.h>
#include "OLED.h"
#include "NEC_driver.h"
#include "menu.h"

static GuessMine_Statu_t p1; //全局控制

extern void Menu_Proc(void);

//初始化布置雷 ！！！该循可能阻塞
void MineBoard_Init(void)
{
	uint16_t MineCount = MINE_COUNT;
	while(MineCount) 
	{
		uint32_t Row_Random = (GetRandom(COL) + 1);
		uint32_t Col_Random = (GetRandom(ROW) + 1);
		if(p1.MineField[Row_Random][Col_Random] == 0)
		{
			p1.MineField[Row_Random][Col_Random] = 1;
			MineCount--;
		}
	}
}

//胜利UI
void Mine_Victory(void)
{
	OLED_Clear();
	OLED_Printf(0, 20, OLED_8X16, "很遗憾你赢了");
	OLED_UpdateArea(0, 20, 96, 8);
}

//更新光标映射在棋盘的实际位置
void Get_CursorToBoard(void)
{
	p1.MineBoard_X = (p1.Mine_Cursor_X / 6 + 1);
	p1.MineBoard_Y = (p1.Mine_Cursor_Y / 8 + 1);
}

//查看周围雷的数量
uint8_t	HasMine(uint8_t mine[ROWS][COLS], uint8_t x, uint8_t y)
{
	return(mine[x - 1][y - 1] + mine[x][y - 1] + 
	mine[x + 1][y - 1] + mine[x - 1][y]+ 
	mine[x + 1][y] + mine[x - 1][y + 1] + 
	mine[x][y + 1] + mine[x + 1][y + 1]);
}


void GuessMine_Proc(void)
{
	static uint8_t InitShow_Flag;
	if(InitShow_Flag == 0)
	{
		OLED_Clear();
		OLED_DrawLine(0, 0, 0, 63);
		OLED_DrawLine(127, 0, 127, 63);	
		InitShow_Flag = 1;
	}
	
	uint8_t key;	//存按键值
	if(IR_GetKey(&key) == -1) return;//无按键或错误
	
	switch(key)
	{
		case Key_W:
			if(p1.Mine_Cursor_Y > 0)
			{
				OLED_ReverseArea(p1.Mine_Cursor_X, p1.Mine_Cursor_Y, WIDTH, HEIGHT);
										
				p1.Mine_Cursor_Y -= HEIGHT;
				Get_CursorToBoard();
								
				OLED_ReverseArea(p1.Mine_Cursor_X, p1.Mine_Cursor_Y, WIDTH, HEIGHT);												
			}
		break;
				
		case Key_S:
			if(p1.Mine_Cursor_Y < 56)
			{
						
				OLED_ReverseArea(p1.Mine_Cursor_X, p1.Mine_Cursor_Y, WIDTH, HEIGHT);
						
				p1.Mine_Cursor_Y += HEIGHT;
				Get_CursorToBoard();
						
				OLED_ReverseArea(p1.Mine_Cursor_X, p1.Mine_Cursor_Y, WIDTH, HEIGHT);
						
			}
		break;
				
		case Key_A:
			if(p1.Mine_Cursor_X > 1)
			{
				OLED_ReverseArea(p1.Mine_Cursor_X, p1.Mine_Cursor_Y, WIDTH, HEIGHT);
						
				p1.Mine_Cursor_X -= WIDTH;
				Get_CursorToBoard();					
						
				OLED_ReverseArea(p1.Mine_Cursor_X, p1.Mine_Cursor_Y, WIDTH, HEIGHT);
			}
		break;
				
		case Key_D:
			if(p1.Mine_Cursor_X < 121)
			{
				OLED_ReverseArea(p1.Mine_Cursor_X, p1.Mine_Cursor_Y, WIDTH, HEIGHT);
						
				p1.Mine_Cursor_X += WIDTH;
				Get_CursorToBoard();	
						
				OLED_ReverseArea(p1.Mine_Cursor_X, p1.Mine_Cursor_Y, WIDTH, HEIGHT);						
			}
		break;
				
		case Key_OK:
		{
			Get_CursorToBoard();
			if(p1.MineField_Flag[p1.MineBoard_X][p1.MineBoard_Y] == 1) break;//如果格子已经翻开
			
			//有雷
			if(p1.MineField[p1.MineBoard_X][p1.MineBoard_Y] == 1)
			{
				OLED_Clear();
				OLED_Printf(0, 0, OLED_8X16, "恭喜你输了");
				break;
			}
			//无雷
			if(p1.MineField[p1.MineBoard_X][p1.MineBoard_Y] == 0)
			{
				p1.MineField_Flag[p1.MineBoard_X][p1.MineBoard_Y] = 1;//翻开时标志位置位
				OLED_ClearArea(p1.Mine_Cursor_X, p1.Mine_Cursor_Y, WIDTH, HEIGHT);//清除光标
				uint8_t Temp = HasMine(p1.MineField, p1.MineBoard_X, p1.MineBoard_Y);
				OLED_Printf(p1.Mine_Cursor_X, p1.Mine_Cursor_Y, OLED_6X8, "%d", Temp);
				OLED_ReverseArea(p1.Mine_Cursor_X, p1.Mine_Cursor_Y, WIDTH, HEIGHT);
						
				//获胜
				p1.Opened_Count++;
				if(p1.Opened_Count == (ROW * COL) - MINE_COUNT)
				{
					Mine_Victory();
				}
			}	
		}
		break;
				
		case Key_JingHao:
		{
			if((p1.MineField_Flag[p1.MineBoard_X][p1.MineBoard_Y] == 1) || (p1.Closed_Flag == 0)) break;
							
			p1.MineField_Flag[p1.MineBoard_X][p1.MineBoard_Y] = 2;
			p1.Closed_Flag -- ;//旗子数量
			if(p1.MineField[p1.MineBoard_X][p1.MineBoard_Y] == 1)
			{
				p1.Opened_Flag++;//标记猜中
			}
			if(p1.Opened_Flag == MINE_COUNT)
			{
				Mine_Victory();
			}
			OLED_ClearArea(p1.Mine_Cursor_X, p1.Mine_Cursor_Y, WIDTH, HEIGHT);
			OLED_DrawRectangle(p1.Mine_Cursor_X, p1.Mine_Cursor_Y, WIDTH, HEIGHT, OLED_UNFILLED);
			break;
		}	

		case Key_XingHao:
		{
			vTask_Delete(GuessMine_Proc);
			Scheduler_AddTask(Menu_Proc, 50, 5, 0);
			memset(p1.MineField_Flag, 0, sizeof(p1.MineField_Flag));//清空标记数组
			InitShow_Flag = 0;
			p1.Opened_Count = 0;
			p1.Opened_Flag = 0;
			p1.Closed_Flag = MINE_FLAG_COUNT;
			p1.Mine_Cursor_X = 7;
			p1.Mine_Cursor_Y = 8;
		}
				
	}		
}
