#include <gui/model/Model.hpp>
#include <gui/model/ModelListener.hpp>
#include <cstdlib>

extern "C"
{
#include "game2048.h"
#include "joystick.h"
#include "stm32f4xx_hal.h"
}

static uint32_t simulator_rng(void*)
{
    return rand();
}

Model::Model() :
    modelListener(0),
    highScore(0),
    gameActive(false)
{
    srand(HAL_GetTick());
    g2048_init(&game, simulator_rng, nullptr);
}

void Model::tick()
{
    /* SW joystick luôn được poll, kể cả trên Screen1, để điều hướng. */
    if (joystick_sw_pressed() && modelListener)
    {
        modelListener->swPressed();
    }

    /* Joystick analog chỉ áp dụng khi đang ở Screen2 (chơi game). */
    if (!gameActive)
        return;

    joy_dir_t dir = joystick_poll();
    if (dir == JOY_NONE)
        return;

    if (g2048_move(&game, (g2048_dir_t)dir))
    {
        if (game.score > highScore)
            highScore = game.score;

        if (modelListener)
        {
            modelListener->boardChanged();
            modelListener->scoreChanged(game.score);
        }
    }
}
