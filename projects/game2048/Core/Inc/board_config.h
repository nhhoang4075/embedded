/*
 * board_config.h — Cấu hình phần cứng board (STM32F429I-DISC1 revB).
 *
 * Tập trung mọi ánh xạ chân, kênh ADC, handle peripheral vào một file.
 * Port sang board khác (F407, F411, revC…) chỉ cần chỉnh file này.
 *
 * Sau khi cấu hình bằng CubeMX, JOY_SW và USART1 đã có macro/handle sinh
 * sẵn trong main.h. File này alias lại thành BOARD_* cho nhất quán với
 * các phần còn lại (ADC channel, flash sector) vẫn để dạng hằng số.
 */
#ifndef GAME2048_BOARD_CONFIG_H
#define GAME2048_BOARD_CONFIG_H

#include "stm32f4xx_hal.h"
#include "main.h"           /* JOY_SW_Pin, JOY_SW_GPIO_Port — do CubeMX sinh */

/* ------------------------------------------------------------------ */
/* Joystick analog (KY-023 hoặc tương tự, VCC = 3.3V)                 */
/* ------------------------------------------------------------------ */

/* Kênh ADC cho hai trục — khớp chân trống trên F429I-DISC1:
 *   PA5  → ADC1 IN5   (trục X, VRx)
 *   PC3  → ADC1 IN13  (trục Y, VRy)
 * Cả hai không đụng LTDC/FMC/SPI5. */
#define BOARD_JOY_ADC_CH_X   ADC_CHANNEL_5
#define BOARD_JOY_ADC_CH_Y   ADC_CHANNEL_13

/* Nút SW của joystick — CubeMX cấu hình PG3 làm GPIO Input, pull-up nội,
 * user label "JOY_SW". main.h sinh JOY_SW_Pin / JOY_SW_GPIO_Port; ta chỉ
 * alias lại cho nhất quán với các định danh BOARD_* khác trong file này.
 *
 * Ghi chú lịch sử: trước đây chân này là PA6 và trùng với LTDC_G2 (bit 2
 * kênh xanh lá RGB565) — đã chuyển sang PG3 trong lần refactor board_config. */
#define BOARD_JOY_SW_PORT    JOY_SW_GPIO_Port
#define BOARD_JOY_SW_PIN     JOY_SW_Pin

/* ------------------------------------------------------------------ */
/* UART âm thanh sang ESP32                                            */
/* ------------------------------------------------------------------ */

/* USART1 TX PA9 → ESP32 D16 = Serial2 RX. CubeMX cấu hình Mode = MODE_TX
 * (Transmit Only) — huart1 handle sinh sẵn ở main.c. PA10 bị AF7 nhưng
 * peripheral chỉ enable TX side (không dùng RX). */
#define BOARD_AUDIO_UART     (&huart1)   /* Cần include main.h ở nơi dùng */

/* ------------------------------------------------------------------ */
/* Flash sector dành cho HighScoreStore                                */
/* ------------------------------------------------------------------ */

/* Sector 11 = 128 KB cuối bank 1 (địa chỉ 0x080E0000). Firmware không
 * chạm tới vùng này. Endurance ~10 000 erase/sector — dư dùng khi chỉ
 * ghi mỗi lần phá kỷ lục. */
#define BOARD_HISCORE_FLASH_SECTOR  FLASH_SECTOR_11
#define BOARD_HISCORE_FLASH_ADDR    0x080E0000U

#endif /* GAME2048_BOARD_CONFIG_H */
