#ifndef MODEL_HPP
#define MODEL_HPP

#include <stdint.h>
class ModelListener;

class Model
{
public:
    Model();
    long modelTickCount = 0;
    void bind(ModelListener* listener)
    {
        modelListener = listener;
    }

    void tick();
    void update(int16_t x);
    int16_t GetImageX();

    /* Scoring: stores the last game score + high score (kept in RAM
       for the lifetime of the application, shared between screens) */
    void SaveScore(uint16_t s);
    uint16_t GetScore();
    uint16_t GetHighScore();

    int16_t ImageX;
protected:
    ModelListener* modelListener;
    uint16_t score;
    uint16_t highScore;
};

#endif // MODEL_HPP
