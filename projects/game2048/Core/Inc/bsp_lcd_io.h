/*
 * bsp_lcd_io.h — Shim giữa driver màn hình (ILI9341, STMPE811) và HAL.
 *
 * Các hàm LCD_IO_* được driver ILI9341 gọi qua bảng function pointer
 * để đọc/ghi bus SPI5. Các hàm IOE_* được driver STMPE811 gọi để đọc
 * bus I²C3. Toàn bộ code này trước đây nằm trong main.c làm mờ nội
 * dung app — tách ra file riêng để main.c gọn hơn và có thể thay driver
 * nền tảng dễ dàng.
 *
 * Hàm nội bộ (I2C3_ReadData/WriteData, SPI5_Read/Write) giữ static
 * trong bsp_lcd_io.c — không lộ ra ngoài.
 */
#ifndef GAME2048_BSP_LCD_IO_H
#define GAME2048_BSP_LCD_IO_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* --- LCD IO gọi bởi driver ILI9341 (Components/ili9341/ili9341.c) --- */
void      LCD_IO_Init(void);
void      LCD_IO_WriteData(uint16_t RegValue);
void      LCD_IO_WriteReg(uint8_t Reg);
uint32_t  LCD_IO_ReadData(uint16_t RegValue, uint8_t ReadSize);
void      LCD_Delay(uint32_t Delay);

/* --- IO expander gọi bởi driver STMPE811 (Components/stmpe811/*) --- */
void      IOE_Init(void);
void      IOE_ITConfig(void);
void      IOE_Write(uint8_t Addr, uint8_t Reg, uint8_t Value);
uint8_t   IOE_Read(uint8_t Addr, uint8_t Reg);
uint16_t  IOE_ReadMultiple(uint8_t Addr, uint8_t Reg, uint8_t *pBuffer, uint16_t Length);
void      IOE_Delay(uint32_t Delay);

#ifdef __cplusplus
}
#endif

#endif /* GAME2048_BSP_LCD_IO_H */
