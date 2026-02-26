#ifndef INDICATORS_H
#define INDICATORS_H

#include <stdint.h>

void Indicators_Init(void);
void Indicators_SetObstacleLed(uint8_t on);
void Indicators_SetMarkLed(uint8_t on);
void Indicators_BlinkBoth(uint8_t count, uint16_t on_ms, uint16_t off_ms);

#endif /* INDICATORS_H */
