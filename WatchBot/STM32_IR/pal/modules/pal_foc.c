#include <math.h>
#include "stm32f10x_pal.h"
#include "pal_foc.h"

#define FOC_CONTROL_MODE_DISABLE       0x00 // 禁止
#define FOC_CONTROL_MODE_ANG_OPEN_LOOP 0x01 // 开环角度控制
#define FOC_CONTROL_MODE_VEL_OPEN_LOOP 0x02 // 开环速度控制
#define FOC_CONTROL_MODE_TOR_OPEN_LOOP 0x06 // 开环力矩控制
#define FOC_CONTROL_MODE_TORQUE        0x03 // 力矩控制
#define FOC_CONTROL_MODE_VEL           0x04 // 闭环速度控制
#define FOC_CONTROL_MODE_ANG           0x05 // 闭环位置控制

#define DEF_CURRENT_LIMIT              5.0f // 默认最大电流，超出后保护

static void SetPhaseVoltage(PalFOC_HandleTypeDef *Handle, float Uq, float Ud, float Angle_el);
static void CalibrateCurrentOffset(PalFOC_HandleTypeDef *Handle);

void PAL_FOC_Init(PalFOC_HandleTypeDef *Handle)
{
	Handle->ControlMode = FOC_CONTROL_MODE_DISABLE;
	
	// 初始化PID控制器
	// Id - PI控制
	Handle->hPIDId.Init.Kp = Handle->Init.PIDParams.Id_Kp;
	Handle->hPIDId.Init.Ki = Handle->Init.PIDParams.Id_Ki;
	Handle->hPIDId.Init.Kd = 0;
	Handle->hPIDId.Init.Setpoint = 0;
	Handle->hPIDId.Init.DefaultOutput = 0;
	Handle->hPIDId.Init.OutputUpperLimit =  Handle->Init.PowerSupplyVoltage / 5;
	Handle->hPIDId.Init.OutputLowerLimit = -Handle->Init.PowerSupplyVoltage / 5;
	PAL_PID_Init(&Handle->hPIDId);
	
	Handle->hLpfId.Init.Tf = Handle->Init.PIDParams.Id_Lpf;
	PAL_Lpf_Init(&Handle->hLpfId);
	
	// Iq - 控制
	Handle->hPIDIq.Init.Kp = Handle->Init.PIDParams.Iq_Kp;
	Handle->hPIDIq.Init.Ki = Handle->Init.PIDParams.Iq_Ki;
	Handle->hPIDIq.Init.Kd = 0;
	Handle->hPIDIq.Init.Setpoint = 0;
	Handle->hPIDIq.Init.DefaultOutput = 0;
	Handle->hPIDIq.Init.OutputUpperLimit =  Handle->Init.PowerSupplyVoltage / 5;
	Handle->hPIDIq.Init.OutputLowerLimit = -Handle->Init.PowerSupplyVoltage / 5;
	PAL_PID_Init(&Handle->hPIDIq);
	
	Handle->hLpfIq.Init.Tf = Handle->Init.PIDParams.Iq_Lpf;
	PAL_Lpf_Init(&Handle->hLpfIq);
	
	if(Handle->Init.CurrentLimit == 0)
	{
		Handle->Init.CurrentLimit = DEF_CURRENT_LIMIT;
	}
	
	Handle->CenterVoltage = Handle->Init.PowerSupplyVoltage / 2;
	
	Handle->hLpfShaftAngleSpeed.Init.Tf = 0.05;
	PAL_Lpf_Init(&Handle->hLpfShaftAngleSpeed);
	
	CalibrateCurrentOffset(Handle);
}  

#define _PI_2    1.5707963267948966192313216916398
#define _PI_3    1.0471975511965977461542144610932
#define _PI      3.1415926535897932384626433832795
#define _2PI     6.283185307179586476925286766559
#define _3PI_2   4.7123889803846898576939650749193
#define _SQRT3   1.7320508075688772935274463415059
#define _SQRT3_2 0.86602540378443864676372317075294
#define _2_SQRT3 1.1547005383792515290182975610039
#define _1_SQRT3 0.57735026918962576450914878050196

