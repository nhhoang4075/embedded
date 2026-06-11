#ifndef SCREEN2VIEW_HPP
#define SCREEN2VIEW_HPP

#include <gui_generated/screen2_screen/Screen2ViewBase.hpp>
#include <gui/screen2_screen/Screen2Presenter.hpp>
#include <gui/common/SevenSegNumber.hpp>
#include <touchgfx/widgets/Image.hpp>

class Screen2View : public Screen2ViewBase
{
public:
    Screen2View();
    virtual ~Screen2View() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
    virtual void handleTickEvent();
protected:
    /* Update one obstacle: it falls down; once it leaves the screen,
       +1 point and respawn in a random lane. Returns true on collision. */
    bool updateObstacle(touchgfx::Image& lb);

    int16_t localImageX;       // car X position
    uint32_t tickCount;
    uint16_t score;            // current game score
    int16_t fallSpeed;         // fall speed (increases with score)

    touchgfx::Image lamb2;     // second obstacle (added in code)
    SevenSegNumber highNum;    // high score (yellow, top-right)
    SevenSegNumber scoreNum;   // current score (green, below highNum)
};

#endif // SCREEN2VIEW_HPP
