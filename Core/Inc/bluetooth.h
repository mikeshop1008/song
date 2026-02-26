#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include "stm32f4xx_hal.h"
#include "sensors.h"

void Bluetooth_Init(UART_HandleTypeDef *huart);
void Bluetooth_SendText(const char *text);
void Bluetooth_SendStatus(uint8_t counter, uint8_t scene_id, const SensorSnapshot *snapshot);

#endif /* BLUETOOTH_H */
