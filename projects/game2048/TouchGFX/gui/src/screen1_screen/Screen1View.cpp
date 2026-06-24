#include <gui/screen1_screen/Screen1View.hpp>

Screen1View::Screen1View()
{

}

void Screen1View::setupScreen()
{
    Screen1ViewBase::setupScreen();

    /* Wildcard cho "High Score: <>" chưa được Designer wire. */
    textHighScore.setWildcard(highScoreBuffer);
    Unicode::snprintf(highScoreBuffer, HIGH_SCORE_BUFFER_SIZE, "0");

    /* 7-seg tạm hiển thị high score, sát phải textHighScore (y≈225). */
    highScoreSeg.layout(140, 227, touchgfx::Color::getColorFromRGB(0, 0, 0));
    for (int i = 0; i < SevenSegDigits4::SEG_COUNT; ++i)
        add(highScoreSeg.segment(i));
    highScoreSeg.setNumber(0);
}

void Screen1View::tearDownScreen()
{
    Screen1ViewBase::tearDownScreen();
}

void Screen1View::updateHighScore(uint32_t score)
{
    Unicode::snprintf(highScoreBuffer, HIGH_SCORE_BUFFER_SIZE, "%lu", score);
    textHighScore.invalidate();
    highScoreSeg.setNumber((uint16_t)(score > 9999 ? 9999 : score));
}
