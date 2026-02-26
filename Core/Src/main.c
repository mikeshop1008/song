#include "stm32f4xx_hal.h"

#include <stdio.h>

#include "app_config.h"
#include "bluetooth.h"
#include "buzzer.h"
#include "indicators.h"
#include "lcd1602.h"
#include "motor.h"
#include "navigation.h"
#include "pin_map.h"
#include "sensors.h"
#include "seven_seg.h"

ADC_HandleTypeDef hadc1;
UART_HandleTypeDef huart2;
#if ENABLE_MOTOR_PWM
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
#endif

static void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_USART2_UART_Init(void);
#if ENABLE_MOTOR_PWM
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
#endif
static void Error_Handler(void);

#if ENABLE_LCD
static const char *MotionToText(NavMotion motion)
{
    switch (motion)
    {
    case NAV_MOTION_FORWARD:
        return "FWD";
    case NAV_MOTION_BACKWARD:
        return "BACK";
    case NAV_MOTION_TURN_LEFT:
        return "L-TURN";
    case NAV_MOTION_TURN_RIGHT:
        return "R-TURN";
    default:
        return "STOP";
    }
}

static void Lcd_ShowStatus(void)
{
    const SensorSnapshot *snapshot = Sensors_GetSnapshot();
    char line1[17];
    char line2[17];

    (void)snprintf(
        line1,
        sizeof(line1),
        "S%u %-12s",
        (unsigned int)Navigation_GetCurrentScene(),
        MotionToText(Navigation_GetMotion()));

    (void)snprintf(
        line2,
        sizeof(line2),
        "C:%u F%dL%dR%d",
        (unsigned int)Navigation_GetCounter(),
        snapshot->front_blocked,
        snapshot->left_blocked,
        snapshot->right_blocked);

    Lcd1602_PrintLine(0U, line1);
    Lcd1602_PrintLine(1U, line2);
}
#endif

int main(void)
{
#if ENABLE_BLUETOOTH
    uint32_t last_bluetooth_report_ms = 0U;
#endif
#if ENABLE_LCD
    uint32_t last_lcd_refresh_ms = 0U;
#endif

    HAL_Init();
    SystemClock_Config();

    MX_GPIO_Init();
    MX_ADC1_Init();
    MX_USART2_UART_Init();
#if ENABLE_MOTOR_PWM
    MX_TIM2_Init();
    MX_TIM3_Init();
    Motor_SetPwmChannels(&htim3, TIM_CHANNEL_2, &htim2, TIM_CHANNEL_2);
#endif

    Motor_Init();
    Sensors_Init(&hadc1);
    SevenSeg_Init();
    Buzzer_Init();
    Indicators_Init();
    Bluetooth_Init(&huart2);
#if ENABLE_LCD
    Lcd1602_Init();
#endif
    Navigation_Init();

#if ENABLE_BLUETOOTH
    Bluetooth_SendText("boot:navcar ready\r\n");
#endif

    while (1)
    {
        Navigation_Process();

#if ENABLE_BLUETOOTH
        if ((HAL_GetTick() - last_bluetooth_report_ms) >= BLUETOOTH_STATUS_PERIOD_MS)
        {
            Bluetooth_SendStatus(
                Navigation_GetCounter(),
                (uint8_t)Navigation_GetCurrentScene(),
                Sensors_GetSnapshot());
            last_bluetooth_report_ms = HAL_GetTick();
        }
#endif

#if ENABLE_LCD
        if ((HAL_GetTick() - last_lcd_refresh_ms) >= LCD_REFRESH_PERIOD_MS)
        {
            Lcd_ShowStatus();
            last_lcd_refresh_ms = HAL_GetTick();
        }
#endif

        HAL_Delay(MAIN_LOOP_PERIOD_MS);
    }
}

static void SystemClock_Config(void)
{
    RCC_OscInitTypeDef osc = {0};
    RCC_ClkInitTypeDef clk = {0};

    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

    osc.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    osc.HSIState = RCC_HSI_ON;
    osc.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    osc.PLL.PLLState = RCC_PLL_ON;
    osc.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    osc.PLL.PLLM = 16U;
    osc.PLL.PLLN = 336U;
    osc.PLL.PLLP = RCC_PLLP_DIV4;
    osc.PLL.PLLQ = 7U;
    if (HAL_RCC_OscConfig(&osc) != HAL_OK)
    {
        Error_Handler();
    }

    clk.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    clk.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    clk.AHBCLKDivider = RCC_SYSCLK_DIV1;
    clk.APB1CLKDivider = RCC_HCLK_DIV2;
    clk.APB2CLKDivider = RCC_HCLK_DIV1;
    if (HAL_RCC_ClockConfig(&clk, FLASH_LATENCY_2) != HAL_OK)
    {
        Error_Handler();
    }
}

