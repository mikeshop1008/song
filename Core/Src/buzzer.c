#include "buzzer.h"

#include "pin_map.h"
#include "stm32f4xx_hal.h"

void Buzzer_Init(void)
{
    Buzzer_Off();
}

void Buzzer_On(void)
{
    HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_SET);
}

void Buzzer_Off(void)
{
    HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_RESET);
}

void Buzzer_BeepBlocking(uint16_t duration_ms)
{
    Buzzer_On();
    HAL_Delay(duration_ms);
    Buzzer_Off();
}

void Buzzer_BeepPattern(uint8_t count, uint16_t on_ms, uint16_t off_ms)
{
    uint8_t i;
    for (i = 0U; i < count; ++i)
    {
        Buzzer_On();
        HAL_Delay(on_ms);
        Buzzer_Off();
        if ((i + 1U) < count)
        {
            HAL_Delay(off_ms);
        }
    }
}
