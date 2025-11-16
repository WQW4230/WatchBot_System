#include "LED_Frequency.h"
#include "LED_Blink.h"
#include "Scheduler.h"
#include "OLED.h"
#include "NEC_driver.h"
#include "USART_FrameHandler.h"

LED_SetState_t LED_Set; //led全局控制句柄
LED_t LED_PC13_Struct;  //PC13LED控制句柄

extern void Menu_Proc(void); //退出一级菜单进入的进程函数

void LedSet_Menu_Proc(void); //LED设置一级菜单
static void LedSet_Proc_Stm(void); //LED设置二级菜单 第一行STM32LED设置
static void LedSet_Proc_Esp(void); //LED设置二级菜单 第二行ESP32LED设置
static void LedSet_Proc_Cam(void); //LED设置二级菜单 第二行ESP32CAM_LED设置

void Led_BlinkInit(void)
{
	LED_PC13_Struct.GPIOX = GPIOC;
	LED_PC13_Struct.GPIO_PIN = GPIO_Pin_13;
	LED_PC13_Struct.Lofic = LED_ACTIVE_LOW; //输出开漏接法低电平点亮
	LED_Init(&LED_PC13_Struct);
	
	LED_SetMode(&LED_PC13_Struct, LED_BLINK, 1000, 500); //设置亮1000ms 暗500ms
	
	Scheduler_AddTask(LED_Proc, 100, 5, 1000); //驱动层LED任务进程
}

/*
	菜单选项确认后进入功能设置界面
	参数 index：选择的是哪个功能
*/
static void menu_selector(uint8_t index)
{
	switch(index)
	{
		//选择0为STM32板载LED设置界面
		case 0:
			//删除LED一级菜单 创建二级菜单任务
			vTask_Delete(LedSet_Menu_Proc);
			Scheduler_AddTask(LedSet_Proc_Stm, 50, 5, 1000);
			break;
		//选择1为ESP32板载LED设置界面
		case 1:
			vTask_Delete(LedSet_Menu_Proc);
			Scheduler_AddTask(LedSet_Proc_Esp, 50, 5, 1000);
			break;
		//选择2为ESP32_CAMLED设置界面
		case 2:
			vTask_Delete(LedSet_Menu_Proc);
			Scheduler_AddTask(LedSet_Proc_Cam, 50, 5, 1000);
			break;
	}
	//清空屏幕
	OLED_Clear();
}

