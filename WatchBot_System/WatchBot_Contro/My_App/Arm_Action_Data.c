#include "Arm_Action_Data.h"

//动作1 快速机动
const Arm_ActionAngle_t QuickTravel_Action[] = 
{
	{  0,   0,   0, 100, 1000},
	{-90, -50, -90, 100, 1300},
	{ 90,  60, -90, 100, 1300},
	{-90, -90,   0, 100, 1000},
	{ 90,  90,   0, 100, 1000},
	{  0, -90, -90, 100, 1000},
	{  0,  90,  90, 100, 1000},
	{  0,   0, -90, 100, 1000},
	{  0,   0,  90, 100, 1000},
	{  0,   0, -90, 100, 1000},
	{  0,   0,  90, 100, 1000},
};
//const uint16_t QuickTravel_Num = sizeof(QuickTravel_Action) / sizeof(QuickTravel_Action[0]);

//动作2 鞠躬
const Arm_ActionAngle_t Bow_Action[] =
{
	{-90,   0, -90, 100, 1000},
	{-80,   0,  90, 100, 1000},
	{-70,   0, -90, 100, 1000},
	{-60,   0,  90, 100, 1000},
	{-50,   0, -90, 100, 1000},
	{-40,   0,  90, 100, 1000},
	{-30,   0, -90, 100, 1000},
	{-20,   0,  90, 100, 1000},
	{-10,   0, -90, 100, 1000},
	{  0,   0,  90, 100, 1000},
	{ 10,   0, -90, 100, 1000},
	{ 20,   0,  90, 100, 1000},
	{ 30,   0, -90, 100, 1000},
	{ 40,   0,  90, 100, 1000},
	{ 50,   0, -90, 100, 1000},
	{ 60,   0,  90, 100, 1000},
	{ 70,   0, -90, 100, 1000},
	{ 80,   0,  90, 100, 1000},
	{ 90,   0, -90, 100, 1000},
	
};
//const uint16_t Bow_Num = sizeof(Bow_Action) / sizeof(Bow_Action[0]);

//动作3


const Arm_ActionaGroup Actiona_Table[ARM_ACTIONS_NUM] =
{
	{"QuickTravel", QuickTravel_Action, QuickTravel_Num},
	{"Bow", Bow_Action, Bow_Num},
};

