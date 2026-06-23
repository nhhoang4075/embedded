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
}

void Screen1View::tearDownScreen()
{
    Screen1ViewBase::tearDownScreen();
}
