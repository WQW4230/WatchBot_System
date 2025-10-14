#include "stm32f10x_pal_device_uid.h"

PalDeviceUID PAL_GetDeviceUID(void)
{
	PalDeviceUID uid;
	uid.UID_15_0  = *((uint16_t *)(0x1ffff7e8 + 0x00));
	uid.UID_31_16 = *((uint16_t *)(0x1ffff7e8 + 0x02));
	uid.UID_63_32 = *((uint32_t *)(0x1ffff7e8 + 0x04));
	uid.UID_95_64 = *((uint32_t *)(0x1ffff7e8 + 0x08));
	return uid;
}

uint32_t PAL_GetDeviceFlashSize(void)
{
	return ((uint32_t)*((uint16_t *)0x1ffff7e0))*1024;
}
