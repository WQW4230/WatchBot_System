#include <GuessMine.h>
#include <string.h>
#include "OLED.h"
#include "NEC_driver.h"
#include "menu.h"

uint8_t GuessMine_Flag = 0;

static uint8_t MineField[ROWS][COLS] = {0};//整个棋盘
static uint8_t MineField_Flag[ROWS][COLS] = {0};//为1表示格子已经翻开

//光标映射在屏幕上的实际点位
static uint8_t Mine_Cursor_X = 7;
static uint8_t Mine_Cursor_Y = 8;

//光标映射在棋盘的实际位置
static uint8_t MineBoard_X = 0;
static uint8_t MineBoard_Y = 0;

//打开格子数量
static uint16_t Opened_Count = 0;

//插旗正确数量
static uint16_t Opened_Flag = 0;

//剩余旗子数量
static uint16_t Closed_Flag = MINE_FLAG_COUNT;

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
	MineBoard_X = (Mine_Cursor_X / 6 + 1);
	MineBoard_Y = (Mine_Cursor_Y / 8 + 1);
}


//布置雷 ！！！该循可能阻塞
void MineBoard_Init(void)
{
	uint16_t MineCount = MINE_COUNT;
	while(MineCount) 
	{
		uint32_t Row_Random = (GetRandom(COL) + 1);
		uint32_t Col_Random = (GetRandom(ROW) + 1);
		if( MineField[Row_Random][Col_Random] == 0)
		{
			MineField[Row_Random][Col_Random] = 1;
			MineCount--;
		}
	}
}

//查看周围雷的数量
uint8_t	HasMine(uint8_t mine[ROWS][COLS], uint8_t x, uint8_t y)
{
	return(mine[x - 1][y - 1] + mine[x][y - 1] + 
	mine[x + 1][y - 1] + mine[x - 1][y]+ 
	mine[x + 1][y] + mine[x - 1][y + 1] + 
	mine[x][y + 1] + mine[x + 1][y + 1]);
}

//显示光标函数
void Mine_CursorShow(void)
{
	//线条显示
	OLED_DrawLine(0, 0, 0, 63);
	OLED_DrawLine(127, 0, 127, 63);		
	
	OLED_Update();
}

void GuessMine_Proc(void)
{
	if(GuessMine_Flag == 1)
	{
		uint8_t key;	//存按键值
	if(IR_GetKey(&key) == -1) return;//无按键或错误
			switch(key)
			{
				case Key_W:
					if(Mine_Cursor_Y > 0)
					{
						OLED_ReverseArea(Mine_Cursor_X, Mine_Cursor_Y, WIDTH, HEIGHT);
										
						Mine_Cursor_Y -= HEIGHT;
						Get_CursorToBoard();
									
						OLED_ReverseArea(Mine_Cursor_X, Mine_Cursor_Y, WIDTH, HEIGHT);												
					}
					break;
				
				case Key_S:
					if(Mine_Cursor_Y < 56)
					{
						
						OLED_ReverseArea(Mine_Cursor_X, Mine_Cursor_Y, WIDTH, HEIGHT);
						
						Mine_Cursor_Y += HEIGHT;
						Get_CursorToBoard();
						
						OLED_ReverseArea(Mine_Cursor_X, Mine_Cursor_Y, WIDTH, HEIGHT);
						
					}
					break;
				
				case Key_A:
					if(Mine_Cursor_X > 1)
					{
						OLED_ReverseArea(Mine_Cursor_X, Mine_Cursor_Y, WIDTH, HEIGHT);
						
						Mine_Cursor_X -= WIDTH;
						Get_CursorToBoard();					
						
						OLED_ReverseArea(Mine_Cursor_X, Mine_Cursor_Y, WIDTH, HEIGHT);
					}
					break;
				
				case Key_D:
					if(Mine_Cursor_X < 121)
					{
						OLED_ReverseArea(Mine_Cursor_X, Mine_Cursor_Y, WIDTH, HEIGHT);
						
						Mine_Cursor_X += WIDTH;
						Get_CursorToBoard();	
						
						OLED_ReverseArea(Mine_Cursor_X, Mine_Cursor_Y, WIDTH, HEIGHT);						
					}
					break;
				
				case Key_OK:
				{
					Get_CursorToBoard();
					if(MineField_Flag[MineBoard_X][MineBoard_Y] == 1)//如果格子已经翻开
					{
						break;
					}
					//有雷
					if(MineField[MineBoard_X][MineBoard_Y] == 1)
					{
						OLED_Clear();
						OLED_Printf(0, 0, OLED_8X16, "恭喜你输了");
						break;
					}
					//无雷
					if(MineField[MineBoard_X][MineBoard_Y] == 0)
					{
						MineField_Flag[MineBoard_X][MineBoard_Y] = 1;//翻开时标志位置位
						OLED_ClearArea(Mine_Cursor_X, Mine_Cursor_Y, WIDTH, HEIGHT);//清除光标
						uint8_t Temp = HasMine(MineField, MineBoard_X, MineBoard_Y);
						OLED_Printf(Mine_Cursor_X, Mine_Cursor_Y, OLED_6X8, "%d", Temp);
						OLED_ReverseArea(Mine_Cursor_X, Mine_Cursor_Y, WIDTH, HEIGHT);
						
						//获胜
						Opened_Count++;
						if(Opened_Count == (ROW * COL) - MINE_COUNT)
						{
							Mine_Victory();
						}
						break;
					}	
				}
				break;
				
				case Key_JingHao:
				{
					if((MineField_Flag[MineBoard_X][MineBoard_Y] == 1) || (Closed_Flag == 0))
					{
						break;
					}		
					MineField_Flag[MineBoard_X][MineBoard_Y] = 2;
					Closed_Flag -- ;//旗子数量
					if(MineField[MineBoard_X][MineBoard_Y] == 1)
					{
						Opened_Flag++;//标记猜中
					}
					if(Opened_Flag == MINE_COUNT)
					{
						Mine_Victory();
					}
					OLED_ClearArea(Mine_Cursor_X, Mine_Cursor_Y, WIDTH, HEIGHT);
					OLED_DrawRectangle(Mine_Cursor_X, Mine_Cursor_Y, WIDTH, HEIGHT, OLED_UNFILLED);
					break;
				}			
				
				case Key_XingHao:
				{
					GuessMine_Flag = 0;
					memset(MineField_Flag, 0, sizeof(MineField_Flag));//清空标记数组
					Opened_Count = 0;
					Opened_Flag = 0;
					Closed_Flag = MINE_FLAG_COUNT;
					OLED_Clear();
					Mine_Cursor_X = 7;
					Mine_Cursor_Y = 8;
					break;
				}
			}	
			OLED_Update();			
		}	

			
	

}
