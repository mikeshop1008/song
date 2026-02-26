#ifndef PIN_MAP_H
#define PIN_MAP_H

#include "app_config.h"
#include "stm32f4xx_hal.h"

/*
 * Mapping follows the requirement sheet.
 * Dx names correspond to Nucleo-style Arduino headers.
 */

/* L298N motor driver pins. */
#define MOTOR_IN1_GPIO_Port         GPIOA   /* D8  */
#define MOTOR_IN1_Pin               GPIO_PIN_9
#define MOTOR_IN2_GPIO_Port         GPIOA   /* D7  */
#define MOTOR_IN2_Pin               GPIO_PIN_8
#define MOTOR_IN3_GPIO_Port         GPIOB   /* D5  */
#define MOTOR_IN3_Pin               GPIO_PIN_4
#define MOTOR_IN4_GPIO_Port         GPIOB   /* D4  */
#define MOTOR_IN4_Pin               GPIO_PIN_5
#define MOTOR_ENA_GPIO_Port         GPIOC   /* D9  */
#define MOTOR_ENA_Pin               GPIO_PIN_7
#define MOTOR_ENB_GPIO_Port         GPIOB   /* D3  */
#define MOTOR_ENB_Pin               GPIO_PIN_3

/* Sensor ADC pins. */
#define OPB704_ADC_GPIO_Port        GPIOA   /* A0 */
#define OPB704_ADC_Pin              GPIO_PIN_0
#define OPB704_ADC_CHANNEL          ADC_CHANNEL_0

#define OBST_FRONT_ADC_GPIO_Port    GPIOA   /* A1 */
#define OBST_FRONT_ADC_Pin          GPIO_PIN_1
#define OBST_FRONT_ADC_CHANNEL      ADC_CHANNEL_1

#define OBST_LEFT_ADC_GPIO_Port     GPIOA   /* A2 */
#define OBST_LEFT_ADC_Pin           GPIO_PIN_4
#define OBST_LEFT_ADC_CHANNEL       ADC_CHANNEL_4

#define OBST_RIGHT_ADC_GPIO_Port    GPIOB   /* A3 */
#define OBST_RIGHT_ADC_Pin          GPIO_PIN_0
#define OBST_RIGHT_ADC_CHANNEL      ADC_CHANNEL_8

/* LED indicators. */
#define LED_OBSTACLE_GPIO_Port      GPIOA
#define LED_OBSTACLE_Pin            GPIO_PIN_5

/*
 * Requirement lists red LED on PA0, but PA0 is also OPB704 ADC input.
 * Default keeps OPB704 on PA0 and moves red LED to PC13.
 * If your board really wires red LED to PA0, set this to 1 and rework A0.
 */
#define RED_LED_SHARED_WITH_OPB704  0U
#if RED_LED_SHARED_WITH_OPB704
#define LED_MARK_GPIO_Port          GPIOA
#define LED_MARK_Pin                GPIO_PIN_0
#else
#define LED_MARK_GPIO_Port          GPIOC
#define LED_MARK_Pin                GPIO_PIN_13
#endif

/* 7-segment display (common cathode), segments A..G. */
#define SEG_A_GPIO_Port             GPIOC
#define SEG_A_Pin                   GPIO_PIN_0
#define SEG_B_GPIO_Port             GPIOC
#define SEG_B_Pin                   GPIO_PIN_1
#define SEG_C_GPIO_Port             GPIOC
#define SEG_C_Pin                   GPIO_PIN_2
#define SEG_D_GPIO_Port             GPIOC
#define SEG_D_Pin                   GPIO_PIN_3
#define SEG_E_GPIO_Port             GPIOC
#define SEG_E_Pin                   GPIO_PIN_4
#define SEG_F_GPIO_Port             GPIOC
#define SEG_F_Pin                   GPIO_PIN_5
#define SEG_G_GPIO_Port             GPIOC
#define SEG_G_Pin                   GPIO_PIN_6

/* Buzzer. */
#define BUZZER_GPIO_Port            GPIOB
#define BUZZER_Pin                  GPIO_PIN_10

/* HC-05 on USART2 (cross TX/RX externally as needed). */
#define HC05_UART_TX_GPIO_Port      GPIOA
#define HC05_UART_TX_Pin            GPIO_PIN_2
#define HC05_UART_RX_GPIO_Port      GPIOA
#define HC05_UART_RX_Pin            GPIO_PIN_3

/*
 * LCD1602 4-bit mode.
 * Default uses a conflict-free profile controlled by LCD_USE_CONFLICT_FREE_PINS.
 */
#define LCD_RS_GPIO_Port            GPIOC
#define LCD_RS_Pin                  GPIO_PIN_10
#define LCD_E_GPIO_Port             GPIOC
#define LCD_E_Pin                   GPIO_PIN_12

#if LCD_USE_CONFLICT_FREE_PINS
#define LCD_D4_GPIO_Port            GPIOB
#define LCD_D4_Pin                  GPIO_PIN_6
#define LCD_D5_GPIO_Port            GPIOB
#define LCD_D5_Pin                  GPIO_PIN_7
#define LCD_D6_GPIO_Port            GPIOA
#define LCD_D6_Pin                  GPIO_PIN_10
#define LCD_D7_GPIO_Port            GPIOB
#define LCD_D7_Pin                  GPIO_PIN_8
#define LCD_UART2_PA23_SHARED       0U
#else
#define LCD_D4_GPIO_Port            GPIOA
#define LCD_D4_Pin                  GPIO_PIN_3
#define LCD_D5_GPIO_Port            GPIOA
#define LCD_D5_Pin                  GPIO_PIN_2
#define LCD_D6_GPIO_Port            GPIOA
#define LCD_D6_Pin                  GPIO_PIN_10
#define LCD_D7_GPIO_Port            GPIOA
#define LCD_D7_Pin                  GPIO_PIN_8
#define LCD_UART2_PA23_SHARED       1U
#endif

#endif /* PIN_MAP_H */
