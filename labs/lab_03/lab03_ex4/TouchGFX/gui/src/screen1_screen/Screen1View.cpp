#include <gui/screen1_screen/Screen1View.hpp>
#include <touchgfx/Color.hpp>
#include "cmsis_os2.h"

extern osMessageQueueId_t Queue1Handle;

Screen1View::Screen1View()
{

}

void Screen1View::setupScreen()
{
    Screen1ViewBase::setupScreen();

    /* Menu: "SCORE: x" (green) above, "MAX: y" (yellow) below,
       block-font labels + 7-segment digits (no Designer text resources) */
    scoreLabel.setText("SCORE:", 5);
    scoreLabel.setXY(8, 20);
    scoreLabel.setColor(touchgfx::Color::getColorFromRGB(0, 255, 0));
    add(scoreLabel);

    lastScoreNum.setXY(130, 20);
    lastScoreNum.setColor(touchgfx::Color::getColorFromRGB(0, 255, 0));
    lastScoreNum.setValue(presenter->GetScore());
    add(lastScoreNum);

    maxLabel.setText("MAX:", 5);
    maxLabel.setXY(8, 56);
    maxLabel.setColor(touchgfx::Color::getColorFromRGB(255, 255, 0));
    add(maxLabel);

    highScoreNum.setXY(130, 56);
    highScoreNum.setColor(touchgfx::Color::getColorFromRGB(255, 255, 0));
    highScoreNum.setValue(presenter->GetHighScore());
    add(highScoreNum);
}

void Screen1View::tearDownScreen()
{
    Screen1ViewBase::tearDownScreen();
}

void Screen1View::handleTickEvent()
{
	tickCounter += 1;

	/* Start the game when both buttons (L+R) are pressed at the same time
	   ('B' messages sent by defaultTask). Presses during the first ~0.5 s
	   after entering the menu are ignored, so holding the buttons at game
	   over does not instantly restart the game. */
	uint8_t res = 0;
	while (osMessageQueueGetCount(Queue1Handle) > 0)
	{
		osMessageQueueGet(Queue1Handle, &res, NULL, 0);
		if (res == 'B' && tickCounter > 30)
		{
			application().gotoScreen2ScreenCoverTransitionEast();
			return;
		}
	}
}
