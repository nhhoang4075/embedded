#include <gui/screen1_screen/Screen1View.hpp>
#include <gui/screen1_screen/Screen1Presenter.hpp>
#include <gui/common/FrontendApplication.hpp>

Screen1Presenter::Screen1Presenter(Screen1View& v)
    : view(v)
{

}

void Screen1Presenter::activate()
{
    /* Touch bị disable (STMPE811 không phản hồi) -> không bấm được nút Start.
     * Auto chuyển sang Screen2 (màn game). */
    static_cast<FrontendApplication*>(Application::getInstance())
        ->gotoScreen2ScreenSlideTransitionEast();
}

void Screen1Presenter::deactivate()
{

}
