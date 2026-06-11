#include <gui/screen1_screen/Screen1View.hpp>
#include "cmsis_os.h"

extern	osMessageQueueId_t myQueue01Handle;

Screen1View::Screen1View()
{

}

void Screen1View::setupScreen()
{
    Screen1ViewBase::setupScreen();
}

void Screen1View::tearDownScreen()
{
    Screen1ViewBase::tearDownScreen();
}

void Screen1View::tickEventHandler()
{
	uint16_t res;
	if (osMessageQueueGetCount(myQueue01Handle) > 0)
	{
	  osMessageQueueGet(myQueue01Handle, &res, NULL, osWaitForever);

	  circle1.moveTo(res, 50);
	  circle1.invalidate();
	}
}