//
// @简介：FOC控制算法进程函数
// @注意：该方法需要在主函数main的while循环中被调用
//
void PAL_FOC_Proc(PalFOC_HandleTypeDef *Handle)
{
	Handle->Init.Callbacks.UpdateCurrentSensor();
	Handle->Init.Callbacks.UpdateAngleSensor();
	//
	// 运行时间检测：
	// FOC全功能运行一次耗时1.5ms
	// 
	//	static uint64_t lastTime = 0;
	//	uint64_t now = PAL_GetUs();
	//	uint32_t deltaT = now - lastTime;
	////	App_BT_Printf("$%ud;", (uint32_t)(now - lastTime));
	//	lastTime = now;
	uint64_t now = PAL_GetUs();
	
	//
	// 角度传感器检测
	// 实验证明TLE5012b角度传感器有2.75ms左右的滞后
	// 
	//	Uq     theta_m  theta_e  speed_m      Δt
	//	0.3     4.09     28.66    1604      2.551ms
	//	0.4     6.55     45.86    2292      2.857ms
	//	0.5     9.08     63.57    3037      2.965ms
	//	0.6    10.65     74.52    3611      2.948ms
	//	0.7    11.46     80.25    4183      2.740ms
	//  0.8    11.87     83.08    4870      2.437ms
	//  0.9    13.51     94.54    5443      2.482ms
	//  1.0    15.55    108.86    5959      2.609ms
	//               平均     2.804ms
	//
//	App_BT_Printf("$%u;", (uint32_t)(PAL_GetUs() - start)); // 5
	
	// 读取电流传感器
	float Ia, Ib, Ic;
	
	Handle->Init.Callbacks.ReadCurrentSensor(&Ia, &Ib, &Ic); // Δt=110us
	
	Ia -= Handle->Ia_offset;
	Ib -= Handle->Ib_offset;
	Ic -= Handle->Ic_offset;
	
//	App_BT_Printf("$%u;", (uint32_t)(PAL_GetUs() - start)); // 115
	
	
	// 通过角度传感器读取电机轴的角度和角速度
	float angle_shaft, angleSpeed_shaft;
	
	Handle->Init.Callbacks.ReadAngleSensor(&angle_shaft); // Δt=365us
	
//	App_BT_WaveformPrintf("$%.3f;", angle_shaft);
	
	// 计算角速度
	float deltaT = (now - Handle->LastTime) * 1e-6;
	
	angleSpeed_shaft = angle_shaft - Handle->LastAngle_Shaft;
	
	if(angleSpeed_shaft >  _PI) angleSpeed_shaft -= _2PI;
	if(angleSpeed_shaft < -_PI) angleSpeed_shaft += _2PI;
	
	angleSpeed_shaft /= deltaT;
	
	angleSpeed_shaft = PAL_Lpf_Calc(&Handle->hLpfShaftAngleSpeed, angleSpeed_shaft);
	
	Handle->LastAngle_Shaft = angle_shaft;
	
//	App_BT_Printf("$%f;", angleSpeed_shaft);
	
	
//	App_BT_Printf("$%u;", (uint32_t)(PAL_GetUs() - start)); // 475
	
	// 过流保护
	if(  fabsf(Ia) > Handle->Init.CurrentLimit 
		|| fabsf(Ib) > Handle->Init.CurrentLimit 
	  || fabsf(Ic) > Handle->Init.CurrentLimit) 
	{
		Handle->ControlMode = FOC_CONTROL_MODE_DISABLE; 
	}
	
	//
	// 电机禁止时直接关闭PWM输出
	// 
	if(Handle->ControlMode == FOC_CONTROL_MODE_DISABLE // 电机禁止
	   || (Handle->ControlMode == FOC_CONTROL_MODE_VEL_OPEN_LOOP && Handle->hVelOL.TargetVelocity < 1)) // 或者开环速度小于1
	{
		Handle->Init.Callbacks.SetPWM(0, 0, 0);
		
		return;
	}
	
//	App_BT_Printf("$%u;", (uint32_t)(PAL_GetUs() - start)); // 475
	
	float angle_el;
	
	//
	// 计算当前电角度
	// 
	if(Handle->ControlMode == FOC_CONTROL_MODE_VEL_OPEN_LOOP)
	{
		// 如果速度开环，则当前电角度手动递增
		
		uint64_t now = PAL_GetUs();
		
		float target_angle_shaft = Handle->hVelOL.LastAngle + (now - Handle->hVelOL.LastTimeStamp) * 1e-6 * Handle->hVelOL.TargetVelocity;
		
		while(target_angle_shaft > _2PI) target_angle_shaft -= _2PI; 
		
		// 将轴角度转换为电角度
		angle_el = target_angle_shaft * Handle->Init.PolePairs - Handle->Init.ZeroElectricAngle;
		
		Handle->hVelOL.LastTimeStamp = now;
		Handle->hVelOL.LastAngle = target_angle_shaft;
	}
	else
	{
		// 否则根据传感器的实际值计算电角度
		angle_el = angle_shaft * Handle->Init.PolePairs - Handle->Init.ZeroElectricAngle + Handle->hTorOL.theta;
		// angle_el = angle_shaft * Handle->Init.PolePairs - Handle->Init.ZeroElectricAngle + Handle->hTorOL.theta + 2.75e-3 * angleSpeed_shaft * 7;
	}

//	angle_el = fmod(angle_el, _2PI); // 归一化便于显示
//	if(angle_el < 0) angle_el += _2PI;
	
	float sin_theta = sin(angle_el);
	float cos_theta = cos(angle_el);
	
	// App_BT_Printf("$%u;", (uint32_t)(PAL_GetUs() - start)); // 505
	
	//
	// 计算Ud和Uq
	//
	float Uq, Ud;
	
	if(Handle->ControlMode == FOC_CONTROL_MODE_VEL_OPEN_LOOP)
	{
		Uq = Handle->Init.PowerSupplyVoltage / 20;
		Ud = 0;
	}
	else
	{
		/////////////////////////////////////////////////////////////////
		// 位置环
		/////////////////////////////////////////////////////////////////	
		float vel_ref;
		
		vel_ref = PAL_PID_Compute1(&Handle->hPIDAng, angle_shaft);
		
		PAL_PID_ChangeSetpoint(&Handle->hPIDVel, vel_ref);
		
		/////////////////////////////////////////////////////////////////
		// 速度环
		/////////////////////////////////////////////////////////////////	
		float Iq_ref;
		
		Iq_ref = PAL_PID_Compute1(&Handle->hPIDVel, angleSpeed_shaft);
		
		PAL_PID_ChangeSetpoint(&Handle->hPIDIq, Iq_ref);
		
		/////////////////////////////////////////////////////////////////
		// 力矩环（电流环）
		/////////////////////////////////////////////////////////////////
		
		// Clark变换
		float Ialpha, Ibeta;
		
		Ialpha = Ia;
		Ibeta = _1_SQRT3 * Ia + _2_SQRT3 * Ib;
		
		// Park变换
		float Id, Iq;
		
		Id =  Ialpha * cos_theta + Ibeta * sin_theta;
		Iq = -Ialpha * sin_theta + Ibeta * cos_theta;
		
		// 滤波
		Id = PAL_Lpf_Calc(&Handle->hLpfId, Id);
		Iq = PAL_Lpf_Calc(&Handle->hLpfIq, Iq);
		
//		App_BT_WaveformPrintf("$%.3f %.3f %.3f;", angleSpeed_shaft / 10, Id, Iq);
//		App_BT_WaveformPrintf("%.3f %.3f", Id, Iq);
		
		// 将Id和Iq输入PID控制器，从而计算Ud和Uq
		Uq = PAL_PID_Compute1(&Handle->hPIDIq, Iq);
		Ud = PAL_PID_Compute1(&Handle->hPIDId, Id);
		
		if(Handle->ControlMode == FOC_CONTROL_MODE_TOR_OPEN_LOOP)
		{
			Uq = Handle->hTorOL.Uq;
			Ud = Handle->hTorOL.Ud;
		}
		else
		{
			Ud = 0;
		}
	}
	
//	App_BT_WaveformPrintf("$%u;", (uint32_t)(PAL_GetUs() - start)); // 690us
	
	// 帕克逆变换
	float Ualpha, Ubeta;
	
	Ualpha = -sin_theta * Uq + cos_theta * Ud;
	Ubeta  =  cos_theta * Uq + sin_theta * Ud;
	
	// 克拉克逆变换
	float Ua, Ub, Uc;
	
	Ua = Ualpha + Handle->CenterVoltage;
	Ub = -0.5 * Ualpha + _SQRT3_2 * Ubeta + Handle->CenterVoltage;
	Uc = -0.5 * Ualpha - _SQRT3_2 * Ubeta + Handle->CenterVoltage;
	
	// 设置PWM
	Handle->Init.Callbacks.SetPWM(Ua, Ub, Uc);
//	Handle->Init.Callbacks.SetPWM(0, 0, 0);
	
//	 App_BT_Printf("$%u;", (uint32_t)(PAL_GetUs() - start)); // 810 us
	
//	App_BT_WaveformPrintf("$%.3f %.3f %.3f %.3f %.3f %.3f %.3f;", Ia, Ib, Ic, Ua - 3.7, Ub - 3.7, Uc - 3.7);
//  App_BT_WaveformPrintf("$%.3f %.3f %.3f;", Ia, Ib, Ic);
//	App_BT_WaveformPrintf("$%.3f %.3f %.3f;", Ua, Ub, Uc);
  Handle->LastTime = now;
}

