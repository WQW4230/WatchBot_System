#ifndef _MY_SD_DRIVER_H_
#define _MY_SD_DRIVER_H_

#define SD_PIN_CMD 38
#define SD_PIN_CLK 39
#define SD_PIN_D0  40

#define MOUNT_POINT              "/sdcard"
#define EXAMPLE_MAX_CHAR_SIZE    64

void My_SD_Init(void);

#endif
