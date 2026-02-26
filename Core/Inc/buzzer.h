#ifndef BUZZER_H
#define BUZZER_H

#include <stdint.h>

void Buzzer_Init(void);
void Buzzer_On(void);
void Buzzer_Off(void);
void Buzzer_BeepBlocking(uint16_t duration_ms);
void Buzzer_BeepPattern(uint8_t count, uint16_t on_ms, uint16_t off_ms);

#endif /* BUZZER_H */
