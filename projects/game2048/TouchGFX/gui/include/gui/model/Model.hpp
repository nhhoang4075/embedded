#ifndef MODEL_HPP
#define MODEL_HPP

class ModelListener;

#include "game2048.h"

class Model
{
public:
    Model();

    void bind(ModelListener* listener)
    {
        modelListener = listener;
    }

    void tick();

    g2048_game_t* getGame()
    {
        return &game;
    }

    uint32_t getScore() const
    {
        return game.score;
    }

    uint32_t getHighScore() const
    {
        return highScore;
    }

    /* Bật/tắt nhận joystick analog cho game (Screen2 = true, Screen1 = false).
     * SW vẫn được poll ở mọi trạng thái để điều hướng. */
    void setGameActive(bool active)
    {
        gameActive = active;
    }

private:
    ModelListener* modelListener;

    g2048_game_t game;
    uint32_t     highScore;  /* RAM only, mất khi reset/tắt nguồn */
    bool         gameActive;
};

#endif