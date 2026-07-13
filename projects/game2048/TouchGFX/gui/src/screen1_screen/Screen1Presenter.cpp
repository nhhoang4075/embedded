#include <gui/screen1_screen/Screen1View.hpp>
#include <gui/screen1_screen/Screen1Presenter.hpp>
#include <gui/common/FrontendApplication.hpp>

Screen1Presenter::Screen1Presenter(Screen1View& v)
    : view(v)
{

}

void Screen1Presenter::activate()
{
    if (model)
    {
        /* Model tự lo tắt joystick + phát BGM — Presenter chỉ ra tín hiệu
         * "vào menu". */
        model->enterMenu();
        view.updateHighScore(model->getHighScore());
    }
}

void Screen1Presenter::deactivate()
{

}

void Screen1Presenter::swPressed()
{
    /* Ấn SW từ Screen1 -> sang Screen2 chơi game. */
    static_cast<FrontendApplication*>(Application::getInstance())
        ->gotoScreen2ScreenSlideTransitionEast();
}

void Screen1Presenter::highScoreChanged(uint32_t highScore)
{
    view.updateHighScore(highScore);
}
