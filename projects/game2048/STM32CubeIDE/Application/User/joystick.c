/*
 * joystick.c - Đọc ADC 2 trục và quy ra hướng gạt (edge-trigger + deadzone).
 */
#include "joystick.h"

static bool s_centered = true;   /* cần đã về vùng giữa kể từ lần đẩy trước chưa */
static ADC_HandleTypeDef *s_hadc;

extern uint16_t adc_buffer[2];

void joystick_init(ADC_HandleTypeDef *hadc)
{
    s_hadc = hadc;
    s_centered = true;
}

joy_dir_t joystick_poll(void)
{
	int32_t x = (int32_t)adc_buffer[0] - 2048;
	int32_t y = (int32_t)adc_buffer[1] - 2048;

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
