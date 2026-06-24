#ifndef SCREEN2PRESENTER_HPP
#define SCREEN2PRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class Screen2View;

class Screen2Presenter : public touchgfx::Presenter,
                         public ModelListener
{
public:
    Screen2Presenter(Screen2View& v);

    virtual void activate();
    virtual void deactivate();

    virtual ~Screen2Presenter() {}

    void boardChanged();
    void scoreChanged(uint32_t score);
    /* SW joystick: Screen2 -> goto Screen1 (giữ điểm trong RAM). */
    virtual void swPressed();

private:
    Screen2Presenter();

    Screen2View& view;
};

#endif