#ifndef __OLED_H
#define __OLED_H

void OLED_WriteCommand(uint8_t Command);
void OLED_WriteData(uint8_t Data);
void OLED_Init(void);
void OLED_SetCursor(uint8_t X, uint8_t Page);
void OLED_Clear(void);
void OLED_ShowChar(uint8_t X, uint8_t Page, char Char, uint8_t FontSize);
void OLED_ShowString(uint8_t X, uint8_t Page, char *String, uint8_t FontSize);
void OLED_ShowImage(uint8_t X, uint8_t Page, uint8_t Width, uint8_t Height, const uint8_t *Image);
void OLED_ShowChinese(uint8_t X, uint8_t Page, char *Chinese);

#endif
