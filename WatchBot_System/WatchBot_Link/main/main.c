#include <stdio.h>
#include "esp32_s3_szp.h"
#include "yingwu.h"
#include "logo_en_240x240_lcd.h"


void app_main(void)
{
    bsp_lcd_init();  // 液晶屏初始化
    //lcd_draw_pictrue(0, 0, 240, 240, logo_en_240x240_lcd); // 显示乐鑫logo图片
    lcd_draw_pictrue(0, 0, 320, 240, gImage_yingwu); // 显示3只鹦鹉图片
}