/*
	LED闪烁二级菜单，用于设置STM32板载的LED灯
*/
static void LedSet_Proc_Stm(void)
{
	static uint8_t Led_Stm_index = 0;//光标指向的位置
	static uint8_t first_show = 1; //第一次进入菜单的时候显示
	
	if(first_show)
	{
		OLED_Clear();
		OLED_Printf(0, 0, OLED_8X16, "STM32LED_SET");
		OLED_Printf(0, 16, OLED_8X16, "Brigh_time:%d", LED_Set.stm_LedBrigh_time);
		OLED_Printf(0, 32, OLED_8X16, "Dark_time:%d", LED_Set.stm_LedDark_time);
		OLED_ReverseArea(0, 16 * Led_Stm_index + 16, 128, 16);
		
		first_show = 0; //复位
	}
	
	uint8_t key;	//存按键值
	if(IR_GetKey(&key) == -1) return;//无按键或错误

	switch(key)
	{
		// *号键 退出菜单 消除任务 退出函数
		case Key_XingHao:
			OLED_Clear();
			first_show = 1; //第一次进入显示置位
			vTask_Delete(&LedSet_Proc_Stm);//删除LED二级菜单任务
			Scheduler_AddTask(LedSet_Menu_Proc, 100, 5, 1000);//创建LED一级菜单任务
			return;
		
		// #号键 删除当前选中所有数值
		case Key_JingHao:
			if(Led_Stm_index == 0)
			{
				LED_Set.stm_LedBrigh_time = 0;
			}
			if(Led_Stm_index == 1)
			{
				LED_Set.stm_LedDark_time = 0;
			}
			break;
			
		// W S键 上下编辑页切换
		case Key_W:
		case Key_S:
			Led_Stm_index = (Led_Stm_index + 1) % 2;
			break;
		
		// 数字键 0 -9 输入
		default:
			if(key >= 10) break; //如果按键不是0 - 9就退出;
			if(Led_Stm_index == 0)
			{
				LED_Set.stm_LedBrigh_time = LED_Set.stm_LedBrigh_time * 10 + key;
			}
			if(Led_Stm_index == 1)
			{
				LED_Set.stm_LedDark_time = LED_Set.stm_LedDark_time * 10 + key;
			}
			break;
	}
	
	//更新LED状态
	LED_SetMode(&LED_PC13_Struct, LED_BLINK, LED_Set.stm_LedBrigh_time, LED_Set.stm_LedDark_time);
	
	//亮小于等于50ms 灭大于50ms 关灯
	if(LED_Set.stm_LedBrigh_time <= 50 && LED_Set.stm_LedDark_time > 50) LED_SetMode(&LED_PC13_Struct, LED_OFF, 0, 0);
	
	//亮大于等于50ms 灭小于50ms 关灯
	if(LED_Set.stm_LedBrigh_time >= 50 && LED_Set.stm_LedDark_time < 50) LED_SetMode(&LED_PC13_Struct, LED_ON, 0, 0);
	
	//亮小于50ms 灭小于50ms 关灯
	if(LED_Set.stm_LedBrigh_time <= 50 && LED_Set.stm_LedDark_time <= 50) LED_SetMode(&LED_PC13_Struct, LED_ON, 0, 0);
	
	//屏幕显示刷新
	OLED_Clear();
	OLED_Printf(0, 0, OLED_8X16, "STM32LED_SET");
	OLED_Printf(0, 16, OLED_8X16, "Brigh_time:%d", LED_Set.stm_LedBrigh_time);
	OLED_Printf(0, 32, OLED_8X16, "Dark_time:%d", LED_Set.stm_LedDark_time);
	OLED_ReverseArea(0, 16 * Led_Stm_index + 16, 128, 16);
}

/*
	LED闪烁二级菜单，用于设置ESP32板载的LED灯
*/
static void LedSet_Proc_Esp(void) //LED设置二级菜单 第二行
{
	static uint8_t Led_Esp_index = 0;//光标指向的位置
	static uint8_t first_show = 1; //第一次进入菜单的时候显示
	
	if(first_show)
	{
		OLED_Clear();
		OLED_Printf(0, 0, OLED_8X16, "ESP32LED_SET");
		OLED_Printf(0, 16, OLED_8X16, "Brigh_time:%d", LED_Set.esp_LedBrigh_time);
		OLED_Printf(0, 32, OLED_8X16, "Dark_time:%d", LED_Set.esp_LedDark_time);
		OLED_ReverseArea(0, 16 * Led_Esp_index + 16, 128, 16);
		
		first_show = 0; //复位
	}
	
	uint8_t key;	//存按键值
	if(IR_GetKey(&key) == -1) return;//无按键或错误

	switch(key)
	{
		// *号键 退出菜单 消除任务 退出函数
		case Key_XingHao:
			OLED_Clear();
			first_show = 1; //第一次进入显示置位
			vTask_Delete(LedSet_Proc_Esp);//删除LED二级菜单任务
			Scheduler_AddTask(LedSet_Menu_Proc, 100, 5, 1000);//创建LED一级菜单任务
			return;
		
		// #号键 删除当前选中所有数值
		case Key_JingHao:
			if(Led_Esp_index == 0)
			{
				LED_Set.esp_LedBrigh_time = 0;
			}
			if(Led_Esp_index == 1)
			{
				LED_Set.esp_LedDark_time = 0;
			}
			break;
			
		// W S键 上下编辑页切换
		case Key_W:
		case Key_S:
			Led_Esp_index = (Led_Esp_index + 1) % 2;
			break;
		
		case Key_OK:
			USART_SenFrame(CMD_ESP32_LED, LED_Set.esp_LedBrigh_time, LED_Set.esp_LedDark_time);
			break;
		
		// 数字键 0 -9 输入
		default:
			if(key >= 10) break; //如果按键不是0 - 9就退出;
			if(Led_Esp_index == 0)
			{
				LED_Set.esp_LedBrigh_time = LED_Set.esp_LedBrigh_time * 10 + key;
			}
			if(Led_Esp_index == 1)
			{
				LED_Set.esp_LedDark_time = LED_Set.esp_LedDark_time * 10 + key;
			}
			break;
	}
	
	//更新LED状态
	//////////待更新
	
	//屏幕显示刷新
	OLED_Clear();
	OLED_Printf(0, 0, OLED_8X16, "ESP32LED_SET");
	OLED_Printf(0, 16, OLED_8X16, "Brigh_time:%d", LED_Set.esp_LedBrigh_time);
	OLED_Printf(0, 32, OLED_8X16, "Dark_time:%d", LED_Set.esp_LedDark_time);
	OLED_ReverseArea(0, 16 * Led_Esp_index + 16, 128, 16);
}

