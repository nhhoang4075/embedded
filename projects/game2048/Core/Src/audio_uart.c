/*
 * audio_uart.c — Bọc HAL_UART_Transmit qua huart1 do CubeMX sinh.
 *
 * Không giữ trạng thái, không expose handle ra ngoài. Chỉ một hàm
 * duy nhất — audio_uart_send() — được AudioBus (TouchGFX layer) gọi.
 */
#include "audio_uart.h"
#include "stm32f4xx_hal.h"

/* Handle do CubeMX sinh trong main.c (MX_USART1_UART_Init). */
extern UART_HandleTypeDef huart1;

void audio_uart_send(const uint8_t *data, size_t len)
{
    if (!data || len == 0) return;
    /* Timeout 5 ms — đủ cho vài byte ở 115200 baud (~87 μs/byte). Nếu
     * ESP32 chết hoặc dây đứt, hàm này trả về sau 5 ms, GUI không treo. */
    HAL_UART_Transmit(&huart1, (uint8_t *)data, (uint16_t)len, 5);
}