static void MX_ADC1_Init(void)
{
    hadc1.Instance = ADC1;
    hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
    hadc1.Init.Resolution = ADC_RESOLUTION_12B;
    hadc1.Init.ScanConvMode = DISABLE;
    hadc1.Init.ContinuousConvMode = DISABLE;
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1.Init.NbrOfConversion = 1U;
    hadc1.Init.DMAContinuousRequests = DISABLE;
    hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
    if (HAL_ADC_Init(&hadc1) != HAL_OK)
    {
        Error_Handler();
    }
}

static void MX_USART2_UART_Init(void)
{
    huart2.Instance = USART2;
    huart2.Init.BaudRate = 9600;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&huart2) != HAL_OK)
    {
        Error_Handler();
    }
}

#if ENABLE_MOTOR_PWM
static void MX_TIM2_Init(void)
{
    TIM_OC_InitTypeDef oc = {0};

    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 83U;     /* 84 MHz / (83+1) = 1 MHz */
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 999U;       /* 1 kHz PWM */
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
    {
        Error_Handler();
    }

    oc.OCMode = TIM_OCMODE_PWM1;
    oc.Pulse = 0U;
    oc.OCPolarity = TIM_OCPOLARITY_HIGH;
    oc.OCFastMode = TIM_OCFAST_DISABLE;
    if (HAL_TIM_PWM_ConfigChannel(&htim2, &oc, TIM_CHANNEL_2) != HAL_OK)
    {
        Error_Handler();
    }
}

static void MX_TIM3_Init(void)
{
    TIM_OC_InitTypeDef oc = {0};

    htim3.Instance = TIM3;
    htim3.Init.Prescaler = 83U;     /* 84 MHz / (83+1) = 1 MHz */
    htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim3.Init.Period = 999U;       /* 1 kHz PWM */
    htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
    {
        Error_Handler();
    }

    oc.OCMode = TIM_OCMODE_PWM1;
    oc.Pulse = 0U;
    oc.OCPolarity = TIM_OCPOLARITY_HIGH;
    oc.OCFastMode = TIM_OCFAST_DISABLE;
    if (HAL_TIM_PWM_ConfigChannel(&htim3, &oc, TIM_CHANNEL_2) != HAL_OK)
    {
        Error_Handler();
    }
}
#endif

static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef gpio = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    HAL_GPIO_WritePin(GPIOA, MOTOR_IN1_Pin | MOTOR_IN2_Pin | LED_OBSTACLE_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, MOTOR_IN3_Pin | MOTOR_IN4_Pin | BUZZER_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC, SEG_A_Pin | SEG_B_Pin | SEG_C_Pin | SEG_D_Pin | SEG_E_Pin | SEG_F_Pin | SEG_G_Pin, GPIO_PIN_RESET);
#if !ENABLE_MOTOR_PWM
    HAL_GPIO_WritePin(MOTOR_ENA_GPIO_Port, MOTOR_ENA_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MOTOR_ENB_GPIO_Port, MOTOR_ENB_Pin, GPIO_PIN_RESET);
#endif
#if !RED_LED_SHARED_WITH_OPB704
    HAL_GPIO_WritePin(LED_MARK_GPIO_Port, LED_MARK_Pin, GPIO_PIN_RESET);
#endif
#if ENABLE_LCD
    HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_E_GPIO_Port, LCD_E_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_D4_GPIO_Port, LCD_D4_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_D5_GPIO_Port, LCD_D5_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_D6_GPIO_Port, LCD_D6_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_D7_GPIO_Port, LCD_D7_Pin, GPIO_PIN_RESET);
#endif

    gpio.Pin = MOTOR_IN1_Pin | MOTOR_IN2_Pin | LED_OBSTACLE_Pin;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &gpio);

    gpio.Pin = MOTOR_IN3_Pin | MOTOR_IN4_Pin | BUZZER_Pin;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &gpio);

    gpio.Pin = SEG_A_Pin | SEG_B_Pin | SEG_C_Pin | SEG_D_Pin | SEG_E_Pin | SEG_F_Pin | SEG_G_Pin;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &gpio);

