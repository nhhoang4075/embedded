/*
 * platform_clock.c — Cài đặt trên Cortex-M4 (STM32F4).
 *
 * DWT (Data Watchpoint and Trace) có thanh ghi CYCCNT đếm chu kỳ CPU
 * 32-bit, chạy tự do khi được bật. Đây là nguồn entropy rất tốt cho
 * RNG và cũng là timer độ phân giải cao nhất trên Cortex-M4.
 */
#include "platform_clock.h"
#include "stm32f4xx_hal.h"

void platform_clock_init(void)
{
    /* Bật trace subsystem (DWT nằm trong nhóm trace). */
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT       = 0;
    DWT->CTRL        |= DWT_CTRL_CYCCNTENA_Msk;
}

uint32_t platform_now_ms(void)
{
    return HAL_GetTick();
}

uint32_t platform_cycles(void)
{
    return DWT->CYCCNT;
}
