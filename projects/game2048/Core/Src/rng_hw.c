/*
 * rng_hw.c - Sinh số ngẫu nhiên bằng RNG phần cứng của STM32F4.
 */
#include "rng_hw.h"
#include "stm32f4xx_hal.h"

uint32_t rng_hw(void *ctx)
{
    RNG_HandleTypeDef *hrng = (RNG_HandleTypeDef *)ctx;
    uint32_t v = 0;
    if (hrng && HAL_RNG_GenerateRandomNumber(hrng, &v) == HAL_OK)
        return v;
    return 0;   /* hiếm khi lỗi; trả 0 cho an toàn */
}
