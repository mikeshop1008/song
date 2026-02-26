#ifndef MOTOR_H
#define MOTOR_H

#include "stm32f4xx_hal.h"

void Motor_Init(void);
void Motor_Enable(void);
void Motor_Disable(void);
void Motor_SetSpeed(uint8_t left_percent, uint8_t right_percent);
void Motor_Forward(void);
void Motor_Backward(void);
void Motor_TurnLeftInPlace(void);
void Motor_TurnRightInPlace(void);
void Motor_Stop(void);

void Motor_SetPwmChannels(
    TIM_HandleTypeDef *left_htim,
    uint32_t left_channel,
    TIM_HandleTypeDef *right_htim,
    uint32_t right_channel);

#endif /* MOTOR_H */
