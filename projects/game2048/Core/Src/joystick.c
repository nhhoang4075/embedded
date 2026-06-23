/*
 * joystick.c - Đọc joystick 2 trục qua ADC1 + DMA circular.
 *
 * DMA tự chuyển 2 mẫu (rank1 = CH_X, rank2 = CH_Y) vào s_buf liên tục,
 * không tốn CPU. joystick_poll() chỉ việc đọc buffer và áp dụng
 * deadzone + edge-trigger để phát ra 1 hướng cho mỗi lần đẩy.
 */
#include "joystick.h"

static ADC_HandleTypeDef *s_hadc;
static volatile uint16_t  s_buf[2];   /* [0] = trục X (rank 1), [1] = trục Y (rank 2) */
static bool s_centered = true;        /* đã về vùng giữa kể từ lần đẩy trước chưa */

void joystick_init(ADC_HandleTypeDef *hadc)
{
    s_hadc = hadc;
    s_centered = true;
    HAL_ADC_Start_DMA(s_hadc, (uint32_t *)s_buf, 2);
    /* ADC continuous + DMA circular sẽ sinh DMA transfer-complete IRQ
     * mỗi 2 conversion (~750K IRQ/giây ở config hiện tại) -> CPU 100% trong
     * IRQ -> FreeRTOS scheduler không bao giờ start được.
     * Mình đọc s_buf trực tiếp nên không cần IRQ - disable hết. */
    __HAL_DMA_DISABLE_IT(s_hadc->DMA_Handle, DMA_IT_TC | DMA_IT_HT | DMA_IT_TE);
    HAL_NVIC_DisableIRQ(DMA2_Stream0_IRQn);
}

joy_dir_t joystick_poll(void)
{
    if (!s_hadc)
        return JOY_NONE;

    int32_t x = (int32_t)s_buf[0] - 2048;  /* lệch so với tâm 12-bit */
    int32_t y = (int32_t)s_buf[1] - 2048;

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
