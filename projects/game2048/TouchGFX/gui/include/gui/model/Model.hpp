#ifndef MODEL_HPP
#define MODEL_HPP

class ModelListener;

#include "game2048.h"
#include "audio_protocol.h"           /* enum class AudioCmd + kAudioFrameStart */
#include "flash_high_score_store.h"   /* FlashHighScoreStore */
#include <gui/audio/AudioBus.hpp>     /* UartAudioBus mặc định */
#include <stdint.h>

/* Alias tương thích ngược cho các call-site cũ tham chiếu AUDIO_MOVE
 * trực tiếp (Screen1Presenter, Screen2Presenter). Sẽ được gỡ ở Pha 3
 * khi Model chuyển sang AudioBus::send(AudioCmd). */
static constexpr uint8_t AUDIO_MOVE      = static_cast<uint8_t>(AudioCmd::Move);
static constexpr uint8_t AUDIO_MERGE     = static_cast<uint8_t>(AudioCmd::Merge);
static constexpr uint8_t AUDIO_START     = static_cast<uint8_t>(AudioCmd::Start);
static constexpr uint8_t AUDIO_OVER      = static_cast<uint8_t>(AudioCmd::Over);
static constexpr uint8_t AUDIO_NEW_HIGH  = static_cast<uint8_t>(AudioCmd::NewHigh);
static constexpr uint8_t AUDIO_BGM_PLAY  = static_cast<uint8_t>(AudioCmd::BgmPlay);
static constexpr uint8_t AUDIO_BGM_STOP  = static_cast<uint8_t>(AudioCmd::BgmStop);

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

    /* Chuyển sang state MENU: tắt joystick analog, phát BGM. Gọi khi
     * Presenter menu activate. Model đóng gói mọi hành vi transition
     * — Presenter chỉ nói "vào menu", không cần biết BGM hay flag. */
    void enterMenu();

    /* Chuyển sang state GAME: bật joystick analog, tắt BGM, auto-reset
     * nếu ván trước đã LOST. Gọi khi Presenter game activate. */
    void enterGame();

    /* Reset ván + bắn SFX start. enterGame gọi tự động nếu cần —
     * Presenter thường không phải gọi trực tiếp. */
    void resetGame();

    /* Gửi 1 lệnh âm thanh qua AudioBus (framing 2 byte, fire-and-forget). */
    void playSfx(AudioCmd cmd);

    /* Overload byte cho tương thích ngược tạm thời — Presenter đang gọi
     * playSfx(AUDIO_BGM_STOP). Sẽ được xoá sau khi migrate hết. */
    void playSfx(uint8_t cmd) { playSfx(static_cast<AudioCmd>(cmd)); }

    /* Reseed RNG lần đầu user tương tác — lấy entropy từ tick + DWT +
     * ADC noise của joystick. Chỉ tác dụng lần đầu gọi. */
    void reseedRngIfNeeded();

private:
    ModelListener* modelListener;

    /* Trạng thái điều hướng: Menu (Screen1) hay Game (Screen2). Thay
     * cho gameActive bool cũ — API sự kiện, không phải cấu hình. */
    enum class Phase { Menu, Game };
    Phase        phase;

    g2048_game_t game;
    uint32_t     highScore;       /* cache — nạp từ store lúc boot */

    bool         recordBroken;    /* đã phát NEW_HIGH chưa trong ván này */
    bool         overEmitted;     /* đã phát OVER chưa trong ván này */
    bool         rngSeeded;       /* đã reseed sau user input đầu tiên chưa */

    UartAudioBus          audioBus;   /* production; test dùng FakeAudioBus */
    FlashHighScoreStore   scoreStore; /* production; test dùng InMemory */
};

#endif
