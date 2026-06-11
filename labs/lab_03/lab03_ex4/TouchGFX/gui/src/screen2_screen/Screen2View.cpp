#include <gui/screen2_screen/Screen2View.hpp>
#include <touchgfx/Color.hpp>
#include <images/BitmapDatabase.hpp>
#include "cmsis_os2.h"

extern osMessageQueueId_t Queue1Handle;

/* ===== Game constants ===== */
static const int16_t SCREEN_W   = 240;
static const int16_t SCREEN_H   = 320;
static const int16_t CAR_W      = 50;    // redcar bitmap size
static const int16_t CAR_H      = 85;
static const int16_t CAR_Y      = 160;   // car is fixed on the Y axis
static const int16_t LAMB_W     = 32;    // lamb bitmap size
static const int16_t LAMB_H     = 32;
static const int16_t CAR_STEP   = 2;     // px moved per 'L'/'R' message
static const int16_t HITBOX_PAD = 4;     // shrink hitbox for fairness

static uint32_t seed = 12345;

static uint8_t getRandomLane()
{
    seed = seed * 1103515245 + 12345;
    return (seed >> 16) % 4;
}

Screen2View::Screen2View()
{
	tickCount = 0;
	score = 0;
	fallSpeed = 2;
}

void Screen2View::setupScreen()
{
	localImageX = presenter->GetImageX();
    Screen2ViewBase::setupScreen();
    image1.setX(localImageX);

    /* Reset state for a new game */
    score = 0;
    fallSpeed = 2;
    lamb.setX(getRandomLane() * 60 + 15);
    lamb.setY(-LAMB_H);

    /* Second obstacle: enters half a cycle after the first one */
    lamb2.setBitmap(touchgfx::Bitmap(BITMAP_LAMB_ID));
    lamb2.setX(getRandomLane() * 60 + 15);
    lamb2.setY(-LAMB_H - (SCREEN_H + LAMB_H) / 2);
    add(lamb2);

    /* Flush stale L/R commands left in the queue before the game starts */
    osMessageQueueReset(Queue1Handle);

    /* 7-segment scores stacked in the top-right corner:
       MAX (yellow) on top, current score (green) below it */
    highNum.setXY(SCREEN_W - highNum.getWidth() - 4, 4);
    highNum.setColor(touchgfx::Color::getColorFromRGB(255, 255, 0));
    highNum.setValue(presenter->GetHighScore());
    add(highNum);

    scoreNum.setXY(SCREEN_W - scoreNum.getWidth() - 4, 4 + highNum.getHeight() + 4);
    scoreNum.setColor(touchgfx::Color::getColorFromRGB(0, 255, 0));
    scoreNum.setValue(0);
    add(scoreNum);
}

void Screen2View::tearDownScreen()
{
    Screen2ViewBase::tearDownScreen();
    presenter->UpdateImageX(localImageX);
}

bool Screen2View::updateObstacle(touchgfx::Image& lb)
{
	int16_t newY = lb.getY() + fallSpeed;
	if (newY >= SCREEN_H)
	{
		/* Dodged successfully: +1 point, respawn in a random lane, speed up */
		newY = -LAMB_H;
		lb.setX(getRandomLane() * 60 + 15);

		score++;
		fallSpeed = 2 + score / 5;
		if (fallSpeed > 8) fallSpeed = 8;
		scoreNum.setValue(score);
	}
	lb.setY(newY);

	/* AABB collision test between the car and the obstacle */
	int16_t carL = localImageX + HITBOX_PAD;
	int16_t carR = localImageX + CAR_W - HITBOX_PAD;
	int16_t carT = CAR_Y + HITBOX_PAD;
	int16_t carB = CAR_Y + CAR_H - HITBOX_PAD;
	int16_t lmbL = lb.getX() + HITBOX_PAD;
	int16_t lmbR = lb.getX() + LAMB_W - HITBOX_PAD;
	int16_t lmbT = newY + HITBOX_PAD;
	int16_t lmbB = newY + LAMB_H - HITBOX_PAD;

	return (carL < lmbR) && (lmbL < carR) && (carT < lmbB) && (lmbT < carB);
}

void Screen2View::handleTickEvent()
{
	Screen2ViewBase::handleTickEvent();

	tickCount++;

	/* 1) Scroll the track: the 5 track frames rotate every tick */
	switch (tickCount % 5)
	{
	case 0:
		track0.setVisible(true);
		track4.setVisible(false);
		break;
	case 1:
		track1.setVisible(true);
		track0.setVisible(false);
		break;
	case 2:
		track2.setVisible(true);
		track1.setVisible(false);
		break;
	case 3:
		track3.setVisible(true);
		track2.setVisible(false);
		break;
	case 4:
		track4.setVisible(true);
		track3.setVisible(false);
		break;
	default:
		break;
	}

	/* 2) Receive steering commands from the queue ('L'/'R' sent by defaultTask on PG2/PG3 press) */
	uint8_t res = 0;
	while (osMessageQueueGetCount(Queue1Handle) > 0)
	{
		osMessageQueueGet(Queue1Handle, &res, NULL, 0);
		if (res == 'L')
		{
			localImageX -= CAR_STEP;
		}
		else if (res == 'R')
		{
			localImageX += CAR_STEP;
		}
	}
	if (localImageX < 0) localImageX = 0;
	if (localImageX > SCREEN_W - CAR_W) localImageX = SCREEN_W - CAR_W;
	image1.setX(localImageX);

	/* 3) Update both obstacles + collision check */
	bool hit = updateObstacle(lamb);
	hit = updateObstacle(lamb2) || hit;

	if (hit)
	{
		/* Game over: save the score (Model updates the high score) and return to the menu */
		presenter->SaveScore(score);
		presenter->UpdateImageX(localImageX);
		application().gotoScreen1ScreenWipeTransitionEast();
		return;
	}

	invalidate();
}
