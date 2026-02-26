#include "indicators.h"

#include "pin_map.h"
#include "stm32f4xx_hal.h"

void Indicators_Init(void)
{
    Indicators_SetObstacleLed(0U);
    Indicators_SetMarkLed(0U);
}

void Indicators_SetObstacleLed(uint8_t on)
{
    HAL_GPIO_WritePin(LED_OBSTACLE_GPIO_Port, LED_OBSTACLE_Pin, (on != 0U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void Indicators_SetMarkLed(uint8_t on)
{
    HAL_GPIO_WritePin(LED_MARK_GPIO_Port, LED_MARK_Pin, (on != 0U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void Indicators_BlinkBoth(uint8_t count, uint16_t on_ms, uint16_t off_ms)
{
    uint8_t i;
    for (i = 0U; i < count; ++i)
    {
        Indicators_SetObstacleLed(1U);
        Indicators_SetMarkLed(1U);
        HAL_Delay(on_ms);
        Indicators_SetObstacleLed(0U);
        Indicators_SetMarkLed(0U);
        if ((i + 1U) < count)
        {
            HAL_Delay(off_ms);
        }
    }
}
