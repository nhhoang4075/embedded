/*
 * render.h - Vẽ giao diện 2048 lên LCD của STM32F429I-DISCO bằng thư viện BSP.
 *
 * Giao diện "đơn giản": ô vuông tô màu theo giá trị + số ở giữa + điểm phía trên.
 * Sau này muốn đẹp/animation thì thay bằng TouchGFX, lõi game vẫn giữ nguyên.
 *
 * Phụ thuộc: stm32f429i_discovery_lcd.{c,h} (copy từ CubeF4 BSP) và fonts.
 */
#ifndef RENDER_H
#define RENDER_H

#include "game2048.h"

/* Khởi tạo LCD + layer + vẽ nền tĩnh (tiêu đề). Gọi 1 lần lúc đầu. */
void render_init(void);

/* Vẽ lại toàn bộ trạng thái ván (lưới + điểm + thông báo WIN/LOSE). */
void render_board(const g2048_game_t *g);

#endif /* RENDER_H */