void PAL_FOC_Disable(PalFOC_HandleTypeDef *Handle)
{
	Handle->ControlMode = FOC_CONTROL_MODE_DISABLE;
}

void PAL_FOC_TunePID_Iq(PalFOC_HandleTypeDef *Handle, float Kp, float Ki)
{
	PAL_PID_ChangeTunings(&Handle->hPIDIq, Kp, Ki, 0);
}

void PAL_FOC_TunePID_Id(PalFOC_HandleTypeDef *Handle, float Kp, float Ki)
{
	PAL_PID_ChangeTunings(&Handle->hPIDId, Kp, Ki, 0);
}

void PAL_FOC_TunePID_Vel(PalFOC_HandleTypeDef *Handle, float Kp, float Ki)
{
	PAL_PID_ChangeTunings(&Handle->hPIDVel, Kp, Ki, 0);
}
void PAL_FOC_TunePID_Ang(PalFOC_HandleTypeDef *Handle, float Kp)
{
	PAL_PID_ChangeTunings(&Handle->hPIDAng, Kp, 0, 0);
}

//
// @简介：控制电机转动力矩
// 
void PAL_FOC_TorqueControl(PalFOC_HandleTypeDef *Handle, float TargetIq)
{
	Handle->ControlMode = FOC_CONTROL_MODE_TORQUE;
	
	PAL_PID_Cmd(&Handle->hPIDAng, 0); // 位置环手动
	PAL_PID_Cmd(&Handle->hPIDVel, 0); // 速度环手动
	PAL_PID_Cmd(&Handle->hPIDIq, 1); // Iq闭环
	PAL_PID_Cmd(&Handle->hPIDId, 1); // Id闭环
	
	// 设置目标力矩
	PAL_PID_ChangeManualOutput(&Handle->hPIDVel, TargetIq);
}

