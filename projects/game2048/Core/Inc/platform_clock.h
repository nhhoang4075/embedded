/*
 * platform_clock.h — Nguồn thời gian và entropy cho lớp app.
 *
 * Model không nên gọi HAL_GetTick trực tiếp (tránh phụ thuộc HAL trong
 * app layer, khó test host-side). Header này bọc lại thành C++ inline
 * và cũng expose bộ đếm chu kỳ CPU (DWT->CYCCNT) làm entropy tốt.
 *
 * DWT->CYCCNT được bật một lần trong platform_clock_init(). Nếu chưa
 * gọi init, gọi platform_clock_cycles() trả về 0.
 */
#ifndef GAME2048_PLATFORM_CLOCK_H
#define GAME2048_PLATFORM_CLOCK_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Bật DWT->CYCCNT (đếm chu kỳ CPU). Gọi 1 lần trong USER CODE BEGIN 2. */
void platform_clock_init(void);

/* Millisecond từ khi boot (giống HAL_GetTick nhưng không leak HAL). */
uint32_t platform_now_ms(void);

/* Chu kỳ CPU từ khi bật DWT — nguồn entropy rất tốt cho seed RNG. */
uint32_t platform_cycles(void);

#ifdef __cplusplus
}
#endif

#endif /* GAME2048_PLATFORM_CLOCK_H */
