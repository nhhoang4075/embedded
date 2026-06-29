/*
 * joystick.c - Doc joystick 2 truc qua ADC1 + DMA2 Stream 0 (circular).
 *
 * Mode: SCAN + CONTINUOUS + DMA Circular, NDTR=2 (X roi Y).
 * s_buf[] luon co gia tri ADC moi nhat -> joystick_poll() chi doc, ~0us.
 *
 * Phong ngu:
 *   - Disable DMA TC/HT/TE IT va ADC OVR IT (khong can IRQ).
 *   - poll() check Stream EN bit + co OVR -> auto restart neu Stream chet.
 *   - Sample time 56 cycles + DMAContinuousRequests=ENABLE de tranh OVR.
 *   - s_buf trong BSS (SRAM1, DMA-accessible).
 */
#include "joystick.h"
#include "stm32f4xx_hal.h"

static ADC_HandleTypeDef *s_hadc;
static volatile uint16_t  s_buf[2];   /* [0] = X, [1] = Y */
static int32_t s_center_x = 2048;
static int32_t s_center_y = 2048;
static bool    s_centered = true;

static bool     s_sw_released = true;
static uint32_t s_sw_last_ms  = 0;
#define SW_DEBOUNCE_MS  20u

static void sw_gpio_init(void)
{
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
    gpio.Pull = GPIO_PULLUP;
    gpio.Speed= GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(JOY_SW_PORT, &gpio);
}

/* Clear toan bo flag cua DMA2 Stream 0 (TCIF0 / HTIF0 / TEIF0 / DMEIF0 / FEIF0)
 * truoc khi restart - de tranh stream tu tat lai do flag cu chua clear. */
static void dma2_stream0_clear_flags(void)
{
    DMA2->LIFCR = DMA_LIFCR_CTCIF0 | DMA_LIFCR_CHTIF0
                | DMA_LIFCR_CTEIF0 | DMA_LIFCR_CDMEIF0
                | DMA_LIFCR_CFEIF0;
}

static void joystick_start_dma(void)
{
    HAL_NVIC_DisableIRQ(DMA2_Stream0_IRQn);   /* ko can IRQ, doc trustic*/
    __HAL_ADC_DISABLE_IT(s_hadc, ADC_IT_OVR);
    __HAL_ADC_CLEAR_FLAG(s_hadc, ADC_FLAG_OVR);
    dma2_stream0_clear_flags();

    if (HAL_ADC_Start_DMA(s_hadc, (uint32_t *)s_buf, 2) != HAL_OK) {
        /* Khong fatal - lan poll() ke se thu restart lai */
        return;
    }
    /* Tat callback DMA - khong can vi doc s_buf truc tiep. Disable interrupts
     * de DMA khong fire NVIC -> tranh tranh chap voi LTDC IRQ neu co. */
    __HAL_DMA_DISABLE_IT(s_hadc->DMA_Handle, DMA_IT_TC | DMA_IT_HT | DMA_IT_TE);
}

void joystick_init(ADC_HandleTypeDef *hadc)
{
    s_hadc = hadc;
    s_centered = true;
    s_sw_released = true;
    s_sw_last_ms  = 0;
    sw_gpio_init();

    joystick_start_dma();

    s_center_x = 2048;
    s_center_y = 2048;
}

bool joystick_sw_pressed(void)
{
    bool pressed_now = (HAL_GPIO_ReadPin(JOY_SW_PORT, JOY_SW_PIN) == GPIO_PIN_RESET);
    uint32_t now = HAL_GetTick();

    if (!pressed_now) {
        if (!s_sw_released)
            s_sw_last_ms = now;
        s_sw_released = true;
        return false;
    }

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

    /* Health check: neu DMA Stream EN bit = 0 hoac co OVR cua ADC -> restart.
     * Day la fail-safe duy nhat phai co cho DMA ADC tren STM32F4. */
    DMA_Stream_TypeDef *dmaStream = s_hadc->DMA_Handle->Instance;
    if (!(dmaStream->CR & DMA_SxCR_EN) ||
        (__HAL_ADC_GET_FLAG(s_hadc, ADC_FLAG_OVR)))
    {
        HAL_ADC_Stop_DMA(s_hadc);
        joystick_start_dma();
        /* Conversion dau tien can ~3us de san data - doc tam s_buf cu cung khong sao */
    }

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

    if (ax < (int32_t)JOY_DEADZONE && ay < (int32_t)JOY_DEADZONE) {
        s_centered = true;
        return JOY_NONE;
    }
    if (!s_centered)
        return JOY_NONE;

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