/*
	LED闪烁二级菜单，用于设置ESP32CAM旁的LED灯
*/
static void LedSet_Proc_Cam(void) //LED设置二级菜单 第二行ESP32CAM_LED设置
{
	static uint8_t Led_Cam_index = 0;//光标指向的位置
	static uint8_t first_show = 1; //第一次进入菜单的时候显示
	static uint8_t Cam_Colour = 0; //当前颜色
	
	if(first_show)
	{
		OLED_Clear();
		OLED_Printf(0, 0, OLED_8X16, "CAMLED_SET");
		OLED_Printf(0, 16, OLED_8X16, "Brigh_time:%d", LED_Set.cam_LedBrigh_time);
		OLED_Printf(0, 32, OLED_8X16, "Dark_time:%d", LED_Set.cam_LedDark_time);
		switch(Cam_Colour)
		{
			case WHITE:
				OLED_Printf(0, 48, OLED_8X16, "Colour:White");
				break;	
			case BLUE:
				OLED_Printf(0, 48, OLED_8X16, "Colour:Blue");
				break;
			case RED:
				OLED_Printf(0, 48, OLED_8X16, "Colour:Red");
				break;
			case ALARM:
				OLED_Printf(0, 48, OLED_8X16, "Colour:Alarm");
				break;
			case OFF:
				OLED_Printf(0, 48, OLED_8X16, "Colour:Off");
				break;
		}
		OLED_ReverseArea(0, 16 * Led_Cam_index + 16, 128, 16);
		
		first_show = 0; //复位
	}
	
	uint8_t key;	//存按键值
	if(IR_GetKey(&key) == -1) return;//无按键或错误

	switch(key)
	{
		// *号键 退出菜单 消除任务 退出函数
		case Key_XingHao:
			OLED_Clear();
			first_show = 1; //第一次进入显示置位
			vTask_Delete(LedSet_Proc_Cam);//删除LED二级菜单任务
			Scheduler_AddTask(LedSet_Menu_Proc, 100, 5, 1000);//创建LED一级菜单任务
			return;
		
		// #号键 删除当前选中所有数值
		case Key_JingHao:
			if(Led_Cam_index == 0)
			{
				LED_Set.cam_LedBrigh_time = 0;
			}
			if(Led_Cam_index == 1)
			{
				LED_Set.cam_LedDark_time = 0;
			}
			break;
			
		// W S键 上下编辑页切换
		case Key_W:
		case Key_S:
			Led_Cam_index = (Led_Cam_index + 1) % 3;
			break;
		
		// OK键 选择颜色时候点一下切换一次
		case Key_A:
		case Key_D:
			if(Led_Cam_index != 2) break; //光标不在颜色位置直接退出
		
			Cam_Colour = (Cam_Colour + 1) % 5;//五种模式
			switch(Cam_Colour)
			{
				case WHITE:
					LED_Set.cam_LedColour = WHITE;
					break;
				
				case BLUE:
					LED_Set.cam_LedColour = BLUE;
					break;
				
				case RED:
					LED_Set.cam_LedColour = RED;
					break;
				case ALARM:
					LED_Set.cam_LedColour = ALARM;
					break;
				case OFF:
					LED_Set.cam_LedColour = OFF;
			}
			break;
		
		//确认键
		case Key_OK:
			switch(LED_Set.cam_LedColour)
			{
				case WHITE: //白色
					USART_SenFrame(CMD_ESP32CAM_WHITE_LED, LED_Set.esp_LedBrigh_time, LED_Set.esp_LedDark_time);
					break;
				case BLUE:  //蓝色
					USART_SenFrame(CMD_ESP32CAM_BLUE_LED, LED_Set.esp_LedBrigh_time, LED_Set.esp_LedDark_time);
					break;
				case RED:		//红色
					USART_SenFrame(CMD_ESP32CAM_RED_LED, LED_Set.esp_LedBrigh_time, LED_Set.esp_LedDark_time);
					break;
				case ALARM:
					USART_SenFrame(CMD_ESP32CAM_ALARM_LED, LED_Set.esp_LedBrigh_time, LED_Set.esp_LedDark_time);
					break;
				case OFF:
					USART_SenFrame(CMD_ESPCAM_OFF_LDE, LED_Set.esp_LedBrigh_time, LED_Set.esp_LedDark_time);
					break;
			}
			
		// 数字键 0 -9 输入
		default:
			if(key >= 10) break; //如果按键不是0 - 9就退出;
			if(Led_Cam_index == 0)
			{
				LED_Set.cam_LedBrigh_time = LED_Set.cam_LedBrigh_time * 10 + key;
			}
			if(Led_Cam_index == 1)
			{
				LED_Set.cam_LedDark_time = LED_Set.cam_LedDark_time * 10 + key;
			}
			break;
	}
	
	//更新LED状态
	//////////待更新
	
	//屏幕显示刷新
	OLED_Clear();
	OLED_Printf(0, 0, OLED_8X16, "CAMLED_SET");
	OLED_Printf(0, 16, OLED_8X16, "Brigh_time:%d", LED_Set.cam_LedBrigh_time);
	OLED_Printf(0, 32, OLED_8X16, "Dark_time:%d", LED_Set.cam_LedDark_time);
	switch(Cam_Colour)
	{
		case WHITE:
			OLED_Printf(0, 48, OLED_8X16, "Colour:White");
			break;	
		case BLUE:
			OLED_Printf(0, 48, OLED_8X16, "Colour:Blue");
			break;
		case RED:
			OLED_Printf(0, 48, OLED_8X16, "Colour:Red");
			break;
		case ALARM:
			OLED_Printf(0, 48, OLED_8X16, "Colour:Alarm");
			break;
		case OFF:
			OLED_Printf(0, 48, OLED_8X16, "Colour:Off");
			break;
		
	}
	OLED_ReverseArea(0, 16 * Led_Cam_index + 16, 128, 16);
}

