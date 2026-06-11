/*
 * play_host.c - Chơi 2048 trên terminal (Mac/Linux) để kiểm chứng lõi game.
 *
 * Build & chạy:
 *   cd projects/game2048
 *   clang -I core core/game2048.c test/play_host.c -o /tmp/play2048 && /tmp/play2048
 *
 * Điều khiển: W = lên, S = xuống, A = trái, D = phải, Q = thoát.
 * (Gõ phím rồi Enter, hoặc gõ liền 'wasd' rồi Enter.)
 */
#include "game2048.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Nguồn ngẫu nhiên cho bản PC: dùng rand() của thư viện chuẩn. */
static uint32_t host_rng(void *ctx)
{
    (void)ctx;
    return (uint32_t)rand();
}

static void draw(const g2048_game_t *g)
{
    printf("\n  Score: %u\n", g->score);
    printf("  +------+------+------+------+\n");
    for (int r = 0; r < G2048_SIZE; r++) {
        printf("  |");
        for (int c = 0; c < G2048_SIZE; c++) {
            if (g->grid[r][c] == 0)
                printf("      |");
            else
                printf("%5u |", g->grid[r][c]);
        }
        printf("\n  +------+------+------+------+\n");
    }
    if (g->state == G2048_WON)  printf("  >>> Ban da dat 2048! Choi tiep duoc.\n");
    if (g->state == G2048_LOST) printf("  >>> GAME OVER. Khong con nuoc di.\n");
}

int main(void)
{
    srand((unsigned)time(NULL));

    g2048_game_t game;
    g2048_init(&game, host_rng, NULL);
    draw(&game);

    int ch;
    while ((ch = getchar()) != EOF) {
        bool act = false;
        g2048_dir_t dir = G2048_LEFT;
        switch (ch) {
        case 'w': case 'W': dir = G2048_UP;    act = true; break;
        case 's': case 'S': dir = G2048_DOWN;  act = true; break;
        case 'a': case 'A': dir = G2048_LEFT;  act = true; break;
        case 'd': case 'D': dir = G2048_RIGHT; act = true; break;
        case 'q': case 'Q': printf("Tam biet!\n"); return 0;
        default: break; /* bỏ qua newline & ký tự khác */
        }
        if (act) {
            g2048_move(&game, dir);
            draw(&game);
            if (game.state == G2048_LOST)
                break;
        }
    }
    return 0;
}
