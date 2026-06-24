#ifndef SCREEN1VIEW_HPP
#define SCREEN1VIEW_HPP

#include <gui_generated/screen1_screen/Screen1ViewBase.hpp>
#include <gui/screen1_screen/Screen1Presenter.hpp>
#include <gui/common/SevenSegDigits4.hpp>
#include <touchgfx/Unicode.hpp>

class Screen1View : public Screen1ViewBase
{
public:
    Screen1View();
    virtual ~Screen1View() {}
    virtual void setupScreen();
    virtual void tearDownScreen();

    void updateHighScore(uint32_t score);

protected:
private:
    static const uint16_t HIGH_SCORE_BUFFER_SIZE = 12;
    Unicode::UnicodeChar highScoreBuffer[HIGH_SCORE_BUFFER_SIZE];

    /* 7-seg tạm hiển thị high score. Bỏ khi font có glyph 0-9. */
    SevenSegDigits4 highScoreSeg;
};

#endif // SCREEN1VIEW_HPP
