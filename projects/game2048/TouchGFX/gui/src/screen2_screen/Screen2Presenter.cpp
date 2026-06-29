#include <gui/screen2_screen/Screen2View.hpp>
#include <gui/screen2_screen/Screen2Presenter.hpp>
#include <gui/common/FrontendApplication.hpp>

extern "C"
{
#include "stm32f4xx_hal.h"
#include "main.h"
}

Screen2Presenter::Screen2Presenter(Screen2View& v)
    : view(v)
{
}

void Screen2Presenter::activate()
{
    if (!model) return;

    /* Bật xử lý joystick analog cho game. */
    model->setGameActive(true);

    /* Tat nhac nen khi vao gameplay -> SFX khong bi trong am voi BGM. */
    model->playSfx(AUDIO_BGM_STOP);

    /* Nếu ván trước đã thua, auto-start ván mới khi quay lại Screen2.
     * Model::resetGame() tự bắn SFX START. High score giữ nguyên. */
    g2048_game_t *g = model->getGame();
    if (g->state == G2048_LOST)
    {
        model->resetGame();
    }

    view.updateBoard(g->grid);
    view.updateScore(model->getScore());
    view.updateHighScore(model->getHighScore());
}

void Screen2Presenter::deactivate()
{
    if (model) model->setGameActive(false);
}

void Screen2Presenter::boardChanged()
{
    view.updateBoard(model->getGame()->grid);

    if (model->getGame()->state == G2048_LOST)
        {
            view.showGameOver();
        }
}

void Screen2Presenter::scoreChanged(uint32_t score)
{
    view.updateScore(score);
    view.updateHighScore(model->getHighScore());
}

void Screen2Presenter::swPressed()
{
    g2048_game_t *g = model->getGame();

    /* Đang chơi -> ép game over (highScore đã được Model::tick cập nhật real-time). */
    if (g->state != G2048_LOST)
    {
        g->state = G2048_LOST;
        view.showGameOver();
        return;
    }

    /* Popup Game Over đang hiện -> SW = nút Return, về Screen1. */
    static_cast<FrontendApplication*>(Application::getInstance())
        ->gotoScreen1ScreenNoTransition();
}
