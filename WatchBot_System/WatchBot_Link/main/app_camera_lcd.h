#ifndef _APP_CAMERA_LCD_H_
#define _APP_CAMERA_LCD_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


void app_camera_lcd_init(void);
// void app_camera_lcd(void);
void app_capture(void);
void bsp_camera_init(void);
// 液晶屏显示内容，方便外部文件调用
void lcd_draw_bitmap(int x_start, int y_start, int x_end, int y_end, const void *color_data);

#ifdef __cplusplus
}
#endif

#endif // _APP_CAMERA_LCD_H_
