#include "lcd1602.h"

#include <string.h>

#include "app_config.h"
#include "pin_map.h"
#include "stm32f4xx_hal.h"

#if ENABLE_LCD
static uint8_t g_lcd_pins_ready = 0U;

static void Lcd1602_DelayUs(uint32_t us)
{
    uint32_t loops;

    if (us == 0U)
    {
        return;
    }

    loops = (SystemCoreClock / 5000000U) * us;
    while (loops-- > 0U)
    {
        __NOP();
    }
}

static void Lcd1602_ConfigPinsForWrite(void)
{
    GPIO_InitTypeDef gpio = {0};

    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;

    gpio.Pin = LCD_RS_Pin | LCD_E_Pin;
    HAL_GPIO_Init(LCD_RS_GPIO_Port, &gpio);

    gpio.Pin = LCD_D4_Pin;
    HAL_GPIO_Init(LCD_D4_GPIO_Port, &gpio);
    gpio.Pin = LCD_D5_Pin;
    HAL_GPIO_Init(LCD_D5_GPIO_Port, &gpio);
    gpio.Pin = LCD_D6_Pin;
    HAL_GPIO_Init(LCD_D6_GPIO_Port, &gpio);
    gpio.Pin = LCD_D7_Pin;
    HAL_GPIO_Init(LCD_D7_GPIO_Port, &gpio);

    g_lcd_pins_ready = 1U;
}

static void Lcd1602_EnsurePinsForWrite(void)
{
#if ENABLE_BLUETOOTH && LCD_UART2_PA23_SHARED
    Lcd1602_ConfigPinsForWrite();
#else
    if (g_lcd_pins_ready == 0U)
    {
        Lcd1602_ConfigPinsForWrite();
    }
#endif
}

static void Lcd1602_WriteDataPins(uint8_t nibble)
{
    HAL_GPIO_WritePin(LCD_D4_GPIO_Port, LCD_D4_Pin, ((nibble & 0x01U) != 0U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_D5_GPIO_Port, LCD_D5_Pin, ((nibble & 0x02U) != 0U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_D6_GPIO_Port, LCD_D6_Pin, ((nibble & 0x04U) != 0U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_D7_GPIO_Port, LCD_D7_Pin, ((nibble & 0x08U) != 0U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static void Lcd1602_PulseEnable(void)
{
    HAL_GPIO_WritePin(LCD_E_GPIO_Port, LCD_E_Pin, GPIO_PIN_SET);
    Lcd1602_DelayUs(2U);
    HAL_GPIO_WritePin(LCD_E_GPIO_Port, LCD_E_Pin, GPIO_PIN_RESET);
    Lcd1602_DelayUs(50U);
}

static void Lcd1602_SendNibble(uint8_t nibble)
{
    Lcd1602_WriteDataPins((uint8_t)(nibble & 0x0FU));
    Lcd1602_PulseEnable();
}

static void Lcd1602_SendByte(uint8_t value, uint8_t is_data)
{
    Lcd1602_EnsurePinsForWrite();
    HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, (is_data != 0U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    Lcd1602_SendNibble((uint8_t)(value >> 4U));
    Lcd1602_SendNibble((uint8_t)(value & 0x0FU));
    Lcd1602_DelayUs(50U);
}

static void Lcd1602_SendCommand(uint8_t command)
{
    Lcd1602_SendByte(command, 0U);
}

static void Lcd1602_SendData(uint8_t data)
{
    Lcd1602_SendByte(data, 1U);
}

void Lcd1602_Init(void)
{
    Lcd1602_ConfigPinsForWrite();
    HAL_Delay(40U);

    HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_E_GPIO_Port, LCD_E_Pin, GPIO_PIN_RESET);

    Lcd1602_SendNibble(0x03U);
    HAL_Delay(5U);
    Lcd1602_SendNibble(0x03U);
    HAL_Delay(1U);
    Lcd1602_SendNibble(0x03U);
    HAL_Delay(1U);
    Lcd1602_SendNibble(0x02U);
    HAL_Delay(1U);

    Lcd1602_SendCommand(0x28U); /* 4-bit, 2 lines, 5x8 font */
    Lcd1602_SendCommand(0x0CU); /* display on, cursor off */
    Lcd1602_SendCommand(0x06U); /* entry mode */
    Lcd1602_Clear();
}

void Lcd1602_Clear(void)
{
    Lcd1602_SendCommand(0x01U);
    HAL_Delay(2U);
}

void Lcd1602_SetCursor(uint8_t row, uint8_t col)
{
    uint8_t address = (row == 0U) ? 0x00U : 0x40U;
    if (col > 15U)
    {
        col = 15U;
    }
    Lcd1602_SendCommand((uint8_t)(0x80U | (address + col)));
}

void Lcd1602_Print(const char *text)
{
    uint8_t i = 0U;

    if (text == NULL)
    {
        return;
    }

    while ((text[i] != '\0') && (i < 16U))
    {
        Lcd1602_SendData((uint8_t)text[i]);
        ++i;
    }
}

void Lcd1602_PrintLine(uint8_t row, const char *text16)
{
    char buf[17];
    size_t len;

    if (text16 == NULL)
    {
        return;
    }

    memset(buf, ' ', sizeof(buf));
    buf[16] = '\0';

    len = strlen(text16);
    if (len > 16U)
    {
        len = 16U;
    }
    memcpy(buf, text16, len);

    Lcd1602_SetCursor(row, 0U);
    Lcd1602_Print(buf);
}
#else
void Lcd1602_Init(void) {}
void Lcd1602_Clear(void) {}
void Lcd1602_SetCursor(uint8_t row, uint8_t col)
{
    (void)row;
    (void)col;
}
void Lcd1602_Print(const char *text)
{
    (void)text;
}
void Lcd1602_PrintLine(uint8_t row, const char *text16)
{
    (void)row;
    (void)text16;
}
#endif
