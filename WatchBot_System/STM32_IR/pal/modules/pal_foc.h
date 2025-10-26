#include <stdint.h>
#include "pal_pid.h"

typedef struct{
	float Id_Kp; // Id电流PID控制器的Kp参数
	float Id_Ki; // Id电流PID控制器的Ki参数
	float Id_Lpf;
	
	float Iq_Kp; // Iq电流PID控制器的Kp参数
	float Iq_Ki; // Iq电流PID控制器的Ki参数
	float Iq_Lpf;
	
	float Velocity_Kp;
	float Velocity_Ki; 
	float Velocity_Lpf;
	
	float Angle_Kp;
	float Angle_Lpf;
} PalFOC_PIDParamsTypeDef;

typedef struct{
	void (*SetPWM)(float Ua, float Ub, float Uc);                           // 回调函数，由用户提供，用于设置UVW三相的PWM占空比（中心对齐）
	void (*UpdateAngleSensor)(void);
	void (*ReadAngleSensor)(float *pAngleOut);                              // 回调函数，由用户提供，用于获取电机的轴角度
	void (*UpdateCurrentSensor)(void);
	void (*ReadCurrentSensor)(float *pIaOut, float *pIbOut, float *pIcOut); // 回调函数，由用户提供，用于获取电机的相电流
} PalFOC_CallbacksTypeDef;

typedef struct{
	uint8_t PolePairs;                  // 极对数
	float PowerSupplyVoltage;           // 电机的供电电压
	float ZeroElectricAngle;            // 零电角度
	PalFOC_PIDParamsTypeDef PIDParams;
	PalFOC_CallbacksTypeDef Callbacks;
	float CurrentLimit;                 // 电流门限，过流后启用保护
} PalFOC_InitTypeDef;

typedef struct 
{
	float TargetVelocity;   // 开环速度设定值
	float LastAngle;        // 上次角度值
	uint64_t LastTimeStamp; // 上次时间戳
} PalFOC_VelOLHandleTypeDef;

typedef struct
{
	float Ud;
	float Uq;
	float theta;
} PalFOC_TorOLHandleTypeDef;

typedef struct{
	float TargetAngle;
} PalFOC_AngOLHandleTypeDef;

typedef struct{
	PalFOC_InitTypeDef Init;
	uint32_t ControlMode;              // 控制模式
	PalPID_HandleTypeDef hPIDId;       // Id PI控制
	PalLpf_HandleTypeDef hLpfId;
	PalPID_HandleTypeDef hPIDIq;       // Iq PI控制
	PalLpf_HandleTypeDef hLpfIq;
	PalPID_HandleTypeDef hPIDVel; // 转速 PI控制
	PalLpf_HandleTypeDef hLpfVel;
	PalPID_HandleTypeDef hPIDAng;    // 角度 P控制
	PalLpf_HandleTypeDef hLpfAng;
	PalFOC_VelOLHandleTypeDef hVelOL;  // 开环速度句柄
	PalFOC_AngOLHandleTypeDef hAngOL;  // 开环角度句柄
	PalFOC_TorOLHandleTypeDef hTorOL;
	uint64_t LastTime;
	float CenterVoltage; // 中心电压
	float LastAngle_Shaft;
	PalLpf_HandleTypeDef hLpfShaftAngleSpeed;
	float Ia_offset;
	float Ib_offset;
	float Ic_offset;
} PalFOC_HandleTypeDef;

 void PAL_FOC_Init(PalFOC_HandleTypeDef *Handle);
 void PAL_FOC_Proc(PalFOC_HandleTypeDef *Handle);
 void PAL_FOC_Disable(PalFOC_HandleTypeDef *Handle);

 void PAL_FOC_VelocityOpenLoop(PalFOC_HandleTypeDef *Handle, float Velocity);
 void PAL_FOC_AngleOpenLoop(PalFOC_HandleTypeDef *Handle, float Angle);
 void PAL_FOC_TorqueOpenLoop(PalFOC_HandleTypeDef *Handle, float Uq, float Ud, float Theta);
 void PAL_FOC_MotorCmd(PalFOC_HandleTypeDef *Handle, uint8_t NewState);
 void PAL_FOC_TorqueControl(PalFOC_HandleTypeDef *Handle, float TargetIq);
 void PAL_FOC_SpeedControl(PalFOC_HandleTypeDef *Handle, float TargetAngleSpeed);
 void PAL_FOC_AngleControl(PalFOC_HandleTypeDef *Handle, float TargetAngle);

 void PAL_FOC_TunePID_Iq(PalFOC_HandleTypeDef *Handle, float Kp, float Ki);
 void PAL_FOC_TunePID_Id(PalFOC_HandleTypeDef *Handle, float Kp, float Ki);
 void PAL_FOC_TunePID_Vel(PalFOC_HandleTypeDef *Handle, float Kp, float Ki);   
 void PAL_FOC_TunePID_Ang(PalFOC_HandleTypeDef *Handle, float Kp);   

float PAL_FOC_AlignZeroElectricAngle(PalFOC_HandleTypeDef *Handle);
 void PAL_FOC_AlignCurrentSensor(PalFOC_HandleTypeDef *Handle, float *pIaOffsetOut, float *pIbOffsetOut, float *pIcOffsetOut);
 void PAL_FOC_SetZeroElectricAngle(PalFOC_HandleTypeDef *Handle, float Angle);