//
// @简介：控制电机转速
// 
void PAL_FOC_SpeedControl(PalFOC_HandleTypeDef *Handle, float TargetAngleSpeed)
{
	Handle->ControlMode = FOC_CONTROL_MODE_VEL;
	
	PAL_PID_Cmd(&Handle->hPIDAng, 0); // 位置环手动
	PAL_PID_Cmd(&Handle->hPIDVel, 1); // 速度闭环
	PAL_PID_Cmd(&Handle->hPIDIq, 1); // Iq闭环
	PAL_PID_Cmd(&Handle->hPIDId, 1); // Id闭环
	
	// 设置目标转速
	PAL_PID_ChangeSetpoint(&Handle->hPIDVel, TargetAngleSpeed);
}

//
// @简介：控制电机转角
// 
void PAL_FOC_AngleControl(PalFOC_HandleTypeDef *Handle, float TargetAngle)
{
	Handle->ControlMode = FOC_CONTROL_MODE_ANG;
	
	PAL_PID_Cmd(&Handle->hPIDAng, 1); // 位置闭环
	PAL_PID_Cmd(&Handle->hPIDVel, 1); // 速度闭环
	PAL_PID_Cmd(&Handle->hPIDIq, 1); // Iq闭环
	PAL_PID_Cmd(&Handle->hPIDId, 1); // Id闭环
	
	// 设置目标位置
	PAL_PID_ChangeSetpoint(&Handle->hPIDAng, TargetAngle);
}

