/*
 * joystick.c - Đọc ADC 2 trục và quy ra hướng gạt (edge-trigger + deadzone).
 */
#include "joystick.h"

static ADC_HandleTypeDef *s_hadc;
static bool s_centered = true;   /* cần đã về vùng giữa kể từ lần đẩy trước chưa */

void joystick_init(ADC_HandleTypeDef *hadc)
{
    s_hadc = hadc;
    s_centered = true;
}

/* Đọc 1 mẫu ADC ở kênh chỉ định (chế độ polling, dùng chung 1 ADC). */
static uint32_t read_channel(uint32_t channel)
{
    ADC_ChannelConfTypeDef cfg = {0};
    cfg.Channel = channel;
    cfg.Rank = 1;
    cfg.SamplingTime = ADC_SAMPLETIME_84CYCLES;
    HAL_ADC_ConfigChannel(s_hadc, &cfg);

    HAL_ADC_Start(s_hadc);
    uint32_t val = 0;
    if (HAL_ADC_PollForConversion(s_hadc, 10) == HAL_OK)
        val = HAL_ADC_GetValue(s_hadc);
    HAL_ADC_Stop(s_hadc);
    return val;
}

joy_dir_t joystick_poll(void)
{
    int32_t x = (int32_t)read_channel(JOY_ADC_CH_X) - 2048;  /* lệch so với tâm */
    int32_t y = (int32_t)read_channel(JOY_ADC_CH_Y) - 2048;

    /* Hiệu chỉnh chiều theo cách lắp joystick (cấu hình trong joystick.h). */
#if JOY_SWAP_XY
    { int32_t t = x; x = y; y = t; }
#endif
#if JOY_INVERT_X
    x = -x;
#endif
#if JOY_INVERT_Y
    y = -y;
#endif

    int32_t ax = x < 0 ? -x : x;
    int32_t ay = y < 0 ? -y : y;

    /* Đã về vùng giữa -> cho phép nhận lần đẩy mới. */
    if (ax < (int32_t)JOY_DEADZONE && ay < (int32_t)JOY_DEADZONE) {
        s_centered = true;
        return JOY_NONE;
    }

    if (!s_centered)
        return JOY_NONE;   /* vẫn đang giữ từ lần đẩy trước -> bỏ qua */

    /* Vượt ngưỡng theo trục mạnh hơn -> phát đúng 1 hướng. */
    if (ax >= (int32_t)JOY_THRESHOLD && ax >= ay) {
        s_centered = false;
        return (x > 0) ? JOY_DIR_RIGHT : JOY_DIR_LEFT;
    }
    if (ay >= (int32_t)JOY_THRESHOLD && ay > ax) {
        s_centered = false;
        return (y > 0) ? JOY_DIR_DOWN : JOY_DIR_UP;
    }
    return JOY_NONE;
}
