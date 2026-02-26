#ifndef SEVEN_SEG_H
#define SEVEN_SEG_H

#include <stdint.h>

void SevenSeg_Init(void);
void SevenSeg_ShowDigit(uint8_t digit);
void SevenSeg_ShowNumber(int32_t number);
void SevenSeg_Blank(void);

#endif /* SEVEN_SEG_H */