/*
	LED一级菜单函数
*/
void LedSet_Menu_Proc(void)
{
	static uint8_t index; 		 //当前菜单索引
	static uint8_t first_show = 1; //第一次进入菜单的时候显示
	if(first_show)
	{
		OLED_Clear();
		OLED_Printf(0, 0, OLED_8X16, "LED_SET");
		OLED_Printf(0, 16, OLED_8X16, "Set-STM32-LED");
		OLED_Printf(0, 32, OLED_8X16, "Set-ESP32-LED");
		OLED_Printf(0, 48, OLED_8X16, "Set-CAM-LED");
		OLED_ReverseArea(0, 16 * index + 16, 128, 16);
		
		first_show = 0; //复位
	}
	
	uint8_t key;	//存按键值
	if(IR_GetKey(&key) == -1) return;//无按键或错误
	
	switch(key)
	{
		case Key_XingHao:
			first_show = 1;//退出时复位第一次进入显示
			vTask_Delete(LedSet_Menu_Proc); //销毁该函数进程
			Scheduler_AddTask(Menu_Proc, 50, 5, 1000);
			
			break;
		case Key_OK:
			first_show = 1;//进入其他菜单时复位第一次进入显示
			menu_selector(index);
			break;
		
		//按键W
		case Key_W:
			if(index == 0) 
				index = 2;
			else
				index--;
			break;
			
		//按键S
		case Key_S:
			index = (index + 1) % 3;
			break;
	
	}
	//OLED显示刷新模块
	OLED_Clear();
	OLED_Printf(0, 0, OLED_8X16, "LED_SET");
	OLED_Printf(0, 16, OLED_8X16, "Set-STM32-LED");
	OLED_Printf(0, 32, OLED_8X16, "Set-ESP32-LED");
	OLED_Printf(0, 48, OLED_8X16, "Set-CAM-LED");
	OLED_ReverseArea(0, 16 * index + 16, 128, 16);
		
}

