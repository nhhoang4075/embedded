/*
 * board_config.h — Cấu hình phần cứng board (STM32F429I-DISC1 revB).
 *
 * Tập trung mọi ánh xạ chân, kênh ADC, handle peripheral vào một file.
 * Port sang board khác (F407, F411, revC…) chỉ cần chỉnh file này.
 *
 * Nguyên tắc: file này KHÔNG chứa driver logic, chỉ chứa hằng số. File
 * driver (joystick.c, audio_uart.c…) include header này và không đưa
 * hằng số chân vào bất cứ đâu khác.
 */
#ifndef GAME2048_BOARD_CONFIG_H
#define GAME2048_BOARD_CONFIG_H

#include "stm32f4xx_hal.h"

/* ------------------------------------------------------------------ */
/* Joystick analog (KY-023 hoặc tương tự, VCC = 3.3V)                 */
/* ------------------------------------------------------------------ */

/* Kênh ADC cho hai trục — khớp chân trống trên F429I-DISC1:
 *   PA5  → ADC1 IN5   (trục X, VRx)
 *   PC3  → ADC1 IN13  (trục Y, VRy)
 * Cả hai không đụng LTDC/FMC/SPI5. */
#define BOARD_JOY_ADC_CH_X   ADC_CHANNEL_5
#define BOARD_JOY_ADC_CH_Y   ADC_CHANNEL_13

/* Nút SW của joystick (bấm xuống stick) — GPIO input, pull-up nội.
 *
 * CẢNH BÁO PA6: chân PA6 trước đây bị đặt trùng với LTDC_G2 (bit 2 kênh
 * xanh lá RGB565). Khi joystick_init override PA6 sang GPIO input, kênh
 * green trên panel bị mất 1 bit, tint xanh giảm ~6%. Vì thế phải chọn
 * chân KHÁC.
 *
 * PG3 là chân trống trên F429I-DISC1 (không dính LTDC/FMC/SPI5/USART),
 * được đưa ra trên header P1 nên dễ rewire. */
#define BOARD_JOY_SW_PORT    GPIOG
#define BOARD_JOY_SW_PIN     GPIO_PIN_3

/* ------------------------------------------------------------------ */
/* UART âm thanh sang ESP32                                            */
/* ------------------------------------------------------------------ */

/* USART1 TX PA9 → ESP32 D16 = Serial2 RX. Chỉ TX, không dùng RX.
 * Không đi qua CubeMX vì PA2 (USART2 TX) đụng chân với LTDC trên board
 * này; USART1 PA9 là chân trống dùng được. */
#define BOARD_AUDIO_UART_PORT     GPIOA
#define BOARD_AUDIO_UART_TX_PIN   GPIO_PIN_9
#define BOARD_AUDIO_UART_AF       GPIO_AF7_USART1
#define BOARD_AUDIO_UART_INSTANCE USART1
#define BOARD_AUDIO_UART_BAUD     115200U

/* ------------------------------------------------------------------ */
/* Flash sector dành cho HighScoreStore                                */
/* ------------------------------------------------------------------ */

/* Sector 11 = 128 KB cuối bank 1 (địa chỉ 0x080E0000). Firmware không
 * chạm tới vùng này. Endurance ~10 000 erase/sector — dư dùng khi chỉ
 * ghi mỗi lần phá kỷ lục. */
#define BOARD_HISCORE_FLASH_SECTOR  FLASH_SECTOR_11
#define BOARD_HISCORE_FLASH_ADDR    0x080E0000U

#endif /* GAME2048_BOARD_CONFIG_H */
