/*
 * bsp_lcd_io.c — Cài đặt shim SPI5/I²C3 cho ILI9341 + STMPE811.
 *
 * Toàn bộ code này được trích ra từ main.c (khu vực USER CODE 4).
 * Handle hi2c3 và hspi5 vẫn thuộc về main.c (CubeMX sinh), file này
 * chỉ tham chiếu qua extern.
 *
 * Timeout là hằng số nội bộ — không cần thay đổi runtime.
 */
#include "bsp_lcd_io.h"
#include "stm32f4xx_hal.h"

/* Handle peripheral do CubeMX khai báo trong main.c. */
extern I2C_HandleTypeDef hi2c3;
extern SPI_HandleTypeDef hspi5;

/* Timeout tính bằng số vòng lặp (theo template BSP STMicro). */
#define I2C3_TIMEOUT_MAX  0x3000u
#define SPI5_TIMEOUT_MAX  0x1000u

/* -----------------------------------------------------------------
 * Lớp SPI5 wrapper — driver LCD gọi gián tiếp qua LCD_IO_*.
 * ----------------------------------------------------------------- */

static void SPI5_Error(void)
{
    /* Reserve cho tương lai — hiện tại không tự khởi động lại bus. */
}

static void SPI5_Write(uint16_t Value)
{
    HAL_StatusTypeDef status =
        HAL_SPI_Transmit(&hspi5, (uint8_t *)&Value, 1, SPI5_TIMEOUT_MAX);
    if (status != HAL_OK) SPI5_Error();
}

static uint32_t SPI5_Read(uint8_t ReadSize)
{
    uint32_t readvalue = 0;
    HAL_StatusTypeDef status =
        HAL_SPI_Receive(&hspi5, (uint8_t *)&readvalue, ReadSize, SPI5_TIMEOUT_MAX);
    if (status != HAL_OK) SPI5_Error();
    return readvalue;
}

/* -----------------------------------------------------------------
 * Lớp I²C3 wrapper — driver IO expander STMPE811 gọi qua IOE_*.
 * ----------------------------------------------------------------- */

static void I2C3_WriteData(uint8_t Addr, uint8_t Reg, uint8_t Value)
{
    /* Trả về bị bỏ qua — driver STMPE811 không xử lý lỗi bus. */
    (void)HAL_I2C_Mem_Write(&hi2c3, Addr, (uint16_t)Reg,
                            I2C_MEMADD_SIZE_8BIT, &Value, 1, I2C3_TIMEOUT_MAX);
}

static uint8_t I2C3_ReadData(uint8_t Addr, uint8_t Reg)
{
    uint8_t value = 0;
    (void)HAL_I2C_Mem_Read(&hi2c3, Addr, Reg,
                           I2C_MEMADD_SIZE_8BIT, &value, 1, I2C3_TIMEOUT_MAX);
    return value;
}

static uint8_t I2C3_ReadBuffer(uint8_t Addr, uint8_t Reg,
                               uint8_t *pBuffer, uint16_t Length)
{
    HAL_StatusTypeDef status =
        HAL_I2C_Mem_Read(&hi2c3, Addr, (uint16_t)Reg,
                         I2C_MEMADD_SIZE_8BIT, pBuffer, Length, I2C3_TIMEOUT_MAX);
    return (status == HAL_OK) ? 0u : 1u;
}

/* -----------------------------------------------------------------
 * LCD IO — driver ILI9341 gọi qua các function pointer.
 * ----------------------------------------------------------------- */

void LCD_IO_Init(void)
{
    /* Toggle /CS để reset trạng thái driver. */
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_SET);
}

void LCD_IO_WriteData(uint16_t RegValue)
{
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);   /* WRX = data */
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2,  GPIO_PIN_RESET); /* /CS low   */
    SPI5_Write(RegValue);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2,  GPIO_PIN_SET);   /* /CS high  */
}

void LCD_IO_WriteReg(uint8_t Reg)
{
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET); /* WRX = cmd */
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2,  GPIO_PIN_RESET);
    SPI5_Write(Reg);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2,  GPIO_PIN_SET);
}

uint32_t LCD_IO_ReadData(uint16_t RegValue, uint8_t ReadSize)
{
    uint32_t readvalue = 0;
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2,  GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);
    SPI5_Write(RegValue);
    readvalue = SPI5_Read(ReadSize);
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2,  GPIO_PIN_SET);
    return readvalue;
}

void LCD_Delay(uint32_t Delay)
{
    HAL_Delay(Delay);
}

/* -----------------------------------------------------------------
 * IO expander STMPE811 — driver gọi các IOE_* để chạm bus I²C3.
 * ----------------------------------------------------------------- */

void IOE_Init(void)
{
    /* CubeMX đã bật clock I²C3 và cấu hình chân — không cần làm gì thêm. */
}

void IOE_ITConfig(void)
{
    /* STMPE811 IRQ line không được nối trên board — không cấu hình. */
}

void IOE_Write(uint8_t Addr, uint8_t Reg, uint8_t Value)
{
    I2C3_WriteData(Addr, Reg, Value);
}

uint8_t IOE_Read(uint8_t Addr, uint8_t Reg)
{
    return I2C3_ReadData(Addr, Reg);
}

uint16_t IOE_ReadMultiple(uint8_t Addr, uint8_t Reg, uint8_t *pBuffer, uint16_t Length)
{
    return I2C3_ReadBuffer(Addr, Reg, pBuffer, Length);
}

void IOE_Delay(uint32_t Delay)
{
    HAL_Delay(Delay);
}
