#include <gui/model/Model.hpp>
#include <gui/model/ModelListener.hpp>
#include <cstdlib>

extern "C"
{
#include "game2048.h"
#include "joystick.h"
#include "stm32f4xx_hal.h"
}

static uint32_t simulator_rng(void*)
{
    return rand();
}

Model::Model() :
    modelListener(0)
{
    srand(HAL_GetTick());
    g2048_init(&game, simulator_rng, nullptr);
}

void Model::tick()
{
	joy_dir_t dir = joystick_poll();

	    if(dir != JOY_NONE)
	    {
	        if(g2048_move(&game, (g2048_dir_t)dir))
	        {
	            if(modelListener)
	            {
	                modelListener->boardChanged();
	                modelListener->scoreChanged(game.score);
	            }
	        }
	    }
}
