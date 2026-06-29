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
        model->setGameActive(false);
        model->playSfx(AUDIO_BGM_PLAY);   // bat nhac nen tren menu
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
