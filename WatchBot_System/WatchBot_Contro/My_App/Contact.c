#include "Contact.h"
#include "Scheduler.h"
#include <string.h>
#include "OLED.h"
#include "menu.h"
#include "NEC_driver.h"

extern void Menu_Proc(void);

static SLT contact_list;//声明顺序表

static void Name_HomeShow(void);

//键位输入函数全局变量
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
static void Input_Char(void)
{
	OLED_Clear();
	OLED_Printf(0, 0, OLED_8X16, "Input:");
	OLED_Printf(0, 20, OLED_8X16, "%s", contact_list.input_Data);
	OLED_Printf(55, 40, OLED_8X16, "%c", contact_list.input_Data[contact_list.input_index]);
	
	uint8_t key;	//存按键值
	if(IR_GetKey(&key) == -1) return;//无按键或错误
	
	switch(key)
	{
		//OK键确认字符
		case Key_OK:
			contact_list.input_index++;
			contact_list.key_count = 0;
			contact_list.key_last = 0;
		break;
		
		//井号键确认修改
		case Key_JingHao:
			if(contact_list.modify_cursor == 0)//表示当前光标指向为姓名
			{
				strncpy(contact_list.SQDataType[contact_list.select_contact].name, contact_list.input_Data, 11);
			}
			else if(contact_list.modify_cursor == 1)//表示当前光标指为qq
			{
				strncpy(contact_list.SQDataType[contact_list.select_contact].QQ, contact_list.input_Data, 11);
			}
			else if(contact_list.modify_cursor == 2)//表示当前光标指向微信
			{
				strncpy(contact_list.SQDataType[contact_list.select_contact].WX, contact_list.input_Data, 11);
			}
			
			contact_list.input_index++;;
			memset(contact_list.input_Data, 0, sizeof(contact_list.input_Data));
			contact_list.key_count = 0;
			contact_list.key_last  = 0;
		break;
		
		case Key_XingHao:
			//清空三级输入状态数组
			vTask_Delete(Input_Char);	
			Scheduler_AddTask(Name_HomeShow, 50, 5, 1000);
			contact_list.input_index = 0;
			memset(contact_list.input_Data, 0, sizeof(contact_list.input_Data));
			contact_list.key_count = 0;
			contact_list.key_last = 0;
		break;
		
		default :
			if(key > 9) break;//不是数字键
			if(key == contact_list.key_last)//循环下一个字母
			{
				contact_list.key_count++;
			}
			else
			{
				contact_list.key_last = key;
				contact_list.key_count = 0;
			}
			
			//映射键位表
			char c = key_map[key][contact_list.key_count % 4];
			contact_list.input_Data[contact_list.input_index] = c;
		break;
	}
}
	
//点击名字查看信息显示------二级菜单
static void Name_HomeShow(void)
{
	OLED_Clear();	
	OLED_Printf(0,  0, OLED_8X16, "Name:%s", contact_list.SQDataType[contact_list.select_contact].name);
	OLED_Printf(0, 20, OLED_8X16, "QQ  :%s", contact_list.SQDataType[contact_list.select_contact].QQ);
	OLED_Printf(0, 40, OLED_8X16, "WX  :%s", contact_list.SQDataType[contact_list.select_contact].WX);
	OLED_ReverseArea(0, contact_list.content_cursor * 20, 128, 20);
		
	uint8_t key;	//存按键值
	if(IR_GetKey(&key) == -1) return;//无按键或错误
	switch(key)
	{
		case Key_W:
			if(contact_list.content_cursor == 0)	
			  contact_list.content_cursor = 3 - 1;	
			else
				contact_list.content_cursor--;
		break;
			
		case Key_S:
			if(contact_list.content_cursor == 3 - 1)
				contact_list.content_cursor = 0;
			else
				contact_list.content_cursor++;
		break;
			
		//进入三级菜单输入修改页面	
		case Key_OK:
			vTask_Delete(Name_HomeShow);
			Scheduler_AddTask(Input_Char, 50, 5, 1000);
		break;
				
		case Key_XingHao:
			vTask_Delete(Name_HomeShow);
			Scheduler_AddTask(Contact_Proc, 50, 5, 1000);
		break;
	}
}

//一级菜单翻页显示
static void Contact_HomeShow(void)
{
    OLED_Clear();
	
    for(uint8_t i = 0; i < CONTACH_PAGESIZE; i++)
    {
			uint8_t index = (contact_list.top_index + CONTACH_MAX + i) % CONTACH_MAX; // 计算需要显示的名字下标
        if(index < contact_list.contach_size)
				{
					OLED_Printf(0, (20*i), OLED_8X16, contact_list.SQDataType[index].name) ;
					if(i == contact_list.name_cursor)
					{
						OLED_ReverseArea(0, (20*i), 70, 20);
					}
				}
    }
}

