#include <gui/screen1_screen/Screen1View.hpp>
#include <cmsis_os.h>

extern osMessageQueueId_t Queue1Handle;

Screen1View::Screen1View()
{
	tickCount = 0;
}

void Screen1View::setupScreen()
{
    Screen1ViewBase::setupScreen();
}

void Screen1View::tearDownScreen()
{
    Screen1ViewBase::tearDownScreen();
}

void Screen1View::handleTickEvent()
{
	static uint32_t prevTick = 0;
	Screen1ViewBase::handleTickEvent();
	tickCount++;
	float rad = 0;

	/* ===== STAGE 1 (demo): hand rotates by tick count =====
	rad = (tickCount % 360) * 3.14f / 180;
	txtrHand.updateZAngle(rad);
	*/

	/* ===== STAGE 2 ("self-study" demo): real second hand using osKernelGetTickCount =====
	uint32_t sysTick = osKernelGetTickCount();
	rad = (sysTick % 60000) * 3.14f / 30000;
	txtrHand.updateZAngle(rad);
	*/

	/* ===== STAGE 3 (PDF sample code): stopwatch controlled by USER_BUTTON via queue =====
	uint8_t res = 0;
	uint32_t count = osMessageQueueGetCount(Queue1Handle);
	if (count > 0)
	{
		osMessageQueueGet(Queue1Handle, &res, NULL, osWaitForever);
		if (res == 'P')
		{
			//update and invalidate the clock hand
			rad = ((tickCount - prevTick) % 360) * 3.14f / 180;
			txtrHand.updateZAngle(rad);
		}
	}
	else
	{
		prevTick = tickCount;
	}
	*/

	/* ===== STAGE 4 (upgrade): true real-time stopwatch =====
	   - Hold button: hand runs at real second-hand speed (1 rev / 60 s)
	   - Release: hand stops in place
	   - Press again: continues from where it stopped (accumulated time) */
	static uint32_t elapsedMs = 0;   // total accumulated stopwatch time
	static uint32_t lastMs = 0;      // timestamp of the previous update
	static bool running = false;

	uint8_t res = 0;
	if (osMessageQueueGetCount(Queue1Handle) > 0)
	{
		osMessageQueueGet(Queue1Handle, &res, NULL, osWaitForever);
		if (res == 'P')
		{
			uint32_t now = osKernelGetTickCount();   // ms since boot
			if (running)
			{
				elapsedMs += now - lastMs;           // accumulate time while the button is held
			}
			lastMs = now;
			running = true;

			rad = (elapsedMs % 60000) * 3.14f / 30000;  // 60000 ms = one revolution
			txtrHand.updateZAngle(rad);
		}
	}
	else
	{
		running = false;   // button released: stop accumulating, hand keeps its position
	}
}
