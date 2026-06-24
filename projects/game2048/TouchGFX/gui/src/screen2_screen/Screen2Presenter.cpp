#include <gui/screen2_screen/Screen2View.hpp>
#include <gui/screen2_screen/Screen2Presenter.hpp>
#include <gui/common/FrontendApplication.hpp>

Screen2Presenter::Screen2Presenter(Screen2View& v)
    : view(v)
{
}

void Screen2Presenter::activate()
{
    /* Bật xử lý joystick analog cho game. */
    if (model) model->setGameActive(true);

    /* Nếu ván trước đã thua, auto-start ván mới khi quay lại Screen2.
     * High score nằm trong Model nên không bị reset. */
    g2048_game_t *g = model->getGame();
    if (g->state == G2048_LOST)
    {
        g2048_init(g, g->rng, g->rng_ctx);
    }

    view.updateBoard(g->grid);
    view.updateScore(model->getScore());
}

void Screen2Presenter::deactivate()
{
    if (model) model->setGameActive(false);
}

void Screen2Presenter::boardChanged()
{
    view.updateBoard(model->getGame()->grid);
}

void Screen2Presenter::scoreChanged(uint32_t score)
{
    view.updateScore(score);
}

void Screen2Presenter::swPressed()
{
    /* Ấn SW từ Screen2 -> thoát về Screen1. Điểm vẫn giữ trong Model. */
    static_cast<FrontendApplication*>(Application::getInstance())
        ->gotoScreen1ScreenNoTransition();
}
