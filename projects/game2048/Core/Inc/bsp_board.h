/*
 * bsp_board.h — Nhận diện phiên bản board và các fix-up khởi tạo.
 *
 * F429I-DISC1 có hai revision (revA/B/C và revD trở lên) khác nhau về
 * cách nối touch. Cách phát hiện: hỏi thanh ghi WHO_AM_I của
 * gyroscope trên SPI5 — mã 0xD3 = revD, khác đi = pre-revD.
 *
 * File này thay cho hai đoạn code trước đây nằm rải rác trong main.c:
 * biến global isRevD + đoạn WHO_AM_I. Cùng đóng vai trò lớp BSP là
 * fix "template CubeMX gọi LcdDrv->DisplayOff() nhưng không có
 * DisplayOn() lại" — bọc thành bsp_lcd_display_on() để chống bị dọn.
 */
#ifndef GAME2048_BSP_BOARD_H
#define GAME2048_BSP_BOARD_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Đọc WHO_AM_I của gyro qua SPI5, cache kết quả. Gọi 1 lần sau khi
 * MX_SPI5_Init đã chạy. */
void bsp_board_detect_rev(void);

/* Trả kết quả cached từ bsp_board_detect_rev(). Nếu chưa gọi detect,
 * trả false (giả định pre-revD). */
bool bsp_board_is_revd(void);

/* Bật lại LCD sau khi ILI9341 template gọi DisplayOff().
 *
 *  ┌─────────────────────────── CẢNH BÁO ───────────────────────────┐
 *  │ KHÔNG XOÁ HÀM NÀY. CubeMX BSP template gọi DisplayOff() sau     │
 *  │ Init nhưng KHÔNG có chỗ nào gọi DisplayOn() lại — panel ILI9341 │
 *  │ stay off, backlight on, không xuất GRAM → màn hình trắng vĩnh   │
 *  │ viễn. Hàm này là fix bắt buộc, phải được gọi sau LcdDrv->Init().│
 *  └────────────────────────────────────────────────────────────────┘ */
void bsp_lcd_display_on(void);

#ifdef __cplusplus
}
#endif

#endif /* GAME2048_BSP_BOARD_H */
