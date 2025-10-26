#include "NEC_driver.h"
#include <stddef.h>

//NEC协议的全局句柄！！！！！！！！！
static IR_NEC_HandleTypedef IR_NEC;

typedef struct
{
	
	uint8_t IR_KeyData;//按键编码
	Key_Enum KeyEnum;//按键枚举
	
}IR_KeyStruct;

static IR_KeyStruct Key_arr[18] = 
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
	{0x00, Key_ERROR},
	{0x68, Key_XingHao},
	{0xB0, Key_JingHao},
	{0x18, Key_W},
	{0x4A, Key_S},
	{0x10, Key_A},
	{0x5A, Key_D},
	{0x38, Key_OK},		
};

//当前帧错误需要销毁
static void NEC_ClearError(IR_NEC_HandleTypedef *IR_NEC)
{
	IR_NEC->FullCode    = 0;              //完整数据
	IR_NEC->PulseCount  = 0;							//数据计数到几位
	IR_NEC->State       = NEC_STATE_IDLE; //回到接收状态
	IR_NEC->RepeatCount = 0;							//连发状态
}

//地址位 数据位校验
//当完整的接收到一帧以后才会进行校验，如若地址位正反码、数据位正反码、地址位和用户码对不上则直接丢弃这一帧
//可用的时候将Fla标志位置位
static void NEC_CheckCode(IR_NEC_HandleTypedef *IR_NEC)
{
	IR_NEC->Addres =    (uint8_t)(IR_NEC->FullCode>>24);
	uint8_t AddresInv = (uint8_t)(IR_NEC->FullCode>>16);
	IR_NEC->Data =      (uint8_t)(IR_NEC->FullCode>>8);
	uint8_t DataInv =   (uint8_t)(IR_NEC->FullCode);
	//   地址位数据位无错误                         数据位无错误                         用户码校验
	if((IR_NEC->Addres ^ AddresInv) == 0xff && (IR_NEC->Data ^ DataInv) == 0xff && IR_NEC->Addres == ADDRES)
	{
		IR_NEC->Flag  = 1; //当前数据可用
		IR_NEC->State = NEC_STATE_REPEAT; //进入连发码状态
	}
	else
	{
		//数据校验失败，清除数据
		NEC_ClearError(IR_NEC);
	}	
}

//计算脉宽，直接调用放入句柄即可
static void IR_CalcPulseWidth(IR_NEC_HandleTypedef *IR_NEC)
{
	IR_NEC->KeyUpFlag = 0;
	IR_NEC->ThisCapture = TIM_GetCapture1(TIM1);
	
	//得到脉宽
	if(IR_NEC->ThisCapture > IR_NEC->LastCapture)
	{
		//为发生溢出则直接相减
		IR_NEC->PulseWidth = IR_NEC->ThisCapture - IR_NEC->LastCapture;
	}
	else
	{
		//发生溢出的时候
		IR_NEC->PulseWidth = IR_NEC->ThisCapture + (0xffff - IR_NEC->LastCapture);
	}
	
	IR_NEC->LastCapture = IR_NEC->ThisCapture;
	IR_NEC->PulseCount++;
}

//空闲状态时候 等待起始位
static void NEC_State_IDLE(IR_NEC_HandleTypedef *IR_NEC)
{	
	uint16_t PulseCount = IR_NEC->PulseCount;
	uint16_t PulseWidth = IR_NEC->PulseWidth;
	
	// 两个下降沿到来时说明起始位已经完整发送
	if(PulseCount == 2)
	{
		//判断是不是起始位
		if(PulseWidth > 6000 && PulseWidth < 8000)
		{
			IR_NEC->State = NEC_STATE_RECEIVING;
		}
		else
		{
			//错误，清除当前数据帧
			NEC_ClearError(IR_NEC);
		}
	}

}

//接收状态时候 等待数据位发送完成
static void NEC_State_Receive(IR_NEC_HandleTypedef *IR_NEC)
{
	uint16_t PulseCount = IR_NEC->PulseCount;
	uint16_t PulseWidth = IR_NEC->PulseWidth;	
	
	//小端向左一位填充数据位
	if(PulseCount > 2)
	{
		IR_NEC->FullCode <<= 1;
		
		//该分辨率下的值为逻辑0
		if(PulseWidth > 450 && PulseWidth < 700)
		{
			IR_NEC->FullCode |= 0x00;
		}
		//该分辨率下的值为逻辑1
		else if(PulseWidth > 800 && PulseWidth < 1300)
		{
			IR_NEC->FullCode |= 0x01;
		}
		//错误全部清除
		else
		{
			NEC_ClearError(IR_NEC);
		}
		
		//一帧数据已经完整接收
		//进行数据校验是否正确
		if(PulseCount == 34)
		{
			NEC_CheckCode(IR_NEC);
		}
	}
		
}

//进入连发模式
static void NEC_State_Repeat(IR_NEC_HandleTypedef *IR_NEC)
{
	if(IR_NEC->PulseCount == 40) //330ms未抬起则连发
	{
		uint16_t PulseWidth = IR_NEC->PulseWidth;
		IR_NEC->Flag  = 1; //当前数据可用
		IR_NEC->PulseCount = 34;
		
		if(PulseWidth > 4500 && PulseWidth < 6000)
		{
			IR_NEC->RepeatCount++;
		}
	}
}

//
//起始位 逻辑01分析
static void NEC_CaptureEdge(IR_NEC_HandleTypedef *IR_NEC)
{
	//计算脉宽
	IR_CalcPulseWidth(IR_NEC);
	
	switch(IR_NEC->State)
	{
		//等待起始位
		case NEC_STATE_IDLE:
			NEC_State_IDLE(IR_NEC);
		
			break;
		//接收数据中
		case NEC_STATE_RECEIVING:
			NEC_State_Receive(IR_NEC);
			
			break;
		//进入连发模式
		case NEC_STATE_REPEAT:
			NEC_State_Repeat(IR_NEC);
		
			break;
	}
	
}

void NEC_Init(void)
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

//获取当前按钮值 存入key中
//如果接收到红外信号，并且读取该函数会返回@Key_Enum中的值
//如果没有收到红外信号，则返回-1 有信号则返回 1
int IR_GetKey(uint8_t *key)
{
	if(key == NULL) return -1;
	if(IR_NEC.Flag == 1)
	{
		IR_NEC.Flag = 0;
		for(uint16_t i=0; i<(sizeof(Key_arr) / sizeof(Key_arr[0])); i++)
		{
			if(IR_NEC.Data == Key_arr[i].IR_KeyData)
			{
				*key = Key_arr[i].KeyEnum;
				return 1;
			}
		}
	}
	return -1;
}

//中断入口
void TIM1_CC_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM1, TIM_IT_CC1) == SET)
	{
		//清除中断
		TIM_ClearITPendingBit(TIM1, TIM_IT_CC1);
		NEC_CaptureEdge(&IR_NEC);
		
		
	}						
	
}

void TIM1_UP_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM1, TIM_IT_Update) == SET)
	{
		TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
		if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_8) == 1)
		{
			IR_NEC.KeyUpFlag++;
			if(IR_NEC.KeyUpFlag >= 2)
			{
				NEC_ClearError(&IR_NEC);
			}
		}					
	}
	
}
