/*
 * joystick.c — Đọc joystick 2 trục qua ADC1 + DMA2 Stream 0 (circular).
 *
 * Chế độ: SCAN + CONTINUOUS + DMA Circular, NDTR = 2 (X rồi Y).
 * s_buf[] luôn có giá trị ADC mới nhất → joystick_poll() chỉ đọc.
 *
 * Phòng ngự:
 *   - Tắt DMA TC/HT/TE IT và ADC OVR IT (không cần IRQ, tránh preempt LTDC).
 *   - poll() kiểm bit Stream EN + cờ OVR → auto restart nếu stream chết.
 *   - Sample time 56 chu kỳ + DMAContinuousRequests=ENABLE để tránh OVR.
 *   - s_buf ở BSS (SRAM1, DMA truy cập được).
 *
 * Cấu trúc: mỗi bước xử lý là một hàm nhỏ, joystick_poll() chỉ điều phối.
 */
#include "joystick.h"
#include "board_config.h"
#include "stm32f4xx_hal.h"

/* ------------------------------------------------------------------ */
/* Trạng thái nội bộ                                                  */
/* ------------------------------------------------------------------ */

static ADC_HandleTypeDef *s_hadc;
static volatile uint16_t  s_buf[2];    /* [0] = X, [1] = Y */

/* Tâm đã calibrate — đọc trong joystick_init. Fallback 2048 nếu chưa
 * kịp đo được. */
static int32_t s_center_x = 2048;
static int32_t s_center_y = 2048;

/* Cần đã về deadzone chưa — điều kiện để lần "đẩy" tiếp theo tính. */
static bool    s_centered = true;

/* Debounce & edge cho nút SW. */
static bool     s_sw_released = true;
static uint32_t s_sw_last_ms  = 0;
#define SW_DEBOUNCE_MS  20u

/* ------------------------------------------------------------------ */
/* Khởi tạo phần cứng                                                 */
/* ------------------------------------------------------------------ */

/* Chân SW (PG3) do CubeMX cấu hình sẵn qua MX_GPIO_Init() — Input mode,
 * Pull-up. Không cần khởi tạo lại ở đây. */

/* Xoá toàn bộ cờ DMA2 Stream 0 (TCIF0/HTIF0/TEIF0/DMEIF0/FEIF0) trước
 * khi restart — cờ tồn đọng sẽ khiến bit EN không lên lại được. */
static void dma2_stream0_clear_flags(void)
{
    DMA2->LIFCR = DMA_LIFCR_CTCIF0 | DMA_LIFCR_CHTIF0
                | DMA_LIFCR_CTEIF0 | DMA_LIFCR_CDMEIF0
                | DMA_LIFCR_CFEIF0;
}

static void start_dma(void)
{
    /* Tắt IRQ trên đường DMA để không tranh chấp với LTDC. Con số OVR
     * thì ADC vẫn tự set — ta đọc cờ ở watchdog để tự khắc phục. */
    HAL_NVIC_DisableIRQ(DMA2_Stream0_IRQn);
    __HAL_ADC_DISABLE_IT(s_hadc, ADC_IT_OVR);
    __HAL_ADC_CLEAR_FLAG(s_hadc, ADC_FLAG_OVR);
    dma2_stream0_clear_flags();

    if (HAL_ADC_Start_DMA(s_hadc, (uint32_t *)s_buf, 2) != HAL_OK) {
        return;   /* watchdog sẽ thử restart ở poll() tiếp theo */
    }
    __HAL_DMA_DISABLE_IT(s_hadc->DMA_Handle,
                         DMA_IT_TC | DMA_IT_HT | DMA_IT_TE);
}

/* Đọc N mẫu, lấy trung bình làm tâm. Gọi trong joystick_init sau khi
 * DMA đã có thời gian nạp buffer. Dùng nguồn s_buf trực tiếp — DMA
 * đang liên tục ghi vào đó. */
static void calibrate_center(void)
{
    /* Đợi ~20 ms cho DMA có đủ thời gian đạt điểm ổn định (còn dư
     * nhiều lần chuyển đổi ở tốc độ 22.5 MHz / 56 cycles). */
    HAL_Delay(20);

    uint32_t sum_x = 0;
    uint32_t sum_y = 0;
    const uint32_t N = 8;
    for (uint32_t i = 0; i < N; ++i) {
        sum_x += s_buf[0];
        sum_y += s_buf[1];
        HAL_Delay(1);
    }
    s_center_x = (int32_t)(sum_x / N);
    s_center_y = (int32_t)(sum_y / N);
}

