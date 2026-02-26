#include "seven_seg.h"

#include "pin_map.h"
#include "stm32f4xx_hal.h"

/* Bit order: A B C D E F G */
static const uint8_t kDigitMask[10] =
{
    0x3FU, /* 0 */
    0x06U, /* 1 */
    0x5BU, /* 2 */
    0x4FU, /* 3 */
    0x66U, /* 4 */
    0x6DU, /* 5 */
    0x7DU, /* 6 */
    0x07U, /* 7 */
    0x7FU, /* 8 */
    0x6FU  /* 9 */
};

static GPIO_TypeDef *const kSegPorts[7] =
{
    SEG_A_GPIO_Port,
    SEG_B_GPIO_Port,
    SEG_C_GPIO_Port,
    SEG_D_GPIO_Port,
    SEG_E_GPIO_Port,
    SEG_F_GPIO_Port,
    SEG_G_GPIO_Port
};

static const uint16_t kSegPins[7] =
{
    SEG_A_Pin,
    SEG_B_Pin,
    SEG_C_Pin,
    SEG_D_Pin,
    SEG_E_Pin,
    SEG_F_Pin,
    SEG_G_Pin
};

static void SevenSeg_WriteMask(uint8_t mask)
{
    uint8_t idx;

    for (idx = 0U; idx < 7U; ++idx)
    {
        GPIO_PinState state = ((mask & (1U << idx)) != 0U) ? GPIO_PIN_SET : GPIO_PIN_RESET;
        HAL_GPIO_WritePin(kSegPorts[idx], kSegPins[idx], state);
    }
}

void SevenSeg_Init(void)
{
    SevenSeg_Blank();
}

void SevenSeg_ShowDigit(uint8_t digit)
{
    if (digit > 9U)
    {
        SevenSeg_Blank();
        return;
    }
    SevenSeg_WriteMask(kDigitMask[digit]);
}

void SevenSeg_ShowNumber(int32_t number)
{
    uint8_t digit = 0U;

    if (number < 0)
    {
        SevenSeg_Blank();
        return;
    }

    digit = (uint8_t)(number % 10);
    SevenSeg_ShowDigit(digit);
}

void SevenSeg_Blank(void)
{
    SevenSeg_WriteMask(0U);
}
