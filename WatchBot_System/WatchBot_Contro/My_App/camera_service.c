#include "camera_service.h"
#include "Scheduler.h"
#include "OLED.h"
#include "NEC_driver.h"
#include "USART_FrameHandler.h"

extern void Menu_Proc(void);//退出后进入的任务进程

void camera_service_proc(void)
{
	OLED_Clear();
	OLED_Printf(0, 0, OLED_8X16, "相机按下OK键拍照");
	
	uint8_t key;	//存按键值
	if(IR_GetKey(&key) == -1) return;//无按键或错误
	
	switch(key)
	{
		case Key_OK:
		{
			uint64_t now_time = App_Timer_GetTick();
			static uint64_t last_time;
			if((now_time - last_time) >= CAPTURE_TIME_INTERVAL)
			{
				UART_SenCmd(CMD_ESP32_PICTURE);//OK键拍照
				last_time = App_Timer_GetTick();
			}
			
		}
			
			break;
		
		//星号退出
		case Key_XingHao:
			vTask_Delete(camera_service_proc);
			Scheduler_AddTask(Menu_Proc, 100, 5, 1000);
			break;
		default:
			break;
	}

}
