#include <gui/screen2_screen/Screen2View.hpp>
#include <gui/screen2_screen/Screen2Presenter.hpp>
#include <gui/common/FrontendApplication.hpp>

/* Presenter chỉ chuyển sự kiện Model ↔ View — không cần HAL/main.h nữa. */

Screen2Presenter::Screen2Presenter(Screen2View& v)
    : view(v)
{
}

void Screen2Presenter::activate()
{
    if (!model) return;

    /* Model tự bật joystick, tắt BGM, auto-reset nếu cần — Presenter
     * chỉ ra tín hiệu "vào ván chơi". */
    model->enterGame();

    g2048_game_t *g = model->getGame();
    view.updateBoard(g->grid);
    view.updateScore(model->getScore());
    view.updateHighScore(model->getHighScore());
}

void Screen2Presenter::deactivate()
{
    /* Deactivate không đồng nghĩa "vào menu" — có thể chuyển sang màn
     * khác. enterMenu chỉ gọi khi Screen1Presenter::activate. */
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
