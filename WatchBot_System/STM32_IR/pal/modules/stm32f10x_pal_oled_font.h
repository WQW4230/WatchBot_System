/**
  ******************************************************************************
  * @file    stm32f10x_pal_oled_font.h
  * @author  铁头山羊stm32工作组
  * @version V1.0.0
  * @date    2023年2月24日
  * @brief   oled字体
  ******************************************************************************
  * @attention
  * Copyright (c) 2022 - 2023 东莞市明玉科技有限公司. 保留所有权力.
  ******************************************************************************
*/

#ifndef _STM32F10x_PAL_OLED_FONT_H_
#define _STM32F10x_PAL_OLED_FONT_H_

#include "stm32f10x.h"

typedef struct
{
	const char *Name; // 字形名称
	uint32_t Encoding; // unicode编码
	
	uint16_t Swx0; // 可伸缩宽度
	uint16_t Swy0; // 可伸缩高度
	uint16_t Dwx0; // 设备宽度
	uint16_t Dwy0; // 设备高度
	
	uint16_t Swx1; // 可伸缩宽度
	uint16_t Swy1; // 可伸缩高度
	uint16_t Dwx1; // 设备宽度
	uint16_t Dwy1; // 设备高度
	
	uint16_t VVectorXoff; // 原点0到原点1的向量横坐标
	uint16_t VVectorYoff; // 原点0到原点1的向量纵坐标
	
	uint16_t BBw; // 像素区域宽度
	uint16_t BBh; // 像素区域高度
	int16_t BBxoff0x; // 像素区域左下角的横坐标
	int16_t BByoff0y; // 像素区域左下角的纵坐标
	
	const uint16_t nBytes; // 位图字节数
	const uint8_t *Bitmap; // 字形的位图数据
}PalGlyph_TypeDef;

typedef struct
{
	const char * SpecVersion; // bdf规范版本号
	const char * FontName; // 字体名称
	uint16_t ContentVersion; // 字体版本号
	uint8_t MetricsSet; // 书写方向，0-左到右，1-右到左，2-混合
	uint16_t FontSize; // 字号（磅）
	uint16_t Xres; // 横向分辨率(DPI)
	uint16_t Yres; // 纵向分辨率(DPI)
	uint16_t FBBx; // 最大边框宽度
	uint16_t FBBy; // 最大边框高度
	int16_t FBBXoff; // 最大边框横坐标
	int16_t FBBYoff; // 最大边框纵坐标
	uint16_t nChars; // 包含的字形数量
	const uint32_t *Map;
	const PalGlyph_TypeDef *Glyphs; // 字形数据
}PalFont_TypeDef;



#endif
