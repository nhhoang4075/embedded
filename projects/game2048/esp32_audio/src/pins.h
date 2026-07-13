/*
 * pins.h — Tập trung mọi ánh xạ chân của board ESP32 audio slave.
 *
 * Port sang board khác chỉ cần sửa file này.
 */
#pragma once

// microSD qua SPI ---------------------------------------------------
constexpr int PIN_SD_MISO = 5;
constexpr int PIN_SD_MOSI = 18;
constexpr int PIN_SD_SCK  = 19;
constexpr int PIN_SD_CS   = 21;
constexpr uint32_t SD_SPI_CLOCK_HZ = 4000000;   // 4 MHz — Catalex ổn

// MAX98357A I²S -----------------------------------------------------
constexpr int PIN_I2S_BCLK = 26;
constexpr int PIN_I2S_LRC  = 25;
constexpr int PIN_I2S_DOUT = 27;

// Serial2 nhận lệnh từ STM32 ---------------------------------------
constexpr int PIN_UART_RX = 16;   // STM32 USART1 PA9 -> đây
constexpr int PIN_UART_TX = 17;   // Không dùng (audio slave chỉ nhận)
constexpr uint32_t UART_BAUD_HZ = 115200;
