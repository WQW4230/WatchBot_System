#include "Contact.h"
#include <string.h>
#include "OLED.h"
#include "remote_nec_driver.h"
#include "menu.h"

uint8_t Contact_Flag = 0;           //一级菜单标志位
static uint8_t ContactName_Flag = 0;//二级菜单标志位
static uint8_t Input_State_Flag = 0;//三级菜单输入状态标志位

static uint8_t Contact_PageStart = 0;//页面起始 0/0-2， 1/1-3， 2/2-4， 3/3-5

static uint8_t Contact_Cursor = 0;//一级光标指向的位置 -》联系人
static uint8_t Temp_Cursor = 0; //二级光标指向的位置   -》&具体信息name qq wx

static SLT contact_list;//声明顺序表

//键位输入函数全局变量
static char input_Data[12];   //即将存入顺序表的值
static uint8_t input_index = 0;   // 当前输入数组位置
static uint8_t key_last = 0;      // 上次按键
static uint8_t key_count = 0;     // 按键次数
const char key_map[10][4] = 
{
		{'0','-','0','0'}, 
    {'1','A','B','C'},      
    {'2','D','E','F'},      
    {'3','G','H','I'},     
    {'4','J','K','L'},     
    {'5','M','N','O'},     
    {'6','P','Q','R'},     
    {'7','S','T','U'},     
    {'8','V','W','X'},      
    {'9','Y','Z','.'}             
};

//多键输入处理
void Input_Char(uint8_t Input_key)
{
	if(Input_key <= 9)
	{
		if(Input_key == key_last)//循环下一个字母
		{
			key_count++;
		}
		else
		{
			key_last = Input_key;
			key_count = 0;
		}
		
		//映射键位表
		char c = key_map[Input_key][key_count % 4];
		input_Data[input_index] = c;
		
		OLED_Printf(0, 0, OLED_8X16, "Input:");
		OLED_Printf(0, 20, OLED_8X16, "%s", input_Data);
		OLED_Printf(55, 40, OLED_8X16, "%c", c);
		OLED_Update();
	}
	else if(Input_key == Key_OK)//Key_OK的枚举值
	{
		input_index++;
		key_count = 0;
		key_last = 0;
	}
	else if(Input_key == Key_JingHao) //向顺序表写入值
	{
		if(Temp_Cursor == 0)//表示当前光标指向为姓名
		{
			strncpy(contact_list.SQDataType[Contact_Cursor].name, input_Data, 11);
		}
		else if(Temp_Cursor == 1)//表示当前光标指为qq
		{
			strncpy(contact_list.SQDataType[Contact_Cursor].QQ, input_Data, 11);
		}
		else if(Temp_Cursor == 2)//表示当前光标指向微信
		{
			strncpy(contact_list.SQDataType[Contact_Cursor].WX, input_Data, 11);
		}
		
		input_index = 0;
		memset(input_Data, 0, sizeof(input_Data));
		key_count = 0;
		key_last = 0;
	}
	OLED_Update();
}

//输入状态函数-----三级菜单
void Input_State(void)
{
	if(Input_State_Flag == 1)
	{
		if(IR_Flag == 1)
		{
			Input_Char(IR_GetKey());//进入按键输入函数
			
			IR_Flag = 0;
			
			if(IR_GetKey() == Key_XingHao)
			{
				OLED_Clear();
				Input_State_Flag = 0;
				ContactName_Flag = 1;
				//清空三级输入状态数组
				input_index = 0;
				memset(input_Data, 0, sizeof(input_Data));
				key_count = 0;
				key_last = 0;
			}
		}
	}
}

//初始化顺序表
void Contact_Init(void)
{
	memset(&contact_list, 0, sizeof(contact_list));  // 清空全局顺序表
	PersonInfo p1;
	strncpy(p1.name, "Mr.9175", sizeof(p1.name) - 1);
	strncpy(p1.QQ, "498644995", sizeof(p1.QQ) - 1);
	strncpy(p1.WX, "wqw4230", sizeof(p1.WX) - 1);
	contact_list.SQDataType[0] = p1;
	contact_list.size++;
	
	strncpy(p1.name, "Mr.AA", sizeof(p1.name) - 1);
	strncpy(p1.QQ, "51662118", sizeof(p1.QQ) - 1);
	strncpy(p1.WX, "hdfsih", sizeof(p1.WX) - 1);
	contact_list.SQDataType[1] = p1;
	contact_list.size++;
	
	strncpy(p1.name, "Mr.BB.", sizeof(p1.name) - 1);
	strncpy(p1.QQ, "1461516156", sizeof(p1.QQ) - 1);
	strncpy(p1.WX, "FDSFM5", sizeof(p1.WX) - 1);
	contact_list.SQDataType[2] = p1;
	contact_list.size++;
	
	strncpy(p1.name, "Mr.CC", sizeof(p1.name) - 1);
	strncpy(p1.QQ, "46145651", sizeof(p1.QQ) - 1);
	strncpy(p1.WX, "WEFF422", sizeof(p1.WX) - 1);
	contact_list.SQDataType[3] = p1;		
	contact_list.size++;
}
	

