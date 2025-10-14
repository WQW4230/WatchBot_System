/**
  ******************************************************************************
  * @file    user_persistance.h
  * @author  铁头山羊
  * @version V 1.0.0
  * @date    2023年4月24日
  * @brief   参数持久化
  ******************************************************************************
  * @attention
  * Copyright (c) 2022 - 2023 东莞市明玉科技有限公司. 保留所有权力.
  ******************************************************************************
*/
#ifndef _USER_PERSISTANCE_H_
#define _USER_PERSISTANCE_H_

#include "stm32f10x.h"

#define PREDEFINED_INITIALIZE_TOKEN 0x11ca24e2

typedef struct
{
	float Kp;
	float Ki;
	float Kd;
	float Sp;
	FlagStatus Manual;
	float ManualOutput;
}PIDPersist_TypeDef;

typedef struct
{
	uint32_t InitializeToken;
	float MechanicalAngle;
	PIDPersist_TypeDef BalancePID;
	PIDPersist_TypeDef SpeedPID;
} AppPersist_TypeDef;

void User_Persist_ReadFromFlash(AppPersist_TypeDef *pStruct);
void User_Persist_WriteToFlash(AppPersist_TypeDef *pStruct);

#endif
