/*
 * audio_uart.h — Kênh UART một chiều gửi lệnh âm thanh sang ESP32.
 *
 * Đóng vai trò lớp driver mỏng nhất trên HAL. Chỉ expose hai hàm và
 * không leak UART_HandleTypeDef ra bên ngoài. Lớp cao hơn (AudioBus)
 * gói framing 2 byte quanh mỗi opcode.
 *
 * Chân, baud, instance đều lấy từ board_config.h.
 */
#ifndef GAME2048_AUDIO_UART_H
#define GAME2048_AUDIO_UART_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Bật clock, cấu hình PA9 AF7 và khởi tạo USART1. Gọi 1 lần trong
 * USER CODE BEGIN 2 của main(). Trả về false nếu HAL_UART_Init lỗi
 * (nhưng bên gọi hiện tại không cần biết — audio là non-critical). */
bool audio_uart_init(void);

/* Gửi buffer thô ra UART. Non-blocking effectively vì timeout đặt
 * 5 ms (đủ cho vài byte ở 115200); nếu ESP32 chết hoặc dây đứt, tối
 * đa tốn 5 ms rồi trả về. Không có ACK, không retransmit. */
void audio_uart_send(const uint8_t *data, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* GAME2048_AUDIO_UART_H */
