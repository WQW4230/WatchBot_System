#ifndef _SCHEDULER_H_
#define	_SCHEDULER_H_

#include "stm32f10x.h"            
#include "App_Timer.h" //提供获取系统时间/延迟接口

typedef enum
{
	TASK_STATE_READY, 		//任务就绪
	TASK_STATE_RUNNING,	  //正在执行
	TASK_STATE_SUSPEND,		//任务挂起
	TASK_STATE_TIMEOUT,		//任务超时
}task_state_e;

typedef struct
{
	void (*Task_Fun)(void); //任务函数
	task_state_e state;     //任务状态
	uint32_t frequency;     //执行频率
	uint64_t last_time;     //上次执行时间
	uint32_t max_time;      //最大执行时间
	uint32_t sleep_time;    //任务休眠的时间
	uint16_t priority;      //优先级 0最高
	uint64_t tick_counter;  //任务计数器 统计任务执行次数
	
}task_cotrol_block_t;

#define MAX_TASK 10  //任务最大数量

//调度器初始化
void Scheduler_Init(void);

//任务创建
int Scheduler_AddTask(void(*Func)(void), uint32_t frequency, uint16_t priority,uint32_t max_ms);

//任务非阻塞延时
void vTask_Delay(uint32_t Delay_ms);

//任务删除
void vTask_Delete(task_cotrol_block_t *task);

//调度器
void Scheduler_Run(void);


#endif
