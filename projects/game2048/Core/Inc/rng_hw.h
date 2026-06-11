/*
 * rng_hw.h - Bọc bộ sinh số ngẫu nhiên phần cứng (RNG) cho lõi game.
 *
 * Cấu hình trong CubeMX: bật RNG (Analog > RNG).
 * Truyền &hrng làm rng_ctx khi gọi g2048_init().
 */
#ifndef RNG_HW_H
#define RNG_HW_H

#include <stdint.h>

/* Hợp với chữ ký g2048_rng_fn. ctx phải là (RNG_HandleTypeDef*). */
uint32_t rng_hw(void *ctx);

#endif /* RNG_HW_H */
