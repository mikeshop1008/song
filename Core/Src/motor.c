#include "motor.h"

#include "app_config.h"
#include "pin_map.h"

static TIM_HandleTypeDef *g_left_pwm_timer = NULL;
static TIM_HandleTypeDef *g_right_pwm_timer = NULL;
static uint32_t g_left_pwm_channel = 0U;
static uint32_t g_right_pwm_channel = 0U;
static uint8_t g_pwm_ready = 0U;
static uint8_t g_motor_enabled = 0U;
static uint8_t g_left_speed_percent = 100U;
static uint8_t g_right_speed_percent = 100U;

static void Motor_WriteBridge(GPIO_PinState in1, GPIO_PinState in2, GPIO_PinState in3, GPIO_PinState in4)
{
    HAL_GPIO_WritePin(MOTOR_IN1_GPIO_Port, MOTOR_IN1_Pin, in1);
    HAL_GPIO_WritePin(MOTOR_IN2_GPIO_Port, MOTOR_IN2_Pin, in2);
    HAL_GPIO_WritePin(MOTOR_IN3_GPIO_Port, MOTOR_IN3_Pin, in3);
    HAL_GPIO_WritePin(MOTOR_IN4_GPIO_Port, MOTOR_IN4_Pin, in4);
}

static void Motor_ApplyDuty(TIM_HandleTypeDef *htim, uint32_t channel, uint8_t percent)
{
    uint32_t period = __HAL_TIM_GET_AUTORELOAD(htim) + 1U;
    __HAL_TIM_SET_COMPARE(htim, channel, (period * percent) / 100U);
}

static void Motor_ApplyEnableState(void)
{
#if ENABLE_MOTOR_PWM
    if ((g_pwm_ready == 0U) || (g_left_pwm_timer == NULL) || (g_right_pwm_timer == NULL))
    {
        return;
    }

    if (g_motor_enabled != 0U)
    {
        Motor_ApplyDuty(g_left_pwm_timer, g_left_pwm_channel, g_left_speed_percent);
        Motor_ApplyDuty(g_right_pwm_timer, g_right_pwm_channel, g_right_speed_percent);
    }
    else
    {
        Motor_ApplyDuty(g_left_pwm_timer, g_left_pwm_channel, 0U);
        Motor_ApplyDuty(g_right_pwm_timer, g_right_pwm_channel, 0U);
    }
#endif
}

void Motor_SetPwmChannels(
    TIM_HandleTypeDef *left_htim,
    uint32_t left_channel,
    TIM_HandleTypeDef *right_htim,
    uint32_t right_channel)
{
    g_left_pwm_timer = left_htim;
    g_left_pwm_channel = left_channel;
    g_right_pwm_timer = right_htim;
    g_right_pwm_channel = right_channel;

#if ENABLE_MOTOR_PWM
    if ((g_left_pwm_timer != NULL) && (g_right_pwm_timer != NULL))
    {
        (void)HAL_TIM_PWM_Start(g_left_pwm_timer, g_left_pwm_channel);
        (void)HAL_TIM_PWM_Start(g_right_pwm_timer, g_right_pwm_channel);
        g_pwm_ready = 1U;
        Motor_ApplyEnableState();
    }
#else
    (void)left_htim;
    (void)right_htim;
    (void)left_channel;
    (void)right_channel;
#endif
}

void Motor_Init(void)
{
    Motor_WriteBridge(GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_RESET);
    Motor_Enable();
    Motor_SetSpeed(100U, 100U);
}

void Motor_Enable(void)
{
#if ENABLE_MOTOR_PWM
    g_motor_enabled = 1U;
    Motor_ApplyEnableState();
#else
    HAL_GPIO_WritePin(MOTOR_ENA_GPIO_Port, MOTOR_ENA_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(MOTOR_ENB_GPIO_Port, MOTOR_ENB_Pin, GPIO_PIN_SET);
#endif
}

void Motor_Disable(void)
{
#if ENABLE_MOTOR_PWM
    g_motor_enabled = 0U;
    Motor_ApplyEnableState();
#else
    HAL_GPIO_WritePin(MOTOR_ENA_GPIO_Port, MOTOR_ENA_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MOTOR_ENB_GPIO_Port, MOTOR_ENB_Pin, GPIO_PIN_RESET);
#endif
}

void Motor_SetSpeed(uint8_t left_percent, uint8_t right_percent)
{
#if ENABLE_MOTOR_PWM
    if (left_percent > 100U)
    {
        left_percent = 100U;
    }
    if (right_percent > 100U)
    {
        right_percent = 100U;
    }

    g_left_speed_percent = left_percent;
    g_right_speed_percent = right_percent;
    Motor_ApplyEnableState();
#else
    (void)left_percent;
    (void)right_percent;
#endif
}

void Motor_Forward(void)
{
    Motor_Enable();
    Motor_WriteBridge(GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_RESET);
}

void Motor_Backward(void)
{
    Motor_Enable();
    Motor_WriteBridge(GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_SET);
}

void Motor_TurnLeftInPlace(void)
{
    Motor_Enable();
    Motor_WriteBridge(GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_RESET);
}

void Motor_TurnRightInPlace(void)
{
    Motor_Enable();
    Motor_WriteBridge(GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_SET);
}

void Motor_Stop(void)
{
    Motor_WriteBridge(GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_RESET);
    Motor_Disable();
}