void PAL_FOC_VelocityOpenLoop(PalFOC_HandleTypeDef *Handle, float Velocity)
{
	Handle->ControlMode = FOC_CONTROL_MODE_VEL_OPEN_LOOP;
	Handle->hVelOL.TargetVelocity = Velocity;
	Handle->hVelOL.LastTimeStamp = PAL_GetUs();
	Handle->hVelOL.LastAngle = 0;
}

void PAL_FOC_SetZeroElectricAngle(PalFOC_HandleTypeDef *Handle, float Angle)
{
	Handle->Init.ZeroElectricAngle = Angle;
}

void PAL_FOC_AngleOpenLoop(PalFOC_HandleTypeDef *Handle, float Angle)
{
	
}

void PAL_FOC_TorqueOpenLoop(PalFOC_HandleTypeDef *Handle, float Uq, float Ud, float Theta)
{
	Handle->hTorOL.Ud = Ud;
	Handle->hTorOL.Uq = Uq;
	Handle->hTorOL.theta = Theta;
	Handle->ControlMode = FOC_CONTROL_MODE_TOR_OPEN_LOOP;
}

void PAL_FOC_AlignCurrentSensor(PalFOC_HandleTypeDef *Handle, float *pIaOffsetOut, float *pIbOffsetOut, float *pIcOffsetOut)
{
	SetPhaseVoltage(Handle, 0, 0, 0);
	
	
}

float PAL_FOC_AlignZeroElectricAngle(PalFOC_HandleTypeDef *Handle)
{
	float zeroElectricAngle;
	float shaftAngle;
	
	// zero electric angle not known
	// align the electrical phases of the motor and sensor
	// set angle -90(270 = 3PI/2) degrees
	SetPhaseVoltage(Handle, Handle->Init.PowerSupplyVoltage / 3, 0, _3PI_2);
	PAL_Delay(700);
	
	// read the sensor
	Handle->Init.Callbacks.ReadAngleSensor(&shaftAngle);
	
	SetPhaseVoltage(Handle, 0, 0, 0);
	
	// get the current zero electric angle
	zeroElectricAngle = shaftAngle * Handle->Init.PolePairs;
	
	// normalize
	if(zeroElectricAngle < 0 || zeroElectricAngle > _2PI)
	{
		zeroElectricAngle = fmod(zeroElectricAngle, _2PI);
		
		if(zeroElectricAngle < 0) 
			zeroElectricAngle += _2PI;
	}
	
	PAL_Delay(200);
	
	return zeroElectricAngle;
}

static void SetPhaseVoltage(PalFOC_HandleTypeDef *Handle, float Uq, float Ud, float Angle_el)
{
	// 反Park变换
	float Ualpha, Ubeta;
	
	Ualpha = -sin(Angle_el) * Uq;
	Ubeta  =  cos(Angle_el) * Uq;
	
	// 反Clark变换
	float Ua, Ub, Uc;
	
	Ua = Ualpha + Handle->CenterVoltage;
	Ub = -0.5 * Ualpha + _SQRT3_2 * Ubeta + Handle->CenterVoltage;
	Uc = -0.5 * Ualpha - _SQRT3_2 * Ubeta + Handle->CenterVoltage;
	
	// 设置PWM
	Handle->Init.Callbacks.SetPWM(Ua, Ub, Uc);
}

static void CalibrateCurrentOffset(PalFOC_HandleTypeDef *Handle)
{
	Handle->Init.Callbacks.SetPWM(0,0,0);
	
	uint32_t i;
	
	float Ia, Ib, Ic;
	
	Handle->Ia_offset = 0;
	Handle->Ib_offset = 0;
	Handle->Ic_offset = 0;
	
	PAL_Delay(3); 
	
	for(i=0;i<100;i++)
	{
		Handle->Init.Callbacks.UpdateCurrentSensor();
		
		Handle->Init.Callbacks.ReadCurrentSensor(&Ia, &Ib, &Ic);
		
		Handle->Ia_offset += Ia;
		Handle->Ib_offset += Ib;
		Handle->Ic_offset += Ic;
		
		PAL_Delay(3);    
	}
	
	Handle->Ia_offset /= 100.0;
	Handle->Ib_offset /= 100.0;
	Handle->Ic_offset /= 100.0;
}
