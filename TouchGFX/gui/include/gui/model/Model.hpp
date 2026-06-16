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

private:
    ModelListener* modelListener;

    g2048_game_t game;
};

#endif