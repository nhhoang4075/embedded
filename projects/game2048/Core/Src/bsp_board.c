/*
 * bsp_board.c — Cài đặt cho F429I-DISC1.
 *
 * Đoạn WHO_AM_I trước đây nằm trong USER CODE END SPI5_Init 2 của
 * main.c, nhưng bản chất nó không phải khởi tạo SPI5 — nó là logic
 * detect board version chạy sau khi SPI5 sẵn sàng. Chuyển vào đây
 * cho đúng lớp.
 */
#include "bsp_board.h"
#include "main.h"    /* SPI5_NCS_Pin, SPI5_NCS_GPIO_Port */
#include "stm32f4xx_hal.h"

/* Handle peripheral do CubeMX khai báo trong main.c. */
extern SPI_HandleTypeDef  hspi5;

/* LCD driver do main.c khởi tạo (LcdDrv = &ili9341_drv; LcdDrv->Init(); …).
 * bsp_lcd_display_on() chỉ tái sử dụng — không đụng khởi tạo. */
#include "../../Drivers/BSP/Components/Common/lcd.h"
extern LCD_DrvTypeDef *LcdDrv;

static bool s_is_revd = false;

void bsp_board_detect_rev(void)
{
    /* WHO_AM_I: byte lệnh 0x8F = 0b1000_1111 (bit 7 = read, bit 0..6 =
     * địa chỉ 0x0F của thanh ghi WHO_AM_I). */
    const uint8_t READ_ID_CMD = 0x8F;
    uint8_t pdata = 0;

    HAL_GPIO_WritePin(SPI5_NCS_GPIO_Port, SPI5_NCS_Pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&hspi5, (uint8_t *)&READ_ID_CMD, 1, 1000);
    HAL_SPI_Receive(&hspi5, &pdata, 1, 1000);
    HAL_GPIO_WritePin(SPI5_NCS_GPIO_Port, SPI5_NCS_Pin, GPIO_PIN_SET);

    /* 0xD3 = I3G4250D → board revD trở lên.
     * 0xD4 = L3GD20   → board revC hoặc cũ hơn. */
    s_is_revd = (pdata == 0xD3);
}

bool bsp_board_is_revd(void)
{
    return s_is_revd;
}

void bsp_lcd_display_on(void)
{
    /* Xem comment cảnh báo trong bsp_board.h. */
    if (LcdDrv) LcdDrv->DisplayOn();
}
