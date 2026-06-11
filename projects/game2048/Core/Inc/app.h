/*
 * app.h - Vòng lặp chính của game 2048 trên board.
 *
 * Gọi app_run() từ main.c SAU khi đã chạy hết các MX_*_Init().
 * Hàm này không trả về (chứa vòng lặp game).
 */
#ifndef APP_H
#define APP_H

#include "stm32f4xx_hal.h"

/*
 * hadc : ADC dùng đọc joystick (vd. &hadc1)
 * hrng : RNG phần cứng để sinh ô   (vd. &hrng)
 */
void app_run(ADC_HandleTypeDef *hadc, RNG_HandleTypeDef *hrng);

#endif /* APP_H */
