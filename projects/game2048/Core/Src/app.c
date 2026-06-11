/*
 * app.c - Vòng lặp game 2048: đọc joystick -> cập nhật lõi -> vẽ LCD.
 *
 * Nút "chơi lại": dùng nút USER có sẵn trên F429I-DISCO (PA0, nhấn = mức cao).
 * Nếu CubeMX của bạn đặt tên macro khác, sửa 2 #define dưới đây.
 */
#include "app.h"
#include "game2048.h"
#include "render.h"
#include "joystick.h"
#include "rng_hw.h"

/* Nút USER trên board (B1). Trên F429I-DISCO: PA0, nhấn -> đọc được mức 1. */
#ifndef USER_BTN_PORT
#define USER_BTN_PORT  GPIOA
#endif
#ifndef USER_BTN_PIN
#define USER_BTN_PIN   GPIO_PIN_0
#endif
#define USER_BTN_ACTIVE  GPIO_PIN_SET   /* mức khi nhấn */

static g2048_game_t game;

/* Trả về true đúng MỘT lần cho mỗi lần nhấn (chống rung + giữ nút). */
static bool button_pressed(void)
{
    static bool was_down = false;
    bool down = (HAL_GPIO_ReadPin(USER_BTN_PORT, USER_BTN_PIN) == USER_BTN_ACTIVE);
    bool edge = down && !was_down;
    was_down = down;
    if (edge)
        HAL_Delay(20);   /* khử rung đơn giản */
    return edge;
}

void app_run(ADC_HandleTypeDef *hadc, RNG_HandleTypeDef *hrng)
{
    render_init();
    joystick_init(hadc);

    g2048_init(&game, rng_hw, hrng);
    render_board(&game);

    for (;;) {
        /* Nút USER -> ván mới (luôn cho phép). */
        if (button_pressed()) {
            g2048_init(&game, rng_hw, hrng);
            render_board(&game);
        }

        /* Joystick -> nước đi (chỉ khi đang chơi). */
        if (game.state != G2048_LOST) {
            joy_dir_t d = joystick_poll();
            if (d != JOY_NONE) {
                if (g2048_move(&game, (g2048_dir_t)d))
                    render_board(&game);
            }
        }

        HAL_Delay(15);   /* nhịp quét ~66Hz, đủ mượt và êm cho ADC */
    }
}