//点击名字查看信息显示------二级菜单
static void Name_HomeShow(void)
{
	if(ContactName_Flag == 1)
	{
		OLED_Clear();	
		OLED_Printf(0, 0, OLED_8X16, "Name:%s", contact_list.SQDataType[Contact_Cursor].name);
		OLED_Printf(0, 20, OLED_8X16, "QQ:%s", contact_list.SQDataType[Contact_Cursor].QQ);
		OLED_Printf(0, 40, OLED_8X16, "WX:%s", contact_list.SQDataType[Contact_Cursor].WX);
		
		OLED_ReverseArea(0, Temp_Cursor*20, 128, 20);
		
		if(IR_Flag == 1)
		{
			IR_Flag = 0;
			switch(IR_GetKey())
			{
				case Key_W:
				{
					if(Temp_Cursor != 0)
					{
						Temp_Cursor--;				
					}
					break;
				}
				
				case Key_S:
				{
					if(Temp_Cursor != 2)
					{
						Temp_Cursor++;
					}
					break;
				}
				
				case Key_OK:
				{
					OLED_Clear();
					Input_State_Flag = 1;//进入输入状态
					ContactName_Flag = 0;//退出查看
					break;
				}
				
				case Key_XingHao:
				{
					ContactName_Flag = 0;
					Contact_Flag = 1;
					break;
				}

			}
		}
		
		OLED_Update();
	}
}

//一级菜单翻页显示
static void Contact_HomeShow(void)
{
    OLED_Clear();
	
    for(uint8_t i = 0; i < Contact_PageSize; i++)
    {
        uint8_t index = (Contact_PageStart + i); // 当前要显示的第几个应用
        if(index < MAX_CONTACT)
				{
					OLED_Printf(0, (20*i), OLED_8X16, contact_list.SQDataType[index].name) ;
					if(index == Contact_Cursor)
					{
						OLED_ReverseArea(0, (20*i), 70, 20);
					}
				}
    }
		
    OLED_Update();
}


//一级菜单显示
void Contact_Proc(void)
{
	if(Input_State_Flag == 1)//三级进入输入状态
	{
		Input_State();
	}
	if(ContactName_Flag == 1)//二级菜单查看名字
	{
		Name_HomeShow();
	}
	
	if(Contact_Flag == 1)
	{
		if(IR_Flag == 1)
		{
			IR_Flag = 0;
			switch(IR_GetKey())
			{
				case Key_W:
				{
					if(Contact_Cursor > 0)
					{
						Contact_Cursor--;
						if(Contact_Cursor < Contact_PageStart)
						{
							Contact_PageStart -= Contact_PageSize;
						}
					}
					break;
				}

					
				case Key_S:
				{
					
					if(Contact_Cursor < contact_list.size - 1)//小于联系人个数才能向下移动光标
					{
						Contact_Cursor++;
						if(Contact_Cursor >= Contact_PageStart + Contact_PageSize)
						{
							Contact_PageStart += Contact_PageSize;
						}
					}
					break;
				}
				
				case Key_A://指定位置头插
				{
					if(Contact_Cursor < contact_list.size)
					{
						for(uint8_t i = contact_list.size; i > Contact_Cursor; i--)
						{
							contact_list.SQDataType[i] = contact_list.SQDataType[i - 1];
						}
					}
					contact_list.size++;
					//清除光标内残留数据
					memset(&contact_list.SQDataType[Contact_Cursor], 0, sizeof(PersonInfo));
					OLED_Clear();
					break;
				}
				
				case Key_D://尾插
				{
					Contact_Cursor = contact_list.size;
					
					contact_list.size++;
					OLED_Clear();
					break;
				}
				
				case Key_OK:
				{
					OLED_Clear();
					ContactName_Flag = 1;
					Contact_Flag = 0;
					break;
				}	
				
				case Key_XingHao:
				{
					OLED_Clear();
					Contact_Flag = 0;
					Menu_Flag = 1;
					break;
				}
				
				case Key_JingHao://删除
				{
					if(Contact_Cursor < contact_list.size)
					{
						for(uint8_t i = Contact_Cursor; i < contact_list.size; i++)
						{
							contact_list.SQDataType[i] = contact_list.SQDataType[i + 1];
						}
						contact_list.size--;
						OLED_Clear();
						break;
					}
					if(Contact_Cursor == contact_list.size)
					{
						memset(&contact_list.SQDataType[Contact_Cursor], 0, sizeof(PersonInfo));
						Contact_Cursor--;
						contact_list.size--;
					}
					OLED_Clear();
					break;
				}
		
			}			
		}
		Contact_HomeShow();
	}	
}
