#ifndef MODEL_HPP
#define MODEL_HPP

class ModelListener;

#include "game2048.h"
#include <stdint.h>

/* UART command map (byte gui qua USART2 sang ESP32 audio slave).
 * Phai dong bo voi enum CMD_* o slave/src/main.cpp. */
enum AudioCmd : uint8_t
{
    AUDIO_MOVE      = 0x01,
    AUDIO_MERGE     = 0x02,
    AUDIO_START     = 0x03,
    AUDIO_OVER      = 0x04,
    AUDIO_NEW_HIGH  = 0x05,
    AUDIO_BGM_PLAY  = 0x06,   // vao Screen1 -> bat nhac nen
    AUDIO_BGM_STOP  = 0x07,   // vao Screen2 -> dung nhac nen
};

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

    void setGameActive(bool active)
    {
        gameActive = active;
    }

    /* Reset ván + bắn SFX start. Gọi từ Screen2Presenter khi vào ván mới. */
    void resetGame();

    /* Gửi 1 byte command qua UART2 sang ESP32 (timeout 5ms, fire-and-forget). */
    void playSfx(uint8_t cmd);

private:
    ModelListener* modelListener;

    g2048_game_t game;
    uint32_t     highScore;       /* RAM only, mất khi reset/tắt nguồn */
    bool         gameActive;

    bool         recordBroken;    /* đã phát NEW_HIGH chưa trong ván này */
    bool         overEmitted;     /* đã phát OVER chưa trong ván này */
};

#endif