void joystick_init(ADC_HandleTypeDef *hadc)
{
    s_hadc        = hadc;
    s_centered    = true;
    s_sw_released = true;
    s_sw_last_ms  = 0;

    /* Chân SW do CubeMX MX_GPIO_Init() đã cấu hình xong (Input pull-up
     * trên PG3). Ở đây chỉ cần start DMA và calibrate tâm joystick. */
    start_dma();
    calibrate_center();
}

/* ------------------------------------------------------------------ */
/* Nút SW — debounce + edge-trigger                                   */
/* ------------------------------------------------------------------ */

bool joystick_sw_pressed(void)
{
    bool pressed_now =
        (HAL_GPIO_ReadPin(BOARD_JOY_SW_PORT, BOARD_JOY_SW_PIN) == GPIO_PIN_RESET);
    uint32_t now = HAL_GetTick();

    if (!pressed_now) {
        if (!s_sw_released) s_sw_last_ms = now;
        s_sw_released = true;
        return false;
    }

    if (!s_sw_released) return false;
    if ((now - s_sw_last_ms) < SW_DEBOUNCE_MS) return false;

    s_sw_released = false;
    s_sw_last_ms  = now;
    return true;
}

/* ------------------------------------------------------------------ */
/* joystick_poll — điều phối bốn bước                                 */
/* ------------------------------------------------------------------ */

/* 1) Watchdog: nếu DMA stream đã chết (bit EN = 0) hoặc ADC vừa
 *    overrun, dừng và khởi động lại. Đảm bảo joystick không "đông cứng". */
static void dma_watchdog(void)
{
    DMA_Stream_TypeDef *dmaStream = s_hadc->DMA_Handle->Instance;
    if (!(dmaStream->CR & DMA_SxCR_EN) ||
        (__HAL_ADC_GET_FLAG(s_hadc, ADC_FLAG_OVR)))
    {
        HAL_ADC_Stop_DMA(s_hadc);
        start_dma();
        /* Chuyển đổi đầu tiên cần ~3 μs để sẵn dữ liệu — đọc tạm
         * giá trị cũ ở s_buf trong poll() hiện tại cũng không sao. */
    }
}

/* 2) Chuẩn hoá 1 trục: chuyển raw ADC sang lệch có dấu so với tâm,
 *    áp dụng hoán đổi/nghịch chiều theo cấu hình. */
static void axis_normalize(int32_t *x_out, int32_t *y_out)
{
    int32_t x = (int32_t)s_buf[0] - s_center_x;
    int32_t y = (int32_t)s_buf[1] - s_center_y;

#if JOY_SWAP_XY
    { int32_t t = x; x = y; y = t; }
#endif
#if JOY_INVERT_X
    x = -x;
#endif
#if JOY_INVERT_Y
    y = -y;
#endif

    *x_out = x;
    *y_out = y;
}

/* 3) Chốt hướng theo edge trigger: chỉ phát 1 nước khi cần vừa vượt
 *    threshold sau khi đã về deadzone. Trả JOY_NONE nếu chưa đủ. */
static joy_dir_t edge_trigger(int32_t x, int32_t y)
{
    int32_t ax = x < 0 ? -x : x;
    int32_t ay = y < 0 ? -y : y;

    /* Trong deadzone — reset trạng thái "đã centered" để lần đẩy sau tính. */
    if (ax < (int32_t)JOY_DEADZONE && ay < (int32_t)JOY_DEADZONE) {
        s_centered = true;
        return JOY_NONE;
    }

    /* Chưa về deadzone giữa các lần đẩy → không phát hướng mới. */
    if (!s_centered) return JOY_NONE;

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

joy_dir_t joystick_poll(void)
{
    if (!s_hadc) return JOY_NONE;

    dma_watchdog();

    int32_t x, y;
    axis_normalize(&x, &y);
    return edge_trigger(x, y);
}

/* ------------------------------------------------------------------ */
/* Truy cập giá trị thô — nguồn entropy cho reseed RNG                */
/* ------------------------------------------------------------------ */

uint16_t joystick_raw_x(void) { return s_buf[0]; }
uint16_t joystick_raw_y(void) { return s_buf[1]; }
