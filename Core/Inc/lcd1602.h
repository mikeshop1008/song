#ifndef LCD1602_H
#define LCD1602_H

#include <stdint.h>

void Lcd1602_Init(void);
void Lcd1602_Clear(void);
void Lcd1602_SetCursor(uint8_t row, uint8_t col);
void Lcd1602_Print(const char *text);
void Lcd1602_PrintLine(uint8_t row, const char *text16);

#endif /* LCD1602_H */
