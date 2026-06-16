/*
 * game2048.c - Cài đặt lõi logic 2048. Xem mô tả ở game2048.h.
 */
#include "game2048.h"
#include <string.h>

/* ----- Tiện ích nội bộ ----- */

/* Đếm số ô trống trong lưới. */
static int count_empty(const g2048_game_t *g)
{
    int n = 0;
    for (int r = 0; r < G2048_SIZE; r++)
        for (int c = 0; c < G2048_SIZE; c++)
            if (g->grid[r][c] == 0)
                n++;
    return n;
}

/*
 * Sinh một ô mới (90% là 2, 10% là 4) tại một vị trí trống ngẫu nhiên.
 * Giả định lưới còn ít nhất 1 ô trống.
 */
static void spawn_tile(g2048_game_t *g)
{
    int empty = count_empty(g);
    if (empty == 0)
        return;

    uint32_t r = (g->rng ? g->rng(g->rng_ctx) : 0);
    int target = (int)(r % (uint32_t)empty);   /* ô trống thứ 'target' */

    uint32_t value = ((g->rng ? g->rng(g->rng_ctx) : 0) % 10u == 0u) ? 4u : 2u;

    int idx = 0;
    for (int row = 0; row < G2048_SIZE; row++) {
        for (int col = 0; col < G2048_SIZE; col++) {
            if (g->grid[row][col] == 0) {
                if (idx == target) {
                    g->grid[row][col] = value;
                    return;
                }
                idx++;
            }
        }
    }
}

/*
 * Dồn + gộp một "dòng" 4 phần tử về phía chỉ số 0.
 * line[0] là phía mà các ô trượt tới.
 * Cộng điểm gộp vào *score. Trả về true nếu dòng có thay đổi.
 */
static bool slide_line(uint32_t line[G2048_SIZE], uint32_t *score)
{
    uint32_t tmp[G2048_SIZE];
    int n = 0;

    /* 1) Nén: gom các ô khác 0 về đầu, giữ thứ tự. */
    for (int i = 0; i < G2048_SIZE; i++)
        if (line[i] != 0)
            tmp[n++] = line[i];
    for (int i = n; i < G2048_SIZE; i++)
        tmp[i] = 0;

    /* 2) Gộp: cặp kề nhau bằng nhau -> nhân đôi, mỗi ô chỉ gộp 1 lần. */
    for (int i = 0; i < G2048_SIZE - 1; i++) {
        if (tmp[i] != 0 && tmp[i] == tmp[i + 1]) {
            tmp[i] *= 2;
            *score += tmp[i];
            /* dồn phần còn lại lên 1 bậc */
            for (int j = i + 1; j < G2048_SIZE - 1; j++)
                tmp[j] = tmp[j + 1];
            tmp[G2048_SIZE - 1] = 0;
        }
    }

    /* 3) So sánh với dòng gốc để biết có thay đổi không. */
    bool changed = false;
    for (int i = 0; i < G2048_SIZE; i++) {
        if (line[i] != tmp[i])
            changed = true;
        line[i] = tmp[i];
    }
    return changed;
}

/*
 * Trích 4 ô của một "đường" theo hướng dồn vào 'line' (line[0] = phía đích),
 * và ghi lại sau khi xử lý. 'index' là số thứ tự đường (0..3):
 *   - LEFT/RIGHT: index = hàng
 *   - UP/DOWN:    index = cột
 */
static void read_line(const g2048_game_t *g, g2048_dir_t dir, int index,
                      uint32_t line[G2048_SIZE])
{
    for (int i = 0; i < G2048_SIZE; i++) {
        switch (dir) {
        case G2048_LEFT:  line[i] = g->grid[index][i];                 break;
        case G2048_RIGHT: line[i] = g->grid[index][G2048_SIZE - 1 - i];break;
        case G2048_UP:    line[i] = g->grid[i][index];                 break;
        case G2048_DOWN:  line[i] = g->grid[G2048_SIZE - 1 - i][index];break;
        }
    }
}

static void write_line(g2048_game_t *g, g2048_dir_t dir, int index,
                       const uint32_t line[G2048_SIZE])
{
    for (int i = 0; i < G2048_SIZE; i++) {
        switch (dir) {
        case G2048_LEFT:  g->grid[index][i]                  = line[i]; break;
        case G2048_RIGHT: g->grid[index][G2048_SIZE - 1 - i] = line[i]; break;
        case G2048_UP:    g->grid[i][index]                  = line[i]; break;
        case G2048_DOWN:  g->grid[G2048_SIZE - 1 - i][index] = line[i]; break;
        }
    }
}

/* ----- API công khai ----- */

void g2048_init(g2048_game_t *g, g2048_rng_fn rng, void *rng_ctx)
{
    memset(g, 0, sizeof(*g));
    g->rng = rng;
    g->rng_ctx = rng_ctx;
    g->state = G2048_PLAYING;
    spawn_tile(g);
    spawn_tile(g);
}

bool g2048_move(g2048_game_t *g, g2048_dir_t dir)
{
    bool moved = false;
    uint32_t line[G2048_SIZE];

    for (int index = 0; index < G2048_SIZE; index++) {
        read_line(g, dir, index, line);
        if (slide_line(line, &g->score))
            moved = true;
        write_line(g, dir, index, line);
    }

    if (!moved)
        return false;

    /* Kiểm tra thắng (lần đầu chạm 2048). */
    if (!g->won_announced) {
        for (int r = 0; r < G2048_SIZE; r++)
            for (int c = 0; c < G2048_SIZE; c++)
                if (g->grid[r][c] >= G2048_WIN_TILE) {
                    g->state = G2048_WON;
                    g->won_announced = true;
                }
    }

    spawn_tile(g);

    if (!g2048_can_move(g))
        g->state = G2048_LOST;

    return true;
}

bool g2048_can_move(const g2048_game_t *g)
{
    /* Còn ô trống -> còn đi được. */
    if (count_empty(g) > 0)
        return true;

    /* Có cặp kề nhau bằng nhau theo hàng hoặc cột -> còn gộp được. */
    for (int r = 0; r < G2048_SIZE; r++) {
        for (int c = 0; c < G2048_SIZE; c++) {
            uint32_t v = g->grid[r][c];
            if (c + 1 < G2048_SIZE && v == g->grid[r][c + 1]) return true;
            if (r + 1 < G2048_SIZE && v == g->grid[r + 1][c]) return true;
        }
    }
    return false;
}