#if !ENABLE_MOTOR_PWM
    gpio.Pin = MOTOR_ENA_Pin;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(MOTOR_ENA_GPIO_Port, &gpio);

    gpio.Pin = MOTOR_ENB_Pin;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(MOTOR_ENB_GPIO_Port, &gpio);
#endif

#if !RED_LED_SHARED_WITH_OPB704
    gpio.Pin = LED_MARK_Pin;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_MARK_GPIO_Port, &gpio);
#endif

#if ENABLE_LCD
    gpio.Pin = LCD_RS_Pin | LCD_E_Pin;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LCD_RS_GPIO_Port, &gpio);

    gpio.Pin = LCD_D4_Pin;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LCD_D4_GPIO_Port, &gpio);

    gpio.Pin = LCD_D5_Pin;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LCD_D5_GPIO_Port, &gpio);

    gpio.Pin = LCD_D6_Pin;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LCD_D6_GPIO_Port, &gpio);

    gpio.Pin = LCD_D7_Pin;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LCD_D7_GPIO_Port, &gpio);
#endif

    gpio.Pin = OPB704_ADC_Pin | OBST_FRONT_ADC_Pin | OBST_LEFT_ADC_Pin;
    gpio.Mode = GPIO_MODE_ANALOG;
    gpio.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &gpio);

    gpio.Pin = OBST_RIGHT_ADC_Pin;
    gpio.Mode = GPIO_MODE_ANALOG;
    gpio.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &gpio);

#if ENABLE_BLUETOOTH
#if !(ENABLE_LCD && LCD_UART2_PA23_SHARED)
    gpio.Pin = HC05_UART_TX_Pin | HC05_UART_RX_Pin;
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Pull = GPIO_PULLUP;
    gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    gpio.Alternate = GPIO_AF7_USART2;
    HAL_GPIO_Init(GPIOA, &gpio);
#endif
#endif
}

void HAL_ADC_MspInit(ADC_HandleTypeDef *adcHandle)
{
    if (adcHandle->Instance == ADC1)
    {
        __HAL_RCC_ADC1_CLK_ENABLE();
    }
}

void HAL_UART_MspInit(UART_HandleTypeDef *uartHandle)
{
    if (uartHandle->Instance == USART2)
    {
        __HAL_RCC_USART2_CLK_ENABLE();
    }
}

#if ENABLE_MOTOR_PWM
void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef *tim_pwmHandle)
{
    GPIO_InitTypeDef gpio = {0};

    if (tim_pwmHandle->Instance == TIM2)
    {
        __HAL_RCC_TIM2_CLK_ENABLE();

        gpio.Pin = MOTOR_ENB_Pin;
        gpio.Mode = GPIO_MODE_AF_PP;
        gpio.Pull = GPIO_NOPULL;
        gpio.Speed = GPIO_SPEED_FREQ_LOW;
        gpio.Alternate = GPIO_AF1_TIM2;
        HAL_GPIO_Init(MOTOR_ENB_GPIO_Port, &gpio);
    }
    else if (tim_pwmHandle->Instance == TIM3)
    {
        __HAL_RCC_TIM3_CLK_ENABLE();

        gpio.Pin = MOTOR_ENA_Pin;
        gpio.Mode = GPIO_MODE_AF_PP;
        gpio.Pull = GPIO_NOPULL;
        gpio.Speed = GPIO_SPEED_FREQ_LOW;
        gpio.Alternate = GPIO_AF2_TIM3;
        HAL_GPIO_Init(MOTOR_ENA_GPIO_Port, &gpio);
    }
}

void HAL_TIM_PWM_MspDeInit(TIM_HandleTypeDef *tim_pwmHandle)
{
    if (tim_pwmHandle->Instance == TIM2)
    {
        __HAL_RCC_TIM2_CLK_DISABLE();
    }
    else if (tim_pwmHandle->Instance == TIM3)
    {
        __HAL_RCC_TIM3_CLK_DISABLE();
    }
}
#endif

void HAL_ADC_MspDeInit(ADC_HandleTypeDef *adcHandle)
{
    if (adcHandle->Instance == ADC1)
    {
        __HAL_RCC_ADC1_CLK_DISABLE();
    }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef *uartHandle)
{
    if (uartHandle->Instance == USART2)
    {
        __HAL_RCC_USART2_CLK_DISABLE();
    }
}

static void Error_Handler(void)
{
    __disable_irq();
    while (1)
    {
        Indicators_BlinkBoth(1U, 150U, 0U);
        HAL_Delay(150U);
    }
}
