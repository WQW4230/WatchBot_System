#ifndef _STM32F10x_PAL_DEVICE_UID_H_
#define _STM32F10x_PAL_DEVICE_UID_H_

#include "stm32f10x.h"

#define IS_DEVICE_UID_ANY(ID) ((ID)->UID_15_0 == 0xffff && (ID)->UID_31_16 == 0xffff && (ID)->UID_63_32 == 0xffffffff && (ID)->UID_95_64 == 0xffffffff)
#define IS_DEVICE_UID_NONE(ID) ((ID)->UID_15_0 == 0 && (ID)->UID_31_16 == 0 && (ID)->UID_63_32 == 0 && (ID)->UID_95_64 == 0)

#define IS_DEVICE_UID_EQU(ID1, ID2) ((ID1)->UID_15_0 == (ID2)->UID_15_0 && (ID1)->UID_31_16 == (ID2)->UID_31_16 && (ID1)->UID_63_32 == (ID2)->UID_63_32 && (ID1)->UID_95_64 == (ID2)->UID_95_64)

typedef struct
{
  uint16_t UID_15_0;
  uint16_t UID_31_16;
  uint32_t UID_63_32;
  uint32_t UID_95_64;
}PalDeviceUID;

//const PalDeviceUID DEVICE_UID_ANY = { 0xffff, 0xffff, 0xffffffff, 0xffffffff };
//const PalDeviceUID DEVICE_UID_NONE = { 0, 0, 0, 0 };

PalDeviceUID PAL_GetDeviceUID(void);
uint32_t PAL_GetDeviceFlashSize(void);
FlagStatus IsAnyDevice(PalDeviceUID DeviceUID);

#endif