//菜单向上
static void Option_up(void)
{
	if(contact_list.name_cursor > 0)
		contact_list.name_cursor--;
	else
		contact_list.top_index = (contact_list.top_index + CONTACH_MAX - 1) % CONTACH_MAX;
}

//菜单向下
static void Option_down(void)
{
	if(contact_list.name_cursor < CONTACH_PAGESIZE - 1)
		contact_list.name_cursor++;
	else
		contact_list.top_index = (contact_list.top_index + CONTACH_MAX + 1) % CONTACH_MAX;
}

//获取当前光标实际指向的联系人下标
static void Current_option(void)
{
	contact_list.select_contact = (contact_list.top_index + contact_list.name_cursor) % contact_list.contach_size;
}

//指定位置头部插入
static void Head_insertion(void)
{
	if(contact_list.contach_size == CONTACH_MAX) return; //联系人满
	for(uint8_t i = contact_list.contach_size; i > contact_list.select_contact; i--)
	{
		contact_list.SQDataType[i] = contact_list.SQDataType[i - 1];
	}
	contact_list.contach_size++;
	//清除光标内残留数据
	memset(&contact_list.SQDataType[contact_list.select_contact], 0, sizeof(PersonInfo));
	OLED_Clear();
}

//尾部插入
static void Tail_insertion(void)
{
	if(contact_list.contach_size == CONTACH_MAX) return; //联系人满
	
	contact_list.contach_size++;
}

//联系人删除
static void Contact_Delete(void)
{
	if(contact_list.contach_size == 0) return; //无可删除联系人
	
	Current_option();//  先获得联系人索引
	if(contact_list.select_contact == contact_list.contach_size - 1)//要删除最后一行
	{
		memset(&contact_list.SQDataType[contact_list.select_contact], 0, sizeof(PersonInfo));
		contact_list.contach_size--;
	}
	else if(contact_list.select_contact < contact_list.contach_size - 1)
	{
		for(uint8_t i = contact_list.select_contact; i < contact_list.contach_size - 1; i++)
		{
			contact_list.SQDataType[i] = contact_list.SQDataType[i + 1];
		}
		contact_list.contach_size--;
	}
}
//一级菜单显示
void Contact_Proc(void)
{
	Contact_HomeShow();//屏幕刷新显示
	
	uint8_t key;	//存按键值
	if(IR_GetKey(&key) == -1) return;//无按键或错误
	switch(key)
	{
		case Key_W:
			Option_up();
			break;
		
		case Key_S:
			Option_down();
			break;
		
		//A键头部插入
		case Key_A:
			Head_insertion();
			break;
		
		//D键尾部插入
		case Key_D:
			Tail_insertion();
			break;
		
		//井号键任意位置删除
		case Key_JingHao:
			Contact_Delete();
			break;
		
		//OK键进入联系人详情
		case Key_OK:
			Current_option();//更新光标指向联系人实际下标
			Scheduler_AddTask(Name_HomeShow, 50, 5, 1000);
			vTask_Delete(Contact_Proc);
			break;
		
		//星号退出
		case Key_XingHao:
			Scheduler_AddTask(Menu_Proc, 50, 5, 1000);
			vTask_Delete(Contact_Proc);
			break;	
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
	contact_list.contach_size++;
		
	strncpy(p1.name, "Mr.AA", sizeof(p1.name) - 1);
	strncpy(p1.QQ, "51662118", sizeof(p1.QQ) - 1);
	strncpy(p1.WX, "hdfsih", sizeof(p1.WX) - 1);
	contact_list.SQDataType[1] = p1;
	contact_list.contach_size++;
	
	strncpy(p1.name, "Mr.BB.", sizeof(p1.name) - 1);
	strncpy(p1.QQ, "1461516156", sizeof(p1.QQ) - 1);
	strncpy(p1.WX, "FDSFM5", sizeof(p1.WX) - 1);
	contact_list.SQDataType[2] = p1;
	contact_list.contach_size++;
	
	strncpy(p1.name, "Mr.DD", sizeof(p1.name) - 1);
	strncpy(p1.QQ, "46145651", sizeof(p1.QQ) - 1);
	strncpy(p1.WX, "WEFF422", sizeof(p1.WX) - 1);
	contact_list.SQDataType[3] = p1;		
	contact_list.contach_size++;
	
	strncpy(p1.name, "Mr.EE", sizeof(p1.name) - 1);
	strncpy(p1.QQ, "46145651", sizeof(p1.QQ) - 1);
	strncpy(p1.WX, "WEFF422", sizeof(p1.WX) - 1);
	contact_list.SQDataType[4] = p1;		
	contact_list.contach_size++;
	
	strncpy(p1.name, "Mr.FF", sizeof(p1.name) - 1);
	strncpy(p1.QQ, "46145651", sizeof(p1.QQ) - 1);
	strncpy(p1.WX, "WEFF422", sizeof(p1.WX) - 1);
	contact_list.SQDataType[5] = p1;		
	contact_list.contach_size++;
}
