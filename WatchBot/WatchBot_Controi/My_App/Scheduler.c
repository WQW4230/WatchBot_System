#include "Scheduler.h"
#include <stddef.h>

static task_cotrol_block_t task[MAX_TASK];			 //任务块指针数组
static uint16_t task_count = 0; 								 //当前任务数量
static task_cotrol_block_t *current_task = NULL; //当前在执行的任务块

/*
	初始化任务块
	定时器3用作系统时钟,不可修改
*/
void Scheduler_Init(void)
{
	App_Timer_Init();
	
	for(uint16_t i=0; i<MAX_TASK; i++)
	{
		task[i].Task_Fun = NULL;
		task[i].state = TASK_STATE_SUSPEND;
		task[i].sleep_time = 0;
	}
}

/*
	添加任务
	Func:任务函数指针
	frequency：执行频率
	priority：任务优先级
	max_ms ：任务最大执行时间
*/
int Scheduler_AddTask(void(*Func)(void), uint32_t frequency, uint16_t priority, uint32_t max_ms)
{
	if(task_count >= MAX_TASK) return -1; //任务满
	for(uint16_t i=0; i<MAX_TASK; i++)
	{
		if(task[i].Task_Fun == NULL)
		{
			task[task_count].Task_Fun  = Func;
			task[task_count].frequency = frequency; 					//执行频率
			task[task_count].max_time  = max_ms;							//最大执行时间
			task[task_count].last_time = App_Timer_GetTick(); //获取当前系统时间
			task[task_count].priority  = priority;						//优先级
			task[task_count].state     = TASK_STATE_READY;     //任务就绪
			task_count++; //任务数量++
			return 1; //任务添加成功，返回1
		}
		
	}
	
	return 0;
}

/*
	更新任务状态
	task：任务函数指针
*/
static void Task_Update(void)
{
	//任务改成就绪态
	uint64_t current_time = App_Timer_GetTick(); //当前时间
	for(uint16_t i=0; i<MAX_TASK; i++)
	{
		if(task[i].Task_Fun == NULL) continue;

		uint64_t last_time    = task[i].last_time;    					 						//上次执行的时间
		uint32_t sleep_time   = task[i].sleep_time;            							//休眠时间
		uint32_t frequency 		= task[i].frequency;                          //执行频率
		
		//到达执行时间改为就绪态
		if((current_time - last_time) >= (sleep_time + frequency))
		{
			task[i].state = TASK_STATE_READY;
			task[i].sleep_time = 0;
		}
	}
}

/*
	挑选优先级最高的任务
	返回值为任务指针数组的索引
	如果返回值为负数则无任务处于就绪态
*/
static int Scheduler_SlelctTask(void)
{
	int index = -1;
	uint16_t HighPriority = 0xffff;
	
	for(uint16_t i=0; i<MAX_TASK; i++)
	{
		//任务处于就绪态且任务优先级最高
		if(task[i].state == TASK_STATE_READY && task[i].priority < HighPriority)
		{
			HighPriority = task[i].priority;
			index = i;
		}
	}
	return index;
}

/*
	任务延时
	参数task : 需要延时的任务指针
	参数Delay_ms : 需要延时的时间
*/
void vTask_Delay(uint32_t Delay_ms)
{
	if(current_task->Task_Fun == NULL) return;
	//任务挂起
	current_task->state = TASK_STATE_SUSPEND;
	current_task->sleep_time = Delay_ms;
	current_task->last_time = App_Timer_GetTick();
}

/*
	销毁任务
	参数task：需要销毁任务函数的指针
*/
void vTask_Delete(task_cotrol_block_t *task)
{
	for(uint16_t i=0; i<task_count; i++)
	{
		if(task[i].Task_Fun == task->Task_Fun)
		{
			task->Task_Fun = NULL;
			task->sleep_time = 0;
			task->tick_counter = 0;
			task_count--;
		}
	}
}
/*
任务调度器
*/
void Scheduler_Run(void)
{
	while(1)
	{
		Task_Update();
		int Task_index = Scheduler_SlelctTask();
		
		//无符合条件任务跳过本次轮询
		if(Task_index < 0) continue;
		
		task_cotrol_block_t *t = &task[Task_index];
		
		uint64_t current_time = App_Timer_GetTick(); 				 //记录任务开始时
		uint64_t execut_time = current_time + t->max_time;   //理论运行的最大执行时间

		//进入运行态
		current_task = t;
		t->state = TASK_STATE_RUNNING;
		t->Task_Fun();
		current_task = NULL;
		
		current_time = App_Timer_GetTick();
		
		if(current_time > execut_time )
		{
			//计算超时了多久
			//uint32_t time_lag = current_time - execut_time;
			
			//进入超时处理阶段
			t->state = TASK_STATE_TIMEOUT;
			//vTask_Delay(t, time_lag); 延时补偿
			t->last_time = App_Timer_GetTick(); //依旧推进
		}
		else
		{
			t->state = TASK_STATE_SUSPEND;
			t->last_time = current_time;
			t->tick_counter++;
		}
		
	}
}
