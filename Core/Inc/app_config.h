#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include <stdint.h>

/* Optional features from the requirement document. */
#define ENABLE_BLUETOOTH            1U
#define ENABLE_LCD                  1U
#define ENABLE_MOTOR_PWM            1U

/*
 * 1: use conflict-free LCD remap so core motors + Bluetooth + LCD can run together.
 * 0: use requirement sheet LCD pins (may conflict with motor IN2 / USART2).
 */
#define LCD_USE_CONFLICT_FREE_PINS  1U

/* Main scheduling. */
#define MAIN_LOOP_PERIOD_MS         20U
#define LCD_REFRESH_PERIOD_MS       200U

/* OPB704 mark detection (A0). Active-low because collector is pulled up. */
#define OPB704_ACTIVE_LOW           1U
#define MARK_ADC_THRESHOLD          1800U
#define MARK_DEBOUNCE_MS            80U
#define MARK_REARM_MS               120U

/* 2Y0A21 25 cm threshold mapped to ADC count (12-bit @ 3.3V). */
#define OBSTACLE_ADC_THRESHOLD_25CM 1750U

/* Counter + display behavior (single common-cathode 7-seg digit). */
#define COUNTER_MAX_VALUE           9U

/* Motion timing (tune during on-car calibration). */
#define PAUSE_BEFORE_REVERSE_MS     250U
#define REVERSE_LONG_MS             1200U
#define BACKOFF_SHORT_MS            350U
#define TURN_90_MS                  560U
#define TURN_180_MS                 1080U

/* PWM speed setpoints (0..100). */
#define MOTOR_SPEED_FORWARD_PERCENT 72U
#define MOTOR_SPEED_REVERSE_PERCENT 62U
#define MOTOR_SPEED_TURN_PERCENT    60U

/* Buzzer feedback timings. */
#define BEEP_MARK_MS                50U
#define BEEP_OBSTACLE_MS            120U
#define BEEP_DONE_ON_MS             180U
#define BEEP_DONE_OFF_MS            80U

/* Optional UART report interval (HC-05). */
#define BLUETOOTH_STATUS_PERIOD_MS  500U

#endif /* APP_CONFIG_H */
