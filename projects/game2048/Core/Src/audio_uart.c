/*
 * audio_uart.c — Khởi tạo USART1 PA9 và gửi byte thô sang ESP32.
 *
 * Chọn USART1 chứ không phải USART2 vì PA2 (chân TX USART2 default)
 * bị LTDC chiếm trên F429I-DISC1. PA9 là chân trống, dùng qua AF7.
 *
 * Không đi qua CubeMX — muốn CubeMX regen mà không mất code này thì
 * phải đặt ở đây, tách khỏi main.c.
 */
#include "audio_uart.h"
#include "board_config.h"
#include "stm32f4xx_hal.h"

/* Handle giữ static — không expose ra ngoài TU. Model / AudioBus chỉ
 * biết audio_uart_send(). */
static UART_HandleTypeDef s_huart;

bool audio_uart_init(void)
{
    /* Bật clock cho GPIO và cho USART. Không dùng __HAL_RCC_USART1_CLK_ENABLE
     * chung một chỗ để tránh gọi trùng nếu CubeMX cũng bật đâu đó. */
    __HAL_RCC_USART1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();   /* PA9 nằm ở GPIOA */

    GPIO_InitTypeDef gpio = {0};
    gpio.Pin       = BOARD_AUDIO_UART_TX_PIN;
    gpio.Mode      = GPIO_MODE_AF_PP;
    gpio.Pull      = GPIO_NOPULL;
    gpio.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    gpio.Alternate = BOARD_AUDIO_UART_AF;
    HAL_GPIO_Init(BOARD_AUDIO_UART_PORT, &gpio);

    s_huart.Instance          = BOARD_AUDIO_UART_INSTANCE;
    s_huart.Init.BaudRate     = BOARD_AUDIO_UART_BAUD;
    s_huart.Init.WordLength   = UART_WORDLENGTH_8B;
    s_huart.Init.StopBits     = UART_STOPBITS_1;
    s_huart.Init.Parity       = UART_PARITY_NONE;
    s_huart.Init.Mode         = UART_MODE_TX;         /* chỉ TX, không lắng nghe */
    s_huart.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
    s_huart.Init.OverSampling = UART_OVERSAMPLING_16;

    return HAL_UART_Init(&s_huart) == HAL_OK;
}

void audio_uart_send(const uint8_t *data, size_t len)
{
    if (!data || len == 0) return;
    /* Timeout 5 ms — đủ cho vài byte ở 115200 (~87 μs/byte). Nếu ESP32
     * chết, hàm này trả về sau đúng 5 ms, GUI không treo lâu. */
    HAL_UART_Transmit(&s_huart, (uint8_t *)data, (uint16_t)len, 5);
}
