#include <gui/screen1_screen/Screen1View.hpp>
#include <gui/screen1_screen/Screen1Presenter.hpp>
#include <gui/common/FrontendApplication.hpp>

Screen1Presenter::Screen1Presenter(Screen1View& v)
    : view(v)
{

}

void Screen1Presenter::activate()
{
    /* Tắt xử lý joystick analog -> Model::tick sẽ bỏ qua move. */
    if (model) model->setGameActive(false);

    /* Hiển thị high score từ Model lên 7-seg của Screen1. */
    if (model) view.updateHighScore(model->getHighScore());
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
