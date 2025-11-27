#ifndef _CAMERA_SERVICE_H_
#define _CAMERA_SERVICE_H_

#include "stm32f10x.h" 
#define CAPTURE_TIME_INTERVAL 5000 //每次拍照时间间隔ms

void camera_service_proc(void);

#endif
