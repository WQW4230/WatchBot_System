#include "remote_nec_driver.h"
#include "App_Timer.h"
#include "Alarm.h"

uint32_t IR_Key;//成功解码的完整数据
uint8_t IR_Data;//8bit数据位
uint8_t IR_Flag;//可用状态标志位
uint8_t IR_Sta = 0;//状态标志位
uint8_t IR_Up = 0;//按钮抬起
uint8_t IR_PulseCnt = 0;//已经解码的次数 共计32位
uint32_t IR_Code = 0;//正在解码的数据
uint16_t IR_ThisPulse = 0;//当前捕获的值
uint16_t IR_LastPulse = 0;//上次捕获的值
uint16_t IR_PulseSub = 0;//脉宽
uint16_t LianfaCnt;//连发判定

typedef struct
{
	uint8_t IR_KeyData;//按键编码
	Key_Enum KeyEnum;//按键枚举
}IR_KeyStruct;

IR_KeyStruct Key_arr[18] = 
{
	{0x98, Key_Zero},
	{0xA2, Key_One},
	{0x62, Key_Two},
	{0xE2, Key_three},
	{0x22, Key_Four}, 
	{0x02, Key_Five}, 
	{0xC2, Key_Six},
	{0xE0, Key_Seven},
	{0xA8, Key_Eight},
	{0x90, Key_Nine},
	{0x00, Key_ON},
	{0x68, Key_XingHao},
	{0xB0, Key_JingHao},
	{0x18, Key_W},
	{0x4A, Key_S},
	{0x10, Key_A},
	{0x5A, Key_D},
	{0x38, Key_OK},		
};

void Capture_Config(void)
{
	//PA8_IPU
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	//定时器时钟
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_TIM1, ENABLE);
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_TIM1,DISABLE);
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
	
	TIM_ARRPreloadConfig(TIM1, ENABLE);//预加载
	
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
	TIM_TimeBaseInitStruct.TIM_Prescaler = 144-1;//APB2到TIM频率分频器
	TIM_TimeBaseInitStruct.TIM_Period = 0xffff;//定时周期
	TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStruct.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInitStruct.TIM_ClockDivision = 0;//不分割
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseInitStruct);
	
	TIM_GenerateEvent(TIM1, TIM_EventSource_Update);//手动产生事件立即生效以上配置
	
	//输入捕获配置
	TIM_SelectHallSensor(TIM1, DISABLE);//禁用霍尔模式
	
	TIM_SetClockDivision(TIM1,TIM_CKD_DIV1);//输入滤波不进行分频
	
	TIM_ICInitTypeDef TIM_ICInitStruct;
	TIM_ICInitStruct.TIM_Channel = TIM_Channel_1;//选择通道1
	TIM_ICInitStruct.TIM_ICFilter = 0x09;//输入滤波参数
	TIM_ICInitStruct.TIM_ICPolarity = TIM_ICPolarity_Falling;//下降边沿触发
	TIM_ICInitStruct.TIM_ICPrescaler = TIM_ICPSC_DIV1;//输入的信号无需分配
	TIM_ICInitStruct.TIM_ICSelection = TIM_ICSelection_DirectTI;//信号直接来自通道本身
	TIM_ICInit(TIM1, &TIM_ICInitStruct);
	
	//定时器中断使能
	TIM_Cmd(TIM1, ENABLE);
	TIM_CCxCmd(TIM1, TIM_Channel_1, TIM_CCx_Enable);//捕获使能
	TIM_ITConfig(TIM1, TIM_IT_CC1 | TIM_IT_Update, ENABLE);
	TIM_ClearFlag(TIM1, TIM_IT_CC1 | TIM_IT_Update);
	
	//中断优先级配置
	NVIC_InitTypeDef NVIC_InitStruct;
	NVIC_InitStruct.NVIC_IRQChannel = TIM1_CC_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);
	
	//TIM1_UP_IRQn 更新中断
	NVIC_InitStruct.NVIC_IRQChannel = TIM1_UP_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);
}


Key_Enum IR_GetKey(void)
{
	uint16_t i;
	for( i = 0; i < 18; i++)
	{
		if(IR_Data == (Key_arr[i].IR_KeyData))
			return Key_arr[i].KeyEnum;
	}
	return Key_ON;
}

//中断入口
void TIM1_CC_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM1, TIM_IT_CC1) == SET)
	{
		TIM_ClearITPendingBit(TIM1, TIM_IT_CC1);
		IR_Up = 0;
		IR_ThisPulse = TIM_GetCapture1(TIM1);
		
		//得到脉宽
		if(IR_ThisPulse > IR_LastPulse)
		{
			IR_PulseSub = IR_ThisPulse - IR_LastPulse;
		}
		else
		{
			IR_PulseSub = IR_ThisPulse + (0xffff - IR_LastPulse);
		}
		IR_LastPulse = IR_ThisPulse;
		IR_PulseCnt++;
		
		//起始位校验
		if(IR_PulseCnt == 2)
		{
			if((IR_PulseSub > 6000) && (IR_PulseSub < 8000))
			{
				IR_Sta = 0x01;
			}
			else
			{
				IR_PulseSub = 0;
			}
		}
		if((IR_PulseCnt > 2) && (IR_Sta == 0x01))
		{
			IR_Code <<= 1;
			if((IR_PulseSub > 450) && (IR_PulseSub < 700))
			{
				IR_Code |= 0x00;
			}
			else if((IR_PulseSub > 800) && (IR_PulseSub < 1300))
			{
				IR_Code |= 0x01;
			}
			else
			{
				IR_Sta = 0;
				IR_Code = 0;
				IR_PulseCnt = 0;
			}
		}
		//如果下降沿次数已经达到34
		if(IR_PulseCnt == 34)
		{
			IR_Key = IR_Code;
			IR_Data = (uint8_t)(IR_Key>>8);
			IR_Sta = 0x02;//进入连发码状态
			IR_Flag = 1;	
      Alarm_count++;//蜂鸣器响一次			
		}
			
		//进入长按状态
		if((IR_PulseCnt == 40) && (IR_Sta == 0x02)) //220ms未抬起则连发
		{
			IR_PulseCnt = 34;
			if((IR_PulseSub > 4500) && (IR_PulseSub < 6000)) //连发脉宽11.5ms
			{
				LianfaCnt++;
				IR_Key = IR_Code;
				IR_Data = (uint8_t)(IR_Key>>8);
				IR_Flag = 1;
			}
			
		}		
		
	}						
	
}

void TIM1_UP_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM1, TIM_IT_Update) == SET)
	{
		TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
		if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_8) == 1)
		{
			IR_Up++;
			if(IR_Up >= 2)
			{
				IR_Code = 0;
				IR_Sta = 0;
				IR_PulseCnt = 0;
				LianfaCnt = 0;		
			}
		}					
	}
	
}
