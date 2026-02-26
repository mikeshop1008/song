#include "bluetooth.h"

#include <stdio.h>
#include <string.h>

#include "app_config.h"
#include "pin_map.h"

static UART_HandleTypeDef *g_uart = NULL;

#if ENABLE_BLUETOOTH && ENABLE_LCD && LCD_UART2_PA23_SHARED
static void Bluetooth_ConfigPinsForUart(void)
{
    GPIO_InitTypeDef gpio = {0};

    gpio.Pin = HC05_UART_TX_Pin | HC05_UART_RX_Pin;
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Pull = GPIO_PULLUP;
    gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    gpio.Alternate = GPIO_AF7_USART2;
    HAL_GPIO_Init(GPIOA, &gpio);
}
#endif

void Bluetooth_Init(UART_HandleTypeDef *huart)
{
    g_uart = huart;
}

void Bluetooth_SendText(const char *text)
{
#if ENABLE_BLUETOOTH
    if ((g_uart == NULL) || (text == NULL))
    {
        return;
    }

#if ENABLE_LCD && LCD_UART2_PA23_SHARED
    Bluetooth_ConfigPinsForUart();
#endif
    (void)HAL_UART_Transmit(g_uart, (uint8_t *)text, (uint16_t)strlen(text), 100U);
#else
    (void)text;
#endif
}

void Bluetooth_SendStatus(uint8_t counter, uint8_t scene_id, const SensorSnapshot *snapshot)
{
#if ENABLE_BLUETOOTH
    char msg[128];
    int len;

    if ((g_uart == NULL) || (snapshot == NULL))
    {
        return;
    }

#if ENABLE_LCD && LCD_UART2_PA23_SHARED
    Bluetooth_ConfigPinsForUart();
#endif

    len = snprintf(
        msg,
        sizeof(msg),
        "scene=%u,cnt=%u,opb=%u,f=%u,l=%u,r=%u\r\n",
        (unsigned int)scene_id,
        (unsigned int)counter,
        (unsigned int)snapshot->opb704_adc,
        (unsigned int)snapshot->front_adc,
        (unsigned int)snapshot->left_adc,
        (unsigned int)snapshot->right_adc);

    if (len > 0)
    {
        (void)HAL_UART_Transmit(g_uart, (uint8_t *)msg, (uint16_t)len, 100U);
    }
#else
    (void)counter;
    (void)scene_id;
    (void)snapshot;
#endif
}
