/*
 * game2048.h - Lõi logic trò chơi 2048 (C thuần, không phụ thuộc phần cứng).
 *
 * Module này KHÔNG biết gì về STM32, LCD hay joystick. Nhờ vậy có thể:
 *   - Biên dịch & test ngay trên PC (Mac/Linux) bằng clang/gcc.
 *   - Tái sử dụng nguyên vẹn khi nhúng vào firmware STM32F429.
 *
 * Nguồn ngẫu nhiên (sinh ô mới) được "tiêm" vào qua con trỏ hàm, nên:
 *   - Trên PC: dùng rand().
 *   - Trên STM32: dùng bộ sinh số ngẫu nhiên phần cứng (RNG peripheral).
 */
#ifndef GAME2048_H
#define GAME2048_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#define G2048_SIZE      4                 /* Lưới 4x4 */
#define G2048_WIN_TILE  2048              /* Giá trị ô để thắng */

/* 4 hướng dồn số */
typedef enum {
    G2048_LEFT = 0,
    G2048_RIGHT,
    G2048_UP,
    G2048_DOWN
} g2048_dir_t;

/* Trạng thái ván chơi */
typedef enum {
    G2048_PLAYING = 0,    /* Đang chơi */
    G2048_WON,            /* Đã đạt ô 2048 (vẫn có thể chơi tiếp) */
    G2048_LOST            /* Hết nước đi */
} g2048_state_t;

/*
 * Con trỏ hàm sinh số ngẫu nhiên.
 * Trả về một số nguyên không dấu bất kỳ; lõi game sẽ tự lấy modulo.
 * 'ctx' là dữ liệu tuỳ ý người dùng (vd. handle của RNG), có thể NULL.
 */
typedef uint32_t (*g2048_rng_fn)(void *ctx);

typedef struct {
    uint32_t      grid[G2048_SIZE][G2048_SIZE]; /* 0 = ô trống, còn lại là 2,4,8,... */
    uint32_t      score;                        /* Điểm hiện tại */
    g2048_state_t state;                        /* Trạng thái ván */
    bool          won_announced;                /* Đã báo thắng chưa (để chỉ báo 1 lần) */
    g2048_rng_fn  rng;                          /* Nguồn ngẫu nhiên */
    void         *rng_ctx;                      /* Ngữ cảnh cho rng */
} g2048_game_t;

/* Khởi tạo ván mới: xoá lưới, điểm = 0, sinh 2 ô ban đầu. */
void g2048_init(g2048_game_t *g, g2048_rng_fn rng, void *rng_ctx);

/*
 * Thực hiện một nước đi theo hướng 'dir'.
 * Trả về true nếu lưới có thay đổi (khi đó tự động sinh 1 ô mới và cập nhật state).
 * Trả về false nếu nước đi không hợp lệ (không ô nào di chuyển) -> KHÔNG sinh ô mới.
 */
bool g2048_move(g2048_game_t *g, g2048_dir_t dir);

/* Còn nước đi hợp lệ nào không (có ô trống hoặc có cặp kề nhau bằng nhau). */
bool g2048_can_move(const g2048_game_t *g);

#ifdef __cplusplus
}
#endif

#endif /* GAME2048_H */
