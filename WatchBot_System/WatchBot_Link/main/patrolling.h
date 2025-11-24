#ifndef _PATROLLING_H_
#define _PATROLLING_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

//最大角度
#define PATROL_PAN_MAX    90
#define PATROL_PAN_MIN   -90
#define PATROL_ROLL_MAX   30
#define PATROL_ROLL_MIN  -30
#define PATROL_TILT_MAX   40
#define PATROL_TILT_MIN  -40
//巡逻步幅
#define PATROL_STEP_PAN    1.0f
#define PATROL_STEP_ROLL   1.0f
#define PATROL_STEP_TILT   1.0f
//下次动作的时间
#define PATROL_STEP_TIME 100

typedef enum
{
    positive =  1, //正序
    negative = -1, //反序
}action_sequence_e;

typedef struct
{
	float Pan_Angle;    //基座偏航角
	float Roll_Angle; 	//大臂翻滚角
	float Tilt_Angle;	//末端俯仰角
	float Fan_Speed;    //风扇转速
	uint16_t Duration;  //动作持续时间
}Arm_ActionAngle_t;

void patrolling_init(void);
void patrolling_off(void);
void patrolling_on(void);


#ifdef __cplusplus
}
#endif

#endif
