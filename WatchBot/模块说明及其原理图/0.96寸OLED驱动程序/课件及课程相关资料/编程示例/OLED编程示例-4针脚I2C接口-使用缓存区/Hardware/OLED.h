#ifndef __OLED_H
#define __OLED_H

void OLED_WriteCommand(uint8_t Command);
void OLED_WriteData(uint8_t *Data, uint8_t Count);
void OLED_Init(void);
void OLED_SetCursor(uint8_t X, uint8_t Page);
void OLED_Update(void);
void OLED_Clear(void);
void OLED_ClearArea(uint8_t X, uint8_t Y, uint8_t Width, uint8_t Height);
void OLED_ShowChar(uint8_t X, uint8_t Y, char Char, uint8_t FontSize);
void OLED_ShowString(uint8_t X, uint8_t Y, char *String, uint8_t FontSize);
void OLED_ShowImage(uint8_t X, uint8_t Y, uint8_t Width, uint8_t Height, const uint8_t *Image);
void OLED_ShowChinese(uint8_t X, uint8_t Y, char *Chinese);
void OLED_DrawPoint(uint8_t X, uint8_t Y);
uint8_t OLED_GetPoint(uint8_t X, uint8_t Y);
void OLED_DrawLine(uint8_t X0, uint8_t Y0, uint8_t X1, uint8_t Y1);

#endif
