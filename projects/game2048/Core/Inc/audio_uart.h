/*
 * audio_uart.h — Kênh UART một chiều gửi lệnh âm thanh sang ESP32.
 *
 * Lớp wrapper mỏng nhất trên HAL_UART_Transmit. Không expose UART
 * handle ra ngoài; lớp cao hơn (AudioBus) gói framing 2 byte quanh
 * mỗi opcode. Chân/baud do CubeMX cấu hình qua game2048.ioc, xem
 * huart1 handle trong main.c.
 */
#ifndef GAME2048_AUDIO_UART_H
#define GAME2048_AUDIO_UART_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Gửi buffer thô ra UART. Non-blocking effectively vì timeout đặt
 * 5 ms (đủ cho vài byte ở 115200); nếu ESP32 chết hoặc dây đứt, tối
 * đa tốn 5 ms rồi trả về. Không có ACK, không retransmit. */
void audio_uart_send(const uint8_t *data, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* GAME2048_AUDIO_UART_H */
