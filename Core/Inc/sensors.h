#ifndef SENSORS_H
#define SENSORS_H

#include "stm32f4xx_hal.h"

typedef struct
{
    uint16_t opb704_adc;
    uint16_t front_adc;
    uint16_t left_adc;
    uint16_t right_adc;
    uint8_t mark_detected;
    uint8_t front_blocked;
    uint8_t left_blocked;
    uint8_t right_blocked;
} SensorSnapshot;

void Sensors_Init(ADC_HandleTypeDef *hadc);
void Sensors_Update(void);
const SensorSnapshot *Sensors_GetSnapshot(void);

uint8_t Sensors_ConsumeMarkEdge(void);

#endif /* SENSORS_H */
