#include "LED_Frequency.h"
#include "LED_Blink.h"
#include "Scheduler.h"
#include "OLED.h"
#include "NEC_driver.h"

void LedSet_Menu_Proc(void); //LED设置一级菜单

void LedSet_Proc_Stm(void); //LED设置二级菜单 第一行STM32LED设置


//led全局控制句柄
LED_SetState_t LED_Set;

//PC13LED控制句柄
LED_t LED_PC13_Struct;


void Led_BlinkInit(void)
{
	LED_PC13_Struct.GPIOX = GPIOC;
	LED_PC13_Struct.GPIO_PIN = GPIO_Pin_13;
	LED_PC13_Struct.Lofic = LED_ACTIVE_LOW; //输出开漏接法低电平点亮
	LED_Init(&LED_PC13_Struct);
	
	LED_SetMode(&LED_PC13_Struct, LED_BLINK, 1000, 500);
	
	Scheduler_AddTask(LED_Proc_test, 100, 5, 1000);
	Scheduler_AddTask(LedSet_Menu_Proc, 100, 2, 1000);
	Scheduler_AddTask(OLED_UpdateStep, 100, 5, 1000);
}

void LED_Proc_test(void)
{
	LED_Proc();
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
			break;
		//选择2为ESP32_CAMLED设置界面
		case 2:
			break;
	}
	//清空屏幕
	OLED_Clear();
}

/*
	LED闪烁二级菜单，用于设置STM32板载的LED灯
*/
void LedSet_Proc_Stm(void)
{
	if(NEC_RxFlag == 0) return;
	
	//获取按键值
	uint8_t key = IR_GetKey();
	
	static uint8_t Led_Stm_index = 0;//光标指向的位置

	switch(key)
	{
		// *号键 退出菜单 消除任务 退出函数
		case Key_XingHao:
			OLED_Clear();
			//删除LED二级菜单任务
			vTask_Delete(&LedSet_Proc_Stm);
			//创建LED一级菜单任务
			Scheduler_AddTask(LedSet_Menu_Proc, 100, 5, 1000);
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

void LedSet_Menu_Proc(void)
{
	if(NEC_RxFlag == 0) return;
	
	//获取按键值
	uint8_t key = IR_GetKey();
	
	//当前选择的菜单
	static uint8_t index;
	
	switch(key)
	{
		case Key_XingHao:
			break;
		case Key_OK:
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

