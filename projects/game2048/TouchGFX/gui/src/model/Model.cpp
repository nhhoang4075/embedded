#include <gui/model/Model.hpp>
#include <gui/model/ModelListener.hpp>
#include <cstdlib>

extern "C"
{
#include "game2048.h"
#include "joystick.h"
#include "stm32f4xx_hal.h"
#include "main.h"
}

static uint32_t simulator_rng(void*)
{
    return rand();
}

Model::Model() :
    modelListener(0),
    highScore(0),
    gameActive(false),
    recordBroken(false),
    overEmitted(false)
{
    srand(HAL_GetTick());
    g2048_init(&game, simulator_rng, nullptr);
}

void Model::playSfx(uint8_t cmd)
{
    /* Fire-and-forget. Timeout 5ms de GUI task khong treo neu ESP32 chet.
     * Dung USART1 PA9 vi PA2 (USART2) bi conflict tren F429-DISC1 board. */
    HAL_UART_Transmit(&huart1, &cmd, 1, 5);
}

void Model::resetGame()
{
    g2048_init(&game, simulator_rng, nullptr);
    recordBroken = false;
    overEmitted  = false;
    playSfx(AUDIO_START);
}

void Model::tick()
{
    /* SW joystick luôn được poll, kể cả trên Screen1, để điều hướng. */
    if (joystick_sw_pressed() && modelListener)
    {
        modelListener->swPressed();
    }

    /* Phát OVER sfx khi vừa transition vào LOST. Bắt được cả 2 đường:
     *   - thua tự nhiên (state đổi trong g2048_move dưới)
     *   - thua chủ động qua swPressed (presenter set state = LOST) */
    if (game.state == G2048_LOST && !overEmitted)
    {
        overEmitted = true;
        playSfx(AUDIO_OVER);
    }

    /* Joystick analog chỉ áp dụng khi đang ở Screen2 (chơi game). */
    if (!gameActive)
        return;

    /* Đã thua thì không xử lý nước đi nữa, đợi resetGame. */
    if (game.state == G2048_LOST)
        return;

    joy_dir_t dir = joystick_poll();
    if (dir == JOY_NONE)
        return;

    uint32_t prevScore = game.score;

    if (!g2048_move(&game, (g2048_dir_t)dir))
        return;

    uint32_t newScore = game.score;
    bool     hadMerge = (newScore > prevScore);

    /* MOVE = swipe không gộp. MERGE = swipe có gộp (điểm tăng). */
    playSfx(hadMerge ? AUDIO_MERGE : AUDIO_MOVE);

    /* Break high score: chỉ lần đầu vượt qua highScore trong ván này. */
    if (!recordBroken && newScore > highScore)
    {
        recordBroken = true;
        playSfx(AUDIO_NEW_HIGH);
    }

    if (newScore > highScore)
        highScore = newScore;

    if (modelListener)
    {
        modelListener->boardChanged();
        modelListener->scoreChanged(newScore);
    }
}
