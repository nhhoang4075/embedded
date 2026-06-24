/*
 * joystick.c - Đọc joystick 2 trục qua ADC1.
 *
 * MODE HIỆN TẠI: POLLED (HAL_ADC_PollForConversion).
 *   - Mỗi lần joystick_poll được gọi (~60Hz từ TouchGFX tick) -> chạy 2
 *     conversion thủ công cho channel X (PA5) rồi channel Y (PC3).
 *   - ~10µs/lần đọc -> CPU < 0.1%.
 *   - KHÔNG dùng DMA -> tránh stuck Stream do TE/OVR/bus conflict.
 *
 * MODE CŨ: DMA Circular. Code giữ lại trong khối #if 0 ở init() để fix sau.
 *   Triệu chứng cũ: DMA Stream EN bit tự tắt rất nhanh -> NDTR đứng yên,
 *   s_buf đông cứng ở giá trị mid-rail.
 *   Khả nghi: tranh chấp AHB bus với LTDC + DMA2D vẽ frame.
 */
#include "joystick.h"

static ADC_HandleTypeDef *s_hadc;
static volatile uint16_t  s_buf[2];   /* [0] = trục X, [1] = trục Y */
static int32_t s_center_x = 2048;
static int32_t s_center_y = 2048;
static bool    s_centered = true;     /* đã về vùng giữa kể từ lần đẩy trước chưa */

/* SW button: edge-trigger + debounce. SW nối GND khi ấn (active-low). */
static bool     s_sw_released = true; /* đã thả ra từ lần ấn trước */
static uint32_t s_sw_last_ms  = 0;    /* timestamp lần edge cuối, cho debounce */
#define SW_DEBOUNCE_MS  20u

/* Đọc 1 channel ADC theo kiểu polled, blocking ~5-10µs.
 * Yêu cầu ADC đã được chuyển về single-shot trong joystick_init. */
static uint16_t adc_read_polled(uint32_t channel)
{
    ADC_ChannelConfTypeDef sConfig = {0};
    sConfig.Channel      = channel;
    sConfig.Rank         = 1;
    sConfig.SamplingTime = ADC_SAMPLETIME_56CYCLES;
    HAL_ADC_ConfigChannel(s_hadc, &sConfig);

    HAL_ADC_Start(s_hadc);
    uint16_t val = 0;
    if (HAL_ADC_PollForConversion(s_hadc, 2) == HAL_OK) {
        val = (uint16_t)HAL_ADC_GetValue(s_hadc);
    }
    HAL_ADC_Stop(s_hadc);
    return val;
}

static void sw_gpio_init(void)
{
    /* Bật clock GPIO port chứa SW pin. Hỗ trợ A/B/C/D/E/F/G/H. */
    if      (JOY_SW_PORT == GPIOA) __HAL_RCC_GPIOA_CLK_ENABLE();
    else if (JOY_SW_PORT == GPIOB) __HAL_RCC_GPIOB_CLK_ENABLE();
    else if (JOY_SW_PORT == GPIOC) __HAL_RCC_GPIOC_CLK_ENABLE();
    else if (JOY_SW_PORT == GPIOD) __HAL_RCC_GPIOD_CLK_ENABLE();
    else if (JOY_SW_PORT == GPIOE) __HAL_RCC_GPIOE_CLK_ENABLE();
    else if (JOY_SW_PORT == GPIOF) __HAL_RCC_GPIOF_CLK_ENABLE();
    else if (JOY_SW_PORT == GPIOG) __HAL_RCC_GPIOG_CLK_ENABLE();
    else if (JOY_SW_PORT == GPIOH) __HAL_RCC_GPIOH_CLK_ENABLE();

    GPIO_InitTypeDef gpio = {0};
    gpio.Pin  = JOY_SW_PIN;
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_PULLUP;          /* SW nối GND khi ấn -> đọc 0 = đang ấn */
    gpio.Speed= GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(JOY_SW_PORT, &gpio);
}

void joystick_init(ADC_HandleTypeDef *hadc)
{
    s_hadc = hadc;
    s_centered = true;
    s_sw_released = true;
    s_sw_last_ms  = 0;
    sw_gpio_init();

#if 0  /* === MODE CŨ: DMA circular (stuck Stream, fix sau) === */
    HAL_ADC_Start_DMA(s_hadc, (uint32_t *)s_buf, 2);
    __HAL_DMA_DISABLE_IT(s_hadc->DMA_Handle, DMA_IT_TC | DMA_IT_HT | DMA_IT_TE);
    HAL_NVIC_DisableIRQ(DMA2_Stream0_IRQn);
    __HAL_ADC_DISABLE_IT(s_hadc, ADC_IT_OVR);
    __HAL_ADC_CLEAR_FLAG(s_hadc, ADC_FLAG_OVR);
#endif

    /* CubeMX init ADC ở SCAN+CONTINUOUS+DMA. Đổi sang single-shot polled.
     * Dùng direct register access vì HAL_ADC_Stop_DMA / HAL_ADC_Init có
     * thể treo nếu trạng thái nội bộ của ADC handle còn lỗi cũ.
     *  - CR1.SCAN  = 0 (single channel mỗi lần convert)
     *  - CR1.OVRIE = 0 (không kích NVIC ADC_IRQn chưa có handler)
     *  - CR2.CONT  = 0 (single-shot)
     *  - CR2.DMA   = 0 + CR2.DDS = 0 (không yêu cầu DMA)
     *  - SQR1.L    = 0 (chỉ 1 conversion trong sequence) */
    __HAL_ADC_DISABLE(s_hadc);                       /* ADON = 0 */
    s_hadc->Instance->CR1 &= ~(ADC_CR1_SCAN | ADC_CR1_OVRIE);
    s_hadc->Instance->CR2 &= ~(ADC_CR2_CONT | ADC_CR2_DMA | ADC_CR2_DDS);
    s_hadc->Instance->SQR1 &= ~ADC_SQR1_L;
    __HAL_ADC_CLEAR_FLAG(s_hadc, ADC_FLAG_OVR | ADC_FLAG_EOC);

    /* Center default 2048 - đủ chính xác vì joystick rest ~ mid-rail.
     * Không chạy calibration loop để tránh treo init nếu ADC còn lỗi. */
    s_center_x = 2048;
    s_center_y = 2048;
}

bool joystick_sw_pressed(void)
{
    /* Active-low: pressed = GPIO PIN reads 0. */
    bool pressed_now = (HAL_GPIO_ReadPin(JOY_SW_PORT, JOY_SW_PIN) == GPIO_PIN_RESET);
    uint32_t now = HAL_GetTick();

    if (!pressed_now) {
        /* Đã thả -> chuẩn bị nhận lần ấn tiếp theo. Cập nhật timestamp để
         * debounce phía edge ấn xuống cũng có hiệu lực. */
        if (!s_sw_released)
            s_sw_last_ms = now;
        s_sw_released = true;
        return false;
    }

    /* Đang ấn. Chỉ phát event nếu trước đó đã thả và qua debounce window. */
    if (!s_sw_released)
        return false;
    if ((now - s_sw_last_ms) < SW_DEBOUNCE_MS)
        return false;

    s_sw_released = false;
    s_sw_last_ms  = now;
    return true;
}

joy_dir_t joystick_poll(void)
{
    if (!s_hadc)
        return JOY_NONE;

    s_buf[0] = adc_read_polled(JOY_ADC_CH_X);
    s_buf[1] = adc_read_polled(JOY_ADC_CH_Y);

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
