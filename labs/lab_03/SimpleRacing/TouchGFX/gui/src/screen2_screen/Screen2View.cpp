#include <gui/screen2_screen/Screen2View.hpp>
#include "cmsis_os.h"

extern osMessageQueueId_t Queue1Handle;

Screen2View::Screen2View()
{
	tickCount = 0;
}

void Screen2View::setupScreen()
{
	localImageX = presenter->GetImageX();
    Screen2ViewBase::setupScreen();
    image1.setX(localImageX);
    lamb.setX(14);
    lamb.setY(0);
}

void Screen2View::tearDownScreen()
{
    Screen2ViewBase::tearDownScreen();
    presenter->UpdateImageX(localImageX);
}

void Screen2View::handleTickEvent()
{
	Screen2ViewBase::handleTickEvent();

	tickCount++;
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

	lamb.setY(tickCount*2%320);
	lamb.setX(tickCount*2/320%4*60+15);

	invalidate();
}
