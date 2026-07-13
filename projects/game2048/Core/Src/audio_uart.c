/*
 * audio_uart.c — Wrapper mỏng trên HAL_UART_Transmit gửi lệnh âm thanh
 * sang ESP32.
 *
 * Sau khi USART1 được đưa vào CubeMX, MX_USART1_UART_Init() do CubeMX
 * sinh đã lo phần khởi tạo. File này chỉ còn expose 2 API:
 *
 *   audio_uart_init() — no-op (giữ interface cho tương thích ngược,
 *                       tránh phải sửa main.c mỗi khi đổi backend).
 *   audio_uart_send() — gửi buffer thô với timeout 5 ms, non-blocking
 *                       thực tế vì payload chỉ 2 byte / lệnh.
 */
#include "audio_uart.h"
#include "stm32f4xx_hal.h"

/* Handle do CubeMX sinh trong main.c. */
extern UART_HandleTypeDef huart1;

bool audio_uart_init(void)
{
    /* CubeMX đã init trong MX_USART1_UART_Init() — không cần làm gì thêm. */
    return true;
}

void audio_uart_send(const uint8_t *data, size_t len)
{
    if (!data || len == 0) return;
    /* Timeout 5 ms — đủ cho vài byte ở 115200 baud (~87 μs/byte). Nếu
     * ESP32 chết hoặc dây đứt, hàm này trả về sau 5 ms, GUI không treo. */
    HAL_UART_Transmit(&huart1, (uint8_t *)data, (uint16_t)len, 5);
}
