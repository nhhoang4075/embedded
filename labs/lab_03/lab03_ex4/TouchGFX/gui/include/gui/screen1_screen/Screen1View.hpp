#ifndef SCREEN1VIEW_HPP
#define SCREEN1VIEW_HPP

#include <gui_generated/screen1_screen/Screen1ViewBase.hpp>
#include <gui/screen1_screen/Screen1Presenter.hpp>
#include <gui/common/SevenSegNumber.hpp>
#include <gui/common/TinyLabel.hpp>

class Screen1View : public Screen1ViewBase
{
public:
    Screen1View();
    virtual ~Screen1View() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
    virtual void handleTickEvent();
protected:
    long tickCounter = 0;
    TinyLabel scoreLabel;         // "SCORE:" (green)
    TinyLabel maxLabel;           // "MAX:"   (yellow)
    SevenSegNumber lastScoreNum;  // last game score (green)
    SevenSegNumber highScoreNum;  // high score (yellow)
};

#endif // SCREEN1VIEW_HPP
